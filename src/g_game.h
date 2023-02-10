/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#pragma once

#include "d_event.h"
#include "i_video.h"
#include "w_file.h"

#define NUMKEYS         256

#define FORWARDMOVE0    0x19
#define FORWARDMOVE1    0x32

#define SIDEMOVE0       0x18
#define SIDEMOVE1       0x28

//
// GAME
//
void G_InitNew(skill_t skill, int ep, int map);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferredInitNew(skill_t skill, int ep, int map);

void G_DeferredLoadLevel(skill_t skill, int ep, int map);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel.
void G_LoadGame(char *name);

void G_DoLoadGame(void);
void G_DoLoadLevel(void);

// Called by M_Responder.
void G_SaveGame(int slot, char *description, char *name);

void G_ExitLevel(void);
void G_SecretExitLevel(void);

int G_GetParTime(void);

void G_WorldDone(void);

// Read current data from inputs and build a player movement command.
void G_BuildTiccmd(ticcmd_t *cmd);

void G_Ticker(void);
bool G_Responder(event_t *ev);

void G_PlayerReborn(void);

void G_ScreenShot(void);

void G_SetFastParms(bool fast_pending);
void G_SetMovementSpeed(int scale);
void G_ToggleAlwaysRun(evtype_t type);

void G_NextWeapon(void);
void G_PrevWeapon(void);
void G_RemoveChoppers(void);

void G_LoadedGameMessage(void);

extern fixed_t      forwardmove[2];
extern fixed_t      sidemove[2];
extern fixed_t      angleturn[3];
extern bool         gamekeydown[NUMKEYS];
extern bool         *mousebuttons;
extern bool         gamecontrollerpress;
extern char         lbmname1[MAX_PATH];
extern char         lbmpath1[MAX_PATH];
extern char         lbmpath2[MAX_PATH];
extern char         keyactionlist[NUMKEYS][255];
extern char         mouseactionlist[MAX_MOUSE_BUTTONS + 2][255];
extern int          st_facecount;
extern bool         skipaction;
extern char         savename[MAX_PATH];
extern bool         usemouselook;
extern bool         secretexit;
extern gameaction_t loadaction;
extern bool         newpars;
extern int          pars[10][10];
extern int          cpars[100];
