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
#include "d_deh.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_fix.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "r_sky.h"
#include "s_sound.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"

const bool islightspecial[] =
{
    false, true,  true,  true, false, false, false, false, true,
    false, false, false, true, true,  false, false, false, true
};

//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
    bool    istexture;
    int     picnum;
    int     basepic;
    int     numpics;
    int     speed;
} anim_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

//
// source animation definition
//
typedef struct
{
    signed char istexture;          // if false, it is a flat
    char        endname[9];
    char        startname[9];
    int         speed;
} PACKEDATTR animdef_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

#define MAXANIMS    32

int             animatedtic;
fixed_t         animatedliquiddiff;
fixed_t         animatedliquidxdir;
fixed_t         animatedliquidydir;
fixed_t         animatedliquidxoffs;
fixed_t         animatedliquidyoffs;

const fixed_t animatedliquiddiffs[ANIMATEDLIQUIDDIFFS] =
{
     6422,  6422,  6360,  6238,  6054,  5814,  5516,  5164,
     4764,  4318,  3830,  3306,  2748,  2166,  1562,   942,
      314,  -314,  -942, -1562, -2166, -2748, -3306, -3830,
    -4318, -4764, -5164, -5516, -5814, -6054, -6238, -6360,
    -6422, -6422, -6360, -6238, -6054, -5814, -5516, -5164,
    -4764, -4318, -3830, -3306, -2748, -2166, -1562,  -942,
     -314,   314,   942,  1562,  2166,  2748,  3306,  3830,
     4318,  4764,  5164,  5516,  5814,  6054,  6238,  6360
};

int             timer = 0;
int             timeremaining = 0;

static anim_t   *lastanim;
static anim_t   *anims;             // new structure w/o limits -- killough

terraintype_t   *terraintypes;
bool            *isteleport;

bool            zerotag_manual;

// killough 03/07/98: Initialize generalized scrolling
static void P_SpawnScrollers(void);
static void P_SpawnFriction(void);  // phares 03/16/98
static void P_SpawnPushers(void);   // phares 03/20/98

static struct
{
    char            *startname;
    char            *endname;
    terraintype_t   terraintype;
} texturepacks[] = {
    { "GRAYSLM1", "GRAYSLM4", GRAYSLIME },
    { "MLAVA1",   "MLAVA4",   LAVA      },
    { "OBLODA01", "OBLODA08", BLOOD     },
    { "OGOOPY01", "OGOOPY08", GOOP      },
    { "OICYWA01", "OICYWA08", ICYWATER  },
    { "OLAVAC01", "OLAVAC08", LAVA      },
    { "OLAVAD01", "OLAVAD08", LAVA      },
    { "ONUKEA01", "ONUKEA08", NUKAGE    },
    { "OSLUDG01", "OSLUDG08", SLUDGE    },
    { "OTAR__01", "OTAR__08", TAR       },
    { "OWATER01", "OWATER08", SLUDGE    },
    { "PLOOD1",   "PLOOD3",   LIQUID    },
    { "PURPW1",   "PURPW4",   WATER     },
    { "TEALW1",   "TEALW4",   WATER     },
    { "ZO1_01",   "ZO1_99",   LAVA      },
    { "",         "",         0         }
};

static void SetTerrainType(const anim_t *anim, const terraintype_t terraintype)
{
    for (int i = anim->basepic; i < anim->basepic + anim->numpics; i++)
        terraintypes[i] = terraintype;
}

//
// P_InitPicAnims
//
void P_InitPicAnims(void)
{
    int         lump = W_GetNumForName("ANIMATED");
    animdef_t   *animdefs = W_CacheLumpNum(lump);
    size_t      maxanims = 0;

    short       NUKAGE1 = R_CheckFlatNumForName("NUKAGE1");
    short       NUKAGE3 = R_CheckFlatNumForName("NUKAGE3");
    short       FWATER1 = R_CheckFlatNumForName("FWATER1");
    short       FWATER4 = R_CheckFlatNumForName("FWATER4");
    short       SWATER1 = R_CheckFlatNumForName("SWATER1");
    short       SWATER4 = R_CheckFlatNumForName("SWATER4");
    short       LAVA1 = R_CheckFlatNumForName("LAVA1");
    short       LAVA4 = R_CheckFlatNumForName("LAVA4");
    short       BLOOD1 = R_CheckFlatNumForName("BLOOD1");
    short       BLOOD3 = R_CheckFlatNumForName("BLOOD3");
    short       SLIME01 = R_CheckFlatNumForName("SLIME01");
    short       SLIME08 = R_CheckFlatNumForName("SLIME08");

    terraintypes = Z_Calloc(1, ((size_t)numflats + 1) * sizeof(*terraintypes), PU_STATIC, NULL);
    isteleport = Z_Calloc(1, ((size_t)numflats + 1) * sizeof(*isteleport), PU_STATIC, NULL);

    RROCK05 = R_CheckFlatNumForName("RROCK05");
    RROCK08 = R_CheckFlatNumForName("RROCK08");
    SLIME09 = R_CheckFlatNumForName("SLIME09");
    SLIME12 = R_CheckFlatNumForName("SLIME12");

    // Init animation
    lastanim = anims;

    for (int i = 0; animdefs[i].istexture != -1; i++)
    {
        // killough 01/11/98 -- removed limit by array-doubling
        if (lastanim >= anims + maxanims)
        {
            size_t  newmax = (maxanims ? maxanims * 2 : MAXANIMS);

            anims = I_Realloc(anims, newmax * sizeof(*anims));
            lastanim = anims + maxanims;
            maxanims = newmax;
        }

        if (animdefs[i].istexture)
        {
            // different episode?
            if (R_CheckTextureNumForName(animdefs[i].startname) == -1)
                continue;

            lastanim->picnum = R_TextureNumForName(animdefs[i].endname);
            lastanim->basepic = R_TextureNumForName(animdefs[i].startname);

            lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;
            lastanim->istexture = true;
        }
        else
        {
            int     basepic;
            bool    isliquid = false;

            if (R_CheckFlatNumForName(animdefs[i].startname) == -1)
                continue;

            lastanim->picnum = R_FlatNumForName(animdefs[i].endname);
            lastanim->basepic = basepic = R_FlatNumForName(animdefs[i].startname);

            lastanim->numpics = lastanim->picnum - basepic + 1;
            lastanim->istexture = false;

            // Check if flat is liquid in IWAD
            if (basepic >= NUKAGE1 && basepic <= NUKAGE3)
            {
                SetTerrainType(lastanim, NUKAGE);
                isliquid = true;
            }
            else if ((basepic >= FWATER1 && basepic <= FWATER4)
                || (basepic >= SWATER1 && basepic <= SWATER4))
            {
                SetTerrainType(lastanim, WATER);
                isliquid = true;
            }
            else if (basepic >= LAVA1 && basepic <= LAVA4)
            {
                SetTerrainType(lastanim, LAVA);
                isliquid = true;
            }
            else if (basepic >= BLOOD1 && basepic <= BLOOD3)
            {
                SetTerrainType(lastanim, BLOOD);
                isliquid = true;
            }
            else if (basepic >= SLIME01 && basepic <= SLIME08)
            {
                SetTerrainType(lastanim, SLIME);
                isliquid = true;
            }

            // Check if name of flat indicates it is liquid
            if (!isliquid)
            {
                if (M_StrCaseStr(animdefs[i].startname, "NUK")
                    && !M_StrCaseStr(animdefs[i].startname, "WALK"))
                {
                    SetTerrainType(lastanim, NUKAGE);
                    isliquid = true;
                }
                else if (M_StrCaseStr(animdefs[i].startname, "WAT")
                    || M_StrCaseStr(animdefs[i].startname, "WTR")
                    || M_StrCaseStr(animdefs[i].startname, "WAV"))
                {
                    SetTerrainType(lastanim, WATER);
                    isliquid = true;
                }
                else if (M_StrCaseStr(animdefs[i].startname, "LAV"))
                {
                    SetTerrainType(lastanim, LAVA);
                    isliquid = true;
                }
                else if (M_StrCaseStr(animdefs[i].startname, "BLO"))
                {
                    SetTerrainType(lastanim, BLOOD);
                    isliquid = true;
                }
                else if ((M_StrCaseStr(animdefs[i].startname, "SLI")
                    && (basepic < SLIME09 || basepic > SLIME12))
                    || M_StrCaseStr(animdefs[i].startname, "SLM")
                    || M_StrCaseStr(animdefs[i].startname, "POOP"))
                {
                    SetTerrainType(lastanim, SLIME);
                    isliquid = true;
                }
                else if (M_StrCaseStr(animdefs[i].startname, "LIQ"))
                {
                    SetTerrainType(lastanim, LIQUID);
                    isliquid = true;
                }
            }

            // Check if flat is liquid in popular texture packs
            if (!isliquid)
                for (int j = 0; *texturepacks[j].startname; j++)
                    if (basepic >= R_CheckFlatNumForName(texturepacks[j].startname)
                        && basepic <= R_CheckFlatNumForName(texturepacks[j].endname))
                    {
                        SetTerrainType(lastanim, texturepacks[j].terraintype);
                        break;
                    }
        }

        if (lastanim->numpics < 2)
            I_Error("P_InitPicAnims: bad cycle from %s to %s", animdefs[i].startname, animdefs[i].endname);

        lastanim->speed = LONG(animdefs[i].speed);

        if (!lastanim->speed)
            lastanim->speed = 1;

        lastanim++;
    }

    W_ReleaseLumpNum(lump);

    SC_Open(W_CheckNumForName("DRCOMPAT"));

    while (SC_GetString())
    {
        const bool  noliquid = SC_Compare("NOLIQUID");

        if (noliquid || SC_Compare("LIQUID"))
        {
            int first;
            int last;

            SC_MustGetString();
            first = R_CheckFlatNumForName(sc_String);
            SC_MustGetString();
            last = R_CheckFlatNumForName(sc_String);
            SC_MustGetString();

            if (first >= 0 && last >= 0 && M_StringEndsWith(lumpinfo[firstflat + first]->wadfile->path, sc_String))
                for (int i = first; i <= last; i++)
                    terraintypes[i] = (noliquid ? SOLID : LIQUID);
        }
    }

    SC_Close();

    // [BH] indicate obvious teleport textures for automap
    if (BTSX)
    {
        isteleport[R_CheckFlatNumForName("SLIME05")] = true;
        isteleport[R_CheckFlatNumForName("SLIME08")] = true;
        isteleport[R_CheckFlatNumForName("SLIME09")] = true;
        isteleport[R_CheckFlatNumForName("SLIME12")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT02")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT03")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT04")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT05")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT06")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT07")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT08")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT09")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT10")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT11")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT12")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT13")] = true;
        isteleport[R_CheckFlatNumForName("SHNPRT14")] = true;
        isteleport[R_CheckFlatNumForName("TELEPRT1")] = true;
        isteleport[R_CheckFlatNumForName("TELEPRT2")] = true;
        isteleport[R_CheckFlatNumForName("TELEPRT3")] = true;
        isteleport[R_CheckFlatNumForName("TELEPRT4")] = true;
        isteleport[R_CheckFlatNumForName("TELEPRT5")] = true;
        isteleport[R_CheckFlatNumForName("TELEPRT6")] = true;
    }
    else
    {
        isteleport[R_CheckFlatNumForName("GATE1")] = true;
        isteleport[R_CheckFlatNumForName("GATE2")] = true;
        isteleport[R_CheckFlatNumForName("GATE3")] = true;
        isteleport[R_CheckFlatNumForName("GATE4")] = true;
    }

    for (anim_t *anim = anims; anim < lastanim; anim++)
        for (int i = anim->basepic; i < anim->basepic + anim->numpics; i++)
        {
            if (anim->istexture)
            {
                texture_t   *texture = textures[i];

                for (int j = 0; j < texture->patchcount; j++)
                    W_CacheLumpNum(texture->patches[j].patch);
            }
            else
                W_CacheLumpNum(firstflat + i);
        }
}

//
// P_SetLiquids
//
void P_SetLiquids(void)
{
    numliquid = 0;

    for (int i = 0; i < numsectors; i++)
        if ((sectors[i].terraintype = terraintypes[sectors[i].floorpic]) >= LIQUID)
            numliquid++;
}

//
// P_FindLifts
//
void P_FindLifts(void)
{
    for (int i = 0; i < numsectors; i++)
    {
        line_t  line;

        memset(&line, 0, sizeof(line));

        // Check to see if it's in a sector which can be activated as a lift.
        if ((line.tag = sectors[i].tag))
        {
            for (int j = -1; (j = P_FindLineFromLineTag(&line, j)) >= 0; )
                switch (lines[j].special)
                {
                    case W1_Lift_LowerWaitRaise:
                    case S1_Floor_RaiseBy32_ChangesTexture:
                    case S1_Floor_RaiseBy24_ChangesTexture:
                    case S1_Floor_RaiseToNextHighestFloor_ChangesTexture:
                    case S1_Lift_LowerWaitRaise:
                    case W1_Floor_RaiseToNextHighestFloor_ChangesTexture:
                    case G1_Floor_RaiseToNextHighestFloor_ChangesTexture:
                    case W1_Floor_StartMovingUpAndDown:
                    case SR_Lift_LowerWaitRaise:
                    case SR_Floor_RaiseBy24_ChangesTexture:
                    case SR_Floor_RaiseBy32_ChangesTexture:
                    case SR_Floor_RaiseToNextHighestFloor_ChangesTexture:
                    case WR_Floor_StartMovingUpAndDown:
                    case WR_Lift_LowerWaitRaise:
                    case WR_Floor_RaiseToNextHighestFloor_ChangesTexture:
                    case WR_Lift_LowerWaitRaise_Fast:
                    case W1_Lift_LowerWaitRaise_Fast:
                    case S1_Lift_LowerWaitRaise_Fast:
                    case SR_Lift_LowerWaitRaise_Fast:
                    case W1_Lift_RaiseBy24_ChangesTexture:
                    case W1_Lift_RaiseBy32_ChangesTexture:
                    case WR_Lift_RaiseBy24_ChangesTexture:
                    case WR_Lift_RaiseBy32_ChangesTexture:
                    case S1_Lift_PerpetualLowestAndHighestFloors:
                    case S1_Lift_Stop:
                    case SR_Lift_PerpetualLowestAndHighestFloors:
                    case SR_Lift_Stop:
                    case SR_Lift_RaiseToCeiling_Instantly:
                    case WR_Lift_RaiseToCeiling_Instantly:
                    case W1_Lift_RaiseToNextHighestFloor_Fast:
                    case WR_Lift_RaiseToNextHighestFloor_Fast:
                    case S1_Lift_RaiseToNextHighestFloor_Fast:
                    case SR_Lift_RaiseToNextHighestFloor_Fast:
                    case W1_Lift_LowerToNextLowestFloor_Fast:
                    case WR_Lift_LowerToNextLowestFloor_Fast:
                    case S1_Lift_LowerToNextLowestFloor_Fast:
                    case SR_Lift_LowerToNextLowestFloor_Fast:
                    case W1_Lift_MoveToSameFloorHeight_Fast:
                    case WR_Lift_MoveToSameFloorHeight_Fast:
                    case S1_Lift_MoveToSameFloorHeight_Fast:
                    case SR_Lift_MoveToSameFloorHeight_Fast:
                        sectors[i].islift = true;
                }
        }
        else
            sectors[i].islift = false;
    }
}

//
// UTILITIES
//

//
// P_GetSide
// Will return a side_t
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
side_t *P_GetSide(const int currentsector, const int line, const int side)
{
    return &sides[(sectors[currentsector].lines[line])->sidenum[side]];
}

//
// P_GetSector
// Will return a sector_t
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
sector_t *P_GetSector(const int currentsector, const int line, const int side)
{
    return sides[(sectors[currentsector].lines[line])->sidenum[side]].sector;
}

//
// P_TwoSided
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
bool P_TwoSided(const int sector, const int line)
{
    // jff 01/26/98 return what is actually needed, whether the line
    // has two sidedefs, rather than whether the 2S flag is set
    return (sectors[sector].lines[line]->sidenum[1] != NO_INDEX);
}

//
// P_GetNextSector
// Return sector_t of sector next to current.
// NULL if not two-sided line
//
sector_t *P_GetNextSector(line_t *line, const sector_t *sec)
{
    // jff 01/26/98 check unneeded since line->backsector already
    // returns NULL if the line is not two sided, and does so from
    // the actual two-sidedness of the line, rather than its 2S flag
    // if (!(line->flags & ML_TWOSIDED))
    //     return NULL;
    return (line->frontsector == sec ? (line->backsector != sec ? line->backsector : NULL) : line->frontsector);
}

//
// P_FindLowestFloorSurrounding
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t P_FindLowestFloorSurrounding(sector_t *sec)
{
    const int   linecount = sec->linecount;
    fixed_t     floor = sec->floorheight;

    for (int i = 0; i < linecount; i++)
    {
        const sector_t  *other = P_GetNextSector(sec->lines[i], sec);

        if (other && other->floorheight < floor)
            floor = other->floorheight;
    }

    return floor;
}

//
// P_FindHighestFloorSurrounding
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t P_FindHighestFloorSurrounding(sector_t *sec)
{
    const int   linecount = sec->linecount;
    fixed_t     floor = -32000 * FRACUNIT;

    for (int i = 0; i < linecount; i++)
    {
        const sector_t  *other = P_GetNextSector(sec->lines[i], sec);

        if (other && other->floorheight > floor)
            floor = other->floorheight;
    }

    return floor;
}

//
// P_FindNextHighestFloor
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
//
fixed_t P_FindNextHighestFloor(sector_t *sec, const int currentheight)
{
    const int   linecount = sec->linecount;

    for (int i = 0; i < linecount; i++)
    {
        sector_t    *other = P_GetNextSector(sec->lines[i], sec);

        if (other && other->floorheight > currentheight)
        {
            int height = other->floorheight;

            while (++i < linecount)
            {
                other = P_GetNextSector(sec->lines[i], sec);

                if (other && other->floorheight < height && other->floorheight > currentheight)
                    height = other->floorheight;
            }

            return height;
        }
    }

    return currentheight;
}

//
// P_FindNextLowestFloor
// FIND NEXT LOWEST FLOOR IN SURROUNDING SECTORS
//
fixed_t P_FindNextLowestFloor(sector_t *sec, const int currentheight)
{
    const int   linecount = sec->linecount;

    for (int i = 0; i < linecount; i++)
    {
        sector_t    *other = P_GetNextSector(sec->lines[i], sec);

        if (other && other->floorheight < currentheight)
        {
            int height = other->floorheight;

            while (++i < linecount)
            {
                other = P_GetNextSector(sec->lines[i], sec);

                if (other && other->floorheight > height && other->floorheight < currentheight)
                    height = other->floorheight;
            }

            return height;
        }
    }

    return currentheight;
}

//
// P_FindNextLowestCeiling
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the largest ceiling height in a surrounding sector smaller than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
fixed_t P_FindNextLowestCeiling(sector_t *sec, const int currentheight)
{
    const int   linecount = sec->linecount;

    for (int i = 0; i < linecount; i++)
    {
        sector_t    *other = P_GetNextSector(sec->lines[i], sec);

        if (other && other->ceilingheight < currentheight)
        {
            int height = other->ceilingheight;

            while (++i < linecount)
            {
                other = P_GetNextSector(sec->lines[i], sec);

                if (other && other->ceilingheight > height && other->ceilingheight < currentheight)
                    height = other->ceilingheight;
            }

            return height;
        }
    }

    return currentheight;
}

//
// P_FindNextHighestCeiling
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the smallest ceiling height in a surrounding sector larger than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
fixed_t P_FindNextHighestCeiling(sector_t *sec, const int currentheight)
{
    const int   linecount = sec->linecount;

    for (int i = 0; i < linecount; i++)
    {
        sector_t    *other = P_GetNextSector(sec->lines[i], sec);

        if (other && other->ceilingheight > currentheight)
        {
            int height = other->ceilingheight;

            while (++i < linecount)
            {
                other = P_GetNextSector(sec->lines[i], sec);

                if (other && other->ceilingheight < height && other->ceilingheight > currentheight)
                    height = other->ceilingheight;
            }

            return height;
        }
    }

    return currentheight;
}

//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t P_FindLowestCeilingSurrounding(sector_t *sec)
{
    const int   linecount = sec->linecount;
    fixed_t     height = 32000 * FRACUNIT;

    for (int i = 0; i < linecount; i++)
    {
        const sector_t  *other = P_GetNextSector(sec->lines[i], sec);

        if (other && other->ceilingheight < height)
            height = other->ceilingheight;
    }

    return height;
}

//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec)
{
    const int   linecount = sec->linecount;
    fixed_t     height = -32000 * FRACUNIT;

    for (int i = 0; i < linecount; i++)
    {
        const sector_t  *other = P_GetNextSector(sec->lines[i], sec);

        if (other && other->ceilingheight > height)
            height = other->ceilingheight;
    }

    return height;
}

//
// P_FindShortestTextureAround
//
// Passed a sector number, returns the shortest lower texture on a
// linedef bounding the sector.
//
// Note: If no lower texture exists 32000 * FRACUNIT is returned.
//       but if compatibility then MAXINT is returned
//
// jff 02/03/98 Add routine to find shortest lower texture
//
// killough 11/98: reformatted
fixed_t P_FindShortestTextureAround(const int secnum)
{
    const int   linecount = sectors[secnum].linecount;
    int         minsize = 32000 * FRACUNIT;

    for (int i = 0; i < linecount; i++)
        if (P_TwoSided(secnum, i))
        {
            short   texture;

            if ((texture = P_GetSide(secnum, i, 0)->bottomtexture) > 0 && textureheight[texture] < minsize)
                minsize = textureheight[texture];

            if ((texture = P_GetSide(secnum, i, 1)->bottomtexture) > 0 && textureheight[texture] < minsize)
                minsize = textureheight[texture];
        }

    return minsize;
}

//
// P_FindShortestUpperAround
//
// Passed a sector number, returns the shortest upper texture on a
// linedef bounding the sector.
//
// Note: If no upper texture exists 32000 * FRACUNIT is returned.
//       but if compatibility then MAXINT is returned
//
// jff 03/20/98 Add routine to find shortest upper texture
//
// killough 11/98: reformatted
fixed_t P_FindShortestUpperAround(const int secnum)
{
    const int   linecount = sectors[secnum].linecount;
    int         minsize = 32000 * FRACUNIT;

    for (int i = 0; i < linecount; i++)
        if (P_TwoSided(secnum, i))
        {
            short   texture;

            if ((texture = P_GetSide(secnum, i, 0)->toptexture) > 0 && textureheight[texture] < minsize)
                minsize = textureheight[texture];

            if ((texture = P_GetSide(secnum, i, 1)->toptexture) > 0 && textureheight[texture] < minsize)
                minsize = textureheight[texture];
        }

    return minsize;
}

//
// P_FindModelFloorSector
//
// Passed a floor height and a sector number, return a pointer to a
// a sector with that floor height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
// jff 02/03/98 Add routine to find numeric model floor
//  around a sector specified by sector number
// jff 03/14/98 change first parameter to plain height to allow call
//  from routine not using floormove_t
//
// killough 11/98: reformatted
sector_t *P_FindModelFloorSector(const fixed_t floordestheight, const int secnum)
{
    const int   linecount = sectors[secnum].linecount;

    for (int i = 0; i < linecount; i++)
        if (P_TwoSided(secnum, i))
        {
            sector_t    *sec = P_GetSector(secnum, i, (P_GetSide(secnum, i, 0)->sector->id == secnum));

            if (sec->floorheight == floordestheight)
                return sec;
        }

    return NULL;
}

//
// P_FindModelCeilingSector
//
// Passed a ceiling height and a sector number, return a pointer to a
// a sector with that ceiling height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
// jff 02/03/98 Add routine to find numeric model ceiling
//  around a sector specified by sector number
//  used only from generalized ceiling types
// jff 03/14/98 change first parameter to plain height to allow call
//  from routine not using ceiling_t
//
// killough 11/98: reformatted
sector_t *P_FindModelCeilingSector(const fixed_t ceildestheight, const int secnum)
{
    const int   linecount = sectors[secnum].linecount;

    for (int i = 0; i < linecount; i++)
        if (P_TwoSided(secnum, i))
        {
            sector_t    *sec = P_GetSector(secnum, i, (P_GetSide(secnum, i, 0)->sector->id == secnum));

            if (sec->ceilingheight == ceildestheight)
                return sec;
        }

    return NULL;
}

//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//

// Find the next sector with the same tag as a linedef.
// Rewritten by Lee Killough to use chained hashing to improve speed
int P_FindSectorFromLineTag(const line_t *line, int start)
{
    start = (start >= 0 ? sectors[start].nexttag : sectors[(unsigned int)line->tag % numsectors].firsttag);

    while (start >= 0 && sectors[start].tag != line->tag)
        start = sectors[start].nexttag;

    return start;
}

// killough 04/16/98: same thing, only for linedefs
int P_FindLineFromLineTag(const line_t *line, int start)
{
    start = (start >= 0 ? lines[start].nexttag : lines[(unsigned int)line->tag % numlines].firsttag);

    while (start >= 0 && lines[start].tag != line->tag)
        start = lines[start].nexttag;

    return start;
}

// Hash the sector tags across the sectors and linedefs.
void P_InitTagLists(void)
{
    for (int i = numsectors; --i >= 0; )                            // Initially make all slots empty.
        sectors[i].firsttag = -1;

    for (int i = numsectors; --i >= 0; )                            // Proceed from last to first sector
    {                                                               // so that lower sectors appear first
        const int   j = (unsigned int)sectors[i].tag % numsectors;  // Hash func

        sectors[i].nexttag = sectors[j].firsttag;                   // Prepend sector to chain
        sectors[j].firsttag = i;
    }

    // killough 04/17/98: same thing, only for linedefs
    for (int i = numlines; --i >= 0; )                              // Initially make all slots empty.
        lines[i].firsttag = -1;

    for (int i = numlines; --i >= 0; )                              // Proceed from last to first linedef
    {                                                               // so that lower linedefs appear first
        const int   j = (unsigned int)lines[i].tag % numlines;      // Hash func

        lines[i].nexttag = lines[j].firsttag;                       // Prepend linedef to chain
        lines[j].firsttag = i;
    }
}

//
// Find minimum light from an adjacent sector
//
int P_FindMinSurroundingLight(sector_t *sec, int min)
{
    const int   linecount = sec->linecount;

    for (int i = 0; i < linecount; i++)
    {
        const sector_t  *check = P_GetNextSector(sec->lines[i], sec);

        if (check && check->lightlevel < min)
            min = check->lightlevel;
    }

    return min;
}

//
// P_CanUnlockGenDoor
//
// Passed a generalized locked door linedef, returns whether
// the player has the keys necessary to unlock that door.
//
// Note: The linedef passed MUST be a generalized locked door type
//       or results are undefined.
//
// jff 02/05/98 routine added to test for unlockability of
//  generalized locked doors
//
// killough 11/98: reformatted
bool P_CanUnlockGenDoor(const line_t *line)
{
    static char buffer[1024];

    // does this line special distinguish between skulls and keys?
    const bool  skulliscard = (line->special & LockedNKeys) >> LockedNKeysShift;

    // determine for each case of lock type if player's keys are adequate
    switch ((line->special & LockedKey) >> LockedKeyShift)
    {
        case AnyKey:
            if (viewplayer->cards[it_redcard] <= 0 && viewplayer->cards[it_redskull] <= 0
                && viewplayer->cards[it_bluecard] <= 0 && viewplayer->cards[it_blueskull] <= 0
                && viewplayer->cards[it_yellowcard] <= 0 && viewplayer->cards[it_yellowskull] <= 0)
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_allkeys)
                {
                    viewplayer->neededcard = it_allkeys;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                if (M_StringCompare(playername, playername_default))
                    M_snprintf(buffer, sizeof(buffer), s_PD_ANY, "You", "", s_PD_KEYCARDORSKULLKEY);
                else
                    M_snprintf(buffer, sizeof(buffer), s_PD_ANY, playername, "s", s_PD_KEYCARDORSKULLKEY);

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            break;

        case RCard:
            if (viewplayer->cards[it_redcard] <= 0 && (!skulliscard || viewplayer->cards[it_redskull] <= 0))
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_redcard)
                {
                    viewplayer->neededcard = it_redcard;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                if (M_StringCompare(playername, playername_default))
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_REDK : s_PD_REDC), "You", "",
                        (viewplayer->cards[it_redskull] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_KEYCARD));
                else
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_REDK : s_PD_REDC), playername, "s",
                        (viewplayer->cards[it_redskull] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_KEYCARD));

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            break;

        case BCard:
            if (viewplayer->cards[it_bluecard] <= 0 && (!skulliscard || viewplayer->cards[it_blueskull] <= 0))
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_bluecard)
                {
                    viewplayer->neededcard = it_bluecard;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                if (M_StringCompare(playername, playername_default))
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_BLUEK : s_PD_BLUEC), "You", "",
                        (viewplayer->cards[it_blueskull] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_KEYCARD));
                else
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_BLUEK : s_PD_BLUEC), playername, "s",
                        (viewplayer->cards[it_blueskull] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_KEYCARD));

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            break;

        case YCard:
            if (viewplayer->cards[it_yellowcard] <= 0 && (!skulliscard || viewplayer->cards[it_yellowskull] <= 0))
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_yellowcard)
                {
                    viewplayer->neededcard = it_yellowcard;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                if (M_StringCompare(playername, playername_default))
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_YELLOWK : s_PD_YELLOWC), "You", "",
                        (viewplayer->cards[it_yellowskull] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_KEYCARD));
                else
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_YELLOWK : s_PD_YELLOWC), playername, "s",
                        (viewplayer->cards[it_yellowskull] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_KEYCARD));

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            break;

        case RSkull:
            if (viewplayer->cards[it_redskull] <= 0 && (!skulliscard || viewplayer->cards[it_redcard] <= 0))
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_redskull)
                {
                    viewplayer->neededcard = it_redskull;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                if (M_StringCompare(playername, playername_default))
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_REDK : s_PD_REDS), "You", "",
                        (viewplayer->cards[it_redcard] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_SKULLKEY));
                else
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_REDK : s_PD_REDS), playername, "s",
                        (viewplayer->cards[it_redcard] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_SKULLKEY));

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            break;

        case BSkull:
            if (viewplayer->cards[it_blueskull] <= 0 && (!skulliscard || viewplayer->cards[it_bluecard] <= 0))
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_blueskull)
                {
                    viewplayer->neededcard = it_blueskull;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                if (M_StringCompare(playername, playername_default))
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_BLUEK : s_PD_BLUES), "You", "",
                        (viewplayer->cards[it_bluecard] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_SKULLKEY));
                else
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_BLUEK : s_PD_BLUES), playername, "s",
                        (viewplayer->cards[it_bluecard] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_SKULLKEY));

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            break;

        case YSkull:
            if (viewplayer->cards[it_yellowskull] <= 0 && (!skulliscard || viewplayer->cards[it_yellowcard] <= 0))
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_yellowskull)
                {
                    viewplayer->neededcard = it_yellowskull;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                if (M_StringCompare(playername, playername_default))
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_YELLOWK : s_PD_YELLOWS), "You", "",
                        (viewplayer->cards[it_yellowcard] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_SKULLKEY));
                else
                    M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_YELLOWK : s_PD_YELLOWS), playername, "s",
                        (viewplayer->cards[it_yellowcard] == CARDNOTFOUNDYET && skulliscard ? s_PD_KEYCARDORSKULLKEY : s_PD_SKULLKEY));

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            break;

        case AllKeys:
            if (!skulliscard && (viewplayer->cards[it_redcard] <= 0 || viewplayer->cards[it_redskull] <= 0
                || viewplayer->cards[it_bluecard] <= 0 || viewplayer->cards[it_blueskull] <= 0
                || viewplayer->cards[it_yellowcard] <= 0 || viewplayer->cards[it_yellowskull] <= 0))
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_allkeys)
                {
                    viewplayer->neededcard = it_allkeys;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                M_snprintf(buffer, sizeof(buffer), s_PD_ALL6,
                    (M_StringCompare(playername, playername_default) ? "You" : playername),
                    (M_StringCompare(playername, playername_default) ? "" : "s"));

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            if (skulliscard && ((viewplayer->cards[it_redcard] <= 0 && viewplayer->cards[it_redskull] <= 0)
                || (viewplayer->cards[it_bluecard] <= 0 && viewplayer->cards[it_blueskull] <= 0)
                || (viewplayer->cards[it_yellowcard] <= 0 && viewplayer->cards[it_yellowskull] <= 0)))
            {
                if (!viewplayer->neededcardflash || viewplayer->neededcard != it_allkeys)
                {
                    viewplayer->neededcard = it_allkeys;
                    viewplayer->neededcardflash = NEEDEDCARDFLASH;
                }

                M_snprintf(buffer, sizeof(buffer), s_PD_ALL3,
                    (M_StringCompare(playername, playername_default) ? "You" : playername),
                    (M_StringCompare(playername, playername_default) ? "" : "s"));

                if (autousing && M_StringCompare(buffer, console[numconsolestrings - 1].string))
                    return false;

                HU_PlayerMessage(buffer, false, false);

                if (P_DoorClosed(line))
                    S_StartSound(viewplayer->mo, sfx_noway);

                return false;
            }

            break;
    }

    return true;
}

//
// P_SectorActive
//
// Passed a linedef special class (floor, ceiling, lighting) and a sector
// returns whether the sector is already busy with a linedef special of the
// same class. If old demo compatibility true, all linedef special classes
// are the same.
//
// jff 02/23/98 added to prevent old demos from
//  succeeding in starting multiple specials on one sector
//
// killough 11/98: reformatted
bool P_SectorActive(const special_e t, const sector_t *sec)
{
    return (t == floor_special ? !!sec->floordata :     // return whether
        (t == ceiling_special ? !!sec->ceilingdata :    // thinker of same
        true));                                         // don't know which special, must be active, shouldn't be here
}

//
// P_CheckTag
//
// Passed a line, returns true if the tag is non-zero or the line special
// allows no tag without harm.
//
// Note: Only line specials activated by walkover, pushing, or shooting are
//       checked by this routine.
//
// jff 02/27/98 Added to check for zero tag allowed for regular special types
//
bool P_CheckTag(const line_t *line)
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
        case Scroll_ScrollTextureLeft:
        case S1_ExitLevel_GoesToSecretLevel:
        case W1_ExitLevel:
        case WR_Light_ChangeTo35:
        case WR_Light_ChangeToBrightestAdjacent:
        case WR_Light_ChangeTo255:
        case Scroll_ScrollTextureRight:
        case WR_Teleport:
        case W1_Light_ChangeToDarkestAdjacent:
        case DR_Door_OpenWaitClose_Fast:
        case D1_Door_OpenStay_Fast:
        case W1_ExitLevel_GoesToSecretLevel:
        case W1_Teleport_MonstersOnly:
        case WR_Teleport_MonstersOnly:
        case SR_Light_ChangeTo255:
        case SR_Light_ChangeTo35:
        case WR_Light_StartBlinking:
        case WR_Light_ChangeToDarkestAdjacent:
        case S1_Light_ChangeToBrightestAdjacent:
        case S1_Light_ChangeTo35:
        case S1_Light_ChangeTo255:
        case S1_Light_StartBlinking:
        case S1_Light_ChangeToDarkestAdjacent:
        case S1_Teleport_AlsoMonsters:
        case SR_Light_ChangeToBrightestAdjacent:
        case SR_Light_StartBlinking:
        case SR_Light_ChangeToDarkestAdjacent:
        case SR_Teleport_AlsoMonsters:
        case G1_ExitLevel:
        case G1_ExitLevel_GoesToSecretLevel:
        case W1_Teleport_AlsoMonsters_Silent_SameAngle:
        case WR_Teleport_AlsoMonsters_Silent_SameAngle:
        case S1_Teleport_AlsoMonsters_Silent_SameAngle:
        case SR_Teleport_AlsoMonsters_Silent_SameAngle:
        case Scroll_ScrollWallUsingSidedefOffsets:
        case Translucent_MiddleTexture:
        case W1_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
        case WR_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
        case S1_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
        case SR_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
        case G1_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
        case GR_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
        case W1_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
        case WR_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
        case S1_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
        case SR_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
        case G1_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
        case GR_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
        case SetTheTargetSectorsColormap:
        case W1_SetTheTargetSectorsColormap:
        case WR_SetTheTargetSectorsColormap:
        case S1_SetTheTargetSectorsColormap:
        case SR_SetTheTargetSectorsColormap:
        case G1_SetTheTargetSectorsColormap:
        case GR_SetTheTargetSectorsColormap:
        case W1_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        case WR_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        case S1_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        case SR_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        case G1_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        case GR_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        case W1_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        case WR_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        case S1_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        case SR_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        case G1_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        case GR_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
            return true;    // zero tag allowed
    }

    return false;           // zero tag not allowed
}

//
// EVENTS
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//

//
// P_CrossSpecialLine - TRIGGER
// Called every time a thing origin is about
//  to cross a line with a non 0 special.
//
void P_CrossSpecialLine(line_t *line, const int side, mobj_t *thing, const bool bossaction)
{
    // Triggers that other things can activate
    if (!thing->player && !bossaction)
    {
        // Things that should NOT trigger specials...
        switch (thing->type)
        {
            case MT_ROCKET:
            case MT_PLASMA:
            case MT_BFG:
            case MT_TROOPSHOT:
            case MT_HEADSHOT:
            case MT_BRUISERSHOT:
                return;

            default:
                break;
        }
    }

    // jff 02/04/98 add check here for generalized linedef types
    {
        // pointer to line function is NULL by default, set non-NULL if
        // line special is walkover generalized linedef type
        bool (*linefunc)(line_t *line) = NULL;

        // check each range of generalized linedefs
        if (line->special >= GenEnd)
        {
            // Out of range for GenFloors
        }
        else if (line->special >= GenFloorBase)
        {
            if (!thing->player && !bossaction && ((line->special & FloorChange) || !(line->special & FloorModel)))
                return;                         // FloorModel is "Allow Monsters" if FloorChange is 0

            linefunc = &EV_DoGenFloor;
        }
        else if (line->special >= GenCeilingBase)
        {
            if (!thing->player && !bossaction && ((line->special & CeilingChange) || !(line->special & CeilingModel)))
                return;                         // CeilingModel is "Allow Monsters" if CeilingChange is 0

            linefunc = &EV_DoGenCeiling;
        }
        else if (line->special >= GenDoorBase)
        {
            if (!thing->player && !bossaction)
            {
                if (!(line->special & DoorMonster))
                    return;                     // monsters disallowed from this door

                if (line->flags & ML_SECRET)    // they can't open secret doors either
                    return;
            }

            linefunc = &EV_DoGenDoor;
        }
        else if (line->special >= GenLockedBase)
        {
            if (!thing->player && !bossaction)
                return;                         // monsters disallowed from unlocking doors

            if ((line->special & TriggerType) == WalkOnce || (line->special & TriggerType) == WalkMany)
            {
                // jff 04/01/98 check for being a walk type before reporting door type
                if (!P_CanUnlockGenDoor(line))
                    return;
            }
            else
                return;

            linefunc = &EV_DoGenLockedDoor;
        }
        else if (line->special >= GenLiftBase)
        {
            if (!thing->player && !bossaction && !(line->special & LiftMonster))
                return;                         // monsters disallowed

            linefunc = &EV_DoGenLift;
        }
        else if (line->special >= GenStairsBase)
        {
            if (!thing->player && !bossaction && !(line->special & StairMonster))
                return;                         // monsters disallowed

            linefunc = &EV_DoGenStairs;
        }
        else if (line->special >= GenCrusherBase)
        {
            if (!thing->player && !bossaction && !(line->special & CrusherMonster))
                return;                         // monsters disallowed

            linefunc = &EV_DoGenCrusher;
        }

        // if it was a valid generalized type
        if (linefunc)
            switch ((line->special & TriggerType) >> TriggerTypeShift)
            {
                case WalkOnce:
                    if (linefunc(line))
                        line->special = 0;      // clear special if a walk once type

                    return;

                case WalkMany:
                    linefunc(line);
                    return;

                default:                        // if not a walk type, do nothing here
                    return;
            }
    }

    if (!thing->player || bossaction)
    {
        bool    okay = bossaction;

        switch (line->special)
        {
            case W1_Teleport:
            case WR_Teleport:
            case W1_Teleport_MonstersOnly:
            case WR_Teleport_MonstersOnly:
            case W1_Teleport_AlsoMonsters_Silent_SameAngle:
            case WR_Teleport_AlsoMonsters_Silent_SameAngle:
            case W1_TeleportToLineWithSameTag_Silent_SameAngle:
            case WR_TeleportToLineWithSameTag_Silent_SameAngle:
            case W1_TeleportToLineWithSameTag_Silent_ReversedAngle:
            case WR_TeleportToLineWithSameTag_Silent_ReversedAngle:
            case W1_TeleportToLineWithSameTag_MonstersOnly_Silent_ReversedAngle:
            case WR_TeleportToLineWithSameTag_MonstersOnly_Silent_ReversedAngle:
            case W1_TeleportToLineWithSameTag_MonstersOnly_Silent:
            case WR_TeleportToLineWithSameTag_MonstersOnly_Silent:
            case W1_Teleport_MonstersOnly_Silent:
            case WR_Teleport_MonstersOnly_Silent:
                if (bossaction)
                    return;

            case W1_Door_OpenWaitClose:
            case W1_Lift_LowerWaitRaise:
            case WR_Lift_LowerWaitRaise:
                okay = true;
                break;
        }

        if (!okay)
            return;
    }

    if (!P_CheckTag(line))                      // jff 02/27/98 disallow zero tag on some types
        return;

    switch (line->special)
    {
        // Triggers
        case W1_Door_OpenStay:
            if (EV_DoDoor(line, DoorOpen, VDOORSPEED))
            {
                line->special = 0;

                if (nomonsters && (line->flags & ML_TRIGGER666))
                {
                    line_t  junk = { 0 };

                    switch (gameepisode)
                    {
                        case 1:
                            junk.tag = 666;
                            EV_DoFloor(&junk, LowerFloorToLowest);
                            break;

                        case 4:
                            junk.tag = 666;
                            EV_DoDoor(&junk, DoorBlazeOpen, VDOORSPEED * 4);
                            break;
                    }

                    line->flags &= ~ML_TRIGGER666;
                }
            }

            break;

        case W1_Door_CloseStay:
            if (EV_DoDoor(line, DoorClose, VDOORSPEED))
                line->special = 0;

            break;

        case W1_Door_OpenWaitClose:
            if (EV_DoDoor(line, DoorNormal, VDOORSPEED))
                line->special = 0;

            break;

        case W1_Floor_RaiseToLowestCeiling:
            if (E4M3)
            {
                if (EV_DoFloor(line, RaiseFloorCrush))
                    line->special = 0;
            }
            else if (EV_DoFloor(line, RaiseFloor))
                line->special = 0;

            break;

        case W1_Crusher_StartWithFastDamage:
            if (EV_DoCeiling(line, FastCrushAndRaise))
                line->special = 0;

            break;

        case W1_Stairs_RaiseBy8:
            if (EV_BuildStairs(line, FLOORSPEED / 4, 8 * FRACUNIT, false))
                line->special = 0;

            break;

        case W1_Lift_LowerWaitRaise:
            if (EV_DoPlat(line, DownWaitUpStay, 0))
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
            if (EV_DoDoor(line, DoorClose30ThenOpen, VDOORSPEED))
                line->special = 0;

            break;

        case W1_Light_StartBlinking:
            if (EV_StartLightStrobing(line))
                line->special = 0;

            break;

        case W1_Floor_LowerToHighestFloor:
            if (EV_DoFloor(line, LowerFloor))
                line->special = 0;

            break;

        case W1_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, RaiseToNearestAndChange, 0))
                line->special = 0;

            break;

        case W1_Crusher_StartWithSlowDamage:
            if (EV_DoCeiling(line, CrushAndRaise))
                line->special = 0;

            break;

        case W1_Floor_RaiseByShortestLowerTexture:
            if (EV_DoFloor(line, RaiseToTexture))
                line->special = 0;

            break;

        case W1_Light_ChangeTo35:
            if (EV_LightTurnOn(line, TICRATE))
                line->special = 0;

            break;

        case W1_Floor_LowerTo8AboveHighestFloor:
            if (EV_DoFloor(line, TurboLower))
                line->special = 0;

            break;

        case W1_Floor_LowerToLowestFloor_ChangesTexture:
            if (EV_DoFloor(line, LowerAndChange))
                line->special = 0;

            break;

        case W1_Floor_LowerToLowestFloor:
            if (EV_DoFloor(line, LowerFloorToLowest))
                line->special = 0;

            break;

        case W1_Teleport:
            if (EV_Teleport(line, side, thing) && !(thing->flags & MF_CORPSE))
                line->special = 0;

            break;

        case W1_Ceiling_RaiseToHighestCeiling:
            if (EV_DoCeiling(line, RaiseToHighest))
                line->special = 0;

            break;

        case W1_Ceiling_LowerTo8AboveFloor:
            if (EV_DoCeiling(line, LowerAndCrush))
                line->special = 0;

            break;

        case W1_ExitLevel:
            // killough 10/98: prevent zombies from exiting levels
            if (bossaction || !(thing->player && thing->player->health <= 0 && !compat_zombie))
                G_ExitLevel();

            break;

        case W1_Floor_StartMovingUpAndDown:
            if (EV_DoPlat(line, PerpetualRaise, 0))
                line->special = 0;

            break;

        case W1_Floor_StopMoving:
            if (EV_StopPlat(line))
                line->special = 0;

            break;

        case W1_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            if (EV_DoFloor(line, RaiseFloorCrush))
                line->special = 0;

            break;

        case W1_Crusher_Stop:
            if (EV_CeilingCrushStop(line))
                line->special = 0;

            break;

        case W1_Floor_RaiseBy24:
            if (EV_DoFloor(line, RaiseFloor24))
                line->special = 0;

            break;

        case W1_Floor_RaiseBy24_ChangesTexture:
            if (EV_DoFloor(line, RaiseFloor24AndChange))
                line->special = 0;

            break;

        case W1_Stairs_RaiseBy16_Fast:
            if (EV_BuildStairs(line, FLOORSPEED * 4, 16 * FRACUNIT, true))
                line->special = 0;

            break;

        case W1_Light_ChangeToDarkestAdjacent:
            if (EV_TurnTagLightsOff(line))
                line->special = 0;

            break;

        case W1_Door_OpenWaitClose_Fast:
            if (EV_DoDoor(line, DoorBlazeRaise, VDOORSPEED * 4))
                line->special = 0;

            break;

        case W1_Door_OpenStay_Fast:
            if (EV_DoDoor(line, DoorBlazeOpen, VDOORSPEED * 4))
                line->special = 0;

            break;

        case W1_Door_CloseStay_Fast:
            if (EV_DoDoor(line, DoorBlazeClose, VDOORSPEED * 4))
                line->special = 0;

            break;

        case W1_Floor_RaiseToNextHighestFloor:
            if (EV_DoFloor(line, RaiseFloorToNearest))
                line->special = 0;

            break;

        case W1_Lift_LowerWaitRaise_Fast:
            if (EV_DoPlat(line, BlazeDWUS, 0))
                line->special = 0;

            break;

        case W1_ExitLevel_GoesToSecretLevel:
            // killough 10/98: prevent zombies from exiting levels
            if (bossaction || !(thing->player && thing->player->health <= 0 && !compat_zombie))
                G_SecretExitLevel();

            break;

        case W1_Teleport_MonstersOnly:
            if (!thing->player && EV_Teleport(line, side, thing) && !(thing->flags & MF_CORPSE))
                line->special = 0;

            break;

        case W1_Floor_RaiseToNextHighestFloor_Fast:
            if (EV_DoFloor(line, RaiseFloorTurbo))
                line->special = 0;

            break;

        case W1_Crusher_StartWithSlowDamage_Silent:
            if (EV_DoCeiling(line, SilentCrushAndRaise))
                line->special = 0;

            break;

        // Retriggers
        case WR_Ceiling_LowerTo8AboveFloor:
            EV_DoCeiling(line, LowerAndCrush);
            break;

        case WR_Crusher_StartWithSlowDamage:
            EV_DoCeiling(line, CrushAndRaise);
            break;

        case WR_Crusher_Stop:
            EV_CeilingCrushStop(line);
            break;

        case WR_Door_CloseStay:
            EV_DoDoor(line, DoorClose, VDOORSPEED);
            break;

        case WR_Door_CloseStayOpen:
            EV_DoDoor(line, DoorClose30ThenOpen, VDOORSPEED);
            break;

        case WR_Crusher_StartWithFastDamage:
            EV_DoCeiling(line, FastCrushAndRaise);
            break;

        case WR_Light_ChangeTo35:
            EV_LightTurnOn(line, TICRATE);
            break;

        case WR_Light_ChangeToBrightestAdjacent:
            EV_LightTurnOn(line, 0);
            break;

        case WR_Light_ChangeTo255:
            EV_LightTurnOn(line, 255);
            break;

        case WR_Floor_LowerToLowestFloor:
            EV_DoFloor(line, LowerFloorToLowest);
            break;

        case WR_Floor_LowerToHighestFloor:
            EV_DoFloor(line, LowerFloor);
            break;

        case WR_Floor_LowerToLowestFloor_ChangesTexture:
            EV_DoFloor(line, LowerAndChange);
            break;

        case WR_Door_OpenStay:
            EV_DoDoor(line, DoorOpen, VDOORSPEED);
            break;

        case WR_Floor_StartMovingUpAndDown:
            EV_DoPlat(line, PerpetualRaise, 0);
            break;

        case WR_Lift_LowerWaitRaise:
            EV_DoPlat(line, DownWaitUpStay, 0);
            break;

        case WR_Floor_StopMoving:
            EV_StopPlat(line);
            break;

        case WR_Door_OpenWaitClose:
            EV_DoDoor(line, DoorNormal, VDOORSPEED);
            break;

        case WR_Floor_RaiseToLowestCeiling:
            EV_DoFloor(line, RaiseFloor);
            break;

        case WR_Floor_RaiseBy24:
            EV_DoFloor(line, RaiseFloor24);
            break;

        case WR_Floor_RaiseBy24_ChangesTexture:
            EV_DoFloor(line, RaiseFloor24AndChange);
            break;

        case WR_Floor_RaiseTo8BelowLowestCeiling_Crushes:
            EV_DoFloor(line, RaiseFloorCrush);
            break;

        case WR_Floor_RaiseToNextHighestFloor_ChangesTexture:
            EV_DoPlat(line, RaiseToNearestAndChange, 0);
            break;

        case WR_Floor_RaiseByShortestLowerTexture:
            EV_DoFloor(line, RaiseToTexture);
            break;

        case WR_Teleport:
            EV_Teleport(line, side, thing);
            break;

        case WR_Floor_LowerTo8AboveHighestFloor:
            EV_DoFloor(line, TurboLower);
            break;

        case WR_Door_OpenWaitClose_Fast:
            EV_DoDoor(line, DoorBlazeRaise, VDOORSPEED * 4);
            break;

        case WR_Door_OpenStay_Fast:
            EV_DoDoor(line, DoorBlazeOpen, VDOORSPEED * 4);
            break;

        case WR_Door_CloseStay_Fast:
            EV_DoDoor(line, DoorBlazeClose, VDOORSPEED * 4);
            break;

        case WR_Lift_LowerWaitRaise_Fast:
            EV_DoPlat(line, BlazeDWUS, 0);
            break;

        case WR_Teleport_MonstersOnly:
            if (!thing->player)
                EV_Teleport(line, side, thing);

            break;

        case WR_Floor_RaiseToNextHighestFloor:
            EV_DoFloor(line, RaiseFloorToNearest);
            break;

        case WR_Floor_RaiseToNextHighestFloor_Fast:
            EV_DoFloor(line, RaiseFloorTurbo);
            break;

        // Extended triggers
        case W1_Floor_RaiseBy512:
            if (EV_DoFloor(line, RaiseFloor512))
                line->special = 0;

            break;

        case W1_Lift_RaiseBy24_ChangesTexture:
            if (EV_DoPlat(line, RaiseAndChange, 24))
                line->special = 0;

            break;

        case W1_Lift_RaiseBy32_ChangesTexture:
            if (EV_DoPlat(line, RaiseAndChange, 32))
                line->special = 0;

            break;

        case W1_CeilingLowerToFloor_Fast:
            if (EV_DoCeiling(line, LowerToFloor))
                line->special = 0;

            break;

        case W1_Floor_RaiseDonut_ChangesTexture:
            if (EV_DoDonut(line))
                line->special = 0;

            break;

        case W1_Ceiling_LowerToLowestCeiling:
            if (EV_DoCeiling(line, LowerToLowest))
                line->special = 0;

            break;

        case W1_Ceiling_LowerToHighestFloor:
            if (EV_DoCeiling(line, LowerToMaxFloor))
                line->special = 0;

            break;

        case W1_Teleport_AlsoMonsters_Silent_SameAngle:
            if (EV_SilentTeleport(line, side, thing) && !(thing->flags & MF_CORPSE))
                line->special = 0;

            break;

        case W1_ChangeTextureAndEffect:
            if (EV_DoChange(line, TrigChangeOnly))
                line->special = 0;

            break;

        case W1_ChangeTextureAndEffectToNearest:
            if (EV_DoChange(line, NumChangeOnly))
                line->special = 0;

            break;

        case W1_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, LowerFloorToNearest))
                line->special = 0;

            break;

        case W1_Lift_RaiseToNextHighestFloor_Fast:
            if (EV_DoElevator(line, ElevateUp))
                line->special = 0;

            break;

        case W1_Lift_LowerToNextLowestFloor_Fast:
            if (EV_DoElevator(line, ElevateDown))
                line->special = 0;

            break;

        case W1_Lift_MoveToSameFloorHeight_Fast:
            if (EV_DoElevator(line, ElevateCurrent))
                line->special = 0;

            break;

        case W1_TeleportToLineWithSameTag_Silent_SameAngle:
            if (EV_SilentLineTeleport(line, side, thing, false) && !(thing->flags & MF_CORPSE))
                line->special = 0;

            break;

        case W1_TeleportToLineWithSameTag_Silent_ReversedAngle:
            if (EV_SilentLineTeleport(line, side, thing, true) && !(thing->flags & MF_CORPSE))
                line->special = 0;

            break;

        case W1_TeleportToLineWithSameTag_MonstersOnly_Silent_ReversedAngle:
            if (!thing->player && EV_SilentLineTeleport(line, side, thing, true) && !(thing->flags & MF_CORPSE))
                line->special = 0;

            break;

        case W1_TeleportToLineWithSameTag_MonstersOnly_Silent:
            if (!thing->player && EV_SilentLineTeleport(line, side, thing, false) && !(thing->flags & MF_CORPSE))
                line->special = 0;

            break;

        case W1_Teleport_MonstersOnly_Silent:
            if (!thing->player && EV_SilentTeleport(line, side, thing) && !(thing->flags & MF_CORPSE))
                line->special = 0;

            break;

        // Extended retriggers
        case WR_Floor_RaiseBy512:
            EV_DoFloor(line, RaiseFloor512);
            break;

        case WR_Lift_RaiseBy24_ChangesTexture:
            EV_DoPlat(line, RaiseAndChange, 24);
            break;

        case WR_Lift_RaiseBy32_ChangesTexture:
            EV_DoPlat(line, RaiseAndChange, 32);
            break;

        case WR_Crusher_Start_Silent:
            EV_DoCeiling(line, SilentCrushAndRaise);
            break;

        case WR_Ceiling_RaiseToHighestCeiling:
            EV_DoCeiling(line, RaiseToHighest);
            EV_DoFloor(line, LowerFloorToLowest);
            break;

        case WR_Ceiling_LowerToFloor_Fast:
            EV_DoCeiling(line, LowerToFloor);
            break;

        case WR_Stairs_RaiseBy8:
            EV_BuildStairs(line, FLOORSPEED / 4, 8 * FRACUNIT, false);
            break;

        case WR_Stairs_RaiseBy16_Fast:
            EV_BuildStairs(line, FLOORSPEED * 4, 16 * FRACUNIT, true);
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
            EV_DoCeiling(line, LowerToLowest);
            break;

        case WR_Ceiling_LowerToHighestFloor:
            EV_DoCeiling(line, LowerToMaxFloor);
            break;

        case WR_Teleport_AlsoMonsters_Silent_SameAngle:
            EV_SilentTeleport(line, side, thing);
            break;

        case WR_Lift_RaiseToCeiling_Instantly:
            EV_DoPlat(line, ToggleUpDn, 0);
            break;

        case WR_ChangeTextureAndEffect:
            EV_DoChange(line, TrigChangeOnly);
            break;

        case WR_ChangeTextureAndEffectToNearest:
            EV_DoChange(line, NumChangeOnly);
            break;

        case WR_Floor_LowerToNearestFloor:
            EV_DoFloor(line, LowerFloorToNearest);
            break;

        case WR_Lift_RaiseToNextHighestFloor_Fast:
            EV_DoElevator(line, ElevateUp);
            break;

        case WR_Lift_LowerToNextLowestFloor_Fast:
            EV_DoElevator(line, ElevateDown);
            break;

        case WR_Lift_MoveToSameFloorHeight_Fast:
            EV_DoElevator(line, ElevateCurrent);
            break;

        case WR_TeleportToLineWithSameTag_Silent_SameAngle:
            EV_SilentLineTeleport(line, side, thing, false);
            break;

        case WR_TeleportToLineWithSameTag_Silent_ReversedAngle:
            EV_SilentLineTeleport(line, side, thing, true);
            break;

        case WR_TeleportToLineWithSameTag_MonstersOnly_Silent_ReversedAngle:
            if (!thing->player)
                EV_SilentLineTeleport(line, side, thing, true);

            break;

        case WR_TeleportToLineWithSameTag_MonstersOnly_Silent:
            if (!thing->player)
                EV_SilentLineTeleport(line, side, thing, false);

            break;

        case WR_Teleport_MonstersOnly_Silent:
            if (!thing->player)
                EV_SilentTeleport(line, side, thing);

            break;

        // ID24 specials

        case W1_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
            S_ChangeMusInfoMusic((side ? line->backmusic : line->frontmusic), true);
            line->special = 0;
            break;

        case WR_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
            S_ChangeMusInfoMusic((side ? line->backmusic : line->frontmusic), true);
            break;

        case W1_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
            S_ChangeMusInfoMusic((side ? line->backmusic : line->frontmusic), false);
            line->special = 0;
            break;

        case WR_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
            S_ChangeMusInfoMusic((side ? line->backmusic : line->frontmusic), false);
            break;

        case W1_ExitToTheNextMapAndResetInventory:
            if (bossaction || !(thing->player && thing->player->health <= 0 && !compat_zombie))
            {
                resetinventory = true;
                G_ExitLevel();
            }

            break;

        case W1_ExitToTheSecretMapAndResetInventory:
            if (bossaction || !(thing->player && thing->player->health <= 0 && !compat_zombie))
            {
                resetinventory = true;
                G_SecretExitLevel();
            }

            break;

        case W1_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        {
            const int   music = (side ? line->backmusic : line->frontmusic);

            S_ChangeMusInfoMusic((music ? music : musinfo.items[0]), true);
            line->special = 0;
            break;
        }

        case WR_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        {
            const int   music = (side ? line->backmusic : line->frontmusic);

            S_ChangeMusInfoMusic((music ? music : musinfo.items[0]), true);
            break;
        }

        case W1_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        {
            const int   music = (side ? line->backmusic : line->frontmusic);

            S_ChangeMusInfoMusic((music ? music : musinfo.items[0]), false);
            line->special = 0;
            break;
        }

        case WR_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        {
            const int   music = (side ? line->backmusic : line->frontmusic);

            S_ChangeMusInfoMusic((music ? music : musinfo.items[0]), false);
            break;
        }

        case W1_SetTheTargetSectorsColormap:
            // [KLN] 04/13/25 support for the ID24 spec "set target" colormap 2076 (W1)
            for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                sectors[s].colormap = (side ? sides[*line->sidenum].backcolormap : sides[*line->sidenum].frontcolormap);

            line->special = 0;
            break;

        case WR_SetTheTargetSectorsColormap:
            // [KLN] 04/13/25 support for the ID24 spec "set target" colormap 2077 (WR)
            for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                sectors[s].colormap = (side ? sides[*line->sidenum].backcolormap : sides[*line->sidenum].frontcolormap);

            break;
    }
}

//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void P_ShootSpecialLine(const mobj_t *thing, line_t *line, const int side)
{
    // jff 02/04/98 add check here for generalized linedef
    // pointer to line function is NULL by default, set non-null if
    // line special is gun triggered generalized linedef type
    bool (*linefunc)(line_t *line) = NULL;

    unsigned short  special = line->special;

    // check each range of generalized linedefs
    if (special >= GenEnd)
    {
        // Out of range for GenFloors
    }
    else if (special >= GenFloorBase)
    {
        if (!thing->player && ((special & FloorChange) || !(special & FloorModel)))
            return;                         // FloorModel is "Allow Monsters" if FloorChange is 0

        linefunc = &EV_DoGenFloor;
    }
    else if (special >= GenCeilingBase)
    {
        if (!thing->player && ((special & CeilingChange) || !(special & CeilingModel)))
            return;                         // CeilingModel is "Allow Monsters" if CeilingChange is 0

        linefunc = &EV_DoGenCeiling;
    }
    else if (special >= GenDoorBase)
    {
        if (!thing->player)
        {
            if (!(special & DoorMonster))
                return;                     // monsters disallowed from this door

            if (line->flags & ML_SECRET)    // they can't open secret doors either
                return;
        }

        linefunc = &EV_DoGenDoor;
    }
    else if (special >= GenLockedBase)
    {
        if (!thing->player)
            return;                         // monsters disallowed from unlocking doors

        if ((special & TriggerType) == GunOnce || (special & TriggerType) == GunMany)
        {
            // jff 04/01/98 check for being a gun type before reporting door type
            if (!P_CanUnlockGenDoor(line))
                return;
        }
        else
            return;

        linefunc = &EV_DoGenLockedDoor;
    }
    else if (special >= GenLiftBase)
    {
        if (!thing->player && !(special & LiftMonster))
            return;                         // monsters disallowed

        linefunc = &EV_DoGenLift;
    }
    else if (special >= GenStairsBase)
    {
        if (!thing->player && !(special & StairMonster))
            return;                         // monsters disallowed

        linefunc = &EV_DoGenStairs;
    }
    else if (special >= GenCrusherBase)
    {
        if (!thing->player && !(special & CrusherMonster))
            return;                         // monsters disallowed

        linefunc = &EV_DoGenCrusher;
    }

    if (linefunc)
        switch ((special & TriggerType) >> TriggerTypeShift)
        {
            case GunOnce:
                if (linefunc(line))
                    P_ChangeSwitchTexture(line, false);

                return;

            case GunMany:
                if (linefunc(line))
                    P_ChangeSwitchTexture(line, true);

                return;

            default:                        // if not a gun type, do nothing here
                return;
        }

    // Impacts that other things can activate.
    if (!thing->player && special != GR_Door_OpenStay)
        return;

    if (!P_CheckTag(line))                  // jff 02/27/98 disallow zero tag on some types
        return;

    switch (special)
    {
        case G1_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, RaiseFloor))
                P_ChangeSwitchTexture(line, false);

            break;

        case GR_Door_OpenStay:
            if (EV_DoDoor(line, DoorOpen, VDOORSPEED))
                P_ChangeSwitchTexture(line, !canmodify);

            break;

        case G1_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, RaiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, false);

            break;

        case G1_ExitLevel:
            // killough 10/98: prevent zombies from exiting levels
            if (thing->player && thing->player->health <= 0 && !compat_zombie)
                break;

            P_ChangeSwitchTexture(line, false);
            G_ExitLevel();
            break;

        case G1_ExitLevel_GoesToSecretLevel:
            // killough 10/98: prevent zombies from exiting levels
            if (thing->player && thing->player->health <= 0 && !compat_zombie)
                break;

            P_ChangeSwitchTexture(line, false);
            G_SecretExitLevel();
            break;

        // ID24 specials

        case G1_ExitToTheNextMapAndResetInventory:
            if (thing->player && thing->player->health <= 0 && !compat_zombie)
                break;

            P_ChangeSwitchTexture(line, false);
            resetinventory = true;
            G_ExitLevel();
            break;

        case G1_ExitToTheSecretMapAndResetInventory:
            if (thing->player && thing->player->health <= 0 && !compat_zombie)
                break;

            P_ChangeSwitchTexture(line, false);
            resetinventory = true;
            G_SecretExitLevel();
            break;

        case G1_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
            S_ChangeMusInfoMusic((side ? line->backmusic : line->frontmusic), true);
            line->special = 0;
            break;

        case GR_ChangeMusicAndMakeItLoopOnlyIfATrackIsDefined:
            S_ChangeMusInfoMusic((side ? line->backmusic : line->frontmusic), true);
            break;

        case G1_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
            S_ChangeMusInfoMusic((side ? line->backmusic : line->frontmusic), false);
            line->special = 0;
            break;

        case GR_ChangeMusicAndMakeItPlayOnlyOnceAndStopAllMusicAfter:
            S_ChangeMusInfoMusic((side ? line->backmusic : line->frontmusic), false);
            break;

        case G1_SetTheTargetSectorsColormap:
            // [KLN] 04/13/25 support for the ID24 spec "set target" colormap 2080 (G1)
            for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                sectors[s].colormap = sides[*line->sidenum].frontcolormap;

            P_ChangeSwitchTexture(line, false);
            break;

        case GR_SetTheTargetSectorsColormap:
           // [KLN] 04/13/25 support for the ID24 spec "set target" colormap 2081 (GR)
           for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                sectors[s].colormap = sides[*line->sidenum].frontcolormap;

            P_ChangeSwitchTexture(line, true);
            break;

        case G1_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        {
            const int   music = (side ? line->backmusic : line->frontmusic);

            S_ChangeMusInfoMusic((music ? music : musinfo.items[0]), true);
            line->special = 0;
            break;
        }

        case GR_ChangeMusicAndMakeItLoop_ResetToLoopingDefaultIfNoTrackDefined:
        {
            const int   music = (side ? line->backmusic : line->frontmusic);

            S_ChangeMusInfoMusic((music ? music : musinfo.items[0]), true);
            break;
        }

        case G1_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        {
            const int   music = (side ? line->backmusic : line->frontmusic);

            S_ChangeMusInfoMusic((music ? music : musinfo.items[0]), false);
            line->special = 0;
            break;
        }

        case GR_ChangeMusicAndMakeItPlayOnlyOnce_ResetToLoopingDefaultIfNoTrackDefined:
        {
            const int   music = (side ? line->backmusic : line->frontmusic);

            S_ChangeMusInfoMusic((music ? music : musinfo.items[0]), false);
            break;
        }
    }
}

static void P_SecretFound(void)
{
    char    buffer[133] = "";

    M_snprintf(buffer, sizeof(buffer), s_SECRETMESSAGE,
        (M_StringCompare(playername, playername_default) ? "you" : playername));
    C_PlayerMessage(buffer);

    if (messages && secretmessages)
    {
        HU_SetPlayerMessage(buffer, false, false);
        message_dontfuckwithme = true;
        message_secret = true;
        S_StartSound(NULL, sfx_secret);
    }

    viewplayer->secretcount++;
    stat_secretsfound = SafeAdd(stat_secretsfound, 1);
}

//
// P_PlayerInSpecialSector
// Called every tic frame
//  that the player origin is in a special sector
//
void P_PlayerInSpecialSector(sector_t *sector)
{
    // jff add if to handle old vs generalized types
    if (sector->special < 32)   // regular sector specials
    {
        switch (sector->special)
        {
            case DamageNegative5Or10PercentHealth:
                if (!(maptime & 31) && !(viewplayer->cheats & CF_GODMODE)
                    && !viewplayer->powers[pw_ironfeet])
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 10, true, false);

                break;

            case DamageNegative2Or5PercentHealth:
                if (!(maptime & 31) && !(viewplayer->cheats & CF_GODMODE)
                    && !viewplayer->powers[pw_ironfeet])
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 5, true, false);

                break;

            case DamageNegative10Or20PercentHealth:
            case DamageNegative10Or20PercentHealthAndLightBlinks_2Hz:
                if (!(maptime & 31) && !(viewplayer->cheats & CF_GODMODE)
                    && (!viewplayer->powers[pw_ironfeet] || M_Random() < 5))
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 20, true, false);

                break;

            case Secret:
                if (!(viewplayer->cheats & CF_NOCLIP) && !freeze)
                {
                    P_SecretFound();
                    sector->special = 0;

                    for (int i = 0; i < sector->linecount; i++)
                        sector->lines[i]->flags &= ~ML_SECRET;
                }

                break;

            case DamageNegative10Or20PercentHealthAndEndLevel:
                if (!(maptime & 31))
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 20, true, false);

                if (viewplayer->health <= 10)
                    G_ExitLevel();

                break;
        }
    }
    else if (sector->special & DEATH_MASK)
    {
        // MBF21
        switch ((sector->special & DAMAGE_MASK) >> DAMAGE_SHIFT)
        {
            case 0:
                if (!viewplayer->powers[pw_invulnerability]
                    && !viewplayer->powers[pw_ironfeet])
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 10000, false, false);

                break;

            case 1:
                P_DamageMobj(viewplayer->mo, NULL, NULL, 10000, false, false);
                break;

            case 2:
                P_DamageMobj(viewplayer->mo, NULL, NULL, 10000, false, false);
                G_ExitLevel();
                break;

            case 3:
                P_DamageMobj(viewplayer->mo, NULL, NULL, 10000, false, false);
                G_SecretExitLevel();
                break;
        }
    }
    else
    {
        switch ((sector->special & DAMAGE_MASK) >> DAMAGE_SHIFT)
        {
            case 0:
                // no damage
                break;

            case 1:
                // 2/5 damage per 31 tics
                if (!(maptime & 31) && !(viewplayer->cheats & CF_GODMODE)
                    && !viewplayer->powers[pw_ironfeet])
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 5, true, false);

                break;

            case 2:
                // 5/10 damage per 31 tics
                if (!(maptime & 31) && !(viewplayer->cheats & CF_GODMODE)
                    && !viewplayer->powers[pw_ironfeet])
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 10, true, false);

                break;

            case 3:
                // 10/20 damage per 31 tics
                if (!(maptime & 31) && !(viewplayer->cheats & CF_GODMODE)
                    && (!viewplayer->powers[pw_ironfeet] || M_Random() < 5))
                    P_DamageMobj(viewplayer->mo, NULL, NULL, 20, true, false);

                break;
        }

        if ((sector->special & SECRET_MASK) && !(viewplayer->cheats & CF_NOCLIP) && !freeze)
        {
            P_SecretFound();
            sector->special &= ~SECRET_MASK;

            for (int i = 0; i < sector->linecount; i++)
                sector->lines[i]->flags &= ~ML_SECRET;

            if (sector->special < 32)   // if all extended bits clear,
                sector->special = 0;    // sector is not special anymore
        }
    }
}

//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//
void P_UpdateSpecials(void)
{
    // ANIMATE FLATS AND TEXTURES GLOBALLY
    for (anim_t *anim = anims; anim < lastanim; anim++)
        for (int i = 0; i < anim->numpics; i++)
        {
            const int   pic = anim->basepic + (animatedtic / anim->speed + i) % anim->numpics;

            if (anim->istexture)
                texturetranslation[anim->basepic + i] = pic;
            else
                flattranslation[anim->basepic + i] = firstflat + pic;
        }

    if (menuactive && (gametime & 2))
        return;

    animatedliquiddiff += animatedliquiddiffs[animatedtic & (ANIMATEDLIQUIDDIFFS - 1)];

    if ((animatedliquidxoffs += animatedliquidxdir) > 64 * FRACUNIT)
        animatedliquidxoffs = 0;

    if ((animatedliquidyoffs += animatedliquidydir) > 64 * FRACUNIT)
        animatedliquidyoffs = 0;

    skycolumnoffset += skyscrolldelta;

    R_UpdateSky();

    if (menuactive)
        return;

    if (timer && !timeremaining && viewplayer->health > 0)
        G_ExitLevel();

    // DO BUTTONS
    for (int i = 0; i < maxbuttons; i++)
        if (buttonlist[i].btimer && !--buttonlist[i].btimer)
        {
            line_t          *line = buttonlist[i].line;
            const sector_t  *sector = line->backsector;
            const int       sidenum = line->sidenum[0];
            const short     toptexture = sides[sidenum].toptexture;
            const short     midtexture = sides[sidenum].midtexture;
            const short     bottomtexture = sides[sidenum].bottomtexture;
            const int       btexture = buttonlist[i].btexture;

            switch (buttonlist[i].bwhere)
            {
                case top:
                    sides[sidenum].toptexture = btexture;

                    if (midtexture == toptexture)
                        sides[sidenum].midtexture = btexture;

                    if (bottomtexture == toptexture)
                        sides[sidenum].bottomtexture = btexture;

                    break;

                case middle:
                    sides[sidenum].midtexture = btexture;

                    if (toptexture == midtexture)
                        sides[sidenum].toptexture = btexture;

                    if (bottomtexture == midtexture)
                        sides[sidenum].bottomtexture = btexture;

                    break;

                case bottom:
                    sides[sidenum].bottomtexture = btexture;

                    if (toptexture == bottomtexture)
                        sides[sidenum].toptexture = btexture;

                    if (midtexture == bottomtexture)
                        sides[sidenum].midtexture = btexture;

                    break;

                case nowhere:
                    break;
            }

            if (!sector || (!sector->floordata && !sector->ceilingdata) || line->tag != sector->tag)
                S_StartSectorSound(buttonlist[i].soundorg, sfx_swtchn);
        }
}

//
// Special stuff that cannot be categorized
//
bool EV_DoDonut(const line_t *line)
{
    int     secnum = -1;
    bool    rtn = false;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sector_t    *s1 = sectors + secnum;
        sector_t    *s2;

        // ALREADY MOVING? IF SO, KEEP GOING...
        if (!compat_floormove && P_SectorActive(floor_special, s1))
            continue;

        if (!(s2 = P_GetNextSector(s1->lines[0], s1)))
            continue;

        if (P_SectorActive(floor_special, s2))
            continue;

        for (int i = 0; i < s2->linecount; i++)
        {
            const sector_t  *s3 = s2->lines[i]->backsector;
            floormove_t     *floor;

            if (!s3 || s3 == s1)
                continue;

            rtn = true;

            // Spawn rising slime
            floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, NULL);

            floor->thinker.function = &T_MoveFloor;
            P_AddThinker(&floor->thinker);

            s2->floordata = floor;
            floor->type = DonutRaise;
            floor->direction = 1;
            floor->sector = s2;
            floor->speed = FLOORSPEED / 2;
            floor->texture = s3->floorpic;
            floor->floordestheight = s3->floorheight;
            floor->stopsound = (s2->floorheight != floor->floordestheight);

            // Spawn lowering donut-hole
            floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, NULL);

            floor->thinker.function = &T_MoveFloor;
            P_AddThinker(&floor->thinker);

            s1->floordata = floor;
            floor->type = LowerFloor;
            floor->direction = -1;
            floor->sector = s1;
            floor->speed = FLOORSPEED / 2;
            floor->floordestheight = s3->floorheight;
            floor->stopsound = (s1->floorheight != floor->floordestheight);
            break;
        }
    }

    return rtn;
}

void P_SetTimer(const int minutes)
{
    timer = minutes;
    timeremaining = timer * 60 * TICRATE;
}

//
// SPECIAL SPAWNING
//

//
// P_SpawnSpecials
// After the map has been loaded, scan for specials that spawn thinkers
//
void P_SpawnSpecials(void)
{
    line_t      *line = lines;
    sector_t    *sector = sectors;
    const int   p = M_CheckParmWithArgs("-timer", 1);

    if (p)
    {
        const int   minutes = strtol(myargv[p + 1], NULL, 10);

        if (minutes > 0)
        {
            char    *temp = commify(minutes);

            timer = BETWEEN(0, minutes, TIMERMAXMINUTES);
            C_Output("A " BOLD("-timer") " parameter was found on the command-line. A timer "
                "has been set for %s minute%s. %s will automatically exit each map once the timer runs out.",
                temp, (minutes == 1 ? "" : "s"), (M_StringCompare(playername, playername_default) ? "You" : playername));
            P_SetTimer(minutes);
            free(temp);
        }
    }

    if (M_CheckParm("-avg"))
    {
        P_SetTimer(20);
        C_Output("An " BOLD("-avg") " parameter was found on the command-line. A timer "
            "has been set for %i minutes. %s will automatically exit each map once the timer runs out.",
            timer, (M_StringCompare(playername, playername_default) ? "You" : playername));
    }

    // Init special SECTORs.
    for (int i = 0; i < numsectors; i++, sector++)
    {
        if (!sector->special)
            continue;

        if (sector->special & SECRET_MASK)
            totalsecrets++;

        switch (sector->special & 31)
        {
            case LightBlinks_Randomly:
                P_SpawnLightFlash(sector);
                break;

            case LightBlinks_2Hz:
            case DamageNegative10Or20PercentHealthAndLightBlinks_2Hz:
                P_SpawnStrobeFlash(sector, FASTDARK, false);
                break;

            case LightBlinks_1Hz:
                P_SpawnStrobeFlash(sector, SLOWDARK, false);
                break;

            case LightGlows_1PlusSec:
                P_SpawnGlowingLight(sector);
                break;

            case Secret:
                if (sector->special < 32)
                    totalsecrets++;

                break;

            case Door_CloseStay_After30sec:
                P_SpawnDoorCloseIn30(sector);
                break;

            case LightBlinks_1HzSynchronized:
                P_SpawnStrobeFlash(sector, SLOWDARK, true);
                break;

            case LightBlinks_2HzSynchronized:
                P_SpawnStrobeFlash(sector, FASTDARK, true);
                break;

            case Door_OpenClose_OpensAfter5Min:
                P_SpawnDoorRaiseIn5Mins(sector);
                break;

            case LightFlickers_Randomly:
                P_SpawnFireFlicker(sector);
                break;
        }
    }

    P_RemoveAllActiveCeilings();        // jff 02/22/98 use killough's scheme
    P_RemoveAllActivePlats();           // killough

    for (int i = 0; i < maxbuttons; i++)
        memset(&buttonlist[i], 0, sizeof(button_t));

    P_SpawnScrollers();                 // killough 03/07/98: Add generalized scrollers
    P_SpawnFriction();                  // phares 03/12/98: New friction model using linedefs
    P_SpawnPushers();                   // phares 03/20/98: New pusher model using linedefs

    for (int i = 0; i < numlines; i++, line++)
        switch (line->special)
        {
            // killough 03/07/98: support for drawn heights coming from different sector
            case CreateFakeCeilingAndFloor:
            {
                sector_t    *sec = sides[*line->sidenum].sector;

                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    sectors[s].heightsec = sec;

                break;
            }

            // killough 03/16/98: Add support for setting floor lighting independently (e.g. lava)
            case Floor_ChangeBrightnessToThisBrightness:
            {
                sector_t    *sec = sides[*line->sidenum].sector;

                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    sectors[s].floorlightsec = sec;

                break;
            }

            // killough 04/11/98: Add support for setting ceiling lighting independently
            case Ceiling_ChangeBrightnessToThisBrightness:
            {
                sector_t    *sec = sides[*line->sidenum].sector;

                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    sectors[s].ceilinglightsec = sec;

                break;
            }

            // killough 10/98:
            // Support for sky textures being transferred from sidedefs.
            // Allows scrolling and other effects (but if scrolling is
            // used, then the same sector tag needs to be used for the
            // sky sector, the sky-transfer linedef, and the scroll-effect
            // linedef). Still requires user to use F_SKY1 for the floor
            // or ceiling texture, to distinguish floor and ceiling sky.
            case TransferSkyTextureToTaggedSectors:
            case TransferSkyTextureToTaggedSectors_Flipped:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    sectors[s].floorsky = sectors[s].ceilingsky = (i | PL_SKYFLAT);

                break;

            case OffsetTargetFloorTextureByLineDirection:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                {
                    sectors[s].floorxoffset -= line->dx;
                    sectors[s].flooryoffset += line->dy;
                }

                break;

            case OffsetTargetCeilingTextureByLineDirection:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                {
                    sectors[s].ceilingxoffset -= line->dx;
                    sectors[s].ceilingyoffset += line->dy;
                }

                break;

            case OffsetTargetFloorAndCeilingTextureByLineDirection:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                {
                    sectors[s].floorxoffset -= line->dx;
                    sectors[s].flooryoffset += line->dy;
                    sectors[s].ceilingxoffset -= line->dx;
                    sectors[s].ceilingyoffset += line->dy;
                }

                break;

            case RotateTargetFloorTextureByLineAngle:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                        sectors[s].floorrotation -= line->angle;

                break;

            case RotateTargetCeilingTextureByLineAngle:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                        sectors[s].ceilingrotation -= line->angle;

                break;

            case RotateTargetFloorAndCeilingTextureByLineAngle:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                {
                    sectors[s].floorrotation -= line->angle;
                    sectors[s].ceilingrotation -= line->angle;
                }

                break;

            case OffsetThenRotateTargetFloorTextureByLineDirectionAndAngle:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                {
                    sectors[s].floorxoffset -= line->dx;
                    sectors[s].flooryoffset += line->dy;
                    sectors[s].floorrotation -= line->angle;
                }

                break;

            case OffsetThenRotateTargetCeilingTextureByLineDirectionAndAngle:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                {
                    sectors[s].ceilingxoffset -= line->dx;
                    sectors[s].ceilingyoffset += line->dy;
                    sectors[s].ceilingrotation -= line->angle;
                }

                break;

            case OffsetThenRotateTargetFloorAndCeiilngTextureByLineDirectionAndAngle:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                {
                    sectors[s].floorxoffset -= line->dx;
                    sectors[s].flooryoffset += line->dy;
                    sectors[s].ceilingxoffset -= line->dx;
                    sectors[s].ceilingyoffset += line->dy;
                    sectors[s].floorrotation -= line->angle;
                    sectors[s].ceilingrotation -= line->angle;
                }

                break;

            // [KLN] 04/13/25: Support for the ID24 line special 2075: Set the target sector's colormap
            // (Always) uses a new SHORT in side_t, which is loaded via P_LoadSideDefs2
            // Uses the front color map index set by the toptexture since this line cannot be activated
            // by the back (or any other way but automatically)
            case SetTheTargetSectorsColormap:
            {
                const short index = sides[*line->sidenum].frontcolormap;

                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    sectors[s].colormap = index;

                break;
            }
        }
}

// killough 02/28/98:
// This function, with the help of r_plane.c and r_bsp.c, supports generalized
// scrolling floors and walls, with optional mobj-carrying properties, e.g.
// conveyor belts, rivers, etc. A linedef with a special type affects all
// tagged sectors the same way, by creating scrolling and/or object-carrying
// properties. Multiple linedefs may be used on the same sector and are
// cumulative, although the special case of scrolling a floor and carrying
// things on it, requires only one linedef. The linedef's direction determines
// the scrolling direction, and the linedef's length determines the scrolling
// speed. This was designed so that an edge around the sector could be used to
// control the direction of the sector's scrolling, which is usually what is
// desired.
//
// Process the active scrollers.
//
// This is the main scrolling code
// killough 03/07/98
void T_Scroll(scroll_t *scroller)
{
    fixed_t dx = scroller->dx;
    fixed_t dy = scroller->dy;

    // [BH] only allow wall scrollers to update once per tic
    if (scroller->type == sc_side)
    {
        static int  prevaffectee = -1;
        static int  prevtime = -1;

        if (prevaffectee == scroller->affectee && prevtime == gametime)
            return;

        prevaffectee = scroller->affectee;
        prevtime = gametime;
    }

    if (scroller->control != -1)
    {
        // compute scroll amounts based on a sector's height changes
        const fixed_t   height = sectors[scroller->control].floorheight + sectors[scroller->control].ceilingheight;
        const fixed_t   delta = height - scroller->lastheight;

        scroller->lastheight = height;
        dx = FixedMul(dx, delta);
        dy = FixedMul(dy, delta);
    }

    // killough 03/14/98: Add acceleration
    if (scroller->accel)
    {
        scroller->vdx = (dx += scroller->vdx);
        scroller->vdy = (dy += scroller->vdy);
    }

    if (!(dx | dy))                     // no-op if both (x,y) offsets 0
        return;

    switch (scroller->type)
    {
        case sc_side:                   // killough 03/07/98: Scroll wall texture
        {
            side_t  *side = sides + scroller->affectee;

            if (side->oldgametime != gametime)
            {
                side->oldtextureoffset = side->basetextureoffset;
                side->oldrowoffset = side->baserowoffset;
                side->oldgametime = gametime;
            }

            side->basetextureoffset += dx;
            side->baserowoffset += dy;
            side->textureoffset = side->basetextureoffset;
            side->rowoffset = side->baserowoffset;
            break;
        }

        case sc_floor:                  // killough 03/07/98: Scroll floor texture
        {
            sector_t    *sec = sectors + scroller->affectee;

            if (sec->oldflooroffsetgametime != gametime)
            {
                sec->oldfloorxoffset = sec->basefloorxoffset;
                sec->oldflooryoffset = sec->baseflooryoffset;
                sec->oldflooroffsetgametime = gametime;
            }

            sec->basefloorxoffset += dx;
            sec->baseflooryoffset += dy;
            sec->floorxoffset = sec->basefloorxoffset;
            sec->flooryoffset = sec->baseflooryoffset;
            break;
        }

        case sc_ceiling:                // killough 03/07/98: Scroll ceiling texture
        {
            sector_t    *sec = sectors + scroller->affectee;

            if (sec->oldceilingoffsetgametime != gametime)
            {
                sec->oldceilingxoffset = sec->baseceilingxoffset;
                sec->oldceilingyoffset = sec->baseceilingyoffset;
                sec->oldceilingoffsetgametime = gametime;
            }

            sec->baseceilingxoffset += dx;
            sec->baseceilingyoffset += dy;
            sec->ceilingxoffset = sec->baseceilingxoffset;
            sec->ceilingyoffset = sec->baseceilingyoffset;
            break;
        }

        case sc_carry:
        {
            // killough 03/07/98: Carry things on floor
            // killough 03/20/98: Use new sector list which reflects true members
            // killough 03/27/98: Fix carrier bug
            // killough 04/04/98: Underwater, carry things even w/o gravity
            sector_t    *sec = sectors + scroller->affectee;
            fixed_t     height = sec->floorheight;
            fixed_t     waterheight = (sec->heightsec && sec->heightsec->floorheight > height ? sec->heightsec->floorheight : FIXED_MIN);

            // Move objects only if on floor or underwater,
            // non-floating, and clipped.
            for (msecnode_t *node = sec->touching_thinglist; node; node = node->m_snext)
            {
                mobj_t  *thing = node->m_thing;

                if (!(thing->flags & MF_NOCLIP) && (!((thing->flags & MF_NOGRAVITY) || thing->z > height) || thing->z < waterheight))
                {
                    thing->momx += dx;
                    thing->momy += dy;
                    thing->flags2 |= MF2_SCROLLING;
                }
            }

            dx <<= 3;
            dy <<= 3;

            // [BH] scroll any blood splats as well
            for (bloodsplat_t *splat = sec->splatlist; splat; splat = splat->next)
                if (sec != R_PointInSubsector((splat->x += dx), (splat->y += dy))->sector)
                    P_UnsetBloodSplatPosition(splat);

            break;
        }
    }
}

//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// type: the enumerated type of scrolling: floor, ceiling, floor carrier,
//   wall, floor carrier and scroller
//
// (dx,dy): the direction and speed of the scrolling or its acceleration
//
// control: the sector whose heights control this scroller's effect
//   remotely, or -1 if no control sector
//
// affectee: the index of the affected object (sector or sidedef)
//
// accel: true if this is an accelerative effect
//
static void Add_Scroller(int type, fixed_t dx, fixed_t dy, int control, int affectee, bool accel)
{
    scroll_t    *scroller = Z_Calloc(1, sizeof(*scroller), PU_LEVSPEC, NULL);

    scroller->type = type;
    scroller->dx = dx;
    scroller->dy = dy;
    scroller->accel = accel;

    if ((scroller->control = control) != -1)
        scroller->lastheight = sectors[control].floorheight + sectors[control].ceilingheight;

    scroller->affectee = affectee;
    scroller->thinker.function = &T_Scroll;
    scroller->thinker.menu = (type != sc_carry);
    P_AddThinker(&scroller->thinker);
}

// Adds wall scroller. Scroll amount is rotated with respect to wall's
// linedef first, so that scrolling towards the wall in a perpendicular
// direction is translated into vertical motion, while scrolling along
// the wall in a parallel direction is translated into horizontal motion.
//
// killough 05/25/98: cleaned up arithmetic to avoid drift due to roundoff
// killough 10/98: fix scrolling aliasing problems, caused by long linedefs causing overflowing
static void Add_WallScroller(int64_t dx, int64_t dy, const line_t *line, int control, bool accel)
{
    fixed_t x = ABS(line->dx);
    fixed_t y = ABS(line->dy);
    fixed_t d;

    if (y > x)
        SWAP(x, y);

    d = FixedDiv(x, finesine[(tantoangle[FixedDiv(y, x) >> DBITS] + ANG90) >> ANGLETOFINESHIFT]);

    x = (fixed_t)((dy * -(int64_t)line->dy - dx * (int64_t)line->dx) / d);  // killough 10/98:
    y = (fixed_t)((dy * (int64_t)line->dx - dx * (int64_t)line->dy) / d);   // Use int64_t arithmetic
    Add_Scroller(sc_side, x, y, control, *line->sidenum, accel);
}

// Initialize the scrollers
static void P_SpawnScrollers(void)
{
    line_t  *line = lines;

    for (int i = 0; i < numlines; i++, line++)
    {
        fixed_t         dx = line->dx >> SCROLL_SHIFT;                      // direction and speed of scrolling
        fixed_t         dy = line->dy >> SCROLL_SHIFT;
        int             control = -1;                                       // no control sector or acceleration
        bool            accel = false;
        unsigned short  special = line->special;

        // killough 03/07/98: Types 245-249 are same as 250-254 except that the
        // first side's sector's heights cause scrolling when they change, and
        // this linedef controls the direction and speed of the scrolling. The
        // most complicated linedef since donuts, but powerful :)
        //
        // killough 03/15/98: Add acceleration. Types 214-218 are the same but
        // are accelerative.
        if (special >= Scroll_ScrollCeilingWhenSectorChangesHeight
            && special <= Scroll_ScrollWallWhenSectorChangesHeight)         // displacement scrollers
        {
            special += Scroll_ScrollCeilingAccordingToLineVector - Scroll_ScrollCeilingWhenSectorChangesHeight;
            control = sides[*line->sidenum].sector->id;
        }
        else if (special >= Scroll_CeilingAcceleratesWhenSectorHeightChanges
            && special <= Scroll_WallAcceleratesWhenSectorHeightChanges)    // accelerative scrollers
        {
            accel = true;
            special += Scroll_ScrollCeilingAccordingToLineVector - Scroll_CeilingAcceleratesWhenSectorHeightChanges;
            control = sides[*line->sidenum].sector->id;
        }

        switch (special)
        {
            case Scroll_ScrollCeilingAccordingToLineVector:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    Add_Scroller(sc_ceiling, -dx, dy, control, s, accel);

                break;

            case Scroll_ScrollFloorAccordingToLineVector:
            case Scroll_ScrollFloorAndMoveThings:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    Add_Scroller(sc_floor, -dx, dy, control, s, accel);

                if (special != Scroll_ScrollFloorAndMoveThings)
                    break;

            case Scroll_MoveThingsAccordingToLineVector:
                dx = FixedMul(dx, CARRYFACTOR);
                dy = FixedMul(dy, CARRYFACTOR);

                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    Add_Scroller(sc_carry, dx, dy, control, s, accel);

                break;

            // killough 03/01/98: scroll wall according to linedef
            // (same direction and speed as scrolling floors)
            case Scroll_ScrollWallAccordingToLineVector:
                if (!line->tag)
                    Add_WallScroller(dx, dy, line, control, accel);
                else
                    for (int s = -1; (s = P_FindLineFromLineTag(line, s)) >= 0; )
                        if (s != i)
                            Add_WallScroller(dx, dy, lines + s, control, accel);

                break;

            case Scroll_ScrollWallUsingSidedefOffsets:
            {
                int side = lines[i].sidenum[0];

                Add_Scroller(sc_side, -sides[side].textureoffset, sides[side].rowoffset, -1, side, accel);
                break;
            }

            case Scroll_ScrollTextureLeft:
                Add_Scroller(sc_side, FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
                break;

            case Scroll_ScrollTextureRight:
                Add_Scroller(sc_side, -FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
                break;

            // MBF21
            case Scroll_ScrollWallWithSameTagUsingSidedefOffsets:
            case Scroll_ScrollWallWithSameTagUsingSidedefOffsetsWhenSectorChangesHeight:
            case Scroll_ScrollWallWithSameTagUsingSidedefOffsetsAcceleratesWhenSectorChangesHeight:
            {
                int side = lines[i].sidenum[0];

                if (special >= Scroll_ScrollWallWithSameTagUsingSidedefOffsetsWhenSectorChangesHeight)
                    control = sides[*line->sidenum].sector->id;

                if (special == Scroll_ScrollWallWithSameTagUsingSidedefOffsetsAcceleratesWhenSectorChangesHeight)
                    accel = true;

                dx = -sides[side].textureoffset / 8;
                dy = sides[side].rowoffset / 8;

                for (side = -1; (side = P_FindLineFromLineTag(line, side)) >= 0; )
                    if (side != i)
                        Add_Scroller(sc_side, dx, dy, control, lines[side].sidenum[0], accel);

                break;
            }
        }
    }
}

//
// FRICTION EFFECTS
//
// phares 03/12/98: Start of friction effects
//
// As the player moves, friction is applied by decreasing the x and y
// momentum values on each tic. By varying the percentage of decrease,
// we can simulate muddy or icy conditions. In mud, the player slows
// down faster. In ice, the player slows down more slowly.
//
// The amount of friction change is controlled by the length of a linedef
// with type 223. A length < 100 gives you mud. A length > 100 gives you ice.
//
// Also, each sector where these effects are to take place is given a
// new special type _______. Changing the type value at runtime allows
// these effects to be turned on or off.
//
// Sector boundaries present problems. The player should experience these
// friction changes only when his feet are touching the sector floor. At
// sector boundaries where floor height changes, the player can find
// himself still 'in' one sector, but with his feet at the floor level
// of the next sector (steps up or down). To handle this, Thinkers are used
// in icy/muddy sectors. These thinkers examine each object that is touching
// their sectors, looking for players whose feet are at the same level as
// their floors. Players satisfying this condition are given new friction
// values that are applied by the player movement code later.
//
// killough 08/28/98:
//
// Completely redid code, which did not need thinkers, and which put a heavy
// drag on CPU. Friction is now a property of sectors, NOT objects inside
// them. All objects, not just players, are affected by it, if they touch
// the sector's floor. Code simpler and faster, only calling on friction
// calculations when an object needs friction considered, instead of doing
// friction calculations on every sector during every tic.
//
// Although this -might- ruin BOOM demo sync involving friction, it's the only
// way, short of code explosion, to fix the original design bug. Fixing the
// design bug in BOOM's original friction code, while maintaining demo sync
// under every conceivable circumstance, would double or triple code size, and
// would require maintenance of buggy legacy code which is only useful for old
// demos. DOOM demos, which are more important IMO, are not affected by this
// change.
//

//
// Initialize the sectors where friction is increased or decreased
static void P_SpawnFriction(void)
{
    line_t  *line = lines;

    // killough 08/28/98: initialize all sectors to normal friction first
    for (int i = 0; i < numsectors; i++)
    {
        sectors[i].friction = ORIG_FRICTION;
        sectors[i].movefactor = ORIG_FRICTION_FACTOR;
    }

    for (int i = 0; i < numlines; i++, line++)
        if (line->special == FrictionTaggedSector)
        {
            int length = P_ApproxDistance(line->dx, line->dy) >> FRACBITS;
            int friction = BETWEEN(0, (0x1EB8 * length) / 0x80 + 0xD000, FRACUNIT);
            int movefactor;

            // The following check might seem odd. At the time of movement,
            // the move distance is multiplied by 'friction/0x10000', so a
            // higher friction value actually means 'less friction'.
            if (friction > ORIG_FRICTION)   // ice
                movefactor = MAX(32, ((0x10092 - friction) * 0x70) / 0x0158);
            else
                movefactor = MAX(32, ((friction - 0xDB34) * 0x0A) / 0x80);

            for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
            {
                // killough 08/28/98:
                //
                // Instead of spawning thinkers, which are slow and expensive,
                // modify the sector's own friction values. Friction should be
                // a property of sectors, not objects which reside inside them.
                // Original code scanned every object in every friction sector
                // on every tic, adjusting its friction, putting unnecessary
                // drag on CPU. New code adjusts friction of sector only once
                // at level startup, and then uses this friction value.
                sectors[s].friction = friction;
                sectors[s].movefactor = movefactor;
            }
        }
}

//
// PUSH/PULL EFFECT
//
// phares 03/20/98: Start of push/pull effects
//
// This is where push/pull effects are applied to objects in the sectors.
//
// There are four kinds of push effects
//
// 1) Pushing Away
//
//    Pushes you away from a point source defined by the location of an
//    MT_PUSH Thing. The force decreases linearly with distance from the
//    source. This force crosses sector boundaries and is felt w/in a circle
//    whose center is at the MT_PUSH. The force is felt only if the point
//    MT_PUSH can see the target object.
//
// 2) Pulling toward
//
//    Same as Pushing Away except you're pulled toward an MT_PULL point
//    source. This force crosses sector boundaries and is felt w/in a circle
//    whose center is at the MT_PULL. The force is felt only if the point
//    MT_PULL can see the target object.
//
// 3) Wind
//
//    Pushes you in a constant direction. Full force above ground, half
//    force on the ground, nothing if you're below it (water).
//
// 4) Current
//
//    Pushes you in a constant direction. No force above ground, full
//    force if on the ground or below it (water).
//
// The magnitude of the force is controlled by the length of a controlling
// linedef. The force vector for types 3 and 4 is determined by the angle
// of the linedef, and is constant.
//
// For each sector where these effects occur, the sector special type has
// to have the PUSH_MASK bit set. If this bit is turned off by a switch
// at run-time, the effect will not occur. The controlling sector for
// types 1 and 2 is the sector containing the MT_PUSH/MT_PULL thing.

#define PUSH_FACTOR 7

// Add a push thinker to the thinker list
static void Add_Pusher(int type, int x_mag, int y_mag, mobj_t *source, int affectee)
{
    pusher_t    *pusher = Z_Calloc(1, sizeof(*pusher), PU_LEVSPEC, NULL);

    pusher->source = source;
    pusher->type = type;
    pusher->x_mag = x_mag >> FRACBITS;
    pusher->y_mag = y_mag >> FRACBITS;
    pusher->magnitude = P_ApproxDistance(pusher->x_mag, pusher->y_mag);

    // point source exist?
    if (source)
    {
        pusher->radius = pusher->magnitude << (FRACBITS + 1);   // where force goes to zero
        pusher->x = pusher->source->x;
        pusher->y = pusher->source->y;
    }

    pusher->affectee = affectee;
    pusher->thinker.function = &T_Pusher;
    pusher->thinker.menu = false;
    P_AddThinker(&pusher->thinker);
}

//
// PIT_PushThing determines the angle and magnitude of the effect.
// The object's x and y momentum values are changed.
//
// tmpusher belongs to the point source (MT_PUSH/MT_PULL).
//
// killough 10/98: allow to affect things besides players

static pusher_t *tmpusher;  // pusher structure for blockmap searches

static bool PIT_PushThing(mobj_t *thing)
{
    if ((sentient(thing) || (thing->flags & MF_SHOOTABLE)) && !(thing->flags & MF_NOCLIP))
    {
        fixed_t sx = tmpusher->x;
        fixed_t sy = tmpusher->y;
        fixed_t speed = (tmpusher->magnitude - ((P_ApproxDistance(thing->x - sx, thing->y - sy) >> FRACBITS) >> 1))
                    << (FRACBITS - PUSH_FACTOR - 1);

        // killough 10/98: make magnitude decrease with square
        // of distance, making it more in line with real nature,
        // so long as it's still in range with original formula.
        //
        // Removes angular distortion, and makes effort required
        // to stay close to source, grow increasingly hard as you
        // get closer, as expected. Still, it doesn't consider z :(
        if (speed > 0)
        {
            int x = (thing->x - sx) >> FRACBITS;
            int y = (thing->y - sy) >> FRACBITS;

            speed = (fixed_t)(((int64_t)tmpusher->magnitude << 23) / ((int64_t)x * x + (int64_t)y * y + 1));
        }

        // If speed <= 0, you're outside the effective radius. You also have
        // to be able to see the push/pull source point.
        if (speed > 0 && P_CheckSight(thing, tmpusher->source))
        {
            angle_t pushangle = R_PointToAngle2(thing->x, thing->y, sx, sy);

            if (tmpusher->source->type == MT_PUSH)
                pushangle += ANG180;    // away

            pushangle >>= ANGLETOFINESHIFT;
            thing->momx += FixedMul(speed, finecosine[pushangle]);
            thing->momy += FixedMul(speed, finesine[pushangle]);
            thing->flags2 |= MF2_SCROLLING;
        }
    }

    return true;
}

//
// T_Pusher
// Looks for all objects that are inside the radius of the effect.
//
void T_Pusher(pusher_t *pusher)
{
    sector_t    *sec = sectors + pusher->affectee;
    int         xspeed, yspeed;
    int         ht = 0;

    // Be sure the special sector type is still turned on. If so, proceed.
    // Else, bail out; the sector type has been changed on us.
    if (!(sec->special & PUSH_MASK))
        return;

    // For constant pushers (wind/current) there are 3 situations:
    //
    // 1) Affected Thing is above the floor.
    //
    //    Apply the full force if wind, no force if current.
    //
    // 2) Affected Thing is on the ground.
    //
    //    Apply half force if wind, full force if current.
    //
    // 3) Affected Thing is below the ground (underwater effect).
    //
    //    Apply no force if wind, full force if current.
    if (pusher->type == p_push)
    {
        // Seek out all pushable things within the force radius of this
        // point pusher. Crosses sectors, so use blockmap.
        int xl;
        int xh;
        int yl;
        int yh;
        int radius;

        tmpusher = pusher;                              // MT_PUSH/MT_PULL point source
        radius = pusher->radius;                        // where force goes to zero
        tmbbox[BOXTOP] = pusher->y + radius;
        tmbbox[BOXBOTTOM] = pusher->y - radius;
        tmbbox[BOXRIGHT] = pusher->x + radius;
        tmbbox[BOXLEFT] = pusher->x - radius;

        xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS);
        xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS);
        yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS);
        yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS);

        for (int bx = xl; bx <= xh; bx++)
            for (int by = yl; by <= yh; by++)
                P_BlockThingsIterator(bx, by, &PIT_PushThing, true);

        return;
    }

    // constant pushers p_wind and p_current
    if (sec->heightsec)                                 // special water sector?
        ht = sec->heightsec->floorheight;

    // things touching this sector
    for (msecnode_t *node = sec->touching_thinglist; node; node = node->m_snext)
    {
        mobj_t  *thing = node->m_thing;

        if (!thing->player || (thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
            continue;

        if (pusher->type == p_wind)
        {
            if (!sec->heightsec)                        // NOT special water sector
            {
                if (thing->z > thing->floorz)           // above ground
                {
                    xspeed = pusher->x_mag;             // full force
                    yspeed = pusher->y_mag;
                }
                else                                    // on ground
                {
                    xspeed = pusher->x_mag >> 1;        // half force
                    yspeed = pusher->y_mag >> 1;
                }
            }
            else                                        // special water sector
            {
                if (thing->z > ht)                      // above ground
                {
                    xspeed = pusher->x_mag;             // full force
                    yspeed = pusher->y_mag;
                }
                else
                {
                    if (thing->player->viewz < ht)      // underwater
                        xspeed = yspeed = 0;            // no force
                    else                                // wading in water
                    {
                        xspeed = pusher->x_mag >> 1;    // half force
                        yspeed = pusher->y_mag >> 1;
                    }
                }
            }
        }
        else                                            // p_current
        {
            if (!sec->heightsec)                        // NOT special water sector
            {
                if (thing->z > sec->floorheight)        // above ground
                {
                    xspeed = 0;                         // no force
                    yspeed = 0;
                }
                else                                    // on ground
                {
                    xspeed = pusher->x_mag;             // full force
                    yspeed = pusher->y_mag;
                }
            }
            else                                        // special water sector
            {
                if (thing->z > ht)                      // above ground
                {
                    xspeed = 0;                         // no force
                    yspeed = 0;
                }
                else                                    // underwater
                {
                    xspeed = pusher->x_mag;             // full force
                    yspeed = pusher->y_mag;
                }
            }
        }

        thing->momx += (xspeed << (FRACBITS - PUSH_FACTOR));
        thing->momy += (yspeed << (FRACBITS - PUSH_FACTOR));
        thing->flags2 |= MF2_SCROLLING;
    }
}

// P_GetPushThing() returns a pointer to an MT_PUSH or MT_PULL thing, NULL otherwise.
mobj_t *P_GetPushThing(int s)
{
    for (mobj_t *thing = sectors[s].thinglist; thing; thing = thing->snext)
        if (thing->type == MT_PUSH || thing->type == MT_PULL)
            return thing;

    return NULL;
}

//
// Initialize the sectors where pushers are present
//
static void P_SpawnPushers(void)
{
    line_t  *line = lines;
    mobj_t  *thing;

    for (int i = 0; i < numlines; i++, line++)
        switch (line->special)
        {
            case WindAccordingToLineVector:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    Add_Pusher(p_wind, line->dx, line->dy, NULL, s);

                break;

            case CurrentAccordingToLineVector:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    Add_Pusher(p_current, line->dx, line->dy, NULL, s);

                break;

            case WindCurrentByPushPullThingInSector:
                for (int s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0; )
                    if ((thing = P_GetPushThing(s)))    // No MT_P* means no effect
                        Add_Pusher(p_push, line->dx, line->dy, thing, s);

                break;
        }
}

bool P_ProcessNoTagLines(const line_t *line, sector_t **sec, int *secnum)
{
    zerotag_manual = false;

    if (!line->tag)
    {
        if (!(*sec = line->backsector))
            return true;

        *secnum = (*sec)->id;
        zerotag_manual = true;

        return true;
    }

    return false;
}
