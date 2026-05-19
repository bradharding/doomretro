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

#include "p_mapinfo.h"

#define MAXEPISODES 10

extern bool         canmodify;
extern bool         samelevel;
extern bool         secretmap;
extern bool         skipblstart;
extern bool         transferredsky;
extern const char   *linespecials[];

extern bool         compat_corpsegibs;
extern bool         compat_floormove;
extern bool         compat_light;
extern bool         compat_limitpain;
extern bool         compat_nopassover;
extern bool         compat_stairs;
extern bool         compat_useblocking;
extern bool         compat_zombie;
extern bool         nograduallighting;

extern char         mapnum[16];
extern char         maptitle[256];
extern char         mapnumandtitle[512];
extern char         automaptitle[512];

void P_SetupLevel(int ep, int map);
void P_MapName(int ep, int map);
fixed_t P_GetOffset(const vertex_t *v1, const vertex_t *v2);

// Called by startup code.
void P_Init(void);
