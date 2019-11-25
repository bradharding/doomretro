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

#include "c_console.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"

int         r_blood = r_blood_default;
int         r_bloodsplats_max = r_bloodsplats_max_default;
int         r_bloodsplats_total;
dboolean    r_corpses_color = r_corpses_color_default;
dboolean    r_corpses_mirrored = r_corpses_mirrored_default;
dboolean    r_corpses_moreblood = r_corpses_moreblood_default;
dboolean    r_corpses_nudge = r_corpses_nudge_default;
dboolean    r_corpses_slide = r_corpses_slide_default;
dboolean    r_corpses_smearblood = r_corpses_smearblood_default;
dboolean    r_floatbob = r_floatbob_default;
dboolean    r_rockettrails = r_rockettrails_default;
dboolean    r_shadows = r_shadows_default;

static fixed_t floatbobdiffs[64] =
{
     25695,  25695,  25447,  24955,  24222,  23256,  22066,  20663,
     19062,  17277,  15325,  13226,  10999,   8667,   6251,   3775,
      1262,  -1262,  -3775,  -6251,  -8667, -10999, -13226, -15325,
    -17277, -19062, -20663, -22066, -23256, -24222, -24955, -25447,
    -25695, -25695, -25447, -24955, -24222, -23256, -22066, -20663,
    -19062, -17277, -15325, -13226, -11000,  -8667,  -6251,  -3775,
     -1262,   1262,   3775,   6251,   8667,  10999,  13226,  15325,
     17277,  19062,  20663,  22066,  23256,  24222,  24955,  25447
};

extern fixed_t      animatedliquiddiffs[64];
extern int          deadlookdir;
extern int          deathcount;
extern msecnode_t   *sector_list;   // phares 3/16/98
extern dboolean     usemouselook;

void A_Recoil(weapontype_t weapon);
void G_PlayerReborn(void);
void P_DelSeclist(msecnode_t *node);

//
//
// P_SetMobjState
// Returns true if the mobj is still present.
//
dboolean P_SetMobjState(mobj_t *mobj, statenum_t state)
{
    do
    {
        state_t *st;

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
        if (st->action)
            st->action(mobj, NULL, NULL);

        state = st->nextstate;
    } while (!mobj->tics);

    return true;
}

//
// P_ExplodeMissile
//
void P_ExplodeMissile(mobj_t *mo)
{
    mo->momx = 0;
    mo->momy = 0;
    mo->momz = 0;

    P_SetMobjState(mo, mo->info->deathstate);

    mo->tics = MAX(1, mo->tics - (M_Random() & 3));
    mo->flags &= ~MF_MISSILE;

    // [BH] make explosion translucent
    if (mo->type == MT_ROCKET)
    {
        mo->colfunc = tlcolfunc;
        mo->flags2 &= ~MF2_CASTSHADOW;
    }

    if (mo->info->deathsound)
        S_StartSound(mo, mo->info->deathsound);
}

//
// P_XYMovement
//
#define STOPSPEED       0x1000
#define WATERFRICTION   0xD500

static int  puffcount;

static void P_XYMovement(mobj_t *mo)
{
    player_t    *player;
    fixed_t     xmove, ymove;
    mobjtype_t  type = mo->type;
    int         flags2 = mo->flags2;
    dboolean    corpse;
    int         stepdir = 0;

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
    corpse = ((mo->flags & MF_CORPSE) && type != MT_BARREL);

    // [BH] give smoke trails to rockets
    if (flags2 & MF2_SMOKETRAIL)
        if (puffcount++)
            P_SpawnSmokeTrail(mo->x, mo->y, mo->z, mo->angle);

    mo->momx = BETWEEN(-MAXMOVE, mo->momx, MAXMOVE);
    mo->momy = BETWEEN(-MAXMOVE, mo->momy, MAXMOVE);

    xmove = mo->momx;
    ymove = mo->momy;

    if (xmove < 0)
    {
        xmove = -xmove;
        stepdir = 1;
    }

    if (ymove < 0)
    {
        ymove = -ymove;
        stepdir |= 2;
    }

    do
    {
        fixed_t stepx = MIN(xmove, MAXMOVE_STEP);
        fixed_t stepy = MIN(ymove, MAXMOVE_STEP);
        fixed_t ptryx = mo->x + ((stepdir & 1) ? -stepx : stepx);
        fixed_t ptryy = mo->y + ((stepdir & 2) ? -stepy : stepy);

        xmove -= stepx;
        ymove -= stepy;

        // killough 3/15/98: Allow objects to drop off
        if (!P_TryMove(mo, ptryx, ptryy, 1))
        {
            // blocked move
            // killough 8/11/98: bouncing off walls
            // killough 10/98:
            // Add ability for objects other than players to bounce on ice
            if (!(mo->flags & MF_MISSILE) && !player && blockline && mo->z <= mo->floorz && P_GetFriction(mo, NULL) > ORIG_FRICTION)
            {
                fixed_t r = ((blockline->dx >> FRACBITS) * mo->momx + (blockline->dy >> FRACBITS) * mo->momy)
                            / ((blockline->dx >> FRACBITS) * (blockline->dx >> FRACBITS)
                            + (blockline->dy >> FRACBITS) * (blockline->dy >> FRACBITS));
                fixed_t x = FixedMul(r, blockline->dx);
                fixed_t y = FixedMul(r, blockline->dy);

                // reflect momentum away from wall
                mo->momx = x * 2 - mo->momx;
                mo->momy = y * 2 - mo->momy;

                // if under gravity, slow down in
                // direction perpendicular to wall.
                if (!(mo->flags & MF_NOGRAVITY))
                {
                    mo->momx = (mo->momx + x) / 2;
                    mo->momy = (mo->momy + y) / 2;
                }
            }
            else if (player)
            {
                // try to slide along it
                P_SlideMove(mo);
                break;
            }
            else if (mo->flags & MF_MISSILE)
            {
                // explode a missile
                if (ceilingline && ceilingline->backsector
                    && ceilingline->backsector->ceilingpic == skyflatnum
                    && mo->z > ceilingline->backsector->ceilingheight)
                {
                    // Hack to prevent missiles exploding
                    // against the sky.
                    // Does not handle sky floors.

                    // [BH] still play sound when firing BFG into sky
                    if (type == MT_BFG)
                        S_StartSound(mo, mo->info->deathsound);

                    P_RemoveMobj(mo);
                    return;
                }

                P_ExplodeMissile(mo);
            }
            else
            {
                mo->momx = 0;
                mo->momy = 0;
            }
        }
    } while (xmove || ymove);

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
        return;         // no friction for missiles or lost souls ever

    if (mo->z > mo->floorz && !(flags2 & MF2_ONMOBJ))
        return;         // no friction when airborne

    // [BH] spawn random blood splats on floor as corpses slide
    if (corpse && !(mo->flags & MF_NOBLOOD) && mo->blood && r_corpses_slide && r_corpses_smearblood && (mo->momx || mo->momy)
        && mo->bloodsplats && r_bloodsplats_max && !mo->nudge)
    {
        int blood = mobjinfo[mo->blood].blood;

        if (blood)
        {
            int     radius = (spritewidth[sprites[mo->sprite].spriteframes[mo->frame & FF_FRAMEMASK].lump[0]] >> FRACBITS) >> 1;
            int     max = MIN((ABS(mo->momx) + ABS(mo->momy)) >> (FRACBITS - 2), 8);
            fixed_t floorz = mo->floorz;

            for (int i = 0; i < max; i++)
            {
                fixed_t x, y;

                if (!mo->bloodsplats)
                    break;

                x = mo->x + (M_RandomInt(-radius, radius) << FRACBITS);
                y = mo->y + (M_RandomInt(-radius, radius) << FRACBITS);

                P_SpawnBloodSplat(x, y, blood, floorz, mo);
            }
        }
    }

    if ((corpse || (flags2 & MF2_FALLING))
        && (mo->momx > FRACUNIT / 4 || mo->momx < -FRACUNIT / 4 || mo->momy > FRACUNIT / 4 || mo->momy < -FRACUNIT / 4)
        && mo->floorz != mo->subsector->sector->floorheight)
        return;         // do not stop sliding if halfway off a step with some momentum

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED && mo->momy > -STOPSPEED && mo->momy < STOPSPEED
        && (!player || (!player->cmd.forwardmove && !player->cmd.sidemove) || player->mo != mo))
    {
        mo->momx = 0;
        mo->momy = 0;

        // killough 10/98: kill any bobbing momentum too (except in voodoo dolls)
        if (player && player->mo == mo)
        {
            player->momx = 0;
            player->momy = 0;
        }
    }
    else if ((flags2 & MF2_FEETARECLIPPED) && corpse && !player)
    {
        // [BH] increase friction for corpses in water
        mo->momx = FixedMul(mo->momx, WATERFRICTION);
        mo->momy = FixedMul(mo->momy, WATERFRICTION);
    }
    else
    {
        // phares 3/17/98
        //
        // Friction will have been adjusted by friction thinkers for
        // icy or muddy floors. Otherwise it was never touched and
        // remained set at ORIG_FRICTION
        //
        // killough 8/28/98: removed inefficient thinker algorithm,
        // instead using touching_sectorlist in P_GetFriction() to
        // determine friction (and thus only when it is needed).
        //
        // killough 10/98: changed to work with new bobbing method.
        // Reducing player momentum is no longer needed to reduce
        // bobbing, so ice works much better now.
        fixed_t friction = P_GetFriction(mo, NULL);

        mo->momx = FixedMul(mo->momx, friction);
        mo->momy = FixedMul(mo->momy, friction);

        // killough 10/98: Always decrease player bobbing by ORIG_FRICTION.
        // This prevents problems with bobbing on ice, where it was not being
        // reduced fast enough, leading to all sorts of kludges being developed.
        if (player && player->mo == mo)     //  Not voodoo dolls
        {
            player->momx = FixedMul(player->momx, ORIG_FRICTION);
            player->momy = FixedMul(player->momy, ORIG_FRICTION);
        }
    }
}

//
// P_ZMovement
//
static void P_ZMovement(mobj_t *mo)
{
    player_t    *player = mo->player;
    int         flags = mo->flags;
    fixed_t     floorz = mo->floorz;

    // check for smooth step up
    if (player && player->mo == mo && mo->z < floorz && !viewplayer->jumptics)
    {
        player->viewheight -= floorz - mo->z;
        player->deltaviewheight = (VIEWHEIGHT - player->viewheight) >> 3;
    }

    // adjust height
    mo->z += mo->momz;

    // float down towards target if too close
    if (!((flags ^ MF_FLOAT) & (MF_FLOAT | MF_SKULLFLY | MF_INFLOAT)) && mo->target)
    {
        fixed_t delta = (mo->target->z + (mo->height >> 1) - mo->z) * 3;

        if (P_ApproxDistance(mo->x - mo->target->x, mo->y - mo->target->y) < ABS(delta))
            mo->z += SIGN(delta) * FLOATSPEED;
    }

    // clip movement
    if (mo->z <= floorz)
    {
        int blood = mo->blood;

        // [BH] remove blood the moment it hits the ground and spawn blood splats in its place
        if (blood && (mo->flags2 & MF2_BLOOD))
        {
            P_RemoveBloodMobj(mo);

            if (r_bloodsplats_max)
            {
                fixed_t x = mo->x;
                fixed_t y = mo->y;

                P_SpawnBloodSplat(x, y, blood, floorz, NULL);

                if (blood != FUZZYBLOOD)
                {
                    fixed_t r1 = M_RandomInt(-3, 3) << FRACBITS;
                    fixed_t r2 = M_RandomInt(-3, 3) << FRACBITS;

                    P_SpawnBloodSplat(x + r1, y + r2, blood, floorz, NULL);
                    P_SpawnBloodSplat(x - r1, y - r2, blood, floorz, NULL);
                }
            }

            return;
        }

        // hit the floor
        if (flags & MF_SKULLFLY)
            mo->momz = -mo->momz;       // the skull slammed into something

        if (mo->momz < 0)
        {
            if (player && player->mo == mo)
            {
                player->jumptics = 7;

                if (weaponbounce && !freeze)
                    player->bouncemax = mo->momz >> 1;

                if (mo->momz < -GRAVITY * 8)
                {
                    // Squat down.
                    // Decrease viewheight for a moment
                    // after hitting the ground (hard),
                    // and utter appropriate sound.
                    player->deltaviewheight = mo->momz >> 3;

                    if (mo->health > 0)
                        S_StartSound(mo, sfx_oof);
                }
            }

            mo->momz = 0;
        }

        mo->z = floorz;

        if (!((flags ^ MF_MISSILE) & (MF_MISSILE | MF_NOCLIP)))
        {
            P_ExplodeMissile(mo);
            return;
        }
    }
    else if (!(flags & MF_NOGRAVITY))
    {
        if (!mo->momz)
            mo->momz = -GRAVITY;

        mo->momz -= GRAVITY;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
        if (flags & MF_SKULLFLY)
            mo->momz = -mo->momz;       // the skull slammed into something

        // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;

        mo->z = mo->ceilingz - mo->height;

        if (!((flags ^ MF_MISSILE) & (MF_MISSILE | MF_NOCLIP)))
        {
            if (mo->subsector->sector->ceilingpic == skyflatnum)
                P_RemoveMobj(mo);
            else
                P_ExplodeMissile(mo);
        }
    }
}

//
// P_NightmareRespawn
//
static void P_NightmareRespawn(mobj_t *mobj)
{
    fixed_t     x = mobj->spawnpoint.x << FRACBITS;
    fixed_t     y = mobj->spawnpoint.y << FRACBITS;
    fixed_t     z = ((mobj->flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ);
    mobj_t      *mo;
    mapthing_t  *mthing = &mobj->spawnpoint;

    // [BH] Fix <https://doomwiki.org/wiki/(0,0)_respawning_bug>.
    if (!x && !y)
    {
        x = mobj->x;
        y = mobj->y;
    }

    // something is occupying it's position?
    if (!P_CheckPosition(mobj, x, y))
        return;         // no respawn

    // spawn a teleport fog at old spot
    //  because of removal of the body?
    mo = P_SpawnMobj(mobj->x, mobj->y, z, MT_TFOG);
    mo->angle = mobj->angle;

    // initiate teleport sound
    S_StartSound(mo, sfx_telept);

    // spawn a teleport fog at the new spot
    if (x != mobj->x || y != mobj->y)
    {
        mo = P_SpawnMobj(x, y, z, MT_TFOG);
        mo->angle = ANG45 * (mthing->angle / 45);
        S_StartSound(mo, sfx_telept);
    }

    // spawn the new monster
    // inherit attributes from deceased one
    mo = P_SpawnMobj(x, y, z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;
    mo->angle = ANG45 * (mthing->angle / 45);

    mo->flags &= ~MF_COUNTKILL;

    if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;

    // killough 11/98: transfer friendliness from deceased
    mo->flags = (mo->flags & ~MF_FRIEND) | (mobj->flags & MF_FRIEND);

    mo->reactiontime = 18;

    // remove the old monster
    P_RemoveMobj(mobj);
}

//
// P_MobjThinker
//
void P_MobjThinker(mobj_t *mobj)
{
    int         flags = mobj->flags;
    int         flags2 = mobj->flags2;
    player_t    *player = mobj->player;
    sector_t    *sector = mobj->subsector->sector;

    // [AM] Handle interpolation unless we're an active player.
    if (mobj->interpolate == -1)
        mobj->interpolate = 0;
    else if (!(player && mobj == player->mo))
    {
        // Assume we can interpolate at the beginning of the tic.
        mobj->interpolate = 1;

        // Store starting position for mobj interpolation.
        mobj->oldx = mobj->x;
        mobj->oldy = mobj->y;
        mobj->oldz = mobj->z;
        mobj->oldangle = mobj->angle;
    }

    if (mobj->nudge > 0)
        mobj->nudge--;

    // momentum movement
    if (mobj->momx || mobj->momy || (flags & MF_SKULLFLY))
    {
        P_XYMovement(mobj);

        if (mobj->thinker.function == P_RemoveThinkerDelayed)
            return;
    }

    // [BH] bob objects in liquid
    if ((flags2 & MF2_FEETARECLIPPED) && !(flags2 & MF2_NOLIQUIDBOB) && mobj->z <= sector->floorheight && !mobj->momz
        && !sector->heightsec && r_liquid_bob)
        mobj->z += animatedliquiddiffs[(mobj->floatbob + leveltime) & 63];

    // [BH] otherwise bob certain power-ups
    else if ((flags2 & MF2_FLOATBOB) && !(flags & MF_CORPSE) && r_floatbob)
        mobj->z = BETWEEN(mobj->floorz - 1, mobj->z + floatbobdiffs[(mobj->floatbob + leveltime) & 63], mobj->ceilingz);

    else if (mobj->z != mobj->floorz || mobj->momz)
    {
        if ((flags2 & MF2_PASSMOBJ) && !infiniteheight)
        {
            mobj_t  *onmo = P_CheckOnMobj(mobj);

            if (!onmo)
            {
                P_ZMovement(mobj);
                mobj->flags2 &= ~MF2_ONMOBJ;
            }
            else if (player)
            {
                if (mobj->momz < -GRAVITY * 8)
                {
                    player->deltaviewheight = mobj->momz >> 3;

                    if (mobj->momz < -23 * FRACUNIT)
                        P_NoiseAlert(mobj);
                }

                if (onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
                {
                    player->viewheight -= onmo->z + onmo->height - mobj->z;
                    player->deltaviewheight = (VIEWHEIGHT - player->viewheight) >> 3;
                    mobj->z = onmo->z + onmo->height;
                    mobj->flags2 |= MF2_ONMOBJ;
                }

                mobj->momz = 0;
            }
        }
        else
            P_ZMovement(mobj);

        if (mobj->thinker.function == P_RemoveThinkerDelayed)
            return;
    }
    else if (!(mobj->momx | mobj->momy) && !sentient(mobj))
    {
        // killough 9/12/98: objects fall off ledges if they are hanging off
        // slightly push off of ledge if hanging more than halfway off
        if (((flags & MF_CORPSE) || (flags & MF_DROPPED) || mobj->type == MT_BARREL) && mobj->z - mobj->dropoffz > 2 * FRACUNIT)
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
        if (!--mobj->tics)
            P_SetMobjState(mobj, mobj->state->nextstate);
    }
    else
    {
        // check for nightmare respawn
        if ((flags & MF_COUNTKILL) && (gameskill == sk_nightmare || respawnmonsters))
            if (++mobj->movecount >= 12 * TICRATE && !(leveltime & 31) && M_Random() <= 4)
                P_NightmareRespawn(mobj);
    }
}

//
// P_SetShadowColumnFunction
//
void P_SetShadowColumnFunction(mobj_t *mobj)
{
    if ((mobj->flags & MF_FUZZ) && r_textures)
        mobj->shadowcolfunc = (r_shadows_translucency ? R_DrawFuzzyShadowColumn : R_DrawSolidFuzzyShadowColumn);
    else
        mobj->shadowcolfunc = (r_shadows_translucency ? R_DrawShadowColumn : R_DrawSolidShadowColumn);
}

//
// P_SpawnMobj
//
mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
    mobj_t      *mobj = Z_Calloc(1, sizeof(*mobj), PU_LEVEL, NULL);
    state_t     *st;
    mobjinfo_t  *info = &mobjinfo[type];
    sector_t    *sector;
    static int  prevx, prevy;
    static int  prevbob;

    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = (z == ONCEILINGZ && type != MT_KEEN && info->projectilepassheight ? info->projectilepassheight : info->height);
    mobj->flags = info->flags;
    mobj->flags2 = info->flags2;
    mobj->health = info->spawnhealth;

    if (z == ONCEILINGZ)
        mobj->flags2 &= ~MF2_CASTSHADOW;

    if (gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    // do not set the state with P_SetMobjState,
    // because action routines cannot be called yet
    st = &states[info->spawnstate];

    // [BH] initialize certain mobj's animations to random start frame
    // so groups of same mobjs are deliberately out of sync
    if (info->frames > 1)
    {
        int frames = M_RandomInt(0, info->frames);
        int i = 0;

        while (i++ < frames && st->nextstate != S_NULL)
            st = &states[st->nextstate];
    }

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    mobj->colfunc = info->colfunc;
    mobj->altcolfunc = info->altcolfunc;
    mobj->id = -1;

    P_SetShadowColumnFunction(mobj);

    mobj->shadowoffset = info->shadowoffset;
    mobj->blood = info->blood;

    // [BH] set random pitch for monster sounds when spawned
    mobj->pitch = NORM_PITCH;

    if ((mobj->flags & MF_SHOOTABLE) && type != MT_PLAYER && type != MT_BARREL)
        mobj->pitch += M_RandomInt(-16, 16);

    // set subsector and/or block links
    P_SetThingPosition(mobj);

    sector = mobj->subsector->sector;
    mobj->dropoffz =           // killough 11/98: for tracking dropoffs
    mobj->floorz = sector->floorheight;
    mobj->ceilingz = sector->ceilingheight;

    // [BH] initialize bobbing things
    mobj->floatbob = prevbob = (x == prevx && y == prevy ? prevbob : M_Random());
    prevx = x;
    prevy = y;

    if (z == ONFLOORZ)
    {
        mobj->z = mobj->floorz;

        if ((mobj->flags2 & MF2_FOOTCLIP) && !sector->heightsec && P_IsInLiquid(mobj))
            mobj->flags2 |= MF2_FEETARECLIPPED;
    }
    else if (z == ONCEILINGZ)
        mobj->z = mobj->ceilingz - mobj->height;
    else
        mobj->z = z;

    mobj->oldx = mobj->x;
    mobj->oldy = mobj->y;
    mobj->oldz = mobj->z;
    mobj->oldangle = mobj->angle;

    mobj->thinker.function = (type == MT_MUSICSOURCE ? MusInfoThinker : P_MobjThinker);
    P_AddThinker(&mobj->thinker);

    return mobj;
}

mapthing_t  itemrespawnque[ITEMQUEUESIZE];
int         itemrespawntime[ITEMQUEUESIZE];
int         iquehead;
int         iquetail;

//
// P_RemoveMobj
//
void P_RemoveMobj(mobj_t *mobj)
{
    int         flags = mobj->flags;
    mobjtype_t  type = mobj->type;

    if ((flags & MF_SPECIAL) && !(flags & MF_DROPPED) && type != MT_INV && type != MT_INS)
    {
        itemrespawnque[iquehead] = mobj->spawnpoint;
        itemrespawntime[iquehead] = leveltime;
        iquehead = (iquehead + 1) & (ITEMQUEUESIZE - 1);

        // lose one off the end?
        if (iquehead == iquetail)
            iquetail = (iquetail + 1) & (ITEMQUEUESIZE - 1);
    }

    // unlink from sector and block lists
    P_UnsetThingPosition(mobj);

    // [crispy] removed map objects may finish their sounds
    S_UnlinkSound(mobj);

    // Delete all nodes on the current sector_list
    if (sector_list)
    {
        P_DelSeclist(sector_list);
        sector_list = NULL;
    }

    if (flags & MF_SHOOTABLE)
    {
        P_SetTarget(&mobj->target, NULL);
        P_SetTarget(&mobj->tracer, NULL);
        P_SetTarget(&mobj->lastenemy, NULL);
    }

    // free block
    P_RemoveThinker((thinker_t *)mobj);
}

//
// P_RemoveBlood
//
void P_RemoveBloodMobj(mobj_t *mobj)
{
    // unlink from sector and block lists
    P_UnsetThingPosition(mobj);

    // free block
    P_RemoveThinker((thinker_t *)mobj);
}

//
// P_FindDoomedNum
// Finds a mobj type with a matching doomednum
// killough 8/24/98: rewrote to use hashing
//
mobjtype_t P_FindDoomedNum(unsigned int type)
{
    static struct
    {
        int first;
        int next;
    } *hash;

    mobjtype_t  i;

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

    while (i < NUMMOBJTYPES && (unsigned int)mobjinfo[i].doomednum != type)
        i = hash[i].next;

    return i;
}

//
// P_RespawnSpecials
//
void P_RespawnSpecials(void)
{
    fixed_t     x, y, z;
    mobj_t      *mo;
    mapthing_t  *mthing;
    int         i;

    if (!respawnitems)
        return;

    // nothing left to respawn?
    if (iquehead == iquetail)
        return;

    // wait at least 30 seconds
    if (leveltime - itemrespawntime[iquetail] < 30 * TICRATE)
        return;

    mthing = &itemrespawnque[iquetail];

    // find which type to spawn
    // killough 8/23/98: use table for faster lookup
    i = P_FindDoomedNum(mthing->type);

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ((mobjinfo[i].flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ);

    // spawn a teleport fog at the new spot
    mo = P_SpawnMobj(x, y, z, MT_IFOG);
    S_StartSound(mo, sfx_itmbk);

    // spawn it
    mo = P_SpawnMobj(x, y, z, i);
    mo->spawnpoint = *mthing;
    mo->angle = ANG45 * (mthing->angle / 45);

    // pull it from the queue
    iquetail = (iquetail + 1) & (ITEMQUEUESIZE - 1);
}

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
extern int  lastlevel;
extern int  lastepisode;

static void P_SpawnPlayer(const mapthing_t *mthing)
{
    mobj_t  *mobj;

    if (viewplayer->playerstate == PST_REBORN)
        G_PlayerReborn();

    mobj = P_SpawnMobj(mthing->x << FRACBITS, mthing->y << FRACBITS, ONFLOORZ, MT_PLAYER);

    for (const struct msecnode_s *seclist = mobj->touching_sectorlist; seclist; seclist = seclist->m_tnext)
        mobj->z = mobj->floorz = MAX(mobj->z, seclist->m_sector->floorheight);

    mobj->angle = ((mthing->angle % 45) ? mthing->angle * (ANG45 / 45) : ANG45 * (mthing->angle / 45));
    mobj->player = viewplayer;
    mobj->health = viewplayer->health;

    viewplayer->mo = mobj;
    viewplayer->playerstate = PST_LIVE;
    viewplayer->refire = 0;
    viewplayer->message = NULL;
    viewplayer->damagecount = 0;
    viewplayer->bonuscount = 0;
    viewplayer->extralight = 0;
    viewplayer->fixedcolormap = 0;

    viewplayer->viewheight = VIEWHEIGHT;
    viewplayer->viewz = viewplayer->oldviewz = mobj->z + viewplayer->viewheight;

    if ((mobj->flags2 & MF2_FEETARECLIPPED) && r_liquid_lowerview)
        viewplayer->viewz -= FOOTCLIPSIZE;

    viewplayer->psprites[ps_weapon].sx = 0;
    viewplayer->mo->momx = 0;
    viewplayer->mo->momy = 0;
    viewplayer->momx = 0;
    viewplayer->momy = 0;
    viewplayer->lookdir = 0;
    viewplayer->recoil = 0;
    viewplayer->jumptics = 0;
    viewplayer->bounce = 0;
    viewplayer->bouncemax = 0;

    deathcount = 0;
    deadlookdir = -1;

    // setup gun psprite
    P_SetupPsprites();

    lastlevel = -1;
    lastepisode = -1;

    ST_Start(); // wake up the status bar
    HU_Start(); // wake up the heads up text
}

//
// P_SpawnMoreBlood
// [BH] Spawn blood splats around corpses
//
static void P_SpawnMoreBlood(mobj_t *mobj)
{
    int blood = mobjinfo[mobj->blood].blood;

    if (blood)
    {
        int     radius = ((spritewidth[sprites[mobj->sprite].spriteframes[0].lump[0]] >> FRACBITS) >> 1) + 12;
        int     max = M_RandomInt(50, 100) + radius;
        fixed_t x = mobj->x;
        fixed_t y = mobj->y;
        fixed_t floorz = mobj->floorz;

        if (!(mobj->flags & MF_SPAWNCEILING))
        {
            x += M_RandomInt(-radius / 3, radius / 3) << FRACBITS;
            y += M_RandomInt(-radius / 3, radius / 3) << FRACBITS;
        }

        for (int i = 0; i < max; i++)
        {
            angle_t angle;
            fixed_t fx, fy;

            if (!mobj->bloodsplats)
                break;

            angle = M_RandomInt(0, FINEANGLES - 1);
            fx = x + FixedMul(M_RandomInt(0, radius) << FRACBITS, finecosine[angle]);
            fy = y + FixedMul(M_RandomInt(0, radius) << FRACBITS, finesine[angle]);

            P_SpawnBloodSplat(fx, fy, blood, floorz, mobj);
        }
    }
}

//
// P_SpawnMapThing
// The fields of the mapthing should
//  already be in host byte order.
//
mobj_t *P_SpawnMapThing(mapthing_t *mthing, dboolean spawnmonsters)
{
    int     i;
    int     bit;
    mobj_t  *mobj;
    fixed_t x, y, z;
    short   type = mthing->type;
    short   options = mthing->options;
    int     flags;
    int     musicid = 0;

    // check for players specially
    if (type == Player1Start)
    {
        P_SpawnPlayer(mthing);
        viewplayer->mo->id = thingid;
        return NULL;
    }
    else if ((type >= Player2Start && type <= Player4Start) || type == PlayerDeathmatchStart)
        return NULL;

    if (options & MTF_NOTSINGLE)
        return NULL;

    if (gameskill == sk_baby)
        bit = 1;
    else if (gameskill == sk_nightmare)
        bit = 4;
    else
        bit = 1 << (gameskill - 1);

    if (type >= 14100 && type <= 14164)
    {
        musicid = type - 14100;
        type = MusicSource;
    }

    // killough 8/23/98: use table for faster lookup
    if ((i = P_FindDoomedNum(type)) == NUMMOBJTYPES)
    {
        // [BH] make unknown thing type non-fatal and show console warning instead
        if (type != VisualModeCamera)
            C_Warning(2, "Thing %s at (%i,%i) didn't spawn because it has an unknown type.", commify(thingid), mthing->x, mthing->y);

        return NULL;
    }

    // check for appropriate skill level
    if (!(options & (MTF_EASY | MTF_NORMAL | MTF_HARD)) && (!canmodify || !r_fixmaperrors) && type != VisualModeCamera)
    {
        if (*mobjinfo[i].name1)
            C_Warning(2, "The %s at (%i,%i) didn't spawn because it has no skill flags.", mobjinfo[i].name1, mthing->x, mthing->y);
        else
            C_Warning(2, "Thing %s at (%i,%i) didn't spawn because it has no skill flags.", commify(thingid), mthing->x, mthing->y);

        return NULL;
    }

    if (!(options & bit))
        return NULL;

    // [BH] don't spawn any monster corpses if -nomonsters
    if ((mobjinfo[i].flags & MF_CORPSE) && !spawnmonsters && i != MT_MISC62)
        return NULL;

    if (mobjinfo[i].flags & MF_COUNTKILL)
    {
        // don't spawn any monsters if -nomonsters
        if (!spawnmonsters && i != MT_KEEN)
            return NULL;

        // killough 7/20/98: exclude friends
        if (!((mobjinfo[i].flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
            totalkills++;

        monstercount[i]++;
    }
    else if (i == MT_BARREL)
        barrelcount++;

    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ((mobjinfo[i].flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ);

    mobj = P_SpawnMobj(x, y, z, (mobjtype_t)i);
    mobj->spawnpoint = *mthing;
    mobj->musicid = musicid;

    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;

    if (!(mobj->flags & MF_FRIEND) && (options & MTF_FRIEND))
    {
        mobj->flags |= MF_FRIEND;           // killough 10/98:
        mbfcompatible = true;
        P_UpdateThinker(&mobj->thinker);    // transfer friendliness flag
    }

    flags = mobj->flags;

    if (flags & MF_COUNTITEM)
        totalitems++;

    if (flags & MF_SPECIAL)
        totalpickups++;

    if (mobj->tics > 0)
        mobj->tics = 1 + (M_Random() % mobj->tics);

    mobj->angle = ((mthing->angle % 45) ? mthing->angle * (ANG45 / 45) : ANG45 * (mthing->angle / 45));

    // [BH] identify boss monsters so sounds won't be clipped
    if (gamemode != commercial)
        if (gamemap == 8 && ((gameepisode == 1 && i == MT_BRUISER) || (gameepisode == 2 && i == MT_CYBORG)
            || (gameepisode == 3 && i == MT_SPIDER)))
            mobj->flags2 |= MF2_BOSS;

    // [BH] randomly mirror corpses
    if ((flags & MF_CORPSE) && r_corpses_mirrored && (M_Random() & 1))
        mobj->flags2 |= MF2_MIRRORED;

    // [BH] randomly mirror weapons
    if (r_mirroredweapons && (type == SuperShotgun || (type >= Shotgun && type <= BFG9000)) && (M_Random() & 1))
        mobj->flags2 |= MF2_MIRRORED;

    // [BH] spawn blood splats around corpses
    if (!(flags & (MF_SHOOTABLE | MF_NOBLOOD | MF_SPECIAL)) && mobj->blood && !chex
        && (!hacx || !(mobj->flags2 & MF2_DECORATION)) && r_bloodsplats_max
        && (BTSX || lumpinfo[firstspritelump + sprites[mobj->sprite].spriteframes[0].lump[0]]->wadfile->type != PWAD))
    {
        mobj->bloodsplats = CORPSEBLOODSPLATS;

        if (r_corpses_moreblood && mobj->subsector->sector->terraintype == SOLID)
            P_SpawnMoreBlood(mobj);
    }

    // [crispy] randomly colorize space marine corpse objects
    if (mobj->info->spawnstate == S_PLAY_DIE7 || mobj->info->spawnstate == S_PLAY_XDIE9)
        mobj->flags |= (M_RandomInt(0, 3) << MF_TRANSSHIFT);

    if ((mobj->flags2 & MF2_DECORATION) && i != MT_BARREL)
        numdecorations++;

    return mobj;
}

//
// GAME SPAWN FUNCTIONS
//

//
// P_SpawnPuff
//
void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z, angle_t angle)
{
    mobj_t      *th = Z_Calloc(1, sizeof(*th), PU_LEVEL, NULL);
    mobjinfo_t  *info = &mobjinfo[MT_PUFF];
    state_t     *st = &states[info->spawnstate];
    sector_t    *sector;

    th->type = MT_PUFF;
    th->info = info;
    th->x = x;
    th->y = y;
    th->momz = FRACUNIT;
    th->angle = angle;
    th->flags = info->flags;
    th->flags2 = (info->flags2 | ((M_Random() & 1) * MF2_MIRRORED));

    th->state = st;
    th->tics = MAX(1, st->tics - (M_Random() & 3));
    th->sprite = st->sprite;
    th->frame = st->frame;

    th->colfunc = info->colfunc;
    th->altcolfunc = info->altcolfunc;
    th->id = -1;

    P_SetThingPosition(th);

    sector = th->subsector->sector;
    th->floorz = sector->interpfloorheight;
    th->ceilingz = sector->interpceilingheight;

    th->z = z + (M_SubRandom() << 10);

    th->thinker.function = P_MobjThinker;
    P_AddThinker(&th->thinker);

    // don't make punches spark on the wall
    if (attackrange == MELEERANGE)
    {
        P_SetMobjState(th, S_PUFF3);

        if (gp_vibrate_damage)
        {
            int strength = weaponinfo[wp_fist].strength * gp_vibrate_damage / 100;

            if (viewplayer->powers[pw_strength])
                strength *= 2;

            I_GamepadVibration(strength);
            weaponvibrationtics = weaponinfo[wp_fist].tics;
        }
    }
}

//
// P_SpawnSmokeTrail
//
void P_SpawnSmokeTrail(fixed_t x, fixed_t y, fixed_t z, angle_t angle)
{
    mobj_t  *th = P_SpawnMobj(x, y, z + (M_SubRandom() << 10), MT_TRAIL);

    th->momz = FRACUNIT / 2;
    th->tics -= M_Random() & 3;

    th->angle = angle;

    th->flags2 |= (M_Random() & 1) * MF2_MIRRORED;
}

//
// P_SpawnBlood
// [BH] spawn much more blood than Vanilla DOOM
//
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, angle_t angle, int damage, mobj_t *target)
{
    int         minz = target->z;
    int         maxz = minz + spriteheight[sprites[target->sprite].spriteframes[0].lump[0]];
    int         type = (r_blood == r_blood_all && target->blood ? target->blood : MT_BLOOD);
    mobjinfo_t  *info = &mobjinfo[type];
    int         blood = info->blood;
    state_t     *st = &states[info->spawnstate];

    angle += ANG180;

    for (int i = (damage >> 2) + 1; i > 0; i--)
    {
        mobj_t      *th = Z_Calloc(1, sizeof(*th), PU_LEVEL, NULL);
        sector_t    *sector;

        th->type = type;
        th->info = info;
        th->x = x + M_RandomInt(-2, 2) * FRACUNIT;
        th->y = y + M_RandomInt(-2, 2) * FRACUNIT;
        th->flags = info->flags;
        th->flags2 = (info->flags2 | ((M_Random() & 1) * MF2_MIRRORED));

        th->state = st;
        th->tics = MAX(1, st->tics - (M_Random() & 2));
        th->sprite = st->sprite;
        th->frame = st->frame;

        th->colfunc = info->colfunc;
        th->altcolfunc = info->altcolfunc;
        th->blood = blood;
        th->id = -1;

        P_SetThingPosition(th);

        sector = th->subsector->sector;
        th->floorz = sector->interpfloorheight;
        th->ceilingz = sector->interpceilingheight;

        th->z = BETWEEN(minz, z + (M_SubRandom() << 10), maxz);

        th->thinker.function = P_MobjThinker;
        P_AddThinker(&th->thinker);

        th->momx = FixedMul(i * FRACUNIT / 4, finecosine[angle >> ANGLETOFINESHIFT]);
        th->momy = FixedMul(i * FRACUNIT / 4, finesine[angle >> ANGLETOFINESHIFT]);
        th->momz = FRACUNIT * (2 + i / 6);

        th->angle = angle;
        angle += M_SubRandom() * 0xB60B60;

        if (damage <= 12 && th->state->nextstate)
            P_SetMobjState(th, th->state->nextstate);

        if (damage < 9 && th->state->nextstate)
            P_SetMobjState(th, th->state->nextstate);
    }
}

//
// P_SpawnBloodSplat
//
void P_SpawnBloodSplat(fixed_t x, fixed_t y, int blood, fixed_t maxheight, mobj_t *target)
{
    if (r_bloodsplats_total >= r_bloodsplats_max)
        return;
    else
    {
        sector_t    *sec = R_PointInSubsector(x, y)->sector;

        if (sec->terraintype == SOLID && sec->interpfloorheight <= maxheight && sec->floorpic != skyflatnum)
        {
            bloodsplat_t    *splat = malloc(sizeof(*splat));
            int             patch = firstbloodsplatlump + (M_Random() & 7);

            splat->patch = patch;
            splat->flip = M_Random() & 1;
            splat->colfunc = (blood == FUZZYBLOOD ? fuzzcolfunc : bloodsplatcolfunc);
            splat->blood = blood;
            splat->x = x;
            splat->y = y;
            splat->width = spritewidth[patch];
            splat->sector = sec;
            P_SetBloodSplatPosition(splat);
            r_bloodsplats_total++;

            if (target && target->bloodsplats)
                target->bloodsplats--;
        }
    }
}

//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
void P_CheckMissileSpawn(mobj_t *th)
{
    th->tics = MAX(1, th->tics - (M_Random() & 3));

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx >> 1);
    th->y += (th->momy >> 1);
    th->z += (th->momz >> 1);

    // killough 8/12/98: for non-missile objects (e.g. grenades)
    if (!(th->flags & MF_MISSILE))
        return;

    // killough 3/15/98: no dropoff (really = don't care for missiles)
    if (!P_TryMove(th, th->x, th->y, 0))
        P_ExplodeMissile(th);
}

//
// P_SpawnMissile
//
mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type)
{
    fixed_t z = source->z + 32 * FRACUNIT;
    mobj_t  *th;
    angle_t an;
    int     dist;
    int     speed;

    if ((source->flags2 & MF2_FEETARECLIPPED) && !source->subsector->sector->heightsec && r_liquid_clipsprites)
        z -= FOOTCLIPSIZE;

    th = P_SpawnMobj(source->x, source->y, z, type);

    if (th->info->seesound)
        S_StartSound(th, th->info->seesound);

    P_SetTarget(&th->target, source);   // where it came from
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);

    // fuzzy player
    if (dest->flags & MF_FUZZ)
        an += M_SubRandom() << 20;

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    speed = th->info->speed;
    th->momx = FixedMul(speed, finecosine[an]);
    th->momy = FixedMul(speed, finesine[an]);
    dist = MAX(1, P_ApproxDistance(dest->x - source->x, dest->y - source->y) / speed);
    th->momz = (dest->z - source->z) / dist;
    th->flags2 |= MF2_MONSTERMISSILE;
    P_CheckMissileSpawn(th);

    return th;
}

//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster.
//
void P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type)
{
    mobj_t  *th;
    angle_t an = source->angle;
    fixed_t x, y, z;
    fixed_t slope;
    int     speed;

    if (usemouselook && !autoaim)
        slope = PLAYERSLOPE(source->player);
    else
    {
        // killough 8/2/98: prefer autoaiming at enemies
        int mask = MF_FRIEND;

        do
        {
            // see which target is to be aimed at
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, mask);

            if (!linetarget)
            {
                slope = P_AimLineAttack(source, (an += 1 << 26), 16 * 64 * FRACUNIT, mask);

                if (!linetarget)
                {
                    slope = P_AimLineAttack(source, (an -= 2 << 26), 16 * 64 * FRACUNIT, mask);

                    if (!linetarget)
                    {
                        an = source->angle;
                        slope = (usemouselook ? PLAYERSLOPE(source->player) : 0);
                    }
                }
            }
        } while (mask && (mask = 0, !linetarget));  // killough 8/2/98
    }

    x = source->x;
    y = source->y;
    z = source->z + 4 * 8 * FRACUNIT;

    if ((source->flags2 & MF2_FEETARECLIPPED) && !source->subsector->sector->heightsec && r_liquid_lowerview)
        z -= FOOTCLIPSIZE;

    th = P_SpawnMobj(x, y, z, type);

    if (th->info->seesound)
        S_StartSound(th, th->info->seesound);

    P_SetTarget(&th->target, source);
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    speed = th->info->speed;
    th->momx = FixedMul(speed, finecosine[an]);
    th->momy = FixedMul(speed, finesine[an]);
    th->momz = FixedMul(speed, slope);
    th->interpolate = -1;

    P_NoiseAlert(source);

    if (type == MT_ROCKET && r_rockettrails && !hacx && viewplayer->readyweapon == wp_missile && !doom4vanilla)
    {
        th->flags2 |= MF2_SMOKETRAIL;
        puffcount = 0;
    }

    P_CheckMissileSpawn(th);
    A_Recoil(source->player->readyweapon);
}
