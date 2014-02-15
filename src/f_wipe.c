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

#include "i_video.h"
#include "f_wipe.h"
#include "v_video.h"
#include "z_zone.h"

//
// SCREEN WIPE PACKAGE
//

static byte *wipe_scr_start;
static byte *wipe_scr_end;
static byte *wipe_scr;

void wipe_shittyColMajorXform(short *array)
{
    int   x, y;
    short *dest = (short *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, 0);

    for (y = 0; y < SCREENHEIGHT; y++)
        for (x = 0; x < SCREENWIDTH / 2; x++)
            dest[x * SCREENHEIGHT + y] = array[y * SCREENWIDTH / 2 + x];

    memcpy(array, dest, SCREENWIDTH * SCREENHEIGHT);

    Z_Free(dest);
}

static int *y;
static int speed;

boolean wipe_initMelt(int tics)
{
    int i;

    speed = (SCREENHEIGHT - (SBARHEIGHT * widescreen)) / 16;

    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, SCREENWIDTH * SCREENHEIGHT);

    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((short *)wipe_scr_start);
    wipe_shittyColMajorXform((short *)wipe_scr_end);

    // setup initial column positions
    // (y < 0 => not ready to scroll yet)
    y = (int *)Z_Malloc(SCREENWIDTH * sizeof(int), PU_STATIC, 0);
    y[0] = y[1] = -(rand() % 16);
    for (i = 2; i < SCREENWIDTH - 1; i += 2)
        y[i] = y[i + 1] = MAX(-15, MIN(y[i - 1] + (rand() % 3) - 1, 0));

    return false;
}

boolean wipe_doMelt(int tics)
{
    boolean done = true;

    while (tics--)
    {
        int i;

        for (i = 0; i < SCREENWIDTH / 2; i++)
        {
            if (y[i] < 0)
            {
                y[i]++;
                done = false;
                continue;
            }
            if (y[i] < SCREENHEIGHT)
            {
                int   j;
                int   dy = (y[i] < 16 ? y[i] + 1 : speed);
                int   idx = 0;

                short *s = &((short *)wipe_scr_end)[i * SCREENHEIGHT + y[i]];
                short *d = &((short *)wipe_scr)[y[i] * SCREENWIDTH / 2 + i];

                if (y[i] + dy >= SCREENHEIGHT)
                    dy = SCREENHEIGHT - y[i];
                for (j = dy; j; j--)
                {
                    d[idx] = *s++;
                    idx += SCREENWIDTH / 2;
                }
                y[i] += dy;
                s = &((short *)wipe_scr_start)[i * SCREENHEIGHT];
                d = &((short *)wipe_scr)[y[i] * SCREENWIDTH / 2 + i];
                idx = 0;
                for (j = SCREENHEIGHT - y[i]; j; j--)
                {
                    d[idx] = *s++;
                    idx += SCREENWIDTH / 2;
                }
                done = false;
            }
        }
    }

    return done;
}

boolean wipe_exitMelt(int tics)
{
    Z_Free(y);
    Z_Free(wipe_scr_start);
    Z_Free(wipe_scr_end);
    return false;
}

boolean wipe_StartScreen(void)
{
    wipe_scr_start = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_start);
    return false;
}

boolean wipe_EndScreen(void)
{
    wipe_scr_end = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(0, 0, 0, SCREENWIDTH, SCREENHEIGHT, wipe_scr_start);
    return false;
}

boolean wipe_ScreenWipe(int tics)
{
    // when zero, stop the wipe
    static boolean go = false;

    // initial stuff
    if (!go)
    {
        go = true;
        wipe_scr = screens[0];
        wipe_initMelt(tics);
    }

    // do a piece of wipe-in
    if (wipe_doMelt(tics))
    {
        // final stuff
        go = false;
        wipe_exitMelt(tics);
    }

    return !go;
}
