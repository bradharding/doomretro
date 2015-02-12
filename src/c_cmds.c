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
#include "m_config.h"
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
boolean C_MapCondition(char *);
boolean C_NoCondition(char *);
boolean C_SummonCondition(char *);

void C_Clear(void);
void C_CmdList(void);
void C_CvarList(void);
void C_God(void);
void C_Map(void);
void C_NoClip(void);
void C_Quit(void);
void C_Summon(void);

consolecommand_t consolecommands[] =
{
    { "clear",      C_NoCondition,     C_Clear,    0, "Clear the console."                   },
    { "cmdlist",    C_NoCondition,     C_CmdList,  0, "Display a list of console commands."  },
    { "cvarlist",   C_NoCondition,     C_CvarList, 0, "Display a list of console variables." },
    { "god",        C_GameCondition,   C_God,      0, "Toggle degreelessness mode on/off."   },
    { "idbeholda",  C_CheatCondition,  NULL,       0, ""                                     },
    { "idbeholdl",  C_CheatCondition,  NULL,       0, ""                                     },
    { "idbeholdi",  C_CheatCondition,  NULL,       0, ""                                     },
    { "idbeholdr",  C_CheatCondition,  NULL,       0, ""                                     },
    { "idbeholds",  C_CheatCondition,  NULL,       0, ""                                     },
    { "idbeholdv",  C_CheatCondition,  NULL,       0, ""                                     },
    { "idchoppers", C_CheatCondition,  NULL,       0, ""                                     },
    { "idclev",     C_CheatCondition,  NULL,       1, ""                                     },
    { "idclip",     C_CheatCondition,  NULL,       0, ""                                     },
    { "iddqd",      C_CheatCondition,  NULL,       0, ""                                     },
    { "iddt",       C_CheatCondition,  NULL,       0, ""                                     },
    { "idfa",       C_CheatCondition,  NULL,       0, ""                                     },
    { "idkfa",      C_CheatCondition,  NULL,       0, ""                                     },
    { "idmus",      C_CheatCondition,  NULL,       1, ""                                     },
    { "idmypos",    C_CheatCondition,  NULL,       0, ""                                     },
    { "idspispopd", C_CheatCondition,  NULL,       0, ""                                     },
    { "map",        C_MapCondition,    C_Map,      1, "Warp to a map."                       },
    { "noclip",     C_GameCondition,   C_NoClip,   0, "Toggle no clipping mode on/off."      },
    { "quit",       C_NoCondition,     C_Quit,     0, "Quit DOOM RETRO."                     },
    { "summon",     C_SummonCondition, C_Summon,   1, "Summon a monster or map decoration."  },
    { "",           C_NoCondition,     NULL,       0, ""                                     }
};

boolean C_CheatCondition(char *command)
{
    if (gamestate != GS_LEVEL)
        return false;
    if (!strcasecmp(command, "idclip") && gamemode != commercial)
        return false;
    if (!strcasecmp(command, "idspispopd") && gamemode == commercial)
        return false;
    return true;
}

boolean C_GameCondition(char *command)
{
    return (gamestate == GS_LEVEL);
}

static int      mapcommandepisode;
static int      mapcommandmap;

boolean C_MapCondition(char *command)
{
    mapcommandepisode = 0;
    mapcommandmap = 0;

    if (gamemode == commercial)
    {
        if (BTSX)
        {
            sscanf(uppercase(consolecommandparm), "E%iM%02i", &mapcommandepisode, &mapcommandmap);
            if (mapcommandmap && ((mapcommandepisode == 1 && BTSXE1) || (mapcommandepisode == 2 && BTSXE2)))
            {
                M_snprintf(consolecommandparm, sizeof(consolecommandparm), "MAP%02i", mapcommandmap);
                return (W_CheckMultipleLumps(consolecommandparm) == 2);
            }
        }
        sscanf(uppercase(consolecommandparm), "MAP%02i", &mapcommandmap);
        if (!mapcommandmap)
            return false;
        if (BTSX && (W_CheckMultipleLumps(consolecommandparm) == 1))
            return false;
        if (gamestate != GS_LEVEL && gamemission == pack_nerve)
            gamemission = doom2;
    }
    else
    {
        sscanf(uppercase(consolecommandparm), "E%iM%i", &mapcommandepisode, &mapcommandmap);
        if (!mapcommandepisode || !mapcommandmap)
            return false;
    }

    return (W_CheckNumForName(consolecommandparm) >= 0);
}

boolean C_NoCondition(char *command)
{
    return true;
}

static int      summoncommandtype = NUMMOBJTYPES;

boolean C_SummonCondition(char *command)
{
    if (gamestate == GS_LEVEL)
    {
        int i;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (!strcasecmp(mobjinfo[i].summon, consolecommandparm))
            {
                boolean     summon = true;

                summoncommandtype = mobjinfo[i].doomednum;
                if (gamemode != commercial)
                {
                    switch (summoncommandtype)
                    {
                        case Arachnotron:
                        case ArchVile:
                        case BossBrain:
                        case HellKnight:
                        case Mancubus:
                        case PainElemental:
                        case HeavyWeaponDude:
                        case Revenant:
                        case WolfensteinSS:
                            summon = false;
                            break;
                    }
                }
                else if (summoncommandtype == WolfensteinSS && bfgedition)
                    summoncommandtype = Zombieman;

                return summon;
            }
    }
    return false;
}

extern int      consolestrings;

void C_Clear(void)
{
    consolestrings = 0;
}

void C_CvarList(void)
{
    int i = 0;
    int count = 1;

    while (i < doom_defaults.numdefaults)
    {
        static char     buffer[1024];

        M_snprintf(buffer, 1024, "%i\t%s", count++, doom_defaults.defaults[i].name);
        C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        ++i;
    }
}

void C_CmdList(void)
{
    int i = 0;
    int count = 1;

    while (consolecommands[i].command[0])
    {
        if (consolecommands[i].condition != C_CheatCondition)
        {
            static char     buffer[1024];

            M_snprintf(buffer, 1024, "%i\t%s\t%s", count++, consolecommands[i].command,
                consolecommands[i].description);
            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

void C_God(void)
{
    M_StringCopy(consolecheat, "iddqd", sizeof(consolecheat));
}

extern boolean  samelevel;
extern int      selectedepisode;
extern int      selectedskilllevel;
extern menu_t   EpiDef;

void C_Map(void)
{
    samelevel = (gameepisode == mapcommandepisode && gamemap == mapcommandmap);
    gameepisode = mapcommandepisode;
    if (gamemission == doom && gameepisode <= 4)
    {
        selectedepisode = gameepisode - 1;
        EpiDef.lastOn = selectedepisode;
    }
    gamemap = mapcommandmap;
    if (gamestate == GS_LEVEL)
        G_DeferredLoadLevel(gamestate == GS_LEVEL ? gameskill : selectedskilllevel, gameepisode,
            gamemap);
    else
        G_DeferredInitNew(gamestate == GS_LEVEL ? gameskill : selectedskilllevel, gameepisode,
            gamemap);
}

void C_NoClip(void)
{
    M_StringCopy(consolecheat, (gamemode == commercial ? "idclip" : "idspispopd"),
        sizeof(consolecheat));
}

void C_Quit(void)
{
    I_Quit(true);
}

void C_Summon(void)
{
    mobj_t      *player = players[displayplayer].mo;
    fixed_t     x = player->x;
    fixed_t     y = player->y;
    angle_t     angle = player->angle >> ANGLETOFINESHIFT;
    mobj_t      *thing = P_SpawnMobj(x + 100 * finecosine[angle], y + 100 * finesine[angle],
                    ONFLOORZ, P_FindDoomedNum(summoncommandtype));

    thing->angle = R_PointToAngle2(thing->x, thing->y, x, y);
}
