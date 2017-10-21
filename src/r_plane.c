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

#include "c_console.h"
#include "doomstat.h"
#include "m_config.h"
#include "p_local.h"
#include "r_sky.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAXVISPLANES    128                         // must be a power of 2

static visplane_t   *visplanes[MAXVISPLANES];       // killough
static visplane_t   *freetail;                      // killough
static visplane_t   **freehead = &freetail;         // killough
visplane_t          *floorplane;
visplane_t          *ceilingplane;

// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:
#define visplane_hash(picnum, lightlevel, height) (((unsigned int)(picnum) * 3 + (unsigned int)(lightlevel) \
            + (unsigned int)(height) * 7) & (MAXVISPLANES - 1))

int                *openings;                       // dropoff overflow
int                *lastopening;                    // dropoff overflow

// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
int                 floorclip[SCREENWIDTH];         // dropoff overflow
int                 ceilingclip[SCREENWIDTH];       // dropoff overflow

// spanstart holds the start of a plane span
// initialized to 0 at start
static int          spanstart[SCREENHEIGHT];

// texture mapping
static lighttable_t **planezlight;
static fixed_t      planeheight;

static fixed_t      xoffs, yoffs;                   // killough 2/28/98: flat offsets

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
    static fixed_t  cachedxstep[SCREENHEIGHT];
    static fixed_t  cachedystep[SCREENHEIGHT];
    fixed_t         distance;
    int             dx;

    if (centery == y)
        return;

    if (planeheight != cachedheight[y])
    {
        int dy = ABS(centery - y);

        distance = FixedMul(planeheight, yslope[y]);
        ds_xstep = FixedMul(viewsin, planeheight) / dy;
        ds_ystep = FixedMul(viewcos, planeheight) / dy;

        cachedheight[y] = planeheight;
        cacheddistance[y] = distance;
        cachedxstep[y] = ds_xstep;
        cachedystep[y] = ds_ystep;
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }

    dx = x1 - centerx;
    ds_xfrac = viewx + xoffs + FixedMul(viewcos, distance) + dx * ds_xstep;
    ds_yfrac = -viewy + yoffs - FixedMul(viewsin, distance) + dx * ds_ystep;

    ds_colormap = (fixedcolormap ? fixedcolormap :
        planezlight[BETWEEN(0, distance >> LIGHTZSHIFT, MAXLIGHTZ - 1)]);

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
    // opening/clipping determination
    for (int i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    // texture calculation
    memset(cachedheight, 0, sizeof(cachedheight));

    for (int i = 0; i < MAXVISPLANES; i++)  // new code -- killough
        for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead;)
            freehead = &(*freehead)->next;

    lastopening = openings;
}

// New function, by Lee Killough
static visplane_t *new_visplane(unsigned int hash)
{
    visplane_t  *check = freetail;

    if (!check)
        check = calloc(1, sizeof(*check));
    else if (!(freetail = freetail->next))
        freehead = &freetail;

    check->next = visplanes[hash];
    visplanes[hash] = check;
    return check;
}

//
// R_FindPlane
//
visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel, fixed_t xoffs, fixed_t yoffs)
{
    visplane_t      *check;
    unsigned int    hash;                                       // killough

    if (picnum == skyflatnum || (picnum & PL_SKYFLAT))          // killough 10/98
    {
        height = 0;                                             // killough 7/19/98: most skies map together
        lightlevel = 0;
    }

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum, lightlevel, height);

    for (check = visplanes[hash]; check; check = check->next)   // killough
        if (height == check->height && picnum == check->picnum && lightlevel == check->lightlevel
            && xoffs == check->xoffs && yoffs == check->yoffs)
            return check;

    check = new_visplane(hash);                                 // killough

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = viewwidth;
    check->maxx = -1;

    if (!(picnum & PL_SKYFLAT) && isliquid[picnum] && r_liquid_current && !xoffs && !yoffs)
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

    for (x = intrl; x <= intrh && pl->top[x] == USHRT_MAX; x++);

    // [crispy] fix HOM if ceilingplane and floorplane are the same
    // visplane (e.g. both skies)
    if (pl != floorplane && !markceiling && floorplane != ceilingplane && x > intrh)
    {
        pl->minx = unionl;
        pl->maxx = unionh;
    }
    else
    {
        unsigned int    hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
        visplane_t      *new_pl = new_visplane(hash);

        new_pl->height = pl->height;
        new_pl->picnum = pl->picnum;
        new_pl->lightlevel = pl->lightlevel;
        new_pl->xoffs = pl->xoffs;      // killough 2/28/98
        new_pl->yoffs = pl->yoffs;
        pl = new_pl;
        pl->minx = start;
        pl->maxx = stop;
        memset(pl->top, USHRT_MAX, sizeof(pl->top));
    }

    return pl;
}

//
// R_MakeSpans
//
static void R_MakeSpans(visplane_t *pl)
{
    xoffs = pl->xoffs;
    yoffs = pl->yoffs;
    planeheight = ABS(pl->height - viewz);

    planezlight = zlight[BETWEEN(0, (pl->lightlevel >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];

    pl->top[pl->minx - 1] = USHRT_MAX;
    pl->top[pl->maxx + 1] = USHRT_MAX;

    for (int x = pl->minx; x <= pl->maxx + 1; x++)
    {
        unsigned short  t1 = pl->top[x - 1];
        unsigned short  b1 = pl->bottom[x - 1];
        unsigned short  t2 = pl->top[x];
        unsigned short  b2 = pl->bottom[x];

        for (; t1 < t2 && t1 <= b1; t1++)
            R_MapPlane(t1, spanstart[t1], x - 1);

        for (; b1 > b2 && b1 >= t1; b1--)
            R_MapPlane(b1, spanstart[b1], x - 1);

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
// Generates a distorted flat from a normal one using a two-dimensional
// sine wave pattern.
//
static byte *R_DistortedFlat(int flatnum)
{
    static int  lastflat = -1;
    static int  swirltic = -1;
    static int  offset[4096];
    static byte *normalflat;
    static byte distortedflat[4096];
    int         leveltic = activetic;

    // Already swirled this one?
    if (leveltic == swirltic && lastflat == flatnum)
        return distortedflat;

    lastflat = flatnum;

    // built this tic?
    if (leveltic != swirltic && (!consoleactive || swirltic == -1) && !menuactive && !paused)
    {
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

        swirltic = activetic;
    }

    normalflat = W_CacheLumpNum(firstflat + flatnum);

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
    for (int i = 0; i < MAXVISPLANES; i++)
        for (visplane_t *pl = visplanes[i]; pl; pl = pl->next)
            if (pl->minx <= pl->maxx)
            {
                int picnum = pl->picnum;

                // sky flat
                if (picnum == skyflatnum || (picnum & PL_SKYFLAT))
                {
                    int             texture;
                    int             offset;
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
                        //
                        // DOOM always flipped the picture, so we make it optional,
                        // to make it easier to use the new feature, while to still
                        // allow old sky textures to be used.
                        flip = (l->special == TransferSkyTextureToTaggedSectors_Flipped ? 0u : ~0u);
                    }
                    else        // Normal DOOM sky, only one allowed per level
                    {
                        texture = skytexture;                   // Default texture
                        dc_texheight = textureheight[texture] >> FRACBITS;
                        dc_texturemid = skytexturemid;
                    }

                    dc_colormap = (viewplayer->fixedcolormap == INVERSECOLORMAP ? fixedcolormap : fullcolormap);
                    dc_iscale = skyiscale;
                    tex_patch = R_CacheTextureCompositePatchNum(texture);
                    offset = skycolumnoffset >> FRACBITS;

                    for (int x = pl->minx; x <= pl->maxx; x++)
                    {
                        dc_yl = pl->top[x];
                        dc_yh = pl->bottom[x];

                        if (dc_yl <= dc_yh)
                        {
                            dc_x = x;
                            dc_source = R_GetTextureColumn(tex_patch,
                                (((an + xtoviewangle[x]) ^ flip) >> ANGLETOSKYSHIFT) + offset);
                            skycolfunc();
                        }
                    }

                    R_UnlockTextureCompositePatchNum(texture);
                }
                else
                {
                    // regular flat
                    if (isliquid[picnum] && r_liquid_swirl)
                    {
                        ds_source = R_DistortedFlat(picnum);
                        R_MakeSpans(pl);
                    }
                    else
                    {
                        lumpindex_t lumpnum = firstflat + flattranslation[picnum];

                        ds_source = W_CacheLumpNum(lumpnum);
                        R_MakeSpans(pl);
                        W_UnlockLumpNum(lumpnum);
                    }
                }
            }
}
