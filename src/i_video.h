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

#if !defined(__I_VIDEO__)
#define __I_VIDEO__

#include "doomtype.h"
#include "SDL.h"

#define MAX_MOUSE_BUTTONS       8

#define GAMMALEVELS             31

typedef struct
{
    int         width;
    int         height;
    char        *acronym;
    char        *aspectratio;
} resolution_t;

void I_InitKeyboard(void);

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics(void);
void I_RestartGraphics(void);
void I_ShutdownGraphics(void);

void GetWindowPosition(void);
void GetWindowSize(void);
void GetScreenResolution(void);

void I_ShutdownKeyboard(void);

// Takes full 8 bit values.
void I_SetPalette(byte *palette);

void I_FinishUpdate(void);
void I_FinishUpdate_Best(void);
void I_FinishUpdate_ShowFPS(void);
void I_FinishUpdate_Best_ShowFPS(void);
void I_FinishUpdate_Shake(void);
void I_FinishUpdate_Best_Shake(void);
void I_FinishUpdate_ShowFPS_Shake(void);
void I_FinishUpdate_Best_ShowFPS_Shake(void);

void ToggleFullscreen(void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen(byte *scr);

void M_QuitDOOM(int choice);
void R_SetViewSize(int blocks);

extern dboolean screenvisible;

extern float    m_acceleration;
extern int      m_threshold;

extern dboolean sendpause;
extern dboolean quitting;
extern int      r_screensize;

extern int      keydown;

extern dboolean idclev;
extern dboolean idmus;
extern dboolean idbehold;
extern dboolean message_clearable;

extern int      gammaindex;
extern float    r_gamma;
extern float    gammalevels[GAMMALEVELS];

extern dboolean blurred;
extern dboolean splashscreen;
extern dboolean noinput;

extern void(*updatefunc)(void);

extern dboolean vid_showfps;
extern dboolean wipe;

extern int      windowx;
extern int      windowy;
extern int      windowheight;
extern int      windowwidth;

extern dboolean bestscale;

#endif
