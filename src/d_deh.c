/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#include <ctype.h>

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "i_system.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "p_local.h"
#include "sounds.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

// killough 10/98: new functions, to allow processing DEH files in-memory
// (e.g. from wads)

typedef struct
{
    byte    *inp;
    byte    *lump;
    long    size;
    FILE    *f;
} DEHFILE;

static dboolean addtocount;
static int      linecount;

int             dehcount;
dboolean        dehacked;

// killough 10/98: emulate IO whether input really comes from a file or not

// haleyjd: got rid of macros for MSVC
static char *dehfgets(char *buf, int n, DEHFILE *fp)
{
    if (!fp->lump)                              // If this is a real file,
        return fgets(buf, n, fp->f);            // return regular fgets

    if (!n || !*fp->inp || fp->size <= 0)       // If no more characters
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
    return (!fp->lump ? feof(fp->f) : !*fp->inp || fp->size <= 0);
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

char    *s_D_DEVSTR = D_DEVSTR;
char    *s_D_CDROM = D_CDROM;

char    *s_PRESSKEY = PRESSKEY;
char    *s_PRESSYN = PRESSYN;
char    *s_PRESSA = "";
char    *s_QUITMSG = QUITMSG;
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
char    *s_OTHERY = "";
char    *s_OTHERA = "";
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
char    *s_GOTCLIPX2 = "";
char    *s_GOTHALFCLIP = "";
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

char    *s_SECRET = "";

char    *s_GGSAVED = GGSAVED;
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
char    *s_HUSTR_E1M5 = HUSTR_E1M5;
char    *s_HUSTR_E1M6 = HUSTR_E1M6;
char    *s_HUSTR_E1M7 = HUSTR_E1M7;
char    *s_HUSTR_E1M8 = HUSTR_E1M8;
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
char    *s_STSTR_BUDDHA = "";
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
char    *s_STSTR_VMON = "";
char    *s_STSTR_VMOFF = "";

char    *s_E1TEXT = E1TEXT;
char    *s_E2TEXT = E2TEXT;
char    *s_E3TEXT = E3TEXT;
char    *s_E4TEXT = E4TEXT;
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
char    *s_CC_SPECTRE = "";
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

char    *s_DOOM_ENDMSG1 = DOOM_ENDMSG1;
char    *s_DOOM_ENDMSG2 = DOOM_ENDMSG2;
char    *s_DOOM_ENDMSG3 = DOOM_ENDMSG3;
char    *s_DOOM_ENDMSG4 = DOOM_ENDMSG4;
char    *s_DOOM_ENDMSG5 = DOOM_ENDMSG5;
char    *s_DOOM_ENDMSG6 = DOOM_ENDMSG6;
char    *s_DOOM_ENDMSG7 = DOOM_ENDMSG7;
char    *s_DOOM2_ENDMSG1 = DOOM2_ENDMSG1;
char    *s_DOOM2_ENDMSG2 = DOOM2_ENDMSG2;
char    *s_DOOM2_ENDMSG3 = DOOM2_ENDMSG3;
char    *s_DOOM2_ENDMSG4 = DOOM2_ENDMSG4;
char    *s_DOOM2_ENDMSG5 = DOOM2_ENDMSG5;
char    *s_DOOM2_ENDMSG6 = DOOM2_ENDMSG6;
char    *s_DOOM2_ENDMSG7 = DOOM2_ENDMSG7;

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
char    *s_M_GAMEPADSENSITIVITY = "";
char    *s_M_SOUNDVOLUME = "";
char    *s_M_SFXVOLUME = "";
char    *s_M_MUSICVOLUME = "";
char    *s_M_PAUSED = "";

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
char    *s_CAPTION_BTSXE3 = "";
char    *s_CAPTION_E1M4B = "";
char    *s_CAPTION_E1M8B = "";

char    *s_AUTHOR_BESTOR = "";
char    *s_AUTHOR_ROMERO = "";

char    *bgflatE1 = "FLOOR4_8";
char    *bgflatE2 = "SFLR6_1";
char    *bgflatE3 = "MFLR8_4";
char    *bgflatE4 = "MFLR8_3";
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

// end d_deh.h variable declarations
// ====================================================================

// Do this for a lookup--the pointer (loaded above) is cross-referenced
// to a string key that is the same as the define above. We will use
// strdups to set these new values that we read from the file, orphaning
// the original value set above.

deh_strs deh_strlookup[] =
{
    { &s_D_DEVSTR,             "D_DEVSTR"             },
    { &s_D_CDROM,              "D_CDROM"              },

    { &s_PRESSKEY,             "PRESSKEY"             },
    { &s_PRESSYN,              "PRESSYN"              },
    { &s_PRESSA,               "PRESSA"               },
    { &s_QUITMSG,              "QUITMSG"              },
    { &s_LOADNET,              "LOADNET"              },
    { &s_QLOADNET,             "QLOADNET"             },
    { &s_QSAVESPOT,            "QSAVESPOT"            },
    { &s_SAVEDEAD,             "SAVEDEAD"             },
    { &s_QSPROMPT,             "QSPROMPT"             },
    { &s_QLPROMPT,             "QLPROMPT"             },
    { &s_DELPROMPT,            "DELPROMPT"            },
    { &s_NEWGAME,              "NEWGAME"              },
    { &s_NIGHTMARE,            "NIGHTMARE"            },
    { &s_SWSTRING,             "SWSTRING"             },
    { &s_MSGOFF,               "MSGOFF"               },
    { &s_MSGON,                "MSGON"                },
    { &s_NETEND,               "NETEND"               },
    { &s_ENDGAME,              "ENDGAME"              },
    { &s_DOSY,                 "DOSY"                 },
    { &s_DOSA,                 "DOSA"                 },
    { &s_OTHERY,               "OTHERY"               },
    { &s_OTHERA,               "OTHERA"               },
    { &s_DETAILHI,             "DETAILHI"             },
    { &s_DETAILLO,             "DETAILLO"             },
    { &s_GAMMALVL0,            "GAMMALVL0"            },
    { &s_GAMMALVL1,            "GAMMALVL1"            },
    { &s_GAMMALVL2,            "GAMMALVL2"            },
    { &s_GAMMALVL3,            "GAMMALVL3"            },
    { &s_GAMMALVL4,            "GAMMALVL4"            },
    { &s_GAMMALVL,             "GAMMALVL"             },
    { &s_GAMMAOFF,             "GAMMAOFF"             },
    { &s_EMPTYSTRING,          "EMPTYSTRING"          },

    { &s_GOTARMOR,             "GOTARMOR"             },
    { &s_GOTMEGA,              "GOTMEGA"              },
    { &s_GOTHTHBONUS,          "GOTHTHBONUS"          },
    { &s_GOTARMBONUS,          "GOTARMBONUS"          },
    { &s_GOTSTIM,              "GOTSTIM"              },
    { &s_GOTMEDINEED,          "GOTMEDINEED"          },
    { &s_GOTMEDIKIT,           "GOTMEDIKIT"           },
    { &s_GOTSUPER,             "GOTSUPER"             },

    { &s_GOTBLUECARD,          "GOTBLUECARD"          },
    { &s_GOTYELWCARD,          "GOTYELWCARD"          },
    { &s_GOTREDCARD,           "GOTREDCARD"           },
    { &s_GOTBLUESKUL,          "GOTBLUESKUL"          },
    { &s_GOTYELWSKUL,          "GOTYELWSKUL"          },
    { &s_GOTREDSKUL,           "GOTREDSKUL"           },
    { &s_GOTREDSKULL,          "GOTREDSKULL"          },

    { &s_GOTINVUL,             "GOTINVUL"             },
    { &s_GOTBERSERK,           "GOTBERSERK"           },
    { &s_GOTINVIS,             "GOTINVIS"             },
    { &s_GOTSUIT,              "GOTSUIT"              },
    { &s_GOTMAP,               "GOTMAP"               },
    { &s_GOTVISOR,             "GOTVISOR"             },

    { &s_GOTCLIP,              "GOTCLIP"              },
    { &s_GOTCLIPX2,            "GOTCLIPX2"            },
    { &s_GOTHALFCLIP,          "GOTHALFCLIP"          },
    { &s_GOTCLIPBOX,           "GOTCLIPBOX"           },
    { &s_GOTROCKET,            "GOTROCKET"            },
    { &s_GOTROCKETX2,          "GOTROCKETX2"          },
    { &s_GOTROCKBOX,           "GOTROCKBOX"           },
    { &s_GOTCELL,              "GOTCELL"              },
    { &s_GOTCELLX2,            "GOTCELLX2"            },
    { &s_GOTCELLBOX,           "GOTCELLBOX"           },
    { &s_GOTSHELLS,            "GOTSHELLS"            },
    { &s_GOTSHELLSX2,          "GOTSHELLSX2"          },
    { &s_GOTSHELLBOX,          "GOTSHELLBOX"          },
    { &s_GOTBACKPACK,          "GOTBACKPACK"          },

    { &s_GOTBFG9000,           "GOTBFG9000"           },
    { &s_GOTCHAINGUN,          "GOTCHAINGUN"          },
    { &s_GOTCHAINSAW,          "GOTCHAINSAW"          },
    { &s_GOTLAUNCHER,          "GOTLAUNCHER"          },
    { &s_GOTMSPHERE,           "GOTMSPHERE"           },
    { &s_GOTPLASMA,            "GOTPLASMA"            },
    { &s_GOTSHOTGUN,           "GOTSHOTGUN"           },
    { &s_GOTSHOTGUN2,          "GOTSHOTGUN2"          },

    { &s_PD_BLUEO,             "PD_BLUEO"             },
    { &s_PD_REDO,              "PD_REDO"              },
    { &s_PD_YELLOWO,           "PD_YELLOWO"           },
    { &s_PD_BLUEK,             "PD_BLUEK"             },
    { &s_PD_REDK,              "PD_REDK"              },
    { &s_PD_YELLOWK,           "PD_YELLOWK"           },
    { &s_PD_BLUEC,             "PD_BLUEC"             },
    { &s_PD_REDC,              "PD_REDC"              },
    { &s_PD_YELLOWC,           "PD_YELLOWC"           },
    { &s_PD_BLUES,             "PD_BLUES"             },
    { &s_PD_REDS,              "PD_REDS"              },
    { &s_PD_YELLOWS,           "PD_YELLOWS"           },
    { &s_PD_ANY,               "PD_ANY"               },
    { &s_PD_ALL3,              "PD_ALL3"              },
    { &s_PD_ALL6,              "PD_ALL6"              },

    { &s_SECRET,               "SECRET"               },

    { &s_GGSAVED,              "GGSAVED"              },
    { &s_GGLOADED,             "GGLOADED"             },
    { &s_GGAUTOLOADED,         "GGAUTOLOADED"         },
    { &s_GGDELETED,            "GGDELETED"            },
    { &s_GSCREENSHOT,          "GSCREENSHOT"          },

    { &s_ALWAYSRUNOFF,         "ALWAYSRUNOFF"         },
    { &s_ALWAYSRUNON,          "ALWAYSRUNON"          },

    { &s_HUSTR_MSGU,           "HUSTR_MSGU"           },

    { &s_HUSTR_E1M1,           "HUSTR_E1M1"           },
    { &s_HUSTR_E1M2,           "HUSTR_E1M2"           },
    { &s_HUSTR_E1M3,           "HUSTR_E1M3"           },
    { &s_HUSTR_E1M4,           "HUSTR_E1M4"           },
    { &s_HUSTR_E1M5,           "HUSTR_E1M5"           },
    { &s_HUSTR_E1M6,           "HUSTR_E1M6"           },
    { &s_HUSTR_E1M7,           "HUSTR_E1M7"           },
    { &s_HUSTR_E1M8,           "HUSTR_E1M8"           },
    { &s_HUSTR_E1M9,           "HUSTR_E1M9"           },
    { &s_HUSTR_E2M1,           "HUSTR_E2M1"           },
    { &s_HUSTR_E2M2,           "HUSTR_E2M2"           },
    { &s_HUSTR_E2M3,           "HUSTR_E2M3"           },
    { &s_HUSTR_E2M4,           "HUSTR_E2M4"           },
    { &s_HUSTR_E2M5,           "HUSTR_E2M5"           },
    { &s_HUSTR_E2M6,           "HUSTR_E2M6"           },
    { &s_HUSTR_E2M7,           "HUSTR_E2M7"           },
    { &s_HUSTR_E2M8,           "HUSTR_E2M8"           },
    { &s_HUSTR_E2M9,           "HUSTR_E2M9"           },
    { &s_HUSTR_E3M1,           "HUSTR_E3M1"           },
    { &s_HUSTR_E3M2,           "HUSTR_E3M2"           },
    { &s_HUSTR_E3M3,           "HUSTR_E3M3"           },
    { &s_HUSTR_E3M4,           "HUSTR_E3M4"           },
    { &s_HUSTR_E3M5,           "HUSTR_E3M5"           },
    { &s_HUSTR_E3M6,           "HUSTR_E3M6"           },
    { &s_HUSTR_E3M7,           "HUSTR_E3M7"           },
    { &s_HUSTR_E3M8,           "HUSTR_E3M8"           },
    { &s_HUSTR_E3M9,           "HUSTR_E3M9"           },
    { &s_HUSTR_E4M1,           "HUSTR_E4M1"           },
    { &s_HUSTR_E4M2,           "HUSTR_E4M2"           },
    { &s_HUSTR_E4M3,           "HUSTR_E4M3"           },
    { &s_HUSTR_E4M4,           "HUSTR_E4M4"           },
    { &s_HUSTR_E4M5,           "HUSTR_E4M5"           },
    { &s_HUSTR_E4M6,           "HUSTR_E4M6"           },
    { &s_HUSTR_E4M7,           "HUSTR_E4M7"           },
    { &s_HUSTR_E4M8,           "HUSTR_E4M8"           },
    { &s_HUSTR_E4M9,           "HUSTR_E4M9"           },
    { &s_HUSTR_1,              "HUSTR_1"              },
    { &s_HUSTR_2,              "HUSTR_2"              },
    { &s_HUSTR_3,              "HUSTR_3"              },
    { &s_HUSTR_4,              "HUSTR_4"              },
    { &s_HUSTR_5,              "HUSTR_5"              },
    { &s_HUSTR_6,              "HUSTR_6"              },
    { &s_HUSTR_7,              "HUSTR_7"              },
    { &s_HUSTR_8,              "HUSTR_8"              },
    { &s_HUSTR_9,              "HUSTR_9"              },
    { &s_HUSTR_10,             "HUSTR_10"             },
    { &s_HUSTR_11,             "HUSTR_11"             },
    { &s_HUSTR_12,             "HUSTR_12"             },
    { &s_HUSTR_13,             "HUSTR_13"             },
    { &s_HUSTR_14,             "HUSTR_14"             },
    { &s_HUSTR_15,             "HUSTR_15"             },
    { &s_HUSTR_16,             "HUSTR_16"             },
    { &s_HUSTR_17,             "HUSTR_17"             },
    { &s_HUSTR_18,             "HUSTR_18"             },
    { &s_HUSTR_19,             "HUSTR_19"             },
    { &s_HUSTR_20,             "HUSTR_20"             },
    { &s_HUSTR_21,             "HUSTR_21"             },
    { &s_HUSTR_22,             "HUSTR_22"             },
    { &s_HUSTR_23,             "HUSTR_23"             },
    { &s_HUSTR_24,             "HUSTR_24"             },
    { &s_HUSTR_25,             "HUSTR_25"             },
    { &s_HUSTR_26,             "HUSTR_26"             },
    { &s_HUSTR_27,             "HUSTR_27"             },
    { &s_HUSTR_28,             "HUSTR_28"             },
    { &s_HUSTR_29,             "HUSTR_29"             },
    { &s_HUSTR_30,             "HUSTR_30"             },
    { &s_HUSTR_31,             "HUSTR_31"             },
    { &s_HUSTR_31_BFG,         "HUSTR_31_BFG"         },
    { &s_HUSTR_32,             "HUSTR_32"             },
    { &s_HUSTR_32_BFG,         "HUSTR_32_BFG"         },
    { &s_HUSTR_33,             "HUSTR_33"             },
    { &s_PHUSTR_1,             "PHUSTR_1"             },
    { &s_PHUSTR_2,             "PHUSTR_2"             },
    { &s_PHUSTR_3,             "PHUSTR_3"             },
    { &s_PHUSTR_4,             "PHUSTR_4"             },
    { &s_PHUSTR_5,             "PHUSTR_5"             },
    { &s_PHUSTR_6,             "PHUSTR_6"             },
    { &s_PHUSTR_7,             "PHUSTR_7"             },
    { &s_PHUSTR_8,             "PHUSTR_8"             },
    { &s_PHUSTR_9,             "PHUSTR_9"             },
    { &s_PHUSTR_10,            "PHUSTR_10"            },
    { &s_PHUSTR_11,            "PHUSTR_11"            },
    { &s_PHUSTR_12,            "PHUSTR_12"            },
    { &s_PHUSTR_13,            "PHUSTR_13"            },
    { &s_PHUSTR_14,            "PHUSTR_14"            },
    { &s_PHUSTR_15,            "PHUSTR_15"            },
    { &s_PHUSTR_16,            "PHUSTR_16"            },
    { &s_PHUSTR_17,            "PHUSTR_17"            },
    { &s_PHUSTR_18,            "PHUSTR_18"            },
    { &s_PHUSTR_19,            "PHUSTR_19"            },
    { &s_PHUSTR_20,            "PHUSTR_20"            },
    { &s_PHUSTR_21,            "PHUSTR_21"            },
    { &s_PHUSTR_22,            "PHUSTR_22"            },
    { &s_PHUSTR_23,            "PHUSTR_23"            },
    { &s_PHUSTR_24,            "PHUSTR_24"            },
    { &s_PHUSTR_25,            "PHUSTR_25"            },
    { &s_PHUSTR_26,            "PHUSTR_26"            },
    { &s_PHUSTR_27,            "PHUSTR_27"            },
    { &s_PHUSTR_28,            "PHUSTR_28"            },
    { &s_PHUSTR_29,            "PHUSTR_29"            },
    { &s_PHUSTR_30,            "PHUSTR_30"            },
    { &s_PHUSTR_31,            "PHUSTR_31"            },
    { &s_PHUSTR_32,            "PHUSTR_32"            },
    { &s_THUSTR_1,             "THUSTR_1"             },
    { &s_THUSTR_2,             "THUSTR_2"             },
    { &s_THUSTR_3,             "THUSTR_3"             },
    { &s_THUSTR_4,             "THUSTR_4"             },
    { &s_THUSTR_5,             "THUSTR_5"             },
    { &s_THUSTR_6,             "THUSTR_6"             },
    { &s_THUSTR_7,             "THUSTR_7"             },
    { &s_THUSTR_8,             "THUSTR_8"             },
    { &s_THUSTR_9,             "THUSTR_9"             },
    { &s_THUSTR_10,            "THUSTR_10"            },
    { &s_THUSTR_11,            "THUSTR_11"            },
    { &s_THUSTR_12,            "THUSTR_12"            },
    { &s_THUSTR_13,            "THUSTR_13"            },
    { &s_THUSTR_14,            "THUSTR_14"            },
    { &s_THUSTR_15,            "THUSTR_15"            },
    { &s_THUSTR_16,            "THUSTR_16"            },
    { &s_THUSTR_17,            "THUSTR_17"            },
    { &s_THUSTR_18,            "THUSTR_18"            },
    { &s_THUSTR_19,            "THUSTR_19"            },
    { &s_THUSTR_20,            "THUSTR_20"            },
    { &s_THUSTR_21,            "THUSTR_21"            },
    { &s_THUSTR_22,            "THUSTR_22"            },
    { &s_THUSTR_23,            "THUSTR_23"            },
    { &s_THUSTR_24,            "THUSTR_24"            },
    { &s_THUSTR_25,            "THUSTR_25"            },
    { &s_THUSTR_26,            "THUSTR_26"            },
    { &s_THUSTR_27,            "THUSTR_27"            },
    { &s_THUSTR_28,            "THUSTR_28"            },
    { &s_THUSTR_29,            "THUSTR_29"            },
    { &s_THUSTR_30,            "THUSTR_30"            },
    { &s_THUSTR_31,            "THUSTR_31"            },
    { &s_THUSTR_32,            "THUSTR_32"            },
    { &s_NHUSTR_1,             "NHUSTR_1"             },
    { &s_NHUSTR_2,             "NHUSTR_2"             },
    { &s_NHUSTR_3,             "NHUSTR_3"             },
    { &s_NHUSTR_4,             "NHUSTR_4"             },
    { &s_NHUSTR_5,             "NHUSTR_5"             },
    { &s_NHUSTR_6,             "NHUSTR_6"             },
    { &s_NHUSTR_7,             "NHUSTR_7"             },
    { &s_NHUSTR_8,             "NHUSTR_8"             },
    { &s_NHUSTR_9,             "NHUSTR_9"             },

    { &s_HUSTR_CHATMACRO1,     "HUSTR_CHATMACRO1"     },
    { &s_HUSTR_CHATMACRO2,     "HUSTR_CHATMACRO2"     },
    { &s_HUSTR_CHATMACRO3,     "HUSTR_CHATMACRO3"     },
    { &s_HUSTR_CHATMACRO4,     "HUSTR_CHATMACRO4"     },
    { &s_HUSTR_CHATMACRO5,     "HUSTR_CHATMACRO5"     },
    { &s_HUSTR_CHATMACRO6,     "HUSTR_CHATMACRO6"     },
    { &s_HUSTR_CHATMACRO7,     "HUSTR_CHATMACRO7"     },
    { &s_HUSTR_CHATMACRO8,     "HUSTR_CHATMACRO8"     },
    { &s_HUSTR_CHATMACRO9,     "HUSTR_CHATMACRO9"     },
    { &s_HUSTR_CHATMACRO0,     "HUSTR_CHATMACRO0"     },
    { &s_HUSTR_TALKTOSELF1,    "HUSTR_TALKTOSELF1"    },
    { &s_HUSTR_TALKTOSELF2,    "HUSTR_TALKTOSELF2"    },
    { &s_HUSTR_TALKTOSELF3,    "HUSTR_TALKTOSELF3"    },
    { &s_HUSTR_TALKTOSELF4,    "HUSTR_TALKTOSELF4"    },
    { &s_HUSTR_TALKTOSELF5,    "HUSTR_TALKTOSELF5"    },
    { &s_HUSTR_MESSAGESENT,    "HUSTR_MESSAGESENT"    },
    { &s_HUSTR_PLRGREEN,       "HUSTR_PLRGREEN"       },
    { &s_HUSTR_PLRINDIGO,      "HUSTR_PLRINDIGO"      },
    { &s_HUSTR_PLRBROWN,       "HUSTR_PLRBROWN"       },
    { &s_HUSTR_PLRRED,         "HUSTR_PLRRED"         },

    { &s_AMSTR_FOLLOWON,       "AMSTR_FOLLOWON"       },
    { &s_AMSTR_FOLLOWOFF,      "AMSTR_FOLLOWOFF"      },
    { &s_AMSTR_GRIDON,         "AMSTR_GRIDON"         },
    { &s_AMSTR_GRIDOFF,        "AMSTR_GRIDOFF"        },
    { &s_AMSTR_MARKEDSPOT,     "AMSTR_MARKEDSPOT"     },
    { &s_AMSTR_MARKCLEARED,    "AMSTR_MARKCLEARED"    },
    { &s_AMSTR_MARKSCLEARED,   "AMSTR_MARKSCLEARED"   },
    { &s_AMSTR_ROTATEON,       "AMSTR_ROTATEON"       },
    { &s_AMSTR_ROTATEOFF,      "AMSTR_ROTATEOFF"      },

    { &s_STSTR_MUS,            "STSTR_MUS"            },
    { &s_STSTR_NOMUS,          "STSTR_NOMUS"          },
    { &s_STSTR_DQDON,          "STSTR_DQDON"          },
    { &s_STSTR_DQDOFF,         "STSTR_DQDOFF"         },
    { &s_STSTR_KFAADDED,       "STSTR_KFAADDED"       },
    { &s_STSTR_FAADDED,        "STSTR_FAADDED"        },
    { &s_STSTR_NCON,           "STSTR_NCON"           },
    { &s_STSTR_NCOFF,          "STSTR_NCOFF"          },
    { &s_STSTR_BEHOLD,         "STSTR_BEHOLD"         },
    { &s_STSTR_BEHOLDX,        "STSTR_BEHOLDX"        },
    { &s_STSTR_BEHOLDON,       "STSTR_BEHOLDON"       },
    { &s_STSTR_BEHOLDOFF,      "STSTR_BEHOLDOFF"      },
    { &s_STSTR_BUDDHA,         "STSTR_BUDDHA"         },
    { &s_STSTR_CHOPPERS,       "STSTR_CHOPPERS"       },
    { &s_STSTR_CLEV,           "STSTR_CLEV"           },
    { &s_STSTR_CLEVSAME,       "STSTR_CLEVSAME"       },
    { &s_STSTR_MYPOS,          "STSTR_MYPOS"          },
    { &s_STSTR_NTON,           "STSTR_NTON"           },
    { &s_STSTR_NTOFF,          "STSTR_NTOFF"          },
    { &s_STSTR_GODON,          "STSTR_GODON"          },
    { &s_STSTR_GODOFF,         "STSTR_GODOFF"         },
    { &s_STSTR_NMON,           "STSTR_NMON"           },
    { &s_STSTR_NMOFF,          "STSTR_NMOFF"          },
    { &s_STSTR_PSON,           "STSTR_PSON"           },
    { &s_STSTR_PSOFF,          "STSTR_PSOFF"          },
    { &s_STSTR_FMON,           "STSTR_FMON"           },
    { &s_STSTR_FMOFF,          "STSTR_FMOFF"          },
    { &s_STSTR_RION,           "STSTR_RION"           },
    { &s_STSTR_RIOFF,          "STSTR_RIOFF"          },
    { &s_STSTR_RMON,           "STSTR_RMON"           },
    { &s_STSTR_RMOFF,          "STSTR_RMOFF"          },
    { &s_STSTR_FON,            "STSTR_FON"            },
    { &s_STSTR_FOFF,           "STSTR_FOFF"           },
    { &s_STSTR_RHON,           "STSTR_RHON"           },
    { &s_STSTR_RHOFF,          "STSTR_RHOFF"          },
    { &s_STSTR_VMON,           "STSTR_VMON"           },
    { &s_STSTR_VMOFF,          "STSTR_VMOFF"          },

    { &s_E1TEXT,               "E1TEXT"               },
    { &s_E2TEXT,               "E2TEXT"               },
    { &s_E3TEXT,               "E3TEXT"               },
    { &s_E4TEXT,               "E4TEXT"               },
    { &s_C1TEXT,               "C1TEXT"               },
    { &s_C2TEXT,               "C2TEXT"               },
    { &s_C3TEXT,               "C3TEXT"               },
    { &s_C4TEXT,               "C4TEXT"               },
    { &s_C5TEXT,               "C5TEXT"               },
    { &s_C6TEXT,               "C6TEXT"               },
    { &s_P1TEXT,               "P1TEXT"               },
    { &s_P2TEXT,               "P2TEXT"               },
    { &s_P3TEXT,               "P3TEXT"               },
    { &s_P4TEXT,               "P4TEXT"               },
    { &s_P5TEXT,               "P5TEXT"               },
    { &s_P6TEXT,               "P6TEXT"               },
    { &s_T1TEXT,               "T1TEXT"               },
    { &s_T2TEXT,               "T2TEXT"               },
    { &s_T3TEXT,               "T3TEXT"               },
    { &s_T4TEXT,               "T4TEXT"               },
    { &s_T5TEXT,               "T5TEXT"               },
    { &s_T6TEXT,               "T6TEXT"               },
    { &s_N1TEXT,               "N1TEXT"               },

    { &s_CC_ZOMBIE,            "CC_ZOMBIE"            },
    { &s_CC_SHOTGUN,           "CC_SHOTGUN"           },
    { &s_CC_HEAVY,             "CC_HEAVY"             },
    { &s_CC_IMP,               "CC_IMP"               },
    { &s_CC_DEMON,             "CC_DEMON"             },
    { &s_CC_SPECTRE,           "CC_SPECTRE"           },
    { &s_CC_LOST,              "CC_LOST"              },
    { &s_CC_CACO,              "CC_CACO"              },
    { &s_CC_HELL,              "CC_HELL"              },
    { &s_CC_BARON,             "CC_BARON"             },
    { &s_CC_ARACH,             "CC_ARACH"             },
    { &s_CC_PAIN,              "CC_PAIN"              },
    { &s_CC_REVEN,             "CC_REVEN"             },
    { &s_CC_MANCU,             "CC_MANCU"             },
    { &s_CC_ARCH,              "CC_ARCH"              },
    { &s_CC_SPIDER,            "CC_SPIDER"            },
    { &s_CC_CYBER,             "CC_CYBER"             },
    { &s_CC_HERO,              "CC_HERO"              },

    { &s_DOOM_ENDMSG1,         "DOOM_ENDMSG1"         },
    { &s_DOOM_ENDMSG2,         "DOOM_ENDMSG2"         },
    { &s_DOOM_ENDMSG3,         "DOOM_ENDMSG3"         },
    { &s_DOOM_ENDMSG4,         "DOOM_ENDMSG4"         },
    { &s_DOOM_ENDMSG5,         "DOOM_ENDMSG5"         },
    { &s_DOOM_ENDMSG6,         "DOOM_ENDMSG6"         },
    { &s_DOOM_ENDMSG7,         "DOOM_ENDMSG7"         },
    { &s_DOOM2_ENDMSG1,        "DOOM2_ENDMSG1"        },
    { &s_DOOM2_ENDMSG2,        "DOOM2_ENDMSG2"        },
    { &s_DOOM2_ENDMSG3,        "DOOM2_ENDMSG3"        },
    { &s_DOOM2_ENDMSG4,        "DOOM2_ENDMSG4"        },
    { &s_DOOM2_ENDMSG5,        "DOOM2_ENDMSG5"        },
    { &s_DOOM2_ENDMSG6,        "DOOM2_ENDMSG6"        },
    { &s_DOOM2_ENDMSG7,        "DOOM2_ENDMSG7"        },

    { &s_M_NEWGAME,            "M_NEWGAME"            },
    { &s_M_OPTIONS,            "M_OPTIONS"            },
    { &s_M_LOADGAME,           "M_LOADGAME"           },
    { &s_M_SAVEGAME,           "M_SAVEGAME"           },
    { &s_M_QUITGAME,           "M_QUITGAME"           },
    { &s_M_WHICHEPISODE,       "M_WHICHEPISODE"       },
    { &s_M_EPISODE1,           "M_EPISODE1"           },
    { &s_M_EPISODE2,           "M_EPISODE2"           },
    { &s_M_EPISODE3,           "M_EPISODE3"           },
    { &s_M_EPISODE4,           "M_EPISODE4"           },
    { &s_M_WHICHEXPANSION,     "M_WHICHEXPANSION"     },
    { &s_M_EXPANSION1,         "M_EXPANSION1"         },
    { &s_M_EXPANSION2,         "M_EXPANSION2"         },
    { &s_M_CHOOSESKILLLEVEL,   "M_CHOOSESKILLLEVEL"   },
    { &s_M_SKILLLEVEL1,        "M_SKILLLEVEL1"        },
    { &s_M_SKILLLEVEL2,        "M_SKILLLEVEL2"        },
    { &s_M_SKILLLEVEL3,        "M_SKILLLEVEL3"        },
    { &s_M_SKILLLEVEL4,        "M_SKILLLEVEL4"        },
    { &s_M_SKILLLEVEL5,        "M_SKILLLEVEL5"        },
    { &s_M_ENDGAME,            "M_ENDGAME"            },
    { &s_M_MESSAGES,           "M_MESSAGES"           },
    { &s_M_ON,                 "M_ON"                 },
    { &s_M_OFF,                "M_OFF"                },
    { &s_M_GRAPHICDETAIL,      "M_GRAPHICDETAIL"      },
    { &s_M_HIGH,               "M_HIGH"               },
    { &s_M_LOW,                "M_LOW"                },
    { &s_M_SCREENSIZE,         "M_SCREENSIZE"         },
    { &s_M_MOUSESENSITIVITY,   "M_MOUSESENSITIVITY"   },
    { &s_M_GAMEPADSENSITIVITY, "M_GAMEPADSENSITIVITY" },
    { &s_M_SOUNDVOLUME,        "M_SOUNDVOLUME"        },
    { &s_M_SFXVOLUME,          "M_SFXVOLUME"          },
    { &s_M_MUSICVOLUME,        "M_MUSICVOLUME"        },
    { &s_M_PAUSED,             "M_PAUSED"             },

    { &s_CAPTION_SHAREWARE,    "CAPTION_SHAREWARE"    },
    { &s_CAPTION_REGISTERED,   "CAPTION_REGISTERED"   },
    { &s_CAPTION_ULTIMATE,     "CAPTION_ULTIMATE"     },
    { &s_CAPTION_DOOM2,        "CAPTION_DOOM2"        },
    { &s_CAPTION_HELLONEARTH,  "CAPTION_HELLONEARTH"  },
    { &s_CAPTION_NERVE,        "CAPTION_NERVE"        },
    { &s_CAPTION_BFGEDITION,   "CAPTION_BFGEDITION"   },
    { &s_CAPTION_PLUTONIA,     "CAPTION_PLUTONIA"     },
    { &s_CAPTION_TNT,          "CAPTION_TNT"          },
    { &s_CAPTION_CHEX,         "CAPTION_CHEX"         },
    { &s_CAPTION_CHEX2,        "CAPTION_CHEX2"        },
    { &s_CAPTION_HACX,         "CAPTION_HACX"         },
    { &s_CAPTION_FREEDOOM1,    "CAPTION_FREEDOOM1"    },
    { &s_CAPTION_FREEDOOM2,    "CAPTION_FREEDOOM2"    },
    { &s_CAPTION_FREEDM,       "CAPTION_FREEDM"       },
    { &s_CAPTION_BTSXE1,       "CAPTION_BTSXE1"       },
    { &s_CAPTION_BTSXE2,       "CAPTION_BTSXE2"       },
    { &s_CAPTION_BTSXE3,       "CAPTION_BTSXE3"       },
    { &s_CAPTION_E1M4B,        "CAPTION_E1M4B"        },
    { &s_CAPTION_E1M8B,        "CAPTION_E1M8B"        },
    { &s_AUTHOR_BESTOR,        "AUTHOR_BESTOR"        },
    { &s_AUTHOR_ROMERO,        "AUTHOR_ROMERO"        },

    { &bgflatE1,               "BGFLATE1"             },
    { &bgflatE2,               "BGFLATE2"             },
    { &bgflatE3,               "BGFLATE3"             },
    { &bgflatE4,               "BGFLATE4"             },
    { &bgflat06,               "BGFLAT06"             },
    { &bgflat11,               "BGFLAT11"             },
    { &bgflat15,               "BGFLAT15"             },
    { &bgflat20,               "BGFLAT20"             },
    { &bgflat30,               "BGFLAT30"             },
    { &bgflat31,               "BGFLAT31"             },
    { &bgcastcall,             "BGCASTCALL"           },

    // Ty 04/08/98 - added 5 general purpose startup announcement
    // strings for hacker use. See m_menu.c
    { &startup1,               "STARTUP1"             },
    { &startup2,               "STARTUP2"             },
    { &startup3,               "STARTUP3"             },
    { &startup4,               "STARTUP4"             },
    { &startup5,               "STARTUP5"             },

    { &savegamename,           "SAVEGAMENAME"         },

    { &s_BANNER1,              "BANNER1"              },
    { &s_BANNER2,              "BANNER2"              },
    { &s_BANNER3,              "BANNER3"              },
    { &s_BANNER4,              "BANNER4"              },
    { &s_BANNER5,              "BANNER5"              },
    { &s_BANNER6,              "BANNER6"              },
    { &s_BANNER7,              "BANNER7"              },

    { &s_COPYRIGHT1,           "COPYRIGHT1"           },
    { &s_COPYRIGHT2,           "COPYRIGHT2"           },
    { &s_COPYRIGHT3,           "COPYRIGHT3"           },

    { &s_OB_SUICIDE,           "OB_SUICIDE"           },
    { &s_OB_FALLING,           "OB_FALLING"           },
    { &s_OB_CRUSH,             "OB_CRUSH"             },
    { &s_OB_EXIT,              "OB_EXIT"              },
    { &s_OB_WATER,             "OB_WATER"             },
    { &s_OB_SLIME,             "OB_SLIME"             },
    { &s_OB_LAVA,              "OB_LAVA"              },
    { &s_OB_BARREL,            "OB_BARREL"            },
    { &s_OB_SPLASH,            "OB_SPLASH"            },
    { &s_OB_R_SPLASH,          "OB_R_SPLASH"          },
    { &s_OB_ROCKET,            "OB_ROCKET"            },
    { &s_OB_KILLEDSELF,        "OB_KILLEDSELF"        },
    { &s_OB_STEALTHBABY,       "OB_STEALTHBABY"       },
    { &s_OB_STEALTHVILE,       "OB_STEALTHVILE"       },
    { &s_OB_STEALTHBARON,      "OB_STEALTHBARON"      },
    { &s_OB_STEALTHCACO,       "OB_STEALTHCACO"       },
    { &s_OB_STEALTHCHAINGUY,   "OB_STEALTHCHAINGUY"   },
    { &s_OB_STEALTHDEMON,      "OB_STEALTHDEMON"      },
    { &s_OB_STEALTHKNIGHT,     "OB_STEALTHKNIGHT"     },
    { &s_OB_STEALTHIMP,        "OB_STEALTHIMP"        },
    { &s_OB_STEALTHFATSO,      "OB_STEALTHFATSO"      },
    { &s_OB_STEALTHUNDEAD,     "OB_STEALTHUNDEAD"     },
    { &s_OB_STEALTHSHOTGUY,    "OB_STEALTHSHOTGUY"    },
    { &s_OB_STEALTHZOMBIE,     "OB_STEALTHZOMBIE"     },
    { &s_OB_UNDEADHIT,         "OB_UNDEADHIT"         },
    { &s_OB_IMPHIT,            "OB_IMPHIT"            },
    { &s_OB_CACOHIT,           "OB_CACOHIT"           },
    { &s_OB_DEMONHIT,          "OB_DEMONHIT"          },
    { &s_OB_SPECTREHIT,        "OB_SPECTREHIT"        },
    { &s_OB_BARONHIT,          "OB_BARONHIT"          },
    { &s_OB_KNIGHTHIT,         "OB_KNIGHTHIT"         },
    { &s_OB_ZOMBIE,            "OB_ZOMBIE"            },
    { &s_OB_SHOTGUY,           "OB_SHOTGUY"           },
    { &s_OB_VILE,              "OB_VILE"              },
    { &s_OB_UNDEAD,            "OB_UNDEAD"            },
    { &s_OB_FATSO,             "OB_FATSO"             },
    { &s_OB_CHAINGUY,          "OB_CHAINGUY"          },
    { &s_OB_SKULL,             "OB_SKULL"             },
    { &s_OB_IMP,               "OB_IMP"               },
    { &s_OB_CACO,              "OB_CACO"              },
    { &s_OB_BARON,             "OB_BARON"             },
    { &s_OB_KNIGHT,            "OB_KNIGHT"            },
    { &s_OB_SPIDER,            "OB_SPIDER"            },
    { &s_OB_BABY,              "OB_BABY"              },
    { &s_OB_CYBORG,            "OB_CYBORG"            },
    { &s_OB_WOLFSS,            "OB_WOLFSS"            }
};

static const int deh_numstrlookup = sizeof(deh_strlookup) / sizeof(deh_strlookup[0]);

char *deh_newlevel = "NEWLEVEL";

char **mapnames[] =     // DOOM shareware/registered/retail (Ultimate) names.
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
    &deh_newlevel,      // spares? Unused.
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel
};

char **mapnames2[] =    // DOOM 2 map names.
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

char **mapnames2_bfg[] =    // DOOM 2 map names.
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
    &s_HUSTR_31_BFG,
    &s_HUSTR_32_BFG,
    &s_HUSTR_33
};

char **mapnamesp[] =    // Plutonia WAD map names.
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

char **mapnamest[] =    // TNT WAD map names.
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

char **mapnamesn[] =    // Nerve WAD map names.
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

// Function prototypes
static void lfstrip(char *s);       // strip the \r and/or \n off of a line
static void rstrip(char *s);        // strip trailing whitespace
static char *ptr_lstrip(char *p);   // point past leading whitespace
static int deh_GetData(char *s, char *k, long *l, char **strval);
static dboolean deh_procStringSub(char *key, char *lookfor, char *newstring);
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

// Structure deh_block is used to hold the block names that can
// be encountered, and the routines to use to decipher them
typedef struct
{
    char    *key;                                       // a mnemonic block code name
    void    (*const fptr)(DEHFILE *, char *);           // handler
} deh_block;

#define DEH_BUFFERMAX   1024    // input buffer area size, hardcoded for now
// killough 8/9/98: make DEH_BLOCKMAX self-adjusting
#define DEH_BLOCKMAX    arrlen(deh_blocks)              // size of array
#define DEH_MAXKEYLEN   32      // as much of any key as we'll look at
#define DEH_MOBJINFOMAX 31      // number of ints in the mobjinfo_t structure (!)

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

    //     begin BOOM Extensions (BEX)
    /* 10 */ { "[STRINGS]", deh_procStrings         },  // new string changes
    /* 11 */ { "[PARS]",    deh_procPars            },  // alternative block marker
    /* 12 */ { "[CODEPTR]", deh_procBexCodePointers },  // bex codepointers by mnemonic
    /* 13 */ { "",          deh_procError           }   // dummy to handle anything else
};

// flag to skip included deh-style text, used with INCLUDE NOTEXT directive
static dboolean includenotext;

// MOBJINFO - Dehacked block name = "Thing"
// Usage: Thing nn (name)
// These are for mobjinfo_t types. Each is an integer
// within the structure, so we can use index of the string in this
// array to offset by sizeof(int) into the mobjinfo_t array at [nn]
// * things are base zero but dehacked considers them to start at #1. ***
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
    "Speed",                    // .speed
    "Width",                    // .radius
    "Pickup width",             // .pickupradius
    "Height",                   // .height
    "Projectile pass height",   // .projectilepassheight
    "Mass",                     // .mass
    "Missile damage",           // .damage
    "Action sound",             // .activesound
    "Bits",                     // .flags
    "Retro Bits",               // .flags2
    "Respawn frame",            // .raisestate
    "Frames",                   // .frames
    "Fullbright",               // .fullbright
    "Blood",                    // .blood
    "Shadow offset"             // .shadowoffset
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

struct deh_mobjflags_s
{
    char    *name;
    long    value;
};

static const struct deh_mobjflags_s deh_mobjflags[] =
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
    { "FRIEND",       MF_FRIEND       },    // a friend of the player(s) (MBF)
    { "TRANSLUCENT",  MF_TRANSLUCENT  }     // apply translucency to sprite (BOOM)
};

static const struct deh_mobjflags_s deh_mobjflags2[] =
{
    { "TRANSLUCENT",               MF2_TRANSLUCENT               },
    { "TRANSLUCENT_REDONLY",       MF2_TRANSLUCENT_REDONLY       },
    { "TRANSLUCENT_GREENONLY",     MF2_TRANSLUCENT_GREENONLY     },
    { "TRANSLUCENT_BLUEONLY",      MF2_TRANSLUCENT_BLUEONLY      },
    { "TRANSLUCENT_33",            MF2_TRANSLUCENT_33            },
    { "TRANSLUCENT_50",            MF2_TRANSLUCENT_50            },
    { "TRANSLUCENT_REDWHITEONLY",  MF2_TRANSLUCENT_REDWHITEONLY  },
    { "TRANSLUCENT_REDTOGREEN_33", MF2_TRANSLUCENT_REDTOGREEN_33 },
    { "TRANSLUCENT_REDTOBLUE_33",  MF2_TRANSLUCENT_REDTOBLUE_33  },
    { "TRANSLUCENT_BLUE_25",       MF2_TRANSLUCENT_BLUE_25       },
    { "REDTOGREEN",                MF2_TRANSLUCENT               },
    { "GREENTORED",                MF2_GREENTORED                },
    { "REDTOBLUE",                 MF2_REDTOBLUE                 },
    { "FLOATBOB",                  MF2_FLOATBOB                  },
    { "MIRRORED",                  MF2_MIRRORED                  },
    { "FALLING",                   MF2_FALLING                   },
    { "ONMOBJ",                    MF2_ONMOBJ                    },
    { "PASSMOBJ",                  MF2_PASSMOBJ                  },
    { "RESURRECTING",              MF2_RESURRECTING              },
    { "NOFOOTCLIP",                MF2_NOFOOTCLIP                },
    { "NOLIQUIDBOB",               MF2_NOLIQUIDBOB               },
    { "FEETARECLIPPED",            MF2_FEETARECLIPPED            },
    { "CASTSHADOW",                MF2_CASTSHADOW                },
    { "BLOOD",                     MF2_BLOOD                     },
    { "DONTMAP",                   MF2_DONTMAP                   },
    { "SMOKETRAIL",                MF2_SMOKETRAIL                },
    { "CRUSHABLE",                 MF2_CRUSHABLE                 },
    { "MASSACRE",                  MF2_MASSACRE                  },
    { "DECORATION",                MF2_DECORATION                },
    { "DONTDRAW",                  MF2_DONTDRAW                  },
    { "MONSTERMISSILE",            MF2_MONSTERMISSILE            }
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
    "Sprite number",    // .sprite (spritenum_t) // an enum
    "Sprite subnumber", // .frame (long)
    "Duration",         // .tics (long)
    "Next frame",       // .nextstate (statenum_t)
    // This is set in a separate "Pointer" block from Dehacked
    "Codep Frame",      // pointer to first use of action (actionf_t)
    "Unknown 1",        // .misc1 (long)
    "Unknown 2"         // .misc2 (long)
};

// SFXINFO_STRUCT - Dehacked block name = "Sounds"
// Sound effects, typically not changed (redirected, and new sfx put
// into the pwad, but not changed here. Can you tell that Greg didn't
// know what they were for, mostly? Can you tell that I don't either?
// Mostly I just put these into the same slots as they are in the struct.
// This may not be supported in our -deh option if it doesn't make sense by then.

// * sounds are base zero but have a dummy #0
static const char *deh_sfxinfo[] =
{
    "Offset",           // pointer to a name string, changed in text
    "Zero/One",          // .singularity (int, one at a time flag)
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
    "Max ammo",         // maxammo[]
    "Per ammo"          // clipammo[]
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
    "Firing frame"      // .flashstate
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
    "Ammo & Keys",      // idkfa
    "Ammo",             // idfa
    "No Clipping 1",    // idspispopd
    "No Clipping 2",    // idclip
    "Invincibility",    // idbeholdv
    "Berserk",          // idbeholds
    "Invisibility",     // idbeholdi
    "Radiation Suit",   // idbeholdr
    "Auto-map",         // idbeholda
    "Lite-Amp Goggles", // idbeholdl
    "BEHOLD menu",      // idbehold
    "Level Warp",       // idclev
    "Player Position"   // idmypos
};

// MISC - Dehacked block name = "Misc"
// Usage: Misc 0
// Always uses a zero in the dehacked file, for consistency. No meaning.
static const char *deh_misc[] =
{
    "Initial Health",           // initial_health
    "Initial Bullets",          // initial_bullets
    "Max Health",               // maxhealth
    "Max Armor",                // max_armor
    "Green Armor Class",        // green_armor_class
    "Blue Armor Class",         // blue_armor_class
    "Max Soulsphere",           // max_soul
    "Soulsphere Health",        // soul_health
    "Megasphere Health",        // mega_health
    "God Mode Health",          // god_health
    "IDFA Armor",               // idfa_armor
    "IDFA Armor Class",         // idfa_armor_class
    "IDKFA Armor",              // idkfa_armor
    "IDKFA Armor Class",        // idkfa_armor_class
    "BFG Cells/Shot",           // BFGCELLS
    "Monsters Infight"          // species_infighting
};

// TEXT - Dehacked block name = "Text"
// Usage: Text fromlen tolen
// Dehacked allows a bit of adjustment to the length (why?)

// BEX extension [CODEPTR]
// Usage: Start block, then each line is:
// FRAME nnn = PointerMnemonic

// External references to action functions scattered about the code
void A_Light0(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Punch(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePistol(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireShotgun(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckReload(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_OpenShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_LoadShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CloseShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCGun(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GunFlash(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireMissile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Saw(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePlasma(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BFGsound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireBFG(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BFGSpray(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Explode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PlayerScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_XScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileChase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileStart(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_StartFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCrackle(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Tracer(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelWhoosh(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelFist(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelMissile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatRaise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack3(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BossDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CPosRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_TroopAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SargAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BruisAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Metal(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpidRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BabyMetal(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BspiAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Hoof(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CyberAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PainAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PainDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_KeenDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainAwake(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainSpit(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnFly(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainExplode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Detonate(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Mushroom(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullPop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Die(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Spawn(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Turn(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Face(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scratch(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PlaySound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RandomJump(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_LineEffect(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireOldBFG(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BetaSkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Stop(mobj_t *actor, player_t *player, pspdef_t *psp);

typedef struct
{
    actionf_t   cptr;           // actual pointer to the subroutine
    const char  *lookup;        // mnemonic lookup string to be specified in BEX
} deh_bexptr;

static const deh_bexptr deh_bexptrs[] =
{
    { A_Light0,          "A_Light0"          },
    { A_WeaponReady,     "A_WeaponReady"     },
    { A_Lower,           "A_Lower"           },
    { A_Raise,           "A_Raise"           },
    { A_Punch,           "A_Punch"           },
    { A_ReFire,          "A_ReFire"          },
    { A_FirePistol,      "A_FirePistol"      },
    { A_Light1,          "A_Light1"          },
    { A_FireShotgun,     "A_FireShotgun"     },
    { A_Light2,          "A_Light2"          },
    { A_FireShotgun2,    "A_FireShotgun2"    },
    { A_CheckReload,     "A_CheckReload"     },
    { A_OpenShotgun2,    "A_OpenShotgun2"    },
    { A_LoadShotgun2,    "A_LoadShotgun2"    },
    { A_CloseShotgun2,   "A_CloseShotgun2"   },
    { A_FireCGun,        "A_FireCGun"        },
    { A_GunFlash,        "A_GunFlash"        },
    { A_FireMissile,     "A_FireMissile"     },
    { A_Saw,             "A_Saw"             },
    { A_FirePlasma,      "A_FirePlasma"      },
    { A_BFGsound,        "A_BFGsound"        },
    { A_FireBFG,         "A_FireBFG"         },
    { A_BFGSpray,        "A_BFGSpray"        },
    { A_Explode,         "A_Explode"         },
    { A_Pain,            "A_Pain"            },
    { A_PlayerScream,    "A_PlayerScream"    },
    { A_Fall,            "A_Fall"            },
    { A_XScream,         "A_XScream"         },
    { A_Look,            "A_Look"            },
    { A_Chase,           "A_Chase"           },
    { A_FaceTarget,      "A_FaceTarget"      },
    { A_PosAttack,       "A_PosAttack"       },
    { A_Scream,          "A_Scream"          },
    { A_SPosAttack,      "A_SPosAttack"      },
    { A_VileChase,       "A_VileChase"       },
    { A_VileStart,       "A_VileStart"       },
    { A_VileTarget,      "A_VileTarget"      },
    { A_VileAttack,      "A_VileAttack"      },
    { A_StartFire,       "A_StartFire"       },
    { A_Fire,            "A_Fire"            },
    { A_FireCrackle,     "A_FireCrackle"     },
    { A_Tracer,          "A_Tracer"          },
    { A_SkelWhoosh,      "A_SkelWhoosh"      },
    { A_SkelFist,        "A_SkelFist"        },
    { A_SkelMissile,     "A_SkelMissile"     },
    { A_FatRaise,        "A_FatRaise"        },
    { A_FatAttack1,      "A_FatAttack1"      },
    { A_FatAttack2,      "A_FatAttack2"      },
    { A_FatAttack3,      "A_FatAttack3"      },
    { A_BossDeath,       "A_BossDeath"       },
    { A_CPosAttack,      "A_CPosAttack"      },
    { A_CPosRefire,      "A_CPosRefire"      },
    { A_TroopAttack,     "A_TroopAttack"     },
    { A_SargAttack,      "A_SargAttack"      },
    { A_HeadAttack,      "A_HeadAttack"      },
    { A_BruisAttack,     "A_BruisAttack"     },
    { A_SkullAttack,     "A_SkullAttack"     },
    { A_Metal,           "A_Metal"           },
    { A_SpidRefire,      "A_SpidRefire"      },
    { A_BabyMetal,       "A_BabyMetal"       },
    { A_BspiAttack,      "A_BspiAttack"      },
    { A_Hoof,            "A_Hoof"            },
    { A_CyberAttack,     "A_CyberAttack"     },
    { A_PainAttack,      "A_PainAttack"      },
    { A_PainDie,         "A_PainDie"         },
    { A_KeenDie,         "A_KeenDie"         },
    { A_BrainPain,       "A_BrainPain"       },
    { A_BrainScream,     "A_BrainScream"     },
    { A_BrainDie,        "A_BrainDie"        },
    { A_BrainAwake,      "A_BrainAwake"      },
    { A_BrainSpit,       "A_BrainSpit"       },
    { A_SpawnSound,      "A_SpawnSound"      },
    { A_SpawnFly,        "A_SpawnFly"        },
    { A_BrainExplode,    "A_BrainExplode"    },
    { A_Detonate,        "A_Detonate"        },   // killough 8/9/98
    { A_Mushroom,        "A_Mushroom"        },   // killough 10/98
    { A_SkullPop,        "A_SkullPop"        },
    { A_Die,             "A_Die"             },   // killough 11/98
    { A_Spawn,           "A_Spawn"           },   // killough 11/98
    { A_Turn,            "A_Turn"            },   // killough 11/98
    { A_Face,            "A_Face"            },   // killough 11/98
    { A_Scratch,         "A_Scratch"         },   // killough 11/98
    { A_PlaySound,       "A_PlaySound"       },   // killough 11/98
    { A_RandomJump,      "A_RandomJump"      },   // killough 11/98
    { A_LineEffect,      "A_LineEffect"      },   // killough 11/98

    { A_FireOldBFG,      "A_FireOldBFG"      },   // killough 7/19/98: classic BFG firing function
    { A_BetaSkullAttack, "A_BetaSkullAttack" },   // killough 10/98: beta lost souls attacked different
    { A_Stop,            "A_Stop"            },

    // This NULL entry must be the last in the list
    { NULL,              "A_NULL"            }    // Ty 05/16/98
};

// to hold startup code pointers from INFO.C
static actionf_t deh_codeptr[NUMSTATES];

dboolean CheckPackageWADVersion(void)
{
    DEHFILE infile;
    DEHFILE *filein = &infile;
    char    inbuffer[32];

    for (int i = 0; i < numlumps; i++)
        if (!strncasecmp(lumpinfo[i]->name, "VERSION", 7))
        {
            infile.size = W_LumpLength(i);
            infile.inp = infile.lump = W_CacheLumpNum(i);

            while (dehfgets(inbuffer, sizeof(inbuffer), filein))
            {
                lfstrip(inbuffer);

                if (!*inbuffer || *inbuffer == '#' || *inbuffer == ' ')
                    continue;   // Blank line or comment line

                if (M_StringCompare(inbuffer, PACKAGE_NAMEANDVERSIONSTRING))
                {
                    Z_ChangeTag(infile.lump, PU_CACHE);
                    return true;
                }
            }

            Z_ChangeTag(infile.lump, PU_CACHE);
        }

    return false;
}

// ====================================================================
// ProcessDehFile
// Purpose: Read and process a DEH or BEX file
// Args:    filename    -- name of the DEH/BEX file
// Returns: void
//
// killough 10/98:
// substantially modified to allow input from wad lumps instead of .deh files.
void ProcessDehFile(char *filename, int lumpnum)
{
    DEHFILE infile;
    DEHFILE *filein = &infile;              // killough 10/98
    char    inbuffer[DEH_BUFFERMAX];        // Place to put the primary infostring

    linecount = 0;
    addtocount = false;

    // killough 10/98: allow DEH files to come from wad lumps
    if (filename)
    {
        if (!(infile.f = fopen(filename, "rt")))
            return;             // should be checked up front anyway

        infile.lump = NULL;
    }
    else                        // DEH file comes from lump indicated by second argument
    {
        if (!(infile.size = W_LumpLength(lumpnum)))
            return;

        infile.inp = infile.lump = W_CacheLumpNum(lumpnum);
        filename = lumpinfo[lumpnum]->wadfile->path;
    }

    {
        static int  i;          // killough 10/98: only run once, by keeping index static

        // remember what they start as for deh xref
        for (; i < EXTRASTATES; i++)
            deh_codeptr[i] = states[i].action;

        // [BH] Initialize extra DeHackEd states 1089 to 3999
        for (; i < NUMSTATES; i++)
        {
            states[i].sprite = SPR_TNT1;
            states[i].frame = 0;
            states[i].tics = -1;
            states[i].action = NULL;
            states[i].nextstate = i;
            states[i].misc1 = 0;
            states[i].misc2 = 0;
            states[i].dehacked = false;
            deh_codeptr[i] = states[i].action;
        }
    }

    // loop until end of file
    while (dehfgets(inbuffer, sizeof(inbuffer), filein))
    {
        dboolean        match;
        unsigned int    i;
        unsigned int    last_i = DEH_BLOCKMAX - 1;
        long            filepos = 0;

        lfstrip(inbuffer);

        if (devparm)
            C_Output("Line = \"%s\"", inbuffer);

        if (!*inbuffer || *inbuffer == '#' || *inbuffer == ' ')
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
            dboolean    oldnotext = includenotext;      // killough 10/98

            // killough 10/98: exclude if inside wads (only to discourage
            // the practice, since the code could otherwise handle it)
            if (infile.lump)
            {
                C_Warning("No files may be included from wads: \"%s\".", inbuffer);
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
                C_Output("Branching to include file <b>%s</b>...", nextfile);

            ProcessDehFile(nextfile, 0); // do the included file

            includenotext = oldnotext;

            if (devparm)
                C_Output("...continuing with <b>%s</b>", filename);

            continue;
        }

        for (match = false, i = 0; i < DEH_BLOCKMAX; i++)
            if (!strncasecmp(inbuffer, deh_blocks[i].key, strlen(deh_blocks[i].key)))
            {
                if (i < DEH_BLOCKMAX - 1)
                    match = true;

                break;          // we got one, that's enough for this block
            }

        if (match)              // inbuffer matches a valid block code name
            last_i = i;
        else if (last_i >= 10 && last_i < DEH_BLOCKMAX - 1)     // restrict to BEX style lumps
        {
            // process that same line again with the last valid block code handler
            i = last_i;

            if (!filein->lump)
                fseek(filein->f, filepos, SEEK_SET);
        }

        if (devparm)
            C_Output("Processing function [%i] for %s", i, deh_blocks[i].key);

        deh_blocks[i].fptr(filein, inbuffer);           // call function

        if (!filein->lump)                              // back up line start
            filepos = ftell(filein->f);
    }

    if (infile.lump)
        Z_ChangeTag(infile.lump, PU_CACHE);     // Mark purgeable
    else
        fclose(infile.f);                       // Close real file

    if (addtocount)
        dehcount++;

    if (infile.lump)
        C_Output("Parsed %s line%s in the <b>DEHACKED</b> lump in %s <b>%s</b>.", commify(linecount),
            (linecount > 1 ? "s" : ""), (W_WadType(filename) == IWAD ? "IWAD" : "PWAD"), filename);
    else
        C_Output("Parsed %s line%s in the <i><b>DeHackEd</b></i>%s file <b>%s</b>.", commify(linecount),
            (linecount > 1 ? "s" : ""), (M_StringEndsWith(uppercase(filename), "BEX") ?
            " with <i><b>BOOM</b></i> extensions" : ""), GetCorrectCase(filename));
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
    char    mnemonic[DEH_MAXKEYLEN] = "";   // to hold the codepointer mnemonic

    // Ty 05/16/98 - initialize it to something, dummy!
    strncpy(inbuffer, line, DEH_BUFFERMAX);

    // for this one, we just read 'em until we hit a blank line
    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        int         i = 0;                  // looper
        dboolean    found = false;          // know if we found this one during lookup or not

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98: really exit on blank line

        // killough 8/98: allow hex numbers in input:
        if ((sscanf(inbuffer, "%31s %10i = %31s", key, &indexnum, mnemonic) != 3)
            || !M_StringCompare(key, "FRAME"))        // NOTE: different format from normal
        {
            C_Warning("Invalid BEX codepointer line - must start with \"FRAME\": \"%s\".", inbuffer);
            return;     // early return
        }

        if (devparm)
            C_Output("Processing pointer at index %i: %s", indexnum, mnemonic);

        if (indexnum < 0 || indexnum >= NUMSTATES)
        {
            C_Warning("Bad pointer number %i of %i.", indexnum, NUMSTATES);
            return;     // killough 10/98: fix SegViol
        }

        strcpy(key, "A_");      // reusing the key area to prefix the mnemonic
        strcat(key, ptr_lstrip(mnemonic));

        while (!found && deh_bexptrs[i].lookup)
        {
            if (M_StringCompare(key, deh_bexptrs[i].lookup))
            {   // Ty 06/01/98  - add to states[].action for new djgcc version
                states[indexnum].action = deh_bexptrs[i].cptr;  // assign

                if (devparm)
                    C_Output(" - applied %s from codeptr[%i] to states[%i]", deh_bexptrs[i].lookup, i,
                        indexnum);

                found = true;
            }

            i++;
        }

        if (!found)
            C_Warning("Invalid frame pointer mnemonic \"%s\" at %i.", mnemonic, indexnum);
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
    long    value;          // All deh values are ints or longs
    int     indexnum;
    int     ix;
    int     *pix;           // Ptr to int, since all Thing structure entries are ints
    char    *strval;

    strncpy(inbuffer, line, DEH_BUFFERMAX);

    if (devparm)
        C_Output("Thing line: \"%s\"", inbuffer);

    // killough 8/98: allow hex numbers in input:
    ix = sscanf(inbuffer, "%31s %10i", key, &indexnum);

    if (devparm)
        C_Output("count = %i, Thing %i", ix, indexnum);

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
        int         bGetData;
        dboolean    gibhealth = false;

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);      // toss the end of line

        // killough 11/98: really bail out on blank lines (break != continue)
        if (!*inbuffer)
            break;              // bail out with blank line between sections

        // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
        bGetData = deh_GetData(inbuffer, key, &value, &strval);

        if (!bGetData)
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        for (ix = 0; ix < DEH_MOBJINFOMAX; ix++)
        {
            if (!M_StringCompare(key, deh_mobjinfo[ix]))
                continue;

            if (M_StringCompare(key, "Bits"))
            {
                // bit set
                // e6y: Correction of wrong processing of Bits parameter if its value is equal to
                // zero
                if (bGetData == 1)
                    mobjinfo[indexnum].flags = value;
                else
                {
                    // figure out what the bits are
                    value = 0;

                    // killough 10/98: replace '+' kludge with strtok() loop
                    // Fix error-handling case ('found' var wasn't being reset)
                    //
                    // Use OR logic instead of addition, to allow repetition
                    for (; (strval = strtok(strval, ",+| \t\f\r")); strval = NULL)
                    {
                        int iy;

                        for (iy = 0; iy < DEH_MOBJFLAGMAX; iy++)
                        {
                            if (!M_StringCompare(strval, deh_mobjflags[iy].name))
                                continue;

                            if (devparm)
                                C_Output("ORed value 0x%08lX %s.", deh_mobjflags[iy].value, strval);

                            value |= deh_mobjflags[iy].value;

                            // [BH] hack to use vanilla height of dehacked hanging decorations
                            if (deh_mobjflags[iy].value == MF_SPAWNCEILING
                                && (mobjinfo[indexnum].flags2 & MF2_DECORATION))
                                mobjinfo[indexnum].height = 16 * FRACUNIT;

                            // [BH] no blood splats if thing is dehacked...
                            mobjinfo[indexnum].blood = 0;
                            break;
                        }

                        if (iy >= DEH_MOBJFLAGMAX)
                            C_Warning("Could not find bit mnemonic \"%s\".", strval);
                    }

                    // Don't worry about conversion -- simply print values
                    if (devparm)
                        C_Output("Bits = 0x%08lX = %ld.", value, value);

                    mobjinfo[indexnum].flags = value; // e6y

                    // [BH] ...but add blood splats if thing is still shootable
                    if (value & MF_SHOOTABLE)
                        mobjinfo[indexnum].blood = MT_BLOOD;
                }
            }
            else if (M_StringCompare(key, "Retro Bits"))
            {
                // bit set
                if (bGetData == 1)
                    mobjinfo[indexnum].flags2 = value;
                else
                {
                    // figure out what the bits are
                    value = 0;

                    for (; (strval = strtok(strval, ",+| \t\f\r")); strval = NULL)
                    {
                        int iy;

                        for (iy = 0; iy < DEH_MOBJFLAG2MAX; iy++)
                        {
                            if (!M_StringCompare(strval, deh_mobjflags2[iy].name))
                                continue;

                            if (devparm)
                                C_Output("ORed value 0x%08lX %s.", deh_mobjflags2[iy].value, strval);

                            value |= deh_mobjflags[iy].value;
                            break;
                        }

                        if (iy >= DEH_MOBJFLAG2MAX)
                            C_Warning("Could not find bit mnemonic \"%s\".", strval);
                    }

                    // Don't worry about conversion -- simply print values
                    if (devparm)
                        C_Output("Bits = 0x%08lX = %ld.", value, value);

                    mobjinfo[indexnum].flags2 = value;
                }
            }
            else
            {
                pix = (int *)&mobjinfo[indexnum];
                pix[ix] = (int)value;

                if (M_StringCompare(key, "Height"))
                    mobjinfo[indexnum].projectilepassheight = 0;
                else if (M_StringCompare(key, "Width"))
                    mobjinfo[indexnum].pickupradius = (int)value;
                else if (M_StringCompare(key, "Gib health"))
                    gibhealth = true;
            }

            if (devparm)
                C_Output("Assigned %i to %s (%i) at index %i.", (int)value, key, indexnum, ix);
        }

        if (!gibhealth && mobjinfo[indexnum].spawnhealth && !mobjinfo[indexnum].gibhealth)
            mobjinfo[indexnum].gibhealth = -mobjinfo[indexnum].spawnhealth;
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
    long    value;  // All deh values are ints or longs
    int     indexnum;

    strncpy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);

    if (devparm)
        C_Output("Processing Frame at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMSTATES)
        C_Warning("Bad frame number %i of %i.", indexnum, NUMSTATES);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                                              // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))          // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (M_StringCompare(key, deh_state[0]))                 // Sprite number
        {
            if (devparm)
                C_Output(" - sprite = %ld", value);

            states[indexnum].sprite = (spritenum_t)value;
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[1]))            // Sprite subnumber
        {
            if (devparm)
                C_Output(" - frame = %ld", value);

            states[indexnum].frame = value;                     // long
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[2]))            // Duration
        {
            if (devparm)
                C_Output(" - tics = %ld", value);

            states[indexnum].tics = value;                      // long
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[3]))            // Next frame
        {
            if (devparm)
                C_Output(" - nextstate = %ld", value);

            states[indexnum].nextstate = value;
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[4]))    // Codep frame (not set in Frame deh block)
            C_Warning("Codep frame should not be set in Frame section.");
        else if (M_StringCompare(key, deh_state[5]))            // Unknown 1
        {
            if (devparm)
                C_Output(" - misc1 = %ld", value);

            states[indexnum].misc1 = value;                     // long
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, deh_state[6]))            // Unknown 2
        {
            if (devparm)
                C_Output(" - misc2 = %ld", value);

            states[indexnum].misc2 = value;                     // long
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else if (M_StringCompare(key, "translucent"))           // Translucent
        {
            if (devparm)
                C_Output(" - translucent = %ld", value);

            states[indexnum].translucent = !!value;             // bool
            states[indexnum].dehacked = dehacked = !BTSX;
        }
        else
            C_Warning("Invalid frame string index for \"%s\".", key);
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
    long    value;  // All deh values are ints or longs
    int     indexnum;
    int     i;      // looper

    strncpy(inbuffer, line, DEH_BUFFERMAX);
    // NOTE: different format from normal

    // killough 8/98: allow hex numbers in input, fix error case:
    if (sscanf(inbuffer, "%*s %*i (%31s %10i)", key, &indexnum) != 2)
    {
        C_Warning("Bad data pair in \"%s\".", inbuffer);
        return;
    }

    if (devparm)
        C_Output("Processing Pointer at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMSTATES)
    {
        C_Warning("Bad pointer number %i of %i.", indexnum, NUMSTATES);
        return;
    }

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))   // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (value < 0 || value >= NUMSTATES)
        {
            C_Warning("Bad pointer number %ld of %i.", value, NUMSTATES);
            return;
        }

        if (M_StringCompare(key, deh_state[4]))     // Codep frame (not set in Frame deh block)
        {
            states[indexnum].action = deh_codeptr[value];

            if (devparm)
                C_Output(" - applied %p from codeptr[%ld] to states[%i]", (void *)deh_codeptr[value], value,
                    indexnum);

            // Write BEX-oriented line to match:
            for (i = 0; i < arrlen(deh_bexptrs); i++)
                if (!memcmp(&deh_bexptrs[i].cptr, &deh_codeptr[value], sizeof(actionf_t)))
                {
                    if (devparm)
                        C_Output("BEX [CODEPTR] -> FRAME %i = %s", indexnum, &deh_bexptrs[i].lookup[2]);

                    break;
                }
        }
        else
            C_Warning("Invalid frame pointer index for \"%s\" at %ld, xref %p.", key, value,
                (void *)deh_codeptr[value]);
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
    long    value;  // All deh values are ints or longs
    int     indexnum;

    strncpy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);

    if (devparm)
        C_Output("Processing Sounds at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMSFX)
        C_Warning("Bad sound number %i of %i.", indexnum, NUMSFX);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;      // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))   // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\"\n", inbuffer);
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
            S_sfx[indexnum].volume = value;
        else if (M_StringCompare(key, deh_sfxinfo[6]))      // Zero 4
            /* nop */;
        else  if (M_StringCompare(key, deh_sfxinfo[7]))     // Neg. One 1
            /* nop */;
        else if (M_StringCompare(key, deh_sfxinfo[8]))      // Neg. One 2
             S_sfx[indexnum].lumpnum = value;
        else if (devparm)
            C_Warning("Invalid sound string index for \"%s\"", key);
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
    long    value;  // All deh values are ints or longs
    int     indexnum;

    strncpy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);

    if (devparm)
        C_Output("Processing Ammo at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMAMMO)
        C_Warning("Bad ammo number %i of %i.", indexnum, NUMAMMO);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                                          // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))      // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (M_StringCompare(key, deh_ammo[0]))              // Max ammo
            maxammo[indexnum] = value;
        else if (M_StringCompare(key, deh_ammo[1]))         // Per ammo
            clipammo[indexnum] = value;
        else
            C_Warning("Invalid ammo string index for \"%s\".", key);
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
    long    value;      // All deh values are ints or longs
    int     indexnum;

    strncpy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);

    if (devparm)
        C_Output("Processing Weapon at index %i: %s", indexnum, key);

    if (indexnum < 0 || indexnum >= NUMWEAPONS)
        C_Warning("Bad weapon number %i of %i.", indexnum, NUMAMMO);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                                              // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))          // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (M_StringCompare(key, deh_weapon[0]))                    // Ammo type
            weaponinfo[indexnum].ammotype = value;
        else if (M_StringCompare(key, deh_weapon[1]))               // Deselect frame
            weaponinfo[indexnum].upstate = value;
        else if (M_StringCompare(key, deh_weapon[2]))               // Select frame
            weaponinfo[indexnum].downstate = value;
        else if (M_StringCompare(key, deh_weapon[3]))               // Bobbing frame
            weaponinfo[indexnum].readystate = value;
        else if (M_StringCompare(key, deh_weapon[4]))               // Shooting frame
            weaponinfo[indexnum].atkstate = value;
        else if (M_StringCompare(key, deh_weapon[5]))               // Firing frame
            weaponinfo[indexnum].flashstate = value;
        else
            C_Warning("Invalid weapon string index for \"%s\".", key);
    }
}

// ====================================================================
// deh_procSprite
// Purpose: Dummy - we do not support the DEH Sprite block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procSprite(DEHFILE *fpin, char *line) // Not supported
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     indexnum;

    // Too little is known about what this is supposed to do, and
    // there are better ways of handling sprite renaming. Not supported.
    strncpy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);
    C_Warning("Ignoring sprite offset change at index %i: \"%s\".", indexnum, key);

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

extern int pars[5][10];
extern int cpars[33];

// ====================================================================
// deh_procPars
// Purpose: Handle BEX extension for PAR times
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procPars(DEHFILE *fpin, char *line) // extension
{
    char    key[DEH_MAXKEYLEN];
    char    inbuffer[DEH_BUFFERMAX];
    int     indexnum;
    int     episode;
    int     level;
    int     partime;
    int     oldpar;

    // new item, par times
    // usage: After [PARS] Par 0 section identifier, use one or more of these
    // lines:
    //  par 3 5 120
    //  par 14 230
    // The first would make the par for E3M5 be 120 seconds, and the
    // second one makes the par for MAP14 be 230 seconds. The number
    // of parameters on the line determines which group of par values
    // is being changed. Error checking is done based on current fixed
    // array sizes of[4][10] and [32]
    strncpy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);

    if (devparm)
        C_Output("Processing Par value at index %i: %s", indexnum, key);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        if (*inbuffer == '#')
            continue;                           // skip comment lines

        lfstrip(lowercase(inbuffer));           // lowercase it

        if (!*inbuffer)
            break;                              // killough 11/98

        if (sscanf(inbuffer, "par %10i %10i %10i", &episode, &level, &partime) != 3)
        {
            if (sscanf(inbuffer, "par %10i %10i", &level, &partime) != 2)
                C_Warning("Invalid par time setting string \"%s\".", inbuffer);
            else
            {
                // Ty 07/11/98 - wrong range check, not zero-based
                if (level < 1 || level > 32)    // base 0 array (but 1-based parm)
                    C_Warning("Invalid MAPxy value MAP%i.", level);
                else
                {
                    oldpar = cpars[level - 1];

                    if (devparm)
                        C_Output("Changed par time for MAP%02d from %i to %i seconds", level, oldpar,
                            partime);

                    cpars[level - 1] = partime;
                }
            }
        }
        else
        {
            // note that though it's a [4][10] array, the "left" and "top" aren't used,
            // effectively making it a base 1 array.
            // Ty 07/11/98 - level was being checked against max 3 - dumb error
            // Note that episode 4 does not have par times per original design
            // in Ultimate DOOM so that is not supported here.
            if (episode < 1 || episode > 3 || level < 1 || level > 9)
                C_Warning("Invalid ExMy values E%iM%i.", episode, level);
            else
            {
                oldpar = pars[episode][level];
                pars[episode][level] = partime;

                if (devparm)
                    C_Output("Changed par time for E%iM%i from %i to %i seconds", episode, level, oldpar,
                        partime);
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
    long    value;          // All deh values are ints or longs
    char    ch = 0;         // CPhipps - `writable' null string to initialize...
    char    *strval = &ch;  // pointer to the value area
    int     iy;             // array index
    char    *p;             // utility pointer

    if (devparm)
        C_Output("Processing Cheat: %s", line);

    strncpy(inbuffer, line, DEH_BUFFERMAX);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        dboolean    success = false;

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;              // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, &strval))       // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        // Otherwise we got a (perhaps valid) cheat name
        if (M_StringCompare(key, deh_cheat[0]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_mus.sequence = strdup(p);
            cheat_mus_xy.sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[1]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_choppers.sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[2]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_god.sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[3]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_ammo.sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[4]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_ammonokey.sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[5]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_noclip.sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[6]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_commercial_noclip.sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[7]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[0].sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[8]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[1].sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[9]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[2].sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[10]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[3].sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[11]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[4].sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[12]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[5].sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[13]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_powerup[6].sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[14]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_clev.sequence = strdup(p);
            cheat_clev_xy.sequence = strdup(p);
            success = true;
        }
        else if (M_StringCompare(key, deh_cheat[15]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xFF ? '\0' : strval[iy]);

            p = strval;

            while (*p == ' ')
                p++;

            cheat_mypos.sequence = strdup(p);
            success = true;
        }

        if (success && devparm)
            C_Output("Assigned new cheat '%s' to cheat '%s' at index %i", p, cheat_mus.sequence, iy);

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
    long    value;  // All deh values are ints or longs

    strncpy(inbuffer, line, DEH_BUFFERMAX);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        lfstrip(inbuffer);

        if (!*inbuffer)
            break;                                              // killough 11/98

        if (!deh_GetData(inbuffer, key, &value, NULL))          // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        // Otherwise it's ok
        if (devparm)
            C_Output("Processing Misc item '%s'", key);

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
            bfgcells = value;
        else if (M_StringCompare(key, deh_misc[15]))                // Monsters Infight
            species_infighting = value;
        else
            C_Warning("Invalid misc item string index for \"%s\".", key);
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
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX * 2];    // can't use line -- double size buffer too.
    int         i;                              // loop variable
    int         fromlen, tolen;                 // as specified on the text block line
    int         usedlen;                        // shorter of fromlen and tolen if not matched
    dboolean    found = false;                  // to allow early exit once found
    char        *line2 = NULL;                  // duplicate line for rerouting

    // Ty 04/11/98 - Included file may have NOTEXT skip flag set
    if (includenotext)                      // flag to skip included deh-style text
    {
        C_Output("Skipped text block because of NOTEXT directive.");
        strcpy(inbuffer, line);

        while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
            dehfgets(inbuffer, sizeof(inbuffer), fpin); // skip block

        // Ty 05/17/98 - don't care if this fails
        return;                             // ************** Early return
    }

    // killough 8/98: allow hex numbers in input:
    sscanf(line, "%31s %10i %10i", key, &fromlen, &tolen);

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

        while (sprnames[i])     // null terminated list in info.c       // jff 3/19/98
        {                                                               // check pointer
            if (!strncasecmp(sprnames[i], inbuffer, fromlen))           // not first char
            {
                if (devparm)
                    C_Output("Changing name of sprite at index %i from %s to %*s", i, sprnames[i], tolen,
                        &inbuffer[fromlen]);

                // Ty 03/18/98 - not using strdup because length is fixed

                // killough 10/98: but it's an array of pointers, so we must
                // use strdup unless we redeclare sprnames and change all else
                sprnames[i] = strdup(sprnames[i]);

                strncpy(sprnames[i], &inbuffer[fromlen], tolen);
                found = true;
                break;          // only one will match--quit early
            }

            i++;                // next array element
        }
    }
    else if (fromlen < 7 && tolen < 7)   // lengths of music and sfx are 6 or shorter
    {
        usedlen = (fromlen < tolen ? fromlen : tolen);

        if (fromlen != tolen && !hacx)
            C_Warning("Mismatched lengths from %i to %i. Using %i.", fromlen, tolen, usedlen);

        // Try sound effects entries - see sounds.c
        for (i = 1; i < NUMSFX; i++)
        {
            // avoid short prefix erroneous match
            if (strlen(S_sfx[i].name) != fromlen)
                continue;

            if (!strncasecmp(S_sfx[i].name, inbuffer, fromlen))
            {
                if (devparm)
                    C_Output("Changing name of sfx from %s to %*s", S_sfx[i].name, usedlen,
                        &inbuffer[fromlen]);

                strncpy(S_sfx[i].name, &inbuffer[fromlen], 9);
                found = true;
                break;          // only one matches, quit early
            }
        }
        if (!found)             // not yet
        {
            // Try music name entries - see sounds.c
            for (i = 1; i < NUMMUSIC; i++)
            {
                // avoid short prefix erroneous match
                if (strlen(S_music[i].name) != fromlen)
                    continue;

                if (!strncasecmp(S_music[i].name, inbuffer, fromlen))
                {
                    if (devparm)
                        C_Output("Changing name of music from %s to %*s", S_music[i].name, usedlen,
                            &inbuffer[fromlen]);

                    strncpy(S_music[i].name, &inbuffer[fromlen], 9);
                    found = true;
                    break;      // only one matches, quit early
                }
            }
        }                       // end !found test
    }

    if (!found) // Nothing we want to handle here -- see if strings can deal with it.
    {
        if (devparm)
            C_Output("Checking text area through strings for \"%.12s%s\" from = %i to = %i", inbuffer,
            (strlen(inbuffer) > 12 ? "..." : ""), fromlen, tolen);

        if (fromlen <= (int)strlen(inbuffer))
        {
            line2 = strdup(&inbuffer[fromlen]);
            inbuffer[fromlen] = '\0';
        }

        deh_procStringSub(NULL, inbuffer, trimwhitespace(line2));
    }

    free(line2);        // may be NULL, ignored by free()
}

static void deh_procError(DEHFILE *fpin, char *line)
{
    char    inbuffer[DEH_BUFFERMAX];

    strncpy(inbuffer, line, DEH_BUFFERMAX);

    if (devparm)
        C_Warning("Ignoring \"%s\".", inbuffer);
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
    long        value;                  // All deh values are ints or longs
    char        *strval;                // holds the string value of the line
    static int  maxstrlen = 128;        // maximum string length, bumped 128 at a time as needed
                                        // holds the final result of the string after concatenation
    static char *holdstring;

    if (devparm)
        C_Output("Processing extended string substitution");

    if (!holdstring)
        holdstring = malloc(maxstrlen * sizeof(*holdstring));

    *holdstring = '\0';                 // empty string to start with
    strncpy(inbuffer, line, DEH_BUFFERMAX);

    // Ty 04/24/98 - have to allow inbuffer to start with a blank for
    // the continuations of C1TEXT etc.
    while (!dehfeof(fpin) && *inbuffer)
    {
        int len;

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;

        if (*inbuffer == '#')
            continue;                   // skip comment lines

        lfstrip(inbuffer);

        if (!*inbuffer && !*holdstring)
            break;                      // killough 11/98

        if (!*holdstring)               // first one--get the key
            if (!deh_GetData(inbuffer, key, &value, &strval))   // returns TRUE if ok
            {
                C_Warning("Bad data pair in \"%s\".", inbuffer);
                continue;
            }

        len = (int)strlen(inbuffer);

        while (strlen(holdstring) + len > (unsigned int)maxstrlen)
        {
            // killough 11/98: allocate enough the first time
            maxstrlen += (int)strlen(holdstring) + len - maxstrlen;

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
static dboolean deh_procStringSub(char *key, char *lookfor, char *newstring)
{
    dboolean    found = false;  // loop exit flag

    for (int i = 0; i < deh_numstrlookup; i++)
    {
        found = (lookfor ? M_StringCompare(*deh_strlookup[i].ppstr, lookfor) :
            M_StringCompare(deh_strlookup[i].lookup, key));

        if (found)
        {
            char    *t;

            if (deh_strlookup[i].assigned)
                break;

            *deh_strlookup[i].ppstr = t = strdup(newstring);    // orphan originalstring
            found = true;

            // Handle embedded \n's in the incoming string, convert to 0x0a's
            {
                char    *s;

                for (s = *deh_strlookup[i].ppstr; *s; s++, t++)
                    if (*s == '\\' && (s[1] == 'n' || s[1] == 'N'))     // found one
                    {
                        s++;
                        *t = '\n';      // skip one extra for second character
                    }
                    else
                        *t = *s;

                *t = '\0';              // cap off the target string
            }

            if (devparm)
            {
                if (key)
                    C_Output("Assigned key %s to \"%s\"", key, newstring);
                else
                {
                    C_Output("Assigned \"%.12s%s\" to \"%.12s%s\" at key %s", lookfor,
                        (strlen(lookfor) > 12 ? "..." : ""), newstring,
                        (strlen(newstring) > 12 ? "..." : ""), deh_strlookup[i].lookup);
                    C_Output("*BEX FORMAT:");
                    C_Output("%s = %s", deh_strlookup[i].lookup, dehReformatStr(newstring));
                    C_Output("*END BEX");
                }
            }

            deh_strlookup[i].assigned++;

            if (M_StrCaseStr(deh_strlookup[i].lookup, "HUSTR"))
                addtocount = true;

            // [BH] allow either GOTREDSKUL or GOTREDSKULL
            if (M_StringCompare(deh_strlookup[i].lookup, "GOTREDSKUL")
                && !deh_strlookup[p_GOTREDSKULL].assigned)
            {
                s_GOTREDSKULL = s_GOTREDSKUL;
                deh_strlookup[p_GOTREDSKULL].assigned++;
                return true;
            }

            break;
        }
    }

    if (!found && !hacx)
        C_Warning("The <b>%s</b> string can't be found.", (key ? key : lookfor));

    return found;
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

    while (p > s && (*--p == '\r' || *p == '\n'))
        *p = 0;
}

// ====================================================================
// rstrip
// Purpose: Strips trailing blanks off a string
// Args:    s -- the string to work on
// Returns: void -- the string is modified in place
//
static void rstrip(char *s)         // strip trailing whitespace
{
    char    *p = s + strlen(s);     // killough 4/4/98: same here

    while (p > s && isspace(*--p))  // break on first non-whitespace
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
    while (isspace(*p))
        p++;

    return p;
}

// ====================================================================
// deh_GetData
// Purpose: Get a key and data pair from a passed string
// Args:    s -- the string to be examined
//          k -- a place to put the key
//          l -- pointer to a long integer to store the number
//          strval -- a pointer to the place in s where the number
//                    value comes from. Pass NULL to not use this.
// Notes:   Expects a key phrase, optional space, equal sign,
//          optional space and a value, mostly an int but treated
//          as a long just in case. The passed pointer to hold
//          the key must be DEH_MAXKEYLEN in size.
//
static int deh_GetData(char *s, char *k, long *l, char **strval)
{
    char            *t;                     // current char
    unsigned int    val;                    // to hold value of pair
    char            buffer[DEH_MAXKEYLEN];  // to hold key in progress
    int             okrc = 1;               // assume good unless we have problems
    int             i;                      // iterator

    *buffer = '\0';
    val = 0;                            // defaults in case not otherwise set

    for (i = 0, t = s; *t && i < DEH_MAXKEYLEN; t++, i++)
    {
        if (*t == '=')
            break;

        buffer[i] = *t;                 // copy it
    }

    if (isspace(buffer[i - 1]))
        i--;

    buffer[i] = '\0';                   // terminate the key before the '='

    if (!*t)                            // end of string with no equal sign
        okrc = false;
    else
    {
        if (!*++t)
        {
            val = 0;                    // in case "thiskey =" with no value
            okrc = 0;
        }

        // we've incremented t
        if (!M_StrToInt(t, &val))
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
