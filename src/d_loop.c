/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

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

========================================================================
*/

#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_menu.h"

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0). Used for interpolation.
fixed_t     fractionaltic = 0;

ticcmd_t    localcmds[BACKUPTICS];

void TryRunTics(void)
{
    static int      maketic;
    static uint64_t lastmadetic;
    uint64_t        newtics = I_GetTime() - lastmadetic;
    int             runtics;

    lastmadetic += newtics;

    if (vid_capfps != TICRATE)
        fractionaltic = ((I_GetTimeMS() * TICRATE) % 1000) * FRACUNIT / 1000;

    while (newtics--)
    {
        I_StartTic();

        if (maketic - gametime > BACKUPTICS / 2)
            break;

        G_BuildTiccmd(&localcmds[maketic++ % BACKUPTICS]);
    }

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
