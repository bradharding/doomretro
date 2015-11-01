/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (c) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (c) 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

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

#if !defined(__D_EVENT__)
#define __D_EVENT__

#include "doomtype.h"
#include "i_video.h"

//
// Event handling.
//

// Input event types.
typedef enum
{
    ev_keydown,
    ev_keyup,
    ev_mouse,
    ev_mousewheel,
    ev_gamepad
} evtype_t;

// Event structure.
typedef struct
{
    evtype_t    type;
    int         data1;          // keys / mouse buttons
    int         data2;          // mouse x move
    int         data3;          // mouse y move
} event_t;

typedef enum
{
    ga_nothing,
    ga_loadlevel,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_completed,
    ga_victory,
    ga_worlddone,
    ga_screenshot,
    ga_reloadgame
} gameaction_t;

//
// Button/action code definitions.
//
typedef enum
{
    // Press "Fire".
    BT_ATTACK           = 1,
    // Use button, to open doors, activate switches.
    BT_USE              = 2,

    // Flag: game events, not really buttons.
    BT_SPECIAL          = 128,
    BT_SPECIALMASK      = 3,

    // Flag, weapon change pending.
    // If true, the next 3 bits hold weapon num.
    BT_CHANGE           = 4,
    // The 3bit weapon mask and shift, convenience.
    BT_WEAPONMASK       = (8 + 16 + 32),
    BT_WEAPONSHIFT      = 3,

    // Pause the game.
    BTS_PAUSE           = 1,
    // Save the game at each console.
    BTS_SAVEGAME        = 2,

    // Savegame slot numbers
    //  occupy the second byte of buttons.
    BTS_SAVEMASK        = (4 + 8 + 16),
    BTS_SAVESHIFT       = 2
} buttoncode_t;

//
// GLOBAL VARIABLES
//

extern gameaction_t     gameaction;

extern dboolean         altdown;

#endif
