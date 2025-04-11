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

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

static void T_GradualLightingToDoor(vldoor_t *door)
{
    sector_t        *sec = door->sector;
    const fixed_t   level = door->topheight - sec->floorheight;

    if (level > 0)
    {
        if (door->lighttag)
            EV_LightTurnOnPartway(door->line, FixedDiv(sec->ceilingheight - sec->floorheight, level));
        else if (!islightspecial[sec->special] && sec->ceilingpic != skyflatnum)
            EV_LightByAdjacentSectors(sec, FixedDiv(sec->ceilingheight - sec->floorheight, level));
    }
}

//
// VERTICAL DOORS
//

//
// T_VerticalDoor
//
void T_VerticalDoor(vldoor_t *door)
{
    result_e    res;

    switch (door->direction)
    {
        case 0:
            // WAITING
            if (!--door->topcountdown)
                switch (door->type)
                {
                    case DoorBlazeRaise:
                    case GenBlazeRaise:
                        door->direction = -1;   // time to go back down
                        S_StartSectorSound(&door->sector->soundorg, sfx_bdcls);
                        break;

                    case DoorNormal:
                    case GenRaise:
                        door->direction = -1;   // time to go back down
                        S_StartSectorSound(&door->sector->soundorg, sfx_dorcls);
                        break;

                    case DoorClose30ThenOpen:
                    case GenCdO:
                        door->direction = 1;    // time to go back up
                        S_StartSectorSound(&door->sector->soundorg, sfx_doropn);
                        break;

                    case GenBlazeCdO:
                        door->direction = 1;    // time to go back up
                        S_StartSectorSound(&door->sector->soundorg, sfx_bdopn);
                        break;

                    default:
                        break;
                }

            break;

        case 2:
            // INITIAL WAIT
            if (!--door->topcountdown)
                switch (door->type)
                {
                    case DoorRaiseIn5Mins:
                        door->direction = 1;
                        door->type = DoorNormal;
                        S_StartSectorSound(&door->sector->soundorg, sfx_doropn);
                        break;

                    default:
                        break;
                }

            break;

        case -1:
            // DOWN
            res = T_MovePlane(door->sector, door->speed, door->sector->floorheight, false, 1, door->direction);

            // killough 10/98: implement gradual lighting effects
            // [BH] enhanced to apply effects to all doors
            if (r_graduallighting && !nograduallighting)
                T_GradualLightingToDoor(door);

            if (res == pastdest)
            {
                switch (door->type)
                {
                    case DoorBlazeRaise:
                    case DoorBlazeClose:
                    case GenBlazeRaise:
                    case GenBlazeClose:
                    case DoorNormal:
                    case DoorClose:
                    case GenRaise:
                    case GenClose:
                        door->sector->ceilingdata = NULL;
                        P_RemoveThinker(&door->thinker);     // unlink and free
                        break;

                    case DoorClose30ThenOpen:
                        door->direction = 0;
                        door->topcountdown = TICRATE * 30;
                        break;

                    case GenCdO:
                    case GenBlazeCdO:
                        door->direction = 0;
                        door->topcountdown = door->topwait;     // jff 05/08/98 insert delay
                        break;

                    default:
                        break;
                }
            }
            else if (res == crushed)
            {
                switch (door->type)
                {
                    case DoorBlazeClose:
                    case DoorClose:
                    case GenClose:
                    case GenBlazeClose:                         // DO NOT GO BACK UP!
                        break;

                    // [BH] play correct sound when raising fast doors
                    case DoorBlazeRaise:
                    case GenBlazeRaise:
                        door->direction = 1;
                        S_StartSectorSound(&door->sector->soundorg, sfx_bdopn);
                        break;

                    default:
                        door->direction = 1;
                        S_StartSectorSound(&door->sector->soundorg, sfx_doropn);
                        break;
                }
            }

            break;

        case 1:
            // UP
            res = T_MovePlane(door->sector, door->speed, door->topheight, false, 1, door->direction);

            // killough 10/98: implement gradual lighting effects
            // [BH] enhanced to apply effects to all doors
            if (r_graduallighting && !nograduallighting)
                T_GradualLightingToDoor(door);

            if (res == pastdest)
            {
                switch (door->type)
                {
                    case DoorBlazeRaise:
                    case DoorNormal:
                    case GenRaise:
                    case GenBlazeRaise:
                        door->direction = 0;                    // wait at top
                        door->topcountdown = door->topwait;
                        break;

                    case DoorClose30ThenOpen:
                    case DoorBlazeOpen:
                    case DoorOpen:
                    case GenBlazeOpen:
                    case GenOpen:
                    case GenCdO:
                    case GenBlazeCdO:
                        door->sector->ceilingdata = NULL;
                        P_RemoveThinker(&door->thinker);     // unlink and free
                        break;

                    default:
                        break;
                }
            }

            break;
    }
}

//
// EV_DoLockedDoor
// Move a locked door up/down
//
bool EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing, fixed_t speed)
{
    player_t    *player = thing->player;
    static char buffer[1024];

    if (!player)
        return false;

    switch (line->special)
    {
        case SR_Door_Blue_OpenStay_Fast:
        case S1_Door_Blue_OpenStay_Fast:
            if (player->cards[it_bluecard] <= 0 && player->cards[it_blueskull] <= 0)
            {
                // [BH] display player message distinguishing between keycard and skull key
                // [BH] flash needed key on HUD
                if (player->cards[it_blueskull] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardflash || player->neededcard != it_blueskull)
                    {
                        player->neededcard = it_blueskull;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_BLUEO, "You", "", s_PD_SKULLKEY);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_BLUEO, playername, "s", s_PD_SKULLKEY);
                }
                else
                {
                    if (!player->neededcardflash || player->neededcard != it_bluecard)
                    {
                        player->neededcard = it_bluecard;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_BLUEO, "You", "", s_PD_KEYCARD);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_BLUEO, playername, "s", s_PD_KEYCARD);
                }

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);
                S_StartSound(player->mo, sfx_noway);    // [BH] use sfx_noway instead of sfx_oof
                return false;
            }

            break;

        case SR_Door_Red_OpenStay_Fast:
        case S1_Door_Red_OpenStay_Fast:
            if (player->cards[it_redcard] <= 0 && player->cards[it_redskull] <= 0)
            {
                // [BH] display player message distinguishing between keycard and skull key
                // [BH] flash needed key on HUD
                if (player->cards[it_redskull] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardflash || player->neededcard != it_redskull)
                    {
                        player->neededcard = it_redskull;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_REDO, "You", "", s_PD_SKULLKEY);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_REDO, playername, "s", s_PD_SKULLKEY);
                }
                else
                {
                    if (!player->neededcardflash || player->neededcard != it_redcard)
                    {
                        player->neededcard = it_redcard;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_REDO, "You", "", s_PD_KEYCARD);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_REDO, playername, "s", s_PD_KEYCARD);
                }

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);
                S_StartSound(player->mo, sfx_noway);    // [BH] use sfx_noway instead of sfx_oof
                return false;
            }

            break;

        case SR_Door_Yellow_OpenStay_Fast:
        case S1_Door_Yellow_OpenStay_Fast:
            if (player->cards[it_yellowcard] <= 0 && player->cards[it_yellowskull] <= 0)
            {
                // [BH] display player message distinguishing between keycard and skull key
                // [BH] flash needed key on HUD
                if (player->cards[it_yellowskull] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardflash || player->neededcard != it_yellowskull)
                    {
                        player->neededcard = it_yellowskull;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_YELLOWO, "You", "", s_PD_SKULLKEY);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_YELLOWO, playername, "s", s_PD_SKULLKEY);
                }
                else
                {
                    if (!player->neededcardflash || player->neededcard != it_yellowcard)
                    {
                        player->neededcard = it_yellowcard;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_YELLOWO, "You", "", s_PD_KEYCARD);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_YELLOWO, playername, "s", s_PD_KEYCARD);
                }

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);
                S_StartSound(player->mo, sfx_noway);    // [BH] use sfx_noway instead of sfx_oof
                return false;
            }

            break;
    }

    return EV_DoDoor(line, type, speed);
}

bool EV_DoDoor(line_t *line, vldoor_e type, fixed_t speed)
{
    int         secnum = -1;
    bool        rtn = false;
    sector_t    *sec;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
            goto manual_door;
        else
            return false;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        vldoor_t    *door;

        sec = sectors + secnum;

manual_door:
        if (P_SectorActive(ceiling_special, sec))
        {
            if (!zerotag_manual)
                continue;
            else
                return rtn;
        }

        // new door thinker
        rtn = true;
        door = Z_Calloc(1, sizeof(*door), PU_LEVSPEC, NULL);

        door->thinker.function = &T_VerticalDoor;
        P_AddThinker(&door->thinker);

        sec->ceilingdata = door;
        door->sector = sec;
        door->type = type;
        door->topwait = VDOORWAIT;
        door->speed = speed;
        door->line = line;      // jff 01/31/98 remember line that triggered us

        switch (type)
        {
            case DoorBlazeClose:
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                door->direction = -1;

                if (sec->ceilingheight != sec->floorheight)
                    S_StartSectorSound(&door->sector->soundorg, sfx_bdcls);

                break;

            case DoorClose:
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                door->direction = -1;

                if (sec->ceilingheight != sec->floorheight)
                    S_StartSectorSound(&door->sector->soundorg, sfx_dorcls);

                break;

            case DoorClose30ThenOpen:
                door->topheight = sec->ceilingheight;
                door->direction = -1;

                if (sec->ceilingheight != sec->floorheight)
                    S_StartSectorSound(&door->sector->soundorg, sfx_dorcls);

                break;

            case DoorBlazeRaise:
            case DoorBlazeOpen:
                door->direction = 1;
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;

                if (door->topheight != sec->ceilingheight)
                    S_StartSectorSound(&door->sector->soundorg, sfx_bdopn);

                break;

            case DoorNormal:
            case DoorOpen:
                door->direction = 1;
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;

                if (door->topheight != sec->ceilingheight)
                    S_StartSectorSound(&door->sector->soundorg, sfx_doropn);

                break;

            default:
                break;
        }

        // [BH] door is no longer secret
        for (int i = 0; i < sec->linecount; i++)
            sec->lines[i]->flags &= ~ML_SECRET;

        if (zerotag_manual)
            return rtn; // e6y
    }

    return rtn;
}

//
// EV_VerticalDoor
// Open a door manually, no tag value
//
void EV_VerticalDoor(line_t *line, mobj_t *thing)
{
    player_t    *player = thing->player;
    static char buffer[1024];
    sector_t    *sec;
    vldoor_t    *door;

    // if the wrong side of door is pushed, give oof sound
    if (line->sidenum[1] == NO_INDEX && player)     // killough
    {
        if (!autousing && P_DoorClosed(line))
            S_StartSound(player->mo, sfx_noway);    // [BH] use sfx_noway instead of sfx_oof

        return;
    }

    switch (line->special)
    {
        case DR_Door_Blue_OpenWaitClose:
        case D1_Door_Blue_OpenStay:
            if (!player)
                return;

            if (player->cards[it_bluecard] <= 0 && player->cards[it_blueskull] <= 0)
            {
                // [BH] display player message distinguishing between keycard and skull key
                // [BH] flash needed key on HUD
                if (player->cards[it_blueskull] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardflash || player->neededcard != it_blueskull)
                    {
                        player->neededcard = it_blueskull;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_BLUEK, "You", "", s_PD_SKULLKEY);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_BLUEK, playername, "s", s_PD_SKULLKEY);
                }
                else
                {
                    if (!player->neededcardflash || player->neededcard != it_bluecard)
                    {
                        player->neededcard = it_bluecard;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_BLUEK, "You", "", s_PD_KEYCARD);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_BLUEK, playername, "s", s_PD_KEYCARD);
                }

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return;

                HU_PlayerMessage(buffer, false, false);
                S_StartSound(player->mo, sfx_noway);    // [BH] use sfx_noway instead of sfx_oof
                return;
            }

            break;

        case DR_Door_Yellow_OpenWaitClose:
        case D1_Door_Yellow_OpenStay:
            if (!player)
                return;

            if (player->cards[it_yellowcard] <= 0 && player->cards[it_yellowskull] <= 0)
            {
                // [BH] display player message distinguishing between keycard and skull key
                // [BH] flash needed key on HUD
                if (player->cards[it_yellowskull] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardflash || player->neededcard != it_yellowskull)
                    {
                        player->neededcard = it_yellowskull;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_YELLOWK, "You", "", s_PD_SKULLKEY);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_YELLOWK, playername, "s", s_PD_SKULLKEY);
                }
                else
                {
                    if (!player->neededcardflash || player->neededcard != it_yellowcard)
                    {
                        player->neededcard = it_yellowcard;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_YELLOWK, "You", "", s_PD_KEYCARD);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_YELLOWK, playername, "s", s_PD_KEYCARD);
                }

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return;

                HU_PlayerMessage(buffer, false, false);
                S_StartSound(player->mo, sfx_noway);    // [BH] use sfx_noway instead of sfx_oof
                return;
            }

            break;

        case DR_Door_Red_OpenWaitClose:
        case D1_Door_Red_OpenStay:
            if (!player)
                return;

            if (player->cards[it_redcard] <= 0 && player->cards[it_redskull] <= 0)
            {
                // [BH] display player message distinguishing between keycard and skull key
                // [BH] flash needed key on HUD
                if (player->cards[it_redskull] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardflash || player->neededcard != it_redskull)
                    {
                        player->neededcard = it_redskull;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_REDK, "You", "", s_PD_SKULLKEY);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_REDK, playername, "s", s_PD_SKULLKEY);
                }
                else
                {
                    if (!player->neededcardflash || player->neededcard != it_redcard)
                    {
                        player->neededcard = it_redcard;
                        player->neededcardflash = NEEDEDCARDFLASH;
                    }

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), s_PD_REDK, "You", "", s_PD_KEYCARD);
                    else
                        M_snprintf(buffer, sizeof(buffer), s_PD_REDK, playername, "s", s_PD_KEYCARD);
                }

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return;

                HU_PlayerMessage(buffer, false, false);
                S_StartSound(player->mo, sfx_noway);    // [BH] use sfx_noway instead of sfx_oof
                return;
            }

            break;
    }

    sec = sides[line->sidenum[1]].sector;

    if (sec->ceilingdata)
    {
        door = sec->ceilingdata;

        switch (line->special)
        {
            case DR_Door_OpenWaitClose_AlsoMonsters:
            case DR_Door_Blue_OpenWaitClose:
            case DR_Door_Yellow_OpenWaitClose:
            case DR_Door_Red_OpenWaitClose:
            case DR_Door_OpenWaitClose_Fast:
                if (door->direction == -1)
                {
                    door->direction = 1;        // go back up

                    // [BH] play correct door sound
                    S_StartSectorSound(&sec->soundorg, (door->type == DoorBlazeRaise ? sfx_bdopn : sfx_doropn));
                }
                else
                {
                    if (!player)
                        return;

                    if (door->thinker.function == &T_VerticalDoor)
                    {
                        door->direction = -1;   // start going down immediately

                        // [BH] play correct door sound
                        S_StartSectorSound(&sec->soundorg, (door->type == DoorBlazeRaise ? sfx_bdcls : sfx_dorcls));
                    }
                    else if (door->thinker.function == &T_PlatRaise)
                    {
                        plat_t  *plat = (plat_t *)door;

                        plat->wait = -1;
                    }
                    else
                        door->direction = -1;
                }

                return;
        }
    }

    // for proper sound
    switch (line->special)
    {
        case DR_Door_OpenWaitClose_Fast:
        case D1_Door_OpenStay_Fast:
            S_StartSectorSound(&sec->soundorg, sfx_bdopn);
            break;

        default:                // LOCKED DOOR SOUND
            S_StartSectorSound(&sec->soundorg, sfx_doropn);
            break;
    }

    // new door thinker
    door = Z_Calloc(1, sizeof(*door), PU_LEVSPEC, NULL);

    door->thinker.function = &T_VerticalDoor;
    P_AddThinker(&door->thinker);

    sec->ceilingdata = door;
    door->sector = sec;
    door->direction = 1;
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;
    door->line = line;          // jff 01/31/98 remember line that triggered us

    // killough 10/98: use gradual lighting changes if nonzero tag given
    // [BH] check if tag is valid
    if (P_FindLineFromLineTag(line, 0))
        door->lighttag = line->tag;

    switch (line->special)
    {
        case DR_Door_OpenWaitClose_AlsoMonsters:
        case DR_Door_Blue_OpenWaitClose:
        case DR_Door_Yellow_OpenWaitClose:
        case DR_Door_Red_OpenWaitClose:
            door->type = DoorNormal;
            break;

        case D1_Door_OpenStay:
        case D1_Door_Blue_OpenStay:
        case D1_Door_Red_OpenStay:
        case D1_Door_Yellow_OpenStay:
            door->type = DoorOpen;
            line->special = 0;
            break;

        case DR_Door_OpenWaitClose_Fast:
            door->type = DoorBlazeRaise;
            door->speed = VDOORSPEED * 4;
            break;

        case D1_Door_OpenStay_Fast:
            door->type = DoorBlazeOpen;
            line->special = 0;
            door->speed = VDOORSPEED * 4;
            break;

        default:
            door->lighttag = 0; // killough 10/98
            break;
    }

    // find the top and bottom of the movement range
    door->topheight = P_FindLowestCeilingSurrounding(sec) - 4 * FRACUNIT;

    // [BH] door is no longer secret
    for (int i = 0; i < sec->linecount; i++)
        sec->lines[i]->flags &= ~ML_SECRET;
}

//
// Spawn a door that closes after 30 seconds
//
void P_SpawnDoorCloseIn30(sector_t *sec)
{
    vldoor_t    *door = Z_Calloc(1, sizeof(*door), PU_LEVSPEC, NULL);

    door->thinker.function = &T_VerticalDoor;
    P_AddThinker(&door->thinker);

    sec->ceilingdata = door;
    sec->special = 0;
    door->sector = sec;
    door->type = DoorNormal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * TICRATE;
    door->topheight = sec->ceilingheight;
}

//
// Spawn a door that opens after 5 minutes
//
void P_SpawnDoorRaiseIn5Mins(sector_t *sec)
{
    vldoor_t    *door = Z_Calloc(1, sizeof(*door), PU_LEVSPEC, NULL);

    door->thinker.function = &T_VerticalDoor;
    P_AddThinker(&door->thinker);

    sec->ceilingdata = door;
    sec->special = 0;
    door->sector = sec;
    door->direction = 2;
    door->type = DoorRaiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * TICRATE;
}
