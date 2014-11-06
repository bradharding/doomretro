/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __AMMAP_H__
#define __AMMAP_H__

#include "d_event.h"
#include "m_cheat.h"

// Used by ST StatusBar stuff.
#define AM_MSGENTERED   1
#define AM_MSGEXITED    0

// Called by main loop.
boolean AM_Responder(event_t *ev);

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

extern int        key_up;
extern int        key_up2;
extern int        key_down;
extern int        key_down2;
extern int        key_right;
extern int        key_straferight;
extern int        key_straferight2;
extern int        key_left;
extern int        key_strafeleft;
extern int        key_strafeleft2;

extern int        gamepadautomap;

extern byte       *tinttab60;
extern byte       *tinttab80;

extern boolean    message_dontfuckwithme;
extern boolean    idbehold;
extern boolean    idclev;
extern boolean    idmus;

boolean keystate(int key);

#endif
