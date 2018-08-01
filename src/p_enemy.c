/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#include <string.h>

#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"

#define BARRELRANGE (512 * FRACUNIT)

static dirtype_t opposite[] =
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

static dirtype_t diags[] =
{
    DI_NORTHWEST,
    DI_NORTHEAST,
    DI_SOUTHWEST,
    DI_SOUTHEAST
};

int barrelms = 0;

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
// killough 5/5/98: reformatted, cleaned up
//
static void P_RecursiveSound(sector_t *sec, int soundblocks, mobj_t *soundtarget)
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

        if (flags & ML_SOUNDBLOCK)
        {
            if (!soundblocks)
                P_RecursiveSound(sides[line->sidenum[(sides[line->sidenum[0]].sector == sec)]].sector, 1, soundtarget);
        }
        else
            P_RecursiveSound(sides[line->sidenum[(sides[line->sidenum[0]].sector == sec)]].sector, soundblocks, soundtarget);
    }
}

//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert(mobj_t *target)
{
    // [BH] don't alert if notarget is enabled
    if (target->player && (viewplayer->cheats & CF_NOTARGET))
        return;

    validcount++;
    P_RecursiveSound(target->subsector->sector, 0, target);
}

//
// P_CheckMeleeRange
//
dboolean P_CheckMeleeRange(mobj_t *actor)
{
    mobj_t  *target = actor->target;

    if (!target)
        return false;

    if (P_ApproxDistance(target->x - actor->x, target->y - actor->y) >= MELEERANGE - 20 * FRACUNIT + target->info->radius)
        return false;

    // [BH] check difference in height as well
    if (!infiniteheight && (target->z > actor->z + actor->height || actor->z > target->z + target->height))
        return false;

    if (!P_CheckSight(actor, target))
        return false;

    return true;
}

//
// P_CheckMissileRange
//
static dboolean P_CheckMissileRange(mobj_t *actor)
{
    fixed_t     dist;
    mobj_t      *target = actor->target;
    mobjtype_t  type;

    if (!P_CheckSight(actor, target))
        return false;

    if (actor->flags & MF_JUSTHIT)
    {
        // the target just hit the enemy, so fight back!
        actor->flags &= ~MF_JUSTHIT;
        return true;
    }

    if (actor->reactiontime)
        return false;                   // do not attack yet

    dist = (P_ApproxDistance(actor->x - target->x, actor->y - target->y) >> FRACBITS) - 64;

    if (!actor->info->meleestate)
        dist -= 128;                    // no melee attack, so fire more

    type = actor->type;

    if (type == MT_VILE)
    {
        if (dist > 14 * 64)
            return false;               // too far away
    }
    else if (type == MT_UNDEAD)
    {
        if (dist < 196)
            return false;               // close for fist attack

        dist >>= 1;
    }
    else if (type == MT_CYBORG)
    {
        dist >>= 1;

        if (dist > 160)
            dist = 160;
    }
    else if (type == MT_SPIDER || type == MT_SKULL)
        dist >>= 1;

    if (dist > 200)
        dist = 200;

    if (M_Random() < dist)
        return false;

    return true;
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
    int dir = 0;

    for (const struct msecnode_s *seclist = actor->touching_sectorlist; seclist; seclist = seclist->m_tnext)
    {
        const ceiling_t *ceiling = seclist->m_sector->ceilingdata;  // Crushing ceiling

        if (ceiling && ceiling->thinker.function == T_MoveCeiling)
            dir |= ceiling->direction;
    }

    return dir;
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
static const fixed_t    xspeed[8] = { FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000 };
static const fixed_t    yspeed[8] = { 0, 47000, FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000 };

// 1/11/98 killough: Limit removed on special lines crossed
extern line_t   **spechit;
extern int      numspechit;

static dboolean P_Move(mobj_t *actor, dboolean dropoff) // killough 9/12/98
{
    fixed_t tryx, tryy;
    fixed_t deltax, deltay;
    fixed_t origx, origy;
    int     movefactor;
    int     friction = ORIG_FRICTION;
    int     speed;

    if (actor->movedir == DI_NODIR)
        return false;

    // killough 10/98: make monsters get affected by ice and sludge too:
    movefactor = P_GetMoveFactor(actor, &friction);

    speed = actor->info->speed;

    if (friction < ORIG_FRICTION        // sludge
        && !(speed = ((ORIG_FRICTION_FACTOR - (ORIG_FRICTION_FACTOR - movefactor) / 2) * speed) / ORIG_FRICTION_FACTOR))
        speed = 1;                      // always give the monster a little bit of speed

    tryx = (origx = actor->x) + (deltax = speed * xspeed[actor->movedir]);
    tryy = (origy = actor->y) + (deltay = speed * yspeed[actor->movedir]);

    if (!P_TryMove(actor, tryx, tryy, dropoff))
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

        if (!good)
            return false;

        return (good && ((M_Random() >= 230) ^ (good & 1)));
    }
    else
    {
        actor->flags &= ~MF_INFLOAT;

        // killough 10/98:
        // Let normal momentum carry them, instead of steptoeing them across ice.
        if (friction > ORIG_FRICTION)
        {
            actor->x = origx;
            actor->y = origy;
            movefactor *= FRACUNIT / ORIG_FRICTION_FACTOR / 4;
            actor->momx += FixedMul(deltax, movefactor);
            actor->momy += FixedMul(deltay, movefactor);
        }
    }

    // killough 11/98: fall more slowly, under gravity, if felldown == true
    if (!(actor->flags & MF_FLOAT) && !felldown)
        actor->z = actor->floorz;

    return true;
}

//
// P_SmartMove
//
// killough 9/12/98: Same as P_Move, except smarter
//
static dboolean P_SmartMove(mobj_t *actor)
{
    mobj_t      *target = actor->target;
    dboolean    onlift;
    int         underdamage;

    // killough 9/12/98: stay on a lift if target is on one
    onlift = (target && target->health > 0 && target->subsector->sector->tag == actor->subsector->sector->tag
        && actor->subsector->sector->islift);

    underdamage = P_IsUnderDamage(actor);

    if (!P_Move(actor, false))
        return false;

    // killough 9/9/98: avoid crushing ceilings or other damaging areas
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
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
static dboolean P_TryWalk(mobj_t *actor)
{
    if (!P_SmartMove(actor))
        return false;

    actor->movecount = M_Random() & 15;
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
    dirtype_t       d[2];
    const dirtype_t olddir = actor->movedir;
    const dirtype_t turnaround = opposite[olddir];
    dboolean        attempts[NUMDIRS - 1];

    memset(&attempts, false, sizeof(attempts));

    d[0] = (deltax > 10 * FRACUNIT ? DI_EAST : (deltax < -10 * FRACUNIT ? DI_WEST : DI_NODIR));
    d[1] = (deltay < -10 * FRACUNIT ? DI_SOUTH : (deltay > 10 * FRACUNIT ? DI_NORTH : DI_NODIR));

    // try direct route
    if (d[0] != DI_NODIR && d[1] != DI_NODIR)
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
        SWAP(d[0], d[1]);

    if (d[0] == turnaround)
        d[0] = DI_NODIR;

    if (d[1] == turnaround)
        d[1] = DI_NODIR;

    if (d[0] != DI_NODIR && !attempts[d[0]])
    {
        actor->movedir = d[0];
        attempts[d[0]] = true;

        if (P_TryWalk(actor))
            return;     // either moved forward or attacked
    }

    if (d[1] != DI_NODIR && !attempts[d[1]])
    {
        actor->movedir = d[1];
        attempts[d[1]] = true;

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
        for (int tdir = DI_SOUTHEAST; tdir != (DI_EAST - 1); tdir--)
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

static dboolean PIT_AvoidDropoff(line_t *line)
{
    if (line->backsector                                // Ignore one-sided linedefs
        && tmbbox[BOXRIGHT] > line->bbox[BOXLEFT]
        && tmbbox[BOXLEFT] < line->bbox[BOXRIGHT]
        && tmbbox[BOXTOP] > line->bbox[BOXBOTTOM]       // Linedef must be contacted
        && tmbbox[BOXBOTTOM] < line->bbox[BOXTOP]
        && P_BoxOnLineSide(tmbbox, line) == -1)
    {
        const fixed_t   front = line->frontsector->floorheight;
        const fixed_t   back = line->backsector->floorheight;
        angle_t         angle;

        // The monster must contact one of the two floors,
        // and the other must be a tall dropoff (more than 24).
        if (back == floorz && front < floorz - FRACUNIT * 24)
            // front side dropoff
            angle = R_PointToAngle2(0, 0, line->dx, line->dy) >> ANGLETOFINESHIFT;
        else if (front == floorz && back < floorz - FRACUNIT * 24)
            // back side dropoff
            angle = R_PointToAngle2(line->dx, line->dy, 0, 0) >> ANGLETOFINESHIFT;
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
static fixed_t P_AvoidDropoff(mobj_t *actor)
{
    const int   yh = ((tmbbox[BOXTOP] = actor->y + actor->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    const int   yl = ((tmbbox[BOXBOTTOM] = actor->y - actor->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    const int   xh = ((tmbbox[BOXRIGHT] = actor->x + actor->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    const int   xl = ((tmbbox[BOXLEFT] = actor->x - actor->radius) - bmaporgx) >> MAPBLOCKSHIFT;

    floorz = actor->z;                                      // remember floor height
    dropoff_deltax = 0;
    dropoff_deltay = 0;

    // check lines
    validcount++;

    for (int bx = xl; bx <= xh; bx++)
        for (int by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, PIT_AvoidDropoff); // all contacted lines

    return (dropoff_deltax | dropoff_deltay);               // Non-zero if movement prescribed
}

//
// P_NewChaseDir
//
// killough 9/8/98: Split into two functions
//
static void P_NewChaseDir(mobj_t *actor)
{
    mobj_t  *target = actor->target;
    fixed_t deltax = target->x - actor->x;
    fixed_t deltay = target->y - actor->y;

    if (actor->floorz - actor->dropoffz > FRACUNIT * 24 && actor->z <= actor->floorz
        && !(actor->flags & (MF_DROPOFF | MF_FLOAT)) && P_AvoidDropoff(actor))   // Move away from dropoff
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

static dboolean P_LookForMonsters(mobj_t *actor)
{
    if (!P_CheckSight(viewplayer->mo, actor))
        return false;           // player can't see monster

    for (thinker_t *th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
    {
        mobj_t  *mo = (mobj_t *)th;

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
static dboolean P_LookForPlayers(mobj_t *actor, dboolean allaround)
{
    mobj_t  *mo;

    if (infight)
        // player is dead, look for monsters
        return P_LookForMonsters(actor);

    if (viewplayer->cheats & CF_NOTARGET)
        return false;

    mo = viewplayer->mo;

    if (viewplayer->health <= 0 || !P_CheckSight(actor, mo))
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

    if (!allaround)
    {
        const angle_t   an = R_PointToAngle2(actor->x, actor->y, mo->x, mo->y) - actor->angle;

        if (an > ANG90 && an < ANG270)
            // if real close, react anyway
            if (P_ApproxDistance(mo->x - actor->x, mo->y - actor->y) > MELEERANGE)
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

    actor->threshold = 60;

    return true;
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    line_t  junk;

    A_Fall(actor, NULL, NULL);

    // scan the remaining thinkers to see if all Keens are dead
    for (thinker_t *th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
    {
        mobj_t  *mo = (mobj_t *)th;

        if (mo != actor && mo->type == actor->type && mo->health > 0)
            return;         // other Keen not dead
    }

    junk.tag = 666;
    EV_DoDoor(&junk, doorOpen, VDOORSPEED);
}

//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->subsector->sector->soundtarget;

    actor->threshold = 0;       // any shot will wake up

    if (target && (target->flags & MF_SHOOTABLE))
    {
        P_SetTarget(&actor->target, target);

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
        int sound;

        switch (actor->info->seesound)
        {
            case sfx_posit1:
            case sfx_posit2:
            case sfx_posit3:
                sound = sfx_posit1 + M_Random() % 3;
                break;

            case sfx_bgsit1:
            case sfx_bgsit2:
                sound = sfx_bgsit1 + M_Random() % 2;
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
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t      *target = actor->target;
    mobjinfo_t  *info = actor->info;

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
        int delta = (actor->angle &= (7 << 29)) - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90 / 2;
        else if (delta < 0)
            actor->angle += ANG90 / 2;
    }

    if (!target || !(target->flags & MF_SHOOTABLE))
    {
        // look for a new target
        if (!P_LookForPlayers(actor, true))
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
    if (info->meleestate && P_CheckMeleeRange(actor))
    {
        if (info->attacksound)
            S_StartSound(actor, info->attacksound);

        P_SetMobjState(actor, info->meleestate);

        // killough 8/98: remember an attack
        if (!info->missilestate)
            actor->flags |= MF_JUSTHIT;

        return;
    }

    // check for missile attack
    if (info->missilestate)
    {
        if (gameskill < sk_nightmare && !fastparm && actor->movecount)
            goto nomissile;

        if (!P_CheckMissileRange(actor))
            goto nomissile;

        P_SetMobjState(actor, info->missilestate);
        actor->flags |= MF_JUSTATTACKED;
        return;
    }

nomissile:
    // chase towards player
    if (--actor->movecount < 0 || !P_SmartMove(actor))
        P_NewChaseDir(actor);

    // make active sound
    if (info->activesound && M_Random() < 3)
        S_StartSound(actor, info->activesound);
}

//
// A_FaceTarget
//
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    actor->flags &= ~MF_AMBUSH;
    actor->angle = R_PointToAngle2(actor->x, actor->y, target->x, target->y);

    if (target->flags & MF_FUZZ)
        actor->angle += M_NegRandom() << 21;
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
    P_LineAttack(actor, actor->angle + (M_NegRandom() << 20), MISSILERANGE,
        P_AimLineAttack(actor, actor->angle, MISSILERANGE), ((M_Random() % 5) + 1) * 3);
}

void A_SPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    S_StartSound(actor, sfx_shotgn);

    for (int i = 0; i < 3; i++)
        P_LineAttack(actor, actor->angle + (M_NegRandom() << 20), MISSILERANGE,
            P_AimLineAttack(actor, actor->angle, MISSILERANGE), ((M_Random() % 5) + 1) * 3);
}

void A_CPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!actor->target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    S_StartSound(actor, sfx_shotgn);
    P_LineAttack(actor, actor->angle + (M_NegRandom() << 20), MISSILERANGE,
        P_AimLineAttack(actor, actor->angle, MISSILERANGE), ((M_Random() % 5) + 1) * 3);
}

void A_CPosRefire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target;

    // keep firing unless target got out of sight
    A_FaceTarget(actor, NULL, NULL);

    if (M_Random() < 40)
        return;

    if (!(target = actor->target) ||target->health <= 0 || !P_CheckSight(actor, target))
        P_SetMobjState(actor, actor->info->seestate);
}

void A_SpidRefire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target;

    // keep firing unless target got out of sight
    A_FaceTarget(actor, NULL, NULL);

    if (M_Random() < 10)
        return;

    if (!(target = actor->target) || target->health <= 0 || !P_CheckSight(actor, target))
        P_SetMobjState(actor, actor->info->seestate);
}

void A_BspiAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

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
        S_StartSound(actor, sfx_claw);
        P_DamageMobj(target, actor, actor, ((M_Random() & 7) + 1) * 3, true);
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
        P_DamageMobj(target, actor, actor, (M_Random() % 10 + 1) * 4, true);
}

void A_HeadAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(target, actor, actor, (M_Random() % 6 + 1) * 10, true);
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
    mobj_t  *mo;
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);
    mo = P_SpawnMissile(actor, target, MT_ROCKET);

    // [BH] give cyberdemon rockets smoke trails
    if (r_rockettrails)
        mo->flags2 |= MF2_SMOKETRAIL;
}

void A_BruisAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    // [BH] fix baron nobles not facing targets correctly when attacking
    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        S_StartSound(actor, sfx_claw);
        P_DamageMobj(target, actor, actor, ((M_Random() & 7) + 1) * 10, true);
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

    if (leveltime & 3)
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
        S_StartSound(actor, sfx_skepch);
        P_DamageMobj(target, actor, actor, ((M_Random() % 10) + 1) * 6, true);
    }
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
static mobj_t   *corpsehit;
static fixed_t  viletryx;
static fixed_t  viletryy;

static dboolean PIT_VileCheck(mobj_t *thing)
{
    int         maxdist;
    dboolean    check;
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
    corpsehit->momx = 0;
    corpsehit->momy = 0;

    // [BH] fix potential of corpse being resurrected as a "ghost"
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
// Check for resurrecting a body
//
void A_VileChase(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int movedir = actor->movedir;

    if (movedir != DI_NODIR)
    {
        int xl;
        int xh;
        int yl;
        int yh;
        int speed = actor->info->speed;

        // check for corpses to raise
        viletryx = actor->x + speed * xspeed[movedir];
        viletryy = actor->y + speed * yspeed[movedir];

        xl = (viletryx - bmaporgx - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        xh = (viletryx - bmaporgx + MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        yl = (viletryy - bmaporgy - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        yh = (viletryy - bmaporgy + MAXRADIUS * 2) >> MAPBLOCKSHIFT;

        for (int bx = xl; bx <= xh; bx++)
            for (int by = yl; by <= yh; by++)
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
                    A_FaceTarget(actor, NULL, NULL);
                    actor->target = temp;

                    P_SetMobjState(actor, S_VILE_HEAL1);
                    S_StartSound(corpsehit, sfx_slop);
                    info = corpsehit->info;

                    P_SetMobjState(corpsehit, info->raisestate);

                    // [BH] fix potential of corpse being resurrected as a "ghost"
                    corpsehit->height = info->height;
                    corpsehit->radius = info->radius;
                    corpsehit->flags = info->flags;
                    corpsehit->flags2 = info->flags2;
                    corpsehit->health = info->spawnhealth;
                    corpsehit->shadowoffset = info->shadowoffset;
                    P_SetTarget(&corpsehit->target, NULL);
                    P_SetTarget(&corpsehit->lastenemy, NULL);

                    viewplayer->killcount--;
                    stat_monsterskilled--;
                    P_UpdateKillStat(corpsehit->type, -1);

                    // [BH] display an obituary message in the console
                    if (con_obituaries)
                        C_Obituary("%s %s resurrected %s %s.", (isvowel(actor->info->name1[0]) ? "An" : "A"),
                            actor->info->name1, (isvowel(info->name1[0]) ? "an" : "a"), info->name1);

                    // killough 8/29/98: add to appropriate thread
                    P_UpdateThinker(&corpsehit->thinker);

                    return;
                }
            }
    }

    // Return to normal attack.
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
    mobj_t  *dest = actor->tracer;
    mobj_t  *target;
    angle_t an;

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
    actor->floorz = actor->subsector->sector->floorheight;
    actor->ceilingz = actor->subsector->sector->ceilingheight;
}

void A_StartFire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
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

    S_StartSound(fog, sfx_flamst);
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
    P_DamageMobj(target, actor, actor, 20, true);

    // [BH] don't apply upward momentum from vile attack to player when no clipping mode on
    if (!target->player || !(target->flags & MF_NOCLIP))
        target->momz = 1000 * FRACUNIT / target->info->mass;

    an = actor->angle >> ANGLETOFINESHIFT;

    if (!(fire = actor->tracer))
        return;

    // move the fire between the vile and the player
    fire->x = target->x - FixedMul(24 * FRACUNIT, finecosine[an]);
    fire->y = target->y - FixedMul(24 * FRACUNIT, finesine[an]);
    P_RadiusAttack(fire, actor, 70, true);
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
    angle_t an;
    mobj_t  *mo;
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    // Change direction to...
    actor->angle += FATSPREAD;
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t an;
    mobj_t  *mo;
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    // Now here choose opposite deviation.
    actor->angle -= FATSPREAD;
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mo = P_SpawnMissile(actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD * 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack3(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t an;
    mobj_t  *mo;
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

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
#define SKULLSPEED  (20 * FRACUNIT)

void A_SkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t an;
    mobj_t  *dest = actor->target;

    if (!dest)
        return;

    actor->flags |= MF_SKULLFLY;

    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor, NULL, NULL);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
    actor->momy = FixedMul(SKULLSPEED, finesine[an]);
    actor->momz = (dest->z + (dest->height >> 1) - actor->z) /
        MAX(1, P_ApproxDistance(dest->x - actor->x, dest->y - actor->y) / SKULLSPEED);
}

void A_BetaSkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target || target->type == actor->type)
        return;

    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor, NULL, NULL);
    P_DamageMobj(target, actor, actor, ((M_Random() & 7) + 1) * actor->info->damage, true);
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
    fixed_t         x = actor->x + FixedMul(prestep, finecosine[an]);
    fixed_t         y = actor->y + FixedMul(prestep, finesine[an]);

    // [BH] removed check for number of lost souls

    // Check whether the Lost Soul is being fired through a 1-sided
    // wall or an impassible line, or a "monsters can't cross" line.
    // If it is, then we don't allow the spawn.
    if (P_CheckLineSide(actor, x, y))
        return;

    newmobj = P_SpawnMobj(x, y, actor->z + 8 * FRACUNIT, MT_SKULL);
    newmobj->flags &= ~MF_COUNTKILL;

    // killough 8/29/98: add to appropriate thread
    P_UpdateThinker(&newmobj->thinker);

    if (!P_TryMove(newmobj, newmobj->x, newmobj->y, false)
        // Check to see if the new Lost Soul's z value is above the
        // ceiling of its new sector, or below the floor. If so, kill it.
        || newmobj->z > newmobj->subsector->sector->ceilingheight - newmobj->height
        || newmobj->z < newmobj->subsector->sector->floorheight)
    {
        // kill it immediately
        P_DamageMobj(newmobj, actor, actor, 10000, true);
        return;
    }

    // [BH] put in attack state
    P_SetMobjState(newmobj, S_SKULL_ATK2);

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
    angle_t angle = actor->angle;

    A_Fall(actor, NULL, NULL);
    A_PainShootSkull(actor, angle + ANG90);
    A_PainShootSkull(actor, angle + ANG180);
    A_PainShootSkull(actor, angle + ANG270);
}

void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int sound;

    switch (actor->info->deathsound)
    {
        case sfx_None:
            return;

        case sfx_podth1:
        case sfx_podth2:
        case sfx_podth3:
            sound = sfx_podth1 + M_Random() % 3;
            break;

        case sfx_bgdth1:
        case sfx_bgdth2:
            sound = sfx_bgdth1 + M_Random() % 2;
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
    mo->momx = M_NegRandom() << 9;
    mo->momy = M_NegRandom() << 9;
    mo->momz = FRACUNIT * 2 + (M_Random() << 6);

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
    if (actor->info->painsound)
        S_StartSound(actor, actor->info->painsound);
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
    if (r_shake_barrels && actor->type == MT_BARREL)
    {
        mobj_t  *mo = viewplayer->mo;

        if (mo->z <= mo->floorz && P_ApproxDistance(actor->x - mo->x, actor->y - mo->y) < BARRELRANGE)
        {
            barrelms = I_GetTimeMS() + BARRELMS;

            if (gp_vibrate_barrels && vibrate)
            {
                XInputVibration(20000 * gp_vibrate_barrels / 100);
                barrelvibrationtics = TICRATE;
            }
        }
    }

    P_RadiusAttack(actor, actor->target, 128, true);
}

//
// A_BossDeath
// Possibly trigger special effects if on first boss level
//
void A_BossDeath(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    line_t  junk;

    if (gamemode == commercial)
    {
        if (gamemap != 7)
            return;

        if (actor->type != MT_FATSO && actor->type != MT_BABY)
            return;
    }
    else
    {
        switch (gameepisode)
        {
            case 1:
                if (gamemap != 8)
                    return;

                if (actor->type != MT_BRUISER)
                    return;

                break;

            case 2:
                if (gamemap != 8)
                    return;

                if (actor->type != MT_CYBORG)
                    return;

                break;

            case 3:
                if (gamemap != 8)
                    return;

                if (actor->type != MT_SPIDER)
                    return;

                break;

            case 4:
                switch (gamemap)
                {
                    case 6:
                        if (actor->type != MT_CYBORG)
                            return;

                        break;

                    case 8:
                        if (actor->type != MT_SPIDER)
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

    if (viewplayer->health <= 0)
        return;         // no one left alive, so do not end game

    actor->health = 0;  // P_KillMobj() sets this to -1

    // scan the remaining thinkers to see if all bosses are dead
    for (thinker_t *th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
    {
        mobj_t  *mo = (mobj_t *)th;

        if (mo != actor && mo->type == actor->type && mo->health)
            return;     // other boss not dead
    }

    // victory!
    if (gamemode == commercial)
    {
        if (gamemap == 7)
        {
            if (actor->type == MT_FATSO)
            {
                junk.tag = 666;
                EV_DoFloor(&junk, lowerFloorToLowest);
                return;
            }

            if (actor->type == MT_BABY)
            {
                junk.tag = 667;
                EV_DoFloor(&junk, raiseToTexture);
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
                        EV_DoDoor(&junk, doorBlazeOpen, VDOORSPEED * 4);
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
    // [BH] explosions are correctly centered
    for (int x = actor->x - 258 * FRACUNIT; x < actor->x + 258 * FRACUNIT; x += FRACUNIT * 8)
    {
        int     y = actor->y - 320 * FRACUNIT;
        int     z = 128 + M_Random() * 2 * FRACUNIT;
        mobj_t  *th = P_SpawnMobj(x, y, z, MT_ROCKET);

        th->momz = M_Random() * 512;
        P_SetMobjState(th, S_BRAINEXPLODE1);
        th->tics = MAX(1, th->tics - (M_Random() & 7));
        th->colfunc = tlcolfunc;
        th->flags2 &= ~MF2_CASTSHADOW;
    }

    S_StartSound(NULL, sfx_bosdth);
}

void A_BrainExplode(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *th = P_SpawnMobj(actor->x + M_NegRandom() * 2048, actor->y, 128 + M_Random() * 2 * FRACUNIT, MT_ROCKET);

    th->momz = M_Random() * 512;
    P_SetMobjState(th, S_BRAINEXPLODE1);
    th->tics = MAX(1, th->tics - (M_Random() & 7));
    th->colfunc = tlcolfunc;
    th->flags2 &= ~MF2_CASTSHADOW;
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
    for (thinker_t *th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
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

extern dboolean massacre;

void A_BrainSpit(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t          *target;
    static dboolean easy;

    easy = !easy;

    if (gameskill <= sk_easy && !easy)
        return;

    if (nomonsters || massacre)
        return;

    // shoot a cube at current target
    if ((target = A_NextBrainTarget()))
    {
        // spawn brain missile
        mobj_t  *newmobj = P_SpawnMissile(actor, target, MT_SPAWNSHOT);

        P_SetTarget(&newmobj->target, target);

        // Use the reactiontime to hold the distance (squared) from the target after the next move.
        newmobj->reactiontime = P_ApproxDistance(target->x - (actor->x + actor->momx),
            target->y - (actor->y + actor->momy));

        // killough 8/29/98: add to appropriate thread
        P_UpdateThinker(&newmobj->thinker);

        S_StartSound(NULL, sfx_bospit);
    }
}

void A_SpawnFly(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int     dist;
    mobj_t  *target = actor->target;

    if (!target)
        return;

    // Will the next move put the cube closer to the target point than it is now?
    dist = P_ApproxDistance(target->x - (actor->x + actor->momx), target->y - (actor->y + actor->momy));

    if ((unsigned int)dist < (unsigned int)actor->reactiontime)
    {
        actor->reactiontime = dist; // Yes. Still flying
        return;
    }

    if (!nomonsters && !massacre)
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

        newmobj->flags &= ~MF_COUNTKILL;

        // killough 8/29/98: add to appropriate thread
        P_UpdateThinker(&newmobj->thinker);

        if (!P_LookForPlayers(newmobj, true) || P_SetMobjState(newmobj, newmobj->info->seestate))
            // telefrag anything in this spot
            P_TeleportMove(newmobj, newmobj->x, newmobj->y, newmobj->z, true);

        if (newmobj->flags & MF_COUNTKILL)
        {
            totalkills++;
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

// killough 11/98: kill an object
void A_Die(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_DamageMobj(actor, NULL, NULL, actor->health, true);
}

//
// A_Detonate
// killough 8/9/98: same as A_Explode, except that the damage is variable
//
void A_Detonate(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_RadiusAttack(actor, actor->target, actor->info->damage, false);
}

//
// killough 9/98: a mushroom explosion effect, sorta :)
//
void A_Mushroom(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int     n = actor->info->damage;

    // Mushroom parameters are part of code pointer's state
    fixed_t misc1 = (actor->state->misc1 ? actor->state->misc1 : FRACUNIT * 4);
    fixed_t misc2 = (actor->state->misc2 ? actor->state->misc2 : FRACUNIT / 2);

    A_Explode(actor, NULL, NULL);                               // First make normal explosion

    // Now launch mushroom cloud
    for (int i = -n; i <= n; i += 8)
        for (int j = -n; j <= n; j += 8)
        {
            mobj_t  target = *actor;
            mobj_t  *mo;

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
void A_Spawn(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobjtype_t  type = (mobjtype_t)actor->state->misc1;

    if (type--)
        // If we're in massacre mode then don't spawn anything killable.
        if (!(actor->flags2 & MF2_MASSACRE) || !(mobjinfo[type].flags & MF_COUNTKILL))
        {
            mobj_t  *newmobj = P_SpawnMobj(actor->x, actor->y, (actor->state->misc2 << FRACBITS) + actor->z, type);

            if (newmobj->flags & MF_COUNTKILL)
            {
                totalkills++;
                monstercount[type]++;
            }
        }
}

void A_Turn(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    actor->angle += (unsigned int)(((uint64_t)actor->state->misc1 << 32) / 360);
}

void A_Face(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    actor->angle = (unsigned int)(((uint64_t)actor->state->misc1 << 32) / 360);
}

void A_Scratch(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *target = actor->target;

    if (!target)
        return;

    A_FaceTarget(actor, NULL, NULL);

    if (P_CheckMeleeRange(actor))
    {
        state_t *state = actor->state;

        if (state->misc2)
            S_StartSound(actor, state->misc2);

        P_DamageMobj(target, actor, actor, state->misc1, true);
    }
}

void A_PlaySound(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    state_t *state = actor->state;

    S_StartSound((state->misc2 ? NULL : actor), state->misc1);
}

void A_RandomJump(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    // [BH] allow A_RandomJump to work for weapon states as well
    if (psp)
    {
        state_t *state = psp->state;

        if (M_Random() < state->misc2)
            P_SetPsprite(psp - &player->psprites[ps_weapon], state->misc1);
    }
    else
    {
        state_t *state = actor->state;

        if (M_Random() < state->misc2)
            P_SetMobjState(actor, state->misc1);
    }
}

//
// This allows linedef effects to be activated inside deh frames.
//
void A_LineEffect(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    line_t      junk = *lines;
    player_t    newplayer;
    player_t    *oldplayer = actor->player;

    actor->player = &newplayer;
    newplayer.health = initial_health;

    if (!(junk.special = (short)actor->state->misc1))
        return;

    junk.tag = (short)actor->state->misc2;

    if (!P_UseSpecialLine(actor, &junk, 0))
        P_CrossSpecialLine(&junk, 0, actor);

    actor->state->misc1 = junk.special;
    actor->player = oldplayer;
}

