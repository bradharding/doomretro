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

#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
    boolean     istexture;
    int         picnum;
    int         basepic;
    int         numpics;
    int         speed;
} anim_t;

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif

//
// source animation definition
//
typedef struct
{
    boolean     istexture;              // if false, it is a flat
    char        endname[9];
    char        startname[9];
    int         speed;
    boolean     isliquid;
} animdef_t;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

#define MAXANIMS        32

#define ANIMSPEED       8

static anim_t   *lastanim;
static anim_t   *anims;         // new structure w/o limits -- killough
static size_t   maxanims;

//
// P_InitPicAnims
//

// Floor/ceiling animation sequences,
//  defined by first and last frame,
//  i.e. the flat (64x64 tile) name to
//  be used.
// The full animation sequence is given
//  using all the flats between the start
//  and end entry, in the order found in
//  the WAD file.
//
animdef_t animdefs[] =
{
    { false, "NUKAGE3",  "NUKAGE1",  ANIMSPEED, true  },
    { false, "FWATER4",  "FWATER1",  ANIMSPEED, true  },
    { false, "SWATER4",  "SWATER1",  ANIMSPEED, true  },
    { false, "LAVA4",    "LAVA1",    ANIMSPEED, true  },
    { false, "BLOOD3",   "BLOOD1",   ANIMSPEED, true  },

    // DOOM II flat animations.
    { false, "RROCK08",  "RROCK05",  ANIMSPEED, false },
    { false, "SLIME04",  "SLIME01",  ANIMSPEED, true  },
    { false, "SLIME08",  "SLIME05",  ANIMSPEED, true  },
    { false, "SLIME12",  "SLIME09",  ANIMSPEED, false },

    { true,  "BLODGR4",  "BLODGR1",  ANIMSPEED, false },
    { true,  "SLADRIP3", "SLADRIP1", ANIMSPEED, false },

    { true,  "BLODRIP4", "BLODRIP1", ANIMSPEED, false },
    { true,  "FIREWALL", "FIREWALA", ANIMSPEED, false },
    { true,  "GSTFONT3", "GSTFONT1", ANIMSPEED, false },
    { true,  "FIRELAVA", "FIRELAV3", ANIMSPEED, false },
    { true,  "FIREMAG3", "FIREMAG1", ANIMSPEED, false },
    { true,  "FIREBLU2", "FIREBLU1", ANIMSPEED, false },
    { true,  "ROCKRED3", "ROCKRED1", ANIMSPEED, false },

    { true,  "BFALL4",   "BFALL1",   ANIMSPEED, false },
    { true,  "SFALL4",   "SFALL1",   ANIMSPEED, false },
    { true,  "WFALL4",   "WFALL1",   ANIMSPEED, false },
    { true,  "DBRAIN4",  "DBRAIN1",  ANIMSPEED, false },

    { false, "",         "",                 0, false }
};

//
// Animating line specials
//
extern int      numflats;
extern boolean  canmodify;

boolean         *isliquid;

void P_InitPicAnims(void)
{
    int i;
    int size = (numflats + 1) * sizeof(boolean);

    isliquid = Z_Malloc(size, PU_STATIC, 0);
    memset(isliquid, false, size);

    //  Init animation
    lastanim = anims;
    for (i = 0; animdefs[i].endname[0]; i++)
    {
        char    *startname = animdefs[i].startname;
        char    *endname = animdefs[i].endname;

        // 1/11/98 killough -- removed limit by array-doubling
        if (lastanim >= anims + maxanims)
        {
            size_t      newmax = (maxanims ? maxanims * 2 : MAXANIMS);

            anims = realloc(anims, newmax * sizeof(*anims));
            lastanim = anims + maxanims;
            maxanims = newmax;
        }

        if (animdefs[i].istexture)
        {
            // different episode?
            if (R_CheckTextureNumForName(startname) == -1)
                continue;

            lastanim->picnum = R_TextureNumForName(endname);
            lastanim->basepic = R_TextureNumForName(startname);

            lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;
        }
        else
        {
            if (W_CheckNumForName(startname) == -1)
                continue;

            lastanim->picnum = R_FlatNumForName(endname);
            lastanim->basepic = R_FlatNumForName(startname);

            lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;

            if (animdefs[i].isliquid)
            {
                int     j;

                for (j = 0; j < lastanim->numpics; j++)
                    isliquid[lastanim->basepic + j] = true;
            }
        }

        lastanim->istexture = animdefs[i].istexture;

        lastanim->speed = animdefs[i].speed;
        lastanim++;
    }

    if (BTSX)
    {
        int     SHNPRT02 = R_FlatNumForName("SHNPRT02");

        for (i = 0; i < 13; ++i)
            isliquid[SHNPRT02 + i] = false;
        isliquid[R_FlatNumForName("SLIME05")] = false;
        isliquid[R_FlatNumForName("SLIME08")] = false;
    }
}

//
// UTILITIES
//

//
// getSide()
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
side_t *getSide(int currentSector, int line, int side)
{
    return &sides[(sectors[currentSector].lines[line])->sidenum[side]];
}

//
// getSector()
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
sector_t *getSector(int currentSector, int line, int side)
{
    return sides[(sectors[currentSector].lines[line])->sidenum[side]].sector;
}

//
// twoSided()
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
int twoSided(int sector, int line)
{
    // jff 1/26/98 return what is actually needed, whether the line
    // has two sidedefs, rather than whether the 2S flag is set
    return (sectors[sector].lines[line]->sidenum[1] != NO_INDEX);
}

//
// getNextSector()
// Return sector_t * of sector next to current.
// NULL if not two-sided line
//
sector_t *getNextSector(line_t *line, sector_t *sec)
{
    // jff 1/26/98 check unneeded since line->backsector already
    // returns NULL if the line is not two sided, and does so from
    // the actual two-sidedness of the line, rather than its 2S flag
    //if (!(line->flags & ML_TWOSIDED))
    //    return NULL;
    return (line->frontsector == sec ? (line->backsector != sec ? line->backsector : NULL) :
        line->frontsector);
}

//
// P_FindLowestFloorSurrounding()
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t P_FindLowestFloorSurrounding(sector_t *sec)
{
    int         i;
    sector_t    *other;
    fixed_t     floor = sec->floorheight;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight < floor)
            floor = other->floorheight;
    return floor;
}

//
// P_FindHighestFloorSurrounding()
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t P_FindHighestFloorSurrounding(sector_t *sec)
{
    int         i;
    sector_t    *other;
    fixed_t     floor = -32000 << FRACBITS;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight > floor)
            floor = other->floorheight;
    return floor;
}

//
// P_FindNextHighestFloor
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
//
fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight)
{
    int         i;
    sector_t    *other;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight > currentheight)
        {
            int height = other->floorheight;

            while (++i < sec->linecount)
                if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight < height
                    && other->floorheight > currentheight)
                    height = other->floorheight;
            return height;
        }
    return currentheight;
}


//
// P_FindNextLowestFloor
// FIND NEXT LOWEST FLOOR IN SURROUNDING SECTORS
//
fixed_t P_FindNextLowestFloor(sector_t *sec, int currentheight)
{
    int         i;
    sector_t    *other;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight < currentheight)
        {
            int height = other->floorheight;

            while (++i < sec->linecount)
                if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight > height
                    && other->floorheight < currentheight)
                    height = other->floorheight;
            return height;
        }
    return currentheight;
}

//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t P_FindLowestCeilingSurrounding(sector_t *sec)
{
    int         i;
    sector_t    *other;
    fixed_t     height = 32000 << FRACBITS;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight < height)
            height = other->ceilingheight;
    return height;
}

//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec)
{
    int         i;
    sector_t    *other;
    fixed_t     height = -32000 << FRACBITS;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight > height)
            height = other->ceilingheight;
    return height;
}

//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//

// Find the next sector with the same tag as a linedef.
// Rewritten by Lee Killough to use chained hashing to improve speed
int P_FindSectorFromLineTag(const line_t *line, int start)
{
    start = (start >= 0 ? sectors[start].nexttag :
        sectors[(unsigned int)line->tag % (unsigned int)numsectors].firsttag);
    while (start >= 0 && sectors[start].tag != line->tag)
        start = sectors[start].nexttag;
    return start;
}

// killough 4/16/98: Same thing, only for linedefs
int P_FindLineFromLineTag(const line_t *line, int start)
{
    start = (start >= 0 ? lines[start].nexttag :
        lines[(unsigned int)line->tag % (unsigned int)numlines].firsttag);
    while (start >= 0 && lines[start].tag != line->tag)
        start = lines[start].nexttag;
    return start;
}

// Hash the sector tags across the sectors and linedefs.
static void P_InitTagLists(void)
{
    int i;

    for (i = numsectors; --i >= 0;)     // Initially make all slots empty.
        sectors[i].firsttag = -1;
    for (i = numsectors; --i >= 0;)     // Proceed from last to first sector
    {                                   // so that lower sectors appear first
        int     j = (unsigned int)sectors[i].tag % (unsigned int)numsectors;    // Hash func

        sectors[i].nexttag = sectors[j].firsttag;     // Prepend sector to chain
        sectors[j].firsttag = i;
    }

    // killough 4/17/98: same thing, only for linedefs
    for (i = numlines; --i >= 0;)       // Initially make all slots empty.
        lines[i].firsttag = -1;
    for (i = numlines; --i >= 0;)       // Proceed from last to first linedef
    {                                   // so that lower linedefs appear first
        int     j = (unsigned int)lines[i].tag % (unsigned int)numlines;        // Hash func

        lines[i].nexttag = lines[j].firsttag;   // Prepend linedef to chain
        lines[j].firsttag = i;
    }
}

//
// Find minimum light from an adjacent sector
//
int P_FindMinSurroundingLight(sector_t *sector, int min)
{
    int         i;
    sector_t    *check;

    for (i = 0; i < sector->linecount; i++)
        if ((check = getNextSector(sector->lines[i], sector)) && check->lightlevel < min)
            min = check->lightlevel;
    return min;
}

//
// EVENTS
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//

//
// P_CheckTag()
//
// Passed a line, returns true if the tag is non-zero or the line special
// allows no tag without harm.
//
// Note: Only line specials activated by walkover, pushing, or shooting are
//       checked by this routine.
//
// jff 2/27/98 Added to check for zero tag allowed for regular special types
//
boolean P_CheckTag(line_t *line)
{
    // tag not zero, allowed
    if (line->tag)
        return true;

    switch (line->special)
    {
        case DR_Door_OpenWaitClose_AlsoMonsters:
        case S1_ExitLevel:
        case W1_Light_ChangeToBrightestAdjacent:
        case W1_Light_ChangeTo255:
        case W1_Light_StartBlinking:
        case DR_Door_Blue_OpenWaitClose:
        case DR_Door_Yellow_OpenWaitClose:
        case DR_Door_Red_OpenWaitClose:
        case D1_Door_OpenStay:
        case D1_Door_Blue_OpenStay:
        case D1_Door_Red_OpenStay:
        case D1_Door_Yellow_OpenStay:
        case W1_Light_ChangeTo35:
        case W1_Teleport:
        case ScrollTextureLeft:
        case S1_ExitLevel_GoesToSecretLevel:
        case W1_ExitLevel:
        case WR_Light_ChangeTo35:
        case WR_Light_ChangeToBrightestAdjacent:
        case WR_Light_ChangeTo255:
        case ScrollTextureRight:
        case WR_Teleport:
        case W1_Light_ChangeToDarkestAdjacent:
        case DR_Door_OpenWaitClose_Fast:
        case D1_Door_OpenStay_Fast:
        case W1_ExitLevel_GoesToSecretLevel:
        case W1_Teleport_MonstersOnly:
        case WR_Teleport_MonstersOnly:
        case SR_Light_ChangeTo255:
        case SR_Light_ChangeTo35:
            return true;        // zero tag allowed

        default:
            break;
    }
    return false;               // zero tag not allowed
}

//
// P_CrossSpecialLine - TRIGGER
// Called every time a thing origin is about
//  to cross a line with a non 0 special.
//
void P_CrossSpecialLine(line_t *line, int side, mobj_t *thing)
{
    // Triggers that other things can activate
    if (!thing->player)
    {
        boolean ok = false;

        // Things that should NOT trigger specials...
        switch (thing->type)
        {
            case MT_ROCKET:
            case MT_PLASMA:
            case MT_BFG:
            case MT_TROOPSHOT:
            case MT_HEADSHOT:
            case MT_BRUISERSHOT:
            case MT_TRACER:
            case MT_FATSHOT:
            case MT_SPAWNSHOT:
            case MT_ARACHPLAZ:
                return;
                break;

            default:
                break;
        }

        switch (line->special)
        {
            case W1_Door_OpenWaitClose:
            case W1_Lift_LowerWaitRaise:
            case W1_Teleport:
            case WR_Lift_LowerWaitRaise:
            case WR_Teleport:
            case W1_Teleport_MonstersOnly:
            case WR_Teleport_MonstersOnly:
                ok = true;
                break;
        }
        if (!ok)
            return;
    }

    if (!P_CheckTag(line))      // jff 2/27/98 disallow zero tag on some types
        return;

    switch (line->special)
    {
        // Triggers
        case W1_Door_OpenStay:
            if (EV_DoDoor(line, doorOpen))
            {
                line->special = 0;

                if (nomonsters && (line->flags & ML_TRIGGER666))
                {
                    line_t      junk;

                    switch (gameepisode)
                    {
                        case 1:
                            junk.tag = 666;
                            EV_DoFloor(&junk, lowerFloorToLowest);
                            break;

                        case 4:
                            junk.tag = 666;
                            EV_DoDoor(&junk, doorBlazeOpen);
                            break;
                    }
                    line->flags &= ~ML_TRIGGER666;
                }
            }
            break;

        case W1_Door_CloseStay:
            if (EV_DoDoor(line, doorClose))
                line->special = 0;
            break;

        case W1_Door_OpenWaitClose:
            if (EV_DoDoor(line, doorNormal))
                line->special = 0;
            break;

        case W1_Floor_RaiseToLowestCeiling:
            if (gamemission == doom && gameepisode == 4 && gamemap == 3 && canmodify)
            {
                if (EV_DoFloor(line, raiseFloorCrush))
                    line->special = 0;
            }
            else if (EV_DoFloor(line, raiseFloor))
                line->special = 0;
            break;

        case W1_Crusher_StartWithFastDamage:
            if (EV_DoCeiling(line, fastCrushAndRaise))
                line->special = 0;
            break;

        case W1_Stairs_RaiseBy8:
            if (EV_BuildStairs(line, build8))
                line->special = 0;
            break;

        case W1_Lift_LowerWaitRaise:
            if (EV_DoPlat(line, downWaitUpStay, 0))
                line->special = 0;
            break;

        case W1_Light_ChangeToBrightestAdjacent:
            if (EV_LightTurnOn(line, 0))
                line->special = 0;
            break;

        case W1_Light_ChangeTo255:
            if (EV_LightTurnOn(line, 255))
                line->special = 0;
            break;

        case W1_Door_CloseWaitOpen:
            if (EV_DoDoor(line, doorClose30ThenOpen))
                line->special = 0;
            break;

        case W1_Light_StartBlinking:
            if (EV_StartLightStrobing(line))
                line->special = 0;
            break;

        case W1_Floor_LowerToHighestFloor:
            if (EV_DoFloor(line, lowerFloor))
                line->special = 0;
            break;

        case W1_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                line->special = 0;
            break;

        case W1_Crusher_StartWithSlowDamage:
            if (EV_DoCeiling(line, crushAndRaise))
                line->special = 0;
            break;

        case W1_Floor_RaiseByShortestLowerTexture:
            if (EV_DoFloor(line, raiseToTexture))
                line->special = 0;
            break;

        case W1_Light_ChangeTo35:
            if (EV_LightTurnOn(line, 35))
                line->special = 0;
            break;

        case W1_Floor_LowerTo8AboveHighestFloor:
            if (EV_DoFloor(line, turboLower))
                line->special = 0;
            break;

        case W1_Floor_LowerToLowestFloor_ChangesTexture:
            if (EV_DoFloor(line, lowerAndChange))
                line->special = 0;
            break;

        case W1_Floor_LowerToLowestFloor:
            if (EV_DoFloor(line, lowerFloorToLowest))
                line->special = 0;
            break;

        case W1_Teleport:
            if (EV_Teleport(line, side, thing))
                line->special = 0;
            break;

        case W1_Ceiling_RaiseToHighestCeiling:
            if (EV_DoCeiling(line, raiseToHighest))
                line->special = 0;
            break;

        case W1_Ceiling_LowerTo8AboveFloor:
            if (EV_DoCeiling(line, lowerAndCrush))
                line->special = 0;
            break;

        case W1_ExitLevel:
            if (!(thing->player && thing->player->health <= 0))
                G_ExitLevel();
            break;

        case W1_Floor_StartMovingUpAndDown:
            if (EV_DoPlat(line, perpetualRaise, 0))
                line->special = 0;
            break;

        case W1_Floor_StopMoving:
            if (EV_StopPlat(line))
                line->special = 0;
            break;

        case W1_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            if (EV_DoFloor(line, raiseFloorCrush))
                line->special = 0;
            break;

        case W1_Crusher_Stop:
            if (EV_CeilingCrushStop(line))
                line->special = 0;
            break;

        case W1_Floor_RaiseBy24:
            if (EV_DoFloor(line, raiseFloor24))
                line->special = 0;
            break;

        case W1_Floor_RaiseBy24_ChangesTexture:
            if (EV_DoFloor(line, raiseFloor24AndChange))
                line->special = 0;
            break;

        case W1_Stairs_RaiseBy16_Fast:
            if (EV_BuildStairs(line, turbo16))
                line->special = 0;
            break;

        case W1_Light_ChangeToDarkestAdjacent:
            if (EV_TurnTagLightsOff(line))
                line->special = 0;
            break;

        case W1_Door_OpenWaitClose_Fast:
            if (EV_DoDoor (line, doorBlazeRaise))
                line->special = 0;
            break;

        case W1_Door_OpenStay_Fast:
            if (EV_DoDoor (line, doorBlazeOpen))
                line->special = 0;
            break;

        case W1_Door_CloseStay_Fast:
            if (EV_DoDoor (line, doorBlazeClose))
                line->special = 0;
            break;

        case W1_Floor_RaiseToNextHighestFloor:
            if (EV_DoFloor(line, raiseFloorToNearest))
                line->special = 0;
            break;

        case W1_Lift_LowerWaitRaise_Fast:
            if (EV_DoPlat(line, blazeDWUS, 0))
                line->special = 0;
            break;

        case W1_ExitLevel_GoesToSecretLevel:
            if (!(thing->player && thing->player->health <= 0))
                G_SecretExitLevel();
            break;

        case W1_Teleport_MonstersOnly:
            if (!thing->player && EV_Teleport(line, side, thing))
                line->special = 0;
            break;

        case W1_Floor_RaiseToNextHighestFloor_Fast:
            if (EV_DoFloor(line, raiseFloorTurbo))
                line->special = 0;
            break;

        case W1_Crusher_StartWithSlowDamage_Silent:
            if (EV_DoCeiling(line, silentCrushAndRaise))
                line->special = 0;
            break;

        // Retriggers
        case WR_Ceiling_LowerTo8AboveFloor:
            EV_DoCeiling(line, lowerAndCrush);
            break;

        case WR_Crusher_StartWithSlowDamage:
            EV_DoCeiling(line, crushAndRaise);
            break;

        case WR_Crusher_Stop:
            EV_CeilingCrushStop(line);
            break;

        case WR_Door_CloseStay:
            EV_DoDoor(line, doorClose);
            break;

        case WR_Door_CloseStayOpen:
            EV_DoDoor(line, doorClose30ThenOpen);
            break;

        case WR_Crusher_StartWithFastDamage:
            EV_DoCeiling(line, fastCrushAndRaise);
            break;

        case WR_Light_ChangeTo35:
            EV_LightTurnOn(line, 35);
            break;

        case WR_Light_ChangeToBrightestAdjacent:
            EV_LightTurnOn(line, 0);
            break;

        case WR_Light_ChangeTo255:
            EV_LightTurnOn(line, 255);
            break;

        case WR_Floor_LowerToLowestFloor:
            EV_DoFloor(line, lowerFloorToLowest);
            break;

        case WR_Floor_LowerToHighestFloor:
            EV_DoFloor(line, lowerFloor);
            break;

        case WR_Floor_LowerToLowestFloor_ChangesTexture:
            EV_DoFloor(line, lowerAndChange);
            break;

        case WR_Door_OpenStay:
            EV_DoDoor(line, doorOpen);
            break;

        case WR_Floor_StartMovingUpAndDown:
            EV_DoPlat(line, perpetualRaise, 0);
            break;

        case WR_Lift_LowerWaitRaise:
            EV_DoPlat(line, downWaitUpStay, 0);
            break;

        case WR_Floor_StopMoving:
            EV_StopPlat(line);
            break;

        case WR_Door_OpenWaitClose:
            EV_DoDoor(line, doorNormal);
            break;

        case WR_Floor_RaiseToLowestCeiling:
            EV_DoFloor(line, raiseFloor);
            break;

        case WR_Floor_RaiseBy24:
            EV_DoFloor(line, raiseFloor24);
            break;

        case WR_Floor_RaiseBy24_ChangesTexture:
            EV_DoFloor(line, raiseFloor24AndChange);
            break;

        case WR_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            EV_DoFloor(line, raiseFloorCrush);
            break;

        case WR_Floor_RaiseToNextHighestFloor_ChangesTexture:
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            break;

        case WR_Floor_RaiseByShortestLowerTexture:
            EV_DoFloor(line, raiseToTexture);
            break;

        case WR_Teleport:
            EV_Teleport(line, side, thing);
            break;

        case WR_Floor_LowerTo8AboveHighestFloor:
            EV_DoFloor(line, turboLower);
            break;

        case WR_Door_OpenWaitClose_Fast:
            EV_DoDoor(line, doorBlazeRaise);
            break;

        case WR_Door_OpenStay_Fast:
            EV_DoDoor(line, doorBlazeOpen);
            break;

        case WR_Door_CloseStay_Fast:
            EV_DoDoor(line, doorBlazeClose);
            break;

        case WR_Lift_LowerWaitRaise_Fast:
            EV_DoPlat(line, blazeDWUS, 0);
            break;

        case WR_Teleport_MonstersOnly:
            if (!thing->player)
                EV_Teleport(line, side, thing);
            break;

        case WR_Floor_RaiseToNextHighestFloor:
            EV_DoFloor(line, raiseFloorToNearest);
            break;

        case WR_Floor_RaiseToNextHighestFloor_Fast:
            EV_DoFloor(line, raiseFloorTurbo);
            break;

        // Extended triggers
        case W1_Floor_RaiseBy512:
            if (EV_DoFloor(line, raiseFloor512))
                line->special = 0;
            break;

        case W1_Lift_RaiseBy24_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 24))
                line->special = 0;
            break;

        case W1_Lift_RaiseBy32_ChangesTexture:
            if (EV_DoPlat(line, raiseAndChange, 32))
                line->special = 0;
            break;

        case W1_CeilingLowerToFloor_Fast:
            if (EV_DoCeiling(line, lowerToFloor))
                line->special = 0;
            break;

        case W1_Floor_RaiseDonut_ChangesTexture:
            if (EV_DoDonut(line))
                line->special = 0;
            break;

        case W1_Ceiling_LowerToLowestCeiling:
            if (EV_DoCeiling(line, lowerToLowest))
                line->special = 0;
            break;

        case W1_Ceiling_LowerToHighestFloor:
            if (EV_DoCeiling(line, lowerToMaxFloor))
                line->special = 0;
            break;

        case W1_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, lowerFloorToNearest))
                line->special = 0;
            break;

        // Extended retriggers
        case WR_Floor_RaiseBy512:
            EV_DoFloor(line, raiseFloor512);
            break;

        case WR_Lift_RaiseBy24_ChangesTexture:
            EV_DoPlat(line, raiseAndChange, 24);
            break;

        case WR_Lift_RaiseBy32_ChangesTexture:
            EV_DoPlat(line, raiseAndChange, 32);
            break;

        case WR_Crusher_Start_Silent:
            EV_DoCeiling(line, silentCrushAndRaise);
            break;

        case WR_Ceiling_RaiseToHighestCeiling:
            EV_DoCeiling(line, raiseToHighest);
            EV_DoFloor(line, lowerFloorToLowest);
            break;

        case WR_Ceiling_LowerToFloor_Fast:
            EV_DoCeiling(line, lowerToFloor);
            break;

        case WR_Stairs_RaiseBy8:
            EV_BuildStairs(line, build8);
            break;

        case WR_Stairs_RaiseBy16_Fast:
            EV_BuildStairs(line, turbo16);
            break;

        case WR_Floor_RaiseDonut_ChangesTexture:
            EV_DoDonut(line);
            break;

        case WR_Light_StartBlinking:
            EV_StartLightStrobing(line);
            break;

        case WR_Light_ChangeToDarkestAdjacent:
            EV_TurnTagLightsOff(line);
            break;

        case WR_Ceiling_LowerToLowestCeiling:
            EV_DoCeiling(line, lowerToLowest);
            break;

        case WR_Ceiling_LowerToHighestFloor:
            EV_DoCeiling(line, lowerToMaxFloor);
            break;

        case WR_Lift_RaiseToCeiling_Instantly:
            EV_DoPlat(line, toggleUpDn, 0);
            break;

        case WR_Floor_LowerToNearestFloor:
            EV_DoFloor(line, lowerFloorToNearest);
            break;

        default:
            if ((unsigned short)line->special >= UNKNOWNLINESPECIAL)
                C_Warning("The player has crossed a line with an unknown special of %s.",
                    commify(line->special));
            break;
    }
}

//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void P_ShootSpecialLine(mobj_t *thing, line_t *line)
{
    // Impacts that other things can activate.
    if (!thing->player)
    {
        boolean ok = false;

        switch (line->special)
        {
            case GR_Door_OpenStay:
                ok = true;
                break;
        }
        if (!ok)
            return;
    }

    if (!P_CheckTag(line))      // jff 2/27/98 disallow zero tag on some types
        return;

    switch (line->special)
    {
        case G1_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 0);
            break;

        case GR_Door_OpenStay:
            EV_DoDoor(line, doorOpen);
            P_ChangeSwitchTexture(line, 1);
            if (canmodify && gamemission == doom2 && gamemap == 18)
                line->special = -GR_Door_OpenStay;
            break;

        case G1_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 0);
            break;

        case G1_ExitLevel:
            P_ChangeSwitchTexture(line, 0);
            G_ExitLevel();
            break;

        case G1_ExitLevel_GoesToSecretLevel:
            P_ChangeSwitchTexture(line, 0);
            G_SecretExitLevel();
            break;

        default:
            if ((unsigned short)line->special >= UNKNOWNLINESPECIAL)
                C_Warning("The player has shot a wall with an unknown special of %s.",
                    commify(line->special));
            break;
    }
}

//
// P_PlayerInSpecialSector
// Called every tic frame
//  that the player origin is in a special sector
//
void P_PlayerInSpecialSector(player_t *player)
{
    int      i;
    sector_t *sector = player->mo->subsector->sector;

    // Falling, not all the way down yet?
    if (player->mo->z != sector->floorheight)
        return;

    // Has hit ground.
    switch (sector->special)
    {
        case DamageNegative5Or10PercentHealth:
            if (!player->powers[pw_ironfeet])
                if (!(leveltime & 0x1f))
                    P_DamageMobj(player->mo, NULL, NULL, 10);
            break;

        case DamageNegative2Or5PercentHealth:
            if (!player->powers[pw_ironfeet])
                if (!(leveltime & 0x1f))
                      P_DamageMobj(player->mo, NULL, NULL, 5);
            break;

        case DamageNegative10Or20PercentHealth:
        case DamageNegative10Or20PercentHealthAndLightBlinks_2Hz:
            if (!player->powers[pw_ironfeet] || P_Random() < 5)
                if (!(leveltime & 0x1f))
                    P_DamageMobj(player->mo, NULL, NULL, 20);
            break;

        case Secret:
            player->secretcount++;
            sector->special = 0;

            for (i = 0; i < sector->linecount; i++)
                sector->lines[i]->flags &= ~ML_SECRET;
            break;

        case DamageNegative10Or20PercentHealthAndEndLevel:
            // for E1M8 finale
            player->cheats &= ~CF_GODMODE;
            player->powers[pw_invulnerability] = 0;

            if (!(leveltime & 0x1f))
                P_DamageMobj(player->mo, NULL, NULL, 20);

            if (player->health <= 10)
            {
                player->health = 0;
                G_ExitLevel();
            }
            break;

        default:
            if ((unsigned short)sector->special >= UNKNOWNSECTORSPECIAL)
                C_Warning("The player is in a sector with an unknown special of %s.",
                    commify(sector->special));
            break;
    }
}

//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//
#define MAXLINEANIMS    16384

short   numlinespecials;
line_t  *linespeciallist[MAXLINEANIMS];

void P_UpdateSpecials(void)
{
    anim_t      *anim;
    int         pic;
    int         i;

    // ANIMATE FLATS AND TEXTURES GLOBALLY
    for (anim = anims; anim < lastanim; anim++)
    {
        for (i = anim->basepic; i < anim->basepic + anim->numpics; i++)
        {
            pic = anim->basepic + ((leveltime / anim->speed + i) % anim->numpics);
            if (anim->istexture)
                texturetranslation[i] = pic;
            else
                flattranslation[i] = pic;
        }
    }

    // ANIMATE LINE SPECIALS
    for (i = 0; i < numlinespecials; i++)
    {
        line_t  *line = linespeciallist[i];

        switch (line->special)
        {
            case ScrollTextureLeft:
                sides[line->sidenum[0]].textureoffset += FRACUNIT;
                break;
        }
    }

    // DO BUTTONS
    for (i = 0; i < MAXBUTTONS; i++)
        if (buttonlist[i].btimer)
        {
            buttonlist[i].btimer--;
            if (!buttonlist[i].btimer)
            {
                switch (buttonlist[i].where)
                {
                    case top:
                        sides[buttonlist[i].line->sidenum[0]].toptexture =
                            buttonlist[i].btexture;
                        break;

                    case middle:
                        sides[buttonlist[i].line->sidenum[0]].midtexture =
                            buttonlist[i].btexture;
                        break;

                    case bottom:
                        sides[buttonlist[i].line->sidenum[0]].bottomtexture =
                            buttonlist[i].btexture;
                        break;
                }
                if (buttonlist[i].line->special != -GR_Door_OpenStay)
                    S_StartSound(buttonlist[i].soundorg, sfx_swtchn);
                memset(&buttonlist[i], 0, sizeof(button_t));
            }
        }
}

//
// Special Stuff that can not be categorized
//
int EV_DoDonut(line_t *line)
{
    sector_t    *s1;
    sector_t    *s2;
    sector_t    *s3;
    int         secnum = -1;
    int         rtn = 0;
    int         i;
    floormove_t *floor;
    fixed_t     s3_floorheight;
    short       s3_floorpic;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        s1 = &sectors[secnum];

        // ALREADY MOVING?  IF SO, KEEP GOING...
        if (s1->specialdata)
            continue;

        rtn = 1;
        s2 = getNextSector(s1->lines[0], s1);

        if (s2 == NULL)
            continue;

        for (i = 0; i < s2->linecount; i++)
        {
            s3 = s2->lines[i]->backsector;

            if (s3 == s1)
                continue;

            if (s3 == NULL)
                continue;
            else
            {
                s3_floorheight = s3->floorheight;
                s3_floorpic = s3->floorpic;
            }

            // Spawn rising slime
            floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
            P_AddThinker(&floor->thinker);
            s2->specialdata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = donutRaise;
            floor->crush = false;
            floor->direction = 1;
            floor->sector = s2;
            floor->speed = FLOORSPEED / 2;
            floor->texture = s3_floorpic;
            floor->newspecial = 0;
            floor->floordestheight = s3_floorheight;
            floor->stopsound = (floor->sector->floorheight != floor->floordestheight);

            // Spawn lowering donut-hole
            floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
            P_AddThinker(&floor->thinker);
            s1->specialdata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = lowerFloor;
            floor->crush = false;
            floor->direction = -1;
            floor->sector = s1;
            floor->speed = FLOORSPEED / 2;
            floor->floordestheight = s3_floorheight;
            floor->stopsound = (floor->sector->floorheight != floor->floordestheight);
            break;
        }
    }
    return rtn;
}

//
// SPECIAL SPAWNING
//

//
// P_SpawnSpecials
// After the map has been loaded, scan for specials
//  that spawn thinkers
//
void P_SpawnSpecials(void)
{
    sector_t    *sector;
    int         i;

    // Init special SECTORs.
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
        if (!sector->special)
            continue;

        switch (sector->special)
        {
            case LightBlinks_Randomly:
                P_SpawnLightFlash(sector);
                break;

            case LightBlinks_2Hz:
                P_SpawnStrobeFlash(sector, FASTDARK, 0);
                break;

            case LightBlinks_1Hz:
                P_SpawnStrobeFlash(sector, SLOWDARK, 0);
                break;

            case DamageNegative10Or20PercentHealthAndLightBlinks_2Hz:
                P_SpawnStrobeFlash(sector, FASTDARK, 0);
                sector->special = DamageNegative10Or20PercentHealthAndLightBlinks_2Hz;
                break;

            case LightGlows_1PlusSec:
                P_SpawnGlowingLight(sector);
                break;

            case Secret:
                totalsecret++;
                break;

            case Door_CloseStay_After30sec:
                P_SpawnDoorCloseIn30(sector);
                break;

            case LightBlinks_1HzSynchronized:
                P_SpawnStrobeFlash(sector, SLOWDARK, 1);
                break;

            case LightBlinks_2HzSynchronized:
                P_SpawnStrobeFlash(sector, FASTDARK, 1);
                break;

            case Door_OpenClose_OpensAfter5Min:
                P_SpawnDoorRaiseIn5Mins(sector);
                break;

            case LightFlickers_Randomly:
                P_SpawnFireFlicker(sector);
                break;
        }
    }

    // Init line EFFECTs
    numlinespecials = 0;
    for (i = 0; i < numlines; i++)
    {
        switch (lines[i].special)
        {
            case ScrollTextureLeft:
                linespeciallist[numlinespecials] = &lines[i];
                numlinespecials++;
                break;
        }
    }

    P_RemoveAllActiveCeilings();
    P_RemoveAllActivePlats();
    for (i = 0; i < MAXBUTTONS; i++)
        memset(&buttonlist[i], 0, sizeof(button_t));

    P_InitTagLists();
}
