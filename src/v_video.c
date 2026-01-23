/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <SDL3_image/SDL_image.h>

#include "c_cmds.h"
#include "c_console.h"
#include "d_iwad.h"
#include "d_main.h"
#include "doomstat.h"
#include "hu_lib.h"
#include "i_colors.h"
#include "i_swap.h"
#include "m_array.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_setup.h"
#include "r_draw.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

byte    *screens[NUMSCREENS];
int     lowpixelwidth;
int     lowpixelheight;

void (*postprocessfunc)(byte *, int, int, int, int, int, int, int);

byte    *colortranslation[10];
byte    cr_gold[256];
byte    cr_none[256];

typedef struct
{
    const char  *name;
    byte        **lump;
} colortranslation_t;

static colortranslation_t colortranslations[] =
{
    { "CRRED",    &colortranslation[0] },
    { "CRGRAY",   &colortranslation[1] },
    { "CRGREEN",  &colortranslation[2] },
    { "CRBLUE",   &colortranslation[3] },
    { "CRYELLOW", &colortranslation[4] },
    { "CRBLACK",  &colortranslation[5] },
    { "CRPURPLE", &colortranslation[6] },
    { "CRWHITE",  &colortranslation[7] },
    { "CRORANGE", &colortranslation[8] },
    { "",         NULL                 }
};

void V_InitColorTranslation(void)
{
    for (colortranslation_t *p = colortranslations; *p->name; p++)
        *p->lump = W_CacheLumpName(p->name);

    for (int i = 0; i < 256; i++)
    {
        cr_gold[i] = I_GoldTranslation(PLAYPAL, (byte)i);
        cr_none[i] = i;
    }
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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

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

void V_FillPillarboxes(int screen, int color)
{
    const int   pillarwidth = (SCREENWIDTH - NONWIDEWIDTH) / 2;
    byte        *base = screens[screen];
    byte        *end = base + SCREENWIDTH * (SCREENHEIGHT - 1);

    memset(base, color, pillarwidth);

    for (byte *row = base; row < end; row += SCREENWIDTH)
        memset(row + SCREENWIDTH - pillarwidth, color, pillarwidth * 2);

    memset(base + SCREENWIDTH * SCREENHEIGHT - pillarwidth, color, pillarwidth);
}

void V_DrawPagePatch(int screen, patch_t *patch)
{
    if (SCREENWIDTH != NONWIDEWIDTH)
    {
        static patch_t  *prevpatch = NULL;
        static int      pillarboxcolor;

        if (prevpatch != patch)
        {
            pillarboxcolor = FindDominantEdgeColor(patch);
            prevpatch = patch;
        }

        V_FillPillarboxes(screen, pillarboxcolor);
    }

    V_DrawWidePatch((SCREENWIDTH / 2 - SHORT(patch->width)) / 2, 0, screen, patch);
}

void V_DrawShadowPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);

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

void V_DrawColorBackPatch(const int x, const int y, const patch_t *patch, const int width,
    const int height, const int color)
{
    byte    *desttop = &screens[0][y * SCREENWIDTH + x];

    for (int col = 0; col < width; col++, desttop++)
    {
        byte    *source = (byte *)patch + LONG(patch->columnoffset[col]) + 3;
        byte    *dest = desttop;

        for (int i = 0; i < height; i++)
        {
            if (*source == WHITE)
                *dest = color;
            else if (*source != BLACK)
                *dest = tinttab33[*dest];

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

void V_DrawOverlayTextPatch(byte *screen, int screenwidth, int x,
    int y, patch_t *patch, int width, int color, int shadowcolor, const byte *tinttab)
{
    byte    *desttop = &screen[y * screenwidth + x];

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
            bool    shadow = false;

            for (int i = 0; i < length; i++)
            {
                if (*source++)
                {
                    *dest = (tinttab ? tinttab[(color << 8) + *dest] : color);
                    shadow = (color != shadowcolor);
                }
                else if (shadow && shadowcolor != -1)
                {
                    *dest = (tinttab ? black10[*dest] : shadowcolor);
                    shadow = false;
                }

                dest += screenwidth;
            }

            if (shadow && shadowcolor != -1)
                *dest = (tinttab ? black10[*dest] : shadowcolor);

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawConsoleHeaderPatch(int x, int y, patch_t *patch, const int maxwidth, const int color1,
    const int color2)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = MIN(SHORT(patch->width), maxwidth);

    for (int col = 0; col < width; col++, desttop++)
    {
        const column_t  *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        byte            *source = (byte *)column + 3;
        byte            *dest = desttop;
        int             count = column->length;
        int             height = y + 1;

        while (count-- > 0)
        {
            if (height > 0)
            {
                *dest = (*source == WHITE ? (color2 == nearestblack ?
                    tinttab20[color1 + nearestblack] : color2) : tinttab60[color1 + *dest]);

                if (height == 1)
                    *dest = tinttab60[*dest];
                else if (height == 2)
                    *dest = tinttab30[*dest];

                if (col == width - 1)
                    for (int xx = 1; xx <= maxwidth - width; xx++)
                    {
                        byte    *dot = dest + xx;

                        *dot = tinttab60[color1 + *dot];

                        if (height == 1)
                            *dot = tinttab60[*dot];
                        else if (height == 2)
                            *dot = tinttab30[*dot];
                    }
            }

            source++;
            dest += SCREENWIDTH;
            height++;
        }
    }
}

void V_DrawConsoleBrandingPatch(int x, int y, patch_t *patch, const int color1, const int color2,
    const int color3)
{
    byte        *desttop = &screens[0][y * SCREENWIDTH + x];
    const int   width = SHORT(patch->width);

    for (int col = 0; col < width; col++, desttop++, x++)
    {
        const column_t  *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        byte            *source = (byte *)column + 3;
        byte            *dest = desttop;
        int             count = column->length;
        int             height = y + 1;

        if (x > SCREENWIDTH)
            return;

        while (count-- > 0)
        {
            if (*source && height > 0)
            {
                if (*source == WHITE)
                    *dest = (color3 == nearestblack ? tinttab20[color1 + nearestblack] : color3);
                else if (*source == LIGHTGRAY2)
                    *dest = (color3 == nearestblack ? tinttab33[color1 + nearestblack] : nearestcolors[LIGHTGRAY2]);
                else if (*source == CONSOLEEDGECOLOR2)
                    *dest = tinttab60[color2 + *dest];
                else
                    *dest = tinttab60[color1 + *dest];

                if (height == 1)
                    *dest = tinttab60[*dest];
                else if (height == 2)
                    *dest = tinttab30[*dest];

                if (col == width - 1)
                    for (int xx = 1; xx < SCREENWIDTH - x; xx++)
                    {
                        byte    *dot = dest + xx;

                        *dot = tinttab60[color1 + *dot];

                        if (height == 1)
                            *dot = tinttab60[*dot];
                        else if (height == 2)
                            *dot = tinttab30[*dot];
                    }
            }

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

void V_DrawPatchToTempScreen(int x, int y, patch_t *patch, byte *cr, int screenwidth)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    x -= SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

    desttop = &tempscreen[((y * DY) >> FRACBITS) * screenwidth + ((x * DX) >> FRACBITS)];

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
                *dest = cr[source[srccol >> FRACBITS]];
                dest += screenwidth;

                if (!vanilla)
                    *(dest + screenwidth + 2) = nearestblack;

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawAltHUDText(int x, int y, byte *screen, patch_t *patch,
    bool italics, int color, int shadowcolor, int screenwidth, const byte *tinttab)
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
            bool    shadow = false;

            for (int i = 0; i < length; i++)
            {
                if (*source++)
                {
                    byte    *dot = dest;

                    if (italics)
                        dot += italicize[i];

                    *dot = color;
                    shadow = true;
                }
                else if (shadow && shadowcolor != -1)
                {
                    if (italics)
                        *(dest + italicize[i]) = shadowcolor;
                    else
                        *dest = shadowcolor;

                    shadow = false;
                }

                dest += screenwidth;
            }

            if (shadow && shadowcolor != -1)
            {
                if (italics)
                    *(dest + italicize[length - 1]) = shadowcolor;
                else
                    *dest = shadowcolor;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawTranslucentAltHUDText(int x, int y, byte *screen, patch_t *patch,
    bool italics, int color, int shadowcolor, int screenwidth, const byte *tinttab)
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
            bool    shadow = false;

            for (int i = 0; i < length; i++)
            {
                if (*source++)
                {
                    byte    *dot = dest;

                    if (italics)
                        dot += italicize[i];

                    *dot = tinttab[(color << 8) + *dot];
                    shadow = true;
                }
                else if (shadow && shadowcolor != -1)
                {
                    if (italics)
                        *(dest + italicize[i]) = black10[*(dest + italicize[i])];
                    else
                        *dest = black10[*dest];

                    shadow = false;
                }

                dest += screenwidth;
            }

            if (shadow && shadowcolor != -1)
            {
                if (italics)
                    *(dest + italicize[length - 1]) = black10[*(dest + italicize[length - 1])];
                else
                    *dest = black10[*dest];
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawMenuPatch(int x, int y, patch_t *patch, bool highlight, int shadowwidth)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

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
                    const byte  dot = source[srccol >> FRACBITS];

                    *dest = (menuhighlight ? (highlight ? gold4[dot] : colormaps[0][6 * 256 + dot]) : dot);
                }

                dest += SCREENWIDTH;

                if (height + 2 > 0 && menushadow)
                {
                    byte    *dot = dest + SCREENWIDTH + 2;

                    if (i <= shadowwidth && *dot != 47 && *dot != 191)
                        *dot = black40[*dot];
                }

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawBigFontPatch(int x, int y, patch_t *patch, bool highlight, int shadowwidth)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

    desttop = &tempscreen[((y * DY) >> FRACBITS) * SCREENWIDTH + ((x * DX) >> FRACBITS)];

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
                    const byte  dot = source[srccol >> FRACBITS];

                    *dest = (menuhighlight ? (highlight ? gold4[dot] : colormaps[0][6 * 256 + dot]) : dot);
                }

                dest += SCREENWIDTH;

                if (menushadow && height + 2 > 0 && i <= shadowwidth)
                {
                    byte    *dot = dest + SCREENWIDTH + 2;

                    if (*dot != 47 && *dot != 191)
                        *dot = BLUE1;
                }

                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawHelpPatch(patch_t *patch)
{
    byte        *desttop = &screens[0][((WIDESCREENDELTA * DX) >> FRACBITS)];
    const int   width = SHORT(patch->width) << FRACBITS;

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
                byte    *dot;

                *dest = nearestcolors[source[srccol >> FRACBITS]];
                dest += SCREENWIDTH;
                dot = dest + SCREENWIDTH + 2;
                *dot = black40[*dot];
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

void V_DrawAltHUDPatch(int x, int y, patch_t *patch, int from, int to, const byte *tinttab, int shadowcolor)
{
    const int   width = SHORT(patch->width);
    byte        *desttop;

    x -= SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

    desttop = &screens[0][y * SCREENWIDTH + x];

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
            byte        dot = 0;

            while (count-- > 0)
            {
                if ((dot = *source++) == from)
                    *dest = to;
                else if (dot)
                    *dest = nearestcolors[dot];

                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + length + 4);

            if (shadowcolor != -1 && dot != DARKGRAY1)
                *dest = shadowcolor;
        }
    }
}

void V_DrawTranslucentAltHUDPatch(int x, int y, patch_t *patch, int from, int to, const byte *tinttab, int shadowcolor)
{
    const int   width = SHORT(patch->width);
    byte        *desttop;

    x -= SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

    desttop = &screens[0][y * SCREENWIDTH + x];

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
            byte        dot = 0;

            while (count-- > 0)
            {
                if ((dot = *source++) == from)
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

            if (shadowcolor != -1 && dot != DARKGRAY1)
                *dest = black10[*dest];
        }
    }
}

void V_DrawAltHUDWeaponPatch(int x, int y, patch_t *patch, int color, int shadowcolor, const byte *tinttab)
{
    const int   width = SHORT(patch->width);
    byte        *desttop = &screens[0][y * SCREENWIDTH + x + width];

    for (int col = 0; col < width; col++, desttop--)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        int         yy = y;

        if (x + width - col >= SCREENWIDTH)
            continue;

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                *dest = color;
                dest += SCREENWIDTH;

                if (++yy == SCREENHEIGHT)
                    break;
            }

            column = (column_t *)((byte *)column + length + 4);

            if (shadowcolor != -1)
                *dest = shadowcolor;
        }
    }
}

void V_DrawTranslucentAltHUDWeaponPatch(int x, int y, patch_t *patch, int color, int shadowcolor, const byte *tinttab)
{
    const int   width = SHORT(patch->width);
    byte        *desttop = &screens[0][y * SCREENWIDTH + x + width];

    for (int col = 0; col < width; col++, desttop--)
    {
        column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
        int         yy = y;

        if (x + width - col >= SCREENWIDTH)
            continue;

        // step through the posts in a column
        while (column->topdelta != 0xFF)
        {
            byte        *dest = &desttop[column->topdelta * SCREENWIDTH];
            const byte  length = column->length;
            byte        count = length;

            while (count-- > 0)
            {
                *dest = tinttab[(color << 8) + *dest];
                dest += SCREENWIDTH;

                if (++yy == SCREENHEIGHT - 1)
                    break;
            }

            column = (column_t *)((byte *)column + length + 4);
            *dest = black10[*dest];
        }
    }
}

void V_DrawTranslucentRedPatch(int x, int y, patch_t *patch)
{
    byte        *desttop;
    const int   width = SHORT(patch->width) << FRACBITS;

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset) / 10;

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset) / 10;

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset) / 10;

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

    fuzz1pos = 0;

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
                    fuzz1pos++;

                    if (!menuactive && !consoleactive && !paused)
                        fuzz1table[fuzz1pos] = FUZZ1(-1, 1);
                }

                *dest = fullcolormap[6 * 256 + dest[fuzz1table[fuzz1pos]]];
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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

    fuzz1pos = 0;

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
                    fuzz1pos++;

                    if (!menuactive && !consoleactive && !paused)
                        fuzz1table[fuzz1pos] = FUZZ1(-1, 1);
                }

                *dest = fullcolormap[6 * 256 + dest[fuzz1table[fuzz1pos]]];
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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

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

    x += WIDESCREENDELTA - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

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
                    *dest = tinttab25[(*dest << 8) + src];

                dest += SCREENWIDTH;
                srccol += DYI;
            }

            column = (column_t *)((byte *)column + length + 4);
        }
    }
}

void V_DrawPixel(int x, int y, byte color, bool highlight, bool shadow)
{
    if (color == PINK)
    {
        if (shadow && menushadow)
        {
            byte    *dot = *screens + (y * SCREENWIDTH + x + WIDESCREENDELTA) * 2;

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
        byte    *dot = *screens + (y * SCREENWIDTH + x + WIDESCREENDELTA) * 2;

        if (menuhighlight)
            color = (highlight ? gold4[color] : colormaps[0][6 * 256 + color]);

        *(dot++) = color;
        *dot = color;
        *(dot += SCREENWIDTH) = color;
        *(--dot) = color;
    }
}

static void V_LowGraphicDetail(byte *screen, int screenwidth, int left,
    int top, int width, int height, int pixelwidth, int pixelheight)
{
    for (int y = top; y < height; y += pixelheight)
    {
        const int   maxy = MIN(pixelheight, height - y);

        for (int x = left; x < width; x += pixelwidth)
        {
            const int       blockw = MIN(pixelwidth, width - x);
            byte *restrict  dot = screen + y + x;
            const byte      color = *dot;

            if (blockw > 1)
                memset(dot + 1, color, (size_t)(blockw - 1));

            for (int yy = screenwidth; yy < maxy; yy += screenwidth)
                memset(dot + yy, color, (size_t)blockw);
        }
    }
}

static void V_LowGraphicDetail_Antialiased(byte *screen, int screenwidth,
    int left, int top, int width, int height, int pixelwidth, int pixelheight)
{
    for (int y = top; y < height; y += pixelheight)
    {
        const int   maxy = MIN(pixelheight, height - y);

        for (int x = left; x < width; x += pixelwidth)
        {
            const int       blockw = MIN(pixelwidth, width - x);
            byte *restrict  dot1 = screen + y + x;
            byte            color;

            if (y + pixelheight < height)
            {
                if (x + pixelwidth < width)
                {
                    const byte  *dot2 = dot1 + pixelwidth;
                    const byte  *dot3 = dot2 + pixelheight;
                    const byte  *dot4 = dot3 - pixelwidth;

                    color = tinttab50[(tinttab50[(*dot1 << 8) + *dot2] << 8)
                        + tinttab50[(*dot3 << 8) + *dot4]];
                }
                else
                    color = tinttab50[(*dot1 << 8) + *(dot1 + pixelheight)];

                for (int yy = 0; yy < maxy; yy += screenwidth)
                    memset(dot1 + yy, color, (size_t)blockw);
            }
            else if (x + pixelwidth < width)
            {
                color = tinttab50[(*dot1 << 8) + *(dot1 + pixelwidth)];

                for (int yy = 0; yy < maxy; yy += screenwidth)
                    memset(dot1 + yy, color, (size_t)blockw);
            }
            else
            {
                color = *dot1;

                if (blockw > 1)
                    memset(dot1 + 1, color, (size_t)(blockw - 1));

                for (int yy = screenwidth; yy < maxy; yy += screenwidth)
                    memset(dot1 + yy, color, (size_t)blockw);
            }
        }
    }
}

void V_LowGraphicDetail_2x2(byte *screen, int screenwidth, int left,
    int top, int width, int height, int pixelwidth, int pixelheight)
{
    const int   xend = width - 1;

    for (int y = top; y < height; y += 2 * screenwidth)
    {
        byte    *row0 = screen + y;
        byte    *row1 = row0 + screenwidth;

        for (int x = left; x < xend; x += 2)
        {
            byte        *dot0 = row0 + x;
            const byte  color = *dot0;

            dot0[1] = color;
            row1[x] = color;
            row1[x + 1] = color;
        }
    }
}

static void V_LowGraphicDetail_2x2_Antialiased(byte *screen, int screenwidth,
    int left, int top, int width, int height, int pixelwidth, int pixelheight)
{
    const int   xend = width - 1;

    for (int y = top; y < height; y += 2 * screenwidth)
    {
        byte   *row0 = screen + y;
        byte   *row1 = row0 + screenwidth;

        for (int x = left; x < xend; x += 2)
        {
            byte        *dot1 = row0 + x;
            const byte  *dot2 = dot1 + 1;
            const byte  *dot3 = dot2 + screenwidth;
            const byte  *dot4 = dot3 - 1;
            const byte  color = tinttab50[(tinttab50[(*dot1 << 8) + *dot2] << 8)
                            + tinttab50[(*dot3 << 8) + *dot4]];

            dot1[0] = color;
            dot1[1] = color;
            row1[x] = color;
            row1[x + 1] = color;
        }
    }
}

void GetPixelSize(void)
{
    int width = -1;
    int height = -1;

    if (sscanf(r_lowpixelsize, "%2dx%2d", &width, &height) == 2
        && ((width >= 2 && height >= 1) || (width >= 1 && height >= 2)))
    {
        if (width == 2 && height == 2)
            postprocessfunc = (r_antialiasing ? &V_LowGraphicDetail_2x2_Antialiased : &V_LowGraphicDetail_2x2);
        else
        {
            lowpixelwidth = width;
            lowpixelheight = height * SCREENWIDTH;
            postprocessfunc = (r_antialiasing ? &V_LowGraphicDetail_Antialiased : &V_LowGraphicDetail);
        }
    }
    else
    {
        r_lowpixelsize = r_lowpixelsize_default;
        M_SaveCVARs();

        postprocessfunc = (r_antialiasing ? &V_LowGraphicDetail_2x2_Antialiased : &V_LowGraphicDetail_2x2);
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

    if (linked->major != SDL_IMAGE_MAJOR_VERSION
        || linked->minor != SDL_IMAGE_MINOR_VERSION
        || linked->patch != SDL_IMAGE_PATCHLEVEL)
        C_Warning(0, "The wrong version of " SDL_IMAGE_FILENAME " was found. "
            DOOMRETRO_NAME " requires v%i.%i.%i.",
            SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);

    for (int i = 0; i < NUMSCREENS; i++)
        screens[i] = &base[i * MAXSCREENAREA];
}

char        lbmname1[MAX_PATH];
char        lbmpath1[MAX_PATH];
static char lbmname2[MAX_PATH];
char        lbmpath2[MAX_PATH] = "";

static bool V_SavePNG(SDL_Window *sdlwindow, const char *path)
{
    bool    result = false;
    int     width = 0;
    int     height = 0;

    SDL_GetWindowSize(sdlwindow, &width, &height);

    if (width > 0 && height > 0)
    {
        SDL_Surface *screenshot = SDL_CreateRGBSurface(0, (vid_widescreen ? width : height * 4 / 3),
                        height, 32, 0, 0, 0, 0);

        if (screenshot)
        {
            if (!SDL_RenderReadPixels(SDL_GetRenderer(sdlwindow), NULL, 0, screenshot->pixels, screenshot->pitch))
                result = !IMG_SavePNG(screenshot, path);

            SDL_FreeSurface(screenshot);
        }
    }

    return result;
}

bool V_ScreenShot(void)
{
    bool    result;
    char    mapname[128];
    char    *temp1;
    char    *temp2;
    int     count = 0;

    if (consoleactive)
        M_StringCopy(mapname, "Console", sizeof(mapname));
    else if (palettescreen)
        M_StringCopy(mapname, "Palette", sizeof(mapname));
    else if (helpscreen)
        M_StringCopy(mapname, "Help", sizeof(mapname));
    else if (menuactive)
        M_StringCopy(mapname, "Menu", sizeof(mapname));
    else if (automapactive)
        M_StringCopy(mapname, "Automap", sizeof(mapname));
    else if (paused)
        M_StringCopy(mapname, "Paused", sizeof(mapname));
    else if (splashscreen)
        M_StringCopy(mapname, "Splash", sizeof(mapname));
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
                temp2 = titlecase(maptitle);
                M_StringCopy(mapname, temp2, sizeof(mapname));
                free(temp2);
                break;
        }

    if (M_StringStartsWith(mapname, "The "))
    {
        temp2 = M_SubString(mapname, 4, strlen(mapname) - 4);
        M_snprintf(mapname, sizeof(mapname), "%s, The", temp2);
        free(temp2);
    }
    else if (M_StringStartsWith(mapname, "A "))
    {
        temp2 = M_SubString(mapname, 2, strlen(mapname) - 2);
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
            temp2 = commify(count);
            M_snprintf(lbmname1, sizeof(lbmname1), "%s (%s).png", temp1, temp2);
            free(temp2);
        }

        count++;
        M_snprintf(lbmpath1, sizeof(lbmpath1), "%s%s", screenshotfolder, lbmname1);
    } while (M_FileExists(lbmpath1));

    free(temp1);

    if ((result = V_SavePNG(window, lbmpath1)) && mapwindow && gamestate == GS_LEVEL)
    {
        count = 0;

        do
        {
            if (!count)
                M_StringCopy(lbmname2, "Automap.png", sizeof(lbmname2));
            else
            {
                temp2 = commify(count);
                M_snprintf(lbmname2, sizeof(lbmname2), "Automap (%s).png", temp2);
                free(temp2);
            }

            count++;
            M_snprintf(lbmpath2, sizeof(lbmpath2), "%s%s", screenshotfolder, lbmname2);
        } while (M_FileExists(lbmpath2));

        V_SavePNG(mapwindow, lbmpath2);
    }

    return result;
}

typedef struct
{
    byte    row_off;
    byte    *pixels;
} vpost_t;

typedef struct
{
    vpost_t *posts;
} vcolumn_t;

#define PUTBYTE(r, v)  \
    *r = (uint8_t)(v); \
    r++

#define PUTSHORT(r, v)                              \
    *(r + 0) = (byte)(((uint16_t)(v) >> 0) & 0xFF); \
    *(r + 1) = (byte)(((uint16_t)(v) >> 8) & 0xFF); \
    r += 2

#define PUTLONG(r, v)                                \
    *(r + 0) = (byte)(((uint32_t)(v) >> 0) & 0xFF);  \
    *(r + 1) = (byte)(((uint32_t)(v) >> 8) & 0xFF);  \
    *(r + 2) = (byte)(((uint32_t)(v) >> 16) & 0xFF); \
    *(r + 3) = (byte)(((uint32_t)(v) >> 24) & 0xFF); \
    r += 4

//
// Converts a linear graphic to a patch with transparency. Mostly straight
// from psxwadgen, which is mostly straight from SLADE.
//
patch_t *V_LinearToTransPatch(const byte *data, int width, int height, int color_key)
{
    vcolumn_t   *columns = NULL;
    size_t      size = 0;
    byte        *output;
    byte        *rover;
    byte        *col_offsets;

    // Go through columns
    for (int c = 0; c < width; c++)
    {
        vcolumn_t   col = { 0 };
        vpost_t     post = { 0 };
        bool        ispost = false;
        bool        first_254 = true;   // first 254 pixels use absolute offsets
        byte        row_off = 0;
        uint32_t    offset = c;

        post.row_off = 0;

        for (int r = 0; r < height; r++)
        {
            // if we're at offset 254, create a dummy post for tall DOOM GFX support
            if (row_off == 254)
            {
                // Finish current post if any
                if (ispost)
                {
                    array_push(col.posts, post);
                    post.pixels = NULL;
                    ispost = false;
                }

                // Begin relative offsets
                first_254 = false;

                // Create dummy post
                post.row_off = 254;
                array_push(col.posts, post);

                // Clear post
                row_off = 0;
            }

            // If the current pixel is not transparent, add it to the current post
            if (data[offset] != color_key)
            {
                // If we're not currently building a post, begin one and set its offset
                if (!ispost)
                {
                    // Set offset
                    post.row_off = row_off;

                    // Reset offset if we're in relative offsets mode
                    if (!first_254)
                        row_off = 0;

                    // Start post
                    ispost = true;
                }

                // Add the pixel to the post
                array_push(post.pixels, data[offset]);
            }
            else if (ispost)
            {
                // If the current pixel is transparent and we are currently
                // building a post, add the current post to the list and clear it
                array_push(col.posts, post);
                post.pixels = NULL;
                ispost = false;
            }

            // Go to next row
            offset += width;
            row_off++;
        }

        // If the column ended with a post, add it
        if (ispost)
            array_push(col.posts, post);

        // Add the column data
        array_push(columns, col);
    }

    // Calculate needed memory size to allocate patch buffer
    size += 4 * sizeof(int16_t);                                // 4 header shorts
    size += array_size(columns) * sizeof(int32_t);              // offsets table

    for (int c = 0; c < array_size(columns); c++)
    {
        for (int p = 0; p < array_size(columns[c].posts); p++)
        {
            size_t  post_len = 0;

            post_len += 2;                                      // two bytes for post header
            post_len++;                                         // dummy byte
            post_len += array_size(columns[c].posts[p].pixels); // pixels
            post_len++;                                         // dummy byte

            size += post_len;
        }

        size++;                                                 // room for 0xFF cap byte
    }

    output = I_Malloc(size);
    rover = output;

    // write header fields
    PUTSHORT(rover, width);
    PUTSHORT(rover, height);

    // This is written to afterwards
    PUTSHORT(rover, 0);
    PUTSHORT(rover, 0);

    // set starting position of column offsets table, and skip over it
    col_offsets = rover;

    rover += array_size(columns) * 4;

    for (int c = 0; c < array_size(columns); c++)
    {
        // write column offset to offset table
        uint32_t    offs = (uint32_t)(rover - output);

        PUTLONG(col_offsets, offs);

        // write column posts
        for (int p = 0; p < array_size(columns[c].posts); p++)
        {
            int     numpixels = array_size(columns[c].posts[p].pixels);
            byte    lastval = (numpixels ? columns[c].posts[p].pixels[0] : 0);

            // Write row offset
            PUTBYTE(rover, columns[c].posts[p].row_off);

            // Write number of pixels
            PUTBYTE(rover, numpixels);

            // Write pad byte
            PUTBYTE(rover, lastval);

            // Write pixels
            for (int a = 0; a < numpixels; a++)
            {
                lastval = columns[c].posts[p].pixels[a];
                PUTBYTE(rover, lastval);
            }

            // Write pad byte
            PUTBYTE(rover, lastval);

            array_free(columns[c].posts[p].pixels);
        }

        // Write 255 cap byte
        PUTBYTE(rover, 0xFF);

        array_free(columns[c].posts);
    }

    array_free(columns);

    // Done!
    return (patch_t *)output;
}
