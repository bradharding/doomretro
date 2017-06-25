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

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_timer.h"
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
int maxhealth = MAXHEALTH * 2;
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
int bfgcells = BFGCELLS;
int species_infighting;

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[NUMAMMO] = { 200, 50, 300, 50 };
int clipammo[NUMAMMO] = { 10, 4, 20, 1 };

char *weapondescription[] =
{
    "fist",
    "pistol",
    "shotgun",
    "chaingun",
    "rocket launcher",
    "plasma rifle",
    "BFG-9000",
    "chainsaw",
    "super shotgun"
};

dboolean        con_obituaries = con_obituaries_default;
dboolean        r_mirroredweapons = r_mirroredweapons_default;

unsigned int    stat_barrelsexploded;
unsigned int    stat_damageinflicted;
unsigned int    stat_damagereceived;
unsigned int    stat_deaths;
unsigned int    stat_itemspickedup;
unsigned int    stat_itemspickedup_ammo_bullets;
unsigned int    stat_itemspickedup_ammo_cells;
unsigned int    stat_itemspickedup_ammo_rockets;
unsigned int    stat_itemspickedup_ammo_shells;
unsigned int    stat_itemspickedup_armor;
unsigned int    stat_itemspickedup_health;
unsigned int    stat_monsterskilled;
unsigned int    stat_monsterskilled_arachnotrons;
unsigned int    stat_monsterskilled_archviles;
unsigned int    stat_monsterskilled_baronsofhell;
unsigned int    stat_monsterskilled_cacodemons;
unsigned int    stat_monsterskilled_cyberdemons;
unsigned int    stat_monsterskilled_demons;
unsigned int    stat_monsterskilled_heavyweapondudes;
unsigned int    stat_monsterskilled_hellknights;
unsigned int    stat_monsterskilled_imps;
unsigned int    stat_monsterskilled_lostsouls;
unsigned int    stat_monsterskilled_mancubi;
unsigned int    stat_monsterskilled_painelementals;
unsigned int    stat_monsterskilled_revenants;
unsigned int    stat_monsterskilled_shotgunguys;
unsigned int    stat_monsterskilled_spectres;
unsigned int    stat_monsterskilled_spidermasterminds;
unsigned int    stat_monsterskilled_zombiemen;

extern int      idclevtics;
extern char     *playername;
extern dboolean r_althud;
extern dboolean r_hud;

static void P_AddAmmo(player_t *player, ammotype_t ammo, int num)
{
    switch (ammo)
    {
        case am_clip:
            player->itemspickedup_ammo_bullets += num;
            stat_itemspickedup_ammo_bullets = SafeAdd(stat_itemspickedup_ammo_bullets, num);
            break;

        case am_shell:
            player->itemspickedup_ammo_shells += num;
            stat_itemspickedup_ammo_shells = SafeAdd(stat_itemspickedup_ammo_shells, num);
            break;

        case am_cell:
            player->itemspickedup_ammo_cells += num;
            stat_itemspickedup_ammo_cells = SafeAdd(stat_itemspickedup_ammo_cells, num);
            break;

        case am_misl:
            player->itemspickedup_ammo_rockets += num;
            stat_itemspickedup_ammo_rockets = SafeAdd(stat_itemspickedup_ammo_rockets, num);
            break;

        default:
            break;
    }
}

//
// GET STUFF
//

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns the amount of ammo given to the player
//
int P_GiveAmmo(player_t *player, ammotype_t ammo, int num, dboolean stat)
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
    player->ammo[ammo] = MIN(oldammo + num, player->maxammo[ammo]);

    if (r_hud && !r_althud && num && ammo == weaponinfo[player->readyweapon].ammo)
        ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

    if (stat)
        P_AddAmmo(player, ammo, player->ammo[ammo] - oldammo);

    // If non-zero ammo, don't change up weapons, player was lower on purpose.
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
                if (player->weaponowned[wp_supershotgun] && player->preferredshotgun == wp_supershotgun
                    && player->ammo[am_shell] >= 2)
                    player->pendingweapon = wp_supershotgun;
                else if (player->weaponowned[wp_shotgun])
                    player->pendingweapon = wp_shotgun;
            }

            break;

        case am_cell:
            if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
                if (player->weaponowned[wp_plasma])
                    player->pendingweapon = wp_plasma;

            break;

        default:
            break;
    }

    return num;
}

//
// P_GiveBackpack
//
dboolean P_GiveBackpack(player_t *player, dboolean giveammo, dboolean stat)
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

            if (r_hud && !r_althud && (ammotype_t)i == weaponinfo[player->readyweapon].ammo)
                ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
        }

        if (giveammo)
            P_GiveAmmo(player, (ammotype_t)i, 1, stat);
    }

    return result;
}

//
// P_GiveFullAmmo
//
dboolean P_GiveFullAmmo(player_t *player, dboolean stat)
{
    int         i;
    dboolean    result = false;

    for (i = 0; i < NUMAMMO; i++)
        if (player->ammo[i] < player->maxammo[i])
        {
            if (stat)
                P_AddAmmo(player, i, player->maxammo[i] - player->ammo[i]);

            player->ammo[i] = player->maxammo[i];
            result = true;
        }

    if (result)
    {
        if (r_hud && !r_althud)
            ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

        return true;
    }

    return false;
}

//
// P_AddBonus
//
void P_AddBonus(player_t *player, int amount)
{
    player->bonuscount = MIN(player->bonuscount + amount, 3 * TICRATE);
}

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ORed in.
//
dboolean P_GiveWeapon(player_t *player, weapontype_t weapon, dboolean dropped, dboolean stat)
{
    dboolean    gaveammo = false;
    dboolean    gaveweapon = false;
    ammotype_t  ammotype = weaponinfo[weapon].ammo;

    if (ammotype != am_noammo)
        // give one clip with a dropped weapon, two clips with a found weapon
        gaveammo = P_GiveAmmo(player, ammotype, (dropped ? 1 : 2), stat);

    if (!player->weaponowned[weapon])
    {
        gaveweapon = true;
        player->weaponowned[weapon] = true;
        player->pendingweapon = weapon;
    }

    if (gaveammo && ammotype == weaponinfo[player->readyweapon].ammo)
    {
        if (r_hud && !r_althud)
            ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

        return true;
    }

    return (gaveweapon || gaveammo);
}

//
// P_GiveAllWeapons
//
dboolean P_GiveAllWeapons(player_t *player)
{
    dboolean    result = false;

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

    player->shotguns = (player->weaponowned[wp_shotgun] || player->weaponowned[wp_supershotgun]);

    return result;
}

static void P_AddHealth(player_t *player, int num)
{
    player->itemspickedup_health += num;
    stat_itemspickedup_health = SafeAdd(stat_itemspickedup_health, num);
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//
dboolean P_GiveBody(player_t *player, int num, dboolean stat)
{
    int oldhealth;

    if (player->health >= MAXHEALTH)
        return false;

    oldhealth = player->health;
    player->health = MIN(oldhealth + num, MAXHEALTH);
    player->mo->health = player->health;

    healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;

    if (stat)
        P_AddHealth(player, player->health - oldhealth);

    return true;
}

//
// P_GiveMegaHealth
//
void P_GiveMegaHealth(player_t *player, dboolean stat)
{
    if (!(player->cheats & CF_GODMODE))
    {
        if (player->health < mega_health)
        {
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;

            if (stat)
                P_AddHealth(player, mega_health - player->health);
        }

        player->health = player->mo->health = mega_health;
    }
}

static void P_AddArmor(player_t *player, int num)
{
    player->itemspickedup_armor += num;
    stat_itemspickedup_armor = SafeAdd(stat_itemspickedup_armor, num);
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
dboolean P_GiveArmor(player_t *player, armortype_t armortype, dboolean stat)
{
    int hits = armortype * 100;

    if (player->armorpoints >= hits)
        return false;   // don't pick up

    player->armortype = armortype;

    if (stat)
        P_AddArmor(player, hits - player->armorpoints);

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

//
// P_GiveAllCards
//
dboolean P_GiveAllCards(player_t *player)
{
    int         i;
    dboolean    skulliscard = true;
    dboolean    result = false;

    for (i = 0; i < numlines; i++)
        if (lines[i].special >= GenLockedBase && !((lines[i].special & LockedNKeys) >> LockedNKeysShift))
        {
            skulliscard = false;
            break;
        }

    for (i = NUMCARDS - 1; i >= 0; i--)
        if (player->cards[i] != CARDNOTINMAP && player->cards[i] == CARDNOTFOUNDYET)
        {
            if (skulliscard && ((i == it_blueskull && player->cards[it_bluecard] != CARDNOTINMAP)
                || (i == it_redskull && player->cards[it_redcard] != CARDNOTINMAP)
                || (i == it_yellowskull && player->cards[it_yellowcard] != CARDNOTINMAP)))
                continue;

            P_GiveCard(player, i);
            result = true;
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
        /* pw_strength        */ 1,
        /* pw_invisibility    */ INVISTICS,
        /* pw_ironfeet        */ IRONTICS,
        /* pw_allmap          */ STARTFLASHING + 1,
        /* pw_infrared        */ INFRATICS
   };

    if (player->powers[power] < 0)
        return false;

    switch (power)
    {
        case pw_strength:
            P_GiveBody(player, 100, true);
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
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher, dboolean message, dboolean stat)
{
    player_t    *player;
    fixed_t     delta = special->z - toucher->z;
    int         sound;
    int         weaponowned;
    int         ammo;
    static int  prevsound;
    static int  prevtic;

    if (freeze)
        return;

    if (delta > toucher->height || delta < -8 * FRACUNIT)
        return;         // out of reach

    // Dead thing touching.
    // Can happen with a sliding player corpse.
    if (toucher->health <= 0)
        return;

    sound = sfx_itemup;
    player = toucher->player;

    // Identify by sprite.
    switch (special->sprite)
    {
        // armor
        case SPR_ARM1:
            if (!P_GiveArmor(player, green_armor_class, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTARMOR, false);

            break;

        case SPR_ARM2:
            if (!P_GiveArmor(player, blue_armor_class, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTMEGA, false);

            break;

        // bonus items
        case SPR_BON1:
            if (!(player->cheats & CF_GODMODE))
            {
                player->health++;       // can go over 100%

                if (player->health > maxhealth)
                    player->health = maxhealth;
                else
                {
                    P_AddHealth(player, 1);
                    healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
                }

                player->mo->health = player->health;
            }

            if (message)
                HU_PlayerMessage(s_GOTHTHBONUS, false);

            break;

        case SPR_BON2:
            if (player->armorpoints < max_armor)
            {
                player->armorpoints++;
                P_AddArmor(player, 1);
                armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            }

            if (!player->armortype)
                player->armortype = GREENARMOR;

            if (message)
                HU_PlayerMessage(s_GOTARMBONUS, false);

            break;

        case SPR_SOUL:
            if (!(player->cheats & CF_GODMODE))
            {
                P_AddHealth(player, soul_health - player->health);
                player->health += soul_health;

                if (player->health > max_soul)
                    player->health = max_soul;

                player->mo->health = player->health;
                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            }

            if (message)
                HU_PlayerMessage(s_GOTSUPER, false);

            sound = sfx_getpow;
            break;

        case SPR_MEGA:
            P_GiveMegaHealth(player, stat);
            P_GiveArmor(player, blue_armor_class, stat);

            if (message)
                HU_PlayerMessage(s_GOTMSPHERE, false);

            sound = sfx_getpow;
            break;

        // cards
        case SPR_BKEY:
            if (player->cards[it_bluecard] <= 0)
            {
                P_GiveCard(player, it_bluecard);

                if (message)
                    HU_PlayerMessage(s_GOTBLUECARD, false);

                break;
            }

            return;

        case SPR_YKEY:
            if (player->cards[it_yellowcard] <= 0)
            {
                P_GiveCard(player, it_yellowcard);

                if (message)
                    HU_PlayerMessage(s_GOTYELWCARD, false);

                break;
            }

            return;

        case SPR_RKEY:
            if (player->cards[it_redcard] <= 0)
            {
                P_GiveCard(player, it_redcard);

                if (message)
                    HU_PlayerMessage(s_GOTREDCARD, false);

                break;
            }

            return;

        case SPR_BSKU:
            if (player->cards[it_blueskull] <= 0)
            {
                P_GiveCard(player, it_blueskull);

                if (message)
                    HU_PlayerMessage(s_GOTBLUESKUL, false);

                break;
            }

            return;

        case SPR_YSKU:
            if (player->cards[it_yellowskull] <= 0)
            {
                P_GiveCard(player, it_yellowskull);

                if (message)
                    HU_PlayerMessage(s_GOTYELWSKUL, false);

                break;
            }

            return;

        case SPR_RSKU:
            if (player->cards[it_redskull] <= 0)
            {
                P_GiveCard(player, it_redskull);

                if (message)
                    HU_PlayerMessage(s_GOTREDSKULL, false);

                break;
            }

            return;

        // medikits, heals
        case SPR_STIM:
            if (!P_GiveBody(player, 10, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTSTIM, false);

            break;

        case SPR_MEDI:
            if (!P_GiveBody(player, 25, stat))
                return;

            if (message)
                HU_PlayerMessage((player->health < 50 ? s_GOTMEDINEED : s_GOTMEDIKIT), false);

            break;

        // power ups
        case SPR_PINV:
            if (!P_GivePower(player, pw_invulnerability))
                return;

            if (message)
                HU_PlayerMessage(s_GOTINVUL, false);

            sound = sfx_getpow;
            break;

        case SPR_PSTR:
            if (!P_GivePower(player, pw_strength))
                return;

            if (message)
                HU_PlayerMessage(s_GOTBERSERK, false);

            if (player->readyweapon != wp_fist)
                player->pendingweapon = wp_fist;

            player->fistorchainsaw = wp_fist;
            sound = sfx_getpow;
            break;

        case SPR_PINS:
            if (!P_GivePower(player, pw_invisibility))
                return;

            if (message)
                HU_PlayerMessage(s_GOTINVIS, false);

            sound = sfx_getpow;
            break;

        case SPR_SUIT:
            if (!P_GivePower(player, pw_ironfeet))
                return;

            if (message)
                HU_PlayerMessage(s_GOTSUIT, false);

            sound = sfx_getpow;
            break;

        case SPR_PMAP:
            P_GivePower(player, pw_allmap);

            if (message)
                HU_PlayerMessage(s_GOTMAP, false);

            sound = sfx_getpow;
            break;

        case SPR_PVIS:
            if (!P_GivePower(player, pw_infrared))
                return;

            if (message)
                HU_PlayerMessage(s_GOTVISOR, false);

            sound = sfx_getpow;
            break;

        // ammo
        case SPR_CLIP:
            if (!(ammo = P_GiveAmmo(player, am_clip, !(special->flags & MF_DROPPED), stat)))
                return;

            if (message)
            {
                if (ammo == clipammo[am_clip] || deh_strlookup[p_GOTCLIP].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTCLIP, false);
                else
                    HU_PlayerMessage((ammo == clipammo[am_clip] / 2 ? s_GOTHALFCLIP : s_GOTCLIPX2), false);
            }

            break;

        case SPR_AMMO:
            if (!P_GiveAmmo(player, am_clip, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCLIPBOX, false);

            break;

        case SPR_ROCK:
            if (!(ammo = P_GiveAmmo(player, am_misl, 1, stat)))
                return;

            if (message)
            {
                if (ammo == clipammo[am_misl] || deh_strlookup[p_GOTROCKET].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTROCKET, false);
                else
                    HU_PlayerMessage(s_GOTROCKETX2, false);
            }

            break;

        case SPR_BROK:
            if (!P_GiveAmmo(player, am_misl, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTROCKBOX, false);

            break;

        case SPR_CELL:
            if (!(ammo = P_GiveAmmo(player, am_cell, 1, stat)))
                return;

            if (message)
            {
                if (ammo == clipammo[am_cell] || deh_strlookup[p_GOTCELL].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTCELL, false);
                else
                    HU_PlayerMessage(s_GOTCELLX2, false);
            }

            break;

        case SPR_CELP:
            if (!P_GiveAmmo(player, am_cell, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCELLBOX, false);

            break;

        case SPR_SHEL:
            if (!(ammo = P_GiveAmmo(player, am_shell, 1, stat)))
                return;

            if (message)
            {
                if (ammo == clipammo[am_shell] || deh_strlookup[p_GOTSHELLS].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTSHELLS, false);
                else
                    HU_PlayerMessage(s_GOTSHELLSX2, false);
            }

            break;

        case SPR_SBOX:
            if (!P_GiveAmmo(player, am_shell, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTSHELLBOX, false);

            break;

        case SPR_BPAK:
            if (!P_GiveBackpack(player, true, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTBACKPACK, false);

            break;

        // weapons
        case SPR_BFUG:
            if (!P_GiveWeapon(player, wp_bfg, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTBFG9000, false);

            sound = sfx_wpnup;
            break;

        case SPR_MGUN:
            if (!P_GiveWeapon(player, wp_chaingun, (special->flags & MF_DROPPED), stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCHAINGUN, false);

            sound = sfx_wpnup;
            break;

        case SPR_CSAW:
            if (!P_GiveWeapon(player, wp_chainsaw, false, stat))
                return;

            player->fistorchainsaw = wp_chainsaw;

            if (message)
                HU_PlayerMessage(s_GOTCHAINSAW, false);

            sound = sfx_wpnup;
            break;

        case SPR_LAUN:
            if (!P_GiveWeapon(player, wp_missile, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTLAUNCHER, false);

            sound = sfx_wpnup;
            break;

        case SPR_PLAS:
            if (!P_GiveWeapon(player, wp_plasma, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTPLASMA, false);

            sound = sfx_wpnup;
            break;

        case SPR_SHOT:
            weaponowned = player->weaponowned[wp_shotgun];

            if (!P_GiveWeapon(player, wp_shotgun, (special->flags & MF_DROPPED), stat))
                return;

            if (!weaponowned)
                player->preferredshotgun = wp_shotgun;

            player->shotguns = (player->weaponowned[wp_shotgun] || player->weaponowned[wp_supershotgun]);

            if (message)
                HU_PlayerMessage(s_GOTSHOTGUN, false);

            sound = sfx_wpnup;
            break;

        case SPR_SGN2:
            weaponowned = player->weaponowned[wp_supershotgun];

            if (!P_GiveWeapon(player, wp_supershotgun, (special->flags & MF_DROPPED), stat))
                return;

            if (!weaponowned)
                player->preferredshotgun = wp_supershotgun;

            player->shotguns = (player->weaponowned[wp_shotgun] || player->weaponowned[wp_supershotgun]);

            if (message)
                HU_PlayerMessage(s_GOTSHOTGUN2, false);

            sound = sfx_wpnup;
            break;

        default:
            return;
    }

    if ((special->flags & MF_COUNTITEM) && stat)
    {
        player->itemcount++;
        stat_itemspickedup = SafeAdd(stat_itemspickedup, 1);
    }

    P_RemoveMobj(special);
    P_AddBonus(player, BONUSADD);

    if (sound == prevsound && gametic == prevtic)
        return;

    prevsound = sound;
    prevtic = gametic;

    S_StartSound(player->mo, sound);
}

//
// P_UpdateKillStat
//
void P_UpdateKillStat(mobjtype_t type, int value)
{
    switch (type)
    {
        case MT_BABY:
            stat_monsterskilled_arachnotrons = SafeAdd(stat_monsterskilled_arachnotrons, value);
            break;

        case MT_VILE:
            stat_monsterskilled_archviles = SafeAdd(stat_monsterskilled_archviles, value);
            break;

        case MT_BRUISER:
            stat_monsterskilled_baronsofhell = SafeAdd(stat_monsterskilled_baronsofhell, value);
            break;

        case MT_HEAD:
            stat_monsterskilled_cacodemons = SafeAdd(stat_monsterskilled_cacodemons, value);
            break;

        case MT_CYBORG:
            stat_monsterskilled_cyberdemons = SafeAdd(stat_monsterskilled_cyberdemons, value);
            break;

        case MT_SERGEANT:
            stat_monsterskilled_demons = SafeAdd(stat_monsterskilled_demons, value);
            break;

        case MT_CHAINGUY:
            stat_monsterskilled_heavyweapondudes = SafeAdd(stat_monsterskilled_heavyweapondudes, value);
            break;

        case MT_KNIGHT:
            stat_monsterskilled_hellknights = SafeAdd(stat_monsterskilled_hellknights, value);
            break;

        case MT_TROOP:
            stat_monsterskilled_imps = SafeAdd(stat_monsterskilled_imps, value);
            break;

        case MT_SKULL:
            stat_monsterskilled_lostsouls = SafeAdd(stat_monsterskilled_lostsouls, value);
            break;

        case MT_FATSO:
            stat_monsterskilled_mancubi = SafeAdd(stat_monsterskilled_mancubi, value);
            break;

        case MT_PAIN:
            stat_monsterskilled_painelementals = SafeAdd(stat_monsterskilled_painelementals, value);
            break;

        case MT_UNDEAD:
            stat_monsterskilled_revenants = SafeAdd(stat_monsterskilled_revenants, value);
            break;

        case MT_SHOTGUY:
            stat_monsterskilled_shotgunguys = SafeAdd(stat_monsterskilled_shotgunguys, value);
            break;

        case MT_SHADOWS:
            stat_monsterskilled_spectres = SafeAdd(stat_monsterskilled_spectres, value);
            break;

        case MT_SPIDER:
            stat_monsterskilled_spidermasterminds = SafeAdd(stat_monsterskilled_spidermasterminds, value);
            break;

        case MT_POSSESSED:
            stat_monsterskilled_zombiemen = SafeAdd(stat_monsterskilled_zombiemen, value);
            break;

        default:
            break;
    }
}

//
// P_KillMobj
//
void P_KillMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source)
{
    dboolean    gibbed;
    mobjtype_t  item;
    mobjtype_t  type = target->type;
    mobjinfo_t  *info = &mobjinfo[type];
    mobj_t      *mo;
    int         gibhealth;
    player_t    *player = &players[0];

    target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY);

    if (type == MT_SKULL)
    {
        target->momx = 0;
        target->momy = 0;
        target->momz = 0;
    }
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
            static int prev;
            int        r = M_RandomInt(1, 10);

            if (r <= 5 + prev)
            {
                prev--;
                target->flags2 |= MF2_MIRRORED;
            }
            else
                prev++;
        }
    }

    if (target->flags & MF_COUNTKILL)
    {
        // count all monster deaths, even those caused by other monsters
        player->killcount++;
        stat_monsterskilled = SafeAdd(stat_monsterskilled, 1);

        if (!chex && !hacx)
        {
            player->mobjcount[type]++;
            P_UpdateKillStat(type, 1);
        }
    }
    else if (type == MT_BARREL && !chex && !hacx)
    {
        player->mobjcount[type]++;
        stat_barrelsexploded = SafeAdd(stat_barrelsexploded, 1);
    }

    if (type == MT_BARREL && source)
        P_SetTarget(&target->target, source);

    if (target->player)
    {
        // count environment kills against you
        target->flags &= ~MF_SOLID;
        target->player->playerstate = PST_DEAD;
        P_DropWeapon(target->player);

        if (automapactive)
            AM_Stop();          // don't die in auto map, switch view prior to dying

        player->deaths++;
        stat_deaths = SafeAdd(stat_deaths, 1);
    }
    else
        target->flags2 &= ~MF2_NOLIQUIDBOB;

    gibhealth = info->gibhealth;

    if ((gibbed = (gibhealth < 0 && target->health < gibhealth && info->xdeathstate)))
        P_SetMobjState(target, info->xdeathstate);
    else
        P_SetMobjState(target, info->deathstate);

    target->tics = MAX(1, target->tics - (M_Random() & 3));

    if (type == MT_BARREL || type == MT_PAIN || type == MT_SKULL)
        target->flags2 &= ~MF2_CASTSHADOW;

    if (chex)
        return;

    if (con_obituaries && source && source != target && !hacx)
    {
        if (inflicter && inflicter->type == MT_BARREL && type != MT_BARREL)
            C_Obituary("%s %s was %s by an exploding barrel.", (isvowel(info->name1[0]) ? "An" : "A"),
                info->name1, (gibbed ? "gibbed" : "killed"));
        else if (source->player)
            C_Obituary("%s %s %s%s with %s %s.", titlecase(playername), (type == MT_BARREL ? "exploded" :
                (gibbed ? "gibbed" : "killed")), (target->player ? "" : (isvowel(info->name1[0]) ? "an " :
                "a ")), (target->player ? (M_StringCompare(playername, playername_default) ? "yourself" :
                "themselves") : info->name1), (M_StringCompare(playername, playername_default) ? "your" :
                "their"), weapondescription[source->player->readyweapon]);
        else
            C_Obituary("%s %s %s %s%s.", (isvowel(source->info->name1[0]) ? "An" : "A"), source->info->name1,
                (type == MT_BARREL ? "exploded" : (gibbed ? "gibbed" : "killed")),  (target->player ? "" :
                (source->type == target->type ? "another " : (isvowel(info->name1[0]) ? "an " : "a "))),
                (target->player ? (M_StringCompare(playername, playername_default) ? playername :
                titlecase(playername)) : info->name1));
    }

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
    mo->momx = M_RandomInt(-255, 255) << 8;
    mo->momy = M_RandomInt(-255, 255) << 8;
    mo->momz = FRACUNIT * 5 + (M_Random() << 10);
    mo->angle = target->angle + ((M_Random() - M_Random()) << 20);
    mo->flags |= MF_DROPPED;    // special versions of items

    if (r_mirroredweapons && (rand() & 1))
        mo->flags2 |= MF2_MIRRORED;
}

dboolean P_CheckMeleeRange(mobj_t *actor);

//
// P_DamageMobj
// Damages both enemies and players
// "inflicter" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflicter are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
void P_DamageMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source, int damage, dboolean usearmor)
{
    player_t    *splayer = NULL;
    player_t    *tplayer;
    int         flags = target->flags;
    dboolean    corpse = flags & MF_CORPSE;
    int         type = target->type;
    mobjinfo_t  *info = &mobjinfo[type];

    if (!(flags & MF_SHOOTABLE) && (!corpse || !r_corpses_slide))
        return;

    if (type == MT_BARREL && corpse)
        return;

    if (flags & MF_SKULLFLY)
    {
        target->momx = 0;
        target->momy = 0;
        target->momz = 0;
    }

    if (source)
        splayer = source->player;

    tplayer = target->player;

    if (tplayer && gameskill == sk_baby)
        damage >>= (damage > 1);

    // Some close combat weapons should not
    // inflict thrust and push the victim out of reach,
    // thus kick away unless using the chainsaw.
    if (inflicter && (!tplayer || !inflicter->player) && !(flags & MF_NOCLIP)
        && (!source || !splayer || splayer->readyweapon != wp_chainsaw))
    {
        unsigned int    ang = R_PointToAngle2(inflicter->x, inflicter->y, target->x, target->y);
        int             mass = (corpse ? MAX(200, info->mass) : info->mass);
        fixed_t         thrust = damage * (FRACUNIT >> 3) * 100 / mass;

        // make fall forwards sometimes
        if (damage < 40 && damage > target->health  && target->z - inflicter->z > 64 * FRACUNIT
            && (M_Random() & 1))
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
                viewx = inflicter->x;
                viewy = inflicter->y;
                z = inflicter->z;
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

    if (corpse)
        return;

    // player specific
    if (splayer && type != MT_BARREL)
    {
        splayer->damageinflicted += damage;
        stat_damageinflicted = SafeAdd(stat_damageinflicted, damage);
    }

    if (tplayer)
    {
        int damagecount;

        if (freeze && (!inflicter || !inflicter->player))
            return;

        // end of game hell hack
        if (target->subsector->sector->special == DamageNegative10Or20PercentHealthAndEndLevel
            && damage >= target->health)
            damage = target->health - 1;

        // Below certain threshold,
        // ignore damage in GOD mode, or with INVUL power.
        if ((tplayer->cheats & CF_GODMODE) || idclevtics
            || (damage < 1000 && tplayer->powers[pw_invulnerability]))
            return;

        if (usearmor && tplayer->armortype)
        {
            int saved = damage / (tplayer->armortype == GREENARMOR ? 3 : 2);

            if (tplayer->armorpoints <= saved)
            {
                // armor is used up
                saved = tplayer->armorpoints;
                tplayer->armortype = NOARMOR;
            }

            tplayer->armorpoints -= saved;
            damage -= saved;
        }

        tplayer->health -= damage;
        target->health -= damage;

        if (tplayer->health <= 0 && (tplayer->cheats & CF_BUDDHA))
        {
            tplayer->health = 1;
            target->health = 1;
        }

        if (!(tplayer->cheats & CF_BUDDHA) || tplayer->health >= 1)
        {
            tplayer->damagereceived += damage;
            stat_damagereceived = SafeAdd(stat_damagereceived, damage);
        }

        tplayer->attacker = source;
        damagecount = tplayer->damagecount + damage;    // add damage after armor / invuln

        if (damage > 0 && damagecount < 8)
             damagecount = 8;

        damagecount = MIN(damagecount, 100);

        tplayer->damagecount = damagecount;

        if (gp_vibrate_damage && vibrate)
        {
            XInputVibration((30000 + (100 - MIN(tplayer->health, 100)) / 100 * 30000)
                * gp_vibrate_damage / 100);
            damagevibrationtics += BETWEEN(12, damage, 100);
        }

        if (tplayer->health <= 0)
        {
            P_KillMobj(target, inflicter, source);
            return;
        }
    }
    else
    {
        // do the damage
        target->health -= damage;

        if (target->health <= 0)
        {
            int gibhealth = info->gibhealth;

            if (type == MT_BARREL || type == MT_PAIN || type == MT_SKULL)
                target->colfunc = tlredcolfunc;
            else if (type == MT_BRUISER || type == MT_KNIGHT)
                target->colfunc = redtogreencolfunc;

            // [crispy] the lethal pellet of a point-blank SSG blast
            // gets an extra damage boost for the occasional gib chance
            if (splayer && splayer->readyweapon == wp_supershotgun && info->xdeathstate
                && P_CheckMeleeRange(target) && damage >= 10 && gibhealth < 0)
                target->health = gibhealth - 1;

            P_KillMobj(target, inflicter, source);
            return;
        }
    }

    if (M_Random() < info->painchance)
    {
        target->flags |= MF_JUSTHIT;                            // fight back!

        target->flags &= ~MF_SKULLFLY;

        P_SetMobjState(target, info->painstate);
    }

    target->reactiontime = 0;                                   // we're awake now...

    if ((!target->threshold || type == MT_VILE) && source && source != target && source->type != MT_VILE)
    {
        // if not intent on another player,
        // chase after this one
        if (!target->lastenemy || target->lastenemy->health <= 0 || !target->lastenemy->player)
            P_SetTarget(&target->lastenemy, target->target);    // remember last enemy - killough

        P_SetTarget(&target->target, source);                   // killough 11/98
        target->threshold = BASETHRESHOLD;

        if (target->state == &states[info->spawnstate] && info->seestate != S_NULL)
            P_SetMobjState(target, info->seestate);
    }
}
