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

#include "m_fixed.h"
#include "z_zone.h"

#define ADDITIVE       -1

#define R               1
#define W               2
#define G               4
#define B               8

static byte cfilter[256] =
{
    0,0,0,0,R|B,0,0,0,0,0,0,0,0,0,0,0,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,R,R,R,R,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,R,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,R,R,R,R,R,R,R,R,R,R,R,R,R,
    R,R,R,R|B,R|B,R,R,R,R,R,R,0,0,0,0,0,0,0,0,B,B,B,B,B,B,B,B,R,R,0,0,0,0,0,0
};

#define ALL             0
#define REDS            R
#define WHITES          W
#define GREENS          G
#define BLUES           B

byte    *tinttab;

byte    *tinttab25;
byte    *tinttab33;
byte    *tinttab40;
byte    *tinttab50;
byte    *tinttab60;
byte    *tinttab66;
byte    *tinttab75;
byte    *tinttab80;

byte    *tinttabred;
byte    *tinttabredwhite;
byte    *tinttabgreen;
byte    *tinttabblue;

byte    *tinttabred50;
byte    *tinttabredwhite50;
byte    *tinttabgreen50;
byte    *tinttabblue50;

int FindNearestColor(byte *palette, int red, int green, int blue)
{
    double      best_difference = 257 * 257 + 257 * 257 + 257 * 257;
    int         best_color = 0;
    int         i;

    for (i = 0; i < 256; ++i)
    {
        int     r = red - *palette++;
        int     g = green - *palette++;
        int     b = blue - *palette++;
        int     difference = r * r + g * g + b * b;

        if (difference < best_difference)
        {
            if (!difference)
                return i;

            best_color = i;
            best_difference = difference;
        }
    }
    return best_color;
}

static byte *GenerateTintTable(byte *palette, int percent, int colors)
{
    byte        *result = Z_Malloc(65536, PU_STATIC, NULL);
    int         foreground, background;

    for (foreground = 0; foreground < 256; ++foreground)
    {
        if ((cfilter[foreground] & colors) || colors == ALL)
        {
            for (background = 0; background < 256; ++background)
            {
                byte    *color1 = palette + background * 3;
                byte    *color2 = palette + foreground * 3;
                int     r, g, b;

                if (percent == ADDITIVE)
                {
                    if ((cfilter[background] & BLUES) && !(cfilter[foreground] & WHITES))
                    {
                        r = ((int)color1[0] * 25 + (int)color2[0] * 75) / 100;
                        g = ((int)color1[1] * 25 + (int)color2[1] * 75) / 100;
                        b = ((int)color1[2] * 25 + (int)color2[2] * 75) / 100;
                    }
                    else
                    {
                        r = MIN(color1[0] + color2[0], 255);
                        g = MIN(color1[1] + color2[1], 255);
                        b = MIN(color1[2] + color2[2], 255);
                    }
                }
                else
                {
                    int blues = (color1[2] < color1[0] + color1[1] ? 0 :
                        (color1[2] - (color1[0] + color1[1])) / 2);

                    r = ((int)color1[0] * percent + (int)color2[0] * (100 - percent)) / (100 + blues);
                    g = ((int)color1[1] * percent + (int)color2[1] * (100 - percent)) / (100 + blues);
                    b = ((int)color1[2] * percent + (int)color2[2] * (100 - percent)) / 100;
                }
                *(result + (background << 8) + foreground) = FindNearestColor(palette, r, g, b);
            }
        }
        else
        {
            for (background = 0; background < 256; ++background)
                *(result + (background << 8) + foreground) = foreground;
        }
    }
    if (colors == ALL && percent != ADDITIVE)
    {
        *(result + (77 << 8) + 109) = *(result + (109 << 8) + 77) = 77;
        *(result + (78 << 8) + 109) = *(result + (109 << 8) + 78) = 109;
    }
    return result;
}

void I_InitTintTables(byte *palette)
{
    tinttab = GenerateTintTable(palette, ADDITIVE, ALL);

    tinttab25 = GenerateTintTable(palette, 25, ALL);
    tinttab33 = GenerateTintTable(palette, 33, ALL);
    tinttab40 = GenerateTintTable(palette, 40, ALL);
    tinttab50 = GenerateTintTable(palette, 50, ALL);
    tinttab60 = GenerateTintTable(palette, 60, ALL);
    tinttab66 = GenerateTintTable(palette, 66, ALL);
    tinttab75 = GenerateTintTable(palette, 75, ALL);
    tinttab80 = GenerateTintTable(palette, 80, ALL);

    tinttabred = GenerateTintTable(palette, ADDITIVE, REDS);
    tinttabredwhite = GenerateTintTable(palette, ADDITIVE, REDS | WHITES);
    tinttabgreen = GenerateTintTable(palette, ADDITIVE, GREENS);
    tinttabblue = GenerateTintTable(palette, ADDITIVE, BLUES);

    tinttabred50 = GenerateTintTable(palette, 50, REDS);
    tinttabredwhite50 = GenerateTintTable(palette, 50, REDS | WHITES);
    tinttabgreen50 = GenerateTintTable(palette, 50, GREENS);
    tinttabblue50 = GenerateTintTable(palette, 50, BLUES);
}
