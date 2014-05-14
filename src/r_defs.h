/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

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

#ifndef __R_DEFS__
#define __R_DEFS__

#include "p_mobj.h"

// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE        0
#define SIL_BOTTOM      1
#define SIL_TOP         2
#define SIL_BOTH        3

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

    // sound origin for switches/buttons
    degenmobj_t         soundorg;
} line_t;

typedef enum
{
    NoSpecial                                                     =   0,
    DR_OpenDoorWait4SecondsClose                                  =   1,
    W1_OpenDoorStayOpen                                           =   2,
    W1_CloseDoor                                                  =   3,
    W1_OpenDoorWait4SecondsClose                                  =   4,
    W1_SetFloorToLowestNeighbouringCeiling                        =   5,
    W1_StartFastCrusher                                           =   6,
    S1_RaiseStairsHeight8Units                                    =   7,
    W1_RaiseStairsHeight8Units                                    =   8,
    S1_LowerPillarRaiseDonutChangeDonutFloorTextureAndType        =   9,
    W1_LowerLiftWait3SecondsRise                                  =  10,
    S1_ExitLevel                                                  =  11,
    W1_LightsToMaximumNeighbouringLevel                           =  12,
    W1_LightsTo255                                                =  13,
    S1_RaiseFloorBy32UnitsChangeFloorTextureAndType               =  14,
    S1_RaiseFloorBy24UnitsChangeFloorTextureAndType               =  15,
    W1_CloseDoorWait30SecondsOpen                                 =  16,
    W1_StartLightsBlinkingEverySecond                             =  17,
    S1_RaiseFloorToNextFloor                                      =  18,
    W1_SetFloorToHighestNeighbouringFloor                         =  19,
    S1_RaiseFloorToNextFloorChangeFloorTextureAndType             =  20,
    S1_LowerLiftWait3SecondsRise                                  =  21,
    W1_RaiseFloorToNextFloorChangeFloorTextureAndType             =  22,
    S1_SetFloorToLowestNeighbouringFloor                          =  23,
    G1_SetFloorToLowestNeighbouringCeiling                        =  24,
    W1_StartSlowCrusher                                           =  25,
    DR_OpenDoorWait4SecondsCloseBlueKeyRequired                   =  26,
    DR_OpenDoorWait4SecondsCloseYellowKeyRequired                 =  27,
    DR_OpenDoorWait4SecondsCloseRedKeyRequired                    =  28,
    S1_OpenDoorWait4SecondsClose                                  =  29,
    W1_RaiseFloorBy64Units                                        =  30,
    D1_OpenDoorStayOpen                                           =  31,
    D1_OpenDoorStayOpenBlueKeyRequired                            =  32,
    D1_OpenDoorStayOpenRedKeyRequired                             =  33,
    D1_OpenDoorStayOpenYellowKeyRequired                          =  34,
    W1_LightsTo0                                                  =  35,
    W1_SetFloorTo8UnitsAboveHighestNeighbouringFloor              =  36,
    W1_SetFloorToLowestNeighbouringFloorChangeFloorTextureAndType =  37,
    W1_SetFloorToLowestNeighbouringFloor                          =  38,
    W1_TeleportToTaggedSectorContainingTeleportLanding            =  39,
    W1_RaiseCeilingToHighestNeighbouringCeiling                   =  40,
    S1_LowerCeilingToFloor                                        =  41,
    SR_LowerCeilingToFloorCloseDoor                               =  42,
    SR_LowerCeilingToFloor                                        =  43,
    W1_LowerCeilingTo8UnitsAboveFloor                             =  44,
    SR_SetFloorToHighestNeighbouringFloor                         =  45,
    G1_OpenDoorStayOpen                                           =  46,
    G1_RaiseFloorToNextFloorChangeFloorTextureAndType             =  47,
    MovingWallTextureToLeft                                       =  48,
    S1_StartSlowCrusher                                           =  49,
    S1_CloseDoor                                                  =  50,
    S1_ExitLevelAndGoToSecretLevel                                =  51,
    W1_ExitLevel                                                  =  52,
    W1_StartUpDownMovingFloor                                     =  53,
    W1_StopUpDownMovingFloor                                      =  54,
    S1_SetFloorTo8UnitsUnderLowestNeighbouringCeiling             =  55,
    W1_CrushFloorTo8UnitsUnderCeiling                             =  56,
    W1_StopCrusher                                                =  57,
    W1_RaiseFloorBy24Units                                        =  58,
    W1_RaiseFloorBy24UnitsChangeFloorTextureAndType               =  59,
    SR_SetFloorToLowestNeighbouringFloor                          =  60,
    SR_OpenDoorStayOpen                                           =  61,
    SR_LowerLiftWait3SecondsRise                                  =  62,
    SR_OpenDoorWait4SecondsClose                                  =  63,
    SR_SetFloorToLowestNeighbouringCeiling                        =  64,
    SR_SetFloorTo8UnitsUnderLowestNeighbouringCeiling             =  65,
    SR_RaiseFloorBy24UnitsChangeFloorTextureAndType               =  66,
    SR_RaiseFloorBy32UnitsChangeFloorTextureAndType               =  67,
    SR_RaiseFloorToNextFloorChangeFloorTextureAndType             =  68,
    SR_RaiseFloorToNextFloor                                      =  69,
    SR_SetFloorTo8UnitsAboveHighestNeighbouringFloor              =  70,
    S1_SetFloorTo8UnitsAboveHighestNeighbouringFloor              =  71,
    WR_LowerCeilingTo8UnitsAboveFloor                             =  72,
    WR_StartSlowCrusher                                           =  73,
    WR_StopCrusher                                                =  74,
    WR_CloseDoor                                                  =  75,
    WR_CloseDoorWait30SecondsOpen                                 =  76,
    WR_StartFastCrusher                                           =  77,
    WR_LightsToMinimumNeighbouringLevel                           =  78,
    WR_LightsTo0                                                  =  79,
    WR_LightsToMaximumNeighbouringLevel                           =  80,
    WR_LightsTo255                                                =  81,
    WR_SetFloorToLowestNeighbouringFloor                          =  82,
    WR_SetFloorToHighestNeighbouringFloor                         =  83,
    WR_SetFloorToLowestNeighbouringFloorChangeFloorTextureAndType =  84,
    WR_RaiseCeilingToHighestNeighbouringCeiling                   =  85,
    WR_OpenDoorStayOpen                                           =  86,
    WR_StartUpDownMovingFloor                                     =  87,
    WR_LowerLiftWait3SecondsRise                                  =  88,
    WR_StopUpDownMovingFloor                                      =  89,
    WR_OpenDoorWait4SecondsClose                                  =  90,
    WR_SetFloorToLowestNeighbouringCeiling                        =  91,
    WR_RaiseFloorBy24Units                                        =  92,
    WR_RaiseFloorBy24UnitsChangeFloorTextureAndType               =  93,
    WR_CrushFloorTo8UnderCeiling                                  =  94,
    WR_RaiseFloorToNextFloorChangeFloorTextureAndType             =  95,
    WR_RaiseFloorBy64Units                                        =  96,
    WR_TeleportToTaggedSectorContainingTeleportLanding            =  97,
    WR_SetFloorTo8UnitsAboveHighestNeighbouringFloor              =  98,
    SR_OpenFastDoorStayOpenBlueKeyRequired                        =  99,
    W1_RaiseFastStairsHeight16Units                               = 100,
    S1_SetFloorToLowestNeighbouringCeiling                        = 101,
    S1_SetFloorToHighestNeighbouringFloor                         = 102,
    S1_OpenDoorStayOpen                                           = 103,
    W1_LightsToMinimumNeighbouringLevel                           = 104,
    WR_OpenFastDoorWait4SecondsClose                              = 105,
    WR_OpenFastDoorStayOpen                                       = 106,
    WR_CloseFastDoor                                              = 107,
    W1_OpenFastDoorWait4SecondsClose                              = 108,
    W1_OpenFastDoorStayOpen                                       = 109,
    W1_CloseFastDoor                                              = 110,
    S1_OpenFastDoorWait4SecondsClose                              = 111,
    S1_OpenFastDoorStayOpen                                       = 112,
    S1_CloseFastDoor                                              = 113,
    SR_OpenFastDoorWait4SecondsClose                              = 114,
    SR_OpenFastDoorStayOpen                                       = 115,
    SR_CloseFastDoor                                              = 116,
    DR_OpenFastDoorWait4SecondsClose                              = 117,
    D1_OpenFastDoorStayOpen                                       = 118,
    W1_RaiseFloorToNextFloor                                      = 119,
    WR_LowerFastLiftWait3SecondsRise                              = 120,
    W1_LowerFastLiftWait3SecondsRise                              = 121,
    S1_LowerFastLiftWait3SecondsRise                              = 122,
    SR_LowerFastLiftWait3SecondsRise                              = 123,
    W1_ExitLevelAndGoToSecretLevel                                = 124,
    M1_TeleportToTaggedSectorContainingTeleportLanding            = 125,
    MR_TeleportToTaggedSectorContainingTeleportLanding            = 126,
    S1_RaiseFastStairsHeight16Units                               = 127,
    WR_RaiseFloorToNextFloor                                      = 128,
    WR_RaiseFastFloorToNextFloor                                  = 129,
    W1_RaiseFastFloorToNextFloor                                  = 130,
    S1_RaiseFastFloorToNextFloor                                  = 131,
    SR_RaiseFastFloorToNextFloor                                  = 132,
    S1_OpenFastDoorStayOpenBlueKeyRequired                        = 133,
    SR_OpenFastDoorStayOpenRedKeyRequired                         = 134,
    S1_OpenFastDoorStayOpenRedKeyRequired                         = 135,
    SR_OpenFastDoorStayOpenYellowKeyRequired                      = 136,
    S1_OpenFastDoorStayOpenYellowKeyRequired                      = 137,
    SR_LightsTo255                                                = 138,
    SR_LightsTo0                                                  = 139,
    S1_RaiseFloorBy512Units                                       = 140,
    W1_StartSlowQuietCrusher                                      = 141
} linespecial_t;

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
    FlickeringFire       = 17
} sectorspecial_t;

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
    FloatingSkull                                      =   42,
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

typedef struct msecnode_s
{
    sector_t          *m_sector; // a sector containing this object
    struct mobj_s     *m_thing;  // this object
    struct msecnode_s *m_tprev;  // prev msecnode_t for this thing
    struct msecnode_s *m_tnext;  // next msecnode_t for this thing
    struct msecnode_s *m_sprev;  // prev msecnode_t for this sector
    struct msecnode_s *m_snext;  // next msecnode_t for this sector
    boolean visited; // killough 4/4/98, 4/7/98: used in search algorithms
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

// posts are runs of non masked source pixels
typedef struct
{
    byte               topdelta;        // -1 is the last post in a column
    byte               length;          // length data bytes follows
} PACKEDATTR post_t;

// column_t is a list of 0 or more post_t, (byte)-1 terminated
typedef post_t column_t;

//
// OTHER TYPES
//

// This could be wider for >8 bit display.
// Indeed, true color support is posibble
//  precalculating 24bpp lightmap/colormap LUT.
//  from darkening PLAYPAL to all black.
// Could even us emore than 32 levels.
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

// A vissprite_t is a thing
//  that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
typedef struct vissprite_s
{
    // Doubly linked list.
    struct vissprite_s  *prev;
    struct vissprite_s  *next;

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
    boolean             rotate;

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
    unsigned short      top[SCREENWIDTH];
    unsigned short      pad2;
    unsigned short      pad3;
    // See above.
    unsigned short      bottom[SCREENWIDTH];
    unsigned short      pad4;

    struct visplane_s   *next;  // Next visplane in hash chain -- killough
} visplane_t;

#endif
