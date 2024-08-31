/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <stdlib.h>

#include "doomstat.h"
#include "i_colors.h"
#include "i_swap.h"
#include "w_wad.h"

#define R   1
#define W   2
#define G   4
#define B   8
#define X  16

static const byte filter[256] =
{
    0,   X,   0,   0,   R|B, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //   0 to  15
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   //  16 to  31
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   //  32 to  47
    W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   W,   //  48 to  63
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   X,   X,   X,   //  64 to  79
    R,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //  80 to  95
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //  96 to 111
    G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   G,   // 112 to 127
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 128 to 143
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 144 to 159
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   // 160 to 175
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   // 176 to 191
    B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   B,   // 192 to 207
    R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   // 208 to 223
    R|B, R|B, R,   R,   R,   R,   R,   R,   X,   X,   X,   X,   0,   0,   0,   0,   // 224 to 239
    B,   B,   B,   B,   B,   B,   B,   B,   R,   R,   0,   0,   0,   0,   0,   0    // 240 to 255
};

#define ALL     0
#define REDS    R
#define WHITES  W
#define GREENS  G
#define BLUES   B
#define EXTRAS  X

typedef struct vect
{
    double  x;
    double  y;
    double  z;
} vect;

byte    *tinttab4;
byte    *tinttab5;
byte    *tinttab10;
byte    *tinttab15;
byte    *tinttab20;
byte    *tinttab25;
byte    *tinttab30;
byte    *tinttab33;
byte    *tinttab40;
byte    *tinttab45;
byte    *tinttab50;
byte    *tinttab60;
byte    *tinttab66;
byte    *tinttab70;
byte    *tinttab75;
byte    *tinttab80;
byte    *tinttab90;

byte    *tranmap;

byte    *tinttabadditive;
byte    *tinttabred;
byte    *tinttabredwhite1;
byte    *tinttabredwhite2;
byte    *tinttabgreen;
byte    *tinttabblue;

byte    *tinttabred33;
byte    *tinttabredwhite50;
byte    *tinttabgreen33;
byte    *tinttabblue25;

byte    nearestcolors[256];
byte    nearestblack;
byte    nearestdarkblue;
byte    nearestdarkgray;
byte    nearestgold;
byte    nearestgreen;
byte    nearestlightgray;
byte    nearestred;
byte    nearestwhite;

byte    *black25;
byte    *black40;
byte    *black45;
byte    *black75;
byte    *gold4;
byte    *white5;
byte    *white25;
byte    *white33;
byte    *white75;

int FindNearestColor(byte *palette, const byte red, const byte green, const byte blue)
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
    byte    *playpal = W_CacheLastLumpName("PLAYPAL");

    for (int i = 0; i < 256; i++)
    {
        const byte  red = *playpal++;
        const byte  green = *playpal++;
        const byte  blue = *playpal++;

        nearestcolors[i] = FindNearestColor(palette, red, green, blue);
    }

    nearestblack = nearestcolors[BLACK];
    nearestdarkblue = nearestcolors[DARKBLUE];
    nearestdarkgray = nearestcolors[DARKGRAY2];
    nearestgold = nearestcolors[YELLOW2];
    nearestgreen = nearestcolors[GREEN4];
    nearestlightgray = nearestcolors[LIGHTGRAY3];
    nearestred = nearestcolors[RED1];
    nearestwhite = nearestcolors[WHITE];

    black25 = &tinttab25[nearestblack << 8];
    black40 = &tinttab40[nearestblack << 8];
    black45 = &tinttab45[nearestblack << 8];
    black75 = &tinttab75[nearestblack << 8];
    gold4 = &tinttab4[nearestgold << 8];
    white5 = &tinttab5[nearestwhite << 8];
    white25 = &tinttab25[nearestwhite << 8];
    white33 = &tinttab33[nearestwhite << 8];
    white75 = &tinttab75[nearestwhite << 8];
}

int FindBrightDominantColor(patch_t *patch)
{
    int         color = 0;
    int         colors[256] = { 0 };
    const int   width = SHORT(patch->width);
    byte        *playpal = PLAYPAL;

    for (int x = 0; x < width; x++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[x]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            const int   length = column->length;

            for (int y = 0; y < length; y++)
                colors[*source++]++;

            column = (column_t *)((byte *)column + length + 4);
        }
    }

    for (int i = 0, dominant = 1; i < 256; i++)
    {
        const byte  red = *playpal++;
        const byte  green = *playpal++;
        const byte  blue = *playpal++;

        if (colors[i] > dominant && (red >= 128 || green >= 128 || blue >= 128))
        {
            color = i;
            dominant = colors[i];
        }
    }

    if (!color)
        for (int i = 0, dominant = 1; i < 256; i++)
        {
            const byte  red = *playpal++;
            const byte  green = *playpal++;
            const byte  blue = *playpal++;

            if (colors[i] > dominant && (red >= 64 || green >= 64 || blue >= 64))
            {
                color = i;
                dominant = colors[i];
            }
        }

    return (color ? color : nearestwhite);
}

int FindDominantEdgeColor(patch_t *patch, const int maxlength, const int edge)
{
    int         color = 0;
    const int   width = SHORT(patch->width);

    if (width >= edge)
    {
        int colors[256] = { 0 };

        for (int x = 0; x < edge; x++)
        {
            column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[x]));
            byte        *source = (byte *)column + 3;
            const int   length = MIN(column->length, maxlength);

            for (int y = 0; y < length; y++)
                colors[*source++]++;
        }

        for (int x = width - edge; x < width; x++)
        {
            column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[x]));
            byte        *source = (byte *)column + 3;
            const int   length = MIN(column->length, maxlength);

            for (int y = 0; y < length; y++)
                colors[*source++]++;
        }

        colors[nearestblack] /= 2;

        for (int i = 0, dominant = 0; i < 256; i++)
            if (colors[i] > dominant)
            {
                color = i;
                dominant = colors[i];
            }
    }

    return color;
}

static byte *GenerateTintTable(byte *palette, int percent, int colors)
{
    byte    *result = malloc((size_t)256 * 256);

    if (result)
    {
        for (int foreground = 0; foreground < 256; foreground++)
            if ((filter[foreground] & colors) || colors == ALL)
                for (int background = 0; background < 256; background++)
                {
                    byte        *color1 = &palette[background * 3];
                    byte        *color2 = &palette[foreground * 3];
                    const byte  r = ((byte)color1[0] * percent + (byte)color2[0] * (100 - percent)) / 100;
                    const byte  g = ((byte)color1[1] * percent + (byte)color2[1] * (100 - percent)) / 100;
                    const byte  b = ((byte)color1[2] * percent + (byte)color2[2] * (100 - percent)) / 100;

                    result[(background << 8) + foreground] = FindNearestColor(palette, r, g, b);
                }
            else
                for (int background = 0; background < 256; background++)
                    result[(background << 8) + foreground] = foreground;
    }

    return result;
}

static byte *GenerateAdditiveTintTable(byte *palette, int colors)
{
    byte    *result = malloc((size_t)256 * 256);

    if (result)
    {
        for (int foreground = 0; foreground < 256; foreground++)
            if ((filter[foreground] & colors) || colors == ALL)
                for (int background = 0; background < 256; background++)
                {
                    const byte  *color1 = &palette[background * 3];
                    const byte  *color2 = &palette[foreground * 3];
                    const byte  r = MIN(color1[0] + color2[0], 255);
                    const byte  g = MIN(color1[1] + color2[1], 255);
                    const byte  b = MIN(color1[2] + color2[2], 255);

                    result[(background << 8) + foreground] = FindNearestColor(palette, r, g, b);
                }
            else
                for (int background = 0; background < 256; background++)
                    result[(background << 8) + foreground] = foreground;
    }

    return result;
}

void I_InitTintTables(byte *palette)
{
    const int   lump = W_CheckNumForName("TRANMAP");

    tinttab4 = GenerateTintTable(palette, 4, ALL);
    tinttab5 = GenerateTintTable(palette, 5, ALL);
    tinttab10 = GenerateTintTable(palette, 10, ALL);
    tinttab15 = GenerateTintTable(palette, 15, ALL);
    tinttab20 = GenerateTintTable(palette, 20, ALL);
    tinttab25 = GenerateTintTable(palette, 25, ALL);
    tinttab30 = GenerateTintTable(palette, 30, ALL);
    tinttab33 = GenerateTintTable(palette, 33, ALL);
    tinttab40 = GenerateTintTable(palette, 40, ALL);
    tinttab45 = GenerateTintTable(palette, 45, ALL);
    tinttab50 = GenerateTintTable(palette, 50, ALL);
    tinttab60 = GenerateTintTable(palette, 60, ALL);
    tinttab66 = GenerateTintTable(palette, 66, ALL);
    tinttab70 = GenerateTintTable(palette, 70, ALL);
    tinttab75 = GenerateTintTable(palette, 75, ALL);
    tinttab80 = GenerateTintTable(palette, 80, ALL);
    tinttab90 = GenerateTintTable(palette, 90, ALL);

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

static void HSVtoRGB(vect *hsv, vect *rgb)
{
    double  h = hsv->x * 360.0;
    double  s = hsv->y;
    double  v = hsv->z;

    if (s < CTOLERANCE)
    {
        rgb->x = v;
        rgb->y = v;
        rgb->z = v;
    }
    else
    {
        int     i;
        double  f;
        double  p;
        double  q;
        double  t;

        if (h >= 360.0)
            h -= 360.0;

        h /= 60.0;
        i = (int)floor(h);
        f = h - i;
        p = v * (1.0 - s);
        q = v * (1.0 - s * f);
        t = v * (1.0 - s * (1.0 - f));

        switch (i)
        {
            case 0:
                rgb->x = v;
                rgb->y = t;
                rgb->z = p;
                break;

            case 1:
                rgb->x = q;
                rgb->y = v;
                rgb->z = p;
                break;

            case 2:
                rgb->x = p;
                rgb->y = v;
                rgb->z = t;
                break;

            case 3:
                rgb->x = p;
                rgb->y = q;
                rgb->z = v;
                break;

            case 4:
                rgb->x = t;
                rgb->y = p;
                rgb->z = v;
                break;

            case 5:
                rgb->x = v;
                rgb->y = p;
                rgb->z = q;
                break;
        }
    }
}

static void RGBtoHSV(vect *rgb, vect *hsv)
{
    double  h;
    double  s;
    double  v;
    double  r = rgb->x;
    double  g = rgb->y;
    double  b = rgb->z;
    double  cmax = r;
    double  cmin = r;

    cmax = (g > cmax ? g : cmax);
    cmin = (g < cmin ? g : cmin);
    cmax = (b > cmax ? b : cmax);
    cmin = (b < cmin ? b : cmin);

    v = cmax;
    s = (cmax > CTOLERANCE ? (cmax - cmin) / cmax : 0.0);

    if (s < CTOLERANCE)
        h = 0.0;
    else
    {
        double  cdelta = cmax - cmin;
        double  rc = (cmax - r) / cdelta;
        double  gc = (cmax - g) / cdelta;
        double  bc = (cmax - b) / cdelta;

        h = (r == cmax ? bc - gc : (g == cmax ? 2.0 + rc - bc : 4.0 + gc - rc)) * 60.0;

        if (h < 0.0)
            h += 360.0;
    }

    hsv->x = h / 360.0;
    hsv->y = s;
    hsv->z = v;
}

byte I_GoldTranslation(byte *playpal, byte color)
{
    vect rgb;
    vect hsv;

    rgb.x = playpal[color * 3] / 255.0;
    rgb.y = playpal[color * 3 + 1] / 255.0;
    rgb.z = playpal[color * 3 + 2] / 255.0;

    RGBtoHSV(&rgb, &hsv);

    hsv.x = (7.0 + 53.0 * hsv.z) / 360.0;
    hsv.y = 1.0 - 0.4 * hsv.z;
    hsv.z = (hsv.z < 0.2 ? hsv.z : 0.2) + 0.8 * hsv.z;

    HSVtoRGB(&hsv, &rgb);

    rgb.x *= 255.0;
    rgb.y *= 255.0;
    rgb.z *= 255.0;

    return FindNearestColor(playpal, (int)rgb.x, (int)rgb.y, (int)rgb.z);
}
