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

#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"

#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
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
    short       originx;
    short       originy;
    short       patch;
    short       stepdir;
    short       colormap;
} PACKEDATTR mappatch_t;

//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct
{
    char        name[8];
    int         masked;
    short       width;
    short       height;
    int         obsolete;
    short       patchcount;
    mappatch_t  patches[1];
} PACKEDATTR maptexture_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct
{
    // Block origin (allways UL),
    // which has allready accounted
    // for the internal origin of the patch.
    short       originx;
    short       originy;
    int         patch;
} texpatch_t;

// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.

typedef struct texture_s texture_t;

struct texture_s
{
    // Keep name for switch changing, etc.
    char        name[8];
    short       width;
    short       height;

    // Index in textures list
    int         index;

    // Next in hash table chain
    texture_t   *next;

    // All the patches[patchcount]
    //  are drawn back to front into the cached texture.
    short       patchcount;
    texpatch_t  patches[1];
};

// Retrieve column data for span blitting.
byte *R_GetColumn(int tex, int col);

// I/O, setting up the stuff.
void R_InitData(void);
void R_PrecacheLevel(void);

// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_FlatNumForName(char *name);
int R_CheckFlatNumForName(char *name);

// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName(char *name);
int R_CheckTextureNumForName(char *name);

#endif
