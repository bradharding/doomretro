/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 1999 Lee Killough.
Copyright (C) 1999 Rand Phares.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

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

#include <stdlib.h>
#include "doomstat.h"
#include "m_bbox.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "z_zone.h"

fixed_t    tmbbox[4];
mobj_t     *tmthing;
int        tmflags;
int        tmradius;
fixed_t    tmx;
fixed_t    tmy;
fixed_t    tmz;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean    floatok;

// killough 11/98: if "felldown" true, object was pushed down ledge
boolean   felldown;

fixed_t    tmfloorz;
fixed_t    tmceilingz;
fixed_t    tmdropoffz;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t     *ceilingline;
line_t     *blockline;  // killough 8/11/98: blocking linedef
line_t     *floorline;  // killough 8/1/98: Highest touched floor
static int tmunstuck;   // killough 8/1/98: whether to allow unsticking

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// 1/11/98 killough: removed limit on special lines crossed
line_t     **spechit;
static int spechit_max;
int        numspechit;

angle_t    shootangle;

// Temporary holder for thing_sectorlist threads
msecnode_t *sector_list = NULL;                             // phares 3/16/98

extern int followplayer;

//
// TELEPORT MOVE
//

//
// PIT_StompThing
//
boolean PIT_StompThing(mobj_t *thing)
{
    fixed_t blockdist;

    // don't clip against self
    if (thing == tmthing)
        return true;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (ABS(thing->x - tmx) >= blockdist || ABS(thing->y - tmy) >= blockdist)
        return true;    // didn't hit it

    // monsters don't stomp things except on boss level
    if (!tmthing->player && gamemap != 30)
        return false;

    if (tmz > thing->z + thing->height)
        return true;        // overhead
    if (tmz + tmthing->height < thing->z)
        return true;        // underneath

    P_DamageMobj(thing, tmthing, tmthing, 10000);

    return true;
}

//
// P_TeleportMove
//
boolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z)
{
    int         xl;
    int         xh;
    int         yl;
    int         yh;
    int         bx;
    int         by;

    subsector_t *newsubsec;

    // kill anything occupying the position
    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;
    tmz = z;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector(x, y);
    ceilingline = NULL;

    // The base floor/ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

    // stomp on any things contacted
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_StompThing))
                return false;

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition(thing);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;        // killough 11/98

    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    return true;
}

//
// MOVEMENT ITERATOR FUNCTIONS
//

// killough 8/1/98: used to test intersection between thing and line
// assuming NO movement occurs -- used to avoid sticky situations.
static int untouched(line_t *ld)
{
    fixed_t x, y, tmbbox[4];

    return
        (tmbbox[BOXRIGHT] = (x = tmthing->x) + tmthing->radius) <= ld->bbox[BOXLEFT] ||
        (tmbbox[BOXLEFT] = x - tmthing->radius) >= ld->bbox[BOXRIGHT] ||
        (tmbbox[BOXTOP] = (y = tmthing->y) + tmthing->radius) <= ld->bbox[BOXBOTTOM] ||
        (tmbbox[BOXBOTTOM] = y - tmthing->radius) >= ld->bbox[BOXTOP] ||
        P_BoxOnLineSide(tmbbox, ld) != -1;
}

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
static boolean PIT_CheckLine(line_t *ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
        || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
        return true;

    if (P_BoxOnLineSide(tmbbox, ld) != -1)
        return true;

    // A line has been hit

    // The moving thing's destination position will cross
    // the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.

    // killough 7/24/98: allow player to move out of 1s wall, to prevent sticking
    if (!ld->backsector)                          // one sided line
    {
        blockline = ld;
        return (tmunstuck
                && !untouched(ld)
                && FixedMul(tmx - tmthing->x, ld->dy) > FixedMul(tmy - tmthing->y, ld->dx));
    }

    if (!(tmthing->flags & MF_MISSILE))
    {
        if (ld->flags & ML_BLOCKING)              // explicitly blocking everything
            return (tmunstuck && !untouched(ld)); // killough 8/1/98: allow escape

        if (!tmthing->player && (ld->flags & ML_BLOCKMONSTERS))
            return false;                         // block monsters only
    }

    // set openrange, opentop, openbottom
    P_LineOpening(ld);

    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
    {
        tmceilingz = opentop;
        ceilingline = ld;
        blockline = ld;
    }

    if (openbottom > tmfloorz)
    {
        tmfloorz = openbottom;
        floorline = ld;         // killough 8/1/98: remember floor linedef
        blockline = ld;
    }

    if (lowfloor < tmdropoffz)
        tmdropoffz = lowfloor;

    // if contacted a special line, add it to the list
    if (ld->special)
    {
        // 1/11/98 killough: remove limit on lines hit, by array doubling
        if (numspechit >= spechit_max)
        {
            spechit_max = (spechit_max ? spechit_max * 2 : 8);
            spechit = (line_t **)realloc(spechit, sizeof(*spechit) * spechit_max);
        }
        spechit[numspechit++] = ld;
    }

    if (tmthing->flags & MF_MISSILE)
        if (tmthing->z < openbottom || tmthing->z + tmthing->height > opentop)
            return false;       // hit upper or lower texture

    return true;
}

//
// PIT_CheckThing
//
boolean PIT_CheckThing(mobj_t *thing)
{
    fixed_t blockdist;
    int     damage;

    if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
        return true;

    blockdist = ((thing->flags & MF_SPECIAL) ? 20 * FRACUNIT : thing->radius) + tmthing->radius;

    if (ABS(thing->x - tmx) >= blockdist || ABS(thing->y - tmy) >= blockdist)
        return true;            // didn't hit it

    // don't clip against self
    if (thing == tmthing)
        return true;

    // see if it went over / under
    if (tmthing->z >= thing->z + thing->height)
    {
        if (!(thing->flags & MF_SPECIAL))
        {
            if ((thing->z + thing->height) > tmfloorz)
                tmfloorz = thing->z + thing->height;
            return true;        // overhead
        }
    }
    if (tmthing->z + tmthing->height < thing->z)
    {
        if (!(thing->flags & MF_SPECIAL))
        {
            if (thing->z < tmceilingz)
                tmceilingz = thing->z;
            return true;        // underneath
        }
    }

    // check for skulls slamming into things
    if ((tmthing->flags & MF_SKULLFLY) && (thing->flags & MF_SOLID))
    {
        damage = ((P_Random() % 8) + 1) * tmthing->info->damage;

        P_DamageMobj(thing, tmthing, tmthing, damage);

        tmthing->flags &= ~MF_SKULLFLY;
        tmthing->momx = tmthing->momy = tmthing->momz = 0;

        P_SetMobjState(tmthing, (statenum_t)tmthing->info->spawnstate);

        return false;           // stop moving
    }

    // missiles can hit other things
    if (tmthing->flags & MF_MISSILE)
    {
        int clipheight = (thing->projectilepassheight ? thing->projectilepassheight : thing->height);

        // see if it went over / under
        if (tmthing->z > thing->z + clipheight)
            return true;        // overhead

        if (tmthing->z + tmthing->height < thing->z)
            return true;        // underneath

        if (tmthing->target
            && (tmthing->target->type == thing->type ||
               (tmthing->target->type == MT_KNIGHT && thing->type == MT_BRUISER) ||
               (tmthing->target->type == MT_BRUISER && thing->type == MT_KNIGHT)))
        {
            // Don't hit same species as originator.
            if (thing == tmthing->target)
                return true;

            if (thing->type != MT_PLAYER)
            {
                // Explode, but do no damage.
                // Let players missile other players.
                return false;
            }
        }

        if (!(thing->flags & MF_SHOOTABLE))
            return !(thing->flags & MF_SOLID);         // didn't do any damage

        // damage / explode
        damage = ((P_Random() % 8) + 1) * tmthing->info->damage;
        P_DamageMobj(thing, tmthing, tmthing->target, damage);

        // don't traverse anymore
        return false;
    }

    // check for special pickup
    if (thing->flags & MF_SPECIAL)
    {
        boolean solid = thing->flags & MF_SOLID;

        if (tmflags & MF_PICKUP)
            P_TouchSpecialThing(thing, tmthing);        // can remove thing
        return !solid;
    }

    return !(thing->flags & MF_SOLID);
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
boolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y)
{
    int          xl;
    int          xh;
    int          yl;
    int          yh;
    int          bx;
    int          by;
    subsector_t  *newsubsec;

    tmthing = thing;
    tmflags = thing->flags;
    tmradius = tmthing->radius;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmradius;
    tmbbox[BOXBOTTOM] = y - tmradius;
    tmbbox[BOXRIGHT] = x + tmradius;
    tmbbox[BOXLEFT] = x - tmradius;

    newsubsec = R_PointInSubsector(x, y);
    floorline = blockline = ceilingline = NULL; // killough 8/1/98

    // Whether object can get out of a sticky situation:
    tmunstuck = (thing->player &&               // only players
                thing->player->mo == thing);    // not voodoo dolls

    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

    if (tmflags & MF_NOCLIP)
        return true;

    // Check things first, possibly picking things up.
    // The bounding box is extended by MAXRADIUS
    // because mobj_ts are grouped into mapblocks
    // based on their origin point, and can overlap
    // into adjacent blocks by up to MAXRADIUS units.
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_CheckThing))
                return false;

    // check lines
    xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockLinesIterator(bx, by, PIT_CheckLine))
                return false;

    return true;
}

//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//
boolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y, boolean dropoff)
{
    fixed_t oldx;
    fixed_t oldy;

    felldown = false;               // killough 11/98
    floatok = false;

    if (!P_CheckPosition(thing, x, y))
        return false;           // solid wall or thing

    if (!(thing->flags & MF_NOCLIP))
    {
        // killough 7/26/98: reformatted slightly
        // killough 8/1/98: Possibly allow escape if otherwise stuck

        if (tmceilingz - tmfloorz < thing->height ||     // doesn't fit
            // mobj must lower to fit
            (floatok = true, !(thing->flags & MF_TELEPORT) &&
            tmceilingz - thing->z < thing->height) ||
            // too big a step up
            (!(thing->flags & MF_TELEPORT) &&
            tmfloorz - thing->z > 24 * FRACUNIT))
            return tmunstuck
            && !(ceilingline && untouched(ceilingline))
            && !(floorline && untouched(floorline));

        //if (!(thing->flags & (MF_DROPOFF | MF_FLOAT)) && tmfloorz - tmdropoffz > 24 * FRACUNIT)
        //    return false;       // don't stand over a dropoff
        // killough 3/15/98: Allow certain objects to drop off
        // killough 7/24/98, 8/1/98: 
        // Prevent monsters from getting stuck hanging off ledges
        // killough 10/98: Allow dropoffs in controlled circumstances
        // killough 11/98: Improve symmetry of clipping on stairs
        if (!(thing->flags & (MF_DROPOFF | MF_FLOAT)))
            if (!dropoff || (dropoff == 2 && 
                (tmfloorz - tmdropoffz > 128 * FRACUNIT ||
                !thing->target || thing->target->z > tmdropoffz)))
            {
            felldown = !(thing->flags & MF_NOGRAVITY) &&
                thing->z - tmfloorz > 24 * FRACUNIT;

            // killough 11/98: prevent falling objects from going up too many steps
            if (thing->flags2 & MF2_FALLING && tmfloorz - thing->z >
                FixedMul(thing->momx, thing->momx) + FixedMul(thing->momy, thing->momy))
                return false;
            }
    }

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition(thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;      // killough 11/98: keep track of dropoffs
    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    // if any special lines were hit, do the effect
    if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)))
    {
        while (numspechit--)
        {
            // see if the line was crossed
            line_t *ld = spechit[numspechit];
            int    oldside = P_PointOnLineSide(oldx, oldy, ld);

            if (oldside != P_PointOnLineSide(thing->x, thing->y, ld) && ld->special)
                P_CrossSpecialLine(ld, oldside, thing);
        }
    }

    return true;
}

//
// killough 9/12/98:
//
// Apply "torque" to objects hanging off of ledges, so that they
// fall off. It's not really torque, since Doom has no concept of
// rotation, but it's a convincing effect which avoids anomalies
// such as lifeless objects hanging more than halfway off of ledges,
// and allows objects to roll off of the edges of moving lifts, or
// to slide up and then back down stairs, or to fall into a ditch.
// If more than one linedef is contacted, the effects are cumulative,
// so balancing is possible.
//

static boolean PIT_ApplyTorque(line_t *ld)
{
    if (ld->backsector &&       // If thing touches two-sided pivot linedef
        tmbbox[BOXRIGHT]  > ld->bbox[BOXLEFT] &&
        tmbbox[BOXLEFT]   < ld->bbox[BOXRIGHT] &&
        tmbbox[BOXTOP]    > ld->bbox[BOXBOTTOM] &&
        tmbbox[BOXBOTTOM] < ld->bbox[BOXTOP] &&
        P_BoxOnLineSide(tmbbox, ld) == -1)
    {
        mobj_t *mo = tmthing;

        fixed_t dist =                               // lever arm
            +(ld->dx >> FRACBITS) * (mo->y >> FRACBITS)
            - (ld->dy >> FRACBITS) * (mo->x >> FRACBITS)
            - (ld->dx >> FRACBITS) * (ld->v1->y >> FRACBITS)
            + (ld->dy >> FRACBITS) * (ld->v1->x >> FRACBITS);

        if (dist < 0 ?                               // dropoff direction
            ld->frontsector->floorheight < mo->z &&
            ld->backsector->floorheight >= mo->z :
            ld->backsector->floorheight < mo->z &&
            ld->frontsector->floorheight >= mo->z)
        {
            // At this point, we know that the object straddles a two-sided
            // linedef, and that the object's center of mass is above-ground.

            fixed_t x = abs(ld->dx), y = abs(ld->dy);

            if (y > x)
            {
                fixed_t t = x;
                x = y;
                y = t;
            }

            y = finesine[(tantoangle[FixedDiv(y, x) >> DBITS] +
                ANG90) >> ANGLETOFINESHIFT];

            // Momentum is proportional to distance between the
            // object's center of mass and the pivot linedef.
            //
            // It is scaled by 2^(OVERDRIVE - gear). When gear is
            // increased, the momentum gradually decreases to 0 for
            // the same amount of pseudotorque, so that oscillations
            // are prevented, yet it has a chance to reach equilibrium.

            dist = FixedDiv(FixedMul(dist, (mo->gear < OVERDRIVE) ?
                y << -(mo->gear - OVERDRIVE) :
                y >> +(mo->gear - OVERDRIVE)), x);

            // Apply momentum away from the pivot linedef.

            x = FixedMul(ld->dy, dist);
            y = FixedMul(ld->dx, dist);

            // Avoid moving too fast all of a sudden (step into "overdrive")

            dist = FixedMul(x, x) + FixedMul(y, y);

            while (dist > FRACUNIT * 4 && mo->gear < MAXGEAR)
                ++mo->gear, x >>= 1, y >>= 1, dist >>= 1;

            mo->momx -= x;
            mo->momy += y;
        }
    }
    return true;
}

//
// killough 9/12/98
//
// Applies "torque" to objects, based on all contacted linedefs
//

void P_ApplyTorque(mobj_t *mo)
{
    int xl = ((tmbbox[BOXLEFT] =
        mo->x - mo->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    int xh = ((tmbbox[BOXRIGHT] =
        mo->x + mo->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    int yl = ((tmbbox[BOXBOTTOM] =
        mo->y - mo->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    int yh = ((tmbbox[BOXTOP] =
        mo->y + mo->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    int bx, by, flags2 = mo->flags2; //Remember the current state, for gear-change

    tmthing = mo;
    validcount++; // prevents checking same line twice

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, PIT_ApplyTorque);

    // If any momentum, mark object as 'falling' using engine-internal flags
    if (mo->momx | mo->momy)
        mo->flags2 |= MF2_FALLING;
    else  // Clear the engine-internal flag indicating falling object.
        mo->flags2 &= ~MF2_FALLING;

    // If the object has been moving, step up the gear.
    // This helps reach equilibrium and avoid oscillations.
    //
    // Doom has no concept of potential energy, much less
    // of rotation, so we have to creatively simulate these 
    // systems somehow :)

    if (!((mo->flags2 | flags2) & MF2_FALLING))   // If not falling for a while,
        mo->gear = 0;                                // Reset it to full strength
    else
        if (mo->gear < MAXGEAR)                      // Else if not at max gear,
            mo->gear++;                                // move up a gear
}

static fixed_t pe_x;
static fixed_t pe_y;
static fixed_t ls_x;
static fixed_t ls_y;

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
static boolean PIT_CrossLine(line_t *ld)
{
    if (!(ld->flags & ML_TWOSIDED) || (ld->flags & (ML_BLOCKING | ML_BLOCKMONSTERS)))
    {
        if (!(tmbbox[BOXLEFT] > ld->bbox[BOXRIGHT]
            || tmbbox[BOXRIGHT] < ld->bbox[BOXLEFT]
            || tmbbox[BOXTOP] < ld->bbox[BOXBOTTOM]
            || tmbbox[BOXBOTTOM] > ld->bbox[BOXTOP]))
        {
            if (P_PointOnLineSide(pe_x, pe_y, ld) != P_PointOnLineSide(ls_x, ls_y, ld))
                return false;   // line blocks trajectory
        }
    }
    return true;                // line doesn't block trajectory
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
boolean P_CheckLineSide(mobj_t *actor, fixed_t x, fixed_t y)
{
    int bx;
    int by;
    int xl;
    int xh;
    int yl;
    int yh;

    pe_x = actor->x;
    pe_y = actor->y;
    ls_x = x;
    ls_y = y;

    // here is the bounding box of the trajectory
    tmbbox[BOXLEFT] = (pe_x < x ? pe_x : x);
    tmbbox[BOXRIGHT] = (pe_x > x ? pe_x : x);
    tmbbox[BOXTOP] = (pe_y > y ? pe_y : y);
    tmbbox[BOXBOTTOM] = (pe_y < y ? pe_y : y);

    // determine which blocks to look in for blocking lines
    xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;

    validcount++;               // prevents checking same line twice
    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockLinesIterator(bx, by, PIT_CrossLine))
                return true;
    return false;
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
boolean P_ThingHeightClip(mobj_t *thing)
{
    boolean onfloor = (thing->z == thing->floorz);

    P_CheckPosition(thing, thing->x, thing->y);
    // what about stranding a monster partially off an edge?

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;         // killough 11/98: remember dropoffs

    if (onfloor)
    {
        // walking monsters rise and fall with the floor
        thing->z = thing->floorz;
    }
    else
    {
        // don't adjust a floating monster unless forced to
        if (thing->z + thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;
    }

    if (thing->ceilingz - thing->floorz < thing->height)
        return false;

    return true;
}

//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
fixed_t bestslidefrac;

line_t  *bestslideline;

mobj_t  *slidemo;

fixed_t tmxmove;
fixed_t tmymove;

//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
void P_HitSlideLine(line_t *ld)
{
    int     side;

    angle_t lineangle;
    angle_t moveangle;
    angle_t deltaangle;

    fixed_t movelen;
    fixed_t newlen;

    if (ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = 0;
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {
        tmxmove = 0;
        return;
    }

    side = P_PointOnLineSide(slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);

    if (side == 1)
        lineangle += ANG180;

    moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove) + 10;
    moveangle += 10;    // prevents sudden path reversal due to rounding error
    deltaangle = moveangle - lineangle;

    if (deltaangle > ANG180)
        deltaangle += ANG180;

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_ApproxDistance(tmxmove, tmymove);
    newlen = FixedMul(movelen, finecosine[deltaangle]);

    tmxmove = FixedMul(newlen, finecosine[lineangle]);
    tmymove = FixedMul(newlen, finesine[lineangle]);
}

//
// PTR_SlideTraverse
//
boolean PTR_SlideTraverse(intercept_t *in)
{
    line_t *li = in->d.line;

    if (!(li->flags & ML_TWOSIDED))
    {
        if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
            return true;        // don't hit the back side
        goto isblocking;
    }

    // set openrange, opentop, openbottom
    P_LineOpening(li);

    if (openrange < slidemo->height)
        goto isblocking;        // doesn't fit

    if (opentop - slidemo->z < slidemo->height)
        goto isblocking;        // mobj is too high

    if (openbottom - slidemo->z > 24 * FRACUNIT)
        goto isblocking;        // too big a step up

    // this line doesn't block movement
    return true;

    // the line does block movement,
    // see if it is closer than best so far
isblocking:
    if (in->frac < bestslidefrac)
    {
        bestslidefrac = in->frac;
        bestslideline = li;
    }

    return false;               // stop
}

//
// P_SlideMove
// The momx/momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
void P_SlideMove(mobj_t *mo)
{
    fixed_t leadx, leady;
    fixed_t trailx, traily;
    int     hitcount = 0;

    slidemo = mo;

retry:
    if (++hitcount == 3)
        goto stairstep;         // don't loop forever

    // trace along the three leading corners
    if (mo->momx > 0)
    {
        leadx = mo->x + mo->radius;
        trailx = mo->x - mo->radius;
    }
    else
    {
        leadx = mo->x - mo->radius;
        trailx = mo->x + mo->radius;
    }

    if (mo->momy > 0)
    {
        leady = mo->y + mo->radius;
        traily = mo->y - mo->radius;
    }
    else
    {
        leady = mo->y - mo->radius;
        traily = mo->y + mo->radius;
    }

    bestslidefrac = FRACUNIT + 1;

    P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);

    // move up to the wall
    if (bestslidefrac == FRACUNIT + 1)
    {
        // the move must have hit the middle, so stairstep
stairstep:
        if (!P_TryMove(mo, mo->x, mo->y + mo->momy, true))
            P_TryMove(mo, mo->x + mo->momx, mo->y, true);
        return;
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;
    if (bestslidefrac > 0)
    {
        fixed_t newx = FixedMul(mo->momx, bestslidefrac);
        fixed_t newy = FixedMul(mo->momy, bestslidefrac);

        if (!P_TryMove(mo, mo->x + newx, mo->y + newy, true))
            goto stairstep;
    }

    // Now continue along the wall.
    // First calculate remainder.
    bestslidefrac = FRACUNIT - (bestslidefrac + 0x800);

    if (bestslidefrac > FRACUNIT)
        bestslidefrac = FRACUNIT;

    if (bestslidefrac <= 0)
        return;

    tmxmove = FixedMul(mo->momx, bestslidefrac);
    tmymove = FixedMul(mo->momy, bestslidefrac);

    P_HitSlideLine(bestslideline);      // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;

    if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove, true))
        goto retry;
}

//
// P_LineAttack
//
mobj_t         *linetarget;    // who got hit (or NULL)
mobj_t         *shootthing;

// height if not aiming up or down
fixed_t        shootz;

int            la_damage;
fixed_t        attackrange;

fixed_t        aimslope;

// slopes to top and bottom of target
static fixed_t topslope;
static fixed_t bottomslope;

//
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
//
boolean PTR_AimTraverse(intercept_t *in)
{
    line_t  *li;
    mobj_t  *th;
    fixed_t slope;
    fixed_t thingtopslope;
    fixed_t thingbottomslope;
    fixed_t dist;

    if (in->isaline)
    {
        li = in->d.line;

        if (!(li->flags & ML_TWOSIDED))
            return false;               // stop

        // Crosses a two sided line.
        // A two sided line will restrict
        // the possible target ranges.
        P_LineOpening(li);

        if (openbottom >= opentop)
            return false;               // stop

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
            return false;               // stop

        return true;                    // shot continues
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
        return true;                    // can't shoot self

    if (!(th->flags & MF_SHOOTABLE))
        return true;                    // corpse or something

    // check angles to see if the thing can be aimed at
    dist = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv(th->z + th->height - shootz, dist);

    if (thingtopslope < bottomslope)
        return true;                    // shot over the thing

    thingbottomslope = FixedDiv(th->z - shootz, dist);

    if (thingbottomslope > topslope)
        return true;                    // shot under the thing

    // this thing can be hit!
    if (thingtopslope > topslope)
        thingtopslope = topslope;

    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    aimslope = (thingtopslope + thingbottomslope) / 2;
    linetarget = th;

    return false;                       // don't go any farther
}

//
// PTR_ShootTraverse
//
boolean PTR_ShootTraverse(intercept_t *in)
{
    fixed_t x;
    fixed_t y;
    fixed_t z;
    fixed_t frac;

    mobj_t  *th;

    fixed_t slope;
    fixed_t dist;
    fixed_t thingtopslope;
    fixed_t thingbottomslope;

    if (in->isaline)
    {
        line_t *li = in->d.line;

        if (li->special)
            P_ShootSpecialLine(shootthing, li);

        if (!(li->flags & ML_TWOSIDED))
            goto hitline;

        // crosses a two sided line
        P_LineOpening(li);

        dist = FixedMul(attackrange, in->frac);

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv(openbottom - shootz, dist);
            if (slope > aimslope)
                goto hitline;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv(opentop - shootz, dist);
            if (slope < aimslope)
                goto hitline;
        }

        // shot continues
        return true;

        // hit line
hitline:
        // position a bit closer
        frac = in->frac - FixedDiv(4 * FRACUNIT, attackrange);
        x = trace.x + FixedMul(trace.dx, frac);
        y = trace.y + FixedMul(trace.dy, frac);
        z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

        if (li->frontsector->ceilingpic == skyflatnum)
        {
            // don't shoot the sky!
            if (z > li->frontsector->ceilingheight)
                return false;

            // it's a sky hack wall
            if (li->backsector && li->backsector->ceilingpic == skyflatnum
                && li->backsector->ceilingheight < z)
                return false;
        }

        // Spawn bullet puffs.
        P_SpawnPuff(x, y, z, shootangle, true);

        // don't go any farther
        return false;
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
        return true;                    // can't shoot self

    if (!(th->flags & MF_SHOOTABLE))
        return true;                    // corpse or something

    // check angles to see if the thing can be aimed at
    dist = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv(th->z + th->height - shootz, dist);

    if (thingtopslope < aimslope)
        return true;                    // shot over the thing

    thingbottomslope = FixedDiv(th->z - shootz, dist);

    if (thingbottomslope > aimslope)
        return true;                    // shot under the thing

    // hit thing
    // position a bit closer
    frac = in->frac - FixedDiv(10 * FRACUNIT, attackrange);

    x = trace.x + FixedMul(trace.dx, frac);
    y = trace.y + FixedMul(trace.dy, frac);
    z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

    // Spawn bullet puffs or blood spots,
    // depending on target type.
    if (th->flags & MF_NOBLOOD)
        P_SpawnPuff(x, y, z, shootangle, true);
    else
    {
        mobjtype_t type = th->type;

        if (type == MT_SKULL)
            P_SpawnPuff(x, y, z - FRACUNIT * 8, shootangle, true);
        else if (type != MT_PLAYER)
            P_SpawnBlood(x, y, z, shootangle, la_damage, th);
        else
        {
            player_t *player = &players[consoleplayer];

            if (!player->powers[pw_invulnerability] && !(player->cheats & CF_GODMODE))
            {
                if (player->powers[pw_invisibility])
                    type = MT_FUZZPLAYER;
                P_SpawnBlood(x, y, z + FRACUNIT * M_RandomInt(4, 16), shootangle, la_damage, th);
            }
        }
    }

    if (la_damage)
        P_DamageMobj(th, shootthing, shootthing, la_damage);

    // don't go any farther
    return false;
}

//
// P_AimLineAttack
//
fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance)
{
    fixed_t x2, y2;

    t1 = P_SubstNullMobj(t1);

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;

    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;

    // can't shoot outside view angles
    topslope = 100 * FRACUNIT / 160;
    bottomslope = -100 * FRACUNIT / 160;

    attackrange = distance;
    linetarget = NULL;

    P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS, PTR_AimTraverse);

    if (linetarget)
        return aimslope;

    return 0;
}

//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//
void P_LineAttack(mobj_t *t1, angle_t angle, fixed_t distance, fixed_t slope, int damage)
{
    fixed_t x2, y2;

    shootangle = angle;
    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    attackrange = distance;
    aimslope = slope;

    P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS, PTR_ShootTraverse);
}

//
// USE LINES
//
static mobj_t *usething;

static boolean PTR_UseTraverse(intercept_t *in)
{
    int  side;

    if (!in->d.line->special)
    {
        P_LineOpening(in->d.line);
        if (openrange <= 0)
        {
            S_StartSound(usething, sfx_noway);

            // can't use through a wall
            return false;
        }

        // not a special line, but keep checking
        return true;
    }

    side = 0;
    if (P_PointOnLineSide(usething->x, usething->y, in->d.line) == 1)
        side = 1;

    P_UseSpecialLine(usething, in->d.line, side);

    // can't use for more than one special line in a row
    return false;
}

// Returns false if a "oof" sound should be made because of a blocking
// linedef. Makes 2s middles which are impassable, as well as 2s uppers
// and lowers which block the player, cause the sound effect when the
// player tries to activate them. Specials are excluded, although it is
// assumed that all special linedefs within reach have been considered
// and rejected already (see P_UseLines).
//
// by Lee Killough
//
boolean PTR_NoWayTraverse(intercept_t *in)
{
    line_t *ld = in->d.line;

    return (ld->special 
            || !(ld->flags & ML_BLOCKING 
                 || (P_LineOpening(ld),
                     openrange <= 0 || 
                     openbottom > usething->z + 24 * FRACUNIT ||
                     opentop < usething->z + usething->height)));
}

//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines(player_t *player)
{
    int     angle;
    fixed_t x1, y1;
    fixed_t x2, y2;

    if (automapactive && !followplayer)
        return;

    usething = player->mo;

    angle = player->mo->angle >> ANGLETOFINESHIFT;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
    y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];

    // This added test makes the "oof" sound work on 2s lines -- killough:
    if (P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse))
        if (!P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_NoWayTraverse))
            S_StartSound(usething, sfx_noway);
}

//
// RADIUS ATTACK
//
mobj_t *bombsource;
mobj_t *bombspot;
int    bombdamage;

//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
boolean PIT_RadiusAttack(mobj_t *thing)
{
    fixed_t dx, dy, dz;
    fixed_t dist;

    if (!(thing->flags & MF_SHOOTABLE) && !(thing->flags & MF_CORPSE))
        return true;

    // Boss spider and cyborg
    // take no damage from concussion.
    if (thing->type == MT_CYBORG || thing->type == MT_SPIDER)
        return true;

    dx = ABS(thing->x - bombspot->x);
    dy = ABS(thing->y - bombspot->y);

    dist = MAX(dx, dy) - thing->radius;

    if (thing->type == MT_BOSSBRAIN)
    {
        // [BH] if killing boss in DOOM II MAP30, use old code that
        //  doesn't use z height in blast radius
        dist = MAX(0, dist >> FRACBITS);

        if (dist >= bombdamage)
            return true;        // out of range
    }
    else
    {
        dz = ABS(thing->z + (thing->height >> 1) - bombspot->z);
        dist = MAX(0, MAX (dist, dz) >> FRACBITS);

        if (dist >= bombdamage)
            return true;        // out of range

        if (thing->floorz > bombspot->z && bombspot->ceilingz < thing->z)
            return true;

        if (thing->ceilingz < bombspot->z && bombspot->floorz > thing->z)
            return true;
    }

    if (P_CheckSight(thing, bombspot))
    {
        // must be in direct path
        P_DamageMobj(thing, bombspot, bombsource, bombdamage - dist);
    }

    return true;
}

//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage)
{
    int x, y;

    fixed_t dist = (damage + MAXRADIUS) << FRACBITS;
    int     yh = (spot->y + dist - bmaporgy) >> MAPBLOCKSHIFT;
    int     yl = (spot->y - dist - bmaporgy) >> MAPBLOCKSHIFT;
    int     xh = (spot->x + dist - bmaporgx) >> MAPBLOCKSHIFT;
    int     xl = (spot->x - dist - bmaporgx) >> MAPBLOCKSHIFT;

    bombspot = spot;
    bombsource = source;
    bombdamage = damage;

    for (y = yl; y <= yh; y++)
        for (x = xl; x <= xh; x++)
            P_BlockThingsIterator(x, y, PIT_RadiusAttack);
}

//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//
static boolean crushchange;
static boolean nofit;

void (*bloodSplatSpawner)(fixed_t, fixed_t, void(*)(void));

//
// PIT_ChangeSector
//
boolean PIT_ChangeSector(mobj_t *thing)
{
    player_t *player;

    if (P_ThingHeightClip(thing))
    {
        // keep checking
        return true;
    }

    player = &players[consoleplayer];

    if (thing->type == MT_PLAYER && (player->health <= 0 ||  
        player->powers[pw_invulnerability] || (player->cheats & CF_GODMODE)))
    {
        nofit = true;
        return true;
    }

    // crunch bodies to giblets
    if (thing->health <= 0 && thing->type != MT_BARREL
        && thing->type != MT_SKULL && thing->type != MT_PAIN)
    {
        int     i;

        P_SetMobjState(thing, S_GIBS);

        thing->flags &= ~MF_SOLID;
        thing->flags2 = 0;
        thing->height = 0;
        thing->radius = 0;

        for (i = 0; i < M_RandomInt(100, 150); i++)
            bloodSplatSpawner(thing->x + (M_RandomInt(-28, 28) << FRACBITS),
                              thing->y + (M_RandomInt(-28, 28) << FRACBITS),
                              tl50colfunc);

        S_StartSound(thing, sfx_slop);

        // keep checking
        return true;
    }

    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
        P_RemoveMobj(thing);

        // keep checking
        return true;
    }

    if (!(thing->flags & MF_SHOOTABLE))
    {
        // assume it is bloody gibs or something
        return true;
    }

    nofit = true;

    if (crushchange && !(leveltime & 3))
    {
        mobjtype_t type = thing->type;

        P_DamageMobj(thing, NULL, NULL, 10);

        // spray blood in a random direction
        if (type != MT_BARREL && type != MT_SKULL)
        {
            if (type != MT_PLAYER
                || (type == MT_PLAYER
                    && !player->powers[pw_invulnerability]
                    && !(player->cheats & CF_GODMODE)))
            {
                int x = thing->x + M_RandomInt(-10, 10) * FRACUNIT;
                int y = thing->y + M_RandomInt(-10, 10) * FRACUNIT;
                int z = thing->z + thing->height / 2 + M_RandomInt(-10, 10) * FRACUNIT;

                P_SpawnBlood(x, y, z, 0, 10, thing);
            }
        }
    }

    // keep checking (crush other things)
    return true;
}

//
// P_ChangeSector
// jff 3/19/98 added to just check monsters on the periphery
// of a moving sector instead of all in bounding box of the
// sector. Both more accurate and faster.
// [BH] renamed from P_CheckSector to P_ChangeSector to replace old one entirely
//
boolean P_ChangeSector(sector_t *sector, boolean crunch)
{
    msecnode_t  *n;

    nofit = false;
    crushchange = crunch;

    // killough 4/4/98: scan list front-to-back until empty or exhausted,
    // restarting from beginning after each thing is processed. Avoids
    // crashes, and is sure to examine all things in the sector, and only
    // the things which are in the sector, until a steady-state is reached.
    // Things can arbitrarily be inserted and removed and it won't mess up.
    //
    // killough 4/7/98: simplified to avoid using complicated counter

    // Mark all things invalid
    for (n = sector->touching_thinglist; n; n = n->m_snext)
        n->visited = false;

    do
        for (n = sector->touching_thinglist; n; n = n->m_snext) // go through list
            if (!n->visited)                                    // unprocessed thing found
            {
                n->visited = true;                              // mark thing as processed
                if (!(n->m_thing->flags & MF_NOBLOCKMAP))       // jff 4/7/98 don't do these
                    PIT_ChangeSector(n->m_thing);               // process it
                break;                                         // exit and start over
            }
    while (n);  // repeat from scratch until all things left are marked valid

    return nofit;
}

// phares 3/21/98
//
// Maintain a freelist of msecnode_t's to reduce memory allocs and frees.
msecnode_t *headsecnode = NULL;

// P_GetSecnode() retrieves a node from the freelist. The calling routine
// should make sure it sets all fields properly.
//
// killough 11/98: reformatted
static msecnode_t *P_GetSecnode(void)
{
    msecnode_t  *node;

    return (headsecnode ?
        node = headsecnode, headsecnode = node->m_snext, node :
        Z_Malloc(sizeof *node, PU_LEVEL, NULL));
}

// P_PutSecnode() returns a node to the freelist.
static void P_PutSecnode(msecnode_t *node)
{
    node->m_snext = headsecnode;
    headsecnode = node;
}

// phares 3/16/98
//
// P_AddSecnode() searches the current list to see if this sector is
// already there. If not, it adds a sector node at the head of the list of
// sectors this object appears in. This is called when creating a list of
// nodes that will get linked in later. Returns a pointer to the new node.
//
// killough 11/98: reformatted
static msecnode_t *P_AddSecnode(sector_t *s, mobj_t *thing, msecnode_t *nextnode)
{
    msecnode_t  *node;

    for (node = nextnode; node; node = node->m_tnext)
        if (node->m_sector == s)        // Already have a node for this sector?
        {
            node->m_thing = thing;      // Yes. Setting m_thing says 'keep it'.
            return nextnode;
        }

    // Couldn't find an existing node for this sector. Add one at the head
    // of the list.
    node = P_GetSecnode();

    node->visited = 0;                          // killough 4/4/98, 4/7/98: mark new nodes unvisited.

    node->m_sector = s;                         // sector
    node->m_thing = thing;                      // mobj
    node->m_tprev = NULL;                       // prev node on Thing thread
    node->m_tnext = nextnode;                   // next node on Thing thread

    if (nextnode)
        nextnode->m_tprev = node;               // set back link on Thing

    // Add new node at head of sector thread starting at s->touching_thinglist
    node->m_sprev = NULL;                       // prev node on sector thread
    node->m_snext = s->touching_thinglist;      // next node on sector thread
    if (s->touching_thinglist)
        node->m_snext->m_sprev = node;
    return s->touching_thinglist = node;
}

// P_DelSecnode() deletes a sector node from the list of
// sectors this object appears in. Returns a pointer to the next node
// on the linked list, or NULL.
//
// killough 11/98: reformatted
static msecnode_t *P_DelSecnode(msecnode_t *node)
{
    if (node)
    {
        msecnode_t      *tp = node->m_tprev;    // prev node on thing thread
        msecnode_t      *tn = node->m_tnext;    // next node on thing thread
        msecnode_t      *sp;                    // prev node on sector thread
        msecnode_t      *sn;                    // next node on sector thread

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

        node = tn;
    }
    return node;
}

// Delete an entire sector list
void P_DelSeclist(msecnode_t *node)
{
    while (node)
        node = P_DelSecnode(node);
}

// phares 3/14/98
//
// PIT_GetSectors
// Locates all the sectors the object is in by looking at the lines that
// cross through it. You have already decided that the object is allowed
// at this location, so don't bother with checking impassable or
// blocking lines.
static boolean PIT_GetSectors(line_t *ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
        tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] ||
        tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] ||
        tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
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

    // killough 3/27/98, 4/4/98:
    // Use sidedefs instead of 2s flag to determine two-sidedness.
    // killough 8/1/98: avoid duplicate if same sector on both sides
    if (ld->backsector && ld->backsector != ld->frontsector)
        sector_list = P_AddSecnode(ld->backsector, tmthing, sector_list);

    return true;
}

// phares 3/14/98
//
// P_CreateSecNodeList alters/creates the sector_list that shows what sectors
// the object resides in.
//
// killough 11/98: reformatted
void P_CreateSecNodeList(mobj_t *thing, fixed_t x, fixed_t y)
{
    int xl, xh, yl, yh, bx, by;
    msecnode_t *node;

    // First, clear out the existing m_thing fields. As each node is
    // added or verified as needed, m_thing will be set properly. When
    // finished, delete all nodes where m_thing is still NULL. These
    // represent the sectors the Thing has vacated.
    for (node = sector_list; node; node = node->m_tnext)
        node->m_thing = NULL;

    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    validcount++;       // used to make sure we only process a line once

    xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, PIT_GetSectors);

    // Add the sector of the (x,y) point to sector_list.
    sector_list = P_AddSecnode(thing->subsector->sector, thing, sector_list);

    // Now delete any nodes that won't be used. These are the ones where
    // m_thing is still NULL.
    for (node = sector_list; node;)
        if (node->m_thing == NULL)
        {
            if (node == sector_list)
                sector_list = node->m_tnext;
            node = P_DelSecnode(node);
        }
        else
            node = node->m_tnext;
}
