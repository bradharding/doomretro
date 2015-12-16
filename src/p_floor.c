/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "doomstat.h"
#include "m_random.h"
#include "p_fix.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

extern dboolean canmodify;

//
// FLOORS
//

//
// Move a plane (floor or ceiling) and check for crushing
//
result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest, dboolean crush,
    int floorOrCeiling, int direction)
{
    fixed_t     lastpos;
    fixed_t     destheight;

    sector->oldgametic = gametic;

    switch (floorOrCeiling)
    {
        case 0:
            // FLOOR
            sector->oldfloorheight = sector->floorheight;
            switch (direction)
            {
                case -1:
                    // DOWN
                    if (sector->floorheight - speed < dest)
                    {
                        lastpos = sector->floorheight;
                        sector->floorheight = dest;
                        if (P_ChangeSector(sector, crush))
                        {
                            sector->floorheight = lastpos;
                            P_ChangeSector(sector, crush);
                        }
                        return pastdest;
                    }
                    else
                    {
                        lastpos = sector->floorheight;
                        sector->floorheight -= speed;
                        P_ChangeSector(sector, crush);
                    }
                    break;

                case 1:
                    // UP
                    destheight = MIN(dest, sector->ceilingheight);
                    if (sector->floorheight + speed > destheight)
                    {
                        lastpos = sector->floorheight;
                        sector->floorheight = dest;
                        if (P_ChangeSector(sector, crush))
                        {
                            sector->floorheight = lastpos;
                            P_ChangeSector(sector, crush);
                        }
                        return pastdest;
                    }
                    else
                    {
                        // COULD GET CRUSHED
                        lastpos = sector->floorheight;
                        sector->floorheight += speed;
                        if (P_ChangeSector(sector, crush))
                        {
                            sector->floorheight = lastpos;
                            P_ChangeSector(sector, crush);
                            return crushed;
                        }
                    }
                    break;
            }
            break;

        case 1:
            // CEILING
            sector->oldceilingheight = sector->ceilingheight;
            switch (direction)
            {
                case -1:
                    // DOWN
                    destheight = MAX(dest, sector->floorheight);
                    if (sector->ceilingheight - speed < destheight)
                    {
                        lastpos = sector->ceilingheight;
                        sector->ceilingheight = dest;
                        if (P_ChangeSector(sector, crush))
                        {
                            sector->ceilingheight = lastpos;
                            P_ChangeSector(sector, crush);
                        }
                        return pastdest;
                    }
                    else
                    {
                        // COULD GET CRUSHED
                        lastpos = sector->ceilingheight;
                        sector->ceilingheight -= speed;
                        if (P_ChangeSector(sector, crush))
                        {
                            if (crush)
                                return crushed;
                            sector->ceilingheight = lastpos;
                            P_ChangeSector(sector, crush);
                            return crushed;
                        }
                    }
                    break;

                case 1:
                    // UP
                    if (sector->ceilingheight + speed > dest)
                    {
                        lastpos = sector->ceilingheight;
                        sector->ceilingheight = dest;
                        if (P_ChangeSector(sector, crush))
                        {
                            sector->ceilingheight = lastpos;
                            P_ChangeSector(sector, crush);
                        }
                        return pastdest;
                    }
                    else
                    {
                        sector->ceilingheight += speed;
                        P_ChangeSector(sector, crush);
                    }
                    break;
            }
            break;
    }
    return ok;
}

//
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.
void T_MoveFloor(floormove_t *floor)
{
    sector_t    *sec = floor->sector;
    result_e    res = T_MovePlane(sec, floor->speed, floor->floordestheight,
                                  floor->crush, 0, floor->direction);

    if (!(leveltime & 7)
        // [BH] don't make sound once floor is at its destination height
        && sec->floorheight != floor->floordestheight)
        S_StartSectorSound(&sec->soundorg, sfx_stnmov);

    if (res == pastdest)
    {
        if (floor->direction == 1)
        {
            switch (floor->type)
            {
                case donutRaise:
                    sec->special = floor->newspecial;
                    sec->floorpic = floor->texture;
                    P_ChangeSector(sec, false);

                case genFloorChgT:
                case genFloorChg0:
                    sec->special = floor->newspecial;
                    // fall thru

                case genFloorChg:
                    sec->floorpic = floor->texture;
                    P_ChangeSector(sec, false);
                    break;

                default:
                    break;
            }
        }
        else if (floor->direction == -1)
        {
            switch (floor->type)
            {
                case lowerAndChange:
                    sec->special = floor->newspecial;
                    sec->floorpic = floor->texture;
                    P_ChangeSector(sec, false);

                case genFloorChgT:
                case genFloorChg0:
                    sec->special = floor->newspecial;
                    // fall thru

                case genFloorChg:
                    sec->floorpic = floor->texture;
                    P_ChangeSector(sec, false);
                    break;

                default:
                    break;
            }
        }
        floor->sector->floordata = NULL;
        P_RemoveThinker(&floor->thinker);

        // jff 2/26/98 implement stair retrigger lockout while still building
        // note this only applies to the retriggerable generalized stairs
        if (sec->stairlock == -2)               // if this sector is stairlocked
        {
            sec->stairlock = -1;                // thinker done, promote lock to -1

            while (sec->prevsec != -1 && sectors[sec->prevsec].stairlock != -2)
                sec = &sectors[sec->prevsec];   // search for a non-done thinker
            if (sec->prevsec == -1)             // if all thinkers previous are done
            {
                sec = floor->sector;            // search forward
                while (sec->nextsec != -1 && sectors[sec->nextsec].stairlock != -2)
                    sec = &sectors[sec->nextsec];
                if (sec->nextsec == -1)         // if all thinkers ahead are done too
                {
                    while (sec->prevsec != -1)  // clear all locks
                    {
                        sec->stairlock = 0;
                        sec = &sectors[sec->prevsec];
                    }
                    sec->stairlock = 0;
                }
            }
        }

        // [BH] don't make stop sound if floor already at its destination height
        if (floor->stopsound)
            S_StartSectorSound(&sec->soundorg, sfx_pstop);
    }
}

//
// T_MoveElevator()
//
// Move an elevator to it's destination (up or down)
// Called once per tick for each moving floor.
//
// Passed an elevator_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return value.
//
// jff 02/22/98 added to support parallel floor/ceiling motion
//
void T_MoveElevator(elevator_t *elevator)
{
    result_e    res;

    if (elevator->direction < 0)                // moving down
    {
        // jff 4/7/98 reverse order of ceiling/floor
        res = T_MovePlane(elevator->sector, elevator->speed, elevator->ceilingdestheight, 0, 1,
            elevator->direction);

        // jff 4/7/98 don't move ceil if blocked
        if (res == ok || res == pastdest)
            T_MovePlane(elevator->sector, elevator->speed, elevator->floordestheight, 0, 0,
                elevator->direction);
    }
    else                                        // up
    {
        // jff 4/7/98 reverse order of ceiling/floor
        res = T_MovePlane(elevator->sector, elevator->speed, elevator->floordestheight, 0, 0,
            elevator->direction);

        // jff 4/7/98 don't move floor if blocked
        if (res == ok || res == pastdest)
            T_MovePlane(elevator->sector, elevator->speed, elevator->ceilingdestheight, 0, 1,
                elevator->direction);
    }

    // make floor move sound
    if (!(leveltime & 7))
        S_StartSectorSound(&elevator->sector->soundorg, sfx_stnmov);

    if (res == pastdest)                        // if destination height achieved
    {
        elevator->sector->floordata = NULL;
        elevator->sector->ceilingdata = NULL;
        P_RemoveThinker(&elevator->thinker);     // remove elevator from actives

        // make floor stop sound
        S_StartSectorSound(&elevator->sector->soundorg, sfx_pstop);
    }
}

//
// HANDLE FLOOR TYPES
//
dboolean EV_DoFloor(line_t *line, floor_e floortype)
{
    int         secnum = -1;
    int         i;
    dboolean    rtn = false;
    floormove_t *floor;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sector_t        *sec = &sectors[secnum];

        // ALREADY MOVING? IF SO, KEEP GOING...
        if (P_SectorActive(floor_special, sec))
            continue;

        // new floor thinker
        rtn = true;
        floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker(&floor->thinker);
        sec->floordata = floor;
        floor->thinker.function = T_MoveFloor;
        floor->type = floortype;
        floor->crush = false;

        switch (floortype)
        {
            case lowerFloor:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindHighestFloorSurrounding(sec);
                break;

            case lowerFloor24:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
                break;

            case lowerFloor32Turbo:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED * 4;
                floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
                break;

            case lowerFloorToLowest:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindLowestFloorSurrounding(sec);
                break;

            case lowerFloorToNearest:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindNextLowestFloor(sec, floor->sector->floorheight);
                break;

            case turboLower:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED * 4;
                floor->floordestheight = P_FindHighestFloorSurrounding(sec);
                if (floor->floordestheight != sec->floorheight)
                    floor->floordestheight += 8 * FRACUNIT;
                break;

            case raiseFloorCrush:
                floor->crush = true;
            case raiseFloor:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = MIN(P_FindLowestCeilingSurrounding(sec),
                    sec->ceilingheight) - 8 * FRACUNIT * (floortype == raiseFloorCrush);
                break;

            case raiseFloorTurbo:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED * 4;
                floor->floordestheight = P_FindNextHighestFloor(sec, sec->floorheight);
                break;

            case raiseFloorToNearest:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindNextHighestFloor(sec, sec->floorheight);
                break;

            case raiseFloor24:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = sec->floorheight + 24 * FRACUNIT;
                break;

            case raiseFloor32Turbo:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED * 4;
                floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
                break;

            case raiseFloor512:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = sec->floorheight + 512 * FRACUNIT;
                break;

            case raiseFloor24AndChange:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = sec->floorheight + 24 * FRACUNIT;

                if (E2M2)
                    sec->floorpic = R_FlatNumForName("FLOOR5_4");
                else if (MAP12)
                    sec->floorpic = R_FlatNumForName("FLOOR7_1");
                else
                    sec->floorpic = line->frontsector->floorpic;

                sec->special = line->frontsector->special;
                break;

            case raiseToTexture:
            {
                int     minsize = 32000 << FRACBITS;

                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                for (i = 0; i < sec->linecount; i++)
                {
                    if (twoSided(secnum, i))
                    {
                        side_t  *side = getSide(secnum, i, 0);

                        if (side->bottomtexture > 0
                            && textureheight[side->bottomtexture] < minsize)
                            minsize = textureheight[side->bottomtexture];
                        side = getSide(secnum, i, 1);
                        if (side->bottomtexture > 0
                            && textureheight[side->bottomtexture] < minsize)
                            minsize = textureheight[side->bottomtexture];
                    }
                }
                floor->floordestheight = MIN((sec->floorheight >> FRACBITS)
                    + (minsize >> FRACBITS), 32000) << FRACBITS;
            }
            break;

            case lowerAndChange:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindLowestFloorSurrounding(sec);
                floor->texture = sec->floorpic;
                floor->newspecial = sec->special;

                for (i = 0; i < sec->linecount; i++)
                {
                    if (twoSided(secnum, i))
                    {
                        if (getSide(secnum, i, 0)->sector - sectors == secnum)
                        {
                            sec = getSector(secnum, i, 1);

                            if (sec->floorheight == floor->floordestheight)
                            {
                                floor->texture = sec->floorpic;
                                floor->newspecial = sec->special;
                                break;
                            }
                        }
                        else
                        {
                            sec = getSector(secnum, i, 0);

                            if (sec->floorheight == floor->floordestheight)
                            {
                                floor->texture = sec->floorpic;
                                floor->newspecial = sec->special;
                                break;
                            }
                        }
                    }
                }

            default:
                break;
        }

        floor->stopsound = (floor->sector->floorheight != floor->floordestheight);

        // [BH] floor is no longer secret
        for (i = 0; i < sec->linecount; i++)
            sec->lines[i]->flags &= ~ML_SECRET;
    }
    return rtn;
}

//
// EV_DoChange()
//
// Handle pure change types. These change floor texture and sector type
// by trigger or numeric model without moving the floor.
//
// The linedef causing the change and the type of change is passed
// Returns true if any sector changes
//
// jff 3/15/98 added to better support generalized sector types
//
dboolean EV_DoChange(line_t *line, change_e changetype)
{
    int         secnum;
    dboolean    rtn;
    sector_t    *secm;

    secnum = -1;
    rtn = false;

    // change all sectors with the same tag as the linedef
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sector_t        *sec = &sectors[secnum];

        rtn = true;

        // handle trigger or numeric change type
        switch (changetype)
        {
            case trigChangeOnly:
                sec->floorpic = line->frontsector->floorpic;
                P_ChangeSector(sec, false);
                sec->special = line->frontsector->special;
                break;

            case numChangeOnly:
                secm = P_FindModelFloorSector(sec->floorheight, secnum);
                if (secm)       // if no model, no change
                {
                    sec->floorpic = secm->floorpic;
                    P_ChangeSector(sec, false);
                    sec->special = secm->special;
                }
                break;
            default:
                break;
        }
    }
    return rtn;
}

//
// BUILD A STAIRCASE!
//

// cph 2001/09/21 - compatibility nightmares again
// There are three different ways this function has, during its history, stepped
// through all the stairs to be triggered by the single switch
// - original DOOM used a linear P_FindSectorFromLineTag, but failed to preserve
// the index of the previous sector found, so instead it would restart its
// linear search from the last sector of the previous staircase
// - MBF/PrBoom with comp_stairs fail to emulate this, because their
// P_FindSectorFromLineTag is a chained hash table implementation. Instead they
// start following the hash chain from the last sector of the previous
// staircase, which will (probably) have the wrong tag, so they miss any further
// stairs
// - BOOM fixed the bug, and MBF/PrBoom without comp_stairs work right
//
static int P_FindSectorFromLineTagWithLowerBound(line_t *l, int start, int min)
{
  do
  {
      start = P_FindSectorFromLineTag(l, start);
  } while (start >= 0 && start <= min);

  return start;
}

dboolean EV_BuildStairs(line_t *line, stair_e type)
{
    int         ssec = -1;
    int         minssec = -1;
    dboolean    rtn = false;

    while ((ssec = P_FindSectorFromLineTagWithLowerBound(line, ssec, minssec)) >= 0)
    {
        int             secnum = ssec;
        sector_t        *sec = &sectors[secnum];
        floormove_t     *floor;
        fixed_t         stairsize = 0;
        fixed_t         speed = 0;
        dboolean        crushing = false;
        dboolean        okay;
        int             height;
        int             texture;

        // ALREADY MOVING?  IF SO, KEEP GOING...
        if (P_SectorActive(floor_special, sec))
            continue;

        // new floor thinker
        rtn = true;
        floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker(&floor->thinker);
        sec->floordata = floor;
        floor->thinker.function = T_MoveFloor;
        floor->direction = 1;
        floor->sector = sec;
        switch (type)
        {
            case build8:
                speed = FLOORSPEED / 4;
                stairsize = 8 * FRACUNIT;
                crushing = false;
                break;
            case turbo16:
                speed = FLOORSPEED * 4;
                stairsize = 16 * FRACUNIT;
                crushing = true;
                break;
        }
        floor->speed = speed;
        height = sec->floorheight + stairsize;
        floor->floordestheight = height;
        floor->newspecial = 0;
        floor->texture = 0;
        floor->crush = crushing;
        floor->type = buildStair;
        floor->stopsound = (sec->floorheight != floor->floordestheight);

        texture = sec->floorpic;

        // Find next sector to raise
        // 1. Find 2-sided line with same sector side[0]
        // 2. Other side is the next sector to raise
        do
        {
            int i;

            okay = false;
            for (i = 0; i < sec->linecount; ++i)
            {
                line_t          *line = sec->lines[i];
                sector_t        *tsec;
                int             newsecnum;

                if (!(line->flags & ML_TWOSIDED))
                    continue;

                tsec = line->frontsector;
                newsecnum = tsec - sectors;

                if (secnum != newsecnum)
                    continue;

                tsec = line->backsector;
                if (!tsec)
                    continue;
                newsecnum = tsec - sectors;

                if (tsec->floorpic != texture)
                    continue;

                height += stairsize;

                if (P_SectorActive(floor_special, tsec))
                    continue;

                sec = tsec;
                secnum = newsecnum;
                floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, 0);
                P_AddThinker(&floor->thinker);

                sec->floordata = floor;
                floor->thinker.function = T_MoveFloor;
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = speed;
                floor->floordestheight = height;
                floor->type = buildStair;
                floor->crush = (type != build8);
                floor->stopsound = (sec->floorheight != height);
                okay = true;
                break;
            }
        } while (okay);
    }
    return rtn;
}

//
// EV_DoElevator
//
// Handle elevator linedef types
//
// Passed the linedef that triggered the elevator and the elevator action
//
// jff 2/22/98 new type to move floor and ceiling in parallel
//
dboolean EV_DoElevator(line_t *line, elevator_e elevtype)
{
    int         secnum;
    dboolean    rtn;
    sector_t    *sec;
    elevator_t  *elevator;

    secnum = -1;
    rtn = false;

    // act on all sectors with the same tag as the triggering linedef
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];

        // If either floor or ceiling is already activated, skip it
        if (sec->floordata || sec->ceilingdata)
            continue;

        // create and initialize new elevator thinker
        rtn = true;
        elevator = Z_Malloc(sizeof(*elevator), PU_LEVSPEC, 0);
        P_AddThinker(&elevator->thinker);
        sec->floordata = elevator;
        sec->ceilingdata = elevator;
        elevator->thinker.function = T_MoveElevator;
        elevator->type = elevtype;

        // set up the fields according to the type of elevator action
        switch (elevtype)
        {
            // elevator down to next floor
            case elevateDown:
                elevator->direction = -1;
                elevator->sector = sec;
                elevator->speed = ELEVATORSPEED;
                elevator->floordestheight = P_FindNextLowestFloor(sec, sec->floorheight);
                elevator->ceilingdestheight = elevator->floordestheight + sec->ceilingheight
                    - sec->floorheight;
                break;

            // elevator up to next floor
            case elevateUp:
                elevator->direction = 1;
                elevator->sector = sec;
                elevator->speed = ELEVATORSPEED;
                elevator->floordestheight = P_FindNextHighestFloor(sec, sec->floorheight);
                elevator->ceilingdestheight = elevator->floordestheight + sec->ceilingheight
                    - sec->floorheight;
                break;

            // elevator to floor height of activating switch's front sector
            case elevateCurrent:
                elevator->sector = sec;
                elevator->speed = ELEVATORSPEED;
                elevator->floordestheight = line->frontsector->floorheight;
                elevator->ceilingdestheight = elevator->floordestheight + sec->ceilingheight
                    - sec->floorheight;
                elevator->direction = (elevator->floordestheight > sec->floorheight ? 1 : -1);
                break;

            default:
                break;
        }
    }
    return rtn;
}
