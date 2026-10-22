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
    General Public License for more details.

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
#include <limits.h>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <dirent.h>
#endif

#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_system.h"
#include "i_video.h"
#include "id24info.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_mapinfo.h"
#include "p_tick.h"
#include "r_things.h"
#include "sounds.h"
#include "w_wad.h"

#define MAXWORDS        32
#define WORDLEN         128
#define MAXCANDIDATES   4096
#define BODYMARK        "\x01"

typedef struct
{
    char    text[WORDLEN];
    int     group;
    int     order;
    bool    keepcase;
    bool    more;
    bool    skipfirst;
} candidate_t;

typedef struct
{
    char    word[MAXWORDS][WORDLEN];
    int     count;
} words_t;

typedef struct
{
    words_t cmd;
    words_t outer;
    int     depth;
    char    partial[WORDLEN];
    int     partialstart;
} parse_t;

typedef enum
{
    CVARS_ALL,
    CVARS_WRITABLE,
    CVARS_TOGGLEABLE
} cvarfilter_t;

typedef enum
{
    MOBJS_GIVE,
    MOBJS_KILL,
    MOBJS_RESURRECT,
    MOBJS_NAME,
    MOBJS_SPAWN
} mobjfilter_t;

static candidate_t      *candidates;
static int              numcandidates;
static int              maxcandidates;

static char             partialtext[WORDLEN];
static bool             sortalphabetically;
static const consolecmd_t   *valuecvar;
static bool             morevalues;

static struct
{
    bool    active;
    char    output[255];
    int     index;
    char    partial[WORDLEN];
    int     partialstart;
} session;

static const char *const givekeywords[] =
{
    "all", "ammo", "armor", "health", "keycards", "keys", "pistol", "powerups", "skullkeys", "weapons"
};

static const char *const killkeywords[] =
{
    "player", "monsters", "friends", "missiles"
};

static const char *const explodekeywords[] =
{
    "monsters", "missiles"
};

static const char *const removekeywords[] =
{
    "bloodsplats", "corpses", "decorations", "everything", "friends", "items", "missiles", "monsters"
};
static const char *const resurrectkeywords[] =
{
    "player", "monsters", "friends" 
};

static const char *const mapkeywords[] =
{
    "first", "last", "random"
};

static bool IsEmptyValue(const char *string)
{
    return (!string || !*string || M_StringCompare(string, EMPTYVALUE));
}

static const char *CmdName(const consolecmd_t *cmd)
{
    return (english != english_american && !IsEmptyValue(cmd->altspelling) ? cmd->altspelling : cmd->name);
}

static bool CmdIs(const consolecmd_t *cmd, const char *word)
{
    return (M_StringCompare(word, cmd->name)
        || (!IsEmptyValue(cmd->altspelling) && M_StringCompare(word, cmd->altspelling))
        || (!IsEmptyValue(cmd->alternate) && M_StringCompare(word, cmd->alternate)));
}

static const consolecmd_t *FindCmd(const char *word)
{
    for (int i = 0; *consolecmds[i].name; i++)
        if ((consolecmds[i].type == CT_CCMD || consolecmds[i].type == CT_CVAR) && CmdIs(&consolecmds[i], word))
            return &consolecmds[i];

    return NULL;
}

static const consolecmd_t *FindCVAR(const char *word)
{
    const consolecmd_t  *cmd = FindCmd(word);

    return (cmd && cmd->type == CT_CVAR ? cmd : NULL);
}

//
// Does a CVAR's current value equal the text that is about to be offered for it?
//
static bool MatchesValue(const consolecmd_t *cvar, const char *text)
{
    const int   flags = cvar->flags;

    if (flags & (CF_STRING | CF_OTHER))
    {
        const char      *value = *(char **)cvar->variable;
        const size_t    len = strlen(text);

        return (M_StringCompare(text, value)
            || (len >= 2 && text[0] == '"' && text[len - 1] == '"'
                && strlen(value) == len - 2 && !strncasecmp(text + 1, value, len - 2)));
    }
    else if (flags & (CF_BOOLEAN | CF_INTEGER | CF_FLOAT))
    {
        double      value;
        double      current;
        char        buffer[255];
        char        *end;
        const int   type = (cvar->variable == &r_gamma ? GAMMAVALUEALIAS : cvar->aliases);

        if (flags & CF_BOOLEAN)
            current = *(bool *)cvar->variable;
        else if (flags & CF_FLOAT)
            current = *(float *)cvar->variable;
        else
        {
            int integer = *(int *)cvar->variable;

            if (gamestate == GS_LEVEL)
            {
                if (M_StringCompare(cvar->name, "ammo"))
                {
                    const ammotype_t    ammotype = weaponinfo[viewplayer->readyweapon].ammotype;

                    integer = (ammotype == am_noammo ? 0 : viewplayer->ammo[ammotype]);
                }
                else if (M_StringCompare(cvar->name, "armor"))
                    integer = viewplayer->armor;
                else if (M_StringCompare(cvar->name, "armortype"))
                    integer = viewplayer->armortype;
                else if (M_StringCompare(cvar->name, "health"))
                    integer = (negativehealth && minuspatch && !viewplayer->health ?
                        viewplayer->negativehealth : viewplayer->health);
                else if (M_StringCompare(cvar->name, "weapon"))
                    integer = viewplayer->readyweapon;
            }

            current = integer;
        }

        for (int i = 0; *valuealiases[i].text; i++)
            if (valuealiases[i].type == type && M_StringCompare(text, valuealiases[i].text))
                return (current == valuealiases[i].value);

        M_StringCopy(buffer, text, sizeof(buffer));
        M_StringReplaceAll(buffer, ",", "", false);
        value = strtod(buffer, &end);

        return (end != buffer && (!*end || ((flags & CF_PERCENT) && M_StringCompare(end, "%")))
            && value == current);
    }

    return false;
}

// Replicates the rules the toggle CCMD uses to decide whether a CVAR can be toggled.
static bool CanToggle(const consolecmd_t *cmd)
{
    if (cmd->type != CT_CVAR || (cmd->flags & CF_READONLY))
        return false;

    if (cmd->flags & CF_BOOLEAN)
        return true;

    if (!(cmd->flags & CF_INTEGER) || !cmd->aliases || cmd->maximumvalue <= cmd->minimumvalue)
        return false;

    for (int value = cmd->minimumvalue; value <= cmd->maximumvalue; value++)
    {
        bool    found = false;

        for (int i = 0; *valuealiases[i].text; i++)
            if (valuealiases[i].type == cmd->aliases && valuealiases[i].value == value)
            {
                found = true;
                break;
            }

        if (!found)
            return false;
    }

    return true;
}

//
// Candidate list
//
static bool HasCandidate(const char *text)
{
    for (int i = 0; i < numcandidates; i++)
        if (M_StringCompare(candidates[i].text, text))
            return true;

    return false;
}

static void AddCandidate(const char *text, const int group, const bool keepcase, const bool more)
{
    candidate_t *candidate;

    if (!*text || strlen(text) >= WORDLEN || !M_StringStartsWith(text, partialtext) || HasCandidate(text))
        return;

    if (numcandidates >= MAXCANDIDATES)
        return;

    if (numcandidates >= maxcandidates)
    {
        maxcandidates += 256;
        candidates = I_Realloc(candidates, maxcandidates * sizeof(*candidates));
    }

    candidate = &candidates[numcandidates];
    M_StringCopy(candidate->text, text, sizeof(candidate->text));
    candidate->group = group;
    candidate->order = numcandidates++;
    candidate->keepcase = keepcase;
    candidate->more = more;

    // Don't offer what has already been typed, or a CVAR's current value, as the first suggestion.
    candidate->skipfirst = (*partialtext ? M_StringCompare(text, partialtext) :
        (valuecvar && MatchesValue(valuecvar, text)));
}

// Adds a word that is part of the console's own vocabulary, in the player's chosen spelling.
static void AddWord(const char *text, const int group, const bool more)
{
    char    buffer[WORDLEN * 2];

    M_StringCopy(buffer, text, sizeof(buffer));

    if (english != english_american)
        M_AmericanToBritishEnglish(buffer);

    AddCandidate(buffer, group, false, more);
}

static void AddWords(const char *const *words, const int count, const int group, const bool more)
{
    for (int i = 0; i < count; i++)
        AddWord(words[i], group, more);
}

static int NaturalCompare(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (isdigit((unsigned char)*a) && isdigit((unsigned char)*b))
        {
            const char  *a2;
            const char  *b2;
            size_t      alen;
            size_t      blen;
            int         result;

            while (*a == '0' && isdigit((unsigned char)a[1]))
                a++;

            while (*b == '0' && isdigit((unsigned char)b[1]))
                b++;

            a2 = a;
            b2 = b;

            while (isdigit((unsigned char)*a2))
                a2++;

            while (isdigit((unsigned char)*b2))
                b2++;

            alen = a2 - a;
            blen = b2 - b;

            if (alen != blen)
                return (alen < blen ? -1 : 1);

            if ((result = strncmp(a, b, alen)))
                return result;

            a = a2;
            b = b2;
        }
        else
        {
            const int   ca = tolower((unsigned char)*a);
            const int   cb = tolower((unsigned char)*b);

            if (ca != cb)
                return (ca - cb);

            a++;
            b++;
        }
    }

    return (*a ? 1 : (*b ? -1 : 0));
}

static int CompareCandidates(const void *p1, const void *p2)
{
    const candidate_t   *a = (const candidate_t *)p1;
    const candidate_t   *b = (const candidate_t *)p2;

    if (a->group != b->group)
        return (a->group - b->group);

    if (sortalphabetically)
    {
        const int   result = NaturalCompare(a->text, b->text);

        if (result)
            return result;
    }

    return (a->order - b->order);
}

//
// Providers
//
static void AddAliasNames(void)
{
    for (int i = 0; i < MAXALIASES; i++)
        if (*aliases[i].name)
            AddCandidate(aliases[i].name, 0, true, false);
}

static void AddCommandNames(void)
{
    for (int i = 0; *consolecmds[i].name; i++)
        if (consolecmds[i].type == CT_CCMD || consolecmds[i].type == CT_CVAR)
            AddCandidate(CmdName(&consolecmds[i]), 0, false, consolecmds[i].parameters);

    AddAliasNames();
}

static void AddActions(const int group)
{
    for (int i = 0; *actions[i].action; i++)
        AddCandidate((english == english_american ? actions[i].action : actions[i].oldaction),
            group, false, false);
}

static void AddControls(const int group)
{
    for (int i = 0; controls[i].type; i++)
        if (!M_StringCompare(controls[i].control, "="))
            AddCandidate(controls[i].control, group, false, false);
}

static void AddCVARNames(const cvarfilter_t filter, const bool more)
{
    for (int i = 0; *consolecmds[i].name; i++)
    {
        const consolecmd_t  *cvar = &consolecmds[i];

        if (cvar->type != CT_CVAR
            || (filter == CVARS_WRITABLE && (cvar->flags & CF_READONLY))
            || (filter == CVARS_TOGGLEABLE && !CanToggle(cvar)))
            continue;

        AddCandidate(CmdName(cvar), 0, false, more);
    }
}

// Offers wildcards such as "r_*" or "vid_window*" for CVAR prefixes shared by at least two CVARs.
static void AddCVARWildcards(void)
{
    for (int i = 0; *consolecmds[i].name; i++)
    {
        const char  *name;

        if (consolecmds[i].type != CT_CVAR)
            continue;

        name = CmdName(&consolecmds[i]);

        for (const char *underscore = strchr(name, '_'); underscore; underscore = strchr(underscore + 1, '_'))
        {
            const int   len = (int)(underscore - name) + 1;
            char        pattern[WORDLEN];
            int         count = 0;

            if (len + 2 > WORDLEN)
                break;

            memcpy(pattern, name, len);
            pattern[len] = '*';
            pattern[len + 1] = '\0';

            if (!M_StringStartsWith(pattern, partialtext) || HasCandidate(pattern))
                continue;

            for (int j = 0; *consolecmds[j].name && count < 2; j++)
                if (consolecmds[j].type == CT_CVAR && !strncasecmp(CmdName(&consolecmds[j]), name, len))
                    count++;

            if (count >= 2)
                AddCandidate(pattern, 1, false, false);
        }
    }
}

static void AddValueAliases(const valuealiastype_t type)
{
    for (int i = 0; *valuealiases[i].text; i++)
    {
        const valuealias_t  *alias = &valuealiases[i];

        if (alias->type != type)
            continue;

        // Without anything typed yet, only offer the first alias of each value.
        if (!*partialtext)
        {
            bool    duplicate = false;

            for (int j = 0; j < i; j++)
                if (valuealiases[j].type == type && valuealiases[j].value == alias->value)
                {
                    duplicate = true;
                    break;
                }

            if (duplicate)
                continue;
        }

        if (type == WEAPONVALUEALIAS)
        {
            if (M_StringCompare(alias->text, "supershotgun") && gamemode != commercial)
                continue;

            if ((M_StringCompare(alias->text, "incinerator") || M_StringCompare(alias->text, "calamityblade"))
                && !legacyofrust && !id24res)
                continue;
        }

        AddWord(alias->text, 0, morevalues);
    }
}

static void AddNumber(const int value, const bool percent)
{
    char    buffer[32];

    M_snprintf(buffer, sizeof(buffer), "%i%s", value, (percent ? "%" : ""));
    AddWord(buffer, 0, morevalues);
}

static void AddCVARValues(const consolecmd_t *cvar, const bool direct, const bool more)
{
    const int   flags = cvar->flags;

    if (direct && (flags & CF_READONLY))
        return;

    sortalphabetically = false;
    valuecvar = (direct ? cvar : NULL);
    morevalues = more;

    if (M_StringCompare(cvar->name, "ammo"))
    {
        if (gamestate == GS_LEVEL)
        {
            const ammotype_t    ammotype = weaponinfo[viewplayer->readyweapon].ammotype;

            if (ammotype != am_noammo)
                AddNumber(viewplayer->maxammo[ammotype], false);
        }
    }
    else if (M_StringCompare(cvar->name, "armor"))
    {
        AddNumber(0, true);
        AddNumber(100, true);
        AddNumber(200, true);
    }
    else if (M_StringCompare(cvar->name, "health"))
    {
        AddNumber(100, true);
        AddNumber(200, true);
    }
    else if (cvar->variable == &r_gamma)
    {
        AddValueAliases(GAMMAVALUEALIAS);

        for (int i = 0; i < GAMMALEVELS; i++)
        {
            char    *temp = striptrailingzero(gammalevels[i], 2);

            AddWord(temp, 0, morevalues);
            free(temp);
        }
    }
    else if (flags & CF_BOOLEAN)
        AddValueAliases(cvar->aliases ? (valuealiastype_t)cvar->aliases : BOOLVALUEALIAS);
    else if (flags & CF_INTEGER)
    {
        if (cvar->aliases)
            AddValueAliases((valuealiastype_t)cvar->aliases);
        else
        {
            const int   min = cvar->minimumvalue;
            const int   max = cvar->maximumvalue;
            const bool  percent = ((flags & CF_PERCENT) != 0);

            if (min > INT_MIN && max < INT_MAX && max > min && max - min <= 10)
                for (int value = min; value <= max; value++)
                    AddNumber(value, percent);
            else
            {
                const int   value = (int)cvar->defaultnumber;

                if (min > INT_MIN)
                    AddNumber(min, percent);

                if (value >= min && value <= max)
                    AddNumber(value, percent);

                if (max < INT_MAX)
                    AddNumber(max, percent);
            }
        }
    }
    else if (flags & CF_FLOAT)
    {
        char    *temp = striptrailingzero(cvar->defaultnumber, 2);
        char    buffer[64];

        M_snprintf(buffer, sizeof(buffer), "%s%s", temp, ((flags & CF_PERCENT) ? "%" : ""));
        AddWord(buffer, 0, morevalues);
        free(temp);
    }
    else if (flags & (CF_STRING | CF_OTHER))
    {
        if (M_StringCompare(cvar->name, "vid_scalefilter"))
        {
            AddWord("linear", 0, morevalues);
            AddWord("nearest", 0, morevalues);
            AddWord("nearest_linear", 0, morevalues);
        }
        else if (!IsEmptyValue(cvar->defaultstring))
            AddWord(cvar->defaultstring, 0, morevalues);
    }

    valuecvar = NULL;
    morevalues = false;
}

static bool MobjAvailable(const int i, const mobjfilter_t filter)
{
    const mobjinfo_t    *info = &mobjinfo[i];
    const int           doomednum = info->doomednum;

    if (i >= (filter == MOBJS_SPAWN ? nummobjtypes : NUMMOBJTYPES))
        return false;

    if (i >= NUMMOBJTYPES && !ID24_IsPlaceable(i))
        return false;

    switch (filter)
    {
        case MOBJS_GIVE:
            if (!(info->flags & MF_SPECIAL))
                return false;

            break;

        case MOBJS_KILL:
            if (!*info->name1 || doomednum < 0
                || !((info->flags & (MF_SHOOTABLE | MF_SPECIAL)) || (info->flags2 & MF2_DECORATION)))
                return false;

            break;

        case MOBJS_RESURRECT:
            if (!*info->name1 || doomednum < 0 || !(info->flags & MF_SHOOTABLE))
                return false;

            break;

        case MOBJS_NAME:
            if (!*info->name1 || !(info->flags & MF_SHOOTABLE) || i == MT_PLAYER || i == MT_BARREL)
                return false;

            break;

        case MOBJS_SPAWN:
            if (!*info->name1 || doomednum == -1)
                return false;

            break;
    }

    // Some things only exist in DOOM II and later.
    if (doomednum != -1)
    {
        if (gamemode != commercial && !REKKR && doomednum >= ArchVile && doomednum <= MonsterSpawner)
            return false;

        if (gamemode == shareware && (doomednum == Cyberdemon || doomednum == SpiderMastermind))
            return false;

        if (doomednum == WolfensteinSS && !allowwolfensteinss)
            return false;
    }

    return true;
}

static void AddMobjName(const char *name)
{
    if (*name)
    {
        char    *temp = removenonalpha(name);

        AddCandidate(temp, 0, false, false);
        free(temp);
    }
}

static void AddMobjNames(const mobjfilter_t filter, const bool plural, const bool friendly)
{
    for (int i = 0; i < nummobjtypes; i++)
    {
        const mobjinfo_t    *info = &mobjinfo[i];

        if (!MobjAvailable(i, filter) || (friendly && !(info->flags & MF_SHOOTABLE)))
            continue;

        AddMobjName(plural && *info->plural1 ? info->plural1 : info->name1);
        AddMobjName(plural && *info->plural2 ? info->plural2 : info->name2);
        AddMobjName(plural && *info->plural3 ? info->plural3 : info->name3);
    }
}

static void AddLiveMobjNames(void)
{
    if (gamestate != GS_LEVEL)
        return;

    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        const mobj_t    *mobj = (const mobj_t *)th;

        AddMobjName(mobj->name);
    }
}

static void AddMapTitle(const int ep, const int map)
{
    const char  *mapinfo = P_GetMapName(ep, map);
    const char  *title = NULL;

    if (map < 1)
        return;

    switch (gamemission)
    {
        case doom:
            if (map <= 9 && (ep - 1) * 9 + map - 1 >= 0 && (ep - 1) * 9 + map - 1 < nummapnames)
                title = (mapinfo && *mapinfo ? mapinfo : *mapnames[(ep - 1) * 9 + map - 1]);

            break;

        case doom2:
            if (!BTSX)
            {
                if (bfgedition && map <= nummapnames2_bfg)
                    title = (mapinfo && *mapinfo ? mapinfo : *mapnames2_bfg[map - 1]);
                else if (!bfgedition && map <= nummapnames2)
                    title = (mapinfo && *mapinfo ? mapinfo : *mapnames2[map - 1]);
            }

            break;

        case pack_nerve:
            if (map <= nummapnamesn)
                title = *mapnamesn[map - 1];

            break;

        case pack_masterlevels:
            if (map <= nummapnamesm)
                title = (mapinfo && *mapinfo ? mapinfo : *mapnamesm[map - 1]);

            break;

        case pack_plut:
            if (map <= nummapnamesp)
                title = *mapnamesp[map - 1];

            break;

        case pack_tnt:
            if (map <= nummapnamest)
                title = *mapnamest[map - 1];

            break;

        default:
            break;
    }

    if (title && *title)
        AddMobjName(title);
}

static void AddMaps(void)
{
    AddWords(mapkeywords, sizeof(mapkeywords) / sizeof(mapkeywords[0]), 0, false);

    if (gamestate != GS_TITLESCREEN)
    {
        AddWord("previous", 0, false);
        AddWord("next", 0, false);
    }

    if (gamemission == doom && gamemode != shareware)
    {
        if (!E1M4)
        {
            AddCandidate("E1M4B", 0, false, false);
            AddCandidate("phobosmissioncontrol", 0, false, false);
        }

        if (!E1M8)
        {
            AddCandidate("E1M8B", 0, false, false);
            AddCandidate("techgonebad", 0, false, false);
        }
    }

    for (int i = 0; i < numlumps; i++)
    {
        const char  *name = lumpinfo[i]->name;
        char        lump[9];
        int         ep = 1;
        int         map;

        if (gamemode == commercial)
        {
            if (toupper((unsigned char)name[0]) != 'M' || toupper((unsigned char)name[1]) != 'A'
                || toupper((unsigned char)name[2]) != 'P'
                || !isdigit((unsigned char)name[3]) || !isdigit((unsigned char)name[4]) || name[5])
                continue;

            map = (name[3] - '0') * 10 + (name[4] - '0');

            if (legacyofrust && map != 99 && map != 0)
            {
                if (map <= 7)
                    M_snprintf(lump, sizeof(lump), "E1M%i", map);
                else if (map == 15)
                    M_StringCopy(lump, "E1M0", sizeof(lump));
                else if (map == 16)
                    M_StringCopy(lump, "E2M0", sizeof(lump));
                else if (map <= 14)
                    M_snprintf(lump, sizeof(lump), "E2M%i", map - 7);
                else
                    M_snprintf(lump, sizeof(lump), "MAP%02i", map);
            }
            else
                M_snprintf(lump, sizeof(lump), "MAP%02i", map);
        }
        else
        {
            if (toupper((unsigned char)name[0]) != 'E' || !isdigit((unsigned char)name[1])
                || toupper((unsigned char)name[2]) != 'M' || !isdigit((unsigned char)name[3]) || name[4])
                continue;

            ep = name[1] - '0';
            map = name[3] - '0';
            M_snprintf(lump, sizeof(lump), "E%iM%i", ep, map);
        }

        AddCandidate(lump, 0, false, false);
        AddMapTitle(ep, map);
    }
}

static void AddMusicLump(const int i, const char *title)
{
    char    lump[9];

    if (!*title || M_StringCompare(title, "n/a"))
        return;

    M_snprintf(lump, sizeof(lump), "h_%s", s_music[i].name2);

    if (W_CheckNumForName(lump) == -1)
        M_snprintf(lump, sizeof(lump), "d_%s", s_music[i].name2);

    if (W_CheckNumForName(lump) >= 0)
        AddMobjName(title);
}

static void AddSoundsAndMusic(void)
{
    for (int i = 1; i < NUMSFX; i++)
    {
        char    lump[9];

        if (!*s_sfx[i].name2)
            continue;

        M_snprintf(lump, sizeof(lump), "ds%s", s_sfx[i].name2);

        if (W_CheckNumForName(lump) >= 0)
            AddCandidate(lump, 0, false, false);
    }

    for (int i = 1; i < NUMMUSIC; i++)
    {
        if (*s_music[i].name2)
        {
            char    lump[9];

            M_snprintf(lump, sizeof(lump), "h_%s", s_music[i].name2);

            if (W_CheckNumForName(lump) == -1)
                M_snprintf(lump, sizeof(lump), "d_%s", s_music[i].name2);

            if (W_CheckNumForName(lump) >= 0)
                AddCandidate(lump, 0, false, false);
        }

        AddMusicLump(i, s_music[i].title1);

        if (!M_StringCompare(s_music[i].title1, s_music[i].title2))
            AddMusicLump(i, s_music[i].title2);

        if (!M_StringCompare(s_music[i].title1, s_music[i].title3)
            && !M_StringCompare(s_music[i].title2, s_music[i].title3))
            AddMusicLump(i, s_music[i].title3);
    }
}

// Offers the files in a folder with the given extension, without the extension.
static void AddFiles(const char *folder, const char *extension)
{
    const size_t    extensionlen = strlen(extension);

#if defined(_WIN32)
    WIN32_FIND_DATA data;
    char            path[MAX_PATH];
    HANDLE          handle;

    M_snprintf(path, sizeof(path), "%s*%s", folder, extension);

    if ((handle = FindFirstFile(path, &data)) == INVALID_HANDLE_VALUE)
        return;

    do
    {
        const size_t    len = strlen(data.cFileName);
        char            name[WORDLEN];

        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            || len <= extensionlen || len >= WORDLEN
            || !M_StringEndsWith(data.cFileName, extension))
            continue;

        M_StringCopy(name, data.cFileName, sizeof(name));
        name[len - extensionlen] = '\0';
        AddCandidate(name, 0, true, false);
    } while (FindNextFile(handle, &data));

    FindClose(handle);
#else
    DIR             *dir = opendir(*folder ? folder : ".");
    struct dirent   *entry;

    if (!dir)
        return;

    while ((entry = readdir(dir)))
    {
        const size_t    len = strlen(entry->d_name);
        char            name[WORDLEN];

        if (len <= extensionlen || len >= WORDLEN || !M_StringEndsWith(entry->d_name, extension))
            continue;

        M_StringCopy(name, entry->d_name, sizeof(name));
        name[len - extensionlen] = '\0';
        AddCandidate(name, 0, true, false);
    }

    closedir(dir);
#endif
}

//
// Context resolution
//
static void ResolveCommand(const char (*w)[WORDLEN], const int n);

static void ResolveBody(const char (*w)[WORDLEN], const int b, const int a, const bool allowelse)
{
    if (a == b)
        ResolveCommand(w + b, 0);
    else if (M_StringCompare(w[b], BODYMARK))
    {
        if (!allowelse)
            return;

        if (a == b + 1)
            AddWord("else", 0, true);
        else if (M_StringCompare(w[b + 1], "else"))
            ResolveBody(w, b + 2, a, false);
    }
    else
    {
        // an unquoted body ends at "else", if there is one
        for (int i = b + 1; i < a; i++)
            if (M_StringCompare(w[i], "else"))
            {
                if (allowelse)
                    ResolveBody(w, i + 1, a, false);

                return;
            }

        ResolveCommand(w + b, a - b);

        if (allowelse && a > b)
            AddWord("else", 2, true);
    }
}

static void ResolveIf(const char (*w)[WORDLEN], const int a)
{
    const consolecmd_t  *cvar;
    int                 value;

    if (!a)
    {
        AddCVARNames(CVARS_ALL, true);
        return;
    }

    if (!(cvar = FindCVAR(w[0])))
        return;

    if (a == 1)
    {
        AddCVARValues(cvar, false, true);
        AddWord("is", 1, true);
        AddWord("isn't", 1, true);
        return;
    }

    if (M_StringCompare(w[1], "is"))
    {
        value = 2;

        if (a == 2)
        {
            AddCVARValues(cvar, false, true);
            AddWord("not", 1, true);
            return;
        }

        if (M_StringCompare(w[2], "not"))
            value = 3;
    }
    else
        value = (M_StringCompare(w[1], "isn't") ? 2 : 1);

    if (a == value)
        AddCVARValues(cvar, false, true);
    else if (a == value + 1)
        AddWord("then", 0, true);
    else if (M_StringCompare(w[value + 1], "then"))
        ResolveBody(w, value + 2, a, true);
}

static void ResolveBind(const char (*w)[WORDLEN], const int a)
{
    if (!a)
        AddControls(0);
    else if (a == 1)
    {
        if (!*partialtext || *partialtext == '+')
            AddActions(0);
        else
            ResolveCommand(w + 1, 0);
    }
    else if (w[1][0] != '+')
        ResolveCommand(w + 1, a - 1);
}

static void ResolveKill(const consolecmd_t *cmd, const char (*w)[WORDLEN], const int a)
{
    const char  *name = cmd->name;
    bool        all = false;
    bool        flagged = false;
    int         i = 0;

    if (a > i && M_StringCompare(w[i], "all"))
    {
        all = true;
        i++;
    }

    if (a > i && (M_StringCompare(w[i], "friendly") || M_StringCompare(w[i], "unfriendly")))
    {
        flagged = true;
        i++;
    }

    if (a != i)
        return;

    if (!flagged)
    {
        AddWord("friendly", 0, true);
        AddWord("unfriendly", 0, true);
    }

    if (!all && !flagged)
    {
        AddWord("all", 0, true);

        if (M_StringCompare(name, "kill"))
            AddWords(killkeywords, sizeof(killkeywords) / sizeof(killkeywords[0]), 0, false);
        else if (M_StringCompare(name, "explode"))
            AddWords(explodekeywords, sizeof(explodekeywords) / sizeof(explodekeywords[0]), 0, false);
        else if (M_StringCompare(name, "remove"))
            AddWords(removekeywords, sizeof(removekeywords) / sizeof(removekeywords[0]), 0, false);
        else
            AddWords(resurrectkeywords, sizeof(resurrectkeywords) / sizeof(resurrectkeywords[0]), 0, false);
    }

    if (M_StringCompare(name, "resurrect"))
        AddMobjNames(MOBJS_RESURRECT, true, false);
    else
    {
        AddMobjNames(MOBJS_KILL, true, false);
        AddLiveMobjNames();
    }
}

static void ResolveSpawn(const char (*w)[WORDLEN], const int a)
{
    const bool  friendly = (a > 0 && M_StringCompare(w[0], "friendly"));
    const bool  flagged = (friendly || (a > 0 && M_StringCompare(w[0], "unfriendly")));

    if (a != (flagged ? 1 : 0))
        return;

    if (!flagged)
    {
        AddWord("friendly", 0, true);
        AddWord("unfriendly", 0, true);
    }

    AddMobjNames(MOBJS_SPAWN, false, friendly);
}

static void ResolveName(const char (*w)[WORDLEN], const int a)
{
    const bool  flagged = (a > 0 && (M_StringCompare(w[0], "friendly")
                    || M_StringCompare(w[0], "unfriendly")));

    if (a != (flagged ? 1 : 0))
        return;

    if (!flagged)
    {
        AddWord("player", 0, true);
        AddWord("friendly", 0, true);
        AddWord("unfriendly", 0, true);
    }

    AddWord("monster", 0, true);
    AddMobjNames(MOBJS_NAME, false, false);
}

static void ResolveArguments(const consolecmd_t *cmd, const char (*w)[WORDLEN], const int a)
{
    const char  *name = cmd->name;

    if (M_StringCompare(name, "alias"))
    {
        if (!a)
            AddAliasNames();
        else
            ResolveCommand(w + 1, a - 1);
    }
    else if (M_StringCompare(name, "bind"))
        ResolveBind(w, a);
    else if (M_StringCompare(name, "unbind"))
    {
        if (!a)
        {
            AddControls(0);
            AddActions(1);
        }
    }
    else if (M_StringCompare(name, "if"))
        ResolveIf(w, a);
    else if (M_StringCompare(name, "give") || M_StringCompare(name, "take"))
    {
        if (!a)
        {
            AddWords(givekeywords, sizeof(givekeywords) / sizeof(givekeywords[0]), 0, false);
            AddMobjNames(MOBJS_GIVE, false, false);
        }
    }
    else if (M_StringCompare(name, "kill") || M_StringCompare(name, "explode")
        || M_StringCompare(name, "remove") || M_StringCompare(name, "resurrect"))
        ResolveKill(cmd, w, a);
    else if (M_StringCompare(name, "spawn"))
        ResolveSpawn(w, a);
    else if (M_StringCompare(name, "name"))
        ResolveName(w, a);
    else if (M_StringCompare(name, "play"))
    {
        if (!a)
            AddSoundsAndMusic();
    }
    else if (M_StringCompare(name, "map"))
    {
        if (!a)
            AddMaps();
    }
    else if (M_StringCompare(name, "toggle"))
    {
        if (!a)
            AddCVARNames(CVARS_TOGGLEABLE, false);
    }
    else if (M_StringCompare(name, "reset"))
    {
        if (!a)
        {
            AddWord("all", 0, false);
            AddCVARNames(CVARS_WRITABLE, false);
            AddCVARWildcards();
        }
    }
    else if (M_StringCompare(name, "print"))
        AddCVARNames(CVARS_ALL, false);
    else if (M_StringCompare(name, "cvarlist"))
    {
        if (!a)
            AddCVARWildcards();
    }
    else if (M_StringCompare(name, "exec"))
    {
        if (!a)
            AddFiles("", ".cfg");
    }
    else if (M_StringCompare(name, "condump"))
    {
        if (!a)
            AddFiles("", ".txt");
    }
    else if (M_StringCompare(name, "load") || M_StringCompare(name, "save"))
    {
        if (!a)
        {
            for (int i = 1; i <= 8; i++)
                AddNumber(i, false);

            AddFiles(savegamefolder, ".save");
        }
    }
    else if (!a && cmd->format && strstr(cmd->format, "off"))
    {
        sortalphabetically = false;
        AddValueAliases(BOOLVALUEALIAS);
    }
}

// n is the number of completed words, the first of which is the command.
static void ResolveCommand(const char (*w)[WORDLEN], const int n)
{
    const consolecmd_t  *cmd;

    if (!n)
    {
        AddCommandNames();

        if (*partialtext == '+')
            AddActions(1);

        return;
    }

    if (!(cmd = FindCmd(w[0])))
        return;

    if (cmd->type == CT_CVAR)
    {
        if (n == 1)
            AddCVARValues(cmd, true, false);
    }
    else
        ResolveArguments(cmd, w + 1, n - 1);
}

//
// Parsing
//
static bool IsOpenQuote(const char c)
{
    return (c == '"' || c == '\x93');
}

static bool IsCloseQuote(const char c)
{
    return (c == '"' || c == '\x94');
}

// bind, unbind and alias take the rest of the line as is, so semicolons within them are literal.
static bool IsSingleCommand(const words_t *words)
{
    return (words->count > 0
        && (M_StringCompare(words->word[0], "bind") || M_StringCompare(words->word[0], "unbind")
            || M_StringCompare(words->word[0], "alias")));
}

// A quote opens a nested command only where a command is expected.
static bool CanOpenScope(const words_t *words)
{
    if (words->count < 2)
        return false;

    if (M_StringCompare(words->word[0], "bind") || M_StringCompare(words->word[0], "alias"))
        return true;

    return (M_StringCompare(words->word[0], "if")
        && (M_StringCompare(words->word[words->count - 1], "then")
            || M_StringCompare(words->word[words->count - 1], "else")));
}

static bool PushWord(words_t *words, const char *text, const int len)
{
    if (words->count >= MAXWORDS || len >= WORDLEN)
        return false;

    memcpy(words->word[words->count], text, len);
    words->word[words->count++][len] = '\0';
    return true;
}

static bool IsWordBreak(const parse_t *parse, const char c)
{
    return (isspace((unsigned char)c)
        || (c == ';' && (parse->depth || !IsSingleCommand(&parse->cmd)))
        || (parse->depth && IsCloseQuote(c)));
}

// Splits the input into the words of the command being typed, and the partial word at the end.
// Returns false if there is nothing that can be completed.
static bool Parse(const char *input, parse_t *parse)
{
    const int   len = (int)strlen(input);
    int         i = 0;

    parse->cmd.count = 0;
    parse->outer.count = 0;
    parse->depth = 0;
    parse->partial[0] = '\0';
    parse->partialstart = len;

    while (i < len)
    {
        const char  c = input[i];
        int         start = i;

        if (isspace((unsigned char)c))
        {
            i++;
            continue;
        }

        if (c == ';' && (parse->depth || !IsSingleCommand(&parse->cmd)))
        {
            parse->cmd.count = 0;
            i++;
            continue;
        }

        if (parse->depth && IsCloseQuote(c))
        {
            parse->cmd = parse->outer;
            parse->depth = 0;

            if (!PushWord(&parse->cmd, BODYMARK, 1) || ++i == len)
                return false;

            continue;
        }

        if (!parse->depth && IsOpenQuote(c))
        {
            if (CanOpenScope(&parse->cmd))
            {
                parse->outer = parse->cmd;
                parse->cmd.count = 0;
                parse->depth = 1;
                i++;
                continue;
            }

            // a quoted argument such as a message, which has nothing to complete
            i++;

            while (i < len && !IsCloseQuote(input[i]))
                i++;

            if (i >= len)
                return false;

            i++;

            if (!PushWord(&parse->cmd, input + start, i - start) || i == len)
                return false;

            continue;
        }

        while (i < len && !IsWordBreak(parse, input[i]))
            i++;

        if (i >= len)
        {
            if (i - start >= WORDLEN)
                return false;

            memcpy(parse->partial, input + start, i - start);
            parse->partial[i - start] = '\0';
            parse->partialstart = start;
            break;
        }

        if (!PushWord(&parse->cmd, input + start, i - start))
            return false;
    }

    return true;
}

//
// Output
//
static bool BuildOutput(const int index, char *output, const size_t size)
{
    const candidate_t   *candidate = &candidates[index];
    const size_t        partiallen = strlen(session.partial);
    char                text[WORDLEN];

    M_StringCopy(text, candidate->text, sizeof(text));

    if (*session.partial && !candidate->keepcase)
    {
        // match the case of what was typed, keeping the typed characters as they are
        if (isuppercase(session.partial))
            for (char *p = text; *p; p++)
                *p = (char)toupper((unsigned char)*p);
        else if (islowercase(session.partial))
            for (char *p = text; *p; p++)
                *p = (char)tolower((unsigned char)*p);

        memcpy(text, session.partial, MIN((int)partiallen, (int)strlen(text)));
    }

    if (session.partialstart + strlen(text) + 2 > size)
        return false;

    memcpy(output, session.output, session.partialstart);
    output[session.partialstart] = '\0';
    M_StringCopy(output + session.partialstart, text, size - session.partialstart);

    if (numcandidates == 1 && candidate->more)
        M_StringCopy(output + strlen(output), " ", size - strlen(output));

    return true;
}

bool C_Autocomplete(char *input, const size_t size, int *caret, const bool reverse)
{
    char    output[255];

    if (!*input)
        return false;

    if (session.active && numcandidates > 1 && !strcmp(input, session.output))
        session.index = (session.index + (reverse ? numcandidates - 1 : 1)) % numcandidates;
    else
    {
        static parse_t  parse;

        session.active = false;

        if (!Parse(input, &parse))
            return false;

        M_StringCopy(partialtext, parse.partial, sizeof(partialtext));
        numcandidates = 0;
        sortalphabetically = true;
        valuecvar = NULL;
        morevalues = false;

        ResolveCommand((const char (*)[WORDLEN])parse.cmd.word, parse.cmd.count);

        if (!numcandidates)
            return false;

        qsort(candidates, numcandidates, sizeof(*candidates), CompareCandidates);

        M_StringCopy(session.output, input, sizeof(session.output));
        M_StringCopy(session.partial, parse.partial, sizeof(session.partial));
        session.partialstart = parse.partialstart;
        session.index = (reverse ? numcandidates - 1 : 0);

        for (int i = 0; i < numcandidates; i++)
        {
            const int   index = (reverse ? numcandidates - 1 - i : i);

            if (!candidates[index].skipfirst)
            {
                session.index = index;
                break;
            }
        }
    }

    if (!BuildOutput(session.index, output, sizeof(output)) || !strcmp(output, input))
    {
        session.active = false;
        return false;
    }

    M_StringCopy(input, output, size);
    M_StringCopy(session.output, output, sizeof(session.output));
    session.active = (numcandidates > 1);
    *caret = (int)strlen(input);
    return true;
}
