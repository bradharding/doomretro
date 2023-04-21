/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <math.h>
#include <ctype.h>

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_setup.h"
#include "s_sound.h"
#include "v_video.h"
#include "z_zone.h"

static mobj_t   *tmthing;
static fixed_t  tmx, tmy, tmz;
static int      pe_x, pe_y;     // Pain Elemental position for Lost Soul checks // phares
static int      ls_x, ls_y;     // Lost Soul position for Lost Soul checks      // phares

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
bool            floatok;

// killough 11/98: if "felldown" true, object was pushed down ledge
bool            felldown;

fixed_t         tmbbox[4];
fixed_t         tmfloorz;
static fixed_t  tmceilingz;
static fixed_t  tmdropoffz;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t          *ceilingline;
line_t          *blockline;     // killough 08/11/98: blocking linedef
static line_t   *floorline;     // killough 08/01/98: Highest touched floor
static int      tmunstuck;      // killough 08/01/98: whether to allow unsticking

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// killough 01/11/98: removed limit on special lines crossed
line_t          **spechit;
int             numspechit = 0;

static angle_t  shootangle;     // [BH] angle of blood and puffs for automap

// Temporary holder for thing_sectorlist threads
msecnode_t      *sector_list;   // phares 03/16/98

bool            infight;

static mobj_t   *onmobj;

//
// TELEPORT MOVE
//

//
// PIT_StompThing
//
static bool telefrag;   // killough 08/09/98: whether to telefrag at exit

static bool PIT_StompThing(mobj_t *thing)
{
    fixed_t blockdist;

    // phares 09/10/98: moved this self-check to start of routine
    // don't clip against self
    if (thing == tmthing)
        return true;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (ABS(thing->x - tmx) >= blockdist || ABS(thing->y - tmy) >= blockdist)
        return true;        // didn't hit it

    // monsters don't stomp things except on boss level
    if (!telefrag)          // killough 08/09/98: make consistent across all levels
        return false;

    if (((tmthing->flags2 & MF2_PASSMOBJ) || (thing->flags2 & MF2_PASSMOBJ))
        && !infiniteheight && !compat_nopassover)
    {
        if (tmz > thing->z + thing->height)
            return true;    // overhead

        if (tmz + tmthing->height < thing->z)
            return true;    // underneath
    }

    P_DamageMobj(thing, NULL, tmthing, 10000, true, true);   // Stomp!

    return true;
}

//
// killough 08/28/98:
//
// P_GetFriction
//
// Returns the friction associated with a particular mobj.
int P_GetFriction(const mobj_t *mo, int *frictionfactor)
{
    int friction = ORIG_FRICTION;
    int movefactor = ORIG_FRICTION_FACTOR;

    // Assign the friction value to objects on the floor, non-floating,
    // and clipped. Normally the object's friction value is kept at
    // ORIG_FRICTION and this thinker changes it for icy or muddy floors.
    //
    // When the object is straddling sectors with the same
    // floorheight that have different frictions, use the lowest
    // friction value (muddy has precedence over icy).
    if (!(mo->flags & (MF_NOCLIP | MF_NOGRAVITY)) && !freeze)
        for (const msecnode_t *m = mo->touching_sectorlist; m; m = m->m_tnext)
        {
            const sector_t  *sec = m->m_sector;

            if ((sec->special & FRICTION_MASK)
                && (sec->friction < friction || friction == ORIG_FRICTION)
                && (mo->z <= sec->floorheight || (sec->heightsec && mo->z <= sec->heightsec->floorheight)))
            {
                friction = sec->friction;
                movefactor = sec->movefactor;
            }
        }

    if (frictionfactor)
        *frictionfactor = movefactor;

    return friction;
}

// phares 03/19/98
// P_GetMoveFactor() returns the value by which the x,y
// movements are multiplied to add to player movement.
//
// killough 08/28/98: rewritten
int P_GetMoveFactor(const mobj_t *mo, int *frictionp)
{
    int         movefactor;
    const int   friction = P_GetFriction(mo, &movefactor);

    // If the floor is icy or muddy, it's harder to get moving. This is where
    // the different friction factors are applied to 'trying to move'. In
    // p_mobj.c, the friction factors are applied as you coast and slow down.
    if (friction < ORIG_FRICTION)
    {
        // phares 03/11/98: you start off slowly, then increase as
        // you get better footing
        const int   momentum = P_ApproxDistance(mo->momx, mo->momy);

        if (momentum > (MORE_FRICTION_MOMENTUM << 2))
            movefactor <<= 3;
        else if (momentum > (MORE_FRICTION_MOMENTUM << 1))
            movefactor <<= 2;
        else if (momentum > MORE_FRICTION_MOMENTUM)
            movefactor <<= 1;
    }

    if (frictionp)
        *frictionp = friction;

    return movefactor;
}

//
// P_TeleportMove
//
bool P_TeleportMove(mobj_t *thing, const fixed_t x, const fixed_t y, const fixed_t z, const bool boss)
{
    int             xl;
    int             xh;
    int             yl;
    int             yh;
    const sector_t  *newsec;
    const fixed_t   radius = thing->radius;

    telefrag = (thing->player || boss || P_GetAllowMonsterTelefrags((gameepisode - 1) * 10 + gamemap));

    // kill anything occupying the position
    tmthing = thing;

    tmx = x;
    tmy = y;
    tmz = z;

    tmbbox[BOXTOP] = y + radius;
    tmbbox[BOXBOTTOM] = y - radius;
    tmbbox[BOXRIGHT] = x + radius;
    tmbbox[BOXLEFT] = x - radius;

    newsec = R_PointInSubsector(x, y)->sector;
    ceilingline = NULL;

    // The base floor/ceiling is from the subsector that contains the point.
    // Any contacted lines the step closer together will adjust them.
    tmfloorz = tmdropoffz = newsec->floorheight;
    tmceilingz = newsec->ceilingheight;

    validcount++;
    numspechit = 0;

    // stomp on any things contacted
    xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS);
    xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS);
    yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS);
    yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS);

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, &PIT_StompThing))
                return false;

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition(thing);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;   // killough 11/98

    thing->x = x;
    thing->y = y;

    // [AM] Don't interpolate mobjs that pass through teleporters
    thing->interpolate = 0;

    P_SetThingPosition(thing);

    thing->z = z;

    // [BH] check if new sector is liquid and clip/unclip feet as necessary
    if ((thing->flags2 & MF2_FOOTCLIP) && P_IsInLiquid(thing))
        thing->flags2 |= MF2_FEETARECLIPPED;
    else
        thing->flags2 &= ~MF2_FEETARECLIPPED;

    return true;
}

//
// MOVEMENT ITERATOR FUNCTIONS
//

//
// PIT_CrossLine
// Checks to see if a PE->LS trajectory line crosses a blocking
// line. Returns false if it does.
//
// tmbbox holds the bounding box of the trajectory. If that box
// does not touch the bounding box of the line in question,
// then the trajectory is not blocked. If the PE is on one side
// of the line and the LS is on the other side, then the
// trajectory is blocked.
//
// Currently this assumes an infinite line, which is not quite
// correct. A more correct solution would be to check for an
// intersection of the trajectory and the line, but that takes
// longer and probably really isn't worth the effort.
//
// [BH] Allow pain elementals to shoot lost souls through 2-sided walls with an ML_BLOCKMONSTERS
//  flag. This is a compromise between BOOM and Vanilla DOOM behaviors, and allows pain elementals
//  at the end of REQUIEM.WAD's MAP04 to do their thing.
static bool PIT_CrossLine(line_t *ld)
{
    if (!(ld->flags & ML_TWOSIDED) || (ld->flags & (ML_BLOCKING/* | ML_BLOCKMONSTERS*/)))
        if (!(tmbbox[BOXLEFT] > ld->bbox[BOXRIGHT]
            || tmbbox[BOXRIGHT] < ld->bbox[BOXLEFT]
            || tmbbox[BOXTOP] < ld->bbox[BOXBOTTOM]
            || tmbbox[BOXBOTTOM] > ld->bbox[BOXTOP]))
            if (P_PointOnLineSide(pe_x, pe_y, ld) != P_PointOnLineSide(ls_x, ls_y, ld))
                return false;   // line blocks trajectory

    return true;    // line doesn't block trajectory
}

// killough 08/01/98: used to test intersection between thing and line
// assuming NO movement occurs -- used to avoid sticky situations.
static int untouched(line_t *ld)
{
    fixed_t         x, y;
    fixed_t         bbox[4] = { 0 };
    const fixed_t   tmradius = tmthing->radius;

    return ((bbox[BOXRIGHT] = (x = tmthing->x) + tmradius) <= ld->bbox[BOXLEFT]
        || (bbox[BOXLEFT] = x - tmradius) >= ld->bbox[BOXRIGHT]
        || (bbox[BOXTOP] = (y = tmthing->y) + tmradius) <= ld->bbox[BOXBOTTOM]
        || (bbox[BOXBOTTOM] = y - tmradius) >= ld->bbox[BOXTOP]
        || P_BoxOnLineSide(bbox, ld) != -1);
}

void P_CheckSpechits(void)
{
    static int  spechit_max;

    // killough 01/11/98: remove limit on lines hit, by array doubling
    if (numspechit >= spechit_max)
    {
        spechit_max = (spechit_max ? spechit_max * 2 : 8);
        spechit = I_Realloc(spechit, spechit_max * sizeof(*spechit));
    }
}

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
static bool PIT_CheckLine(line_t *ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
        return true;    // didn't hit it

    if (P_BoxOnLineSide(tmbbox, ld) != -1)
        return true;    // didn't hit it

    // A line has been hit

    // The moving thing's destination position will cross the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.

    // killough 07/24/98: allow player to move out of 1s wall, to prevent sticking
    if (!ld->backsector)    // one sided line
    {
        blockline = ld;
        return (tmunstuck && !untouched(ld) && FixedMul(tmx - tmthing->x, ld->dy) > FixedMul(tmy - tmthing->y, ld->dx));
    }

    // killough 08/10/98: allow bouncing objects to pass through as missiles
    if (!(tmthing->flags & (MF_MISSILE | MF_BOUNCES)))
    {
        // explicitly blocking everything
        // or blocking player
        if ((ld->flags & ML_BLOCKING) || (tmthing->player && (ld->flags & ML_BLOCKPLAYERS)))
            return (tmunstuck && !untouched(ld));   // killough 08/01/98: allow escape

        // killough 08/09/98: monster-blockers don't affect friends
        // MBF21: Block land monsters
        // [BH] monster-blockers don't affect corpses
        if (!((tmthing->flags & MF_FRIEND) || tmthing->player || (tmthing->flags2 & MF2_SPAWNEDBYPLAYER))
            && ((ld->flags & ML_BLOCKMONSTERS) || ((ld->flags & ML_BLOCKLANDMONSTERS) && !(tmthing->flags & MF_FLOAT)))
            && !(tmthing->flags & MF_CORPSE))
            return false;   // block monsters only
    }

    // set openrange, opentop, openbottom
    // these define a 'window' from one sector to another across this line
    P_LineOpening(ld);

    // adjust floor/ceiling heights
    if (opentop < tmceilingz)
    {
        tmceilingz = opentop;
        ceilingline = ld;
        blockline = ld;
    }

    if (openbottom > tmfloorz)
    {
        tmfloorz = openbottom;
        floorline = ld; // killough 08/01/98: remember floor linedef
        blockline = ld;
    }

    if (lowfloor < tmdropoffz)
        tmdropoffz = lowfloor;

    // if contacted a special line, add it to the list
    if (ld->special)
    {
        P_CheckSpechits();
        spechit[numspechit++] = ld;
    }

    return true;
}

// MBF21: dehacked projectile groups
static bool P_ProjectileImmune(mobj_t *target, mobj_t *source)
{
    const int   projectilegroup = mobjinfo[target->type].projectilegroup;

    // PG_GROUPLESS means no immunity, even to own species
    return ((projectilegroup != PG_GROUPLESS || target == source)
        // target type has default behavior, and things are the same type
        && ((projectilegroup == PG_DEFAULT && source->type == target->type)
            // target type has special behavior, and things have the same group
            || (projectilegroup != PG_DEFAULT && projectilegroup == mobjinfo[source->type].projectilegroup)));
}

//
// PIT_CheckThing
//
static bool PIT_CheckThing(mobj_t *thing)
{
    fixed_t             blockdist;
    bool                unblocking = false;
    const int           flags = thing->flags;
    const int           tmflags = tmthing->flags;
    const bool          corpse = (flags & MF_CORPSE);
    const mobjtype_t    type = thing->type;
    const mobjtype_t    tmtype = tmthing->type;

    // don't clip against self (can happen with corpses)
    if (thing == tmthing)
        return true;

    // killough 11/98: add touchy things
    if (!(flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE | MF_TOUCHY)))
        return true;

    // [BH] nudge corpse or dropped item when walked over
    if (((corpse && type != MT_BARREL) || (flags & MF_DROPPED)) && !thing->nudge && thing->floorz == tmthing->floorz
        && ((tmflags & MF_SHOOTABLE) || ((tmflags & MF_CORPSE) && (tmthing->momx || tmthing->momy))) && r_corpses_nudge)
        if (P_ApproxDistance(thing->x - tmthing->x, thing->y - tmthing->y) < 16 * FRACUNIT)
        {
            const int   r = M_RandomInt(-1, 1);

            thing->momx += r * FRACUNIT;
            thing->momy += (!r ? M_RandomIntNoRepeat(-1, 1, 0) : M_RandomInt(-1, 1)) * FRACUNIT;
            thing->nudge = TICRATE;

            if (!(thing->flags2 & MF2_FEETARECLIPPED))
            {
                thing->momx /= 2;
                thing->momy /= 2;
            }
        }

    // [BH] specify standard radius of 20 for pickups here as thing->radius
    // has been changed to allow better clipping
    blockdist = ((flags & MF_SPECIAL) ? thing->info->pickupradius : thing->info->radius) + tmthing->radius;

    if (ABS(thing->x - tmx) >= blockdist || ABS(thing->y - tmy) >= blockdist)
        return true;                    // didn't hit it

    // killough 11/98:
    // TOUCHY flag, for mines or other objects which die on contact with solids.
    // If a solid object of a different type comes in contact with a touchy
    // thing, and the touchy thing is not the sole one moving relative to fixed
    // surroundings such as walls, then the touchy thing dies immediately.
    if ((flags & MF_TOUCHY)                                             // touchy object
        && (tmflags & MF_SOLID)                                         // solid object touches it
        && thing->health > 0                                            // touchy object is alive
        && ((thing->flags2 & MF2_ARMED)                                 // Thing is an armed mine
            || sentient(thing))                                         // ...or a sentient thing
        && (type != tmtype                                              // only different species
            || type == MT_PLAYER)                                       // ...or different players
        && thing->z + thing->height >= tmthing->z                       // touches vertically
        && tmthing->z + tmthing->height >= thing->z
        && ((type ^ MT_PAIN) | (tmtype ^ MT_SKULL))                     // PEs and lost souls are considered same
        && ((type ^ MT_SKULL) | (tmtype ^ MT_PAIN)))                    // (but Barons and Knights are intentionally not)
    {
        P_DamageMobj(thing, NULL, NULL, thing->health, true, false);    // kill object
        return true;
    }

    // [BH] check if things are stuck and allow move if it makes them further apart
    if (!thing->player && !corpse && (flags & MF_SHOOTABLE) && (tmflags & MF_SHOOTABLE) && type != MT_BARREL && tmtype != MT_BARREL)
    {
        if (tmx == tmthing->x && tmy == tmthing->y)
            unblocking = true;
        else if (P_ApproxDistance(thing->x - tmx, thing->y - tmy) > P_ApproxDistance(thing->x - tmthing->x, thing->y - tmthing->y))
            unblocking = (tmthing->z < thing->z + thing->height && tmthing->z + tmthing->height > thing->z);
    }

    // check if a mobj passed over/under another object
    if (((tmthing->flags2 & MF2_PASSMOBJ) || (thing->flags2 & MF2_PASSMOBJ))
        && !infiniteheight && !compat_nopassover && !(flags & MF_SPECIAL))
    {
        if (tmthing->z >= thing->z + thing->height)
            return true;    // over thing
        else if (tmthing->z + tmthing->height <= thing->z)
            return true;    // under thing
    }

    // check for skulls slamming into things
    if ((tmflags & MF_SKULLFLY) && ((flags & MF_SOLID) || infiniteheight || compat_nopassover))
    {
        P_DamageMobj(thing, tmthing, tmthing, ((M_Random() & 7) + 1) * tmthing->info->damage, true, false);

        tmthing->flags &= ~MF_SKULLFLY;
        tmthing->momx = 0;
        tmthing->momy = 0;
        tmthing->momz = 0;

        P_SetMobjState(tmthing, tmthing->info->spawnstate);

        return false;   // stop moving
    }

    // missiles can hit other things
    // killough 08/10/98: bouncing non-solid things can hit other things too
    if ((tmflags & MF_MISSILE) || ((tmflags & MF_BOUNCES) && !(tmflags & MF_SOLID)))
    {
        int height = thing->info->projectilepassheight;

        if (!height || infiniteheight || compat_nopassover)
            height = thing->height;

        // see if it went over/under
        if (tmthing->z > thing->z + height)
            return true;    // overhead

        if (tmthing->z + tmthing->height < thing->z)
            return true;    // underneath

        if (tmthing->target && P_ProjectileImmune(thing, tmthing->target))
        {
            // Don't hit same species as originator.
            if (thing == tmthing->target)
                return true;
            else if (!infight && !species_infighting)
                return false;   // Explode, but do no damage.
        }

        // killough 08/10/98: if moving thing is not a missile, no damage
        // is inflicted, and momentum is reduced if object hit is solid.
        if (!(tmflags & MF_MISSILE))
        {
            if (!(flags & MF_SOLID))
                return true;

            tmthing->momx = -tmthing->momx;
            tmthing->momy = -tmthing->momy;

            if (!(tmflags & MF_NOGRAVITY))
            {
                tmthing->momx >>= 2;
                tmthing->momy >>= 2;
            }

            return false;
        }

        if (!(flags & MF_SHOOTABLE))
            return !(flags & MF_SOLID); // didn't do any damage

        // MBF21: ripper projectile
        if (tmthing->mbf21flags & MF_MBF21_RIP)
        {
            const int   damage = ((M_Random() & 3) + 2) * tmthing->info->damage;

            if (r_blood != r_blood_none)
            {
                if (thing->player)
                {
                    if (!viewplayer->powers[pw_invulnerability] && !(viewplayer->cheats & CF_GODMODE))
                        P_SpawnBlood(tmthing->x, tmthing->y, tmthing->z, shootangle, damage, tmthing);
                }
                else if (!(thing->flags & MF_NOBLOOD))
                    P_SpawnBlood(tmthing->x, tmthing->y, tmthing->z, shootangle, damage, tmthing);
            }

            if (tmthing->info->ripsound)
                S_StartSound(tmthing, tmthing->info->ripsound);

            P_DamageMobj(thing, tmthing, tmthing->target, damage, true, false);
            numspechit = 0;

            return true;
        }

        // damage/explode
        P_DamageMobj(thing, tmthing, tmthing->target, ((M_Random() & 7) + 1) * tmthing->info->damage, true, false);

        if (type != MT_BARREL)
        {
            if (tmtype == MT_PLASMA)
            {
                viewplayer->shotssuccessful[wp_plasma]++;
                stat_shotssuccessful_plasmarifle = SafeAdd(stat_shotssuccessful_plasmarifle, 1);
            }
            else if (tmtype == MT_ROCKET)
            {
                if (tmthing->nudge == 1)
                {
                    viewplayer->shotssuccessful[wp_missile]++;
                    stat_shotssuccessful_rocketlauncher = SafeAdd(stat_shotssuccessful_rocketlauncher, 1);
                }

                tmthing->nudge++;
            }
        }

        // don't traverse anymore
        return false;
    }

    // check for special pickup
    if (flags & MF_SPECIAL)
    {
        if (tmflags & MF_PICKUP)
            P_TouchSpecialThing(thing, tmthing, (tmthing->player->mo == tmthing), true);

        return !(flags & MF_SOLID);
    }

    // [BH] don't hit if either thing is a corpse, which may still be solid if
    // they are still going through their death sequence.
    if (!(thing->flags2 & MF2_RESURRECTING) && (corpse || (tmflags & MF_CORPSE)) && type != MT_BARREL)
        return true;

    // RjY
    // an attempt to handle blocking hanging bodies
    // A solid hanging body will allow sufficiently small things underneath it.
    if (!((~flags) & (MF_SOLID | MF_SPAWNCEILING))      // solid and hanging
        // invert everything, then both bits should be clear
        && tmthing->z + tmthing->height <= thing->z)    // head height <= base
        // top of thing trying to move under the body <= bottom of body
    {
        tmceilingz = thing->z;   // pretend ceiling height is at body's base
        return true;
    }

    // killough 03/16/98: Allow non-solid moving objects to move through solid
    // ones, by allowing the moving thing (tmthing) to move if it's non-solid,
    // despite another solid thing being in the way.
    // killough 04/11/98: Treat no-clipping things as not blocking
    return (!((flags & MF_SOLID) && !(flags & MF_NOCLIP) && !freeze && (tmflags & MF_SOLID)) || unblocking);
}

//
// P_CheckLineSide
//
// This routine checks for Lost Souls trying to be spawned
// across 1-sided lines, impassible lines, or "monsters can't
// cross" lines. Draw an imaginary line between the PE
// and the new Lost Soul spawn spot. If that line crosses
// a 'blocking' line, then disallow the spawn. Only search
// lines in the blocks of the blockmap where the bounding box
// of the trajectory line resides. Then check bounding box
// of the trajectory vs. the bounding box of each blocking
// line to see if the trajectory and the blocking line cross.
// Then check the PE and LS to see if they're on different
// sides of the blocking line. If so, return true, otherwise
// false.
bool P_CheckLineSide(mobj_t *actor, const fixed_t x, const fixed_t y)
{
    int xl;
    int xh;
    int yl;
    int yh;

    pe_x = actor->x;
    pe_y = actor->y;
    ls_x = x;
    ls_y = y;

    // here is the bounding box of the trajectory
    tmbbox[BOXLEFT] = MIN(pe_x, x);
    tmbbox[BOXRIGHT] = MAX(pe_x, x);
    tmbbox[BOXTOP] = MAX(pe_y, y);
    tmbbox[BOXBOTTOM] = MIN(pe_y, y);

    // determine which blocks to look in for blocking lines
    xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx);
    xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx);
    yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy);
    yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy);

    // prevents checking same line twice
    validcount++;

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            if (!P_BlockLinesIterator(bx, by, &PIT_CrossLine))
                return true;

    return false;
}

//
// PIT_CheckOnMobjZ
//
static bool PIT_CheckOnMobjZ(mobj_t *thing)
{
    fixed_t blockdist;

    if (!(thing->flags & MF_SOLID))
        return true;

    // [RH] Corpses and specials don't block moves
    if (thing->flags & (MF_CORPSE | MF_SPECIAL))
        return true;

    // Don't clip against self
    if (thing == tmthing)
        return true;

    // over/under thing
    if (tmthing->z > thing->z + thing->height)
        return true;
    else if (tmthing->z + tmthing->height <= thing->z)
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (ABS(thing->x - tmx) >= blockdist || ABS(thing->y - tmy) >= blockdist)
        return true;    // Didn't hit thing

    onmobj = thing;
    return false;
}

//
// MOVEMENT CLIPPING
//

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
//
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//
bool P_CheckPosition(mobj_t *thing, const fixed_t x, const fixed_t y)
{
    int             xl;
    int             xh;
    int             yl;
    int             yh;
    const sector_t  *newsec;
    fixed_t         radius = thing->radius;

    tmthing = thing;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + radius;
    tmbbox[BOXBOTTOM] = y - radius;
    tmbbox[BOXRIGHT] = x + radius;
    tmbbox[BOXLEFT] = x - radius;

    newsec = R_PointInSubsector(x, y)->sector;
    floorline = NULL;                               // killough 08/01/98
    blockline = NULL;
    ceilingline = NULL;

    // Whether object can get out of a sticky situation:
    tmunstuck = (thing->player                      // only players
                 && thing->player->mo == thing);    // not voodoo dolls

    // the base floor/ceiling is from the subsector that contains the
    // point. Any contacted lines the step closer together will adjust them
    tmfloorz = tmdropoffz = newsec->floorheight;
    tmceilingz = newsec->ceilingheight;
    validcount++;
    numspechit = 0;

    if ((tmthing->flags & MF_NOCLIP) || freeze)
        return true;

    // Check things first, possibly picking things up.
    // The bounding box is extended by MAXRADIUS
    // because mobj_ts are grouped into mapblocks
    // based on their origin point, and can overlap
    // into adjacent blocks by up to MAXRADIUS units.
    xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS);
    xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS);
    yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS);
    yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS);

    validcount++;

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, &PIT_CheckThing))
                return false;

    // check lines
    if ((thing->flags & MF_SPECIAL) && !(thing->flags & MF_DROPPED))
    {
        radius = thing->info->pickupradius;

        tmbbox[BOXTOP] = y + radius;
        tmbbox[BOXBOTTOM] = y - radius;
        tmbbox[BOXRIGHT] = x + radius;
        tmbbox[BOXLEFT] = x - radius;
    }

    xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx);
    xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx);
    yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy);
    yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy);

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            if (!P_BlockLinesIterator(bx, by, &PIT_CheckLine))
                return false;

    return true;
}

//
// P_FakeZMovement
//
static void P_FakeZMovement(mobj_t *mo)
{
    // adjust height
    mo->z += mo->momz;

    if ((mo->flags & MF_FLOAT) && mo->target)
        // float down towards target if too close
        if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
        {
            const fixed_t   delta = (mo->target->z + (mo->height >> 1) - mo->z) * 3;

            if (P_ApproxDistance(mo->x - mo->target->x, mo->y - mo->target->y) < ABS(delta))
                mo->z += (delta < 0 ? -FLOATSPEED : FLOATSPEED);
        }

    // clip movement
    if (mo->z <= mo->floorz)
    {
        // hit the floor
        if (mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;   // the skull slammed into something

        if (mo->momz < 0)
            mo->momz = 0;

        mo->z = mo->floorz;
    }
    else if (!(mo->flags & MF_NOGRAVITY))
    {
        if (!mo->momz)
            mo->momz = -GRAVITY;

        mo->momz -= GRAVITY;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
        // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;

        if (mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;   // the skull slammed into something

        mo->z = mo->ceilingz - mo->height;
    }
}

//
// P_CheckOnMobj
// Checks if the new Z position is legal
//
mobj_t *P_CheckOnMobj(mobj_t *thing)
{
    int             xl;
    int             xh;
    int             yl;
    int             yh;
    const sector_t  *newsec;
    const fixed_t   x = thing->x;
    const fixed_t   y = thing->y;
    const mobj_t    oldmo = *thing; // save the old mobj before the fake zmovement
    fixed_t         radius;

    tmthing = thing;

    P_FakeZMovement(tmthing);

    tmx = x;
    tmy = y;

    radius = tmthing->radius;
    tmbbox[BOXTOP] = y + radius;
    tmbbox[BOXBOTTOM] = y - radius;
    tmbbox[BOXRIGHT] = x + radius;
    tmbbox[BOXLEFT] = x - radius;

    newsec = R_PointInSubsector(x, y)->sector;
    ceilingline = NULL;

    // the base floor/ceiling is from the subsector that contains the
    // point. Any contacted lines the step closer together will adjust them
    tmfloorz = tmdropoffz = newsec->floorheight;
    tmceilingz = newsec->ceilingheight;

    validcount++;
    numspechit = 0;

    if ((tmthing->flags & MF_NOCLIP) || freeze)
        return NULL;

    // check things first, possibly picking things up
    // the bounding box is extended by MAXRADIUS because mobj_ts are grouped
    // into mapblocks based on their origin point, and can overlap into adjacent
    // blocks by up to MAXRADIUS units
    xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS);
    xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS);
    yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS);
    yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS);

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, &PIT_CheckOnMobjZ))
            {
                *tmthing = oldmo;
                return onmobj;
            }

    *tmthing = oldmo;
    return NULL;
}

//
// P_IsInLiquid
//
bool P_IsInLiquid(mobj_t *thing)
{
    if (thing->player)
    {
        for (const struct msecnode_s *seclist = thing->touching_sectorlist; seclist; seclist = seclist->m_tnext)
            if (seclist->m_sector->terraintype < LIQUID)
                return false;
    }
    else
    {
        const int   flags = thing->flags;

        if (flags & MF_NOGRAVITY)
            return false;
        else
        {
            if (flags & MF_SHOOTABLE)
            {
                for (const struct msecnode_s *seclist = thing->touching_sectorlist; seclist; seclist = seclist->m_tnext)
                    if (seclist->m_sector->terraintype < LIQUID)
                        return false;
            }
            else
            {
                sector_t    *sector = thing->subsector->sector;

                if (sector->terraintype < LIQUID)
                    return false;

                if ((flags & MF_CORPSE) && thing->z > sector->floorheight + FRACUNIT)
                    return false;
            }
        }
    }

    return true;
}

//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//
bool P_TryMove(mobj_t *thing, const fixed_t x, const fixed_t y, const int dropoff)
{
    fixed_t oldx, oldy;
    int     flags;

    felldown = false;   // killough 11/98
    floatok = false;

    if (!P_CheckPosition(thing, x, y))
        return false;   // solid wall or thing

    flags = thing->flags;

    if (!(flags & MF_NOCLIP) && !freeze)
    {
        // killough 07/26/98: reformatted slightly
        // killough 08/01/98: Possibly allow escape if otherwise stuck
        if (tmceilingz - tmfloorz < thing->height   // doesn't fit
            // mobj must lower to fit
            || (floatok = true, !(flags & MF_TELEPORT) && tmceilingz - thing->z < thing->height)
            // too big a step up
            || (!(flags & MF_TELEPORT) && tmfloorz - thing->z > 24 * FRACUNIT))
            return (tmunstuck && !(ceilingline && untouched(ceilingline)) && !(floorline && untouched(floorline)));

        if (!(flags & (MF_DROPOFF | MF_FLOAT)))
        {
            if (!dropoff
                // large jump down (e.g. dogs)
                || (dropoff == 2
                    && (tmfloorz - tmdropoffz > 128 * FRACUNIT
                        || !thing->target
                        || thing->target->z > tmdropoffz)))
            {
                if (tmfloorz - tmdropoffz > 24 * FRACUNIT)
                    return false;
            }
            else
                // dropoff allowed -- check for whether it fell more than 24
                felldown = (!(flags & MF_NOGRAVITY) && thing->z - tmfloorz > 24 * FRACUNIT);
        }

        if ((flags & MF_BOUNCES)
            && !(flags & (MF_MISSILE | MF_NOGRAVITY))
            && !sentient(thing)
            && tmfloorz - thing->z > 16 * FRACUNIT)
            return false;   // too big a step up for bouncers under gravity

        // killough 11/98: prevent falling objects from going up too many steps
        if ((thing->flags2 & MF2_FALLING)
            && tmfloorz - thing->z > FixedMul(thing->momx, thing->momx) + FixedMul(thing->momy, thing->momy))
            return false;
    }

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition(thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;   // killough 11/98: keep track of dropoffs
    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    if (thing->player && thing->player->mo == thing)
    {
        const int   dist = P_ApproxDistance(x - oldx, y - oldy) >> FRACBITS;

        if (dist)
        {
            stat_distancetraveled = SafeAdd(stat_distancetraveled, dist);
            viewplayer->distancetraveled += dist;

            AM_DropBreadCrumb();
        }
    }

    // [BH] check if new sector is liquid and clip/unclip feet as necessary
    if ((thing->flags2 & MF2_FOOTCLIP) && P_IsInLiquid(thing))
        thing->flags2 |= MF2_FEETARECLIPPED;
    else
        thing->flags2 &= ~MF2_FEETARECLIPPED;

    // if any special lines were hit, do the effect
    if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)) && !freeze)
        while (numspechit--)
        {
            // see if the line was crossed
            line_t  *ld = spechit[numspechit];

            if (ld->special)
            {
                const int   oldside = P_PointOnLineSide(oldx, oldy, ld);

                if (oldside != P_PointOnLineSide(thing->x, thing->y, ld))
                    P_CrossSpecialLine(ld, oldside, thing);
            }
        }

    return true;
}

//
// killough 09/12/98:
//
// Apply "torque" to objects hanging off of ledges, so that they
// fall off. It's not really torque, since DOOM has no concept of
// rotation, but it's a convincing effect which avoids anomalies
// such as lifeless objects hanging more than halfway off of ledges,
// and allows objects to roll off of the edges of moving lifts, or
// to slide up and then back down stairs, or to fall into a ditch.
// If more than one linedef is contacted, the effects are cumulative,
// so balancing is possible.
//
static bool PIT_ApplyTorque(line_t *ld)
{
    // If thing touches two-sided pivot linedef
    if (ld->backsector
        && tmbbox[BOXRIGHT] > ld->bbox[BOXLEFT]
        && tmbbox[BOXLEFT] < ld->bbox[BOXRIGHT]
        && tmbbox[BOXTOP] > ld->bbox[BOXBOTTOM]
        && tmbbox[BOXBOTTOM] < ld->bbox[BOXTOP]
        && P_BoxOnLineSide(tmbbox, ld) == -1)
    {
        mobj_t  *mo = tmthing;

        // lever arm
        fixed_t dist = (ld->dx >> FRACBITS) * (mo->y >> FRACBITS)
            - (ld->dy >> FRACBITS) * (mo->x >> FRACBITS)
            - (ld->dx >> FRACBITS) * (ld->v1->y >> FRACBITS)
            + (ld->dy >> FRACBITS) * (ld->v1->x >> FRACBITS);

        // dropoff direction
        if (dist < 0 ?
            (ld->frontsector->floorheight < mo->z && ld->backsector->floorheight >= mo->z) :
            (ld->backsector->floorheight < mo->z && ld->frontsector->floorheight >= mo->z))
        {
            // At this point, we know that the object straddles a two-sided
            // linedef, and that the object's center of mass is above-ground.
            fixed_t x = ABS(ld->dx);
            fixed_t y = ABS(ld->dy);

            if (y > x)
                SWAP(x, y);

            y = finesine[(tantoangle[FixedDiv(y, x) >> DBITS] + ANG90) >> ANGLETOFINESHIFT];

            // Momentum is proportional to distance between the
            // object's center of mass and the pivot linedef.

            // It is scaled by 2 ^ (OVERDRIVE - gear). When gear is
            // increased, the momentum gradually decreases to 0 for
            // the same amount of pseudotorque, so that oscillations
            // are prevented, yet it has a chance to reach equilibrium.
            dist = FixedDiv(FixedMul(dist, (mo->gear < OVERDRIVE ? y << (OVERDRIVE - mo->gear) : y >> (mo->gear - OVERDRIVE))), x);

            // Apply momentum away from the pivot linedef.
            x = FixedMul(ld->dy, dist);
            y = FixedMul(ld->dx, dist);

            // Avoid moving too fast all of a sudden (step into "overdrive")
            dist = FixedMul(x, x) + FixedMul(y, y);

            while (dist > 4 * FRACUNIT && mo->gear < MAXGEAR)
            {
                mo->gear++;
                x >>= 1;
                y >>= 1;
                dist >>= 1;
            }

            mo->momx -= x;
            mo->momy += y;
        }
    }

    return true;
}

//
// killough 09/12/98
//
// Applies "torque" to objects, based on all contacted linedefs
//
void P_ApplyTorque(mobj_t *mo)
{
    const int   x = mo->x;
    const int   y = mo->y;
    const int   radius = mo->radius;
    const int   xl = P_GetSafeBlockX((tmbbox[BOXLEFT] = x - radius) - bmaporgx);
    const int   xh = P_GetSafeBlockX((tmbbox[BOXRIGHT] = x + radius) - bmaporgx);
    const int   yl = P_GetSafeBlockY((tmbbox[BOXBOTTOM] = y - radius) - bmaporgy);
    const int   yh = P_GetSafeBlockY((tmbbox[BOXTOP] = y + radius) - bmaporgy);
    const int   flags2 = mo->flags2;    // Remember the current state, for gear-change

    tmthing = mo;
    validcount++;   // prevents checking same line twice

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, &PIT_ApplyTorque);

    // If any momentum, mark object as 'falling' using engine-internal flags
    if (mo->momx | mo->momy)
        mo->flags2 |= MF2_FALLING;
    else        // Clear the engine-internal flag indicating falling object.
        mo->flags2 &= ~MF2_FALLING;

    // If the object has been moving, step up the gear.
    // This helps reach equilibrium and avoid oscillations.

    // DOOM has no concept of potential energy, much less
    // of rotation, so we have to creatively simulate these
    // systems somehow :)
    if (!((mo->flags2 | flags2) & MF2_FALLING)) // If not falling for a while,
        mo->gear = 0;                           // Reset it to full strength
    else if (mo->gear < MAXGEAR)                // Else if not at max gear,
        mo->gear++;                             // move up a gear

    // [JN] Reduce torque tics while torque is applied, don't go negative.
    if ((mo->flags2 & MF2_FALLING) && mo->geartime > 0)
        mo->geartime--;
}

//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//
static bool P_ThingHeightClip(mobj_t *thing)
{
    const fixed_t   oldfloorz = thing->floorz;  // haleyjd
    const bool      onfloor = (thing->z == oldfloorz);
    const int       flags2 = thing->flags2;
    const player_t  *player = thing->player;

    P_CheckPosition(thing, thing->x, thing->y);

    // what about stranding a monster partially off an edge?
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;   // killough 11/98: remember dropoffs

    if ((flags2 & MF2_FEETARECLIPPED) && !player && r_liquid_bob)
        thing->z = tmfloorz;
    else if (flags2 & MF2_FLOATBOB)
    {
        if (tmfloorz > oldfloorz || !(thing->flags & MF_NOGRAVITY))
            thing->z = thing->z - oldfloorz + tmfloorz;

        if (thing->z + thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;
    }
    else if (onfloor)
    {
        // walking monsters rise and fall with the floor
        thing->z = tmfloorz;

        // [BH] immediately update player's view
        if (player)
            P_CalcHeight();

        // killough 11/98: Possibly upset balance of objects hanging off ledges
        if ((flags2 & MF2_FALLING) && thing->gear >= MAXGEAR)
            thing->gear = 0;
    }
    else
    {
        // don't adjust a floating monster unless forced to
        if (thing->z + thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;
    }

    return (thing->ceilingz - tmfloorz >= thing->height);
}

//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
static fixed_t  bestslidefrac;
static line_t   *bestslideline;
static mobj_t   *slidemo;
static fixed_t  tmxmove;
static fixed_t  tmymove;

//
// P_HitSlideLine
// Adjusts the xmove/ymove so that the next move will slide along the wall.
//
static void P_HitSlideLine(line_t *ld)
{
    angle_t     lineangle;
    angle_t     moveangle;
    angle_t     deltaangle;
    fixed_t     movelen;

    // phares:
    // Under icy conditions, if the angle of approach to the wall
    // is more than 45 degrees, then you'll bounce and lose half
    // your momentum. If less than 45 degrees, you'll slide along
    // the wall. 45 is arbitrary and is believable.
    //
    // Check for the special cases of horizontal or vertical walls.

    // killough 10/98: only bounce if hit hard (prevents wobbling)
    const bool  icyfloor = (P_ApproxDistance(tmxmove, tmymove) > 4 * FRACUNIT
                            && slidemo->z <= slidemo->floorz
                            && P_GetFriction(slidemo, NULL) > ORIG_FRICTION);

    if (ld->slopetype == ST_HORIZONTAL)
    {
        if (icyfloor && ABS(tmymove) > ABS(tmxmove))
        {
            if (slidemo->health > 0)
                S_StartSound(slidemo, sfx_oof); // oooff!

            tmxmove /= 2;   // absorb half the momentum
            tmymove = -tmymove / 2;
        }
        else
            tmymove = 0;    // no more movement in the Y direction

        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {
        if (icyfloor && ABS(tmxmove) > ABS(tmymove))
        {
            if (slidemo->health > 0)
                S_StartSound(slidemo, sfx_oof); // oooff!

            tmxmove = -tmxmove / 2; // absorb half the momentum
            tmymove /= 2;
        }
        else
            tmxmove = 0;    // no more movement in the X direction

        return;
    }

    lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);

    if (P_PointOnLineSide(slidemo->x, slidemo->y, ld) == 1)
        lineangle += ANG180;

    moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
    moveangle += 10;    // prevents sudden path reversal due to rounding error
    deltaangle = moveangle - lineangle;
    movelen = P_ApproxDistance(tmxmove, tmymove);

    if (icyfloor && deltaangle > ANG45 && deltaangle < ANG90 + ANG45)
    {
        moveangle = (lineangle - deltaangle) >> ANGLETOFINESHIFT;
        movelen /= 2;   // absorb
        tmxmove = FixedMul(movelen, finecosine[moveangle]);
        tmymove = FixedMul(movelen, finesine[moveangle]);

        if (slidemo->health > 0)
            S_StartSound(slidemo, sfx_oof); // oooff!
    }
    else
    {
        fixed_t newlen;

        if (deltaangle > ANG180)
            deltaangle += ANG180;

        lineangle >>= ANGLETOFINESHIFT;
        newlen = FixedMul(movelen, finecosine[deltaangle >> ANGLETOFINESHIFT]);
        tmxmove = FixedMul(newlen, finecosine[lineangle]);
        tmymove = FixedMul(newlen, finesine[lineangle]);
    }
}

//
// PTR_SlideTraverse
//
static bool PTR_SlideTraverse(intercept_t *in)
{
    line_t                  *li = in->d.line;
    const unsigned short    flags = li->flags;

    if (!(flags & ML_TWOSIDED))
    {
        if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
            return true;    // don't hit the back side

        goto isblocking;
    }

    // [JN] Treat two-sided linedefs as single sided for smooth sliding.
    else if (flags & ML_BLOCKING)
        goto isblocking;

    // set openrange, opentop, openbottom
    P_LineOpening(li);

    if (openrange < slidemo->height)
        goto isblocking;    // doesn't fit

    if (opentop - slidemo->z < slidemo->height)
        goto isblocking;    // mobj is too high

    if (openbottom - slidemo->z > 24 * FRACUNIT)
        goto isblocking;    // too big a step up

    // this line doesn't block movement
    return true;

    // the line does block movement, see if it is closer than best so far
isblocking:
    if (in->frac < bestslidefrac)
    {
        bestslidefrac = in->frac;
        bestslideline = li;
    }

    return false;   // stop
}

//
// P_SlideMove
// The momx/momy move is bad, so try to slide along a wall.
// Find the first line hit, move flush to it, and slide along it.
//
// killough 11/98: reformatted
void P_SlideMove(mobj_t *mo)
{
    int             hitcount = 3;
    const fixed_t   radius = mo->radius;
    player_t        *player = mo->player;

    slidemo = mo;   // the object that's sliding

    do
    {
        fixed_t leadx, leady;
        fixed_t trailx, traily;

        if (!--hitcount)
            goto stairstep; // don't loop forever

        // trace along the three leading corners
        if (mo->momx > 0)
        {
            leadx = mo->x + radius;
            trailx = mo->x - radius;
        }
        else
        {
            leadx = mo->x - radius;
            trailx = mo->x + radius;
        }

        if (mo->momy > 0)
        {
            leady = mo->y + radius;
            traily = mo->y - radius;
        }
        else
        {
            leady = mo->y - radius;
            traily = mo->y + radius;
        }

        bestslidefrac = FRACUNIT + 1;

        P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy, PT_ADDLINES, &PTR_SlideTraverse);
        P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy, PT_ADDLINES, &PTR_SlideTraverse);
        P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy, PT_ADDLINES, &PTR_SlideTraverse);

        // move up to the wall
        if (bestslidefrac == FRACUNIT + 1)
        {
            // the move must have hit the middle, so stairstep

stairstep:
            // killough 03/15/98: Allow objects to drop off ledges
            // phares 05/04/98: kill momentum if you can't move at all
            if (!P_TryMove(mo, mo->x, mo->y + mo->momy, 1))
                P_TryMove(mo, mo->x + mo->momx, mo->y, 1);

            break;
        }

        // fudge a bit to make sure it doesn't hit
        if ((bestslidefrac -= 0x0800) > 0)
            // killough 03/15/98: Allow objects to drop off ledges
            if (!P_TryMove(mo, mo->x + FixedMul(mo->momx, bestslidefrac), mo->y + FixedMul(mo->momy, bestslidefrac), 1))
                goto stairstep;

        // Now continue along the wall.
        // First calculate remainder.
        bestslidefrac = FRACUNIT - (bestslidefrac + 0x0800);

        if (bestslidefrac > FRACUNIT)
            bestslidefrac = FRACUNIT;
        else if (bestslidefrac <= 0)
            break;

        tmxmove = FixedMul(mo->momx, bestslidefrac);
        tmymove = FixedMul(mo->momy, bestslidefrac);

        P_HitSlideLine(bestslideline);  // clip the moves

        mo->momx = tmxmove;
        mo->momy = tmymove;

        // killough 10/98: affect the bobbing the same way (but not voodoo dolls)
        if (player && player->mo == mo)
        {
            if (ABS(player->momx) > ABS(tmxmove))
                player->momx = tmxmove;

            if (ABS(player->momy) > ABS(tmymove))
                player->momy = tmymove;
        }
    } while (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove, 1));
}

//
// P_LineAttack
//
mobj_t          *linetarget;    // who got hit (or NULL)
static mobj_t   *shootthing;

// killough 08/02/98: for more intelligent autoaiming
static int      aim_flags_mask;

// height if not aiming up or down
static fixed_t  shootz;

static int      la_damage;
fixed_t         attackrange;

static fixed_t  aimslope;

// slopes to top and bottom of target
static fixed_t  topslope;
static fixed_t  bottomslope;

//
// PTR_AimTraverse
// Sets linetarget and aimslope when a target is aimed at.
//
static bool PTR_AimTraverse(intercept_t *in)
{
    mobj_t  *th;
    fixed_t thingtopslope;
    fixed_t thingbottomslope;
    fixed_t dist;

    if (in->isaline)
    {
        line_t  *li = in->d.line;
        fixed_t slope;

        if (!(li->flags & ML_TWOSIDED))
            return false;   // stop

        // Crosses a two sided line.
        // A two sided line will restrict the possible target ranges.
        P_LineOpening(li);

        if (openbottom >= opentop)
            return false;   // stop

        dist = FixedMul(attackrange, in->frac);

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv(openbottom - shootz, dist);

            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv(opentop - shootz, dist);

            if (slope < topslope)
                topslope = slope;
        }

        if (topslope <= bottomslope)
            return false;   // stop

        return true;    // shot continues
    }

    // shoot a thing
    th = in->d.thing;

    if (th == shootthing)
        return true;    // can't shoot self

    if (!(th->flags & MF_SHOOTABLE))
        return true;    // corpse or something

    // killough 07/19/98, 08/02/98:
    // friends don't aim at friends (except players), at least not first
    if ((th->flags & shootthing->flags & aim_flags_mask) && !th->player)
        return true;

    // check angles to see if the thing can be aimed at
    dist = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv(th->z + th->height - shootz, dist);

    if (thingtopslope < bottomslope)
        return true;    // shot over the thing

    thingbottomslope = FixedDiv(th->z - shootz, dist);

    if (thingbottomslope > topslope)
        return true;    // shot under the thing

    // this thing can be hit!
    if (thingtopslope > topslope)
        thingtopslope = topslope;

    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    aimslope = (thingtopslope + thingbottomslope) / 2;
    linetarget = th;

    return false;   // don't go any farther
}

bool    hitwall;

//
// PTR_ShootTraverse
//
static bool PTR_ShootTraverse(intercept_t *in)
{
    fixed_t x, y, z;
    fixed_t frac;
    mobj_t  *th;
    fixed_t dist;

    if (in->isaline)
    {
        line_t          *li = in->d.line;
        unsigned short  side;
        fixed_t         distz;

        if (li->special)
            P_ShootSpecialLine(shootthing, li);

        if (li->flags & ML_TWOSIDED)
        {
            // crosses a two sided line
            P_LineOpening(li);

            dist = FixedMul(attackrange, in->frac);

            if (!li->backsector)
            {
                if (FixedDiv(openbottom - shootz, dist) <= aimslope && FixedDiv(opentop - shootz, dist) >= aimslope)
                    return true;    // shot continues
            }
            else
            {
                if ((li->frontsector->interpfloorheight == li->backsector->interpfloorheight
                    || FixedDiv(openbottom - shootz, dist) <= aimslope)
                    && (li->frontsector->interpceilingheight == li->backsector->interpceilingheight
                        || FixedDiv(opentop - shootz, dist) >= aimslope))
                    return true;    // shot continues
            }
        }

        // position a bit closer
        frac = in->frac - 128;
        distz = FixedMul(aimslope, FixedMul(attackrange, frac));
        z = shootz + distz;

        // clip shots on floor and ceiling
        if ((side = li->sidenum[P_PointOnLineSide(shootthing->x, shootthing->y, li)]) != NO_INDEX)
        {
            const sector_t  *sector = sides[side].sector;
            const fixed_t   ceilingz = sector->interpceilingheight;

            if (z > ceilingz && distz)
            {
                if (sector->ceilingpic == skyflatnum)
                    return false;

                frac = FixedDiv(FixedMul(frac, ceilingz - shootz), distz);
                z = ceilingz;
            }
            else
            {
                fixed_t floorz = sector->interpfloorheight;

                if (z < floorz && distz)
                {
                    if (sector->terraintype != SOLID)
                        return false;

                    frac = -FixedDiv(FixedMul(frac, shootz - floorz), distz);
                    z = floorz;
                }
                else
                    hitwall = true;
            }
        }

        if (li->frontsector->ceilingpic == skyflatnum)
        {
            // don't shoot the sky!
            if (z > li->frontsector->interpceilingheight)
                return false;

            // it's a sky hack wall
            if (li->backsector && li->backsector->ceilingpic == skyflatnum && li->backsector->interpceilingheight < z)
                return false;
        }

        // spawn bullet puff
        P_SpawnPuff(dltrace.x + FixedMul(dltrace.dx, frac), dltrace.y + FixedMul(dltrace.dy, frac), z, shootangle);

        // don't go any farther
        return false;
    }

    // shoot a thing
    th = in->d.thing;

    if (th == shootthing)
        return true;    // can't shoot self

    if (!(th->flags & MF_SHOOTABLE))
        return true;    // corpse or something

    dist = FixedMul(attackrange, in->frac);

    // check angles to see if the thing can be aimed at
    if (FixedDiv(th->z + th->height - shootz, dist) < aimslope)
        return true;    // shot over the thing

    if (FixedDiv(th->z - shootz, dist) > aimslope)
        return true;    // shot under the thing

    // hit thing
    // position a bit closer
    frac = in->frac - (attackrange == MISSILERANGE ? 320 : 10240);

    x = dltrace.x + FixedMul(dltrace.dx, frac);
    y = dltrace.y + FixedMul(dltrace.dy, frac);
    z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

    if ((shootthing->flags2 & MF2_FEETARECLIPPED) && (shootthing->player && r_liquid_lowerview))
        z -= FOOTCLIPSIZE;

    // Spawn bullet puff or blood, depending on target type
    if (th->flags & MF_NOBLOOD)
        P_SpawnPuff(x, y, z - M_RandomInt(0, 16) * FRACUNIT, shootangle);
    else if (r_blood != r_blood_none)
    {
        if (th->type == MT_SKULL && !(th->flags & MF_FUZZ))
        {
            if (r_blood == r_blood_red)
                P_SpawnBlood(x, y, z + M_RandomInt(-8, 8) * FRACUNIT, shootangle, la_damage, th);
            else
                P_SpawnPuff(x, y, z - M_RandomInt(0, 16) * FRACUNIT, shootangle);
        }
        else if (th->bloodcolor)
        {
            if (th->type != MT_PLAYER)
                P_SpawnBlood(x, y, z + M_RandomInt(-8, 8) * FRACUNIT, shootangle, la_damage, th);
            else if (!viewplayer->powers[pw_invulnerability] && !(viewplayer->cheats & CF_GODMODE))
                P_SpawnBlood(x, y, z + M_RandomInt(4, 16) * FRACUNIT, shootangle, la_damage * 2, th);
        }
    }

    if (la_damage)
    {
        successfulshot = true;
        P_DamageMobj(th, shootthing, shootthing, la_damage, true, false);
    }

    // don't go any farther
    return false;
}

//
// P_AimLineAttack
//
fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, const fixed_t distance, const int mask)
{
    fixed_t x2, y2;

    if (!t1)
        return 0;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;

    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;

    // can't shoot outside view angles
    topslope = (VANILLAHEIGHT / 2) * FRACUNIT / (VANILLAWIDTH / 2);
    bottomslope = -(VANILLAHEIGHT / 2) * FRACUNIT / (VANILLAWIDTH / 2);

    attackrange = distance;
    linetarget = NULL;

    // killough 08/02/98: prevent friends from aiming at friends
    aim_flags_mask = mask;

    P_PathTraverse(t1->x, t1->y, x2, y2, (PT_ADDLINES | PT_ADDTHINGS), &PTR_AimTraverse);

    if (linetarget)
        return aimslope;

    return 0;
}

//
// P_LineAttack
// If damage == 0, it is just a test trace that will leave linetarget set.
//
void P_LineAttack(mobj_t *t1, angle_t angle, const fixed_t distance, const fixed_t slope, const int damage)
{
    fixed_t x2, y2;

    shootangle = angle;
    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;

    if ((t1->flags2 & MF2_FEETARECLIPPED) && !t1->player && r_liquid_clipsprites)
        shootz -= FOOTCLIPSIZE;

    attackrange = distance;
    aimslope = slope;

    P_PathTraverse(t1->x, t1->y, x2, y2, (PT_ADDLINES | PT_ADDTHINGS), &PTR_ShootTraverse);
}

//
// USE LINES
//
static mobj_t   *usething;

static bool PTR_UseTraverse(intercept_t *in)
{
    int     side = 0;
    line_t  *line = in->d.line;

    if (autousing)
    {
        sector_t    *sector = line->backsector;

        if (sector && sector->ceilingdata && sector->interpfloorheight != sector->interpceilingheight)
            return false;
    }

    if (!line->special)
    {
        P_LineOpening(line);

        if (openrange <= 0)
        {
            if (!autousing)
                S_StartSound(usething, sfx_noway);

            // can't use through a wall
            return false;
        }

        // not a special line, but keep checking
        return true;
    }

    if (P_PointOnLineSide(usething->x, usething->y, line) == 1)
        side = 1;

    P_UseSpecialLine(usething, line, side);

    // can't use for more than one special line in a row
    // [BH] unless its the wrong side
    return ((side && !compat_useblocking) || (line->flags & ML_PASSUSE));
}

//
// Returns false if a "oof" sound should be made because of a blocking
// linedef. Makes 2s middles which are impassable, as well as 2s uppers
// and lowers which block the player, cause the sound effect when the
// player tries to activate them. Specials are excluded, although it is
// assumed that all special linedefs within reach have been considered
// and rejected already (see P_UseLines).
//
// by Lee Killough
//
static bool PTR_NoWayTraverse(intercept_t *in)
{
    line_t                  *ld = in->d.line;
    const unsigned short    flags = ld->flags;

    return (ld->special || ((flags & ML_TWOSIDED) && (flags & ML_BLOCKING)) || !((flags & ML_BLOCKING)
        || (P_LineOpening(ld), (openrange <= 0 || openbottom > usething->z + 24 * FRACUNIT ||
            opentop < usething->z + usething->height))));
}

bool P_DoorClosed(line_t *line)
{
    P_LineOpening(line);

    return (openrange <= 0 || openbottom > usething->z + 24 * FRACUNIT || opentop < usething->z + usething->height);
}

//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines(void)
{
    angle_t angle;
    fixed_t x1, y1;
    fixed_t x2, y2;

    if (automapactive && !am_followmode)
        return;

    usething = viewplayer->mo;

    angle = usething->angle >> ANGLETOFINESHIFT;

    x1 = usething->x;
    y1 = usething->y;
    x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
    y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];

    // This added test makes the "oof" sound work on 2s lines -- killough:
    if (P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, &PTR_UseTraverse))
        if (!P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, &PTR_NoWayTraverse))
            if (!autousing)
                S_StartSound(usething, sfx_noway);
}

//
// RADIUS ATTACK
//
static mobj_t   *bombsource;
static mobj_t   *bombspot;
static int      bombdamage;
static int      bombdistance;
static bool     bombverticality;

// MBF21: dehacked splash groups
static bool P_SplashImmune(mobj_t *target, mobj_t *spot)
{
    // not default behavior and same group
    return (mobjinfo[target->type].splashgroup != SG_DEFAULT
        && mobjinfo[target->type].splashgroup == mobjinfo[spot->type].splashgroup);
}

//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
bool PIT_RadiusAttack(mobj_t *thing)
{
    fixed_t     dist;
    mobjtype_t  type;

    // killough 08/20/98: allow bouncers to take damage
    // (missile bouncers are already excluded with MF_NOBLOCKMAP)
    if (!(thing->flags & (MF_SHOOTABLE | MF_BOUNCES))
        // [BH] allow corpses to react to blast damage
        && !(thing->flags & MF_CORPSE))
        return true;

    if (P_SplashImmune(thing, bombspot))
        return true;

    type = thing->type;

    // killough 08/10/98: allow grenades to hurt anyone, unless
    // fired by Cyberdemons, in which case it won't hurt Cybers.
    if ((bombspot->flags & MF_BOUNCES) ? (type == MT_CYBORG && bombsource->type == MT_CYBORG) :
        (thing->mbf21flags & (MF_MBF21_NORADIUSDMG | MF_MBF21_BOSS)
            && !(bombspot->mbf21flags & MF_MBF21_FORCERADIUSDMG)))
        return true;

    dist = MAX(ABS(thing->x - bombspot->x), ABS(thing->y - bombspot->y)) - thing->radius;

    if (!bombverticality || infiniteheight || compat_nopassover || type == MT_BOSSBRAIN)
    {
        // [BH] if killing boss in DOOM II MAP30, use old code that
        //  doesn't use z height in blast radius
        dist = MAX(0, dist >> FRACBITS);

        if (dist >= bombdistance)
            return true;    // out of range
    }
    else
    {
        dist = MAX(0, MAX(dist, ABS(thing->z + (thing->height >> 1) - bombspot->z)) >> FRACBITS);

        if (dist >= bombdistance)
            return true;    // out of range

        // [BH] check z height for blast damage
        if ((thing->floorz > bombspot->z && bombspot->ceilingz < thing->z)
            || (thing->ceilingz < bombspot->z && bombspot->floorz > thing->z))
            return true;
    }

    if (P_CheckSight(thing, bombspot))
    {
        // [XA] independent damage/distance calculation.
        const int   damage = (bombdamage == bombdistance ? bombdamage - dist :
                        (bombdamage * (bombdistance - dist) / bombdistance) + 1);

        // must be in direct path
        P_DamageMobj(thing, bombspot, bombsource, damage, true, false);

        // [BH] count number of times player's rockets hit a monster
        if (bombspot->type == MT_ROCKET && type != MT_BARREL && !(thing->flags & MF_CORPSE))
        {
            if (bombspot->nudge == 1)
            {
                viewplayer->shotssuccessful[wp_missile]++;
                stat_shotssuccessful_rocketlauncher = SafeAdd(stat_shotssuccessful_rocketlauncher, 1);
            }

            bombspot->nudge++;
        }
    }

    return true;
}

//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack(mobj_t *spot, mobj_t *source, const int damage, const int distance, const bool verticality)
{
    const fixed_t   dist = (damage << FRACBITS) + MAXRADIUS;
    const int       xh = P_GetSafeBlockX(spot->x + dist - bmaporgx);
    const int       xl = P_GetSafeBlockX(spot->x - dist - bmaporgx);
    const int       yh = P_GetSafeBlockY(spot->y + dist - bmaporgy);
    const int       yl = P_GetSafeBlockY(spot->y - dist - bmaporgy);

    bombspot = spot;
    bombsource = source;
    bombdamage = damage;
    bombdistance = distance;
    bombverticality = verticality;

    for (int y = yl; y <= yh; y++)
        for (int x = xl; x <= xh; x++)
            P_BlockThingsIterator(x, y, &PIT_RadiusAttack);
}

//
// SECTOR HEIGHT CHANGING
// After modifying a sector's floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage as they are being crushed.
// If crunch is false, you should set the sector height back the way it was and call P_ChangeSector() again to undo the changes.
//
static bool crushchange;
static bool nofit;

//
// PIT_ChangeSector
//
static void PIT_ChangeSector(mobj_t *thing)
{
    int   flags;

    if (P_ThingHeightClip(thing))
        return; // keep checking

    flags = thing->flags;

    // crunch bodies to giblets
    if (thing->health <= 0 && (thing->flags2 & MF2_CRUSHABLE))
    {
        if (thing->player)
        {
            nofit = true;
            return;
        }

        if (!(flags & MF_NOBLOOD) && thing->bloodcolor > NOBLOOD)
        {
            const int       radius = ((spritewidth[sprites[thing->sprite].spriteframes[0].lump[0]] >> FRACBITS) >> 1) + 12;
            const int       max = M_RandomInt(50, 100) + radius;
            const int       color = colortranslation[thing->bloodcolor - 1][REDBLOODSPLATCOLOR];
            const fixed_t   x = thing->x;
            const fixed_t   y = thing->y;
            const fixed_t   floorz = thing->floorz;

            for (int i = 0; i < max; i++)
            {
                const angle_t   angle = M_BigRandomInt(0, FINEANGLES - 1);

                P_SpawnBloodSplat(x + FixedMul(M_RandomInt(0, radius) << FRACBITS, finecosine[angle]),
                    y + FixedMul(M_RandomInt(0, radius) << FRACBITS, finesine[angle]), color, true, floorz, NULL);
            }

            P_SetMobjState(thing, S_GIBS);

            if (thing->bloodcolor > REDBLOOD)
            {
                thing->colfunc = translatedcolfunc;
                thing->altcolfunc = translatedcolfunc;
            }

            thing->flags &= ~MF_SOLID;

            if (r_corpses_mirrored && (M_Random() & 1) && !(thing->flags2 & MF2_NOMIRROREDCORPSE)
                && (thing->type != MT_PAIN || !doom4vanilla))
                thing->flags2 |= MF2_MIRRORED;

            thing->height = 0;
            thing->radius = 0;
            thing->shadowoffset = 0;

            S_StartSound(thing, sfx_slop);
        }
        else
            P_RemoveMobj(thing);

        // keep checking
        return;
    }

    // crunch dropped items
    if (flags & MF_DROPPED)
    {
        P_RemoveMobj(thing);

        // keep checking
        return;
    }

    // killough 11/98: kill touchy things immediately
    if ((flags & MF_TOUCHY) && ((thing->flags2 & MF2_ARMED) || sentient(thing)))
    {
        P_DamageMobj(thing, NULL, NULL, thing->health, true, false);    // kill object
        return;
    }

    if (!(flags & MF_SHOOTABLE))
        return; // assume it is bloody gibs or something

    nofit = true;

    if (crushchange && !(maptime & 3))
    {
        if (!(flags & MF_NOBLOOD) && thing->bloodcolor && r_blood != r_blood_none
            && (thing->type != MT_PLAYER || (!viewplayer->powers[pw_invulnerability] && !(viewplayer->cheats & CF_GODMODE))))
        {
            bool        fuzz = ((flags & MF_FUZZ) && r_blood == r_blood_all);
            const int   z = thing->z + thing->height * 2 / 3;
            int         color;

            if (!fuzz)
                color = (r_blood == r_blood_red ? REDBLOOD : (r_blood == r_blood_green ? GREENBLOOD : thing->bloodcolor));

            for (int i = 0; i < 6; i++)
            {
                // spray blood in a random direction
                mobj_t  *mo = P_SpawnMobj(thing->x, thing->y, z, MT_BLOOD);

                mo->momx = M_SubRandom() << 11;
                mo->momy = M_SubRandom() << 11;
                mo->flags2 |= (M_Random() & 1) * MF2_MIRRORED;

                if (fuzz)
                {
                    mo->flags |= MF_FUZZ;
                    mo->colfunc = &R_DrawFuzzColumn;
                    mo->altcolfunc = &R_DrawFuzzColumn;
                }
                else
                {
                    mo->colfunc = bloodcolfunc;
                    mo->altcolfunc = bloodcolfunc;
                    mo->bloodcolor = color;
                }
            }
        }

        P_DamageMobj(thing, NULL, NULL, 10, true, false);

        if (thing->health <= 0 && !thing->player && con_obituaries)
        {
            char    name[128];

            if (*thing->name)
                M_StringCopy(name, thing->name, sizeof(name));
            else
                M_snprintf(name, sizeof(name), "%s %s%s",
                    ((thing->flags & MF_FRIEND) && monstercount[thing->type] == 1 ? "the" :
                        (isvowel(thing->info->name1[0]) && !(thing->flags & MF_FRIEND) ? "an" : "a")),
                    ((thing->flags & MF_FRIEND) ? "friendly " : ""),
                    (*thing->info->name1 ? thing->info->name1 : "monster"));

            name[0] = toupper(name[0]);
            C_PlayerMessage("%s was crushed%s.", name, (thing->type == MT_BARREL ? "" : " to death"));
        }
    }
}

//
// P_ChangeSector
// jff 3/19/98 added to just check monsters on the periphery
// of a moving sector instead of all in bounding box of the
// sector. Both more accurate and faster.
// [BH] renamed from P_CheckSector to P_ChangeSector to replace old one entirely
//
bool P_ChangeSector(sector_t *sector, const bool crunch)
{
    msecnode_t  *n;

    nofit = false;
    crushchange = crunch;

    // Mark all things invalid
    for (n = sector->touching_thinglist; n; n = n->m_snext)
        n->visited = false;

    do
    {
        for (n = sector->touching_thinglist; n; n = n->m_snext) // go through list
            if (!n->visited)                                    // unprocessed thing found
            {
                mobj_t  *mobj = n->m_thing;

                n->visited = true;                              // mark thing as processed

                if (mobj && !(mobj->flags & MF_NOBLOCKMAP))
                    PIT_ChangeSector(mobj);                     // process it

                break;                                          // exit and start over
            }
    } while (n);    // repeat from scratch until all things left are marked valid

    return nofit;
}

// phares 03/21/98
//
// Maintain a freelist of msecnode_t's to reduce memory allocs and frees.
static msecnode_t   *headsecnode;

void P_FreeSecNodeList(void)
{
    headsecnode = NULL; // this is all that's needed to fix the bug
}

// P_GetSecnode() retrieves a node from the freelist. The calling routine
// should make sure it sets all fields properly.
static msecnode_t *P_GetSecnode(void)
{
    msecnode_t  *node;

    if (headsecnode)
    {
        node = headsecnode;
        headsecnode = headsecnode->m_snext;
    }
    else
        node = Z_Malloc(sizeof(*node), PU_LEVEL, NULL);

    return node;
}

// P_PutSecnode() returns a node to the freelist.
static void P_PutSecnode(msecnode_t *node)
{
    node->m_snext = headsecnode;
    headsecnode = node;
}

// phares 03/16/98
//
// P_AddSecnode() searches the current list to see if this sector is
// already there. If not, it adds a sector node at the head of the list of
// sectors this object appears in. This is called when creating a list of
// nodes that will get linked in later. Returns a pointer to the new node.
//
// killough 11/98: reformatted
static msecnode_t *P_AddSecnode(sector_t *s, mobj_t *thing, msecnode_t *nextnode)
{
    msecnode_t  *node = nextnode;

    while (node)
    {
        if (node->m_sector == s)    // Already have a node for this sector?
        {
            node->m_thing = thing;  // Yes. Setting m_thing says 'keep it'.
            return nextnode;
        }

        node = node->m_tnext;
    }

    // Couldn't find an existing node for this sector. Add one at the head
    // of the list.
    node = P_GetSecnode();

    node->m_sector = s;                     // sector
    node->m_thing = thing;                  // mobj
    node->m_tprev = NULL;                   // prev node on Thing thread
    node->m_tnext = nextnode;               // next node on Thing thread

    if (nextnode)
        nextnode->m_tprev = node;           // set back link on Thing

    // Add new node at head of sector thread starting at s->touching_thinglist
    node->m_sprev = NULL;                   // prev node on sector thread
    node->m_snext = s->touching_thinglist;  // next node on sector thread

    if (s->touching_thinglist)
        node->m_snext->m_sprev = node;

    s->touching_thinglist = node;
    return node;
}

// P_DelSecnode() deletes a sector node from the list of
// sectors this object appears in. Returns a pointer to the next node
// on the linked list.
//
// killough 11/98: reformatted
static msecnode_t *P_DelSecnode(msecnode_t *node)
{
    msecnode_t  *tp = node->m_tprev;    // prev node on thing thread
    msecnode_t  *tn = node->m_tnext;    // next node on thing thread
    msecnode_t  *sp;                    // prev node on sector thread
    msecnode_t  *sn;                    // next node on sector thread

    // Unlink from the Thing thread. The Thing thread begins at
    // sector_list and not from mobj_t->touching_sectorlist.
    if (tp)
        tp->m_tnext = tn;

    if (tn)
        tn->m_tprev = tp;

    // Unlink from the sector thread. This thread begins at
    // sector_t->touching_thinglist.
    sp = node->m_sprev;
    sn = node->m_snext;

    if (sp)
        sp->m_snext = sn;
    else
        node->m_sector->touching_thinglist = sn;

    if (sn)
        sn->m_sprev = sp;

    // Return this node to the freelist
    P_PutSecnode(node);
    return tn;
}

// Delete an entire sector list
void P_DelSeclist(msecnode_t *node)
{
    while (node)
        node = P_DelSecnode(node);
}

// phares 03/14/98
//
// PIT_GetSectors
// Locates all the sectors the object is in by looking at the lines that
// cross through it. You have already decided that the object is allowed
// at this location, so don't bother with checking impassable or
// blocking lines.
static bool PIT_GetSectors(line_t *ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
        return true;

    if (P_BoxOnLineSide(tmbbox, ld) != -1)
        return true;

    // This line crosses through the object.

    // Collect the sector(s) from the line and add to the
    // sector_list you're examining. If the Thing ends up being
    // allowed to move to this position, then the sector_list
    // will be attached to the Thing's mobj_t at touching_sectorlist.
    sector_list = P_AddSecnode(ld->frontsector, tmthing, sector_list);

    // Don't assume all lines are 2-sided, since some Things
    // like MT_TFOG are allowed regardless of whether their radius takes
    // them beyond an impassable linedef.

    // killough 03/27/98, 04/04/98:
    // Use sidedefs instead of 2s flag to determine two-sidedness.
    // killough 08/01/98: avoid duplicate if same sector on both sides
    if (ld->backsector && ld->backsector != ld->frontsector)
        sector_list = P_AddSecnode(ld->backsector, tmthing, sector_list);

    return true;
}

// phares 03/14/98
//
// P_CreateSecNodeList alters/creates the sector_list that shows what sectors
// the object resides in.
//
// killough 11/98: reformatted
void P_CreateSecNodeList(mobj_t *thing, const fixed_t x, const fixed_t y)
{
    int             xl;
    int             xh;
    int             yl;
    int             yh;
    msecnode_t      *node = sector_list;
    mobj_t          *saved_tmthing = tmthing;
    const fixed_t   saved_tmx = tmx;
    const fixed_t   saved_tmy = tmy;
    fixed_t         radius = thing->info->pickupradius;

    // First, clear out the existing m_thing fields. As each node is
    // added or verified as needed, m_thing will be set properly. When
    // finished, delete all nodes where m_thing is still NULL. These
    // represent the sectors the Thing has vacated.
    while (node)
    {
        node->m_thing = NULL;
        node = node->m_tnext;
    }

    tmthing = thing;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + radius;
    tmbbox[BOXBOTTOM] = y - radius;
    tmbbox[BOXRIGHT] = x + radius;
    tmbbox[BOXLEFT] = x - radius;

    validcount++;   // used to make sure we only process a line once

    xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx);
    xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx);
    yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy);
    yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy);

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, &PIT_GetSectors);

    // Add the sector of the (x,y) point to sector_list.
    sector_list = P_AddSecnode(thing->subsector->sector, thing, sector_list);

    // Now delete any nodes that won't be used. These are the ones where
    // m_thing is still NULL.
    node = sector_list;

    while (node)
        if (!node->m_thing)
        {
            if (node == sector_list)
                sector_list = node->m_tnext;

            node = P_DelSecnode(node);
        }
        else
            node = node->m_tnext;

    // cph -
    // This is the strife we get into for using global variables. tmthing
    //  is being used by several different functions calling
    //  P_BlockThingIterator, including functions that can be called *from*
    //  P_BlockThingIterator. Using a global tmthing is not reentrant.
    //  Fun. We restore its previous value.
    tmthing = saved_tmthing;
    tmx = saved_tmx;
    tmy = saved_tmy;

    if (tmthing)
    {
        radius = tmthing->radius;
        tmbbox[BOXTOP] = tmy + radius;
        tmbbox[BOXBOTTOM] = tmy - radius;
        tmbbox[BOXRIGHT] = tmx + radius;
        tmbbox[BOXLEFT] = tmx - radius;
    }
}

void P_MapEnd(void)
{
    tmthing = NULL;
}
