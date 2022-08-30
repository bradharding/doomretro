/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

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

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define NUMREDPALS              (PLAYPALs > 2 ? 8 : 20)
#define STARTBONUSPALS          (STARTREDPALS + NUMREDPALS)
#define NUMBONUSPALS            4
#define RADIATIONPAL            (STARTBONUSPALS + NUMBONUSPALS)

// Size of status bar.
#define ST_WIDTH                SCREENWIDTH

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE           (ST_NUMSTRAIGHTFACES + ST_NUMTURNFACES + ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES             (ST_FACESTRIDE * ST_NUMPAINFACES + ST_NUMEXTRAFACES)

#define ST_STRAIGHTFACE         1
#define ST_STRAIGHTFACECOUNT    (TICRATE / 2)

#define MAPCHANGETICS           TICRATE

//
// STATUS BAR
//

// Called by main loop.
bool ST_Responder(event_t *ev);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(bool fullscreen, bool refresh);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);

static int ST_CalcPainOffset(void);

extern bool     idclev;
extern int      idclevtics;
extern bool     idmus;
extern int      st_palette;
extern bool     oldweaponsowned[NUMWEAPONS];
extern patch_t  *tallnum[10];
extern patch_t  *tallpercent;
extern short    tallpercentwidth;
extern bool     emptytallpercent;
extern int      caretcolor;
extern patch_t  *faces[ST_NUMFACES];
extern int      st_faceindex;
extern int      oldhealth;

extern byte     *grnrock;
extern patch_t  *brdr_t;
extern patch_t  *brdr_b;
extern patch_t  *brdr_l;
extern patch_t  *brdr_r;
extern patch_t  *brdr_tl;
extern patch_t  *brdr_tr;
extern patch_t  *brdr_bl;
extern patch_t  *brdr_br;

extern bool     st_drawbrdr;
