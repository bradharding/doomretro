/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __P_SPEC__
#define __P_SPEC__

//
// End-level timer (-TIMER option)
//
extern  boolean levelTimer;
extern  int     levelTimeCount;

// Define values for map objects
#define MO_TELEPORTMAN          14

// at game start
void P_InitPicAnims(void);

// at map load
void P_SpawnSpecials(void);

// every tic
void P_UpdateSpecials(void);

boolean P_CheckTag(line_t *line);

// when needed
boolean P_UseSpecialLine(mobj_t *thing, line_t *line, int side);

void P_ShootSpecialLine(mobj_t *thing, line_t *line);

void P_CrossSpecialLine(line_t *line, int side, mobj_t *thing);

void P_PlayerInSpecialSector(player_t *player);

int twoSided(int sector, int line);

sector_t *getSector(int currentSector, int line, int side);

side_t *getSide(int currentSector, int line, int side);

fixed_t P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(sector_t *sec);

fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight);

fixed_t P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec);

int P_FindSectorFromLineTag(const line_t *line, int start);
int P_FindLineFromLineTag(const line_t *line, int start);

int P_FindMinSurroundingLight(sector_t *sector, int max);

sector_t *getNextSector(line_t *line, sector_t *sec);

//
// SPECIAL
//
int EV_DoDonut(line_t *line);

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

#define GLOWSPEED               8
#define STROBEBRIGHT            5
#define FASTDARK                15
#define SLOWDARK                35

void P_SpawnFireFlicker(sector_t *sector);
void T_LightFlash(lightflash_t *flash);
void P_SpawnLightFlash(sector_t *sector);
void T_StrobeFlash(strobe_t *flash);

void P_SpawnStrobeFlash(sector_t *sector, int fastOrSlow, int inSync);

int EV_StartLightStrobing(line_t *line);
int EV_TurnTagLightsOff(line_t *line);

int EV_LightTurnOn(line_t *line, int bright);

void T_Glow(glow_t *g);
void P_SpawnGlowingLight(sector_t *sector);

void T_FireFlicker(fireflicker_t *flick);

//
// P_SWITCH
//

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct
{
    char        name1[9];
    char        name2[9];
    short       episode;
} switchlist_t;

#ifdef _MSC_VER
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

// max # of wall switches in a level
#define MAXSWITCHES             100

// 4 players, 4 buttons each at once, max.
#define MAXBUTTONS              32

// 1 second, in ticks.
#define BUTTONTIME              35

button_t buttonlist[MAXBUTTONS];

void P_ChangeSwitchTexture(line_t *line, int useAgain);

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

typedef enum
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS
} plattype_e;

typedef struct plat_s
{
    thinker_t          thinker;
    struct plat_s       *next;
    struct plat_s      *prev;
    sector_t           *sector;
    fixed_t            speed;
    fixed_t            low;
    fixed_t            high;
    int                wait;
    int                count;
    plat_e             status;
    plat_e             oldstatus;
    boolean            crush;
    int                tag;
    plattype_e         type;
} plat_t;

#define PLATWAIT                3
#define PLATSPEED               FRACUNIT

extern plat_t *activeplatshead;

void T_PlatRaise(plat_t *plat);

int EV_DoPlat(line_t *line, plattype_e type, int amount);

void P_AddActivePlat(plat_t *plat);
void P_RemoveActivePlat(plat_t *plat);
int EV_StopPlat(line_t *line);
void P_ActivateInStasis(int tag);

//
// P_DOORS
//
typedef enum
{
    normal,
    close30ThenOpen,
    close,
    open,
    raiseIn5Mins,
    blazeRaise,
    blazeOpen,
    blazeClose
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
} vldoor_t;

#define VDOORSPEED              FRACUNIT * 2
#define VDOORWAIT               150

void EV_VerticalDoor(line_t *line, mobj_t *thing);

int EV_DoDoor(line_t *line, vldoor_e type);

int EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing);

void T_VerticalDoor(vldoor_t *door);
void P_SpawnDoorCloseIn30(sector_t *sec);

void P_SpawnDoorRaiseIn5Mins(sector_t *sec);

//
// P_CEILNG
//
typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise
} ceiling_e;

typedef struct ceiling_s
{
    thinker_t           thinker;
    struct ceiling_s    *next;
    struct ceiling_s    *prev;
    ceiling_e           type;
    sector_t            *sector;
    fixed_t             bottomheight;
    fixed_t             topheight;
    fixed_t             speed;
    boolean             crush;

    // 1 = up, 0 = waiting, -1 = down
    int                 direction;

    // ID
    int                 tag;
    int                 olddirection;
} ceiling_t;

#define CEILSPEED               FRACUNIT
#define CEILWAIT                150

extern ceiling_t        *activeceilingshead;

int EV_DoCeiling(line_t *line, ceiling_e type);

void T_MoveCeiling(ceiling_t *ceiling);
void P_AddActiveCeiling(ceiling_t *c);
void P_RemoveActiveCeiling(ceiling_t *c);
int EV_CeilingCrushStop(line_t *line);
int P_ActivateInStasisCeiling(line_t *line);

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

    // raise floor to shortest height texture around it
    raiseToTexture,

    // lower floor to lowest surrounding floor
    //  and change floorpic
    lowerAndChange,

    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,

    // raise to next highest floor, turbo-speed
    raiseFloorTurbo,
    donutRaise,
    raiseFloor512,

    buildStair
} floor_e;

typedef enum
{
    build8,     // slowly build by 8
    turbo16     // quickly build by 16
} stair_e;

typedef struct
{
    thinker_t   thinker;
    floor_e     type;
    boolean     crush;
    sector_t    *sector;
    int         direction;
    int         newspecial;
    short       texture;
    fixed_t     floordestheight;
    fixed_t     speed;
    boolean     stopsound;
} floormove_t;

#define FLOORSPEED              FRACUNIT

typedef enum
{
    ok,
    crushed,
    pastdest
} result_e;

result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest, boolean crush,
    int floorOrCeiling, int direction);

boolean EV_BuildStairs(line_t *line, stair_e type);

boolean EV_DoFloor(line_t *line, floor_e floortype);

void T_MoveFloor(floormove_t *floor);

//
// P_TELEPT
//
boolean EV_Teleport(line_t *line, int side, mobj_t *thing);

#endif
