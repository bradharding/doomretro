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

#include "doomstat.h"
#include "i_swap.h"
#include "m_misc.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define CONSOLESPEED            8
#define CONSOLEHEIGHT           ((SCREENHEIGHT - SBARHEIGHT) / 2)

#define CONSOLEFONTSTART        '!'
#define CONSOLEFONTEND          '~'
#define CONSOLEFONTSIZE         (CONSOLEFONTEND - CONSOLEFONTSTART + 1)

int             consoleheight = 0;
int             consoledirection = 1;

byte            *background;
patch_t         *border;
patch_t         *consolefont[CONSOLEFONTSIZE];

extern byte     *tinttab75;

void C_Init(void)
{
    int         i;
    int         j = CONSOLEFONTSTART;
    char        buffer[9];

    background = W_CacheLumpName((gamemode == commercial ? "GRNROCK" : "FLOOR7_2"), PU_CACHE);
    border = W_CacheLumpName("BRDR_B", PU_CACHE);

    for (i = 0; i < CONSOLEFONTSIZE; i++)
    {
        M_snprintf(buffer, 9, "DRFON%03d", j++);
        consolefont[i] = W_CacheLumpName(buffer, PU_STATIC);
    }
}

static void C_DrawBackground(int height)
{
    byte        *dest = screens[0];
    int         x, y;
    int         offset = CONSOLEHEIGHT - height;

    for (y = offset; y < height + offset; y += 2)
        for (x = 0; x < SCREENWIDTH / 32; x += 2)
        {
            int i;

            for (i = 0; i < 64; i++)
            {
                int     j = i * 2;
                int     dot = *(background + (((y / 2) & 63) << 6) + i) << 8;

                *(dest + j) = tinttab75[dot + *(dest + j)];
                ++j;
                *(dest + j) = tinttab75[dot + *(dest + j)];
            }
            dest += 128;
        }

    for (x = 0; x < ORIGINALWIDTH; x += 8)
        V_DrawTranslucentPatch(x, height / 2, 0, border);

    for (x = 0; x < ORIGINALWIDTH; ++x)
        V_DrawPixel(x, height / 2 + 3, 251, true);
}

static void C_DrawText(int x, int y, char *text)
{
    int i;

    for (i = 0; (unsigned int)i < strlen(text); ++i)
    {
        int     c = text[i] - CONSOLEFONTSTART;

        if (c < 0 || c >= CONSOLEFONTSIZE)
            x += 4;
        else
        {
            patch_t     *patch = consolefont[c];

            V_DrawBigPatch(x, y - (CONSOLEHEIGHT - consoleheight), 0, patch);
            x += SHORT(patch->width);
        }
    }
}

void C_Drawer(void)
{
    if (!consoleheight)
        return;

    consoleheight = BETWEEN(0, consoleheight + CONSOLESPEED * consoledirection, CONSOLEHEIGHT);

    C_DrawBackground(consoleheight);

    C_DrawText(SCREENWIDTH - 100, CONSOLEHEIGHT - 15, PACKAGE_VERSIONSTRING);
}
