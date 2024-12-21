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

#pragma once

#include "r_defs.h"
#include "r_patch.h"
#include "r_state.h"

#define LOOKDIRMAX  100
#define LOOKDIRS    (LOOKDIRMAX * 2 + 1)

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef struct
{
    short           originx, originy;
    short           patch;
    short           stepdir;    // unused
    short           colormap;   // unused
} PACKEDATTR mappatch_t;

//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct
{
    char            name[8];
    char            pad2[4];    // unused
    short           width;
    short           height;
    char            pad[4];     // unused
    short           patchcount;
    mappatch_t      patches[1];
} PACKEDATTR maptexture_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct
{
    // Block origin (always UL),
    // which has already accounted
    // for the internal origin of the patch.
    short           originx, originy;
    int             patch;
} texpatch_t;

// A texture_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.

typedef struct
{
    // Keep name for switch changing, etc.
    char            name[8];
    short           width;
    short           height;

    // Index in textures list
    int             index;

    // Next in hash table chain
    int             next;

    unsigned int    widthmask;

    // All the patches[patchcount] are drawn back to front into the cached texture.
    short           patchcount;
    texpatch_t      patches[1];
} texture_t;

// Retrieve column data for span blitting.
byte *R_GetTextureColumn(const rpatch_t *texpatch, int col);

// I/O, setting up the stuff.
void R_InitData(void);
void R_PrecacheLevel(void);

// Retrieval.
// Floor/ceiling opaque texture tiles, lookup by name. For animation?
int R_FlatNumForName(const char *name);
int R_CheckFlatNumForName(const char *name);

// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName(const char *name);
int R_CheckTextureNumForName(const char *name);

int R_ColormapNumForName(const char *name); // killough 04/04/98

extern char         berserk[64];
extern byte         grays[256];
extern texture_t    **textures;
extern bool         anybossdeath;
extern bool         fixspriteoffsets;
extern bool         incompatiblepalette;
extern bool         suppresswarnings;
extern int          numflats;
extern int          numspritelumps;
