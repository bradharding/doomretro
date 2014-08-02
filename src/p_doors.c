/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#include "dstrings.h"
#include "p_local.h"
#include "s_sound.h"
#include "z_zone.h"

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
            {
                switch (door->type)
                {
                    case blazeRaise:
                        door->direction = -1;   // time to go back down
                        S_StartSound(&door->sector->soundorg, sfx_bdcls);
                        break;

                    case normal:
                        door->direction = -1;   // time to go back down
                        S_StartSound(&door->sector->soundorg, sfx_dorcls);
                        break;

                    case close30ThenOpen:
                        door->direction = 1;
                        S_StartSound(&door->sector->soundorg, sfx_doropn);
                        break;

                    default:
                        break;
                }
            }
            break;

        case 2:
            //  INITIAL WAIT
            if (!--door->topcountdown)
            {
                switch (door->type)
                {
                    case raiseIn5Mins:
                        door->direction = 1;
                        door->type = normal;
                        S_StartSound(&door->sector->soundorg, sfx_doropn);
                        break;

                    default:
                        break;
                }
            }
            break;

        case -1:
            // DOWN
            res = T_MovePlane(door->sector, door->speed, door->sector->floorheight,
                              false, 1, door->direction);
            if (res == pastdest)
            {
                switch (door->type)
                {
                    case blazeRaise:
                    case blazeClose:
                        door->sector->specialdata = NULL;
                        P_RemoveThinker(&door->thinker);        // unlink and free
                        break;

                    case normal:
                    case close:
                        door->sector->specialdata = NULL;
                        P_RemoveThinker(&door->thinker);        // unlink and free
                        break;

                    case close30ThenOpen:
                        door->direction = 0;
                        door->topcountdown = TICRATE * 30;
                        break;

                    default:
                        break;
                }
            }
            else if (res == crushed)
            {
                switch (door->type)
                {
                    case blazeClose:
                    case close:         // DO NOT GO BACK UP!
                        break;

                    case blazeRaise:
                        door->direction = 1;
                        S_StartSound(&door->sector->soundorg, sfx_bdopn);
                        break;

                    default:
                        door->direction = 1;
                        S_StartSound(&door->sector->soundorg, sfx_doropn);
                        break;
                }
            }
            break;

        case 1:
            // UP
            res = T_MovePlane(door->sector, door->speed, door->topheight,
                              false, 1, door->direction);
            if (res == pastdest)
            {
                switch (door->type)
                {
                    case blazeRaise:
                    case normal:
                        door->direction = 0;                    // wait at top
                        door->topcountdown = door->topwait;
                        break;

                    case close30ThenOpen:
                    case blazeOpen:
                    case open:
                        door->sector->specialdata = NULL;
                        P_RemoveThinker(&door->thinker);        // unlink and free
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
int EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing)
{
    player_t    *player = thing->player;

    if (!player)
        return 0;

    switch (line->special)
    {
        case SR_OpenFastDoorStayOpenBlueKeyRequired:
        case S1_OpenFastDoorStayOpenBlueKeyRequired:
            if (player->cards[it_bluecard] <= 0 && player->cards[it_blueskull] <= 0)
            {
                if (player->cards[it_bluecard] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardtics || player->neededcard != it_bluecard)
                    {
                        player->neededcard = it_bluecard;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_BLUEO;
                }
                else
                {
                    if (!player->neededcardtics || player->neededcard != it_blueskull)
                    {
                        player->neededcard = it_blueskull;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_BLUEO2;
                }
                S_StartSound(player->mo, sfx_noway);
                return 0;
            }
            break;

        case SR_OpenFastDoorStayOpenRedKeyRequired:
        case S1_OpenFastDoorStayOpenRedKeyRequired:
            if (player->cards[it_redcard] <= 0 && player->cards[it_redskull] <= 0)
            {
                if (player->cards[it_redcard] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardtics || player->neededcard != it_redcard)
                    {
                        player->neededcard = it_redcard;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_REDO;
                }
                else
                {
                    if (!player->neededcardtics || player->neededcard != it_redskull)
                    {
                        player->neededcard = it_redskull;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_REDO2;
                }
                S_StartSound(player->mo, sfx_noway);
                return 0;
            }
            break;

        case SR_OpenFastDoorStayOpenYellowKeyRequired:
        case S1_OpenFastDoorStayOpenYellowKeyRequired:
            if (player->cards[it_yellowcard] <= 0 && player->cards[it_yellowskull] <= 0)
            {
                if (player->cards[it_yellowcard] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardtics || player->neededcard != it_yellowcard)
                    {
                        player->neededcard = it_yellowcard;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_YELLOWO;
                }
                else
                {
                    if (!player->neededcardtics || player->neededcard != it_yellowskull)
                    {
                        player->neededcard = it_yellowskull;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_YELLOWO2;
                }
                S_StartSound(player->mo, sfx_noway);
                return 0;
            }
            break;
    }

    return EV_DoDoor(line, type);
}

int EV_DoDoor(line_t *line, vldoor_e type)
{
    int         secnum = -1;
    int         rtn = 0;
    int         i;
    sector_t    *sec;
    vldoor_t    *door;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
            continue;

        // new door thinker
        rtn = 1;
        door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
        P_AddThinker(&door->thinker);
        sec->specialdata = door;

        door->thinker.function.acp1 = T_VerticalDoor;
        door->sector = sec;
        door->type = type;
        door->topwait = VDOORWAIT;
        door->speed = VDOORSPEED;

        for (i = 0; i < door->sector->linecount; i++)
            door->sector->lines[i]->flags &= ~ML_SECRET;

        switch (type)
        {
            case blazeClose:
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                door->direction = -1;
                door->speed = VDOORSPEED * 4;
                S_StartSound(&door->sector->soundorg, sfx_bdcls);
                break;

            case close:
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                door->direction = -1;
                S_StartSound(&door->sector->soundorg, sfx_dorcls);
                break;

            case close30ThenOpen:
                door->topheight = sec->ceilingheight;
                door->direction = -1;
                S_StartSound(&door->sector->soundorg, sfx_dorcls);
                break;

            case blazeRaise:
            case blazeOpen:
                door->direction = 1;
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                door->speed = VDOORSPEED * 4;
                if (door->topheight != sec->ceilingheight)
                    S_StartSound(&door->sector->soundorg, sfx_bdopn);
                break;

            case normal:
            case open:
                door->direction = 1;
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                if (door->topheight != sec->ceilingheight)
                    S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;

            default:
                break;
        }
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
    sector_t    *sec;
    vldoor_t    *door;
    int         i;

    switch (line->special)
    {
        case DR_OpenDoorWait4SecondsCloseBlueKeyRequired:
        case D1_OpenDoorStayOpenBlueKeyRequired:
            if (!player)
                return;

            if (player->cards[it_bluecard] <= 0 && player->cards[it_blueskull] <= 0)
            {
                if (player->cards[it_bluecard] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardtics || player->neededcard != it_bluecard)
                    {
                        player->neededcard = it_bluecard;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_BLUEK;
                }
                else
                {
                    if (!player->neededcardtics || player->neededcard != it_blueskull)
                    {
                        player->neededcard = it_blueskull;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_BLUEK2;
                }
                S_StartSound(player->mo, sfx_noway);
                return;
            }
            break;

        case DR_OpenDoorWait4SecondsCloseYellowKeyRequired:
        case D1_OpenDoorStayOpenYellowKeyRequired:
            if (!player)
                return;

            if (player->cards[it_yellowcard] <= 0 && player->cards[it_yellowskull] <= 0)
            {
                if (player->cards[it_yellowcard] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardtics || player->neededcard != it_yellowcard)
                    {
                        player->neededcard = it_yellowcard;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_YELLOWK;
                }
                else
                {
                    if (!player->neededcardtics || player->neededcard != it_yellowskull)
                    {
                        player->neededcard = it_yellowskull;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_YELLOWK2;
                }
                S_StartSound(player->mo, sfx_noway);
                return;
            }
            break;

        case DR_OpenDoorWait4SecondsCloseRedKeyRequired:
        case D1_OpenDoorStayOpenRedKeyRequired:
            if (!player)
                return;

            if (player->cards[it_redcard] <= 0 && player->cards[it_redskull] <= 0)
            {
                if (player->cards[it_redcard] == CARDNOTFOUNDYET)
                {
                    if (!player->neededcardtics || player->neededcard != it_redcard)
                    {
                        player->neededcard = it_redcard;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_REDK;
                }
                else
                {
                    if (!player->neededcardtics || player->neededcard != it_redskull)
                    {
                        player->neededcard = it_redskull;
                        player->neededcardtics = NEEDEDCARDTICS;
                    }
                    player->message = PD_REDK2;
                }
                S_StartSound(player->mo, sfx_noway);
                return;
            }
            break;
    }

    // if the wrong side of door is pushed, give oof sound
    if (line->sidenum[1] == NO_INDEX)           // killough
    {
        S_StartSound(player->mo, sfx_noway);    // killough 3/20/98
        return;
    }

    sec = sides[line->sidenum[1]].sector;

    if (sec->specialdata)
    {
        door = (vldoor_t *)sec->specialdata;
        switch (line->special)
        {
            case DR_OpenDoorWait4SecondsClose:
            case DR_OpenDoorWait4SecondsCloseBlueKeyRequired:
            case DR_OpenDoorWait4SecondsCloseYellowKeyRequired:
            case DR_OpenDoorWait4SecondsCloseRedKeyRequired:
            case DR_OpenFastDoorWait4SecondsClose:
                if (door->direction == -1)
                {
                    door->direction = 1;        // go back up

                    if (door->type == blazeRaise)
                        S_StartSound(&door->sector->soundorg, sfx_bdopn);
                    else
                        S_StartSound(&door->sector->soundorg, sfx_doropn);
                }
                else
                {
                    if (!thing->player)
                        return;

                    if (door->thinker.function.acp1 == T_VerticalDoor)
                    {
                        door->direction = -1;   // start going down immediately

                        if (door->type == blazeRaise)
                            S_StartSound(&door->sector->soundorg, sfx_bdcls);
                        else
                            S_StartSound(&door->sector->soundorg, sfx_dorcls);
                    }
                    else if (door->thinker.function.acp1 == T_PlatRaise)
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
        case DR_OpenFastDoorWait4SecondsClose:
        case D1_OpenFastDoorStayOpen:
            S_StartSound(&sec->soundorg, sfx_bdopn);
            break;

        case DR_OpenDoorWait4SecondsClose:
        case D1_OpenDoorStayOpen:
            S_StartSound(&sec->soundorg, sfx_doropn);
            break;

        default:        // LOCKED DOOR SOUND
            S_StartSound(&sec->soundorg, sfx_doropn);
            break;
    }

    // new door thinker
    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker(&door->thinker);
    sec->specialdata = door;
    door->thinker.function.acp1 = T_VerticalDoor;
    door->sector = sec;
    door->direction = 1;
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;

    switch (line->special)
    {
        case DR_OpenDoorWait4SecondsClose:
        case DR_OpenDoorWait4SecondsCloseBlueKeyRequired:
        case DR_OpenDoorWait4SecondsCloseYellowKeyRequired:
        case DR_OpenDoorWait4SecondsCloseRedKeyRequired:
            door->type = normal;
            break;

        case D1_OpenDoorStayOpen:
        case D1_OpenDoorStayOpenBlueKeyRequired:
        case D1_OpenDoorStayOpenRedKeyRequired:
        case D1_OpenDoorStayOpenYellowKeyRequired:
            door->type = open;
            line->special = 0;
            break;

        case DR_OpenFastDoorWait4SecondsClose:
            door->type = blazeRaise;
            door->speed = VDOORSPEED * 4;
            break;

        case D1_OpenFastDoorStayOpen:
            door->type = blazeOpen;
            line->special = 0;
            door->speed = VDOORSPEED * 4;
            break;
    }

    // find the top and bottom of the movement range
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;

    for (i = 0; i < sec->linecount; i++)
        sec->lines[i]->flags &= ~ML_SECRET;
}

//
// Spawn a door that closes after 30 seconds
//
void P_SpawnDoorCloseIn30(sector_t *sec)
{
    vldoor_t    *door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);

    P_AddThinker(&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * TICRATE;
}

//
// Spawn a door that opens after 5 minutes
//
void P_SpawnDoorRaiseIn5Mins(sector_t *sec)
{
    vldoor_t    *door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);

    P_AddThinker(&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * TICRATE;
}
