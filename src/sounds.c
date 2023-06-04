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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "i_system.h"
#include "sounds.h"

//
// Information about all the music
//
musicinfo_t s_music[] =
{
    { "",        "",        "",                                "",                                0, 0, 0 },
    { "e1m1",    "e1m1",    "At Doom's Gate",                  "At Doom's Gate",                  0, 0, 0 },
    { "e1m2",    "e1m2",    "The Imp's Song",                  "The Imp's Song",                  0, 0, 0 },
    { "e1m3",    "e1m3",    "Dark Halls",                      "Dark Halls",                      0, 0, 0 },
    { "e1m4",    "e1m4",    "Kitchen Ace (And Taking Names)",  "Kitchen Ace (And Taking Names)",  0, 0, 0 },
    { "e1m5",    "e1m5",    "Suspense",                        "Suspense",                        0, 0, 0 },
    { "e1m6",    "e1m6",    "On The Hunt",                     "On The Hunt",                     0, 0, 0 },
    { "e1m7",    "e1m7",    "Demons On The Prey",              "Demons On The Prey",              0, 0, 0 },
    { "e1m8",    "e1m8",    "Sign Of Evil",                    "Sign Of Evil",                    0, 0, 0 },
    { "e1m9",    "e1m9",    "Hiding The Secrets",              "Hiding The Secrets",              0, 0, 0 },
    { "e2m1",    "e2m1",    "I Sawed The Demons",              "I Sawed The Demons",              0, 0, 0 },
    { "e2m2",    "e2m2",    "The Demons From Adrian's Pen",    "The Demons From Adrian's Pen",    0, 0, 0 },
    { "e2m3",    "e2m3",    "Intermission From DOOM",          "Intermission From DOOM",          0, 0, 0 },
    { "e2m4",    "e2m4",    "They're Going To Get You",        "They're Going To Get You",        0, 0, 0 },
    { "e2m5",    "e2m5",    "Demons On The Prey",              "Demons On The Prey",              0, 0, 0 },
    { "e2m6",    "e2m6",    "Sinister",                        "Sinister",                        0, 0, 0 },
    { "e2m7",    "e2m7",    "Waltz Of The Demons",             "Waltz Of The Demons",             0, 0, 0 },
    { "e2m8",    "e2m8",    "Nobody Told Me About id",         "Nobody Told Me About id",         0, 0, 0 },
    { "e2m9",    "e2m9",    "n/a",                             "n/a",                             0, 0, 0 },
    { "e3m1",    "e3m1",    "n/a",                             "n/a",                             0, 0, 0 },
    { "e3m2",    "e3m2",    "Donna To The Rescue",             "Donna To The Rescue",             0, 0, 0 },
    { "e3m3",    "e3m3",    "Deep Into The Code",              "Deep Into The Code",              0, 0, 0 },
    { "e3m4",    "e3m4",    "Sign Of Evil",                    "Sign Of Evil",                    0, 0, 0 },
    { "e3m5",    "e3m5",    "Demons On The Prey",              "Demons On The Prey",              0, 0, 0 },
    { "e3m6",    "e3m6",    "Suspense",                        "Suspense",                        0, 0, 0 },
    { "e3m7",    "e3m7",    "Waltz Of The Demons",             "Waltz Of The Demons",             0, 0, 0 },
    { "e3m8",    "e3m8",    "Facing The Spider",               "Facing The Spider",               0, 0, 0 },
    { "e3m9",    "e3m9",    "Hiding The Secrets",              "Hiding The Secrets",              0, 0, 0 },
    { "e5m1",    "e5m1",    "Hate Machine",                    "Romero One Mind Any Weapon",      0, 0, 0 },
    { "e5m2",    "e5m2",    "You Ain't The Boss Of Me",        "13th Floor",                      0, 0, 0 },
    { "e5m3",    "e5m3",    "Quell The Beast",                 "Buildor Part 2",                  0, 0, 0 },
    { "e5m4",    "e5m4",    "Riot Squadron",                   "The Patrolman",                   0, 0, 0 },
    { "e5m5",    "e5m5",    "Alice",                           "Cold Frost Part 6",               0, 0, 0 },
    { "e5m6",    "e5m6",    "Besieged City",                   "Melting Man Part 2",              0, 0, 0 },
    { "e5m7",    "e5m7",    "Watching You",                    "Far 5",                           0, 0, 0 },
    { "e5m8",    "e5m8",    "Easel",                           "Poseidon 4-6",                    0, 0, 0 },
    { "e5m9",    "e5m9",    "Adrenaline In The Blood",         "Fastpass",                        0, 0, 0 },
    { "inter",   "inter",   "Intermission From DOOM",          "Triceratoptron",                  0, 0, 0 },
    { "intro",   "intro",   "n/a",                             "Eye On Spiral Part 1",            0, 0, 0 },
    { "bunny",   "bunny",   "Sweet Little Dead Bunny",         "Sweet Little Dead Bunny",         0, 0, 0 },
    { "victor",  "victor",  "The End Of DOOM",                 "The End Of DOOM",                 0, 0, 0 },
    { "introa",  "introa",  "n/a",                             "n/a",                             0, 0, 0 },
    { "runnin",  "runnin",  "Running From Evil",               "Running From Evil",               0, 0, 0 },
    { "stalks",  "stalks",  "The Healer Stalks",               "The Healer Stalks",               0, 0, 0 },
    { "countd",  "countd",  "Countdown To Death",              "Countdown To Death",              0, 0, 0 },
    { "betwee",  "betwee",  "Between Levels",                  "Between Levels",                  0, 0, 0 },
    { "doom",    "doom",    "DOOM",                            "DOOM",                            0, 0, 0 },
    { "the_da",  "the_da",  "In The Dark",                     "In The Dark",                     0, 0, 0 },
    { "shawn",   "shawn",   "Shawn's Got The Shotgun",         "Shawn's Got The Shotgun",         0, 0, 0 },
    { "ddtblu",  "ddtblu",  "The Dave D. Taylor Blues",        "The Dave D. Taylor Blues",        0, 0, 0 },
    { "in_cit",  "in_cit",  "Into Sandy's City",               "Into Sandy's City",               0, 0, 0 },
    { "dead",    "dead",    "The Demon's Dead",                "The Demon's Dead",                0, 0, 0 },
    { "stlks2",  "stlks2",  "The Healer Stalks",               "The Healer Stalks",               0, 0, 0 },
    { "theda2",  "theda2",  "In The Dark",                     "In The Dark",                     0, 0, 0 },
    { "doom2",   "doom2",   "DOOM",                            "DOOM",                            0, 0, 0 },
    { "ddtbl2",  "ddtbl2",  "The Dave D. Taylor Blues",        "The Dave D. Taylor Blues",        0, 0, 0 },
    { "runni2",  "runni2",  "Running From Evil",               "Running From Evil",               0, 0, 0 },
    { "dead2",   "dead2",   "The Demon's Dead",                "The Demon's Dead",                0, 0, 0 },
    { "stlks3",  "stlks3",  "The Healer Stalks",               "The Healer Stalks",               0, 0, 0 },
    { "romero",  "romero",  "Waiting For Romero To Play",      "Waiting For Romero To Play",      0, 0, 0 },
    { "shawn2",  "shawn2",  "Shawn's Got The Shotgun",         "Shawn's Got The Shotgun",         0, 0, 0 },
    { "messag",  "messag",  "Message For The Arch-vile",       "Message For The Arch-vile",       0, 0, 0 },
    { "count2",  "count2",  "Countdown To Death",              "Countdown To Death",              0, 0, 0 },
    { "ddtbl3",  "ddtbl3",  "The Dave D. Taylor Blues",        "The Dave D. Taylor Blues",        0, 0, 0 },
    { "ampie",   "ampie",   "Bye Bye American Pie",            "Bye Bye American Pie",            0, 0, 0 },
    { "theda3",  "theda3",  "In The Dark",                     "In The Dark",                     0, 0, 0 },
    { "adrian",  "adrian",  "Adrian's Asleep",                 "Adrian's Asleep",                 0, 0, 0 },
    { "messg2",  "messg2",  "Message For The Arch-vile",       "Message For The Arch-vile",       0, 0, 0 },
    { "romer2",  "romer2",  "Waiting For Romero To Play",      "Waiting For Romero To Play",      0, 0, 0 },
    { "tense",   "tense",   "Getting Too Tense",               "Getting Too Tense",               0, 0, 0 },
    { "shawn3",  "shawn3",  "Shawn's Got The Shotgun",         "Shawn's Got The Shotgun",         0, 0, 0 },
    { "openin",  "openin",  "Opening To Hell",                 "Opening To Hell",                 0, 0, 0 },
    { "evil",    "evil",    "Evil Incarnate",                  "Evil Incarnate",                  0, 0, 0 },
    { "ultima",  "ultima",  "The Ultimate Challenge/Conquest", "The Ultimate Challenge/Conquest", 0, 0, 0 },
    { "read_m",  "read_m",  "n/a",                             "n/a",                             0, 0, 0 },
    { "dm2ttl",  "dm2ttl",  "n/a",                             "n/a",                             0, 0, 0 },
    { "dm2int",  "dm2int",  "n/a",                             "n/a",                             0, 0, 0 },
    { "",        "",        "",                                "",                                0, 0, 0 },

    // custom music from MUSINFO lump
    { "musinfo", "musinfo", "n/a",                             "n/a",                             0, 0, 0 }
};

//
// Information about all the SFX
//
sfxinfo_t original_s_sfx[NUMSFX] =
{
    // s_sfx[0] needs to be a dummy for odd reasons.
    { "none",   "none",   sg_none,     0, -1 },
    { "pistol", "pistol", sg_none,    64, -1 },
    { "shotgn", "shotgn", sg_none,    64, -1 },
    { "sgcock", "sgcock", sg_none,    64, -1 },
    { "dshtgn", "dshtgn", sg_none,    64, -1 },
    { "dbopn",  "dbopn",  sg_none,    64, -1 },
    { "dbcls",  "dbcls",  sg_none,    64, -1 },
    { "dbload", "dbload", sg_none,    64, -1 },
    { "plasma", "plasma", sg_none,    64, -1 },
    { "bfg",    "bfg",    sg_none,    64, -1 },
    { "sawup",  "sawup",  sg_saw,     64, -1 },
    { "sawidl", "sawidl", sg_saw,    118, -1 },
    { "sawful", "sawful", sg_saw,     64, -1 },
    { "sawhit", "sawhit", sg_saw,     64, -1 },
    { "rlaunc", "rlaunc", sg_none,    64, -1 },
    { "rxplod", "rxplod", sg_none,    70, -1 },
    { "firsht", "firsht", sg_none,    70, -1 },
    { "firxpl", "firxpl", sg_none,    70, -1 },
    { "pstart", "pstart", sg_none,   100, -1 },
    { "pstop",  "pstop",  sg_none,   100, -1 },
    { "doropn", "doropn", sg_none,   100, -1 },
    { "dorcls", "dorcls", sg_none,   100, -1 },
    { "stnmov", "stnmov", sg_stnmov, 119, -1 },
    { "swtchn", "swtchn", sg_none,    78, -1 },
    { "swtchx", "swtchx", sg_none,    78, -1 },
    { "plpain", "plpain", sg_none,    96, -1 },
    { "dmpain", "dmpain", sg_none,    96, -1 },
    { "popain", "popain", sg_none,    96, -1 },
    { "vipain", "vipain", sg_none,    96, -1 },
    { "mnpain", "mnpain", sg_none,    96, -1 },
    { "pepain", "pepain", sg_none,    96, -1 },
    { "slop",   "slop",   sg_none,    78, -1 },
    { "itemup", "itemup", sg_itemup,  78, -1 },
    { "wpnup",  "wpnup",  sg_wpnup,   78, -1 },
    { "oof",    "oof",    sg_oof,     96, -1 },
    { "telept", "telept", sg_none,    32, -1 },
    { "posit1", "posit1", sg_none,    98, -1 },
    { "posit2", "posit2", sg_none,    98, -1 },
    { "posit3", "posit3", sg_none,    98, -1 },
    { "bgsit1", "bgsit1", sg_none,    98, -1 },
    { "bgsit2", "bgsit2", sg_none,    98, -1 },
    { "sgtsit", "sgtsit", sg_none,    98, -1 },
    { "cacsit", "cacsit", sg_none,    98, -1 },
    { "brssit", "brssit", sg_none,    94, -1 },
    { "cybsit", "cybsit", sg_none,    92, -1 },
    { "spisit", "spisit", sg_none,    90, -1 },
    { "bspsit", "bspsit", sg_none,    90, -1 },
    { "kntsit", "kntsit", sg_none,    90, -1 },
    { "vilsit", "vilsit", sg_none,    90, -1 },
    { "mansit", "mansit", sg_none,    90, -1 },
    { "pesit",  "pesit",  sg_none,    90, -1 },
    { "sklatk", "sklatk", sg_none,    70, -1 },
    { "sgtatk", "sgtatk", sg_none,    70, -1 },
    { "skepch", "skepch", sg_none,    70, -1 },
    { "vilatk", "vilatk", sg_none,    70, -1 },
    { "claw",   "claw",   sg_none,    70, -1 },
    { "skeswg", "skeswg", sg_none,    70, -1 },
    { "pldeth", "pldeth", sg_none,    32, -1 },
    { "pdiehi", "pdiehi", sg_none,    32, -1 },
    { "podth1", "podth1", sg_none,    70, -1 },
    { "podth2", "podth2", sg_none,    70, -1 },
    { "podth3", "podth3", sg_none,    70, -1 },
    { "bgdth1", "bgdth1", sg_none,    70, -1 },
    { "bgdth2", "bgdth2", sg_none,    70, -1 },
    { "sgtdth", "sgtdth", sg_none,    70, -1 },
    { "cacdth", "cacdth", sg_none,    70, -1 },
    { "skldth", "skldth", sg_none,    70, -1 },
    { "brsdth", "brsdth", sg_none,    32, -1 },
    { "cybdth", "cybdth", sg_none,    32, -1 },
    { "spidth", "spidth", sg_none,    32, -1 },
    { "bspdth", "bspdth", sg_none,    32, -1 },
    { "vildth", "vildth", sg_none,    32, -1 },
    { "kntdth", "kntdth", sg_none,    32, -1 },
    { "pedth",  "pedth",  sg_none,    32, -1 },
    { "skedth", "skedth", sg_none,    32, -1 },
    { "posact", "posact", sg_none,   120, -1 },
    { "bgact",  "bgact",  sg_none,   120, -1 },
    { "dmact",  "dmact",  sg_none,   120, -1 },
    { "bspact", "bspact", sg_none,   100, -1 },
    { "bspwlk", "bspwlk", sg_none,   100, -1 },
    { "vilact", "vilact", sg_none,   100, -1 },
    { "noway",  "noway",  sg_oof,     78, -1 },
    { "barexp", "barexp", sg_none,    60, -1 },
    { "punch",  "punch",  sg_none,    64, -1 },
    { "hoof",   "hoof",   sg_none,    70, -1 },
    { "metal",  "metal",  sg_none,    70, -1 },
    { "chgun",  "chgun",  sg_none,    64, -1 },
    { "tink",   "tink",   sg_none,    60, -1 },
    { "bdopn",  "bdopn",  sg_none,   100, -1 },
    { "bdcls",  "bdcls",  sg_none,   100, -1 },
    { "itmbk",  "itmbk",  sg_none,   100, -1 },
    { "flame",  "flame",  sg_none,    32, -1 },
    { "flamst", "flamst", sg_none,    32, -1 },
    { "getpow", "getpow", sg_getpow,  60, -1 },
    { "bospit", "bospit", sg_none,    70, -1 },
    { "boscub", "boscub", sg_none,    70, -1 },
    { "bossit", "bossit", sg_none,    70, -1 },
    { "bospn",  "bospn",  sg_none,    70, -1 },
    { "bosdth", "bosdth", sg_none,    70, -1 },
    { "manatk", "manatk", sg_none,    70, -1 },
    { "mandth", "mandth", sg_none,    70, -1 },
    { "sssit",  "sssit",  sg_none,    70, -1 },
    { "ssdth",  "ssdth",  sg_none,    70, -1 },
    { "keenpn", "keenpn", sg_none,    70, -1 },
    { "keendt", "keendt", sg_none,    70, -1 },
    { "skeact", "skeact", sg_none,    70, -1 },
    { "skesit", "skesit", sg_none,    70, -1 },
    { "skeatk", "skeatk", sg_none,    70, -1 },
    { "radio",  "radio",  sg_none,    60, -1 },

    // killough 11/98: dog sounds
    { "dgsit",  "dgsit",  sg_none,    98, -1 },
    { "dgatk",  "dgatk",  sg_none,    70, -1 },
    { "dgact",  "dgact",  sg_none,   120, -1 },
    { "dgdth",  "dgdth",  sg_none,    70, -1 },
    { "dgpain", "dgpain", sg_none,    96, -1 },

    // e6y
    { "secret", "secret", sg_none,    60, -1 },
    { "gibdth", "gibdth", sg_none,    60, -1 },

    { "scrsht", "scrsht", sg_none,     0, -1 },
    { "consol", "consol", sg_none,     0, -1 }
};

// DSDHacked
sfxinfo_t   *s_sfx;
int         numsfx;
static char **deh_soundnames;
static int  deh_soundnames_size;
static byte *sfx_state;

void InitSFX(void)
{
    s_sfx = original_s_sfx;
    numsfx = NUMSFX;
    deh_soundnames_size = numsfx + 1;
    deh_soundnames = malloc(deh_soundnames_size * sizeof(*deh_soundnames));

    for (int i = 1; i < numsfx; i++)
        deh_soundnames[i] = (s_sfx[i].name1 ? strdup(s_sfx[i].name1) : NULL);

    deh_soundnames[0] = NULL;
    deh_soundnames[numsfx] = NULL;
    sfx_state = calloc(numsfx, sizeof(*sfx_state));
}

void FreeSFX(void)
{
    for (int i = 1; i < deh_soundnames_size; i++)
        if (deh_soundnames[i])
            free(deh_soundnames[i]);

    free(deh_soundnames);
    free(sfx_state);
}

void dsdh_EnsureSFXCapacity(const int limit)
{
    static int  first_allocation = true;

    while (limit >= numsfx)
    {
        const int   old_numsfx = numsfx;

        numsfx *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            s_sfx = malloc(numsfx * sizeof(*s_sfx));
            memcpy(s_sfx, original_s_sfx, old_numsfx * sizeof(*s_sfx));
        }
        else
            s_sfx = I_Realloc(s_sfx, numsfx * sizeof(*s_sfx));

        memset(s_sfx + old_numsfx, 0, (numsfx - old_numsfx) * sizeof(*s_sfx));

        sfx_state = I_Realloc(sfx_state, numsfx * sizeof(*sfx_state));
        memset(sfx_state + old_numsfx, 0,
            (numsfx - old_numsfx) * sizeof(*sfx_state));

        for (int i = old_numsfx; i < numsfx; i++)
        {
            s_sfx[i].priority = 127;
            s_sfx[i].lumpnum = -1;
        }
    }
}

int dsdh_GetDehSFXIndex(const char *key, size_t length)
{
    for (int i = 1; i < numsfx; i++)
        if (s_sfx[i].name1
            && strlen(s_sfx[i].name1) == length
            && !strncasecmp(s_sfx[i].name1, key, length)
            && !sfx_state[i])
        {
            sfx_state[i] = true;    // sfx has been edited
            return i;
        }

    return -1;
}

int dsdh_GetOriginalSFXIndex(const char *key)
{
    int limit;

    for (int i = 1; deh_soundnames[i]; i++)
        if (!strncasecmp(deh_soundnames[i], key, 6))
            return i;

    // is it a number?
    for (const char *c = key; *c; c++)
        if (!isdigit(*c))
            return -1;

    limit = atoi(key);
    dsdh_EnsureSFXCapacity(limit);

    return limit;
}
