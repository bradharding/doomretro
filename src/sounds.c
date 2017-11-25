/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "sounds.h"

//
// Information about all the music
//
musicinfo_t S_music[] =
{
    { "",        "",                                0, 0, 0 },
    { "e1m1",    "At Doom's Gate",                  0, 0, 0 },
    { "e1m2",    "The Imp's Song",                  0, 0, 0 },
    { "e1m3",    "Dark Halls",                      0, 0, 0 },
    { "e1m4",    "Kitchen Ace (And Taking Names)",  0, 0, 0 },
    { "e1m5",    "Suspense",                        0, 0, 0 },
    { "e1m6",    "On The Hunt",                     0, 0, 0 },
    { "e1m7",    "Demons On The Prey",              0, 0, 0 },
    { "e1m8",    "Sign Of Evil",                    0, 0, 0 },
    { "e1m9",    "Hiding The Secrets",              0, 0, 0 },
    { "e2m1",    "I Sawed The Demons",              0, 0, 0 },
    { "e2m2",    "The Demons From Adrian's Pen",    0, 0, 0 },
    { "e2m3",    "Intermission From DOOM",          0, 0, 0 },
    { "e2m4",    "They're Going To Get You",        0, 0, 0 },
    { "e2m5",    "Demons On The Prey",              0, 0, 0 },
    { "e2m6",    "Sinister",                        0, 0, 0 },
    { "e2m7",    "Waltz Of The Demons",             0, 0, 0 },
    { "e2m8",    "Nobody Told Me About id",         0, 0, 0 },
    { "e2m9",    "n/a",                             0, 0, 0 },
    { "e3m1",    "n/a",                             0, 0, 0 },
    { "e3m2",    "Donna To The Rescue",             0, 0, 0 },
    { "e3m3",    "Deep Into The Code",              0, 0, 0 },
    { "e3m4",    "Sign Of Evil",                    0, 0, 0 },
    { "e3m5",    "Demons On The Prey",              0, 0, 0 },
    { "e3m6",    "Suspense",                        0, 0, 0 },
    { "e3m7",    "Waltz Of The Demons",             0, 0, 0 },
    { "e3m8",    "Facing The Spider",               0, 0, 0 },
    { "e3m9",    "Hiding The Secrets",              0, 0, 0 },
    { "inter",   "Intermission From DOOM",          0, 0, 0 },
    { "intro",   "n/a",                             0, 0, 0 },
    { "bunny",   "Sweet Little Dead Bunny",         0, 0, 0 },
    { "victor",  "The End Of DOOM",                 0, 0, 0 },
    { "introa",  "n/a",                             0, 0, 0 },
    { "runnin",  "Running From Evil",               0, 0, 0 },
    { "stalks",  "The Healer Stalks",               0, 0, 0 },
    { "countd",  "Countdown To Death",              0, 0, 0 },
    { "betwee",  "Between Levels",                  0, 0, 0 },
    { "doom",    "DOOM",                            0, 0, 0 },
    { "the_da",  "In The Dark",                     0, 0, 0 },
    { "shawn",   "Shawn's Got The Shotgun",         0, 0, 0 },
    { "ddtblu",  "The Dave D. Taylor Blues",        0, 0, 0 },
    { "in_cit",  "Into Sandy's City",               0, 0, 0 },
    { "dead",    "The Demon's Dead",                0, 0, 0 },
    { "stlks2",  "The Healer Stalks",               0, 0, 0 },
    { "theda2",  "In The Dark",                     0, 0, 0 },
    { "doom2",   "DOOM",                            0, 0, 0 },
    { "ddtbl2",  "The Dave D. Taylor Blues",        0, 0, 0 },
    { "runni2",  "Running From Evil",               0, 0, 0 },
    { "dead2",   "The Demon's Dead",                0, 0, 0 },
    { "stlks3",  "The Healer Stalks",               0, 0, 0 },
    { "romero",  "Waiting For Romero To Play",      0, 0, 0 },
    { "shawn2",  "Shawn's Got The Shotgun",         0, 0, 0 },
    { "messag",  "Message For The Arch-vile",       0, 0, 0 },
    { "count2",  "Countdown To Death",              0, 0, 0 },
    { "ddtbl3",  "The Dave D. Taylor Blues",        0, 0, 0 },
    { "ampie",   "Bye Bye American Pie",            0, 0, 0 },
    { "theda3",  "In The Dark",                     0, 0, 0 },
    { "adrian",  "Adrian's Asleep",                 0, 0, 0 },
    { "messg2",  "Message For The Arch-vile",       0, 0, 0 },
    { "romer2",  "Waiting For Romero To Play",      0, 0, 0 },
    { "tense",   "Getting Too Tense",               0, 0, 0 },
    { "shawn3",  "Shawn's Got The Shotgun",         0, 0, 0 },
    { "openin",  "Opening To Hell",                 0, 0, 0 },
    { "evil",    "Evil Incarnate",                  0, 0, 0 },
    { "ultima",  "The Ultimate Challenge/Conquest", 0, 0, 0 },
    { "read_m",  "n/a",                             0, 0, 0 },
    { "dm2ttl",  "n/a",                             0, 0, 0 },
    { "dm2int",  "n/a",                             0, 0, 0 },

    // custom music from MUSINFO lump
    { "musinfo", "n/a",                             0, 0, 0 }
};

//
// Information about all the SFX
//
sfxinfo_t S_sfx[] =
{
    // S_sfx[0] needs to be a dummy for odd reasons.
    { "none",   sg_none,     0,                  0,  -1, 0 },
    { "pistol", sg_none,    64,                  0,  -1, 0 },
    { "shotgn", sg_none,    64,                  0,  -1, 0 },
    { "sgcock", sg_none,    64,                  0,  -1, 0 },
    { "dshtgn", sg_none,    64,                  0,  -1, 0 },
    { "dbopn",  sg_none,    64,                  0,  -1, 0 },
    { "dbcls",  sg_none,    64,                  0,  -1, 0 },
    { "dbload", sg_none,    64,                  0,  -1, 0 },
    { "plasma", sg_none,    64,                  0,  -1, 0 },
    { "bfg",    sg_none,    64,                  0,  -1, 0 },
    { "sawup",  sg_saw,     64,                  0,  -1, 0 },
    { "sawidl", sg_saw,    118,                  0,  -1, 0 },
    { "sawful", sg_saw,     64,                  0,  -1, 0 },
    { "sawhit", sg_saw,     64,                  0,  -1, 0 },
    { "rlaunc", sg_none,    64,                  0,  -1, 0 },
    { "rxplod", sg_none,    70,                  0,  -1, 0 },
    { "firsht", sg_none,    70,                  0,  -1, 0 },
    { "firxpl", sg_none,    70,                  0,  -1, 0 },
    { "pstart", sg_none,   100,                  0,  -1, 0 },
    { "pstop",  sg_none,   100,                  0,  -1, 0 },
    { "doropn", sg_none,   100,                  0,  -1, 0 },
    { "dorcls", sg_none,   100,                  0,  -1, 0 },
    { "stnmov", sg_stnmov, 119,                  0,  -1, 0 },
    { "swtchn", sg_none,    78,                  0,  -1, 0 },
    { "swtchx", sg_none,    78,                  0,  -1, 0 },
    { "plpain", sg_none,    96,                  0,  -1, 0 },
    { "dmpain", sg_none,    96,                  0,  -1, 0 },
    { "popain", sg_none,    96,                  0,  -1, 0 },
    { "vipain", sg_none,    96,                  0,  -1, 0 },
    { "mnpain", sg_none,    96,                  0,  -1, 0 },
    { "pepain", sg_none,    96,                  0,  -1, 0 },
    { "slop",   sg_none,    78,                  0,  -1, 0 },
    { "itemup", sg_itemup,  78,                  0,  -1, 0 },
    { "wpnup",  sg_wpnup,   78,                  0,  -1, 0 },
    { "oof",    sg_oof,     96,                  0,  -1, 0 },
    { "telept", sg_none,    32,                  0,  -1, 0 },
    { "posit1", sg_none,    98,                  0,  -1, 0 },
    { "posit2", sg_none,    98,                  0,  -1, 0 },
    { "posit3", sg_none,    98,                  0,  -1, 0 },
    { "bgsit1", sg_none,    98,                  0,  -1, 0 },
    { "bgsit2", sg_none,    98,                  0,  -1, 0 },
    { "sgtsit", sg_none,    98,                  0,  -1, 0 },
    { "cacsit", sg_none,    98,                  0,  -1, 0 },
    { "brssit", sg_none,    94,                  0,  -1, 0 },
    { "cybsit", sg_none,    92,                  0,  -1, 0 },
    { "spisit", sg_none,    90,                  0,  -1, 0 },
    { "bspsit", sg_none,    90,                  0,  -1, 0 },
    { "kntsit", sg_none,    90,                  0,  -1, 0 },
    { "vilsit", sg_none,    90,                  0,  -1, 0 },
    { "mansit", sg_none,    90,                  0,  -1, 0 },
    { "pesit",  sg_none,    90,                  0,  -1, 0 },
    { "sklatk", sg_none,    70,                  0,  -1, 0 },
    { "sgtatk", sg_none,    70,                  0,  -1, 0 },
    { "skepch", sg_none,    70,                  0,  -1, 0 },
    { "vilatk", sg_none,    70,                  0,  -1, 0 },
    { "claw",   sg_none,    70,                  0,  -1, 0 },
    { "skeswg", sg_none,    70,                  0,  -1, 0 },
    { "pldeth", sg_none,    32,                  0,  -1, 0 },
    { "pdiehi", sg_none,    32,                  0,  -1, 0 },
    { "podth1", sg_none,    70,                  0,  -1, 0 },
    { "podth2", sg_none,    70,                  0,  -1, 0 },
    { "podth3", sg_none,    70,                  0,  -1, 0 },
    { "bgdth1", sg_none,    70,                  0,  -1, 0 },
    { "bgdth2", sg_none,    70,                  0,  -1, 0 },
    { "sgtdth", sg_none,    70,                  0,  -1, 0 },
    { "cacdth", sg_none,    70,                  0,  -1, 0 },
    { "skldth", sg_none,    70,                  0,  -1, 0 },
    { "brsdth", sg_none,    32,                  0,  -1, 0 },
    { "cybdth", sg_none,    32,                  0,  -1, 0 },
    { "spidth", sg_none,    32,                  0,  -1, 0 },
    { "bspdth", sg_none,    32,                  0,  -1, 0 },
    { "vildth", sg_none,    32,                  0,  -1, 0 },
    { "kntdth", sg_none,    32,                  0,  -1, 0 },
    { "pedth",  sg_none,    32,                  0,  -1, 0 },
    { "skedth", sg_none,    32,                  0,  -1, 0 },
    { "posact", sg_none,   120,                  0,  -1, 0 },
    { "bgact",  sg_none,   120,                  0,  -1, 0 },
    { "dmact",  sg_none,   120,                  0,  -1, 0 },
    { "bspact", sg_none,   100,                  0,  -1, 0 },
    { "bspwlk", sg_none,   100,                  0,  -1, 0 },
    { "vilact", sg_none,   100,                  0,  -1, 0 },
    { "noway",  sg_oof,     78,                  0,  -1, 0 },
    { "barexp", sg_none,    60,                  0,  -1, 0 },
    { "punch",  sg_none,    64,                  0,  -1, 0 },
    { "hoof",   sg_none,    70,                  0,  -1, 0 },
    { "metal",  sg_none,    70,                  0,  -1, 0 },
    { "chgun",  sg_none,    64, &S_sfx[sfx_pistol],   0, 0 },
    { "tink",   sg_none,    60,                  0,  -1, 0 },
    { "bdopn",  sg_none,   100,                  0,  -1, 0 },
    { "bdcls",  sg_none,   100,                  0,  -1, 0 },
    { "itmbk",  sg_none,   100,                  0,  -1, 0 },
    { "flame",  sg_none,    32,                  0,  -1, 0 },
    { "flamst", sg_none,    32,                  0,  -1, 0 },
    { "getpow", sg_getpow,  60,                  0,  -1, 0 },
    { "bospit", sg_none,    70,                  0,  -1, 0 },
    { "boscub", sg_none,    70,                  0,  -1, 0 },
    { "bossit", sg_none,    70,                  0,  -1, 0 },
    { "bospn",  sg_none,    70,                  0,  -1, 0 },
    { "bosdth", sg_none,    70,                  0,  -1, 0 },
    { "manatk", sg_none,    70,                  0,  -1, 0 },
    { "mandth", sg_none,    70,                  0,  -1, 0 },
    { "sssit",  sg_none,    70,                  0,  -1, 0 },
    { "ssdth",  sg_none,    70,                  0,  -1, 0 },
    { "keenpn", sg_none,    70,                  0,  -1, 0 },
    { "keendt", sg_none,    70,                  0,  -1, 0 },
    { "skeact", sg_none,    70,                  0,  -1, 0 },
    { "skesit", sg_none,    70,                  0,  -1, 0 },
    { "skeatk", sg_none,    70,                  0,  -1, 0 },
    { "radio",  sg_none,    60,                  0,  -1, 0 },

    // killough 11/98: dog sounds
    { "dgsit",  sg_none,    98,                  0,  -1, 0 },
    { "dgatk",  sg_none,    70,                  0,  -1, 0 },
    { "dgact",  sg_none,   120,                  0,  -1, 0 },
    { "dgdth",  sg_none,    70,                  0,  -1, 0 },
    { "dgpain", sg_none,    96,                  0,  -1, 0 },

    { "secret", sg_none,    60,                  0,  -1, 0 }
};
