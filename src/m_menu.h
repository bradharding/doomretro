/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#pragma once

#include "p_saveg.h"

typedef struct
{
    short           status;
    char            name[10];
    void            (*routine)(int choice);
    char            **text;
    int             x;
    int             y;
    int             width;
    int             height;
    int             sliderx;
} menuitem_t;

typedef struct menu_s
{
    short           numitems;
    struct menu_s   *prevmenu;
    menuitem_t      *menuitems;
    void            (*routine)(void);
    short           x;
    short           y;
    int             laston;
    bool            change;
} menu_t;

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
bool M_Responder(event_t *ev);

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
void M_OpenMainMenu(void);
void M_CloseMenu(void);
void M_EndingGame(void);

void M_DrawMenuBackground(void);
void M_DrawSmallChar(int x, int y, int i, bool highlight, bool shadow);
void M_DrawString(int x, int y, char *string, bool highlight, bool shadow);
void M_DrawCenteredString(int y, char *string);
int M_StringWidth(char *string);

void M_SetWindowCaption(void);

void M_UpdateSaveGameName(int i);
int M_CountSaveGames(void);

void M_StartMessage(char *string, void (*routine)(int), bool input);

void M_QuitResponse(int key);
void M_QuitDOOM(int choice);

void M_AddEpisode(const int map, const int ep, const char *lumpname, const char *string);

extern bool     messagetoprint;
extern bool     nomusic;
extern bool     nosound;
extern bool     nosfx;
extern bool     firstevent;
extern bool     savegames;
extern bool     helpscreen;
extern bool     quitting;
extern int      menuspindirection;
extern int      menuspinspeed;
extern int      quicksaveslot;
extern char     savegamestrings[savegame_max][SAVESTRINGSIZE];
extern menu_t   EpiDef;
extern menu_t   ExpDef;
extern menu_t   LoadDef;
extern menu_t   MainDef;
extern menu_t   NewDef;
extern menu_t   SaveDef;
extern bool     customepisode;
extern uint64_t gamecontrollerwait;
extern uint64_t mousewait;
extern int      highlightcount;
