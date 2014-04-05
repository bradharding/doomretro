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

#define _USE_MATH_DEFINES

#include <math.h>
#include "d_net.h"
#include "m_menu.h"
#include "r_local.h"
#include "r_sky.h"
#include "v_video.h"

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW     2048

// increment every time a check is made
int                     validcount = 1;

lighttable_t            *fixedcolormap;
extern lighttable_t     **walllights;

int                     centerx;
int                     centery;

fixed_t                 centerxfrac;
fixed_t                 centeryfrac;
fixed_t                 viewheightfrac;
fixed_t                 projection;
fixed_t                 projectiony;

fixed_t                 viewx;
fixed_t                 viewy;
fixed_t                 viewz;

angle_t                 viewangle;

fixed_t                 viewcos;
fixed_t                 viewsin;

player_t                *viewplayer;

//
// precalculated math tables
//
angle_t                 clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
int                     viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t                 xtoviewangle[SCREENWIDTH + 1];

const fixed_t           *finecosine = &finesine[FINEANGLES / 4];

lighttable_t            *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t            *scalelight2[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t            *scalelightfixed[MAXLIGHTSCALE];
lighttable_t            *zlight[LIGHTLEVELS][MAXLIGHTZ];

// bumped light from gun blasts
int                     extralight;

int                     translucency = true;

extern int              automapactive;

void (*colfunc)(void);
void (*wallcolfunc)(void);
void (*fbwallcolfunc)(byte *);
void (*basecolfunc)(void);
void (*fuzzcolfunc)(void);
void (*tlcolfunc)(void);
void (*tl50colfunc)(void);
void (*tl33colfunc)(void);
void (*tlgreencolfunc)(void);
void (*tlredcolfunc)(void);
void (*tlredwhitecolfunc)(void);
void (*tlbluecolfunc)(void);
void (*tlgreen50colfunc)(void);
void (*tlred50colfunc)(void);
void (*tlredwhite50colfunc)(void);
void (*tlblue50colfunc)(void);
void (*redtobluecolfunc)(void);
void (*transcolfunc)(void);
void (*spanfunc)(void);
void (*skycolfunc)(void);
void (*redtogreencolfunc)(void);
void (*tlredtoblue33colfunc)(void);
void (*tlredtogreen33colfunc)(void);
void (*psprcolfunc)(void);

//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node)
{
    return ((int64_t)(y - node->y) * node->dx + (int64_t)(node->x - x) * node->dy >= 0);
}

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t *line)
{
    return ((int64_t)(line->v2->x - line->v1->x) * (y - line->v1->y)
            - (int64_t)(line->v2->y - line->v1->y) * (x - line->v1->x) >= 0);
}

//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table.
//
angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
    static fixed_t      oldx;
    static fixed_t      oldy;
    static angle_t      oldresult;

    x -= viewx;
    y -= viewy;

    if (oldx != x || oldy != y)
    {
        oldx = x;
        oldy = y;
        oldresult = (int)(atan2((float)y, (float)x) * (ANG180 / M_PI));
    }
    return oldresult;
}

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    viewx = x1;
    viewy = y1;

    return R_PointToAngle(x2, y2);
}

fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
    fixed_t dx = ABS(x - viewx);
    fixed_t dy = ABS(y - viewy);

    if (dy > dx)
    {
        fixed_t t = dx;

        dx = dy;
        dy = t;
    }

    return FixedDiv(dx, finecosine[tantoangle[FixedDiv(dy, dx) >> DBITS] >> ANGLETOFINESHIFT]);
}

//
// R_InitTextureMapping
//
void R_InitTextureMapping(void)
{
    int     i;
    int     x;
    int     t;
    fixed_t focallength;

    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.

    const fixed_t hitan = finetangent[FINEANGLES / 4 + FIELDOFVIEW / 2];
    const fixed_t lotan = finetangent[FINEANGLES / 4 - FIELDOFVIEW / 2];
    const int     highend = viewwidth + 1;

    // Calc focallength
    //  so FIELDOFVIEW angles covers SCREENWIDTH.
    focallength = FixedDiv(centerxfrac, hitan);

    for (i = 0; i < FINEANGLES / 2; i++)
    {
        fixed_t tangent = finetangent[i];

        if (tangent > hitan)
            t = -1;
        else if (tangent < lotan)
            t = highend;
        else
        {
            t = (centerxfrac - FixedMul(tangent, focallength) + FRACUNIT - 1) >> FRACBITS;
            t = MAX(-1, (MIN(t, highend)));
        }
        viewangletox[i] = t;
    }

    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x.
    for (x = 0; x <= viewwidth; x++)
    {
        for (i = 0; viewangletox[i] > x; i++);
        xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
    }

    // Take out the fencepost cases from viewangletox.
    for (i = 0; i < FINEANGLES / 2; i++)
    {
        if (viewangletox[i] == -1)
            viewangletox[i] = 0;
        else if (viewangletox[i] == highend)
            viewangletox[i]--;
    }

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
    int i;

    // Calculate the light levels to use
    //  for each level / distance combination.
    for (i = 0; i < LIGHTLEVELS; i++)
    {
        int j, startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;

        for (j = 0; j < MAXLIGHTZ; j++)
        {
            int scale = FixedDiv(SCREENWIDTH / 2 * FRACUNIT, (j + 1) << LIGHTZSHIFT);
            int level = startmap - (scale >> LIGHTSCALESHIFT) / DISTMAP;

            if (level < 0)
                level = 0;
            else if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;

            zlight[i][j] = colormaps + level * 256;
        }
    }
}

//
// R_SetViewSize
// Do not really change anything here,
//  because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
boolean setsizeneeded;
int     setblocks;


void R_SetViewSize(int blocks)
{
    setsizeneeded = true;
    setblocks = blocks;
}

//
// R_ExecuteSetViewSize
//
void R_ExecuteSetViewSize(void)
{
    fixed_t cosadj;
    fixed_t dy;
    int     i;
    int     j;
    int     level;
    int     startmap;

    setsizeneeded = false;

    if (setblocks == 11)
    {
        scaledviewwidth = SCREENWIDTH;
        viewheight = SCREENHEIGHT;
    }
    else
    {
        scaledviewwidth = setblocks * 64;
        viewheight = ((setblocks * 168 / 10) & ~7) * 2;
    }

    viewwidth = scaledviewwidth;
    viewheightfrac = viewheight << FRACBITS;

    centery = viewheight / 2;
    centerx = viewwidth / 2;
    centerxfrac = centerx << FRACBITS;
    centeryfrac = centery << FRACBITS;
    projection = centerxfrac;
    projectiony = ((SCREENHEIGHT * centerx * ORIGINALWIDTH) / ORIGINALHEIGHT) / SCREENWIDTH * FRACUNIT;

    colfunc = basecolfunc = R_DrawColumn;
    fuzzcolfunc = R_DrawFuzzColumn;

    if (translucency)
    {
        tlcolfunc = R_DrawTranslucentColumn;
        tl50colfunc = R_DrawTranslucent50Column;
        tl33colfunc = R_DrawTranslucent33Column;
        tlgreencolfunc = R_DrawTranslucentGreenColumn;
        tlredcolfunc = R_DrawTranslucentRedColumn;
        tlredwhitecolfunc = R_DrawTranslucentRedWhiteColumn;
        tlbluecolfunc = R_DrawTranslucentBlueColumn;
        tlgreen50colfunc = R_DrawTranslucentGreen50Column;
        tlred50colfunc = R_DrawTranslucentRed50Column;
        tlredwhite50colfunc = R_DrawTranslucentRedWhite50Column;
        tlblue50colfunc = R_DrawTranslucentBlue50Column;
        tlredtoblue33colfunc = R_DrawTranslucentRedToBlue33Column;
        tlredtogreen33colfunc = R_DrawTranslucentRedToGreen33Column;
    }
    else
    {
        tlcolfunc = R_DrawColumn;
        tl50colfunc = R_DrawColumn;
        tl33colfunc = R_DrawColumn;
        tlgreencolfunc = R_DrawColumn;
        tlredcolfunc = R_DrawColumn;
        tlredwhitecolfunc = R_DrawColumn;
        tlbluecolfunc = R_DrawColumn;
        tlgreen50colfunc = R_DrawColumn;
        tlred50colfunc = R_DrawColumn;
        tlredwhite50colfunc = R_DrawColumn;
        tlblue50colfunc = R_DrawColumn;
        tlredtoblue33colfunc = R_DrawColumn;
        tlredtogreen33colfunc = R_DrawColumn;
    }

    transcolfunc = R_DrawTranslatedColumn;
    spanfunc = R_DrawSpan;
    skycolfunc = R_DrawSkyColumn;
    redtobluecolfunc = R_DrawRedToBlueColumn;
    redtogreencolfunc = R_DrawRedToGreenColumn;
    wallcolfunc = R_DrawWallColumn;
    fbwallcolfunc = R_DrawFullbrightWallColumn;
    psprcolfunc = R_DrawPlayerSpriteColumn;

    R_InitBuffer(scaledviewwidth, viewheight);

    R_InitTextureMapping();

    // psprite scales
    pspritexscale = (centerx << FRACBITS) / (ORIGINALWIDTH / 2);
    pspriteyscale = (((SCREENHEIGHT * viewwidth) / SCREENWIDTH) << FRACBITS) / ORIGINALHEIGHT;
    pspriteiscale = FixedDiv(FRACUNIT, pspritexscale);

    // thing clipping
    for (i = 0; i < viewwidth; i++)
        screenheightarray[i] = viewheight;

    // planes
    for (i = 0; i < viewheight; i++)
    {
        dy = ABS(((i - viewheight / 2) << FRACBITS) + FRACUNIT / 2);
        yslope[i] = FixedDiv(projectiony, dy);
    }

    for (i = 0; i < viewwidth; i++)
    {
        cosadj = ABS(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
        distscale[i] = FixedDiv(FRACUNIT, cosadj);
    }

    // Calculate the light levels to use
    //  for each level / scale combination.
    for (i = 0; i < LIGHTLEVELS; i++)
    {
        startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
        for (j = 0; j < MAXLIGHTSCALE; j++)
        {
            level = startmap - j * SCREENWIDTH / (viewwidth * DISTMAP);

            if (level < 0)
                level = 0;
            else if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;

            scalelight[i][j] = colormaps + level * 256;

            // [BH] calculate separate light levels to use when drawing
            //  player's weapon, so it stays consistent regardless of view size
            level = startmap - j / DISTMAP;

            if (level < 0)
                level = 0;
            else if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;

            scalelight2[i][j] = colormaps + level * 256;
        }
    }
}

//
// R_Init
//
void R_Init(void)
{
    R_InitData();

    R_SetViewSize(screenblocks);
    R_InitLightTables();
    R_InitSkyMap();
    R_InitTranslationTables();
}

//
// R_PointInSubsector
//
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
    int    nodenum;

    // single subsector is a special case
    if (!numnodes)
        return subsectors;

    nodenum = numnodes - 1;

    while (!(nodenum & NF_SUBSECTOR))
        nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];

    return &subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_SetupFrame
//
void R_SetupFrame(player_t *player)
{
    int i;

    viewplayer = player;
    viewx = player->mo->x;
    viewy = player->mo->y;
    viewangle = player->mo->angle;
    extralight = player->extralight;

    viewz = player->viewz;

    viewsin = finesine[viewangle >> ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];

    if (player->fixedcolormap)
    {
        fixedcolormap = colormaps + player->fixedcolormap * 256 * sizeof(lighttable_t);

        walllights = scalelightfixed;

        for (i = 0; i < MAXLIGHTSCALE; i++)
            scalelightfixed[i] = fixedcolormap;
    }
    else
        fixedcolormap = 0;

    validcount++;
}

//
// R_RenderView
//
void R_RenderPlayerView(player_t *player)
{
    R_SetupFrame(player);

    // Clear buffers.
    R_ClearClipSegs();
    R_ClearDrawSegs();

    if (automapactive)
    {
        // The head node is the last node output.
        R_RenderBSPNode(numnodes - 1);
    }
    else
    {
        // Clear buffers.
        R_ClearPlanes();
        R_ClearSprites();

        if (player->cheats & CF_NOCLIP)
            V_FillRect(0, viewwindowx, viewwindowy, viewwidth, viewheight, 0);

        // The head node is the last node output.
        R_RenderBSPNode(numnodes - 1);

        R_DrawPlanes();
        R_DrawMasked();
    }
}
