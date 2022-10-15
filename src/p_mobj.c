/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

#include "am_map.h"
#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_gamecontroller.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
bool P_SetMobjState(mobj_t *mobj, statenum_t state)
{
    do
    {
        if (state == S_NULL)
        {
            mobj->state = (state_t *)S_NULL;
            P_RemoveMobj(mobj);

            return false;
        }
        else
        {
            state_t *st = &states[state];

            mobj->state = st;
            mobj->tics = st->tics;
            mobj->sprite = st->sprite;
            mobj->frame = st->frame;

            // Modified handling.
            // Call action functions when the state is set
            if (st->action)
                st->action(mobj, NULL, NULL);

            state = st->nextstate;
        }
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

static void P_XYMovement(mobj_t *mo)
{
    player_t    *player;
    fixed_t     xmove, ymove;
    mobjtype_t  type;
    int         flags;
    int         flags2;
    bool        corpse;
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
    type = mo->type;
    flags = mo->flags;
    flags2 = mo->flags2;
    corpse = ((flags & MF_CORPSE) && type != MT_BARREL);

    // [BH] give smoke trails to rockets
    if ((flags2 & MF2_SMOKETRAIL) && mo->pursuecount++)
        P_SpawnSmokeTrail(mo->x, mo->y, mo->z, mo->angle);

    mo->momx = BETWEEN(-MAXMOVE, mo->momx, MAXMOVE);
    mo->momy = BETWEEN(-MAXMOVE, mo->momy, MAXMOVE);

    if ((xmove = mo->momx) < 0)
    {
        xmove = -xmove;
        stepdir = 1;
    }

    if ((ymove = mo->momy) < 0)
    {
        ymove = -ymove;
        stepdir |= 2;
    }

    do
    {
        const fixed_t   stepx = MIN(xmove, MAXMOVE_STEP);
        const fixed_t   stepy = MIN(ymove, MAXMOVE_STEP);
        const fixed_t   ptryx = mo->x + ((stepdir & 1) ? -stepx : stepx);
        const fixed_t   ptryy = mo->y + ((stepdir & 2) ? -stepy : stepy);

        xmove -= stepx;
        ymove -= stepy;

        // killough 03/15/98: Allow objects to drop off
        if (!P_TryMove(mo, ptryx, ptryy, 1))
        {
            // blocked move
            // killough 08/11/98: bouncing off walls
            // killough 10/98:
            // Add ability for objects other than players to bounce on ice
            if (!(flags & MF_MISSILE)
                && ((flags & MF_BOUNCES)
                    || (!player && blockline && mo->z <= mo->floorz && P_GetFriction(mo, NULL) > ORIG_FRICTION)))
            {
                if (blockline)
                {
                    const fixed_t   r = ((blockline->dx >> FRACBITS) * mo->momx + (blockline->dy >> FRACBITS) * mo->momy)
                                        / ((blockline->dx >> FRACBITS) * (blockline->dx >> FRACBITS)
                                        + (blockline->dy >> FRACBITS) * (blockline->dy >> FRACBITS));
                    const fixed_t   x = FixedMul(r, blockline->dx);
                    const fixed_t   y = FixedMul(r, blockline->dy);

                    // reflect momentum away from wall
                    mo->momx = x * 2 - mo->momx;
                    mo->momy = y * 2 - mo->momy;

                    // if under gravity, slow down in direction perpendicular to wall.
                    if (!(flags & MF_NOGRAVITY))
                    {
                        mo->momx = (mo->momx + x) / 2;
                        mo->momy = (mo->momy + y) / 2;
                    }
                }
                else
                {
                    mo->momx = 0;
                    mo->momy = 0;
                }
            }
            else if (player)
            {
                // try to slide along it
                P_SlideMove(mo);
                break;
            }
            else if (flags & MF_MISSILE)
            {
                // explode a missile
                if (ceilingline && ceilingline->backsector
                    && ceilingline->backsector->ceilingpic == skyflatnum
                    && mo->z > ceilingline->backsector->ceilingheight)
                {
                    // Hack to prevent missiles exploding against the sky.
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

    if (flags & (MF_MISSILE | MF_SKULLFLY))
        return;         // no friction for missiles or lost souls ever

    if (mo->z > mo->floorz && !(flags2 & MF2_ONMOBJ))
        return;         // no friction when airborne

    // [BH] spawn random blood splats on floor as corpses slide
    if (corpse && !(flags & MF_NOBLOOD) && mo->bloodcolor > 0 && r_corpses_smearblood
        && (mo->momx || mo->momy) && mo->bloodsplats && r_bloodsplats_max && !mo->nudge
        && (r_blood != r_blood_all || !(flags & MF_FUZZ)))
    {
        const int   max = MIN((ABS(mo->momx) + ABS(mo->momy)) >> (FRACBITS - 2), 8);

        if (max)
        {
            const int       color = colortranslation[mo->bloodcolor - 1][REDBLOODSPLATCOLOR];
            const int       radius = (spritewidth[sprites[mo->sprite].spriteframes[mo->frame & FF_FRAMEMASK].lump[0]] >> FRACBITS) >> 1;
            const fixed_t   floorz = mo->floorz;

            for (int i = 0; i < max; i++)
                P_SpawnBloodSplat(mo->x + (M_BigRandomInt(-radius, radius) << FRACBITS),
                    mo->y + (M_BigRandomInt(-radius, radius) << FRACBITS), color, true, floorz, mo);
        }
    }

    // killough 08/11/98: add bouncers
    // killough 09/15/98: add objects falling off ledges
    // killough 11/98: only include bouncers hanging off ledges
    if ((((flags & MF_BOUNCES) && mo->z > mo->dropoffz) || corpse || (flags2 & MF2_FALLING))
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
    else if ((flags2 & MF2_FEETARECLIPPED) && corpse)
    {
        // [BH] increase friction for corpses in water
        mo->momx = FixedMul(mo->momx, WATERFRICTION);
        mo->momy = FixedMul(mo->momy, WATERFRICTION);
    }
    else
    {
        // phares 03/17/98
        //
        // Friction will have been adjusted by friction thinkers for
        // icy or muddy floors. Otherwise it was never touched and
        // remained set at ORIG_FRICTION
        //
        // killough 08/28/98: removed inefficient thinker algorithm,
        // instead using touching_sectorlist in P_GetFriction() to
        // determine friction (and thus only when it is needed).
        //
        // killough 10/98: changed to work with new bobbing method.
        // Reducing player momentum is no longer needed to reduce
        // bobbing, so ice works much better now.
        const fixed_t   friction = P_GetFriction(mo, NULL);

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
    player_t        *player = mo->player;
    const int       flags = mo->flags;
    const fixed_t   floorz = mo->floorz;

    // killough 07/11/98:
    // BFG fireballs bounced on floors and ceilings in Pre-Beta DOOM
    // killough 08/09/98: added support for non-missile objects bouncing
    // (e.g. grenade, mine, pipebomb)
    if ((flags & MF_BOUNCES) && mo->momz)
    {
        mo->z += mo->momz;

        if (mo->z <= floorz)                            // bounce off floors
        {
            mo->z = floorz;

            if (mo->momz < 0)
            {
                mo->momz = -mo->momz;

                if (!(flags & MF_NOGRAVITY))            // bounce back with decay
                {
                    mo->momz = ((flags & MF_FLOAT) ?    // floaters fall slowly
                        ((flags & MF_DROPOFF) ?         // DROPOFF indicates rate
                        FixedMul(mo->momz, (fixed_t)(0.85 * FRACUNIT)) :
                        FixedMul(mo->momz, (fixed_t)(0.70 * FRACUNIT))) :
                        FixedMul(mo->momz, (fixed_t)(0.45 * FRACUNIT)));

                    // Bring it to rest below a certain speed
                    if (ABS(mo->momz) <= mo->info->mass * (GRAVITY * 4 / 256))
                        mo->momz = 0;
                }

                // killough 11/98: touchy objects explode on impact
                if ((flags & MF_TOUCHY) && (mo->flags2 & MF2_ARMED) && mo->health > 0)
                    P_DamageMobj(mo, NULL, NULL, mo->health, true, false);
                else if ((flags & MF_FLOAT) && sentient(mo))
                    goto floater;

                return;
            }
        }
        else if (mo->z >= mo->ceilingz - mo->height)
        {
            // bounce off ceilings
            mo->z = mo->ceilingz - mo->height;

            if (mo->momz > 0)
            {
                if (mo->subsector->sector->ceilingpic != skyflatnum)
                    mo->momz = -mo->momz;               // always bounce off non-sky ceiling
                else if (flags & MF_MISSILE)
                    P_RemoveMobj(mo);                   // missiles don't bounce off skies
                else if (flags & MF_NOGRAVITY)
                    mo->momz = -mo->momz;               // bounce unless under gravity

                if ((flags & MF_FLOAT) && sentient(mo))
                    goto floater;

                return;
            }
        }
        else
        {
            if (!(flags & MF_NOGRAVITY))                // free-fall under gravity
                mo->momz -= mo->info->mass * (GRAVITY / 256);

            if ((flags & MF_FLOAT) && sentient(mo))
                goto floater;

            return;
        }

        // came to a stop
        mo->momz = 0;

        if (flags & MF_MISSILE)
        {
            if (ceilingline && ceilingline->backsector && ceilingline->backsector->ceilingpic == skyflatnum
                && mo->z > ceilingline->backsector->ceilingheight)
                P_RemoveMobj(mo);                       // don't explode on skies
            else
                P_ExplodeMissile(mo);
        }

        if ((flags & MF_FLOAT) && sentient(mo))
            goto floater;

        return;
    }

    // check for smooth step up
    if (player && player->mo == mo && mo->z < floorz && !viewplayer->jumptics)
    {
        player->viewheight -= floorz - mo->z;
        player->deltaviewheight = (VIEWHEIGHT - player->viewheight) >> 3;
    }

    // adjust height
    mo->z += mo->momz;

floater:
    // float down towards target if too close
    if (!((flags ^ MF_FLOAT) & (MF_FLOAT | MF_SKULLFLY | MF_INFLOAT)) && mo->target)
    {
        const fixed_t   delta = (mo->target->z + (mo->height >> 1) - mo->z) * 3;

        if (P_ApproxDistance(mo->x - mo->target->x, mo->y - mo->target->y) < ABS(delta))
            mo->z += SIGN(delta) * FLOATSPEED;
    }

    // clip movement
    if (mo->z <= floorz)
    {
        // [BH] remove blood the moment it hits the ground and spawn blood splats in its place
        if (mo->type == MT_BLOOD)
        {
            P_RemoveBloodMobj(mo);

            if (r_bloodsplats_max)
            {
                if (flags & MF_FUZZ)
                    P_SpawnBloodSplat(mo->x, mo->y, FUZZYBLOOD, false, 0, NULL);
                else if (mo->bloodcolor > 0)
                {
                    const fixed_t   x = mo->x;
                    const fixed_t   y = mo->y;
                    const fixed_t   x1 = M_BigRandomInt(-5, 5) << FRACBITS;
                    const fixed_t   y1 = M_BigRandomInt(-5, 5) << FRACBITS;
                    const fixed_t   x2 = M_BigRandomIntNoRepeat(-5, 5, x1) << FRACBITS;
                    const fixed_t   y2 = M_BigRandomIntNoRepeat(-5, 5, y1) << FRACBITS;
                    const int       bloodcolor = colortranslation[mo->bloodcolor - 1][REDBLOODSPLATCOLOR];

                    P_SpawnBloodSplat(x, y, bloodcolor, false, 0, NULL);
                    P_SpawnBloodSplat(x + x1, y + y1, bloodcolor, false, 0, NULL);
                    P_SpawnBloodSplat(x - x2, y - y2, bloodcolor, false, 0, NULL);
                }
            }

            return;
        }

        // hit the floor
        if (flags & MF_SKULLFLY)
            mo->momz = -mo->momz;       // the skull slammed into something

        if (mo->momz < 0)
        {
            // killough 11/98: touchy objects explode on impact
            if ((flags & MF_TOUCHY) && (mo->flags2 & MF2_ARMED) && mo->health > 0)
                P_DamageMobj(mo, NULL, NULL, mo->health, true, false);
            else if (player && player->mo == mo)
            {
                player->jumptics = 7;

                if (weaponbounce && !freeze)
                    player->bouncemax = MAX(MINBOUNCEMAX, mo->momz) * 3 / 4;

                if (mo->momz < -GRAVITY * 8)
                {
                    // Squat down.
                    // Decrease viewheight for a moment after hitting the ground (hard), and utter appropriate sound.
                    player->deltaviewheight = mo->momz / 8;

                    if (mo->health > 0 && !(viewplayer->cheats & CF_NOCLIP) && !freeze)
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
    else if (mo->mbf21flags & MF_MBF21_LOGRAV)
    {
        if (!mo->momz)
            mo->momz = -(GRAVITY >> 3) * 2;
        else
            mo->momz -= GRAVITY >> 3;
    }
    else if (!(flags & MF_NOGRAVITY))
    {
        // still above the floor
        if (!mo->momz)
            mo->momz = -GRAVITY * 2;
        else
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
    fixed_t         x = mobj->spawnpoint.x << FRACBITS;
    fixed_t         y = mobj->spawnpoint.y << FRACBITS;
    const fixed_t   z = ((mobj->flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ);
    mobj_t          *mo;
    mapthing_t      *mthing = &mobj->spawnpoint;

    // [BH] Fix <https://doomwiki.org/wiki/(0,0)_respawning_bug>.
    if (!x && !y)
    {
        x = mobj->x;
        y = mobj->y;
    }

    // something is occupying it's position?
    if (!P_CheckPosition(mobj, x, y))
        return; // no respawn

    // spawn a teleport fog at the old spot
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
    mo->flags = ((mo->flags & ~MF_FRIEND) | (mobj->flags & MF_FRIEND));

    mo->reactiontime = 18;

    // remove the old monster
    P_RemoveMobj(mobj);

    if (con_obituaries)
        C_PlayerMessage("%s dead%s%s has respawned.",
            ((mo->flags & MF_FRIEND) && monstercount[mo->type] == 1 ? "The" : "A"),
            ((mo->flags & MF_FRIEND) ? ", friendly " : " "),
            (*mo->info->name1 ? mo->info->name1 : "monster"));

    viewplayer->respawncount++;
    stat_monstersrespawned = SafeAdd(stat_monstersrespawned, 1);
}

//
// P_MobjThinker
//
void P_MobjThinker(mobj_t *mobj)
{
    const int   flags = mobj->flags;
    int         flags2;
    player_t    *player = mobj->player;
    sector_t    *sector = mobj->subsector->sector;

    // [AM] Handle interpolation unless we're an active player.
    if (mobj->interpolate == -1 || mobj->type == MT_FIRE)
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

        if (mobj->thinker.function == &P_RemoveThinkerDelayed)
            return;
    }

    flags2 = mobj->flags2;

    // [BH] bob objects in liquid
    if ((flags2 & MF2_FEETARECLIPPED) && !(flags2 & MF2_NOLIQUIDBOB)
        && mobj->z <= sector->floorheight && !sector->heightsec && r_liquid_bob)
        mobj->z += animatedliquiddiffs[((mobj->floatbob + animatedtic) & (ANIMATEDLIQUIDDIFFS - 1))];
    else if (mobj->z != mobj->floorz || mobj->momz)
    {
        if ((flags2 & MF2_PASSMOBJ) && !infiniteheight && !compat_nopassover)
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
                    player->deltaviewheight = mobj->momz / 8;

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

        if (mobj->thinker.function == &P_RemoveThinkerDelayed)
            return;
    }
    else if (!(mobj->momx | mobj->momy) && !sentient(mobj))
    {
        mobj->flags2 |= MF2_ARMED;  // arm a mine which has come to rest

        // killough 09/12/98: objects fall off ledges if they are hanging off slightly.
        // push off of ledge if hanging more than halfway off
        if (((flags & MF_CORPSE) || (flags & MF_DROPPED) || mobj->type == MT_BARREL)
            && mobj->geartime > 0 && mobj->z - mobj->dropoffz > 2 * FRACUNIT)
            P_ApplyTorque(mobj);
        else
        {
            // Reset torque
            mobj->flags2 &= ~MF2_FALLING;
            mobj->gear = 0;
        }
    }

    if ((sector->special & KILL_MONSTERS_MASK) && mobj->z == mobj->floorz
        && !player && (flags & MF_SHOOTABLE) && !(flags & MF_FLOAT))
    {
        P_DamageMobj(mobj, NULL, NULL, 10000, false, false);

        if (mobj->thinker.function == &P_RemoveThinkerDelayed)
            return;
    }

    // cycle through states, calling action functions at transitions
    if (mobj->tics != -1)
    {
        if (!--mobj->tics)
            P_SetMobjState(mobj, mobj->state->nextstate);
    }
    else
    {
        // check for nightmare respawn
        if ((flags & MF_COUNTKILL) && (gameskill == sk_nightmare || respawnmonsters)
            && ++mobj->movecount >= 12 * TICRATE && !(maptime & 31) && M_Random() <= 4)
            P_NightmareRespawn(mobj);
    }
}

//
// P_SetShadowColumnFunction
//
void P_SetShadowColumnFunction(mobj_t *mobj)
{
    if ((mobj->flags & MF_FUZZ) && r_textures)
        mobj->shadowcolfunc = (r_shadows_translucency ? &R_DrawFuzzyShadowColumn : &R_DrawSolidFuzzyShadowColumn);
    else
        mobj->shadowcolfunc = (r_shadows_translucency ? &R_DrawShadowColumn : &R_DrawSolidShadowColumn);
}

//
// P_SpawnMobj
//
mobj_t *P_SpawnMobj(const fixed_t x, const fixed_t y, const fixed_t z, const mobjtype_t type)
{
    mobj_t      *mobj = Z_Calloc(1, sizeof(*mobj), PU_LEVEL, NULL);
    mobjinfo_t  *info = &mobjinfo[type];
    state_t     *st = &states[info->spawnstate];
    sector_t    *sector;

    mobj->type = type;
    mobj->info = info;
    mobj->x = mobj->oldx = x;
    mobj->y = mobj->oldy = y;
    mobj->radius = info->radius;
    mobj->flags = info->flags;
    mobj->flags2 = info->flags2;
    mobj->mbf21flags = info->mbf21flags;
    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    // do not set the state with P_SetMobjState,
    // because action routines cannot be called yet
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    mobj->colfunc = info->colfunc;
    mobj->altcolfunc = info->altcolfunc;
    mobj->id = -1;
    mobj->shadowoffset = info->shadowoffset;
    mobj->bloodcolor = info->bloodcolor;

    P_SetShadowColumnFunction(mobj);

    // set subsector and/or block links
    P_SetThingPosition(mobj);

    sector = mobj->subsector->sector;
    mobj->dropoffz = mobj->floorz = sector->floorheight;
    mobj->ceilingz = sector->ceilingheight;

    if (z == ONFLOORZ)
    {
        mobj->height = info->height;
        mobj->z = mobj->oldz = mobj->floorz;

        if ((mobj->flags2 & MF2_FOOTCLIP) && !sector->heightsec && P_IsInLiquid(mobj))
            mobj->flags2 |= MF2_FEETARECLIPPED;
    }
    else if (z == ONCEILINGZ)
    {
        mobj->height = (type != MT_KEEN && info->projectilepassheight ? info->projectilepassheight : info->height);
        mobj->z = mobj->oldz = mobj->ceilingz - mobj->height;
        mobj->flags2 &= ~MF2_CASTSHADOW;
    }
    else
    {
        mobj->height = info->height;
        mobj->z = mobj->oldz = z;
    }

    mobj->thinker.function = (type == MT_MUSICSOURCE ? &MusInfoThinker : &P_MobjThinker);
    P_AddThinker(&mobj->thinker);

    return mobj;
}

static mapthing_t   itemrespawnqueue[ITEMQUEUESIZE];
static int          itemrespawntime[ITEMQUEUESIZE];
static int          iqueuehead;
static int          iqueuetail;

//
// P_RemoveMobj
//
void P_RemoveMobj(mobj_t *mobj)
{
    const int   flags = mobj->flags;

    if ((flags & MF_SPECIAL) && !(flags & MF_DROPPED))
    {
        itemrespawnqueue[iqueuehead] = mobj->spawnpoint;
        itemrespawntime[iqueuehead] = maptime;
        iqueuehead = ((iqueuehead + 1) & (ITEMQUEUESIZE - 1));

        // lose one off the end?
        if (iqueuehead == iqueuetail)
            iqueuetail = ((iqueuetail + 1) & (ITEMQUEUESIZE - 1));
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

    // Delete all nodes on the current sector_list
    if (sector_list)
    {
        P_DelSeclist(sector_list);
        sector_list = NULL;
    }

    // free block
    P_RemoveThinker2((thinker_t *)mobj);
}

//
// P_RemoveBloodSplats
//
void P_RemoveBloodSplats(void)
{
    for (int i = 0; i < numsectors; i++)
    {
        bloodsplat_t    *splat = sectors[i].splatlist;

        while (splat)
        {
            bloodsplat_t    *next = splat->next;

            P_UnsetBloodSplatPosition(splat);
            splat = next;
        }
    }

    r_bloodsplats_total = 0;
}

//
// P_FindDoomedNum
// Finds a mobj type with a matching doomednum
// killough 08/24/98: rewrote to use hashing
//
mobjtype_t P_FindDoomedNum(const unsigned int type)
{
    static struct
    {
        int first;
        int next;
    } *hash;

    int i;

    if (!hash)
    {
        hash = Z_Malloc(NUMMOBJTYPES * sizeof(*hash), PU_CACHE, (void **)&hash);

        for (int j = 0; j < NUMMOBJTYPES; j++)
            hash[j].first = NUMMOBJTYPES;

        for (int j = 0; j < NUMMOBJTYPES; j++)
            if (mobjinfo[j].doomednum != -1)
            {
                const int   h = mobjinfo[j].doomednum % NUMMOBJTYPES;

                hash[j].next = hash[h].first;
                hash[h].first = j;
            }
    }

    i = hash[type % NUMMOBJTYPES].first;

    while (i < NUMMOBJTYPES && mobjinfo[i].doomednum != type)
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
    if (iqueuehead == iqueuetail)
        return;

    // wait at least 30 seconds
    if (maptime - itemrespawntime[iqueuetail] < 30 * TICRATE)
        return;

    mthing = &itemrespawnqueue[iqueuetail];

    // find which type to spawn
    // killough 08/23/98: use table for faster lookup
    i = P_FindDoomedNum(mthing->type);

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ((mobjinfo[i].flags & MF_SPAWNCEILING) ? ONCEILINGZ : ((mobjinfo[i].flags2 & MF2_FLOATBOB) ? 14 * FRACUNIT : ONFLOORZ));

    // spawn a teleport fog at the new spot
    mo = P_SpawnMobj(x, y, z, MT_IFOG);
    S_StartSound(mo, sfx_itmbk);

    // spawn it
    mo = P_SpawnMobj(x, y, z, i);
    mo->spawnpoint = *mthing;
    mo->angle = ANG45 * (mthing->angle / 45);

    if (con_obituaries)
        C_PlayerMessage("%s %s has respawned.", (isvowel(mo->info->name1[0]) ? "An" : "A"), mo->info->name1);

    // pull it from the queue
    iqueuetail = ((iqueuetail + 1) & (ITEMQUEUESIZE - 1));
}

//
// P_SetPlayerViewHeight
//
void P_SetPlayerViewHeight(void)
{
    mobj_t  *mo = viewplayer->mo;

    for (const struct msecnode_s *seclist = mo->touching_sectorlist; seclist; seclist = seclist->m_tnext)
        if (seclist->m_sector->floorheight + mo->height < seclist->m_sector->ceilingheight)
            mo->z = MAX(mo->z, seclist->m_sector->floorheight);

    mo->floorz = mo->z;

    viewplayer->viewheight = VIEWHEIGHT;
    viewplayer->viewz = viewplayer->oldviewz = mo->z + viewplayer->viewheight;

    if ((mo->flags2 & MF2_FEETARECLIPPED) && r_liquid_lowerview)
        viewplayer->viewz -= FOOTCLIPSIZE;
}

//
// P_SpawnPlayer
// Called when the player is spawned in the level.
// Most of the player structure stays unchanged between levels.
//
static void P_SpawnPlayer(const mapthing_t *mthing)
{
    mobj_t  *mobj;

    if (viewplayer->playerstate == PST_REBORN)
        G_PlayerReborn();

    mobj = P_SpawnMobj(mthing->x << FRACBITS, mthing->y << FRACBITS, ONFLOORZ, MT_PLAYER);

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

    viewplayer->psprites[ps_weapon].sx = 0;
    viewplayer->mo->momx = 0;
    viewplayer->mo->momy = 0;
    viewplayer->mo->bloodsplats = CORPSEBLOODSPLATS;
    viewplayer->mo->floatbob = (M_BigRandom() & 63);
    viewplayer->momx = 0;
    viewplayer->momy = 0;
    viewplayer->lookdir = 0;
    viewplayer->recoil = 0;
    viewplayer->jumptics = 0;
    viewplayer->bounce = 0;
    viewplayer->bouncemax = 0;

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
void P_SpawnMoreBlood(mobj_t *mobj)
{
    if (mobj->bloodcolor > 0)
    {
        const int       bloodcolor = colortranslation[mobj->bloodcolor - 1][REDBLOODSPLATCOLOR];
        const int       radius = ((spritewidth[sprites[mobj->sprite].spriteframes[0].lump[0]] >> FRACBITS) >> 1) + 12;
        const int       max = M_BigRandomInt(150, 200) + radius;
        const fixed_t   floorz = mobj->floorz;
        fixed_t         x = mobj->x;
        fixed_t         y = mobj->y;

        if (!(mobj->flags & MF_SPAWNCEILING))
        {
            x += (M_BigRandomInt(-radius / 3, radius / 3) << FRACBITS);
            y += (M_BigRandomInt(-radius / 3, radius / 3) << FRACBITS);
        }

        for (int j = 0; j < max; j++)
        {
            angle_t angle;
            fixed_t fx, fy;

            if (!mobj->bloodsplats)
                break;

            angle = M_BigRandomInt(0, FINEANGLES - 1);
            fx = x + FixedMul(M_BigRandomInt(0, radius) << FRACBITS, finecosine[angle]);
            fy = y + FixedMul(M_BigRandomInt(0, radius) << FRACBITS, finesine[angle]);

            P_SpawnBloodSplat(fx, fy, bloodcolor, true, floorz, mobj);
        }
    }
}

//
// P_SpawnMapThing
// The fields of the mapthing should already be in host byte order.
//
int prevthingx, prevthingy;
int prevthingbob;

mobj_t *P_SpawnMapThing(mapthing_t *mthing, const bool spawnmonsters)
{
    mobjtype_t  i;
    mobj_t      *mobj;
    fixed_t     x, y;
    short       type = mthing->type;
    short       options = mthing->options;
    int         flags;
    int         flags2;
    int         musicid = 0;
    mobjinfo_t  *info;

    // killough 11/98: clear flags unused by DOOM
    //
    // We clear the flags unused in DOOM if we see flag mask 256 set, since
    // it is reserved to be 0 under the new scheme. A 1 in this reserved bit
    // indicates it's a DOOM WAD made by a DOOM editor which puts 1's in
    // bits that weren't used in DOOM (such as HellMaker wads). So we should
    // then simply ignore all upper bits.
    if (options & MTF_RESERVED)
        options &= (MTF_EASY | MTF_NORMAL | MTF_HARD | MTF_AMBUSH | MTF_NOTSINGLE);

    // check for players specially
    if (type == Player1Start)
    {
        P_SpawnPlayer(mthing);
        viewplayer->mo->id = thingid;
        return NULL;
    }

    if ((type >= Player2Start && type <= Player4Start) || type == PlayerDeathmatchStart)
        return NULL;

    if (type == VisualModeCamera)
        return NULL;

    if (type >= MusicSourceMin && type <= MusicSourceMax)
    {
        musicid = type - MusicSourceMin;
        type = MusicSourceMax;
    }

    if ((options & MTF_NOTSINGLE) && !solonet)
        return NULL;

    if (!(options & (gameskill == sk_baby ? 1 : (gameskill == sk_nightmare ? 4 : (1 << (gameskill - 1))))))
        return NULL;

    // killough 08/23/98: use table for faster lookup
    if ((i = P_FindDoomedNum(type)) == NUMMOBJTYPES)
    {
        // [BH] make unknown thing type non-fatal and show console warning instead
        char    *temp = commify(thingid);

        C_Warning(2, "Thing %s at (%i,%i) wasn't spawned because its type is unknown.",
            temp, mthing->x, mthing->y);
        free(temp);

        return NULL;
    }

    // check for appropriate skill level
    if (!(options & (MTF_EASY | MTF_NORMAL | MTF_HARD)) && (!canmodify || !r_fixmaperrors))
    {
        if (*mobjinfo[i].name1)
            C_Warning(2, "The %s at (%i,%i) wasn't spawned because it has no skill flags.",
                mobjinfo[i].name1, mthing->x, mthing->y);
        else
        {
            char    *temp = commify(thingid);

            C_Warning(2, "Thing %s at (%i,%i) wasn't spawned because it has no skill flags.",
                temp, mthing->x, mthing->y);
            free(temp);
        }

        return NULL;
    }

    if (mobjinfo[i].flags & MF_COUNTKILL)
    {
        // don't spawn any monsters if -nomonsters
        if (!spawnmonsters)
            return NULL;

        // killough 07/20/98: exclude friends
        if (!((mobjinfo[i].flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
            totalkills++;

        monstercount[i]++;
    }

    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    mobj = P_SpawnMobj(x, y, ((mobjinfo[i].flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ), i);
    mobj->spawnpoint = *mthing;
    mobj->musicid = musicid;

    flags = mobj->flags;
    flags2 = mobj->flags2;

    numspawnedthings++;

    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;

    if (!(flags & MF_FRIEND) && (options & MTF_FRIEND))
    {
        mobj->flags |= MF_FRIEND;   // killough 10/98
        mbfcompatible = true;
    }

    if (flags & MF_COUNTITEM)
        totalitems++;

    if (flags & MF_SPECIAL)
        totalpickups++;

    if (mobj->tics > 0)
        mobj->tics = (M_BigRandom() % mobj->tics) + 1;

    mobj->angle = ((mthing->angle % 45) ? mthing->angle * (ANG45 / 45) : ANG45 * (mthing->angle / 45));

    // [BH] randomly mirror corpses
    if (flags & MF_CORPSE)
    {
        mobj->geartime = MAXGEARTIME;

        if ((M_BigRandom() & 1) && r_corpses_mirrored)
            mobj->flags2 |= MF2_MIRRORED;
    }

    // [BH] randomly mirror weapons
    if (r_mirroredweapons && (M_BigRandom() & 1) && (type == SuperShotgun || (type >= Shotgun && type <= BFG9000)))
        mobj->flags2 |= MF2_MIRRORED;

    info = mobj->info;
    mobj->bloodsplats = CORPSEBLOODSPLATS;

    // [BH] spawn blood splats around corpses
    if (r_corpses_moreblood
        && r_bloodsplats_max
        && !(flags & (MF_SHOOTABLE | MF_NOBLOOD | MF_SPECIAL))
        && mobj->bloodcolor > 0
        && (!hacx || !(flags2 & MF2_DECORATION))
        && mobj->subsector->sector->terraintype == SOLID)
    {
        const short lump = sprites[mobj->sprite].spriteframes[mobj->frame & FF_FRAMEMASK].lump[0];

        if (moreblood || lumpinfo[firstspritelump + lump]->wadfile->type == IWAD)
            P_SpawnMoreBlood(mobj);
    }

    // [crispy] randomly colorize player corpses
    if (info->spawnstate == S_PLAY_DIE7 || info->spawnstate == S_PLAY_XDIE9)
        mobj->flags |= (M_BigRandomInt(0, 3) << MF_TRANSLATIONSHIFT);

    if (type == Barrel)
    {
        barrelcount++;
        mobj->geartime = MAXGEARTIME;
    }
    else if (flags2 & MF2_DECORATION)
        numdecorations++;

    // [BH] initialize certain mobj's animations to a random start frame
    // so groups of same mobjs are deliberately out of sync with each other
    if (info->frames > 1 && r_randomstartframes)
    {
        const int   numframes = M_BigRandomInt(0, info->frames);
        state_t     *st = mobj->state;

        for (int j = 0; j < numframes && st->nextstate != S_NULL; j++)
            st = &states[st->nextstate];

        mobj->state = st;
    }

    // [BH] set random pitch for monster sounds when spawned
    mobj->pitch = ((flags & MF_SHOOTABLE) && type != Barrel ? NORM_PITCH + M_BigRandomInt(-16, 16) : NORM_PITCH);

    // [BH] initialize bobbing things
    if (!(flags2 & MF2_NOLIQUIDBOB))
    {
        if (x == prevthingx && y == prevthingy)
            mobj->floatbob = prevthingbob;
        else
        {
            mobj->floatbob = (M_BigRandom() & 63);

            prevthingbob = mobj->floatbob;
            prevthingx = x;
            prevthingy = y;
        }
    }

    return mobj;
}

//
// GAME SPAWN FUNCTIONS
//

//
// P_SpawnPuff
//
void P_SpawnPuff(const fixed_t x, const fixed_t y, const fixed_t z, const angle_t angle)
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
    th->tics = MAX(1, st->tics - (M_BigRandom() & 3));
    th->sprite = st->sprite;
    th->frame = st->frame;

    th->colfunc = info->colfunc;
    th->altcolfunc = info->altcolfunc;
    th->id = -1;

    P_SetThingPosition(th);

    sector = th->subsector->sector;
    th->floorz = sector->interpfloorheight;
    th->ceilingz = sector->interpceilingheight;

    th->z = z + (M_BigSubRandom() << 10);

    th->thinker.function = &P_MobjThinker;
    P_AddThinker(&th->thinker);

    // don't make punches spark on the wall
    if (attackrange == MELEERANGE)
        P_SetMobjState(th, S_PUFF3);
}

//
// P_SpawnSmokeTrail
//
void P_SpawnSmokeTrail(const fixed_t x, const fixed_t y, const fixed_t z, const angle_t angle)
{
    mobj_t  *th = P_SpawnMobj(x, y, z + (M_BigSubRandom() << 10), MT_TRAIL);

    th->momz = FRACUNIT / 2;
    th->angle = angle;
    th->flags2 |= (M_Random() & 1) * MF2_MIRRORED;
}

//
// P_SpawnBlood
// [BH] spawn much more blood than Vanilla DOOM
//
void P_SpawnBlood(const fixed_t x, const fixed_t y, const fixed_t z, angle_t angle, const int damage, mobj_t *target)
{
    if (target->bloodcolor)
    {
        const int   minz = target->z;
        const int   maxz = minz + spriteheight[sprites[target->sprite].spriteframes[0].lump[0]];
        mobjinfo_t  *info = &mobjinfo[MT_BLOOD];
        const bool  fuzz = ((target->flags & MF_FUZZ) && r_blood == r_blood_all);
        int         color;
        state_t     *st = &states[info->spawnstate];

        if (!fuzz)
            color = (r_blood == r_blood_red ? REDBLOOD : (r_blood == r_blood_green ? GREENBLOOD : target->bloodcolor));

        angle += ANG180;

        for (int i = (damage >> 2) + 1; i > 0; i--)
        {
            mobj_t      *th = Z_Calloc(1, sizeof(*th), PU_LEVEL, NULL);
            sector_t    *sector;

            th->type = MT_BLOOD;
            th->info = info;
            th->x = x + M_BigRandomInt(-2, 2) * FRACUNIT;
            th->y = y + M_BigRandomInt(-2, 2) * FRACUNIT;
            th->flags = info->flags;
            th->flags2 = (info->flags2 | ((M_Random() & 1) * MF2_MIRRORED));

            th->state = st;
            th->tics = MAX(1, st->tics - (M_BigRandom() & 2));
            th->sprite = st->sprite;
            th->frame = st->frame;

            if (fuzz)
            {
                th->flags |= MF_FUZZ;
                th->colfunc = &R_DrawFuzzColumn;
                th->altcolfunc = &R_DrawFuzzColumn;
            }
            else
            {
                th->colfunc = bloodcolfunc;
                th->altcolfunc = bloodcolfunc;
                th->bloodcolor = color;
            }

            th->id = -1;

            P_SetThingPosition(th);

            sector = th->subsector->sector;
            th->floorz = sector->interpfloorheight;
            th->ceilingz = sector->interpceilingheight;

            th->z = BETWEEN(minz, z + (M_BigSubRandom() << 10), maxz);

            th->thinker.function = &P_MobjThinker;
            P_AddThinker(&th->thinker);

            th->momx = FixedMul(i * FRACUNIT / 4, finecosine[angle >> ANGLETOFINESHIFT]);
            th->momy = FixedMul(i * FRACUNIT / 4, finesine[angle >> ANGLETOFINESHIFT]);
            th->momz = (2 + i / 6) * FRACUNIT;

            th->angle = angle;
            angle += M_BigSubRandom() * 0xB60B60;

            if (damage <= 12 && th->state->nextstate != S_NULL)
                P_SetMobjState(th, th->state->nextstate);

            if (damage < 9 && th->state->nextstate != S_NULL)
                P_SetMobjState(th, th->state->nextstate);
        }
    }
}

//
// P_SetBloodSplatColor
//
void P_SetBloodSplatColor(bloodsplat_t *splat)
{
    if (r_blood == r_blood_nofuzz)
    {
        splat->viscolor = (splat->color == FUZZYBLOOD ? REDBLOODSPLATCOLOR : splat->color) + M_BigRandomInt(-2, 1);
        splat->viscolfunc = bloodsplatcolfunc;
    }
    else if (r_blood == r_blood_all)
    {
        if (splat->color == FUZZYBLOOD && r_textures)
        {
            splat->viscolor = 0;
            splat->viscolfunc = &R_DrawFuzzColumn;
        }
        else
        {
            splat->viscolor = splat->color + M_BigRandomInt(-2, 1);
            splat->viscolfunc = bloodsplatcolfunc;
        }
    }
    else if (r_blood == r_blood_red)
    {
        splat->viscolor = REDBLOODSPLATCOLOR + M_BigRandomInt(-2, 1);
        splat->viscolfunc = bloodsplatcolfunc;
    }
    else
    {
        splat->viscolor = GREENBLOODSPLATCOLOR + M_BigRandomInt(-2, 1);
        splat->viscolfunc = bloodsplatcolfunc;
    }
}

//
// P_SpawnBloodSplat
//
void P_SpawnBloodSplat(const fixed_t x, const fixed_t y, const int color,
    const bool usemaxheight, const fixed_t maxheight, mobj_t *target)
{
    if (r_bloodsplats_total >= r_bloodsplats_max)
        return;
    else
    {
        sector_t    *sec = R_PointInSubsector(x, y)->sector;

        if (sec->terraintype == SOLID && (!usemaxheight || sec->interpfloorheight <= maxheight))
        {
            bloodsplat_t    *splat = malloc(sizeof(*splat));

            if (splat)
            {
                const int   patch = firstbloodsplatlump + (M_BigRandom() & (BLOODSPLATLUMPS - 1));

                splat->patch = firstspritelump + patch;
                splat->color = color;
                P_SetBloodSplatColor(splat);
                splat->x = x;
                splat->y = y;
                splat->angle = M_BigSubRandom() * 0xB60B60;
                splat->width = spritewidth[patch];
                splat->sector = sec;
                P_SetBloodSplatPosition(splat);
                r_bloodsplats_total++;

                if (target && target->bloodsplats)
                    target->bloodsplats--;
            }
        }
    }
}

//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
bool P_CheckMissileSpawn(mobj_t *th)
{
    th->tics = MAX(1, th->tics - (M_Random() & 3));

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx >> 1);
    th->y += (th->momy >> 1);
    th->z += (th->momz >> 1);

    // killough 08/12/98: for non-missile objects (e.g. grenades)
    if (!(th->flags & MF_MISSILE))
        return true;

    // killough 03/15/98: no dropoff (really = don't care for missiles)
    if (!P_TryMove(th, th->x, th->y, 0))
    {
        P_ExplodeMissile(th);
        return false;
    }

    return true;
}

//
// P_SpawnMissile
//
mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type)
{
    fixed_t z = source->z + 32 * FRACUNIT;
    mobj_t  *th;
    angle_t an;
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
        an += (M_SubRandom() << 20);

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    speed = th->info->speed;
    th->momx = FixedMul(speed, finecosine[an]);
    th->momy = FixedMul(speed, finesine[an]);
    th->momz = (dest->z - source->z) / MAX(1, P_ApproxDistance(dest->x - source->x, dest->y - source->y) / speed);
    th->flags2 |= MF2_MONSTERMISSILE;
    P_CheckMissileSpawn(th);

    return th;
}

//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster.
//
mobj_t *P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type)
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
        // killough 08/02/98: prefer autoaiming at enemies
        int mask = MF_FRIEND;

        do
        {
            // see which target is to be aimed at
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, mask);

            if (!linetarget)
            {
                slope = P_AimLineAttack(source, (an += (1 << 26)), 16 * 64 * FRACUNIT, mask);

                if (!linetarget)
                {
                    slope = P_AimLineAttack(source, (an -= (2 << 26)), 16 * 64 * FRACUNIT, mask);

                    if (!linetarget)
                    {
                        an = source->angle;
                        slope = (usemouselook ? PLAYERSLOPE(source->player) : 0);
                    }
                }
            }
        } while (mask && (mask = 0, !linetarget));  // killough 08/02/98
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

    if (type == MT_ROCKET && r_rockettrails && !(th->flags & MF_BOUNCES)
        && viewplayer->readyweapon == wp_missile && !chex && !hacx)
    {
        th->flags2 |= MF2_SMOKETRAIL;
        th->pursuecount = 0;
    }

    A_Recoil(source->player->readyweapon);

    // MBF21: return missile if it's ok
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

//
// MBF21: P_FaceMobj
// Returns true if 'source' needs to turn clockwise, or false if 'source' needs
// to turn counter clockwise. 'delta' is set to the amount 'source' needs to turn.
//
static bool P_FaceMobj(mobj_t *source, mobj_t *target, angle_t *delta)
{
    angle_t         diff;
    const angle_t   angle1 = source->angle;
    const angle_t   angle2 = R_PointToAngle2(source->x, source->y, target->x, target->y);

    if (angle2 > angle1)
    {
        if ((diff = angle2 - angle1) > ANG180)
        {
            *delta = ANGLE_MAX - diff;
            return false;
        }

        *delta = diff;
        return true;
    }
    else if ((diff = angle1 - angle2) > ANG180)
    {
        *delta = ANGLE_MAX - diff;
        return true;
    }

    *delta = diff;
    return false;
}

//
// MBF21: P_SeekerMissile
//
bool P_SeekerMissile(mobj_t *actor, mobj_t **seektarget, angle_t thresh, angle_t turnmax, const bool seekcenter)
{
    int     dir;
    angle_t delta;
    angle_t angle;
    mobj_t  *target = *seektarget;

    if (!target)
        return false;

    if (!(target->flags & MF_SHOOTABLE))
    {
        // Target died
        *seektarget = NULL;
        return false;
    }

    dir = P_FaceMobj(actor, target, &delta);

    if (delta > thresh)
    {
        delta >>= 1;

        if (delta > turnmax)
            delta = turnmax;
    }

    if (dir)
        // Turn clockwise
        actor->angle += delta;
    else
        // Turn counter clockwise
        actor->angle -= delta;

    angle = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
    actor->momy = FixedMul(actor->info->speed, finesine[angle]);

    // Need to seek vertically
    if (actor->z + actor->height < target->z || target->z + target->height < actor->z || seekcenter)
        actor->momz = (target->z + (seekcenter ? target->height / 2 : 0) - actor->z)
            / MAX(1, P_ApproxDistance(target->x - actor->x, target->y - actor->y) / actor->info->speed);

    return true;
}
