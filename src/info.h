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

#if !defined(__INFO_H__)
#define __INFO_H__

#include "doomtype.h"
#include "d_think.h"

typedef enum
{
    NOTYPE = -1,
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
    MT_PULL,    // controls pull source                     // phares 3/20/98

    MT_DOGS,    // killough 7/19/98: Marine's best friend

    MT_PLASMA1, // killough 7/11/98: first of alternating beta plasma fireballs
    MT_PLASMA2, // killough 7/11/98: second of alternating beta plasma fireballs
    MT_SCEPTRE, // killough 7/11/98: evil sceptre in beta version
    MT_BIBLE,   // killough 7/11/98: unholy bible in beta version

    MT_MUSICSOURCE,
    MT_GIBDTH,

    // [BH] DOOM Retro mobjs
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

    HMT_MISC0,
    HMT_ITEMSHIELD1,
    HMT_ITEMSHIELD2,
    HMT_MISC1,
    HMT_MISC2,
    HMT_ARTIINVISIBILITY,
    HMT_MISC3,
    HMT_ARTIFLY,
    HMT_ARTIINVULNERABILITY,
    HMT_ARTITOMEOFPOWER,
    HMT_ARTIEGG,
    HMT_EGGFX,
    HMT_ARTISUPERHEAL,
    HMT_MISC4,
    HMT_MISC5,
    HMT_FIREBOMB,
    HMT_ARTITELEPORT,
    HMT_POD,
    HMT_PODGOO,
    HMT_PODGENERATOR,
    HMT_SPLASH,
    HMT_SPLASHBASE,
    HMT_LAVASPLASH,
    HMT_LAVASMOKE,
    HMT_SLUDGECHUNK,
    HMT_SLUDGESPLASH,
    HMT_SKULLHANG70,
    HMT_SKULLHANG60,
    HMT_SKULLHANG45,
    HMT_SKULLHANG35,
    HMT_CHANDELIER,
    HMT_SERPTORCH,
    HMT_SMALLPILLAR,
    HMT_STALAGMITESMALL,
    HMT_STALAGMITELARGE,
    HMT_STALACTITESMALL,
    HMT_STALACTITELARGE,
    HMT_MISC6,
    HMT_BARREL,
    HMT_MISC7,
    HMT_MISC8,
    HMT_MISC9,
    HMT_MISC10,
    HMT_MISC11,
    HMT_KEYGIZMOBLUE,
    HMT_KEYGIZMOGREEN,
    HMT_KEYGIZMOYELLOW,
    HMT_KEYGIZMOFLOAT,
    HMT_MISC12,
    HMT_VOLCANOBLAST,
    HMT_VOLCANOTBLAST,
    HMT_TELEGLITGEN,
    HMT_TELEGLITGEN2,
    HMT_TELEGLITTER,
    HMT_TELEGLITTER2,
    HMT_TFOG,
    HMT_TELEPORTMAN,
    HMT_STAFFPUFF,
    HMT_STAFFPUFF2,
    HMT_BEAKPUFF,
    HMT_MISC13,
    HMT_GAUNTLETPUFF1,
    HMT_GAUNTLETPUFF2,
    HMT_MISC14,
    HMT_BLASTERFX1,
    HMT_BLASTERSMOKE,
    HMT_RIPPER,
    HMT_BLASTERPUFF1,
    HMT_BLASTERPUFF2,
    HMT_WMACE,
    HMT_MACEFX1,
    HMT_MACEFX2,
    HMT_MACEFX3,
    HMT_MACEFX4,
    HMT_WSKULLROD,
    HMT_HORNRODFX1,
    HMT_HORNRODFX2,
    HMT_RAINPLR1,
    HMT_RAINPLR2,
    HMT_RAINPLR3,
    HMT_RAINPLR4,
    HMT_GOLDWANDFX1,
    HMT_GOLDWANDFX2,
    HMT_GOLDWANDPUFF1,
    HMT_GOLDWANDPUFF2,
    HMT_WPHOENIXROD,
    HMT_PHOENIXFX1,
    HMT_PHOENIXFX_REMOVED,  // In Heretic 1.0, but removed.
    HMT_PHOENIXPUFF,
    HMT_PHOENIXFX2,
    HMT_MISC15,
    HMT_CRBOWFX1,
    HMT_CRBOWFX2,
    HMT_CRBOWFX3,
    HMT_CRBOWFX4,
    HMT_BLOOD,
    HMT_BLOODSPLATTER,
    HMT_PLAYER,
    HMT_BLOODYSKULL,
    HMT_CHICPLAYER,
    HMT_CHICKEN,
    HMT_FEATHER,
    HMT_MUMMY,
    HMT_MUMMYLEADER,
    HMT_MUMMYGHOST,
    HMT_MUMMYLEADERGHOST,
    HMT_MUMMYSOUL,
    HMT_MUMMYFX1,
    HMT_BEAST,
    HMT_BEASTBALL,
    HMT_BURNBALL,
    HMT_BURNBALLFB,
    HMT_PUFFY,
    HMT_SNAKE,
    HMT_SNAKEPRO_A,
    HMT_SNAKEPRO_B,
    HMT_HEAD,
    HMT_HEADFX1,
    HMT_HEADFX2,
    HMT_HEADFX3,
    HMT_WHIRLWIND,
    HMT_CLINK,
    HMT_WIZARD,
    HMT_WIZFX1,
    HMT_IMP,
    HMT_IMPLEADER,
    HMT_IMPCHUNK1,
    HMT_IMPCHUNK2,
    HMT_IMPBALL,
    HMT_KNIGHT,
    HMT_KNIGHTGHOST,
    HMT_KNIGHTAXE,
    HMT_REDAXE,
    HMT_SORCERER1,
    HMT_SRCRFX1,
    HMT_SORCERER2,
    HMT_SOR2FX1,
    HMT_SOR2FXSPARK,
    HMT_SOR2FX2,
    HMT_SOR2TELEFADE,
    HMT_MINOTAUR,
    HMT_MNTRFX1,
    HMT_MNTRFX2,
    HMT_MNTRFX3,
    HMT_AKYY,
    HMT_BKYY,
    HMT_CKEY,
    HMT_AMGWNDWIMPY,
    HMT_AMGWNDHEFTY,
    HMT_AMMACEWIMPY,
    HMT_AMMACEHEFTY,
    HMT_AMCBOWWIMPY,
    HMT_AMCBOWHEFTY,
    HMT_AMSKRDWIMPY,
    HMT_AMSKRDHEFTY,
    HMT_AMPHRDWIMPY,
    HMT_AMPHRDHEFTY,
    HMT_AMBLSRWIMPY,
    HMT_AMBLSRHEFTY,
    HMT_SOUNDWIND,
    HMT_SOUNDWATERFALL,

    NUMMOBJTYPES
} mobjtype_t;

typedef struct
{
    int         doomednum;
    int         spawnstate;
    int         spawnhealth;
    int         gibhealth;
    int         seestate;
    int         seesound;
    int         reactiontime;
    int         attacksound;
    int         painstate;
    int         painchance;
    int         painsound;
    int         meleestate;
    int         missilestate;
    int         crashstate;
    int         deathstate;
    int         xdeathstate;
    int         deathsound;
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
    int         flags3;
    int         raisestate;
    int         frames;
    dboolean    fullbright;
    int         blood;
    int         shadowoffset;
    char        name1[100];
    char        plural1[100];
    char        name2[100];
    char        plural2[100];
    char        name3[100];
    char        plural3[100];
    void        (*colfunc)(void);
    void        (*altcolfunc)(void);
} mobjinfo_t;

extern mobjinfo_t   mobjinfo[];
extern mobjtype_t   playermobjtype;

#endif
