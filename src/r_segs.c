/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#include <stdlib.h>

#include "doomstat.h"
#include "m_config.h"
#include "r_local.h"

// killough 1/6/98: replaced globals with statics where appropriate
static boolean  segtextured;    // True if any of the segs textures might be visible.
static boolean  markfloor;      // False if the back side is the same plane.
static boolean  markceiling;
static boolean  maskedtexture;
static int      toptexture;
static int      bottomtexture;
static int      midtexture;

static fixed_t  toptexheight;
static fixed_t  midtexheight;
static fixed_t  bottomtexheight;

angle_t         rw_normalangle; // angle to line origin
int             rw_angle1;
fixed_t         rw_distance;
lighttable_t    **walllights;

//
// regular wall
//
static int      rw_x;
static int      rw_stopx;
static angle_t  rw_centerangle;
static fixed_t  rw_offset;
static fixed_t  rw_scale;
static fixed_t  rw_scalestep;
static fixed_t  rw_midtexturemid;
static fixed_t  rw_toptexturemid;
static fixed_t  rw_bottomtexturemid;
static int      worldtop;
static int      worldbottom;
static int      worldhigh;
static int      worldlow;
static fixed_t  pixhigh;
static fixed_t  pixlow;
static fixed_t  pixhighstep;
static fixed_t  pixlowstep;
static fixed_t  topfrac;
static fixed_t  topstep;
static fixed_t  bottomfrac;
static fixed_t  bottomstep;
static int      *maskedtexturecol;

boolean         brightmaps = BRIGHTMAPS_DEFAULT;

// [kb] hack to improve rendering precision (wall wiggle)
static int      max_rwscale = 64 * FRACUNIT;
static int      HEIGHTBITS = 12;
static int      HEIGHTUNIT = (1 << 12);
static int      invhgtbits = 4;

// [kb] Adjusts renderer wall/texture precision based on the maximum difference in height
// of all adjoining sectors. P_SetupWiggleFix() passes max_diff, which is what is needed
// to make the renderer as precise as possible without overflowing the 16.16 fixed point
// coordinate system. As a bonus, this also allows the render to display sectors that are
// up to 32767 units tall (or greater, maybe), improving an old bug. Doom doesn't allow
// anything to pass through a sector any taller than 32767 units, so this limit is ok.
// Of course, levels with sectors this large WILL suffer from some wall wiggle...
// e6y - rewritten kb1's original code
void R_SetWiggleHack(int height)
{
    static const int MultiplyDeBruijnBitPosition[16] = {
        0, 6, 1, 12, 7, 9, 2, 13, 5, 11, 8, 4, 10, 3, 15, 14
    };

    static const int pairs[16][2] = {
        { 2048, 12 }, { 2048, 12 }, { 2048, 12 }, { 2048, 12 },
        { 2048, 12 }, { 2048, 12 }, { 2048, 12 }, { 2048, 12 },
        { 2048, 11 }, { 2048, 10 }, { 2048,  9 }, { 1024,  9 },
        {  512,  9 }, {  256,  9 }, {  128,  9 }, {   64,  9 }
    };

    int n;

    height |= height >> 1;
    height |= height >> 2;
    height |= height >> 4;
    height |= height >> 8;

    n = MultiplyDeBruijnBitPosition[(unsigned int)(height * 0xe59fcb4u) >> 28];

    max_rwscale = pairs[n][0] << FRACBITS;
    HEIGHTBITS = pairs[n][1];
    HEIGHTUNIT = (1 << HEIGHTBITS);
    invhgtbits = 16 - HEIGHTBITS;
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
    int         anglea = ANG90 + visangle - viewangle;
    int         angleb = ANG90 + visangle - rw_normalangle;
    int         den = FixedMul(rw_distance, finesine[anglea >> ANGLETOFINESHIFT]);
    fixed_t     num = FixedMul(projectiony, finesine[angleb >> ANGLETOFINESHIFT]);

    return (den > (num >> 16) ? ((num = FixedDiv(num, den)) > max_rwscale ? max_rwscale :
        MAX(256, num)) : max_rwscale);
}

//
// R_RenderMaskedSegRange
//
void R_RenderMaskedSegRange(drawseg_t *ds, int x1, int x2)
{
    column_t    *col;
    int         lightnum;
    int         texnum;
    fixed_t     texheight;

    // Calculate light table.
    // Use different light tables for horizontal / vertical.
    curline = ds->curline;
    frontsector = curline->frontsector;
    backsector = curline->backsector;
    texnum = texturetranslation[curline->sidedef->midtexture];
    texheight = textureheight[texnum];

    lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT) + extralight * LIGHTBRIGHT;
    if (frontsector->ceilingpic != skyflatnum)
    {
        if (curline->v1->y == curline->v2->y)
            lightnum -= LIGHTBRIGHT;
        else if (curline->v1->x == curline->v2->x)
            lightnum += LIGHTBRIGHT;
    }

    walllights = scalelight[BETWEEN(0, lightnum, LIGHTLEVELS - 1)];

    maskedtexturecol = ds->maskedtexturecol;

    rw_scalestep = ds->scalestep;
    spryscale = ds->scale1 + (x1 - ds->x1) * rw_scalestep;
    mfloorclip = ds->sprbottomclip;
    mceilingclip = ds->sprtopclip;

    // find positioning
    if (curline->linedef->flags & ML_DONTPEGBOTTOM)
        dc_texturemid = MAX(frontsector->floorheight, backsector->floorheight) +
            texheight - viewz + curline->sidedef->rowoffset;
    else
        dc_texturemid = MIN(frontsector->ceilingheight, backsector->ceilingheight) -
            viewz + curline->sidedef->rowoffset;

    dc_texheight = 0;

    if (fixedcolormap)
        dc_colormap = fixedcolormap;

    // draw the columns
    for (dc_x = x1; dc_x <= x2; ++dc_x, spryscale += rw_scalestep)
    {
        // calculate lighting
        if (maskedtexturecol[dc_x] != INT_MAX)
        {
            int64_t     t = ((int64_t)centeryfrac << FRACBITS) - (int64_t)dc_texturemid * spryscale;

            if (t + (int64_t)texheight * spryscale < 0 || t > (int64_t)SCREENHEIGHT << FRACBITS * 2)
                continue;        // skip if the texture is out of screen's range

            if (!fixedcolormap)
                dc_colormap = walllights[BETWEEN(0, spryscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)];

            sprtopscreen = (long)(t >> FRACBITS);
            dc_iscale = 0xffffffffu / (unsigned int)spryscale;

            // draw the texture
            col = (column_t *)((byte *)R_GetColumn(texnum, maskedtexturecol[dc_x]) - 3);

            R_DrawMaskedColumn(col, -1);
            maskedtexturecol[dc_x] = INT_MAX;
        }
    }
}

//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked
//  texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling
//  textures.
// CALLED: CORE LOOPING ROUTINE.
//
void R_RenderSegLoop(void)
{
    fixed_t     texturecolumn = 0;

    for (; rw_x < rw_stopx; ++rw_x)
    {
        // mark floor / ceiling areas
        int     yl = (topfrac + HEIGHTUNIT - 1) >> HEIGHTBITS;
        int     yh = bottomfrac >> HEIGHTBITS;

        // no space above wall?
        int     bottom;
        int     top = ceilingclip[rw_x] + 1;
        boolean bottomclipped = false;
        boolean topclipped = false;

        if (yl < top)
        {
            yl = top;
            topclipped = true;
        }

        if (markceiling)
        {
            bottom = yl - 1;

            if (bottom >= floorclip[rw_x])
                bottom = floorclip[rw_x] - 1;

            if (top <= bottom)
            {
                ceilingplane->top[rw_x] = top;
                ceilingplane->bottom[rw_x] = bottom;
            }

            ceilingclip[rw_x] = bottom;
        }

        bottom = floorclip[rw_x] - 1;
        if (yh > bottom)
        {
            yh = bottom;
            bottomclipped = true;
        }

        if (markfloor)
        {

            top = MAX(yh, ceilingclip[rw_x]);
            if (++top <= bottom)
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
            angle_t     angle = ((rw_centerangle + xtoviewangle[rw_x]) >> ANGLETOFINESHIFT) & 0x1fff;

            texturecolumn = (rw_offset - FixedMul(finetangent[angle], rw_distance)) >> FRACBITS;

            if (fixedcolormap)
                dc_colormap = fixedcolormap;
            else
                dc_colormap = walllights[BETWEEN(0, rw_scale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)];
            dc_x = rw_x;
            dc_iscale = 0xffffffffu / (unsigned int)rw_scale;
        }

        // draw the wall tiers
        if (midtexture)
        {
            // single sided line
            dc_yl = yl;
            dc_yh = yh;
            dc_topsparkle = false;
            dc_bottomsparkle = (!bottomclipped && dc_yh > dc_yl && rw_distance < (512 << FRACBITS));
            dc_texturemid = rw_midtexturemid;
            dc_source = R_GetColumn(midtexture, texturecolumn);
            dc_texheight = midtexheight;
            if ((brightmaps & WALLS) && texturefullbright[midtexture] && !fixedcolormap)
                fbwallcolfunc(texturefullbright[midtexture]);
            else
                wallcolfunc();
            ceilingclip[rw_x] = viewheight;
            floorclip[rw_x] = -1;
        }
        else
        {
            // two sided line
            if (toptexture)
            {
                // top wall
                int     mid = pixhigh >> HEIGHTBITS;

                pixhigh += pixhighstep;

                dc_bottomsparkle = true;
                if (mid >= floorclip[rw_x])
                {
                    mid = floorclip[rw_x] - 1;
                    dc_bottomsparkle = false;
                }

                if (mid >= yl)
                {
                    dc_yl = yl;
                    dc_yh = mid;
                    dc_topsparkle = false;
                    dc_bottomsparkle = (dc_bottomsparkle && dc_yh > dc_yl && rw_distance < (512 << FRACBITS));
                    dc_texturemid = rw_toptexturemid;
                    dc_source = R_GetColumn(toptexture, texturecolumn);
                    dc_texheight = toptexheight;
                    if ((brightmaps & WALLS) && texturefullbright[toptexture] && !fixedcolormap)
                        fbwallcolfunc(texturefullbright[toptexture]);
                    else
                        wallcolfunc();
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
                int     mid = (pixlow + HEIGHTUNIT - 1) >> HEIGHTBITS;

                pixlow += pixlowstep;

                // no space above wall?
                dc_topsparkle = true;
                if (mid <= ceilingclip[rw_x])
                {
                    mid = ceilingclip[rw_x] + 1;
                    dc_topsparkle = false;
                }

                if (mid <= yh)
                {
                    dc_yl = mid;
                    dc_yh = yh;
                    dc_topsparkle = (dc_topsparkle && dc_yh > dc_yl && rw_distance < (128 << FRACBITS));
                    dc_bottomsparkle = (!bottomclipped && dc_yh > dc_yl && rw_distance < (512 << FRACBITS));
                    dc_texturemid = rw_bottomtexturemid;
                    dc_source = R_GetColumn(bottomtexture, texturecolumn);
                    dc_texheight = bottomtexheight;
                    if ((brightmaps & WALLS) && texturefullbright[bottomtexture] && !fixedcolormap)
                        fbwallcolfunc(texturefullbright[bottomtexture]);
                    else
                        wallcolfunc();
                    floorclip[rw_x] = mid;
                }
                else
                    floorclip[rw_x] = yh + 1;
            }
            else
                // no bottom wall
                if (markfloor)
                    floorclip[rw_x] = yh + 1;

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
// R_StoreWallRange
// A wall segment will be drawn
//  between start and stop pixels (inclusive).
//
void R_StoreWallRange(int start, int stop)
{
    fixed_t     hyp;
    angle_t     offsetangle;
    int         lightnum;

    sidedef = curline->sidedef;
    linedef = curline->linedef;

    // mark the segment as visible for automap
    linedef->flags |= ML_MAPPED;

    // [BH] if in automap, we're done now that line is mapped
    if (automapactive)
        return;

    // killough 1/98 -- fix 2s line HOM
    if (ds_p == drawsegs + maxdrawsegs)
    {
        unsigned int    pos = ds_p - drawsegs;                          // jff 8/9/98 fix from ZDOOM1.14a
        unsigned int    newmax = (maxdrawsegs ? maxdrawsegs * 2 : 128); // killough

        drawsegs = realloc(drawsegs, newmax * sizeof(*drawsegs));
        ds_p = drawsegs + pos;                                          // jff 8/9/98 fix from ZDOOM1.14a
        maxdrawsegs = newmax;
    }

    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle + ANG90;
    offsetangle = rw_normalangle - rw_angle1;

    if (ABS(offsetangle) > ANG90)
        offsetangle = ANG90;

    hyp = (viewx == curline->v1->x && viewy == curline->v1->y ? 0 :
        R_PointToDist(curline->v1->x, curline->v1->y));
    rw_distance = FixedMul(hyp, finecosine[offsetangle >> ANGLETOFINESHIFT]);

    ds_p->x1 = rw_x = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    rw_stopx = stop + 1;

    // killough 1/6/98, 2/1/98: remove limit on openings
    {
        extern int      *openings;
        extern size_t   maxopenings;
        size_t          pos = lastopening - openings;
        size_t          need = (rw_stopx - start) * 4 + pos;

        if (need > maxopenings)
        {
            drawseg_t   *ds;                    // jff 8/9/98 needed for fix from ZDoom
            int         *oldopenings = openings;
            int         *oldlast = lastopening;

            do
                maxopenings = (maxopenings ? maxopenings * 2 : 16384);
            while (need > maxopenings);
            openings = (int *)realloc(openings, maxopenings * sizeof(*openings));
            lastopening = openings + pos;

            // jff 8/9/98 borrowed fix for openings from ZDOOM1.14
            // [RH] We also need to adjust the openings pointers that
            //    were already stored in drawsegs.
            for (ds = drawsegs; ds < ds_p; ds++)
            {
                if (ds->maskedtexturecol + ds->x1 >= oldopenings
                    && ds->maskedtexturecol + ds->x1 <= oldlast)
                    ds->maskedtexturecol = ds->maskedtexturecol - oldopenings + openings;
                if (ds->sprtopclip + ds->x1 >= oldopenings
                    && ds->sprtopclip + ds->x1 <= oldlast)
                    ds->sprtopclip = ds->sprtopclip - oldopenings + openings;
                if (ds->sprbottomclip + ds->x1 >= oldopenings
                    && ds->sprbottomclip + ds->x1 <= oldlast)
                    ds->sprbottomclip = ds->sprbottomclip - oldopenings + openings;
            }
        }
    }

    worldtop = frontsector->ceilingheight - viewz;
    worldbottom = frontsector->floorheight - viewz;

    R_SetWiggleHack((worldtop - worldbottom) >> 16);

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
    midtexture = toptexture = bottomtexture = maskedtexture = 0;
    ds_p->maskedtexturecol = NULL;

    if (!backsector)
    {
        // single sided line
        midtexture = texturetranslation[sidedef->midtexture];
        midtexheight = textureheight[midtexture] >> FRACBITS;

        // a single sided line is terminal, so it must mark ends
        markfloor = markceiling = true;

        if (linedef->flags & ML_DONTPEGBOTTOM)
            // bottom of texture at bottom
            rw_midtexturemid = frontsector->floorheight + textureheight[sidedef->midtexture] - viewz;
        else
            // top of texture at top
            rw_midtexturemid = worldtop;

        rw_midtexturemid += sidedef->rowoffset;

        // killough 3/27/98: reduce offset
        {
            fixed_t     h = textureheight[sidedef->midtexture];

            if (h & (h - FRACUNIT))
                rw_midtexturemid %= h;
        }

        ds_p->silhouette = SIL_BOTH;
        ds_p->sprtopclip = screenheightarray;
        ds_p->sprbottomclip = negonearray;
        ds_p->bsilheight = INT_MAX;
        ds_p->tsilheight = INT_MIN;
    }
    else
    {
        // two sided line
        ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
        ds_p->silhouette = 0;

        if (frontsector->floorheight > backsector->floorheight)
        {
            ds_p->silhouette = SIL_BOTTOM;
            ds_p->bsilheight = frontsector->floorheight;
        }
        else if (backsector->floorheight > viewz)
        {
            ds_p->silhouette = SIL_BOTTOM;
            ds_p->bsilheight = INT_MAX;
        }

        if (frontsector->ceilingheight < backsector->ceilingheight)
        {
            ds_p->silhouette |= SIL_TOP;
            ds_p->tsilheight = frontsector->ceilingheight;
        }
        else if (backsector->ceilingheight < viewz)
        {
            ds_p->silhouette |= SIL_TOP;
            ds_p->tsilheight = INT_MIN;
        }

        // killough 1/17/98: this test is required if the fix
        // for the automap bug (r_bsp.c) is used, or else some
        // sprites will be displayed behind closed doors. That
        // fix prevents lines behind closed doors with dropoffs
        // from being displayed on the automap.
        //
        // killough 4/7/98: make doorclosed external variable
        {
            extern int doorclosed;

            if (doorclosed || backsector->ceilingheight <= frontsector->floorheight)
            {
                ds_p->sprbottomclip = negonearray;
                ds_p->bsilheight = INT_MAX;
                ds_p->silhouette |= SIL_BOTTOM;
            }
            if (doorclosed || backsector->floorheight >= frontsector->ceilingheight)
            {
                ds_p->sprtopclip = screenheightarray;
                ds_p->tsilheight = INT_MIN;
                ds_p->silhouette |= SIL_TOP;
            }
        }

        worldhigh = backsector->ceilingheight - viewz;
        worldlow = backsector->floorheight - viewz;

        // hack to allow height changes in outdoor areas
        if (frontsector->ceilingpic == skyflatnum && backsector->ceilingpic == skyflatnum)
            worldtop = worldhigh;

        if (worldlow != worldbottom
            || backsector->floorpic != frontsector->floorpic
            || backsector->lightlevel != frontsector->lightlevel)
            markfloor = true;
        else
            // same plane on both sides
            markfloor = false;

        if (worldhigh != worldtop
            || backsector->ceilingpic != frontsector->ceilingpic
            || backsector->lightlevel != frontsector->lightlevel)
            markceiling = true;
        else
            // same plane on both sides
            markceiling = false;

        if (backsector->ceilingheight <= frontsector->floorheight
            || backsector->floorheight >= frontsector->ceilingheight)
            // closed door
            markceiling = markfloor = true;

        if (worldhigh < worldtop)
        {
            // top texture
            toptexture = texturetranslation[sidedef->toptexture];
            toptexheight = textureheight[toptexture] >> FRACBITS;

            if (linedef->flags & ML_DONTPEGTOP)
                // top of texture at top
                rw_toptexturemid = worldtop;
            else
                // bottom of texture
                rw_toptexturemid = backsector->ceilingheight + textureheight[sidedef->toptexture] - viewz;
        }
        if (worldlow > worldbottom)
        {
            // bottom texture
            bottomtexture = texturetranslation[sidedef->bottomtexture];
            bottomtexheight = textureheight[bottomtexture] >> FRACBITS;

            if (linedef->flags & ML_DONTPEGBOTTOM)
                // bottom of texture at bottom, top of texture at top
                rw_bottomtexturemid = worldtop;
            else        // top of texture at top
                rw_bottomtexturemid = worldlow;
        }
        rw_toptexturemid += sidedef->rowoffset;

        // killough 3/27/98: reduce offset
        {
            fixed_t     h = textureheight[sidedef->toptexture];

            if (h & (h - FRACUNIT))
                rw_toptexturemid %= h;
        }

        rw_bottomtexturemid += sidedef->rowoffset;

        // killough 3/27/98: reduce offset
        {
            fixed_t     h = textureheight[sidedef->bottomtexture];

            if (h & (h - FRACUNIT))
                rw_bottomtexturemid %= h;
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
    segtextured = (midtexture | toptexture | bottomtexture | maskedtexture);

    if (segtextured)
    {
        rw_offset = FixedMul(hyp, -finesine[offsetangle >> ANGLETOFINESHIFT]);

        rw_offset += sidedef->textureoffset + curline->offset;

        rw_centerangle = ANG90 + viewangle - rw_normalangle;

        // calculate light table
        //  use different light tables
        //  for horizontal / vertical / diagonal
        if (!fixedcolormap)
        {
            lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT) + extralight * LIGHTBRIGHT;

            if (frontsector->ceilingpic != skyflatnum)
            {
                if (curline->v1->y == curline->v2->y)
                    lightnum -= LIGHTBRIGHT;
                else if (curline->v1->x == curline->v2->x)
                    lightnum += LIGHTBRIGHT;
            }

            walllights = scalelight[BETWEEN(0, lightnum, LIGHTLEVELS - 1)];
        }
    }

    // if a floor / ceiling plane is on the wrong side
    //  of the view plane, it is definitely invisible
    //  and doesn't need to be marked.
    if (frontsector->floorheight >= viewz)
        // above view plane
        markfloor = false;

    if (frontsector->ceilingheight <= viewz && frontsector->ceilingpic != skyflatnum)
        // below view plane
        markceiling = false;

    // calculate incremental stepping values for texture edges
    worldtop >>= invhgtbits;
    worldbottom >>= invhgtbits;

    topstep = -FixedMul(rw_scalestep, worldtop);
    topfrac = (centeryfrac >> invhgtbits) - FixedMul(worldtop, rw_scale);

    bottomstep = -FixedMul(rw_scalestep, worldbottom);
    bottomfrac = (centeryfrac >> invhgtbits) - FixedMul(worldbottom, rw_scale);

    if (backsector)
    {
        worldhigh >>= invhgtbits;
        worldlow >>= invhgtbits;

        if (worldhigh < worldtop)
        {
            pixhigh = (centeryfrac >> invhgtbits) - FixedMul(worldhigh, rw_scale);
            pixhighstep = -FixedMul(rw_scalestep, worldhigh);
        }

        if (worldlow > worldbottom)
        {
            pixlow = (centeryfrac >> invhgtbits) - FixedMul(worldlow, rw_scale);
            pixlowstep = -FixedMul(rw_scalestep, worldlow);
        }
    }

    // render it
    if (markceiling)
    {
        if (ceilingplane)
            ceilingplane = R_CheckPlane(ceilingplane, rw_x, rw_stopx - 1);
        else
            markceiling = false;
    }

    if (markfloor)
    {
        if (floorplane)
            floorplane = R_CheckPlane(floorplane, rw_x, rw_stopx - 1);
        else
            markfloor = false;
    }

    R_RenderSegLoop();

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

    if (maskedtexture && !(ds_p->silhouette & SIL_TOP))
    {
        ds_p->silhouette |= SIL_TOP;
        ds_p->tsilheight = INT_MIN;
    }
    if (maskedtexture && !(ds_p->silhouette & SIL_BOTTOM))
    {
        ds_p->silhouette |= SIL_BOTTOM;
        ds_p->bsilheight = INT_MAX;
    }
    ++ds_p;
}
