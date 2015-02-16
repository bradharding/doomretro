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

#ifndef __D_STATE__
#define __D_STATE__

// We need globally shared data structures,
//  for defining the global state variables.
#include "doomdata.h"
#include "d_net.h"

// We need the playr data structure as well.
#include "d_player.h"

// ------------------------
// Command line parameters.
//
extern  boolean         nomonsters;     // checkparm of -nomonsters
extern  boolean         respawnparm;    // checkparm of -respawn
extern  boolean         fastparm;       // checkparm of -fast

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t       gamemode;
extern GameMission_t    gamemission;
extern GameVersion_t    gameversion;
extern char             *gamedescription;

// Set if homebrew PWAD stuff has been added.
extern  boolean         modifiedgame;

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

extern boolean          autostart;

// Selected by user.
extern skill_t          gameskill;
extern int              gameepisode;
extern int              gamemap;

extern boolean          nerve;
extern boolean          bfgedition;

extern boolean          mergedcacodemon;
extern boolean          mergednoble;

extern boolean          chex;
extern boolean          chexdeh;
extern boolean          hacx;
extern boolean          BTSX;
extern boolean          BTSXE1;
extern boolean          BTSXE2;
extern boolean          BTSXE2A;
extern boolean          BTSXE2B;

extern boolean          DMENUPIC;
extern boolean          FREEDOOM;
extern boolean          FREEDM;
extern boolean          M_DOOM;
extern boolean          M_EPISOD;
extern boolean          M_GDHIGH;
extern boolean          M_GDLOW;
extern boolean          M_LOADG;
extern boolean          M_LSCNTR;
extern boolean          M_MSENS;
extern boolean          M_MSGOFF;
extern boolean          M_MSGON;
extern boolean          M_NEWG;
extern boolean          M_NMARE;
extern boolean          M_OPTTTL;
extern boolean          M_PAUSE;
extern boolean          M_SAVEG;
extern boolean          M_SKILL;
extern boolean          M_SKULL1;
extern boolean          M_SVOL;
extern boolean          STARMS;
extern boolean          STBAR;
extern boolean          STCFN034;
extern boolean          STCFN039;
extern boolean          STCFN121;
extern boolean          STYSNUM0;
extern boolean          TITLEPIC;
extern boolean          WISCRT2;

// If non-zero, exit the level after this number of minutes
extern int              timelimit;

// Nightmare mode flag, single player.
extern boolean          respawnmonsters;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern int              deathmatch;

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

// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
// Ideally, this would use indices found
//  in: /usr/include/linux/soundcard.h
extern int              snd_MusicDevice;
extern int              snd_SfxDevice;

// -------------------------
// Status flags for refresh.
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern boolean          statusbaractive;

extern boolean          automapactive;  // In AutoMap mode?
extern boolean          followplayer;   // Following player in AutoMap mode?
extern boolean          menuactive;     // Menu overlayed?
extern boolean          paused;         // Game Pause?

extern boolean          viewactive;

// Player taking events, and displaying.
extern int              consoleplayer;
extern int              displayplayer;

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

extern boolean          usergame;

//?
extern gamestate_t      gamestate;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int              gametic;

// Bookkeeping on players - state.
extern player_t         players[MAXPLAYERS];

// Alive? Disconnected?
extern boolean          playeringame[MAXPLAYERS];

// Player spawn spots for deathmatch.
#define MAX_DM_STARTS   10
extern mapthing_t       deathmatchstarts[MAX_DM_STARTS];
extern mapthing_t       *deathmatch_p;

// Player spawn spots.
extern mapthing_t       playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t  wminfo;

//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern char             *savegamedir;
extern char             basedefault[1024];

// if true, load all graphics at level load
extern boolean          precache;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern gamestate_t      wipegamestate;

extern int              mousesensitivity;
extern int              gamepadsensitivity;
extern float            gamepadsensitivityf;

// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int              skyflatnum;

// Netgame stuff (buffers and pointers, i.e. indices).
extern int              maketic;
extern int              nettics[MAXPLAYERS];

extern ticcmd_t         netcmds[MAXPLAYERS][BACKUPTICS];
extern int              ticdup;

#endif
