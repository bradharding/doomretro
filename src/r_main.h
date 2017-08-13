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

#if !defined(__R_MAIN_H__)
#define __R_MAIN_H__

#include "r_data.h"

//
// POV related.
//
extern fixed_t          viewcos;
extern fixed_t          viewsin;

extern int              viewwindowx;
extern int              viewwindowy;

extern int              centerx;
extern int              centery;

extern fixed_t          centerxfrac;
extern fixed_t          centeryfrac;
extern fixed_t          projectiony;

extern int              validcount;

//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.
#define LIGHTLEVELS             128
#define LIGHTSEGSHIFT           1
#define LIGHTBRIGHT             2
#define MAXLIGHTSCALE           384
#define LIGHTSCALESHIFT         12
#define MAXLIGHTZ               1024
#define LIGHTZSHIFT             17

#define OLDLIGHTLEVELS          32
#define OLDLIGHTSEGSHIFT        3
#define OLDMAXLIGHTSCALE        48

// killough 3/20/98: Allow colormaps to be dynamic (e.g. underwater)
extern lighttable_t     *(*scalelight)[MAXLIGHTSCALE];
extern lighttable_t     *(*zlight)[MAXLIGHTZ];
extern lighttable_t     *(*psprscalelight)[OLDMAXLIGHTSCALE];
extern lighttable_t     *fullcolormap;
extern int              numcolormaps;   // killough 4/4/98: dynamic number of maps
extern lighttable_t     **colormaps;

extern int              extralight;
extern lighttable_t     *fixedcolormap;

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS    32

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0). Used for interpolation.
extern fixed_t          fractionaltic;

//
// Function pointers to switch refresh/drawing functions.
// Used to select shadow mode etc.
//
void (*colfunc)(void);
void (*wallcolfunc)(void);
void (*fbwallcolfunc)(void);
void (*transcolfunc)(void);
void (*basecolfunc)(void);
void (*fuzzcolfunc)(void);
void (*tlcolfunc)(void);
void (*tl50colfunc)(void);
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
void (*redtobluecolfunc)(void);
void (*tlredtoblue33colfunc)(void);
void (*skycolfunc)(void);
void (*redtogreencolfunc)(void);
void (*tlredtogreen33colfunc)(void);
void (*psprcolfunc)(void);
void (*spanfunc)(void);
void (*bloodsplatcolfunc)(void);
void (*megaspherecolfunc)(void);
void (*supershotguncolfunc)(void);

//
// Utility functions.
int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node);

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t *line);

angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x, fixed_t y);
angle_t R_PointToAngleEx(fixed_t x, fixed_t y);
angle_t R_PointToAngleEx2(fixed_t x1, fixed_t y1, fixed_t x, fixed_t y);

fixed_t R_PointToDist(fixed_t x, fixed_t y);

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);

//
// REFRESH - the actual rendering functions.
//

// Called by G_Drawer.
void R_RenderPlayerView(player_t *player);

// Called by startup code.
void R_Init(void);

// Called by M_Responder.
void R_SetViewSize(int blocks);
void R_InitColumnFunctions(void);

#endif
