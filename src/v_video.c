/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (c) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (c) 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if defined(WIN32)
#pragma warning( disable : 4091 )
#include <Shlobj.h>
#endif

#include "c_console.h"
#include "d_deh.h"
#include "d_main.h"
#include "doomstat.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define WHITE   4

// Each screen is [SCREENWIDTH * SCREENHEIGHT];
byte            *screens[5];

int             pixelwidth;
int             pixelheight;
char            *r_lowpixelsize = r_lowpixelsize_default;

extern dboolean r_translucency;

//
// V_CopyRect
//
void V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height,
                int destx, int desty, int destscrn)
{
    byte        *src;
    byte        *dest;

    src = screens[srcscrn] + SCREENWIDTH * srcy + srcx;
    dest = screens[destscrn] + SCREENWIDTH * desty + destx;

    for (; height > 0; height--)
    {
        memcpy(dest, src, width);
        src += SCREENWIDTH;
        dest += SCREENWIDTH;
    }
}

//
// V_FillRect
//
void V_FillRect(int scrn, int x, int y, int width, int height, byte color)
{
    byte        *dest = screens[scrn] + y * SCREENWIDTH + x;

    while (height--)
    {
        memset(dest, color, width);
        dest += SCREENWIDTH;
    }
}

void V_FillTransRect(int x, int y, int width, int height, int color)
{
    byte        *dest = screens[0] + y * SCREENWIDTH + x;
    byte        *dot;
    int         xx, yy;

    color <<= 8;

    dot = dest - 1 - SCREENWIDTH * 2;
    *dot = tinttab20[*dot + color];
    dot += SCREENWIDTH;
    for (yy = 0; yy < height + 2; ++yy, dot += SCREENWIDTH)
        *dot = tinttab40[*dot + color];
    *dot = tinttab20[*dot + color];

    dot = dest - 2 - SCREENWIDTH;
    for (yy = 0; yy < height + 2; ++yy, dot += SCREENWIDTH)
        *dot = tinttab20[*dot + color];

    for (xx = 0; xx < width; ++xx)
    {
        dot = dest + xx - SCREENWIDTH * 2;
        *dot = tinttab20[*dot + color];
        dot += SCREENWIDTH;
        *dot = tinttab40[*dot + color];
        dot += SCREENWIDTH;
        for (yy = 0; yy < height; ++yy, dot += SCREENWIDTH)
            *dot = tinttab60[*dot + color];
        *dot = tinttab40[*dot + color];
        dot += SCREENWIDTH;
        *dot = tinttab20[*dot + color];
    }

    dot = dest + width - SCREENWIDTH * 2;
    *dot = tinttab20[*dot + color];
    dot += SCREENWIDTH;
    for (yy = 0; yy < height + 2; ++yy, dot += SCREENWIDTH)
        *dot = tinttab40[*dot + color];
    *dot = tinttab20[*dot + color];

    dot = dest + width + 1 - SCREENWIDTH;
    for (yy = 0; yy < height + 2; ++yy, dot += SCREENWIDTH)
        *dot = tinttab20[*dot + color];
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen.
//
void V_DrawPatch(int x, int y, int scrn, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[scrn] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                *dest = source[srccol >> FRACBITS];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPagePatch(patch_t *patch)
{
    short       width = SHORT(patch->width);
    short       height = SHORT(patch->height);

    DX = (SCREENWIDTH << FRACBITS) / width;
    DXI = (width << FRACBITS) / SCREENWIDTH;
    DY = (SCREENHEIGHT << FRACBITS) / height;
    DYI = (height << FRACBITS) / SCREENHEIGHT;

    V_DrawPatch(0, 0, 0, patch);

    DX = (SCREENWIDTH << FRACBITS) / ORIGINALWIDTH;
    DXI = (ORIGINALWIDTH << FRACBITS) / SCREENWIDTH;
    DY = (SCREENHEIGHT << FRACBITS) / ORIGINALHEIGHT;
    DYI = (ORIGINALHEIGHT << FRACBITS) / SCREENHEIGHT;
}

void V_DrawTranslucentPatch(int x, int y, int scrn, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[scrn] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                *dest = tinttab25[(*dest << 8) + source[srccol >> FRACBITS]];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawShadowPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if (--count)
            {
                *dest = tinttab25[*dest];
                dest += SCREENWIDTH;
            }
            while (--count > 0)
            {
                *dest = tinttab40[*dest];
                dest += SCREENWIDTH;
            }
            *dest = tinttab25[*dest];

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawSolidShadowPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if (--count)
            {
                *dest = 1;
                dest += SCREENWIDTH;
            }
            while (--count > 0)
            {
                *dest = 0;
                dest += SCREENWIDTH;
            }
            *dest = 1;

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawSpectreShadowPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if (--count)
            {
                if (!(rand() % 4))
                    *dest = tinttab25[*dest];
                dest += SCREENWIDTH;
            }
            while (--count > 0)
            {
                *dest = tinttab25[*dest];
                dest += SCREENWIDTH;
            }
            if (!(rand() % 4))
                *dest = tinttab25[*dest];

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawBigPatch(int x, int y, int scrn, patch_t *patch)
{
    int         col = 0;
    byte        *desttop = screens[scrn] + y * SCREENWIDTH + x;
    int         w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        int             td;
        int             topdelta = -1;
        int             lastlength = 0;

        // step through the posts in a column
        while ((td = column->topdelta) != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest;
            int         count;

            topdelta = (td < topdelta + lastlength - 1 ? topdelta + td : td);
            dest = desttop + topdelta * SCREENWIDTH;
            count = lastlength = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

int     italicize[15] = { 0, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1 };

void V_DrawConsoleChar(int x, int y, patch_t *patch, int color1, int color2, dboolean italics,
    byte *tinttab)
{
    int         col = 0;
    byte        *desttop = screens[0] + y * SCREENWIDTH + x;
    int         w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        byte            topdelta;

        // step through the posts in a column
        while ((topdelta = column->topdelta) != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + topdelta * SCREENWIDTH;
            byte        length = column->length;
            int         count = length;

            while (count--)
            {
                int     height = topdelta + length - count;

                if (y + height > CONSOLETOP)
                {
                    if (color2 == -1)
                    {
                        if (*source)
                            if (italics)
                                *(dest + italicize[height]) = color1;
                            else
                                *dest = (!tinttab ? color1 : tinttab[(color1 << 8) + *dest]);
                    }
                    else if (*source == WHITE)
                        *dest = color1;
                    else if (*dest != color1)
                        *dest = color2;
                }
                ++source;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

dboolean V_EmptyPatch(patch_t *patch)
{
    int col = 0;
    int w = SHORT(patch->width);

    for (; col < w; col++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            if (column->length)
                return false;

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }

    return true;
}

extern byte *tempscreen;

void V_DrawPatchToTempScreen(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = tempscreen + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                *dest = source[srccol >> FRACBITS];
                dest += SCREENWIDTH;
                *(dest + SCREENWIDTH + 2) = 0;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPatchWithShadow(int x, int y, patch_t *patch, dboolean flag)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                int height = (((y + column->topdelta + column->length) * DY) >> FRACBITS) - count;

                if (height > 0)
                    *dest = source[srccol >> FRACBITS];
                dest += SCREENWIDTH;
                if (height + 2 > 0)
                {
                    byte        *shadow = dest + SCREENWIDTH + 2;

                    if (!flag || (*shadow != 47 && *shadow != 191))
                        *shadow = (r_translucency ? tinttab50[*shadow] : 0);
                }
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawHUDPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int         col = 0;
    byte        *desttop;
    int         w;

    if (!tinttab)
        return;

    desttop = screens[0] + y * SCREENWIDTH + x;
    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + column->topdelta * SCREENWIDTH;
            int         count = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawHighlightedHUDNumberPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int         col = 0;
    byte        *desttop;
    int         w;

    if (!tinttab)
        return;

    desttop = screens[0] + y * SCREENWIDTH + x;
    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + column->topdelta * SCREENWIDTH;
            int         count = column->length;

            while (count--)
            {
                byte    dot = *source++;

                *dest = (dot == 109 && tinttab ? tinttab33[*dest] : dot);
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawYellowHUDPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int         col = 0;
    byte        *desttop;
    int         w;

    if (!tinttab)
        return;

    desttop = screens[0] + y * SCREENWIDTH + x;
    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + column->topdelta * SCREENWIDTH;
            int         count = column->length;

            while (count--)
            {
                *dest = redtoyellow[*source++];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int         col = 0;
    byte        *desttop = screens[0] + y * SCREENWIDTH + x;
    int         w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + column->topdelta * SCREENWIDTH;
            int         count = column->length;

            while (count--)
            {
                *dest = tinttab[(*source++ << 8) + *dest];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDNumberPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int         col = 0;
    byte        *desttop = screens[0] + y * SCREENWIDTH + x;
    int         w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + column->topdelta * SCREENWIDTH;
            int         count = column->length;

            while (count--)
            {
                byte    dot = *source++;

                *dest = (dot == 109 ? tinttab33[*dest] : tinttab[(dot << 8) + *dest]);
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentYellowHUDPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int         col = 0;
    byte        *desttop = screens[0] + y * SCREENWIDTH + x;
    int         w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + column->topdelta * SCREENWIDTH;
            int         count = column->length;

            while (count--)
            {
                *dest = tinttab75[(redtoyellow[*source++] << 8) + *dest];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawAltHUDPatch(int x, int y, patch_t *patch, int from, int to)
{
    int         col = 0;
    byte        *desttop = screens[0] + y * SCREENWIDTH + x;
    int         w = SHORT(patch->width);

    to <<= 8;

    for (; col < w; col++, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + column->topdelta * SCREENWIDTH;
            int         count = column->length;

            while (count--)
            {
                byte    dot = *source++;

                *dest = tinttab60[(dot == from ? to : (dot << 8)) + *dest];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentRedPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                *dest = tinttabred[(*dest << 8) + source[srccol >> FRACBITS]];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

//
// V_DrawFlippedPatch
//
// The co-ordinates for this procedure are always based upon a
// 320x200 screen and multiplies the size of the patch by the
// scaledwidth & scaledheight. The purpose of this is to produce
// a clean and undistorted patch upon the screen, The scaled screen
// size is based upon the nearest whole number ratio from the
// current screen size to 320x200.
//
// This Procedure flips the patch horizontally.
//
void V_DrawFlippedPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
                            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                *dest = source[srccol >> FRACBITS];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedShadowPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
                            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if (--count)
            {
                *dest = tinttab25[*dest];
                dest += SCREENWIDTH;
            }
            while (--count > 0)
            {
                *dest = tinttab40[*dest];
                dest += SCREENWIDTH;
            }
            *dest = tinttab25[*dest];

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedSolidShadowPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if (--count)
            {
                *dest = 1;
                dest += SCREENWIDTH;
            }
            while (--count > 0)
            {
                *dest = 0;
                dest += SCREENWIDTH;
            }
            *dest = 1;

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedSpectreShadowPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if (--count)
            {
                if (!(rand() % 4))
                    *dest = tinttab25[*dest];
                dest += SCREENWIDTH;
            }
            while (--count > 0)
            {
                *dest = tinttab25[*dest];
                dest += SCREENWIDTH;
            }
            if (!(rand() % 4))
                *dest = tinttab25[*dest];

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedTranslucentRedPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
                            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                *dest = tinttabred[(*dest << 8) + source[srccol >> FRACBITS]];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

#define _FUZZ(a, b)     _fuzzrange[M_RandomInt(a + 1, b + 1)]

const int       _fuzzrange[3] = { -SCREENWIDTH, 0, SCREENWIDTH };

extern int      fuzztable[SCREENWIDTH * SCREENHEIGHT];
extern dboolean menuactive;
extern dboolean paused;

void V_DrawFuzzPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;
    int         _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;

            while (count--)
            {
                if (!menuactive && !paused && !consoleactive)
                    fuzztable[_fuzzpos] = _FUZZ(-1, 1);
                *dest = fullcolormap[6 * 256 + dest[fuzztable[_fuzzpos++]]];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedFuzzPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;
    int         _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch
                        + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;

            while (count--)
            {
                if (!menuactive && !paused && !consoleactive)
                    fuzztable[_fuzzpos] = _FUZZ(-1, 1);
                *dest = fullcolormap[6 * 256 + dest[fuzztable[_fuzzpos++]]];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

byte nogreen[256] =
{
    1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

void V_DrawNoGreenPatchWithShadow(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                byte    src = source[srccol >> FRACBITS];

                if (nogreen[src])
                {
                    byte        *shadow;

                    *dest = src;

                    shadow = dest + SCREENWIDTH * 2 + 2;
                    if (*shadow != 47 && *shadow != 191)
                        *shadow = tinttab50[*shadow];
                }
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawCenteredPatch(int y, patch_t *patch)
{
    V_DrawPatch((ORIGINALWIDTH - SHORT(patch->width)) / 2, y, 0, patch);
}

void V_DrawTranslucentNoGreenPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int         count = (column->length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count--)
            {
                byte src = source[srccol >> FRACBITS];

                if (nogreen[src])
                    *dest = (r_translucency ? tinttab33[(*dest << 8) + src] : src);
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//
void V_DrawBlock(int x, int y, int width, int height, byte *src)
{
    byte        *dest;

    dest = screens[0] + y * SCREENWIDTH + x;

    while (height--)
    {
        memcpy(dest, src, width);
        src += width;
        dest += SCREENWIDTH;
    }
}

void V_DrawPixel(int x, int y, byte color, dboolean shadow)
{
    byte        *dest = &screens[0][y * SCREENSCALE * SCREENWIDTH + x * SCREENSCALE];

    if (color == 251)
    {
        if (shadow)
        {
            int xx, yy;

            if (r_translucency)
            {
                for (yy = 0; yy < SCREENSCALE; ++yy)
                    for (xx = 0; xx < SCREENSCALE; ++xx)
                        *(dest + yy * SCREENWIDTH + xx) = tinttab50[*(dest + yy * SCREENWIDTH
                            + xx)];
            }
            else
            {
                for (yy = 0; yy < SCREENSCALE; ++yy)
                    for (xx = 0; xx < SCREENSCALE; ++xx)
                        *(dest + yy * SCREENWIDTH + xx) = 0;
            }
        }
    }
    else if (color && color != 32)
    {
        int xx, yy;

        for (yy = 0; yy < SCREENSCALE; ++yy)
            for (xx = 0; xx < SCREENSCALE; ++xx)
                *(dest + yy * SCREENWIDTH + xx) = color;
    }
}

void GetPixelSize(void)
{
    int     width = -1;
    int     height = -1;
    char    *left = strtok(strdup(r_lowpixelsize), "x");
    char    *right = strtok(NULL, "x");

    if (!right)
        right = "";

    sscanf(left, "%10i", &width);
    sscanf(right, "%10i", &height);

    if (width > 0 && width <= SCREENWIDTH && height > 0 && height <= SCREENHEIGHT
        && (width >= 2 || height >= 2))
    {
        pixelwidth = width;
        pixelheight = height;
    }
    else
    {
        pixelwidth = 2;
        pixelheight = 2;
        r_lowpixelsize = r_lowpixelsize_default;

        M_SaveCVARs();
    }
}

void V_LowGraphicDetail(int height)
{
    int x, y;
    int h = pixelheight * SCREENWIDTH;

    for (y = 0; y < height; y += h)
        for (x = 0; x < SCREENWIDTH; x += pixelwidth)
        {
            byte        *dot = screens[0] + y + x;
            int         xx, yy;

            for (yy = 0; yy < h; yy += SCREENWIDTH)
                for (xx = 0; xx < pixelwidth; xx++)
                    *(dot + yy + xx) = *dot;
        }
}

//
// V_Init
//
void V_Init(void)
{
    int         i;
    byte        *base = Z_Malloc(SCREENWIDTH * SCREENHEIGHT * 4, PU_STATIC, NULL);

    for (i = 0; i < 4; i++)
        screens[i] = base + i * SCREENWIDTH * SCREENHEIGHT;

    DX = (SCREENWIDTH << FRACBITS) / ORIGINALWIDTH;
    DXI = (ORIGINALWIDTH << FRACBITS) / SCREENWIDTH;
    DY = (SCREENHEIGHT << FRACBITS) / ORIGINALHEIGHT;
    DYI = (ORIGINALHEIGHT << FRACBITS) / SCREENHEIGHT;

    GetPixelSize();
}

#if !defined(MAX_PATH)
#define MAX_PATH        260
#endif

char                    lbmname[MAX_PATH];
char                    lbmpath[MAX_PATH];

extern dboolean         vid_widescreen;
extern dboolean         inhelpscreens;
extern char             maptitle[128];
extern dboolean         splashscreen;
extern int              titlesequence;

dboolean V_SaveBMP(SDL_Window *window, char *path)
{
    dboolean            result = false;
    SDL_Surface         *surface = SDL_GetWindowSurface(window);

    if (surface)
    {
        unsigned char   *pixels = malloc(surface->w * surface->h * 4);

        if (pixels)
        {
            SDL_Renderer        *renderer = SDL_GetRenderer(window);
            SDL_Rect            rect = surface->clip_rect;

            rect.w = (vid_widescreen ? rect.h * 16 / 10 : rect.h * 4 / 3);
            rect.x = (surface->w - rect.w) / 2;
            if (renderer && !SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_ARGB8888,
                pixels, rect.w * 4))
            {
                SDL_Surface     *screenshot = SDL_CreateRGBSurfaceFrom(pixels, rect.w, rect.h, 32,
                    rect.w * 4, 0, 0, 0, 0);

                if (screenshot)
                {
                    result = !SDL_SaveBMP(screenshot, path);
                    SDL_FreeSurface(screenshot);
                }
            }

            free(pixels);
        }

        SDL_FreeSurface(surface);
    }

    return result;
}

dboolean V_ScreenShot(void)
{
    dboolean    result = false;
    char        mapname[128];
    char        folder[MAX_PATH] = "";
    int         count = 0;

#if defined(WIN32)
    HRESULT     hr = SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, folder);

    if (hr != S_OK)
        return false;
#endif

    switch (gamestate)
    {
        case GS_INTERMISSION:
            M_StringCopy(mapname, "Intermission", sizeof(mapname));
            break;

        case GS_FINALE:
            M_StringCopy(mapname, "Finale", sizeof(mapname));
            break;

        case GS_TITLESCREEN:
            M_StringCopy(mapname, (splashscreen ? "Splash" :
                (titlesequence == 1 ? "Credits" : "Title")), sizeof(mapname));
            break;

        default:
            M_StringCopy(mapname, (inhelpscreens ? "Help" : titlecase(maptitle)), sizeof(mapname));
            break;
    }

    if (sscanf(mapname, "The %124[^\n]", mapname))
        M_snprintf(mapname, sizeof(mapname), "%s, The", mapname);
    else if (sscanf(mapname, "A %126[^\n]", mapname))
        M_snprintf(mapname, sizeof(mapname), "%s, A", mapname);

    do
    {
        if (!count)
            M_snprintf(lbmname, sizeof(lbmname), "%s.bmp", makevalidfilename(mapname));
        else
            M_snprintf(lbmname, sizeof(lbmname), "%s (%i).bmp", makevalidfilename(mapname), count);
        ++count;
        M_snprintf(lbmpath, sizeof(lbmpath), "%s" DIR_SEPARATOR_S PACKAGE_NAME, folder);
        M_MakeDirectory(lbmpath);
        M_snprintf(lbmpath, sizeof(lbmpath), "%s" DIR_SEPARATOR_S "%s", lbmpath, lbmname);
    } while (M_FileExists(lbmpath));

    result = V_SaveBMP(window, lbmpath);

    if (mapwindow && result && gamestate == GS_LEVEL)
    {
        C_Output(s_GSCREENSHOT, lbmname);

        do
        {
            M_snprintf(lbmname, sizeof(lbmname), "%s (%i).bmp", makevalidfilename(mapname), count);
            ++count;
            M_snprintf(lbmpath, sizeof(lbmpath), "%s" DIR_SEPARATOR_S PACKAGE_NAME, folder);
            M_MakeDirectory(lbmpath);
            M_snprintf(lbmpath, sizeof(lbmpath), "%s" DIR_SEPARATOR_S "%s", lbmpath, lbmname);
        } while (M_FileExists(lbmpath));

        result = V_SaveBMP(mapwindow, lbmpath);
    }

    return result;
}

void V_AverageColorInPatch(patch_t *patch, int *red, int *green, int *blue, int *total)
{
    int         col = 0;
    int         w = SHORT(patch->width);
    int         red1 = 0, blue1 = 0, green1 = 0;
    byte        *playpal = W_CacheLumpName("PLAYPAL", PU_CACHE);

    for (; col < w; col++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        int             td;
        int             topdelta = -1;
        int             lastlength = 0;

        // step through the posts in a column
        while ((td = column->topdelta) != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            int         count;

            topdelta = (td < topdelta + lastlength - 1 ? topdelta + td : td);
            count = lastlength = column->length;

            while (count--)
            {
                byte    color = *source++;
                byte    r = playpal[color * 3];
                byte    g = playpal[color * 3 + 1];
                byte    b = playpal[color * 3 + 2];

                if (!red1)
                {
                    red1 = r;
                    blue1 = g;
                    green1 = b;
                    continue;
                }
                else if (r == red1 && g == green1 && b == blue1)
                    continue;

                *red += r;
                *green += g;
                *blue += b;
                (*total)++;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}
