/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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

#include "d_event.h"
#include "doomstat.h"
#include "i_gamepad.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"

#define LOWERSPEED              (6 * FRACUNIT)
#define RAISESPEED              (6 * FRACUNIT)

#define CHAINSAWIDLEMOTORSPEED  15000
#define MAXMOTORSPEED           65535

dboolean        centerweapon = centerweapon_default;
dboolean        weaponrecoil = weaponrecoil_default;
int             weaponbob = weaponbob_default;

unsigned int    stat_shotsfired = 0;
unsigned int    stat_shotshit = 0;

dboolean        successfulshot;
dboolean        skippsprinterp;

static const int recoilvalues[] = {
     0, // wp_fist
     4, // wp_pistol
     8, // wp_shotgun
     4, // wp_chaingun
    16, // wp_missile
     4, // wp_plasma
    20, // wp_bfg
    -2, // wp_chainsaw
    16  // wp_supershotgun
};

extern dboolean canmouselook;
extern dboolean hitwall;
extern dboolean usemouselook;

void P_CheckMissileSpawn(mobj_t *th);

void A_Recoil(player_t *player, weapontype_t weapon)
{
    if (weaponrecoil && canmouselook)
        player->recoil = recoilvalues[weapon];
}

//
// P_SetPsprite
//
void P_SetPsprite(player_t *player, size_t position, statenum_t stnum)
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

        if (state->dehacked)
            weaponinfo[player->readyweapon].altered = true;

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
static void P_BringUpWeapon(player_t *player)
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
    weapontype_t    readyweapon = player->readyweapon;
    ammotype_t      ammo = weaponinfo[readyweapon].ammo;
    int             count = 1;  // Regular.

    // Some do not need ammunition anyway.
    if (ammo == am_noammo)
        return true;

    // Minimal amount for one shot varies.
    if (readyweapon == wp_bfg)
        count = bfgcells;
    else if (readyweapon == wp_supershotgun)
        count = 2;              // Double barrel.

    // Return if current ammunition sufficient.
    if (player->ammo[ammo] >= count)
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
    else if (player->weaponowned[wp_bfg] && (player->ammo[am_cell] >= bfgcells || bfgcells != BFGCELLS))
        player->pendingweapon = wp_bfg;
    else
        // If everything fails.
        player->pendingweapon = wp_fist;

    return false;
}

//
// P_SubtractAmmo
//
static void P_SubtractAmmo(player_t *player, int amount)
{
    ammotype_t  ammotype = weaponinfo[player->readyweapon].ammo;

    if (ammotype < NUMAMMO)
        player->ammo[ammotype] = MAX(0, player->ammo[ammotype] - amount);
}

//
// P_FireWeapon
//
void P_FireWeapon(player_t *player)
{
    weapontype_t    readyweapon;

    if (!P_CheckAmmo(player) || (automapactive && !am_followmode))
        return;

    readyweapon = player->readyweapon;

    P_SetMobjState(player->mo, S_PLAY_ATK1);
    P_SetPsprite(player, ps_weapon, weaponinfo[readyweapon].atkstate);

    if (gp_vibrate_weapons && vibrate)
    {
        int motorspeed = weaponinfo[readyweapon].motorspeed * gp_vibrate_weapons / 100;

        if ((readyweapon == wp_fist && player->powers[pw_strength])
            || (readyweapon == wp_chainsaw && linetarget))
            motorspeed = MAXMOTORSPEED;

        XInputVibration(motorspeed);
        weaponvibrationtics = weaponinfo[readyweapon].tics;
    }

    if (centerweapon)
    {
        pspdef_t    *psp = &player->psprites[ps_weapon];
        state_t     *state = psp->state;

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
    weapontype_t    readyweapon;
    weapontype_t    pendingweapon;

    readyweapon = player->readyweapon;
    pendingweapon = player->pendingweapon;

    // get out of attack state
    if (actor->state == &states[S_PLAY_ATK1] || actor->state == &states[S_PLAY_ATK2])
        P_SetMobjState(actor, S_PLAY);

    if (readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
        S_StartSound(actor, sfx_sawidl);

    // check for change
    //  if player is dead, put the weapon away
    if (pendingweapon != wp_nochange || player->health <= 0)
    {
        if (gp_vibrate_weapons && vibrate)
        {
            if (pendingweapon == wp_chainsaw)
            {
                idlemotorspeed = CHAINSAWIDLEMOTORSPEED * gp_vibrate_weapons / 100;
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
    //  the missile launcher and BFG do not auto fire
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
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    // check for fire
    //  (if a weapon change is pending, let it go through instead)
    if ((player->cmd.buttons & BT_ATTACK) && player->pendingweapon == wp_nochange && player->health > 0)
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
    if (!P_CheckAmmo(player))
        P_SetPsprite(player, ps_weapon, weaponinfo[player->readyweapon].downstate);
}

//
// A_Lower
// Lowers current weapon,
// and changes weapon at bottom.
//
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp)
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
    if (player->health <= 0)
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
    angle_t angle = actor->angle + ((M_Random() - M_Random()) << 18);
    int     slope = P_AimLineAttack(actor, angle, MELEERANGE);
    int     damage = (M_Random() % 10 + 1) << 1;

    if (player->powers[pw_strength])
        damage *= 10;

    hitwall = false;
    P_LineAttack(actor, angle, MELEERANGE, slope, damage);

    if (linetarget || hitwall)
    {
        P_NoiseAlert(player->mo, player->mo);
        S_StartSound(actor, sfx_punch);

        // turn to face target
        if (linetarget)
            actor->angle = R_PointToAngle2(actor->x, actor->y, linetarget->x, linetarget->y);
    }
}

//
// A_Saw
//
void A_Saw(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int     damage = 2 * (M_Random() % 10 + 1);
    angle_t angle = actor->angle + ((M_Random() - M_Random()) << 18);
    int     slope = P_AimLineAttack(actor, angle, MELEERANGE + 1);

    // use MELEERANGE + 1 so the puff doesn't skip the flash
    P_LineAttack(actor, angle, MELEERANGE + 1, slope, damage);

    A_Recoil(player, wp_chainsaw);

    P_NoiseAlert(player->mo, player->mo);

    if (!linetarget)
    {
        S_StartSound(actor, sfx_sawful);
        return;
    }

    S_StartSound(actor, sfx_sawhit);

    // turn to face target
    angle = R_PointToAngle2(actor->x, actor->y, linetarget->x, linetarget->y);

    if (angle - actor->angle > ANG180)
    {
        if (angle - actor->angle < (angle_t)(-ANG90 / 20))
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
    P_SubtractAmmo(player, 1);
    P_SpawnPlayerMissile(player->mo, MT_ROCKET);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);
}

//
// A_FireBFG
//
void A_FireBFG(mobj_t *actor, player_t *player, pspdef_t *psp)
{
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
//
void A_FireOldBFG(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobjtype_t  type = MT_PLASMA1;

    P_SubtractAmmo(player, 1);

    player->extralight = 2;

    do
    {
        mobj_t  *th;
        mobj_t  *mo = player->mo;
        angle_t an = mo->angle;
        angle_t an1 = ((M_Random() & 127) - 64) * (ANG90 / 768) + an;
        angle_t an2 = ((M_Random() & 127) - 64) * (ANG90 / 640) + ANG90;
        fixed_t slope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);

        if (!linetarget)
            slope = P_AimLineAttack(mo, an += 1 << 26, 16 * 64 * FRACUNIT);

        if (!linetarget)
            slope = P_AimLineAttack(mo, an -= 2 << 26, 16 * 64 * FRACUNIT);

        if (!linetarget)
        {
            slope = 0;
            an = mo->angle;
        }

        an1 += an - mo->angle;
        an2 += tantoangle[slope >> DBITS];

        th = P_SpawnMobj(mo->x, mo->y, mo->z + 62 * FRACUNIT - player->psprites[ps_weapon].sy, type);
        P_SetTarget(&th->target, mo);
        th->angle = an1;
        th->momx = finecosine[an1 >> ANGLETOFINESHIFT] * 25;
        th->momy = finesine[an1 >> ANGLETOFINESHIFT] * 25;
        th->momz = finetangent[an2 >> ANGLETOFINESHIFT] * 25;
        P_CheckMissileSpawn(th);
    }
    while (type != MT_PLASMA2 && (type = MT_PLASMA2)); // killough: obfuscated!

    A_Recoil(player, wp_plasma);
}

//
// A_FirePlasma
//
void A_FirePlasma(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_SubtractAmmo(player, 1);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate + (M_Random() & 1));

    P_SpawnPlayerMissile(player->mo, MT_PLASMA);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at approximately
// the height of the intended target
//
static fixed_t  bulletslope;

static void P_BulletSlope(mobj_t *mo)
{
    angle_t an = mo->angle;

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

            if (!linetarget && usemouselook)
                bulletslope = ((mo->player->lookdir / MLOOKUNIT) << FRACBITS) / 173;
        }
    }
}

//
// P_GunShot
//
static void P_GunShot(mobj_t *actor, dboolean accurate)
{
    int     damage = 5 * (M_Random() % 3 + 1);
    angle_t angle = actor->angle;

    if (!accurate)
        angle += (M_Random() - M_Random()) << 18;

    P_LineAttack(actor, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FirePistol
//
void A_FirePistol(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_NoiseAlert(player->mo, player->mo);

    S_StartSound(actor, sfx_pistol);

    P_SetMobjState(actor, S_PLAY_ATK2);

    P_SubtractAmmo(player, 1);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    P_GunShot(actor, !player->refire);

    A_Recoil(player, wp_pistol);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);

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
    P_NoiseAlert(player->mo, player->mo);

    S_StartSound(actor, sfx_shotgn);
    P_SetMobjState(actor, S_PLAY_ATK2);

    P_SubtractAmmo(player, 1);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    for (int i = 0; i < 7; i++)
        P_GunShot(actor, false);

    A_Recoil(player, wp_shotgun);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);

    if (successfulshot)
    {
        successfulshot = false;
        player->shotshit++;
        stat_shotshit = SafeAdd(stat_shotshit, 1);
    }

    if (player->ammo[am_shell])
        player->preferredshotgun = wp_shotgun;
}

//
// A_FireShotgun2
//
void A_FireShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_NoiseAlert(player->mo, player->mo);

    S_StartSound(actor, sfx_dshtgn);
    P_SetMobjState(actor, S_PLAY_ATK2);

    P_SubtractAmmo(player, 2);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    for (int i = 0; i < 20; i++)
    {
        int     damage = 5 * (M_Random() % 3 + 1);
        angle_t angle = actor->angle + ((M_Random() - M_Random()) << ANGLETOFINESHIFT);

        P_LineAttack(actor, angle, MISSILERANGE, bulletslope + ((M_Random() - M_Random()) << 5), damage);
    }

    A_Recoil(player, wp_supershotgun);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);

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
    if (!player->ammo[weaponinfo[player->readyweapon].ammo])
        return;

    P_NoiseAlert(player->mo, player->mo);
    S_StartSound(actor, sfx_pistol);

    P_SetMobjState(actor, S_PLAY_ATK2);

    P_SubtractAmmo(player, 1);

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate
        + (unsigned int)((psp->state - &states[S_CHAIN1]) & 1));

    P_BulletSlope(actor);

    successfulshot = false;

    P_GunShot(actor, !player->refire);

    A_Recoil(player, wp_chaingun);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);

    if (successfulshot)
    {
        successfulshot = false;
        player->shotshit++;
        stat_shotshit = SafeAdd(stat_shotshit, 1);
    }
}

void A_Light0(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    player->extralight = 0;
}

void A_Light1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    player->extralight = 1;
}

void A_Light2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    player->extralight = 2;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//
void A_BFGSpray(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = actor->target;

    if (mo->player)
        P_NoiseAlert(mo->player->mo, mo->player->mo);

    // offset angles from its attack angle
    for (int i = 0; i < 40; i++)
    {
        int damage = 0;

        P_AimLineAttack(mo, actor->angle - ANG90 / 2 + ANG90 / 40 * i, 16 * 64 * FRACUNIT);

        if (!linetarget)
            continue;

        successfulshot = true;

        P_SpawnMobj(linetarget->x, linetarget->y, linetarget->z + (linetarget->height >> 2), MT_EXTRABFG);

        for (int j = 0; j < 15; j++)
            damage += (M_Random() & 7) + 1;

        P_DamageMobj(linetarget, mo, mo, damage, true);
    }

    if (mo->player)
    {
        mo->player->shotsfired++;
        stat_shotsfired = SafeAdd(stat_shotsfired, 1);

        if (successfulshot)
        {
            mo->player->shotshit++;
            stat_shotshit = SafeAdd(stat_shotshit, 1);
        }
    }

    successfulshot = false;
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
    // remove all psprites
    player->psprites[ps_weapon].state = NULL;
    player->psprites[ps_flash].state = NULL;

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
    pspdef_t    *psp = player->psprites;
    pspdef_t    *weapon = &psp[ps_weapon];
    pspdef_t    *flash = &psp[ps_flash];

    for (int i = 0; i < NUMPSPRITES; i++, psp++)
        if (psp->state && psp->tics != -1 && !--psp->tics)
            P_SetPsprite(player, i, psp->state->nextstate);

    if (weapon->state->action == A_WeaponReady)
    {
        // bob the weapon based on movement speed
        fixed_t momx = player->momx;
        fixed_t momy = player->momy;
        fixed_t bob = (FixedMul(momx, momx) + FixedMul(momy, momy)) >> 2;

        bob = (bob ? MAX(MIN(bob, MAXBOB) * weaponbob / 100, MAXBOB * stillbob / 400) :
            MAXBOB * stillbob / 400);

        // [BH] smooth out weapon bob by zeroing out really small bobs
        if (bob < FRACUNIT / 2)
        {
            weapon->sx = 0;
            weapon->sy = WEAPONTOP;
        }
        else
        {
            int angle = (128 * leveltime) & FINEMASK;

            weapon->sx = FixedMul(bob, finecosine[angle]);
            weapon->sy = WEAPONTOP + FixedMul(bob, finesine[angle & (FINEANGLES / 2 - 1)]);
        }
    }

    // [BH] shake the BFG before firing when weapon recoil enabled
    if (player->readyweapon == wp_bfg && weaponrecoil && canmouselook)
    {
        if (weapon->state == &states[S_BFG1])
        {
            weapon->sx = M_RandomInt(-2, 2) * FRACUNIT;
            weapon->sy = WEAPONTOP + M_RandomInt(-1, 1) * FRACUNIT;
        }
        else if (weapon->state == &states[S_BFG2])
        {
            weapon->sx = 0;
            weapon->sy = WEAPONTOP;
        }
    }

    flash->sx = weapon->sx;
    flash->sy = weapon->sy;
}
