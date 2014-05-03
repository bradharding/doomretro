/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include "doomstat.h"


// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t      gamemode = indetermined;
GameMission_t   gamemission = doom;
GameVersion_t   gameversion = exe_final;
char            *gamedescription;

boolean         nerve = false;
boolean         bfgedition = false;

// Set if homebrew PWAD stuff has been added.
boolean         modifiedgame;

boolean         DMENUPIC = false;
boolean         MAPINFO = false;
boolean         M_DOOM = false;
boolean         M_EPISOD = false;
boolean         M_GDHIGH = false;
boolean         M_GDLOW = false;
boolean         M_LGTTL = false;
boolean         M_LSCNTR = false;
boolean         M_MSENS = false;
boolean         M_MSGOFF = false;
boolean         M_MSGON = false;
boolean         M_NEWG = false;
boolean         M_NMARE = false;
boolean         M_OPTTTL = false;
boolean         M_PAUSE = false;
boolean         M_SGTTL = false;
boolean         M_SKILL = false;
boolean         M_SKULL1 = false;
boolean         M_SVOL = false;
boolean         STARMS = false;
boolean         STBAR = false;
boolean         STCFN034 = false;
boolean         STCFN039 = false;
boolean         STCFN121 = false;
boolean         STYSNUM0 = false;
boolean         TITLEPIC = false;
boolean         WISCRT2 = false;
