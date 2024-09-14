/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "doomtype.h"
#include "m_fixed.h"

typedef enum
{
    SkyType_Normal,
    SkyType_Fire,
    SkyType_WithForeground
} skytype_t;

typedef struct
{
    byte        *palette;
    int         updatetime;
    int         ticsleft;
} fire_t;

typedef struct
{
    const char  *name;
    double      mid;
    fixed_t     scrollx;
    fixed_t     currx;
    fixed_t     scrolly;
    fixed_t     curry;
    fixed_t     scalex;
    fixed_t     scaley;
} skytex_t;

typedef struct
{
    skytype_t   type;
    skytex_t    skytex;
    fire_t      fire;
    skytex_t    foreground;
} sky_t;

typedef struct
{
    const char  *flat;
    const char  *sky;
} flatmap_t;

typedef struct
{
    sky_t       *skies;
    flatmap_t   *flatmapping;
} skydefs_t;

skydefs_t *R_ParseSkyDefs(void);
