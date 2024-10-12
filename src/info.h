/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "doomtype.h"
#include "states.h"

#define NO_ALTSPEED     -1

#define MT_GHOUL        MT_EXTRA00
#define MT_BANSHEE      MT_EXTRA01
#define MT_MINDWEAVER   MT_EXTRA02
#define MT_SHOCKTROOPER MT_EXTRA03
#define MT_VASSAGO      MT_EXTRA04
#define MT_TYRANT       MT_EXTRA05
#define MT_TYRANTBOSS1  MT_EXTRA06
#define MT_TYRANTBOSS2  MT_EXTRA07
#define MT_FLAME        MT_EXTRA08
#define MT_LAMP         MT_EXTRA50

typedef enum
{
    MT_NULL = -1,
    MT_PLAYER,
    MT_POSSESSED,
    MT_SHOTGUY,
    MT_VILE,
    MT_FIRE,
    MT_UNDEAD,
    MT_TRACER,
    MT_SMOKE,
    MT_FATSO,
    MT_FATSHOT,
    MT_CHAINGUY,
    MT_TROOP,
    MT_SERGEANT,
    MT_SHADOWS,
    MT_HEAD,
    MT_BRUISER,
    MT_BRUISERSHOT,
    MT_KNIGHT,
    MT_SKULL,
    MT_SPIDER,
    MT_BABY,
    MT_CYBORG,
    MT_PAIN,
    MT_WOLFSS,
    MT_KEEN,
    MT_BOSSBRAIN,
    MT_BOSSSPIT,
    MT_BOSSTARGET,
    MT_SPAWNSHOT,
    MT_SPAWNFIRE,
    MT_BARREL,
    MT_TROOPSHOT,
    MT_HEADSHOT,
    MT_ROCKET,
    MT_PLASMA,
    MT_BFG,
    MT_ARACHPLAZ,
    MT_PUFF,
    MT_BLOOD,
    MT_TFOG,
    MT_IFOG,
    MT_TELEPORTMAN,
    MT_EXTRABFG,
    MT_MISC0,
    MT_MISC1,
    MT_MISC2,
    MT_MISC3,
    MT_MISC4,
    MT_MISC5,
    MT_MISC6,
    MT_MISC7,
    MT_MISC8,
    MT_MISC9,
    MT_MISC10,
    MT_MISC11,
    MT_MISC12,
    MT_INV,
    MT_MISC13,
    MT_INS,
    MT_MISC14,
    MT_MISC15,
    MT_MISC16,
    MT_MEGA,
    MT_CLIP,
    MT_MISC17,
    MT_MISC18,
    MT_MISC19,
    MT_MISC20,
    MT_MISC21,
    MT_MISC22,
    MT_MISC23,
    MT_MISC24,
    MT_MISC25,
    MT_CHAINGUN,
    MT_MISC26,
    MT_MISC27,
    MT_MISC28,
    MT_SHOTGUN,
    MT_SUPERSHOTGUN,
    MT_MISC29,
    MT_MISC30,
    MT_MISC31,
    MT_MISC32,
    MT_MISC33,
    MT_MISC34,
    MT_MISC35,
    MT_MISC36,
    MT_MISC37,
    MT_MISC38,
    MT_MISC39,
    MT_MISC40,
    MT_MISC41,
    MT_MISC42,
    MT_MISC43,
    MT_MISC44,
    MT_MISC45,
    MT_MISC46,
    MT_MISC47,
    MT_MISC48,
    MT_MISC49,
    MT_MISC50,
    MT_MISC51,
    MT_MISC52,
    MT_MISC53,
    MT_MISC54,
    MT_MISC55,
    MT_MISC56,
    MT_MISC57,
    MT_MISC58,
    MT_MISC59,
    MT_MISC60,
    MT_MISC61,
    MT_MISC62,
    MT_MISC63,
    MT_MISC64,
    MT_MISC65,
    MT_MISC66,
    MT_MISC67,
    MT_MISC68,
    MT_MISC69,
    MT_MISC70,
    MT_MISC71,
    MT_MISC72,
    MT_MISC73,
    MT_MISC74,
    MT_MISC75,
    MT_MISC76,
    MT_MISC77,
    MT_MISC78,
    MT_MISC79,
    MT_MISC80,
    MT_MISC81,
    MT_MISC82,
    MT_MISC83,
    MT_MISC84,
    MT_MISC85,
    MT_MISC86,

    MT_PUSH,    // controls push source                     // phares
    MT_PULL,    // controls pull source                     // phares 03/20/98

    MT_DOGS,    // killough 07/19/98: Marine's best friend

    MT_PLASMA1, // killough 07/11/98: first of alternating beta plasma fireballs
    MT_PLASMA2, // killough 07/11/98: second of alternating beta plasma fireballs
    MT_SCEPTRE, // killough 07/11/98: evil sceptre in beta version
    MT_BIBLE,   // killough 07/11/98: unholy bible in beta version

    MT_MUSICSOURCE,
    MT_GIBDTH,

    // [BH] no longer used
    MT_BLUEBLOOD,
    MT_GREENBLOOD,
    MT_FUZZYBLOOD,

    MT_TRAIL,

    // [BH] Mobjs 150 to 249 (100 extra mobjs to use in DeHackEd patches)
    MT_EXTRA00, MT_EXTRA01, MT_EXTRA02, MT_EXTRA03, MT_EXTRA04,
    MT_EXTRA05, MT_EXTRA06, MT_EXTRA07, MT_EXTRA08, MT_EXTRA09,
    MT_EXTRA10, MT_EXTRA11, MT_EXTRA12, MT_EXTRA13, MT_EXTRA14,
    MT_EXTRA15, MT_EXTRA16, MT_EXTRA17, MT_EXTRA18, MT_EXTRA19,
    MT_EXTRA20, MT_EXTRA21, MT_EXTRA22, MT_EXTRA23, MT_EXTRA24,
    MT_EXTRA25, MT_EXTRA26, MT_EXTRA27, MT_EXTRA28, MT_EXTRA29,
    MT_EXTRA30, MT_EXTRA31, MT_EXTRA32, MT_EXTRA33, MT_EXTRA34,
    MT_EXTRA35, MT_EXTRA36, MT_EXTRA37, MT_EXTRA38, MT_EXTRA39,
    MT_EXTRA40, MT_EXTRA41, MT_EXTRA42, MT_EXTRA43, MT_EXTRA44,
    MT_EXTRA45, MT_EXTRA46, MT_EXTRA47, MT_EXTRA48, MT_EXTRA49,
    MT_EXTRA50, MT_EXTRA51, MT_EXTRA52, MT_EXTRA53, MT_EXTRA54,
    MT_EXTRA55, MT_EXTRA56, MT_EXTRA57, MT_EXTRA58, MT_EXTRA59,
    MT_EXTRA60, MT_EXTRA61, MT_EXTRA62, MT_EXTRA63, MT_EXTRA64,
    MT_EXTRA65, MT_EXTRA66, MT_EXTRA67, MT_EXTRA68, MT_EXTRA69,
    MT_EXTRA70, MT_EXTRA71, MT_EXTRA72, MT_EXTRA73, MT_EXTRA74,
    MT_EXTRA75, MT_EXTRA76, MT_EXTRA77, MT_EXTRA78, MT_EXTRA79,
    MT_EXTRA80, MT_EXTRA81, MT_EXTRA82, MT_EXTRA83, MT_EXTRA84,
    MT_EXTRA85, MT_EXTRA86, MT_EXTRA87, MT_EXTRA88, MT_EXTRA89,
    MT_EXTRA90, MT_EXTRA91, MT_EXTRA92, MT_EXTRA93, MT_EXTRA94,
    MT_EXTRA95, MT_EXTRA96, MT_EXTRA97, MT_EXTRA98, MT_EXTRA99,

    NUMMOBJTYPES
} mobjtype_t;

typedef enum
{
    IG_DEFAULT,
    IG_END
} infightinggroup_t;

typedef enum
{
    PG_GROUPLESS = -1,
    PG_DEFAULT,
    PG_BARON,
    PG_END
} projectilegroup_t;

typedef enum
{
    SG_DEFAULT,
    SG_END
} splashgroup_t;

typedef struct
{
    int         doomednum;
    statenum_t  spawnstate;
    int         spawnhealth;
    int         gibhealth;
    int         giblevel;
    statenum_t  seestate;
    int         seesound;
    int         reactiontime;
    int         attacksound;
    statenum_t  painstate;
    int         painchance;
    int         painsound;
    statenum_t  meleestate;
    statenum_t  missilestate;
    statenum_t  deathstate;
    statenum_t  xdeathstate;
    int         deathsound;
    mobjtype_t  droppeditem;
    int         speed;
    int         radius;
    int         pickupradius;
    int         height;
    int         projectilepassheight;
    int         mass;
    int         damage;
    int         activesound;
    int         flags;
    int         flags2;
    statenum_t  raisestate;
    int         frames;
    bool        fullbright;
    int         bloodcolor;
    int         shadowoffset;

    // MBF21
    int         mbf21flags;
    int         infightinggroup;
    int         projectilegroup;
    int         splashgroup;
    int         ripsound;
    int         altspeed;
    int         meleerange;

    char        name1[64];
    char        plural1[64];
    char        name2[64];
    char        plural2[64];
    char        name3[64];
    char        plural3[64];

    void        (*colfunc)(void);
    void        (*altcolfunc)(void);
    byte        automapcolor;
    bool        dehacked;
} mobjinfo_t;

extern mobjinfo_t   original_mobjinfo[];

// DSDHacked
extern mobjinfo_t   *mobjinfo;
extern int          nummobjtypes;

void InitMobjInfo(void);
void dsdh_EnsureMobjInfoCapacity(const int limit);
