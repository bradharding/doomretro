/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "c_console.h"
#include "doomstat.h"
#include "i_system.h"
#include "i_timer.h"
#include "p_local.h"
#include "r_local.h"
#include "r_sky.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAXVISPLANES    128                             // must be a power of 2

static visplane_t       *visplanes[MAXVISPLANES];       // killough
static visplane_t       *freetail;                      // killough
static visplane_t       **freehead = &freetail;         // killough
visplane_t              *floorplane;
visplane_t              *ceilingplane;

// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:
#define visplane_hash(picnum, lightlevel, height) \
    (((unsigned int)(picnum) * 3 + (unsigned int)(lightlevel) + \
    (unsigned int)(height) * 7) & (MAXVISPLANES - 1))

size_t                 maxopenings;
int                    *openings;                       // dropoff overflow
int                    *lastopening;                    // dropoff overflow

// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
int                     floorclip[SCREENWIDTH];         // dropoff overflow
int                     ceilingclip[SCREENWIDTH];       // dropoff overflow

// spanstart holds the start of a plane span
// initialized to 0 at start
static int              spanstart[SCREENHEIGHT];

// texture mapping
static lighttable_t     **planezlight;
static fixed_t          planeheight;

static fixed_t          xoffs, yoffs;                   // killough 2/28/98: flat offsets

fixed_t                 yslope[SCREENHEIGHT];
fixed_t                 distscale[SCREENWIDTH];

dboolean                r_liquid_swirl = r_liquid_swirl_default;

extern fixed_t          animatedliquiddiff;
extern dboolean         r_liquid_bob;

//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
static void R_MapPlane(int y, int x1, int x2)
{
    fixed_t     distance;
    int         dx, dy;

    if (y == centery)
        return;

    distance = FixedMul(planeheight, yslope[y]);

    dx = x1 - centerx;
    dy = ABS(centery - y);
    ds_xstep = FixedMul(viewsin, planeheight) / dy;
    ds_ystep = FixedMul(viewcos, planeheight) / dy;

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
    int i;

    // opening/clipping determination
    for (i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    for (i = 0; i < MAXVISPLANES; i++)  // new code -- killough
        for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead;)
            freehead = &(*freehead)->next;

    lastopening = openings;
}

// New function, by Lee Killough
static visplane_t *new_visplane(unsigned hash)
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
    visplane_t          *check;
    unsigned int        hash;                                   // killough

    if (picnum == skyflatnum || (picnum & PL_SKYFLAT))          // killough 10/98
        height = lightlevel = 0;                // killough 7/19/98: most skies map together

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
    check->xoffs = xoffs;                                      // killough 2/28/98: Save offsets
    check->yoffs = yoffs;

    memset(check->top, SHRT_MAX, sizeof(check->top));

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

    for (x = intrl; x <= intrh && pl->top[x] == SHRT_MAX; x++);

    // [crispy] fix HOM if ceilingplane and floorplane are the same
    // visplane (e.g. both skies)
    if (!(pl == floorplane && markceiling && floorplane == ceilingplane) && x > intrh)
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
        new_pl->sector = pl->sector;
        new_pl->xoffs = pl->xoffs;      // killough 2/28/98
        new_pl->yoffs = pl->yoffs;
        pl = new_pl;
        pl->minx = start;
        pl->maxx = stop;
        memset(pl->top, SHRT_MAX, sizeof(pl->top));
    }

    return pl;
}

//
// R_MakeSpans
//
static void R_MakeSpans(visplane_t *pl)
{
    int x;

    for (x = pl->minx; x <= pl->maxx + 1; ++x)
    {
        unsigned short  t1 = pl->top[x - 1];
        unsigned short  b1 = pl->bottom[x - 1];
        unsigned short  t2 = pl->top[x];
        unsigned short  b2 = pl->bottom[x];

        for (; t1 < t2 && t1 <= b1; ++t1)
            R_MapPlane(t1, spanstart[t1], x - 1);
        for (; b1 > b2 && b1 >= t1; --b1)
            R_MapPlane(b1, spanstart[b1], x - 1);
        while (t2 < t1 && t2 <= b2)
            spanstart[t2++] = x;
        while (b2 > b1 && b2 >= t2)
            spanstart[b2--] = x;
    }
}

// Ripple Effect from Eternity Engine (r_ripple.cpp) by Simon Howard
#define AMP             2
#define AMP2            2
#define SPEED           40

// swirl factors determine the number of waves per flat width
// 1 cycle per 64 units
#define SWIRLFACTOR     (8192 / 64)

// 1 cycle per 32 units (2 in 64)
#define SWIRLFACTOR2    (8192 / 32)

static byte     *normalflat;
static byte     distortedflat[4096];

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
    int         i;
    int         leveltic = gametic;

    // Already swirled this one?
    if (leveltic == swirltic && lastflat == flatnum)
        return distortedflat;

    lastflat = flatnum;

    // built this tic?
    if (leveltic != swirltic && (!consoleactive || swirltic == -1) && !menuactive && !paused)
    {
        int     x, y;

        leveltic *= SPEED;
        for (x = 0; x < 64; ++x)
            for (y = 0; y < 64; ++y)
            {
                int     x1, y1;
                int     sinvalue, sinvalue2;

                sinvalue = (y * SWIRLFACTOR + leveltic * 5 + 900) & 8191;
                sinvalue2 = (x * SWIRLFACTOR2 + leveltic * 4 + 300) & 8191;
                x1 = x + 128 + ((finesine[sinvalue] * AMP) >> FRACBITS)
                    + ((finesine[sinvalue2] * AMP2) >> FRACBITS);

                sinvalue = (x * SWIRLFACTOR + leveltic * 3 + 700) & 8191;
                sinvalue2 = (y * SWIRLFACTOR2 + leveltic * 4 + 1200) & 8191;
                y1 = y + 128 + ((finesine[sinvalue] * AMP) >> FRACBITS)
                    + ((finesine[sinvalue2] * AMP2) >> FRACBITS);

                offset[(y << 6) + x] = ((y1 & 63) << 6) + (x1 & 63);
            }

        swirltic = gametic;
    }

    normalflat = W_CacheLumpNum(firstflat + flatnum, PU_LEVEL);

    for (i = 0; i < 4096; ++i)
        distortedflat[i] = normalflat[offset[i]];

    return distortedflat;
}

//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes(void)
{
    int i;

    for (i = 0; i < MAXVISPLANES; i++)
    {
        visplane_t      *pl;

        for (pl = visplanes[i]; pl; pl = pl->next)
            if (pl->minx <= pl->maxx)
            {
                int     picnum = pl->picnum;

                // sky flat
                if (picnum == skyflatnum || (picnum & PL_SKYFLAT))
                {
                    int         x;
                    int         texture;
                    int         offset;
                    angle_t     an, flip;

                    // killough 10/98: allow skies to come from sidedefs.
                    // Allows scrolling and/or animated skies, as well as
                    // arbitrary multiple skies per level without having
                    // to use info lumps.
                    an = viewangle;

                    if (picnum & PL_SKYFLAT)
                    {
                        // Sky Linedef
                        const line_t    *l = &lines[picnum & ~PL_SKYFLAT];

                        // Sky transferred from first sidedef
                        const side_t    *s = *l->sidenum + sides;

                        // Texture comes from upper texture of reference sidedef
                        texture = texturetranslation[s->toptexture];

                        // Horizontal offset is turned into an angle offset,
                        // to allow sky rotation as well as careful positioning.
                        // However, the offset is scaled very small, so that it
                        // allows a long-period of sky rotation.
                        an += s->textureoffset;

                        // Vertical offset allows careful sky positioning.
                        dc_texturemid = s->rowoffset - 28 * FRACUNIT;

                        // We sometimes flip the picture horizontally.
                        //
                        // DOOM always flipped the picture, so we make it optional,
                        // to make it easier to use the new feature, while to still
                        // allow old sky textures to be used.
                        flip = (l->special == TransferSkyTextureToTaggedSectors_Flipped ?
                            0u : ~0u);
                    }
                    else        // Normal DOOM sky, only one allowed per level
                    {
                        dc_texturemid = skytexturemid;  // Default y-offset
                        texture = skytexture;           // Default texture
                        flip = 0;                       // DOOM flips it
                    }

                    // Sky is always drawn full bright,
                    //  i.e. colormaps[0] is used.
                    // Because of this hack, sky is not affected
                    //  by INVUL inverse mapping.
                    dc_colormap = (fixedcolormap ? fixedcolormap : fullcolormap);

                    dc_texheight = textureheight[texture] >> FRACBITS;
                    dc_iscale = pspriteiscale;

                    offset = skycolumnoffset >> FRACBITS;

                    for (x = pl->minx; x <= pl->maxx; x++)
                    {
                        dc_yl = pl->top[x];
                        dc_yh = pl->bottom[x];

                        if (dc_yl <= dc_yh)
                        {
                            dc_x = x;
                            dc_source = R_GetColumn(texture, (((an + xtoviewangle[x]) ^ flip)
                                >> ANGLETOSKYSHIFT) + offset, false);
                            skycolfunc();
                        }
                    }
                }
                else
                {
                    // regular flat
                    dboolean    liquid = isliquid[picnum];
                    dboolean    swirling = (liquid && r_liquid_swirl);
                    int         lumpnum = firstflat + flattranslation[picnum];

                    ds_source = (swirling ? R_DistortedFlat(picnum) :
                        W_CacheLumpNum(lumpnum, PU_STATIC));

                    xoffs = pl->xoffs;  // killough 2/28/98: Add offsets
                    yoffs = pl->yoffs;
                    planeheight = ABS(pl->height - viewz);

                    if (liquid && pl->sector && r_liquid_bob && isliquid[pl->sector->floorpic])
                        planeheight -= animatedliquiddiff;

                    planezlight = zlight[BETWEEN(0, (pl->lightlevel >> LIGHTSEGSHIFT)
                        + extralight * LIGHTBRIGHT, LIGHTLEVELS - 1)];

                    pl->top[pl->minx - 1] = pl->top[pl->maxx + 1] = SHRT_MAX;

                    R_MakeSpans(pl);

                    if (!swirling)
                        W_ReleaseLumpNum(lumpnum);
                }
            }
    }
}
