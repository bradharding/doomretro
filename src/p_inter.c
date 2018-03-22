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
#include "p_local.h"
#include "p_inter.h"
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
int             green_armor_class = GREENARMOR;
int             blue_armor_class = BLUEARMOR;
int             max_soul = 200;
int             soul_health = 100;
int             mega_health = 200;
int             god_health = 100;
int             idfa_armor = 200;
int             idfa_armor_class = BLUEARMOR;
int             idkfa_armor = 200;
int             idkfa_armor_class = BLUEARMOR;
int             bfgcells = BFGCELLS;
dboolean        species_infighting;

// a weapon is found with two clip loads,
// a big item has five clip loads
int             maxammo[NUMAMMO] = { 200, 50, 300, 50 };
int             clipammo[NUMAMMO] = { 10, 4, 20, 1 };

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

extern int      idclevtics;

static void P_UpdateAmmoStat(ammotype_t ammotype, int num)
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

    if (vid_widescreen && r_hud && !r_althud && num && ammotype == weaponinfo[viewplayer->readyweapon].ammotype)
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
        {
            result = true;

            if (vid_widescreen && r_hud && !r_althud && i == weaponinfo[viewplayer->readyweapon].ammotype)
                ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
        }

        if (giveammo)
            P_GiveAmmo(i, 1, stat);
    }

    return result;
}

//
// P_GiveFullAmmo
//
dboolean P_GiveFullAmmo(dboolean stat)
{
    dboolean    result = false;

    for (int i = 0; i < NUMAMMO; i++)
        if (viewplayer->ammo[i] < viewplayer->maxammo[i])
        {
            if (stat)
                P_UpdateAmmoStat(i, viewplayer->maxammo[i] - viewplayer->ammo[i]);

            viewplayer->ammo[i] = viewplayer->maxammo[i];
            result = true;
        }

    if (result)
    {
        if (vid_widescreen && r_hud && !r_althud)
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
        gaveammo = !!P_GiveAmmo(ammotype, (dropped ? 1 : 2), stat);

    if (!viewplayer->weaponowned[weapon])
    {
        gaveweapon = true;
        viewplayer->weaponowned[weapon] = true;
        viewplayer->pendingweapon = weapon;
    }

    if (gaveammo && ammotype == weaponinfo[viewplayer->readyweapon].ammotype)
    {
        if (vid_widescreen && r_hud && !r_althud)
            ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

        return true;
    }

    return (gaveweapon || gaveammo);
}

//
// P_GiveAllWeapons
//
dboolean P_GiveAllWeapons(void)
{
    dboolean    result = false;

    if (!viewplayer->weaponowned[wp_shotgun])
    {
        viewplayer->weaponowned[wp_shotgun] = true;
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

    if (!viewplayer->weaponowned[wp_chainsaw])
    {
        viewplayer->weaponowned[wp_chainsaw] = true;
        viewplayer->fistorchainsaw = wp_chainsaw;

        if (viewplayer->readyweapon == wp_fist)
            viewplayer->pendingweapon = wp_chainsaw;

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

    return result;
}

static void P_UpdateHealthStat(int num)
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
                P_UpdateHealthStat(mega_health - viewplayer->health);
        }

        if (viewplayer->health < mega_health)
            result = true;

        viewplayer->health = mega_health;
        viewplayer->mo->health = mega_health;
    }

    return result;
}

static void P_UpdateArmorStat(int num)
{
    viewplayer->itemspickedup_armor += num;
    stat_itemspickedup_armor = SafeAdd(stat_itemspickedup_armor, num);
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
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

    for (int i = 0; i < numsectors; i++)
    {
        mobj_t  *thing = sectors[i].thinglist;

        while (thing)
        {
            switch (thing->sprite)
            {
                case SPR_BKEY:
                    viewplayer->cards[it_bluecard] = CARDNOTFOUNDYET;
                    break;

                case SPR_RKEY:
                    viewplayer->cards[it_redcard] = CARDNOTFOUNDYET;
                    break;

                case SPR_YKEY:
                    viewplayer->cards[it_yellowcard] = CARDNOTFOUNDYET;
                    break;

                case SPR_BSKU:
                    viewplayer->cards[it_blueskull] = CARDNOTFOUNDYET;
                    break;

                case SPR_RSKU:
                    viewplayer->cards[it_redskull] = CARDNOTFOUNDYET;
                    break;

                case SPR_YSKU:
                    viewplayer->cards[it_yellowskull] = CARDNOTFOUNDYET;
                    break;

                default:
                    break;
            }

            thing = thing->snext;
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
    static const int tics[NUMPOWERS] =
    {
        /* pw_invulnerability */ INVULNTICS,
        /* pw_strength        */ 1,
        /* pw_invisibility    */ INVISTICS,
        /* pw_ironfeet        */ IRONTICS,
        /* pw_allmap          */ STARTFLASHING + 1,
        /* pw_infrared        */ INFRATICS
   };

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

    viewplayer->powers[power] = tics[power];
    return true;
}

//
// P_TouchSpecialThing
//
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher, dboolean message, dboolean stat)
{
    fixed_t     delta;
    int         sound = sfx_itemup;
    int         weaponowned;
    int         ammo;
    static int  prevsound;
    static int  prevtic;

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
                HU_PlayerMessage(s_GOTARMOR, false);

            break;

        // blue armor
        case SPR_ARM2:
            if (!P_GiveArmor(blue_armor_class, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTMEGA, false);

            break;

        // bonus health
        case SPR_BON1:
            if (!(viewplayer->cheats & CF_GODMODE))
            {
                viewplayer->health++;       // can go over 100%

                if (viewplayer->health > maxhealth)
                    viewplayer->health = maxhealth;
                else
                {
                    P_UpdateHealthStat(1);
                    healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
                }

                viewplayer->mo->health = viewplayer->health;
            }

            if (message)
                HU_PlayerMessage(s_GOTHTHBONUS, false);

            break;

        // bonus armor
        case SPR_BON2:
            if (viewplayer->armorpoints < max_armor)
            {
                viewplayer->armorpoints++;
                P_UpdateArmorStat(1);
                armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
            }

            if (!viewplayer->armortype)
                viewplayer->armortype = GREENARMOR;

            if (message)
                HU_PlayerMessage(s_GOTARMBONUS, false);

            break;

        // soulsphere
        case SPR_SOUL:
            if (!(viewplayer->cheats & CF_GODMODE))
            {
                P_UpdateHealthStat(soul_health - viewplayer->health);
                viewplayer->health += soul_health;

                if (viewplayer->health > max_soul)
                    viewplayer->health = max_soul;

                viewplayer->mo->health = viewplayer->health;
                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            }

            if (message)
                HU_PlayerMessage(s_GOTSUPER, false);

            sound = sfx_getpow;
            break;

        // mega health
        case SPR_MEGA:
            P_GiveMegaHealth(stat);
            P_GiveArmor(blue_armor_class, stat);

            if (message)
                HU_PlayerMessage(s_GOTMSPHERE, false);

            sound = sfx_getpow;
            break;

        // blue keycard
        case SPR_BKEY:
            if (viewplayer->cards[it_bluecard] <= 0)
            {
                P_GiveCard(it_bluecard);

                if (message)
                    HU_PlayerMessage(s_GOTBLUECARD, false);

                break;
            }

            return;

        // yellow keycard
        case SPR_YKEY:
            if (viewplayer->cards[it_yellowcard] <= 0)
            {
                P_GiveCard(it_yellowcard);

                if (message)
                    HU_PlayerMessage(s_GOTYELWCARD, false);

                break;
            }

            return;

        // red keycard
        case SPR_RKEY:
            if (viewplayer->cards[it_redcard] <= 0)
            {
                P_GiveCard(it_redcard);

                if (message)
                    HU_PlayerMessage(s_GOTREDCARD, false);

                break;
            }

            return;

        // blue skull key
        case SPR_BSKU:
            if (viewplayer->cards[it_blueskull] <= 0)
            {
                P_GiveCard(it_blueskull);

                if (message)
                    HU_PlayerMessage(s_GOTBLUESKUL, false);

                break;
            }

            return;

        // yellow skull key
        case SPR_YSKU:
            if (viewplayer->cards[it_yellowskull] <= 0)
            {
                P_GiveCard(it_yellowskull);

                if (message)
                    HU_PlayerMessage(s_GOTYELWSKUL, false);

                break;
            }

            return;

        // red skull key
        case SPR_RSKU:
            if (viewplayer->cards[it_redskull] <= 0)
            {
                P_GiveCard(it_redskull);

                if (message)
                    HU_PlayerMessage(s_GOTREDSKULL, false);

                break;
            }

            return;

        // stimpack
        case SPR_STIM:
            if (!P_GiveBody(10, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTSTIM, false);

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
                    HU_PlayerMessage(buffer, false);
                }
                else
                    HU_PlayerMessage((viewplayer->health < 50 ? s_GOTMEDINEED : s_GOTMEDIKIT), false);
            }

            break;

        // invulnerability power-up
        case SPR_PINV:
            if (!P_GivePower(pw_invulnerability))
                return;

            if (message)
                HU_PlayerMessage(s_GOTINVUL, false);

            sound = sfx_getpow;
            break;

        // berserk power-up
        case SPR_PSTR:
            if (!P_GivePower(pw_strength))
                return;

            if (message)
                HU_PlayerMessage(s_GOTBERSERK, false);

            if (viewplayer->readyweapon != wp_fist)
                viewplayer->pendingweapon = wp_fist;

            viewplayer->fistorchainsaw = wp_fist;
            sound = sfx_getpow;
            break;

        // partial invisibility power-up
        case SPR_PINS:
            if (!P_GivePower(pw_invisibility))
                return;

            if (message)
                HU_PlayerMessage(s_GOTINVIS, false);

            sound = sfx_getpow;
            break;

        // radiation shielding suit power-up
        case SPR_SUIT:
            if (!P_GivePower(pw_ironfeet))
                return;

            if (message)
                HU_PlayerMessage(s_GOTSUIT, false);

            sound = sfx_getpow;
            break;

        // computer area map power-up
        case SPR_PMAP:
            P_GivePower(pw_allmap);

            if (message)
                HU_PlayerMessage(s_GOTMAP, false);

            sound = sfx_getpow;
            break;

        // light amplification visor power-up
        case SPR_PVIS:
            if (!P_GivePower(pw_infrared))
                return;

            if (message)
                HU_PlayerMessage(s_GOTVISOR, false);

            sound = sfx_getpow;
            break;

        // clip
        case SPR_CLIP:
            if (!(ammo = P_GiveAmmo(am_clip, !(special->flags & MF_DROPPED), stat)))
                return;

            if (message)
            {
                if (ammo == clipammo[am_clip] || deh_strlookup[p_GOTCLIP].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTCLIP, false);
                else
                    HU_PlayerMessage((ammo == clipammo[am_clip] / 2 ? s_GOTHALFCLIP : s_GOTCLIPX2), false);
            }

            break;

        // box of bullets
        case SPR_AMMO:
            if (!P_GiveAmmo(am_clip, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCLIPBOX, false);

            break;

        // rocket
        case SPR_ROCK:
            if (!(ammo = P_GiveAmmo(am_misl, 1, stat)))
                return;

            if (message)
            {
                if (ammo == clipammo[am_misl] || deh_strlookup[p_GOTROCKET].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTROCKET, false);
                else
                    HU_PlayerMessage(s_GOTROCKETX2, false);
            }

            break;

        // box of rockets
        case SPR_BROK:
            if (!P_GiveAmmo(am_misl, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTROCKBOX, false);

            break;

        // cell
        case SPR_CELL:
            if (!(ammo = P_GiveAmmo(am_cell, 1, stat)))
                return;

            if (message)
            {
                if (ammo == clipammo[am_cell] || deh_strlookup[p_GOTCELL].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTCELL, false);
                else
                    HU_PlayerMessage(s_GOTCELLX2, false);
            }

            break;

        // cell pack
        case SPR_CELP:
            if (!P_GiveAmmo(am_cell, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCELLBOX, false);

            break;

        // shells
        case SPR_SHEL:
            if (!(ammo = P_GiveAmmo(am_shell, 1, stat)))
                return;

            if (message)
            {
                if (ammo == clipammo[am_shell] || deh_strlookup[p_GOTSHELLS].assigned == 2 || hacx)
                    HU_PlayerMessage(s_GOTSHELLS, false);
                else
                    HU_PlayerMessage(s_GOTSHELLSX2, false);
            }

            break;

        // box of shells
        case SPR_SBOX:
            if (!P_GiveAmmo(am_shell, 5, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTSHELLBOX, false);

            break;

        // backpack
        case SPR_BPAK:
            if (!P_GiveBackpack(true, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTBACKPACK, false);

            break;

        // BFG-9000
        case SPR_BFUG:
            if (!P_GiveWeapon(wp_bfg, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTBFG9000, false);

            sound = sfx_wpnup;
            break;

        // chaingun
        case SPR_MGUN:
            if (!P_GiveWeapon(wp_chaingun, (special->flags & MF_DROPPED), stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTCHAINGUN, false);

            sound = sfx_wpnup;
            break;

        // chainsaw
        case SPR_CSAW:
            if (!P_GiveWeapon(wp_chainsaw, false, stat))
                return;

            viewplayer->fistorchainsaw = wp_chainsaw;

            if (message)
                HU_PlayerMessage(s_GOTCHAINSAW, false);

            sound = sfx_wpnup;
            break;

        // rocket launcher
        case SPR_LAUN:
            if (!P_GiveWeapon(wp_missile, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTLAUNCHER, false);

            sound = sfx_wpnup;
            break;

        // plasma rifle
        case SPR_PLAS:
            if (!P_GiveWeapon(wp_plasma, false, stat))
                return;

            if (message)
                HU_PlayerMessage(s_GOTPLASMA, false);

            sound = sfx_wpnup;
            break;

        // shotgun
        case SPR_SHOT:
            weaponowned = viewplayer->weaponowned[wp_shotgun];

            if (!P_GiveWeapon(wp_shotgun, (special->flags & MF_DROPPED), stat))
                return;

            if (!weaponowned)
                viewplayer->preferredshotgun = wp_shotgun;

            if (message)
                HU_PlayerMessage(s_GOTSHOTGUN, false);

            sound = sfx_wpnup;
            break;

        // super shotgun
        case SPR_SGN2:
            weaponowned = viewplayer->weaponowned[wp_supershotgun];

            if (!P_GiveWeapon(wp_supershotgun, (special->flags & MF_DROPPED), stat))
                return;

            if (!weaponowned)
                viewplayer->preferredshotgun = wp_supershotgun;

            if (message)
                HU_PlayerMessage(s_GOTSHOTGUN2, false);

            sound = sfx_wpnup;
            break;

        default:
            return;
    }

    if ((special->flags & MF_COUNTITEM) && stat)
    {
        viewplayer->itemcount++;
        stat_itemspickedup = SafeAdd(stat_itemspickedup, 1);
    }

    P_RemoveMobj(special);
    P_AddBonus();

    if (sound == prevsound && gametic == prevtic)
        return;

    prevsound = sound;
    prevtic = gametic;
    S_StartSound(viewplayer->mo, sound);
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
            static int  prev;
            int         r = M_RandomInt(1, 10);

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
        viewplayer->killcount++;
        stat_monsterskilled = SafeAdd(stat_monsterskilled, 1);

        if (!chex && !hacx)
        {
            viewplayer->mobjcount[type]++;
            P_UpdateKillStat(type, 1);
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
            AM_Stop();          // don't die in auto map, switch view prior to dying

        viewplayer->deaths++;
        stat_deaths = SafeAdd(stat_deaths, 1);
    }
    else
        target->flags2 &= ~MF2_NOLIQUIDBOB;

    gibhealth = info->gibhealth;

    if ((gibbed = (gibhealth < 0 && target->health < gibhealth && info->xdeathstate)))
        P_SetMobjState(target, info->xdeathstate);
    else
        P_SetMobjState(target, info->deathstate);

    if (!target->player)
        target->health = -1;

    target->tics = MAX(1, target->tics - (M_Random() & 3));

    if (type == MT_BARREL || type == MT_PAIN || type == MT_SKULL)
        target->flags2 &= ~MF2_CASTSHADOW;

    if (chex)
        return;

    if (con_obituaries && !hacx && !(target->flags2 & MF2_MASSACRE))
    {
        char        *name = (*info->name1 ? info->name1 : "monster");
        dboolean    defaultplayername = M_StringCompare(playername, playername_default);

        if (source)
        {
            if (inflicter && inflicter->type == MT_BARREL && type != MT_BARREL)
            {
                if (target->player)
                    C_Obituary("%s %s %s by an exploding barrel.", titlecase(playername),
                        (defaultplayername ? "were" : "was"), (gibbed ? "gibbed" : "killed"));
                else
                    C_Obituary("%s %s was %s by an exploding barrel.", (isvowel(name[0]) ? "An" : "A"), name,
                        (gibbed ? "gibbed" : "killed"));
            }
            else if (source->player)
            {
                weapontype_t    readyweapon = source->player->readyweapon;

                if (target->player)
                    C_Obituary("%s %s %s with %s own %s.", titlecase(playername),
                        (type == MT_BARREL ? "exploded" : (gibbed ? "gibbed" : "killed")),
                        (defaultplayername ? "yourself" : "themselves"), (defaultplayername ? "your" : "their"),
                        weaponinfo[readyweapon].description);
                else
                    C_Obituary("%s %s %s%s with %s %s%s.", titlecase(playername),
                        (type == MT_BARREL ? "exploded" : (gibbed ? "gibbed" : "killed")),
                        (isvowel(name[0]) ? "an " : "a "), name, (defaultplayername ? "your" : "their"),
                        (readyweapon == wp_fist && source->player->powers[pw_strength] ? "berserk " : ""),
                        weaponinfo[readyweapon].description);

            }
            else
            {
                if (source->type == MT_TFOG)
                    C_Obituary("%s%s %s telefragged.", (target->player ? "" : (isvowel(name[0]) ? "An " : "A ")),
                        (target->player ? titlecase(playername) : name), (defaultplayername ? "were" : "was"));
                else
                {
                    char    *sourcename = (*source->info->name1 ? source->info->name1 : "monster");

                    if (target->player)
                        C_Obituary("%s %s %s %s.", (isvowel(sourcename[0]) ? "An" : "A"), sourcename,
                            (type == MT_BARREL ? "exploded" : (gibbed ? "gibbed" : "killed")),
                            (defaultplayername ? playername : titlecase(playername)));
                    else
                        C_Obituary("%s %s %s %s%s.", (isvowel(sourcename[0]) ? "An" : "A"), sourcename,
                            (type == MT_BARREL ? "exploded" : (gibbed ? "gibbed" : "killed")),
                            (source->type == target->type ? "another " : (isvowel(name[0]) ? "an " : "a ")),
                            name);

                }
            }
        }
        else if (target->player)
        {
            sector_t    *sector = target->player->mo->subsector->sector;
            char        *liquid = "";

            if (sector->isliquid)
            {
                short   floorpic = sector->floorpic;

                if (floorpic >= nukagestart && floorpic <= nukageend)
                    liquid = " in nukage";
                else if ((floorpic >= fwaterstart && floorpic <= fwaterend)
                    || (floorpic >= swaterstart && floorpic <= swaterend))
                    liquid = " in water";
                else if (floorpic >= lavastart && floorpic <= lavaend)
                    liquid = " in lava";
                else if (floorpic >= bloodstart && floorpic <= bloodend)
                    liquid = " in blood";
                else if (floorpic >= slimestart && floorpic <= slimeend)
                    liquid = " in slime";
            }

            C_Obituary("%s %s %s%s.", titlecase(playername), (gibbed ? "gibbed" : "killed"),
                (defaultplayername ? "yourself" : "themselves"), liquid);
        }
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

    if (tossdrop)
    {
        mo = P_SpawnMobj(target->x, target->y, target->floorz + target->height * 3 / 2 - 3 * FRACUNIT, item);
        mo->momx = M_NegRandom() << 8;
        mo->momy = M_NegRandom() << 8;
        mo->momz = FRACUNIT * 2 + (M_Random() << 10);
    }
    else
        mo = P_SpawnMobj(target->x, target->y, ONFLOORZ, item);

    mo->angle = target->angle + (M_NegRandom() << 20);
    mo->flags |= MF_DROPPED;    // special versions of items

    if (r_mirroredweapons && (M_Random() & 1))
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
void P_DamageMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source, int damage, dboolean adjust)
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
        if (target->subsector->sector->special == DamageNegative10Or20PercentHealthAndEndLevel
            && damage >= target->health)
            damage = target->health - 1;

        // Below certain threshold,
        // ignore damage in GOD mode, or with INVUL power.
        if ((tplayer->cheats & CF_GODMODE) || idclevtics
            || (damage < 1000 && tplayer->powers[pw_invulnerability]))
            return;

        if (adjust && tplayer->armortype)
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
        damagecount = tplayer->damagecount + damage;            // add damage after armor/invuln

        if (damage > 0 && damagecount < 8)
             damagecount = 8;

        tplayer->damagecount = MIN(damagecount, 100);

        if (gp_vibrate_damage && vibrate)
        {
            XInputVibration((30000 + (100 - MIN(tplayer->health, 100)) / 100 * 30000) * gp_vibrate_damage / 100);
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

    if (M_Random() < info->painchance && !(target->flags & MF_SKULLFLY))
    {
        target->flags |= MF_JUSTHIT;                            // fight back!
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
