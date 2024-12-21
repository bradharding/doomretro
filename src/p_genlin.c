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

#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

//
// EV_DoGenFloor()
//
// Handle generalized floor types
//
// Passed the line activating the generalized floor function
// Returns true if a thinker is created
//
// jff 02/04/98 Added this routine (and file) to handle generalized
// floor movers using bit fields in the line special type.
//
bool EV_DoGenFloor(line_t *line)
{
    int                 secnum = -1;
    bool                rtn = false;
    bool                manual = false;
    sector_t            *sec;
    floormove_t         *floor;
    const unsigned int  value = line->special - GenFloorBase;

    // parse the bit fields in the line's special type
    const int           Crsh = (value & FloorCrush) >> FloorCrushShift;
    const int           ChgT = (value & FloorChange) >> FloorChangeShift;
    const int           Targ = (value & FloorTarget) >> FloorTargetShift;
    const int           Dirn = (value & FloorDirection) >> FloorDirectionShift;
    const int           ChgM = (value & FloorModel) >> FloorModelShift;
    const int           Sped = (value & FloorSpeed) >> FloorSpeedShift;
    const int           Trig = (value & TriggerType) >> TriggerTypeShift;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
        {
            manual = true;
            goto manual_floor;
        }
        else
            return false;
    }

    // check if a manual trigger, if so do just the sector on the backside
    if (Trig == PushOnce || Trig == PushMany)
    {
        if (!(sec = line->backsector))
            return rtn;

        secnum = sec->id;
        manual = true;

        goto manual_floor;
    }

    // if not manual do all sectors tagged the same as the line
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = sectors + secnum;

manual_floor:
        // Do not start another function if floor already moving
        if (P_SectorActive(floor_special, sec))
        {
            if (!manual)
                continue;
            else
                return rtn;
        }

        // new floor thinker
        rtn = true;
        floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, NULL);

        floor->thinker.function = &T_MoveFloor;
        P_AddThinker(&floor->thinker);

        sec->floordata = floor;
        floor->crush = Crsh;
        floor->direction = (Dirn ? 1 : -1);
        floor->sector = sec;
        floor->texture = sec->floorpic;
        floor->newspecial = sec->special;
        floor->type = GenFloor;

        // set the speed of motion
        switch (Sped)
        {
            case SpeedSlow:
                floor->speed = FLOORSPEED;
                break;

            case SpeedNormal:
                floor->speed = FLOORSPEED * 2;
                break;

            case SpeedFast:
                floor->speed = FLOORSPEED * 4;
                break;

            case SpeedTurbo:
                floor->speed = FLOORSPEED * 8;
                break;
        }

        // set the destination height
        switch (Targ)
        {
            case FtoHnF:
                floor->floordestheight = P_FindHighestFloorSurrounding(sec);
                break;

            case FtoLnF:
                floor->floordestheight = P_FindLowestFloorSurrounding(sec);
                break;

            case FtoNnF:
                floor->floordestheight = (Dirn ? P_FindNextHighestFloor(sec, sec->floorheight) :
                    P_FindNextLowestFloor(sec, sec->floorheight));
                break;

            case FtoLnC:
                floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
                break;

            case FtoC:
                floor->floordestheight = sec->ceilingheight;
                break;

            case FbyST:
                floor->floordestheight = (floor->sector->floorheight >> FRACBITS)
                    + floor->direction * (P_FindShortestTextureAround(secnum) >> FRACBITS);
                floor->floordestheight = BETWEEN(-32000, floor->floordestheight, 32000) * FRACUNIT;
                break;

            case Fby24:
                floor->floordestheight = floor->sector->floorheight + floor->direction * 24 * FRACUNIT;
                break;

            case Fby32:
                floor->floordestheight = floor->sector->floorheight + floor->direction * 32 * FRACUNIT;
                break;
        }

        // set texture/type change properties
        if (ChgT)       // if a texture change is indicated
        {
            if (ChgM)   // if a numeric model change
            {
                // jff 05/23/98 find model with ceiling at target height if target is a ceiling type
                sec = (Targ == FtoLnC || Targ == FtoC ? P_FindModelCeilingSector(floor->floordestheight, secnum) :
                    P_FindModelFloorSector(floor->floordestheight, secnum));

                if (sec)
                {
                    floor->texture = sec->floorpic;

                    switch (ChgT)
                    {
                        case FChgZero:  // zero type
                            floor->newspecial = 0;
                            floor->type = GenFloorChg0;
                            break;

                        case FChgTyp:   // copy type
                            floor->newspecial = sec->special;
                            floor->type = GenFloorChgT;
                            break;

                        case FChgTxt:   // leave type be
                            floor->type = GenFloorChg;
                            break;
                    }
                }
            }
            else        // else if a trigger model change
            {
                floor->texture = line->frontsector->floorpic;

                switch (ChgT)
                {
                    case FChgZero:      // zero type
                        floor->newspecial = 0;
                        floor->type = GenFloorChg0;
                        break;

                    case FChgTyp:       // copy type
                        floor->newspecial = line->frontsector->special;
                        floor->type = GenFloorChgT;
                        break;

                    case FChgTxt:       // leave type be
                        floor->type = GenFloorChg;
                        break;
                }
            }
        }

        if (manual)
            return rtn;
    }

    return rtn;
}

//
// EV_DoGenCeiling()
//
// Handle generalized ceiling types
//
// Passed the linedef activating the ceiling function
// Returns true if a thinker created
//
// jff 02/04/98 Added this routine (and file) to handle generalized
// floor movers using bit fields in the line special type.
//
bool EV_DoGenCeiling(line_t *line)
{
    int                 secnum = -1;
    bool                rtn = false;
    bool                manual = false;
    fixed_t             targheight;
    sector_t            *sec;
    ceiling_t           *ceiling;
    const unsigned int  value = line->special - GenCeilingBase;

    // parse the bit fields in the line's special type
    const int           Crsh = (value & CeilingCrush) >> CeilingCrushShift;
    const int           ChgT = (value & CeilingChange) >> CeilingChangeShift;
    const int           Targ = (value & CeilingTarget) >> CeilingTargetShift;
    const int           Dirn = (value & CeilingDirection) >> CeilingDirectionShift;
    const int           ChgM = (value & CeilingModel) >> CeilingModelShift;
    const int           Sped = (value & CeilingSpeed) >> CeilingSpeedShift;
    const int           Trig = (value & TriggerType) >> TriggerTypeShift;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
        {
            manual = true;
            goto manual_ceiling;
        }
        else
            return false;
    }

    // check if a manual trigger, if so do just the sector on the backside
    if (Trig == PushOnce || Trig == PushMany)
    {
        if (!(sec = line->backsector))
            return rtn;

        secnum = sec->id;
        manual = true;

        goto manual_ceiling;
    }

    // if not manual do all sectors tagged the same as the line
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = sectors + secnum;

manual_ceiling:
        // Do not start another function if ceiling already moving
        if (P_SectorActive(ceiling_special, sec))
        {
            if (manual)
                return rtn;
            else
                continue;
        }

        // new ceiling thinker
        rtn = true;
        ceiling = Z_Calloc(1, sizeof(*ceiling), PU_LEVSPEC, NULL);

        ceiling->thinker.function = &T_MoveCeiling;
        P_AddThinker(&ceiling->thinker);

        sec->ceilingdata = ceiling;
        ceiling->crush = Crsh;
        ceiling->direction = (Dirn ? 1 : -1);
        ceiling->sector = sec;
        ceiling->texture = sec->ceilingpic;
        ceiling->newspecial = sec->special;
        ceiling->tag = sec->tag;
        ceiling->type = GenCeiling;

        // set speed of motion
        switch (Sped)
        {
            case SpeedSlow:
                ceiling->speed = CEILSPEED;
                break;

            case SpeedNormal:
                ceiling->speed = CEILSPEED * 2;
                break;

            case SpeedFast:
                ceiling->speed = CEILSPEED * 4;
                break;

            case SpeedTurbo:
                ceiling->speed = CEILSPEED * 8;
                break;
        }

        // set destination target height
        targheight = sec->ceilingheight;

        switch (Targ)
        {
            case CtoHnC:
                targheight = P_FindHighestCeilingSurrounding(sec);
                break;

            case CtoLnC:
                targheight = P_FindLowestCeilingSurrounding(sec);
                break;

            case CtoNnC:
                targheight = (Dirn ? P_FindNextHighestCeiling(sec, sec->ceilingheight) :
                    P_FindNextLowestCeiling(sec, sec->ceilingheight));
                break;

            case CtoHnF:
                targheight = P_FindHighestFloorSurrounding(sec);
                break;

            case CtoF:
                targheight = sec->floorheight;
                break;

            case CbyST:
                targheight = (ceiling->sector->ceilingheight >> FRACBITS)
                    + ceiling->direction * (P_FindShortestUpperAround(secnum) >> FRACBITS);
                targheight = BETWEEN(-32000, targheight, 32000) * FRACUNIT;
                break;

            case Cby24:
                targheight = ceiling->sector->ceilingheight + ceiling->direction * 24 * FRACUNIT;
                break;

            case Cby32:
                targheight = ceiling->sector->ceilingheight + ceiling->direction * 32 * FRACUNIT;
                break;
        }

        if (Dirn)
            ceiling->topheight = targheight;
        else
            ceiling->bottomheight = targheight;

        // set texture/type change properties
        if (ChgT)       // if a texture change is indicated
        {
            if (ChgM)   // if a numeric model change
            {
                // jff 05/23/98 find model with floor at target height if target is a floor type
                sec = (Targ == CtoHnF || Targ == CtoF ? P_FindModelFloorSector(targheight, secnum) :
                    P_FindModelCeilingSector(targheight, secnum));

                if (sec)
                {
                    ceiling->texture = sec->ceilingpic;

                    switch (ChgT)
                    {
                        case CChgZero:  // type is zeroed
                            ceiling->newspecial = 0;
                            ceiling->type = GenCeilingChg0;
                            break;

                        case CChgTyp:   // type is copied
                            ceiling->newspecial = sec->special;
                            ceiling->type = GenCeilingChgT;
                            break;

                        case CChgTxt:   // type is left alone
                            ceiling->type = GenCeilingChg;
                            break;
                    }
                }
            }
            else        // else if a trigger model change
            {
                ceiling->texture = line->frontsector->ceilingpic;

                switch (ChgT)
                {
                    case CChgZero:      // type is zeroed
                        ceiling->newspecial = 0;
                        ceiling->type = GenCeilingChg0;
                        break;

                    case CChgTyp:       // type is copied
                        ceiling->newspecial = line->frontsector->special;
                        ceiling->type = GenCeilingChgT;
                        break;

                    case CChgTxt:       // type is left alone
                        ceiling->type = GenCeilingChg;
                        break;
                }
            }
        }

        P_AddActiveCeiling(ceiling);    // add this ceiling to the active list

        if (manual)
            return rtn;
    }

    return rtn;
}

//
// EV_DoGenLift()
//
// Handle generalized lift types
//
// Passed the linedef activating the lift
// Returns true if a thinker is created
//
bool EV_DoGenLift(line_t *line)
{
    plat_t              *plat;
    int                 secnum = -1;
    bool                rtn = false;
    bool                manual = false;
    sector_t            *sec;
    const unsigned int  value = line->special - GenLiftBase;

    // parse the bit fields in the line's special type
    const int           Targ = (value & LiftTarget) >> LiftTargetShift;
    const int           Dely = (value & LiftDelay) >> LiftDelayShift;
    const int           Sped = (value & LiftSpeed) >> LiftSpeedShift;
    const int           Trig = (value & TriggerType) >> TriggerTypeShift;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
        {
            manual = true;
            goto manual_lift;
        }
        else
            return false;
    }

    // Activate all <type> plats that are in_stasis
    if (Targ == LnF2HnF)
        P_ActivateInStasis(line->tag);

    // check if a manual trigger, if so do just the sector on the backside
    if (Trig == PushOnce || Trig == PushMany)
    {
        if (!(sec = line->backsector))
            return rtn;

        secnum = sec->id;
        manual = true;

        goto manual_lift;
    }

    // if not manual do all sectors tagged the same as the line
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = sectors + secnum;

manual_lift:
        // Do not start another function if floor already moving
        if (P_SectorActive(floor_special, sec))
        {
            if (!manual)
                continue;
            else
                return rtn;
        }

        // Setup the plat thinker
        rtn = true;
        plat = Z_Calloc(1, sizeof(*plat), PU_LEVSPEC, NULL);

        plat->thinker.function = &T_PlatRaise;
        P_AddThinker(&plat->thinker);

        plat->sector = sec;
        plat->sector->floordata = plat;
        plat->tag = line->tag;
        plat->type = GenLift;
        plat->high = sec->floorheight;
        plat->status = down;

        // setup the target destination height
        switch (Targ)
        {
            case F2LnF:
                plat->low = MIN(P_FindLowestFloorSurrounding(sec), sec->floorheight);
                break;

            case F2NnF:
                plat->low = P_FindNextLowestFloor(sec, sec->floorheight);
                break;

            case F2LnC:
                plat->low = MIN(P_FindLowestCeilingSurrounding(sec), sec->floorheight);
                break;

            case LnF2HnF:
                plat->type = GenPerpetual;
                plat->low = MIN(P_FindLowestFloorSurrounding(sec), sec->floorheight);
                plat->high = MAX(P_FindHighestFloorSurrounding(sec), sec->floorheight);
                plat->status = (plat_e)(M_Random() & 1);
                break;
        }

        // setup the speed of motion
        switch (Sped)
        {
            case SpeedSlow:
                plat->speed = PLATSPEED * 2;
                break;

            case SpeedNormal:
                plat->speed = PLATSPEED * 4;
                break;

            case SpeedFast:
                plat->speed = PLATSPEED * 8;
                break;

            case SpeedTurbo:
                plat->speed = PLATSPEED * 16;
                break;
        }

        // setup the delay time before the floor returns
        switch (Dely)
        {
            case 0:
                plat->wait = TICRATE;
                break;

            case 1:
                plat->wait = PLATWAIT * TICRATE;
                break;

            case 2:
                plat->wait = 5 * TICRATE;
                break;

            case 3:
                plat->wait = 10 * TICRATE;
                break;
        }

        S_StartSectorSound(&sec->soundorg, sfx_pstart);
        P_AddActivePlat(plat);  // add this plat to the list of active plats

        if (manual)
            return rtn;
    }

    return rtn;
}

//
// EV_DoGenStairs()
//
// Handle generalized stair building
//
// Passed the linedef activating the stairs
// Returns true if a thinker is created
//
bool EV_DoGenStairs(line_t *line)
{
    int                 secnum = -1;
    int                 osecnum;        // jff 03/04/98 preserve loop index
    int                 height;
    int                 newsecnum;
    int                 texture;
    bool                okay;
    bool                rtn = false;
    bool                manual = false;
    sector_t            *sec;
    sector_t            *tsec;
    floormove_t         *floor;
    fixed_t             stairsize;
    fixed_t             speed;
    const unsigned int  value = line->special - GenStairsBase;

    // parse the bit fields in the line's special type
    const int           Igno = (value & StairIgnore) >> StairIgnoreShift;
    const int           Dirn = (value & StairDirection) >> StairDirectionShift;
    const int           Step = (value & StairStep) >> StairStepShift;
    const int           Sped = (value & StairSpeed) >> StairSpeedShift;
    const int           Trig = (value & TriggerType) >> TriggerTypeShift;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
        {
            manual = true;
            goto manual_stair;
        }
        else
            return false;
    }

    // check if a manual trigger, if so do just the sector on the backside
    if (Trig == PushOnce || Trig == PushMany)
    {
        if (!(sec = line->backsector))
            return rtn;

        secnum = sec->id;
        manual = true;

        goto manual_stair;
    }

    // if not manual do all sectors tagged the same as the line
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = sectors + secnum;

manual_stair:
        // Do not start another function if floor already moving
        // jff 02/26/98 add special lockout condition to wait for entire
        // staircase to build before retriggering
        if (P_SectorActive(floor_special, sec) || sec->stairlock)
        {
            if (!manual)
                continue;
            else
                return rtn;
        }

        // new floor thinker
        rtn = true;
        floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, NULL);

        floor->thinker.function = &T_MoveFloor;
        P_AddThinker(&floor->thinker);

        sec->floordata = floor;
        floor->direction = (Dirn ? 1 : -1);
        floor->sector = sec;

        // setup speed of stair building
        switch (Sped)
        {
            default:
            case SpeedSlow:
                floor->speed = FLOORSPEED / 4;
                break;

            case SpeedNormal:
                floor->speed = FLOORSPEED / 2;
                break;

            case SpeedFast:
                floor->speed = FLOORSPEED * 2;
                break;

            case SpeedTurbo:
                floor->speed = FLOORSPEED * 4;
                break;
        }

        // setup stepsize for stairs
        switch (Step)
        {
            default:
            case 0:
                stairsize = 4 * FRACUNIT;
                break;

            case 1:
                stairsize = 8 * FRACUNIT;
                break;

            case 2:
                stairsize = 16 * FRACUNIT;
                break;

            case 3:
                stairsize = 24 * FRACUNIT;
                break;
        }

        speed = floor->speed;
        height = sec->floorheight + floor->direction * stairsize;
        floor->floordestheight = height;
        texture = sec->floorpic;
        floor->crush = false;
        floor->type = GenBuildStair;    // jff 03/31/98 do not leave uninited

        sec->stairlock = -2;            // jff 02/26/98 set up lock on current sector
        sec->nextsec = -1;
        sec->prevsec = -1;

        osecnum = secnum;               // jff 03/04/98 preserve loop index

        // Find next sector to raise
        // 1. Find 2-sided line with same sector side[0]
        // 2. Other side is the next sector to raise
        do
        {
            okay = false;

            for (int i = 0; i < sec->linecount; i++)
            {
                if (!((sec->lines[i])->backsector))
                    continue;

                tsec = (sec->lines[i])->frontsector;
                newsecnum = tsec->id;

                if (secnum != newsecnum)
                    continue;

                tsec = (sec->lines[i])->backsector;
                newsecnum = tsec->id;

                if (!Igno && tsec->floorpic != texture)
                    continue;

                // jff 06/19/98 prevent double stepsize

                // jff 02/26/98 special lockout condition for retriggering
                if (P_SectorActive(floor_special, tsec) || tsec->stairlock)
                    continue;

                // jff 06/19/98 increase height AFTER continue
                height += floor->direction * stairsize;

                // jff 02/26/98
                // link the stair chain in both directions
                // lock the stair sector until building complete
                sec->nextsec = newsecnum;       // link step to next
                tsec->prevsec = secnum;         // link next back
                tsec->nextsec = -1;             // set next forward link as end
                tsec->stairlock = -2;           // lock the step

                sec = tsec;
                secnum = newsecnum;
                floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, NULL);

                floor->thinker.function = &T_MoveFloor;
                P_AddThinker(&floor->thinker);

                sec->floordata = floor;
                floor->direction = (Dirn ? 1 : -1);
                floor->sector = sec;
                floor->speed = speed;
                floor->floordestheight = height;
                floor->type = GenBuildStair;    // jff 03/31/98 do not leave uninited

                okay = true;
                break;
            }
        } while (okay);

        if (manual)
            return rtn;

        secnum = osecnum;                       // jff 03/04/98 restore old loop index
    }

    // retriggerable generalized stairs build up or down alternately
    if (rtn)
        line->special ^= StairDirection;        // alternate dir on succ activations

    return rtn;
}

//
// EV_DoGenCrusher()
//
// Handle generalized crusher types
//
// Passed the linedef activating the crusher
// Returns true if a thinker created
//
bool EV_DoGenCrusher(line_t *line)
{
    int                 secnum = -1;
    bool                rtn = false;
    bool                manual = false;
    sector_t            *sec;
    ceiling_t           *ceiling;
    const unsigned int  value = line->special - GenCrusherBase;

    // parse the bit fields in the line's special type
    const int           Slnt = (value & CrusherSilent) >> CrusherSilentShift;
    const int           Sped = (value & CrusherSpeed) >> CrusherSpeedShift;
    const int           Trig = (value & TriggerType) >> TriggerTypeShift;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
        {
            manual = true;
            goto manual_crusher;
        }
        else
            return false;
    }

    // jff 02/22/98  Reactivate in-stasis ceilings... for certain types.
    // jff 04/05/98 return if activated
    rtn = P_ActivateInStasisCeiling(line);

    // check if a manual trigger, if so do just the sector on the backside
    if (Trig == PushOnce || Trig == PushMany)
    {
        if (!(sec = line->backsector))
            return rtn;

        secnum = sec->id;
        manual = true;

        goto manual_crusher;
    }

    // if not manual do all sectors tagged the same as the line
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = sectors + secnum;

manual_crusher:
        // Do not start another function if ceiling already moving
        if (P_SectorActive(ceiling_special, sec))
        {
            if (!manual)
                continue;
            else
                return rtn;
        }

        // new ceiling thinker
        rtn = true;
        ceiling = Z_Calloc(1, sizeof(*ceiling), PU_LEVSPEC, NULL);

        ceiling->thinker.function = &T_MoveCeiling;
        P_AddThinker(&ceiling->thinker);

        sec->ceilingdata = ceiling;     // jff 02/22/98
        ceiling->crush = true;
        ceiling->direction = -1;
        ceiling->sector = sec;
        ceiling->texture = sec->ceilingpic;
        ceiling->newspecial = sec->special;
        ceiling->tag = sec->tag;
        ceiling->type = (Slnt ? GenSilentCrusher : GenCrusher);
        ceiling->topheight = sec->ceilingheight;
        ceiling->bottomheight = sec->floorheight + 8 * FRACUNIT;

        // setup ceiling motion speed
        switch (Sped)
        {
            case SpeedSlow:
                ceiling->speed = CEILSPEED;
                break;

            case SpeedNormal:
                ceiling->speed = CEILSPEED * 2;
                break;

            case SpeedFast:
                ceiling->speed = CEILSPEED * 4;
                break;

            case SpeedTurbo:
                ceiling->speed = CEILSPEED * 8;
                break;
        }

        ceiling->oldspeed = ceiling->speed;

        P_AddActiveCeiling(ceiling);    // add to list of active ceilings

        if (manual)
            return rtn;
    }

    return rtn;
}

//
// EV_DoGenLockedDoor()
//
// Handle generalized locked door types
//
// Passed the linedef activating the generalized locked door
// Returns true if a thinker created
//
bool EV_DoGenLockedDoor(line_t *line)
{
    int                 secnum = -1;
    bool                rtn = false;
    sector_t            *sec;
    vldoor_t            *door;
    bool                manual = false;
    const unsigned int  value = line->special - GenLockedBase;

    // parse the bit fields in the line's special type
    const int           Kind = (value & LockedKind) >> LockedKindShift;
    const int           Sped = (value & LockedSpeed) >> LockedSpeedShift;
    const int           Trig = (value & TriggerType) >> TriggerTypeShift;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
        {
            manual = true;
            goto manual_locked;
        }
        else
            return false;
    }

    // check if a manual trigger, if so do just the sector on the backside
    if (Trig == PushOnce || Trig == PushMany)
    {
        if (!(sec = line->backsector))
            return rtn;

        secnum = sec->id;
        manual = true;

        goto manual_locked;
    }

    // if not manual do all sectors tagged the same as the line
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = sectors + secnum;

manual_locked:
        // Do not start another function if ceiling already moving
        if (P_SectorActive(ceiling_special, sec))   // jff 02/22/98
        {
            if (!manual)
                continue;
            else
                return rtn;
        }

        // new door thinker
        rtn = true;
        door = Z_Calloc(1, sizeof(*door), PU_LEVSPEC, NULL);

        door->thinker.function = &T_VerticalDoor;
        P_AddThinker(&door->thinker);

        sec->ceilingdata = door;        // jff 02/22/98
        door->sector = sec;
        door->topwait = VDOORWAIT;
        door->line = line;
        door->topheight = P_FindLowestCeilingSurrounding(sec);
        door->topheight -= 4 * FRACUNIT;
        door->direction = 1;

        // killough 10/98: implement gradual lighting
        if ((line->special & 6) == 6 && line->special > GenLockedBase)
            door->lighttag = line->tag;

        // setup speed of door motion
        switch (Sped)
        {
            default:
            case SpeedSlow:
                door->type = (Kind ? GenOpen : GenRaise);
                door->speed = VDOORSPEED;
                break;

            case SpeedNormal:
                door->type = (Kind ? GenOpen : GenRaise);
                door->speed = VDOORSPEED * 2;
                break;

            case SpeedFast:
                door->type = (Kind ? GenBlazeOpen : GenBlazeRaise);
                door->speed = VDOORSPEED * 4;
                break;

            case SpeedTurbo:
                door->type = (Kind ? GenBlazeOpen : GenBlazeRaise);
                door->speed = VDOORSPEED * 8;
                break;
        }

        // killough 04/15/98: fix generalized door opening sounds
        // (previously they always had the blazing door close sound)
        S_StartSectorSound(&door->sector->soundorg,
            (door->speed >= VDOORSPEED * 4 ? sfx_bdopn : sfx_doropn));

        if (manual)
            return rtn;
    }

    return rtn;
}

//
// EV_DoGenDoor()
//
// Handle generalized door types
//
// Passed the linedef activating the generalized door
// Returns true if a thinker created
//
bool EV_DoGenDoor(line_t *line)
{
    int                 secnum = -1;
    bool                rtn = false;
    sector_t            *sec;
    bool                manual = false;
    vldoor_t            *door;
    const unsigned int  value = line->special - GenDoorBase;

    // parse the bit fields in the line's special type
    const int           Dely = (value & DoorDelay) >> DoorDelayShift;
    const int           Kind = (value & DoorKind) >> DoorKindShift;
    const int           Sped = (value & DoorSpeed) >> DoorSpeedShift;
    const int           Trig = (value & TriggerType) >> TriggerTypeShift;

    if (P_ProcessNoTagLines(line, &sec, &secnum))
    {
        if (zerotag_manual)
        {
            manual = true;
            goto manual_door;
        }
        else
            return false;
    }

    // check if a manual trigger, if so do just the sector on the backside
    if (Trig == PushOnce || Trig == PushMany)
    {
        if (!(sec = line->backsector))
            return rtn;

        secnum = sec->id;
        manual = true;

        goto manual_door;
    }

    // if not manual do all sectors tagged the same as the line
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = sectors + secnum;

manual_door:
        // Do not start another function if ceiling already moving
        if (P_SectorActive(ceiling_special, sec))
        {
            if (!manual)
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

        // setup delay for door remaining open/closed
        switch (Dely)
        {
            default:
            case 0:
                door->topwait = TICRATE;
                break;

            case 1:
                door->topwait = VDOORWAIT;
                break;

            case 2:
                door->topwait = 2 * VDOORWAIT;
                break;

            case 3:
                door->topwait = 7 * VDOORWAIT;
                break;
        }

        // setup speed of door motion
        switch (Sped)
        {
            default:
            case SpeedSlow:
                door->speed = VDOORSPEED;
                break;

            case SpeedNormal:
                door->speed = VDOORSPEED * 2;
                break;

            case SpeedFast:
                door->speed = VDOORSPEED * 4;
                break;

            case SpeedTurbo:
                door->speed = VDOORSPEED * 8;
                break;
        }

        door->line = line;      // jff 01/31/98 remember line that triggered us

        // killough 10/98: implement gradual lighting
        if ((line->special & 6) == 6 && line->special > GenLockedBase)
            door->lighttag = line->tag;

        // set kind of door, whether it opens then close, opens, closes etc.
        // assign target heights accordingly
        switch (Kind)
        {
            case OdCDoor:
                door->direction = 1;
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;

                if (door->topheight != sec->ceilingheight)
                    S_StartSectorSound(&door->sector->soundorg,
                        (door->speed >= VDOORSPEED * 4 ? sfx_bdopn : sfx_doropn));

                door->type = (Sped >= SpeedFast ? GenBlazeRaise : GenRaise);
                break;

            case ODoor:
                door->direction = 1;
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;

                if (door->topheight != sec->ceilingheight)
                    S_StartSectorSound(&door->sector->soundorg,
                        (door->speed >= VDOORSPEED * 4 ? sfx_bdopn : sfx_doropn));

                door->type = (Sped >= SpeedFast ? GenBlazeOpen : GenOpen);
                break;

            case CdODoor:
                door->topheight = sec->ceilingheight;
                door->direction = -1;
                S_StartSectorSound(&door->sector->soundorg,
                    (door->speed >= VDOORSPEED * 4 ? sfx_bdcls : sfx_dorcls));
                door->type = (Sped >= SpeedFast ? GenBlazeCdO : GenCdO);
                break;

            case CDoor:
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                door->direction = -1;
                S_StartSectorSound(&door->sector->soundorg,
                    (door->speed >= VDOORSPEED * 4 ? sfx_bdcls : sfx_dorcls));
                door->type = (Sped >= SpeedFast ? GenBlazeClose : GenClose);
                break;
        }

        if (manual)
            return rtn;
    }

    return rtn;
}
