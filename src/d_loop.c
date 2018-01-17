/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#include "c_console.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_menu.h"
#include "i_system.h"
#include "i_timer.h"

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tick that hasn't had control made for it yet
//
// a gametic cannot be run until nettics > gametic for all players
//

ticcmd_t    netcmds[BACKUPTICS];

static int  maketic;

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
static int  lasttime;

static dboolean BuildNewTic(void)
{
    ticcmd_t    cmd;

    I_StartTic();
    D_ProcessEvents();

    // Always run the menu
    if (menuactive)
        M_Ticker();

    if (maketic - gametic > 2)
        return false;

    G_BuildTiccmd(&cmd);
    netcmds[maketic++ % BACKUPTICS] = cmd;

    return true;
}

static void NetUpdate(void)
{
    int         nowtime = I_GetTime();
    int         newtics = nowtime - lasttime;
    static int  skiptics;

    lasttime = nowtime;

    if (skiptics <= newtics)
    {
        newtics -= skiptics;
        skiptics = 0;

        // build new ticcmds for console player
        while (newtics--)
            if (!BuildNewTic())
                break;
    }
    else
        skiptics -= newtics;
}

//
// Start game loop
//
// Called after the screen is set but before the game starts running.
//
void D_StartGameLoop(void)
{
    lasttime = I_GetTime();
}

//
// TryRunTics
//
extern dboolean advancetitle;

void TryRunTics(void)
{
    // get real tics
    int counts;

    // get available tics
    NetUpdate();

    if (!(counts = maketic - gametic) && vid_capfps != TICRATE)
        return;

    // run the count tics
    while (counts-- > 0)
    {
        if (advancetitle)
            D_DoAdvanceTitle();

        G_Ticker();
        gametic++;
        gametime++;

        if (netcmds[0].buttons & BT_SPECIAL)
            netcmds[0].buttons = 0;
    }
}
