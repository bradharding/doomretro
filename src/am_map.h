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

#if !defined(__AM_MAP__)
#define __AM_MAP__

#include "d_event.h"
#include "m_cheat.h"
#include "m_fixed.h"

#define MAPBITS         12
#define FRACTOMAPBITS   (FRACBITS - MAPBITS)

// Used by ST StatusBar stuff.
#define AM_MSGENTERED   1
#define AM_MSGEXITED    0

typedef struct
{
    fixed_t     x, y;
} mpoint_t;

// Called by main loop.
dboolean AM_Responder(event_t *ev);

// Called by main loop.
void AM_Ticker(void);

// Called by main loop,
// called instead of view drawer if automap active.
void AM_Drawer(void);

void AM_Start(void);

// Called to force the automap to quit
// if the level is completed while it is up.
void AM_Stop(void);

void AM_Init(void);

void D_PostEvent(event_t *ev);

extern int      key_up;
extern int      key_up2;
extern int      key_down;
extern int      key_down2;
extern int      key_right;
extern int      key_straferight;
extern int      key_straferight2;
extern int      key_left;
extern int      key_strafeleft;
extern int      key_strafeleft2;

extern int      key_automap;
extern int      key_automap_clearmark;
extern int      key_automap_followmode;
extern int      key_automap_grid;
extern int      key_automap_mark;
extern int      key_automap_maxzoom;
extern int      key_automap_rotatemode;
extern int      key_automap_zoomin;
extern int      key_automap_zoomout;

extern int      gamepadautomap;
extern int      gamepadautomapclearmark;
extern int      gamepadautomapfollowmode;
extern int      gamepadautomapgrid;
extern int      gamepadautomapmark;
extern int      gamepadautomapmaxzoom;
extern int      gamepadautomaprotatemode;
extern int      gamepadautomapzoomin;
extern int      gamepadautomapzoomout;

extern byte     *tinttab60;
extern byte     *tinttab80;

extern dboolean message_dontfuckwithme;
extern dboolean message_clearable;
extern dboolean idbehold;
extern dboolean idclev;
extern dboolean idmus;

extern int      viewheight2;

extern mpoint_t *markpoints;
extern int      markpointnum;
extern int      markpointnum_max;

dboolean keystate(int key);

typedef struct am_frame_s
{
    fixed_t centerx;
    fixed_t centery;
    fixed_t sin;
    fixed_t cos;

    fixed_t bbox[4];
} am_frame_t;

extern am_frame_t am_frame;

#endif
