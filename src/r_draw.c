/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "i_colors.h"
#include "st_stuff.h"
#include "v_video.h"

#define NOFUZZ  251

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about coordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width * height * depth / 8.
//

int             viewwidth;
int             viewheight;
int             viewwindowx;
int             viewwindowy;

int             fuzzrange[3];
int             fuzz1pos;
int             fuzz2pos;
int             fuzz1table[MAXSCREENAREA];
int             fuzz2table[MAXSCREENAREA];

static byte     *ylookup0[MAXHEIGHT];
static byte     *ylookup1[MAXHEIGHT];

lighttable_t    *dc_colormap[2];
lighttable_t    *dc_nextcolormap[2];
lighttable_t    *dc_sectorcolormap;
int             dc_x;
int             dc_yl;
int             dc_yh;
int             dc_z;
fixed_t         dc_iscale;
fixed_t         dc_texturemid;
fixed_t         dc_texheight;
fixed_t         dc_texturefrac;
byte            dc_solidbloodcolor;
byte            *dc_bloodcolor;
byte            *dc_brightmap;
int             dc_floorclip;
int             dc_ceilingclip;
int             dc_numposts;
byte            dc_black;
byte            *dc_black33;
byte            *dc_black40;
byte            *dc_source;
byte            *dc_translation;

#define DITHERSIZE  4

static const byte ditherlowmatrix[DITHERSIZE * 2][DITHERSIZE * 2] =
{
    {   0,   0, 224, 224,  48,  48, 208, 208 },
    {   0,   0, 224, 224,  48,  48, 208, 208 },
    { 176, 176,  80,  80, 128, 128,  96,  96 },
    { 176, 176,  80,  80, 128, 128,  96,  96 },
    { 192, 192,  32,  32, 240, 240,  16,  16 },
    { 192, 192,  32,  32, 240, 240,  16,  16 },
    { 112, 112, 144, 144,  64,  64, 160, 160 },
    { 112, 112, 144, 144,  64,  64, 160, 160 }
};

#define ditherlow(x, y, z)  (ditherlowmatrix[((y) & (DITHERSIZE * 2 - 1))][((x) & (DITHERSIZE * 2 - 1))] < (z))

static const byte dithermatrix[DITHERSIZE][DITHERSIZE] =
{
    {   0, 224,  48, 208 },
    { 176,  80, 128,  96 },
    { 192,  32, 240,  16 },
    { 112, 144,  64, 160 }
};

#define dither(x, y, z)     (dithermatrix[((y) & (DITHERSIZE - 1))][((x) & (DITHERSIZE - 1))] < (z))

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z-depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

void R_DrawColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawBrightmapColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;
    fixed_t frac = dc_texturefrac;
    byte    dot;

    while (--count)
    {
        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[dc_colormap[dc_brightmap[dot]][dot]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    dot = dc_source[frac >> FRACBITS];
    *dest = dc_sectorcolormap[dc_colormap[dc_brightmap[dot]][dot]];
}

void R_DrawDitherLowColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl++, dc_z)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl, dc_z)][dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl++, dc_z)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl, dc_z)][dc_source[frac >> FRACBITS]]];
}

void R_DrawBrightmapDitherLowColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] }, { fullcolormap, fullcolormap } };
    byte                dot;

    while (--count)
    {
        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][ditherlow(dc_x, dc_yl++, dc_z)][dot]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    dot = dc_source[frac >> FRACBITS];
    *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][ditherlow(dc_x, dc_yl++, dc_z)][dot]];
}

void R_DrawBrightmapDitherColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] }, { fullcolormap, fullcolormap } };
    byte                dot;

    while (--count)
    {
        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][dither(dc_x, dc_yl++, dc_z)][dot]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    dot = dc_source[frac >> FRACBITS];
    *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][dither(dc_x, dc_yl++, dc_z)][dot]];
}

void R_DrawCorrectedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[nearestcolors[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[nearestcolors[dc_source[frac >> FRACBITS]]]];
}

void R_DrawCorrectedDitherLowColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl++, dc_z)][nearestcolors[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl, dc_z)][nearestcolors[dc_source[frac >> FRACBITS]]]];
}

void R_DrawCorrectedDitherColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl++, dc_z)][nearestcolors[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl, dc_z)][nearestcolors[dc_source[frac >> FRACBITS]]]];
}

void R_DrawColorColumn(void)
{
    int         count = dc_yh - dc_yl + 1;
    byte        *dest = ylookup0[dc_yl] + dc_x;
    const byte  color = dc_sectorcolormap[dc_colormap[0][NOTEXTURECOLOR]];

    while (--count)
    {
        *dest = color;
        dest += SCREENWIDTH;
    }

    *dest = color;
}

void R_DrawColorDitherLowColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl++, dc_z)][NOTEXTURECOLOR]];
        dest += SCREENWIDTH;
    }

    *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl, dc_z)][NOTEXTURECOLOR]];
}

void R_DrawColorDitherColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl++, dc_z)][NOTEXTURECOLOR]];
        dest += SCREENWIDTH;
    }

    *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl, dc_z)][NOTEXTURECOLOR]];
}

void R_DrawShadowColumn(void)
{
    int     count = dc_yh - dc_yl;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    if (count)
    {
        *dest = *(*dest + dc_black33);
        dest += SCREENWIDTH;

        while (--count)
        {
            *dest = *(*dest + dc_black40);
            dest += SCREENWIDTH;
        }

        *dest = *(*dest + (dc_yh == dc_floorclip ? dc_black40 : dc_black33));
    }
    else
        *dest = *(*dest + dc_black33);
}

void R_DrawFuzzyShadowColumn(void)
{
    byte    *dest;
    int     count;

    if (dc_x & 1)
        return;

    dest = ylookup0[dc_yl] + dc_x;

    if ((count = dc_yh - dc_yl))
    {
        *dest = *(*dest + dc_black33);
        *(dest + 1) = *(*(dest + 1) + dc_black33);
        dest += SCREENWIDTH;

        while (--count)
        {
            *dest = *(*dest + dc_black33);
            *(dest + 1) = *(*(dest + 1) + dc_black33);
            dest += SCREENWIDTH;
        }

        *dest = *(*dest + dc_black33);
        *(dest + 1) = *(*(dest + 1) + dc_black33);
    }
    else
    {
        *dest = *(*dest + dc_black33);
        *(dest + 1) = *(*(dest + 1) + dc_black33);
    }
}

void R_DrawSolidShadowColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    while (--count)
    {
        *dest = dc_black;
        dest += SCREENWIDTH;
    }

    *dest = dc_black;
}

void R_DrawBloodSplatColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    while (--count)
    {
        *dest = dc_sectorcolormap[*(*dest + dc_bloodcolor)];
        dest += SCREENWIDTH;
    }

    *dest = dc_sectorcolormap[*(*dest + dc_bloodcolor)];
}

void R_DrawSolidBloodSplatColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    while (--count)
    {
        *dest = dc_sectorcolormap[dc_solidbloodcolor];
        dest += SCREENWIDTH;
    }

    *dest = dc_sectorcolormap[dc_solidbloodcolor];
}

void R_DrawWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap = dc_colormap[0];
    fixed_t             heightmask = dc_texheight - 1;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            *dest = dc_sectorcolormap[colormap[dc_source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        *dest = dc_sectorcolormap[colormap[dc_source[frac >> FRACBITS]]];
    }
    else
    {
        while (--count)
        {
            *dest = dc_sectorcolormap[colormap[dc_source[((frac >> FRACBITS) & heightmask)]]];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        *dest = dc_sectorcolormap[colormap[dc_source[((frac >> FRACBITS) & heightmask)]]];
    }
}

void R_DrawDitherLowWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    fixed_t             heightmask = dc_texheight - 1;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl++, dc_z)][dc_source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl, dc_z)][dc_source[frac >> FRACBITS]]];
    }
    else
    {
        while (--count)
        {
            *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl++, dc_z)][dc_source[((frac >> FRACBITS) & heightmask)]]];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl, dc_z)][dc_source[((frac >> FRACBITS) & heightmask)]]];
    }
}

void R_DrawDitherWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    fixed_t             heightmask = dc_texheight - 1;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl++, dc_z)][dc_source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl, dc_z)][dc_source[frac >> FRACBITS]]];
    }
    else
    {
        while (--count)
        {
            *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl++, dc_z)][dc_source[((frac >> FRACBITS) & heightmask)]]];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl, dc_z)][dc_source[((frac >> FRACBITS) & heightmask)]]];
    }
}

void R_DrawBrightmapWallColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;
    fixed_t frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    fixed_t heightmask = dc_texheight - 1;
    byte    dot;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            dot = dc_source[frac >> FRACBITS];
            *dest = dc_sectorcolormap[dc_colormap[dc_brightmap[dot]][dot]];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[dc_colormap[dc_brightmap[dot]][dot]];
    }
    else
    {
        while (--count)
        {
            dot = dc_source[((frac >> FRACBITS) & heightmask)];
            *dest = dc_sectorcolormap[dc_colormap[dc_brightmap[dot]][dot]];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        dot = dc_source[((frac >> FRACBITS) & heightmask)];
        *dest = dc_sectorcolormap[dc_colormap[dc_brightmap[dot]][dot]];
    }
}

void R_DrawBrightmapDitherLowWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] }, { fullcolormap, fullcolormap } };
    fixed_t             heightmask = dc_texheight - 1;
    byte                dot;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            dot = dc_source[frac >> FRACBITS];
            *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][ditherlow(dc_x, dc_yl++, dc_z)][dot]];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][ditherlow(dc_x, dc_yl, dc_z)][dot]];
    }
    else
    {
        while (--count)
        {
            dot = dc_source[((frac >> FRACBITS) & heightmask)];
            *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][ditherlow(dc_x, dc_yl++, dc_z)][dot]];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        dot = dc_source[((frac >> FRACBITS) & heightmask)];
        *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][ditherlow(dc_x, dc_yl, dc_z)][dot]];
    }
}

void R_DrawBrightmapDitherWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] }, { fullcolormap, fullcolormap } };
    fixed_t             heightmask = dc_texheight - 1;
    byte                dot;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            dot = dc_source[frac >> FRACBITS];
            *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][dither(dc_x, dc_yl++, dc_z)][dot]];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][dither(dc_x, dc_yl, dc_z)][dot]];
    }
    else
    {
        while (--count)
        {
            dot = dc_source[((frac >> FRACBITS) & heightmask)];
            *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][dither(dc_x, dc_yl++, dc_z)][dot]];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        dot = dc_source[((frac >> FRACBITS) & heightmask)];
        *dest = dc_sectorcolormap[colormap[dc_brightmap[dot]][dither(dc_x, dc_yl, dc_z)][dot]];
    }
}

void R_DrawPlayerSpriteColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup1[dc_yl] + dc_x;
    fixed_t frac = dc_texturefrac;

    while (--count)
    {
        *dest = dc_source[frac >> FRACBITS];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_source[frac >> FRACBITS];
}

void R_DrawSkyColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap = dc_colormap[0];
    fixed_t             heightmask = dc_texheight - 1;
    byte                dot;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            if ((dot = dc_source[frac >> FRACBITS]))
                *dest = dc_sectorcolormap[colormap[dot]];

            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        if ((dot = dc_source[frac >> FRACBITS]))
            *dest = dc_sectorcolormap[colormap[dot]];
    }
    else
    {
        while (--count)
        {
            if ((dot = dc_source[((frac >> FRACBITS) & heightmask)]))
                *dest = dc_sectorcolormap[colormap[dot]];

            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        if ((dot = dc_source[((frac >> FRACBITS) & heightmask)]))
            *dest = dc_sectorcolormap[colormap[dot]];
    }
}

void R_DrawFlippedSkyColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap = dc_colormap[0];
    fixed_t             i;

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[dc_source[((i = frac >> FRACBITS) < 128 ? i : 126 - (i & 127))]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[dc_source[((i = frac >> FRACBITS) < 128 ? i : 126 - (i & 127))]]];
}

void R_DrawTranslucentBloodColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttab33[(*dest << 8) + colormap[dc_translation[dc_source[frac >> FRACBITS]]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttab33[(*dest << 8) + colormap[dc_translation[dc_source[frac >> FRACBITS]]]]];
}

void R_DrawTranslucentColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabadditive[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabadditive[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawBrightmapTranslucent50Column(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;
    fixed_t frac = dc_texturefrac;
    byte    dot;

    while (--count)
    {
        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + dc_colormap[dc_brightmap[dot]][dot]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    dot = dc_source[frac >> FRACBITS];
    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + dc_colormap[dc_brightmap[dot]][dot]]];
}

void R_DrawBrightmapDitherTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] }, { fullcolormap, fullcolormap } };
    byte                dot;

    while (--count)
    {
        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dc_brightmap[dot]][dither(dc_x, dc_yl++, dc_z)][dot]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    dot = dc_source[frac >> FRACBITS];
    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dc_brightmap[dot]][dither(dc_x, dc_yl++, dc_z)][dot]]];
}

void R_DrawBrightmapDitherLowTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] }, { fullcolormap, fullcolormap } };
    byte                dot;

    while (--count)
    {
        dot = dc_source[frac >> FRACBITS];
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dc_brightmap[dot]][ditherlow(dc_x, dc_yl++, dc_z)][dot]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    dot = dc_source[frac >> FRACBITS];
    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dc_brightmap[dot]][ditherlow(dc_x, dc_yl++, dc_z)][dot]]];
}

void R_DrawDitherLowTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[ditherlow(dc_x, dc_yl++, dc_z)][dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[ditherlow(dc_x, dc_yl, dc_z)][dc_source[frac >> FRACBITS]]]];
}

void R_DrawDitherTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dither(dc_x, dc_yl++, dc_z)][dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dither(dc_x, dc_yl, dc_z)][dc_source[frac >> FRACBITS]]]];
}

void R_DrawCorrectedTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[nearestcolors[dc_source[frac >> FRACBITS]]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[nearestcolors[dc_source[frac >> FRACBITS]]]]];
}

void R_DrawTranslucent50ColorColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    while (--count)
    {
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + NOTEXTURECOLOR]];
        dest += SCREENWIDTH;
    }

    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + NOTEXTURECOLOR]];
}

void R_DrawTranslucent50ColorDitherLowColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[ditherlow(dc_x, dc_yl++, dc_z)][NOTEXTURECOLOR]]];
        dest += SCREENWIDTH;
    }

    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[ditherlow(dc_x, dc_yl, dc_z)][NOTEXTURECOLOR]]];
}

void R_DrawTranslucent50ColorDitherColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dither(dc_x, dc_yl++, dc_z)][NOTEXTURECOLOR]]];
        dest += SCREENWIDTH;
    }

    *dest = dc_sectorcolormap[tranmap[(*dest << 8) + colormap[dither(dc_x, dc_yl, dc_z)][NOTEXTURECOLOR]]];
}

void R_DrawTranslucent33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttab33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttab33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabred[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabred[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRedWhiteColumn1(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabredwhite1[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabredwhite1[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRedWhiteColumn2(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabredwhite2[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabredwhite2[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRedWhite50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabredwhite50[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabredwhite50[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentGreenColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabgreen[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabgreen[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentBlueColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabblue[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabblue[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRed33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabred33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabred33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentGreen33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabgreen33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabgreen33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentBlue25Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[tinttabblue25[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[tinttabblue25[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]]];
}

void R_DrawFuzzColumn(void)
{
    byte    *dest;
    int     count;

    if (dc_x & 1)
        return;

    if (!(count = (dc_yh - dc_yl) / 2))
        return;

    dest = ylookup0[dc_yl] + dc_x;

    // top
    BIGFUZZYPIXEL(6, (fuzz1table[fuzz1pos++] = FUZZ1((dc_yl >= 2 ? -1 : 0), 1)));

    dest += SCREENWIDTH * 2;

    while (--count)
    {
        // middle
        BIGFUZZYPIXEL(6, (fuzz1table[fuzz1pos++] = FUZZ1(-1, 1)));
        dest += SCREENWIDTH * 2;
    }

    // bottom
    if (dc_yl & 1)
        HALFBIGFUZZYPIXEL(5, (fuzz1table[fuzz1pos++] = FUZZ1(-1, 0)));
    else
        BIGFUZZYPIXEL(5, (fuzz1table[fuzz1pos++] = FUZZ1(-1, 0)));
}

void R_DrawFuzzColumns(void)
{
    const int   width = viewwindowx + viewwidth;
    const int   height = (viewwindowy + viewheight) * SCREENWIDTH;

    for (int y = viewwindowy * SCREENWIDTH; y < height; y += SCREENWIDTH * 2)
        for (int x = viewwindowx + y; x < width + y; x += 2)
        {
            const byte  *source = screens[1] + x;

            if (*source != NOFUZZ)
            {
                byte    *dest = screens[0] + x;

                if (y == height - SCREENWIDTH * 2)
                    BIGFUZZYPIXEL(5, (fuzz2table[fuzz2pos++] = FUZZ2(-1, 0)));
                else if (y >= SCREENWIDTH * 2 && *(source - SCREENWIDTH * 2) == NOFUZZ)
                    BIGFUZZYPIXEL(8, (fuzz2table[fuzz2pos++] = FUZZ2(-1, 1)));
                else
                    BIGFUZZYPIXEL(6, (fuzz2table[fuzz2pos++] = FUZZ2(-1, 1)));
            }
        }
}

//
// R_DrawTranslatedColumn
//
byte    translationtables[256 * 3];

void R_DrawTranslatedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[dc_translation[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[dc_translation[dc_source[frac >> FRACBITS]]]];
}

void R_DrawDitherLowTranslatedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl++, dc_z)][dc_translation[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[ditherlow(dc_x, dc_yl, dc_z)][dc_translation[dc_source[frac >> FRACBITS]]]];
}

void R_DrawDitherTranslatedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };

    while (--count)
    {
        *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl++, dc_z)][dc_translation[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_sectorcolormap[colormap[dither(dc_x, dc_yl, dc_z)][dc_translation[dc_source[frac >> FRACBITS]]]];
}

//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//
void R_InitTranslationTables(void)
{
    // translate just the 16 green colors
    for (int i = 0; i < 256; i++)
        if (i >= 0x70 && i <= 0x7F)
        {
            // map green ramp to gray, brown, red
            translationtables[i] = 0x60 + (i & 0x0F);
            translationtables[i + 256] = 0x40 + (i & 0x0F);
            translationtables[i + 512] = 0x20 + (i & 0x0F);
        }
        else
        {
            // keep all other colors as is
            translationtables[i] = i;
            translationtables[i + 256] = i;
            translationtables[i + 512] = i;
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
int             ds_x1;
int             ds_x2;
int             ds_y;
int             ds_z;

lighttable_t    *ds_colormap[2];
lighttable_t    *ds_sectorcolormap;

fixed_t         ds_xfrac;
fixed_t         ds_yfrac;
fixed_t         ds_xstep;
fixed_t         ds_ystep;

// start of a 64x64 tile image
byte            *ds_source;

//
// Draws the actual span.
//
void R_DrawSpan(void)
{
    int                 count = ds_x2 - ds_x1;
    byte                *dest = ylookup0[ds_y] + ds_x1;
    const lighttable_t  *colormap = ds_colormap[0];

    while (--count)
    {
        *dest++ = ds_sectorcolormap[colormap[ds_source[((ds_xfrac >> 16) & 63) | ((ds_yfrac >> 10) & 4032)]]];
        ds_xfrac += ds_xstep;
        ds_yfrac += ds_ystep;
    }

    *dest = ds_sectorcolormap[colormap[ds_source[((ds_xfrac >> 16) & 63) | ((ds_yfrac >> 10) & 4032)]]];
}

void R_DrawDitherLowSpan(void)
{
    int     count = ds_x2 - ds_x1;
    byte    *dest = ylookup0[ds_y] + ds_x1;

    while (--count)
    {
        *dest++ = ds_sectorcolormap[ds_colormap[ditherlow(ds_x1++, ds_y, ds_z)][ds_source[((ds_xfrac >> 16) & 63) | ((ds_yfrac >> 10) & 4032)]]];
        ds_xfrac += ds_xstep;
        ds_yfrac += ds_ystep;
    }

    *dest = ds_sectorcolormap[ds_colormap[ditherlow(ds_x1, ds_y, ds_z)][ds_source[((ds_xfrac >> 16) & 63) | ((ds_yfrac >> 10) & 4032)]]];
}

void R_DrawDitherSpan(void)
{
    int     count = ds_x2 - ds_x1;
    byte    *dest = ylookup0[ds_y] + ds_x1;

    while (--count)
    {
        *dest++ = ds_sectorcolormap[ds_colormap[dither(ds_x1++, ds_y, ds_z)][ds_source[((ds_xfrac >> 16) & 63) | ((ds_yfrac >> 10) & 4032)]]];
        ds_xfrac += ds_xstep;
        ds_yfrac += ds_ystep;
    }

    *dest = ds_sectorcolormap[ds_colormap[dither(ds_x1, ds_y, ds_z)][ds_source[((ds_xfrac >> 16) & 63) | ((ds_yfrac >> 10) & 4032)]]];
}

void R_DrawColorSpan(void)
{
    int         count = ds_x2 - ds_x1;
    byte        *dest = ylookup0[ds_y] + ds_x1;
    const byte  color = ds_sectorcolormap[ds_colormap[0][NOTEXTURECOLOR]];

    while (--count)
        *dest++ = color;

    *dest = color;
}

void R_DrawDitherLowColorSpan(void)
{
    int     count = ds_x2 - ds_x1;
    byte    *dest = ylookup0[ds_y] + ds_x1;

    while (--count)
        *dest++ = ds_sectorcolormap[ds_colormap[ditherlow(ds_x1++, ds_y, ds_z)][NOTEXTURECOLOR]];

    *dest = ds_sectorcolormap[ds_colormap[ditherlow(ds_x1, ds_y, ds_z)][NOTEXTURECOLOR]];
}

void R_DrawDitherColorSpan(void)
{
    int     count = ds_x2 - ds_x1;
    byte    *dest = ylookup0[ds_y] + ds_x1;

    while (--count)
        *dest++ = ds_sectorcolormap[ds_colormap[dither(ds_x1++, ds_y, ds_z)][NOTEXTURECOLOR]];

    *dest = ds_sectorcolormap[ds_colormap[dither(ds_x1, ds_y, ds_z)][NOTEXTURECOLOR]];
}

//
// R_InitBuffer
//
void R_InitBuffer(void)
{
    const int   end = (viewwindowy + viewheight) * SCREENWIDTH + viewwindowx + viewwidth;

    for (int i = 0, y = viewwindowy * SCREENWIDTH + viewwindowx; y < end; i++, y += SCREENWIDTH)
    {
        ylookup0[i] = screens[0] + y;
        ylookup1[i] = screens[1] + y;
    }

    fuzzrange[0] = -SCREENWIDTH * 2;
    fuzzrange[1] = 0;
    fuzzrange[2] = SCREENWIDTH * 2;

    memset(fuzz1table, 0, MAXSCREENAREA);
    memset(fuzz2table, 0, MAXSCREENAREA);
}

void R_FillBezel(void)
{
    byte    *dest = &screens[0][(SCREENHEIGHT - SBARHEIGHT) * SCREENWIDTH];

    for (int y = SCREENHEIGHT - SBARHEIGHT; y < SCREENHEIGHT; y++)
        for (int x = 0; x < SCREENWIDTH; x += 2)
        {
            const byte  dot = grnrock[(((y >> 1) & 63) << 6) + ((x >> 1) & 63)];

            *dest++ = dot;
            *dest++ = dot;
        }

    if (st_drawbrdr)
    {
        for (int x = 0; x < (SCREENWIDTH - NONWIDEWIDTH) / 2 / 2; x += 8)
            V_DrawPatch(x - WIDESCREENDELTA, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, brdr_b);

        for (int x = SCREENWIDTH / 2 - 8; x >= (SCREENWIDTH + NONWIDEWIDTH) / 2 / 2 - 8; x -= 8)
            V_DrawPatch(x - WIDESCREENDELTA, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, brdr_b);
    }
}

//
// R_FillBackScreen
// Fills the back screen with a pattern for variable screen sizes.
// Also draws a beveled edge.
//
void R_FillBackScreen(void)
{
    byte    *dest = screens[1];

    for (int y = 0; y < SCREENHEIGHT - SBARHEIGHT; y++)
        for (int x = 0; x < SCREENWIDTH; x += 2)
        {
            const byte  dot = grnrock[(((y >> 1) & 63) << 6) + ((x >> 1) & 63)];

            *dest++ = dot;
            *dest++ = dot;
        }

    if (st_drawbrdr)
    {
        const int   x1 = viewwindowx / 2 - WIDESCREENDELTA;
        const int   y1 = viewwindowy / 2;
        const int   x2 = viewwidth / 2 + x1;
        const int   y2 = viewheight / 2 + y1;

        for (int x = x1; x < x2 - 8; x += 8)
        {
            V_DrawPatch(x, y1 - 8, 1, brdr_t);
            V_DrawPatch(x, y2, 1, brdr_b);
        }

        V_DrawPatch(x2 - 8, y1 - 8, 1, brdr_t);
        V_DrawPatch(x2 - 8, y2, 1, brdr_b);

        for (int y = y1; y < y2 - 8; y += 8)
        {
            V_DrawPatch(x1 - 8, y, 1, brdr_l);
            V_DrawPatch(x2, y, 1, brdr_r);
        }

        V_DrawPatch(x1 - 8, y2 - 8, 1, brdr_l);
        V_DrawPatch(x2, y2 - 8, 1, brdr_r);

        V_DrawPatch(x1 - 8, y1 - 8, 1, brdr_tl);
        V_DrawPatch(x2, y1 - 8, 1, brdr_tr);
        V_DrawPatch(x1 - 8, y2, 1, brdr_bl);
        V_DrawPatch(x2, y2, 1, brdr_br);
    }
}

//
// Copy a screen buffer.
//
void R_VideoErase(unsigned int offset, int count)
{
    memcpy(screens[0] + offset, screens[1] + offset, count);
}

//
// R_DrawViewBorder
// Draws the border around the view for different size windows?
//
void R_DrawViewBorder(void)
{
    const int   top = (SCREENHEIGHT - SBARHEIGHT - viewheight) * SCREENWIDTH / 2;
    int         side = (SCREENWIDTH - viewwidth) / 2;
    int         offset = top - side;
    const int   count = top + side;

    // copy top and one line of left side
    R_VideoErase(0, count);

    // copy one line of right side and bottom
    R_VideoErase(viewheight * SCREENWIDTH + offset, count);

    side *= 2;

    // copy sides using wraparound
    for (int y = 1; y < viewheight; y++)
    {
        offset += SCREENWIDTH;
        R_VideoErase(offset, side);
    }
}
