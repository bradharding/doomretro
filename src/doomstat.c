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

#include "doomstat.h"

// Game Mode - identify IWAD as shareware, retail, etc.
GameMode_t      gamemode = indetermined;
GameMission_t   gamemission = doom;
char            gamedescription[255];

bool            sigil;
bool            autosigil = false;
bool            buckethead = false;
bool            nerve;
bool            bfgedition;
bool            unity;

bool            chex;
bool            chex1;
bool            chex2;
bool            hacx;
bool            BTSX;
bool            BTSXE1;
bool            BTSXE1A;
bool            BTSXE1B;
bool            BTSXE2;
bool            BTSXE2A;
bool            BTSXE2B;
bool            E1M4B;
bool            E1M8B;
bool            onehumanity;
bool            sprfix18;
bool            eviternity;
bool            doom4vanilla;
bool            REKKR;
bool            REKKRSA;
bool            REKKRSL;
bool            anomalyreport;
bool            arrival;
bool            dbimpact;
bool            deathless;
bool            doomzero;
bool            earthless;
bool            neis;
bool            revolution;
bool            syringe;

bool            moreblood = false;

// Set if homebrew PWAD stuff has been added.
bool            modifiedgame = false;

bool            DMENUPIC = false;
bool            DSFLAMST;
bool            FREEDOOM;
bool            FREEDOOM1;
bool            FREEDM;
bool            M_DOOM;
bool            M_EPISOD;
bool            M_GDHIGH;
bool            M_GDLOW;
bool            M_LGTTL;
bool            M_LOADG;
bool            M_LSCNTR;
bool            M_MSENS;
bool            M_MSGOFF;
bool            M_MSGON;
bool            M_NEWG;
bool            M_NGAME;
bool            M_NMARE;
bool            M_OPTTTL;
bool            M_PAUSE;
bool            M_SAVEG;
bool            M_SGTTL;
bool            M_SKILL;
bool            M_SKULL1;
bool            M_SVOL;
short           RROCK05;
short           RROCK08;
short           SLIME09;
short           SLIME12;
bool            STCFNxxx;
bool            STYSNUM0;
bool            WISCRT2;

int             PLAYPALs;
int             STBARs;
