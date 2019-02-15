/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

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

#include <string.h>

#include "SDL_image.h"

#include "c_console.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "r_draw.h"
#include "r_main.h"
#include "version.h"
#include "w_wad.h"

#define WHITE   4

// Each screen is [SCREENWIDTH * SCREENHEIGHT];
byte            *screens[5];

static fixed_t  DX, DY;
static fixed_t  DXI, DYI;

static int      pixelwidth;
static int      pixelheight;
char            *r_lowpixelsize = r_lowpixelsize_default;

static char     screenshotfolder[MAX_PATH];

extern dboolean vanilla;

//
// V_FillRect
//
void V_FillRect(int scrn, int x, int y, int width, int height, int color, dboolean right)
{
    byte    *dest = screens[scrn] + y * SCREENWIDTH + x;

    while (height--)
    {
        memset(dest, color, width);
        dest += SCREENWIDTH;
    }
}

void V_FillTransRect(int scrn, int x, int y, int width, int height, int color, dboolean right)
{
    byte        *dest = screens[scrn] + y * SCREENWIDTH + x;
    byte        *dot;
    const byte  *tint60 = tinttab60 + (color <<= 8);

    for (int xx = 0; xx < width; xx++)
    {
        dot = dest + xx;

        for (int yy = 0; yy < height; yy++, dot += SCREENWIDTH)
            *dot = *(tint60 + *dot);
    }

    if (height > 2)
    {
        const byte  *tint20 = tinttab20 + color;
        const byte  *tint40 = tinttab40 + color;

        dot = dest - 1 - SCREENWIDTH * 2;
        *dot = *(tint20 + *dot);
        dot += SCREENWIDTH;

        for (int yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tint40 + *dot);

        *dot = *(tint20 + *dot);
        dot = dest - 2 - SCREENWIDTH;

        for (int yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tint20 + *dot);

        for (int xx = 0; xx < width; xx++)
        {
            dot = dest + xx - SCREENWIDTH * 2;
            *dot = *(tint20 + *dot);
            dot += SCREENWIDTH;
            *dot = *(tint40 + *dot);
            dot += SCREENWIDTH * (height + 1);
            *dot = *(tint40 + *dot);
            dot += SCREENWIDTH;
            *dot = *(tint20 + *dot);
        }

        if (right)
        {
            dot = dest + width - SCREENWIDTH * 2;
            *dot = *(tint20 + *dot);
            dot += SCREENWIDTH;

            for (int yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
                *dot = *(tint40 + *dot);

            *dot = *(tint20 + *dot);
            dot = dest + width + 1 - SCREENWIDTH;

            for (int yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
                *dot = *(tint20 + *dot);
        }
    }
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen.
//
void V_DrawPatch(int x, int y, int scrn, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[scrn] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

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

void V_DrawSTBARPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width);
    int     col = 0;

    if (w > ORIGINALWIDTH)
    {
        col = (w - ORIGINALWIDTH) / 2;
        w = ORIGINALWIDTH + col;
    }

    w <<= FRACBITS;
    col <<= FRACBITS;
    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

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
    short   width = SHORT(patch->width);
    short   height = SHORT(patch->height);

    patch->leftoffset = 0;
    patch->topoffset = 0;

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

void V_DrawShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;
    const int   black = nearestblack << 8;
    const byte  *body = tinttab40 + black;
    const byte  *edge = tinttab25 + black;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if (count == 1)
                *dest = edge[*dest];
            else if (count == 2)
            {
                *dest = edge[*dest];
                dest += SCREENWIDTH;
                *dest = edge[*dest];
            }
            else
            {
                count--;
                *dest = edge[*dest];
                dest += SCREENWIDTH;

                while (--count)
                {
                    *dest = body[*dest];
                    dest += SCREENWIDTH;
                }

                *dest = edge[*dest];
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawSolidShadowPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

            while (--count)
            {
                *dest = nearestblack;
                dest += SCREENWIDTH;
            }

            *dest = nearestblack;
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawSpectreShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;
    const byte  *shadow = tinttab40 + (nearestblack << 8);
    int         fuzzpos = 0;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if ((consoleactive && !fuzztable[fuzzpos++]) || (!consoleactive && !(M_Random() & 3)))
                *dest = shadow[*dest];

            dest += SCREENWIDTH;

            while (--count)
            {
                *dest = shadow[*dest];
                dest += SCREENWIDTH;
            }

            if ((consoleactive && !fuzztable[fuzzpos++]) || (!consoleactive && !(M_Random() & 3)))
                *dest = shadow[*dest];

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawSolidSpectreShadowPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;
    int     _fuzzpos = 0;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if ((consoleactive && !fuzztable[_fuzzpos++]) || (!consoleactive && !(M_Random() & 3)))
                *dest = nearestblack;

            dest += SCREENWIDTH;

            while (--count)
            {
                *dest = nearestblack;
                dest += SCREENWIDTH;
            }

            if ((consoleactive && !fuzztable[_fuzzpos++]) || (!consoleactive && !(M_Random() & 3)))
                *dest = nearestblack;

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawBigPatch(int x, int y, int scrn, patch_t *patch)
{
    byte    *desttop = screens[scrn] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        int         td;
        int         topdelta = -1;
        int         lastlength = 0;

        // step through the posts in a column
        while ((td = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest;
            int     count;

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

void V_DrawConsoleTextPatch(int x, int y, patch_t *patch, int color, int backgroundcolor, dboolean italics, byte *translucency)
{
    byte        *desttop = screens[0] + y * SCREENWIDTH + x;
    int         w = SHORT(patch->width);
    const int   italicize[15] = { 0, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1 };

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        byte        topdelta;

        // step through the posts in a column
        while ((topdelta = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + topdelta * SCREENWIDTH;
            byte    length = column->length;
            int     count = length;

            while (count--)
            {
                int height = topdelta + length - count;

                if (y + height > CONSOLETOP)
                {
                    if (backgroundcolor == NOBACKGROUNDCOLOR)
                    {
                        if (*source)
                        {
                            if (italics)
                                *(dest + italicize[height]) = (!translucency ? color : translucency[(color << 8)
                                    + *(dest + italicize[height])]);
                            else
                                *dest = (!translucency ? color : translucency[(color << 8) + *dest]);
                        }
                    }
                    else if (*source == WHITE)
                        *dest = color;
                    else if (*dest != color)
                        *dest = backgroundcolor;
                }

                source++;
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawBigTranslucentPatch(int x, int y, patch_t *patch)
{
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        byte        topdelta;

        // step through the posts in a column
        while ((topdelta = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + topdelta * SCREENWIDTH;
            byte    length = column->length;
            int     count = length;

            while (count--)
            {
                int height = topdelta + length - count;

                if (y + height > CONSOLETOP && *source)
                    *dest = tinttab50[(nearestcolors[*source] << 8) + *dest];

                source++;
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

dboolean V_IsEmptyPatch(patch_t *patch)
{
    int w = SHORT(patch->width);

    for (int col = 0; col < w; col++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
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
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = tempscreen + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

            while (count--)
            {
                *dest = source[srccol >> FRACBITS];
                dest += SCREENWIDTH;

                if (!vanilla)
                    *(dest + SCREENWIDTH + 2) = nearestblack;

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawBigPatchToTempScreen(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width);

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = tempscreen + y * SCREENWIDTH + x;

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        int         td;
        int         topdelta = -1;
        int         lastlength = 0;

        // step through the posts in a column
        while ((td = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest;
            int     count;

            topdelta = (td < topdelta + lastlength - 1 ? topdelta + td : td);
            dest = desttop + topdelta * SCREENWIDTH;
            count = lastlength = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
                *(dest + 1) = nearestblack;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawAltHUDText(int x, int y, patch_t *patch, int color)
{
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        int         topdelta;

        // step through the posts in a column
        while ((topdelta = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + topdelta * SCREENWIDTH;
            int     count = column->length;

            while (count--)
            {
                if (*source++ == WHITE)
                    *dest = color;

                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentAltHUDText(int x, int y, patch_t *patch, int color)
{
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);
    byte    *tinttab = (automapactive ? tinttab25 : tinttab60);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
        int         topdelta;

        // step through the posts in a column
        while ((topdelta = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + topdelta * SCREENWIDTH;
            int     count = column->length;

            while (count--)
            {
                if (*source++ == WHITE)
                    *dest = tinttab[(*dest << 8) + color];

                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPatchWithShadow(int x, int y, patch_t *patch, dboolean flag)
{
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;
    const byte  *shadow = tinttab50 + (nearestblack << 8);

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

            while (count--)
            {
                int height = (((y + column->topdelta + column->length) * DY) >> FRACBITS) - count;

                if (height > 0)
                    *dest = source[srccol >> FRACBITS];

                dest += SCREENWIDTH;

                if (height + 2 > 0)
                {
                    byte    *dot = dest + SCREENWIDTH + 2;

                    if (!flag || (*dot != 47 && *dot != 191))
                        *dot = shadow[*dot];
                }

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawHUDPatch(int x, int y, patch_t *patch, byte *translucency)
{
    byte    *desttop;
    int     w;

    if (!translucency)
        return;

    desttop = screens[0] + y * SCREENWIDTH + x;
    w = SHORT(patch->width);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + column->topdelta * SCREENWIDTH;
            int     count = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawHighlightedHUDNumberPatch(int x, int y, patch_t *patch, byte *translucency)
{
    byte    *desttop;
    int     w;

    if (!translucency)
        return;

    desttop = screens[0] + y * SCREENWIDTH + x;
    w = SHORT(patch->width);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + column->topdelta * SCREENWIDTH;
            int     count = column->length;

            while (count--)
            {
                byte    dot = *source++;

                *dest = (dot == 109 ? tinttab33[*dest] : dot);
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDPatch(int x, int y, patch_t *patch, byte *translucency)
{
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + column->topdelta * SCREENWIDTH;
            int     count = column->length;

            while (count--)
            {
                *dest = translucency[(*source++ << 8) + *dest];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDNumberPatch(int x, int y, patch_t *patch, byte *translucency)
{
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + column->topdelta * SCREENWIDTH;
            int     count = column->length;

            while (count--)
            {
                byte    dot = *source++;

                *dest = (dot == 109 ? tinttab33[*dest] : translucency[(dot << 8) + *dest]);
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawAltHUDPatch(int x, int y, patch_t *patch, int from, int to)
{
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + column->topdelta * SCREENWIDTH;
            byte    length = column->length;
            byte    count = length;

            while (count--)
            {
                byte    dot = *source++;

                if (dot)
                    *dest = (dot == from ? to : nearestcolors[dot]);

                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentAltHUDPatch(int x, int y, patch_t *patch, int from, int to)
{
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    to <<= 8;

    for (int col = 0; col < w; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + column->topdelta * SCREENWIDTH;
            byte    length = column->length;
            byte    count = length;

            while (count--)
            {
                byte    dot = *source++;

                if (dot)
                    *dest = tinttab60[(dot == from ? to : (nearestcolors[dot] << 8)) + *dest];

                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentRedPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

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
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch
                        + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

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
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;
    const int   black = nearestblack << 8;
    const byte  *body = tinttab40 + black;
    const byte  *edge = tinttab25 + black;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if (count == 1)
                *dest = edge[*dest];
            else if (count == 2)
            {
                *dest = edge[*dest];
                dest += SCREENWIDTH;
                *dest = edge[*dest];
            }
            else
            {
                count--;
                *dest = edge[*dest];
                dest += SCREENWIDTH;

                while (--count)
                {
                    *dest = body[*dest];
                    dest += SCREENWIDTH;
                }

                *dest = edge[*dest];
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedSolidShadowPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

            while (--count)
            {
                *dest = nearestblack;
                dest += SCREENWIDTH;
            }

            *dest = nearestblack;
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedSpectreShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;
    const byte  *shadow = tinttab40 + (nearestblack << 8);
    int         _fuzzpos = 0;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if ((consoleactive && !fuzztable[_fuzzpos++]) || (!consoleactive && !(M_Random() & 3)))
                *dest = shadow[*dest];

            dest += SCREENWIDTH;

            while (--count)
            {
                *dest = shadow[*dest];
                dest += SCREENWIDTH;
            }

            if ((consoleactive && !fuzztable[_fuzzpos++]) || (!consoleactive && !(M_Random() & 3)))
                *dest = shadow[*dest];

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedSolidSpectreShadowPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;
    int     _fuzzpos = 0;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

            if ((consoleactive && !fuzztable[_fuzzpos++]) || (!consoleactive && !(M_Random() & 3)))
                *dest = nearestblack;

            dest += SCREENWIDTH;

            while (--count)
            {
                *dest = nearestblack;
                dest += SCREENWIDTH;
            }

            if ((consoleactive && !fuzztable[_fuzzpos++]) || (!consoleactive && !(M_Random() & 3)))
                *dest = nearestblack;

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedTranslucentRedPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

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

void V_DrawFuzzPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;
    int     fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;

            while (count--)
            {
                if (!menuactive && !paused && !consoleactive)
                    fuzztable[fuzzpos] = FUZZ(-1, 1);

                *dest = fullcolormap[6 * 256 + dest[fuzztable[fuzzpos++]]];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawFlippedFuzzPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;
    int     fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;

            while (count--)
            {
                if (!menuactive && !paused && !consoleactive)
                    fuzztable[fuzzpos] = FUZZ(-1, 1);

                *dest = fullcolormap[6 * 256 + dest[fuzztable[fuzzpos++]]];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

static const byte nogreen[256] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

void V_DrawNoGreenPatchWithShadow(int x, int y, patch_t *patch)
{
    byte        *desttop;
    int         w = SHORT(patch->width) << FRACBITS;
    const byte  *shadow = tinttab50 + (nearestblack << 8);

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

            while (count--)
            {
                byte    src = source[srccol >> FRACBITS];

                if (nogreen[src])
                {
                    byte    *dot;

                    *dest = src;
                    dot = dest + SCREENWIDTH * 2 + 2;

                    if (*dot != 47 && *dot != 191)
                        *dot = shadow[*dot];
                }

                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentNoGreenPatch(int x, int y, patch_t *patch)
{
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (int col = 0; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;
            int     srccol = 0;

            while (count--)
            {
                byte    src = source[srccol >> FRACBITS];

                if (nogreen[src])
                    *dest = tinttab33[(*dest << 8) + src];

                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPixel(int x, int y, byte color, dboolean drawshadow)
{
    byte        *dest = &screens[0][(y * SCREENWIDTH + x) * SCREENSCALE];
    const byte  *shadow = tinttab50 + (nearestblack << 8);

    if (color == 251)
    {
        if (drawshadow)
            for (int yy = 0; yy < SCREENSCALE * SCREENWIDTH; yy += SCREENWIDTH)
                for (int xx = 0; xx < SCREENSCALE; xx++)
                {
                    byte    *dot = dest + yy + xx;

                    *dot = shadow[*dot];
                }
    }
    else if (color && color != 32)
    {
        for (int yy = 0; yy < SCREENSCALE * SCREENWIDTH; yy += SCREENWIDTH)
            for (int xx = 0; xx < SCREENSCALE; xx++)
                *(dest + yy + xx) = color;
    }
}

void GetPixelSize(dboolean reset)
{
    int width = -1;
    int height = -1;

    if (sscanf(r_lowpixelsize, "%10dx%10d", &width, &height) == 2
        && width > 0 && width <= SCREENWIDTH && height > 0 && height <= SCREENHEIGHT && (width >= 2 || height >= 2))
    {
        pixelwidth = width;
        pixelheight = height * SCREENWIDTH;
    }
    else if (reset)
    {
        pixelwidth = 2;
        pixelheight = 2 * SCREENWIDTH;
        r_lowpixelsize = r_lowpixelsize_default;

        M_SaveCVARs();
    }
}

void V_LowGraphicDetail(void)
{
    int w = viewwindowx + viewwidth;
    int h = (viewwindowy + viewheight) * SCREENWIDTH;

    for (int y = viewwindowy * SCREENWIDTH; y < h; y += pixelheight)
        for (int x = viewwindowx; x < w; x += pixelwidth)
        {
            byte    *dot = *screens + y + x;

            for (int yy = 0; yy < pixelheight && y + yy < h; yy += SCREENWIDTH)
                for (int xx = 0; xx < pixelwidth && x + xx < w; xx++)
                    *(dot + yy + xx) = *dot;
        }
}

void V_InvertScreen(void)
{
    int w = viewwindowx + viewwidth;
    int h = viewwindowy + viewheight;

    for (int y = viewwindowy; y < h; y++)
        for (int x = viewwindowx; x < w; x++)
        {
            byte    *dot = *screens + y * SCREENWIDTH + x;

            *dot = colormaps[0][32 * 256 + *dot];
        }
}

//
// V_Init
//
void V_Init(void)
{
    byte                *base = malloc(SCREENWIDTH * SCREENHEIGHT * 4);
    const SDL_version   *linked = IMG_Linked_Version();
    int                 p;

    if (linked->major != SDL_IMAGE_MAJOR_VERSION || linked->minor != SDL_IMAGE_MINOR_VERSION)
        I_Error("The wrong version of %s was found. %s requires v%i.%i.%i.",
            SDL_IMAGE_FILENAME, PACKAGE_NAME, SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);

    if (linked->patch != SDL_IMAGE_PATCHLEVEL)
        C_Warning("The wrong version of <b>%s</b> was found. <i>%s</i> requires v%i.%i.%i.",
            SDL_IMAGE_FILENAME, PACKAGE_NAME, SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);

    for (int i = 0; i < 4; i++)
        screens[i] = base + i * SCREENWIDTH * SCREENHEIGHT;

    DX = (SCREENWIDTH << FRACBITS) / ORIGINALWIDTH;
    DXI = (ORIGINALWIDTH << FRACBITS) / SCREENWIDTH;
    DY = (SCREENHEIGHT << FRACBITS) / ORIGINALHEIGHT;
    DYI = (ORIGINALHEIGHT << FRACBITS) / SCREENHEIGHT;

    GetPixelSize(true);

    if ((p = M_CheckParmWithArgs("-shotdir", 1, 1)))
        M_StringCopy(screenshotfolder, myargv[p + 1], sizeof(screenshotfolder));
    else
    {
        char    *appdatafolder = M_GetAppDataFolder();

        M_snprintf(screenshotfolder, sizeof(screenshotfolder), "%s"DIR_SEPARATOR_S"screenshots", appdatafolder);
        free(appdatafolder);
    }
}

char            lbmname1[MAX_PATH];
char            lbmpath1[MAX_PATH];
char            lbmpath2[MAX_PATH];

extern char     maptitle[128];
extern dboolean splashscreen;
extern int      titlesequence;

static dboolean V_SavePNG(SDL_Renderer *renderer, char *path)
{
    dboolean    result = false;

    if (renderer)
    {
        int rendererwidth;
        int rendererheight;

        if (!SDL_GetRendererOutputSize(renderer, &rendererwidth, &rendererheight))
        {
            int         width = (vid_widescreen ? rendererheight * 16 / 10 : rendererheight * 4 / 3);
            int         height = rendererheight;
            SDL_Surface *screenshot;

            if (width > rendererwidth)
            {
                width = rendererwidth;
                height = (vid_widescreen ? rendererwidth * 10 / 16 : rendererwidth * 3 / 4);
            }

            if ((screenshot = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0)))
            {
                if (!SDL_RenderReadPixels(renderer, NULL, 0, screenshot->pixels, screenshot->pitch))
                    result = !IMG_SavePNG(screenshot, path);

                SDL_FreeSurface(screenshot);
            }
        }
    }

    return result;
}

dboolean V_ScreenShot(void)
{
    dboolean    result = false;
    char        mapname[128];
    int         count = 0;

    if (consoleactive)
        M_StringCopy(mapname, "Console", sizeof(mapname));
    else if (menuactive)
        M_StringCopy(mapname, "Menu", sizeof(mapname));
    else if (automapactive)
        M_StringCopy(mapname, "Automap", sizeof(mapname));
    else
        switch (gamestate)
        {
            case GS_INTERMISSION:
                M_StringCopy(mapname, "Intermission", sizeof(mapname));
                break;

            case GS_FINALE:
                M_StringCopy(mapname, "Finale", sizeof(mapname));
                break;

            case GS_TITLESCREEN:
                M_StringCopy(mapname, (splashscreen ? "Splash" : (titlesequence == 1 ? "Credits" : "Title")), sizeof(mapname));
                break;

            default:
                M_StringCopy(mapname, (inhelpscreens ? "Help" : titlecase(maptitle)), sizeof(mapname));
                break;
        }

    if (M_StringStartsWith(mapname, "The "))
        M_snprintf(mapname, sizeof(mapname), "%s, The", M_SubString(mapname, 4, strlen(mapname) - 4));
    else if (M_StringStartsWith(mapname, "A "))
        M_snprintf(mapname, sizeof(mapname), "%s, A", M_SubString(mapname, 2, strlen(mapname) - 2));

    do
    {
        if (!count)
            M_snprintf(lbmname1, sizeof(lbmname1), "%s.png", makevalidfilename(mapname));
        else
            M_snprintf(lbmname1, sizeof(lbmname1), "%s (%s).png", makevalidfilename(mapname), commify(count));

        count++;
        M_MakeDirectory(screenshotfolder);
        M_snprintf(lbmpath1, sizeof(lbmpath1), "%s"DIR_SEPARATOR_S"%s", screenshotfolder, lbmname1);
    } while (M_FileExists(lbmpath1));

    result = V_SavePNG(renderer, lbmpath1);
    lbmpath2[0] = '\0';

    if (mapwindow && result && gamestate == GS_LEVEL)
    {
        do
        {
            M_snprintf(lbmpath2, sizeof(lbmpath2), "%s"DIR_SEPARATOR_S"%s (%s).png", screenshotfolder, makevalidfilename(mapname),
                commify(count++));
        } while (M_FileExists(lbmpath2));

        V_SavePNG(maprenderer, lbmpath2);
    }

    return result;
}
