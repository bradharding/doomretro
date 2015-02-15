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

boolean C_BooleanCondition(char *, char *);
boolean C_CheatCondition(char *, char *);
boolean C_GameCondition(char *, char *);
boolean C_MapCondition(char *, char *);
boolean C_NoCondition(char *, char *);
boolean C_SummonCondition(char *, char *);

void C_BooleanCvar(char *, char *);
void C_Clear(char *, char *);
void C_CmdList(char *, char *);
void C_CvarList(char *, char *);
void C_God(char *, char *);
void C_Kill(char *, char *);
void C_Map(char *, char *);
void C_NoClip(char *, char *);
void C_NoTarget(char *, char *);
void C_Quit(char *, char *);
void C_Summon(char *, char *);

extern boolean  alwaysrun;
extern boolean  animatedliquid;
extern boolean  brightmaps;
extern boolean  centerweapon;
extern boolean  dclick_use;
extern boolean  floatbob;
extern boolean  footclip;
extern boolean  fullscreen;
extern boolean  grid;
extern boolean  homindicator;
extern boolean  hud;
extern boolean  messages;
extern boolean  mirrorweapons;
extern boolean  novert;
extern boolean  rotatemode;
extern boolean  shadows;
extern boolean  translucency;
extern boolean  widescreen;

consolecmd_t consolecmds[] =
{
    { "alwaysrun",      C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &alwaysrun,      ""                                     },
    { "animatedliquid", C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &animatedliquid, ""                                     },
    { "brightmaps",     C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &brightmaps,     ""                                     },
    { "centerweapon",   C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &centerweapon,   ""                                     },
    { "clear",          C_NoCondition,      C_Clear,       0, CT_CMD,   CF_NONE,    NULL,            "Clear the console."                   },
    { "cmdlist",        C_NoCondition,      C_CmdList,     0, CT_CMD,   CF_NONE,    NULL,            "Display a list of console commands."  },
    { "cvarlist",       C_NoCondition,      C_CvarList,    0, CT_CMD,   CF_NONE,    NULL,            "Display a list of console variables." },
    { "dclick_use",     C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &dclick_use,     ""                                     },
    { "floatbob",       C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &floatbob,       ""                                     },
    { "footclip",       C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &footclip,       ""                                     },
    { "fullscreen",     C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &fullscreen,     ""                                     },
    { "god",            C_GameCondition,    C_God,         0, CT_CMD,   CF_NONE,    NULL,            "Toggle degreelessness mode on/off."   },
    { "grid",           C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &grid,           ""                                     },
    { "homindicator",   C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &homindicator,   ""                                     },
    { "hud",            C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &hud,            ""                                     },
    { "idbeholda",      C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idbeholdl",      C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idbeholdi",      C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idbeholdr",      C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idbeholds",      C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idbeholdv",      C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idchoppers",     C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idclev",         C_CheatCondition,   NULL,          1, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idclip",         C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "iddqd",          C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "iddt",           C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idfa",           C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idkfa",          C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idmus",          C_CheatCondition,   NULL,          1, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idmypos",        C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "idspispopd",     C_CheatCondition,   NULL,          0, CT_CHEAT, CF_NONE,    NULL,            ""                                     },
    { "kill",           C_GameCondition,    C_Kill,        1, CT_CMD,   CF_NONE,    NULL,            "Kill the player or monsters."         },
    { "map",            C_MapCondition,     C_Map,         1, CT_CMD,   CF_NONE,    NULL,            "Warp to a map."                       },
    { "messages",       C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &messages,       ""                                     },
    { "mirrorweapons",  C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &mirrorweapons,  ""                                     },
    { "noclip",         C_GameCondition,    C_NoClip,      0, CT_CMD,   CF_NONE,    NULL,            "Toggle no clipping mode on/off."      },
    { "notarget",       C_GameCondition,    C_NoTarget,    0, CT_CMD,   CF_NONE,    NULL,            "Toggle no target mode on/off."        },
    { "novert",         C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &novert,         ""                                     },
    { "quit",           C_NoCondition,      C_Quit,        0, CT_CMD,   CF_NONE,    NULL,            "Quit DOOM RETRO."                     },
    { "rotatemode",     C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &rotatemode,     ""                                     },
    { "shadows",        C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &shadows,        ""                                     },
    { "summon",         C_SummonCondition,  C_Summon,      1, CT_CMD,   CF_NONE,    NULL,            "Summon a monster or map decoration."  },
    { "translucency",   C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &translucency,   ""                                     },
    { "widescreen",     C_BooleanCondition, C_BooleanCvar, 1, CT_CVAR,  CF_BOOLEAN, &widescreen,     ""                                     },
    { "",               C_NoCondition,      NULL,          0, 0,        CF_NONE,    NULL,            ""                                     }
};

boolean C_CheatCondition(char *cmd, char *parm)
{
    if (gamestate != GS_LEVEL)
        return false;
    if (!strcasecmp(cmd, "idclip") && gamemode != commercial)
        return false;
    if (!strcasecmp(cmd, "idspispopd") && gamemode == commercial)
        return false;
    return true;
}

boolean C_GameCondition(char *cmd, char *parm)
{
    return (gamestate == GS_LEVEL);
}

static int      mapcmdepisode;
static int      mapcmdmap;

boolean C_MapCondition(char *cmd, char *parm)
{
    if (!parm[0])
        return false;

    mapcmdepisode = 0;
    mapcmdmap = 0;

    if (gamemode == commercial)
    {
        if (BTSX)
        {
            sscanf(uppercase(parm), "E%iM%02i", &mapcmdepisode, &mapcmdmap);
            if (mapcmdmap && ((mapcmdepisode == 1 && BTSXE1) || (mapcmdepisode == 2 && BTSXE2)))
            {
                M_snprintf(parm, sizeof(parm), "MAP%02i", mapcmdmap);
                return (W_CheckMultipleLumps(parm) == 2);
            }
        }
        sscanf(uppercase(parm), "MAP%02i", &mapcmdmap);
        if (!mapcmdmap)
            return false;
        if (BTSX && (W_CheckMultipleLumps(parm) == 1))
            return false;
        if (gamestate != GS_LEVEL && gamemission == pack_nerve)
            gamemission = doom2;
    }
    else
    {
        sscanf(uppercase(parm), "E%iM%i", &mapcmdepisode, &mapcmdmap);
        if (!mapcmdepisode || !mapcmdmap)
            return false;
    }

    return (W_CheckNumForName(parm) >= 0);
}

boolean C_NoCondition(char *cmd, char *parm)
{
    return true;
}

boolean C_BooleanCondition(char *cmd, char *parm)
{
    return (!parm[0] || !strcasecmp(parm, "on") || !strcasecmp(parm, "yes")
        || !strcasecmp(parm, "true") || !strcasecmp(parm, "1") || !strcasecmp(parm, "off")
        || !strcasecmp(parm, "no") || !strcasecmp(parm, "false") || !strcasecmp(parm, "0"));
}

static int      summoncmdtype = NUMMOBJTYPES;

boolean C_SummonCondition(char *cmd, char *parm)
{
    if (!parm[0])
        return false;

    if (gamestate == GS_LEVEL)
    {
        int i;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (!strcasecmp(parm, mobjinfo[i].name1)
                || (mobjinfo[i].name2[0] && !strcasecmp(parm, mobjinfo[i].name2)))
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

void C_Clear(char *cmd, char *parm)
{
    consolestrings = 0;
}

void C_CmdList(char *cmd, char *parm)
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

void C_CvarList(char *cmd, char *parm)
{
    int i = 0;
    int count = 1;

    while (consolecmds[i].cmd[0])
    {
        if (consolecmds[i].type == CT_CVAR)
        {
            static char     buffer[1024];

            if (consolecmds[i].flags & CF_BOOLEAN)
                M_snprintf(buffer, sizeof(buffer), "%i\t\"%s\" is \"%s\"", count++,
                    consolecmds[i].cmd, (*(boolean *)consolecmds[i].value ? "on" : "off"));

            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

void C_God(char *cmd, char *parm)
{
    M_StringCopy(consolecheat, "iddqd", sizeof(consolecheat));
}

void A_Fall(mobj_t *actor);

void C_Kill(char *cmd, char *parm)
{
    if (!parm[0])
    {
        P_KillMobj(NULL, players[displayplayer].mo);
        C_AddConsoleString("Player killed.", output, CONSOLEOUTPUTCOLOR);
    }
    else
    {
        int             i, j;
        int             kills = 0;
        static char     buffer[1024];

        if (!strcasecmp(parm, "all") || !strcasecmp(parm, "monsters"))
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
                if (!strcasecmp(parm, mobjinfo[i].name1)
                    || !strcasecmp(parm, mobjinfo[i].plural1)
                    || (mobjinfo[i].name2[0] && !strcasecmp(parm, mobjinfo[i].name2))
                    || (mobjinfo[i].plural2[0] && !strcasecmp(parm, mobjinfo[i].plural2)))
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

void C_Map(char *cmd, char *parm)
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

void C_BooleanCvar(char *cmd, char *parm)
{
    int i = 0;

    while (consolecmds[i].cmd[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].cmd) && consolecmds[i].type == CT_CVAR)
        {
            static char     buffer[1024];

            if (parm[0])
            {
                if (!strcasecmp(parm, "on") || !strcasecmp(parm, "yes")
                    || !strcasecmp(parm, "true") || !strcasecmp(parm, "1"))
                    *(boolean *)consolecmds[i].value = true;
                else if (!strcasecmp(parm, "off") || !strcasecmp(parm, "no")
                    || !strcasecmp(parm, "false") || !strcasecmp(parm, "0"))
                    *(boolean *)consolecmds[i].value = false;

                M_SaveDefaults();

                M_snprintf(buffer, sizeof(buffer), "\"%s\" is \"%s\"", cmd, parm);
            }
            else
                M_snprintf(buffer, sizeof(buffer), "\"%s\" is \"%s\"", cmd,
                    (*(boolean *)consolecmds[i].value ? "on" : "off"));

            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

void C_NoClip(char *cmd, char *parm)
{
    M_StringCopy(consolecheat, (gamemode == commercial ? "idclip" : "idspispopd"),
        sizeof(consolecheat));
}

void C_NoTarget(char *cmd, char *parm)
{
    players[displayplayer].cheats ^= CF_NOTARGET;
    C_AddConsoleString(players[displayplayer].cheats & CF_NOTARGET ? s_STSTR_NTON : s_STSTR_NTOFF,
        output, CONSOLEOUTPUTCOLOR);
}

void C_Quit(char *cmd, char *parm)
{
    I_Quit(true);
}

void C_Summon(char *cmd, char *parm)
{
    mobj_t      *player = players[displayplayer].mo;
    fixed_t     x = player->x;
    fixed_t     y = player->y;
    angle_t     angle = player->angle >> ANGLETOFINESHIFT;
    mobj_t      *thing = P_SpawnMobj(x + 100 * finecosine[angle], y + 100 * finesine[angle],
        ONFLOORZ, P_FindDoomedNum(summoncmdtype));

    thing->angle = R_PointToAngle2(thing->x, thing->y, x, y);
}
