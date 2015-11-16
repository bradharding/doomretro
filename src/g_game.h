/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__G_GAME__)
#define __G_GAME__

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"

//
// GAME
//
void G_InitNew(skill_t skill, int ep, int map);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferredInitNew(skill_t skill, int ep, int map);

void G_DeferredLoadLevel(skill_t skill, int ep, int map); // [BH]

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel.
void G_LoadGame(char *name);

void G_DoLoadGame(void);

// Called by M_Responder.
void G_SaveGame(int slot, char *description, char *name);

void G_ExitLevel(void);
void G_SecretExitLevel(void);

void G_WorldDone(void);

// Read current data from inputs and build a player movement command.
void G_BuildTiccmd(ticcmd_t *cmd);

void G_Ticker(void);
dboolean G_Responder(event_t *ev);

void G_ScreenShot(void);
void I_ToggleWidescreen(dboolean toggle);

extern dboolean canmodify;
extern dboolean flag667;
extern dboolean message_dontpause;
extern dboolean vibrate;
extern dboolean gamepadpress;
extern char     lbmname[256];
extern char     mapnumandtitle[512];
extern int      mousewait;
extern int      gamepadwait;
extern int      keydown;
extern int      markpointnum;
extern int      quickSaveSlot;
extern int      st_facecount;
extern dboolean oldweaponsowned[NUMWEAPONS];
extern dboolean blurred;

#endif
