/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_system.h"
#include "i_swap.h"
#include "p_local.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

// killough 2/8/98: Remove switch limit
static int          *switchlist;        // killough
static int          numswitches;        // killough

button_t            *buttonlist = NULL;
int                 maxbuttons = MAXBUTTONS;

extern texture_t    **textures;

//
// P_InitSwitchList()
//
// Only called at game initialization in order to list the set of switches
// and buttons known to the engine. This enables their texture to change
// when activated, and in the case of buttons, change back after a timeout.
//
// This routine modified to read its data from a predefined lump or
// PWAD lump called SWITCHES rather than a static table in this module to
// allow wad designers to insert or modify switches.
//
// Lump format is an array of byte packed switchlist_t structures, terminated
// by a structure with episode == -1. The lump can be generated from a
// text source file using SWANTBLS.EXE, distributed with the BOOM utils.
// The standard list of switches and animations is contained in the example
// source text file DEFSWANI.DAT also in the BOOM util distribution.
//
// Rewritten by Lee Killough to remove limit 2/8/98
//
void P_InitSwitchList(void)
{
    int             index = 0;
    int             episode = (gamemode == registered || gamemode == retail ? 2 : (gamemode == commercial ? 3 : 1));
    switchlist_t    *alphSwitchList;                        // jff 3/23/98 pointer to switch table
    int             lump = W_GetNumForName("SWITCHES");     // cph - new wad lump handling

    // jff 3/23/98 read the switch table from a predefined lump
    alphSwitchList = (switchlist_t *)W_CacheLumpNum(lump);

    for (int i = 0; ; i++)
    {
        static int  max_numswitches;

        if (index + 1 >= max_numswitches)
            switchlist = I_Realloc(switchlist, sizeof(*switchlist) * (max_numswitches = (max_numswitches ? max_numswitches * 2 : 8)));

        if (SHORT(alphSwitchList[i].episode) <= episode)    // jff 5/11/98 endianness
        {
            int texture1;
            int texture2;

            if (!SHORT(alphSwitchList[i].episode))
                break;

            // Ignore switches referencing unknown texture names, instead of exiting.
            // Warn if either one is missing, but only add if both are valid.
            if ((texture1 = R_CheckTextureNumForName(alphSwitchList[i].name1)) == -1)
                C_Warning("Switch %i in the <b>SWITCHES</b> lump has an unknown texture of <b>%s</b>.", i, alphSwitchList[i].name1);

            if ((texture2 = R_CheckTextureNumForName(alphSwitchList[i].name2)) == -1)
                C_Warning("Switch %i in the <b>SWITCHES</b> lump has an unknown texture of <b>%s</b>.", i, alphSwitchList[i].name2);

            if (texture1 != -1 && texture2 != -1)
            {
                texture_t   *texture;

                switchlist[index++] = texture1;
                switchlist[index++] = texture2;

                texture = textures[texture1];

                for (int j = 0; j < texture->patchcount; j++)
                    W_CacheLumpNum(texture->patches[j].patch);

                texture = textures[texture2];

                for (int j = 0; j < texture->patchcount; j++)
                    W_CacheLumpNum(texture->patches[j].patch);
            }
        }
    }

    numswitches = index / 2;
    switchlist[index] = -1;
    W_ReleaseLumpNum(lump);

    buttonlist = calloc(maxbuttons, sizeof(*buttonlist));
}

//
// Start a button counting down until it turns off.
//
void P_StartButton(line_t *line, bwhere_e where, int texture, int time)
{
    // See if button is already pressed
    for (int i = 0; i < maxbuttons; i++)
        if (buttonlist[i].btimer && buttonlist[i].line == line)
            return;

    for (int i = 0; i < maxbuttons; i++)
        if (!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].where = where;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = &line->soundorg;
            return;
        }

    // [crispy] remove MAXBUTTONS limit
    maxbuttons *= 2;
    buttonlist = I_Realloc(buttonlist, sizeof(*buttonlist) * maxbuttons);
    memset(buttonlist + maxbuttons / 2, 0, sizeof(*buttonlist) * (maxbuttons - maxbuttons / 2));
    P_StartButton(line, where, texture, time);
}

//
// Function that changes wall texture.
// Tell it if switch is ok to use again (true=yes, it's a button).
//
void P_ChangeSwitchTexture(line_t *line, dboolean useagain)
{
    int     sidenum = line->sidenum[0];
    short   *toptexture = &sides[sidenum].toptexture;
    short   *midtexture = &sides[sidenum].midtexture;
    short   *bottomtexture = &sides[sidenum].bottomtexture;

    if (!useagain)
        line->special = 0;

    for (int i = 0; i < numswitches * 2; i++)
    {
        bwhere_e    where = nowhere;

        if (switchlist[i] == *bottomtexture)
        {
            where = bottom;
            *bottomtexture = switchlist[i ^ 1];
        }

        if (switchlist[i] == *midtexture)
        {
            where = middle;
            *midtexture = switchlist[i ^ 1];
        }

        if (switchlist[i] == *toptexture)
        {
            where = top;
            *toptexture = switchlist[i ^ 1];
        }

        if (where != nowhere)
        {
            if (useagain)
                P_StartButton(line, where, switchlist[i], BUTTONTIME);

            S_StartSectorSound(&line->soundorg, sfx_swtchn);
            break;
        }
    }
}

//
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
//
dboolean P_UseSpecialLine(mobj_t *thing, line_t *line, int side)
{
    if (side)
        return false;

    // jff 02/04/98 add check here for generalized floor/ceil mover
    if (line->special >= GenCrusherBase)
    {
        // pointer to line function is NULL by default, set non-null if
        // line special is push or switch generalized linedef type
        dboolean (*linefunc)(line_t *line) = NULL;

        // check each range of generalized linedefs
        if (line->special >= GenFloorBase)
        {
            if (!thing->player)
                if ((line->special & FloorChange) || !(line->special & FloorModel))
                    return false;                       // FloorModel is "Allow Monsters" if FloorChange is 0

            if (!line->tag && (line->special & 6) != 6) // jff 2/27/98 all non-manual
                return false;                           // generalized types require tag

            linefunc = EV_DoGenFloor;
        }
        else if (line->special >= GenCeilingBase)
        {
            if (!thing->player)
                if ((line->special & CeilingChange) || !(line->special & CeilingModel))
                    return false;                       // CeilingModel is "Allow Monsters" if CeilingChange is 0

            linefunc = EV_DoGenCeiling;
        }
        else if (line->special >= GenDoorBase)
        {
            if (!thing->player)
            {
                if (!(line->special & DoorMonster))
                    return false;                       // monsters disallowed from this door

                if (line->flags & ML_SECRET)            // they can't open secret doors either
                    return false;
            }

            linefunc = EV_DoGenDoor;
        }
        else if (line->special >= GenLockedBase)
        {
            if (!thing->player)
                return false;                           // monsters disallowed from unlocking doors

            if (!P_CanUnlockGenDoor(line))
                return false;

            linefunc = EV_DoGenLockedDoor;
        }
        else if (line->special >= GenLiftBase)
        {
            if (!thing->player)
                if (!(line->special & LiftMonster))
                    return false;                       // monsters disallowed

            linefunc = EV_DoGenLift;
        }
        else if (line->special >= GenStairsBase)
        {
            if (!thing->player)
                if (!(line->special & StairMonster))
                    return false;                       // monsters disallowed

            linefunc = EV_DoGenStairs;
        }
        else
        {
            if (!thing->player)
                if (!(line->special & CrusherMonster))
                    return false;                       // monsters disallowed

            linefunc = EV_DoGenCrusher;
        }

        if (linefunc)
            switch ((line->special & TriggerType) >> TriggerTypeShift)
            {
                case PushOnce:
                    if (linefunc(line))
                        line->special = 0;

                    return true;

                case PushMany:
                    linefunc(line);
                    return true;

                case SwitchOnce:
                    if (linefunc(line))
                        P_ChangeSwitchTexture(line, false);

                    return true;

                case SwitchMany:
                    if (linefunc(line))
                        P_ChangeSwitchTexture(line, true);

                    return true;

                default:
                    // if not a switch/push type, do nothing here
                    return false;
            }
    }

    // Switches that other things can activate.
    if (!thing->player)
    {
        // never open secret doors
        if (line->flags & ML_SECRET)
            return false;

        switch (line->special)
        {
            case DR_Door_OpenWaitClose_AlsoMonsters:
            case D1_Door_Blue_OpenStay:
            case D1_Door_Red_OpenStay:
            case D1_Door_Yellow_OpenStay:

            // jff 3/5/98 add ability to use teleporters for monsters
            case SR_Teleport_AlsoMonsters:
            case S1_Teleport_AlsoMonsters:
            case S1_Teleport_AlsoMonsters_Silent_SameAngle:
            case SR_Teleport_AlsoMonsters_Silent_SameAngle:
                break;

            default:
                return false;
        }
    }

    if (!P_CheckTag(line))      // jff 2/27/98 disallow zero tag on some types
        return false;

    // do something
    switch (line->special)
    {
        // MANUALS
        case DR_Door_OpenWaitClose_Fast:
            if (nomonsters && (line->flags & ML_TRIGGER666))
            {
                line_t  junk;

                junk.tag = 666;
                EV_DoFloor(&junk, lowerFloorToLowest);
                line->flags &= ~ML_TRIGGER666;
            }

            EV_VerticalDoor(line, thing);
            break;

        case D1_Door_Blue_OpenStay:
        case D1_Door_Red_OpenStay:
        case D1_Door_Yellow_OpenStay:
            if (!thing->player)
                return false;

            EV_VerticalDoor(line, thing);
            break;

        case DR_Door_Blue_OpenWaitClose:
        case DR_Door_Yellow_OpenWaitClose:
        case DR_Door_Red_OpenWaitClose:

        case DR_Door_OpenWaitClose_AlsoMonsters:
        case D1_Door_OpenStay_Fast:
        case D1_Door_OpenStay:
            EV_VerticalDoor(line, thing);
            break;

        // Switches
        case S1_Stairs_RaiseBy8:
            if (EV_BuildStairs(line, FLOORSPEED / 4, 8 * FRACUNIT, false))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseDonut_ChangesTexture:
            if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_ExitLevel:
            P_ChangeSwitchTexture(line, false);
            G_ExitLevel();
            break;

        case S1_Floor_RaiseBy32_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseBy24_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseToNextHighestFloor:
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Lift_LowerWaitRaise:
            if (EV_DoPlat(line, downWaitUpStay, 0))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_LowerToLowestFloor:
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, false);

            if (nomonsters && (line->flags & ML_TRIGGER666))
            {
                line_t  junk;

                junk.tag = 666;
                EV_DoFloor(&junk, lowerFloorToLowest);
                junk.tag = 667;
                EV_DoFloor(&junk, raiseToTexture);
                line->flags &= ~ML_TRIGGER666;
            }

            break;

        case S1_Door_OpenWaitClose:
            if (EV_DoDoor(line, doorNormal, VDOORSPEED))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Ceiling_LowerToFloor:
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_LowerTo8AboveHighestFloor:
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Ceiling_LowerTo8AboveFloor_PerpetualSlowCrusherDamage:
            if (EV_DoCeiling(line, crushAndRaise))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Door_CloseStay:
            if (EV_DoDoor(line, doorClose, VDOORSPEED))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_ExitLevel_GoesToSecretLevel:
            P_ChangeSwitchTexture(line, false);
            G_SecretExitLevel();
            break;

        case S1_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_LowerToHighestFloor:
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, false);
            break;

        case S1_Door_OpenStay:
            if (EV_DoDoor(line, doorOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Door_OpenWaitClose_Fast:
            if (EV_DoDoor(line, doorBlazeRaise, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Door_OpenStay_Fast:
            if (EV_DoDoor(line, doorBlazeOpen, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Door_CloseStay_Fast:
            if (EV_DoDoor(line, doorBlazeClose, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Lift_LowerWaitRaise_Fast:
            if (EV_DoPlat(line, blazeDWUS, 0))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Stairs_RaiseBy16_Fast:
            if (EV_BuildStairs(line, FLOORSPEED * 4, 16 * FRACUNIT, true))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseToNextHighestFloor_Fast:
            if (EV_DoFloor(line, raiseFloorTurbo))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Door_Blue_OpenStay_Fast:
        case S1_Door_Red_OpenStay_Fast:
        case S1_Door_Yellow_OpenStay_Fast:
            if (EV_DoLockedDoor(line, doorBlazeOpen, thing, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseBy512:
            if (EV_DoFloor(line, raiseFloor512))
                P_ChangeSwitchTexture(line, false);

            break;

        // Extended switches
        case S1_Floor_RaiseByShortestLowerTexture:
            if (EV_DoFloor(line, raiseToTexture))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_LowerToLowestFloor_ChangesTexture:
            if (EV_DoFloor(line, lowerAndChange))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseBy24_ChangesTextureAndEffect:
            if (EV_DoFloor(line, raiseFloor24AndChange))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_RaiseBy24:
            if (EV_DoFloor(line, raiseFloor24))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Lift_PerpetualLowestAndHighestFloors:
            if (EV_DoPlat(line, perpetualRaise, 0))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Lift_Stop:
            EV_StopPlat(line);
            P_ChangeSwitchTexture(line, false);
            break;

        case S1_Crusher_Start_Fast:
            if (EV_DoCeiling(line, fastCrushAndRaise))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Crusher_Start_Silent:
            if (EV_DoCeiling(line, silentCrushAndRaise))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Ceiling_RaiseToHighestCeiling:
            if (EV_DoCeiling(line, raiseToHighest) | EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Ceiling_LowerTo8AboveFloor:
            if (EV_DoCeiling(line, lowerAndCrush))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Crusher_Stop:
            if (EV_CeilingCrushStop(line))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Light_ChangeToBrightestAdjacent:
            EV_LightTurnOn(line, 0);
            P_ChangeSwitchTexture(line, false);
            break;

        case S1_Light_ChangeTo35:
            EV_LightTurnOn(line, 35);
            P_ChangeSwitchTexture(line, false);
            break;

        case S1_Light_ChangeTo255:
            EV_LightTurnOn(line, 255);
            P_ChangeSwitchTexture(line, false);
            break;

        case S1_Light_StartBlinking:
            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line, false);
            break;

        case S1_Light_ChangeToDarkestAdjacent:
            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line, false);
            break;

        case S1_Teleport_AlsoMonsters:
            if (EV_Teleport(line, side, thing))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Door_CloseWaitOpen_30Seconds:
            if (EV_DoDoor(line, doorClose30ThenOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_ChangeTextureAndEffect:
            if (EV_DoChange(line, trigChangeOnly))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Ceiling_LowerToLowestCeiling:
            if (EV_DoCeiling(line, lowerToLowest))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Ceiling_LowerToHighestFloor:
            if (EV_DoCeiling(line, lowerToMaxFloor))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Teleport_AlsoMonsters_Silent_SameAngle:
            if (EV_SilentTeleport(line, side, thing))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_ChangeTextureAndEffectToNearest:
            if (EV_DoChange(line, numChangeOnly))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, lowerFloorToNearest))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Lift_RaiseToNextHighestFloor_Fast:
            if (EV_DoElevator(line, elevateUp))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Lift_LowerToNextLowestFloor_Fast:
            if (EV_DoElevator(line, elevateDown))
                P_ChangeSwitchTexture(line, false);

            break;

        case S1_Lift_MoveToSameFloorHeight_Fast:
            if (EV_DoElevator(line, elevateCurrent))
                P_ChangeSwitchTexture(line, false);

            break;

        case SR_ChangeTextureAndEffectToNearest:
            if (EV_DoChange(line, numChangeOnly))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseByShortestLowerTexture:
            if (EV_DoFloor(line, raiseToTexture))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_LowerToLowestFloor_ChangesTexture:
            if (EV_DoFloor(line, lowerAndChange))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseBy512:
            if (EV_DoFloor(line, raiseFloor512))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseBy512_ChangesTextureAndEffect:
            if (EV_DoFloor(line, raiseFloor24AndChange))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseBy24:
            if (EV_DoFloor(line, raiseFloor24))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Lift_PerpetualLowestAndHighestFloors:
            EV_DoPlat(line, perpetualRaise, 0);
            P_ChangeSwitchTexture(line, true);
            break;

        case SR_Lift_Stop:
            EV_StopPlat(line);
            P_ChangeSwitchTexture(line, true);
            break;

        case SR_Crusher_Start_Fast:
            if (EV_DoCeiling(line, fastCrushAndRaise))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Crusher_Start:
            if (EV_DoCeiling(line, crushAndRaise))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Crusher_Start_Silent:
            if (EV_DoCeiling(line, silentCrushAndRaise))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Ceiling_RaiseToHighestCeiling:
            if (EV_DoCeiling(line, raiseToHighest) | EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Ceiling_LowerTo8AboveFloor:
            if (EV_DoCeiling(line, lowerAndCrush))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Crusher_Stop:
            if (EV_CeilingCrushStop(line))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseDonut_ChangesTexture:
            if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Light_ChangeToBrightestAdjacent:
            EV_LightTurnOn(line, 0);
            P_ChangeSwitchTexture(line, true);
            break;

        case SR_Light_StartBlinking:
            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line, true);
            break;

        case SR_Light_ChangeToDarkestAdjacent:
            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line, true);
            break;

        case SR_Teleport_AlsoMonsters:
            if (EV_Teleport(line, side, thing))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Door_CloseWaitOpen_30Seconds:
            if (EV_DoDoor(line, doorClose30ThenOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Ceiling_LowerToLowestCeiling:
            if (EV_DoCeiling(line, lowerToLowest))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Ceiling_LowerToHighestFloor:
            if (EV_DoCeiling(line, lowerToMaxFloor))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Teleport_AlsoMonsters_Silent_SameAngle:
            if (EV_SilentTeleport(line, side, thing))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Lift_RaiseToCeiling_Instantly:
            if (EV_DoPlat(line, toggleUpDn, 0))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, lowerFloorToNearest))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Lift_RaiseToNextHighestFloor_Fast:
            if (EV_DoElevator(line, elevateUp))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Lift_LowerToNextLowestFloor_Fast:
            if (EV_DoElevator(line, elevateDown))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Lift_MoveToSameFloorHeight_Fast:
            if (EV_DoElevator(line, elevateCurrent))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Stairs_RaiseBy8:
            if (EV_BuildStairs(line, FLOORSPEED / 4, 8 * FRACUNIT, false))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Stairs_RaiseBy16_Fast:
            if (EV_BuildStairs(line, FLOORSPEED * 4, 16 * FRACUNIT, true))
                P_ChangeSwitchTexture(line, true);

            break;

        // Buttons (retriggerable switches)
        case SR_Door_CloseStay:
            if (EV_DoDoor(line, doorClose, VDOORSPEED))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Ceiling_LowerToFloor:
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_LowerToHighestFloor:
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_LowerToLowestFloor:
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Door_OpenStay:
            if (EV_DoDoor(line, doorOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Lift_LowerWaitRaise:
            if (EV_DoPlat(line, downWaitUpStay, 1))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Door_OpenWaitClose:
            if (EV_DoDoor(line, doorNormal, VDOORSPEED))
                P_ChangeSwitchTexture(line, true);
            else if (thing->player)
                S_StartSound(thing, sfx_oof);

            break;

        case SR_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseBy24_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseBy32_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseToNextHighestFloor:
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_LowerTo8AboveHighestFloor:
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Door_OpenWaitClose_Fast:
            if (EV_DoDoor(line, doorBlazeRaise, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Door_OpenStay_Fast:
            if (EV_DoDoor(line, doorBlazeOpen, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Door_CloseStay_Fast:
            if (EV_DoDoor(line, doorBlazeClose, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Lift_LowerWaitRaise_Fast:
            if (EV_DoPlat(line, blazeDWUS, 0))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Floor_RaiseToNextHighestFloor_Fast:
            if (EV_DoFloor(line, raiseFloorTurbo))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Door_Blue_OpenStay_Fast:
        case SR_Door_Red_OpenStay_Fast:
        case SR_Door_Yellow_OpenStay_Fast:
            if (EV_DoLockedDoor(line, doorBlazeOpen, thing, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, true);

            break;

        case SR_Light_ChangeTo255:
            EV_LightTurnOn(line, 255);
            P_ChangeSwitchTexture(line, true);

            break;

        case SR_Light_ChangeTo35:
            EV_LightTurnOn(line, (canmodify && gamemission == doom2 && gamemap == 4 ? 0 : 35));
            P_ChangeSwitchTexture(line, true);
            break;
    }

    return true;
}
