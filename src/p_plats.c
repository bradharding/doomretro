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

#include "doomstat.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

platlist_t  *activeplats;   // killough 2/14/98: made global again

//
// Move a plat up and down
//
void T_PlatRaise(plat_t *plat)
{
    result_e    res;

    if (freeze)
        return;

    switch (plat->status)
    {
        case up:
            res = T_MovePlane(plat->sector, plat->speed, plat->high, plat->crush, 0, 1);

            if (plat->type == raiseAndChange || plat->type == raiseToNearestAndChange)
                if (!(leveltime & 7) && plat->sector->floorheight != plat->high)
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
                    if (plat->type != toggleUpDn)
                    {
                        plat->count = plat->wait;
                        plat->status = waiting;
                        S_StartSectorSound(&plat->sector->soundorg, sfx_pstop);
                    }
                    else // else go into stasis awaiting next toggle activation
                    {
                        plat->oldstatus = plat->status; // jff 3/14/98 after action wait
                        plat->status = in_stasis;       // for reactivation of toggle
                    }

                    switch (plat->type)
                    {
                        case blazeDWUS:
                        case downWaitUpStay:
                        case raiseAndChange:
                        case raiseToNearestAndChange:
                        case genLift:
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
                if (plat->type != toggleUpDn)           // jff 3/14/98 toggle up down
                {                                       // is silent, instant, no waiting
                    plat->count = plat->wait;
                    plat->status = waiting;
                    S_StartSectorSound(&plat->sector->soundorg, sfx_pstop);
                }
                else    // instant toggles go into stasis awaiting next activation
                {
                    plat->oldstatus = plat->status;     // jff 3/14/98 after action wait
                    plat->status = in_stasis;           // for reactivation of toggle
                }

                // jff 1/26/98 remove the plat if it bounced so it can be tried again
                // only affects plats that raise and bounce
                switch (plat->type)
                {
                    case raiseAndChange:
                    case raiseToNearestAndChange:
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
dboolean EV_DoPlat(line_t *line, plattype_e type, int amount)
{
    plat_t      *plat;
    int         secnum = -1;
    dboolean    rtn = false;
    sector_t    *sec = NULL;

    // Activate all <type> plats that are in_stasis
    switch (type)
    {
        case perpetualRaise:
            P_ActivateInStasis(line->tag);
            break;

        case toggleUpDn:
            P_ActivateInStasis(line->tag);
            rtn = true;
            break;

        default:
            break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];

        if (P_SectorActive(floor_special, sec))
            continue;

        // Find lowest & highest floors around sector
        rtn = true;
        plat = Z_Calloc(1, sizeof(*plat), PU_LEVSPEC, NULL);
        P_AddThinker(&plat->thinker);

        plat->type = type;
        plat->sector = sec;
        plat->sector->floordata = plat;
        plat->thinker.function = T_PlatRaise;
        plat->crush = false;
        plat->tag = line->tag;
        plat->low = sec->floorheight;

        switch (type)
        {
            case raiseToNearestAndChange:
                plat->speed = PLATSPEED / 2;
                sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
                plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
                plat->wait = 0;
                plat->status = up;
                sec->special = 0;
                S_StartSectorSound(&sec->soundorg, sfx_stnmov);
                break;

            case raiseAndChange:
                plat->speed = PLATSPEED / 2;
                sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
                plat->high = sec->floorheight + amount * FRACUNIT;
                plat->wait = 0;
                plat->status = up;
                S_StartSectorSound(&sec->soundorg, sfx_stnmov);
                break;

            case downWaitUpStay:
                plat->speed = PLATSPEED * 4;
                plat->low = P_FindLowestFloorSurrounding(sec);

                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;

                plat->high = sec->floorheight;
                plat->wait = TICRATE * PLATWAIT;
                plat->status = down;
                S_StartSectorSound(&sec->soundorg, sfx_pstart);
                break;

            case blazeDWUS:
                plat->speed = PLATSPEED * 8;
                plat->low = P_FindLowestFloorSurrounding(sec);

                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;

                plat->high = sec->floorheight;
                plat->wait = TICRATE * PLATWAIT;
                plat->status = down;
                S_StartSectorSound(&sec->soundorg, sfx_pstart);
                break;

            case perpetualRaise:
                plat->speed = PLATSPEED;
                plat->low = P_FindLowestFloorSurrounding(sec);

                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;

                plat->high = P_FindHighestFloorSurrounding(sec);

                if (plat->high < sec->floorheight)
                    plat->high = sec->floorheight;

                plat->wait = TICRATE * PLATWAIT;
                plat->status = (plat_e)(M_Random() & 1);
                S_StartSectorSound(&sec->soundorg, sfx_pstart);
                break;

            case toggleUpDn:                        // jff 3/14/98 add new type to support instant toggle
                plat->speed = PLATSPEED;            // not used
                plat->wait = TICRATE * PLATWAIT;    // not used
                plat->crush = true;                 // jff 3/14/98 crush anything in the way

                // set up toggling between ceiling, floor inclusive
                plat->low = sec->ceilingheight;
                plat->high = sec->floorheight;
                plat->status = down;
                break;

            default:
                break;
        }

        P_AddActivePlat(plat);
    }

    if (sec)
    {
        int i;

        for (i = 0; i < sec->linecount; i++)
            sec->lines[i]->flags &= ~ML_SECRET;
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
// P_ActivateInStasis()
// Activate a plat that has been put in stasis
// (stopped perpetual floor, instant floor/ceil toggle)
//
void P_ActivateInStasis(int tag)
{
    platlist_t  *platlist;

    for (platlist = activeplats; platlist; platlist = platlist->next)   // search the active plats
    {
        plat_t  *plat = platlist->plat;                 // for one in stasis with right tag

        if (plat->tag == tag && plat->status == in_stasis)
        {
            if (plat->type == toggleUpDn)
                plat->status = (plat->oldstatus == up ? down : up);
            else
                plat->status = plat->oldstatus;

            plat->thinker.function = T_PlatRaise;
        }
    }
}

//
// EV_StopPlat()
// Handler for "stop perpetual floor" linedef type
//
dboolean EV_StopPlat(line_t *line)
{
    platlist_t  *platlist;

    for (platlist = activeplats; platlist; platlist = platlist->next)   // search the active plats
    {
        plat_t  *plat = platlist->plat;                 // for one with the tag not in stasis

        if (plat->status != in_stasis && plat->tag == line->tag)
        {
            plat->oldstatus = plat->status;             // put it in stasis
            plat->status = in_stasis;
            plat->thinker.function = NULL;
        }
    }
    return true;
}

//
// P_AddActivePlat()
// Add a plat to the head of the active plat list
//
void P_AddActivePlat(plat_t *plat)
{
    platlist_t  *list = malloc(sizeof(*list));

    list->plat = plat;
    plat->list = list;

    if ((list->next = activeplats))
        list->next->prev = &list->next;

    list->prev = &activeplats;
    activeplats = list;
}

//
// P_RemoveActivePlat()
// Remove a plat from the active plat list
//
void P_RemoveActivePlat(plat_t *plat)
{
    platlist_t  *list = plat->list;

    plat->sector->floordata = NULL;
    P_RemoveThinker(&plat->thinker);

    if ((*list->prev = list->next))
        list->next->prev = list->prev;

    free(list);
}

//
// P_RemoveAllActivePlats()
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
