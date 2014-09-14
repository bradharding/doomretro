/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __DOOMDEF__
#define __DOOMDEF__

#include <stdio.h>

#include "doomkeys.h"

#ifdef WIN32
#define snprintf _snprintf
#if _MSC_VER < 1400
#define vsnprintf _vsnprintf
#endif
#define strcasecmp stricmp
#define strncasecmp strnicmp
#else
#include <strings.h>
#endif

//
// The packed attribute forces structures to be packed into the minimum
// space necessary.  If this is not done, the compiler may align structure
// fields differently to optimize memory access, inflating the overall
// structure size.  It is important to use the packed attribute on certain
// structures where alignment is important, particularly data read/written
// to disk.
//
#ifdef __GNUC__
#define PACKEDATTR __attribute__((packed))
#else
#define PACKEDATTR
#endif

//
// Global parameters/defines.
//

// Game mode handling - identify IWAD version
//  to handle IWAD dependend animations etc.
typedef enum
{
    shareware,          // DOOM 1 shareware, E1, M9
    registered,         // DOOM 1 registered, E3, M27
    commercial,         // DOOM 2 retail, E1 M34
    retail,             // DOOM 1 retail, E4, M36
    indetermined        // Well, no IWAD found.
} GameMode_t;

// Mission packs - might be useful for TC stuff?
typedef enum
{
    doom,               // DOOM 1
    doom2,              // DOOM 2
    pack_tnt,           // TNT mission pack
    pack_plut,          // Plutonia pack
    pack_nerve,         // No Rest for the Living
    doom2bfg,           // DOOM 2 (BFG Edition)
    none
} GameMission_t;

// What version are we emulating?
typedef enum
{
    exe_doom_1_9,       // Doom 1.9: used for shareware, registered and commercial
    exe_ultimate,       // Ultimate Doom (retail)
    exe_final           // Final Doom
} GameVersion_t;

// Screen width and height.

#define ORIGINALWIDTH   320
#define ORIGINALHEIGHT  200

#define SCREENSCALE     2

#define SCREENWIDTH     (ORIGINALWIDTH * SCREENSCALE)
#define SCREENHEIGHT    (ORIGINALHEIGHT * SCREENSCALE)

#define SBARHEIGHT      (32 * SCREENSCALE)

// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS      4

// State updates, number of tics / second.
#define TICRATE         35

// The current state of the game: whether we are
// playing, gazing at the intermission screen,
// the game final animation, or title screen.
typedef enum
{
    GS_LEVEL,
    GS_INTERMISSION,
    GS_FINALE,
    GS_TITLESCREEN
} gamestate_t;

//
// Difficulty/skill settings/filters.
//

// Skill flags.
#define MTF_EASY        1
#define MTF_NORMAL      2
#define MTF_HARD        4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH      8

#define MTF_NETGAME     16

typedef enum
{
    sk_baby,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare
} skill_t;

//
// Key cards.
//
typedef enum
{
    it_bluecard,
    it_yellowcard,
    it_redcard,
    it_blueskull,
    it_yellowskull,
    it_redskull,

    NUMCARDS
} card_t;

// The defined weapons,
//  including a marker indicating
//  user has not changed weapon.
typedef enum
{
    wp_fist,
    wp_pistol,
    wp_shotgun,
    wp_chaingun,
    wp_missile,
    wp_plasma,
    wp_bfg,
    wp_chainsaw,
    wp_supershotgun,

    NUMWEAPONS,

    // No pending weapon change.
    wp_nochange
} weapontype_t;

// Ammunition types defined.
typedef enum
{
    am_clip,    // Pistol / chaingun ammo.
    am_shell,   // Shotgun / double barreled shotgun.
    am_cell,    // Plasma rifle, BFG.
    am_misl,    // Missile launcher.
    NUMAMMO,
    am_noammo   // Unlimited for chainsaw / fist.
} ammotype_t;

// Power up artifacts.
typedef enum
{
    pw_invulnerability,
    pw_strength,
    pw_invisibility,
    pw_ironfeet,
    pw_allmap,
    pw_infrared,
    NUMPOWERS
} powertype_t;

//
// Power up durations,
//  how many seconds till expiration,
//  assuming TICRATE is 35 ticks/second.
//
typedef enum
{
    INVULNTICS  = (30 * TICRATE),
    INVISTICS   = (60 * TICRATE),
    INFRATICS   = (120 * TICRATE),
    IRONTICS    = (60 * TICRATE)
} powerduration_t;

#define STARTFLASHING   127

// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP 32

#endif
