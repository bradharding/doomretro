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

#include "doomstat.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

//
// CEILINGS
//

ceiling_t       *activeceilingshead;

extern boolean  canmodify;

//
// T_MoveCeiling
//
void T_MoveCeiling(ceiling_t *ceiling)
{
    result_e    res;

    switch (ceiling->direction)
    {
        case 0:
            // IN STASIS
            break;

        case 1:
            // UP
            res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->topheight,
                              false, 1, ceiling->direction);

            if (!(leveltime & 7) && ceiling->sector->ceilingheight != ceiling->topheight)
                if (ceiling->type != silentCrushAndRaise)
                    S_StartSound(&ceiling->sector->soundorg, sfx_stnmov);

            if (res == pastdest)
            {
                switch (ceiling->type)
                {
                    case raiseToHighest:
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    case silentCrushAndRaise:
                        S_StartSound(&ceiling->sector->soundorg, sfx_pstop);

                    case fastCrushAndRaise:
                    case crushAndRaise:
                        ceiling->direction = -1;
                        break;

                    default:
                        break;
                }
            }
            break;

        case -1:
            // DOWN
            res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->bottomheight,
                              ceiling->crush, 1, ceiling->direction);

            if (!(leveltime & 7) && ceiling->sector->ceilingheight != ceiling->bottomheight)
                if (ceiling->type != silentCrushAndRaise)
                    S_StartSound(&ceiling->sector->soundorg, sfx_stnmov);

            if (res == pastdest)
            {
                switch (ceiling->type)
                {
                    case silentCrushAndRaise:
                        S_StartSound(&ceiling->sector->soundorg, sfx_pstop);

                    case crushAndRaise:
                        ceiling->speed = CEILSPEED;

                    case fastCrushAndRaise:
                        ceiling->direction = 1;
                        break;

                    case lowerAndCrush:
                    case lowerToFloor:
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    default:
                        break;
                }
            }
            else
            {
                if (res == crushed)
                {
                    switch (ceiling->type)
                    {
                        case silentCrushAndRaise:
                        case crushAndRaise:
                        case lowerAndCrush:
                            ceiling->speed = CEILSPEED / 8;
                            break;

                        default:
                            break;
                    }
                }
            }
            break;
    }
}

//
// EV_DoCeiling
// Move a ceiling up/down and all around!
//
boolean EV_DoCeiling(line_t *line, ceiling_e type)
{
    int         secnum = -1;
    boolean     rtn = false;
    sector_t    *sec;
    ceiling_t   *ceiling;

    // Reactivate in-stasis ceilings...for certain types.
    switch (type)
    {
        case fastCrushAndRaise:
        case silentCrushAndRaise:
        case crushAndRaise:
            rtn = P_ActivateInStasisCeiling(line);

        default:
            break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
            continue;

        // new door thinker
        rtn = true;
        ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVSPEC, 0);
        memset(ceiling, 0, sizeof(*ceiling));
        P_AddThinker(&ceiling->thinker);
        sec->specialdata = ceiling;
        ceiling->thinker.function = T_MoveCeiling;
        ceiling->sector = sec;
        ceiling->crush = false;

        switch (type)
        {
            case fastCrushAndRaise:
                ceiling->crush = true;
                ceiling->topheight = sec->ceilingheight;
                ceiling->bottomheight = sec->floorheight + 8 * FRACUNIT;
                ceiling->direction = -1;
                ceiling->speed = CEILSPEED * 2;
                break;

            case silentCrushAndRaise:
            case crushAndRaise:
                ceiling->crush = true;
                ceiling->topheight = sec->ceilingheight;

            case lowerAndCrush:
            case lowerToFloor:
                ceiling->bottomheight = sec->floorheight;
                if (type != lowerToFloor && !(gamemission == doom2 && gamemap == 4 && canmodify))
                    ceiling->bottomheight += 8 * FRACUNIT;
                ceiling->direction = -1;
                ceiling->speed = CEILSPEED;
                break;

            case raiseToHighest:
                ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
                ceiling->direction = 1;
                ceiling->speed = CEILSPEED;
                break;
        }

        ceiling->tag = sec->tag;
        ceiling->type = type;
        P_AddActiveCeiling(ceiling);
    }
    return rtn;
}

//
// P_AddActiveCeiling
// Add an active ceiling
//
void P_AddActiveCeiling(ceiling_t *c)
{
    ceiling_t   *next = activeceilingshead;

    if (next)
        next->prev = c;
    activeceilingshead = c;
    c->next = next;
    c->prev = NULL;
}

//
// P_RemoveActiveCeiling
// Remove a ceiling's thinker
//
void P_RemoveActiveCeiling(ceiling_t *c)
{
    ceiling_t   *next = c->next;
    ceiling_t   *prev = c->prev;

    if (next)
        next->prev = prev;

    if (!prev)
        activeceilingshead = next;
    else
        prev->next = next;

    c->sector->specialdata = NULL;
    P_RemoveThinker(&c->thinker);
}

//
// P_ActivateInStasisCeiling
// Restart a ceiling that's in-stasis
//
boolean P_ActivateInStasisCeiling(line_t *line)
{
    boolean     rtn = false;
    ceiling_t   *ceiling;

    for (ceiling = activeceilingshead; ceiling; ceiling = ceiling->next)
        if (ceiling->tag == line->tag && ceiling->thinker.function == NULL)
        {
            ceiling->thinker.function = T_MoveCeiling;
            rtn = true;
        }

    return rtn;
}

//
// EV_CeilingCrushStop
// Stop a ceiling from crushing!
//
boolean EV_CeilingCrushStop(line_t *line)
{
    boolean     rtn = false;
    ceiling_t   *ceiling;

    for (ceiling = activeceilingshead; ceiling; ceiling = ceiling->next)
        if (ceiling->tag == line->tag && ceiling->thinker.function != NULL)
        {
            ceiling->thinker.function = NULL;
            rtn = true;
        }

    return rtn;
}
