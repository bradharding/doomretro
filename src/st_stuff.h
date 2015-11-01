/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (c) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (c) 2013-2016 Brad Harding.

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

#if !defined(__ST_STUFF__)
#define __ST_STUFF__

#include "doomtype.h"
#include "d_event.h"
#include "m_cheat.h"

// Size of statusbar.
// Now sensitive for scaling.
#define ST_WIDTH                SCREENWIDTH
#define ST_Y                    (SCREENHEIGHT - SBARHEIGHT)

#define ST_STRAIGHTFACECOUNT    (TICRATE / 2)

#define MAPCHANGETICS           TICRATE

//
// STATUS BAR
//

// Called by main loop.
dboolean ST_Responder(event_t *ev);
void ST_AutomapEvent(int type);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(dboolean fullscreen, dboolean refresh);

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

extern cheatseq_t       cheat_mus;
extern cheatseq_t       cheat_mus_xy;
extern cheatseq_t       cheat_god;
extern cheatseq_t       cheat_ammo;
extern cheatseq_t       cheat_ammonokey;
extern cheatseq_t       cheat_noclip;
extern cheatseq_t       cheat_commercial_noclip;
extern cheatseq_t       cheat_powerup[7];
extern cheatseq_t       cheat_choppers;
extern cheatseq_t       cheat_clev;
extern cheatseq_t       cheat_clev_xy;
extern cheatseq_t       cheat_mypos;
extern cheatseq_t       cheat_amap;

extern dboolean         vid_widescreen;
extern dboolean         returntowidescreen;

extern patch_t          *grnrock;
extern patch_t          *brdr_t;
extern patch_t          *brdr_b;
extern patch_t          *brdr_l;
extern patch_t          *brdr_r;
extern patch_t          *brdr_tl;
extern patch_t          *brdr_tr;
extern patch_t          *brdr_bl;
extern patch_t          *brdr_br;

#endif
