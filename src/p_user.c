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

#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "info.h"
#include "m_config.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "s_sound.h"

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
extern int      artifactflash;
extern int      curpos;
extern int      inv_ptr;

void G_RemoveChoppers(void);

//
// Movement
//

//
// P_Thrust
// Moves the given origin along a given angle.
//
void P_Thrust(angle_t angle, fixed_t move)
{
    mobj_t  *mo = viewplayer->mo;

    angle >>= ANGLETOFINESHIFT;

    if (viewplayer->powers[pw_flight] && mo->z > mo->floorz)
    {
        mo->momx += FixedMul(move, finecosine[angle]);
        mo->momy += FixedMul(move, finesine[angle]);
    }
    else if (mo->subsector->sector->special == Friction)
    {
        mo->momx += FixedMul(move >> 2, finecosine[angle]);
        mo->momy += FixedMul(move >> 2, finesine[angle]);
    }
    else
    {
        mo->momx += FixedMul(move, finecosine[angle]);
        mo->momy += FixedMul(move, finesine[angle]);
    }
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
    viewplayer->momx += FixedMul(move, finecosine[angle >>= ANGLETOFINESHIFT]);
    viewplayer->momy += FixedMul(move, finesine[angle]);
}

//
// P_CalcHeight
// Calculate the walking/running height adjustment
//
void P_CalcHeight(void)
{
    mobj_t  *mo = viewplayer->mo;

    if (viewplayer->playerstate == PST_LIVE)
    {
        // Regular movement bobbing
        // (needs to be calculated for gun swing
        // even if not on ground)
        fixed_t momx = viewplayer->momx;
        fixed_t momy = viewplayer->momy;
        fixed_t bob;

        if (momx | momy)
            bob = MAX(MIN((FixedMul(momx, momx) + FixedMul(momy, momy)) >> 2, MAXBOB) * movebob / 100, MAXBOB * stillbob / 400) / 2;
        else
            bob = (MAXBOB * stillbob / 400) / 2;

        if ((viewplayer->mo->flags3 & MF3_FLY) && !onground)
            bob = FRACUNIT / 2;

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

        if (viewplayer->chickentics)
            viewplayer->viewz = mo->z + viewplayer->viewheight - 20 * FRACUNIT;
        else
            viewplayer->viewz = mo->z + viewplayer->viewheight + FixedMul(bob, finesine[(FINEANGLES / 20 * leveltime) & FINEMASK]);
    }
    else
        viewplayer->viewz = mo->z + viewplayer->viewheight;

    if (mo->flags2 & MF2_FEETARECLIPPED)
    {
        dboolean    liquid = true;

        for (const struct msecnode_s *seclist = mo->touching_sectorlist; seclist; seclist = seclist->m_tnext)
            if (!seclist->m_sector->isliquid)
            {
                liquid = false;
                break;
            }

        if (liquid)
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
    }

    viewplayer->viewz = BETWEEN(mo->floorz + 4 * FRACUNIT, viewplayer->viewz, mo->ceilingz - 4 * FRACUNIT);
}

//
// P_MovePlayer
//
void P_MovePlayer(void)
{
    mobj_t      *mo = viewplayer->mo;
    ticcmd_t    *cmd = &viewplayer->cmd;
    signed char forwardmove = cmd->forwardmove;
    signed char sidemove = cmd->sidemove;
    int         fly;

    mo->angle += cmd->angleturn << FRACBITS;
    onground = (mo->z <= mo->floorz || (mo->flags2 & MF2_ONMOBJ));

    // killough 10/98:
    //
    // We must apply thrust to the player and bobbing separately, to avoid
    // anomalies. The thrust applied to bobbing is always the same strength on
    // ice, because the player still "works just as hard" to move, while the
    // thrust applied to the movement varies with 'movefactor'.
    if ((forwardmove | sidemove) && (onground || (mo->flags3 & MF3_FLY)))
    {
        int     friction;
        int     movefactor = P_GetMoveFactor(mo, &friction);
        angle_t angle = mo->angle;

        // killough 11/98:
        // On sludge, make bobbing depend on efficiency.
        // On ice, make it depend on effort.
        int     bobfactor = (friction < ORIG_FRICTION ? movefactor : ORIG_FRICTION_FACTOR);

        if (forwardmove)
        {
            P_Bob(angle, forwardmove * bobfactor);
            P_Thrust(angle, forwardmove * movefactor);
        }

        if (sidemove)
        {
            P_Bob((angle -= ANG90), sidemove * bobfactor);
            P_Thrust(angle, sidemove * movefactor);
        }
    }

    viewplayer->lookdir = BETWEEN(-LOOKDIRMAX * MLOOKUNIT, viewplayer->lookdir + cmd->lookdir, LOOKDIRMAX * MLOOKUNIT);

    if (viewplayer->lookdir && !usemouselook)
    {
        if (viewplayer->lookdir > 0)
            viewplayer->lookdir -= 16 * MLOOKUNIT;
        else
            viewplayer->lookdir += 16 * MLOOKUNIT;

        if (ABS(viewplayer->lookdir) < 16 * MLOOKUNIT)
            viewplayer->lookdir = 0;
    }

    if ((fly = cmd->lookfly >> 4) > 7)
        fly -= 16;

    if (fly && viewplayer->powers[pw_flight])
    {
        if (fly != TOCENTER)
        {
            viewplayer->flyheight = fly * 2;

            if (!(mo->flags3 & MF3_FLY))
            {
                mo->flags3 |= MF3_FLY;
                mo->flags |= MF_NOGRAVITY;
            }
        }
        else
        {
            mo->flags3 &= ~MF3_FLY;
            mo->flags &= ~MF_NOGRAVITY;
        }
    }
    else if (fly > 0)
        P_PlayerUseArtifact(arti_fly);

    if (mo->flags3 & MF3_FLY)
    {
        mo->momz = viewplayer->flyheight * FRACUNIT;

        if (viewplayer->flyheight)
            viewplayer->flyheight /= 2;
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
    idlemotorspeed = 0;
    freeze = false;
    infight = infighting;

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

    if (((viewplayer->cmd.buttons & BT_USE) || gamekeydown[KEY_ENTER]
        || ((viewplayer->cmd.buttons & BT_ATTACK) && !viewplayer->damagecount && deathcount > TICRATE * 2)))
    {
        deathcount = 0;
        damagevibrationtics = 1;
        viewplayer->playerstate = PST_REBORN;
        facingkiller = false;
        skipaction = true;
        inv_ptr = 0;
        curpos = 0;
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
    thing = P_SpawnMobj(viewx + 20 * viewcos, viewy + 20 * viewsin, ONFLOORZ, (gamemission == heretic ? HMT_TFOG : MT_TFOG));
    thing->angle = viewangle;
    S_StartSound(thing, sfx_telept);

    // telefrag anything in this spot
    P_TeleportMove(thing, thing->x, thing->y, thing->z, true);

    // respawn the player
    thing = P_SpawnMobj(viewx, viewy, ONFLOORZ, playermobjtype);
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
    ammotype_t  ammotype = wpnlev1info[newweapon].ammotype;

    if (gamemission == heretic)
    {
        if (newweapon == wp_staff && viewplayer->weaponowned[wp_gauntlets] && viewplayer->readyweapon != wp_gauntlets)
            newweapon = (weapontype_t)wp_gauntlets;

        if ((ammotype == am_noammo || viewplayer->ammo[ammotype] >= wpnlev1info[newweapon].minammo)
            && viewplayer->weaponowned[newweapon] && newweapon != viewplayer->readyweapon)
            viewplayer->pendingweapon = newweapon;

        return;
    }

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
    else if (ammotype != am_noammo && viewplayer->ammo[ammotype] < wpnlev1info[newweapon].minammo)
        newweapon = wp_nochange;

    // Select the preferred shotgun.
    else if (newweapon == wp_shotgun)
    {
        if ((!viewplayer->weaponowned[wp_shotgun] || viewplayer->readyweapon == wp_shotgun)
            && viewplayer->weaponowned[wp_supershotgun] && viewplayer->ammo[am_shell] >= 2)
            viewplayer->preferredshotgun = wp_supershotgun;
        else if (viewplayer->readyweapon == wp_supershotgun
            || (viewplayer->preferredshotgun == wp_supershotgun && viewplayer->ammo[am_shell] == 1))
            viewplayer->preferredshotgun = wp_shotgun;

        newweapon = viewplayer->preferredshotgun;
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

void P_ChickenPlayerThink(void)
{
    mobj_t  *mo;

    if (viewplayer->health > 0)
        P_UpdateBeak(NULL, viewplayer, &viewplayer->psprites[ps_weapon]);   // Handle beak movement

    if (viewplayer->chickentics & 15)
        return;

    mo = viewplayer->mo;

    if (!(mo->momx + mo->momy) && M_Random() < 160)
        mo->angle += M_NegRandom() << 19;                                   // Twitch view angle

    if (mo->z <= mo->floorz && M_Random() < 32)
    {
        mo->momz += FRACUNIT;                                               // Jump and noise
        P_SetMobjState(mo, HS_CHICPLAY_PAIN);
        return;
    }

    if (M_Random() < 48)
        S_StartSound(mo, hsfx_chicact);                                     // Just noise
}

dboolean P_UndoPlayerChicken(void)
{
    mobj_t          *fog;
    mobj_t          *mo;
    mobj_t          *pmo = viewplayer->mo;
    fixed_t         x = pmo->x;
    fixed_t         y = pmo->y;
    fixed_t         z = pmo->z;
    angle_t         angle = pmo->angle;
    weapontype_t    weapon = pmo->special1.i;
    int             oldflags = pmo->flags;
    int             oldflags2 = pmo->flags2;
    int             oldflags3 = pmo->flags3;

    P_SetMobjState(pmo, HS_FREETARGMOBJ);
    mo = P_SpawnMobj(x, y, z, HMT_PLAYER);

    if (!P_TestMobjLocation(mo))
    {
        // Didn't fit
        P_RemoveMobj(mo);
        mo = P_SpawnMobj(x, y, z, HMT_CHICPLAYER);
        mo->angle = angle;
        mo->health = viewplayer->health;
        mo->special1.i = weapon;
        mo->player = viewplayer;
        mo->flags = oldflags;
        mo->flags2 = oldflags2;
        mo->flags3 = oldflags3;
        viewplayer->mo = mo;
        viewplayer->chickentics = 2 * 35;
        return false;
    }

    mo->angle = angle;
    mo->player = viewplayer;
    mo->reactiontime = 18;

    if (oldflags3 & MF3_FLY)
    {
        mo->flags3 |= MF3_FLY;
        mo->flags |= MF_NOGRAVITY;
    }

    viewplayer->chickentics = 0;
    viewplayer->powers[pw_weaponlevel2] = 0;
    viewplayer->health = mo->health = MAXHEALTH;
    viewplayer->mo = mo;
    angle >>= ANGLETOFINESHIFT;
    fog = P_SpawnMobj(x + 20 * finecosine[angle], y + 20 * finesine[angle], z + TELEFOGHEIGHT, HMT_TFOG);
    S_StartSound(fog, hsfx_telept);
    P_PostChickenWeapon(weapon);
    return true;
}

//
// P_PlayerThink
//
void P_PlayerThink(void)
{
    ticcmd_t    *cmd = &viewplayer->cmd;
    mobj_t      *mo = viewplayer->mo;
    static int  motionblur;

    if (viewplayer->bonuscount)
        viewplayer->bonuscount--;

    if (consoleactive)
        return;

    // [AM] Assume we can interpolate at the beginning of the tic.
    mo->interpolate = true;

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

    if (viewplayer->chickentics)
        P_ChickenPlayerThink();

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
        mo->health = viewplayer->health = MIN(viewplayer->health + 1, initial_health);

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

    if (cmd->arti)
    {
        // Use an artifact
        if (cmd->arti == 0xFF)
            P_PlayerNextArtifact();
        else
            P_PlayerUseArtifact(cmd->arti);
    }

    // Check for weapon change.

    // A special event has no other buttons.
    if (cmd->buttons & BT_SPECIAL)
        cmd->buttons = 0;

    if ((cmd->buttons & BT_CHANGE) && (!automapactive || am_followmode))
        P_ChangeWeapon((cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT);

    if (autouse && !(leveltime % TICRATE))
    {
        autousing = true;
        P_UseLines();
        autousing = false;
    }

    // check for use
    if (cmd->buttons & BT_USE)
    {
        if (!viewplayer->usedown)
        {
            P_UseLines();
            viewplayer->usedown = true;
        }
    }
    else
        viewplayer->usedown = false;

    // Chicken counter
    if (viewplayer->chickentics)
    {
        if (viewplayer->chickenpeck)
            viewplayer->chickenpeck -= 3;   // Chicken attack counter

        if (!--viewplayer->chickentics)
            P_UndoPlayerChicken();          // Attempt to undo the chicken
    }

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

    if (viewplayer->powers[pw_flight])
        if (!--viewplayer->powers[pw_flight])
        {
            viewplayer->mo->flags3 &= ~MF3_FLY;
            viewplayer->mo->flags &= ~MF_NOGRAVITY;
        }

    if (viewplayer->powers[pw_weaponlevel2])
    {
        if (!--viewplayer->powers[pw_weaponlevel2])
        {
            if (viewplayer->readyweapon == wp_phoenixrod
                && viewplayer->psprites[ps_weapon].state != &states[HS_PHOENIXREADY]
                && viewplayer->psprites[ps_weapon].state != &states[HS_PHOENIXUP])
            {
                P_SetPsprite(ps_weapon, HS_PHOENIXREADY);
                viewplayer->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
                viewplayer->refire = 0;
            }
            else if (viewplayer->readyweapon == wp_gauntlets || viewplayer->readyweapon == wp_staff)
                viewplayer->pendingweapon = viewplayer->readyweapon;
        }
    }

    P_ReduceDamageCount();

    // Handling colormaps.
    if (viewplayer->powers[pw_invulnerability] > STARTFLASHING || (viewplayer->powers[pw_invulnerability] & 8))
        viewplayer->fixedcolormap = INVERSECOLORMAP;
    else
        viewplayer->fixedcolormap = (viewplayer->powers[pw_infrared] > STARTFLASHING || (viewplayer->powers[pw_infrared] & 8));
}

void P_ArtiTele(void)
{
    P_TeleportMove(viewplayer->mo, playerstart.x << FRACBITS, playerstart.y << FRACBITS, ANG45 * (playerstart.angle / 45), false);
    S_StartSound(NULL, hsfx_wpnup);
}

void P_PlayerNextArtifact(void)
{
    if (--inv_ptr < 6)
        curpos = MAX(0, curpos - 1);

    if (inv_ptr < 0)
    {
        inv_ptr = viewplayer->inventoryslotnum - 1;
        curpos = MIN(inv_ptr, 6);
    }

    viewplayer->readyartifact = viewplayer->inventory[inv_ptr].type;
}

void P_PlayerRemoveArtifact(int slot)
{
    viewplayer->artifactcount--;

    if (!--viewplayer->inventory[slot].count)
    {
        // Used last of a type - compact the artifact list
        viewplayer->readyartifact = arti_none;
        viewplayer->inventory[slot].type = arti_none;

        for (int i = slot + 1; i < viewplayer->inventoryslotnum; i++)
            viewplayer->inventory[i - 1] = viewplayer->inventory[i];

        viewplayer->inventoryslotnum--;

        // Set position markers and get next readyArtifact
        if (--inv_ptr < 6)
            curpos = MAX(0, curpos - 1);

        inv_ptr = BETWEEN(0, inv_ptr, viewplayer->inventoryslotnum - 1);
        viewplayer->readyartifact = viewplayer->inventory[inv_ptr].type;
    }
}

void P_PlayerUseArtifact(artitype_t arti)
{
    for (int i = 0; i < viewplayer->inventoryslotnum; i++)
        if (viewplayer->inventory[i].type == arti)
        {
            // Found match - try to use
            if (P_UseArtifact(arti))
            {
                // Artifact was used - remove it from inventory
                P_PlayerRemoveArtifact(i);
                S_StartSound(NULL, hsfx_artiuse);
                artifactflash = 4;
            }
            else
                // Unable to use artifact, advance pointer
                P_PlayerNextArtifact();

            break;
        }
}

dboolean P_UseArtifact(artitype_t arti)
{
    mobj_t *mo;
    angle_t angle;

    switch (arti)
    {
        case arti_invulnerability:
            if (!P_GivePower(pw_invulnerability))
                return false;

            break;

        case arti_invisibility:
            if (!P_GivePower(pw_invisibility))
                return false;

            break;

        case arti_health:
            if (!P_GiveBody(25, true))
                return false;

            break;

        case arti_superhealth:
            if (!P_GiveBody(100, true))
                return false;

            break;

        case arti_tomeofpower:
            if (viewplayer->chickentics)
            {
                // Attempt to undo chicken
                if (!P_UndoPlayerChicken())
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 10000, false);
                else
                {
                    viewplayer->chickentics = 0;
                    S_StartSound(viewplayer->mo, hsfx_wpnup);
                }
            }
            else
            {
                if (!P_GivePower(pw_weaponlevel2))
                    return false;

                if (viewplayer->readyweapon == wp_staff)
                    P_SetPsprite(ps_weapon, HS_STAFFREADY2_1);
                else if (viewplayer->readyweapon == wp_gauntlets)
                    P_SetPsprite(ps_weapon, HS_GAUNTLETREADY2_1);
            }

            break;

        case arti_torch:
            if (!P_GivePower(pw_infrared))
                return false;

            break;

        case arti_firebomb:
            angle = viewplayer->mo->angle >> ANGLETOFINESHIFT;
            mo = P_SpawnMobj(viewplayer->mo->x + 24 * finecosine[angle],
                viewplayer->mo->y + 24 * finesine[angle],
                viewplayer->mo->z - 15 * FRACUNIT * (viewplayer->mo->flags2 & MF2_FEETARECLIPPED),
                HMT_FIREBOMB);
            mo->target = viewplayer->mo;
            break;

        case arti_egg:
            mo = viewplayer->mo;
            P_SpawnPlayerMissile(mo, HMT_EGGFX);
            P_SPMAngle(mo, HMT_EGGFX, mo->angle - (ANG45 / 6));
            P_SPMAngle(mo, HMT_EGGFX, mo->angle + (ANG45 / 6));
            P_SPMAngle(mo, HMT_EGGFX, mo->angle - (ANG45 / 3));
            P_SPMAngle(mo, HMT_EGGFX, mo->angle + (ANG45 / 3));
            break;

        case arti_fly:
            if (!P_GivePower(pw_flight))
                return false;

            break;

        case arti_teleport:
            P_ArtiTele();
            break;

        default:
            return false;
    }

    return true;
}
