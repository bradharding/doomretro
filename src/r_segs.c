/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "doomstat.h"
#include "i_system.h"
#include "m_config.h"

static bool         segtextured;        // True if any of the segs textures might be visible.

static bool         markfloor;          // False if the back side is the same plane.
static bool         markceiling;

static bool         maskedtexture;
static int          toptexture;
static int          midtexture;
static int          bottomtexture;

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
static fixed_t      rw_scale;
static fixed_t      rw_scalestep;
static fixed_t      rw_midtexturemid;
static fixed_t      rw_toptexturemid;
static fixed_t      rw_bottomtexturemid;

static int64_t      pixhigh;
static int64_t      pixlow;
static fixed_t      pixhighstep;
static fixed_t      pixlowstep;

static int64_t      topfrac;
static fixed_t      topstep;

static int64_t      bottomfrac;
static fixed_t      bottomstep;

static lighttable_t **walllights;
static lighttable_t **walllightsnext;

static int          *maskedtexturecol;  // dropoff overflow

unsigned int        maxdrawsegs;

//
// R_FixWiggle()
// Dynamic wall/texture rescaler, AKA "WiggleHack II"
//  by Kurt "kb1" Baumgardner ("kb") and Andrey "Entryway" Budko ("e6y")
//
//  [kb] When the rendered view is positioned, such that the viewer is
//   looking almost parallel down a wall, the result of the scale
//   calculation in R_ScaleFromGlobalAngle becomes very large. And, the
//   taller the wall, the larger that value becomes. If these large
//   values were used as-is, subsequent calculations would overflow,
//   causing full-screen HOM, and possible program crashes.
//
//  Therefore, Vanilla DOOM clamps this scale calculation, preventing it
//   from becoming larger than 0x400000 (64 * FRACUNIT). This number was
//   chosen carefully, to allow reasonably-tight angles, with reasonably
//   tall sectors to be rendered, within the limits of the fixed-point
//   math system being used. When the scale gets clamped, DOOM cannot
//   properly render the wall, causing an undesirable wall-bending
//   effect that I call "floor wiggle". Not a crash, but still ugly.
//
//  Modern source ports offer higher video resolutions, which worsens
//   the issue. And, DOOM is simply not adjusted for the taller walls
//   found in many PWADs.
//
//  This code attempts to correct these issues, by dynamically
//   adjusting the fixed-point math, and the maximum scale clamp,
//   on a wall-by-wall basis. This has 2 effects:
//
//  1. Floor wiggle is greatly reduced and/or eliminated.
//  2. Overflow is no longer possible, even in levels with maximum
//     height sectors (65535 is the theoretical height, though DOOM
//     cannot handle sectors > 32767 units in height.
//
//  The code is not perfect across all situations. Some floor wiggle can
//   still be seen, and some texture strips may be slightly misaligned in
//   extreme cases. These effects cannot be corrected further, without
//   increasing the precision of various renderer variables, and,
//   possibly, creating a noticeable performance penalty.
//
static int  max_rwscale = 64 * FRACUNIT;
static int  heightbits = 12;
static int  heightunit = 1 << 12;
static int  invhgtbits = FRACBITS - 12;

static void R_FixWiggle(sector_t *sector)
{
    // disallow negative heights, force cache initialization
    int height = MAX(1, (sector->interpceilingheight - sector->interpfloorheight) >> FRACBITS);

    // initialize, or handle moving sector
    if (height != sector->cachedheight)
    {
        typedef struct
        {
            int clamp;
            int heightbits;
        } scalevalues_t;

        const scalevalues_t scalevalues[] =
        {
            { 2048 * FRACUNIT, 12 }, { 1024 * FRACUNIT, 12 }, { 1024 * FRACUNIT, 11 },
            {  512 * FRACUNIT, 11 }, {  512 * FRACUNIT, 10 }, {  256 * FRACUNIT, 10 },
            {  256 * FRACUNIT,  9 }, {  128 * FRACUNIT,  9 }, {   64 * FRACUNIT,  9 }
        };

        int                 scaleindex = 0;
        const scalevalues_t *scalevalue;

        sector->cachedheight = height;
        height >>= 7;

        // calculate adjustment
        while ((height >>= 1))
            scaleindex++;

        // fine-tune renderer for this wall
        scalevalue = &scalevalues[scaleindex];
        max_rwscale = scalevalue->clamp;
        heightbits = scalevalue->heightbits;
        heightunit = 1 << heightbits;
        invhgtbits = FRACBITS - heightbits;
    }
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
                dc_tint = (frontsector->tint ? colormaps[frontsector->tint] : fullcolormap);
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
    fixed_t         texheight;
    const rpatch_t  *patch;

    curline = ds->curline;
    frontsector = curline->frontsector;
    backsector = curline->backsector;
    texnum = texturetranslation[curline->sidedef->midtexture];
    texheight = textureheight[texnum];

    // Calculate light table.
    // Use different light tables for horizontal/vertical.
    // killough 04/13/98: get correct lightlevel for 2s normal textures
    if (fixedcolormap)
    {
        dc_colormap[0] = fixedcolormap;
        dc_nextcolormap[0] = fixedcolormap;
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
    }

    maskedtexturecol = ds->maskedtexturecol;
    rw_scalestep = ds->scalestep;
    spryscale = ds->scale + (x1 - ds->x1) * rw_scalestep;
    mceilingclip = ds->sprtopclip;
    mfloorclip = ds->sprbottomclip;

    // find positioning
    if (curline->linedef->flags & ML_DONTPEGBOTTOM)
        dc_texturemid = MAX(frontsector->interpfloorheight, backsector->interpfloorheight) + texheight - viewz
            + curline->sidedef->rowoffset;
    else
        dc_texturemid = MIN(frontsector->interpceilingheight, backsector->interpceilingheight) - viewz
            + curline->sidedef->rowoffset;

    patch = R_CacheTextureCompositePatchNum(texnum);

    // draw the columns
    for (dc_x = x1; dc_x <= x2; dc_x++, spryscale += rw_scalestep)
        if (maskedtexturecol[dc_x] != INT_MAX)
        {
            const rcolumn_t *column = R_GetPatchColumnWrapped(patch, maskedtexturecol[dc_x]);

            if ((dc_numposts = column->numposts))
            {
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
                if (t + (int64_t)texheight * spryscale < 0 || t > (int64_t)SCREENHEIGHT << FRACBITS * 2)
                    continue;

                sprtopscreen = (int64_t)(t >> FRACBITS);

                // calculate lighting
                if (!fixedcolormap)
                {
                    const int   index = MIN(spryscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1);

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

        // no space above wall?
        int         bottom;
        int         top = ceilingclip[rw_x] + 1;

        // mark floor/ceiling areas
        const int   yl = MAX((int)((topfrac + heightunit - 1) >> heightbits), top);
        int         yh = (int)(bottomfrac >> heightbits);

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
            // calculate texture offset and lighting
            const angle_t   angle = MIN((rw_centerangle + xtoviewangle[rw_x]) >> ANGLETOFINESHIFT, FINEANGLES / 2 - 1);

            texturecolumn = (rw_offset - FixedMul(finetangent[angle], rw_distance)) >> FRACBITS;

            if (!fixedcolormap)
            {
                const int   index = MIN(rw_scale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1);

                dc_colormap[0] = walllights[index];
                dc_nextcolormap[0] = walllightsnext[index];
                dc_z = ((rw_scale >> 5) & 255);
                dc_tint = (frontsector->tint ? colormaps[frontsector->tint] : fullcolormap);
            }

            dc_x = rw_x;
            dc_iscale = UINT_MAX / rw_scale;
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
                dc_source = R_GetTextureColumn(R_CacheTextureCompositePatchNum(midtexture), texturecolumn);
                dc_texturemid = rw_midtexturemid;
                dc_texheight = midtexheight;

                if (midbrightmap)
                {
                    dc_brightmap = midbrightmap;

                    if (r_ditheredlighting)
                    {
                        if (!memcmp(dc_colormap, dc_nextcolormap, sizeof(dc_colormap)))
                            altbmapwallcolfunc();
                        else
                            bmapwallcolfunc();
                    }
                    else
                        bmapwallcolfunc();
                }
                else if (r_ditheredlighting)
                {
                    if (!memcmp(dc_colormap, dc_nextcolormap, sizeof(dc_colormap)))
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
                const int   mid = MIN((int)(pixhigh >> heightbits), floorclip[rw_x] - 1);

                pixhigh += pixhighstep;

                if (mid >= yl)
                {
                    dc_yl = yl;
                    dc_yh = mid;

                    if (missingtoptexture)
                        missingcolfunc();
                    else
                    {
                        dc_source = R_GetTextureColumn(R_CacheTextureCompositePatchNum(toptexture), texturecolumn);
                        dc_texturemid = rw_toptexturemid + (dc_yl - centery + 1) * SPARKLEFIX;
                        dc_iscale -= SPARKLEFIX;
                        dc_texheight = toptexheight;

                        if (topbrightmap)
                        {
                            dc_brightmap = topbrightmap;

                            if (r_ditheredlighting)
                            {
                                if (!memcmp(dc_colormap, dc_nextcolormap, sizeof(dc_colormap)))
                                    altbmapwallcolfunc();
                                else
                                    bmapwallcolfunc();
                            }
                            else
                                bmapwallcolfunc();
                        }
                        else if (r_ditheredlighting)
                        {
                            if (!memcmp(dc_colormap, dc_nextcolormap, sizeof(dc_colormap)))
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
                const int   mid = MAX((int)((pixlow + heightunit - 1) >> heightbits), ceilingclip[rw_x] + 1);

                pixlow += pixlowstep;

                if (mid <= yh)
                {
                    dc_yl = mid;
                    dc_yh = yh;

                    if (missingbottomtexture)
                        missingcolfunc();
                    else
                    {
                        dc_source = R_GetTextureColumn(R_CacheTextureCompositePatchNum(bottomtexture), texturecolumn);
                        dc_texturemid = rw_bottomtexturemid;
                        dc_texheight = bottomtexheight;

                        if (bottombrightmap)
                        {
                            dc_brightmap = bottombrightmap;

                            if (r_ditheredlighting)
                            {
                                if (!memcmp(dc_colormap, dc_nextcolormap, sizeof(dc_colormap)))
                                    altbmapwallcolfunc();
                                else
                                    bmapwallcolfunc();
                            }
                            else
                                bmapwallcolfunc();
                        }
                        else if (r_ditheredlighting)
                        {
                            if (!memcmp(dc_colormap, dc_nextcolormap, sizeof(dc_colormap)))
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
    const int       angle = ANG90 + visangle;
    const int       den = FixedMul(rw_distance, finesine[angle >> ANGLETOFINESHIFT]);
    const fixed_t   num = FixedMul(projection, finesine[(angle + viewangle - rw_normalangle) >> ANGLETOFINESHIFT]);

    return (den > (num >> FRACBITS) ? BETWEEN(256, FixedDiv(num, den), max_rwscale) : max_rwscale);
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

    linedef = curline->linedef;
    flags = linedef->flags;

    // mark the segment as visible for automap
    if (!menuactive && !(flags & ML_MAPPED) && !(flags & ML_DONTDRAW) && (am_dynamic || !automapactive))
    {
        nummappedlines++;
        linedef->flags |= ML_MAPPED;
    }

    // [BH] if in automap, we're done now that line is mapped
    if (automapactive)
        return;

    sidedef = curline->sidedef;

    // killough 01/98 -- fix 2s line HOM
    if (ds_p == drawsegs + maxdrawsegs)
    {
        const size_t    pos = ds_p - drawsegs;

        maxdrawsegs = (maxdrawsegs ? 2 * maxdrawsegs : MAXDRAWSEGS);
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
    rw_distance = (fixed_t)((dy * dx1 - dx * dy1) / len) << 1;

    ds_p->x1 = start;
    rw_x = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    rw_stopx = stop + 1;

    if ((rw_stopx - start) * sizeof(*lastopening) + (lastopening - openings) > MAXOPENINGS)
        return;

    worldtop = frontsector->interpceilingheight - viewz;
    worldbottom = frontsector->interpfloorheight - viewz;

    // [BH] animate liquid sectors
    if (frontsector->terraintype >= LIQUID
        && (!frontsector->heightsec || viewz > frontsector->heightsec->interpfloorheight)
        && r_liquid_bob)
        worldbottom += animatedliquiddiff;

    R_FixWiggle(frontsector);

    // calculate scale at both ends and step
    rw_scale = R_ScaleFromGlobalAngle(xtoviewangle[start]);
    ds_p->scale = rw_scale;

    if (stop > start)
    {
        const fixed_t   scale = R_ScaleFromGlobalAngle(xtoviewangle[stop]);

        rw_scalestep = (scale - rw_scale) / (stop - start);
        ds_p->scalestep = rw_scalestep;

        if (rw_scale > scale)
        {
            ds_p->minscale = scale;
            ds_p->maxscale = rw_scale;
        }
        else
        {
            ds_p->minscale = rw_scale;
            ds_p->maxscale = scale;
        }
    }
    else
    {
        ds_p->scalestep = 0;
        rw_scalestep = 0;
        ds_p->minscale = rw_scale;
        ds_p->maxscale = rw_scale;
    }

    // calculate texture boundaries and decide if floor/ceiling marks are needed
    midtexture = 0;
    toptexture = 0;
    bottomtexture = 0;
    maskedtexture = false;
    ds_p->maskedtexturecol = NULL;

    if (!backsector)
    {
        // single sided line
        if ((missingmidtexture = sidedef->missingmidtexture))
            midtexture = -1;
        else
        {
            fixed_t height;

            midtexture = texturetranslation[sidedef->midtexture];
            height = textureheight[midtexture];
            midtexheight = ((linedef->r_flags & RF_MID_TILE) ? 0 : (height >> FRACBITS));
            midbrightmap = (usebrightmaps && !nobrightmap[midtexture] ? brightmap[midtexture] : NULL);
            rw_midtexturemid = ((linedef->flags & ML_DONTPEGBOTTOM) ? frontsector->interpfloorheight + height - viewz : worldtop)
                + FixedMod(sidedef->rowoffset, height);
        }

        // a single sided line is terminal, so it must mark ends
        markfloor = true;
        markceiling = true;

        ds_p->sprtopclip = viewheightarray;
        ds_p->sprbottomclip = negonearray;
        ds_p->silhouette = SIL_BOTH;
    }
    else
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

            if (frontsector->interpfloorheight > backsector->interpfloorheight || backsector->interpfloorheight > viewz)
                ds_p->silhouette = SIL_BOTTOM;

            if (frontsector->interpceilingheight < backsector->interpceilingheight || backsector->interpceilingheight < viewz)
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
            && (!backsector->heightsec || viewz > backsector->heightsec->interpfloorheight)
            && !sidedef->midtexture
            && r_liquid_bob)
        {
            liquidoffset = animatedliquiddiff;
            worldlow += liquidoffset;
        }

        markfloor = (worldlow != worldbottom
            || backsector->floorpic != frontsector->floorpic
            || backsector->lightlevel != frontsector->lightlevel

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
            else
            {
                fixed_t height;

                toptexture = texturetranslation[sidedef->toptexture];
                height = textureheight[toptexture];
                toptexheight = ((linedef->r_flags & RF_TOP_TILE) ? 0 : (height >> FRACBITS));
                topbrightmap = (usebrightmaps && !nobrightmap[toptexture] ? brightmap[toptexture] : NULL);
                rw_toptexturemid = ((linedef->flags & ML_DONTPEGTOP) ? worldtop : backsector->interpceilingheight + height - viewz)
                    + FixedMod(sidedef->rowoffset, height);
            }
        }

        if (worldlow > worldbottom && frontsector->interpfloorheight != backsector->interpfloorheight)
        {
            // bottom texture
            if ((missingbottomtexture = sidedef->missingbottomtexture))
                bottomtexture = -1;
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
        if (sidedef->midtexture)
        {
            // masked midtexture
            maskedtexture = true;
            ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
            lastopening += (size_t)rw_stopx - rw_x;
        }
    }

    // calculate rw_offset (only needed for textured lines)
    if ((segtextured = (midtexture || toptexture || bottomtexture || maskedtexture)))
    {
        rw_offset = (fixed_t)(((dx * dx1 + dy * dy1) / len) << 1) + sidedef->textureoffset + curline->offset;
        rw_centerangle = ANG90 + viewangle - rw_normalangle;

        // calculate light table
        //  use different light tables for horizontal/vertical
        if (!fixedcolormap)
        {
            const short lightlevel = frontsector->lightlevel;

            walllights = GetLightTable(lightlevel);
            walllightsnext = GetLightTable(lightlevel + 4);
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

    // calculate incremental stepping values for texture edges
    topstep = -FixedMul(rw_scalestep, (worldtop >>= invhgtbits));
    topfrac = ((int64_t)centeryfrac >> invhgtbits) - (((int64_t)worldtop * rw_scale) >> FRACBITS);

    bottomstep = -FixedMul(rw_scalestep, (worldbottom >>= invhgtbits));
    bottomfrac = ((int64_t)centeryfrac >> invhgtbits) - (((int64_t)worldbottom * rw_scale) >> FRACBITS);

    if (backsector)
    {
        if ((worldhigh >>= invhgtbits) < worldtop)
        {
            pixhigh = ((int64_t)centeryfrac >> invhgtbits) - (((int64_t)worldhigh * rw_scale) >> FRACBITS);
            pixhighstep = -FixedMul(rw_scalestep, worldhigh);
        }

        if ((worldlow >>= invhgtbits) > worldbottom)
        {
            pixlow = ((int64_t)centeryfrac >> invhgtbits) - (((int64_t)worldlow * rw_scale) >> FRACBITS);
            pixlowstep = -FixedMul(rw_scalestep, worldlow);
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

    if ((backsector && didsolidcol) || maskedtexture)
        ds_p->silhouette = SIL_BOTH;

    // save sprite clipping info
    if ((ds_p->silhouette & SIL_TOP) && !ds_p->sprtopclip)
    {
        memcpy(lastopening, ceilingclip + start, sizeof(*lastopening) * ((size_t)rw_stopx - start));
        ds_p->sprtopclip = lastopening - start;
        lastopening += (size_t)rw_stopx - start;
    }

    if ((ds_p->silhouette & SIL_BOTTOM) && !ds_p->sprbottomclip)
    {
        memcpy(lastopening, floorclip + start, sizeof(*lastopening) * ((size_t)rw_stopx - start));
        ds_p->sprbottomclip = lastopening - start;
        lastopening += (size_t)rw_stopx - start;
    }

    ds_p++;
}
