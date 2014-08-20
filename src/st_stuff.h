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

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "m_cheat.h"

// Size of statusbar.
// Now sensitive for scaling.
#define ST_WIDTH                SCREENWIDTH
#define ST_Y                    (SCREENHEIGHT - SBARHEIGHT)

#define ST_STRAIGHTFACECOUNT    (TICRATE / 2)

//
// STATUS BAR
//

// Called by main loop.
boolean ST_Responder(event_t *ev);
void ST_AutomapEvent(int type);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(boolean fullscreen, boolean refresh);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);

// States for status bar code.
typedef enum
{
    AutomapState,
    FirstPersonState
} st_stateenum_t;

// States for the chat code.
typedef enum
{
    StartChatState,
    WaitDestState,
    GetChatState
} st_chatstateenum_t;

extern cheatseq_t cheat_mus;
extern cheatseq_t cheat_mus_xy;
extern cheatseq_t cheat_god;
extern cheatseq_t cheat_ammo;
extern cheatseq_t cheat_ammonokey;
extern cheatseq_t cheat_noclip;
extern cheatseq_t cheat_commercial_noclip;
extern cheatseq_t cheat_powerup[7];
extern cheatseq_t cheat_choppers;
extern cheatseq_t cheat_clev;
extern cheatseq_t cheat_clev_xy;
extern cheatseq_t cheat_mypos;
extern cheatseq_t cheat_amap;

extern boolean widescreen;
extern boolean returntowidescreen;

#endif
