/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_controller.h"
#include "i_timer.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"

#define DISTFRIEND  (128 * FRACUNIT)    // distance friends tend to move towards players
#define BARRELRANGE (512 * FRACUNIT)

uint64_t    shake = 0;
int         shakeduration = 0;

void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);

//
// ENEMY THINKING
// Enemies are always spawned
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
// killough 05/05/98: reformatted, cleaned up
//
static void P_RecursiveSound(sector_t *sec, const int soundblocks, mobj_t *soundtarget)
{
    // wake up all monsters in this sector
    if (sec->validcount == validcount && sec->soundtraversed <= soundblocks + 1)
        return;         // already flooded

    sec->validcount = validcount;
    sec->soundtraversed = soundblocks + 1;
    P_SetTarget(&sec->soundtarget, soundtarget);

    for (int i = 0; i < sec->linecount; i++)
    {
        line_t      *line = sec->lines[i];
        const int   flags = line->flags;

        if (!(flags & ML_TWOSIDED))
            continue;

        P_LineOpening(line);

        if (openrange <= 0)
            continue;   // closed door

        if (!(flags & ML_SOUNDBLOCK))
            P_RecursiveSound(sides[line->sidenum[(sides[line->sidenum[0]].sector == sec)]].sector, soundblocks, soundtarget);
        else if (!soundblocks)
            P_RecursiveSound(sides[line->sidenum[(sides[line->sidenum[0]].sector == sec)]].sector, 1, soundtarget);
    }
}

//
// P_NoiseAlert
// If a monster yells at the player,
// it will alert other monsters to the player.
//
void P_NoiseAlert(mobj_t *target, mobj_t *emitter)
{
    // [BH] don't alert if notarget CCMD is enabled
    if (target->player && (viewplayer->cheats & CF_NOTARGET))
        return;

    validcount++;
    P_RecursiveSound(emitter->subsector->sector, 0, target);
}

//
// P_CheckRange
//
static bool P_CheckRange(mobj_t *actor, const fixed_t range)
{
    mobj_t  *target = actor->target;

    // killough 07/18/98: friendly monsters don't attack other friends
    if (actor->flags & target->flags & MF_FRIEND)
        return false;

    if (P_ApproxDistance(target->x - actor->x, target->y - actor->y) >= range)
        return false;

    // [BH] check difference in height as well
    if (!infiniteheight && !compat_nopassover
        && (target->z > actor->z + actor->height || actor->z > target->z + target->height))
        return false;

    if (!P_CheckSight(actor, target))
        return false;

    return true;
}

//
// P_CheckMeleeRange
//
// MBF21: add meleerange property
//
bool P_CheckMeleeRange(mobj_t *actor)
{
    mobj_t  *target = actor->target;

    if (!target)
        return false;

    return P_CheckRange(actor, actor->info->meleerange + target->info->radius - 20 * FRACUNIT);
}

//
// P_HitFriend
//
// killough 12/98
// This function tries to prevent shooting at friends
//
static bool P_HitFriend(mobj_t *actor)
{
    mobj_t  *target;

    if (!(actor->flags & MF_FRIEND) || !(target = actor->target))
        return false;

    P_AimLineAttack(actor, R_PointToAngle2(actor->x, actor->y, target->x, target->y),
        P_ApproxDistance(actor->x - target->x, actor->y - target->y), 0);

    return (linetarget && linetarget != target && !((linetarget->flags ^ actor->flags) & MF_FRIEND));
}

//
// P_CheckMissileRange
//
static bool P_CheckMissileRange(mobj_t *actor)
{
    fixed_t dist;
    mobj_t  *target = actor->target;

    if (!P_CheckSight(actor, target))
        return false;

    if (actor->flags & MF_JUSTHIT)
    {
        // the target just hit the enemy, so fight back!
        actor->flags &= ~MF_JUSTHIT;

        // killough 07/18/98: no friendly fire at corpses
        // killough 11/98: prevent too much infighting among friends
        return (!(actor->flags & MF_FRIEND)
            || (target->health > 0
                && (!(target->flags & MF_FRIEND)
                    || (target->player ? M_Random() > 128 : (!(target->flags & MF_JUSTHIT) && M_Random() > 128)))));
    }

    // killough 07/18/98: friendly monsters don't attack other friendly
    // monsters or players (except when attacked, and then only once)
    if (actor->flags & target->flags & MF_FRIEND)
        return false;

    if (actor->reactiontime)
        return false;                   // do not attack yet

    dist = (P_ApproxDistance(actor->x - target->x, actor->y - target->y) >> FRACBITS) - 64;

    if (actor->info->meleestate == S_NULL)
        dist -= 128;                    // no melee attack, so fire more

    if ((actor->mbf21flags & MF_MBF21_SHORTMRANGE) && dist > 14 * 64)
        return false;                   // too far away

    if ((actor->mbf21flags & MF_MBF21_LONGMELEE) && dist < 196)
        return false;                   // close for fist attack

    if (actor->mbf21flags & MF_MBF21_RANGEHALF)
        dist >>= 1;

    if (dist > 200)
        dist = 200;

    if ((actor->mbf21flags & MF_MBF21_HIGHERMPROB) && dist > 160)
        dist = 160;

    if (M_Random() < dist)
        return false;

    if ((actor->flags & MF_FRIEND) && P_HitFriend(actor))
        return false;

    return true;
}

//
// P_IsUnderDamage
//
// killough 09/09/98:
//
// Returns nonzero if the object is under damage based on
// their current position. Returns 1 if the damage is moderate,
// -1 if it is serious. Used for AI.
//
static int P_IsUnderDamage(const mobj_t *actor)
{
    int direction = 0;

    for (const struct msecnode_s *seclist = actor->touching_sectorlist; seclist; seclist = seclist->m_tnext)
    {
        const ceiling_t *ceiling = seclist->m_sector->ceilingdata;  // Crushing ceiling

        if (ceiling && ceiling->thinker.function == &T_MoveCeiling)
            direction |= ceiling->direction;
    }

    return direction;
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
static const fixed_t    xspeed[] = { FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000 };
static const fixed_t    yspeed[] = { 0, 47000, FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000 };

static bool P_Move(mobj_t *actor, const int dropoff)    // killough 09/12/98
{
    bool    try_ok;
    fixed_t tryx, tryy;
    fixed_t deltax, deltay;
    int     movefactor;
    int     friction = ORIG_FRICTION;
    int     speed;

    if (actor->movedir == DI_NODIR)
        return false;

    // killough 10/98: make monsters get affected by ice and sludge too:
    movefactor = P_GetMoveFactor(actor, &friction);

    speed = actor->info->speed;

    if (friction < ORIG_FRICTION    // sludge
        && !(speed = ((ORIG_FRICTION_FACTOR - (ORIG_FRICTION_FACTOR - movefactor) / 2) * speed) / ORIG_FRICTION_FACTOR))
        speed = 1;                  // always give the monster a little bit of speed

    tryx = actor->x + (deltax = speed * xspeed[actor->movedir]);
    tryy = actor->y + (deltay = speed * yspeed[actor->movedir]);

    // killough 12/98: rearrange, fix potential for stickiness on ice
    if (friction <= ORIG_FRICTION)
        try_ok = P_TryMove(actor, tryx, tryy, dropoff);
    else
    {
        fixed_t x = actor->x;
        fixed_t y = actor->y;
        fixed_t z = actor->floorz;
        fixed_t ceilingz = actor->ceilingz;
        fixed_t dropoffz = actor->dropoffz;

        // killough 10/98:
        // Let normal momentum carry them, instead of steptoeing them across ice.
        if ((try_ok = P_TryMove(actor, tryx, tryy, dropoff)))
        {
            P_UnsetThingPosition(actor);

            actor->x = x;
            actor->y = y;
            actor->floorz = z;
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
        int good;

        if ((actor->flags & MF_FLOAT) && floatok)
        {
            actor->z += (actor->z < tmfloorz ? FLOATSPEED : -FLOATSPEED);   // must adjust height
            actor->flags |= MF_INFLOAT;
            return true;
        }

        if (!numspechit)
            return false;

        actor->movedir = DI_NODIR;

        // if the special is not a door that can be opened, return false
        //
        // killough 08/09/98: this is what caused monsters to get stuck in
        // doortracks, because it thought that the monster freed itself
        // by opening a door, even if it was moving towards the doortrack,
        // and not the door itself.
        //
        // killough 09/09/98: If a line blocking the monster is activated,
        // return true 90% of the time. If a line blocking the monster is
        // not activated, but some other line is, return false 90% of the
        // time. A bit of randomness is needed to ensure it's free from
        // lockups, but for most cases, it returns the correct result.
        //
        // Do NOT simply return false 1/4th of the time (causes monsters to
        // back out when they shouldn't, and creates secondary stickiness).
        for (good = 0; numspechit--; )
            if (P_UseSpecialLine(actor, spechit[numspechit], 0, false))
                good |= (spechit[numspechit] == blockline ? 1 : 2);

        // There are checks elsewhere for numspechit == 0, so we don't want to
        // leave numspechit == -1.
        numspechit = 0;

        return (good && ((M_Random() >= 230) ^ (good & 1)));
    }
    else
        actor->flags &= ~MF_INFLOAT;

    // killough 11/98: fall more slowly, under gravity, if felldown == true
    if (!(actor->flags & MF_FLOAT) && !felldown)
        actor->z = actor->floorz;

    return true;
}

//
// P_SmartMove
//
// killough 09/12/98: Same as P_Move(), except smarter
//
static bool P_SmartMove(mobj_t *actor)
{
    mobj_t  *target = actor->target;
    int     dropoff = 0;
    int     underdamage = P_IsUnderDamage(actor);

    // killough 09/12/98: stay on a lift if target is on one
    bool    onlift = (target && target->health > 0
                && target->subsector->sector->tag == actor->subsector->sector->tag
                && actor->subsector->sector->islift);

    // killough 10/98: allow dogs to drop off of taller ledges sometimes.
    // dropoff == 1 means always allow it, dropoff == 2 means only up to 128 high,
    // and only if the target is immediately on the other side of the line.
    if (actor->type == MT_DOGS
        && target && !((target->flags ^ actor->flags) & MF_FRIEND)
        && (target->player || P_ApproxDistance(actor->x - target->x, actor->y - target->y) < 144 * FRACUNIT)
        && M_Random() < 235)
        dropoff = (target->player ? 1 : 2);

    if (!P_Move(actor, dropoff))
        return false;

    // killough 09/09/98: avoid crushing ceilings or other damaging areas
    if ((onlift && M_Random() < 230          // stay on lift
        && !actor->subsector->sector->islift)
        || (!underdamage                     // get away from damage
            && (underdamage = P_IsUnderDamage(actor))
            && (underdamage < 0 || M_Random() < 200)))
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
// returns true and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
static bool P_TryWalk(mobj_t *actor)
{
    if (!P_SmartMove(actor))
        return false;

    actor->movecount = (M_Random() & 15);
    return true;
}

//
// P_DoNewChaseDir
//
// killough 09/08/98:
//
// Most of P_NewChaseDir(), except for what
// determines the new direction to take
//
static void P_DoNewChaseDir(mobj_t *actor, const fixed_t deltax, const fixed_t deltay)
{
    const dirtype_t opposite[] =
    {
        DI_WEST,
        DI_SOUTHWEST,
        DI_SOUTH,
        DI_SOUTHEAST,
        DI_EAST,
        DI_NORTHEAST,
        DI_NORTH,
        DI_NORTHWEST,
        DI_NODIR
    };

    const dirtype_t diags[] =
    {
        DI_NORTHWEST,
        DI_NORTHEAST,
        DI_SOUTHWEST,
        DI_SOUTHEAST
    };

    dirtype_t       xdir, ydir;
    const dirtype_t olddir = actor->movedir;
    const dirtype_t turnaround = opposite[olddir];
    bool            attempts[NUMDIRS - 1] = { false };

    xdir = (deltax > 10 * FRACUNIT ? DI_EAST : (deltax < -10 * FRACUNIT ? DI_WEST : DI_NODIR));
    ydir = (deltay < -10 * FRACUNIT ? DI_SOUTH : (deltay > 10 * FRACUNIT ? DI_NORTH : DI_NODIR));

    // try direct route
    if (xdir != DI_NODIR && ydir != DI_NODIR)
    {
        actor->movedir = diags[((deltay < 0) << 1) + (deltax > 0)];

        if (actor->movedir != turnaround)
        {
            attempts[actor->movedir] = true;

            if (P_TryWalk(actor))
                return;
        }
    }

    // try other directions
    if (M_Random() > 200 || ABS(deltay) > ABS(deltax))
        SWAP(xdir, ydir);

    if (xdir == turnaround)
        xdir = DI_NODIR;

    if (ydir == turnaround)
        ydir = DI_NODIR;

    if (xdir != DI_NODIR && !attempts[xdir])
    {
        actor->movedir = xdir;
        attempts[xdir] = true;

        if (P_TryWalk(actor))
            return;     // either moved forward or attacked
    }

    if (ydir != DI_NODIR && !attempts[ydir])
    {
        actor->movedir = ydir;
        attempts[ydir] = true;

        if (P_TryWalk(actor))
            return;
    }

    // there is no direct path to the player, so pick another direction.
    if (olddir != DI_NODIR && !attempts[olddir])
    {
        actor->movedir = olddir;
        attempts[olddir] = true;

        if (P_TryWalk(actor))
            return;
    }

    // randomly determine direction of search
    if (M_Random() & 1)
    {
        for (int tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
            if (tdir != turnaround && !attempts[tdir])
            {
                actor->movedir = tdir;
                attempts[tdir] = true;

                if (P_TryWalk(actor))
                    return;
            }
    }
    else
        for (int tdir = DI_SOUTHEAST; tdir >= DI_EAST; tdir--)
            if (tdir != turnaround && !attempts[tdir])
            {
                actor->movedir = tdir;
                attempts[tdir] = true;

                if (P_TryWalk(actor))
                    return;
            }

    if (turnaround != DI_NODIR && !attempts[turnaround])
    {
        actor->movedir = turnaround;

        if (P_TryWalk(actor))
            return;
    }

    actor->movedir = DI_NODIR;  // cannot move
}

//
// killough 11/98:
//
// Monsters try to move away from tall dropoffs.
//
// In DOOM, they were never allowed to hang over dropoffs,
// and would remain stuck if involuntarily forced over one.
// This logic, combined with p_map.c (P_TryMove), allows
// monsters to free themselves without making them tend to
// hang over dropoffs.
//
static fixed_t  dropoff_deltax;
static fixed_t  dropoff_deltay;
static fixed_t  floorz;

static bool PIT_AvoidDropoff(line_t *line)
{
    if (line->backsector                                                            // ignore one-sided linedefs
        && tmbbox[BOXRIGHT] > line->bbox[BOXLEFT]
        && tmbbox[BOXLEFT] < line->bbox[BOXRIGHT]
        && tmbbox[BOXTOP] > line->bbox[BOXBOTTOM]                                   // linedef must be contacted
        && tmbbox[BOXBOTTOM] < line->bbox[BOXTOP]
        && P_BoxOnLineSide(tmbbox, line) == -1)
    {
        const fixed_t   front = line->frontsector->floorheight;
        const fixed_t   back = line->backsector->floorheight;
        angle_t         angle;

        // The monster must contact one of the two floors,
        // and the other must be a tall dropoff (more than 24).
        if (back == floorz && front < floorz - 24 * FRACUNIT)
            angle = R_PointToAngle2(0, 0, line->dx, line->dy) >> ANGLETOFINESHIFT;  // front side dropoff
        else if (front == floorz && back < floorz - 24 * FRACUNIT)
            angle = R_PointToAngle2(line->dx, line->dy, 0, 0) >> ANGLETOFINESHIFT;  // back side dropoff
        else
            return true;

        // Move away from dropoff at a standard speed.
        // Multiple contacted linedefs are cumulative (e.g. hanging over corner)
        dropoff_deltax -= finesine[angle] * 32;
        dropoff_deltay += finecosine[angle] * 32;
    }

    return true;
}

//
// Driver for above
//
static bool P_AvoidDropoff(const mobj_t *actor)
{
    const int   xh = P_GetSafeBlockX((tmbbox[BOXRIGHT] = actor->x + actor->radius) - bmaporgx);
    const int   xl = P_GetSafeBlockX((tmbbox[BOXLEFT] = actor->x - actor->radius) - bmaporgx);
    const int   yh = P_GetSafeBlockY((tmbbox[BOXTOP] = actor->y + actor->radius) - bmaporgy);
    const int   yl = P_GetSafeBlockY((tmbbox[BOXBOTTOM] = actor->y - actor->radius) - bmaporgy);

    floorz = actor->z;                                          // remember floor height
    dropoff_deltax = 0;
    dropoff_deltay = 0;

    // check lines
    validcount++;

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, &PIT_AvoidDropoff);    // all contacted lines

    return (dropoff_deltax || dropoff_deltay);                  // false if movement prescribed
}

//
// P_NewChaseDir
//
// killough 09/08/98: Split into two functions
//
static void P_NewChaseDir(mobj_t *actor)
{
    mobj_t  *target;
    fixed_t deltax, deltay;

    // Move away from dropoff
    if (actor->floorz - actor->dropoffz > 24 * FRACUNIT
        && actor->z <= actor->floorz
        && !(actor->flags & (MF_DROPOFF | MF_FLOAT))
        && P_AvoidDropoff(actor))
    {
        P_DoNewChaseDir(actor, dropoff_deltax, dropoff_deltay);

        // If moving away from dropoff, set movecount to 1 so that
        // small steps are taken to get monster away from dropoff.
        actor->movecount = 1;
        return;
    }

    target = actor->target;
    deltax = target->x - actor->x;
    deltay = target->y - actor->y;

    // Move away from friends when too close, except in certain situations (e.g. a crowded lift)
    if ((actor->flags & target->flags & MF_FRIEND)
        && !actor->subsector->sector->islift
        && P_ApproxDistance(deltax, deltay) < DISTFRIEND
        && !P_IsUnderDamage(actor))
    {
        deltax = -deltax;
        deltay = -deltay;
    }

    P_DoNewChaseDir(actor, deltax, deltay);
}

static bool P_LookForMonsters(mobj_t *actor)
{
    // Remember last enemy
    if (actor->lastenemy && actor->lastenemy->health > 0
        && !(actor->lastenemy->flags & actor->flags & MF_FRIEND))   // not friends
    {
        P_SetTarget(&actor->target, actor->lastenemy);
        P_SetTarget(&actor->lastenemy, NULL);
        return true;
    }

    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        mobj_t      *mo = (mobj_t *)th;
        mobj_t      *target;
        thinker_t   *cap;

        if (!(mo->flags & MF_COUNTKILL) || mo == actor || mo->health <= 0)
            continue;           // not a valid monster

        if (!((mo->flags ^ actor->flags) & MF_FRIEND) && !infight)
            continue;           // don't attack other friends

        // If the monster is already engaged in a one-on-one attack
        // with a healthy friend, don't attack around 60% the time
        if ((target = mo->target) && target->target == mo && M_Random() > 100
            && ((target->flags ^ mo->flags) & MF_FRIEND)
            && target->health * 2 >= target->info->spawnhealth)
            continue;

        if (P_ApproxDistance(actor->x - mo->x, actor->y - mo->y) > 32 * 64 * FRACUNIT)
            continue;           // out of range

        if (!P_CheckSight(actor, mo))
            continue;           // out of sight

        // Found a target monster
        P_SetTarget(&actor->lastenemy, actor->target);
        P_SetTarget(&actor->target, mo);

        // Move the selected monster to the end of the
        // list, so that it gets searched last next time.
        cap = &thinkers[th_mobj];
        (mo->thinker.cprev->cnext = mo->thinker.cnext)->cprev = mo->thinker.cprev;
        (mo->thinker.cprev = cap->cprev)->cnext = &mo->thinker;
        (mo->thinker.cnext = cap)->cprev = &mo->thinker;

        return true;
    }

    return false;
}

//
// P_LookForPlayer
// If allaround is false, only look 180 degrees in front.
// Returns true if the player is targeted.
//
static bool P_LookForPlayer(mobj_t *actor, const bool allaround)
{
    mobj_t  *mo = viewplayer->mo;

    // the player is dead, if near the player then look for other monsters
    if (infight && P_CheckSight(actor, mo))
        return P_LookForMonsters(actor);

    if (viewplayer->cheats & CF_NOTARGET)
        return false;

    // killough 09/09/98: friendly monsters go about players differently
    if (actor->flags & MF_FRIEND)
    {
        // Go back to the player, no matter whether they're visible or not
        if (viewplayer->playerstate == PST_LIVE)
        {
            P_SetTarget(&actor->target, mo);

            // killough 12/98:
            // get out of refiring loop, to avoid hitting player accidentally
            if (actor->info->missilestate != S_NULL)
            {
                P_SetMobjState(actor, actor->info->seestate);
                actor->flags &= ~MF_JUSTHIT;
            }

            return true;
        }

        return false;
    }

    if (viewplayer->health <= 0 || !P_CheckSight(actor, mo))
    {
        // Use last known enemy if no players sighted -- killough 02/15/98
        if (actor->lastenemy && actor->lastenemy->health > 0)
        {
            P_SetTarget(&actor->target, actor->lastenemy);
            P_SetTarget(&actor->lastenemy, NULL);

            return true;
        }

        return false;
    }

    if (!allaround)
    {
        const angle_t   an = R_PointToAngle2(actor->x, actor->y, mo->x, mo->y) - actor->angle;

        if (an > ANG90 && an < ANG270)
            // if real close, react anyway
            if (P_ApproxDistance(mo->x - actor->x, mo->y - actor->y) > WAKEUPRANGE)
            {
                // Use last known enemy if no players sighted -- killough 02/15/98
                if (actor->lastenemy && actor->lastenemy->health > 0)
                {
                    P_SetTarget(&actor->target, actor->lastenemy);
                    P_SetTarget(&actor->lastenemy, NULL);

                    return true;
                }

                return false;
            }
    }

    if (mo->flags & MF_FUZZ)
    {
        // player is invisible
        if (P_ApproxDistance(mo->x - actor->x, mo->y - actor->y) > 2 * MELEERANGE
            && P_ApproxDistance(mo->momx, mo->momy) < 5 * FRACUNIT)
            return false;       // player is sneaking - can't detect

        if (M_Random() < 225)
            return false;       // player isn't sneaking, but still didn't detect
    }

    P_SetTarget(&actor->target, mo);

    // killough 09/09/98: give monsters a threshold towards getting players
    // (we don't want it to be too easy for a player with dogs :)
    actor->threshold = 60;

    return true;
}

static bool P_LookForTargets(mobj_t *actor, int allaround)
{
    if ((actor->flags & MF_FRIEND) && P_LookForMonsters(actor))
        return true;

    return P_LookForPlayer(actor, allaround);
}

static void P_ShakeOnExplode(const mobj_t *actor)
{
    if (actor->type == MT_BARREL && r_shake_barrels)
    {
        const mobj_t    *mo = viewplayer->mo;

        if (mo->z <= mo->floorz && P_ApproxDistance(actor->x - mo->x, actor->y - mo->y) < BARRELRANGE)
        {
            shakeduration = EXPLODINGBARREL;
            shake = I_GetTimeMS() + shakeduration;

            if (joy_rumble_barrels)
            {
                const int   strength = 20000 * joy_rumble_barrels / 100;

                I_ControllerRumble(strength, strength);
                barrelrumbletics = TICRATE;
            }
        }
    }
}

static void P_SpawnBloodOnMelee(mobj_t *target, const int damage)
{
    if (!r_blood_melee || r_blood == r_blood_none)
        return;

    if (target->player)
    {
        if (!viewplayer->powers[pw_invulnerability] && !(viewplayer->cheats & CF_GODMODE))
        {
            const unsigned int  an = viewangle >> ANGLETOFINESHIFT;

            P_SpawnBlood(viewx + 20 * finecosine[an], viewy + 20 * finesine[an], viewz, 0, damage, target);
        }
    }
    else if (!(target->flags & MF_NOBLOOD))
        P_SpawnBlood(target->x, target->y, target->z + M_RandomInt(4, 16) * FRACUNIT, 0, damage, target);
}

//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until the player is sighted.
//
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t      *target;
    const int   flags = actor->flags;
    const bool  friendly = (flags & MF_FRIEND);

    actor->threshold = 0;       // any shot will wake up

    // killough 07/18/98:
    // Friendly monsters go after other monsters first, but
    // also return to player, without attacking them, if they
    // cannot find any targets. A marine's best friend :)
    actor->pursuecount = 0;

    if (!(friendly
        && P_LookForTargets(actor, false))
        && !((target = actor->subsector->sector->soundtarget)
            && (target->flags & MF_SHOOTABLE)
            && (P_SetTarget(&actor->target, target), (!(flags & MF_AMBUSH) || P_CheckSight(actor, target))))
        && (friendly || !P_LookForTargets(actor, false)))
        return;

    // go into chase state
    if (actor->info->seesound)
    {
        switch (actor->info->seesound)
        {
            case sfx_posit1:
            case sfx_posit2:
            case sfx_posit3:
                S_StartSound(actor, sfx_posit1 + M_Random() % 3);
                break;

            case sfx_bgsit1:
            case sfx_bgsit2:
                S_StartSound(actor, sfx_bgsit1 + M_Random() % 2);
                break;

            default:
                S_StartSound(((actor->mbf21flags & (MF_MBF21_BOSS | MF_MBF21_FULLVOLSOUNDS)) ? NULL : actor),
                    actor->info->seesound);
                break;
        }

        // [crispy] make seesounds uninterruptible
        S_UnlinkSound(actor);
    }

    P_SetMobjState(actor, actor->info->seestate);
}

//
// A_FaceTarget
//
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const mobj_t    *target = actor->target;

    if (!target)
        return;

    actor->flags &= ~MF_AMBUSH;
    actor->angle = R_PointToAngle2(actor->x, actor->y, target->x, target->y);

    if (target->flags & MF_FUZZ)
        actor->angle += (M_SubRandom() << 21);
}

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t              *target = actor->target;
    const mobjinfo_t    *info = actor->info;

    if (actor->reactiontime)
        actor->reactiontime--;

    // modify target threshold
    if (actor->threshold)
    {
        if (!target || target->health <= 0)
            actor->threshold = 0;
        else
            actor->threshold--;
    }

    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        const int   delta = (actor->angle &= (7u << 29)) - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90 / 2;
        else if (delta < 0)
            actor->angle += ANG90 / 2;
    }

    if (!target || !(target->flags & MF_SHOOTABLE))
    {
        // look for a new target
        if (!P_LookForPlayer(actor, true))
            P_SetMobjState(actor, info->spawnstate);    // no new target

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
    if (info->meleestate != S_NULL && P_CheckMeleeRange(actor))
    {
        if (info->attacksound)
            S_StartSound(actor, info->attacksound);

        P_SetMobjState(actor, info->meleestate);

        // killough 08/98: remember an attack
        if (info->missilestate == S_NULL)
            actor->flags |= MF_JUSTHIT;

        return;
    }

    // check for missile attack
    if (info->missilestate != S_NULL)
        if (!(gameskill < sk_nightmare && !fastparm && actor->movecount))
            if (P_CheckMissileRange(actor))
            {
                P_SetMobjState(actor, info->missilestate);
                actor->flags |= MF_JUSTATTACKED;
                return;
            }

    if (!actor->threshold)
    {
        // killough 07/18/98, 09/09/98: new monster AI
        // Look for new targets if current one is bad or is out of view
        if (actor->pursuecount)
            actor->pursuecount--;
        else
        {
            // Our pursuit time has expired. We're going to think about
            // changing targets
            actor->pursuecount = BASETHRESHOLD;

            // Unless (we have a live target and it's not friendly and we can see it)
            //  try to find a new one; return if successful
            if (!(target->health > 0
                && (((((target->flags ^ actor->flags) & MF_FRIEND) || !(actor->flags & MF_FRIEND)) && P_CheckSight(actor, target))))
                && P_LookForTargets(actor, true))
                return;

            // (Current target was good, or no new target was found.)
            // If monster is a missile-less friend, give up pursuit and
            // return to player, if no attacks have occurred recently.
            if (actor->info->missilestate == S_NULL && (actor->flags & MF_FRIEND))
            {
                if (actor->flags & MF_JUSTHIT)          // if recent action,
                    actor->flags &= ~MF_JUSTHIT;        // keep fighting
                else if (P_LookForPlayer(actor, true))  // else return to player
                    return;
            }
        }
    }

    // chase towards player
    if (--actor->movecount < 0 || !P_SmartMove(actor))
        P_NewChaseDir(actor);

    // make active sound
    if (info->activesound && M_Random() < 3)
        S_StartSound(actor, info->activesound);
}

//
// A_PosAttack
//
void A_PosAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    S_StartSound(actor, sfx_pistol);
    P_LineAttack(actor, actor->angle + (M_SubRandom() << 20), MISSILERANGE,
        P_AimLineAttack(actor, actor->angle, MISSILERANGE, 0), (M_Random() % 5 + 1) * 3);
}

void A_SPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    S_StartSound(actor, sfx_shotgn);

    for (int i = 0; i < 3; i++)
        P_LineAttack(actor, actor->angle + (M_SubRandom() << 20), MISSILERANGE,
            P_AimLineAttack(actor, actor->angle, MISSILERANGE, 0), (M_Random() % 5 + 1) * 3);
}

void A_CPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    S_StartSound(actor, sfx_shotgn);
    P_LineAttack(actor, actor->angle + (M_SubRandom() << 20), MISSILERANGE,
        P_AimLineAttack(actor, actor->angle, MISSILERANGE, 0), (M_Random() % 5 + 1) * 3);
}

void A_CPosRefire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target;

    // keep firing unless target got out of sight
    A_FaceTarget(actor, NULL, NULL);

    // killough 12/98: Stop firing if a friend has gotten in the way
    if (P_HitFriend(actor))
    {
        P_SetMobjState(actor, actor->info->seestate);
        return;
    }

    if (M_Random() < 40)
        return;

    // killough 11/98: prevent refiring on friends continuously
    if (!(target = actor->target) || target->health <= 0 || (actor->flags & target->flags & MF_FRIEND) || !P_CheckSight(actor, target))
        P_SetMobjState(actor, actor->info->seestate);
}

void A_SpidRefire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target;

    // keep firing unless target got out of sight
    A_FaceTarget(actor, NULL, NULL);

    // killough 12/98: Stop firing if a friend has gotten in the way
    if (P_HitFriend(actor))
    {
        P_SetMobjState(actor, actor->info->seestate);
        return;
    }

    if (M_Random() < 10)
        return;

    // killough 11/98: prevent refiring on friends continuously
    if (!(target = actor->target) || target->health <= 0 || (actor->flags & target->flags & MF_FRIEND) || !P_CheckSight(actor, target))
        P_SetMobjState(actor, actor->info->seestate);
}

void A_BspiAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const mobj_t    *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    // launch a missile
    P_SpawnMissile(actor, target, MT_ARACHPLAZ);
}

//
// A_TroopAttack
//
void A_TroopAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        const int   damage = ((M_Random() & 7) + 1) * 3;

        S_StartSound(actor, sfx_claw);
        P_DamageMobj(target, actor, actor, damage, true, false);
        P_SpawnBloodOnMelee(target, damage);
        return;
    }

    // [BH] make imp fullbright when launching missile
    actor->frame |= FF_FULLBRIGHT;

    // launch a missile
    P_SpawnMissile(actor, target, MT_TROOPSHOT);
}

void A_SargAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        const int   damage = (M_Random() % 10 + 1) * 4;

        P_DamageMobj(target, actor, actor, damage, true, false);
        P_SpawnBloodOnMelee(target, damage);
    }
}

void A_HeadAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        const int   damage = (M_Random() % 6 + 1) * 10;

        P_DamageMobj(target, actor, actor, damage, true, false);
        P_SpawnBloodOnMelee(target, damage);

        return;
    }

    // [BH] make cacodemon fullbright when launching missile here instead of in its
    // S_HEAD_ATK3 state so its not fullbright during its melee attack above.
    actor->frame |= FF_FULLBRIGHT;

    // launch a missile
    P_SpawnMissile(actor, target, MT_HEADSHOT);
}

void A_CyberAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t          *mo;
    const mobj_t    *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    mo = P_SpawnMissile(actor, target, MT_ROCKET);

    // [BH] give cyberdemon rockets smoke trails
    if (r_rockettrails && !PUFFA0 && !norockettrails && !incompatiblepalette)
        mo->flags2 |= MF2_SMOKETRAIL;
}

void A_BruisAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    // [BH] fix <https://doomwiki.org/wiki/Baron_attacks_a_monster_behind_him>
    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        const int   damage = ((M_Random() & 7) + 1) * 10;

        S_StartSound(actor, sfx_claw);
        P_DamageMobj(target, actor, actor, damage, true, false);
        P_SpawnBloodOnMelee(target, damage);

        return;
    }

    // [BH] make baron nobles fullbright when launching missile
    actor->frame |= FF_FULLBRIGHT;

    // launch a missile
    P_SpawnMissile(actor, target, MT_BRUISERSHOT);
}

//
// A_SkelMissile
//
void A_SkelMissile(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo;
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    actor->z += 14 * FRACUNIT;          // so missile spawns higher
    mo = P_SpawnMissile(actor, target, MT_TRACER);
    actor->z -= 14 * FRACUNIT;          // back to normal

    mo->x += mo->momx;
    mo->y += mo->momy;
    P_SetTarget(&mo->tracer, target);
}

#define TRACEANGLE  0x0C000000

void A_Tracer(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t exact;
    fixed_t dist;
    fixed_t slope;
    mobj_t  *dest;
    int     speed;

    if (maptime & 3)
        return;

    // spawn a puff of smoke behind the homing rocket
    if (!doom4vanilla)
    {
        if (r_rockettrails && !PUFFA0 && !norockettrails && !incompatiblepalette)
            actor->flags2 |= MF2_SMOKETRAIL;
        else
            P_SpawnPuff(actor->x, actor->y, actor->z, actor->angle);
    }

    // adjust direction
    if (!(dest = actor->tracer) || dest->health <= 0)
        return;

    // change angle
    exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

    if (exact != actor->angle)
    {
        if (exact - actor->angle > ANG180)
        {
            actor->angle -= TRACEANGLE;

            if (exact - actor->angle < ANG180)
                actor->angle = exact;
        }
        else
        {
            actor->angle += TRACEANGLE;

            if (exact - actor->angle > ANG180)
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

void A_SkelWhoosh(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    S_StartSound(actor, sfx_skeswg);
}

void A_SkelFist(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        const int   damage = (M_Random() % 10 + 1) * 6;

        S_StartSound(actor, sfx_skepch);
        P_DamageMobj(target, actor, actor, damage, true, false);
        P_SpawnBloodOnMelee(target, damage);
    }
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
static mobj_t   *corpsehit;
static fixed_t  viletryx;
static fixed_t  viletryy;
static int      viletryradius;

static bool PIT_VileCheck(mobj_t *thing)
{
    int     maxdist;
    bool    check;

    if (!(thing->flags & MF_CORPSE))
        return true;    // not a monster

    if (thing->tics != -1)
        return true;    // not lying still yet

    if (thing->info->raisestate == S_NULL)
        return true;    // monster doesn't have a raise state

    maxdist = thing->info->radius + viletryradius;

    if (ABS(thing->x - viletryx) > maxdist || ABS(thing->y - viletryy) > maxdist)
        return true;    // not actually touching

    corpsehit = thing;
    corpsehit->momx = 0;
    corpsehit->momy = 0;

    if (compat_corpsegibs)
    {
        corpsehit->height <<= 2;
        corpsehit->flags2 |= MF2_RESURRECTING;
        check = P_CheckPosition(corpsehit, corpsehit->x, corpsehit->y);
        corpsehit->flags2 &= ~MF2_RESURRECTING;
        corpsehit->height >>= 2;
    }
    else
    {
        // [BH] fix <https://doomwiki.org/wiki/Ghost_monster>
        const fixed_t   height = corpsehit->height;
        const fixed_t   radius = corpsehit->radius;

        corpsehit->height = corpsehit->info->height;
        corpsehit->radius = corpsehit->info->radius;
        corpsehit->flags |= MF_SOLID;
        corpsehit->flags2 |= MF2_RESURRECTING;
        check = P_CheckPosition(corpsehit, corpsehit->x, corpsehit->y);
        corpsehit->flags2 &= ~MF2_RESURRECTING;
        corpsehit->flags &= ~MF_SOLID;
        corpsehit->radius = radius;
        corpsehit->height = height;

        for (const struct msecnode_s *seclist = corpsehit->touching_sectorlist; seclist; seclist = seclist->m_tnext)
            corpsehit->floorz = MAX(corpsehit->floorz, seclist->m_sector->floorheight);
    }

    return !check;      // got one, so stop checking
}

//
// MBF21: P_HealCorpse
// Check for resurrecting a body
//
static bool P_HealCorpse(mobj_t *actor, int radius, statenum_t healstate, sfxnum_t healsound)
{
    const dirtype_t movedir = actor->movedir;

    if (movedir != DI_NODIR)
    {
        int         xl;
        int         xh;
        int         yl;
        int         yh;
        const int   speed = actor->info->speed;

        // check for corpses to raise
        viletryx = actor->x + speed * xspeed[movedir];
        viletryy = actor->y + speed * yspeed[movedir];
        viletryradius = radius;

        xl = P_GetSafeBlockX(viletryx - bmaporgx - MAXRADIUS * 2);
        xh = P_GetSafeBlockX(viletryx - bmaporgx + MAXRADIUS * 2);
        yl = P_GetSafeBlockY(viletryy - bmaporgy - MAXRADIUS * 2);
        yh = P_GetSafeBlockY(viletryy - bmaporgy + MAXRADIUS * 2);

        for (int bx = xl; bx <= xh; bx++)
            for (int by = yl; by <= yh; by++)
                // Call PIT_VileCheck() to check whether object is a corpse that can be raised.
                if (!P_BlockThingsIterator(bx, by, &PIT_VileCheck, true))
                {
                    // got one!
                    mobj_t              *prevtarget = actor->target;
                    mobjinfo_t          *info = corpsehit->info;
                    const mobjtype_t    type = corpsehit->type;

                    actor->target = corpsehit;
                    A_FaceTarget(actor, NULL, NULL);
                    actor->target = prevtarget;

                    P_SetMobjState(actor, healstate);
                    S_StartSound(corpsehit, healsound);

                    P_SetMobjState(corpsehit, info->raisestate);

                    if (compat_corpsegibs)
                        corpsehit->height <<= 2;
                    else
                    {
                        // [BH] fix <https://doomwiki.org/wiki/Ghost_monster>
                        corpsehit->height = info->height;
                        corpsehit->radius = info->radius;
                    }

                    // killough 07/18/98: friendliness is transferred from AV to raised corpse
                    corpsehit->flags = ((info->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND));

                    corpsehit->flags2 &= ~MF2_MIRRORED;
                    corpsehit->flags2 |= MF2_CASTSHADOW;
                    corpsehit->health = info->spawnhealth;
                    corpsehit->shadowoffset = info->shadowoffset;
                    corpsehit->colfunc = info->colfunc;
                    corpsehit->altcolfunc = info->altcolfunc;
                    corpsehit->giblevel = 0;
                    corpsehit->gibtimer = 0;

                    P_SetTarget(&corpsehit->target, NULL);

                    // killough 09/09/98
                    P_SetTarget(&corpsehit->lastenemy, NULL);
                    corpsehit->flags &= ~MF_JUSTHIT;

                    viewplayer->killcount--;
                    stat_monsterskilled_total--;
                    viewplayer->resurrectioncount++;
                    stat_monstersresurrected = SafeAdd(stat_monstersresurrected, 1);

                    if (type < NUMMOBJTYPES)
                        stat_monsterskilled[type] = SafeAdd(stat_monsterskilled[type], -1);

                    // [BH] display an obituary message in the console
                    if (obituaries)
                    {
                        char    actorname[128];
                        char    corpsehitname[128];
                        char    *temp;

                        if (*actor->name)
                            M_StringCopy(actorname, actor->name, sizeof(actorname));
                        else
                            M_snprintf(actorname, sizeof(actorname), "%s %s%s",
                                ((actor->flags & MF_FRIEND) && actor->type < NUMMOBJTYPES && monstercount[actor->type] == 1 ? "the" :
                                    (isvowel(actor->info->name1[0]) && !(actor->flags & MF_FRIEND) ? "an" : "a")),
                                ((actor->flags & MF_FRIEND) ? "friendly " : ""),
                                (*actor->info->name1 ? actor->info->name1 : "monster"));

                        temp = sentencecase(actorname);

                        if (*corpsehit->name)
                            M_StringCopy(corpsehitname, corpsehit->name, sizeof(corpsehitname));
                        else
                            M_snprintf(corpsehitname, sizeof(corpsehitname), "%s dead%s%s",
                                ((corpsehit->flags & MF_FRIEND) && corpsehit->type < NUMMOBJTYPES && monstercount[corpsehit->type] == 1 ? "the" : "a"),
                                ((corpsehit->flags & MF_FRIEND) ? ", friendly " : " "),
                                (*corpsehit->info->name1 ? corpsehit->info->name1 : "monster"));

                        C_PlayerMessage("%s resurrected %s.", temp, corpsehitname);
                        free(temp);
                    }

                    // killough 08/29/98: add to appropriate thread
                    P_UpdateThinker(&corpsehit->thinker);
                    return true;
                }
    }

    return false;
}

//
// A_VileChase
// Check for resurrecting a body
//
void A_VileChase(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!P_HealCorpse(actor, mobjinfo[MT_VILE].radius, S_VILE_HEAL1, sfx_slop))
        A_Chase(actor, NULL, NULL);
}

//
// A_VileStart
//
void A_VileStart(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_vilatk);
}

//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t      *dest = actor->tracer;
    mobj_t      *target;
    angle_t     an;
    sector_t    *sector;

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
    sector = actor->subsector->sector;
    actor->floorz = sector->floorheight;
    actor->ceilingz = sector->ceilingheight;
}

void A_StartFire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_flamst);
    A_Fire(actor, NULL, NULL);
}

void A_FireCrackle(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_flame);
    A_Fire(actor, NULL, NULL);
}

//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *fog;
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    fog = P_SpawnMobj(target->x, target->y, target->z, MT_FIRE);

    P_SetTarget(&actor->tracer, fog);
    P_SetTarget(&fog->target, actor);
    P_SetTarget(&fog->tracer, target);

    if (!DSFLAMST)
    {
        S_StartSound(fog, sfx_flamst);
        S_UnlinkSound(fog);
    }

    A_Fire(fog, NULL, NULL);
}

//
// A_VileAttack
//
void A_VileAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *fire;
    mobj_t  *target = actor->target;
    angle_t an;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (!P_CheckSight(actor, target))
        return;

    S_StartSound(actor, sfx_barexp);
    P_DamageMobj(target, actor, actor, 20, true, false);

    // [BH] don't apply upward momentum from vile attack to player when no clipping mode on
    if (!target->player || !(target->flags & MF_NOCLIP))
        target->momz = 1000 * FRACUNIT / MAX(1, target->info->mass);

    if (!(fire = actor->tracer))
        return;

    // move the fire between the vile and the player
    an = actor->angle >> ANGLETOFINESHIFT;
    fire->x = target->x - FixedMul(24 * FRACUNIT, finecosine[an]);
    fire->y = target->y - FixedMul(24 * FRACUNIT, finesine[an]);
    P_RadiusAttack(fire, actor, 70, 70, true);
}

//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//
#define FATSPREAD   (ANG90 / 8)

void A_FatRaise(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    A_FaceTarget(actor, NULL, NULL);
    S_StartSound(actor, sfx_manatk);
}

void A_FatAttack1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t         an;
    mobj_t          *mo;
    const mobj_t    *target = actor->target;
    int             speed;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    // Change direction to...
    actor->angle += FATSPREAD;
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD;
    an = mo->angle >> ANGLETOFINESHIFT;
    speed = mo->info->speed;
    mo->momx = FixedMul(speed, finecosine[an]);
    mo->momy = FixedMul(speed, finesine[an]);
}

void A_FatAttack2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t         an;
    mobj_t          *mo;
    const mobj_t    *target = actor->target;
    int             speed;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    // Now here choose opposite deviation.
    actor->angle -= FATSPREAD;
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD * 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    speed = mo->info->speed;
    mo->momx = FixedMul(speed, finecosine[an]);
    mo->momy = FixedMul(speed, finesine[an]);
}

void A_FatAttack3(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t         an;
    mobj_t          *mo;
    const mobj_t    *target = actor->target;
    int             speed;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD / 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    speed = mo->info->speed;
    mo->momx = FixedMul(speed, finecosine[an]);
    mo->momy = FixedMul(speed, finesine[an]);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD / 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(speed, finecosine[an]);
    mo->momy = FixedMul(speed, finesine[an]);
}

//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED  (20 * FRACUNIT)

void A_SkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t         an;
    const mobj_t    *dest = actor->target;

    if (!dest)
        return;

    actor->flags |= MF_SKULLFLY;

    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor, NULL, NULL);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
    actor->momy = FixedMul(SKULLSPEED, finesine[an]);
    actor->momz = (dest->z + (dest->height >> 1) - actor->z)
        / MAX(1, P_ApproxDistance(dest->x - actor->x, dest->y - actor->y) / SKULLSPEED);
}

void A_BetaSkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target || target->type == actor->type)
        return;

    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor, NULL, NULL);
    P_DamageMobj(target, actor, actor, ((M_Random() & 7) + 1) * actor->info->damage, true, false);
}

void A_Stop(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    actor->momx = 0;
    actor->momy = 0;
    actor->momz = 0;
}

//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
static void A_PainShootSkull(mobj_t *actor, angle_t angle)
{
    mobj_t          *newmobj;
    const angle_t   an = angle >> ANGLETOFINESHIFT;
    const int       prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[MT_SKULL].radius) / 2;
    const fixed_t   x = actor->x + FixedMul(prestep, finecosine[an]);
    const fixed_t   y = actor->y + FixedMul(prestep, finesine[an]);

    if (compat_limitpain)
    {
        // count total number of skulls currently on the level
        int count = 20;

        for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
            if (((mobj_t *)th)->type == MT_SKULL && --count < 0)
                return;
    }

    // Check whether the lost soul is being fired through a 1-sided
    // wall or an impassible line, or a "monsters can't cross" line.
    // If it is, then we don't allow the spawn.
    if (P_CheckLineSide(actor, x, y))
        return;

    newmobj = P_SpawnMobj(x, y, actor->z + 8 * FRACUNIT, MT_SKULL);

    if (!P_TryMove(newmobj, newmobj->x, newmobj->y, 0)
        // Check to see if the new lost soul's z value is above the
        // ceiling of its new sector, or below the floor. If so, kill it.
        || newmobj->z > newmobj->subsector->sector->ceilingheight - newmobj->height
        || newmobj->z < newmobj->subsector->sector->floorheight)
    {
        // kill it immediately
        massacre = true;    // [BH] set this to avoid obituary
        newmobj->flags &= ~MF_COUNTKILL;
        P_DamageMobj(newmobj, actor, actor, 10000, true, false);
        massacre = false;

        return;
    }

    // killough 07/20/98: PEs shoot lost souls with the same friendliness
    newmobj->flags = ((newmobj->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND));

    // [BH] count lost soul in player stats
    totalkills++;
    monstercount[MT_SKULL]++;

    // killough 08/29/98: add to appropriate thread
    P_UpdateThinker(&newmobj->thinker);

    P_SetTarget(&newmobj->target, actor->target);
    A_SkullAttack(newmobj, NULL, NULL);
}

//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//
void A_PainAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    A_PainShootSkull(actor, actor->angle);
}

void A_PainDie(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const angle_t   angle = actor->angle;

    A_Fall(actor, NULL, NULL);
    A_PainShootSkull(actor, angle + ANG90);
    A_PainShootSkull(actor, angle + ANG180);
    A_PainShootSkull(actor, angle + ANG270);
}

void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int sound = actor->info->deathsound;

    if (!sound)
        return;

    if (sound >= sfx_podth1 && sound <= sfx_podth3)
        sound = sfx_podth1 + M_Random() % 3;
    else if (sound == sfx_bgdth1 || sound == sfx_bgdth2)
        sound = sfx_bgdth1 + M_Random() % 2;

    S_StartSound(((actor->mbf21flags & (MF_MBF21_BOSS | MF_MBF21_FULLVOLSOUNDS)) ? NULL : actor), sound);
}

void A_XScream(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_slop);
}

void A_SkullPop(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo;

    S_StartSound(actor, sfx_pldeth);

    actor->flags &= ~MF_SOLID;
    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 48 * FRACUNIT, MT_GIBDTH);
    mo->momx = M_SubRandom() << 9;
    mo->momy = M_SubRandom() << 9;
    mo->momz = 2 * FRACUNIT + (M_Random() << 6);

    // Attach player mobj to bloody skull
    player = actor->player;
    actor->player = NULL;
    mo->player = player;
    mo->health = actor->health;
    mo->angle = actor->angle;

    if (player)
    {
        player->mo = mo;
        player->damagecount = 32;
    }
}

void A_Pain(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const int   painsound = actor->info->painsound;

    if (painsound)
        S_StartSound(actor, painsound);
}

void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;
}

//
// A_Explode
//
void A_Explode(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_RadiusAttack(actor, actor->target, 128, 128, true);
    P_ShakeOnExplode(actor);
}

//
// A_BossDeath
// Possibly trigger special effects if on first boss level
//
void A_BossDeath(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    line_t  junk = { 0 };
    int     numbossactions = P_GetNumBossActions(gameepisode, gamemap);

    // numbossactions == 0 means to use the defaults.
    // numbossactions == -1 means to do nothing.
    // positive values mean to check the list of boss actions and run all that apply.
    if (numbossactions)
    {
        int i;

        if (numbossactions < 0)
            return;

        if (viewplayer->health <= 0)
            return;         // no one left alive, so do not end game

        for (i = 0; i < numbossactions; i++)
            if (P_GetBossAction(gameepisode, gamemap, i)->type == actor->type)
                break;

        if (i >= numbossactions)
            return;  // no matches found

        // scan the remaining thinkers to see
        // if all bosses are dead
        for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
        {
            const mobj_t    *mo = (mobj_t *)th;

            if (mo != actor && mo->type == actor->type && mo->health > 0)
                return;         // other boss not dead
        }

        for (i = 0; i < numbossactions; i++)
        {
            bossaction_t    *bossaction = P_GetBossAction(gameepisode, gamemap, i);

            if (bossaction->type == actor->type)
            {
                junk = *lines;
                junk.special = (short)bossaction->special;
                junk.tag = (short)bossaction->tag;

                // use special semantics for line activation to block problem types.
                if (!P_UseSpecialLine(actor, &junk, 0, true))
                    P_CrossSpecialLine(&junk, 0, actor, true);
            }
        }

        return;
    }

    if (!anybossdeath)
    {
        if (gamemode == commercial)
        {
            if (gamemap != 7)
                return;

            if (!(actor->mbf21flags & (MF_MBF21_MAP07BOSS1 | MF_MBF21_MAP07BOSS2)))
                return;
        }
        else
        {
            switch (gameepisode)
            {
                case 1:
                    if (gamemap != 8 || !(actor->mbf21flags & MF_MBF21_E1M8BOSS))
                        return;

                    break;

                case 2:
                    if (gamemap != 8 || !(actor->mbf21flags & MF_MBF21_E2M8BOSS))
                        return;

                    break;

                case 3:
                    if (gamemap != 8 || !(actor->mbf21flags & MF_MBF21_E3M8BOSS))
                        return;

                    break;

                case 4:
                    switch (gamemap)
                    {
                        case 6:
                            if (!(actor->mbf21flags & MF_MBF21_E4M6BOSS))
                                return;

                            break;

                        case 8:
                            if (!(actor->mbf21flags & MF_MBF21_E4M8BOSS))
                                return;

                            break;

                        default:
                            return;
                    }

                    break;

                case 5:
                    return;

                default:
                    if (gamemap != 8)
                        return;

                    break;
            }
        }
    }

    if (viewplayer->health <= 0)
        return;         // no one left alive, so do not end game

    actor->health = 0;  // P_KillMobj() sets this to -1

    // scan the remaining thinkers to see if all bosses are dead
    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        const mobj_t    *mo = (mobj_t *)th;

        if (mo != actor && mo->type == actor->type && mo->health > 0)
            return;     // other boss not dead
    }

    // victory!
    if (gamemode == commercial)
    {
        if (gamemap == 7)
        {
            if (actor->mbf21flags & MF_MBF21_MAP07BOSS1)
            {
                junk.tag = 666;
                EV_DoFloor(&junk, LowerFloorToLowest);
                return;
            }

            if (actor->mbf21flags & MF_MBF21_MAP07BOSS2)
            {
                junk.tag = 667;
                EV_DoFloor(&junk, RaiseToTexture);
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
                EV_DoFloor(&junk, LowerFloorToLowest);
                return;

            case 4:
                switch (gamemap)
                {
                    case 6:
                        junk.tag = 666;
                        EV_DoDoor(&junk, DoorBlazeOpen, VDOORSPEED * 4);
                        return;

                    case 8:
                        junk.tag = 666;
                        EV_DoFloor(&junk, LowerFloorToLowest);
                        return;
                }
        }
    }

    G_ExitLevel();
}

void A_Hoof(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_hoof);
    A_Chase(actor, NULL, NULL);
}

void A_Metal(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_metal);
    A_Chase(actor, NULL, NULL);
}

void A_BabyMetal(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_bspwlk);
    A_Chase(actor, NULL, NULL);
}

// [jeff] remove limit on braintargets
//  and fix <https://doomwiki.org/wiki/Spawn_cubes_miss_east_and_west_targets>
void A_BrainAwake(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(NULL, sfx_bossit);
}

void A_BrainPain(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(NULL, sfx_bospn);
}

void A_BrainScream(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    // [BH] Fix <https://doomwiki.org/wiki/Lopsided_final_boss_explosions>
    for (int x = actor->x - 258 * FRACUNIT; x < actor->x + 258 * FRACUNIT; x += 8 * FRACUNIT)
    {
        mobj_t  *th = P_SpawnMobj(x, actor->y - 320 * FRACUNIT, 128 + M_Random() * 2 * FRACUNIT, MT_ROCKET);

        th->momz = M_Random() * 512;
        P_SetMobjState(th, S_BRAINEXPLODE1);
        th->tics = MAX(1, th->tics - (M_Random() & 7));
        th->colfunc = tlcolfunc;
        th->flags2 |= MF2_EXPLODING;
    }

    S_StartSound(NULL, sfx_bosdth);
}

void A_BrainExplode(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *th = P_SpawnMobj(actor->x + M_SubRandom() * 2048, actor->y, 128 + M_Random() * 2 * FRACUNIT, MT_ROCKET);

    th->momz = M_Random() * 512;
    P_SetMobjState(th, S_BRAINEXPLODE1);
    th->tics = MAX(1, th->tics - (M_Random() & 7));
    th->colfunc = tlcolfunc;
    th->flags2 |= MF2_EXPLODING;
}

void A_BrainDie(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    G_ExitLevel();
}

static mobj_t *A_NextBrainTarget(void)
{
    unsigned int        count = 0;
    static unsigned int braintargeted;
    mobj_t              *found = NULL;

    // find all the target spots
    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        mobj_t  *mo = (mobj_t *)th;

        if (mo->type == MT_BOSSTARGET)
        {
            if (count++ == braintargeted)   // This one the one that we want?
            {
                braintargeted++;            // Yes.
                return mo;
            }

            if (!found)                     // Remember first one in case we wrap.
                found = mo;
        }
    }

    braintargeted = 1;                      // Start again.
    return found;
}

void A_BrainSpit(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t      *target;
    static bool easy;

    easy = !easy;

    if (gameskill <= sk_easy && !easy)
        return;

    if (nomonsters)
        return;

    // shoot a cube at current target
    if ((target = A_NextBrainTarget()))
    {
        // spawn brain missile
        mobj_t  *newmobj = P_SpawnMissile(actor, target, MT_SPAWNSHOT);

        P_SetTarget(&newmobj->target, target);

        // Use the reactiontime to hold the distance (squared) from the target after the next move.
        newmobj->reactiontime = P_ApproxDistance(target->x - (actor->x + actor->momx), target->y - (actor->y + actor->momy));

        // killough 07/18/98: brain friendliness is transferred
        newmobj->flags = ((newmobj->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND));

        // killough 08/29/98: add to appropriate thread
        P_UpdateThinker(&newmobj->thinker);

        S_StartSound(NULL, sfx_bospit);
    }
}

void A_SpawnFly(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int             dist;
    const mobj_t    *target = actor->target;

    if (!target)
        return;

    // Will the next move put the cube closer to the target point than it is now?
    dist = P_ApproxDistance(target->x - (actor->x + actor->momx), target->y - (actor->y + actor->momy));

    if (dist < actor->reactiontime)
    {
        actor->reactiontime = dist; // Yes. Still flying
        return;
    }

    if (!nomonsters)
    {
        mobj_t      *fog;
        mobj_t      *newmobj;
        int         r;
        mobjtype_t  type;

        // First spawn teleport fog.
        fog = P_SpawnMobj(target->x, target->y, target->z, MT_SPAWNFIRE);
        S_StartSound(fog, sfx_telept);

        // Randomly select monster to spawn.
        r = M_Random();

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

        newmobj = P_SpawnMobj(target->x, target->y, target->z, type);

        // killough 07/18/98: brain friendliness is transferred
        newmobj->flags = ((newmobj->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND));

        // killough 08/29/98: add to appropriate thread
        P_UpdateThinker(&newmobj->thinker);

        if (!P_LookForPlayer(newmobj, true) || P_SetMobjState(newmobj, newmobj->info->seestate))
            P_TeleportMove(newmobj, newmobj->x, newmobj->y, newmobj->z, true);  // telefrag anything in this spot

        if (newmobj->flags & MF_COUNTKILL)
        {
            totalkills++;

            if (type < NUMMOBJTYPES)
                monstercount[type]++;
        }
    }

    // remove self (i.e., cube).
    P_RemoveMobj(actor);
}

// traveling cube sound
void A_SpawnSound(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_boscub);

    if (actor->type == MT_SPAWNSHOT)
        A_SpawnFly(actor, NULL, NULL);
}

void A_PlayerScream(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, (gamemode == commercial && actor->health < -50 ? sfx_pdiehi : sfx_pldeth));
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    line_t  junk = { 0 };

    A_Fall(actor, NULL, NULL);

    // scan the remaining thinkers to see if all Keens are dead
    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        const mobj_t    *mo = (mobj_t *)th;

        if (mo != actor && mo->type == actor->type && mo->health > 0)
            return;         // other Keen not dead
    }

    junk.tag = 666;
    EV_DoDoor(&junk, DoorOpen, VDOORSPEED);
}

// killough 11/98: kill an object
void A_Die(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_DamageMobj(actor, NULL, NULL, actor->health, false, false);
}

//
// A_Detonate
// killough 08/09/98: same as A_Explode, except that the damage is variable
//
void A_Detonate(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_RadiusAttack(actor, actor->target, actor->info->damage, actor->info->damage, false);
    P_ShakeOnExplode(actor);
}

//
// killough 09/98: a mushroom explosion effect, sorta :)
//
void A_Mushroom(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const int       n = actor->info->damage;

    // Mushroom parameters are part of code pointer's state
    const fixed_t   misc1 = (actor->state->misc1 ? actor->state->misc1 : 4 * FRACUNIT);
    const fixed_t   misc2 = (actor->state->misc2 ? actor->state->misc2 : FRACUNIT / 2);

    A_Explode(actor, NULL, NULL);                               // First make normal explosion

    // Now launch mushroom cloud
    for (int i = -n; i <= n; i += 8)
        for (int j = -n; j <= n; j += 8)
        {
            mobj_t  target = *actor;
            mobj_t  *mo;

            target.x += (i << FRACBITS);                        // Aim in many directions from source
            target.y += (j << FRACBITS);
            target.z += P_ApproxDistance(i, j) * misc1;         // Aim up fairly high

            mo = P_SpawnMissile(actor, &target, MT_FATSHOT);    // Launch fireball
            mo->momx = FixedMul(mo->momx, misc2);
            mo->momy = FixedMul(mo->momy, misc2);               // Slow down a bit
            mo->momz = FixedMul(mo->momz, misc2);
            mo->flags &= ~MF_NOGRAVITY;                         // Make debris fall under gravity
        }

    P_ShakeOnExplode(actor);
}

//
// killough 11/98
//
// The following were inspired by Len Pitre
//
// A small set of highly-sought-after code pointers
//
void A_Spawn(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobjtype_t  type = (mobjtype_t)actor->state->misc1;

    if (type--)
        // If we're in massacre mode then don't spawn anything killable.
        if (!(actor->flags2 & MF2_MASSACRE) || !(mobjinfo[type].flags & MF_COUNTKILL))
        {
            mobj_t  *newmobj = P_SpawnMobj(actor->x, actor->y, (actor->state->misc2 << FRACBITS) + actor->z, type);

            newmobj->flags = ((newmobj->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND));

            if (newmobj->flags & MF_COUNTKILL)
            {
                totalkills++;

                if (type < NUMMOBJTYPES)
                    monstercount[type]++;
            }
        }
}

void A_Turn(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    actor->angle += (angle_t)(((uint64_t)actor->state->misc1 << 32) / 360);
}

void A_Face(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    actor->angle = (angle_t)(((uint64_t)actor->state->misc1 << 32) / 360);
}

void A_Scratch(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        const state_t   *state = actor->state;

        if (state->misc2)
            S_StartSound(actor, state->misc2);

        P_DamageMobj(target, actor, actor, state->misc1, true, false);
        P_SpawnBloodOnMelee(target, state->misc1);
    }
}

void A_PlaySound(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const state_t   *state = actor->state;

    S_StartSound((state->misc2 ? NULL : actor), state->misc1);
}

void A_RandomJump(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    // [BH] allow A_RandomJump() to work for weapon states as well
    if (psp)
    {
        const state_t   *state = psp->state;

        if (M_Random() < state->misc2)
            P_SetPlayerSprite(psp - &player->psprites[ps_weapon], state->misc1);
    }
    else
    {
        const state_t   *state = actor->state;

        if (M_Random() < state->misc2)
            P_SetMobjState(actor, state->misc1);
    }
}

//
// This allows linedef effects to be activated inside deh frames.
//
void A_LineEffect(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!(actor->flags2 & MF2_LINEDONE))
    {
        line_t  junk = *lines;

        if ((junk.special = (short)actor->state->misc1))
        {
            static player_t newplayer = { 0 };
            player_t        *oldplayer = actor->player;

            actor->player = &newplayer;
            newplayer.health = initial_health;

            junk.tag = (short)actor->state->misc2;

            if (!P_UseSpecialLine(actor, &junk, 0, false))
                P_CrossSpecialLine(&junk, 0, actor, false);

            if (!junk.special)
                actor->flags2 |= MF2_LINEDONE;

            actor->player = oldplayer;
        }
    }
}

//
// [XA] New MBF21 codepointers
//

//
// A_SpawnObject
// Basically just A_Spawn with better behavior and more args.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling actor's angle
//   args[2]: X spawn offset (fixed point), relative to calling actor
//   args[3]: Y spawn offset (fixed point), relative to calling actor
//   args[4]: Z spawn offset (fixed point), relative to calling actor
//   args[5]: X velocity (fixed point)
//   args[6]: Y velocity (fixed point)
//   args[7]: Z velocity (fixed point)
//
void A_SpawnObject(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t an;
    int     fan;
    int     dx, dy;
    mobj_t  *mo;

    if (!actor->state->args[0])
        return;

    // calculate position offsets
    an = actor->angle + (angle_t)(((int64_t)actor->state->args[1] << 16) / 360);
    fan = an >> ANGLETOFINESHIFT;
    dx = FixedMul(actor->state->args[2], finecosine[fan]) - FixedMul(actor->state->args[3], finesine[fan]);
    dy = FixedMul(actor->state->args[2], finesine[fan]) + FixedMul(actor->state->args[3], finecosine[fan]);

    // spawn it, yo
    if (!(mo = P_SpawnMobj(actor->x + dx, actor->y + dy, actor->z + actor->state->args[4], actor->state->args[0] - 1)))
        return;

    // [BH] inherit blood color from spawner
    if (actor->bloodcolor > REDBLOOD)
    {
        mo->bloodcolor = actor->bloodcolor;
        mo->colfunc = actor->colfunc;
        mo->altcolfunc = actor->altcolfunc;
    }

    // angle dangle
    mo->angle = an;

    // set velocity
    mo->momx = FixedMul(actor->state->args[5], finecosine[fan]) - FixedMul(actor->state->args[6], finesine[fan]);
    mo->momy = FixedMul(actor->state->args[5], finesine[fan]) + FixedMul(actor->state->args[6], finecosine[fan]);
    mo->momz = actor->state->args[7];

    // if spawned object is a missile, set target+tracer
    if (mo->info->flags & (MF_MISSILE | MF_BOUNCES))
    {
        // if spawner is also a missile, copy 'em
        if (actor->info->flags & (MF_MISSILE | MF_BOUNCES))
        {
            P_SetTarget(&mo->target, actor->target);
            P_SetTarget(&mo->tracer, actor->tracer);
        }
        else
        {
            // otherwise, set 'em as if a monster fired 'em
            P_SetTarget(&mo->target, actor);
            P_SetTarget(&mo->tracer, actor->target);
        }
    }

    // [XA] don't bother with the don't-inherit-friendliness hack
    // that exists in A_Spawn, 'cause WTF is that about anyway?
}

//
// A_MonsterProjectile
// A parameterized monster projectile attack.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling actor's angle
//   args[2]: Pitch (degrees, in fixed point), relative to calling actor's pitch; approximated
//   args[3]: X/Y spawn offset, relative to calling actor's angle
//   args[4]: Z spawn offset, relative to actor's default projectile fire height
//
void A_MonsterProjectile(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int     an;
    mobj_t  *mo;
    mobj_t  *target = actor->target;

    if (!target || !actor->state->args[0])
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (!(mo = P_SpawnMissile(actor, target, actor->state->args[0] - 1)))
        return;

    // adjust angle
    mo->angle += (angle_t)(((int64_t)actor->state->args[1] << 16) / 360);
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);

    // adjust pitch (approximated, using DOOM's ye olde
    // finetangent table; same method as monster aim)
    mo->momz += FixedMul(mo->info->speed, DegToSlope(actor->state->args[2]));

    // adjust position
    an = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
    mo->x += FixedMul(actor->state->args[3], finecosine[an]);
    mo->y += FixedMul(actor->state->args[3], finesine[an]);
    mo->z += actor->state->args[4];

    // always set the 'tracer' field, so this pointer
    // can be used to fire seeker missiles at will.
    P_SetTarget(&mo->tracer, target);
}

//
// A_MonsterBulletAttack
// A parameterized monster bullet attack.
//   args[0]: Horizontal spread (degrees, in fixed point)
//   args[1]: Vertical spread (degrees, in fixed point)
//   args[2]: Number of bullets to fire; if not set, defaults to 1
//   args[3]: Base damage of attack (e.g. for 3d5, customize the 3); if not set, defaults to 3
//   args[4]: Attack damage modulus (e.g. for 3d5, customize the 5); if not set, defaults to 5
//
void A_MonsterBulletAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int numbullets;
    int aimslope;

    if (!actor->target)
        return;

    numbullets = actor->state->args[2];

    A_FaceTarget(actor, NULL, NULL);
    S_StartSound(actor, actor->info->attacksound);

    aimslope = P_AimLineAttack(actor, actor->angle, MISSILERANGE, 0);

    for (int i = 0; i < numbullets; i++)
        P_LineAttack(actor, actor->angle + P_RandomHitscanAngle(actor->state->args[0]), MISSILERANGE,
            aimslope + P_RandomHitscanSlope(actor->state->args[1]), (M_Random() % actor->state->args[4] + 1) * actor->state->args[3]);
}

//
// A_MonsterMeleeAttack
// A parameterized monster melee attack.
//   args[0]: Base damage of attack (e.g. for 3d8, customize the 3); if not set, defaults to 3
//   args[1]: Attack damage modulus (e.g. for 3d8, customize the 8); if not set, defaults to 8
//   args[2]: Sound to play if attack hits
//   args[3]: Range (fixed point); if not set, defaults to monster's melee range
//
void A_MonsterMeleeAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int     range;
    mobj_t  *target = actor->target;

    if (!target)
        return;

    if (!(range = actor->state->args[3]))
        range = actor->info->meleerange;

    range += target->info->radius - 20 * FRACUNIT;

    A_FaceTarget(actor, NULL, NULL);

    if (!P_CheckRange(actor, range))
        return;

    S_StartSound(actor, actor->state->args[2]);

    P_DamageMobj(target, actor, actor, (M_Random() % actor->state->args[1] + 1) * actor->state->args[0], true, false);
}

//
// A_RadiusDamage
// A parameterized version of A_Explode. Friggin' finally. :P
//   args[0]: Damage (int)
//   args[1]: Radius (also int; no real need for fractional precision here)
//
void A_RadiusDamage(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->state)
        return;

    P_RadiusAttack(actor, actor->target, actor->state->args[0], actor->state->args[1], false);
    P_ShakeOnExplode(actor);
}

//
// A_NoiseAlert
// Alerts nearby monsters (via sound) to the calling actor's target's presence.
//
void A_NoiseAlert(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->target)
        return;

    P_NoiseAlert(actor->target, actor);
}

//
// A_HealChase
// A parameterized version of A_VileChase.
//   args[0]: State to jump to on the calling actor when resurrecting a corpse
//   args[1]: Sound to play when resurrecting a corpse
//
void A_HealChase(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor)
        return;

    if (!P_HealCorpse(actor, actor->info->radius, actor->state->args[0], actor->state->args[1]))
        A_Chase(actor, NULL, NULL);
}

//
// A_SeekTracer
// A parameterized seeker missile function.
//   args[0]: direct-homing threshold angle (degrees, in fixed point)
//   args[1]: maximum turn angle (degrees, in fixed point)
//
void A_SeekTracer(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor)
        return;

    P_SeekerMissile(actor, &actor->tracer, FixedToAngle(actor->state->args[0]), FixedToAngle(actor->state->args[1]), true);
}

//
// A_FindTracer
// Search for a valid tracer (seek target), if the calling actor doesn't already have one.
//   args[0]: field-of-view to search in (degrees, in fixed point); if zero, will search in all directions
//   args[1]: distance to search (map blocks, i.e. 128 units)
//
void A_FindTracer(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor || actor->tracer)
        return;

    actor->tracer = P_RoughTargetSearch(actor, FixedToAngle(actor->state->args[0]), actor->state->args[1]);
}

//
// A_ClearTracer
// Clear current tracer (seek target).
//
void A_ClearTracer(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor)
        return;

    actor->tracer = NULL;
}

//
// A_JumpIfHealthBelow
// Jumps to a state if caller's health is below the specified threshold.
//   args[0]: State to jump to
//   args[1]: Health threshold
//
void A_JumpIfHealthBelow(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor)
        return;

    if (actor->health < actor->state->args[1])
        P_SetMobjState(actor, actor->state->args[0]);
}

//
// A_JumpIfTargetInSight
// Jumps to a state if caller's target is in line-of-sight.
//   args[0]: State to jump to
//   args[1]: Field-of-view to check (degrees, in fixed point); if zero, will check in all directions
//
void A_JumpIfTargetInSight(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t fieldofview;
    mobj_t  *target;

    if (!actor || !(target = actor->target))
        return;

    // Check FOV first since it's faster
    if ((fieldofview = FixedToAngle(actor->state->args[1])) > 0 && !P_CheckFOV(actor, target, fieldofview))
        return;

    if (P_CheckSight(actor, target))
        P_SetMobjState(actor, actor->state->args[0]);
}

//
// A_JumpIfTargetCloser
// Jumps to a state if caller's target is closer than the specified distance.
//   args[0]: State to jump to
//   args[1]: Distance threshold
//
void A_JumpIfTargetCloser(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target;

    if (!actor || !(target = actor->target))
        return;

    if (actor->state->args[1] > P_ApproxDistance(actor->x - target->x, actor->y - target->y))
        P_SetMobjState(actor, actor->state->args[0]);
}

//
// A_JumpIfTracerInSight
// Jumps to a state if caller's tracer (seek target) is in line-of-sight.
//   args[0]: State to jump to
//   args[1]: Field-of-view to check (degrees, in fixed point); if zero, will check in all directions
//
void A_JumpIfTracerInSight(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t fieldofview;

    if (!actor || !actor->tracer)
        return;

    // Check FOV first since it's faster
    if ((fieldofview = FixedToAngle(actor->state->args[1])) > 0 && !P_CheckFOV(actor, actor->tracer, fieldofview))
        return;

    if (P_CheckSight(actor, actor->tracer))
        P_SetMobjState(actor, actor->state->args[0]);
}

//
// A_JumpIfTracerCloser
// Jumps to a state if caller's tracer (seek target) is closer than the specified distance.
//   args[0]: State to jump to
//   args[1]: Distance threshold (fixed point)
//
void A_JumpIfTracerCloser(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor || !actor->tracer)
        return;

    if (actor->state->args[1] > P_ApproxDistance(actor->x - actor->tracer->x, actor->y - actor->tracer->y))
        P_SetMobjState(actor, actor->state->args[0]);
}

//
// A_JumpIfFlagsSet
// Jumps to a state if caller has the specified thing flags set.
//   args[0]: State to jump to
//   args[1]: Standard flag(s) to check
//   args[2]: MBF21 flag(s) to check
//
void A_JumpIfFlagsSet(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int flags;
    int mbf21flags;

    if (!actor)
        return;

    flags = actor->state->args[1];
    mbf21flags = actor->state->args[2];

    if ((actor->flags & flags) == flags && (actor->mbf21flags & mbf21flags) == mbf21flags)
        P_SetMobjState(actor, actor->state->args[0]);
}

//
// A_AddFlags
// Adds the specified thing flags to the caller.
//   args[0]: Standard flag(s) to add
//   args[1]: MBF21 flag(s) to add
//
void A_AddFlags(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int     flags;
    int     mbf21flags;
    bool    updateblockmap;

    if (!actor)
        return;

    flags = actor->state->args[0];
    mbf21flags = actor->state->args[1];

    // unlink/relink the thing from the blockmap if
    // the NOBLOCKMAP or NOSECTOR flags are added
    updateblockmap = ((flags & MF_NOBLOCKMAP) && !(actor->flags & MF_NOBLOCKMAP))
        || ((flags & MF_NOSECTOR) && !(actor->flags & MF_NOSECTOR));

    if (updateblockmap)
        P_UnsetThingPosition(actor);

    actor->flags |= flags;
    actor->mbf21flags |= mbf21flags;

    if (updateblockmap)
        P_SetThingPosition(actor);

    R_UpdateMobjColfunc(actor);
}

//
// A_RemoveFlags
// Removes the specified thing flags from the caller.
//   args[0]: flag(s) to remove
//   args[1]: MBF21 flag(s) to remove
//
void A_RemoveFlags(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int     flags;
    int     mbf21flags;
    bool    updateblockmap;

    if (!actor)
        return;

    flags = actor->state->args[0];
    mbf21flags = actor->state->args[1];

    // unlink/relink the thing from the blockmap if
    // the NOBLOCKMAP or NOSECTOR flags are removed
    updateblockmap = ((flags & MF_NOBLOCKMAP) && (actor->flags & MF_NOBLOCKMAP))
        || ((flags & MF_NOSECTOR) && (actor->flags & MF_NOSECTOR));

    if (updateblockmap)
        P_UnsetThingPosition(actor);

    actor->flags &= ~flags;
    actor->mbf21flags &= ~mbf21flags;

    if (updateblockmap)
        P_SetThingPosition(actor);

    R_UpdateMobjColfunc(actor);
}
