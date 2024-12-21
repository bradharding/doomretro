/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "i_system.h"
#include "m_array.h"
#include "m_misc.h"
#include "sounds.h"

//
// Information about all the music
//
musicinfo_t s_music[] =
{
    { "",        "",        "",         "",                                "",                                ""            },
    { "e1m1",    "e1m1",    "H_E1M1",   "At DOOM's Gate",                  "At DOOM's Gate",                  ""            },
    { "e1m2",    "e1m2",    "H_E1M2",   "The Imp's Song",                  "The Imp's Song",                  ""            },
    { "e1m3",    "e1m3",    "H_E1M3",   "Dark Halls",                      "Dark Halls",                      ""            },
    { "e1m4",    "e1m4",    "H_E1M4",   "Kitchen Ace (And Taking Names)",  "Kitchen Ace (And Taking Names)",  ""            },
    { "e1m5",    "e1m5",    "H_E1M5",   "Suspense",                        "Suspense",                        ""            },
    { "e1m6",    "e1m6",    "H_E1M6",   "On The Hunt",                     "On The Hunt",                     ""            },
    { "e1m7",    "e1m7",    "H_E1M7",   "Demons On The Prey",              "Demons On The Prey",              ""            },
    { "e1m8",    "e1m8",    "H_E1M8",   "Sign Of Evil",                    "Sign Of Evil",                    ""            },
    { "e1m9",    "e1m9",    "H_E1M9",   "Hiding The Secrets",              "Hiding The Secrets",              ""            },
    { "e2m1",    "e2m1",    "H_E2M1",   "I Sawed The Demons",              "I Sawed The Demons",              ""            },
    { "e2m2",    "e2m2",    "H_E2M2",   "The Demons From Adrian's Pen",    "The Demons From Adrian's Pen",    ""            },
    { "e2m3",    "e2m3",    "H_E2M3",   "Intermission From DOOM",          "Intermission From DOOM",          ""            },
    { "e2m4",    "e2m4",    "H_E2M4",   "They're Going To Get You",        "They're Going To Get You",        ""            },
    { "e2m5",    "e2m5",    "H_E1M7",   "Demons On The Prey",              "Demons On The Prey",              ""            },
    { "e2m6",    "e2m6",    "H_E2M6",   "Sinister",                        "Sinister",                        ""            },
    { "e2m7",    "e2m7",    "H_E2M7",   "Waltz Of The Demons",             "Waltz Of The Demons",             ""            },
    { "e2m8",    "e2m8",    "H_E2M8",   "Nobody Told Me About id",         "Nobody Told Me About id",         ""            },
    { "e2m9",    "e2m9",    "H_E2M9",   "n/a",                             "n/a",                             ""            },
    { "e3m1",    "e3m1",    "H_E2M9",   "n/a",                             "n/a",                             ""            },
    { "e3m2",    "e3m2",    "H_E3M2",   "Donna To The Rescue",             "Donna To The Rescue",             ""            },
    { "e3m3",    "e3m3",    "H_E3M3",   "Deep Into The Code",              "Deep Into The Code",              ""            },
    { "e3m4",    "e3m4",    "H_E1M8",   "Sign Of Evil",                    "Sign Of Evil",                    ""            },
    { "e3m5",    "e3m5",    "H_E1M7",   "Demons On The Prey",              "Demons On The Prey",              ""            },
    { "e3m6",    "e3m6",    "H_E1M5",   "Suspense",                        "Suspense",                        ""            },
    { "e3m7",    "e3m7",    "H_E2M7",   "Waltz Of The Demons",             "Waltz Of The Demons",             ""            },
    { "e3m8",    "e3m8",    "H_E3M8",   "Facing The Spider",               "Facing The Spider",               ""            },
    { "e3m9",    "e3m9",    "H_E1M9",   "Hiding The Secrets",              "Hiding The Secrets",              ""            },
    { "e4m1",    "e4m1",    "H_E1M8",   "n/a",                             "n/a",                             ""            },
    { "e4m2",    "e4m2",    "H_E3M2",   "n/a",                             "n/a",                             ""            },
    { "e4m3",    "e4m3",    "H_E3M3",   "n/a",                             "n/a",                             ""            },
    { "e4m4",    "e4m4",    "H_E1M5",   "n/a",                             "n/a",                             ""            },
    { "e4m5",    "e4m5",    "H_E2M7",   "n/a",                             "n/a",                             ""            },
    { "e4m6",    "e4m6",    "H_E2M4",   "n/a",                             "n/a",                             ""            },
    { "e4m7",    "e4m7",    "H_E2M6",   "n/a",                             "n/a",                             ""            },
    { "e4m8",    "e4m8",    "H_E2M5",   "n/a",                             "n/a",                             ""            },
    { "e4m9",    "e4m9",    "H_E1M9",   "n/a",                             "n/a",                             ""            },
    { "e5m1",    "e5m1",    "",         "Hate Machine",                    "Romero One Mind Any Weapon",      ""            },
    { "e5m2",    "e5m2",    "",         "You Ain't The Boss Of Me",        "13th Floor",                      ""            },
    { "e5m3",    "e5m3",    "",         "Quell The Beast",                 "Buildor Part 2",                  ""            },
    { "e5m4",    "e5m4",    "",         "Riot Squadron",                   "The Patrolman",                   ""            },
    { "e5m5",    "e5m5",    "",         "Alice",                           "Cold Frost Part 6",               ""            },
    { "e5m6",    "e5m6",    "",         "Besieged City",                   "Melting Man Part 2",              ""            },
    { "e5m7",    "e5m7",    "",         "Watching You",                    "Far 5",                           ""            },
    { "e5m8",    "e5m8",    "",         "Easel",                           "Poseidon 4-6",                    ""            },
    { "e5m9",    "e5m9",    "",         "Adrenaline In The Blood",         "Fastpass",                        ""            },
    { "e6m1",    "e6m1",    "",         "Nightmare Overture",              "Umbral Uprising",                 ""            },
    { "e6m2",    "e6m2",    "",         "Sleep Of Reason",                 "Hellspawn Havoc",                 ""            },
    { "e6m3",    "e6m3",    "",         "Cathedral Rock",                  "Evil Dead",                       ""            },
    { "e6m4",    "e6m4",    "",         "Fractures",                       "Pain",                            ""            },
    { "e6m5",    "e6m5",    "",         "Hexaphobia",                      "Chainsaw Chorus",                 ""            },
    { "e6m6",    "e6m6",    "",         "Walls Of The Minotaur",           "Incineration Anthem",             ""            },
    { "e6m7",    "e6m7",    "",         "The Impenetrable Dark",           "Wicked Warpath",                  ""            },
    { "e6m8",    "e6m8",    "",         "Final Impact",                    "Vitriolic Vendetta",              ""            },
    { "e6m9",    "e6m9",    "",         "I'm The Doomguy With The Gun",    "Raging Revenant",                 ""            },
    { "inter",   "inter",   "H_E2M3",   "Intermission From DOOM",          "Triceratoptron",                  "Extirpation" },
    { "intro",   "intro",   "H_INTRO",  "n/a",                             "Eye On Spiral Part 1",            "Immolator"   },
    { "bunny",   "bunny",   "H_BUNNY",  "Sweet Little Dead Bunny",         "Sweet Little Dead Bunny",         ""            },
    { "victor",  "victor",  "H_VICTOR", "The End Of DOOM",                 "The End Of DOOM",                 ""            },
    { "introa",  "introa",  "H_INTRO",  "n/a",                             "n/a",                             ""            },
    { "runnin",  "runnin",  "H_RUNNIN", "Running From Evil",               "Running From Evil",               ""            },
    { "stalks",  "stalks",  "H_STALKS", "The Healer Stalks",               "The Healer Stalks",               ""            },
    { "countd",  "countd",  "H_COUNTD", "Countdown To Death",              "Countdown To Death",              ""            },
    { "betwee",  "betwee",  "H_BETWEE", "Between Levels",                  "Between Levels",                  ""            },
    { "doom",    "doom",    "H_DOOM",   "DOOM",                            "DOOM",                            ""            },
    { "the_da",  "the_da",  "H_THE_DA", "In The Dark",                     "In The Dark",                     ""            },
    { "shawn",   "shawn",   "H_SHAWN",  "Shawn's Got The Shotgun",         "Shawn's Got The Shotgun",         ""            },
    { "ddtblu",  "ddtblu",  "H_DDTBLU", "The Dave D. Taylor Blues",        "The Dave D. Taylor Blues",        ""            },
    { "in_cit",  "in_cit",  "H_IN_CIT", "Into Sandy's City",               "Into Sandy's City",               ""            },
    { "dead",    "dead",    "H_DEAD",   "The Demon's Dead",                "The Demon's Dead",                ""            },
    { "stlks2",  "stlks2",  "H_STALKS", "The Healer Stalks",               "The Healer Stalks",               ""            },
    { "theda2",  "theda2",  "H_THE_DA", "In The Dark",                     "In The Dark",                     ""            },
    { "doom2",   "doom2",   "H_DOOM",   "DOOM",                            "DOOM",                            ""            },
    { "ddtbl2",  "ddtbl2",  "H_DDTBLU", "The Dave D. Taylor Blues",        "The Dave D. Taylor Blues",        ""            },
    { "runni2",  "runni2",  "H_RUNNIN", "Running From Evil",               "Running From Evil",               ""            },
    { "dead2",   "dead2",   "H_DEAD",   "The Demon's Dead",                "The Demon's Dead",                ""            },
    { "stlks3",  "stlks3",  "H_STALKS", "The Healer Stalks",               "The Healer Stalks",               ""            },
    { "romero",  "romero",  "H_ROMERO", "Waiting For Romero To Play",      "Waiting For Romero To Play",      ""            },
    { "shawn2",  "shawn2",  "H_SHAWN",  "Shawn's Got The Shotgun",         "Shawn's Got The Shotgun",         ""            },
    { "messag",  "messag",  "H_MESSAG", "Message For The Arch-vile",       "Message For The Arch-vile",       ""            },
    { "count2",  "count2",  "H_COUNTD", "Countdown To Death",              "Countdown To Death",              ""            },
    { "ddtbl3",  "ddtbl3",  "H_DDTBLU", "The Dave D. Taylor Blues",        "The Dave D. Taylor Blues",        ""            },
    { "ampie",   "ampie",   "H_AMPIE",  "Bye Bye American Pie",            "Bye Bye American Pie",            ""            },
    { "theda3",  "theda3",  "H_THE_DA", "In The Dark",                     "In The Dark",                     ""            },
    { "adrian",  "adrian",  "H_ADRIAN", "Adrian's Asleep",                 "Adrian's Asleep",                 ""            },
    { "messg2",  "messg2",  "H_MESSAG", "Message For The Arch-vile",       "Message For The Arch-vile",       ""            },
    { "romer2",  "romer2",  "H_ROMERO", "Waiting For Romero To Play",      "Waiting For Romero To Play",      ""            },
    { "tense",   "tense",   "H_TENSE",  "Getting Too Tense",               "Getting Too Tense",               ""            },
    { "shawn3",  "shawn3",  "H_SHAWN",  "Shawn's Got The Shotgun",         "Shawn's Got The Shotgun",         ""            },
    { "openin",  "openin",  "H_OPENIN", "Opening To Hell",                 "Opening To Hell",                 ""            },
    { "evil",    "evil",    "H_EVIL",   "Evil Incarnate",                  "Evil Incarnate",                  ""            },
    { "ultima",  "ultima",  "H_ULTIMA", "The Ultimate Challenge/Conquest", "The Ultimate Challenge/Conquest", ""            },
    { "read_m",  "read_m",  "H_READ_M", "n/a",                             "n/a",                             ""            },
    { "dm2ttl",  "dm2ttl",  "H_DM2TTL", "n/a",                             "n/a",                             ""            },
    { "dm2int",  "dm2int",  "H_DM2INT", "n/a",                             "n/a",                             ""            },
    { "",        "",        "",         "",                                "",                                ""            },

    // custom music from MUSINFO lump
    { "musinfo", "musinfo", "",         "n/a",                             "n/a",                             ""            }
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
    { "consol", "consol", sg_none,     0, -1 },

    [500] = { "fre000", "fre000", sg_none, 127, -1 },
    [501] = { "fre001", "fre001", sg_none, 127, -1 },
    [502] = { "fre002", "fre002", sg_none, 127, -1 },
    [503] = { "fre003", "fre003", sg_none, 127, -1 },
    [504] = { "fre004", "fre004", sg_none, 127, -1 },
    [505] = { "fre005", "fre005", sg_none, 127, -1 },
    [506] = { "fre006", "fre006", sg_none, 127, -1 },
    [507] = { "fre007", "fre007", sg_none, 127, -1 },
    [508] = { "fre008", "fre008", sg_none, 127, -1 },
    [509] = { "fre009", "fre009", sg_none, 127, -1 },
    [510] = { "fre010", "fre010", sg_none, 127, -1 },
    [511] = { "fre011", "fre011", sg_none, 127, -1 },
    [512] = { "fre012", "fre012", sg_none, 127, -1 },
    [513] = { "fre013", "fre013", sg_none, 127, -1 },
    [514] = { "fre014", "fre014", sg_none, 127, -1 },
    [515] = { "fre015", "fre015", sg_none, 127, -1 },
    [516] = { "fre016", "fre016", sg_none, 127, -1 },
    [517] = { "fre017", "fre017", sg_none, 127, -1 },
    [518] = { "fre018", "fre018", sg_none, 127, -1 },
    [519] = { "fre019", "fre019", sg_none, 127, -1 },
    [520] = { "fre020", "fre020", sg_none, 127, -1 },
    [521] = { "fre021", "fre021", sg_none, 127, -1 },
    [522] = { "fre022", "fre022", sg_none, 127, -1 },
    [523] = { "fre023", "fre023", sg_none, 127, -1 },
    [524] = { "fre024", "fre024", sg_none, 127, -1 },
    [525] = { "fre025", "fre025", sg_none, 127, -1 },
    [526] = { "fre026", "fre026", sg_none, 127, -1 },
    [527] = { "fre027", "fre027", sg_none, 127, -1 },
    [528] = { "fre028", "fre028", sg_none, 127, -1 },
    [529] = { "fre029", "fre029", sg_none, 127, -1 },
    [530] = { "fre030", "fre030", sg_none, 127, -1 },
    [531] = { "fre031", "fre031", sg_none, 127, -1 },
    [532] = { "fre032", "fre032", sg_none, 127, -1 },
    [533] = { "fre033", "fre033", sg_none, 127, -1 },
    [534] = { "fre034", "fre034", sg_none, 127, -1 },
    [535] = { "fre035", "fre035", sg_none, 127, -1 },
    [536] = { "fre036", "fre036", sg_none, 127, -1 },
    [537] = { "fre037", "fre037", sg_none, 127, -1 },
    [538] = { "fre038", "fre038", sg_none, 127, -1 },
    [539] = { "fre039", "fre039", sg_none, 127, -1 },
    [540] = { "fre040", "fre040", sg_none, 127, -1 },
    [541] = { "fre041", "fre041", sg_none, 127, -1 },
    [542] = { "fre042", "fre042", sg_none, 127, -1 },
    [543] = { "fre043", "fre043", sg_none, 127, -1 },
    [544] = { "fre044", "fre044", sg_none, 127, -1 },
    [545] = { "fre045", "fre045", sg_none, 127, -1 },
    [546] = { "fre046", "fre046", sg_none, 127, -1 },
    [547] = { "fre047", "fre047", sg_none, 127, -1 },
    [548] = { "fre048", "fre048", sg_none, 127, -1 },
    [549] = { "fre049", "fre049", sg_none, 127, -1 },
    [550] = { "fre050", "fre050", sg_none, 127, -1 },
    [551] = { "fre051", "fre051", sg_none, 127, -1 },
    [552] = { "fre052", "fre052", sg_none, 127, -1 },
    [553] = { "fre053", "fre053", sg_none, 127, -1 },
    [554] = { "fre054", "fre054", sg_none, 127, -1 },
    [555] = { "fre055", "fre055", sg_none, 127, -1 },
    [556] = { "fre056", "fre056", sg_none, 127, -1 },
    [557] = { "fre057", "fre057", sg_none, 127, -1 },
    [558] = { "fre058", "fre058", sg_none, 127, -1 },
    [559] = { "fre059", "fre059", sg_none, 127, -1 },
    [560] = { "fre060", "fre060", sg_none, 127, -1 },
    [561] = { "fre061", "fre061", sg_none, 127, -1 },
    [562] = { "fre062", "fre062", sg_none, 127, -1 },
    [563] = { "fre063", "fre063", sg_none, 127, -1 },
    [564] = { "fre064", "fre064", sg_none, 127, -1 },
    [565] = { "fre065", "fre065", sg_none, 127, -1 },
    [566] = { "fre066", "fre066", sg_none, 127, -1 },
    [567] = { "fre067", "fre067", sg_none, 127, -1 },
    [568] = { "fre068", "fre068", sg_none, 127, -1 },
    [569] = { "fre069", "fre069", sg_none, 127, -1 },
    [570] = { "fre070", "fre070", sg_none, 127, -1 },
    [571] = { "fre071", "fre071", sg_none, 127, -1 },
    [572] = { "fre072", "fre072", sg_none, 127, -1 },
    [573] = { "fre073", "fre073", sg_none, 127, -1 },
    [574] = { "fre074", "fre074", sg_none, 127, -1 },
    [575] = { "fre075", "fre075", sg_none, 127, -1 },
    [576] = { "fre076", "fre076", sg_none, 127, -1 },
    [577] = { "fre077", "fre077", sg_none, 127, -1 },
    [578] = { "fre078", "fre078", sg_none, 127, -1 },
    [579] = { "fre079", "fre079", sg_none, 127, -1 },
    [580] = { "fre080", "fre080", sg_none, 127, -1 },
    [581] = { "fre081", "fre081", sg_none, 127, -1 },
    [582] = { "fre082", "fre082", sg_none, 127, -1 },
    [583] = { "fre083", "fre083", sg_none, 127, -1 },
    [584] = { "fre084", "fre084", sg_none, 127, -1 },
    [585] = { "fre085", "fre085", sg_none, 127, -1 },
    [586] = { "fre086", "fre086", sg_none, 127, -1 },
    [587] = { "fre087", "fre087", sg_none, 127, -1 },
    [588] = { "fre088", "fre088", sg_none, 127, -1 },
    [589] = { "fre089", "fre089", sg_none, 127, -1 },
    [590] = { "fre090", "fre090", sg_none, 127, -1 },
    [591] = { "fre091", "fre091", sg_none, 127, -1 },
    [592] = { "fre092", "fre092", sg_none, 127, -1 },
    [593] = { "fre093", "fre093", sg_none, 127, -1 },
    [594] = { "fre094", "fre094", sg_none, 127, -1 },
    [595] = { "fre095", "fre095", sg_none, 127, -1 },
    [596] = { "fre096", "fre096", sg_none, 127, -1 },
    [597] = { "fre097", "fre097", sg_none, 127, -1 },
    [598] = { "fre098", "fre098", sg_none, 127, -1 },
    [599] = { "fre099", "fre099", sg_none, 127, -1 },
    [600] = { "fre100", "fre100", sg_none, 127, -1 },
    [601] = { "fre101", "fre101", sg_none, 127, -1 },
    [602] = { "fre102", "fre102", sg_none, 127, -1 },
    [603] = { "fre103", "fre103", sg_none, 127, -1 },
    [604] = { "fre104", "fre104", sg_none, 127, -1 },
    [605] = { "fre105", "fre105", sg_none, 127, -1 },
    [606] = { "fre106", "fre106", sg_none, 127, -1 },
    [607] = { "fre107", "fre107", sg_none, 127, -1 },
    [608] = { "fre108", "fre108", sg_none, 127, -1 },
    [609] = { "fre109", "fre109", sg_none, 127, -1 },
    [610] = { "fre110", "fre110", sg_none, 127, -1 },
    [611] = { "fre111", "fre111", sg_none, 127, -1 },
    [612] = { "fre112", "fre112", sg_none, 127, -1 },
    [613] = { "fre113", "fre113", sg_none, 127, -1 },
    [614] = { "fre114", "fre114", sg_none, 127, -1 },
    [615] = { "fre115", "fre115", sg_none, 127, -1 },
    [616] = { "fre116", "fre116", sg_none, 127, -1 },
    [617] = { "fre117", "fre117", sg_none, 127, -1 },
    [618] = { "fre118", "fre118", sg_none, 127, -1 },
    [619] = { "fre119", "fre119", sg_none, 127, -1 },
    [620] = { "fre120", "fre120", sg_none, 127, -1 },
    [621] = { "fre121", "fre121", sg_none, 127, -1 },
    [622] = { "fre122", "fre122", sg_none, 127, -1 },
    [623] = { "fre123", "fre123", sg_none, 127, -1 },
    [624] = { "fre124", "fre124", sg_none, 127, -1 },
    [625] = { "fre125", "fre125", sg_none, 127, -1 },
    [626] = { "fre126", "fre126", sg_none, 127, -1 },
    [627] = { "fre127", "fre127", sg_none, 127, -1 },
    [628] = { "fre128", "fre128", sg_none, 127, -1 },
    [629] = { "fre129", "fre129", sg_none, 127, -1 },
    [630] = { "fre130", "fre130", sg_none, 127, -1 },
    [631] = { "fre131", "fre131", sg_none, 127, -1 },
    [632] = { "fre132", "fre132", sg_none, 127, -1 },
    [633] = { "fre133", "fre133", sg_none, 127, -1 },
    [634] = { "fre134", "fre134", sg_none, 127, -1 },
    [635] = { "fre135", "fre135", sg_none, 127, -1 },
    [636] = { "fre136", "fre136", sg_none, 127, -1 },
    [637] = { "fre137", "fre137", sg_none, 127, -1 },
    [638] = { "fre138", "fre138", sg_none, 127, -1 },
    [639] = { "fre139", "fre139", sg_none, 127, -1 },
    [640] = { "fre140", "fre140", sg_none, 127, -1 },
    [641] = { "fre141", "fre141", sg_none, 127, -1 },
    [642] = { "fre142", "fre142", sg_none, 127, -1 },
    [643] = { "fre143", "fre143", sg_none, 127, -1 },
    [644] = { "fre144", "fre144", sg_none, 127, -1 },
    [645] = { "fre145", "fre145", sg_none, 127, -1 },
    [646] = { "fre146", "fre146", sg_none, 127, -1 },
    [647] = { "fre147", "fre147", sg_none, 127, -1 },
    [648] = { "fre148", "fre148", sg_none, 127, -1 },
    [649] = { "fre149", "fre149", sg_none, 127, -1 },
    [650] = { "fre150", "fre150", sg_none, 127, -1 },
    [651] = { "fre151", "fre151", sg_none, 127, -1 },
    [652] = { "fre152", "fre152", sg_none, 127, -1 },
    [653] = { "fre153", "fre153", sg_none, 127, -1 },
    [654] = { "fre154", "fre154", sg_none, 127, -1 },
    [655] = { "fre155", "fre155", sg_none, 127, -1 },
    [656] = { "fre156", "fre156", sg_none, 127, -1 },
    [657] = { "fre157", "fre157", sg_none, 127, -1 },
    [658] = { "fre158", "fre158", sg_none, 127, -1 },
    [659] = { "fre159", "fre159", sg_none, 127, -1 },
    [660] = { "fre160", "fre160", sg_none, 127, -1 },
    [661] = { "fre161", "fre161", sg_none, 127, -1 },
    [662] = { "fre162", "fre162", sg_none, 127, -1 },
    [663] = { "fre163", "fre163", sg_none, 127, -1 },
    [664] = { "fre164", "fre164", sg_none, 127, -1 },
    [665] = { "fre165", "fre165", sg_none, 127, -1 },
    [666] = { "fre166", "fre166", sg_none, 127, -1 },
    [667] = { "fre167", "fre167", sg_none, 127, -1 },
    [668] = { "fre168", "fre168", sg_none, 127, -1 },
    [669] = { "fre169", "fre169", sg_none, 127, -1 },
    [670] = { "fre170", "fre170", sg_none, 127, -1 },
    [671] = { "fre171", "fre171", sg_none, 127, -1 },
    [672] = { "fre172", "fre172", sg_none, 127, -1 },
    [673] = { "fre173", "fre173", sg_none, 127, -1 },
    [674] = { "fre174", "fre174", sg_none, 127, -1 },
    [675] = { "fre175", "fre175", sg_none, 127, -1 },
    [676] = { "fre176", "fre176", sg_none, 127, -1 },
    [677] = { "fre177", "fre177", sg_none, 127, -1 },
    [678] = { "fre178", "fre178", sg_none, 127, -1 },
    [679] = { "fre179", "fre179", sg_none, 127, -1 },
    [680] = { "fre180", "fre180", sg_none, 127, -1 },
    [681] = { "fre181", "fre181", sg_none, 127, -1 },
    [682] = { "fre182", "fre182", sg_none, 127, -1 },
    [683] = { "fre183", "fre183", sg_none, 127, -1 },
    [684] = { "fre184", "fre184", sg_none, 127, -1 },
    [685] = { "fre185", "fre185", sg_none, 127, -1 },
    [686] = { "fre186", "fre186", sg_none, 127, -1 },
    [687] = { "fre187", "fre187", sg_none, 127, -1 },
    [688] = { "fre188", "fre188", sg_none, 127, -1 },
    [689] = { "fre189", "fre189", sg_none, 127, -1 },
    [690] = { "fre190", "fre190", sg_none, 127, -1 },
    [691] = { "fre191", "fre191", sg_none, 127, -1 },
    [692] = { "fre192", "fre192", sg_none, 127, -1 },
    [693] = { "fre193", "fre193", sg_none, 127, -1 },
    [694] = { "fre194", "fre194", sg_none, 127, -1 },
    [695] = { "fre195", "fre195", sg_none, 127, -1 },
    [696] = { "fre196", "fre196", sg_none, 127, -1 },
    [697] = { "fre197", "fre197", sg_none, 127, -1 },
    [698] = { "fre198", "fre198", sg_none, 127, -1 },
    [699] = { "fre199", "fre199", sg_none, 127, -1 }
};

// DSDHacked
sfxinfo_t   *s_sfx;
int         numsfx;
static char **deh_soundnames;
static byte *sfx_state;

void InitSFX(void)
{
    s_sfx = original_s_sfx;
    numsfx = NUMSFX;

    array_grow(deh_soundnames, numsfx);

    for (int i = 1; i < numsfx; i++)
        deh_soundnames[i] = (*s_sfx[i].name1 ? M_StringDuplicate(s_sfx[i].name1) : NULL);

    array_grow(sfx_state, numsfx);
    memset(sfx_state, 0, numsfx * sizeof(*sfx_state));
}

void FreeSFX(void)
{
    for (int i = 1; i < array_capacity(deh_soundnames); i++)
        if (deh_soundnames[i])
            free(deh_soundnames[i]);

    array_free(deh_soundnames);
    array_free(sfx_state);
}

void dsdh_EnsureSFXCapacity(const int limit)
{
    const int   old_numsfx = numsfx;
    static bool first_allocation = true;
    int         size_delta;

    if (limit < numsfx)
        return;

    if (first_allocation)
    {
        s_sfx = NULL;
        array_grow(s_sfx, old_numsfx + limit);
        memcpy(s_sfx, original_s_sfx, old_numsfx * sizeof(*s_sfx));
        first_allocation = false;
    }
    else
        array_grow(s_sfx, limit);

    numsfx = array_capacity(s_sfx);
    size_delta = numsfx - old_numsfx;
    memset(s_sfx + old_numsfx, 0, size_delta * sizeof(*s_sfx));

    array_grow(sfx_state, size_delta);
    memset(sfx_state + old_numsfx, 0, size_delta * sizeof(*sfx_state));

    for (int i = old_numsfx; i < numsfx; i++)
    {
        s_sfx[i].priority = 127;
        s_sfx[i].lumpnum = -1;
    }
}

int dsdh_GetDehSFXIndex(const char *key, size_t length)
{
    for (int i = 1; i < numsfx; i++)
        if (*s_sfx[i].name1
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

    for (int i = 1; i < array_capacity(deh_soundnames); i++)
        if (deh_soundnames[i] && !strncasecmp(deh_soundnames[i], key, 6))
            return i;

    // is it a number?
    for (const char *c = key; *c; c++)
        if (!isdigit(*c))
            return -1;

    limit = strtol(key, NULL, 10);
    dsdh_EnsureSFXCapacity(limit);

    return limit;
}
