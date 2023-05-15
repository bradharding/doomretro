/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

========================================================================
*/

#pragma once

#include "d_main.h"
#include "m_config.h"
#include "r_local.h"

#define FOOTCLIPSIZE        (10 * FRACUNIT)

#define FLOATSPEED          (4 * FRACUNIT)

#define VIEWHEIGHT          (41 * FRACUNIT)
#define DEADVIEWHEIGHT      (6 * FRACUNIT)
#define MENUVIEWHEIGHT      (6 * FRACUNIT)
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
#define WAKEUPRANGE         (64 * FRACUNIT)

// follow the player exclusively for 3 seconds
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
void P_SetupPlayerSprites(void);
void P_MovePlayerSprites(void);
void P_FireWeapon(void);
void P_DropWeapon(void);
void P_SetPlayerSprite(const size_t position, const statenum_t stnum);

//
// P_USER.C
//
// 16 pixels of bob
#define MAXBOB              0x100000
#define MLOOKUNIT           8
#define PLAYERSLOPE(a)      ((((a)->lookdir / MLOOKUNIT) << FRACBITS) / 153)

extern bool autousing;
extern int  deadlookdir;

void P_CalcHeight(void);
void P_MovePlayer(void);
void P_PlayerThink(void);
void P_ResurrectPlayer(const int health);
void P_ChangeWeapon(weapontype_t newweapon);

void P_AnimateHealth(int diff);
void P_AnimateArmor(int diff);
void P_AnimateAmmo(int diff, ammotype_t type);
void P_AnimateMaxAmmo(int diff, ammotype_t type);

//
// P_MOBJ.C
//
#define ONFLOORZ            FIXED_MIN
#define ONCEILINGZ          FIXED_MAX

// Time interval for item respawning.
#define ITEMQUEUESIZE       1024

#define CARDNOTFOUNDYET    -1
#define CARDNOTINMAP        0

void P_RespawnSpecials(void);

void P_SetPlayerViewHeight(void);

void P_LookForCards(void);
void P_InitCards(void);

mobj_t *P_SpawnMobj(const fixed_t x, const fixed_t y, const fixed_t z, const mobjtype_t type);
void P_SetShadowColumnFunction(mobj_t *mobj);
mobjtype_t P_FindDoomedNum(const unsigned int type);

void P_RemoveMobj(mobj_t *mobj);
void P_RemoveBloodMobj(mobj_t *mobj);
void P_RemoveBloodSplats(void);
bool P_SetMobjState(mobj_t *mobj, statenum_t state);
void P_MobjThinker(mobj_t *mobj);

void P_SpawnMoreBlood(mobj_t *mobj);
mobj_t *P_SpawnMapThing(mapthing_t *mthing, const bool spawnmonsters);
void P_SpawnPuff(const fixed_t x, const fixed_t y, const fixed_t z, const angle_t angle);
void P_SpawnSmokeTrail(const fixed_t x, const fixed_t y, const fixed_t z, const angle_t angle);
void P_SpawnBlood(const fixed_t x, const fixed_t y, const fixed_t z, angle_t angle, const int damage, mobj_t *target);
void P_SetBloodSplatColor(bloodsplat_t *splat);
void P_SpawnBloodSplat(const fixed_t x, const fixed_t y, const int color,
    const bool usemaxheight, const fixed_t maxheight, mobj_t *target);
bool P_CheckMissileSpawn(mobj_t *th);
mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
mobj_t *P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type);
void P_ExplodeMissile(mobj_t *mo);
bool P_SeekerMissile(mobj_t *actor, mobj_t **seektarget, angle_t thresh, angle_t turnmax, const bool seekcenter);

//
// P_ENEMY.C
//
#define BERSERKPUNCHMONSTER 500
#define BERSERKPUNCHWALL    200
#define EXPLODINGBARREL    1500

extern uint64_t shake;
extern int      shakeduration;

void P_NoiseAlert(mobj_t *target);
bool P_CheckMeleeRange(mobj_t *actor);

//
// P_MAPUTL.C
//
typedef struct
{
    fixed_t x, y;
    fixed_t dx, dy;
} divline_t;

typedef struct
{
    fixed_t frac;   // along trace line
    mobj_t  *thing;
    line_t  *line;
} intercept_t;

typedef bool (*traverser_t)(intercept_t *in);

fixed_t P_ApproxDistance(fixed_t dx, fixed_t dy);
int P_PointOnLineSide(const fixed_t x, const fixed_t y, const line_t *line);
int P_BoxOnLineSide(const fixed_t *tmbox, const line_t *ld);
fixed_t P_InterceptVector(divline_t *v2, divline_t *v1);

// MAES: support 512x512 blockmaps.
int P_GetSafeBlockX(int coord);
int P_GetSafeBlockY(int coord);

mobj_t *P_RoughTargetSearch(mobj_t *mo, const angle_t fov, const int distance);

extern fixed_t  opentop;
extern fixed_t  openbottom;
extern fixed_t  openrange;
extern fixed_t  lowfloor;

void P_LineOpening(line_t *line);

bool P_BlockLinesIterator(const int x, const int y, bool func(line_t *));
bool P_BlockThingsIterator(const int x, const int y, bool func(mobj_t *));

#define PT_ADDLINES     1
#define PT_ADDTHINGS    2

extern divline_t    dltrace;

bool P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, const int flags, traverser_t trav);

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
extern bool         floatok;
extern bool         felldown;       // killough 11/98: indicates object pushed off ledge
extern fixed_t      tmfloorz;
extern fixed_t      tmbbox[4];      // phares 03/20/98
extern msecnode_t   *sector_list;
extern line_t       *ceilingline;
extern line_t       *blockline;

// killough 01/11/98: Limit removed on special lines crossed
extern line_t       **spechit;
extern int          numspechit;

extern bool         infight;
extern bool         hitwall;

void P_CheckSpechits(void);
bool P_CheckPosition(mobj_t *thing, const fixed_t x, const fixed_t y);
mobj_t *P_CheckOnMobj(mobj_t *thing);
bool P_IsInLiquid(mobj_t *thing);
bool P_TryMove(mobj_t *thing, const fixed_t x, const fixed_t y, const int dropoff);
bool P_CheckLineSide(mobj_t *actor, const fixed_t x, const fixed_t y);
bool P_TeleportMove(mobj_t *thing, const fixed_t x, const fixed_t y, const fixed_t z, const bool boss);
void P_SlideMove(mobj_t *mo);
bool P_CheckSight(mobj_t *t1, mobj_t *t2);
bool P_CheckFOV(mobj_t *t1, mobj_t *t2, angle_t fov);
bool P_DoorClosed(line_t *line);
void P_UseLines(void);

bool P_ChangeSector(sector_t *sector, const bool crunch);
void P_CreateSecNodeList(mobj_t *thing, const fixed_t x, const fixed_t y);
void P_FreeSecNodeList(void);
void P_DelSeclist(msecnode_t *node);

extern mobj_t   *linetarget;    // who got hit (or NULL)

fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, const fixed_t distance, const int mask);

void P_LineAttack(mobj_t *t1, angle_t angle, const fixed_t distance, const fixed_t slope, const int damage);

bool PIT_RadiusAttack(mobj_t *thing);
void P_RadiusAttack(mobj_t *spot, mobj_t *source, const int damage, const int distance, const bool verticality);

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
extern fixed_t      bmaporgy;       // origin of blockmap
extern mobj_t       **blocklinks;   // for thing chains

// MAES: extensions to support 512x512 blockmaps.
extern int          blockmapxneg;
extern int          blockmapyneg;

//
// P_INTER.C
//
#define MAXHEALTH   100

bool P_TouchSpecialThing(mobj_t *special, mobj_t *toucher, const bool message, const bool stat);
bool P_TakeSpecialThing(const mobjtype_t type);

void P_DamageMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source, int damage, const bool adjust, const bool telefragged);

void P_ResurrectMobj(mobj_t *target);

extern int          god_health;
extern int          idfa_armor;
extern int          idfa_armor_class;
extern int          idkfa_armor;
extern int          idkfa_armor_class;
extern int          initial_health;
extern int          initial_bullets;
extern int          maxhealth;
extern int          max_armor;
extern int          green_armor_class;
extern int          blue_armor_class;
extern int          max_soul;
extern int          soul_health;
extern int          mega_health;
extern int          bfgcells;
extern bool         species_infighting;
extern int          maxammo[];
extern int          clipammo[];
extern mobjtype_t   prevtouchtype;

//
// P_SPEC.C
//
#include "p_spec.h"
