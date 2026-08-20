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
#include "c_console.h"
#include "doomstat.h"
#include "m_config.h"
#include "m_menu.h"
#include "r_sky.h"
#include "tables.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAXVISPLANES    1024                    // must be a power of 2

// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:
#define visplane_hash(picnum, lightlevel, height, colormap) \
    ((unsigned int)((picnum) * 3 + (lightlevel) + (height) * 7 + (colormap) * 11) & (MAXVISPLANES - 1))

static visplane_t   *visplanes[MAXVISPLANES];   // killough
static visplane_t   *freetail;                  // killough
static visplane_t   **freehead = &freetail;     // killough
visplane_t          *floorplane;
visplane_t          *ceilingplane;

fixed_t             planenum;

int                 openings[MAXOPENINGS];
int                 *lastopening;               // dropoff overflow

// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
int                 floorclip[MAXWIDTH];        // dropoff overflow
int                 ceilingclip[MAXWIDTH];      // dropoff overflow

// texture mapping
static lighttable_t **planezlight;
static fixed_t      planeheight;

static fixed_t      xoffset, yoffset;           // killough 02/28/98: flat offsets

static angle_t      rotation;

static fixed_t      angle_sin;
static fixed_t      angle_cos;
static fixed_t      viewx_trans;
static fixed_t      viewy_trans;

fixed_t             *yslope;
fixed_t             yslopes[PITCHES][MAXHEIGHT];

static fixed_t      cachedheight[MAXHEIGHT];

static angle_t      *xtoskyangle;

static byte         **texflatcache;

static byte *R_GetTexFlat(const int texnum, const rpatch_t *patch)
{
    if (!texflatcache)
        texflatcache = Z_Calloc(numtextures, sizeof(byte *), PU_LEVEL, (void **)&texflatcache);

    if (!texflatcache[texnum])
    {
        const int   w = patch->width;
        const int   h = patch->height;
        byte        *buf = Z_Malloc((size_t)w * h, PU_LEVEL, NULL);

        for (int x = 0; x < w; x++)
        {
            const byte  *col = R_GetTextureColumn(patch, x);

            for (int y = 0; y < h; y++)
                buf[y * w + x] = col[y];
        }

        texflatcache[texnum] = buf;
    }

    return texflatcache[texnum];
}

//
// R_MapPlane
//
static void R_MapPlane(const int y, const int x1)
{
    static fixed_t  cacheddistance[MAXHEIGHT];
    static fixed_t  cachedanglecosdistance[MAXHEIGHT];
    static fixed_t  cachedanglesindistance[MAXHEIGHT];
    static fixed_t  cachedxstep[MAXHEIGHT];
    static fixed_t  cachedystep[MAXHEIGHT];
    static float    cachedradiallightdistancebase[MAXHEIGHT];
    static float    cachedradiallightstep[MAXHEIGHT];
    static float    cachedradiallightstepstep[MAXHEIGHT];
    static angle_t  cachedangle[MAXHEIGHT];
    static int      cachedcentery[MAXHEIGHT];
    fixed_t         anglecosdistance;
    fixed_t         anglesindistance;
    float           radiallightdistancebase;
    int             dx;

    if (planeheight != cachedheight[y] || rotation != cachedangle[y] || centery != cachedcentery[y])
    {
        // SoM: because centery is an actual row of pixels (and it isn't really the
        // center row because there are an even number of rows) some corrections need
        // to be made depending on where the row lies relative to the centery row.
        const fixed_t   dy = (ABS(centery - y) << FRACBITS) + (y < centery ? -FRACUNIT : FRACUNIT) / 2;

        cachedheight[y] = planeheight;
        cachedangle[y] = rotation;
        cachedcentery[y] = centery;
        ds_z = cacheddistance[y] = FixedMul(planeheight, FixedDiv(planenum, dy));
        anglecosdistance = cachedanglecosdistance[y] = FixedMul(angle_cos, ds_z);
        anglesindistance = cachedanglesindistance[y] = FixedMul(angle_sin, ds_z);
        ds_xstep = cachedxstep[y] = (fixed_t)((int64_t)angle_sin * planeheight / dy);
        ds_ystep = cachedystep[y] = (fixed_t)((int64_t)angle_cos * planeheight / dy);
        radiallightdistancebase = cachedradiallightdistancebase[y] = (float)ds_z * ds_z;
        ds_radiallightstep = cachedradiallightstep[y] = (float)ds_xstep * ds_xstep + (float)ds_ystep * ds_ystep;
        ds_radiallightstepstep = cachedradiallightstepstep[y] = 2.0f * ds_radiallightstep;
    }
    else
    {
        ds_z = cacheddistance[y];
        anglecosdistance = cachedanglecosdistance[y];
        anglesindistance = cachedanglesindistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
        radiallightdistancebase = cachedradiallightdistancebase[y];
        ds_radiallightstep = cachedradiallightstep[y];
        ds_radiallightstepstep = cachedradiallightstepstep[y];
    }

    dx = x1 - centerx;
    ds_xfrac = viewx_trans + anglecosdistance + dx * ds_xstep;
    ds_yfrac = viewy_trans - anglesindistance + dx * ds_ystep;
    ds_radiallightdistance = radiallightdistancebase + (float)dx * (float)dx * ds_radiallightstep;
    ds_radiallightdistancestep = (2.0f * (float)dx + 1.0f) * ds_radiallightstep;
    ds_y = y;
    ds_x1 = x1;

    if (fixedcolormap)
    {
        ds_colormap[0] = ds_colormap[1] = fixedcolormap;

        if (ds_flatwidth == 64 && ds_flatheight == 64)
            altspanfunc64();
        else
            altspanfunc();
    }
    else
    {
        ds_colormap[0] = planezlight[BETWEEN(0, ds_z >> LIGHTZSHIFT, MAXLIGHTZ - 1)];

        if (r_ditheredlighting)
        {
            if (r_radiallighting)
            {
                if (ds_flatwidth == 64 && ds_flatheight == 64)
                {
                    if (ds_brightmap)
                        bmapspanfunc64();
                    else
                        spanfunc64();
                }
                else
                {
                    if (ds_brightmap)
                        bmapspanfunc();
                    else
                        spanfunc();
                }

                return;
            }

            ds_colormap[1] = planezlight[BETWEEN(0, (ds_z >> LIGHTZSHIFT) + 1, MAXLIGHTZ - 1)];

            if (ds_colormap[0] == ds_colormap[1])
            {
                if (ds_flatwidth == 64 && ds_flatheight == 64)
                {
                    if (ds_brightmap)
                        altbmapspanfunc64();
                    else
                        altspanfunc64();
                }
                else
                {
                    if (ds_brightmap)
                        altbmapspanfunc();
                    else
                        altspanfunc();
                }
            }
            else
            {
                ds_z = ((ds_z >> 12) & 255);

                if (ds_flatwidth == 64 && ds_flatheight == 64)
                {
                    if (ds_brightmap)
                        bmapspanfunc64();
                    else
                        spanfunc64();
                }
                else
                {
                    if (ds_brightmap)
                        bmapspanfunc();
                    else
                        spanfunc();
                }
            }
        }
        else if (ds_flatwidth == 64 && ds_flatheight == 64)
        {
            if (ds_brightmap)
                bmapspanfunc64();
            else
                spanfunc64();
        }
        else if (ds_brightmap)
            bmapspanfunc();
        else
            spanfunc();
    }
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

    // [PN] Optimize loop by avoiding unnecessary assignments and checks.
    // Only process non-null visplanes and simplify inner loop performance.
    for (int i = 0; i < MAXVISPLANES; i++)
        if (visplanes[i])
        {
            *freehead = visplanes[i];
            visplanes[i] = NULL;

            while (*freehead)
                freehead = &(*freehead)->next;
        }

    lastopening = openings;

    // texture calculation
    memset(cachedheight, 0, viewheight * sizeof(*cachedheight));
}

// New function, by Lee Killough
static visplane_t *new_visplane(const unsigned int hash)
{
    visplane_t  *check = freetail;

    if (!check)
        check = calloc(1, sizeof(*check));
    else if (!(freetail = freetail->next))
        freehead = &freetail;

    if (check)
    {
        check->next = visplanes[hash];
        visplanes[hash] = check;
    }

    return check;
}

//
// R_FindPlane
//
visplane_t *R_FindPlane(fixed_t height, const int picnum, int lightlevel,
    const fixed_t x, const fixed_t y, const int colormap, const angle_t angle)
{
    visplane_t      *check;
    unsigned int    hash;

    if (picnum == skyflatnum || (picnum & PL_SKYFLAT))
    {
        // killough 7/19/98: most skies map together
        lightlevel = 0;

        // haleyjd 05/06/08: but not all. If height > viewz, set height to 1
        // instead of 0, to keep ceilings mapping with ceilings, and floors mapping
        // with floors.
        height = (height > viewz);
    }

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum, lightlevel, height, colormap);

    for (check = visplanes[hash]; check; check = check->next)
        if (height == check->height && picnum == check->picnum && lightlevel == check->lightlevel
            && x == check->xoffset && y == check->yoffset && colormap == check->colormap
            && angle == check->angle)
            return check;

    check = new_visplane(hash);

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->xoffset = x;
    check->yoffset = y;
    check->left = viewwidth;
    check->right = -1;
    check->modified = false;
    check->colormap = colormap;
    check->angle = angle;

    memset(check->top, USHRT_MAX, viewwidth * sizeof(*check->top));

    return check;
}

//
// R_DupPlane
//
visplane_t *R_DupPlane(const visplane_t *pl, const int start, const int stop)
{
    visplane_t  *new_pl = new_visplane(visplane_hash(pl->picnum, pl->lightlevel, pl->height, pl->colormap));

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->xoffset = pl->xoffset;
    new_pl->yoffset = pl->yoffset;
    new_pl->left = start;
    new_pl->right = stop;
    new_pl->modified = false;
    new_pl->colormap = pl->colormap;
    new_pl->angle = pl->angle;

    memset(new_pl->top, USHRT_MAX, viewwidth * sizeof(*new_pl->top));

    return new_pl;
}

//
// R_CheckPlane
//
visplane_t *R_CheckPlane(visplane_t *pl, const int start, const int stop)
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

    if (x > intrh)
    {
        pl->left = unionl;
        pl->right = unionh;
        return pl;
    }

    // make a new visplane
    return R_DupPlane(pl, start, stop);
}

//
// R_MakeSpans
//
static void R_MakeSpans(visplane_t *pl)
{
    // spanstart holds the start of a plane span
    // initialized to 0 at start
    static int  spanstart[MAXHEIGHT];
    const int   stop = pl->right + 1;

    if (!(pl->picnum & PL_TEXFLAT) && terraintypes[pl->picnum] >= LIQUID
        && r_liquid_current && !pl->xoffset && !pl->yoffset)
    {
        xoffset = animatedliquidxoffs;
        yoffset = animatedliquidyoffs;
    }
    else
    {
        xoffset = pl->xoffset;
        yoffset = pl->yoffset;
    }

    rotation = pl->angle;
    angle_sin = finesine[(viewangle + rotation) >> ANGLETOFINESHIFT];
    angle_cos = finecosine[(viewangle + rotation) >> ANGLETOFINESHIFT];

    if (!rotation)
    {
        viewx_trans = xoffset + viewx;
        viewy_trans = yoffset - viewy;
    }
    else
    {
        const fixed_t   sin = finesine[rotation >> ANGLETOFINESHIFT];
        const fixed_t   cos = finecosine[rotation >> ANGLETOFINESHIFT];

        viewx_trans = FixedMul(viewx + xoffset, cos) - FixedMul(viewy - yoffset, sin);
        viewy_trans = -(FixedMul(viewx + xoffset, sin) + FixedMul(viewy - yoffset, cos));
    }

    if ((planeheight = ABS(pl->height - viewz)) < FRACUNIT
        && !(pl->picnum & PL_TEXFLAT) && terraintypes[pl->picnum] >= LIQUID)
        planeheight = FRACUNIT;

    planezlight = zlight[BETWEEN(0, (pl->lightlevel >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
    ds_zlight = planezlight;
    pl->top[pl->left - 1] = USHRT_MAX;
    pl->top[stop] = USHRT_MAX;

    for (ds_x2 = pl->left; ds_x2 <= stop; ds_x2++)
    {
        unsigned int    t1 = pl->top[ds_x2 - 1];
        unsigned int    b1 = pl->bottom[ds_x2 - 1];
        unsigned int    t2 = pl->top[ds_x2];
        unsigned int    b2 = pl->bottom[ds_x2];

        for (; t1 < t2 && t1 <= b1; t1++)
            R_MapPlane(t1, spanstart[t1]);

        for (; b1 > b2 && b1 >= t1; b1--)
            R_MapPlane(b1, spanstart[b1]);

        while (t2 < t1 && t2 <= b2)
            spanstart[t2++] = ds_x2;

        while (b2 > b1 && b2 >= t2)
            spanstart[b2--] = ds_x2;
    }
}

// Ripple Effect from SMMU (r_ripple.cpp) by Simon Howard
#define SPEED           24

// swirl factors determine the number of waves per flat width
// 1 cycle per 64 units
#define SWIRLFACTOR     (FINEANGLES / 64)

// 1 cycle per 32 units (2 in 64)
#define SWIRLFACTOR2    (FINEANGLES / 32)

// Cache multiple flats
#define MAXCACHEDFLATS  16

static uint16_t         offsets[1024 * 4096];
static const uint16_t   *swirloffset;
static int              swirloffsettic = -1;

typedef struct
{
    byte    distortedflat[64 * 64];
    int     flatnum;
    int     lasttic;
} swirlcache_t;

static swirlcache_t swirlcache[MAXCACHEDFLATS];

//
// R_InitSwirlingFlats
// [BH] Moved to separate function and called at startup
//
void R_InitSwirlingFlats(void)
{
    for (int i = 0; i < 1024 * SPEED; i += SPEED)
    {
        int         xoffset1[64];
        int         xoffset2[64];
        int         yoffset1[64];
        int         yoffset2[64];
        uint16_t    *offset = &offsets[(i / SPEED) << 12];

        for (int j = 0; j < 64; j++)
        {
            xoffset1[j] = (finesine[(j * SWIRLFACTOR + i * 3 + 700) & FINEMASK] * 2) >> FRACBITS;
            xoffset2[j] = (finesine[(j * SWIRLFACTOR2 + i * 4 + 300) & FINEMASK] * 2) >> FRACBITS;
            yoffset1[j] = (finesine[(j * SWIRLFACTOR + i * 5 + 900) & FINEMASK] * 2) >> FRACBITS;
            yoffset2[j] = (finesine[(j * SWIRLFACTOR2 + i * 4 + 1200) & FINEMASK] * 2) >> FRACBITS;
        }

        for (int y = 0; y < 64; y++)
            for (int x = 0; x < 64; x++)
                offset[(y << 6) + x] = (((y + 128 + xoffset1[x] + yoffset2[y]) & 63) << 6)
                    + ((x + 128 + yoffset1[y] + xoffset2[x]) & 63);
    }

    for (int i = 0; i < MAXCACHEDFLATS; i++)
    {
        swirlcache[i].flatnum = -1;
        swirlcache[i].lasttic = -1;
    }

    swirloffset = offsets;
}

//
// R_SwirlingFlat
// Generates a distorted flat from a normal one using a two-dimensional sine wave pattern.
//
byte *R_SwirlingFlat(const int flatnum)
{
    swirlcache_t    *cache = NULL;
    int             oldestcache = 0;
    int             oldesttic = INT_MAX;

    for (int i = 0; i < MAXCACHEDFLATS; i++)
    {
        if (swirlcache[i].flatnum == flatnum)
        {
            cache = &swirlcache[i];
            break;
        }

        if (swirlcache[i].lasttic < oldesttic)
        {
            oldesttic = swirlcache[i].lasttic;
            oldestcache = i;
        }
    }

    if (!cache)
    {
        cache = &swirlcache[oldestcache];
        cache->flatnum = flatnum;
    }

    if ((updateswirl || cache->lasttic == -1) && cache->lasttic != animatedtic)
    {
        const uint16_t  *offset;
        const byte      *normalflat = lumpinfo[firstflat + flatnum]->cache;
        byte            *dest = cache->distortedflat;

        if (swirloffsettic != animatedtic)
        {
            swirloffsettic = animatedtic;
            swirloffset = &offsets[(animatedtic & 1023) << 12];
        }

        offset = swirloffset;
        cache->lasttic = animatedtic;

        for (int i = 0; i < 64 * 64; i += 8)
        {
            dest[i] = normalflat[offset[i]];
            dest[i + 1] = normalflat[offset[i + 1]];
            dest[i + 2] = normalflat[offset[i + 2]];
            dest[i + 3] = normalflat[offset[i + 3]];
            dest[i + 4] = normalflat[offset[i + 4]];
            dest[i + 5] = normalflat[offset[i + 5]];
            dest[i + 6] = normalflat[offset[i + 6]];
            dest[i + 7] = normalflat[offset[i + 7]];
        }
    }

    return cache->distortedflat;
}

static void DrawSkyTexture(visplane_t *pl, skytexture_t *skytexture, void func(void))
{
    const int       texture = R_TextureNumForName(skytexture->name);
    const angle_t   angle = viewangle + (skytexture->currentx << (ANGLETOSKYSHIFT - FRACBITS));

    dc_iscale = FixedMul(skyiscale, skytexture->scaley);

    dc_texturemid = (fixed_t)(skytexture->mid * (double)FRACUNIT) + skytexture->currenty;
    dc_texheight = textureheight[texture] >> FRACBITS;

    for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
        if ((dc_yl = pl->top[dc_x]) != USHRT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
        {
            // [Nugget] Sky projection
            if (r_skyprojection == r_skyprojection_cylindrical)
                dc_iscale = FixedMul(dc_iscale, finecosine[xtoviewangle[dc_x] >> ANGLETOFINESHIFT]);

            dc_source = R_GetTextureColumn(R_CacheTextureCompositePatchNum(texture),
                FixedMul((angle + xtoskyangle[dc_x]) >> ANGLETOSKYSHIFT, skytexture->scalex));

            func();
        }
}

//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes(void)
{
    xtoskyangle = (r_skyprojection == r_skyprojection_linear ? linearskyangle : xtoviewangle);
    dc_colormap[0] = (fixedcolormap && r_textures ? fixedcolormap : fullcolormap);
    dc_sectorcolormap = fullcolormap;

    for (int i = 0; i < MAXVISPLANES; i++)
        for (visplane_t *pl = visplanes[i]; pl; pl = pl->next)
            if (pl->modified && pl->left <= pl->right)
            {
                const int   picnum = pl->picnum;

                if (picnum == skyflatnum)
                {
                    dc_iscale = skyiscale;

                    if (sky && (!vanilla || sky->type != SkyType_Fire))
                    {
                        id24compatible = true;

                        if (sky->type == SkyType_Fire)
                        {
                            dc_texheight = FIREHEIGHT;
                            dc_texturemid = -28 * FRACUNIT;

                            for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
                                if ((dc_yl = pl->top[dc_x]) != USHRT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
                                {
                                    // [Nugget] Sky projection
                                    if (r_skyprojection == r_skyprojection_cylindrical)
                                        dc_iscale = FixedMul(skyiscale, finecosine[xtoviewangle[dc_x] >> ANGLETOFINESHIFT]);

                                    dc_source = R_GetFireColumn((viewangle + xtoskyangle[dc_x]) >> ANGLETOSKYSHIFT);

                                    skycolfunc();
                                }
                        }
                        else
                        {
                            DrawSkyTexture(pl, &sky->skytexture, skycolfunc);

                            if (sky->type == SkyType_WithForeground)
                                DrawSkyTexture(pl, &sky->foreground, &R_DrawSkyColumn);
                        }
                    }
                    else if (!skytexture)
                    {
                        // Sky texture not found, draw white
                        for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
                            if ((dc_yl = pl->top[dc_x]) != USHRT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
                                R_DrawSolidColorColumn();
                    }
                    else
                    {
                        // Normal DOOM sky, only one allowed per level
                        const int       texture = texturetranslation[skytexture];
                        const rpatch_t  *patch = R_CacheTextureCompositePatchNum(texture);

                        dc_texheight = textureheight[texture] >> FRACBITS;
                        dc_texturemid = skytexturemid;

                        for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
                            if ((dc_yl = pl->top[dc_x]) != USHRT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
                            {
                                // [Nugget] Sky projection
                                if (r_skyprojection == r_skyprojection_cylindrical)
                                    dc_iscale = FixedMul(skyiscale, finecosine[xtoviewangle[dc_x] >> ANGLETOFINESHIFT]);

                                dc_source = R_GetTextureColumn(patch, (((viewangle + xtoskyangle[dc_x])
                                    / (1 << (ANGLETOSKYSHIFT - FRACBITS))) + skycolumnoffset) / FRACUNIT);

                                skycolfunc();
                            }
                    }
                }
                else if ((picnum & PL_FLATMAPPING) == PL_FLATMAPPING)
                {
                    const int       texture = (picnum & ~PL_FLATMAPPING);
                    const rpatch_t  *patch = R_CacheTextureCompositePatchNum(texture);

                    dc_texheight = textureheight[texture] >> FRACBITS;
                    dc_texturemid = skytexturemid;

                    for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
                        if ((dc_yl = pl->top[dc_x]) != USHRT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
                        {
                            // [Nugget] Sky projection
                            if (r_skyprojection == r_skyprojection_cylindrical)
                                dc_iscale = FixedMul(skyiscale, finecosine[xtoviewangle[dc_x] >> ANGLETOFINESHIFT]);

                            dc_source = R_GetTextureColumn(patch, (((viewangle + xtoskyangle[dc_x])
                                / (1 << (ANGLETOSKYSHIFT - FRACBITS))) + skycolumnoffset) / FRACUNIT);

                            R_DrawWallColumn();
                        }
                }
                else if (picnum & PL_SKYFLAT)
                {
                    // killough 10/98: allow skies to come from sidedefs.
                    // Allows scrolling and/or animated skies, as well as
                    // arbitrary multiple skies per level without having
                    // to use info lumps.

                    // Sky linedef
                    const line_t    *line = lines + (picnum & ~PL_SKYFLAT);

                    // Sky transferred from first sidedef
                    const side_t    *side = sides + *line->sidenum;

                    if (side->missingtoptexture)
                    {
                        for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
                            if ((dc_yl = pl->top[dc_x]) != USHRT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
                                R_DrawSolidColorColumn();
                    }
                    else
                    {
                        // Texture comes from upper texture of reference sidedef
                        const int       texture = texturetranslation[side->toptexture];

                        // Horizontal offset is turned into an angle offset,
                        // to allow sky rotation as well as careful positioning.
                        // However, the offset is scaled very small, so that it
                        // allows a long-period of sky rotation.
                        const angle_t   angle = viewangle + side->textureoffset;

                        angle_t         flip = 0U;
                        const rpatch_t  *patch = R_CacheTextureCompositePatchNum(texture);

                        // Vertical offset allows careful sky positioning.
                        dc_texturemid = side->rowoffset - 28 * FRACUNIT;

                        dc_texheight = textureheight[texture] >> FRACBITS;

                        if (canfreelook)
                            dc_texturemid = dc_texturemid * dc_texheight / skystretchheight;

                        // We sometimes flip the picture horizontally.

                        // DOOM always flipped the picture, so we make it optional,
                        // to make it easier to use the new feature, while to still
                        // allow old sky textures to be used.
                        if (line->special != TransferSkyTextureToTaggedSectors_Flipped)
                            flip = ~0U;

                        for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
                            if ((dc_yl = pl->top[dc_x]) != USHRT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
                            {
                                // [Nugget] Sky projection
                                dc_iscale = (r_skyprojection == r_skyprojection_cylindrical ?
                                    FixedMul(skyiscale, finecosine[xtoviewangle[dc_x] >> ANGLETOFINESHIFT]) :
                                    skyiscale);
                                dc_source = R_GetTextureColumn(patch,
                                    ((((angle + xtoskyangle[dc_x]) ^ flip)
                                        / (1 << (ANGLETOSKYSHIFT - FRACBITS))) + skycolumnoffset) / FRACUNIT);

                                skycolfunc();
                            }
                    }
                }
                else if (picnum & PL_TEXFLAT)
                {
                    // wall texture drawn as a tiled floor/ceiling flat
                    const int       texnum = (picnum & ~PL_TEXFLAT);
                    const rpatch_t  *patch = R_CacheTextureCompositePatchNum(texnum);

                    ds_source = R_GetTexFlat(texnum, patch);
                    ds_flatwidth = patch->width;
                    ds_flatheight = patch->height;
                    ds_brightmap = NULL;
                    ds_sectorcolormap = (pl->colormap && !ISINVULNERABILITYCOLORMAP(viewplayer->fixedcolormap) ?
                        colormaps[pl->colormap] : nocolormap);

                    R_MakeSpans(pl);
                }
                else
                {
                    // regular flat
                    const int   flatnum = flattranslation[picnum] - firstflat;

                    if (terraintypes[picnum] >= LIQUID && r_liquid_swirl)
                    {
                        ds_source = R_SwirlingFlat(picnum);
                        ds_flatwidth = 64;
                        ds_flatheight = 64;
                    }
                    else
                    {
                        lumpinfo_t  *lump = lumpinfo[firstflat + flatnum];
                        byte        *data = lump->cache;
                        const int   size = lump->size;
                        bool        headered = false;

                        if (size > 4)
                        {
                            const int   width = (data[0] | (data[1] << 8));
                            const int   height = (data[2] | (data[3] << 8));

                            if (width > 0 && height > 0 && width * height + 4 == size)
                            {
                                ds_flatwidth = width;
                                ds_flatheight = height;
                                ds_source = data + 4;
                                headered = true;
                            }
                        }

                        if (!headered)
                        {
                            int side = 1;

                            while (side * side < size)
                                side++;

                            ds_flatwidth = side;
                            ds_flatheight = side;
                            ds_source = data;
                        }
                    }

                    ds_brightmap = (usebrightmaps ? flatbrightmap[flatnum] : NULL);
                    ds_sectorcolormap = (pl->colormap && !ISINVULNERABILITYCOLORMAP(viewplayer->fixedcolormap) ?
                        colormaps[pl->colormap] : nocolormap);

                    R_MakeSpans(pl);
                }
            }
}
