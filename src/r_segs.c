/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

    This file is a part of DOOM Retro.

    DOOM Retro is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the license, or (at your
    option) any later version.

    DOOM Retro is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

    DOOM is a registered trademark of id Software LLC, a ZeniMax Media
    company, in the US and/or other countries, and is used without
    permission. All other trademarks are the property of their respective
    holders. DOOM Retro is in no way affiliated with nor endorsed by
    id Software.

==============================================================================
*/

#include "c_cmds.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_config.h"
#include "r_patch.h"

#define WALLSCALEBITS   (20 - FRACBITS)
#define WALLEDGEBITS    (2 * FRACBITS)
#define WALLEDGEUNIT    (INT64_C(1) << WALLEDGEBITS)

static bool         segtextured;        // True if any of the segs textures might be visible.

static bool         markfloor;          // False if the back side is the same plane.
static bool         markceiling;

static bool         maskedtexture;
static int          toptexture;
static int          midtexture;
static int          bottomtexture;

// flat-as-wall-texture: flat index or -1 if unused
static short        topflatnum;
static short        midflatnum;
static short        bottomflatnum;

static bool         missingtoptexture;
static bool         missingmidtexture;
static bool         missingbottomtexture;

static fixed_t      toptexheight;
static fixed_t      midtexheight;
static fixed_t      bottomtexheight;

static byte         *topbrightmap;
static byte         *midbrightmap;
static byte         *bottombrightmap;

static angle_t      rw_normalangle;
static fixed_t      rw_distance;

//
// regular wall
//
static int          rw_x;
static int          rw_stopx;
static angle_t      rw_centerangle;
static fixed_t      rw_offset;
static int64_t      rw_scale;
static int64_t      rw_scalestep;
static fixed_t      rw_midtexturemid;
static fixed_t      rw_toptexturemid;
static fixed_t      rw_bottomtexturemid;

static int64_t      pixhigh;
static int64_t      pixlow;
static int64_t      pixhighstep;
static int64_t      pixlowstep;

static int64_t      topfrac;
static int64_t      topstep;

static int64_t      bottomfrac;
static int64_t      bottomstep;

static lighttable_t **walllights;
static lighttable_t **walllightsnext;

static int          *maskedtexturecol;  // dropoff overflow

unsigned int        maxdrawsegs;

static inline int64_t R_ToWallScale(const fixed_t scale)
{
    return (int64_t)scale * (1 << WALLSCALEBITS);
}

static inline fixed_t R_WallScaleToFixed(const int64_t scale)
{
    return (fixed_t)(scale >> WALLSCALEBITS);
}

static inline int R_WallTopToScreen(const int64_t edge)
{
    return (int)BETWEEN64(-1, (edge + WALLEDGEUNIT - 1) >> WALLEDGEBITS, viewheight);
}

static inline int R_WallBottomToScreen(const int64_t edge)
{
    return (int)BETWEEN64(-1, edge >> WALLEDGEBITS, viewheight);
}

static bool R_LiquidBobs(const sector_t *sector)
{
    if (!sector->heightsec)
        return true;

    return ((viewplayer->mo->subsector->sector->heightsec ? viewplayer->mo->z + viewplayer->viewheight : viewz)
        > sector->heightsec->interpfloorheight);
}

static fixed_t R_LiquidOffset(const sector_t *sector)
{
    if (viewplayer->mo->subsector->sector->heightsec)
    {
        const fixed_t   depth = viewplayer->mo->z + viewplayer->viewheight - sector->interpfloorheight;

        if (depth >= 0)
            return BETWEEN(0, MAX(0, depth - FRACUNIT * 2), animatedliquiddiff);
    }

    return animatedliquiddiff;
}

static lighttable_t **GetLightTable(const int lightlevel)
{
    return scalelight[BETWEEN(0, (lightlevel >> LIGHTSEGSHIFT) + extralight + curline->fakecontrast, LIGHTLEVELS - 1)];
}

static void R_BlastMaskedSegColumn(const rcolumn_t *column)
{
    unsigned char   *pixels = column->pixels;

    dc_ceilingclip = mceilingclip[dc_x] + 1;
    dc_floorclip = mfloorclip[dc_x] - 1;

    while (dc_numposts--)
    {
        const rpost_t   *post = &column->posts[dc_numposts];
        const int       topdelta = post->topdelta;

        // calculate unclipped screen coordinates for post
        const int64_t   topscreen = sprtopscreen + (int64_t)spryscale * topdelta + 1;

        if ((dc_yh = MIN((int)((topscreen + (int64_t)spryscale * post->length) >> FRACBITS), dc_floorclip)) >= 0)
            if ((dc_yl = MAX(dc_ceilingclip, (int)((topscreen + FRACUNIT - 1) >> FRACBITS))) <= dc_yh)
            {
                dc_texturefrac = dc_texturemid - (topdelta << FRACBITS) + FixedMul((dc_yl - centery) << FRACBITS, dc_iscale);
                dc_source = pixels + topdelta;
                colfunc();
            }
    }
}

//
// R_RenderMaskedSegRange
//
void R_RenderMaskedSegRange(const drawseg_t *ds, const int x1, const int x2)
{
    int             texnum;
    int64_t         texheight;
    const rpatch_t  *patch;
    int64_t         scale;

    curline = ds->curline;
    frontsector = curline->frontsector;
    backsector = curline->backsector;

    if (curline->sidedef->midflat >= 0)
    {
        const rpatch_t  *flat = R_CacheFlatAsPatch(curline->sidedef->midflat);

        texnum = 0;
        texheight = ((fixed_t)flat->height) << FRACBITS;
        patch = flat;
    }
    else
    {
        texnum = texturetranslation[curline->sidedef->midtexture];
        texheight = textureheight[texnum];
        patch = NULL;
    }

    // Calculate light table.
    // Use different light tables for horizontal/vertical.
    // killough 04/13/98: get correct lightlevel for 2s normal textures
    if (fixedcolormap)
    {
        dc_colormap[0] = fixedcolormap;
        dc_nextcolormap[0] = fixedcolormap;
        dc_sectorcolormap = (frontsector->colormap && !ISINVULNERABILITYCOLORMAP(viewplayer->fixedcolormap) ?
            colormaps[frontsector->colormap] : nocolormap);
        colfunc = (curline->linedef->tranlump >= 0 ? tl50segcolfunc : segcolfunc);
    }
    else
    {
        sector_t    tempsec;
        const short lightlevel = R_FakeFlat(frontsector, &tempsec, NULL, NULL, false)->lightlevel;

        walllights = GetLightTable(lightlevel - 2);
        walllightsnext = GetLightTable(lightlevel + 2);

        if (usebrightmaps && !nobrightmap[texnum] && brightmap[texnum])
        {
            dc_brightmap = brightmap[texnum];
            colfunc = (curline->linedef->tranlump >= 0 ? tl50bmapsegcolfunc : bmapsegcolfunc);
        }
        else
            colfunc = (curline->linedef->tranlump >= 0 ? tl50segcolfunc : segcolfunc);

        dc_sectorcolormap = (frontsector->colormap ? colormaps[frontsector->colormap] : nocolormap);
    }

    maskedtexturecol = ds->maskedtexturecol;
    scale = R_ToWallScale(ds->scale) + (x1 - ds->x1) * ds->scalestep;

    mceilingclip = ds->sprtopclip;
    mfloorclip = ds->sprbottomclip;

    // find positioning
    if (curline->linedef->flags & ML_DONTPEGBOTTOM)
        dc_texturemid = MAX(frontsector->interpfloorheight, backsector->interpfloorheight)
            + (fixed_t)texheight - viewz + curline->sidedef->rowoffset;
    else
        dc_texturemid = MIN(frontsector->interpceilingheight, backsector->interpceilingheight) - viewz
            + curline->sidedef->rowoffset;

    if (!patch)
        patch = R_CacheTextureCompositePatchNum(texnum);

    // draw the columns
    for (dc_x = x1; dc_x <= x2; dc_x++, scale += ds->scalestep)
    {
        if (maskedtexturecol[dc_x] != INT_MAX)
        {
            const rcolumn_t *column = R_GetPatchColumnWrapped(patch, maskedtexturecol[dc_x]);

            if ((dc_numposts = column->numposts))
            {
                spryscale = R_WallScaleToFixed(scale);

                // killough 03/02/98:
                //
                // This calculation used to overflow and cause crashes in DOOM:
                //
                // sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);
                //
                // This code fixes it, by using double-precision intermediate
                // arithmetic and by skipping the drawing of 2s normals whose
                // mapping to screen coordinates is totally out of range:
                const int64_t   t = ((int64_t)centeryfrac << FRACBITS) - (int64_t)dc_texturemid * spryscale;

                // skip if the texture is out of screen's range
                if (t + texheight * spryscale < 0 || t >((int64_t)SCREENHEIGHT << (FRACBITS * 2)))
                    continue;

                sprtopscreen = (int64_t)(t >> FRACBITS);

                // calculate lighting
                if (!fixedcolormap)
                {
                    const int   index = MIN((spryscale >> LIGHTSCALESHIFT), MAXLIGHTSCALE - 1);

                    dc_colormap[0] = walllights[index];
                    dc_nextcolormap[0] = walllightsnext[index];
                    dc_z = ((spryscale >> 5) & 255);
                }

                dc_iscale = UINT_MAX / (unsigned int)spryscale;

                // draw the texture
                R_BlastMaskedSegColumn(column);
                maskedtexturecol[dc_x] = INT_MAX;   // dropoff overflow
            }
        }
    }
}

//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling textures.
// CALLED: CORE LOOPING ROUTINE.
//
static bool didsolidcol;

static void R_RenderSegLoop(void)
{
    if (fixedcolormap)
    {
        dc_colormap[0] = fixedcolormap;
        dc_nextcolormap[0] = fixedcolormap;
    }

    for (; rw_x < rw_stopx; rw_x++)
    {
        fixed_t     texturecolumn = 0;
        bool        samecolormap = false;

        // no space above wall?
        int         bottom;
        int         top = ceilingclip[rw_x] + 1;

        // mark floor/ceiling areas
        const int   yl = MAX(R_WallTopToScreen(topfrac), top);
        int         yh = R_WallBottomToScreen(bottomfrac);

        if (markceiling)
        {
            if (top <= (bottom = MIN(yl, floorclip[rw_x]) - 1))
            {
                ceilingplane->top[rw_x] = top;
                ceilingplane->bottom[rw_x] = bottom;
                ceilingplane->modified = true;
            }

            ceilingclip[rw_x] = bottom;
        }

        if (yh > (bottom = floorclip[rw_x] - 1))
            yh = bottom;

        if (markfloor)
        {
            if ((top = MAX(ceilingclip[rw_x], yh) + 1) <= bottom)
            {
                floorplane->top[rw_x] = top;
                floorplane->bottom[rw_x] = bottom;
                floorplane->modified = true;
            }

            floorclip[rw_x] = top;
        }

        // texturecolumn and lighting are independent of wall tiers
        if (segtextured)
        {
            const fixed_t   fracscale = R_WallScaleToFixed(rw_scale);

            // calculate texture offset and lighting
            const angle_t   angle = ((rw_centerangle + xtoviewangle[rw_x]) >> ANGLETOFINESHIFT) & 0x0FFF;

            texturecolumn = (rw_offset - FixedMul(finetangent[angle], rw_distance)) >> FRACBITS;

            if (fixedcolormap)
                dc_sectorcolormap = (frontsector->colormap && !ISINVULNERABILITYCOLORMAP(viewplayer->fixedcolormap) ?
                    colormaps[frontsector->colormap] : nocolormap);
            else
            {
                const int   index = MIN(fracscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1);

                dc_colormap[0] = walllights[index];
                dc_nextcolormap[0] = walllightsnext[index];
                dc_z = ((fracscale >> 5) & 255);
                dc_sectorcolormap = (frontsector->colormap ? colormaps[frontsector->colormap] : nocolormap);
            }

            dc_x = rw_x;
            dc_iscale = UINT_MAX / (unsigned int)fracscale;

            if (r_ditheredlighting)
                samecolormap = !memcmp(dc_colormap, dc_nextcolormap, sizeof(dc_colormap));
        }

        // draw the wall tiers
        if (midtexture && yh >= yl)
        {
            // single sided line
            dc_yl = yl;
            dc_yh = yh;

            if (missingmidtexture)
                missingcolfunc();
            else
            {
                dc_source = R_GetTextureColumn((midflatnum >= 0 ? R_CacheFlatAsPatch(midflatnum) :
                    R_CacheTextureCompositePatchNum(midtexture)), texturecolumn);
                dc_texturemid = rw_midtexturemid;
                dc_texheight = midtexheight;

                if (midbrightmap)
                {
                    dc_brightmap = midbrightmap;

                    if (r_ditheredlighting)
                    {
                        if (samecolormap)
                            altbmapwallcolfunc();
                        else
                            bmapwallcolfunc();
                    }
                    else
                        bmapwallcolfunc();
                }
                else if (r_ditheredlighting)
                {
                    if (samecolormap)
                        altwallcolfunc();
                    else
                        wallcolfunc();
                }
                else
                    wallcolfunc();
            }

            ceilingclip[rw_x] = viewheight;
            floorclip[rw_x] = -1;
        }
        else
        {
            // two sided line
            if (toptexture)
            {
                // top wall
                const int   mid = MIN(R_WallBottomToScreen(pixhigh), floorclip[rw_x] - 1);

                pixhigh += pixhighstep;

                if (mid >= yl)
                {
                    dc_yl = yl;
                    dc_yh = mid;

                    if (missingtoptexture)
                        missingcolfunc();
                    else
                    {
                        dc_source = R_GetTextureColumn((topflatnum >= 0 ? R_CacheFlatAsPatch(topflatnum) :
                            R_CacheTextureCompositePatchNum(toptexture)), texturecolumn);
                        dc_texturemid = rw_toptexturemid;
                        dc_texheight = toptexheight;

                        if (topbrightmap)
                        {
                            dc_brightmap = topbrightmap;

                            if (r_ditheredlighting)
                            {
                                if (samecolormap)
                                    altbmapwallcolfunc();
                                else
                                    bmapwallcolfunc();
                            }
                            else
                                bmapwallcolfunc();
                        }
                        else if (r_ditheredlighting)
                        {
                            if (samecolormap)
                                altwallcolfunc();
                            else
                                wallcolfunc();
                        }
                        else
                            wallcolfunc();
                    }

                    ceilingclip[rw_x] = mid;
                }
                else
                    ceilingclip[rw_x] = yl - 1;
            }
            else
                // no top wall
                if (markceiling)
                    ceilingclip[rw_x] = yl - 1;

            if (bottomtexture)
            {
                // bottom wall
                const int   mid = MAX(R_WallTopToScreen(pixlow), ceilingclip[rw_x] + 1);

                pixlow += pixlowstep;

                if (mid <= yh)
                {
                    dc_yl = mid;
                    dc_yh = yh;

                    if (missingbottomtexture)
                        missingcolfunc();
                    else
                    {
                        dc_source = R_GetTextureColumn(bottomflatnum >= 0 ? R_CacheFlatAsPatch(bottomflatnum) :
                            R_CacheTextureCompositePatchNum(bottomtexture), texturecolumn);
                        dc_texturemid = rw_bottomtexturemid;
                        dc_texheight = bottomtexheight;

                        if (bottombrightmap)
                        {
                            dc_brightmap = bottombrightmap;

                            if (r_ditheredlighting)
                            {
                                if (samecolormap)
                                    altbmapwallcolfunc();
                                else
                                    bmapwallcolfunc();
                            }
                            else
                                bmapwallcolfunc();
                        }
                        else if (r_ditheredlighting)
                        {
                            if (samecolormap)
                                altwallcolfunc();
                            else
                                wallcolfunc();
                        }
                        else
                            wallcolfunc();
                    }

                    floorclip[rw_x] = mid;
                }
                else
                    floorclip[rw_x] = yh + 1;
            }
            else
                // no bottom wall
                if (markfloor)
                    floorclip[rw_x] = yh + 1;

            // cph - if we completely blocked further sight through this column,
            // add this info to the solid columns array for r_bsp.c
            if ((markceiling || markfloor) && floorclip[rw_x] <= ceilingclip[rw_x] + 1)
            {
                solidcol[rw_x] = 1;
                didsolidcol = true;
            }

            // save texturecolumn for backdrawing of masked midtexture
            if (maskedtexture)
                maskedtexturecol[rw_x] = texturecolumn;
        }

        rw_scale += rw_scalestep;
        topfrac += topstep;
        bottomfrac += bottomstep;
    }
}

//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale for the current line (horizontal span) at the given angle.
// rw_distance must be calculated first.
//
static fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
    const angle_t   angle = ANG90 + visangle;
    const int       den = FixedMul(rw_distance, finesine[angle >> ANGLETOFINESHIFT]);
    const fixed_t   num = FixedMul(projection, finesine[(angle + viewangle - rw_normalangle) >> ANGLETOFINESHIFT]);

    return MAX(256, FixedDiv(num, den));
}

//
// R_StoreWallRange
// A wall segment will be drawn between start and stop pixels (inclusive).
//
void R_StoreWallRange(const int start, const int stop)
{
    int64_t         dx, dy;
    int64_t         dx1, dy1;
    int64_t         len;
    int             worldtop;
    int             worldbottom;
    int             worldhigh = 0;
    int             worldlow = 0;
    side_t          *sidedef;
    unsigned short  flags;
    size_t          span;
    const int       steps = MAX(1, stop - start);
    int64_t         screenmid;
    int64_t         scaledelta;

    if (!(linedef = curline->linedef))
        return;

    flags = linedef->flags;

    // mark the segment as visible for automap
    if (!menuactive && (am_dynamic || !automapactive))
    {
        if (!(flags & ML_MAPPED))
            nummappedlines++;

        if (!(flags & ML_DONTDRAW))
            linedef->flags |= ML_MAPPED;
    }

    // [BH] if in automap, we're done now that line is mapped
    if (automapactive)
        return;

    sidedef = curline->sidedef;
    topflatnum = midflatnum = bottomflatnum = -1;

    // killough 01/98 -- fix 2s line HOM
    if (ds_p == drawsegs + maxdrawsegs)
    {
        const size_t    pos = ds_p - drawsegs;

        maxdrawsegs = (maxdrawsegs ? maxdrawsegs * 2 : MAXDRAWSEGS);
        drawsegs = I_Realloc(drawsegs, maxdrawsegs * sizeof(*drawsegs));
        ds_p = drawsegs + pos;
    }

    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle + ANG90;

    // shift right to avoid possibility of int64 overflow in rw_distance calculation
    dx = curline->dx;
    dy = curline->dy;
    dx1 = ((int64_t)viewx - curline->v1->x) >> 1;
    dy1 = ((int64_t)viewy - curline->v1->y) >> 1;
    len = curline->length;
    rw_distance = (fixed_t)((((int64_t)dy * dx1 - (int64_t)dx * dy1) << 1) / len);

    ds_p->x1 = start;
    rw_x = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    rw_stopx = stop + 1;
    span = (int)((int64_t)rw_stopx - start);

    if ((size_t)span * sizeof(*lastopening) + (size_t)(lastopening - openings) > MAXOPENINGS)
        return;

    worldtop = frontsector->interpceilingheight - viewz;
    worldbottom = frontsector->interpfloorheight - viewz;

    // [BH] animate liquid sectors
    if (frontsector->terraintype >= LIQUID && R_LiquidBobs(frontsector) && r_liquid_bob)
        worldbottom += R_LiquidOffset(frontsector);

    // calculate scale at both ends and step
    ds_p->scale = R_ScaleFromGlobalAngle(xtoviewangle[start]);
    ds_p->scale2 = (stop > start ? R_ScaleFromGlobalAngle(xtoviewangle[stop]) : ds_p->scale);
    rw_scale = R_ToWallScale(ds_p->scale);
    ds_p->scalestep = rw_scalestep = R_ToWallScale(ds_p->scale2 - ds_p->scale) / steps;
    ds_p->minscale = MIN(ds_p->scale, ds_p->scale2);
    ds_p->maxscale = MAX(ds_p->scale, ds_p->scale2);

    // calculate texture boundaries and decide if floor/ceiling marks are needed
    midtexture = 0;
    toptexture = 0;
    bottomtexture = 0;
    maskedtexture = false;
    ds_p->maskedtexturecol = NULL;

    if (backsector)
    {
        int liquidoffset = 0;

        // two sided line
        if (linedef->r_flags & RF_CLOSED)
        {
            ds_p->sprtopclip = viewheightarray;
            ds_p->sprbottomclip = negonearray;
            ds_p->silhouette = SIL_BOTH;
        }
        else
        {
            ds_p->sprtopclip = NULL;
            ds_p->sprbottomclip = NULL;
            ds_p->silhouette = SIL_NONE;

            if (frontsector->interpfloorheight > backsector->interpfloorheight
                || backsector->interpfloorheight > viewz)
                ds_p->silhouette = SIL_BOTTOM;

            if (frontsector->interpceilingheight < backsector->interpceilingheight
                || backsector->interpceilingheight < viewz)
                ds_p->silhouette |= SIL_TOP;
        }

        worldhigh = backsector->interpceilingheight - viewz;
        worldlow = backsector->interpfloorheight - viewz;

        // hack to allow height changes in outdoor areas
        if (frontsector->ceilingpic == skyflatnum && backsector->ceilingpic == skyflatnum)
            worldtop = worldhigh;

        // [BH] animate liquid sectors
        if (backsector->terraintype >= LIQUID
            && backsector->interpfloorheight >= frontsector->interpfloorheight
            && R_LiquidBobs(backsector) && !sidedef->midtexture && r_liquid_bob)
        {
            liquidoffset = R_LiquidOffset(backsector);
            worldlow += liquidoffset;
        }

        markfloor = (worldlow != worldbottom
            || backsector->floorpic != frontsector->floorpic
            || backsector->lightlevel != frontsector->lightlevel
            || backsector->colormap != frontsector->colormap
            || backsector->floorrotation != frontsector->floorrotation

            // killough 03/07/98: add checks for (x,y) offsets
            || backsector->floorxoffset != frontsector->floorxoffset
            || backsector->flooryoffset != frontsector->flooryoffset

            // killough 04/15/98: prevent 2s normals from bleeding through deep water
            || frontsector->heightsec

            // killough 04/17/98: draw floors if different light levels
            || backsector->floorlightsec != frontsector->floorlightsec

            // draw floors if different specials
            || backsector->special != frontsector->special);

        markceiling = (worldhigh != worldtop
            || backsector->ceilingpic != frontsector->ceilingpic
            || backsector->lightlevel != frontsector->lightlevel
            || backsector->colormap != frontsector->colormap
            || backsector->ceilingrotation != frontsector->ceilingrotation

            // killough 03/07/98: add checks for (x,y) offsets
            || backsector->ceilingxoffset != frontsector->ceilingxoffset
            || backsector->ceilingyoffset != frontsector->ceilingyoffset

            // killough 04/15/98: prevent 2s normals from bleeding through fake ceilings
            || (frontsector->heightsec && frontsector->ceilingpic != skyflatnum)

            // killough 04/17/98: draw ceilings if different light levels
            || backsector->ceilinglightsec != frontsector->ceilinglightsec);

        if (backsector->interpceilingheight <= frontsector->interpfloorheight
            || backsector->interpfloorheight >= frontsector->interpceilingheight)
        {
            // closed door
            markceiling = true;
            markfloor = true;
        }

        if (worldhigh < worldtop)
        {
            // top texture
            if ((missingtoptexture = sidedef->missingtoptexture))
                toptexture = -1;
            else if (sidedef->topflat >= 0)
            {
                const rpatch_t  *flat = R_CacheFlatAsPatch(sidedef->topflat);
                const fixed_t   height = flat->height << FRACBITS;

                toptexture = 1;
                topflatnum = sidedef->topflat;
                toptexheight = ((linedef->r_flags & RF_TOP_TILE) ? 0 : flat->height);
                topbrightmap = NULL;
                rw_toptexturemid = ((linedef->flags & ML_DONTPEGTOP) ? worldtop :
                    backsector->interpceilingheight + height - viewz) + FixedMod(sidedef->rowoffset, height);
            }
            else
            {
                fixed_t height;

                toptexture = texturetranslation[sidedef->toptexture];
                height = textureheight[toptexture];
                toptexheight = ((linedef->r_flags & RF_TOP_TILE) ? 0 : (height >> FRACBITS));
                topbrightmap = (usebrightmaps && !nobrightmap[toptexture] ? brightmap[toptexture] : NULL);
                rw_toptexturemid = ((linedef->flags & ML_DONTPEGTOP) ? worldtop :
                    backsector->interpceilingheight + height - viewz) + FixedMod(sidedef->rowoffset, height);
            }
        }

        if (worldlow > worldbottom && frontsector->interpfloorheight != backsector->interpfloorheight)
        {
            // bottom texture
            if ((missingbottomtexture = sidedef->missingbottomtexture))
                bottomtexture = -1;
            else if (sidedef->bottomflat >= 0)
            {
                const rpatch_t  *flat = R_CacheFlatAsPatch(sidedef->bottomflat);
                const fixed_t   height = flat->height << FRACBITS;

                bottomtexture = 1;
                bottomflatnum = sidedef->bottomflat;
                bottomtexheight = ((linedef->r_flags & RF_BOT_TILE) ? 0 : flat->height);
                bottombrightmap = NULL;
                rw_bottomtexturemid = ((linedef->flags & ML_DONTPEGBOTTOM) ? worldtop : worldlow - liquidoffset)
                    + FixedMod(sidedef->rowoffset, height);

                if (liquidoffset)
                    rw_bottomtexturemid += 4 * FRACUNIT;
            }
            else
            {
                fixed_t height;

                bottomtexture = texturetranslation[sidedef->bottomtexture];
                height = textureheight[bottomtexture];
                bottomtexheight = ((linedef->r_flags & RF_BOT_TILE) ? 0 : (height >> FRACBITS));
                bottombrightmap = (usebrightmaps && !nobrightmap[bottomtexture] ? brightmap[bottomtexture] : NULL);
                rw_bottomtexturemid = ((linedef->flags & ML_DONTPEGBOTTOM) ? worldtop : worldlow - liquidoffset)
                    + FixedMod(sidedef->rowoffset, height);

                if (liquidoffset)
                    rw_bottomtexturemid += 4 * FRACUNIT;
            }
        }

        // allocate space for masked texture tables
        if (sidedef->midtexture || sidedef->midflat >= 0)
        {
            // masked midtexture
            maskedtexture = true;
            ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
            lastopening += (size_t)rw_stopx - rw_x;
        }
    }
    else
    {
        // single sided line
        if ((missingmidtexture = sidedef->missingmidtexture))
            midtexture = -1;
        else if (sidedef->midflat >= 0)
        {
            const rpatch_t  *flat = R_CacheFlatAsPatch(sidedef->midflat);
            const fixed_t   height = flat->height << FRACBITS;

            midtexture = 1;
            midflatnum = sidedef->midflat;
            midtexheight = ((linedef->r_flags & RF_MID_TILE) ? 0 : flat->height);
            midbrightmap = NULL;
            rw_midtexturemid = ((linedef->flags & ML_DONTPEGBOTTOM) ? frontsector->interpfloorheight
                + height - viewz : worldtop) + FixedMod(sidedef->rowoffset, height);
        }
        else
        {
            fixed_t height;

            midtexture = texturetranslation[sidedef->midtexture];
            height = textureheight[midtexture];
            midtexheight = ((linedef->r_flags & RF_MID_TILE) ? 0 : (height >> FRACBITS));
            midbrightmap = (usebrightmaps && !nobrightmap[midtexture] ? brightmap[midtexture] : NULL);
            rw_midtexturemid = ((linedef->flags & ML_DONTPEGBOTTOM) ? frontsector->interpfloorheight
                + height - viewz : worldtop) + FixedMod(sidedef->rowoffset, height);
        }

        // a single sided line is terminal, so it must mark ends
        markfloor = true;
        markceiling = true;

        ds_p->sprtopclip = viewheightarray;
        ds_p->sprbottomclip = negonearray;
        ds_p->silhouette = SIL_BOTH;
    }

    // calculate rw_offset (only needed for textured lines)
    if ((segtextured = (midtexture || toptexture || bottomtexture || maskedtexture
        || midflatnum >= 0 || topflatnum >= 0 || bottomflatnum >= 0)))
    {
        rw_offset = (fixed_t)(((dx * dx1 + dy * dy1) / len) << 1) + sidedef->textureoffset + curline->offset;
        rw_centerangle = ANG90 + viewangle - rw_normalangle;

        // calculate light table
        //  use different light tables for horizontal/vertical
        if (!fixedcolormap)
        {
            const short lightlevel = frontsector->lightlevel;

            walllights = GetLightTable(lightlevel - 2);
            walllightsnext = GetLightTable(lightlevel + 2);
        }
    }

    // if a floor/ceiling plane is on the wrong side of the view plane, it is definitely invisible
    //  and doesn't need to be marked.

    // killough 03/07/98: add deep water check
    if (!frontsector->heightsec)
    {
        if (frontsector->interpfloorheight >= viewz)
            markfloor = false;          // above view plane

        if (frontsector->interpceilingheight <= viewz && frontsector->ceilingpic != skyflatnum)
            markceiling = false;        // below view plane
    }

    screenmid = (int64_t)centery * WALLEDGEUNIT;
    scaledelta = (int64_t)ds_p->scale - ds_p->scale2;
    topfrac = screenmid - (int64_t)worldtop * ds_p->scale;
    topstep = worldtop * scaledelta / steps;
    bottomfrac = screenmid - (int64_t)worldbottom * ds_p->scale;
    bottomstep = worldbottom * scaledelta / steps;

    if (backsector)
    {
        if (worldhigh < worldtop)
        {
            pixhigh = screenmid - (int64_t)worldhigh * ds_p->scale;
            pixhighstep = worldhigh * scaledelta / steps;
        }

        if (worldlow > worldbottom)
        {
            pixlow = screenmid - (int64_t)worldlow * ds_p->scale;
            pixlowstep = worldlow * scaledelta / steps;
        }
    }

    // render it
    if (markceiling)
    {
        // killough 04/11/98: add NULL ptr checks
        if (ceilingplane)
            ceilingplane = R_CheckPlane(ceilingplane, rw_x, rw_stopx - 1);
        else
            markceiling = false;
    }

    if (markfloor)
    {
        // killough 04/11/98: add NULL ptr checks
        if (floorplane)
        {
            // cph 04/18/03: ceilingplane and floorplane might be the same
            // visplane (e.g. if both skies); R_CheckPlane() doesn't know about
            // modifications to the plane that might happen in parallel with the check
            // being made, so we have to override it and split them anyway if that is
            // a possibility, otherwise the floor marking would overwrite the ceiling
            // marking, resulting in HOM.
            if (markceiling && ceilingplane == floorplane)
                floorplane = R_DupPlane(floorplane, rw_x, rw_stopx - 1);
            else
                floorplane = R_CheckPlane(floorplane, rw_x, rw_stopx - 1);
        }
        else
            markfloor = false;
    }

    didsolidcol = false;
    R_RenderSegLoop();

    ds_p->sameliquidfloor = (maskedtexture && backsector && !didsolidcol
        && !(ds_p->silhouette & SIL_BOTTOM) && !frontsector->heightsec && !backsector->heightsec
        && frontsector->terraintype >= LIQUID && backsector->terraintype >= LIQUID
        && frontsector->interpfloorheight == backsector->interpfloorheight);

    if ((backsector && didsolidcol) || maskedtexture)
        ds_p->silhouette = SIL_BOTH;

    // save sprite clipping info
    if ((ds_p->silhouette & SIL_TOP) && !ds_p->sprtopclip)
    {
        memcpy(lastopening, ceilingclip + start, sizeof(*lastopening) * span);
        ds_p->sprtopclip = lastopening - start;
        lastopening += span;
    }

    if ((ds_p->silhouette & SIL_BOTTOM) && !ds_p->sprbottomclip)
    {
        memcpy(lastopening, floorclip + start, sizeof(*lastopening) * span);
        ds_p->sprbottomclip = lastopening - start;
        lastopening += span;
    }

    ds_p++;
}
