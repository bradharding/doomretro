/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

boolean         sigil;
boolean         autosigil = false;
boolean         buckethead = false;
boolean         nerve;
boolean         bfgedition;
boolean         unity;

boolean         chex;
boolean         chex1;
boolean         chex2;
boolean         hacx;
boolean         BTSX;
boolean         BTSXE1;
boolean         BTSXE1A;
boolean         BTSXE1B;
boolean         BTSXE2;
boolean         BTSXE2A;
boolean         BTSXE2B;
boolean         BTSXE3;
boolean         BTSXE3A;
boolean         BTSXE3B;
boolean         E1M4B;
boolean         E1M8B;
boolean         onehumanity;
boolean         sprfix18;
boolean         eviternity;
boolean         doom4vanilla;
boolean         REKKR;
boolean         REKKRSA;
boolean         REKKRSL;

boolean         moreblood;

// Set if homebrew PWAD stuff has been added.
boolean         modifiedgame = false;

boolean         DMENUPIC = false;
boolean         DSSECRET;
boolean         FREEDOOM;
boolean         FREEDM;
boolean         M_DOOM;
boolean         M_EPISOD;
boolean         M_GDHIGH;
boolean         M_GDLOW;
boolean         M_LGTTL;
boolean         M_LOADG;
boolean         M_LSCNTR;
boolean         M_MSENS;
boolean         M_MSGOFF;
boolean         M_MSGON;
boolean         M_NEWG;
boolean         M_NGAME;
boolean         M_NMARE;
boolean         M_OPTTTL;
boolean         M_PAUSE;
boolean         M_SAVEG;
boolean         M_SGTTL;
boolean         M_SKILL;
boolean         M_SKULL1;
boolean         M_SVOL;
short           RROCK05;
short           RROCK08;
short           SLIME09;
short           SLIME12;
boolean         STCFN034;
boolean         STYSNUM0;
boolean         WISCRT2;

int             PLAYPALs;
int             STBARs;
