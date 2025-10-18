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

#include <math.h>

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_random.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "r_sky.h"
#include "v_video.h"

// increment every time a check is made
int                 validcount = 1;

lighttable_t        *fixedcolormap;

bool                usebrightmaps;

int                 centerx;
int                 centery;

fixed_t             centerxfrac;
fixed_t             centeryfrac;
fixed_t             projection;

fixed_t             viewx;
fixed_t             viewy;
fixed_t             viewz;

angle_t             viewangle;

fixed_t             viewcos;
fixed_t             viewsin;

fixed_t             viewheightfrac;

player_t            *viewplayer = NULL;

angle_t             clipangle;

// The viewangletox[viewangle + FINEANGLES / 4] lookup
// maps the visible view angles to screen x coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same x.
int                 viewangletox[FINEANGLES / 2];

// The xtoviewangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t             xtoviewangle[MAXWIDTH + 1];

angle_t             linearskyangle[MAXWIDTH + 1];

fixed_t             finesine[5 * FINEANGLES / 4];
fixed_t             *finecosine = &finesine[FINEANGLES / 4];
fixed_t             finetangent[FINEANGLES / 2];
angle_t             tantoangle[SLOPERANGE + 1];

// killough 03/20/98: Support dynamic colormaps, e.g. deep water
// killough 04/04/98: Support dynamic number of them as well
int                 numcolormaps = 1;
static lighttable_t *(*c_scalelight)[LIGHTLEVELS][MAXLIGHTSCALE];
static lighttable_t *(*c_zlight)[LIGHTLEVELS][MAXLIGHTZ];
static lighttable_t *(*c_psprscalelight)[OLDLIGHTLEVELS][OLDMAXLIGHTSCALE];
lighttable_t        *(*scalelight)[MAXLIGHTSCALE];
lighttable_t        *(*psprscalelight)[OLDMAXLIGHTSCALE];
lighttable_t        *(*zlight)[MAXLIGHTZ];
lighttable_t        *fullcolormap;
lighttable_t        **colormaps;

// bumped light from gun blasts
int                 extralight;

bool                drawbloodsplats;

static fixed_t      fovscale;

// [ceski] [JN] Higher than 21:9 aspect ratios require an extended range
// of 'tx' value, used by R_ProjectSprite for sprite projection.
fixed_t             fovtx;

//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node)
{
    const fixed_t   nx = node->x;
    const fixed_t   ny = node->y;
    const int64_t   ndx = node->dx;
    const int64_t   ndy = node->dy;

    if (!ndx)
        return (x <= nx ? (ndy > 0) : (ndy < 0));

    if (!ndy)
        return (y <= ny ? (ndx < 0) : (ndx > 0));

    x -= nx;
    y -= ny;

    // Try to quickly decide by looking at sign bits.
    if ((ndy ^ ndx ^ x ^ y) < 0)
        return ((ndy ^ x) < 0); // (left is negative)

    return (y * ndx >= ndy * x);
}

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t *line)
{
    const fixed_t   lx = line->v1->x;
    const fixed_t   ly = line->v1->y;
    const int64_t   ldx = line->dx;
    const int64_t   ldy = line->dy;

    if (!ldx)
        return (x <= lx ? (ldy > 0) : (ldy < 0));

    if (!ldy)
        return (y <= ly ? (ldx < 0) : (ldx > 0));

    x -= lx;
    y -= ly;

    // Try to quickly decide by looking at sign bits.
    if ((ldy ^ ldx ^ x ^ y) < 0)
        return ((ldy ^ x) < 0); // (left is negative)

    return (y * ldx >= ldy * x);
}

static int SlopeDiv(unsigned int num, unsigned int den)
{
    uint64_t    ans;

    if (den < 512)
        return (ANG45 - 1);

    ans = ((uint64_t)num << 3) / (den >> 8);

    return (int)(ans <= SLOPERANGE ? tantoangle[ans] : ANG45 - 1);
}

//
// R_PointToAngle
// To get a global angle from Cartesian coordinates,
// the coordinates are flipped until they are in the first octant of
// the coordinate system, then the y (<=x) is scaled and divided by x
// to get a tangent (slope) value which is looked up in the
// tantoangle[] table.

// Point of view (viewx, viewy) to point (x, y) angle.
angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
    return R_PointToAngle2(viewx, viewy, x, y);
}

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x, fixed_t y)
{
    x -= x1;
    y -= y1;

    if (!x && !y)
        return 0;

    if (x >= 0)
    {
        if (y >= 0)
            return (x > y ? SlopeDiv(y, x) : ANG90 - 1 - SlopeDiv(x, y));
        else
            return (x > (y = -y) ? -SlopeDiv(y, x) : ANG270 + SlopeDiv(x, y));
    }

    x = -x;

    if (y >= 0)
        return (x > y ? ANG180 - 1 - SlopeDiv(y, x) : ANG90 + SlopeDiv(x, y));
    else
        return (x > (y = -y) ? ANG180 + SlopeDiv(y, x) : ANG270 - 1 - SlopeDiv(x, y));
}

// Point of view (viewx, viewy) to point (x1, y1) angle.
angle_t R_PointToAngleEx(fixed_t x, fixed_t y)
{
    return R_PointToAngleEx2(viewx, viewy, x, y);
}

angle_t R_PointToAngleEx2(fixed_t x1, fixed_t y1, fixed_t x, fixed_t y)
{
    // [crispy] fix overflows for very long distances
    const int64_t   x_viewx = (int64_t)x - x1;
    const int64_t   y_viewy = (int64_t)y - y1;

    // [crispy] the worst that could happen is e.g. INT_MIN - INT_MAX = 2 * INT_MIN
    if (x_viewx < INT_MIN || x_viewx > INT_MAX || y_viewy < INT_MIN || y_viewy > INT_MAX)
    {
        // [crispy] preserving the angle by halving the distance in both directions
        x = (fixed_t)(x_viewx / 2 + x1);
        y = (fixed_t)(y_viewy / 2 + y1);
    }

    return R_PointToAngle2(x1, y1, x, y);
}

// [AM] Interpolate between two angles.
angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle)
{
    if (nangle == oangle)
        return nangle;

    if (nangle > oangle)
    {
        if (nangle - oangle < ANG270)
            return (oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(fractionaltic)));
        else
            return (oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(fractionaltic)));
    }
    else
    {
        if (oangle - nangle < ANG270)
            return (oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(fractionaltic)));
        else
            return (oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(fractionaltic)));
    }
}

//
// R_InitTables
//
static void R_InitTables(void)
{
    // viewangle tangent table
    for (int i = 0; i < FINEANGLES / 2; i++)
        finetangent[i] = (fixed_t)(tan(((double)i - FINEANGLES / 4 + 0.5) * M_PI * 2 / FINEANGLES) * FRACUNIT);

    // finesine table
    for (int i = 0; i < 5 * FINEANGLES / 4; i++)
        finesine[i] = (fixed_t)(sin((i + 0.5) * M_PI * 2 / FINEANGLES) * FRACUNIT);
}

static void R_InitPointToAngle(void)
{
    // slope (tangent) to angle lookup
    for (int i = 0; i <= SLOPERANGE; i++)
        tantoangle[i] = (angle_t)(0xFFFFFFFF * atan((i + 0.5) / SLOPERANGE) / (M_PI * 2));
}

//
// R_InitTextureMapping
//
static void R_InitTextureMapping(void)
{
    // Calc focallength so field of view angles covers SCREENWIDTH.
    const fixed_t   focallength = FixedDiv(centerxfrac, fovscale);

    for (int i = 0; i < FINEANGLES / 2; i++)
    {
        const fixed_t   tangent = finetangent[i];

        if (tangent > fovscale)
            viewangletox[i] = -1;
        else if (tangent < -fovscale)
            viewangletox[i] = viewwidth + 1;
        else
            viewangletox[i] = BETWEEN(-1, (centerxfrac - FixedMul(tangent, focallength)
                + FRACUNIT - 1) >> FRACBITS, viewwidth + 1);
    }

    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle that maps to x.
    for (int i, x = 0; x <= viewwidth; x++)
    {
        for (i = 0; viewangletox[i] > x; i++);

        xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;

        // [crispy] calculate sky angle for drawing horizontally linear skies.
        // Taken from GZDoom and refactored for integer math.
        linearskyangle[x] = (viewwidth / 2 - x) * ((SCREENWIDTH << FRACBITS) / viewwidth)
            * (ANG90 / (NONWIDEWIDTH << FRACBITS));
    }

    // Take out the fencepost cases from viewangletox.
    for (int i = 0; i < FINEANGLES / 2; i++)
        if (viewangletox[i] == -1)
            viewangletox[i] = 0;
        else if (viewangletox[i] == viewwidth + 1)
            viewangletox[i]--;

    clipangle = xtoviewangle[0];
}

//
// R_InitLightTables
//
static void R_InitLightTables(void)
{
    const unsigned int  wfovscale = FixedMul(SCREENWIDTH, FixedDiv(FRACUNIT, fovscale)) + 1;
    const int           width = wfovscale / 2 * FRACUNIT;

    // Calculate the light levels to use for each level/distance combination.
    for (int i = 0; i < LIGHTLEVELS; i++)
    {
        const int   start = ((LIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;

        for (int j = 0; j < MAXLIGHTZ; j++)
        {
            const int   scale = FixedDiv(width, (j + 1) << LIGHTZSHIFT) >> LIGHTSCALESHIFT;
            const int   level = BETWEEN(0, start - scale / 2, NUMCOLORMAPS - 1) * 256;

            // killough 03/20/98: Initialize multiple colormaps
            for (int t = 0; t < numcolormaps; t++)
                c_zlight[t][i][j] = &colormaps[t][level];
        }
    }
}

//
// R_SetViewSize
// Do not really change anything here, because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
bool        setsizeneeded;
static int  setblocks;

void R_SetViewSize(int blocks)
{
    setsizeneeded = true;
    setblocks = blocks + 3;
}

//
// R_ExecuteSetViewSize
//
void R_ExecuteSetViewSize(void)
{
    fixed_t num;

    setsizeneeded = false;

    if (setblocks == 11)
    {
        viewwidth = SCREENWIDTH;
        viewheight = SCREENHEIGHT;
        viewwindowx = 0;
        viewwindowy = 0;
        pspritescale = FixedDiv(NONWIDEWIDTH, VANILLAWIDTH);
    }
    else
    {
        viewwidth = setblocks * SCREENWIDTH / 10;
        viewheight = ((setblocks * (SCREENHEIGHT - SBARHEIGHT) / 10) & ~7);
        viewwindowx = (SCREENWIDTH - viewwidth) / 2;
        viewwindowy = (SCREENHEIGHT - SBARHEIGHT - viewheight) / 2;
        pspritescale = FixedDiv(setblocks * NONWIDEWIDTH / 10, VANILLAWIDTH);
    }

    centerx = viewwidth / 2;
    centerxfrac = centerx << FRACBITS;
    fovscale = finetangent[FINEANGLES / 4 + ((menuactive && !helpscreen && menuspin ? r_fov_max : r_fov)
        + WIDEFOVDELTA) * FINEANGLES / 360 / 2];
    fovtx = (vid_aspectratio >= vid_aspectratio_21_9 && r_fov >= 90 ? 4 : 2);
    projection = FixedDiv(centerxfrac, fovscale);
    viewheightfrac = viewheight << (FRACBITS + 2);

    R_InitBuffer();
    R_InitTextureMapping();
    R_InitLightTables();

    pspriteiscale = FixedDiv(FRACUNIT, pspritescale);

    if (gamestate == GS_LEVEL)
    {
        suppresswarnings = true;
        R_InitSkyMap();
        suppresswarnings = false;
    }

    // thing clipping
    for (int i = 0; i < viewwidth; i++)
        viewheightarray[i] = viewheight;

    // planes
    num = FixedMul(FixedDiv(FRACUNIT, fovscale), viewwidth * FRACUNIT / 2);

    for (int i = 0; i < viewheight; i++)
        for (int j = 0; j < LOOKDIRS; j++)
            yslopes[j][i] = FixedDiv(num, ABS(((i - (viewheight / 2 + (j - LOOKDIRMAX) * 2
                * setblocks / 10)) << FRACBITS) + FRACUNIT / 2));

    yslope = yslopes[LOOKDIRMAX];

    // Calculate the light levels to use for each level/scale combination.
    for (int i = 0; i < LIGHTLEVELS; i++)
    {
        const int   start = ((LIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;

        for (int j = 0; j < MAXLIGHTSCALE; j++)
        {
            const int   level = BETWEEN(0, start - j * SCREENWIDTH / (viewwidth * 2), NUMCOLORMAPS - 1) * 256;

            // killough 03/20/98: initialize multiple colormaps
            for (int k = 0; k < numcolormaps; k++)
                c_scalelight[k][i][j] = &colormaps[k][level];
        }
    }

    // [BH] calculate separate light levels to use when drawing player's weapon,
    // so it stays consistent regardless of view size.
    for (int i = 0; i < OLDLIGHTLEVELS; i++)
    {
        const int   start = ((OLDLIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / OLDLIGHTLEVELS;

        for (int j = 0; j < OLDMAXLIGHTSCALE; j++)
        {
            const int   level = BETWEEN(0, start - j / 2, NUMCOLORMAPS - 1) * 256;

            for (int k = 0; k < numcolormaps; k++)
                c_psprscalelight[k][i][j] = &colormaps[k][level];
        }
    }

    AM_SetAutomapSize(r_screensize);
}

void (*colfunc)(void);
void (*wallcolfunc)(void);
void (*altwallcolfunc)(void);
void (*missingcolfunc)(void);
void (*bmapwallcolfunc)(void);
void (*altbmapwallcolfunc)(void);
void (*segcolfunc)(void);
void (*bmapsegcolfunc)(void);
void (*translatedcolfunc)(void);
void (*basecolfunc)(void);
void (*tlcolfunc)(void);
void (*tl50colfunc)(void);
void (*tl50segcolfunc)(void);
void (*tl50bmapsegcolfunc)(void);
void (*tl33colfunc)(void);
void (*tlgreencolfunc)(void);
void (*tlredcolfunc)(void);
void (*tlredwhitecolfunc1)(void);
void (*tlredwhitecolfunc2)(void);
void (*tlredwhite50colfunc)(void);
void (*tlbluecolfunc)(void);
void (*tlgreen33colfunc)(void);
void (*tlred33colfunc)(void);
void (*tlblue25colfunc)(void);
void (*skycolfunc)(void);
void (*psprcolfunc)(void);
void (*spanfunc)(void);
void (*altspanfunc)(void);
void (*bloodcolfunc)(void);
void (*bloodsplatcolfunc)(void);

void R_UpdateMobjColfunc(mobj_t *mobj)
{
    const int   flags = mobj->flags;

    if (flags & MF_FUZZ)
    {
        if (r_textures)
        {
            if (mobj->type == MT_BLOOD && r_blood != r_blood_all)
            {
                if (r_sprites_translucency)
                {
                    mobj->colfunc = &R_DrawTranslucent33Column;
                    mobj->altcolfunc = &R_DrawTranslucent33Column;
                }
                else
                {
                    mobj->colfunc = &R_DrawColumn;
                    mobj->altcolfunc = &R_DrawColumn;
                }
            }
            else
            {
                mobj->colfunc = &R_DrawFuzzColumn;
                mobj->altcolfunc = &R_DrawFuzzColumn;
            }
        }
        else if (r_sprites_translucency)
        {
            mobj->colfunc = &R_DrawTranslucent50SolidColorColumn;
            mobj->altcolfunc = &R_DrawTranslucent50SolidColorColumn;
        }
        else
        {
            mobj->colfunc = &R_DrawSolidColorColumn;
            mobj->altcolfunc = &R_DrawSolidColorColumn;
        }
    }
    else if (flags & MF_TRANSLUCENT)
    {
        mobj->colfunc = tl50colfunc;
        mobj->altcolfunc = tl50colfunc;
    }
    else
    {
        const int   flags2 = mobj->flags2;

        if (flags2 & MF2_TRANSLUCENT)
        {
            mobj->colfunc = tlcolfunc;
            mobj->altcolfunc = tl50colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_REDONLY)
        {
            mobj->colfunc = tlredcolfunc;
            mobj->altcolfunc = tlred33colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_GREENONLY)
        {
            mobj->colfunc = tlgreencolfunc;
            mobj->altcolfunc = tlgreen33colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_BLUEONLY)
        {
            mobj->colfunc = tlbluecolfunc;
            mobj->altcolfunc = tlblue25colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_33)
        {
            mobj->colfunc = tl33colfunc;
            mobj->altcolfunc = tl33colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_50)
        {
            mobj->colfunc = tl50colfunc;
            mobj->altcolfunc = tl50colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_BLUE_25)
        {
            mobj->colfunc = tlblue25colfunc;
            mobj->altcolfunc = tlblue25colfunc;
        }
        else
        {
            mobj->colfunc = basecolfunc;
            mobj->altcolfunc = basecolfunc;
        }
    }
}

void R_InitColumnFunctions(void)
{
    if (r_textures)
    {
        skycolfunc = (canmodify && !transferredsky && (gamemode != commercial || gamemap < 21) && !canfreelook ?
            &R_DrawFlippedSkyColumn : &R_DrawWallColumn);

        if (r_ditheredlighting)
        {
            if (r_detail == r_detail_low)
            {
                basecolfunc = &R_DrawLowResDitheredColumn;
                translatedcolfunc = &R_DrawDitherLowTranslatedColumn;
                wallcolfunc = &R_DrawLowResDitheredWallColumn;
                missingcolfunc = &R_DrawLowResDitheredSolidColorColumn;
                bmapwallcolfunc = &R_DrawLowResDitheredWallColumnWithBrightmap;
                segcolfunc = &R_DrawLowResDitheredColumn;
                bmapsegcolfunc = &R_DrawLowResDitheredColumnWithBrightmap;
                tl50segcolfunc = (r_textures_translucency ? &R_DrawLowResDitheredTranslucent50Column : &R_DrawLowResDitheredColumn);
                tl50bmapsegcolfunc = (r_textures_translucency ? &R_DrawLowResDitheredTranslucent50ColumnWithBrightmap : &R_DrawLowResDitheredColumnWithBrightmap);
                spanfunc = &R_DrawLowResDitheredSpan;
            }
            else
            {
                basecolfunc = &R_DrawDitheredColumn;
                translatedcolfunc = &R_DrawDitherTranslatedColumn;
                wallcolfunc = &R_DrawDitheredWallColumn;
                missingcolfunc = &R_DrawDitheredSolidColorColumn;
                bmapwallcolfunc = &R_DrawDitheredWallColumnWithBrightmap;
                segcolfunc = &R_DrawDitheredColumn;
                bmapsegcolfunc = &R_DrawDitheredColumnWithBrightmap;
                tl50segcolfunc = (r_textures_translucency ? &R_DrawDitheredTranslucent50Column : &R_DrawDitheredColumn);
                tl50bmapsegcolfunc = (r_textures_translucency ? &R_DrawDitheredTranslucent50ColumnWithBrightmap : &R_DrawDitheredColumnWithBrightmap);
                spanfunc = &R_DrawDitheredSpan;
            }

            altwallcolfunc = &R_DrawWallColumn;
            altbmapwallcolfunc = &R_DrawWallColumnWithBrightmap;
            altspanfunc = &R_DrawSpan;

            if (r_sprites_translucency)
            {
                tlcolfunc = &R_DrawTranslucentColumn;
                tl50colfunc = &R_DrawTranslucent50Column;
                tl33colfunc = &R_DrawTranslucent33Column;

                if (incompatiblepalette)
                {
                    if (r_detail == r_detail_low)
                    {
                        tlgreencolfunc = &R_DrawLowResDitheredColumn;
                        tlredcolfunc = &R_DrawLowResDitheredColumn;
                        tlredwhitecolfunc1 = &R_DrawLowResDitheredColumn;
                        tlredwhitecolfunc2 = &R_DrawLowResDitheredColumn;
                        tlredwhite50colfunc = &R_DrawLowResDitheredColumn;
                        tlbluecolfunc = &R_DrawLowResDitheredColumn;
                        tlgreen33colfunc = &R_DrawLowResDitheredColumn;
                        tlred33colfunc = &R_DrawLowResDitheredColumn;
                        tlblue25colfunc = &R_DrawLowResDitheredColumn;
                    }
                    else
                    {
                        tlgreencolfunc = &R_DrawDitheredColumn;
                        tlredcolfunc = &R_DrawDitheredColumn;
                        tlredwhitecolfunc1 = &R_DrawDitheredColumn;
                        tlredwhitecolfunc2 = &R_DrawDitheredColumn;
                        tlredwhite50colfunc = &R_DrawDitheredColumn;
                        tlbluecolfunc = &R_DrawDitheredColumn;
                        tlgreen33colfunc = &R_DrawDitheredColumn;
                        tlred33colfunc = &R_DrawDitheredColumn;
                        tlblue25colfunc = &R_DrawDitheredColumn;
                    }
                }
                else
                {
                    tlgreencolfunc = &R_DrawTranslucentGreenColumn;
                    tlredcolfunc = &R_DrawTranslucentRedColumn;
                    tlredwhitecolfunc1 = &R_DrawTranslucentRedWhiteColumn1;
                    tlredwhitecolfunc2 = &R_DrawTranslucentRedWhiteColumn2;
                    tlredwhite50colfunc = &R_DrawTranslucentRedWhite50Column;
                    tlbluecolfunc = &R_DrawTranslucentBlueColumn;
                    tlgreen33colfunc = &R_DrawTranslucentGreen33Column;
                    tlred33colfunc = &R_DrawTranslucentRed33Column;
                    tlblue25colfunc = &R_DrawTranslucentBlue25Column;
                }
            }
            else
            {
                if (r_detail == r_detail_low)
                {
                    tlcolfunc = &R_DrawLowResDitheredColumn;
                    tl50colfunc = &R_DrawLowResDitheredColumn;
                    tl33colfunc = &R_DrawLowResDitheredColumn;
                    tlgreencolfunc = &R_DrawLowResDitheredColumn;
                    tlredcolfunc = &R_DrawLowResDitheredColumn;
                    tlredwhitecolfunc1 = &R_DrawLowResDitheredColumn;
                    tlredwhitecolfunc2 = &R_DrawLowResDitheredColumn;
                    tlredwhite50colfunc = &R_DrawLowResDitheredColumn;
                    tlbluecolfunc = &R_DrawLowResDitheredColumn;
                    tlgreen33colfunc = &R_DrawLowResDitheredColumn;
                    tlred33colfunc = &R_DrawLowResDitheredColumn;
                    tlblue25colfunc = &R_DrawLowResDitheredColumn;
                }
                else
                {
                    tlcolfunc = &R_DrawDitheredColumn;
                    tl50colfunc = &R_DrawDitheredColumn;
                    tl33colfunc = &R_DrawDitheredColumn;
                    tlgreencolfunc = &R_DrawDitheredColumn;
                    tlredcolfunc = &R_DrawDitheredColumn;
                    tlredwhitecolfunc1 = &R_DrawDitheredColumn;
                    tlredwhitecolfunc2 = &R_DrawDitheredColumn;
                    tlredwhite50colfunc = &R_DrawDitheredColumn;
                    tlbluecolfunc = &R_DrawDitheredColumn;
                    tlgreen33colfunc = &R_DrawDitheredColumn;
                    tlred33colfunc = &R_DrawDitheredColumn;
                    tlblue25colfunc = &R_DrawDitheredColumn;
                }
            }
        }
        else
        {
            basecolfunc = &R_DrawColumn;
            translatedcolfunc = &R_DrawTranslatedColumn;
            wallcolfunc = &R_DrawWallColumn;
            altwallcolfunc = &R_DrawWallColumn;
            missingcolfunc = &R_DrawSolidColorColumn;
            bmapwallcolfunc = &R_DrawWallColumnWithBrightmap;
            altbmapwallcolfunc = &R_DrawWallColumnWithBrightmap;
            segcolfunc = &R_DrawColumn;
            bmapsegcolfunc = &R_DrawColumnWithBrightmap;
            tl50segcolfunc = (r_textures_translucency ? &R_DrawTranslucent50Column : &R_DrawColumn);
            tl50bmapsegcolfunc = (r_textures_translucency ? &R_DrawTranslucent50ColumnWithBrightmap : &R_DrawColumnWithBrightmap);
            spanfunc = &R_DrawSpan;
            altspanfunc = &R_DrawSpan;

            if (r_sprites_translucency)
            {
                tlcolfunc = &R_DrawTranslucentColumn;
                tl50colfunc = &R_DrawTranslucent50Column;
                tl33colfunc = &R_DrawTranslucent33Column;

                if (incompatiblepalette)
                {
                    tlgreencolfunc = &R_DrawColumn;
                    tlredcolfunc = &R_DrawColumn;
                    tlredwhitecolfunc1 = &R_DrawColumn;
                    tlredwhitecolfunc2 = &R_DrawColumn;
                    tlredwhite50colfunc = &R_DrawColumn;
                    tlbluecolfunc = &R_DrawColumn;
                    tlgreen33colfunc = &R_DrawColumn;
                    tlred33colfunc = &R_DrawColumn;
                    tlblue25colfunc = &R_DrawColumn;
                }
                else
                {
                    tlgreencolfunc = &R_DrawTranslucentGreenColumn;
                    tlredcolfunc = &R_DrawTranslucentRedColumn;
                    tlredwhitecolfunc1 = &R_DrawTranslucentRedWhiteColumn1;
                    tlredwhitecolfunc2 = &R_DrawTranslucentRedWhiteColumn2;
                    tlredwhite50colfunc = &R_DrawTranslucentRedWhite50Column;
                    tlbluecolfunc = &R_DrawTranslucentBlueColumn;
                    tlgreen33colfunc = &R_DrawTranslucentGreen33Column;
                    tlred33colfunc = &R_DrawTranslucentRed33Column;
                    tlblue25colfunc = &R_DrawTranslucentBlue25Column;
                }
            }
            else
            {
                tlcolfunc = &R_DrawColumn;
                tl50colfunc = &R_DrawColumn;
                tl33colfunc = &R_DrawColumn;
                tlgreencolfunc = &R_DrawColumn;
                tlredcolfunc = &R_DrawColumn;
                tlredwhitecolfunc1 = &R_DrawColumn;
                tlredwhitecolfunc2 = &R_DrawColumn;
                tlredwhite50colfunc = &R_DrawColumn;
                tlbluecolfunc = &R_DrawColumn;
                tlgreen33colfunc = &R_DrawColumn;
                tlred33colfunc = &R_DrawColumn;
                tlblue25colfunc = &R_DrawColumn;
            }
        }

        bloodcolfunc = (r_sprites_translucency ? &R_DrawTranslucentBloodColumn : &R_DrawTranslatedColumn);
        bloodsplatcolfunc = (r_bloodsplats_translucency ? &R_DrawBloodSplatColumn : &R_DrawSolidBloodSplatColumn);
        psprcolfunc = &R_DrawPlayerSpriteColumn;
    }
    else
    {
        skycolfunc = &R_DrawSolidColorColumn;

        if (r_ditheredlighting)
        {
            if (r_detail == r_detail_low)
            {
                basecolfunc = &R_DrawLowResDitheredSolidColorColumn;
                translatedcolfunc = &R_DrawLowResDitheredSolidColorColumn;
                wallcolfunc = &R_DrawLowResDitheredSolidColorColumn;
                missingcolfunc = &R_DrawLowResDitheredSolidColorColumn;
                bmapwallcolfunc = &R_DrawLowResDitheredSolidColorColumn;
                segcolfunc = &R_DrawLowResDitheredSolidColorColumn;
                bmapsegcolfunc = &R_DrawLowResDitheredSolidColorColumn;
                tl50segcolfunc = (r_textures_translucency ? &R_DrawLowResDitheredTranslucent50SolidColorColumn : &R_DrawLowResDitheredSolidColorColumn);
                tl50bmapsegcolfunc = (r_textures_translucency ? &R_DrawLowResDitheredTranslucent50SolidColorColumn : &R_DrawLowResDitheredSolidColorColumn);
                spanfunc = &R_DrawLowResDitheredSolidColorSpan;
                tlcolfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawLowResDitheredSolidColorColumn);
                tl50colfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawLowResDitheredSolidColorColumn);
                tl33colfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawLowResDitheredSolidColorColumn);
                tlgreencolfunc = &R_DrawLowResDitheredSolidColorColumn;
                tlredcolfunc = &R_DrawLowResDitheredSolidColorColumn;
                tlredwhitecolfunc1 = &R_DrawLowResDitheredSolidColorColumn;
                tlredwhitecolfunc2 = &R_DrawLowResDitheredSolidColorColumn;
                tlredwhite50colfunc = &R_DrawLowResDitheredSolidColorColumn;
                tlbluecolfunc = &R_DrawLowResDitheredSolidColorColumn;
                tlgreen33colfunc = &R_DrawLowResDitheredSolidColorColumn;
                tlred33colfunc = &R_DrawLowResDitheredSolidColorColumn;
                tlblue25colfunc = &R_DrawLowResDitheredSolidColorColumn;
            }
            else
            {
                basecolfunc = &R_DrawDitheredSolidColorColumn;
                translatedcolfunc = &R_DrawDitheredSolidColorColumn;
                wallcolfunc = &R_DrawDitheredSolidColorColumn;
                missingcolfunc = &R_DrawDitheredSolidColorColumn;
                bmapwallcolfunc = &R_DrawDitheredSolidColorColumn;
                segcolfunc = &R_DrawDitheredSolidColorColumn;
                bmapsegcolfunc = &R_DrawDitheredSolidColorColumn;
                tl50segcolfunc = (r_textures_translucency ? &R_DrawDitheredTranslucent50SolidColorColumn : &R_DrawDitheredSolidColorColumn);
                tl50bmapsegcolfunc = (r_textures_translucency ? &R_DrawDitheredTranslucent50SolidColorColumn : &R_DrawDitheredSolidColorColumn);
                spanfunc = &R_DrawDitheredSolidColorSpan;
                tlcolfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawDitheredSolidColorColumn);
                tl50colfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawDitheredSolidColorColumn);
                tl33colfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawDitheredSolidColorColumn);
                tlgreencolfunc = &R_DrawDitheredSolidColorColumn;
                tlredcolfunc = &R_DrawDitheredSolidColorColumn;
                tlredwhitecolfunc1 = &R_DrawDitheredSolidColorColumn;
                tlredwhitecolfunc2 = &R_DrawDitheredSolidColorColumn;
                tlredwhite50colfunc = &R_DrawDitheredSolidColorColumn;
                tlbluecolfunc = &R_DrawDitheredSolidColorColumn;
                tlgreen33colfunc = &R_DrawDitheredSolidColorColumn;
                tlred33colfunc = &R_DrawDitheredSolidColorColumn;
                tlblue25colfunc = &R_DrawDitheredSolidColorColumn;
            }
        }
        else
        {
            basecolfunc = &R_DrawSolidColorColumn;
            translatedcolfunc = &R_DrawSolidColorColumn;
            wallcolfunc = &R_DrawSolidColorColumn;
            missingcolfunc = &R_DrawSolidColorColumn;
            bmapwallcolfunc = &R_DrawSolidColorColumn;
            segcolfunc = &R_DrawSolidColorColumn;
            bmapsegcolfunc = &R_DrawSolidColorColumn;
            tl50segcolfunc = (r_textures_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawSolidColorColumn);
            tl50bmapsegcolfunc = (r_textures_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawSolidColorColumn);
            spanfunc = &R_DrawSolidColorSpan;
            tlcolfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawSolidColorColumn);
            tl50colfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawSolidColorColumn);
            tl33colfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawSolidColorColumn);
            tlgreencolfunc = &R_DrawSolidColorColumn;
            tlredcolfunc = &R_DrawSolidColorColumn;
            tlredwhitecolfunc1 = &R_DrawSolidColorColumn;
            tlredwhitecolfunc2 = &R_DrawSolidColorColumn;
            tlredwhite50colfunc = &R_DrawSolidColorColumn;
            tlbluecolfunc = &R_DrawSolidColorColumn;
            tlgreen33colfunc = &R_DrawSolidColorColumn;
            tlred33colfunc = &R_DrawSolidColorColumn;
            tlblue25colfunc = &R_DrawSolidColorColumn;
        }

        bloodcolfunc = (r_sprites_translucency ? &R_DrawTranslucent50SolidColorColumn : &R_DrawSolidColorColumn);
        bloodsplatcolfunc = (r_bloodsplats_translucency ? &R_DrawBloodSplatColumn : &R_DrawSolidBloodSplatColumn);
        psprcolfunc = &R_DrawSolidColorColumn;
        altwallcolfunc = &R_DrawSolidColorColumn;
        altbmapwallcolfunc = &R_DrawSolidColorColumn;
        altspanfunc = &R_DrawSolidColorSpan;
    }

    for (mobjtype_t i = 0; i < nummobjtypes; i++)
    {
        mobjinfo_t  *info = &mobjinfo[i];
        const int   flags = info->flags;
        const int   flags2 = info->flags2;

        if (flags & MF_FUZZ)
        {
            if (r_textures)
            {
                info->colfunc = &R_DrawFuzzColumn;
                info->altcolfunc = &R_DrawFuzzColumn;
            }
            else if (r_sprites_translucency)
            {
                info->colfunc = &R_DrawTranslucent50SolidColorColumn;
                info->altcolfunc = &R_DrawTranslucent50SolidColorColumn;
            }
            else
            {
                info->colfunc = &R_DrawSolidColorColumn;
                info->altcolfunc = &R_DrawSolidColorColumn;
            }
        }
        else if (flags & MF_TRANSLUCENT)
        {
            info->colfunc = tl50colfunc;
            info->altcolfunc = tl50colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT)
        {
            info->colfunc = tlcolfunc;
            info->altcolfunc = tl50colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_REDONLY)
        {
            info->colfunc = tlredcolfunc;
            info->altcolfunc = tlred33colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_GREENONLY)
        {
            info->colfunc = tlgreencolfunc;
            info->altcolfunc = tlgreen33colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_BLUEONLY)
        {
            info->colfunc = tlbluecolfunc;
            info->altcolfunc = tlblue25colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_33)
        {
            info->colfunc = tl33colfunc;
            info->altcolfunc = tl33colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_50)
        {
            info->colfunc = tl50colfunc;
            info->altcolfunc = tl50colfunc;
        }
        else if (flags2 & MF2_TRANSLUCENT_BLUE_25)
        {
            info->colfunc = tlblue25colfunc;
            info->altcolfunc = tlblue25colfunc;
        }
        else
        {
            info->colfunc = basecolfunc;
            info->altcolfunc = basecolfunc;
        }
    }

    if (r_ditheredlighting)
    {
        if (!norockettrails)
        {
            if (r_rockettrails_translucency)
            {
                mobjinfo[MT_TRAIL].colfunc = &R_DrawCorrectedTranslucent50Column;
                mobjinfo[MT_TRAIL].altcolfunc = &R_DrawCorrectedTranslucent50Column;
            }
            else
            {
                if (r_detail == r_detail_low)
                {
                    mobjinfo[MT_TRAIL].colfunc = &R_DrawCorrectedLowResDitheredColumn;
                    mobjinfo[MT_TRAIL].altcolfunc = &R_DrawCorrectedLowResDitheredColumn;
                }
                else
                {
                    mobjinfo[MT_TRAIL].colfunc = &R_DrawCorrectedDitheredColumn;
                    mobjinfo[MT_TRAIL].altcolfunc = &R_DrawCorrectedDitheredColumn;
                }
            }
        }

        if (!mobjinfo[MT_DOGS].dehacked)
        {
            if (r_detail == r_detail_low)
            {
                mobjinfo[MT_DOGS].colfunc = &R_DrawCorrectedLowResDitheredColumn;
                mobjinfo[MT_DOGS].altcolfunc = &R_DrawCorrectedLowResDitheredColumn;
            }
            else
            {
                mobjinfo[MT_DOGS].colfunc = &R_DrawCorrectedDitheredColumn;
                mobjinfo[MT_DOGS].altcolfunc = &R_DrawCorrectedDitheredColumn;
            }
        }
    }
    else
    {
        if (!norockettrails)
        {
            if (r_rockettrails_translucency)
            {
                mobjinfo[MT_TRAIL].colfunc = &R_DrawCorrectedTranslucent50Column;
                mobjinfo[MT_TRAIL].altcolfunc = &R_DrawCorrectedTranslucent50Column;
            }
            else
            {
                mobjinfo[MT_TRAIL].colfunc = &R_DrawCorrectedColumn;
                mobjinfo[MT_TRAIL].altcolfunc = &R_DrawCorrectedColumn;
            }
        }

        if (!mobjinfo[MT_DOGS].dehacked)
        {
            mobjinfo[MT_DOGS].colfunc = &R_DrawCorrectedColumn;
            mobjinfo[MT_DOGS].altcolfunc = &R_DrawCorrectedColumn;
        }
    }

    if (gamestate == GS_LEVEL)
        for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
        {
            mobj_t  *mo = (mobj_t *)th;

            mo->colfunc = mo->info->colfunc;
            mo->altcolfunc = mo->info->altcolfunc;
        }
}

//
// R_Init
//
void R_Init(void)
{
    R_InitClipSegs();
    R_InitData();
    R_InitPointToAngle();
    R_InitTables();
    R_SetViewSize(r_screensize);

    c_zlight = malloc(numcolormaps * sizeof(*c_zlight));
    c_scalelight = malloc(numcolormaps * sizeof(*c_scalelight));
    c_psprscalelight = malloc(numcolormaps * sizeof(*c_psprscalelight));

    R_InitLightTables();
    R_InitTranslationTables();
    R_InitPatches();
    R_InitDistortedFlats();
    R_InitColumnFunctions();
}

//
// R_PointInSubsector
//
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
    // single subsector is a special case
    if (!numnodes)
        return subsectors;
    else
    {
        int nodenum = numnodes - 1;

        while (!(nodenum & NF_SUBSECTOR))
        {
            node_t  *node = nodes + nodenum;

            nodenum = node->children[R_PointOnSide(x, y, node)];
        }

        return (subsectors + (nodenum & ~NF_SUBSECTOR));
    }
}

//
// R_SetupFrame
//
static void R_SetupFrame(void)
{
    int     colormap = 0;
    mobj_t  *mo = viewplayer->mo;
    int     pitch = 0;

    // [AM] Interpolate the player camera if the feature is enabled.
    if (vid_capfps != TICRATE
        // Don't interpolate if the player did something that would necessitate turning it off for a tic.
        && mo->interpolate
        // Don't interpolate during a paused state
        && !menuactive && !consoleactive && !paused)
    {
        // Interpolate player camera from their old position to their current one.
        viewx = mo->oldx + FixedMul(mo->x - mo->oldx, fractionaltic);
        viewy = mo->oldy + FixedMul(mo->y - mo->oldy, fractionaltic);
        viewz = viewplayer->oldviewz + FixedMul(viewplayer->viewz - viewplayer->oldviewz, fractionaltic);
        viewangle = R_InterpolateAngle(mo->oldangle, mo->angle);

        if (canfreelook)
            pitch = (viewplayer->oldlookdir + (int)((viewplayer->lookdir - viewplayer->oldlookdir)
                * FIXED2DOUBLE(fractionaltic))) / MLOOKUNIT;

        if (weaponrecoil)
            pitch = BETWEEN(-LOOKDIRMAX, pitch + viewplayer->oldrecoil
                + FixedMul(viewplayer->recoil - viewplayer->oldrecoil, fractionaltic), LOOKDIRMAX);
    }
    else
    {
        viewx = mo->x;
        viewy = mo->y;
        viewz = viewplayer->viewz;
        viewangle = mo->angle;

        if (canfreelook)
            pitch = viewplayer->lookdir / MLOOKUNIT;

        if (weaponrecoil)
            pitch = BETWEEN(-LOOKDIRMAX, pitch + viewplayer->recoil, LOOKDIRMAX);
    }

    if (shake && !menuactive && !consoleactive && !paused)
    {
        const uint64_t  time = I_GetTimeMS();

        if (shake > time && shakeduration)
        {
            const fixed_t   amount = FRACUNIT * (fixed_t)(shake - time) / shakeduration;

            viewx += M_RandomInt(-3, 3) * amount;
            viewy += M_RandomInt(-3, 3) * amount;
            viewz += M_RandomInt(-2, 2) * amount;
        }
    }

    if (automapactive)
        return;

    centery = viewheight / 2;

    if (pitch)
        centery += pitch * 2 * (r_screensize + 3) / 10;

    extralight = (viewplayer->extralight << 2) + r_extralighting / 3;

    centeryfrac = centery << FRACBITS;
    yslope = yslopes[LOOKDIRMAX + pitch];

    viewsin = finesine[viewangle >> ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];

    // killough 03/20/98, 04/04/98: select colormap based on player status
    if (mo->subsector->sector->heightsec)
    {
        const sector_t  *s = mo->subsector->sector->heightsec;

        colormap = (viewz < s->interpfloorheight ? s->bottommap :
            (viewz > s->interpceilingheight ? s->topmap : s->midmap));

        if (colormap < 0 || colormap > numcolormaps)
            colormap = 0;
    }

    fullcolormap = colormaps[colormap];
    zlight = c_zlight[colormap];
    scalelight = c_scalelight[colormap];
    psprscalelight = c_psprscalelight[colormap];
    drawbloodsplats = (r_blood != r_blood_none && r_bloodsplats_max);

    if (viewplayer->fixedcolormap && r_textures)
    {
        // killough 03/20/98: localize scalelightfixed (readability/optimization)
        static lighttable_t *scalelightfixed[MAXLIGHTSCALE];

        // killough 03/20/98: use fullcolormap
        fixedcolormap = fullcolormap;

        if (viewplayer->fixedcolormap == INVERSECOLORMAP)
            fixedcolormap += (size_t)32 * 256 * sizeof(lighttable_t);

        usebrightmaps = false;

        for (int i = 0; i < MAXLIGHTSCALE; i++)
            scalelightfixed[i] = fixedcolormap;
    }
    else
    {
        fixedcolormap = 0;
        usebrightmaps = (r_brightmaps && !colormap);
    }

    validcount++;
}

//
// R_RenderPlayerView
//
void R_RenderPlayerView(void)
{
    R_SetupFrame();

    // Clear buffers.
    R_ClearClipSegs();
    R_ClearDrawSegs();
    R_ClearPlanes();
    R_ClearSprites();

    if (automapactive)
    {
        R_RenderBSPNode(numnodes - 1);
        return;
    }

    if (r_homindicator)
        V_FillRect(0, viewwindowx, viewwindowy, viewwidth, viewheight,
            (maptime % 20 < 9 ? nearestred : (viewplayer->fixedcolormap == INVERSECOLORMAP ?
                colormaps[0][32 * 256 + WHITE] : nearestblack)), 0, false, false, NULL, NULL);
    else if ((viewplayer->cheats & CF_NOCLIP) || freeze)
        V_FillRect(0, viewwindowx, viewwindowy, viewwidth, viewheight,
            (viewplayer->fixedcolormap == INVERSECOLORMAP ? colormaps[0][32 * 256 + WHITE] : nearestblack),
            0, false, false, NULL, NULL);

    R_RenderBSPNode(numnodes - 1);  // head node is the last node output

    R_DrawNearbySprites();

    R_DrawPlanes();

    R_DrawMasked();

    if (!r_textures && viewplayer->fixedcolormap == INVERSECOLORMAP)
        V_InvertScreen();
}
