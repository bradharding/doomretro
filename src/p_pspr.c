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

#include "d_event.h"
#include "doomstat.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"

#define LOWERSPEED              (6 * FRACUNIT)
#define RAISESPEED              (6 * FRACUNIT)

#define CHAINSAWIDLEMOTORSPEED  15000
#define MAXMOTORSPEED           65535

#define FLAMETHROWERTICS        (10 * 35)
#define MAGICJUNK               1234
#define MAXMACESPOTS            8

dboolean        autoaim = autoaim_default;
dboolean        centerweapon = centerweapon_default;
dboolean        weaponrecoil = weaponrecoil_default;
int             weaponbob = weaponbob_default;

unsigned int    stat_shotsfired = 0;
unsigned int    stat_shotshit = 0;

dboolean        successfulshot;
dboolean        skippsprinterp;

static int      macespotcount;

static struct
{
    fixed_t     x, y;
} macespots[MAXMACESPOTS];

static int weaponammousepl1[NUMWEAPONS] =
{
    0,                          // staff
    USE_GWND_AMMO_1,            // gold wand
    USE_CBOW_AMMO_1,            // crossbow
    USE_BLSR_AMMO_1,            // blaster
    USE_SKRD_AMMO_1,            // skull rod
    USE_PHRD_AMMO_1,            // phoenix rod
    USE_MACE_AMMO_1,            // mace
    0,                          // gauntlets
    0                           // beak
};

static int weaponammousepl2[NUMWEAPONS] =
{
    0,                          // staff
    USE_GWND_AMMO_2,            // gold wand
    USE_CBOW_AMMO_2,            // crossbow
    USE_BLSR_AMMO_2,            // blaster
    USE_SKRD_AMMO_2,            // skull rod
    USE_PHRD_AMMO_2,            // phoenix rod
    USE_MACE_AMMO_2,            // mace
    0,                          // gauntlets
    0                           // beak
};

extern dboolean canmouselook;
extern dboolean hitwall;
extern dboolean usemouselook;

dboolean P_CheckMissileSpawn(mobj_t *th);

void A_Recoil(weapontype_t weapon)
{
    if (weaponrecoil && canmouselook)
        viewplayer->recoil = weaponinfo[weapon].recoil;
}

void P_OpenWeapons(void)
{
    macespotcount = 0;
}

void P_AddMaceSpot(mapthing_t *mthing)
{
    if (macespotcount == MAXMACESPOTS)
        I_Error("Too many mace spots.");

    macespots[macespotcount].x = mthing->x << FRACBITS;
    macespots[macespotcount].y = mthing->y << FRACBITS;
    macespotcount++;
}

void P_RepositionMace(mobj_t *mo)
{
    int         spot = M_Random() % macespotcount;
    subsector_t *ss;

    P_UnsetThingPosition(mo);
    mo->x = macespots[spot].x;
    mo->y = macespots[spot].y;
    ss = R_PointInSubsector(mo->x, mo->y);
    mo->z = mo->floorz = ss->sector->floorheight;
    mo->ceilingz = ss->sector->ceilingheight;
    P_SetThingPosition(mo);
}

void P_CloseWeapons(void)
{
    int spot;

    if (!macespotcount)
        return; // No maces placed

    if (M_Random() < 64)
        return;

    spot = M_Random() % macespotcount;
    P_SpawnMobj(macespots[spot].x, macespots[spot].y, ONFLOORZ, HMT_WMACE);
}

//
// P_SetPsprite
//
void P_SetPsprite(size_t position, statenum_t stnum)
{
    pspdef_t    *psp = &viewplayer->psprites[position];

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
        psp->tics = state->tics;    // could be 0

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
            state->action(viewplayer->mo, viewplayer, psp);

            if (!psp->state)
                break;
        }

        stnum = psp->state->nextstate;
    } while (!psp->tics);           // an initial state of 0 could cycle through
}

void P_ActivateBeak(void)
{
    viewplayer->pendingweapon = wp_nochange;
    viewplayer->readyweapon = (weapontype_t)wp_beak;
    viewplayer->psprites[ps_weapon].sy = WEAPONTOP;
    P_SetPsprite(ps_weapon, HS_BEAKREADY);
}

void P_PostChickenWeapon(weapontype_t weapon)
{
    if (weapon == wp_beak)
        weapon = (weapontype_t)wp_staff;

    viewplayer->pendingweapon = wp_nochange;
    viewplayer->readyweapon = weapon;
    viewplayer->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(ps_weapon, wpnlev1info[weapon].upstate);
}

//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
static void P_BringUpWeapon(void)
{
    statenum_t  new;

    if (viewplayer->pendingweapon == wp_nochange)
        viewplayer->pendingweapon = viewplayer->readyweapon;
    else if (viewplayer->pendingweapon == wp_chainsaw)
        S_StartSound(viewplayer->mo, sfx_sawup);

    if (gamemission == heretic)
    {
        if (viewplayer->pendingweapon == wp_gauntlets)
            S_StartSound(viewplayer->mo, hsfx_gntact);

        if (viewplayer->powers[pw_weaponlevel2])
            new = wpnlev2info[viewplayer->pendingweapon].upstate;
        else
            new = wpnlev1info[viewplayer->pendingweapon].upstate;
    }
    else
        new = weaponinfo[viewplayer->pendingweapon].upstate;

    viewplayer->pendingweapon = wp_nochange;
    viewplayer->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(ps_weapon, new);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
dboolean P_CheckAmmo(weapontype_t weapon)
{
    ammotype_t  ammotype = weaponinfo[weapon].ammotype;

    // Some do not need ammunition anyway.
    if (ammotype == am_noammo)
        return true;

    // Return if current ammunition sufficient.
    if (viewplayer->ammo[ammotype] >= weaponinfo[weapon].minammo)
        return true;

    // Out of ammo, pick a weapon to change to.
    // Preferences are set here.
    if (viewplayer->weaponowned[wp_plasma] && viewplayer->ammo[am_cell] >= weaponinfo[wp_plasma].minammo)
        viewplayer->pendingweapon = wp_plasma;
    else if (viewplayer->weaponowned[wp_supershotgun] && viewplayer->ammo[am_shell] >= weaponinfo[wp_supershotgun].minammo
             && viewplayer->preferredshotgun == wp_supershotgun)
        viewplayer->pendingweapon = wp_supershotgun;
    else if (viewplayer->weaponowned[wp_chaingun] && viewplayer->ammo[am_clip] >= weaponinfo[wp_chaingun].minammo)
        viewplayer->pendingweapon = wp_chaingun;
    else if (viewplayer->weaponowned[wp_shotgun] && viewplayer->ammo[am_shell] >= weaponinfo[wp_shotgun].minammo)
        viewplayer->pendingweapon = wp_shotgun;
    else if (viewplayer->ammo[am_clip] >= weaponinfo[wp_pistol].minammo)
        viewplayer->pendingweapon = wp_pistol;
    else if (viewplayer->weaponowned[wp_chainsaw])
        viewplayer->pendingweapon = wp_chainsaw;
    else if (viewplayer->weaponowned[wp_missile] && viewplayer->ammo[am_misl] >= weaponinfo[wp_missile].minammo)
        viewplayer->pendingweapon = wp_missile;
    else if (viewplayer->weaponowned[wp_bfg] && viewplayer->ammo[am_cell] >= weaponinfo[wp_bfg].minammo)
        viewplayer->pendingweapon = wp_bfg;
    else
        viewplayer->pendingweapon = wp_fist;

    return false;
}

dboolean P_CheckHereticAmmo(weapontype_t weapon)
{
    ammotype_t  ammotype = wpnlev1info[weapon].ammotype;
    int         *ammouse = (viewplayer->powers[pw_weaponlevel2] ? weaponammousepl2 : weaponammousepl1);
    int         count = ammouse[viewplayer->readyweapon];

    if (ammotype == am_noammo || viewplayer->ammo[ammotype] >= count)
        return true;

    // out of ammo, pick a weapon to change to
    do
    {
        if (viewplayer->weaponowned[wp_skullrod] && viewplayer->ammo[am_skullrod] > ammouse[wp_skullrod])
            viewplayer->pendingweapon = (weapontype_t)wp_skullrod;
        else if (viewplayer->weaponowned[wp_blaster] && viewplayer->ammo[am_blaster] > ammouse[wp_blaster])
            viewplayer->pendingweapon = (weapontype_t)wp_blaster;
        else if (viewplayer->weaponowned[wp_crossbow] && viewplayer->ammo[am_crossbow] > ammouse[wp_crossbow])
            viewplayer->pendingweapon = (weapontype_t)wp_crossbow;
        else if (viewplayer->weaponowned[wp_mace] && viewplayer->ammo[am_mace] > ammouse[wp_mace])
            viewplayer->pendingweapon = (weapontype_t)wp_mace;
        else if (viewplayer->ammo[am_goldwand] > ammouse[wp_goldwand])
            viewplayer->pendingweapon = (weapontype_t)wp_goldwand;
        else if (viewplayer->weaponowned[wp_gauntlets])
            viewplayer->pendingweapon = (weapontype_t)wp_gauntlets;
        else if (viewplayer->weaponowned[wp_phoenixrod] && viewplayer->ammo[am_phoenixrod] > ammouse[wp_phoenixrod])
            viewplayer->pendingweapon = (weapontype_t)wp_phoenixrod;
        else
            viewplayer->pendingweapon = (weapontype_t)wp_staff;
    } while (viewplayer->pendingweapon == wp_nochange);

    if (viewplayer->powers[pw_weaponlevel2])
        P_SetPsprite(ps_weapon, wpnlev2info[viewplayer->readyweapon].downstate);
    else
        P_SetPsprite(ps_weapon, wpnlev1info[viewplayer->readyweapon].downstate);

    return false;
}

//
// P_SubtractAmmo
//
static void P_SubtractAmmo(weapontype_t weapon, int amount)
{
    ammotype_t  ammotype = weaponinfo[weapon].ammotype;

    if (ammotype != am_noammo)
        viewplayer->ammo[ammotype] = MAX(0, viewplayer->ammo[ammotype] - amount);
}

//
// P_FireWeapon
//
void P_FireWeapon(void)
{
    weapontype_t    readyweapon = viewplayer->readyweapon;

    if (gamemission == heretic)
    {
        weaponinfo_t    *wpinfo = (viewplayer->powers[pw_weaponlevel2] ? &wpnlev2info[0] : &wpnlev1info[0]);

        if (!P_CheckHereticAmmo(readyweapon) || (automapactive && !am_followmode))
            return;

        P_SetPsprite(ps_weapon, (viewplayer->refire ? wpinfo[viewplayer->readyweapon].holdatkstate :
            wpinfo[viewplayer->readyweapon].atkstate));

        if (viewplayer->readyweapon == wp_gauntlets && !viewplayer->refire)
            S_StartSound(viewplayer->mo, hsfx_gntuse);
    }
    else
    {
        if (!P_CheckAmmo(readyweapon) || (automapactive && !am_followmode))
            return;

        P_SetPsprite(ps_weapon, weaponinfo[viewplayer->readyweapon].atkstate);
    }

    if (gp_vibrate_weapons && vibrate)
    {
        int motorspeed = weaponinfo[readyweapon].motorspeed * gp_vibrate_weapons / 100;

        if ((readyweapon == wp_fist && viewplayer->powers[pw_strength]) || (readyweapon == wp_chainsaw && linetarget))
            motorspeed = MAXMOTORSPEED;

        XInputVibration(motorspeed);
        weaponvibrationtics = weaponinfo[readyweapon].tics;
    }

    if (centerweapon)
    {
        pspdef_t    *psp = &viewplayer->psprites[ps_weapon];
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
void P_DropWeapon(void)
{
    if (gamemission == heretic)
    {
        if (viewplayer->powers[pw_weaponlevel2])
            P_SetPsprite(ps_weapon, wpnlev2info[viewplayer->readyweapon].downstate);
        else
            P_SetPsprite(ps_weapon, wpnlev1info[viewplayer->readyweapon].downstate);
    }
    else
        P_SetPsprite(ps_weapon, weaponinfo[viewplayer->readyweapon].downstate);
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
    weapontype_t    readyweapon = player->readyweapon;
    weapontype_t    pendingweapon = player->pendingweapon;

    if (player->chickentics)
    {
        P_ActivateBeak();
        return;
    }

    if (gamemission != heretic && readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
        S_StartSound(actor, sfx_sawidl);
    else if (gamemission == heretic && readyweapon == wp_staff && psp->state == &states[HS_STAFFREADY2_1] && M_Random() < 128)
        S_StartSound(player->mo, hsfx_stfcrk);

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

        P_DropWeapon();
        return;
    }

    // check for fire
    //  the missile launcher and BFG do not auto fire
    if (player->cmd.buttons & BT_ATTACK)
    {
        if (!player->attackdown || (readyweapon != wp_missile && readyweapon != wp_bfg && gamemission != heretic)
            || (readyweapon != wp_phoenixrod && gamemission == heretic))
        {
            player->attackdown = true;
            P_FireWeapon();
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
        P_FireWeapon();
    }
    else
    {
        player->refire = 0;

        if (gamemission == heretic)
            P_CheckHereticAmmo(player->readyweapon);
        else
            P_CheckAmmo(player->readyweapon);
    }
}

void P_UpdateBeak(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    psp->sy = WEAPONTOP + (player->chickenpeck << (FRACBITS - 1));
}

void A_BeakReady(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (player->cmd.buttons & BT_ATTACK)
    {
        // Chicken beak attack
        player->attackdown = true;
        P_SetMobjState(player->mo, HS_CHICPLAY_ATK1);
        P_SetPsprite(ps_weapon, (player->powers[pw_weaponlevel2] ? HS_BEAKATK2_1 : HS_BEAKATK1_1));
        P_NoiseAlert(player->mo);
    }
    else
    {
        if (player->mo->state == &states[HS_CHICPLAY_ATK1])
            P_SetMobjState(player->mo, HS_CHICPLAY);

        player->attackdown = false;
    }
}

void A_CheckReload(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    weapontype_t    readyweapon = player->readyweapon;

    if (!P_CheckAmmo(readyweapon))
        P_SetPsprite(ps_weapon, weaponinfo[readyweapon].downstate);
}

//
// A_Lower
// Lowers current weapon,
// and changes weapon at bottom.
//
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (player->chickentics)
        psp->sy = WEAPONBOTTOM;
    else
    {
        psp->sy += LOWERSPEED;

        if (psp->sy < WEAPONBOTTOM)
            return;
    }

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
        P_SetPsprite(ps_weapon, S_NULL);
        return;
    }

    if (player->pendingweapon != wp_nochange)
        player->readyweapon = player->pendingweapon;

    P_BringUpWeapon();
}

void A_BeakRaise(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    psp->sy = WEAPONTOP;
    P_SetPsprite(ps_weapon, wpnlev1info[player->readyweapon].readystate);
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

    if (gamemission == heretic)
    {
        if (player->powers[pw_weaponlevel2])
            P_SetPsprite(ps_weapon, wpnlev2info[player->readyweapon].readystate);
        else
            P_SetPsprite(ps_weapon, wpnlev1info[player->readyweapon].readystate);
    }
    else
        P_SetPsprite(ps_weapon, weaponinfo[player->readyweapon].readystate);
}

//
// A_GunFlash
//
void A_GunFlash(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_SetPsprite(ps_flash, weaponinfo[player->readyweapon].flashstate);
}

//
// WEAPON ATTACKS
//

//
// A_Punch
//
void A_Punch(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t angle = actor->angle + (M_NegRandom() << 18);
    int     slope = P_AimLineAttack(actor, angle, MELEERANGE);
    int     damage = (M_Random() % 10 + 1) << 1;

    if (player->powers[pw_strength])
        damage *= 10;

    hitwall = false;
    P_LineAttack(actor, angle, MELEERANGE, slope, damage);

    if (linetarget || hitwall)
    {
        P_NoiseAlert(actor);
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
    angle_t angle = actor->angle + (M_NegRandom() << 18);
    int     slope = P_AimLineAttack(actor, angle, MELEERANGE + 1);

    // use MELEERANGE + 1 so the puff doesn't skip the flash
    P_LineAttack(actor, angle, MELEERANGE + 1, slope, 2 * (M_Random() % 10 + 1));

    A_Recoil(wp_chainsaw);

    P_NoiseAlert(actor);

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
    P_SubtractAmmo(wp_missile, 1);
    P_SpawnPlayerMissile(actor, MT_ROCKET);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);
}

//
// A_FireBFG
//
void A_FireBFG(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_SubtractAmmo(wp_bfg, bfgcells);
    P_SpawnPlayerMissile(actor, MT_BFG);
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

    P_SubtractAmmo(wp_bfg, 1);

    player->extralight = 2;

    do
    {
        mobj_t  *th;
        angle_t an = actor->angle;
        angle_t an1 = ((M_Random() & 127) - 64) * (ANG90 / 768) + an;
        angle_t an2 = ((M_Random() & 127) - 64) * (ANG90 / 640) + ANG90;
        fixed_t slope = P_AimLineAttack(actor, an, 16 * 64 * FRACUNIT);

        if (!linetarget)
            slope = P_AimLineAttack(actor, (an += 1 << 26), 16 * 64 * FRACUNIT);

        if (!linetarget)
            slope = P_AimLineAttack(actor, (an -= 2 << 26), 16 * 64 * FRACUNIT);

        if (!linetarget)
        {
            slope = 0;
            an = actor->angle;
        }

        an1 += an - actor->angle;
        an2 += tantoangle[slope >> DBITS];

        th = P_SpawnMobj(actor->x, actor->y, actor->z + 62 * FRACUNIT - player->psprites[ps_weapon].sy, type);
        P_SetTarget(&th->target, actor);
        th->angle = an1;
        th->momx = finecosine[an1 >> ANGLETOFINESHIFT] * 25;
        th->momy = finesine[an1 >> ANGLETOFINESHIFT] * 25;
        th->momz = finetangent[an2 >> ANGLETOFINESHIFT] * 25;
        P_CheckMissileSpawn(th);
    } while (type != MT_PLASMA2 && (type = MT_PLASMA2));    // killough: obfuscated!

    A_Recoil(wp_plasma);
}

//
// A_FirePlasma
//
void A_FirePlasma(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_SubtractAmmo(wp_plasma, 1);

    P_SetPsprite(ps_flash, weaponinfo[player->readyweapon].flashstate + (M_Random() & 1));

    P_SpawnPlayerMissile(actor, MT_PLASMA);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at approximately
// the height of the intended target
//
static fixed_t  bulletslope;

static void P_BulletSlope(mobj_t *actor)
{
    if (usemouselook && !autoaim)
        bulletslope = ((viewplayer->lookdir / MLOOKUNIT) << FRACBITS) / 173;
    else
    {
        angle_t an = actor->angle;

        // see which target is to be aimed at
        bulletslope = P_AimLineAttack(actor, an, 16 * 64 * FRACUNIT);

        if (!linetarget)
        {
            bulletslope = P_AimLineAttack(actor, (an += 1 << 26), 16 * 64 * FRACUNIT);

            if (!linetarget)
            {
                bulletslope = P_AimLineAttack(actor, (an -= 2 << 26), 16 * 64 * FRACUNIT);

                if (!linetarget && usemouselook)
                    bulletslope = ((viewplayer->lookdir / MLOOKUNIT) << FRACBITS) / 173;
            }
        }
    }
}

void A_BeakAttackPL1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;
    angle_t angle = mo->angle;
    int     damage = 1 + (M_Random() & 3);
    int     slope = P_AimLineAttack(mo, angle, MELEERANGE);

    pufftype = HMT_BEAKPUFF;
    P_LineAttack(mo, angle, MELEERANGE, slope, damage);

    if (linetarget)
        mo->angle = R_PointToAngle2(mo->x, mo->y, linetarget->x, linetarget->y);

    S_StartSound(mo, hsfx_chicpk1 + (M_Random() % 3));
    player->chickenpeck = 12;
    psp->tics -= M_Random() & 7;
}

void A_BeakAttackPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;
    angle_t angle = mo->angle;
    int     damage = HITDICE(4);
    int     slope = P_AimLineAttack(mo, angle, MELEERANGE);

    pufftype = HMT_BEAKPUFF;
    P_LineAttack(mo, angle, MELEERANGE, slope, damage);

    if (linetarget)
        mo->angle = R_PointToAngle2(mo->x, mo->y, linetarget->x, linetarget->y);

    S_StartSound(mo, hsfx_chicpk1 + (M_Random() % 3));
    player->chickenpeck = 12;
    psp->tics -= M_Random() & 3;
}

void A_StaffAttackPL1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;
    angle_t angle = mo->angle + (M_NegRandom() << 18);
    int     damage = 5 + (M_Random() & 15);
    int     slope = P_AimLineAttack(mo, angle, MELEERANGE);

    pufftype = HMT_STAFFPUFF;
    P_LineAttack(mo, angle, MELEERANGE, slope, damage);

    if (linetarget)
        mo->angle = R_PointToAngle2(mo->x, mo->y, linetarget->x, linetarget->y);
}

void A_StaffAttackPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;
    angle_t angle = mo->angle + (M_NegRandom() << 18);
    int     damage = 18 + (M_Random() & 63);
    int     slope = P_AimLineAttack(mo, angle, MELEERANGE);

    pufftype = HMT_STAFFPUFF2;
    P_LineAttack(mo, angle, MELEERANGE, slope, damage);

    if (linetarget)
        mo->angle = R_PointToAngle2(mo->x, mo->y, linetarget->x, linetarget->y);
}

void A_FireBlasterPL1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;
    angle_t angle = mo->angle;
    int     damage = HITDICE(4);

    S_StartSound(mo, hsfx_gldhit);
    player->ammo[am_blaster] -= USE_BLSR_AMMO_1;
    P_BulletSlope(mo);

    if (player->refire)
        angle += M_NegRandom() << 18;

    pufftype = HMT_BLASTERPUFF1;
    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
    S_StartSound(player->mo, hsfx_blssht);
}

void A_FireBlasterPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t *mo;

    player->ammo[am_blaster] -= USE_BLSR_AMMO_2;
    mo = P_SpawnPlayerMissile(player->mo, HMT_BLASTERFX1);

    if (mo)
        mo->thinker.function = P_BlasterMobjThinker;

    S_StartSound(player->mo, hsfx_blssht);
}

void A_FireGoldWandPL1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;
    angle_t angle = mo->angle;
    int     damage = 7 + (M_Random() & 7);

    player->ammo[am_goldwand] -= USE_GWND_AMMO_1;
    P_BulletSlope(mo);

    if (player->refire)
        angle += M_NegRandom() << 18;

    pufftype = HMT_GOLDWANDPUFF1;
    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
    S_StartSound(mo, hsfx_gldhit);
}

void A_FireGoldWandPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;
    angle_t angle;
    fixed_t momz;

    player->ammo[am_goldwand] -= USE_GWND_AMMO_1;
    pufftype = HMT_GOLDWANDPUFF2;
    P_BulletSlope(mo);
    momz = FixedMul(mobjinfo[HMT_GOLDWANDFX2].speed, bulletslope);
    P_SpawnMissileAngle(mo, HMT_GOLDWANDFX2, mo->angle - (ANG45 / 8), momz);
    P_SpawnMissileAngle(mo, HMT_GOLDWANDFX2, mo->angle + (ANG45 / 8), momz);
    angle = mo->angle - (ANG45 / 8);

    for (int i = 0; i < 5; i++)
    {
        P_LineAttack(mo, angle, MISSILERANGE, bulletslope, 1 + (M_Random() & 7));
        angle += ((ANG45 / 8) * 2) / 4;
    }

    S_StartSound(mo, hsfx_gldhit);
}

void A_FireMacePL1B(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;
    mobj_t  *ball;
    angle_t angle;

    if (player->ammo[am_mace] < USE_MACE_AMMO_1)
        return;

    player->ammo[am_mace] -= USE_MACE_AMMO_1;
    mo = player->mo;
    ball = P_SpawnMobj(mo->x, mo->y, mo->z + 28 * FRACUNIT - FOOTCLIPSIZE * !!(mo->flags2 & MF2_FEETARECLIPPED), HMT_MACEFX2);
    ball->momz = 2 * FRACUNIT + ((player->lookdir) << (FRACBITS - 5));
    angle = mo->angle;
    ball->target = mo;
    ball->angle = angle;
    ball->z += player->lookdir << (FRACBITS - 4);
    angle >>= ANGLETOFINESHIFT;
    ball->momx = (mo->momx >> 1) + FixedMul(ball->info->speed, finecosine[angle]);
    ball->momy = (mo->momy >> 1) + FixedMul(ball->info->speed, finesine[angle]);
    S_StartSound(ball, hsfx_lobsht);
    P_CheckMissileSpawn(ball);
}

void A_FireMacePL1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *ball;

    if (M_Random() < 28)
    {
        A_FireMacePL1B(NULL, player, psp);
        return;
    }

    if (player->ammo[am_mace] < USE_MACE_AMMO_1)
        return;

    player->ammo[am_mace] -= USE_MACE_AMMO_1;
    psp->sx = ((M_Random() & 3) - 2) * FRACUNIT;
    psp->sy = WEAPONTOP + (M_Random() & 3) * FRACUNIT;
    ball = P_SPMAngle(player->mo, HMT_MACEFX1, player->mo->angle + (((M_Random() & 7) - 4) << 24));

    if (ball)
        ball->special1.i = 16;  // tics till dropoff
}

void A_MacePL1Check(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t angle;

    if (!actor->special1.i)
        return;

    actor->special1.i -= 4;

    if (actor->special1.i > 0)
        return;

    actor->special1.i = 0;
    actor->flags3 |= MF3_LOGRAV;
    angle = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(7 * FRACUNIT, finecosine[angle]);
    actor->momy = FixedMul(7 * FRACUNIT, finesine[angle]);
    actor->momz -= actor->momz >> 1;
}

void A_MaceBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (actor->z <= actor->floorz && P_HitFloor(actor) != FLOOR_SOLID)
    {
        // Landed in some sort of liquid
        P_RemoveMobj(actor);
        return;
    }

    if (actor->health != MAGICJUNK && actor->z <= actor->floorz && actor->momz)
    {
        // Bounce
        actor->health = MAGICJUNK;
        actor->momz = (actor->momz * 192) >> 8;
        actor->flags3 &= ~MF3_FLOORBOUNCE;
        P_SetMobjState(actor, actor->info->spawnstate);
        S_StartSound(actor, hsfx_bounce);
    }
    else
    {
        // Explode
        actor->flags |= MF_NOGRAVITY;
        actor->flags3 &= ~MF3_LOGRAV;
        S_StartSound(actor, hsfx_lobhit);
    }
}

void A_MaceBallImpact2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (actor->z <= actor->floorz && P_HitFloor(actor) != FLOOR_SOLID)
    {
        // Landed in some sort of liquid
        P_RemoveMobj(actor);
        return;
    }

    if (actor->z != actor->floorz || actor->momz < 2 * FRACUNIT)
    {
        // Explode
        actor->momx = actor->momy = actor->momz = 0;
        actor->flags |= MF_NOGRAVITY;
        actor->flags3 &= ~(MF3_LOGRAV | MF3_FLOORBOUNCE);
    }
    else
    {
        mobj_t  *tiny;
        angle_t angle;

        // Bounce
        actor->momz = (actor->momz * 192) >> 8;
        P_SetMobjState(actor, actor->info->spawnstate);

        tiny = P_SpawnMobj(actor->x, actor->y, actor->z, HMT_MACEFX3);
        angle = actor->angle + ANG90;
        tiny->target = actor->target;
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = (actor->momx >> 1) + FixedMul(actor->momz - FRACUNIT, finecosine[angle]);
        tiny->momy = (actor->momy >> 1) + FixedMul(actor->momz - FRACUNIT, finesine[angle]);
        tiny->momz = actor->momz;
        P_CheckMissileSpawn(tiny);

        tiny = P_SpawnMobj(actor->x, actor->y, actor->z, HMT_MACEFX3);
        angle = actor->angle - ANG90;
        tiny->target = actor->target;
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = (actor->momx >> 1) + FixedMul(actor->momz - FRACUNIT, finecosine[angle]);
        tiny->momy = (actor->momy >> 1) + FixedMul(actor->momz - FRACUNIT, finesine[angle]);
        tiny->momz = actor->momz;
        P_CheckMissileSpawn(tiny);
    }
}

void A_FireMacePL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = P_SpawnPlayerMissile(player->mo, HMT_MACEFX4);

    player->ammo[am_mace] -= USE_MACE_AMMO_2;

    if (mo)
    {
        mo->momx += player->mo->momx;
        mo->momy += player->mo->momy;
        mo->momz = 2 * FRACUNIT + (player->lookdir << (FRACBITS - 5));

        if (linetarget)
            mo->special1.m = linetarget;
    }

    S_StartSound(player->mo, hsfx_lobsht);
}

void A_DeathBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (actor->z <= actor->floorz && P_HitFloor(actor) != FLOOR_SOLID)
    {
        // Landed in some sort of liquid
        P_RemoveMobj(actor);
        return;
    }

    if (actor->z <= actor->floorz && actor->momz)
    {
        // Bounce
        dboolean    newangle = false;
        mobj_t      *target = (mobj_t *)actor->special1.m;
        angle_t     angle;

        if (target)
        {
            if (!(target->flags & MF_SHOOTABLE))
                // Target died
                actor->special1.m = NULL;
            else
            {
                // Seek
                angle = R_PointToAngle2(actor->x, actor->y, target->x, target->y);
                newangle = true;
            }
        }
        else
        {
            // Find new target
            angle = 0;

            for (int i = 0; i < 16; i++)
            {
                P_AimLineAttack(actor, angle, 10 * 64 * FRACUNIT);

                if (linetarget && actor->target != linetarget)
                {
                    actor->special1.m = linetarget;
                    angle = R_PointToAngle2(actor->x, actor->y, linetarget->x, linetarget->y);
                    newangle = true;
                    break;
                }
                angle += ANG45 / 2;
            }
        }

        if (newangle)
        {
            actor->angle = angle;
            angle >>= ANGLETOFINESHIFT;
            actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
            actor->momy = FixedMul(actor->info->speed, finesine[angle]);
        }

        P_SetMobjState(actor, actor->info->spawnstate);
        S_StartSound(actor, sfx_pstop);
    }
    else
    {
        // Explode
        actor->flags |= MF_NOGRAVITY;
        actor->flags3 &= ~MF3_LOGRAV;
        S_StartSound(actor, hsfx_phohit);
    }
}

void A_SpawnRippers(mobj_t *actor, player_t *player, pspdef_t *psp)
{

    for (unsigned int i = 0; i < 8; i++)
    {
        mobj_t  *ripper = P_SpawnMobj(actor->x, actor->y, actor->z, HMT_RIPPER);
        angle_t angle = i * ANG45;

        ripper->target = actor->target;
        ripper->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        ripper->momx = FixedMul(ripper->info->speed, finecosine[angle]);
        ripper->momy = FixedMul(ripper->info->speed, finesine[angle]);
        P_CheckMissileSpawn(ripper);
    }
}

void A_FireCrossbowPL1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;

    player->ammo[am_crossbow] -= USE_CBOW_AMMO_1;
    P_SpawnPlayerMissile(mo, HMT_CRBOWFX1);
    P_SPMAngle(mo, HMT_CRBOWFX3, mo->angle - ANG45 / 10);
    P_SPMAngle(mo, HMT_CRBOWFX3, mo->angle + ANG45 / 10);
}

void A_FireCrossbowPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo = player->mo;

    player->ammo[am_crossbow] -= USE_CBOW_AMMO_2;
    P_SpawnPlayerMissile(mo, HMT_CRBOWFX2);
    P_SPMAngle(mo, HMT_CRBOWFX2, mo->angle - ANG45 / 10);
    P_SPMAngle(mo, HMT_CRBOWFX2, mo->angle + ANG45 / 10);
    P_SPMAngle(mo, HMT_CRBOWFX3, mo->angle - ANG45 / 5);
    P_SPMAngle(mo, HMT_CRBOWFX3, mo->angle + ANG45 / 5);
}

void A_BoltSpark(mobj_t *actor, player_t *player, pspdef_t *psp)
{

    if (M_Random() > 50)
    {
        mobj_t  *spark = P_SpawnMobj(actor->x, actor->y, actor->z, HMT_CRBOWFX4);

        spark->x += M_NegRandom() << 10;
        spark->y += M_NegRandom() << 10;
    }
}

void A_FireSkullRodPL1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo;

    if (player->ammo[am_skullrod] < USE_SKRD_AMMO_1)
        return;

    player->ammo[am_skullrod] -= USE_SKRD_AMMO_1;
    mo = P_SpawnPlayerMissile(player->mo, HMT_HORNRODFX1);

    // Randomize the first frame
    if (mo && M_Random() > 128)
        P_SetMobjState(mo, HS_HRODFX1_2);
}

void A_FireSkullRodPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    player->ammo[am_skullrod] -= USE_SKRD_AMMO_2;

    P_SpawnPlayerMissile(player->mo, HMT_HORNRODFX2);

    missilemobj->special2.i = 2;

    if (linetarget)
        missilemobj->special1.m = linetarget;

    S_StartSound(missilemobj, hsfx_hrnpow);
}

void A_SkullRodPL2Seek(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_SeekerMissile(actor, ANG1_X * 10, ANG1_X * 30);
}

void A_AddPlayerRain(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (viewplayer->health <= 0)
        return;

    if (viewplayer->rain1 && viewplayer->rain2)
    {
        // Terminate an active rain
        if (viewplayer->rain1->health < viewplayer->rain2->health)
        {
            if (viewplayer->rain1->health > 16)
                viewplayer->rain1->health = 16;

            viewplayer->rain1 = NULL;
        }
        else
        {
            if (viewplayer->rain2->health > 16)
                viewplayer->rain2->health = 16;

            viewplayer->rain2 = NULL;
        }
    }
    // Add rain mobj to list
    if (viewplayer->rain1)
        viewplayer->rain2 = actor;
    else
        viewplayer->rain1 = actor;
}

void A_SkullRodStorm(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    fixed_t x, y;
    mobj_t  *mo;

    if (!actor->health--)
    {
        P_SetMobjState(actor, S_NULL);

        if (viewplayer->health <= 0)
            return;

        if (viewplayer->rain1 == actor)
            viewplayer->rain1 = NULL;
        else if (viewplayer->rain2 == actor)
            viewplayer->rain2 = NULL;

        return;
    }

    if (M_Random() < 25)
        return;

    x = actor->x + ((M_Random() & 127) - 64) * FRACUNIT;
    y = actor->y + ((M_Random() & 127) - 64) * FRACUNIT;
    mo = P_SpawnMobj(x, y, ONCEILINGZ, HMT_RAINPLR1 + actor->special2.i);
    mo->target = actor->target;
    mo->momx = 1;                       // Force collision detection
    mo->momz = -mo->info->speed;
    mo->special2.i = actor->special2.i; // Transfer player number
    P_CheckMissileSpawn(mo);

    if (!(actor->special1.i & 31))
        S_StartSound(actor, hsfx_ramrain);

    actor->special1.i++;
}

void A_RainImpact(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (actor->z > actor->floorz)
        P_SetMobjState(actor, HS_RAINAIRXPLR1_1 + actor->special2.i);
    else if (M_Random() < 40)
        P_HitFloor(actor);
}

void A_HideInCeiling(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    actor->z = actor->ceilingz + 4 * FRACUNIT;
}

void A_FirePhoenixPL1(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t angle;

    player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_1;
    P_SpawnPlayerMissile(player->mo, HMT_PHOENIXFX1);
    angle = (player->mo->angle + ANG180) >> ANGLETOFINESHIFT;
    player->mo->momx += FixedMul(4 * FRACUNIT, finecosine[angle]);
    player->mo->momy += FixedMul(4 * FRACUNIT, finesine[angle]);
}

void A_PhoenixPuff(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *puff = P_SpawnMobj(actor->x, actor->y, actor->z, HMT_PHOENIXPUFF);
    angle_t angle = (actor->angle + ANG90) >> ANGLETOFINESHIFT;

    P_SeekerMissile(actor, ANG1_X * 5, ANG1_X * 10);
    puff->momx = FixedMul((fixed_t)(FRACUNIT * 1.3), finecosine[angle]);
    puff->momy = FixedMul((fixed_t)(FRACUNIT * 1.3), finesine[angle]);
    puff->momz = 0;
    puff = P_SpawnMobj(actor->x, actor->y, actor->z, HMT_PHOENIXPUFF);
    angle = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
    puff->momx = FixedMul((fixed_t)(FRACUNIT * 1.3), finecosine[angle]);
    puff->momy = FixedMul((fixed_t)(FRACUNIT * 1.3), finesine[angle]);
    puff->momz = 0;
}

void A_InitPhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    player->flamecount = FLAMETHROWERTICS;
}

void A_FirePhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo;
    mobj_t  *pmo;
    angle_t angle;
    fixed_t x, y, z;
    fixed_t slope;

    if (!--player->flamecount)
    {
        // Out of flame
        P_SetPsprite(ps_weapon, HS_PHOENIXATK2_4);
        player->refire = 0;
        return;
    }

    pmo = player->mo;
    angle = pmo->angle;
    x = pmo->x + (M_NegRandom() << 9);
    y = pmo->y + (M_NegRandom() << 9);
    z = pmo->z + 26 * FRACUNIT + (player->lookdir << FRACBITS) / 173;

    if (pmo->flags2 & MF2_FEETARECLIPPED)
        z -= FOOTCLIPSIZE;

    slope = (player->lookdir << FRACBITS) / 173 + FRACUNIT / 10;
    mo = P_SpawnMobj(x, y, z, HMT_PHOENIXFX2);
    mo->target = pmo;
    mo->angle = angle;
    mo->momx = pmo->momx + FixedMul(mo->info->speed, finecosine[angle >> ANGLETOFINESHIFT]);
    mo->momy = pmo->momy + FixedMul(mo->info->speed, finesine[angle >> ANGLETOFINESHIFT]);
    mo->momz = FixedMul(mo->info->speed, slope);

    if (!player->refire || !(leveltime % 38))
        S_StartSound(player->mo, hsfx_phopow);

    P_CheckMissileSpawn(mo);
}

void A_ShutdownPhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
}

void A_FlameEnd(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    actor->momz += (fixed_t)(1.5 * FRACUNIT);
}

void A_FloatPuff(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    actor->momz += (fixed_t)(1.8 * FRACUNIT);
}

void A_GauntletAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t angle = player->mo->angle;
    int     damage;
    int     slope;
    int     randval;
    fixed_t dist;

    psp->sx = ((M_Random() & 3) - 2) * FRACUNIT;
    psp->sy = WEAPONTOP + (M_Random() & 3) * FRACUNIT;

    if (player->powers[pw_weaponlevel2])
    {
        damage = HITDICE(2);
        dist = 4 * MELEERANGE;
        angle += M_NegRandom() << 17;
        pufftype = HMT_GAUNTLETPUFF2;
    }
    else
    {
        damage = HITDICE(2);
        dist = MELEERANGE + 1;
        angle += M_NegRandom() << 18;
        pufftype = HMT_GAUNTLETPUFF1;
    }

    slope = P_AimLineAttack(player->mo, angle, dist);
    P_LineAttack(player->mo, angle, dist, slope, damage);

    if (!linetarget)
    {
        if (M_Random() > 64)
            player->extralight = !player->extralight;

        S_StartSound(player->mo, hsfx_gntful);
        return;
    }

    randval = M_Random();

    if (randval < 64)
        player->extralight = 0;
    else if (randval < 160)
        player->extralight = 1;
    else
        player->extralight = 2;

    if (player->powers[pw_weaponlevel2])
    {
        P_GiveBody(damage >> 1, false);
        S_StartSound(player->mo, hsfx_gntpow);
    }
    else
        S_StartSound(player->mo, hsfx_gnthit);

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
// P_GunShot
//
static void P_GunShot(mobj_t *actor, dboolean accurate)
{
    angle_t angle = actor->angle;

    if (!accurate)
        angle += M_NegRandom() << 18;

    P_LineAttack(actor, angle, MISSILERANGE, bulletslope, 5 * (M_Random() % 3 + 1));
}

//
// A_FirePistol
//
void A_FirePistol(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_NoiseAlert(actor);

    S_StartSound(actor, sfx_pistol);

    P_SubtractAmmo(wp_pistol, 1);

    P_SetPsprite(ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    P_GunShot(actor, !player->refire);

    A_Recoil(wp_pistol);

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
    P_NoiseAlert(actor);

    S_StartSound(actor, sfx_shotgn);

    P_SubtractAmmo(wp_shotgun, 1);

    P_SetPsprite(ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    for (int i = 0; i < 7; i++)
        P_GunShot(actor, false);

    A_Recoil(wp_shotgun);

    player->shotsfired++;
    stat_shotsfired = SafeAdd(stat_shotsfired, 1);

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
    P_NoiseAlert(actor);

    S_StartSound(actor, sfx_dshtgn);

    P_SubtractAmmo(wp_supershotgun, 2);

    P_SetPsprite(ps_flash, weaponinfo[player->readyweapon].flashstate);

    P_BulletSlope(actor);

    successfulshot = false;

    for (int i = 0; i < 20; i++)
        P_LineAttack(actor, actor->angle + (M_NegRandom() << ANGLETOFINESHIFT), MISSILERANGE,
            bulletslope + (M_NegRandom() << 5), 5 * (M_Random() % 3 + 1));

    A_Recoil(wp_supershotgun);

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
    // [BH] Fix <https://doomwiki.org/wiki/Chaingun_makes_two_sounds_firing_single_bullet>.
    if (!player->ammo[weaponinfo[player->readyweapon].ammotype])
        return;

    P_NoiseAlert(actor);
    S_StartSound(actor, sfx_pistol);

    P_SubtractAmmo(wp_chaingun, 1);

    P_SetPsprite(ps_flash, weaponinfo[player->readyweapon].flashstate + (unsigned int)((psp->state - &states[S_CHAIN1]) & 1));

    P_BulletSlope(actor);

    successfulshot = false;

    P_GunShot(actor, !player->refire);

    A_Recoil(wp_chaingun);

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
        P_NoiseAlert(mo->player->mo);

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
void P_SetupPsprites(void)
{
    // remove all psprites
    viewplayer->psprites[ps_weapon].state = NULL;
    viewplayer->psprites[ps_flash].state = NULL;

    // spawn the gun
    viewplayer->pendingweapon = viewplayer->readyweapon;
    P_BringUpWeapon();
    skippsprinterp = true;
}

//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites(void)
{
    pspdef_t    *psp = viewplayer->psprites;
    pspdef_t    *weapon = &psp[ps_weapon];
    pspdef_t    *flash = &psp[ps_flash];

    if (weapon->state && weapon->tics != -1 && !--weapon->tics)
        P_SetPsprite(ps_weapon, weapon->state->nextstate);

    if (flash->state && flash->tics != -1 && !--flash->tics)
        P_SetPsprite(ps_flash, flash->state->nextstate);

    if (weapon->state->action == A_WeaponReady)
    {
        // bob the weapon based on movement speed
        fixed_t momx = viewplayer->momx;
        fixed_t momy = viewplayer->momy;
        fixed_t bob;

        if (momx | momy)
            bob = MAX(MIN((FixedMul(momx, momx) + FixedMul(momy, momy)) >> 2, MAXBOB) * weaponbob / 100, MAXBOB * stillbob / 400);
        else
            bob = MAXBOB * stillbob / 400;

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
    if (viewplayer->readyweapon == wp_bfg && weaponrecoil && canmouselook)
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
