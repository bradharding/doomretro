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

#include <memory.h>

#include "d_main.h"
#include "m_argv.h"
#include "m_menu.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "g_game.h"
#include "doomdef.h"
#include "doomstat.h"

// Maximum time that we wait in TryRunTics() for netgame data to be
// received before we bail out and render a frame anyway.
// Vanilla Doom used 20 for this value, but we use a smaller value
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

ticcmd_t        netcmds[BACKUPTICS];

int             maketic;

// Used for original sync code.
int             lastnettic;
int             skiptics = 0;

// Reduce the bandwidth needed by sampling game input less and transmitting
// less.  If ticdup is 2, sample half normal, 3 = one third normal, etc.
int             ticdup;

// Send this many extra (backup) tics in each packet.
int             extratics;

// 35 fps clock
static int GetTime(void)
{
    return (I_GetTimeMS() * TICRATE) / 1000;
}

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
int             lasttime;

void NetUpdate(void)
{
    int         nowtime;
    int         newtics;
    int         i;
    int         gameticdiv;

    // check time
    nowtime = GetTime() / ticdup;
    newtics = nowtime - lasttime;

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
    gameticdiv = gametic / ticdup;

    for (i = 0; i < newtics; i++)
    {
        ticcmd_t        cmd;

        I_StartTic();
        D_ProcessEvents();

        // Always run the menu
        M_Ticker();

        if (maketic - gameticdiv >= 5)
            break;

        memset(&cmd, 0, sizeof(ticcmd_t));
        G_BuildTiccmd(&cmd);

        netcmds[maketic % BACKUPTICS] = cmd;

        ++maketic;
    }
}

//
// Start game loop
//
// Called after the screen is set but before the game starts running.
//
void D_StartGameLoop(void)
{
    lasttime = GetTime() / ticdup;
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
void D_CheckNetGame(void)
{
    // default values for single player
    ticdup = 1;
    extratics = 1;
}

//
// TryRunTics
//
extern dboolean advancetitle;

void TryRunTics(void)
{
    int         i;
    int         entertic;
    static int  oldentertics;
    int         realtics;
    int         availabletics;
    int         counts;

    // get real tics
    entertic = I_GetTime() / ticdup;
    realtics = entertic - oldentertics;
    oldentertics = entertic;

    // get available tics
    NetUpdate();

    availabletics = maketic - gametic / ticdup;

    // decide how many tics to run
    if (realtics < availabletics - 1)
        counts = realtics + 1;
    else if (realtics < availabletics)
        counts = realtics;
    else
        counts = availabletics;

    if (!counts && !vid_capfps)
        return;

    if (counts < 1)
        counts = 1;

    // wait for new tics if needed
    while (maketic < gametic / ticdup + counts)
    {
        NetUpdate();

        // Still no tics to run? Sleep until some are available.
        if (maketic < gametic/ticdup + counts)
        {
            // If we're in a netgame, we might spin forever waiting for
            // new network data to be received. So don't stay in here
            // forever - give the menu a chance to work.
            if (I_GetTime() / ticdup - entertic >= MAX_NETGAME_STALL_TICS)
                return;
 
            I_Sleep(1);
        }
    }

    // run the count * ticdup tics
    while (counts--)
    {
        for (i = 0; i < ticdup; i++)
        {
            if (advancetitle)
                D_DoAdvanceTitle();

            G_Ticker();
            gametic++;
            gametime++;

            // modify command for duplicated tics
            if (i != ticdup - 1)
            {
                ticcmd_t    *cmd = &netcmds[(gametic / ticdup) % BACKUPTICS];

                if (cmd->buttons & BT_SPECIAL)
                    cmd->buttons = 0;
            }
        }
        NetUpdate();
    }
}
