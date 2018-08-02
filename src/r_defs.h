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

#if !defined(__R_DEFS_H__)
#define __R_DEFS_H__

#include "p_mobj.h"

// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE    0
#define SIL_BOTTOM  1
#define SIL_TOP     2
#define SIL_BOTH    3

#define MAXDRAWSEGS 1280
#define MAXOPENINGS 16384

//
// INTERNAL MAP TYPES
//  used by play and refresh
//

//
// Your plain vanilla vertex.
// Note: transformed values not buffered locally,
//  like some DOOM-alikes ("wt", "WebView") did.
//
typedef struct
{
    fixed_t             x, y;
} vertex_t;

// Each sector has a degenmobj_t in its center
//  for sound origin purposes.
// I suppose this does not handle sound from
//  moving objects (doppler), because
//  position is prolly just buffered, not
//  updated.
typedef struct
{
    thinker_t           thinker;        // not used for anything
    fixed_t             x, y, z;
} degenmobj_t;

typedef enum
{
    SOLID,
    NUKAGE,
    WATER,
    LAVA,
    BLOOD,
    SLIME
} terraintype_t;

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//
typedef struct sector_s
{
    int                 id;

    fixed_t             floorheight;
    fixed_t             ceilingheight;
    int                 nexttag;
    int                 firsttag;
    short               special;
    short               tag;

    // 0 = untraversed, 1,2 = sndlines -1
    int                 soundtraversed;

    // thing that made a sound (or null)
    mobj_t              *soundtarget;

    // mapblock bounding box for height changes
    int                 blockbox[4];

    // origin for any sounds played by the sector
    degenmobj_t         soundorg;

    // if == validcount, already checked
    int                 validcount;

    // list of mobjs in sector
    mobj_t              *thinglist;

    bloodsplat_t        *splatlist;

    // thinker_t for reversible actions
    void                *floordata;             // jff 2/22/98 make thinkers on
    void                *ceilingdata;           // floors, ceilings, lighting,
    void                *lightingdata;          // independent of one another

    // list of mobjs that are at least partially in the sector
    // thinglist is a subset of touching_thinglist
    struct msecnode_s   *touching_thinglist;    // phares 3/14/98

    int                 linecount;
    struct line_s       **lines;                // [linecount] size

    int                 cachedheight;

    // [AM] Previous position of floor and ceiling before
    //      think. Used to interpolate between positions.
    fixed_t             oldfloorheight;
    fixed_t             oldceilingheight;

    // [AM] Gametic when the old positions were recorded.
    //      Has a dual purpose; it prevents movement thinkers
    //      from storing old positions twice in a tic, and
    //      prevents the renderer from attempting to interpolate
    //      if old values were not updated recently.
    int                 oldgametime;

    // [AM] Interpolated floor and ceiling height.
    //      Calculated once per tic and used inside
    //      the renderer.
    fixed_t             interpfloorheight;
    fixed_t             interpceilingheight;

    // jff 2/26/98 lockout machinery for stairbuilding
    int                 stairlock;      // -2 on first locked -1 after thinker done 0 normally
    int                 prevsec;        // -1 or number of sector for previous step
    int                 nextsec;        // -1 or number of next step sector

    // killough 3/7/98: floor and ceiling texture offsets
    fixed_t             floor_xoffs, floor_yoffs;
    fixed_t             ceiling_xoffs, ceiling_yoffs;

    // killough 4/11/98: support for lightlevels coming from another sector
    struct sector_s     *floorlightsec;
    struct sector_s     *ceilinglightsec;

    short               floorpic;
    short               ceilingpic;
    short               lightlevel;

    // killough 3/7/98: support flat heights drawn at another sector's heights
    struct sector_s     *heightsec;     // other sector, or NULL if no other sector

    // killough 4/4/98: dynamic colormaps
    int                 bottommap;
    int                 midmap;
    int                 topmap;

    // killough 8/28/98: friction is a sector property, not an mobj property.
    // these fields used to be in mobj_t, but presented performance problems
    // when processed as mobj properties. Fix is to make them sector properties.
    int                 friction;
    int                 movefactor;

    // killough 10/98: support skies coming from sidedefs. Allows scrolling
    // skies and other effects. No "level info" kind of lump is needed,
    // because you can use an arbitrary number of skies per level with this
    // method. This field only applies when skyflatnum is used for floorpic
    // or ceilingpic, because the rest of DOOM needs to know which is sky
    // and which isn't, etc.
    int                 sky;

    terraintype_t       terraintype;

    dboolean            islift;
} sector_t;

//
// The SideDef.
//
typedef struct
{
    // add this to the calculated texture column
    fixed_t             textureoffset;

    // add this to the calculated texture top
    fixed_t             rowoffset;

    // Texture indices.
    // We do not maintain names here.
    short               toptexture;
    short               bottomtexture;
    short               midtexture;

    // Sector the SideDef is facing.
    sector_t            *sector;

    // killough 4/4/98, 4/11/98: highest referencing special linedef's type,
    // or lump number of special effect. Allows texture names to be overloaded
    // for other functions.
    int                 special;

    dboolean            missingtoptexture;
    dboolean            missingmidtexture;
    dboolean            missingbottomtexture;
} side_t;

//
// Move clipping aid for LineDefs.
//
typedef enum
{
    ST_HORIZONTAL,
    ST_VERTICAL,
    ST_POSITIVE,
    ST_NEGATIVE
} slopetype_t;

typedef struct line_s
{
    int                 id;

    // Vertices, from v1 to v2.
    vertex_t            *v1;
    vertex_t            *v2;

    // Precalculated v2 - v1 for side checking.
    fixed_t             dx, dy;

    // Animation related.
    unsigned short      flags;

    unsigned short      special;
    short               tag;

    // Visual appearance: SideDefs.
    //  sidenum[1] will be -1 if one sided
    unsigned short      sidenum[2];

    // Neat. Another bounding box, for the extent
    //  of the LineDef.
    fixed_t             bbox[4];

    // To aid move clipping.
    slopetype_t         slopetype;

    // Front and back sector.
    // Note: redundant? Can be retrieved from SideDefs.
    sector_t            *frontsector;
    sector_t            *backsector;

    // if == validcount, already checked
    int                 validcount;

    int                 tranlump;       // killough 4/11/98: translucency filter, -1 == none

    int                 nexttag;
    int                 firsttag;

    int                 r_validcount;   // cph: if == gametime, r_flags already done

    enum
    {                                   // cph:
        RF_TOP_TILE =  1,               // Upper texture needs tiling
        RF_MID_TILE =  2,               // Mid texture needs tiling
        RF_BOT_TILE =  4,               // Lower texture needs tiling
        RF_IGNORE   =  8,               // Renderer can skip this line
        RF_CLOSED   = 16                // Line blocks view
    } r_flags;

    // sound origin for switches/buttons
    degenmobj_t         soundorg;
} line_t;

enum
{
    DR_Door_OpenWaitClose_AlsoMonsters                             =   1,
    W1_Door_OpenStay                                               =   2,
    W1_Door_CloseStay                                              =   3,
    W1_Door_OpenWaitClose                                          =   4,
    W1_Floor_RaiseToLowestCeiling                                  =   5,
    W1_Crusher_StartWithFastDamage                                 =   6,
    S1_Stairs_RaiseBy8                                             =   7,
    W1_Stairs_RaiseBy8                                             =   8,
    S1_Floor_RaiseDonut_ChangesTexture                             =   9,
    W1_Lift_LowerWaitRaise                                         =  10,
    S1_ExitLevel                                                   =  11,
    W1_Light_ChangeToBrightestAdjacent                             =  12,
    W1_Light_ChangeTo255                                           =  13,
    S1_Floor_RaiseBy32_ChangesTexture                              =  14,
    S1_Floor_RaiseBy24_ChangesTexture                              =  15,
    W1_Door_CloseWaitOpen                                          =  16,
    W1_Light_StartBlinking                                         =  17,
    S1_Floor_RaiseToNextHighestFloor                               =  18,
    W1_Floor_LowerToHighestFloor                                   =  19,
    S1_Floor_RaiseToNextHighestFloor_ChangesTexture                =  20,
    S1_Lift_LowerWaitRaise                                         =  21,
    W1_Floor_RaiseToNextHighestFloor_ChangesTexture                =  22,
    S1_Floor_LowerToLowestFloor                                    =  23,
    G1_Floor_RaiseToLowestCeiling                                  =  24,
    W1_Crusher_StartWithSlowDamage                                 =  25,
    DR_Door_Blue_OpenWaitClose                                     =  26,
    DR_Door_Yellow_OpenWaitClose                                   =  27,
    DR_Door_Red_OpenWaitClose                                      =  28,
    S1_Door_OpenWaitClose                                          =  29,
    W1_Floor_RaiseByShortestLowerTexture                           =  30,
    D1_Door_OpenStay                                               =  31,
    D1_Door_Blue_OpenStay                                          =  32,
    D1_Door_Red_OpenStay                                           =  33,
    D1_Door_Yellow_OpenStay                                        =  34,
    W1_Light_ChangeTo35                                            =  35,
    W1_Floor_LowerTo8AboveHighestFloor                             =  36,
    W1_Floor_LowerToLowestFloor_ChangesTexture                     =  37,
    W1_Floor_LowerToLowestFloor                                    =  38,
    W1_Teleport                                                    =  39,
    W1_Ceiling_RaiseToHighestCeiling                               =  40,
    S1_Ceiling_LowerToFloor                                        =  41,
    SR_Door_CloseStay                                              =  42,
    SR_Ceiling_LowerToFloor                                        =  43,
    W1_Ceiling_LowerTo8AboveFloor                                  =  44,
    SR_Floor_LowerToHighestFloor                                   =  45,
    G1_Door_OpenStay                                               =  46,
    G1_Floor_RaiseToNextHighestFloor_ChangesTexture                =  47,
    Scroll_ScrollTextureLeft                                       =  48,
    S1_Ceiling_LowerTo8AboveFloor_PerpetualSlowCrusherDamage       =  49,
    S1_Door_CloseStay                                              =  50,
    S1_ExitLevel_GoesToSecretLevel                                 =  51,
    W1_ExitLevel                                                   =  52,
    W1_Floor_StartMovingUpAndDown                                  =  53,
    W1_Floor_StopMoving                                            =  54,
    S1_Floor_RaiseTo8BelowLowestCeiling_Crushes                    =  55,
    W1_Floor_RaiseTo8BelowLowestCeiling_Crushes                    =  56,
    W1_Crusher_Stop                                                =  57,
    W1_Floor_RaiseBy24                                             =  58,
    W1_Floor_RaiseBy24_ChangesTexture                              =  59,
    SR_Floor_LowerToLowestFloor                                    =  60,
    SR_Door_OpenStay                                               =  61,
    SR_Lift_LowerWaitRaise                                         =  62,
    SR_Door_OpenWaitClose                                          =  63,
    SR_Floor_RaiseToLowestCeiling                                  =  64,
    SR_Floor_RaiseTo8BelowLowestCeiling_Crushes                    =  65,
    SR_Floor_RaiseBy24_ChangesTexture                              =  66,
    SR_Floor_RaiseBy32_ChangesTexture                              =  67,
    SR_Floor_RaiseToNextHighestFloor_ChangesTexture                =  68,
    SR_Floor_RaiseToNextHighestFloor                               =  69,
    SR_Floor_LowerTo8AboveHighestFloor                             =  70,
    S1_Floor_LowerTo8AboveHighestFloor                             =  71,
    WR_Ceiling_LowerTo8AboveFloor                                  =  72,
    WR_Crusher_StartWithSlowDamage                                 =  73,
    WR_Crusher_Stop                                                =  74,
    WR_Door_CloseStay                                              =  75,
    WR_Door_CloseStayOpen                                          =  76,
    WR_Crusher_StartWithFastDamage                                 =  77,
    SR_ChangeTextureAndEffectToNearest                             =  78,
    WR_Light_ChangeTo35                                            =  79,
    WR_Light_ChangeToBrightestAdjacent                             =  80,
    WR_Light_ChangeTo255                                           =  81,
    WR_Floor_LowerToLowestFloor                                    =  82,
    WR_Floor_LowerToHighestFloor                                   =  83,
    WR_Floor_LowerToLowestFloor_ChangesTexture                     =  84,
    Scroll_ScrollTextureRight                                      =  85,
    WR_Door_OpenStay                                               =  86,
    WR_Floor_StartMovingUpAndDown                                  =  87,
    WR_Lift_LowerWaitRaise                                         =  88,
    WR_Floor_StopMoving                                            =  89,
    WR_Door_OpenWaitClose                                          =  90,
    WR_Floor_RaiseToLowestCeiling                                  =  91,
    WR_Floor_RaiseBy24                                             =  92,
    WR_Floor_RaiseBy24_ChangesTexture                              =  93,
    WR_Floor_RaiseTo8BelowLowestCeiling_Crushes                    =  94,
    WR_Floor_RaiseToNextHighestFloor_ChangesTexture                =  95,
    WR_Floor_RaiseByShortestLowerTexture                           =  96,
    WR_Teleport                                                    =  97,
    WR_Floor_LowerTo8AboveHighestFloor                             =  98,
    SR_Door_Blue_OpenStay_Fast                                     =  99,
    W1_Stairs_RaiseBy16_Fast                                       = 100,
    S1_Floor_RaiseToLowestCeiling                                  = 101,
    S1_Floor_LowerToHighestFloor                                   = 102,
    S1_Door_OpenStay                                               = 103,
    W1_Light_ChangeToDarkestAdjacent                               = 104,
    WR_Door_OpenWaitClose_Fast                                     = 105,
    WR_Door_OpenStay_Fast                                          = 106,
    WR_Door_CloseStay_Fast                                         = 107,
    W1_Door_OpenWaitClose_Fast                                     = 108,
    W1_Door_OpenStay_Fast                                          = 109,
    W1_Door_CloseStay_Fast                                         = 110,
    S1_Door_OpenWaitClose_Fast                                     = 111,
    S1_Door_OpenStay_Fast                                          = 112,
    S1_Door_CloseStay_Fast                                         = 113,
    SR_Door_OpenWaitClose_Fast                                     = 114,
    SR_Door_OpenStay_Fast                                          = 115,
    SR_Door_CloseStay_Fast                                         = 116,
    DR_Door_OpenWaitClose_Fast                                     = 117,
    D1_Door_OpenStay_Fast                                          = 118,
    W1_Floor_RaiseToNextHighestFloor                               = 119,
    WR_Lift_LowerWaitRaise_Fast                                    = 120,
    W1_Lift_LowerWaitRaise_Fast                                    = 121,
    S1_Lift_LowerWaitRaise_Fast                                    = 122,
    SR_Lift_LowerWaitRaise_Fast                                    = 123,
    W1_ExitLevel_GoesToSecretLevel                                 = 124,
    W1_Teleport_MonstersOnly                                       = 125,
    WR_Teleport_MonstersOnly                                       = 126,
    S1_Stairs_RaiseBy16_Fast                                       = 127,
    WR_Floor_RaiseToNextHighestFloor                               = 128,
    WR_Floor_RaiseToNextHighestFloor_Fast                          = 129,
    W1_Floor_RaiseToNextHighestFloor_Fast                          = 130,
    S1_Floor_RaiseToNextHighestFloor_Fast                          = 131,
    SR_Floor_RaiseToNextHighestFloor_Fast                          = 132,
    S1_Door_Blue_OpenStay_Fast                                     = 133,
    SR_Door_Red_OpenStay_Fast                                      = 134,
    S1_Door_Red_OpenStay_Fast                                      = 135,
    SR_Door_Yellow_OpenStay_Fast                                   = 136,
    S1_Door_Yellow_OpenStay_Fast                                   = 137,
    SR_Light_ChangeTo255                                           = 138,
    SR_Light_ChangeTo35                                            = 139,
    S1_Floor_RaiseBy512                                            = 140,
    W1_Crusher_StartWithSlowDamage_Silent                          = 141,

    BOOMLINESPECIALS                                               = 142,

    W1_Floor_RaiseBy512                                            = 142,
    W1_Lift_RaiseBy24_ChangesTexture                               = 143,
    W1_Lift_RaiseBy32_ChangesTexture                               = 144,
    W1_CeilingLowerToFloor_Fast                                    = 145,
    W1_Floor_RaiseDonut_ChangesTexture                             = 146,
    WR_Floor_RaiseBy512                                            = 147,
    WR_Lift_RaiseBy24_ChangesTexture                               = 148,
    WR_Lift_RaiseBy32_ChangesTexture                               = 149,
    WR_Crusher_Start_Silent                                        = 150,
    WR_Ceiling_RaiseToHighestCeiling                               = 151,
    WR_Ceiling_LowerToFloor_Fast                                   = 152,
    W1_ChangeTextureAndEffect                                      = 153,
    WR_ChangeTextureAndEffect                                      = 154,
    WR_Floor_RaiseDonut_ChangesTexture                             = 155,
    WR_Light_StartBlinking                                         = 156,
    WR_Light_ChangeToDarkestAdjacent                               = 157,
    S1_Floor_RaiseByShortestLowerTexture                           = 158,
    S1_Floor_LowerToLowestFloor_ChangesTexture                     = 159,
    S1_Floor_RaiseBy24_ChangesTextureAndEffect                     = 160,
    S1_Floor_RaiseBy24                                             = 161,
    S1_Lift_PerpetualLowestAndHighestFloors                        = 162,
    S1_Lift_Stop                                                   = 163,
    S1_Crusher_Start_Fast                                          = 164,
    S1_Crusher_Start_Silent                                        = 165,
    S1_Ceiling_RaiseToHighestCeiling                               = 166,
    S1_Ceiling_LowerTo8AboveFloor                                  = 167,
    S1_Crusher_Stop                                                = 168,
    S1_Light_ChangeToBrightestAdjacent                             = 169,
    S1_Light_ChangeTo35                                            = 170,
    S1_Light_ChangeTo255                                           = 171,
    S1_Light_StartBlinking                                         = 172,
    S1_Light_ChangeToDarkestAdjacent                               = 173,
    S1_Teleport_AlsoMonsters                                       = 174,
    S1_Door_CloseWaitOpen_30Seconds                                = 175,
    SR_Floor_RaiseByShortestLowerTexture                           = 176,
    SR_Floor_LowerToLowestFloor_ChangesTexture                     = 177,
    SR_Floor_RaiseBy512                                            = 178,
    SR_Floor_RaiseBy512_ChangesTextureAndEffect                    = 179,
    SR_Floor_RaiseBy24                                             = 180,
    SR_Lift_PerpetualLowestAndHighestFloors                        = 181,
    SR_Lift_Stop                                                   = 182,
    SR_Crusher_Start_Fast                                          = 183,
    SR_Crusher_Start                                               = 184,
    SR_Crusher_Start_Silent                                        = 185,
    SR_Ceiling_RaiseToHighestCeiling                               = 186,
    SR_Ceiling_LowerTo8AboveFloor                                  = 187,
    SR_Crusher_Stop                                                = 188,
    S1_ChangeTextureAndEffect                                      = 189,
    SR_ChangeTextureAndEffect                                      = 190,
    SR_Floor_RaiseDonut_ChangesTexture                             = 191,
    SR_Light_ChangeToBrightestAdjacent                             = 192,
    SR_Light_StartBlinking                                         = 193,
    SR_Light_ChangeToDarkestAdjacent                               = 194,
    SR_Teleport_AlsoMonsters                                       = 195,
    SR_Door_CloseWaitOpen_30Seconds                                = 196,
    G1_ExitLevel                                                   = 197,
    G1_ExitLevel_GoesToSecretLevel                                 = 198,
    W1_Ceiling_LowerToLowestCeiling                                = 199,
    W1_Ceiling_LowerToHighestFloor                                 = 200,
    WR_Ceiling_LowerToLowestCeiling                                = 201,
    WR_Ceiling_LowerToHighestFloor                                 = 202,
    S1_Ceiling_LowerToLowestCeiling                                = 203,
    S1_Ceiling_LowerToHighestFloor                                 = 204,
    SR_Ceiling_LowerToLowestCeiling                                = 205,
    SR_Ceiling_LowerToHighestFloor                                 = 206,
    W1_Teleport_AlsoMonsters_Silent_SameAngle                      = 207,
    WR_Teleport_AlsoMonsters_Silent_SameAngle                      = 208,
    S1_Teleport_AlsoMonsters_Silent_SameAngle                      = 209,
    SR_Teleport_AlsoMonsters_Silent_SameAngle                      = 210,
    SR_Lift_RaiseToCeiling_Instantly                               = 211,
    WR_Lift_RaiseToCeiling_Instantly                               = 212,
    Floor_ChangeBrightnessToThisBrightness                         = 213,
    Scroll_CeilingAcceleratesWhenSectorHeightChanges               = 214,
    Scroll_FloorAcceleratesWhenSectorHeightChanges                 = 215,
    Scroll_ThingsAccelerateWhenSectorHeightChanges                 = 216,
    Scroll_FloorAndThingsAccelerateWhenSectorHeightChanges         = 217,
    Scroll_WallAcceleratesWhenSectorHeightChanges                  = 218,
    W1_Floor_LowerToNearestFloor                                   = 219,
    WR_Floor_LowerToNearestFloor                                   = 220,
    S1_Floor_LowerToNearestFloor                                   = 221,
    SR_Floor_LowerToNearestFloor                                   = 222,
    FrictionTaggedSector                                           = 223,
    WindAccordingToLineVector                                      = 224,
    CurrentAccordingToLineVector                                   = 225,
    WindCurrentByPushPullThingInSector                             = 226,
    W1_Lift_RaiseToNextHighestFloor_Fast                           = 227,
    WR_Lift_RaiseToNextHighestFloor_Fast                           = 228,
    S1_Lift_RaiseToNextHighestFloor_Fast                           = 229,
    SR_Lift_RaiseToNextHighestFloor_Fast                           = 230,
    W1_Lift_LowerToNextLowestFloor_Fast                            = 231,
    WR_Lift_LowerToNextLowestFloor_Fast                            = 232,
    S1_Lift_LowerToNextLowestFloor_Fast                            = 233,
    SR_Lift_LowerToNextLowestFloor_Fast                            = 234,
    W1_Lift_MoveToSameFloorHeight_Fast                             = 235,
    WR_Lift_MoveToSameFloorHeight_Fast                             = 236,
    S1_Lift_MoveToSameFloorHeight_Fast                             = 237,
    SR_Lift_MoveToSameFloorHeight_Fast                             = 238,
    W1_ChangeTextureAndEffectToNearest                             = 239,
    WR_ChangeTextureAndEffectToNearest                             = 240,
    S1_ChangeTextureAndEffectToNearest                             = 241,
    CreateFakeCeilingAndFloor                                      = 242,
    W1_TeleportToLineWithSameTag_Silent_SameAngle                  = 243,
    WR_TeleportToLineWithSameTag_Silent_SameAngle                  = 244,
    Scroll_ScrollCeilingWhenSectorChangesHeight                    = 245,
    Scroll_ScrollFloorWhenSectorChangesHeight                      = 246,
    Scroll_MoveThingsWhenSectorChangesHeight                       = 247,
    Scroll_ScrollFloorAndMoveThingsWhenSectorChangesHeight         = 248,
    Scroll_ScrollWallWhenSectorChangesHeight                       = 249,
    Scroll_ScrollCeilingAccordingToLineVector                      = 250,
    Scroll_ScrollFloorAccordingToLineVector                        = 251,
    Scroll_MoveThingsAccordingToLineVector                         = 252,
    Scroll_ScrollFloorAndMoveThings                                = 253,
    Scroll_ScrollWallAccordingToLineVector                         = 254,
    Scroll_ScrollWallUsingSidedefOffsets                           = 255,
    WR_Stairs_RaiseBy8                                             = 256,
    WR_Stairs_RaiseBy16_Fast                                       = 257,
    SR_Stairs_RaiseBy8                                             = 258,
    SR_Stairs_RaiseBy16_Fast                                       = 259,
    Translucent_MiddleTexture                                      = 260,
    Ceiling_ChangeBrightnessToThisBrightness                       = 261,
    W1_TeleportToLineWithSameTag_Silent_ReversedAngle              = 262,
    WR_TeleportToLineWithSameTag_Silent_ReversedAngle              = 263,
    W1_TeleportToLineWithSameTag_MonstersOnly_Silent_ReversedAngle = 264,
    WR_TeleportToLineWithSameTag_MonstersOnly_Silent_ReversedAngle = 265,
    W1_TeleportToLineWithSameTag_MonstersOnly_Silent               = 266,
    WR_TeleportToLineWithSameTag_MonstersOnly_Silent               = 267,
    W1_Teleport_MonstersOnly_Silent                                = 268,
    WR_Teleport_MonstersOnly_Silent                                = 269,

    // Extended line specials from MBF
    TransferSkyTextureToTaggedSectors                              = 271,
    TransferSkyTextureToTaggedSectors_Flipped                      = 272
};

enum
{
    Normal                                              =  0,
    LightBlinks_Randomly                                =  1,
    LightBlinks_2Hz                                     =  2,
    LightBlinks_1Hz                                     =  3,
    DamageNegative10Or20PercentHealthAndLightBlinks_2Hz =  4,
    DamageNegative5Or10PercentHealth                    =  5,
    DamageNegative2Or5PercentHealth                     =  7,
    LightGlows_1PlusSec                                 =  8,
    Secret                                              =  9,
    Door_CloseStay_After30sec                           = 10,
    DamageNegative10Or20PercentHealthAndEndLevel        = 11,
    LightBlinks_1HzSynchronized                         = 12,
    LightBlinks_2HzSynchronized                         = 13,
    Door_OpenClose_OpensAfter5Min                       = 14,
    DamageNegative10Or20PercentHealth                   = 16,
    LightFlickers_Randomly                              = 17,

    UNKNOWNSECTORSPECIAL
};

enum
{
    Nothing                                            =     0,
    Player1Start                                       =     1,
    Player2Start                                       =     2,
    Player3Start                                       =     3,
    Player4Start                                       =     4,
    BlueKeycard                                        =     5,
    YellowKeycard                                      =     6,
    SpiderMastermind                                   =     7,
    Backpack                                           =     8,
    ShotgunGuy                                         =     9,
    BloodyMess1                                        =    10,
    PlayerDeathmatchStart                              =    11,
    BloodyMess2                                        =    12,
    RedKeycard                                         =    13,
    TeleportDestination                                =    14,
    DeadPlayer                                         =    15,
    Cyberdemon                                         =    16,
    CellPack                                           =    17,
    DeadZombieman                                      =    18,
    DeadShotgunGuy                                     =    19,
    DeadImp                                            =    20,
    DeadDemon                                          =    21,
    DeadCacodemon                                      =    22,
    DeadLostSoulInvisible                              =    23,
    PoolOfBloodAndBones                                =    24,
    ImpaledHuman                                       =    25,
    TwitchingImpaledHuman                              =    26,
    SkullOnAPole                                       =    27,
    FiveSkullsShishKebab                               =    28,
    PileOfSkullsAndCandles                             =    29,
    TallGreenColumn                                    =    30,
    ShortGreenColumn                                   =    31,
    TallRedColumn                                      =    32,
    ShortRedColumn                                     =    33,
    Candlestick                                        =    34,
    Candelabra                                         =    35,
    ShortGreenColumnWithBeatingHeart                   =    36,
    ShortRedColumnWithSkull                            =    37,
    RedSkullKey                                        =    38,
    YellowSkullKey                                     =    39,
    BlueSkullKey                                       =    40,
    EvilEye                                            =    41,
    FloatingSkullRock                                  =    42,
    TorchedTree                                        =    43,
    TallBlueFirestick                                  =    44,
    TallGreenFirestick                                 =    45,
    TallRedFirestick                                   =    46,
    Stalagmite                                         =    47,
    TallTechnoPillar                                   =    48,
    HangingVictimTwitchingBlocking                     =    49,
    HangingVictimArmsOutBlocking                       =    50,
    HangingVictimOneLeggedBlocking                     =    51,
    HangingPairOfLegsBlocking                          =    52,
    HangingLegBlocking                                 =    53,
    LargeBrownTree                                     =    54,
    ShortBlueFirestick                                 =    55,
    ShortGreenFirestick                                =    56,
    ShortRedFirestick                                  =    57,
    Spectre                                            =    58,
    HangingVictimArmsOut                               =    59,
    HangingPairOfLegs                                  =    60,
    HangingVictimOneLegged                             =    61,
    HangingLeg                                         =    62,
    HangingVictimTwitching                             =    63,
    ArchVile                                           =    64,
    HeavyWeaponDude                                    =    65,
    Revenant                                           =    66,
    Mancubus                                           =    67,
    Arachnotron                                        =    68,
    HellKnight                                         =    69,
    BurningBarrel                                      =    70,
    PainElemental                                      =    71,
    CommanderKeen                                      =    72,
    HangingVictimGutsRemoved                           =    73,
    HangingVictimGutsAndBrainRemoved                   =    74,
    HangingTorsoLookingDown                            =    75,
    HangingTorsoOpenSkull                              =    76,
    HangingTorsoLookingUp                              =    77,
    HangingTorsoBrainRemoved                           =    78,
    PoolOfBloodAndGuts                                 =    79,
    PoolOfBlood                                        =    80,
    PoolOfBrains                                       =    81,
    SuperShotgun                                       =    82,
    MegaSphere                                         =    83,
    WolfensteinSS                                      =    84,
    TallTechnoFloorLamp                                =    85,
    ShortTechnoFloorLamp                               =    86,
    MonstersTarget                                     =    87,
    BossBrain                                          =    88,
    MonstersSpawner                                    =    89,
    Shotgun                                            =  2001,
    Chaingun                                           =  2002,
    RocketLauncher                                     =  2003,
    PlasmaRifle                                        =  2004,
    Chainsaw                                           =  2005,
    BFG9000                                            =  2006,
    Clip                                               =  2007,
    ShotgunShells                                      =  2008,
    Rocket                                             =  2010,
    Stimpack                                           =  2011,
    Medikit                                            =  2012,
    SoulSphere                                         =  2013,
    HealthBonus                                        =  2014,
    ArmorBonus                                         =  2015,
    GreenArmor                                         =  2018,
    BlueArmor                                          =  2019,
    Invulnerability                                    =  2022,
    Berserk                                            =  2023,
    PartialInvisibility                                =  2024,
    RadiationShieldingSuit                             =  2025,
    ComputerAreaMap                                    =  2026,
    FloorLamp                                          =  2028,
    Barrel                                             =  2035,
    LightAmplificationVisor                            =  2045,
    BoxOfRockets                                       =  2046,
    Cell                                               =  2047,
    BoxOfBullets                                       =  2048,
    BoxOfShells                                        =  2049,
    Imp                                                =  3001,
    Demon                                              =  3002,
    BaronOfHell                                        =  3003,
    Zombieman                                          =  3004,
    Cacodemon                                          =  3005,
    LostSoul                                           =  3006,
    Pusher                                             =  5001,
    Puller                                             =  5002,
    MusicSource                                        = 14164,
    VisualModeCamera                                   = 32000
};

//
// A SubSector.
// References a Sector.
// Basically, this is a list of LineSegs,
//  indicating the visible walls that define
//  (all or some) sides of a convex BSP leaf.
//
typedef struct subsector_s
{
    sector_t            *sector;
    int                 numlines;
    int                 firstline;
} subsector_t;

// phares 3/14/98
//
// Sector list node showing all sectors an object appears in.
//
// There are two threads that flow through these nodes. The first thread
// starts at touching_thinglist in a sector_t and flows through the m_snext
// links to find all mobjs that are entirely or partially in the sector.
// The second thread starts at touching_sectorlist in an mobj_t and flows
// through the m_tnext links to find all sectors a thing touches. This is
// useful when applying friction or push effects to sectors. These effects
// can be done as thinkers that act upon all objects touching their sectors.
// As an mobj moves through the world, these nodes are created and
// destroyed, with the links changed appropriately.
//
// For the links, NULL means top or end of list.
//
typedef struct msecnode_s
{
    sector_t            *m_sector;      // a sector containing this object
    struct mobj_s       *m_thing;       // this object
    struct msecnode_s   *m_tprev;       // prev msecnode_t for this thing
    struct msecnode_s   *m_tnext;       // next msecnode_t for this thing
    struct msecnode_s   *m_sprev;       // prev msecnode_t for this sector
    struct msecnode_s   *m_snext;       // next msecnode_t for this sector
    dboolean            visited;        // killough 4/4/98, 4/7/98: used in search algorithms
} msecnode_t;

//
// The LineSeg.
//
typedef struct
{
    vertex_t            *v1;
    vertex_t            *v2;

    fixed_t             offset;

    angle_t             angle;

    int64_t             dx, dy;

    int64_t             length;

    side_t              *sidedef;
    line_t              *linedef;

    // Sector references.
    // Could be retrieved from linedef, too.
    // backsector is NULL for one sided lines
    sector_t            *frontsector;
    sector_t            *backsector;

    int                 fakecontrast;
} seg_t;

//
// BSP node.
//
typedef struct
{
    // Partition line.
    fixed_t             x, y;
    fixed_t             dx, dy;

    // Bounding box for each child.
    fixed_t             bbox[2][4];

    // If NF_SUBSECTOR its a subsector.
    int                 children[2];
} node_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

// posts are runs of non masked source pixels
typedef struct
{
    byte               topdelta;        // -1 is the last post in a column
    byte               length;          // length data bytes follows
} PACKEDATTR post_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

// column_t is a list of 0 or more post_t, (byte)-1 terminated
typedef post_t column_t;

//
// OTHER TYPES
//

// This could be wider for >8 bit display.
// Indeed, true color support is possible
//  precalculating 24bpp lightmap/colormap LUT.
//  from darkening PLAYPAL to all black.
// Could even use more than 32 levels.
typedef byte lighttable_t;

typedef struct
{
    seg_t               *curline;
    int                 x1;
    int                 x2;

    fixed_t             scale1;
    fixed_t             scale2;
    fixed_t             scalestep;

    fixed_t             minscale;
    fixed_t             maxscale;

    // 0=none, 1=bottom, 2=top, 3=both
    int                 silhouette;

    // Pointers to lists for sprite clipping,
    //  all three adjusted so [x1] is first value.
    int                 *sprtopclip;
    int                 *sprbottomclip;
    int                 *maskedtexturecol;
} drawseg_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.
typedef struct
{
    short               width;          // bounding box size
    short               height;
    short               leftoffset;     // pixels to the left of origin
    short               topoffset;      // pixels below the origin
    int                 columnofs[8];   // only [width] used
    // the [0] is &columnofs[width]
} PACKEDATTR patch_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

// A vissprite_t is a thing
//  that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
typedef struct
{
    int                 x1;
    int                 x2;

    // for line side calculation
    fixed_t             gx, gy;

    // global bottom/top for silhouette clipping
    fixed_t             gz;
    fixed_t             gzt;

    // horizontal position of x1
    fixed_t             startfrac;

    fixed_t             scale;

    // negative if flipped
    fixed_t             xiscale;

    fixed_t             texturemid;
    int                 patch;

    // for color translation and shadow draw,
    //  maxbright frames as well
    lighttable_t        *colormap[2];

    mobj_t              *mobj;

    void                (*colfunc)(void);

    // foot clipping
    fixed_t             footclip;

    // killough 3/27/98: height sector for underwater/fake ceiling support
    sector_t            *heightsec;

    int                 shadowpos;
} vissprite_t;

typedef struct
{
    int                 x1;
    int                 x2;
    fixed_t             gx, gy;
    fixed_t             startfrac;
    fixed_t             scale;
    fixed_t             xiscale;
    fixed_t             texturemid;
    int                 patch;
    lighttable_t        *colormap;
    void                (*colfunc)(void);
    fixed_t             blood;
} bloodsplatvissprite_t;

//
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre-drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//
typedef struct
{
    // If false use 0 for any position.
    // Note: as eight entries are available,
    //  we might as well insert the same name eight times.
    int                 rotate;

    // Lump to use for view angles 0-7.
    short               lump[16];

    // Flip bit (1 = flip) to use for view angles 0-15.
    unsigned short      flip;
} spriteframe_t;

//
// A sprite definition:
//  a number of animation frames.
//
typedef struct
{
    int                 numframes;
    spriteframe_t       *spriteframes;
} spritedef_t;

//
// Now what is a visplane, anyway?
//
typedef struct
{
    int                 picnum;
    int                 lightlevel;
    int                 left;
    int                 right;
    fixed_t             height;
    fixed_t             xoffs, yoffs;   // killough 2/28/98: Support scrolling flats

    // leave pads for [minx-1]/[maxx+1]
    unsigned short      pad1;

    // Here lies the rub for all
    //  dynamic resize/change of resolution.
    unsigned short      top[SCREENWIDTH];

    unsigned short      pad2;
    unsigned short      pad3;

    unsigned short      bottom[SCREENWIDTH];

    unsigned short      pad4;
} visplane_t;

#endif
