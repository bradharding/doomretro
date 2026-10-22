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

#pragma once

#define SHADOWHEIGHTPERDEPTH    (FRACUNIT * 2)
#define SHADOWSKEW              (r_radiallighting ? FRACUNIT * 3 / 2 : FRACUNIT)
#define SHADOWSWIRLSLACK        (FRACUNIT * 6)
#define SHADOWSOFTDISTANCE      (512 * FRACUNIT)

typedef struct
{
    fixed_t                 xfrac, yfrac;
    fixed_t                 xstep, ystep;
    fixed_t                 sin, cos;
} shadowswirl_t;

typedef struct
{
    short                   x1, x2;
    drawseg_t               *user;
} drawseg_xrange_item_t;

typedef struct
{
    drawseg_xrange_item_t   *items;
    int                     count;
} drawsegs_xrange_t;

typedef struct
{
    const rcolumn_t         *columns;
    const byte              *tint;
    const byte              *softtints[3];
    fixed_t                 top;
    fixed_t                 tz;
    fixed_t                 shift;
    fixed_t                 spread;
    int                     height;
    int                     width;
    int                     rows;
    int                     clipy;
    float                   center;
    float                   quad;
    float                   edgeshift;
    float                   halfwidth;
    float                   extent;
    float                   skew;
    float                   tilt;
    float                   rowtilt;
    float                   squash;
    float                   invsquash;
    bool                    flipped;
    bool                    soft;
    byte                    black;
} shadowcaster_t;

// Constant arrays used for psprite clipping
//  and initializing clipping.
extern int      negonearray[MAXWIDTH];
extern int      viewheightarray[MAXWIDTH];

// vars for R_DrawMaskedColumn
extern int      *mfloorclip;
extern int      *mceilingclip;
extern fixed_t  spryscale;
extern int64_t  sprtopscreen;

extern fixed_t  pspritescale;
extern fixed_t  pspriteiscale;

extern short    firstbloodsplatlump;
extern int      numbloodsplatlumps;

extern bool     allowwolfensteinss;

extern int      liquidrocktic;
extern bool     interpolatesprites;

void R_AddSprites(sector_t *sec, int lightlevel);
lighttable_t *R_GetSectorColormap(sector_t *sector);
void R_InitSpriteBottomOffsets(void);
void R_InitSprites(void);
void R_ClearSprites(void);
vissprite_t *R_NewVisSprite(void);
void R_ClipToDrawSegs(const int x1, const int x2, const fixed_t scale, const fixed_t gx,
    const fixed_t gy, int *sprtop, int *sprbottom, const bool rendermasked);
void R_DrawMasked(void);
void R_AddNearbySprites(sector_t *sec);
void R_DrawNearbySprites(void);
bool R_PrepareShadows(void);
void R_DrawSpanShadows(const int y, const int x1, const int x2, const fixed_t depth,
    const fixed_t planez, const shadowswirl_t *swirl);
