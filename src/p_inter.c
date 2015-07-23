/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#include <ctype.h>
#include <stdlib.h>

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_tick.h"
#include "s_sound.h"

// Ty 03/07/98 - add deh externals
// Maximums and such were hardcoded values. Need to externalize those for
// dehacked support (and future flexibility). Most var names came from the key
// strings used in dehacked.
int initial_health = 100;
int initial_bullets = 50;
int maxhealth = 100;
int max_armor = 200;
int green_armor_class = 1;
int blue_armor_class = 2;
int max_soul = 200;
int soul_health = 100;
int mega_health = 200;
int god_health = 100;
int idfa_armor = 200;
int idfa_armor_class = 2;
int idkfa_armor = 200;
int idkfa_armor_class = 2;
int bfgcells = 40;
int species_infighting = 0;

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[NUMAMMO] = { 200, 50, 300, 50 };
int clipammo[NUMAMMO] = { 10, 4, 20, 1 };

dboolean r_mirroredweapons = r_mirroredweapons_default;

//
// GET STUFF
//

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns the amount of ammo given to the player
//
int P_GiveAmmo(player_t *player, ammotype_t ammo, int num)
{
    int oldammo;

    if (ammo == am_noammo)
        return 0;

    if (player->ammo[ammo] == player->maxammo[ammo])
        return 0;

    if (num)
        num *= clipammo[ammo];
    else
        num = clipammo[ammo] / 2;

    // give double ammo in trainer mode, you'll need in nightmare
    if (gameskill == sk_baby || gameskill == sk_nightmare)
        num <<= 1;

    oldammo = player->ammo[ammo];
    player->ammo[ammo] += num;

    if (player->ammo[ammo] > player->maxammo[ammo])
        player->ammo[ammo] = player->maxammo[ammo];

    if (num && ammo == weaponinfo[player->readyweapon].ammo)
        ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

    // If non zero ammo, don't change up weapons, player was lower on purpose.
    if (oldammo)
        return num;

    // We were down to zero, so select a new weapon.
    // Preferences are not user selectable.
    switch (ammo)
    {
        case am_clip:
            if (player->readyweapon == wp_fist)
            {
                if (player->weaponowned[wp_chaingun])
                    player->pendingweapon = wp_chaingun;
                else
                    player->pendingweapon = wp_pistol;
            }
            break;

        case am_shell:
            if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
            {
                if (player->weaponowned[wp_supershotgun]
                    && player->preferredshotgun == wp_supershotgun
                    && player->ammo[am_shell] >= 2)
                    player->pendingweapon = wp_supershotgun;
                else if (player->weaponowned[wp_shotgun])
                    player->pendingweapon = wp_shotgun;
            }
            break;

        case am_cell:
            if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
            {
                if (player->weaponowned[wp_plasma])
                    player->pendingweapon = wp_plasma;
            }
            break;

        default:
            break;
    }

    return num;
}

dboolean P_GiveBackpack(player_t *player, dboolean giveammo)
{
    int         i;
    dboolean    result = false;

    if (!player->backpack)
    {
        for (i = 0; i < NUMAMMO; i++)
            player->maxammo[i] *= 2;
        player->backpack = true;

    }
    for (i = 0; i < NUMAMMO; i++)
    {
        if (player->ammo[i] < player->maxammo[i])
        {
            result = true;
            if ((ammotype_t)i == weaponinfo[player->readyweapon].ammo)
                ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
        }
        if (giveammo)
            P_GiveAmmo(player, (ammotype_t)i, 1);
    }
    return result;
}

dboolean P_GiveFullAmmo(player_t *player)
{
    int         i;
    dboolean    result = false;

    for (i = 0; i < NUMAMMO; i++)
        if (player->ammo[i] < player->maxammo[i])
        {
            player->ammo[i] = player->maxammo[i];
            result = true;
        }

    if (result)
    {
        ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
        return true;
    }
    else
        return false;
}

void P_AddBonus(player_t *player, int amount)
{
    if (!consoleactive)
        player->bonuscount = MIN(player->bonuscount + amount, 3 * TICRATE);
}

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//
dboolean P_GiveWeapon(player_t *player, weapontype_t weapon, dboolean dropped)
{
    dboolean    gaveammo = false;
    dboolean    gaveweapon = false;
    ammotype_t  ammotype = weaponinfo[weapon].ammo;

    if (ammotype != am_noammo)
        // give one clip with a dropped weapon, two clips with a found weapon
        gaveammo = P_GiveAmmo(player, ammotype, (dropped ? 1 : 2));

    if (!player->weaponowned[weapon])
    {
        gaveweapon = true;
        player->weaponowned[weapon] = true;
        player->pendingweapon = weapon;
    }

    if (gaveammo && ammotype == weaponinfo[player->readyweapon].ammo)
    {
        ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
        return true;
    }
    else
        return (gaveweapon || gaveammo);
}

dboolean P_GiveAllWeapons(player_t *player)
{
    dboolean result = false;

    if (!oldweaponsowned[wp_shotgun])
    {
        result = true;
        player->weaponowned[wp_shotgun] = true;
        oldweaponsowned[wp_shotgun] = true;
    }
    if (!oldweaponsowned[wp_chaingun])
    {
        result = true;
        player->weaponowned[wp_chaingun] = true;
        oldweaponsowned[wp_chaingun] = true;
    }
    if (!oldweaponsowned[wp_missile])
    {
        result = true;
        player->weaponowned[wp_missile] = true;
        oldweaponsowned[wp_missile] = true;
    }
    if (gamemode != shareware)
    {
        if (!oldweaponsowned[wp_plasma])
        {
            result = true;
            player->weaponowned[wp_plasma] = true;
            oldweaponsowned[wp_plasma] = true;
        }
        if (!oldweaponsowned[wp_bfg])
        {
            result = true;
            player->weaponowned[wp_bfg] = true;
            oldweaponsowned[wp_bfg] = true;
        }
    }
    if (!oldweaponsowned[wp_chainsaw])
    {
        result = true;
        player->weaponowned[wp_chainsaw] = true;
        oldweaponsowned[wp_chainsaw] = true;
        player->fistorchainsaw = wp_chainsaw;

        if (player->readyweapon == wp_fist)
            player->pendingweapon = wp_chainsaw;
    }
    if (gamemode == commercial && !oldweaponsowned[wp_supershotgun])
    {
        player->preferredshotgun = wp_supershotgun;
        result = true;
        player->weaponowned[wp_supershotgun] = true;
        oldweaponsowned[wp_supershotgun] = true;

        if (player->readyweapon == wp_shotgun)
            player->pendingweapon = wp_supershotgun;
    }
    player->shotguns = (player->weaponowned[wp_shotgun]
        || player->weaponowned[wp_supershotgun]);

    return result;
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//
dboolean P_GiveBody(player_t *player, int num)
{
    if (player->health >= maxhealth)
        return false;

    player->health = MIN(player->health + num, maxhealth);
    player->mo->health = player->health;

    healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;

    return true;
}

void P_GiveMegaHealth(player_t *player)
{
    if (!(player->cheats & CF_GODMODE))
    {
        if (player->health < mega_health)
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
        player->health = player->mo->health = mega_health;
    }
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
dboolean P_GiveArmor(player_t *player, int armortype)
{
    int hits = armortype * 100;

    if (player->armorpoints >= hits)
        return false;   // don't pick up

    player->armortype = armortype;
    player->armorpoints = hits;
    armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;

    return true;
}

int cardsfound;

//
// P_InitCards
//
void P_InitCards(player_t *player)
{
    int i;

    for (i = 0; i < NUMCARDS; i++)
        player->cards[i] = CARDNOTINMAP;

    cardsfound = 0;

    for (i = 0; i < numsectors; i++)
    {
        mobj_t  *thing = sectors[i].thinglist;

        while (thing)
        {
            switch (thing->sprite)
            {
                case SPR_BKEY:
                    player->cards[it_bluecard] = CARDNOTFOUNDYET;
                    break;
                case SPR_RKEY:
                    player->cards[it_redcard] = CARDNOTFOUNDYET;
                    break;
                case SPR_YKEY:
                    player->cards[it_yellowcard] = CARDNOTFOUNDYET;
                    break;
                case SPR_BSKU:
                    player->cards[it_blueskull] = CARDNOTFOUNDYET;
                    break;
                case SPR_RSKU:
                    player->cards[it_redskull] = CARDNOTFOUNDYET;
                    break;
                case SPR_YSKU:
                    player->cards[it_yellowskull] = CARDNOTFOUNDYET;
                    break;
                default:
                    break;
            }
            thing = thing->snext;
        }
    }

    for (i = 0; i < numlines; i++)
    {
        line_t  *line = &lines[i];

        switch (line->special)
        {
            case DR_Door_Blue_OpenWaitClose:
            case D1_Door_Blue_OpenStay:
            case SR_Door_Blue_OpenStay_Fast:
            case S1_Door_Blue_OpenStay_Fast:
                if (player->cards[it_blueskull] == CARDNOTINMAP)
                    player->cards[it_bluecard] = CARDNOTFOUNDYET;
                break;
            case DR_Door_Red_OpenWaitClose:
            case D1_Door_Red_OpenStay:
            case SR_Door_Red_OpenStay_Fast:
            case S1_Door_Red_OpenStay_Fast:
                if (player->cards[it_redskull] == CARDNOTINMAP)
                    player->cards[it_redcard] = CARDNOTFOUNDYET;
                break;
            case DR_Door_Yellow_OpenWaitClose:
            case D1_Door_Yellow_OpenStay:
            case SR_Door_Yellow_OpenStay_Fast:
            case S1_Door_Yellow_OpenStay_Fast:
                if (player->cards[it_yellowskull] == CARDNOTINMAP)
                    player->cards[it_yellowcard] = CARDNOTFOUNDYET;
                break;
        }
    }
}

//
// P_GiveCard
//
void P_GiveCard(player_t *player, card_t card)
{
    player->cards[card] = ++cardsfound;
    if (card == player->neededcard)
        player->neededcard = player->neededcardflash = 0;
}

dboolean P_GiveAllCards(player_t *player)
{
    int         i;
    dboolean    result = false;

    cardsfound = 0;
    for (i = NUMCARDS - 1; i >= 0; i--)
        if (player->cards[i] != CARDNOTINMAP)
        {
            if (player->cards[i] == CARDNOTFOUNDYET)
                result = true;
            P_GiveCard(player, i);
        }

    return result;
}

//
// P_GivePower
//
dboolean P_GivePower(player_t *player, int power)
{
    static const int tics[NUMPOWERS] =
    {
        /* pw_invulnerability */ INVULNTICS,
        /* pw_strength */        1,
        /* pw_invisibility */    INVISTICS,
        /* pw_ironfeet */        IRONTICS,
        /* pw_allmap */          STARTFLASHING + 1,
        /* pw_infrared */        INFRATICS
   };

    if (player->powers[power] < 0)
        return false;

    switch (power)
    {
        case pw_strength:
            P_GiveBody(player, 100);
            break;
        case pw_invisibility:
            player->mo->flags |= MF_FUZZ;
            break;
    }

    player->powers[power] = tics[power];
    return true;
}

//
// P_TouchSpecialThing
//
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher)
{
    player_t    *player;
    fixed_t     delta = special->z - toucher->z;
    int         sound;
    int         weaponowned;
    int         ammo;
    static int  prevsound = 0;
    static int  prevtic = 0;

    if (delta > toucher->height || delta < -8 * FRACUNIT)
        return;         // out of reach

    sound = sfx_itemup;
    player = toucher->player;

    // Dead thing touching.
    // Can happen with a sliding player corpse.
    if (toucher->health <= 0)
        return;

    // Identify by sprite.
    switch (special->sprite)
    {
        // armor
        case SPR_ARM1:
            if (!P_GiveArmor(player, green_armor_class))
                return;
            HU_PlayerMessage(s_GOTARMOR, true);
            break;

        case SPR_ARM2:
            if (!P_GiveArmor(player, blue_armor_class))
                return;
            HU_PlayerMessage(s_GOTMEGA, true);
            break;

        // bonus items
        case SPR_BON1:
            if (!(player->cheats & CF_GODMODE))
            {
                player->health++;       // can go over 100%
                if (player->health > maxhealth * 2)
                    player->health = maxhealth * 2;
                else
                    healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
                player->mo->health = player->health;
            }
            HU_PlayerMessage(s_GOTHTHBONUS, true);
            break;

        case SPR_BON2:
            player->armorpoints++;      // can go over 100%
            if (player->armorpoints > max_armor)
                player->armorpoints = max_armor;
            else
                armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            if (!player->armortype)
                player->armortype = 1;
            HU_PlayerMessage(s_GOTARMBONUS, true);
            break;

        case SPR_SOUL:
            if (!(player->cheats & CF_GODMODE))
            {
                player->health += soul_health;
                if (player->health > max_soul)
                    player->health = max_soul;
                player->mo->health = player->health;
                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            }
            HU_PlayerMessage(s_GOTSUPER, true);
            sound = sfx_getpow;
            break;

        case SPR_MEGA:
            P_GiveMegaHealth(player);
            P_GiveArmor(player, 2);
            HU_PlayerMessage(s_GOTMSPHERE, true);
            sound = sfx_getpow;
            break;

        // cards
        // leave cards for everyone
        case SPR_BKEY:
            if (player->cards[it_bluecard] <= 0)
            {
                P_GiveCard(player, it_bluecard);
                HU_PlayerMessage(s_GOTBLUECARD, true);
                break;
            }
            return;

        case SPR_YKEY:
            if (player->cards[it_yellowcard] <= 0)
            {
                P_GiveCard(player, it_yellowcard);
                HU_PlayerMessage(s_GOTYELWCARD, true);
                break;
            }
            return;

        case SPR_RKEY:
            if (player->cards[it_redcard] <= 0)
            {
                P_GiveCard(player, it_redcard);
                HU_PlayerMessage(s_GOTREDCARD, true);
                break;
            }
            return;

        case SPR_BSKU:
            if (player->cards[it_blueskull] <= 0)
            {
                P_GiveCard(player, it_blueskull);
                HU_PlayerMessage(s_GOTBLUESKUL, true);
                break;
            }
            return;

        case SPR_YSKU:
            if (player->cards[it_yellowskull] <= 0)
            {
                P_GiveCard(player, it_yellowskull);
                HU_PlayerMessage(s_GOTYELWSKUL, true);
                break;
            }
            return;

        case SPR_RSKU:
            if (player->cards[it_redskull] <= 0)
            {
                P_GiveCard(player, it_redskull);
                HU_PlayerMessage(s_GOTREDSKULL, true);
                break;
            }
            return;

        // medikits, heals
        case SPR_STIM:
            if (!P_GiveBody(player, 10))
                return;
            HU_PlayerMessage(s_GOTSTIM, true);
            break;

        case SPR_MEDI:
            if (!P_GiveBody(player, 25))
                return;
            HU_PlayerMessage((player->health < 50 ? s_GOTMEDINEED : s_GOTMEDIKIT), true);
            break;

        // power ups
        case SPR_PINV:
            if (!P_GivePower(player, pw_invulnerability))
                return;
            HU_PlayerMessage(s_GOTINVUL, true);
            sound = sfx_getpow;
            break;

        case SPR_PSTR:
            if (!P_GivePower(player, pw_strength))
                return;
            HU_PlayerMessage(s_GOTBERSERK, true);
            if (player->readyweapon != wp_fist)
                player->pendingweapon = wp_fist;
            player->fistorchainsaw = wp_fist;
            sound = sfx_getpow;
            break;

        case SPR_PINS:
            if (!P_GivePower(player, pw_invisibility))
                return;
            HU_PlayerMessage(s_GOTINVIS, true);
            sound = sfx_getpow;
            break;

        case SPR_SUIT:
            if (!P_GivePower(player, pw_ironfeet))
                return;
            HU_PlayerMessage(s_GOTSUIT, true);
            sound = sfx_getpow;
            break;

        case SPR_PMAP:
            P_GivePower(player, pw_allmap);
            HU_PlayerMessage(s_GOTMAP, true);
            sound = sfx_getpow;
            break;

        case SPR_PVIS:
            if (!P_GivePower(player, pw_infrared))
                return;
            HU_PlayerMessage(s_GOTVISOR, true);
            sound = sfx_getpow;
            break;

        // ammo
        case SPR_CLIP:
            if (!(ammo = P_GiveAmmo(player, am_clip, (special->flags & MF_DROPPED) == 0)))
                return;
            if (ammo == clipammo[am_clip] || (deh_strlookup[p_GOTCLIP].assigned && dehacked))
                HU_PlayerMessage(s_GOTCLIP, true);
            else
                HU_PlayerMessage((ammo == clipammo[am_clip] / 2 ? s_GOTHALFCLIP : s_GOTCLIPX2), true);
            break;

        case SPR_AMMO:
            if (!P_GiveAmmo(player, am_clip, 5))
                return;
            HU_PlayerMessage(s_GOTCLIPBOX, true);
            break;

        case SPR_ROCK:
            if (!(ammo = P_GiveAmmo(player, am_misl, 1)))
                return;
            if (ammo == clipammo[am_misl] || (deh_strlookup[p_GOTROCKET].assigned && dehacked))
                HU_PlayerMessage(s_GOTROCKET, true);
            else
                HU_PlayerMessage(s_GOTROCKETX2, true);
            break;

        case SPR_BROK:
            if (!P_GiveAmmo(player, am_misl, 5))
                return;
            HU_PlayerMessage(s_GOTROCKBOX, true);
            break;

        case SPR_CELL:
            if (!(ammo = P_GiveAmmo(player, am_cell, 1)))
                return;
            if (ammo == clipammo[am_cell] || (deh_strlookup[p_GOTCELL].assigned && dehacked))
                HU_PlayerMessage(s_GOTCELL, true);
            else
                HU_PlayerMessage(s_GOTCELLX2, true);
            break;

        case SPR_CELP:
            if (!P_GiveAmmo(player, am_cell, 5))
                return;
            HU_PlayerMessage(s_GOTCELLBOX, true);
            break;

        case SPR_SHEL:
            if (!(ammo = P_GiveAmmo(player, am_shell, 1)))
                return;
            if (!message_dontfuckwithme)
                if (ammo == clipammo[am_shell] || (deh_strlookup[p_GOTSHELLS].assigned && dehacked))
                    HU_PlayerMessage(s_GOTSHELLS, true);
                else
                    HU_PlayerMessage(s_GOTSHELLSX2, true);
            break;

        case SPR_SBOX:
            if (!P_GiveAmmo(player, am_shell, 5))
                return;
            HU_PlayerMessage(s_GOTSHELLBOX, true);
            break;

        case SPR_BPAK:
            if (!P_GiveBackpack(player, true))
                return;
            HU_PlayerMessage(s_GOTBACKPACK, true);
            break;

        // weapons
        case SPR_BFUG:
            if (!P_GiveWeapon(player, wp_bfg, false))
                return;
            HU_PlayerMessage(s_GOTBFG9000, true);
            sound = sfx_wpnup;
            break;

        case SPR_MGUN:
            if (!P_GiveWeapon(player, wp_chaingun, (special->flags & MF_DROPPED) != 0))
                return;
            HU_PlayerMessage(s_GOTCHAINGUN, true);
            sound = sfx_wpnup;
            break;

        case SPR_CSAW:
            if (!P_GiveWeapon(player, wp_chainsaw, false))
                return;
            player->fistorchainsaw = wp_chainsaw;
            HU_PlayerMessage(s_GOTCHAINSAW, true);
            sound = sfx_wpnup;
            break;

        case SPR_LAUN:
            if (!P_GiveWeapon(player, wp_missile, false))
                return;
            HU_PlayerMessage(s_GOTLAUNCHER, true);
            sound = sfx_wpnup;
            break;

        case SPR_PLAS:
            if (!P_GiveWeapon(player, wp_plasma, false))
                return;
            HU_PlayerMessage(s_GOTPLASMA, true);
            sound = sfx_wpnup;
            break;

        case SPR_SHOT:
            weaponowned = player->weaponowned[wp_shotgun];
            if (!P_GiveWeapon(player, wp_shotgun, (special->flags & MF_DROPPED) != 0))
                return;
            if (!weaponowned)
                player->preferredshotgun = wp_shotgun;
            player->shotguns = (player->weaponowned[wp_shotgun]
                                || player->weaponowned[wp_supershotgun]);
            HU_PlayerMessage(s_GOTSHOTGUN, true);
            sound = sfx_wpnup;
            break;

        case SPR_SGN2:
            weaponowned = player->weaponowned[wp_supershotgun];
            if (!P_GiveWeapon(player, wp_supershotgun, (special->flags & MF_DROPPED) != 0))
                return;
            if (!weaponowned)
                player->preferredshotgun = wp_supershotgun;
            player->shotguns = (player->weaponowned[wp_shotgun]
                                || player->weaponowned[wp_supershotgun]);
            HU_PlayerMessage(s_GOTSHOTGUN2, true);
            sound = sfx_wpnup;
            break;

        default:
            return;
    }

    if (special->flags & MF_COUNTITEM)
        player->itemcount++;
    if (special->shadow)
        P_RemoveMobjShadow(special);
    P_RemoveMobj(special);
    P_AddBonus(player, BONUSADD);

    if (sound == prevsound && gametic == prevtic)
        return;
    prevsound = sound;
    prevtic = gametic;

    S_StartSound(player->mo, sound);
}

//
// P_KillMobj
//
void P_KillMobj(mobj_t *source, mobj_t *target)
{
    mobjtype_t  item;
    mobjtype_t  type = target->type;
    mobj_t      *mo;

    target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY);

    if (type == MT_SKULL)
        target->momx = target->momy = target->momz = 0;
    else
        target->flags &= ~MF_NOGRAVITY;

    target->flags |= (MF_CORPSE | MF_DROPOFF);
    target->height >>= 2;

    // killough 8/29/98: remove from threaded list
    P_UpdateThinker(&target->thinker);

    if (type != MT_BARREL)
    {
        if (!(target->flags & MF_FUZZ))
            target->bloodsplats = CORPSEBLOODSPLATS;

        if (r_corpses_mirrored && type != MT_CHAINGUY && type != MT_CYBORG)
        {
            static int prev = 0;
            int        r = M_RandomInt(1, 10);

            if (r <= 5 + prev)
            {
                prev--;
                target->flags2 |= MF2_MIRRORED;
                if (target->shadow)
                    target->shadow->flags2 |= MF2_MIRRORED;
            }
            else
                prev++;
        }
    }

    if (source && source->player)
    {
        // count for intermission
        if (target->flags & MF_COUNTKILL)
            source->player->killcount++;
    }
    else if (target->flags & MF_COUNTKILL)
        // count all monster deaths, even those caused by other monsters
        players[0].killcount++;

    if (type == MT_BARREL && source && source->player)
        target->target = source;

    if (target->player)
    {
        // count environment kills against you
        target->flags &= ~MF_SOLID;
        target->player->playerstate = PST_DEAD;
        P_DropWeapon(target->player);

        if (target->player == &players[0] && automapactive)
            AM_Stop();          // don't die in auto map, switch view prior to dying
    }
    else
        target->flags2 &= ~MF2_NOLIQUIDBOB;

    if (target->health < -target->info->spawnhealth && target->info->xdeathstate)
        P_SetMobjState(target, target->info->xdeathstate);
    else
        P_SetMobjState(target, target->info->deathstate);

    target->tics = MAX(1, target->tics - (P_Random() & 3));

    if ((type == MT_BARREL || type == MT_PAIN || type == MT_SKULL) && target->shadow)
        P_RemoveMobjShadow(target);

    if (chex)
        return;

    // Drop stuff.
    // This determines the kind of object spawned during the death frame of a thing.
    switch (type)
    {
        case MT_WOLFSS:
        case MT_POSSESSED:
            item = MT_CLIP;
            break;

        case MT_SHOTGUY:
            item = MT_SHOTGUN;
            break;

        case MT_CHAINGUY:
            item = MT_CHAINGUN;
            break;

        default:
            return;
    }

    mo = P_SpawnMobj(target->x, target->y, target->floorz + FRACUNIT * target->height / 2, item);
    mo->momx += P_Random() << 8;
    mo->momy += P_Random() << 8;
    mo->momz = FRACUNIT * 5 + (P_Random() << 10);
    mo->angle = target->angle + ((P_Random() - P_Random()) << 20);
    mo->flags |= MF_DROPPED;    // special versions of items
    if (r_mirroredweapons && (rand() & 1))
    {
        mo->flags2 |= MF2_MIRRORED;
        if (mo->shadow)
            mo->shadow->flags2 |= MF2_MIRRORED;
    }
}

dboolean P_CheckMeleeRange(mobj_t *actor);

//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
void P_DamageMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage)
{
    player_t    *splayer = NULL;
    player_t    *tplayer;
    int         flags = target->flags;
    int         type = target->type;

    if (!(flags & MF_SHOOTABLE) && (!(flags & MF_CORPSE) || !r_corpses_slide))
        return;

    if (type == MT_BARREL && (flags & MF_CORPSE))
        return;

    if (flags & MF_SKULLFLY)
        target->momx = target->momy = target->momz = 0;

    if (source)
        splayer = source->player;

    tplayer = target->player;

    if (tplayer && gameskill == sk_baby)
        damage >>= (damage > 1);

    // Some close combat weapons should not
    // inflict thrust and push the victim out of reach,
    // thus kick away unless using the chainsaw.
    if (inflictor && !(flags & MF_NOCLIP) && (!splayer || splayer->readyweapon != wp_chainsaw))
    {
        unsigned int    ang = R_PointToAngle2(inflictor->x, inflictor->y, target->x, target->y);
        fixed_t         thrust = damage * (FRACUNIT >> 3) * 100 / target->info->mass;

        // make fall forwards sometimes
        if (damage < 40 && damage > target->health
            && target->z - inflictor->z > 64 * FRACUNIT && (P_Random() & 1))
        {
            ang += ANG180;
            thrust *= 4;
        }

        ang >>= ANGLETOFINESHIFT;
        target->momx += FixedMul(thrust, finecosine[ang]);
        target->momy += FixedMul(thrust, finesine[ang]);

        // killough 11/98: thrust objects hanging off ledges
        if ((target->flags2 & MF2_FALLING) && target->gear >= MAXGEAR)
            target->gear = 0;

        if (source)
        {
            int dist;
            int z;

            if (source == target)
            {
                viewx = inflictor->x;
                viewy = inflictor->y;
                z = inflictor->z;
            }
            else
            {
                viewx = source->x;
                viewy = source->y;
                z = source->z;
            }
            dist = R_PointToDist(target->x, target->y);

            if (target->flags2 & MF2_FEETARECLIPPED)
                z += FOOTCLIPSIZE;

            viewx = 0;
            viewy = z;
            ang = R_PointToAngle(dist, target->z);

            ang >>= ANGLETOFINESHIFT;
            target->momz += FixedMul(thrust, finesine[ang]);
        }
    }

    // player specific
    if (tplayer)
    {
        int     damagecount;

        if (tplayer->health <= 0)
            return;

        // end of game hell hack
        if (target->subsector->sector->special == DamageNegative10Or20PercentHealthAndEndLevel
            && damage >= target->health)
            damage = target->health - 1;

        // Below certain threshold,
        // ignore damage in GOD mode, or with INVUL power.
        if ((tplayer->cheats & CF_GODMODE) || (damage < 1000 && tplayer->powers[pw_invulnerability]))
            return;

        if (tplayer->armortype)
        {
            int saved = damage / (tplayer->armortype == 1 ? 3 : 2);

            if (tplayer->armorpoints <= saved)
            {
                // armor is used up
                saved = tplayer->armorpoints;
                tplayer->armortype = 0;
            }
            tplayer->armorpoints -= saved;
            damage -= saved;
        }
        tplayer->health = MAX(0, tplayer->health - damage);     // mirror mobj health here for Dave

        tplayer->attacker = source;
        damagecount = tplayer->damagecount + damage;            // add damage after armor / invuln

        if (damage > 0 && damagecount < 2)                      // damagecount gets decremented before
             damagecount = 2;                                   // being used so needs to be at least 2
        damagecount = MIN(damagecount, 100);                    // teleport stomp does 10k points...

        tplayer->damagecount = damagecount;

        if (gp_vibrate && vibrate && tplayer == &players[0])
        {
            XInputVibration(30000 + (100 - MIN(tplayer->health, 100)) / 100 * 30000);
            damagevibrationtics += BETWEEN(12, damage, 100);
        }
    }

    // do the damage
    if (!(flags & MF_CORPSE))
    {
        target->health -= damage;
        if (target->health <= 0)
        {
            if (type == MT_BARREL || type == MT_PAIN || type == MT_SKULL)
                target->colfunc = tlredcolfunc;
            else if (type == MT_BRUISER || type == MT_KNIGHT)
                target->colfunc = redtogreencolfunc;

            // [crispy] the lethal pellet of a point-blank SSG blast
            // gets an extra damage boost for the occasional gib chance
            if (splayer && splayer->readyweapon == wp_supershotgun && target->info->xdeathstate
                && P_CheckMeleeRange(target) && damage >= 10)
                target->health -= target->info->spawnhealth;

            P_KillMobj(source, target);
            return;
        }

        if (P_Random() < target->info->painchance && !(flags & MF_SKULLFLY))
        {
            target->flags |= MF_JUSTHIT;        // fight back!

            P_SetMobjState(target, target->info->painstate);
        }

        target->reactiontime = 0;               // we're awake now...

        if ((!target->threshold || type == MT_VILE)
            && source && source != target && source->type != MT_VILE)
        {
            // if not intent on another player,
            // chase after this one
            if (!target->lastenemy || target->lastenemy->health <= 0 || !target->lastenemy->player)
                target->lastenemy = target->target;     // remember last enemy - killough

            target->target = source;
            target->threshold = BASETHRESHOLD;
            if (target->state == &states[target->info->spawnstate]
                && target->info->seestate != S_NULL)
                P_SetMobjState(target, target->info->seestate);
        }
    }
}
