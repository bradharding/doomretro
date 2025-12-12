/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

#pragma once

// jff 02/23/98 identify the special classes that can share sectors
typedef enum
{
    floor_special,
    ceiling_special
} special_e;

#define ANIMATEDLIQUIDDIFFS 64
#define TIMERMAXMINUTES     (24 * 60)

extern int              timer;
extern int              timeremaining;
extern int              animatedtic;
extern fixed_t          animatedliquiddiff;
extern fixed_t          animatedliquidxdir;
extern fixed_t          animatedliquidydir;
extern fixed_t          animatedliquidxoffs;
extern fixed_t          animatedliquidyoffs;
extern const fixed_t    animatedliquiddiffs[ANIMATEDLIQUIDDIFFS];

extern terraintype_t    *terraintypes;
extern bool             *isteleport;

extern bool             zerotag_manual;

// at game start
void P_InitPicAnims(void);

// at map load
void P_SetTimer(const int minutes);
void P_SpawnSpecials(void);
void P_SetLiquids(void);
void P_FindLifts(void);

// every tic
void P_UpdateSpecials(void);

bool P_SectorActive(const special_e t, const sector_t *sec);

bool P_CheckTag(const line_t *line);

// when needed
bool P_UseSpecialLine(mobj_t *thing, line_t *line, const int side, const bool bossaction);

void P_ShootSpecialLine(const mobj_t *thing, line_t *line, const int side);

void P_CrossSpecialLine(line_t *line, const int side, mobj_t *thing, const bool bossaction);

void P_PlayerInSpecialSector(sector_t *sector);

bool P_TwoSided(const int sector, const int line);

sector_t *P_GetSector(const int currentsector, const int line, const int side);

side_t *P_GetSide(const int currentsector, const int line, const int side);

fixed_t P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(sector_t *sec);

fixed_t P_FindNextHighestFloor(sector_t *sec, const int currentheight);
fixed_t P_FindNextLowestFloor(sector_t *sec, const int currentheight);

fixed_t P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec);

// jff 02/04/98
fixed_t P_FindNextLowestCeiling(sector_t *sec, const int currentheight);
fixed_t P_FindNextHighestCeiling(sector_t *sec, const int currentheight);
fixed_t P_FindShortestTextureAround(const int secnum);
fixed_t P_FindShortestUpperAround(const int secnum);
sector_t *P_FindModelFloorSector(const fixed_t floordestheight, const int secnum);
sector_t *P_FindModelCeilingSector(const fixed_t ceildestheight, const int secnum);

int P_FindSectorFromLineTag(const line_t *line, int start);
int P_FindLineFromLineTag(const line_t *line, int start);

void P_InitTagLists(void);

int P_FindMinSurroundingLight(sector_t *sec, int min);

bool P_CanUnlockGenDoor(const line_t *line);

sector_t *P_GetNextSector(line_t *line, const sector_t *sec);

bool P_ProcessNoTagLines(const line_t *line, sector_t **sec, int *secnum);

//
// SPECIAL
//
bool EV_DoDonut(const line_t *line);

//
// P_LIGHTS.C
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
void T_StrobeFlash(strobe_t *strobe);

void P_SpawnStrobeFlash(sector_t *sector, int fastorslow, bool insync);

bool EV_StartLightStrobing(const line_t *line);
bool EV_TurnTagLightsOff(const line_t *line);

bool EV_LightTurnOn(const line_t *line, int bright);

void EV_LightTurnOnPartway(const line_t *line, fixed_t level);        // killough 10/10/98
void EV_LightByAdjacentSectors(sector_t *sector, fixed_t level);

void T_Glow(glow_t *glow);
void P_SpawnGlowingLight(sector_t *sector);

void T_FireFlicker(fireflicker_t *flick);

//
// P_SWITCH.C
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
    nowhere = -1,
    top,
    middle,
    bottom
} bwhere_e;

typedef struct
{
    line_t      *line;
    bwhere_e    bwhere;
    int         btexture;
    int         btimer;
    degenmobj_t *soundorg;
} button_t;

#define MAXBUTTONS  32

// 1 second, in tics.
#define BUTTONTIME  35

extern button_t *buttonlist;
extern int      maxbuttons;

void P_InitSwitchList(void);
void P_StartButton(line_t *line, bwhere_e bwhere, int texture, int time);
void P_ChangeSwitchTexture(line_t *line, bool useagain);

//
// P_PLATS.C
//
typedef enum
{
    up,
    down,
    waiting,
    in_stasis
} plat_e;

// jff 03/15/98 pure texture/type change for better generalized support
typedef enum
{
    TrigChangeOnly,
    NumChangeOnly
} change_e;

typedef enum
{
    PerpetualRaise,
    DownWaitUpStay,
    RaiseAndChange,
    RaiseToNearestAndChange,
    BlazeDWUS,
    GenLift,            // jff added to support generalized Plat types
    GenPerpetual,
    ToggleUpDn
} plattype_e;

typedef struct
{
    thinker_t           thinker;
    sector_t            *sector;
    fixed_t             speed;
    fixed_t             low;
    fixed_t             high;
    int                 wait;
    int                 count;
    plat_e              status;
    plat_e              oldstatus;
    bool                crush;
    int                 tag;
    plattype_e          type;

    struct platlist_s   *list;  // killough
} plat_t;

// New limit-free plat structure -- killough
typedef struct platlist_s
{
    plat_t              *plat;
    struct platlist_s   *next;
    struct platlist_s   **prev;
} platlist_t;

#define PLATWAIT    3
#define PLATSPEED   FRACUNIT

extern platlist_t   *activeplats;

void T_PlatStay(plat_t *plat);
void T_PlatRaise(plat_t *plat);

bool EV_DoPlat(line_t *line, plattype_e type, int amount);

void P_AddActivePlat(plat_t *plat);
void P_RemoveActivePlat(plat_t *plat);
void P_RemoveAllActivePlats(void);
bool EV_StopPlat(const line_t *line);
void P_ActivateInStasis(int tag);

//
// P_DOORS.C
//
typedef enum
{
    DoorNormal,
    DoorClose30ThenOpen,
    DoorClose,
    DoorOpen,
    DoorRaiseIn5Mins,
    DoorBlazeRaise,
    DoorBlazeOpen,
    DoorBlazeClose,

    // jff 02/05/98 add generalized door types
    GenRaise,
    GenBlazeRaise,
    GenOpen,
    GenBlazeOpen,
    GenClose,
    GenBlazeClose,
    GenCdO,
    GenBlazeCdO
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

    // jff 01/31/98 keep track of line door is triggered by
    line_t      *line;

    // killough 10/98: sector tag for gradual lighting effects
    int         lighttag;
} vldoor_t;

#define VDOORSPEED  (2 * FRACUNIT)
#define VDOORWAIT   150

void EV_VerticalDoor(line_t *line, mobj_t *thing);

bool EV_DoDoor(line_t *line, vldoor_e type, fixed_t speed);

bool EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing, fixed_t speed);

void T_VerticalDoor(vldoor_t *door);
void P_SpawnDoorCloseIn30(sector_t *sec);

void P_SpawnDoorRaiseIn5Mins(sector_t *sec);

//
// P_CEILING.C
//
typedef enum
{
    LowerToFloor,
    RaiseToHighest,
    LowerToLowest,
    LowerToMaxFloor,
    LowerAndCrush,
    CrushAndRaise,
    FastCrushAndRaise,
    SilentCrushAndRaise,

    // jff 02/04/98 add types for generalized ceiling mover
    GenCeiling,
    GenCeilingChg,
    GenCeilingChg0,
    GenCeilingChgT,

    // jff 02/05/98 add types for generalized ceiling mover
    GenCrusher,
    GenSilentCrusher
} ceiling_e;

typedef struct
{
    thinker_t               thinker;
    ceiling_e               type;
    sector_t                *sector;
    fixed_t                 bottomheight;
    fixed_t                 topheight;
    fixed_t                 speed;
    fixed_t                 oldspeed;
    bool                    crush;

    // jff 02/04/98 add these to support ceiling changers
    int                     newspecial;
    short                   texture;

    // 1 = up, 0 = waiting, -1 = down
    int                     direction;

    // ID
    int                     tag;
    int                     olddirection;
    struct ceilinglist_s    *list;          // jff 02/22/98 copied from killough's plats
} ceiling_t;

typedef struct ceilinglist_s
{
    ceiling_t               *ceiling;
    struct ceilinglist_s    *next;
    struct ceilinglist_s    **prev;
} ceilinglist_t;

#define CEILSPEED   FRACUNIT

extern ceilinglist_t    *activeceilings;

bool EV_DoCeiling(const line_t *line, ceiling_e type);

void T_CeilingStay(ceiling_t *ceiling);
void T_MoveCeiling(ceiling_t *ceiling);
void P_AddActiveCeiling(ceiling_t *ceiling);
void P_RemoveActiveCeiling(ceiling_t *ceiling);
void P_RemoveAllActiveCeilings(void);
bool EV_CeilingCrushStop(const line_t *line);
bool P_ActivateInStasisCeiling(const line_t *line);

//
// P_FLOOR.C
//
typedef enum
{
    // lower floor to highest surrounding floor
    LowerFloor,

    // lower floor to lowest surrounding floor
    LowerFloorToLowest,

    // lower floor to highest surrounding floor VERY FAST
    TurboLower,

    // raise floor to lowest surrounding CEILING
    RaiseFloor,

    // raise floor to next highest surrounding floor
    RaiseFloorToNearest,

    // jff 02/03/98 lower floor to next lowest neighbor
    LowerFloorToNearest,

    // jff 02/03/98 lower floor 24 absolute
    LowerFloor24,

    // jff 02/03/98 lower floor 32 absolute
    LowerFloor32Turbo,

    // raise floor to shortest height texture around it
    RaiseToTexture,

    // lower floor to lowest surrounding floor
    //  and change floorpic
    LowerAndChange,

    RaiseFloor24,

    // jff 02/03/98 raise floor 32 absolute
    RaiseFloor32Turbo,

    RaiseFloor24AndChange,
    RaiseFloorCrush,

    // raise to next highest floor, turbo-speed
    RaiseFloorTurbo,
    DonutRaise,
    RaiseFloor512,

    // jff 02/04/98  add types for generalized floor mover
    GenFloor,
    GenFloorChg,
    GenFloorChg0,
    GenFloorChgT,

    BuildStair,
    GenBuildStair
} floor_e;

typedef enum
{
    ElevateUp,
    ElevateDown,
    ElevateCurrent
} elevator_e;

typedef struct
{
    thinker_t   thinker;
    floor_e     type;
    bool        crush;
    sector_t    *sector;
    int         direction;
    int         newspecial;
    short       texture;
    fixed_t     floordestheight;
    fixed_t     speed;
    bool        stopsound;
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
    bool        stopsound;
} elevator_t;

#define ELEVATORSPEED   (4 * FRACUNIT)
#define FLOORSPEED      FRACUNIT

typedef enum
{
    ok,
    crushed,
    pastdest
} result_e;

result_e T_MovePlane(sector_t *sector, const fixed_t speed, fixed_t dest,
    const bool crush, const int floororceiling, const int direction);
bool EV_BuildStairs(const line_t *line, const fixed_t speed, const fixed_t stairsize, const bool crushing);
bool EV_DoFloor(const line_t *line, const floor_e floortype);
bool EV_DoChange(const line_t *line, const change_e changetype);
bool EV_DoElevator(const line_t *line, const elevator_e elevtype);
void T_MoveFloor(floormove_t *floor);
void T_MoveElevator(elevator_t *elevator);
void P_CheckTerrainType(sector_t *sector);

// Amount (dx,dy) vector linedef is shifted to get scroll amount
#define SCROLL_SHIFT    5

// Factor to scale scrolling effect into mobj-carrying properties = 3/32.
// (This is so scrolling floors and objects on them can move at same speed.)
#define CARRYFACTOR     ((fixed_t)(0.09375 * FRACUNIT))

typedef enum
{
    sc_side,
    sc_floor,
    sc_ceiling,
    sc_carry
} scroll_e;

// killough 03/07/98: Add generalized scroll effects
typedef struct
{
    thinker_t   thinker;        // Thinker structure for scrolling
    fixed_t     dx, dy;         // (dx,dy) scroll speeds
    int         affectee;       // Number of affected sidedef, sector, tag, or whatever
    int         control;        // Control sector (-1 if none) used to control scrolling
    fixed_t     lastheight;     // Last known height of control sector
    fixed_t     vdx, vdy;       // Accumulated velocity if accelerative
    bool        accel;          // Whether it's accelerative
    scroll_e    type;           // Type of scroll effect
} scroll_t;

void T_Scroll(scroll_t *scroller);

typedef enum
{
    p_push,
    p_pull,
    p_wind,
    p_current
} pusher_e;

// phares 03/20/98: Added new model of pushers for push/pull effects
typedef struct
{
    thinker_t   thinker;        // Thinker structure for pusher
    pusher_e    type;           // Type of pusher
    mobj_t      *source;        // Point source if point pusher
    int         x_mag;          // X Strength
    int         y_mag;          // Y Strength
    int         magnitude;      // Vector strength for point pusher
    int         radius;         // Effective radius for point pusher
    int         x;              // X of point source if point pusher
    int         y;              // Y of point source if point pusher
    int         affectee;       // Number of affected sector
} pusher_t;

void T_Pusher(pusher_t *pusher);    // phares 03/20/98: Push thinker
mobj_t *P_GetPushThing(int s);

//
// P_TELEPT.C
//
bool EV_Teleport(const line_t *line, const int side, mobj_t *thing);
bool EV_SilentTeleport(const line_t *line, const int side, mobj_t *thing);
bool EV_SilentLineTeleport(const line_t *line, int side, mobj_t *thing, const bool reverse);

// jff 03/14/98 add bits and shifts for generalized sector types
#define DAMAGE_MASK             0x0060
#define DAMAGE_SHIFT            5
#define SECRET_MASK             0x0080
#define FRICTION_MASK           0x0100
#define PUSH_MASK               0x0200

// reserved by BOOM spec - not implemented?
// bit 10: suppress all sounds within the sector
// bit 11: disable any sounds due to floor or ceiling motion by the sector

// MBF21
#define DEATH_MASK              0x1000  // bit 12
#define KILL_MONSTERS_MASK      0x2000  // bit 13

// jff 02/04/98 Define masks, shifts, for fields in generalized linedef types
#define GenEnd                  0x8000
#define GenFloorBase            0x6000
#define GenCeilingBase          0x4000
#define GenDoorBase             0x3C00
#define GenLockedBase           0x3800
#define GenLiftBase             0x3400
#define GenStairsBase           0x3000
#define GenCrusherBase          0x2F80

#define TriggerType             0x0007
#define TriggerTypeShift        0

// define masks and shifts for the floor type fields
#define FloorCrush              0x1000
#define FloorChange             0x0C00
#define FloorTarget             0x0380
#define FloorDirection          0x0040
#define FloorModel              0x0020
#define FloorSpeed              0x0018

#define FloorCrushShift         12
#define FloorChangeShift        10
#define FloorTargetShift        7
#define FloorDirectionShift     6
#define FloorModelShift         5
#define FloorSpeedShift         3

// define masks and shifts for the ceiling type fields
#define CeilingCrush            0x1000
#define CeilingChange           0x0C00
#define CeilingTarget           0x0380
#define CeilingDirection        0x0040
#define CeilingModel            0x0020
#define CeilingSpeed            0x0018

#define CeilingCrushShift       12
#define CeilingChangeShift      10
#define CeilingTargetShift      7
#define CeilingDirectionShift   6
#define CeilingModelShift       5
#define CeilingSpeedShift       3

// define masks and shifts for the lift type fields
#define LiftTarget              0x0300
#define LiftDelay               0x00C0
#define LiftMonster             0x0020
#define LiftSpeed               0x0018

#define LiftTargetShift         8
#define LiftDelayShift          6
#define LiftSpeedShift          3

// define masks and shifts for the stairs type fields
#define StairIgnore             0x0200
#define StairDirection          0x0100
#define StairStep               0x00C0
#define StairMonster            0x0020
#define StairSpeed              0x0018

#define StairIgnoreShift        9
#define StairDirectionShift     8
#define StairStepShift          6
#define StairSpeedShift         3

// define masks and shifts for the crusher type fields
#define CrusherSilent           0x0040
#define CrusherMonster          0x0020
#define CrusherSpeed            0x0018

#define CrusherSilentShift      6
#define CrusherSpeedShift       3

// define masks and shifts for the door type fields
#define DoorDelay               0x0300
#define DoorMonster             0x0080
#define DoorKind                0x0060
#define DoorSpeed               0x0018

#define DoorDelayShift          8
#define DoorKindShift           5
#define DoorSpeedShift          3

// define masks and shifts for the locked door type fields
#define LockedNKeys             0x0200
#define LockedKey               0x01C0
#define LockedKind              0x0020
#define LockedSpeed             0x0018

#define LockedNKeysShift        9
#define LockedKeyShift          6
#define LockedKindShift         5
#define LockedSpeedShift        3

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

#if defined(X11) && defined(AnyKey)
#undef AnyKey
#endif

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

bool EV_DoGenFloor(line_t *line);
bool EV_DoGenCeiling(line_t *line);
bool EV_DoGenLift(line_t *line);
bool EV_DoGenStairs(line_t *line);
bool EV_DoGenCrusher(line_t *line);
bool EV_DoGenDoor(line_t *line);
bool EV_DoGenLockedDoor(line_t *line);
