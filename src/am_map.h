/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#pragma once

#include "d_event.h"
#include "m_fixed.h"

#define MAPBITS         12
#define FRACTOMAPBITS   (FRACBITS - MAPBITS)
#define NUMBREADCRUMBS  1024

typedef struct
{
    fixed_t x, y;
} mpoint_t;

// Called by main loop.
bool AM_Responder(const event_t *ev);

// Called by main loop.
void AM_Ticker(void);

// Called by main loop, called instead of view drawer if automap active.
void AM_Drawer(void);
void AM_ClearFB(void);

void AM_Start(const bool mainwindow);
void AM_ClearMarks(void);
void AM_ToggleFollowMode(bool value);
void AM_ToggleGrid(void);
void AM_AddMark(void);
void AM_ToggleRotateMode(bool value);
void AM_ToggleMaxZoom(void);

// Called to force the automap to quit if the level is completed while it is up.
void AM_Stop(void);

void AM_SetAutomapSize(int screensize);

void AM_Init(void);
void AM_SetColors(void);
void AM_GetGridSize(void);
void AM_AddToPath(void);

typedef struct
{
    mpoint_t    center;
    fixed_t     sin;
    fixed_t     cos;
    fixed_t     bbox[4];
} am_frame_t;

extern int          lastlevel;
extern int          lastepisode;

extern mpoint_t     *markpoints;
extern int          markpointnum;
extern int          markpointnum_max;

extern mpoint_t     *breadcrumb;
extern int          numbreadcrumbs;
extern int          maxbreadcrumbs;

extern am_frame_t   am_frame;
extern int          direction;

bool keystate(int key);
