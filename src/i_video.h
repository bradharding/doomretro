/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"
#include "SDL.h"

#define MAX_MOUSE_BUTTONS       8

#define GAMMALEVELS             31

void I_InitKeyboard(void);

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics(void);

void I_ShutdownGraphics(void);
void I_SaveWindowPosition(void);

void I_ShutdownKeyboard(void);

// Takes full 8 bit values.
void I_SetPalette(byte *palette);

void I_FinishUpdate(void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen(byte *scr);

void done_win32(void);
void M_QuitDOOM(int choice);
void R_SetViewSize(int blocks);

extern boolean  screenvisible;

extern float    mouse_acceleration;
extern int      mouse_threshold;

extern boolean  sendpause;
extern boolean  quitting;
extern int      screensize;

extern int      keydown;

extern boolean  idclev;
extern boolean  idmus;
extern boolean  idbehold;

extern int      gammaindex;
extern float    gammalevel;
extern float    gammalevels[GAMMALEVELS];

extern boolean  blurred;
extern boolean  splashscreen;
extern boolean  noinput;

extern boolean showfps;

#endif
