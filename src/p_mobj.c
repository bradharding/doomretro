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

#include "d_main.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"

void G_PlayerReborn(int player);
void P_DelSeclist(msecnode_t *node);

int             bloodsplats = BLOODSPLATS_DEFAULT;
mobj_t          *bloodSplatQueue[BLOODSPLATS_MAX];
int             bloodSplatQueueSlot;
void            (*P_BloodSplatSpawner)(fixed_t, fixed_t, int, void (*)(void));

boolean         smoketrails = true;

int             corpses = (SLIDE | SMEARBLOOD | MOREBLOOD);

extern msecnode_t *sector_list; // phares 3/16/98

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
boolean P_SetMobjState(mobj_t *mobj, statenum_t state)
{
    state_t     *st;

    do
    {
        if (state == S_NULL)
        {
            mobj->state = (state_t *)S_NULL;
            P_RemoveMobj(mobj);
            return false;
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

        state = st->nextstate;
    } while (!mobj->tics);

    return true;
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
        mo->colfunc = tlcolfunc;

    S_StartSound(mo, mo->info->deathsound);
}

//
// P_XYMovement
//
#define STOPSPEED       0x1000
#define FRICTION        0xe800

boolean shootingsky = false;
int     puffcount = 0;

void P_XYMovement(mobj_t *mo)
{
    player_t    *player;
    fixed_t     xmove, ymove;
    mobjtype_t  type;

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

    if (type == MT_ROCKET && smoketrails)
        if (puffcount++ > 1)
            P_SpawnPuff(mo->x, mo->y, mo->z, mo->angle, false);

    xmove = mo->momx = BETWEEN(-MAXMOVE, mo->momx, MAXMOVE);
    ymove = mo->momy = BETWEEN(-MAXMOVE, mo->momy, MAXMOVE);

    do
    {
        fixed_t ptryx;
        fixed_t ptryy;

        if (xmove > MAXMOVE / 2 || ymove > MAXMOVE / 2 ||
            xmove < -MAXMOVE / 2 || ymove < -MAXMOVE / 2)
        {
            ptryx = mo->x + xmove / 2;
            ptryy = mo->y + ymove / 2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }

        if (!P_TryMove(mo, ptryx, ptryy, true))
        {
            // blocked move
            if (mo->player)
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
                    shootingsky = true;
                    P_RemoveMobj(mo);
                    return;
                }
                P_ExplodeMissile(mo);
            }
            else
                xmove = ymove = mo->momx = mo->momy = 0;
        }
    }
    while (xmove || ymove);

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
        return;         // no friction for missiles or lost souls ever

    if (mo->z > mo->floorz && !(mo->flags2 & MF2_ONMOBJ))
        return;         // no friction when airborne

    if ((mo->flags & MF_CORPSE) && (corpses & SLIDE) && (corpses & SMEARBLOOD) &&
        (mo->momx || mo->momy) && mo->bloodsplats && bloodsplats)
    {
        int     i;
        int     flags2 = MF2_TRANSLUCENT_50;
        void    (*colfunc)(void) = tl50colfunc;
        int     radius = (spritewidth[sprites[mo->sprite].spriteframes[0].lump[0]] >> FRACBITS) >> 1;

        if (!FREEDOOM)
        {
            if (type == MT_HEAD || type == MT_MISC61)
            {
                flags2 = MF2_TRANSLUCENT_REDTOBLUE_33;
                colfunc = tlredtoblue33colfunc;
            }
            else if (type == MT_BRUISER || type == MT_KNIGHT)
            {
                flags2 = MF2_TRANSLUCENT_REDTOGREEN_33;
                colfunc = tlredtogreen33colfunc;
            }
        }

        for (i = 0; i < ((MAXMOVE - (ABS(mo->momx) + ABS(mo->momy)) / 2) >> FRACBITS) / 12; i++)
        {
            if (!--mo->bloodsplats)
                break;

            P_BloodSplatSpawner(mo->x + (M_RandomInt(-radius, radius) << FRACBITS),
                mo->y + (M_RandomInt(-radius, radius) << FRACBITS), flags2, colfunc);
        }
    }

    if (((mo->flags & MF_CORPSE) || (mo->flags2 & MF2_FALLING))
        && (mo->momx > FRACUNIT / 4 || mo->momx < -FRACUNIT / 4
            || mo->momy > FRACUNIT / 4 || mo->momy < -FRACUNIT / 4)
        && mo->z != mo->floorz)
        return;         // do not stop sliding if halfway off a step with some momentum

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED && mo->momy < STOPSPEED
        && (!player || (!player->cmd.forwardmove && !player->cmd.sidemove)))
    {
        // if in a walking frame, stop moving
        if (player && (unsigned)((player->mo->state - states) - S_PLAY_RUN1) < 4)
            P_SetMobjState(player->mo, S_PLAY);

        mo->momx = mo->momy = 0;
    }
    else
    {
        mo->momx = FixedMul(mo->momx, FRICTION);
        mo->momy = FixedMul(mo->momy, FRICTION);
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
        if (mo->type == MT_BLOOD)
        {
            P_RemoveMobj(mo);
            if (bloodsplats)
                P_BloodSplatSpawner(mo->x, mo->y, mo->flags2, mo->colfunc);
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
        // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;

        if (mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;       // the skull slammed into something

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
    mapthing_t  *mthing;

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

    S_StartSound(mo, sfx_telept);

    // spawn the new monster
    mthing = &mobj->spawnpoint;
    z = ((mobj->info->flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ);

    // inherit attributes from deceased one
    mo = P_SpawnMobj(x, y, z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;
    mo->angle = ANG45 * (mthing->angle / 45);

    if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;

    mo->flags2 &= ~MF2_MIRRORED;

    mo->reactiontime = 18;

    // remove the old monster
    P_RemoveMobj(mobj);
}

static void PlayerLandedOnThing(mobj_t *mo, mobj_t *onmobj)
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

    if (mobj->flags2 & MF2_FLOATBOB)
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
                        PlayerLandedOnThing(mobj, onmo);
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
    else if (!(mobj->momx | mobj->momy) && !mobj->player)
    {
        if (mobj->z > mobj->dropoffz && !(mobj->flags & MF_NOGRAVITY) &&
            mobj->flags & (MF_CORPSE | MF_SHOOTABLE | MF_DROPPED))
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
        if ((mobj->flags & MF_COUNTKILL) && respawnmonsters)
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

    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    mobj->lastlook = P_Random() % MAXPLAYERS;

    // do not set the state with P_SetMobjState,
    // because action routines cannot be called yet
    if (info->frames > 1)
        st = &states[info->spawnstate + M_RandomInt(0, info->frames - 1)];
    else
        st = &states[info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // NULL head of sector list
    mobj->touching_sectorlist = NULL;

    if (type != MT_BLOOD)
    {
        int     lump = firstspritelump + sprites[mobj->sprite].spriteframes[0].lump[0];

        if (W_CheckMultipleLumps(lumpinfo[lump].name) == 1)
            mobj->flags2 = info->flags2;

        if (mobj->flags & MF_SHADOW)
            mobj->colfunc = fuzzcolfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT)
            mobj->colfunc = tlcolfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT_REDONLY)
            mobj->colfunc = tlredcolfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT_GREENONLY)
            mobj->colfunc = tlgreencolfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT_BLUEONLY)
            mobj->colfunc = tlbluecolfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT_33)
            mobj->colfunc = tl33colfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT_50)
            mobj->colfunc = tl50colfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT_REDWHITEONLY)
            mobj->colfunc = tlredwhitecolfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT_REDTOGREEN_33)
            mobj->colfunc = tlredtogreen33colfunc;
        else if (mobj->flags2 & MF2_TRANSLUCENT_REDTOBLUE_33)
            mobj->colfunc = tlredtoblue33colfunc;
        else if (mobj->flags2 & MF2_REDTOGREEN)
            mobj->colfunc = redtogreencolfunc;
        else if (mobj->flags2 & MF2_REDTOBLUE)
            mobj->colfunc = redtobluecolfunc;
        else
            mobj->colfunc = basecolfunc;

        if (bfgedition)
        {
            if (mobj->sprite == SPR_STIM)
                mobj->sprite = SPR_STIM_BFG;
            else if (mobj->sprite == SPR_MEDI)
                mobj->sprite = SPR_MEDI_BFG;
        }
    }

    // set subsector and/or block links
    P_SetThingPosition(mobj);

    mobj->dropoffz =           // killough 11/98: for tracking dropoffs
    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    if (mobj->flags2 & MF2_FLOATBOB)
        mobj->floatbob = P_Random();

    mobj->z = (z == ONFLOORZ ? mobj->floorz : 
              (z == ONCEILINGZ ? mobj->ceilingz - mobj->height : z));

    mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
    P_AddThinker(&mobj->thinker);

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
    if (mobj->old_sectorlist)
        P_DelSeclist(mobj->old_sectorlist);
    
    mobj->flags |= (MF_NOSECTOR | MF_NOBLOCKMAP);
    mobj->old_sectorlist = NULL;

    if (shootingsky)
    {
        if (mobj->type == MT_BFG)
            S_StartSound(mobj, mobj->info->deathsound);
        shootingsky = false;
    }

    mobj->target = mobj->tracer = NULL;

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
                unsigned int h = (unsigned int)mobjinfo[i].doomednum % NUMMOBJTYPES;

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

void P_SpawnPlayer(mapthing_t *mthing)
{
    player_t    *p;
    fixed_t     x, y, z;
    mobj_t      *mobj;

    // not playing?
    if (!playeringame[mthing->type - 1])
        return;

    p = &players[mthing->type - 1];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn(mthing->type - 1);

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ONFLOORZ;
    mobj = P_SpawnMobj(x, y, z, MT_PLAYER);

    // set color translations for player sprites
    if (mthing->type > 1)
        mobj->flags |= (mthing->type - 1) << MF_TRANSSHIFT;

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

    if (mthing->type - 1 == consoleplayer)
    {
        // wake up the status bar
        ST_Start();

        // wake up the heads up text
        HU_Start();
    }
}

void P_SpawnMoreBlood(mobj_t *mobj, int flags2, void (*colfunc)(void))
{
    int     radius = ((spritewidth[sprites[mobj->sprite].spriteframes[0].lump[0]] >> FRACBITS) >> 1) + 8;
    int     i;
    int     max = M_RandomInt(100, 150);

    for (i = 0; i < max; i++)
        P_BloodSplatSpawner(mobj->x + (M_RandomInt(-radius, radius) << FRACBITS),
            mobj->y + (M_RandomInt(-radius, radius) << FRACBITS), flags2, colfunc);
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
            P_SpawnPlayer(mthing);

        return;
    }

    // check for appropriate skill level
    if (!netgame && (mthing->options & 16))
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
    if (nomonsters && (i == MT_SKULL || (mobjinfo[i].flags & MF_COUNTKILL)) && i != MT_KEEN)
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
    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;

    if (mobjinfo[i].flags2 & (MF2_MOREREDBLOODSPLATS | MF2_MOREBLUEBLOODSPLATS))
    {
        mobj->bloodsplats = CORPSEBLOODSPLATS;

        if ((corpses & MOREBLOOD) && bloodsplats)
            if ((mobjinfo[i].flags2 & MF2_MOREREDBLOODSPLATS)
                || (FREEDOOM && (mobjinfo[i].flags2 & MF2_MOREBLUEBLOODSPLATS)))
                P_SpawnMoreBlood(mobj, MF2_TRANSLUCENT_50, tl50colfunc);
            else if (mobjinfo[i].flags2 & MF2_MOREBLUEBLOODSPLATS)
                P_SpawnMoreBlood(mobj, MF2_TRANSLUCENT_REDTOBLUE_33, tlredtoblue33colfunc);
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
    mobj_t *th = P_SpawnMobj(x, y, z + ((P_Random() - P_Random()) << 10), MT_PUFF);

    th->momz = FRACUNIT;
    th->tics = MAX(1, th->tics - (P_Random() & 3));

    th->angle = angle;

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

//
// P_SpawnBlood
//
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, angle_t angle, int damage, mobj_t *target)
{
    int         i;
    int         flags2 = MF2_TRANSLUCENT_50;
    int         type = target->type;
    int         minz = target->z;
    int         maxz = minz + spriteheight[sprites[target->sprite].spriteframes[0].lump[0]];
    void        (*colfunc)(void) = tl50colfunc;

    if (!FREEDOOM)
    {
        if (type == MT_HEAD)
        {
            flags2 = MF2_TRANSLUCENT_REDTOBLUE_33;
            colfunc = tlredtoblue33colfunc;
        }
        else if (target->flags & MF_SHADOW)
        {
            flags2 = MF2_FUZZ;
            colfunc = fuzzcolfunc;
        }
        else if (type == MT_BRUISER || type == MT_KNIGHT)
        {
            flags2 = MF2_TRANSLUCENT_REDTOGREEN_33;
            colfunc = tlredtogreen33colfunc;
        }
    }

    angle += ANG180;

    for (i = MAX(P_Random() & 10, damage >> 2); i; i--)
    {
        mobj_t      *th;

        z = BETWEEN(minz, z + ((P_Random() - P_Random()) << 10), maxz);

        th = P_SpawnMobj(x, y, z, MT_BLOOD);

        th->tics = MAX(1, th->tics - (P_Random() & 6));

        th->momx = FixedMul(i * FRACUNIT / 4, finecosine[angle >> ANGLETOFINESHIFT]);
        th->momy = FixedMul(i * FRACUNIT / 4, finesine[angle >> ANGLETOFINESHIFT]);
        th->momz = FRACUNIT * (2 + i / 6);

        th->angle = angle;
        angle += ((P_Random() - P_Random()) * 0xb60b60);

        th->flags2 = flags2 | (rand() & 1) * MF2_MIRRORED;

        th->colfunc = colfunc;

        if (damage <= 12)
            P_SetMobjState(th, th->state->nextstate);
        if (damage < 9)
            P_SetMobjState(th, th->state->nextstate);
    }
}

extern boolean *isliquid;

//
// P_SpawnBloodSplat
//
void P_SpawnBloodSplat(fixed_t x, fixed_t y, int flags2, void (*colfunc)(void))
{
    subsector_t *ss;

    x += ((rand() % 16 - 5) << FRACBITS);
    y += ((rand() % 16 - 5) << FRACBITS);

    ss = R_PointInSubsector(x, y);

    if (!isliquid[ss->sector->floorpic])
    {
        mobj_t  *newsplat = (mobj_t *)Z_Malloc(sizeof(*newsplat), PU_LEVEL, NULL);

        memset(newsplat, 0, sizeof(*newsplat));

        newsplat->type = MT_BLOODSPLAT;
        newsplat->state = &states[S_BLOODSPLAT];
        newsplat->sprite = SPR_BLD2;
        newsplat->frame = rand() & 7;

        newsplat->flags2 = flags2 | (rand() & 1) * MF2_MIRRORED;
        newsplat->colfunc = colfunc;

        newsplat->x = x;
        newsplat->y = y;
        P_SetThingPosition(newsplat);
        newsplat->z = ss->sector->floorheight;

        newsplat->thinker.function.acp1 = (actionf_p1)P_NullMobjThinker;
        P_AddThinker(&newsplat->thinker);
    }
}

void P_SpawnBloodSplat2(fixed_t x, fixed_t y, int flags2, void (*colfunc)(void))
{
    subsector_t *ss;

    x += ((rand() % 16 - 5) << FRACBITS);
    y += ((rand() % 16 - 5) << FRACBITS);

    ss = R_PointInSubsector(x, y);

    if (!isliquid[ss->sector->floorpic])
    {
        mobj_t  *newsplat = (mobj_t *)Z_Malloc(sizeof(*newsplat), PU_LEVEL, NULL);

        memset(newsplat, 0, sizeof(*newsplat));

        newsplat->type = MT_BLOODSPLAT;
        newsplat->state = &states[S_BLOODSPLAT];
        newsplat->sprite = SPR_BLD2;
        newsplat->frame = rand() & 7;

        newsplat->flags2 = flags2 | (rand() & 1) * MF2_MIRRORED;
        newsplat->colfunc = colfunc;

        newsplat->x = x;
        newsplat->y = y;
        P_SetThingPosition(newsplat);
        newsplat->z = ss->sector->floorheight;

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

void P_NullBloodSplatSpawner(fixed_t x, fixed_t y, int flags2, void(*colfunc)(void))
{
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

    th = P_SpawnMobj(source->x, source->y, source->z + 4 * 8 * FRACUNIT, type);

    if (th->info->seesound)
        S_StartSound(th, th->info->seesound);

    th->target = source;        // where it came from
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);

    // fuzzy player
    if (dest->flags & MF_SHADOW)
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

    th = P_SpawnMobj(x, y, z, type);

    if (th->info->seesound)
        S_StartSound(th, th->info->seesound);

    th->target = source;
    th->angle = an;
    th->momx = FixedMul(th->info->speed, finecosine[an >> ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->info->speed, finesine[an >> ANGLETOFINESHIFT]);
    th->momz = FixedMul(th->info->speed, slope);

    puffcount = 0;

    P_CheckMissileSpawn(th);
}
