/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2021 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2021 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

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
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__P_LOCAL_H__)
#define __P_LOCAL_H__

#include "d_main.h"
#include "r_local.h"

#define FOOTCLIPSIZE        (10 * FRACUNIT)

#define FLOATSPEED          (4 * FRACUNIT)

#define VIEWHEIGHT          (41 * FRACUNIT)
#define DEADVIEWHEIGHT      (6 * FRACUNIT)
#define JUMPHEIGHT          (9 * FRACUNIT)

#define DEADLOOKDIR         128

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS       128
#define MAPBLOCKSIZE        (MAPBLOCKUNITS * FRACUNIT)
#define MAPBLOCKSHIFT       (FRACBITS + 7)
#define MAPBTOFRAC          (MAPBLOCKSHIFT - FRACBITS)

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS           (32 * FRACUNIT)

#define GRAVITY             FRACUNIT
#define MAXMOVE             (30 * FRACUNIT)
#define MAXMOVE_STEP        (8 * FRACUNIT)
#define MINBOUNCEMAX        (-20 * FRACUNIT)

#define USERANGE            (64 * FRACUNIT)
#define MELEERANGE          (64 * FRACUNIT)
#define MISSILERANGE        (32 * 64 * FRACUNIT)

// follow a player exclusively for 3 seconds
#define BASETHRESHOLD       100

#define BONUSADD            6

#define MOUSE_LEFTBUTTON    1
#define MOUSE_RIGHTBUTTON   2

#define MOUSE_WHEELUP       MAX_MOUSE_BUTTONS
#define MOUSE_WHEELDOWN     (MAX_MOUSE_BUTTONS + 1)

#define NEEDEDCARDFLASH     10

#define WEAPONBOTTOM        (128 * FRACUNIT)
#define WEAPONTOP           (32 * FRACUNIT)

//
// P_PSPR.C
//
void P_SetupPsprites(void);
void P_MovePsprites(void);
void P_FireWeapon(void);
void P_DropWeapon(void);
void P_SetPsprite(size_t position, statenum_t stnum);

//
// P_USER.C
//
// 16 pixels of bob
#define MAXBOB              0x100000
#define MLOOKUNIT           8
#define PLAYERSLOPE(a)      ((((a)->lookdir / MLOOKUNIT) << FRACBITS) / 153)

extern dboolean autousing;
extern int      deadlookdir;
extern fixed_t  animatedliquiddiffs[64];

void P_CalcHeight(void);
void P_MovePlayer(void);
void P_PlayerThink(void);
void P_ResurrectPlayer(int health);
void P_ChangeWeapon(weapontype_t newweapon);

//
// P_MOBJ.C
//
#define ONFLOORZ            FIXED_MIN
#define ONCEILINGZ          FIXED_MAX

// Time interval for item respawning.
#define ITEMQUEUESIZE       512

#define CARDNOTFOUNDYET    -1
#define CARDNOTINMAP        0

void P_RespawnSpecials(void);

void P_SetPlayerViewHeight(void);

void P_InitCards(void);

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
void P_SetShadowColumnFunction(mobj_t *mobj);
mobjtype_t P_FindDoomedNum(int type);

void P_RemoveMobj(mobj_t *mobj);
void P_RemoveBloodMobj(mobj_t *mobj);
dboolean P_SetMobjState(mobj_t *mobj, statenum_t state);
void P_MobjThinker(mobj_t *mobj);

void P_SpawnMoreBlood(mobj_t *mobj);
mobj_t *P_SpawnMapThing(mapthing_t *mthing, dboolean spawnmonsters);
void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z, angle_t angle);
void P_SpawnSmokeTrail(fixed_t x, fixed_t y, fixed_t z, angle_t angle);
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, angle_t angle, int damage, mobj_t *target);
void P_SpawnBloodSplat(fixed_t x, fixed_t y, int blood, fixed_t maxheight, mobj_t *target);
void P_CheckMissileSpawn(mobj_t *th);
mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
void P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type);
void P_ExplodeMissile(mobj_t *mo);

//
// P_ENEMY.C
//
#define BARRELMS    1500

extern int  barrelms;

void P_NoiseAlert(mobj_t *target);
dboolean P_CheckMeleeRange(mobj_t *actor);

//
// P_MAPUTL.C
//
typedef struct
{
    fixed_t     x, y;
    fixed_t     dx, dy;
} divline_t;

typedef struct
{
    fixed_t     frac;           // along trace line
    dboolean    isaline;

    union
    {
        mobj_t  *thing;
        line_t  *line;
    } d;
} intercept_t;

typedef dboolean (*traverser_t)(intercept_t *in);

fixed_t P_ApproxDistance(fixed_t dx, fixed_t dy);
int P_PointOnLineSide(fixed_t x, fixed_t y, line_t *line);
int P_BoxOnLineSide(fixed_t *tmbox, line_t *ld);
fixed_t P_InterceptVector(divline_t *v2, divline_t *v1);

// MAES: support 512x512 blockmaps.
int P_GetSafeBlockX(int coord);
int P_GetSafeBlockY(int coord);

extern fixed_t  opentop;
extern fixed_t  openbottom;
extern fixed_t  openrange;
extern fixed_t  lowfloor;

void P_LineOpening(line_t *line);

dboolean P_BlockLinesIterator(int x, int y, dboolean func(line_t *));
dboolean P_BlockThingsIterator(int x, int y, dboolean func(mobj_t *));

#define PT_ADDLINES     1
#define PT_ADDTHINGS    2

extern divline_t    dltrace;

dboolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags, traverser_t trav);

void P_UnsetThingPosition(mobj_t *thing);
void P_UnsetBloodSplatPosition(bloodsplat_t *splat);
void P_SetThingPosition(mobj_t *thing);
void P_SetBloodSplatPosition(bloodsplat_t *splat);

void P_CheckIntercepts(void);

//
// P_MAP.C
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern fixed_t      attackrange;
extern dboolean     floatok;
extern dboolean     felldown;       // killough 11/98: indicates object pushed off ledge
extern fixed_t      tmfloorz;
extern fixed_t      tmbbox[4];      // phares 03/20/98
extern msecnode_t   *sector_list;
extern line_t       *ceilingline;
extern line_t       *blockline;

extern dboolean     infight;

void P_CheckSpechits(void);
dboolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);
mobj_t *P_CheckOnMobj(mobj_t *thing);
dboolean P_IsInLiquid(mobj_t *thing);
dboolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y, int dropoff);
dboolean P_CheckLineSide(mobj_t *actor, fixed_t x, fixed_t y);
dboolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, dboolean boss);
void P_SlideMove(mobj_t *mo);
dboolean P_CheckSight(mobj_t *t1, mobj_t *t2);
dboolean P_DoorClosed(line_t *line);
void P_UseLines(void);

dboolean P_ChangeSector(sector_t *sector, dboolean crunch);
void P_FreeSecNodeList(void);
void P_DelSeclist(msecnode_t *node);

extern mobj_t   *linetarget;    // who got hit (or NULL)

fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance, int mask);

void P_LineAttack(mobj_t *t1, angle_t angle, fixed_t distance, fixed_t slope, int damage);

dboolean PIT_RadiusAttack(mobj_t *thing);
void P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage, dboolean verticality);

int P_GetMoveFactor(const mobj_t *mo, int *frictionp);      // killough 08/28/98
int P_GetFriction(const mobj_t *mo, int *frictionfactor);   // killough 08/28/98
void P_ApplyTorque(mobj_t *mo);                             // killough 09/12/98

void P_MapEnd(void);

//
// P_SETUP.C
//
extern const byte   *rejectmatrix;  // for fast sight rejection
extern int          *blockmaplump;
extern int          *blockmap;
extern int          bmapwidth;
extern int          bmapheight;     // in mapblocks
extern fixed_t      bmaporgx;
extern fixed_t      bmaporgy;       // origin of block map
extern mobj_t       **blocklinks;   // for thing chains

// MAES: extensions to support 512x512 blockmaps.
extern int          blockmapxneg;
extern int          blockmapyneg;

//
// P_INTER.C
//
#define MAXHEALTH   100

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher, dboolean message, dboolean stat);
dboolean P_TakeSpecialThing(mobjtype_t type);

void P_DamageMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source, int damage, dboolean adjust);

void P_ResurrectMobj(mobj_t *target);

extern int      god_health;
extern int      idfa_armor;
extern int      idfa_armor_class;
extern int      idkfa_armor;
extern int      idkfa_armor_class;
extern int      initial_health;
extern int      initial_bullets;
extern int      maxhealth;
extern int      max_armor;
extern int      green_armor_class;
extern int      blue_armor_class;
extern int      max_soul;
extern int      soul_health;
extern int      mega_health;
extern int      bfgcells;
extern dboolean species_infighting;
extern int      maxammo[];
extern int      clipammo[];
extern int      prevobituarytics;

//
// P_SPEC.C
//
#include "p_spec.h"

#endif
