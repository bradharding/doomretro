/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

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

#if !defined(__DOOMDEF_H__)
#define __DOOMDEF_H__

#include <stdio.h>

#include "m_controls.h"

#if defined(_WIN32)
#define snprintf        _snprintf
#if (_MSC_VER < 1400)
#define vsnprintf       _vsnprintf
#endif
#define strcasecmp      stricmp
#define strncasecmp     strnicmp
#else
#include <strings.h>
#endif

//
// The packed attribute forces structures to be packed into the minimum
// space necessary. If this is not done, the compiler may align structure
// fields differently to optimize memory access, inflating the overall
// structure size. It is important to use the packed attribute on certain
// structures where alignment is important, particularly data read/written
// to disk.
//
#if defined(__GNUC__)

#if defined(__clang__)
#define PACKEDATTR  __attribute__((packed))
#else
#define PACKEDATTR  __attribute__((packed,gcc_struct))
#endif

#else
#define PACKEDATTR
#endif

//
// Global parameters/defines.
//

// Game mode handling - identify IWAD version
//  to handle IWAD dependent animations etc.
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
    none
} GameMission_t;

// What version are we emulating?
typedef enum
{
    exe_doom_1_9,       // DOOM 1.9: used for shareware, registered and commercial
    exe_ultimate,       // Ultimate DOOM (retail)
    exe_final           // Final DOOM
} GameVersion_t;

// Screen width and height.
#define ORIGINALWIDTH       320
#define ORIGINALHEIGHT      200

#define ORIGINALSBARHEIGHT  32

#define SCREENSCALE         2

#define SCREENWIDTH         (ORIGINALWIDTH * SCREENSCALE)
#define SCREENHEIGHT        (ORIGINALHEIGHT * SCREENSCALE)

#define SBARHEIGHT          (ORIGINALSBARHEIGHT * SCREENSCALE)

// State updates, number of tics / second.
#define TICRATE             35

// The current state of the game: whether we are
// playing, gazing at the intermission screen,
// the game final animation, or title screen.
typedef enum
{
    GS_NONE = -1,
    GS_LEVEL,
    GS_INTERMISSION,
    GS_FINALE,
    GS_TITLESCREEN
} gamestate_t;

//
// Difficulty/skill settings/filters.
//

// Skill flags.
#define MTF_EASY    1
#define MTF_NORMAL  2
#define MTF_HARD    4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH  8

#define MTF_NETGAME 16

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
enum
{
    pw_invulnerability,
    pw_strength,
    pw_invisibility,
    pw_ironfeet,
    pw_allmap,
    pw_infrared,
    NUMPOWERS
};

//
// Power up durations,
//  how many seconds till expiration,
//  assuming TICRATE is 35 ticks/second.
//
enum
{
    INVULNTICS =  30 * TICRATE,
    INVISTICS  =  60 * TICRATE,
    INFRATICS  = 120 * TICRATE,
    IRONTICS   =  60 * TICRATE
};

#define STARTFLASHING           127

// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP         32

// phares 3/20/98:
//
// Player friction is variable, based on controlling
// linedefs. More friction can create mud, sludge,
// magnetized floors, etc. Less friction can create ice.
#define MORE_FRICTION_MOMENTUM  15000   // mud factor based on momentum
#define ORIG_FRICTION           0xE800  // original value
#define ORIG_FRICTION_FACTOR    2048    // original value

#endif
