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

#if !defined(__R_DEFS__)
#define __R_DEFS__

#include "p_mobj.h"

// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE        0
#define SIL_BOTTOM      1
#define SIL_TOP         2
#define SIL_BOTH        3

#define MAXDRAWSEGS     256

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
    fixed_t             x;
    fixed_t             y;
    angle_t             viewangle;      // e6y: precalculated angle for clipping
    int                 angletime;      // e6y: recalculation time for view angle 
} vertex_t;

// Forward of LineDefs, for Sectors.
struct line_s;

// Each sector has a degenmobj_t in its center
//  for sound origin purposes.
// I suppose this does not handle sound from
//  moving objects (doppler), because
//  position is prolly just buffered, not
//  updated.
typedef struct
{
    thinker_t           thinker;        // not used for anything
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
} degenmobj_t;

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//
typedef struct
{
    fixed_t             floorheight;
    fixed_t             ceilingheight;
    int                 nexttag, firsttag;
    short               floorpic;
    short               ceilingpic;
    short               lightlevel;
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

    // thinker_t for reversable actions
    void                *specialdata;

    // list of mobjs that are at least partially in the sector
    // thinglist is a subset of touching_thinglist
    struct msecnode_s   *touching_thinglist;               // phares 3/14/98  

    int                 linecount;
    struct line_s       **lines;  // [linecount] size

    int                 cachedheight;
    int                 scaleindex;

    int                 animate;

    // [AM] Previous position of floor and ceiling before
    //      think. Used to interpolate between positions.
    fixed_t             oldfloorheight;
    fixed_t             oldceilingheight;

    // [AM] Gametic when the old positions were recorded.
    //      Has a dual purpose; it prevents movement thinkers
    //      from storing old positions twice in a tic, and
    //      prevents the renderer from attempting to interpolate
    //      if old values were not updated recently.
    int                 oldgametic;

    // [AM] Interpolated floor and ceiling height.
    //      Calculated once per tic and used inside
    //      the renderer.
    fixed_t             interpfloorheight;
    fixed_t             interpceilingheight;
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
    // Vertices, from v1 to v2.
    vertex_t            *v1;
    vertex_t            *v2;

    // Precalculated v2 - v1 for side checking.
    fixed_t             dx;
    fixed_t             dy;

    // Animation related.
    unsigned short      flags;

    boolean             hidden;

    short               special;
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

    // thinker_t for reversable actions
    void                *specialdata;

    int                 nexttag, firsttag;

    // sound origin for switches/buttons
    degenmobj_t         soundorg;
} line_t;

typedef enum
{
    NoSpecial                                                =   0,
    DR_Door_OpenWaitClose_AlsoMonsters                       =   1,
    W1_Door_OpenStay                                         =   2,
    W1_Door_CloseStay                                        =   3,
    W1_Door_OpenWaitClose                                    =   4,
    W1_Floor_RaiseToLowestCeiling                            =   5,
    W1_Crusher_StartWithFastDamage                           =   6,
    S1_Stairs_RaiseBy8                                       =   7,
    W1_Stairs_RaiseBy8                                       =   8,
    S1_Floor_RaiseDonutChangesTexture                        =   9,
    W1_Lift_LowerWaitRaise                                   =  10,
    S1_ExitLevel                                             =  11,
    W1_Light_ChangeToBrightestAdjacent                       =  12,
    W1_Light_ChangeTo255                                     =  13,
    S1_Floor_RaiseBy32_ChangesTexture                        =  14,
    S1_Floor_RaiseBy24_ChangesTexture                        =  15,
    W1_Door_CloseWaitOpen                                    =  16,
    W1_Light_StartBlinking                                   =  17,
    S1_Floor_RaiseToNextHighestFloor                         =  18,
    W1_Floor_LowerToHighestFloor                             =  19,
    S1_Floor_RaiseToNextHighestFloor_ChangesTexture          =  20,
    S1_Lift_LowerWaitRaise                                   =  21,
    W1_Floor_RaiseToNextHighestFloor_ChangesTexture          =  22,
    S1_Floor_LowerToLowestFloor                              =  23,
    G1_Floor_RaiseToLowestCeiling                            =  24,
    W1_Crusher_StartWithSlowDamage                           =  25,
    DR_Door_Blue_OpenWaitClose                               =  26,
    DR_Door_Yellow_OpenWaitClose                             =  27,
    DR_Door_Red_OpenWaitClose                                =  28,
    S1_Door_OpenWaitClose                                    =  29,
    W1_Floor_RaiseByShortestLowerTexture                     =  30,
    D1_Door_OpenStay                                         =  31,
    D1_Door_Blue_OpenStay                                    =  32,
    D1_Door_Red_OpenStay                                     =  33,
    D1_Door_Yellow_OpenStay                                  =  34,
    W1_Light_ChangeTo35                                      =  35,
    W1_Floor_LowerTo8AboveHighestFloor                       =  36,
    W1_Floor_LowerToLowestFloor_ChangesTexture               =  37,
    W1_Floor_LowerToLowestFloor                              =  38,
    W1_Teleport                                              =  39,
    W1_Ceiling_RaiseToHighestCeiling                         =  40,
    S1_Ceiling_LowerToFloor                                  =  41,
    SR_Door_CloseStay                                        =  42,
    SR_Ceiling_LowerToFloor                                  =  43,
    W1_Ceiling_LowerTo8AboveFloor                            =  44,
    SR_Floor_LowerToHighestFloor                             =  45,
    GR_Door_OpenStay                                         =  46,
    G1_Floor_RaiseToNextHighestFloor_ChangesTexture          =  47,
    ScrollTextureLeft                                        =  48,
    S1_Ceiling_LowerTo8AboveFloor_PerpetualSlowCrusherDamage =  49,
    S1_Door_CloseStay                                        =  50,
    S1_ExitLevel_GoesToSecretLevel                           =  51,
    W1_ExitLevel                                             =  52,
    W1_Floor_StartMovingUpAndDown                            =  53,
    W1_Floor_StopMoving                                      =  54,
    S1_Floor_RaiseTo8BelowLowestCeiling_Crushes              =  55,
    W1_Floor_RaiseTo8BelowLowestCeiling_Crushes              =  56,
    W1_Crusher_Stop                                          =  57,
    W1_Floor_RaiseBy24                                       =  58,
    W1_Floor_RaiseBy24_ChangesTexture                        =  59,
    SR_Floor_LowerToLowestFloor                              =  60,
    SR_Door_OpenStay                                         =  61,
    SR_Lift_LowerWaitRaise                                   =  62,
    SR_Door_OpenWaitClose                                    =  63,
    SR_Floor_RaiseToLowestCeiling                            =  64,
    SR_Floor_RaiseTo8BelowLowestCeiling_Crushes              =  65,
    SR_Floor_RaiseBy24_ChangesTexture                        =  66,
    SR_Floor_RaiseBy32_ChangesTexture                        =  67,
    SR_Floor_RaiseToNextHighestFloor_ChangesTexture          =  68,
    SR_Floor_RaiseToNextHighestFloor                         =  69,
    SR_Floor_LowerTo8AboveHighestFloor                       =  70,
    S1_Floor_LowerTo8AboveHighestFloor                       =  71,
    WR_Ceiling_LowerTo8AboveFloor                            =  72,
    WR_Crusher_StartWithSlowDamage                           =  73,
    WR_Crusher_Stop                                          =  74,
    WR_Door_CloseStay                                        =  75,
    WR_Door_CloseStayOpen                                    =  76,
    WR_Crusher_StartWithFastDamage                           =  77,
    SR_ChangeTextureAndEffectToNearest                       =  78,
    WR_Light_ChangeTo35                                      =  79,
    WR_Light_ChangeToBrightestAdjacent                       =  80,
    WR_Light_ChangeTo255                                     =  81,
    WR_Floor_LowerToLowestFloor                              =  82,
    WR_Floor_LowerToHighestFloor                             =  83,
    WR_Floor_LowerToLowestFloor_ChangesTexture               =  84,
    ScrollTextureRight                                       =  85,
    WR_Door_OpenStay                                         =  86,
    WR_Floor_StartMovingUpAndDown                            =  87,
    WR_Lift_LowerWaitRaise                                   =  88,
    WR_Floor_StopMoving                                      =  89,
    WR_Door_OpenWaitClose                                    =  90,
    WR_Floor_RaiseToLowestCeiling                            =  91,
    WR_Floor_RaiseBy24                                       =  92,
    WR_Floor_RaiseBy24_ChangesTexture                        =  93,
    WR_Floor_RaiseTo8BelowLowestCeiling_Crushes              =  94,
    WR_Floor_RaiseToNextHighestFloor_ChangesTexture          =  95,
    WR_Floor_RaiseByShortestLowerTexture                     =  96,
    WR_Teleport                                              =  97,
    WR_Floor_LowerTo8AboveHighestFloor                       =  98,
    SR_Door_Blue_OpenStay_Fast                               =  99,
    W1_Stairs_RaiseBy16_Fast                                 = 100,
    S1_Floor_RaiseToLowestCeiling                            = 101,
    S1_Floor_LowerToHighestFloor                             = 102,
    S1_Door_OpenStay                                         = 103,
    W1_Light_ChangeToDarkestAdjacent                         = 104,
    WR_Door_OpenWaitClose_Fast                               = 105,
    WR_Door_OpenStay_Fast                                    = 106,
    WR_Door_CloseStay_Fast                                   = 107,
    W1_Door_OpenWaitClose_Fast                               = 108,
    W1_Door_OpenStay_Fast                                    = 109,
    W1_Door_CloseStay_Fast                                   = 110,
    S1_Door_OpenWaitClose_Fast                               = 111,
    S1_Door_OpenStay_Fast                                    = 112,
    S1_Door_CloseStay_Fast                                   = 113,
    SR_Door_OpenWaitClose_Fast                               = 114,
    SR_Door_OpenStay_Fast                                    = 115,
    SR_Door_CloseStay_Fast                                   = 116,
    DR_Door_OpenWaitClose_Fast                               = 117,
    D1_Door_OpenStay_Fast                                    = 118,
    W1_Floor_RaiseToNextHighestFloor                         = 119,
    WR_Lift_LowerWaitRaise_Fast                              = 120,
    W1_Lift_LowerWaitRaise_Fast                              = 121,
    S1_Lift_LowerWaitRaise_Fast                              = 122,
    SR_Lift_LowerWaitRaise_Fast                              = 123,
    W1_ExitLevel_GoesToSecretLevel                           = 124,
    W1_Teleport_MonstersOnly                                 = 125,
    WR_Teleport_MonstersOnly                                 = 126,
    S1_Stairs_RaiseBy16_Fast                                 = 127,
    WR_Floor_RaiseToNextHighestFloor                         = 128,
    WR_Floor_RaiseToNextHighestFloor_Fast                    = 129,
    W1_Floor_RaiseToNextHighestFloor_Fast                    = 130,
    S1_Floor_RaiseToNextHighestFloor_Fast                    = 131,
    SR_Floor_RaiseToNextHighestFloor_Fast                    = 132,
    S1_Door_Blue_OpenStay_Fast                               = 133,
    SR_Door_Red_OpenStay_Fast                                = 134,
    S1_Door_Red_OpenStay_Fast                                = 135,
    SR_Door_Yellow_OpenStay_Fast                             = 136,
    S1_Door_Yellow_OpenStay_Fast                             = 137,
    SR_Light_ChangeTo255                                     = 138,
    SR_Light_ChangeTo35                                      = 139,
    S1_Floor_RaiseBy512                                      = 140,
    W1_Crusher_StartWithSlowDamage_Silent                    = 141,

    UNKNOWNLINESPECIAL
} linespecial_e;

typedef enum
{
    FlickeringLights     =  1,
    StrobeFast           =  2,
    StrobeSlow           =  3,
    StrobeHurt           =  4,
    HellslimeDamage      =  5,
    NukageDamage         =  7,
    GlowingLight         =  8,
    SecretSector         =  9,
    CloseDoorIn30Seconds = 10,
    ExitSuperDamage      = 11,
    SyncStrobeSlow       = 12,
    SyncStrobeFast       = 13,
    RaiseDoorIn5Minutes  = 14,
    SuperHellslimeDamage = 16,
    FlickeringFire       = 17,

    UNKNOWNSECTORSPECIAL
} sectorspecial_e;

typedef enum
{
    Player1Start                                       =    1,
    Player2Start                                       =    2,
    Player3Start                                       =    3,
    Player4Start                                       =    4,
    BlueKeycard                                        =    5,
    YellowKeycard                                      =    6,
    SpiderMastermind                                   =    7,
    Backpack                                           =    8,
    ShotgunGuy                                         =    9,
    BloodyMess1                                        =   10,
    PlayerDeathmatchStart                              =   11,
    BloodyMess2                                        =   12,
    RedKeycard                                         =   13,
    TeleportDestination                                =   14,
    DeadPlayer                                         =   15,
    Cyberdemon                                         =   16,
    CellPack                                           =   17,
    DeadZombieman                                      =   18,
    DeadShotgunGuy                                     =   19,
    DeadImp                                            =   20,
    DeadDemon                                          =   21,
    DeadCacodemon                                      =   22,
    DeadLostSoulInvisible                              =   23,
    PoolOfBloodAndBones                                =   24,
    ImpaledHuman                                       =   25,
    TwitchingImpaledHuman                              =   26,
    SkullOnAPole                                       =   27,
    FiveSkullsShishKebab                               =   28,
    PileOfSkullsAndCandles                             =   29,
    TallGreenColumn                                    =   30,
    ShortGreenColumn                                   =   31,
    TallRedColumn                                      =   32,
    ShortRedColumn                                     =   33,
    Candlestick                                        =   34,
    Candelabra                                         =   35,
    ShortGreenColumnWithBeatingHeart                   =   36,
    ShortRedColumnWithSkull                            =   37,
    RedSkullKey                                        =   38,
    YellowSkullKey                                     =   39,
    BlueSkullKey                                       =   40,
    EvilEye                                            =   41,
    FloatingSkullRock                                  =   42,
    TorchedTree                                        =   43,
    TallBlueFirestick                                  =   44,
    TallGreenFirestick                                 =   45,
    TallRedFirestick                                   =   46,
    Stalagmite                                         =   47,
    TallTechnoPillar                                   =   48,
    HangingVictimTwitchingBlocking                     =   49,
    HangingVictimArmsOutBlocking                       =   50,
    HangingVictimOneLeggedBlocking                     =   51,
    HangingPairOfLegsBlocking                          =   52,
    HangingLegBlocking                                 =   53,
    LargeBrownTree                                     =   54,
    ShortBlueFirestick                                 =   55,
    ShortGreenFirestick                                =   56,
    ShortRedFirestick                                  =   57,
    Spectre                                            =   58,
    HangingVictimArmsOut                               =   59,
    HangingPairOfLegs                                  =   60,
    HangingVictimOneLegged                             =   61,
    HangingLeg                                         =   62,
    HangingVictimTwitching                             =   63,
    ArchVile                                           =   64,
    HeavyWeaponDude                                    =   65,
    Revenant                                           =   66,
    Mancubus                                           =   67,
    Arachnotron                                        =   68,
    HellKnight                                         =   69,
    BurningBarrel                                      =   70,
    PainElemental                                      =   71,
    CommanderKeen                                      =   72,
    HangingVictimGutsRemoved                           =   73,
    HangingVictimGutsAndBrainRemoved                   =   74,
    HangingTorsoLookingDown                            =   75,
    HangingTorsoOpenSkull                              =   76,
    HangingTorsoLookingUp                              =   77,
    HangingTorsoBrainRemoved                           =   78,
    PoolOfBloodAndGuts                                 =   79,
    PoolOfBlood                                        =   80,
    PoolOfBrains                                       =   81,
    SuperShotgun                                       =   82,
    MegaSphere                                         =   83,
    WolfensteinSS                                      =   84,
    TallTechnoFloorLamp                                =   85,
    ShortTechnoFloorLamp                               =   86,
    MonstersTarget                                     =   87,
    BossBrain                                          =   88,
    MonstersSpawner                                    =   89,
    Shotgun                                            = 2001,
    Chaingun                                           = 2002,
    RocketLauncher                                     = 2003,
    PlasmaRifle                                        = 2004,
    Chainsaw                                           = 2005,
    BFG9000                                            = 2006,
    Clip                                               = 2007,
    ShotgunShells                                      = 2008,
    Rocket                                             = 2010,
    Stimpack                                           = 2011,
    Medikit                                            = 2012,
    SoulSphere                                         = 2013,
    HealthBonus                                        = 2014,
    ArmorBonus                                         = 2015,
    GreenArmor                                         = 2018,
    BlueArmor                                          = 2019,
    Invulnerability                                    = 2022,
    Berserk                                            = 2023,
    PartialInvisibility                                = 2024,
    RadiationShieldingSuit                             = 2025,
    ComputerAreaMap                                    = 2026,
    FloorLamp                                          = 2028,
    Barrel                                             = 2035,
    LightAmplificationVisor                            = 2045,
    BoxOfRockets                                       = 2046,
    Cell                                               = 2047,
    BoxOfBullets                                       = 2048,
    BoxOfShells                                        = 2049,
    Imp                                                = 3001,
    Demon                                              = 3002,
    BaronOfHell                                        = 3003,
    Zombieman                                          = 3004,
    Cacodemon                                          = 3005,
    LostSoul                                           = 3006
} thingtype_t;

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

    fixed_t             length;

    side_t              *sidedef;
    line_t              *linedef;

    // Sector references.
    // Could be retrieved from linedef, too.
    // backsector is NULL for one sided lines
    sector_t            *frontsector;
    sector_t            *backsector;
} seg_t;

//
// BSP node.
//
typedef struct
{
    // Partition line.
    fixed_t             x;
    fixed_t             y;
    fixed_t             dx;
    fixed_t             dy;

    // Bounding box for each child.
    fixed_t             bbox[2][4];

    // If NF_SUBSECTOR its a subsector.
    int                 children[2];
} node_t;

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

// posts are runs of non masked source pixels
typedef struct
{
    byte               topdelta;        // -1 is the last post in a column
    byte               length;          // length data bytes follows
} PACKEDATTR post_t;

#if defined(_MSC_VER)
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

typedef struct drawseg_s
{
    seg_t               *curline;
    int                 x1;
    int                 x2;

    fixed_t             scale1;
    fixed_t             scale2;
    fixed_t             scalestep;

    // 0=none, 1=bottom, 2=top, 3=both
    int                 silhouette;

    // do not clip sprites above this
    fixed_t             bsilheight;

    // do not clip sprites below this
    fixed_t             tsilheight;

    // Pointers to lists for sprite clipping,
    //  all three adjusted so [x1] is first value.
    int                 *sprtopclip;
    int                 *sprbottomclip;
    int                 *maskedtexturecol;
} drawseg_t;

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
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

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

// A vissprite_t is a thing
//  that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
typedef struct vissprite_s
{
    int                 x1;
    int                 x2;

    // for line side calculation
    fixed_t             gx;
    fixed_t             gy;

    // global bottom / top for silhouette clipping
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
    lighttable_t        *colormap;

    int                 mobjflags;
    int                 mobjflags2;

    mobjtype_t          type;

    void                (*colfunc)(void);

    // foot clipping
    fixed_t             footclip;

    fixed_t             blood;

    boolean             drawn;
} vissprite_t;

//
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
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
    short               lump[8];

    // Flip bit (1 = flip) to use for view angles 0-7.
    byte                flip[8];
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
typedef struct visplane_s
{
    fixed_t             height;
    int                 picnum;
    int                 lightlevel;
    int                 minx;
    int                 maxx;

    // leave pads for [minx-1]/[maxx+1]
    unsigned short      pad1;
    // Here lies the rub for all
    //  dynamic resize/change of resolution.
    unsigned int        top[SCREENWIDTH];
    unsigned int        pad2;
    unsigned int        pad3;
    // See above.
    unsigned int        bottom[SCREENWIDTH];
    unsigned int        pad4;

    struct visplane_s   *next;  // Next visplane in hash chain -- killough

    sector_t            *sector;
} visplane_t;

#endif
