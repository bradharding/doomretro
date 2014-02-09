/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include <stdlib.h>

#include "i_system.h"

#include "doomdef.h"
#include "doomstat.h"

#include "r_local.h"
#include "r_sky.h"


// killough 1/6/98: replaced globals with statics where appropriate

// True if any of the segs textures might be visible.
boolean                 segtextured;

// False if the back side is the same plane.
boolean                 markfloor;
boolean                 markceiling;

boolean                 maskedtexture;
int                     toptexture;
int                     bottomtexture;
int                     midtexture;


angle_t                 rw_normalangle;
// angle to line origin
int                     rw_angle1;

fixed_t                 rw_distance;

//
// regular wall
//
static int              rw_x;
static int              rw_stopx;
static angle_t          rw_centerangle;
static fixed_t          rw_offset;
static fixed_t          rw_scale;
static fixed_t          rw_scalestep;
static fixed_t          rw_midtexturemid;
static fixed_t          rw_toptexturemid;
static fixed_t          rw_bottomtexturemid;

static int              worldtop;
static int              worldbottom;
static int              worldhigh;
static int              worldlow;

static fixed_t          pixhigh;
static fixed_t          pixlow;
static fixed_t          pixhighstep;
static fixed_t          pixlowstep;

static fixed_t          topfrac;
static fixed_t          topstep;

static fixed_t          bottomfrac;
static fixed_t          bottomstep;


lighttable_t            **walllights;

static int              *maskedtexturecol;



//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
static fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
    int     anglea = ANG90 + (visangle-viewangle);
    int     angleb = ANG90 + (visangle-rw_normalangle);
    int     den = FixedMul(rw_distance, finesine[anglea>>ANGLETOFINESHIFT]);
    int     max = 1024 * FRACUNIT;      // [BH] fix wobbly vertexes
    fixed_t num = FixedMul(projectiony, finesine[angleb>>ANGLETOFINESHIFT]);

    return ((den >> 8) > 0 && den > (num >> 16) ? ((num = FixedDiv(num, den)) > max ? 
            max : MAX(256, num)) : max);
}

//
// R_RenderMaskedSegRange
//
void R_RenderMaskedSegRange(drawseg_t *ds, int x1, int x2)
{
    unsigned            index;
    column_t            *col;
    int                 lightnum;
    int                 texnum;

    // Calculate light table.
    // Use different light tables
    //   for horizontal / vertical / diagonal. Diagonal?
    curline = ds->curline;
    frontsector = curline->frontsector;
    backsector = curline->backsector;
    texnum = texturetranslation[curline->sidedef->midtexture];

    lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT) + extralight;
    if (curline->v1->y == curline->v2->y)
        lightnum--;
    else if (curline->v1->x == curline->v2->x)
        lightnum++;

    walllights = scalelight[lightnum >= LIGHTLEVELS ? LIGHTLEVELS - 1 : MAX(0, lightnum)];

    maskedtexturecol = ds->maskedtexturecol;

    rw_scalestep = ds->scalestep;
    spryscale = ds->scale1 + (x1 - ds->x1) * rw_scalestep;
    mfloorclip = ds->sprbottomclip;
    mceilingclip = ds->sprtopclip;

    // find positioning
    if (curline->linedef->flags & ML_DONTPEGBOTTOM)
    {
        dc_texturemid = (frontsector->floorheight > backsector->floorheight
            ? frontsector->floorheight : backsector->floorheight);
        dc_texturemid = dc_texturemid + textureheight[texnum] - viewz;
    }
    else
    {
        dc_texturemid = (frontsector->ceilingheight < backsector->ceilingheight
            ? frontsector->ceilingheight : backsector->ceilingheight);
        dc_texturemid = dc_texturemid - viewz;
    }
    dc_texturemid += curline->sidedef->rowoffset;

    dc_texheight = 0;

    if (fixedcolormap)
        dc_colormap = fixedcolormap;

    // draw the columns
    for (dc_x = x1; dc_x <= x2; dc_x++)
    {
        // calculate lighting
        if (maskedtexturecol[dc_x] != INT_MAX)
        {
            int64_t t = ((int64_t)centeryfrac << FRACBITS) - (int64_t)dc_texturemid * spryscale;

            if (!fixedcolormap)
            {
                index = spryscale >> LIGHTSCALESHIFT;

                if (index >=  MAXLIGHTSCALE)
                    index = MAXLIGHTSCALE - 1;

                dc_colormap = walllights[index];
            }

            sprtopscreen = (fixed_t)(t >> FRACBITS);
            dc_iscale = 0xffffffffu / (unsigned)spryscale;

            // draw the texture
            col = (column_t *)((byte *)R_GetColumn(texnum, maskedtexturecol[dc_x]) - 3);

            R_DrawMaskedColumn2(col);
            maskedtexturecol[dc_x] = INT_MAX;
        }
        spryscale += rw_scalestep;
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
#define HEIGHTBITS      12
#define HEIGHTUNIT      (1 << HEIGHTBITS)

void R_RenderSegLoop(void)
{
    fixed_t             texturecolumn;

    for (; rw_x < rw_stopx; rw_x++)
    {
        int             yl;
        int             yh;
        int             top;
        int             bottom;
        boolean         bottomclipped = false;
        boolean         topclipped = false;

        // mark floor / ceiling areas
        yl = (topfrac + HEIGHTUNIT - 1) >> HEIGHTBITS;

        // no space above wall?
        top = ceilingclip[rw_x] + 1;
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

        yh = bottomfrac >> HEIGHTBITS;

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
            angle_t     angle;
            unsigned    index;

            // calculate texture offset
            angle = ((rw_centerangle + xtoviewangle[rw_x]) >> ANGLETOFINESHIFT) & 0x1fff;
            texturecolumn = rw_offset - FixedMul(finetangent[angle], rw_distance);
            texturecolumn >>= FRACBITS;
            // calculate lighting
            index = rw_scale >> LIGHTSCALESHIFT;

            if (index >= MAXLIGHTSCALE)
                index = MAXLIGHTSCALE - 1;

            dc_colormap = walllights[index];
            dc_x = rw_x;
            dc_iscale = 0xffffffffu / (unsigned)rw_scale;
        }
        else
        {
            // purely to shut up the compiler

            texturecolumn = 0;
        }

        // draw the wall tiers
        if (midtexture)
        {
            // single sided line
            dc_yl = yl;
            dc_yh = yh;
            dc_topsparkle = false;
            dc_bottomsparkle = (!bottomclipped
                                && dc_yh > dc_yl
                                && rw_distance < (512 << FRACBITS));
            dc_texturemid = rw_midtexturemid;
            dc_source = R_GetColumn(midtexture, texturecolumn);
            dc_texheight = textureheight[midtexture] >> FRACBITS;
            if (texturefullbright[midtexture] && !fixedcolormap)
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
                int mid;

                // top wall
                mid = pixhigh >> HEIGHTBITS;
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
                    dc_bottomsparkle = (dc_bottomsparkle
                                        && dc_yh > dc_yl
                                        && rw_distance < (512 << FRACBITS));
                    dc_texturemid = rw_toptexturemid;
                    dc_source = R_GetColumn(toptexture, texturecolumn);
                    dc_texheight = textureheight[toptexture] >> FRACBITS;
                    if (texturefullbright[toptexture] && !fixedcolormap)
                        fbwallcolfunc(texturefullbright[toptexture]);
                    else
                        wallcolfunc();
                    ceilingclip[rw_x] = mid;
                }
                else
                    ceilingclip[rw_x] = yl - 1;
            }
            else
            {
                // no top wall
                if (markceiling)
                    ceilingclip[rw_x] = yl - 1;
            }

            if (bottomtexture)
            {
                // bottom wall
                int mid = (pixlow + HEIGHTUNIT - 1) >> HEIGHTBITS;
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
                    dc_topsparkle = (dc_topsparkle
                                     && dc_yh > dc_yl
                                     && rw_distance < (128 << FRACBITS));
                    dc_bottomsparkle = (!bottomclipped
                                        && dc_yh > dc_yl
                                        && rw_distance < (512 << FRACBITS));
                    dc_texturemid = rw_bottomtexturemid;
                    dc_source = R_GetColumn(bottomtexture, texturecolumn);
                    dc_texheight = textureheight[bottomtexture] >> FRACBITS;
                    if (texturefullbright[bottomtexture] && !fixedcolormap)
                        fbwallcolfunc(texturefullbright[bottomtexture]);
                    else
                        wallcolfunc();
                    floorclip[rw_x] = mid;
                }
                else
                    floorclip[rw_x] = yh + 1;
            }
            else
            {
                // no bottom wall
                if (markfloor)
                    floorclip[rw_x] = yh + 1;
            }

            if (maskedtexture)
            {
                // save texturecol
                //  for backdrawing of masked mid texture
                maskedtexturecol[rw_x] = texturecolumn;
            }
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
    fixed_t             hyp;
    fixed_t             sineval;
    angle_t             distangle, offsetangle;
    fixed_t             vtop;
    int                 lightnum;

    sidedef = curline->sidedef;
    linedef = curline->linedef;

    // mark the segment as visible for automap
    linedef->flags |= ML_MAPPED;

    // [BH] if in automap, we're done now that line is mapped
    if (automapactive)
        return;

    // don't overflow and crash
    if (ds_p == drawsegs + maxdrawsegs)
    {
        unsigned pos = ds_p - drawsegs;
        unsigned newmax = (maxdrawsegs ? maxdrawsegs * 2 : 128);

        drawsegs = (drawseg_t *)realloc(drawsegs, newmax * sizeof(*drawsegs));
        ds_p = drawsegs + pos;
        maxdrawsegs = newmax;
    }

    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle + ANG90;
    offsetangle = ABS(rw_normalangle - rw_angle1);

    if (offsetangle > ANG90)
        offsetangle = ANG90;

    distangle = ANG90 - offsetangle;
    hyp = (viewx == curline->v1->x
           && viewy == curline->v1->y ? 0 : R_PointToDist(curline->v1->x, curline->v1->y));
    sineval = finesine[distangle >> ANGLETOFINESHIFT];
    rw_distance = FixedMul(hyp, sineval);

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
            drawseg_t *ds;                      //jff 8/9/98 needed for fix from ZDoom
            int *oldopenings = openings;
            int *oldlast = lastopening;

            do
            {
                maxopenings = (maxopenings ? maxopenings * 2 : 16384);
            }
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

    // calculate scale at both ends and step
    ds_p->scale1 = rw_scale =
        R_ScaleFromGlobalAngle(viewangle + xtoviewangle[start]);

    if (stop > start)
    {
        ds_p->scale2 = R_ScaleFromGlobalAngle(viewangle + xtoviewangle[stop]);
        ds_p->scalestep = rw_scalestep =
            (ds_p->scale2 - rw_scale) / (stop - start);
    }
    else
    {
        ds_p->scale2 = ds_p->scale1;
    }

    // calculate texture boundaries
    //  and decide if floor / ceiling marks are needed
    worldtop = frontsector->ceilingheight - viewz;
    worldbottom = frontsector->floorheight - viewz;

    midtexture = toptexture = bottomtexture = maskedtexture = 0;
    ds_p->maskedtexturecol = NULL;

    if (!backsector)
    {
        // single sided line
        midtexture = texturetranslation[sidedef->midtexture];
        // a single sided line is terminal, so it must mark ends
        markfloor = markceiling = true;
        if (linedef->flags & ML_DONTPEGBOTTOM)
        {
            vtop = frontsector->floorheight +
                textureheight[sidedef->midtexture];
            // bottom of texture at bottom
            rw_midtexturemid = vtop - viewz;
        }
        else
        {
            // top of texture at top
            rw_midtexturemid = worldtop;
        }
        rw_midtexturemid += sidedef->rowoffset;

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
        if (frontsector->ceilingpic == skyflatnum
            && backsector->ceilingpic == skyflatnum)
        {
            worldtop = worldhigh;
        }

        if (worldlow != worldbottom
            || backsector->floorpic != frontsector->floorpic
            || backsector->lightlevel != frontsector->lightlevel)
        {
            markfloor = true;
        }
        else
        {
            // same plane on both sides
            markfloor = false;
        }

        if (worldhigh != worldtop
            || backsector->ceilingpic != frontsector->ceilingpic
            || backsector->lightlevel != frontsector->lightlevel)
        {
            markceiling = true;
        }
        else
        {
            // same plane on both sides
            markceiling = false;
        }

        if (backsector->ceilingheight <= frontsector->floorheight
            || backsector->floorheight >= frontsector->ceilingheight)
        {
            // closed door
            markceiling = markfloor = true;
        }

        if (worldhigh < worldtop)
        {
            // top texture
            toptexture = texturetranslation[sidedef->toptexture];
            if (linedef->flags & ML_DONTPEGTOP)
            {
                // top of texture at top
                rw_toptexturemid = worldtop;
            }
            else
            {
                vtop =
                    backsector->ceilingheight
                    + textureheight[sidedef->toptexture];

                // bottom of texture
                rw_toptexturemid = vtop - viewz;
            }
        }
        if (worldlow > worldbottom)
        {
            // bottom texture
            bottomtexture = texturetranslation[sidedef->bottomtexture];

            if (linedef->flags & ML_DONTPEGBOTTOM)
            {
                // bottom of texture at bottom
                // top of texture at top
                rw_bottomtexturemid = worldtop;
            }
            else        // top of texture at top
                rw_bottomtexturemid = worldlow;
        }
        rw_toptexturemid += sidedef->rowoffset;
        rw_bottomtexturemid += sidedef->rowoffset;

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
        offsetangle = rw_normalangle - rw_angle1;

        if (offsetangle > ANG180)
            offsetangle = 0 - offsetangle;

        if (offsetangle > ANG90)
            offsetangle = ANG90;

        sineval = finesine[offsetangle >> ANGLETOFINESHIFT];
        rw_offset = FixedMul(hyp, sineval);

        if (rw_normalangle - rw_angle1 < ANG180)
            rw_offset = -rw_offset;

        rw_offset += sidedef->textureoffset + curline->offset;
        rw_centerangle = ANG90 + viewangle - rw_normalangle;

        // calculate light table
        //  use different light tables
        //  for horizontal / vertical / diagonal
        if (!fixedcolormap)
        {
            lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT) + extralight;

            if (curline->v1->y == curline->v2->y)
                lightnum--;
            else if (curline->v1->x == curline->v2->x)
                lightnum++;

            if (lightnum < 0)
                walllights = scalelight[0];
            else if (lightnum >= LIGHTLEVELS)
                walllights = scalelight[LIGHTLEVELS - 1];
            else
                walllights = scalelight[lightnum];
        }
    }

    // if a floor / ceiling plane is on the wrong side
    //  of the view plane, it is definitely invisible
    //  and doesn't need to be marked.

    if (frontsector->floorheight >= viewz)
    {
        // above view plane
        markfloor = false;
    }

    if (frontsector->ceilingheight <= viewz
        && frontsector->ceilingpic != skyflatnum)
    {
        // below view plane
        markceiling = false;
    }

    // calculate incremental stepping values for texture edges
    worldtop >>= 4;
    worldbottom >>= 4;

    topstep = -FixedMul(rw_scalestep, worldtop);
    topfrac = (centeryfrac >> 4) - FixedMul(worldtop, rw_scale);

    bottomstep = -FixedMul(rw_scalestep, worldbottom);
    bottomfrac = (centeryfrac >> 4) - FixedMul(worldbottom, rw_scale);

    if (backsector)
    {
        worldhigh >>= 4;
        worldlow >>= 4;

        if (worldhigh < worldtop)
        {
            pixhigh = (centeryfrac >> 4) - FixedMul(worldhigh, rw_scale);
            pixhighstep = -FixedMul(rw_scalestep, worldhigh);
        }

        if (worldlow > worldbottom)
        {
            pixlow = (centeryfrac >> 4) - FixedMul(worldlow, rw_scale);
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
    if (((ds_p->silhouette & SIL_TOP) || maskedtexture)
        && !ds_p->sprtopclip)
    {
        memcpy(lastopening, ceilingclip + start, sizeof(int) * (rw_stopx - start));
        ds_p->sprtopclip = lastopening - start;
        lastopening += rw_stopx - start;
    }

    if (((ds_p->silhouette & SIL_BOTTOM) || maskedtexture)
        && !ds_p->sprbottomclip)
    {
        memcpy(lastopening, floorclip + start, sizeof(int) * (rw_stopx - start));
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
    ds_p++;
}