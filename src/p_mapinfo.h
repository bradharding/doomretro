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

#include "doomtype.h"

typedef struct
{
    int type;
    int special;
    int tag;
} bossaction_t;

extern char         *mapinfolump;

void P_InitMapInfo(void);
bool P_ParseMapInfo(const char *scriptname);
bool P_MapInfoIsFromIWAD(void);

const char *P_GetMapAuthor(const int ep, const int map);
int P_GetNumBossActions(const int ep, const int map);
bossaction_t *P_GetBossAction(const int ep, const int map, const int i);
char *P_GetInterBackrop(const int ep, const int map);
int P_GetInterMusic(const int ep, const int map);
char *P_GetInterText(const int ep, const int map);
char *P_GetInterSecretText(const int ep, const int map);
char *P_GetLabel(const int ep, const int map);
bool P_GetMapCompatCorpseGibs(const int ep, const int map);
bool P_GetMapCompatFloorMove(const int ep, const int map);
bool P_GetMapCompatLight(const int ep, const int map);
bool P_GetMapCompatLimitPain(const int ep, const int map);
bool P_GetMapCompatNoPassOver(const int ep, const int map);
bool P_GetMapCompatStairs(const int ep, const int map);
bool P_GetMapCompatUseBlocking(const int ep, const int map);
bool P_GetMapCompatZombie(const int ep, const int map);
bool P_GetMapEndBunny(const int ep, const int map);
bool P_GetMapEndCast(const int ep, const int map);
bool P_GetMapEndGame(const int ep, const int map);
int P_GetMapEndPic(const int ep, const int map);
char *P_GetMapEnterAnim(const int ep, const int map);
int P_GetMapEnterPic(const int ep, const int map);
char *P_GetMapExitAnim(const int ep, const int map);
int P_GetMapExitPic(const int ep, const int map);
void P_GetMapLiquids(const int ep, const int map);
int P_GetMapMusic(const int ep, const int map);
char *P_GetMapMusicComposer(const int ep, const int map);
char *P_GetMapMusicTitle(const int ep, const int map);
char *P_GetMapName(const int ep, const int map);
int P_GetMapNext(const int ep, const int map);
bool P_GetMapNoFreelook(const int ep, const int map);
bool P_GetMapNoGradualLighting(const int ep, const int map);
bool P_GetMapNoJump(const int ep, const int map);
void P_GetMapNoLiquids(const int ep, const int map);
int P_GetMapPar(const int ep, const int map);
bool P_GetMapPistolStart(const int ep, const int map);
int P_GetMapSecretNext(const int ep, const int map);
int P_GetMapSky1Texture(const int ep, const int map);
float P_GetMapSky1ScrollDelta(const int ep, const int map);
int P_GetMapTitlePatch(const int ep, const int map);
int P_GetAllowMonsterTelefrags(const int ep, const int map);
bool P_IsSecret(const int ep, const int map);
