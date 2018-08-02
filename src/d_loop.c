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

#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_menu.h"

static int      maketic;
ticcmd_t        localcmds[BACKUPTICS];

extern dboolean advancetitle;

void D_BuildNewTiccmds(void)
{
    static int  lastmadetic;
    int         newtics = I_GetTime() - lastmadetic;

    lastmadetic += newtics;

    while (newtics--)
    {
        I_StartTic();

        if (maketic - gametime > BACKUPTICS / 2)
            break;

        G_BuildTiccmd(&localcmds[maketic++ % BACKUPTICS]);
    }
}

void TryRunTics(void)
{
    int runtics;

    D_BuildNewTiccmds();

    if (!(runtics = maketic - gametime) && vid_capfps != TICRATE)
        return;

    while (runtics--)
    {
        if (advancetitle)
            D_DoAdvanceTitle();

        if (menuactive)
            M_Ticker();

        G_Ticker();
        gametime++;

        if (localcmds[0].buttons & BT_SPECIAL)
            localcmds[0].buttons = 0;
    }
}
