/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "g_game.h"
#include "i_system.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_local.h"
#include "sounds.h"
#include "version.h"
#include "w_wad.h"
#include "v_video.h"
#include "z_zone.h"

// killough 10/98: new functions, to allow processing DEH files in-memory
// (e.g. from wads)

typedef struct
{
    byte    *inp;
    byte    *lump;
    int     size;
    FILE    *f;
} DEHFILE;

static bool addtodehmaptitlecount;
static int  linecount;

int         dehcount = 0;
int         dehmaptitlecount = 0;
bool        dehacked = false;

byte        defined_codeptr_args[NUMSTATES] = { 0 };

// killough 10/98: emulate IO whether input really comes from a file or not

// haleyjd: got rid of macros for MSVC
static char *dehfgets(char *buf, int n, DEHFILE *fp)
{
    if (!fp->lump)                              // If this is a real file, return regular fgets
    {
        linecount++;
        return fgets(buf, n, fp->f);
    }

    if (!n || fp->size <= 0 || !*fp->inp)       // If no more characters
        return NULL;

    if (n == 1)
    {
        fp->size--;
        *buf = *fp->inp++;
    }
    else
    {                                           // copy buffer
        char    *p = buf;

        while (n > 1 && *fp->inp && fp->size && (n--, fp->size--, *p++ = *fp->inp++) != '\n');

        *p = 0;
    }

    linecount++;

    return buf;                                 // Return buffer pointer
}

static int dehfeof(DEHFILE *fp)
{
    return (!fp->lump ? feof(fp->f) : fp->size <= 0 || !*fp->inp);
}

static int dehfgetc(DEHFILE *fp)
{
    return (!fp->lump ? fgetc(fp->f) : fp->size > 0 ? fp->size--, *fp->inp++ : EOF);
}

// #include "d_deh.h" -- we don't do that here but we declare the
// variables. This externalizes everything that there is a string
// set for in the language files. See d_deh.h for detailed comments,
// original English values etc. These are set to the macro values,
// which are set by D_ENGLSH.H or D_FRENCH.H(etc). BEX files are a
// better way of changing these strings globally by language.

// ====================================================================
// Any of these can be changed using the bex extensions

char    *s_VERSION = "";

char    *s_D_DEVSTR = D_DEVSTR;
char    *s_D_CDROM = D_CDROM;

char    *s_PRESSKEY = PRESSKEY;
char    *s_PRESSYN = PRESSYN;
char    *s_PRESSA = "";
char    *s_QUITMSG = QUITMSG;
char    *s_QUITMSG1 = QUITMSG1;
char    *s_QUITMSG2 = QUITMSG2;
char    *s_QUITMSG3 = QUITMSG3;
char    *s_QUITMSG4 = QUITMSG4;
char    *s_QUITMSG5 = QUITMSG5;
char    *s_QUITMSG6 = QUITMSG6;
char    *s_QUITMSG7 = QUITMSG7;
char    *s_QUITMSG8 = QUITMSG8;
char    *s_QUITMSG9 = QUITMSG9;
char    *s_QUITMSG10 = QUITMSG10;
char    *s_QUITMSG11 = QUITMSG11;
char    *s_QUITMSG12 = QUITMSG12;
char    *s_QUITMSG13 = QUITMSG13;
char    *s_QUITMSG14 = QUITMSG14;
char    *s_LOADNET = LOADNET;
char    *s_QLOADNET = QLOADNET;
char    *s_QSAVESPOT = QSAVESPOT;
char    *s_SAVEDEAD = SAVEDEAD;
char    *s_QSPROMPT = QSPROMPT;
char    *s_QLPROMPT = QLPROMPT;
char    *s_DELPROMPT = "";
char    *s_NEWGAME = NEWGAME;
char    *s_NIGHTMARE = NIGHTMARE;
char    *s_SWSTRING = SWSTRING;
char    *s_MSGOFF = MSGOFF;
char    *s_MSGON = MSGON;
char    *s_NETEND = NETEND;
char    *s_ENDGAME = ENDGAME;
char    *s_DOSY = DOSY;
char    *s_DOSA = "";
char    *s_DETAILHI = DETAILHI;
char    *s_DETAILLO = DETAILLO;
char    *s_GAMMALVL0 = GAMMALVL0;
char    *s_GAMMALVL1 = GAMMALVL1;
char    *s_GAMMALVL2 = GAMMALVL2;
char    *s_GAMMALVL3 = GAMMALVL3;
char    *s_GAMMALVL4 = GAMMALVL4;
char    *s_GAMMALVL = "";
char    *s_GAMMAOFF = "";
char    *s_EMPTYSTRING = EMPTYSTRING;

char    *s_GOTARMOR = GOTARMOR;
char    *s_GOTMEGA = GOTMEGA;
char    *s_GOTHTHBONUS = GOTHTHBONUS;
char    *s_GOTARMBONUS = GOTARMBONUS;
char    *s_GOTSTIM = GOTSTIM;
char    *s_GOTMEDINEED = GOTMEDINEED;
char    *s_GOTMEDIKIT = GOTMEDIKIT;
char    *s_GOTSUPER = GOTSUPER;

char    *s_GOTBLUECARD = GOTBLUECARD;
char    *s_GOTYELWCARD = GOTYELWCARD;
char    *s_GOTREDCARD = GOTREDCARD;
char    *s_GOTBLUESKUL = GOTBLUESKUL;
char    *s_GOTYELWSKUL = GOTYELWSKUL;
char    *s_GOTREDSKUL = GOTREDSKULL;
char    *s_GOTREDSKULL = GOTREDSKULL;

char    *s_GOTINVUL = GOTINVUL;
char    *s_GOTBERSERK = GOTBERSERK;
char    *s_GOTINVIS = GOTINVIS;
char    *s_GOTSUIT = GOTSUIT;
char    *s_GOTMAP = GOTMAP;
char    *s_GOTVISOR = GOTVISOR;

char    *s_GOTCLIP = GOTCLIP;
char    *s_GOTCLIPBOX = GOTCLIPBOX;
char    *s_GOTROCKET = GOTROCKET;
char    *s_GOTROCKETX2 = "";
char    *s_GOTROCKBOX = GOTROCKBOX;
char    *s_GOTCELL = GOTCELL;
char    *s_GOTCELLX2 = "";
char    *s_GOTCELLBOX = GOTCELLBOX;
char    *s_GOTSHELLS = GOTSHELLS;
char    *s_GOTSHELLSX2 = "";
char    *s_GOTSHELLBOX = GOTSHELLBOX;
char    *s_GOTBACKPACK = GOTBACKPACK;

char    *s_GOTBFG9000 = GOTBFG9000;
char    *s_GOTCHAINGUN = GOTCHAINGUN;
char    *s_GOTCHAINSAW = GOTCHAINSAW;
char    *s_GOTLAUNCHER = GOTLAUNCHER;
char    *s_GOTMSPHERE = GOTMSPHERE;
char    *s_GOTPLASMA = GOTPLASMA;
char    *s_GOTSHOTGUN = GOTSHOTGUN;
char    *s_GOTSHOTGUN2 = GOTSHOTGUN2;

char    *s_PD_BLUEO = PD_BLUEO;
char    *s_PD_REDO = PD_REDO;
char    *s_PD_YELLOWO = PD_YELLOWO;
char    *s_PD_BLUEK = PD_BLUEK;
char    *s_PD_REDK = PD_REDK;
char    *s_PD_YELLOWK = PD_YELLOWK;
char    *s_PD_BLUEC = "";
char    *s_PD_REDC = "";
char    *s_PD_YELLOWC = "";
char    *s_PD_BLUES = "";
char    *s_PD_REDS = "";
char    *s_PD_YELLOWS = "";
char    *s_PD_ANY = "";
char    *s_PD_ALL3 = "";
char    *s_PD_ALL6 = "";
char    *s_PD_KEYCARD = "";
char    *s_PD_SKULLKEY = "";
char    *s_PD_KEYCARDORSKULLKEY = "";
char    *s_PD_BLUECO = "";
char    *s_PD_REDCO = "";
char    *s_PD_YELLOWCO = "";

char    *s_SECRETMESSAGE = "";

char    *s_GGSAVED = GGSAVED;
char    *s_GGAUTOSAVED = "";
char    *s_GGLOADED = "";
char    *s_GGAUTOLOADED = "";
char    *s_GGDELETED = "";
char    *s_GSCREENSHOT = GSCREENSHOT;

char    *s_ALWAYSRUNOFF = "";
char    *s_ALWAYSRUNON = "";

char    *s_HUSTR_MSGU = HUSTR_MSGU;

char    *s_HUSTR_E1M1 = HUSTR_E1M1;
char    *s_HUSTR_E1M2 = HUSTR_E1M2;
char    *s_HUSTR_E1M3 = HUSTR_E1M3;
char    *s_HUSTR_E1M4 = HUSTR_E1M4;
char    *s_HUSTR_E1M4B = "";
char    *s_HUSTR_E1M5 = HUSTR_E1M5;
char    *s_HUSTR_E1M6 = HUSTR_E1M6;
char    *s_HUSTR_E1M7 = HUSTR_E1M7;
char    *s_HUSTR_E1M8 = HUSTR_E1M8;
char    *s_HUSTR_E1M8B = "";
char    *s_HUSTR_E1M9 = HUSTR_E1M9;
char    *s_HUSTR_E2M1 = HUSTR_E2M1;
char    *s_HUSTR_E2M2 = HUSTR_E2M2;
char    *s_HUSTR_E2M3 = HUSTR_E2M3;
char    *s_HUSTR_E2M4 = HUSTR_E2M4;
char    *s_HUSTR_E2M5 = HUSTR_E2M5;
char    *s_HUSTR_E2M6 = HUSTR_E2M6;
char    *s_HUSTR_E2M7 = HUSTR_E2M7;
char    *s_HUSTR_E2M8 = HUSTR_E2M8;
char    *s_HUSTR_E2M9 = HUSTR_E2M9;
char    *s_HUSTR_E3M1 = HUSTR_E3M1;
char    *s_HUSTR_E3M2 = HUSTR_E3M2;
char    *s_HUSTR_E3M3 = HUSTR_E3M3;
char    *s_HUSTR_E3M4 = HUSTR_E3M4;
char    *s_HUSTR_E3M5 = HUSTR_E3M5;
char    *s_HUSTR_E3M6 = HUSTR_E3M6;
char    *s_HUSTR_E3M7 = HUSTR_E3M7;
char    *s_HUSTR_E3M7_ALT = "";
char    *s_HUSTR_E3M8 = HUSTR_E3M8;
char    *s_HUSTR_E3M9 = HUSTR_E3M9;
char    *s_HUSTR_E4M1 = HUSTR_E4M1;
char    *s_HUSTR_E4M2 = HUSTR_E4M2;
char    *s_HUSTR_E4M3 = HUSTR_E4M3;
char    *s_HUSTR_E4M4 = HUSTR_E4M4;
char    *s_HUSTR_E4M5 = HUSTR_E4M5;
char    *s_HUSTR_E4M6 = HUSTR_E4M6;
char    *s_HUSTR_E4M7 = HUSTR_E4M7;
char    *s_HUSTR_E4M8 = HUSTR_E4M8;
char    *s_HUSTR_E4M9 = HUSTR_E4M9;
char    *s_HUSTR_E5M1 = "";
char    *s_HUSTR_E5M2 = "";
char    *s_HUSTR_E5M3 = "";
char    *s_HUSTR_E5M4 = "";
char    *s_HUSTR_E5M5 = "";
char    *s_HUSTR_E5M6 = "";
char    *s_HUSTR_E5M7 = "";
char    *s_HUSTR_E5M8 = "";
char    *s_HUSTR_E5M9 = "";
char    *s_HUSTR_1 = HUSTR_1;
char    *s_HUSTR_2 = HUSTR_2;
char    *s_HUSTR_3 = HUSTR_3;
char    *s_HUSTR_4 = HUSTR_4;
char    *s_HUSTR_5 = HUSTR_5;
char    *s_HUSTR_6 = HUSTR_6;
char    *s_HUSTR_7 = HUSTR_7;
char    *s_HUSTR_8 = HUSTR_8;
char    *s_HUSTR_9 = HUSTR_9;
char    *s_HUSTR_10 = HUSTR_10;
char    *s_HUSTR_11 = HUSTR_11;
char    *s_HUSTR_11_ALT = "";
char    *s_HUSTR_12 = HUSTR_12;
char    *s_HUSTR_13 = HUSTR_13;
char    *s_HUSTR_14 = HUSTR_14;
char    *s_HUSTR_15 = HUSTR_15;
char    *s_HUSTR_16 = HUSTR_16;
char    *s_HUSTR_17 = HUSTR_17;
char    *s_HUSTR_18 = HUSTR_18;
char    *s_HUSTR_19 = HUSTR_19;
char    *s_HUSTR_20 = HUSTR_20;
char    *s_HUSTR_21 = HUSTR_21;
char    *s_HUSTR_22 = HUSTR_22;
char    *s_HUSTR_23 = HUSTR_23;
char    *s_HUSTR_24 = HUSTR_24;
char    *s_HUSTR_25 = HUSTR_25;
char    *s_HUSTR_26 = HUSTR_26;
char    *s_HUSTR_27 = HUSTR_27;
char    *s_HUSTR_28 = HUSTR_28;
char    *s_HUSTR_29 = HUSTR_29;
char    *s_HUSTR_30 = HUSTR_30;
char    *s_HUSTR_31 = HUSTR_31;
char    *s_HUSTR_31_BFG = "";
char    *s_HUSTR_32 = HUSTR_32;
char    *s_HUSTR_32_BFG = "";
char    *s_HUSTR_33 = "";
char    *s_PHUSTR_1 = PHUSTR_1;
char    *s_PHUSTR_2 = PHUSTR_2;
char    *s_PHUSTR_3 = PHUSTR_3;
char    *s_PHUSTR_4 = PHUSTR_4;
char    *s_PHUSTR_5 = PHUSTR_5;
char    *s_PHUSTR_6 = PHUSTR_6;
char    *s_PHUSTR_7 = PHUSTR_7;
char    *s_PHUSTR_8 = PHUSTR_8;
char    *s_PHUSTR_9 = PHUSTR_9;
char    *s_PHUSTR_10 = PHUSTR_10;
char    *s_PHUSTR_11 = PHUSTR_11;
char    *s_PHUSTR_12 = PHUSTR_12;
char    *s_PHUSTR_13 = PHUSTR_13;
char    *s_PHUSTR_14 = PHUSTR_14;
char    *s_PHUSTR_15 = PHUSTR_15;
char    *s_PHUSTR_16 = PHUSTR_16;
char    *s_PHUSTR_17 = PHUSTR_17;
char    *s_PHUSTR_18 = PHUSTR_18;
char    *s_PHUSTR_19 = PHUSTR_19;
char    *s_PHUSTR_20 = PHUSTR_20;
char    *s_PHUSTR_21 = PHUSTR_21;
char    *s_PHUSTR_22 = PHUSTR_22;
char    *s_PHUSTR_23 = PHUSTR_23;
char    *s_PHUSTR_24 = PHUSTR_24;
char    *s_PHUSTR_25 = PHUSTR_25;
char    *s_PHUSTR_26 = PHUSTR_26;
char    *s_PHUSTR_27 = PHUSTR_27;
char    *s_PHUSTR_28 = PHUSTR_28;
char    *s_PHUSTR_29 = PHUSTR_29;
char    *s_PHUSTR_30 = PHUSTR_30;
char    *s_PHUSTR_31 = PHUSTR_31;
char    *s_PHUSTR_32 = PHUSTR_32;
char    *s_THUSTR_1 = THUSTR_1;
char    *s_THUSTR_2 = THUSTR_2;
char    *s_THUSTR_3 = THUSTR_3;
char    *s_THUSTR_4 = THUSTR_4;
char    *s_THUSTR_5 = THUSTR_5;
char    *s_THUSTR_6 = THUSTR_6;
char    *s_THUSTR_7 = THUSTR_7;
char    *s_THUSTR_8 = THUSTR_8;
char    *s_THUSTR_9 = THUSTR_9;
char    *s_THUSTR_10 = THUSTR_10;
char    *s_THUSTR_11 = THUSTR_11;
char    *s_THUSTR_12 = THUSTR_12;
char    *s_THUSTR_13 = THUSTR_13;
char    *s_THUSTR_14 = THUSTR_14;
char    *s_THUSTR_15 = THUSTR_15;
char    *s_THUSTR_16 = THUSTR_16;
char    *s_THUSTR_17 = THUSTR_17;
char    *s_THUSTR_18 = THUSTR_18;
char    *s_THUSTR_19 = THUSTR_19;
char    *s_THUSTR_20 = THUSTR_20;
char    *s_THUSTR_21 = THUSTR_21;
char    *s_THUSTR_22 = THUSTR_22;
char    *s_THUSTR_23 = THUSTR_23;
char    *s_THUSTR_24 = THUSTR_24;
char    *s_THUSTR_25 = THUSTR_25;
char    *s_THUSTR_26 = THUSTR_26;
char    *s_THUSTR_27 = THUSTR_27;
char    *s_THUSTR_28 = THUSTR_28;
char    *s_THUSTR_29 = THUSTR_29;
char    *s_THUSTR_30 = THUSTR_30;
char    *s_THUSTR_31 = THUSTR_31;
char    *s_THUSTR_32 = THUSTR_32;
char    *s_NHUSTR_1 = "";
char    *s_NHUSTR_2 = "";
char    *s_NHUSTR_3 = "";
char    *s_NHUSTR_4 = "";
char    *s_NHUSTR_5 = "";
char    *s_NHUSTR_6 = "";
char    *s_NHUSTR_7 = "";
char    *s_NHUSTR_8 = "";
char    *s_NHUSTR_9 = "";

char    *s_HUSTR_CHATMACRO1 = HUSTR_CHATMACRO1;
char    *s_HUSTR_CHATMACRO2 = HUSTR_CHATMACRO2;
char    *s_HUSTR_CHATMACRO3 = HUSTR_CHATMACRO3;
char    *s_HUSTR_CHATMACRO4 = HUSTR_CHATMACRO4;
char    *s_HUSTR_CHATMACRO5 = HUSTR_CHATMACRO5;
char    *s_HUSTR_CHATMACRO6 = HUSTR_CHATMACRO6;
char    *s_HUSTR_CHATMACRO7 = HUSTR_CHATMACRO7;
char    *s_HUSTR_CHATMACRO8 = HUSTR_CHATMACRO8;
char    *s_HUSTR_CHATMACRO9 = HUSTR_CHATMACRO9;
char    *s_HUSTR_CHATMACRO0 = HUSTR_CHATMACRO0;

char    *s_HUSTR_TALKTOSELF1 = HUSTR_TALKTOSELF1;
char    *s_HUSTR_TALKTOSELF2 = HUSTR_TALKTOSELF2;
char    *s_HUSTR_TALKTOSELF3 = HUSTR_TALKTOSELF3;
char    *s_HUSTR_TALKTOSELF4 = HUSTR_TALKTOSELF4;
char    *s_HUSTR_TALKTOSELF5 = HUSTR_TALKTOSELF5;

char    *s_HUSTR_MESSAGESENT = HUSTR_MESSAGESENT;

char    *s_HUSTR_PLRGREEN = HUSTR_PLRGREEN;
char    *s_HUSTR_PLRINDIGO = HUSTR_PLRINDIGO;
char    *s_HUSTR_PLRBROWN = HUSTR_PLRBROWN;
char    *s_HUSTR_PLRRED = HUSTR_PLRRED;

char    *s_AMSTR_FOLLOWON = AMSTR_FOLLOWON;
char    *s_AMSTR_FOLLOWOFF = AMSTR_FOLLOWOFF;
char    *s_AMSTR_GRIDON = AMSTR_GRIDON;
char    *s_AMSTR_GRIDOFF = AMSTR_GRIDOFF;
char    *s_AMSTR_MARKEDSPOT = AMSTR_MARKEDSPOT;
char    *s_AMSTR_MARKCLEARED = "";
char    *s_AMSTR_MARKSCLEARED = AMSTR_MARKSCLEARED;
char    *s_AMSTR_ROTATEON = "";
char    *s_AMSTR_ROTATEOFF = "";

char    *s_STSTR_MUS = STSTR_MUS;
char    *s_STSTR_NOMUS = STSTR_NOMUS;
char    *s_STSTR_DQDON = STSTR_DQDON;
char    *s_STSTR_DQDOFF = STSTR_DQDOFF;
char    *s_STSTR_KFAADDED = STSTR_KFAADDED;
char    *s_STSTR_FAADDED = STSTR_FAADDED;
char    *s_STSTR_NCON = STSTR_NCON;
char    *s_STSTR_NCOFF = STSTR_NCOFF;
char    *s_STSTR_BEHOLD = STSTR_BEHOLD;
char    *s_STSTR_BEHOLDX = STSTR_BEHOLDX;
char    *s_STSTR_BEHOLDON = "";
char    *s_STSTR_BEHOLDOFF = "";
char    *s_STSTR_BUDDHAON = "";
char    *s_STSTR_BUDDHAOFF = "";
char    *s_STSTR_CHOPPERS = STSTR_CHOPPERS;
char    *s_STSTR_CLEV = STSTR_CLEV;
char    *s_STSTR_CLEVSAME = "";
char    *s_STSTR_MYPOS = "";
char    *s_STSTR_NTON = "";
char    *s_STSTR_NTOFF = "";
char    *s_STSTR_GODON = "";
char    *s_STSTR_GODOFF = "";
char    *s_STSTR_NMON = "";
char    *s_STSTR_NMOFF = "";
char    *s_STSTR_PSON = "";
char    *s_STSTR_PSOFF = "";
char    *s_STSTR_FMON = "";
char    *s_STSTR_FMOFF = "";
char    *s_STSTR_RION = "";
char    *s_STSTR_RIOFF = "";
char    *s_STSTR_RMON = "";
char    *s_STSTR_RMOFF = "";
char    *s_STSTR_FON = "";
char    *s_STSTR_FOFF = "";
char    *s_STSTR_RHON = "";
char    *s_STSTR_RHOFF = "";
char    *s_STSTR_VON = "";
char    *s_STSTR_VOFF = "";
char    *s_STSTR_FPS = "";
char    *s_STSTR_SUCKS = "";
char    *s_STSTR_KILLS = "";
char    *s_STSTR_ITEMS = "";
char    *s_STSTR_SECRETS = "";

char    *s_E1TEXT = E1TEXT;
char    *s_E2TEXT = E2TEXT;
char    *s_E3TEXT = E3TEXT;
char    *s_E4TEXT = E4TEXT;
char    *s_E5TEXT = "";
char    *s_C1TEXT = C1TEXT;
char    *s_C2TEXT = C2TEXT;
char    *s_C3TEXT = C3TEXT;
char    *s_C4TEXT = C4TEXT;
char    *s_C5TEXT = C5TEXT;
char    *s_C6TEXT = C6TEXT;
char    *s_P1TEXT = P1TEXT;
char    *s_P2TEXT = P2TEXT;
char    *s_P3TEXT = P3TEXT;
char    *s_P4TEXT = P4TEXT;
char    *s_P5TEXT = P5TEXT;
char    *s_P6TEXT = P6TEXT;
char    *s_T1TEXT = T1TEXT;
char    *s_T2TEXT = T2TEXT;
char    *s_T3TEXT = T3TEXT;
char    *s_T4TEXT = T4TEXT;
char    *s_T5TEXT = T5TEXT;
char    *s_T6TEXT = T6TEXT;
char    *s_N1TEXT = "";

char    *s_CC_ZOMBIE = CC_ZOMBIE;
char    *s_CC_SHOTGUN = CC_SHOTGUN;
char    *s_CC_HEAVY = CC_HEAVY;
char    *s_CC_IMP = CC_IMP;
char    *s_CC_DEMON = CC_DEMON;
char    *s_CC_SPECTRE = CC_SPECTRE;
char    *s_CC_LOST = CC_LOST;
char    *s_CC_CACO = CC_CACO;
char    *s_CC_HELL = CC_HELL;
char    *s_CC_BARON = CC_BARON;
char    *s_CC_ARACH = CC_ARACH;
char    *s_CC_PAIN = CC_PAIN;
char    *s_CC_REVEN = CC_REVEN;
char    *s_CC_MANCU = CC_MANCU;
char    *s_CC_ARCH = CC_ARCH;
char    *s_CC_SPIDER = CC_SPIDER;
char    *s_CC_CYBER = CC_CYBER;
char    *s_CC_HERO = CC_HERO;

char    *s_M_NEWGAME = "";
char    *s_M_OPTIONS = "";
char    *s_M_LOADGAME = "";
char    *s_M_SAVEGAME = "";
char    *s_M_QUITGAME = "";
char    *s_M_WHICHEPISODE = "";
char    *s_M_EPISODE1 = "";
char    *s_M_EPISODE2 = "";
char    *s_M_EPISODE3 = "";
char    *s_M_EPISODE4 = "";
char    *s_M_EPISODE5 = "";
char    *s_M_EPISODE6 = "";
char    *s_M_EPISODE7 = "";
char    *s_M_EPISODE8 = "";
char    *s_M_WHICHEXPANSION = "";
char    *s_M_EXPANSION1 = "";
char    *s_M_EXPANSION2 = "";
char    *s_M_CHOOSESKILLLEVEL = "";
char    *s_M_SKILLLEVEL1 = "";
char    *s_M_SKILLLEVEL2 = "";
char    *s_M_SKILLLEVEL3 = "";
char    *s_M_SKILLLEVEL4 = "";
char    *s_M_SKILLLEVEL5 = "";
char    *s_M_ENDGAME = "";
char    *s_M_MESSAGES = "";
char    *s_M_ON = "";
char    *s_M_OFF = "";
char    *s_M_GRAPHICDETAIL = "";
char    *s_M_HIGH = "";
char    *s_M_LOW = "";
char    *s_M_SCREENSIZE = "";
char    *s_M_MOUSESENSITIVITY = "";
char    *s_M_GAMECONTROLLERSENSITIVITY = "";
char    *s_M_SOUNDVOLUME = "";
char    *s_M_SFXVOLUME = "";
char    *s_M_MUSICVOLUME = "";
char    *s_M_PAUSED = "";
char    *s_M_MORE = "";

char    *s_CAPTION_SHAREWARE = "";
char    *s_CAPTION_REGISTERED = "";
char    *s_CAPTION_ULTIMATE = "";
char    *s_CAPTION_DOOM2 = "";
char    *s_CAPTION_HELLONEARTH = "";
char    *s_CAPTION_NERVE = "";
char    *s_CAPTION_BFGEDITION = "";
char    *s_CAPTION_PLUTONIA = "";
char    *s_CAPTION_TNT = "";
char    *s_CAPTION_CHEX = "";
char    *s_CAPTION_CHEX2 = "";
char    *s_CAPTION_HACX = "";
char    *s_CAPTION_FREEDOOM1 = "";
char    *s_CAPTION_FREEDOOM2 = "";
char    *s_CAPTION_FREEDM = "";
char    *s_CAPTION_BTSXE1 = "";
char    *s_CAPTION_BTSXE2 = "";
char    *s_CAPTION_REKKR = "";
char    *s_CAPTION_REKKRSL = "";
char    *s_CAPTION_ANOMALYREPORT = "";
char    *s_CAPTION_ARRIVAL = "";
char    *s_CAPTION_DBIMPACT = "";
char    *s_CAPTION_DEATHLESS = "";
char    *s_CAPTION_DOOMZERO = "";
char    *s_CAPTION_EARTHLESS = "";
char    *s_CAPTION_HARMONY = "";
char    *s_CAPTION_NEIS = "";
char    *s_CAPTION_REVOLUTION = "";
char    *s_CAPTION_SYRINGE = "";

char    *s_AUTHOR_ROMERO = "";

char    *bgflatE1 = "FLOOR4_8";
char    *bgflatE2 = "SFLR6_1";
char    *bgflatE3 = "MFLR8_4";
char    *bgflatE4 = "MFLR8_3";
char    *bgflatE5 = "FLOOR4_8";
char    *bgflat06 = "SLIME16";
char    *bgflat11 = "RROCK14";
char    *bgflat20 = "RROCK07";
char    *bgflat30 = "RROCK17";
char    *bgflat15 = "RROCK13";
char    *bgflat31 = "RROCK19";
char    *bgcastcall = "BOSSBACK";

char    *startup1 = "";
char    *startup2 = "";
char    *startup3 = "";
char    *startup4 = "";
char    *startup5 = "";

char    *savegamename = SAVEGAMENAME;

char    *s_BANNER1 = BANNER1;
char    *s_BANNER2 = BANNER2;
char    *s_BANNER3 = BANNER3;
char    *s_BANNER4 = BANNER4;
char    *s_BANNER5 = BANNER5;
char    *s_BANNER6 = BANNER6;
char    *s_BANNER7 = BANNER7;

char    *s_COPYRIGHT1 = COPYRIGHT1;
char    *s_COPYRIGHT2 = COPYRIGHT2;
char    *s_COPYRIGHT3 = COPYRIGHT3;

char    *s_OB_SUICIDE = "";
char    *s_OB_FALLING = "";
char    *s_OB_CRUSH = "";
char    *s_OB_EXIT = "";
char    *s_OB_WATER = "";
char    *s_OB_SLIME = "";
char    *s_OB_LAVA = "";
char    *s_OB_BARREL = "";
char    *s_OB_SPLASH = "";
char    *s_OB_R_SPLASH = "";
char    *s_OB_ROCKET = "";
char    *s_OB_KILLEDSELF = "";
char    *s_OB_STEALTHBABY = "";
char    *s_OB_STEALTHVILE = "";
char    *s_OB_STEALTHBARON = "";
char    *s_OB_STEALTHCACO = "";
char    *s_OB_STEALTHCHAINGUY = "";
char    *s_OB_STEALTHDEMON = "";
char    *s_OB_STEALTHKNIGHT = "";
char    *s_OB_STEALTHIMP = "";
char    *s_OB_STEALTHFATSO = "";
char    *s_OB_STEALTHUNDEAD = "";
char    *s_OB_STEALTHSHOTGUY = "";
char    *s_OB_STEALTHZOMBIE = "";
char    *s_OB_UNDEADHIT = "";
char    *s_OB_IMPHIT = "";
char    *s_OB_CACOHIT = "";
char    *s_OB_DEMONHIT = "";
char    *s_OB_SPECTREHIT = "";
char    *s_OB_BARONHIT = "";
char    *s_OB_KNIGHTHIT = "";
char    *s_OB_ZOMBIE = "";
char    *s_OB_SHOTGUY = "";
char    *s_OB_VILE = "";
char    *s_OB_UNDEAD = "";
char    *s_OB_FATSO = "";
char    *s_OB_CHAINGUY = "";
char    *s_OB_SKULL = "";
char    *s_OB_IMP = "";
char    *s_OB_CACO = "";
char    *s_OB_BARON = "";
char    *s_OB_KNIGHT = "";
char    *s_OB_SPIDER = "";
char    *s_OB_BABY = "";
char    *s_OB_CYBORG = "";
char    *s_OB_WOLFSS = "";
char    *s_OB_MPFIST = "";
char    *s_OB_MPCHAINSAW = "";
char    *s_OB_MPPISTOL = "";
char    *s_OB_MPSHOTGUN = "";
char    *s_OB_MPSSHOTGUN = "";
char    *s_OB_MPCHAINGUN = "";
char    *s_OB_MPROCKET = "";
char    *s_OB_MPR_SPLASH = "";
char    *s_OB_MPPLASMARIFLE = "";
char    *s_OB_MPBFG_BOOM = "";
char    *s_OB_MPBFG_SPLASH = "";
char    *s_OB_MPTELEFRAG = "";
char    *s_OB_RAILGUN = "";
char    *s_OB_MPBFG_MBF = "";
char    *s_OB_MONTELEFRAG = "";
char    *s_OB_DEFAULT = "";
char    *s_OB_FRIENDLY1 = "";
char    *s_OB_FRIENDLY2 = "";
char    *s_OB_FRIENDLY3 = "";
char    *s_OB_FRIENDLY4 = "";
char    *s_OB_VOODOO = "";
char    *s_OB_MPDEFAULT = "";

// end d_deh.h variable declarations
// ====================================================================

// Do this for a lookup--the pointer (loaded above) is cross-referenced
// to a string key that is the same as the define above. We will use
// M_StringDuplicate to set these new values that we read from the file, orphaning
// the original value set above.

deh_strs deh_strlookup[] =
{
    { &s_VERSION,                     "VERSION"                     },

    { &s_D_DEVSTR,                    "D_DEVSTR"                    },
    { &s_D_CDROM,                     "D_CDROM"                     },

    { &s_PRESSKEY,                    "PRESSKEY"                    },
    { &s_PRESSYN,                     "PRESSYN"                     },
    { &s_PRESSA,                      "PRESSA"                      },
    { &s_QUITMSG,                     "QUITMSG"                     },
    { &s_QUITMSG1,                    "QUITMSG1"                    },
    { &s_QUITMSG2,                    "QUITMSG2"                    },
    { &s_QUITMSG3,                    "QUITMSG3"                    },
    { &s_QUITMSG4,                    "QUITMSG4"                    },
    { &s_QUITMSG5,                    "QUITMSG5"                    },
    { &s_QUITMSG6,                    "QUITMSG6"                    },
    { &s_QUITMSG7,                    "QUITMSG7"                    },
    { &s_QUITMSG8,                    "QUITMSG8"                    },
    { &s_QUITMSG9,                    "QUITMSG9"                    },
    { &s_QUITMSG10,                   "QUITMSG10"                   },
    { &s_QUITMSG11,                   "QUITMSG11"                   },
    { &s_QUITMSG12,                   "QUITMSG12"                   },
    { &s_QUITMSG13,                   "QUITMSG13"                   },
    { &s_QUITMSG14,                   "QUITMSG14"                   },
    { &s_LOADNET,                     "LOADNET"                     },
    { &s_QLOADNET,                    "QLOADNET"                    },
    { &s_QSAVESPOT,                   "QSAVESPOT"                   },
    { &s_SAVEDEAD,                    "SAVEDEAD"                    },
    { &s_QSPROMPT,                    "QSPROMPT"                    },
    { &s_QLPROMPT,                    "QLPROMPT"                    },
    { &s_DELPROMPT,                   "DELPROMPT"                   },
    { &s_NEWGAME,                     "NEWGAME"                     },
    { &s_NIGHTMARE,                   "NIGHTMARE"                   },
    { &s_SWSTRING,                    "SWSTRING"                    },
    { &s_MSGOFF,                      "MSGOFF"                      },
    { &s_MSGON,                       "MSGON"                       },
    { &s_NETEND,                      "NETEND"                      },
    { &s_ENDGAME,                     "ENDGAME"                     },
    { &s_DOSY,                        "DOSY"                        },
    { &s_DOSA,                        "DOSA"                        },
    { &s_DETAILHI,                    "DETAILHI"                    },
    { &s_DETAILLO,                    "DETAILLO"                    },
    { &s_GAMMALVL0,                   "GAMMALVL0"                   },
    { &s_GAMMALVL1,                   "GAMMALVL1"                   },
    { &s_GAMMALVL2,                   "GAMMALVL2"                   },
    { &s_GAMMALVL3,                   "GAMMALVL3"                   },
    { &s_GAMMALVL4,                   "GAMMALVL4"                   },
    { &s_GAMMALVL,                    "GAMMALVL"                    },
    { &s_GAMMAOFF,                    "GAMMAOFF"                    },
    { &s_EMPTYSTRING,                 "EMPTYSTRING"                 },

    { &s_GOTARMOR,                    "GOTARMOR"                    },
    { &s_GOTMEGA,                     "GOTMEGA"                     },
    { &s_GOTHTHBONUS,                 "GOTHTHBONUS"                 },
    { &s_GOTARMBONUS,                 "GOTARMBONUS"                 },
    { &s_GOTSTIM,                     "GOTSTIM"                     },
    { &s_GOTMEDINEED,                 "GOTMEDINEED"                 },
    { &s_GOTMEDIKIT,                  "GOTMEDIKIT"                  },
    { &s_GOTSUPER,                    "GOTSUPER"                    },

    { &s_GOTBLUECARD,                 "GOTBLUECARD"                 },
    { &s_GOTYELWCARD,                 "GOTYELWCARD"                 },
    { &s_GOTREDCARD,                  "GOTREDCARD"                  },
    { &s_GOTBLUESKUL,                 "GOTBLUESKUL"                 },
    { &s_GOTYELWSKUL,                 "GOTYELWSKUL"                 },
    { &s_GOTREDSKUL,                  "GOTREDSKUL"                  },
    { &s_GOTREDSKULL,                 "GOTREDSKULL"                 },

    { &s_GOTINVUL,                    "GOTINVUL"                    },
    { &s_GOTBERSERK,                  "GOTBERSERK"                  },
    { &s_GOTINVIS,                    "GOTINVIS"                    },
    { &s_GOTSUIT,                     "GOTSUIT"                     },
    { &s_GOTMAP,                      "GOTMAP"                      },
    { &s_GOTVISOR,                    "GOTVISOR"                    },

    { &s_GOTCLIP,                     "GOTCLIP"                     },
    { &s_GOTCLIPBOX,                  "GOTCLIPBOX"                  },
    { &s_GOTROCKET,                   "GOTROCKET"                   },
    { &s_GOTROCKETX2,                 "GOTROCKETX2"                 },
    { &s_GOTROCKBOX,                  "GOTROCKBOX"                  },
    { &s_GOTCELL,                     "GOTCELL"                     },
    { &s_GOTCELLX2,                   "GOTCELLX2"                   },
    { &s_GOTCELLBOX,                  "GOTCELLBOX"                  },
    { &s_GOTSHELLS,                   "GOTSHELLS"                   },
    { &s_GOTSHELLSX2,                 "GOTSHELLSX2"                 },
    { &s_GOTSHELLBOX,                 "GOTSHELLBOX"                 },
    { &s_GOTBACKPACK,                 "GOTBACKPACK"                 },

    { &s_GOTBFG9000,                  "GOTBFG9000"                  },
    { &s_GOTCHAINGUN,                 "GOTCHAINGUN"                 },
    { &s_GOTCHAINSAW,                 "GOTCHAINSAW"                 },
    { &s_GOTLAUNCHER,                 "GOTLAUNCHER"                 },
    { &s_GOTMSPHERE,                  "GOTMSPHERE"                  },
    { &s_GOTPLASMA,                   "GOTPLASMA"                   },
    { &s_GOTSHOTGUN,                  "GOTSHOTGUN"                  },
    { &s_GOTSHOTGUN2,                 "GOTSHOTGUN2"                 },

    { &s_PD_BLUEO,                    "PD_BLUEO"                    },
    { &s_PD_REDO,                     "PD_REDO"                     },
    { &s_PD_YELLOWO,                  "PD_YELLOWO"                  },
    { &s_PD_BLUEK,                    "PD_BLUEK"                    },
    { &s_PD_REDK,                     "PD_REDK"                     },
    { &s_PD_YELLOWK,                  "PD_YELLOWK"                  },
    { &s_PD_BLUEC,                    "PD_BLUEC"                    },
    { &s_PD_REDC,                     "PD_REDC"                     },
    { &s_PD_YELLOWC,                  "PD_YELLOWC"                  },
    { &s_PD_BLUES,                    "PD_BLUES"                    },
    { &s_PD_REDS,                     "PD_REDS"                     },
    { &s_PD_YELLOWS,                  "PD_YELLOWS"                  },
    { &s_PD_ANY,                      "PD_ANY"                      },
    { &s_PD_ALL3,                     "PD_ALL3"                     },
    { &s_PD_ALL6,                     "PD_ALL6"                     },
    { &s_PD_KEYCARD,                  "PD_KEYCARD"                  },
    { &s_PD_SKULLKEY,                 "PD_SKULLKEY"                 },
    { &s_PD_KEYCARDORSKULLKEY,        "PD_KEYCARDORSKULLKEY"        },
    { &s_PD_BLUECO,                   "PD_BLUECO"                   },
    { &s_PD_REDCO,                    "PD_REDCO"                    },
    { &s_PD_YELLOWCO,                 "PD_YELLOWCO"                 },

    { &s_SECRETMESSAGE,               "SECRETMESSAGE"               },

    { &s_GGSAVED,                     "GGSAVED"                     },
    { &s_GGAUTOSAVED,                 "GGAUTOSAVED"                 },
    { &s_GGLOADED,                    "GGLOADED"                    },
    { &s_GGAUTOLOADED,                "GGAUTOLOADED"                },
    { &s_GGDELETED,                   "GGDELETED"                   },
    { &s_GSCREENSHOT,                 "GSCREENSHOT"                 },

    { &s_ALWAYSRUNOFF,                "ALWAYSRUNOFF"                },
    { &s_ALWAYSRUNON,                 "ALWAYSRUNON"                 },

    { &s_HUSTR_MSGU,                  "HUSTR_MSGU"                  },

    { &s_HUSTR_E1M1,                  "HUSTR_E1M1"                  },
    { &s_HUSTR_E1M2,                  "HUSTR_E1M2"                  },
    { &s_HUSTR_E1M3,                  "HUSTR_E1M3"                  },
    { &s_HUSTR_E1M4,                  "HUSTR_E1M4"                  },
    { &s_HUSTR_E1M4B,                 "HUSTR_E1M4B"                 },
    { &s_HUSTR_E1M8B,                 "HUSTR_E1M8B"                 },
    { &s_HUSTR_E1M5,                  "HUSTR_E1M5"                  },
    { &s_HUSTR_E1M6,                  "HUSTR_E1M6"                  },
    { &s_HUSTR_E1M7,                  "HUSTR_E1M7"                  },
    { &s_HUSTR_E1M8,                  "HUSTR_E1M8"                  },
    { &s_HUSTR_E1M9,                  "HUSTR_E1M9"                  },
    { &s_HUSTR_E2M1,                  "HUSTR_E2M1"                  },
    { &s_HUSTR_E2M2,                  "HUSTR_E2M2"                  },
    { &s_HUSTR_E2M3,                  "HUSTR_E2M3"                  },
    { &s_HUSTR_E2M4,                  "HUSTR_E2M4"                  },
    { &s_HUSTR_E2M5,                  "HUSTR_E2M5"                  },
    { &s_HUSTR_E2M6,                  "HUSTR_E2M6"                  },
    { &s_HUSTR_E2M7,                  "HUSTR_E2M7"                  },
    { &s_HUSTR_E2M8,                  "HUSTR_E2M8"                  },
    { &s_HUSTR_E2M9,                  "HUSTR_E2M9"                  },
    { &s_HUSTR_E3M1,                  "HUSTR_E3M1"                  },
    { &s_HUSTR_E3M2,                  "HUSTR_E3M2"                  },
    { &s_HUSTR_E3M3,                  "HUSTR_E3M3"                  },
    { &s_HUSTR_E3M4,                  "HUSTR_E3M4"                  },
    { &s_HUSTR_E3M5,                  "HUSTR_E3M5"                  },
    { &s_HUSTR_E3M6,                  "HUSTR_E3M6"                  },
    { &s_HUSTR_E3M7,                  "HUSTR_E3M7"                  },
    { &s_HUSTR_E3M7_ALT,              "HUSTR_E3M7_ALT"              },
    { &s_HUSTR_E3M8,                  "HUSTR_E3M8"                  },
    { &s_HUSTR_E3M9,                  "HUSTR_E3M9"                  },
    { &s_HUSTR_E4M1,                  "HUSTR_E4M1"                  },
    { &s_HUSTR_E4M2,                  "HUSTR_E4M2"                  },
    { &s_HUSTR_E4M3,                  "HUSTR_E4M3"                  },
    { &s_HUSTR_E4M4,                  "HUSTR_E4M4"                  },
    { &s_HUSTR_E4M5,                  "HUSTR_E4M5"                  },
    { &s_HUSTR_E4M6,                  "HUSTR_E4M6"                  },
    { &s_HUSTR_E4M7,                  "HUSTR_E4M7"                  },
    { &s_HUSTR_E4M8,                  "HUSTR_E4M8"                  },
    { &s_HUSTR_E4M9,                  "HUSTR_E4M9"                  },
    { &s_HUSTR_E5M1,                  "HUSTR_E5M1"                  },
    { &s_HUSTR_E5M2,                  "HUSTR_E5M2"                  },
    { &s_HUSTR_E5M3,                  "HUSTR_E5M3"                  },
    { &s_HUSTR_E5M4,                  "HUSTR_E5M4"                  },
    { &s_HUSTR_E5M5,                  "HUSTR_E5M5"                  },
    { &s_HUSTR_E5M6,                  "HUSTR_E5M6"                  },
    { &s_HUSTR_E5M7,                  "HUSTR_E5M7"                  },
    { &s_HUSTR_E5M8,                  "HUSTR_E5M8"                  },
    { &s_HUSTR_E5M9,                  "HUSTR_E5M9"                  },
    { &s_HUSTR_1,                     "HUSTR_1"                     },
    { &s_HUSTR_2,                     "HUSTR_2"                     },
    { &s_HUSTR_3,                     "HUSTR_3"                     },
    { &s_HUSTR_4,                     "HUSTR_4"                     },
    { &s_HUSTR_5,                     "HUSTR_5"                     },
    { &s_HUSTR_6,                     "HUSTR_6"                     },
    { &s_HUSTR_7,                     "HUSTR_7"                     },
    { &s_HUSTR_8,                     "HUSTR_8"                     },
    { &s_HUSTR_9,                     "HUSTR_9"                     },
    { &s_HUSTR_10,                    "HUSTR_10"                    },
    { &s_HUSTR_11,                    "HUSTR_11"                    },
    { &s_HUSTR_11_ALT,                "HUSTR_11_ALT"                },
    { &s_HUSTR_12,                    "HUSTR_12"                    },
    { &s_HUSTR_13,                    "HUSTR_13"                    },
    { &s_HUSTR_14,                    "HUSTR_14"                    },
    { &s_HUSTR_15,                    "HUSTR_15"                    },
    { &s_HUSTR_16,                    "HUSTR_16"                    },
    { &s_HUSTR_17,                    "HUSTR_17"                    },
    { &s_HUSTR_18,                    "HUSTR_18"                    },
    { &s_HUSTR_19,                    "HUSTR_19"                    },
    { &s_HUSTR_20,                    "HUSTR_20"                    },
    { &s_HUSTR_21,                    "HUSTR_21"                    },
    { &s_HUSTR_22,                    "HUSTR_22"                    },
    { &s_HUSTR_23,                    "HUSTR_23"                    },
    { &s_HUSTR_24,                    "HUSTR_24"                    },
    { &s_HUSTR_25,                    "HUSTR_25"                    },
    { &s_HUSTR_26,                    "HUSTR_26"                    },
    { &s_HUSTR_27,                    "HUSTR_27"                    },
    { &s_HUSTR_28,                    "HUSTR_28"                    },
    { &s_HUSTR_29,                    "HUSTR_29"                    },
    { &s_HUSTR_30,                    "HUSTR_30"                    },
    { &s_HUSTR_31,                    "HUSTR_31"                    },
    { &s_HUSTR_31_BFG,                "HUSTR_31_BFG"                },
    { &s_HUSTR_32,                    "HUSTR_32"                    },
    { &s_HUSTR_32_BFG,                "HUSTR_32_BFG"                },
    { &s_HUSTR_33,                    "HUSTR_33"                    },
    { &s_PHUSTR_1,                    "PHUSTR_1"                    },
    { &s_PHUSTR_2,                    "PHUSTR_2"                    },
    { &s_PHUSTR_3,                    "PHUSTR_3"                    },
    { &s_PHUSTR_4,                    "PHUSTR_4"                    },
    { &s_PHUSTR_5,                    "PHUSTR_5"                    },
    { &s_PHUSTR_6,                    "PHUSTR_6"                    },
    { &s_PHUSTR_7,                    "PHUSTR_7"                    },
    { &s_PHUSTR_8,                    "PHUSTR_8"                    },
    { &s_PHUSTR_9,                    "PHUSTR_9"                    },
    { &s_PHUSTR_10,                   "PHUSTR_10"                   },
    { &s_PHUSTR_11,                   "PHUSTR_11"                   },
    { &s_PHUSTR_12,                   "PHUSTR_12"                   },
    { &s_PHUSTR_13,                   "PHUSTR_13"                   },
    { &s_PHUSTR_14,                   "PHUSTR_14"                   },
    { &s_PHUSTR_15,                   "PHUSTR_15"                   },
    { &s_PHUSTR_16,                   "PHUSTR_16"                   },
    { &s_PHUSTR_17,                   "PHUSTR_17"                   },
    { &s_PHUSTR_18,                   "PHUSTR_18"                   },
    { &s_PHUSTR_19,                   "PHUSTR_19"                   },
    { &s_PHUSTR_20,                   "PHUSTR_20"                   },
    { &s_PHUSTR_21,                   "PHUSTR_21"                   },
    { &s_PHUSTR_22,                   "PHUSTR_22"                   },
    { &s_PHUSTR_23,                   "PHUSTR_23"                   },
    { &s_PHUSTR_24,                   "PHUSTR_24"                   },
    { &s_PHUSTR_25,                   "PHUSTR_25"                   },
    { &s_PHUSTR_26,                   "PHUSTR_26"                   },
    { &s_PHUSTR_27,                   "PHUSTR_27"                   },
    { &s_PHUSTR_28,                   "PHUSTR_28"                   },
    { &s_PHUSTR_29,                   "PHUSTR_29"                   },
    { &s_PHUSTR_30,                   "PHUSTR_30"                   },
    { &s_PHUSTR_31,                   "PHUSTR_31"                   },
    { &s_PHUSTR_32,                   "PHUSTR_32"                   },
    { &s_THUSTR_1,                    "THUSTR_1"                    },
    { &s_THUSTR_2,                    "THUSTR_2"                    },
    { &s_THUSTR_3,                    "THUSTR_3"                    },
    { &s_THUSTR_4,                    "THUSTR_4"                    },
    { &s_THUSTR_5,                    "THUSTR_5"                    },
    { &s_THUSTR_6,                    "THUSTR_6"                    },
    { &s_THUSTR_7,                    "THUSTR_7"                    },
    { &s_THUSTR_8,                    "THUSTR_8"                    },
    { &s_THUSTR_9,                    "THUSTR_9"                    },
    { &s_THUSTR_10,                   "THUSTR_10"                   },
    { &s_THUSTR_11,                   "THUSTR_11"                   },
    { &s_THUSTR_12,                   "THUSTR_12"                   },
    { &s_THUSTR_13,                   "THUSTR_13"                   },
    { &s_THUSTR_14,                   "THUSTR_14"                   },
    { &s_THUSTR_15,                   "THUSTR_15"                   },
    { &s_THUSTR_16,                   "THUSTR_16"                   },
    { &s_THUSTR_17,                   "THUSTR_17"                   },
    { &s_THUSTR_18,                   "THUSTR_18"                   },
    { &s_THUSTR_19,                   "THUSTR_19"                   },
    { &s_THUSTR_20,                   "THUSTR_20"                   },
    { &s_THUSTR_21,                   "THUSTR_21"                   },
    { &s_THUSTR_22,                   "THUSTR_22"                   },
    { &s_THUSTR_23,                   "THUSTR_23"                   },
    { &s_THUSTR_24,                   "THUSTR_24"                   },
    { &s_THUSTR_25,                   "THUSTR_25"                   },
    { &s_THUSTR_26,                   "THUSTR_26"                   },
    { &s_THUSTR_27,                   "THUSTR_27"                   },
    { &s_THUSTR_28,                   "THUSTR_28"                   },
    { &s_THUSTR_29,                   "THUSTR_29"                   },
    { &s_THUSTR_30,                   "THUSTR_30"                   },
    { &s_THUSTR_31,                   "THUSTR_31"                   },
    { &s_THUSTR_32,                   "THUSTR_32"                   },
    { &s_NHUSTR_1,                    "NHUSTR_1"                    },
    { &s_NHUSTR_2,                    "NHUSTR_2"                    },
    { &s_NHUSTR_3,                    "NHUSTR_3"                    },
    { &s_NHUSTR_4,                    "NHUSTR_4"                    },
    { &s_NHUSTR_5,                    "NHUSTR_5"                    },
    { &s_NHUSTR_6,                    "NHUSTR_6"                    },
    { &s_NHUSTR_7,                    "NHUSTR_7"                    },
    { &s_NHUSTR_8,                    "NHUSTR_8"                    },
    { &s_NHUSTR_9,                    "NHUSTR_9"                    },

    { &s_HUSTR_CHATMACRO1,            "HUSTR_CHATMACRO1"            },
    { &s_HUSTR_CHATMACRO2,            "HUSTR_CHATMACRO2"            },
    { &s_HUSTR_CHATMACRO3,            "HUSTR_CHATMACRO3"            },
    { &s_HUSTR_CHATMACRO4,            "HUSTR_CHATMACRO4"            },
    { &s_HUSTR_CHATMACRO5,            "HUSTR_CHATMACRO5"            },
    { &s_HUSTR_CHATMACRO6,            "HUSTR_CHATMACRO6"            },
    { &s_HUSTR_CHATMACRO7,            "HUSTR_CHATMACRO7"            },
    { &s_HUSTR_CHATMACRO8,            "HUSTR_CHATMACRO8"            },
    { &s_HUSTR_CHATMACRO9,            "HUSTR_CHATMACRO9"            },
    { &s_HUSTR_CHATMACRO0,            "HUSTR_CHATMACRO0"            },
    { &s_HUSTR_TALKTOSELF1,           "HUSTR_TALKTOSELF1"           },
    { &s_HUSTR_TALKTOSELF2,           "HUSTR_TALKTOSELF2"           },
    { &s_HUSTR_TALKTOSELF3,           "HUSTR_TALKTOSELF3"           },
    { &s_HUSTR_TALKTOSELF4,           "HUSTR_TALKTOSELF4"           },
    { &s_HUSTR_TALKTOSELF5,           "HUSTR_TALKTOSELF5"           },
    { &s_HUSTR_MESSAGESENT,           "HUSTR_MESSAGESENT"           },
    { &s_HUSTR_PLRGREEN,              "HUSTR_PLRGREEN"              },
    { &s_HUSTR_PLRINDIGO,             "HUSTR_PLRINDIGO"             },
    { &s_HUSTR_PLRBROWN,              "HUSTR_PLRBROWN"              },
    { &s_HUSTR_PLRRED,                "HUSTR_PLRRED"                },

    { &s_AMSTR_FOLLOWON,              "AMSTR_FOLLOWON"              },
    { &s_AMSTR_FOLLOWOFF,             "AMSTR_FOLLOWOFF"             },
    { &s_AMSTR_GRIDON,                "AMSTR_GRIDON"                },
    { &s_AMSTR_GRIDOFF,               "AMSTR_GRIDOFF"               },
    { &s_AMSTR_MARKEDSPOT,            "AMSTR_MARKEDSPOT"            },
    { &s_AMSTR_MARKCLEARED,           "AMSTR_MARKCLEARED"           },
    { &s_AMSTR_MARKSCLEARED,          "AMSTR_MARKSCLEARED"          },
    { &s_AMSTR_ROTATEON,              "AMSTR_ROTATEON"              },
    { &s_AMSTR_ROTATEOFF,             "AMSTR_ROTATEOFF"             },

    { &s_STSTR_MUS,                   "STSTR_MUS"                   },
    { &s_STSTR_NOMUS,                 "STSTR_NOMUS"                 },
    { &s_STSTR_DQDON,                 "STSTR_DQDON"                 },
    { &s_STSTR_DQDOFF,                "STSTR_DQDOFF"                },
    { &s_STSTR_KFAADDED,              "STSTR_KFAADDED"              },
    { &s_STSTR_FAADDED,               "STSTR_FAADDED"               },
    { &s_STSTR_NCON,                  "STSTR_NCON"                  },
    { &s_STSTR_NCOFF,                 "STSTR_NCOFF"                 },
    { &s_STSTR_BEHOLD,                "STSTR_BEHOLD"                },
    { &s_STSTR_BEHOLDX,               "STSTR_BEHOLDX"               },
    { &s_STSTR_BEHOLDON,              "STSTR_BEHOLDON"              },
    { &s_STSTR_BEHOLDOFF,             "STSTR_BEHOLDOFF"             },
    { &s_STSTR_BUDDHAON,              "STSTR_BUDDHAON"              },
    { &s_STSTR_BUDDHAOFF,             "STSTR_BUDDHAOFF"             },
    { &s_STSTR_CHOPPERS,              "STSTR_CHOPPERS"              },
    { &s_STSTR_CLEV,                  "STSTR_CLEV"                  },
    { &s_STSTR_CLEVSAME,              "STSTR_CLEVSAME"              },
    { &s_STSTR_MYPOS,                 "STSTR_MYPOS"                 },
    { &s_STSTR_NTON,                  "STSTR_NTON"                  },
    { &s_STSTR_NTOFF,                 "STSTR_NTOFF"                 },
    { &s_STSTR_GODON,                 "STSTR_GODON"                 },
    { &s_STSTR_GODOFF,                "STSTR_GODOFF"                },
    { &s_STSTR_NMON,                  "STSTR_NMON"                  },
    { &s_STSTR_NMOFF,                 "STSTR_NMOFF"                 },
    { &s_STSTR_PSON,                  "STSTR_PSON"                  },
    { &s_STSTR_PSOFF,                 "STSTR_PSOFF"                 },
    { &s_STSTR_FMON,                  "STSTR_FMON"                  },
    { &s_STSTR_FMOFF,                 "STSTR_FMOFF"                 },
    { &s_STSTR_RION,                  "STSTR_RION"                  },
    { &s_STSTR_RIOFF,                 "STSTR_RIOFF"                 },
    { &s_STSTR_RMON,                  "STSTR_RMON"                  },
    { &s_STSTR_RMOFF,                 "STSTR_RMOFF"                 },
    { &s_STSTR_FON,                   "STSTR_FON"                   },
    { &s_STSTR_FOFF,                  "STSTR_FOFF"                  },
    { &s_STSTR_RHON,                  "STSTR_RHON"                  },
    { &s_STSTR_RHOFF,                 "STSTR_RHOFF"                 },
    { &s_STSTR_VON,                   "STSTR_VON"                   },
    { &s_STSTR_VOFF,                  "STSTR_VOFF"                  },
    { &s_STSTR_FPS,                   "STSTR_FPS"                   },
    { &s_STSTR_SUCKS,                 "STSTR_SUCKS"                 },
    { &s_STSTR_KILLS,                 "STSTR_KILLS"                 },
    { &s_STSTR_ITEMS,                 "STSTR_ITEMS"                 },
    { &s_STSTR_SECRETS,               "STSTR_SECRETS"               },

    { &s_E1TEXT,                      "E1TEXT"                      },
    { &s_E2TEXT,                      "E2TEXT"                      },
    { &s_E3TEXT,                      "E3TEXT"                      },
    { &s_E4TEXT,                      "E4TEXT"                      },
    { &s_E5TEXT,                      "E5TEXT"                      },
    { &s_C1TEXT,                      "C1TEXT"                      },
    { &s_C2TEXT,                      "C2TEXT"                      },
    { &s_C3TEXT,                      "C3TEXT"                      },
    { &s_C4TEXT,                      "C4TEXT"                      },
    { &s_C5TEXT,                      "C5TEXT"                      },
    { &s_C6TEXT,                      "C6TEXT"                      },
    { &s_P1TEXT,                      "P1TEXT"                      },
    { &s_P2TEXT,                      "P2TEXT"                      },
    { &s_P3TEXT,                      "P3TEXT"                      },
    { &s_P4TEXT,                      "P4TEXT"                      },
    { &s_P5TEXT,                      "P5TEXT"                      },
    { &s_P6TEXT,                      "P6TEXT"                      },
    { &s_T1TEXT,                      "T1TEXT"                      },
    { &s_T2TEXT,                      "T2TEXT"                      },
    { &s_T3TEXT,                      "T3TEXT"                      },
    { &s_T4TEXT,                      "T4TEXT"                      },
    { &s_T5TEXT,                      "T5TEXT"                      },
    { &s_T6TEXT,                      "T6TEXT"                      },
    { &s_N1TEXT,                      "N1TEXT"                      },

    { &s_CC_ZOMBIE,                   "CC_ZOMBIE"                   },
    { &s_CC_SHOTGUN,                  "CC_SHOTGUN"                  },
    { &s_CC_HEAVY,                    "CC_HEAVY"                    },
    { &s_CC_IMP,                      "CC_IMP"                      },
    { &s_CC_DEMON,                    "CC_DEMON"                    },
    { &s_CC_SPECTRE,                  "CC_SPECTRE"                  },
    { &s_CC_LOST,                     "CC_LOST"                     },
    { &s_CC_CACO,                     "CC_CACO"                     },
    { &s_CC_HELL,                     "CC_HELL"                     },
    { &s_CC_BARON,                    "CC_BARON"                    },
    { &s_CC_ARACH,                    "CC_ARACH"                    },
    { &s_CC_PAIN,                     "CC_PAIN"                     },
    { &s_CC_REVEN,                    "CC_REVEN"                    },
    { &s_CC_MANCU,                    "CC_MANCU"                    },
    { &s_CC_ARCH,                     "CC_ARCH"                     },
    { &s_CC_SPIDER,                   "CC_SPIDER"                   },
    { &s_CC_CYBER,                    "CC_CYBER"                    },
    { &s_CC_HERO,                     "CC_HERO"                     },

    { &s_M_NEWGAME,                   "M_NEWGAME"                   },
    { &s_M_OPTIONS,                   "M_OPTIONS"                   },
    { &s_M_LOADGAME,                  "M_LOADGAME"                  },
    { &s_M_SAVEGAME,                  "M_SAVEGAME"                  },
    { &s_M_QUITGAME,                  "M_QUITGAME"                  },
    { &s_M_WHICHEPISODE,              "M_WHICHEPISODE"              },
    { &s_M_EPISODE1,                  "M_EPISODE1"                  },
    { &s_M_EPISODE2,                  "M_EPISODE2"                  },
    { &s_M_EPISODE3,                  "M_EPISODE3"                  },
    { &s_M_EPISODE4,                  "M_EPISODE4"                  },
    { &s_M_EPISODE5,                  "M_EPISODE5"                  },
    { &s_M_WHICHEXPANSION,            "M_WHICHEXPANSION"            },
    { &s_M_EXPANSION1,                "M_EXPANSION1"                },
    { &s_M_EXPANSION2,                "M_EXPANSION2"                },
    { &s_M_CHOOSESKILLLEVEL,          "M_CHOOSESKILLLEVEL"          },
    { &s_M_SKILLLEVEL1,               "M_SKILLLEVEL1"               },
    { &s_M_SKILLLEVEL2,               "M_SKILLLEVEL2"               },
    { &s_M_SKILLLEVEL3,               "M_SKILLLEVEL3"               },
    { &s_M_SKILLLEVEL4,               "M_SKILLLEVEL4"               },
    { &s_M_SKILLLEVEL5,               "M_SKILLLEVEL5"               },
    { &s_M_ENDGAME,                   "M_ENDGAME"                   },
    { &s_M_MESSAGES,                  "M_MESSAGES"                  },
    { &s_M_ON,                        "M_ON"                        },
    { &s_M_OFF,                       "M_OFF"                       },
    { &s_M_GRAPHICDETAIL,             "M_GRAPHICDETAIL"             },
    { &s_M_HIGH,                      "M_HIGH"                      },
    { &s_M_LOW,                       "M_LOW"                       },
    { &s_M_SCREENSIZE,                "M_SCREENSIZE"                },
    { &s_M_MOUSESENSITIVITY,          "M_MOUSESENSITIVITY"          },
    { &s_M_GAMECONTROLLERSENSITIVITY, "M_GAMECONTROLLERSENSITIVITY" },
    { &s_M_SOUNDVOLUME,               "M_SOUNDVOLUME"               },
    { &s_M_SFXVOLUME,                 "M_SFXVOLUME"                 },
    { &s_M_MUSICVOLUME,               "M_MUSICVOLUME"               },
    { &s_M_PAUSED,                    "M_PAUSED"                    },
    { &s_M_MORE,                      "M_MORE"                      },

    { &s_CAPTION_SHAREWARE,           "CAPTION_SHAREWARE"           },
    { &s_CAPTION_REGISTERED,          "CAPTION_REGISTERED"          },
    { &s_CAPTION_ULTIMATE,            "CAPTION_ULTIMATE"            },
    { &s_CAPTION_DOOM2,               "CAPTION_DOOM2"               },
    { &s_CAPTION_HELLONEARTH,         "CAPTION_HELLONEARTH"         },
    { &s_CAPTION_NERVE,               "CAPTION_NERVE"               },
    { &s_CAPTION_BFGEDITION,          "CAPTION_BFGEDITION"          },
    { &s_CAPTION_PLUTONIA,            "CAPTION_PLUTONIA"            },
    { &s_CAPTION_TNT,                 "CAPTION_TNT"                 },
    { &s_CAPTION_CHEX,                "CAPTION_CHEX"                },
    { &s_CAPTION_CHEX2,               "CAPTION_CHEX2"               },
    { &s_CAPTION_HACX,                "CAPTION_HACX"                },
    { &s_CAPTION_FREEDOOM1,           "CAPTION_FREEDOOM1"           },
    { &s_CAPTION_FREEDOOM2,           "CAPTION_FREEDOOM2"           },
    { &s_CAPTION_FREEDM,              "CAPTION_FREEDM"              },
    { &s_CAPTION_BTSXE1,              "CAPTION_BTSXE1"              },
    { &s_CAPTION_BTSXE2,              "CAPTION_BTSXE2"              },
    { &s_CAPTION_REKKR,               "CAPTION_REKKR"               },
    { &s_CAPTION_REKKRSL,             "CAPTION_REKKRSL"             },
    { &s_CAPTION_ANOMALYREPORT,       "CAPTION_ANOMALYREPORT"       },
    { &s_CAPTION_ARRIVAL,             "CAPTION_ARRIVAL"             },
    { &s_CAPTION_DBIMPACT,            "CAPTION_DBIMPACT"            },
    { &s_CAPTION_DEATHLESS,           "CAPTION_DEATHLESS"           },
    { &s_CAPTION_DOOMZERO,            "CAPTION_DOOMZERO"            },
    { &s_CAPTION_EARTHLESS,           "CAPTION_EARTHLESS"           },
    { &s_CAPTION_HARMONY,             "CAPTION_HARMONY"             },
    { &s_CAPTION_NEIS,                "CAPTION_NEIS"                },
    { &s_CAPTION_REVOLUTION,          "CAPTION_REVOLUTION"          },
    { &s_CAPTION_SYRINGE,             "CAPTION_SYRINGE"             },

    { &s_AUTHOR_ROMERO,               "AUTHOR_ROMERO"               },

    { &bgflatE1,                      "BGFLATE1"                    },
    { &bgflatE2,                      "BGFLATE2"                    },
    { &bgflatE3,                      "BGFLATE3"                    },
    { &bgflatE4,                      "BGFLATE4"                    },
    { &bgflatE5,                      "BGFLATE5"                    },
    { &bgflat06,                      "BGFLAT06"                    },
    { &bgflat11,                      "BGFLAT11"                    },
    { &bgflat15,                      "BGFLAT15"                    },
    { &bgflat20,                      "BGFLAT20"                    },
    { &bgflat30,                      "BGFLAT30"                    },
    { &bgflat31,                      "BGFLAT31"                    },
    { &bgcastcall,                    "BGCASTCALL"                  },

    // Ty 04/08/98 - added 5 general purpose startup announcement
    // strings for hacker use.
    { &startup1,                      "STARTUP1"                    },
    { &startup2,                      "STARTUP2"                    },
    { &startup3,                      "STARTUP3"                    },
    { &startup4,                      "STARTUP4"                    },
    { &startup5,                      "STARTUP5"                    },

    { &savegamename,                  "SAVEGAMENAME"                },

    { &s_BANNER1,                     "BANNER1"                     },
    { &s_BANNER2,                     "BANNER2"                     },
    { &s_BANNER3,                     "BANNER3"                     },
    { &s_BANNER4,                     "BANNER4"                     },
    { &s_BANNER5,                     "BANNER5"                     },
    { &s_BANNER6,                     "BANNER6"                     },
    { &s_BANNER7,                     "BANNER7"                     },

    { &s_COPYRIGHT1,                  "COPYRIGHT1"                  },
    { &s_COPYRIGHT2,                  "COPYRIGHT2"                  },
    { &s_COPYRIGHT3,                  "COPYRIGHT3"                  },

    { &s_OB_SUICIDE,                  "OB_SUICIDE"                  },
    { &s_OB_FALLING,                  "OB_FALLING"                  },
    { &s_OB_CRUSH,                    "OB_CRUSH"                    },
    { &s_OB_EXIT,                     "OB_EXIT"                     },
    { &s_OB_WATER,                    "OB_WATER"                    },
    { &s_OB_SLIME,                    "OB_SLIME"                    },
    { &s_OB_LAVA,                     "OB_LAVA"                     },
    { &s_OB_BARREL,                   "OB_BARREL"                   },
    { &s_OB_SPLASH,                   "OB_SPLASH"                   },
    { &s_OB_R_SPLASH,                 "OB_R_SPLASH"                 },
    { &s_OB_ROCKET,                   "OB_ROCKET"                   },
    { &s_OB_KILLEDSELF,               "OB_KILLEDSELF"               },
    { &s_OB_STEALTHBABY,              "OB_STEALTHBABY"              },
    { &s_OB_STEALTHVILE,              "OB_STEALTHVILE"              },
    { &s_OB_STEALTHBARON,             "OB_STEALTHBARON"             },
    { &s_OB_STEALTHCACO,              "OB_STEALTHCACO"              },
    { &s_OB_STEALTHCHAINGUY,          "OB_STEALTHCHAINGUY"          },
    { &s_OB_STEALTHDEMON,             "OB_STEALTHDEMON"             },
    { &s_OB_STEALTHKNIGHT,            "OB_STEALTHKNIGHT"            },
    { &s_OB_STEALTHIMP,               "OB_STEALTHIMP"               },
    { &s_OB_STEALTHFATSO,             "OB_STEALTHFATSO"             },
    { &s_OB_STEALTHUNDEAD,            "OB_STEALTHUNDEAD"            },
    { &s_OB_STEALTHSHOTGUY,           "OB_STEALTHSHOTGUY"           },
    { &s_OB_STEALTHZOMBIE,            "OB_STEALTHZOMBIE"            },
    { &s_OB_UNDEADHIT,                "OB_UNDEADHIT"                },
    { &s_OB_IMPHIT,                   "OB_IMPHIT"                   },
    { &s_OB_CACOHIT,                  "OB_CACOHIT"                  },
    { &s_OB_DEMONHIT,                 "OB_DEMONHIT"                 },
    { &s_OB_SPECTREHIT,               "OB_SPECTREHIT"               },
    { &s_OB_BARONHIT,                 "OB_BARONHIT"                 },
    { &s_OB_KNIGHTHIT,                "OB_KNIGHTHIT"                },
    { &s_OB_ZOMBIE,                   "OB_ZOMBIE"                   },
    { &s_OB_SHOTGUY,                  "OB_SHOTGUY"                  },
    { &s_OB_VILE,                     "OB_VILE"                     },
    { &s_OB_UNDEAD,                   "OB_UNDEAD"                   },
    { &s_OB_FATSO,                    "OB_FATSO"                    },
    { &s_OB_CHAINGUY,                 "OB_CHAINGUY"                 },
    { &s_OB_SKULL,                    "OB_SKULL"                    },
    { &s_OB_IMP,                      "OB_IMP"                      },
    { &s_OB_CACO,                     "OB_CACO"                     },
    { &s_OB_BARON,                    "OB_BARON"                    },
    { &s_OB_KNIGHT,                   "OB_KNIGHT"                   },
    { &s_OB_SPIDER,                   "OB_SPIDER"                   },
    { &s_OB_BABY,                     "OB_BABY"                     },
    { &s_OB_CYBORG,                   "OB_CYBORG"                   },
    { &s_OB_WOLFSS,                   "OB_WOLFSS"                   },
    { &s_OB_MPFIST,                   "OB_MPFIST"                   },
    { &s_OB_MPCHAINSAW,               "OB_MPCHAINSAW"               },
    { &s_OB_MPPISTOL,                 "OB_MPPISTOL"                 },
    { &s_OB_MPSHOTGUN,                "OB_MPSHOTGUN"                },
    { &s_OB_MPSSHOTGUN,               "OB_MPSSHOTGUN"               },
    { &s_OB_MPCHAINGUN,               "OB_MPCHAINGUN"               },
    { &s_OB_MPROCKET,                 "OB_MPROCKET"                 },
    { &s_OB_MPR_SPLASH,               "OB_MPR_SPLASH"               },
    { &s_OB_MPPLASMARIFLE,            "OB_MPPLASMARIFLE"            },
    { &s_OB_MPBFG_BOOM,               "OB_MPBFG_BOOM"               },
    { &s_OB_MPBFG_SPLASH,             "OB_MPBFG_SPLASH"             },
    { &s_OB_MPTELEFRAG,               "OB_MPTELEFRAG"               },
    { &s_OB_RAILGUN,                  "OB_RAILGUN"                  },
    { &s_OB_MPBFG_MBF,                "OB_MPBFG_MBF"                },
    { &s_OB_MONTELEFRAG,              "OB_MONTELEFRAG"              },
    { &s_OB_DEFAULT,                  "OB_DEFAULT"                  },
    { &s_OB_FRIENDLY1,                "OB_FRIENDLY1"                },
    { &s_OB_FRIENDLY2,                "OB_FRIENDLY2"                },
    { &s_OB_FRIENDLY3,                "OB_FRIENDLY3"                },
    { &s_OB_FRIENDLY4,                "OB_FRIENDLY4"                },
    { &s_OB_VOODOO,                   "OB_VOODOO"                   },
    { &s_OB_MPDEFAULT,                "OB_MPDEFAULT"                }
};

static const int    deh_numstrlookup = sizeof(deh_strlookup) / sizeof(deh_strlookup[0]);

// DOOM shareware/registered/retail (Ultimate) names.
char **mapnames[] =
{
    &s_HUSTR_E1M1,
    &s_HUSTR_E1M2,
    &s_HUSTR_E1M3,
    &s_HUSTR_E1M4,
    &s_HUSTR_E1M5,
    &s_HUSTR_E1M6,
    &s_HUSTR_E1M7,
    &s_HUSTR_E1M8,
    &s_HUSTR_E1M9,
    &s_HUSTR_E2M1,
    &s_HUSTR_E2M2,
    &s_HUSTR_E2M3,
    &s_HUSTR_E2M4,
    &s_HUSTR_E2M5,
    &s_HUSTR_E2M6,
    &s_HUSTR_E2M7,
    &s_HUSTR_E2M8,
    &s_HUSTR_E2M9,
    &s_HUSTR_E3M1,
    &s_HUSTR_E3M2,
    &s_HUSTR_E3M3,
    &s_HUSTR_E3M4,
    &s_HUSTR_E3M5,
    &s_HUSTR_E3M6,
    &s_HUSTR_E3M7,
    &s_HUSTR_E3M8,
    &s_HUSTR_E3M9,
    &s_HUSTR_E4M1,
    &s_HUSTR_E4M2,
    &s_HUSTR_E4M3,
    &s_HUSTR_E4M4,
    &s_HUSTR_E4M5,
    &s_HUSTR_E4M6,
    &s_HUSTR_E4M7,
    &s_HUSTR_E4M8,
    &s_HUSTR_E4M9,
    &s_HUSTR_E5M1,
    &s_HUSTR_E5M2,
    &s_HUSTR_E5M3,
    &s_HUSTR_E5M4,
    &s_HUSTR_E5M5,
    &s_HUSTR_E5M6,
    &s_HUSTR_E5M7,
    &s_HUSTR_E5M8,
    &s_HUSTR_E5M9
};

// DOOM 2 map names.
char **mapnames2[] =
{
    &s_HUSTR_1,
    &s_HUSTR_2,
    &s_HUSTR_3,
    &s_HUSTR_4,
    &s_HUSTR_5,
    &s_HUSTR_6,
    &s_HUSTR_7,
    &s_HUSTR_8,
    &s_HUSTR_9,
    &s_HUSTR_10,
    &s_HUSTR_11,
    &s_HUSTR_12,
    &s_HUSTR_13,
    &s_HUSTR_14,
    &s_HUSTR_15,
    &s_HUSTR_16,
    &s_HUSTR_17,
    &s_HUSTR_18,
    &s_HUSTR_19,
    &s_HUSTR_20,
    &s_HUSTR_21,
    &s_HUSTR_22,
    &s_HUSTR_23,
    &s_HUSTR_24,
    &s_HUSTR_25,
    &s_HUSTR_26,
    &s_HUSTR_27,
    &s_HUSTR_28,
    &s_HUSTR_29,
    &s_HUSTR_30,
    &s_HUSTR_31,
    &s_HUSTR_32
};

// DOOM 2 map names.
char **mapnames2_bfg[] =
{
    &s_HUSTR_1,
    &s_HUSTR_2,
    &s_HUSTR_3,
    &s_HUSTR_4,
    &s_HUSTR_5,
    &s_HUSTR_6,
    &s_HUSTR_7,
    &s_HUSTR_8,
    &s_HUSTR_9,
    &s_HUSTR_10,
    &s_HUSTR_11,
    &s_HUSTR_12,
    &s_HUSTR_13,
    &s_HUSTR_14,
    &s_HUSTR_15,
    &s_HUSTR_16,
    &s_HUSTR_17,
    &s_HUSTR_18,
    &s_HUSTR_19,
    &s_HUSTR_20,
    &s_HUSTR_21,
    &s_HUSTR_22,
    &s_HUSTR_23,
    &s_HUSTR_24,
    &s_HUSTR_25,
    &s_HUSTR_26,
    &s_HUSTR_27,
    &s_HUSTR_28,
    &s_HUSTR_29,
    &s_HUSTR_30,
    &s_HUSTR_31,
    &s_HUSTR_32,
    &s_HUSTR_33
};

// Plutonia WAD map names.
char **mapnamesp[] =
{
    &s_PHUSTR_1,
    &s_PHUSTR_2,
    &s_PHUSTR_3,
    &s_PHUSTR_4,
    &s_PHUSTR_5,
    &s_PHUSTR_6,
    &s_PHUSTR_7,
    &s_PHUSTR_8,
    &s_PHUSTR_9,
    &s_PHUSTR_10,
    &s_PHUSTR_11,
    &s_PHUSTR_12,
    &s_PHUSTR_13,
    &s_PHUSTR_14,
    &s_PHUSTR_15,
    &s_PHUSTR_16,
    &s_PHUSTR_17,
    &s_PHUSTR_18,
    &s_PHUSTR_19,
    &s_PHUSTR_20,
    &s_PHUSTR_21,
    &s_PHUSTR_22,
    &s_PHUSTR_23,
    &s_PHUSTR_24,
    &s_PHUSTR_25,
    &s_PHUSTR_26,
    &s_PHUSTR_27,
    &s_PHUSTR_28,
    &s_PHUSTR_29,
    &s_PHUSTR_30,
    &s_PHUSTR_31,
    &s_PHUSTR_32
};

// TNT WAD map names.
char **mapnamest[] =
{
    &s_THUSTR_1,
    &s_THUSTR_2,
    &s_THUSTR_3,
    &s_THUSTR_4,
    &s_THUSTR_5,
    &s_THUSTR_6,
    &s_THUSTR_7,
    &s_THUSTR_8,
    &s_THUSTR_9,
    &s_THUSTR_10,
    &s_THUSTR_11,
    &s_THUSTR_12,
    &s_THUSTR_13,
    &s_THUSTR_14,
    &s_THUSTR_15,
    &s_THUSTR_16,
    &s_THUSTR_17,
    &s_THUSTR_18,
    &s_THUSTR_19,
    &s_THUSTR_20,
    &s_THUSTR_21,
    &s_THUSTR_22,
    &s_THUSTR_23,
    &s_THUSTR_24,
    &s_THUSTR_25,
    &s_THUSTR_26,
    &s_THUSTR_27,
    &s_THUSTR_28,
    &s_THUSTR_29,
    &s_THUSTR_30,
    &s_THUSTR_31,
    &s_THUSTR_32
};

// Nerve WAD map names.
char **mapnamesn[] =
{
    &s_NHUSTR_1,
    &s_NHUSTR_2,
    &s_NHUSTR_3,
    &s_NHUSTR_4,
    &s_NHUSTR_5,
    &s_NHUSTR_6,
    &s_NHUSTR_7,
    &s_NHUSTR_8,
    &s_NHUSTR_9
};

const int   nummapnames = sizeof(mapnames) / sizeof(mapnames[0]);
const int   nummapnames2 = sizeof(mapnames2) / sizeof(mapnames2[0]);
const int   nummapnames2_bfg = sizeof(mapnames2_bfg) / sizeof(mapnames2_bfg[0]);
const int   nummapnamesp = sizeof(mapnamesp) / sizeof(mapnamesp[0]);
const int   nummapnamest = sizeof(mapnamest) / sizeof(mapnamest[0]);
const int   nummapnamesn = sizeof(mapnamesn) / sizeof(mapnamesn[0]);

// Function prototypes
static void lfstrip(char *s);       // strip the \r and/or \n off of a line
static void rstrip(char *s);        // strip trailing whitespace
static char *ptr_lstrip(char *p);   // point past leading whitespace
static int deh_GetData(char *s, char *k, int *l, char **strval);
static bool deh_procStringSub(char *key, char *lookfor, char *newstring);
static char *dehReformatStr(char *string);

// Prototypes for block processing functions
// Pointers to these functions are used as the blocks are encountered.
static void deh_procThing(DEHFILE *fpin, char *line);
static void deh_procFrame(DEHFILE *fpin, char *line);
static void deh_procPointer(DEHFILE *fpin, char *line);
static void deh_procSounds(DEHFILE *fpin, char *line);
static void deh_procAmmo(DEHFILE *fpin, char *line);
static void deh_procWeapon(DEHFILE *fpin, char *line);
static void deh_procSprite(DEHFILE *fpin, char *line);
static void deh_procCheat(DEHFILE *fpin, char *line);
static void deh_procMisc(DEHFILE *fpin, char *line);
static void deh_procText(DEHFILE *fpin, char *line);
static void deh_procPars(DEHFILE *fpin, char *line);
static void deh_procStrings(DEHFILE *fpin, char *line);
static void deh_procError(DEHFILE *fpin, char *line);
static void deh_procBexCodePointers(DEHFILE *fpin, char *line);

// haleyjd: handlers to fully deprecate the DeHackEd text section
static void deh_procBexSounds(DEHFILE *fpin, char *line);
static void deh_procBexMusic(DEHFILE *fpin, char *line);
static void deh_procBexSprites(DEHFILE *fpin, char *line);

// Structure deh_block is used to hold the block names that can
// be encountered, and the routines to use to decipher them
typedef struct
{
    char    *key;                                       // a mnemonic block code name
    void    (*const fptr)(DEHFILE *, char *);           // handler
} deh_block;

#define DEH_BUFFERMAX   1024                            // input buffer area size, hardcoded for now
// killough 08/09/98: make DEH_BLOCKMAX self-adjusting
#define DEH_BLOCKMAX    arrlen(deh_blocks)              // size of array
#define DEH_MAXKEYLEN   32                              // as much of any key as we'll look at
#define DEH_MOBJINFOMAX 39                              // number of mobjinfo configuration keys

// Put all the block header values, and the function to be called when that
// one is encountered, in this array:
static const deh_block deh_blocks[] =
{
    /*  0 */ { "Thing",     deh_procThing           },
    /*  1 */ { "Frame",     deh_procFrame           },
    /*  2 */ { "Pointer",   deh_procPointer         },
    /*  3 */ { "Sound",     deh_procSounds          },  // Ty 03/16/98 corrected from "Sounds"
    /*  4 */ { "Ammo",      deh_procAmmo            },
    /*  5 */ { "Weapon",    deh_procWeapon          },
    /*  6 */ { "Sprite",    deh_procSprite          },
    /*  7 */ { "Cheat",     deh_procCheat           },
    /*  8 */ { "Misc",      deh_procMisc            },
    /*  9 */ { "Text",      deh_procText            },  // --  end of standard "deh" entries,

    // begin BOOM Extensions (BEX)
    /* 10 */ { "[STRINGS]", deh_procStrings         },  // new string changes
    /* 11 */ { "[PARS]",    deh_procPars            },  // alternative block marker
    /* 12 */ { "[CODEPTR]", deh_procBexCodePointers },  // bex codepointers by mnemonic
    /* 14 */ { "[SPRITES]", deh_procBexSprites      },  // bex style sprites
    /* 15 */ { "[SOUNDS]",  deh_procBexSounds       },  // bex style sounds
    /* 16 */ { "[MUSIC]",   deh_procBexMusic        },  // bex style music
    /* 13 */ { "",          deh_procError           }   // dummy to handle anything else
};

// flag to skip included deh-style text, used with INCLUDE NOTEXT directive
static bool includenotext;

// MOBJINFO - Dehacked block name = "Thing"
// Usage: Thing nn (name)
// These are for mobjinfo_t types. Each is an integer
// within the structure, so we can use index of the string in this
// array to offset by sizeof(int) into the mobjinfo_t array at [nn]
// * things are base zero but dehacked considers them to start at #1.
static const char *deh_mobjinfo[DEH_MOBJINFOMAX] =
{
    "ID #",                     // .doomednum
    "Initial frame",            // .spawnstate
    "Hit points",               // .spawnhealth
    "Gib health",               // .gibhealth
    "First moving frame",       // .seestate
    "Alert sound",              // .seesound
    "Reaction time",            // .reactiontime
    "Attack sound",             // .attacksound
    "Injury frame",             // .painstate
    "Pain chance",              // .painchance
    "Pain sound",               // .painsound
    "Close attack frame",       // .meleestate
    "Far attack frame",         // .missilestate
    "Death frame",              // .deathstate
    "Exploding frame",          // .xdeathstate
    "Death sound",              // .deathsound
    "Dropped item",             // .droppeditem
    "Speed",                    // .speed
    "Width",                    // .radius
    "Pickup width",             // .pickupradius
    "Height",                   // .height
    "Projectile pass height",   // .projectilepassheight
    "Mass",                     // .mass
    "Missile damage",           // .damage
    "Action sound",             // .activesound
    "Bits",                     // .flags
    "Retro bits",               // .flags2
    "Respawn frame",            // .raisestate
    "Frames",                   // .frames
    "Fullbright",               // .fullbright
    "Blood color",              // .bloodcolor
    "Shadow offset",            // .shadowoffset

    // MBF21
    "MBF21 bits",               // .mbf21flags
    "Infighting group",         // .infightinggroup
    "Projectile group",         // .projectilegroup
    "Splash group",             // .splashgroup
    "Rip sound",                // .ripsound
    "Fast speed",               // .altspeed
    "Melee range"               // .meleerange
};

// Strings that are used to indicate flags ("Bits" in mobjinfo)
// This is an array of bit masks that are related to p_mobj.h
// values, using the same names without the MF_ in front.
// Ty 08/27/98 new code
//
// killough 10/98:
//
// Convert array to struct to allow multiple values, make array size variable
#define DEH_MOBJFLAGMAX     arrlen(deh_mobjflags)
#define DEH_MOBJFLAG2MAX    arrlen(deh_mobjflags2)

struct deh_flag_s
{
    const char  *name;
    int         value;
};

static const struct deh_flag_s deh_mobjflags[] =
{
    { "SPECIAL",      MF_SPECIAL      },    // call P_Specialthing when touched
    { "SOLID",        MF_SOLID        },    // block movement
    { "SHOOTABLE",    MF_SHOOTABLE    },    // can be hit
    { "NOSECTOR",     MF_NOSECTOR     },    // invisible but touchable
    { "NOBLOCKMAP",   MF_NOBLOCKMAP   },    // inert but displayable
    { "AMBUSH",       MF_AMBUSH       },    // deaf monster
    { "JUSTHIT",      MF_JUSTHIT      },    // will try to attack right back
    { "JUSTATTACKED", MF_JUSTATTACKED },    // take at least 1 step before attacking
    { "SPAWNCEILING", MF_SPAWNCEILING },    // initially hang from ceiling
    { "NOGRAVITY",    MF_NOGRAVITY    },    // don't apply gravity during play
    { "DROPOFF",      MF_DROPOFF      },    // can jump from high places
    { "PICKUP",       MF_PICKUP       },    // will pick up items
    { "NOCLIP",       MF_NOCLIP       },    // goes through walls
    { "SLIDE",        MF_SLIDE        },    // keep info about sliding along walls
    { "FLOAT",        MF_FLOAT        },    // allow movement to any height
    { "TELEPORT",     MF_TELEPORT     },    // don't cross lines or look at heights
    { "MISSILE",      MF_MISSILE      },    // don't hit same species, explode on block
    { "DROPPED",      MF_DROPPED      },    // dropped, not spawned (like ammo clip)
    { "SHADOW",       MF_FUZZ         },    // use fuzzy draw like spectres
    { "NOBLOOD",      MF_NOBLOOD      },    // puffs instead of blood when shot
    { "CORPSE",       MF_CORPSE       },    // so it will slide down steps when dead
    { "INFLOAT",      MF_INFLOAT      },    // float but not to target height
    { "COUNTKILL",    MF_COUNTKILL    },    // count toward the kills total
    { "COUNTITEM",    MF_COUNTITEM    },    // count toward the items total
    { "SKULLFLY",     MF_SKULLFLY     },    // special handling for flying skulls
    { "NOTDMATCH",    MF_NOTDMATCH    },    // do not spawn in deathmatch

    // killough 10/98: TRANSLATION consists of 2 bits, not 1:
    { "TRANSLATION",  0x04000000      },    // for BOOM bug-compatibility
    { "TRANSLATION1", 0x04000000      },    // use translation table for color (players)
    { "TRANSLATION2", 0x08000000      },    // use translation table for color (players)

    { "UNUSED1",      0x08000000      },    // unused bit # 1 -- For BOOM bug-compatibility
    { "UNUSED2",      0x10000000      },    // unused bit # 2 -- For BOOM compatibility
    { "UNUSED3",      0x20000000      },    // unused bit # 3 -- For BOOM compatibility
    { "UNUSED4",      0x40000000      },    // unused bit # 4 -- For BOOM compatibility

    { "TOUCHY",       MF_TOUCHY       },    // dies on contact with solid objects (MBF)
    { "BOUNCES",      MF_BOUNCES      },    // bounces off floors, ceilings and maybe walls
    { "FRIEND",       MF_FRIEND       },    // a friend of the player (MBF)
    { "TRANSLUCENT",  MF_TRANSLUCENT  }     // apply translucency to sprite (BOOM)
};

static const struct deh_flag_s deh_mobjflags2[] =
{
    { "TRANSLUCENT",               MF2_TRANSLUCENT               },
    { "TRANSLUCENT_REDONLY",       MF2_TRANSLUCENT_REDONLY       },
    { "TRANSLUCENT_GREENONLY",     MF2_TRANSLUCENT_GREENONLY     },
    { "TRANSLUCENT_BLUEONLY",      MF2_TRANSLUCENT_BLUEONLY      },
    { "TRANSLUCENT_33",            MF2_TRANSLUCENT_33            },
    { "TRANSLUCENT_50",            MF2_TRANSLUCENT_50            },
    { "TRANSLUCENT_REDWHITEONLY",  MF2_TRANSLUCENT_REDWHITEONLY  },
    { "NOMIRROREDCORPSE",          MF2_NOMIRROREDCORPSE          },
    { "TRANSLUCENT_BLUE_25",       MF2_TRANSLUCENT_BLUE_25       },
    { "FLOATBOB",                  MF2_FLOATBOB                  },
    { "PASSMOBJ",                  MF2_PASSMOBJ                  },
    { "FOOTCLIP",                  MF2_FOOTCLIP                  },
    { "NOLIQUIDBOB",               MF2_NOLIQUIDBOB               },
    { "CASTSHADOW",                MF2_CASTSHADOW                },
    { "DONTMAP",                   MF2_DONTMAP                   },
    { "CRUSHABLE",                 MF2_CRUSHABLE                 },
    { "DECORATION",                MF2_DECORATION                }
};

#define DEH_MOBJFLAGMAX_MBF21   arrlen(deh_mobjflags_mbf21)

static const struct deh_flag_s deh_mobjflags_mbf21[] =
{
    { "LOGRAV",         MF_MBF21_LOGRAV         },  // low gravity
    { "SHORTMRANGE",    MF_MBF21_SHORTMRANGE    },  // short missile range
    { "DMGIGNORED",     MF_MBF21_DMGIGNORED     },  // other things ignore its attacks
    { "NORADIUSDMG",    MF_MBF21_NORADIUSDMG    },  // doesn't take splash damage
    { "FORCERADIUSDMG", MF_MBF21_FORCERADIUSDMG },  // causes splash damage even if target immune
    { "HIGHERMPROB",    MF_MBF21_HIGHERMPROB    },  // higher missile attack probability
    { "RANGEHALF",      MF_MBF21_RANGEHALF      },  // use half distance for missile attack probability
    { "NOTHRESHOLD",    MF_MBF21_NOTHRESHOLD    },  // no targeting threshold
    { "LONGMELEE",      MF_MBF21_LONGMELEE      },  // long melee range
    { "BOSS",           MF_MBF21_BOSS           },  // full volume see/death sound + splash immunity
    { "MAP07BOSS1",     MF_MBF21_MAP07BOSS1     },  // Tag 666 "boss" on DOOM 2 map 7
    { "MAP07BOSS2",     MF_MBF21_MAP07BOSS2     },  // Tag 667 "boss" on DOOM 2 map 7
    { "E1M8BOSS",       MF_MBF21_E1M8BOSS       },  // E1M8 boss
    { "E2M8BOSS",       MF_MBF21_E2M8BOSS       },  // E2M8 boss
    { "E3M8BOSS",       MF_MBF21_E3M8BOSS       },  // E3M8 boss
    { "E4M6BOSS",       MF_MBF21_E4M6BOSS       },  // E4M6 boss
    { "E4M8BOSS",       MF_MBF21_E4M8BOSS       },  // E4M8 boss
    { "RIP",            MF_MBF21_RIP            },  // projectile rips through targets
    { "FULLVOLSOUNDS",  MF_MBF21_FULLVOLSOUNDS  }   // full volume see/death sound
};

#define DEH_WEAPONFLAGMAX_MBF21 arrlen(deh_weaponflags_mbf21)

static const struct deh_flag_s deh_weaponflags_mbf21[] =
{
    { "NOTHRUST",       WPF_NOTHRUST            },  // doesn't thrust Mobj's
    { "SILENT",         WPF_SILENT              },  // weapon is silent
    { "NOAUTOFIRE",     WPF_NOAUTOFIRE          },  // weapon won't autofire in A_WeaponReady
    { "FLEEMELEE",      WPF_FLEEMELEE           },  // monsters consider it a melee weapon
    { "AUTOSWITCHFROM", WPF_AUTOSWITCHFROM      },  // can be switched away from when ammo is picked up
    { "NOAUTOSWITCHTO", WPF_NOAUTOSWITCHTO      }   // cannot be switched to when ammo is picked up
};

// STATE - Dehacked block name = "Frame" and "Pointer"
// Usage: Frame nn
// Usage: Pointer nn (Frame nn)
// These are indexed separately, for lookup to the actual
// function pointers. Here we'll take whatever Dehacked gives
// us and go from there. The (Frame nn) after the pointer is the
// real place to put this value. The "Pointer" value is an xref
// that Dehacked uses and is useless to us.
// * states are base zero and have a dummy #0 (TROO)
static const char *deh_state[] =
{
    "Sprite number",    // .sprite (spritenum_t)
    "Sprite subnumber", // .frame
    "Duration",         // .tics
    "Next frame",       // .nextstate (statenum_t)
    "Codep Frame",      // pointer to first use of action (actionf_t)
    "Unknown 1",        // .misc1
    "Unknown 2",        // .misc2
    "Args1",            // .args[0]
    "Args2",            // .args[1]
    "Args3",            // .args[2]
    "Args4",            // .args[3]
    "Args5",            // .args[4]
    "Args6",            // .args[5]
    "Args7",            // .args[6]
    "Args8",            // .args[7]
    "MBF21 bits"        // .flags
};

static const struct deh_flag_s deh_stateflags_mbf21[] =
{
    { "SKILL5FAST", STATEF_SKILL5FAST },    // tics halve on nightmare skill
    { NULL,         0                 }
};

// SFXINFO_STRUCT - Dehacked block name = "Sounds"
// Sound effects, typically not changed (redirected, and new sfx put
// into the PWAD, but not changed here. Can you tell that Greg didn't
// know what they were for, mostly? Can you tell that I don't either?
// Mostly I just put these into the same slots as they are in the struct.
// This may not be supported in our -deh option if it doesn't make sense by then.

// * sounds are base zero but have a dummy #0
static const char *deh_sfxinfo[] =
{
    "Offset",           // pointer to a name string, changed in text
    "Zero/One",         // .singularity (int, one at a time flag)
    "Value",            // .priority
    "Zero 1",           // .link (sfxinfo_t*) referenced sound if linked
    "Zero 2",           // .pitch
    "Zero 3",           // .volume
    "Zero 4",           // .data (SAMPLE*) sound data
    "Neg. One 1",       // .usefulness
    "Neg. One 2"        // .lumpnum
};

// AMMO - Dehacked block name = "Ammo"
// usage = Ammo n (name)
// Ammo information for the few types of ammo
static const char *deh_ammo[] =
{
    "Max Ammo",         // maxammo[]
    "Per Ammo"          // clipammo[]
};

// WEAPONS - Dehacked block name = "Weapon"
// Usage: Weapon nn (name)
// Basically a list of frames and what kind of ammo (see above) it uses.
static const char *deh_weapon[] =
{
    "Ammo type",        // .ammo
    "Deselect frame",   // .upstate
    "Select frame",     // .downstate
    "Bobbing frame",    // .readystate
    "Shooting frame",   // .atkstate
    "Firing frame",     // .flashstate

    // MBF21
    "Ammo per shot",    // .minammo
    "MBF21 bits"        // .flags
};

// CHEATS - Dehacked block name = "Cheat"
// Usage: Cheat 0
// Always uses a zero in the dehacked file, for consistency. No meaning.
// These are just plain funky terms compared with id's
static const char *deh_cheat[] =
{
    "Change music",     // idmus
    "Chainsaw",         // idchoppers
    "God mode",         // iddqd
    "Ammo & keys",      // idkfa
    "Ammo",             // idfa
    "No clipping 1",    // idspispopd
    "No clipping 2",    // idclip
    "Invincibility",    // idbeholdv
    "Berserk",          // idbeholds
    "Invisibility",     // idbeholdi
    "Radiation suit",   // idbeholdr
    "Auto-map",         // idbeholda
    "Lite-amp goggles", // idbeholdl
    "BEHOLD menu",      // idbehold
    "Level warp",       // idclev
    "Player position"   // idmypos
};

// MISC - Dehacked block name = "Misc"
// Usage: Misc 0
// Always uses a zero in the dehacked file, for consistency. No meaning.
static const char *deh_misc[] =
{
    "Initial health",           // initial_health
    "Initial bullets",          // initial_bullets
    "Max health",               // maxhealth
    "Max armor",                // max_armor
    "Green armor class",        // green_armor_class
    "Blue armor class",         // blue_armor_class
    "Max soulsphere",           // max_soul
    "Soulsphere health",        // soul_health
    "Megasphere health",        // mega_health
    "God mode health",          // god_health
    "IDFA armor",               // idfa_armor
    "IDFA armor class",         // idfa_armor_class
    "IDKFA armor",              // idkfa_armor
    "IDKFA armor class",        // idkfa_armor_class
    "BFG cells/shot",           // BFGCELLS
    "Monsters infight"          // species_infighting
};

// TEXT - Dehacked block name = "Text"
// Usage: Text fromlen tolen
// Dehacked allows a bit of adjustment to the length (why?)

// BEX extension [CODEPTR]
// Usage: Start block, then each line is:
// FRAME nnn = PointerMnemonic

// External references to action functions scattered about the code
void A_BabyMetal(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BetaSkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BFGSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BFGSpray(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BossDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainAwake(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainExplode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainSpit(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BruisAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BspiAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckReload(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CloseShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CPosRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CyberAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Detonate(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Die(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Explode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Face(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack3(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatRaise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireBFG(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCGun(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCrackle(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireMissile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireOldBFG(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePistol(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePlasma(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireShotgun(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GunFlash(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Hoof(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_KeenDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light0(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_LineEffect(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_LoadShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Metal(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Mushroom(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_OpenShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PainAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PainDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PlayerScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PlaySound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Punch(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RandomJump(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SargAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Saw(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scratch(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelFist(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelMissile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelWhoosh(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullPop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Spawn(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnFly(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpidRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_StartFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Stop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Tracer(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_TroopAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Turn(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileChase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileStart(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_XScream(mobj_t *actor, player_t *player, pspdef_t *psp);

// New MBF21 codepointers
void A_SpawnObject(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MonsterProjectile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MonsterBulletAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MonsterMeleeAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RadiusDamage(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_NoiseAlert(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HealChase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SeekTracer(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FindTracer(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ClearTracer(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_JumpIfHealthBelow(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_JumpIfTargetInSight(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_JumpIfTargetCloser(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_JumpIfTracerInSight(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_JumpIfTracerCloser(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_JumpIfFlagsSet(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_AddFlags(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RemoveFlags(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponProjectile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponBulletAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponMeleeAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponAlert(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponJump(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ConsumeAmmo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckAmmo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RefireTo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GunFlashTo(mobj_t *actor, player_t *player, pspdef_t *psp);

#define MBF     1
#define MBF21   2

typedef struct
{
    actionf_t   cptr;                       // actual pointer to the subroutine
    const char  *lookup;                    // mnemonic lookup string to be specified in BEX
    int         mbf;

    // MBF21
    int         argcount;                   // [XA] number of MBF21 args this action uses, if any
    int         default_args[MAXSTATEARGS]; // default values for MBF21 args
} deh_bexptr;

static const deh_bexptr deh_bexptrs[] =
{
    { &A_Light0,              "A_Light0"                                                      },
    { &A_WeaponReady,         "A_WeaponReady"                                                 },
    { &A_Lower,               "A_Lower"                                                       },
    { &A_Raise,               "A_Raise"                                                       },
    { &A_Punch,               "A_Punch"                                                       },
    { &A_ReFire,              "A_ReFire"                                                      },
    { &A_FirePistol,          "A_FirePistol"                                                  },
    { &A_Light1,              "A_Light1"                                                      },
    { &A_FireShotgun,         "A_FireShotgun"                                                 },
    { &A_Light2,              "A_Light2"                                                      },
    { &A_FireShotgun2,        "A_FireShotgun2"                                                },
    { &A_CheckReload,         "A_CheckReload"                                                 },
    { &A_OpenShotgun2,        "A_OpenShotgun2"                                                },
    { &A_LoadShotgun2,        "A_LoadShotgun2"                                                },
    { &A_CloseShotgun2,       "A_CloseShotgun2"                                               },
    { &A_FireCGun,            "A_FireCGun"                                                    },
    { &A_GunFlash,            "A_GunFlash"                                                    },
    { &A_FireMissile,         "A_FireMissile"                                                 },
    { &A_Saw,                 "A_Saw"                                                         },
    { &A_FirePlasma,          "A_FirePlasma"                                                  },
    { &A_BFGSound,            "A_BFGSound"                                                    },
    { &A_FireBFG,             "A_FireBFG"                                                     },
    { &A_BFGSpray,            "A_BFGSpray"                                                    },
    { &A_Explode,             "A_Explode"                                                     },
    { &A_Pain,                "A_Pain"                                                        },
    { &A_PlayerScream,        "A_PlayerScream"                                                },
    { &A_Fall,                "A_Fall"                                                        },
    { &A_XScream,             "A_XScream"                                                     },
    { &A_Look,                "A_Look"                                                        },
    { &A_Chase,               "A_Chase"                                                       },
    { &A_FaceTarget,          "A_FaceTarget"                                                  },
    { &A_PosAttack,           "A_PosAttack"                                                   },
    { &A_Scream,              "A_Scream"                                                      },
    { &A_SPosAttack,          "A_SPosAttack"                                                  },
    { &A_VileChase,           "A_VileChase"                                                   },
    { &A_VileStart,           "A_VileStart"                                                   },
    { &A_VileTarget,          "A_VileTarget"                                                  },
    { &A_VileAttack,          "A_VileAttack"                                                  },
    { &A_StartFire,           "A_StartFire"                                                   },
    { &A_Fire,                "A_Fire"                                                        },
    { &A_FireCrackle,         "A_FireCrackle"                                                 },
    { &A_Tracer,              "A_Tracer"                                                      },
    { &A_SkelWhoosh,          "A_SkelWhoosh"                                                  },
    { &A_SkelFist,            "A_SkelFist"                                                    },
    { &A_SkelMissile,         "A_SkelMissile"                                                 },
    { &A_FatRaise,            "A_FatRaise"                                                    },
    { &A_FatAttack1,          "A_FatAttack1"                                                  },
    { &A_FatAttack2,          "A_FatAttack2"                                                  },
    { &A_FatAttack3,          "A_FatAttack3"                                                  },
    { &A_BossDeath,           "A_BossDeath"                                                   },
    { &A_CPosAttack,          "A_CPosAttack"                                                  },
    { &A_CPosRefire,          "A_CPosRefire"                                                  },
    { &A_TroopAttack,         "A_TroopAttack"                                                 },
    { &A_SargAttack,          "A_SargAttack"                                                  },
    { &A_HeadAttack,          "A_HeadAttack"                                                  },
    { &A_BruisAttack,         "A_BruisAttack"                                                 },
    { &A_SkullAttack,         "A_SkullAttack"                                                 },
    { &A_Metal,               "A_Metal"                                                       },
    { &A_SpidRefire,          "A_SpidRefire"                                                  },
    { &A_BabyMetal,           "A_BabyMetal"                                                   },
    { &A_BspiAttack,          "A_BspiAttack"                                                  },
    { &A_Hoof,                "A_Hoof"                                                        },
    { &A_CyberAttack,         "A_CyberAttack"                                                 },
    { &A_PainAttack,          "A_PainAttack"                                                  },
    { &A_PainDie,             "A_PainDie"                                                     },
    { &A_KeenDie,             "A_KeenDie"                                                     },
    { &A_BrainPain,           "A_BrainPain"                                                   },
    { &A_BrainScream,         "A_BrainScream"                                                 },
    { &A_BrainDie,            "A_BrainDie"                                                    },
    { &A_BrainAwake,          "A_BrainAwake"                                                  },
    { &A_BrainSpit,           "A_BrainSpit"                                                   },
    { &A_SpawnSound,          "A_SpawnSound"                                                  },
    { &A_SpawnFly,            "A_SpawnFly"                                                    },
    { &A_BrainExplode,        "A_BrainExplode"                                                },
    { &A_Detonate,            "A_Detonate",            MBF,                                   },    // killough 08/09/98
    { &A_Mushroom,            "A_Mushroom",            MBF,                                   },    // killough 10/98
    { &A_SkullPop,            "A_SkullPop"                                                    },
    { &A_Die,                 "A_Die",                 MBF,                                   },    // killough 11/98
    { &A_Spawn,               "A_Spawn",               MBF,                                   },    // killough 11/98
    { &A_Turn,                "A_Turn",                MBF,                                   },    // killough 11/98
    { &A_Face,                "A_Face",                MBF,                                   },    // killough 11/98
    { &A_Scratch,             "A_Scratch",             MBF,                                   },    // killough 11/98
    { &A_PlaySound,           "A_PlaySound",           MBF,                                   },    // killough 11/98
    { &A_RandomJump,          "A_RandomJump",          MBF,                                   },    // killough 11/98
    { &A_LineEffect,          "A_LineEffect",          MBF,                                   },    // killough 11/98

    { &A_FireOldBFG,          "A_FireOldBFG"                                                  },    // killough 07/19/98: classic BFG firing function
    { &A_BetaSkullAttack,     "A_BetaSkullAttack"                                             },    // killough 10/98: beta lost souls attacked different
    { &A_Stop,                "A_Stop"                                                        },

    // [XA] New MBF21 codepointers
    { &A_SpawnObject,         "A_SpawnObject",         MBF21, 8                               },
    { &A_MonsterProjectile,   "A_MonsterProjectile",   MBF21, 5                               },
    { &A_MonsterBulletAttack, "A_MonsterBulletAttack", MBF21, 5, { 0, 0, 1, 3, 5 }            },
    { &A_MonsterMeleeAttack,  "A_MonsterMeleeAttack",  MBF21, 4, { 3, 8, 0, 0 }               },
    { &A_RadiusDamage,        "A_RadiusDamage",        MBF21, 2                               },
    { &A_NoiseAlert,          "A_NoiseAlert",          MBF21, 0                               },
    { &A_HealChase,           "A_HealChase",           MBF21, 2                               },
    { &A_SeekTracer,          "A_SeekTracer",          MBF21, 2                               },
    { &A_FindTracer,          "A_FindTracer",          MBF21, 2, { 0, 10 }                    },
    { &A_ClearTracer,         "A_ClearTracer",         MBF21, 0                               },
    { &A_JumpIfHealthBelow,   "A_JumpIfHealthBelow",   MBF21, 2                               },
    { &A_JumpIfTargetInSight, "A_JumpIfTargetInSight", MBF21, 2                               },
    { &A_JumpIfTargetCloser,  "A_JumpIfTargetCloser",  MBF21, 2                               },
    { &A_JumpIfTracerInSight, "A_JumpIfTracerInSight", MBF21, 2                               },
    { &A_JumpIfTracerCloser,  "A_JumpIfTracerCloser",  MBF21, 2                               },
    { &A_JumpIfFlagsSet,      "A_JumpIfFlagsSet",      MBF21, 3                               },
    { &A_AddFlags,            "A_AddFlags",            MBF21, 2                               },
    { &A_RemoveFlags,         "A_RemoveFlags",         MBF21, 2                               },
    { &A_WeaponProjectile,    "A_WeaponProjectile",    MBF21, 5                               },
    { &A_WeaponBulletAttack,  "A_WeaponBulletAttack",  MBF21, 5, {0, 0, 1, 5, 3 }             },
    { &A_WeaponMeleeAttack,   "A_WeaponMeleeAttack",   MBF21, 5, {2, 10, 1 * FRACUNIT, 0, 0 } },
    { &A_WeaponSound,         "A_WeaponSound",         MBF21, 2                               },
    { &A_WeaponAlert,         "A_WeaponAlert",         MBF21, 0                               },
    { &A_WeaponJump,          "A_WeaponJump",          MBF21, 2                               },
    { &A_ConsumeAmmo,         "A_ConsumeAmmo",         MBF21, 1                               },
    { &A_CheckAmmo,           "A_CheckAmmo",           MBF21, 2                               },
    { &A_RefireTo,            "A_RefireTo",            MBF21, 2                               },
    { &A_GunFlashTo,          "A_GunFlashTo",          MBF21, 2                               },

    // This NULL entry must be the last in the list
    { NULL,                   "A_NULL"                                                        }
};

// to hold startup code pointers from INFO.C
static actionf_t    deh_codeptr[NUMSTATES];

// haleyjd: support for BEX SPRITES, SOUNDS, and MUSIC
char                *deh_spritenames[NUMSPRITES + 1];
char                *deh_musicnames[NUMMUSIC + 1];
char                *deh_soundnames[NUMSFX + 1];

void D_BuildBEXTables(void)
{
    int i;

    // moved from D_ProcessDehFile, then we don't need the static int i
    for (i = 0; i < EXTRASTATES; i++)  // remember what they start as for deh xref
        deh_codeptr[i] = states[i].action;

    // initialize extra dehacked states
    for (; i < NUMSTATES; i++)
    {
        states[i].sprite = SPR_TNT1;
        states[i].frame = 0;
        states[i].tics = -1;
        states[i].action = NULL;
        states[i].nextstate = i;
        states[i].misc1 = 0;
        states[i].misc2 = 0;

        for (int j = 0; j < MAXSTATEARGS; j++)
            states[i].args[j] = 0;

        states[i].flags = 0;
        states[i].translucent = false;
        states[i].dehacked = false;

        deh_codeptr[i] = states[i].action;
    }

    for (i = MT_EXTRA00; i <= MT_EXTRA99; i++)
        M_snprintf(mobjinfo[i].name1, sizeof(mobjinfo[0].name1), "Deh_Actor_%i", i);

    for (i = 0; i < NUMSPRITES; i++)
        deh_spritenames[i] = M_StringDuplicate(sprnames[i]);

    deh_spritenames[NUMSPRITES] = NULL;

    for (i = 1; i < NUMMUSIC; i++)
        deh_musicnames[i] = M_StringDuplicate(S_music[i].name1);

    deh_musicnames[0] = NULL;
    deh_musicnames[NUMMUSIC] = NULL;

    for (i = 1; i < NUMSFX; i++)
        deh_soundnames[i] = (S_sfx[i].name1[0] != '\0' ? M_StringDuplicate(S_sfx[i].name1) : NULL);

    deh_soundnames[0] = NULL;
    deh_soundnames[NUMSFX] = NULL;

    // MBF21
    for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
        states[i].flags |= STATEF_SKILL5FAST;
}

// ====================================================================
// D_ProcessDehFile
// Purpose: Read and process a DEH or BEX file
// Args:    filename    -- name of the DEH/BEX file
// Returns: void
//
// killough 10/98:
// substantially modified to allow input from WAD lumps instead of .deh files.
void D_ProcessDehFile(char *filename, int lumpnum, bool autoloaded)
{
    DEHFILE infile;
    DEHFILE *filein = &infile;              // killough 10/98
    char    inbuffer[DEH_BUFFERMAX];        // Place to put the primary infostring

    linecount = 0;
    addtodehmaptitlecount = false;

    // killough 10/98: allow DEH files to come from WAD lumps
    if (filename)
    {
        if (!(infile.f = fopen(filename, "rt")))
            return;                         // should be checked up front anyway

        infile.lump = NULL;
    }
    else
    {
        // DEH file comes from lump indicated by second argument
        if (!(infile.size = W_LumpLength(lumpnum)))
            return;

        infile.inp = infile.lump = W_CacheLumpNum(lumpnum);
        filename = lumpinfo[lumpnum]->wadfile->path;
    }

    // loop until end of file
    while (dehfgets(inbuffer, sizeof(inbuffer), filein))
    {
        bool                match = false;
        unsigned int        i;
        static unsigned int last_i = DEH_BLOCKMAX - 1;
        static int          filepos;

        lfstrip(inbuffer);

        if (devparm)
            C_Output("Line = \"%s\"", inbuffer);

        if (!*inbuffer || *inbuffer == '#' || *inbuffer == ' ' || (*inbuffer == '/' && *(inbuffer + 1) == '/'))
            continue;   // Blank line or comment line

        // -- If DEH_BLOCKMAX is set right, the processing is independently
        // -- handled based on data in the deh_blocks[] structure array

        // killough 10/98: INCLUDE code rewritten to allow arbitrary nesting,
        // and to greatly simplify code, fix memory leaks, other bugs
        if (!strncasecmp(inbuffer, "INCLUDE", 7))  // include a file
        {
            // preserve state while including a file
            // killough 10/98: moved to here

            char    *nextfile;
            bool    oldnotext = includenotext;              // killough 10/98

            // killough 10/98: exclude if inside wads (only to discourage
            // the practice, since the code could otherwise handle it)
            if (infile.lump)
            {
                C_Warning(1, "No files may be included from wads: \"%s\".", inbuffer);
                continue;
            }

            // check for no-text directive, used when including a DEH
            // file but using the BEX format to handle strings
            if (!strncasecmp((nextfile = ptr_lstrip(inbuffer + 7)), "NOTEXT", 6))
            {
                includenotext = true;
                nextfile = ptr_lstrip(nextfile + 6);
            }

            if (devparm)
                C_Output("Branching to include file " BOLD("%s") "...", nextfile);

            D_ProcessDehFile(nextfile, 0, false);                 // do the included file

            includenotext = oldnotext;

            if (devparm)
                C_Output("...continuing with " BOLD("%s"), filename);

            continue;
        }

        for (i = 0; i < DEH_BLOCKMAX - 1; i++)
            if (!strncasecmp(inbuffer, deh_blocks[i].key, strlen(deh_blocks[i].key)))
            {
                match = true;
                break;                                          // we got one, that's enough for this block
            }

        if (match)                                              // inbuffer matches a valid block code name
            last_i = i;
        else if (last_i >= 10 && last_i < DEH_BLOCKMAX - 1)     // restrict to BEX style lumps
        {
            // process that same line again with the last valid block code handler
            i = last_i;

            if (!filein->lump)
                fseek(filein->f, filepos, SEEK_SET);
        }

        if (i < DEH_BLOCKMAX)
        {
            if (devparm)
                C_Output("Processing function [%i] for %s", i, deh_blocks[i].key);

            deh_blocks[i].fptr(filein, inbuffer);               // call function
        }

        if (!filein->lump)                                      // back up line start
            filepos = ftell(filein->f);
    }

    if (infile.lump)
        Z_ChangeTag(infile.lump, PU_CACHE);                     // mark purgeable
    else
        fclose(infile.f);                                       // close real file

    dehcount++;

    if (addtodehmaptitlecount)
        dehmaptitlecount++;

    if (infile.lump)
    {
        char    *temp1 = commify(linecount);
        char    *temp2 = uppercase(lumpinfo[lumpnum]->name);

        if (!M_StringCompare(leafname(filename), DOOMRETRO_WAD) && !devparm)
            C_Output("Parsed %s line%s from the " BOLD("%s") " lump in the %s " BOLD("%s") "%s.",
                temp1, (linecount == 1 ? "" : "s"), temp2, (W_WadType(filename) == IWAD ? "IWAD" : "PWAD"),
                filename, (dehfileignored ? " instead" : ""));

        dehfileignored = false;
        free(temp1);
        free(temp2);
    }
    else
    {
        char    *temp = commify(linecount);

        C_Output("%s %s line%s from the " ITALICS("DeHackEd") "%s file " BOLD("%s") ".",
            (autoloaded ? "Automatically parsed" : "Parsed"), temp, (linecount == 1 ? "" : "s"),
            (M_StringEndsWith(filename, "BEX") ? " with " ITALICS("BOOM") " extensions" : ""), GetCorrectCase(filename));

        free(temp);
    }
}

// ====================================================================
// deh_procBexCodePointers
// Purpose: Handle [CODEPTR] block, BOOM Extension
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procBexCodePointers(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN] = "";
    char    inbuffer[DEH_BUFFERMAX] = "";
    int     indexnum;
    char    mnemonic[DEH_MAXKEYLEN] = "";       // to hold the codepointer mnemonic

    boomcompatible = true;

    // Ty 05/16/98 - initialize it to something, dummy!
    strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

    // for this one, we just read 'em until we hit a blank line
    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        int     i = -1;                         // looper
        bool    found = false;                  // know if we found this one during lookup or not

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                              // killough 11/98: really exit on blank line

        // killough 08/98: allow hex numbers in input:
        if ((sscanf(inbuffer, "%s %i = %s", key, &indexnum, mnemonic) != 3)
            || !M_StringCompare(key, "FRAME"))  // NOTE: different format from normal
        {
            C_Warning(1, "Invalid BEX codepointer line - must start with \"FRAME\": \"%s\".", inbuffer);
            return;                             // early return
        }

        if (devparm)
            C_Output("Processing pointer at index %i: %s", indexnum, mnemonic);

        if (indexnum < 0 || indexnum >= NUMSTATES)
        {
            C_Warning(1, "Bad pointer number %i of %i.", indexnum, NUMSTATES);
            return;                             // killough 10/98: fix SegViol
        }

        strcpy(key, "A_");                      // reusing the key area to prefix the mnemonic
        strcat(key, ptr_lstrip(mnemonic));

        do
        {
            i++;

            if (M_StringCompare(key, deh_bexptrs[i].lookup))
            {
                states[indexnum].action = deh_bexptrs[i].cptr;

                if (devparm)
                    C_Output(" - applied %s from codeptr[%i] to states[%i]", deh_bexptrs[i].lookup, i, indexnum);

                if (deh_bexptrs[i].mbf == MBF)
                    mbfcompatible = true;
                else if (deh_bexptrs[i].mbf == MBF21)
                    mbf21compatible = true;

                found = true;
            }
        } while (!found && deh_bexptrs[i].cptr);

        if (!found && !M_StringCompare(mnemonic, "NULL"))
            C_Warning(1, "Invalid frame pointer mnemonic \"%s\" at %i.", mnemonic, indexnum);
    }
}

// ====================================================================
// deh_procThing
// Purpose: Handle DEH Thing block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
// Ty 8/27/98 - revised to also allow mnemonics for
// bit masks for monster attributes
//
static void deh_procThing(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    int     indexnum;
    int     ix;
    char    *strval;
    bool    namechange = false;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX - 1);

    if ((ix = sscanf(inbuffer, "%s %i", key, &indexnum)) != 2)
    {
        C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
        return;
    }

    if (devparm)
    {
        C_Output("Thing line: \"%s\"", inbuffer);
        C_Output("count = %i, Thing %i", ix, indexnum);
    }

    // Note that the mobjinfo[] array is base zero, but object numbers
    // in the dehacked file start with one. Grumble.
    indexnum--;

    // now process the stuff
    // Note that for Things we can look up the key and use its offset
    // in the array of key strings as an int offset in the structure

    // get a line until a blank or end of file -- it's not
    // blank now because it has our incoming key in it
    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
        int     bGetData;
        bool    gibhealth = false;
        bool    string = false;

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);      // toss the end of line

        // killough 11/98: really bail out on blank lines (break != continue)
        if (!*inbuffer)
            break;              // bail out with blank line between sections

        // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
        if (!(bGetData = deh_GetData(inbuffer, key, &value, &strval)))
        {
            C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if ((string = M_StringCompare(key, "Name")) || (string = M_StringCompare(key, "Name1")))
        {
            M_StringCopy(mobjinfo[indexnum].name1, lowercase(trimwhitespace(strval)), sizeof(mobjinfo[0].name1));
            M_snprintf(mobjinfo[indexnum].plural1, sizeof(mobjinfo[0].plural1), "%ss", mobjinfo[indexnum].name1);
            namechange = true;
        }
        else if ((string = M_StringCompare(key, "Plural")) || (string = M_StringCompare(key, "Plural1")))
            M_StringCopy(mobjinfo[indexnum].plural1, lowercase(trimwhitespace(strval)), sizeof(mobjinfo[0].plural1));
        else if ((string = M_StringCompare(key, "Name2")))
        {
            M_StringCopy(mobjinfo[indexnum].name2, lowercase(trimwhitespace(strval)), sizeof(mobjinfo[0].name2));
            M_snprintf(mobjinfo[indexnum].plural2, sizeof(mobjinfo[0].plural2), "%ss", mobjinfo[indexnum].name2);
        }
        else if ((string = M_StringCompare(key, "Plural2")))
            M_StringCopy(mobjinfo[indexnum].plural2, lowercase(trimwhitespace(strval)), sizeof(mobjinfo[0].plural2));
        else if ((string = M_StringCompare(key, "Name3")))
        {
            M_StringCopy(mobjinfo[indexnum].name3, lowercase(trimwhitespace(strval)), sizeof(mobjinfo[0].name3));
            M_snprintf(mobjinfo[indexnum].plural3, sizeof(mobjinfo[0].plural3), "%ss", mobjinfo[indexnum].name3);
        }
        else if ((string = M_StringCompare(key, "Plural3")))
            M_StringCopy(mobjinfo[indexnum].plural3, lowercase(trimwhitespace(strval)), sizeof(mobjinfo[0].plural3));

        if (string)
        {
            if (devparm)
                C_Output("Assigned %s to %s (%i) at index %i.", lowercase(trimwhitespace(strval)), key, indexnum, ix);

            continue;
        }

        for (ix = 0; ix < DEH_MOBJINFOMAX; ix++)
        {
            if (!M_StringCompare(key, deh_mobjinfo[ix]))
                continue;

            if (M_StringCompare(key, "Bits"))
            {
                // bit set
                // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
                if (bGetData == 1)
                    mobjinfo[indexnum].flags = value;
                else
                {
                    // killough 10/98: replace '+' kludge with strtok() loop
                    // Fix error-handling case ('found' var wasn't being reset)
                    //
                    // Use OR logic instead of addition, to allow repetition
                    for (value = 0; (strval = strtok(strval, ",+| \t\f\r")); strval = NULL)
                    {
                        int iy;

                        for (iy = 0; iy < DEH_MOBJFLAGMAX; iy++)
                        {
                            if (!M_StringCompare(strval, deh_mobjflags[iy].name))
                                continue;

                            if (devparm)
                                C_Output("ORed value 0x%08x %s.", deh_mobjflags[iy].value, strval);

                            if (M_StringCompare(key, "TRANSLUCENT"))
                                boomcompatible = true;
                            else if (M_StringCompare(key, "TOUCHY") || M_StringCompare(key, "BOUNCES") || M_StringCompare(key, "FRIEND"))
                                mbfcompatible = true;

                            value |= deh_mobjflags[iy].value;
                            break;
                        }

                        if (iy >= DEH_MOBJFLAGMAX)
                            C_Warning(1, "Could not find bit mnemonic \"%s\".", strval);
                    }

                    // Don't worry about conversion -- simply print values
                    if (devparm)
                        C_Output("Bits = 0x%08x = %i.", value, value);

                    mobjinfo[indexnum].flags = value; // e6y
                }
            }
            else if (M_StringCompare(key, "Retro bits"))
            {
                // bit set
                if (bGetData == 1)
                    mobjinfo[indexnum].flags2 = value;
                else
                {
                    for (value = 0; (strval = strtok(strval, ",+| \t\f\r")); strval = NULL)
                    {
                        int iy;

                        for (iy = 0; iy < DEH_MOBJFLAG2MAX; iy++)
                        {
                            if (!M_StringCompare(strval, deh_mobjflags2[iy].name))
                                continue;

                            if (devparm)
                                C_Output("ORed value 0x%08x %s.", deh_mobjflags2[iy].value, strval);

                            value |= deh_mobjflags2[iy].value;
                            break;
                        }

                        if (iy >= DEH_MOBJFLAG2MAX)
                            C_Warning(1, "Could not find bit mnemonic \"%s\".", strval);
                    }

                    // Don't worry about conversion -- simply print values
                    if (devparm)
                        C_Output("Bits = 0x%08x = %i.", value, value);

                    mobjinfo[indexnum].flags2 = value;
                }
            }
            else if (M_StringCompare(key, "MBF21 bits"))
            {
                // bit set
                if (bGetData == 1)
                    mobjinfo[indexnum].mbf21flags = value;
                else
                {
                    for (value = 0; (strval = strtok(strval, ",+| \t\f\r")); strval = NULL)
                    {
                        int iy;

                        for (iy = 0; iy < DEH_MOBJFLAGMAX_MBF21; iy++)
                        {
                            if (!M_StringCompare(strval, deh_mobjflags_mbf21[iy].name))
                                continue;

                            if (devparm)
                                C_Output("ORed value 0x%08x %s.", deh_mobjflags_mbf21[iy].value, strval);

                            value |= deh_mobjflags_mbf21[iy].value;
                            break;
                        }

                        if (iy >= DEH_MOBJFLAGMAX_MBF21)
                            C_Warning(1, "Could not find MBF21 bit mnemonic \"%s\".", strval);
                    }

                    // Don't worry about conversion -- simply print values
                    if (devparm)
                        C_Output("Bits = 0x%08x = %i.", value, value);

                    mobjinfo[indexnum].mbf21flags = value;
                }

                mbf21compatible = true;
            }
            else if (M_StringCompare(key, "Blood color"))
            {
                if (value >= 0 && value < CR_LIMIT)
                    mobjinfo[indexnum].bloodcolor = value + 1;
            }
            else if (M_StringCompare(key, "Blood"))
            {
                if (value == MT_BLOOD)
                    mobjinfo[indexnum].bloodcolor = REDBLOOD;
                else if (value == MT_BLUEBLOOD)
                    mobjinfo[indexnum].bloodcolor = BLUEBLOOD;
                else if (value == MT_GREENBLOOD)
                    mobjinfo[indexnum].bloodcolor = GREENBLOOD;
                else if (value == MT_FUZZYBLOOD)
                    mobjinfo[indexnum].bloodcolor = FUZZYBLOOD;
            }
            else if (M_StringCompare(key, "Dropped item"))
                mobjinfo[indexnum].droppeditem = value - 1;
            else if (M_StringCompare(key, "Infighting group"))
                mobjinfo[indexnum].infightinggroup = value + IG_END;
            else if (M_StringCompare(key, "Projectile group"))
                mobjinfo[indexnum].projectilegroup = (value < 0 ? PG_GROUPLESS : value + PG_END);
            else if (M_StringCompare(key, "Splash group"))
                mobjinfo[indexnum].splashgroup = value + SG_END;
            else
            {
                int *pix = (int *)&mobjinfo[indexnum];

                pix[ix] = value;

                if (M_StringCompare(key, "Height"))
                    mobjinfo[indexnum].projectilepassheight = 0;
                else if (M_StringCompare(key, "Gib health"))
                    gibhealth = true;
            }

            if (devparm)
                C_Output("Assigned %i to %s (%i) at index %i.", value, key, indexnum, ix);

            mobjinfo[indexnum].dehacked = true;
            break;
        }

        if (!gibhealth && mobjinfo[indexnum].spawnhealth && !mobjinfo[indexnum].gibhealth)
            mobjinfo[indexnum].gibhealth = -mobjinfo[indexnum].spawnhealth;
    }

    // [BH] Disable bobbing and translucency if thing no longer a pickup
    if ((mobjinfo[indexnum].flags2 & MF2_FLOATBOB) && !(mobjinfo[indexnum].flags & MF_SPECIAL))
        mobjinfo[indexnum].flags2 &= ~(MF2_FLOATBOB | MF2_TRANSLUCENT_33 | MF2_TRANSLUCENT_BLUE_25);

    // [BH] No extra barrel frame
    if (indexnum == MT_BARREL)
    {
        states[S_BAR1].nextstate = S_BAR2;
        mobjinfo[MT_BARREL].frames = 2;
    }

    // [BH] Call Wolf SS and Keen "monsters" if no name given
    if ((indexnum == MT_WOLFSS || indexnum == MT_KEEN) && !namechange)
    {
        M_StringCopy(mobjinfo[indexnum].name1, "monster", sizeof(mobjinfo[0].name1));
        M_snprintf(mobjinfo[indexnum].plural1, sizeof(mobjinfo[0].plural1), "monsters");
    }
}

// ====================================================================
// deh_procFrame
// Purpose: Handle DEH Frame block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procFrame(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    int     indexnum;
    char    *strval;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX - 1);

    if (sscanf(inbuffer, "%s %i", key, &indexnum) != 2)
    {
        C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
        return;
    }

    if (devparm)
        C_Output("Processing Frame at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMSTATES)
    {
        C_Warning(1, "Bad frame number %i of %i.", indexnum, NUMSTATES);
        return;
    }

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                                              // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, &strval))       // returns true if ok
        {
            C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (M_StringCompare(key, deh_state[0]))                 // Sprite number
        {
            if (devparm)
                C_Output(" - sprite = %i", value);

            states[indexnum].sprite = (spritenum_t)value;
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[1]))            // Sprite subnumber
        {
            if (devparm)
                C_Output(" - frame = %i", value);

            states[indexnum].frame = value;
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[2]))            // Duration
        {
            if (devparm)
                C_Output(" - tics = %i", value);

            states[indexnum].tics = value;
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[3]))            // Next frame
        {
            if (devparm)
                C_Output(" - nextstate = %i", value);

            states[indexnum].nextstate = value;
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[4]))            // Codep frame (not set in Frame deh block)
            C_Warning(1, "Codep frame should not be set in Frame section.");
        else if (M_StringCompare(key, deh_state[5]))            // Unknown 1
        {
            if (devparm)
                C_Output(" - misc1 = %i", value);

            states[indexnum].misc1 = value;
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[6]))            // Unknown 2
        {
            if (devparm)
                C_Output(" - misc2 = %i", value);

            states[indexnum].misc2 = value;
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (!strcasecmp(key, deh_state[7]))                // Args1
        {
            states[indexnum].args[0] = value;
            defined_codeptr_args[indexnum] |= (1 << 0);
            mbf21compatible = true;
        }
        else if (!strcasecmp(key, deh_state[8]))                // Args2
        {
            states[indexnum].args[1] = value;
            defined_codeptr_args[indexnum] |= (1 << 1);
            mbf21compatible = true;
        }
        else if (!strcasecmp(key, deh_state[9]))                // Args3
        {
            states[indexnum].args[2] = value;
            defined_codeptr_args[indexnum] |= (1 << 2);
            mbf21compatible = true;
        }
        else if (!strcasecmp(key, deh_state[10]))               // Args4
        {
            states[indexnum].args[3] = value;
            defined_codeptr_args[indexnum] |= (1 << 3);
            mbf21compatible = true;
        }
        else if (!strcasecmp(key, deh_state[11]))               // Args5
        {
            states[indexnum].args[4] = value;
            defined_codeptr_args[indexnum] |= (1 << 4);
            mbf21compatible = true;
        }
        else if (!strcasecmp(key, deh_state[12]))               // Args6
        {
            states[indexnum].args[5] = value;
            defined_codeptr_args[indexnum] |= (1 << 5);
            mbf21compatible = true;
        }
        else if (!strcasecmp(key, deh_state[13]))               // Args7
        {
            states[indexnum].args[6] = value;
            defined_codeptr_args[indexnum] |= (1 << 6);
            mbf21compatible = true;
        }
        else if (!strcasecmp(key, deh_state[14]))               // Args8
        {
            states[indexnum].args[7] = value;
            defined_codeptr_args[indexnum] |= (1 << 7);
            mbf21compatible = true;
        }

        // MBF21: process state flags
        else if (!strcasecmp(key, deh_state[15]))               // MBF21 bits
        {
            if (!value)
                for (value = 0; (strval = strtok(strval, ",+| \t\f\r")); strval = NULL)
                {
                    const struct deh_flag_s *flag;

                    for (flag = deh_stateflags_mbf21; flag->name; flag++)
                    {
                        if (strcasecmp(strval, flag->name))
                            continue;

                        value |= flag->value;
                        break;
                    }
                }

            states[indexnum].flags = value;
            mbf21compatible = true;
        }
        else if (M_StringCompare(key, "translucent"))           // Translucent
        {
            if (devparm)
                C_Output(" - translucent = %i", value);

            states[indexnum].translucent = !!value;             // bool
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else
            C_Warning(1, "Invalid frame string index for \"%s\".", key);
    }
}

// ====================================================================
// deh_procPointer
// Purpose: Handle DEH Code pointer block, can use BEX [CODEPTR] instead
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procPointer(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    int     indexnum;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX - 1);

    if (sscanf(inbuffer, "%*s %*i (%s %i)", key, &indexnum) != 2)
    {
        C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
        return;
    }

    if (devparm)
        C_Output("Processing pointer at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMSTATES)
    {
        C_Warning(1, "Bad pointer number %i of %i.", indexnum, NUMSTATES);
        return;
    }

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))   // returns true if ok
        {
            C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (value < 0 || value >= NUMSTATES)
        {
            C_Warning(1, "Bad pointer number %i of %i.", value, NUMSTATES);
            return;
        }

        if (M_StringCompare(key, deh_state[4]))     // Codep frame (not set in Frame deh block)
        {
            states[indexnum].action = deh_codeptr[value];

            if (devparm)
                C_Output(" - applied %p from codeptr[%i] to states[%i]", (void *)deh_codeptr[value], value, indexnum);

            // Write BEX-oriented line to match:
            for (int i = 0; i < arrlen(deh_bexptrs); i++)
                if (!memcmp(&deh_bexptrs[i].cptr, &deh_codeptr[value], sizeof(actionf_t)))
                {
                    if (devparm)
                        C_Output("BEX [CODEPTR] -> FRAME %i = %s", indexnum, &deh_bexptrs[i].lookup[2]);

                    break;
                }
        }
        else
            C_Warning(1, "Invalid frame pointer index for \"%s\" at %i, xref %p.", key, value, (void *)deh_codeptr[value]);
    }
}

// ====================================================================
// deh_procSounds
// Purpose: Handle DEH Sounds block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procSounds(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    int     indexnum;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX - 1);

    if (sscanf(inbuffer, "%s %i", key, &indexnum) != 2)
    {
        C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
        return;
    }

    if (devparm)
        C_Output("Processing Sounds at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMSFX)
        C_Warning(1, "Bad sound number %i of %i.", indexnum, NUMSFX);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))   // returns true if ok
        {
            C_Warning(1, "Bad data pair in \"%s\"\n", inbuffer);
            continue;
        }

        if (M_StringCompare(key, deh_sfxinfo[0]))           // Offset
            /* nop */;
        else if (M_StringCompare(key, deh_sfxinfo[1]))      // Zero/One
            S_sfx[indexnum].singularity = value;
        else if (M_StringCompare(key, deh_sfxinfo[2]))      // Value
            S_sfx[indexnum].priority = value;
        else if (M_StringCompare(key, deh_sfxinfo[3]))      // Zero 1
            /* nop */;
        else if (M_StringCompare(key, deh_sfxinfo[4]))      // Zero 2
            /* nop */;
        else if (M_StringCompare(key, deh_sfxinfo[5]))      // Zero 3
            /* nop */;
        else if (M_StringCompare(key, deh_sfxinfo[6]))      // Zero 4
            /* nop */;
        else if (M_StringCompare(key, deh_sfxinfo[7]))      // Neg. One 1
            /* nop */;
        else if (M_StringCompare(key, deh_sfxinfo[8]))      // Neg. One 2
            S_sfx[indexnum].lumpnum = value;
        else if (devparm)
            C_Warning(1, "Invalid sound string index for \"%s\"", key);
    }
}

// ====================================================================
// deh_procAmmo
// Purpose: Handle DEH Ammo block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procAmmo(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    int     indexnum;
    char    *strval;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX - 1);

    if (sscanf(inbuffer, "%s %i", key, &indexnum) != 2)
    {
        C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
        return;
    }

    if (devparm)
        C_Output("Processing Ammo at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMAMMO)
        C_Warning(1, "Bad ammo number %i of %i.", indexnum, NUMAMMO);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                                          // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, &strval))   // returns true if ok
        {
            C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (M_StringCompare(key, deh_ammo[0]))              // Max ammo
            maxammo[indexnum] = value;
        else if (M_StringCompare(key, deh_ammo[1]))         // Per ammo
            clipammo[indexnum] = value;
        else if (M_StringCompare(key, "Name") || M_StringCompare(key, "Plural"))
        {
            for (int i = 0; i < NUMWEAPONS; i++)
                if (indexnum == weaponinfo[i].ammotype)
                {
                    if (M_StringCompare(key, "Name"))
                        M_StringCopy(weaponinfo[i].ammoname, lowercase(trimwhitespace(strval)), sizeof(weaponinfo[0].ammoname));
                    else if (M_StringCompare(key, "Plural"))
                        M_StringCopy(weaponinfo[i].ammoplural, lowercase(trimwhitespace(strval)), sizeof(weaponinfo[0].ammoplural));
                }
        }
        else
            C_Warning(1, "Invalid ammo string index for \"%s\".", key);
    }
}

// ====================================================================
// deh_procWeapon
// Purpose: Handle DEH Weapon block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procWeapon(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    int     indexnum;
    char    *strval;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX - 1);

    if (sscanf(inbuffer, "%s %i", key, &indexnum) != 2)
    {
        C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
        return;
    }

    if (devparm)
        C_Output("Processing Weapon at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMWEAPONS)
    {
        C_Warning(1, "Bad weapon number %i of %i.", indexnum, NUMWEAPONS);
        return;
    }

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        int bGetData;

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                                          // killough 11/98

        if (!(bGetData = deh_GetData(inbuffer, key, &value, &strval)))
        {
            C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (M_StringCompare(key, deh_weapon[0]))            // Ammo type
            weaponinfo[indexnum].ammotype = value;
        else if (M_StringCompare(key, deh_weapon[1]))       // Deselect frame
            weaponinfo[indexnum].upstate = value;
        else if (M_StringCompare(key, deh_weapon[2]))       // Select frame
            weaponinfo[indexnum].downstate = value;
        else if (M_StringCompare(key, deh_weapon[3]))       // Bobbing frame
            weaponinfo[indexnum].readystate = value;
        else if (M_StringCompare(key, deh_weapon[4]))       // Shooting frame
            weaponinfo[indexnum].atkstate = value;
        else if (M_StringCompare(key, deh_weapon[5]))       // Firing frame
            weaponinfo[indexnum].flashstate = value;
        else if (M_StringCompare(key, deh_weapon[6]))       // Ammo per shot
        {
            weaponinfo[indexnum].ammopershot = value;
            mbf21compatible = true;
        }
        else if (M_StringCompare(key, deh_weapon[7]))       // MBF21 bits
        {
            if (bGetData == 1)
                weaponinfo[indexnum].flags = value;
            else
            {
                for (value = 0; (strval = strtok(strval, ",+| \t\f\r")); strval = NULL)
                {
                    int iy;

                    for (iy = 0; iy < DEH_WEAPONFLAGMAX_MBF21; iy++)
                    {
                        if (!M_StringCompare(strval, deh_weaponflags_mbf21[iy].name))
                            continue;

                        value |= deh_weaponflags_mbf21[iy].value;
                        break;
                    }

                    if (iy >= DEH_WEAPONFLAGMAX_MBF21)
                        C_Warning(1, "Could not find MBF21 weapon bit mnemonic \"%s\".", strval);
                }

                weaponinfo[indexnum].flags = value;
            }

            mbf21compatible = true;
        }
        else if (M_StringCompare(key, "Name"))
            M_StringCopy(weaponinfo[indexnum].name, lowercase(trimwhitespace(strval)), sizeof(weaponinfo[0].name));
        else
            C_Warning(1, "Invalid weapon string index for \"%s\".", key);
    }
}

// ====================================================================
// deh_procSprite
// Purpose: Dummy - we do not support the DEH Sprite block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procSprite(DEHFILE *fpin, char *line)   // Not supported
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     indexnum;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX - 1);

    if (sscanf(inbuffer, "%s %i", key, &indexnum) != 2)
    {
        C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
        return;
    }

    // Too little is known about what this is supposed to do, and
    // there are better ways of handling sprite renaming. Not supported.
    C_Warning(1, "Ignoring sprite offset change at index %i: \"%s\".", indexnum, key);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98

        // ignore line
        if (devparm)
            C_Output("- %s", inbuffer);
    }
}

// ====================================================================
// deh_procPars
// Purpose: Handle BEX extension for PAR times
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procPars(DEHFILE *fpin, char *line)
{
    char    inbuffer[DEH_BUFFERMAX];
    int     ep;
    int     level;
    int     partime;

    boomcompatible = true;

    // new item, par times
    // usage: After [PARS] Par 0 section identifier, use one or more of these
    // lines:
    //  par 3 5 120
    //  par 14 230
    // The first would make the par for E3M5 be 120 seconds, and the
    // second one makes the par for MAP14 be 230 seconds. The number
    // of parameters on the line determines which group of par values
    // is being changed. Error checking is done based on current fixed
    // array sizes of [4][10] and [32]

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX - 1);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        if (*inbuffer == '#' || (*inbuffer == '/' && *(inbuffer + 1) == '/'))
            continue;                           // skip comment lines

        lfstrip(lowercase(inbuffer));           // lowercase it

        if (!*inbuffer)
            break;                              // killough 11/98

        if (sscanf(inbuffer, "par %10i %10i %10i", &ep, &level, &partime) != 3)
        {
            if (sscanf(inbuffer, "par %10i %10i", &level, &partime) != 2)
                C_Warning(1, "Invalid par time setting string \"%s\".", inbuffer);
            else
            {
                // Ty 07/11/98 - wrong range check, not zero-based
                if (level < 1 || level > 99)    // base 0 array (but 1-based parm)
                    C_Warning(1, "Invalid MAP" ITALICS("xy") " value MAP %02i.", level);
                else
                {
                    if (devparm)
                        C_Output("Changed par time for MAP%02i from %i to %i seconds", level, cpars[level - 1], partime);

                    cpars[level - 1] = partime;
                    newpars = true;
                }
            }
        }
        else
        {
            // note that though it's a [6][10] array, the "left" and "top" aren't used,
            // effectively making it a base 1 array.
            // Ty 07/11/98 - level was being checked against max 3 - dumb error
            if (ep < 1 || ep > 5 || level < 1 || level > 9)
                C_Warning(1, "Invalid ExMy values E%iM%i.", ep, level);
            else
            {
                if (devparm)
                    C_Output("Changed par time for E%iM%i from %i to %i seconds", ep, level, pars[ep][level], partime);

                pars[ep][level] = partime;
                newpars = true;
            }
        }
    }
}

// ====================================================================
// deh_procCheat
// Purpose: Handle DEH Cheat block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procCheat(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    char    ch = 0;         // CPhipps - `writable' null string to initialize...
    char    *strval = &ch;  // pointer to the value area
    int     iy;             // array index
    char    *p;             // utility pointer

    strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;              // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, &strval))       // returns true if ok
            continue;

        // Otherwise we got a (perhaps valid) cheat name
        if (M_StringCompare(key, deh_cheat[0]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_mus.sequence = M_StringDuplicate(p);
            cheat_mus_xy.sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[1]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_choppers.sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[2]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_god.sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[3]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_ammo.sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[4]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_ammonokey.sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[5]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_noclip.sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[6]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_commercial_noclip.sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[7]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[0].sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[8]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[1].sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[9]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[2].sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[10]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[3].sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[11]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[4].sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[12]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[5].sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[13]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[6].sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[14]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_clev.sequence = M_StringDuplicate(p);
            cheat_clev_xy.sequence = M_StringDuplicate(p);
        }
        else if (M_StringCompare(key, deh_cheat[15]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_mypos.sequence = M_StringDuplicate(p);
        }

        if (devparm)
            C_Output("- %s", inbuffer);
    }
}

// ====================================================================
// deh_procMisc
// Purpose: Handle DEH Misc block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procMisc(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;

    strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                                              // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))          // returns true if ok
        {
            C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        // Otherwise it's ok
        if (devparm)
            C_Output("Processing Misc item \"%s\"", key);

        if (M_StringCompare(key, deh_misc[0]))                      // Initial Health
            initial_health = MIN(value, 999);
        else if (M_StringCompare(key, deh_misc[1]))                 // Initial Bullets
            initial_bullets = value;
        else if (M_StringCompare(key, deh_misc[2]))                 // Max Health
            maxhealth = MIN(value, 999);
        else if (M_StringCompare(key, deh_misc[3]))                 // Max Armor
            max_armor = MIN(value, 999);
        else if (M_StringCompare(key, deh_misc[4]))                 // Green Armor Class
            green_armor_class = value;
        else if (M_StringCompare(key, deh_misc[5]))                 // Blue Armor Class
            blue_armor_class = value;
        else if (M_StringCompare(key, deh_misc[6]))                 // Max Soulsphere
            max_soul = value;
        else if (M_StringCompare(key, deh_misc[7]))                 // Soulsphere Health
            soul_health = value;
        else if (M_StringCompare(key, deh_misc[8]))                 // Megasphere Health
            mega_health = value;
        else if (M_StringCompare(key, deh_misc[9]))                 // God Mode Health
            god_health = value;
        else if (M_StringCompare(key, deh_misc[10]))                // IDFA Armor
            idfa_armor = value;
        else if (M_StringCompare(key, deh_misc[11]))                // IDFA Armor Class
            idfa_armor_class = value;
        else if (M_StringCompare(key, deh_misc[12]))                // IDKFA Armor
            idkfa_armor = value;
        else if (M_StringCompare(key, deh_misc[13]))                // IDKFA Armor Class
            idkfa_armor_class = value;
        else if (M_StringCompare(key, deh_misc[14]))                // BFG Cells/Shot
            weaponinfo[wp_bfg].ammopershot = bfgcells = value;
        else if (M_StringCompare(key, deh_misc[15]))                // Monsters Infight
        {
            if (value == 202)
                species_infighting = false;
            else if (value == 221)
                species_infighting = true;
        }
        else
            C_Warning(1, "Invalid misc item string index for \"%s\".", key);
    }

    maxhealth = MAX(maxhealth, initial_health);
}

// ====================================================================
// deh_procText
// Purpose: Handle DEH Text block
// Notes:   We look things up in the current information and if found
//          we replace it. At the same time we write the new and
//          improved BEX syntax to the log file for future use.
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procText(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX * 2];                    // can't use line -- double size buffer too.
    int     i;                                              // loop variable
    int     fromlen, tolen;                                 // as specified on the text block line
    bool    found = false;                                  // to allow early exit once found
    char    *line2 = NULL;                                  // duplicate line for rerouting

    // Ty 04/11/98 - Included file may have NOTEXT skip flag set
    if (includenotext)                                      // flag to skip included deh-style text
    {
        C_Output("Skipped text block because of NOTEXT directive.");
        strcpy(inbuffer, line);

        while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
            dehfgets(inbuffer, sizeof(inbuffer), fpin);     // skip block

        // Ty 05/17/98 - don't care if this fails
        return;
    }

    if (sscanf(line, "%s %i %10i", key, &fromlen, &tolen) != 3)
        return;

    if (devparm)
        C_Output("Processing Text (key = %s, from = %i, to = %i)", key, fromlen, tolen);

    // killough 10/98: fix incorrect usage of feof
    {
        int c;
        int totlen = 0;

        while (totlen < fromlen + tolen && (c = dehfgetc(fpin)) != EOF)
            if (c != '\r')
                inbuffer[totlen++] = c;

        inbuffer[totlen] = '\0';
    }

    // if the from and to are 4, this may be a sprite rename. Check it
    // against the array and process it as such if it matches. Remember
    // that the original names are (and should remain) uppercase.
    // Future: this will be from a separate [SPRITES] block.
    if (fromlen == 4 && tolen == 4)
    {
        i = 0;

        while (sprnames[i])                                 // null terminated list in info.c   // jff 3/19/98
        {                                                                                       // check pointer
            if (!strncasecmp(sprnames[i], inbuffer, fromlen))                                   // not first char
            {
                if (devparm)
                    C_Output("Changing name of sprite at index %i from %s to %*s", i, sprnames[i], tolen, &inbuffer[fromlen]);

                if (M_StringCompare(sprnames[i], "BAR1") || M_StringCompare(sprnames[i], "BEXP"))
                {
                    states[S_BAR1].nextstate = S_BAR2;
                    mobjinfo[MT_BARREL].frames = 2;
                }

                // Ty 03/18/98 - not using M_StringDuplicate because length is fixed

                // killough 10/98: but it's an array of pointers, so we must
                // use M_StringDuplicate unless we redeclare sprnames and change all else
                sprnames[i] = M_StringDuplicate(sprnames[i]);

                strncpy(sprnames[i], &inbuffer[fromlen], tolen);
                found = true;
                break;                                      // only one matches, quit early
            }

            i++;                                            // next array element
        }
    }

    if (!found && fromlen < 7 && tolen < 7)                 // lengths of music and sfx are 6 or shorter
    {
        int usedlen = (fromlen < tolen ? fromlen : tolen);  // shorter of fromlen and tolen if not matched

        if (fromlen != tolen && devparm)
            C_Warning(1, "Mismatched lengths from %i to %i. Using %i.", fromlen, tolen, usedlen);

        // Try sound effects entries - see sounds.c
        for (i = 1; i < NUMSFX; i++)
        {
            // avoid short prefix erroneous match
            if (strlen(S_sfx[i].name2) != fromlen)
                continue;

            if (!strncasecmp(S_sfx[i].name1, inbuffer, fromlen))
            {
                if (devparm)
                    C_Output("Changing name of sfx from %s to %*s", S_sfx[i].name1, usedlen, &inbuffer[fromlen]);

                strncpy(S_sfx[i].name1, &inbuffer[fromlen], 9);
                found = true;
                break;                                      // only one matches, quit early
            }
        }

        if (!found)                                         // not yet
        {
            // Try music name entries - see sounds.c
            for (i = 1; i < NUMMUSIC; i++)
            {
                // avoid short prefix erroneous match
                if (strlen(S_music[i].name2) != fromlen)
                    continue;

                if (!strncasecmp(S_music[i].name1, inbuffer, fromlen))
                {
                    if (devparm)
                        C_Output("Changing name of music from %s to %*s", S_music[i].name1, usedlen, &inbuffer[fromlen]);

                    strncpy(S_music[i].name1, &inbuffer[fromlen], 9);
                    found = true;
                    break;                                  // only one matches, quit early
                }
            }
        }
    }

    if (!found)                                             // Nothing we want to handle here -- see if strings can deal with it.
    {
        if (devparm)
            C_Output("Checking text area through strings for \"%.12s%s\" from = %i to = %i",
                inbuffer, (strlen(inbuffer) > 12 ? "..." : ""), fromlen, tolen);

        if (fromlen <= (int)strlen(inbuffer))
        {
            line2 = M_StringDuplicate(&inbuffer[fromlen]);
            inbuffer[fromlen] = '\0';
        }

        deh_procStringSub(NULL, inbuffer, trimwhitespace(line2));
    }

    free(line2);                                            // may be NULL, ignored by free()
}

static void deh_procError(DEHFILE *fpin, char *line)
{
    char    inbuffer[DEH_BUFFERMAX];

    strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

    if (devparm)
        C_Warning(1, "Ignoring \"%s\".", inbuffer);
}

// ====================================================================
// deh_procStrings
// Purpose: Handle BEX [STRINGS] extension
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procStrings(DEHFILE *fpin, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    int         value;
    char        *strval;                // holds the string value of the line
    static int  maxstrlen = 128;        // maximum string length, bumped 128 at a time as needed
                                        // holds the final result of the string after concatenation
    static char *holdstring;

    boomcompatible = true;

    if (devparm)
        C_Output("Processing extended string substitution");

    if (!holdstring)
        holdstring = malloc(maxstrlen * sizeof(*holdstring));

    *holdstring = '\0';                 // empty string to start with
    strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

    // Ty 04/24/98 - have to allow inbuffer to start with a blank for
    // the continuations of C1TEXT etc.
    while (!dehfeof(fpin) && *inbuffer)
    {
        int len;

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        if (*inbuffer == '#' || (*inbuffer == '/' && *(inbuffer + 1) == '/'))
            continue;                   // skip comment lines

        lfstrip(inbuffer);

        if (!*inbuffer && !*holdstring)
            break;                      // killough 11/98

        if (!*holdstring)               // first one--get the key
            if (!deh_GetData(inbuffer, key, &value, &strval))   // returns true if ok
            {
                C_Warning(1, "Bad data pair in \"%s\".", inbuffer);
                continue;
            }

        len = (int)strlen(inbuffer);

        while (strlen(holdstring) + len > (unsigned int)maxstrlen)
        {
            // killough 11/98: allocate enough the first time
            maxstrlen = (int)strlen(holdstring) + len;

            if (devparm)
                C_Output("* increased buffer from to %i for buffer size %i", maxstrlen, len);

            holdstring = I_Realloc(holdstring, maxstrlen * sizeof(*holdstring));
        }

        // concatenate the whole buffer if continuation or the value if first
        strcat(holdstring, ptr_lstrip(*holdstring ? inbuffer : strval));
        rstrip(holdstring);

        // delete any trailing blanks past the backslash
        // note that blanks before the backslash will be concatenated
        // but ones at the beginning of the next line will not, allowing
        // indentation in the file to read well without affecting the
        // string itself.
        if (holdstring[strlen(holdstring) - 1] == '\\')
        {
            holdstring[strlen(holdstring) - 1] = '\0';
            continue;           // ready to concatenate
        }

        if (*holdstring)        // didn't have a backslash, trap above would catch that
        {
            // go process the current string
            deh_procStringSub(key, NULL, trimwhitespace(holdstring));
            *holdstring = '\0';  // empty string for the next one
        }
    }
}

// ====================================================================
// deh_procStringSub
// Purpose: Common string parsing and handling routine for DEH and BEX
// Args:    key       -- place to put the mnemonic for the string if found
//          lookfor   -- original value string to look for
//          newstring -- string to put in its place if found
// Returns: bool: True if string found, false if not
//
static bool deh_procStringSub(char *key, char *lookfor, char *newstring)
{
    bool    found = false;  // loop exit flag

    for (int i = 0; i < deh_numstrlookup; i++)
        if ((found = (lookfor ? M_StringCompare(*deh_strlookup[i].ppstr, lookfor) : M_StringCompare(deh_strlookup[i].lookup, key))))
        {
            char    *t;

            if (deh_strlookup[i].assigned)
                break;

            *deh_strlookup[i].ppstr = t = M_StringDuplicate(newstring);     // orphan originalstring

            // Handle embedded \n's in the incoming string, convert to 0x0A's
            for (char *s = *deh_strlookup[i].ppstr; *s; s++, t++)
                if (*s == '\\' && (s[1] == 'n' || s[1] == 'N'))             // found one
                {
                    s++;
                    *t = '\n';                                              // skip one extra for second character
                }
                else if (*s == '_')
                    *t = ITALICSTOGGLECHAR;
                else
                    *t = *s;

            *t = '\0';                                                      // cap off the target string

            if (devparm)
            {
                if (key)
                    C_Output("Assigned key %s to \"%s\"", key, newstring);
                else if (lookfor)
                {
                    C_Output("Assigned \"%.12s%s\" to \"%.12s%s\" at key %s", lookfor, (strlen(lookfor) > 12 ? "..." : ""),
                        newstring, (strlen(newstring) > 12 ? "..." : ""), deh_strlookup[i].lookup);
                    C_Output("*BEX FORMAT:");
                    C_Output("%s = %s", deh_strlookup[i].lookup, dehReformatStr(newstring));
                    C_Output("*END BEX");
                }
            }

            deh_strlookup[i].assigned++;

            if (M_StrCaseStr(deh_strlookup[i].lookup, "HUSTR"))
                addtodehmaptitlecount = true;

            // [BH] allow either GOTREDSKUL or GOTREDSKULL
            if (M_StringCompare(deh_strlookup[i].lookup, "GOTREDSKUL") && !deh_strlookup[p_GOTREDSKULL].assigned)
            {
                s_GOTREDSKULL = s_GOTREDSKUL;
                deh_strlookup[p_GOTREDSKULL].assigned++;
                return true;
            }

            break;
        }

    if (!found && !hacx && lookfor)
    {
        M_StringReplaceAll(lookfor, "\n", "", false);
        M_StringReplaceAll(lookfor, "\t", "", false);
        C_Warning(1, "The " BOLD("\"%s\"") " string can't be found.", lookfor);
    }

    return found;
}

//
// deh_procBexSprites
//
// Supports sprite name substitutions without requiring use
// of the DeHackEd Text block
//
static void deh_procBexSprites(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    char    *strval;    // holds the string value of the line
    char    candidate[5];
    int     rover;

    if (devparm)
        C_Output("Processing sprite name substitution");

    strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        if (*inbuffer == '#' || (*inbuffer == '/' && *(inbuffer + 1) == '/'))
            continue;   // skip comment lines

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, &strval))    // returns true if ok
        {
            C_Warning(1, "Bad data pair in \"%s\"", inbuffer);
            continue;
        }

        // do it
        memset(candidate, 0, sizeof(candidate));
        M_StringCopy(candidate, ptr_lstrip(strval), sizeof(candidate));

        if (strlen(candidate) != 4)
        {
            C_Warning(1, "Bad length for sprite name \"%s\"", candidate);
            continue;
        }

        rover = 0;

        while (deh_spritenames[rover])
        {
            if (!strncasecmp(deh_spritenames[rover], key, 4))
            {
                if (devparm)
                    C_Output("Substituting \"%s\" for sprite \"%s\"", candidate, deh_spritenames[rover]);

                if (M_StringCompare(candidate, "BAR1") || M_StringCompare(candidate, "BEXP"))
                {
                    states[S_BAR1].nextstate = S_BAR2;
                    mobjinfo[MT_BARREL].frames = 2;
                }

                sprnames[rover] = M_StringDuplicate(candidate);
                break;
            }

            rover++;
        }
    }
}

// ditto for sound names
static void deh_procBexSounds(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    char    *strval;    // holds the string value of the line
    char    candidate[7];
    int     rover;
    size_t  len;

    if (devparm)
        C_Output("Processing sound name substitution");

    strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        if (*inbuffer == '#' || (*inbuffer == '/' && *(inbuffer + 1) == '/'))
            continue;   // skip comment lines

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, &strval))   // returns true if ok
        {
            C_Warning(1, "Bad data pair in \"%s\"\n", inbuffer);
            continue;
        }

        // do it
        memset(candidate, 0, 7);
        M_StringCopy(candidate, ptr_lstrip(strval), sizeof(candidate));
        len = strlen(candidate);

        if (len < 1 || len > 6)
        {
            C_Warning(1, "Bad length for sound name \"%s\"\n", candidate);
            continue;
        }

        rover = 1;

        while (deh_soundnames[rover])
        {
            if (!strncasecmp(deh_soundnames[rover], key, 6))
            {
                if (devparm)
                    C_Output("Substituting \"%s\" for sound \"%s\"\n", candidate, deh_soundnames[rover]);

                M_StringCopy(S_sfx[rover].name1, candidate, sizeof(S_sfx[0].name1));
                break;
            }

            rover++;
        }
    }
}

// ditto for music names
static void deh_procBexMusic(DEHFILE *fpin, char *line)
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     value;
    char    *strval;    // holds the string value of the line
    char    candidate[7];
    int     rover;
    size_t  len;

    if (devparm)
        C_Output("Processing music name substitution");

    strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        if (*inbuffer == '#' || (*inbuffer == '/' && *(inbuffer + 1) == '/'))
            continue;   // skip comment lines

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, &strval))   // returns true if ok
        {
            C_Warning(1, "Bad data pair in \"%s\"", inbuffer);
            continue;
        }

        // do it
        memset(candidate, 0, 7);
        M_StringCopy(candidate, ptr_lstrip(strval), sizeof(candidate));
        len = strlen(candidate);

        if (len < 1 || len > 6)
        {
            C_Warning(1, "Bad length for music name \"%s\"", candidate);
            continue;
        }

        rover = 1;

        while (deh_musicnames[rover])
        {
            if (!strncasecmp(deh_musicnames[rover], key, 6))
            {
                if (devparm)
                    C_Output("Substituting \"%s\" for music \"%s\"", candidate, deh_musicnames[rover]);

                M_StringCopy(S_music[rover].name1, candidate, sizeof(S_music[0].name1));
                break;
            }

            rover++;
        }
    }
}

// ====================================================================
// General utility function(s)
// ====================================================================

// ====================================================================
// dehReformatStr
// Purpose: Convert a string into a continuous string with embedded
//          linefeeds for "\n" sequences in the source string
// Args:    string -- the string to convert
// Returns: the converted string (converted in a static buffer)
//
static char *dehReformatStr(char *string)
{
    static char buff[DEH_BUFFERMAX];    // only processing the changed string,
    //  don't need double buffer
    char        *s = string;            // source
    char        *t = buff;              // target

    // let's play...
    while (*s)
    {
        if (*s == '\n')
        {
            s++;
            *t++ = '\\';
            *t++ = 'n';
            *t++ = '\\';
            *t++ = '\n';
        }
        else
            *t++ = *s++;
    }

    *t = '\0';
    return buff;
}

// ====================================================================
// lfstrip
// Purpose: Strips CR/LF off the end of a string
// Args:    s -- the string to work on
// Returns: void -- the string is modified in place
//
// killough 10/98: only strip at end of line, not entire string
//
static void lfstrip(char *s)        // strip the \r and/or \n off of a line
{
    char    *p = s + strlen(s);

    while (p > s && (*(--p) == '\r' || *p == '\n'))
        *p = 0;
}

// ====================================================================
// rstrip
// Purpose: Strips trailing blanks off a string
// Args:    s -- the string to work on
// Returns: void -- the string is modified in place
//
static void rstrip(char *s)                         // strip trailing whitespace
{
    char    *p = s + strlen(s);                     // killough 04/04/98: same here

    while (p > s && isspace((unsigned char)*(--p))) // break on first non-whitespace
        *p = '\0';
}

// ====================================================================
// ptr_lstrip
// Purpose: Points past leading whitespace in a string
// Args:    s -- the string to work on
// Returns: char * pointing to the first nonblank character in the
//          string. The original string is not changed.
//
static char *ptr_lstrip(char *p)    // point past leading whitespace
{
    while (isspace((unsigned char)*p))
        p++;

    return p;
}

// ====================================================================
// deh_GetData
// Purpose: Get a key and data pair from a passed string
// Args:    s -- the string to be examined
//          k -- a place to put the key
//          l -- pointer to an integer to store the number
//          strval -- a pointer to the place in s where the number
//                    value comes from. Pass NULL to not use this.
// Notes:   Expects a key phrase, optional space, equal sign,
//          optional space and a value. The passed pointer to hold
//          the key must be DEH_MAXKEYLEN in size.
//
static int deh_GetData(char *s, char *k, int *l, char **strval)
{
    char            *t;                     // current char
    unsigned int    val;                    // to hold value of pair
    char            buffer[DEH_MAXKEYLEN];  // to hold key in progress
    int             okrc = 1;               // assume good unless we have problems
    int             i;                      // iterator

    *buffer = '\0';
    val = 0;                                // defaults in case not otherwise set

    for (i = 0, t = s; *t && i < DEH_MAXKEYLEN; t++, i++)
    {
        if (*t == '=')
            break;

        buffer[i] = *t;                     // copy it
    }

    if (i >= 1 && isspace((unsigned char)buffer[i - 1]))
        i--;

    buffer[i] = '\0';                       // terminate the key before the '='

    if (!*t)                                // end of string with no equal sign
        okrc = 0;
    else
    {
        if (!*++t)
            okrc = 0;                       // in case "thiskey =" with no value

        // we've incremented t
        if (!M_StrToInt(t, (int *)&val))
        {
            val = 0;
            okrc = 2;
        }
    }

    // go put the results in the passed pointers
    *l = val;                           // may be a faked zero

    // if spaces between key and equal sign, strip them
    strcpy(k, ptr_lstrip(buffer));      // could be a zero-length string

    if (strval)                         // pass NULL if you don't want this back
        *strval = t;                    // pointer, has to be somewhere in s,
                                        // even if pointing at the zero byte.
    return okrc;
}

void D_TranslateDehStrings(void)
{
    if (english == english_international)
    {
        for (int i = 0; i < deh_numstrlookup; i++)
            if (deh_strlookup[i].assigned)
                M_AmericanToInternationalEnglish(*deh_strlookup[i].ppstr);
    }
    else
        for (int i = 0; i < deh_numstrlookup; i++)
            if (deh_strlookup[i].assigned)
                M_InternationalToAmericanEnglish(*deh_strlookup[i].ppstr);
}

static deh_bexptr   null_bexptr = { NULL, "(NULL)" };

void D_PostProcessDeh(void)
{
    const deh_bexptr    *bexptr_match;

    for (int i = 0, j; i < NUMSTATES; i++)
    {
        bexptr_match = &null_bexptr;

        for (j = 0; deh_bexptrs[j].cptr; j++)
            if (states[i].action == deh_bexptrs[j].cptr)
            {
                bexptr_match = &deh_bexptrs[j];
                break;
            }

        // ensure states don't use more MBF21 args than their
        // action pointer expects, for future-proofing's sake
        for (j = MAXSTATEARGS - 1; j >= bexptr_match->argcount; j--)
            if (states[i].args[j])
                I_Error("Action %s on state %i expects no more than %i nonzero args (%i found). Check your dehacked.",
                    bexptr_match->lookup, i, bexptr_match->argcount, j + 1);

        // replace unset fields with default values
        for (; j >= 0; j--)
            if (!(defined_codeptr_args[i] & (1 << j)))
                states[i].args[j] = bexptr_match->default_args[j];
    }

    if (english == english_international)
        D_TranslateDehStrings();
}
