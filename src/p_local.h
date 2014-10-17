/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

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

#ifndef __P_LOCAL__
#define __P_LOCAL__

#ifndef __R_LOCAL__
#include "r_local.h"
#endif

#ifndef __M_CONFIG__
#include "m_config.h"
#endif

#ifndef __D_MAIN__
#include "d_main.h"
#endif

#define FOOTCLIPSIZE            (10 * FRACUNIT)

#define FLOATSPEED              (FRACUNIT * 4)

#define VIEWHEIGHT              (41 * FRACUNIT)

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS           128
#define MAPBLOCKSIZE            (MAPBLOCKUNITS * FRACUNIT)
#define MAPBLOCKSHIFT           (FRACBITS + 7)
#define MAPBMASK                (MAPBLOCKSIZE - 1)
#define MAPBTOFRAC              (MAPBLOCKSHIFT - FRACBITS)

// player radius for movement checking
#define PLAYERRADIUS            (16 * FRACUNIT)

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS               32 * FRACUNIT

#define GRAVITY                 FRACUNIT
#define MAXMOVE                 (30 * FRACUNIT)

#define USERANGE                (64 * FRACUNIT)
#define MELEERANGE              (64 * FRACUNIT)
#define MISSILERANGE            (32 * 64 * FRACUNIT)

// follow a player exlusively for 3 seconds
#define BASETHRESHOLD           100

#define BONUSADD                6

#define MOUSE_LEFTBUTTON        1
#define MOUSE_RIGHTBUTTON       2
#define MOUSE_MIDDLEBUTTON      4
#define MOUSE_WHEELUP           8
#define MOUSE_WHEELDOWN         16

#define NEEDEDCARDTICS          85

//
// P_TICK
//

// both the head and tail of the thinker list
extern thinker_t        thinkercap;

void P_InitThinkers(void);
void P_AddThinker(thinker_t *thinker);
void P_RemoveThinker(thinker_t *thinker);

//
// P_PSPR
//
void P_SetupPsprites(player_t *player);
void P_MovePsprites(player_t *player);
void P_DropWeapon(player_t *player);

//
// P_USER
//
void P_PlayerThink(player_t *player);

//
// P_MOBJ
//
#define ONFLOORZ                INT_MIN
#define ONCEILINGZ              INT_MAX

// Time interval for item respawning.
#define ITEMQUEUESIZE           128

#define CARDNOTFOUNDYET         -1
#define CARDNOTINMAP            0

extern int              iqueuehead;
extern int              iqueuetail;

extern mobj_t           *bloodSplatQueue[BLOODSPLATS_MAX];
extern int              bloodSplatQueueSlot;
extern int              bloodsplats;

extern int              corpses;

void P_InitCards(player_t *player);

void P_RespawnSpecials(void);

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);

void P_RemoveMobj(mobj_t *th);
boolean P_SetMobjState(mobj_t *mobj, statenum_t state);
void P_MobjThinker(mobj_t *mobj);
void P_NullMobjThinker(mobj_t *mobj);

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z, angle_t angle, boolean sound);
void P_SpawnSmokeTrail(fixed_t x, fixed_t y, fixed_t z, angle_t angle);
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, angle_t angle, int damage, mobj_t *target);
void P_SpawnBloodSplat(fixed_t x, fixed_t y, int blood, int maxheight);
void P_SpawnBloodSplat2(fixed_t x, fixed_t y, int blood, int maxheight);
void P_NullBloodSplatSpawner(fixed_t x, fixed_t y, int blood, int maxheight);
mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
void P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type);

//
// P_ENEMY
//
void P_NoiseAlert(mobj_t *target, mobj_t *emmiter);

//
// P_MAPUTL
//
typedef struct
{
    fixed_t     x;
    fixed_t     y;
    fixed_t     dx;
    fixed_t     dy;

} divline_t;

typedef struct
{
    fixed_t     frac;           // along trace line
    boolean     isaline;
    union {
        mobj_t  *thing;
        line_t  *line;
    } d;
} intercept_t;

typedef boolean (*traverser_t)(intercept_t *in);

fixed_t P_ApproxDistance(fixed_t dx, fixed_t dy);
int P_PointOnLineSide(fixed_t x, fixed_t y, line_t *line);
int P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t *line);
void P_MakeDivline(line_t *li, divline_t *dl);
fixed_t P_InterceptVector(divline_t *v2, divline_t *v1);
int P_BoxOnLineSide(fixed_t *tmbox, line_t *ld);

extern fixed_t          opentop;
extern fixed_t          openbottom;
extern fixed_t          openrange;
extern fixed_t          lowfloor;

void P_LineOpening(line_t *linedef);

boolean P_BlockLinesIterator(int x, int y, boolean(*func)(line_t *));
boolean P_BlockThingsIterator(int x, int y, boolean(*func)(mobj_t *));

#define PT_ADDLINES     1
#define PT_ADDTHINGS    2
#define PT_EARLYOUT     4

extern divline_t        trace;

boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags,
                       boolean (*trav)(intercept_t *));

void P_UnsetThingPosition(mobj_t *thing);
void P_SetThingPosition(mobj_t *thing);

//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean          floatok;
extern boolean          felldown;       // killough 11/98: indicates object pushed off ledge
extern fixed_t          tmfloorz;
extern fixed_t          tmceilingz;
extern fixed_t          tmbbox[4];      // phares 3/20/98

extern line_t           *ceilingline;
extern line_t           *blockline;

extern boolean          infight;

boolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);
mobj_t *P_CheckOnmobj(mobj_t *thing);
void P_FakeZMovement(mobj_t *mo);
boolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y, boolean dropoff);
boolean P_CheckLineSide(mobj_t *actor, fixed_t x, fixed_t y);
boolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, boolean boss);
void P_SlideMove(mobj_t *mo);
boolean P_CheckSight(mobj_t *t1, mobj_t *t2);
void P_UseLines(player_t *player);

boolean P_ChangeSector(sector_t *sector, boolean crunch);
void P_FreeSecNodeList(void);

extern mobj_t           *linetarget;    // who got hit (or NULL)

fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance);

void P_LineAttack(mobj_t *t1, angle_t angle, fixed_t distance, fixed_t slope, int damage);

void P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage);

void P_ApplyTorque(mobj_t *mo); // killough 9/12/98

void P_MapEnd(void);

//
// P_SETUP
//
extern byte             *rejectmatrix;  // for fast sight rejection
extern int              rejectmatrixsize;
extern uint32_t         *blockmapindex;
extern uint32_t         *blockmaphead;
extern int              bmapwidth;
extern int              bmapheight;     // in mapblocks
extern fixed_t          bmaporgx;
extern fixed_t          bmaporgy;       // origin of block map
extern mobj_t           **blocklinks;   // for thing chains

//
// P_INTER
//
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher);

void P_DamageMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage);

extern int god_health;
extern int idfa_armor;
extern int idfa_armor_class;
extern int idkfa_armor;
extern int idkfa_armor_class;
extern int initial_health;
extern int initial_bullets;
extern int maxhealth;
extern int max_armor;
extern int green_armor_class;
extern int blue_armor_class;
extern int max_soul;
extern int soul_health;
extern int mega_health;
extern int god_health;
extern int idfa_armor;
extern int idfa_armor_class;
extern int idkfa_armor;
extern int idkfa_armor_class;
extern int bfgcells;
extern int species_infighting;
extern int maxammo[];
extern int clipammo[];

//
// P_SPEC
//
#include "p_spec.h"

#endif
