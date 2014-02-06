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

// when zero, stop the wipe
static boolean go = false;

static byte    *wipe_scr_start;
static byte    *wipe_scr_end;
static byte    *wipe_scr;

void wipe_shittyColMajorXform(short *array, int width, int height)
{
    int   x;
    int   y;
    short *dest = (short *)Z_Malloc(width * height * 2, PU_STATIC, 0);

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            dest[x * height + y] = array[y * width + x];

    memcpy(array, dest, width * height * 2);

    Z_Free(dest);
}

boolean wipe_initColorXForm(int width, int height, int tics)
{
    memcpy(wipe_scr, wipe_scr_start, width * height);
    return false;
}

boolean wipe_doColorXForm(int width, int height, int tics)
{
    boolean changed = false;
    byte    *w = wipe_scr;
    byte    *e = wipe_scr_end;

    while (w != wipe_scr + width * height)
    {
        if (*w > *e)
        {
            *w = MAX(*w - tics, *e);
            changed = true;
        }
        else if (*w < *e)
        {
            *w = MIN(*w + tics, *e);
            changed = true;
        }
        w++;
        e++;
    }

    return !changed;
}

boolean wipe_exitColorXForm(int width, int height, int tics)
{
    return false;
}

static int *y;
static int speed;

boolean wipe_initMelt(int width, int height, int tics)
{
    int i;

    speed = (height - (SBARHEIGHT << widescreen)) >> 4;

    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, width * height);

    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((short *)wipe_scr_start, width / 2, height);
    wipe_shittyColMajorXform((short *)wipe_scr_end, width / 2, height);

    // setup initial column positions
    // (y < 0 => not ready to scroll yet)
    y = (int *)Z_Malloc(width * sizeof(int), PU_STATIC, 0);
    y[0] = y[1] = -(rand() % 16);
    for (i = 2; i < width - 1; i += 2)
        y[i] = y[i + 1] = MAX(-15, MIN(y[i - 1] + (rand() % 3) - 1, 0));

    return false;
}

boolean wipe_doMelt(int width, int height, int tics)
{
    boolean done = true;

    width >>= 1;

    while (tics--)
    {
        int i;

        for (i = 0; i < width; i++)
        {
            if (y[i] < 0)
            {
                y[i]++;
                done = false;
            }
            else if (y[i] < height)
            {
                int   j;
                int   dy = (y[i] < 16 ? y[i] + 1 : speed);
                int   idx = 0;

                short *s = &((short *)wipe_scr_end)[i * height + y[i]];
                short *d = &((short *)wipe_scr)[y[i] * width + i];

                if (y[i] + dy >= height)
                    dy = height - y[i];
                for (j = dy; j; j--)
                {
                    d[idx] = *s++;
                    idx += width;
                }
                y[i] += dy;
                s = &((short *)wipe_scr_start)[i * height];
                d = &((short *)wipe_scr)[y[i] * width + i];
                idx = 0;
                for (j = height - y[i]; j; j--)
                {
                    d[idx] = *s++;
                    idx += width;
                }
                done = false;
            }
        }
    }

    return done;
}

boolean wipe_exitMelt(int width, int height, int tics)
{
    Z_Free(y);
    Z_Free(wipe_scr_start);
    Z_Free(wipe_scr_end);
    return false;
}

boolean wipe_StartScreen(int x, int y, int width, int height)
{
    wipe_scr_start = (byte *)Z_Malloc(width * height, PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_start);
    return false;
}

boolean wipe_EndScreen(int x, int y, int width, int height)
{
    wipe_scr_end = (byte *)Z_Malloc(width * height, PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(x, y, 0, width, height, wipe_scr_start); // restore start scr.
    return false;
}

boolean wipe_ScreenWipe(int wipeno, int x, int y, int width, int height, int tics)
{
    static boolean (*wipes[])(int, int, int) =
    {
        wipe_initColorXForm,
        wipe_doColorXForm,
        wipe_exitColorXForm,
        wipe_initMelt,
        wipe_doMelt,
        wipe_exitMelt
    };

    // initial stuff
    if (!go)
    {
        go = true;
        wipe_scr = screens[0];
        (*wipes[wipeno * 3])(width, height, tics);
    }

    // do a piece of wipe-in
    V_MarkRect(0, 0, width, height);
    if ((*wipes[wipeno * 3 + 1])(width, height, tics))
    {
        // final stuff
        go = false;
        (*wipes[wipeno * 3 + 2])(width, height, tics);
    }

    return !go;
}