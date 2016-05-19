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

#if !defined(__DOOMSTAT_H__)
#define __DOOMSTAT_H__

// We need globally shared data structures,
//  for defining the global state variables.
#include "d_loop.h"

// We need the player data structure as well.
#include "d_player.h"

// ------------------------
// Command line parameters.
//
extern  bool            nomonsters;     // checkparm of -nomonsters
extern  bool            respawnparm;    // checkparm of -respawn
extern  bool            pistolstart;    // [BH] checkparm of -pistolstart
extern  bool            fastparm;       // checkparm of -fast

extern  bool            devparm;        // DEBUG: launched with -devparm

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t       gamemode;
extern GameMission_t    gamemission;
extern GameVersion_t    gameversion;
extern char             *gamedescription;

// Set if homebrew PWAD stuff has been added.
extern  bool            modifiedgame;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern skill_t          startskill;
extern int              startepisode;
extern int              startmap;

// Savegame slot to load on startup. This is the value provided to
// the -loadgame option. If this has not been provided, this is -1.
extern int              startloadgame;

extern bool             autostart;

// Selected by user.
extern skill_t          gameskill;
extern int              gameepisode;
extern int              gamemap;

extern bool             nerve;
extern bool             bfgedition;

extern bool             breach;
extern bool             chex;
extern bool             chexdeh;
extern bool             hacx;
extern bool             BTSX;
extern bool             BTSXE1;
extern bool             BTSXE2;
extern bool             BTSXE2A;
extern bool             BTSXE2B;
extern bool             BTSXE3;
extern bool             BTSXE3A;
extern bool             BTSXE3B;
extern bool             E1M4B;
extern bool             E1M8B;

extern bool             DMENUPIC;
extern bool             FREEDOOM;
extern bool             FREEDM;
extern bool             M_DOOM;
extern bool             M_EPISOD;
extern bool             M_GDHIGH;
extern bool             M_GDLOW;
extern bool             M_LOADG;
extern bool             M_LSCNTR;
extern bool             M_MSENS;
extern bool             M_MSGOFF;
extern bool             M_MSGON;
extern bool             M_NEWG;
extern bool             M_NMARE;
extern bool             M_OPTTTL;
extern bool             M_PAUSE;
extern bool             M_SAVEG;
extern bool             M_SKILL;
extern bool             M_SKULL1;
extern bool             M_SVOL;
extern int              STARMS;
extern int              STBAR;
extern bool             STCFN034;
extern bool             STCFN039;
extern bool             STCFN121;
extern bool             STYSNUM0;
extern bool             TITLEPIC;
extern bool             WISCRT2;

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

extern bool             automapactive;  // In AutoMap mode?
extern bool             am_followmode;  // Following player in AutoMap mode?
extern bool             menuactive;     // Menu overlayed?
extern bool             paused;         // Game Pause?

extern bool             viewactive;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int              totalkills;
extern int              totalitems;
extern int              totalsecret;
extern int              monstercount[NUMMOBJTYPES];

// Timer, for scores.
extern int              levelstarttic;  // gametic at level start
extern int              leveltime;      // tics in game play for par

extern bool             usergame;

//?
extern gamestate_t      gamestate;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int              gametic;
extern int              gametime;

extern bool             vid_capfps;
extern bool             realframe;

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

extern int              m_sensitivity;
extern int              gp_sensitivity;

// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int              skyflatnum;

extern ticcmd_t         netcmds[BACKUPTICS];

#endif
