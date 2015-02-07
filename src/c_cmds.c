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

#include "c_cmds.h"
#include "c_console.h"
#include "d_event.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_cheat.h"
#include "m_menu.h"
#include "m_misc.h"
#include "SDL.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

void C_CmdList(void);
void C_Map(void);
void C_Quit(void);

consolecommand_t consolecommands[] =
{
    { "cmdlist",    false, C_CmdList, 0 },
    { "idbeholda",  true,  NULL,      0 },
    { "idbeholdl",  true,  NULL,      0 },
    { "idbeholdi",  true,  NULL,      0 },
    { "idbeholdr",  true,  NULL,      0 },
    { "idbeholds",  true,  NULL,      0 },
    { "idbeholdv",  true,  NULL,      0 },
    { "idchoppers", true,  NULL,      0 },
    { "idclev",     true,  NULL,      1 },
    { "idclip",     true,  NULL,      0 },
    { "iddqd",      true,  NULL,      0 },
    { "iddt",       true,  NULL,      0 },
    { "idfa",       true,  NULL,      0 },
    { "idkfa",      true,  NULL,      0 },
    { "idmus",      true,  NULL,      1 },
    { "idmypos",    true,  NULL,      0 },
    { "idspispopd", true,  NULL,      0 },
    { "map",        false, C_Map,     1 },
    { "quit",       false, C_Quit,    0 },
    { "",           false, NULL,      0 }
};

extern boolean  samelevel;
extern int      selectedepisode;
extern menu_t   EpiDef;

void C_Map(void)
{
    int epsd = 0;
    int map = 0;

    if (gamemode == commercial)
    {
        epsd = 1;
        sscanf(uppercase(consolecommandparm), "MAP%i", &map);
    }
    else
        sscanf(uppercase(consolecommandparm), "E%iM%i", &epsd, &map);

    if (W_CheckNumForName(consolecommandparm) >= 0)
    {
        samelevel = (gameepisode == epsd && gamemap == map);
        gameepisode = epsd;
        if (gamemission == doom && epsd <= 4)
        {
            selectedepisode = gameepisode - 1;
            EpiDef.lastOn = selectedepisode;
        }
        gamemap = map;
        if (usergame)
            G_DeferredLoadLevel(gameskill, gameepisode, gamemap);
        else
            G_DeferredInitNew(gameskill, gameepisode, gamemap);
    }
}

void C_Quit(void)
{
    I_Quit(true);
}
