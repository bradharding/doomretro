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

#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_system.h"
#include "i_swap.h"
#include "p_fix.h"
#include "p_local.h"
#include "p_setup.h"
#include "s_sound.h"
#include "w_wad.h"

// killough 02/08/98: Remove switch limit
static int  *switchlist;    // killough
static int  numswitches;    // killough

button_t    *buttonlist = NULL;
int         maxbuttons = MAXBUTTONS;

//
// P_InitSwitchList
//
// Only called at game initialization in order to list the set of switches
// and buttons known to the engine. This enables their texture to change
// when activated, and in the case of buttons, change back after a timeout.
//
// This routine modified to read its data from a predefined lump or
// PWAD lump called SWITCHES rather than a static table in this file to
// allow WAD designers to insert or modify switches.
//
// Lump format is an array of byte packed switchlist_t structures, terminated
// by a structure with episode == -1. The lump can be generated from a
// text source file using SWANTBLS.EXE, distributed with the BOOM utils.
// The standard list of switches and animations is contained in the example
// source text file DEFSWANI.DAT also in the BOOM util distribution.
//
// Rewritten by Lee Killough to remove limit 02/08/98
//
void P_InitSwitchList(void)
{
    int             index = 0;
    const int       ep = (gamemode == registered || gamemode == retail ? 2 : (gamemode == commercial ? 3 : 1));
    switchlist_t    *alphSwitchList;                    // jff 03/23/98 pointer to switch table
    const int       lump = W_GetNumForName("SWITCHES"); // cph - new WAD lump handling

    // jff 03/23/98 read the switch table from a predefined lump
    alphSwitchList = (switchlist_t *)W_CacheLumpNum(lump);

    for (int i = 0; ; i++)
    {
        static int  max_numswitches;

        if (index + 1 >= max_numswitches)
            switchlist = I_Realloc(switchlist,
                (size_t)(max_numswitches = (max_numswitches ? max_numswitches * 2 : 8)) * sizeof(*switchlist));

        if (SHORT(alphSwitchList[i].episode) <= ep)     // jff 05/11/98 endianness
        {
            int texture1;
            int texture2;

            if (!SHORT(alphSwitchList[i].episode))
                break;

            // Ignore switches referencing unknown texture names, instead of exiting.
            // Warn if either one is missing, but only add if both are valid.
            if ((texture1 = R_CheckTextureNumForName(alphSwitchList[i].name1)) == -1)
                C_Warning(1, "Switch %i in the " BOLD("SWITCHES") " lump has an unknown " BOLD("%s") " texture.",
                    i, alphSwitchList[i].name1);

            if ((texture2 = R_CheckTextureNumForName(alphSwitchList[i].name2)) == -1)
                C_Warning(1, "Switch %i in the " BOLD("SWITCHES") " lump has an unknown " BOLD("%s") " texture.",
                    i, alphSwitchList[i].name2);

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
void P_StartButton(line_t *line, bwhere_e bwhere, int texture, int time)
{
    // See if button is already pressed
    for (int i = 0; i < maxbuttons; i++)
        if (buttonlist[i].btimer && buttonlist[i].line == line)
            return;

    for (int i = 0; i < maxbuttons; i++)
        if (!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].bwhere = bwhere;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = &line->soundorg;
            return;
        }

    // [crispy] remove MAXBUTTONS limit
    maxbuttons *= 2;
    buttonlist = I_Realloc(buttonlist, maxbuttons * sizeof(*buttonlist));
    memset(buttonlist + maxbuttons / 2, 0, ((size_t)maxbuttons - maxbuttons / 2) * sizeof(*buttonlist));
    P_StartButton(line, bwhere, texture, time);
}

//
// Function that changes wall texture.
// Tell it if switch is ok to use again (true = yes, it's a button).
//
void P_ChangeSwitchTexture(line_t *line, bool useagain)
{
    const int   sidenum = line->sidenum[0];
    short       *toptexture = &sides[sidenum].toptexture;
    short       *midtexture = &sides[sidenum].midtexture;
    short       *bottomtexture = &sides[sidenum].bottomtexture;

    if (!useagain)
        line->special = 0;

    for (int i = 0; i < numswitches * 2; i++)
    {
        bwhere_e    bwhere = nowhere;

        if (switchlist[i] == *bottomtexture)
        {
            bwhere = bottom;
            *bottomtexture = switchlist[i ^ 1];
        }

        if (switchlist[i] == *midtexture)
        {
            bwhere = middle;
            *midtexture = switchlist[i ^ 1];
        }

        if (switchlist[i] == *toptexture)
        {
            bwhere = top;
            *toptexture = switchlist[i ^ 1];
        }

        if (bwhere != nowhere)
        {
            if (useagain)
                P_StartButton(line, bwhere, switchlist[i], BUTTONTIME);

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
bool P_UseSpecialLine(mobj_t *thing, line_t *line, const int side, const bool bossaction)
{
    unsigned short  special;

    if (side)
        return false;

    // jff 02/04/98 add check here for generalized floor/ceiling mover
    if ((special = line->special) >= GenCrusherBase && special < GenEnd)
    {
        // pointer to line function is NULL by default, set non-null if
        // line special is push or switch generalized linedef type
        bool (*linefunc)(line_t *line) = NULL;

        // check each range of generalized linedefs
        if (special >= GenFloorBase)
        {
            if (!thing->player && !bossaction)
                if ((special & FloorChange) || !(special & FloorModel))
                    return false;                       // FloorModel is "Allow Monsters" if FloorChange is 0

            if (!line->tag && (special & 6) != 6)       // jff 02/27/98 all non-manual
                return false;                           // generalized types require tag

            linefunc = &EV_DoGenFloor;
        }
        else if (special >= GenCeilingBase)
        {
            if (!thing->player && !bossaction)
                if ((special & CeilingChange) || !(special & CeilingModel))
                    return false;                       // CeilingModel is "Allow Monsters" if CeilingChange is 0

            linefunc = &EV_DoGenCeiling;
        }
        else if (special >= GenDoorBase)
        {
            if (!thing->player && !bossaction)
            {
                if (!(special & DoorMonster))
                    return false;                       // monsters disallowed from this door

                if (line->flags & ML_SECRET)            // they can't open secret doors either
                    return false;
            }

            linefunc = &EV_DoGenDoor;
        }
        else if (special >= GenLockedBase)
        {
            if (!thing->player && !bossaction)
                return false;                           // monsters disallowed from unlocking doors

            if (!P_CanUnlockGenDoor(line))
                return false;

            linefunc = &EV_DoGenLockedDoor;
        }
        else if (special >= GenLiftBase)
        {
            if (!thing->player && !bossaction)
                if (!(special & LiftMonster))
                    return false;                       // monsters disallowed

            linefunc = &EV_DoGenLift;
        }
        else if (special >= GenStairsBase)
        {
            if (!thing->player && !bossaction)
                if (!(special & StairMonster))
                    return false;                       // monsters disallowed

            linefunc = &EV_DoGenStairs;
        }
        else
        {
            if (!thing->player && !bossaction)
                if (!(special & CrusherMonster))
                    return false;                       // monsters disallowed

            linefunc = &EV_DoGenCrusher;
        }

        if (linefunc)
            switch ((special & TriggerType) >> TriggerTypeShift)
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
    if (!thing->player && !bossaction)
    {
        // never open secret doors
        if (line->flags & ML_SECRET)
            return false;

        switch (special)
        {
            case DR_Door_OpenWaitClose_AlsoMonsters:
            case D1_Door_Blue_OpenStay:
            case D1_Door_Red_OpenStay:
            case D1_Door_Yellow_OpenStay:

            // jff 03/05/98 add ability to use teleporters for monsters
            case SR_Teleport_AlsoMonsters:
            case S1_Teleport_AlsoMonsters:
            case S1_Teleport_AlsoMonsters_Silent_SameAngle:
            case SR_Teleport_AlsoMonsters_Silent_SameAngle:
                break;

            default:
                return false;
        }
    }

    if (bossaction)
    {
        switch (line->special)
        {
            // 0-tag specials, locked switches and teleporters need to be blocked for boss actions.
            case DR_Door_OpenWaitClose_AlsoMonsters:
            case D1_Door_Blue_OpenStay:
            case D1_Door_Red_OpenStay:
            case D1_Door_Yellow_OpenStay:
            case DR_Door_OpenWaitClose_Fast:
            case D1_Door_OpenStay_Fast:
            case S1_Door_Blue_OpenStay_Fast:
            case S1_Door_Red_OpenStay_Fast:
            case S1_Door_Yellow_OpenStay_Fast:

            case SR_Door_Blue_OpenStay_Fast:
            case SR_Door_Red_OpenStay_Fast:
            case SR_Door_Yellow_OpenStay_Fast:

            // jff 03/05/98 add ability to use teleporters for monsters
            case SR_Teleport_AlsoMonsters:
            case S1_Teleport_AlsoMonsters:
            case S1_Teleport_AlsoMonsters_Silent_SameAngle:
            case SR_Teleport_AlsoMonsters_Silent_SameAngle:
                return false;
                break;
        }
    }

    if (!P_CheckTag(line))      // jff 02/27/98 disallow zero tag on some types
        return false;

    // do something
    switch (special)
    {
        // MANUALS
        case DR_Door_OpenWaitClose_Fast:
            if (nomonsters && (line->flags & ML_TRIGGER666))
            {
                line_t  junk = { 0 };

                junk.tag = 666;
                EV_DoFloor(&junk, LowerFloorToLowest);
                line->flags &= ~ML_TRIGGER666;
            }

            EV_VerticalDoor(line, thing);
            return true;

        case D1_Door_Blue_OpenStay:
        case D1_Door_Red_OpenStay:
        case D1_Door_Yellow_OpenStay:
            if (!thing->player)
                return false;

            EV_VerticalDoor(line, thing);
            return true;

        case DR_Door_Blue_OpenWaitClose:
        case DR_Door_Yellow_OpenWaitClose:
        case DR_Door_Red_OpenWaitClose:

        case DR_Door_OpenWaitClose_AlsoMonsters:
        case D1_Door_OpenStay_Fast:
        case D1_Door_OpenStay:
            EV_VerticalDoor(line, thing);
            return true;

        // Switches
        case S1_Stairs_RaiseBy8:
            if (EV_BuildStairs(line, FLOORSPEED / 4, 8 * FRACUNIT, false))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseDonut_ChangesTexture:
            if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_ExitLevel:
            // killough 10/98: prevent zombies from exiting levels
            if (!bossaction && thing->player && thing->player->health <= 0 && !compat_zombie)
            {
                S_StartSound(thing, sfx_noway);
                return false;
            }

            P_ChangeSwitchTexture(line, false);
            G_ExitLevel();
            return true;

        case S1_Floor_RaiseBy32_ChangesTexture:
            if (EV_DoPlat(line, RaiseAndChange, 32))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseBy24_ChangesTexture:
            if (EV_DoPlat(line, RaiseAndChange, 24))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseToNextHighestFloor:
            if (EV_DoFloor(line, RaiseFloorToNearest))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, RaiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Lift_LowerWaitRaise:
            if (EV_DoPlat(line, DownWaitUpStay, 0))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_LowerToLowestFloor:
            if (EV_DoFloor(line, LowerFloorToLowest))
                P_ChangeSwitchTexture(line, false);

            if (nomonsters && (line->flags & ML_TRIGGER666))
            {
                line_t  junk = { 0 };

                junk.tag = 666;
                EV_DoFloor(&junk, LowerFloorToLowest);
                junk.tag = 667;
                EV_DoFloor(&junk, RaiseToTexture);
                line->flags &= ~ML_TRIGGER666;
            }

            return true;

        case S1_Door_OpenWaitClose:
            if (EV_DoDoor(line, DoorNormal, VDOORSPEED))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Ceiling_LowerToFloor:
            if (EV_DoCeiling(line, LowerToFloor))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_LowerTo8AboveHighestFloor:
            if (EV_DoFloor(line, TurboLower))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Ceiling_LowerTo8AboveFloor_PerpetualSlowCrusherDamage:
            if (EV_DoCeiling(line, CrushAndRaise))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Door_CloseStay:
            if (EV_DoDoor(line, DoorClose, VDOORSPEED))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_ExitLevel_GoesToSecretLevel:
            // killough 10/98: prevent zombies from exiting levels
            if (!bossaction && thing->player && thing->player->health <= 0 && !compat_zombie)
            {
                S_StartSound(thing, sfx_noway);
                return false;
            }

            P_ChangeSwitchTexture(line, false);
            G_SecretExitLevel();
            return true;

        case S1_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            if (EV_DoFloor(line, RaiseFloorCrush))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, RaiseFloor))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_LowerToHighestFloor:
            if (EV_DoFloor(line, LowerFloor))
                P_ChangeSwitchTexture(line, false);
            return true;

        case S1_Door_OpenStay:
            if (EV_DoDoor(line, DoorOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Door_OpenWaitClose_Fast:
            if (EV_DoDoor(line, DoorBlazeRaise, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Door_OpenStay_Fast:
            if (EV_DoDoor(line, DoorBlazeOpen, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Door_CloseStay_Fast:
            if (EV_DoDoor(line, DoorBlazeClose, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Lift_LowerWaitRaise_Fast:
            if (EV_DoPlat(line, BlazeDWUS, 0))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Stairs_RaiseBy16_Fast:
            if (EV_BuildStairs(line, FLOORSPEED * 4, 16 * FRACUNIT, true))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseToNextHighestFloor_Fast:
            if (EV_DoFloor(line, RaiseFloorTurbo))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Door_Blue_OpenStay_Fast:
        case S1_Door_Red_OpenStay_Fast:
        case S1_Door_Yellow_OpenStay_Fast:
            if (EV_DoLockedDoor(line, DoorBlazeOpen, thing, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseBy512:
            if (EV_DoFloor(line, RaiseFloor512))
                P_ChangeSwitchTexture(line, false);

            return true;

        // Extended switches
        case S1_Floor_RaiseByShortestLowerTexture:
            if (EV_DoFloor(line, RaiseToTexture))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_LowerToLowestFloor_ChangesTexture:
            if (EV_DoFloor(line, LowerAndChange))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseBy24_ChangesTextureAndEffect:
            if (EV_DoFloor(line, RaiseFloor24AndChange))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_RaiseBy24:
            if (EV_DoFloor(line, RaiseFloor24))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Lift_PerpetualLowestAndHighestFloors:
            if (EV_DoPlat(line, PerpetualRaise, 0))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Lift_Stop:
            EV_StopPlat(line);
            P_ChangeSwitchTexture(line, false);
            return true;

        case S1_Crusher_Start_Fast:
            if (EV_DoCeiling(line, FastCrushAndRaise))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Crusher_Start_Silent:
            if (EV_DoCeiling(line, SilentCrushAndRaise))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Ceiling_RaiseToHighestCeiling:
            if (EV_DoCeiling(line, RaiseToHighest) || EV_DoFloor(line, LowerFloorToLowest))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Ceiling_LowerTo8AboveFloor:
            if (EV_DoCeiling(line, LowerAndCrush))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Crusher_Stop:
            if (EV_CeilingCrushStop(line))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Light_ChangeToBrightestAdjacent:
            EV_LightTurnOn(line, 0);
            P_ChangeSwitchTexture(line, false);
            return true;

        case S1_Light_ChangeTo35:
            EV_LightTurnOn(line, TICRATE);
            P_ChangeSwitchTexture(line, false);
            return true;

        case S1_Light_ChangeTo255:
            EV_LightTurnOn(line, 255);
            P_ChangeSwitchTexture(line, false);
            return true;

        case S1_Light_StartBlinking:
            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line, false);
            return true;

        case S1_Light_ChangeToDarkestAdjacent:
            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line, false);
            return true;

        case S1_Teleport_AlsoMonsters:
            if (EV_Teleport(line, side, thing))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Door_CloseWaitOpen_30Seconds:
            if (EV_DoDoor(line, DoorClose30ThenOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_ChangeTextureAndEffect:
            if (EV_DoChange(line, TrigChangeOnly))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Ceiling_LowerToLowestCeiling:
            if (EV_DoCeiling(line, LowerToLowest))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Ceiling_LowerToHighestFloor:
            if (EV_DoCeiling(line, LowerToMaxFloor))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Teleport_AlsoMonsters_Silent_SameAngle:
            if (EV_SilentTeleport(line, side, thing))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_ChangeTextureAndEffectToNearest:
            if (EV_DoChange(line, NumChangeOnly))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, LowerFloorToNearest))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Lift_RaiseToNextHighestFloor_Fast:
            if (EV_DoElevator(line, ElevateUp))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Lift_LowerToNextLowestFloor_Fast:
            if (EV_DoElevator(line, ElevateDown))
                P_ChangeSwitchTexture(line, false);

            return true;

        case S1_Lift_MoveToSameFloorHeight_Fast:
            if (EV_DoElevator(line, ElevateCurrent))
                P_ChangeSwitchTexture(line, false);

            return true;

        case SR_ChangeTextureAndEffectToNearest:
            if (EV_DoChange(line, NumChangeOnly))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseByShortestLowerTexture:
            if (EV_DoFloor(line, RaiseToTexture))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_LowerToLowestFloor_ChangesTexture:
            if (EV_DoFloor(line, LowerAndChange))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseBy512:
            if (EV_DoFloor(line, RaiseFloor512))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseBy24_ChangesTextureAndEffect:
            if (EV_DoFloor(line, RaiseFloor24AndChange))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseBy24:
            if (EV_DoFloor(line, RaiseFloor24))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Lift_PerpetualLowestAndHighestFloors:
            EV_DoPlat(line, PerpetualRaise, 0);
            P_ChangeSwitchTexture(line, true);
            return true;

        case SR_Lift_Stop:
            EV_StopPlat(line);
            P_ChangeSwitchTexture(line, true);
            return true;

        case SR_Crusher_Start_Fast:
            if (EV_DoCeiling(line, FastCrushAndRaise))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Crusher_Start:
            if (EV_DoCeiling(line, CrushAndRaise))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Crusher_Start_Silent:
            if (EV_DoCeiling(line, SilentCrushAndRaise))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Ceiling_RaiseToHighestCeiling:
            if (EV_DoCeiling(line, RaiseToHighest) || EV_DoFloor(line, LowerFloorToLowest))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Ceiling_LowerTo8AboveFloor:
            if (EV_DoCeiling(line, LowerAndCrush))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Crusher_Stop:
            if (EV_CeilingCrushStop(line))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_ChangeTextureAndEffect:
            if (EV_DoChange(line, TrigChangeOnly))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseDonut_ChangesTexture:
            if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Light_ChangeToBrightestAdjacent:
            EV_LightTurnOn(line, 0);
            P_ChangeSwitchTexture(line, true);
            return true;

        case SR_Light_StartBlinking:
            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line, true);
            return true;

        case SR_Light_ChangeToDarkestAdjacent:
            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line, true);
            return true;

        case SR_Teleport_AlsoMonsters:
            if (EV_Teleport(line, side, thing))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Door_CloseWaitOpen_30Seconds:
            if (EV_DoDoor(line, DoorClose30ThenOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Ceiling_LowerToLowestCeiling:
            if (EV_DoCeiling(line, LowerToLowest))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Ceiling_LowerToHighestFloor:
            if (EV_DoCeiling(line, LowerToMaxFloor))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Teleport_AlsoMonsters_Silent_SameAngle:
            if (EV_SilentTeleport(line, side, thing))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Lift_RaiseToCeiling_Instantly:
            if (EV_DoPlat(line, ToggleUpDn, 0))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, LowerFloorToNearest))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Lift_RaiseToNextHighestFloor_Fast:
            if (EV_DoElevator(line, ElevateUp))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Lift_LowerToNextLowestFloor_Fast:
            if (EV_DoElevator(line, ElevateDown))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Lift_MoveToSameFloorHeight_Fast:
            if (EV_DoElevator(line, ElevateCurrent))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Stairs_RaiseBy8:
            if (EV_BuildStairs(line, FLOORSPEED / 4, 8 * FRACUNIT, false))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Stairs_RaiseBy16_Fast:
            if (EV_BuildStairs(line, FLOORSPEED * 4, 16 * FRACUNIT, true))
                P_ChangeSwitchTexture(line, true);

            return true;

        // Buttons (retriggerable switches)
        case SR_Door_CloseStay:
            if (EV_DoDoor(line, DoorClose, VDOORSPEED))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Ceiling_LowerToFloor:
            if (EV_DoCeiling(line, LowerToFloor))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_LowerToHighestFloor:
            if (EV_DoFloor(line, LowerFloor))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_LowerToLowestFloor:
            if (EV_DoFloor(line, LowerFloorToLowest))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Door_OpenStay:
            if (EV_DoDoor(line, DoorOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Lift_LowerWaitRaise:
            if (EV_DoPlat(line, DownWaitUpStay, 1))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Door_OpenWaitClose:
            if (EV_DoDoor(line, DoorNormal, VDOORSPEED))
                P_ChangeSwitchTexture(line, true);
            else if (!autousing && thing->player)
                S_StartSound(thing, sfx_noway);

            return true;

        case SR_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, RaiseFloor))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseBy24_ChangesTexture:
            if (EV_DoPlat(line, RaiseAndChange, 24))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseBy32_ChangesTexture:
            if (EV_DoPlat(line, RaiseAndChange, 32))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            if (EV_DoFloor(line, RaiseFloorCrush))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, RaiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseToNextHighestFloor:
            if (EV_DoFloor(line, RaiseFloorToNearest))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_LowerTo8AboveHighestFloor:
            if (EV_DoFloor(line, TurboLower))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Door_OpenWaitClose_Fast:
            if (EV_DoDoor(line, DoorBlazeRaise, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Door_OpenStay_Fast:
            if (EV_DoDoor(line, DoorBlazeOpen, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Door_CloseStay_Fast:
            if (EV_DoDoor(line, DoorBlazeClose, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Lift_LowerWaitRaise_Fast:
            if (EV_DoPlat(line, BlazeDWUS, 0))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Floor_RaiseToNextHighestFloor_Fast:
            if (EV_DoFloor(line, RaiseFloorTurbo))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Door_Blue_OpenStay_Fast:
        case SR_Door_Red_OpenStay_Fast:
        case SR_Door_Yellow_OpenStay_Fast:
            if (EV_DoLockedDoor(line, DoorBlazeOpen, thing, VDOORSPEED * 4))
                P_ChangeSwitchTexture(line, true);

            return true;

        case SR_Light_ChangeTo255:
            EV_LightTurnOn(line, 255);
            P_ChangeSwitchTexture(line, true);
            return true;

        case SR_Light_ChangeTo35:
            EV_LightTurnOn(line, (MAP04 ? 0 : TICRATE));
            P_ChangeSwitchTexture(line, true);
            return true;

        case G1_Floor_RaiseToLowestCeiling:
        case GR_Door_OpenStay:
        case G1_Floor_RaiseToNextHighestFloor_ChangesTexture:
        case Scroll_ScrollTextureLeft:
        case Scroll_ScrollTextureRight:
        case G1_ExitLevel:
        case G1_ExitLevel_GoesToSecretLevel:
        case Scroll_ScrollWallUsingSidedefOffsets:
        case Translucent_MiddleTexture:
            if (thing->player && !autousing && P_DoorClosed(line))
                S_StartSound(thing, sfx_noway);

            return true;

        // ID24 specials

        case S1_SetTheTargetSectorsColormap:
            // [KLN] 04/13/25 support for the ID24 spec "set target" colormap 2078 (S1)
            for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                sectors[s].id24colormap = sides[*line->sidenum].id24colormapindex;

            P_ChangeSwitchTexture(line, false);
            return true;

        case SR_SetTheTargetSectorsColormap:
            // [KLN] 04/13/25 support for the ID24 spec "set target" colormap 2079 (SR)
            for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                sectors[s].id24colormap = sides[*line->sidenum].id24colormapindex;

            P_ChangeSwitchTexture(line, true);
            return true;
    }

    return !bossaction;
}
