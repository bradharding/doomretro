/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"

plat_t *activeplatshead;

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

            if (plat->type == raiseAndChange || plat->type == raiseToNearestAndChange)
            {
                if (!(leveltime & 7) && plat->sector->floorheight != plat->high)
                    S_StartSound(&plat->sector->soundorg, sfx_stnmov);
            }

            if (res == crushed && !plat->crush)
            {
                plat->count = plat->wait;
                plat->status = down;
                S_StartSound(&plat->sector->soundorg, sfx_pstart);
            }
            else
            {
                if (res == pastdest)
                {
                    plat->count = plat->wait;
                    plat->status = waiting;
                    S_StartSound(&plat->sector->soundorg, sfx_pstop);

                    switch (plat->type)
                    {
                        case blazeDWUS:
                        case downWaitUpStay:
                        case raiseAndChange:
                        case raiseToNearestAndChange:
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
                plat->count = plat->wait;
                plat->status = waiting;
                S_StartSound(&plat->sector->soundorg, sfx_pstop);
            }
            break;

        case waiting:
            if (!--plat->count)
            {
                plat->status = (plat->sector->floorheight == plat->low ? up : down);
                S_StartSound(&plat->sector->soundorg, sfx_pstart);
            }

        case in_stasis:
            break;
    }
}

//
// Do Platforms
//  "amount" is only used for SOME platforms.
//
int EV_DoPlat(line_t *line, plattype_e type, int amount)
{
    plat_t      *plat;
    int         secnum = -1;
    int         rtn = 0;
    sector_t    *sec = NULL;

    //  Activate all <type> plats that are in_stasis
    switch (type)
    {
        case perpetualRaise:
            P_ActivateInStasis(line->tag);
            break;

        default:
            break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];

        if (sec->specialdata)
            continue;

        // Find lowest & highest floors around sector
        rtn = 1;
        plat = (plat_t *)Z_Malloc(sizeof(*plat), PU_LEVSPEC, 0);
        P_AddThinker(&plat->thinker);

        plat->type = type;
        plat->sector = sec;
        plat->sector->specialdata = plat;
        plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;
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

                S_StartSound(&sec->soundorg, sfx_stnmov);
                break;

            case raiseAndChange:
                plat->speed = PLATSPEED / 2;
                sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
                plat->high = sec->floorheight + amount * FRACUNIT;
                plat->wait = 0;
                plat->status = up;

                S_StartSound(&sec->soundorg, sfx_stnmov);
                break;

            case downWaitUpStay:
                plat->speed = PLATSPEED * 4;
                plat->low = P_FindLowestFloorSurrounding(sec);

                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;

                plat->high = sec->floorheight;
                plat->wait = TICRATE * PLATWAIT;
                plat->status = down;
                S_StartSound(&sec->soundorg, sfx_pstart);
                break;

            case blazeDWUS:
                plat->speed = PLATSPEED * 8;
                plat->low = P_FindLowestFloorSurrounding(sec);

                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;

                plat->high = sec->floorheight;
                plat->wait = TICRATE * PLATWAIT;
                plat->status = down;
                S_StartSound(&sec->soundorg, sfx_pstart);
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
                plat->status = (plat_e)(P_Random() & 1);

                S_StartSound(&sec->soundorg, sfx_pstart);
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

void P_ActivateInStasis(int tag)
{
    plat_t *plat;

    for (plat = activeplatshead; plat; plat = plat->next)
        if (plat->tag == tag && plat->status == in_stasis)
        {
            plat->status = plat->oldstatus;
            plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;
        }
}

int EV_StopPlat(line_t *line)
{
    plat_t *plat;

    for (plat = activeplatshead; plat; plat = plat->next)
        if (plat->tag == line->tag && plat->status != in_stasis)
        {
            plat->oldstatus = plat->status;
            plat->status = in_stasis;
            plat->thinker.function.acv = (actionf_v)NULL;

            return 1;
        }

    return 0;
}

void P_AddActivePlat(plat_t *plat)
{
    plat_t *next = activeplatshead;

    if (next)
        next->prev = plat;

    activeplatshead = plat;
    plat->next = next;
    plat->prev = NULL;
}

void P_RemoveActivePlat(plat_t *plat)
{
    plat_t      *next = plat->next;
    plat_t      *prev = plat->prev;

    if (next)
        next->prev = prev;

    if (!prev)
        activeplatshead = next;
    else
        prev->next = next;

    plat->sector->specialdata = NULL;
    P_RemoveThinker(&plat->thinker);
}
