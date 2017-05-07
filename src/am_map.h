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

#if !defined(__AM_MAP_H__)
#define __AM_MAP_H__

#include "d_event.h"
#include "m_fixed.h"

#define MAPBITS         12
#define FRACTOMAPBITS   (FRACBITS - MAPBITS)

// Used by ST StatusBar stuff.
#define AM_MSGENTERED   1
#define AM_MSGEXITED    0

typedef struct
{
    fixed_t x, y;
} mpoint_t;

// Called by main loop.
dboolean AM_Responder(event_t *ev);

// Called by main loop.
void AM_Ticker(void);

// Called by main loop,
// called instead of view drawer if automap active.
void AM_Drawer(void);
void AM_clearFB(void);

void AM_Start(dboolean mainwindow);

// Called to force the automap to quit
// if the level is completed while it is up.
void AM_Stop(void);

void AM_Init(void);
void AM_setColors(void);
void AM_getGridSize(void);
void AM_addToPath(void);

extern dboolean message_dontfuckwithme;
extern dboolean message_external;

extern int      gamepadwait;

extern int      viewheight2;

extern mpoint_t *markpoints;
extern int      markpointnum;
extern int      markpointnum_max;

extern dboolean am_path;
extern mpoint_t *pathpoints;
extern int      pathpointnum;
extern int      pathpointnum_max;

dboolean keystate(int key);

typedef struct
{
    fixed_t     centerx;
    fixed_t     centery;
    fixed_t     sin;
    fixed_t     cos;
    fixed_t     bbox[4];
} am_frame_t;

#endif
