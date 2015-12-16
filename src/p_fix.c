/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "doomdata.h"
#include "doomdef.h"
#include "p_fix.h"
#include "r_defs.h"

vertexfix_t vertexfix[] =
{
   // mission, episode, map, vertex,  oldx,  oldy,  newx,  newy

    { doom,          1,   3,    771,  -328, -1920,  -320, -1920 },
    { doom,          2,   2,   1344,  1312,  3992,  1312,  4000 },

    { doom2,         1,   1,    288,  1984,  3784,  1984,  3776 },
    { doom2,         1,   1,    288,   320,  1416,   320,  1408 },
    { doom2,         1,   2,    273,   736,  1216,   736,  1208 },
    { doom2,         1,   2,    289,  2336,  2952,  2336,  2944 },

    { -1,            0,   0,      0,     0,     0,     0,     0 }
};

linefix_t linefix[] =
{
   // mission,    episode, map, linedef, side, toptexture, middletexture, bottomtexture,  offset, rowoffset, flags,                   special,                                    tag

    { doom,             1,   1,     159,    1, "",         "",            "",            DEFAULT,   DEFAULT, ML_SECRET,               DEFAULT,                                    DEFAULT },
    { doom,             1,   1,     471,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_LowerToLowestFloor_ChangesTexture, DEFAULT },
    { doom,             1,   1,     477,    1, "",         "",            "",            DEFAULT,   DEFAULT, ML_SECRET,               DEFAULT,                                    DEFAULT },
    { doom,             1,   1,     479,    1, "",         "",            "",            DEFAULT,   DEFAULT, ML_SECRET,               DEFAULT,                                    DEFAULT },
    { doom,             1,   1,     480,    1, "",         "",            "",            DEFAULT,   DEFAULT, ML_SECRET,               DEFAULT,                                    DEFAULT },
    { doom,             1,   1,     482,    1, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW | ML_SECRET, DEFAULT,                                    DEFAULT },
    { doom,             1,   1,     483,    1, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW | ML_SECRET, DEFAULT,                                    DEFAULT },

    { doom,             1,   2,     134,    1, "STARTAN2", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   2,     574,    0, "",         "",            "",            DEFAULT,       -24, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom,             1,   2,     575,    0, "",         "",            "",               -128,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   2,     576,    0, "",         "-",           "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom,             1,   2,     577,    0, "",         "-",           "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom,             1,   2,     578,    0, "",         "-",           "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },

    { doom,             1,   4,     321,    0, "",         "",            "STEP2",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   4,     327,    0, "",         "",            "STEP2",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   4,     338,    0, "",         "",            "STEP2",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   4,     346,    0, "",         "",            "STEP2",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   4,     693,    1, "BROWN1",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             1,   6,    1026,    0, "",         "SUPPORT2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             1,   7,     447,    0, "",         "-",           "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom,             1,   7,     450,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom,             1,   7,     451,    0, "",         "-",           "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom,             1,   7,     452,    0, "",         "-",           "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom,             1,   7,     744,    1, "",         "",            "TEKWALL1",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   7,     745,    1, "",         "",            "TEKWALL1",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   7,     746,    1, "",         "",            "TEKWALL1",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             1,   7,     747,    1, "",         "",            "TEKWALL1",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             1,   8,     176,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_TRIGGER666,           DEFAULT,                                    DEFAULT },

    { doom,             2,   1,      11,    0, "STONE2",   "",            "STONE2",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             2,   2,     947,    1, "BROWN1",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   2,    1494,    1, "ICKWALL2", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   2,    1596,    1, "WOOD1",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             2,   3,     502,    1, "",         "MIDVINE2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   3,     611,    1, "",         "MIDVINE2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   3,     936,    1, "",         "MIDVINE2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   3,     938,    1, "",         "MIDVINE2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   3,     905,    1, "",         "MIDBRN1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   3,     906,    1, "",         "MIDBRN1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   3,     907,    1, "",         "MIDBRN1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   3,     908,    1, "",         "MIDBRN1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             2,   4,     551,    1, "PIPE4",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   4,     865,    1, "",         "",            "STEP5",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   4,     955,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   4,    1062,    0, "GRAYVINE", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   4,    1071,    0, "MARBLE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             2,   5,     382,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                         34 },
    { doom,             2,   5,     388,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                         34 },
    { doom,             2,   5,     590,    0, "",         "",            "STEP1",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   5,     590,    1, "BROVINE",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   5,    1027,    1, "COMPSPAN", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             2,   6,    1091,    1, "COMPSPAN", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             2,   7,     193,    1, "BROWNHUG", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   7,    1286,    0, "",         "",            "SHAWN2",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             2,   9,     110,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   9,     115,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   9,     121,    1, "GSTONE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   9,     123,    1, "GSTONE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             2,   9,     140,    1, "GSTONE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             3,   2,     146,    1, "",         "MIDVINE1",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             3,   3,     854,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   3,     855,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   3,     994,    1, "SLADWALL", "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   3,     995,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   3,     996,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             3,   4,     470,    0, "BIGDOOR2", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   4,    1069,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 NoSpecial,                                  DEFAULT },

    { doom,             3,   5,    1285,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 NoSpecial,                                  DEFAULT },
    { doom,             3,   5,    1299,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 NoSpecial,                                  DEFAULT },

    { doom,             3,   6,     298,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Teleport_MonstersOnly,                   DEFAULT },
    { doom,             3,   6,     299,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Teleport_MonstersOnly,                   DEFAULT },
    { doom,             3,   6,     408,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   6,     410,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   6,     412,    1, "",         "BRNSMAL1",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   6,     414,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             3,   7,     901,    1, "",         "",            "STEP2",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             3,   7,     971,    1, "SP_HOT1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             3,   9,      24,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DR_Door_OpenWaitClose_AlsoMonsters,         DEFAULT },
    { doom,             3,   9,     102,    1, "",         "",            "STONE",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             4,   1,     252,    1, "SUPPORT3", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   1,     253,    1, "SUPPORT3", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   1,     254,    1, "SUPPORT3", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   1,     255,    1, "SUPPORT3", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   1,     470,    0, "GSTONE1",  "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             4,   2,     165,    1, "WOOD5",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             4,   3,     735,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToLowestCeiling,              DEFAULT },

    { doom,             4,   4,     427,    1, "BROWNHUG", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   4,     558,    1, "BROWNHUG", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   4,     567,    0, "BROWNHUG", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   4,     572,    0, "BROWNHUG", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             4,   5,       0,    0, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,       5,    1, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,      19,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,      35,    1, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     109,    1, "GSTONE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     155,    1, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     182,    0, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     183,    0, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     184,    0, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     367,    0, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     368,    0, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     407,    0, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     408,    0, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     625,    1, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     711,    1, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   5,     713,    1, "",         "",            "FIRELAV3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             4,   6,     475,    1, "MARBLE2",  "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   6,     476,    1, "MARBLE2",  "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   6,     479,    1, "MARBLE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   6,     480,    1, "MARBLE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   6,     481,    1, "MARBLE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   6,     482,    1, "MARBLE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   6,    1129,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_TRIGGER666,           DEFAULT,                                    DEFAULT },

    { doom,             4,   7,     325,    1, "",         "BRNBIGR",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   7,     326,    1, "",         "BRNBIGC",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   7,     327,    1, "",         "BRNBIGL",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   7,     451,    1, "",         "BRNBIGR",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   7,     452,    1, "",         "BRNBIGC",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   7,     453,    1, "",         "BRNBIGL",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom,             4,   8,     231,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_TRIGGER666,           DEFAULT,                                    DEFAULT },
    { doom,             4,   8,     418,    0, "",         "SP_HOT1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   8,     419,    0, "",         "SP_HOT1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   8,     420,    0, "",         "SP_HOT1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom,             4,   8,     425,    0, "",         "SP_HOT1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

   // mission,    episode, map, linedef, side, toptexture, middletexture, bottomtexture,  offset, rowoffset, flags,                   special,                                        tag

    { doom2,            1,   1,     169,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   1,     334,    1, "",         "MIDBARS3",    "BROWNGRN",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   1,     335,    1, "",         "MIDBARS3",    "BROWNGRN",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   1,     369,    1, "",         "MIDBARS3",    "BROWNGRN",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,   2,     327,    0, "",         "",            "STONE4",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   2,     328,    0, "",         "",            "STONE4",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   2,     338,    0, "",         "",            "STONE4",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   2,     339,    0, "",         "",            "STONE4",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,   4,     108,    0, "STONE",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     109,    0, "STONE",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     110,    0, "STONE",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     111,    0, "STONE",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     127,    0, "STONE",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     128,    0, "STONE",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     456,    1, "SUPPORT3", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,       3,    1, "",         "",            "STUCCO",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     187,    1, "",         "",            "STUCCO",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     200,    1, "",         "",            "STUCCO",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   4,     201,    1, "",         "",            "STUCCO",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,   5,     489,    1, "SUPPORT3", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   5,     560,    1, "SUPPORT3", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,   7,     162,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_TRIGGER666,           DEFAULT,                                    DEFAULT },
    { doom2,            1,   7,     168,    1, "",         "",            "TANROCK4",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,   8,     101,    1, "BRICK7",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   8,     232,    0, "",         "",            "STEP2",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,   8,     270,    1, "COMPTALL", "",            "",            DEFAULT,   DEFAULT, ML_DONTPEGBOTTOM,        DEFAULT,                                    DEFAULT },
    { doom2,            1,   8,     276,    1, "COMPTALL", "",            "",            DEFAULT,   DEFAULT, ML_DONTPEGBOTTOM,        DEFAULT,                                    DEFAULT },
    { doom2,            1,   8,     598,    0, "",         "GRAY5",       "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  10,     880,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  10,     899,    1, "",         "-",           "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  10,     900,    1, "",         "-",           "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  10,     901,    1, "",         "-",           "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  10,     902,    1, "",         "-",           "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  12,     269,    0, "",         "PANEL6",      "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  12,     632,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 NoSpecial,                                  DEFAULT },
    { doom2,            1,  12,     648,    1, "",         "",            "PIPES",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  12,     773,    1, "",         "",            "PANCASE2",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  13,     305,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     308,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     318,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     331,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     529,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     546,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     547,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     548,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     553,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     622,    1, "BROWNGRN", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     790,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     791,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     792,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     806,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     807,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     810,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     811,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     879,    1, "BROWN144", "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  13,     880,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  14,     191,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     237,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     331,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     429,    0, "BSTONE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     430,    0, "BSTONE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     531,    0, "BSTONE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     607,    1, "BSTONE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     608,    1, "BSTONE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     609,    1, "BSTONE2",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     626,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     786,    1, "TANROCK5", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     787,    1, "TANROCK5", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     788,    1, "TANROCK5", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     789,    1, "TANROCK5", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     791,    1, "TANROCK5", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,     792,    1, "TANROCK5", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,    1259,    1, "BSTONE2",  "",            "BSTONE2",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  14,    1305,    1, "BSTONE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  15,      94,    1, "BRICK1",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  15,      95,    1, "BRICK1",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  15,     989,    1, "BRICK10",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  16,     162,    1, "BRICK6",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  16,     303,    0, "STUCCO1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  16,     304,    0, "STUCCO1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  16,     328,    1, "",         "",            "ROCK3",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  17,     182,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  17,     316,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  17,     379,    1, "METAL2",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  17,     726,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },

    { doom2,            1,  18,     451,    0, "",         "DOORSTOP",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  18,     459,    0, "",         "DOORSTOP",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  18,     574,    0, "GRAYVINE", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  19,     286,    1, "",         "",            "STEP4",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     287,    1, "",         "",            "STEP4",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     288,    1, "",         "",            "STEP4",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     355,    1, "STONE2",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     455,    1, "",         "",            "METAL",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     464,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 NoSpecial,                                  DEFAULT },
    { doom2,            1,  19,     529,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     577,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     578,    1, "",         "BRNSMAL1",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     618,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,     736,    0, "SLADWALL", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,    1181,    1, "MARBLE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  19,    1229,    0, "MARBGRAY", "",            "",            DEFAULT,        16, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  20,      73,    1, "",         "",            "SP_HOT1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  20,      74,    1, "",         "",            "SP_HOT1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  20,      75,    1, "",         "",            "SP_HOT1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  20,      76,    1, "",         "",            "SP_HOT1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  20,     453,    1, "",         "",            "STONE7",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  22,     158,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     223,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     442,    1, "METAL",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     443,    1, "METAL",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     530,    0, "METAL2",   "",            "METAL2",      DEFAULT,   DEFAULT, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     539,    0, "METAL2",   "",            "METAL2",      DEFAULT,   DEFAULT, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     542,    0, "METAL2",   "",            "METAL2",      DEFAULT,   DEFAULT, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     543,    0, "METAL2",   "",            "METAL2",      DEFAULT,   DEFAULT, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     544,    0, "METAL2",   "",            "METAL2",      DEFAULT,   DEFAULT, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     545,    0, "METAL2",   "",            "METAL2",      DEFAULT,   DEFAULT, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     548,    0, "METAL2",   "",            "METAL2",      DEFAULT,   DEFAULT, ML_DRAWASWALL,           DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     607,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  22,     610,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },

    { doom2,            1,  24,     687,    1, "",         "",            "SILVER2",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  24,     688,    1, "",         "",            "SILVER2",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  25,     348,    1, "",         "MIDSPACE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  25,     349,    1, "",         "MIDSPACE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  25,     436,    1, "BFALL1",   "BFALL1",      "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  26,      14,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  26,     761,    1, "METAL2",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  27,     342,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_BLOCKING,             DEFAULT,                                    DEFAULT },
    { doom2,            1,  27,     580,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 D1_Door_OpenStay,                           DEFAULT },
    { doom2,            1,  27,     581,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 D1_Door_OpenStay,                           DEFAULT },
    { doom2,            1,  27,     582,    1, "ZIMMER3",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  27,     727,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 SR_Floor_LowerTo8AboveHighestFloor,         DEFAULT },
    { doom2,            1,  27,     810,    1, "",         "",            "WOODVERT",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  27,     814,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DR_Door_Red_OpenWaitClose,                  DEFAULT },

    { doom2,            1,  28,      38,    1, "",         "",            "ZIMMER8",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,      39,    1, "",         "",            "ZIMMER8",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     103,    0, "ASHWALL6", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     104,    0, "ASHWALL6", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     105,    0, "ASHWALL6", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     106,    0, "ASHWALL6", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     107,    0, "ASHWALL6", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     161,    1, "",         "",            "ZIMMER8",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     170,    1, "",         "",            "BFALL4",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     213,    1, "",         "",            "ZIMMER8",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     214,    1, "",         "",            "ZIMMER8",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     215,    1, "",         "",            "ZIMMER8",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     221,    0, "",         "",            "BFALL4",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     256,    1, "",         "",            "ZIMMER8",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     388,    1, "",         "",            "FIREBLU2",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     391,    0, "BIGDOOR5", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     531,    1, "WOOD8",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     547,    1, "WOOD8",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     548,    1, "WOOD8",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     584,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     585,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     586,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     587,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     588,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     589,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     590,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     591,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     592,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     593,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     594,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     595,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor,                 5 },
    { doom2,            1,  28,     650,    1, "",         "MIDBARS1",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     651,    1, "",         "MIDBARS1",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  28,     652,    1, "",         "MIDBARS1",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  29,     405,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     406,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     407,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     408,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     516,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     517,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     518,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     519,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     524,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     525,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     526,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     527,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,     603,    1, "WOOD5",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,    1138,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,    1139,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,    1140,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,    1141,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,    1146,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,    1147,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,    1148,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  29,    1149,    1, "",         "",            "SUPPORT3",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  30,      55,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },

    { doom2,            1,  31,      32,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,      34,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,      41,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,      43,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,      57,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     137,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     163,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     210,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     218,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     226,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     234,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     243,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     251,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     259,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     266,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     274,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     282,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     316,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     330,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     338,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     364,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     409,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     431,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     452,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     459,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     569,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { doom2,            1,  31,     594,    0, "ZDOORB1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { doom2,            1,  33,     400,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                         41 },
    { doom2,            1,  33,     401,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                         41 },
    { doom2,            1,  33,     559,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                         41 },
    { doom2,            1,  33,     560,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                         41 },

   // mission,    episode, map, linedef, side, toptexture, middletexture, bottomtexture,  offset, rowoffset, flags,                   special,                                        tag

    { pack_nerve,       1,   2,     431,    1, "",         "BRNSMALR",    "",                 11,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,     433,    1, "",         "BRNSMALC",    "",                 -5,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,     427,    1, "",         "BRNSMALL",    "",                 11,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,     633,    1, "",         "BRNSMALR",    "",                 11,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,     411,    1, "",         "BRNSMALC",    "",                 -5,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,     409,    1, "",         "BRNSMALL",    "",                 11,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,     638,    1, "",         "BRNSMALR",    "",                 11,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,     639,    1, "",         "BRNSMALC",    "",                 -5,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,     637,    1, "",         "BRNSMALL",    "",                 11,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1070,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1071,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1072,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1073,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1173,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1175,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1221,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1222,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1223,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1245,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1246,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1247,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1271,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1273,    1, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1208,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1209,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1210,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1204,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1205,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1206,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1248,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1232,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1233,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1234,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1228,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1229,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   2,    1230,    0, "",         "",            "BROWN96",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_nerve,       1,   3,    1406,    1, "",         "",            "TANROCK5",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   3,    1429,    0, "",         "",            "TANROCK5",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   3,    1434,    0, "",         "",            "TANROCK5",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_nerve,       1,   4,    2234,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   4,    3765,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   4,    3768,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   4,    4456,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   4,    4457,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   4,    4458,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   4,    4461,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   4,    4462,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   4,    4463,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },

    { pack_nerve,       1,   5,    1361,    0, "",         "MIDGRATE",    "",                 30,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   5,    2243,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_nerve,       1,   7,     739,    1, "",         "MIDGRATE",    "",               -150,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1374,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1375,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1376,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1377,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1382,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1383,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1384,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1540,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1541,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    1542,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    2542,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    2547,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    2550,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_nerve,       1,   7,    2552,    1, "",         "MIDGRATE",    "",            DEFAULT,      -256, DEFAULT,                 DEFAULT,                                    DEFAULT },

   // mission,    episode, map, linedef, side, toptexture, middletexture, bottomtexture,  offset, rowoffset, flags,                   special,                                        tag

    { pack_plut,        1,   4,     303,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,   4,     308,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,   4,     762,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,   4,     763,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,   6,    1337,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,   6,    1343,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,   8,     236,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,   8,     239,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,   8,     247,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,   8,     249,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  10,     254,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  10,     548,    0, "",         "",            "METAL",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  10,    1010,    0, "GSTONE1",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  12,     230,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  13,     107,    1, "A-BROWN1", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  13,     119,    1, "A-BROWN1", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  13,    1060,    0, "A-BROWN1", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  14,    1099,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  14,    1103,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  14,    1109,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  14,    1113,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  15,     407,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  15,     874,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  16,     667,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  17,     253,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  17,     265,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  17,     379,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  18,     986,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  19,     361,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  19,     366,    1, "",         "MIDBRONZ",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  20,     267,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     276,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     278,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     297,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     315,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     331,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     333,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     511,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     517,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,     904,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,    1110,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  20,    1115,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  22,      58,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,      61,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,     375,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,     393,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1033,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1034,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1035,    1, "",         "A-RAIL1",     "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1085,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1086,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1087,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1088,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1089,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1090,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1091,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1122,    0, "METAL",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1123,    1, "",         "",            "METAL",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1135,    0, "METAL",    "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1136,    1, "",         "",            "METAL",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1680,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1681,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1682,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1683,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1684,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1685,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  22,    1686,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  23,    1100,    0, "",         "",            "",            DEFAULT,        16, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  23,    1353,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  23,    1463,    1, "",         "BRNSMALR",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  23,    1468,    1, "",         "BRNSMALC",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  23,    1469,    1, "",         "BRNSMALL",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  25,    1073,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1074,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1075,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1076,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1077,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1078,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1079,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1152,    0, "A-BROCK2", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1153,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1154,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1155,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1156,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1157,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1158,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  25,    1159,    0, "",         "",            "",            DEFAULT,   DEFAULT, ML_DONTDRAW,             DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  26,     322,    0, "A-MUD",    "A-MUD",       "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  26,     323,    0, "A-MUD",    "A-MUD",       "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  28,     179,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,     195,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,     199,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,     204,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,     675,    0, "",         "",            "BRICK10",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,     676,    0, "",         "",            "BRICK10",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,     834,    0, "",         "",            "WOOD8",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,     835,    0, "",         "",            "WOOD8",       DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,    2073,    0, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,    2352,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,    2360,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,    2460,    0, "BIGDOOR7", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,    2496,    0, "BRICK10",  "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  28,    2496,    1, "",         "",            "METAL2",      DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  29,    2842,    0, "PANCASE2", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  30,     730,    0, "ROCKRED1", "",            "ROCKRED1",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  31,     682,    1, "",         "MIDBARS1",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_plut,        1,  32,     569,    0, "A-MOSROK", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  32,     570,    0, "A-MOSROK", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  32,     571,    0, "A-MOSROK", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  32,     572,    0, "A-MOSROK", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_plut,        1,  32,     805,    0, "A-BRICK3", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

   // mission,    episode, map, linedef, side, toptexture, middletexture, bottomtexture,  offset, rowoffset, flags,                   special,                                        tag

    { pack_tnt,         1,   1,      47,    0, "",         "",            "",                  8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,   3,     880,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,     885,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,     889,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,     890,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1108,    0, "",         "",            "",                  8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1110,    0, "",         "",            "",                  8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1111,    0, "",         "",            "",                 24,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1113,    0, "",         "",            "",                 24,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1120,    0, "",         "",            "",                 24,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1121,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1122,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1124,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1132,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1134,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1135,    0, "",         "",            "",                 -8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1140,    0, "",         "",            "",                -24,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1144,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1145,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1153,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1211,    0, "",         "",            "",                 48,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1221,    0, "",         "",            "",                112,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1249,    0, "",         "",            "",                 45,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1251,    0, "",         "",            "",                -45,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1291,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1293,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1298,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1299,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1300,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1301,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1302,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1304,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1306,    0, "",         "",            "",                 11,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1310,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1315,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1316,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1318,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1319,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1320,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1321,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1324,    0, "",         "",            "",                -11,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1327,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1331,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1332,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1333,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1335,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1336,    0, "",         "",            "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1337,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1338,    0, "",         "",            "",                 24,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1339,    0, "",         "",            "",                 -8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1340,    0, "",         "",            "",                  8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1341,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1346,    0, "",         "",            "",                  8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1348,    0, "",         "",            "",                -16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1350,    0, "",         "",            "",                 24,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1354,    0, "",         "",            "",                -16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1356,    0, "",         "",            "",                  8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1358,    0, "",         "",            "",                 -8,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1362,    0, "",         "",            "",                -24,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1364,    0, "",         "",            "",                 16,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1366,    0, "",         "",            "",                  3,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,   3,    1367,    0, "",         "",            "",                 11,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,   7,     999,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },
    { pack_tnt,         1,   7,    1000,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },
    { pack_tnt,         1,   7,    1001,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },
    { pack_tnt,         1,   7,    1002,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },
    { pack_tnt,         1,   7,    1003,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },
    { pack_tnt,         1,   7,    1004,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },
    { pack_tnt,         1,   7,    1005,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },
    { pack_tnt,         1,   7,    1006,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },
    { pack_tnt,         1,   7,    1007,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 W1_Floor_RaiseToNextHighestFloor_Fast,           16 },

    { pack_tnt,         1,  11,    1087,    0, "",         "",            "GRAY4",       DEFAULT,         2, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  18,      52,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  18,      57,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  18,      63,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  18,      69,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  18,      76,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  18,      77,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  18,      85,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  18,      90,    1, "",         "MIDBARS3",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  19,     605,    1, "",         "SMGLASS1",    "",            DEFAULT,       -64, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  19,     607,    1, "",         "SMGLASS1",    "",            DEFAULT,       -64, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  19,     841,    1, "",         "SMGLASS1",    "",            DEFAULT,       -64, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  19,     845,    1, "",         "SMGLASS1",    "",            DEFAULT,       -64, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  19,    1369,    1, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  19,    1932,    1, "",         "SMGLASS1",    "",            DEFAULT,       -64, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  19,    1937,    1, "",         "SMGLASS1",    "",            DEFAULT,       -64, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  21,     357,    1, "",         "DOWINDOW",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  21,    1138,    0, "PANEL4",   "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  23,    1777,    0, "",         "MIDBARS3",    "",            DEFAULT,        16, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  23,    1781,    0, "",         "MIDBARS3",    "",            DEFAULT,        16, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  23,    1785,    0, "",         "MIDBARS3",    "",            DEFAULT,        16, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  23,    1790,    0, "",         "MIDBARS3",    "",            DEFAULT,        16, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  23,    1843,    0, "",         "MIDBARS3",    "",            DEFAULT,        16, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  25,    1287,    0, "",         "MIDGRATE",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  26,     813,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  26,     814,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  26,     815,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  26,    1297,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  26,    1298,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  26,    1299,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  26,    1300,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  26,    1301,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  26,    1302,    1, "",         "TYIRONLG",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  27,    1231,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    1234,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    1237,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    1240,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    1243,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    1246,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    1249,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    1252,    1, "",         "BRNSMAL2",    "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    1604,    1, "ROCKRED1", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    2002,    1, "ROCKRED1", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    2007,    1, "ROCKRED1", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    2008,    1, "ROCKRED1", "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    2131,    1, "",         "",            "ROCKRED1",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    2140,    1, "",         "",            "ROCKRED1",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  27,    2141,    1, "",         "",            "ROCKRED1",    DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  31,     138,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 S1_Floor_LowerToLowestFloor,                DEFAULT },
    { pack_tnt,         1,  31,     279,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 S1_Floor_LowerToLowestFloor,                DEFAULT },
    { pack_tnt,         1,  31,    1251,    0, "",         "",            "",            DEFAULT,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  31,    1252,    0, "",         "",            "",            DEFAULT,         4, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  31,    1403,    1, "",         "BRICK5",      "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  31,    2527,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 S1_Floor_LowerToLowestFloor,                DEFAULT },
    { pack_tnt,         1,  31,    2528,    1, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 S1_Floor_LowerToLowestFloor,                DEFAULT },
    { pack_tnt,         1,  31,    2643,    1, "",         "DRFRONT",     "",                 32,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { pack_tnt,         1,  32,     622,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     627,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     628,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     629,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     630,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     631,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     632,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     633,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     634,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     635,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },
    { pack_tnt,         1,  32,     636,    1, "",         "",            "GSTONE1",     DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT },

    { -1,               0,   0,       0,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                 DEFAULT,                                    DEFAULT }
};

sectorfix_t sectorfix[] =
{
   // mission,    episode, map, sector, floorpic,  ceilingpic, floorheight, ceilingheight, special,     tag

    { doom,             1,   1,     14, "FLAT18",  "",             DEFAULT,       DEFAULT, DEFAULT, DEFAULT },

    { doom,             1,   4,     41, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },

    { doom,             1,   9,     46, "",        "",             DEFAULT,           160, DEFAULT, DEFAULT },

    { doom,             2,   4,    173, "BLOOD3",  "",             DEFAULT,       DEFAULT, DEFAULT, DEFAULT },
    { doom,             2,   4,    177, "BLOOD3",  "",             DEFAULT,       DEFAULT, DEFAULT, DEFAULT },

    { doom,             2,   5,    127, "",        "",             DEFAULT,       DEFAULT, DEFAULT,      34 },

    { doom,             3,   5,    161, "",        "",                   0,       DEFAULT, DEFAULT, DEFAULT },

    { doom,             3,   7,     20, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom,             3,   7,     23, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },

    { doom,             4,   3,    124, "",        "",             DEFAULT,       DEFAULT,       0, DEFAULT },
    { doom,             4,   3,    125, "",        "",             DEFAULT,       DEFAULT,       0, DEFAULT },

    { doom,             4,   7,    263, "",        "",             DEFAULT,       DEFAULT,       0, DEFAULT },
    { doom,             4,   7,    264, "",        "",             DEFAULT,       DEFAULT,       0, DEFAULT },

    { doom2,            1,   1,     29, "RROCK09", "",             DEFAULT,       DEFAULT, DEFAULT, DEFAULT },

    { doom2,            1,   3,    107, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },

    { doom2,            1,   4,     10, "SLIME15", "",             DEFAULT,       DEFAULT, DEFAULT, DEFAULT },
    { doom2,            1,   4,     19, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom2,            1,   4,     20, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom2,            1,   4,     23, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom2,            1,   4,     28, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom2,            1,   4,     33, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom2,            1,   4,     34, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom2,            1,   4,     83, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom2,            1,   4,     85, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },
    { doom2,            1,   4,     72, "",        "",                  32,       DEFAULT, DEFAULT, DEFAULT },
    { doom2,            1,   4,     76, "",        "",                  32,       DEFAULT, DEFAULT, DEFAULT },

    { doom2,            1,   5,      4, "",        "",             DEFAULT,       DEFAULT, DEFAULT,       0 },

    { doom2,            1,  14,    106, "",        "",                 176,       DEFAULT, DEFAULT, DEFAULT },

    { doom2,            1,  15,    147, "",        "",             DEFAULT,       DEFAULT,       0, DEFAULT },

    { doom2,            1,  19,     63, "",        "",                 176,       DEFAULT, DEFAULT, DEFAULT },

    { doom2,            1,  27,     80, "",        "FLAT5_2",      DEFAULT,       DEFAULT, DEFAULT, DEFAULT },
    { doom2,            1,  27,     93, "",        "",             DEFAULT,       DEFAULT,       0, DEFAULT },

    { pack_nerve,       1,   4,    868, "",        "",             DEFAULT,       DEFAULT,       0, DEFAULT },

    { pack_plut,        1,   8,    130, "",        "",             DEFAULT,             0, DEFAULT, DEFAULT },

    { pack_plut,        1,  26,    156, "",        "",             DEFAULT,       DEFAULT,       0, DEFAULT },

    { pack_tnt,         1,   3,    260, "",        "CEIL3_3",      DEFAULT,       DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt,         1,   3,    329, "",        "CEIL3_3",      DEFAULT,       DEFAULT, DEFAULT, DEFAULT },

    { -1,               0,   0,      0, "",        "",             DEFAULT,       DEFAULT, DEFAULT, DEFAULT },
};

thingfix_t thingfix[] =
{
   // mission,    episode, map, thing, type,                      oldx,    oldy,    newx,    newy,   angle,  options

    { doom,             1,   6,    16, Demon,                      800,    -704,     800,    -688, DEFAULT,  DEFAULT                          },
    { doom,             1,   6,   403, Spectre,                  -2016,    2096,   -2016,    2080, DEFAULT,  DEFAULT                          },

    { doom,             1,   7,    15, Medikit,                   -304,   -2256,    -304,   -2264, DEFAULT,  DEFAULT                          },
    { doom,             1,   7,    16, BoxOfShells,               -272,   -2256,    -272,   -2264, DEFAULT,  DEFAULT                          },
    { doom,             1,   7,    17, BoxOfBullets,              -240,   -2256,    -240,   -2264, DEFAULT,  DEFAULT                          },
    { doom,             1,   7,    18, Chaingun,                  -208,   -2256,    -208,   -2264, DEFAULT,  DEFAULT                          },
    { doom,             1,   7,    19, BoxOfBullets,              -176,   -2256,    -176,   -2264, DEFAULT,  DEFAULT                          },

    { doom,             2,   2,     2, Player1Start,               608,    4576,     608,    4574, DEFAULT,  DEFAULT                          },
    { doom,             2,   2,     3, Player2Start,                96,    4576,      96,    4574, DEFAULT,  DEFAULT                          },

    { doom,             2,   3,    37, Backpack,                   864,    -224,     864,    -216, DEFAULT,  DEFAULT                          },

    { doom,             2,   5,    79, Chainsaw,                 -1056,     160,   -1052,     160, DEFAULT,  DEFAULT                          },

    { doom,             2,   6,   290, Demon,                      992,    -128,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },
    { doom,             2,   6,   291, Demon,                     1056,    -128,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { doom,             2,   7,   195, HangingVictimTwitching,    3360,     320,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },
    { doom,             2,   7,   196, HangingVictimOneLegged,    3232,       0,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { doom,             3,   2,   186, HangingLeg,                 -80,    1216,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { doom,             3,   3,    19, Cell,                      -480,     864,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { doom,             4,   1,     0, Player1Start,               160,     352,     160,     350, DEFAULT,  DEFAULT                          },
    { doom,             4,   1,     1, Player2Start,               288,     352,     288,     350, DEFAULT,  DEFAULT                          },
    { doom,             4,   1,     2, Player3Start,               416,     224,     414,     224, DEFAULT,  DEFAULT                          },
    { doom,             4,   1,     3, Player4Start,                32,     224,      34,     224, DEFAULT,  DEFAULT                          },

    { doom,             4,   3,    69, Imp,                       -608,   -1696,    -640,   -1696, DEFAULT,  DEFAULT                          },
    { doom,             4,   3,   232, Rocket,                    1296,    1008,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { doom,             4,   4,   106, Spectre,                    320,    -160,     328,    -128, DEFAULT,  DEFAULT                          },
    { doom,             4,   4,   107, Spectre,                    -16,    -160,      -8,    -128, DEFAULT,  DEFAULT                          },
    { doom,             4,   4,   108, Cacodemon,                  320,     -96,     328,     -88, DEFAULT,  DEFAULT                          },
    { doom,             4,   4,   109, Cacodemon,                  -16,     -96,      -8,     -88, DEFAULT,  DEFAULT                          },

    { doom2,            1,   2,    69, Barrel,                    2064,    2528,    2064,    2544, DEFAULT,  DEFAULT                          },
    { doom2,            1,   2,    69, Barrel,                     464,     784,     464,     800, DEFAULT,  DEFAULT                          },
    { doom2,            1,   2,    84, Barrel,                    2592,    2640,    2592,    2648, DEFAULT,  DEFAULT                          },
    { doom2,            1,   2,    84, Barrel,                     992,     896,     992,     904, DEFAULT,  DEFAULT                          },
    { doom2,            1,   2,    85, ShotgunGuy,                2560,    2640,    2560,    2656, DEFAULT,  DEFAULT                          },
    { doom2,            1,   2,    85, ShotgunGuy,                 944,     896,     944,     912, DEFAULT,  DEFAULT                          },

    { doom2,            1,   4,   136, ShortTechnoFloorLamp,      -368,     880,    -360,     880, DEFAULT,  DEFAULT                          },
    { doom2,            1,   4,   137, ShortTechnoFloorLamp,      -368,    1072,    -360,    1080, DEFAULT,  DEFAULT                          },

    { doom2,            1,   5,   108, LostSoul,                  2848,    -400,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { doom2,            1,  11,   165, Spectre,                    752,     304,     760,     304, DEFAULT,  DEFAULT                          },
    { doom2,            1,  11,   236, CellPack,                  1952,     272,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { doom2,            1,  15,   301, HeavyWeaponDude,           1488,   -1984,    1488,   -1984, DEFAULT,  MTF_NORMAL | MTF_HARD            },

    { doom2,            1,  17,   127, Spectre,                  -1888,   -2896,   -1896,   -2872, DEFAULT,  DEFAULT                          },
    { doom2,            1,  17,   128, Spectre,                  -1984,   -2896,   -1976,   -2872, DEFAULT,  DEFAULT                          },
    { doom2,            1,  17,   129, Spectre,                  -2064,   -2896,   -2056,   -2872, DEFAULT,  DEFAULT                          },
    { doom2,            1,  17,   130, Spectre,                  -1936,   -2848,   -1936,   -2800, DEFAULT,  DEFAULT                          },
    { doom2,            1,  17,   131, Spectre,                  -2016,   -2848,   -2016,   -2800, DEFAULT,  DEFAULT                          },

    { doom2,            1,  19,   112, TeleportDestination,       -912,    -880,    -912,    -880, DEFAULT,  MTF_EASY | MTF_NORMAL | MTF_HARD },

    { doom2,            1,  24,   238, Demon,                    -1200,    -352,   -1184,    -352, DEFAULT,  DEFAULT                          },
    { doom2,            1,  24,   239, Demon,                    -1136,    -352,   -1120,    -352, DEFAULT,  DEFAULT                          },

    { doom2,            1,  30,     1, MonstersSpawner,           2880,    1424,    2880,    1424,     270,  DEFAULT                          },

    { doom2,            1,  31,    60, Demon,                     1376,     800,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },
    { doom2,            1,  31,   265, WolfensteinSS,            -4576,    1952,   -4576,    1952, DEFAULT,  MTF_NORMAL | MTF_HARD            },

    { doom2,            1,  32,    67, PlayerDeathmatchStart,     1216,    4640,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { pack_nerve,       1,   9,   595, DeadPlayer,               -1280,    3904,   -1280,    3880, DEFAULT,  DEFAULT                          },

    { pack_plut,        1,   9,   304, ShortGreenFirestick,      -5216,   -1568,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { pack_plut,        1,  30,   206, MegaSphere,                -480,    1920,    -480,    1920, DEFAULT,  MTF_NETGAME                      },
    { pack_plut,        1,  30,   251, Berserk,                  -2272,    2368,   -2272,    2368, DEFAULT,  MTF_NETGAME                      },
    { pack_plut,        1,  30,   252, Berserk,                  -1632,    2464,   -1632,    2464, DEFAULT,  MTF_NETGAME                      },
    { pack_plut,        1,  30,   254, Berserk,                  -1248,    3104,   -1248,    3104, DEFAULT,  MTF_NETGAME                      },

    { pack_tnt,         1,   3,     7, TeleportDestination,       -416,     416,    -416,     416,     135,  DEFAULT                          },
    { pack_tnt,         1,   3,    34, TeleportDestination,        960,    1472,     960,    1472,     135,  DEFAULT                          },

    { pack_tnt,         1,  20,   511, CellPack,                  2910,   -1595,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { pack_tnt,         1,  24,   158, BurningBarrel,             -912,    1440,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { pack_tnt,         1,  25,   362, BoxOfRockets,              1656,    -512,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { pack_tnt,         1,  29,   405, Mancubus,                 -5536,     992,  REMOVE,  REMOVE, DEFAULT,  DEFAULT                          },

    { pack_tnt,         1,  31,   470, YellowKeycard,            -2727,   -1976,   -2727,   -1976, DEFAULT,  MTF_EASY | MTF_NORMAL | MTF_HARD },

    { -1,               0,   0,     0, 0,                      DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT,  DEFAULT                          }
};
