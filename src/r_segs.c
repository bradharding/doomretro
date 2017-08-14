/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "doomstat.h"
#include "p_local.h"
#include "z_zone.h"

static unsigned int maxdrawsegs;

static dboolean     segtextured;        // True if any of the segs textures might be visible.

static side_t       *sidedef;

static dboolean     markfloor;          // False if the back side is the same plane.
dboolean            markceiling;

static dboolean     maskedtexture;
static int          toptexture;
static int          midtexture;
static int          bottomtexture;

static fixed_t      toptexheight;
static fixed_t      midtexheight;
static fixed_t      bottomtexheight;

static byte         *toptexfullbright;
static byte         *midtexfullbright;
static byte         *bottomtexfullbright;

angle_t             rw_normalangle;
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

lighttable_t        **walllights;

static int          *maskedtexturecol;  // dropoff overflow

dboolean            r_brightmaps = r_brightmaps_default;
dboolean            r_liquid_current = r_liquid_current_default;

extern int          *openings;          // dropoff overflow
extern fixed_t      animatedliquiddiff;
extern fixed_t      animatedliquidxoffs;
extern fixed_t      animatedliquidyoffs;
extern dboolean     r_dither;
extern dboolean     r_liquid_bob;
extern dboolean     r_textures;
extern dboolean     r_translucency;
extern dboolean     usebrightmaps;

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
//   from becoming larger than 0x400000 (64*FRACUNIT). This number was
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
static int  heightunit = (1 << 12);
static int  invhgtbits = 4;

static void R_FixWiggle(sector_t *sector)
{
    typedef struct
    {
        int clamp;
        int heightbits;
    } scalevalues_t;

    static const scalevalues_t scale_values[] =
    {
        { 2048 * FRACUNIT, 12 }, { 1024 * FRACUNIT, 12 }, { 1024 * FRACUNIT, 11 },
        {  512 * FRACUNIT, 11 }, {  512 * FRACUNIT, 10 }, {  256 * FRACUNIT, 10 },
        {  256 * FRACUNIT,  9 }, {  128 * FRACUNIT,  9 }, {   64 * FRACUNIT,  9 }
    };

    static int  lastheight;

    // disallow negative heights, force cache initialization
    int         height = MAX(1, (sector->interpceilingheight - sector->interpfloorheight) >> FRACBITS);

    // early out?
    if (height != lastheight)
    {
        const scalevalues_t *svp;

        lastheight = height;

        // initialize, or handle moving sector
        if (height != sector->cachedheight)
        {
            int scaleindex = 0;

            sector->cachedheight = height;
            height >>= 7;

            // calculate adjustment
            while ((height >>= 1))
                scaleindex++;

            sector->scaleindex = scaleindex;
        }

        // fine-tune renderer for this wall
        svp = &scale_values[sector->scaleindex];
        max_rwscale = svp->clamp;
        heightbits = svp->heightbits;
        heightunit = 1 << heightbits;
        invhgtbits = FRACBITS - heightbits;
    }
}

static lighttable_t **GetLightTable(const int lightlevel)
{
    return scalelight[BETWEEN(0, (lightlevel >> LIGHTSEGSHIFT) + extralight + curline->fakecontrast,
        LIGHTLEVELS - 1)];
}

static void R_BlastMaskedSegColumn(const rcolumn_t *column)
{
    int             count = column->numposts;
    unsigned char   *pixels = column->pixels;

    dc_ceilingclip = mceilingclip[dc_x] + 1;
    dc_floorclip = mfloorclip[dc_x] - 1;

    while (count--)
    {
        const rpost_t   *post = &column->posts[count];
        const int       topdelta = post->topdelta;

        // calculate unclipped screen coordinates for post
        const int64_t   topscreen = sprtopscreen + spryscale * topdelta + 1;

        if ((dc_yh = MIN((int)((topscreen + spryscale * post->length) >> FRACBITS), dc_floorclip)) >= 0)
            if ((dc_yl = MAX(dc_ceilingclip, (int)((topscreen + FRACUNIT) >> FRACBITS))) <= dc_yh)
            {
                dc_texturefrac = dc_texturemid - (topdelta << FRACBITS)
                    + FixedMul((dc_yl - centery) << FRACBITS, dc_iscale);
                dc_source = pixels + topdelta;
                colfunc();
            }
    }
}

//
// R_RenderMaskedSegRange
//
void R_RenderMaskedSegRange(drawseg_t *ds, const int x1, const int x2)
{
    int             texnum;
    fixed_t         texheight;
    const rpatch_t  *patch;
    sector_t        tempsec;        // killough 4/13/98

    // Calculate light table.
    // Use different light tables for horizontal / vertical.
    curline = ds->curline;

    if (r_textures)
        colfunc = (curline->linedef->tranlump >= 0 && r_translucency ?
            (r_dither ? R_DrawDitheredColumn : R_DrawTranslucent50Column) : R_DrawColumn);
    else
        colfunc = R_DrawColorColumn;

    frontsector = curline->frontsector;
    backsector = curline->backsector;
    texnum = texturetranslation[curline->sidedef->midtexture];
    texheight = textureheight[texnum];

    // killough 4/13/98: get correct lightlevel for 2s normal textures
    walllights = GetLightTable(R_FakeFlat(frontsector, &tempsec, NULL, NULL, false)->lightlevel);

    maskedtexturecol = ds->maskedtexturecol;

    rw_scalestep = ds->scalestep;
    spryscale = ds->scale1 + (x1 - ds->x1) * rw_scalestep;
    mfloorclip = ds->sprbottomclip;
    mceilingclip = ds->sprtopclip;

    // find positioning
    if (curline->linedef->flags & ML_DONTPEGBOTTOM)
        dc_texturemid = MAX(frontsector->interpfloorheight, backsector->interpfloorheight) + texheight
            - viewz + curline->sidedef->rowoffset;
    else
        dc_texturemid = MIN(frontsector->interpceilingheight, backsector->interpceilingheight) - viewz
            + curline->sidedef->rowoffset;

    dc_colormap = fixedcolormap;

    patch = R_CacheTextureCompositePatchNum(texnum);

    // draw the columns
    for (dc_x = x1; dc_x <= x2; dc_x++, spryscale += rw_scalestep)
        if (maskedtexturecol[dc_x] != INT_MAX)
        {
            // killough 3/2/98:
            //
            // This calculation used to overflow and cause crashes in DOOM:
            //
            // sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);
            //
            // This code fixes it, by using double-precision intermediate
            // arithmetic and by skipping the drawing of 2s normals whose
            // mapping to screen coordinates is totally out of range:
            int64_t t = ((int64_t)centeryfrac << FRACBITS) - (int64_t)dc_texturemid * spryscale;

            if (t + (int64_t)texheight * spryscale < 0 || t > (int64_t)SCREENHEIGHT << FRACBITS * 2)
                continue;                       // skip if the texture is out of screen's range

            sprtopscreen = (int64_t)(t >> FRACBITS);

            // calculate lighting
            if (!fixedcolormap)
                dc_colormap = walllights[BETWEEN(0, spryscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)];

            dc_iscale = 0xFFFFFFFFu / (unsigned int)spryscale;

            // draw the texture
            R_BlastMaskedSegColumn(R_GetPatchColumnWrapped(patch, maskedtexturecol[dc_x]));
            maskedtexturecol[dc_x] = INT_MAX;   // dropoff overflow
        }

    R_UnlockTextureCompositePatchNum(texnum);
}

//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling textures.
// CALLED: CORE LOOPING ROUTINE.
//
static dboolean didsolidcol;

static void R_RenderSegLoop(void)
{
    for (; rw_x < rw_stopx; rw_x++)
    {
        fixed_t texturecolumn = 0;

        // no space above wall?
        int     bottom;
        int     top = ceilingclip[rw_x] + 1;

        // mark floor / ceiling areas
        int     yl = MAX((int)((topfrac + heightunit - 1) >> heightbits), top);
        int     yh = (int)(bottomfrac >> heightbits);

        if (markceiling)
        {
            if (top <= (bottom = MIN(yl, floorclip[rw_x]) - 1))
            {
                ceilingplane->top[rw_x] = top;
                ceilingplane->bottom[rw_x] = bottom;
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
            }

            floorclip[rw_x] = top;
        }

        // texturecolumn and lighting are independent of wall tiers
        if (segtextured)
        {
            // calculate texture offset and lighting
            angle_t angle = MIN((rw_centerangle + xtoviewangle[rw_x]) >> ANGLETOFINESHIFT,
                        FINEANGLES / 2 - 1);

            texturecolumn = (rw_offset - FixedMul(finetangent[angle], rw_distance)) >> FRACBITS;

            if (fixedcolormap)
                dc_colormap = fixedcolormap;
            else
                dc_colormap = walllights[BETWEEN(0, rw_scale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)];

            dc_x = rw_x;
            dc_iscale = 0xFFFFFFFFu / (unsigned int)rw_scale;
        }

        // draw the wall tiers
        if (midtexture && yh >= yl)
        {
            // single sided line
            dc_yl = yl;
            dc_yh = yh;
            dc_texturemid = rw_midtexturemid;
            dc_source = R_GetTextureColumn(R_CacheTextureCompositePatchNum(midtexture), texturecolumn);
            dc_texheight = midtexheight;
            dc_sparklefix = SPARKLEFIX;

            // [BH] apply brightmap
            if (midtexfullbright)
            {
                dc_colormask = midtexfullbright;
                fbwallcolfunc();
            }
            else
                wallcolfunc();

            R_UnlockTextureCompositePatchNum(midtexture);
            ceilingclip[rw_x] = viewheight;
            floorclip[rw_x] = -1;
        }
        else
        {
            // two sided line
            if (toptexture)
            {
                // top wall
                int mid = MIN((int)(pixhigh >> heightbits), floorclip[rw_x] - 1);

                pixhigh += pixhighstep;

                if (mid >= yl)
                {
                    dc_yl = yl;
                    dc_yh = mid;
                    dc_texturemid = rw_toptexturemid;
                    dc_source = R_GetTextureColumn(R_CacheTextureCompositePatchNum(toptexture),
                        texturecolumn);
                    dc_texheight = toptexheight;
                    dc_sparklefix = SPARKLEFIX;

                    // [BH] apply brightmap
                    if (toptexfullbright)
                    {
                        dc_colormask = toptexfullbright;
                        fbwallcolfunc();
                    }
                    else
                        wallcolfunc();

                    R_UnlockTextureCompositePatchNum(toptexture);
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
                int mid = MAX((int)((pixlow + heightunit - 1) >> heightbits), ceilingclip[rw_x] + 1);

                pixlow += pixlowstep;

                if (mid <= yh)
                {
                    dc_yl = mid;
                    dc_yh = yh;
                    dc_texturemid = rw_bottomtexturemid;
                    dc_source = R_GetTextureColumn(R_CacheTextureCompositePatchNum(bottomtexture),
                        texturecolumn);
                    dc_texheight = bottomtexheight;
                    dc_sparklefix = SPARKLEFIX;

                    // [BH] apply brightmap
                    if (bottomtexfullbright)
                    {
                        dc_colormask = bottomtexfullbright;
                        fbwallcolfunc();
                    }
                    else
                        wallcolfunc();

                    R_UnlockTextureCompositePatchNum(bottomtexture);
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

            // save texturecol for backdrawing of masked mid texture
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
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
static fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
    const int       angle = ANG90 + visangle;
    const int       den = FixedMul(rw_distance, finesine[(angle - viewangle) >> ANGLETOFINESHIFT]);
    const fixed_t   num = FixedMul(projectiony, finesine[(angle - rw_normalangle) >> ANGLETOFINESHIFT]);

    return (den > (num >> FRACBITS) ? BETWEEN(256, FixedDiv(num, den), max_rwscale) : max_rwscale);
}

//
// R_StoreWallRange
// A wall segment will be drawn
//  between start and stop pixels (inclusive).
//
void R_StoreWallRange(const int start, const int stop)
{
    int64_t  dx, dy;
    int64_t  dx1, dy1;
    int64_t  len;
    int      worldtop;
    int      worldbottom;
    int      worldhigh;
    int      worldlow;
    
    linedef = curline->linedef;

    // mark the segment as visible for automap
    linedef->flags |= ML_MAPPED;

    // [BH] if in automap, we're done now that line is mapped
    if (automapactive)
        return;

    sidedef = curline->sidedef;

    // killough 1/98 -- fix 2s line HOM
    if (ds_p == drawsegs + maxdrawsegs)
    {
        const unsigned int  pos = ds_p - drawsegs;
        const unsigned int  newmax = (maxdrawsegs ? 2 * maxdrawsegs : MAXDRAWSEGS);

        drawsegs = Z_Realloc(drawsegs, newmax * sizeof(*drawsegs));
        ds_p = drawsegs + pos;
        maxdrawsegs = newmax;
    }

    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle + ANG90;

    // shift right to avoid possibility of int64 overflow in rw_distance calculation
    dx = ((int64_t)curline->v2->x - curline->v1->x) >> 1;
    dy = ((int64_t)curline->v2->y - curline->v1->y) >> 1;
    dx1 = ((int64_t)viewx - curline->v1->x) >> 1;
    dy1 = ((int64_t)viewy - curline->v1->y) >> 1;
    len = curline->length >> 1;
    rw_distance = (fixed_t)((dy * dx1 - dx * dy1) / len) << 1;

    ds_p->x1 = start;
    rw_x = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    rw_stopx = stop + 1;

    // killough 1/6/98, 2/1/98: remove limit on openings
    {
        size_t          pos = lastopening - openings;
        size_t          need = (rw_stopx - start) * sizeof(*lastopening) + pos;
        static size_t   maxopenings;

        if (need > maxopenings)
        {
            drawseg_t   *ds;                    // jff 8/9/98 needed for fix from ZDoom
            const int   *oldopenings = openings;
            const int   *oldlast = lastopening;

            do
                maxopenings = (maxopenings ? maxopenings * 2 : MAXOPENINGS);
            while (need > maxopenings);

            openings = Z_Realloc(openings, maxopenings * sizeof(*openings));
            lastopening = openings + pos;

            // jff 8/9/98 borrowed fix for openings from ZDOOM1.14
            // [RH] We also need to adjust the openings pointers that
            //    were already stored in drawsegs.
            for (ds = drawsegs; ds < ds_p; ds++)
            {
                if (ds->maskedtexturecol + ds->x1 >= oldopenings && ds->maskedtexturecol + ds->x1 <= oldlast)
                    ds->maskedtexturecol = ds->maskedtexturecol - oldopenings + openings;

                if (ds->sprtopclip + ds->x1 >= oldopenings && ds->sprtopclip + ds->x1 <= oldlast)
                    ds->sprtopclip = ds->sprtopclip - oldopenings + openings;

                if (ds->sprbottomclip + ds->x1 >= oldopenings && ds->sprbottomclip + ds->x1 <= oldlast)
                    ds->sprbottomclip = ds->sprbottomclip - oldopenings + openings;
            }
        }
    }

    worldtop = frontsector->interpceilingheight - viewz;
    worldbottom = frontsector->interpfloorheight - viewz;

    // [BH] animate liquid sectors
    if (frontsector->isliquid && !freeze)
    {
        if (r_liquid_bob && (frontsector->heightsec == -1
            || viewz > sectors[frontsector->heightsec].interpfloorheight))
            worldbottom += animatedliquiddiff;

        if (r_liquid_current && frontsector->heightsec == -1)
        {
            frontsector->floor_xoffs = animatedliquidxoffs;
            frontsector->floor_yoffs = animatedliquidyoffs;
        }
    }

    R_FixWiggle(frontsector);

    // calculate scale at both ends and step
    ds_p->scale1 = rw_scale = R_ScaleFromGlobalAngle(viewangle + xtoviewangle[start]);

    if (stop > start)
    {
        ds_p->scale2 = R_ScaleFromGlobalAngle(viewangle + xtoviewangle[stop]);
        ds_p->scalestep = rw_scalestep = (ds_p->scale2 - rw_scale) / (stop - start);
    }
    else
        ds_p->scale2 = ds_p->scale1;

    // calculate texture boundaries
    //  and decide if floor / ceiling marks are needed
    midtexture = 0;
    toptexture = 0;
    bottomtexture = 0;
    maskedtexture = 0;
    ds_p->maskedtexturecol = NULL;

    if (!backsector)
    {
        // single sided line
        midtexture = texturetranslation[sidedef->midtexture];
        midtexheight = ((linedef->r_flags & RF_MID_TILE) ? 0 : textureheight[midtexture] >> FRACBITS);
        midtexfullbright = (usebrightmaps && !nobrightmap[midtexture] ? texturefullbright[midtexture] :
            NULL);
        rw_midtexturemid = ((linedef->flags & ML_DONTPEGBOTTOM) ? frontsector->interpfloorheight
            + textureheight[midtexture] - viewz : worldtop);
        rw_midtexturemid += FixedMod(sidedef->rowoffset, textureheight[midtexture]);

        // a single sided line is terminal, so it must mark ends
        markfloor = true;
        markceiling = true;

        ds_p->sprtopclip = screenheightarray;
        ds_p->sprbottomclip = negonearray;
        ds_p->silhouette = SIL_BOTH;
    }
    else
    {
        int liquidoffset = 0;

        if (linedef->r_flags & RF_CLOSED)
        {
            ds_p->sprbottomclip = negonearray;
            ds_p->sprtopclip = screenheightarray;
            ds_p->silhouette = SIL_BOTH;
        }
        else
        {
            // two sided line
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

        markfloor = (worldlow != worldbottom
            || backsector->floorpic != frontsector->floorpic
            || backsector->lightlevel != frontsector->lightlevel

            // killough 3/7/98: Add checks for (x,y) offsets
            || backsector->floor_xoffs != frontsector->floor_xoffs
            || backsector->floor_yoffs != frontsector->floor_yoffs

            // killough 4/15/98: prevent 2s normals
            // from bleeding through deep water
            || frontsector->heightsec != -1

            // killough 4/17/98: draw floors if different light levels
            || backsector->floorlightsec != frontsector->floorlightsec);

        markceiling = (worldhigh != worldtop
            || backsector->ceilingpic != frontsector->ceilingpic
            || backsector->lightlevel != frontsector->lightlevel

            // killough 3/7/98: Add checks for (x,y) offsets
            || backsector->ceiling_xoffs != frontsector->ceiling_xoffs
            || backsector->ceiling_yoffs != frontsector->ceiling_yoffs

            // killough 4/15/98: prevent 2s normals
            // from bleeding through fake ceilings
            || (frontsector->heightsec != -1 && frontsector->ceilingpic != skyflatnum)

            // killough 4/17/98: draw ceilings if different light levels
            || backsector->ceilinglightsec != frontsector->ceilinglightsec);

        if (backsector->interpceilingheight <= frontsector->interpfloorheight
            || backsector->interpfloorheight >= frontsector->interpceilingheight)
        {
            // closed door
            markceiling = true;
            markfloor = true;
        }

        // [BH] animate liquid sectors
        if (r_liquid_bob && backsector->isliquid && !freeze
            && backsector->interpfloorheight >= frontsector->interpfloorheight
            && (backsector->heightsec == -1 || viewz > sectors[backsector->heightsec].interpfloorheight))
        {
            liquidoffset = animatedliquiddiff;
            worldlow += liquidoffset;
        }

        if (worldhigh < worldtop)
        {
            // top texture
            toptexture = texturetranslation[sidedef->toptexture];
            toptexheight = ((linedef->r_flags & RF_TOP_TILE) ? 0 : textureheight[toptexture] >> FRACBITS);
            toptexfullbright = (usebrightmaps && !nobrightmap[toptexture] ? texturefullbright[toptexture] :
                NULL);
            rw_toptexturemid = ((linedef->flags & ML_DONTPEGTOP) ? worldtop :
                backsector->interpceilingheight + textureheight[toptexture] - viewz);
            rw_toptexturemid += FixedMod(sidedef->rowoffset, textureheight[toptexture]);
        }

        if (worldlow > worldbottom)
        {
            // bottom texture
            bottomtexture = texturetranslation[sidedef->bottomtexture];
            bottomtexheight = ((linedef->r_flags & RF_BOT_TILE) ? 0 :
                textureheight[bottomtexture] >> FRACBITS);
            bottomtexfullbright = (usebrightmaps && !nobrightmap[bottomtexture] ?
                texturefullbright[bottomtexture] : NULL);
            rw_bottomtexturemid = ((linedef->flags & ML_DONTPEGBOTTOM) ? worldtop : worldlow - liquidoffset);
            rw_bottomtexturemid += FixedMod(sidedef->rowoffset, textureheight[bottomtexture]);
        }

        // allocate space for masked texture tables
        if (sidedef->midtexture)
        {
            // masked midtexture
            maskedtexture = true;
            ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
            lastopening += rw_stopx - rw_x;
        }
    }

    // calculate rw_offset (only needed for textured lines)
    if ((segtextured = (midtexture | toptexture | bottomtexture | maskedtexture)))
    {
        rw_offset = (fixed_t)(((dx * dx1 + dy * dy1) / len) << 1) + sidedef->textureoffset + curline->offset;
        rw_centerangle = ANG90 + viewangle - rw_normalangle;

        // calculate light table
        //  use different light tables
        //  for horizontal / vertical / diagonal
        if (!fixedcolormap)
            walllights = GetLightTable(frontsector->lightlevel);
    }

    // if a floor / ceiling plane is on the wrong side
    //  of the view plane, it is definitely invisible
    //  and doesn't need to be marked.

    // killough 3/7/98: add deep water check
    if (frontsector->heightsec == -1)
    {
        if (frontsector->interpfloorheight >= viewz)
            markfloor = false;          // above view plane

        if (frontsector->interpceilingheight <= viewz && frontsector->ceilingpic != skyflatnum)
            markceiling = false;        // below view plane
    }

    // calculate incremental stepping values for texture edges
    worldtop >>= invhgtbits;
    worldbottom >>= invhgtbits;

    topstep = -FixedMul(rw_scalestep, worldtop);
    topfrac = ((int64_t)centeryfrac >> invhgtbits) - (((int64_t)worldtop * rw_scale) >> FRACBITS);

    bottomstep = -FixedMul(rw_scalestep, worldbottom);
    bottomfrac = ((int64_t)centeryfrac >> invhgtbits) - (((int64_t)worldbottom * rw_scale) >> FRACBITS);

    if (backsector)
    {
        worldhigh >>= invhgtbits;
        worldlow >>= invhgtbits;

        if (worldhigh < worldtop)
        {
            pixhigh = ((int64_t)centeryfrac >> invhgtbits) - (((int64_t)worldhigh * rw_scale) >> FRACBITS);
            pixhighstep = -FixedMul(rw_scalestep, worldhigh);
        }

        if (worldlow > worldbottom)
        {
            pixlow = ((int64_t)centeryfrac >> invhgtbits) - (((int64_t)worldlow * rw_scale) >> FRACBITS);
            pixlowstep = -FixedMul(rw_scalestep, worldlow);
        }
    }

    // render it
    if (markceiling)
    {
        if (ceilingplane)   // killough 4/11/98: add NULL ptr checks
            ceilingplane = R_CheckPlane(ceilingplane, rw_x, rw_stopx - 1);
        else
            markceiling = false;
    }

    if (markfloor)
    {
        if (floorplane)     // killough 4/11/98: add NULL ptr checks
            floorplane = R_CheckPlane(floorplane, rw_x, rw_stopx - 1);
        else
            markfloor = false;
    }

    didsolidcol = false;
    R_RenderSegLoop();

    if (backsector && didsolidcol)
    {
        ds_p->silhouette |= SIL_TOP;
        ds_p->silhouette |= SIL_BOTTOM;
    }

    // save sprite clipping info
    if (((ds_p->silhouette & SIL_TOP) || maskedtexture) && !ds_p->sprtopclip)
    {
        memcpy(lastopening, ceilingclip + start, sizeof(*lastopening) * (rw_stopx - start));
        ds_p->sprtopclip = lastopening - start;
        lastopening += rw_stopx - start;
    }

    if (((ds_p->silhouette & SIL_BOTTOM) || maskedtexture) && !ds_p->sprbottomclip)
    {
        memcpy(lastopening, floorclip + start, sizeof(*lastopening) * (rw_stopx - start));
        ds_p->sprbottomclip = lastopening - start;
        lastopening += rw_stopx - start;
    }

    if (maskedtexture)
    {
        ds_p->silhouette |= SIL_TOP;
        ds_p->silhouette |= SIL_BOTTOM;
    }

    ds_p++;
}
