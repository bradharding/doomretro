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
#include "m_random.h"
#include "p_fix.h"
#include "p_local.h"
#include "s_sound.h"
#include "z_zone.h"

boolean animatedliquid = ANIMATEDLIQUID_DEFAULT;

fixed_t animatedliquiddiffs[128] =
{
     3211,  3211,  3211,  3211,  3180,  3180,  3119,  3119,
     3027,  3027,  2907,  2907,  2758,  2758,  2582,  2582,
     2382,  2382,  2159,  2159,  1915,  1915,  1653,  1653,
     1374,  1374,  1083,  1083,   781,   781,   471,   471,
      157,   157,  -157,  -157,  -471,  -471,  -781,  -781,
    -1083, -1083, -1374, -1374, -1653, -1653, -1915, -1915,
    -2159, -2159, -2382, -2382, -2582, -2582, -2758, -2758,
    -2907, -2907, -3027, -3027, -3119, -3119, -3180, -3180,
    -3211, -3211, -3211, -3211, -3180, -3180, -3119, -3119,
    -3027, -3027, -2907, -2907, -2758, -2758, -2582, -2582,
    -2382, -2382, -2159, -2159, -1915, -1915, -1653, -1653,
    -1374, -1374, -1083, -1083,  -781,  -781,  -471,  -471,
     -157,  -157,   157,   157,   471,   471,   781,   781,
     1083,  1083,  1374,  1374,  1653,  1653,  1915,  1915,
     2159,  2159,  2382,  2382,  2582,  2582,  2758,  2758,
     2907,  2907,  3027,  3027,  3119,  3119,  3180,  3180
};

extern boolean  canmodify;

void T_AnimateLiquid(floormove_t *floor)
{
    sector_t    *sector = floor->sector;

    if (isliquid[sector->floorpic] && sector->ceilingheight != sector->floorheight)
        sector->animate += animatedliquiddiffs[leveltime & 127];
    else
        sector->animate = INT_MAX;
}

void P_StartAnimatedLiquid(sector_t *sector)
{
    thinker_t   *th;
    int         j;
    boolean     contained = true;

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
        if (th->function.acp1 == (actionf_p1)T_AnimateLiquid
            && ((floormove_t *)th)->sector == sector)
            return;

    for (j = 0; j < sector->linecount; j++)
    {
        sector_t       *adjacent = getNextSector(sector->lines[j], sector);

        if (adjacent)
            if (isliquid[adjacent->floorpic]
                && sector->floorheight == sector->ceilingheight
                && adjacent->floorheight != adjacent->ceilingheight)
                contained = false;
    }

    if (contained)
    {
        floormove_t     *floor = (floormove_t *)Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);

        P_AddThinker(&floor->thinker);
        floor->thinker.function.acp1 = (actionf_p1)T_AnimateLiquid;
        floor->sector = sector;
        for (j = 0; j < sector->linecount; j++)
        {
            sector_t       *adjacent = getNextSector(sector->lines[j], sector);

            if (adjacent)
                if (isliquid[adjacent->floorpic] && sector->floorheight == adjacent->floorheight)
                {
                    sides[(sector->lines[j])->sidenum[0]].bottomtexture = 0;
                    sides[(sector->lines[j])->sidenum[1]].bottomtexture = 0;
                }
        }
    }
}

void P_InitAnimatedLiquids(void)
{
    int         i;
    sector_t    *sector;
    thinker_t   *th;

    if (!animatedliquid)
        return;

    for (i = 0, sector = sectors; i < numsectors; i++, sector++)
        if (isliquid[sector->floorpic])
            P_StartAnimatedLiquid(sector);
        else
            sector->animate = 0;

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
        if (th->function.acp1 == (actionf_p1)T_AnimateLiquid)
            ((floormove_t *)th)->sector->floorheight += FRACUNIT;
}

//
// FLOORS
//

//
// Move a plane (floor or ceiling) and check for crushing
//
result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest,
                     boolean crush, int floorOrCeiling, int direction)
{
    fixed_t     lastpos;
    fixed_t     destheight;

    switch (floorOrCeiling)
    {
        case 0:
            // FLOOR
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
                        lastpos = sector->ceilingheight;
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
void T_MoveFloor(floormove_t *floor)
{
    sector_t    *sec = floor->sector;
    result_e    res = T_MovePlane(sec, floor->speed, floor->floordestheight,
                                  floor->crush, 0, floor->direction);

    if (!(leveltime & 7) && sec->floorheight != floor->floordestheight)
        S_StartSound(&sec->soundorg, sfx_stnmov);

    if (res == pastdest)
    {
        sec->specialdata = NULL;

        if (floor->direction == 1)
        {
            switch (floor->type)
            {
                case donutRaise:
                    sec->special = floor->newspecial;
                    sec->floorpic = floor->texture;
                    if (isliquid[sec->floorpic])
                    {
                        P_ChangeSector(sec, false);
                        if (animatedliquid)
                            P_StartAnimatedLiquid(sec);
                    }
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
                    if (isliquid[sec->floorpic])
                    {
                        P_ChangeSector(sec, false);
                        if (animatedliquid)
                            P_StartAnimatedLiquid(sec);
                    }
                default:
                    break;
            }
        }
        P_RemoveThinker(&floor->thinker);

        if (floor->stopsound)
            S_StartSound(&sec->soundorg, sfx_pstop);
    }
}

//
// HANDLE FLOOR TYPES
//
boolean EV_DoFloor(line_t *line, floor_e floortype)
{
    int         secnum = -1;
    int         i;
    boolean     rtn = false;
    floormove_t *floor;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sector_t        *sec = &sectors[secnum];

        // ALREADY MOVING? IF SO, KEEP GOING...
        if (sec->specialdata)
            continue;

        // new floor thinker
        rtn = true;
        floor = (floormove_t *)Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker(&floor->thinker);
        sec->specialdata = floor;
        floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
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

            case lowerFloorToLowest:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindLowestFloorSurrounding(sec);
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

                        if (side->bottomtexture > 0 && textureheight[side->bottomtexture] < minsize)
                            minsize = textureheight[side->bottomtexture];
                        side = getSide(secnum, i, 1);
                        if (side->bottomtexture > 0 && textureheight[side->bottomtexture] < minsize)
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

        for (i = 0; i < floor->sector->linecount; i++)
            floor->sector->lines[i]->flags &= ~ML_SECRET;
    }
    return rtn;
}

//
// BUILD A STAIRCASE!
//
boolean EV_BuildStairs(line_t *line, stair_e type)
{
    int         secnum = -1;
    int         height;
    int         i;
    int         newsecnum;
    int         texture;
    int         ok;
    boolean     rtn = false;

    sector_t    *sec;
    sector_t    *tsec;

    floormove_t *floor;

    fixed_t     stairsize = 0;
    fixed_t     speed = 0;

    boolean     crushing = false;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];

        // ALREADY MOVING?  IF SO, KEEP GOING...
        if (sec->specialdata)
            continue;

        // new floor thinker
        rtn = true;
        floor = (floormove_t *)Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker(&floor->thinker);
        sec->specialdata = floor;
        floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
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
            ok = 0;
            for (i = 0; i < sec->linecount; i++)
            {
                if (!((sec->lines[i])->flags & ML_TWOSIDED))
                    continue;

                tsec = (sec->lines[i])->frontsector;
                newsecnum = tsec - sectors;

                if (secnum != newsecnum)
                    continue;

                tsec = (sec->lines[i])->backsector;
                if (!tsec)
                    continue;
                newsecnum = tsec - sectors;

                if (tsec->floorpic != texture)
                    continue;

                height += stairsize;

                if (tsec->specialdata)
                    continue;

                sec = tsec;
                secnum = newsecnum;
                floor = (floormove_t *)Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);

                P_AddThinker(&floor->thinker);

                sec->specialdata = floor;
                floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = speed;
                floor->floordestheight = height;
                floor->stopsound = (sec->floorheight != floor->floordestheight);
                ok = 1;
                break;
            }
        } while (ok);
    }
    return rtn;
}
