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

#if defined(WIN32)
#include <Shlobj.h>
#endif

#include "c_console.h"
#include "d_main.h"
#include "doomstat.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "v_video.h"
#include "version.h"
#include "z_zone.h"

#define WHITE   4

// Each screen is [SCREENWIDTH * SCREENHEIGHT];
byte            *screens[5];

int             pixelwidth;
int             pixelheight;
char            *pixelsize = PIXELSIZE_DEFAULT;

extern boolean  translucency;

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

//
// V_DrawPatch
// Masks a column based masked pic to the screen.
//
void V_DrawPatch(int x, int y, int scrn, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[scrn] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                *dest = source[srccol >> 16];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentPatch(int x, int y, int scrn, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[scrn] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                *dest = tinttab25[(*dest << 8) + source[srccol >> 16]];
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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> 16) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> 16) + 1;

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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> 16) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> 16) + 1;

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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> 16) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> 16) + 1;

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

int     italicize[15] = { 0, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1 };

void V_DrawConsoleChar(int x, int y, patch_t *patch, int color1, int color2, boolean italics,
    int translucency)
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
                int     height = column->topdelta + column->length - count;

                if (y + height > CONSOLETOP)
                {
                    if (color2 == -1)
                    {
                        if (*source)
                            if (italics)
                                *(dest + italicize[height]) = color1;
                            else
                                *dest = (translucency == 1 ? tinttab25[(color1 << 8) + *dest] :
                                    (translucency == 2 ? tinttab25[(*dest << 8) + color1] :
                                    color1));
                    }
                    else if (*source == WHITE)
                        *dest = color1;
                    else if (*dest != color1)
                        *dest = color2;
                }
                *(source++);
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

boolean V_EmptyPatch(patch_t *patch)
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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = tempscreen + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                *dest = source[srccol >> 16];
                dest += SCREENWIDTH;
                *(dest + SCREENWIDTH + 2) = 0;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPatchWithShadow(int x, int y, patch_t *patch, boolean flag)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                byte    *shadow;

                *dest = source[srccol >> 16];
                dest += SCREENWIDTH;
                shadow = dest + SCREENWIDTH + 2;
                if (!flag || (*shadow != 47 && *shadow != 191))
                    *shadow = (translucency ? tinttab50[*shadow] : 0);
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawHUDPatch(int x, int y, patch_t *patch, boolean invert)
{
    int         col = 0;
    byte        *desttop;
    int         w;

    if (!invert)
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

void V_DrawYellowHUDPatch(int x, int y, patch_t *patch, boolean invert)
{
    int         col = 0;
    byte        *desttop;
    int         w;

    if (!invert)
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

void V_DrawTranslucentHUDPatch(int x, int y, patch_t *patch, boolean invert)
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
                *dest = tinttab75[(*source++ << (8 * invert)) + (*dest << (8 * !invert))];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDNumberPatch(int x, int y, patch_t *patch, boolean invert)
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

                if (dot == 109 && invert)
                    *dest = tinttab33[*dest];
                else
                    *dest = tinttab75[(dot << (8 * invert)) + (*dest << (8 * !invert))];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentYellowHUDPatch(int x, int y, patch_t *patch, boolean invert)
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
                *dest = tinttab75[(redtoyellow[*source++] << (8 * invert)) + (*dest << (8 * !invert))];
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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                *dest = tinttabred[(*dest << 8) + source[srccol >> 16]];
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
// a clean and undistorted patch opon the screen, The scaled screen
// size is based upon the nearest whole number ratio from the
// current screen size to 320x200.
//
// This Procedure flips the patch horizontally.
//
void V_DrawFlippedPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
                            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                *dest = source[srccol >> 16];
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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
                            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> 16) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> 16) + 1;

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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> 16) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> 16) + 1;

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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY / 10) >> 16) * SCREENWIDTH;
            int         count = ((column->length * DY / 10) >> 16) + 1;

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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch
                            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                *dest = tinttabred[(*dest << 8) + source[srccol >> 16]];
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
extern boolean  menuactive;
extern boolean  paused;

void V_DrawFuzzPatch(int x, int y, patch_t *patch)
{
    int         col = 0;
    byte        *desttop;
    int         w = SHORT(patch->width) << 16;
    int         _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;

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
    int         w = SHORT(patch->width) << 16;
    int         _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch
                        + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        while (column->topdelta != 0xff)
        {
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;

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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                byte    src = source[srccol >> 16];

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
    int         w = SHORT(patch->width) << 16;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> 16) * SCREENWIDTH + ((x * DX) >> 16);

    for (; col < w; col += DXI, desttop++)
    {
        column_t        *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            int         count = (column->length * DY) >> 16;
            int         srccol = 0;

            while (count--)
            {
                byte src = source[srccol >> 16];

                if (nogreen[src])
                    *dest = (translucency ? tinttab33[(*dest << 8) + src] : src);
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

void V_DrawPixel(int x, int y, byte color, boolean shadow)
{
    byte        *dest = &screens[0][y * SCREENSCALE * SCREENWIDTH + x * SCREENSCALE];

    if (color == 251)
    {
        if (shadow)
        {
            int xx, yy;

            if (translucency)
            {
                for (yy = 0; yy < SCREENSCALE; ++yy)
                    for (xx = 0; xx < SCREENSCALE; ++xx)
                        *(dest + yy * SCREENWIDTH + xx) = tinttab50[*(dest + yy * SCREENWIDTH + xx)];
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

void V_LowGraphicDetail(int height)
{
    int x, y;
    int h = pixelheight * SCREENWIDTH;

    height *= SCREENWIDTH;

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

    DX = (SCREENWIDTH << 16) / ORIGINALWIDTH;
    DXI = (ORIGINALWIDTH << 16) / SCREENWIDTH;
    DY = (SCREENHEIGHT << 16) / ORIGINALHEIGHT;
    DYI = (ORIGINALHEIGHT << 16) / SCREENHEIGHT;
}

#if !defined(MAX_PATH)
#define MAX_PATH        260
#endif

char                    lbmname[MAX_PATH];
char                    lbmpath[MAX_PATH];

extern boolean          widescreen;
extern boolean          inhelpscreens;
extern char             maptitle[128];
extern boolean          splashscreen;
extern int              titlesequence;

extern SDL_Window       *window;
extern SDL_Renderer     *renderer;

boolean V_ScreenShot(void)
{
    boolean     result = false;
    char        mapname[128];
    char        folder[MAX_PATH] = "";
    int         count = 0;
    SDL_Surface *screenshot = NULL;
    SDL_Surface *surface;

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
            M_StringCopy(mapname, (inhelpscreens ? "Help" : maptitle), sizeof(mapname));
            break;
    }

    if (sscanf(mapname, "The %124[^\n]", mapname))
        M_snprintf(mapname, sizeof(mapname), "%s, The", mapname);
    else if (sscanf(mapname, "A %126[^\n]", mapname))
        M_snprintf(mapname, sizeof(mapname), "%s, A", mapname);

    do
    {
        if (!count)
            M_snprintf(lbmname, sizeof(lbmname), "%s.bmp", mapname);
        else
            M_snprintf(lbmname, sizeof(lbmname), "%s (%i).bmp", mapname, count);
        count++;
        M_snprintf(lbmpath, sizeof(lbmpath), "%s" DIR_SEPARATOR_S PACKAGE_NAME, folder);
        M_MakeDirectory(lbmpath);
        M_snprintf(lbmpath, sizeof(lbmpath), "%s" DIR_SEPARATOR_S "%s", lbmpath, lbmname);
    } while (M_FileExists(lbmpath));

    surface = SDL_GetWindowSurface(window);
    if (surface)
    {
        unsigned char   *pixels = malloc(surface->w * surface->h * surface->format->BytesPerPixel);

        if (pixels)
        {
            if (!SDL_RenderReadPixels(renderer, &surface->clip_rect, surface->format->format,
                pixels, surface->w * surface->format->BytesPerPixel))
            {
                screenshot = SDL_CreateRGBSurfaceFrom(pixels, surface->w, surface->h,
                    surface->format->BitsPerPixel, surface->w * surface->format->BytesPerPixel,
                    surface->format->Rmask, surface->format->Gmask, surface->format->Bmask,
                    surface->format->Amask);

                if (screenshot)
                {
                    result = !SDL_SaveBMP(screenshot, lbmpath);
                    SDL_FreeSurface(screenshot);
                    screenshot = NULL;
                    free(pixels);
                    SDL_FreeSurface(surface);
                    surface = NULL;
                }
            }
        }
    }
    return result;
}
