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

#if !defined(__P_SPEC_H__)
#define __P_SPEC_H__

// jff 2/23/98 identify the special classes that can share sectors
typedef enum
{
    floor_special,
    ceiling_special,
    lighting_special
} special_e;

extern dboolean *isliquid;
extern dboolean *isteleport;

// at game start
void P_InitPicAnims(void);

// at map load
void P_SetTimer(int minutes);
void P_SpawnSpecials(void);
void P_SetLiquids(void);

// every tic
void P_UpdateSpecials(void);

dboolean P_SectorActive(special_e t, sector_t *sec);
dboolean P_SectorHasLightSpecial(sector_t *sec);

dboolean P_CheckTag(line_t *line);

// when needed
dboolean P_UseSpecialLine(mobj_t *thing, line_t *line, int side);

void P_ShootSpecialLine(mobj_t *thing, line_t *line);

void P_CrossSpecialLine(line_t *line, int side, mobj_t *thing);

void P_PlayerInSpecialSector(player_t *player);

dboolean twoSided(int sector, int line);

sector_t *getSector(int currentSector, int line, int side);

side_t *getSide(int currentSector, int line, int side);

dboolean P_IsSelfReferencingSector(sector_t *sec);

fixed_t P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(sector_t *sec);

fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight);
fixed_t P_FindNextLowestFloor(sector_t *sec, int currentheight);

fixed_t P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec);

fixed_t P_FindNextLowestCeiling(sector_t *sec, int currentheight); // jff 2/04/98

fixed_t P_FindNextHighestCeiling(sector_t *sec, int currentheight); // jff 2/04/98

fixed_t P_FindShortestTextureAround(int secnum); // jff 2/04/98

fixed_t P_FindShortestUpperAround(int secnum); // jff 2/04/98

sector_t *P_FindModelFloorSector(fixed_t floordestheight, int secnum); // jff 02/04/98

sector_t *P_FindModelCeilingSector(fixed_t ceildestheight, int secnum); // jff 02/04/98

int P_FindSectorFromLineTag(const line_t *line, int start);
int P_FindLineFromLineTag(const line_t *line, int start);

int P_FindMinSurroundingLight(sector_t *sec, int min);

dboolean P_CanUnlockGenDoor(line_t *line, player_t *player);

sector_t *getNextSector(line_t *line, sector_t *sec);

//
// SPECIAL
//
dboolean EV_DoDonut(line_t *line);

//
// P_LIGHTS
//
typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    int         count;
    int         maxlight;
    int         minlight;
} fireflicker_t;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    int         count;
    int         maxlight;
    int         minlight;
    int         maxtime;
    int         mintime;
} lightflash_t;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    int         count;
    int         minlight;
    int         maxlight;
    int         darktime;
    int         brighttime;
} strobe_t;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    int         minlight;
    int         maxlight;
    int         direction;
} glow_t;

#define GLOWSPEED       8
#define STROBEBRIGHT    5
#define FASTDARK        15
#define SLOWDARK        35

void P_SpawnFireFlicker(sector_t *sector);
void T_LightFlash(lightflash_t *flash);
void P_SpawnLightFlash(sector_t *sector);
void T_StrobeFlash(strobe_t *flash);

void P_SpawnStrobeFlash(sector_t *sector, int fastOrSlow, dboolean inSync);

dboolean EV_StartLightStrobing(line_t *line);
dboolean EV_TurnTagLightsOff(line_t *line);

dboolean EV_LightTurnOn(line_t *line, int bright);

void EV_LightTurnOnPartway(line_t *line, fixed_t level);        // killough 10/10/98
void EV_LightByAdjacentSectors(sector_t *sector, fixed_t level);

void T_Glow(glow_t *g);
void P_SpawnGlowingLight(sector_t *sector);

void T_FireFlicker(fireflicker_t *flick);

//
// P_SWITCH
//

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

typedef struct
{
    char        name1[9];
    char        name2[9];
    short       episode;
} switchlist_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

typedef enum
{
    top,
    middle,
    bottom
} bwhere_e;

typedef struct
{
    line_t      *line;
    bwhere_e    where;
    int         btexture;
    int         btimer;
    degenmobj_t *soundorg;
} button_t;

// 4 players, 4 buttons each at once, max.
#define MAXBUTTONS  32

// 1 second, in ticks.
#define BUTTONTIME  35

extern button_t buttonlist[MAXBUTTONS];

void P_ChangeSwitchTexture(line_t *line, dboolean useAgain);

void P_InitSwitchList(void);

//
// P_PLATS
//
typedef enum
{
    up,
    down,
    waiting,
    in_stasis
} plat_e;

// jff 3/15/98 pure texture/type change for better generalized support
typedef enum
{
    trigChangeOnly,
    numChangeOnly
} change_e;

typedef enum
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS,
    genLift,            // jff added to support generalized Plat types
    genPerpetual,
    toggleUpDn
} plattype_e;

typedef struct
{
    thinker_t          thinker;
    sector_t           *sector;
    fixed_t            speed;
    fixed_t            low;
    fixed_t            high;
    int                wait;
    int                count;
    plat_e             status;
    plat_e             oldstatus;
    dboolean           crush;
    int                tag;
    plattype_e         type;

    struct platlist_s  *list;   // killough
} plat_t;

// New limit-free plat structure -- killough
typedef struct platlist_s
{
    plat_t             *plat;
    struct platlist_s  *next, **prev;
} platlist_t;

#define PLATWAIT    3
#define PLATSPEED   FRACUNIT

extern platlist_t      *activeplats;

void T_PlatRaise(plat_t *plat);

dboolean EV_DoPlat(line_t *line, plattype_e type, int amount);

void P_AddActivePlat(plat_t *plat);
void P_RemoveActivePlat(plat_t *plat);
void P_RemoveAllActivePlats(void);
dboolean EV_StopPlat(line_t *line);
void P_ActivateInStasis(int tag);

//
// P_DOORS
//
typedef enum
{
    doorNormal,
    doorClose30ThenOpen,
    doorClose,
    doorOpen,
    doorRaiseIn5Mins,
    doorBlazeRaise,
    doorBlazeOpen,
    doorBlazeClose,

    // jff 02/05/98 add generalized door types
    genRaise,
    genBlazeRaise,
    genOpen,
    genBlazeOpen,
    genClose,
    genBlazeClose,
    genCdO,
    genBlazeCdO
} vldoor_e;

typedef struct
{
    thinker_t   thinker;
    vldoor_e    type;
    sector_t    *sector;
    fixed_t     topheight;
    fixed_t     speed;

    // 1 = up, 0 = waiting at top, -1 = down
    int         direction;

    // tics to wait at the top
    int         topwait;

    // (keep in case a door going down is reset)
    // when it reaches 0, start going down
    int         topcountdown;

    // jff 1/31/98 keep track of line door is triggered by
    line_t      *line;

    // killough 10/98: sector tag for gradual lighting effects
    int         lighttag;
} vldoor_t;

#define VDOORSPEED  (FRACUNIT * 2)
#define VDOORWAIT   150

void EV_VerticalDoor(line_t *line, mobj_t *thing);

dboolean EV_DoDoor(line_t *line, vldoor_e type);

dboolean EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing);

void T_VerticalDoor(vldoor_t *door);
void P_SpawnDoorCloseIn30(sector_t *sec);

void P_SpawnDoorRaiseIn5Mins(sector_t *sec);

//
// P_CEILING
//
typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerToLowest,
    lowerToMaxFloor,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise,

    // jff 02/04/98 add types for generalized ceiling mover
    genCeiling,
    genCeilingChg,
    genCeilingChg0,
    genCeilingChgT,

    // jff 02/05/98 add types for generalized ceiling mover
    genCrusher,
    genSilentCrusher
} ceiling_e;

typedef struct
{
    thinker_t                   thinker;
    ceiling_e                   type;
    sector_t                    *sector;
    fixed_t                     bottomheight;
    fixed_t                     topheight;
    fixed_t                     speed;
    fixed_t                     oldspeed;
    dboolean                    crush;

    // jff 02/04/98 add these to support ceiling changers
    int                         newspecial;
    short                       texture;

    // 1 = up, 0 = waiting, -1 = down
    int                         direction;

    // ID
    int                         tag;
    int                         olddirection;
    struct ceilinglist_s        *list;          // jff 2/22/98 copied from killough's plats
} ceiling_t;

typedef struct ceilinglist_s
{
    ceiling_t                   *ceiling;
    struct ceilinglist_s        *next;
    struct ceilinglist_s        **prev;
} ceilinglist_t;

#define CEILSPEED               FRACUNIT

extern ceilinglist_t            *activeceilings;

dboolean EV_DoCeiling(line_t *line, ceiling_e type);

void T_MoveCeiling(ceiling_t *ceiling);
void P_AddActiveCeiling(ceiling_t *ceiling);
void P_RemoveActiveCeiling(ceiling_t *ceiling);
void P_RemoveAllActiveCeilings(void);
dboolean EV_CeilingCrushStop(line_t *line);
dboolean P_ActivateInStasisCeiling(line_t *line);

//
// P_FLOOR
//
typedef enum
{
    // lower floor to highest surrounding floor
    lowerFloor,

    // lower floor to lowest surrounding floor
    lowerFloorToLowest,

    // lower floor to highest surrounding floor VERY FAST
    turboLower,

    // raise floor to lowest surrounding CEILING
    raiseFloor,

    // raise floor to next highest surrounding floor
    raiseFloorToNearest,

    // jff 02/03/98 lower floor to next lowest neighbor
    lowerFloorToNearest,

    // jff 02/03/98 lower floor 24 absolute
    lowerFloor24,

    // jff 02/03/98 lower floor 32 absolute
    lowerFloor32Turbo,

    // raise floor to shortest height texture around it
    raiseToTexture,

    // lower floor to lowest surrounding floor
    //  and change floorpic
    lowerAndChange,

    raiseFloor24,

    // jff 02/03/98 raise floor 32 absolute
    raiseFloor32Turbo,

    raiseFloor24AndChange,
    raiseFloorCrush,

    // raise to next highest floor, turbo-speed
    raiseFloorTurbo,
    donutRaise,
    raiseFloor512,

    // jff 02/04/98  add types for generalized floor mover
    genFloor,
    genFloorChg,
    genFloorChg0,
    genFloorChgT,

    buildStair,
    genBuildStair
} floor_e;

typedef enum
{
    elevateUp,
    elevateDown,
    elevateCurrent
} elevator_e;

typedef enum
{
    build8,     // slowly build by 8
    turbo16     // quickly build by 16
} stair_e;

typedef struct
{
    thinker_t   thinker;
    floor_e     type;
    dboolean    crush;
    sector_t    *sector;
    int         direction;
    int         newspecial;
    short       texture;
    fixed_t     floordestheight;
    fixed_t     speed;
    dboolean    stopsound;
} floormove_t;

typedef struct
{
    thinker_t   thinker;
    elevator_e  type;
    sector_t    *sector;
    int         direction;
    fixed_t     floordestheight;
    fixed_t     ceilingdestheight;
    fixed_t     speed;
} elevator_t;

#define ELEVATORSPEED   (FRACUNIT * 4)
#define FLOORSPEED      FRACUNIT

typedef enum
{
    ok,
    crushed,
    pastdest
} result_e;

result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest, dboolean crush, int floorOrCeiling,
    int direction, dboolean elevator);
dboolean EV_BuildStairs(line_t *line, stair_e type);
dboolean EV_DoFloor(line_t *line, floor_e floortype);
dboolean EV_DoChange(line_t *line, change_e changetype);
dboolean EV_DoElevator(line_t *line, elevator_e elevtype);
void T_MoveFloor(floormove_t *floor);
void T_MoveElevator(elevator_t *elevator);

// killough 3/7/98: Add generalized scroll effects
typedef struct
{
    thinker_t   thinker;        // Thinker structure for scrolling
    fixed_t     dx, dy;         // (dx,dy) scroll speeds
    int         affectee;       // Number of affected sidedef, sector, tag, or whatever
    int         control;        // Control sector (-1 if none) used to control scrolling
    fixed_t     last_height;    // Last known height of control sector
    fixed_t     vdx, vdy;       // Accumulated velocity if accelerative
    int         accel;          // Whether it's accelerative

    enum
    {
        sc_side,
        sc_floor,
        sc_ceiling,
        sc_carry
    } type;                     // Type of scroll effect
} scroll_t;

void T_Scroll(scroll_t *s);

// phares 3/20/98: added new model of Pushers for push/pull effects

typedef struct
{
    thinker_t   thinker;        // Thinker structure for Pusher

    enum
    {
        p_push,
        p_pull,
        p_wind,
        p_current
    } type;

    mobj_t      *source;        // Point source if point pusher
    int         x_mag;          // X Strength
    int         y_mag;          // Y Strength
    int         magnitude;      // Vector strength for point pusher
    int         radius;         // Effective radius for point pusher
    int         x;              // X of point source if point pusher
    int         y;              // Y of point source if point pusher
    int         affectee;       // Number of affected sector
} pusher_t;

void T_Pusher(pusher_t *p);     // phares 3/20/98: Push thinker
mobj_t *P_GetPushThing(int s);

//
// P_TELEPT
//
dboolean EV_Teleport(line_t *line, int side, mobj_t *thing);
dboolean EV_SilentTeleport(line_t *line, int side, mobj_t *thing);
dboolean EV_SilentLineTeleport(line_t *line, int side, mobj_t *thing, dboolean reverse);

// jff 3/14/98 add bits and shifts for generalized sector types

#define DAMAGE_MASK             0x60
#define DAMAGE_SHIFT               5
#define SECRET_MASK             0x80
#define FRICTION_MASK          0x100
#define PUSH_MASK              0x200

// jff 02/04/98 Define masks, shifts, for fields in
// generalized linedef types
#define GenFloorBase          0x6000
#define GenCeilingBase        0x4000
#define GenDoorBase           0x3C00
#define GenLockedBase         0x3800
#define GenLiftBase           0x3400
#define GenStairsBase         0x3000
#define GenCrusherBase        0x2F80

#define TriggerType           0x0007
#define TriggerTypeShift           0

// define masks and shifts for the floor type fields
#define FloorCrush            0x1000
#define FloorChange           0x0C00
#define FloorTarget           0x0380
#define FloorDirection        0x0040
#define FloorModel            0x0020
#define FloorSpeed            0x0018

#define FloorCrushShift           12
#define FloorChangeShift          10
#define FloorTargetShift           7
#define FloorDirectionShift        6
#define FloorModelShift            5
#define FloorSpeedShift            3

// define masks and shifts for the ceiling type fields
#define CeilingCrush          0x1000
#define CeilingChange         0x0C00
#define CeilingTarget         0x0380
#define CeilingDirection      0x0040
#define CeilingModel          0x0020
#define CeilingSpeed          0x0018

#define CeilingCrushShift         12
#define CeilingChangeShift        10
#define CeilingTargetShift         7
#define CeilingDirectionShift      6
#define CeilingModelShift          5
#define CeilingSpeedShift          3

// define masks and shifts for the lift type fields
#define LiftTarget            0x0300
#define LiftDelay             0x00C0
#define LiftMonster           0x0020
#define LiftSpeed             0x0018

#define LiftTargetShift            8
#define LiftDelayShift             6
#define LiftSpeedShift             3

// define masks and shifts for the stairs type fields
#define StairIgnore           0x0200
#define StairDirection        0x0100
#define StairStep             0x00C0
#define StairMonster          0x0020
#define StairSpeed            0x0018

#define StairIgnoreShift           9
#define StairDirectionShift        8
#define StairStepShift             6
#define StairSpeedShift            3

// define masks and shifts for the crusher type fields
#define CrusherSilent         0x0040
#define CrusherMonster        0x0020
#define CrusherSpeed          0x0018

#define CrusherSilentShift         6
#define CrusherSpeedShift          3

// define masks and shifts for the door type fields
#define DoorDelay             0x0300
#define DoorMonster           0x0080
#define DoorKind              0x0060
#define DoorSpeed             0x0018

#define DoorDelayShift             8
#define DoorKindShift              5
#define DoorSpeedShift             3

// define masks and shifts for the locked door type fields
#define LockedNKeys           0x0200
#define LockedKey             0x01C0
#define LockedKind            0x0020
#define LockedSpeed           0x0018

#define LockedNKeysShift           9
#define LockedKeyShift             6
#define LockedKindShift            5
#define LockedSpeedShift           3

// define names for the TriggerType field of the general linedefs
enum
{
    WalkOnce,
    WalkMany,
    SwitchOnce,
    SwitchMany,
    GunOnce,
    GunMany,
    PushOnce,
    PushMany
};

// define names for the Speed field of the general linedefs
enum
{
    SpeedSlow,
    SpeedNormal,
    SpeedFast,
    SpeedTurbo
};

// define names for the Target field of the general floor
enum
{
    FtoHnF,
    FtoLnF,
    FtoNnF,
    FtoLnC,
    FtoC,
    FbyST,
    Fby24,
    Fby32
};

// define names for the Changer Type field of the general floor
enum
{
    FNoChg,
    FChgZero,
    FChgTxt,
    FChgTyp
};

// define names for the Change Model field of the general floor
enum
{
    FTriggerModel,
    FNumericModel
};

// define names for the Target field of the general ceiling
enum
{
    CtoHnC,
    CtoLnC,
    CtoNnC,
    CtoHnF,
    CtoF,
    CbyST,
    Cby24,
    Cby32
};

// define names for the Changer Type field of the general ceiling
enum
{
    CNoChg,
    CChgZero,
    CChgTxt,
    CChgTyp
};

// define names for the Change Model field of the general ceiling
enum
{
    CTriggerModel,
    CNumericModel
};

// define names for the Target field of the general lift
enum
{
    F2LnF,
    F2NnF,
    F2LnC,
    LnF2HnF
};

// define names for the door Kind field of the general ceiling
enum
{
    OdCDoor,
    ODoor,
    CdODoor,
    CDoor
};

// define names for the locked door Kind field of the general ceiling
enum
{
    AnyKey,
    RCard,
    BCard,
    YCard,
    RSkull,
    BSkull,
    YSkull,
    AllKeys
};

dboolean EV_DoGenFloor(line_t *line);
dboolean EV_DoGenCeiling(line_t *line);
dboolean EV_DoGenLift(line_t *line);
dboolean EV_DoGenStairs(line_t *line);
dboolean EV_DoGenCrusher(line_t *line);
dboolean EV_DoGenDoor(line_t *line);
dboolean EV_DoGenLockedDoor(line_t *line);

#endif
