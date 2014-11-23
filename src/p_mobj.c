/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

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

#include "d_deh.h"
#include "d_main.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "m_config.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"

void G_PlayerReborn(int player);
void P_DelSeclist(msecnode_t *node);
void P_SpawnShadow(mobj_t *actor);

int                     bloodsplats = BLOODSPLATS_DEFAULT;
mobj_t                  *bloodSplatQueue[BLOODSPLATS_MAX];
int                     bloodSplatQueueSlot;
void                    (*P_BloodSplatSpawner)(fixed_t, fixed_t, int, int);

int                     corpses = CORPSES_DEFAULT;
boolean                 floatbob = FLOATBOB_DEFAULT;
boolean                 footclip = FOOTCLIP_DEFAULT;
boolean                 shadows = SHADOWS_DEFAULT;
int                     smoketrails = SMOKETRAILS_DEFAULT;

extern msecnode_t       *sector_list;   // phares 3/16/98
extern boolean          *isliquid;

//
//
// P_SetMobjState
// Returns true if the mobj is still present.
//
boolean P_SetMobjState(mobj_t *mobj, statenum_t state)
{
    state_t             *st;

    // killough 4/9/98: remember states seen, to detect cycles:
    static statenum_t   seenstate_tab[NUMSTATES];               // fast transition table
    statenum_t          *seenstate = seenstate_tab;             // pointer to table
    static int          recursion;                              // detects recursion
    statenum_t          i = state;                              // initial state
    boolean             ret = true;                             // return value
    statenum_t          tempstate[NUMSTATES];                   // for use with recursion
    boolean             hasshadow = ((mobj->flags2 & MF2_SHADOW) && mobj->shadow);

    if (recursion++)                                            // if recursion detected,
        memset((seenstate = tempstate), 0, sizeof(tempstate));  // clear state table

    do
    {
        if (state == S_NULL)
        {
            mobj->state = (state_t *)S_NULL;
            P_RemoveMobj(mobj);
            if (hasshadow)
                P_RemoveMobj(mobj->shadow);
            ret = false;
            break;                                              // killough 4/9/98
        }

        st = &states[state];
        mobj->state = st;
        mobj->tics = st->tics;
        mobj->sprite = st->sprite;
        mobj->frame = st->frame;

        // Modified handling.
        // Call action functions when the state is set
        if (st->action.acp1)
            st->action.acp1(mobj);

        seenstate[state] = 1 + st->nextstate;                   // killough 4/9/98

        state = st->nextstate;
    } while (!mobj->tics && !seenstate[state]);                 // killough 4/9/98

    if (!--recursion)
        for (; (state = seenstate[i]); i = state - 1)
            seenstate[i] = 0;                                   // killough 4/9/98: erase memory of states

    if (ret)
        if (hasshadow)
        {
            mobj->shadow->sprite = mobj->state->sprite;
            mobj->shadow->frame = mobj->frame & ~FF_FULLBRIGHT;
            mobj->shadow->angle = mobj->angle;
        }

    return ret;
}

//
// P_ExplodeMissile
//
void P_ExplodeMissile(mobj_t *mo)
{
    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState(mo, mo->info->deathstate);

    mo->tics = MAX(1, mo->tics - (P_Random() & 3));

    mo->flags &= ~MF_MISSILE;

    if (mo->type == MT_ROCKET)
    {
        mo->colfunc = tlcolfunc;
        if (mo->shadow)
            P_RemoveMobj(mo->shadow);
    }

    S_StartSound(mo, mo->info->deathsound);
}

//
// P_XYMovement
//
#define STOPSPEED       0x1000
#define FRICTION        0xe800
#define WATERFRICTION   0xfb00

int     puffcount = 0;

void P_XYMovement(mobj_t *mo)
{
    player_t    *player;
    fixed_t     xmove, ymove;
    mobjtype_t  type;
    fixed_t     fac;
    fixed_t     ptryx;
    fixed_t     ptryy;
    int         numsteps = 1;

    if (!(mo->momx | mo->momy))
    {
        if (mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->flags &= ~MF_SKULLFLY;
            mo->momz = 0;
            P_SetMobjState(mo, mo->info->spawnstate);
        }
        return;
    }

    player = mo->player;
    type = mo->type;

    if (mo->flags2 & MF2_SMOKETRAIL)
        if (puffcount++ > 1)
            P_SpawnSmokeTrail(mo->x, mo->y, mo->z, mo->angle);

    // preserve the direction instead of clamping x and y independently.
    xmove = BETWEEN(-MAXMOVE, mo->momx, MAXMOVE);
    ymove = BETWEEN(-MAXMOVE, mo->momy, MAXMOVE);

    fac = MIN(FixedDiv(xmove, mo->momx), FixedDiv(ymove, mo->momy));

    xmove = mo->momx = FixedMul(mo->momx, fac);
    ymove = mo->momy = FixedMul(mo->momy, fac);

    // [WDJ] 3/2011 Moved out of loop and converted to stepping.
    // Fixes mancubus fireballs which were too fast for collision tests,
    // makes steps equal in size, and makes loop test faster and predictable.
    // Boom bug had only the positive tests.
    if (xmove > MAXMOVE / 2 || ymove > MAXMOVE / 2
        || xmove < -MAXMOVE / 2 || ymove < -MAXMOVE / 2)
    {
        xmove >>= 1;
        ymove >>= 1;
        numsteps = 2;
    }

    if (mo->info->speed > mo->radius * 2)       // faster than radius * 2
    {
        // Mancubus missiles and the like.
        xmove >>= 1;
        ymove >>= 1;
        numsteps *= 2;
    }

    ptryx = mo->x;
    ptryy = mo->y;

    do
    {
        ptryx += xmove;
        ptryy += ymove;

        if (!P_TryMove(mo, ptryx, ptryy, true))
        {
            // blocked move
            if (player)
                // try to slide along it
                P_SlideMove(mo);
            else if (mo->flags & MF_MISSILE)
            {
                // explode a missile
                if (ceilingline
                    && ceilingline->backsector
                    && ceilingline->backsector->ceilingpic == skyflatnum
                    && mo->z > ceilingline->backsector->ceilingheight)
                {
                    // Hack to prevent missiles exploding
                    // against the sky.
                    // Does not handle sky floors.
                    P_RemoveMobj(mo);
                    if (mo->type == MT_BFG)
                        S_StartSound(mo, mo->info->deathsound);
                    else if (mo->type == MT_ROCKET && mo->shadow)
                        P_RemoveMobj(mo->shadow);
                    return;
                }
                P_ExplodeMissile(mo);
            }
            else
                mo->momx = mo->momy = 0;
        }
    } while (--numsteps);

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
        return;         // no friction for missiles or lost souls ever

    if (mo->z > mo->floorz && !(mo->flags2 & MF2_ONMOBJ))
        return;         // no friction when airborne

    if ((mo->flags & MF_CORPSE) && !(mo->flags & MF_NOBLOOD) && (corpses & SLIDE)
        && (corpses & SMEARBLOOD) && (mo->momx || mo->momy) && mo->bloodsplats && bloodsplats)
    {
        int     i;
        int     max = ((MAXMOVE - (ABS(mo->momx) + ABS(mo->momy)) / 2) >> FRACBITS) / 12;
        int     radius = (spritewidth[sprites[mo->sprite].spriteframes[0].lump[0]] >> FRACBITS) >> 1;
        int     blood = mobjinfo[mo->blood].blood;

        for (i = 0; i < max; i++)
        {
            if (!--mo->bloodsplats)
                break;

            P_BloodSplatSpawner(mo->x + (M_RandomInt(-radius, radius) << FRACBITS),
                mo->y + (M_RandomInt(-radius, radius) << FRACBITS), blood, mo->floorz);
        }
    }

    if (((mo->flags & MF_CORPSE) || (mo->flags2 & MF2_FALLING))
        && (mo->momx > FRACUNIT / 4 || mo->momx < -FRACUNIT / 4
            || mo->momy > FRACUNIT / 4 || mo->momy < -FRACUNIT / 4)
        && mo->floorz != mo->subsector->sector->floorheight)
        return;         // do not stop sliding if halfway off a step with some momentum

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED && mo->momy < STOPSPEED
        && (!player || (!player->cmd.forwardmove && !player->cmd.sidemove)))
    {
        // if in a walking frame, stop moving
        if (player && (unsigned int)((player->mo->state - states) - S_PLAY_RUN1) < 4)
            P_SetMobjState(player->mo, S_PLAY);

        mo->momx = mo->momy = 0;
    }
    else
    {
        if (((mo->flags & MF_CORPSE) || (mo->flags & MF_DROPPED))
            && isliquid[mo->subsector->sector->floorpic])
        {
            mo->momx = FixedMul(mo->momx, WATERFRICTION);
            mo->momy = FixedMul(mo->momy, WATERFRICTION);
        }
        else
        {
            mo->momx = FixedMul(mo->momx, FRICTION);
            mo->momy = FixedMul(mo->momy, FRICTION);
        }
    }
}

//
// P_ZMovement
//
void P_ZMovement(mobj_t *mo)
{
    player_t    *player = mo->player;

    // check for smooth step up
    if (player && mo->z < mo->floorz)
    {
        player->viewheight -= mo->floorz - mo->z;
        player->deltaviewheight = (VIEWHEIGHT - player->viewheight) >> 3;
    }

    // adjust height
    mo->z += mo->momz;

    if ((mo->flags & MF_FLOAT) && mo->target)
    {
        // float down towards target if too close
        if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
        {
            fixed_t     delta = (mo->target->z + (mo->height >> 1) - mo->z) * 3;

            if (P_ApproxDistance(mo->x - mo->target->x, mo->y - mo->target->y) < ABS(delta))
                mo->z += (delta < 0 ? -FLOATSPEED : FLOATSPEED);
        }
    }

    // clip movement
    if (mo->z <= mo->floorz)
    {
        // [BH] remove blood the moment it hits the ground
        //  and spawn a blood splat in its place
        if (mo->flags2 & MF2_BLOOD)
        {
            P_RemoveMobj(mo);
            if (bloodsplats)
                P_BloodSplatSpawner(mo->x, mo->y, mo->blood, mo->floorz);
            return;
        }

        // hit the floor
        if (mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;       // the skull slammed into something

        if (mo->momz < 0)
        {
            if (player && mo->momz < -GRAVITY * 8)
            {
                // Squat down.
                // Decrease viewheight for a moment
                // after hitting the ground (hard),
                // and utter appropriate sound.
                player->deltaviewheight = mo->momz >> 3;

                if (mo->health > 0)
                    S_StartSound(mo, sfx_oof);
            }
            mo->momz = 0;
        }
        mo->z = mo->floorz;

        if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
        {
            P_ExplodeMissile(mo);
            return;
        }
    }
    else if (!(mo->flags & MF_NOGRAVITY))
    {
        if (!mo->momz)
            mo->momz = -GRAVITY;
        mo->momz -= GRAVITY;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
        if (mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;       // the skull slammed into something

        // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;

        mo->z = mo->ceilingz - mo->height;

        if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
        {
            P_ExplodeMissile(mo);
            return;
        }
    }
}

//
// P_NightmareRespawn
//
void P_NightmareRespawn(mobj_t *mobj)
{
    fixed_t     x = mobj->spawnpoint.x << FRACBITS;
    fixed_t     y = mobj->spawnpoint.y << FRACBITS;
    fixed_t     z;
    subsector_t *ss;
    mobj_t      *mo;
    mapthing_t  *mthing = &mobj->spawnpoint;

    if (!x && !y)
    {
        x = mobj->x;
        y = mobj->y;
    }

    // something is occupying it's position?
    if (!P_CheckPosition(mobj, x, y))
        return;         // no respwan

    // spawn a teleport fog at old spot
    //  because of removal of the body?
    mo = P_SpawnMobj(mobj->x, mobj->y, mobj->subsector->sector->floorheight, MT_TFOG);
    mo->angle = mobj->angle;

    // initiate teleport sound
    S_StartSound(mo, sfx_telept);

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector(x, y);

    mo = P_SpawnMobj(x, y, ss->sector->floorheight, MT_TFOG);
    mo->angle = ANG45 * (mthing->angle / 45);

    S_StartSound(mo, sfx_telept);

    // spawn the new monster
    z = ((mobj->info->flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ);

    // inherit attributes from deceased one
    mo = P_SpawnMobj(x, y, z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;
    mo->angle = ANG45 * (mthing->angle / 45);

    mo->flags &= ~MF_COUNTKILL;

    if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;

    mo->reactiontime = 18;

    // remove the old monster
    P_RemoveMobj(mobj);
    if (mobj->shadow)
        P_RemoveMobj(mobj->shadow);
}

static void PlayerLandedOnThing(mobj_t *mo)
{
    mo->player->deltaviewheight = mo->momz >> 3;
    if (mo->momz < -23 * FRACUNIT)
        P_NoiseAlert(mo, mo);
}

fixed_t floatbobdiffs[64] =
{
     25695,  25695,  25447,  24955,  24222,  23256,  22066,  20663,
     19062,  17277,  15325,  13226,  10999,   8667,   6251,   3775,
      1262,  -1262,  -3775,  -6251,  -8667, -10999, -13226, -15325,
    -17277, -19062, -20663, -22066, -23256, -24222, -24955, -25447,
    -25695, -25695, -25447, -24955, -24222, -23256, -22066, -20663,
    -19062, -17277, -15325, -13226, -11000,  -8667,  -6251,  -3775,
     -1262,   1262,   3775,   6251,   8667,  10999,  13226,  15325,
     17276,  19062,  20663,  22066,  23256,  24222,  24955,  25448
};

//
// P_MobjThinker
//
void P_MobjThinker(mobj_t *mobj)
{
    // momentum movement
    if (mobj->momx || mobj->momy || (mobj->flags & MF_SKULLFLY))
    {
        P_XYMovement(mobj);

        if (mobj->thinker.function.acv == (actionf_v)(-1))
            return;             // mobj was removed
    }

    if ((mobj->flags2 & MF2_FLOATBOB) && floatbob)
        mobj->z += floatbobdiffs[(mobj->floatbob + leveltime) & 63];
    else if (mobj->z != mobj->floorz || mobj->momz)
    {
        if (mobj->flags2 & MF2_PASSMOBJ)
        {
            mobj_t *onmo;

            if (!(onmo = P_CheckOnmobj(mobj)))
            {
                P_ZMovement(mobj);
                mobj->flags2 &= ~MF2_ONMOBJ;
            }
            else
            {
                if (mobj->player)
                {
                    if (mobj->momz < -GRAVITY * 8)
                        PlayerLandedOnThing(mobj);
                    if (onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
                    {
                        mobj->player->viewheight -= onmo->z + onmo->height - mobj->z;
                        mobj->player->deltaviewheight = (VIEWHEIGHT - mobj->player->viewheight) >> 3;
                        mobj->z = onmo->z + onmo->height;
                        mobj->flags2 |= MF2_ONMOBJ;
                    }
                    mobj->momz = 0;
                }
            }
        }
        else
            P_ZMovement(mobj);

        if (mobj->thinker.function.acv == (actionf_v)(-1))
            return;             // mobj was removed
    }
    else if (!mobj->momx && !mobj->momy && !mobj->player)
    {
        // killough 9/12/98: objects fall off ledges if they are hanging off
        // slightly push off of ledge if hanging more than halfway off
        // [RH] Be more restrictive to avoid pushing monsters/players down steps
        if (!(mobj->flags & MF_NOGRAVITY) && !(mobj->flags2 & MF2_FLOATBOB)
            && mobj->z > mobj->dropoffz && (mobj->health <= 0 || ((mobj->flags & MF_COUNTKILL)
            && mobj->z - mobj->dropoffz > 24 * FRACUNIT)))
            P_ApplyTorque(mobj);
        else
        {
            // Reset torque
            mobj->flags2 &= ~MF2_FALLING;
            mobj->gear = 0;
        }
    }

    // cycle through states,
    //  calling action functions at transitions
    if (mobj->tics != -1)
    {
        // you can cycle through multiple states in a tic
        if (!--mobj->tics)
            P_SetMobjState(mobj, mobj->state->nextstate);
    }
    else
    {
        // check for nightmare respawn
        if ((mobj->flags & MF_SHOOTABLE) && respawnmonsters)
        {
            mobj->movecount++;

            if (mobj->movecount >= 12 * TICRATE && !(leveltime & 31) && P_Random() <= 4)
                P_NightmareRespawn(mobj);
        }
    }
}

//
// P_NullMobjThinker
//
void P_NullMobjThinker(mobj_t *mobj)
{
}

//
// P_SpawnMobj
//
mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
    mobj_t      *mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
    state_t     *st;
    mobjinfo_t  *info = &mobjinfo[type];

    memset(mobj, 0, sizeof(*mobj));

    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->projectilepassheight = info->projectilepassheight;
    mobj->flags = info->flags;
    mobj->flags2 = info->flags2;

    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    mobj->lastlook = P_Random() % MAXPLAYERS;

    // do not set the state with P_SetMobjState,
    // because action routines cannot be called yet
    st = &states[info->spawnstate];

    if (info->frames > 1)
    {
        int     frames = M_RandomInt(0, info->frames);
        int     i = 0;

        while (i++ < frames && st->nextstate != S_NULL)
            st = &states[st->nextstate];
    }

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    mobj->colfunc = info->colfunc;
    mobj->blood = info->blood;

    // set subsector and/or block links
    P_SetThingPosition(mobj);

    mobj->dropoffz =           // killough 11/98: for tracking dropoffs
    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    if ((mobj->flags2 & MF2_FLOATBOB) && floatbob)
        mobj->floatbob = P_Random();

    mobj->z = (z == ONFLOORZ ? mobj->floorz : 
              (z == ONCEILINGZ ? mobj->ceilingz - mobj->height : z));

    mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
    P_AddThinker(&mobj->thinker);

    if (mobj->flags2 & MF2_SHADOW)
        P_SpawnShadow(mobj);

    if (footclip)
        if (isliquid[mobj->subsector->sector->floorpic])
        {
            if (!(mobj->flags2 & MF2_NOFOOTCLIP))
                mobj->flags2 |= MF2_FEETARECLIPPED;
            if ((mobj->flags2 & MF2_SHADOW) && mobj->shadow)
                mobj->shadow->flags2 |= MF2_FEETARECLIPPED;
        }

    return mobj;
}

//
// P_RemoveMobj
//
static mapthing_t       itemrespawnqueue[ITEMQUEUESIZE];
static int              itemrespawntime[ITEMQUEUESIZE];
int                     iqueuehead;
int                     iqueuetail;

void P_RemoveMobj(mobj_t *mobj)
{
    if ((mobj->flags & MF_SPECIAL) && !(mobj->flags & MF_DROPPED)
        && mobj->type != MT_INV && mobj->type != MT_INS)
    {
        itemrespawnqueue[iqueuehead] = mobj->spawnpoint;
        itemrespawntime[iqueuehead] = leveltime;
        iqueuehead = (iqueuehead + 1) & (ITEMQUEUESIZE - 1);

        // lose one off the end?
        if (iqueuehead == iqueuetail)
            iqueuetail = (iqueuetail + 1) & (ITEMQUEUESIZE - 1);
    }

    // unlink from sector and block lists
    P_UnsetThingPosition(mobj);

    // Delete all nodes on the current sector_list
    if (sector_list)
    {
        P_DelSeclist(sector_list);
        sector_list = NULL;
    }
    
    mobj->flags |= (MF_NOSECTOR | MF_NOBLOCKMAP);

    mobj->target = mobj->tracer = mobj->lastenemy = NULL;

    // free block
    P_RemoveThinker((thinker_t *)mobj);
}

//
// P_FindDoomedNum
//
// Finds a mobj type with a matching doomednum
// killough 8/24/98: rewrote to use hashing
//
static int P_FindDoomedNum(unsigned int type)
{
    static struct
    {
        int     first;
        int     next;
    } *hash;

    int i;

    if (!hash)
    {
        hash = Z_Malloc(sizeof(*hash) * NUMMOBJTYPES, PU_CACHE, (void **)&hash);
        for (i = 0; i < NUMMOBJTYPES; i++)
            hash[i].first = NUMMOBJTYPES;
        for (i = 0; i < NUMMOBJTYPES; i++)
            if (mobjinfo[i].doomednum != -1)
            {
                unsigned int    h = (unsigned int)mobjinfo[i].doomednum % NUMMOBJTYPES;

                hash[i].next = hash[h].first;
                hash[h].first = i;
            }
    }

    i = hash[type % NUMMOBJTYPES].first;
    while ((i < NUMMOBJTYPES) && ((unsigned int)mobjinfo[i].doomednum != type))
        i = hash[i].next;
    return i;
}

//
// P_RespawnSpecials
//
void P_RespawnSpecials(void)
{
    fixed_t     x, y, z;
    subsector_t *ss;
    mobj_t      *mo;
    mapthing_t  *mthing;
    int         i;

    // only respawn items in deathmatch
    if (deathmatch != 2)
        return;

    // nothing left to respawn?
    if (iqueuehead == iqueuetail)
        return;

    // wait at least 30 seconds
    if (leveltime - itemrespawntime[iqueuetail] < 30 * TICRATE)
        return;

    mthing = &itemrespawnqueue[iqueuetail];

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector(x, y);
    mo = P_SpawnMobj(x, y, ss->sector->floorheight, MT_IFOG);
    mo->angle = mthing->angle;
    S_StartSound(mo, sfx_itmbk);

    // killough 8/23/98: use table for faster lookup
    i = P_FindDoomedNum(mthing->type);

    // spawn it
    z = ((mobjinfo[i].flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ);

    mo = P_SpawnMobj(x, y, z, (mobjtype_t)P_FindDoomedNum(mthing->type));
    mo->spawnpoint = *mthing;
    mo->angle = ANG45 * (mthing->angle / 45);

    // pull it from the queue
    iqueuetail = (iqueuetail + 1) & (ITEMQUEUESIZE - 1);
}

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
extern int lastlevel;
extern int lastepisode;

void P_SpawnPlayer(int n, const mapthing_t *mthing)
{
    player_t    *p;
    fixed_t     x, y, z;
    mobj_t      *mobj;

    // not playing?
    if (!playeringame[n])
        return;

    p = &players[n];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn(n);

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ONFLOORZ;
    mobj = P_SpawnMobj(x, y, z, MT_PLAYER);

    mobj->angle = (mthing->angle % 45 ? mthing->angle * (ANG45 / 45) : 
                                        ANG45 * (mthing->angle / 45));
    mobj->player = p;
    mobj->health = p->health;

    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = VIEWHEIGHT;

    p->viewz = p->mo->z + p->viewheight;
    p->psprites[ps_weapon].sx = 0;
    p->mo->momx = p->mo->momy = 0;

    // setup gun psprite
    P_SetupPsprites(p);

    // give all cards in deathmatch mode
    if (deathmatch)
    {
        int     i;

        for (i = 0; i < NUMCARDS; i++)
            p->cards[i] = true;
    }

    lastlevel = -1;
    lastepisode = -1;

    if (n == consoleplayer)
    {
        ST_Start();     // wake up the status bar
        HU_Start();     // wake up the heads up text
    }
}

void P_SpawnMoreBlood(mobj_t *mobj)
{
    int     radius = ((spritewidth[sprites[mobj->sprite].spriteframes[0].lump[0]] >> FRACBITS) >> 1) + 8;
    int     i;
    int     max = M_RandomInt(100, 150);
    int     blood = mobjinfo[mobj->blood].blood;

    for (i = 0; i < max; i++)
        P_BloodSplatSpawner(mobj->x + (M_RandomInt(-radius, radius) << FRACBITS),
            mobj->y + (M_RandomInt(-radius, radius) << FRACBITS), blood, mobj->floorz);
}

//
// P_SpawnMapThing
// The fields of the mapthing should
//  already be in host byte order.
//
void P_SpawnMapThing(mapthing_t *mthing)
{
    int         i;
    int         bit;
    mobj_t      *mobj;
    fixed_t     x, y, z;
    short       type = mthing->type;

    // count deathmatch start positions
    if (type == PlayerDeathmatchStart)
    {
        if (deathmatch_p < &deathmatchstarts[10])
        {
            memcpy(deathmatch_p, mthing, sizeof(*mthing));
            deathmatch_p++;
        }
        return;
    }

    // check for players specially
    if (type >= Player1Start && type <= Player4Start)
    {
        // save spots for respawning in network games
        playerstarts[type - 1] = *mthing;
        if (!deathmatch)
            P_SpawnPlayer(type - 1, &playerstarts[type - 1]);

        return;
    }

    // check for appropriate skill level
    if (mthing->options & 16)
        return;

    if (gameskill == sk_baby)
        bit = 1;
    else if (gameskill == sk_nightmare)
        bit = 4;
    else
        bit = 1 << (gameskill - 1);

    if (!(mthing->options & bit))
        return;

    // find which type to spawn

    // killough 8/23/98: use table for faster lookup
    i = P_FindDoomedNum(type);

    if (i == NUMMOBJTYPES)
        return;

    // don't spawn keycards and players in deathmatch
    if (deathmatch && (mobjinfo[i].flags & MF_NOTDMATCH))
        return;

    // don't spawn any monsters if -nomonsters
    if (nomonsters && (mobjinfo[i].flags & MF_COUNTKILL) && i != MT_KEEN)
        return;

    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ((mobjinfo[i].flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ);

    mobj = P_SpawnMobj(x, y, z, (mobjtype_t)i);
    mobj->spawnpoint = *mthing;

    if (mobj->tics > 0)
        mobj->tics = 1 + (P_Random() % mobj->tics);

    if (mobj->flags & MF_COUNTKILL)
        totalkills++;

    if (mobj->flags & MF_COUNTITEM)
        totalitems++;

    mobj->angle = (mthing->angle % 45 ? mthing->angle * (ANG45 / 45) : 
                                        ANG45 * (mthing->angle / 45));
    if ((mobj->flags2 & MF2_SHADOW) && mobj->shadow)
        mobj->shadow->angle = mobj->angle;

    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;

    if (mobj->type == MF_CORPSE && (corpses & MIRROR))
    {
        static int      prev = 0;
        int             r = M_RandomInt(1, 10);

        if (r <= 5 + prev)
        {
            prev--;
            mobj->flags2 |= MF2_MIRRORED;
            if (mobj->shadow)
                mobj->shadow->flags2 |= MF2_MIRRORED;
        }
        else
            prev++;
    }

    if (!(mobj->flags & MF_SHOOTABLE) && !(mobj->flags & MF_NOBLOOD) && mobj->blood && !chex
        && (corpses & MOREBLOOD) && bloodsplats && !dehacked)
    {
        mobj->bloodsplats = CORPSEBLOODSPLATS;
        P_SpawnMoreBlood(mobj);
    }
}

//
// GAME SPAWN FUNCTIONS
//

//
// P_SpawnPuff
//
extern fixed_t  attackrange;
extern angle_t  shootangle;

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z, angle_t angle, boolean sound)
{
    mobj_t      *th = P_SpawnMobj(x, y, z + ((P_Random() - P_Random()) << 10), MT_PUFF);

    th->momz = FRACUNIT;
    th->tics = MAX(1, th->tics - (P_Random() & 3));

    th->angle = angle;

    th->flags2 |= (rand() & 1) * MF2_MIRRORED;

    // don't make punches spark on the wall
    if (attackrange == MELEERANGE)
    {
        P_SetMobjState(th, S_PUFF3);
        if (sound)
        {
            S_StartSound(th, sfx_punch);

            if (gamepadvibrate && vibrate)
            {
                int motorspeed = weaponinfo[wp_fist].motorspeed;

                if (players[consoleplayer].powers[pw_strength])
                    motorspeed *= 2;
                XInputVibration(motorspeed);
                weaponvibrationtics = weaponinfo[wp_fist].tics;
            }
        }
    }
}

void P_SpawnSmokeTrail(fixed_t x, fixed_t y, fixed_t z, angle_t angle)
{
    mobj_t      *th = P_SpawnMobj(x, y, z + ((P_Random() - P_Random()) << 10), MT_TRAIL);

    th->momz = FRACUNIT / 2;
    th->tics -= (P_Random() & 3);

    th->angle = angle;

    th->flags2 |= (rand() & 1) * MF2_MIRRORED;
}

//
// P_SpawnBlood
//
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, angle_t angle, int damage, mobj_t *target)
{
    int         i;
    int         minz = target->z;
    int         maxz = minz + spriteheight[sprites[target->sprite].spriteframes[0].lump[0]];
    int         blood = target->blood;
    mobjinfo_t  *info = &mobjinfo[blood];

    angle += ANG180;

    for (i = MAX(P_Random() & 10, damage >> 2); i; i--)
    {
        mobj_t      *th = Z_Malloc(sizeof(*th), PU_LEVEL, NULL);
        state_t     *st;

        memset(th, 0, sizeof(*th));

        th->type = blood;
        th->info = info;
        th->x = x;
        th->y = y;
        th->flags = info->flags;
        th->flags2 = (info->flags2 | (rand() & 1) * MF2_MIRRORED);

        st = &states[info->spawnstate];

        th->state = st;
        th->tics = MAX(1, st->tics - (P_Random() & 6));
        th->sprite = st->sprite;
        th->frame = st->frame;

        th->colfunc = info->colfunc;
        th->blood = info->blood;

        P_SetThingPosition(th);

        th->dropoffz = th->floorz = th->subsector->sector->floorheight;
        th->ceilingz = th->subsector->sector->ceilingheight;

        th->z = BETWEEN(minz, z + ((P_Random() - P_Random()) << 10), maxz);

        th->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
        P_AddThinker(&th->thinker);

        th->momx = FixedMul(i * FRACUNIT / 4, finecosine[angle >> ANGLETOFINESHIFT]);
        th->momy = FixedMul(i * FRACUNIT / 4, finesine[angle >> ANGLETOFINESHIFT]);
        th->momz = FRACUNIT * (2 + i / 6);

        th->angle = angle;
        angle += ((P_Random() - P_Random()) * 0xb60b60);

        if (damage <= 12 && th->state->nextstate)
            P_SetMobjState(th, th->state->nextstate);
        if (damage < 9 && th->state->nextstate)
            P_SetMobjState(th, th->state->nextstate);
    }
}

extern boolean *isliquid;

//
// P_SpawnBloodSplat
//
void P_SpawnBloodSplat(fixed_t x, fixed_t y, int blood, int maxheight)
{
    subsector_t *ss;
    fixed_t     height;

    x += ((rand() % 16 - 5) << FRACBITS);
    y += ((rand() % 16 - 5) << FRACBITS);

    ss = R_PointInSubsector(x, y);
    height = ss->sector->floorheight;

    if (!isliquid[ss->sector->floorpic] && height <= maxheight)
    {
        mobj_t  *newsplat = (mobj_t *)Z_Malloc(sizeof(*newsplat), PU_LEVEL, NULL);

        memset(newsplat, 0, sizeof(*newsplat));

        newsplat->type = MT_BLOODSPLAT;
        newsplat->state = &states[S_BLOODSPLAT];
        newsplat->sprite = SPR_BLD2;
        newsplat->frame = rand() & 7;

        newsplat->flags2 = (MF2_DRAWFIRST | MF2_DONOTMAP | (rand() & 1) * MF2_MIRRORED);
        if (blood == FUZZYBLOOD)
        {
            newsplat->flags = MF_FUZZ;
            newsplat->colfunc = fuzzcolfunc;
        }
        else
            newsplat->colfunc = bloodsplatcolfunc;
        newsplat->blood = blood;

        newsplat->x = x;
        newsplat->y = y;
        P_SetThingPosition(newsplat);
        newsplat->z = height;

        newsplat->thinker.function.acp1 = (actionf_p1)P_NullMobjThinker;
        P_AddThinker(&newsplat->thinker);
    }
}

void P_SpawnBloodSplat2(fixed_t x, fixed_t y, int blood, int maxheight)
{
    subsector_t *ss;
    fixed_t     height;

    x += ((rand() % 16 - 5) << FRACBITS);
    y += ((rand() % 16 - 5) << FRACBITS);

    ss = R_PointInSubsector(x, y);
    height = ss->sector->floorheight;

    if (!isliquid[ss->sector->floorpic] && height <= maxheight)
    {
        mobj_t  *newsplat = (mobj_t *)Z_Malloc(sizeof(*newsplat), PU_LEVEL, NULL);

        memset(newsplat, 0, sizeof(*newsplat));

        newsplat->type = MT_BLOODSPLAT;
        newsplat->state = &states[S_BLOODSPLAT];
        newsplat->sprite = SPR_BLD2;
        newsplat->frame = rand() & 7;

        newsplat->flags2 = (MF2_DRAWFIRST | MF2_DONOTMAP | (rand() & 1) * MF2_MIRRORED);
        if (blood == FUZZYBLOOD)
        {
            newsplat->flags = MF_FUZZ;
            newsplat->colfunc = fuzzcolfunc;
        }
        else
            newsplat->colfunc = bloodsplatcolfunc;
        newsplat->blood = blood;

        newsplat->x = x;
        newsplat->y = y;
        P_SetThingPosition(newsplat);
        newsplat->z = height;

        newsplat->thinker.function.acp1 = (actionf_p1)P_NullMobjThinker;
        P_AddThinker(&newsplat->thinker);

        if (bloodSplatQueueSlot > bloodsplats)
        {
            mobj_t *oldsplat = bloodSplatQueue[bloodSplatQueueSlot % bloodsplats];

            if (oldsplat)
            {
                P_UnsetThingPosition(oldsplat);
                ((thinker_t *)oldsplat)->function.acv = (actionf_v)(-1);
            }
        }

        bloodSplatQueue[bloodSplatQueueSlot++ % bloodsplats] = newsplat;
    }
}

void P_NullBloodSplatSpawner(fixed_t x, fixed_t y, int blood, int maxheight)
{
}

//
// P_SpawnShadow
//
void P_SpawnShadow(mobj_t *actor)
{
    mobj_t      *mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);

    memset(mobj, 0, sizeof(*mobj));

    mobj->type = MT_SHADOW;
    mobj->info = &mobjinfo[MT_SHADOW];
    mobj->x = actor->x;
    mobj->y = actor->y;

    mobj->sprite = actor->state->sprite;
    mobj->frame = actor->state->frame;

    mobj->flags2 = MF2_DONOTMAP;

    mobj->colfunc = (actor->type == MT_SHADOWS ? R_DrawSpectreShadowColumn : R_DrawShadowColumn);

    P_SetThingPosition(mobj);

    mobj->z = mobj->subsector->sector->floorheight;

    mobj->thinker.function.acp1 = (actionf_p1)P_NullMobjThinker;
    P_AddThinker(&mobj->thinker);

    actor->shadow = mobj;
    mobj->shadow = actor;
}

//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
void P_CheckMissileSpawn(mobj_t *th)
{
    th->tics = MAX(1, th->tics - (P_Random() & 3));

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx >> 1);
    th->y += (th->momy >> 1);
    th->z += (th->momz >> 1);

    if (!P_TryMove(th, th->x, th->y, false))
        P_ExplodeMissile(th);
}

//
// P_SpawnMissile
//
mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type)
{
    fixed_t     z;
    mobj_t      *th;
    angle_t     an;
    int         dist;

    if (!dest)
    {
        dest->x = 0;
        dest->y = 0;
        dest->z = 0;
        dest->flags = 0;
    }

    z = source->z + 4 * 8 * FRACUNIT;
    if (source->flags2 & MF2_FEETARECLIPPED)
        z -= FOOTCLIPSIZE;

    th = P_SpawnMobj(source->x, source->y, z, type);

    if (th->info->seesound)
        S_StartSound(th, th->info->seesound);

    th->target = source;        // where it came from
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);

    // fuzzy player
    if (dest->flags & MF_FUZZ)
        an += (P_Random() - P_Random()) << 20;

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);

    dist = MAX(1, P_ApproxDistance(dest->x - source->x, dest->y - source->y) / th->info->speed);

    th->momz = (dest->z - source->z) / dist;
    P_CheckMissileSpawn(th);

    return th;
}

//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster.
//
void P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type)
{
    mobj_t      *th;
    angle_t     an;
    fixed_t     x, y, z;
    fixed_t     slope;

    // see which target is to be aimed at
    an = source->angle;
    slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);

    if (!linetarget)
    {
        an += 1 << 26;
        slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);

        if (!linetarget)
        {
            an -= 2 << 26;
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
        }

        if (!linetarget)
        {
            an = source->angle;
            slope = 0;
        }
    }

    x = source->x;
    y = source->y;
    z = source->z + 4 * 8 * FRACUNIT;
    if (source->flags2 & MF2_FEETARECLIPPED)
        z -= FOOTCLIPSIZE;

    th = P_SpawnMobj(x, y, z, type);

    if (th->info->seesound)
        S_StartSound(th, th->info->seesound);

    th->target = source;
    th->angle = an;
    th->momx = FixedMul(th->info->speed, finecosine[an >> ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->info->speed, finesine[an >> ANGLETOFINESHIFT]);
    th->momz = FixedMul(th->info->speed, slope);

    if (type == MT_ROCKET && (smoketrails & PLAYER) && !dehacked)
    {
        th->flags2 |= MF2_SMOKETRAIL;
        puffcount = 0;
    }

    P_CheckMissileSpawn(th);
}
