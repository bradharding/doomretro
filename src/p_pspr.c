/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include "d_event.h"
#include "i_gamepad.h"
#include "m_random.h"
#include "m_menu.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"

#define LOWERSPEED              6 * FRACUNIT
#define RAISESPEED              6 * FRACUNIT

#define WEAPONBOTTOM            128 * FRACUNIT
#define WEAPONTOP               32 * FRACUNIT

#define BFGCELLS                40

#define CHAINSAWIDLEMOTORSPEED  7500

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

        // Call action routine.
        // Modified handling.
        if (state->action.acp2)
        {
            state->action.acp2(player, psp);
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
    statenum_t  newstate = weaponinfo[player->pendingweapon].upstate;

    if (player->pendingweapon == wp_nochange)
        player->pendingweapon = player->readyweapon;

    if (player->pendingweapon == wp_chainsaw)
        S_StartSound(player->mo, sfx_sawup);

    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;

    P_SetPsprite(player, ps_weapon, newstate);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
boolean P_CheckAmmo(player_t *player)
{
    ammotype_t  ammo = weaponinfo[player->readyweapon].ammo;
    int         count = 1;       // Regular.

    // Minimal amount for one shot varies.
    if (player->readyweapon == wp_bfg)
        count = BFGCELLS;
    else if (player->readyweapon == wp_supershotgun)
        count = 2;      // Double barrel.

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
    else if (player->weaponowned[wp_bfg] && player->ammo[am_cell] >= BFGCELLS)
        player->pendingweapon = wp_bfg;
    else
        // If everything fails.
        player->pendingweapon = wp_fist;

    // Now set appropriate weapon overlay.
    P_SetPsprite(player, ps_weapon, weaponinfo[player->readyweapon].downstate);

    return false;
}

//
// P_FireWeapon.
//
void P_FireWeapon(player_t *player)
{
    statenum_t          newstate;
    weapontype_t        readyweapon;

    if (!P_CheckAmmo(player) || (automapactive && !followplayer))
        return;

    readyweapon = player->readyweapon;

    P_SetMobjState(player->mo, S_PLAY_ATK1);
    newstate = weaponinfo[readyweapon].atkstate;
    P_SetPsprite(player, ps_weapon, newstate);

    if (readyweapon == wp_fist && !linetarget)
        return;

    P_NoiseAlert(player->mo, player->mo);

    if (gamepadvibrate && vibrate)
    {
        int motorspeed = weaponinfo[readyweapon].motorspeed;

        if (readyweapon == wp_fist && players[consoleplayer].powers[pw_strength])
            motorspeed *= 2;
        XInputVibration(motorspeed);
        weaponvibrationtics = weaponinfo[readyweapon].tics;
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
void A_WeaponReady(player_t *player, pspdef_t *psp)
{
    weapontype_t        readyweapon = player->readyweapon;
    weapontype_t        pendingweapon = player->pendingweapon;

    // get out of attack state
    if (player->mo->state == &states[S_PLAY_ATK1] || player->mo->state == &states[S_PLAY_ATK2])
        P_SetMobjState(player->mo, S_PLAY);

    if (readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
        S_StartSound(player->mo, sfx_sawidl);

    // check for change
    //  if player is dead, put the weapon away
    if (pendingweapon != wp_nochange || !player->health)
    {
        if (gamepadvibrate && vibrate)
        {
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

    if (player->mo->momx || player->mo->momy || player->mo->momz)
    {
        // bob the weapon based on movement speed
        int     angle = (128 * leveltime) & FINEMASK;
        int     bob = player->bob;

        if (bob < FRACUNIT / 2)
            bob = 0;

        psp->sx = FixedMul(bob, finecosine[angle]);
        psp->sy = WEAPONTOP + (bob - FixedMul(bob, finecosine[angle * 2 & (FINEANGLES - 1)])) / 2;
    }
    else
    {
        psp->sx = 0;
        psp->sy = WEAPONTOP;
    }
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire(player_t *player, pspdef_t *psp)
{
    // check for fire
    //  (if a weaponchange is pending, let it go through instead)
    if ((player->cmd.buttons & BT_ATTACK)
        && player->pendingweapon == wp_nochange && player->health)
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

void A_CheckReload(player_t *player, pspdef_t *psp)
{
    P_CheckAmmo(player);
}

//
// A_Lower
// Lowers current weapon,
// and changes weapon at bottom.
//
void A_Lower(player_t *player, pspdef_t *psp)
{
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

    player->readyweapon = player->pendingweapon;

    P_BringUpWeapon(player);
}

//
// A_Raise
//
void A_Raise(player_t *player, pspdef_t *psp)
{
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
void A_GunFlash(player_t *player, pspdef_t *psp)
{
    P_SetMobjState(player->mo, S_PLAY_ATK2);
    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);
}

//
// WEAPON ATTACKS
//

//
// A_Punch
//
void A_Punch(player_t *player, pspdef_t *psp)
{
    int         damage = (P_Random() % 10 + 1) << 1;
    angle_t     angle = player->mo->angle + ((P_Random() - P_Random()) << 18);
    int         slope = P_AimLineAttack(player->mo, angle, MELEERANGE);

    if (player->powers[pw_strength])
        damage *= 10;
    
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);

    if (!linetarget)
        return;

    S_StartSound(player->mo, sfx_punch);

    // turn to face target
    player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y,
                                        linetarget->x, linetarget->y);
}

//
// A_Saw
//
void A_Saw(player_t *player, pspdef_t *psp)
{
    int         damage = 2 * (P_Random() % 10 + 1);
    angle_t     angle = player->mo->angle + ((P_Random() - P_Random()) << 18);
    int         slope = P_AimLineAttack(player->mo, angle, MELEERANGE + 1);

    // use meleerange + 1 so the puff doesn't skip the flash
    P_LineAttack(player->mo, angle, MELEERANGE + 1, slope, damage);

    if (!linetarget)
    {
        S_StartSound(player->mo, sfx_sawful);
        return;
    }
    S_StartSound (player->mo, sfx_sawhit);

    // turn to face target
    angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);
    if (angle - player->mo->angle > ANG180)
    {
        if (angle - player->mo->angle < -ANG90 / 20)
            player->mo->angle = angle + ANG90 / 21;
        else
            player->mo->angle -= ANG90 / 20;
    }
    else
    {
        if (angle - player->mo->angle > ANG90 / 20)
            player->mo->angle = angle - ANG90 / 21;
        else
            player->mo->angle += ANG90 / 20;
    }
    player->mo->flags |= MF_JUSTATTACKED;
}

//
// A_FireMissile
//
void A_FireMissile(player_t *player, pspdef_t *psp)
{
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SpawnPlayerMissile(player->mo, MT_ROCKET);
}

//
// A_FireBFG
//
void A_FireBFG(player_t *player, pspdef_t *psp)
{
    player->ammo[weaponinfo[player->readyweapon].ammo] -= BFGCELLS;
    P_SpawnPlayerMissile(player->mo, MT_BFG);
}

//
// A_FirePlasma
//
void A_FirePlasma(player_t *player, pspdef_t *psp)
{
    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate + (P_Random() & 1));

    P_SpawnPlayerMissile(player->mo, MT_PLASMA);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//
fixed_t bulletslope;


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
void P_GunShot(mobj_t *mo, boolean accurate)
{
    int         damage = 5 * (P_Random() % 3 + 1);
    angle_t     angle = mo->angle;

    if (!accurate)
        angle += (P_Random() - P_Random()) << 18;

    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FirePistol
//
void A_FirePistol(player_t *player, pspdef_t *psp)
{
    S_StartSound(player->mo, sfx_pistol);

    P_SetMobjState(player->mo, S_PLAY_ATK2);
    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    P_SetPsprite(player, ps_flash, (statenum_t)weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(player->mo);
    P_GunShot(player->mo, !player->refire);
}

//
// A_FireShotgun
//
void A_FireShotgun(player_t *player, pspdef_t *psp)
{
    int         i;

    S_StartSound(player->mo, sfx_shotgn);
    P_SetMobjState(player->mo, S_PLAY_ATK2);

    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    P_SetPsprite(player, ps_flash, (statenum_t)weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(player->mo);

    for (i = 0; i < 7; i++)
        P_GunShot(player->mo, false);

    player->preferredshotgun = wp_shotgun;
}

//
// A_FireShotgun2
//
void A_FireShotgun2(player_t *player, pspdef_t *psp)
{
    int         i;

    S_StartSound(player->mo, sfx_dshtgn);
    P_SetMobjState(player->mo, S_PLAY_ATK2);

    player->ammo[weaponinfo[player->readyweapon].ammo] -= 2;

    P_SetPsprite(player, ps_flash, (statenum_t)weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(player->mo);

    for (i = 0; i < 20; i++)
    {
        int     damage = 5 * (P_Random() % 3 + 1);
        angle_t angle = player->mo->angle + ((P_Random() - P_Random()) << 19);

        P_LineAttack(player->mo, angle, MISSILERANGE,
                     bulletslope + ((P_Random() - P_Random()) << 5), damage);
    }

    player->preferredshotgun = wp_supershotgun;
}

//
// A_FireCGun
//
void A_FireCGun(player_t *player, pspdef_t *psp)
{
    if (player->ammo[weaponinfo[player->readyweapon].ammo])
        S_StartSound(player->mo, sfx_pistol);

    if (!player->ammo[weaponinfo[player->readyweapon].ammo])
        return;

    P_SetMobjState(player->mo, S_PLAY_ATK2);
    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    P_SetPsprite(player, ps_flash,
                 weaponinfo[player->readyweapon].flashstate + psp->state - &states[S_CHAIN1]);

    P_BulletSlope(player->mo);

    P_GunShot(player->mo, !player->refire);
}

void A_Light0(player_t *player, pspdef_t *psp)
{
    player->extralight = 0;
}

void A_Light1(player_t *player, pspdef_t *psp)
{
    player->extralight = 1;
}

void A_Light2(player_t *player, pspdef_t *psp)
{
    player->extralight = 2;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//
void A_BFGSpray(mobj_t *mo)
{
    int         i;

    // offset angles from its attack angle
    for (i = 0; i < 40; i++)
    {
        int     j;
        int     damage;
        angle_t an = mo->angle - ANG90 / 2 + ANG90 / 40 * i;

        // mo->target is the originator (player)
        //  of the missile
        P_AimLineAttack(mo->target, an, 16 * 64 * FRACUNIT);

        if (!linetarget)
            continue;

        P_SpawnMobj(linetarget->x, linetarget->y, linetarget->z + (linetarget->height >> 2),
                    MT_EXTRABFG);

        damage = 0;
        for (j = 0; j < 15; j++)
            damage += (P_Random() & 7) + 1;

        P_DamageMobj(linetarget, mo->target, mo->target, damage);
    }
}

//
// A_BFGsound
//
void A_BFGsound(player_t *player, pspdef_t *psp)
{
    S_StartSound(player->mo, sfx_bfg);
}

//
// P_SetupPsprites
// Called at start of level for each player.
//
void P_SetupPsprites(player_t *player)
{
    int         i;

    // remove all psprites
    for (i = 0; i < NUMPSPRITES; i++)
        player->psprites[i].state = NULL;

    // spawn the gun
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon(player);
}

//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites(player_t *player)
{
    int         i;
    pspdef_t    *psp = &player->psprites[0];

    for (i = 0; i < NUMPSPRITES; i++, psp++)
        if (psp->state && psp->tics != -1 && !--psp->tics)
            P_SetPsprite(player, i, psp->state->nextstate);

    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}
