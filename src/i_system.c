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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void I_ShutdownWindows32(void);
#else
#include <unistd.h>
#endif

#include "d_net.h"
#include "doomdef.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "s_sound.h"
#include "SDL.h"
#include "version.h"
#include "w_merge.h"
#include "w_wad.h"
#include "z_zone.h"

extern boolean  widescreen;
extern boolean  hud;
extern boolean  returntowidescreen;

//
// I_Quit
//
void I_Quit(boolean shutdown)
{
    if (shutdown)
    {
        S_Shutdown();

        I_SaveWindowPosition();

        if (returntowidescreen)
            widescreen = true;

        M_SaveDefaults();

        I_ShutdownGraphics();

        I_ShutdownKeyboard();

        I_ShutdownGamepad();
    }

#ifdef WIN32
    I_ShutdownWindows32();
#endif

    SDL_Quit();

    exit(0);
}

void I_WaitVBL(int count)
{
    I_Sleep((count * 1000) / 70);
}

//
// I_Error
//
static boolean already_quitting = false;

void I_Error(char *error, ...)
{
    va_list     argptr;
    char        msgbuf[512];

    if (already_quitting)
        exit(-1);
    else
        already_quitting = true;

    // Shutdown. Here might be other errors.
    S_Shutdown();

    I_SaveWindowPosition();

    if (returntowidescreen)
        widescreen = true;

    I_ShutdownGraphics();

    I_ShutdownKeyboard();

    I_ShutdownGamepad();

    va_start(argptr, error);
    memset(msgbuf, 0, sizeof(msgbuf));
    M_vsnprintf(msgbuf, sizeof(msgbuf) - 1, error, argptr);
    va_end(argptr);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PACKAGE_NAME, msgbuf, NULL);

    SDL_Quit();

    exit(-1);
}
