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
#include "info.h"
#include "r_defs.h"

typedef struct
{
    mobjtype_t      target;
    mobjtype_t      inflicter;
    mobjtype_t      source;

    mobjtype_t      barrelinflicter;
    short           floorpic;
    terraintype_t   terraintype;
    bool            crushed;
    bool            targetisplayer;
    bool            sourceisplayer;
    bool            targetfriendly;
    bool            sourcefriendly;
    bool            targetcorpse;

    bool            gibbed;
    bool            telefragged;

    weapontype_t    weapon;

    char            targetname[64];
    char            sourcename[64];
} obituaryinfo_t;

void C_BuildObituaryString(const int index);
void C_WriteObituary(mobj_t *target, mobj_t *inflicter, mobj_t *source, const bool gibbed,
    const bool telefragged);
