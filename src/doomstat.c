/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

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

#include "doomstat.h"

// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t      gamemode = indetermined;
GameMission_t   gamemission = doom;
GameVersion_t   gameversion = exe_final;
char            *gamedescription;

bool            nerve = false;
bool            bfgedition = false;

bool            breach = false;
bool            chex = false;
bool            chexdeh = false;
bool            hacx = false;
bool            BTSX = false;
bool            BTSXE1 = false;
bool            BTSXE2 = false;
bool            BTSXE2A = false;
bool            BTSXE2B = false;
bool            BTSXE3 = false;
bool            BTSXE3A = false;
bool            BTSXE3B = false;
bool            E1M4B = false;
bool            E1M8B = false;

// Set if homebrew PWAD stuff has been added.
bool            modifiedgame;

bool            DMENUPIC = false;
bool            FREEDOOM = false;
bool            FREEDM = false;
bool            M_DOOM = false;
bool            M_EPISOD = false;
bool            M_GDHIGH = false;
bool            M_GDLOW = false;
bool            M_LOADG = false;
bool            M_LSCNTR = false;
bool            M_MSENS = false;
bool            M_MSGOFF = false;
bool            M_MSGON = false;
bool            M_NEWG = false;
bool            M_NMARE = false;
bool            M_OPTTTL = false;
bool            M_PAUSE = false;
bool            M_SAVEG = false;
bool            M_SKILL = false;
bool            M_SKULL1 = false;
bool            M_SVOL = false;
int             STARMS = 0;
int             STBAR = 0;
bool            STCFN034 = false;
bool            STCFN039 = false;
bool            STCFN121 = false;
bool            STYSNUM0 = false;
bool            TITLEPIC = false;
bool            WISCRT2 = false;
