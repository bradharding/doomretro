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

#include <math.h>

#include "doomdef.h"
#include "doomtype.h"

#include "i_video.h"
#include "z_zone.h"
#include "m_argv.h"

#define ADDITIVE -1

#define R 1
#define W 2
#define G 4
#define B 8

static byte filter[256] = {
    0,0,0,0,R|B,0,0,0,0,0,0,0,0,0,0,0,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,R,R,R,R,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,R,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,R,R,R,R,R,R,R,R,R,R,R,R,R,
    R,R,R,R|B,R|B,R,R,R,R,R,R,0,0,0,0,0,0,0,0,B,B,B,B,B,B,B,B,R,R,0,0,0,0,0,0
};

#define ALL    0
#define REDS   R
#define WHITES W
#define GREENS G
#define BLUES  B

int FindNearestColor(byte *palette, int red, int green, int blue)
{
    double best_difference = LONG_MAX;
    int best_color = 0;
    int i;

    for (i = 0; i < 256; ++i)
    {
        byte *color = palette + i * 3;
        long rmean = ((long)red + (long)color[0]) >> 1;
        long r = (long)red - (long)color[0];
        long g = (long)green - (long)color[1];
        long b = (long)blue - (long)color[2];
        double difference = sqrt((double)((((512 + rmean) * r * r) >> 8) +
                            4 * g * g + (((767 - rmean) * b * b) >> 8)));

        if (!difference)
            return i;
        else if (difference < best_difference)
        {
            best_color = i;
            best_difference = difference;
        }
    }
    return best_color;
}

static byte *GenerateTintTable(byte *palette, int percent, int colors)
{
    byte *result;
    int x, y;

    result = (byte *)Z_Malloc(65536, PU_STATIC, NULL);
    for (y = 0; y < 256; ++y)
    {
        if ((filter[y] & colors) || colors == ALL)
        {
            for (x = 0; x < 256; ++x)
            {
                byte *col1, *col2;
                int r, g, b;

                col1 = palette + x * 3;
                col2 = palette + y * 3;
                if (percent == ADDITIVE)
                {
                    r = col1[0] + col2[0];
                    if (r > 255)
                        r = 255;
                    g = col1[1] + col2[1];
                    if (g > 255)
                        g = 255;
                    b = col1[2] + col2[2];
                    if (b > 255)
                        b = 255;
                }
                else
                {
                    r = ((int)col1[0] * percent + (int)col2[0] * (100 - percent)) / 100;
                    g = ((int)col1[1] * percent + (int)col2[1] * (100 - percent)) / 100;
                    b = ((int)col1[2] * percent + (int)col2[2] * (100 - percent)) / 100;
                }
                *(result + (x << 8) + y) = FindNearestColor(palette, r, g, b);
            }
        }
        else
        {
            for (x = 0; x < 256; ++x)
                *(result + (x << 8) + y) = y;
        }
    }
    if (colors == ALL && percent != ADDITIVE)
    {
        *(result + (77 << 8) + 109) = *(result + (109 << 8) + 77) = 77;
        *(result + (78 << 8) + 109) = *(result + (109 << 8) + 78) = 109;
    }
    return result;
}

byte *tinttab;

byte *tinttab33;
byte *tinttab50;
byte *tinttab60;
byte *tinttab75;
byte *tinttab80;

byte *tinttabred;
byte *tinttabredwhite;
byte *tinttabgreen;
byte *tinttabblue;

byte *tinttabred50;
byte *tinttabredwhite50;
byte *tinttabgreen50;
byte *tinttabblue50;

void I_InitTintTables(byte *palette)
{
    tinttab = GenerateTintTable(palette, ADDITIVE, ALL);

    tinttab33 = GenerateTintTable(palette, 33, ALL);
    tinttab50 = GenerateTintTable(palette, 50, ALL);
    tinttab60 = GenerateTintTable(palette, 60, ALL);
    tinttab75 = GenerateTintTable(palette, 75, ALL);
    tinttab80 = GenerateTintTable(palette, 80, ALL);

    tinttabred = GenerateTintTable(palette, ADDITIVE, REDS);
    tinttabredwhite = GenerateTintTable(palette, ADDITIVE, REDS|WHITES);
    tinttabgreen = GenerateTintTable(palette, ADDITIVE, GREENS);
    tinttabblue = GenerateTintTable(palette, ADDITIVE, BLUES);

    tinttabred50 = GenerateTintTable(palette, 50, REDS);
    tinttabredwhite50 = GenerateTintTable(palette, 50, REDS|WHITES);
    tinttabgreen50 = GenerateTintTable(palette, 50, GREENS);
    tinttabblue50 = GenerateTintTable(palette, 50, BLUES);
}