/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "c_cmds.h"
#include "c_console.h"
#include "d_iwad.h"
#include "d_main.h"
#include "doomstat.h"
#include "hu_lib.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_setup.h"
#include "r_draw.h"
#include "SDL_image.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

byte    *screens[NUMSCREENS];
int     lowpixelwidth;
int     lowpixelheight;

void (*postprocessfunc)(int, int, int, int, int, int);

byte    *colortranslation[CR_LIMIT];
byte    *redtogold;

typedef struct
{
    const char  *name;
    byte        **lump;
} colortranslation_t;

static colortranslation_t colortranslations[] =
{
    { "CRRED",    &colortranslation[CR_RED]    },
    { "CRGRAY",   &colortranslation[CR_GRAY]   },
    { "CRGREEN",  &colortranslation[CR_GREEN]  },
    { "CRBLUE",   &colortranslation[CR_BLUE]   },
    { "CRYELLOW", &colortranslation[CR_YELLOW] },
    { "CRBLACK",  &colortranslation[CR_BLACK]  },
    { "CRPURPLE", &colortranslation[CR_PURPLE] },
    { "CRWHITE",  &colortranslation[CR_WHITE]  },
    { "CRORANGE", &colortranslation[CR_ORANGE] },
    { "",         NULL                         }
};

void V_InitColorTranslation(void)
{
    for (colortranslation_t *p = colortranslations; *p->name; p++)
        *p->lump = W_CacheLumpName(p->name);

    redtogold = W_CacheLumpName("CRGOLD");
}

//
// V_FillRect
//
void V_FillRect(int screen, int x, int y, int width, int height, int color1, int color2,
    bool left, bool right, const byte *tinttab1, const byte *tinttab2)
{
    byte    *dest = &screens[screen][y * SCREENWIDTH + x];

    while (height--)
    {
        memset(dest, color1, width);
        dest += SCREENWIDTH;
    }
}

void V_FillTransRect(int screen, int x, int y, int width, int height, int color1, int color2,
    bool left, bool right, const byte *tinttab1, const byte *tinttab2)
{
    byte    *dest = &screens[screen][y * SCREENWIDTH + x];

    tinttab1 += ((size_t)color1 << 8);

    for (int xx = 0; xx < width; xx++)
    {
        byte    *dot = dest + xx;

        for (int yy = 0; yy < height; yy++, dot += SCREENWIDTH)
            *dot = *(tinttab1 + *dot);
    }
}

void V_FillSoftTransRect(int screen, int x, int y, int width, int height, int color1, int color2,
    bool left, bool right, const byte *tinttab1, const byte *tinttab2)
{
    byte    *dest = &screens[screen][y * SCREENWIDTH + x];
    byte    *dot;

    tinttab1 += ((size_t)color1 << 8);
    tinttab2 += ((size_t)color2 << 8);

    for (int xx = 0; xx < width; xx++)
    {
        dot = dest + xx;

        for (int yy = 0; yy < height; yy++, dot += SCREENWIDTH)
            *dot = *(tinttab1 + *dot);
    }

    if (left)
    {
        dot = dest - 1 - 2 * (size_t)SCREENWIDTH;
        *dot = *(tinttab2 + *dot);
        dot += SCREENWIDTH;

        for (int yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tinttab2 + *dot);

        *dot = *(tinttab2 + *dot);
        dot = dest - 2 - SCREENWIDTH;

        for (int yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tinttab2 + *dot);
    }

    for (int xx = 0; xx < width; xx++)
    {
        dot = dest + xx - 2 * (size_t)SCREENWIDTH;
        *dot = *(tinttab2 + *dot);
        dot += SCREENWIDTH;
        *dot = *(tinttab2 + *dot);
        dot += ((size_t)height + 1) * SCREENWIDTH;
        *dot = *(tinttab2 + *dot);
        dot += SCREENWIDTH;
        *dot = *(tinttab2 + *dot);
    }

    if (right)
    {
        dot = dest + width - 2 * (size_t)SCREENWIDTH;
        *dot = *(tinttab2 + *dot);
        dot += SCREENWIDTH;

        for (int yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tinttab2 + *dot);

        *dot = *(tinttab2 + *dot);
        dot = dest + width + 1 - SCREENWIDTH;

        for (int yy = 0; yy < height + 2; yy++, dot += SCREENWIDTH)
            *dot = *(tinttab2 + *dot);
    }
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen.
//
void V_DrawPatch(int x, int y, int screen, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[screen][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                *dest = source[srccol >> FRACBITS];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawWidePatch(int x, int y, int screen, patch_t *patch)
{
    byte        *desttop;
    int         col = 0;
    const int   width = SHORT(patch->width) << FRACBITS;

    if (x < 0)
    {
        col += DXI * ((-x * DX) >> FRACBITS);
        x = 0;
        desttop = &screens[screen][((y * DY) >> FRACBITS) * SCREENWIDTH];
    }
    else
    {
        x = (x * DX) >> FRACBITS;
        desttop = &screens[screen][((y * DY) >> FRACBITS) * SCREENWIDTH + x];
    }

    for (; col < width; x++, col += DXI, desttop++)
    {
        column_t    *column;
        int         topdelta;

        if (x >= SCREENWIDTH)
            break;

        column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        while ((topdelta = column->topdelta) != 0xFF)
        {
            byte        *dest = &desttop[((topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  *source = (byte *)column + 3;
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;
            int         top = ((y + topdelta) * DY) >> FRACBITS;

            if (top + count > SCREENHEIGHT)
                count = SCREENHEIGHT - top;

            while (count-- > 0)
            {
                if (top++ >= 0)
                    *dest = source[srccol >> FRACBITS];

                srccol += DYI;
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawPagePatch(int screen, patch_t *patch)
{
    if (SCREENWIDTH != NONWIDEWIDTH)
    {
        static patch_t  *prevpatch = NULL;
        static int      pillarboxcolor;

        if (prevpatch != patch)
        {
            pillarboxcolor = tinttab25[FindDominantEdgeColor(patch, SHORT(patch->height), 16)];
            prevpatch = patch;
        }

        memset(screens[screen], pillarboxcolor, SCREENAREA);
    }

    V_DrawWidePatch((SCREENWIDTH / SCREENSCALE - SHORT(patch->width)) / 2, 0, screen, patch);
}

void V_DrawShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = ((length * DY / 10) >> FRACBITS) + 1;

            if (count == 1)
                *dest = black25[*dest];
            else if (count == 2)
            {
                *dest = black25[*dest];
                dest += SCREENWIDTH;
                *dest = black25[*dest];
            }
            else
            {
                count--;
                *dest = black25[*dest];
                dest += SCREENWIDTH;

                while (--count)
                {
                    *dest = black40[*dest];
                    dest += SCREENWIDTH;
                }

                *dest = black25[*dest];
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawSolidShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = ((length * DY / 10) >> FRACBITS) + 1;

            while (--count)
            {
                *dest = nearestblack;
                dest += SCREENWIDTH;
            }

            *dest = nearestblack;
            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawSpectreShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = ((length * DY / 10) >> FRACBITS) + 1;

            dest += SCREENWIDTH;

            while (--count)
            {
                *dest = black25[*dest];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawBigPatch(int x, int y, short width, short height, patch_t *patch)
{
    short   col = 0;

    if (width > SCREENWIDTH)
    {
        col = (width - SCREENWIDTH) / 2;
        width = SCREENWIDTH + col;
        x = 0;
    }

    for (byte *desttop = &screens[0][y * SCREENWIDTH + x]; col < width; col++, desttop++)
    {
        byte    *source = (byte *)patch + LONG(patch->columnoffset[col]) + 3;
        byte    *dest = desttop;

        for (short i = 0; i < height; i++)
        {
            *dest = *source++;
            dest += SCREENWIDTH;
        }
    }
}

void V_DrawMenuBorderPatch(int x, int y, patch_t *patch)
{
    byte        *desttopleft = &screens[0][y * SCREENWIDTH + x];
    byte        *desttopright = &screens[0][y * SCREENWIDTH + SCREENWIDTH - x - 1];
    const int   width = SHORT(patch->width);
    const int   black = (nearestblack << 8);

    for (int col = 0; col < width; col++, desttopleft++, desttopright--)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        int         td;
        int         topdelta = -1;
        int         lastlength = 0;

        // step through the posts in a column
        while ((td = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *destleft;
            byte    *destright;
            int     count;

            topdelta = (td < topdelta + lastlength - 1 ? topdelta + td : td);
            destleft = &desttopleft[topdelta * SCREENWIDTH];
            destright = &desttopright[topdelta * SCREENWIDTH];
            count = lastlength = column->length;

            while (count-- > 0)
            {
                if (*source == GRAY2)
                {
                    *destleft = tinttab50[*destleft + black];
                    *destright = tinttab50[*destright];
                }
                else if (*source == DARKGRAY2)
                {
                    *destleft = tinttab20[*destleft + black];
                    *destright = tinttab20[*destright + black];
                }
                else
                {
                    *destleft = nearestblack;
                    *destright = nearestblack;
                }

                source++;
                destleft += SCREENWIDTH;
                destright += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + lastlength + 4);
        }
    }
}

void V_DrawConsoleSelectedTextPatch(const int x, const int y, const patch_t *patch, const int width,
    const int color, const int backgroundcolor, const bool italics, const byte *tinttab)
{
    byte    *desttop = &screens[0][y * SCREENWIDTH + x];

    for (int col = 0; col < width; col++, desttop++)
    {
        byte    *source = (byte *)patch + LONG(patch->columnoffset[col]) + 3;
        byte    *dest = desttop;

        for (int i = 0; i < CONSOLELINEHEIGHT; i++)
        {
            if (y + i >= 0)
            {
                if (*source == WHITE)
                    *dest = color;
                else if (*dest != color)
                    *dest = backgroundcolor;
            }

            source++;
            dest += SCREENWIDTH;
        }
    }
}

void V_DrawConsoleTextPatch(const int x, const int y, const patch_t *patch, const int width,
    const int color, const int backgroundcolor, const bool italics, const byte *tinttab)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   italicize[] = { 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1 };

    for (int col = 0; col < width - 1; col++, desttop++)
    {
        byte    *source = (byte *)patch + LONG(patch->columnoffset[col]) + 3;
        byte    *dest = desttop;

        for (int i = 0; i < CONSOLELINEHEIGHT; i++)
        {
            if (y + i >= 0 && *source)
            {
                byte    *dot = dest;

                if (italics)
                    dot += italicize[i];

                *dot = (!tinttab ? color : tinttab[(color << 8) + *dot]);

                if (!(y + i))
                    *dot = tinttab50[*dot];
                else if (y + i == 1)
                    *dot = tinttab25[*dot];
            }

            source++;
            dest += SCREENWIDTH;
        }
    }
}

void V_DrawOverlayTextPatch(byte *screen, int screenwidth, int x,
    int y, patch_t *patch, int width, int color, const byte *tinttab)
{
    byte    *desttop = &screen[y * screenwidth + x];

    for (int col = 0; col < width; col++, desttop++)
    {
        byte    *source = (byte *)patch + LONG(patch->columnoffset[col]) + 3;
        byte    *dest = desttop;

        for (int i = 0; i < CONSOLELINEHEIGHT; i++)
        {
            if (*source++)
                *dest = (!tinttab ? color : tinttab[(color << 8) + *dest]);

            dest += screenwidth;
        }
    }
}

void V_DrawConsolePatch(int x, int y, patch_t *patch, int maxwidth)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = MIN(SHORT(patch->width), maxwidth);

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        byte        *source = (byte *)column + 3;
        byte        *dest = desttop;
        int         count = column->length;
        int         height = y + 1;

        while (count-- > 0)
        {
            if (height > 0)
            {
                *dest = tinttab60[(nearestcolors[*source] << 8) + *dest];

                if (height == 1)
                    *dest = tinttab60[*dest];
                else if (height == 2)
                    *dest = tinttab30[*dest];
            }

            source++;
            dest += SCREENWIDTH;
            height++;
        }
    }
}

void V_DrawConsoleBrandingPatch(int x, int y, patch_t *patch)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++, desttop++, x++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        byte        *source = (byte *)column + 3;
        byte        *dest = desttop;
        int         count = column->length;
        int         height = y + 1;

        if (x > SCREENWIDTH)
            return;

        while (count-- > 0)
        {
            if (*source && height > 0)
                *dest = (*source == WHITE || *source == LIGHTGRAY2 ? nearestcolors[*source] :
                    tinttab50[(consolebrandingcolor = (nearestcolors[*source] << 8)) + *dest]);

            source++;
            dest += SCREENWIDTH;
            height++;
        }
    }
}

bool V_IsEmptyPatch(patch_t *patch)
{
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            if (column->length)
                return false;

            column = (column_t *)((byte *)column + 4);
        }
    }

    return true;
}

void V_DrawPatchToTempScreen(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    desttop = &tempscreen[((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                *dest = source[srccol >> FRACBITS];
                dest += SCREENWIDTH;

                if (!vanilla)
                    *(dest + SCREENWIDTH + 2) = nearestblack;

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawHUDText(int x, int y, byte *screen, patch_t *patch, int screenwidth)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) * SCREENSCALE;
    x -= SHORT(patch->leftoffset) * SCREENSCALE;

    desttop = &screen[y * screenwidth + x];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * screenwidth];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                *dest = source[srccol >> FRACBITS];
                dest += screenwidth;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentHUDText(int x, int y, byte *screen, patch_t *patch, int screenwidth)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) * SCREENSCALE;
    x -= SHORT(patch->leftoffset) * SCREENSCALE;

    desttop = &screen[y * screenwidth + x];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * screenwidth];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                *dest = tinttab25[(*dest << 8) + source[srccol >> FRACBITS]];
                dest += screenwidth;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawAltHUDText(int x, int y, byte *screen, patch_t *patch,
    bool italics, int color, int screenwidth, const byte *tinttab)
{
    byte        *desttop = &screen[y * screenwidth + x];
    const int   width = SHORT(patch->width);
    const int   italicize[] = { 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1 };

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        int         topdelta;
        const byte  length = column->length;

        // step through the posts in a column
        while ((topdelta = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = &desttop[topdelta * screenwidth];

            for (int i = 0; i < length; i++)
            {
                if (*source++)
                {
                    byte    *dot = dest;

                    if (italics)
                        dot += italicize[i];

                    *dot = color;
                }

                dest += screenwidth;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentAltHUDText(int x, int y, byte *screen, patch_t *patch,
    bool italics, int color, int screenwidth, const byte *tinttab)
{
    byte        *desttop = &screen[y * screenwidth + x];
    const int   width = SHORT(patch->width);
    const int   italicize[] = { 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1 };

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        int         topdelta;
        const byte  length = column->length;

        // step through the posts in a column
        while ((topdelta = column->topdelta) != 0xFF)
        {
            byte    *source = (byte *)column + 3;
            byte    *dest = &desttop[topdelta * screenwidth];

            for (int i = 0; i < length; i++)
            {
                if (*source++)
                {
                    byte    *dot = dest;

                    if (italics)
                        dot += italicize[i];

                    *dot = tinttab[(color << 8) + *dot];
                }

                dest += screenwidth;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawMenuPatch(int x, int y, patch_t *patch, bool highlight, int shadowwidth)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0, i = 0; col < width; col += DXI, i++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                const int   height = (((y + column->topdelta + length) * DY) >> FRACBITS) - count;

                if (height > 0)
                {
                    if (highlight)
                        *dest = source[srccol >> FRACBITS];
                    else
                        *dest = colormaps[0][4 * 256 + source[srccol >> FRACBITS]];
                }

                dest += SCREENWIDTH;

                if (height + SCREENSCALE > 0)
                {
                    byte    *dot = dest + SCREENWIDTH + SCREENSCALE;

                    if (i <= shadowwidth && *dot != 47 && *dot != 191)
                        *dot = black40[*dot];
                }

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawHUDPatch(int x, int y, patch_t *patch, const byte *tinttab)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawHighlightedHUDNumberPatch(int x, int y, patch_t *patch, const byte *tinttab)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                const byte  dot = *source++;

                *dest = (dot == DARKGRAY3 ? dot : white5[dot]);
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentHighlightedHUDNumberPatch(int x, int y, patch_t *patch, const byte *tinttab)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                const byte  dot = *source++;

                *dest = (dot == DARKGRAY3 ? tinttab33[*dest] : white5[dot]);
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentHUDPatch(int x, int y, patch_t *patch, const byte *tinttab)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                *dest = tinttab[(*source++ << 8) + *dest];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentHUDNumberPatch(int x, int y, patch_t *patch, const byte *tinttab)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                const byte  dot = *source++;

                *dest = (dot == DARKGRAY3 ? tinttab33[*dest] : tinttab[(dot << 8) + *dest]);
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawAltHUDPatch(int x, int y, patch_t *patch, int from, int to, const byte *tinttab)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                const byte  dot = *source++;

                if (dot == from)
                    *dest = to;
                else if (dot)
                    *dest = nearestcolors[dot];

                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentAltHUDPatch(int x, int y, patch_t *patch, int from, int to, const byte *tinttab)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    if (tinttab)
        to <<= 8;

    for (int col = 0; col < width; col++, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *source = (byte *)column + 3;
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                const byte  dot = *source++;

                if (dot == from)
                    *dest = (tinttab ? tinttab[to + *dest] : to);
                else if (dot == DARKGRAY1)
                    *dest = tinttab20[(nearestwhite << 8) + *dest];
                else if (dot)
                {
                    if (from == -1)
                        *dest = tinttab20[(nearestwhite << 8) + *dest];
                    else if (tinttab)
                        *dest = tinttab[(nearestcolors[dot] << 8) + *dest];
                }

                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentRedPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                *dest = tinttabred[(*dest << 8) + source[srccol >> FRACBITS]];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

//
// V_DrawFlippedPatch
//
// The co-ordinates for this procedure are always based upon a
// 320x200 screen and multiplies the size of the patch by the
// scaledwidth and scaledheight. The purpose of this is to produce
// a clean and undistorted patch upon the screen, The scaled screen
// size is based upon the nearest whole number ratio from the
// current screen size to 320x200.
//
// This Procedure flips the patch horizontally.
//
void V_DrawFlippedPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                *dest = source[srccol >> FRACBITS];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawFlippedShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;
    const int   black = (nearestblack << 8);
    const byte  *body = &tinttab40[black];
    const byte  *edge = &tinttab25[black];

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][(((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = ((length * DY / 10) >> FRACBITS) + 1;

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

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawFlippedSolidShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][(((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = ((length * DY / 10) >> FRACBITS) + 1;

            while (--count)
            {
                *dest = nearestblack;
                dest += SCREENWIDTH;
            }

            *dest = nearestblack;
            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawFlippedSpectreShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset) / 10;
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][(((y + 3) * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[((column->topdelta * DY / 10) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = ((length * DY / 10) >> FRACBITS) + 1;

            dest += SCREENWIDTH;

            while (--count)
            {
                *dest = black25[*dest];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawFlippedTranslucentRedPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                *dest = tinttabred[(*dest << 8) + source[srccol >> FRACBITS]];
                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawFuzzPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    fuzzpos = 0;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;

            while (count-- > 0)
            {
                if (count & 1)
                {
                    fuzzpos++;

                    if (!menuactive && !consoleactive && !paused)
                        fuzztable[fuzzpos] = FUZZ(-1, 1);
                }

                *dest = fullcolormap[6 * 256 + dest[fuzztable[fuzzpos]]];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawFlippedFuzzPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    fuzzpos = 0;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[SHORT(patch->width) - 1 - (col >> FRACBITS)]));

        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;

            while (count-- > 0)
            {
                if (count & 1)
                {
                    fuzzpos++;

                    if (!menuactive && !consoleactive && !paused)
                        fuzztable[fuzzpos] = FUZZ(-1, 1);
                }

                *dest = fullcolormap[6 * 256 + dest[fuzztable[fuzzpos]]];
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);
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
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                const byte  src = source[srccol >> FRACBITS];

                if (nogreen[src])
                {
                    byte    *dot;

                    *dest = src;
                    dot = dest + 2 * (size_t)SCREENWIDTH + 2;

                    if (*dot != 47 && *dot != 191)
                        *dot = black40[*dot];
                }

                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentNoGreenPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA;

    desttop = &screens[0][((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

    for (int col = 0; col < width; col += DXI, desttop++)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            const byte  *source = (byte *)column + 3;
            byte        *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
            const byte  length = column->length;
            int         count = (length * DY) >> FRACBITS;
            int         srccol = 0;

            while (count-- > 0)
            {
                const byte  src = source[srccol >> FRACBITS];

                if (nogreen[src])
                    *dest = tinttab33[(*dest << 8) + src];

                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawPixel(int x, int y, byte color, bool highlight, bool shadow)
{
    x += WIDESCREENDELTA;

#if SCREENSCALE == 2
    if (color == PINK)
    {
        if (shadow)
        {
            byte    *dot = *screens + ((size_t)y * SCREENWIDTH + x) * SCREENSCALE;

            *dot = black40[*dot];
            dot++;
            *dot = black40[*dot];
            dot += SCREENWIDTH;
            *dot = black40[*dot];
            dot--;
            *dot = black40[*dot];
        }
    }
    else if (color && color != 32)
    {
        byte    *dot = *screens + ((size_t)y * SCREENWIDTH + x) * SCREENSCALE;

        if (!highlight)
            color = colormaps[0][4 * 256 + color];

        *(dot++) = color;
        *dot = color;
        *(dot += SCREENWIDTH) = color;
        *(--dot) = color;
    }
#elif SCREENSCALE == 1
    if (color == PINK)
    {
        if (shadow)
        {
            byte    *dot = *screens + ((size_t)y * SCREENWIDTH + x);

            *dot = black40[*dot];
        }
    }
    else if (color && color != 32)
        screens[0][y * SCREENWIDTH + x] = color;
#else
    if (color == PINK)
    {
        if (shadow)
        {
            x *= SCREENSCALE;
            y *= SCREENSCALE;

            for (int yy = 0; yy < SCREENSCALE; yy++)
                for (int xx = 0; xx < SCREENSCALE; xx++)
                {
                    byte    *dot = *screens + ((size_t)(y + yy) * SCREENWIDTH + x + xx);

                    *dot = black40[*dot];
                }
        }
    }
    else if (color && color != 32)
    {
        x *= SCREENSCALE;
        y *= SCREENSCALE;

        for (int yy = 0; yy < SCREENSCALE; yy++)
            for (int xx = 0; xx < SCREENSCALE; xx++)
                screens[0][(size_t)(y + yy) * SCREENWIDTH + x + xx] = color;
    }
#endif
}

static void V_LowGraphicDetail(int left, int top, int width, int height, int pixelwidth, int pixelheight)
{
    for (int y = top; y < height; y += pixelheight)
        for (int x = left; x < width; x += pixelwidth)
        {
            byte        *dot = *screens + y + x;
            const byte  color = *dot;

            for (int xx = 1; xx < pixelwidth && x + xx < width; xx++)
                *(dot + xx) = color;

            for (int yy = SCREENWIDTH; yy < pixelheight && y + yy < height; yy += SCREENWIDTH)
                for (int xx = 0; xx < pixelwidth && x + xx < width; xx++)
                    *(dot + yy + xx) = color;
        }
}

static void V_LowGraphicDetail_SSAA(int left, int top, int width, int height, int pixelwidth, int pixelheight)
{
    for (int y = top; y < height; y += pixelheight)
        for (int x = left; x < width; x += pixelwidth)
        {
            byte    *dot1 = *screens + y + x;
            byte    color;

            if (y + pixelheight < height)
            {
                if (x + pixelwidth < width)
                {
                    const byte  *dot2 = dot1 + pixelwidth;
                    const byte  *dot3 = dot2 + pixelheight;
                    const byte  *dot4 = dot3 - pixelwidth;

                    color = tinttab50[(tinttab50[(*dot1 << 8) + *dot2] << 8) + tinttab50[(*dot3 << 8) + *dot4]];
                }
                else
                    color = tinttab50[(*dot1 << 8) + *(dot1 + pixelheight)];

                for (int yy = 0; yy < pixelheight && y + yy < height; yy += SCREENWIDTH)
                    for (int xx = 0; xx < pixelwidth && x + xx < width; xx++)
                        *(dot1 + yy + xx) = color;
            }
            else if (x + pixelwidth < width)
            {
                color = tinttab50[(*dot1 << 8) + *(dot1 + pixelwidth)];

                for (int yy = 0; yy < pixelheight && y + yy < height; yy += SCREENWIDTH)
                    for (int xx = 0; xx < pixelwidth && x + xx < width; xx++)
                        *(dot1 + yy + xx) = color;
            }
            else
            {
                color = *dot1;

                for (int xx = 1; xx < pixelwidth && x + xx < width; xx++)
                    *(dot1 + xx) = color;

                for (int yy = SCREENWIDTH; yy < pixelheight && y + yy < height; yy += SCREENWIDTH)
                    for (int xx = 0; xx < pixelwidth && x + xx < width; xx++)
                        *(dot1 + yy + xx) = color;
            }
        }
}

static void V_LowGraphicDetail_2x2(int left, int top, int width, int height, int pixelwidth, int pixelheight)
{
    for (int y = top; y < height; y += 2 * SCREENWIDTH)
        for (int x = left; x < width; x += 2)
        {
            byte        *dot = *screens + y + x;
            const byte  color = *dot;

            *(++dot) = color;
            *(dot += SCREENWIDTH) = color;
            *(--dot) = color;
        }
}

static void V_LowGraphicDetail_2x2_SSAA(int left, int top, int width, int height, int pixelwidth, int pixelheight)
{
    for (int y = top; y < height; y += 2 * SCREENWIDTH)
        for (int x = left; x < width; x += 2)
        {
            byte        *dot1 = *screens + y + x;
            byte        *dot2 = dot1 + 1;
            byte        *dot3 = dot2 + SCREENWIDTH;
            byte        *dot4 = dot3 - 1;
            const byte  color = tinttab50[(tinttab50[(*dot1 << 8) + *dot2] << 8) + tinttab50[(*dot3 << 8) + *dot4]];

            *dot1 = color;
            *dot2 = color;
            *dot3 = color;
            *dot4 = color;
        }
}

void GetPixelSize(void)
{
    int width = -1;
    int height = -1;

    if (sscanf(r_lowpixelsize, "%2ix%2i", &width, &height) == 2
        && ((width >= 2 && height >= 1) || (width >= 1 && height >= 2)))
    {
        if (width == 2 && height == 2)
            postprocessfunc = (r_antialiasing ? &V_LowGraphicDetail_2x2_SSAA : &V_LowGraphicDetail_2x2);
        else
        {
            lowpixelwidth = width;
            lowpixelheight = height * SCREENWIDTH;
            postprocessfunc = (r_antialiasing ? &V_LowGraphicDetail_SSAA : &V_LowGraphicDetail);
        }
    }
    else
    {
        r_lowpixelsize = r_lowpixelsize_default;
        M_SaveCVARs();

        postprocessfunc = (r_antialiasing ? &V_LowGraphicDetail_2x2_SSAA : &V_LowGraphicDetail_2x2);
    }
}

void V_InvertScreen(void)
{
    const int           width = viewwindowx + viewwidth;
    const int           height = (viewwindowy + viewheight) * SCREENWIDTH;
    const lighttable_t  *colormap = &colormaps[0][32 * 256];

    for (int y = viewwindowy * SCREENWIDTH; y < height; y += SCREENWIDTH)
        for (int x = viewwindowx; x < width; x++)
        {
            byte    *dot = *screens + y + x;

            *dot = *(colormap + *dot);
        }
}

//
// V_Init
//
void V_Init(void)
{
    byte                *base = Z_Malloc(MAXSCREENAREA * NUMSCREENS, PU_STATIC, NULL);
    const SDL_version   *linked = IMG_Linked_Version();

    if (linked->major != SDL_IMAGE_MAJOR_VERSION || linked->minor != SDL_IMAGE_MINOR_VERSION)
        I_Error("The wrong version of " SDL_IMAGE_FILENAME " was found. " DOOMRETRO_NAME " requires v%i.%i.%i.",
            SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);

    if (linked->patch != SDL_IMAGE_PATCHLEVEL)
        C_Warning(1, "The wrong version of " BOLD(SDL_IMAGE_FILENAME) " was found. " ITALICS(DOOMRETRO_NAME) " requires v%i.%i.%i.",
            SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);

    for (int i = 0; i < NUMSCREENS; i++)
        screens[i] = &base[i * MAXSCREENAREA];
}

char    lbmname1[MAX_PATH];
char    lbmpath1[MAX_PATH];
char    lbmpath2[MAX_PATH];

static bool V_SavePNG(SDL_Renderer *sdlrenderer, const char *path)
{
    bool    result = false;
    int     width;
    int     height;

    if (!SDL_GetRendererOutputSize(sdlrenderer, &width, &height))
    {
        SDL_Surface *screenshot = SDL_CreateRGBSurface(0, (vid_widescreen ? width : height * 4 / 3), height, 32, 0, 0, 0, 0);

        if (screenshot)
        {
            if (!SDL_RenderReadPixels(sdlrenderer, NULL, 0, screenshot->pixels, screenshot->pitch))
                result = !IMG_SavePNG(screenshot, path);

            SDL_FreeSurface(screenshot);
        }
    }

    return result;
}

bool V_ScreenShot(void)
{
    bool    result = false;
    char    mapname[128];
    char    *temp1;
    int     count = 0;

    if (consoleactive)
        M_StringCopy(mapname, "Console", sizeof(mapname));
    else if (menuactive)
        M_StringCopy(mapname, (helpscreen ? "Help" : "Menu"), sizeof(mapname));
    else if (automapactive)
        M_StringCopy(mapname, "Automap", sizeof(mapname));
    else if (paused)
        M_StringCopy(mapname, "Paused", sizeof(mapname));
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
                M_StringCopy(mapname, (titlesequence == 1 ? "Credits" : "Title"), sizeof(mapname));
                break;

            default:
            {
                char    *temp2 = titlecase(maptitle);

                M_StringCopy(mapname, temp2, sizeof(mapname));
                free(temp2);
                break;
            }
        }

    if (M_StringStartsWith(mapname, "The "))
    {
        char    *temp2 = M_SubString(mapname, 4, strlen(mapname) - 4);

        M_snprintf(mapname, sizeof(mapname), "%s, The", temp2);
        free(temp2);
    }
    else if (M_StringStartsWith(mapname, "A "))
    {
        char    *temp2 = M_SubString(mapname, 2, strlen(mapname) - 2);

        M_snprintf(mapname, sizeof(mapname), "%s, A", temp2);
        free(temp2);
    }

    temp1 = makevalidfilename(mapname);

    do
    {
        if (!count)
            M_snprintf(lbmname1, sizeof(lbmname1), "%s.png", temp1);
        else
        {
            char    *temp2 = commify(count);

            M_snprintf(lbmname1, sizeof(lbmname1), "%s (%s).png", temp1, temp2);
            free(temp2);
        }

        count++;
        M_snprintf(lbmpath1, sizeof(lbmpath1), "%s%s", screenshotfolder, lbmname1);
    } while (M_FileExists(lbmpath1));

    result = V_SavePNG(renderer, lbmpath1);

    if (result && mapwindow && gamestate == GS_LEVEL)
    {
        lbmpath2[0] = '\0';

        do
        {
            char    *temp2 = commify(count++);

            M_snprintf(lbmpath2, sizeof(lbmpath2), "%s%s (%s).png", screenshotfolder, temp1, temp2);
            free(temp2);
        } while (M_FileExists(lbmpath2));

        V_SavePNG(maprenderer, lbmpath2);
    }

    free(temp1);

    return result;
}
