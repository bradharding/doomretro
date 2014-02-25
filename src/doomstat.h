/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

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

extern  boolean         devparm;        // DEBUG: launched with -devparm



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

extern boolean          MAPINFO;
extern boolean          M_EPISOD;
extern boolean          M_LGTTL;
extern boolean          M_MSENS;
extern boolean          M_MSGOFF;
extern boolean          M_MSGON;
extern boolean          M_NEWG;
extern boolean          M_NMARE;
extern boolean          M_OPTTTL;
extern boolean          M_SGTTL;
extern boolean          M_SKILL;
extern boolean          M_SKULL1;
extern boolean          M_SVOL;
extern boolean          STBAR;
extern boolean          STCFN034;
extern boolean          STCFN039;
extern boolean          STYSNUM0;
extern boolean          TITLEPIC;
extern boolean          WISCRT2;

// If non-zero, exit the level after this number of minutes
extern int              timelimit;

// Nightmare mode flag, single player.
extern boolean          respawnmonsters;

// Netgame? Only true if >1 player.
extern boolean          netgame;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern boolean          deathmatch;

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
// Config file? Same disclaimer as above.
extern int              snd_DesiredMusicDevice;
extern int              snd_DesiredSfxDevice;


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



// --------------------------------------
// DEMO playback/recording related stuff.
// No demo, there is a human player in charge?
// Disable save/end game?
extern boolean          usergame;

//?
extern boolean          demoplayback;
extern boolean          demorecording;

// Round angleturn in ticcmds to the nearest 256.  This is used when
// recording Vanilla demos in netgames.

//extern boolean lowres_turn;

// Quit after playing a demo from cmdline.
extern boolean          singledemo;




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

extern int              mouseSensitivity;
extern float            gamepadSensitivity;



// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int              skyflatnum;



// Netgame stuff (buffers and pointers, i.e. indices).


extern int              rndindex;

extern int              maketic;
extern int              nettics[MAXPLAYERS];

extern ticcmd_t         netcmds[MAXPLAYERS][BACKUPTICS];
extern int              ticdup;



#endif