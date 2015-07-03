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

#include <ctype.h>

#include "doomstat.h"
#include "g_game.h"
#include "m_bbox.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"

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

void A_Fall(mobj_t *actor);

extern boolean  smoketrails;

//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//

//
// P_RecursiveSound
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//
// killough 5/5/98: reformatted, cleaned up
//
static void P_RecursiveSound(sector_t *sec, int soundblocks, mobj_t *soundtarget)
{
    int i;

    if (players[0].cheats & CF_NOTARGET)
        return;

    // wake up all monsters in this sector
    if (sec->validcount == validcount && sec->soundtraversed <= soundblocks + 1)
        return;         // already flooded

    sec->validcount = validcount;
    sec->soundtraversed = soundblocks + 1;
    P_SetTarget(&sec->soundtarget, soundtarget);

    for (i = 0; i < sec->linecount; i++)
    {
        sector_t        *other;
        line_t          *check = sec->lines[i];

        if (!(check->flags & ML_TWOSIDED))
            continue;

        P_LineOpening(check);

        if (openrange <= 0)
            continue;   // closed door

        other = sides[check->sidenum[sides[check->sidenum[0]].sector == sec]].sector;

        if (!(check->flags & ML_SOUNDBLOCK))
            P_RecursiveSound(other, soundblocks, soundtarget);
        else if (!soundblocks)
            P_RecursiveSound(other, 1, soundtarget);
    }
}

//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert(mobj_t *target, mobj_t *emmiter)
{
    validcount++;
    P_RecursiveSound(emmiter->subsector->sector, 0, target);
}

//
// P_CheckMeleeRange
//
boolean P_CheckMeleeRange(mobj_t *actor)
{
    mobj_t      *pl = actor->target;
    fixed_t     dist;

    if (!pl)
        return false;

    dist = P_ApproxDistance(pl->x - actor->x, pl->y - actor->y);

    if (dist >= MELEERANGE - 20 * FRACUNIT + pl->info->radius)
        return false;

    // [BH] check difference in height as well
    if (pl->z > actor->z + actor->height || actor->z > pl->z + pl->height)
        return false;

    if (!P_CheckSight(actor, pl))
        return false;

    return true;
}

//
// P_CheckMissileRange
//
static boolean P_CheckMissileRange(mobj_t *actor)
{
    fixed_t     dist;

    if (!P_CheckSight(actor, actor->target))
        return false;

    if (actor->flags & MF_JUSTHIT)
    {
        // the target just hit the enemy, so fight back!
        actor->flags &= ~MF_JUSTHIT;
        return true;
    }

    if (actor->reactiontime)
        return false;                   // do not attack yet

    dist = P_ApproxDistance(actor->x - actor->target->x,
                            actor->y - actor->target->y) - 64 * FRACUNIT;

    if (!actor->info->meleestate)
        dist -= 128 * FRACUNIT;         // no melee attack, so fire more

    dist >>= FRACBITS;

    if (actor->type == MT_VILE)
        if (dist > 14 * 64)
            return false;               // too far away

    if (actor->type == MT_UNDEAD)
    {
        if (dist < 196)
            return false;               // close for fist attack
        dist >>= 1;
    }

    if (actor->type == MT_CYBORG || actor->type == MT_SPIDER || actor->type == MT_SKULL)
        dist >>= 1;

    if (dist > 200)
        dist = 200;

    if (actor->type == MT_CYBORG && dist > 160)
        dist = 160;

    if (P_Random() < dist)
        return false;

    return true;
}

//
// P_IsOnLift
//
// killough 9/9/98:
//
// Returns true if the object is on a lift. Used for AI,
// since it may indicate the need for crowded conditions,
// or that a monster should stay on the lift for a while
// while it goes up or down.
//
static boolean P_IsOnLift(const mobj_t *actor)
{
    const sector_t      *sec = actor->subsector->sector;
    line_t              line;

    // Short-circuit: it's on a lift which is active.
    if (sec->floordata && ((thinker_t *)sec->floordata)->function == T_PlatRaise)
        return true;

    // Check to see if it's in a sector which can be activated as a lift.
    if ((line.tag = sec->tag))
    {
        int     l;

        for (l = -1; (l = P_FindLineFromLineTag(&line, l)) >= 0;)
            switch (lines[l].special)
            {
                case W1_Lift_LowerWaitRaise:
                case S1_Floor_RaiseBy32_ChangesTexture:
                case S1_Floor_RaiseBy24_ChangesTexture:
                case S1_Floor_RaiseToNextHighestFloor_ChangesTexture:
                case S1_Lift_LowerWaitRaise:
                case W1_Floor_RaiseToNextHighestFloor_ChangesTexture:
                case G1_Floor_RaiseToNextHighestFloor_ChangesTexture:
                case W1_Floor_StartMovingUpAndDown:
                case SR_Lift_LowerWaitRaise:
                case SR_Floor_RaiseBy24_ChangesTexture:
                case SR_Floor_RaiseBy32_ChangesTexture:
                case SR_Floor_RaiseToNextHighestFloor_ChangesTexture:
                case WR_Floor_StartMovingUpAndDown:
                case WR_Lift_LowerWaitRaise:
                case WR_Floor_RaiseToNextHighestFloor_ChangesTexture:
                case WR_Lift_LowerWaitRaise_Fast:
                case W1_Lift_LowerWaitRaise_Fast:
                case S1_Lift_LowerWaitRaise_Fast:
                case SR_Lift_LowerWaitRaise_Fast:
                    return true;
            }
        }

    return false;
}

//
// P_IsUnderDamage
//
// killough 9/9/98:
//
// Returns nonzero if the object is under damage based on
// their current position. Returns 1 if the damage is moderate,
// -1 if it is serious. Used for AI.
//
static int P_IsUnderDamage(mobj_t *actor)
{
    const struct msecnode_s     *seclist;
    int                         dir = 0;

    for (seclist = actor->touching_sectorlist; seclist; seclist = seclist->m_tnext)
    {
        const ceiling_t          *cl;    // Crushing ceiling

        if ((cl = seclist->m_sector->ceilingdata) && cl->thinker.function == T_MoveCeiling)
            dir |= cl->direction;
    }
    return dir;
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
fixed_t         xspeed[8] = { FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000 };
fixed_t         yspeed[8] = { 0, 47000, FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000 };

// 1/11/98 killough: Limit removed on special lines crossed
extern line_t   **spechit;
extern int      numspechit;

static boolean P_Move(mobj_t *actor, boolean dropoff)   // killough 9/12/98
{
    fixed_t     tryx, tryy, deltax, deltay;
    boolean     try_ok;
    int         movefactor = ORIG_FRICTION_FACTOR;      // killough 10/98
    int         friction = ORIG_FRICTION;
    int         speed;

    if (actor->movedir == DI_NODIR)
        return false;

    // [RH] Instead of yanking non-floating monsters to the ground,
    // let gravity drop them down, unless they're moving down a step.
    if (!(actor->flags & MF_NOGRAVITY) && actor->z > actor->floorz
        && !(actor->flags2 & MF2_ONMOBJ))
    {
        if (actor->z > actor->floorz + 24 * FRACUNIT)
            return false;
        else
            actor->z = actor->floorz;
    }

    // killough 10/98: make monsters get affected by ice and sludge too:
    movefactor = P_GetMoveFactor(actor, &friction);
    speed = actor->info->speed;

    if (friction < ORIG_FRICTION        // sludge
        && !(speed = ((ORIG_FRICTION_FACTOR - (ORIG_FRICTION_FACTOR - movefactor) / 2)
        * speed) / ORIG_FRICTION_FACTOR))
        speed = 1;                      // always give the monster a little bit of speed

    tryx = actor->x + (deltax = speed * xspeed[actor->movedir]);
    tryy = actor->y + (deltay = speed * yspeed[actor->movedir]);

    // killough 12/98: rearrange, fix potential for stickiness on ice
    if (friction <= ORIG_FRICTION)
        try_ok = P_TryMove(actor, tryx, tryy, dropoff);
    else
    {
        fixed_t x = actor->x;
        fixed_t y = actor->y;
        fixed_t floorz = actor->floorz;
        fixed_t ceilingz = actor->ceilingz;
        fixed_t dropoffz = actor->dropoffz;

        try_ok = P_TryMove(actor, tryx, tryy, dropoff);

        // killough 10/98:
        // Let normal momentum carry them, instead of steptoeing them across ice.
        if (try_ok)
        {
            P_UnsetThingPosition(actor);
            actor->x = x;
            actor->y = y;
            actor->floorz = floorz;
            actor->ceilingz = ceilingz;
            actor->dropoffz = dropoffz;
            P_SetThingPosition(actor);
            movefactor *= FRACUNIT / ORIG_FRICTION_FACTOR / 4;
            actor->momx += FixedMul(deltax, movefactor);
            actor->momy += FixedMul(deltay, movefactor);
        }
    }

    if (!try_ok)
    {
        // open any specials
        int     good;

        if (actor->flags & MF_FLOAT && floatok)
        {
            if (actor->z < tmfloorz)          // must adjust height
                actor->z += FLOATSPEED;
            else
                actor->z -= FLOATSPEED;

            actor->flags |= MF_INFLOAT;

            return true;
        }

        if (!numspechit)
            return false;

        actor->movedir = DI_NODIR;

        // if the special is not a door that can be opened, return false
        //
        // killough 8/9/98: this is what caused monsters to get stuck in
        // doortracks, because it thought that the monster freed itself
        // by opening a door, even if it was moving towards the doortrack,
        // and not the door itself.
        //
        // killough 9/9/98: If a line blocking the monster is activated,
        // return true 90% of the time. If a line blocking the monster is
        // not activated, but some other line is, return false 90% of the
        // time. A bit of randomness is needed to ensure it's free from
        // lockups, but for most cases, it returns the correct result.
        //
        // Do NOT simply return false 1/4th of the time (causes monsters to
        // back out when they shouldn't, and creates secondary stickiness).
        for (good = false; numspechit--;)
            if (P_UseSpecialLine(actor, spechit[numspechit], 0))
                good |= spechit[numspechit] == blockline ? 1 : 2;

        return good && ((P_Random() >= 230) ^ (good & 1));
    }
    else
        actor->flags &= ~MF_INFLOAT;

    // killough 11/98: fall more slowly, under gravity, if felldown==true
    if (!(actor->flags & MF_FLOAT) && !felldown)
        actor->z = actor->floorz;

    return true;
}

//
// P_SmartMove
//
// killough 9/12/98: Same as P_Move, except smarter
//
static boolean P_SmartMove(mobj_t *actor)
{
    mobj_t      *target = actor->target;
    int         on_lift;
    int         under_damage;

    // killough 9/12/98: Stay on a lift if target is on one
    on_lift = (target && target->health > 0
        && target->subsector->sector->tag == actor->subsector->sector->tag && P_IsOnLift(actor));

    under_damage = P_IsUnderDamage(actor);

    if (!P_Move(actor, false))
        return false;

    // killough 9/9/98: avoid crushing ceilings or other damaging areas
    if ((on_lift && P_Random() < 230         // Stay on lift
         && !P_IsOnLift(actor))
        || (!under_damage                    // Get away from damage
            && (under_damage = P_IsUnderDamage(actor))
            && (under_damage < 0 || P_Random() < 200)))
        actor->movedir = DI_NODIR;           // avoid the area (most of the time anyway)

    return true;
}

//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
static boolean P_TryWalk(mobj_t *actor)
{
    if (!P_SmartMove(actor))
        return false;

    actor->movecount = P_Random() & 15;
    return true;
}

//
// P_DoNewChaseDir
//
// killough 9/8/98:
//
// Most of P_NewChaseDir(), except for what
// determines the new direction to take
//
static void P_DoNewChaseDir(mobj_t *actor, fixed_t deltax, fixed_t deltay)
{
    dirtype_t   xdir, ydir, tdir;
    dirtype_t   olddir = actor->movedir;
    dirtype_t   turnaround = olddir;

    // find reverse direction
    if (turnaround != DI_NODIR)
        turnaround ^= 4;

    xdir = (deltax >  10 * FRACUNIT ? DI_EAST : (deltax < -10 * FRACUNIT ? DI_WEST : DI_NODIR));
    ydir = (deltay < -10 * FRACUNIT ? DI_SOUTH : (deltay >  10 * FRACUNIT ? DI_NORTH : DI_NODIR));

    // try direct route
    if (xdir != DI_NODIR && ydir != DI_NODIR && turnaround !=
        (actor->movedir = deltay < 0 ? deltax > 0 ? DI_SOUTHEAST : DI_SOUTHWEST :
        deltax > 0 ? DI_NORTHEAST : DI_NORTHWEST) && P_TryWalk(actor))
        return;

    // try other directions
    if (P_Random() > 200 || ABS(deltay) > ABS(deltax))
    {
        tdir = xdir;
        xdir = ydir;
        ydir = tdir;
    }

    if ((xdir == turnaround ? xdir = DI_NODIR : xdir) != DI_NODIR
        && (actor->movedir = xdir, P_TryWalk(actor)))
        return;         // either moved forward or attacked

    if ((ydir == turnaround ? ydir = DI_NODIR : ydir) != DI_NODIR
        && (actor->movedir = ydir, P_TryWalk(actor)))
        return;

    // there is no direct path to the player, so pick another direction.
    if (olddir != DI_NODIR && (actor->movedir = olddir, P_TryWalk(actor)))
        return;

    // randomly determine direction of search
    if (P_Random() & 1)
    {
        for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
            if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
                return;
    }
    else
        for (tdir = DI_SOUTHEAST; tdir != DI_EAST - 1; tdir--)
            if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
                return;

    if ((actor->movedir = turnaround) != DI_NODIR && !P_TryWalk(actor))
        actor->movedir = DI_NODIR;
}

//
// killough 11/98:
//
// Monsters try to move away from tall dropoffs.
//
// In Doom, they were never allowed to hang over dropoffs,
// and would remain stuck if involuntarily forced over one.
// This logic, combined with p_map.c (P_TryMove), allows
// monsters to free themselves without making them tend to
// hang over dropoffs.
//
static fixed_t  dropoff_deltax;
static fixed_t  dropoff_deltay;
static fixed_t  floorz;

static boolean PIT_AvoidDropoff(line_t *line)
{
    if (line->backsector                                // Ignore one-sided linedefs
        && tmbbox[BOXRIGHT]  > line->bbox[BOXLEFT]
        && tmbbox[BOXLEFT]   < line->bbox[BOXRIGHT]
        && tmbbox[BOXTOP]    > line->bbox[BOXBOTTOM]    // Linedef must be contacted
        && tmbbox[BOXBOTTOM] < line->bbox[BOXTOP]
        && P_BoxOnLineSide(tmbbox, line) == -1)
    {
        fixed_t front = line->frontsector->floorheight;
        fixed_t back = line->backsector->floorheight;
        angle_t angle;

        // The monster must contact one of the two floors,
        // and the other must be a tall dropoff (more than 24).
        if (back == floorz && front < floorz - FRACUNIT * 24)
            angle = R_PointToAngle2(0, 0, line->dx, line->dy);          // front side dropoff
        else if (front == floorz && back < floorz - FRACUNIT * 24)
            angle = R_PointToAngle2(line->dx, line->dy, 0, 0);          // back side dropoff
        else
            return true;

        // Move away from dropoff at a standard speed.
        // Multiple contacted linedefs are cumulative (e.g. hanging over corner)
        dropoff_deltax -= finesine[angle >> ANGLETOFINESHIFT] * 32;
        dropoff_deltay += finecosine[angle >> ANGLETOFINESHIFT] * 32;
    }
    return true;
}

//
// Driver for above
//
static fixed_t P_AvoidDropoff(mobj_t *actor)
{
    int yh = ((tmbbox[BOXTOP] = actor->y + actor->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    int yl = ((tmbbox[BOXBOTTOM] = actor->y - actor->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    int xh = ((tmbbox[BOXRIGHT] = actor->x + actor->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    int xl = ((tmbbox[BOXLEFT] = actor->x - actor->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    int bx, by;

    floorz = actor->z;                                          // remember floor height

    dropoff_deltax = dropoff_deltay = 0;

    // check lines
    validcount++;
    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, PIT_AvoidDropoff);     // all contacted lines

    return (dropoff_deltax | dropoff_deltay);                   // Non-zero if movement prescribed
}

//
// P_NewChaseDir
//
// killough 9/8/98: Split into two functions
//
static void P_NewChaseDir(mobj_t *actor)
{
    mobj_t      *target = actor->target;
    fixed_t     deltax = target->x - actor->x;
    fixed_t     deltay = target->y - actor->y;

    if (actor->floorz - actor->dropoffz > FRACUNIT * 24
        && actor->z <= actor->floorz
        && !(actor->flags & (MF_DROPOFF | MF_FLOAT))
        && P_AvoidDropoff(actor))       // Move away from dropoff
    {
        P_DoNewChaseDir(actor, dropoff_deltax, dropoff_deltay);

        // If moving away from dropoff, set movecount to 1 so that
        // small steps are taken to get monster away from dropoff.
        actor->movecount = 1;
        return;
    }

    P_DoNewChaseDir(actor, deltax, deltay);
}

#define MONS_LOOK_RANGE (32 * 64 * FRACUNIT)

static boolean P_LookForMonsters(mobj_t *actor)
{
    mobj_t      *mo;
    thinker_t   *think;

    if (!P_CheckSight(players[0].mo, actor))
        return false;           // player can't see monster

    for (think = thinkerclasscap[th_mobj].cnext; think != &thinkerclasscap[th_mobj];
        think = think->cnext)
    {
        mo = (mobj_t *)think;
        if (!(mo->flags & MF_COUNTKILL) || mo == actor || mo->health <= 0)
            continue;           // not a valid monster

        if (P_ApproxDistance(actor->x - mo->x, actor->y - mo->y) > MONS_LOOK_RANGE)
            continue;           // out of range

        if (!P_CheckSight(actor, mo))
            continue;           // out of sight

        // Found a target monster
        P_SetTarget(&actor->lastenemy, actor->target);
        P_SetTarget(&actor->target, mo);
        return true;
    }
    return false;
}

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
static boolean P_LookForPlayers(mobj_t *actor, boolean allaround)
{
    player_t    *player;
    mobj_t      *mo;
    angle_t     an;
    fixed_t     dist;

    if (infight)
        // player is dead, look for monsters
        return P_LookForMonsters(actor);

    player = &players[0];
    mo = player->mo;

    if (player->cheats & CF_NOTARGET)
        return false;

    if (player->health <= 0 || !P_CheckSight(actor, mo))
    {
        // Use last known enemy if no players sighted -- killough 2/15/98
        if (actor->lastenemy && actor->lastenemy->health > 0)
        {
            P_SetTarget(&actor->target, actor->lastenemy);
            P_SetTarget(&actor->lastenemy, NULL);
            return true;
        }
        return false;
    }

    dist = P_ApproxDistance(mo->x - actor->x, mo->y - actor->y);

    if (!allaround)
    {
        an = R_PointToAngle2(actor->x, actor->y, mo->x, mo->y) - actor->angle;

        if (an > ANG90 && an < ANG270)
        {
            // if real close, react anyway
            if (dist > MELEERANGE)
            {
                // Use last known enemy if no players sighted -- killough 2/15/98
                if (actor->lastenemy && actor->lastenemy->health > 0)
                {
                    P_SetTarget(&actor->target, actor->lastenemy);
                    P_SetTarget(&actor->lastenemy, NULL);
                    return true;
                }
                return false;
            }
        }
    }

    if (mo->flags & MF_FUZZ)
    {
        // player is invisible
        if (dist > 2 * MELEERANGE && P_ApproxDistance(mo->momx, mo->momy) < 5 * FRACUNIT)
            return false;       // player is sneaking - can't detect
        if (P_Random() < 225)
            return false;       // player isn't sneaking, but still didn't detect
    }

    P_SetTarget(&actor->target, mo);
    actor->threshold = 60;
    return true;
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t *mo)
{
    thinker_t   *th;
    line_t      junk;

    A_Fall(mo);

    // scan the remaining thinkers to see if all Keens are dead
    for (th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
    {
        mobj_t      *mo2 = (mobj_t *)th;

        if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
            return;         // other Keen not dead
    }

    junk.tag = 666;
    EV_DoDoor(&junk, doorOpen);
}

//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look(mobj_t *actor)
{
    mobj_t      *targ;

    if (!actor->subsector)
        return;

    actor->threshold = 0;       // any shot will wake up
    targ = actor->subsector->sector->soundtarget;
    
    if (targ && (targ->flags & MF_SHOOTABLE))
    {
        P_SetTarget(&actor->target, targ);

        if (actor->flags & MF_AMBUSH)
        {
            if (P_CheckSight(actor, actor->target))
                goto seeyou;
        }
        else
            goto seeyou;
    }

    if (!P_LookForPlayers(actor, false))
        return;

    // go into chase state
seeyou:
    if (actor->info->seesound)
    {
        int     sound;

        switch (actor->info->seesound)
        {
            case sfx_posit1:
            case sfx_posit2:
            case sfx_posit3:
                sound = sfx_posit1 + P_Random() % 3;
                break;

            case sfx_bgsit1:
            case sfx_bgsit2:
                sound = sfx_bgsit1 + P_Random() % 2;
                break;

            default:
                sound = actor->info->seesound;
                break;
        }

        if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
            S_StartSound(NULL, sound);          // full volume
        else
            S_StartSound(actor, sound);
    }

    P_SetMobjState(actor, actor->info->seestate);
}

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase(mobj_t *actor)
{
    if (actor->reactiontime)
        actor->reactiontime--;

    // modify target threshold
    if (actor->threshold)
    {
        if (!actor->target || actor->target->health <= 0)
            actor->threshold = 0;
        else
            actor->threshold--;
    }

    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        int delta = (actor->angle &= (7 << 29)) - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90 / 2;
        else if (delta < 0)
            actor->angle += ANG90 / 2;
    }

    if (actor->shadow)
        actor->shadow->angle = actor->angle;

    if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
    {
        // look for a new target
        if (!P_LookForPlayers(actor, true))
            P_SetMobjState(actor, actor->info->spawnstate);
        return;
    }

    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        if (gameskill != sk_nightmare && !fastparm)
            P_NewChaseDir(actor);
        return;
    }

    // check for melee attack
    if (actor->info->meleestate && P_CheckMeleeRange(actor))
    {
        if (actor->info->attacksound)
            S_StartSound(actor, actor->info->attacksound);

        P_SetMobjState(actor, actor->info->meleestate);

        // killough 8/98: remember an attack
        if (!actor->info->missilestate)
            actor->flags |= MF_JUSTHIT;
        return;
    }

    // check for missile attack
    if (actor->info->missilestate)
    {
        if (gameskill < sk_nightmare && !fastparm && actor->movecount)
            goto nomissile;

        if (!P_CheckMissileRange(actor))
            goto nomissile;

        P_SetMobjState(actor, actor->info->missilestate);
        actor->flags |= MF_JUSTATTACKED;
        return;
    }

nomissile:
    // chase towards player
    if (--actor->movecount < 0 || !P_SmartMove(actor))
        P_NewChaseDir(actor);

    // make active sound
    if (actor->info->activesound && P_Random() < 3)
        S_StartSound(actor, actor->info->activesound);
}

//
// A_FaceTarget
//
void A_FaceTarget(mobj_t *actor)
{
    if (!actor->target)
        return;

    actor->flags &= ~MF_AMBUSH;
    actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

    if (actor->target->flags & MF_FUZZ)
        actor->angle += (P_Random() - P_Random()) << 21;

    if (actor->shadow)
        actor->shadow->angle = actor->angle;
}

//
// A_PosAttack
//
void A_PosAttack(mobj_t *actor)
{
    int angle;
    int damage;
    int slope;

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    angle = actor->angle;
    slope = P_AimLineAttack(actor, angle, MISSILERANGE);

    S_StartSound(actor, sfx_pistol);
    angle += (P_Random() - P_Random()) << 20;
    damage = ((P_Random() % 5) + 1) * 3;
    P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack(mobj_t *actor)
{
    int i;
    int bangle;
    int slope;

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    bangle = actor->angle;
    slope = P_AimLineAttack(actor, bangle, MISSILERANGE);

    S_StartSound(actor, sfx_shotgn);
    for (i = 0; i < 3; i++)
    {
        int     angle = bangle + ((P_Random() - P_Random()) << 20);
        int     damage = ((P_Random() % 5) + 1) * 3;

        P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
    }
}

void A_CPosAttack(mobj_t *actor)
{
    int angle;
    int bangle;
    int damage;
    int slope;

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    bangle = actor->angle;
    slope = P_AimLineAttack(actor, bangle, MISSILERANGE);

    S_StartSound(actor, sfx_shotgn);
    angle = bangle + ((P_Random() - P_Random()) << 20);
    damage = ((P_Random() % 5) + 1) * 3;
    P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire(mobj_t *actor)
{
    // keep firing unless target got out of sight
    A_FaceTarget(actor);

    if (P_Random() < 40)
        return;

    if (!actor->target || actor->target->health <= 0 || !P_CheckSight(actor, actor->target))
        P_SetMobjState(actor, actor->info->seestate);
}

void A_SpidRefire(mobj_t *actor)
{
    // keep firing unless target got out of sight
    A_FaceTarget(actor);

    if (P_Random() < 10)
        return;

    if (!actor->target || actor->target->health <= 0 || !P_CheckSight(actor, actor->target))
        P_SetMobjState(actor, actor->info->seestate);
}

void A_BspiAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);

    // launch a missile
    P_SpawnMissile(actor, actor->target, MT_ARACHPLAZ);
}

//
// A_TroopAttack
//
void A_TroopAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    if (P_CheckMeleeRange(actor))
    {
        int     damage = (P_Random() % 8 + 1) * 3;

        S_StartSound(actor, sfx_claw);
        P_DamageMobj(actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    actor->frame |= FF_FULLBRIGHT;
    P_SpawnMissile(actor, actor->target, MT_TROOPSHOT);
}

void A_SargAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    if (P_CheckMeleeRange(actor))
    {
        int     damage = (P_Random() % 10 + 1) * 4;

        P_DamageMobj(actor->target, actor, actor, damage);
    }
}

void A_HeadAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    if (P_CheckMeleeRange(actor))
    {
        int     damage = (P_Random() % 6 + 1) * 10;

        P_DamageMobj(actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    actor->frame |= FF_FULLBRIGHT;
    P_SpawnMissile(actor, actor->target, MT_HEADSHOT);
}

void A_CyberAttack(mobj_t *actor)
{
    mobj_t      *mo;

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    mo = P_SpawnMissile(actor, actor->target, MT_ROCKET);

    if (smoketrails)
        mo->flags2 |= MF2_SMOKETRAIL;
}

void A_BruisAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    if (P_CheckMeleeRange(actor))
    {
        int     damage = (P_Random() % 8 + 1) * 10;

        S_StartSound(actor, sfx_claw);
        P_DamageMobj(actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    actor->frame |= FF_FULLBRIGHT;
    P_SpawnMissile(actor, actor->target, MT_BRUISERSHOT);
}

//
// A_SkelMissile
//
void A_SkelMissile(mobj_t *actor)
{
    mobj_t      *mo;

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    actor->z += 14 * FRACUNIT;          // so missile spawns higher
    mo = P_SpawnMissile(actor, actor->target, MT_TRACER);
    actor->z -= 14 * FRACUNIT;          // back to normal

    mo->x += mo->momx;
    mo->y += mo->momy;
    P_SetTarget(&mo->tracer, actor->target);
}

#define TRACEANGLE      0xc000000;

void A_Tracer(mobj_t *actor)
{
    angle_t     exact;
    fixed_t     dist;
    fixed_t     slope;
    mobj_t      *dest;
    int         speed;

    if ((gametic - levelstarttic) & 3)
        return;

    // spawn a puff of smoke behind the homing rocket
    P_SpawnSmokeTrail(actor->x, actor->y, actor->z, actor->angle);

    // adjust direction
    dest = actor->tracer;

    if (!dest || dest->health <= 0)
        return;

    // change angle
    exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

    if (exact != actor->angle)
    {
        if (exact - actor->angle > 0x80000000)
        {
            actor->angle -= TRACEANGLE;
            if (exact - actor->angle < 0x80000000)
                actor->angle = exact;
        }
        else
        {
            actor->angle += TRACEANGLE;
            if (exact - actor->angle > 0x80000000)
                actor->angle = exact;
        }
    }

    exact = actor->angle >> ANGLETOFINESHIFT;
    speed = actor->info->speed;
    actor->momx = FixedMul(speed, finecosine[exact]);
    actor->momy = FixedMul(speed, finesine[exact]);

    // change slope
    dist = MAX(1, P_ApproxDistance(dest->x - actor->x, dest->y - actor->y) / speed);

    slope = (dest->z + 40 * FRACUNIT - actor->z) / dist;

    if (slope < actor->momz)
        actor->momz -= FRACUNIT / 8;
    else
        actor->momz += FRACUNIT / 8;
}

void A_SkelWhoosh(mobj_t *actor)
{
    if (!actor->target)
        return;
    A_FaceTarget(actor);
    S_StartSound(actor, sfx_skeswg);
}

void A_SkelFist(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (P_CheckMeleeRange(actor))
    {
        int damage = ((P_Random() % 10) + 1) * 6;

        S_StartSound(actor, sfx_skepch);
        P_DamageMobj(actor->target, actor, actor, damage);
    }
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
mobj_t  *corpsehit;
fixed_t viletryx;
fixed_t viletryy;

boolean PIT_VileCheck(mobj_t *thing)
{
    int         maxdist;
    boolean     check;
    fixed_t     height;
    fixed_t     radius;

    if (!(thing->flags & MF_CORPSE))
        return true;    // not a monster

    if (thing->tics != -1)
        return true;    // not lying still yet

    if (thing->info->raisestate == S_NULL)
        return true;    // monster doesn't have a raise state

    maxdist = thing->info->radius + mobjinfo[MT_VILE].radius;

    if (ABS(thing->x - viletryx) > maxdist || ABS(thing->y - viletryy) > maxdist)
        return true;    // not actually touching

    corpsehit = thing;
    corpsehit->momx = corpsehit->momy = 0;

    height = corpsehit->height;
    radius = corpsehit->radius;
    corpsehit->height = corpsehit->info->height;
    corpsehit->radius = corpsehit->info->radius;
    corpsehit->flags |= MF_SOLID;
    corpsehit->flags2 |= MF2_RESURRECTING;
    check = P_CheckPosition(corpsehit, corpsehit->x, corpsehit->y);
    corpsehit->height = height;
    corpsehit->radius = radius;
    corpsehit->flags &= ~MF_SOLID;
    corpsehit->flags2 &= ~MF2_RESURRECTING;

    return !check;        // got one, so stop checking
}

//
// A_VileChase
// Check for ressurecting a body
//
void A_VileChase(mobj_t *actor)
{
    int movedir = actor->movedir;

    if (movedir != DI_NODIR)
    {
        int     xl, xh;
        int     yl, yh;
        int     bx, by;
        int     speed = actor->info->speed;

        // check for corpses to raise
        viletryx = actor->x + speed * xspeed[movedir];
        viletryy = actor->y + speed * yspeed[movedir];

        xl = (viletryx - bmaporgx - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        xh = (viletryx - bmaporgx + MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        yl = (viletryy - bmaporgy - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        yh = (viletryy - bmaporgy + MAXRADIUS * 2) >> MAPBLOCKSHIFT;

        for (bx = xl; bx <= xh; bx++)
        {
            for (by = yl; by <= yh; by++)
            {
                // Call PIT_VileCheck to check
                // whether object is a corpse
                // that can be raised.
                if (!P_BlockThingsIterator(bx, by, PIT_VileCheck))
                {
                    // got one!
                    mobj_t      *temp = actor->target;
                    mobjinfo_t  *info;

                    actor->target = corpsehit;
                    A_FaceTarget(actor);
                    actor->target = temp;

                    P_SetMobjState(actor, S_VILE_HEAL1);
                    S_StartSound(corpsehit, sfx_slop);
                    info = corpsehit->info;

                    P_SetMobjState(corpsehit, info->raisestate);

                    corpsehit->height = info->height;
                    corpsehit->radius = info->radius;
                    corpsehit->flags = info->flags;
                    corpsehit->flags2 = info->flags2;
                    if (corpsehit->shadow)
                        corpsehit->shadow->flags2 &= ~MF2_MIRRORED;
                    corpsehit->health = info->spawnhealth;
                    P_SetTarget(&corpsehit->target, NULL);
                    P_SetTarget(&corpsehit->lastenemy, NULL);

                    players[0].killcount--;

                    // killough 8/29/98: add to appropriate thread
                    P_UpdateThinker(&corpsehit->thinker);

                    return;
                }
            }
        }
    }

    // Return to normal attack.
    A_Chase(actor);
}

//
// A_VileStart
//
void A_VileStart(mobj_t *actor)
{
    S_StartSound(actor, sfx_vilatk);
}

//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire(mobj_t *actor);

void A_StartFire(mobj_t *actor)
{
    S_StartSound(actor, sfx_flamst);
    A_Fire(actor);
}

void A_FireCrackle(mobj_t *actor)
{
    S_StartSound(actor, sfx_flame);
    A_Fire(actor);
}

void A_Fire(mobj_t *actor)
{
    mobj_t              *dest = actor->tracer;
    mobj_t              *target;
    unsigned int        an;

    if (!dest)
        return;

    if (!(target = actor->target))
        return;

    // don't move it if the vile lost sight
    if (!P_CheckSight(target, dest))
        return;

    an = dest->angle >> ANGLETOFINESHIFT;

    P_UnsetThingPosition(actor);
    actor->x = dest->x + FixedMul(24 * FRACUNIT, finecosine[an]);
    actor->y = dest->y + FixedMul(24 * FRACUNIT, finesine[an]);
    actor->z = dest->z;
    P_SetThingPosition(actor);
    actor->floorz = dest->floorz;
    actor->ceilingz = dest->ceilingz;
}

//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget(mobj_t *actor)
{
    mobj_t      *fog;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    fog = P_SpawnMobj(actor->target->x, actor->target->y, actor->target->z, MT_FIRE);

    P_SetTarget(&actor->tracer, fog);
    P_SetTarget(&fog->target, actor);
    P_SetTarget(&fog->tracer, actor->target);
    A_Fire(fog);
}

//
// A_VileAttack
//
void A_VileAttack(mobj_t *actor)
{
    mobj_t      *fire;
    mobj_t      *target = actor->target;
    int         an;

    if (!target)
        return;

    A_FaceTarget(actor);

    if (!P_CheckSight(actor, target))
        return;

    S_StartSound(actor, sfx_barexp);
    P_DamageMobj(target, actor, actor, 20);
    if (!target->player || !(target->flags & MF_NOCLIP))
        target->momz = 1000 * FRACUNIT / target->info->mass;

    an = actor->angle >> ANGLETOFINESHIFT;

    fire = actor->tracer;

    if (!fire)
        return;

    // move the fire between the vile and the player
    fire->x = target->x - FixedMul(24 * FRACUNIT, finecosine[an]);
    fire->y = target->y - FixedMul(24 * FRACUNIT, finesine[an]);
    P_RadiusAttack(fire, actor, 70);
}

//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//
#define FATSPREAD       (ANG90 / 8)

void A_FatRaise(mobj_t *actor)
{
    A_FaceTarget(actor);
    S_StartSound(actor, sfx_manatk);
}

void A_FatAttack1(mobj_t *actor)
{
    mobj_t      *mo;
    mobj_t      *target = actor->target;
    int         an;

    if (!target)
        return;

    A_FaceTarget(actor);

    // Change direction to...
    actor->angle += FATSPREAD;
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack2(mobj_t *actor)
{
    mobj_t      *mo;
    mobj_t      *target = actor->target;
    int         an;

    if (!target)
        return;

    A_FaceTarget(actor);

    // Now here choose opposite deviation.
    actor->angle -= FATSPREAD;
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD * 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack3(mobj_t *actor)
{
    mobj_t      *mo;
    mobj_t      *target = actor->target;
    int         an;

    if (!target)
        return;

    A_FaceTarget(actor);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD / 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD / 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED      (20 * FRACUNIT)

void A_SkullAttack(mobj_t *actor)
{
    mobj_t      *dest;
    angle_t     an;
    int         dist;

    if (!actor->target)
        return;

    dest = actor->target;
    actor->flags |= MF_SKULLFLY;

    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
    actor->momy = FixedMul(SKULLSPEED, finesine[an]);
    dist = MAX(1, P_ApproxDistance(dest->x - actor->x, dest->y - actor->y) / SKULLSPEED);

    actor->momz = (dest->z + (dest->height >> 1) - actor->z) / dist;
}

void A_BetaSkullAttack(mobj_t *actor)
{
    int damage;

    if (!actor->target || actor->target->type == MT_SKULL)
        return;
    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    damage = (P_Random() % 8 + 1) * actor->info->damage;
    P_DamageMobj(actor->target, actor, actor, damage);
}

void A_Stop(mobj_t *actor)
{
    actor->momx = actor->momy = actor->momz = 0;
}

//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void A_PainShootSkull(mobj_t *actor, angle_t angle)
{
    mobj_t      *newmobj;
    angle_t     an = angle >> ANGLETOFINESHIFT;
    int         prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[MT_SKULL].radius) / 2;
    fixed_t     x = actor->x + FixedMul(prestep, finecosine[an]);
    fixed_t     y = actor->y + FixedMul(prestep, finesine[an]);
    fixed_t     z = actor->z + 8 * FRACUNIT;

    if (P_CheckLineSide(actor, x, y))
        return;

    newmobj = P_SpawnMobj(x , y, z, MT_SKULL);

    newmobj->flags &= ~MF_COUNTKILL;

    if (!P_TryMove(newmobj, newmobj->x, newmobj->y, false))
    {
        if (newmobj->shadow)
            P_RemoveMobjShadow(newmobj);
        P_RemoveMobj(newmobj);
        return;
    }

    P_SetTarget(&newmobj->target, actor->target);
    A_SkullAttack(newmobj);
}

//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//
void A_PainAttack(mobj_t *actor)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    A_PainShootSkull(actor, actor->angle);
}

void A_PainDie(mobj_t *actor)
{
    angle_t     angle = actor->angle;

    A_Fall(actor);
    A_PainShootSkull(actor, angle + ANG90);
    A_PainShootSkull(actor, angle + ANG180);
    A_PainShootSkull(actor, angle + ANG270);
}

void A_Scream(mobj_t *actor)
{
    int sound;

    switch (actor->info->deathsound)
    {
        case 0:
            return;

        case sfx_podth1:
        case sfx_podth2:
        case sfx_podth3:
            sound = sfx_podth1 + P_Random() % 3;
            break;

        case sfx_bgdth1:
        case sfx_bgdth2:
            sound = sfx_bgdth1 + P_Random() % 2;
            break;

        default:
            sound = actor->info->deathsound;
            break;
    }

    // Check for bosses.
    if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
        S_StartSound(NULL, sound);      // full volume
    else
        S_StartSound(actor, sound);
}

void A_XScream(mobj_t *actor)
{
    S_StartSound(actor, sfx_slop);
}

void A_Pain(mobj_t *actor)
{
    if (actor->info->painsound)
        S_StartSound(actor, actor->info->painsound);
}

void A_Fall(mobj_t *actor)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;
}

//
// A_Explode
//
void A_Explode(mobj_t *thingy)
{
    P_RadiusAttack(thingy, thingy->target, 128);
}

boolean flag667 = false;

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
void A_BossDeath(mobj_t *mo)
{
    thinker_t   *th;
    line_t      junk;

    if (gamemode == commercial)
    {
        if (gamemap != 7)
            return;

        if (mo->type != MT_FATSO && mo->type != MT_BABY)
            return;
    }
    else
    {
        switch (gameepisode)
        {
            case 1:
                if (gamemap != 8)
                    return;

                if (mo->type != MT_BRUISER)
                    return;
                break;

            case 2:
                if (gamemap != 8)
                    return;

                if (mo->type != MT_CYBORG)
                    return;
                break;

            case 3:
                if (gamemap != 8)
                    return;

                if (mo->type != MT_SPIDER)
                    return;
            break;

            case 4:
                switch (gamemap)
                {
                    case 6:
                        if (mo->type != MT_CYBORG)
                            return;
                        break;

                    case 8:
                        if (mo->type != MT_SPIDER)
                            return;
                        break;

                    default:
                        return;
                        break;
                }
                break;

            default:
                if (gamemap != 8)
                    return;
                break;
        }
    }

    if (!players[0].health)
        return;         // no one left alive, so do not end game

    // scan the remaining thinkers to see
    // if all bosses are dead
    for (th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
    {
        mobj_t      *mo2 = (mobj_t *)th;

        if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
            return;     // other boss not dead
    }

    // victory!
    if (gamemode == commercial)
    {
        if (gamemap == 7)
        {
            if (mo->type == MT_FATSO)
            {
                junk.tag = 666;
                EV_DoFloor(&junk, lowerFloorToLowest);
                return;
            }

            if (mo->type == MT_BABY)
            {
                if (!flag667)
                {
                    junk.tag = 667;
                    EV_DoFloor(&junk, raiseToTexture);
                    flag667 = true;
                }
                return;
            }
        }
    }
    else
    {
        switch (gameepisode)
        {
            case 1:
                junk.tag = 666;
                EV_DoFloor(&junk, lowerFloorToLowest);
                return;
                break;

            case 4:
                switch (gamemap)
                {
                    case 6:
                        junk.tag = 666;
                        EV_DoDoor(&junk, doorBlazeOpen);
                        return;
                        break;

                    case 8:
                        junk.tag = 666;
                        EV_DoFloor(&junk, lowerFloorToLowest);
                        return;
                        break;
                }
        }
    }

    G_ExitLevel();
}

void A_Hoof(mobj_t *mo)
{
    S_StartSound(mo, sfx_hoof);
    A_Chase(mo);
}

void A_Metal(mobj_t *mo)
{
    S_StartSound(mo, sfx_metal);
    A_Chase(mo);
}

void A_BabyMetal(mobj_t *mo)
{
    S_StartSound(mo, sfx_bspwlk);
    A_Chase(mo);
}

void A_OpenShotgun2(player_t *player, pspdef_t *psp)
{
    S_StartSound(player->mo, sfx_dbopn);
}

void A_LoadShotgun2(player_t *player, pspdef_t *psp)
{
    S_StartSound(player->mo, sfx_dbload);
}

void A_ReFire(player_t *player, pspdef_t *psp);

void A_CloseShotgun2(player_t *player, pspdef_t *psp)
{
    S_StartSound(player->mo, sfx_dbcls);
    A_ReFire(player, psp);
}

// [jeff] remove limit on braintargets
//  and fix http://doomwiki.org/wiki/Spawn_cubes_miss_east_and_west_targets

unsigned int    braintargeted;

void A_BrainAwake(mobj_t *mo)
{
    S_StartSound(NULL, sfx_bossit);
}

void A_BrainPain(mobj_t *mo)
{
    S_StartSound(NULL, sfx_bospn);
}

void A_BrainScream(mobj_t *mo)
{
    int x;

    for (x = mo->x - 258 * FRACUNIT; x < mo->x + 258 * FRACUNIT; x += FRACUNIT * 8)
    {
        int     y = mo->y - 320 * FRACUNIT;
        int     z = 128 + P_Random() * 2 * FRACUNIT;
        mobj_t  *th = P_SpawnMobj(x, y, z, MT_ROCKET);

        th->momz = P_Random() * 512;
        P_SetMobjState(th, S_BRAINEXPLODE1);
        th->tics = MAX(1, th->tics - (P_Random() & 7));
    }

    S_StartSound(NULL, sfx_bosdth);
}

void A_BrainExplode(mobj_t *mo)
{
    int         x = mo->x + (P_Random() - P_Random()) * 2048;
    int         y = mo->y;
    int         z = 128 + P_Random() * 2 * FRACUNIT;
    mobj_t      *th = P_SpawnMobj(x, y, z, MT_ROCKET);

    th->momz = P_Random() * 512;
    P_SetMobjState(th, S_BRAINEXPLODE1);
    th->tics = MAX(1, th->tics - (P_Random() & 7));
}

void A_BrainDie(mobj_t *mo)
{
    G_ExitLevel();
}

static mobj_t *A_NextBrainTarget(void)
{
    unsigned int        count = 0;
    thinker_t           *thinker;
    mobj_t              *found = NULL;

    // find all the target spots
    for (thinker = thinkerclasscap[th_mobj].cnext; thinker != &thinkerclasscap[th_mobj];
        thinker = thinker->cnext)
    {
        mobj_t      *mo = (mobj_t *)thinker;

        if (mo->type == MT_BOSSTARGET)
        {
            if (count == braintargeted) // This one the one that we want?
            {
                braintargeted++;        // Yes.
                return mo;
            }
            count++;
            if (!found)                 // Remember first one in case we wrap.
                found = mo;
        }
    }

    braintargeted = 1;                  // Start again.
    return found;
}

void A_BrainSpit(mobj_t *mo)
{
    mobj_t      *targ;

    static int  easy = 0;

    easy ^= 1;
    if (gameskill <= sk_easy && !easy)
        return;

    if (nomonsters)
        return;

    // shoot a cube at current target
    targ = A_NextBrainTarget();

    if (targ)
    {
        mobj_t  *newmobj;
        int     dist;

        // spawn brain missile
        newmobj = P_SpawnMissile(mo, targ, MT_SPAWNSHOT);
        P_SetTarget(&newmobj->target, targ);
        dist = P_ApproxDistance(targ->x - (mo->x + mo->momx), targ->y - (mo->y + mo->momy));

        // Use the reactiontime to hold the distance (squared)
        // from the target after the next move.
        newmobj->reactiontime = dist;
        S_StartSound(NULL, sfx_bospit);
    }
}

void A_SpawnFly(mobj_t* mo)
{
    mobj_t      *targ;

    targ = mo->target;
    if (targ)
    {
        int     dist;

        // Will the next move put the cube closer to
        // the target point than it is now?
        dist = P_ApproxDistance(targ->x - (mo->x + mo->momx), targ->y - (mo->y + mo->momy));
        if ((unsigned int)dist < (unsigned int)mo->reactiontime)
        {
            mo->reactiontime = dist;    // Yes. Still flying
            return;
        }

        if (!nomonsters)
        {
            mobj_t      *fog;
            mobj_t      *newmobj;
            int         r;
            mobjtype_t  type;

            // First spawn teleport fog.
            fog = P_SpawnMobj(targ->x, targ->y, targ->z, MT_SPAWNFIRE);
            S_StartSound(fog, sfx_telept);

            // Randomly select monster to spawn.
            r = P_Random();

            // Probability distribution (kind of :),
            // decreasing likelihood.
            if (r < 50)
                type = MT_TROOP;
            else if (r < 90)
                type = MT_SERGEANT;
            else if (r < 120)
                type = MT_SHADOWS;
            else if (r < 130)
                type = MT_PAIN;
            else if (r < 160)
                type = MT_HEAD;
            else if (r < 162)
                type = MT_VILE;
            else if (r < 172)
                type = MT_UNDEAD;
            else if (r < 192)
                type = MT_BABY;
            else if (r < 222)
                type = MT_FATSO;
            else if (r < 246)
                type = MT_KNIGHT;
            else
                type = MT_BRUISER;

            newmobj = P_SpawnMobj(targ->x, targ->y, targ->z, type);

            newmobj->flags &= ~MF_COUNTKILL;

            if (!(P_LookForPlayers(newmobj, true))
                || P_SetMobjState(newmobj, newmobj->info->seestate))
                // telefrag anything in this spot
                P_TeleportMove(newmobj, newmobj->x, newmobj->y, newmobj->z, true);

            totalkills++;
        }
    }

    // remove self (i.e., cube).
    if (mo->shadow)
        P_RemoveMobjShadow(mo);
    P_RemoveMobj(mo);
}

// travelling cube sound
void A_SpawnSound(mobj_t *mo)
{
    S_StartSound(mo, sfx_boscub);
    A_SpawnFly(mo);
}

void A_PlayerScream(mobj_t *mo)
{
    // Default death sound.
    int sound = sfx_pldeth;

    if (gamemode == commercial &&  mo->health < -50)
    {
        // IF THE PLAYER DIES
        // LESS THAN -50% WITHOUT GIBBING
        sound = sfx_pdiehi;
    }

    S_StartSound(mo, sound);
}

// killough 11/98: kill an object
void A_Die(mobj_t *actor)
{
    P_DamageMobj(actor, NULL, NULL, actor->health);
}

//
// A_Detonate
// killough 8/9/98: same as A_Explode, except that the damage is variable
//
void A_Detonate(mobj_t *mo)
{
    P_RadiusAttack(mo, mo->target, mo->info->damage);
}

//
// killough 9/98: a mushroom explosion effect, sorta :)
// Original idea: Linguica
//
void A_Mushroom(mobj_t *actor)
{
    int         i, j, n = actor->info->damage;

    // Mushroom parameters are part of code pointer's state
    fixed_t     misc1 = (actor->state->misc1 ? actor->state->misc1 : FRACUNIT * 4);
    fixed_t     misc2 = (actor->state->misc2 ? actor->state->misc2 : FRACUNIT / 2);

    A_Explode(actor);                                           // First make normal explosion

    // Now launch mushroom cloud
    for (i = -n; i <= n; i += 8)
        for (j = -n; j <= n; j += 8)
        {
            mobj_t target = *actor, *mo;

            target.x += i << FRACBITS;                          // Aim in many directions from source
            target.y += j << FRACBITS;
            target.z += P_ApproxDistance(i, j) * misc1;         // Aim up fairly high
            mo = P_SpawnMissile(actor, &target, MT_FATSHOT);    // Launch fireball
            mo->momx = FixedMul(mo->momx, misc2);
            mo->momy = FixedMul(mo->momy, misc2);               // Slow down a bit
            mo->momz = FixedMul(mo->momz, misc2);
            mo->flags &= ~MF_NOGRAVITY;                         // Make debris fall under gravity
        }
}

//
// killough 11/98
//
// The following were inspired by Len Pitre
//
// A small set of highly-sought-after code pointers
//
void A_Spawn(mobj_t *mo)
{
    if (mo->state->misc1)
        P_SpawnMobj(mo->x, mo->y, (mo->state->misc2 << FRACBITS) + mo->z, mo->state->misc1 - 1);
}

void A_Turn(mobj_t *mo)
{
    mo->angle += (unsigned int)(((uint64_t)mo->state->misc1 << 32) / 360);
}

void A_Face(mobj_t *mo)
{
    mo->angle = (unsigned int)(((uint64_t)mo->state->misc1 << 32) / 360);
}

void A_Scratch(mobj_t *mo)
{
    mo->target && (A_FaceTarget(mo), P_CheckMeleeRange(mo)) ?
        mo->state->misc2 ? S_StartSound(mo, mo->state->misc2) : (void)0,
        P_DamageMobj(mo->target, mo, mo, mo->state->misc1) : (void)0;
}

void A_PlaySound(mobj_t *mo)
{
    S_StartSound(mo->state->misc2 ? NULL : mo, mo->state->misc1);
}

void A_RandomJump(mobj_t *mo)
{
    if (P_Random() < mo->state->misc2)
        P_SetMobjState(mo, mo->state->misc1);
}

//
// This allows linedef effects to be activated inside deh frames.
//
void A_LineEffect(mobj_t *mo)
{
    static line_t       junk;
    player_t            player;
    player_t            *oldplayer;

    junk = *lines;
    oldplayer = mo->player;
    mo->player = &player;
    player.health = 100;
    junk.special = (short)mo->state->misc1;
    if (!junk.special)
        return;
    junk.tag = (short)mo->state->misc2;
    if (!P_UseSpecialLine(mo, &junk, 0))
        P_CrossSpecialLine(&junk, 0, mo);
    mo->state->misc1 = junk.special;
    mo->player = oldplayer;
}
