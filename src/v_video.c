/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifdef WIN32
#include <Shlobj.h>
#endif

#include "doomstat.h"
#include "d_main.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "v_video.h"
#include "version.h"
#include "z_zone.h"

// Each screen is [SCREENWIDTH * SCREENHEIGHT];
byte            *screens[5];

int             pixelwidth = PIXELWIDTH_DEFAULT;
int             pixelheight = PIXELHEIGHT_DEFAULT;

extern byte     redtoyellow[];

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
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;
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

void V_DrawScaledPatch(int x, int y, int scrn, int scale, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    int         dx = ((ORIGINALWIDTH * scale) << 16) / ORIGINALWIDTH;
    int         dxi = (ORIGINALWIDTH << 16) / (ORIGINALWIDTH * scale);
    int         dy = ((ORIGINALHEIGHT * scale) << 16) / ORIGINALHEIGHT;
    int         dyi = (ORIGINALHEIGHT << 16) / (ORIGINALHEIGHT * scale);

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * dx) >> 16;
    stretchy = (y * dy) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += dxi, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * dy) >> 16) * SCREENWIDTH;
            count = (column->length * dy) >> 16;
            srccol = 0;
            while (count--)
            {
                *dest = source[srccol >> 16];
                dest += SCREENWIDTH;
                srccol += dyi;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawBigPatch(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

boolean V_EmptyPatch(patch_t *patch)
{
    int         col;
    column_t    *column;
    int         w = SHORT(patch->width);

    for (col = 0; col < w; col++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

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
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = tempscreen + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;
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

void V_DrawPatchWithShadow(int x, int y, int scrn, patch_t *patch, boolean flag)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;
            while (count--)
            {
                byte    *shadow;

                *dest = source[srccol >> 16];
                dest += SCREENWIDTH;
                shadow = dest + SCREENWIDTH + 2;
                if (!flag || (*shadow != 47 && *shadow != 191))
                    *shadow = tinttab50[*shadow];
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawHUDPatch(int x, int y, int scrn, patch_t *patch, boolean invert)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    if (!invert)
        return;

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawHUDNumberPatch(int x, int y, int scrn, patch_t *patch, boolean invert)
{

    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    if (!invert)
        return;

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                byte    dot = *source++;

                *dest = (dot == 109 ? tinttab50[*dest] : dot);
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawYellowHUDPatch(int x, int y, int scrn, patch_t *patch, boolean invert)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    if (!invert)
        return;

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = redtoyellow[*source++];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDPatch(int x, int y, int scrn, patch_t *patch, boolean invert)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = tinttab75[(*source++ << (8 * invert)) + (*dest << (8 * !invert))];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDNumberPatch(int x, int y, int scrn, patch_t *patch, boolean invert)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

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

void V_DrawTranslucentYellowHUDPatch(int x, int y, int scrn, patch_t *patch, boolean invert)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = tinttab75[(redtoyellow[*source++] << (8 * invert)) + (*dest << (8 * !invert))];
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentRedPatch(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;
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
// V_DrawPatchFlipped
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
void V_DrawPatchFlipped(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch
            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;

            while (count--)
            {
                *dest = source[srccol >> 16];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column+ column->length + 4);
        }
    }
}

void V_DrawTranslucentRedPatchFlipped(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch
            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;

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

void V_DrawFuzzPatch(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            while (count--)
            {
                if (!menuactive && !paused)
                    fuzztable[_fuzzpos] = _FUZZ(-1, 1);
                *dest = colormaps[1536 + dest[fuzztable[_fuzzpos++]]];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFuzzPatchFlipped(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch
            + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> 16)]));

        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            while (count--)
            {
                if (!menuactive && !paused)
                    fuzztable[_fuzzpos] = _FUZZ(-1, 1);
                *dest = colormaps[1536 + dest[fuzztable[_fuzzpos++]]];
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

void V_DrawPatchNoGreenWithShadow(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;
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

void V_DrawPatchCentered(int y, int scrn, patch_t *patch)
{
    V_DrawPatch((ORIGINALWIDTH - SHORT(patch->width)) / 2, y, scrn, patch);
}

extern boolean translucency;

void V_DrawTranslucentNoGreenPatch(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;
    int         stretchx;
    int         stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = SHORT(patch->width) << 16; col < w; col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;

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
void V_DrawBlock(int x, int y, int scrn, int width, int height, byte *src)
{
    byte        *dest;

    dest = screens[scrn] + y * SCREENWIDTH + x;

    while (height--)
    {
        memcpy(dest, src, width);
        src += width;
        dest += SCREENWIDTH;
    }
}

void V_DrawPixel(int x, int y, int screen, byte color, boolean shadow)
{
    byte        *dest = &screens[screen][y * SCREENSCALE * SCREENWIDTH + x * SCREENSCALE];

    if (color == 251)
    {
        if (shadow)
        {
            int xx, yy;

            for (yy = 0; yy < SCREENSCALE; ++yy)
                for (xx = 0; xx < SCREENSCALE; ++xx)
                    *(dest + yy * SCREENWIDTH + xx) = tinttab50[*(dest + yy * SCREENWIDTH + xx)];
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

void V_LowGraphicDetail(int screen, int height)
{
    int x;
    int y;

    for (y = 0; y < height; y += pixelheight)
        for (x = 0; x < SCREENWIDTH; x += pixelwidth)
        {
            byte        *dot = screens[screen] + y * SCREENWIDTH + x;
            int         xx, yy;

            for (yy = 0; yy < pixelheight; yy++)
                for (xx = 0; xx < pixelwidth; xx++)
                    *(dot + yy * SCREENWIDTH + xx) = *dot;
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

#ifndef MAX_PATH
#define MAX_PATH        4096
#endif

char                    lbmname[MAX_PATH];
char                    lbmpath[MAX_PATH];

extern boolean          widescreen;
extern boolean          inhelpscreens;
extern char             maptitle[128];
extern SDL_Surface      *screen;
extern SDL_Surface      *screenbuffer;
extern SDL_Color        palette[256];
extern boolean          splashscreen;
extern int              titlesequence;

boolean V_ScreenShot(void)
{
    boolean     result;
    char        mapname[128];
    char        folder[MAX_PATH];
    int         count = 0;
    int         width, height;
    SDL_Surface *screenshot;

#ifdef WIN32
    HRESULT     hr = SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, folder);

    if (hr != S_OK)
        return false;
#else
    M_StringCopy(folder, "", MAX_PATH);
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

    if (sscanf(mapname, "The %[^\n]", mapname))
        M_snprintf(mapname, sizeof(mapname), "%s, The", mapname);
    else if (sscanf(mapname, "A %[^\n]", mapname))
        M_snprintf(mapname, sizeof(mapname), "%s, A", mapname);

    do
    {
        if (!count)
            M_snprintf(lbmname, sizeof(lbmname), "%s.bmp", mapname);
        else
            M_snprintf(lbmname, sizeof(lbmname), "%s (%i).bmp", mapname, count);
        count++;
        M_snprintf(lbmpath, sizeof(lbmpath), "%s\\" PACKAGE_NAME, folder);
        M_MakeDirectory(lbmpath);
        M_snprintf(lbmpath, sizeof(lbmpath), "%s\\%s", lbmpath, lbmname);
    } while (M_FileExists(lbmpath));

    if (widescreen)
    {
        width = screen->w;
        height = screen->h;
    }
    else
    {
        width = screenbuffer->w;
        height = screenbuffer->h;
    }

    screenshot = SDL_CreateRGBSurface(screenbuffer->flags, width, height,
                                      screenbuffer->format->BitsPerPixel,
                                      screenbuffer->format->Rmask,
                                      screenbuffer->format->Gmask,
                                      screenbuffer->format->Bmask,
                                      screenbuffer->format->Amask);

#ifdef SDL20
    SDL_SetPaletteColors(screenshot->format->palette, palette, 0, 256);
#else
    SDL_SetColors(screenshot, palette, 0, 256);
#endif

    SDL_BlitSurface(screenbuffer, NULL, screenshot, NULL);

    result = !SDL_SaveBMP(screenshot, lbmpath);

    SDL_FreeSurface(screenshot);

    return result;
}
