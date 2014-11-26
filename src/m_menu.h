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

#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"

typedef struct
{
    short               status;
    char                name[10];
    void                (*routine)(int choice);
    char                **text;
} menuitem_t;

typedef struct menu_s
{
    short               numitems;
    struct menu_s       *prevMenu;
    menuitem_t          *menuitems;
    void                (*routine)(void);
    short               x;
    short               y;
    short               lastOn;
} menu_t;

boolean startingnewgame;

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
boolean M_Responder(event_t *ev);

// Called by main loop,
// only used for menu (skull cursor) animation.
void M_Ticker(void);

// Called by main loop,
// draws the menus directly into the screen buffer.
void M_Drawer(void);

// Called by D_DoomMain,
// loads the config file.
void M_Init(void);

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.
void M_StartControlPanel(void);

void M_DarkBackground(void);
void M_DrawCenteredString(int y, char *str);

void M_UpdateWindowCaption(void);

extern int screensize;

extern int gamepadmenu;
extern boolean nomusic;
extern boolean nosound;
extern boolean nosfx;
extern boolean firstevent;
extern byte grays[256];

#endif
