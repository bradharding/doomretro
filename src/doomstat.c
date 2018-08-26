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

#include "doomstat.h"

// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t      gamemode = indetermined;
GameMission_t   gamemission = doom;
GameVersion_t   gameversion = exe_final;
char            *gamedescription;

dboolean        nerve;
dboolean        bfgedition;

dboolean        breach;
dboolean        chex;
dboolean        chex1;
dboolean        chex2;
dboolean        chexdeh;
dboolean        hacx;
dboolean        BTSX;
dboolean        BTSXE1;
dboolean        BTSXE1A;
dboolean        BTSXE1B;
dboolean        BTSXE2;
dboolean        BTSXE2A;
dboolean        BTSXE2B;
dboolean        BTSXE3;
dboolean        BTSXE3A;
dboolean        BTSXE3B;
dboolean        E1M4B;
dboolean        E1M8B;
dboolean        sprfix18;

// Set if homebrew PWAD stuff has been added.
dboolean        modifiedgame;

dboolean        DMENUPIC;
dboolean        FREEDOOM;
dboolean        FREEDM;
dboolean        M_DOOM;
dboolean        M_EPISOD;
dboolean        M_GDHIGH;
dboolean        M_GDLOW;
dboolean        M_LOADG;
dboolean        M_LSCNTR;
dboolean        M_MSENS;
dboolean        M_MSGOFF;
dboolean        M_MSGON;
dboolean        M_NEWG;
dboolean        M_NGAME;
dboolean        M_NMARE;
dboolean        M_OPTTTL;
dboolean        M_PAUSE;
dboolean        M_SAVEG;
dboolean        M_SKILL;
dboolean        M_SKULL1;
dboolean        M_SVOL;
int             STBAR;
dboolean        STCFN034;
dboolean        STYSNUM0;
dboolean        TITLEPIC;
dboolean        WISCRT2;
dboolean        DSSECRET;
