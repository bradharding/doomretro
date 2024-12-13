/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_controller.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"

#define LOWERSPEED  (6 * FRACUNIT)
#define RAISESPEED  (6 * FRACUNIT)

bool    successfulshot;
bool    skippsprinterp;

//
// A_Recoil
//
void A_Recoil(const weapontype_t weapon)
{
    if (weaponrecoil)
        viewplayer->recoil = weaponinfo[weapon].recoil;
}

//
// MBF21: P_SetPlayerSpritePtr
//
static void P_SetPlayerSpritePtr(pspdef_t *psp, statenum_t stnum)
{
    do
    {
        if (!stnum)
        {
            // object removed itself
            psp->state = NULL;
            break;
        }
        else
        {
            state_t *state = &states[stnum];

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
        }
    } while (!psp->tics);           // an initial state of 0 could cycle through
}

//
// P_SetPlayerSprite
//
void P_SetPlayerSprite(const size_t position, const statenum_t stnum)
{
    P_SetPlayerSpritePtr(&viewplayer->psprites[position], stnum);
}

//
// P_BringUpWeapon
// Starts bringing the pending weapon up from the bottom of the screen.
//
static void P_BringUpWeapon(void)
{
    statenum_t  newstate;

    if (viewplayer->pendingweapon == wp_nochange)
        viewplayer->pendingweapon = viewplayer->readyweapon;
    else if (viewplayer->pendingweapon == wp_chainsaw)
        S_StartSound(viewplayer->mo, sfx_sawup);

    newstate = weaponinfo[viewplayer->pendingweapon].upstate;

    viewplayer->pendingweapon = wp_nochange;
    viewplayer->psprites[ps_weapon].sy = WEAPONBOTTOM;

    P_SetPlayerSprite(ps_weapon, newstate);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
bool P_CheckAmmo(const weapontype_t weapon)
{
    const ammotype_t    ammotype = weaponinfo[weapon].ammotype;

    // Some do not need ammunition anyway.
    if (ammotype == am_noammo || infiniteammo)
        return true;

    // Return if current ammunition sufficient.
    if (viewplayer->ammo[ammotype] >= weaponinfo[weapon].ammopershot && viewplayer->weaponowned[weapon])
        return true;

    // Out of ammo, pick a weapon to change to.
    if (viewplayer->weaponowned[wp_plasma]
        && viewplayer->ammo[weaponinfo[wp_plasma].ammotype] >= weaponinfo[wp_plasma].ammopershot)
        viewplayer->pendingweapon = wp_plasma;
    else if (viewplayer->weaponowned[wp_supershotgun]
        && viewplayer->ammo[weaponinfo[wp_supershotgun].ammotype] >= weaponinfo[wp_supershotgun].ammopershot
        && viewplayer->preferredshotgun == wp_supershotgun)
        viewplayer->pendingweapon = wp_supershotgun;
    else if (viewplayer->weaponowned[wp_chaingun]
        && viewplayer->ammo[weaponinfo[wp_chaingun].ammotype] >= weaponinfo[wp_chaingun].ammopershot)
        viewplayer->pendingweapon = wp_chaingun;
    else if (viewplayer->weaponowned[wp_shotgun]
        && viewplayer->ammo[weaponinfo[wp_shotgun].ammotype] >= weaponinfo[wp_shotgun].ammopershot)
        viewplayer->pendingweapon = wp_shotgun;
    else if (viewplayer->weaponowned[wp_pistol]
        && viewplayer->ammo[weaponinfo[wp_pistol].ammotype] >= weaponinfo[wp_pistol].ammopershot)
        viewplayer->pendingweapon = wp_pistol;
    else if (viewplayer->weaponowned[wp_chainsaw])
        viewplayer->pendingweapon = wp_chainsaw;
    else
        viewplayer->pendingweapon = wp_fist;

    return false;
}

//
// P_SubtractAmmo
//
static void P_SubtractAmmo(void)
{
    if (!infiniteammo)
    {
        const weapontype_t  readyweapon = viewplayer->readyweapon;
        const ammotype_t    ammotype = weaponinfo[readyweapon].ammotype;

        if (ammotype != am_noammo)
        {
            const int   value = MAX(0, viewplayer->ammo[ammotype] - weaponinfo[readyweapon].ammopershot);

            P_AnimateAmmo(viewplayer->ammo[ammotype] - value, ammotype);
            viewplayer->ammo[ammotype] = value;
        }
    }

    ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
}

//
// P_RumbleWeapon
//
static void P_RumbleWeapon(const weapontype_t weapon)
{
    if (joy_rumble_weapons)
    {
        const weaponinfo_t  readyweaponinfo = weaponinfo[weapon];

        I_ControllerRumble(readyweaponinfo.lowrumble * joy_rumble_weapons / 100,
            readyweaponinfo.highrumble * joy_rumble_weapons / 100);
        weaponrumbletics = readyweaponinfo.tics;
    }
}

//
// P_FireWeapon
//
void P_FireWeapon(void)
{
    const weapontype_t  readyweapon = viewplayer->readyweapon;

    if (!P_CheckAmmo(readyweapon) || (automapactive && !am_followmode))
        return;

    P_SetMobjState(viewplayer->mo, S_PLAY_ATK1);
    P_SetPlayerSprite(ps_weapon, weaponinfo[readyweapon].atkstate);

    if (readyweapon == wp_bfg)
        P_RumbleWeapon(wp_bfg);

    if (centerweapon)
    {
        pspdef_t        *psp = &viewplayer->psprites[ps_weapon];
        const state_t   *state = psp->state;

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
    P_SetPlayerSprite(ps_weapon, weaponinfo[viewplayer->readyweapon].downstate);
}

//
// A_WeaponReady
// The player can fire the weapon or change to another weapon at this time.
// Follows after getting weapon up, or after previous attack/fire sequence.
//
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const weapontype_t  readyweapon = player->readyweapon;
    const weapontype_t  pendingweapon = player->pendingweapon;

    // get out of attack state
    if (player->mo->state == &states[S_PLAY_ATK1] || player->mo->state == &states[S_PLAY_ATK2])
        P_SetMobjState(player->mo, S_PLAY);

    if (readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
    {
        S_StopSound(sfx_sawup);
        S_StartSound(actor, sfx_sawidl);
    }

    // check for change
    //  if player is dead, put the weapon away
    if (pendingweapon != wp_nochange || player->health <= 0)
    {
        if (joy_rumble_weapons)
        {
            if (pendingweapon == wp_chainsaw && !REKKR)
            {
                idlechainsawrumblestrength = IDLE_CHAINSAW_RUMBLE_STRENGTH * joy_rumble_weapons / 100;
                I_ControllerRumble(idlechainsawrumblestrength, idlechainsawrumblestrength);
            }
            else if (idlechainsawrumblestrength)
            {
                idlechainsawrumblestrength = 0;
                I_StopControllerRumble();
            }
        }

        P_DropWeapon();
        return;
    }

    // check for fire
    //  the missile launcher and BFG do not auto fire
    if (player->cmd.buttons & BT_ATTACK)
    {
        if (!player->attackdown || !(weaponinfo[readyweapon].flags & WPF_NOAUTOFIRE))
        {
            player->attackdown = true;
            P_FireWeapon();
        }
    }
    else
    {
        player->attackdown = false;

        if (joy_rumble_weapons && readyweapon == wp_chainsaw && !REKKR)
        {
            idlechainsawrumblestrength = IDLE_CHAINSAW_RUMBLE_STRENGTH * joy_rumble_weapons / 100;
            I_ControllerRumble(idlechainsawrumblestrength, idlechainsawrumblestrength);
        }
    }
}

//
// A_ReFire
// The player can refire the weapon without lowering it entirely.
//
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    // check for fire (if a weapon change is pending, let it go through instead)
    if ((player->cmd.buttons & BT_ATTACK) && player->pendingweapon == wp_nochange && player->health > 0)
    {
        player->refire++;
        P_FireWeapon();
    }
    else
    {
        player->refire = 0;
        P_CheckAmmo(player->readyweapon);
    }
}

void A_CheckReload(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    if (!P_CheckAmmo(player->readyweapon))
        P_SetPlayerSprite(ps_weapon, weaponinfo[player->readyweapon].downstate);
}

//
// A_Lower
// Lowers current weapon, and changes weapon at bottom.
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

    // Player is dead, so keep the weapon off screen.
    if (player->health <= 0)
    {
        P_SetPlayerSprite(ps_weapon, S_NULL);
        return;
    }

    // The old weapon has been lowered off the screen,
    // so change the weapon and start raising it
    if (player->pendingweapon != wp_nochange)
        player->readyweapon = player->pendingweapon;

    P_BringUpWeapon();
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
    P_SetPlayerSprite(ps_weapon, weaponinfo[player->readyweapon].readystate);
}

//
// A_GunFlash
//
void A_GunFlash(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_SetMobjState(player->mo, S_PLAY_ATK2);
    P_SetPlayerSprite(ps_flash, weaponinfo[player->readyweapon].flashstate);
}

//
// WEAPON ATTACKS
//

//
// A_Punch
//
void A_Punch(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const angle_t       angle = actor->angle + (M_SubRandom() << 18);
    const int           range = player->mo->info->meleerange;
    int                 slope = P_AimLineAttack(actor, angle, range, MF_FRIEND);
    const int           isberserk = player->powers[pw_strength];
    int                 damage = (M_Random() % 10 + 1) << 1;
    const weapontype_t  readyweapon = player->readyweapon;

    if (isberserk)
        damage *= 10;

    if (!linetarget)
        slope = P_AimLineAttack(actor, angle, range, 0);

    hitwall = false;
    P_LineAttack(actor, angle, range, slope, damage);

    player->shotsfired[readyweapon]++;
    stat_shotsfired[wp_fist] = SafeAdd(stat_shotsfired[wp_fist], 1);

    if (linetarget || hitwall)
    {
        if (!(weaponinfo[readyweapon].flags & WPF_SILENT))
            P_NoiseAlert(actor);

        S_StartSound(actor, sfx_punch);

        // turn to face target
        if (linetarget)
        {
            if (isberserk && r_shake_berserk)
            {
                shakeduration = BERSERKPUNCHMONSTER;
                shake = I_GetTimeMS() + shakeduration;
            }

            actor->angle = R_PointToAngle2(actor->x, actor->y, linetarget->x, linetarget->y);

            player->shotssuccessful[readyweapon]++;
            stat_shotssuccessful[wp_fist] = SafeAdd(stat_shotssuccessful[wp_fist], 1);
        }
        else if (isberserk && r_shake_berserk)
        {
            shakeduration = BERSERKPUNCHWALL;
            shake = I_GetTimeMS() + shakeduration;
        }

        P_RumbleWeapon(readyweapon);
    }
}

//
// A_Saw
//
void A_Saw(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    angle_t             angle = actor->angle + (M_SubRandom() << 18);
    const int           range = viewplayer->mo->info->meleerange + 1;
    int                 slope = P_AimLineAttack(actor, angle, range, MF_FRIEND);
    const weapontype_t  readyweapon = player->readyweapon;

    if (!linetarget)
        slope = P_AimLineAttack(actor, angle, range, 0);

    P_LineAttack(actor, angle, range, slope, 2 * (M_Random() % 10 + 1));
    A_Recoil(readyweapon);
    P_RumbleWeapon(readyweapon);

    if (!(weaponinfo[readyweapon].flags & WPF_SILENT))
        P_NoiseAlert(actor);

    player->shotsfired[readyweapon]++;
    stat_shotsfired[wp_chainsaw] = SafeAdd(stat_shotsfired[wp_chainsaw], 1);
    idlechainsawrumblestrength = 0;

    if (!linetarget)
    {
        S_StartSound(actor, sfx_sawful);
        return;
    }

    player->shotssuccessful[readyweapon]++;
    stat_shotssuccessful[wp_chainsaw] = SafeAdd(stat_shotssuccessful[wp_chainsaw], 1);

    S_StartSound(actor, sfx_sawhit);

    // turn to face target
    angle = R_PointToAngle2(actor->x, actor->y, linetarget->x, linetarget->y);

    if (angle - actor->angle > ANG180)
    {
        if ((int)(angle - actor->angle) < -ANG90 / 20)
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
    const weapontype_t  readyweapon = player->readyweapon;

    P_SubtractAmmo();
    P_SpawnPlayerMissile(actor, MT_ROCKET);
    P_RumbleWeapon(readyweapon);

    player->shotsfired[readyweapon]++;
    stat_shotsfired[wp_missile] = SafeAdd(stat_shotsfired[wp_missile], 1);
}

//
// A_FireBFG
//
void A_FireBFG(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_SubtractAmmo();
    P_SpawnPlayerMissile(actor, MT_BFG);
}

//
// A_FireOldBFG
//
// This function emulates DOOM's Pre-Beta BFG
// By Lee Killough 06/06/98, 07/11/98, 07/19/98, 08/20/98
//
// This code may not be used in other mods without appropriate credit given.
// Code leeches will be telefragged.
//
void A_FireOldBFG(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobjtype_t  type = MT_PLASMA1;

    P_SubtractAmmo();

    player->extralight = 2;

    do
    {
        mobj_t  *th;
        angle_t an = actor->angle;
        angle_t an1 = ((M_Random() & 127) - 64) * (ANG90 / 768) + an;
        angle_t an2 = ((M_Random() & 127) - 64) * (ANG90 / 640) + ANG90;
        fixed_t slope;

        if (usefreelook && !autoaim)
            slope = PLAYERSLOPE(player);
        else
        {
            // killough 08/02/98: make autoaiming prefer enemies
            int mask = MF_FRIEND;

            do
            {
                slope = P_AimLineAttack(actor, an, 16 * 64 * FRACUNIT, mask);

                if (!linetarget)
                {
                    slope = P_AimLineAttack(actor, (an += (1 << 26)), 16 * 64 * FRACUNIT, mask);

                    if (!linetarget)
                    {
                        slope = P_AimLineAttack(actor, (an -= (2 << 26)), 16 * 64 * FRACUNIT, mask);

                        if (!linetarget)
                        {
                            an = actor->angle;
                            slope = (usefreelook ? PLAYERSLOPE(player) : 0);
                        }
                    }
                }
            } while (mask && (mask = 0, !linetarget));  // killough 08/02/98
        }

        an1 += an - actor->angle;

        // [crispy] consider negative slope
        if (slope < 0)
            an2 -= tantoangle[-slope >> DBITS];
        else
            an2 += tantoangle[slope >> DBITS];

        th = P_SpawnMobj(actor->x, actor->y, actor->z + 62 * FRACUNIT - player->psprites[ps_weapon].sy, type);
        P_SetTarget(&th->target, actor);
        th->angle = an1;
        th->momx = finecosine[an1 >> ANGLETOFINESHIFT] * 25;
        th->momy = finesine[an1 >> ANGLETOFINESHIFT] * 25;
        th->momz = finetangent[an2 >> ANGLETOFINESHIFT] * 25;

        // [crispy] suppress interpolation of player missiles for the first tic
        th->interpolate = -1;

        P_CheckMissileSpawn(th);
    } while (type != MT_PLASMA2 && (type = MT_PLASMA2));  // killough: obfuscated!

    A_Recoil(wp_bfg);
}

//
// A_FirePlasma
//
void A_FirePlasma(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const weapontype_t  readyweapon = player->readyweapon;

    P_SubtractAmmo();
    P_SetPlayerSprite(ps_flash, weaponinfo[readyweapon].flashstate + (M_Random() & 1));
    P_SpawnPlayerMissile(actor, MT_PLASMA);

    player->shotsfired[readyweapon]++;
    stat_shotsfired[wp_plasma] = SafeAdd(stat_shotsfired[wp_plasma], 1);
    P_RumbleWeapon(readyweapon);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at approximately
// the height of the intended target
//
static fixed_t  bulletslope;

static void P_BulletSlope(mobj_t *actor)
{
    if (usefreelook && !autoaim)
        bulletslope = PLAYERSLOPE(viewplayer);
    else
    {
        // killough 08/02/98: make autoaiming prefer enemies
        int mask = MF_FRIEND;

        do
        {
            angle_t an = actor->angle;

            // see which target is to be aimed at
            bulletslope = P_AimLineAttack(actor, an, 16 * 64 * FRACUNIT, mask);

            if (!linetarget)
            {
                bulletslope = P_AimLineAttack(actor, (an += (1 << 26)), 16 * 64 * FRACUNIT, mask);

                if (!linetarget)
                {
                    bulletslope = P_AimLineAttack(actor, an - (2 << 26), 16 * 64 * FRACUNIT, mask);

                    if (!linetarget && usefreelook)
                        bulletslope = PLAYERSLOPE(viewplayer);
                }
            }
        } while (mask && (mask = 0, !linetarget));  // killough 08/02/98
    }
}

//
// P_GunShot
//
static void P_GunShot(mobj_t *actor, bool accurate)
{
    angle_t angle = actor->angle;

    if (!accurate)
        angle += (M_SubRandom() << 18);

    P_LineAttack(actor, angle, MISSILERANGE, bulletslope, 5 * (M_Random() % 3 + 1));
}

//
// A_FirePistol
//
void A_FirePistol(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const weapontype_t  readyweapon = player->readyweapon;
    const weaponinfo_t  readyweaponinfo = weaponinfo[readyweapon];

    if (!(weaponinfo[player->readyweapon].flags & WPF_SILENT))
        P_NoiseAlert(actor);

    S_StartSound(actor, sfx_pistol);
    P_SetMobjState(player->mo, S_PLAY_ATK2);
    P_SubtractAmmo();
    P_SetPlayerSprite(ps_flash, readyweaponinfo.flashstate);
    P_BulletSlope(actor);

    successfulshot = false;

    P_GunShot(actor, !player->refire);
    A_Recoil(readyweapon);
    P_RumbleWeapon(readyweapon);

    player->shotsfired[readyweapon]++;
    stat_shotsfired[wp_pistol] = SafeAdd(stat_shotsfired[wp_pistol], 1);

    if (successfulshot)
    {
        player->shotssuccessful[readyweapon]++;
        stat_shotssuccessful[wp_pistol] = SafeAdd(stat_shotssuccessful[wp_pistol], 1);
    }
}

//
// A_FireShotgun
//
void A_FireShotgun(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const weapontype_t  readyweapon = player->readyweapon;
    const weaponinfo_t  readyweaponinfo = weaponinfo[readyweapon];

    if (!(readyweaponinfo.flags & WPF_SILENT))
        P_NoiseAlert(actor);

    S_StartSound(actor, sfx_shotgn);
    P_SetMobjState(player->mo, S_PLAY_ATK2);
    P_SubtractAmmo();
    P_SetPlayerSprite(ps_flash, readyweaponinfo.flashstate);
    P_BulletSlope(actor);

    successfulshot = false;

    for (int i = 0; i < 7; i++)
        P_GunShot(actor, false);

    A_Recoil(readyweapon);
    P_RumbleWeapon(readyweapon);

    player->shotsfired[readyweapon]++;
    stat_shotsfired[wp_shotgun] = SafeAdd(stat_shotsfired[wp_shotgun], 1);

    if (successfulshot)
    {
        player->shotssuccessful[readyweapon]++;
        stat_shotssuccessful[wp_shotgun] = SafeAdd(stat_shotssuccessful[wp_shotgun], 1);
    }

    player->preferredshotgun = readyweapon;
}

//
// A_FireShotgun2
//
void A_FireShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    const weapontype_t  readyweapon = player->readyweapon;
    const weaponinfo_t  readyweaponinfo = weaponinfo[readyweapon];

    if (!(readyweaponinfo.flags & WPF_SILENT))
        P_NoiseAlert(actor);

    S_StartSound(actor, sfx_dshtgn);
    P_SetMobjState(player->mo, S_PLAY_ATK2);
    P_SubtractAmmo();
    P_SetPlayerSprite(ps_flash, readyweaponinfo.flashstate);
    P_BulletSlope(actor);

    successfulshot = false;

    for (int i = 0; i < 20; i++)
        P_LineAttack(actor, actor->angle + (M_SubRandom() << ANGLETOFINESHIFT), MISSILERANGE,
            bulletslope + (M_SubRandom() << 5), 5 * (M_Random() % 3 + 1));

    A_Recoil(readyweapon);
    P_RumbleWeapon(readyweapon);

    player->shotsfired[readyweapon]++;
    stat_shotsfired[wp_supershotgun] = SafeAdd(stat_shotsfired[wp_supershotgun], 1);

    if (successfulshot)
    {
        player->shotssuccessful[readyweapon]++;
        stat_shotssuccessful[wp_supershotgun] = SafeAdd(stat_shotssuccessful[wp_supershotgun], 1);
    }

    player->preferredshotgun = readyweapon;
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
    const weapontype_t  readyweapon = player->readyweapon;
    const weaponinfo_t  readyweaponinfo = weaponinfo[readyweapon];

    // [BH] Fix <https://doomwiki.org/wiki/Chaingun_makes_two_sounds_firing_single_bullet>.
    if (!player->ammo[readyweaponinfo.ammotype] && !infiniteammo)
        return;

    P_SetMobjState(player->mo, S_PLAY_ATK2);
    S_StartSound(actor, sfx_pistol);

    if (!(readyweaponinfo.flags & WPF_SILENT))
        P_NoiseAlert(actor);

    P_SubtractAmmo();
    P_SetPlayerSprite(ps_flash, readyweaponinfo.flashstate + (unsigned int)((psp->state - &states[S_CHAIN1]) & 1));
    P_BulletSlope(actor);

    successfulshot = false;

    P_GunShot(actor, !player->refire);
    A_Recoil(readyweapon);
    P_RumbleWeapon(readyweapon);

    player->shotsfired[readyweapon]++;
    stat_shotsfired[wp_chaingun] = SafeAdd(stat_shotsfired[wp_chaingun], 1);

    if (successfulshot)
    {
        player->shotssuccessful[readyweapon]++;
        stat_shotssuccessful[wp_chaingun] = SafeAdd(stat_shotssuccessful[wp_chaingun], 1);
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
    mobj_t  *mo = actor->target;
    angle_t an = actor->angle - ANG90 / 2;

    if (!(weaponinfo[wp_bfg].flags & WPF_SILENT))
        P_NoiseAlert(actor);

    // offset angles from its attack angle
    for (int i = 0; i < 40; i++)
    {
        int damage = 15;

        // killough 08/02/98: make autoaiming prefer enemies
        if (P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT, MF_FRIEND), !linetarget)
            P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT, 0);

        an += ANG90 / 40;

        if (!linetarget)
            continue;

        successfulshot = true;

        P_SpawnMobj(linetarget->x, linetarget->y, linetarget->z + (linetarget->height >> 2), MT_EXTRABFG);

        for (int j = 0; j < 15; j++)
            damage += (M_Random() & 7);

        P_DamageMobj(linetarget, mo, mo, damage, true, false);
    }

    viewplayer->shotsfired[wp_bfg]++;
    stat_shotsfired[wp_bfg] = SafeAdd(stat_shotsfired[wp_bfg], 1);

    if (successfulshot)
    {
        viewplayer->shotssuccessful[wp_bfg]++;
        stat_shotssuccessful[wp_bfg] = SafeAdd(stat_shotssuccessful[wp_bfg], 1);
    }

    successfulshot = false;
}

//
// A_BFGSound
//
void A_BFGSound(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    S_StartSound(actor, sfx_bfg);
}

//
// P_SetupPlayerSprites
// Called at start of level for each player.
//
void P_SetupPlayerSprites(void)
{
    // remove all psprites
    viewplayer->psprites[ps_weapon].state = NULL;
    viewplayer->psprites[ps_weapon].tics = -1;
    viewplayer->psprites[ps_flash].state = NULL;
    viewplayer->psprites[ps_flash].tics = -1;

    // spawn the gun
    viewplayer->pendingweapon = viewplayer->readyweapon;
    P_BringUpWeapon();

    if (r_playersprites)
        skippsprinterp = true;
}

//
// P_MovePlayerSprites
// Called every tic by player thinking routine.
//
void P_MovePlayerSprites(void)
{
    pspdef_t    *psp = viewplayer->psprites;
    pspdef_t    *weapon = &psp[ps_weapon];
    pspdef_t    *flash = &psp[ps_flash];

    if (weapon->tics != -1 && !--weapon->tics)
        P_SetPlayerSprite(ps_weapon, weapon->state->nextstate);

    if (flash->tics != -1 && !--flash->tics)
        P_SetPlayerSprite(ps_flash, flash->state->nextstate);

    if (weapon->state->action == &A_WeaponReady)
    {
        // bob the weapon based on movement speed
        const fixed_t   momx = viewplayer->momx;
        const fixed_t   momy = viewplayer->momy;
        fixed_t         bob = MAXBOB * stillbob / 400;

        if (momx | momy)
            bob = MAX(MIN((FixedMul(momx, momx) + FixedMul(momy, momy)) >> 2, MAXBOB) * weaponbob / 100, bob);

        // [BH] smooth out weapon bob by zeroing out really small bobs
        if (bob < FRACUNIT / 2)
        {
            weapon->sx = 0;
            weapon->sy = WEAPONTOP;
        }
        else
        {
            const angle_t   angle = ((128 * maptime) & FINEMASK);

            weapon->sx = FixedMul(bob, finecosine[angle]);
            weapon->sy = WEAPONTOP + FixedMul(bob, finesine[angle & (FINEANGLES / 2 - 1)]);
        }
    }

    // [BH] shake the BFG before firing when weapon recoil enabled
    if (viewplayer->readyweapon == wp_bfg && weaponrecoil)
    {
        if (weapon->state == &states[S_BFG1])
        {
            weapon->sx = M_BigRandomInt(-2, 2) * FRACUNIT;
            weapon->sy = WEAPONTOP + M_BigRandomInt(-1, 1) * FRACUNIT;
        }
        else if (weapon->state == &states[S_BFG2])
        {
            weapon->sx = 0;
            weapon->sy = WEAPONTOP;
        }
    }

    // [BH] hack to make weapon less blurred when moving in low detail
    if (r_detail == r_detail_low)
    {
        weapon->sx = ((weapon->sx >> FRACBITS) << FRACBITS);
        weapon->sy = ((weapon->sy >> FRACBITS) << FRACBITS);
    }

    flash->sx = weapon->sx;
    flash->sy = weapon->sy;
}

//
// [XA] New MBF21 codepointers
//

//
// A_WeaponProjectile
// A parameterized player weapon projectile attack. Does not consume ammo.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling player's angle
//   args[2]: Pitch (degrees, in fixed point), relative to calling player's pitch; approximated
//   args[3]: X/Y spawn offset, relative to calling player's angle
//   args[4]: Z spawn offset, relative to player's default projectile fire height
//
void A_WeaponProjectile(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    mobj_t  *mo;
    int     an;
    state_t *state = psp->state;

    if (!state || !state->args[0])
        return;

    if (!(mo = P_SpawnPlayerMissile(player->mo, state->args[0] - 1)))
        return;

    if (legacyofrust && (M_BigRandom() & 1))
        mo->flags2 |= MF2_MIRRORED;

    // adjust angle
    mo->angle += (angle_t)(((int64_t)state->args[1] << 16) / 360);
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);

    // adjust pitch (approximated, using DOOM's ye olde finetangent table; same method as autoaim)
    mo->momz += FixedMul(mo->info->speed, DegToSlope(state->args[2]));

    // adjust position
    an = (player->mo->angle - ANG90) >> ANGLETOFINESHIFT;
    mo->x += FixedMul(state->args[3], finecosine[an]);
    mo->y += FixedMul(state->args[3], finesine[an]);
    mo->z += state->args[4];

    // set tracer to the player's autoaim target,
    // so player seeker missiles prioritizing the
    // baddie the player is actually aiming at. ;)
    mo->tracer = linetarget;

    ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

    P_RumbleWeapon(player->readyweapon);
}

//
// A_WeaponBulletAttack
// A parameterized player weapon bullet attack. Does not consume ammo.
//   args[0]: Horizontal spread (degrees, in fixed point)
//   args[1]: Vertical spread (degrees, in fixed point)
//   args[2]: Number of bullets to fire; if not set, defaults to 1
//   args[3]: Base damage of attack (e.g. for 5d3, customize the 5); if not set, defaults to 5
//   args[4]: Attack damage modulus (e.g. for 5d3, customize the 3); if not set, defaults to 3
//
void A_WeaponBulletAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int             numbullets;
    state_t         *state = psp->state;
    weapontype_t    readyweapon;

    if (!state)
        return;

    numbullets = state->args[2];
    readyweapon = player->readyweapon;

    if (!(weaponinfo[readyweapon].flags & WPF_SILENT))
        P_NoiseAlert(actor);

    P_BulletSlope(player->mo);

    for (int i = 0; i < numbullets; i++)
        P_LineAttack(player->mo, player->mo->angle + P_RandomHitscanAngle(state->args[0]),
            MISSILERANGE, bulletslope + P_RandomHitscanSlope(state->args[1]),
            (M_Random() % state->args[4] + 1) * state->args[3]);

    ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

    P_RumbleWeapon(readyweapon);
}

//
// A_WeaponMeleeAttack
// A parameterized player weapon melee attack.
//   args[0]: Base damage of attack (e.g. for 2d10, customize the 2); if not set, defaults to 2
//   args[1]: Attack damage modulus (e.g. for 2d10, customize the 10); if not set, defaults to 10
//   args[2]: Berserk damage multiplier (fixed point); if not set, defaults to 1.0 (no change).
//   args[3]: Sound to play if attack hits
//   args[4]: Range (fixed point); if not set, defaults to player mobj's melee range
//
void A_WeaponMeleeAttack(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    int             range;
    angle_t         angle;
    int             slope;
    int             damage;
    state_t         *state = psp->state;
    weapontype_t    readyweapon;

    if (!state)
        return;

    if (!(range = state->args[4]))
        range = player->mo->info->meleerange;

    damage = (M_Random() % state->args[1] + 1) * state->args[0];

    if (player->powers[pw_strength])
        damage = (damage * state->args[2]) >> FRACBITS;

    // slight randomization; weird vanillaism here. :P
    angle = player->mo->angle + (M_SubRandom() << 18);

    // make autoaim prefer enemies
    slope = P_AimLineAttack(player->mo, angle, range, MF_FRIEND);

    if (!linetarget)
        slope = P_AimLineAttack(player->mo, angle, range, 0);

    // attack, dammit!
    P_LineAttack(player->mo, angle, range, slope, damage);

    // missed? ah, welp.
    if (!linetarget)
        return;

    if (!(weaponinfo[(readyweapon = player->readyweapon)].flags & WPF_SILENT))
        P_NoiseAlert(actor);

    // un-missed!
    S_StartSound(player->mo, state->args[3]);

    // turn to face target
    player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);

    P_RumbleWeapon(readyweapon);
}

//
// A_WeaponSound
// Plays a sound. Usable from weapons, unlike A_PlaySound
//   args[0]: ID of sound to play
//   args[1]: If 1, play sound at full volume (may be useful in DM?)
//
void A_WeaponSound(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    state_t *state = psp->state;

    if (!state)
        return;

    S_StartSound((state->args[1] ? NULL : player->mo), state->args[0]);
}

//
// A_WeaponAlert
// Alerts monsters to the player's presence. Handy when combined with WPF_SILENT.
//
void A_WeaponAlert(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    P_NoiseAlert(player->mo);
}

//
// A_WeaponJump
// Jumps to the specified state, with variable random chance.
// Basically the same as A_RandomJump, but for weapons.
//   args[0]: State number
//   args[1]: Chance, out of 255, to make the jump
//
void A_WeaponJump(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    state_t *state = psp->state;

    if (!state)
        return;

    if (M_Random() < state->args[1])
        P_SetPlayerSpritePtr(psp, state->args[0]);
}

//
// A_ConsumeAmmo
// Subtracts ammo from the player's "inventory". 'Nuff said.
//   args[0]: Amount of ammo to consume. If zero, use the weapon's ammo-per-shot amount.
//
void A_ConsumeAmmo(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    state_t             *state = psp->state;
    const weapontype_t  readyweapon = player->readyweapon;
    const weaponinfo_t  readyweaponinfo = weaponinfo[readyweapon];
    const ammotype_t    type = readyweaponinfo.ammotype;
    int                 ammo;

    if (!state || type == am_noammo)
        return;

    ammo = (state->args[0] ? state->args[0] : readyweaponinfo.ammopershot);

    if (legacyofrust)
    {
        if (readyweapon == wp_incinerator)
        {
            player->shotsfired_incinerator += MIN(player->ammo[type], ammo);
            stat_shotsfired_incinerator = SafeAdd(stat_shotsfired_incinerator, 1);
        }
        else if (readyweapon == wp_calamityblade)
        {
            player->shotsfired_calamityblade += MIN(player->ammo[type], ammo);
            stat_shotsfired_calamityblade = SafeAdd(stat_shotsfired_calamityblade, 1);
        }
    }

    // subtract ammo, but don't let it get below zero
    if (!infiniteammo)
        player->ammo[type] = MAX(0, player->ammo[type] - ammo);
}

//
// A_CheckAmmo
// Jumps to a state if the player's ammo is lower than the specified amount.
//   args[0]: State to jump to
//   args[1]: Minimum required ammo to NOT jump. If zero, use the weapon's ammo-per-shot amount.
//
void A_CheckAmmo(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    state_t             *state = psp->state;
    const weaponinfo_t  readyweapon = weaponinfo[player->readyweapon];
    const ammotype_t    type = readyweapon.ammotype;

    if (!state || type == am_noammo)
        return;

    if (player->ammo[type] < (state->args[1] ? state->args[1] : readyweapon.ammopershot))
        P_SetPlayerSpritePtr(psp, state->args[0]);
}

//
// A_RefireTo
// Jumps to a state if the player is holding down the fire button
//   args[0]: State to jump to
//   args[1]: If nonzero, skip the ammo check
//
void A_RefireTo(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    state_t *state = psp->state;

    if (!state)
        return;

    if ((state->args[1] || P_CheckAmmo(player->readyweapon))
        && (player->cmd.buttons & BT_ATTACK)
        && player->pendingweapon == wp_nochange
        && player->health > 0)
        P_SetPlayerSpritePtr(psp, state->args[0]);
}

//
// A_GunFlashTo
// Sets the weapon flash layer to the specified state.
//   args[0]: State number
//   args[1]: If nonzero, don't change the player actor state
//
void A_GunFlashTo(mobj_t *actor, player_t *player, pspdef_t *psp)
{
    state_t *state = psp->state;

    if (!state)
        return;

    if (!state->args[1])
        P_SetMobjState(player->mo, S_PLAY_ATK2);

    P_SetPlayerSprite(ps_flash, state->args[0]);
}
