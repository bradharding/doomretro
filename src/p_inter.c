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

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"

// Ty 03/07/98 - add deh externals
// Maximums and such were hardcoded values. Need to externalize those for
// dehacked support (and future flexibility). Most var names came from the key
// strings used in dehacked.
int             initial_health = 100;
int             initial_bullets = 50;
int             maxhealth = MAXHEALTH * 2;
int             max_armor = 200;
int             green_armor_class = armortype_green;
int             blue_armor_class = armortype_blue;
int             max_soul = 200;
int             soul_health = 100;
int             mega_health = 200;
int             god_health = 100;
int             idfa_armor = 200;
int             idfa_armor_class = armortype_blue;
int             idkfa_armor = 200;
int             idkfa_armor_class = armortype_blue;
int             bfgcells = BFGCELLS;
dboolean        species_infighting = false;

// a weapon is found with two clip loads,
// a big item has five clip loads
int             maxammo[NUMAMMO] =  { 200, 50, 300, 50 };
int             clipammo[NUMAMMO] = {  10,  4,  20,  1 };

static int      cardsprites[NUMCARDS] = { SPR_BKEY, SPR_YKEY, SPR_RKEY, SPR_BSKU, SPR_YSKU, SPR_RSKU };

dboolean        con_obituaries = con_obituaries_default;
dboolean        r_mirroredweapons = r_mirroredweapons_default;
dboolean        tossdrop = tossdrop_default;

unsigned int    stat_barrelsexploded = 0;
unsigned int    stat_damageinflicted = 0;
unsigned int    stat_damagereceived = 0;
unsigned int    stat_deaths = 0;
unsigned int    stat_itemspickedup = 0;
unsigned int    stat_itemspickedup_ammo_bullets = 0;
unsigned int    stat_itemspickedup_ammo_cells = 0;
unsigned int    stat_itemspickedup_ammo_rockets = 0;
unsigned int    stat_itemspickedup_ammo_shells = 0;
unsigned int    stat_itemspickedup_armor = 0;
unsigned int    stat_itemspickedup_health = 0;
unsigned int    stat_monsterskilled = 0;
unsigned int    stat_monsterskilled_arachnotrons = 0;
unsigned int    stat_monsterskilled_archviles = 0;
unsigned int    stat_monsterskilled_baronsofhell = 0;
unsigned int    stat_monsterskilled_cacodemons = 0;
unsigned int    stat_monsterskilled_cyberdemons = 0;
unsigned int    stat_monsterskilled_demons = 0;
unsigned int    stat_monsterskilled_heavyweapondudes = 0;
unsigned int    stat_monsterskilled_hellknights = 0;
unsigned int    stat_monsterskilled_imps = 0;
unsigned int    stat_monsterskilled_lostsouls = 0;
unsigned int    stat_monsterskilled_mancubi = 0;
unsigned int    stat_monsterskilled_painelementals = 0;
unsigned int    stat_monsterskilled_revenants = 0;
unsigned int    stat_monsterskilled_shotgunguys = 0;
unsigned int    stat_monsterskilled_spectres = 0;
unsigned int    stat_monsterskilled_spidermasterminds = 0;
unsigned int    stat_monsterskilled_zombiemen = 0;

extern dboolean healthcvar;
extern int      idclevtics;

void P_UpdateAmmoStat(ammotype_t ammotype, int num)
{
    switch (ammotype)
    {
        case am_clip:
            viewplayer->itemspickedup_ammo_bullets += num;
            stat_itemspickedup_ammo_bullets = SafeAdd(stat_itemspickedup_ammo_bullets, num);
            break;

        case am_shell:
            viewplayer->itemspickedup_ammo_shells += num;
            stat_itemspickedup_ammo_shells = SafeAdd(stat_itemspickedup_ammo_shells, num);
            break;

        case am_cell:
            viewplayer->itemspickedup_ammo_cells += num;
            stat_itemspickedup_ammo_cells = SafeAdd(stat_itemspickedup_ammo_cells, num);
            break;

        case am_misl:
            viewplayer->itemspickedup_ammo_rockets += num;
            stat_itemspickedup_ammo_rockets = SafeAdd(stat_itemspickedup_ammo_rockets, num);
            break;

        default:
            break;
    }
}

//
// P_TakeAmmo
//
static dboolean P_TakeAmmo(ammotype_t ammotype, int num)
{
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

    if (ammotype == weaponinfo[viewplayer->readyweapon].ammotype)
        ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

    P_CheckAmmo(viewplayer->readyweapon);
    return true;
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
static int P_GiveAmmo(ammotype_t ammotype, int num, dboolean stat)
{
    int oldammo;

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
        num <<= 1;

    oldammo = viewplayer->ammo[ammotype];
    viewplayer->ammo[ammotype] = MIN(oldammo + num, viewplayer->maxammo[ammotype]);

    if (num && ammotype == weaponinfo[viewplayer->readyweapon].ammotype)
        ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

    if (stat)
        P_UpdateAmmoStat(ammotype, viewplayer->ammo[ammotype] - oldammo);

    // If non-zero ammo, don't change up weapons, player was lower on purpose.
    if (oldammo)
        return num;

    // We were down to zero, so select a new weapon.
    // Preferences are not user selectable.
    switch (ammotype)
    {
        case am_clip:
            if (viewplayer->readyweapon == wp_fist)
            {
                if (viewplayer->weaponowned[wp_chaingun])
                    viewplayer->pendingweapon = wp_chaingun;
                else
                    viewplayer->pendingweapon = wp_pistol;
            }

            break;

        case am_shell:
            if (viewplayer->readyweapon == wp_fist || viewplayer->readyweapon == wp_pistol)
            {
                if (viewplayer->weaponowned[wp_supershotgun] && viewplayer->preferredshotgun == wp_supershotgun
                    && viewplayer->ammo[am_shell] >= 2)
                    viewplayer->pendingweapon = wp_supershotgun;
                else if (viewplayer->weaponowned[wp_shotgun])
                    viewplayer->pendingweapon = wp_shotgun;
            }

            break;

        case am_cell:
            if (viewplayer->readyweapon == wp_fist || viewplayer->readyweapon == wp_pistol)
                if (viewplayer->weaponowned[wp_plasma])
                    viewplayer->pendingweapon = wp_plasma;

            break;

        default:
            break;
    }

    return num;
}

//
// P_GiveBackpack
//
dboolean P_GiveBackpack(dboolean giveammo, dboolean stat)
{
    dboolean    result = false;

    if (!viewplayer->backpack)
    {
        for (ammotype_t i = 0; i < NUMAMMO; i++)
            viewplayer->maxammo[i] *= 2;

        viewplayer->backpack = true;
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
dboolean P_GiveFullAmmo(void)
{
    dboolean    result = false;

    for (int i = 0; i < NUMAMMO; i++)
        if (viewplayer->ammo[i] < viewplayer->maxammo[i])
        {
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
static dboolean P_GiveWeapon(weapontype_t weapon, dboolean dropped, dboolean stat)
{
    dboolean    gaveammo = false;
    dboolean    gaveweapon = false;
    ammotype_t  ammotype = weaponinfo[weapon].ammotype;

    if (ammotype != am_noammo)
        // give one clip with a dropped weapon, two clips with a found weapon
        gaveammo = P_GiveAmmo(ammotype, (dropped ? 1 : 2), stat);

    if (!viewplayer->weaponowned[weapon])
    {
        gaveweapon = true;
        viewplayer->weaponowned[weapon] = true;
        viewplayer->pendingweapon = weapon;
    }

    return (gaveweapon || gaveammo);
}

//
// P_GiveAllWeapons
//
dboolean P_GiveAllWeapons(void)
{
    dboolean    result = false;

    if (!viewplayer->weaponowned[wp_chainsaw])
    {
        viewplayer->weaponowned[wp_chainsaw] = true;
        viewplayer->fistorchainsaw = wp_chainsaw;

        if (viewplayer->readyweapon == wp_fist)
            viewplayer->pendingweapon = wp_chainsaw;

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

void P_UpdateHealthStat(int num)
{
    viewplayer->itemspickedup_health += num;
    stat_itemspickedup_health = SafeAdd(stat_itemspickedup_health, num);
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//
dboolean P_GiveBody(int num, dboolean stat)
{
    int oldhealth;

    if (viewplayer->health >= MAXHEALTH)
        return false;

    oldhealth = viewplayer->health;
    viewplayer->health = MIN(oldhealth + num, MAXHEALTH);
    viewplayer->mo->health = viewplayer->health;
    healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;

    if (stat)
        P_UpdateHealthStat(viewplayer->health - oldhealth);

    return true;
}

//
// P_GiveMegaHealth
//
dboolean P_GiveMegaHealth(dboolean stat)
{
    dboolean    result = false;

    if (!(viewplayer->cheats & CF_GODMODE))
    {
        if (viewplayer->health < mega_health)
        {
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;

            if (stat)
                P_UpdateHealthStat(MAX(0, mega_health - viewplayer->health));

            result = true;
        }

        viewplayer->health = mega_health;
        viewplayer->mo->health = mega_health;
    }

    return result;
}

void P_UpdateArmorStat(int num)
{
    viewplayer->itemspickedup_armor += num;
    stat_itemspickedup_armor = SafeAdd(stat_itemspickedup_armor, num);
}

//
// P_GiveArmor
// Returns false if the armor is worse than the current armor.
//
dboolean P_GiveArmor(armortype_t armortype, dboolean stat)
{
    int hits = armortype * 100;

    if (viewplayer->armorpoints >= hits)
        return false;   // don't pick up

    viewplayer->armortype = armortype;

    if (stat)
        P_UpdateArmorStat(hits - viewplayer->armorpoints);

    viewplayer->armorpoints = hits;
    armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
    return true;
}

int cardsfound;

//
// P_InitCards
//
void P_InitCards(void)
{
    for (int i = 0; i < NUMCARDS; i++)
        viewplayer->cards[i] = CARDNOTINMAP;

    cardsfound = 0;

    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        mobj_t  *mo = (mobj_t *)th;

        for (int i = 0; i < NUMCARDS; i++)
            if (mo->sprite == cardsprites[i])
            {
                viewplayer->cards[i] = CARDNOTFOUNDYET;
                break;
            }
    }

    for (int i = 0; i < numlines; i++)
    {
        line_t  *line = lines + i;

        switch (line->special)
        {
            case DR_Door_Blue_OpenWaitClose:
            case D1_Door_Blue_OpenStay:
            case SR_Door_Blue_OpenStay_Fast:
            case S1_Door_Blue_OpenStay_Fast:
                if (viewplayer->cards[it_blueskull] == CARDNOTINMAP)
                    viewplayer->cards[it_bluecard] = CARDNOTFOUNDYET;

                break;

            case DR_Door_Red_OpenWaitClose:
            case D1_Door_Red_OpenStay:
            case SR_Door_Red_OpenStay_Fast:
            case S1_Door_Red_OpenStay_Fast:
                if (viewplayer->cards[it_redskull] == CARDNOTINMAP)
                    viewplayer->cards[it_redcard] = CARDNOTFOUNDYET;

                break;

            case DR_Door_Yellow_OpenWaitClose:
            case D1_Door_Yellow_OpenStay:
            case SR_Door_Yellow_OpenStay_Fast:
            case S1_Door_Yellow_OpenStay_Fast:
                if (viewplayer->cards[it_yellowskull] == CARDNOTINMAP)
                    viewplayer->cards[it_yellowcard] = CARDNOTFOUNDYET;

                break;
        }
    }
}

//
// P_GiveCard
//
static void P_GiveCard(card_t card)
{
    viewplayer->cards[card] = ++cardsfound;

    if (card == viewplayer->neededcard)
    {
        viewplayer->neededcard = 0;
        viewplayer->neededcardflash = 0;
    }
}

//
// P_GiveAllCards
//
dboolean P_GiveAllCards(void)
{
    dboolean    result = false;

    for (int i = 0; i < NUMCARDS; i++)
        if (viewplayer->cards[i] <= 0)
        {
            P_GiveCard(i);
            result = true;
        }

    return result;
}

//
// P_GiveAllKeyCards
//
dboolean P_GiveAllKeyCards(void)
{
    dboolean    result = false;

    if (viewplayer->cards[it_bluecard] <= 0)
    {
        P_GiveCard(it_bluecard);
        result = true;
    }

    if (viewplayer->cards[it_yellowcard] <= 0)
    {
        P_GiveCard(it_yellowcard);
        result = true;
    }

    if (viewplayer->cards[it_redcard] <= 0)
    {
        P_GiveCard(it_redcard);
        result = true;
    }

    return result;
}

//
// P_GiveAllSkullKeys
//
dboolean P_GiveAllSkullKeys(void)
{
    dboolean    result = false;

    if (viewplayer->cards[it_blueskull] <= 0)
    {
        P_GiveCard(it_blueskull);
        result = true;
    }

    if (viewplayer->cards[it_yellowskull] <= 0)
    {
        P_GiveCard(it_yellowskull);
        result = true;
    }

    if (viewplayer->cards[it_redskull] <= 0)
    {
        P_GiveCard(it_redskull);
        result = true;
    }

    return result;
}

//
// P_GiveAllCardsInMap
//
dboolean P_GiveAllCardsInMap(void)
{
    dboolean    skulliscard = true;
    dboolean    result = false;

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
dboolean P_GivePower(int power)
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

    dboolean    given;

    if (viewplayer->powers[power] < 0)
        return false;

    switch (power)
    {
        case pw_invulnerability:
            viewplayer->fixedcolormap = INVERSECOLORMAP;
            break;

        case pw_strength:
            P_GiveBody(100, true);
            break;

        case pw_invisibility:
            viewplayer->mo->flags |= MF_FUZZ;
            break;

        case pw_infrared:
            viewplayer->fixedcolormap = 1;
            break;
    }

    given = (viewplayer->powers[power] <= 0);
    viewplayer->powers[power] = tics[power];
    return given;
}

//
// P_TouchSpecialThing
//
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher, dboolean message, dboolean stat)
{
    fixed_t     delta;
    int         sound = sfx_itemup;
    static int  prevsound;
    static int  prevtic;
    int         temp;

    if (freeze)
        return;

    if ((delta = special->z - toucher->z) > toucher->height || delta < -8 * FRACUNIT)
        return;         // out of reach

    // Dead thing touching.
    // Can happen with a sliding player corpse.
    if (toucher->health <= 0)
        return;

    // Identify by sprite.
    switch (special->sprite)
    {
        // green armor
        case SPR_ARM1:
            if (!P_GiveArmor(green_armor_class, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTARMOR, true, false);

            break;

        // blue armor
        case SPR_ARM2:
            if (!P_GiveArmor(blue_armor_class, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTMEGA, true, false);

            break;

        // bonus health
        case SPR_BON1:
            if (viewplayer->health < maxhealth && !(viewplayer->cheats & CF_GODMODE))
            {
                viewplayer->health++;
                viewplayer->mo->health = viewplayer->health;
                P_UpdateHealthStat(1);
                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            }

            if (message)
                HU_PlayerMessage(s_GOTHTHBONUS, true, false);

            break;

        // bonus armor
        case SPR_BON2:
            if (viewplayer->armorpoints < max_armor)
            {
                viewplayer->armorpoints++;
                P_UpdateArmorStat(1);
                armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;

                if (!viewplayer->armortype)
                    viewplayer->armortype = armortype_green;
            }

            if (message)
                HU_PlayerMessage(s_GOTARMBONUS, true, false);

            break;

        // soulsphere
        case SPR_SOUL:
            if (!(viewplayer->cheats & CF_GODMODE))
            {
                P_UpdateHealthStat(MAX(0, soul_health - viewplayer->health));
                viewplayer->health = MIN(viewplayer->health + soul_health, max_soul);
                viewplayer->mo->health = viewplayer->health;
                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            }

            if (message)
                HU_PlayerMessage(s_GOTSUPER, true, false);

            sound = sfx_getpow;
            break;

        // mega health
        case SPR_MEGA:
            P_GiveMegaHealth(stat);
            P_GiveArmor(blue_armor_class, stat);

            if (message)
                HU_PlayerMessage(s_GOTMSPHERE, true, false);

            sound = sfx_getpow;
            break;

        // blue keycard
        case SPR_BKEY:
            if (viewplayer->cards[it_bluecard] <= 0)
            {
                P_GiveCard(it_bluecard);

                if (message)
                    HU_PlayerMessage(s_GOTBLUECARD, true, false);

                break;
            }
            else
                return;

        // yellow keycard
        case SPR_YKEY:
            if (viewplayer->cards[it_yellowcard] <= 0)
            {
                P_GiveCard(it_yellowcard);

                if (message)
                    HU_PlayerMessage(s_GOTYELWCARD, true, false);

                break;
            }
            else
                return;

        // red keycard
        case SPR_RKEY:
            if (viewplayer->cards[it_redcard] <= 0)
            {
                P_GiveCard(it_redcard);

                if (message)
                    HU_PlayerMessage(s_GOTREDCARD, true, false);

                break;
            }
            else
                return;

        // blue skull key
        case SPR_BSKU:
            if (viewplayer->cards[it_blueskull] <= 0)
            {
                P_GiveCard(it_blueskull);

                if (message)
                    HU_PlayerMessage(s_GOTBLUESKUL, true, false);

                break;
            }
            else
                return;

        // yellow skull key
        case SPR_YSKU:
            if (viewplayer->cards[it_yellowskull] <= 0)
            {
                P_GiveCard(it_yellowskull);

                if (message)
                    HU_PlayerMessage(s_GOTYELWSKUL, true, false);

                break;
            }
            else
                return;

        // red skull key
        case SPR_RSKU:
            if (viewplayer->cards[it_redskull] <= 0)
            {
                P_GiveCard(it_redskull);

                if (message)
                    HU_PlayerMessage(s_GOTREDSKULL, true, false);

                break;
            }
            else
                return;

        // stimpack
        case SPR_STIM:
            if (!P_GiveBody(10, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTSTIM, true, false);

            break;

        // medikit
        case SPR_MEDI:
            if (!P_GiveBody(25, stat))
                return;

            if (message)
            {
                if (viewplayer->health < 50)
                {
                    static char buffer[1024];

                    M_snprintf(buffer, sizeof(buffer), s_GOTMEDINEED, playername,
                        (M_StringCompare(playername, playername_default) ? "you" : "they"));
                    HU_PlayerMessage(buffer, true, false);
                }
                else
                    HU_PlayerMessage((viewplayer->health < 50 ? s_GOTMEDINEED : s_GOTMEDIKIT), true, false);
            }

            break;

        // invulnerability power-up
        case SPR_PINV:
            P_GivePower(pw_invulnerability);

            if (message)
                HU_PlayerMessage(s_GOTINVUL, true, false);

            sound = sfx_getpow;
            break;

        // berserk power-up
        case SPR_PSTR:
        {
            dboolean    strength = viewplayer->powers[pw_strength];

            P_GivePower(pw_strength);

            if (message)
                HU_PlayerMessage(s_GOTBERSERK, true, false);

            if (viewplayer->readyweapon != wp_fist && !strength)
                viewplayer->pendingweapon = wp_fist;

            viewplayer->fistorchainsaw = wp_fist;
            sound = sfx_getpow;
            break;
        }

        // partial invisibility power-up
        case SPR_PINS:
            P_GivePower(pw_invisibility);

            if (message)
                HU_PlayerMessage(s_GOTINVIS, true, false);

            sound = sfx_getpow;
            break;

        // radiation shielding suit power-up
        case SPR_SUIT:
            P_GivePower(pw_ironfeet);

            if (message)
                HU_PlayerMessage(s_GOTSUIT, true, false);

            sound = sfx_getpow;
            break;

        // computer area map power-up
        case SPR_PMAP:
            P_GivePower(pw_allmap);

            if (message)
                HU_PlayerMessage(s_GOTMAP, true, false);

            sound = sfx_getpow;
            break;

        // light amplification visor power-up
        case SPR_PVIS:
            P_GivePower(pw_infrared);

            if (message)
                HU_PlayerMessage(s_GOTVISOR, true, false);

            sound = sfx_getpow;
            break;

        // clip
        case SPR_CLIP:
            if (!P_GiveAmmo(am_clip, !(special->flags & MF_DROPPED), stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCLIP, true, false);

            break;

        // box of bullets
        case SPR_AMMO:
            if (!P_GiveAmmo(am_clip, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCLIPBOX, true, false);

            break;

        // rocket
        case SPR_ROCK:
            if (!(temp = P_GiveAmmo(am_misl, 1, stat)))
                return;

            if (message)
            {
                if (temp == clipammo[am_misl] || deh_strlookup[p_GOTROCKET].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTROCKET, true, false);
                else
                    HU_PlayerMessage(s_GOTROCKETX2, true, false);
            }

            break;

        // box of rockets
        case SPR_BROK:
            if (!P_GiveAmmo(am_misl, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTROCKBOX, true, false);

            break;

        // cell
        case SPR_CELL:
            if (!(temp = P_GiveAmmo(am_cell, 1, stat)))
                return;

            if (message)
            {
                if (temp == clipammo[am_cell] || deh_strlookup[p_GOTCELL].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTCELL, true, false);
                else
                    HU_PlayerMessage(s_GOTCELLX2, true, false);
            }

            break;

        // cell pack
        case SPR_CELP:
            if (!P_GiveAmmo(am_cell, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCELLBOX, true, false);

            break;

        // shells
        case SPR_SHEL:
            if (!(temp = P_GiveAmmo(am_shell, 1, stat)))
                return;

            if (message)
            {
                if (temp == clipammo[am_shell] || deh_strlookup[p_GOTSHELLS].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTSHELLS, true, false);
                else
                    HU_PlayerMessage(s_GOTSHELLSX2, true, false);
            }

            break;

        // box of shells
        case SPR_SBOX:
            if (!P_GiveAmmo(am_shell, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTSHELLBOX, true, false);

            break;

        // backpack
        case SPR_BPAK:
            if (!P_GiveBackpack(true, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTBACKPACK, true, false);

            break;

        // BFG-9000
        case SPR_BFUG:
            if (!P_GiveWeapon(wp_bfg, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTBFG9000, true, false);

            sound = sfx_wpnup;
            break;

        // chaingun
        case SPR_MGUN:
            if (!P_GiveWeapon(wp_chaingun, (special->flags & MF_DROPPED), stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCHAINGUN, true, false);

            sound = sfx_wpnup;
            break;

        // chainsaw
        case SPR_CSAW:
            if (!P_GiveWeapon(wp_chainsaw, false, stat))
                return;

            viewplayer->fistorchainsaw = wp_chainsaw;

            if (message)
                HU_PlayerMessage(s_GOTCHAINSAW, true, false);

            sound = sfx_wpnup;
            break;

        // rocket launcher
        case SPR_LAUN:
            if (!P_GiveWeapon(wp_missile, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTLAUNCHER, true, false);

            sound = sfx_wpnup;
            break;

        // plasma rifle
        case SPR_PLAS:
            if (!P_GiveWeapon(wp_plasma, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTPLASMA, true, false);

            sound = sfx_wpnup;
            break;

        // shotgun
        case SPR_SHOT:
            temp = viewplayer->weaponowned[wp_shotgun];

            if (!P_GiveWeapon(wp_shotgun, (special->flags & MF_DROPPED), stat))
                return;

            if (!temp)
                viewplayer->preferredshotgun = wp_shotgun;

            if (message)
                HU_PlayerMessage(s_GOTSHOTGUN, true, false);

            sound = sfx_wpnup;
            break;

        // super shotgun
        case SPR_SGN2:
            temp = viewplayer->weaponowned[wp_supershotgun];

            if (!P_GiveWeapon(wp_supershotgun, (special->flags & MF_DROPPED), stat))
                return;

            if (!temp)
                viewplayer->preferredshotgun = wp_supershotgun;

            if (message)
                HU_PlayerMessage(s_GOTSHOTGUN2, true, false);

            sound = sfx_wpnup;
            break;

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
        prevtic = gametime;
        S_StartSound(viewplayer->mo, sound);
    }

    P_RemoveMobj(special);
    P_AddBonus();
}

//
// P_TakeSpecialThing
//
dboolean P_TakeSpecialThing(mobjtype_t type)
{
    switch (type)
    {
        // green armor
        case MT_MISC0:
            if (viewplayer->armortype != armortype_green)
                return false;

            if (viewplayer->armorpoints < green_armor_class * 100)
                return false;

            viewplayer->armorpoints -= green_armor_class * 100;
            armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            return true;

        // blue armor
        case MT_MISC1:
            if (viewplayer->armortype != armortype_blue)
                return false;

            if (viewplayer->armorpoints < blue_armor_class * 100)
                return false;

            viewplayer->armorpoints -= blue_armor_class * 100;
            armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            return true;

        // bonus health
        case MT_MISC2:
            if ((viewplayer->cheats & CF_GODMODE))
                return false;

            if (viewplayer->powers[pw_invulnerability])
                return false;

            if ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health == 1)
                return false;

            if (viewplayer->health <= 0)
                return false;

            viewplayer->health--;
            viewplayer->mo->health--;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            return true;

        // bonus armor
        case MT_MISC3:
            if (!viewplayer->armorpoints)
                return false;

            viewplayer->armorpoints--;
            armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            return true;

        // soulsphere
        case MT_MISC12:
            if ((viewplayer->cheats & CF_GODMODE))
                return false;

            if (viewplayer->powers[pw_invulnerability])
                return false;

            if ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health <= soul_health)
                return false;

            if (viewplayer->health < soul_health)
                return false;

            viewplayer->health -= soul_health;
            viewplayer->mo->health -= soul_health;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            return true;

        // mega health
        case MT_MEGA:
            if ((viewplayer->cheats & CF_GODMODE))
                return false;

            if (viewplayer->powers[pw_invulnerability])
                return false;

            if ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health <= mega_health)
                return false;

            if (viewplayer->health < mega_health)
                return false;

            viewplayer->health -= mega_health;
            viewplayer->mo->health -= mega_health;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            return true;

        // blue keycard
        case MT_MISC4:
            if (viewplayer->cards[it_bluecard] <= 0)
                return false;

            viewplayer->cards[it_bluecard] = 0;
            cardsfound--;
            return true;

        // yellow keycard
        case MT_MISC6:
            if (viewplayer->cards[it_yellowcard] <= 0)
                return false;

            viewplayer->cards[it_yellowcard] = 0;
            cardsfound--;
            return true;

        // red keycard
        case MT_MISC5:
            if (viewplayer->cards[it_redcard] <= 0)
                return false;

            viewplayer->cards[it_redcard] = 0;
            cardsfound--;
            return true;

        // blue skull key
        case MT_MISC9:
            if (viewplayer->cards[it_blueskull] <= 0)
                return false;

            viewplayer->cards[it_blueskull] = 0;
            cardsfound--;
            return true;

        // yellow skull key
        case MT_MISC7:
            if (viewplayer->cards[it_yellowskull] <= 0)
                return false;

            viewplayer->cards[it_yellowskull] = 0;
            cardsfound--;
            return true;

        // red skull key
        case MT_MISC8:
            if (viewplayer->cards[it_redskull] <= 0)
                return false;

            viewplayer->cards[it_redskull] = 0;
            cardsfound--;
            return true;

        // stimpack
        case MT_MISC10:
            if ((viewplayer->cheats & CF_GODMODE))
                return false;

            if (viewplayer->powers[pw_invulnerability])
                return false;

            if ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health <= 10)
                return false;

            if (viewplayer->health < 10)
                return false;

            viewplayer->health -= 10;
            viewplayer->mo->health -= 10;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            return true;

        // medikit
        case MT_MISC11:
            if ((viewplayer->cheats & CF_GODMODE))
                return false;

            if (viewplayer->powers[pw_invulnerability])
                return false;

            if ((viewplayer->cheats & CF_BUDDHA) && viewplayer->health <= 25)
                return false;

            if (viewplayer->health < 25)
                return false;

            viewplayer->health -= 25;
            viewplayer->mo->health -= 25;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            return true;

        // invulnerability power-up
        case MT_INV:
            if (!viewplayer->powers[pw_invulnerability])
                return false;

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

            viewplayer->powers[pw_invisibility] = STARTFLASHING;
            return true;

        // radiation shielding suit power-up
        case MT_MISC14:
            if (!viewplayer->powers[pw_ironfeet])
                return false;

            viewplayer->powers[pw_ironfeet] = STARTFLASHING;
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

            viewplayer->powers[pw_infrared] = STARTFLASHING;
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

        // shells
        case MT_MISC22:
            return P_TakeAmmo(am_shell, 1);

        // box of shells
        case MT_MISC23:
            return P_TakeAmmo(am_shell, 5);

        // backpack
        case MT_MISC24:
            if (!viewplayer->backpack)
                return false;

            for (ammotype_t i = 0; i < NUMAMMO; i++)
            {
                viewplayer->maxammo[i] /= 2;

                P_TakeAmmo(i, 1);

                if (viewplayer->ammo[i] > viewplayer->maxammo[i])
                    viewplayer->ammo[i] = viewplayer->maxammo[i];
            }

            viewplayer->backpack = false;
            return true;

        // BFG-9000
        case MT_MISC25:
            if (!viewplayer->weaponowned[wp_bfg])
                return false;

            viewplayer->weaponowned[wp_bfg] = oldweaponsowned[wp_bfg] = false;
            P_CheckAmmo(viewplayer->readyweapon);
            return true;

        // chaingun
        case MT_CHAINGUN:
            if (!viewplayer->weaponowned[wp_chaingun])
                return false;

            viewplayer->weaponowned[wp_chaingun] = oldweaponsowned[wp_chaingun] = false;
            P_CheckAmmo(viewplayer->readyweapon);
            return true;

        // chainsaw
        case MT_MISC26:
            if (!viewplayer->weaponowned[wp_chainsaw])
                return false;

            viewplayer->weaponowned[wp_chainsaw] = oldweaponsowned[wp_chainsaw] = false;
            P_CheckAmmo(viewplayer->readyweapon);
            return true;

        // rocket launcher
        case MT_MISC27:
            if (!viewplayer->weaponowned[wp_missile])
                return false;

            viewplayer->weaponowned[wp_missile] = oldweaponsowned[wp_missile] = false;
            P_CheckAmmo(viewplayer->readyweapon);
            return true;

        // plasma rifle
        case MT_MISC28:
            if (!viewplayer->weaponowned[wp_plasma])
                return false;

            viewplayer->weaponowned[wp_plasma] = oldweaponsowned[wp_plasma] = false;
            P_CheckAmmo(viewplayer->readyweapon);
            return true;

        // shotgun
        case MT_SHOTGUN:
            if (!viewplayer->weaponowned[wp_shotgun])
                return false;

            viewplayer->weaponowned[wp_shotgun] = oldweaponsowned[wp_shotgun] = false;
            P_CheckAmmo(viewplayer->readyweapon);
            return true;

        // super shotgun
        case MT_SUPERSHOTGUN:
            if (!viewplayer->weaponowned[wp_supershotgun])
                return false;

            viewplayer->weaponowned[wp_supershotgun] = oldweaponsowned[wp_supershotgun] = false;
            P_CheckAmmo(viewplayer->readyweapon);
            return true;

        default:
            return false;
    }
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

static void P_WriteObituary(mobj_t *target, mobj_t *inflicter, mobj_t *source, dboolean gibbed)
{
    if (source)
    {
        if (inflicter && inflicter->type == MT_BARREL && target->type != MT_BARREL)
        {
            if (target->player)
                C_Obituary("%s %s %s by an exploding %s.",
                    sentencecase(playername),
                    (M_StringCompare(playername, playername_default) ? "were" : "was"),
                    (gibbed ? "gibbed" : "killed"),
                    inflicter->info->name1);
            else
            {
                char    targetname[100];

                if (*target->name)
                    M_StringCopy(targetname, target->name, sizeof(targetname));
                else
                    M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                        ((target->flags & MF_FRIEND) && monstercount[target->type] == 1 ? "the" :
                            (isvowel(target->info->name1[0]) ? "an" : "a")),
                        ((target->flags & MF_FRIEND) ? "friendly " : ""),
                        (*target->info->name1 ? target->info->name1 : "monster"));

                C_Obituary("%s was %s by an exploding %s.",
                    sentencecase(targetname),
                    (gibbed ? "gibbed" : "killed"),
                    inflicter->info->name1);
            }
        }
        else if (source->player)
        {
            if (source->player->mo == source)
            {
                weapontype_t    readyweapon = viewplayer->readyweapon;

                if (M_StringCompare(playername, playername_default))
                {
                    if (target->player)
                        C_Obituary("You %s yourself with your own %s.",
                            (gibbed ? "gibbed" : "killed"),
                            weaponinfo[readyweapon].description);
                    else
                    {
                        char    targetname[100];

                        if (*target->name)
                            M_StringCopy(targetname, target->name, sizeof(targetname));
                        else
                            M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                                ((target->flags & MF_FRIEND) && monstercount[target->type] == 1 ? "the" :
                                    (isvowel(target->info->name1[0]) ? "an" : "a")),
                                ((target->flags & MF_FRIEND) ? "friendly " : ""),
                                (*target->info->name1 ? target->info->name1 : "monster"));

                        C_Obituary("You %s %s with your %s%s.",
                            (target->type == MT_BARREL ? "exploded" : (gibbed ? "gibbed" : "killed")),
                            targetname,
                            weaponinfo[readyweapon].description,
                            (readyweapon == wp_fist && viewplayer->powers[pw_strength] ? " while you went berserk" : ""));
                    }
                }
                else
                {
                    if (target->player)
                        C_Obituary("%s %s themselves with their own %s.",
                            titlecase(playername),
                            (gibbed ? "gibbed" : "killed"),
                            weaponinfo[readyweapon].description);
                    else
                    {
                        char    targetname[100];

                        if (*target->name)
                            M_StringCopy(targetname, target->name, sizeof(targetname));
                        else
                            M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                                ((target->flags & MF_FRIEND) && monstercount[target->type] == 1 ? "the" :
                                    (isvowel(target->info->name1[0]) ? "an" : "a")),
                                ((target->flags & MF_FRIEND) ? "friendly " : ""),
                                (*target->info->name1 ? target->info->name1 : "monster"));

                        C_Obituary("%s %s %s with their %s%s.",
                            titlecase(playername),
                            (target->type == MT_BARREL ? "exploded" : (gibbed ? "gibbed" :
                                (M_StringCompare(targetname, "\x44\x6F\x6E\x61\x6C\x64\x20\x54\x72\x75\x6D\x70") ?
                                "\x69\x6D\x70\x65\x61\x63\x68\x65\x64" : "killed"))),
                            targetname,
                            weaponinfo[readyweapon].description,
                            (readyweapon == wp_fist && viewplayer->powers[pw_strength] ? " while they went berserk" : ""));
                    }
                }
            }
        }
        else
        {
            if (source->type == MT_TFOG)
            {
                if (target->player)
                    C_Obituary("%s %s telefragged.",
                        titlecase(playername),
                        (M_StringCompare(playername, playername_default) ? "were" : "was"));
                else
                {
                    char    targetname[100];

                    if (*target->name)
                        M_StringCopy(targetname, target->name, sizeof(targetname));
                    else
                        M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                            ((target->flags &MF_FRIEND) && monstercount[target->type] == 1 ? "the" :
                                (isvowel(target->info->name1[0]) ? "an" : "a")),
                            ((target->flags & MF_FRIEND) ? "friendly " : ""),
                            (*target->info->name1 ? target->info->name1 : "monster"));

                    C_Obituary("%s was telefragged.", targetname);
                }
            }
            else
            {
                char    sourcename[100];

                if (*source->name)
                    M_StringCopy(sourcename, source->name, sizeof(sourcename));
                else
                    M_snprintf(sourcename, sizeof(sourcename), "%s %s%s",
                        ((source->flags &MF_FRIEND) && monstercount[source->type] == 1 ? "the" :
                            (isvowel(source->info->name1[0]) ? "an" : "a")),
                        ((source->flags & MF_FRIEND) ? "friendly " : ""),
                        (*source->info->name1 ? source->info->name1 : "monster"));

                if (target->player)
                    C_Obituary("%s %s %s.",
                        sentencecase(sourcename),
                        (gibbed ? "gibbed" : "killed"),
                        (M_StringCompare(playername, playername_default) ? playername : titlecase(playername)));
                else
                {
                    char    targetname[100];

                    if (*target->name)
                        M_StringCopy(targetname, target->name, sizeof(targetname));
                    else
                        M_snprintf(targetname, sizeof(targetname), "%s %s%s",
                            (source->type == target->type ? "another" :
                                ((target->flags & MF_FRIEND) && monstercount[target->type] == 1 ? "the" :
                                (isvowel(target->info->name1[0]) ? "an" : "a"))),
                            ((target->flags & MF_FRIEND) ? "friendly " : ""),
                            (*target->info->name1 ? target->info->name1 : "monster"));

                    C_Obituary("%s %s %s.",
                        sentencecase(sourcename),
                        (target->type == MT_BARREL ? "exploded" : (gibbed ? "gibbed" : "killed")),
                        targetname);
                }
            }
        }
    }
    else if (target->player && target->player->mo == target)
    {
        sector_t    *sector = viewplayer->mo->subsector->sector;

        if (sector->ceilingdata && sector->ceilingheight - sector->floorheight < VIEWHEIGHT)
            C_Obituary("%s %s crushed to death.",
                titlecase(playername),
                (M_StringCompare(playername, playername_default) ? "were" : "was"));
        else
        {
            if (sector->terraintype != SOLID)
            {
                char *liquids[] =
                {
                    "",      "liquid",     "nukage", "water",     "lava", "blood",
                    "slime", "gray slime", "goop",   "icy water", "tar",  "sludge"
                };

                C_Obituary("%s died in %s.", titlecase(playername), liquids[sector->terraintype]);
            }
            else
            {
                short   floorpic = sector->floorpic;

                if ((floorpic >= RROCK05 && floorpic <= RROCK08) || (floorpic >= SLIME09 && floorpic <= SLIME12))
                    C_Obituary("%s died on molten rock.", titlecase(playername));
                else if (healthcvar)
                    C_Obituary("%s killed %s.",
                        titlecase(playername),
                        (M_StringCompare(playername, playername_default) ? "yourself" : "themselves"));
                else
                    C_Obituary("%s blew %s up.",
                        titlecase(playername),
                        (M_StringCompare(playername, playername_default) ? "yourself" : "themselves"));
            }
        }
    }
}

//
// P_KillMobj
//
void P_KillMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source)
{
    dboolean    gibbed;
    dboolean    massacre = (target->flags2 & MF2_MASSACRE);
    mobjtype_t  type = target->type;
    mobjinfo_t  *info = &mobjinfo[type];
    int         gibhealth = info->gibhealth;

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
    target->flags2 &= ~MF2_PASSMOBJ;
    target->height >>= 2;

    // killough 8/29/98: remove from threaded list
    P_UpdateThinker(&target->thinker);

    if (type != MT_BARREL)
    {
        if (!(target->flags & MF_FUZZ))
            target->bloodsplats = CORPSEBLOODSPLATS;

        if (r_corpses_mirrored && type != MT_CHAINGUY && type != MT_CYBORG && (type != MT_PAIN || !doom4vanilla) && (M_Random() & 1))
            target->flags2 |= MF2_MIRRORED;
    }

    if (target->flags & MF_COUNTKILL)
    {
        // count all monster deaths, even those caused by other monsters
        viewplayer->killcount++;

        if ((source && source->player) || massacre)
        {
            stat_monsterskilled = SafeAdd(stat_monsterskilled, 1);

            if (!chex && !hacx)
            {
                viewplayer->mobjcount[type]++;
                P_UpdateKillStat(type, 1);
            }
        }
    }
    else if (type == MT_BARREL && !chex && !hacx)
    {
        viewplayer->mobjcount[type]++;
        stat_barrelsexploded = SafeAdd(stat_barrelsexploded, 1);
    }

    if (type == MT_BARREL && source)
        P_SetTarget(&target->target, source);

    if (target->player)
    {
        target->flags &= ~MF_SOLID;
        viewplayer->playerstate = PST_DEAD;
        P_DropWeapon();

        if (automapactive)
            AM_Stop();          // don't die in automap, switch view prior to dying

        viewplayer->deaths++;
        stat_deaths = SafeAdd(stat_deaths, 1);
    }
    else
        target->flags2 &= ~MF2_NOLIQUIDBOB;

    if ((gibbed = (gibhealth < 0 && target->health < gibhealth && info->xdeathstate && !(source && source->type == MT_DOGS))))
        P_SetMobjState(target, info->xdeathstate);
    else
        P_SetMobjState(target, info->deathstate);

    if (!target->player)
        target->health = -1;

    target->tics = MAX(1, target->tics - (M_Random() & 3));

    if (type == MT_BARREL || (type == MT_PAIN && !doom4vanilla) || type == MT_SKULL)
        target->flags2 &= ~MF2_CASTSHADOW;

    if (chex)
        return;

    if (con_obituaries && !hacx && !massacre)
        P_WriteObituary(target, inflicter, source, gibbed);

    // Drop stuff.
    // This determines the kind of object spawned during the death frame of a thing.
    if (info->droppeditem)
    {
        mobj_t  *mo;

        if (tossdrop)
        {
            mo = P_SpawnMobj(target->x, target->y, target->floorz + target->height * 3 / 2 - 3 * FRACUNIT, info->droppeditem);
            mo->momx = (target->momx >> 1) + (M_SubRandom() << 8);
            mo->momy = (target->momy >> 1) + (M_SubRandom() << 8);
            mo->momz = FRACUNIT * 2 + (M_Random() << 9);
        }
        else
            mo = P_SpawnMobj(target->x, target->y, ONFLOORZ, info->droppeditem);

        mo->angle = target->angle + (M_SubRandom() << 20);
        mo->flags |= MF_DROPPED;    // special versions of items

        if (r_mirroredweapons && (M_Random() & 1))
            mo->flags2 |= MF2_MIRRORED;
    }
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
void P_DamageMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source, int damage, dboolean adjust)
{
    player_t    *splayer = NULL;
    player_t    *tplayer;
    int         flags = target->flags;
    dboolean    corpse = flags & MF_CORPSE;
    int         type = target->type;
    mobjinfo_t  *info = &mobjinfo[type];
    dboolean    justhit = false;

    if (!(flags & MF_SHOOTABLE) && (!corpse || !r_corpses_slide))
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
    if (inflicter && (!tplayer || !inflicter->player) && !(flags & MF_NOCLIP)
        && (!source || !splayer || splayer->readyweapon != wp_chainsaw))
    {
        unsigned int    ang = R_PointToAngle2(inflicter->x, inflicter->y, target->x, target->y);
        int             mass = (corpse ? MAX(200, info->mass) : info->mass);
        fixed_t         thrust = damage * (FRACUNIT >> 3) * 100 / mass;

        // make fall forwards sometimes
        if (damage < 40 && damage > target->health && target->z - inflicter->z > 64 * FRACUNIT && (M_Random() & 1))
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
        if (target->subsector->sector->special == DamageNegative10Or20PercentHealthAndEndLevel && damage >= target->health)
            damage = target->health - 1;

        // below certain threshold, ignore damage in god mode, or with invulnerability power-up
        if ((tplayer->cheats & CF_GODMODE) || idclevtics || (damage < 1000 && tplayer->powers[pw_invulnerability]))
            return;

        if (adjust && tplayer->armorpoints)
        {
            int saved = damage / (tplayer->armortype == armortype_green ? 3 : 2);

            if (tplayer->armorpoints <= saved)
            {
                // armor is used up
                saved = tplayer->armorpoints;
                tplayer->armortype = armortype_none;
            }

            tplayer->armorpoints -= saved;
            damage -= saved;

            if (saved)
                armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
        }

        tplayer->health -= damage;
        target->health -= damage;
        healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;

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

        if (tplayer->mo == target)
            tplayer->attacker = source;

        damagecount = tplayer->damagecount + damage;            // add damage after armor/invuln

        if (damage > 0 && damagecount < 8)
             damagecount = 8;

        tplayer->damagecount = MIN(damagecount, 100);

        if (gp_vibrate_damage)
        {
            I_GamepadVibration((30000 + (100 - MIN(tplayer->health, 100)) / 100 * 30000) * gp_vibrate_damage / 100);
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

            if (!(flags & MF_FUZZ))
            {
                if (type == MT_BARREL || (type == MT_PAIN && !doom4vanilla) || type == MT_SKULL)
                    target->colfunc = tlredcolfunc;
                else if (type == MT_BRUISER || (type == MT_KNIGHT && !doom4vanilla))
                    target->colfunc = redtogreencolfunc;
            }

            // [crispy] the lethal pellet of a point-blank SSG blast
            // gets an extra damage boost for the occasional gib chance
            if (splayer && splayer->readyweapon == wp_supershotgun && info->xdeathstate
                && P_CheckMeleeRange(target) && damage >= 10 && gibhealth < 0)
                target->health = gibhealth - 1;

            P_KillMobj(target, inflicter, source);
            return;
        }
    }

    if (M_Random() < info->painchance && !(target->flags & MF_SKULLFLY))
    {
        justhit = true;
        P_SetMobjState(target, info->painstate);
    }

    target->reactiontime = 0;                                   // we're awake now...

    if ((!target->threshold || type == MT_VILE) && source && source != target && source->type != MT_VILE)
    {
        // if not intent on another player, chase after this one
        if (!target->lastenemy || target->lastenemy->health <= 0 || !target->lastenemy->player)
            P_SetTarget(&target->lastenemy, target->target);    // remember last enemy - killough

        P_SetTarget(&target->target, source);                   // killough 11/98
        target->threshold = BASETHRESHOLD;

        if (target->state == &states[info->spawnstate] && info->seestate != S_NULL)
            P_SetMobjState(target, info->seestate);
    }

    if (justhit && (target->target == source || !target->target || !(target->flags & target->target->flags & MF_FRIEND)))
        target->flags |= MF_JUSTHIT;                            // fight back!
}

//
// P_ResurrectMobj
//
void P_ResurrectMobj(mobj_t *target)
{
    mobjinfo_t  *info = target->info;

    S_StartSound(target, sfx_slop);
    P_SetMobjState(target, info->raisestate);

    target->height = info->height;
    target->radius = info->radius;
    target->flags = info->flags | (target->flags & MF_FRIEND);
    target->flags2 = info->flags2;
    target->health = info->spawnhealth;
    target->shadowoffset = info->shadowoffset;
    P_SetTarget(&target->target, NULL);

    P_SetTarget(&target->lastenemy, NULL);
    target->flags &= ~MF_JUSTHIT;

    viewplayer->killcount--;
    stat_monsterskilled--;
    P_UpdateKillStat(target->type, -1);
    P_UpdateThinker(&target->thinker);
}
