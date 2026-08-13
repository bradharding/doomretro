/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#include "doomstat.h"

// Game Mode - identify IWAD as shareware, retail, etc.
gamemode_t      gamemode = indetermined;
gamemission_t   gamemission = doom;
char            gamedescription[255];

bool            sigil = false;
bool            sigil2 = false;
bool            autosigil = false;
bool            autosigil2 = false;
bool            buckethead = false;
bool            registeredsigil = false;
bool            thorr = false;
bool            nerve = false;
bool            bfgedition = false;
bool            kex = false;
bool            unity = false;
bool            extras = false;

bool            chex = false;
bool            chex1 = false;
bool            chex2 = false;
bool            hacx = false;
bool            BTSX = false;
bool            BTSXE1 = false;
bool            BTSXE1A = false;
bool            BTSXE1B = false;
bool            BTSXE2 = false;
bool            BTSXE2A = false;
bool            BTSXE2B = false;
bool            BTSXE3 = false;
bool            BTSXE3A = false;
bool            BTSXE3B = false;
bool            E1M4B = false;
bool            E1M8B = false;
bool            IDDM1 = false;
bool            KDIKDIZD = false;
bool            KDIKDIZDA = false;
bool            KDIKDIZDB = false;
bool            onehumanity = false;
bool            sprfix18 = false;
bool            eviternity = false;
bool            doom4vanilla = false;
bool            REKKR = false;
bool            REKKRSA = false;
bool            REKKRSL = false;
bool            anomalyreport = false;
bool            arrival = false;
bool            dbimpact = false;
bool            deathless = false;
bool            doomzero = false;
bool            earthless = false;
bool            ganymede = false;
bool            goingdown = false;
bool            goingdownturbo = false;
bool            harmony = false;
bool            harmonyc = false;
bool            legacyofrust = false;
bool            masterlevels = false;
bool            neis = false;
bool            revolution = false;
bool            scientist = false;
bool            SD21 = false;
bool            syringe = false;
bool            TTNS = false;
bool            TTP = false;

bool            moreblood = false;

// Set if homebrew PWAD stuff has been added.
bool            modifiedgame = false;

bool            DBIGFONT;
bool            DSFLAMST;
bool            E1M4;
bool            E1M8;
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
bool            PUFFA0 = false;
short           RROCK05;
short           RROCK08;
short           SLIME09;
short           SLIME12;
bool            STCFNxxx = false;
bool            STYSNUM0;
bool            WICOLON;
bool            WIPERIOD;
bool            WISCRT2;

int             PLAYPALs;
int             STBARs;
