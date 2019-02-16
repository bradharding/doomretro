/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

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
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__P_MOBJ_H__)
#define __P_MOBJ_H__

#include "doomdata.h"
#include "info.h"
#include "states.h"
#include "tables.h"

#define REDBLOOD            184
#define GREENBLOOD          123
#define BLUEBLOOD           204
#define FUZZYBLOOD          -1

// killough 11/98:
// For torque simulation:
#define OVERDRIVE           6
#define MAXGEAR             (OVERDRIVE + 16)

// killough 11/98:
// Whether an object is "sentient" or not. Used for environmental influences.
#define sentient(mobj)      (mobj->health > 0 && mobj->info->seestate)

#define CORPSEBLOODSPLATS   256

//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are almost always set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound emitted by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//
enum
{
    // Call P_SpecialThing when touched.
    MF_SPECIAL          = 0x00000001,
    // Blocks.
    MF_SOLID            = 0x00000002,
    // Can be hit.
    MF_SHOOTABLE        = 0x00000004,
    // Don't use the sector links (invisible but touchable).
    MF_NOSECTOR         = 0x00000008,
    // Don't use the blocklinks (inert but displayable)
    MF_NOBLOCKMAP       = 0x00000010,

    // Not to be activated by sound, deaf monster.
    MF_AMBUSH           = 0x00000020,
    // Will try to attack right back.
    MF_JUSTHIT          = 0x00000040,
    // Will take at least one step before attacking.
    MF_JUSTATTACKED     = 0x00000080,
    // On level spawning (initial position),
    //  hang from ceiling instead of stand on floor.
    MF_SPAWNCEILING     = 0x00000100,
    // Don't apply gravity (every tic),
    //  that is, object will float, keeping current height
    //  or changing it actively.
    MF_NOGRAVITY        = 0x00000200,

    // Movement flags.
    // This allows jumps from high places.
    MF_DROPOFF          = 0x00000400,
    // For players, will pick up items.
    MF_PICKUP           = 0x00000800,
    // Player cheat. ???
    MF_NOCLIP           = 0x00001000,
    // Player: keep info about sliding along walls.
    MF_SLIDE            = 0x00002000,
    // Allow moves to any height, no gravity.
    // For active floaters, e.g. cacodemons, pain elementals.
    MF_FLOAT            = 0x00004000,
    // Don't cross lines
    //   ??? or look at heights on teleport.
    MF_TELEPORT         = 0x00008000,
    // Don't hit same species, explode on block.
    // Player missiles as well as fireballs of various kinds.
    MF_MISSILE          = 0x00010000,
    // Dropped by a demon, not level spawned.
    // E.g. ammo clips dropped by dying former humans.
    MF_DROPPED          = 0x00020000,
    // Use fuzzy draw (spectres),
    //  temporary player invisibility powerup.
    MF_FUZZ             = 0x00040000,
    // Flag: don't bleed when shot (use puff),
    //  barrels and shootable furniture shall not bleed.
    MF_NOBLOOD          = 0x00080000,
    // Don't stop moving halfway off a step,
    //  that is, have dead bodies slide down all the way.
    MF_CORPSE           = 0x00100000,
    // Floating to a height for a move, ???
    //  don't auto float to target's height.
    MF_INFLOAT          = 0x00200000,

    // On kill, count this enemy object
    //  towards intermission kill total.
    // Happy gathering.
    MF_COUNTKILL        = 0x00400000,

    // On picking up, count this item object
    //  towards intermission item total.
    MF_COUNTITEM        = 0x00800000,

    // Special handling: skull in flight.
    // Neither a cacodemon nor a missile.
    MF_SKULLFLY         = 0x01000000,

    // Player sprites in multiplayer modes are modified
    //  using an internal color lookup table for re-indexing.
    // If 0x04 0x08 or 0x0C,
    //  use a translation table for player colormaps
    MF_TRANSLATION      = 0x0C000000,
    // Hmm ???.
    MF_TRANSSHIFT       = 26,

    MF_TOUCHY           = 0x10000000,   // killough 11/98: dies when solids touch it
    MF_BOUNCES          = 0x20000000,   // killough 7/11/98: for beta BFG fireballs
    MF_FRIEND           = 0x40000000,   // killough 7/18/98: friendly monsters

    // Translucent sprite?              // phares
    MF_TRANSLUCENT      = 0x80000000    // phares
};

enum
{
    // Apply additive translucency
    MF2_TRANSLUCENT               = 0x00000001,
    // Apply additive translucency on red only
    MF2_TRANSLUCENT_REDONLY       = 0x00000002,
    // Apply additive translucency on green only
    MF2_TRANSLUCENT_GREENONLY     = 0x00000004,
    // Apply additive translucency on blue only
    MF2_TRANSLUCENT_BLUEONLY      = 0x00000008,
    // Apply 33% alpha translucency
    MF2_TRANSLUCENT_33            = 0x00000010,
    // Apply 50% alpha translucency
    MF2_TRANSLUCENT_50            = 0x00000020,
    // Apply additive translucency on all red to white
    MF2_TRANSLUCENT_REDWHITEONLY  = 0x00000040,
    // Convert all red to green, then apply 33% alpha translucency
    MF2_TRANSLUCENT_REDTOGREEN_33 = 0x00000080,
    // Convert all red to blue, then apply 33% alpha translucency
    MF2_TRANSLUCENT_REDTOBLUE_33  = 0x00000100,
    // Apply 33% alpha translucency on all blue
    MF2_TRANSLUCENT_BLUE_25       = 0x00000200,

    // Convert all red to green
    MF2_REDTOGREEN                = 0x00000400,
    // Convert all green to red
    MF2_GREENTORED                = 0x00000800,
    // Convert all red to blue
    MF2_REDTOBLUE                 = 0x00001000,

    // Object bobs up and down
    MF2_FLOATBOB                  = 0x00002000,

    // Mirrored horizontally
    MF2_MIRRORED                  = 0x00004000,

    MF2_FALLING                   = 0x00008000,

    // Object is resting on top of another object
    MF2_ONMOBJ                    = 0x00010000,

    // Object is allowed to pass over/under other objects
    MF2_PASSMOBJ                  = 0x00020000,

    // Object is a corpse and being resurrected
    MF2_RESURRECTING              = 0x00040000,

    // Object's feet will be clipped in liquid
    MF2_FOOTCLIP                  = 0x00080000,

    // Object won't bob in liquid
    MF2_NOLIQUIDBOB               = 0x00100000,

    // Object's feet are now being clipped
    MF2_FEETARECLIPPED            = 0x00200000,

    // Object casts a shadow
    MF2_CASTSHADOW                = 0x00400000,

    // Object is blood
    MF2_BLOOD                     = 0x00800000,

    // Object's thing triangle is not displayed in automap
    MF2_DONTMAP                   = 0x01000000,

    // Object has smoke trail
    MF2_SMOKETRAIL                = 0x02000000,

    // Object can be crushed into blood splats by moving sectors
    MF2_CRUSHABLE                 = 0x04000000,

    // All objects are being killed
    MF2_MASSACRE                  = 0x08000000,

    // Object is a map decoration
    MF2_DECORATION                = 0x10000000,

    // Don't draw object
    MF2_DONTDRAW                  = 0x20000000,

    // Object is a missile from a monster
    MF2_MONSTERMISSILE            = 0x40000000
};

typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
} dirtype_t;

// Map Object definition.
typedef struct mobj_s
{
    // List: thinker links.
    thinker_t           thinker;

    // Info for drawing: position.
    fixed_t             x, y, z;

    // More list: links in sector (if needed)
    struct mobj_s       *snext;
    struct mobj_s       **sprev;                // killough 8/10/98: change to ptr-to-ptr

    // More drawing info: to determine current sprite.
    angle_t             angle;                  // orientation
    spritenum_t         sprite;                 // used to find patch_t and flip value
    int                 frame;                  // might be ORed with FF_FULLBRIGHT

    // Interaction info, by BLOCKMAP.
    // Links in blocks (if needed).
    struct mobj_s       *bnext;
    struct mobj_s       **bprev;                // killough 8/11/98: change to ptr-to-ptr

    struct subsector_s  *subsector;

    // The closest interval over all contacted Sectors.
    fixed_t             floorz;
    fixed_t             ceilingz;

    // killough 11/98: the lowest floor over all contacted Sectors.
    fixed_t             dropoffz;

    // For movement checking.
    fixed_t             radius;
    fixed_t             height;

    // Momentums, used to update position.
    fixed_t             momx, momy, momz;

    mobjtype_t          type;
    mobjinfo_t          *info;                  // &mobjinfo[mobj->type]

    int                 tics;                   // state tic counter
    state_t             *state;
    int                 flags;
    int                 flags2;

    int                 health;

    // Movement direction, movement generation (zig-zagging).
    dirtype_t           movedir;                // 0-7
    int                 movecount;              // when 0, select a new dir

    // Thing being chased/attacked (or NULL),
    // also the originator for missiles.
    struct mobj_s       *target;

    // Reaction time: if non 0, don't attack yet.
    // Used by player to freeze a bit after teleporting.
    int                 reactiontime;

    // If >0, the target will be chased
    // no matter what (even if shot)
    int                 threshold;

    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    struct player_s     *player;

    // For nightmare respawn.
    mapthing_t          spawnpoint;

    // Thing being chased/attacked for tracers.
    struct mobj_s       *tracer;

    // new field: last known enemy -- killough 2/15/98
    struct mobj_s       *lastenemy;

    // For bobbing up and down.
    int                 floatbob;

    void                (*colfunc)(void);
    void                (*altcolfunc)(void);
    void                (*shadowcolfunc)(void);

    int                 shadowoffset;

    // a linked list of sectors where this object appears
    struct msecnode_s   *touching_sectorlist;   // phares 3/14/98

    short               gear;                   // killough 11/98: used in torque simulation

    int                 bloodsplats;

    int                 blood;

    // [AM] If true, ok to interpolate this tic.
    int                 interpolate;

    // [AM] Previous position of mobj before think.
    //      Used to interpolate between positions.
    fixed_t             oldx, oldy, oldz;
    angle_t             oldangle;

    fixed_t             nudge;

    int                 pitch;

    int                 id;
    int                 musicid;
} mobj_t;

typedef struct bloodsplat_s
{
    fixed_t             x, y;
    struct bloodsplat_s *snext;
    struct bloodsplat_s **sprev;
    int                 patch;
    fixed_t             width;
    struct sector_s     *sector;
    dboolean            flip;
    int                 blood;
    void                (*colfunc)(void);
} bloodsplat_t;

#endif
