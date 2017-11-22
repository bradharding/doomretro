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

#include "c_console.h"
#include "doomstat.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_random.h"
#include "p_local.h"
#include "r_sky.h"
#include "v_video.h"

#define BLACK       0
#define RED         176

// increment every time a check is made
int                 validcount = 1;

lighttable_t        *fixedcolormap;

dboolean            usebrightmaps;

int                 centerx;
int                 centery;

fixed_t             centerxfrac;
fixed_t             centeryfrac;
fixed_t             projection;

fixed_t             fovscale = FRACUNIT;

fixed_t             viewx;
fixed_t             viewy;
fixed_t             viewz;

angle_t             viewangle;

fixed_t             viewcos;
fixed_t             viewsin;

player_t            *viewplayer;

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0). Used for interpolation.
fixed_t             fractionaltic;

//
// precalculated math tables
//
angle_t             clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
int                 viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t             xtoviewangle[SCREENWIDTH + 1];

fixed_t             finesine[5 * FINEANGLES / 4];
fixed_t             *finecosine = &finesine[FINEANGLES / 4];
fixed_t             finetangent[FINEANGLES / 2];
angle_t             tantoangle[SLOPERANGE + 1];

// killough 3/20/98: Support dynamic colormaps, e.g. deep water
// killough 4/4/98: support dynamic number of them as well
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

dboolean            drawbloodsplats;

dboolean            r_bloodsplats_translucency = r_bloodsplats_translucency_default;
dboolean            r_dither = r_dither_default;
int                 r_fov = r_fov_default;
dboolean            r_homindicator = r_homindicator_default;
dboolean            r_shadows_translucency = r_shadows_translucency_default;
dboolean            r_shake_barrels = r_shake_barrels_default;
int                 r_skycolor = r_skycolor_default;
dboolean            r_textures = r_textures_default;
dboolean            r_translucency = r_translucency_default;

extern dboolean     canmodify;
extern dboolean     canmouselook;
extern int          barreltics;
extern dboolean     transferredsky;
extern dboolean     vanilla;
extern lighttable_t **walllights;

//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node)
{
    return ((int)(((int64_t)(y - node->y) * node->dx + (int64_t)(node->x - x) * node->dy) >> 32) > 0);
}

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t *line)
{
    return ((int)(((int64_t)(line->v2->x - line->v1->x) * (y - line->v1->y)
        - (int64_t)(line->v2->y - line->v1->y) * (x - line->v1->x)) >> 32) > 0);
}

static int SlopeDiv(unsigned int num, unsigned int den)
{
    uint64_t    ans;

    if (den < 512)
        return (ANG45 - 1);

    ans = ((uint64_t)num << 3) / (den >> 8);
    return (int)(ans <= SLOPERANGE ? tantoangle[ans] : (ANG45 - 1));
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
        {
            y = -y;
            return (x > y ? -SlopeDiv(y, x) : ANG270 + SlopeDiv(x, y));
        }
    }
    else
    {
        x = -x;

        if (y >= 0)
            return (x > y ? ANG180 - 1 - SlopeDiv(y, x) : ANG90 + SlopeDiv(x, y));
        else
        {
            y = -y;
            return (x > y ? ANG180 + SlopeDiv(y, x) : ANG270 - 1 - SlopeDiv(x, y));
        }
    }
}

// Point of view (viewx, viewy) to point (x1, y1) angle.
angle_t R_PointToAngleEx(fixed_t x, fixed_t y)
{
    return R_PointToAngleEx2(viewx, viewy, x, y);
}

angle_t R_PointToAngleEx2(fixed_t x1, fixed_t y1, fixed_t x, fixed_t y)
{
    // [crispy] fix overflows for very long distances
    const int64_t   y_viewy = (int64_t)y - y1;
    const int64_t   x_viewx = (int64_t)x - x1;

    // [crispy] the worst that could happen is e.g. INT_MIN - INT_MAX = 2 * INT_MIN
    if (x_viewx < INT_MIN || x_viewx > INT_MAX || y_viewy < INT_MIN || y_viewy > INT_MAX)
    {
        // [crispy] preserving the angle by halving the distance in both directions
        x = (int)(x_viewx / 2 + x1);
        y = (int)(y_viewy / 2 + y1);
    }

    return R_PointToAngle2(x1, y1, x, y);
}

// [AM] Interpolate between two angles.
static angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle, fixed_t scale)
{
    if (nangle == oangle)
        return nangle;
    else if (nangle > oangle)
    {
        if (nangle - oangle < ANG270)
            return (oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(scale)));
        else
            return (oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(scale)));   // Wrapped around
    }
    else
    {
        if (oangle - nangle < ANG270)
            return (oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(scale)));
        else
            return (oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(scale)));   // Wrapped around
    }
}

//
// R_InitTables
//
static void R_InitTables(void)
{
    // viewangle tangent table
    for (int i = 0; i < FINEANGLES / 2; i++)
        finetangent[i] = (fixed_t)(FRACUNIT * tan((i - FINEANGLES / 4 + 0.5) * M_PI * 2 / FINEANGLES));

    // finesine table
    for (int i = 0; i < 5 * FINEANGLES / 4; i++)
        finesine[i] = (fixed_t)(FRACUNIT * sin((i + 0.5) * M_PI * 2 / FINEANGLES));
}

static void R_InitPointToAngle(void)
{
    // slope (tangent) to angle lookup
    for (int i = 0; i <= SLOPERANGE; i++)
        tantoangle[i] = (angle_t)(0xFFFFFFFF * atan2((double)i, (double)SLOPERANGE) / (M_PI * 2));
}

//
// R_InitTextureMapping
//
static void R_InitTextureMapping(void)
{
    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.
    const fixed_t   limit = finetangent[FINEANGLES / 4 + (r_fov * FINEANGLES / 360) / 2];

    // Calc focallength
    //  so field of view angles covers SCREENWIDTH.
    const fixed_t   focallength = FixedDiv(centerxfrac, limit);

    for (int i = 0; i < FINEANGLES / 2; i++)
    {
        const fixed_t   tangent = finetangent[i];

        if (tangent > limit)
            viewangletox[i] = -1;
        else if (tangent < -limit)
            viewangletox[i] = viewwidth + 1;
        else
            viewangletox[i] = BETWEEN(-1, (centerxfrac - FixedMul(tangent, focallength)
                + FRACUNIT - 1) >> FRACBITS, viewwidth + 1);
    }

    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x.
    for (int i, x = 0; x <= viewwidth; x++)
    {
        for (i = 0; viewangletox[i] > x; i++);

        xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
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
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP 2

void R_InitLightTables(void)
{
    int width = FixedMul(SCREENWIDTH, FixedDiv(FRACUNIT, finetangent[FINEANGLES / 4 + (r_fov * FINEANGLES / 360) / 2])) + 1;

    if (!c_zlight)
    {
        c_zlight = malloc(sizeof(*c_zlight) * numcolormaps);
        c_scalelight = malloc(sizeof(*c_scalelight) * numcolormaps);
        c_psprscalelight = malloc(sizeof(*c_psprscalelight) * numcolormaps);
    }

    // Calculate the light levels to use
    //  for each level / distance combination.
    for (int i = 0; i < LIGHTLEVELS; i++)
    {
        const int   startmap = ((LIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;

        for (int j = 0; j < MAXLIGHTZ; j++)
        {
            const int   scale = FixedDiv(width / 2 * FRACUNIT, (j + 1) << LIGHTZSHIFT) >> LIGHTSCALESHIFT;
            const int   level = BETWEEN(0, startmap - scale / DISTMAP, NUMCOLORMAPS - 1) * 256;

            // killough 3/20/98: Initialize multiple colormaps
            for (int t = 0; t < numcolormaps; t++)
                c_zlight[t][i][j] = &colormaps[t][level];
        }
    }
}

//
// R_SetViewSize
// Do not really change anything here,
//  because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
dboolean    setsizeneeded;
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
    setsizeneeded = false;

    if (setblocks == 11)
    {
        scaledviewwidth = SCREENWIDTH;
        viewheight = SCREENHEIGHT;
    }
    else
    {
        scaledviewwidth = setblocks * SCREENWIDTH / 10;
        viewheight = (setblocks * (SCREENHEIGHT - SBARHEIGHT) / 10) & ~7;
    }

    viewwidth = scaledviewwidth;

    centerx = viewwidth / 2;
    centery = viewheight / 2;
    centerxfrac = centerx << FRACBITS;
    centeryfrac = centery << FRACBITS;
    fovscale = finetangent[FINEANGLES / 4 + (r_fov * FINEANGLES / 360) / 2];
    projection = FixedDiv(centerxfrac, fovscale);

    R_InitBuffer(scaledviewwidth, viewheight);
    R_InitTextureMapping();

    // psprite scales
    pspritescale = FixedDiv(viewwidth, ORIGINALWIDTH);
    pspriteiscale = FixedDiv(FRACUNIT, pspritescale);

    R_InitSkyMap();

    // thing clipping
    for (int i = 0; i < viewwidth; i++)
        viewheightarray[i] = viewheight;

    // planes
    for (int i = 0; i < viewheight; i++)
    {
        const fixed_t   num = FixedMul(FixedDiv(FRACUNIT, fovscale), viewwidth * (FRACUNIT / 2));

        for (int j = 0; j < LOOKDIRS; j++)
            yslopes[j][i] = FixedDiv(num, ABS(((i - (viewheight / 2 + (j - LOOKDIRMAX) * 2
                * (r_screensize + 3) / 10)) << FRACBITS) + FRACUNIT / 2));
    }

    yslope = yslopes[LOOKDIRMAX];

    // Calculate the light levels to use
    //  for each level / scale combination.
    for (int i = 0; i < LIGHTLEVELS; i++)
    {
        const int   startmap = ((LIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;

        for (int j = 0; j < MAXLIGHTSCALE; j++)
        {
            const int   level = BETWEEN(0, startmap - j * SCREENWIDTH / (viewwidth * DISTMAP), NUMCOLORMAPS - 1) * 256;

            // killough 3/20/98: initialize multiple colormaps
            for (int t = 0; t < numcolormaps; t++)
                c_scalelight[t][i][j] = &colormaps[t][level];
        }
    }

    // [BH] calculate separate light levels to use when drawing
    //  player's weapon, so it stays consistent regardless of view size
    for (int i = 0; i < OLDLIGHTLEVELS; i++)
    {
        const int   startmap = ((OLDLIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / OLDLIGHTLEVELS;

        for (int j = 0; j < OLDMAXLIGHTSCALE; j++)
        {
            const int   level = BETWEEN(0, startmap - j / DISTMAP, NUMCOLORMAPS - 1) * 256;

            for (int t = 0; t < numcolormaps; t++)
                c_psprscalelight[t][i][j] = &colormaps[t][level];
        }
    }
}

void R_InitColumnFunctions(void)
{
    if (r_textures)
    {
        basecolfunc = R_DrawColumn;
        fuzzcolfunc = R_DrawFuzzColumn;
        transcolfunc = R_DrawTranslatedColumn;
        wallcolfunc = R_DrawWallColumn;
        fbwallcolfunc = R_DrawFullbrightWallColumn;

        if (r_skycolor != r_skycolor_default)
            skycolfunc = R_DrawSkyColorColumn;
        else
            skycolfunc = (canmodify && !transferredsky && (gamemode != commercial || gamemap < 21)
                && !canmouselook ? R_DrawFlippedSkyColumn : R_DrawSkyColumn);

        spanfunc = R_DrawSpan;

        if (r_translucency)
        {
            tlcolfunc = R_DrawTranslucentColumn;
            tl50colfunc = R_DrawTranslucent50Column;
            tl33colfunc = R_DrawTranslucent33Column;
            tlgreencolfunc = R_DrawTranslucentGreenColumn;
            tlredcolfunc = R_DrawTranslucentRedColumn;
            tlredwhitecolfunc1 = R_DrawTranslucentRedWhiteColumn1;
            tlredwhitecolfunc2 = R_DrawTranslucentRedWhiteColumn2;
            tlredwhite50colfunc = R_DrawTranslucentRedWhite50Column;
            tlbluecolfunc = R_DrawTranslucentBlueColumn;
            tlgreen33colfunc = R_DrawTranslucentGreen33Column;
            tlred33colfunc = R_DrawTranslucentRed33Column;
            tlblue25colfunc = R_DrawTranslucentBlue25Column;
            tlredtoblue33colfunc = R_DrawTranslucentRedToBlue33Column;
            tlredtogreen33colfunc = R_DrawTranslucentRedToGreen33Column;
            megaspherecolfunc = R_DrawMegaSphereColumn;
            supershotguncolfunc = R_DrawTranslucentSuperShotgunColumn;
        }
        else
        {
            tlcolfunc = R_DrawColumn;
            tl50colfunc = R_DrawColumn;
            tl33colfunc = R_DrawColumn;
            tlgreencolfunc = R_DrawColumn;
            tlredcolfunc = R_DrawColumn;
            tlredwhitecolfunc1 = R_DrawColumn;
            tlredwhitecolfunc2 = R_DrawColumn;
            tlredwhite50colfunc = R_DrawColumn;
            tlbluecolfunc = R_DrawColumn;
            tlgreen33colfunc = R_DrawColumn;
            tlred33colfunc = R_DrawColumn;
            tlblue25colfunc = R_DrawColumn;
            tlredtoblue33colfunc = R_DrawRedToBlueColumn;
            tlredtogreen33colfunc = R_DrawRedToGreenColumn;
            megaspherecolfunc = R_DrawSolidMegaSphereColumn;
            supershotguncolfunc = R_DrawSuperShotgunColumn;
        }

        bloodsplatcolfunc = (r_bloodsplats_translucency ? R_DrawBloodSplatColumn : R_DrawSolidBloodSplatColumn);
        redtobluecolfunc = R_DrawRedToBlueColumn;
        redtogreencolfunc = R_DrawRedToGreenColumn;
        psprcolfunc = R_DrawPlayerSpriteColumn;
    }
    else
    {
        basecolfunc = R_DrawColorColumn;
        fuzzcolfunc = R_DrawColorColumn;
        transcolfunc = R_DrawColorColumn;
        wallcolfunc = R_DrawColorColumn;
        fbwallcolfunc = R_DrawColorColumn;
        skycolfunc = (r_skycolor == r_skycolor_default ? R_DrawColorColumn : R_DrawSkyColorColumn);
        spanfunc = R_DrawColorSpan;
        tlcolfunc = R_DrawColorColumn;
        tl50colfunc = R_DrawColorColumn;
        tl33colfunc = R_DrawColorColumn;
        tlgreencolfunc = R_DrawColorColumn;
        tlredcolfunc = R_DrawColorColumn;
        tlredwhitecolfunc1 = R_DrawColorColumn;
        tlredwhitecolfunc2 = R_DrawColorColumn;
        tlredwhite50colfunc = R_DrawColorColumn;
        tlbluecolfunc = R_DrawColorColumn;
        tlgreen33colfunc = R_DrawColorColumn;
        tlred33colfunc = R_DrawColorColumn;
        tlblue25colfunc = R_DrawColorColumn;
        tlredtoblue33colfunc = R_DrawColorColumn;
        tlredtogreen33colfunc = R_DrawColorColumn;
        bloodsplatcolfunc = R_DrawColorColumn;
        megaspherecolfunc = R_DrawColorColumn;
        supershotguncolfunc = R_DrawColorColumn;
        redtobluecolfunc = R_DrawColorColumn;
        redtogreencolfunc = R_DrawColorColumn;
        psprcolfunc = R_DrawColorColumn;
    }

    for (int i = 0; i < NUMMOBJTYPES; i++)
    {
        mobjinfo_t  *info = &mobjinfo[i];
        const int   flags2 = info->flags2;

        if (flags2 & MF2_TRANSLUCENT)
            info->colfunc = tlcolfunc;
        else if (info->doomednum == MegaSphere && !hacx)
            info->colfunc = megaspherecolfunc;
        else if (info->flags & MF_FUZZ)
            info->colfunc = fuzzcolfunc;
        else if (flags2 & MF2_TRANSLUCENT_REDONLY)
            info->colfunc = tlredcolfunc;
        else if (flags2 & MF2_TRANSLUCENT_GREENONLY)
            info->colfunc = tlgreencolfunc;
        else if (flags2 & MF2_TRANSLUCENT_BLUEONLY)
            info->colfunc = tlbluecolfunc;
        else if (flags2 & MF2_TRANSLUCENT_33)
            info->colfunc = tl33colfunc;
        else if ((info->flags & MF_TRANSLUCENT) || (flags2 & MF2_TRANSLUCENT_50))
            info->colfunc = tl50colfunc;
        else if (flags2 & MF2_TRANSLUCENT_REDWHITEONLY)
            info->colfunc = tlredwhitecolfunc1;
        else if (flags2 & MF2_TRANSLUCENT_REDTOGREEN_33)
            info->colfunc = tlredtogreen33colfunc;
        else if (flags2 & MF2_TRANSLUCENT_REDTOBLUE_33)
            info->colfunc = tlredtoblue33colfunc;
        else if (flags2 & MF2_TRANSLUCENT_BLUE_25)
            info->colfunc = tlblue25colfunc;
        else if (flags2 & MF2_REDTOGREEN)
            info->colfunc = redtogreencolfunc;
        else if (flags2 & MF2_REDTOBLUE)
            info->colfunc = redtobluecolfunc;
        else
            info->colfunc = basecolfunc;
    }

    if (chex)
        mobjinfo[MT_BLOOD].blood = GREENBLOOD;
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
    R_InitLightTables();
    R_InitTranslationTables();
    R_InitPatches();
    R_InitColumnFunctions();
}

//
// R_PointInSubsector
//
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
    int nodenum;

    // single subsector is a special case
    if (!numnodes)
        return subsectors;

    nodenum = numnodes - 1;

    while (!(nodenum & NF_SUBSECTOR))
    {
        node_t  *node = nodes + nodenum;

        nodenum = node->children[R_PointOnSide(x, y, node)];
    }

    return (subsectors + (nodenum & ~NF_SUBSECTOR));
}

//
// R_SetupFrame
//
static void R_SetupFrame(player_t *player)
{
    int     cm = 0;
    mobj_t  *mo = player->mo;
    int     tempCentery = viewheight / 2;
    int     pitch = 0;

    viewplayer = player;
    mo->flags2 |= MF2_DONTDRAW;

    // [AM] Interpolate the player camera if the feature is enabled.

    // Figure out how far into the current tic we're in as a fixed_t
    if (vid_capfps != TICRATE)
        fractionaltic = I_GetTimeMS() * TICRATE % 1000 * FRACUNIT / 1000;

    if (vid_capfps != TICRATE
        // Don't interpolate on the first tic of a level, otherwise
        // oldviewz might be garbage.
        && leveltime > 1
        // Don't interpolate if the player did something
        // that would necessitate turning it off for a tic.
        && mo->interpolate
        // Don't interpolate during a paused state
        && !paused && !menuactive && !consoleactive)
    {
        // Interpolate player camera from their old position to their current one.
        viewx = mo->oldx + FixedMul(mo->x - mo->oldx, fractionaltic);
        viewy = mo->oldy + FixedMul(mo->y - mo->oldy, fractionaltic);
        viewz = player->oldviewz + FixedMul(player->viewz - player->oldviewz, fractionaltic);
        viewangle = R_InterpolateAngle(mo->oldangle, mo->angle, fractionaltic);

        if (canmouselook)
        {
            pitch = (player->oldlookdir + (int)((player->lookdir - player->oldlookdir)
                * FIXED2DOUBLE(fractionaltic))) / MLOOKUNIT;

            if (weaponrecoil)
                pitch = BETWEEN(-LOOKDIRMAX, pitch + player->oldrecoil + FixedMul(player->recoil
                    - player->oldrecoil, fractionaltic), LOOKDIRMAX);

            tempCentery += (pitch << 1) * (r_screensize + 3) / 10;
        }
    }
    else
    {
        viewx = mo->x;
        viewy = mo->y;
        viewz = player->viewz;
        viewangle = mo->angle;

        if (canmouselook)
        {
            pitch = player->lookdir / MLOOKUNIT;

            if (weaponrecoil)
                pitch = BETWEEN(-LOOKDIRMAX, pitch + player->recoil, LOOKDIRMAX);

            tempCentery += (pitch << 1) * (r_screensize + 3) / 10;
        }
    }

    if (barreltics && !consoleactive && !menuactive && !paused)
    {
        viewx += M_RandomInt(-2, 2) * FRACUNIT;
        viewy += M_RandomInt(-2, 2) * FRACUNIT;
        barreltics--;
    }

    extralight = player->extralight << 2;

    if (centery != tempCentery)
    {
        centery = tempCentery;
        centeryfrac = centery << FRACBITS;
        yslope = yslopes[LOOKDIRMAX + pitch];
    }

    viewsin = finesine[viewangle >> ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];

    // killough 3/20/98, 4/4/98: select colormap based on player status
    if (mo->subsector->sector->heightsec)
    {
        const sector_t  *s = mo->subsector->sector->heightsec;

        cm = (viewz < s->interpfloorheight ? s->bottommap : (viewz > s->interpceilingheight ? s->topmap : s->midmap));

        if (cm < 0 || cm > numcolormaps)
            cm = 0;
    }

    fullcolormap = colormaps[cm];
    zlight = c_zlight[cm];
    scalelight = c_scalelight[cm];
    psprscalelight = c_psprscalelight[cm];
    drawbloodsplats = (r_blood != r_blood_none && r_bloodsplats_max && !vanilla);

    if (player->fixedcolormap)
    {
        // killough 3/20/98: localize scalelightfixed (readability/optimization)
        static lighttable_t *scalelightfixed[MAXLIGHTSCALE];

        // killough 3/20/98: use fullcolormap
        fixedcolormap = fullcolormap + player->fixedcolormap * 256 * sizeof(lighttable_t);

        usebrightmaps = false;
        walllights = scalelightfixed;

        for (int i = 0; i < MAXLIGHTSCALE; i++)
            scalelightfixed[i] = fixedcolormap;
    }
    else
    {
        fixedcolormap = 0;
        usebrightmaps = (r_brightmaps && !cm && !BTSX);
    }

    validcount++;
}

//
// R_RenderPlayerView
//
void R_RenderPlayerView(player_t *player)
{
    R_SetupFrame(player);

    // Clear buffers.
    R_ClearClipSegs();
    R_ClearDrawSegs();
    R_ClearPlanes();
    R_ClearSprites();

    if (automapactive)
        R_RenderBSPNode(numnodes - 1);
    else
    {
        if (r_homindicator)
            V_FillRect(0, viewwindowx, viewwindowy, viewwidth, viewheight, ((activetic % 20) < 9 ? RED : BLACK));
        else if ((player->cheats & CF_NOCLIP) || freeze)
            V_FillRect(0, viewwindowx, viewwindowy, viewwidth, viewheight, BLACK);

        R_RenderBSPNode(numnodes - 1);  // head node is the last node output
        R_DrawPlanes();
        R_DrawMasked();
    }
}
