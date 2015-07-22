/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#if !defined(__D_STATE__)
#define __D_STATE__

// We need globally shared data structures,
//  for defining the global state variables.
#include "doomdata.h"
#include "d_loop.h"

// We need the playr data structure as well.
#include "d_player.h"

// ------------------------
// Command line parameters.
//
extern  dboolean        nomonsters;     // checkparm of -nomonsters
extern  dboolean        fastparm;       // checkparm of -fast

extern  dboolean        devparm;        // DEBUG: launched with -devparm

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t       gamemode;
extern GameMission_t    gamemission;
extern GameVersion_t    gameversion;
extern char             *gamedescription;

// Set if homebrew PWAD stuff has been added.
extern  dboolean        modifiedgame;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern skill_t          startskill;
extern int              startepisode;
extern int              startmap;

// Savegame slot to load on startup.  This is the value provided to
// the -loadgame option.  If this has not been provided, this is -1.
extern int              startloadgame;

extern dboolean         autostart;

// Selected by user.
extern skill_t          gameskill;
extern int              gameepisode;
extern int              gamemap;

extern dboolean         nerve;
extern dboolean         bfgedition;

extern dboolean         chex;
extern dboolean         chexdeh;
extern dboolean         hacx;
extern dboolean         BTSX;
extern dboolean         BTSXE1;
extern dboolean         BTSXE2;
extern dboolean         BTSXE2A;
extern dboolean         BTSXE2B;
extern dboolean         BTSXE3;
extern dboolean         BTSXE3A;
extern dboolean         BTSXE3B;

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
extern dboolean         M_NMARE;
extern dboolean         M_OPTTTL;
extern dboolean         M_PAUSE;
extern dboolean         M_SAVEG;
extern dboolean         M_SKILL;
extern dboolean         M_SKULL1;
extern dboolean         M_SVOL;
extern dboolean         STARMS;
extern dboolean         STBAR;
extern dboolean         STCFN034;
extern dboolean         STCFN039;
extern dboolean         STCFN121;
extern dboolean         STYSNUM0;
extern dboolean         TITLEPIC;
extern dboolean         WISCRT2;

// Nightmare mode flag, single player.
extern dboolean         respawnmonsters;

// -------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.

// From m_menu.c:
//  Sound FX volume has default, 0 - 15
//  Music volume has default, 0 - 15
// These are multiplied by 8.
extern int              sfxVolume;
extern int              musicVolume;

// -------------------------
// Status flags for refresh.
//

extern dboolean         automapactive;  // In AutoMap mode?
extern dboolean         am_followmode;  // Following player in AutoMap mode?
extern dboolean         menuactive;     // Menu overlayed?
extern dboolean         paused;         // Game Pause?

extern dboolean         viewactive;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int              totalkills;
extern int              totalitems;
extern int              totalsecret;

// Timer, for scores.
extern int              levelstarttic;  // gametic at level start
extern int              leveltime;      // tics in game play for par

extern dboolean         usergame;

//?
extern gamestate_t      gamestate;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int              gametic;
extern int              gametime;

extern dboolean         vid_capfps;
extern dboolean         realframe;

// Bookkeeping on players - state.
extern player_t         players[MAXPLAYERS];

// Player spawn spots.
extern mapthing_t       playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t  wminfo;

//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern char             *savegamefolder;
extern char             basedefault[1024];

// if true, load all graphics at level load
extern dboolean         precache;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern gamestate_t      wipegamestate;

extern int              m_sensitivity;
extern int              gp_sensitivity;
extern float            gamepadsensitivityf;

// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int              skyflatnum;

// Netgame stuff (buffers and pointers, i.e. indices).
extern int              maketic;

extern ticcmd_t         netcmds[BACKUPTICS];
extern int              ticdup;

#endif
