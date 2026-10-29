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

#include "doomdef.h"

#define VX_MAXFRAMES        29
#define VX_MINZ             (4 * FRACUNIT)
#define VX_MAXDISTANCE      (1024 * FRACUNIT)
#define VX_SHADOWDISTANCE   (768 * FRACUNIT)
#define VX_SPINTICS         (TICRATE * 4)
#define VX_FASTSPINTICS     TICRATE
#define VX_HOVERZ           (4 * FRACUNIT)

enum
{
    F_LEFT   = 0x01,
    F_RIGHT  = 0x02,
    F_BACK   = 0x04,
    F_FRONT  = 0x08,
    F_TOP    = 0x10,
    F_BOTTOM = 0x20
};

typedef struct
{
    int                 xsize, ysize, zsize;
    fixed_t             xpivot, ypivot, zpivot;
    int                 *offsets;
    byte                *data;
} voxel_t;

typedef struct
{
    voxel_t             *model;
    angle_t             angle;
    fixed_t             tlx, tly;
    fixed_t             c;
    fixed_t             s;
    fixed_t             liquidclipz;
    fixed_t             rockx, rocky;
    bool                liquidclip;
    bool                shadow;
    bool                percolumnlighting;
    lighttable_t        **lightrows;
    lighttable_t        **nextlightrows;
    const byte          *brightmap;
    const byte          *tint;
} visvoxel_t;

typedef struct
{
    fixed_t x, y;
} vxpoint_t;

typedef struct
{
    fixed_t             depth;
    int                 owner;
    unsigned int        frame;
    byte                under;
} voxeldepth_t;

typedef struct
{
    const lighttable_t  *colormap;
    const lighttable_t  *nextcolormap;
    const lighttable_t  *sectorcolormap;
    lighttable_t        *const *rows;
    lighttable_t        *const *nextrows;
} vxlighting_t;

void VX_Init(void);
void VX_ClearVoxels(void);
bool VX_SpriteHasVoxels(const int spr);
bool VX_ProjectVoxel(mobj_t *thing, fixed_t gx, fixed_t gy, fixed_t gz);
void VX_DrawVoxel(const vissprite_t *spr);