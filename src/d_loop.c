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

#include "c_console.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_menu.h"
#include "i_system.h"
#include "i_timer.h"

// Maximum time that we wait in TryRunTics() for netgame data to be
// received before we bail out and render a frame anyway.
// Vanilla DOOM used 20 for this value, but we use a smaller value
// instead for better responsiveness of the menu when we're stuck.
#define MAX_NETGAME_STALL_TICS  5

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tick that hasn't had control made for it yet
//
// a gametic cannot be run until nettics > gametic for all players
//

ticcmd_t    netcmds[BACKUPTICS];

int         maketic;

// Used for original sync code.
int         skiptics;

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
int         lasttime;

static dboolean BuildNewTic(void)
{
    ticcmd_t    cmd;

    I_StartTic();
    D_ProcessEvents();

    // Always run the menu
    M_Ticker();

    if (maketic - gametic > 2)
        return false;

    G_BuildTiccmd(&cmd);

    netcmds[maketic++ % BACKUPTICS] = cmd;

    return true;
}

void NetUpdate(void)
{
    int nowtime = I_GetTime();
    int newtics = nowtime - lasttime;
    int i;

    lasttime = nowtime;

    if (skiptics <= newtics)
    {
        newtics -= skiptics;
        skiptics = 0;
    }
    else
    {
        skiptics -= newtics;
        newtics = 0;
    }

    // build new ticcmds for console player
    for (i = 0; i < newtics; i++)
        if (!BuildNewTic())
            break;
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
    int entertic;
    int counts;

    // get available tics
    NetUpdate();

    counts = maketic - gametic;

    if (!counts && vid_capfps != TICRATE)
        return;

    entertic = I_GetTime();

    if (counts < 1)
        counts = 1;

    // wait for new tics if needed
    while (maketic < gametic + counts)
    {
        NetUpdate();

        // Still no tics to run? Sleep until some are available.
        if (maketic < gametic + counts)
        {
            // If we're in a netgame, we might spin forever waiting for
            // new network data to be received. So don't stay in here
            // forever - give the menu a chance to work.
            if (I_GetTime() - entertic >= MAX_NETGAME_STALL_TICS)
                return;

            I_Sleep(1);
        }
    }

    // run the count tics
    while (counts--)
    {
        if (advancetitle)
            D_DoAdvanceTitle();

        G_Ticker();
        gametic++;

        if (!menuactive && !consoleactive && !paused)
            activetic++;

        gametime++;

        if (netcmds[0].buttons & BT_SPECIAL)
            netcmds[0].buttons = 0;

        NetUpdate();
    }
}
