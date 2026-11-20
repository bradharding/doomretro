/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#pragma once

#include "doomtype.h"
#include "w_file.h"

#define CACHESIZE       65536
#define MAXSCREENSHOTS  2
#define MAXJOBS         4

typedef struct
{
    char            path[MAX_PATH];
    int             width;
    int             height;
    byte            *raw;
    size_t          rawsize;
    byte            palette[256 * 3];
} screenshot_t;

typedef struct
{
    screenshot_t    screenshots[MAXSCREENSHOTS];
    int             numscreenshots;
    SDL_Thread      *thread;
    SDL_atomic_t    done;
    SDL_atomic_t    failed;
} screenshotjob_t;

typedef struct
{
    char            key[MAX_PATH * 2];
    int             next;
} screenshotcounter_t;

extern bool takingcleancreenshot;

void V_ScreenShot(void);
void V_FlashScreen(void);
void V_ScreenShotSound(void);
void V_UpdateScreenShot(void);
void V_FinishScreenShot(const bool report);
