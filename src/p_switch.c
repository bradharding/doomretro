/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include "i_system.h"
#include "doomdef.h"
#include "p_local.h"

#include "g_game.h"

#include "s_sound.h"

// Data.
#include "sounds.h"

// State.
#include "doomstat.h"
#include "r_state.h"


//
// CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
switchlist_t alphSwitchList[] =
{
    // Doom shareware episode 1 switches
    {"SW1BRCOM",        "SW2BRCOM",     1},
    {"SW1BRN1",         "SW2BRN1",      1},
    {"SW1BRN2",         "SW2BRN2",      1},
    {"SW1BRNGN",        "SW2BRNGN",     1},
    {"SW1BROWN",        "SW2BROWN",     1},
    {"SW1COMM",         "SW2COMM",      1},
    {"SW1COMP",         "SW2COMP",      1},
    {"SW1DIRT",         "SW2DIRT",      1},
    {"SW1EXIT",         "SW2EXIT",      1},
    {"SW1GRAY",         "SW2GRAY",      1},
    {"SW1GRAY1",        "SW2GRAY1",     1},
    {"SW1METAL",        "SW2METAL",     1},
    {"SW1PIPE",         "SW2PIPE",      1},
    {"SW1SLAD",         "SW2SLAD",      1},
    {"SW1STARG",        "SW2STARG",     1},
    {"SW1STON1",        "SW2STON1",     1},
    {"SW1STON2",        "SW2STON2",     1},
    {"SW1STONE",        "SW2STONE",     1},
    {"SW1STRTN",        "SW2STRTN",     1},

    // Doom registered episodes 2&3 switches
    {"SW1BLUE",         "SW2BLUE",      2},
    {"SW1CMT",          "SW2CMT",       2},
    {"SW1GARG",         "SW2GARG",      2},
    {"SW1GSTON",        "SW2GSTON",     2},
    {"SW1HOT",          "SW2HOT",       2},
    {"SW1LION",         "SW2LION",      2},
    {"SW1SATYR",        "SW2SATYR",     2},
    {"SW1SKIN",         "SW2SKIN",      2},
    {"SW1VINE",         "SW2VINE",      2},
    {"SW1WOOD",         "SW2WOOD",      2},

    // Doom II switches
    {"SW1PANEL",        "SW2PANEL",     3},
    {"SW1ROCK",         "SW2ROCK",      3},
    {"SW1MET2",         "SW2MET2",      3},
    {"SW1WDMET",        "SW2WDMET",     3},
    {"SW1BRIK",         "SW2BRIK",      3},
    {"SW1MOD1",         "SW2MOD1",      3},
    {"SW1ZIM",          "SW2ZIM",       3},
    {"SW1STON6",        "SW2STON6",     3},
    {"SW1TEK",          "SW2TEK",       3},
    {"SW1MARB",         "SW2MARB",      3},
    {"SW1SKULL",        "SW2SKULL",     3},

    {"\0",              "\0",           0}
};

int             switchlist[MAXSWITCHES * 2];
int             numswitches;
button_t        buttonlist[MAXBUTTONS];

extern boolean  canmodify;

//
// P_InitSwitchList
// Only called at game initialization.
//
void P_InitSwitchList(void)
{
    int         i;
    int         index;
    int         episode;

    episode = 1;

    if (gamemode == registered || gamemode == retail)
        episode = 2;
    else
        if (gamemode == commercial)
            episode = 3;

    for (index = 0, i = 0; i < MAXSWITCHES; i++)
    {
        if (!alphSwitchList[i].episode)
        {
            numswitches = index / 2;
            switchlist[index] = -1;
            break;
        }

        if (alphSwitchList[i].episode <= episode)
        {
            switchlist[index++] = R_TextureNumForName(alphSwitchList[i].name1);
            switchlist[index++] = R_TextureNumForName(alphSwitchList[i].name2);
        }
    }
}


//
// Start a button counting down till it turns off.
//
void P_StartButton(line_t *line, bwhere_e w, int texture, int time)
{
    int         i;

    // See if button is already pressed
    for (i = 0; i < MAXBUTTONS; i++)
    {
        if (buttonlist[i].btimer
            && buttonlist[i].line == line)
        {
            return;
        }
    }



    for (i = 0; i < MAXBUTTONS; i++)
    {
        if (!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].where = w;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = &line->soundorg;
            return;
        }
    }

    I_Error("P_StartButton: no button slots left!");
}





//
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
//
void P_ChangeSwitchTexture(line_t *line, int useAgain)
{
    mobj_t      *soundorg;
    int         i;
    int         sound;
    short       *texture;
    short       *texTop;
    short       *texMid;
    short       *texBot;
    bwhere_e    position;

    texTop = &sides[line->sidenum[0]].toptexture;
    texMid = &sides[line->sidenum[0]].midtexture;
    texBot = &sides[line->sidenum[0]].bottomtexture;

    sound = sfx_swtchn;

    // use the sound origin of the linedef (its midpoint)
    soundorg = (mobj_t *)&line->soundorg;

    // don't zero line->special until after exit switch test
    if (!useAgain)
        line->special = 0;

    // search for a texture to change
    texture = NULL;
    position = (bwhere_e)0;
    for (i = 0; i < numswitches * 2; i++)
    {
        if (switchlist[i] == *texTop)
        {
            texture = texTop;
            position = top;
            break;
        }
        else if (switchlist[i] == *texMid)
        {
            texture = texMid;
            position = middle;
            break;
        }
        else if (switchlist[i] == *texBot)
        {
            texture = texBot;
            position = bottom;
            break;
        }
    }
    if (texture == NULL)
        return; // no switch texture was found to change
    *texture = switchlist[i ^ 1];

    S_StartSound(soundorg, sound);

    if (useAgain)
        P_StartButton(line, position, switchlist[i], BUTTONTIME);
}






//
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
//
boolean P_UseSpecialLine(mobj_t *thing, line_t *line, int side)
{

    if (side)
    {
        return false;
    }


    // Switches that other things can activate.
    if (!thing->player)
    {
        // never open secret doors
        if (line->flags & ML_SECRET)
            return false;

        switch(line->special)
        {
          case DR_OpenDoorWait4SecondsClose:
          case D1_OpenDoorStayOpenBlueKeyRequired:
          case D1_OpenDoorStayOpenRedKeyRequired:
          case D1_OpenDoorStayOpenYellowKeyRequired:
            break;

          default:
            return false;
            break;
        }
    }


    // do something
    switch (line->special)
    {
        // MANUALS
        case DR_OpenDoorWait4SecondsClose:
        case DR_OpenDoorWait4SecondsCloseBlueKeyRequired:
        case DR_OpenDoorWait4SecondsCloseYellowKeyRequired:
        case DR_OpenDoorWait4SecondsCloseRedKeyRequired:

        case D1_OpenDoorStayOpen:
        case D1_OpenDoorStayOpenBlueKeyRequired:
        case D1_OpenDoorStayOpenRedKeyRequired:
        case D1_OpenDoorStayOpenYellowKeyRequired:

        case DR_OpenFastDoorWait4SecondsClose:
        case D1_OpenFastDoorStayOpen:
            EV_VerticalDoor(line, thing);
            break;

        // SWITCHES
        case S1_RaiseStairsHeight8Units:
            if (EV_BuildStairs(line, build8))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_LowerPillarRaiseDonutChangeDonutFloorTextureAndType:
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

        case S1_RaiseFloorBy32UnitsChangeFloorTextureAndType:
            if (EV_DoPlat(line,raiseAndChange, 32))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_RaiseFloorBy24UnitsChangeFloorTextureAndType:
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_RaiseFloorToNextFloor:
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_RaiseFloorToNextFloorChangeFloorTextureAndType:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_LowerLiftWait3SecondsRise:
            if (EV_DoPlat(line, downWaitUpStay, 0))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_SetFloorToLowestNeighbouringFloor:
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_OpenDoorWait4SecondsClose:
            if (EV_DoDoor(line, normal))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_LowerCeilingToFloor:
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_SetFloorTo8UnitsAboveHighestNeighbouringFloor:
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_StartSlowCrusher:
            if (EV_DoCeiling(line, crushAndRaise))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_CloseDoor:
            if (EV_DoDoor(line, close))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_ExitLevelAndGoToSecretLevel:
            if (thing->player && thing->player->health <= 0)
            {
                S_StartSound(thing, sfx_noway);
                return false;
            }
            P_ChangeSwitchTexture(line, 0);
            G_SecretExitLevel();
            break;

        case S1_SetFloorTo8UnitsUnderLowestNeighbouringCeiling:
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_SetFloorToLowestNeighbouringCeiling:
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_SetFloorToHighestNeighbouringFloor:
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_OpenDoorStayOpen:
            if (EV_DoDoor(line, open))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_OpenFastDoorWait4SecondsClose:
            if (EV_DoDoor(line, blazeRaise))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_OpenFastDoorStayOpen:
            if (EV_DoDoor(line, blazeOpen))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_CloseFastDoor:
            if (EV_DoDoor(line, blazeClose))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_LowerFastLiftWait3SecondsRise:
            if (EV_DoPlat(line, blazeDWUS, 0))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_RaiseFastStairsHeight16Units:
            if (EV_BuildStairs(line, turbo16))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_RaiseFastFloorToNextFloor:
            if (EV_DoFloor(line, raiseFloorTurbo))
                P_ChangeSwitchTexture(line, 0);
        break;

        case S1_OpenFastDoorStayOpenBlueKeyRequired:
        case S1_OpenFastDoorStayOpenRedKeyRequired:
        case S1_OpenFastDoorStayOpenYellowKeyRequired:
            if (EV_DoLockedDoor(line, blazeOpen, thing))
                P_ChangeSwitchTexture(line, 0);
            break;

        case S1_RaiseFloorBy512Units:
            if (EV_DoFloor(line, raiseFloor512))
                P_ChangeSwitchTexture(line, 0);
        break;

        // BUTTONS
        case SR_LowerCeilingToFloorCloseDoor:
            if (EV_DoDoor(line, close))
                P_ChangeSwitchTexture(line, 1);
        break;

        case SR_LowerCeilingToFloor:
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_SetFloorToHighestNeighbouringFloor:
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_SetFloorToLowestNeighbouringFloor:
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_OpenDoorStayOpen:
            if (EV_DoDoor(line, open))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_LowerLiftWait3SecondsRise:
            if (EV_DoPlat(line, downWaitUpStay, 1))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_OpenDoorWait4SecondsClose:
            if (EV_DoDoor(line, normal))
                P_ChangeSwitchTexture(line, 1);
            else if (thing->player)
                S_StartSound(thing, sfx_oof);
            break;

        case SR_SetFloorToLowestNeighbouringCeiling:
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_RaiseFloorBy24UnitsChangeFloorTextureAndType:
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_RaiseFloorBy32UnitsChangeFloorTextureAndType:
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_SetFloorTo8UnitsUnderLowestNeighbouringCeiling:
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_RaiseFloorToNextFloorChangeFloorTextureAndType:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_RaiseFloorToNextFloor:
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_SetFloorTo8UnitsAboveHighestNeighbouringFloor:
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_OpenFastDoorWait4SecondsClose:
            if (EV_DoDoor(line, blazeRaise))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_OpenFastDoorStayOpen:
            if (EV_DoDoor(line, blazeOpen))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_CloseFastDoor:
            if (EV_DoDoor(line, blazeClose))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_LowerFastLiftWait3SecondsRise:
            if (EV_DoPlat(line, blazeDWUS, 0))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_RaiseFastFloorToNextFloor:
            if (EV_DoFloor(line, raiseFloorTurbo))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_OpenFastDoorStayOpenBlueKeyRequired:
        case SR_OpenFastDoorStayOpenRedKeyRequired:
        case SR_OpenFastDoorStayOpenYellowKeyRequired:
            if (EV_DoLockedDoor(line, blazeOpen, thing))
                P_ChangeSwitchTexture(line, 1);
            break;

        case SR_LightsTo255:
            EV_LightTurnOn(line, 255);
            P_ChangeSwitchTexture(line, 1);
            break;

        case SR_LightsTo0:
            if (canmodify && gamemission == doom2 && gamemap == 4)
                EV_LightTurnOn(line, 0);
            else
                EV_LightTurnOn(line, 35);
            P_ChangeSwitchTexture(line, 1);
            break;

    }

    return true;
}