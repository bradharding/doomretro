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

#include "c_cmds.h"
#include "c_console.h"
#include "c_obituary.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_system.h"
#include "info.h"
#include "m_config.h"
#include "m_misc.h"

static const char *liquids[][2] =
{
    { "liquid",     "liquid"     },
    { "nukage",     "nukage"     },
    { "water",      "water"      },
    { "lava",       "lava"       },
    { "blood",      "blood"      },
    { "slime",      "slime"      },
    { "gray slime", "grey slime" },
    { "goop",       "goop"       },
    { "icy water",  "icy water"  },
    { "tar",        "tar"        },
    { "sludge",     "sludge"     }
};

static void C_BuildThingName(char *dest, const int destsize, const mobjtype_t type,
    const bool friendly, const bool corpse, const char *customname, const char *othername)
{
    const bool  validtype = (type > MT_NULL && type < NUMMOBJTYPES);
    char        *name = (validtype ? mobjinfo[type].name1 : "");
    const bool  validname = (name && *name && !M_StringStartsWith(name, "Deh_Actor_"));
    const char  *basename = (validname ? name : "monster");
    const bool  definite = (friendly && validtype && monstercount[type] == 1);
    const bool  vowelstart = (validname && isvowel((unsigned char)basename[0]));
    const bool  sameasother = (othername && *othername && M_StringCompare(basename, othername));
    const char  *article = (definite ? "the" : (sameasother ? "another" :
                    (vowelstart && !corpse && !friendly ? "an" : "a")));
    const char  *prefix = "";

    if (customname && *customname)
    {
        M_StringCopy(dest, customname, destsize);
        return;
    }

    if (corpse && validname && !M_StringStartsWith(basename, "dead "))
        prefix = "dead ";
    else if (friendly)
        prefix = "friendly ";

    M_snprintf(dest, destsize, "%s %s%s", article, prefix, basename);
}

static const char *C_KillVerb(const mobjtype_t target, const bool gibbed)
{
    if (target == MT_BARREL)
        return "exploded";
    else if (target == MT_LAMP && legacyofrust)
        return "broke";
    else
        return (gibbed ? s_GIBBED : s_KILLED);
}

void C_BuildObituaryString(const int index)
{
    const obituaryinfo_t    *obituary = &console[index].obituary;
    char                    *buffer = console[index].string;
    const int               buffersize = (int)sizeof(console[index].string);
    const mobjtype_t        target = obituary->target;
    const mobjtype_t        inflicter = obituary->inflicter;
    const mobjtype_t        source = obituary->source;

    if (buffer[0])
        return;

    if (obituary->telefragged)
    {
        char    sourcename[128] = "";
        char    targetname[128] = "";

        if (!obituary->sourceisplayer)
            C_BuildThingName(sourcename, sizeof(sourcename), source,
                obituary->sourcefriendly, false, obituary->sourcename, "");

        if (!obituary->targetisplayer)
            C_BuildThingName(targetname, sizeof(targetname), target,
                obituary->targetfriendly, false, obituary->targetname, "");

        if (obituary->targetisplayer)
        {
            if (isdefaultplayername())
                M_snprintf(buffer, buffersize, "You were telefragged by %s!", sourcename);
            else
                M_snprintf(buffer, buffersize, "%s was telefragged by %s!", playername, sourcename);
        }
        else if (obituary->sourceisplayer)
            M_snprintf(buffer, buffersize, "%s telefragged %s.", C_GetPlayerName(), targetname);
        else
            M_snprintf(buffer, buffersize, "%s was telefragged by %s.", targetname, sourcename);
    }
    else if (source != MT_NULL)
    {
        if (inflicter == MT_BARREL && target != MT_BARREL)
        {
            const char  *inflictername = mobjinfo[inflicter].name1;
            const bool  byplayer = (obituary->barrelinflicter == MT_PLAYER);

            if (obituary->targetisplayer)
            {
                if (byplayer)
                {
                    if (isdefaultplayername())
                        M_snprintf(buffer, buffersize, "You were %s by %s %s that you exploded!",
                            (obituary->gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"));
                    else
                        M_snprintf(buffer, buffersize, "%s was %s by %s %s that %s exploded!",
                            playername,
                            (obituary->gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"),
                            pronoun(personal));
                }
                else
                {
                    const mobjtype_t    killer = obituary->barrelinflicter;
                    const char          *killername = (killer > MT_NULL && killer < NUMMOBJTYPES ?
                                            mobjinfo[killer].name1 : "");

                    M_snprintf(buffer, buffersize, "%s were %s by %s %s that %s %s exploded!",
                        (isdefaultplayername() ? "You" : playername),
                        (obituary->gibbed ? s_GIBBED : s_KILLED),
                        (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                        (inflictername ? inflictername : "barrel"),
                        (inflicter == killer || M_StringCompare(inflictername, killername) ? "another" :
                            (*killername && isvowel(killername[0]) ? "an" : "a")),
                        (*killername && !M_StringStartsWith(killername, "Deh_Actor_") ? killername : "monster"));
                }
            }
            else
            {
                char    targetname[128];

                C_BuildThingName(targetname, sizeof(targetname), target, obituary->targetfriendly,
                    (obituary->targetcorpse && source != target), obituary->targetname, "");

                if (byplayer)
                {
                    if (isdefaultplayername())
                        M_snprintf(buffer, buffersize, "%s was %s by %s %s that you exploded.",
                            targetname, (obituary->gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"));
                    else
                        M_snprintf(buffer, buffersize, "%s was %s by %s %s that %s exploded.",
                            targetname, (obituary->gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"),
                            playername);
                }
                else if (source == target)
                    M_snprintf(buffer, buffersize, "%s was %s by %s %s that they exploded.",
                        targetname, (obituary->gibbed ? s_GIBBED : s_KILLED),
                        (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                        (inflictername ? inflictername : "barrel"));
                else
                {
                    const mobjtype_t    killer = obituary->barrelinflicter;
                    const char          *killername = (killer > MT_NULL && killer < NUMMOBJTYPES ?
                                            mobjinfo[killer].name1 : "");

                    M_snprintf(buffer, buffersize, "%s was %s by %s %s that %s %s exploded.",
                        targetname, (obituary->gibbed ? s_GIBBED : s_KILLED),
                        (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                        (inflictername ? inflictername : "barrel"),
                        (inflicter == killer || M_StringCompare(inflictername, killername) ? "another" :
                            (*killername && isvowel(killername[0]) ? "an" : "a")),
                        (*killername && !M_StringStartsWith(killername, "Deh_Actor_") ? killername : "monster"));
                }
            }
        }
        else if (obituary->sourceisplayer || source == MT_BFG)
        {
            const weapontype_t  weapon = obituary->weapon;

            if (isdefaultplayername())
            {
                if (obituary->targetisplayer)
                {
                    if (healthcvar)
                        M_snprintf(buffer, buffersize, "You %s yourself!", s_KILLED);
                    else
                        M_snprintf(buffer, buffersize, "You %s yourself with your own %s!",
                            (obituary->gibbed ? s_GIBBED : s_KILLED), weaponinfo[weapon].name);
                }
                else
                {
                    char    targetname[128];

                    C_BuildThingName(targetname, sizeof(targetname), target,
                        obituary->targetfriendly, false, obituary->targetname, "");

                    if (weapon == wp_fist && viewplayer->powers[pw_strength])
                        M_snprintf(buffer, buffersize, "You %s %s with your %s while %s.",
                            C_KillVerb(target, obituary->gibbed), targetname, weaponinfo[weapon].name, berserk);
                    else
                        M_snprintf(buffer, buffersize, "You %s %s with your %s.",
                            C_KillVerb(target, obituary->gibbed), targetname, weaponinfo[weapon].name);
                }
            }
            else
            {
                if (obituary->targetisplayer)
                {
                    if (healthcvar)
                        M_snprintf(buffer, buffersize, "%s %s %s!", playername, s_KILLED, pronoun(reflexive));
                    else
                        M_snprintf(buffer, buffersize, "%s %s %s with %s own %s!",
                            playername, (obituary->gibbed ? s_GIBBED : s_KILLED), pronoun(reflexive),
                            pronoun(possessive), weaponinfo[weapon].name);
                }
                else
                {
                    char    targetname[128];

                    C_BuildThingName(targetname, sizeof(targetname), target,
                        obituary->targetfriendly, obituary->targetcorpse, obituary->targetname, "");

                    if (weapon == wp_fist && viewplayer->powers[pw_strength])
                        M_snprintf(buffer, buffersize, "%s %s %s with %s %s while %s.",
                            playername, C_KillVerb(target, obituary->gibbed), targetname, pronoun(possessive),
                            weaponinfo[weapon].name, berserk);
                    else
                        M_snprintf(buffer, buffersize, "%s %s %s with %s %s.",
                            playername, C_KillVerb(target, obituary->gibbed), targetname, pronoun(possessive),
                            weaponinfo[weapon].name);
                }
            }
        }
        else
        {
            char    sourcename[128];

            C_BuildThingName(sourcename, sizeof(sourcename), source,
                obituary->sourcefriendly, false, obituary->sourcename, "");

            if (obituary->targetisplayer)
            {
                if (isdefaultplayername())
                    M_snprintf(buffer, buffersize, "You were %s by %s!",
                        (obituary->gibbed ? s_GIBBED : s_KILLED), sourcename);
                else
                    M_snprintf(buffer, buffersize, "%s was %s by %s!",
                        playername, (obituary->gibbed ? s_GIBBED : s_KILLED), sourcename);
            }
            else
            {
                char    targetname[128];

                C_BuildThingName(targetname, sizeof(targetname), target,
                    obituary->targetfriendly, false, obituary->targetname,
                    (source > MT_NULL && source < NUMMOBJTYPES ? mobjinfo[source].name1 : ""));

                M_snprintf(buffer, buffersize, "%s %s %s.",
                    sourcename, C_KillVerb(target, obituary->gibbed), targetname);
            }
        }
    }
    else if (obituary->targetisplayer)
    {
        if (obituary->crushed)
        {
            if (isdefaultplayername())
                M_snprintf(buffer, buffersize, "You were crushed to death!");
            else
                M_snprintf(buffer, buffersize, "%s was crushed to death!", playername);
        }
        else if (obituary->terraintype >= LIQUID && obituary->terraintype < NUMTERRAINTYPES)
            M_snprintf(buffer, buffersize, "%s died in %s.",
                C_GetPlayerName(), liquids[obituary->terraintype - LIQUID][english]);
        else
        {
            const short floorpic = obituary->floorpic;

            if ((floorpic >= RROCK05 && floorpic <= RROCK08) || (floorpic >= SLIME09 && floorpic <= SLIME12))
                M_snprintf(buffer, buffersize, "%s died on molten rock.", C_GetPlayerName());
            else
                M_snprintf(buffer, buffersize, "%s died.", C_GetPlayerName());
        }
    }
    else if (obituary->crushed)
    {
        char    targetname[128];

        C_BuildThingName(targetname, sizeof(targetname), target,
            obituary->targetfriendly, obituary->targetcorpse, obituary->targetname, "");

        M_snprintf(buffer, buffersize, "%s was crushed to death.", targetname);
    }

    if (buffer[0])
        buffer[0] = (char)toupper(buffer[0]);
}

static bool C_SameObituary(const obituaryinfo_t *a, const obituaryinfo_t *b)
{
    return (a->target == b->target
        && a->inflicter == b->inflicter
        && a->source == b->source
        && a->barrelinflicter == b->barrelinflicter
        && a->weapon == b->weapon
        && a->gibbed == b->gibbed
        && a->telefragged == b->telefragged
        && a->targetisplayer == b->targetisplayer
        && a->sourceisplayer == b->sourceisplayer
        && a->targetfriendly == b->targetfriendly
        && a->sourcefriendly == b->sourcefriendly
        && a->targetcorpse == b->targetcorpse
        && a->crushed == b->crushed
        && a->terraintype == b->terraintype
        && a->floorpic == b->floorpic
        && M_StringCompare(a->targetname, b->targetname)
        && M_StringCompare(a->sourcename, b->sourcename));
}

void C_WriteObituary(mobj_t *target, mobj_t *inflicter, mobj_t *source,
    const bool gibbed, const bool telefragged, const bool crushed)
{
    const int       i = MAX(0, numconsolestrings - 1);
    obituaryinfo_t  obituary = { 0 };

    if (target)
    {
        const int   flags = target->flags;

        obituary.target = target->type;
        obituary.targetisplayer = !!target->player;
        obituary.targetfriendly = !!(flags & MF_FRIEND);
        obituary.targetcorpse = !!(flags & MF_CORPSE);

        if (*target->name)
            M_StringCopy(obituary.targetname, target->name, sizeof(obituary.targetname));

        if (!source)
        {
            if (crushed)
                obituary.crushed = true;
            else
            {
                const sector_t  *sector = target->subsector->sector;

                obituary.terraintype = sector->terraintype;
                obituary.floorpic = sector->floorpic;
            }
        }
    }
    else
        obituary.target = MT_NULL;

    if (inflicter)
    {
        const mobjtype_t    type = inflicter->type;

        obituary.inflicter = type;
        obituary.barrelinflicter = (type == MT_BARREL ? inflicter->inflicter : MT_NULL);
    }
    else
        obituary.inflicter = MT_NULL;

    if (source)
    {
        const mobjtype_t    type = source->type;

        obituary.source = type;
        obituary.weapon = (source->player || type == MT_BFG ? viewplayer->readyweapon : wp_nochange);
        obituary.sourceisplayer = !!source->player;
        obituary.sourcefriendly = !!(source->flags & MF_FRIEND);

        if (*source->name)
            M_StringCopy(obituary.sourcename, source->name, sizeof(obituary.sourcename));
    }
    else
    {
        obituary.source = MT_NULL;
        obituary.weapon = wp_nochange;
    }

    obituary.gibbed = gibbed;
    obituary.telefragged = telefragged;

    if (numconsolestrings > 0
        && (console[i].stringtype == obituarystring || console[i].stringtype == playerobituarystring)
        && C_SameObituary(&console[i].obituary, &obituary))
    {
        console[i].obituary = obituary;
        C_CreateTimeStamp(i);
        console[i].count++;
        outputhistory = -1;

        if (obituaries && console[i].stringtype == playerobituarystring)
        {
            console[i].string[0] = '\0';
            C_BuildObituaryString(i);
            HU_SetPlayerMessage(console[i].string, false, false);
            message_warning = true;
        }

        return;
    }

    if (numconsolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    console[numconsolestrings].string[0] = '\0';
    console[numconsolestrings].wrap = 0;
    console[numconsolestrings].count = 1;
    C_CreateTimeStamp(numconsolestrings);

    console[numconsolestrings].obituary = obituary;

    if (obituary.targetisplayer)
    {
        console[numconsolestrings].stringtype = playerobituarystring;
        console[numconsolestrings].indent = WARNINGWIDTH + 2;
    }
    else
    {
        console[numconsolestrings].stringtype = obituarystring;
        console[numconsolestrings].indent = 0;
    }

    if (obituaries && console[numconsolestrings].stringtype == playerobituarystring)
    {
        C_BuildObituaryString(numconsolestrings);
        HU_SetPlayerMessage(console[numconsolestrings].string, false, false);
        message_warning = true;
    }

    numconsolestrings++;
    outputhistory = -1;
}
