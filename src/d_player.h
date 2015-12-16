/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

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

#if !defined(__D_PLAYER__)
#define __D_PLAYER__

// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "d_items.h"
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"

//
// Player states.
//
typedef enum
{
    // Playing or camping.
    PST_LIVE,
    // Dead on the ground, view follows killer.
    PST_DEAD,
    // Ready to restart/respawn???
    PST_REBORN
} playerstate_t;

//
// Player internal flags, for cheats and debug.
//
typedef enum
{
    // No clipping, walk through barriers.
    CF_NOCLIP           = 1,
    // No damage, no health loss.
    CF_GODMODE          = 2,

    CF_NOTARGET         = 4,

    CF_MYPOS            = 8,

    CF_ALLMAP           = 16,

    CF_ALLMAP_THINGS    = 32,

    CF_CHOPPERS         = 64
} cheat_t;

//
// Extended player object info: player_t
//
typedef struct player_s
{
    mobj_t              *mo;
    playerstate_t       playerstate;
    ticcmd_t            cmd;

    // Determine POV,
    //  including viewpoint bobbing during movement.
    // Focal origin above r.z
    fixed_t             viewz;
    // Base height above floor for viewz.
    fixed_t             viewheight;
    // Bob/squat speed.
    fixed_t             deltaviewheight;
    // bounded/scaled total momentum.
    fixed_t             bob;

    // killough 10/98: used for realistic bobbing (i.e. not simply overall speed)
    // mo->momx and mo->momy represent true momenta experienced by player.
    // This only represents the thrust that the player applies himself.
    // This avoids anomalies with such things as BOOM ice and conveyors.
    fixed_t             momx, momy;     // killough 10/98

    // This is only used between levels,
    // mo->health is used during levels.
    int                 health;
    int                 armorpoints;
    // Armor type is 0-2.
    int                 armortype;

    // Power ups. invinc and invis are tic counters.
    int                 powers[NUMPOWERS];
    int                 cards[NUMCARDS];
    int                 neededcard;
    int                 neededcardflash;
    dboolean            backpack;

    weapontype_t        readyweapon;

    // Is wp_nochange if not changing.
    weapontype_t        pendingweapon;

    int                 weaponowned[NUMWEAPONS];
    int                 ammo[NUMAMMO];
    int                 maxammo[NUMAMMO];

    // True if button down last tic.
    int                 attackdown;
    int                 usedown;

    // Bit flags, for cheats and debug.
    // See cheat_t, above.
    int                 cheats;

    // Refired shots are less accurate.
    int                 refire;

    // For intermission stats.
    int                 killcount;
    int                 itemcount;
    int                 secretcount;

    // Hint messages.
    char                *message;
    char                *altmsg;

    // For screen flashing (red or bright).
    int                 damagecount;
    int                 bonuscount;

    // Who did damage (NULL for floors/ceilings).
    mobj_t              *attacker;

    // So gun flashes light up areas.
    int                 extralight;

    // Current PLAYPAL, ???
    //  can be set to REDCOLORMAP for pain, etc.
    int                 fixedcolormap;

    // Player skin colorshift,
    //  0-3 for which color to draw player.
    int                 colormap;

    // Overlay view sprites (gun, etc).
    pspdef_t            psprites[NUMPSPRITES];

    // True if secret level has been done.
    dboolean            didsecret;

    weapontype_t        preferredshotgun;
    int                 shotguns;
    weapontype_t        fistorchainsaw;
    dboolean            invulnbeforechoppers;
    dboolean            chainsawbeforechoppers;
    weapontype_t        weaponbeforechoppers;

    // [AM] Previous position of viewz before think.
    //      Used to interpolate between camera positions.
    angle_t             oldviewz;

    // For playerstats cmd
    int                 damageinflicted;
    int                 damagereceived;
    int                 cheated;
    int                 shotshit;
    int                 shotsfired;
    int                 deaths;
    int                 mobjcount[NUMMOBJTYPES];
} player_t;

//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
    // Player stats, kills, collected items etc.
    int                 skills;
    int                 sitems;
    int                 ssecret;
    int                 stime;
    int                 score;          // current score on entry, modified on return
} wbplayerstruct_t;

typedef struct
{
    int                 epsd;           // episode # (0-2)

    // if true, splash the secret level
    dboolean            didsecret;

    // previous and next levels, origin 0
    int                 last;
    int                 next;

    int                 maxkills;
    int                 maxitems;
    int                 maxsecret;

    // the par time
    int                 partime;

    // index of this player in game
    int                 pnum;

    wbplayerstruct_t    plyr[MAXPLAYERS];
} wbstartstruct_t;

#endif
