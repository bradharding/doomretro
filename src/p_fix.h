/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "m_fixed.h"
#include "tables.h"

#define DEFAULT 0x7FFF
#define REMOVE  0

#define E2M2    (gamemission == doom && gameepisode == 2 && gamemap == 2 && canmodify)
#define E2M7    (gamemission == doom && gameepisode == 2 && gamemap == 7 && canmodify)
#define E4M3    (gamemission == doom && gameepisode == 4 && gamemap == 3 && canmodify)
#define MAP04   (gamemission == doom2 && gamemap == 4 && canmodify)
#define MAP12   (gamemission == doom2 && gamemap == 12 && canmodify)

typedef struct
{
    int     mission;
    int     episode;
    int     map;
    int     vertex;
    int     oldx, oldy;
    int     newx, newy;
} vertexfix_t;

extern vertexfix_t  vertexfix[];

typedef struct
{
    int     mission;
    int     episode;
    int     map;
    int     linedef;
    int     side;
    char    *toptexture;
    char    *middletexture;
    char    *bottomtexture;
    fixed_t offset;
    fixed_t rowoffset;
    int     flags;
    int     special;
    int     tag;
    bool    fixed;
} linefix_t;

extern linefix_t    linefix[];

typedef struct
{
    int     mission;
    int     episode;
    int     map;
    int     sector;
    char    *floorpic;
    char    *ceilingpic;
    int     floorheight;
    int     ceilingheight;
    int     special;
    int     tag;
} sectorfix_t;

extern sectorfix_t  sectorfix[];

typedef struct
{
    int     mission;
    int     episode;
    int     map;
    int     thing;
    int     type;
    int     oldx, oldy;
    int     newx, newy;
    angle_t angle;
    int     options;
} thingfix_t;

extern thingfix_t   thingfix[];
