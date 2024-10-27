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

#include <ctype.h>

#include "doomtype.h"
#include "i_colors.h"
#include "i_video.h"
#include "i_swap.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#define FON2_SPACE  12

typedef struct
{
    uint16_t        width;
    patch_t         *patch;
} fon2_char_t;

typedef struct
{
    byte            magic[4];
    uint16_t        charheight;
    byte            firstc;
    byte            lastc;
    byte            constantw;
    byte            shading;
    byte            palsize;
    byte            kerning;    // flag field, but with only one flag
} fon2_header_t;

static fon2_char_t  *chars;
static int          numchars;
static int          height;
static int          firstc;
static bool         upper;
static int          kerning;

bool M_LoadFON2(byte *gfx_data, int size)
{
    fon2_header_t   *header;
    byte            *p;
    byte            *playpal;
    byte            *translate;
    byte            color_key;

    if (size < (int)sizeof(fon2_header_t))
        return false;

    header = (fon2_header_t *)gfx_data;

    if (memcmp(header->magic, "FON2", 4))
        return false;

    if (!(height = SHORT(header->charheight)))
        return false;

    p = gfx_data + sizeof(fon2_header_t);

    if (header->kerning)
    {
        kerning = SHORT(*(int16_t *)p);
        p += 2;
    }

    firstc = header->firstc;

    if (header->lastc < 'z')
        upper = true;

    numchars = header->lastc - header->firstc + 1;
    chars = malloc(numchars * sizeof(*chars));

    for (int i = 0; i < numchars; i++)
    {
        chars[i].width = SHORT(*(uint16_t *)p);

        // The width information is enumerated for each character only if they
        // are not constant width. Regardless, move the read pointer away after
        // the last.
        if (!header->constantw || i == numchars - 1)
            p += 2;
    }

    // Build translation table for palette.
    playpal = W_CacheLumpName("PLAYPAL");

    if (!(translate = malloc(header->palsize + 1)))
        return false;

    for (int i = 0; i <= header->palsize; i++)
    {
        const byte  r = *p++;
        const byte  g = *p++;
        const byte  b = *p++;

        translate[i] = FindNearestColor(playpal, r, g, b);
    }

    // 0 is transparent, last is border color
    color_key = translate[0];

    // The picture data follows, using the same RLE as FON1 and IMGZ.
    for (int i = 0; i < numchars; i++)
        // A big font is not necessarily continuous; several characters
        // may be skipped; they are given a width of 0.
        if (chars[i].width)
        {
            int     numpixels = chars[i].width * height;
            byte    *data = malloc(numpixels);

            if (data)
            {
                byte    *d = data;

                while (numpixels)
                {
                    byte    code = *p++;
                    int     length;

                    if (code < 0x80)
                    {
                        length = code + 1;

                        for (int j = 0; j < length; j++)
                            d[j] = translate[p[j]];

                        d += length;
                        p += length;
                        numpixels -= length;
                    }
                    else if (code > 0x80)
                    {
                        length = 0x0101 - code;
                        code = *p++;
                        memset(d, translate[code], length);
                        d += length;
                        numpixels -= length;
                    }
                }

                chars[i].patch = V_LinearToTransPatch(data, chars[i].width, height, color_key);
                free(data);
            }
        }

    free(translate);
    return true;
}

bool M_DrawFON2String(int x, int y, const char *str, bool highlight)
{
    int cx = x;

    if (!numchars)
        return false;

    while (*str)
    {
        int c = *str++;

        c = (upper ? toupper(c) : c) - firstc;

        if (c < 0 || c >= numchars || !chars[c].width)
            cx += FON2_SPACE;
        else
        {
            V_DrawBigFontPatch(cx, y, chars[c].patch, highlight, SCREENWIDTH);
            cx += chars[c].width + kerning;
        }
    }

    return true;
}

int M_GetFON2PixelWidth(const char *str)
{
    int width = 0;

    if (!numchars)
        return 0;

    while (*str)
    {
        int c = *str++;

        c = (upper ? toupper(c) : c) - firstc;

        if (c < 0 || c >= numchars || !chars[c].width)
            width += FON2_SPACE;
        else
            width += chars[c].width + kerning;
    }

    return (width - kerning);
}
