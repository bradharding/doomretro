/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (c) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (c) 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

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

#include "doomstat.h"

// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t      gamemode = indetermined;
GameMission_t   gamemission = doom;
GameVersion_t   gameversion = exe_final;
char            *gamedescription;

dboolean        nerve = false;
dboolean        bfgedition = false;

dboolean        chex = false;
dboolean        chexdeh = false;
dboolean        hacx = false;
dboolean        BTSX = false;
dboolean        BTSXE1 = false;
dboolean        BTSXE2 = false;
dboolean        BTSXE2A = false;
dboolean        BTSXE2B = false;
dboolean        BTSXE3 = false;
dboolean        BTSXE3A = false;
dboolean        BTSXE3B = false;

// Set if homebrew PWAD stuff has been added.
dboolean        modifiedgame;

dboolean        DMENUPIC = false;
dboolean        FREEDOOM = false;
dboolean        FREEDM = false;
dboolean        M_DOOM = false;
dboolean        M_EPISOD = false;
dboolean        M_GDHIGH = false;
dboolean        M_GDLOW = false;
dboolean        M_LOADG = false;
dboolean        M_LSCNTR = false;
dboolean        M_MSENS = false;
dboolean        M_MSGOFF = false;
dboolean        M_MSGON = false;
dboolean        M_NEWG = false;
dboolean        M_NMARE = false;
dboolean        M_OPTTTL = false;
dboolean        M_PAUSE = false;
dboolean        M_SAVEG = false;
dboolean        M_SKILL = false;
dboolean        M_SKULL1 = false;
dboolean        M_SVOL = false;
dboolean        STARMS = false;
dboolean        STBAR = false;
dboolean        STCFN034 = false;
dboolean        STCFN039 = false;
dboolean        STCFN121 = false;
dboolean        STYSNUM0 = false;
dboolean        TITLEPIC = false;
dboolean        WISCRT2 = false;
