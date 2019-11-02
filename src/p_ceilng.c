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

#include "doomstat.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

// the list of ceilings moving currently, including crushers
ceilinglist_t   *activeceilings;

static void T_GradualLightingToCeiling(ceiling_t *ceiling)
{
    sector_t    *sector = ceiling->sector;
    fixed_t     level = ceiling->topheight - sector->floorheight;

    if (level > 0 && !islightspecial[sector->special] && sector->ceilingpic != skyflatnum)
        EV_LightByAdjacentSectors(sector, FixedDiv(sector->ceilingheight - sector->floorheight, level));
}

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
            res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->topheight, false, 1, ceiling->direction, false);

            if (!(leveltime & 7)
                // [BH] don't make sound once ceiling is at its destination height
                && ceiling->sector->ceilingheight != ceiling->topheight)
                switch (ceiling->type)
                {
                    case silentCrushAndRaise:
                    case genSilentCrusher:
                        break;

                    default:
                        S_StartSectorSound(&ceiling->sector->soundorg, sfx_stnmov);
                        break;
                }

            T_GradualLightingToCeiling(ceiling);

            if (res == pastdest)
            {
                switch (ceiling->type)
                {
                    case raiseToHighest:
                    case genCeiling:
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    // movers with texture change, change the texture then get removed
                    case genCeilingChgT:
                    case genCeilingChg0:
                        ceiling->sector->special = ceiling->newspecial;

                    case genCeilingChg:
                        ceiling->sector->ceilingpic = ceiling->texture;
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    case silentCrushAndRaise:
                        S_StartSectorSound(&ceiling->sector->soundorg, sfx_pstop);

                    case genSilentCrusher:
                    case genCrusher:
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
            res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->bottomheight, ceiling->crush, 1, ceiling->direction, false);

            if (!(leveltime & 7)
                // [BH] don't make sound once ceiling is at its destination height
                && ceiling->sector->ceilingheight != ceiling->bottomheight)
                switch (ceiling->type)
                {
                    case silentCrushAndRaise:
                    case genSilentCrusher:
                        break;

                    default:
                        S_StartSectorSound(&ceiling->sector->soundorg, sfx_stnmov);
                }

            T_GradualLightingToCeiling(ceiling);

            if (res == pastdest)
            {
                switch (ceiling->type)
                {
                    // 02/09/98 jff change slow crushers' speed back to normal
                    // start back up
                    case genSilentCrusher:
                    case genCrusher:
                        if (ceiling->oldspeed < CEILSPEED * 3)
                            ceiling->speed = ceiling->oldspeed;

                        ceiling->direction = 1; // jff 2/22/98 make it go back up!
                        break;

                    case silentCrushAndRaise:
                        S_StartSectorSound(&ceiling->sector->soundorg, sfx_pstop);

                    case crushAndRaise:
                        ceiling->speed = CEILSPEED;

                    case fastCrushAndRaise:
                        ceiling->direction = 1;
                        break;

                    // in the case of ceiling mover/changer, change the texture
                    // then remove the active ceiling
                    case genCeilingChgT:
                    case genCeilingChg0:
                        ceiling->sector->special = ceiling->newspecial;

                    case genCeilingChg:
                        ceiling->sector->ceilingpic = ceiling->texture;
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    case lowerAndCrush:
                    case lowerToFloor:
                    case lowerToLowest:
                    case lowerToMaxFloor:
                    case genCeiling:
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    default:
                        break;
                }
            }
            else if (res == crushed)
            {
                switch (ceiling->type)
                {
                    // jff 02/08/98 slow down slow crushers on obstacle
                    case genCrusher:
                    case genSilentCrusher:
                        if (ceiling->oldspeed < CEILSPEED * 3)
                            ceiling->speed = CEILSPEED / 8;

                        break;

                    case silentCrushAndRaise:
                    case crushAndRaise:
                    case lowerAndCrush:
                        ceiling->speed = CEILSPEED / 8;
                        break;

                    default:
                        break;
                }
            }

            break;
    }
}

//
// EV_DoCeiling
// Move a ceiling up/down and all around!
//
dboolean EV_DoCeiling(line_t *line, ceiling_e type)
{
    int         secnum = -1;
    dboolean    rtn = false;
    sector_t    *sec;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
            goto manual_ceiling;
        else
            return false;
    }

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
        ceiling_t   *ceiling;

        sec = sectors + secnum;

manual_ceiling:
        if (P_SectorActive(ceiling_special, sec))
        {
            if (!zerotag_manual)
                continue;
            else
                return rtn;
        }

        // new ceiling thinker
        rtn = true;
        ceiling = Z_Calloc(1, sizeof(*ceiling), PU_LEVSPEC, NULL);

        ceiling->thinker.function = T_MoveCeiling;
        P_AddThinker(&ceiling->thinker);

        sec->ceilingdata = ceiling;
        ceiling->sector = sec;

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

            case lowerToLowest:
                ceiling->bottomheight = P_FindLowestCeilingSurrounding(sec);
                ceiling->direction = -1;
                ceiling->speed = CEILSPEED;
                break;

            case lowerToMaxFloor:
                ceiling->bottomheight = P_FindHighestFloorSurrounding(sec);
                ceiling->direction = -1;
                ceiling->speed = CEILSPEED;
                break;

            default:
                break;
        }

        ceiling->tag = sec->tag;
        ceiling->type = type;
        P_AddActiveCeiling(ceiling);

        // [BH] ceiling is no longer secret
        for (int i = 0; i < sec->linecount; i++)
            sec->lines[i]->flags &= ~ML_SECRET;
    }

    return rtn;
}

//
// P_AddActiveCeiling
// Add an active ceiling
//
void P_AddActiveCeiling(ceiling_t *ceiling)
{
    ceilinglist_t   *list = malloc(sizeof(*list));

    if (!list)
        return;

    list->ceiling = ceiling;
    ceiling->list = list;

    if ((list->next = activeceilings))
        list->next->prev = &list->next;

    list->prev = &activeceilings;
    activeceilings = list;
}

//
// P_RemoveActiveCeiling
// Remove a ceiling's thinker
//
void P_RemoveActiveCeiling(ceiling_t *ceiling)
{
    ceilinglist_t   *list = ceiling->list;

    ceiling->sector->ceilingdata = NULL;
    P_RemoveThinker(&ceiling->thinker);

    if ((*list->prev = list->next))
        list->next->prev = list->prev;

    free(list);
}

//
// P_RemoveAllActiveCeilings()
// Removes all ceilings from the active ceiling list
//
void P_RemoveAllActiveCeilings(void)
{
    while (activeceilings)
    {
        ceilinglist_t   *next = activeceilings->next;

        free(activeceilings);
        activeceilings = next;
    }
}

//
// P_ActivateInStasisCeiling
// Restart a ceiling that's in-stasis
//
dboolean P_ActivateInStasisCeiling(line_t *line)
{
    dboolean    result = false;

    for (ceilinglist_t *list = activeceilings; list; list = list->next)
    {
        ceiling_t   *ceiling = list->ceiling;

        if (ceiling->tag == line->tag && !ceiling->direction)
        {
            ceiling->direction = ceiling->olddirection;
            ceiling->thinker.function = T_MoveCeiling;
            result = true;
        }
    }

    return result;
}

//
// EV_CeilingCrushStop
// Stop a ceiling from crushing!
//
dboolean EV_CeilingCrushStop(line_t *line)
{
    dboolean    result = false;

    for (ceilinglist_t *list = activeceilings; list; list = list->next)
    {
        ceiling_t   *ceiling = list->ceiling;

        if (ceiling->direction && ceiling->tag == line->tag)
        {
            ceiling->olddirection = ceiling->direction;
            ceiling->direction = 0;
            ceiling->thinker.function = NULL;
            result = true;
        }
    }

    return result;
}
