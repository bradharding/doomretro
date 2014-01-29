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

#include <stdlib.h>

#include "m_bbox.h"
#include "m_random.h"
#include "i_system.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

// State.
#include "doomstat.h"
#include "r_state.h"
// Data.
#include "sounds.h"


fixed_t         tmbbox[4];
mobj_t          *tmthing;
int             tmflags;
fixed_t         tmx;
fixed_t         tmy;


// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean         floatok;

fixed_t         tmfloorz;
fixed_t         tmceilingz;
fixed_t         tmdropoffz;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t          *ceilingline;

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid
#define MAXSPECIALCROSS         20

line_t          *spechit[MAXSPECIALCROSS];
int             numspechit;

angle_t         shootangle;

extern int      followplayer;



//
// TELEPORT MOVE
//

//
// PIT_StompThing
//
boolean PIT_StompThing(mobj_t *thing)
{
    fixed_t     blockdist;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (ABS(thing->x - tmx) >= blockdist
        || ABS(thing->y - tmy) >= blockdist)
    {
        // didn't hit it
        return true;
    }

    // don't clip against self
    if (thing == tmthing)
        return true;

    // monsters don't stomp things except on boss level
    if (!tmthing->player && gamemap != 30)
        return false;

    P_DamageMobj(thing, tmthing, tmthing, 10000);

    return true;
}


//
// P_TeleportMove
//
boolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z)
{
    int                 xl;
    int                 xh;
    int                 yl;
    int                 yh;
    int                 bx;
    int                 by;

    subsector_t         *newsubsec;

    // kill anything occupying the position
    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

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
    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    return true;
}


//
// MOVEMENT ITERATOR FUNCTIONS
//


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

    if (!ld->backsector)
        return false;           // one sided line

    if (!(tmthing->flags & MF_MISSILE))
    {
        if (ld->flags & ML_BLOCKING)
            return false;       // explicitly blocking everything

        if (!tmthing->player && (ld->flags & ML_BLOCKMONSTERS))
            return false;       // block monsters only
    }

    // set openrange, opentop, openbottom
    P_LineOpening(ld);

    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
    {
        tmceilingz = opentop;
        ceilingline = ld;
    }

    if (openbottom > tmfloorz)
        tmfloorz = openbottom;

    if (lowfloor < tmdropoffz)
        tmdropoffz = lowfloor;

    // if contacted a special line, add it to the list
    if (ld->special)
    {
        spechit[numspechit] = ld;
        numspechit++;
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
    fixed_t             blockdist;
    boolean             solid;
    int                 damage;

    if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (ABS(thing->x - tmx) >= blockdist
        || ABS(thing->y - tmy) >= blockdist)
    {
        // didn't hit it
        return true;
    }

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
    if ((tmthing->flags & MF_SKULLFLY)
        && (thing->flags & MF_SOLID))
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
        // see if it went over / under
        if (tmthing->z > thing->z + thing->height)
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
        {
            // didn't do any damage
            return !(thing->flags & MF_SOLID);
        }

        // damage / explode
        damage = ((P_Random() % 8) + 1) * tmthing->info->damage;
        P_DamageMobj(thing, tmthing, tmthing->target, damage);

        // don't traverse anymore
        return false;
    }

    // check for special pickup
    if (thing->flags & MF_SPECIAL)
    {
        solid = thing->flags & MF_SOLID;
        if (tmflags & MF_PICKUP)
        {
            // can remove thing
            P_TouchSpecialThing(thing, tmthing);
        }
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
    int                 xl;
    int                 xh;
    int                 yl;
    int                 yh;
    int                 bx;
    int                 by;
    subsector_t         *newsubsec;

    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector(x, y);
    ceilingline = NULL;

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

    if (!(tmflags & MF_CORPSE))
    {
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
    }

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
boolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y)
{
    fixed_t     oldx;
    fixed_t     oldy;
    int         side;
    int         oldside;
    line_t      *ld;

    floatok = false;
    if (!P_CheckPosition(thing, x, y))
        return false;           // solid wall or thing

    if (!(thing->flags & MF_NOCLIP))
    {
        if (tmceilingz - tmfloorz < thing->height)
            return false;       // doesn't fit

        floatok = true;

        if (!(thing->flags & MF_TELEPORT)
            && tmceilingz - thing->z < thing->height)
            return false;       // mobj must lower itself to fit

        if (!(thing->flags & MF_TELEPORT)
            && tmfloorz - thing->z > 24 * FRACUNIT)
            return false;       // too big a step up

        if (!(thing->flags & (MF_DROPOFF | MF_FLOAT))
            && tmfloorz - tmdropoffz > 24 * FRACUNIT)
            return false;       // don't stand over a dropoff
    }

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition(thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    // if any special lines were hit, do the effect
    if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)))
    {
        while (numspechit--)
        {
            // see if the line was crossed
            ld = spechit[numspechit];
            side = P_PointOnLineSide(thing->x, thing->y, ld);
            oldside = P_PointOnLineSide(oldx, oldy, ld);
            if (side != oldside)
            {
                if (ld->special)
                    P_CrossSpecialLine(ld - lines, oldside, thing);
            }
        }
    }

    return true;
}



static fixed_t   pe_x;
static fixed_t   pe_y;
static fixed_t   ls_x;
static fixed_t   ls_y;


//
// PIT_CrossLine
//
boolean PIT_CrossLine(line_t *ld)
{
    if (!(ld->flags & ML_TWOSIDED)
        ||  (ld->flags & (ML_BLOCKING | ML_BLOCKMONSTERS)))
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
boolean P_CheckLineSide(mobj_t *actor, int x, int y)
{
    int         bx;
    int         by;
    int         xl;
    int         xh;
    int         yl;
    int         yh;

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
    boolean onfloor;

    onfloor = (thing->z == thing->floorz);

    P_CheckPosition(thing, thing->x, thing->y);
    // what about stranding a monster partially off an edge?

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;

    if (onfloor)
    {
        // walking monsters rise and fall with the floor
        thing->z = thing->floorz;
    }
    else
        // don't adjust a floating monster unless forced to
        if (thing->z + thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;

    if (thing->ceilingz - thing->floorz < thing->height)
        return false;

    return true;
}



//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
fixed_t         bestslidefrac;
fixed_t         secondslidefrac;

line_t          *bestslideline;
line_t          *secondslideline;

mobj_t          *slidemo;

fixed_t         tmxmove;
fixed_t         tmymove;



//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
void P_HitSlideLine(line_t *ld)
{
    int         side;

    angle_t     lineangle;
    angle_t     moveangle;
    angle_t     deltaangle;

    fixed_t     movelen;
    fixed_t     newlen;

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

    moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
    moveangle += 10;
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
    line_t      *li;

    li = in->d.line;

    if (!(li->flags & ML_TWOSIDED))
    {
        if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
        {
            // don't hit the back side
            return true;
        }
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
        secondslidefrac = bestslidefrac;
        secondslideline = bestslideline;
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
    fixed_t             leadx;
    fixed_t             leady;
    fixed_t             trailx;
    fixed_t             traily;
    fixed_t             newx;
    fixed_t             newy;
    int                 hitcount;

    slidemo = mo;
    hitcount = 0;

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
        if (!P_TryMove(mo, mo->x, mo->y + mo->momy))
            P_TryMove(mo, mo->x + mo->momx, mo->y);
        return;
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;
    if (bestslidefrac > 0)
    {
        newx = FixedMul(mo->momx, bestslidefrac);
        newy = FixedMul(mo->momy, bestslidefrac);

        if (!P_TryMove(mo, mo->x + newx, mo->y + newy))
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

    if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove))
    {
        goto retry;
    }
}


//
// P_LineAttack
//
mobj_t                  *linetarget;    // who got hit (or NULL)
mobj_t                  *shootthing;

// height if not aiming up or down
// ???: use slope for monsters?
fixed_t                 shootz;

int                     la_damage;
fixed_t                 attackrange;

fixed_t                 aimslope;

// slopes to top and bottom of target
extern fixed_t          topslope;
extern fixed_t          bottomslope;


//
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
//
boolean PTR_AimTraverse(intercept_t *in)
{
    line_t              *li;
    mobj_t              *th;
    fixed_t             slope;
    fixed_t             thingtopslope;
    fixed_t             thingbottomslope;
    fixed_t             dist;

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
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
    fixed_t             frac;

    line_t              *li;

    mobj_t              *th;

    fixed_t             slope;
    fixed_t             dist;
    fixed_t             thingtopslope;
    fixed_t             thingbottomslope;

    if (in->isaline)
    {
        li = in->d.line;

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
            slope = FixedDiv (opentop - shootz, dist);
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
        P_SpawnPuff(x, y, z, shootangle);

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
    if (in->d.thing->flags & MF_NOBLOOD)
        P_SpawnPuff(x, y, z, shootangle);
    else if (in->d.thing->type == MT_SKULL)
        P_SpawnPuff(x, y, z - FRACUNIT * 8, shootangle);
    else if (in->d.thing->type != MT_PLAYER
             || (in->d.thing->type == MT_PLAYER
                 && !players[consoleplayer].powers[pw_invulnerability]
                 && !(players[consoleplayer].cheats & CF_GODMODE)))
    {
        if (in->d.thing->type == MT_PLAYER)
            z += FRACUNIT * 8;

        if (in->d.thing->type == MT_HEAD)
            P_SpawnBlood(x, y, z, shootangle, la_damage, MF2_TRANSLUCENT_REDTOBLUE_50);
        else if (in->d.thing->type == MT_BRUISER
                 || in->d.thing->type == MT_KNIGHT)
            P_SpawnBlood(x, y, z, shootangle, la_damage, MF2_TRANSLUCENT_REDTOGREEN_50);
        else
            P_SpawnBlood(x, y, z, shootangle, la_damage, MF2_TRANSLUCENT_50);
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
    fixed_t             x2;
    fixed_t             y2;

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

    P_PathTraverse(t1->x, t1->y,
                   x2, y2,
                   PT_ADDLINES | PT_ADDTHINGS,
                   PTR_AimTraverse);

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
    fixed_t             x2;
    fixed_t             y2;

    shootangle = angle;
    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    attackrange = distance;
    aimslope = slope;

    P_PathTraverse(t1->x, t1->y,
                   x2, y2,
                   PT_ADDLINES | PT_ADDTHINGS,
                   PTR_ShootTraverse);
}



//
// USE LINES
//
static mobj_t           *usething;

static boolean PTR_UseTraverse(intercept_t *in)
{
    int                 side;

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


boolean PTR_NoWayTraverse(intercept_t *in)
{
    line_t *ld = in->d.line;

    return (ld->special || !(ld->flags & ML_BLOCKING || (P_LineOpening(ld),
        openrange <= 0 || openbottom > usething->z + 24 * FRACUNIT ||
        opentop < usething->z + usething->height)));
}


//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines(player_t *player)
{
    int         angle;
    fixed_t     x1;
    fixed_t     y1;
    fixed_t     x2;
    fixed_t     y2;

    if (automapactive && !followplayer)
        return;

    usething = player->mo;

    angle = player->mo->angle >> ANGLETOFINESHIFT;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
    y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];

    if (P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse))
        if (!P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_NoWayTraverse))
            S_StartSound(usething, sfx_noway);
}


//
// RADIUS ATTACK
//
mobj_t                  *bombsource;
mobj_t                  *bombspot;
int                     bombdamage;


//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
boolean PIT_RadiusAttack(mobj_t *thing)
{
    fixed_t             dx;
    fixed_t             dy;
    fixed_t             dz;
    fixed_t             dist;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    // Boss spider and cyborg
    // take no damage from concussion.
    if (thing->type == MT_CYBORG
        || thing->type == MT_SPIDER)
        return true;

    dx = ABS(thing->x - bombspot->x);
    dy = ABS(thing->y - bombspot->y);

    dist = MAX(dx, dy);
    dist -= thing->radius;

    if (thing->type == MT_BOSSBRAIN)
    {
        dist >>= FRACBITS;

        dist = MAX(0, dist);

        if (dist >= bombdamage)
            return true;        // out of range
    }
    else
    {
        dz = ABS(thing->z + (thing->height >> 1) - bombspot->z);
        dist = (dist > dz ? dist : dz) >> FRACBITS;

        dist = MAX(0, dist);

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
    int                 x;
    int                 y;

    int                 xl;
    int                 xh;
    int                 yl;
    int                 yh;

    fixed_t             dist;

    dist = (damage + MAXRADIUS) << FRACBITS;
    yh = (spot->y + dist - bmaporgy) >> MAPBLOCKSHIFT;
    yl = (spot->y - dist - bmaporgy) >> MAPBLOCKSHIFT;
    xh = (spot->x + dist - bmaporgx) >> MAPBLOCKSHIFT;
    xl = (spot->x - dist - bmaporgx) >> MAPBLOCKSHIFT;
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
boolean                 crushchange;
boolean                 nofit;


//
// PIT_ChangeSector
//
boolean PIT_ChangeSector(mobj_t *thing)
{
    player_t            *player;

    if (P_ThingHeightClip(thing))
    {
        // keep checking
        return true;
    }

    player = &players[consoleplayer];

    if (thing->type == MT_PLAYER &&
        (!player->health ||
         player->powers[pw_invulnerability] ||
         (player->cheats & CF_GODMODE)))
    {
        nofit = true;
        return true;
    }

    // crunch bodies to giblets
    if (thing->health <= 0
        && thing->type != MT_BARREL
        && thing->type != MT_SKULL
        && thing->type != MT_PAIN)
    {
        P_SetMobjState(thing, S_GIBS);

        thing->flags &= ~MF_SOLID;
        thing->flags2 = 0;
        thing->height = 0;
        thing->radius = 0;

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
        P_DamageMobj(thing, NULL, NULL, 10);

        // spray blood in a random direction
        if (thing->type != MT_BARREL
            && thing->type != MT_SKULL)
        {
            if (thing->type != MT_PLAYER
                || (thing->type == MT_PLAYER
                    && !player->powers[pw_invulnerability]
                    && !(player->cheats & CF_GODMODE)))
            {
                int x = thing->x + M_RandomInt(-10, 10) * FRACUNIT;
                int y = thing->y + M_RandomInt(-10, 10) * FRACUNIT;
                int z = thing->z + thing->height / 2 + M_RandomInt(-10, 10) * FRACUNIT;

                if (thing->type == MT_HEAD)
                    P_SpawnBlood(x, y, z, 0, 10, MF2_TRANSLUCENT_REDTOBLUE_50);
                else if (thing->type == MT_BRUISER
                         || thing->type == MT_KNIGHT)
                    P_SpawnBlood(x, y, z, 0, 10, MF2_TRANSLUCENT_REDTOGREEN_50);
                else
                    P_SpawnBlood(x, y, z, 0, 10, MF2_TRANSLUCENT_50);
            }
        }
    }

    // keep checking (crush other things)
    return true;
}


//
// P_CheckSector
// jff 3/19/98 added to just check monsters on the periphery
// of a moving sector instead of all in bounding box of the
// sector. Both more accurate and faster.
//
boolean P_ChangeSector(sector_t *sector, boolean crunch)
{
    mobj_t *thing;

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

    for (thing = sector->thinglist; thing; thing = thing->snext)
        thing->visited = false;

    do
    {
        for (thing = sector->thinglist; thing; thing = thing->snext) // go through list
            if (!thing->visited)                        // unprocessed thing found
            {
                thing->visited = true;                  // mark thing as processed
                if (!(thing->flags & MF_NOBLOCKMAP))    // jff 4/7/98 don't do these
                    PIT_ChangeSector(thing);            // process it
                break;                                  // exit and start over
            }
    }
    while (thing);      // repeat from scratch until all things left are marked valid

    return nofit;
}