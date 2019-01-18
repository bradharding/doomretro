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
#include "g_game.h"
#include "i_gamepad.h"
#include "m_config.h"
#include "m_menu.h"
#include "p_inter.h"
#include "p_local.h"
#include "s_sound.h"

#define AUTOTILTUNIT    20
#define AUTOTILTMAX     300
#define MINSTEPSIZE     (8 * FRACUNIT)
#define MAXSTEPSIZE     (24 * FRACUNIT)

dboolean        autotilt = autotilt_default;
dboolean        autouse = autouse_default;
dboolean        infighting = infighting_default;
int             movebob = movebob_default;
dboolean        r_liquid_lowerview = r_liquid_lowerview_default;
int             r_shake_damage = r_shake_damage_default;
int             stillbob = stillbob_default;

dboolean        autousing = false;
static dboolean onground;
int             deathcount = 0;
int             deadlookdir = -1;

extern fixed_t  animatedliquiddiff;
extern dboolean canmouselook;
extern dboolean skipaction;
extern dboolean usemouselook;
extern int      spindirection;

void G_RemoveChoppers(void);

//
// Movement
//

//
// P_Thrust
// Moves the given origin along a given angle.
//
static void P_Thrust(angle_t angle, fixed_t move)
{
    viewplayer->mo->momx += FixedMul(move, finecosine[(angle >>= ANGLETOFINESHIFT)]);
    viewplayer->mo->momy += FixedMul(move, finesine[angle]);
}

//
// P_Bob
// Same as P_Thrust, but only affects bobbing.
//
// killough 10/98: We apply thrust separately between the real physical player
// and the part which affects bobbing. This way, bobbing only comes from player
// motion, nothing external, avoiding many problems, e.g. bobbing should not
// occur on conveyors, unless the player walks on one, and bobbing should be
// reduced at a regular rate, even on ice (where the player coasts).
//
static void P_Bob(angle_t angle, fixed_t move)
{
    viewplayer->momx += FixedMul(move, finecosine[(angle >>= ANGLETOFINESHIFT)]);
    viewplayer->momy += FixedMul(move, finesine[angle]);
}

//
// P_CalcHeight
// Calculate the walking/running height adjustment
//
void P_CalcHeight(void)
{
    mobj_t  *mo = viewplayer->mo;

    viewplayer->viewz = mo->z + viewplayer->viewheight;

    if (viewplayer->playerstate == PST_LIVE)
    {
        // Regular movement bobbing
        // (needs to be calculated for gun swing
        // even if not on ground)
        fixed_t momx = viewplayer->momx;
        fixed_t momy = viewplayer->momy;
        fixed_t bob = (MAXBOB * stillbob / 400) / 2;

        if (momx | momy)
            bob = MAX(MIN((FixedMul(momx, momx) + FixedMul(momy, momy)) >> 2, MAXBOB) * movebob / 200, bob);

        if (viewplayer->bouncemax)
        {
            viewplayer->bounce -= FRACUNIT;

            if (viewplayer->bounce < viewplayer->bouncemax)
                viewplayer->bounce = -viewplayer->bounce;

            if (!viewplayer->bounce)
                viewplayer->bouncemax = 0;
        }

        // move viewheight
        viewplayer->viewheight += viewplayer->deltaviewheight;

        if (viewplayer->viewheight > VIEWHEIGHT)
        {
            viewplayer->viewheight = VIEWHEIGHT;
            viewplayer->deltaviewheight = 0;
        }

        if (viewplayer->viewheight < VIEWHEIGHT / 2)
        {
            viewplayer->viewheight = VIEWHEIGHT / 2;

            if (viewplayer->deltaviewheight <= 0)
                viewplayer->deltaviewheight = 1;
        }

        if (viewplayer->deltaviewheight)
        {
            viewplayer->deltaviewheight += FRACUNIT / 4;

            if (!viewplayer->deltaviewheight)
                viewplayer->deltaviewheight = 1;
        }

        viewplayer->viewz += FixedMul(bob, finesine[(FINEANGLES / 20 * leveltime) & FINEMASK]);
    }

    if (mo->flags2 & MF2_FEETARECLIPPED)
    {
        if (viewplayer->playerstate == PST_DEAD)
        {
            if (r_liquid_bob)
                viewplayer->viewz += animatedliquiddiff;
        }
        else if (r_liquid_lowerview)
        {
            sector_t    *sector = mo->subsector->sector;

            if (!P_IsSelfReferencingSector(sector)
                && (!sector->heightsec || mo->z + viewplayer->viewheight - FOOTCLIPSIZE >= sector->heightsec->floorheight))
                viewplayer->viewz -= FOOTCLIPSIZE;
        }
    }

    viewplayer->viewz = BETWEEN(mo->floorz + 4 * FRACUNIT, viewplayer->viewz, mo->ceilingz - 4 * FRACUNIT);
}

//
// P_CheckForSteps
//
dboolean P_CheckForSteps(int width)
{
    sector_t    *sector1 = R_PointInSubsector(viewx + width * viewcos, viewy + width * viewsin)->sector;
    sector_t    *sector2 = R_PointInSubsector(viewx + width * 2 * viewcos, viewy + width * 2 * viewsin)->sector;

    if (sector1->terraintype == sector2->terraintype)
    {
        fixed_t step1 = sector1->floorheight;
        fixed_t step2 = sector2->floorheight;
        int     delta1 = step1 - viewplayer->mo->subsector->sector->floorheight;
        int     delta2 = step2 - step1;

        if (delta1 >= MINSTEPSIZE && delta1 <= MAXSTEPSIZE && delta1 == delta2)
        {
            viewplayer->lookdir = MIN(viewplayer->lookdir + AUTOTILTUNIT, AUTOTILTMAX);
            return true;
        }
        else if (delta1 >= -MAXSTEPSIZE && delta1 <= -MINSTEPSIZE && delta1 == delta2)
        {
            viewplayer->lookdir = MAX(-AUTOTILTMAX, viewplayer->lookdir - AUTOTILTUNIT);
            return true;
        }
    }

    return false;
}

// P_MovePlayer
//
void P_MovePlayer(void)
{
    mobj_t      *mo = viewplayer->mo;
    ticcmd_t    *cmd = &viewplayer->cmd;
    signed char forward = cmd->forwardmove;
    signed char side = cmd->sidemove;

    mo->angle += cmd->angleturn << FRACBITS;
    onground = (mo->z <= mo->floorz || (mo->flags2 & MF2_ONMOBJ));

    // killough 10/98:
    //
    // We must apply thrust to the player and bobbing separately, to avoid
    // anomalies. The thrust applied to bobbing is always the same strength on
    // ice, because the player still "works just as hard" to move, while the
    // thrust applied to the movement varies with 'movefactor'.
    if ((forward | side) && onground)
    {
        int     friction;
        int     movefactor = P_GetMoveFactor(mo, &friction);
        angle_t angle = mo->angle;

        // killough 11/98:
        // On sludge, make bobbing depend on efficiency.
        // On ice, make it depend on effort.
        int     bobfactor = (friction < ORIG_FRICTION ? movefactor : ORIG_FRICTION_FACTOR);

        if (forward)
        {
            P_Bob(angle, forward * bobfactor);
            P_Thrust(angle, forward * movefactor);
        }

        if (side)
        {
            P_Bob((angle -= ANG90), side * bobfactor);
            P_Thrust(angle, side * movefactor);
        }
    }

    if (autotilt && !mouselook)
    {
        if (!P_CheckForSteps(32) && !P_CheckForSteps(24))
        {
            if (viewplayer->lookdir > 0)
            {
                if ((viewplayer->lookdir -= AUTOTILTUNIT) < AUTOTILTUNIT)
                    viewplayer->lookdir = 0;
            }
            else
            {
                if ((viewplayer->lookdir += AUTOTILTUNIT) > -AUTOTILTUNIT)
                    viewplayer->lookdir = 0;
            }
        }
    }
    else if (canmouselook)
    {
        if (cmd->lookdir)
            viewplayer->lookdir = BETWEEN(-LOOKDIRMAX * MLOOKUNIT, viewplayer->lookdir + cmd->lookdir, LOOKDIRMAX * MLOOKUNIT);

        if (viewplayer->lookdir && !usemouselook)
        {
            if (viewplayer->lookdir > 0)
            {
                if ((viewplayer->lookdir -= 16 * MLOOKUNIT) < 16 * MLOOKUNIT)
                    viewplayer->lookdir = 0;
            }
            else
            {
                if ((viewplayer->lookdir += 16 * MLOOKUNIT) > -16 * MLOOKUNIT)
                    viewplayer->lookdir = 0;
            }
        }
    }
}

//
// P_ReduceDamageCount
//
static void P_ReduceDamageCount(void)
{
    if (viewplayer->damagecount)
        viewplayer->damagecount--;

    if (r_shake_damage)
        I_UpdateBlitFunc(!!viewplayer->damagecount);
}

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
static void P_DeathThink(void)
{
    static dboolean facingkiller;
    mobj_t          *mo = viewplayer->mo;
    mobj_t          *attacker = viewplayer->attacker;

    weaponvibrationtics = 1;
    idlevibrationstrength = 0;
    freeze = false;
    infight = (infighting && !(viewplayer->cheats & CF_NOTARGET));

    P_MovePsprites();

    // fall to the ground
    if ((onground = (mo->z <= mo->floorz || (mo->flags2 & MF2_ONMOBJ))))
    {
        if (canmouselook)
        {
            static int  inc;

            if (deadlookdir == -1)
            {
                float   viewheightrange = (float)(viewplayer->viewheight - DEADVIEWHEIGHT) / FRACUNIT;

                inc = MAX(1, ABS(DEADLOOKDIR - viewplayer->lookdir));

                if (viewheightrange)
                    inc = (int)(inc / viewheightrange + 0.5);

                if (inc)
                    deadlookdir = DEADLOOKDIR / inc * inc;
            }

            if (viewplayer->lookdir > deadlookdir)
                viewplayer->lookdir -= inc;
            else if (viewplayer->lookdir < deadlookdir)
                viewplayer->lookdir += inc;

            if (ABS(viewplayer->lookdir - deadlookdir) < inc)
                viewplayer->lookdir = deadlookdir;
        }

        if (viewplayer->viewheight > DEADVIEWHEIGHT)
            viewplayer->viewheight -= FRACUNIT;

        if (viewplayer->viewheight < DEADVIEWHEIGHT)
            viewplayer->viewheight = DEADVIEWHEIGHT;
    }

    viewplayer->deltaviewheight = 0;
    P_CalcHeight();

    if (attacker && attacker != mo && !facingkiller)
    {
        angle_t angle = R_PointToAngle2(mo->x, mo->y, attacker->x, attacker->y);
        angle_t delta = angle - mo->angle;

        if (delta < ANG5 || delta > (unsigned int)(-ANG5))
        {
            // Looking at killer, so fade damage flash down.
            mo->angle = angle;
            P_ReduceDamageCount();
            facingkiller = true;
        }
        else
        {
            mo->angle += (delta < ANG180 ? ANG5 : -ANG5);

            if (r_shake_damage)
                I_UpdateBlitFunc(!!viewplayer->damagecount);
        }
    }
    else
        P_ReduceDamageCount();

    if (viewplayer->bonuscount)
        viewplayer->bonuscount--;

    if (consoleactive)
        return;

    if (((viewplayer->cmd.buttons & BT_USE) || gamekeydown[' '] || gamekeydown[KEY_ENTER]
        || ((viewplayer->cmd.buttons & BT_ATTACK) && !viewplayer->damagecount && deathcount > TICRATE * 2)))
    {
        deathcount = 0;
        damagevibrationtics = 1;
        viewplayer->playerstate = PST_REBORN;
        facingkiller = false;
        skipaction = true;
        gamekeydown[' '] = false;
    }
    else
        deathcount++;
}

//
// P_ResurrectPlayer
//
void P_ResurrectPlayer(int health)
{
    mobj_t  *mo = viewplayer->mo;
    mobj_t  *thing;

    // remove player's corpse
    P_RemoveMobj(mo);

    // spawn a teleport fog
    thing = P_SpawnMobj(viewx + 20 * viewcos, viewy + 20 * viewsin, ONFLOORZ, MT_TFOG);
    thing->angle = viewangle;
    S_StartSound(thing, sfx_telept);

    // telefrag anything in this spot
    P_TeleportMove(thing, thing->x, thing->y, thing->z, true);

    // respawn the player
    thing = P_SpawnMobj(viewx, viewy, ONFLOORZ, MT_PLAYER);
    thing->angle = viewangle;
    thing->player = viewplayer;
    thing->health = health;
    thing->reactiontime = 18;
    viewplayer->mo = thing;
    viewplayer->playerstate = PST_LIVE;
    viewplayer->viewheight = VIEWHEIGHT;
    viewplayer->health = health;
    viewplayer->lookdir = 0;
    viewplayer->oldlookdir = 0;
    viewplayer->recoil = 0;
    viewplayer->oldrecoil = 0;
    infight = false;
    P_SetupPsprites();
    P_MapEnd();

    C_HideConsole();
}

void P_ChangeWeapon(weapontype_t newweapon)
{
    ammotype_t  ammotype = weaponinfo[newweapon].ammotype;

    if (newweapon == wp_fist)
    {
        if (viewplayer->readyweapon == wp_fist)
        {
            if (viewplayer->weaponowned[wp_chainsaw])
            {
                newweapon = wp_chainsaw;
                viewplayer->fistorchainsaw = wp_chainsaw;
            }
        }
        else if (viewplayer->readyweapon == wp_chainsaw)
        {
            if (viewplayer->powers[pw_strength])
                viewplayer->fistorchainsaw = wp_fist;
            else
                newweapon = wp_nochange;
        }
        else
            newweapon = viewplayer->fistorchainsaw;
    }

    // Don't switch to a weapon without any or enough ammo.
    else if (ammotype != am_noammo && viewplayer->ammo[ammotype] < weaponinfo[newweapon].minammo)
        newweapon = wp_nochange;

    // Select the preferred shotgun.
    else if (newweapon == wp_shotgun)
    {
        if ((!viewplayer->weaponowned[wp_shotgun]
            || viewplayer->readyweapon == wp_shotgun
            || (viewplayer->preferredshotgun == wp_supershotgun && viewplayer->readyweapon != wp_supershotgun))
            && viewplayer->weaponowned[wp_supershotgun] && viewplayer->ammo[am_shell] >= 2)
            newweapon = viewplayer->preferredshotgun = wp_supershotgun;
        else if (viewplayer->readyweapon == wp_supershotgun
            || (viewplayer->preferredshotgun == wp_supershotgun && viewplayer->ammo[am_shell] == 1))
            newweapon = viewplayer->preferredshotgun = wp_shotgun;
    }

    if (newweapon != wp_nochange && newweapon != viewplayer->readyweapon && viewplayer->weaponowned[newweapon])
    {
        viewplayer->pendingweapon = newweapon;

        if (newweapon == wp_fist && viewplayer->powers[pw_strength])
            S_StartSound(NULL, sfx_getpow);

        if ((viewplayer->cheats & CF_CHOPPERS) && newweapon != wp_chainsaw)
            G_RemoveChoppers();
    }
}

//
// P_PlayerThink
//
void P_PlayerThink(void)
{
    ticcmd_t    *cmd;
    mobj_t      *mo = viewplayer->mo;
    static int  motionblur;

    if (menuactive)
    {
        if (!inhelpscreens)
            mo->angle += ANG1 / 32 * spindirection;

        return;
    }

    cmd = &viewplayer->cmd;

    if (viewplayer->bonuscount)
        viewplayer->bonuscount--;

    if (consoleactive)
        return;

    // [AM] Assume we can interpolate at the beginning of the tic.
    mo->interpolate = 1;

    // [AM] Store starting position for player interpolation.
    mo->oldx = mo->x;
    mo->oldy = mo->y;
    mo->oldz = mo->z;
    mo->oldangle = mo->angle;
    viewplayer->oldviewz = viewplayer->viewz;
    viewplayer->oldlookdir = viewplayer->lookdir;
    viewplayer->oldrecoil = viewplayer->recoil;

    if (viewplayer->cheats & CF_NOCLIP)
        mo->flags |= MF_NOCLIP;
    else
        mo->flags &= ~MF_NOCLIP;

    // chainsaw run forward
    if (mo->flags & MF_JUSTATTACKED)
    {
        cmd->angleturn = 0;
        cmd->forwardmove = 0xC800 / 512;
        cmd->sidemove = 0;
        mo->flags &= ~MF_JUSTATTACKED;
    }

    if (vid_motionblur)
    {
        motionblur = 0;

        if (!automapactive)
        {
            if (viewplayer->damagecount)
                motionblur = MAX(motionblur, 100);
            else
            {
                if (cmd->angleturn)
                    motionblur = MIN(ABS(cmd->angleturn) * 100 / 960, 150);

                if (cmd->lookdir)
                    motionblur = MAX(motionblur, 100);
            }
        }

        I_SetMotionBlur(motionblur * vid_motionblur / 100);
    }
    else if (motionblur)
    {
        motionblur = 0;
        I_SetMotionBlur(0);
    }

    if (viewplayer->recoil)
        viewplayer->recoil -= SIGN(viewplayer->recoil);

    if (viewplayer->playerstate == PST_DEAD)
    {
        P_DeathThink();
        return;
    }

    if (viewplayer->jumptics)
        viewplayer->jumptics--;

    // Move around.
    // Reaction time is used to prevent movement for a bit after a teleport.
    if (mo->reactiontime)
        mo->reactiontime--;
    else
        P_MovePlayer();

    P_CalcHeight();

    if (freeze)
        return;

    // cycle psprites
    P_MovePsprites();

    // [BH] regenerate health up to 100 every 1 second
    if (regenhealth && mo->health < initial_health && !(leveltime % TICRATE) && !viewplayer->damagecount)
        P_GiveBody(1, false);

    // [BH] Check all sectors player is touching are special
    for (const struct msecnode_s *seclist = mo->touching_sectorlist; seclist; seclist = seclist->m_tnext)
        if (seclist->m_sector->special && mo->z == seclist->m_sector->interpfloorheight)
        {
            P_PlayerInSpecialSector();
            break;
        }

    if ((cmd->buttons & BT_JUMP) && (mo->floorz - mo->z <= 8 * FRACUNIT || (mo->flags2 & MF2_ONMOBJ)) && !viewplayer->jumptics)
    {
        mo->momz = JUMPHEIGHT;
        viewplayer->jumptics = 18;
    }
    else if (cmd->buttons & BT_SPECIAL)
        // A special event has no other buttons.
        cmd->buttons = 0;
    else if ((cmd->buttons & BT_CHANGE) && (!automapactive || am_followmode))
        // Check for weapon change.
        P_ChangeWeapon((cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT);

    // check for use
    if (autouse)
    {
        if (!(leveltime % TICRATE))
        {
            autousing = true;
            P_UseLines();
            autousing = false;
        }
    }
    else if (cmd->buttons & BT_USE)
    {
        if (!viewplayer->usedown)
        {
            P_UseLines();
            viewplayer->usedown = true;
        }
    }
    else
        viewplayer->usedown = false;

    // Counters, time dependent power ups.
    if (viewplayer->powers[pw_invulnerability] > 0)
        viewplayer->powers[pw_invulnerability]--;

    if (viewplayer->powers[pw_invisibility] > 0)
        if (!--viewplayer->powers[pw_invisibility])
            viewplayer->mo->flags &= ~MF_FUZZ;

    if (viewplayer->powers[pw_infrared] > 0)
        viewplayer->powers[pw_infrared]--;

    if (viewplayer->powers[pw_ironfeet] > 0)
        viewplayer->powers[pw_ironfeet]--;

    P_ReduceDamageCount();

    // Handling colormaps.
    if (viewplayer->powers[pw_invulnerability] > STARTFLASHING || (viewplayer->powers[pw_invulnerability] & 8))
        viewplayer->fixedcolormap = INVERSECOLORMAP;
    else
        viewplayer->fixedcolormap = (viewplayer->powers[pw_infrared] > STARTFLASHING || (viewplayer->powers[pw_infrared] & 8));
}
