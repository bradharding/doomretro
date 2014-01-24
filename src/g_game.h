/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"


//
// GAME
//
void G_DeathMatchSpawnPlayer(int playernum);

void G_InitNew(skill_t skill, int episode, int map);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferredInitNew(skill_t skill, int episode, int map);

void G_DeferredLoadLevel(skill_t skill, int episode, int map); // [BH]

void G_DeferredPlayDemo(char *demo);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
void G_LoadGame(char *name);

void G_DoLoadGame(void);

// Called by M_Responder.
void G_SaveGame(int slot, char *description);

// Only called by startup code.
void G_RecordDemo(char *name);

void G_BeginRecording(void);

void G_PlayDemo(char *name);
void G_TimeDemo(char *name);
boolean G_CheckDemoStatus(void);

void G_ExitLevel(void);
void G_SecretExitLevel(void);

void G_WorldDone(void);

// Read current data from inputs and build a player movement command.

void G_BuildTiccmd(ticcmd_t *cmd, int maketic);

void G_Ticker(void);
boolean G_Responder(event_t *ev);

void G_ScreenShot(void);
void ToggleWideScreen(boolean toggle);

extern boolean  canmodify;
extern boolean  flag667;
extern boolean  message_dontpause;
extern boolean  vibrate;
extern char     lbmname[256];
extern char     mapnumandtitle[133];
extern int      gamepadpress;
extern int      gamepadwait;
extern int      keydown;
extern int      markpointnum;
extern int      quickSaveSlot;
extern int      st_facecount;
#endif