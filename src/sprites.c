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

#include "doomdef.h"
#include "i_system.h"
#include "m_array.h"
#include "m_misc.h"
#include "sprites.h"

char *original_sprnames[] =
{
    // Sprites 0 to 137
    "TROO", "SHTG", "PUNG", "PISG", "PISF", "SHTF", "SHT2", "CHGG", "CHGF", "MISG",
    "MISF", "SAWG", "PLSG", "PLSF", "BFGG", "BFGF", "BLUD", "PUFF", "BAL1", "BAL2",
    "PLSS", "PLSE", "MISL", "BFS1", "BFE1", "BFE2", "TFOG", "IFOG", "PLAY", "POSS",
    "SPOS", "VILE", "FIRE", "FATB", "FBXP", "SKEL", "MANF", "FATT", "CPOS", "SARG",
    "HEAD", "BAL7", "BOSS", "BOS2", "SKUL", "SPID", "BSPI", "APLS", "APBX", "CYBR",
    "PAIN", "SSWV", "KEEN", "BBRN", "BOSF", "ARM1", "ARM2", "BAR1", "BEXP", "FCAN",
    "BON1", "BON2", "BKEY", "RKEY", "YKEY", "BSKU", "RSKU", "YSKU", "STIM", "MEDI",
    "SOUL", "PINV", "PSTR", "PINS", "MEGA", "SUIT", "PMAP", "PVIS", "CLIP", "AMMO",
    "ROCK", "BROK", "CELL", "CELP", "SHEL", "SBOX", "BPAK", "BFUG", "MGUN", "CSAW",
    "LAUN", "PLAS", "SHOT", "SGN2", "COLU", "SMT2", "GOR1", "POL2", "POL5", "POL4",
    "POL3", "POL1", "POL6", "GOR2", "GOR3", "GOR4", "GOR5", "SMIT", "COL1", "COL2",
    "COL3", "COL4", "CAND", "CBRA", "COL6", "TRE1", "TRE2", "ELEC", "CEYE", "FSKU",
    "COL5", "TBLU", "TGRN", "TRED", "SMBT", "SMGT", "SMRT", "HDB1", "HDB2", "HDB3",
    "HDB4", "HDB5", "HDB6", "POB1", "POB2", "BRS1", "TLMP", "TLP2",

    // Sprites 138 to 143
    "TNT1", // phares 03/09/98: invisible sprite
    "DOGS", // killough 07/19/98: Marine's best friend :)
    "PLS1", // killough 07/19/98: first of two plasma fireballs in the beta
    "PLS2", // killough 07/19/98: second of two plasma fireballs in the beta
    "BON3", // killough 07/11/98: evil sceptre in the beta version
    "BON4", // killough 07/11/98: unholy bible in the beta version

    // Sprite 144
    "BLD2", // [BH] blood splats

    // [BH] Sprites 145 to 244 (100 extra sprite names to use in DeHackEd patches)
    "SP00", "SP01", "SP02", "SP03", "SP04", "SP05", "SP06", "SP07", "SP08", "SP09",
    "SP10", "SP11", "SP12", "SP13", "SP14", "SP15", "SP16", "SP17", "SP18", "SP19",
    "SP20", "SP21", "SP22", "SP23", "SP24", "SP25", "SP26", "SP27", "SP28", "SP29",
    "SP30", "SP31", "SP32", "SP33", "SP34", "SP35", "SP36", "SP37", "SP38", "SP39",
    "SP40", "SP41", "SP42", "SP43", "SP44", "SP45", "SP46", "SP47", "SP48", "SP49",
    "SP50", "SP51", "SP52", "SP53", "SP54", "SP55", "SP56", "SP57", "SP58", "SP59",
    "SP60", "SP61", "SP62", "SP63", "SP64", "SP65", "SP66", "SP67", "SP68", "SP69",
    "SP70", "SP71", "SP72", "SP73", "SP74", "SP75", "SP76", "SP77", "SP78", "SP79",
    "SP80", "SP81", "SP82", "SP83", "SP84", "SP85", "SP86", "SP87", "SP88", "SP89",
    "SP90", "SP91", "SP92", "SP93", "SP94", "SP95", "SP96", "SP97", "SP98", "SP99",

    // Sprite 245
    "RSMK", // [BH] rocket smoke

    NULL
};

// [BH] Override offsets in WAD file to provide better animation and positioning.
//  Many of these offsets are taken from the DOOM 2 Minor Sprite Fixing Project
//  by Revenant100, and then further refined by me.
const sproffset_t sproffsets[] =
{
    { "AMMOA0",     14,   14,  28,  16, true  },    //   12,   16
    { "APBXA0",     12,   12,  23,  23, true  },    //   12,   15
    { "APBXB0",     19,   17,  37,  33, true  },    //   19,   18
    { "APBXC0",     16,   15,  29,  29, true  },    //   17,   17
    { "APBXD0",     10,   11,  20,  20, true  },    //   10,   13
    { "APBXE0",      3,    4,   7,   7, true  },    //    4,    6
    { "APLSA0",      7,   10,  15,  15, true  },    //    7,   10
    { "APLSB0",      6,    9,  13,  13, true  },    //    6,    8
    { "ARM1A0",     15,   15,  31,  17, true  },    //   15,   17
    { "ARM1B0",     15,   15,  31,  17, true  },    //   15,   17
    { "ARM2A0",     15,   15,  31,  17, true  },    //   15,   17
    { "ARM2B0",     15,   15,  31,  17, true  },    //   15,   17
    { "BAL2A0",      8,    8,  16,  16, true  },    //    7,    8
    { "BAL2B0",      8,    7,  15,  15, true  },    //    7,    7
    { "BAL2C0",     23,   25,  45,  48, true  },    //   23,   24
    { "BAL2D0",     25,   23,  50,  42, true  },    //   25,   21
    { "BAL2E0",     26,   25,  53,  47, true  },    //   26,   23
    { "BAL7C0",     17,   17,  33,  33, true  },    //   20,    5
    { "BAL7D0",     21,   18,  41,  36, true  },    //   23,    6
    { "BAL7E0",     22,   21,  45,  40, true  },    //   23,    8
    { "BAR1A0",     11,   29,  23,  32, true  },    //   10,   28
    { "BAR1B0",     11,   29,  23,  32, true  },    //   10,   28
    { "BEXPA0",     11,   29,  23,  32, true  },    //   10,   28
    { "BEXPB0",     11,   28,  23,  31, true  },    //   10,   27
    { "BEXPC0",     20,   33,  40,  36, true  },    //   19,   32
    { "BEXPD0",     28,   47,  56,  50, true  },    //   27,   46
    { "BEXPE0",     30,   50,  60,  53, true  },    //   29,   49
    { "BFE1A0",     30,   25,  64,  50, true  },    //   30,   27
    { "BFE1B0",     41,   36,  81,  70, true  },    //   40,   36
    { "BFE1C0",     72,   59, 143, 114, true  },    //   71,   58
    { "BFE1D0",     67,   14, 135,  29, true  },    //   68,   14
    { "BFE2A0",     32,   25,  64,  50, true  },    //   32,   24
    { "BFE2B0",     19,   23,  42,  45, true  },    //   18,   22
    { "BFE2C0",     16,   17,  35,  35, true  },    //   19,   17
    { "BFE2D0",      6,    5,  13,   9, true  },    //    5,    4
    { "BFGFA0",   -119,  -98,  82,  40, true  },    // -125,  -98
    { "BFGFB0",    -91,  -77, 139,  67, true  },    //  -97,  -77
    { "BFGGA0",    -75, -116, 170,  84, true  },    //  -81, -116
    { "BFGGB0",    -75, -116, 170,  84, true  },    //  -81, -116
    { "BFGGC0",    -82, -117, 156,  83, true  },    //  -88, -117
    { "BFS1A0",     22,   23,  45,  45, true  },    //   24,   37
    { "BFS1B0",     22,   23,  45,  45, true  },    //   24,   37
    { "BFUGA0",     31,   34,  61,  36, true  },    //   31,   38
    { "BKEYA0",      7,   14,  14,  16, true  },    //    7,   19
    { "BKEYB0",      7,   14,  14,  16, true  },    //    7,   19
    { "BLUDC0",      6,    6,  12,  11, true  },    //    7,    6
    { "BON1A0",      7,   16,  14,  18, true  },    //    7,   14
    { "BON1B0",      7,   16,  14,  18, true  },    //    7,   14
    { "BON1C0",      7,   16,  14,  18, true  },    //    7,   14
    { "BON1D0",      7,   16,  14,  18, true  },    //    7,   14
    { "BON2A0",      8,   13,  16,  15, true  },    //    7,   13
    { "BON2B0",      8,   13,  16,  15, true  },    //    7,   13
    { "BON2C0",      8,   13,  16,  15, true  },    //    7,   13
    { "BON2D0",      8,   13,  16,  15, true  },    //    7,   13
    { "BOS2E1",     39,   66,  65,  70, true  },    //   30,   65
    { "BOS2E2",     13,   68,  34,  72, true  },    //   12,   67
    { "BOS2E5",     19,   69,  64,  71, true  },    //   28,   69
    { "BOS2E6",     19,   73,  49,  76, true  },    //   22,   73
    { "BOS2E7",     21,   73,  43,  77, true  },    //   20,   73
    { "BOS2E8",     34,   74,  62,  79, true  },    //   30,   74
    { "BOS2F1",     43,   66,  69,  70, true  },    //   34,   65
    { "BOS2F2",     38,   66,  60,  70, true  },    //   29,   65
    { "BOS2F5",     22,   62,  59,  64, true  },    //   28,   62
    { "BOS2F6",     17,   63,  61,  66, true  },    //   29,   63
    { "BOS2F7",     22,   64,  44,  68, true  },    //   21,   64
    { "BOS2F8",     22,   65,  54,  70, true  },    //   26,   65
    { "BOS2G1",     21,   60,  53,  64, true  },    //   14,   59
    { "BOS2G2",     22,   60,  61,  64, true  },    //   29,   59
    { "BOS2G3",     24,   58,  49,  62, true  },    //   26,   58
    { "BOS2G4",     24,   56,  49,  59, true  },    //   25,   56
    { "BOS2G5",     37,   55,  61,  57, true  },    //   30,   55
    { "BOS2G6",     32,   55,  52,  58, true  },    //   24,   55
    { "BOS2G7",     19,   57,  46,  61, true  },    //   22,   57
    { "BOS2G8",     22,   55,  56,  60, true  },    //   28,   56
    { "BOS2H3",     21,   67,  47,  71, true  },    //   25,   67
    { "BOS2H4",     24,   64,  49,  67, true  },    //   26,   64
    { "BOS2H5",     22,   62,  47,  65, true  },    //   23,   63
    { "BOS2H6",     19,   63,  40,  66, true  },    //   15,   62
    { "BOS2H7",     21,   66,  44,  70, true  },    //   17,   66
    { "BOS2H8",     19,   67,  44,  72, true  },    //   18,   68
    { "BOS2I0",     21,   69,  48,  73, true  },    //   20,   69
    { "BOS2J0",     27,   60,  55,  64, true  },    //   26,   60
    { "BOS2K0",     27,   50,  56,  54, true  },    //   26,   50
    { "BOS2L0",     30,   32,  59,  36, true  },    //   29,   32
    { "BOS2M0",     31,   26,  60,  30, true  },    //   30,   26
    { "BOS2N0",     31,   26,  60,  31, true  },    //   30,   26
    { "BOS2O0",     31,   26,  60,  31, true  },    //   30,   26
    { "BOSFA0",     17,   35,  33,  33, true  },    //   15,   35
    { "BOSFB0",     16,   37,  37,  36, true  },    //   16,   35
    { "BOSFD0",     18,   37,  35,  36, true  },    //   17,   37
    { "BOSSE1",     39,   66,  65,  70, true  },    //   30,   65
    { "BOSSE2",     13,   68,  34,  72, true  },    //   12,   67
    { "BOSSE5",     19,   69,  64,  71, true  },    //   26,   69
    { "BOSSE6",     19,   73,  49,  76, true  },    //   22,   73
    { "BOSSE7",     21,   73,  43,  77, true  },    //   20,   73
    { "BOSSE8",     34,   74,  62,  79, true  },    //   30,   74
    { "BOSSF1",     43,   66,  69,  70, true  },    //   34,   65
    { "BOSSF2",     38,   66,  60,  70, true  },    //   29,   65
    { "BOSSF5",     22,   62,  59,  64, true  },    //   29,   62
    { "BOSSF6",     17,   63,  61,  66, true  },    //   29,   63
    { "BOSSF7",     22,   64,  44,  68, true  },    //   21,   64
    { "BOSSF8",     22,   65,  54,  70, true  },    //   26,   65
    { "BOSSG1",     21,   60,  53,  64, true  },    //   14,   59
    { "BOSSG2",     20,   59,  61,  64, true  },    //   29,   59
    { "BOSSG3",     24,   58,  49,  62, true  },    //   26,   58
    { "BOSSG4",     24,   56,  49,  59, true  },    //   25,   56
    { "BOSSG5",     37,   55,  61,  57, true  },    //   30,   55
    { "BOSSG6",     32,   55,  52,  58, true  },    //   24,   55
    { "BOSSG7",     19,   57,  46,  61, true  },    //   22,   57
    { "BOSSG8",     22,   55,  56,  60, true  },    //   28,   56
    { "BOSSH3",     21,   67,  47,  71, true  },    //   25,   67
    { "BOSSH4",     24,   64,  49,  67, true  },    //   26,   64
    { "BOSSH5",     22,   62,  47,  65, true  },    //   23,   63
    { "BOSSH6",     15,   62,  40,  66, true  },    //   19,   63
    { "BOSSH7",     21,   66,  44,  70, true  },    //   17,   66
    { "BOSSH8",     19,   67,  44,  72, true  },    //   18,   68
    { "BOSSI0",     21,   69,  48,  73, true  },    //   20,   69
    { "BOSSJ0",     27,   60,  55,  64, true  },    //   26,   60
    { "BOSSK0",     27,   50,  56,  54, true  },    //   26,   50
    { "BOSSL0",     30,   32,  59,  36, true  },    //   29,   32
    { "BOSSM0",     31,   26,  60,  30, true  },    //   30,   26
    { "BOSSN0",     31,   26,  60,  31, true  },    //   30,   26
    { "BOSSO0",     31,   26,  60,  31, true  },    //   30,   26
    { "BPAKA0",     11,   27,  22,  29, true  },    //    8,   25
    { "BROKA0",     27,   19,  54,  21, true  },    //   27,   20
    { "BRS1A0",     14,    6,  29,   8, true  },    //   13,    3
    { "BSKUA0",      7,   14,  13,  16, true  },    //    7,   18
    { "BSKUB0",      7,   14,  13,  16, true  },    //    7,   18
    { "BSPIA2A8",   58,   51, 115,  56, true  },    //   59,   51
    { "BSPIA4A6",   57,   51, 109,  57, true  },    //   57,   52
    { "BSPIA5D5",   54,   48, 106,  52, true  },    //   53,   47
    { "BSPIB2B8",   67,   50, 129,  55, true  },    //   65,   50
    { "BSPIB3B7",   55,   48, 108,  51, true  },    //   55,   46
    { "BSPIB4B6",   56,   49, 108,  50, true  },    //   57,   45
    { "BSPIC4C6",   58,   50, 107,  56, true  },    //   59,   51
    { "BSPIC5F5",   53,   48, 105,  52, true  },    //   52,   47
    { "BSPID2D8",   53,   50, 113,  56, true  },    //   51,   51
    { "BSPIE2E8",   52,   50, 114,  50, true  },    //   51,   45
    { "BSPIE4E6",   65,   49, 126,  55, true  },    //   65,   50
    { "BSPIF2F8",   53,   50, 114,  57, true  },    //   54,   52
    { "BSPIF3F7",   63,   48, 106,  53, true  },    //   62,   48
    { "BSPIF4F6",   59,   51, 116,  56, true  },    //   55,   51
    { "BSPIG1",     50,   49,  96,  53, true  },    //   50,   48
    { "BSPIG2G8",   62,   50, 118,  55, true  },    //   61,   50
    { "BSPIG3G7",   55,   50, 104,  55, true  },    //   56,   50
    { "BSPIG4G6",   61,   51, 116,  57, true  },    //   62,   52
    { "BSPIG5",     55,   49, 107,  54, true  },    //   56,   49
    { "BSPIH1",     50,   49,  96,  53, true  },    //   50,   48
    { "BSPIH2H8",   62,   50, 118,  55, true  },    //   61,   50
    { "BSPIH3H7",   55,   50, 104,  55, true  },    //   56,   50
    { "BSPIH4H6",   61,   51, 116,  57, true  },    //   62,   52
    { "BSPIH5",     55,   49, 107,  54, true  },    //   56,   49
    { "BSPIJ0",     47,   50,  98,  55, true  },    //   50,   50
    { "BSPIK0",     47,   53,  96,  58, true  },    //   50,   53
    { "BSPIL0",     44,   48,  90,  53, true  },    //   47,   48
    { "BSPIM0",     42,   42,  95,  45, true  },    //   45,   42
    { "BSPIN0",     42,   40,  94,  45, true  },    //   45,   40
    { "BSPIO0",     42,   28,  95,  33, true  },    //   45,   28
    { "BSPIP0",     42,   26, 101,  31, true  },    //   45,   26
    { "CANDA0",      8,   14,  16,  15, true  },    //    8,   15
    { "CBRAA0",     14,   58,  29,  61, true  },    //   15,   57
    { "CELLA0",      8,   10,  17,  12, true  },    //    8,   12
    { "CELPA0",     16,   19,  32,  21, true  },    //   15,   20
    { "CEYEA0",     22,   59,  48,  60, false },    //   21,   56
    { "CEYEB0",     22,   58,  47,  59, false },    //   21,   55
    { "CEYEC0",     22,   59,  46,  60, false },    //   21,   56
    { "CHGFA0",   -116,  -98,  86,  46, true  },    // -118,  -98
    { "CHGFB0",   -118,  -97,  85,  47, true  },    // -120,  -97
    { "CHGGA0",   -102, -117, 114,  83, true  },    // -104, -117
    { "CHGGB0",   -102, -119, 114,  81, true  },    // -104, -119
    { "CLIPA0",      4,    9,   9,  11, true  },    //    2,   11
    { "COL1A0",     17,   50,  35,  53, true  },    //   16,   48
    { "COL2A0",     17,   37,  35,  40, true  },    //   16,   35
    { "COL3A0",     17,   50,  35,  53, true  },    //   16,   48
    { "COL4A0",     17,   37,  35,  40, true  },    //   17,   35
    { "COL5A0",     17,   42,  35,  45, true  },    //   16,   40
    { "COL5B0",     17,   43,  35,  46, true  },    //   16,   41
    { "COL6A0",     17,   46,  35,  49, true  },    //   17,   44
    { "COLUA0",     11,   46,  23,  48, true  },    //    9,   43
    { "CPOSE2",     26,   53,  50,  58, true  },    //   25,   53
    { "CPOSE3",     31,   53,  53,  56, true  },    //   27,   53
    { "CPOSE6",     17,   51,  40,  55, true  },    //   18,   51
    { "CPOSE7",     22,   53,  52,  56, true  },    //   25,   53
    { "CPOSE8",     23,   53,  40,  57, true  },    //   18,   53
    { "CPOSF2",     31,   53,  55,  58, true  },    //   29,   53
    { "CPOSF3",     35,   53,  57,  56, true  },    //   31,   53
    { "CPOSF6",     16,   51,  41,  55, true  },    //   21,   51
    { "CPOSF7",     22,   53,  56,  56, true  },    //   25,   53
    { "CPOSF8",     23,   53,  44,  57, true  },    //   22,   53
    { "CPOSG7",     19,   54,  40,  57, true  },    //   20,   54
    { "CPOSH0",     21,   56,  43,  61, true  },    //   20,   56
    { "CPOSI0",     24,   59,  48,  64, true  },    //   23,   59
    { "CPOSJ0",     29,   54,  59,  59, true  },    //   28,   54
    { "CPOSK0",     30,   44,  65,  49, true  },    //   31,   44
    { "CPOSL0",     32,   32,  64,  37, true  },    //   31,   32
    { "CPOSM0",     33,   20,  65,  25, true  },    //   32,   20
    { "CPOSN0",     33,   16,  65,  21, true  },    //   32,   16
    { "CPOSP0",     28,   55,  58,  60, true  },    //   28,   56
    { "CPOSQ0",     34,   45,  64,  49, true  },    //   30,   45
    { "CPOSR0",     40,   36,  70,  41, true  },    //   32,   36
    { "CPOSS0",     40,   27,  70,  32, true  },    //   32,   27
    { "CPOST0",     40,   15,  72,  20, true  },    //   32,   15
    { "CSAWA0",     31,   22,  62,  24, true  },    //   31,   23
    { "CYBRE3",     56,  105, 105, 110, true  },    //   52,  105
    { "CYBRE4",     51,  104, 106, 109, true  },    //   50,  104
    { "CYBRE6",     40,  105,  82, 110, true  },    //   39,  105
    { "CYBRE7",     39,  105,  97, 110, true  },    //   46,  105
    { "CYBRF3",     72,  105, 121, 110, true  },    //   60,  105
    { "CYBRF4",     64,  104, 119, 109, true  },    //   59,  104
    { "CYBRF6",     40,  105,  93, 110, true  },    //   47,  105
    { "CYBRF7",     39,  105, 114, 110, true  },    //   54,  105
    { "CYBRF8",     55,  105, 130, 110, true  },    //   63,  105
    { "CYBRG1",     45,  105, 123, 110, true  },    //   61,  105
    { "CYBRG2",     38,  106,  91, 111, true  },    //   46,  106
    { "CYBRG4",     71,  105, 118, 110, true  },    //   58,  105
    { "CYBRG5",     62,  104, 102, 109, true  },    //   54,  104
    { "CYBRG6",     43,  105,  91, 110, true  },    //   46,  105
    { "CYBRG7",     40,  105,  75, 110, true  },    //   47,  105
    { "CYBRH0",     45,  106, 122, 111, true  },    //   60,  106
    { "CYBRI0",     43,  108, 111, 113, true  },    //   55,  108
    { "CYBRJ0",     45,  111, 100, 116, true  },    //   49,  111
    { "CYBRK0",     51,  112, 113, 117, true  },    //   56,  112
    { "CYBRL0",     55,  119, 125, 124, true  },    //   62,  119
    { "CYBRM0",     62,  126, 136, 131, true  },    //   67,  126
    { "CYBRN0",     65,  129, 141, 134, true  },    //   70,  129
    { "CYBRO0",     64,  129, 139, 134, true  },    //   69,  129
    { "CYBRP0",     55,   25, 120,  30, true  },    //   60,   25
    { "ELECA0",     19,  125,  38, 128, true  },    //   19,  123
    { "FATBA1",      9,    8,  19,  16, true  },    //    9,   11
    { "FATBA2A8",   18,    7,  34,  13, true  },    //   18,   10
    { "FATBA3A7",   22,    6,  49,  11, true  },    //   22,    9
    { "FATBA4A6",   18,    6,  36,  11, true  },    //   18,    9
    { "FATBA5",      9,    8,  19,  16, true  },    //    9,   11
    { "FATBB1",      9,    8,  20,  16, true  },    //    9,   11
    { "FATBB2B8",   18,    7,  36,  13, true  },    //   18,   10
    { "FATBB3B7",   22,    6,  49,  11, true  },    //   21,    9
    { "FATBB4B6",   18,    6,  34,  11, true  },    //   18,    9
    { "FATBB5",      9,    8,  20,  16, true  },    //    9,   11
    { "FATTA1",     40,   60,  73,  65, true  },    //   39,   60
    { "FATTA2A8",   51,   61,  80,  66, true  },    //   39,   61
    { "FATTA3A7",   41,   60,  79,  65, true  },    //   38,   60
    { "FATTA5",     35,   59,  66,  64, true  },    //   39,   59
    { "FATTB1",     43,   64,  88,  70, true  },    //   43,   65
    { "FATTB2B8",   43,   64,  83,  70, true  },    //   43,   65
    { "FATTB3B7",   41,   61,  67,  66, true  },    //   43,   61
    { "FATTB5",     45,   58,  82,  63, true  },    //   43,   58
    { "FATTC5",     37,   59,  77,  63, true  },    //   36,   58
    { "FATTD1",     33,   60,  73,  65, true  },    //   38,   60
    { "FATTE1",     45,   64,  88,  70, true  },    //   43,   65
    { "FATTE2E8",   43,   62,  81,  66, true  },    //   43,   61
    { "FATTE3E7",   41,   62,  68,  67, true  },    //   43,   62
    { "FATTF1",     39,   65,  82,  70, true  },    //   42,   65
    { "FATTF3F7",   41,   61,  69,  67, true  },    //   43,   61
    { "FATTF4F6",   34,   59,  72,  64, true  },    //   43,   59
    { "FATTG1",     44,   61,  87,  65, true  },    //   42,   60
    { "FATTG2G8",   54,   59,  98,  64, true  },    //   51,   59
    { "FATTG5",     41,   61,  82,  66, true  },    //   42,   61
    { "FATTH1",     43,   61,  83,  65, true  },    //   42,   60
    { "FATTH2H8",   53,   59, 101,  64, true  },    //   50,   59
    { "FATTH4H6",   32,   61,  75,  66, true  },    //   42,   61
    { "FATTH5",     43,   60,  86,  65, true  },    //   42,   60
    { "FATTI1",     44,   64,  84,  68, true  },    //   42,   63
    { "FATTI2I8",   54,   64,  95,  69, true  },    //   51,   64
    { "FATTJ1",     35,   59,  74,  67, true  },    //   32,   62
    { "FATTJ2",     36,   60,  88,  66, true  },    //   47,   62
    { "FATTJ3",     40,   58,  75,  64, true  },    //   39,   59
    { "FATTJ4",     30,   59,  61,  66, true  },    //   30,   61
    { "FATTJ5",     42,   59,  77,  64, true  },    //   32,   59
    { "FATTJ6",     31,   59,  61,  66, true  },    //   29,   61
    { "FATTJ7",     35,   58,  75,  64, true  },    //   37,   59
    { "FATTJ8",     52,   60,  88,  66, true  },    //   46,   62
    { "FATTK0",     51,   73, 103,  75, true  },    //   50,   73
    { "FATTL0",     51,   75, 101,  76, true  },    //   50,   75
    { "FATTM0",     46,   66,  95,  70, true  },    //   45,   66
    { "FATTN0",     38,   51,  85,  52, true  },    //   37,   53
    { "FATTO0",     37,   41,  84,  44, true  },    //   35,   41
    { "FATTP0",     38,   40,  86,  43, true  },    //   36,   40
    { "FATTQ0",     38,   39,  86,  43, true  },    //   36,   39
    { "FATTR0",     38,   39,  86,  43, true  },    //   36,   39
    { "FATTS0",     38,   38,  86,  42, true  },    //   36,   38
    { "FATTT0",     38,   36,  86,  41, true  },    //   36,   36
    { "FBXPA0",     17,   17,  33,  33, true  },    //   19,   32
    { "FBXPB0",     20,   17,  38,  33, true  },    //   19,   31
    { "FBXPC0",     22,   20,  45,  40, true  },    //   22,   35
    { "FCANA0",     12,   51,  37,  53, false },    //   19,   49
    { "FCANB0",     12,   51,  34,  53, false },    //   19,   49
    { "FCANC0",     12,   49,  36,  51, false },    //   19,   47
    { "GOR1A0",     11,   67,  30,  68, false },    //   17,   67
    { "GOR1B0",     12,   67,  30,  68, false },    //   18,   67
    { "GOR1C0",     11,   67,  30,  68, false },    //   17,   67
    { "GOR2A0",     20,   83,  41,  84, true  },    //   22,   83
    { "GOR3A0",     17,   83,  39,  79, true  },    //   19,   83
    { "GOR4A0",      8,   67,  18,  67, true  },    //    6,   67
    { "GOR5A0",      5,   51,  14,  53, true  },    //    6,   51
    { "HDB1A0",     11,   83,  22,  88, true  },    //   10,   83
    { "HDB2A0",     11,   83,  22,  85, true  },    //   10,   83
    { "HDB3A0",     11,   59,  22,  64, true  },    //   10,   59
    { "HDB4A0",     11,   59,  22,  64, true  },    //   10,   59
    { "HDB5A0",     11,   59,  22,  57, true  },    //   10,   59
    { "HDB6A0",     11,   59,  22,  61, true  },    //   10,   59
    { "HEADA3A7",   29,   66,  62,  67, true  },    //   27,   68
    { "HEADA4A6",   29,   68,  63,  67, true  },    //   32,   68
    { "HEADA5",     31,   67,  63,  65, true  },    //   28,   66
    { "HEADB1",     31,   66,  63,  65, true  },    //   31,   70
    { "HEADB2B8",   26,   66,  61,  66, true  },    //   29,   69
    { "HEADB3B7",   29,   66,  62,  67, true  },    //   30,   68
    { "HEADB4B6",   29,   66,  63,  65, true  },    //   32,   67
    { "HEADB5",     31,   67,  63,  65, true  },    //   32,   68
    { "HEADC1",     31,   67,  63,  69, true  },    //   31,   71
    { "HEADC2C8",   26,   68,  60,  69, true  },    //   29,   72
    { "HEADC3C7",   29,   66,  62,  69, true  },    //   30,   68
    { "HEADC4C6",   29,   67,  63,  68, true  },    //   32,   67
    { "HEADC5",     31,   68,  63,  67, true  },    //   32,   68
    { "HEADD1",     31,   68,  63,  71, true  },    //   31,   72
    { "HEADD2D8",   26,   69,  61,  71, true  },    //   29,   72
    { "HEADD3D7",   29,   70,  62,  75, true  },    //   30,   72
    { "HEADD4D6",   29,   69,  63,  71, true  },    //   32,   70
    { "HEADD5",     31,   68,  63,  70, true  },    //   32,   68
    { "HEADE2E8",   28,   65,  62,  65, true  },    //   30,   67
    { "HEADE3E7",   29,   67,  62,  68, true  },    //   29,   68
    { "HEADE4E6",   30,   69,  63,  67, true  },    //   31,   69
    { "HEADE5",     31,   69,  63,  66, true  },    //   30,   68
    { "HEADF2F8",   28,   64,  62,  65, true  },    //   31,   66
    { "HEADF3F7",   30,   67,  62,  68, true  },    //   31,   67
    { "HEADF4F6",   30,   69,  63,  67, true  },    //   31,   69
    { "HEADF5",     31,   69,  63,  66, true  },    //   30,   68
    { "HEADK0",     37,   63,  69,  66, true  },    //   35,   63
    { "HEADL0",     40,   47,  75,  49, true  },    //   37,   47
    { "IFOGA0",     19,   33,  40,  37, true  },    //   18,   33
    { "IFOGB0",     16,   27,  34,  30, true  },    //   16,   26
    { "IFOGC0",      8,   20,  17,  16, true  },    //    6,   15
    { "IFOGD0",      4,   13,   9,   8, true  },    //    2,   10
    { "IFOGE0",      1,   11,   3,   4, true  },    //    0,    7
    { "KEENA0",      7,   67,  14,  51, true  },    //    6,   67
    { "KEENB0",     10,   67,  19,  46, true  },    //    8,   67
    { "KEENC0",     11,   67,  22,  47, true  },    //    9,   67
    { "KEEND0",     13,   67,  22,  50, true  },    //   11,   67
    { "KEENE0",     14,   67,  26,  54, true  },    //   13,   67
    { "KEENF0",     14,   67,  27,  62, true  },    //   13,   67
    { "KEENG0",     15,   67,  28,  70, true  },    //   14,   67
    { "KEENH0",     18,   67,  34,  71, true  },    //   17,   67
    { "KEENI0",     18,   67,  34,  71, true  },    //   17,   67
    { "KEENJ0",     18,   67,  34,  72, true  },    //   17,   67
    { "KEENK0",     18,   67,  34,  72, true  },    //   17,   67
    { "KEENL0",     18,   67,  34,  72, true  },    //   17,   67
    { "KEENM0",      9,   67,  16,  43, true  },    //    8,   67
    { "LAUNA0",     31,   14,  62,  16, true  },    //   31,   18
    { "MANFA1",     17,   19,  34,  34, true  },    //   19,   17
    { "MANFA6A4",   25,   14,  54,  30, true  },    //   25,   19
    { "MANFA7A3",   34,   11,  67,  25, true  },    //   34,   16
    { "MANFA8A2",   33,   12,  56,  28, true  },    //   33,   17
    { "MANFB1",     17,   19,  34,  34, true  },    //   19,   17
    { "MANFB6B4",   18,   15,  46,  30, true  },    //   18,   19
    { "MANFB7B3",   30,   13,  63,  27, true  },    //   30,   17
    { "MANFB8B2",   27,   14,  51,  28, true  },    //   27,   17
    { "MEDIA0",     14,   17,  28,  19, true  },    //   13,   19
    { "MEGAA0",     12,   40,  25,  25, true  },    //   12,   32
    { "MEGAB0",     12,   40,  25,  25, true  },    //   12,   32
    { "MEGAC0",     12,   40,  25,  25, true  },    //   12,   32
    { "MEGAD0",     12,   40,  25,  25, true  },    //   12,   32
    { "MGUNA0",     27,   14,  54,  16, true  },    //   25,   18
    { "MISFA0",   -134, -106,  53,  31, true  },    // -136, -105
    { "MISFB0",   -123, -101,  73,  51, true  },    // -126, -101
    { "MISFC0",   -114,  -94,  88,  58, true  },    // -117,  -94
    { "MISFD0",   -109,  -81, 105,  79, true  },    // -112,  -81
    { "MISGA0",   -117, -121,  87,  79, true  },    // -119, -121
    { "MISGB0",   -109, -125, 102,  75, true  },    // -112, -125
    { "MISLA1",      7,    8,  15,  14, true  },    //    7,   13
    { "MISLA5",      7,    8,  15,  14, true  },    //    7,   13
    { "MISLA6A4",   12,    8,  26,  14, true  },    //   12,   13
    { "MISLA7A3",   25,    8,  49,  14, true  },    //   25,   13
    { "MISLA8A2",   16,    8,  32,  14, true  },    //   16,   13
    { "MISLC0",     44,   36,  88,  72, true  },    //   42,   34
    { "MISLD0",     52,   46, 103,  86, true  },    //   50,   43
    { "PAINA2A8",   30,   58,  67,  55, true  },    //   34,   58
    { "PAINA3A7",   37,   58,  70,  56, true  },    //   35,   60
    { "PAINA4A6",   32,   59,  66,  55, true  },    //   35,   59
    { "PAINA5",     36,   59,  72,  53, true  },    //   37,   58
    { "PAINB2B8",   34,   58,  76,  55, true  },    //   38,   58
    { "PAINB3B7",   37,   58,  70,  56, true  },    //   35,   60
    { "PAINB4B6",   32,   59,  66,  55, true  },    //   35,   59
    { "PAINB5",     37,   59,  77,  53, true  },    //   38,   58
    { "PAINC2C8",   35,   58,  78,  55, true  },    //   39,   58
    { "PAINC3C7",   37,   58,  70,  56, true  },    //   35,   60
    { "PAINC4C6",   32,   59,  66,  55, true  },    //   35,   59
    { "PAINC5",     38,   59,  77,  53, true  },    //   36,   58
    { "PAIND1",     44,   61,  89,  55, true  },    //   44,   62
    { "PAIND2D8",   41,   59,  80,  56, true  },    //   40,   61
    { "PAIND3D7",   38,   59,  71,  57, true  },    //   35,   61
    { "PAIND4D6",   36,   59,  70,  56, true  },    //   35,   59
    { "PAIND5",     36,   59,  72,  53, true  },    //   37,   58
    { "PAINE2E8",   42,   60,  81,  57, true  },    //   41,   62
    { "PAINE3E7",   37,   59,  70,  61, true  },    //   34,   61
    { "PAINE4E6",   36,   60,  70,  59, true  },    //   35,   60
    { "PAINE5",     35,   60,  71,  56, true  },    //   36,   59
    { "PAINF2F8",   47,   61,  82,  60, true  },    //   38,   62
    { "PAINF3F7",   33,   61,  66,  64, true  },    //   30,   63
    { "PAINF4F6",   41,   61,  75,  61, true  },    //   36,   69
    { "PAINF5",     36,   62,  73,  59, true  },    //   37,   68
    { "PAING1",     42,   61,  82,  57, true  },    //   38,   60
    { "PAING2G8",   31,   59,  69,  55, true  },    //   35,   60
    { "PAING3G7",   36,   59,  71,  57, true  },    //   34,   62
    { "PAING4G6",   41,   59,  77,  56, true  },    //   36,   59
    { "PAING5",     45,   58,  95,  53, true  },    //   47,   57
    { "PAINH0",     42,   61,  82,  57, true  },    //   41,   57
    { "PAINI0",     38,   58,  76,  56, true  },    //   36,   54
    { "PAINJ0",     41,   57,  84,  56, true  },    //   39,   54
    { "PAINK0",     47,   66,  97,  70, true  },    //   46,   71
    { "PAINL0",     43,   66,  87,  72, true  },    //   43,   71
    { "PAINM0",     52,   76, 102,  86, true  },    //   49,   88
    { "PINSA0",     12,   40,  25,  25, true  },    //   11,   39
    { "PINSB0",     12,   40,  25,  25, true  },    //   11,   39
    { "PINSC0",     12,   40,  25,  25, true  },    //   11,   39
    { "PINSD0",     12,   40,  25,  25, true  },    //   11,   39
    { "PINVA0",     12,   40,  25,  25, true  },    //   11,   23
    { "PINVB0",     12,   40,  25,  25, true  },    //   11,   23
    { "PINVC0",     12,   40,  25,  25, true  },    //   11,   23
    { "PINVD0",     12,   40,  25,  25, true  },    //   11,   23
    { "PLASA0",     27,   14,  54,  16, true  },    //   27,   19
    { "PLSEA0",     11,   12,  23,  23, true  },    //   12,   11
    { "PLSEB0",     18,   20,  39,  41, true  },    //   19,   18
    { "PLSEC0",     19,   18,  37,  35, true  },    //   17,   18
    { "PLSED0",     15,   16,  29,  29, true  },    //   13,   13
    { "PLSEE0",      3,    3,   7,   7, true  },    //    0,    2
    { "PLSFA0",   -119,  -93,  83,  75, true  },    // -123,  -93
    { "PLSFB0",   -118,  -95,  85,  73, true  },    // -122,  -95
    { "PLSGA0",   -119, -107,  83,  61, true  },    // -123, -107
    { "PLSGB0",    -60,  -57, 104, 111, true  },    //  -64,  -57
    { "PMAPA0",     14,   25,  28,  27, true  },    //   13,   23
    { "PMAPB0",     14,   25,  28,  27, true  },    //   13,   23
    { "PMAPC0",     14,   25,  28,  27, true  },    //   13,   23
    { "PMAPD0",     14,   25,  28,  27, true  },    //   13,   23
    { "POB1A0",     20,    5,  41,   7, true  },    //   16,    2
    { "POB2A0",     16,    1,  32,   3, true  },    //   14,   -2
    { "POL1A0",     23,   64,  40,  66, true  },    //   22,   62
    { "POL2A0",     19,   62,  41,  67, true  },    //   19,   62
    { "POL3A0",     19,   41,  41,  43, true  },    //   19,   38
    { "POL3B0",     19,   41,  41,  43, true  },    //   19,   38
    { "POL4A0",     19,   54,  41,  56, true  },    //   19,   51
    { "POL5A0",     27,    6,  55,  10, true  },    //   27,    5
    { "POL6A0",     17,   65,  35,  66, false },    //   17,   62
    { "POL6B0",     19,   65,  38,  66, false },    //   19,   62
    { "POSSC4C6",   22,   51,  41,  55, true  },    //   20,   51
    { "POSSD4D6",   24,   52,  41,  54, true  },    //   22,   52
    { "POSSE1",     13,   50,  26,  55, true  },    //   12,   50
    { "POSSE2E8",   25,   50,  43,  54, true  },    //   21,   50
    { "POSSE3E7",   28,   50,  51,  53, true  },    //   26,   50
    { "POSSE5",     11,   47,  26,  51, true  },    //   12,   46
    { "POSSF1",     14,   50,  27,  55, true  },    //   13,   50
    { "POSSF2F8",   27,   50,  45,  54, true  },    //   23,   50
    { "POSSF3F7",   29,   50,  52,  53, true  },    //   27,   50
    { "POSSF5",     10,   47,  24,  51, true  },    //   11,   46
    { "POSSG2G8",   19,   53,  36,  55, true  },    //   16,   53
    { "POSSG3G7",   22,   53,  43,  54, true  },    //   21,   53
    { "POSSG4G6",   21,   50,  43,  53, true  },    //   20,   50
    { "POSSG5",     16,   49,  34,  53, true  },    //   17,   49
    { "POSSO0",     21,   58,  48,  61, true  },    //   25,   58
    { "POSSQ0",     23,   47,  55,  51, true  },    //   27,   47
    { "PSTRA0",     14,   17,  28,  19, true  },    //   13,   19
    { "PVISA0",     14,   11,  28,  13, true  },    //   13,    9
    { "PVISB0",     14,   11,  28,  13, true  },    //   13,    9
    { "RKEYA0",      7,   14,  14,  16, true  },    //    8,   19
    { "RKEYB0",      7,   14,  14,  16, true  },    //    8,   19
    { "ROCKA0",      6,   25,  12,  27, true  },    //    6,   27
    { "RSKUA0",      7,   14,  13,  16, true  },    //    7,   18
    { "RSKUB0",      7,   14,  13,  16, true  },    //    7,   18
    { "SARGA3A7",   28,   49,  60,  53, true  },    //   28,   48
    { "SARGB3B7",   28,   52,  58,  56, true  },    //   28,   51
    { "SARGB5",     20,   51,  39,  53, true  },    //   20,   48
    { "SARGC3C7",   29,   49,  60,  53, true  },    //   29,   48
    { "SARGD3D7",   29,   54,  59,  56, true  },    //   29,   53
    { "SARGD5",     20,   51,  39,  53, true  },    //   20,   48
    { "SARGE3",     31,   48,  60,  51, true  },    //   23,   48
    { "SARGE6",     23,   47,  49,  50, true  },    //   25,   47
    { "SARGE7",     30,   46,  58,  50, true  },    //   33,   46
    { "SARGF3",     38,   48,  67,  51, true  },    //   30,   48
    { "SARGF6",     23,   47,  52,  50, true  },    //   25,   47
    { "SARGF7",     30,   47,  63,  51, true  },    //   34,   47
    { "SARGG3",     41,   50,  70,  53, true  },    //   33,   50
    { "SARGG6",     24,   47,  54,  50, true  },    //   26,   47
    { "SARGG7",     30,   48,  66,  52, true  },    //   34,   48
    { "SARGH2",     25,   48,  50,  53, true  },    //   27,   48
    { "SARGH3",     30,   48,  59,  51, true  },    //   29,   46
    { "SARGH4",     24,   45,  52,  49, true  },    //   23,   44
    { "SARGH6",     21,   45,  44,  48, true  },    //   20,   45
    { "SARGH7",     29,   46,  57,  50, true  },    //   25,   46
    { "SARGH8",     29,   47,  54,  52, true  },    //   30,   47
    { "SARGK0",     21,   55,  52,  53, true  },    //   21,   57
    { "SARGL0",     30,   52,  62,  57, true  },    //   29,   55
    { "SARGM0",     31,   36,  64,  46, true  },    //   33,   41
    { "SARGN0",     31,   22,  64,  32, true  },    //   33,   27
    { "SAWGA0",    -81, -113, 140,  55, true  },    //  -75, -113
    { "SAWGB0",    -81, -113, 140,  55, true  },    //  -75, -113
    { "SAWGC0",    -72,  -79, 153,  89, true  },    //  -68,  -79
    { "SAWGD0",    -73,  -79, 154,  89, true  },    //  -67,  -79
    { "SBOXA0",     16,   10,  32,  12, true  },    //   16,   12
    { "SGN2A0",     27,   12,  54,  14, true  },    //   27,   15
    { "SHELA0",      7,    5,  15,   7, true  },    //    5,    7
    { "SHOTA0",     31,   10,  63,  12, true  },    //   31,   17
    { "SHT2A0",   -130, -113,  59,  55, true  },    // -134, -113
    { "SHT2B0",    -97,  -65,  83, 103, true  },    // -100,  -65
    { "SHT2C0",    -23,  -38, 121, 130, true  },    //  -25,  -38
    { "SHT2D0",   -117,  -88,  81,  80, true  },    // -118,  -88
    { "SHT2E0",      4, -105, 201,  63, true  },    //    0, -105
    { "SHT2F0",   -101, -117,  88,  51, true  },    // -105, -117
    { "SHT2G0",   -116,  -88,  81,  80, true  },    // -118,  -88
    { "SHT2H0",   -120,  -83,  77,  85, true  },    // -123,  -83
    { "SHT2I0",   -133,  -99,  55,  37, true  },    // -137,  -99
    { "SHT2J0",   -127,  -90,  65,  46, true  },    // -131,  -90
    { "SHTFA0",   -138,  -95,  44,  31, true  },    // -141,  -95
    { "SHTFB0",   -133,  -86,  54,  44, true  },    // -136,  -86
    { "SHTGA0",   -118, -108,  79,  60, true  },    // -121, -108
    { "SHTGB0",    -40,  -47, 119, 121, true  },    //  -43,  -47
    { "SHTGC0",    -27,  -17,  87, 151, true  },    //  -30,  -17
    { "SHTGD0",    -26,  -37, 113, 131, true  },    //  -29,  -37
    { "SKELA5D5",   19,   71,  37,  75, true  },    //   13,   71
    { "SKELB1E1",   26,   81,  56,  85, true  },    //   27,   81
    { "SKELB5E5",   25,   74,  52,  78, true  },    //   19,   74
    { "SKELC1F1",   27,   83,  68,  87, true  },    //   30,   83
    { "SKELC5F5",   34,   81,  63,  85, true  },    //   28,   81
    { "SKELG1",     29,   68,  56,  72, true  },    //   25,   67
    { "SKELG2",      5,   69,  52,  73, true  },    //   25,   69
    { "SKELG3",     16,   69,  69,  73, true  },    //   33,   68
    { "SKELG4",     12,   70,  77,  76, true  },    //   37,   72
    { "SKELG5",     20,   71,  62,  76, true  },    //   31,   72
    { "SKELG6",     41,   68,  53,  74, true  },    //   26,   70
    { "SKELG7",     54,   67,  71,  71, true  },    //   36,   67
    { "SKELG8",     60,   68,  75,  73, true  },    //   37,   68
    { "SKELH1",     39,   78,  61,  83, true  },    //   30,   78
    { "SKELH2",     41,   78,  72,  82, true  },    //   35,   78
    { "SKELH3",     34,   77,  67,  81, true  },    //   30,   77
    { "SKELH4",     22,   76,  43,  81, true  },    //   20,   76
    { "SKELH5",     18,   76,  58,  80, true  },    //   24,   76
    { "SKELH6",     25,   73,  74,  77, true  },    //   37,   72
    { "SKELH7",     29,   74,  69,  78, true  },    //   36,   74
    { "SKELH8",     19,   76,  43,  81, true  },    //   23,   76
    { "SKELI1",     17,   61,  47,  66, true  },    //   20,   61
    { "SKELI2",     29,   63,  61,  67, true  },    //   31,   62
    { "SKELI3",     45,   61,  74,  65, true  },    //   36,   60
    { "SKELI4",     42,   60,  59,  64, true  },    //   29,   60
    { "SKELI5",     23,   61,  40,  64, true  },    //   14,   60
    { "SKELI6",     24,   59,  61,  63, true  },    //   28,   59
    { "SKELI7",     32,   59,  80,  63, true  },    //   39,   59
    { "SKELI8",     17,   61,  62,  66, true  },    //   29,   61
    { "SKELJ1",     23,   67,  49,  71, true  },    //   25,   67
    { "SKELK1",     25,   76,  49,  79, true  },    //   27,   76
    { "SKELK2",     16,   75,  54,  78, true  },    //   23,   75
    { "SKELK3",     17,   74,  46,  79, true  },    //   23,   74
    { "SKELK4",     14,   74,  49,  77, true  },    //   20,   75
    { "SKELK5",     23,   76,  49,  78, true  },    //   27,   75
    { "SKELK6",     31,   75,  49,  77, true  },    //   27,   75
    { "SKELK7",     31,   73,  46,  79, true  },    //   18,   75
    { "SKELK8",     30,   75,  44,  80, true  },    //   17,   75
    { "SKELL2",     12,   67,  44,  71, true  },    //   18,   67
    { "SKELL3",     16,   67,  59,  71, true  },    //   24,   67
    { "SKELL4",     30,   68,  73,  73, true  },    //   34,   68
    { "SKELL5",     29,   69,  60,  74, true  },    //   32,   69
    { "SKELL6",     26,   68,  46,  73, true  },    //   21,   68
    { "SKELL7",     39,   68,  57,  72, true  },    //   25,   68
    { "SKELL8",     38,   69,  69,  73, true  },    //   33,   69
    { "SKELM0",     30,   74,  56,  78, true  },    //   27,   74
    { "SKELN0",     30,   68,  74,  70, true  },    //   38,   65
    { "SKELO0",     27,   51,  65,  55, true  },    //   28,   51
    { "SKELP0",     23,   33,  62,  38, true  },    //   28,   33
    { "SKELQ0",     29,   15,  65,  22, true  },    //   40,   19
    { "SKULA1",     22,   50,  44,  47, true  },    //   20,   50
    { "SKULA5",     22,   49,  44,  46, true  },    //   21,   48
    { "SKULA6A4",   13,   56,  35,  52, true  },    //   13,   53
    { "SKULA7A3",   14,   57,  31,  54, true  },    //   14,   54
    { "SKULA8A2",   15,   50,  32,  47, true  },    //   15,   47
    { "SKULB1",     22,   49,  44,  46, true  },    //   20,   49
    { "SKULB5",     22,   49,  44,  46, true  },    //   21,   48
    { "SKULB6B4",   13,   56,  35,  52, true  },    //   13,   53
    { "SKULB7B3",   14,   57,  31,  54, true  },    //   14,   54
    { "SKULB8B2",   15,   56,  32,  53, true  },    //   15,   53
    { "SKULC1",     22,   47,  44,  44, true  },    //   23,   47
    { "SKULC5",     22,   31,  44,  26, true  },    //   20,   30
    { "SKULC7C3",   33,   37,  67,  33, true  },    //   33,   36
    { "SKULC8C2",   32,   38,  60,  36, true  },    //   32,   37
    { "SKULD1",     22,   46,  44,  44, true  },    //   23,   46
    { "SKULD5",     22,   32,  44,  26, true  },    //   20,   31
    { "SKULD7D3",   33,   37,  67,  33, true  },    //   33,   36
    { "SKULD8D2",   25,   38,  53,  36, true  },    //   25,   37
    { "SKULE1",     15,   53,  34,  51, true  },    //   14,   53
    { "SKULE6E4",   11,   54,  30,  52, true  },    //   11,   53
    { "SKULE7E3",   15,   55,  33,  54, true  },    //   15,   54
    { "SKULE8E2",   15,   55,  36,  54, true  },    //   15,   54
    { "SKULF0",     15,   53,  34,  51, true  },    //   17,   53
    { "SKULG0",     17,   53,  36,  53, true  },    //   15,   53
    { "SKULH0",     23,   48,  45,  48, true  },    //   24,   48
    { "SKULI0",     35,   52,  68,  60, true  },    //   35,   58
    { "SKULJ0",     44,   59,  88,  72, true  },    //   45,   75
    { "SKULK0",     51,   67,  103, 90, true  },    //   49,   85
    { "SMBTA0",      8,   72,  17,  73, false },    //   10,   72
    { "SMBTB0",      8,   67,  17,  68, false },    //   10,   67
    { "SMBTC0",      8,   67,  16,  68, false },    //   10,   67
    { "SMBTD0",      8,   73,  17,  74, false },    //   10,   73
    { "SMGTA0",      8,   72,  17,  73, false },    //   10,   72
    { "SMGTB0",      8,   67,  17,  68, false },    //   10,   67
    { "SMGTC0",      8,   67,  16,  68, false },    //   10,   67
    { "SMGTD0",      8,   73,  17,  74, false },    //   10,   73
    { "SMRTA0",      8,   72,  17,  73, false },    //   10,   72
    { "SMRTB0",      8,   67,  17,  68, false },    //   10,   67
    { "SMRTC0",      8,   67,  16,  68, false },    //   10,   67
    { "SMRTD0",      8,   73,  17,  74, false },    //   10,   73
    { "SOULA0",     12,   40,  25,  25, true  },    //   14,   39
    { "SOULB0",     12,   40,  25,  25, true  },    //   14,   39
    { "SOULC0",     12,   40,  25,  25, true  },    //   14,   39
    { "SOULD0",     12,   40,  25,  25, true  },    //   14,   39
    { "SPIDA1D1",  103,  105, 195, 110, true  },    //  107,  105
    { "SPIDA2A8",  116,  107, 230, 112, true  },    //  110,  107
    { "SPIDA3A7",  106,  106, 208, 111, true  },    //   99,  106
    { "SPIDA4A6",  113,  107, 218, 116, true  },    //  111,  111
    { "SPIDB1E1",  125,  104, 215, 109, true  },    //  130,  104
    { "SPIDB2B8",  134,  106, 257, 111, true  },    //  130,  106
    { "SPIDB3B7",  108,  102, 215, 104, true  },    //  101,   99
    { "SPIDB4B6",  112,  105, 215, 104, true  },    //  110,   99
    { "SPIDC1F1",  102,  103, 192, 108, true  },    //  108,  103
    { "SPIDC3C7",  107,  105, 209, 110, true  },    //  103,  105
    { "SPIDC4C6",  115,  107, 214, 115, true  },    //  114,  110
    { "SPIDD2D8",  105,  104, 227, 112, true  },    //  113,  107
    { "SPIDD3D7",  104,  103, 195, 108, true  },    //   99,  103
    { "SPIDD4D6",  111,  104, 226, 111, true  },    //  107,  106
    { "SPIDE2E8",  103,  103, 227,  99, true  },    //  113,   94
    { "SPIDE3E7",  127,  101, 218, 106, true  },    //  122,  101
    { "SPIDE4E6",  131,  105, 252, 111, true  },    //  128,  106
    { "SPIDF2F8",  105,  104, 228, 114, true  },    //  114,  109
    { "SPIDF3F7",  126,  101, 214, 107, true  },    //  122,  102
    { "SPIDF4F6",  117,  105, 231, 111, true  },    //  114,  106
    { "SPIDG1",    101,  102, 194, 106, true  },    //   95,  101
    { "SPIDG3G7",  111,  108, 208, 113, true  },    //  113,  108
    { "SPIDG4G6",  123,  109, 234, 117, true  },    //  120,  112
    { "SPIDG5",    109,  104, 213, 110, true  },    //  106,  105
    { "SPIDH1",    101,  102, 194, 106, true  },    //   95,  101
    { "SPIDH2H8",  123,  107, 235, 113, true  },    //  123,  108
    { "SPIDH3H7",  111,  108, 208, 113, true  },    //  113,  108
    { "SPIDH4H6",  123,  109, 234, 117, true  },    //  120,  112
    { "SPIDH5",    109,  104, 213, 110, true  },    //  106,  105
    { "SPIDI1",     95,  101, 197, 107, true  },    //  102,  102
    { "SPIDI3",    118,  100, 221, 105, true  },    //  122,  100
    { "SPIDI5",    105,  100, 208, 105, true  },    //   95,  100
    { "SPIDI6",    117,  105, 217, 112, true  },    //  106,  107
    { "SPIDI7",     96,  102, 172, 107, true  },    //   88,  102
    { "SPIDI8",    120,  103, 228, 110, true  },    //  115,  105
    { "SPOSE1",     13,   50,  26,  55, true  },    //   12,   50
    { "SPOSE2E8",   21,   50,  39,  54, true  },    //   17,   50
    { "SPOSE3E7",   19,   50,  44,  53, true  },    //   19,   49
    { "SPOSE5",     11,   47,  26,  51, true  },    //   12,   46
    { "SPOSF1",     14,   50,  27,  55, true  },    //   13,   50
    { "SPOSF2F8",   25,   50,  43,  54, true  },    //   21,   50
    { "SPOSF3F7",   26,   50,  49,  53, true  },    //   24,   49
    { "SPOSF5",     10,   47,  24,  51, true  },    //   11,   46
    { "SPOSG2G8",   16,   52,  33,  54, true  },    //   13,   51
    { "SPOSG3G7",   22,   52,  43,  53, true  },    //   21,   50
    { "SPOSG4G6",   21,   49,  43,  52, true  },    //   20,   50
    { "SPOSG5",     16,   49,  31,  53, true  },    //   17,   49
    { "SPOSO0",     21,   58,  48,  61, true  },    //   25,   58
    { "SPOSQ0",     23,   47,  55,  51, true  },    //   27,   47
    { "SSWVB3",     18,   51,  33,  56, true  },    //   14,   51
    { "SSWVB4",     16,   51,  27,  56, true  },    //   12,   51
    { "SSWVC7",     19,   49,  36,  54, true  },    //   15,   49
    { "SSWVC8",     15,   48,  29,  53, true  },    //   11,   48
    { "SSWVD3",     19,   51,  33,  56, true  },    //   15,   51
    { "SSWVD4",     16,   51,  27,  56, true  },    //   12,   51
    { "SSWVI0",     14,   49,  29,  54, true  },    //   18,   49
    { "SSWVJ0",     11,   41,  33,  46, true  },    //   15,   41
    { "SSWVN0",     19,   54,  37,  59, true  },    //   15,   54
    { "SSWVP0",     25,   56,  48,  61, true  },    //   25,   57
    { "SSWVQ0",     28,   50,  53,  55, true  },    //   24,   51
    { "SSWVS0",     28,   37,  57,  42, true  },    //   24,   37
    { "SSWVT0",     28,   30,  57,  35, true  },    //   24,   30
    { "SSWVU0",     28,   20,  57,  25, true  },    //   24,   20
    { "SSWVV0",     28,   15,  57,  20, true  },    //   24,   15
    { "STIMA0",      7,   13,  14,  15, true  },    //    7,   15
    { "SUITA0",     12,   54,  24,  47, true  },    //   11,   51
    { "TBLUA0",     13,   92,  26,  96, true  },    //   14,   92
    { "TBLUB0",     13,   92,  26,  96, true  },    //   14,   92
    { "TBLUC0",     13,   92,  26,  96, true  },    //   14,   92
    { "TBLUD0",     13,   93,  26,  97, true  },    //   14,   93
    { "TFOGA0",     19,   58,  41,  56, true  },    //   21,   56
    { "TFOGI0",      6,   32,  13,  13, true  },    //    6,   28
    { "TFOGJ0",      8,   34,  17,  17, true  },    //    8,   30
    { "TGRNA0",     13,   92,  26,  96, true  },    //   14,   92
    { "TGRNB0",     13,   87,  26,  91, true  },    //   14,   87
    { "TGRNC0",     13,   87,  26,  91, true  },    //   14,   87
    { "TGRND0",     13,   93,  26,  97, true  },    //   14,   93
    { "TLMPA0",     11,   78,  23,  80, true  },    //   11,   77
    { "TLMPB0",     11,   78,  23,  80, true  },    //   11,   77
    { "TLMPC0",     11,   78,  23,  80, true  },    //   11,   77
    { "TLMPD0",     11,   78,  23,  80, true  },    //   11,   77
    { "TLP2A0",     10,   58,  21,  60, true  },    //   10,   57
    { "TLP2B0",     10,   58,  21,  60, true  },    //   10,   57
    { "TLP2C0",     10,   58,  21,  60, true  },    //   10,   57
    { "TLP2D0",     10,   58,  21,  60, true  },    //   10,   57
    { "TRE1A0",     28,   65,  51,  70, true  },    //   25,   65
    { "TRE2A0",     62,  120, 124, 124, true  },    //   63,  120
    { "TREDA0",     13,   92,  26,  96, true  },    //   14,   92
    { "TREDB0",     13,   87,  26,  91, true  },    //   14,   87
    { "TREDC0",     13,   87,  26,  91, true  },    //   14,   87
    { "TREDD0",     13,   93,  26,  97, true  },    //   14,   93
    { "TROOA1",     21,   52,  41,  57, true  },    //   19,   52
    { "TROOA2A8",   24,   50,  40,  55, true  },    //   17,   50
    { "TROOA3A7",   21,   47,  36,  49, true  },    //   15,   44
    { "TROOA4A6",   17,   47,  30,  47, true  },    //   20,   42
    { "TROOA5",     18,   46,  35,  49, true  },    //   21,   44
    { "TROOB1",     19,   51,  39,  56, true  },    //   17,   51
    { "TROOB2B8",   21,   52,  38,  57, true  },    //   13,   52
    { "TROOB3B7",   22,   48,  38,  51, true  },    //   16,   46
    { "TROOB4B6",   18,   45,  33,  47, true  },    //   19,   42
    { "TROOB5",     17,   43,  33,  46, true  },    //   20,   41
    { "TROOC1",     19,   55,  39,  60, true  },    //   17,   55
    { "TROOC2C8",   21,   53,  41,  58, true  },    //   14,   53
    { "TROOC3C7",   19,   50,  31,  53, true  },    //   13,   48
    { "TROOC4C6",   14,   49,  28,  51, true  },    //   12,   46
    { "TROOC5",     18,   46,  35,  49, true  },    //   22,   44
    { "TROOD1",     18,   52,  37,  57, true  },    //   16,   52
    { "TROOD2D8",   24,   50,  45,  55, true  },    //   17,   50
    { "TROOD3D7",   22,   45,  39,  48, true  },    //   19,   43
    { "TROOD4D6",   17,   44,  30,  46, true  },    //   17,   41
    { "TROOD5",     18,   43,  33,  46, true  },    //   21,   41
    { "TROOE1",     33,   55,  49,  60, true  },    //   30,   55
    { "TROOE2E8",   14,   51,  32,  56, true  },    //   11,   51
    { "TROOE3E7",   20,   46,  39,  49, true  },    //   23,   44
    { "TROOE4E6",   13,   45,  34,  47, true  },    //   20,   42
    { "TROOE5",     12,   45,  38,  48, true  },    //   17,   43
    { "TROOF1",     21,   50,  44,  55, true  },    //   18,   50
    { "TROOF2F8",   28,   49,  45,  54, true  },    //   25,   49
    { "TROOF3F7",   27,   46,  42,  49, true  },    //   18,   44
    { "TROOF4F6",   23,   45,  36,  47, true  },    //   16,   42
    { "TROOF5",     18,   43,  33,  46, true  },    //   12,   41
    { "TROOG1",      8,   50,  32,  55, true  },    //    5,   50
    { "TROOG2G8",   32,   50,  55,  55, true  },    //   25,   50
    { "TROOG3G7",   47,   48,  59,  51, true  },    //   27,   46
    { "TROOG4G6",   33,   45,  46,  47, true  },    //   23,   42
    { "TROOG5",     22,   46,  31,  49, true  },    //   16,   44
    { "TROOH1",     21,   50,  41,  55, true  },    //   18,   50
    { "TROOH2H8",   12,   51,  31,  56, true  },    //    6,   51
    { "TROOH3H7",   15,   54,  34,  57, true  },    //   12,   52
    { "TROOH4H6",   15,   53,  34,  56, true  },    //    9,   51
    { "TROOH5",     17,   52,  39,  57, true  },    //   21,   52
    { "TROOM0",     27,   16,  58,  22, true  },    //   29,   19
    { "TROOO0",     28,   56,  49,  61, true  },    //   20,   56
    { "TROOP0",     28,   56,  55,  61, true  },    //   24,   56
    { "TROOQ0",     28,   56,  57,  61, true  },    //   24,   56
    { "TROOR0",     28,   39,  57,  44, true  },    //   24,   39
    { "VILEA3D7",   44,   68,  81,  71, true  },    //   41,   68
    { "VILEA4D6",   35,   67,  64,  70, true  },    //   31,   67
    { "VILEA5D5",   15,   65,  26,  69, true  },    //   11,   65
    { "VILEA6D4",   32,   66,  56,  69, true  },    //   30,   66
    { "VILEA7D3",   37,   67,  77,  69, true  },    //   36,   67
    { "VILEA8D2",   31,   70,  72,  74, true  },    //   35,   70
    { "VILEB4E6",   18,   67,  45,  69, true  },    //   22,   67
    { "VILEB5E5",   23,   67,  42,  68, true  },    //   18,   67
    { "VILEB6E4",   27,   67,  54,  69, true  },    //   25,   67
    { "VILEB7E3",   34,   70,  67,  72, true  },    //   31,   70
    { "VILEB8E2",   25,   72,  48,  75, true  },    //   23,   72
    { "VILEC4F6",   31,   71,  52,  74, true  },    //   26,   71
    { "VILEC6F4",   25,   70,  50,  73, true  },    //   22,   70
    { "VILEC7F3",   24,   72,  49,  75, true  },    //   21,   72
    { "VILEC8F2",   21,   74,  54,  76, true  },    //   25,   74
    { "VILEG2",      3,   91,  64,  96, true  },    //   27,   91
    { "VILEG3",     24,   97,  73, 102, true  },    //   36,   97
    { "VILEG4",     36,   97,  84, 101, true  },    //   39,   97
    { "VILEG5",     44,   99,  83, 103, true  },    //   40,   99
    { "VILEG6",     54,   99,  59, 104, true  },    //   30,   99
    { "VILEG7",     40,   98,  57, 102, true  },    //   28,   98
    { "VILEH2",      4,   93,  65,  98, true  },    //   28,   93
    { "VILEH3",     24,   99,  75, 104, true  },    //   36,   99
    { "VILEH4",     36,   99,  87, 103, true  },    //   43,   99
    { "VILEH5",     45,   99,  86, 103, true  },    //   41,   99
    { "VILEH6",     55,   99,  62, 104, true  },    //   31,   99
    { "VILEH7",     41,   99,  58, 103, true  },    //   29,   99
    { "VILEI2",      4,   94,  65,  99, true  },    //   28,   94
    { "VILEI3",     24,   99,  75, 104, true  },    //   36,   99
    { "VILEI4",     36,   99,  87, 103, true  },    //   43,   99
    { "VILEI5",     45,  101,  86, 105, true  },    //   41,  101
    { "VILEI6",     56,   99,  63, 104, true  },    //   32,   99
    { "VILEI7",     42,   99,  59, 103, true  },    //   30,   99
    { "VILEJ6",     22,   85,  47,  90, true  },    //   18,   85
    { "VILEJ8",     30,   78,  67,  80, true  },    //   34,   78
    { "VILEK7",     31,   59,  59,  63, true  },    //   27,   59
    { "VILEL3",     24,   58,  56,  63, true  },    //   28,   58
    { "VILEL5",     23,   57,  49,  61, true  },    //   27,   57
    { "VILEL7",     31,   56,  57,  60, true  },    //   27,   56
    { "VILEM3",     24,   58,  56,  63, true  },    //   28,   58
    { "VILEM5",     23,   57,  49,  61, true  },    //   27,   57
    { "VILEM6",     19,   57,  40,  62, true  },    //   15,   57
    { "VILEM7",     31,   56,  57,  60, true  },    //   27,   56
    { "VILEN2",     57,   65,  69,  70, true  },    //   35,   65
    { "VILEN3",     67,   64,  99,  68, true  },    //   51,   64
    { "VILEN4",     52,   61,  87,  65, true  },    //   43,   61
    { "VILEN5",     23,   64,  50,  68, true  },    //   27,   64
    { "VILEN6",     15,   63,  59,  68, true  },    //   26,   63
    { "VILEN7",     30,   63,  97,  67, true  },    //   45,   63
    { "VILEN8",     31,   63,  97,  67, true  },    //   48,   63
    { "VILEO2",     56,   71,  68,  76, true  },    //   34,   71
    { "VILEO3",     67,   65,  99,  69, true  },    //   51,   65
    { "VILEO4",     52,   62,  87,  66, true  },    //   43,   62
    { "VILEO5",     23,   64,  50,  68, true  },    //   27,   64
    { "VILEO6",     15,   63,  59,  68, true  },    //   26,   63
    { "VILEO7",     30,   63,  97,  67, true  },    //   45,   63
    { "VILEO8",     31,   66,  96,  71, true  },    //   48,   66
    { "VILEP2",     56,   83,  68,  88, true  },    //   34,   83
    { "VILEP3",     67,   85,  99,  89, true  },    //   51,   85
    { "VILEP4",     51,   77,  86,  81, true  },    //   42,   77
    { "VILEP5",     23,   77,  50,  81, true  },    //   27,   77
    { "VILEP6",     15,   77,  57,  82, true  },    //   26,   77
    { "VILEP7",     30,   76,  97,  80, true  },    //   45,   76
    { "VILEP8",     31,   79,  96,  84, true  },    //   48,   79
    { "VILEQ2",     20,   69,  49,  74, true  },    //   22,   69
    { "VILEQ3",     36,   67,  71,  72, true  },    //   32,   67
    { "VILEQ4",     29,   69,  60,  73, true  },    //   27,   69
    { "VILEQ7",     29,   68,  63,  71, true  },    //   30,   68
    { "VILEQ8",     31,   66,  69,  69, true  },    //   33,   66
    { "VILES0",     21,   71,  47,  76, true  },    //   23,   71
    { "VILET0",     26,   66,  55,  70, true  },    //   27,   66
    { "VILEU0",     31,   54,  67,  58, true  },    //   35,   54
    { "VILE[1",     50,   68, 107,  72, true  },    //   53,   68
    { "VILE[2",     53,   69,  98,  74, true  },    //   49,   69
    { "VILE[3",     31,   72,  56,  76, true  },    //   26,   72
    { "VILE[4",     49,   71,  79,  75, true  },    //   37,   71
    { "VILE[5",     52,   71,  92,  75, true  },    //   44,   71
    { "VILE[6",     32,   71,  88,  76, true  },    //   43,   73
    { "VILE[7",     19,   70,  61,  75, true  },    //   30,   73
    { "VILE[8",     29,   69,  74,  74, true  },    //   36,   73
    { "VILE\\1",    50,   69, 108,  73, true  },    //   53,   69
    { "VILE\\2",    53,   70,  99,  75, true  },    //   49,   70
    { "VILE\\3",    31,   73,  56,  77, true  },    //   26,   73
    { "VILE\\4",    49,   71,  79,  75, true  },    //   37,   71
    { "VILE\\5",    52,   72,  92,  76, true  },    //   44,   72
    { "VILE\\6",    32,   72,  88,  77, true  },    //   43,   74
    { "VILE\\7",    19,   69,  61,  74, true  },    //   30,   72
    { "VILE\\8",    29,   69,  74,  74, true  },    //   32,   73
    { "VILE]1",     50,   68, 107,  72, true  },    //   53,   68
    { "VILE]2",     53,   69,  98,  74, true  },    //   49,   69
    { "VILE]3",     31,   71,  56,  75, true  },    //   26,   71
    { "VILE]4",     49,   70,  79,  74, true  },    //   37,   70
    { "VILE]5",     52,   70,  92,  74, true  },    //   44,   70
    { "VILE]6",     32,   70,  88,  75, true  },    //   43,   72
    { "VILE]7",     19,   71,  61,  76, true  },    //   30,   74
    { "VILE]8",     29,   67,  74,  72, true  },    //   32,   71
    { "YKEYA0",      7,   14,  14,  16, true  },    //    8,   19
    { "YKEYB0",      7,   14,  14,  16, true  },    //    8,   19
    { "YSKUA0",      7,   14,  13,  16, true  },    //    7,   18
    { "YSKUB0",      7,   14,  13,  16, true  },    //    7,   18
    { "",            0,    0,   0,   0, true  }
};

// DSDHacked
char        **sprnames;
int         numsprites;
static char **deh_spritenames;
static byte *sprnames_state;

void InitSprites(void)
{
    sprnames = original_sprnames;
    numsprites = NUMSPRITES;

    array_grow(deh_spritenames, numsprites);

    for (int i = 0; i < numsprites; i++)
        deh_spritenames[i] = M_StringDuplicate(sprnames[i]);

    array_grow(sprnames_state, numsprites);
    memset(sprnames_state, 0, numsprites * sizeof(*sprnames_state));
}

void FreeSprites(void)
{
    for (int i = 0; i < array_capacity(deh_spritenames); i++)
        if (deh_spritenames[i])
            free(deh_spritenames[i]);

    array_free(deh_spritenames);
    array_free(sprnames_state);
}

static void EnsureSpritesCapacity(const int limit)
{
    const int   old_numsprites = numsprites;
    static bool first_allocation = true;
    int         size_delta;

    if (limit < numsprites)
        return;

    if (first_allocation)
    {
        sprnames = NULL;
        array_grow(sprnames, old_numsprites + limit);
        memcpy(sprnames, original_sprnames, old_numsprites * sizeof(*sprnames));
        first_allocation = false;
    }
    else
        array_grow(sprnames, limit);

    numsprites = array_capacity(sprnames);
    size_delta = numsprites - old_numsprites;
    memset(sprnames + old_numsprites, 0, size_delta * sizeof(*sprnames));

    array_grow(sprnames_state, size_delta);
    memset(sprnames_state + old_numsprites, 0, size_delta * sizeof(*sprnames_state));
}

int dsdh_GetDehSpriteIndex(const char *key)
{
    for (int i = 0; i < numsprites; i++)
        if (sprnames[i] && !strncasecmp(sprnames[i], key, 4) && !sprnames_state[i])
        {
            sprnames_state[i] = true;   // sprite has been edited
            return i;
        }

    return -1;
}

int dsdh_GetOriginalSpriteIndex(const char *key)
{
    int limit;

    for (int i = 0; i < array_capacity(deh_spritenames); i++)
        if (deh_spritenames[i] && !strncasecmp(deh_spritenames[i], key, 4))
            return i;

    // is it a number?
    for (const char *c = key; *c; c++)
        if (!isdigit(*c))
            return -1;

    limit = strtol(key, NULL, 10);
    EnsureSpritesCapacity(limit);

    return limit;
}
