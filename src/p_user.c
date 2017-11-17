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

#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "m_config.h"
#include "p_local.h"
#include "s_sound.h"

#define DEADLOOKDIR     128
#define DEADLOOKDIRINC  24

int             deathcount;

extern fixed_t  animatedliquiddiff;
extern dboolean canmouselook;
extern dboolean skipaction;
extern dboolean usemouselook;

void G_RemoveChoppers(void);

//
// Movement
//

dboolean        autouse = autouse_default;
int             movebob = movebob_default;
dboolean        r_liquid_lowerview = r_liquid_lowerview_default;
int             r_shake_damage = r_shake_damage_default;
int             stillbob = stillbob_default;

dboolean        autousing = false;
static dboolean onground;

//
// P_Thrust
// Moves the given origin along a given angle.
//
static void P_Thrust(player_t *player, angle_t angle, fixed_t move)
{
    player->mo->momx += FixedMul(move, finecosine[angle >>= ANGLETOFINESHIFT]);
    player->mo->momy += FixedMul(move, finesine[angle]);
}

//
// P_Bob
// Same as P_Thrust, but only affects bobbing.
//
// killough 10/98: We apply thrust separately between the real physical player
// and the part which affects bobbing. This way, bobbing only comes from player
// motion, nothing external, avoiding many problems, e.g. bobbing should not
// occur on conveyors, unless the player walks on one, and bobbing should be
// reduced at a regular rate, even on ice (where the player coasts).
//
static void P_Bob(player_t *player, angle_t angle, fixed_t move)
{
    player->momx += FixedMul(move, finecosine[angle >>= ANGLETOFINESHIFT]);
    player->momy += FixedMul(move, finesine[angle]);
}

//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight(player_t *player)
{
    mobj_t  *mo = player->mo;

    if (player->playerstate == PST_LIVE)
    {
        // Regular movement bobbing
        // (needs to be calculated for gun swing
        // even if not on ground)
        fixed_t momx = player->momx;
        fixed_t momy = player->momy;
        fixed_t bob = ((momx | momy) ? (FixedMul(momx, momx) + FixedMul(momy, momy)) >> 2 : 0);

        bob = FixedMul((bob ? MAX(MIN(bob, MAXBOB) * movebob / 100, MAXBOB * stillbob / 400) :
            MAXBOB * stillbob / 400) / 2, finesine[(FINEANGLES / 20 * leveltime) & FINEMASK]);

        // move viewheight
        player->viewheight += player->deltaviewheight;

        if (player->viewheight > VIEWHEIGHT)
        {
            player->viewheight = VIEWHEIGHT;
            player->deltaviewheight = 0;
        }

        if (player->viewheight < VIEWHEIGHT / 2)
        {
            player->viewheight = VIEWHEIGHT / 2;

            if (player->deltaviewheight <= 0)
                player->deltaviewheight = 1;
        }

        if (player->deltaviewheight)
        {
            player->deltaviewheight += FRACUNIT / 4;

            if (!player->deltaviewheight)
                player->deltaviewheight = 1;
        }

        player->viewz = mo->z + player->viewheight + bob;
    }
    else
        player->viewz = mo->z + player->viewheight;

    if (mo->flags2 & MF2_FEETARECLIPPED)
    {
        dboolean    liquid = true;

        for (const struct msecnode_s *seclist = mo->touching_sectorlist; seclist; seclist = seclist->m_tnext)
            if (!seclist->m_sector->isliquid)
            {
                liquid = false;
                break;
            }

        if (liquid)
        {
            if (player->playerstate == PST_DEAD)
            {
                if (r_liquid_bob)
                    player->viewz += animatedliquiddiff;
            }
            else if (r_liquid_lowerview)
            {
                sector_t    *sector = mo->subsector->sector;

                if (!P_IsSelfReferencingSector(sector) && (!sector->heightsec
                    || mo->z + player->viewheight - FOOTCLIPSIZE >= sector->heightsec->floorheight))
                    player->viewz -= FOOTCLIPSIZE;
            }
        }
    }

    player->viewz = BETWEEN(mo->floorz + 4 * FRACUNIT, player->viewz, mo->ceilingz - 4 * FRACUNIT);
}

//
// P_MovePlayer
//
void P_MovePlayer(player_t *player)
{
    ticcmd_t    *cmd = &player->cmd;
    mobj_t      *mo = player->mo;
    char        forwardmove = cmd->forwardmove;
    char        sidemove = cmd->sidemove;

    mo->angle += cmd->angleturn << FRACBITS;

    // killough 10/98:
    //
    // We must apply thrust to the player and bobbing separately, to avoid
    // anomalies. The thrust applied to bobbing is always the same strength on
    // ice, because the player still "works just as hard" to move, while the
    // thrust applied to the movement varies with 'movefactor'.
    if (forwardmove | sidemove)                                                 // killough 10/98
    {
        if ((onground = (mo->z <= mo->floorz || (mo->flags2 & MF2_ONMOBJ))))    // killough 8/9/98
        {
            int     friction;
            int     movefactor = P_GetMoveFactor(mo, &friction);
            angle_t angle = mo->angle;

            // killough 11/98:
            // On sludge, make bobbing depend on efficiency.
            // On ice, make it depend on effort.
            int     bobfactor = (friction < ORIG_FRICTION ? movefactor : ORIG_FRICTION_FACTOR);

            if (forwardmove)
            {
                P_Bob(player, angle, forwardmove * bobfactor);
                P_Thrust(player, angle, forwardmove * movefactor);
            }

            if (sidemove)
            {
                P_Bob(player, (angle -= ANG90), sidemove * bobfactor);
                P_Thrust(player, angle, sidemove * movefactor);
            }
        }

        if (mo->state == states + S_PLAY)
            P_SetMobjState(mo, S_PLAY_RUN1);
    }

    player->lookdir = BETWEEN(-LOOKDIRMAX * MLOOKUNIT, player->lookdir + cmd->lookdir, LOOKDIRMAX * MLOOKUNIT);

    if (player->lookdir && !usemouselook)
    {
        if (player->lookdir > 0)
            player->lookdir -= 16 * MLOOKUNIT;
        else
            player->lookdir += 16 * MLOOKUNIT;

        if (ABS(player->lookdir) < 16 * MLOOKUNIT)
            player->lookdir = 0;
    }
}

//
// P_ReduceDamageCount
//
static void P_ReduceDamageCount(player_t *player)
{
    if (player->damagecount)
        player->damagecount--;

    if (r_shake_damage)
        I_UpdateBlitFunc(!!player->damagecount);
}

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
static void P_DeathThink(player_t *player)
{
    static dboolean facingkiller;
    mobj_t          *mo = player->mo;
    mobj_t          *attacker = player->attacker;

    weaponvibrationtics = 1;
    idlemotorspeed = 0;
    infight = true;

    P_MovePsprites(player);

    // fall to the ground
    if ((onground = (mo->z <= mo->floorz || (mo->flags2 & MF2_ONMOBJ))))
    {
        if (player->viewheight > 6 * FRACUNIT)
            player->viewheight -= FRACUNIT;

        if (player->viewheight < 6 * FRACUNIT)
            player->viewheight = 6 * FRACUNIT;

        if (canmouselook)
        {
            if (player->lookdir > DEADLOOKDIR)
                player->lookdir -= DEADLOOKDIRINC;
            else if (player->lookdir < DEADLOOKDIR)
                player->lookdir += DEADLOOKDIRINC;

            if (ABS(player->lookdir - DEADLOOKDIR) < DEADLOOKDIRINC)
                player->lookdir = DEADLOOKDIR;
        }
    }

    player->deltaviewheight = 0;
    P_CalcHeight(player);

    if (attacker && attacker != mo && !facingkiller)
    {
        angle_t angle = R_PointToAngle2(mo->x, mo->y, attacker->x, attacker->y);
        angle_t delta = angle - mo->angle;

        if (delta < ANG5 || delta > (unsigned int)(-ANG5))
        {
            // Looking at killer, so fade damage flash down.
            mo->angle = angle;

            P_ReduceDamageCount(player);

            facingkiller = true;
        }
        else
            mo->angle += (delta < ANG180 ? ANG5 : -ANG5);
    }
    else
        P_ReduceDamageCount(player);

    if (player->bonuscount)
        player->bonuscount--;

    if (consoleactive)
        return;

    if (((player->cmd.buttons & BT_USE) || ((player->cmd.buttons & BT_ATTACK) && !player->damagecount
        && deathcount > TICRATE * 2) || gamekeydown[KEY_ENTER]))
    {
        deathcount = 0;
        damagevibrationtics = 1;
        player->playerstate = PST_REBORN;
        facingkiller = false;
        skipaction = true;
    }
    else
        deathcount++;
}

//
// P_ResurrectPlayer
//
void P_ResurrectPlayer(player_t *player, int health)
{
    fixed_t x, y;
    int     angle;
    mobj_t  *thing;

    // remove player's corpse
    P_RemoveMobj(player->mo);

    // spawn a teleport fog
    x = player->mo->x;
    y = player->mo->y;
    angle = player->mo->angle >> ANGLETOFINESHIFT;
    thing = P_SpawnMobj(x + 20 * finecosine[angle], y + 20 * finesine[angle], ONFLOORZ, MT_TFOG);
    thing->angle = player->mo->angle;
    S_StartSound(thing, sfx_telept);

    // telefrag anything in this spot
    P_TeleportMove(thing, thing->x, thing->y, thing->z, true);

    // respawn the player
    thing = P_SpawnMobj(x, y, ONFLOORZ, MT_PLAYER);
    thing->angle = player->mo->angle;
    thing->player = player;
    thing->health = health;
    thing->reactiontime = 18;
    player->mo = thing;
    player->playerstate = PST_LIVE;
    player->viewheight = VIEWHEIGHT;
    player->health = health;
    player->lookdir = 0;
    player->oldlookdir = 0;
    infight = false;
    P_SetupPsprites(player);
    P_MapEnd();

    C_HideConsole();
}

void P_ChangeWeapon(player_t *player, weapontype_t newweapon)
{
    if (newweapon == wp_fist)
    {
        if (player->readyweapon == wp_fist)
        {
            if (player->weaponowned[wp_chainsaw])
            {
                newweapon = wp_chainsaw;
                player->fistorchainsaw = wp_chainsaw;
            }
        }
        else if (player->readyweapon == wp_chainsaw)
        {
            if (player->powers[pw_strength])
                player->fistorchainsaw = wp_fist;
            else
                newweapon = wp_nochange;
        }
        else
            newweapon = player->fistorchainsaw;
    }

    // Don't switch to a weapon without any or enough ammo.
    else if (((newweapon == wp_pistol || newweapon == wp_chaingun) && !player->ammo[am_clip])
        || (newweapon == wp_shotgun && !player->ammo[am_shell])
        || (newweapon == wp_missile && !player->ammo[am_misl])
        || (newweapon == wp_plasma && !player->ammo[am_cell])
        || (newweapon == wp_bfg && player->ammo[am_cell] < bfgcells && bfgcells == BFGCELLS))
        newweapon = wp_nochange;

    // Select the preferred shotgun.
    else if (newweapon == wp_shotgun)
    {
        if ((!player->weaponowned[wp_shotgun] || player->readyweapon == wp_shotgun)
            && player->weaponowned[wp_supershotgun] && player->ammo[am_shell] >= 2)
            player->preferredshotgun = wp_supershotgun;
        else if (player->readyweapon == wp_supershotgun
            || (player->preferredshotgun == wp_supershotgun && player->ammo[am_shell] == 1))
            player->preferredshotgun = wp_shotgun;

        newweapon = player->preferredshotgun;
    }

    if (newweapon != wp_nochange && newweapon != player->readyweapon && player->weaponowned[newweapon])
    {
        player->pendingweapon = newweapon;

        if (newweapon == wp_fist && player->powers[pw_strength])
            S_StartSound(NULL, sfx_getpow);

        if ((player->cheats & CF_CHOPPERS) && newweapon != wp_chainsaw)
            G_RemoveChoppers();
    }
}

//
// P_PlayerThink
//
void P_PlayerThink(player_t *player)
{
    ticcmd_t    *cmd = &player->cmd;
    mobj_t      *mo = player->mo;
    static int  motionblur;

    if (player->bonuscount)
        player->bonuscount--;

    if (consoleactive)
        return;

    // [AM] Assume we can interpolate at the beginning of the tic.
    mo->interpolate = true;

    // [AM] Store starting position for player interpolation.
    mo->oldx = mo->x;
    mo->oldy = mo->y;
    mo->oldz = mo->z;
    mo->oldangle = mo->angle;
    player->oldviewz = player->viewz;
    player->oldlookdir = player->lookdir;
    player->oldrecoil = player->recoil;

    if (player->cheats & CF_NOCLIP)
        mo->flags |= MF_NOCLIP;
    else
        mo->flags &= ~MF_NOCLIP;

    // chainsaw run forward
    if (mo->flags & MF_JUSTATTACKED)
    {
        cmd->angleturn = 0;
        cmd->forwardmove = 0xC800 / 512;
        cmd->sidemove = 0;
        mo->flags &= ~MF_JUSTATTACKED;
    }

    if (vid_motionblur)
    {
        motionblur = 0;

        if (!automapactive)
        {
            if (player->damagecount)
                motionblur = MAX(motionblur, 100);
            else
            {
                if (cmd->angleturn)
                    motionblur = MIN(ABS(cmd->angleturn) * 100 / 960, 150);

                if (cmd->lookdir)
                    motionblur = MAX(motionblur, 100);
            }
        }

        I_SetMotionBlur(motionblur * vid_motionblur / 100);
    }
    else if (motionblur)
    {
        motionblur = 0;
        I_SetMotionBlur(0);
    }

    if (player->recoil)
        player->recoil += (player->recoil > 0 ? -1 : 1);

    if (player->playerstate == PST_DEAD)
    {
        P_DeathThink(player);
        return;
    }

    // [BH] regenerate health up to 100 every 1 second
    if (regenhealth && mo->health < initial_health && !(leveltime % TICRATE) && !player->damagecount)
        mo->health = player->health = MIN(player->health + 1, initial_health);

    // Move around.
    // Reaction time is used to prevent movement for a bit after a teleport.
    if (mo->reactiontime)
        mo->reactiontime--;
    else
        P_MovePlayer(player);

    P_CalcHeight(player);

    // [BH] Check all sectors player is touching are special
    if (!freeze)
        for (const struct msecnode_s *seclist = mo->touching_sectorlist; seclist; seclist = seclist->m_tnext)
            if (seclist->m_sector->special && mo->z == seclist->m_sector->interpfloorheight)
            {
                P_PlayerInSpecialSector(player);
                break;
            }

    // Check for weapon change.

    // A special event has no other buttons.
    if (cmd->buttons & BT_SPECIAL)
        cmd->buttons = 0;

    if ((cmd->buttons & BT_CHANGE) && (!automapactive || am_followmode))
        P_ChangeWeapon(player, (cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT);

    if (autouse && !(leveltime % TICRATE))
    {
        autousing = true;
        P_UseLines(player);
        autousing = false;
    }

    // check for use
    if (cmd->buttons & BT_USE)
    {
        if (!player->usedown)
        {
            P_UseLines(player);
            player->usedown = true;
        }
    }
    else
        player->usedown = false;

    // cycle psprites
    P_MovePsprites(player);

    // Counters, time dependent power ups.
    if (player->powers[pw_invulnerability] > 0)
        player->powers[pw_invulnerability]--;

    if (player->powers[pw_invisibility] > 0)
        if (!--player->powers[pw_invisibility])
            player->mo->flags &= ~MF_FUZZ;

    if (player->powers[pw_infrared] > 0)
        player->powers[pw_infrared]--;

    if (player->powers[pw_ironfeet] > 0)
        player->powers[pw_ironfeet]--;

    P_ReduceDamageCount(player);

    // Handling colormaps.
    if (player->powers[pw_invulnerability] > STARTFLASHING || (player->powers[pw_invulnerability] & 8))
        player->fixedcolormap = INVERSECOLORMAP;
    else
        player->fixedcolormap = (player->powers[pw_infrared] > STARTFLASHING
            || (player->powers[pw_infrared] & 8));
}
