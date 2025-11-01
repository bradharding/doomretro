/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_controller.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "w_wad.h"

#define MASSACRETHRUST  (40 * (FRACUNIT >> 5))

// Ty 03/07/98 - add deh externals
// Maximums and such were hardcoded values. Need to externalize those for
// dehacked support (and future flexibility). Most var names came from the key
// strings used in dehacked.
int         initial_health = 100;
int         initial_bullets = 50;
int         maxhealth = MAXHEALTH * 2;
int         max_armor = 200;
int         green_armor_class = armortype_green;
int         blue_armor_class = armortype_blue;
int         max_soul = 200;
int         soul_health = 100;
int         mega_health = 200;
int         god_health = 100;
int         idfa_armor = 200;
int         idfa_armor_class = armortype_blue;
int         idkfa_armor = 200;
int         idkfa_armor_class = armortype_blue;
int         bfgcells = BFGCELLS;
bool        species_infighting = false;

// a weapon is found with two clip loads,
// a big item has five clip loads
int         maxammo[] =  { 200, 50, 300, 50 };
int         clipammo[] = {  10,  4,  20,  1 };

int         cardsfound;
mobjtype_t  prevtouchtype = MT_NULL;

void P_UpdateAmmoStat(const ammotype_t ammotype, const int num)
{
    switch (ammotype)
    {
        case am_clip:
            viewplayer->itemspickedup_ammo_bullets += num;
            stat_itemspickedup_ammo[am_clip] = SafeAdd(stat_itemspickedup_ammo[am_clip], num);
            break;

        case am_shell:
            viewplayer->itemspickedup_ammo_shells += num;
            stat_itemspickedup_ammo[am_shell] = SafeAdd(stat_itemspickedup_ammo[am_shell], num);
            break;

        case am_cell:
            if (legacyofrust)
            {
                viewplayer->itemspickedup_ammo_fuel += num;
                stat_itemspickedup_ammo_fuel = SafeAdd(stat_itemspickedup_ammo_fuel, num);
            }
            else
            {
                viewplayer->itemspickedup_ammo_cells += num;
                stat_itemspickedup_ammo[am_cell] = SafeAdd(stat_itemspickedup_ammo[am_cell], num);
            }

            break;

        case am_misl:
            viewplayer->itemspickedup_ammo_rockets += num;
            stat_itemspickedup_ammo[am_misl] = SafeAdd(stat_itemspickedup_ammo[am_misl], num);
            break;

        default:
            break;
    }
}

//
// P_TakeAmmo
//
static bool P_TakeAmmo(const ammotype_t ammotype, int num)
{
    weapontype_t    readyweapon;

    if (ammotype == am_noammo)
        return false;

    if (num)
        num *= clipammo[ammotype];
    else
        num = clipammo[ammotype] / 2;

    if (gameskill == sk_baby || gameskill == sk_nightmare)
        num <<= 1;

    if (viewplayer->ammo[ammotype] < num)
        return false;

    viewplayer->ammo[ammotype] -= num;
    readyweapon = viewplayer->readyweapon;

    P_AnimateAmmo(num, ammotype);

    if (ammotype == weaponinfo[readyweapon].ammotype)
        ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

    P_CheckAmmo(readyweapon);

    if (viewplayer->pendingweapon != readyweapon)
        C_HideConsole();

    return true;
}

static bool P_TakeWeapon(const weapontype_t weapon)
{
    weapontype_t    readyweapon;

    if (!viewplayer->weaponowned[weapon])
        return false;

    viewplayer->weaponowned[weapon] = false;
    oldweaponsowned[weapon] = false;
    readyweapon = viewplayer->readyweapon;
    P_CheckAmmo(readyweapon);

    if (viewplayer->pendingweapon != readyweapon)
        C_HideConsole();

    return true;
}

bool P_TakeBackpack(void)
{
    if (!viewplayer->backpack)
        return false;

    viewplayer->backpack = false;

    for (ammotype_t i = 0; i < NUMAMMO; i++)
    {
        viewplayer->maxammo[i] /= 2;
        P_AnimateMaxAmmo(viewplayer->maxammo[i], i);

        if (viewplayer->ammo[i] > viewplayer->maxammo[i])
        {
            P_AnimateAmmo(viewplayer->ammo[i] - viewplayer->maxammo[i], i);
            viewplayer->ammo[i] = viewplayer->maxammo[i];
        }
    }

    return true;
}

static void P_AutoSwitchWeapon(weapontype_t weapon)
{
    if (autoswitch)
        viewplayer->pendingweapon = weapon;
}

//
// GET STUFF
//

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0 = 1/2 clip).
// Returns the amount of ammo given to the player
//
static int P_GiveAmmo(const ammotype_t ammotype, int num, const bool stat)
{
    int                 oldammo;
    const weapontype_t  readyweapon = viewplayer->readyweapon;

    if (ammotype == am_noammo)
        return 0;

    if (viewplayer->ammo[ammotype] == viewplayer->maxammo[ammotype])
        return 0;

    if (num)
        num *= clipammo[ammotype];
    else
        num = clipammo[ammotype] / 2;

    // give double ammo in trainer mode, you'll need in nightmare
    if (gameskill == sk_baby || gameskill == sk_nightmare)
        num *= 2;

    oldammo = viewplayer->ammo[ammotype];
    viewplayer->ammo[ammotype] = MIN(oldammo + num, viewplayer->maxammo[ammotype]);

    if (num)
    {
        P_AnimateAmmo(oldammo - viewplayer->ammo[ammotype], ammotype);

        if (ammotype == weaponinfo[readyweapon].ammotype
            || (viewplayer->pendingweapon != wp_nochange && !viewplayer->ammo[viewplayer->pendingweapon]))
            ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
    }

    if (stat)
        P_UpdateAmmoStat(ammotype, viewplayer->ammo[ammotype] - oldammo);

    // MBF21: take into account new weapon autoswitch flags
    if ((weaponinfo[readyweapon].flags & WPF_AUTOSWITCHFROM)
        && weaponinfo[readyweapon].ammotype != ammotype)
        for (int i = NUMWEAPONS - 1; i > readyweapon; i--)
            if (viewplayer->weaponowned[i]
                && !(weaponinfo[i].flags & WPF_NOAUTOSWITCHTO)
                && weaponinfo[i].ammotype == ammotype
                && weaponinfo[i].ammopershot > oldammo
                && (weaponinfo[i].ammopershot <= viewplayer->ammo[ammotype] || infiniteammo))
            {
                if (i == wp_supershotgun && viewplayer->weaponowned[wp_shotgun] && viewplayer->preferredshotgun == wp_shotgun)
                    P_AutoSwitchWeapon(wp_shotgun);
                else
                    P_AutoSwitchWeapon(i);

                break;
            }

    return num;
}

//
// P_GiveBackpack
//
bool P_GiveBackpack(const bool giveammo, const bool stat)
{
    bool    result = false;

    if (!viewplayer->backpack)
    {
        viewplayer->backpack = true;

        for (ammotype_t i = 0; i < NUMAMMO; i++)
        {
            P_AnimateMaxAmmo(viewplayer->maxammo[i], i);
            viewplayer->maxammo[i] *= 2;
        }
    }

    for (ammotype_t i = 0; i < NUMAMMO; i++)
    {
        if (viewplayer->ammo[i] < viewplayer->maxammo[i])
            result = true;

        if (giveammo)
            P_GiveAmmo(i, 1, stat);
    }

    return result;
}

//
// P_GiveFullAmmo
//
bool P_GiveFullAmmo(void)
{
    bool    result = false;

    for (ammotype_t i = 0; i < NUMAMMO; i++)
        if (viewplayer->ammo[i] < viewplayer->maxammo[i])
        {
            P_AnimateAmmo(viewplayer->ammo[i] - viewplayer->maxammo[i], i);
            viewplayer->ammo[i] = viewplayer->maxammo[i];
            result = true;
        }

    if (result)
    {
        ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
        return true;
    }

    return false;
}

//
// P_AddBonus
//
void P_AddBonus(void)
{
    viewplayer->bonuscount = MIN(viewplayer->bonuscount + BONUSADD, TICRATE);
}

//
// P_GiveWeapon
//
static bool P_GiveWeapon(const weapontype_t weapon, const bool dropped, const bool stat)
{
    bool                gaveammo = false;
    bool                gaveweapon = false;
    const ammotype_t    ammotype = weaponinfo[weapon].ammotype;

    if (ammotype != am_noammo)
        // give one clip with a dropped weapon, two clips with a found weapon
        gaveammo = P_GiveAmmo(ammotype, (dropped ? 1 : 2), stat);

    if (!viewplayer->weaponowned[weapon])
    {
        gaveweapon = true;
        viewplayer->weaponowned[weapon] = true;
        P_AutoSwitchWeapon(weapon);
    }

    return (gaveweapon || gaveammo);
}

//
// P_GiveAllWeapons
//
bool P_GiveAllWeapons(void)
{
    bool    result = false;

    if (!viewplayer->weaponowned[wp_chainsaw])
    {
        viewplayer->weaponowned[wp_chainsaw] = true;
        viewplayer->fistorchainsaw = wp_chainsaw;

        if (viewplayer->readyweapon == wp_fist)
            viewplayer->pendingweapon = wp_chainsaw;

        result = true;
    }

    if (!viewplayer->weaponowned[wp_pistol])
    {
        viewplayer->weaponowned[wp_pistol] = true;
        result = true;
    }

    if (!viewplayer->weaponowned[wp_shotgun])
    {
        viewplayer->weaponowned[wp_shotgun] = true;
        result = true;
    }

    if (gamemode == commercial && !viewplayer->weaponowned[wp_supershotgun])
    {
        viewplayer->weaponowned[wp_supershotgun] = true;
        viewplayer->preferredshotgun = wp_supershotgun;

        if (viewplayer->readyweapon == wp_shotgun)
            viewplayer->pendingweapon = wp_supershotgun;

        result = true;
    }

    if (!viewplayer->weaponowned[wp_chaingun])
    {
        viewplayer->weaponowned[wp_chaingun] = true;
        result = true;
    }

    if (!viewplayer->weaponowned[wp_missile])
    {
        viewplayer->weaponowned[wp_missile] = true;
        result = true;
    }

    if (gamemode != shareware)
    {
        if (!viewplayer->weaponowned[wp_plasma])
        {
            viewplayer->weaponowned[wp_plasma] = true;
            result = true;
        }

        if (!viewplayer->weaponowned[wp_bfg])
        {
            viewplayer->weaponowned[wp_bfg] = true;
            result = true;
        }
    }

    return result;
}

void P_UpdateHealthStat(const int num)
{
    viewplayer->itemspickedup_health += num;
    stat_itemspickedup_health = SafeAdd(stat_itemspickedup_health, num);
}

//
// P_GiveHealth
// Returns false if the health isn't needed at all
//
bool P_GiveHealth(const int num, const int max, const bool stat)
{
    const int   health = viewplayer->health;

    if (health >= max)
        return false;

    viewplayer->health = MIN(health + num, max);
    viewplayer->mo->health = viewplayer->health;
    healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
    P_AnimateHealth(health - viewplayer->health);

    if (stat)
        P_UpdateHealthStat(viewplayer->health - health);

    return true;
}

//
// P_GiveMegaHealth
//
bool P_GiveMegaHealth(const bool stat)
{
    bool    result = false;

    if (!(viewplayer->cheats & CF_GODMODE))
    {
        if (viewplayer->health < mega_health)
        {
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            P_AnimateHealth(viewplayer->health - mega_health);

            if (stat)
                P_UpdateHealthStat(mega_health - viewplayer->health);

            result = true;
        }

        viewplayer->health = mega_health;
        viewplayer->mo->health = mega_health;
    }

    return result;
}

void P_UpdateArmorStat(const int num)
{
    viewplayer->itemspickedup_armor += num;
    stat_itemspickedup_armor = SafeAdd(stat_itemspickedup_armor, num);
}

//
// P_GiveArmor
// Returns false if the armor is worse than the current armor.
//
bool P_GiveArmor(const armortype_t armortype, const bool stat)
{
    const int   hits = armortype * 100;

    if (viewplayer->armor >= hits)
        return false;   // don't pick up

    viewplayer->armortype = armortype;
    P_AnimateArmor(viewplayer->armor - hits);

    if (stat)
        P_UpdateArmorStat(hits - viewplayer->armor);

    viewplayer->armor = hits;
    armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;

    return true;
}

void P_LookForCards(void)
{
    const int   cardsprites[] = { SPR_BKEY, SPR_YKEY, SPR_RKEY, SPR_BSKU, SPR_YSKU, SPR_RSKU };

    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        const mobj_t    *mo = (mobj_t *)th;

        for (int i = 0; i < NUMCARDS; i++)
            if (mo->sprite == cardsprites[i])
            {
                if (!viewplayer->cards[i])
                    viewplayer->cards[i] = CARDNOTFOUNDYET;

                break;
            }
    }
}

//
// P_InitCards
//
void P_InitCards(void)
{
    for (int i = 0; i < NUMCARDS; i++)
        viewplayer->cards[i] = CARDNOTINMAP;

    cardsfound = 0;

    P_LookForCards();

    for (int i = 0; i < numlines; i++)
    {
        line_t  *line = lines + i;

        switch (line->special)
        {
            case DR_Door_Blue_OpenWaitClose:
            case D1_Door_Blue_OpenStay:
            case SR_Door_Blue_OpenStay_Fast:
            case S1_Door_Blue_OpenStay_Fast:
                if (viewplayer->cards[it_bluecard] == CARDNOTINMAP && viewplayer->cards[it_blueskull] == CARDNOTINMAP)
                {
                    char    *temp = commify(i);

                    C_Warning(2, "Linedef %s has special %i (\"%s\") but there are no " BOLD("bluekeycard") " or "
                        BOLD("blueskullkey") " things in map.", temp, line->special, linespecials[line->special]);
                    free(temp);
                }

                break;

            case DR_Door_Red_OpenWaitClose:
            case D1_Door_Red_OpenStay:
            case SR_Door_Red_OpenStay_Fast:
            case S1_Door_Red_OpenStay_Fast:
                if (viewplayer->cards[it_redcard] == CARDNOTINMAP && viewplayer->cards[it_redskull] == CARDNOTINMAP)
                {
                    char    *temp = commify(i);

                    C_Warning(2, "Linedef %s has special %i (\"%s\") but there are no " BOLD("redkeycard") " or "
                        BOLD("redskullkey") " things in map.", temp, line->special, linespecials[line->special]);
                    free(temp);
                }

                break;

            case DR_Door_Yellow_OpenWaitClose:
            case D1_Door_Yellow_OpenStay:
            case SR_Door_Yellow_OpenStay_Fast:
            case S1_Door_Yellow_OpenStay_Fast:
                if (viewplayer->cards[it_yellowcard] == CARDNOTINMAP && viewplayer->cards[it_yellowskull] == CARDNOTINMAP)
                {
                    char    *temp = commify(i);

                    C_Warning(2, "Linedef %s has special %i (\"%s\") but there are no " BOLD("yellowkeycard") " or "
                        BOLD("yellowskullkey") " things in map.", temp, line->special, linespecials[line->special]);
                    free(temp);
                }

                break;
        }
    }
}

//
// P_GiveCard
//
static void P_GiveCard(const card_t card)
{
    if (viewplayer->cards[card] <= 0)
    {
        viewplayer->cards[card] = ++cardsfound;
        viewplayer->itemspickedup_keys++;
        stat_itemspickedup_keys = SafeAdd(stat_itemspickedup_keys, 1);

        if (card == viewplayer->neededcard)
        {
            viewplayer->neededcard = 0;
            viewplayer->neededcardflash = 0;
        }
    }
}

//
// P_GiveAllKeyCards
//
bool P_GiveAllKeyCards(void)
{
    bool    result = (viewplayer->cards[it_redcard] <= 0
        || viewplayer->cards[it_yellowcard] <= 0
        || viewplayer->cards[it_bluecard] <= 0);

    P_GiveCard(it_redcard);
    P_GiveCard(it_yellowcard);
    P_GiveCard(it_bluecard);

    return result;
}

//
// P_GiveAllSkullKeys
//
bool P_GiveAllSkullKeys(void)
{
    bool    result = (viewplayer->cards[it_redskull] <= 0
        || viewplayer->cards[it_yellowskull] <= 0
        || viewplayer->cards[it_blueskull] <= 0);

    P_GiveCard(it_redskull);
    P_GiveCard(it_yellowskull);
    P_GiveCard(it_blueskull);

    return result;
}

//
// P_GiveAllCards
//
bool P_GiveAllCards(void)
{
    const bool  result1 = P_GiveAllKeyCards();
    const bool  result2 = P_GiveAllSkullKeys();

    return (result1 || result2);
}

//
// P_GiveAllCardsInMap
//
bool P_GiveAllCardsInMap(void)
{
    bool    skulliscard = true;
    bool    result = false;

    for (int i = 0; i < numlines; i++)
        if (lines[i].special >= GenLockedBase && !((lines[i].special & LockedNKeys) >> LockedNKeysShift))
        {
            skulliscard = false;
            break;
        }

    for (int i = NUMCARDS - 1; i >= 0; i--)
        if (viewplayer->cards[i] == CARDNOTFOUNDYET)
        {
            if (skulliscard && ((i == it_blueskull && viewplayer->cards[it_bluecard] != CARDNOTINMAP)
                || (i == it_redskull && viewplayer->cards[it_redcard] != CARDNOTINMAP)
                || (i == it_yellowskull && viewplayer->cards[it_yellowcard] != CARDNOTINMAP)))
                continue;

            P_GiveCard(i);
            result = true;
        }

    return result;
}

//
// P_GivePower
//
bool P_GivePower(const int power, const bool stat)
{
    const int tics[] =
    {
        /* pw_none            */ 0,
        /* pw_invulnerability */ INVULNTICS,
        /* pw_strength        */ 1,
        /* pw_invisibility    */ INVISTICS,
        /* pw_ironfeet        */ IRONTICS,
        /* pw_allmap          */ STARTFLASHING + 1,
        /* pw_infrared        */ INFRATICS
    };

    bool    given;

    if (viewplayer->powers[power] < 0)
        return false;

    switch (power)
    {
        case pw_invulnerability:
            viewplayer->fixedcolormap = INVERSECOLORMAP;
            break;

        case pw_strength:
            P_GiveHealth(100, MAXHEALTH, true);
            break;

        case pw_invisibility:
            viewplayer->mo->flags |= MF_FUZZ;
            break;

        case pw_allmap:
            break;

        case pw_infrared:
            viewplayer->fixedcolormap = 1;
            break;
    }

    if ((given = (viewplayer->powers[power] <= 0)) && stat)
    {
        viewplayer->itemspickedup_powerups++;
        stat_itemspickedup_powerups = SafeAdd(stat_itemspickedup_powerups, 1);
    }

    viewplayer->powers[power] = tics[power];
    return given;
}

//
// P_TouchSpecialThing
//
bool P_TouchSpecialThing(mobj_t *special, const mobj_t *toucher, const bool message, const bool stat)
{
    fixed_t     delta;
    int         sound = sfx_itemup;
    static int  prevsound;
    static int  prevtic;
    static int  prevx, prevy;
    bool        duplicate;

    if (freeze)
        return false;

    // Dead thing touching.
    // Can happen with a sliding player corpse.
    if (toucher->health <= 0)
        return false;

    // [BH] Use floorz instead of z to calculate special's distance from toucher,
    // if lowered while in liquid sector.
    if ((delta = ((special->flags2 & MF2_FEETARECLIPPED) ? special->floorz : special->z) - toucher->z) > toucher->height
        || delta < -8 * FRACUNIT)
        return false;   // out of reach

    duplicate = (special->type == prevtouchtype && special->x == prevx && special->y == prevy);

    // Identify by sprite.
    switch (special->sprite)
    {
        // green armor
        case SPR_ARM1:
            if (!P_GiveArmor(green_armor_class, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTARMOR, true, false);

            break;

        // blue armor
        case SPR_ARM2:
            if (!P_GiveArmor(blue_armor_class, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTMEGA, true, false);

            break;

        // bonus health
        case SPR_BON1:
            if (viewplayer->health < maxhealth && !(viewplayer->cheats & CF_GODMODE))
            {
                viewplayer->mo->health = ++viewplayer->health;
                P_UpdateHealthStat(1);
                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            }

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTHTHBONUS, true, false);

            break;

        // bonus armor
        case SPR_BON2:
            if (viewplayer->armor < max_armor)
            {
                viewplayer->armor++;
                P_UpdateArmorStat(1);
                armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;

                if (!viewplayer->armortype)
                    viewplayer->armortype = green_armor_class;
            }

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTARMBONUS, true, false);

            break;

        // soulsphere
        case SPR_SOUL:
            if (!(viewplayer->cheats & CF_GODMODE))
            {
                viewplayer->health = MIN(viewplayer->health + soul_health, max_soul);
                viewplayer->mo->health = viewplayer->health;
                P_AnimateHealth(viewplayer->mo->health - viewplayer->health);
                P_UpdateHealthStat(viewplayer->health - viewplayer->mo->health);
                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            }

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTSUPER, true, false);

            sound = sfx_getpow;
            break;

        // mega health
        case SPR_MEGA:
            P_GiveMegaHealth(stat);
            P_GiveArmor(blue_armor_class, stat);
            viewplayer->armortype = blue_armor_class;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTMSPHERE, true, false);

            sound = sfx_getpow;
            break;

        // blue keycard
        case SPR_BKEY:
            P_GiveCard(it_bluecard);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTBLUECARD, true, false);

            break;

        // yellow keycard
        case SPR_YKEY:
            P_GiveCard(it_yellowcard);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTYELWCARD, true, false);

            break;

        // red keycard
        case SPR_RKEY:
            P_GiveCard(it_redcard);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTREDCARD, true, false);

            break;

        // blue skull key
        case SPR_BSKU:
            P_GiveCard(it_blueskull);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTBLUESKUL, true, false);

            break;

        // yellow skull key
        case SPR_YSKU:
            P_GiveCard(it_yellowskull);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTYELWSKUL, true, false);

            break;

        // red skull key
        case SPR_RSKU:
            P_GiveCard(it_redskull);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTREDSKULL, true, false);

            break;

        // stimpack
        case SPR_STIM:
            if (!P_GiveHealth(STIMPACKHEALTH, MAXHEALTH, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTSTIM, true, false);

            break;

        // medikit
        case SPR_MEDI:
            if (!P_GiveHealth(MEDIKITHEALTH, MAXHEALTH, stat))
                return false;

            if (message && !duplicate)
            {
                if (viewplayer->health < MEDIKITHEALTH * 2 && !(viewplayer->cheats & CF_BUDDHA))
                {
                    static char buffer[1024];

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_GOTMEDINEED, "You", "you");
                    else
                        M_snprintf(buffer, sizeof(buffer), s_GOTMEDINEED, playername, pronoun(personal));

                    HU_PlayerMessage(buffer, true, false);
                }
                else
                    HU_PlayerMessage(s_GOTMEDIKIT, true, false);
            }

            break;

        // invulnerability power-up
        case SPR_PINV:
            P_GivePower(pw_invulnerability, stat);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTINVUL, true, false);

            sound = sfx_getpow;
            break;

        // berserk power-up
        case SPR_PSTR:
        {
            const int   strength = viewplayer->powers[pw_strength];

            P_GivePower(pw_strength, stat);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTBERSERK, true, false);

            if (viewplayer->readyweapon != wp_fist)
            {
                viewplayer->pendingweapon = wp_fist;
                viewplayer->fistorchainsaw = wp_fist;
            }

            if (!strength)
                sound = sfx_getpow;

            break;
        }

        // partial invisibility power-up
        case SPR_PINS:
            P_GivePower(pw_invisibility, stat);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTINVIS, true, false);

            sound = sfx_getpow;
            break;

        // radiation shielding suit power-up
        case SPR_SUIT:
            P_GivePower(pw_ironfeet, stat);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTSUIT, true, false);

            sound = sfx_getpow;
            break;

        // computer area map power-up
        case SPR_PMAP:
            P_GivePower(pw_allmap, stat);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTMAP, true, false);

            sound = sfx_getpow;
            break;

        // light amplification visor power-up
        case SPR_PVIS:
            P_GivePower(pw_infrared, stat);

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTVISOR, true, false);

            sound = sfx_getpow;
            break;

        // clip
        case SPR_CLIP:
            if (!P_GiveAmmo(am_clip, !(special->flags & MF_DROPPED), stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTCLIP, true, false);

            break;

        // box of bullets
        case SPR_AMMO:
            if (!P_GiveAmmo(am_clip, 5, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTCLIPBOX, true, false);

            break;

        // rocket
        case SPR_ROCK:
        {
            const int   ammogiven = P_GiveAmmo(am_misl, 1, stat);

            if (!ammogiven)
                return false;

            if (message && !duplicate)
            {
                if (ammogiven == clipammo[am_misl] || deh_strlookup[p_GOTROCKET].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTROCKET, true, false);
                else
                    HU_PlayerMessage(s_GOTROCKETX2, true, false);
            }

            break;
        }

        // box of rockets
        case SPR_BROK:
            if (!P_GiveAmmo(am_misl, 5, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTROCKBOX, true, false);

            break;

        // cell
        case SPR_CELL:
        {
            const int   ammogiven = P_GiveAmmo(am_cell, 1, stat);

            if (!ammogiven)
                return false;

            if (message && !duplicate)
            {
                if (ammogiven == clipammo[am_cell] || deh_strlookup[p_GOTCELL].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTCELL, true, false);
                else
                    HU_PlayerMessage(s_GOTCELLX2, true, false);
            }

            break;
        }

        // cell pack
        case SPR_CELP:
            if (!P_GiveAmmo(am_cell, 5, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTCELLBOX, true, false);

            break;

        // shells
        case SPR_SHEL:
        {
            const int   ammogiven = P_GiveAmmo(am_shell, 1, stat);

            if (!ammogiven)
                return false;

            if (message && !duplicate)
            {
                if (ammogiven == clipammo[am_shell] || deh_strlookup[p_GOTSHELLS].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTSHELLS, true, false);
                else
                    HU_PlayerMessage(s_GOTSHELLSX2, true, false);
            }

            break;
        }

        // box of shells
        case SPR_SBOX:
            if (!P_GiveAmmo(am_shell, 5, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTSHELLBOX, true, false);

            break;

        // backpack
        case SPR_BPAK:
            if (!P_GiveBackpack(true, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTBACKPACK, true, false);

            break;

        // BFG-9000
        case SPR_BFUG:
            if (!P_GiveWeapon(wp_bfg, false, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTBFG9000, true, false);

            sound = sfx_wpnup;
            break;

        // chaingun
        case SPR_MGUN:
            if (!P_GiveWeapon(wp_chaingun, (special->flags & MF_DROPPED), stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTCHAINGUN, true, false);

            sound = sfx_wpnup;
            break;

        // chainsaw
        case SPR_CSAW:
            if (!P_GiveWeapon(wp_chainsaw, false, stat))
                return false;

            viewplayer->fistorchainsaw = wp_chainsaw;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTCHAINSAW, true, false);

            sound = sfx_wpnup;
            break;

        // rocket launcher
        case SPR_LAUN:
            if (!P_GiveWeapon(wp_missile, false, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTLAUNCHER, true, false);

            sound = sfx_wpnup;
            break;

        // plasma rifle
        case SPR_PLAS:
            if (!P_GiveWeapon(wp_plasma, false, stat))
                return false;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTPLASMA, true, false);

            sound = sfx_wpnup;
            break;

        // shotgun
        case SPR_SHOT:
        {
            bool    owned = viewplayer->weaponowned[wp_shotgun];

            if (!P_GiveWeapon(wp_shotgun, (special->flags & MF_DROPPED), stat))
                return false;

            if (!owned)
                viewplayer->preferredshotgun = wp_shotgun;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTSHOTGUN, true, false);

            sound = sfx_wpnup;
            break;
        }

        // super shotgun
        case SPR_SGN2:
        {
            bool    owned = viewplayer->weaponowned[wp_supershotgun];

            if (!P_GiveWeapon(wp_supershotgun, false, stat))
                return false;

            if (!owned)
                viewplayer->preferredshotgun = wp_supershotgun;

            if (message && !duplicate)
                HU_PlayerMessage(s_GOTSHOTGUN2, true, false);

            sound = sfx_wpnup;
            break;
        }

        default:
            break;
    }

    if ((special->flags & MF_COUNTITEM) && stat)
    {
        viewplayer->itemcount++;
        stat_itemspickedup = SafeAdd(stat_itemspickedup, 1);
    }

    if (sound != prevsound || gametime != prevtic)
    {
        prevsound = sound;
        S_StartSound(viewplayer->mo, sound);
    }

    P_RemoveMobj(special);

    if (!duplicate && special->type != MT_MISC13 && special->type != MT_MISC14)
        P_AddBonus();

    prevtouchtype = special->type;
    prevx = special->x;
    prevy = special->y;
    prevtic = gametime;

    return true;
}

//
// P_TakeSpecialThing
//
bool P_TakeSpecialThing(const mobjtype_t type)
{
    switch (type)
    {
        // green armor
        case MT_MISC0:
            if (viewplayer->armortype != green_armor_class
                || viewplayer->armor < green_armor_class * 100)
                return false;

            viewplayer->armor -= green_armor_class * 100;
            P_AnimateArmor(green_armor_class * 100);
            armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            return true;

        // blue armor
        case MT_MISC1:
            if (viewplayer->armortype != blue_armor_class
                || viewplayer->armor < blue_armor_class * 100)
                return false;

            viewplayer->armor -= blue_armor_class * 100;
            P_AnimateArmor(blue_armor_class * 100);
            armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            return true;

        // bonus health
        case MT_MISC2:
            if ((viewplayer->cheats & CF_GODMODE)
                || viewplayer->powers[pw_invulnerability]
                || ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health == 1)
                || viewplayer->health <= 0)
                return false;

            viewplayer->health--;
            viewplayer->mo->health--;
            viewplayer->damagecount = 1;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            S_StartSound(NULL, sfx_plpain);
            return true;

        // bonus armor
        case MT_MISC3:
            if (!viewplayer->armor)
                return false;

            viewplayer->armor--;
            armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            return true;

        // soulsphere
        case MT_MISC12:
            if ((viewplayer->cheats & CF_GODMODE)
                || viewplayer->powers[pw_invulnerability]
                || ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health <= soul_health)
                || viewplayer->health < soul_health)
                return false;

            viewplayer->health -= soul_health;
            viewplayer->mo->health -= soul_health;
            viewplayer->damagecount = soul_health;
            P_AnimateHealth(soul_health);
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            S_StartSound(NULL, sfx_plpain);
            return true;

        // mega health
        case MT_MEGA:
            if ((viewplayer->cheats & CF_GODMODE)
                || viewplayer->powers[pw_invulnerability]
                || ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health <= mega_health)
                || viewplayer->health < mega_health)
                return false;

            viewplayer->health -= mega_health;
            viewplayer->mo->health -= mega_health;
            viewplayer->damagecount = mega_health;
            P_AnimateHealth(mega_health);
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            S_StartSound(NULL, sfx_plpain);
            return true;

        // blue keycard
        case MT_MISC4:
            if (viewplayer->cards[it_bluecard] <= 0)
                return false;

            viewplayer->cards[it_bluecard] = 0;
            P_LookForCards();
            cardsfound--;
            return true;

        // yellow keycard
        case MT_MISC6:
            if (viewplayer->cards[it_yellowcard] <= 0)
                return false;

            viewplayer->cards[it_yellowcard] = 0;
            P_LookForCards();
            cardsfound--;
            return true;

        // red keycard
        case MT_MISC5:
            if (viewplayer->cards[it_redcard] <= 0)
                return false;

            viewplayer->cards[it_redcard] = 0;
            P_LookForCards();
            cardsfound--;
            return true;

        // blue skull key
        case MT_MISC9:
            if (viewplayer->cards[it_blueskull] <= 0)
                return false;

            viewplayer->cards[it_blueskull] = 0;
            P_LookForCards();
            cardsfound--;
            return true;

        // yellow skull key
        case MT_MISC7:
            if (viewplayer->cards[it_yellowskull] <= 0)
                return false;

            viewplayer->cards[it_yellowskull] = 0;
            P_LookForCards();
            cardsfound--;
            return true;

        // red skull key
        case MT_MISC8:
            if (viewplayer->cards[it_redskull] <= 0)
                return false;

            viewplayer->cards[it_redskull] = 0;
            P_LookForCards();
            cardsfound--;
            return true;

        // stimpack
        case MT_MISC10:
            if ((viewplayer->cheats & CF_GODMODE)
                || viewplayer->powers[pw_invulnerability]
                || ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health <= STIMPACKHEALTH)
                || viewplayer->health < STIMPACKHEALTH)
                return false;

            viewplayer->health -= STIMPACKHEALTH;
            viewplayer->mo->health -= STIMPACKHEALTH;
            viewplayer->damagecount = STIMPACKHEALTH;
            P_AnimateHealth(STIMPACKHEALTH);
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            S_StartSound(NULL, sfx_plpain);
            return true;

        // medikit
        case MT_MISC11:
            if ((viewplayer->cheats & CF_GODMODE)
                || viewplayer->powers[pw_invulnerability]
                || ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health <= MEDIKITHEALTH)
                || viewplayer->health < MEDIKITHEALTH)
                return false;

            viewplayer->health -= MEDIKITHEALTH;
            viewplayer->mo->health -= MEDIKITHEALTH;
            viewplayer->damagecount = MEDIKITHEALTH;
            P_AnimateHealth(MEDIKITHEALTH);
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            S_StartSound(NULL, sfx_plpain);
            return true;

        // invulnerability power-up
        case MT_INV:
            if (!viewplayer->powers[pw_invulnerability])
                return false;

            if (freeze)
            {
                viewplayer->powers[pw_invulnerability] = 0;
                viewplayer->fixedcolormap = 0;
                st_faceindex = ST_STRAIGHTFACE;
                st_facecount = ST_STRAIGHTFACECOUNT;
            }
            else
                viewplayer->powers[pw_invulnerability] = STARTFLASHING;

            return true;

        // berserk power-up
        case MT_MISC13:
            if (!viewplayer->powers[pw_strength])
                return false;

            viewplayer->powers[pw_strength] = 0;
            viewplayer->fistorchainsaw = wp_chainsaw;

            if (viewplayer->readyweapon == wp_fist && viewplayer->weaponowned[wp_chainsaw])
                viewplayer->pendingweapon = wp_chainsaw;

            return true;

        // partial invisibility power-up
        case MT_INS:
            if (!viewplayer->powers[pw_invisibility])
                return false;

            viewplayer->powers[pw_invisibility] = STARTFLASHING * !freeze;
            return true;

        // radiation shielding suit power-up
        case MT_MISC14:
            if (!viewplayer->powers[pw_ironfeet])
                return false;

            viewplayer->powers[pw_ironfeet] = STARTFLASHING * !freeze;
            return true;

        // computer area map power-up
        case MT_MISC15:
            if (!viewplayer->powers[pw_allmap])
                return false;

            viewplayer->powers[pw_allmap] = 0;
            return true;

        // light amplification visor power-up
        case MT_MISC16:
            if (!viewplayer->powers[pw_infrared])
                return false;

            viewplayer->powers[pw_infrared] = STARTFLASHING * !freeze;
            return true;

        // clip
        case MT_CLIP:
            return P_TakeAmmo(am_clip, 1);

        // box of bullets
        case MT_MISC17:
            return P_TakeAmmo(am_clip, 5);

        // rocket
        case MT_MISC18:
            return P_TakeAmmo(am_misl, 1);

        // box of rockets
        case MT_MISC19:
            return P_TakeAmmo(am_misl, 5);

        // cell
        case MT_MISC20:
            return P_TakeAmmo(am_cell, 1);

        // cell pack
        case MT_MISC21:
            return P_TakeAmmo(am_cell, 5);

        // shell
        case MT_MISC22:
            return P_TakeAmmo(am_shell, 1);

        // box of shells
        case MT_MISC23:
            return P_TakeAmmo(am_shell, 5);

        // backpack
        case MT_MISC24:
            return P_TakeBackpack();

        // BFG-9000
        case MT_MISC25:
            return P_TakeWeapon(wp_bfg);

        // chaingun
        case MT_CHAINGUN:
            return P_TakeWeapon(wp_chaingun);

        // chainsaw
        case MT_MISC26:
            return P_TakeWeapon(wp_chainsaw);

        // rocket launcher
        case MT_MISC27:
            return P_TakeWeapon(wp_missile);

        // plasma rifle
        case MT_MISC28:
            return P_TakeWeapon(wp_plasma);

        // shotgun
        case MT_SHOTGUN:
            return P_TakeWeapon(wp_shotgun);

        // super shotgun
        case MT_SUPERSHOTGUN:
            return P_TakeWeapon(wp_supershotgun);

        default:
            return false;
    }
}

static void P_WriteObituary(mobj_t *target, mobj_t *inflicter, mobj_t *source, const bool gibbed, const bool telefragged)
{
    if (telefragged)
    {
        if (target->player)
        {
            char    sourcename[128];

            if (*source->name)
                M_StringCopy(sourcename, source->name, sizeof(sourcename));
            else
            {
                const bool  friendly = (source->flags & MF_FRIEND);

                M_snprintf(sourcename, sizeof(sourcename), "%s %s%s",
                    (friendly && source->type > MT_NULL && source->type < NUMMOBJTYPES && monstercount[source->type] == 1 ? "the" :
                        (*source->info->name1 && isvowel(source->info->name1[0]) && !friendly ? "an" : "a")),
                    (friendly ? "friendly " : ""),
                    (*source->info->name1 && !M_StringStartsWith(source->info->name1, "Deh_Actor_") ? source->info->name1 : "monster"));
            }

            if (M_StringCompare(playername, playername_default))
                C_PlayerObituary("You were telefragged by %s!", sourcename);
            else
                C_PlayerObituary("%s was telefragged by %s!", playername, sourcename);
        }
        else if (source->player)
        {
            char    targetname[128];

            if (*target->name)
                M_StringCopy(targetname, target->name, sizeof(targetname));
            else
            {
                const bool  friendly = (target->flags & MF_FRIEND);

                M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                    (friendly && target->type > MT_NULL && target->type < NUMMOBJTYPES && monstercount[target->type] == 1 ? "the" :
                        (*target->info->name1 && isvowel(target->info->name1[0]) && !friendly ? "an" : "a")),
                    (friendly ? "friendly " : ""),
                    (*target->info->name1 && !M_StringStartsWith(target->info->name1, "Deh_Actor_") ? target->info->name1 : "monster"));
            }

            C_PlayerMessage("%s telefragged %s.",
                (M_StringCompare(playername, playername_default) ? "You" : playername), targetname);
        }
        else
        {
            char    sourcename[128];
            char    targetname[128];

            if (*source->name)
                M_StringCopy(sourcename, source->name, sizeof(sourcename));
            else
            {
                const bool  friendly = (source->flags & MF_FRIEND);

                M_snprintf(sourcename, sizeof(sourcename), "%s %s%s",
                    (friendly && source->type > MT_NULL && source->type < NUMMOBJTYPES && monstercount[source->type] == 1 ? "the" :
                        (*source->info->name1 && isvowel(source->info->name1[0]) && !friendly ? "an" : "a")),
                    (friendly ? "friendly " : ""),
                    (*source->info->name1 && !M_StringStartsWith(source->info->name1, "Deh_Actor_") ? source->info->name1 : "monster"));
            }

            if (*target->name)
                M_StringCopy(targetname, target->name, sizeof(targetname));
            else
            {
                const bool  friendly = (target->flags & MF_FRIEND);

                M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                    (friendly && target->type < NUMMOBJTYPES && monstercount[target->type] == 1 ? "the" :
                        (*target->info->name1 && isvowel(target->info->name1[0]) && !friendly ? "an" : "a")),
                    (friendly ? "friendly " : ""),
                    (*target->info->name1 && !M_StringStartsWith(target->info->name1, "Deh_Actor_") ? target->info->name1 : "monster"));
            }

            C_PlayerMessage("%s was telefragged by %s.", targetname, sourcename);
        }
    }
    else if (source)
    {
        if (inflicter && inflicter->type == MT_BARREL && target->type != MT_BARREL)
        {
            char    *inflictername = inflicter->info->name1;

            if (target->player)
            {
                if (inflicter->inflicter == MT_PLAYER)
                {
                    if (M_StringCompare(playername, playername_default))
                        C_PlayerObituary("You were %s by %s %s that you exploded!",
                            (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"));
                    else
                        C_PlayerObituary("%s was %s by %s %s that %s exploded!",
                            playername,
                            (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"),
                            pronoun(personal));
                }
                else
                {
                    if (M_StringCompare(playername, playername_default))
                        C_PlayerObituary("You were %s by %s %s that %s %s exploded!",
                            (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"),
                            (inflicter->type == inflicter->inflicter
                                || M_StringCompare(inflictername, mobjinfo[inflicter->inflicter].name1) ? "another" :
                                (isvowel(mobjinfo[inflicter->inflicter].name1[0]) ? "an" : "a")),
                            (*mobjinfo[inflicter->inflicter].name1 && !M_StringStartsWith(mobjinfo[inflicter->inflicter].name1, "Deh_Actor_") ?
                                mobjinfo[inflicter->inflicter].name1 : "monster"));
                    else
                        C_PlayerObituary("%s was %s by %s %s that %s %s exploded!",
                            playername,
                            (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"),
                            (inflicter->type == inflicter->inflicter
                                || M_StringCompare(inflictername, mobjinfo[inflicter->inflicter].name1) ? "another" :
                                (isvowel(mobjinfo[inflicter->inflicter].name1[0]) ? "an" : "a")),
                            (*mobjinfo[inflicter->inflicter].name1 && !M_StringStartsWith(mobjinfo[inflicter->inflicter].name1, "Deh_Actor_") ?
                                mobjinfo[inflicter->inflicter].name1 : "monster"));
                }
            }
            else
            {
                char    targetname[128];
                char    *temp;

                if (*target->name)
                    M_StringCopy(targetname, target->name, sizeof(targetname));
                else
                {
                    const bool  friendly = (target->flags & MF_FRIEND);
                    const bool  corpse = ((target->flags & MF_CORPSE) && source != target);

                    M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                        (friendly && target->type > MT_NULL && target->type < NUMMOBJTYPES && monstercount[target->type] == 1 ? "the" :
                            (*target->info->name1 && isvowel(target->info->name1[0]) && !corpse && !friendly ? "an" : "a")),
                        (corpse && !M_StringStartsWith(target->info->name1, "dead ") ? "dead " : (friendly ? "friendly " : "")),
                        (*target->info->name1 && !M_StringStartsWith(target->info->name1, "Deh_Actor_") ? target->info->name1 : "monster"));
                }

                temp = sentencecase(targetname);

                if (inflicter->inflicter == MT_PLAYER)
                {
                    if (M_StringCompare(playername, playername_default))
                        C_PlayerMessage("%s was %s by %s %s that you exploded.",
                            temp,
                            (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"));
                    else
                        C_PlayerMessage("%s was %s by %s %s that %s exploded.",
                            temp,
                            (gibbed ? s_GIBBED : s_KILLED),
                            (inflictername &&isvowel(inflictername[0]) ? "an" : "a"),
                            (inflictername ? inflictername : "barrel"),
                            playername);
                }
                else if (source == target)
                    C_PlayerMessage("%s was %s by %s %s that they exploded.",
                        temp,
                        (gibbed ? s_GIBBED : s_KILLED),
                        (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                        (inflictername ? inflictername : "barrel"));
                else
                    C_PlayerMessage("%s was %s by %s %s that %s %s exploded.",
                        temp,
                        (gibbed ? s_GIBBED : s_KILLED),
                        (inflictername && isvowel(inflictername[0]) ? "an" : "a"),
                        (inflictername ? inflictername : "barrel"),
                        (inflicter->type == inflicter->inflicter
                            || M_StringCompare(inflictername, mobjinfo[inflicter->inflicter].name1) ? "another" :
                            (*mobjinfo[inflicter->inflicter].name1 && isvowel(mobjinfo[inflicter->inflicter].name1[0]) ? "an" : "a")),
                        (*mobjinfo[inflicter->inflicter].name1 && !M_StringStartsWith(mobjinfo[inflicter->inflicter].name1, "Deh_Actor_") ?
                            mobjinfo[inflicter->inflicter].name1 : "monster"));

                free(temp);
            }
        }
        else if (source->player || source->type == MT_BFG)
        {
            const weapontype_t  readyweapon = viewplayer->readyweapon;

            if (source->player && source->player->mo != source)
                return;

            if (M_StringCompare(playername, playername_default))
            {
                if (target->player)
                {
                    if (healthcvar)
                        C_PlayerObituary("You %s yourself!", s_KILLED);
                    else
                        C_PlayerObituary("You %s yourself with your own %s!",
                            (gibbed ? s_GIBBED : s_KILLED),
                            weaponinfo[readyweapon].name);
                }
                else
                {
                    char    targetname[128];

                    if (*target->name)
                        M_StringCopy(targetname, target->name, sizeof(targetname));
                    else
                    {
                        const bool  friendly = (target->flags & MF_FRIEND);

                        M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                            (friendly && target->type > MT_NULL && target->type < NUMMOBJTYPES && monstercount[target->type] == 1 ? "the" :
                                (*target->info->name1 && isvowel(target->info->name1[0]) && !friendly ? "an" : "a")),
                            (friendly ? "friendly " : ""),
                            (*target->info->name1 && !M_StringStartsWith(target->info->name1, "Deh_Actor_") ? target->info->name1 : "monster"));
                    }

                    if (readyweapon == wp_fist && viewplayer->powers[pw_strength])
                        C_PlayerMessage("You %s %s with your %s while %s.",
                            (target->type == MT_BARREL ? "exploded" :
                                (target->type == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                            targetname,
                            weaponinfo[readyweapon].name,
                            berserk);
                    else
                        C_PlayerMessage("You %s %s with your %s.",
                            (target->type == MT_BARREL ? "exploded" :
                                (target->type == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                            targetname,
                            weaponinfo[readyweapon].name);
                }
            }
            else
            {
                if (target->player)
                {
                    if (healthcvar)
                        C_PlayerObituary("%s %s %s!",
                            playername, s_KILLED, pronoun(reflexive));
                    else
                        C_PlayerObituary("%s %s %s with %s own %s!",
                            playername,
                            (gibbed ? s_GIBBED : s_KILLED),
                            pronoun(reflexive),
                            pronoun(possessive),
                            weaponinfo[readyweapon].name);
                }
                else
                {
                    char    targetname[128];

                    if (*target->name)
                        M_StringCopy(targetname, target->name, sizeof(targetname));
                    else
                    {
                        const bool  friendly = (target->flags & MF_FRIEND);
                        const bool  corpse = (target->flags & MF_CORPSE);

                        M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                            (friendly && target->type > MT_NULL && target->type < NUMMOBJTYPES && monstercount[target->type] == 1 ? "the" :
                                (*target->info->name1 && isvowel(target->info->name1[0]) && !corpse && !friendly ? "an" : "a")),
                            (corpse && !M_StringStartsWith(target->info->name1, "dead ") ? "dead " : (friendly ? "friendly " : "")),
                            (*target->info->name1 && !M_StringStartsWith(target->info->name1, "Deh_Actor_") ? target->info->name1 : "monster"));
                    }

                    if (readyweapon == wp_fist && viewplayer->powers[pw_strength])
                        C_PlayerMessage("%s %s %s with %s %s while %s.",
                            playername,
                            (target->type == MT_BARREL ? "exploded" :
                                (target->type == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                            targetname,
                            pronoun(possessive),
                            weaponinfo[readyweapon].name,
                            berserk);
                    else
                        C_PlayerMessage("%s %s %s with %s %s.",
                            playername,
                            (target->type == MT_BARREL ? "exploded" :
                                (target->type == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                            targetname,
                            pronoun(possessive),
                            weaponinfo[readyweapon].name);
                }
            }
        }
        else
        {
            char    sourcename[128];

            if (*source->name)
                M_StringCopy(sourcename, source->name, sizeof(sourcename));
            else
            {
                const bool  friendly = (source->flags & MF_FRIEND);

                M_snprintf(sourcename, sizeof(sourcename), "%s %s%s",
                    (friendly && source->type > MT_NULL && source->type < NUMMOBJTYPES && monstercount[source->type] == 1 ? "the" :
                        (*source->info->name1 && isvowel(source->info->name1[0]) && !friendly ? "an" : "a")),
                    (friendly ? "friendly " : ""),
                    (*source->info->name1 && !M_StringStartsWith(source->info->name1, "Deh_Actor_") ? source->info->name1 : "monster"));
            }

            if (target->player)
            {
                if (M_StringCompare(playername, playername_default))
                    C_PlayerObituary("You were %s by %s!",
                        (gibbed ? s_GIBBED : s_KILLED), sourcename);
                else
                    C_PlayerObituary("%s was %s by %s!",
                        playername, (gibbed ? s_GIBBED : s_KILLED), sourcename);
            }
            else
            {
                char    targetname[128];
                char    *temp = sentencecase(sourcename);

                if (*target->name)
                    M_StringCopy(targetname, target->name, sizeof(targetname));
                else
                {
                    const bool  friendly = (target->flags & MF_FRIEND);

                    M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                        (source->type == target->type || M_StringCompare(source->info->name1, target->info->name1) ? "another" :
                            (friendly && target->type > MT_NULL && target->type < NUMMOBJTYPES && monstercount[target->type] == 1 ? "the" :
                            (*target->info->name1 && isvowel(target->info->name1[0]) && !friendly ? "an" : "a"))),
                        (friendly ? "friendly " : ""),
                        (*target->info->name1 && !M_StringStartsWith(target->info->name1, "Deh_Actor_") ? target->info->name1 : "monster"));
                }

                C_PlayerMessage("%s %s %s.",
                    temp,
                    (target->type == MT_BARREL ? "exploded" :
                        (target->type == MT_EXTRA50 && legacyofrust ? "broke" : (gibbed ? s_GIBBED : s_KILLED))),
                    targetname);

                free(temp);
            }
        }
    }
    else if (target->player && target->player->mo == target)
    {
        const sector_t  *sector = viewplayer->mo->subsector->sector;

        if (sector->ceilingdata && sector->ceilingheight - sector->floorheight < VIEWHEIGHT)
        {
            if (M_StringCompare(playername, playername_default))
                C_PlayerObituary("You were crushed to death!");
            else
                C_PlayerObituary("%s was crushed to death!", playername);
        }
        else
        {
            if (sector->terraintype >= LIQUID)
            {
                const char *liquids[][2] =
                {
                    { "liquid",     "liquid"     },
                    { "nukage",     "nukage"     },
                    { "water",      "water"      },
                    { "lava",       "lava"       },
                    { "blood",      "blood"      },
                    { "slime",      "slime"      },
                    { "gray slime", "grey slime" },
                    { "goop",       "goop"       },
                    { "icy water",  "icy water"  },
                    { "tar",        "tar"        },
                    { "sludge",     "sludge"     }
                };

                C_PlayerObituary("%s died in %s.",
                    (M_StringCompare(playername, playername_default) ? "You" : playername),
                    liquids[sector->terraintype - LIQUID][english]);
            }
            else
            {
                const short floorpic = sector->floorpic;

                if ((floorpic >= RROCK05 && floorpic <= RROCK08) || (floorpic >= SLIME09 && floorpic <= SLIME12))
                    C_PlayerObituary("%s died on molten rock.",
                        (M_StringCompare(playername, playername_default) ? "You" : playername));
                else
                    C_PlayerObituary("%s died.", (M_StringCompare(playername, playername_default) ? "You" : playername));
            }
        }
    }
}

static void P_SpawnGibBlood(mobj_t *target)
{
    if (r_blood_gibs && !(target->flags & MF_NOBLOOD))
        for (int i = 1; i <= 30; i++)
        {
            mobj_t  *mo = P_SpawnMobj(target->x, target->y, target->z + target->height * 2 / 3, MT_BLOOD);

            mo->momx = M_BigRandomInt(2, 8) * FRACUNIT;
            mo->momy = M_BigRandomInt(-8, 8) * FRACUNIT;
            mo->momz = M_BigRandomInt(-8, 8) * FRACUNIT;

            if (M_BigRandom() & 1)
                mo->flags2 |= MF2_MIRRORED;

            mo->colfunc = bloodcolfunc;
            mo->altcolfunc = bloodcolfunc;
            mo->bloodcolor = target->bloodcolor;
        }
}

//
// P_KillMobj
//
void P_KillMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source, const bool telefragged)
{
    bool                gibbed;
    const mobjtype_t    type = target->type;
    const mobjinfo_t    *info = &mobjinfo[type];
    const int           gibhealth = info->gibhealth;

    target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY);

    if (type == MT_SKULL)
    {
        target->momx = 0;
        target->momy = 0;
        target->momz = 0;
    }
    else
        target->flags &= ~MF_NOGRAVITY;

    target->flags2 &= ~MF2_PASSMOBJ;
    target->height >>= 2;
    target->geartime = MAXGEARTIME; // [JN] Limit torque to 15 seconds
    target->floatbob = (M_BigRandom() & 63);

    // killough 08/29/98: remove from threaded list
    P_UpdateThinker(&target->thinker);

    if (type != MT_BARREL)
    {
        if (!(target->flags & MF_FUZZ))
            target->bloodsplats = CORPSEBLOODSPLATS;

        if (r_corpses_mirrored && (M_BigRandom() & 1) && !(target->flags2 & MF2_NOMIRROREDCORPSE)
            && (type != MT_PAIN || !doom4vanilla))
            target->flags2 |= MF2_MIRRORED;
    }

    if (target->flags & MF_COUNTKILL)
    {
        // count all monster deaths, even those caused by other monsters
        if (!(target->flags & MF_FRIEND))
            viewplayer->killcount++;

        if ((source && source->player) || massacre)
        {
            stat_monsterskilled_total = SafeAdd(stat_monsterskilled_total, 1);

            if (type > MT_NULL && type < NUMMOBJTYPES)
            {
                viewplayer->monsterskilled[type]++;
                stat_monsterskilled[type] = SafeAdd(stat_monsterskilled[type], 1);
            }
        }
        else
        {
            viewplayer->infightcount++;
            stat_monsterskilled_infighting = SafeAdd(stat_monsterskilled_infighting, 1);
        }
    }
    else if (type == MT_BARREL)
    {
        viewplayer->monsterskilled[type]++;
        stat_barrelsexploded = SafeAdd(stat_barrelsexploded, 1);

        if (inflicter)
            P_SetTarget(&target->target, inflicter);
        else if (source)
            P_SetTarget(&target->target, source);
    }

    if (inflicter)
        target->inflicter = inflicter->type;

    if (target->player)
    {
        target->flags &= ~MF_SOLID;
        viewplayer->playerstate = PST_DEAD;
        P_DropWeapon();

        if (automapactive)
            AM_Stop();          // don't die in automap, switch view prior to dying

        if ((source && source->player)
            || (inflicter && inflicter->type == MT_BARREL && inflicter->inflicter == MT_PLAYER))
        {
            viewplayer->suicides++;
            stat_suicides = SafeAdd(stat_suicides, 1);
        }
        else
        {
            viewplayer->deaths++;
            stat_deaths = SafeAdd(stat_deaths, 1);
        }
    }
    else
    {
        target->flags2 &= ~MF2_NOLIQUIDBOB;
        target->angle += (M_BigSubRandom() << 20);

        if (telefragged)
        {
            viewplayer->telefragcount++;
            stat_monsterstelefragged = SafeAdd(stat_monsterstelefragged, 1);
        }
    }

    if ((gibbed = (gibhealth < 0 && target->health < gibhealth && info->xdeathstate != S_NULL)))
    {
        P_SetMobjState(target, info->xdeathstate);
        P_SpawnGibBlood(target);
        viewplayer->monstersgibbed++;
        stat_monstersgibbed = SafeAdd(stat_monstersgibbed, 1);
        target->giblevel = 1;
        target->gibtimer = TICRATE;
    }
    else
        P_SetMobjState(target, info->deathstate);

    if (!target->player)
        target->health = -1;

    target->tics = MAX(1, target->tics - (M_Random() & 3));

    if (type == MT_BARREL || (type == MT_PAIN && !doom4vanilla) || type == MT_SKULL)
        target->flags2 |= MF2_EXPLODING;

    if (obituaries && !hacx && !massacre)
        P_WriteObituary(target, inflicter, source, gibbed, telefragged);

    target->flags |= (MF_CORPSE | MF_DROPOFF);

    if (chex)
        return;

    // Drop stuff.
    // This determines the kind of object spawned during the death frame of a thing.
    if (info->droppeditem != MT_NULL)
    {
        mobj_t  *mo;

        if (tossdrop)
        {
            mo = P_SpawnMobj(target->x, target->y, target->floorz + target->height * 3 / 2 - 3 * FRACUNIT, info->droppeditem);

            mo->momx = (target->momx >> 1) + (M_BigSubRandom() << 8);
            mo->momy = (target->momy >> 1) + (M_BigSubRandom() << 8);
            mo->momz = 3 * FRACUNIT + ((M_BigRandom() & 255) << 9);
        }
        else
            mo = P_SpawnMobj(target->x, target->y, ONFLOORZ, info->droppeditem);

        mo->angle = target->angle + (M_BigSubRandom() << 20);
        mo->flags |= MF_DROPPED;    // special versions of items
        mo->geartime = MAXGEARTIME;
        mo->floatbob = (M_BigRandom() & 63);

        if (r_mirroredweapons && (M_BigRandom() & 1))
            mo->flags2 |= MF2_MIRRORED;

        if (massacre)
            mo->flags2 |= MF2_MASSACRE;
    }
}

// MBF21: dehacked infighting groups
static bool P_InfightingImmune(const mobj_t *target, const mobj_t *source)
{
    // not default behavior, and same group
    return (mobjinfo[target->type].infightinggroup
        && mobjinfo[target->type].infightinggroup == mobjinfo[source->type].infightinggroup);
}

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
void P_DamageMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source, int damage, const bool adjust, const bool telefragged)
{
    player_t            *splayer = NULL;
    player_t            *tplayer;
    const int           flags = target->flags;
    const bool          corpse = (flags & MF_CORPSE);
    const mobjtype_t    type = target->type;
    mobjinfo_t          *info = &mobjinfo[type];
    bool                justhit = false;

    if (!(flags & (MF_SHOOTABLE | MF_BOUNCES)) && (!corpse || !r_corpses_slide || target->giblevel == 2))
        return;

    if (type == MT_BARREL && corpse && target == inflicter)
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

    if (tplayer && gameskill == sk_baby && adjust)
        damage >>= (damage > 1);

    // Some close combat weapons should not
    // inflict thrust and push the victim out of reach,
    // thus kick away unless using the chainsaw.
    if (massacre)
    {
        if (r_bloodsplats_max
            && !(flags & MF_NOBLOOD) && type != MT_SKULL
            && target->bloodcolor > NOBLOOD)
        {
            const short lump = sprites[target->sprite].spriteframes[target->frame & FF_FRAMEMASK].lump[0];

            if (moreblood || lumpinfo[firstspritelump + lump]->wadfile->type == IWAD)
            {
                angle_t ang = R_PointToAngle2(target->x + (M_BigRandomInt(-100, 100) << FRACBITS),
                    target->y + (M_BigRandomInt(-100, 100) << FRACBITS), target->x, target->y) >> ANGLETOFINESHIFT;

                target->momx += FixedMul(MASSACRETHRUST, finecosine[ang]);
                target->momy += FixedMul(MASSACRETHRUST, finesine[ang]);

                if (r_corpses_moreblood)
                    P_SpawnMoreBlood(target);
            }
        }
    }
    else if (inflicter && !healthcvar && !(flags & MF_NOCLIP)
        && (!source || !splayer || !(weaponinfo[splayer->readyweapon].flags & WPF_NOTHRUST)))
    {
        angle_t ang = R_PointToAngle2(inflicter->x, inflicter->y, target->x, target->y);
        fixed_t thrust = damage * (FRACUNIT >> 3) * 100 / MAX((corpse ? 200 : 1), info->mass);

        // make fall forwards sometimes
        if (damage < 40 && damage > target->health && target->z - inflicter->z > 64 * FRACUNIT && (M_Random() & 1))
        {
            ang += ANG180;
            thrust *= 4;
        }

        target->momx += FixedMul(thrust, finecosine[(ang >>= ANGLETOFINESHIFT)]);
        target->momy += FixedMul(thrust, finesine[ang]);

        // killough 11/98: thrust objects hanging off ledges
        if ((target->flags2 & MF2_FALLING) && target->gear >= MAXGEAR)
            target->gear = 0;
    }

    if (corpse)
    {
        // [BH] gib corpse if enough damage
        if (r_corpses_gib && damage >= 25 && !(flags & MF_NOBLOOD))
        {
            statenum_t  state = info->xdeathstate;

            if (state != S_NULL)
            {
                if (!target->giblevel)
                {
                    target->giblevel = 1;
                    target->gibtimer = TICRATE;

                    while (states[state].nextstate == state + 1)
                        state++;

                    P_SetMobjState(target, state);
                    S_StartSound(target, sfx_slop);

                    if (r_corpses_mirrored && (M_BigRandom() & 1))
                        target->flags2 ^= MF2_MIRRORED;

                    P_SpawnGibBlood(target);
                }
                else if (target->giblevel == 1 && !target->gibtimer)
                {
                    target->giblevel = 2;
                    target->flags2 &= ~MF2_CASTSHADOW;

                    P_SetMobjState(target, S_GIBS);
                    S_StartSound(target, sfx_slop);

                    if (r_corpses_mirrored && (M_BigRandom() & 1))
                        target->flags2 ^= MF2_MIRRORED;

                    P_SpawnGibBlood(target);
                }
            }
        }

        return;
    }

    // player specific
    if (splayer && type != MT_BARREL)
    {
        splayer->damageinflicted += damage;
        stat_damageinflicted = SafeAdd(stat_damageinflicted, damage);
    }

    if (tplayer)
    {
        const int   cheats = tplayer->cheats;

        if (freeze && (!inflicter || !inflicter->player))
            return;

        // end of game hell hack
        if (target->subsector->sector->special == DamageNegative10Or20PercentHealthAndEndLevel && damage >= target->health)
            damage = target->health - 1;

        // below certain threshold, ignore damage if player has invulnerability power-up
        if (tplayer->powers[pw_invulnerability] && damage < 1000)
            return;

        if (!(cheats & CF_GODMODE) && !idclevtics)
        {
            if (adjust && tplayer->armor)
            {
                int saved = damage / (tplayer->armortype == green_armor_class ? 3 : 2);

                if (tplayer->armor <= saved)
                {
                    // armor is used up
                    saved = tplayer->armor;
                    tplayer->armortype = armortype_none;
                }

                if (saved)
                {
                    tplayer->armor -= saved;
                    damage -= saved;
                    P_AnimateArmor(saved);
                    armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
                }
            }

            tplayer->health -= damage;
            tplayer->negativehealth = tplayer->health;
            target->health -= damage;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;

            if ((cheats & CF_BUDDHA) && tplayer->health <= 0)
            {
                const int   stat = tplayer->health + damage - 1;

                tplayer->damagereceived += stat;
                stat_damagereceived = SafeAdd(stat_damagereceived, stat);
                P_AnimateHealth(stat);

                tplayer->health = 1;
                tplayer->negativehealth = 1;
                target->health = 1;
            }
            else
            {
                tplayer->damagereceived += damage;
                stat_damagereceived = SafeAdd(stat_damagereceived, damage);
                P_AnimateHealth(damage);
            }
        }

        if (tplayer->mo == target)
            tplayer->attacker = source;

        if (tplayer->health <= 0)
        {
            tplayer->health = 0;

            if (tplayer->negativehealth < HUD_NUMBER_MIN)
                tplayer->negativehealth = HUD_NUMBER_MIN;

            tplayer->damagecount = 100;
            P_KillMobj(target, inflicter, source, telefragged);
        }
        else
        {
            // add damage after armor/invuln
            int damagecount = tplayer->damagecount + damage;

            if (damage > 0 && damagecount < 8)
                damagecount = 8;

            tplayer->damagecount = MIN(damagecount, ((cheats & CF_GODMODE) ? 30 : 100));
        }

        if (joy_rumble_damage)
        {
            const short strength = (30000 + (100 - MIN(tplayer->health, 100)) / 100 * 30000) * joy_rumble_damage / 100;

            I_ControllerRumble(strength, strength);
            damagerumbletics += BETWEEN(12, damage, 100);
        }

        if (tplayer->health <= 0)
            return;
    }
    else if ((target->health -= damage) <= 0)   // do the damage
    {
        if (!(flags & MF_FUZZ) && target->state != S_NULL && !target->state->dehacked
            && (type == MT_BARREL || type == MT_SKULL || type == MT_PAIN))
            target->colfunc = tlredcolfunc;

        // [crispy] the lethal pellet of a point-blank SSG blast
        // gets an extra damage boost for the occasional gib chance
        if (splayer && splayer->readyweapon == wp_supershotgun && info->xdeathstate != S_NULL
            && damage >= 10 && info->gibhealth < 0 && P_CheckMeleeRange(target))
            target->health = info->gibhealth - 1;

        P_KillMobj(target, inflicter, source, telefragged);
        return;
    }

    if (M_Random() < info->painchance && !(flags & MF_SKULLFLY) && (!tplayer || !(viewplayer->cheats & CF_GODMODE)))
    {
        justhit = true;
        P_SetMobjState(target, info->painstate);
    }

    // we're awake now...
    target->reactiontime = 0;

    if ((!target->threshold || (target->mbf21flags & MF_MBF21_NOTHRESHOLD))
        && source && source != target && !(source->mbf21flags & MF_MBF21_DMGIGNORED)
        && !P_InfightingImmune(target, source))
    {
        const state_t   *state = target->state;
        const state_t   *spawnstate = &states[info->spawnstate];

        // if not intent on another player, chase after this one
        if (!target->lastenemy || target->lastenemy->health <= 0 || !target->lastenemy->player)
            P_SetTarget(&target->lastenemy, target->target);    // remember last enemy -- killough

        P_SetTarget(&target->target, source);                   // killough 11/98
        target->threshold = BASETHRESHOLD;

        // [BH] Fix enemy not waking up if damaged during second frame of their idle animation
        if ((state == spawnstate || state == &states[spawnstate->nextstate]) && info->seestate != S_NULL)
            P_SetMobjState(target, info->seestate);
    }

    // fight back!
    if (justhit && (target->target == source || !target->target || !(flags & target->target->flags & MF_FRIEND)))
        target->flags |= MF_JUSTHIT;
}

//
// P_ResurrectMobj
//
void P_ResurrectMobj(mobj_t *target)
{
    const mobjinfo_t    *info = target->info;
    const mobjtype_t    type = target->type;

    S_StartSound(target, sfx_slop);
    P_SetMobjState(target, info->raisestate);

    target->height = info->height;
    target->radius = info->radius;
    target->flags = (info->flags | (target->flags & MF_FRIEND));
    target->flags2 = info->flags2;
    target->health = info->spawnhealth;
    target->shadowoffset = info->shadowoffset;
    target->giblevel = 0;
    target->gibtimer = 0;

    P_SetTarget(&target->target, NULL);
    P_SetTarget(&target->lastenemy, NULL);

    viewplayer->killcount--;
    stat_monsterskilled_total--;
    viewplayer->resurrectioncount++;
    stat_monstersresurrected = SafeAdd(stat_monstersresurrected, 1);

    if (type > MT_NULL && type < NUMMOBJTYPES)
        stat_monsterskilled[type] = SafeAdd(stat_monsterskilled[type], -1);

    P_UpdateThinker(&target->thinker);
}
