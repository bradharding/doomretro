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

static void C_BuildThingName(char *dest, const int destsize, const mobjtype_t type,
    const bool friendly, const bool corpse, const char *customname)
{
    char    *name;

    if (customname && *customname)
    {
        M_StringCopy(dest, customname, destsize);
        return;
    }

    name = (type > MT_NULL && type < NUMMOBJTYPES ? mobjinfo[type].name1 : NULL);

    M_snprintf(dest, destsize, "%s %s%s",
        (friendly && type > MT_NULL && type < NUMMOBJTYPES && monstercount[type] == 1 ? "the" :
            (name && *name && isvowel(name[0]) && !corpse && !friendly ? "an" : "a")),
        (corpse && name && !M_StringStartsWith(name, "dead ") ? "dead " : (friendly ? "friendly " : "")),
        (name && *name && !M_StringStartsWith(name, "Deh_Actor_") ? name : "monster"));
}

static void C_BuildObituaryString(char *buffer, const int buffersize, const obituaryinfo_t *obituary)
{
    const mobjtype_t    target = obituary->target;
    const mobjtype_t    inflicter = obituary->inflicter;
    const mobjtype_t    source = obituary->source;
    const weapontype_t  weapon = obituary->weapon;
    const bool          gibbed = obituary->gibbed;
    const bool          telefragged = obituary->telefragged;

    buffer[0] = '\0';

    if (telefragged)
    {
        if (obituary->targetisplayer)
        {
            char    sourcename[128];

            C_BuildThingName(sourcename, sizeof(sourcename), source,
                obituary->sourcefriendly, false, obituary->sourcename);

            if (isdefaultplayername())
                M_snprintf(buffer, buffersize, "You were telefragged by %s!", sourcename);
            else
                M_snprintf(buffer, buffersize, "%s was telefragged by %s!", playername, sourcename);

            return;
        }
        else if (obituary->sourceisplayer)
        {
            char    targetname[128];

            C_BuildThingName(targetname, sizeof(targetname), target,
                obituary->targetfriendly, false, obituary->targetname);
            M_snprintf(buffer, buffersize, "%s telefragged %s.", C_GetPlayerName(), targetname);
            return;
        }
        else
        {
            char    sourcename[128];
            char    targetname[128];

            C_BuildThingName(sourcename, sizeof(sourcename), source,
                obituary->sourcefriendly, false, obituary->sourcename);
            C_BuildThingName(targetname, sizeof(targetname), target,
                obituary->targetfriendly, false, obituary->targetname);
            M_snprintf(buffer, buffersize, "%s was telefragged by %s.", targetname, sourcename);
            return;
        }
    }

    if (source != MT_NULL)
    {
        char    sourcename[128];

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
                            (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"));
                    else
                        M_snprintf(buffer, buffersize, "%s was %s by %s %s that %s exploded!",
                            playername,
                            (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"),
                            pronoun(personal));
                }
                else
                {
                    const mobjtype_t    killer = (mobjtype_t)obituary->barrelinflicter;
                    const char          *killername1 = (killer > MT_NULL && killer < NUMMOBJTYPES ?
                                            mobjinfo[killer].name1 : "");

                    M_snprintf(buffer, buffersize, "%s were %s by %s %s that %s %s exploded!",
                        (isdefaultplayername() ? "You" : playername),
                        (gibbed ? s_GIBBED : s_KILLED),
                        (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                        (inflictername ? inflictername : "barrel"),
                        (inflicter == killer || M_StringCompare(inflictername, killername1) ? "another" :
                            (killername1 && isvowel(killername1[0]) ? "an" : "a")),
                        (*killername1 && !M_StringStartsWith(killername1, "Deh_Actor_") ? killername1 : "monster"));
                }

                return;
            }
            else
            {
                char    targetname[128];
                char    *temp;

                C_BuildThingName(targetname, sizeof(targetname), target, obituary->targetfriendly,
                    (obituary->targetcorpse && source != target), obituary->targetname);

                temp = sentencecase(targetname);

                if (byplayer)
                {
                    if (isdefaultplayername())
                        M_snprintf(buffer, buffersize, "%s was %s by %s %s that you exploded.",
                            temp, (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"));
                    else
                        M_snprintf(buffer, buffersize, "%s was %s by %s %s that %s exploded.",
                            temp, (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"),
                            playername);
                }
                else if (source == target)
                    M_snprintf(buffer, buffersize, "%s was %s by %s %s that they exploded.",
                        temp, (gibbed ? s_GIBBED : s_KILLED),
                        (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                        (inflictername ? inflictername : "barrel"));
                else
                {
                    const mobjtype_t    killer = (mobjtype_t)obituary->barrelinflicter;
                    const char          *killername1 = (killer > MT_NULL && killer < NUMMOBJTYPES ?
                                            mobjinfo[killer].name1 : "");

                    M_snprintf(buffer, buffersize, "%s was %s by %s %s that %s %s exploded.",
                        temp, (gibbed ? s_GIBBED : s_KILLED),
                        (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                        (inflictername ? inflictername : "barrel"),
                        (inflicter == killer || M_StringCompare(inflictername, killername1) ? "another" :
                            (*killername1 && isvowel(killername1[0]) ? "an" : "a")),
                        (*killername1 && !M_StringStartsWith(killername1, "Deh_Actor_") ? killername1 : "monster"));
                }

                free(temp);
                return;
            }
        }

        if (obituary->sourceisplayer || source == MT_BFG)
        {
            if (isdefaultplayername())
            {
                if (obituary->targetisplayer)
                {
                    if (healthcvar)
                        M_snprintf(buffer, buffersize, "You %s yourself!", s_KILLED);
                    else
                        M_snprintf(buffer, buffersize, "You %s yourself with your own %s!",
                            (gibbed ? s_GIBBED : s_KILLED), weaponinfo[weapon].name);
                }
                else
                {
                    char    targetname[128];

                    C_BuildThingName(targetname, sizeof(targetname), target,
                        obituary->targetfriendly, false, obituary->targetname);

                    if (weapon == wp_fist && viewplayer->powers[pw_strength])
                        M_snprintf(buffer, buffersize, "You %s %s with your %s while %s.",
                            (target == MT_BARREL ? "exploded" :
                                (target == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                            targetname, weaponinfo[weapon].name, berserk);
                    else
                        M_snprintf(buffer, buffersize, "You %s %s with your %s.",
                            (target == MT_BARREL ? "exploded" :
                                (target == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                            targetname, weaponinfo[weapon].name);
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
                            playername, (gibbed ? s_GIBBED : s_KILLED), pronoun(reflexive),
                            pronoun(possessive), weaponinfo[weapon].name);
                }
                else
                {
                    char    targetname[128];

                    C_BuildThingName(targetname, sizeof(targetname), target,
                        obituary->targetfriendly, obituary->targetcorpse, obituary->targetname);

                    if (weapon == wp_fist && viewplayer->powers[pw_strength])
                        M_snprintf(buffer, buffersize, "%s %s %s with %s %s while %s.",
                            playername,
                            (target == MT_BARREL ? "exploded" :
                                (target == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                            targetname, pronoun(possessive), weaponinfo[weapon].name, berserk);
                    else
                        M_snprintf(buffer, buffersize, "%s %s %s with %s %s.",
                            playername,
                            (target == MT_BARREL ? "exploded" :
                                (target == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                            targetname, pronoun(possessive), weaponinfo[weapon].name);
                }
            }

            return;
        }

        C_BuildThingName(sourcename, sizeof(sourcename), source,
            obituary->sourcefriendly, false, obituary->sourcename);

        if (obituary->targetisplayer)
        {
            if (isdefaultplayername())
                M_snprintf(buffer, buffersize, "You were %s by %s!",
                    (gibbed ? s_GIBBED : s_KILLED), sourcename);
            else
                M_snprintf(buffer, buffersize, "%s was %s by %s!",
                    playername, (gibbed ? s_GIBBED : s_KILLED), sourcename);
        }
        else
        {
            char    targetname[128];
            char *temp = sentencecase(sourcename);

            C_BuildThingName(targetname, sizeof(targetname), target,
                obituary->targetfriendly, false, obituary->targetname);

            M_snprintf(buffer, buffersize, "%s %s %s.",
                temp,
                (target == MT_BARREL ? "exploded" :
                    (target == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                targetname);

            free(temp);
        }

        return;
    }

    if (obituary->targetisplayer)
    {
        if (obituary->crushed)
        {
            if (isdefaultplayername())
                M_snprintf(buffer, buffersize, "You were crushed to death!");
            else
                M_snprintf(buffer, buffersize, "%s was crushed to death!", playername);

            return;
        }

        if (obituary->terraintype >= LIQUID)
        {
            const char *liquids[][2] =
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

            M_snprintf(buffer, buffersize, "%s died in %s.",
                C_GetPlayerName(), liquids[obituary->terraintype - LIQUID][english]);
            return;
        }
        else
        {
            const short floorpic = (short)obituary->floorpic;

            if ((floorpic >= RROCK05 && floorpic <= RROCK08) || (floorpic >= SLIME09 && floorpic <= SLIME12))
                M_snprintf(buffer, buffersize, "%s died on molten rock.", C_GetPlayerName());
            else
                M_snprintf(buffer, buffersize, "%s died.", C_GetPlayerName());

            return;
        }
    }
}

void C_ResolveLazyConsoleString(const int index)
{
    if (console[index].string[0])
        return;

    C_BuildObituaryString(console[index].string, sizeof(console[index].string), &console[index].obituary);
    console[index].string[0] = (char)toupper(console[index].string[0]);
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
    const bool gibbed, const bool telefragged)
{
    const int       i = (numconsolestrings > 0 ? numconsolestrings - 1 : 0);
    obituaryinfo_t  obituary = { 0 };

    obituary.target = (target ? target->type : MT_NULL);
    obituary.inflicter = (inflicter ? inflicter->type : MT_NULL);
    obituary.source = (source ? source->type : MT_NULL);

    obituary.weapon = (source && (source->player || source->type == MT_BFG) ?
        viewplayer->readyweapon : wp_nochange);

    obituary.gibbed = gibbed;
    obituary.telefragged = telefragged;

    obituary.targetisplayer = (target && target->player);
    obituary.sourceisplayer = (source && source->player);

    obituary.targetfriendly = (target && (target->flags & MF_FRIEND));
    obituary.sourcefriendly = (source && (source->flags & MF_FRIEND));
    obituary.targetcorpse = (target && (target->flags & MF_CORPSE));

    obituary.barrelinflicter = ((inflicter && inflicter->type == MT_BARREL) ?
        inflicter->inflicter : MT_NULL);

    obituary.targetname[0] = '\0';
    obituary.sourcename[0] = '\0';

    if (target && *target->name)
        M_StringCopy(obituary.targetname, target->name, sizeof(obituary.targetname));

    if (source && *source->name)
        M_StringCopy(obituary.sourcename, source->name, sizeof(obituary.sourcename));

    obituary.crushed = false;
    obituary.terraintype = -1;
    obituary.floorpic = -1;

    if (!source && target && target->player && target->player->mo == target)
    {
        const sector_t  *sector = viewplayer->mo->subsector->sector;

        if (sector->ceilingdata && sector->ceilingheight - sector->floorheight < VIEWHEIGHT)
            obituary.crushed = true;
        else
        {
            obituary.terraintype = sector->terraintype;
            obituary.floorpic = sector->floorpic;
        }
    }

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
            C_ResolveLazyConsoleString(i);
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
        C_ResolveLazyConsoleString(numconsolestrings);
        HU_SetPlayerMessage(console[numconsolestrings].string, false, false);
        message_warning = true;
    }

    numconsolestrings++;
    outputhistory = -1;
}
