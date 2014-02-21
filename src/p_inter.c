/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

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

#include "am_map.h"
#include "doomstat.h"
#include "dstrings.h"
#include "m_random.h"
#include "p_local.h"
#include "p_inter.h"
#include "s_sound.h"

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[NUMAMMO] = { 200, 50, 300, 50 };
int clipammo[NUMAMMO] = { 10, 4, 20, 1 };

int vibrationtics = 0;

//
// GET STUFF
//

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//
boolean P_GiveAmmo(player_t *player, ammotype_t ammo, int num)
{
    int oldammo;

    if (ammo == am_noammo)
        return false;

    if (player->ammo[ammo] == player->maxammo[ammo])
        return false;

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

    // If non zero ammo, don't change up weapons, player was lower on purpose.
    if (oldammo)
        return true;

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
    return true;
}

void P_AddBonus(player_t *player, int amount)
{
    amount += player->bonuscount;
    if (amount > 3 * TICRATE)
        amount = 3 * TICRATE;
    player->bonuscount = amount;
}

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//
boolean P_GiveWeapon(player_t *player, weapontype_t weapon, boolean dropped)
{
    boolean gaveammo;
    boolean gaveweapon;

    if (netgame && deathmatch != 2 && !dropped)
    {
        // leave placed weapons forever on net games
        if (player->weaponowned[weapon])
            return false;

        P_AddBonus(player, BONUSADD);
        player->weaponowned[weapon] = true;

        if (deathmatch)
            P_GiveAmmo(player, weaponinfo[weapon].ammo, 5);
        else
            P_GiveAmmo(player, weaponinfo[weapon].ammo, 2);
        player->pendingweapon = weapon;

        if ((player->cheats & CF_CHOPPERS) && weapon != wp_chainsaw)
            G_RemoveChoppers();

        if (player == &players[consoleplayer])
            S_StartSound(player->mo, sfx_wpnup);
        return false;
    }

    if (weaponinfo[weapon].ammo != am_noammo)
    {
        // give one clip with a dropped weapon,
        // two clips with a found weapon
        gaveammo = P_GiveAmmo(player, weaponinfo[weapon].ammo, dropped ? 1 : 2);
    }
    else
        gaveammo = false;

    if (player->weaponowned[weapon])
        gaveweapon = false;
    else
    {
        gaveweapon = true;
        player->weaponowned[weapon] = true;
        oldweaponsowned[weapon] = true;
        player->pendingweapon = weapon;
    }

    return (gaveweapon || gaveammo);
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//
boolean P_GiveBody(player_t *player, int num)
{
    if (player->health >= MAXHEALTH)
        return false;

    player->health += num;
    if (player->health > MAXHEALTH)
        player->health = MAXHEALTH;
    player->mo->health = player->health;

    return true;
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
boolean P_GiveArmor(player_t *player, int armortype)
{
    int hits;

    hits = armortype * 100;
    if (player->armorpoints >= hits)
        return false;   // don't pick up

    player->armortype = armortype;
    player->armorpoints = hits;

    return true;
}

//
// P_GiveCard
//
void P_GiveCard(player_t *player, card_t card)
{
    if (player->cards[card])
        return;

    P_AddBonus(player, BONUSADD);
    player->cards[card] = true;
}

//
// P_GivePower
//
boolean P_GivePower(player_t *player, int power)
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
            player->mo->flags |= MF_SHADOW;
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
    player_t   *player;
    int        i;
    fixed_t    delta = special->z - toucher->z;
    int        sound;

    int        weaponowned;
    boolean    ammogiven = false;

    static int prevsound = 0;
    static int prevtic = 0;

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
            if (!P_GiveArmor(player, 1))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTARMOR;
            break;

        case SPR_ARM2:
            if (!P_GiveArmor(player, 2))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTMEGA;
            break;

        // bonus items
        case SPR_BON1:
            if (!(player->cheats & CF_GODMODE))
            {
                player->health++;       // can go over 100%
                if (player->health > 200)
                    player->health = 200;
                player->mo->health = player->health;
            }
            if (!message_dontfuckwithme)
                player->message = GOTHTHBONUS;
            break;

        case SPR_BON2:
            player->armorpoints++;      // can go over 100%
            if (player->armorpoints > 200)
                player->armorpoints = 200;
            if (!player->armortype)
                player->armortype = 1;
            if (!message_dontfuckwithme)
                player->message = GOTARMBONUS;
            break;

        case SPR_SOUL:
            if (!(player->cheats & CF_GODMODE))
            {
                player->health += 100;
                if (player->health > 200)
                    player->health = 200;
                player->mo->health = player->health;
            }
            if (!message_dontfuckwithme)
                player->message = GOTSUPER;
            sound = sfx_getpow;
            break;

        case SPR_MEGA:
            if (!(player->cheats & CF_GODMODE))
            {
                player->health = 200;
                player->mo->health = player->health;
            }
            P_GiveArmor(player, 2);
            if (!message_dontfuckwithme)
                player->message = GOTMSPHERE;
            sound = sfx_getpow;
            break;

        // cards
        // leave cards for everyone
        case SPR_BKEY:
            if (!player->cards[it_bluecard])
            {
                if (!message_dontfuckwithme)
                    player->message = GOTBLUECARD;
                P_GiveCard(player, it_bluecard);
                if (!netgame)
                    break;
            }
            return;

        case SPR_YKEY:
            if (!player->cards[it_yellowcard])
            {
                if (!message_dontfuckwithme)
                    player->message = GOTYELWCARD;
                P_GiveCard(player, it_yellowcard);
                if (!netgame)
                    break;
            }
            return;

        case SPR_RKEY:
            if (!player->cards[it_redcard])
            {
                if (!message_dontfuckwithme)
                    player->message = GOTREDCARD;
                P_GiveCard(player, it_redcard);
                if (!netgame)
                    break;
            }
            return;

        case SPR_BSKU:
            if (!player->cards[it_blueskull])
            {
                if (!message_dontfuckwithme)
                    player->message = GOTBLUESKUL;
                P_GiveCard(player, it_blueskull);
                if (!netgame)
                    break;
            }
            return;

        case SPR_YSKU:
            if (!player->cards[it_yellowskull])
            {
                if (!message_dontfuckwithme)
                    player->message = GOTYELWSKUL;
                P_GiveCard(player, it_yellowskull);
                if (!netgame)
                    break;
            }
            return;

        case SPR_RSKU:
            if (!player->cards[it_redskull])
            {
                if (!message_dontfuckwithme)
                    player->message = GOTREDSKULL;
                P_GiveCard(player, it_redskull);
                if (!netgame)
                    break;
            }
            return;

        // medikits, heals
        case SPR_STIM:
            if (!P_GiveBody(player, 10))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTSTIM;
            break;

        case SPR_MEDI:
            if (!P_GiveBody(player, 25))
                return;
            if (!message_dontfuckwithme)
            {
                if (player->health < 50)
                    player->message = GOTMEDINEED;
                else
                    player->message = GOTMEDIKIT;
            }
            break;

        // power ups
        case SPR_PINV:
            if (!P_GivePower(player, pw_invulnerability))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTINVUL;
            sound = sfx_getpow;
            break;

        case SPR_PSTR:
            if (!P_GivePower(player, pw_strength))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTBERSERK;
            if (player->readyweapon != wp_fist)
                player->pendingweapon = wp_fist;
            player->fistorchainsaw = wp_fist;
            sound = sfx_getpow;
            break;

        case SPR_PINS:
            if (!P_GivePower(player, pw_invisibility))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTINVIS;
            sound = sfx_getpow;
            break;

        case SPR_SUIT:
            if (!P_GivePower(player, pw_ironfeet))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTSUIT;
            sound = sfx_getpow;
            break;

        case SPR_PMAP:
            P_GivePower(player, pw_allmap);
            if (!message_dontfuckwithme)
                player->message = GOTMAP;
            sound = sfx_getpow;
            break;

        case SPR_PVIS:
            if (!P_GivePower(player, pw_infrared))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTVISOR;
            sound = sfx_getpow;
            break;

        // ammo
        case SPR_CLIP:
            if (special->flags & MF_DROPPED)
            {
                if (!P_GiveAmmo(player, am_clip, 0))
                    return;
            }
            else
            {
                if (!P_GiveAmmo(player, am_clip, 1))
                    return;
            }
            if (!message_dontfuckwithme)
                player->message = GOTCLIP;
            break;

        case SPR_AMMO:
            if (!P_GiveAmmo(player, am_clip, 5))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTCLIPBOX;
            break;

        case SPR_ROCK:
            if (!P_GiveAmmo(player, am_misl, 1))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTROCKET;
            break;

        case SPR_BROK:
            if (!P_GiveAmmo(player, am_misl, 5))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTROCKBOX;
            break;

        case SPR_CELL:
            if (!P_GiveAmmo(player, am_cell, 1))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTCELL;
            break;

        case SPR_CELP:
            if (!P_GiveAmmo(player, am_cell, 5))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTCELLBOX;
            break;

        case SPR_SHEL:
            if (!P_GiveAmmo(player, am_shell, 1))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTSHELLS;
            break;

        case SPR_SBOX:
            if (!P_GiveAmmo(player, am_shell, 5))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTSHELLBOX;
            break;

        case SPR_BPAK:
            if (!player->backpack)
            {
                for (i = 0; i < NUMAMMO; i++)
                    player->maxammo[i] *= 2;
                player->backpack = true;
                ammogiven = true;
            }
            for (i = 0; i < NUMAMMO; i++)
            {
                if (player->ammo[i] < player->maxammo[i])
                    ammogiven = true;
                P_GiveAmmo(player, (ammotype_t)i, 1);
            }
            if (!ammogiven)
                return;
            if (!message_dontfuckwithme)
                player->message = GOTBACKPACK;
            break;

        // weapons
        case SPR_BFUG:
            if (!P_GiveWeapon(player, wp_bfg, false))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTBFG9000;
            sound = sfx_wpnup;
            break;

        case SPR_MGUN:
            if (!P_GiveWeapon(player, wp_chaingun, special->flags & MF_DROPPED))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTCHAINGUN;
            sound = sfx_wpnup;
            break;

        case SPR_CSAW:
            if (!P_GiveWeapon(player, wp_chainsaw, false))
                return;
            player->fistorchainsaw = wp_chainsaw;
            if (!message_dontfuckwithme)
                player->message = GOTCHAINSAW;
            sound = sfx_wpnup;
            break;

        case SPR_LAUN:
            if (!P_GiveWeapon(player, wp_missile, false))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTLAUNCHER;
            sound = sfx_wpnup;
            break;

        case SPR_PLAS:
            if (!P_GiveWeapon(player, wp_plasma, false))
                return;
            if (!message_dontfuckwithme)
                player->message = GOTPLASMA;
            sound = sfx_wpnup;
            break;

        case SPR_SHOT:
            weaponowned = player->weaponowned[wp_shotgun];
            if (!P_GiveWeapon(player, wp_shotgun, special->flags & MF_DROPPED))
                return;
            if (!weaponowned)
                player->preferredshotgun = wp_shotgun;
            player->shotguns = (player->weaponowned[wp_shotgun] || player->weaponowned[wp_supershotgun]);
            if (!message_dontfuckwithme)
                player->message = GOTSHOTGUN;
            sound = sfx_wpnup;
            break;

        case SPR_SGN2:
            weaponowned = player->weaponowned[wp_supershotgun];
            if (!P_GiveWeapon(player, wp_supershotgun,
                special->flags & MF_DROPPED))
                return;
            if (!weaponowned)
                player->preferredshotgun = wp_supershotgun;
            player->shotguns = (player->weaponowned[wp_shotgun] || player->weaponowned[wp_supershotgun]);
            if (!message_dontfuckwithme)
                player->message = GOTSHOTGUN2;
            sound = sfx_wpnup;
            break;

        default:
            return;
    }

    if (special->flags & MF_COUNTITEM)
        player->itemcount++;
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
    mobjtype_t item;
    mobj_t     *mo;

    target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY);

    if (target->type == MT_SKULL)
        target->momx = target->momy = target->momz = 0;
    else
        target->flags &= ~MF_NOGRAVITY;

    target->flags |= (MF_CORPSE | MF_DROPOFF);
    target->height >>= 2;

    if (target->type != MT_CYBORG)
    {
        static int prev = 0;
        int        r = M_RandomInt(1, 10);

        if (r <= 5 + prev)
        {
            prev--;
            target->flags2 |= MF2_FLIPPEDCORPSE;
        }
        else
        {
            prev++;
        }
    }

    if (source && source->player)
    {
        // count for intermission
        if (target->flags & MF_COUNTKILL)
            source->player->killcount++;

        if (target->player)
            source->player->frags[target->player-players]++;
    }
    else if (!netgame && (target->flags & MF_COUNTKILL))
    {
        // count all monster deaths,
        // even those caused by other monsters
        players[0].killcount++;
    }

    if (target->type == MT_BARREL && source && source->player)
        target->target = source;

    if (target->player)
    {
        // count environment kills against you
        if (!source)
            target->player->frags[target->player-players]++;

        target->flags &= ~MF_SOLID;
        target->player->playerstate = PST_DEAD;
        P_DropWeapon(target->player);

        if (target->player == &players[consoleplayer] && automapactive)
        {
            // don't die in auto map,
            // switch view prior to dying
            AM_Stop();
        }
    }

    if (target->health < -target->info->spawnhealth && target->info->xdeathstate)
        P_SetMobjState(target, (statenum_t)target->info->xdeathstate);
    else
        P_SetMobjState(target, (statenum_t)target->info->deathstate);

    target->tics -= P_Random() & 3;
    if (target->tics < 1)
        target->tics = 1;

    // Drop stuff.
    // This determines the kind of object spawned
    // during the death frame of a thing.
    switch (target->type)
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

    mo = P_SpawnMobj(target->x, target->y, 
                     target->floorz + FRACUNIT * (target->height / 2), item);
    mo->momx += P_Random() << 8;
    mo->momy += P_Random() << 8;
    mo->momz = FRACUNIT * 5 + (P_Random() << 10);
    mo->angle = target->angle + ((P_Random() - P_Random()) << 20);
    mo->flags |= MF_DROPPED;    // special versions of items
}

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
    player_t *player;

    if (!(target->flags & MF_SHOOTABLE))
        return;         // shouldn't happen...

    if (target->health <= 0)
        return;

    if (target->flags & MF_SKULLFLY)
        target->momx = target->momy = target->momz = 0;

    player = target->player;
    if (player && gameskill == sk_baby)
        damage >>= (damage > 1);

    // Some close combat weapons should not
    // inflict thrust and push the victim out of reach,
    // thus kick away unless using the chainsaw.
    if (inflictor && !(target->flags & MF_NOCLIP)
        && (!source || !source->player || source->player->readyweapon != wp_chainsaw))
    {
        unsigned ang = R_PointToAngle2(inflictor->x, inflictor->y, target->x, target->y);
        fixed_t  thrust = damage * (FRACUNIT >> 3) * 100 / target->info->mass;

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

            viewx = 0;
            viewy = z;
            ang = R_PointToAngle(dist, target->z);

            ang >>= ANGLETOFINESHIFT;
            target->momz += FixedMul(thrust, finesine[ang]);
        }
    }

    // player specific
    if (player)
    {
        // end of game hell hack
        if (target->subsector->sector->special == ExitSuperDamage && damage >= target->health)
            damage = target->health - 1;

        // Below certain threshold,
        // ignore damage in GOD mode, or with INVUL power.
        if ((player->cheats & CF_GODMODE)
            || (damage < 1000 && player->powers[pw_invulnerability]))
            return;

        if (player->armortype)
        {
            int saved = damage / (player->armortype == 1 ? 3 : 2);

            if (player->armorpoints <= saved)
            {
                // armor is used up
                saved = player->armorpoints;
                player->armortype = 0;
            }
            player->armorpoints -= saved;
            damage -= saved;
        }
        player->health -= damage;       // mirror mobj health here for Dave
        if (player->health < 0)
            player->health = 0;

        player->attacker = source;
        player->damagecount += damage;  // add damage after armor / invuln

        if (player->damagecount > 100)
            player->damagecount = 100;  // teleport stomp does 10k points...

        if (gamepadvibrate && vibrate && player == &players[consoleplayer])
        {
            XInputVibration(10000 + (100 - (player->health < 100 ? 
                player->health : 100)) / 100 * 50000, 0);
            vibrationtics += (vibrationtics + damage < 12 ?
                12 : (damage < 100 ? damage: 100));
        }
    }

    // do the damage
    target->health -= damage;
    if (target->health <= 0)
    {
        if (target->type == MT_BARREL || target->type == MT_PAIN || target->type == MT_SKULL)
            target->flags2 |= MF2_TRANSLUCENT_REDONLY;
        else if (target->type == MT_BRUISER || target->type == MT_KNIGHT)
            target->flags2 |= MF2_REDTOGREEN;

        P_KillMobj(source, target);
        return;
    }

    if (P_Random() < target->info->painchance && !(target->flags & MF_SKULLFLY))
    {
        target->flags |= MF_JUSTHIT;    // fight back!

        P_SetMobjState(target, (statenum_t)target->info->painstate);
    }

    target->reactiontime = 0;           // we're awake now...

    if ((!target->threshold || target->type == MT_VILE)
         && source && source != target && source->type != MT_VILE)
    {
        // if not intent on another player,
        // chase after this one
        target->target = source;
        target->threshold = BASETHRESHOLD;
        if (target->state == &states[target->info->spawnstate] && target->info->seestate != S_NULL)
            P_SetMobjState(target, (statenum_t)target->info->seestate);
    }
}
