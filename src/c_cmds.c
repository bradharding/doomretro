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
#include "p_local.h"
#include "SDL.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

boolean C_CheatCondition(char *);
boolean C_GameCondition(char *);
boolean C_NoCondition(char *);

void C_CmdList(void);
void C_Map(void);
void C_Quit(void);
void C_Summon(void);

consolecommand_t consolecommands[] =
{
    { "cmdlist",    C_NoCondition,    C_CmdList, 0 },
    { "idbeholda",  C_CheatCondition, NULL,      0 },
    { "idbeholdl",  C_CheatCondition, NULL,      0 },
    { "idbeholdi",  C_CheatCondition, NULL,      0 },
    { "idbeholdr",  C_CheatCondition, NULL,      0 },
    { "idbeholds",  C_CheatCondition, NULL,      0 },
    { "idbeholdv",  C_CheatCondition, NULL,      0 },
    { "idchoppers", C_CheatCondition, NULL,      0 },
    { "idclev",     C_CheatCondition, NULL,      1 },
    { "idclip",     C_CheatCondition, NULL,      0 },
    { "iddqd",      C_CheatCondition, NULL,      0 },
    { "iddt",       C_CheatCondition, NULL,      0 },
    { "idfa",       C_CheatCondition, NULL,      0 },
    { "idkfa",      C_CheatCondition, NULL,      0 },
    { "idmus",      C_CheatCondition, NULL,      1 },
    { "idmypos",    C_CheatCondition, NULL,      0 },
    { "idspispopd", C_CheatCondition, NULL,      0 },
    { "map",        C_NoCondition,    C_Map,     1 },
    { "quit",       C_NoCondition,    C_Quit,    0 },
    { "summon",     C_GameCondition,  C_Summon,  1 },
    { "",           C_NoCondition,    NULL,      0 }
};

boolean C_CheatCondition(char *command)
{
    if (!usergame)
        return false;
    if (!strcasecmp(command, "idclip") && gamemode != commercial)
        return false;
    if (!strcasecmp(command, "idspispopd") && gamemode == commercial)
        return false;
    return true;
}

boolean C_GameCondition(char *command)
{
    return usergame;
}

boolean C_NoCondition(char *command)
{
    return true;
}

void C_CmdList(void)
{
    int i = 0;

    while (consolecommands[i].command[0])
    {
        if (consolecommands[i].condition != C_CheatCondition)
        {
            static char     buffer[1024];

            M_snprintf(buffer, 1024, "    %s", consolecommands[i].command);
            C_AddConsoleString(buffer);
        }
        ++i;
    }
}

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

void C_Summon(void)
{
    int i;

    for (i = 0; i < NUMMOBJTYPES; i++)
        if (!strcasecmp(mobjinfo[i].summon, consolecommandparm))
        {
            boolean     spawn = true;
            int         type = mobjinfo[i].doomednum;

            if (gamemode != commercial)
            {
                switch (type)
                {
                    case Arachnotron:
                    case ArchVile:
                    case BossBrain:
                    case MonstersSpawner:
                    case HellKnight:
                    case Mancubus:
                    case PainElemental:
                    case HeavyWeaponDude:
                    case Revenant:
                    case WolfensteinSS:
                        spawn = false;
                        break;
                }
            }
            else if (type == WolfensteinSS && bfgedition)
                type = Zombieman;

            if (spawn)
            {
                mobj_t  *player = players[displayplayer].mo;
                fixed_t x = player->x;
                fixed_t y = player->y;
                angle_t angle = player->angle >> ANGLETOFINESHIFT;
                mobj_t  *thing = P_SpawnMobj(x + 100 * finecosine[angle],
                            y + 100 * finesine[angle], ONFLOORZ, P_FindDoomedNum(type));

                thing->angle = R_PointToAngle2(thing->x, thing->y, x, y);
            }
            break;
        }
}
