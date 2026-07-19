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

#include "i_colors.h"
#include "m_config.h"
#include "p_local.h"
#include "r_main.h"
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

static byte     flipindex[256];

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

int             ditherxoffset;

#define DITHERSIZE      4
#define DITHERMASK      (DITHERSIZE - 1)
#define RADIALLIGHTBIAS (16 * FRACUNIT)

static const byte dithermatrix[DITHERSIZE][DITHERSIZE] =
{
    {   0, 224,  48, 208 },
    { 176,  80, 128,  96 },
    { 192,  32, 240,  16 },
    { 112, 144,  64, 160 }
};

static const byte dithercolumns[DITHERSIZE][DITHERSIZE] =
{
    {   0, 176, 192, 112 },
    { 224,  80,  32, 144 },
    {  48, 128, 240,  64 },
    { 208,  96,  16, 160 }
};

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
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[cmap[source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[cmap[source[frac >> FRACBITS]]];
}

void R_DrawColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const lighttable_t  *cmap[2] = { dc_colormap[0], fullcolormap };
    const fixed_t       iscale = dc_iscale;
    byte                dot;

    while (--count)
    {
        dot = source[frac >> FRACBITS];
        *dest = scmap[cmap[bmap[dot]][dot]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    dot = source[frac >> FRACBITS];
    *dest = scmap[cmap[bmap[dot]][dot]];
}

void R_DrawLowResDitheredColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

    while (--count)
    {
        *dest = scmap[cmap[thresholds[yphase] < z][source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += iscale;

        if (++lowy == lowpixelrows)
        {
            lowy = 0;
            yphase = (yphase + 1) & DITHERMASK;
        }
    }

    *dest = scmap[cmap[thresholds[yphase] < z][source[frac >> FRACBITS]]];
}

void R_DrawDitheredColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

    while (--count)
    {
        *dest = scmap[cmap[thresholds[yphase] < z][source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += iscale;
        yphase = (yphase + 1) & DITHERMASK;
    }

    *dest = scmap[cmap[thresholds[yphase] < z][source[frac >> FRACBITS]]];
}

void R_DrawLowResDitheredColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] },
                                            { fullcolormap,   fullcolormap       } };
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    byte                dot;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

    while (--count)
    {
        dot = source[frac >> FRACBITS];
        *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
        dest += SCREENWIDTH;
        frac += iscale;

        if (++lowy == lowpixelrows)
        {
            lowy = 0;
            yphase = (yphase + 1) & DITHERMASK;
        }
    }

    dot = source[frac >> FRACBITS];
    *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
}

void R_DrawDitheredColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] },
                                            { fullcolormap,   fullcolormap       } };
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    byte                dot;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

    while (--count)
    {
        dot = source[frac >> FRACBITS];
        *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
        dest += SCREENWIDTH;
        frac += iscale;
        yphase = (yphase + 1) & DITHERMASK;
    }

    dot = source[frac >> FRACBITS];
    *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
}

void R_DrawCorrectedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[cmap[nearestcolors[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[cmap[nearestcolors[source[frac >> FRACBITS]]]];
}

void R_DrawCorrectedLowResDitheredColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

    while (--count)
    {
        *dest = scmap[cmap[thresholds[yphase] < z][nearestcolors[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;

        if (++lowy == lowpixelrows)
        {
            lowy = 0;
            yphase = (yphase + 1) & DITHERMASK;
        }
    }

    *dest = scmap[cmap[thresholds[yphase] < z][nearestcolors[source[frac >> FRACBITS]]]];
}

void R_DrawCorrectedDitheredColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

    while (--count)
    {
        *dest = scmap[cmap[thresholds[yphase] < z][nearestcolors[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
        yphase = (yphase + 1) & DITHERMASK;
    }

    *dest = scmap[cmap[thresholds[yphase] < z][nearestcolors[source[frac >> FRACBITS]]]];
}

void R_DrawSolidColorColumn(void)
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

void R_DrawLowResDitheredSolidColorColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const byte          color[2] = { dc_sectorcolormap[cmap[0][NOTEXTURECOLOR]],
                                     dc_sectorcolormap[cmap[1][NOTEXTURECOLOR]] };
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

    while (--count)
    {
        *dest = color[thresholds[yphase] < z];
        dest += SCREENWIDTH;

        if (++lowy == lowpixelrows)
        {
            lowy = 0;
            yphase = (yphase + 1) & DITHERMASK;
        }
    }

    *dest = color[thresholds[yphase] < z];
}

void R_DrawDitheredSolidColorColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const byte          color[2] = { dc_sectorcolormap[cmap[0][NOTEXTURECOLOR]],
                                     dc_sectorcolormap[cmap[1][NOTEXTURECOLOR]] };
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

    while (--count)
    {
        *dest = color[thresholds[yphase] < z];
        dest += SCREENWIDTH;
        yphase = (yphase + 1) & DITHERMASK;
    }

    *dest = color[thresholds[yphase] < z];
}

void R_DrawShadowColumn(void)
{
    int     count = dc_yh - dc_yl;
    byte    *dest = ylookup0[dc_yl] + dc_x;
    byte    *black = dc_black40;
    byte    *black33 = dc_black33;

    if (count)
    {
        *dest = *(*dest + black33);
        dest += SCREENWIDTH;

        while (--count)
        {
            *dest = *(*dest + black);
            dest += SCREENWIDTH;
        }

        *dest = *(*dest + (dc_yh == dc_floorclip ? black : black33));
    }
    else
        *dest = *(*dest + black33);
}

void R_DrawFuzzyShadowColumn(void)
{
    byte    *dest;
    int     count;
    byte    *black33 = dc_black33;

    if (dc_x & 1)
        return;

    dest = ylookup0[dc_yl] + dc_x;

    if ((count = dc_yh - dc_yl))
    {
        *dest = *(*dest + black33);
        *(dest + 1) = *(*(dest + 1) + black33);
        dest += SCREENWIDTH;

        while (--count)
        {
            *dest = *(*dest + black33);
            *(dest + 1) = *(*(dest + 1) + black33);
            dest += SCREENWIDTH;
        }

        *dest = *(*dest + black33);
        *(dest + 1) = *(*(dest + 1) + black33);
    }
    else
    {
        *dest = *(*dest + black33);
        *(dest + 1) = *(*(dest + 1) + black33);
    }
}

void R_DrawSolidShadowColumn(void)
{
    int         count = dc_yh - dc_yl + 1;
    byte        *dest = ylookup0[dc_yl] + dc_x;
    const byte  black = dc_black;

    while (--count)
    {
        *dest = black;
        dest += SCREENWIDTH;
    }

    *dest = black;
}

void R_DrawBloodSplatColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;
    byte    *bloodcolor = dc_bloodcolor;
    byte    *scmap = dc_sectorcolormap;

    while (--count)
    {
        *dest = scmap[*(*dest + bloodcolor)];
        dest += SCREENWIDTH;
    }

    *dest = scmap[*(*dest + bloodcolor)];
}

void R_DrawSolidBloodSplatColumn(void)
{
    int         count = dc_yh - dc_yl + 1;
    byte        *dest = ylookup0[dc_yl] + dc_x;
    const byte  color = dc_sectorcolormap[dc_solidbloodcolor];

    while (--count)
    {
        *dest = color;
        dest += SCREENWIDTH;
    }

    *dest = color;
}

void R_DrawWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
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
            *dest = scmap[cmap[source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;

            if ((frac += iscale) >= heightmask)
                frac -= heightmask;
        }

        *dest = scmap[cmap[source[frac >> FRACBITS]]];
    }
    else
    {
        while (--count)
        {
            *dest = scmap[cmap[source[(frac >> FRACBITS) & heightmask]]];
            dest += SCREENWIDTH;
            frac += iscale;
        }

        *dest = scmap[cmap[source[(frac >> FRACBITS) & heightmask]]];
    }
}

void R_DrawLowResDitheredWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    fixed_t             heightmask = dc_texheight - 1;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

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
            *dest = scmap[cmap[thresholds[yphase] < z][source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;

            if ((frac += iscale) >= heightmask)
                frac -= heightmask;

            if (++lowy == lowpixelrows)
            {
                lowy = 0;
                yphase = (yphase + 1) & DITHERMASK;
            }
        }

        *dest = scmap[cmap[thresholds[yphase] < z][source[frac >> FRACBITS]]];
    }
    else
    {
        while (--count)
        {
            *dest = scmap[cmap[thresholds[yphase] < z][source[(frac >> FRACBITS) & heightmask]]];
            dest += SCREENWIDTH;
            frac += iscale;

            if (++lowy == lowpixelrows)
            {
                lowy = 0;
                yphase = (yphase + 1) & DITHERMASK;
            }
        }

        *dest = scmap[cmap[thresholds[yphase] < z][source[(frac >> FRACBITS) & heightmask]]];
    }
}

void R_DrawDitheredWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    fixed_t             heightmask = dc_texheight - 1;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

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
            *dest = scmap[cmap[thresholds[yphase] < z][source[frac >> FRACBITS]]];
            dest += SCREENWIDTH;

            if ((frac += iscale) >= heightmask)
                frac -= heightmask;

            yphase = (yphase + 1) & DITHERMASK;
        }

        *dest = scmap[cmap[thresholds[yphase] < z][source[frac >> FRACBITS]]];
    }
    else
    {
        while (--count)
        {
            *dest = scmap[cmap[thresholds[yphase] < z][source[(frac >> FRACBITS) & heightmask]]];
            dest += SCREENWIDTH;
            frac += iscale;
            yphase = (yphase + 1) & DITHERMASK;
        }

        *dest = scmap[cmap[thresholds[yphase] < z][source[(frac >> FRACBITS) & heightmask]]];
    }
}

void R_DrawWallColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *cmap[2] = { dc_colormap[0], fullcolormap };
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
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
            dot = source[frac >> FRACBITS];
            *dest = scmap[cmap[bmap[dot]][dot]];
            dest += SCREENWIDTH;

            if ((frac += iscale) >= heightmask)
                frac -= heightmask;
        }

        dot = source[frac >> FRACBITS];
        *dest = scmap[cmap[bmap[dot]][dot]];
    }
    else
    {
        while (--count)
        {
            dot = source[(frac >> FRACBITS) & heightmask];
            *dest = scmap[cmap[bmap[dot]][dot]];
            dest += SCREENWIDTH;
            frac += iscale;
        }

        dot = source[(frac >> FRACBITS) & heightmask];
        *dest = scmap[cmap[bmap[dot]][dot]];
    }
}

void R_DrawLowResDitheredWallColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *cmap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] },
                                            { fullcolormap,   fullcolormap       } };
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    fixed_t             heightmask = dc_texheight - 1;
    byte                dot;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

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
            dot = source[frac >> FRACBITS];
            *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
            dest += SCREENWIDTH;

            if ((frac += iscale) >= heightmask)
                frac -= heightmask;

            if (++lowy == lowpixelrows)
            {
                lowy = 0;
                yphase = (yphase + 1) & DITHERMASK;
            }
        }

        dot = source[frac >> FRACBITS];
        *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
    }
    else
    {
        while (--count)
        {
            dot = source[(frac >> FRACBITS) & heightmask];
            *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
            dest += SCREENWIDTH;
            frac += iscale;

            if (++lowy == lowpixelrows)
            {
                lowy = 0;
                yphase = (yphase + 1) & DITHERMASK;
            }
        }

        dot = source[(frac >> FRACBITS) & heightmask];
        *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
    }
}

void R_DrawDitheredWallColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *cmap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] },
                                            { fullcolormap,   fullcolormap       } };
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    fixed_t             heightmask = dc_texheight - 1;
    byte                dot;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

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
            dot = source[frac >> FRACBITS];
            *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
            dest += SCREENWIDTH;

            if ((frac += iscale) >= heightmask)
                frac -= heightmask;

            yphase = (yphase + 1) & DITHERMASK;
        }

        dot = source[frac >> FRACBITS];
        *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
    }
    else
    {
        while (--count)
        {
            dot = source[(frac >> FRACBITS) & heightmask];
            *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
            dest += SCREENWIDTH;
            frac += iscale;
            yphase = (yphase + 1) & DITHERMASK;
        }

        dot = source[(frac >> FRACBITS) & heightmask];
        *dest = scmap[cmap[bmap[dot]][thresholds[yphase] < z][dot]];
    }
}

void R_DrawPlayerSpriteColumn(void)
{
    int             count = dc_yh - dc_yl + 1;
    byte            *dest = ylookup1[dc_yl] + dc_x;
    fixed_t         frac = dc_texturefrac;
    byte            *source = dc_source;
    const fixed_t   iscale = dc_iscale;

    while (--count)
    {
        *dest = source[frac >> FRACBITS];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = source[frac >> FRACBITS];
}

void R_DrawSkyColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
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
            if ((dot = source[frac >> FRACBITS]))
                *dest = scmap[cmap[dot]];

            dest += SCREENWIDTH;

            if ((frac += iscale) >= heightmask)
                frac -= heightmask;
        }

        if ((dot = source[frac >> FRACBITS]))
            *dest = scmap[cmap[dot]];
    }
    else
    {
        while (--count)
        {
            if ((dot = source[(frac >> FRACBITS) & heightmask]))
                *dest = scmap[cmap[dot]];

            dest += SCREENWIDTH;
            frac += iscale;
        }

        if ((dot = source[(frac >> FRACBITS) & heightmask]))
            *dest = scmap[cmap[dot]];
    }
}

void R_DrawFlippedSkyColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    byte                *flip = flipindex;

    while (--count)
    {
        *dest = scmap[cmap[source[flip[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[cmap[source[flip[frac >> FRACBITS]]]];
}

void R_DrawTranslucentBloodColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *translation = dc_translation;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttab33[(*dest << 8) + cmap[translation[source[frac >> FRACBITS]]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttab33[(*dest << 8) + cmap[translation[source[frac >> FRACBITS]]]]];
}

void R_DrawTranslucentColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabadditive[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabadditive[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tranmap[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tranmap[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucent50ColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], fullcolormap };
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    byte                dot;

    while (--count)
    {
        dot = source[frac >> FRACBITS];
        *dest = scmap[tranmap[(*dest << 8) + cmap[bmap[dot]][dot]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    dot = source[frac >> FRACBITS];
    *dest = scmap[tranmap[(*dest << 8) + cmap[bmap[dot]][dot]]];
}

void R_DrawDitheredTranslucent50ColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] },
                                            { fullcolormap,   fullcolormap       } };
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    byte                dot;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

    while (--count)
    {
        dot = source[frac >> FRACBITS];
        *dest = scmap[tranmap[(*dest << 8) + cmap[bmap[dot]][thresholds[yphase] < z][dot]]];
        dest += SCREENWIDTH;
        frac += iscale;
        yphase = (yphase + 1) & DITHERMASK;
    }

    dot = source[frac >> FRACBITS];
    *dest = scmap[tranmap[(*dest << 8) + cmap[bmap[dot]][thresholds[yphase] < z][dot]]];
}

void R_DrawLowResDitheredTranslucent50ColumnWithBrightmap(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] },
                                            { fullcolormap,   fullcolormap       } };
    byte                *source = dc_source;
    byte                *bmap = dc_brightmap;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    byte                dot;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

    while (--count)
    {
        dot = source[frac >> FRACBITS];
        *dest = scmap[tranmap[(*dest << 8) + cmap[bmap[dot]][thresholds[yphase] < z][dot]]];
        dest += SCREENWIDTH;
        frac += iscale;

        if (++lowy == lowpixelrows)
        {
            lowy = 0;
            yphase = (yphase + 1) & DITHERMASK;
        }
    }

    dot = source[frac >> FRACBITS];
    *dest = scmap[tranmap[(*dest << 8) + cmap[bmap[dot]][thresholds[yphase] < z][dot]]];
}

void R_DrawLowResDitheredTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

    while (--count)
    {
        *dest = scmap[tranmap[(*dest << 8) + cmap[thresholds[yphase] < z]
            [source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;

        if (++lowy == lowpixelrows)
        {
            lowy = 0;
            yphase = (yphase + 1) & DITHERMASK;
        }
    }

    *dest = scmap[tranmap[(*dest << 8) + cmap[thresholds[yphase] < z]
        [source[frac >> FRACBITS]]]];
}

void R_DrawDitheredTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

    while (--count)
    {
        *dest = scmap[tranmap[(*dest << 8) + cmap[thresholds[yphase] < z]
            [source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
        yphase = (yphase + 1) & DITHERMASK;
    }

    *dest = scmap[tranmap[(*dest << 8) + cmap[thresholds[yphase] < z]
        [source[frac >> FRACBITS]]]];
}

void R_DrawCorrectedTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tranmap[(*dest << 8) + cmap[nearestcolors[source[frac >> FRACBITS]]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tranmap[(*dest << 8) + cmap[nearestcolors[source[frac >> FRACBITS]]]]];
}

void R_DrawTranslucent50SolidColorColumn(void)
{
    int         count = dc_yh - dc_yl + 1;
    byte        *dest = ylookup0[dc_yl] + dc_x;
    byte        *scmap = dc_sectorcolormap;
    const int   color = (NOTEXTURECOLOR << 8);

    while (--count)
    {
        *dest = scmap[tranmap[color + *dest]];
        dest += SCREENWIDTH;
    }

    *dest = scmap[tranmap[color + *dest]];
}

void R_DrawLowResDitheredTranslucent50SolidColorColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const byte          color[2] = { cmap[0][NOTEXTURECOLOR], cmap[1][NOTEXTURECOLOR] };
    byte                *scmap = dc_sectorcolormap;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

    while (--count)
    {
        *dest = scmap[tranmap[(*dest << 8) + color[thresholds[yphase] < z]]];
        dest += SCREENWIDTH;

        if (++lowy == lowpixelrows)
        {
            lowy = 0;
            yphase = (yphase + 1) & DITHERMASK;
        }
    }

    *dest = scmap[tranmap[(*dest << 8) + color[thresholds[yphase] < z]]];
}

void R_DrawDitheredTranslucent50SolidColorColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const byte          color[2] = { cmap[0][NOTEXTURECOLOR], cmap[1][NOTEXTURECOLOR] };
    byte                *scmap = dc_sectorcolormap;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

    while (--count)
    {
        *dest = scmap[tranmap[(*dest << 8) + color[thresholds[yphase] < z]]];
        dest += SCREENWIDTH;
        yphase = (yphase + 1) & DITHERMASK;
    }

    *dest = scmap[tranmap[(*dest << 8) + color[thresholds[yphase] < z]]];
}

void R_DrawTranslucent33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttab33[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttab33[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabred[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabred[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRedWhiteColumn1(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabredwhite1[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabredwhite1[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRedWhiteColumn2(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabredwhite2[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabredwhite2[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRedWhite50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabredwhite50[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabredwhite50[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentGreenColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabgreen[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabgreen[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentBlueColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabblue[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabblue[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentRed33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabred33[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabred33[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentGreen33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabgreen33[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabgreen33[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentBlue25Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[tinttabblue25[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[tinttabblue25[(*dest << 8) + cmap[source[frac >> FRACBITS]]]];
}

void R_DrawFuzzColumn(void)
{
    byte        *dest;
    int         count;
    const int   step = SCREENWIDTH * 2;

    if (dc_x & 1)
        return;

    if (!(count = (dc_yh - dc_yl) / 2))
        return;

    dest = ylookup0[dc_yl] + dc_x;

    // top
    BIGFUZZYPIXEL(6, (fuzz1table[fuzz1pos++] = FUZZ1((dc_yl >= 2 ? -1 : 0), 1)));

    dest += step;

    while (--count)
    {
        // middle
        BIGFUZZYPIXEL(6, (fuzz1table[fuzz1pos++] = FUZZ1(-1, 1)));
        dest += step;
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
                    BIGFUZZYPIXEL(6, (fuzz2table[fuzz2pos++] = FUZZ2((y >= SCREENWIDTH * 2 ? -1 : 0), 1)));
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
    const lighttable_t  *cmap = dc_colormap[0];
    byte                *source = dc_source;
    byte                *translation = dc_translation;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;

    while (--count)
    {
        *dest = scmap[cmap[translation[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
    }

    *dest = scmap[cmap[translation[source[frac >> FRACBITS]]]];
}

void R_DrawLowResDitheredTranslatedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *translation = dc_translation;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    const byte          *thresholds = dithercolumns[((dc_x + ditherxoffset) / lowpixelwidth) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = (dc_yl / lowpixelrows) & DITHERMASK;
    int                 lowy = dc_yl % lowpixelrows;

    while (--count)
    {
        *dest = scmap[cmap[thresholds[yphase] < z][translation[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;

        if (++lowy == lowpixelrows)
        {
            lowy = 0;
            yphase = (yphase + 1) & DITHERMASK;
        }
    }

    *dest = scmap[cmap[thresholds[yphase] < z][translation[source[frac >> FRACBITS]]]];
}

void R_DrawDitheredTranslatedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *cmap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    byte                *source = dc_source;
    byte                *translation = dc_translation;
    byte                *scmap = dc_sectorcolormap;
    const fixed_t       iscale = dc_iscale;
    const byte          *thresholds = dithercolumns[(dc_x + ditherxoffset) & DITHERMASK];
    const int           z = dc_z;
    int                 yphase = dc_yl & DITHERMASK;

    while (--count)
    {
        *dest = scmap[cmap[thresholds[yphase] < z][translation[source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += iscale;
        yphase = (yphase + 1) & DITHERMASK;
    }

    *dest = scmap[cmap[thresholds[yphase] < z][translation[source[frac >> FRACBITS]]]];
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
lighttable_t    **ds_zlight;
lighttable_t    *ds_sectorcolormap;

fixed_t         ds_xfrac;
fixed_t         ds_yfrac;
fixed_t         ds_xstep;
fixed_t         ds_ystep;
fixed_t         ds_lightxfrac;
fixed_t         ds_lightyfrac;
fixed_t         ds_lightxstep;
fixed_t         ds_lightystep;

// start of a 64x64 tile image
byte            *ds_source;
byte            *ds_brightmap;

static inline fixed_t R_ApproxDistance(fixed_t dx, fixed_t dy)
{
    dx = ABS(dx);
    dy = ABS(dy);

    return dx + dy - (MIN(dx, dy) >> 1);
}

static inline int R_GetRadialLightIndex(const fixed_t lightxfrac, const fixed_t lightyfrac)
{
    const fixed_t   distance = R_ApproxDistance(lightxfrac, lightyfrac) - RADIALLIGHTBIAS;

    if (distance <= 0)
        return 0;

    return BETWEEN(0, distance >> LIGHTZSHIFT, MAXLIGHTZ - 1);
}

static inline int R_GetRadialDitheredLightIndex(const fixed_t lightxfrac, const fixed_t lightyfrac,
    const byte threshold)
{
    const fixed_t   distance = MAX(0, R_ApproxDistance(lightxfrac, lightyfrac) - RADIALLIGHTBIAS);
    const int       lightindex = BETWEEN(0, distance >> LIGHTZSHIFT, MAXLIGHTZ - 1);

    return MIN(lightindex + (threshold < ((distance >> 12) & 255)), MAXLIGHTZ - 1);
}

//
// Draws the actual span.
//
void R_DrawSpan(void)
{
    int                 count = ds_x2 - ds_x1;
    byte                *dest = ylookup0[ds_y] + ds_x1;
    const lighttable_t  *cmap = ds_colormap[0];
    byte                *source = ds_source;
    byte                *scmap = ds_sectorcolormap;
    fixed_t             xfrac = ds_xfrac;
    fixed_t             yfrac = ds_yfrac;
    const fixed_t       xstep = ds_xstep;
    const fixed_t       ystep = ds_ystep;

    while (--count)
    {
        *dest++ = scmap[cmap[source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
        xfrac += xstep;
        yfrac += ystep;
    }

    *dest = scmap[cmap[source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
}

void R_DrawRadialSpan(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    byte            *source = ds_source;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         xfrac = ds_xfrac;
    fixed_t         yfrac = ds_yfrac;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   xstep = ds_xstep;
    const fixed_t   ystep = ds_ystep;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;

    while (--count)
    {
        *dest++ = scmap[light[R_GetRadialLightIndex(lightxfrac, lightyfrac)]
            [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
        xfrac += xstep;
        yfrac += ystep;
        lightxfrac += lightxstep;
        lightyfrac += lightystep;
    }

    *dest = scmap[light[R_GetRadialLightIndex(lightxfrac, lightyfrac)]
        [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
}

void R_DrawSpanWithBrightmap(void)
{
    int                 count = ds_x2 - ds_x1;
    byte                *dest = ylookup0[ds_y] + ds_x1;
    const lighttable_t  *cmap[2] = { ds_colormap[0], fullcolormap };
    byte                *source = ds_source;
    byte                *bmap = ds_brightmap;
    byte                *scmap = ds_sectorcolormap;
    fixed_t             xfrac = ds_xfrac;
    fixed_t             yfrac = ds_yfrac;
    const fixed_t       xstep = ds_xstep;
    const fixed_t       ystep = ds_ystep;
    byte                dot;

    while (--count)
    {
        dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];
        *dest++ = scmap[cmap[bmap[dot]][dot]];
        xfrac += xstep;
        yfrac += ystep;
    }

    dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];
    *dest = scmap[cmap[bmap[dot]][dot]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
}

void R_DrawRadialSpanWithBrightmap(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    byte            *source = ds_source;
    byte            *bmap = ds_brightmap;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         xfrac = ds_xfrac;
    fixed_t         yfrac = ds_yfrac;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   xstep = ds_xstep;
    const fixed_t   ystep = ds_ystep;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;
    byte            dot;

    while (--count)
    {
        dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];

        if (bmap[dot])
            *dest++ = scmap[fullcolormap[dot]];
        else
            *dest++ = scmap[light[R_GetRadialLightIndex(lightxfrac, lightyfrac)][dot]];

        xfrac += xstep;
        yfrac += ystep;
        lightxfrac += lightxstep;
        lightyfrac += lightystep;
    }

    dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];

    if (bmap[dot])
        *dest = scmap[fullcolormap[dot]];
    else
        *dest = scmap[light[R_GetRadialLightIndex(lightxfrac, lightyfrac)][dot]];

    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
}

void R_DrawLowResDitheredSpan(void)
{
    int                 count = ds_x2 - ds_x1;
    byte                *dest = ylookup0[ds_y] + ds_x1;
    const lighttable_t  *cmap[2] = { ds_colormap[0], ds_colormap[1] };
    byte                *source = ds_source;
    byte                *scmap = ds_sectorcolormap;
    fixed_t             xfrac = ds_xfrac;
    fixed_t             yfrac = ds_yfrac;
    const fixed_t       xstep = ds_xstep;
    const fixed_t       ystep = ds_ystep;
    const byte          *thresholds = dithermatrix[(ds_y / lowpixelrows) & DITHERMASK];
    const int           z = ds_z;
    const int           x = ds_x1 + ditherxoffset;
    int                 lowx = x % lowpixelwidth;
    int                 xphase = (x / lowpixelwidth) & DITHERMASK;

    while (--count)
    {
        *dest++ = scmap[cmap[thresholds[xphase] < z]
            [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
        xfrac += xstep;
        yfrac += ystep;

        if (++lowx == lowpixelwidth)
        {
            lowx = 0;
            xphase = (xphase + 1) & DITHERMASK;
        }
    }

    *dest = scmap[cmap[thresholds[xphase] < z]
        [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
}

void R_DrawLowResDitheredRadialSpan(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    byte            *source = ds_source;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         xfrac = ds_xfrac;
    fixed_t         yfrac = ds_yfrac;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   xstep = ds_xstep;
    const fixed_t   ystep = ds_ystep;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;
    const byte      *thresholds = dithermatrix[(ds_y / lowpixelrows) & DITHERMASK];
    const int       x = ds_x1 + ditherxoffset;
    int             lowx = x % lowpixelwidth;
    int             xphase = (x / lowpixelwidth) & DITHERMASK;

    while (--count)
    {
        *dest++ = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])]
            [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
        xfrac += xstep;
        yfrac += ystep;
        lightxfrac += lightxstep;
        lightyfrac += lightystep;

        if (++lowx == lowpixelwidth)
        {
            lowx = 0;
            xphase = (xphase + 1) & DITHERMASK;
        }
    }

    *dest = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])]
        [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
}

void R_DrawLowResDitheredSpanWithBrightmap(void)
{
    int                 count = ds_x2 - ds_x1;
    byte                *dest = ylookup0[ds_y] + ds_x1;
    byte                *source = ds_source;
    byte                *bmap = ds_brightmap;
    byte                *scmap = ds_sectorcolormap;
    fixed_t             xfrac = ds_xfrac;
    fixed_t             yfrac = ds_yfrac;
    const fixed_t       xstep = ds_xstep;
    const fixed_t       ystep = ds_ystep;
    byte                dot;
    const byte          *thresholds = dithermatrix[(ds_y / lowpixelrows) & DITHERMASK];
    const int           z = ds_z;
    const int           x = ds_x1 + ditherxoffset;
    int                 lowx = x % lowpixelwidth;
    int                 xphase = (x / lowpixelwidth) & DITHERMASK;
    const lighttable_t  *cmap[2][2] = { { ds_colormap[0], ds_colormap[1] },
                                        { fullcolormap,   fullcolormap   } };

    while (--count)
    {
        dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];
        *dest++ = scmap[cmap[bmap[dot]][thresholds[xphase] < z][dot]];
        xfrac += xstep;
        yfrac += ystep;

        if (++lowx == lowpixelwidth)
        {
            lowx = 0;
            xphase = (xphase + 1) & DITHERMASK;
        }
    }

    dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];
    *dest = scmap[cmap[bmap[dot]][thresholds[xphase] < z][dot]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
}

void R_DrawLowResDitheredRadialSpanWithBrightmap(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    byte            *source = ds_source;
    byte            *bmap = ds_brightmap;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         xfrac = ds_xfrac;
    fixed_t         yfrac = ds_yfrac;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   xstep = ds_xstep;
    const fixed_t   ystep = ds_ystep;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;
    byte            dot;
    const byte      *thresholds = dithermatrix[(ds_y / lowpixelrows) & DITHERMASK];
    const int       x = ds_x1 + ditherxoffset;
    int             lowx = x % lowpixelwidth;
    int             xphase = (x / lowpixelwidth) & DITHERMASK;

    while (--count)
    {
        dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];

        if (bmap[dot])
            *dest++ = scmap[fullcolormap[dot]];
        else
            *dest++ = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])][dot]];

        xfrac += xstep;
        yfrac += ystep;
        lightxfrac += lightxstep;
        lightyfrac += lightystep;

        if (++lowx == lowpixelwidth)
        {
            lowx = 0;
            xphase = (xphase + 1) & DITHERMASK;
        }
    }

    dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];

    if (bmap[dot])
        *dest = scmap[fullcolormap[dot]];
    else
        *dest = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])][dot]];

    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
}

void R_DrawDitheredSpan(void)
{
    int                 count = ds_x2 - ds_x1;
    byte                *dest = ylookup0[ds_y] + ds_x1;
    const lighttable_t  *cmap[2] = { ds_colormap[0], ds_colormap[1] };
    byte                *source = ds_source;
    byte                *scmap = ds_sectorcolormap;
    fixed_t             xfrac = ds_xfrac;
    fixed_t             yfrac = ds_yfrac;
    const fixed_t       xstep = ds_xstep;
    const fixed_t       ystep = ds_ystep;
    const byte          *thresholds = dithermatrix[ds_y & DITHERMASK];
    const int           z = ds_z;
    int                 xphase = (ds_x1 + ditherxoffset) & DITHERMASK;

    while (--count)
    {
        *dest++ = scmap[cmap[thresholds[xphase] < z]
            [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
        xfrac += xstep;
        yfrac += ystep;
        xphase = (xphase + 1) & DITHERMASK;
    }

    *dest = scmap[cmap[thresholds[xphase] < z]
        [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
}

void R_DrawDitheredRadialSpan(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    byte            *source = ds_source;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         xfrac = ds_xfrac;
    fixed_t         yfrac = ds_yfrac;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   xstep = ds_xstep;
    const fixed_t   ystep = ds_ystep;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;
    const byte      *thresholds = dithermatrix[ds_y & DITHERMASK];
    int             xphase = (ds_x1 + ditherxoffset) & DITHERMASK;

    while (--count)
    {
        *dest++ = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])]
            [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
        xfrac += xstep;
        yfrac += ystep;
        lightxfrac += lightxstep;
        lightyfrac += lightystep;
        xphase = (xphase + 1) & DITHERMASK;
    }

    *dest = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])]
        [source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
}

void R_DrawDitheredSpanWithBrightmap(void)
{
    int                 count = ds_x2 - ds_x1;
    byte                *dest = ylookup0[ds_y] + ds_x1;
    byte                *source = ds_source;
    byte                *bmap = ds_brightmap;
    byte                *scmap = ds_sectorcolormap;
    fixed_t             xfrac = ds_xfrac;
    fixed_t             yfrac = ds_yfrac;
    const fixed_t       xstep = ds_xstep;
    const fixed_t       ystep = ds_ystep;
    byte                dot;
    const byte          *thresholds = dithermatrix[ds_y & DITHERMASK];
    const int           z = ds_z;
    int                 xphase = (ds_x1 + ditherxoffset) & DITHERMASK;
    const lighttable_t  *cmap[2][2] = { { ds_colormap[0], ds_colormap[1] },
                                        { fullcolormap,   fullcolormap   } };

    while (--count)
    {
        dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];
        *dest++ = scmap[cmap[bmap[dot]][thresholds[xphase] < z][dot]];
        xfrac += xstep;
        yfrac += ystep;
        xphase = (xphase + 1) & DITHERMASK;
    }

    dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];
    *dest = scmap[cmap[bmap[dot]][thresholds[xphase] < z][dot]];
    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
}

void R_DrawDitheredRadialSpanWithBrightmap(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    byte            *source = ds_source;
    byte            *bmap = ds_brightmap;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         xfrac = ds_xfrac;
    fixed_t         yfrac = ds_yfrac;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   xstep = ds_xstep;
    const fixed_t   ystep = ds_ystep;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;
    byte            dot;
    const byte      *thresholds = dithermatrix[ds_y & DITHERMASK];
    int             xphase = (ds_x1 + ditherxoffset) & DITHERMASK;

    while (--count)
    {
        dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];

        if (bmap[dot])
            *dest++ = scmap[fullcolormap[dot]];
        else
            *dest++ = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])][dot]];

        xfrac += xstep;
        yfrac += ystep;
        lightxfrac += lightxstep;
        lightyfrac += lightystep;
        xphase = (xphase + 1) & DITHERMASK;
    }

    dot = source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)];

    if (bmap[dot])
        *dest = scmap[fullcolormap[dot]];
    else
        *dest = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])][dot]];

    ds_xfrac = xfrac;
    ds_yfrac = yfrac;
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
}

void R_DrawSolidColorSpan(void)
{
    int         count = ds_x2 - ds_x1;
    byte        *dest = ylookup0[ds_y] + ds_x1;
    const byte  color = ds_sectorcolormap[ds_colormap[0][NOTEXTURECOLOR]];

    while (--count)
        *dest++ = color;

    *dest = color;
}

void R_DrawRadialSolidColorSpan(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;

    while (--count)
    {
        *dest++ = scmap[light[R_GetRadialLightIndex(lightxfrac, lightyfrac)][NOTEXTURECOLOR]];
        lightxfrac += lightxstep;
        lightyfrac += lightystep;
    }

    *dest = scmap[light[R_GetRadialLightIndex(lightxfrac, lightyfrac)][NOTEXTURECOLOR]];
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
}

void R_DrawLowResDitheredSolidColorSpan(void)
{
    int         count = ds_x2 - ds_x1;
    byte        *dest = ylookup0[ds_y] + ds_x1;
    const byte  color[2] = { ds_sectorcolormap[ds_colormap[0][NOTEXTURECOLOR]],
                             ds_sectorcolormap[ds_colormap[1][NOTEXTURECOLOR]] };
    const byte  *thresholds = dithermatrix[(ds_y / lowpixelrows) & DITHERMASK];
    const int   z = ds_z;
    const int   x = ds_x1 + ditherxoffset;
    int         lowx = x % lowpixelwidth;
    int         xphase = (x / lowpixelwidth) & DITHERMASK;

    while (--count)
    {
        *dest++ = color[thresholds[xphase] < z];

        if (++lowx == lowpixelwidth)
        {
            lowx = 0;
            xphase = (xphase + 1) & DITHERMASK;
        }
    }

    *dest = color[thresholds[xphase] < z];
}

void R_DrawLowResDitheredRadialSolidColorSpan(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;
    const byte      *thresholds = dithermatrix[(ds_y / lowpixelrows) & DITHERMASK];
    const int       x = ds_x1 + ditherxoffset;
    int             lowx = x % lowpixelwidth;
    int             xphase = (x / lowpixelwidth) & DITHERMASK;

    while (--count)
    {
        *dest++ = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])]
            [NOTEXTURECOLOR]];
        lightxfrac += lightxstep;
        lightyfrac += lightystep;

        if (++lowx == lowpixelwidth)
        {
            lowx = 0;
            xphase = (xphase + 1) & DITHERMASK;
        }
    }

    *dest = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])]
        [NOTEXTURECOLOR]];
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
}

void R_DrawDitheredSolidColorSpan(void)
{
    int         count = ds_x2 - ds_x1;
    byte        *dest = ylookup0[ds_y] + ds_x1;
    const byte  color[2] = { ds_sectorcolormap[ds_colormap[0][NOTEXTURECOLOR]],
                             ds_sectorcolormap[ds_colormap[1][NOTEXTURECOLOR]] };
    const byte  *thresholds = dithermatrix[ds_y & DITHERMASK];
    const int   z = ds_z;
    int         xphase = (ds_x1 + ditherxoffset) & DITHERMASK;

    while (--count)
    {
        *dest++ = color[thresholds[xphase] < z];
        xphase = (xphase + 1) & DITHERMASK;
    }

    *dest = color[thresholds[xphase] < z];
}

void R_DrawDitheredRadialSolidColorSpan(void)
{
    int             count = ds_x2 - ds_x1;
    byte            *dest = ylookup0[ds_y] + ds_x1;
    lighttable_t    **light = ds_zlight;
    byte            *scmap = ds_sectorcolormap;
    fixed_t         lightxfrac = ds_lightxfrac;
    fixed_t         lightyfrac = ds_lightyfrac;
    const fixed_t   lightxstep = ds_lightxstep;
    const fixed_t   lightystep = ds_lightystep;
    const byte      *thresholds = dithermatrix[ds_y & DITHERMASK];
    int             xphase = (ds_x1 + ditherxoffset) & DITHERMASK;

    while (--count)
    {
        *dest++ = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])]
            [NOTEXTURECOLOR]];
        lightxfrac += lightxstep;
        lightyfrac += lightystep;
        xphase = (xphase + 1) & DITHERMASK;
    }

    *dest = scmap[light[R_GetRadialDitheredLightIndex(lightxfrac, lightyfrac, thresholds[xphase])]
        [NOTEXTURECOLOR]];
    ds_lightxfrac = lightxfrac;
    ds_lightyfrac = lightyfrac;
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

    memset(fuzz1table, 0, MAXSCREENAREA * sizeof(int));
    memset(fuzz2table, 0, MAXSCREENAREA * sizeof(int));

    for (int i = 0; i < 256; i++)
        flipindex[i] = (i < 128 ? i : 126 - (i & 127));
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
