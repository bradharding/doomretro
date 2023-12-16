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
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

platlist_t  *activeplats;   // killough 02/14/98: made global again

void T_PlatStay(plat_t *plat) {}

//
// Move a plat up and down
//
void T_PlatRaise(plat_t *plat)
{
    result_e    res;

    switch (plat->status)
    {
        case up:
            res = T_MovePlane(plat->sector, plat->speed, plat->high, plat->crush, 0, 1);

            if ((plat->type == RaiseAndChange || plat->type == RaiseToNearestAndChange) && !(maptime & 7))
                S_StartSectorSound(&plat->sector->soundorg, sfx_stnmov);

            if (res == crushed && !plat->crush)
            {
                plat->count = plat->wait;
                plat->status = down;
                S_StartSectorSound(&plat->sector->soundorg, sfx_pstart);
            }
            else
            {
                if (res == pastdest)
                {
                    // if not an instant toggle type, wait, make plat stop sound
                    if (plat->type != ToggleUpDn)
                    {
                        plat->count = plat->wait;
                        plat->status = waiting;
                        S_StartSectorSound(&plat->sector->soundorg, sfx_pstop);
                    }
                    else // else go into stasis awaiting next toggle activation
                    {
                        plat->oldstatus = plat->status; // jff 03/14/98 after action wait
                        plat->status = in_stasis;       // for reactivation of toggle
                    }

                    switch (plat->type)
                    {
                        case BlazeDWUS:
                        case DownWaitUpStay:
                        case RaiseAndChange:
                        case RaiseToNearestAndChange:
                        case GenLift:
                            P_RemoveActivePlat(plat);
                            break;

                        default:
                            break;
                    }
                }
            }

            break;

        case down:
            res = T_MovePlane(plat->sector, plat->speed, plat->low, false, 0, -1);

            if (res == pastdest)
            {
                // if not an instant toggle, start waiting, make plat stop sound
                if (plat->type != ToggleUpDn)           // jff 03/14/98 toggle up down
                {                                       // is silent, instant, no waiting
                    plat->count = plat->wait;
                    plat->status = waiting;
                    S_StartSectorSound(&plat->sector->soundorg, sfx_pstop);
                }
                else    // instant toggles go into stasis awaiting next activation
                {
                    plat->oldstatus = plat->status;     // jff 03/14/98 after action wait
                    plat->status = in_stasis;           // for reactivation of toggle
                }

                // jff 01/26/98 remove the plat if it bounced so it can be tried again
                // only affects plats that raise and bounce
                switch (plat->type)
                {
                    case RaiseAndChange:
                    case RaiseToNearestAndChange:
                        P_RemoveActivePlat(plat);

                    default:
                        break;
                }
            }

            break;

        case waiting:
            if (!--plat->count)
            {
                plat->status = (plat->sector->floorheight == plat->low ? up : down);
                S_StartSectorSound(&plat->sector->soundorg, sfx_pstart);
            }

            break;

        case in_stasis:
            break;
    }
}

//
// Do Platforms
//  "amount" is only used for SOME platforms.
//
bool EV_DoPlat(line_t *line, plattype_e type, int amount)
{
    plat_t      *plat;
    int         secnum = -1;
    bool        rtn = false;
    sector_t    *sec;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
            goto manual_plat;
        else
            return false;
    }

    // Activate all <type> plats that are in_stasis
    switch (type)
    {
        case PerpetualRaise:
            P_ActivateInStasis(line->tag);
            break;

        case ToggleUpDn:
            P_ActivateInStasis(line->tag);
            rtn = true;
            break;

        default:
            break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = sectors + secnum;

manual_plat:
        if (P_SectorActive(floor_special, sec))
        {
            if (!zerotag_manual)
                continue;
            else
                return rtn;
        }

        // Find lowest and highest floors around sector
        rtn = true;
        plat = Z_Calloc(1, sizeof(*plat), PU_LEVSPEC, NULL);

        plat->thinker.function = &T_PlatRaise;
        P_AddThinker(&plat->thinker);

        plat->type = type;
        plat->sector = sec;
        plat->sector->floordata = plat;
        plat->tag = line->tag;
        plat->low = sec->floorheight;

        switch (type)
        {
            case RaiseToNearestAndChange:
                plat->speed = PLATSPEED / 2;
                sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
                P_CheckTerrainType(sec);
                plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
                plat->status = up;
                sec->special = 0;
                S_StartSectorSound(&sec->soundorg, sfx_stnmov);
                break;

            case RaiseAndChange:
                plat->speed = PLATSPEED / 2;
                sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
                P_CheckTerrainType(sec);
                plat->high = sec->floorheight + amount * FRACUNIT;
                plat->status = up;
                S_StartSectorSound(&sec->soundorg, sfx_stnmov);
                break;

            case DownWaitUpStay:
                plat->speed = PLATSPEED * 4;
                plat->low = MIN(P_FindLowestFloorSurrounding(sec), sec->floorheight);
                plat->high = sec->floorheight;
                plat->wait = TICRATE * PLATWAIT;
                plat->status = down;
                S_StartSectorSound(&sec->soundorg, sfx_pstart);
                break;

            case BlazeDWUS:
                plat->speed = PLATSPEED * 8;
                plat->low = MIN(P_FindLowestFloorSurrounding(sec), sec->floorheight);
                plat->high = sec->floorheight;
                plat->wait = TICRATE * PLATWAIT;
                plat->status = down;
                S_StartSectorSound(&sec->soundorg, sfx_pstart);
                break;

            case PerpetualRaise:
                plat->speed = PLATSPEED;
                plat->low = MIN(P_FindLowestFloorSurrounding(sec), sec->floorheight);
                plat->high = MAX(sec->floorheight, P_FindHighestFloorSurrounding(sec));
                plat->wait = TICRATE * PLATWAIT;
                plat->status = (plat_e)(M_Random() & 1);
                S_StartSectorSound(&sec->soundorg, sfx_pstart);
                break;

            case ToggleUpDn:                        // jff 03/14/98 add new type to support instant toggle
                plat->speed = PLATSPEED;            // not used
                plat->wait = TICRATE * PLATWAIT;    // not used
                plat->crush = true;                 // jff 03/14/98 crush anything in the way

                // set up toggling between ceiling, floor inclusive
                plat->low = sec->ceilingheight;
                plat->high = sec->floorheight;
                plat->status = down;
                break;

            default:
                break;
        }

        P_AddActivePlat(plat);

        // [BH] plat is no longer secret
        for (int i = 0; i < sec->linecount; i++)
            sec->lines[i]->flags &= ~ML_SECRET;

        if (zerotag_manual)
            return rtn; // e6y
    }

    return rtn;
}

// The following were all rewritten by Lee Killough
// to use the new structure which places no limits
// on active plats. It also avoids spending as much
// time searching for active plats. Previously a
// fixed-size array was used, with NULL indicating
// empty entries, while now a doubly-linked list
// is used.

//
// P_ActivateInStasis
// Activate a plat that has been put in stasis
// (stopped perpetual floor, instant floor/ceiling toggle)
//
void P_ActivateInStasis(int tag)
{
    for (platlist_t *platlist = activeplats; platlist; platlist = platlist->next)   // search the active plats
    {
        plat_t  *plat = platlist->plat;                 // for one in stasis with right tag

        if (plat->tag == tag && plat->status == in_stasis)
        {
            plat->status = (plat->type == ToggleUpDn ? (plat->oldstatus == up ? down : up) : plat->oldstatus);
            plat->thinker.function = &T_PlatRaise;
        }
    }
}

//
// EV_StopPlat
// Handler for "stop perpetual floor" linedef type
//
bool EV_StopPlat(const line_t *line)
{
    for (platlist_t *platlist = activeplats; platlist; platlist = platlist->next)   // search the active plats
    {
        plat_t  *plat = platlist->plat;                 // for one with the tag not in stasis

        if (plat->status != in_stasis && plat->tag == line->tag)
        {
            plat->oldstatus = plat->status;             // put it in stasis
            plat->status = in_stasis;
            plat->thinker.function = &T_PlatStay;
        }
    }

    return true;
}

//
// P_AddActivePlat
// Add a plat to the head of the active plat list
//
void P_AddActivePlat(plat_t *plat)
{
    platlist_t  *list = malloc(sizeof(*list));

    if (list)
    {
        list->plat = plat;
        plat->list = list;

        if ((list->next = activeplats))
            list->next->prev = &list->next;

        list->prev = &activeplats;
        activeplats = list;
    }
}

//
// P_RemoveActivePlat
// Remove a plat from the active plat list
//
void P_RemoveActivePlat(plat_t *plat)
{
    platlist_t  *list = plat->list;

    plat->sector->floordata = NULL;
    P_RemoveThinker2(&plat->thinker);

    if ((*list->prev = list->next))
        list->next->prev = list->prev;

    free(list);
}

//
// P_RemoveAllActivePlats
// Remove all plats from the active plat list
//
void P_RemoveAllActivePlats(void)
{
    while (activeplats)
    {
        platlist_t  *next = activeplats->next;

        free(activeplats);
        activeplats = next;
    }
}
