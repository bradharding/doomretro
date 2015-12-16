/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__M_MENU__)
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
    int                 lastOn;
} menu_t;

dboolean startingnewgame;

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
dboolean M_Responder(event_t *ev);

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
void M_ShowHelp(void);
void M_EndingGame(void);
void M_ChangeGamma(dboolean shift);

void M_DarkBackground(void);
void M_DrawCenteredString(int y, char *str);

void M_SetWindowCaption(void);

extern int      r_screensize;
extern int      gamepadmenu;
extern dboolean nomusic;
extern dboolean nosound;
extern dboolean nosfx;
extern dboolean firstevent;
extern byte     grays[256];
extern int      s_musicvolume;
extern int      s_sfxvolume;
extern dboolean vid_widescreen;
extern dboolean r_hud;
extern dboolean returntowidescreen;
extern int      episode;
extern int      expansion;
extern int      savegame;
extern int      skilllevel;

#endif
