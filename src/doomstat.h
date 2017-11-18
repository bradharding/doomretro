/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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

#if !defined(__DOOMSTAT_H__)
#define __DOOMSTAT_H__

#include <time.h>

// We need globally shared data structures,
//  for defining the global state variables.
#include "d_loop.h"

// We need the player data structure as well.
#include "d_player.h"

// ------------------------
// Command line parameters.
//
extern dboolean         nomonsters;             // checkparm of -nomonsters
extern dboolean         regenhealth;
extern dboolean         respawnitems;
extern dboolean         respawnmonsters;        // checkparm of -respawn
extern dboolean         pistolstart;            // [BH] checkparm of -pistolstart
extern dboolean         fastparm;               // checkparm of -fast

extern dboolean         devparm;                // DEBUG: launched with -devparm

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t       gamemode;
extern GameMission_t    gamemission;
extern GameVersion_t    gameversion;
extern char             *gamedescription;

// Set if homebrew PWAD stuff has been added.
extern dboolean         modifiedgame;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern skill_t          startskill;
extern int              startepisode;
extern int              startmap;

extern dboolean         autostart;

// Selected by user.
extern skill_t          gameskill;
extern int              pendinggameskill;
extern int              gameepisode;
extern int              gamemap;

extern dboolean         freeze;

extern dboolean         nerve;
extern dboolean         bfgedition;

extern dboolean         breach;
extern dboolean         chex;
extern dboolean         chex1;
extern dboolean         chex2;
extern dboolean         chexdeh;
extern dboolean         hacx;
extern dboolean         BTSX;
extern dboolean         BTSXE1;
extern dboolean         BTSXE1A;
extern dboolean         BTSXE1B;
extern dboolean         BTSXE2;
extern dboolean         BTSXE2A;
extern dboolean         BTSXE2B;
extern dboolean         BTSXE3;
extern dboolean         BTSXE3A;
extern dboolean         BTSXE3B;
extern dboolean         E1M4B;
extern dboolean         E1M8B;
extern dboolean         sprfix18;

extern dboolean         DMENUPIC;
extern dboolean         FREEDOOM;
extern dboolean         FREEDM;
extern dboolean         M_DOOM;
extern dboolean         M_EPISOD;
extern dboolean         M_GDHIGH;
extern dboolean         M_GDLOW;
extern dboolean         M_LOADG;
extern dboolean         M_LSCNTR;
extern dboolean         M_MSENS;
extern dboolean         M_MSGOFF;
extern dboolean         M_MSGON;
extern dboolean         M_NEWG;
extern dboolean         M_NGAME;
extern dboolean         M_NMARE;
extern dboolean         M_OPTTTL;
extern dboolean         M_PAUSE;
extern dboolean         M_SAVEG;
extern dboolean         M_SKILL;
extern dboolean         M_SKULL1;
extern dboolean         M_SVOL;
extern int              STARMS;
extern int              STBAR;
extern dboolean         STCFN034;
extern dboolean         STCFN121;
extern dboolean         STYSNUM0;
extern dboolean         TITLEPIC;
extern dboolean         WISCRT2;
extern dboolean         DSSECRET;

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

extern dboolean         automapactive;  // In automap mode?
extern dboolean         menuactive;     // Menu overlaid?
extern dboolean         paused;         // Game Pause?

extern dboolean         viewactive;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int              totalkills;
extern int              totalitems;
extern int              totalsecret;
extern int              totalpickups;
extern int              monstercount[NUMMOBJTYPES];
extern int              barrelcount;

// Timer, for scores.
extern int              leveltime;      // tics in game play for par

//?
extern gamestate_t      gamestate;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int              gametic;
extern int              activetic;
extern int              gametime;
extern struct tm        *gamestarttime;


extern int              vid_capfps;
extern dboolean         realframe;

// Bookkeeping on players - state.
extern player_t         players[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t  wminfo;

//-----------------------------------------
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

extern ticcmd_t         netcmds[BACKUPTICS];

#endif
