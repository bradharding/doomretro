/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

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

#include "doomstat.h"
#include "m_random.h"
#include "r_local.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//

int     viewwidth;
int     scaledviewwidth;
int     viewheight;
int     viewheight2;
int     viewwindowx;
int     viewwindowy;
byte    *ylookup[SCREENHEIGHT];
byte    *ylookup2[SCREENHEIGHT];
int     fuzztable[SCREENWIDTH * SCREENHEIGHT];
int     fuzzclip;

byte redtoblue[] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

byte redtogreen[] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    118, 118, 119, 119, 120, 120, 121, 121, 122, 122, 123, 123, 124, 124, 125, 125,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

byte redtoyellow[] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43, 164, 164, 165, 165,
     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    230, 230, 231, 231, 160, 160, 161, 161, 162, 162, 163, 163, 164, 164, 165, 165,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

//
// R_DrawColumn
// Source is the top of the column to scale.
//
lighttable_t    *dc_colormap;
int             dc_x;
int             dc_yl;
int             dc_yh;
fixed_t         dc_iscale;
fixed_t         dc_texturemid;
fixed_t         dc_texheight;
fixed_t         dc_texturefrac;
boolean         dc_topsparkle;
boolean         dc_bottomsparkle;
fixed_t         dc_blood;

// first pixel in a column (possibly virtual)
byte            *dc_source;

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

void R_DrawColumn(void)
{
    int32_t             count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup[dc_yl] + dc_x + viewwindowx;
    fixed_t             frac = dc_texturefrac;
    const fixed_t       fracstep = dc_iscale;
    const byte          *source = dc_source;
    const lighttable_t  *colormap = dc_colormap;

    while (--count)
    {
        *dest = colormap[source[frac >> FRACBITS]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    *dest = colormap[source[frac >> FRACBITS]];
}

void R_DrawShadowColumn(void)
{
    int32_t     count = dc_yh - dc_yl + 1;
    byte        *dest = ylookup[dc_yl] + dc_x + viewwindowx;

    while (--count)
    {
        *dest = tinttab40[*dest];
        dest += SCREENWIDTH;
    }
    *dest = tinttab40[*dest];
}

void R_DrawBloodSplatColumn(void)
{
    int32_t             count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup[dc_yl] + dc_x + viewwindowx;
    const fixed_t       blood = dc_blood;

    while (--count)
    {
        *dest = tinttab66[*dest + blood];
        dest += SCREENWIDTH;
    }
    *dest = tinttab66[*dest + blood];
}

#define HEIGHTMASK      ((127 << FRACBITS) | 0xffff)

void R_DrawWallColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        if (dc_texheight == 128)
        {
            while (--count)
            {
                *dest = colormap[source[(frac & HEIGHTMASK) >> FRACBITS]];
                dest += SCREENWIDTH;
                frac += fracstep;
            }
            if (dc_bottomsparkle && !((frac >> FRACBITS) & 2))
                *dest = *(dest - SCREENWIDTH);
            else
                *dest = colormap[source[(frac & HEIGHTMASK) >> FRACBITS]];
        }
        else
        {
            uint32_t    heightmask = dc_texheight - 1;

            if (!(dc_texheight & heightmask))
            {
                fixed_t _heightmask = (heightmask << FRACBITS) | 0xffff;

                while ((count -= 2) >= 0)
                {
                    *dest = colormap[source[(frac & _heightmask) >> FRACBITS]];
                    dest += SCREENWIDTH;
                    frac += fracstep;
                    *dest = colormap[source[(frac & _heightmask) >> FRACBITS]];
                    dest += SCREENWIDTH;
                    frac += fracstep;
                }
                if (count & 1)
                {
                    if (dc_bottomsparkle && !((frac >> FRACBITS) & 1))
                        *dest = *(dest - SCREENWIDTH);
                    else
                        *dest = colormap[source[(frac & _heightmask) >> FRACBITS]];
                }
                else if (dc_bottomsparkle && !(((frac - fracstep) >> FRACBITS) & 1))
                    *(dest - SCREENWIDTH) = *(dest - (SCREENWIDTH << 1));
            }
            else
            {
                heightmask++;
                heightmask <<= FRACBITS;
                if (frac < 0)
                    while ((frac += heightmask) < 0);
                else
                    while (frac >= (int32_t)heightmask)
                        frac -= heightmask;

                while (--count)
                {
                    *dest = colormap[source[frac >> FRACBITS]];
                    dest += SCREENWIDTH;

                    if ((frac += fracstep) >= (int32_t)heightmask)
                        frac -= heightmask;
                }
                if (dc_bottomsparkle && !((frac >> FRACBITS) & 1))
                    *dest = *(dest - SCREENWIDTH);
                else
                    *dest = colormap[source[frac >> FRACBITS]];
            }
        }
    }

    if (dc_topsparkle)
    {
        dest = ylookup[dc_yl] + dc_x + viewwindowx;
        *dest = *(dest + SCREENWIDTH);
    }
}

void R_DrawFullbrightWallColumn(byte *colormask)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        if (dc_texheight == 128)
        {
            while (--count)
            {
                byte dot = source[(frac & HEIGHTMASK) >> FRACBITS];

                *dest = (colormask[dot] ? dot : colormap[dot]);
                dest += SCREENWIDTH;
                frac += fracstep;
            }
            if (dc_bottomsparkle && !((frac >> FRACBITS) & 2))
                *dest = *(dest - SCREENWIDTH);
            else
            {
                byte    dot = source[(frac & HEIGHTMASK) >> FRACBITS];

                *dest = (colormask[dot] ? dot : colormap[dot]);
            }
        }
        else
        {
            uint32_t    heightmask = dc_texheight - 1;

            if (!(dc_texheight & heightmask))
            {
                fixed_t _heightmask = (heightmask << FRACBITS) | 0xffff;

                while ((count -= 2) >= 0)
                {
                    byte        dot = source[(frac & _heightmask) >> FRACBITS];

                    *dest = (colormask[dot] ? dot : colormap[dot]);
                    dest += SCREENWIDTH;
                    frac += fracstep;
                    dot = source[(frac & _heightmask) >> FRACBITS];
                    *dest = (colormask[dot] ? dot : colormap[dot]);
                    dest += SCREENWIDTH;
                    frac += fracstep;
                }
                if (count & 1)
                {
                    if (dc_bottomsparkle && !((frac >> FRACBITS) & 1))
                        *dest = *(dest - SCREENWIDTH);
                    else
                    {
                        byte    dot = source[(frac & _heightmask) >> FRACBITS];

                        *dest = (colormask[dot] ? dot : colormap[dot]);
                    }
                }
                else if (dc_bottomsparkle && !(((frac - fracstep) >> FRACBITS) & 1))
                    *(dest - SCREENWIDTH) = *(dest - (SCREENWIDTH << 1));
            }
            else
            {
                heightmask++;
                heightmask <<= FRACBITS;
                if (frac < 0)
                    while ((frac += heightmask) < 0);
                else
                    while (frac >= (int32_t)heightmask)
                        frac -= heightmask;

                while (--count)
                {
                    byte        dot = source[frac >> FRACBITS];

                    *dest = (colormask[dot] ? dot : colormap[dot]);
                    dest += SCREENWIDTH;

                    if ((frac += fracstep) >= (int32_t)heightmask)
                        frac -= heightmask;
                }
                if (dc_bottomsparkle && !((frac >> FRACBITS) & 1))
                    *dest = *(dest - SCREENWIDTH);
                else
                {
                    byte        dot = source[frac >> FRACBITS];

                    *dest = (colormask[dot] ? dot : colormap[dot]);
                }
            }
        }
    }

    if (dc_topsparkle)
    {
        dest = ylookup[dc_yl] + dc_x + viewwindowx;
        *dest = *(dest + SCREENWIDTH);
    }
}

void R_DrawPlayerSpriteColumn(void)
{
    int32_t             count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup2[dc_yl] + dc_x + viewwindowx;
    fixed_t             frac = dc_texturefrac;
    const fixed_t       fracstep = dc_iscale;

    while (--count)
    {
        *dest = dc_source[frac >> FRACBITS];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    *dest = dc_source[frac >> FRACBITS];
}

void R_DrawSuperShotgunColumn(void)
{
    int32_t             count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup[dc_yl] + dc_x + viewwindowx;
    fixed_t             frac = dc_texturefrac;
    const fixed_t       fracstep = dc_iscale;
    byte                dot;
    const byte          *source = dc_source;
    const lighttable_t  *colormap = dc_colormap;

    while (--count)
    {
        dot = source[frac >> FRACBITS];
        if (dot != 71)
            *dest = colormap[dot];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    dot = source[frac >> FRACBITS];
    if (dot != 71)
        *dest = colormap[dot];
}

void R_DrawSkyColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;
        const fixed_t           heightmask = dc_texheight - 1;

        while (--count)
        {
            *dest = colormap[source[(frac >> FRACBITS) & heightmask]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = colormap[source[(frac >> FRACBITS) & heightmask]];
    }
}

void R_DrawFlippedSkyColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;
    fixed_t             i;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            i = frac >> FRACBITS;
            *dest = colormap[source[i > 127 ? 126 - (i & 127) : i]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        i = frac >> FRACBITS;
        *dest = colormap[source[i > 127 ? 126 - (i & 127) : i]];
    }
}

void R_DrawRedToBlueColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = colormap[redtoblue[source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = colormap[redtoblue[source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentRedToBlue33Column(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = tinttab33[(*dest << 8) + colormap[redtoblue[source[frac >> FRACBITS]]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = tinttab33[(*dest << 8) + colormap[redtoblue[source[frac >> FRACBITS]]]];
    }
}

void R_DrawRedToGreenColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = colormap[redtogreen[source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = colormap[redtogreen[source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentRedToGreen33Column(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = tinttab33[(*dest << 8) + colormap[redtogreen[source[frac >> FRACBITS]]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = tinttab33[(*dest << 8) + colormap[redtogreen[source[frac >> FRACBITS]]]];
    }
}

void R_DrawTranslucentColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = tinttab[(*dest << 8) + colormap[source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = tinttab[(*dest << 8) + colormap[source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucent50Column(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        byte                    dot;
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            dot = source[frac >> FRACBITS];
            *dest = tinttab50[(*dest << 8) + colormap[dot]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        dot = source[frac >> FRACBITS];
        *dest = tinttab50[(*dest << 8) + colormap[dot]];
    }
}

extern boolean megasphere;

void R_DrawTranslucent33Column(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        byte                    dot;
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            dot = source[frac >> FRACBITS];
            if (megasphere && (dot == 9 || dot == 159))
                dot = 142;
            *dest = tinttab33[(*dest << 8) + colormap[dot]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        dot = source[frac >> FRACBITS];
        if (megasphere && (dot == 9 || dot == 159))
            dot = 142;
        *dest = tinttab33[(*dest << 8) + colormap[dot]];
    }
}

void R_DrawTranslucentRedColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = tinttabred[(*dest << 8) + colormap[source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = tinttabred[(*dest << 8) + colormap[source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentRedWhiteColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = colormap[tinttabredwhite[(*dest << 8) + source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = colormap[tinttabredwhite[(*dest << 8) + source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentRedWhite50Column(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = colormap[tinttabredwhite50[(*dest << 8) + source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = colormap[tinttabredwhite50[(*dest << 8) + source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentGreenColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = tinttabgreen[(*dest << 8) + colormap[source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = tinttabgreen[(*dest << 8) + colormap[source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentBlueColumn(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = tinttabblue[(*dest << 8) + colormap[source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = tinttabblue[(*dest << 8) + colormap[source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentRed50Column(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = colormap[tinttabred50[(*dest << 8) + source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = colormap[tinttabred50[(*dest << 8) + source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentGreen50Column(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = colormap[tinttabgreen50[(*dest << 8) + source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = colormap[tinttabgreen50[(*dest << 8) + source[frac >> FRACBITS]]];
    }
}

void R_DrawTranslucentBlue50Column(void)
{
    int32_t             count = dc_yh - dc_yl;
    byte                *dest;
    fixed_t             frac;
    const fixed_t       fracstep = dc_iscale;

    if (count++ < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;

    frac = dc_texturefrac;

    {
        const byte              *source = dc_source;
        const lighttable_t      *colormap = dc_colormap;

        while (--count)
        {
            *dest = colormap[tinttabblue50[(*dest << 8) + source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        *dest = colormap[tinttabblue50[(*dest << 8) + source[frac >> FRACBITS]]];
    }
}

//
// Spectre/Invisibility.
//
extern int fuzzpos;

int fuzzrange[3] = { -SCREENWIDTH, 0, SCREENWIDTH };

#define FUZZ(a, b) fuzzrange[M_RandomInt(a + 1, b + 1)]

void R_DrawFuzzColumn(void)
{
    byte        *dest;
    int         count = dc_yh - dc_yl;

    if (count < 0)
        return;

    dest = ylookup[dc_yl] + dc_x + viewwindowx;
    if (menuactive || paused)
    {
        if (count)
        {
            // top
            if (!dc_yl)
                *dest = colormaps[6 * 256 + dest[fuzztable[fuzzpos]]];
            else if (fuzztable[fuzzpos])
                *dest = colormaps[12 * 256 + dest[fuzztable[fuzzpos]]];
            fuzzpos++;
            dest += SCREENWIDTH;

            while (--count)
            {
                // middle
                *dest = colormaps[6 * 256 + dest[fuzztable[fuzzpos++]]];
                dest += SCREENWIDTH;
            }
        }

        // bottom
        if (dc_yh == viewheight - 1)
            *dest = colormaps[5 * 256 + dest[fuzztable[fuzzpos]]];
        else if (fuzzclip == -1 && fuzztable[fuzzpos])
            *dest = colormaps[14 * 256 + dest[fuzztable[fuzzpos]]];
    }
    else
    {
        if (count)
        {
            // top
            fuzztable[fuzzpos] = (!dc_yl ? FUZZ(0, 1) : FUZZ(-1, count > 0));
            if (!dc_yl)
                *dest = colormaps[6 * 256 + dest[fuzztable[fuzzpos]]];
            else if (M_RandomInt(1, 100) < 25)
                *dest = colormaps[12 * 256 + dest[fuzztable[fuzzpos]]];
            fuzzpos++;
            dest += SCREENWIDTH;

            while (--count)
            {
                // middle
                fuzztable[fuzzpos] = FUZZ(-1, 1);
                *dest = colormaps[6 * 256 + dest[fuzztable[fuzzpos++]]];
                dest += SCREENWIDTH;
            }
        }

        // bottom
        fuzztable[fuzzpos] = FUZZ(-1, 0);
        if (dc_yh == viewheight - 1)
            *dest = colormaps[5 * 256 + dest[fuzztable[fuzzpos]]];
        else if (fuzzclip == -1 && M_RandomInt(1, 100) < 25)
            *dest = colormaps[14 * 256 + dest[fuzztable[fuzzpos]]];
    }
}

void R_DrawFuzzColumns(void)
{
    int         x, y;
    int         w = viewwindowx + viewwidth;
    int         h = (viewwindowy + viewheight) * SCREENWIDTH;
    byte        *src = screens[1];
    byte        *dest = screens[0];

    for (x = viewwindowx; x < w; x++)
    {
        for (y = viewwindowy * SCREENWIDTH; y < h; y += SCREENWIDTH)
        {
            src = screens[1] + y + x;
            dest = screens[0] + y + x;
            if (menuactive || paused)
            {
                if (*src != 251)
                    *dest = colormaps[6 * 256 + dest[fuzztable[x + y]]];
            }
            else
            {
                if (*src != 251)
                {
                    if (y == 0 || *(src - SCREENWIDTH) == 251) // top
                    {
                        fuzztable[x + y] = FUZZ(-1, 1);
                        if (M_RandomInt(1, 100) < 25)
                            *dest = colormaps[12 * 256 + dest[fuzztable[x + y]]];
                    }
                    else if (y == h - SCREENWIDTH) // bottom of view
                    {
                        fuzztable[x + y] = FUZZ(-1, 0);
                        *dest = colormaps[5 * 256 + dest[fuzztable[x + y]]];
                    }
                    else if (*(src + SCREENWIDTH) == 251) // bottom of post
                    {
                        fuzztable[x + y] = FUZZ(-1, 1);
                        if (M_RandomInt(1, 100) < 25)
                            *dest = colormaps[12 * 256 + dest[fuzztable[x + y]]];
                    }
                    else // middle
                    {
                        fuzztable[x + y] = FUZZ(-1, 1);
                        if (*(src - 1) == 251 || *(src + 1) == 251)
                        {
                            if (M_RandomInt(1, 100) < 25)
                                *dest = colormaps[12 * 256 + dest[fuzztable[x + y]]];
                        }
                        else
                            *dest = colormaps[6 * 256 + dest[fuzztable[x + y]]];
                    }
                }
            }
        }
    }
}

//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//
int          ds_y;
int          ds_x1;
int          ds_x2;

lighttable_t *ds_colormap;

fixed_t      ds_xfrac;
fixed_t      ds_yfrac;
fixed_t      ds_xstep;
fixed_t      ds_ystep;

// start of a 64*64 tile image
byte         *ds_source;

//
// Draws the actual span.
//
void R_DrawSpan(void)
{
    unsigned int        count = ds_x2 - ds_x1 + 1;
    byte                *dest = ylookup[ds_y] + ds_x1 + viewwindowx;
    fixed_t             xfrac = ds_xfrac;
    fixed_t             yfrac = ds_yfrac;
    const fixed_t       xstep = ds_xstep;
    const fixed_t       ystep = ds_ystep;
    const byte          *source = ds_source;
    const lighttable_t  *colormap = ds_colormap;

    while (--count)
    {
        *dest++ = colormap[source[((yfrac >> 10) & 4032) | ((xfrac >> 16) & 63)]];
        xfrac += xstep;
        yfrac += ystep;
    }
    *dest = colormap[source[((yfrac >> 10) & 4032) | ((xfrac >> 16) & 63)]];
}

//
// R_InitBuffer
// Creates lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//
void R_InitBuffer(int width, int height)
{
    int i;

    // Handle resize,
    //  e.g. smaller view windows
    //  with border and/or status bar.
    viewwindowx = (SCREENWIDTH - width) >> 1;

    // Same with base row offset.
    if (width == SCREENWIDTH)
        viewwindowy = 0;
    else
        viewwindowy = (SCREENHEIGHT - SBARHEIGHT - height) >> 1;

    // Preclaculate all row offsets.
    for (i = 0; i < height; i++)
    {
        ylookup[i] = screens[0] + (i + viewwindowy) * SCREENWIDTH;
        ylookup2[i] = screens[1] + (i + viewwindowy) * SCREENWIDTH;
    }
}

//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen(void)
{
    byte    *src;
    byte    *dest;
    int     x;
    int     y;
    int     i;
    int     width;
    int     height;
    int     windowx;
    int     windowy;
    patch_t *patch;

    if (scaledviewwidth == SCREENWIDTH)
        return;

    src = W_CacheLumpName((gamemode == commercial ? "GRNROCK" : "FLOOR7_2"), PU_CACHE);
    dest = screens[1];

    for (y = 0; y < SCREENHEIGHT; y += 2)
    {
        for (x = 0; x < SCREENWIDTH / 32; x += 2)
        {
            for (i = 0; i < 64; i++)
            {
                int j = i * 2;
                byte dot = *(src + (((y / 2) & 63) << 6) + i);

                if (y * SCREENWIDTH + x + j < SCREENWIDTH * (SCREENHEIGHT - 1))
                    *(dest + j) = dot;
                j++;
                if (y * SCREENWIDTH + x + j < SCREENWIDTH * (SCREENHEIGHT - 1))
                    *(dest + j) = dot;
                j += SCREENWIDTH;
                if (y * SCREENWIDTH + x + j < SCREENWIDTH * (SCREENHEIGHT - 1))
                    *(dest + j) = dot;
                j--;
                if (y * SCREENWIDTH + x + j < SCREENWIDTH * (SCREENHEIGHT - 1))
                    *(dest + j) = dot;
            }
            dest += 128;
        }
    }

    // Draw screen and bezel; this is done to a separate screen buffer.
    width = scaledviewwidth / 2;
    height = viewheight / 2;
    windowx = viewwindowx / 2;
    windowy = viewwindowy / 2;

    patch = W_CacheLumpName("brdr_t", PU_CACHE);
    for (x = 0; x < width; x += 8)
        V_DrawPatch(windowx + x, windowy - 8, 1, patch);

    patch = W_CacheLumpName("brdr_b", PU_CACHE);
    for (x = 0; x < width; x += 8)
        V_DrawPatch (windowx + x, windowy + height, 1, patch);

    patch = W_CacheLumpName("brdr_l", PU_CACHE);
    for (y = 0; y < height; y += 8)
        V_DrawPatch(windowx - 8, windowy + y, 1, patch);

    patch = W_CacheLumpName("brdr_r", PU_CACHE);
    for (y = 0; y < height; y += 8)
        V_DrawPatch(windowx + width, windowy + y, 1, patch);

    // Draw beveled edge.
    V_DrawPatch(windowx - 8, windowy - 8, 1, W_CacheLumpName("brdr_tl", PU_CACHE));
    V_DrawPatch(windowx + width, windowy - 8, 1, W_CacheLumpName("brdr_tr", PU_CACHE));
    V_DrawPatch(windowx - 8, windowy + height, 1, W_CacheLumpName("brdr_bl", PU_CACHE));
    V_DrawPatch(windowx + width, windowy + height, 1, W_CacheLumpName("brdr_br", PU_CACHE));
}

//
// Copy a screen buffer.
//
void R_VideoErase(unsigned int ofs, int count)
{
    // LFB copy.
    // This might not be a good idea if memcpy
    //  is not optiomal, e.g. byte by byte on
    //  a 32bit CPU, as GNU GCC/Linux libc did
    //  at one point.
    memcpy(screens[0] + ofs, screens[1] + ofs, count);
}

//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
void R_DrawViewBorder(void)
{
    int top;
    int side;
    int ofs;
    int i;

    if (scaledviewwidth == SCREENWIDTH)
        return;

    top = (SCREENHEIGHT - SBARHEIGHT - viewheight) / 2;
    side = (SCREENWIDTH - scaledviewwidth) / 2;

    // copy top and one line of left side
    R_VideoErase(0, top * SCREENWIDTH + side);

    // copy one line of right side and bottom
    ofs = (viewheight + top) * SCREENWIDTH - side;
    R_VideoErase(ofs, top * SCREENWIDTH + side);

    // copy sides using wraparound
    ofs = top * SCREENWIDTH + SCREENWIDTH - side;
    side <<= 1;

    for (i = 1; i < viewheight; i++)
    {
        R_VideoErase(ofs, side);
        ofs += SCREENWIDTH;
    }
}
