/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#pragma once

#include <time.h>

// We need globally shared data structures,
//  for defining the global state variables.
#include "d_loop.h"

// We need the player data structure as well.
#include "d_player.h"

// ------------------------
// Command line parameters.
//
extern bool             nomonsters;             // checkparm of -nomonsters
extern bool             regenhealth;
extern bool             respawnitems;
extern bool             respawnmonsters;        // checkparm of -respawn
extern bool             solonet;                // [BH] checkparm of -solonet
extern bool             pistolstart;            // [BH] checkparm of -pistolstart
extern bool             fastparm;               // checkparm of -fast

extern bool             devparm;                // DEBUG: launched with -devparm

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern gamemode_t       gamemode;
extern gamemission_t    gamemission;
extern char             gamedescription[255];

// Set if homebrew PWAD stuff has been added.
extern bool             modifiedgame;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern skill_t          startskill;
extern int              startepisode;

extern bool             autostart;

// Selected by user.
extern skill_t          gameskill;
extern int              pendinggameskill;
extern int              gameepisode;
extern int              gamemap;
extern char             speciallumpname[6];

extern bool             freeze;

extern bool             sigil;
extern bool             sigil2;
extern bool             autosigil;
extern bool             autosigil2;
extern bool             buckethead;
extern bool             thorr;
extern bool             nerve;
extern bool             bfgedition;
extern bool             unity;

extern bool             chex;
extern bool             chex1;
extern bool             chex2;
extern bool             hacx;
extern bool             BTSX;
extern bool             BTSXE1;
extern bool             BTSXE1A;
extern bool             BTSXE1B;
extern bool             BTSXE2;
extern bool             BTSXE2A;
extern bool             BTSXE2B;
extern bool             BTSXE3;
extern bool             BTSXE3A;
extern bool             BTSXE3B;
extern bool             BTSXE3PR;
extern bool             E1M4B;
extern bool             E1M8B;
extern bool             KDIKDIZD;
extern bool             KDIKDIZDA;
extern bool             KDIKDIZDB;
extern bool             onehumanity;
extern bool             sprfix18;
extern bool             eviternity;
extern bool             doom4vanilla;
extern bool             REKKR;
extern bool             REKKRSA;
extern bool             REKKRSL;
extern bool             anomalyreport;
extern bool             arrival;
extern bool             dbimpact;
extern bool             deathless;
extern bool             doomzero;
extern bool             earthless;
extern bool             ganymede;
extern bool             harmony;
extern bool             harmonyc;
extern bool             neis;
extern bool             revolution;
extern bool             scientist;
extern bool             SD21;
extern bool             syringe;

extern bool             moreblood;

extern bool             DSFLAMST;
extern bool             E1M4;
extern bool             E1M8;
extern bool             FREEDOOM;
extern bool             FREEDOOM1;
extern bool             FREEDM;
extern bool             M_DOOM;
extern bool             M_EPISOD;
extern bool             M_GDHIGH;
extern bool             M_GDLOW;
extern bool             M_LGTTL;
extern bool             M_LOADG;
extern bool             M_LSCNTR;
extern bool             M_MSENS;
extern bool             M_MSGOFF;
extern bool             M_MSGON;
extern bool             M_NEWG;
extern bool             M_NGAME;
extern bool             M_NMARE;
extern bool             M_OPTTTL;
extern bool             M_PAUSE;
extern bool             M_SAVEG;
extern bool             M_SGTTL;
extern bool             M_SKILL;
extern bool             M_SKULL1;
extern bool             M_SVOL;
extern bool             PUFFA0;
extern short            RROCK05;
extern short            RROCK08;
extern short            SLIME09;
extern short            SLIME12;
extern bool             STCFNxxx;
extern bool             STYSNUM0;
extern bool             WISCRT2;

extern int              PLAYPALs;
extern int              STBARs;

// -------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.)

// From m_menu.c:
//  Sound FX volume has default, 0 - 31
//  Music volume has default, 0 - 31
// These are multiplied by 4.
extern int              sfxvolume;
extern int              musicvolume;

// -------------------------
// Status flags for refresh.
//

extern bool             automapactive;  // In automap mode?
extern bool             menuactive;     // Menu overlaid?
extern bool             paused;         // Game Pause?

extern bool             viewactive;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int              totalkills;
extern int              totalitems;
extern int              totalsecrets;
extern int              totalpickups;
extern int              monstercount[NUMMOBJTYPES];
extern int              barrelcount;
extern int              player1starts;

// Timer, for scores.
extern int              maptime;        // tics in game play for par

// ?
extern gamestate_t      gamestate;

// -----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int              gametime;
extern struct tm        gamestarttime;

extern bool             realframe;

// Intermission stats.
// Parameters for world map/intermission.
extern wbstartstruct_t  wminfo;

// -----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern char             *savegamefolder;
extern char             *autoloadfolder;
extern char             *autoloadiwadsubfolder;
extern char             *autoloadpwadsubfolder;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern gamestate_t      wipegamestate;

// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int              skyflatnum;

extern ticcmd_t         localcmds[BACKUPTICS];
