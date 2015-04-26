/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#include <ctype.h>

#include "c_console.h"
#include "d_deh.h"
#include "doomdef.h"
#include "doomstat.h"
#include "dstrings.h"
#include "sounds.h"
#include "info.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "p_local.h"
#include "g_game.h"
#include "d_think.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

// killough 10/98: new functions, to allow processing DEH files in-memory
// (e.g. from wads)

typedef struct
{
    byte        *inp, *lump;    // Pointer to string or FILE
    long        size;
    FILE        *f;
} DEHFILE;

boolean addtocount;
int     dehcount = 0;
boolean dehacked = false;

// killough 10/98: emulate IO whether input really comes from a file or not

// haleyjd: got rid of macros for MSVC
char *dehfgets(char *buf, size_t n, DEHFILE *fp)
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
    return buf;                                 // Return buffer pointer
}

int dehfeof(DEHFILE *fp)
{
    return (!fp->lump ? feof(fp->f) : !*fp->inp || fp->size <= 0);
}

int dehfgetc(DEHFILE *fp)
{
    return (!fp->lump ? fgetc(fp->f) : fp->size > 0 ? fp->size--, *fp->inp++ : EOF);
}

// variables used in other routines
boolean deh_pars = false;       // in wi_stuff to allow pars in modified games

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
char    *s_GOTMEDINEED2 = "";
char    *s_GOTMEDIKIT = GOTMEDIKIT;
char    *s_GOTSUPER = GOTSUPER;

char    *s_GOTBLUECARD = GOTBLUECARD;
char    *s_GOTYELWCARD = GOTYELWCARD;
char    *s_GOTREDCARD = GOTREDCARD;
char    *s_GOTBLUESKUL = GOTBLUESKUL;
char    *s_GOTYELWSKUL = GOTYELWSKUL;
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

char    *s_GGSAVED = GGSAVED;
char    *s_GGLOADED = "";
char    *s_GSCREENSHOT = "";

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
char    *s_HUSTR_32 = HUSTR_32;
char    *s_HUSTR_31_BFG = "";
char    *s_HUSTR_32_BFG = "";
char    *s_HUSTR_33_BFG = "";
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
char    *s_STSTR_CHOPPERS = STSTR_CHOPPERS;
char    *s_STSTR_CLEV = STSTR_CLEV;
char    *s_STSTR_CLEVSAME = "";
char    *s_STSTR_MYPOS = "";
char    *s_STSTR_NTON = "";
char    *s_STSTR_NTOFF = "";
char    *s_STSTR_GODON = "";
char    *s_STSTR_GODOFF = "";

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
char    *s_CAPTION_HACX = "";
char    *s_CAPTION_FREEDOOM1 = "";
char    *s_CAPTION_FREEDOOM2 = "";
char    *s_CAPTION_FREEDM = "";
char    *s_CAPTION_BTSXE1 = "";
char    *s_CAPTION_BTSXE2 = "";
char    *s_CAPTION_BTSXE3 = "";

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

// end d_deh.h variable declarations
// ====================================================================

// Do this for a lookup--the pointer (loaded above) is cross-referenced
// to a string key that is the same as the define above.  We will use
// strdups to set these new values that we read from the file, orphaning
// the original value set above.

deh_strs deh_strlookup[] =
{
    { &s_D_DEVSTR,             "D_DEVSTR",             false },
    { &s_D_CDROM,              "D_CDROM",              false },

    { &s_PRESSKEY,             "PRESSKEY",             false },
    { &s_PRESSYN,              "PRESSYN",              false },
    { &s_PRESSA,               "PRESSA",               false },
    { &s_QUITMSG,              "QUITMSG",              false },
    { &s_LOADNET,              "LOADNET",              false },
    { &s_QLOADNET,             "QLOADNET",             false },
    { &s_QSAVESPOT,            "QSAVESPOT",            false },
    { &s_SAVEDEAD,             "SAVEDEAD",             false },
    { &s_QSPROMPT,             "QSPROMPT",             false },
    { &s_QLPROMPT,             "QLPROMPT",             false },
    { &s_NEWGAME,              "NEWGAME",              false },
    { &s_NIGHTMARE,            "NIGHTMARE",            false },
    { &s_SWSTRING,             "SWSTRING",             false },
    { &s_MSGOFF,               "MSGOFF",               false },
    { &s_MSGON,                "MSGON",                false },
    { &s_NETEND,               "NETEND",               false },
    { &s_ENDGAME,              "ENDGAME",              false },
    { &s_DOSY,                 "DOSY",                 false },
    { &s_DOSA,                 "DOSA",                 false },
    { &s_OTHERY,               "OTHERY",               false },
    { &s_OTHERA,               "OTHERA",               false },
    { &s_DETAILHI,             "DETAILHI",             false },
    { &s_DETAILLO,             "DETAILLO",             false },
    { &s_GAMMALVL0,            "GAMMALVL0",            false },
    { &s_GAMMALVL1,            "GAMMALVL1",            false },
    { &s_GAMMALVL2,            "GAMMALVL2",            false },
    { &s_GAMMALVL3,            "GAMMALVL3",            false },
    { &s_GAMMALVL4,            "GAMMALVL4",            false },
    { &s_GAMMALVL,             "GAMMALVL",             false },
    { &s_GAMMAOFF,             "GAMMAOFF",             false },
    { &s_EMPTYSTRING,          "EMPTYSTRING",          false },

    { &s_GOTARMOR,             "GOTARMOR",             false },
    { &s_GOTMEGA,              "GOTMEGA",              false },
    { &s_GOTHTHBONUS,          "GOTHTHBONUS",          false },
    { &s_GOTARMBONUS,          "GOTARMBONUS",          false },
    { &s_GOTSTIM,              "GOTSTIM",              false },
    { &s_GOTMEDINEED,          "GOTMEDINEED",          false },
    { &s_GOTMEDINEED2,         "GOTMEDINEED2",         false },
    { &s_GOTMEDIKIT,           "GOTMEDIKIT",           false },
    { &s_GOTSUPER,             "GOTSUPER",             false },

    { &s_GOTBLUECARD,          "GOTBLUECARD",          false },
    { &s_GOTYELWCARD,          "GOTYELWCARD",          false },
    { &s_GOTREDCARD,           "GOTREDCARD",           false },
    { &s_GOTBLUESKUL,          "GOTBLUESKUL",          false },
    { &s_GOTYELWSKUL,          "GOTYELWSKUL",          false },
    { &s_GOTREDSKULL,          "GOTREDSKULL",          false },

    { &s_GOTINVUL,             "GOTINVUL",             false },
    { &s_GOTBERSERK,           "GOTBERSERK",           false },
    { &s_GOTINVIS,             "GOTINVIS",             false },
    { &s_GOTSUIT,              "GOTSUIT",              false },
    { &s_GOTMAP,               "GOTMAP",               false },
    { &s_GOTVISOR,             "GOTVISOR",             false },

    { &s_GOTCLIP,              "GOTCLIP",              false },
    { &s_GOTCLIPX2,            "GOTCLIPX2",            false },
    { &s_GOTHALFCLIP,          "GOTHALFCLIP",          false },
    { &s_GOTCLIPBOX,           "GOTCLIPBOX",           false },
    { &s_GOTROCKET,            "GOTROCKET",            false },
    { &s_GOTROCKETX2,          "GOTROCKETX2",          false },
    { &s_GOTROCKBOX,           "GOTROCKBOX",           false },
    { &s_GOTCELL,              "GOTCELL",              false },
    { &s_GOTCELLX2,            "GOTCELLX2",            false },
    { &s_GOTCELLBOX,           "GOTCELLBOX",           false },
    { &s_GOTSHELLS,            "GOTSHELLS",            false },
    { &s_GOTSHELLSX2,          "GOTSHELLSX2",          false },
    { &s_GOTSHELLBOX,          "GOTSHELLBOX",          false },
    { &s_GOTBACKPACK,          "GOTBACKPACK",          false },

    { &s_GOTBFG9000,           "GOTBFG9000",           false },
    { &s_GOTCHAINGUN,          "GOTCHAINGUN",          false },
    { &s_GOTCHAINSAW,          "GOTCHAINSAW",          false },
    { &s_GOTLAUNCHER,          "GOTLAUNCHER",          false },
    { &s_GOTMSPHERE,           "GOTMSPHERE",           false },
    { &s_GOTPLASMA,            "GOTPLASMA",            false },
    { &s_GOTSHOTGUN,           "GOTSHOTGUN",           false },
    { &s_GOTSHOTGUN2,          "GOTSHOTGUN2",          false },

    { &s_PD_BLUEO,             "PD_BLUEO",             false },
    { &s_PD_REDO,              "PD_REDO",              false },
    { &s_PD_YELLOWO,           "PD_YELLOWO",           false },
    { &s_PD_BLUEK,             "PD_BLUEK",             false },
    { &s_PD_REDK,              "PD_REDK",              false },
    { &s_PD_YELLOWK,           "PD_YELLOWK",           false },

    { &s_GGSAVED,              "GGSAVED",              false },
    { &s_GGLOADED,             "GGLOADED",             false },
    { &s_GSCREENSHOT,          "GSCREENSHOT",          false },

    { &s_ALWAYSRUNOFF,         "ALWAYSRUNOFF",         false },
    { &s_ALWAYSRUNON,          "ALWAYSRUNON",          false },

    { &s_HUSTR_MSGU,           "HUSTR_MSGU",           false },

    { &s_HUSTR_E1M1,           "HUSTR_E1M1",           false },
    { &s_HUSTR_E1M2,           "HUSTR_E1M2",           false },
    { &s_HUSTR_E1M3,           "HUSTR_E1M3",           false },
    { &s_HUSTR_E1M4,           "HUSTR_E1M4",           false },
    { &s_HUSTR_E1M5,           "HUSTR_E1M5",           false },
    { &s_HUSTR_E1M6,           "HUSTR_E1M6",           false },
    { &s_HUSTR_E1M7,           "HUSTR_E1M7",           false },
    { &s_HUSTR_E1M8,           "HUSTR_E1M8",           false },
    { &s_HUSTR_E1M9,           "HUSTR_E1M9",           false },
    { &s_HUSTR_E2M1,           "HUSTR_E2M1",           false },
    { &s_HUSTR_E2M2,           "HUSTR_E2M2",           false },
    { &s_HUSTR_E2M3,           "HUSTR_E2M3",           false },
    { &s_HUSTR_E2M4,           "HUSTR_E2M4",           false },
    { &s_HUSTR_E2M5,           "HUSTR_E2M5",           false },
    { &s_HUSTR_E2M6,           "HUSTR_E2M6",           false },
    { &s_HUSTR_E2M7,           "HUSTR_E2M7",           false },
    { &s_HUSTR_E2M8,           "HUSTR_E2M8",           false },
    { &s_HUSTR_E2M9,           "HUSTR_E2M9",           false },
    { &s_HUSTR_E3M1,           "HUSTR_E3M1",           false },
    { &s_HUSTR_E3M2,           "HUSTR_E3M2",           false },
    { &s_HUSTR_E3M3,           "HUSTR_E3M3",           false },
    { &s_HUSTR_E3M4,           "HUSTR_E3M4",           false },
    { &s_HUSTR_E3M5,           "HUSTR_E3M5",           false },
    { &s_HUSTR_E3M6,           "HUSTR_E3M6",           false },
    { &s_HUSTR_E3M7,           "HUSTR_E3M7",           false },
    { &s_HUSTR_E3M8,           "HUSTR_E3M8",           false },
    { &s_HUSTR_E3M9,           "HUSTR_E3M9",           false },
    { &s_HUSTR_E4M1,           "HUSTR_E4M1",           false },
    { &s_HUSTR_E4M2,           "HUSTR_E4M2",           false },
    { &s_HUSTR_E4M3,           "HUSTR_E4M3",           false },
    { &s_HUSTR_E4M4,           "HUSTR_E4M4",           false },
    { &s_HUSTR_E4M5,           "HUSTR_E4M5",           false },
    { &s_HUSTR_E4M6,           "HUSTR_E4M6",           false },
    { &s_HUSTR_E4M7,           "HUSTR_E4M7",           false },
    { &s_HUSTR_E4M8,           "HUSTR_E4M8",           false },
    { &s_HUSTR_E4M9,           "HUSTR_E4M9",           false },
    { &s_HUSTR_1,              "HUSTR_1",              false },
    { &s_HUSTR_2,              "HUSTR_2",              false },
    { &s_HUSTR_3,              "HUSTR_3",              false },
    { &s_HUSTR_4,              "HUSTR_4",              false },
    { &s_HUSTR_5,              "HUSTR_5",              false },
    { &s_HUSTR_6,              "HUSTR_6",              false },
    { &s_HUSTR_7,              "HUSTR_7",              false },
    { &s_HUSTR_8,              "HUSTR_8",              false },
    { &s_HUSTR_9,              "HUSTR_9",              false },
    { &s_HUSTR_10,             "HUSTR_10",             false },
    { &s_HUSTR_11,             "HUSTR_11",             false },
    { &s_HUSTR_12,             "HUSTR_12",             false },
    { &s_HUSTR_13,             "HUSTR_13",             false },
    { &s_HUSTR_14,             "HUSTR_14",             false },
    { &s_HUSTR_15,             "HUSTR_15",             false },
    { &s_HUSTR_16,             "HUSTR_16",             false },
    { &s_HUSTR_17,             "HUSTR_17",             false },
    { &s_HUSTR_18,             "HUSTR_18",             false },
    { &s_HUSTR_19,             "HUSTR_19",             false },
    { &s_HUSTR_20,             "HUSTR_20",             false },
    { &s_HUSTR_21,             "HUSTR_21",             false },
    { &s_HUSTR_22,             "HUSTR_22",             false },
    { &s_HUSTR_23,             "HUSTR_23",             false },
    { &s_HUSTR_24,             "HUSTR_24",             false },
    { &s_HUSTR_25,             "HUSTR_25",             false },
    { &s_HUSTR_26,             "HUSTR_26",             false },
    { &s_HUSTR_27,             "HUSTR_27",             false },
    { &s_HUSTR_28,             "HUSTR_28",             false },
    { &s_HUSTR_29,             "HUSTR_29",             false },
    { &s_HUSTR_30,             "HUSTR_30",             false },
    { &s_HUSTR_31,             "HUSTR_31",             false },
    { &s_HUSTR_32,             "HUSTR_32",             false },
    { &s_HUSTR_31_BFG,         "HUSTR_31_BFG",         false },
    { &s_HUSTR_32_BFG,         "HUSTR_32_BFG",         false },
    { &s_HUSTR_33_BFG,         "HUSTR_33_BFG",         false },
    { &s_PHUSTR_1,             "PHUSTR_1",             false },
    { &s_PHUSTR_2,             "PHUSTR_2",             false },
    { &s_PHUSTR_3,             "PHUSTR_3",             false },
    { &s_PHUSTR_4,             "PHUSTR_4",             false },
    { &s_PHUSTR_5,             "PHUSTR_5",             false },
    { &s_PHUSTR_6,             "PHUSTR_6",             false },
    { &s_PHUSTR_7,             "PHUSTR_7",             false },
    { &s_PHUSTR_8,             "PHUSTR_8",             false },
    { &s_PHUSTR_9,             "PHUSTR_9",             false },
    { &s_PHUSTR_10,            "PHUSTR_10",            false },
    { &s_PHUSTR_11,            "PHUSTR_11",            false },
    { &s_PHUSTR_12,            "PHUSTR_12",            false },
    { &s_PHUSTR_13,            "PHUSTR_13",            false },
    { &s_PHUSTR_14,            "PHUSTR_14",            false },
    { &s_PHUSTR_15,            "PHUSTR_15",            false },
    { &s_PHUSTR_16,            "PHUSTR_16",            false },
    { &s_PHUSTR_17,            "PHUSTR_17",            false },
    { &s_PHUSTR_18,            "PHUSTR_18",            false },
    { &s_PHUSTR_19,            "PHUSTR_19",            false },
    { &s_PHUSTR_20,            "PHUSTR_20",            false },
    { &s_PHUSTR_21,            "PHUSTR_21",            false },
    { &s_PHUSTR_22,            "PHUSTR_22",            false },
    { &s_PHUSTR_23,            "PHUSTR_23",            false },
    { &s_PHUSTR_24,            "PHUSTR_24",            false },
    { &s_PHUSTR_25,            "PHUSTR_25",            false },
    { &s_PHUSTR_26,            "PHUSTR_26",            false },
    { &s_PHUSTR_27,            "PHUSTR_27",            false },
    { &s_PHUSTR_28,            "PHUSTR_28",            false },
    { &s_PHUSTR_29,            "PHUSTR_29",            false },
    { &s_PHUSTR_30,            "PHUSTR_30",            false },
    { &s_PHUSTR_31,            "PHUSTR_31",            false },
    { &s_PHUSTR_32,            "PHUSTR_32",            false },
    { &s_THUSTR_1,             "THUSTR_1",             false },
    { &s_THUSTR_2,             "THUSTR_2",             false },
    { &s_THUSTR_3,             "THUSTR_3",             false },
    { &s_THUSTR_4,             "THUSTR_4",             false },
    { &s_THUSTR_5,             "THUSTR_5",             false },
    { &s_THUSTR_6,             "THUSTR_6",             false },
    { &s_THUSTR_7,             "THUSTR_7",             false },
    { &s_THUSTR_8,             "THUSTR_8",             false },
    { &s_THUSTR_9,             "THUSTR_9",             false },
    { &s_THUSTR_10,            "THUSTR_10",            false },
    { &s_THUSTR_11,            "THUSTR_11",            false },
    { &s_THUSTR_12,            "THUSTR_12",            false },
    { &s_THUSTR_13,            "THUSTR_13",            false },
    { &s_THUSTR_14,            "THUSTR_14",            false },
    { &s_THUSTR_15,            "THUSTR_15",            false },
    { &s_THUSTR_16,            "THUSTR_16",            false },
    { &s_THUSTR_17,            "THUSTR_17",            false },
    { &s_THUSTR_18,            "THUSTR_18",            false },
    { &s_THUSTR_19,            "THUSTR_19",            false },
    { &s_THUSTR_20,            "THUSTR_20",            false },
    { &s_THUSTR_21,            "THUSTR_21",            false },
    { &s_THUSTR_22,            "THUSTR_22",            false },
    { &s_THUSTR_23,            "THUSTR_23",            false },
    { &s_THUSTR_24,            "THUSTR_24",            false },
    { &s_THUSTR_25,            "THUSTR_25",            false },
    { &s_THUSTR_26,            "THUSTR_26",            false },
    { &s_THUSTR_27,            "THUSTR_27",            false },
    { &s_THUSTR_28,            "THUSTR_28",            false },
    { &s_THUSTR_29,            "THUSTR_29",            false },
    { &s_THUSTR_30,            "THUSTR_30",            false },
    { &s_THUSTR_31,            "THUSTR_31",            false },
    { &s_THUSTR_32,            "THUSTR_32",            false },
    { &s_NHUSTR_1,             "NHUSTR_1",             false },
    { &s_NHUSTR_2,             "NHUSTR_2",             false },
    { &s_NHUSTR_3,             "NHUSTR_3",             false },
    { &s_NHUSTR_4,             "NHUSTR_4",             false },
    { &s_NHUSTR_5,             "NHUSTR_5",             false },
    { &s_NHUSTR_6,             "NHUSTR_6",             false },
    { &s_NHUSTR_7,             "NHUSTR_7",             false },
    { &s_NHUSTR_8,             "NHUSTR_8",             false },
    { &s_NHUSTR_9,             "NHUSTR_9",             false },

    { &s_HUSTR_CHATMACRO1,     "HUSTR_CHATMACRO1",     false },
    { &s_HUSTR_CHATMACRO2,     "HUSTR_CHATMACRO2",     false },
    { &s_HUSTR_CHATMACRO3,     "HUSTR_CHATMACRO3",     false },
    { &s_HUSTR_CHATMACRO4,     "HUSTR_CHATMACRO4",     false },
    { &s_HUSTR_CHATMACRO5,     "HUSTR_CHATMACRO5",     false },
    { &s_HUSTR_CHATMACRO6,     "HUSTR_CHATMACRO6",     false },
    { &s_HUSTR_CHATMACRO7,     "HUSTR_CHATMACRO7",     false },
    { &s_HUSTR_CHATMACRO8,     "HUSTR_CHATMACRO8",     false },
    { &s_HUSTR_CHATMACRO9,     "HUSTR_CHATMACRO9",     false },
    { &s_HUSTR_CHATMACRO0,     "HUSTR_CHATMACRO0",     false },
    { &s_HUSTR_TALKTOSELF1,    "HUSTR_TALKTOSELF1",    false },
    { &s_HUSTR_TALKTOSELF2,    "HUSTR_TALKTOSELF2",    false },
    { &s_HUSTR_TALKTOSELF3,    "HUSTR_TALKTOSELF3",    false },
    { &s_HUSTR_TALKTOSELF4,    "HUSTR_TALKTOSELF4",    false },
    { &s_HUSTR_TALKTOSELF5,    "HUSTR_TALKTOSELF5",    false },
    { &s_HUSTR_MESSAGESENT,    "HUSTR_MESSAGESENT",    false },
    { &s_HUSTR_PLRGREEN,       "HUSTR_PLRGREEN",       false },
    { &s_HUSTR_PLRINDIGO,      "HUSTR_PLRINDIGO",      false },
    { &s_HUSTR_PLRBROWN,       "HUSTR_PLRBROWN",       false },
    { &s_HUSTR_PLRRED,         "HUSTR_PLRRED",         false },

    { &s_AMSTR_FOLLOWON,       "AMSTR_FOLLOWON",       false },
    { &s_AMSTR_FOLLOWOFF,      "AMSTR_FOLLOWOFF",      false },
    { &s_AMSTR_GRIDON,         "AMSTR_GRIDON",         false },
    { &s_AMSTR_GRIDOFF,        "AMSTR_GRIDOFF",        false },
    { &s_AMSTR_MARKEDSPOT,     "AMSTR_MARKEDSPOT",     false },
    { &s_AMSTR_MARKCLEARED,    "AMSTR_MARKCLEARED",    false },
    { &s_AMSTR_MARKSCLEARED,   "AMSTR_MARKSCLEARED",   false },
    { &s_AMSTR_ROTATEON,       "AMSTR_ROTATEON",       false },
    { &s_AMSTR_ROTATEOFF,      "AMSTR_ROTATEOFF",      false },

    { &s_STSTR_MUS,            "STSTR_MUS",            false },
    { &s_STSTR_NOMUS,          "STSTR_NOMUS",          false },
    { &s_STSTR_DQDON,          "STSTR_DQDON",          false },
    { &s_STSTR_DQDOFF,         "STSTR_DQDOFF",         false },
    { &s_STSTR_KFAADDED,       "STSTR_KFAADDED",       false },
    { &s_STSTR_FAADDED,        "STSTR_FAADDED",        false },
    { &s_STSTR_NCON,           "STSTR_NCON",           false },
    { &s_STSTR_NCOFF,          "STSTR_NCOFF",          false },
    { &s_STSTR_BEHOLD,         "STSTR_BEHOLD",         false },
    { &s_STSTR_BEHOLDX,        "STSTR_BEHOLDX",        false },
    { &s_STSTR_BEHOLDON,       "STSTR_BEHOLDON",       false },
    { &s_STSTR_BEHOLDOFF,      "STSTR_BEHOLDOFF",      false },
    { &s_STSTR_CHOPPERS,       "STSTR_CHOPPERS",       false },
    { &s_STSTR_CLEV,           "STSTR_CLEV",           false },
    { &s_STSTR_CLEVSAME,       "STSTR_CLEVSAME",       false },
    { &s_STSTR_MYPOS,          "STSTR_MYPOS",          false },
    { &s_STSTR_NTON,           "STSTR_NTON",           false },
    { &s_STSTR_NTOFF,          "STSTR_NTOFF",          false },
    { &s_STSTR_GODON,          "STSTR_GODON",          false },
    { &s_STSTR_GODOFF,         "STSTR_GODOFF",         false },

    { &s_E1TEXT,               "E1TEXT",               false },
    { &s_E2TEXT,               "E2TEXT",               false },
    { &s_E3TEXT,               "E3TEXT",               false },
    { &s_E4TEXT,               "E4TEXT",               false },
    { &s_C1TEXT,               "C1TEXT",               false },
    { &s_C2TEXT,               "C2TEXT",               false },
    { &s_C3TEXT,               "C3TEXT",               false },
    { &s_C4TEXT,               "C4TEXT",               false },
    { &s_C5TEXT,               "C5TEXT",               false },
    { &s_C6TEXT,               "C6TEXT",               false },
    { &s_P1TEXT,               "P1TEXT",               false },
    { &s_P2TEXT,               "P2TEXT",               false },
    { &s_P3TEXT,               "P3TEXT",               false },
    { &s_P4TEXT,               "P4TEXT",               false },
    { &s_P5TEXT,               "P5TEXT",               false },
    { &s_P6TEXT,               "P6TEXT",               false },
    { &s_T1TEXT,               "T1TEXT",               false },
    { &s_T2TEXT,               "T2TEXT",               false },
    { &s_T3TEXT,               "T3TEXT",               false },
    { &s_T4TEXT,               "T4TEXT",               false },
    { &s_T5TEXT,               "T5TEXT",               false },
    { &s_T6TEXT,               "T6TEXT",               false },
    { &s_N1TEXT,               "N1TEXT",               false },

    { &s_CC_ZOMBIE,            "CC_ZOMBIE",            false },
    { &s_CC_SHOTGUN,           "CC_SHOTGUN",           false },
    { &s_CC_HEAVY,             "CC_HEAVY",             false },
    { &s_CC_IMP,               "CC_IMP",               false },
    { &s_CC_DEMON,             "CC_DEMON",             false },
    { &s_CC_SPECTRE,           "CC_SPECTRE",           false },
    { &s_CC_LOST,              "CC_LOST",              false },
    { &s_CC_CACO,              "CC_CACO",              false },
    { &s_CC_HELL,              "CC_HELL",              false },
    { &s_CC_BARON,             "CC_BARON",             false },
    { &s_CC_ARACH,             "CC_ARACH",             false },
    { &s_CC_PAIN,              "CC_PAIN",              false },
    { &s_CC_REVEN,             "CC_REVEN",             false },
    { &s_CC_MANCU,             "CC_MANCU",             false },
    { &s_CC_ARCH,              "CC_ARCH",              false },
    { &s_CC_SPIDER,            "CC_SPIDER",            false },
    { &s_CC_CYBER,             "CC_CYBER",             false },
    { &s_CC_HERO,              "CC_HERO",              false },

    { &s_DOOM_ENDMSG1,         "DOOM_ENDMSG1",         false },
    { &s_DOOM_ENDMSG2,         "DOOM_ENDMSG2",         false },
    { &s_DOOM_ENDMSG3,         "DOOM_ENDMSG3",         false },
    { &s_DOOM_ENDMSG4,         "DOOM_ENDMSG4",         false },
    { &s_DOOM_ENDMSG5,         "DOOM_ENDMSG5",         false },
    { &s_DOOM_ENDMSG6,         "DOOM_ENDMSG6",         false },
    { &s_DOOM_ENDMSG7,         "DOOM_ENDMSG7",         false },
    { &s_DOOM2_ENDMSG1,        "DOOM2_ENDMSG1",        false },
    { &s_DOOM2_ENDMSG2,        "DOOM2_ENDMSG2",        false },
    { &s_DOOM2_ENDMSG3,        "DOOM2_ENDMSG3",        false },
    { &s_DOOM2_ENDMSG4,        "DOOM2_ENDMSG4",        false },
    { &s_DOOM2_ENDMSG5,        "DOOM2_ENDMSG5",        false },
    { &s_DOOM2_ENDMSG6,        "DOOM2_ENDMSG6",        false },
    { &s_DOOM2_ENDMSG7,        "DOOM2_ENDMSG7",        false },

    { &s_M_NEWGAME,            "M_NEWGAME",            false },
    { &s_M_OPTIONS,            "M_OPTIONS",            false },
    { &s_M_LOADGAME,           "M_LOADGAME",           false },
    { &s_M_SAVEGAME,           "M_SAVEGAME",           false },
    { &s_M_QUITGAME,           "M_QUITGAME",           false },
    { &s_M_WHICHEPISODE,       "M_WHICHEPISODE",       false },
    { &s_M_EPISODE1,           "M_EPISODE1",           false },
    { &s_M_EPISODE2,           "M_EPISODE2",           false },
    { &s_M_EPISODE3,           "M_EPISODE3",           false },
    { &s_M_EPISODE4,           "M_EPISODE4",           false },
    { &s_M_WHICHEXPANSION,     "M_WHICHEXPANSION",     false },
    { &s_M_EXPANSION1,         "M_EXPANSION1",         false },
    { &s_M_EXPANSION2,         "M_EXPANSION2",         false },
    { &s_M_CHOOSESKILLLEVEL,   "M_CHOOSESKILLLEVEL",   false },
    { &s_M_SKILLLEVEL1,        "M_SKILLLEVEL1",        false },
    { &s_M_SKILLLEVEL2,        "M_SKILLLEVEL2",        false },
    { &s_M_SKILLLEVEL3,        "M_SKILLLEVEL3",        false },
    { &s_M_SKILLLEVEL4,        "M_SKILLLEVEL4",        false },
    { &s_M_SKILLLEVEL5,        "M_SKILLLEVEL5",        false },
    { &s_M_ENDGAME,            "M_ENDGAME",            false },
    { &s_M_MESSAGES,           "M_MESSAGES",           false },
    { &s_M_ON,                 "M_ON",                 false },
    { &s_M_OFF,                "M_OFF",                false },
    { &s_M_GRAPHICDETAIL,      "M_GRAPHICDETAIL",      false },
    { &s_M_HIGH,               "M_HIGH",               false },
    { &s_M_LOW,                "M_LOW",                false },
    { &s_M_SCREENSIZE,         "M_SCREENSIZE",         false },
    { &s_M_MOUSESENSITIVITY,   "M_MOUSESENSITIVITY",   false },
    { &s_M_GAMEPADSENSITIVITY, "M_GAMEPADSENSITIVITY", false },
    { &s_M_SOUNDVOLUME,        "M_SOUNDVOLUME",        false },
    { &s_M_SFXVOLUME,          "M_SFXVOLUME",          false },
    { &s_M_MUSICVOLUME,        "M_MUSICVOLUME",        false },
    { &s_M_PAUSED,             "M_PAUSED",             false },
    { &s_CAPTION_SHAREWARE,    "CAPTION_SHAREWARE",    false },
    { &s_CAPTION_REGISTERED,   "CAPTION_REGISTERED",   false },
    { &s_CAPTION_ULTIMATE,     "CAPTION_ULTIMATE",     false },
    { &s_CAPTION_DOOM2,        "CAPTION_DOOM2",        false },
    { &s_CAPTION_HELLONEARTH,  "CAPTION_HELLONEARTH",  false },
    { &s_CAPTION_NERVE,        "CAPTION_NERVE",        false },
    { &s_CAPTION_BFGEDITION,   "CAPTION_BFGEDITION",   false },
    { &s_CAPTION_PLUTONIA,     "CAPTION_PLUTONIA",     false },
    { &s_CAPTION_TNT,          "CAPTION_TNT",          false },
    { &s_CAPTION_CHEX,         "CAPTION_CHEX",         false },
    { &s_CAPTION_HACX,         "CAPTION_HACX",         false },
    { &s_CAPTION_FREEDOOM1,    "CAPTION_FREEDOOM1",    false },
    { &s_CAPTION_FREEDOOM2,    "CAPTION_FREEDOOM2",    false },
    { &s_CAPTION_FREEDM,       "CAPTION_FREEDM",       false },
    { &s_CAPTION_BTSXE1,       "CAPTION_BTSXE1",       false },
    { &s_CAPTION_BTSXE2,       "CAPTION_BTSXE2",       false },
    { &s_CAPTION_BTSXE3,       "CAPTION_BTSXE3",       false },

    { &bgflatE1,               "BGFLATE1",             false },
    { &bgflatE2,               "BGFLATE2",             false },
    { &bgflatE3,               "BGFLATE3",             false },
    { &bgflatE4,               "BGFLATE4",             false },
    { &bgflat06,               "BGFLAT06",             false },
    { &bgflat11,               "BGFLAT11",             false },
    { &bgflat15,               "BGFLAT15",             false },
    { &bgflat20,               "BGFLAT20",             false },
    { &bgflat30,               "BGFLAT30",             false },
    { &bgflat31,               "BGFLAT31",             false },
    { &bgcastcall,             "BGCASTCALL",           false },

    // Ty 04/08/98 - added 5 general purpose startup announcement
    // strings for hacker use.  See m_menu.c
    { &startup1,               "STARTUP1",             false },
    { &startup2,               "STARTUP2",             false },
    { &startup3,               "STARTUP3",             false },
    { &startup4,               "STARTUP4",             false },
    { &startup5,               "STARTUP5",             false },

    { &savegamename,           "SAVEGAMENAME",         false },

    { &s_BANNER1,              "BANNER1",              false },
    { &s_BANNER2,              "BANNER2",              false },
    { &s_BANNER3,              "BANNER3",              false },
    { &s_BANNER4,              "BANNER4",              false },
    { &s_BANNER5,              "BANNER5",              false },
    { &s_BANNER6,              "BANNER6",              false },
    { &s_BANNER7,              "BANNER7",              false },

    { &s_COPYRIGHT1,           "COPYRIGHT1",           false },
    { &s_COPYRIGHT2,           "COPYRIGHT2",           false },
    { &s_COPYRIGHT3,           "COPYRIGHT3",           false }
};

static int deh_numstrlookup = sizeof(deh_strlookup) / sizeof(deh_strlookup[0]);

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
    &deh_newlevel,      // spares?  Unused.
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel,
    &deh_newlevel
};

char **mapnames2[] =     // DOOM 2 map names.
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

char **mapnames2_bfg[] =     // DOOM 2 map names.
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
    &s_HUSTR_33_BFG
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
void lfstrip(char *);           // strip the \r and/or \n off of a line
void rstrip(char *);            // strip trailing whitespace
char *ptr_lstrip(char *);       // point past leading whitespace
boolean deh_GetData(char *, char *, long *, char **, FILE *);
boolean deh_procStringSub(char *, char *, char *, FILE *);
char *dehReformatStr(char *);

// Prototypes for block processing functions
// Pointers to these functions are used as the blocks are encountered.
void deh_procThing(DEHFILE *, FILE *, char *);
void deh_procFrame(DEHFILE *, FILE *, char *);
void deh_procPointer(DEHFILE *, FILE *, char *);
void deh_procSounds(DEHFILE *, FILE *, char *);
void deh_procAmmo(DEHFILE *, FILE *, char *);
void deh_procWeapon(DEHFILE *, FILE *, char *);
void deh_procSprite(DEHFILE *, FILE *, char *);
void deh_procCheat(DEHFILE *, FILE *, char *);
void deh_procMisc(DEHFILE *, FILE *, char *);
void deh_procText(DEHFILE *, FILE *, char *);
void deh_procPars(DEHFILE *, FILE *, char *);
void deh_procStrings(DEHFILE *, FILE *, char *);
void deh_procError(DEHFILE *, FILE *, char *);
void deh_procBexCodePointers(DEHFILE *, FILE *, char *);

// Structure deh_block is used to hold the block names that can
// be encountered, and the routines to use to decipher them
typedef struct
{
    char        *key;                                   // a mnemonic block code name
    void (*const fptr)(DEHFILE *, FILE *, char *);      // handler
} deh_block;

#define DEH_BUFFERMAX   1024    // input buffer area size, hardcoded for now
// killough 8/9/98: make DEH_BLOCKMAX self-adjusting
#define DEH_BLOCKMAX    (sizeof(deh_blocks) / sizeof(*deh_blocks))      // size of array
#define DEH_MAXKEYLEN   32      // as much of any key as we'll look at
#define DEH_MOBJINFOMAX 27      // number of ints in the mobjinfo_t structure (!)

// Put all the block header values, and the function to be called when that
// one is encountered, in this array:
static deh_block deh_blocks[] =
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
static boolean includenotext = false;

// MOBJINFO - Dehacked block name = "Thing"
// Usage: Thing nn (name)
// These are for mobjinfo_t types.  Each is an integer
// within the structure, so we can use index of the string in this
// array to offset by sizeof(int) into the mobjinfo_t array at [nn]
// * things are base zero but dehacked considers them to start at #1. ***
static char *deh_mobjinfo[DEH_MOBJINFOMAX] =
{
    "ID #",                     // .doomednum
    "Initial frame",            // .spawnstate
    "Hit points",               // .spawnhealth
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
    "Height",                   // .height
    "Projectile pass height",   // .projectilepassheight
    "Mass",                     // .mass
    "Missile damage",           // .damage
    "Action sound",             // .activesound
    "Bits",                     // .flags
    "Bits2",                    // .flags2
    "Respawn frame",            // .raisestate
    "Frames",                   // .frames
    "Blood"                     // .blood
};

// Strings that are used to indicate flags ("Bits" in mobjinfo)
// This is an array of bit masks that are related to p_mobj.h
// values, using the same names without the MF_ in front.
// Ty 08/27/98 new code
//
// killough 10/98:
//
// Convert array to struct to allow multiple values, make array size variable
#define DEH_MOBJFLAGMAX (sizeof(deh_mobjflags) / sizeof(*deh_mobjflags))

struct deh_mobjflags_s
{
    char        *name;
    long        value;
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
    { "TRANSLATION",  MF_TRANSLATION  }     // color translation
};

// STATE - Dehacked block name = "Frame" and "Pointer"
// Usage: Frame nn
// Usage: Pointer nn (Frame nn)
// These are indexed separately, for lookup to the actual
// function pointers.  Here we'll take whatever Dehacked gives
// us and go from there.  The (Frame nn) after the pointer is the
// real place to put this value.  The "Pointer" value is an xref
// that Dehacked uses and is useless to us.
// * states are base zero and have a dummy #0 (TROO)
static char *deh_state[] =
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
// into the pwad, but not changed here.  Can you tell that Gregdidn't
// know what they were for, mostly?  Can you tell that I don't either?
// Mostly I just put these into the same slots as they are in the struct.
// This may not be supported in our -deh option if it doesn't make sense by then.

// * sounds are base zero but have a dummy #0
static char *deh_sfxinfo[] =
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

// MUSICINFO is not supported in Dehacked.  Ignored here.
// * music entries are base zero but have a dummy #0

// SPRITE - Dehacked block name = "Sprite"
// Usage = Sprite nn
// Sprite redirection by offset into the text area - unsupported by BOOM
// * sprites are base zero and dehacked uses it that way.
//static char *deh_sprite[] =
//{
//    "Offset"            // supposed to be the offset into the text section
//};

// AMMO - Dehacked block name = "Ammo"
// usage = Ammo n (name)
// Ammo information for the few types of ammo
static char *deh_ammo[] =
{
    "Max ammo",         // maxammo[]
    "Per ammo"          // clipammo[]
};

// WEAPONS - Dehacked block name = "Weapon"
// Usage: Weapon nn (name)
// Basically a list of frames and what kind of ammo (see above) it uses.
static char *deh_weapon[] =
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
// Always uses a zero in the dehacked file, for consistency.  No meaning.
// These are just plain funky terms compared with id's
static char *deh_cheat[] =
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
// Always uses a zero in the dehacked file, for consistency.  No meaning.
static char *deh_misc[] =
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
extern void A_Light0();
extern void A_WeaponReady();
extern void A_Lower();
extern void A_Raise();
extern void A_Punch();
extern void A_ReFire();
extern void A_FirePistol();
extern void A_Light1();
extern void A_FireShotgun();
extern void A_Light2();
extern void A_FireShotgun2();
extern void A_CheckReload();
extern void A_OpenShotgun2();
extern void A_LoadShotgun2();
extern void A_CloseShotgun2();
extern void A_FireCGun();
extern void A_GunFlash();
extern void A_FireMissile();
extern void A_Saw();
extern void A_FirePlasma();
extern void A_BFGsound();
extern void A_FireBFG();
extern void A_BFGSpray();
extern void A_Explode();
extern void A_Pain();
extern void A_PlayerScream();
extern void A_Fall();
extern void A_XScream();
extern void A_Look();
extern void A_Chase();
extern void A_FaceTarget();
extern void A_PosAttack();
extern void A_Scream();
extern void A_SPosAttack();
extern void A_VileChase();
extern void A_VileStart();
extern void A_VileTarget();
extern void A_VileAttack();
extern void A_StartFire();
extern void A_Fire();
extern void A_FireCrackle();
extern void A_Tracer();
extern void A_SkelWhoosh();
extern void A_SkelFist();
extern void A_SkelMissile();
extern void A_FatRaise();
extern void A_FatAttack1();
extern void A_FatAttack2();
extern void A_FatAttack3();
extern void A_BossDeath();
extern void A_CPosAttack();
extern void A_CPosRefire();
extern void A_TroopAttack();
extern void A_SargAttack();
extern void A_HeadAttack();
extern void A_BruisAttack();
extern void A_SkullAttack();
extern void A_Metal();
extern void A_SpidRefire();
extern void A_BabyMetal();
extern void A_BspiAttack();
extern void A_Hoof();
extern void A_CyberAttack();
extern void A_PainAttack();
extern void A_PainDie();
extern void A_KeenDie();
extern void A_BrainPain();
extern void A_BrainScream();
extern void A_BrainDie();
extern void A_BrainAwake();
extern void A_BrainSpit();
extern void A_SpawnSound();
extern void A_SpawnFly();
extern void A_BrainExplode();

typedef struct
{
    actionf_t   cptr;           // actual pointer to the subroutine
    const char  *lookup;        // mnemonic lookup string to be specified in BEX
} deh_bexptr;

static const deh_bexptr deh_bexptrs[] =
{
  { A_Light0,        "A_Light0"        },
  { A_WeaponReady,   "A_WeaponReady"   },
  { A_Lower,         "A_Lower"         },
  { A_Raise,         "A_Raise"         },
  { A_Punch,         "A_Punch"         },
  { A_ReFire,        "A_ReFire"        },
  { A_FirePistol,    "A_FirePistol"    },
  { A_Light1,        "A_Light1"        },
  { A_FireShotgun,   "A_FireShotgun"   },
  { A_Light2,        "A_Light2"        },
  { A_FireShotgun2,  "A_FireShotgun2"  },
  { A_CheckReload,   "A_CheckReload"   },
  { A_OpenShotgun2,  "A_OpenShotgun2"  },
  { A_LoadShotgun2,  "A_LoadShotgun2"  },
  { A_CloseShotgun2, "A_CloseShotgun2" },
  { A_FireCGun,      "A_FireCGun"      },
  { A_GunFlash,      "A_GunFlash"      },
  { A_FireMissile,   "A_FireMissile"   },
  { A_Saw,           "A_Saw"           },
  { A_FirePlasma,    "A_FirePlasma"    },
  { A_BFGsound,      "A_BFGsound"      },
  { A_FireBFG,       "A_FireBFG"       },
  { A_BFGSpray,      "A_BFGSpray"      },
  { A_Explode,       "A_Explode"       },
  { A_Pain,          "A_Pain"          },
  { A_PlayerScream,  "A_PlayerScream"  },
  { A_Fall,          "A_Fall"          },
  { A_XScream,       "A_XScream"       },
  { A_Look,          "A_Look"          },
  { A_Chase,         "A_Chase"         },
  { A_FaceTarget,    "A_FaceTarget"    },
  { A_PosAttack,     "A_PosAttack"     },
  { A_Scream,        "A_Scream"        },
  { A_SPosAttack,    "A_SPosAttack"    },
  { A_VileChase,     "A_VileChase"     },
  { A_VileStart,     "A_VileStart"     },
  { A_VileTarget,    "A_VileTarget"    },
  { A_VileAttack,    "A_VileAttack"    },
  { A_StartFire,     "A_StartFire"     },
  { A_Fire,          "A_Fire"          },
  { A_FireCrackle,   "A_FireCrackle"   },
  { A_Tracer,        "A_Tracer"        },
  { A_SkelWhoosh,    "A_SkelWhoosh"    },
  { A_SkelFist,      "A_SkelFist"      },
  { A_SkelMissile,   "A_SkelMissile"   },
  { A_FatRaise,      "A_FatRaise"      },
  { A_FatAttack1,    "A_FatAttack1"    },
  { A_FatAttack2,    "A_FatAttack2"    },
  { A_FatAttack3,    "A_FatAttack3"    },
  { A_BossDeath,     "A_BossDeath"     },
  { A_CPosAttack,    "A_CPosAttack"    },
  { A_CPosRefire,    "A_CPosRefire"    },
  { A_TroopAttack,   "A_TroopAttack"   },
  { A_SargAttack,    "A_SargAttack"    },
  { A_HeadAttack,    "A_HeadAttack"    },
  { A_BruisAttack,   "A_BruisAttack"   },
  { A_SkullAttack,   "A_SkullAttack"   },
  { A_Metal,         "A_Metal"         },
  { A_SpidRefire,    "A_SpidRefire"    },
  { A_BabyMetal,     "A_BabyMetal"     },
  { A_BspiAttack,    "A_BspiAttack"    },
  { A_Hoof,          "A_Hoof"          },
  { A_CyberAttack,   "A_CyberAttack"   },
  { A_PainAttack,    "A_PainAttack"    },
  { A_PainDie,       "A_PainDie"       },
  { A_KeenDie,       "A_KeenDie"       },
  { A_BrainPain,     "A_BrainPain"     },
  { A_BrainScream,   "A_BrainScream"   },
  { A_BrainDie,      "A_BrainDie"      },
  { A_BrainAwake,    "A_BrainAwake"    },
  { A_BrainSpit,     "A_BrainSpit"     },
  { A_SpawnSound,    "A_SpawnSound"    },
  { A_SpawnFly,      "A_SpawnFly"      },
  { A_BrainExplode,  "A_BrainExplode"  },

  // This NULL entry must be the last in the list
  { NULL,            "A_NULL"          }        // Ty 05/16/98
};

// to hold startup code pointers from INFO.C
static actionf_t deh_codeptr[NUMSTATES];

boolean CheckPackageWADVersion(void)
{
    DEHFILE             infile, *filein = &infile;
    char                inbuffer[DEH_BUFFERMAX];
    unsigned int        i;

    for (i = 0; i < numlumps; ++i)
        if (!strncasecmp(lumpinfo[i].name, "VERSION", 7))
        {
            infile.size = W_LumpLength(i);
            infile.inp = infile.lump = W_CacheLumpNum(i, PU_STATIC);

            while (dehfgets(inbuffer, sizeof(inbuffer), filein))
            {
                lfstrip(inbuffer);

                if (!*inbuffer || *inbuffer == '#' || *inbuffer == ' ')
                    continue;   // Blank line or comment line

                if (!strcasecmp(inbuffer, PACKAGE_NAMEANDVERSIONSTRING))
                    return true;
            }
        }

    Z_ChangeTag(infile.lump, PU_CACHE);
    return false;
}

// ====================================================================
// ProcessDehFile
// Purpose: Read and process a DEH or BEX file
// Args:    filename    -- name of the DEH/BEX file
//          outfilename -- output file (DEHOUT.TXT), appended to here
// Returns: void
//
// killough 10/98:
// substantially modified to allow input from wad lumps instead of .deh files.
void ProcessDehFile(char *filename, char *outfilename, int lumpnum)
{
    static FILE *fileout = NULL;                // In case -dehout was used
    DEHFILE     infile, *filein = &infile;      // killough 10/98
    char        inbuffer[DEH_BUFFERMAX];        // Place to put the primary infostring

#if defined(_DEBUG)
    // Open output file if we're writing output
    if (outfilename && *outfilename && !fileout)
    {
        static boolean  firstfile = true;       // to allow append to output log

        if (!strcasecmp(outfilename, "-"))
            fileout = stdout;
        else if (!(fileout = fopen(outfilename, firstfile ? "wt" : "at")))
            fileout = stdout;
        firstfile = false;
    }
#endif

    addtocount = false;

    // killough 10/98: allow DEH files to come from wad lumps
    if (filename)
    {
        if (!(infile.f = fopen(filename, "rt")))
            return;     // should be checked up front anyway
        infile.lump = NULL;
        C_Output("Parsed DeHackEd%s file %s.",
            (M_StringEndsWith(uppercase(filename), "BEX") ? " with BOOM extensions" : ""),
            uppercase(filename));
    }
    else        // DEH file comes from lump indicated by third argument
    {
        infile.size = W_LumpLength(lumpnum);
        infile.inp = infile.lump = W_CacheLumpNum(lumpnum, PU_STATIC);
        filename = lumpinfo[lumpnum].wad_file->path;
        C_Output("Parsed DEHACKED lump from %s file %s.",
            (W_WadType(filename) == IWAD ? "IWAD" : "PWAD"), uppercase(filename));
    }

    {
        static int      i;   // killough 10/98: only run once, by keeping index static

        for (; i < NUMSTATES; i++)  // remember what they start as for deh xref
            deh_codeptr[i] = states[i].action;
    }

    // loop until end of file
    while (dehfgets(inbuffer, sizeof(inbuffer), filein))
    {
        boolean                 match;
        unsigned int            i;
        static unsigned int     last_i = DEH_BLOCKMAX - 1;
        static long             filepos = 0;

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

            char        *nextfile;
            boolean     oldnotext = includenotext;      // killough 10/98

            // killough 10/98: exclude if inside wads (only to discourage
            // the practice, since the code could otherwise handle it)
            if (infile.lump)
            {
                C_Warning("No files may be included from wads: \"%s\".", inbuffer);
                continue;
            }

            // check for no-text directive, used when including a DEH
            // file but using the BEX format to handle strings
            if (!strncasecmp(nextfile = ptr_lstrip(inbuffer + 7), "NOTEXT", 6))
            {
                includenotext = true;
                nextfile = ptr_lstrip(nextfile + 6);
            }

            if (devparm)
                C_Output("Branching to include file %s...", nextfile);

            // killough 10/98:
            // Second argument must be NULL to prevent closing fileout too soon

            ProcessDehFile(nextfile, NULL, 0); // do the included file

            includenotext = oldnotext;
            if (devparm)
                C_Output("...continuing with %s", filename);
            continue;
        }

        for (match = 0, i = 0; i < DEH_BLOCKMAX; i++)
            if (!strncasecmp(inbuffer, deh_blocks[i].key, strlen(deh_blocks[i].key)))
            {
                if (i < DEH_BLOCKMAX - 1)
                    match = 1;
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
            C_Output("Processing function [%d] for %s", i, deh_blocks[i].key);
        deh_blocks[i].fptr(filein, fileout, inbuffer);  // call function

        if (!filein->lump)                              // back up line start
            filepos = ftell(filein->f);
    }

    if (infile.lump)
        Z_ChangeTag(infile.lump, PU_CACHE);     // Mark purgable
    else
        fclose(infile.f);                       // Close real file

    if (outfilename)                            // killough 10/98: only at top recursion level
    {
        if (fileout && fileout != stdout)       // haleyjd: don't fclose(NULL)
            fclose(fileout);
        fileout = NULL;
    }

    if (addtocount)
        ++dehcount;
}

// ====================================================================
// deh_procBexCodePointers
// Purpose: Handle [CODEPTR] block, BOOM Extension
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procBexCodePointers(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    int         indexnum;
    char        mnemonic[DEH_MAXKEYLEN];        // to hold the codepointer mnemonic

    // Ty 05/16/98 - initialize it to something, dummy!
    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);

    // for this one, we just read 'em until we hit a blank line
    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        int     i = 0;                          // looper
        boolean found = false;                  // know if we found this one during lookup or not

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;      // killough 11/98: really exit on blank line

        // killough 8/98: allow hex numbers in input:
        if ((3 != sscanf(inbuffer, "%31s %10i = %31s", key, &indexnum, mnemonic))
            || strcasecmp(key, "FRAME"))        // NOTE: different format from normal
        {
            C_Warning("Invalid BEX codepointer line - must start with 'FRAME': \"%s\".", inbuffer);
            return;     // early return
        }

        if (devparm)
            C_Output("Processing pointer at index %d: %s", indexnum, mnemonic);
        if (indexnum < 0 || indexnum >= NUMSTATES)
        {
            C_Warning("Bad pointer number %d of %d.", indexnum, NUMSTATES);
            return;     // killough 10/98: fix SegViol
        }
        strcpy(key, "A_");      // reusing the key area to prefix the mnemonic
        strcat(key, ptr_lstrip(mnemonic));

        while (!found && deh_bexptrs[i].lookup != NULL)
        {
            if (!strcasecmp(key, deh_bexptrs[i].lookup))
            {   // Ty 06/01/98  - add  to states[].action for new djgcc version
                states[indexnum].action = deh_bexptrs[i].cptr;  // assign
                if (devparm)
                    C_Output(" - applied %p from codeptr[%d] to states[%d]",
                        (void *)deh_bexptrs[i].cptr.acp1, i, indexnum);
                found = true;
            }
            ++i;
        }

        if (!found)
            C_Warning("Invalid frame pointer mnemonic \"%s\" at %d.", mnemonic, indexnum);
    }
    return;
}

// ====================================================================
// deh_procThing
// Purpose: Handle DEH Thing block
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
// Ty 8/27/98 - revised to also allow mnemonics for
// bit masks for monster attributes
//
void deh_procThing(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;          // All deh values are ints or longs
    int         indexnum;
    int         ix;
    int         *pix;           // Ptr to int, since all Thing structure entries are ints
    char        *strval;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);
    if (devparm)
        C_Output("Thing line: \"%s\"", inbuffer);

    // killough 8/98: allow hex numbers in input:
    ix = sscanf(inbuffer, "%31s %10i", key, &indexnum);
    if (devparm)
        C_Output("count = %d, Thing %d", ix, indexnum);

    // Note that the mobjinfo[] array is base zero, but object numbers
    // in the dehacked file start with one. Grumble.
    --indexnum;

    // now process the stuff
    // Note that for Things we can look up the key and use its offset
    // in the array of key strings as an int offset in the structure

    // get a line until a blank or end of file -- it's not
    // blank now because it has our incoming key in it
    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
        int bGetData;

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);      // toss the end of line

        // killough 11/98: really bail out on blank lines (break != continue)
        if (!*inbuffer)
            break;              // bail out with blank line between sections

        // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
        bGetData = deh_GetData(inbuffer, key, &value, &strval, fpout);
        if (!bGetData)
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }
        for (ix = 0; ix < DEH_MOBJINFOMAX; ix++)
        {
            if (!strcasecmp(key, deh_mobjinfo[ix]))     // killough 8/98
            {
                if (!strcasecmp(key, "Bits") && !value) // killough 10/98
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
                            if (!strcasecmp(strval, deh_mobjflags[iy].name))
                            {
                                if (devparm)
                                    C_Output("ORed value 0x%08lx %s",
                                        deh_mobjflags[iy].value, strval);
                                value |= deh_mobjflags[iy].value;
                                break;
                            }
                        if (iy >= DEH_MOBJFLAGMAX && fpout)
                            C_Warning("Could not find bit mnemonic \"%s\".", strval);
                    }

                    // Don't worry about conversion -- simply print values
                    if (devparm)
                        C_Output("Bits = 0x%08lX = %ld", value, value);
                }
                pix = (int *)&mobjinfo[indexnum];
                pix[ix] = (int)value;
                if (!BTSX)
                {
                    mobjinfo[indexnum].flags2 = 0;
                    mobjinfo[indexnum].blood = 0;
                }
                if (devparm)
                    C_Output("Assigned %d to %s(%d) at index %d", (int)value, key, indexnum, ix);
            }
        }
    }
    return;
}

// ====================================================================
// deh_procFrame
// Purpose: Handle DEH Frame block
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procFrame(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;  // All deh values are ints or longs
    int         indexnum;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);
    if (devparm)
        C_Output("Processing Frame at index %d: %s", indexnum, key);
    if (indexnum < 0 || indexnum >= NUMSTATES)
        C_Warning("Bad frame number %d of %d.", indexnum, NUMSTATES);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;                                              // killough 11/98
        if (!deh_GetData(inbuffer, key, &value, NULL, fpout))   // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }
        if (!strcasecmp(key, deh_state[0]))                     // Sprite number
        {
            if (devparm)
                C_Output(" - sprite = %ld", value);
            states[indexnum].sprite = (spritenum_t)value;
            dehacked = !BTSX;
        }
        else if (!strcasecmp(key, deh_state[1]))                // Sprite subnumber
        {
            if (devparm)
                C_Output(" - frame = %ld", value);
            states[indexnum].frame = value;                     // long
            dehacked = !BTSX;
        }
        else if (!strcasecmp(key, deh_state[2]))                // Duration
        {
            if (devparm)
                C_Output(" - tics = %ld", value);
            states[indexnum].tics = value;                      // long
            dehacked = !BTSX;
        }
        else if (!strcasecmp(key, deh_state[3]))                // Next frame
        {
            if (devparm)
                C_Output(" - nextstate = %ld", value);
            states[indexnum].nextstate = value;
            dehacked = !BTSX;
        }
        else if (!strcasecmp(key, deh_state[4]))                // Codep frame (not set in Frame deh block)
            C_Warning("Codep frame should not be set in Frame section.");
        else if (!strcasecmp(key, deh_state[5]))                // Unknown 1
        {
            if (devparm)
                C_Output(" - misc1 = %ld", value);
            states[indexnum].misc1 = value;                     // long
            dehacked = !BTSX;
        }
        else if (!strcasecmp(key, deh_state[6]))                // Unknown 2
        {
            if (devparm)
                C_Output(" - misc2 = %ld", value);
            states[indexnum].misc2 = value;                     // long
            dehacked = !BTSX;
        }
        else
            C_Warning("Invalid frame string index for \"%s\".", key);
    }
    return;
}

// ====================================================================
// deh_procPointer
// Purpose: Handle DEH Code pointer block, can use BEX [CODEPTR] instead
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procPointer(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;  // All deh values are ints or longs
    int         indexnum;
    int         i;      // looper

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);
    // NOTE: different format from normal

    // killough 8/98: allow hex numbers in input, fix error case:
    if (sscanf(inbuffer, "%*s %*i (%31s %10i)", key, &indexnum) != 2)
    {
        C_Warning("Bad data pair in \"%s\".", inbuffer);
        return;
    }

    if (devparm)
        C_Output("Processing Pointer at index %d: %s", indexnum, key);
    if (indexnum < 0 || indexnum >= NUMSTATES)
    {
        C_Warning("Bad pointer number %d of %d.", indexnum, NUMSTATES);
        return;
    }

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;      // killough 11/98
        if (!deh_GetData(inbuffer, key, &value, NULL, fpout))   // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        if (value < 0 || value >= NUMSTATES)
        {
            C_Warning("Bad pointer number %ld of %d.", value, NUMSTATES);
            return;
        }

        if (!strcasecmp(key, deh_state[4]))     // Codep frame (not set in Frame deh block)
        {
            states[indexnum].action = deh_codeptr[value];
            if (devparm)
                C_Output(" - applied %p from codeptr[%ld] to states[%d]",
                    (void *)deh_codeptr[value].acp1, value, indexnum);

            // Write BEX-oriented line to match:
            for (i = 0; i < sizeof(deh_bexptrs) / sizeof(*deh_bexptrs); i++)
            {
                if (!memcmp(&deh_bexptrs[i].cptr, &deh_codeptr[value], sizeof(actionf_t)))
                {
                    if (devparm)
                        C_Output("BEX [CODEPTR] -> FRAME %d = %s",
                            indexnum, &deh_bexptrs[i].lookup[2]);
                    break;
                }
            }
        }
        else
            C_Warning("Invalid frame pointer index for \"%s\" at %ld, xref %p.",
                key, value, (void *)deh_codeptr[value].acp1);
    }
    return;
}

// ====================================================================
// deh_procSounds
// Purpose: Handle DEH Sounds block
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procSounds(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;  // All deh values are ints or longs
    int         indexnum;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);
    if (devparm)
        C_Output("Processing Sounds at index %d: %s", indexnum, key);
    if (indexnum < 0 || indexnum >= NUMSFX)
        C_Warning("Bad sound number %d of %d.", indexnum, NUMSFX);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;      // killough 11/98
        if (!deh_GetData(inbuffer, key, &value, NULL, fpout))   // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\"\n", inbuffer);
            continue;
        }
        if (!strcasecmp(key, deh_sfxinfo[0]))           // Offset
            /* nop */;                                  // we don't know what this is, I don't think
        else if (!strcasecmp(key, deh_sfxinfo[1]))      // Zero/One
            S_sfx[indexnum].singularity = value;
        else if (!strcasecmp(key, deh_sfxinfo[2]))      // Value
            S_sfx[indexnum].priority = value;
        else if (!strcasecmp(key, deh_sfxinfo[3]))      // Zero 1
            S_sfx[indexnum].link = (sfxinfo_t *)value;
        else if (!strcasecmp(key, deh_sfxinfo[4]))      // Zero 2
            S_sfx[indexnum].pitch = value;
        else if (!strcasecmp(key, deh_sfxinfo[5]))      // Zero 3
            S_sfx[indexnum].volume = value;
        else if (!strcasecmp(key, deh_sfxinfo[6]))      // Zero 4
            S_sfx[indexnum].data = (void *)value;       // killough 5/3/98: changed cast
        else if (!strcasecmp(key, deh_sfxinfo[7]))      // Neg. One 1
            S_sfx[indexnum].usefulness = value;
        else if (!strcasecmp(key, deh_sfxinfo[8]))      // Neg. One 2
            S_sfx[indexnum].lumpnum = value;
        else if (devparm)
            C_Output("Invalid sound string index for \"%s\"", key);
    }
    return;
}

// ====================================================================
// deh_procAmmo
// Purpose: Handle DEH Ammo block
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procAmmo(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;  // All deh values are ints or longs
    int         indexnum;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);
    if (devparm)
        C_Output("Processing Ammo at index %d: %s", indexnum, key);
    if (indexnum < 0 || indexnum >= NUMAMMO)
        C_Warning("Bad ammo number %d of %d.", indexnum, NUMAMMO);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;                                              // killough 11/98
        if (!deh_GetData(inbuffer, key, &value, NULL, fpout))   // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }
        if (!strcasecmp(key, deh_ammo[0]))                      // Max ammo
            maxammo[indexnum] = value;
        else if (!strcasecmp(key, deh_ammo[1]))                 // Per ammo
            clipammo[indexnum] = value;
        else
            C_Warning("Invalid ammo string index for \"%s\".", key);
    }
    return;
}

// ====================================================================
// deh_procWeapon
// Purpose: Handle DEH Weapon block
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procWeapon(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;      // All deh values are ints or longs
    int         indexnum;

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);
    if (devparm)
        C_Output("Processing Weapon at index %d: %s", indexnum, key);
    if (indexnum < 0 || indexnum >= NUMWEAPONS)
        C_Warning("Bad weapon number %d of %d.", indexnum, NUMAMMO);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;                                              // killough 11/98
        if (!deh_GetData(inbuffer, key, &value, NULL, fpout))   // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }
        if (!strcasecmp(key, deh_weapon[0]))                    // Ammo type
            weaponinfo[indexnum].ammo = value;
        else if (!strcasecmp(key, deh_weapon[1]))               // Deselect frame
            weaponinfo[indexnum].upstate = value;
        else if (!strcasecmp(key, deh_weapon[2]))               // Select frame
            weaponinfo[indexnum].downstate = value;
        else if (!strcasecmp(key, deh_weapon[3]))               // Bobbing frame
            weaponinfo[indexnum].readystate = value;
        else if (!strcasecmp(key, deh_weapon[4]))               // Shooting frame
            weaponinfo[indexnum].atkstate = value;
        else if (!strcasecmp(key, deh_weapon[5]))               // Firing frame
            weaponinfo[indexnum].flashstate = value;
        else
            C_Warning("Invalid weapon string index for \"%s\".", key);
    }
    return;
}

// ====================================================================
// deh_procSprite
// Purpose: Dummy - we do not support the DEH Sprite block
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procSprite(DEHFILE *fpin, FILE* fpout, char *line) // Not supported
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    int         indexnum;

    // Too little is known about what this is supposed to do, and
    // there are better ways of handling sprite renaming.  Not supported.
    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);
    C_Warning("Ignoring sprite offset change at index %d: \"%s\".", indexnum, key);
    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;      // killough 11/98
        // ignore line
        if (devparm)
            C_Output("- %s", inbuffer);
    }
    return;
}

extern int pars[5][10];
extern int cpars[33];

#if !defined(WIN32)
char *strlwr(char *str)
{
    size_t      i;
    size_t      len = strlen(str);

    for (i = 0; i < len; ++i)
        str[i] = tolower((unsigned char)str[i]);

    return str;
}
#endif

// ====================================================================
// deh_procPars
// Purpose: Handle BEX extension for PAR times
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procPars(DEHFILE *fpin, FILE* fpout, char *line) // extension
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    int         indexnum;
    int         episode, level, partime, oldpar;

    // new item, par times
    // usage: After [PARS] Par 0 section identifier, use one or more of these
    // lines:
    //  par 3 5 120
    //  par 14 230
    // The first would make the par for E3M5 be 120 seconds, and the
    // second one makes the par for MAP14 be 230 seconds.  The number
    // of parameters on the line determines which group of par values
    // is being changed.  Error checking is done based on current fixed
    // array sizes of[4][10] and [32]
    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);

    // killough 8/98: allow hex numbers in input:
    sscanf(inbuffer, "%31s %10i", key, &indexnum);
    if (devparm)
        C_Output("Processing Par value at index %d: %s", indexnum, key);

    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        if (*inbuffer == '#')
            continue;                           // skip comment lines
        lfstrip(strlwr(inbuffer));              // lowercase it
        if (!*inbuffer)
            break;                              // killough 11/98
        if (3 != sscanf(inbuffer, "par %10i %10i %10i", &episode, &level, &partime))
        { // not 3
            if (2 != sscanf(inbuffer, "par %10i %10i", &level, &partime))
                // not 2
                C_Warning("Invalid par time setting string \"%s\".", inbuffer);
            else
            { // is 2
                // Ty 07/11/98 - wrong range check, not zero-based
                if (level < 1 || level > 32)    // base 0 array (but 1-based parm)
                    C_Warning("Invalid MAPxy value MAP%d.", level);
                else
                {
                    oldpar = cpars[level - 1];
                    if (devparm)
                        C_Output("Changed par time for MAP%02d from %d to %d",
                            level, oldpar, partime);
                    cpars[level - 1] = partime;
                    deh_pars = true;
                }
            }
        }
        else
        { // is 3
            // note that though it's a [4][10] array, the "left" and "top" aren't used,
            // effectively making it a base 1 array.
            // Ty 07/11/98 - level was being checked against max 3 - dumb error
            // Note that episode 4 does not have par times per original design
            // in Ultimate DOOM so that is not supported here.
            if (episode < 1 || episode > 3 || level < 1 || level > 9)
                C_Warning("Invalid ExMx values E%dM%d.", episode, level);
            else
            {
                oldpar = pars[episode][level];
                pars[episode][level] = partime;
                if (devparm)
                    C_Output("Changed par time for E%dM%d from %d to %d",
                        episode, level, oldpar, partime);
                deh_pars = true;
            }
        }
    }
    return;
}

// ====================================================================
// deh_procCheat
// Purpose: Handle DEH Cheat block
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procCheat(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;          // All deh values are ints or longs
    char        ch = 0;         // CPhipps - `writable' null string to initialise...
    char        *strval = &ch;  // pointer to the value area
    int         iy;             // array index
    char        *p;             // utility pointer

    if (devparm)
        C_Output("Processing Cheat: %s", line);

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);
    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        boolean     success = false;

        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;              // killough 11/98
        if (!deh_GetData(inbuffer, key, &value, &strval, fpout))        // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }

        // Otherwise we got a (perhaps valid) cheat name
        if (!strcasecmp(key, deh_cheat[0]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_mus.sequence = strdup(p);
            cheat_mus_xy.sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[1]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_choppers.sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[2]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_god.sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[3]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_ammo.sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[4]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_ammonokey.sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[5]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_noclip.sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[6]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_commercial_noclip.sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[7]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_powerup[0].sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[8]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_powerup[1].sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[9]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_powerup[2].sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[10]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_powerup[3].sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[11]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_powerup[4].sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[12]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_powerup[5].sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[13]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_powerup[6].sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[14]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_clev.sequence = strdup(p);
            cheat_clev_xy.sequence = strdup(p);
            success = true;
        }
        else if (!strcasecmp(key, deh_cheat[15]))
        {
            for (iy = 0; strval[iy]; iy++)
                strval[iy] = (strval[iy] == (char)0xff ? '\0' : strval[iy]);
            p = strval;
            while (*p == ' ')
                ++p;
            cheat_mypos.sequence = strdup(p);
            success = true;
        }

        if (success && devparm)
            C_Output("Assigned new cheat '%s' to cheat '%s' at index %d",
                p, cheat_mus.sequence, iy);

        if (devparm)
            C_Output("- %s", inbuffer);
    }
    return;
}

// ====================================================================
// deh_procMisc
// Purpose: Handle DEH Misc block
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procMisc(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;  // All deh values are ints or longs

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);
    while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        lfstrip(inbuffer);
        if (!*inbuffer)
            break;                                              // killough 11/98
        if (!deh_GetData(inbuffer, key, &value, NULL, fpout))   // returns TRUE if ok
        {
            C_Warning("Bad data pair in \"%s\".", inbuffer);
            continue;
        }
        // Otherwise it's ok
        if (devparm)
            C_Output("Processing Misc item '%s'", key);

        if (!strcasecmp(key, deh_misc[0]))                      // Initial Health
            initial_health = value;
        else if (!strcasecmp(key, deh_misc[1]))                 // Initial Bullets
            initial_bullets = value;
        else if (!strcasecmp(key, deh_misc[2]))                 // Max Health
            maxhealth = value;
        else if (!strcasecmp(key, deh_misc[3]))                 // Max Armor
            max_armor = value;
        else if (!strcasecmp(key, deh_misc[4]))                 // Green Armor Class
            green_armor_class = value;
        else if (!strcasecmp(key, deh_misc[5]))                 // Blue Armor Class
            blue_armor_class = value;
        else if (!strcasecmp(key, deh_misc[6]))                 // Max Soulsphere
            max_soul = value;
        else if (!strcasecmp(key, deh_misc[7]))                 // Soulsphere Health
            soul_health = value;
        else if (!strcasecmp(key, deh_misc[8]))                 // Megasphere Health
            mega_health = value;
        else if (!strcasecmp(key, deh_misc[9]))                 // God Mode Health
            god_health = value;
        else if (!strcasecmp(key, deh_misc[10]))                // IDFA Armor
            idfa_armor = value;
        else if (!strcasecmp(key, deh_misc[11]))                // IDFA Armor Class
            idfa_armor_class = value;
        else if (!strcasecmp(key, deh_misc[12]))                // IDKFA Armor
            idkfa_armor = value;
        else if (!strcasecmp(key, deh_misc[13]))                // IDKFA Armor Class
            idkfa_armor_class = value;
        else if (!strcasecmp(key, deh_misc[14]))                // BFG Cells/Shot
            bfgcells = value;
        else if (!strcasecmp(key, deh_misc[15]))                // Monsters Infight
            species_infighting = value;
        else
            C_Warning("Invalid misc item string index for \"%s\".", key);
    }
    return;
}

// ====================================================================
// deh_procText
// Purpose: Handle DEH Text block
// Notes:   We look things up in the current information and if found
//          we replace it.  At the same time we write the new and
//          improved BEX syntax to the log file for future use.
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procText(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX * 2];    // can't use line -- double size buffer too.
    int         i;                              // loop variable
    int         fromlen, tolen;                 // as specified on the text block line
    int         usedlen;                        // shorter of fromlen and tolen if not matched
    boolean     found = false;                  // to allow early exit once found
    char        *line2 = NULL;                  // duplicate line for rerouting

    // Ty 04/11/98 - Included file may have NOTEXT skip flag set
    if (includenotext)                          // flag to skip included deh-style text
    {
        C_Output("Skipped text block because of NOTEXT directive.");
        strcpy(inbuffer, line);
        while (!dehfeof(fpin) && *inbuffer && *inbuffer != ' ')
            dehfgets(inbuffer, sizeof(inbuffer), fpin); // skip block
        // Ty 05/17/98 - don't care if this fails
        return;                                 // ************** Early return
    }

    // killough 8/98: allow hex numbers in input:
    sscanf(line, "%31s %10i %10i", key, &fromlen, &tolen);
    if (devparm)
        C_Output("Processing Text (key = %s, from = %d, to = %d)", key, fromlen, tolen);

    // killough 10/98: fix incorrect usage of feof
    {
        int     c, totlen = 0;

        while (totlen < fromlen + tolen && (c = dehfgetc(fpin)) != EOF)
            if (c != '\r')
                inbuffer[totlen++] = c;
        inbuffer[totlen] = '\0';
    }

    // if the from and to are 4, this may be a sprite rename.  Check it
    // against the array and process it as such if it matches.  Remember
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
                    C_Output("Changing name of sprite at index %d from %s to %*s",
                        i, sprnames[i], tolen, &inbuffer[fromlen]);
                // Ty 03/18/98 - not using strdup because length is fixed

                // killough 10/98: but it's an array of pointers, so we must
                // use strdup unless we redeclare sprnames and change all else
                sprnames[i] = strdup(sprnames[i]);

                M_StringCopy(sprnames[i], &inbuffer[fromlen], tolen);
                found = true;
                break;          // only one will match--quit early
            }
            ++i;                // next array element
        }
    }
    else if (fromlen < 7 && tolen < 7)   // lengths of music and sfx are 6 or shorter
    {
        usedlen = (fromlen < tolen ? fromlen : tolen);
        if (fromlen != tolen)
            C_Warning("Mismatched lengths from %d to %d. Using %d.", fromlen, tolen, usedlen);

        // Try sound effects entries - see sounds.c
        for (i = 1; i < NUMSFX; i++)
        {
            // avoid short prefix erroneous match
            if (strlen(S_sfx[i].name) != fromlen)
                continue;
            if (!strncasecmp(S_sfx[i].name, inbuffer, fromlen))
            {
                if (devparm)
                    C_Output("Changing name of sfx from %s to %*s",
                        S_sfx[i].name, usedlen, &inbuffer[fromlen]);

                S_sfx[i].name = strdup(&inbuffer[fromlen]);
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
                        C_Output("Changing name of music from %s to %*s",
                            S_music[i].name, usedlen, &inbuffer[fromlen]);

                    S_music[i].name = strdup(&inbuffer[fromlen]);
                    found = true;
                    break;      // only one matches, quit early
                }
            }
        }                       // end !found test
    }

    if (!found) // Nothing we want to handle here -- see if strings can deal with it.
    {
        if (devparm)
            C_Output("Checking text area through strings for \"%.12s%s\" from = %d to = %d",
                inbuffer, (strlen(inbuffer) > 12 ? "..." : ""), fromlen, tolen);
        if ((size_t)fromlen <= strlen(inbuffer))
        {
            line2 = strdup(&inbuffer[fromlen]);
            inbuffer[fromlen] = '\0';
        }

        deh_procStringSub(NULL, inbuffer, line2, fpout);
    }
    free(line2);        // may be NULL, ignored by free()
    return;
}

void deh_procError(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        inbuffer[DEH_BUFFERMAX];

    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);
    C_Warning("Ignoring \"%s\".", inbuffer);
    return;
}

// ====================================================================
// deh_procStrings
// Purpose: Handle BEX [STRINGS] extension
// Args:    fpin  -- input file stream
//          fpout -- output file stream (DEHOUT.TXT)
//          line  -- current line in file to process
// Returns: void
//
void deh_procStrings(DEHFILE *fpin, FILE* fpout, char *line)
{
    char        key[DEH_MAXKEYLEN];
    char        inbuffer[DEH_BUFFERMAX];
    long        value;                  // All deh values are ints or longs
    char        *strval;                // holds the string value of the line
    static int  maxstrlen = 128;        // maximum string length, bumped 128 at a time as needed
                                        // holds the final result of the string after concatenation
    static char *holdstring = NULL;
    boolean     found = false;          // looking for string continuation

    if (devparm)
        C_Output("Processing extended string substitution");

    if (!holdstring)
        holdstring = malloc(maxstrlen * sizeof(*holdstring));

    *holdstring = '\0';                 // empty string to start with
    M_StringCopy(inbuffer, line, DEH_BUFFERMAX);
    // Ty 04/24/98 - have to allow inbuffer to start with a blank for
    // the continuations of C1TEXT etc.
    while (!dehfeof(fpin) && *inbuffer)
    {
        if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
            break;
        if (*inbuffer == '#')
            continue;                   // skip comment lines
        lfstrip(inbuffer);
        if (!*inbuffer && !*holdstring)
            break;                      // killough 11/98
        if (!*holdstring)               // first one--get the key
        {
            if (!deh_GetData(inbuffer, key, &value, &strval, fpout))    // returns TRUE if ok
            {
                C_Warning("Bad data pair in \"%s\".", inbuffer);
                continue;
            }
        }
        while (strlen(holdstring) + strlen(inbuffer) > (unsigned int)maxstrlen)
        {
            // killough 11/98: allocate enough the first time
            maxstrlen += strlen(holdstring) + strlen(inbuffer) - maxstrlen;
            if (devparm)
                C_Output("* increased buffer from to %d for buffer size %d",
                    maxstrlen, (int)strlen(inbuffer));
            holdstring = realloc(holdstring, maxstrlen * sizeof(*holdstring));
        }
        // concatenate the whole buffer if continuation or the value iffirst
        strcat(holdstring, ptr_lstrip(((*holdstring) ? inbuffer : strval)));
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
            found = deh_procStringSub(key, NULL, holdstring, fpout);    // supply key and not search string

            if (!found)
                C_Warning("Invalid string key \"%s\". Substitution skipped.", key);

            *holdstring = '\0';  // empty string for the next one
        }
    }
    return;
}

// ====================================================================
// deh_procStringSub
// Purpose: Common string parsing and handling routine for DEH and BEX
// Args:    key       -- place to put the mnemonic for the string if found
//          lookfor   -- original value string to look for
//          newstring -- string to put in its place if found
//          fpout     -- file stream pointer for log file (DEHOUT.TXT)
// Returns: boolean: True if string found, false if not
//
boolean deh_procStringSub(char *key, char *lookfor, char *newstring, FILE *fpout)
{
    boolean     found = false;  // loop exit flag
    int         i;              // looper

    for (i = 0; i < deh_numstrlookup; i++)
    {
        found = (lookfor ? !strcasecmp(*deh_strlookup[i].ppstr, lookfor) :
            !strcasecmp(deh_strlookup[i].lookup, key));

        if (found)
        {
            char        *t;

            if (deh_strlookup[i].assigned)
                break;

            *deh_strlookup[i].ppstr = t = strdup(newstring);    // orphan originalstring
            found = true;

            // Handle embedded \n's in the incoming string, convert to 0x0a's
            {
                char    *s;

                for (s = *deh_strlookup[i].ppstr; *s; ++s, ++t)
                {
                    if (*s == '\\' && (s[1] == 'n' || s[1] == 'N'))     // found one
                    {
                        ++s;
                        *t = '\n';      // skip one extra for second character
                    }
                    else
                        *t = *s;
                }
                *t = '\0';              // cap off the target string
            }

            if (key)
                if (devparm)
                    C_Output("Assigned key %s => \"%s\"", key, newstring);

            if (!key)
                if (devparm)
                    C_Output("Assigned \"%.12s%s\" to \"%.12s%s\" at key %s",
                        lookfor, (strlen(lookfor) > 12 ? "..." : ""),
                        newstring, (strlen(newstring) > 12 ? "..." : ""),
                        deh_strlookup[i].lookup);

            if (!key)   // must have passed an old style string so show BEX
                if (devparm)
                {
                    C_Output("*BEX FORMAT:");
                    C_Output("%s = %s", deh_strlookup[i].lookup, dehReformatStr(newstring));
                    C_Output("*END BEX");
                }

            deh_strlookup[i].assigned = true;

            if (M_StrCaseStr(deh_strlookup[i].lookup, "HUSTR"))
                addtocount = true;

            break;
        }
    }
    if (!found)
        C_Warning("Couldn't find \"%s\".", (key ? key : lookfor));

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
char *dehReformatStr(char *string)
{
    static char buff[DEH_BUFFERMAX];    // only processing the changed string,
    //  don't need double buffer
    char        *s, *t;

    s = string; // source
    t = buff;   // target
    // let's play...

    while (*s)
    {
        if (*s == '\n')
            ++s, *t++ = '\\', *t++ = 'n', *t++ = '\\', *t++ = '\n';
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
void lfstrip(char *s)    // strip the \r and/or \n off of a line
{
    char        *p = s + strlen(s);

    while (p > s && (*--p=='\r' || *p=='\n'))
        *p = 0;
}

// ====================================================================
// rstrip
// Purpose: Strips trailing blanks off a string
// Args:    s -- the string to work on
// Returns: void -- the string is modified in place
//
void rstrip(char *s)    // strip trailing whitespace
{
    char        *p = s + strlen(s);       // killough 4/4/98: same here

    while (p > s && isspace(*--p))      // break on first non-whitespace
        *p='\0';
}

// ====================================================================
// ptr_lstrip
// Purpose: Points past leading whitespace in a string
// Args:    s -- the string to work on
// Returns: char * pointing to the first nonblank character in the
//          string.  The original string is not changed.
//
char *ptr_lstrip(char *p)       // point past leading whitespace
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
//                    value comes from.  Pass NULL to not use this.
//          fpout  -- stream pointer to output log (DEHOUT.TXT)
// Notes:   Expects a key phrase, optional space, equal sign,
//          optional space and a value, mostly an int but treated
//          as a long just in case.  The passed pointer to hold
//          the key must be DEH_MAXKEYLEN in size.
//
boolean deh_GetData(char *s, char *k, long *l, char **strval, FILE *fpout)
{
    char        *t;                     // current char
    long        val;                    // to hold value of pair
    char        buffer[DEH_MAXKEYLEN];  // to hold key in progress
    boolean     okrc = true;            // assume good unless we have problems
    int         i;                      // iterator

    *buffer = '\0';
    val = 0;                            // defaults in case not otherwise set
    for (i = 0, t = s; *t && i < DEH_MAXKEYLEN; t++, i++)
    {
        if (*t == '=')
            break;
        buffer[i] = *t;                 // copy it
    }
    buffer[--i] = '\0';                 // terminate the key before the '='
    if (!*t)                            // end of string with no equal sign
    {
        okrc = false;
    }
    else
    {
        if (!*++t)
        {
            val = 0;                    // in case "thiskey =" with no value
            okrc = false;
        }
        // we've incremented t
        val = strtol(t, NULL, 0);       // killough 8/9/98: allow hex or octal input
    }

    // go put the results in the passed pointers
    *l = val;                           // may be a faked zero

    // if spaces between key and equal sign, strip them
    strcpy(k, ptr_lstrip(buffer));      // could be a zero-length string

    if (strval != NULL)                 // pass NULL if you don't want this back
        *strval = t;                    // pointer, has to be somewhere in s,
    // even if pointing at the zero byte.

    return okrc;
}
