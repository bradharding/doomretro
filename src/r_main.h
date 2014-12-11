/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#ifndef __R_MAIN__
#define __R_MAIN__

#include "d_player.h"
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
extern fixed_t          projection;
extern fixed_t          projectiony;

extern int              validcount;

extern int              linecount;
extern int              loopcount;

//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.
#define LIGHTLEVELS             32
#define LIGHTSEGSHIFT           3
#define LIGHTBRIGHT             2

#define MAXLIGHTSCALE           48
#define LIGHTSCALESHIFT         13
#define MAXLIGHTZ               128
#define LIGHTZSHIFT             20

extern lighttable_t     *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t     *psprscalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t     *scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t     *zlight[LIGHTLEVELS][MAXLIGHTZ];

extern int              extralight;
extern lighttable_t     *fixedcolormap;

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS            32

//
// Function pointers to switch refresh/drawing functions.
// Used to select shadow mode etc.
//
extern void (*colfunc)(void);
extern void (*wallcolfunc)(void);
extern void (*fbwallcolfunc)(void);
extern void (*transcolfunc)(void);
extern void (*basecolfunc)(void);
extern void (*fuzzcolfunc)(void);
extern void (*tlcolfunc)(void);
extern void (*tl50colfunc)(void);
extern void (*tl33colfunc)(void);
extern void (*tlgreencolfunc)(void);
extern void (*tlredcolfunc)(void);
extern void (*tlredwhitecolfunc)(void);
extern void (*tlredwhite50colfunc)(void);
extern void (*tlbluecolfunc)(void);
extern void (*tlgreen50colfunc)(void);
extern void (*tlred50colfunc)(void);
extern void (*tlblue50colfunc)(void);
extern void (*redtobluecolfunc)(void);
extern void (*tlredtoblue33colfunc)(void);
extern void (*skycolfunc)(void);
extern void (*redtogreencolfunc)(void);
extern void (*tlredtogreen33colfunc)(void);
extern void (*psprcolfunc)(void);
extern void (*spanfunc)(void);
extern void (*bloodsplatcolfunc)(void);

//
// Utility functions.
int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node);

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t *line);

angle_t R_PointToAngle(fixed_t x, fixed_t y);

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

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

#endif
