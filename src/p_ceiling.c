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

#include "doomstat.h"
#include "m_config.h"
#include "p_fix.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

// the list of ceilings currently moving, including crushers
ceilinglist_t   *activeceilings;

static void P_GradualLightingToCeiling(ceiling_t *ceiling)
{
    sector_t        *sector = ceiling->sector;
    const fixed_t   level = ceiling->topheight - sector->floorheight;

    if (level > 0 && !islightspecial[sector->special] && sector->ceilingpic != skyflatnum)
        EV_LightByAdjacentSectors(sector, FixedDiv(sector->ceilingheight - sector->floorheight, level));
}

void T_CeilingStay(ceiling_t *ceiling) {}

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
            res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->topheight, false, 1, ceiling->direction);

            if (!(maptime & 7))
                switch (ceiling->type)
                {
                    case SilentCrushAndRaise:
                    case GenSilentCrusher:
                        break;

                    default:
                        S_StartSectorSound(&ceiling->sector->soundorg, sfx_stnmov);
                        break;
                }

            if (r_graduallighting && !nograduallighting)
                P_GradualLightingToCeiling(ceiling);

            if (res == pastdest)
            {
                switch (ceiling->type)
                {
                    case RaiseToHighest:
                    case GenCeiling:
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    // movers with texture change, change the texture then get removed
                    case GenCeilingChgT:
                    case GenCeilingChg0:
                        ceiling->sector->special = ceiling->newspecial;

                    case GenCeilingChg:
                        ceiling->sector->ceilingpic = ceiling->texture;
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    case SilentCrushAndRaise:
                        S_StartSectorSound(&ceiling->sector->soundorg, sfx_pstop);

                    case GenSilentCrusher:
                    case GenCrusher:
                    case FastCrushAndRaise:
                    case CrushAndRaise:
                        ceiling->direction = -1;
                        break;

                    default:
                        break;
                }
            }

            break;

        case -1:
            // DOWN
            res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->bottomheight, ceiling->crush, 1, ceiling->direction);

            if (!(maptime & 7))
                switch (ceiling->type)
                {
                    case SilentCrushAndRaise:
                    case GenSilentCrusher:
                        break;

                    default:
                        S_StartSectorSound(&ceiling->sector->soundorg, sfx_stnmov);
                }

            if (r_graduallighting && !nograduallighting)
                P_GradualLightingToCeiling(ceiling);

            if (res == pastdest)
            {
                switch (ceiling->type)
                {
                    // 02/09/98 jff change slow crushers' speed back to normal
                    // start back up
                    case GenSilentCrusher:
                    case GenCrusher:
                        if (ceiling->oldspeed < CEILSPEED * 3)
                            ceiling->speed = ceiling->oldspeed;

                        ceiling->direction = 1; // jff 02/22/98 make it go back up!
                        break;

                    case SilentCrushAndRaise:
                        S_StartSectorSound(&ceiling->sector->soundorg, sfx_pstop);

                    case CrushAndRaise:
                        ceiling->speed = CEILSPEED;

                    case FastCrushAndRaise:
                        ceiling->direction = 1;
                        break;

                    // in the case of ceiling mover/changer, change the texture
                    // then remove the active ceiling
                    case GenCeilingChgT:
                    case GenCeilingChg0:
                        ceiling->sector->special = ceiling->newspecial;

                    case GenCeilingChg:
                        ceiling->sector->ceilingpic = ceiling->texture;
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    case LowerAndCrush:
                    case LowerToFloor:
                    case LowerToLowest:
                    case LowerToMaxFloor:
                    case GenCeiling:
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
                    case GenCrusher:
                    case GenSilentCrusher:
                        if (ceiling->oldspeed < CEILSPEED * 3)
                            ceiling->speed = CEILSPEED / 8;

                        break;

                    case SilentCrushAndRaise:
                    case CrushAndRaise:
                    case LowerAndCrush:
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
bool EV_DoCeiling(const line_t *line, ceiling_e type)
{
    int         secnum = -1;
    bool        rtn = false;
    sector_t    *sec;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
            goto manual_ceiling;
        else
            return false;
    }

    // Reactivate in-stasis ceilings... for certain types.
    switch (type)
    {
        case FastCrushAndRaise:
        case SilentCrushAndRaise:
        case CrushAndRaise:
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

        ceiling->thinker.function = &T_MoveCeiling;
        P_AddThinker(&ceiling->thinker);

        sec->ceilingdata = ceiling;
        ceiling->sector = sec;

        switch (type)
        {
            case FastCrushAndRaise:
                ceiling->crush = true;
                ceiling->topheight = sec->ceilingheight;
                ceiling->bottomheight = sec->floorheight + 8 * FRACUNIT;
                ceiling->direction = -1;
                ceiling->speed = CEILSPEED * 2;
                break;

            case SilentCrushAndRaise:
            case CrushAndRaise:
                ceiling->crush = true;
                ceiling->topheight = sec->ceilingheight;

            case LowerAndCrush:
            case LowerToFloor:
                ceiling->bottomheight = sec->floorheight;

                if (type != LowerToFloor && !MAP04)
                    ceiling->bottomheight += 8 * FRACUNIT;

                ceiling->direction = -1;
                ceiling->speed = CEILSPEED;
                break;

            case RaiseToHighest:
                ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
                ceiling->direction = 1;
                ceiling->speed = CEILSPEED;
                break;

            case LowerToLowest:
                ceiling->bottomheight = P_FindLowestCeilingSurrounding(sec);
                ceiling->direction = -1;
                ceiling->speed = CEILSPEED;
                break;

            case LowerToMaxFloor:
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

        if (zerotag_manual)
            return rtn; // e6y
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
    P_RemoveThinker2(&ceiling->thinker);

    if ((*list->prev = list->next))
        list->next->prev = list->prev;

    free(list);
}

//
// P_RemoveAllActiveCeilings
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
bool P_ActivateInStasisCeiling(const line_t *line)
{
    bool    result = false;

    for (ceilinglist_t *list = activeceilings; list; list = list->next)
    {
        ceiling_t   *ceiling = list->ceiling;

        if (ceiling->tag == line->tag && !ceiling->direction)
        {
            ceiling->direction = ceiling->olddirection;
            ceiling->thinker.function = &T_MoveCeiling;
            result = true;
        }
    }

    return result;
}

//
// EV_CeilingCrushStop
// Stop a ceiling from crushing!
//
bool EV_CeilingCrushStop(const line_t *line)
{
    bool    result = false;

    for (ceilinglist_t *list = activeceilings; list; list = list->next)
    {
        ceiling_t   *ceiling = list->ceiling;

        if (ceiling->direction && ceiling->tag == line->tag)
        {
            ceiling->olddirection = ceiling->direction;
            ceiling->direction = 0;
            ceiling->thinker.function = &T_CeilingStay;
            result = true;
        }
    }

    return result;
}
