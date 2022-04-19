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
extern boolean          nomonsters;             // checkparm of -nomonsters
extern boolean          regenhealth;
extern boolean          respawnitems;
extern boolean          respawnmonsters;        // checkparm of -respawn
extern boolean          pistolstart;            // [BH] checkparm of -pistolstart
extern boolean          fastparm;               // checkparm of -fast

extern boolean          devparm;                // DEBUG: launched with -devparm

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t       gamemode;
extern GameMission_t    gamemission;
extern char             gamedescription[255];

// Set if homebrew PWAD stuff has been added.
extern boolean          modifiedgame;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern skill_t          startskill;
extern int              startepisode;

extern boolean          autostart;

// Selected by user.
extern skill_t          gameskill;
extern int              pendinggameskill;
extern int              gameepisode;
extern int              gamemap;
extern char             speciallumpname[6];

extern boolean          freeze;

extern boolean          sigil;
extern boolean          autosigil;
extern boolean          buckethead;
extern boolean          nerve;
extern boolean          bfgedition;
extern boolean          unity;

extern boolean          chex;
extern boolean          chex1;
extern boolean          chex2;
extern boolean          hacx;
extern boolean          BTSX;
extern boolean          BTSXE1;
extern boolean          BTSXE1A;
extern boolean          BTSXE1B;
extern boolean          BTSXE2;
extern boolean          BTSXE2A;
extern boolean          BTSXE2B;
extern boolean          BTSXE3;
extern boolean          BTSXE3A;
extern boolean          BTSXE3B;
extern boolean          E1M4B;
extern boolean          E1M8B;
extern boolean          onehumanity;
extern boolean          sprfix18;
extern boolean          eviternity;
extern boolean          doom4vanilla;
extern boolean          REKKR;
extern boolean          REKKRSA;
extern boolean          REKKRSL;

extern boolean          moreblood;

extern boolean          DMENUPIC;
extern boolean          DSSECRET;
extern boolean          FREEDOOM;
extern boolean          FREEDM;
extern boolean          M_DOOM;
extern boolean          M_EPISOD;
extern boolean          M_GDHIGH;
extern boolean          M_GDLOW;
extern boolean          M_LGTTL;
extern boolean          M_LOADG;
extern boolean          M_LSCNTR;
extern boolean          M_MSENS;
extern boolean          M_MSGOFF;
extern boolean          M_MSGON;
extern boolean          M_NEWG;
extern boolean          M_NGAME;
extern boolean          M_NMARE;
extern boolean          M_OPTTTL;
extern boolean          M_PAUSE;
extern boolean          M_SAVEG;
extern boolean          M_SGTTL;
extern boolean          M_SKILL;
extern boolean          M_SKULL1;
extern boolean          M_SVOL;
extern short            RROCK05;
extern short            RROCK08;
extern short            SLIME09;
extern short            SLIME12;
extern boolean          STCFN034;
extern boolean          STYSNUM0;
extern boolean          WISCRT2;

extern int              PLAYPALs;
extern int              STBARs;

// -------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.

// From m_menu.c:
//  Sound FX volume has default, 0 - 31
//  Music volume has default, 0 - 31
// These are multiplied by 4.
extern int              sfxVolume;
extern int              musicVolume;

// -------------------------
// Status flags for refresh.
//

extern boolean          automapactive;  // In automap mode?
extern boolean          menuactive;     // Menu overlaid?
extern boolean          paused;         // Game Pause?

extern boolean          viewactive;

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

// Timer, for scores.
extern int              leveltime;      // tics in game play for par

// ?
extern gamestate_t      gamestate;

// -----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int              gametime;
extern struct tm        gamestarttime;

extern boolean          realframe;

// Intermission stats.
// Parameters for world map/intermission.
extern wbstartstruct_t  wminfo;

// -----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern char             *savegamefolder;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern gamestate_t      wipegamestate;

// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int              skyflatnum;

extern ticcmd_t         localcmds[BACKUPTICS];
