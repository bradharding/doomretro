/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <string.h>

#include "i_video.h"
#include "m_random.h"
#include "v_video.h"

//
// SCREEN WIPE PACKAGE
//

static int      y[MAXWIDTH];
static short    src[MAXSCREENAREA];

static void wipe_shittyColMajorXform(short *dest)
{
    for (int yy = 0; yy < SCREENHEIGHT; yy++)
        for (int xx = 0; xx < SCREENWIDTH / 2; xx++)
            src[yy + xx * SCREENHEIGHT] = dest[yy * SCREENWIDTH / 2 + xx];

    memcpy(dest, src, SCREENAREA);
}

static void wipe_initMelt(void)
{
    // copy start screen to main screen
    memcpy(screens[0], screens[2], SCREENAREA);

    // makes this wipe faster (in theory) to have stuff in column-major format
    wipe_shittyColMajorXform((short *)screens[2]);
    wipe_shittyColMajorXform((short *)screens[3]);

    // setup initial column positions (y < 0 => not ready to scroll yet)
    y[0] = y[1] = -(M_BigRandom() & 15);

    for (int i = 2; i < SCREENWIDTH - 1; i += 2)
        y[i] = y[i + 1] = BETWEEN(-15, y[i - 1] + M_BigRandom() % 3 - 1, 0);
}

static void wipe_Melt(const int i, const int dy)
{
    short   *s = &((short *)screens[3])[i * SCREENHEIGHT + y[i]];
    short   *d = &((short *)screens[0])[y[i] * SCREENWIDTH / 2 + i];

    for (int j = 0, k = dy; k > 0; k--, j += SCREENWIDTH / 2)
        d[j] = *s++;

    y[i] += dy;
    s = &((short *)screens[2])[i * SCREENHEIGHT];
    d = &((short *)screens[0])[y[i] * SCREENWIDTH / 2 + i];

    for (int j = 0, k = SCREENHEIGHT - y[i]; k > 0; k--, j += SCREENWIDTH / 2)
        d[j] = *s++;
}

static bool wipe_doMelt(void)
{
    bool    done = true;

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
            wipe_Melt(i, MIN(SCREENHEIGHT / 16, SCREENHEIGHT - y[i]));
            done = false;
        }

    return done;
}

void wipe_StartScreen(void)
{
    memcpy(screens[2], screens[0], SCREENAREA);
}

void wipe_EndScreen(void)
{
    memcpy(screens[3], screens[0], SCREENAREA);
    memcpy(screens[0], screens[2], SCREENAREA);
}

bool wipe_ScreenWipe(void)
{
    // when false, stop the wipe
    static bool go;

    // initial stuff
    if (!go)
    {
        go = true;
        wipe_initMelt();
    }

    // do a piece of wipe-in
    if (wipe_doMelt())
        go = false;

    return !go;
}
