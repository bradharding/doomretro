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

#include <Shlobj.h>
#include "doomstat.h"
#include "d_main.h"
#include "i_swap.h"
#include "m_misc.h"
#include "m_random.h"
#include "SDL.h"
#include "v_video.h"
#include "z_zone.h"

// Each screen is [SCREENWIDTH * SCREENHEIGHT];
byte            *screens[5];

byte            gammatable[GAMMALEVELS][256];

double gammalevel[GAMMALEVELS] =
{
    0.5, 0.625, 0.75, 0.875, 1.0, 1.25, 1.5, 1.75, 2.0
};

// Gamma correction level to use
int             usegamma = USEGAMMA_DEFAULT;

//
// V_SetRes
//
static void V_SetRes(void)
{
    DX  = (SCREENWIDTH << 16) / ORIGINALWIDTH;
    DXI = (ORIGINALWIDTH << 16) / SCREENWIDTH;
    DY  = (SCREENHEIGHT << 16) / ORIGINALHEIGHT;
    DYI = (ORIGINALHEIGHT << 16) / SCREENHEIGHT;
}

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
    byte *dest = screens[scrn] + y * SCREENWIDTH + x;
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

    int         stretchx, stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = patch->width << 16; col < w; x++, col += DXI, desttop++)
    {
        column=(column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

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

void V_DrawPatchWithShadow(int x, int y, int scrn, patch_t *patch, boolean flag)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    int         stretchx, stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = patch->width << 16; col < w; x++, col += DXI, desttop++)
    {
        column=(column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + ((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;
            while (count--)
            {
                byte *shadow;

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

void V_DrawTranslucentRedPatch(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    int         stretchx, stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = patch->width << 16; col < w; x++, col += DXI, desttop++)
    {
        column=(column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

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
// V_DrawPatchInDirectFlipped
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

    int         stretchx, stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = patch->width << 16; col < w; x++, col += DXI, desttop++)
    {
        column=(column_t *)((byte *)patch + LONG(patch->columnofs[patch->width - 1 - (col >> 16)]));

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

extern int fuzztable[SCREENWIDTH * SCREENHEIGHT];

const int _fuzzrange[3] = { -SCREENWIDTH, 0, SCREENWIDTH };

#define _FUZZ(a, b) _fuzzrange[M_RandomInt(a + 1, b + 1)]

extern boolean menuactive;
extern boolean paused;

void V_DrawFuzzPatch(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    int       stretchx, stretchy;
    int       _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = patch->width << 16; col < w; col += DXI, desttop++)
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

    int         stretchx, stretchy;
    int         _fuzzpos = 0;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = patch->width << 16; col < w; col += DXI, desttop++)
    {
        column=(column_t *)((byte *)patch + LONG(patch->columnofs[patch->width - 1 - (col >> 16)]));

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
    1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 000-031
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 032-063
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 064-095
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 096-127
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // 128-159
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 160-191
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 192-223
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1  // 224-255
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

    int         stretchx, stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = patch->width << 16; col < w; x++, col += DXI, desttop++)
    {
        column=(column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

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
                {
                    byte *shadow;

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
    V_DrawPatch((ORIGINALWIDTH - patch->width) / 2, y, scrn, patch);
}

void V_DrawTranslucentNoGreenPatch(int x, int y, int scrn, patch_t *patch)
{
    int         count;
    int         col;
    column_t    *column;
    byte        *desttop;
    byte        *dest;
    byte        *source;
    int         w;

    int         stretchx, stretchy;
    int         srccol;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    stretchx = (x * DX) >> 16;
    stretchy = (y * DY) >> 16;

    col = 0;
    desttop = screens[scrn] + stretchy * SCREENWIDTH + stretchx;

    for (w = patch->width << 16; col < w; x++, col += DXI, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop+((column->topdelta * DY) >> 16) * SCREENWIDTH;
            count = (column->length * DY) >> 16;
            srccol = 0;

            while (count--)
            {
                byte src = source[srccol >> 16];

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

//
// V_GetBlock
// Gets a linear block of pixels from the view buffer.
//
void V_GetBlock(int x, int y, int scrn, int width, int height, byte *dest)
{
    byte        *src;

    src = screens[scrn] + y * SCREENWIDTH + x;

    while (height--)
    {
        memcpy(dest, src, width);
        src += SCREENWIDTH;
        dest += width;
    }
}

void V_DrawPixel(int x, int y, int screen, int color, boolean shadow)
{
    byte *dest = &screens[screen][y * 2 * SCREENWIDTH + x * 2];

    if (color == 251 || color == -5)
    {
        if (shadow)
        {
            *dest = tinttab50[*dest];
            dest++;
            *dest = tinttab50[*dest];
            dest += SCREENWIDTH;
            *dest = tinttab50[*dest];
            dest--;
            *dest = tinttab50[*dest];
        }
    }
    else if (color && color != 32)
    {
        *dest = color;
        dest++;
        *dest = color;
        dest += SCREENWIDTH;
        *dest = color;
        dest--;
        *dest = color;
    }
}

//
// V_Init
//
void V_Init(void)
{
    int         i;
    byte        *base;

    base = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT * 4, PU_STATIC, NULL);

    for (i = 0; i < 4; i++)
        screens[i] = base + i * SCREENWIDTH * SCREENHEIGHT;

    V_SetRes();
}

extern char maptitle[128];

extern boolean nerve;

char lbmname[256];
char lbmpath[256];

boolean V_ScreenShot(void)
{
    char mapname[128];
    int count = 0;
    char folder[MAX_PATH];

    HRESULT hr = SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, folder);

    if (usergame)
        strcpy(mapname, maptitle);
    else
        strcpy(mapname, "Untitled");

    do
    {
      if (!count)
          sprintf(lbmname, "%s.bmp", mapname);
      else
          sprintf(lbmname, "%s (%i).bmp", mapname, count);
      count++;
      sprintf(lbmpath, "%s\\DOOM RETRO", folder);
      M_MakeDirectory(lbmpath);
      sprintf(lbmpath, "%s\\%s", lbmpath, lbmname);
    }
    while (M_FileExists(lbmpath));

    return (!SDL_SaveBMP(SDL_GetVideoSurface(), lbmpath));
}