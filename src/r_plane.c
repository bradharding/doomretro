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
#include "z_zone.h"
#include "w_wad.h"

#include "doomdef.h"
#include "doomstat.h"

#include "r_local.h"
#include "r_sky.h"



planefunction_t         floorfunc;
planefunction_t         ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
#define MAXVISPLANES    1024
visplane_t              visplanes[MAXVISPLANES];
visplane_t              *lastvisplane;
visplane_t              *floorplane;
visplane_t              *ceilingplane;

// ?
#define MAXOPENINGS     SCREENWIDTH * 64
size_t                  maxopenings;
int                     *openings;
int                     *lastopening;


//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
int                     floorclip[SCREENWIDTH];
int                     ceilingclip[SCREENWIDTH];

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int                     spanstart[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t            **planezlight;
fixed_t                 planeheight;

fixed_t                 yslope[SCREENHEIGHT];
fixed_t                 distscale[SCREENWIDTH];
fixed_t                 basexscale;
fixed_t                 baseyscale;



//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
static void R_MapPlane(int y, int x1, int x2)
{
    fixed_t             distance;
    unsigned            index;
    float               slope;
    float               realy;

    distance = FixedMul (planeheight, yslope[y]);
    slope = (float)(planeheight / 65535.0f / ABS(centery - y));
    realy = (float)distance / 65536.0f;

    ds_xstep = (fixed_t)(viewsin * slope);
    ds_ystep = (fixed_t)(viewcos * slope);

    ds_xfrac =  viewx + (int)(viewcos * realy) + (x1 - centerx) * ds_xstep;
    ds_yfrac = -viewy - (int)(viewsin * realy) + (x1 - centerx) * ds_ystep;

    if (!fixedcolormap)
    {
        index = distance >> LIGHTZSHIFT;

        if (index >= MAXLIGHTZ )
            index = MAXLIGHTZ - 1;

        ds_colormap = planezlight[index];
    }
    else
        ds_colormap = fixedcolormap;

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    spanfunc();
}


//
// R_ClearPlanes
// At begining of frame.
//
void R_ClearPlanes(void)
{
    int                 i;
    angle_t             angle;

    // opening / clipping determination
    for (i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;

    // left to right mapping
    angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv(viewsin, projection);
    baseyscale = FixedDiv(viewcos, projection);
}




//
// R_FindPlane
//
visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel)
{
    visplane_t          *check;

    if (picnum == skyflatnum)
    {
        height = 0;                     // all skys map together
        lightlevel = 0;
    }

    for (check = visplanes; check < lastvisplane; check++)
    {
        if (height == check->height
            && picnum == check->picnum
            && lightlevel == check->lightlevel)
        {
            break;
        }
    }


    if (check < lastvisplane)
        return check;

    if (lastvisplane - visplanes == MAXVISPLANES)
        I_Error ("R_FindPlane: no more visplanes");

    lastvisplane++;

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = viewwidth;
    check->maxx = -1;

    memset (check->top, 0xffff, sizeof(check->top));

    return check;
}


//
// R_CheckPlane
//
visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop)
{
    int                 intrl;
    int                 intrh;
    int                 unionl;
    int                 unionh;
    int                 x;

    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    for (x = intrl; x <= intrh; x++)
        if (pl->top[x] != 0xffffffffu)
            break;

    if (x > intrh)
    {
        pl->minx = unionl;
        pl->maxx = unionh;

        // use the same one
        return pl;
    }

    // make a new visplane
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;

    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;

    memset(pl->top, 0xffffffffu, sizeof(pl->top));

    return pl;
}


//
// R_MakeSpans
//
void R_MakeSpans(int x, int t1, int b1, int t2, int b2)
{
    while (t1 < t2 && t1 <= b1)
    {
        R_MapPlane(t1, spanstart[t1], x - 1);
        t1++;
    }
    while (b1 > b2 && b1 >= t1)
    {
        R_MapPlane(b1, spanstart[b1], x - 1);
        b1--;
    }

    while (t2 < t1 && t2 <= b2)
    {
        spanstart[t2] = x;
        t2++;
    }
    while (b2 > b1 && b2 >= t2)
    {
        spanstart[b2] = x;
        b2--;
    }
}



//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes(void)
{
    visplane_t          *pl;
    int                 light;
    int                 x;
    int                 stop;
    int                 angle;
    int                 lumpnum;

    for (pl = visplanes; pl < lastvisplane; pl++)
    {
        if (pl->minx > pl->maxx)
            continue;


        // sky flat
        if (pl->picnum == skyflatnum)
        {
            dc_iscale = pspriteiscale;

            // Sky is always drawn full bright,
            //  i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected
            //  by INVUL inverse mapping.

            dc_colormap = (fixedcolormap ? fixedcolormap : colormaps); // [BH] So let's fix it...
            dc_texturemid = skytexturemid;
            for (x = pl->minx; x <= pl->maxx; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];

                if (dc_yl <= dc_yh)
                {
                    angle = (viewangle + xtoviewangle[x]) >> ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle);
                    dc_texheight = textureheight[skytexture] >> FRACBITS;
                    if (flipsky)
                        skycolfunc();
                    else
                        wallcolfunc();
                }
            }
            continue;
        }

        // regular flat
        lumpnum = firstflat + flattranslation[pl->picnum];
        ds_source = (byte *)W_CacheLumpNum(lumpnum, PU_STATIC);

        planeheight = ABS(pl->height-viewz);
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;

        if (light >= LIGHTLEVELS)
            light = LIGHTLEVELS - 1;

        if (light < 0)
            light = 0;

        planezlight = zlight[light];

        pl->top[pl->maxx + 1] = 0xffff;
        pl->top[pl->minx - 1] = 0xffff;

        stop = pl->maxx + 1;

        for (x = pl->minx; x <= stop; x++)
        {
            R_MakeSpans(x, pl->top[x - 1],
                        pl->bottom[x - 1],
                        pl->top[x],
                        pl->bottom[x]);
        }

        W_ReleaseLumpNum(lumpnum);
    }
}