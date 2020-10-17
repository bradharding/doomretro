/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

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
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include <string.h>

#include "i_video.h"
#include "m_config.h"
#include "m_random.h"
#include "v_video.h"

//
// SCREEN WIPE PACKAGE
//

static short    wipe_scr_start[MAXSCREENAREA];
static short    wipe_scr_end[MAXSCREENAREA];
static short    *wipe_scr;
static int      y[MAXWIDTH];
static int      speed;
static short    dest[MAXSCREENAREA];

static void wipe_shittyColMajorXform(short *array)
{
    for (int yy = 0; yy < SCREENHEIGHT; yy++)
        for (int xx = 0; xx < SCREENWIDTH / 2; xx++)
            dest[yy + xx * SCREENHEIGHT] = array[yy * SCREENWIDTH / 2 + xx];

    memcpy(array, dest, SCREENAREA);
}

static void wipe_initMelt(void)
{
    speed = SCREENHEIGHT / 16;

    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, SCREENAREA);

    // makes this wipe faster (in theory) to have stuff in column-major format
    wipe_shittyColMajorXform(wipe_scr_start);
    wipe_shittyColMajorXform(wipe_scr_end);

    // setup initial column positions (y < 0 => not ready to scroll yet)
    y[0] = y[1] = -(M_Random() & 15);

    for (int i = 2; i < SCREENWIDTH - 1; i += 2)
        y[i] = y[i + 1] = BETWEEN(-15, y[i - 1] + M_Random() % 3 - 1, 0);
}

static void wipe_Melt(int i, int dy)
{
    short   *s = &wipe_scr_end[i * SCREENHEIGHT + y[i]];
    short   *d = &wipe_scr[y[i] * SCREENWIDTH / 2 + i];

    for (int j = 0, k = dy; k > 0; k--, j += SCREENWIDTH / 2)
        d[j] = *s++;

    y[i] += dy;
    s = &wipe_scr_start[i * SCREENHEIGHT];
    d = &wipe_scr[y[i] * SCREENWIDTH / 2 + i];

    for (int j = 0, k = SCREENHEIGHT - y[i]; k > 0; k--, j += SCREENWIDTH / 2)
        d[j] = *s++;
}

static dboolean wipe_doMelt(void)
{
    dboolean    done = true;

    for (int i = 0; i < SCREENWIDTH / 2; i++)
        if (y[i] < 0)
        {
            y[i]++;
            done = false;
        }
        else if (y[i] < 16)
        {
            wipe_Melt(i, y[i] + 1);
            done = false;
        }
        else if (y[i] < SCREENHEIGHT)
        {
            wipe_Melt(i, MIN(speed, SCREENHEIGHT - y[i]));
            done = false;
        }

    return done;
}

void wipe_StartScreen(void)
{
    memcpy(wipe_scr_start, screens[0], SCREENAREA);
}

void wipe_EndScreen(void)
{
    memcpy(wipe_scr_end, screens[0], SCREENAREA);
    memcpy(screens[0], wipe_scr_start, SCREENAREA);
}

dboolean wipe_ScreenWipe(void)
{
    // when false, stop the wipe
    static dboolean go;

    // initial stuff
    if (!go)
    {
        go = true;
        wipe_scr = (short *)screens[0];
        wipe_initMelt();
    }

    // do a piece of wipe-in
    if (wipe_doMelt())
        go = false;

    return !go;
}
