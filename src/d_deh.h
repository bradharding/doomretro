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

#if !defined(__D_DEH_H__)
#define __D_DEH_H__

#include "doomtype.h"

enum
{
    p_QUITMSG = 5, p_GOTARMOR = 33, p_GOTMEGA, p_GOTHTHBONUS, p_GOTARMBONUS, p_GOTSTIM, p_GOTMEDINEED,
    p_GOTMEDIKIT, p_GOTSUPER, p_GOTBLUECARD, p_GOTYELWCARD, p_GOTREDCARD, p_GOTBLUESKUL, p_GOTYELWSKUL,
    p_GOTREDSKUL, p_GOTREDSKULL, p_GOTINVUL, p_GOTBERSERK, p_GOTINVIS, p_GOTSUIT, p_GOTMAP, p_GOTVISOR,
    p_GOTCLIP, p_GOTCLIPX2, p_GOTHALFCLIP, p_GOTCLIPBOX, p_GOTROCKET, p_GOTROCKETX2, p_GOTROCKBOX, p_GOTCELL,
    p_GOTCELLX2, p_GOTCELLBOX, p_GOTSHELLS, p_GOTSHELLSX2, p_GOTSHELLBOX, p_GOTBACKPACK, p_GOTBFG9000,
    p_GOTCHAINGUN, p_GOTCHAINSAW, p_GOTLAUNCHER, p_GOTMSPHERE, p_GOTPLASMA, p_GOTSHOTGUN, p_GOTSHOTGUN2,
    p_PD_BLUEO, p_PD_REDO, p_PD_YELLOWO, p_PD_BLUEK, p_PD_REDK, p_PD_YELLOWK
};

typedef struct
{
    char    **ppstr;        // doubly indirect pointer to string
    char    *lookup;        // pointer to lookup string name
    int     assigned;       // [BH] counter indicating string has been assigned
} deh_strs;

extern dboolean dehacked;
extern deh_strs deh_strlookup[];

extern char     *s_D_DEVSTR;

extern char     *s_PRESSKEY;
extern char     *s_PRESSYN;
extern char     *s_PRESSA;
extern char     *s_QUITMSG;
extern char     *s_QLPROMPT;
extern char     *s_DELPROMPT;
extern char     *s_NIGHTMARE;
extern char     *s_SWSTRING;
extern char     *s_MSGOFF;
extern char     *s_MSGON;
extern char     *s_ENDGAME;
extern char     *s_DOSY;
extern char     *s_DOSA;
extern char     *s_OTHERY;
extern char     *s_OTHERA;
extern char     *s_DETAILHI;
extern char     *s_DETAILLO;
extern char     *s_GAMMALVL;
extern char     *s_GAMMAOFF;
extern char     *s_EMPTYSTRING;

extern char     *s_GOTARMOR;
extern char     *s_GOTMEGA;
extern char     *s_GOTHTHBONUS;
extern char     *s_GOTARMBONUS;
extern char     *s_GOTSTIM;
extern char     *s_GOTMEDINEED;
extern char     *s_GOTMEDIKIT;
extern char     *s_GOTSUPER;

extern char     *s_GOTBLUECARD;
extern char     *s_GOTYELWCARD;
extern char     *s_GOTREDCARD;
extern char     *s_GOTBLUESKUL;
extern char     *s_GOTYELWSKUL;
extern char     *s_GOTREDSKUL;
extern char     *s_GOTREDSKULL;

extern char     *s_GOTINVUL;
extern char     *s_GOTBERSERK;
extern char     *s_GOTINVIS;
extern char     *s_GOTSUIT;
extern char     *s_GOTMAP;
extern char     *s_GOTVISOR;

extern char     *s_GOTCLIP;
extern char     *s_GOTCLIPX2;
extern char     *s_GOTHALFCLIP;
extern char     *s_GOTCLIPBOX;
extern char     *s_GOTROCKET;
extern char     *s_GOTROCKETX2;
extern char     *s_GOTROCKBOX;
extern char     *s_GOTCELL;
extern char     *s_GOTCELLX2;
extern char     *s_GOTCELLBOX;
extern char     *s_GOTSHELLS;
extern char     *s_GOTSHELLSX2;
extern char     *s_GOTSHELLBOX;
extern char     *s_GOTBACKPACK;

extern char     *s_GOTBFG9000;
extern char     *s_GOTCHAINGUN;
extern char     *s_GOTCHAINSAW;
extern char     *s_GOTLAUNCHER;
extern char     *s_GOTMSPHERE;
extern char     *s_GOTPLASMA;
extern char     *s_GOTSHOTGUN;
extern char     *s_GOTSHOTGUN2;

extern char     *s_PD_BLUEO;
extern char     *s_PD_REDO;
extern char     *s_PD_YELLOWO;
extern char     *s_PD_BLUEK;
extern char     *s_PD_REDK;
extern char     *s_PD_YELLOWK;
extern char     *s_PD_BLUEC;
extern char     *s_PD_REDC;
extern char     *s_PD_YELLOWC;
extern char     *s_PD_BLUES;
extern char     *s_PD_REDS;
extern char     *s_PD_YELLOWS;
extern char     *s_PD_ANY;
extern char     *s_PD_ALL3;
extern char     *s_PD_ALL6;

extern char     *s_SECRET;

extern char     *s_GGSAVED;
extern char     *s_GGLOADED;
extern char     *s_GGAUTOLOADED;
extern char     *s_GGDELETED;
extern char     *s_GSCREENSHOT;

extern char     *s_ALWAYSRUNOFF;
extern char     *s_ALWAYSRUNON;

extern char     *s_HUSTR_E1M1;
extern char     *s_HUSTR_E1M2;
extern char     *s_HUSTR_E1M3;
extern char     *s_HUSTR_E1M4;
extern char     *s_HUSTR_E1M5;
extern char     *s_HUSTR_E1M6;
extern char     *s_HUSTR_E1M7;
extern char     *s_HUSTR_E1M8;
extern char     *s_HUSTR_E1M9;
extern char     *s_HUSTR_E2M1;
extern char     *s_HUSTR_E2M2;
extern char     *s_HUSTR_E2M3;
extern char     *s_HUSTR_E2M4;
extern char     *s_HUSTR_E2M5;
extern char     *s_HUSTR_E2M6;
extern char     *s_HUSTR_E2M7;
extern char     *s_HUSTR_E2M8;
extern char     *s_HUSTR_E2M9;
extern char     *s_HUSTR_E3M1;
extern char     *s_HUSTR_E3M2;
extern char     *s_HUSTR_E3M3;
extern char     *s_HUSTR_E3M4;
extern char     *s_HUSTR_E3M5;
extern char     *s_HUSTR_E3M6;
extern char     *s_HUSTR_E3M7;
extern char     *s_HUSTR_E3M8;
extern char     *s_HUSTR_E3M9;
extern char     *s_HUSTR_E4M1;
extern char     *s_HUSTR_E4M2;
extern char     *s_HUSTR_E4M3;
extern char     *s_HUSTR_E4M4;
extern char     *s_HUSTR_E4M5;
extern char     *s_HUSTR_E4M6;
extern char     *s_HUSTR_E4M7;
extern char     *s_HUSTR_E4M8;
extern char     *s_HUSTR_E4M9;
extern char     *s_HUSTR_1;
extern char     *s_HUSTR_2;
extern char     *s_HUSTR_3;
extern char     *s_HUSTR_4;
extern char     *s_HUSTR_5;
extern char     *s_HUSTR_6;
extern char     *s_HUSTR_7;
extern char     *s_HUSTR_8;
extern char     *s_HUSTR_9;
extern char     *s_HUSTR_10;
extern char     *s_HUSTR_11;
extern char     *s_HUSTR_12;
extern char     *s_HUSTR_13;
extern char     *s_HUSTR_14;
extern char     *s_HUSTR_15;
extern char     *s_HUSTR_16;
extern char     *s_HUSTR_17;
extern char     *s_HUSTR_18;
extern char     *s_HUSTR_19;
extern char     *s_HUSTR_20;
extern char     *s_HUSTR_21;
extern char     *s_HUSTR_22;
extern char     *s_HUSTR_23;
extern char     *s_HUSTR_24;
extern char     *s_HUSTR_25;
extern char     *s_HUSTR_26;
extern char     *s_HUSTR_27;
extern char     *s_HUSTR_28;
extern char     *s_HUSTR_29;
extern char     *s_HUSTR_30;
extern char     *s_HUSTR_31;
extern char     *s_HUSTR_31_BFG;
extern char     *s_HUSTR_32;
extern char     *s_HUSTR_32_BFG;
extern char     *s_HUSTR_33;
extern char     *s_PHUSTR_1;
extern char     *s_PHUSTR_2;
extern char     *s_PHUSTR_3;
extern char     *s_PHUSTR_4;
extern char     *s_PHUSTR_5;
extern char     *s_PHUSTR_6;
extern char     *s_PHUSTR_7;
extern char     *s_PHUSTR_8;
extern char     *s_PHUSTR_9;
extern char     *s_PHUSTR_10;
extern char     *s_PHUSTR_11;
extern char     *s_PHUSTR_12;
extern char     *s_PHUSTR_13;
extern char     *s_PHUSTR_14;
extern char     *s_PHUSTR_15;
extern char     *s_PHUSTR_16;
extern char     *s_PHUSTR_17;
extern char     *s_PHUSTR_18;
extern char     *s_PHUSTR_19;
extern char     *s_PHUSTR_20;
extern char     *s_PHUSTR_21;
extern char     *s_PHUSTR_22;
extern char     *s_PHUSTR_23;
extern char     *s_PHUSTR_24;
extern char     *s_PHUSTR_25;
extern char     *s_PHUSTR_26;
extern char     *s_PHUSTR_27;
extern char     *s_PHUSTR_28;
extern char     *s_PHUSTR_29;
extern char     *s_PHUSTR_30;
extern char     *s_PHUSTR_31;
extern char     *s_PHUSTR_32;
extern char     *s_THUSTR_1;
extern char     *s_THUSTR_2;
extern char     *s_THUSTR_3;
extern char     *s_THUSTR_4;
extern char     *s_THUSTR_5;
extern char     *s_THUSTR_6;
extern char     *s_THUSTR_7;
extern char     *s_THUSTR_8;
extern char     *s_THUSTR_9;
extern char     *s_THUSTR_10;
extern char     *s_THUSTR_11;
extern char     *s_THUSTR_12;
extern char     *s_THUSTR_13;
extern char     *s_THUSTR_14;
extern char     *s_THUSTR_15;
extern char     *s_THUSTR_16;
extern char     *s_THUSTR_17;
extern char     *s_THUSTR_18;
extern char     *s_THUSTR_19;
extern char     *s_THUSTR_20;
extern char     *s_THUSTR_21;
extern char     *s_THUSTR_22;
extern char     *s_THUSTR_23;
extern char     *s_THUSTR_24;
extern char     *s_THUSTR_25;
extern char     *s_THUSTR_26;
extern char     *s_THUSTR_27;
extern char     *s_THUSTR_28;
extern char     *s_THUSTR_29;
extern char     *s_THUSTR_30;
extern char     *s_THUSTR_31;
extern char     *s_THUSTR_32;
extern char     *s_NHUSTR_1;
extern char     *s_NHUSTR_2;
extern char     *s_NHUSTR_3;
extern char     *s_NHUSTR_4;
extern char     *s_NHUSTR_5;
extern char     *s_NHUSTR_6;
extern char     *s_NHUSTR_7;
extern char     *s_NHUSTR_8;
extern char     *s_NHUSTR_9;

extern char     *s_AMSTR_FOLLOWON;
extern char     *s_AMSTR_FOLLOWOFF;
extern char     *s_AMSTR_GRIDON;
extern char     *s_AMSTR_GRIDOFF;
extern char     *s_AMSTR_MARKEDSPOT;
extern char     *s_AMSTR_MARKCLEARED;
extern char     *s_AMSTR_MARKSCLEARED;
extern char     *s_AMSTR_ROTATEON;
extern char     *s_AMSTR_ROTATEOFF;

extern char     *s_STSTR_MUS;
extern char     *s_STSTR_DQDON;
extern char     *s_STSTR_DQDOFF;
extern char     *s_STSTR_KFAADDED;
extern char     *s_STSTR_FAADDED;
extern char     *s_STSTR_NCON;
extern char     *s_STSTR_NCOFF;
extern char     *s_STSTR_BEHOLD;
extern char     *s_STSTR_BEHOLDX;
extern char     *s_STSTR_BEHOLDON;
extern char     *s_STSTR_BEHOLDOFF;
extern char     *s_STSTR_BUDDHA;
extern char     *s_STSTR_CHOPPERS;
extern char     *s_STSTR_CLEV;
extern char     *s_STSTR_CLEVSAME;
extern char     *s_STSTR_MYPOS;
extern char     *s_STSTR_NTON;
extern char     *s_STSTR_NTOFF;
extern char     *s_STSTR_GODON;
extern char     *s_STSTR_GODOFF;
extern char     *s_STSTR_NMON;
extern char     *s_STSTR_NMOFF;
extern char     *s_STSTR_PSON;
extern char     *s_STSTR_PSOFF;
extern char     *s_STSTR_FMON;
extern char     *s_STSTR_FMOFF;
extern char     *s_STSTR_RION;
extern char     *s_STSTR_RIOFF;
extern char     *s_STSTR_RMON;
extern char     *s_STSTR_RMOFF;
extern char     *s_STSTR_FON;
extern char     *s_STSTR_FOFF;
extern char     *s_STSTR_RHON;
extern char     *s_STSTR_RHOFF;
extern char     *s_STSTR_VMON;
extern char     *s_STSTR_VMOFF;

extern char     *s_E1TEXT;
extern char     *s_E2TEXT;
extern char     *s_E3TEXT;
extern char     *s_E4TEXT;
extern char     *s_C1TEXT;
extern char     *s_C2TEXT;
extern char     *s_C3TEXT;
extern char     *s_C4TEXT;
extern char     *s_C5TEXT;
extern char     *s_C6TEXT;
extern char     *s_P1TEXT;
extern char     *s_P2TEXT;
extern char     *s_P3TEXT;
extern char     *s_P4TEXT;
extern char     *s_P5TEXT;
extern char     *s_P6TEXT;
extern char     *s_T1TEXT;
extern char     *s_T2TEXT;
extern char     *s_T3TEXT;
extern char     *s_T4TEXT;
extern char     *s_T5TEXT;
extern char     *s_T6TEXT;
extern char     *s_N1TEXT;

extern char     *s_CC_ZOMBIE;
extern char     *s_CC_SHOTGUN;
extern char     *s_CC_HEAVY;
extern char     *s_CC_IMP;
extern char     *s_CC_DEMON;
extern char     *s_CC_SPECTRE;
extern char     *s_CC_LOST;
extern char     *s_CC_CACO;
extern char     *s_CC_HELL;
extern char     *s_CC_BARON;
extern char     *s_CC_ARACH;
extern char     *s_CC_PAIN;
extern char     *s_CC_REVEN;
extern char     *s_CC_MANCU;
extern char     *s_CC_ARCH;
extern char     *s_CC_SPIDER;
extern char     *s_CC_CYBER;
extern char     *s_CC_HERO;

extern char     *s_DOOM_ENDMSG1;
extern char     *s_DOOM_ENDMSG2;
extern char     *s_DOOM_ENDMSG3;
extern char     *s_DOOM_ENDMSG4;
extern char     *s_DOOM_ENDMSG5;
extern char     *s_DOOM_ENDMSG6;
extern char     *s_DOOM_ENDMSG7;
extern char     *s_DOOM2_ENDMSG1;
extern char     *s_DOOM2_ENDMSG2;
extern char     *s_DOOM2_ENDMSG3;
extern char     *s_DOOM2_ENDMSG4;
extern char     *s_DOOM2_ENDMSG5;
extern char     *s_DOOM2_ENDMSG6;
extern char     *s_DOOM2_ENDMSG7;

extern char     *s_M_NEWGAME;
extern char     *s_M_OPTIONS;
extern char     *s_M_LOADGAME;
extern char     *s_M_SAVEGAME;
extern char     *s_M_QUITGAME;
extern char     *s_M_WHICHEPISODE;
extern char     *s_M_EPISODE1;
extern char     *s_M_EPISODE2;
extern char     *s_M_EPISODE3;
extern char     *s_M_EPISODE4;
extern char     *s_M_EPISODE5;
extern char     *s_M_WHICHEXPANSION;
extern char     *s_M_EXPANSION1;
extern char     *s_M_EXPANSION2;
extern char     *s_M_CHOOSESKILLLEVEL;
extern char     *s_M_SKILLLEVEL1;
extern char     *s_M_SKILLLEVEL2;
extern char     *s_M_SKILLLEVEL3;
extern char     *s_M_SKILLLEVEL4;
extern char     *s_M_SKILLLEVEL5;
extern char     *s_M_ENDGAME;
extern char     *s_M_MESSAGES;
extern char     *s_M_ON;
extern char     *s_M_OFF;
extern char     *s_M_GRAPHICDETAIL;
extern char     *s_M_HIGH;
extern char     *s_M_LOW;
extern char     *s_M_SCREENSIZE;
extern char     *s_M_MOUSESENSITIVITY;
extern char     *s_M_GAMEPADSENSITIVITY;
extern char     *s_M_SOUNDVOLUME;
extern char     *s_M_SFXVOLUME;
extern char     *s_M_MUSICVOLUME;
extern char     *s_M_PAUSED;
extern char     *s_M_MORE;

extern char     *s_CAPTION_SHAREWARE;
extern char     *s_CAPTION_REGISTERED;
extern char     *s_CAPTION_ULTIMATE;
extern char     *s_CAPTION_DOOM2;
extern char     *s_CAPTION_HELLONEARTH;
extern char     *s_CAPTION_NERVE;
extern char     *s_CAPTION_BFGEDITION;
extern char     *s_CAPTION_PLUTONIA;
extern char     *s_CAPTION_TNT;
extern char     *s_CAPTION_CHEX;
extern char     *s_CAPTION_CHEX2;
extern char     *s_CAPTION_HACX;
extern char     *s_CAPTION_FREEDOOM1;
extern char     *s_CAPTION_FREEDOOM2;
extern char     *s_CAPTION_FREEDM;
extern char     *s_CAPTION_BTSXE1;
extern char     *s_CAPTION_BTSXE2;
extern char     *s_CAPTION_BTSXE3;
extern char     *s_CAPTION_E1M4B;
extern char     *s_CAPTION_E1M8B;

extern char     *s_AUTHOR_BESTOR;
extern char     *s_AUTHOR_ROMERO;

extern char     *bgflatE1;
extern char     *bgflatE2;
extern char     *bgflatE3;
extern char     *bgflatE4;
extern char     *bgflat06;
extern char     *bgflat11;
extern char     *bgflat20;
extern char     *bgflat30;
extern char     *bgflat15;
extern char     *bgflat31;
extern char     *bgcastcall;

extern char     *startup1;
extern char     *startup2;
extern char     *startup3;
extern char     *startup4;
extern char     *startup5;

dboolean CheckPackageWADVersion(void);

#endif
