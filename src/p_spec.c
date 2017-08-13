/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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
#include "d_deh.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "r_sky.h"
#include "s_sound.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct anim_s
{
    dboolean    istexture;
    int         picnum;
    int         basepic;
    int         numpics;
    int         speed;
} anim_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

//
// source animation definition
//
typedef struct animdef_s
{
    signed char istexture;              // if false, it is a flat
    char        endname[9];
    char        startname[9];
    int         speed;
} PACKEDATTR animdef_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

#define MAXANIMS    32

unsigned int    stat_secretsrevealed;

dboolean        r_liquid_bob = r_liquid_bob_default;

fixed_t         animatedliquiddiff;
fixed_t         animatedliquidxdir;
fixed_t         animatedliquidydir;
fixed_t         animatedliquidxoffs;
fixed_t         animatedliquidyoffs;

fixed_t animatedliquiddiffs[64] =
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

static anim_t   *lastanim;
static anim_t   *anims;                 // new structure w/o limits -- killough
static size_t   maxanims;

// killough 3/7/98: Initialize generalized scrolling
static void P_SpawnScrollers(void);
static void P_SpawnFriction(void);      // phares 3/16/98
static void P_SpawnPushers(void);       // phares 3/20/98

extern char     *playername;
extern int      numflats;
extern dboolean canmodify;

dboolean        *isliquid;
dboolean        *isteleport;

//
// P_InitPicAnims
//
void P_InitPicAnims(void)
{
    int size = (numflats + 1) * sizeof(dboolean);

    isliquid = Z_Malloc(size, PU_STATIC, NULL);
    isteleport = Z_Calloc(1, size, PU_STATIC, NULL);

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
}

//
// P_GetLiquids
//
void P_SetLiquids(void)
{
    int         i;
    int         lump = W_GetNumForName("ANIMATED");
    animdef_t   *animdefs = W_CacheLumpNum(lump);

    for (i = 0; i < numflats; i++)
        isliquid[i] = false;

    // Init animation
    lastanim = anims;

    for (i = 0; animdefs[i].istexture != -1; i++)
    {
        // 1/11/98 killough -- removed limit by array-doubling
        if (lastanim >= anims + maxanims)
        {
            size_t  newmax = (maxanims ? maxanims * 2 : MAXANIMS);

            anims = Z_Realloc(anims, newmax * sizeof(*anims));
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
            int j;

            if (R_CheckFlatNumForName(animdefs[i].startname) == -1)
                continue;

            lastanim->picnum = R_FlatNumForName(animdefs[i].endname);
            lastanim->basepic = R_FlatNumForName(animdefs[i].startname);

            lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;
            lastanim->istexture = false;

            for (j = 0; j < lastanim->numpics; j++)
                isliquid[lastanim->basepic + j] = true;
        }

        if (lastanim->numpics < 2)
            I_Error("P_InitPicAnims: bad cycle from %s to %s", animdefs[i].startname, animdefs[i].endname);

        lastanim->speed = LONG(animdefs[i].speed);
        lastanim++;
    }

    W_UnlockLumpNum(lump);

    // [BH] parse DRCOMPAT lump to find animated textures that are not liquid in current wad
    SC_Open("DRCOMPAT");

    while (SC_GetString())
    {
        if (M_StringCompare(sc_String, "NOLIQUID"))
        {
            int lump;

            SC_MustGetString();
            lump = R_CheckFlatNumForName(sc_String);
            SC_MustGetString();

            if (lump >= 0 && M_StringCompare(leafname(lumpinfo[firstflat + lump]->wadfile->path), sc_String))
                isliquid[lump] = false;
        }
    }

    numliquid = 0;

    for (i = 0; i < numsectors; i++)
        if (isliquid[sectors[i].floorpic])
        {
            sectors[i].isliquid = true;
            numliquid++;
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
dboolean twoSided(int sector, int line)
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
    // if (!(line->flags & ML_TWOSIDED))
    //     return NULL;
    return (line->frontsector == sec ? (line->backsector != sec ? line->backsector : NULL) :
        line->frontsector);
}

//
// P_IsSelfReferencingSector()
//
dboolean P_IsSelfReferencingSector(sector_t *sec)
{
    int i;

    for (i = 0; i < sec->linecount; i++)
    {
        line_t  *line = sec->lines[i];

        if (line->backsector && line->frontsector == line->backsector && (line->flags & ML_TWOSIDED)
            && (line->flags & ML_DONTDRAW))
            return true;
    }

    return false;
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
    fixed_t     floor = -32000 * FRACUNIT;

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
// P_FindNextLowestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the largest ceiling height in a surrounding sector smaller than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this

fixed_t P_FindNextLowestCeiling(sector_t *sec, int currentheight)
{
    int         i;
    sector_t    *other;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight < currentheight)
        {
            int height = other->ceilingheight;

            while (++i < sec->linecount)
                if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight > height
                    && other->ceilingheight < currentheight)
                    height = other->ceilingheight;

            return height;
        }

    return currentheight;
}

//
// P_FindNextHighestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the smallest ceiling height in a surrounding sector larger than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this

fixed_t P_FindNextHighestCeiling(sector_t *sec, int currentheight)
{
    int         i;
    sector_t    *other;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight > currentheight)
        {
            int height = other->ceilingheight;

            while (++i < sec->linecount)
                if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight < height
                    && other->ceilingheight > currentheight)
                    height = other->ceilingheight;

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
    fixed_t     height = 32000 * FRACUNIT;

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
    fixed_t     height = -32000 * FRACUNIT;

    for (i = 0; i < sec->linecount; i++)
        if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight > height)
            height = other->ceilingheight;

    return height;
}

//
// P_FindShortestTextureAround()
//
// Passed a sector number, returns the shortest lower texture on a
// linedef bounding the sector.
//
// Note: If no lower texture exists 32000*FRACUNIT is returned.
//       but if compatibility then MAXINT is returned
//
// jff 02/03/98 Add routine to find shortest lower texture
//
// killough 11/98: reformatted
fixed_t P_FindShortestTextureAround(int secnum)
{
    const sector_t  *sec = &sectors[secnum];
    int             i;
    int             minsize = 32000 * FRACUNIT;

    for (i = 0; i < sec->linecount; i++)
        if (twoSided(secnum, i))
        {
            const side_t    *side;

            if ((side = getSide(secnum, i, 0))->bottomtexture > 0
                && textureheight[side->bottomtexture] < minsize)
                minsize = textureheight[side->bottomtexture];

            if ((side = getSide(secnum, i, 1))->bottomtexture > 0
                && textureheight[side->bottomtexture] < minsize)
                minsize = textureheight[side->bottomtexture];
        }

    return minsize;
}

//
// P_FindShortestUpperAround()
//
// Passed a sector number, returns the shortest upper texture on a
// linedef bounding the sector.
//
// Note: If no upper texture exists 32000*FRACUNIT is returned.
//       but if compatibility then MAXINT is returned
//
// jff 03/20/98 Add routine to find shortest upper texture
//
// killough 11/98: reformatted
fixed_t P_FindShortestUpperAround(int secnum)
{
    const sector_t  *sec = &sectors[secnum];
    int             i;
    int             minsize = 32000 * FRACUNIT;

    for (i = 0; i < sec->linecount; i++)
        if (twoSided(secnum, i))
        {
            const side_t    *side;

            if ((side = getSide(secnum, i, 0))->toptexture > 0 && textureheight[side->toptexture] < minsize)
                minsize = textureheight[side->toptexture];

            if ((side = getSide(secnum, i, 1))->toptexture > 0 && textureheight[side->toptexture] < minsize)
                minsize = textureheight[side->toptexture];
        }

    return minsize;
}

//
// P_FindModelFloorSector()
//
// Passed a floor height and a sector number, return a pointer to a
// a sector with that floor height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
// jff 02/03/98 Add routine to find numeric model floor
//  around a sector specified by sector number
// jff 3/14/98 change first parameter to plain height to allow call
//  from routine not using floormove_t
//
// killough 11/98: reformatted
sector_t *P_FindModelFloorSector(fixed_t floordestheight, int secnum)
{
    sector_t    *sec = &sectors[secnum];
    int         i;
    int         linecount = sec->linecount;

    for (i = 0; i < linecount; i++)
        if (twoSided(secnum, i) && (sec = getSector(secnum, i, getSide(secnum, i, 0)->sector
            - sectors == secnum))->floorheight == floordestheight)
            return sec;

    return NULL;
}

//
// P_FindModelCeilingSector()
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
// jff 3/14/98 change first parameter to plain height to allow call
//  from routine not using ceiling_t
//
// killough 11/98: reformatted
sector_t *P_FindModelCeilingSector(fixed_t ceildestheight, int secnum)
{
    sector_t    *sec = &sectors[secnum];
    int         i;
    int         linecount = sec->linecount;

    for (i = 0; i < linecount; i++)
        if (twoSided(secnum, i) && (sec = getSector(secnum, i, getSide(secnum, i, 0)->sector
            - sectors == secnum))->ceilingheight == ceildestheight)
            return sec;

    return NULL;
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
        int j = (unsigned int)sectors[i].tag % (unsigned int)numsectors;    // Hash func

        sectors[i].nexttag = sectors[j].firsttag;     // Prepend sector to chain
        sectors[j].firsttag = i;
    }

    // killough 4/17/98: same thing, only for linedefs
    for (i = numlines; --i >= 0;)       // Initially make all slots empty.
        lines[i].firsttag = -1;

    for (i = numlines; --i >= 0;)       // Proceed from last to first linedef
    {                                   // so that lower linedefs appear first
        int j = (unsigned int)lines[i].tag % (unsigned int)numlines;        // Hash func

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
// P_CanUnlockGenDoor()
//
// Passed a generalized locked door linedef and a player, returns whether
// the player has the keys necessary to unlock that door.
//
// Note: The linedef passed MUST be a generalized locked door type
//       or results are undefined.
//
// jff 02/05/98 routine added to test for unlockability of
//  generalized locked doors
//
// killough 11/98: reformatted
dboolean P_CanUnlockGenDoor(line_t *line, player_t *player)
{
    static char buffer[1024];

    // does this line special distinguish between skulls and keys?
    int         skulliscard = (line->special & LockedNKeys) >> LockedNKeysShift;

    // determine for each case of lock type if player's keys are adequate
    switch ((line->special & LockedKey) >> LockedKeyShift)
    {
        case AnyKey:
            if (player->cards[it_redcard] <= 0 && player->cards[it_redskull] <= 0
                && player->cards[it_bluecard] <= 0 && player->cards[it_blueskull] <= 0
                && player->cards[it_yellowcard] <= 0 && player->cards[it_yellowskull] <= 0)
            {
                M_snprintf(buffer, sizeof(buffer), s_PD_ANY, playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);
                return false;
            }

            break;

        case RCard:
            if (player->cards[it_redcard] <= 0 && (!skulliscard || player->cards[it_redskull] <= 0))
            {
                if (!player->neededcardflash || player->neededcard != it_redcard)
                {
                    player->neededcard = it_redcard;
                    player->neededcardflash = NEEDEDCARDFLASH;
                }

                M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_REDK : s_PD_REDC), playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);
                return false;
            }

            break;

        case BCard:
            if (player->cards[it_bluecard] <= 0 && (!skulliscard || player->cards[it_blueskull] <= 0))
            {
                if (!player->neededcardflash || player->neededcard != it_bluecard)
                {
                    player->neededcard = it_bluecard;
                    player->neededcardflash = NEEDEDCARDFLASH;
                }

                M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_BLUEK : s_PD_BLUEC), playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);
                return false;
            }

            break;

        case YCard:
            if (player->cards[it_yellowcard] <= 0 && (!skulliscard || player->cards[it_yellowskull] <= 0))
            {
                if (!player->neededcardflash || player->neededcard != it_yellowcard)
                {
                    player->neededcard = it_yellowcard;
                    player->neededcardflash = NEEDEDCARDFLASH;
                }

                M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_YELLOWK : s_PD_YELLOWC), playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);

                return false;
            }
            break;

        case RSkull:
            if (player->cards[it_redskull] <= 0 && (!skulliscard || player->cards[it_redcard] <= 0))
            {
                if (!player->neededcardflash || player->neededcard != it_redskull)
                {
                    player->neededcard = it_redskull;
                    player->neededcardflash = NEEDEDCARDFLASH;
                }

                M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_REDK : s_PD_REDS), playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);
                return false;
            }
            break;

        case BSkull:
            if (player->cards[it_blueskull] <= 0 && (!skulliscard || player->cards[it_bluecard] <= 0))
            {
                if (!player->neededcardflash || player->neededcard != it_blueskull)
                {
                    player->neededcard = it_blueskull;
                    player->neededcardflash = NEEDEDCARDFLASH;
                }

                M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_BLUEK : s_PD_BLUES), playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);
                return false;
            }

            break;

        case YSkull:
            if (player->cards[it_yellowskull] <= 0 && (!skulliscard || player->cards[it_yellowcard] <= 0))
            {
                if (!player->neededcardflash || player->neededcard != it_yellowskull)
                {
                    player->neededcard = it_yellowskull;
                    player->neededcardflash = NEEDEDCARDFLASH;
                }

                M_snprintf(buffer, sizeof(buffer), (skulliscard ? s_PD_YELLOWK : s_PD_YELLOWS), playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);
                return false;
            }

            break;

        case AllKeys:
            if (!skulliscard && (player->cards[it_redcard] <= 0 || player->cards[it_redskull] <= 0
                || player->cards[it_bluecard] <= 0 || player->cards[it_blueskull] <= 0
                || player->cards[it_yellowcard] <= 0 || player->cards[it_yellowskull] <= 0))
            {
                M_snprintf(buffer, sizeof(buffer), s_PD_ALL6, playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);
                return false;
            }

            if (skulliscard && ((player->cards[it_redcard] <= 0 && player->cards[it_redskull] <= 0)
                || (player->cards[it_bluecard] <= 0 && player->cards[it_blueskull] <= 0)
                || (player->cards[it_yellowcard] <= 0 && player->cards[it_yellowskull] <= 0)))
            {
                M_snprintf(buffer, sizeof(buffer), s_PD_ALL3, playername,
                    (M_StringCompare(playername, playername_default) ? "" : "s"));
                HU_PlayerMessage(buffer, false);
                S_StartSound(player->mo, sfx_noway);
                return false;
            }

            break;
    }

    return true;
}

//
// P_SectorActive()
//
// Passed a linedef special class (floor, ceiling, lighting) and a sector
// returns whether the sector is already busy with a linedef special of the
// same class. If old demo compatibility true, all linedef special classes
// are the same.
//
// jff 2/23/98 added to prevent old demos from
//  succeeding in starting multiple specials on one sector
//
// killough 11/98: reformatted
dboolean P_SectorActive(special_e t, sector_t *sec)
{
    return (t == floor_special ? !!sec->floordata :     // return whether
        (t == ceiling_special ? !!sec->ceilingdata :     // thinker of same
        (t == lighting_special ? !!sec->lightingdata :   // type is active
        true)));        // don't know which special, must be active, shouldn't be here
}

//
// P_SectorHasLightSpecial()
//
// [BH] Returns true if sector has a light special
dboolean P_SectorHasLightSpecial(sector_t *sec)
{
    short   special = sec->special;

    return (special && special != Secret && special != Door_CloseStay_After30sec
        && special != Door_OpenClose_OpensAfter5Min);
}

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
dboolean P_CheckTag(line_t *line)
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
            return true;        // zero tag allowed
    }

    return false;               // zero tag not allowed
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
void P_CrossSpecialLine(line_t *line, int side, mobj_t *thing)
{
    // Triggers that other things can activate
    if (!thing->player)
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
            case MT_TRACER:
            case MT_FATSHOT:
            case MT_SPAWNSHOT:
            case MT_ARACHPLAZ:
                return;
                break;

            default:
                break;
        }
    }

    // jff 02/04/98 add check here for generalized linedef types
    {
        // pointer to line function is NULL by default, set non-null if
        // line special is walkover generalized linedef type
        dboolean (*linefunc)(line_t *) = NULL;

        // check each range of generalized linedefs
        if ((unsigned int)line->special >= GenFloorBase)
        {
            if (!thing->player)
                if ((line->special & FloorChange) || !(line->special & FloorModel))
                    return;             // FloorModel is "Allow Monsters" if FloorChange is 0

            linefunc = EV_DoGenFloor;
        }
        else if ((unsigned int)line->special >= GenCeilingBase)
        {
            if (!thing->player)
                if ((line->special & CeilingChange) || !(line->special & CeilingModel))
                    return;             // CeilingModel is "Allow Monsters" if CeilingChange is 0

            linefunc = EV_DoGenCeiling;
        }
        else if ((unsigned int)line->special >= GenDoorBase)
        {
            if (!thing->player)
            {
                if (!(line->special & DoorMonster))
                    return;                     // monsters disallowed from this door

                if (line->flags & ML_SECRET)    // they can't open secret doors either
                    return;
            }

            linefunc = EV_DoGenDoor;
        }
        else if ((unsigned int)line->special >= GenLockedBase)
        {
            if (!thing->player)
                return;                 // monsters disallowed from unlocking doors

            if ((line->special & TriggerType) == WalkOnce || (line->special & TriggerType) == WalkMany)
            {
                // jff 4/1/98 check for being a walk type before reporting door type
                if (!P_CanUnlockGenDoor(line, thing->player))
                    return;
            }
            else
                return;

            linefunc = EV_DoGenLockedDoor;
        }
        else if ((unsigned int)line->special >= GenLiftBase)
        {
            if (!thing->player)
                if (!(line->special & LiftMonster))
                    return;             // monsters disallowed

            linefunc = EV_DoGenLift;
        }
        else if ((unsigned int)line->special >= GenStairsBase)
        {
            if (!thing->player)
                if (!(line->special & StairMonster))
                    return;             // monsters disallowed

            linefunc = EV_DoGenStairs;
        }
        else if ((unsigned int)line->special >= GenCrusherBase)
        {
            if (!thing->player)
                if (!(line->special & CrusherMonster))
                    return;             // monsters disallowed

            linefunc = EV_DoGenCrusher;
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

    if (!thing->player)
    {
        dboolean    okay = false;

        switch (line->special)
        {
            case W1_Door_OpenWaitClose:
            case W1_Lift_LowerWaitRaise:
            case W1_Teleport:
            case WR_Lift_LowerWaitRaise:
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
                okay = true;
                break;
        }

        if (!okay)
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
                    line_t  junk;

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
            if (EV_DoDoor(line, doorBlazeRaise))
                line->special = 0;

            break;

        case W1_Door_OpenStay_Fast:
            if (EV_DoDoor(line, doorBlazeOpen))
                line->special = 0;

            break;

        case W1_Door_CloseStay_Fast:
            if (EV_DoDoor(line, doorBlazeClose))
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

        case W1_Teleport_AlsoMonsters_Silent_SameAngle:
            if (EV_SilentTeleport(line, side, thing))
                line->special = 0;

            break;

        case W1_ChangeTextureAndEffect:
            if (EV_DoChange(line, trigChangeOnly))
                line->special = 0;

            break;

        case W1_ChangeTextureAndEffectToNearest:
            if (EV_DoChange(line, numChangeOnly))
                line->special = 0;

            break;

        case W1_Floor_LowerToNearestFloor:
            if (EV_DoFloor(line, lowerFloorToNearest))
                line->special = 0;

            break;

        case W1_Lift_RaiseToNextHighestFloor_Fast:
            if (EV_DoElevator(line, elevateUp))
                line->special = 0;

            break;

        case W1_Lift_LowerToNextLowestFloor_Fast:
            if (EV_DoElevator(line, elevateDown))
                line->special = 0;

            break;

        case W1_Lift_MoveToSameFloorHeight_Fast:
            if (EV_DoElevator(line, elevateCurrent))
                line->special = 0;

            break;

        case W1_TeleportToLineWithSameTag_Silent_SameAngle:
            if (EV_SilentLineTeleport(line, side, thing, false))
                line->special = 0;

            break;

        case W1_TeleportToLineWithSameTag_Silent_ReversedAngle:
            if (EV_SilentLineTeleport(line, side, thing, true))
                line->special = 0;

            break;

        case W1_TeleportToLineWithSameTag_MonstersOnly_Silent_ReversedAngle:
            if (!thing->player && EV_SilentLineTeleport(line, side, thing, true))
                line->special = 0;

            break;

        case W1_TeleportToLineWithSameTag_MonstersOnly_Silent:
            if (!thing->player && EV_SilentLineTeleport(line, side, thing, false))
                line->special = 0;

            break;

        case W1_Teleport_MonstersOnly_Silent:
            if (!thing->player && EV_SilentTeleport(line, side, thing))
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

        case WR_Teleport_AlsoMonsters_Silent_SameAngle:
            EV_SilentTeleport(line, side, thing);
            break;

        case WR_Lift_RaiseToCeiling_Instantly:
            EV_DoPlat(line, toggleUpDn, 0);
            break;

        case WR_ChangeTextureAndEffect:
            EV_DoChange(line, trigChangeOnly);
            break;

        case WR_ChangeTextureAndEffectToNearest:
            EV_DoChange(line, numChangeOnly);
            break;

        case WR_Floor_LowerToNearestFloor:
            EV_DoFloor(line, lowerFloorToNearest);
            break;

        case WR_Lift_RaiseToNextHighestFloor_Fast:
            EV_DoElevator(line, elevateUp);
            break;

        case WR_Lift_LowerToNextLowestFloor_Fast:
            EV_DoElevator(line, elevateDown);
            break;

        case WR_Lift_MoveToSameFloorHeight_Fast:
            EV_DoElevator(line, elevateCurrent);
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
    }
}

//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void P_ShootSpecialLine(mobj_t *thing, line_t *line)
{
    // jff 02/04/98 add check here for generalized linedef
    // pointer to line function is NULL by default, set non-null if
    // line special is gun triggered generalized linedef type
    dboolean    (*linefunc)(line_t *line) = NULL;

    // check each range of generalized linedefs
    if ((unsigned int)line->special >= GenFloorBase)
    {
        if (!thing->player)
            if ((line->special & FloorChange) || !(line->special & FloorModel))
                return;             // FloorModel is "Allow Monsters" if FloorChange is 0

        linefunc = EV_DoGenFloor;
    }
    else if ((unsigned int)line->special >= GenCeilingBase)
    {
        if (!thing->player)
            if ((line->special & CeilingChange) || !(line->special & CeilingModel))
                return;             // CeilingModel is "Allow Monsters" if CeilingChange is 0

        linefunc = EV_DoGenCeiling;
    }
    else if ((unsigned int)line->special >= GenDoorBase)
    {
        if (!thing->player)
        {
            if (!(line->special & DoorMonster))
                return;                     // monsters disallowed from this door

            if (line->flags & ML_SECRET)    // they can't open secret doors either
                return;
        }

        linefunc = EV_DoGenDoor;
    }
    else if ((unsigned int)line->special >= GenLockedBase)
    {
        if (!thing->player)
            return;                 // monsters disallowed from unlocking doors

        if ((line->special & TriggerType) == GunOnce || (line->special & TriggerType) == GunMany)
        {
            // jff 4/1/98 check for being a gun type before reporting door type
            if (!P_CanUnlockGenDoor(line, thing->player))
                return;
        }
        else
            return;

        linefunc = EV_DoGenLockedDoor;
    }
    else if ((unsigned int)line->special >= GenLiftBase)
    {
        if (!thing->player)
            if (!(line->special & LiftMonster))
                return;             // monsters disallowed

        linefunc = EV_DoGenLift;
    }
    else if ((unsigned int)line->special >= GenStairsBase)
    {
        if (!thing->player)
            if (!(line->special & StairMonster))
                return;             // monsters disallowed

        linefunc = EV_DoGenStairs;
    }
    else if ((unsigned int)line->special >= GenCrusherBase)
    {
        if (!thing->player)
            if (!(line->special & CrusherMonster))
                return;             // monsters disallowed

        linefunc = EV_DoGenCrusher;
    }

    if (linefunc)
        switch ((line->special & TriggerType) >> TriggerTypeShift)
        {
            case GunOnce:
                if (linefunc(line))
                    P_ChangeSwitchTexture(line, false);

                return;

            case GunMany:
                if (linefunc(line))
                    P_ChangeSwitchTexture(line, true);

                return;

            default:            // if not a gun type, do nothing here
                return;
        }

    // Impacts that other things can activate.
    if (!thing->player && line->special != GR_Door_OpenStay)
        return;

    if (!P_CheckTag(line))      // jff 2/27/98 disallow zero tag on some types
        return;

    switch (line->special)
    {
        case G1_Floor_RaiseToLowestCeiling:
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, false);

            break;

        case GR_Door_OpenStay:
            if (EV_DoDoor(line, doorOpen))
            {
                P_ChangeSwitchTexture(line, true);

                if (canmodify && gamemission == doom2 && gamemap == 18)
                    line->special = 0;
            }
            break;

        case G1_Floor_RaiseToNextHighestFloor_ChangesTexture:
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, false);

            break;

        case G1_ExitLevel:
            P_ChangeSwitchTexture(line, false);
            G_ExitLevel();
            break;

        case G1_ExitLevel_GoesToSecretLevel:
            P_ChangeSwitchTexture(line, false);
            G_SecretExitLevel();
            break;
    }
}

static void P_SecretFound(player_t *player)
{
    player->secretcount++;
    stat_secretsrevealed = SafeAdd(stat_secretsrevealed, 1);

    if (DSSECRET)
    {
        static char buffer[1024];

        S_StartSound(NULL, sfx_secret);
        M_snprintf(buffer, sizeof(buffer), s_SECRET, playername);
        HU_PlayerMessage(buffer, false);
    }
}

//
// P_PlayerInSpecialSector
// Called every tic frame
//  that the player origin is in a special sector
//
void P_PlayerInSpecialSector(player_t *player)
{
    sector_t    *sector = player->mo->subsector->sector;

    // jff add if to handle old vs generalized types
    if (sector->special < 32) // regular sector specials
    {
        int i;

        switch (sector->special)
        {
            case DamageNegative5Or10PercentHealth:
                if (!player->powers[pw_ironfeet])
                    if (!(leveltime & 0x1F))
                        P_DamageMobj(player->mo, NULL, NULL, 10, true);

                break;

            case DamageNegative2Or5PercentHealth:
                if (!player->powers[pw_ironfeet])
                    if (!(leveltime & 0x1F))
                        P_DamageMobj(player->mo, NULL, NULL, 5, true);

                break;

            case DamageNegative10Or20PercentHealth:
            case DamageNegative10Or20PercentHealthAndLightBlinks_2Hz:
                if (!player->powers[pw_ironfeet] || M_Random() < 5)
                    if (!(leveltime & 0x1F))
                        P_DamageMobj(player->mo, NULL, NULL, 20, true);

                break;

            case Secret:
                P_SecretFound(player);
                sector->special = 0;

                for (i = 0; i < sector->linecount; i++)
                    sector->lines[i]->flags &= ~ML_SECRET;

                break;

            case DamageNegative10Or20PercentHealthAndEndLevel:
                // for E1M8 finale
                player->cheats &= ~CF_BUDDHA;
                player->cheats &= ~CF_GODMODE;
                player->powers[pw_invulnerability] = 0;

                if (!(leveltime & 0x1F))
                    P_DamageMobj(player->mo, NULL, NULL, 20, true);

                if (player->health <= 10)
                    G_ExitLevel();

                break;
        }
    }
    else
    {
        switch ((sector->special & DAMAGE_MASK) >> DAMAGE_SHIFT)
        {
            case 0:     // no damage
                break;

            case 1:     // 2/5 damage per 31 ticks
                if (!player->powers[pw_ironfeet])
                    if (!(leveltime & 0x1F))
                        P_DamageMobj(player->mo, NULL, NULL, 5, true);

                break;

            case 2:     // 5/10 damage per 31 ticks
                if (!player->powers[pw_ironfeet])
                    if (!(leveltime & 0x1F))
                        P_DamageMobj(player->mo, NULL, NULL, 10, true);

                break;

            case 3:     // 10/20 damage per 31 ticks
                if (!player->powers[pw_ironfeet] || M_Random() < 5)  // take damage even with suit
                    if (!(leveltime & 0x1F))
                        P_DamageMobj(player->mo, NULL, NULL, 20, true);

                break;
        }

        if (sector->special & SECRET_MASK)
        {
            P_SecretFound(player);
            sector->special &= ~SECRET_MASK;

            if (sector->special < 32)   // if all extended bits clear,
                sector->special = 0;    // sector is not special anymore
        }
    }
}

//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//
int timer;
int countdown;

void P_UpdateSpecials(void)
{
    anim_t  *anim;
    int     pic;
    int     i;

    if (freeze)
        return;

    if (timer)
        if (!--countdown)
            G_ExitLevel();

    // ANIMATE FLATS AND TEXTURES GLOBALLY
    for (anim = anims; anim < lastanim; anim++)
        for (i = anim->basepic; i < anim->basepic + anim->numpics; i++)
        {
            pic = anim->basepic + ((leveltime / anim->speed + i) % anim->numpics);

            if (anim->istexture)
                texturetranslation[i] = pic;
            else
                flattranslation[i] = pic;
        }

    animatedliquiddiff += animatedliquiddiffs[leveltime & 63];
    animatedliquidxoffs += animatedliquidxdir;

    if (animatedliquidxoffs > 64 * FRACUNIT)
        animatedliquidxoffs = 0;

    animatedliquidyoffs += animatedliquidydir;

    if (animatedliquidyoffs > 64 * FRACUNIT)
        animatedliquidyoffs = 0;

    skycolumnoffset += skyscrolldelta;

    // DO BUTTONS
    for (i = 0; i < MAXBUTTONS; i++)
        if (buttonlist[i].btimer)
            if (!--buttonlist[i].btimer)
            {
                switch (buttonlist[i].where)
                {
                    case top:
                        sides[buttonlist[i].line->sidenum[0]].toptexture = buttonlist[i].btexture;
                        break;

                    case middle:
                        sides[buttonlist[i].line->sidenum[0]].midtexture = buttonlist[i].btexture;
                        break;

                    case bottom:
                        sides[buttonlist[i].line->sidenum[0]].bottomtexture = buttonlist[i].btexture;
                        break;
                }

                S_StartSectorSound(buttonlist[i].soundorg, sfx_swtchn);
                memset(&buttonlist[i], 0, sizeof(button_t));
            }
}

//
// Special Stuff that cannot be categorized
//
dboolean EV_DoDonut(line_t *line)
{
    sector_t    *s1;
    sector_t    *s2;
    sector_t    *s3;
    int         secnum = -1;
    dboolean    rtn = false;
    int         i;
    floormove_t *floor;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        s1 = &sectors[secnum];

        // ALREADY MOVING? IF SO, KEEP GOING...
        if (P_SectorActive(floor_special, s1))
            continue;

        s2 = getNextSector(s1->lines[0], s1);

        if (!s2)
            continue;

        if (P_SectorActive(floor_special, s2))
            continue;

        for (i = 0; i < s2->linecount; i++)
        {
            s3 = s2->lines[i]->backsector;

            if (!s3 || s3 == s1)
                continue;

            rtn = true;

            // Spawn rising slime
            floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, NULL);
            P_AddThinker(&floor->thinker);
            s2->floordata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = donutRaise;
            floor->crush = false;
            floor->direction = 1;
            floor->sector = s2;
            floor->speed = FLOORSPEED / 2;
            floor->texture = s3->floorpic;
            floor->newspecial = 0;
            floor->floordestheight = s3->floorheight;
            floor->stopsound = (floor->sector->floorheight != floor->floordestheight);

            // Spawn lowering donut-hole
            floor = Z_Calloc(1, sizeof(*floor), PU_LEVSPEC, NULL);
            P_AddThinker(&floor->thinker);
            s1->floordata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = lowerFloor;
            floor->crush = false;
            floor->direction = -1;
            floor->sector = s1;
            floor->speed = FLOORSPEED / 2;
            floor->floordestheight = s3->floorheight;
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
    line_t      *line;
    sector_t    *sector = sectors;
    int         i;

    if ((i = M_CheckParmWithArgs("-timer", 1, 1)))
    {
        timer = atoi(myargv[i + 1]);
        M_SaveCVARs();
        C_Output("A <b>-timer</b> parameter was found on the command-line. "
            "The time limit for each map is %i minutes.", timer);
    }

    if (M_CheckParm("-avg"))
    {
        timer = 20;
        M_SaveCVARs();
        C_Output("An <b>-avg</b> parameter was found on the command-line. "
            "The time limit for each map is %i minutes.", timer);
    }

    // Init special SECTORs.
    for (i = 0; i < numsectors; i++, sector++)
    {
        if (!sector->special)
            continue;

        if (sector->special & SECRET_MASK)
            totalsecret++;

        switch (sector->special & 31)
        {
            case LightBlinks_Randomly:
                P_SpawnLightFlash(sector);
                break;

            case LightBlinks_2Hz:
                P_SpawnStrobeFlash(sector, FASTDARK, false);
                break;

            case LightBlinks_1Hz:
                P_SpawnStrobeFlash(sector, SLOWDARK, false);
                break;

            case DamageNegative10Or20PercentHealthAndLightBlinks_2Hz:
                P_SpawnStrobeFlash(sector, FASTDARK, false);
                sector->special = DamageNegative10Or20PercentHealthAndLightBlinks_2Hz;
                break;

            case LightGlows_1PlusSec:
                P_SpawnGlowingLight(sector);
                break;

            case Secret:
                if (sector->special < 32)
                    totalsecret++;
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

    P_RemoveAllActiveCeilings();        // jff 2/22/98 use killough's scheme

    P_RemoveAllActivePlats();           // killough

    for (i = 0; i < MAXBUTTONS; i++)
        memset(&buttonlist[i], 0, sizeof(button_t));

    // P_InitTagLists() must be called before P_FindSectorFromLineTag()
    // or P_FindLineFromLineTag() can be called.

    P_InitTagLists();                   // killough 1/30/98: Create xref tables for tags

    P_SpawnScrollers();                 // killough 3/7/98: Add generalized scrollers

    P_SpawnFriction();                  // phares 3/12/98: New friction model using linedefs

    P_SpawnPushers();                   // phares 3/20/98: New pusher model using linedefs

    for (line = lines, i = 0; i < numlines; i++, line++)
    {
        int sec;
        int s;

        switch (line->special)
        {
            // killough 3/7/98:
            // support for drawn heights coming from different sector
            case CreateFakeCeilingAndFloor:
                sec = sides[*line->sidenum].sector - sectors;

                for (s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0;)
                    sectors[s].heightsec = sec;

                break;

            // killough 3/16/98: Add support for setting
            // floor lighting independently (e.g. lava)
            case Floor_ChangeBrightnessToThisBrightness:
                sec = sides[*line->sidenum].sector - sectors;

                for (s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0;)
                    sectors[s].floorlightsec = sec;

                break;

            // killough 4/11/98: Add support for setting
            // ceiling lighting independently
            case Ceiling_ChangeBrightnessToThisBrightness:
                sec = sides[*line->sidenum].sector - sectors;

                for (s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0;)
                    sectors[s].ceilinglightsec = sec;

                break;

            // killough 10/98:
            //
            // Support for sky textures being transferred from sidedefs.
            // Allows scrolling and other effects (but if scrolling is
            // used, then the same sector tag needs to be used for the
            // sky sector, the sky-transfer linedef, and the scroll-effect
            // linedef). Still requires user to use F_SKY1 for the floor
            // or ceiling texture, to distinguish floor and ceiling sky.
            case TransferSkyTextureToTaggedSectors:
            case TransferSkyTextureToTaggedSectors_Flipped:
                for (s = -1; (s = P_FindSectorFromLineTag(line, s)) >= 0;)
                    sectors[s].sky = (i | PL_SKYFLAT);

                break;
        }
    }
}

// killough 2/28/98:
//
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
// killough 3/7/98
void T_Scroll(scroll_t *s)
{
    fixed_t dx = s->dx;
    fixed_t dy = s->dy;

    if (freeze)
        return;

    if (s->control != -1)
    {
        // compute scroll amounts based on a sector's height changes
        fixed_t height = sectors[s->control].floorheight + sectors[s->control].ceilingheight;
        fixed_t delta = height - s->last_height;

        s->last_height = height;
        dx = FixedMul(dx, delta);
        dy = FixedMul(dy, delta);
    }

    // killough 3/14/98: Add acceleration
    if (s->accel)
    {
        s->vdx = dx += s->vdx;
        s->vdy = dy += s->vdy;
    }

    if (!(dx | dy))                             // no-op if both (x,y) offsets 0
        return;

    switch (s->type)
    {
        side_t      *side;
        sector_t    *sec;
        fixed_t     height;
        fixed_t     waterheight;                // killough 4/4/98: add waterheight
        msecnode_t  *node;

        case sc_side:                           // killough 3/7/98: Scroll wall texture
            side = sides + s->affectee;
            side->textureoffset += dx;
            side->rowoffset += dy;
            break;

        case sc_floor:                          // killough 3/7/98: Scroll floor texture
            sec = sectors + s->affectee;
            sec->floor_xoffs += dx;
            sec->floor_yoffs += dy;
            break;

        case sc_ceiling:                        // killough 3/7/98: Scroll ceiling texture
            sec = sectors + s->affectee;
            sec->ceiling_xoffs += dx;
            sec->ceiling_yoffs += dy;
            break;

        case sc_carry:
            // killough 3/7/98: Carry things on floor
            // killough 3/20/98: use new sector list which reflects true members
            // killough 3/27/98: fix carrier bug
            // killough 4/4/98: Underwater, carry things even w/o gravity
            sec = sectors + s->affectee;
            height = sec->floorheight;
            waterheight = (sec->heightsec != -1 && sectors[sec->heightsec].floorheight > height ?
                sectors[sec->heightsec].floorheight : INT_MIN);

            // Move objects only if on floor or underwater,
            // non-floating, and clipped.
            for (node = sec->touching_thinglist; node; node = node->m_snext)
            {
                mobj_t  *thing = node->m_thing;

                if (!(thing->flags & MF_NOCLIP) && (!((thing->flags & MF_NOGRAVITY) || thing->z > height)
                    || thing->z < waterheight))
                {
                    thing->momx += dx;
                    thing->momy += dy;
                }
            }

            break;

        case sc_carry_ceiling:       // to be added later
            break;
    }
}

//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// type: the enumerated type of scrolling: floor, ceiling, floor carrier,
//   wall, floor carrier & scroller
//
// (dx,dy): the direction and speed of the scrolling or its acceleration
//
// control: the sector whose heights control this scroller's effect
//   remotely, or -1 if no control sector
//
// affectee: the index of the affected object (sector or sidedef)
//
// accel: non-zero if this is an accelerative effect
//
static void Add_Scroller(int type, fixed_t dx, fixed_t dy, int control, int affectee, int accel)
{
    scroll_t    *s = Z_Calloc(1, sizeof(*s), PU_LEVSPEC, NULL);

    s->thinker.function = T_Scroll;
    s->type = type;
    s->dx = dx;
    s->dy = dy;
    s->accel = accel;
    s->vdx = s->vdy = 0;

    if ((s->control = control) != -1)
        s->last_height = sectors[control].floorheight + sectors[control].ceilingheight;

    s->affectee = affectee;
    P_AddThinker(&s->thinker);
}

// Adds wall scroller. Scroll amount is rotated with respect to wall's
// linedef first, so that scrolling towards the wall in a perpendicular
// direction is translated into vertical motion, while scrolling along
// the wall in a parallel direction is translated into horizontal motion.
//
// killough 5/25/98: cleaned up arithmetic to avoid drift due to roundoff
//
// killough 10/98:
// fix scrolling aliasing problems, caused by long linedefs causing overflowing
static void Add_WallScroller(int64_t dx, int64_t dy, const line_t *l, int control, int accel)
{
    fixed_t     x = ABS(l->dx), y = ABS(l->dy), d;

    if (y > x)
    {
        d = x;
        x = y;
        y = d;
    }

    d = FixedDiv(x, finesine[(tantoangle[FixedDiv(y, x) >> DBITS] + ANG90) >> ANGLETOFINESHIFT]);

    x = (fixed_t)((dy * -l->dy - dx * l->dx) / d);      // killough 10/98:
    y = (fixed_t)((dy * l->dx - dx * l->dy) / d);       // Use long long arithmetic
    Add_Scroller(sc_side, x, y, control, *l->sidenum, accel);
}

// Amount (dx,dy) vector linedef is shifted right to get scroll amount
#define SCROLL_SHIFT    5

// Factor to scale scrolling effect into mobj-carrying properties = 3/32.
// (This is so scrolling floors and objects on them can move at same speed.)
#define CARRYFACTOR     ((fixed_t)(FRACUNIT * 0.09375))

// Initialize the scrollers
static void P_SpawnScrollers(void)
{
    int     i;
    line_t  *l = lines;

    for (i = 0; i < numlines; i++, l++)
    {
        fixed_t dx = l->dx >> SCROLL_SHIFT;             // direction and speed of scrolling
        fixed_t dy = l->dy >> SCROLL_SHIFT;
        int     control = -1, accel = 0;                // no control sector or acceleration
        int     special = l->special;

        // killough 3/7/98: Types 245-249 are same as 250-254 except that the
        // first side's sector's heights cause scrolling when they change, and
        // this linedef controls the direction and speed of the scrolling. The
        // most complicated linedef since donuts, but powerful :)
        //
        // killough 3/15/98: Add acceleration. Types 214-218 are the same but
        // are accelerative.
        if (special >= Scroll_ScrollCeilingWhenSectorChangesHeight
            && special <= Scroll_ScrollWallWhenSectorChangesHeight)      // displacement scrollers
        {
            special += Scroll_ScrollCeilingAccordingToLineVector
                - Scroll_ScrollCeilingWhenSectorChangesHeight;
            control = sides[*l->sidenum].sector - sectors;
        }
        else if (special >= Scroll_CeilingAcceleratesWhenSectorHeightChanges
            && special <= Scroll_WallAcceleratesWhenSectorHeightChanges) // accelerative scrollers
        {
            accel = 1;
            special += Scroll_ScrollCeilingAccordingToLineVector
                - Scroll_CeilingAcceleratesWhenSectorHeightChanges;
            control = sides[*l->sidenum].sector - sectors;
        }

        switch (special)
        {
            int s;

            case Scroll_ScrollCeilingAccordingToLineVector:
                for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
                    Add_Scroller(sc_ceiling, -dx, dy, control, s, accel);

                break;

            case Scroll_ScrollFloorAccordingToLineVector:
            case Scroll_ScrollFloorAndMoveThings:
                for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
                    Add_Scroller(sc_floor, -dx, dy, control, s, accel);

                if (special != Scroll_ScrollFloorAndMoveThings)
                    break;

            case Scroll_MoveThingsAccordingToLineVector:
                dx = FixedMul(dx, CARRYFACTOR);
                dy = FixedMul(dy, CARRYFACTOR);

                for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
                    Add_Scroller(sc_carry, dx, dy, control, s, accel);

                break;

            // killough 3/1/98: scroll wall according to linedef
            // (same direction and speed as scrolling floors)
            case Scroll_ScrollWallAccordingToLineVector:
                for (s = -1; (s = P_FindLineFromLineTag(l, s)) >= 0;)
                    if (s != i)
                        Add_WallScroller(dx, dy, lines + s, control, accel);

                break;

            case Scroll_ScrollWallUsingSidedefOffsets:
                s = lines[i].sidenum[0];
                Add_Scroller(sc_side, -sides[s].textureoffset, sides[s].rowoffset, -1, s, accel);
                break;

            case Scroll_ScrollTextureLeft:
                Add_Scroller(sc_side, FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
                break;

            case Scroll_ScrollTextureRight:
                Add_Scroller(sc_side, -FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
                break;
        }
    }
}

//
// FRICTION EFFECTS
//
// phares 3/12/98: Start of friction effects
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
// killough 8/28/98:
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
    int     i;
    line_t  *l = lines;

    // killough 8/28/98: initialize all sectors to normal friction first
    for (i = 0; i < numsectors; i++)
    {
        sectors[i].friction = ORIG_FRICTION;
        sectors[i].movefactor = ORIG_FRICTION_FACTOR;
    }

    for (i = 0; i < numlines; i++, l++)
        if (l->special == FrictionTaggedSector)
        {
            int length = P_ApproxDistance(l->dx, l->dy) >> FRACBITS;
            int friction = BETWEEN(0, (0x1EB8 * length) / 0x80 + 0xD000, FRACUNIT);
            int movefactor;
            int s;

            // The following check might seem odd. At the time of movement,
            // the move distance is multiplied by 'friction/0x10000', so a
            // higher friction value actually means 'less friction'.
            if (friction > ORIG_FRICTION)       // ice
                movefactor = ((0x10092 - friction) * 0x70) / 0x158;
            else
                movefactor = ((friction - 0xDB34) *0xA) / 0x80;

            if (movefactor < 32)
                movefactor = 32;

            for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
            {
                // killough 8/28/98:
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
// phares 3/20/98: Start of push/pull effects
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
// linedef. The force vector for types 3 & 4 is determined by the angle
// of the linedef, and is constant.
//
// For each sector where these effects occur, the sector special type has
// to have the PUSH_MASK bit set. If this bit is turned off by a switch
// at run-time, the effect will not occur. The controlling sector for
// types 1 & 2 is the sector containing the MT_PUSH/MT_PULL Thing.

#define PUSH_FACTOR 7

//
// Add a push thinker to the thinker list
static void Add_Pusher(int type, int x_mag, int y_mag, mobj_t *source, int affectee)
{
    pusher_t    *p = Z_Calloc(1, sizeof(*p), PU_LEVSPEC, NULL);

    p->thinker.function = T_Pusher;
    p->source = source;
    p->type = type;
    p->x_mag = x_mag >> FRACBITS;
    p->y_mag = y_mag >> FRACBITS;
    p->magnitude = P_ApproxDistance(p->x_mag, p->y_mag);

    if (source)                                         // point source exist?
    {
        p->radius = p->magnitude << (FRACBITS + 1);   // where force goes to zero
        p->x = p->source->x;
        p->y = p->source->y;
    }

    p->affectee = affectee;
    P_AddThinker(&p->thinker);
}

//
// PIT_PushThing determines the angle and magnitude of the effect.
// The object's x and y momentum values are changed.
//
// tmpusher belongs to the point source (MT_PUSH/MT_PULL).
//
// killough 10/98: allow to affect things besides players

static pusher_t *tmpusher;      // pusher structure for blockmap searches

static dboolean PIT_PushThing(mobj_t *thing)
{
    if ((sentient(thing) || (thing->flags & MF_SHOOTABLE)) && !(thing->flags & MF_NOCLIP))
    {
        fixed_t speed;
        fixed_t sx = tmpusher->x;
        fixed_t sy = tmpusher->y;

        speed = (tmpusher->magnitude - ((P_ApproxDistance(thing->x - sx, thing->y - sy)
            >> FRACBITS) >> 1)) << (FRACBITS - PUSH_FACTOR - 1);

        // killough 10/98: make magnitude decrease with square
        // of distance, making it more in line with real nature,
        // so long as it's still in range with original formula.
        //
        // Removes angular distortion, and makes effort required
        // to stay close to source, grow increasingly hard as you
        // get closer, as expected. Still, it doesn't consider z :(
        {
            int x = (thing->x - sx) >> FRACBITS;
            int y = (thing->y - sy) >> FRACBITS;

            speed = (fixed_t)(((int64_t)tmpusher->magnitude << 23) / (x * x + y * y + 1));
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
        }
    }

    return true;
}

//
// T_Pusher looks for all objects that are inside the radius of
// the effect.
//
void T_Pusher(pusher_t *p)
{
    sector_t    *sec;
    msecnode_t  *node;
    int         xspeed, yspeed;
    int         ht = 0;

    if (freeze)
        return;

    sec = sectors + p->affectee;

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
    if (p->type == p_push)
    {
        // Seek out all pushable things within the force radius of this
        // point pusher. Crosses sectors, so use blockmap.
        int xl;
        int xh;
        int yl;
        int yh;
        int bx, by;
        int radius;

        tmpusher = p;                                   // MT_PUSH/MT_PULL point source
        radius = p->radius;                             // where force goes to zero
        tmbbox[BOXTOP] = p->y + radius;
        tmbbox[BOXBOTTOM] = p->y - radius;
        tmbbox[BOXRIGHT] = p->x + radius;
        tmbbox[BOXLEFT] = p->x - radius;

        xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
        xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
        yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
        yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

        for (bx = xl; bx <= xh; bx++)
            for (by = yl; by <= yh; by++)
                P_BlockThingsIterator(bx, by, PIT_PushThing);

        return;
    }

    // constant pushers p_wind and p_current
    if (sec->heightsec != -1)                           // special water sector?
        ht = sectors[sec->heightsec].floorheight;

    node = sec->touching_thinglist;                     // things touching this sector

    for (; node; node = node->m_snext)
    {
        mobj_t  *thing = node->m_thing;

        if (!thing->player || (thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
            continue;

        if (p->type == p_wind)
        {
            if (sec->heightsec == -1)                   // NOT special water sector
                if (thing->z > thing->floorz)           // above ground
                {
                    xspeed = p->x_mag;                  // full force
                    yspeed = p->y_mag;
                }
                else                                    // on ground
                {
                    xspeed = p->x_mag >> 1;             // half force
                    yspeed = p->y_mag >> 1;
                }
            else                                        // special water sector
            {
                if (thing->z > ht)                      // above ground
                {
                    xspeed = p->x_mag;                  // full force
                    yspeed = p->y_mag;
                }
                else
                {
                    if (thing->player->viewz < ht)      // underwater
                        xspeed = yspeed = 0;            // no force
                    else                                // wading in water
                    {
                        xspeed = p->x_mag >> 1;         // half force
                        yspeed = p->y_mag >> 1;
                    }
                }
            }
        }
        else                                            // p_current
        {
            if (sec->heightsec == -1)                   // NOT special water sector
            {
                if (thing->z > sec->floorheight)        // above ground
                {
                    xspeed = 0;                         // no force
                    yspeed = 0;
                }
                else                                    // on ground
                {
                    xspeed = p->x_mag;                  // full force
                    yspeed = p->y_mag;
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
                    xspeed = p->x_mag;                  // full force
                    yspeed = p->y_mag;
                }
            }
        }

        thing->momx += xspeed << (FRACBITS - PUSH_FACTOR);
        thing->momy += yspeed << (FRACBITS - PUSH_FACTOR);
    }
}

//
// P_GetPushThing() returns a pointer to an MT_PUSH or MT_PULL thing,
// NULL otherwise.
mobj_t *P_GetPushThing(int s)
{
    sector_t    *sec = sectors + s;
    mobj_t      *thing = sec->thinglist;

    while (thing)
    {
        switch (thing->type)
        {
            case MT_PUSH:
            case MT_PULL:
                return thing;

            default:
                break;
        }

        thing = thing->snext;
    }

    return NULL;
}

//
// Initialize the sectors where pushers are present
//
static void P_SpawnPushers(void)
{
    int     i;
    line_t  *l = lines;
    int     s;
    mobj_t  *thing;

    for (i = 0; i < numlines; i++, l++)
        switch (l->special)
        {
            case WindAccordingToLineVector:
                for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
                    Add_Pusher(p_wind, l->dx, l->dy, NULL, s);

                break;

            case CurrentAccordingToLineVector:
                for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
                    Add_Pusher(p_current, l->dx, l->dy, NULL, s);

                break;

            case WindCurrentByPushPullThingInSector:
                for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
                {
                    thing = P_GetPushThing(s);

                    if (thing)  // No MT_P* means no effect
                        Add_Pusher(p_push, l->dx, l->dy, thing, s);
                }

                break;
        }
}
