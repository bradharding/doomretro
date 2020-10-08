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

#include <stdlib.h>

#include "i_colors.h"
#include "i_swap.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#define R   1
#define W   2
#define G   4
#define B   8
#define X  16

static byte filter[256] =
{
    0,   X,   0,   0,   R|B, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 000 to 015
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R, // 016 to 031
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R, // 032 to 047
    W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W, // 048 to 063
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   X,   X,   X, // 064 to 079
    R,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 080 to 095
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 096 to 111
    G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G, // 112 to 127
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 128 to 143
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 144 to 159
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R, // 160 to 175
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R, // 176 to 191
    B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B, // 192 to 207
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R, // 208 to 223
    R|B, R|B, R,   R,   R,   R,   R,   R,   X,   X,   X,   X,   0,   0,   0,   0, // 224 to 239
    B,   B,   B,   B,   B,   B,   B,   B,   R,   R,   0,   0,   0,   0,   0,   0  // 240 to 255
};

#define ALTHUD     -1
#define ALL         0
#define REDS        R
#define WHITES      W
#define GREENS      G
#define BLUES       B
#define EXTRAS      X

#define BLACK       0
#define RED         176
#define WHITE       4
#define YELLOW      160

static byte *tinttab15;

byte        *tinttab20;
byte        *tinttab25;
byte        *tinttab30;
byte        *tinttab33;
byte        *tinttab40;
byte        *tinttab50;
byte        *tinttab60;
byte        *tinttab66;
byte        *tinttab75;

byte        *alttinttab20;
byte        *alttinttab40;
byte        *alttinttab60;

byte        *tranmap;

byte        *tinttabadditive;
byte        *tinttabred;
byte        *tinttabredwhite1;
byte        *tinttabredwhite2;
byte        *tinttabgreen;
byte        *tinttabblue;

byte        *tinttabred33;
byte        *tinttabredwhite50;
byte        *tinttabgreen33;
byte        *tinttabblue25;

byte        nearestcolors[256];
byte        nearestblack;
byte        nearestred;
byte        nearestwhite;

byte        *black25;
byte        *black40;
byte        *yellow15;
byte        *white25;

int FindNearestColor(byte *palette, const int red, const int green, const int blue)
{
    int bestdiff = INT_MAX;
    int bestcolor = 0;

    for (int i = 0; i < 256; i++)
    {
        // From <https://www.compuphase.com/cmetric.htm>
        const int   rmean = (red + *palette) / 2;
        const int   r = red - *palette++;
        const int   g = green - *palette++;
        const int   b = blue - *palette++;
        const int   diff = (((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8);

        if (!diff)
            return i;
        else if (diff < bestdiff)
        {
            bestcolor = i;
            bestdiff = diff;
        }
    }

    return bestcolor;
}

void FindNearestColors(byte *palette)
{
    if (W_CheckMultipleLumps("PLAYPAL") > 1)
    {
        byte    *splashpal = W_CacheLumpName("SPLSHPAL");

        for (int i = 0; i < 256; i++)
        {
            byte    red = *splashpal++;
            byte    green = *splashpal++;
            byte    blue = *splashpal++;

            nearestcolors[i] = FindNearestColor(palette, red, green, blue);
        }
    }
    else
        for (int i = 0; i < 256; i++)
            nearestcolors[i] = i;

    nearestblack = nearestcolors[BLACK];
    nearestred = nearestcolors[RED];
    nearestwhite = nearestcolors[WHITE];

    black25 = &tinttab25[nearestblack << 8];
    black40 = &tinttab40[nearestblack << 8];
    yellow15 = &tinttab15[nearestcolors[YELLOW] << 8];
    white25 = &tinttab25[nearestwhite << 8];
}

int FindDominantColor(patch_t *patch, byte *palette)
{
    const int   w = SHORT(patch->width);
    int         dominantcolor = 0;
    int         dominantcolorcount = 0;
    byte        colorcount[256] = { 0 };

    for (int col = 0; col < w; col++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            int     count = column->length;

            while (count--)
                colorcount[*source++]++;

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }

    for (int i = 0; i < 256; i++)
    {
        const byte  red = *palette++;
        const byte  green = *palette++;
        const byte  blue = *palette++;

        if (colorcount[i] > dominantcolorcount && (red >= 128 || green >= 128 || blue >= 128))
        {
            dominantcolor = i;
            dominantcolorcount = colorcount[i];
        }
    }

    return dominantcolor;
}

static byte *GenerateTintTable(byte *palette, int percent, int colors)
{
    byte    *result = malloc(256 * 256);

    for (int foreground = 0; foreground < 256; foreground++)
        if ((filter[foreground] & colors) || colors == ALL || colors == ALTHUD)
            for (int background = 0; background < 256; background++)
            {
                byte    *color1 = &palette[background * 3];
                byte    *color2 = &palette[foreground * 3];

                // [crispy] blended color - emphasize blues
                // Color matching in RGB space doesn't work very well with the blues
                // in DOOM's palette. Rather than do any color conversions, just
                // emphasize the blues when building the translucency table.
                const int   btmp = (colors == ALTHUD && color1[2] * 1.666 >= (double)color1[0] + color1[1] ? 150 : 100);
                const int   r = ((int)color1[0] * percent + (int)color2[0] * (100 - percent)) / btmp;
                const int   g = ((int)color1[1] * percent + (int)color2[1] * (100 - percent)) / btmp;
                const int   b = ((int)color1[2] * percent + (int)color2[2] * (100 - percent)) / 100;

                result[(background << 8) + foreground] = FindNearestColor(palette, r, g, b);
            }
        else
            for (int background = 0; background < 256; background++)
                result[(background << 8) + foreground] = foreground;

    return result;
}

static byte *GenerateAdditiveTintTable(byte *palette, int colors)
{
    byte    *result = malloc(256 * 256);

    for (int foreground = 0; foreground < 256; foreground++)
        if ((filter[foreground] & colors) || colors == ALL)
            for (int background = 0; background < 256; background++)
            {
                byte    *color1 = &palette[background * 3];
                byte    *color2 = &palette[foreground * 3];
                int     r = MIN(color1[0] + color2[0], 255);
                int     g = MIN(color1[1] + color2[1], 255);
                int     b = MIN(color1[2] + color2[2], 255);

                result[(background << 8) + foreground] = FindNearestColor(palette, r, g, b);
            }
        else
            for (int background = 0; background < 256; background++)
                result[(background << 8) + foreground] = foreground;

    return result;
}

void I_InitTintTables(byte *palette)
{
    int lump = W_CheckNumForName("TRANMAP");

    tinttab15 = GenerateTintTable(palette, 15, ALL);
    tinttab20 = GenerateTintTable(palette, 20, ALL);
    tinttab25 = GenerateTintTable(palette, 25, ALL);
    tinttab30 = GenerateTintTable(palette, 30, ALL);
    tinttab33 = GenerateTintTable(palette, 33, ALL);
    tinttab40 = GenerateTintTable(palette, 40, ALL);
    tinttab50 = GenerateTintTable(palette, 50, ALL);
    tinttab60 = GenerateTintTable(palette, 60, ALL);
    tinttab66 = GenerateTintTable(palette, 66, ALL);
    tinttab75 = GenerateTintTable(palette, 75, ALL);

    alttinttab20 = GenerateTintTable(palette, 20, ALTHUD);
    alttinttab40 = GenerateTintTable(palette, 40, ALTHUD);
    alttinttab60 = GenerateTintTable(palette, 60, ALTHUD);

    tranmap = (lump != -1 ? W_CacheLumpNum(lump) : tinttab50);

    tinttabadditive = GenerateAdditiveTintTable(palette, ALL);
    tinttabred = GenerateAdditiveTintTable(palette, REDS);
    tinttabredwhite1 = GenerateAdditiveTintTable(palette, (REDS | WHITES));
    tinttabredwhite2 = GenerateAdditiveTintTable(palette, (REDS | WHITES | EXTRAS));
    tinttabgreen = GenerateAdditiveTintTable(palette, GREENS);
    tinttabblue = GenerateAdditiveTintTable(palette, BLUES);

    tinttabred33 = GenerateTintTable(palette, 33, REDS);
    tinttabredwhite50 = GenerateTintTable(palette, 50, (REDS | WHITES));
    tinttabgreen33 = GenerateTintTable(palette, 33, GREENS);
    tinttabblue25 = GenerateTintTable(palette, 25, BLUES);
}
