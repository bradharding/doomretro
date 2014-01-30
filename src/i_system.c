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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "doomdef.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "i_gamepad.h"
#include "i_timer.h"
#include "i_video.h"
#include "s_sound.h"

#include "d_net.h"
#include "g_game.h"

#include "i_system.h"

#include "w_wad.h"
#include "z_zone.h"

#define DEFAULT_RAM 16 /* MiB */
#define MIN_RAM     4  /* MiB */

// Zone memory auto-allocation function that allocates the zone size
// by trying progressively smaller zone sizes until one is found that
// works.

static byte *AutoAllocMemory(int *size, int default_ram, int min_ram)
{
    byte *zonemem;

    // Allocate the zone memory.  This loop tries progressively smaller
    // zone sizes until a size is found that can be allocated.
    // If we used the -mb command line parameter, only the parameter
    // provided is accepted.

    zonemem = NULL;

    while (zonemem == NULL)
    {
        // We need a reasonable minimum amount of RAM to start.

        if (default_ram < min_ram)
        {
            I_Error("Unable to allocate %i MiB of RAM for zone", default_ram);
        }

        // Try to allocate the zone memory.

        *size = default_ram * 1024 * 1024;

        zonemem = (byte *)malloc(*size);

        // Failed to allocate?  Reduce zone size until we reach a size
        // that is acceptable.

        if (zonemem == NULL)
        {
            default_ram -= 1;
        }
    }

    return zonemem;
}

byte *I_ZoneBase(int *size)
{
    byte *zonemem;
    int min_ram, default_ram;
    int p;

    //!
    // @arg <mb>
    //
    // Specify the heap size, in MiB (default 16).
    //

    p = M_CheckParmWithArgs("-mb", 1);

    if (p > 0)
    {
        default_ram = atoi(myargv[p+1]);
        min_ram = default_ram;
    }
    else
    {
        default_ram = DEFAULT_RAM;
        min_ram = MIN_RAM;
    }

    zonemem = AutoAllocMemory(size, default_ram, min_ram);

    return zonemem;
}


//
// I_Init
//
void I_Init(void)
{
    I_InitTimer();
    I_InitGamepad();
}


//
// I_Quit
//

void I_Quit(void)
{
    D_QuitNetGame();
    if (demorecording)
        G_CheckDemoStatus();
    S_Shutdown();

    I_SaveWindowPosition();

    if (returntowidescreen)
        widescreen = true;

    M_SaveDefaults();

    I_ShutdownGraphics();

    I_ShutdownGamepad();

    exit(0);
}

void I_WaitVBL(int count)
{
    I_Sleep((count * 1000) / 70);
}

//
// I_Error
//
extern boolean demorecording;

static boolean already_quitting = false;

void I_Error(char *error, ...)
{
    va_list     argptr;
    char        msgbuf[512];
    wchar_t     wmsgbuf[512];


    if (already_quitting)
    {
        fprintf(stderr, "Warning: recursive call to I_Error detected.\n");
        exit(-1);
    }
    else
    {
        already_quitting = true;
    }

    // Message first.
    va_start(argptr, error);
    vfprintf(stderr, error, argptr);
    fprintf(stderr, "\n\n");
    va_end(argptr);
    fflush(stderr);

    // Shutdown. Here might be other errors.

    D_QuitNetGame();
    if (demorecording)
        G_CheckDemoStatus();
    S_Shutdown();

    I_SaveWindowPosition();

    if (returntowidescreen)
        widescreen = true;

    M_SaveDefaults();

    I_ShutdownGraphics();

    I_ShutdownGamepad();

    va_start(argptr, error);
    memset(msgbuf, 0, sizeof(msgbuf));
    vsnprintf(msgbuf, sizeof(msgbuf) - 1, error, argptr);
    va_end(argptr);

    MultiByteToWideChar(CP_ACP, 0,
                        msgbuf, strlen(msgbuf) + 1,
                        wmsgbuf, sizeof(wmsgbuf));

    MessageBoxW(NULL, wmsgbuf, L"DOOM RETRO", MB_ICONERROR | MB_OK);

    exit(-1);
}