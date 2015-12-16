/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "d_event.h"
#include "i_gamepad.h"
#include "m_random.h"
#include "m_menu.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "doomstat.h"

#define LOWERSPEED              6 * FRACUNIT
#define RAISESPEED              6 * FRACUNIT

#define CHAINSAWIDLEMOTORSPEED  15000
#define MAXMOTORSPEED           65535

dboolean        centerweapon = centerweapon_default;

unsigned int    stat_shotsfired = 0;
unsigned int    stat_shotshit = 0;

dboolean        successfulshot;
dboolean        skippsprinterp = false;

void P_CheckMissileSpawn(mobj_t *th);

//
// P_SetPsprite
//
void P_SetPsprite(player_t *player, int position, statenum_t stnum)
{
    pspdef_t    *psp = &player->psprites[position];

    do
    {
        state_t *state;

        if (!stnum)
        {
            // object removed itself
            psp->state = NULL;
            break;
        }

        state = &states[stnum];
        psp->state = state;
        psp->tics = state->tics;        // could be 0

        if (state->misc1)
        {
            // coordinate set
            psp->sx = state->misc1 << FRACBITS;
            psp->sy = state->misc2 << FRACBITS;
        }

        // Call action routine.
        // Modified handling.
        if (state->action)
        {
            state->action(player->mo, player, psp);

            if (!psp->state)
                break;
        }

        stnum = psp->state->nextstate;
    }
    while (!psp->tics); // an initial state of 0 could cycle through
}

//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
void P_BringUpWeapon(player_t *player)
{
    if (player->pendingweapon == wp_nochange)
        player->pendingweapon = player->readyweapon;
    else if (player->pendingweapon == wp_chainsaw)
        S_StartSound(player->mo, sfx_sawup);

    player->psprites[ps_weapon].sy = WEAPONBOTTOM;

    P_SetPsprite(player, ps_weapon, weaponinfo[player->pendingweapon].upstate);

    player->pendingweapon = wp_nochange;
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
dboolean P_CheckAmmo(player_t *player)
{
    weapontype_t        readyweapon = player->readyweapon;
    ammotype_t          ammo = weaponinfo[readyweapon].ammo;
    int                 count = 1;      // Regular.

    // Minimal amount for one shot varies.
    if (readyweapon == wp_bfg)
        count = bfgcells;
    else if (readyweapon == wp_supershotgun)
        count = 2;              // Double barrel.

    // Some do not need ammunition anyway.
    // Return if current ammunition sufficient.
    if (ammo == am_noammo || player->ammo[ammo] >= count)
        return true;

    // Out of ammo, pick a weapon to change to.
    // Preferences are set here.
    if (player->weaponowned[wp_plasma] && player->ammo[am_cell])
        player->pendingweapon = wp_plasma;
    else if (player->weaponowned[wp_supershotgun] && player->ammo[am_shell] >= 2
             && player->preferredshotgun == wp_supershotgun)
        player->pendingweapon = wp_supershotgun;
    else if (player->weaponowned[wp_chaingun] && player->ammo[am_clip])
        player->pendingweapon = wp_chaingun;
    else if (player->weaponowned[wp_shotgun] && player->ammo[am_shell])
        player->pendingweapon = wp_shotgun;
    else if (player->ammo[am_clip])
        player->pendingweapon = wp_pistol;
    else if (player->weaponowned[wp_chainsaw])
        player->pendingweapon = wp_chainsaw;
    else if (player->weaponowned[wp_missile] && player->ammo[am_misl])
        player->pendingweapon = wp_missile;
    else if (player->weaponowned[wp_bfg] && player->ammo[am_cell] >= bfgcells)
        player->pendingweapon = wp_bfg;
    else
        // If everything fails.
        player->pendingweapon = wp_fist;

    return false;
}

void P_SubtractAmmo(player_t *player, int amount)
{
    ammotype_t  ammotype = weaponinfo[player->readyweapon].ammo;

    player->ammo[ammotype] = MAX(0, player->ammo[ammotype] - amount);
}

//
// P_FireWeapon.
//
void P_FireWeapon(player_t *player)
{
    weapontype_t        readyweapon;

    if (!P_CheckAmmo(player) || (automapactive && !am_followmode))
        return;

    readyweapon = player->readyweapon;

    P_SetMobjState(player->mo, S_PLAY_ATK1);
    P_SetPsprite(player, ps_weapon, weaponinfo[readyweapon].atkstate);

    // [BH] no noise alert if not punching a monster
    if (readyweapon == wp_fist && !linetarget)
        return;

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);

    P_NoiseAlert(player->mo, player->mo);

    if (gp_vibrate && vibrate)
    {
        int     motorspeed = weaponinfo[readyweapon].motorspeed;

        if ((readyweapon == wp_fist && player->powers[pw_strength])
            || (readyweapon == wp_chainsaw && linetarget))
            motorspeed = MAXMOTORSPEED;
        XInputVibration(motorspeed);
        weaponvibrationtics = weaponinfo[readyweapon].tics;
    }

    if (centerweapon)
    {
        pspdef_t        *psp = &player->psprites[ps_weapon];
        state_t         *state = psp->state;

        if (!state->misc1)
            psp->sx = 0;
        if (!state->misc2)
            psp->sy = WEAPONTOP;
    }
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon(player_t *player)
{
    P_SetPsprite(player, ps_weapon, weaponinfo[player->readyweapon].downstate);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    weapontype_t        readyweapon;
    weapontype_t        pendingweapon;

    if (!player || !psp)
        return;

    readyweapon = player->readyweapon;
    pendingweapon = player->pendingweapon;

    // get out of attack state
    if (actor->state == &states[S_PLAY_ATK1] || actor->state == &states[S_PLAY_ATK2])
        P_SetMobjState(actor, S_PLAY);

    if (readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
        S_StartSound(actor, sfx_sawidl);

    // check for change
    //  if player is dead, put the weapon away
    if (pendingweapon != wp_nochange || !player->health)
    {
        if (gp_vibrate && vibrate)
            if (pendingweapon == wp_chainsaw)
            {
                idlemotorspeed = CHAINSAWIDLEMOTORSPEED;
                XInputVibration(idlemotorspeed);
            }
            else if (idlemotorspeed)
            {
                idlemotorspeed = 0;
                XInputVibration(idlemotorspeed);
            }

        // change weapon (pending weapon should already be validated)
        P_SetPsprite(player, ps_weapon, weaponinfo[readyweapon].downstate);
        return;
    }

    // check for fire
    //  the missile launcher and bfg do not auto fire
    if (player->cmd.buttons & BT_ATTACK)
    {
        if (!player->attackdown || (readyweapon != wp_missile && readyweapon != wp_bfg))
        {
            player->attackdown = true;
            P_FireWeapon(player);
            return;
        }
    }
    else
        player->attackdown = false;

    if (actor->momx || actor->momy || actor->momz)
    {
        // bob the weapon based on movement speed
        int     angle = (128 * leveltime) & FINEMASK;
        int     bob = player->bob;

        // [BH] smooth out weapon bob by zeroing out really small bobs
        if (bob < FRACUNIT / 2)
            bob = 0;

        psp->sx = FixedMul(bob, finecosine[angle]);
        psp->sy = WEAPONTOP + FixedMul(bob, finesine[angle & (FINEANGLES / 2 - 1)]);
    }
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player)
        return;

    // check for fire
    //  (if a weaponchange is pending, let it go through instead)
    if ((player->cmd.buttons & BT_ATTACK) && player->pendingweapon == wp_nochange
        && player->health)
    {
        player->refire++;
        P_FireWeapon(player);
    }
    else
    {
        player->refire = 0;
        P_CheckAmmo(player);
    }
}

void A_CheckReload(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (player && !P_CheckAmmo(player))
        P_SetPsprite(player, ps_weapon, weaponinfo[player->readyweapon].downstate);
}

//
// A_Lower
// Lowers current weapon,
// and changes weapon at bottom.
//
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player || !psp)
        return;

    psp->sy += LOWERSPEED;

    // Is already down.
    if (psp->sy < WEAPONBOTTOM)
        return;

    // Player is dead.
    if (player->playerstate == PST_DEAD)
    {
        psp->sy = WEAPONBOTTOM;
        return;         // don't bring weapon back up
    }

    // The old weapon has been lowered off the screen,
    // so change the weapon and start raising it
    if (!player->health)
    {
        // Player is dead, so keep the weapon off screen.
        P_SetPsprite(player, ps_weapon, S_NULL);
        return;
    }

    if (player->pendingweapon < NUMWEAPONS)
        player->readyweapon = player->pendingweapon;

    P_BringUpWeapon(player);
}

//
// A_Raise
//
void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player || !psp)
        return;

    psp->sy -= RAISESPEED;

    if (psp->sy > WEAPONTOP)
        return;

    psp->sy = WEAPONTOP;

    startingnewgame = false;

    // The weapon has been raised all the way,
    //  so change to the ready state.
    P_SetPsprite(player, ps_weapon, weaponinfo[player->readyweapon].readystate);
}

//
// A_GunFlash
//
void A_GunFlash(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player)
        return;

    P_SetMobjState(actor, S_PLAY_ATK2);
    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);
}

//
// WEAPON ATTACKS
//

//
// A_Punch
//
void A_Punch(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t     angle;
    int         slope;
    int         damage;

    if (!player)
        return;

    angle = actor->angle + ((P_Random() - P_Random()) << 18);
    slope = P_AimLineAttack(actor, angle, MELEERANGE);
    damage = (P_Random() % 10 + 1) << 1;

    if (player->powers[pw_strength])
        damage *= 10;

    P_LineAttack(actor, angle, MELEERANGE, slope, damage);

    if (!linetarget)
        return;

    S_StartSound(actor, sfx_punch);

    // turn to face target
    actor->angle = R_PointToAngle2(actor->x, actor->y, linetarget->x, linetarget->y);
}

//
// A_Saw
//
void A_Saw(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int         damage = 2 * (P_Random() % 10 + 1);
    angle_t     angle = actor->angle + ((P_Random() - P_Random()) << 18);
    int         slope = P_AimLineAttack(actor, angle, MELEERANGE + 1);

    // use meleerange + 1 so the puff doesn't skip the flash
    P_LineAttack(actor, angle, MELEERANGE + 1, slope, damage);

    if (!linetarget)
    {
        S_StartSound(actor, sfx_sawful);
        return;
    }
    S_StartSound (actor, sfx_sawhit);

    // turn to face target
    angle = R_PointToAngle2(actor->x, actor->y, linetarget->x, linetarget->y);
    if (angle - actor->angle > ANG180)
    {
        if (angle - actor->angle < -ANG90 / 20)
            actor->angle = angle + ANG90 / 21;
        else
            actor->angle -= ANG90 / 20;
    }
    else if (angle - actor->angle > ANG90 / 20)
        actor->angle = angle - ANG90 / 21;
    else
        actor->angle += ANG90 / 20;

    actor->flags |= MF_JUSTATTACKED;
}

//
// A_FireMissile
//
void A_FireMissile(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player)
        return;

    P_SubtractAmmo(player, 1);
    P_SpawnPlayerMissile(player->mo, MT_ROCKET);
}

//
// A_FireBFG
//
void A_FireBFG(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player)
        return;

    P_SubtractAmmo(player, bfgcells);
    P_SpawnPlayerMissile(player->mo, MT_BFG);
}

//
// A_FireOldBFG
//
// This function emulates DOOM's Pre-Beta BFG
// By Lee Killough 6/6/98, 7/11/98, 7/19/98, 8/20/98
//
// This code may not be used in other mods without appropriate credit given.
// Code leeches will be telefragged.

void A_FireOldBFG(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int         type = MT_PLASMA1;

    if (!player)
        return;

    P_SubtractAmmo(player, 1);

    player->extralight = 2;

    do
    {
        mobj_t  *th;
        angle_t an = actor->angle;
        angle_t an1 = ((P_Random() & 127) - 64) * (ANG90 / 768) + an;
        angle_t an2 = ((P_Random() & 127) - 64) * (ANG90 / 640) + ANG90;
        fixed_t slope;

        do
        {
            slope = P_AimLineAttack(actor, an, 16 * 64 * FRACUNIT);
            if (!linetarget)
                slope = P_AimLineAttack(actor, an += 1 << 26, 16 * 64 * FRACUNIT);
            if (!linetarget)
                slope = P_AimLineAttack(actor, an -= 2 << 26, 16 * 64 * FRACUNIT);
            if (!linetarget)
            {
                slope = 0;
                an = actor->angle;
            }
        } while (!linetarget);
        an1 += an - actor->angle;
        an2 += tantoangle[slope >> DBITS];

        th = P_SpawnMobj(actor->x, actor->y, actor->z + 62 * FRACUNIT
            - player->psprites[ps_weapon].sy, type);
        P_SetTarget(&th->target, actor);
        th->angle = an1;
        th->momx = finecosine[an1 >> ANGLETOFINESHIFT] * 25;
        th->momy = finesine[an1 >> ANGLETOFINESHIFT] * 25;
        th->momz = finetangent[an2 >> ANGLETOFINESHIFT] * 25;
        P_CheckMissileSpawn(th);
    } while (type != MT_PLASMA2 && (type = MT_PLASMA2)); // killough: obfuscated!
}

//
// A_FirePlasma
//
void A_FirePlasma(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player)
        return;

    P_SubtractAmmo(player, 1);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate + (P_Random() & 1));

    P_SpawnPlayerMissile(player->mo, MT_PLASMA);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at approximately
// the height of the intended target
//
static fixed_t  bulletslope;

static void P_BulletSlope(mobj_t *mo)
{
    angle_t     an = mo->angle;

    // see which target is to be aimed at
    bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);

    if (!linetarget)
    {
        an += 1 << 26;
        bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
        if (!linetarget)
        {
            an -= 2 << 26;
            bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
        }
    }
}

//
// P_GunShot
//
void P_GunShot(mobj_t *actor, dboolean accurate)
{
    int         damage = 5 * (P_Random() % 3 + 1);
    angle_t     angle = actor->angle;

    if (!accurate)
        angle += (P_Random() - P_Random()) << 18;

    P_LineAttack(actor, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FirePistol
//
void A_FirePistol(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player)
        return;

    S_StartSound(actor, sfx_pistol);

    P_SetMobjState(actor, S_PLAY_ATK2);

    P_SubtractAmmo(player, 1);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    P_GunShot(actor, !player->refire);

    if (successfulshot)
    {
        successfulshot = false;
        player->shotshit++;
        stat_shotshit = SafeAdd(stat_shotshit, 1);
    }
}

//
// A_FireShotgun
//
void A_FireShotgun(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int i;

    if (!player)
        return;

    S_StartSound(actor, sfx_shotgn);
    P_SetMobjState(actor, S_PLAY_ATK2);

    P_SubtractAmmo(player, 1);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    for (i = 0; i < 7; i++)
        P_GunShot(actor, false);

    if (successfulshot)
    {
        successfulshot = false;
        player->shotshit++;
        stat_shotshit = SafeAdd(stat_shotshit, 1);
    }

    player->preferredshotgun = wp_shotgun;
}

//
// A_FireShotgun2
//
void A_FireShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int i;

    if (!player)
        return;

    S_StartSound(actor, sfx_dshtgn);
    P_SetMobjState(actor, S_PLAY_ATK2);

    P_SubtractAmmo(player, 2);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    for (i = 0; i < 20; i++)
    {
        int     damage = 5 * (P_Random() % 3 + 1);
        angle_t angle = actor->angle + ((P_Random() - P_Random()) << ANGLETOFINESHIFT);

        P_LineAttack(actor, angle, MISSILERANGE, bulletslope + ((P_Random() - P_Random()) << 5),
            damage);
    }

    if (successfulshot)
    {
        successfulshot = false;
        player->shotshit++;
        stat_shotshit = SafeAdd(stat_shotshit, 1);
    }

    player->preferredshotgun = wp_supershotgun;
}

void A_OpenShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_dbopn);
}

void A_LoadShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_dbload);
}

void A_CloseShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_dbcls);
    A_ReFire(actor, player, psp);
}

//
// A_FireCGun
//
void A_FireCGun(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!player || !psp)
        return;

    if (player->ammo[weaponinfo[player->readyweapon].ammo])
        S_StartSound(actor, sfx_pistol);
    else
        return;

    P_SetMobjState(actor, S_PLAY_ATK2);

    P_SubtractAmmo(player, 1);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate
        + (unsigned int)((psp->state - &states[S_CHAIN1]) & 1));

    P_BulletSlope(actor);

    successfulshot = false;

    P_GunShot(actor, !player->refire);

    if (successfulshot)
    {
        successfulshot = false;
        player->shotshit++;
        stat_shotshit = SafeAdd(stat_shotshit, 1);
    }
}

void A_Light0(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (player)
        player->extralight = 0;
}

void A_Light1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (player)
        player->extralight = 1;
}

void A_Light2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (player)
        player->extralight = 2;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//
void A_BFGSpray(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int         i;
    mobj_t      *mo = actor->target;

    // offset angles from its attack angle
    for (i = 0; i < 40; i++)
    {
        int     j;
        int     damage = 0;
        angle_t an = actor->angle - ANG90 / 2 + ANG90 / 40 * i;

        P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);

        if (!linetarget)
            continue;

        successfulshot = true;

        P_SpawnMobj(linetarget->x, linetarget->y, linetarget->z + (linetarget->height >> 2),
            MT_EXTRABFG);

        for (j = 0; j < 15; j++)
            damage += (P_Random() & 7) + 1;

        P_DamageMobj(linetarget, mo, mo, damage);
    }

    if (successfulshot)
    {
        successfulshot = false;
        mo->player->shotshit++;
        stat_shotshit = SafeAdd(stat_shotshit, 1);
    }
}

//
// A_BFGsound
//
void A_BFGsound(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_bfg);
}

//
// P_SetupPsprites
// Called at start of level for each player.
//
void P_SetupPsprites(player_t *player)
{
    int i;

    // remove all psprites
    for (i = 0; i < NUMPSPRITES; i++)
        player->psprites[i].state = NULL;

    // spawn the gun
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon(player);
    skippsprinterp = true;
}

//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites(player_t *player)
{
    int         i;
    pspdef_t    *psp = player->psprites;

    for (i = 0; i < NUMPSPRITES; i++, psp++)
        if (psp->state && psp->tics != -1 && !--psp->tics)
            P_SetPsprite(player, i, psp->state->nextstate);

    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}
