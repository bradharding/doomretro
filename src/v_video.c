/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if defined(_WIN32)
#pragma warning( disable : 4091 )
#include <Shlobj.h>
#endif

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "r_main.h"
#include "SDL_image.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define BLACK   0
#define WHITE   4

// Each screen is [SCREENWIDTH * SCREENHEIGHT];
byte            *screens[5];

fixed_t         DX, DY;
fixed_t         DXI, DYI;

int             pixelwidth;
int             pixelheight;
char            *r_lowpixelsize = r_lowpixelsize_default;

char            screenshotfolder[MAX_PATH];

extern dboolean r_hud_translucency;
extern dboolean r_translucency;
extern dboolean vanilla;

//
// V_CopyRect
//
void V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn)
{
    byte    *src = screens[srcscrn] + srcy * SCREENWIDTH + srcx;
    byte    *dest = screens[destscrn] + desty * SCREENWIDTH + destx;

    while (height--)
    {
        memcpy(dest, src, width);
        src += SCREENWIDTH;
        dest += SCREENWIDTH;
    }
}

//
// V_FillRect
//
void V_FillRect(int scrn, int x, int y, int width, int height, int color)
{
    byte    *dest = screens[scrn] + y * SCREENWIDTH + x;

    while (height--)
    {
        memset(dest, color, width);
        dest += SCREENWIDTH;
    }
}

void V_FillTransRect(int scrn, int x, int y, int width, int height, int color)
{
    byte        *dest = screens[scrn] + y * SCREENWIDTH + x;
    byte        *dot;
    int         xx, yy;
    const byte  *tint60 = tinttab60 + (color <<= 8);

    for (xx = 0; xx < width; xx++)
    {
        dot = dest + xx;

        for (yy = 0; yy < height; yy++, dot += SCREENWIDTH)
            *dot = *(tint60 + *dot);
    }

    if (height > 2)
    {
        const byte  *tint20 = tinttab20 + color;
        const byte  *tint40 = tinttab40 + color;

        dot = dest - 1 - SCREENWIDTH * 2;
        *dot = *(tint20 + *dot);
        dot += SCREENWIDTH;

        for (yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tint40 + *dot);

        *dot = *(tint20 + *dot);

        dot = dest - 2 - SCREENWIDTH;

        for (yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tint20 + *dot);

        for (xx = 0; xx < width; xx++)
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

        dot = dest + width - SCREENWIDTH * 2;
        *dot = *(tint20 + *dot);
        dot += SCREENWIDTH;

        for (yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tint40 + *dot);

        *dot = *(tint20 + *dot);

        dot = dest + width + 1 - SCREENWIDTH;

        for (yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tint20 + *dot);
    }
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen.
//
void V_DrawPatch(int x, int y, int scrn, patch_t *patch)
{
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[scrn] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[scrn] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

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
    int     col = 0;
    byte    *desttop = screens[scrn] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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

int italicize[15] = { 0, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1 };

void V_DrawConsoleTextPatch(int x, int y, patch_t *patch, int color, int backgroundcolor, dboolean italics,
    byte *tinttab)
{
    int     col = 0;
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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
                int     height = topdelta + length - count;

                if (y + height > CONSOLETOP)
                {
                    if (backgroundcolor == NOBACKGROUNDCOLOR)
                    {
                        if (*source)
                        {
                            if (italics)
                                *(dest + italicize[height]) = (!tinttab ? color :
                                    tinttab[(color << 8) + *(dest + italicize[height])]);
                            else
                                *dest = (!tinttab ? color : tinttab[(color << 8) + *dest]);
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

void V_DrawConsolePatch(int x, int y, patch_t *patch)
{
    int     col = 0;
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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

dboolean V_EmptyPatch(patch_t *patch)
{
    int col = 0;
    int w = SHORT(patch->width);

    for (; col < w; col++)
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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = tempscreen + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
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
                    *(dest + SCREENWIDTH + 2) = BLACK;

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawBigPatchToTempScreen(int x, int y, patch_t *patch)
{
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width);

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = tempscreen + y * SCREENWIDTH + x;

    for (; col < w; col++, desttop++)
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
                *(dest + 1) = BLACK;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawAltHUDText(int x, int y, patch_t *patch, int color)
{
    int     col = 0;
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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
                    *dest = (r_hud_translucency ? tinttab60[(*dest << 8) + color] : color);

                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPatchWithShadow(int x, int y, patch_t *patch, dboolean flag)
{
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

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
                int height = (((y + column->topdelta + column->length) * DY) >> FRACBITS) - count;

                if (height > 0)
                    *dest = source[srccol >> FRACBITS];

                dest += SCREENWIDTH;

                if (height + 2 > 0)
                {
                    byte    *shadow = dest + SCREENWIDTH + 2;

                    if (!flag || (*shadow != 47 && *shadow != 191))
                        *shadow = tinttab50[*shadow];
                }

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawHUDPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int     col = 0;
    byte    *desttop;
    int     w;

    if (!tinttab)
        return;

    desttop = screens[0] + y * SCREENWIDTH + x;
    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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

void V_DrawHighlightedHUDNumberPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int     col = 0;
    byte    *desttop;
    int     w;

    if (!tinttab)
        return;

    desttop = screens[0] + y * SCREENWIDTH + x;
    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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

                *dest = (dot == 109 && tinttab ? tinttab33[*dest] : dot);
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawYellowHUDPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int     col = 0;
    byte    *desttop;
    int     w;

    if (!tinttab)
        return;

    desttop = screens[0] + y * SCREENWIDTH + x;
    w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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
                *dest = redtoyellow[*source++];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int     col = 0;
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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
                *dest = tinttab[(*source++ << 8) + *dest];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentHUDNumberPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int     col = 0;
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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

                *dest = (dot == 109 ? tinttab33[*dest] : tinttab[(dot << 8) + *dest]);
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawTranslucentYellowHUDPatch(int x, int y, patch_t *patch, byte *tinttab)
{
    int     col = 0;
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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
                *dest = tinttab75[(redtoyellow[*source++] << 8) + *dest];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawAltHUDPatch(int x, int y, patch_t *patch, int from, int to)
{
    int     col = 0;
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    for (; col < w; col++, desttop++)
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
    int     col = 0;
    byte    *desttop = screens[0] + y * SCREENWIDTH + x;
    int     w = SHORT(patch->width);

    to <<= 8;

    for (; col < w; col++, desttop++)
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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch
                        + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch
                        + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + (((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch
                        + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH;
            int     count = ((column->length * DY / 10) >> FRACBITS) + 1;

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
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
                *dest = tinttabred[(*dest << 8) + source[srccol >> FRACBITS]];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

#define _FUZZ(a, b) _fuzzrange[M_RandomInt(a + 1, b + 1)]

const int   _fuzzrange[3] = { -SCREENWIDTH, 0, SCREENWIDTH };

extern int  fuzztable[SCREENWIDTH * SCREENHEIGHT];

void V_DrawFuzzPatch(int x, int y, patch_t *patch)
{
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;
    int     _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;
    int     _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = screens[0] + ((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS);

    for (; col < w; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch
                        + LONG(patch->columnofs[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        while (column->topdelta != 0xFF)
        {
            byte    *dest = desttop + ((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH;
            int     count = (column->length * DY) >> FRACBITS;

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

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
                byte    src = source[srccol >> FRACBITS];

                if (nogreen[src])
                {
                    byte    *shadow;

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
    int     col = 0;
    byte    *desttop;
    int     w = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

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
                byte src = source[srccol >> FRACBITS];

                if (nogreen[src])
                    *dest = tinttab33[(*dest << 8) + src];

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
    byte    *dest = screens[0] + y * SCREENWIDTH + x;

    while (height--)
    {
        memcpy(dest, src, width);
        src += width;
        dest += SCREENWIDTH;
    }
}

void V_DrawPixel(int x, int y, byte color, dboolean shadow)
{
    byte    *dest = &screens[0][y * SCREENSCALE * SCREENWIDTH + x * SCREENSCALE];

    if (color == 251)
    {
        if (shadow)
        {
            int xx, yy;

            for (yy = 0; yy < SCREENSCALE; yy++)
                for (xx = 0; xx < SCREENSCALE; xx++)
                    *(dest + yy * SCREENWIDTH + xx) = tinttab50[*(dest + yy * SCREENWIDTH + xx)];
        }
    }
    else if (color && color != 32)
    {
        int xx, yy;

        for (yy = 0; yy < SCREENSCALE; yy++)
            for (xx = 0; xx < SCREENSCALE; xx++)
                *(dest + yy * SCREENWIDTH + xx) = color;
    }
}

void GetPixelSize(dboolean reset)
{
    int     width = -1;
    int     height = -1;
    char    *p_lowpixelsize = strdup(r_lowpixelsize);
    char    *left = strtok(p_lowpixelsize, "x");
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
    else if (reset)
    {
        pixelwidth = 2;
        pixelheight = 2;
        r_lowpixelsize = r_lowpixelsize_default;

        M_SaveCVARs();
    }

    free(p_lowpixelsize);
}

void V_LowGraphicDetail(void)
{
    int x, y;
    int w = viewwindowx + viewwidth;
    int h = (viewwindowy + viewheight) * SCREENWIDTH;
    int hh = pixelheight * SCREENWIDTH;

    for (y = viewwindowy * SCREENWIDTH; y < h; y += hh)
        for (x = viewwindowx; x < w; x += pixelwidth)
        {
            byte    *dot = *screens + y + x;
            int     xx, yy;

            for (yy = 0; yy < hh && y + yy < h; yy += SCREENWIDTH)
                for (xx = 0; xx < pixelwidth && x + xx < w; xx++)
                    *(dot + yy + xx) = *dot;
        }
}

//
// V_Init
//
void V_Init(void)
{
    int                 i;
    byte                *base = Z_Malloc(SCREENWIDTH * SCREENHEIGHT * 4, PU_STATIC, NULL);
    const SDL_version   *linked = IMG_Linked_Version();
#if defined(_WIN32) && !defined(PORTABILITY)
    char                buffer[MAX_PATH];
#endif

    if (linked->major != SDL_IMAGE_MAJOR_VERSION || linked->minor != SDL_IMAGE_MINOR_VERSION)
        I_Error("The wrong version of sdl2_image.dll was found. "PACKAGE_NAME" requires v%i.%i.%i, "
            "not v%i.%i.%i.", linked->major, linked->minor, linked->patch, SDL_IMAGE_MAJOR_VERSION,
            SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);

    if (linked->patch != SDL_IMAGE_PATCHLEVEL)
        C_Warning("The wrong version of sdl2_image.dll was found. <i>"PACKAGE_NAME"</i> requires v%i.%i.%i, "
            "not v%i.%i.%i.", linked->major, linked->minor, linked->patch, SDL_IMAGE_MAJOR_VERSION,
            SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);

    for (i = 0; i < 4; i++)
        screens[i] = base + i * SCREENWIDTH * SCREENHEIGHT;

    DX = (SCREENWIDTH << FRACBITS) / ORIGINALWIDTH;
    DXI = (ORIGINALWIDTH << FRACBITS) / SCREENWIDTH;
    DY = (SCREENHEIGHT << FRACBITS) / ORIGINALHEIGHT;
    DYI = (ORIGINALHEIGHT << FRACBITS) / SCREENHEIGHT;

    GetPixelSize(true);

#if defined(_WIN32) && !defined(PORTABILITY)
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, buffer)))
        M_snprintf(screenshotfolder, sizeof(screenshotfolder), "%s"DIR_SEPARATOR_S PACKAGE_NAME, buffer);
    else
        M_snprintf(screenshotfolder, sizeof(screenshotfolder), "%s"DIR_SEPARATOR_S PACKAGE_NAME,
            M_GetExecutableFolder());
#else
        M_snprintf(screenshotfolder, sizeof(screenshotfolder), "%s"DIR_SEPARATOR_S"screenshots",
            M_GetAppDataFolder());
#endif
}

char            lbmname1[MAX_PATH];
char            lbmname2[MAX_PATH];
char            lbmpath1[MAX_PATH];
char            lbmpath2[MAX_PATH];

extern dboolean vid_widescreen;
extern dboolean inhelpscreens;
extern char     maptitle[128];
extern dboolean splashscreen;
extern int      titlesequence;

dboolean V_SavePNG(SDL_Window *window, char *path)
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
                if (!SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels,
                    screenshot->pitch))
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

    switch (gamestate)
    {
        case GS_INTERMISSION:
            M_StringCopy(mapname, "Intermission", sizeof(mapname));
            break;

        case GS_FINALE:
            M_StringCopy(mapname, "Finale", sizeof(mapname));
            break;

        case GS_TITLESCREEN:
            M_StringCopy(mapname, (splashscreen ? "Splash" : (titlesequence == 1 ? "Credits" : "Title")),
                sizeof(mapname));
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
            M_snprintf(lbmname1, sizeof(lbmname1), "%s (%i).png", makevalidfilename(mapname), count);

        count++;
        M_MakeDirectory(screenshotfolder);
        M_snprintf(lbmpath1, sizeof(lbmpath1), "%s"DIR_SEPARATOR_S"%s", screenshotfolder, lbmname1);
    }
    while (M_FileExists(lbmpath1));

    result = V_SavePNG(window, lbmpath1);

    lbmpath2[0] = '\0';

    if (mapwindow && result && gamestate == GS_LEVEL)
    {
        do
        {
            M_snprintf(lbmname2, sizeof(lbmname2), "%s (%i).png", makevalidfilename(mapname), count++);
            M_snprintf(lbmpath2, sizeof(lbmpath2), "%s"DIR_SEPARATOR_S"%s", screenshotfolder, lbmname2);
        }
        while (M_FileExists(lbmpath2));

        V_SavePNG(mapwindow, lbmpath2);
    }

    return result;
}
