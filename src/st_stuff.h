/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__ST_STUFF_H__)
#define __ST_STUFF_H__

// Size of status bar.
// Now sensitive for scaling.
#define ST_WIDTH                SCREENWIDTH

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE           (ST_NUMSTRAIGHTFACES + ST_NUMTURNFACES + ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES             (ST_FACESTRIDE * ST_NUMPAINFACES + ST_NUMEXTRAFACES)

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

extern dboolean returntowidescreen;

extern patch_t  *grnrock;
extern patch_t  *brdr_t;
extern patch_t  *brdr_b;
extern patch_t  *brdr_l;
extern patch_t  *brdr_r;
extern patch_t  *brdr_tl;
extern patch_t  *brdr_tr;
extern patch_t  *brdr_bl;
extern patch_t  *brdr_br;

#endif
