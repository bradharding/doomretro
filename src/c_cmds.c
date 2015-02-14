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
#include "d_deh.h"
#include "d_event.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_inter.h"
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
boolean C_OnOffCondition(char *);
boolean C_SummonCondition(char *);

void C_Clear(void);
void C_CmdList(void);
void C_CvarList(void);
void C_God(void);
void C_Kill(void);
void C_Map(void);
void C_Messages(void);
void C_NoClip(void);
void C_NoTarget(void);
void C_Quit(void);
void C_Summon(void);

consolecmd_t consolecmds[] =
{
    { "clear",      C_NoCondition,     C_Clear,    0, CT_CMD,   "Clear the console."                   },
    { "cmdlist",    C_NoCondition,     C_CmdList,  0, CT_CMD,   "Display a list of console commands."  },
    { "cvarlist",   C_NoCondition,     C_CvarList, 0, CT_CMD,   "Display a list of console variables." },
    { "god",        C_GameCondition,   C_God,      0, CT_CMD,   "Toggle degreelessness mode on/off."   },
    { "idbeholda",  C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idbeholdl",  C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idbeholdi",  C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idbeholdr",  C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idbeholds",  C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idbeholdv",  C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idchoppers", C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idclev",     C_CheatCondition,  NULL,       1, CT_CHEAT, ""                                     },
    { "idclip",     C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "iddqd",      C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "iddt",       C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idfa",       C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idkfa",      C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idmus",      C_CheatCondition,  NULL,       1, CT_CHEAT, ""                                     },
    { "idmypos",    C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "idspispopd", C_CheatCondition,  NULL,       0, CT_CHEAT, ""                                     },
    { "kill",       C_GameCondition,   C_Kill,     1, CT_CMD,   "Kill the player or monsters."         },
    { "map",        C_MapCondition,    C_Map,      1, CT_CMD,   "Warp to a map."                       },
    { "messages",   C_OnOffCondition,  C_Messages, 1, CT_CVAR,  "Toggle messages on/off."              },
    { "noclip",     C_GameCondition,   C_NoClip,   0, CT_CMD,   "Toggle no clipping mode on/off."      },
    { "notarget",   C_GameCondition,   C_NoTarget, 0, CT_CMD,   "Toggle no target mode on/off."        },
    { "quit",       C_NoCondition,     C_Quit,     0, CT_CMD,   "Quit DOOM RETRO."                     },
    { "summon",     C_SummonCondition, C_Summon,   1, CT_CMD,   "Summon a monster or map decoration."  },
    { "",           C_NoCondition,     NULL,       0, 0,        ""                                     }
};

boolean C_CheatCondition(char *cmd)
{
    if (gamestate != GS_LEVEL)
        return false;
    if (!strcasecmp(cmd, "idclip") && gamemode != commercial)
        return false;
    if (!strcasecmp(cmd, "idspispopd") && gamemode == commercial)
        return false;
    return true;
}

boolean C_GameCondition(char *cmd)
{
    return (gamestate == GS_LEVEL);
}

static int      mapcmdepisode;
static int      mapcmdmap;

boolean C_MapCondition(char *cmd)
{
    if (!consolecmdparm[0])
        return false;

    mapcmdepisode = 0;
    mapcmdmap = 0;

    if (gamemode == commercial)
    {
        if (BTSX)
        {
            sscanf(uppercase(consolecmdparm), "E%iM%02i", &mapcmdepisode, &mapcmdmap);
            if (mapcmdmap && ((mapcmdepisode == 1 && BTSXE1) || (mapcmdepisode == 2 && BTSXE2)))
            {
                M_snprintf(consolecmdparm, sizeof(consolecmdparm), "MAP%02i", mapcmdmap);
                return (W_CheckMultipleLumps(consolecmdparm) == 2);
            }
        }
        sscanf(uppercase(consolecmdparm), "MAP%02i", &mapcmdmap);
        if (!mapcmdmap)
            return false;
        if (BTSX && (W_CheckMultipleLumps(consolecmdparm) == 1))
            return false;
        if (gamestate != GS_LEVEL && gamemission == pack_nerve)
            gamemission = doom2;
    }
    else
    {
        sscanf(uppercase(consolecmdparm), "E%iM%i", &mapcmdepisode, &mapcmdmap);
        if (!mapcmdepisode || !mapcmdmap)
            return false;
    }

    return (W_CheckNumForName(consolecmdparm) >= 0);
}

boolean C_NoCondition(char *cmd)
{
    return true;
}

boolean C_OnOffCondition(char *cmd)
{
    return (!consolecmdparm[0] || !strcasecmp(consolecmdparm, "on")
        || !strcasecmp(consolecmdparm, "off"));
}

static int      summoncmdtype = NUMMOBJTYPES;

boolean C_SummonCondition(char *cmd)
{
    if (!consolecmdparm[0])
        return false;

    if (gamestate == GS_LEVEL)
    {
        int i;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (!strcasecmp(consolecmdparm, mobjinfo[i].name))
            {
                boolean     summon = true;

                summoncmdtype = mobjinfo[i].doomednum;
                if (gamemode != commercial)
                {
                    switch (summoncmdtype)
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
                else if (summoncmdtype == WolfensteinSS && bfgedition)
                    summoncmdtype = Zombieman;

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

void C_CmdList(void)
{
    int i = 0;
    int count = 1;

    while (consolecmds[i].cmd[0])
    {
        if (consolecmds[i].type == CT_CMD)
        {
            static char     buffer[1024];

            M_snprintf(buffer, 1024, "%i\t%s\t%s", count++, consolecmds[i].cmd,
                consolecmds[i].description);
            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

void C_CvarList(void)
{
    int i = 0;
    int count = 1;

    while (consolecmds[i].cmd[0])
    {
        if (consolecmds[i].type == CT_CVAR)
        {
            static char     buffer[1024];

            M_snprintf(buffer, 1024, "%i\t%s\t%s", count++, consolecmds[i].cmd,
                consolecmds[i].description);
            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

void C_God(void)
{
    M_StringCopy(consolecheat, "iddqd", sizeof(consolecheat));
}

void A_Fall(mobj_t *actor);

void C_Kill(void)
{
    if (!consolecmdparm[0])
    {
        P_KillMobj(NULL, players[displayplayer].mo);
        C_AddConsoleString("Player killed.", output, CONSOLEOUTPUTCOLOR);
    }
    else
    {
        int             i, j;
        int             kills = 0;
        static char     buffer[1024];

        if (!strcasecmp(consolecmdparm, "all")
            || !strcasecmp(consolecmdparm, "monsters"))
        {
            for (i = 0; i < numsectors; ++i)
            {
                mobj_t      *thing = sectors[i].thinglist;

                while (thing)
                {
                    if (thing->health > 0)
                    {
                        if (thing->type == MT_PAIN)
                        {
                            A_Fall(thing);
                            P_SetMobjState(thing, S_PAIN_DIE6);
                            kills++;
                        }
                        else if (thing->flags & MF_SHOOTABLE)
                        {
                            P_DamageMobj(thing, NULL, NULL, thing->health);
                            kills++;
                        }
                    }
                    thing = thing->snext;
                }
            }
        }
        else
        {
            for (i = 0; i < NUMMOBJTYPES; i++)
                if (!strcasecmp(consolecmdparm, mobjinfo[i].name)
                    || !strcasecmp(consolecmdparm, mobjinfo[i].plural))
                {
                    boolean     kill = true;
                    int         type = mobjinfo[i].doomednum;

                    if (gamemode != commercial)
                    {
                        switch (summoncmdtype)
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
                                kill = false;
                                break;
                        }
                    }
                    else if (type == WolfensteinSS && bfgedition)
                        type = Zombieman;

                    if (kill)
                    {
                        type = P_FindDoomedNum(type);
                        for (j = 0; j < numsectors; ++j)
                        {
                            mobj_t      *thing = sectors[j].thinglist;

                            while (thing)
                            {
                                if (thing->health > 0)
                                {
                                    if (type == thing->type)
                                        if (type == MT_PAIN)
                                        {
                                            A_Fall(thing);
                                            P_SetMobjState(thing, S_PAIN_DIE6);
                                            kills++;
                                        }
                                        else if (thing->flags & MF_SHOOTABLE)
                                        {
                                            P_DamageMobj(thing, NULL, NULL, thing->health);
                                            kills++;
                                        }
                                }
                                thing = thing->snext;
                            }
                        }
                    }
                    break;
                }
        }
        M_snprintf(buffer, sizeof(buffer), "%i monster%s killed.", kills, kills == 1 ? "" : "s");
        C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
    }
}

extern boolean  samelevel;
extern int      selectedepisode;
extern int      selectedskilllevel;
extern menu_t   EpiDef;

void C_Map(void)
{
    samelevel = (gameepisode == mapcmdepisode && gamemap == mapcmdmap);
    gameepisode = mapcmdepisode;
    if (gamemission == doom && gameepisode <= 4)
    {
        selectedepisode = gameepisode - 1;
        EpiDef.lastOn = selectedepisode;
    }
    gamemap = mapcmdmap;
    if (gamestate == GS_LEVEL)
        G_DeferredLoadLevel(gamestate == GS_LEVEL ? gameskill : selectedskilllevel, gameepisode,
            gamemap);
    else
        G_DeferredInitNew(gamestate == GS_LEVEL ? gameskill : selectedskilllevel, gameepisode,
            gamemap);
}

extern boolean  messages;

void C_Messages(void)
{
    if (!strcasecmp(consolecmdparm, "on"))
        messages = true;
    else if (!strcasecmp(consolecmdparm, "off"))
        messages = false;
    C_AddConsoleString(messages ? "\"messages\" are \"on\"" : "\"messages\" are \"off\"", output,
        CONSOLEOUTPUTCOLOR);
    M_SaveDefaults();
}

void C_NoClip(void)
{
    M_StringCopy(consolecheat, (gamemode == commercial ? "idclip" : "idspispopd"),
        sizeof(consolecheat));
}

void C_NoTarget(void)
{
    players[displayplayer].cheats ^= CF_NOTARGET;
    C_AddConsoleString(players[displayplayer].cheats & CF_NOTARGET ? s_STSTR_NTON : s_STSTR_NTOFF,
        output, CONSOLEOUTPUTCOLOR);
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
        ONFLOORZ, P_FindDoomedNum(summoncmdtype));

    thing->angle = R_PointToAngle2(thing->x, thing->y, x, y);
}
