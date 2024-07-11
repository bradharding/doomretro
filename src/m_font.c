//
// SLADE - It's a Doom Editor
// Copyright(C) 2008 - 2019 Simon Judd
// Copyright(C) 2024 Roman Fomin
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// DESCRIPTION:
//     Load and draw ZDoom FON2 fonts

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "i_colors.h"
#include "i_video.h"
#include "i_swap.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

typedef struct
{
    uint16_t width;
    patch_t *patch;
} fon2_char_t;

typedef struct
{
    byte magic[4];
    uint16_t charheight;
    byte firstc;
    byte lastc;
    byte constantw;
    byte shading;
    byte palsize;
    byte kerning; // flag field, but with only one flag
} fon2_header_t;

static fon2_char_t *chars = NULL;
static int numchars;
static int height;
static int firstc;
static int kerning;

#define FON2_SPACE 12

bool MN_LoadFon2(const byte *gfx_data, int size)
{
    if (size < (int)sizeof(fon2_header_t))
    {
        return false;
    }

    const fon2_header_t *header = (fon2_header_t *)gfx_data;
    if (memcmp(header->magic, "FON2", 4))
    {
        return false;
    }

    height = SHORT(header->charheight);
    if (height == 0)
    {
        return false;
    }

    const byte *p = gfx_data + sizeof(fon2_header_t);

    if (header->kerning)
    {
        kerning = SHORT(*(int16_t *)p);
        p += 2;
    }

    firstc = header->firstc;
    numchars = header->lastc - header->firstc + 1;
    chars = malloc(numchars * sizeof(*chars));

    for (int i = 0; i < numchars; ++i)
    {
        chars[i].width = SHORT(*(uint16_t *)p);
        // The width information is enumerated for each character only if they
        // are not constant width. Regardless, move the read pointer away after
        // the last.
        if (!(header->constantw) || (i == numchars - 1))
        {
            p += 2;
        }
    }

    // Build translation table for palette.
    byte *playpal = W_CacheLumpName("PLAYPAL");
    byte *translate = malloc(header->palsize + 1);
    for (int i = 0; i < header->palsize + 1; ++i)
    {
        int r = *p++;
        int g = *p++;
        int b = *p++;
        translate[i] = FindNearestColor(playpal, r, g, b);
    }

    // 0 is transparent, last is border color
    byte color_key = translate[0];

    // The picture data follows, using the same RLE as FON1 and IMGZ.
    for (int i = 0; i < numchars; ++i)
    {
        // A big font is not necessarily continuous; several characters
        // may be skipped; they are given a width of 0.
        if (!chars[i].width)
        {
            continue;
        }

        int numpixels = chars[i].width * height;
        byte *data = malloc(numpixels);
        byte *d = data;
        byte code = 0;
        int length = 0;

        while (numpixels)
        {
            code = *p++;
            if (code < 0x80)
            {
                length = code + 1;
                for (int k = 0; k < length; ++k)
                {
                    d[k] = translate[p[k]];
                }
                d += length;
                p += length;
                numpixels -= length;
            }
            else if (code > 0x80)
            {
                length = 0x101 - code;
                code = *p++;
                memset(d, translate[code], length);
                d += length;
                numpixels -= length;
            }
        }

        chars[i].patch = V_LinearToTransPatch(data, chars[i].width, height,
                                              color_key);
        free(data);
    }

    free(translate);
    return true;
}

bool MN_DrawFon2String(int x, int y, const char *str)
{
    if (!numchars)
    {
        return false;
    }

    int c, cx;

    cx = x;

    while (*str)
    {
        c = *str++;

        c = toupper(c) - firstc;
        if (c < 0 || c >= numchars)
        {
            cx += FON2_SPACE;
            continue;
        }

        if (chars[c].width)
        {
            V_DrawPatch(cx, y, 0, chars[c].patch);
            cx += chars[c].width + kerning;
        }
        else
        {
            cx += FON2_SPACE + kerning;
        }
    }

    return true;
}

int MN_GetFon2PixelWidth(const char *str)
{
    if (!numchars)
    {
        return 0;
    }

    int len = 0;
    int c;

    while (*str)
    {
        c = *str++;

        c = toupper(c) - firstc;
        if (c < 0 || c > numchars)
        {
            len += FON2_SPACE; // space
            continue;
        }

        len += chars[c].width + kerning;
    }
    len -= kerning;
    return len;
}
