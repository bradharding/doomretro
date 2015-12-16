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

#include <string.h>

#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "i_system.h"
#include "p_local.h"
#include "s_sound.h"

int                     switchlist[MAXSWITCHES * 2 + 1];

extern int              numtextures;
extern texture_t        **textures;

//
// P_InitSwitchList
// Only called at game initialization.
// For each SW1xxxxx texture we look for a corresponding SW2xxxxx texture. (JAD 27/09/11)
//
void P_InitSwitchList(void)
{
    int         i = 0;
    int         count = 0;
    int         *ptr_B;
    texture_t   **ptr_1;
    char        sw2name[9];

    sw2name[8] = 0;

    ptr_1 = textures;
    ptr_B = switchlist;

    do
    {
        texture_t       *ptr_2 = *ptr_1++;

        if (!strncasecmp(ptr_2->name, "SW1", 3))
        {
            int j;

            strncpy(sw2name, ptr_2->name, 8);
            sw2name[2] = '2';
            j = R_CheckTextureNumForName(sw2name);
            if (j != -1)
            {
                if (count < MAXSWITCHES)
                {
                    *ptr_B++ = i;
                    *ptr_B++ = j;
                }
                count++;
            }
        }
    }
    while (++i < numtextures);

    *ptr_B = -1;
}

//
// Start a button counting down till it turns off.
//
void P_StartButton(line_t *line, bwhere_e w, int texture, int time)
{
    int i;

    // See if button is already pressed
    for (i = 0; i < MAXBUTTONS; i++)
        if (buttonlist[i].btimer && buttonlist[i].line == line)
            return;

    for (i = 0; i < MAXBUTTONS; i++)
        if (!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].where = w;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = &line->soundorg;
            return;
        }

    I_Error("P_StartButton: no button slots left!");
}

//
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
//
void P_ChangeSwitchTexture(line_t *line, int useAgain)
{
    int         i = 0;
    int         swtex;
    side_t      *side = &sides[line->sidenum[0]];
    short       texTop = side->toptexture;
    short       texMid = side->midtexture;
    short       texBot = side->bottomtexture;

    // don't zero line->special until after exit switch test
    if (!useAgain)
        line->special = 0;

    // search for a texture to change
    do
    {
        swtex = switchlist[i];

        if (swtex == texTop)
        {
            S_StartSectorSound(&line->soundorg, sfx_swtchn);
            if (useAgain)
                P_StartButton(line, top, swtex, BUTTONTIME);
            side->toptexture = switchlist[i ^ 1];
            break;
        }
        else if (swtex == texMid)
        {
            S_StartSectorSound(&line->soundorg, sfx_swtchn);
            if (useAgain)
                P_StartButton(line, middle, swtex, BUTTONTIME);
            side->midtexture = switchlist[i ^ 1];
            break;
        }
        else if (swtex == texBot)
        {
            S_StartSectorSound(&line->soundorg, sfx_swtchn);
            if (useAgain)
                P_StartButton(line, bottom, swtex, BUTTONTIME);
            side->bottomtexture = switchlist[i ^ 1];
            break;
        }
        i++;
    } while (swtex != -1);
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

    //jff 02/04/98 add check here for generalized floor/ceil mover
    {
        // pointer to line function is NULL by default, set non-null if
        // line special is push or switch generalized linedef type
        dboolean (*linefunc)(line_t *line) = NULL;

        // check each range of generalized linedefs
        if ((unsigned int)line->special >= GenFloorBase)
        {
            if (!thing->player)
                if ((line->special & FloorChange) || !(line->special & FloorModel))
                    return false;       // FloorModel is "Allow Monsters" if FloorChange is 0
            if (!line->tag && ((line->special & 6) != 6))       // jff 2/27/98 all non-manual
                return false;                                   // generalized types require tag
            linefunc = EV_DoGenFloor;
        }
        else if ((unsigned int)line->special >= GenCeilingBase)
        {
            if (!thing->player)
                if ((line->special & CeilingChange) || !(line->special & CeilingModel))
                    return false;       // CeilingModel is "Allow Monsters" if CeilingChange is 0
            linefunc = EV_DoGenCeiling;
        }
        else if ((unsigned int)line->special >= GenDoorBase)
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
        else if ((unsigned int)line->special >= GenLockedBase)
        {
            if (!thing->player)
                return false;                           // monsters disallowed from unlocking doors
            if (!P_CanUnlockGenDoor(line, thing->player))
                return false;
            linefunc = EV_DoGenLockedDoor;
        }
        else if ((unsigned int)line->special >= GenLiftBase)
        {
            if (!thing->player)
                if (!(line->special & LiftMonster))
                    return false;                               // monsters disallowed
            linefunc = EV_DoGenLift;
        }
        else if ((unsigned int)line->special >= GenStairsBase)
        {
            if (!thing->player)
                if (!(line->special & StairMonster))
                    return false;                               // monsters disallowed
            linefunc = EV_DoGenStairs;
        }
        else if ((unsigned int)line->special >= GenCrusherBase)
        {
            if (!thing->player)
                if (!(line->special & CrusherMonster))
                    return false;                               // monsters disallowed
            linefunc = EV_DoGenCrusher;
        }

        if (linefunc)
            switch ((line->special & TriggerType) >> TriggerTypeShift)
            {
                case PushOnce:
                    if (!side)
                        if (linefunc(line))
                            line->special = 0;
                    return true;
                case PushMany:
                    if (!side)
                        linefunc(line);
                    return true;
                case SwitchOnce:
                    if (linefunc(line))
                        P_ChangeSwitchTexture(line, 0);
                    return true;
                case SwitchMany:
                    if (linefunc(line))
                        P_ChangeSwitchTexture(line, 1);
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

            //jff 3/5/98 add ability to use teleporters for monsters
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
        case DR_Door_OpenWaitClose_AlsoMonsters:
        case DR_Door_Blue_OpenWaitClose:
        case DR_Door_Yellow_OpenWaitClose:
        case DR_Door_Red_OpenWaitClose:

        case D1_Door_OpenStay:
        case D1_Door_Blue_OpenStay:
        case D1_Door_Red_OpenStay:
        case D1_Door_Yellow_OpenStay:

        case DR_Door_OpenWaitClose_Fast:
            if (nomonsters && (line->flags & ML_TRIGGER666))
            {
                line_t  junk;

                junk.tag = 666;
                EV_DoFloor(&junk, lowerFloorToLowest);
                line->flags &= ~ML_TRIGGER666;
            }
        case D1_Door_OpenStay_Fast:
            EV_VerticalDoor(line, thing);
            break;

        // Switches
        case S1_Stairs_RaiseBy8:
            if (EV_BuildStairs(line, build8))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseDonut_ChangesTexture:
            if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_ExitLevel:
            if (thing->player && thing->player->health <= 0)
            {
                S_StartSound(thing, sfx_noway);
                return false;
            }
            P_ChangeSwitchTexture(line, 0);
            G_ExitLevel();
            break;

        case S1_Floor_RaiseBy32_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseBy24_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseToNextHighestFloor:
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Lift_LowerWaitRaise:
            if (EV_DoPlat(line, downWaitUpStay, 0))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_LowerToLowestFloor:
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 0);

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
            if (EV_DoDoor(line, doorNormal))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Ceiling_LowerToFloor:
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_LowerTo8AboveHighestFloor:
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Ceiling_LowerTo8AboveFloor_PerpetualSlowCrusherDamage:
            if (EV_DoCeiling(line, crushAndRaise))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Door_CloseStay:
            if (EV_DoDoor(line, doorClose))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_ExitLevel_GoesToSecretLevel:
            if (thing->player && thing->player->health <= 0)
            {
                S_StartSound(thing, sfx_noway);
                return false;
            }
            P_ChangeSwitchTexture(line, 0);
            G_SecretExitLevel();
            break;

        case S1_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_LowerToHighestFloor:
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Door_OpenStay:
            if (EV_DoDoor(line, doorOpen))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Door_OpenWaitClose_Fast:
            if (EV_DoDoor(line, doorBlazeRaise))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Door_OpenStay_Fast:
            if (EV_DoDoor(line, doorBlazeOpen))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Door_CloseStay_Fast:
            if (EV_DoDoor(line, doorBlazeClose))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Lift_LowerWaitRaise_Fast:
            if (EV_DoPlat(line, blazeDWUS, 0))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Stairs_RaiseBy16_Fast:
            if (EV_BuildStairs(line, turbo16))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseToNextHighestFloor_Fast:
            if (EV_DoFloor(line, raiseFloorTurbo))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Door_Blue_OpenStay_Fast:
        case S1_Door_Red_OpenStay_Fast:
        case S1_Door_Yellow_OpenStay_Fast:
            if (EV_DoLockedDoor(line, doorBlazeOpen, thing))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseBy512:
            if (EV_DoFloor(line, raiseFloor512))
                P_ChangeSwitchTexture(line, 0);
            break;

        // Extended switches
        case S1_Floor_RaiseByShortestLowerTexture:
            if (EV_DoFloor(line, raiseToTexture))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_LowerToLowestFloor_ChangesTexture:
            if (EV_DoFloor(line, lowerAndChange))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseBy24_ChangesTextureAndEffect:
            if (EV_DoFloor(line, raiseFloor24AndChange))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_RaiseBy24:
            if (EV_DoFloor(line, raiseFloor24))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Lift_PerpetualLowestAndHighestFloors:
            if (EV_DoPlat(line, perpetualRaise, 0))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Lift_Stop:
            EV_StopPlat(line);
            P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Crusher_Start_Fast:
            if (EV_DoCeiling(line, fastCrushAndRaise))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Crusher_Start_Silent:
            if (EV_DoCeiling(line, silentCrushAndRaise))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Ceiling_RaiseToHighestCeiling:
            if (EV_DoCeiling(line, raiseToHighest) || EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Ceiling_LowerTo8AboveFloor:
            if (EV_DoCeiling(line, lowerAndCrush))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Crusher_Stop:
            if (EV_CeilingCrushStop(line))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Light_ChangeToBrightestAdjacent:
            EV_LightTurnOn(line, 0);
            P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Light_ChangeTo35:
            EV_LightTurnOn(line, 35);
            P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Light_ChangeTo255:
            EV_LightTurnOn(line, 255);
            P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Light_StartBlinking:
            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Light_ChangeToDarkestAdjacent:
            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Teleport_AlsoMonsters:
            if (EV_Teleport(line, side, thing))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Door_CloseWaitOpen_30Seconds:
            if (EV_DoDoor(line, doorClose30ThenOpen))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_ChangeTextureAndEffect:
            if (EV_DoChange(line, trigChangeOnly))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Ceiling_LowerToLowestCeiling:
            if (EV_DoCeiling(line, lowerToLowest))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Ceiling_LowerToHighestFloor:
            if (EV_DoCeiling(line, lowerToMaxFloor))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Teleport_AlsoMonsters_Silent_SameAngle:
            if (EV_SilentTeleport(line, side, thing))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_ChangeTextureAndEffectToNearest:
            if (EV_DoChange(line, numChangeOnly))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, lowerFloorToNearest))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Lift_RaiseToNextHighestFloor_Fast:
            if (EV_DoElevator(line, elevateUp))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Lift_LowerToNextLowestFloor_Fast:
            if (EV_DoElevator(line, elevateDown))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_Lift_MoveToSameFloorHeight_Fast:
            if (EV_DoElevator(line, elevateCurrent))
                P_ChangeSwitchTexture(line, 0);
            break;

        case SR_ChangeTextureAndEffectToNearest:
            if (EV_DoChange(line, numChangeOnly))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseByShortestLowerTexture:
            if (EV_DoFloor(line, raiseToTexture))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_LowerToLowestFloor_ChangesTexture:
            if (EV_DoFloor(line, lowerAndChange))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseBy512:
            if (EV_DoFloor(line, raiseFloor512))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseBy512_ChangesTextureAndEffect:
            if (EV_DoFloor(line, raiseFloor24AndChange))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseBy24:
            if (EV_DoFloor(line, raiseFloor24))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Lift_PerpetualLowestAndHighestFloors:
            EV_DoPlat(line, perpetualRaise, 0);
            P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Lift_Stop:
            EV_StopPlat(line);
            P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Crusher_Start_Fast:
            if (EV_DoCeiling(line, fastCrushAndRaise))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Crusher_Start:
            if (EV_DoCeiling(line, crushAndRaise))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Crusher_Start_Silent:
            if (EV_DoCeiling(line, silentCrushAndRaise))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Ceiling_RaiseToHighestCeiling:
            if (EV_DoCeiling(line, raiseToHighest) || EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Ceiling_LowerTo8AboveFloor:
            if (EV_DoCeiling(line, lowerAndCrush))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Crusher_Stop:
            if (EV_CeilingCrushStop(line))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseDonut_ChangesTexture:
            if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Light_ChangeToBrightestAdjacent:
            EV_LightTurnOn(line, 0);
            P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Light_StartBlinking:
            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Light_ChangeToDarkestAdjacent:
            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Teleport_AlsoMonsters:
            if (EV_Teleport(line, side, thing))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Door_CloseWaitOpen_30Seconds:
            if (EV_DoDoor(line, doorClose30ThenOpen))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Ceiling_LowerToLowestCeiling:
            if (EV_DoCeiling(line, lowerToLowest))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Ceiling_LowerToHighestFloor:
            if (EV_DoCeiling(line, lowerToMaxFloor))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Teleport_AlsoMonsters_Silent_SameAngle:
            if (EV_SilentTeleport(line, side, thing))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Lift_RaiseToCeiling_Instantly:
            if (EV_DoPlat(line, toggleUpDn, 0))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, lowerFloorToNearest))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Lift_RaiseToNextHighestFloor_Fast:
            if (EV_DoElevator(line, elevateUp))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Lift_LowerToNextLowestFloor_Fast:
            if (EV_DoElevator(line, elevateDown))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Lift_MoveToSameFloorHeight_Fast:
            if (EV_DoElevator(line, elevateCurrent))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Stairs_RaiseBy8:
            if (EV_BuildStairs(line, build8))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Stairs_RaiseBy16_Fast:
            if (EV_BuildStairs(line, turbo16))
                P_ChangeSwitchTexture(line, 1);
            break;

        // Buttons (retriggerable switches)
        case SR_Door_CloseStay:
            if (EV_DoDoor(line, doorClose))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Ceiling_LowerToFloor:
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_LowerToHighestFloor:
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_LowerToLowestFloor:
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Door_OpenStay:
            if (EV_DoDoor(line, doorOpen))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Lift_LowerWaitRaise:
            if (EV_DoPlat(line, downWaitUpStay, 1))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Door_OpenWaitClose:
            if (EV_DoDoor(line, doorNormal))
                P_ChangeSwitchTexture(line, 1);
            else if (thing->player)
                S_StartSound(thing, sfx_oof);
            break;

        case SR_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseBy24_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseBy32_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseToNextHighestFloor:
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_LowerTo8AboveHighestFloor:
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Door_OpenWaitClose_Fast:
            if (EV_DoDoor(line, doorBlazeRaise))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Door_OpenStay_Fast:
            if (EV_DoDoor(line, doorBlazeOpen))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Door_CloseStay_Fast:
            if (EV_DoDoor(line, doorBlazeClose))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Lift_LowerWaitRaise_Fast:
            if (EV_DoPlat(line, blazeDWUS, 0))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Floor_RaiseToNextHighestFloor_Fast:
            if (EV_DoFloor(line, raiseFloorTurbo))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Door_Blue_OpenStay_Fast:
        case SR_Door_Red_OpenStay_Fast:
        case SR_Door_Yellow_OpenStay_Fast:
            if (EV_DoLockedDoor(line, doorBlazeOpen, thing))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Light_ChangeTo255:
            EV_LightTurnOn(line, 255);
            P_ChangeSwitchTexture(line, 1);
            break;

        case SR_Light_ChangeTo35:
            if (canmodify && gamemission == doom2 && gamemap == 4)
                EV_LightTurnOn(line, 0);
            else
                EV_LightTurnOn(line, 35);
            P_ChangeSwitchTexture(line, 1);
            break;
    }

    return true;
}
