/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

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
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include <string.h>

#include "c_console.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_config.h"
#include "p_local.h"
#include "r_sky.h"
#include "w_wad.h"

#define MAXVISPLANES    384

static visplane_t   *visplanes = NULL;
static visplane_t   *lastvisplane;
visplane_t          *floorplane;
visplane_t          *ceilingplane;

int                *openings;                   // dropoff overflow
int                *lastopening;                // dropoff overflow

// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
int                 floorclip[SCREENWIDTH];     // dropoff overflow
int                 ceilingclip[SCREENWIDTH];   // dropoff overflow

// texture mapping
static lighttable_t **planezlight;
static fixed_t      planeheight;

static fixed_t      xoffs, yoffs;               // killough 2/28/98: flat offsets

fixed_t             *yslope;
fixed_t             yslopes[LOOKDIRS][SCREENHEIGHT];

static fixed_t      cachedheight[SCREENHEIGHT];

dboolean            r_liquid_current = r_liquid_current_default;
dboolean            r_liquid_swirl = r_liquid_swirl_default;

extern fixed_t      animatedliquidxoffs;
extern fixed_t      animatedliquidyoffs;
extern dboolean     canmouselook;

//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
static void R_MapPlane(int y, int x1, int x2)
{
    static fixed_t  cacheddistance[SCREENHEIGHT];
    static fixed_t  cachedviewcosdistance[SCREENHEIGHT];
    static fixed_t  cachedviewsindistance[SCREENHEIGHT];
    static fixed_t  cachedxstep[SCREENHEIGHT];
    static fixed_t  cachedystep[SCREENHEIGHT];
    fixed_t         distance;
    fixed_t         viewcosdistance;
    fixed_t         viewsindistance;
    int             dx;

    if (planeheight != cachedheight[y])
    {
        int dy = ABS(centery - y);

        if (!dy)
            return;

        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);
        viewcosdistance = cachedviewcosdistance[y] = FixedMul(viewcos, distance);
        viewsindistance = cachedviewsindistance[y] = FixedMul(viewsin, distance);
        ds_xstep = cachedxstep[y] = FixedMul(viewsin, planeheight) / dy;
        ds_ystep = cachedystep[y] = FixedMul(viewcos, planeheight) / dy;
    }
    else
    {
        distance = cacheddistance[y];
        viewcosdistance = cachedviewcosdistance[y];
        viewsindistance = cachedviewsindistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }

    dx = x1 - centerx;
    ds_xfrac = viewx + xoffs + viewcosdistance + dx * ds_xstep;
    ds_yfrac = -viewy + yoffs - viewsindistance + dx * ds_ystep;

    ds_colormap = (fixedcolormap ? fixedcolormap : planezlight[MIN(distance >> LIGHTZSHIFT, MAXLIGHTZ - 1)]);

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    spanfunc();
}

//
// R_ClearPlanes
// At beginning of frame.
//
void R_ClearPlanes(void)
{
    // opening / clipping determination
    for (int i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;

    // texture calculation
    memset(cachedheight, 0, sizeof(cachedheight));
}

static void R_RaiseVisplanes(visplane_t **vp)
{
    static int  numvisplanes;

    if (lastvisplane - visplanes == numvisplanes)
    {
        int         numvisplanes_old = numvisplanes;
        visplane_t  *visplanes_old = visplanes;

        numvisplanes = (numvisplanes ? 2 * numvisplanes : MAXVISPLANES);
        visplanes = I_Realloc(visplanes, numvisplanes * sizeof(*visplanes));
        memset(visplanes + numvisplanes_old, 0, (numvisplanes - numvisplanes_old) * sizeof(*visplanes));

        lastvisplane = visplanes + numvisplanes_old;
        floorplane = visplanes + (floorplane - visplanes_old);
        ceilingplane = visplanes + (ceilingplane - visplanes_old);

        if (vp)
            *vp = visplanes + (*vp - visplanes_old);
    }
}

//
// R_FindPlane
//
visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel, fixed_t xoffs, fixed_t yoffs)
{
    visplane_t  *check;

    if (picnum == skyflatnum || (picnum & PL_SKYFLAT))          // killough 10/98
    {
        height = 0;                                             // killough 7/19/98: most skies map together
        lightlevel = 0;
    }

    for (check = visplanes; check < lastvisplane; check++)
        if (height == check->height && picnum == check->picnum && lightlevel == check->lightlevel
            && xoffs == check->xoffs && yoffs == check->yoffs)
            return check;

    R_RaiseVisplanes(&check);

    lastvisplane++;
    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->left = viewwidth;
    check->right = -1;

    if (!(picnum & PL_SKYFLAT) && terraintypes[picnum] > SOLID && r_liquid_current && !xoffs && !yoffs)
    {
        check->xoffs = animatedliquidxoffs;
        check->yoffs = animatedliquidyoffs;
    }
    else
    {
        check->xoffs = xoffs;
        check->yoffs = yoffs;
    }

    memset(check->top, USHRT_MAX, sizeof(check->top));
    return check;
}

//
// R_CheckPlane
//
visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop)
{
    int intrl;
    int intrh;
    int unionl;
    int unionh;
    int x;

    if (start < pl->left)
    {
        intrl = pl->left;
        unionl = start;
    }
    else
    {
        unionl = pl->left;
        intrl = start;
    }

    if (stop > pl->right)
    {
        intrh = pl->right;
        unionh = stop;
    }
    else
    {
        unionh = pl->right;
        intrh = stop;
    }

    for (x = intrl; x <= intrh && pl->top[x] == USHRT_MAX; x++);

    // [crispy] fix HOM if ceilingplane and floorplane are the same visplane (e.g. both skies)
    if (!(pl == floorplane && markceiling && floorplane == ceilingplane) && x > intrh)
    {
        pl->left = unionl;
        pl->right = unionh;
    }
    else
    {
        // make a new visplane
        R_RaiseVisplanes(&pl);
        lastvisplane->height = pl->height;
        lastvisplane->picnum = pl->picnum;
        lastvisplane->lightlevel = pl->lightlevel;
        lastvisplane->xoffs = pl->xoffs;
        lastvisplane->yoffs = pl->yoffs;

        pl = lastvisplane++;
        pl->left = start;
        pl->right = stop;
        memset(pl->top, USHRT_MAX, sizeof(pl->top));
    }

    return pl;
}

//
// R_MakeSpans
//
static void R_MakeSpans(visplane_t *pl)
{
    // spanstart holds the start of a plane span
    // initialized to 0 at start
    static int  spanstart[SCREENHEIGHT];
    int         stop = pl->right + 1;

    xoffs = pl->xoffs;
    yoffs = pl->yoffs;
    planeheight = ABS(pl->height - viewz);
    planezlight = zlight[MIN((pl->lightlevel >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
    pl->top[pl->left - 1] = USHRT_MAX;
    pl->top[stop] = USHRT_MAX;

    for (int x = pl->left; x <= stop; x++)
    {
        unsigned short  t1 = pl->top[x - 1];
        unsigned short  b1 = pl->bottom[x - 1];
        unsigned short  t2 = pl->top[x];
        unsigned short  b2 = pl->bottom[x];

        for (; t1 < t2 && t1 <= b1; t1++)
            R_MapPlane(t1, spanstart[t1], x);

        for (; b1 > b2 && b1 >= t1; b1--)
            R_MapPlane(b1, spanstart[b1], x);

        while (t2 < t1 && t2 <= b2)
            spanstart[t2++] = x;

        while (b2 > b1 && b2 >= t2)
            spanstart[b2--] = x;
    }
}

// Ripple Effect from SMMU (r_ripple.cpp) by Simon Howard
#define AMP             2
#define AMP2            2
#define SPEED           40

// swirl factors determine the number of waves per flat width
// 1 cycle per 64 units
#define SWIRLFACTOR     (8192 / 64)

// 1 cycle per 32 units (2 in 64)
#define SWIRLFACTOR2    (8192 / 32)

//
// R_DistortedFlat
//
// Generates a distorted flat from a normal one using a two-dimensional sine wave pattern.
//
static byte *R_DistortedFlat(int flatnum)
{
    static int  swirltic = -1;
    static int  lastflat = -1;
    static int  offset[4096];
    static byte distortedflat[4096];
    byte        *normalflat;
    int         leveltic = leveltime;

    // Already swirled this one?
    if (swirltic == leveltic && lastflat == flatnum)
        return distortedflat;

    swirltic = leveltic;
    lastflat = flatnum;

    leveltic *= SPEED;

    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
        {
            int x1, y1;
            int sinvalue, sinvalue2;

            sinvalue = finesine[(y * SWIRLFACTOR + leveltic * 5 + 900) & 8191];
            sinvalue2 = finesine[(x * SWIRLFACTOR2 + leveltic * 4 + 300) & 8191];
            x1 = x + 128 + ((sinvalue * AMP) >> FRACBITS) + ((sinvalue2 * AMP2) >> FRACBITS);
            sinvalue = finesine[(x * SWIRLFACTOR + leveltic * 3 + 700) & 8191];
            sinvalue2 = finesine[(y * SWIRLFACTOR2 + leveltic * 4 + 1200) & 8191];
            y1 = y + 128 + ((sinvalue * AMP) >> FRACBITS) + ((sinvalue2 * AMP2) >> FRACBITS);

            offset[(y << 6) + x] = ((y1 & 63) << 6) + (x1 & 63);
        }

    normalflat = lumpinfo[firstflat + flatnum]->cache;

    for (int i = 0; i < 4096; i++)
        distortedflat[i] = normalflat[offset[i]];

    return distortedflat;
}

//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes(void)
{
    for (visplane_t *pl = visplanes; pl < lastvisplane; pl++)
        if (pl->left <= pl->right)
        {
            int picnum = pl->picnum;

            // sky flat
            if (picnum == skyflatnum || (picnum & PL_SKYFLAT))
            {
                int             texture;
                int             offset = skycolumnoffset >> FRACBITS;
                angle_t         flip = 0;
                const rpatch_t  *tex_patch;

                // killough 10/98: allow skies to come from sidedefs.
                // Allows scrolling and/or animated skies, as well as
                // arbitrary multiple skies per level without having
                // to use info lumps.
                angle_t         an = viewangle;

                if (picnum & PL_SKYFLAT)
                {
                    // Sky Linedef
                    const line_t    *l = lines + (picnum & ~PL_SKYFLAT);

                    // Sky transferred from first sidedef
                    const side_t    *s = sides + *l->sidenum;

                    // Texture comes from upper texture of reference sidedef
                    texture = texturetranslation[s->toptexture];

                    // Horizontal offset is turned into an angle offset,
                    // to allow sky rotation as well as careful positioning.
                    // However, the offset is scaled very small, so that it
                    // allows a long-period of sky rotation.
                    an += s->textureoffset;

                    // Vertical offset allows careful sky positioning.
                    dc_texturemid = s->rowoffset - 28 * FRACUNIT;

                    dc_texheight = textureheight[texture] >> FRACBITS;

                    if (canmouselook)
                        dc_texturemid = dc_texturemid * dc_texheight / SKYSTRETCH_HEIGHT;

                    // We sometimes flip the picture horizontally.

                    // DOOM always flipped the picture, so we make it optional,
                    // to make it easier to use the new feature, while to still
                    // allow old sky textures to be used.
                    flip = (l->special == TransferSkyTextureToTaggedSectors_Flipped ? 0u : ~0u);
                }
                else
                {
                    // Normal DOOM sky, only one allowed per level
                    texture = skytexture;
                    dc_texheight = textureheight[texture] >> FRACBITS;
                    dc_texturemid = skytexturemid;
                }

                dc_colormap[0] = (viewplayer->fixedcolormap == INVERSECOLORMAP && r_textures ? fixedcolormap : fullcolormap);
                dc_iscale = skyiscale;
                tex_patch = R_CacheTextureCompositePatchNum(texture);

                for (int x = pl->left; x <= pl->right; x++)
                {
                    dc_yl = pl->top[x];
                    dc_yh = pl->bottom[x];

                    if (dc_yl <= dc_yh)
                    {
                        dc_x = x;
                        dc_source = R_GetTextureColumn(tex_patch, (((an + xtoviewangle[x]) ^ flip) >> ANGLETOSKYSHIFT) + offset);
                        skycolfunc();
                    }
                }
            }
            else
            {
                // regular flat
                ds_source = (terraintypes[picnum] != SOLID && r_liquid_swirl ? R_DistortedFlat(picnum) :
                    lumpinfo[firstflat + flattranslation[picnum]]->cache);

                R_MakeSpans(pl);
            }
        }
}
