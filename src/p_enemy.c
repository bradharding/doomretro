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

#include <ctype.h>

#include "doomstat.h"
#include "g_game.h"
#include "m_random.h"
#include "p_local.h"
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

//
// P_NewChaseDir related LUT.
//
dirtype_t opposite[] =
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

dirtype_t diags[] =
{
    DI_NORTHWEST,
    DI_NORTHEAST,
    DI_SOUTHWEST,
    DI_SOUTHEAST
};

void A_Fall(mobj_t *actor);

//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//

//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

void P_RecursiveSound(sector_t *sec, int soundblocks, mobj_t *soundtarget)
{
    int i;

    // wake up all monsters in this sector
    if (sec->validcount == validcount && sec->soundtraversed <= soundblocks + 1)
        return;         // already flooded

    sec->validcount = validcount;
    sec->soundtraversed = soundblocks + 1;
    sec->soundtarget = soundtarget;

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

    if (!P_CheckSight(actor, actor->target))
        return false;

    return true;
}

//
// P_CheckMissileRange
//
boolean P_CheckMissileRange(mobj_t *actor)
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
    int                 l;

    // Short-circuit: it's on a lift which is active.
    if (sec->specialdata && ((thinker_t *)sec->specialdata)->function.acp1 == T_PlatRaise)
        return true;

    // Check to see if it's in a sector which can be activated as a lift.
    if ((line.tag = sec->tag))
        for (l = -1; (l = P_FindLineFromLineTag(&line, l)) >= 0;)
            switch (lines[l].special)
            {
                case W1_LowerLiftWait3SecondsRise:
                case S1_RaiseFloorBy32UnitsChangeFloorTextureAndType:
                case S1_RaiseFloorBy24UnitsChangeFloorTextureAndType:
                case S1_RaiseFloorToNextFloorChangeFloorTextureAndType:
                case S1_LowerLiftWait3SecondsRise:
                case W1_RaiseFloorToNextFloorChangeFloorTextureAndType:
                case G1_RaiseFloorToNextFloorChangeFloorTextureAndType:
                case W1_StartUpDownMovingFloor:
                case SR_LowerLiftWait3SecondsRise:
                case SR_RaiseFloorBy24UnitsChangeFloorTextureAndType:
                case SR_RaiseFloorBy32UnitsChangeFloorTextureAndType:
                case SR_RaiseFloorToNextFloorChangeFloorTextureAndType:
                case WR_StartUpDownMovingFloor:
                case WR_LowerLiftWait3SecondsRise:
                case WR_RaiseFloorToNextFloorChangeFloorTextureAndType:
                case WR_LowerFastLiftWait3SecondsRise:
                case W1_LowerFastLiftWait3SecondsRise:
                case S1_LowerFastLiftWait3SecondsRise:
                case SR_LowerFastLiftWait3SecondsRise:
                    return true;
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
    const ceiling_t             *cl;    // Crushing ceiling
    int                         dir = 0;

    for (seclist = actor->touching_sectorlist; seclist; seclist = seclist->m_tnext)
        if ((cl = seclist->m_sector->specialdata) && cl->thinker.function.acp1 == T_MoveCeiling)
            dir |= cl->direction;

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
extern line_t **spechit;
extern int    numspechit;

boolean P_Move(mobj_t *actor, boolean dropoff)
{
    fixed_t     tryx;
    fixed_t     tryy;

    int         speed;

    if (actor->movedir == DI_NODIR)
        return false;

    speed = actor->info->speed;

    tryx = actor->x + speed * xspeed[actor->movedir];
    tryy = actor->y + speed * yspeed[actor->movedir];

    if (!P_TryMove(actor, tryx, tryy, dropoff))
    {
        int good;

        // open any specials
        if ((actor->flags & MF_FLOAT) && floatok)
        {
            // must adjust height
            if (actor->z < tmfloorz)
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
                good |= (spechit[numspechit] == blockline ? 1 : 2);

        return (good && ((P_Random() >= 230) ^ (good & 1)));
    }
    else
        actor->flags &= ~MF_INFLOAT;

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
boolean P_TryWalk(mobj_t *actor)
{
    if (!P_SmartMove(actor))
        return false;

    actor->movecount = P_Random() & 15;
    return true;
}

void P_NewChaseDir(mobj_t *actor)
{
    fixed_t     deltax = actor->target->x - actor->x;
    fixed_t     deltay = actor->target->y - actor->y;

    dirtype_t   d[3];

    int         tdir;
    dirtype_t   olddir = (dirtype_t)actor->movedir;

    dirtype_t   turnaround = opposite[olddir];

    if (deltax > 10 * FRACUNIT)
        d[1] = DI_EAST;
    else if (deltax < -10 * FRACUNIT)
        d[1] = DI_WEST;
    else
        d[1] = DI_NODIR;

    if (deltay < -10 * FRACUNIT)
        d[2] = DI_SOUTH;
    else if (deltay > 10 * FRACUNIT)
        d[2] = DI_NORTH;
    else
        d[2] = DI_NODIR;

    // try direct route
    if (d[1] != DI_NODIR && d[2] != DI_NODIR)
    {
        actor->movedir = diags[((deltay < 0) << 1) + (deltax > 0)];
        if (actor->movedir != (int)turnaround && P_TryWalk(actor))
            return;
    }

    // try other directions
    if (P_Random() > 200 || ABS(deltay) > ABS(deltax))
    {
        tdir = d[1];
        d[1] = d[2];
        d[2] = (dirtype_t)tdir;
    }

    if (d[1] == turnaround)
        d[1] = DI_NODIR;
    if (d[2] == turnaround)
        d[2] = DI_NODIR;

    if (d[1] != DI_NODIR)
    {
        actor->movedir = d[1];

        if (P_TryWalk(actor))
            return;             // either moved forward or attacked
    }

    if (d[2] != DI_NODIR)
    {
        actor->movedir = d[2];

        if (P_TryWalk(actor))
            return;
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir != DI_NODIR)
    {
        actor->movedir = olddir;

        if (P_TryWalk(actor))
            return;
    }

    // randomly determine direction of search
    if (P_Random() & 1)
    {
        for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
        {
            if (tdir != (int)turnaround)
            {
                actor->movedir = tdir;

                if (P_TryWalk(actor))
                    return;
            }
        }
    }
    else
    {
        for (tdir = DI_SOUTHEAST; tdir != DI_EAST - 1; tdir--)
        {
            if (tdir != turnaround)
            {
                actor->movedir = tdir;

                if (P_TryWalk(actor))
                    return;
            }
        }
    }

    if (turnaround != DI_NODIR)
    {
        actor->movedir = turnaround;

        if (P_TryWalk(actor))
            return;
    }

    actor->movedir = DI_NODIR;  // cannot move
}

#define MONS_LOOK_RANGE (32 * 64 * FRACUNIT)

boolean P_LookForMonsters(mobj_t *actor)
{
    mobj_t      *mo;
    thinker_t   *think;

    if (!P_CheckSight(players[0].mo, actor))
        return false;           // player can't see monster

    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;           // not a mobj thinker

        mo = (mobj_t *)think;
        if ((!(mo->flags & MF_COUNTKILL) && mo->type != MT_SKULL)
            || mo == actor || mo->health <= 0)
            continue;           // not a valid monster

        if (P_ApproxDistance(actor->x - mo->x, actor->y - mo->y) > MONS_LOOK_RANGE)
            continue;           // out of range

        if (!P_CheckSight(actor, mo))
            continue;           // out of sight

        // Found a target monster
        actor->lastenemy = actor->target;
        actor->target = mo;
        return true;
    }
    return false;
}

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
boolean P_LookForPlayers(mobj_t *actor, boolean allaround)
{
    int         c;
    int         stop;
    player_t    *player;
    angle_t     an;
    fixed_t     dist;

    if (/*!netgame && */infight)
        // player is dead, look for monsters
        return P_LookForMonsters(actor);
    c = 0;
    stop = (actor->lastlook - 1) % (MAXPLAYERS - 1);

    for (; ; actor->lastlook = (actor->lastlook + 1) & (MAXPLAYERS - 1))
    {
        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == 2 || actor->lastlook == stop)
        {
            // done looking
            return false;
        }

        player = &players[actor->lastlook];

        if (player->health <= 0)
            continue;           // dead

        if (!P_CheckSight(actor, player->mo))
            continue;           // out of sight

        dist = P_ApproxDistance(player->mo->x - actor->x, player->mo->y - actor->y);

        if (!allaround)
        {
            an = R_PointToAngle2(actor->x, actor->y, player->mo->x, player->mo->y) - actor->angle;

            if (an > ANG90 && an < ANG270)
            {
                // if real close, react anyway
                if (dist > MELEERANGE)
                    continue;   // behind back
            }
        }

        if (player->mo->flags & MF_SHADOW)
        {
            // player is invisible
            if (dist > 2 * MELEERANGE
                && P_ApproxDistance(player->mo->momx, player->mo->momy) < 5 * FRACUNIT)
            {
                // player is sneaking - can't detect
                return false;
            }
            if (P_Random() < 225)
            {
                // player isn't sneaking, but still didn't detect
                return false;
            }
        }

        actor->target = player->mo;
        return true;
    }
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t *mo)
{
    thinker_t   *th;
    mobj_t      *mo2;
    line_t      junk;

    A_Fall(mo);

    // scan the remaining thinkers
    // to see if all Keens are dead
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 != P_MobjThinker)
            continue;

        mo2 = (mobj_t *)th;
        if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
        {
            // other Keen not dead
            return;
        }
    }

    junk.tag = 666;
    EV_DoDoor(&junk, open);
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
    mobj_t      *targ = actor->subsector->sector->soundtarget;

    actor->threshold = 0;       // any shot will wake up
    
    if (targ && (targ->flags & MF_SHOOTABLE))
    {
        actor->target = targ;

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

    P_SetMobjState(actor, (statenum_t)actor->info->seestate);
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
            P_SetMobjState(actor, (statenum_t)actor->info->spawnstate);
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

        P_SetMobjState(actor, (statenum_t)actor->info->meleestate);
        return;
    }

    // check for missile attack
    if (actor->info->missilestate)
    {
        if (gameskill < sk_nightmare && !fastparm && actor->movecount)
            goto nomissile;

        if (!P_CheckMissileRange(actor))
            goto nomissile;

        P_SetMobjState(actor, (statenum_t)actor->info->missilestate);
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

    actor->angle = R_PointToAngle2(actor->x, actor->y,
                                   actor->target->x, actor->target->y);

    if (actor->target->flags & MF_SHADOW)
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

    if (!actor->target
        || actor->target->health <= 0
        || !P_CheckSight(actor, actor->target))
    {
        P_SetMobjState(actor, (statenum_t)actor->info->seestate);
    }
}

void A_SpidRefire(mobj_t *actor)
{
    // keep firing unless target got out of sight
    A_FaceTarget(actor);

    if (P_Random() < 10)
        return;

    if (!actor->target
        || actor->target->health <= 0
        || !P_CheckSight(actor, actor->target))
    {
        P_SetMobjState(actor, (statenum_t)actor->info->seestate);
    }
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
    if (!actor->target)
        return;

    A_FaceTarget(actor);
    P_SpawnMissile(actor, actor->target, MT_ROCKET);
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
    actor->z += 16 * FRACUNIT;          // so missile spawns higher
    mo = P_SpawnMissile(actor, actor->target, MT_TRACER);
    actor->z -= 16 * FRACUNIT;          // back to normal

    mo->x += mo->momx;
    mo->y += mo->momy;
    mo->tracer = actor->target;
}

#define TRACEANGLE 0xc000000;

void A_Tracer(mobj_t *actor)
{
    angle_t     exact;
    fixed_t     dist;
    fixed_t     slope;
    mobj_t      *dest;
    int         speed;

    // spawn a puff of smoke behind the rocket
    P_SpawnSmokeTrail(actor->x, actor->y, actor->z - 2 * FRACUNIT, actor->angle);

    if ((gametic - levelstarttic) & 3)
        return;

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
    int xl, xh;
    int yl, yh;
    int bx, by;

    if (actor->movedir != DI_NODIR)
    {
        // check for corpses to raise
        viletryx = actor->x + actor->info->speed * xspeed[actor->movedir];
        viletryy = actor->y + actor->info->speed * yspeed[actor->movedir];

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
                    static char message[128];

                    actor->target = corpsehit;
                    A_FaceTarget(actor);
                    actor->target = temp;

                    P_SetMobjState(actor, S_VILE_HEAL1);
                    S_StartSound(corpsehit, sfx_slop);
                    info = corpsehit->info;

                    P_SetMobjState(corpsehit, (statenum_t)info->raisestate);

                    corpsehit->height = info->height;
                    corpsehit->radius = info->radius;
                    corpsehit->flags = info->flags;
                    corpsehit->flags2 = info->flags2;
                    if (corpsehit->shadow)
                        corpsehit->shadow->flags2 &= ~MF2_MIRRORED;
                    corpsehit->health = info->spawnhealth;
                    corpsehit->target = NULL;
                    corpsehit->lastenemy = NULL;

                    if (corpsehit->type == MT_TROOP)
                        corpsehit->colfunc = troopcolfunc;

                    players[0].killcount--;

                    sprintf(message, "%s resurrected %s.\n", actor->info->description,
                        corpsehit->info->description);
                    message[0] = toupper(message[0]);
                    printf(message);

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
    mobj_t      *dest = actor->tracer;
    mobj_t      *target;
    unsigned    an;

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

    actor->tracer = fog;
    fog->target = actor;
    fog->tracer = actor->target;
    A_Fire(fog);
}

//
// A_VileAttack
//
void A_VileAttack(mobj_t *actor)
{
    mobj_t      *fire;
    int         an;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (!P_CheckSight(actor, actor->target))
        return;

    S_StartSound(actor, sfx_barexp);
    P_DamageMobj(actor->target, actor, actor, 20);
    actor->target->momz = 1000 * FRACUNIT / actor->target->info->mass;

    an = actor->angle >> ANGLETOFINESHIFT;

    fire = actor->tracer;

    if (!fire)
        return;

    // move the fire between the vile and the player
    fire->x = actor->target->x - FixedMul(24 * FRACUNIT, finecosine[an]);
    fire->y = actor->target->y - FixedMul(24 * FRACUNIT, finesine[an]);
    P_RadiusAttack(fire, actor, 70);
}

//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//
#define FATSPREAD (ANG90 / 8)

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
    if (mo)
    {
        mo->angle += FATSPREAD;
        an = mo->angle >> ANGLETOFINESHIFT;
        mo->momx = FixedMul(mo->info->speed, finecosine[an]);
        mo->momy = FixedMul(mo->info->speed, finesine[an]);
    }
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
    if (mo)
    {
        mo->angle -= FATSPREAD * 2;
        an = mo->angle >> ANGLETOFINESHIFT;
        mo->momx = FixedMul(mo->info->speed, finecosine[an]);
        mo->momy = FixedMul(mo->info->speed, finesine[an]);
    }
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
    if (mo)
    {
        mo->angle -= FATSPREAD / 2;
        an = mo->angle >> ANGLETOFINESHIFT;
        mo->momx = FixedMul(mo->info->speed, finecosine[an]);
        mo->momy = FixedMul(mo->info->speed, finesine[an]);
    }

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    if (mo)
    {
        mo->angle += FATSPREAD / 2;
        an = mo->angle >> ANGLETOFINESHIFT;
        mo->momx = FixedMul(mo->info->speed, finecosine[an]);
        mo->momy = FixedMul(mo->info->speed, finesine[an]);
    }
}

//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED (20 * FRACUNIT)

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

//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void A_PainShootSkull(mobj_t *actor, angle_t angle)
{
    fixed_t     x, y, z;
    mobj_t      *newmobj;
    angle_t     an;
    int         prestep;

    an = angle >> ANGLETOFINESHIFT;

    prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[MT_SKULL].radius) / 2;

    x = actor->x + FixedMul(prestep, finecosine[an]);
    y = actor->y + FixedMul(prestep, finesine[an]);
    z = actor->z + 8 * FRACUNIT;

    if (P_CheckLineSide(actor, x, y))
        return;

    newmobj = P_SpawnMobj(x , y, z, MT_SKULL);

    newmobj->flags &= ~MF_COUNTKILL;

    if (!P_TryMove(newmobj, newmobj->x, newmobj->y, false))
    {
        P_RemoveMobj(newmobj);
        return;
    }

    newmobj->target = actor->target;
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
    A_Fall(actor);
    A_PainShootSkull(actor, actor->angle + ANG90);
    A_PainShootSkull(actor, actor->angle + ANG180);
    A_PainShootSkull(actor, actor->angle + ANG270);
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
    mobj_t      *mo2;
    line_t      junk;
    int         i;

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

    // make sure there is a player alive for victory
    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i] && players[i].health > 0)
            break;

    if (i == MAXPLAYERS)
        return;         // no one left alive, so do not end game

    // scan the remaining thinkers to see
    // if all bosses are dead
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;

        mo2 = (mobj_t *)th;
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
                        EV_DoDoor(&junk, blazeOpen);
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
    mobj_t              *mo;
    mobj_t              *found = NULL;

    // find all the target spots
    for (thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
    {
        if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;   // not a mobj

        mo = (mobj_t *)thinker;

        if (mo->type == MT_BOSSTARGET)
        {
            if (count == braintargeted) // This one the one that we want?
            {
                braintargeted++;        // Yes.
                return mo;
            }
            count++;
            if (found == NULL)          // Remember first one in case we wrap.
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
        newmobj->target = targ;
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
        if ((unsigned)dist < (unsigned)mo->reactiontime)
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

            if ((P_LookForPlayers(newmobj, true) == 0)
                || (P_SetMobjState(newmobj, (statenum_t)newmobj->info->seestate)))
                // telefrag anything in this spot
                P_TeleportMove(newmobj, newmobj->x, newmobj->y, newmobj->z, true); // killough 8/9/98

            totalkills++;
        }
    }

    // remove self (i.e., cube).
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
