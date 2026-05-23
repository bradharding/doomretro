/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

    This file is a part of DOOM Retro.

    DOOM Retro is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the license, or (at your
    option) any later version.

    DOOM Retro is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

    DOOM is a registered trademark of id Software LLC, a ZeniMax Media
    company, in the US and/or other countries, and is used without
    permission. All other trademarks are the property of their respective
    holders. DOOM Retro is in no way affiliated with nor endorsed by
    id Software.

==============================================================================
*/

#include <ctype.h>

#include "c_console.h"
#include "d_deh.h"
#include "d_main.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_mapinfo.h"
#include "p_setup.h"
#include "sc_man.h"
#include "w_wad.h"

#define NUMLIQUIDS  256

enum
{
    MCMD_ALLOWMONSTERTELEFRAGS = 1,
    MCMD_AUTHOR,
    MCMD_BOSSACTION,
    MCMD_CLUSTER,
    MCMD_COMPAT_CORPSEGIBS,
    MCMD_COMPAT_FLOORMOVE,
    MCMD_COMPAT_LIGHT,
    MCMD_COMPAT_LIMITPAIN,
    MCMD_COMPAT_NOPASSOVER,
    MCMD_COMPAT_STAIRS,
    MCMD_COMPAT_USEBLOCKING,
    MCMD_COMPAT_VILEGHOSTS,
    MCMD_COMPAT_ZOMBIE,
    MCMD_ENDBUNNY,
    MCMD_ENDCAST,
    MCMD_ENDGAME,
    MCMD_ENDPIC,
    MCMD_ENTERANIM,
    MCMD_ENTERPIC,
    MCMD_EPISODE,
    MCMD_EXITANIM,
    MCMD_EXITPIC,
    MCMD_EXPANSION,
    MCMD_INTERBACKDROP,
    MCMD_INTERMUSIC,
    MCMD_INTERTEXT,
    MCMD_INTERTEXTSECRET,
    MCMD_LABEL,
    MCMD_LEVELNAME,
    MCMD_LEVELPIC,
    MCMD_LIQUID,
    MCMD_MUSIC,
    MCMD_MUSICARTIST,
    MCMD_MUSICTITLE,
    MCMD_NEXT,
    MCMD_NEXTSECRET,
    MCMD_NOBRIGHTMAP,
    MCMD_NOFREELOOK,
    MCMD_NOGRADUALLIGHTING,
    MCMD_NOINTERMISSION,
    MCMD_NOJUMP,
    MCMD_NOLIQUID,
    MCMD_NOMOUSELOOK,
    MCMD_PAR,
    MCMD_PARTIME,
    MCMD_PISTOLSTART,
    MCMD_SECRETNEXT,
    MCMD_SKY1,
    MCMD_SKYTEXTURE,
    MCMD_TITLEPATCH
};

typedef struct
{
    int             allowmonstertelefrags;
    char            author[128];
    bossaction_t    *bossactions;
    int             cluster;
    bool            compat_corpsegibs;
    bool            compat_floormove;
    bool            compat_light;
    bool            compat_limitpain;
    bool            compat_nopassover;
    bool            compat_stairs;
    bool            compat_useblocking;
    bool            compat_zombie;
    bool            endbunny;
    bool            endcast;
    bool            endgame;
    int             endpic;
    char            enteranim[9];
    int             enteranimlump;
    int             enterpic;
    char            exitanim[9];
    int             exitanimlump;
    int             exitpic;
    char            interbackdrop[9];
    int             interbackdroplump;
    int             intermusic;
    char            intertext[1024];
    char            intertextsecret[1024];
    char            label[9];
    int             liquid[NUMLIQUIDS];
    int             music;
    char            musicartist[128];
    char            musictitle[128];
    char            name[128];
    int             next;
    bool            nofreelook;
    bool            nograduallighting;
    bool            nojump;
    int             noliquid[NUMLIQUIDS];
    int             numbossactions;
    int             par;
    bool            pistolstart;
    bool            secret;
    int             secretnext;
    int             sky1texture;
    float           sky1scrolldelta;
    int             titlepatch;
} mapinfo_t;

typedef enum
{
    MAPINFOEXPANSION_HELLONEARTH,
    MAPINFOEXPANSION_NERVE,
    MAPINFOEXPANSION_MASTERLEVELS,
    NUMMAPINFOEXPANSIONS
} mapinfoexpansion_t;

static void P_InitMapInfoEntry(mapinfo_t *info);
static mapinfoexpansion_t P_GetMapInfoExpansionFromGameMission(void);
static bool P_IsMatchingExpansionWAD(const char *path, mapinfoexpansion_t expansion);
static int P_CheckNumForNameForExpansionFromTo(const char *name, mapinfoexpansion_t expansion, int from, int to);
static int P_CheckNumForNameForExpansion(const char *name, mapinfoexpansion_t expansion);
static mapinfo_t *P_GetMapInfoEntryForExpansion(mapinfoexpansion_t expansion, int ep, int map);
static mapinfo_t *P_GetMapInfoEntry(int ep, int map);
static void P_EnsureMapInfoCapacity(int new_max_map);
static void P_ParseMapString(const char *string, int *map, int *ep);
static bool P_MapInfoIsLoaded(void);

static mapinfo_t    ***mapinfo;
static int          mapinfomaxmaps;
static int          MAPINFO = -1;
static int          compat_corpsegibs_global;
static int          compat_floormove_global;
static int          compat_light_global;
static int          compat_limitpain_global;
static int          compat_nopassover_global;
static int          compat_stairs_global;
static int          compat_useblocking_global;
static int          compat_zombie_global;
static int          nograduallighting_global;
static int          liquidlumps;
static int          noliquidlumps;

char                *mapinfolump = "";
bool                nofreelook = false;
bool                nojump = false;

static char *mapcmdnames[] =
{
    "ALLOWMONSTERTELEFRAGS",
    "AUTHOR",
    "BOSSACTION",
    "CLUSTER",
    "COMPAT_CORPSEGIBS",
    "COMPAT_FLOORMOVE",
    "COMPAT_LIGHT",
    "COMPAT_LIMITPAIN",
    "COMPAT_NOPASSOVER",
    "COMPAT_STAIRS",
    "COMPAT_USEBLOCKING",
    "COMPAT_VILEGHOSTS",
    "COMPAT_ZOMBIE",
    "ENDBUNNY",
    "ENDCAST",
    "ENDGAME",
    "ENDPIC",
    "ENTERANIM",
    "ENTERPIC",
    "EPISODE",
    "EXITANIM",
    "EXITPIC",
    "EXPANSION",
    "INTERBACKDROP",
    "INTERMUSIC",
    "INTERTEXT",
    "INTERTEXTSECRET",
    "LABEL",
    "LEVELNAME",
    "LEVELPIC",
    "LIQUID",
    "MUSIC",
    "MUSICARTIST",
    "MUSICTITLE",
    "NEXT",
    "NEXTSECRET",
    "NOBRIGHTMAP",
    "NOFREELOOK",
    "NOGRADUALLIGHTING",
    "NOINTERMISSION",
    "NOJUMP",
    "NOLIQUID",
    "NOMOUSELOOK",
    "PAR",
    "PARTIME",
    "PISTOLSTART",
    "SECRETNEXT",
    "SKY1",
    "SKYTEXTURE",
    "TITLEPATCH",
    NULL
};

static int mapcmdids[] =
{
    MCMD_ALLOWMONSTERTELEFRAGS,
    MCMD_AUTHOR,
    MCMD_BOSSACTION,
    MCMD_CLUSTER,
    MCMD_COMPAT_CORPSEGIBS,
    MCMD_COMPAT_FLOORMOVE,
    MCMD_COMPAT_LIGHT,
    MCMD_COMPAT_LIMITPAIN,
    MCMD_COMPAT_NOPASSOVER,
    MCMD_COMPAT_STAIRS,
    MCMD_COMPAT_USEBLOCKING,
    MCMD_COMPAT_VILEGHOSTS,
    MCMD_COMPAT_ZOMBIE,
    MCMD_ENDBUNNY,
    MCMD_ENDCAST,
    MCMD_ENDGAME,
    MCMD_ENDPIC,
    MCMD_ENTERANIM,
    MCMD_ENTERPIC,
    MCMD_EPISODE,
    MCMD_EXITANIM,
    MCMD_EXITPIC,
    MCMD_EXPANSION,
    MCMD_INTERBACKDROP,
    MCMD_INTERMUSIC,
    MCMD_INTERTEXT,
    MCMD_INTERTEXTSECRET,
    MCMD_LABEL,
    MCMD_LEVELNAME,
    MCMD_LEVELPIC,
    MCMD_LIQUID,
    MCMD_MUSIC,
    MCMD_MUSICARTIST,
    MCMD_MUSICTITLE,
    MCMD_NEXT,
    MCMD_NEXTSECRET,
    MCMD_NOBRIGHTMAP,
    MCMD_NOFREELOOK,
    MCMD_NOGRADUALLIGHTING,
    MCMD_NOINTERMISSION,
    MCMD_NOJUMP,
    MCMD_NOLIQUID,
    MCMD_NOMOUSELOOK,
    MCMD_PAR,
    MCMD_PARTIME,
    MCMD_PISTOLSTART,
    MCMD_SECRETNEXT,
    MCMD_SKY1,
    MCMD_SKYTEXTURE,
    MCMD_TITLEPATCH
};

static void P_InitMapInfoEntry(mapinfo_t *info)
{
    info->allowmonstertelefrags = -1;
    info->author[0] = '\0';
    info->cluster = 0;
    info->compat_corpsegibs = false;
    info->compat_floormove = false;
    info->compat_light = false;
    info->compat_limitpain = false;
    info->compat_nopassover = false;
    info->compat_stairs = false;
    info->compat_useblocking = false;
    info->compat_zombie = false;
    info->endbunny = false;
    info->endcast = false;
    info->endgame = false;
    info->endpic = 0;
    info->enteranim[0] = '\0';
    info->enterpic = 0;
    info->exitanim[0] = '\0';
    info->exitpic = 0;
    info->interbackdrop[0] = '\0';
    info->intermusic = 0;
    info->intertext[0] = '\0';
    info->intertextsecret[0] = '\0';
    info->label[0] = '\0';

    for (int k = 0; k < NUMLIQUIDS; k++)
    {
        info->liquid[k] = -1;
        info->noliquid[k] = -1;
    }

    info->music = 0;
    info->musicartist[0] = '\0';
    info->musictitle[0] = '\0';
    info->name[0] = '\0';
    info->next = 0;
    info->nofreelook = false;
    info->nograduallighting = false;
    info->nojump = false;
    info->numbossactions = 0;
    info->par = 0;
    info->pistolstart = false;
    info->secret = false;
    info->secretnext = 0;
    info->sky1texture = 0;
    info->sky1scrolldelta = 0.0f;
    info->titlepatch = 0;
}

static mapinfoexpansion_t P_GetMapInfoExpansionFromGameMission(void)
{
    if (gamemode != commercial)
        return MAPINFOEXPANSION_HELLONEARTH;

    if (gamemission == pack_nerve)
        return MAPINFOEXPANSION_NERVE;

    if (gamemission == pack_masterlevels)
        return MAPINFOEXPANSION_MASTERLEVELS;

    return MAPINFOEXPANSION_HELLONEARTH;
}

static bool P_IsMatchingExpansionWAD(const char *path, const mapinfoexpansion_t expansion)
{
    switch (expansion)
    {
        case MAPINFOEXPANSION_NERVE:
            return D_IsNERVEWAD(path);

        case MAPINFOEXPANSION_MASTERLEVELS:
            return D_IsMasterLevelsWAD(path);

        case MAPINFOEXPANSION_HELLONEARTH:
        default:
            return (!D_IsNERVEWAD(path) && !D_IsMasterLevelsWAD(path));
    }
}

static int P_CheckNumForNameForExpansionFromTo(const char *name, const mapinfoexpansion_t expansion,
    const int from, const int to)
{
    int fallback = -1;
    int pwadlump = -1;
    int expansionlump = -1;
    int baselump = -1;

    if (from > to)
        return -1;

    for (int i = to; i >= from; i--)
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
        {
            char    *path = lumpinfo[i]->wadfile->path;

            if (fallback < 0)
                fallback = i;

            if (pwadlump < 0 && lumpinfo[i]->wadfile->type == PWAD && !D_IsResourceWAD(path)
                && P_IsMatchingExpansionWAD(path, expansion))
                pwadlump = i;

            switch (expansion)
            {
                case MAPINFOEXPANSION_NERVE:
                    if (expansionlump < 0 && D_IsNERVEWAD(path))
                        expansionlump = i;
                    else if (baselump < 0 && lumpinfo[i]->wadfile->type == IWAD)
                        baselump = i;

                    break;

                case MAPINFOEXPANSION_MASTERLEVELS:
                    if (expansionlump < 0 && D_IsMasterLevelsWAD(path))
                        expansionlump = i;
                    else if (baselump < 0 && lumpinfo[i]->wadfile->type == IWAD)
                        baselump = i;

                    break;

                case MAPINFOEXPANSION_HELLONEARTH:
                default:
                    if (baselump < 0 && !D_IsNERVEWAD(path) && !D_IsMasterLevelsWAD(path)
                        && !D_IsResourceWAD(path))
                        baselump = i;

                    break;
            }
        }

    return (pwadlump >= 0 ? pwadlump : (expansionlump >= 0 ? expansionlump : (baselump >= 0 ? baselump : fallback)));
}

static int P_CheckNumForNameForExpansion(const char *name, const mapinfoexpansion_t expansion)
{
    return P_CheckNumForNameForExpansionFromTo(name, expansion, 0, numlumps - 1);
}

static mapinfo_t *P_GetMapInfoEntryForExpansion(const mapinfoexpansion_t expansion, const int ep, const int map)
{
    return &mapinfo[expansion][(gamemode == commercial ? 1 : ep)][map];
}

static mapinfo_t *P_GetMapInfoEntry(const int ep, const int map)
{
    return P_GetMapInfoEntryForExpansion(P_GetMapInfoExpansionFromGameMission(), ep, map);
}

static void P_EnsureMapInfoCapacity(int new_max_map)
{
    if (new_max_map <= mapinfomaxmaps)
        return;

    for (int expansion = 0; expansion < NUMMAPINFOEXPANSIONS; expansion++)
        for (int ep = 0; ep < MAXEPISODES; ep++)
        {
            if (!mapinfo[expansion][ep])
            {
                mapinfo[expansion][ep] = (mapinfo_t *)I_Calloc(new_max_map + 1, sizeof(mapinfo_t));

                for (int i = 0; i <= new_max_map; i++)
                    P_InitMapInfoEntry(&mapinfo[expansion][ep][i]);

                continue;
            }

            mapinfo[expansion][ep] = (mapinfo_t *)I_Realloc(mapinfo[expansion][ep],
                (size_t)(new_max_map + 1) * sizeof(mapinfo_t));

            for (int i = mapinfomaxmaps + 1; i <= new_max_map; i++)
                P_InitMapInfoEntry(&mapinfo[expansion][ep][i]);
        }

    mapinfomaxmaps = new_max_map;
}

void P_InitMapInfo(void)
{
    mapinfomaxmaps = 0;
    MAPINFO = -1;
    compat_corpsegibs_global = -1;
    compat_floormove_global = -1;
    compat_light_global = -1;
    compat_limitpain_global = -1;
    compat_nopassover_global = -1;
    compat_stairs_global = -1;
    compat_useblocking_global = -1;
    compat_zombie_global = 1;
    nograduallighting_global = -1;
    liquidlumps = 0;
    noliquidlumps = 0;
    nofreelook = false;
    nojump = false;
    mapinfolump = "";
    mapinfo = (mapinfo_t ***)I_Calloc((size_t)NUMMAPINFOEXPANSIONS, sizeof(mapinfo_t **));

    for (int expansion = 0; expansion < NUMMAPINFOEXPANSIONS; expansion++)
        mapinfo[expansion] = (mapinfo_t **)I_Calloc((size_t)MAXEPISODES, sizeof(mapinfo_t *));

    P_EnsureMapInfoCapacity(100);
}

static void P_ParseMapString(const char *string, int *map, int *ep)
{
    char    *buffer = uppercase(string);
    int     value1;
    int     value2;

    if (gamemode == commercial)
    {
        if (sscanf(buffer, "MAP%d", &value1) == 1)
            *map = value1;
    }
    else
    {
        if (sscanf(buffer, "E%dM%d", &value1, &value2) == 2)
        {
            *ep = value1;
            *map = value2;
        }
    }

    free(buffer);
}

static bool P_MapInfoIsLoaded(void)
{
    return (MAPINFO >= 0);
}

bool P_MapInfoIsFromIWAD(void)
{
    return (MAPINFO >= 0 && lumpinfo[MAPINFO]->wadfile->type == IWAD);
}

bool P_ParseMapInfo(const char *scriptname)
{
    int                 maxmaps = 1;
    int                 mcmdvalue;
    mapinfo_t           *info;
    mapinfoexpansion_t  currentexpansion = MAPINFOEXPANSION_HELLONEARTH;
    char                *temp1;
    char                *temp2;

    for (MAPINFO = numlumps - 1; MAPINFO >= 0; MAPINFO--)
        if (!strncasecmp(lumpinfo[MAPINFO]->name, scriptname, 8))
        {
            char    *file = leafname(lumpinfo[MAPINFO]->wadfile->path);

            if (!D_IsNERVEWAD(file) && !D_IsMasterLevelsWAD(file) && !D_IsFinalDOOMIWAD(file)
                && !D_IsSIGILWAD(file) && !D_IsSIGIL2WAD(file))
                break;
        }

    if (MAPINFO == -1)
        return false;

    mapinfolump = uppercase(scriptname);

    SC_Open(MAPINFO);

    while (SC_GetString())
    {
        int ep = 1;
        int map = -1;

        if (SC_Compare("DEFAULTMAP"))
        {
            while (SC_GetString())
            {
                if ((mcmdvalue = SC_MatchString(mapcmdnames)) >= 0)
                {
                    switch (mapcmdids[mcmdvalue])
                    {
                        case MCMD_COMPAT_CORPSEGIBS:
                        case MCMD_COMPAT_VILEGHOSTS:
                            SC_MustGetNumber();
                            compat_corpsegibs_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_LIMITPAIN:
                            SC_MustGetNumber();
                            compat_limitpain_global = (int)sc_Number;
                            break;

                        case MCMD_NOGRADUALLIGHTING:
                            SC_MustGetNumber();
                            nograduallighting_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_FLOORMOVE:
                            SC_MustGetNumber();
                            compat_floormove_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_LIGHT:
                            SC_MustGetNumber();
                            compat_light_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_NOPASSOVER:
                            SC_MustGetNumber();
                            compat_nopassover_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_STAIRS:
                            SC_MustGetNumber();
                            compat_stairs_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_USEBLOCKING:
                            SC_MustGetNumber();
                            compat_useblocking_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_ZOMBIE:
                            SC_MustGetNumber();
                            compat_zombie_global = (int)sc_Number;
                            break;

                        case MCMD_NOJUMP:
                            nojump = true;
                            break;

                        case MCMD_NOFREELOOK:
                        case MCMD_NOMOUSELOOK:
                            nofreelook = true;
                            break;
                    }
                }
                else
                    break;
            }
        }
        else if (SC_Compare("EPISODE"))
        {
            SC_MustGetString();

            if (sscanf(sc_String, "%d", &map) != 1 || map < 0)
            {
                char    *temp = uppercase(sc_String);

                if (gamemode == commercial)
                {
                    if (sscanf(temp, "MAP%d", &map) != 1)
                    {
                        free(temp);
                        continue;
                    }
                }
                else if (sscanf(temp, "E%dM%d", &ep, &map) != 2)
                {
                    free(temp);
                    continue;
                }

                free(temp);
            }

            if (map >= 0)
            {
                SC_MustGetString();

                if (SC_Compare("NAME"))
                {
                    SC_MustGetString();
                    M_AddEpisode(map, ep, "", sc_String);
                    SC_MustGetString();
                }

                if (SC_Compare("PICNAME"))
                {
                    SC_MustGetString();
                    M_AddEpisode(map, ep, sc_String, "");
                }
            }
        }
        else if (SC_Compare("MAP"))
        {
            SC_MustGetString();

            if (sscanf(sc_String, "%d", &map) != 1 || map < 0)
            {
                char    *temp = uppercase(sc_String);

                if (gamemode == commercial)
                {
                    if (sscanf(temp, "MAP%d", &map) != 1)
                    {
                        free(temp);
                        continue;
                    }
                }
                else
                {
                    if (sscanf(temp, "E%dM%d", &ep, &map) != 2)
                    {
                        free(temp);
                        continue;
                    }
                }

                free(temp);
            }

            if (map < 0)
            {
                if (M_StringEndsWith(lumpinfo[MAPINFO]->wadfile->path, "NERVE.WAD"))
                {
                    C_Warning(1, "The map markers in PWAD " BOLD("%s") " are invalid.",
                        lumpinfo[MAPINFO]->wadfile->path);
                    nerve = false;
                    NewDef.prevmenu = &MainDef;
                    MAPINFO = -1;
                    break;
                }
                else
                {
                    C_Warning(1, "The " BOLD("MAPINFO") " lump contains an invalid map marker.");
                    continue;
                }
            }

            if (map > mapinfomaxmaps)
                P_EnsureMapInfoCapacity(map);

            info = P_GetMapInfoEntryForExpansion(currentexpansion, ep, map);

            if (compat_stairs_global == -1)
                info->compat_stairs = true;

            if (compat_zombie_global == -1)
                info->compat_zombie = true;

            while (SC_GetString())
            {
                if (SC_Compare("MAP")
                    || SC_Compare("DEFAULTMAP")
                    || SC_Compare("CLUSTERDEF")
                    || SC_Compare("EXPANSION")
                    || SC_Compare("LIQUID")
                    || SC_Compare("NOLIQUID"))
                {
                    SC_UnGet();
                    break;
                }

                if ((mcmdvalue = SC_MatchString(mapcmdnames)) >= 0)
                    switch (mapcmdids[mcmdvalue])
                    {
                        case MCMD_AUTHOR:
                            SC_MustGetString();
                            M_StringCopy(info->author, sc_String, sizeof(info->author));
                            break;

                        case MCMD_BOSSACTION:
                            SC_MustGetString();

                            if (SC_Compare("CLEAR"))
                            {
                                if (info->bossactions)
                                    free(info->bossactions);

                                info->bossactions = NULL;
                                info->numbossactions = -1;
                            }
                            else
                            {
                                int i;

                                for (i = 0; i < nummobjtypes; i++)
                                {
                                    char    *name1 = removenonalpha(mobjinfo[i].name1);
                                    char    *name2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                                    char    *name3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);
                                    char    *temp = removenonalpha(sc_String);

                                    if ((name1 && M_StringCompare(name1, temp))
                                        || (name2 && M_StringCompare(name2, temp))
                                        || (name3 && M_StringCompare(name3, temp)))
                                        break;
                                }

                                if (i < nummobjtypes)
                                {
                                    int special;
                                    int tag;

                                    SC_MustGetNumber();
                                    special = (int)sc_Number;

                                    SC_MustGetNumber();
                                    tag = (int)sc_Number;

                                    if (tag
                                        || special == S1_ExitLevel
                                        || special == S1_ExitLevel_GoesToSecretLevel
                                        || special == W1_ExitLevel
                                        || special == W1_ExitLevel_GoesToSecretLevel
                                        || (special >= W1_ExitToTheNextMapAndResetInventory
                                        && special <= G1_ExitToTheSecretMapAndResetInventory))
                                    {
                                        if (info->numbossactions == -1)
                                            info->numbossactions = 1;
                                        else
                                            info->numbossactions++;

                                        info->bossactions = (bossaction_t *)I_Realloc(info->bossactions,
                                            info->numbossactions * sizeof(bossaction_t));

                                        info->bossactions[info->numbossactions - 1].type = i;
                                        info->bossactions[info->numbossactions - 1].special = special;
                                        info->bossactions[info->numbossactions - 1].tag = tag;
                                    }
                                }
                            }

                            break;

                        case MCMD_CLUSTER:
                            SC_MustGetNumber();
                            info->cluster = (int)sc_Number;
                            break;

                        case MCMD_ENDBUNNY:
                            SC_MustGetString();

                            if (SC_Compare("TRUE"))
                                info->endbunny = true;

                            break;

                        case MCMD_ENDCAST:
                            SC_MustGetString();

                            if (SC_Compare("TRUE"))
                                info->endcast = true;

                            break;

                        case MCMD_ENDGAME:
                            SC_MustGetString();

                            if (SC_Compare("TRUE"))
                                info->endgame = true;

                            break;

                        case MCMD_ENDPIC:
                            SC_MustGetString();
                            info->endpic = P_CheckNumForNameForExpansion(sc_String, currentexpansion);
                            break;

                        case MCMD_ENTERANIM:
                            SC_MustGetString();
                            M_StringCopy(info->enteranim, sc_String, sizeof(info->enteranim));
                            break;

                        case MCMD_ENTERPIC:
                            SC_MustGetString();
                            info->enterpic = P_CheckNumForNameForExpansion(sc_String, currentexpansion);
                            break;

                        case MCMD_EPISODE:
                        {
                            char    lumpname[9];
                            char    string[128];

                            SC_MustGetString();

                            if (SC_Compare("CLEAR"))
                                break;

                            M_StringCopy(lumpname, sc_String, sizeof(lumpname));
                            SC_MustGetString();

                            if (SC_Compare("NAME"))
                            {
                                SC_MustGetString();
                                M_StringCopy(string, sc_String, sizeof(string));
                                SC_MustGetString();

                                if (SC_Compare("PICNAME"))
                                {
                                    SC_MustGetString();
                                    P_ParseMapString(lumpname, &map, &ep);
                                    M_AddEpisode(map, ep, sc_String, string);
                                }
                            }
                            else if (SC_Compare("PICNAME"))
                            {
                                SC_MustGetString();
                                M_StringCopy(string, sc_String, sizeof(string));
                                SC_MustGetString();

                                if (SC_Compare("NAME"))
                                {
                                    SC_MustGetString();
                                    P_ParseMapString(lumpname, &map, &ep);
                                    M_AddEpisode(map, ep, string, sc_String);
                                }
                            }
                            else
                            {
                                M_StringCopy(string, sc_String, sizeof(string));
                                P_ParseMapString(lumpname, &map, &ep);
                                M_AddEpisode(map, ep, lumpname, string);
                            }

                            break;
                        }

                        case MCMD_EXITANIM:
                            SC_MustGetString();
                            M_StringCopy(info->exitanim, sc_String, sizeof(info->exitanim));
                            break;

                        case MCMD_EXITPIC:
                            SC_MustGetString();
                            info->exitpic = P_CheckNumForNameForExpansion(sc_String, currentexpansion);
                            break;

                        case MCMD_INTERBACKDROP:
                            SC_MustGetString();
                            M_StringCopy(info->interbackdrop, sc_String, sizeof(info->interbackdrop));
                            break;

                        case MCMD_INTERMUSIC:
                            SC_MustGetString();
                            info->intermusic = P_CheckNumForNameForExpansion(sc_String, currentexpansion);
                            break;

                        case MCMD_LABEL:
                            SC_MustGetString();
                            M_StringCopy(info->label, sc_String, sizeof(info->label));
                            break;

                        case MCMD_INTERTEXTSECRET:
                        {
                            char    buffer[1024] = "";
                            bool    firststring = false;

                            while (SC_GetString())
                            {
                                if (SC_MatchString(mapcmdnames) >= 0 || SC_Compare("MAP"))
                                {
                                    SC_UnGet();
                                    break;
                                }

                                if (!firststring)
                                {
                                    firststring = true;
                                    M_StringCopy(buffer, sc_String, sizeof(buffer));
                                }
                                else
                                {
                                    size_t  len = strlen(buffer);

                                    if (len + 1 < sizeof(buffer))
                                    {
                                        buffer[len++] = '\n';
                                        buffer[len] = '\0';
                                    }

                                    if (len < sizeof(buffer))
                                        M_snprintf(buffer + len, (int)(sizeof(buffer) - len), "%s", sc_String);
                                }
                            }

                            M_StringCopy(info->intertextsecret, buffer, sizeof(info->intertextsecret));
                            break;
                        }

                        case MCMD_INTERTEXT:
                        {
                            char    buffer[1024] = "";
                            bool    firststring = false;

                            while (SC_GetString())
                            {
                                if (SC_MatchString(mapcmdnames) >= 0 || SC_Compare("MAP"))
                                {
                                    SC_UnGet();
                                    break;
                                }
                                else if (SC_Compare("CLEAR"))
                                {
                                    M_StringCopy(buffer, sc_String, sizeof(buffer));
                                    break;
                                }

                                if (!firststring)
                                {
                                    firststring = true;
                                    M_StringCopy(buffer, sc_String, sizeof(buffer));
                                }
                                else
                                {
                                    size_t  len = strlen(buffer);

                                    if (len + 1 < sizeof(buffer))
                                    {
                                        buffer[len++] = '\n';
                                        buffer[len] = '\0';
                                    }

                                    if (len < sizeof(buffer))
                                        M_snprintf(buffer + len, (int)(sizeof(buffer) - len), "%s", sc_String);
                                }
                            }

                            M_StringCopy(info->intertext, buffer, sizeof(info->intertext));
                            break;
                        }

                        case MCMD_LIQUID:
                        {
                            int lump;

                            SC_MustGetString();

                            if ((lump = R_CheckFlatNumForName(sc_String)) >= 0)
                                info->liquid[liquidlumps++] = lump;

                            break;
                        }

                        case MCMD_LEVELNAME:
                            SC_MustGetString();
                            M_StringCopy(info->name, sc_String, sizeof(info->name));
                            break;

                        case MCMD_MUSIC:
                            SC_MustGetString();

                            if (legacyofrust && extras)
                            {
                                sc_String[0] = 'H';

                                if (P_CheckNumForNameForExpansion(sc_String, currentexpansion) == -1)
                                {
                                    sc_String[0] = 'O';

                                    if (P_CheckNumForNameForExpansion(sc_String, currentexpansion) == -1)
                                        sc_String[0] = 'D';
                                }
                            }

                            if (!info->music)
                                info->music = P_CheckNumForNameForExpansion(sc_String, currentexpansion);

                            break;

                        case MCMD_MUSICARTIST:
                            SC_MustGetString();
                            M_StringCopy(info->musicartist, sc_String, sizeof(info->musicartist));
                            break;

                        case MCMD_MUSICTITLE:
                            SC_MustGetString();
                            M_StringCopy(info->musictitle, sc_String, sizeof(info->musictitle));
                            break;

                        case MCMD_NEXT:
                        {
                            int nextepisode = 1;
                            int nextmap = 0;

                            SC_MustGetString();

                            if (SC_Compare("ENDGAME1"))
                            {
                                info->endgame = true;
                                break;
                            }

                            if (SC_Compare("ENDGAMEC"))
                            {
                                info->endcast = true;
                                break;
                            }

                            if (sscanf(sc_String, "%d", &nextmap) != 1 || nextmap < 0)
                            {
                                char    *buffer = uppercase(sc_String);

                                if (gamemode == commercial)
                                {
                                    if (sscanf(buffer, "MAP%d", &nextmap) != 1)
                                        continue;
                                }
                                else
                                {
                                    if (sscanf(buffer, "E%dM%d", &nextepisode, &nextmap) != 2)
                                        continue;
                                }

                                free(buffer);
                            }

                            info->next = nextmap;
                            break;
                        }

                        case MCMD_NOBRIGHTMAP:
                        {
                            int texture;

                            SC_MustGetString();

                            if ((texture = R_TextureNumForName(sc_String)) >= 0)
                                nobrightmap[texture] = true;

                            break;
                        }

                        case MCMD_NOJUMP:
                            info->nojump = true;
                            break;

                        case MCMD_NOLIQUID:
                        {
                            int lump;

                            SC_MustGetString();

                            if ((lump = R_CheckFlatNumForName(sc_String)) >= 0)
                                info->noliquid[noliquidlumps++] = lump;

                            break;
                        }

                        case MCMD_NOFREELOOK:
                        case MCMD_NOMOUSELOOK:
                            info->nofreelook = true;
                            break;

                        case MCMD_PAR:
                        case MCMD_PARTIME:
                            SC_MustGetNumber();
                            info->par = (int)sc_Number;
                            break;

                        case MCMD_PISTOLSTART:
                            info->pistolstart = true;
                            break;

                        case MCMD_NEXTSECRET:
                        case MCMD_SECRETNEXT:
                        {
                            int nextepisode = 1;
                            int nextmap = 0;

                            SC_MustGetString();

                            if (sscanf(sc_String, "%d", &nextmap) != 1 || nextmap < 0)
                            {
                                char    *buffer = uppercase(sc_String);

                                if (gamemode == commercial)
                                {
                                    if (sscanf(buffer, "MAP%d", &nextmap) != 1)
                                        continue;
                                }
                                else
                                {
                                    if (sscanf(buffer, "E%dM%d", &nextepisode, &nextmap) != 2)
                                        continue;
                                }

                                free(buffer);
                            }

                            info->secretnext = nextmap;

                            if (info->next != info->secretnext)
                                P_GetMapInfoEntryForExpansion(currentexpansion, nextepisode, nextmap)->secret = true;

                            break;
                        }

                        case MCMD_SKY1:
                            SC_MustGetString();
                            info->sky1texture = R_TextureNumForName(sc_String);

                            if (SC_GetNumber())
                                info->sky1scrolldelta = sc_Number;

                            break;

                        case MCMD_SKYTEXTURE:
                            SC_MustGetString();
                            info->sky1texture = R_TextureNumForName(sc_String);
                            break;

                        case MCMD_LEVELPIC:
                        case MCMD_TITLEPATCH:
                            SC_MustGetString();
                            info->titlepatch = P_CheckNumForNameForExpansion(sc_String, currentexpansion);
                            break;

                        case MCMD_ALLOWMONSTERTELEFRAGS:
                            if (!SC_GetString())
                            {
                                info->allowmonstertelefrags = 1;
                                break;
                            }

                            if (M_StringCompare(sc_String, "0"))
                                info->allowmonstertelefrags = 0;
                            else if (M_StringCompare(sc_String, "1"))
                                info->allowmonstertelefrags = 1;
                            else
                            {
                                info->allowmonstertelefrags = 1;
                                SC_UnGet();
                            }

                            break;

                        case MCMD_COMPAT_CORPSEGIBS:
                        case MCMD_COMPAT_VILEGHOSTS:
                            SC_MustGetNumber();
                            info->compat_corpsegibs = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_LIMITPAIN:
                            SC_MustGetNumber();
                            info->compat_limitpain = !!((int)sc_Number);
                            break;

                        case MCMD_NOGRADUALLIGHTING:
                            SC_MustGetNumber();
                            info->nograduallighting = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_FLOORMOVE:
                            SC_MustGetNumber();
                            info->compat_floormove = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_LIGHT:
                            SC_MustGetNumber();
                            info->compat_light = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_NOPASSOVER:
                            SC_MustGetNumber();
                            info->compat_nopassover = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_STAIRS:
                            SC_MustGetNumber();
                            info->compat_stairs = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_USEBLOCKING:
                            SC_MustGetNumber();
                            info->compat_useblocking = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_ZOMBIE:
                            SC_MustGetNumber();
                            info->compat_zombie = !!((int)sc_Number);
                            break;
                    }
            }

            if (REKKR && ep <= 2)
                info->sky1scrolldelta = 0.05f;

            maxmaps = MAX(map, maxmaps);
        }
        else if (SC_Compare("NOJUMP"))
            nojump = true;
        else if (SC_Compare("NOFREELOOK") || SC_Compare("NOMOUSELOOK"))
            nofreelook = true;
        else if (SC_Compare("EXPANSION"))
        {
            SC_MustGetString();

            if (SC_Compare("NERVE"))
                currentexpansion = MAPINFOEXPANSION_NERVE;
            else if (SC_Compare("MASTERLEVELS"))
                currentexpansion = MAPINFOEXPANSION_MASTERLEVELS;
            else
                currentexpansion = MAPINFOEXPANSION_HELLONEARTH;
        }
        else if (SC_Compare("LIQUID"))
        {
            int lump;

            SC_MustGetString();

            if ((lump = R_CheckFlatNumForName(sc_String)) >= 0)
                terraintypes[lump] = LIQUID;
        }
        else if (SC_Compare("NOLIQUID"))
        {
            int lump;

            SC_MustGetString();

            if ((lump = R_CheckFlatNumForName(sc_String)) >= 0)
                terraintypes[lump] = SOLID;
        }
    }

    SC_Close();

    if (maxmaps > mapinfomaxmaps)
        P_EnsureMapInfoCapacity(maxmaps);

    if (customepisodes)
    {
        if (EpiDef.laston >= EpiDef.numitems)
        {
            EpiDef.laston = 0;
            episode = 1;
            M_SaveCVARs();
        }
    }

    if (legacyofrust)
        P_GetMapInfoEntryForExpansion(MAPINFOEXPANSION_HELLONEARTH, 1, 99)->secret = true;

    temp1 = commify(sc_Line);
    temp2 = uppercase(scriptname);
    C_Output("%s line%s have been parsed in the " BOLD("%s") " lump in the %s " BOLD("%s") ".",
        temp1, (sc_Line == 1 ? "" : "s"), temp2, (lumpinfo[MAPINFO]->wadfile->type == IWAD ? "IWAD" : "PWAD"),
        lumpinfo[MAPINFO]->wadfile->path);
    free(temp1);
    free(temp2);

    return true;
}

const char *P_GetMapAuthor(const int ep, const int map)
{
    mapinfo_t   *info = P_GetMapInfoEntry(ep, map);

    return (P_MapInfoIsLoaded() && info->author[0] ? info->author : (((E1M4B || *speciallumpname) && map == 4)
        || ((E1M8B || *speciallumpname) && map == 8) || (onehumanity && map == 1) ? s_AUTHOR_ROMERO : ""));
}

int P_GetNumBossActions(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->numbossactions;
}

bossaction_t *P_GetBossAction(const int ep, const int map, const int i)
{
    return &P_GetMapInfoEntry(ep, map)->bossactions[i];
}

char *P_GetInterBackrop(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->interbackdrop;
}

int P_GetInterMusic(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->intermusic;
}

char *P_GetInterText(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->intertext;
}

char *P_GetInterSecretText(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->intertextsecret;
}

char *P_GetLabel(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->label;
}

bool P_GetMapCompatCorpseGibs(const int ep, const int map)
{
    return (compat_corpsegibs_global != -1 ? compat_corpsegibs_global : P_GetMapInfoEntry(ep, map)->compat_corpsegibs);
}

bool P_GetMapCompatFloorMove(const int ep, const int map)
{
    return (compat_floormove_global != -1 ? compat_floormove_global : P_GetMapInfoEntry(ep, map)->compat_floormove);
}

bool P_GetMapCompatLight(const int ep, const int map)
{
    return (compat_light_global != -1 ? compat_light_global : P_GetMapInfoEntry(ep, map)->compat_light);
}

bool P_GetMapCompatLimitPain(const int ep, const int map)
{
    return (compat_limitpain_global != -1 ? compat_limitpain_global : P_GetMapInfoEntry(ep, map)->compat_limitpain);
}

bool P_GetMapCompatNoPassOver(const int ep, const int map)
{
    return (compat_nopassover_global != -1 ? compat_nopassover_global : P_GetMapInfoEntry(ep, map)->compat_nopassover);
}

bool P_GetMapCompatStairs(const int ep, const int map)
{
    return (compat_stairs_global != -1 ? compat_stairs_global : P_GetMapInfoEntry(ep, map)->compat_stairs);
}

bool P_GetMapCompatUseBlocking(const int ep, const int map)
{
    return (compat_useblocking_global != -1 ? compat_useblocking_global : P_GetMapInfoEntry(ep, map)->compat_useblocking);
}

bool P_GetMapCompatZombie(const int ep, const int map)
{
    return (compat_zombie_global != -1 ? compat_zombie_global : P_GetMapInfoEntry(ep, map)->compat_zombie);
}

bool P_GetMapEndBunny(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->endbunny;
}

bool P_GetMapEndCast(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->endcast;
}

bool P_GetMapEndGame(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->endgame;
}

int P_GetMapEndPic(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->endpic;
}

char *P_GetMapEnterAnim(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->enteranim;
}

int P_GetMapEnterPic(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->enterpic;
}

char *P_GetMapExitAnim(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->exitanim;
}

int P_GetMapExitPic(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->exitpic;
}

void P_GetMapLiquids(const int ep, const int map)
{
    mapinfo_t   *info = P_GetMapInfoEntry(ep, map);

    for (int i = 0; i < liquidlumps; i++)
        terraintypes[info->liquid[i]] = LIQUID;
}

int P_GetMapMusic(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->music;
}

char *P_GetMapMusicComposer(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->musicartist;
}

char *P_GetMapMusicTitle(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->musictitle;
}

char *P_GetMapName(const int ep, const int map)
{
    return (P_MapInfoIsLoaded() && !sigil ? P_GetMapInfoEntry(ep, map)->name : ((E1M4B || *speciallumpname) && map == 4 ? s_HUSTR_E1M4B :
        ((E1M8B || *speciallumpname) && map == 8 ? s_HUSTR_E1M8B : "")));
}

int P_GetMapNext(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->next;
}

bool P_GetMapNoFreelook(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->nofreelook;
}

bool P_GetMapNoGradualLighting(const int ep, const int map)
{
    return (nograduallighting_global != -1 ? nograduallighting_global : P_GetMapInfoEntry(ep, map)->nograduallighting);
}

bool P_GetMapNoJump(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->nojump;
}

void P_GetMapNoLiquids(const int ep, const int map)
{
    mapinfo_t *info = P_GetMapInfoEntry(ep, map);

    for (int i = 0; i < noliquidlumps; i++)
        terraintypes[info->noliquid[i]] = SOLID;
}

int P_GetMapPar(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->par;
}

bool P_GetMapPistolStart(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->pistolstart;
}

int P_GetMapSecretNext(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->secretnext;
}

int P_GetMapSky1Texture(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->sky1texture;
}

float P_GetMapSky1ScrollDelta(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->sky1scrolldelta;
}

int P_GetMapTitlePatch(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->titlepatch;
}

int P_GetAllowMonsterTelefrags(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->allowmonstertelefrags;
}

bool P_IsSecret(const int ep, const int map)
{
    return P_GetMapInfoEntry(ep, map)->secret;
}
