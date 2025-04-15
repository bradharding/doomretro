/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <math.h>
#include <ctype.h>

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "miniz/miniz.h"
#include "p_fix.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sc_man.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAXMAPINFO  100
#define NUMLIQUIDS  256

enum
{
    MCMD_ALLOWMONSTERTELEFRAGS = 1,
    MCMD_AUTHOR,
    MCMD_BOSSACTION,
    MCMD_CLUSTER,
    MCMD_COMPAT_CORPSEGIBS,
    MCMD_COMPAT_FLOORMOVE,
    MCMD_COMPAT_LIGHT,
    MCMD_COMPAT_LIMITPAIN,
    MCMD_COMPAT_NOPASSOVER,
    MCMD_COMPAT_STAIRS,
    MCMD_COMPAT_USEBLOCKING,
    MCMD_COMPAT_VILEGHOSTS,
    MCMD_COMPAT_ZOMBIE,
    MCMD_ENDBUNNY,
    MCMD_ENDCAST,
    MCMD_ENDGAME,
    MCMD_ENDPIC,
    MCMD_ENTERANIM,
    MCMD_ENTERPIC,
    MCMD_EPISODE,
    MCMD_EXITANIM,
    MCMD_EXITPIC,
    MCMD_INTERBACKDROP,
    MCMD_INTERMUSIC,
    MCMD_INTERTEXT,
    MCMD_INTERTEXTSECRET,
    MCMD_LABEL,
    MCMD_LEVELNAME,
    MCMD_LEVELPIC,
    MCMD_LIQUID,
    MCMD_MUSIC,
    MCMD_MUSICARTIST,
    MCMD_MUSICTITLE,
    MCMD_NEXT,
    MCMD_NEXTSECRET,
    MCMD_NOBRIGHTMAP,
    MCMD_NOFREELOOK,
    MCMD_NOGRADUALLIGHTING,
    MCMD_NOINTERMISSION,
    MCMD_NOJUMP,
    MCMD_NOLIQUID,
    MCMD_NOMOUSELOOK,
    MCMD_PAR,
    MCMD_PARTIME,
    MCMD_PISTOLSTART,
    MCMD_SECRETNEXT,
    MCMD_SKY1,
    MCMD_SKYTEXTURE,
    MCMD_TITLEPATCH
};

typedef struct
{
    int             allowmonstertelefrags;
    char            author[128];
    bossaction_t    *bossactions;
    int             cluster;
    bool            compat_corpsegibs;
    bool            compat_floormove;
    bool            compat_light;
    bool            compat_limitpain;
    bool            compat_nopassover;
    bool            compat_stairs;
    bool            compat_useblocking;
    bool            compat_zombie;
    bool            endbunny;
    bool            endcast;
    bool            endgame;
    int             endpic;
    char            enteranim[9];
    int             enterpic;
    char            exitanim[9];
    int             exitpic;
    char            interbackdrop[9];
    int             intermusic;
    char            intertext[1024];
    char            intertextsecret[1024];
    char            label[9];
    int             liquid[NUMLIQUIDS];
    int             mapinepisode;
    int             music;
    char            musicartist[128];
    char            musictitle[128];
    char            name[128];
    int             next;
    bool            nofreelook;
    bool            nograduallighting;
    bool            nojump;
    int             noliquid[NUMLIQUIDS];
    int             numbossactions;
    int             par;
    bool            pistolstart;
    bool            secret;
    int             secretnext;
    int             sky1texture;
    float           sky1scrolldelta;
    int             titlepatch;
} mapinfo_t;

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int                 numvertexes;
vertex_t            *vertexes;

int                 numsegs;
seg_t               *segs;

int                 numsectors;
sector_t            *sectors;

int                 numliquid;
int                 numdamaging;

int                 numsubsectors;
subsector_t         *subsectors;

int                 numnodes;
node_t              *nodes;

int                 numlines;
int                 numspeciallines;
int                 numvisiblelines;
int                 nummappedlines;
line_t              *lines;

int                 numsides;
side_t              *sides;

int                 numspawnedthings;
int                 thingid;
int                 numdecorations;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int                 bmapwidth;
int                 bmapheight;

// for large maps, WAD is 16-bit
int                 *blockmap;

// offsets in blockmap are from here
int                 *blockmaplump;

// origin of blockmap
fixed_t             bmaporgx;
fixed_t             bmaporgy;

// for thing chains
mobj_t              **blocklinks;

// MAES: extensions to support 512x512 blockmaps.
// They represent the maximum negative number which represents
// a positive offset, otherwise they are left at -257, which
// never triggers a check.
// If a blockmap index is ever LE than either, then
// its actual value is to be interpreted as 0x01FF&x.
// Full 512x512 blockmaps get this value set to -1.
// A 511x511 blockmap would still have a valid negative number
// e.g. -1..510, so they would be set to -2
// Non-extreme maps remain unaffected.
int                 blockmapxneg = -257;
int                 blockmapyneg = -257;

bool                skipblstart;            // MaxW: Skip initial blocklist short

// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
static int          rejectlump = -1;        // cph - store reject lump num if cached
const byte          *rejectmatrix;          // cph - const*

static mapinfo_t    mapinfo[10][100];

static char *mapcmdnames[] =
{
    "ALLOWMONSTERTELEFRAGS",
    "AUTHOR",
    "BOSSACTION",
    "CLUSTER",
    "COMPAT_CORPSEGIBS",
    "COMPAT_FLOORMOVE",
    "COMPAT_LIGHT",
    "COMPAT_LIMITPAIN",
    "COMPAT_NOPASSOVER",
    "COMPAT_STAIRS",
    "COMPAT_USEBLOCKING",
    "COMPAT_VILEGHOSTS",
    "COMPAT_ZOMBIE",
    "ENDBUNNY",
    "ENDCAST",
    "ENDGAME",
    "ENDPIC",
    "ENTERANIM",
    "ENTERPIC",
    "EPISODE",
    "EXITANIM",
    "EXITPIC",
    "INTERBACKDROP",
    "INTERMUSIC",
    "INTERTEXT",
    "INTERTEXTSECRET",
    "LABEL",
    "LEVELNAME",
    "LEVELPIC",
    "LIQUID",
    "MUSIC",
    "MUSICARTIST",
    "MUSICTITLE",
    "NEXT",
    "NEXTSECRET",
    "NOBRIGHTMAP",
    "NOFREELOOK",
    "NOGRADUALLIGHTING",
    "NOINTERMISSION",
    "NOJUMP",
    "NOLIQUID",
    "NOMOUSELOOK",
    "PAR",
    "PARTIME",
    "PISTOLSTART",
    "SECRETNEXT",
    "SKY1",
    "SKYTEXTURE",
    "TITLEPATCH",
    NULL
};

static int mapcmdids[] =
{
    MCMD_ALLOWMONSTERTELEFRAGS,
    MCMD_AUTHOR,
    MCMD_BOSSACTION,
    MCMD_CLUSTER,
    MCMD_COMPAT_CORPSEGIBS,
    MCMD_COMPAT_FLOORMOVE,
    MCMD_COMPAT_LIGHT,
    MCMD_COMPAT_LIMITPAIN,
    MCMD_COMPAT_NOPASSOVER,
    MCMD_COMPAT_STAIRS,
    MCMD_COMPAT_USEBLOCKING,
    MCMD_COMPAT_VILEGHOSTS,
    MCMD_COMPAT_ZOMBIE,
    MCMD_ENDBUNNY,
    MCMD_ENDCAST,
    MCMD_ENDGAME,
    MCMD_ENDPIC,
    MCMD_ENTERANIM,
    MCMD_ENTERPIC,
    MCMD_EPISODE,
    MCMD_EXITANIM,
    MCMD_EXITPIC,
    MCMD_INTERBACKDROP,
    MCMD_INTERMUSIC,
    MCMD_INTERTEXT,
    MCMD_INTERTEXTSECRET,
    MCMD_LABEL,
    MCMD_LEVELNAME,
    MCMD_LEVELPIC,
    MCMD_LIQUID,
    MCMD_MUSIC,
    MCMD_MUSICARTIST,
    MCMD_MUSICTITLE,
    MCMD_NEXT,
    MCMD_NEXTSECRET,
    MCMD_NOBRIGHTMAP,
    MCMD_NOFREELOOK,
    MCMD_NOGRADUALLIGHTING,
    MCMD_NOINTERMISSION,
    MCMD_NOJUMP,
    MCMD_NOLIQUID,
    MCMD_NOMOUSELOOK,
    MCMD_PAR,
    MCMD_PARTIME,
    MCMD_PISTOLSTART,
    MCMD_SECRETNEXT,
    MCMD_SKY1,
    MCMD_SKYTEXTURE,
    MCMD_TITLEPATCH
};

bool            compat_corpsegibs;
bool            compat_floormove;
bool            compat_light;
bool            compat_limitpain;
bool            compat_nopassover;
bool            compat_stairs;
bool            compat_useblocking;
bool            compat_zombie;
bool            nograduallighting;

int             compat_corpsegibs_global = -1;
int             compat_floormove_global = -1;
int             compat_light_global = -1;
int             compat_limitpain_global = -1;
int             compat_nopassover_global = -1;
int             compat_stairs_global = -1;
int             compat_useblocking_global = -1;
int             compat_zombie_global = 1;
int             nograduallighting_global = -1;

bool            canmodify;
bool            transferredsky;

static int      MAPINFO;
char            *mapinfolump = "";

bool            samelevel;
bool            secretmap;

nodeformat_t    nodeformat;

const char *nodeformats[] =
{
    "Vanilla",
    ITALICS("DeeP"),
    ITALICS("ZDoom") " extended",
    ITALICS("ZDoom") " extended"
};

bool            boomcompatible;
bool            id24compatible = false;
bool            mbfcompatible;
bool            mbf21compatible = false;
bool            blockmaprebuilt;
bool            nofreelook = false;
bool            nojump = false;

const char *linespecials[NUMLINESPECIALS] =
{
    "",
    "DR Door open wait close (also monsters)",
    "W1 Door open stay",
    "W1 Door close stay",
    "W1 Door open wait close",
    "W1 Floor raise to lowest ceiling",
    "W1 Crusher start with fast damage",
    "S1 Stairs raise by 8",
    "W1 Stairs raise by 8",
    "S1 Floor raise donut (changes texture)",
    "W1 Lift lower wait raise",
    "S1 Exit level",
    "W1 Light change to brightest adjacent",
    "W1 Light change to 255",
    "S1 Floor raise by 32 (changes texture)",
    "S1 Floor raise by 24 (changes texture)",
    "W1 Door close wait open",
    "W1 Light start blinking",
    "S1 Floor raise to next highest floor",
    "W1 Floor lower to highest floor",
    "S1 Floor raise to next highest floor (changes texture)",
    "S1 Lift lower wait raise",
    "W1 Floor raise to next highest floor (changes texture)",
    "S1 Floor lower to lowest floor",
    "G1 Floor raise to lowest ceiling",
    "W1 Crusher start with slow damage",
    "DR Door (blue) open wait close",
    "DR Door (yellow) open wait close",
    "DR Door (red) open wait close",
    "S1 Door open wait close",
    "W1 Floor raise by shortest lower texture",
    "D1 Door open stay",
    "D1 Door (blue) open stay",
    "D1 Door (red) open stay",
    "D1 Door (yellow) open stay",
    "W1 Light change to 35",
    "W1 Floor lower to 8 above highest floor",
    "W1 Floor lower to lowest floor (changes texture)",
    "W1 Floor lower to lowest floor",
    "W1 Teleport",
    "W1 Ceiling raise to highest ceiling",
    "S1 Ceiling lower to floor",
    "SR Door close stay",
    "SR Ceiling lower to floor",
    "W1 Ceiling lower to 8 above floor",
    "SR Floor lower to highest floor",
    "GR Door open stay",
    "G1 Floor raise to next highest floor (changes texture)",
    "Scroll texture left",
    "S1 Ceiling lower to 8 above floor (perpetual slow crusher damage)",
    "S1 Door close stay",
    "S1 Exit level (goes to secret level)",
    "W1 Exit level",
    "W1 Floor start moving up and down",
    "W1 Floor stop moving",
    "S1 Floor raise to 8 below lowest ceiling (crushes)",
    "W1 Floor raise to 8 below lowest ceiling (crushes)",
    "W1 Crusher stop",
    "W1 Floor raise by 24",
    "W1 Floor raise by 24 (changes texture)",
    "SR Floor lower to lowest floor",
    "SR Door open stay",
    "SR Lift lower wait raise",
    "SR Door open wait close",
    "SR Floor raise to lowest ceiling",
    "SR Floor raise to 8 below lowest ceiling (crushes)",
    "SR Floor raise by 24 (changes texture)",
    "SR Floor raise by 32 (changes texture)",
    "SR Floor raise to next highest floor (changes texture)",
    "SR Floor raise to next highest floor",
    "SR Floor lower to 8 above highest floor",
    "S1 Floor lower to 8 above highest floor",
    "WR Ceiling lower to 8 above floor",
    "WR Crusher start with slow damage",
    "WR Crusher stop",
    "WR Door close stay",
    "WR Door close stay open",
    "WR Crusher start with fast damage",
    "SR Change texture and effect to nearest",
    "WR Light change to 35",
    "WR Light change to brightest adjacent",
    "WR Light change to 255",
    "WR Floor lower to lowest floor",
    "WR Floor lower to highest floor",
    "WR Floor lower to lowest floor (changes texture)",
    "Scroll texture right",
    "WR Door open stay",
    "WR Floor start moving up and down",
    "WR Lift lower wait raise",
    "WR Floor stop moving",
    "WR Door open wait close",
    "WR Floor raise to lowest ceiling",
    "WR Floor raise by 24",
    "WR Floor raise by 24 (changes texture)",
    "WR Floor raise to 8 below lowest ceiling (crushes)",
    "WR Floor raise to next highest floor (changes texture)",
    "WR Floor raise by shortest lower texture",
    "WR Teleport",
    "WR Floor lower to 8 above highest floor",
    "SR Door (blue) open stay (fast)",
    "W1 Stairs raise by 16 (fast)",
    "S1 Floor raise to lowest ceiling",
    "S1 Floor lower to highest floor",
    "S1 Door open stay",
    "W1 Light change to darkest adjacent",
    "WR Door open wait close (fast)",
    "WR Door open stay (fast)",
    "WR Door close stay (fast)",
    "W1 Door open wait close (fast)",
    "W1 Door open stay (fast)",
    "W1 Door close stay (fast)",
    "S1 Door open wait close (fast)",
    "S1 Door open stay (fast)",
    "S1 Door close stay (fast)",
    "SR Door open wait close (fast)",
    "SR Door open stay (fast)",
    "SR Door close stay (fast)",
    "DR Door open wait close (fast)",
    "D1 Door open stay (fast)",
    "W1 Floor raise to next highest floor",
    "WR Lift lower wait raise (fast)",
    "W1 Lift lower wait raise (fast)",
    "S1 Lift lower wait raise (fast)",
    "SR Lift lower wait raise (fast)",
    "W1 Exit level (goes to secret level)",
    "W1 Teleport (monsters only)",
    "WR Teleport (monsters only)",
    "S1 Stairs raise by 16 (fast)",
    "WR Floor raise to next highest floor",
    "WR Floor raise to next highest floor (fast)",
    "W1 Floor raise to next highest floor (fast)",
    "S1 Floor raise to next highest floor (fast)",
    "SR Floor raise to next highest floor (fast)",
    "S1 Door (blue) open stay (fast)",
    "SR Door (red) open stay (fast)",
    "S1 Door (red) open stay (fast)",
    "SR Door (yellow) open stay (fast)",
    "S1 Door (yellow) open stay (fast)",
    "SR Light change to 255",
    "SR Light change to 35",
    "S1 Floor raise by 512",
    "W1 Crusher start with slow damage (silent)",
    "W1 Floor raise by 512",
    "W1 Lift raise by 24 (changes texture)",
    "W1 Lift raise by 32 (changes texture)",
    "W1 Ceiling lower to floor (fast)",
    "W1 Floor raise donut (changes texture)",
    "WR Floor raise by 512",
    "WR Lift raise by 24 (changes texture)",
    "WR Lift raise by 32 (changes texture)",
    "WR Crusher start (silent)",
    "WR Ceiling raise to highest ceiling",
    "WR Ceiling lower to floor (fast)",
    "W1 Change texture and effect",
    "WR Change texture and effect",
    "WR Floor raise donut (changes texture)",
    "WR Light start blinking",
    "WR Light change to darkest adjacent",
    "S1 Floor raise by shortest lower texture",
    "S1 Floor lower to lowest floor (changes texture)",
    "S1 Floor raise by 24 (changes texture and effect)",
    "S1 Floor raise by 24",
    "S1 Lift perpetual lowest and highest floors",
    "S1 Lift stop",
    "S1 Crusher start (fast)",
    "S1 Crusher start (silent)",
    "S1 Ceiling raise to highest ceiling",
    "S1 Ceiling lower to 8 above floor",
    "S1 Crusher stop",
    "S1 Light change to brightest adjacent",
    "S1 Light change to 35",
    "S1 Light change to 255",
    "S1 Light start blinking",
    "S1 Light change to darkest adjacent",
    "S1 Teleport (also monsters)",
    "S1 Door close wait open (30 seconds)",
    "SR Floor raise by shortest lower texture",
    "SR Floor lower to lowest floor (changes texture)",
    "SR Floor raise by 512",
    "SR Floor raise by 24 (changes texture and effect)",
    "SR Floor raise by 24",
    "SR Lift perpetual lowest and highest floors",
    "SR Lift stop",
    "SR Crusher start (fast)",
    "SR Crusher start",
    "SR Crusher start (silent)",
    "SR Ceiling raise to highest ceiling",
    "SR Ceiling lower to 8 above floor",
    "SR Crusher stop",
    "S1 Change texture and effect",
    "SR Change texture and effect",
    "SR Floor raise donut (changes texture)",
    "SR Light change to brightest adjacent",
    "SR Light start blinking",
    "SR Light change to darkest adjacent",
    "SR Teleport (also monsters)",
    "SR Door close wait open (30 seconds)",
    "G1 Exit level",
    "G1 Exit level (goes to secret level)",
    "W1 Ceiling lower to lowest ceiling",
    "W1 Ceiling lower to highest floor",
    "WR Ceiling lower to lowest ceiling",
    "WR Ceiling lower to highest floor",
    "S1 Ceiling lower to lowest ceiling",
    "S1 Ceiling lower to highest floor",
    "SR Ceiling lower to lowest ceiling",
    "SR Ceiling lower to highest floor",
    "W1 Teleport (also monsters, silent, same angle)",
    "WR Teleport (also monsters, silent, same angle)",
    "S1 Teleport (also monsters, silent, same angle)",
    "SR Teleport (also monsters, silent, same angle)",
    "SR Lift raise to ceiling (instantly)",
    "WR Lift raise to ceiling (instantly)",
    "Floor change brightness to this brightness",
    "Scroll ceiling accelerates when sector height changes",
    "Scroll floor accelerates when sector height changes",
    "Scroll things accelerate when sector height changes",
    "Scroll floor and things accelerate when sector height changes",
    "Scroll wall accelerates when sector height changes",
    "W1 Floor lower to nearest floor",
    "WR Floor lower to nearest floor",
    "S1 Floor lower to nearest floor",
    "SR Floor lower to nearest floor",
    "Friction tagged sector",
    "Wind according to line vector",
    "Current according to line vector",
    "Wind/current by push/pull thing in sector",
    "W1 Lift raise to next highest floor (fast)",
    "WR Lift raise to next highest floor (fast)",
    "S1 Lift raise to next highest floor (fast)",
    "SR Lift raise to next highest floor (fast)",
    "W1 Lift lower to next lowest floor (fast)",
    "WR Lift lower to next lowest floor (fast)",
    "S1 Lift lower to next lowest floor (fast)",
    "SR Lift lower to next lowest floor (fast)",
    "W1 Lift move to same floor height (fast)",
    "WR Lift move to same floor height (fast)",
    "S1 Lift move to same floor height (fast)",
    "SR Lift move to same floor height (fast)",
    "W1 Change texture and effect to nearest",
    "WR Change texture and effect to nearest",
    "S1 Change texture and effect to nearest",
    "Create fake ceiling and floor",
    "W1 Teleport to line with same tag (silent, same angle)",
    "WR Teleport to line with same tag (silent, same angle)",
    "Scroll ceiling when sector changes height",
    "Scroll floor when sector changes height",
    "Scroll move things when sector changes height",
    "Scroll floor and move things when sector changes height",
    "Scroll wall when sector changes height",
    "Scroll ceiling according to line vector",
    "Scroll floor according to line vector",
    "Scroll move things according to line vector",
    "Scroll floor, move things",
    "Scroll wall according to line vector",
    "Scroll wall using sidedef offsets",
    "WR Stairs raise by 8",
    "WR Stairs raise by 16 (fast)",
    "SR Stairs raise by 8",
    "SR Stairs raise by 16 (fast)",
    "Translucent (middle texture)",
    "Ceiling change brightness to this brightness",
    "W1 Teleport to line with same tag (silent, reversed angle)",
    "WR Teleport to line with same tag (silent, reversed angle)",
    "W1 Teleport to line with same tag (monsters only, silent, reversed angle)",
    "WR Teleport to line with same tag (monsters only, silent, reversed angle)",
    "W1 Teleport to line with same tag (monsters only, silent)",
    "WR Teleport to line with same tag (monsters only, silent)",
    "W1 Teleport (monsters only, silent)",
    "WR Teleport (monsters only, silent)",
    "",
    "Transfer sky texture to tagged sectors",
    "Transfer sky texture to tagged sectors (flipped)"
};

static const char *sectorspecials[] =
{
    "",
    "Light blinks (randomly)",
    "Light blinks (0.5 sec.)",
    "Light blinks (1 sec.)",
    "Damage -10% or -20% health and light blinks (0.5 sec.)",
    "Damage -5% or -10% health",
    "",
    "Damage -2% or -5% health",
    "Light glows (1+ sec.)",
    "Secret",
    "Door close stay (after 30 sec.)",
    "Damage -10% or -20% health and end level",
    "Light blinks (0.5 sec. synchronized)",
    "Light blinks (1 sec. synchronized)",
    "Door open close (opens after 5 min.)",
    "",
    "Damage -10% or -20% health",
    "Light flickers (randomly)"
};

static fixed_t GetOffset(const vertex_t *v1, const vertex_t *v2)
{
    const fixed_t   dx = (v1->x - v2->x) >> FRACBITS;
    const fixed_t   dy = (v1->y - v2->y) >> FRACBITS;

    return ((fixed_t)(sqrt((double)dx * dx + (double)dy * dy)) << FRACBITS);
}

// e6y: Smart malloc
// Used by P_SetupLevel() for smart data loading
// Do nothing if level is the same
static void *malloc_IfSameLevel(void *p, size_t size)
{
    return (!samelevel || !p ? malloc(size) : p);
}

// e6y: Smart calloc
// Used by P_SetupLevel() for smart data loading
// Clear the memory without allocation if level is the same
static void *calloc_IfSameLevel(void *p, size_t n1, size_t n2)
{
    if (!samelevel)
        return calloc(n1, n2);
    else
    {
        memset(p, 0, n1 * n2);
        return p;
    }
}

//
// P_LoadVertexes
//
static void P_LoadVertexes(int lump)
{
    const mapvertex_t   *data = (const mapvertex_t *)W_CacheLumpNum(lump);

    numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);
    vertexes = calloc_IfSameLevel(vertexes, numvertexes, sizeof(vertex_t));

    if (!data || !numvertexes)
        I_Error("There are no vertexes in this map.");
    else
    {
        // Copy and convert vertex coordinates,
        // internal representation as fixed.
        for (int i = 0; i < numvertexes; i++)
        {
            vertexes[i].x = SHORT(data[i].x) << FRACBITS;
            vertexes[i].y = SHORT(data[i].y) << FRACBITS;

            // Apply any map-specific fixes.
            if (canmodify && r_fixmaperrors && gamemode != shareware)
                for (int j = 0; vertexfix[j].mission != -1; j++)
                    if (gamemission == vertexfix[j].mission && gameepisode == vertexfix[j].episode && gamemap == vertexfix[j].map
                        && i == vertexfix[j].vertex && vertexes[i].x == vertexfix[j].oldx << FRACBITS
                        && vertexes[i].y == vertexfix[j].oldy << FRACBITS)
                    {
                        char    *temp = commify(vertexfix[j].vertex);

                        C_Warning(2, "Vertex %s has been moved from (%i, %i) to (%i, %i).",
                            temp, vertexfix[j].oldx, vertexfix[j].oldy, vertexfix[j].newx, vertexfix[j].newy);

                        vertexes[i].x = vertexfix[j].newx << FRACBITS;
                        vertexes[i].y = vertexfix[j].newy << FRACBITS;
                        free(temp);
                        break;
                    }
        }
    }

    // Free buffer memory.
    W_ReleaseLumpNum(lump);
}

static void P_CheckLinedefs(void)
{
    line_t  *ld = lines;

    numspeciallines = 0;
    numvisiblelines = 0;

    for (int i = numlines; i--; ld++)
    {
        unsigned short  special = ld->special;

        if (!special)
        {
            if (ld->tag)
            {
                char    *temp1 = commify(ld->id);
                char    *temp2 = commify(ld->tag);

                if (ld->tag < 0 || P_FindSectorFromLineTag(ld, -1) == -1)
                    C_Warning(2, "Linedef %s has an unused tag of %s and no line special.", temp1, temp2);
                else
                    C_Warning(2, "Linedef %s has a tag of %s but no line special.", temp1, temp2);

                free(temp1);
                free(temp2);
            }
        }
        else
        {
            if (!P_CheckTag(ld))
            {
                char    *temp1 = commify(ld->id);
                char    *temp2 = commify(special);

                if (special < NUMLINESPECIALS)
                {
                    if (linespecials[special])
                        C_Warning(2, "Linedef %s has %s line special of %s (\"%s\") but no tag.",
                            temp1, (special < BOOMLINESPECIALS ? "a" : (special < MBFLINESPECIALS ? "a " ITALICS("BOOM")
                            "-compatible" : (special < MBF21LINESPECIALS ? "an " ITALICS("MBF") "-compatible" : "an " ITALICS("MBF21")
                            "-compatible"))), temp2, linespecials[special]);
                    else
                        C_Warning(2, "Linedef %s has an invalid line special of %s.", temp1, temp2);
                }
                else if (special >= GenCrusherBase && special < GenEnd)
                    C_Warning(2, "Linedef %s has a " ITALICS("BOOM") "-compatible generalized line special of %s but no tag.", temp1, temp2);
                else
                    C_Warning(2, "Linedef %s has an invalid line special of %s.", temp1, temp2);

                free(temp1);
                free(temp2);
            }
            else if (ld->tag < 0 || P_FindSectorFromLineTag(ld, -1) == -1)
            {
                char    *temp1 = commify(ld->id);
                char    *temp2 = commify(special);
                char    *temp3 = commify(ld->tag);

                if (special < NUMLINESPECIALS)
                {
                    if (linespecials[special])
                        C_Warning(2, "Linedef %s has %s line special of %s (\"%s\") but an unused tag of %s.",
                            temp1, (special < BOOMLINESPECIALS ? "a" : (special < MBFLINESPECIALS ? "a " ITALICS("BOOM")
                            "-compatible" : (special < MBF21LINESPECIALS ? "an " ITALICS("MBF") "-compatible" : "an " ITALICS("MBF21")
                            "-compatible"))), temp2, linespecials[special], temp3);
                    else
                        C_Warning(2, "Linedef %s has an invalid line special of %s.", temp1, temp2);
                }
                else if (special >= GenCrusherBase && special < GenEnd)
                    C_Warning(2, "Linedef %s has a " ITALICS("BOOM") "-compatible generalized line special of %s but an unused tag of %s.",
                        temp1, temp2, temp3);
                else
                    C_Warning(2, "Linedef %s has an invalid line special of %s.", temp1, temp2);

                free(temp1);
                free(temp2);
                free(temp3);
            }
            else
                numspeciallines++;
        }

        if (!(ld->flags & ML_DONTDRAW))
            numvisiblelines++;
    }
}

//
// P_LoadSegs
//
static void P_LoadSegs(int lump)
{
    const mapseg_t  *data = (const mapseg_t *)W_CacheLumpNum(lump);

    numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
    segs = calloc_IfSameLevel(segs, numsegs, sizeof(seg_t));

    if (!data || !numsegs)
        I_Error("There are no segs in this map.");

    for (int j = 0; linefix[j].mission != -1; j++)
        linefix[j].fixed = false;

    for (int i = 0; i < numsegs; i++)
    {
        seg_t                   *li = segs + i;
        const mapseg_t          *ml = data + i;
        const unsigned short    v1 = (unsigned short)SHORT(ml->v1);
        const unsigned short    v2 = (unsigned short)SHORT(ml->v2);
        const int               linedefnum = (unsigned short)SHORT(ml->linedef);
        int                     side;
        line_t                  *ldef;

        if (linedefnum >= numlines)
            I_Error("Seg %s references an invalid linedef of %s.",
                commify(i), commify(linedefnum));

        ldef = lines + linedefnum;
        li->linedef = ldef;
        side = SHORT(ml->side);

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            char    *temp = commify(i);

            C_Warning(2, "Seg %s has an invalid side. It has been changed to 1.", temp);
            side = 1;
            free(temp);
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
            I_Error("Linedef %s for seg %s references an invalid sidedef of %s.",
                commify(linedefnum), commify(i), commify(ldef->sidenum[side]));

        li->sidedef = sides + ldef->sidenum[side];

        // cph 09/30/06: our frontsector can be the second side of the
        // linedef, so must check for NO_INDEX in case we are incorrectly
        // referencing the back of a 1S line
        if (ldef->sidenum[side] != NO_INDEX)
            li->frontsector = sides[ldef->sidenum[side]].sector;
        else
        {
            char    *temp = commify(i);

            C_Warning(2, "The %s of seg %s has no sidedef.", (side ? "back" : "front"), temp);
            li->frontsector = NULL;
            free(temp);
        }

        // killough 05/03/98: ignore 2s flag if second sidedef missing:
        if ((ldef->flags & ML_TWOSIDED) && ldef->sidenum[side ^ 1] != NO_INDEX)
            li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
        else
        {
            li->backsector = NULL;
            ldef->flags &= ~ML_TWOSIDED;
        }

        // e6y
        // check and fix wrong references to non-existent vertexes
        // see E1M9 @ NIVELES.WAD
        // <https://www.doomworld.com/idgames/levels/doom/megawads/niveles>
        if (v1 >= numvertexes || v2 >= numvertexes)
        {
            if (v1 >= numvertexes)
            {
                char    *temp1 = commify(i);
                char    *temp2 = commify(v1);

                C_Warning(2, "Seg %s references an invalid vertex of %s.", temp1, temp2);
                free(temp1);
                free(temp2);
            }

            if (v2 >= numvertexes)
            {
                char    *temp1 = commify(i);
                char    *temp2 = commify(v2);

                C_Warning(2, "Seg %s references an invalid vertex of %s.", temp1, temp2);
                free(temp1);
                free(temp2);
            }

            if (li->sidedef == sides + li->linedef->sidenum[0])
            {
                li->v1 = lines[ml->linedef].v1;
                li->v2 = lines[ml->linedef].v2;
            }
            else
            {
                li->v1 = lines[ml->linedef].v2;
                li->v2 = lines[ml->linedef].v1;
            }
        }
        else
        {
            li->v1 = &vertexes[v1];
            li->v2 = &vertexes[v2];
        }

        li->angle = (SHORT(ml->angle)) << FRACBITS;
        li->offset = GetOffset(li->v1, (side ? ldef->v2 : ldef->v1));

        // [BH] Apply any map-specific fixes.
        if (canmodify && r_fixmaperrors && gamemode != shareware)
            for (int j = 0; linefix[j].mission != -1; j++)
                if (gamemission == linefix[j].mission && gameepisode == linefix[j].episode && gamemap == linefix[j].map
                    && linedefnum == linefix[j].linedef && side == linefix[j].side
                    && !linefix[j].fixed)
                {
                    linefix[j].fixed = true;

                    if (*linefix[j].toptexture)
                    {
                        int     texture = R_TextureNumForName(linefix[j].toptexture);
                        char    *temp = commify(linedefnum);

                        if (!texture)
                            C_Warning(2, "The unused top texture of linedef %s has been removed.", temp);
                        else
                        {
                            if (!li->sidedef->toptexture)
                                C_Warning(2, "The missing top texture of linedef %s has been changed to " BOLD("%.8s") ".",
                                    temp, linefix[j].toptexture);
                            else
                                C_Warning(2, "The top texture of linedef %s has been changed from " BOLD("%.8s") " to " BOLD("%.8s") ".",
                                    temp, textures[li->sidedef->toptexture]->name, linefix[j].toptexture);
                        }

                        li->sidedef->toptexture = texture;
                        free(temp);
                    }

                    if (*linefix[j].middletexture)
                    {
                        int     texture = R_TextureNumForName(linefix[j].middletexture);
                        char    *temp = commify(linedefnum);

                        if (!texture)
                            C_Warning(2, "The unused middle texture of linedef %s has been removed.", temp);
                        else
                        {
                            if (!li->sidedef->midtexture)
                                C_Warning(2, "The missing middle texture of linedef %s has been changed to " BOLD("%.8s") ".",
                                    temp, linefix[j].middletexture);
                            else
                                C_Warning(2, "The middle texture of linedef %s has been changed from " BOLD("%.8s") " to "
                                    BOLD("%.8s") ".", temp, textures[li->sidedef->midtexture]->name, linefix[j].middletexture);
                        }

                        li->sidedef->midtexture = texture;
                        free(temp);
                    }

                    if (*linefix[j].bottomtexture)
                    {
                        int     texture = R_TextureNumForName(linefix[j].bottomtexture);
                        char    *temp = commify(linedefnum);

                        if (!texture)
                            C_Warning(2, "The unused bottom texture of linedef %s has been removed.", temp);
                        else
                        {
                            if (!li->sidedef->bottomtexture)
                                C_Warning(2, "The missing bottom texture of linedef %s has been changed to " BOLD("%.8s") ".",
                                    temp, linefix[j].bottomtexture);
                            else
                                C_Warning(2, "The bottom texture of linedef %s has been changed from " BOLD("%.8s") " to "
                                    BOLD("%.8s") ".", temp, textures[li->sidedef->bottomtexture]->name, linefix[j].bottomtexture);
                        }

                        li->sidedef->bottomtexture = texture;
                        free(temp);
                    }

                    if (linefix[j].offset != DEFAULT)
                    {
                        char    *temp1 = commify(linedefnum);
                        char    *temp2 = commify(li->offset >> FRACBITS);
                        char    *temp3 = commify(linefix[j].offset);

                        C_Warning(2, "The horizontal texture offset of linedef %s has been changed from %s to %s.", temp1, temp2, temp3);

                        li->offset = linefix[j].offset << FRACBITS;
                        li->sidedef->textureoffset = 0;
                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    if (linefix[j].rowoffset != DEFAULT)
                    {
                        char    *temp1 = commify(linedefnum);
                        char    *temp2 = commify(li->sidedef->rowoffset >> FRACBITS);
                        char    *temp3 = commify(linefix[j].rowoffset);

                        C_Warning(2, "The vertical texture offset of linedef %s has been changed from %s to %s.", temp1, temp2, temp3);

                        li->sidedef->rowoffset = linefix[j].rowoffset << FRACBITS;
                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    if (linefix[j].flags != DEFAULT)
                    {
                        char    *temp1 = commify(linedefnum);
                        char    *temp2 = commify(li->linedef->flags);
                        char    *temp3 = commify(linefix[j].flags);

                        C_Warning(2, "The flags of linedef %s have been changed from %s to %s.", temp1, temp2, temp3);

                        if (li->linedef->flags & linefix[j].flags)
                            li->linedef->flags &= ~linefix[j].flags;
                        else
                            li->linedef->flags |= linefix[j].flags;

                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    if (linefix[j].special != DEFAULT)
                    {
                        if (linefix[j].special)
                        {
                            char            *temp1 = commify(linedefnum);
                            char            *temp2 = commify(linefix[j].special);
                            unsigned short  special = li->linedef->special;

                            if (special > 0 && special < NUMLINESPECIALS)
                            {
                                char    *temp3 = commify(linefix[j].special);

                                C_Warning(2, "%s line special of linedef %s has been changed from %s (\"%s\") to %s (\"%s\").",
                                    (special < BOOMLINESPECIALS ? "The" : (special < MBFLINESPECIALS ?
                                    "The " ITALICS("BOOM") "-compatible" : (special < MBF21LINESPECIALS ? "The " ITALICS("MBF")
                                    "-compatible" : "The " ITALICS("MBF21") "-compatible"))), temp1, temp2,
                                    linespecials[special], temp3, linespecials[linefix[j].special]);
                                free(temp3);
                            }
                            else
                                C_Warning(2, "Linedef %s now has %s line special %s (\"%s\").",
                                    temp1, (linefix[j].special < BOOMLINESPECIALS ? "the" : (linefix[j].special < MBFLINESPECIALS ?
                                    "The " ITALICS("BOOM") "-compatible" : (linefix[j].special < MBF21LINESPECIALS ? "the " ITALICS("MBF")
                                    "-compatible" : "the " ITALICS("MBF21") "-compatible"))), temp2, linespecials[linefix[j].special]);

                            free(temp1);
                            free(temp2);
                        }
                        else
                        {
                            char    *temp = commify(linedefnum);

                            C_Warning(2, "%s line special of linedef %s has been removed.",
                                (li->linedef->special < BOOMLINESPECIALS ? "The" : (li->linedef->special < MBFLINESPECIALS ?
                                "The " ITALICS("BOOM") "-compatible" : (li->linedef->special < MBF21LINESPECIALS ? "The " ITALICS("MBF")
                                "-compatible" : "The " ITALICS("MBF21") "-compatible"))), temp);
                            free(temp);
                        }

                        li->linedef->special = linefix[j].special;
                    }

                    if (linefix[j].tag != DEFAULT)
                    {
                        char    *temp1 = commify(linedefnum);
                        char    *temp2 = commify(li->linedef->tag);

                        if (linefix[j].tag)
                        {
                            char    *temp3 = commify(linefix[j].tag);

                            C_Warning(2, "Linedef %s's tag has been changed from %s to %s.", temp1, temp2, temp3);
                            free(temp3);
                        }
                        else
                            C_Warning(2, "Linedef %s's tag of %s has been removed.", temp1, temp2);

                        li->linedef->tag = linefix[j].tag;
                        free(temp1);
                        free(temp2);
                    }

                    break;
                }

        if (li->linedef->special >= MBF21LINESPECIALS && li->linedef->special < NUMLINESPECIALS)
            mbf21compatible = true;

        if (li->linedef->special >= MBFLINESPECIALS && li->linedef->special < MBF21LINESPECIALS)
            mbfcompatible = true;

        if (li->linedef->special >= BOOMLINESPECIALS)
            boomcompatible = true;
    }

    W_ReleaseLumpNum(lump);

    P_CheckLinedefs();
}

static void P_LoadSegs_V4(int lump)
{
    const mapseg_v4_t   *data = (const mapseg_v4_t *)W_CacheLumpNum(lump);

    numsegs = W_LumpLength(lump) / sizeof(mapseg_v4_t);
    segs = calloc_IfSameLevel(segs, numsegs, sizeof(seg_t));

    if (!data || !numsegs)
        I_Error("There are no segs in this map.");

    for (int i = 0; i < numsegs; i++)
    {
        seg_t               *li = segs + i;
        const mapseg_v4_t   *ml = data + i;
        const int           v1 = ml->v1;
        const int           v2 = ml->v2;
        const int           linedefnum = (unsigned short)SHORT(ml->linedef);
        int                 side;
        line_t              *ldef;

        // e6y: check for wrong indexes
        if (linedefnum >= numlines)
            I_Error("Seg %s references an invalid linedef of %s.", commify(i), commify(linedefnum));

        ldef = lines + linedefnum;
        li->linedef = ldef;
        side = SHORT(ml->side);

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            char    *temp = commify(i);

            C_Warning(2, "Seg %s has an invalid side. It has been changed to 1.", temp);
            side = 1;
            free(temp);
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
            I_Error("Linedef %s for seg %s references an invalid sidedef of %s.",
                commify(linedefnum), commify(i), commify(ldef->sidenum[side]));

        li->sidedef = sides + ldef->sidenum[side];

        // cph 09/30/06: our frontsector can be the second side of the
        // linedef, so must check for NO_INDEX in case we are incorrectly
        // referencing the back of a 1S line
        if (ldef->sidenum[side] != NO_INDEX)
            li->frontsector = sides[ldef->sidenum[side]].sector;
        else
        {
            char    *temp = commify(i);

            C_Warning(2, "The %s of seg %s has no sidedef.", (side ? "back" : "front"), temp);
            li->frontsector = NULL;
            free(temp);
        }

        // killough 05/03/98: ignore 2s flag if second sidedef missing:
        if ((ldef->flags & ML_TWOSIDED) && ldef->sidenum[side ^ 1] != NO_INDEX)
            li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
        else
        {
            li->backsector = NULL;
            ldef->flags &= ~ML_TWOSIDED;
        }

        // e6y
        // check and fix wrong references to non-existent vertexes
        // see E1M9 @ NIVELES.WAD
        // <https://www.doomworld.com/idgames/levels/doom/megawads/niveles>
        if (v1 >= numvertexes || v2 >= numvertexes)
        {
            if (v1 >= numvertexes)
            {
                char    *temp1 = commify(i);
                char    *temp2 = commify(v1);

                C_Warning(2, "Seg %s references an invalid vertex of %s.", temp1, temp2);
                free(temp1);
                free(temp2);
            }

            if (v2 >= numvertexes)
            {
                char    *temp1 = commify(i);
                char    *temp2 = commify(v2);

                C_Warning(2, "Seg %s references an invalid vertex of %s.", temp1, temp2);
                free(temp1);
                free(temp2);
            }

            if (li->sidedef == sides + li->linedef->sidenum[0])
            {
                li->v1 = lines[ml->linedef].v1;
                li->v2 = lines[ml->linedef].v2;
            }
            else
            {
                li->v1 = lines[ml->linedef].v2;
                li->v2 = lines[ml->linedef].v1;
            }
        }
        else
        {
            li->v1 = &vertexes[v1];
            li->v2 = &vertexes[v2];
        }

        li->angle = (SHORT(ml->angle)) << FRACBITS;
        li->offset = GetOffset(li->v1, (side ? ldef->v2 : ldef->v1));

        if (li->linedef->special >= MBF21LINESPECIALS && li->linedef->special < NUMLINESPECIALS)
            mbf21compatible = true;

        if (li->linedef->special >= MBFLINESPECIALS && li->linedef->special < MBF21LINESPECIALS)
            mbfcompatible = true;

        if (li->linedef->special >= BOOMLINESPECIALS)
            boomcompatible = true;
    }

    W_ReleaseLumpNum(lump);

    P_CheckLinedefs();
}

//
// P_LoadSubsectors
//
static void P_LoadSubsectors(int lump)
{
    const mapsubsector_t    *data = (const mapsubsector_t *)W_CacheLumpNum(lump);

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_t);
    subsectors = calloc_IfSameLevel(subsectors, numsubsectors, sizeof(subsector_t));

    if (!data || !numsubsectors)
        I_Error("There are no subsectors in this map.");
    else
    {
        for (int i = 0; i < numsubsectors; i++)
        {
            subsectors[i].numlines = (unsigned short)SHORT(data[i].numsegs);
            subsectors[i].firstline = (unsigned short)SHORT(data[i].firstseg);
        }

        W_ReleaseLumpNum(lump);
    }
}

static void P_LoadSubsectors_V4(int lump)
{
    const mapsubsector_v4_t *data = (const mapsubsector_v4_t *)W_CacheLumpNum(lump);

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_v4_t);
    subsectors = calloc_IfSameLevel(subsectors, numsubsectors, sizeof(subsector_t));

    if (!data || !numsubsectors)
        I_Error("There are no subsectors in this map.");
    else
    {
        for (int i = 0; i < numsubsectors; i++)
        {
            subsectors[i].numlines = (int)data[i].numsegs;
            subsectors[i].firstline = (int)data[i].firstseg;
        }

        W_ReleaseLumpNum(lump);
    }
}

//
// P_LoadSectors
//
static void P_LoadSectors(int lump)
{
    mapsector_t *data = W_CacheLumpNum(lump);

    numsectors = W_LumpLength(lump) / sizeof(mapsector_t);
    sectors = calloc_IfSameLevel(sectors, numsectors, sizeof(sector_t));
    numdamaging = 0;

    for (int i = 0; i < numsectors; i++)
    {
        sector_t    *ss = sectors + i;
        mapsector_t *ms = data + i;

        ss->id = i;
        ss->floorheight = SHORT(ms->floorheight) << FRACBITS;
        ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
        ss->floorpic = R_FlatNumForName(ms->floorpic);
        ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
        ss->lightlevel = ss->oldlightlevel = MAX(0, SHORT(ms->lightlevel));
        ss->special = SHORT(ms->special);
        ss->tag = SHORT(ms->tag);

        if (ss->tag == -1)
            ss->tag = 0;

        ss->nextsec = -1;
        ss->prevsec = -1;

        ss->oldceilinggametime = -1;
        ss->oldfloorgametime = -1;
        ss->oldceilingoffsetgametime = -1;
        ss->oldflooroffsetgametime = -1;

        // [BH] Apply any level-specific fixes.
        if (canmodify && r_fixmaperrors && gamemode != shareware)
            for (int j = 0; sectorfix[j].mission != -1; j++)
                if (gamemission == sectorfix[j].mission && gameepisode == sectorfix[j].episode && gamemap == sectorfix[j].map
                    && i == sectorfix[j].sector)
                {
                    if (*sectorfix[j].floorpic)
                    {
                        char    *temp = commify(sectorfix[j].sector);

                        C_Warning(2, "The texture of sector %s's floor has been changed from " BOLD("%.8s") " to " BOLD("%.8s") ".",
                            temp, lumpinfo[ss->floorpic + firstflat]->name, sectorfix[j].floorpic);

                        ss->floorpic = R_FlatNumForName(sectorfix[j].floorpic);
                        free(temp);
                    }

                    if (*sectorfix[j].ceilingpic)
                    {
                        char    *temp = commify(sectorfix[j].sector);

                        C_Warning(2, "The texture of sector %s's ceiling has been changed from " BOLD("%.8s") " to " BOLD("%.8s") ".",
                            temp, lumpinfo[ss->ceilingpic + firstflat]->name, sectorfix[j].ceilingpic);

                        ss->ceilingpic = R_FlatNumForName(sectorfix[j].ceilingpic);
                        free(temp);
                    }

                    if (sectorfix[j].floorheight != DEFAULT)
                    {
                        char    *temp1 = commify(sectorfix[j].sector);
                        char    *temp2 = commify(ss->floorheight >> FRACBITS);
                        char    *temp3 = commify(sectorfix[j].floorheight);

                        C_Warning(2, "The height of sector %s's floor has been changed from %s to %s.", temp1, temp2, temp3);

                        ss->floorheight = sectorfix[j].floorheight << FRACBITS;
                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    if (sectorfix[j].ceilingheight != DEFAULT)
                    {
                        char    *temp1 = commify(sectorfix[j].sector);
                        char    *temp2 = commify(ss->ceilingheight >> FRACBITS);
                        char    *temp3 = commify(sectorfix[j].ceilingheight);

                        C_Warning(2, "The height of sector %s's ceiling has been changed from %s to %s.", temp1, temp2, temp3);

                        ss->ceilingheight = sectorfix[j].ceilingheight << FRACBITS;
                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    if (sectorfix[j].special != DEFAULT)
                    {
                        char    *temp = commify(sectorfix[j].sector);
                        char    oldspecial[18];
                        char    newspecial[18];

                        M_StringCopy(oldspecial, sectorspecials[ss->special], sizeof(oldspecial));
                        M_StringCopy(newspecial, sectorspecials[sectorfix[j].special], sizeof(newspecial));

                        if (english == english_british)
                        {
                            M_AmericanToBritishEnglish(oldspecial);
                            M_AmericanToBritishEnglish(newspecial);
                        }

                        if (ss->special)
                        {
                            if (sectorfix[j].special)
                                C_Warning(2, "Sector %s's special has been changed from %i (\"%s\") to %i (\"%s\").",
                                    temp, ss->special, oldspecial, sectorfix[j].special, newspecial);
                            else
                                C_Warning(2, "Sector %s's special of %i (\"%s\") has been removed.",
                                    temp, ss->special, oldspecial);
                        }
                        else
                            C_Warning(2, "Sector %s now has a special of %i (\"%s\").",
                                temp, sectorfix[j].special, newspecial);

                        ss->special = sectorfix[j].special;
                        free(temp);
                    }

                    if (ss->tag == sectorfix[j].oldtag)
                    {
                        char    *temp1 = commify(sectorfix[j].sector);
                        char    *temp2 = commify(ss->tag);
                        char    *temp3 = commify(sectorfix[j].newtag);

                        if (!sectorfix[j].oldtag)
                            C_Warning(2, "Sector %s's tag of %s has been removed.", temp1, temp2);
                        else if (ss->tag)
                            C_Warning(2, "Sector %s's tag has been changed from %s to %s.", temp1, temp2, temp3);
                        else
                            C_Warning(2, "Sector %s now has a tag of %s.", temp1, temp3);

                        ss->tag = sectorfix[j].newtag;
                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    break;
                }

        // [AM] Sector interpolation. Even if we're not running uncapped, the renderer still uses this data.
        ss->oldfloorheight = ss->floorheight;
        ss->interpfloorheight = ss->floorheight;
        ss->oldceilingheight = ss->ceilingheight;
        ss->interpceilingheight = ss->ceilingheight;

        if (ss->special < 32)
        {
            switch (ss->special)
            {
                case DamageNegative10Or20PercentHealthAndLightBlinks_2Hz:
                case DamageNegative5Or10PercentHealth:
                case DamageNegative2Or5PercentHealth:
                case DamageNegative10Or20PercentHealthAndEndLevel:
                case DamageNegative10Or20PercentHealth:
                    numdamaging++;
                    break;
            }
        }
        else if (ss->special & DEATH_MASK)
        {
            switch ((ss->special & DAMAGE_MASK) >> DAMAGE_SHIFT)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                    numdamaging++;
                    mbf21compatible = true;
                    break;
            }
        }
        else
        {
            switch ((ss->special & DAMAGE_MASK) >> DAMAGE_SHIFT)
            {
                case 1:
                case 2:
                case 3:
                    numdamaging++;
                    boomcompatible = true;
                    break;
            }
        }
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadNodes
//
static void P_LoadNodes(int lump)
{
    const byte  *data = W_CacheLumpNum(lump);

    numnodes = W_LumpLength(lump) / sizeof(mapnode_t);
    nodes = malloc_IfSameLevel(nodes, numnodes * sizeof(node_t));

    if (!data || !numnodes)
    {
        if (numsubsectors == 1)
            C_Warning(2, "There are no nodes and only one subsector in this map.");
        else
            I_Error("There are no nodes in this map.");
    }
    else
    {
        for (int i = 0; i < numnodes; i++)
        {
            node_t          *no = nodes + i;
            const mapnode_t *mn = (const mapnode_t *)data + i;

            no->x = SHORT(mn->x) << FRACBITS;
            no->y = SHORT(mn->y) << FRACBITS;
            no->dx = SHORT(mn->dx) << FRACBITS;
            no->dy = SHORT(mn->dy) << FRACBITS;

            for (int j = 0; j < 2; j++)
            {
                no->children[j] = (unsigned short)SHORT(mn->children[j]);

                if (no->children[j] == 0xFFFF)
                    no->children[j] = -1;
                else if (no->children[j] & 0x8000)
                {
                    // Convert to extended type
                    no->children[j] &= ~0x8000;

                    // haleyjd 11/06/10: check for invalid subsector reference
                    if (no->children[j] >= numsubsectors)
                    {
                        char    *temp1 = commify(i);
                        char    *temp2 = commify(no->children[j]);

                        C_Warning(2, "Node %s references an invalid subsector of %s.", temp1, temp2);
                        no->children[j] = 0;
                        free(temp1);
                        free(temp2);
                    }

                    no->children[j] |= NF_SUBSECTOR;
                }

                for (int k = 0; k < 4; k++)
                    no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
            }
        }

        W_ReleaseLumpNum(lump);
    }
}

static void P_LoadNodes_V4(int lump)
{
    const byte  *data = W_CacheLumpNum(lump);

    numnodes = ((size_t)W_LumpLength(lump) - 8) / sizeof(mapnode_v4_t);
    nodes = malloc_IfSameLevel(nodes, numnodes * sizeof(node_t));

    // skip header
    data = data + 8;

    if (!data || !numnodes)
    {
        if (numsubsectors == 1)
            C_Warning(2, "There are no nodes and only one subsector in this map.");
        else
            I_Error("There are no nodes in this map.");
    }

    for (int i = 0; i < numnodes; i++)
    {
        node_t              *no = nodes + i;
        const mapnode_v4_t  *mn = (const mapnode_v4_t *)data + i;

        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;

        for (int j = 0; j < 2; j++)
        {
            no->children[j] = (unsigned int)(mn->children[j]);

            for (int k = 0; k < 4; k++)
                no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);
}

// MB 2020-03-01: Fix endianness for 32-bit ZDoom nodes
// <https://zdoom.org/wiki/Node#ZDoom_extended_nodes>
static void P_LoadZSegs(const byte *data)
{
    for (int i = 0; i < numsegs; i++)
    {
        line_t              *ldef;
        unsigned int        v1, v2;
        unsigned int        linedefnum;
        unsigned char       side;
        seg_t               *li = segs + i;
        const mapseg_znod_t *ml = (const mapseg_znod_t *)data + i;

        v1 = LONG(ml->v1);
        v2 = LONG(ml->v2);

        linedefnum = (unsigned short)SHORT(ml->linedef);

        // e6y: check for wrong indexes
        if (linedefnum >= (unsigned int)numlines)
            I_Error("Seg %s references an invalid linedef of %s.",
                commify(i), commify(linedefnum));

        ldef = lines + linedefnum;
        li->linedef = ldef;
        side = ml->side;

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            char    *temp = commify(i);

            C_Warning(2, "Seg %s has an invalid side. It has been changed to 1.", temp);
            side = 1;
            free(temp);
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
            I_Error("Linedef %s for seg %s references an invalid sidedef of %s.",
                commify(linedefnum), commify(i), commify(ldef->sidenum[side]));

        li->sidedef = sides + ldef->sidenum[side];

        // cph 09/30/06: our frontsector can be the second side of the
        // linedef, so must check for NO_INDEX in case we are incorrectly
        // referencing the back of a 1S line
        if (ldef->sidenum[side] != NO_INDEX)
            li->frontsector = sides[ldef->sidenum[side]].sector;
        else
        {
            char    *temp = commify(i);

            C_Warning(2, "The %s of seg %s has no sidedef.", (side ? "back" : "front"), temp);
            li->frontsector = NULL;
            free(temp);
        }

        if ((ldef->flags & ML_TWOSIDED) && ldef->sidenum[side ^ 1] != NO_INDEX)
            li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
        else
        {
            li->backsector = NULL;
            ldef->flags &= ~ML_TWOSIDED;
        }

        li->v1 = &vertexes[v1];
        li->v2 = &vertexes[v2];

        li->angle = R_PointToAngle2(segs[i].v1->x, segs[i].v1->y, segs[i].v2->x, segs[i].v2->y);
        li->offset = GetOffset(li->v1, (side ? ldef->v2 : ldef->v1));

        if (li->linedef->special >= MBF21LINESPECIALS && li->linedef->special < NUMLINESPECIALS)
            mbf21compatible = true;

        if (li->linedef->special >= MBFLINESPECIALS && li->linedef->special < MBF21LINESPECIALS)
            mbfcompatible = true;

        if (li->linedef->special >= BOOMLINESPECIALS)
            boomcompatible = true;
    }
}

// MB 2020-03-01: Fix endianness for 32-bit ZDoom nodes
// <https://zdoom.org/wiki/Node#ZDoom_extended_nodes>
static void P_LoadZNodes(int lump, bool compressed)
{
    byte            *data = W_CacheLumpNum(lump);
    byte            *output = NULL;
    unsigned int    orgVerts;
    unsigned int    newVerts;
    unsigned int    numSubs;
    unsigned int    currSeg = 0;
    unsigned int    numSegs;
    unsigned int    numNodes;
    vertex_t        *newvertarray = NULL;

    if (compressed)
    {
        const int   len = W_LumpLength(lump);
        int         outlen;
        int         err;
        z_stream    *zstream;

        // first estimate for compression rate:
        // output buffer size == input size * 2.5
        outlen = (int)(len * 2.5);
        output = I_Malloc(outlen);

        // initialize stream state for decompression
        zstream = I_Malloc(sizeof(*zstream));
        memset(zstream, 0, sizeof(*zstream));
        zstream->next_in = data + 4;
        zstream->avail_in = len - 4;
        zstream->next_out = output;
        zstream->avail_out = outlen;

        if (inflateInit(zstream) != Z_OK)
            I_Error("P_LoadZNodes: Error during ZNOD nodes decompression initialization!");

        // resize if output buffer runs full
        while ((err = inflate(zstream, Z_SYNC_FLUSH)) == Z_OK)
        {
            int outlen_old = outlen;

            outlen = outlen_old * 2;
            output = I_Realloc(output, outlen);
            zstream->next_out = output + outlen_old;
            zstream->avail_out = outlen - outlen_old;
        }

        if (err != Z_STREAM_END)
            I_Error("P_LoadZNodes: Error during ZNOD nodes decompression!");

        data = output;

        if (inflateEnd(zstream) != Z_OK)
            I_Error("P_LoadZNodes: Error during ZNOD nodes decompression shutdown!");

        // release the original data lump
        W_CacheLumpNum(lump);
        free(zstream);
    }
    else
    {
        // skip header
        data += 4;
    }

    // Read extra vertexes added during node building
    orgVerts = LONG(*((const unsigned int *)data));
    data += sizeof(orgVerts);

    newVerts = LONG(*((const unsigned int *)data));
    data += sizeof(newVerts);

    if (!samelevel)
    {
        if (orgVerts + newVerts == (unsigned int)numvertexes)
            newvertarray = vertexes;
        else
        {
            newvertarray = calloc((size_t)orgVerts + newVerts, sizeof(vertex_t));
            memcpy(newvertarray, vertexes, orgVerts * sizeof(vertex_t));
        }

        for (unsigned int i = 0; i < newVerts; i++)
        {
            newvertarray[i + orgVerts].x = LONG(*((const unsigned int *)data));
            data += sizeof(newvertarray[0].x);

            newvertarray[i + orgVerts].y = LONG(*((const unsigned int *)data));
            data += sizeof(newvertarray[0].y);
        }

        if (vertexes != newvertarray)
        {
            for (int i = 0; i < numlines; i++)
            {
                lines[i].v1 = lines[i].v1 - vertexes + newvertarray;
                lines[i].v2 = lines[i].v2 - vertexes + newvertarray;
            }

            free(vertexes);
            vertexes = newvertarray;
            numvertexes = orgVerts + newVerts;
        }
    }
    else
    {
        data += newVerts * (sizeof(newvertarray[0].x) + sizeof(newvertarray[0].y));

        // P_LoadVertexes reset numvertexes, need to increase it again
        numvertexes = orgVerts + newVerts;
    }

    // Read the subsectors
    numSubs = LONG(*((const unsigned int *)data));
    data += sizeof(numSubs);
    numsubsectors = numSubs;

    if (numsubsectors <= 0)
        I_Error("There are no subsectors in this map.");

    subsectors = calloc_IfSameLevel(subsectors, numsubsectors, sizeof(subsector_t));

    for (unsigned int i = 0; i < numSubs; i++)
    {
        const mapsubsector_znod_t   *mseg = (const mapsubsector_znod_t *)data + i;

        subsectors[i].firstline = currSeg;
        subsectors[i].numlines = LONG(mseg->numsegs);
        currSeg += LONG(mseg->numsegs);
    }

    data += numSubs * sizeof(mapsubsector_znod_t);

    // Read the segs
    numSegs = *((const unsigned int *)data);
    data += sizeof(numSegs);

    // The number of segs stored should match the number of
    // segs used by subsectors.
    if (numSegs != currSeg)
        I_Error("There are an incorrect number of segs in the nodes in this map.");

    numsegs = numSegs;
    segs = calloc_IfSameLevel(segs, numsegs, sizeof(seg_t));
    P_LoadZSegs(data);
    data += numsegs * sizeof(mapseg_znod_t);

    // Read nodes
    numNodes = LONG(*((const unsigned int *)data));
    data += sizeof(numNodes);
    numnodes = numNodes;
    nodes = calloc_IfSameLevel(nodes, numNodes, sizeof(node_t));

    for (unsigned int i = 0; i < numNodes; i++)
    {
        node_t                  *no = nodes + i;
        const mapnode_znod_t    *mn = (const mapnode_znod_t *)data + i;

        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;

        for (int j = 0; j < 2; j++)
        {
            no->children[j] = LONG(mn->children[j]);

            for (int k = 0; k < 4; k++)
                no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
        }
    }

    if (compressed && output)
        free(output);
    else
        W_ReleaseLumpNum(lump);

    P_CheckLinedefs();
}

//
// P_LoadThings
//
static void P_LoadThings(int map, int lump)
{
    const mapthing_t    *data = (const mapthing_t *)W_CacheLumpNum(lump);
    int                 numthings = W_LumpLength(lump) / sizeof(mapthing_t);

    if (!numthings || !data)
        I_Error("There are no things in this map.");

    M_BigSeed(gamemission == doom && map == 1 && canmodify ? BIGSEED : numthings);

    numspawnedthings = 0;
    numdecorations = 0;

    prevnumframes = 0;
    prevthingx = 0;
    prevthingy = 0;
    prevthingbob = 0;

    for (thingid = 0; thingid < numthings; thingid++)
    {
        mapthing_t  mt = data[thingid];
        bool        spawn = true;
        const short type = SHORT(mt.type);

        if (gamemode != commercial && type >= ArchVile && type <= MonsterSpawner && W_GetNumLumps("DEHACKED") == 1)
        {
            const int   doomednum = P_FindDoomedNum(type);
            static char buffer[128];

            M_StringCopy(buffer, mobjinfo[doomednum].plural1, sizeof(buffer));

            if (!*buffer)
                M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[doomednum].name1);

            buffer[0] = toupper(buffer[0]);
            C_Warning(2, "%s can't be spawned in " ITALICS("%s."), buffer, gamedescription);

            continue;
        }

        // Do spawn all other stuff.
        mt.x = SHORT(mt.x);
        mt.y = SHORT(mt.y);
        mt.angle = SHORT(mt.angle);
        mt.type = type;
        mt.options = SHORT(mt.options);

        // [BH] Apply any level-specific fixes.
        if (canmodify && r_fixmaperrors && gamemode != shareware)
            for (int j = 0; thingfix[j].mission != -1; j++)
                if (gamemission == thingfix[j].mission && gameepisode == thingfix[j].episode && gamemap == thingfix[j].map
                    && thingid == thingfix[j].thing && mt.type == thingfix[j].type
                    && mt.x == thingfix[j].oldx && mt.y == thingfix[j].oldy)
                {
                    char    *temp = commify(thingid);

                    if (thingfix[j].newx == REMOVE && thingfix[j].newy == REMOVE)
                    {
                        C_Warning(2, "Thing %s has been removed.", temp);

                        spawn = false;
                    }
                    else
                    {
                        C_Warning(2, "Thing %s has been moved from (%i, %i) to (%i, %i).",
                            temp, mt.x, mt.y, thingfix[j].newx, thingfix[j].newy);

                        mt.x = thingfix[j].newx;
                        mt.y = thingfix[j].newy;

                        if (thingfix[j].angle != DEFAULT)
                        {
                            C_Warning(2, "Thing %s has been turned from %i\xB0 to %i\xB0.",
                                temp, mt.angle, thingfix[j].angle);

                            mt.angle = thingfix[j].angle;
                        }

                        if (thingfix[j].options != DEFAULT)
                        {
                            C_Warning(2, "Thing %s's flags have been changed from %i to %i.",
                                temp, mt.options, thingfix[j].options);

                            mt.options = thingfix[j].options;
                        }
                    }

                    free(temp);
                    break;
                }

        if (spawn)
        {
            mobj_t  *thing;

            // Change each Wolfenstein SS into Zombiemen in BFG Edition
            if (mt.type == WolfensteinSS && !allowwolfensteinss)
                mt.type = Zombieman;

            if ((thing = P_SpawnMapThing(&mt, !nomonsters)))
            {
                if (thing->flags & (MF_TOUCHY | MF_BOUNCES | MF_FRIEND))
                    mbfcompatible = true;

                thing->id = thingid;
            }
        }
    }

    M_Seed((unsigned int)time(NULL));
    M_BigSeed((unsigned int)time(NULL));
    M_Fuzz1Seed((unsigned int)time(NULL));
    M_Fuzz2Seed((unsigned int)time(NULL));
    W_ReleaseLumpNum(lump);
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
// killough 04/04/98: split into two functions, to allow sidedef overloading
//
static void P_LoadLineDefs(int lump)
{
    const byte  *data = W_CacheLumpNum(lump);

    numlines = W_LumpLength(lump) / sizeof(maplinedef_t);
    lines = calloc_IfSameLevel(lines, numlines, sizeof(line_t));

    for (int i = 0; i < numlines; i++)
    {
        const maplinedef_t  *mld = (const maplinedef_t *)data + i;
        line_t              *ld = lines + i;
        vertex_t            *v1;
        vertex_t            *v2;

        ld->id = i;
        ld->flags = (unsigned short)SHORT(mld->flags);

        // [BH] Fix some linedefs in E2M7 only due to MBF21's ML_BLOCKPLAYERS flag
        if (E2M7)
            ld->flags = ((unsigned int)ld->flags & 0x03FF);

        ld->special = SHORT(mld->special);
        ld->tag = SHORT(mld->tag);
        v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)];
        v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)];
        ld->dx = v2->x - v1->x;
        ld->dy = v2->y - v1->y;

        ld->tranlump = -1;   // killough 04/11/98: no translucency by default

        ld->slopetype = (!ld->dx ? ST_VERTICAL : (!ld->dy ? ST_HORIZONTAL : (FixedDiv(ld->dy, ld->dx) > 0 ? ST_POSITIVE : ST_NEGATIVE)));

        if (v1->x < v2->x)
        {
            ld->bbox[BOXLEFT] = v1->x;
            ld->bbox[BOXRIGHT] = v2->x;
        }
        else
        {
            ld->bbox[BOXLEFT] = v2->x;
            ld->bbox[BOXRIGHT] = v1->x;
        }

        if (v1->y < v2->y)
        {
            ld->bbox[BOXBOTTOM] = v1->y;
            ld->bbox[BOXTOP] = v2->y;
        }
        else
        {
            ld->bbox[BOXBOTTOM] = v2->y;
            ld->bbox[BOXTOP] = v1->y;
        }

        // calculate sound origin of line to be its midpoint
        // e6y: fix sound origin for large levels
        ld->soundorg.x = ld->bbox[BOXLEFT] / 2 + ld->bbox[BOXRIGHT] / 2;
        ld->soundorg.y = ld->bbox[BOXTOP] / 2 + ld->bbox[BOXBOTTOM] / 2;

        ld->sidenum[0] = SHORT(mld->sidenum[0]);
        ld->sidenum[1] = SHORT(mld->sidenum[1]);

        // killough 04/04/98: support special sidedef interpretation below
        if (ld->sidenum[0] != NO_INDEX && ld->special)
            sides[ld->sidenum[0]].special = ld->special;
    }

    W_ReleaseLumpNum(lump);
}

// killough 04/04/98: delay using sidedefs until they are loaded
static void P_LoadLineDefs2(void)
{
    line_t  *ld = lines;

    transferredsky = false;

    for (int i = numlines; i--; ld++)
    {
        // cph 09/30/06: fix sidedef errors right away
        for (int j = 0; j < 2; j++)
            if (ld->sidenum[j] != NO_INDEX && ld->sidenum[j] >= numsides)
            {
                char    *temp1 = commify(ld->id);
                char    *temp2 = commify(ld->sidenum[j]);

                C_Warning(2, "Linedef %s references an invalid sidedef of %s.", temp1, temp2);
                ld->sidenum[j] = NO_INDEX;
                free(temp1);
                free(temp2);
            }

        // killough 11/98: fix common WAD errors (missing sidedefs):
        if (ld->sidenum[0] == NO_INDEX)
        {
            char    *temp = commify(ld->id);

            C_Warning(2, "Linedef %s is missing its first sidedef.", temp);
            ld->sidenum[0] = 0;                                     // Substitute dummy sidedef for missing right side
            free(temp);
        }

        if (ld->sidenum[1] == NO_INDEX && (ld->flags & ML_TWOSIDED))
        {
            char    *temp = commify(ld->id);

            C_Warning(2, "Linedef %s has the two-sided flag set but no second sidedef.", temp);
            ld->flags &= ~ML_TWOSIDED;                              // Clear 2s flag for missing left side
            free(temp);
        }

        ld->frontsector = (ld->sidenum[0] != NO_INDEX ? sides[ld->sidenum[0]].sector : NULL);
        ld->backsector = (ld->sidenum[1] != NO_INDEX ? sides[ld->sidenum[1]].sector : NULL);

        // killough 04/11/98: handle special types
        switch (ld->special)
        {
            case Translucent_MiddleTexture:                         // killough 04/11/98: translucent 2s textures
            {
                const int   lump = sides[ld->sidenum[0]].special;   // translucency from sidedef

                if (!ld->tag)                                       // if tag == 0,
                    ld->tranlump = lump;                            // affect this linedef only
                else
                    for (int j = 0; j < numlines; j++)              // if tag != 0,
                        if (lines[j].tag == ld->tag)                // affect all matching linedefs
                            lines[j].tranlump = lump;

                break;
            }

            case TransferSkyTextureToTaggedSectors:
            case TransferSkyTextureToTaggedSectors_Flipped:
                transferredsky = true;
                break;
        }
    }
}

//
// P_LoadSideDefs
//
// killough 04/04/98: split into two functions
static void P_LoadSideDefs(int lump)
{
    numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
    sides = calloc_IfSameLevel(sides, numsides, sizeof(side_t));
}

// killough 04/04/98: delay using texture names until after linedefs are loaded, to allow overloading
static void P_LoadSideDefs2(int lump)
{
    mapsidedef_t    *data = W_CacheLumpNum(lump);

    for (int i = 0; i < numsides; i++)
    {
        mapsidedef_t    *msd = data + i;
        side_t          *sd = sides + i;
        sector_t        *sec;
        unsigned short  sector_num = SHORT(msd->sector);

        sd->textureoffset = sd->basetextureoffset = sd->oldtextureoffset = SHORT(msd->textureoffset) << FRACBITS;
        sd->rowoffset = sd->baserowoffset = SHORT(msd->rowoffset) << FRACBITS;

        // cph 09/30/06: catch out-of-range sector numbers; use sector 0 instead
        if (sector_num >= numsectors)
        {
            char    *temp1 = commify(i);
            char    *temp2 = commify(sector_num);

            C_Warning(2, "Sidedef %s references an invalid sector of %s.", temp1, temp2);
            sector_num = 0;
            free(temp1);
            free(temp2);
        }

        sec = sd->sector = sectors + sector_num;

        // killough 04/04/98: allow sidedef texture names to be overloaded
        switch (sd->special)
        {
            case CreateFakeCeilingAndFloor:
                // variable colormap via 242 linedef
                sd->bottomtexture = ((sec->bottommap = R_ColormapNumForName(msd->bottomtexture)) < 0 ?
                    sec->bottommap = 0, R_TextureNumForName(msd->bottomtexture) : 0);
                sd->midtexture = ((sec->midmap = R_ColormapNumForName(msd->midtexture)) < 0 ?
                    sec->midmap = 0, R_TextureNumForName(msd->midtexture) : 0);
                sd->toptexture = ((sec->topmap = R_ColormapNumForName(msd->toptexture)) < 0 ?
                    sec->topmap = 0, R_TextureNumForName(msd->toptexture) : 0);
                break;

            case SetTheTargetSectorsColormap:
            case W1_SetTheTargetSectorsColormap:
            case WR_SetTheTargetSectorsColormap:
            case S1_SetTheTargetSectorsColormap:
            case SR_SetTheTargetSectorsColormap:
            case G1_SetTheTargetSectorsColormap:
            case GR_SetTheTargetSectorsColormap:
                // [KLN] 04/13/25: load the colormap for the ID24 line specials: 2075 - 2081: Set the target sector's colormap
                sd->toptexture = ((sd->id24colormapindex = R_ColormapNumForName(msd->toptexture)) < 0 ?
                    sd->id24colormapindex = 0, R_TextureNumForName(msd->toptexture) : 0);
                sd->midtexture = R_TextureNumForName(msd->midtexture);
                sd->missingmidtexture = (R_CheckTextureNumForName(msd->midtexture) == -1);
                sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
                sd->missingbottomtexture = (R_CheckTextureNumForName(msd->bottomtexture) == -1);
                break;

            case Translucent_MiddleTexture:
                // killough 04/11/98: apply translucency to 2s normal texture
                sd->midtexture = (strncasecmp("TRANMAP", msd->midtexture, 8) ?
                    (sd->special = W_CheckNumForName(msd->midtexture)) < 0
                    || W_LumpLength(sd->special) != 65536 ? sd->special = 0,
                    R_TextureNumForName(msd->midtexture) : (sd->special++, 0) : (sd->special = 0));
                sd->toptexture = R_TextureNumForName(msd->toptexture);
                sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
                break;

            default:
                // normal cases
                sd->midtexture = R_TextureNumForName(msd->midtexture);
                sd->missingmidtexture = (R_CheckTextureNumForName(msd->midtexture) == -1);
                sd->toptexture = R_TextureNumForName(msd->toptexture);
                sd->missingtoptexture = (R_CheckTextureNumForName(msd->toptexture) == -1);
                sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
                sd->missingbottomtexture = (R_CheckTextureNumForName(msd->bottomtexture) == -1);
                break;
        }
    }

    W_ReleaseLumpNum(lump);
}

//
// P_VerifyBlockMap
//
// haleyjd 03/04/10: do verification on validity of blockmap.
//
static bool P_VerifyBlockMap(int count)
{
    bool        isvalid = true;
    const int   *maxoffs = blockmaplump + count;

    skipblstart = true;

    for (int y = 0; y < bmapheight; y++)
    {
        for (int x = 0; x < bmapwidth; x++)
        {
            int         offset = y * bmapwidth + x;
            int         *list;
            const int   *blockoffset = blockmaplump + offset + 4;

            // check that block offset is in bounds
            if (blockoffset >= maxoffs)
            {
                isvalid = false;
                break;
            }

            offset = *blockoffset;
            list = blockmaplump + offset;

            if (*list)
                skipblstart = false;

            // scan forward for a -1 terminator before maxoffs
            for (int *tmplist = list; ; tmplist++)
            {
                // we have overflowed the lump?
                if (tmplist >= maxoffs)
                {
                    isvalid = false;
                    break;
                }

                if (*tmplist == -1) // found -1
                    break;
            }

            if (!isvalid)           // if the list is not terminated, break now
                break;

            // scan the list for out-of-range linedef indicies in list
            for (int *tmplist = list; *tmplist != -1; tmplist++)
                if (*tmplist < 0 || *tmplist >= numlines)
                {
                    isvalid = false;
                    break;
                }

            if (!isvalid)           // if a list has a bad linedef index, break now
                break;
        }

        // break out early on any error
        if (!isvalid)
            break;
    }

    return isvalid;
}

//
// killough 10/98:
//
// Rewritten to use faster algorithm.
//
// New procedure uses Bresenham-like algorithm on the linedefs, adding the
// linedef to each block visited from the beginning to the end of the linedef.
//
// The algorithm's complexity is on the order of nlines * total_linedef_length.
//
// Please note: This section of code is not interchangeable with TeamTNT's
// code which attempts to fix the same problem.
//
static void P_CreateBlockMap(void)
{
    fixed_t minx = FIXED_MAX;
    fixed_t miny = FIXED_MAX;
    fixed_t maxx = FIXED_MIN;
    fixed_t maxy = FIXED_MIN;

    // This fixes MBF's code, which has a bug where maxx/maxy
    // are wrong if the 0th node has the largest x or y
    if (numvertexes)
    {
        minx = maxx = vertexes[0].x;
        miny = maxy = vertexes[0].y;
    }

    blockmaprebuilt = true;

    for (int i = 0; i < numvertexes; i++)
    {
        fixed_t t = vertexes[i].x;

        if (t < minx)
            minx = t;
        else if (t > maxx)
            maxx = t;

        if ((t = vertexes[i].y) < miny)
            miny = t;
        else if (t > maxy)
            maxy = t;
    }

    minx /= FRACUNIT;
    maxx /= FRACUNIT;
    miny /= FRACUNIT;
    maxy /= FRACUNIT;

    // [crispy] doombsp/DRAWING.M:175-178
    minx -= 8;
    miny -= 8;
    maxx += 8;
    maxy += 8;

    // Save blockmap parameters
    bmaporgx = minx << FRACBITS;
    bmaporgy = miny << FRACBITS;
    bmapwidth = ((maxx - minx) >> MAPBTOFRAC) + 1;
    bmapheight = ((maxy - miny) >> MAPBTOFRAC) + 1;

    // Compute blockmap, which is stored as a 2D array of variable-sized lists.
    //
    // Pseudocode:
    //
    // For each linedef:
    //
    //   Map the starting and ending vertexes to blocks.
    //
    //   Starting in the starting vertex's block, do:
    //
    //     Add linedef to current block's list, dynamically resizing it.
    //
    //     If current block is the same as the ending vertex's block, exit loop.
    //
    //     Move to an adjacent block by moving towards the ending block in
    //     either the x or y direction, to the block which contains the linedef.
    {
        // blocklist structure
        typedef struct
        {
            size_t  n;
            size_t  nalloc;
            int     *list;
        } bmap_t;

        unsigned int    tot = bmapwidth * bmapheight;           // size of blockmap
        bmap_t          *bmap = calloc(tot, sizeof(*bmap));     // array of blocklists

        for (int i = 0; i < numlines; i++)
        {
            // starting coordinates
            const int       x = (lines[i].v1->x >> FRACBITS) - minx;
            const int       y = (lines[i].v1->y >> FRACBITS) - miny;

            // x - y deltas
            int             adx = lines[i].dx >> FRACBITS;
            int             dx = SIGN(adx);
            int             ady = lines[i].dy >> FRACBITS;
            int             dy = SIGN(ady);

            // difference in preferring to move across y (> 0) instead of x (< 0)
            int             diff = (!adx ? 1 : (!ady ? -1 : (((x >> MAPBTOFRAC) << MAPBTOFRAC)
                                + (dx > 0 ? MAPBLOCKUNITS - 1 : 0) - x) * (ady = ABS(ady)) * dx
                                - (((y >> MAPBTOFRAC) << MAPBTOFRAC) + (dy > 0 ? MAPBLOCKUNITS - 1 : 0) - y)
                                * (adx = ABS(adx)) * dy));

            // starting block, and pointer to its blocklist structure
            unsigned int    b = (y >> MAPBTOFRAC) * bmapwidth + (x >> MAPBTOFRAC);

            // ending block
            const int       bend = (((lines[i].v2->y >> FRACBITS) - miny) >> MAPBTOFRAC) * bmapwidth
                                + (((lines[i].v2->x >> FRACBITS) - minx) >> MAPBTOFRAC);

            // delta for pointer when moving across y
            dy *= bmapwidth;

            // deltas for diff inside the loop
            adx <<= MAPBTOFRAC;
            ady <<= MAPBTOFRAC;

            // Now we simply iterate block-by-block until we reach the end block.
            while (b < tot) // failsafe -- should ALWAYS be true
            {
                bmap_t  *bp = &bmap[b];

                // Increase size of allocated list if necessary
                if (bp->n >= bp->nalloc)
                    bp->list = I_Realloc(bp->list, (bp->nalloc = (bp->nalloc ? bp->nalloc * 2 : 8)) * sizeof(*bp->list));

                // Add linedef to end of list
                bp->list[bp->n++] = i;

                // If we have reached the last block, exit
                if (b == bend)
                    break;

                // Move in either the x or y direction to the next block
                if (diff < 0)
                {
                    diff += ady;
                    b += dx;
                }
                else
                {
                    diff -= adx;
                    b += dy;
                }
            }
        }

        // Compute the total size of the blockmap.
        //
        // Compression of empty blocks is performed by reserving two offset words
        // at tot and tot + 1.
        //
        // 4 words, unused if this routine is called, are reserved at the start.
        {
            size_t  count = (size_t)tot + 6;    // we need at least 1 word per block, plus reserved's

            for (int i = 0; (unsigned int)i < tot; i++)
                if (bmap[i].n)
                    count += bmap[i].n + 2;     // 1 header word + 1 trailer word + blocklist

            // Allocate blockmap lump with computed count
            blockmaplump = malloc_IfSameLevel(blockmaplump, count * sizeof(*blockmaplump));
        }

        // Now compress the blockmap.
        {
            int     ndx = (tot += 4);   // Advance index to start of linedef lists
            bmap_t  *bp = bmap;         // Start of uncompressed blockmap

            blockmaplump[ndx++] = 0;    // Store an empty blockmap list at start
            blockmaplump[ndx++] = -1;   // (Used for compression)

            for (unsigned int i = 4; i < tot; i++, bp++)
                if (bp->n)                                          // Non-empty blocklist
                {
                    blockmaplump[(blockmaplump[i] = ndx++)] = 0;    // Store index and header

                    do
                        blockmaplump[ndx++] = bp->list[--bp->n];    // Copy linedef list
                    while (bp->n);

                    blockmaplump[ndx++] = -1;                       // Store trailer
                    free(bp->list);                                 // Free linedef list
                }
                else
                    // Empty blocklist: point to reserved empty blocklist
                    blockmaplump[i] = tot;

            free(bmap); // Free uncompressed blockmap
        }
    }

    skipblstart = true;
}

//
// P_LoadBlockMap
//
// killough 03/01/98: substantially modified to work
// towards removing blockmap limit (a WAD limitation)
//
// killough 03/30/98: Rewritten to remove blockmap limit,
// though current algorithm is brute-force and non-optimal.
//
static void P_LoadBlockMap(int lump)
{
    int count;
    int lumplen = 1;

    blockmaprebuilt = false;

    if (lump >= numlumps || (lumplen = W_LumpLength(lump)) < 8 || (count = lumplen / 2) >= 0x010000)
    {
        P_CreateBlockMap();
        C_Warning(2, "The " BOLD("BLOCKMAP") " lump has been rebuilt.");
    }
    else if (M_CheckParm("-blockmap"))
    {
        P_CreateBlockMap();
        C_Warning(1, "A " BOLD("-blockmap") " parameter was found on the command-line. "
            "The " BOLD("BLOCKMAP") " lump has been rebuilt.");
    }
    else
    {
        short   *wadblockmaplump = W_CacheLumpNum(lump);

        blockmaplump = malloc_IfSameLevel(blockmaplump, count * sizeof(*blockmaplump));

        // killough 03/01/98: Expand WAD blockmap into larger internal one,
        // by treating all offsets except -1 as unsigned and zero-extending
        // them. This potentially doubles the size of blockmaps allowed,
        // because DOOM originally considered the offsets as always signed.
        blockmaplump[0] = SHORT(wadblockmaplump[0]);
        blockmaplump[1] = SHORT(wadblockmaplump[1]);
        blockmaplump[2] = ((unsigned int)(SHORT(wadblockmaplump[2])) & 0xFFFF);
        blockmaplump[3] = ((unsigned int)(SHORT(wadblockmaplump[3])) & 0xFFFF);

        // Swap all short integers to native byte ordering.
        for (int i = 4; i < count; i++)
        {
            const short t = SHORT(wadblockmaplump[i]);

            blockmaplump[i] = (t == -1 ? -1L : ((unsigned int)t & 0xFFFF));
        }

        // Read the header
        bmaporgx = blockmaplump[0] << FRACBITS;
        bmaporgy = blockmaplump[1] << FRACBITS;
        bmapwidth = blockmaplump[2];
        bmapheight = blockmaplump[3];

        if (!P_VerifyBlockMap(count))
        {
            P_CreateBlockMap();
            C_Warning(2, "The " BOLD("BLOCKMAP") " lump has been rebuilt.");
        }
    }

    // Clear out mobj chains
    blocklinks = calloc_IfSameLevel(blocklinks, (size_t)bmapwidth * bmapheight, sizeof(*blocklinks));
    blockmap = blockmaplump + 4;

    // MAES: set blockmapxneg and blockmapyneg
    // E.g. for a full 512x512 map, they should be both
    // -1. For a 257x257, they should be both -255 etc.
    blockmapxneg = (bmapwidth > 255 ? bmapwidth - 512 : -257);
    blockmapyneg = (bmapheight > 255 ? bmapheight - 512 : -257);
}

//
// reject overrun emulation
//
static void RejectOverrun(int lump, const byte **matrix)
{
    const size_t    required = ((size_t)numsectors * numsectors + 7) / 8;
    const size_t    length = W_LumpLength(lump);

    if (length < required)
    {
        // allocate a new block and copy the reject table into it; zero the rest
        // PU_LEVEL => will be freed on level exit
        byte    *newreject = Z_Malloc(required, PU_LEVEL, NULL);

        *matrix = memmove(newreject, *matrix, length);
        memset(newreject + length, 0, required - length);

        // unlock the original lump, it is no longer needed
        W_ReleaseLumpNum(lump);

        C_Warning(2, "The " BOLD("REJECT") " lump has been increased in size.");
    }
}

//
// P_LoadReject - load the reject table
//
static void P_LoadReject(int lumpnum)
{
    // dump any old cached reject lump, then cache the new one
    if (rejectlump != -1)
        W_ReleaseLumpNum(rejectlump);

    rejectlump = lumpnum + ML_REJECT;
    rejectmatrix = W_CacheLumpNum(rejectlump);

    // e6y: check for overflow
    RejectOverrun(rejectlump, &rejectmatrix);
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
// killough 05/03/98: reformatted, cleaned up
// cph 18/08/99: rewritten to avoid O(numlines * numsectors) section
// It makes things more complicated, but saves seconds on big levels

// cph - convenient sub-function
static void P_AddLineToSector(line_t *li, sector_t *sector)
{
    fixed_t *bbox = (void *)sector->blockbox;

    sector->lines[sector->linecount++] = li;
    M_AddToBox(bbox, li->v1->x, li->v1->y);
    M_AddToBox(bbox, li->v2->x, li->v2->y);
}

static void P_GroupLines(void)
{
    line_t      *li;
    sector_t    *sector;
    int         i;
    int         total = numlines;
    line_t      **linebuffer;

    // figgi
    for (i = 0; i < numsubsectors; i++)
    {
        seg_t   *seg = segs + subsectors[i].firstline;

        subsectors[i].sector = NULL;

        for (int j = 0; j < subsectors[i].numlines; j++)
        {
            if (seg->sidedef)
            {
                subsectors[i].sector = seg->sidedef->sector;
                break;
            }

            seg++;
        }

        if (!subsectors[i].sector)
            I_Error("Subsector %s is not a part of any sector.", commify(i));
    }

    // count number of lines in each sector
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
        li->frontsector->linecount++;

        if (li->backsector && li->backsector != li->frontsector)
        {
            li->backsector->linecount++;
            total++;
        }
    }

    // allocate line tables for each sector
    linebuffer = Z_Malloc(total * sizeof(line_t *), PU_LEVEL, NULL);

    for (i = 0, sector = sectors; i < numsectors; i++, sector++)
    {
        sector->lines = linebuffer;
        linebuffer += sector->linecount;
        sector->linecount = 0;
        M_ClearBox(sector->blockbox);
    }

    // Enter those lines
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
        P_AddLineToSector(li, li->frontsector);

        if (li->backsector && li->backsector != li->frontsector)
            P_AddLineToSector(li, li->backsector);
    }

    for (i = 0, sector = sectors; i < numsectors; i++, sector++)
    {
        const fixed_t   *bbox = (void *)sector->blockbox;

        // e6y: fix sound origin for large levels
        sector->soundorg.x = bbox[BOXRIGHT] / 2 + bbox[BOXLEFT] / 2;
        sector->soundorg.y = bbox[BOXTOP] / 2 + bbox[BOXBOTTOM] / 2;

        // adjust bounding box to map blocks
        sector->blockbox[BOXTOP] = MIN(P_GetSafeBlockY(bbox[BOXTOP] - bmaporgy + MAXRADIUS), bmapheight - 1);
        sector->blockbox[BOXBOTTOM] = MAX(0, P_GetSafeBlockY(bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS));
        sector->blockbox[BOXRIGHT] = MIN(P_GetSafeBlockX(bbox[BOXRIGHT] - bmaporgx + MAXRADIUS), bmapwidth - 1);
        sector->blockbox[BOXLEFT] = MAX(0, P_GetSafeBlockX(bbox[BOXLEFT] - bmaporgx - MAXRADIUS));
    }
}

//
// killough 10/98
//
// Remove slime trails.
//
// Slime trails are inherent to DOOM's coordinate system -- i.e. there is
// nothing that a node builder can do to prevent slime trails ALL of the time,
// because it's a product of the integer coordinate system, and just because
// two lines pass through exact integer coordinates, doesn't necessarily mean
// that they will intersect at integer coordinates. Thus we must allow for
// fractional coordinates if we are to be able to split segs with node lines,
// as a node builder must do when creating a BSP tree.
//
// A WAD file does not allow fractional coordinates, so node builders are out
// of luck except that they can try to limit the number of splits (they might
// also be able to detect the degree of roundoff error and try to avoid splits
// with a high degree of roundoff error). But we can use fractional coordinates
// here, inside the engine. It's like the difference between square inches and
// square miles, in terms of granularity.
//
// For each vertex of every seg, check to see whether it's also a vertex of
// the linedef associated with the seg (i.e, it's an endpoint). If it's not
// an endpoint, and it wasn't already moved, move the vertex towards the
// linedef by projecting it using the law of cosines. Formula:
//
//      2        2                         2        2
//    dx  x0 + dy  x1 + dx dy (y0 - y1)  dy  y0 + dx  y1 + dx dy (x0 - x1)
//   {---------------------------------, ---------------------------------}
//                  2     2                            2     2
//                dx  + dy                           dx  + dy
//
// (x0, y0) is the vertex being moved, and (x1, y1) - (x1 + dx, y1 + dy) is the
// reference linedef.
//
// Segs corresponding to orthogonal linedefs (exactly vertical or horizontal
// linedefs), which comprise at least half of all linedefs in most wads, don't
// need to be considered, because they almost never contribute to slime trails
// (because then any roundoff error is parallel to the linedef, which doesn't
// cause slime). Skipping simple orthogonal lines lets the code finish quicker.
//
// Please note: This section of code is not interchangeable with TeamTNT's
// code which attempts to fix the same problem.
//
// Firelines (TM) is a Registered Trademark of MBF Productions
//

static void P_RemoveSlimeTrails(void)                   // killough 10/98
{
    byte    *hit = calloc(1, numvertexes);              // Hitlist for vertexes

    if (!hit)
        return;

    for (int i = 0; i < numsegs; i++)                   // Go through each seg
    {
        const line_t    *l = segs[i].linedef;           // The parent linedef

        if (l->dx && l->dy)                             // We can ignore orthogonal lines
        {
            vertex_t    *v = segs[i].v1;

            do
            {
                if (!hit[v - vertexes])                 // If we haven't processed vertex
                {
                    hit[v - vertexes] = 1;              // Mark this vertex as processed

                    if (v != l->v1 && v != l->v2)       // Exclude endpoints of linedefs
                    {
                        // Project the vertex back onto the parent linedef
                        const int64_t   dx2 = (int64_t)(l->dx >> FRACBITS) * (l->dx >> FRACBITS);
                        const int64_t   dy2 = (int64_t)(l->dy >> FRACBITS) * (l->dy >> FRACBITS);
                        const int64_t   dxy = (int64_t)(l->dx >> FRACBITS) * (l->dy >> FRACBITS);
                        const int64_t   s = dx2 + dy2;
                        const fixed_t   x0 = v->x;
                        const fixed_t   y0 = v->y;
                        const fixed_t   x1 = l->v1->x;
                        const fixed_t   y1 = l->v1->y;

                        v->x = (fixed_t)((dx2 * x0 + dy2 * x1 + dxy * ((int64_t)y0 - y1)) / s);
                        v->y = (fixed_t)((dy2 * y0 + dx2 * y1 + dxy * ((int64_t)x0 - x1)) / s);

                        // [crispy] wait a minute... moved more than 8 map units?
                        // maybe that's a linguortal then, back to the original coordinates
                        if (ABS(v->x - x0) > 8 * FRACUNIT || ABS(v->y - y0) > 8 * FRACUNIT)
                        {
                            v->x = x0;
                            v->y = y0;
                        }
                    }
                }
            } while (v != segs[i].v2 && (v = segs[i].v2));
        }
    }

    free(hit);
}

// Precalculate values for use later in long wall error fix in R_StoreWallRange()
static void P_CalcSegsLength(void)
{
    const int   rightangle = finesine[ANG30 >> ANGLETOFINESHIFT];

    for (int i = 0; i < numsegs; i++)
    {
        seg_t           *li = segs + i;

        // [BH] recalculate angle used for rendering. Fixes <https://doomwiki.org/wiki/Bad_seg_angle>.
        const angle_t   angle = R_PointToAngleEx2(li->v1->x, li->v1->y, li->v2->x, li->v2->y);

        if (AngleDiff(li->angle, angle) <= ANG30)
            li->angle = angle;

        li->dx = (int64_t)li->v2->x - li->v1->x;
        li->dy = (int64_t)li->v2->y - li->v1->y;

        li->length = (int64_t)sqrt((double)li->dx * li->dx + (double)li->dy * li->dy) / 2;

        if (!li->dy)
            li->fakecontrast = -LIGHTBRIGHT;
        else if (ABS(finesine[li->angle >> ANGLETOFINESHIFT]) < rightangle)
            li->fakecontrast = -LIGHTBRIGHT / 2;
        else if (!li->dx)
            li->fakecontrast = LIGHTBRIGHT;
        else if (ABS(finecosine[li->angle >> ANGLETOFINESHIFT]) < rightangle)
            li->fakecontrast = LIGHTBRIGHT / 2;
        else
            li->fakecontrast = 0;

        li->dx /= 2;
        li->dy /= 2;
    }
}

// Find all self-referencing sectors
static void P_FindSelfReferencingSectors(void)
{
    for (int i = 0; i < numsectors; i++)
    {
        sector_t    *sec = sectors + i;
        const int   linecount = sec->linecount;
        int         count = 0;

        for (int j = 0; j < linecount; j++)
        {
            const line_t    *line = sec->lines[j];

            if (line->backsector
                && line->frontsector == line->backsector
                && !line->frontsector->tag)
            {
                const side_t    *side1 = &sides[line->sidenum[0]];
                const side_t    *side2 = &sides[line->sidenum[1]];

                if (!side1->toptexture && !side1->bottomtexture
                    && !side2->toptexture && !side2->bottomtexture)
                    count++;
            }
        }

        sec->isselfreferencing = (count == linecount);
    }
}

char    mapnum[6];
char    maptitle[256];
char    mapnumandtitle[512];
char    automaptitle[512];

// Determine map name to use
void P_MapName(int ep, int map)
{
    bool        mapnumonly = false;
    const char  *mapinfoname = trimwhitespace(P_GetMapName(ep, map));

    switch (gamemission)
    {
        case doom:
            M_snprintf(mapnum, sizeof(mapnum), "E%iM%i%s", ep, map, (((E1M4B || *speciallumpname) && ep == 1 && map == 4)
                || ((E1M8B || *speciallumpname) && ep == 1 && map == 8) ? "B" : ""));

            if (*mapinfoname)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (W_GetNumLumps(mapnum) > 1 && dehmaptitlecount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_StringCopy(automaptitle, mapnum, sizeof(mapnumandtitle));
            }
            else if (map <= nummapnames)
                M_StringCopy(maptitle, trimwhitespace(*mapnames[(ep - 1) * 9 + map - 1]), sizeof(maptitle));

            break;

        case doom2:
            if (legacyofrust)
                M_snprintf(mapnum, sizeof(mapnum), "E%iM%i", maptoepisode[map], P_GetMapInEpisode(map));
            else
                M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);

            if (*mapinfoname && !BTSX)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (W_GetNumLumps(mapnum) > 1 && (!nerve || map > 9) && dehmaptitlecount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_StringCopy(automaptitle, mapnum, sizeof(automaptitle));
            }
            else if (bfgedition && (!modifiedgame || nerve) && map <= nummapnames2_bfg)
                M_StringCopy(maptitle, trimwhitespace(*mapnames2_bfg[map - 1]), sizeof(maptitle));
            else if (map <= nummapnames2)
                M_StringCopy(maptitle, trimwhitespace(*mapnames2[map - 1]), sizeof(maptitle));

            break;

        case pack_nerve:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);

            if (*mapinfoname)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (map <= nummapnamesn)
                M_StringCopy(maptitle, trimwhitespace(*mapnamesn[map - 1]), sizeof(maptitle));

            break;

        case pack_plut:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);

            if (*mapinfoname)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (W_GetNumLumps(mapnum) > 1 && dehmaptitlecount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_StringCopy(automaptitle, mapnum, sizeof(automaptitle));
            }
            else if (map <= nummapnamesp)
                M_StringCopy(maptitle, trimwhitespace(*mapnamesp[map - 1]), sizeof(maptitle));

            break;

        case pack_tnt:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);

            if (*mapinfoname)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (W_GetNumLumps(mapnum) > 1 && dehmaptitlecount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_StringCopy(automaptitle, mapnum, sizeof(automaptitle));
            }
            else if (map <= nummapnamest)
                M_StringCopy(maptitle, trimwhitespace(*mapnamest[map - 1]), sizeof(maptitle));

            break;

        default:
            break;
    }

    if (strlen(maptitle) >= 4)
    {
        if (maptitle[0] == 'm' && maptitle[1] == 'a' && maptitle[2] == 'p'
            && isdigit((int)maptitle[3]) && isdigit((int)maptitle[4]))
        {
            maptitle[0] = 'M';
            maptitle[1] = 'A';
            maptitle[2] = 'P';
        }
        else if (maptitle[0] == 'e' && isdigit((int)maptitle[1]) && maptitle[2] == 'm' && isdigit((int)maptitle[3]))
        {
            maptitle[0] = 'E';
            maptitle[2] = 'M';
        }
    }

    if (!mapnumonly)
    {
        const char  *pos = strchr(maptitle, ':');

        if (pos)
        {
            const int   index = (int)(pos - maptitle) + 1;
            char        *temp;

            if (toupper(maptitle[0]) == 'Z' && isdigit(maptitle[1])
                && toupper(maptitle[2]) == 'M' && isdigit(maptitle[3]))
            {
                M_StringCopy(mapnum, maptitle, 5);
                memmove(maptitle, maptitle + index, strlen(maptitle) - index + 1);

                if (maptitle[0] == ' ')
                    memmove(maptitle, maptitle + 1, strlen(maptitle));

                temp = titlecase(maptitle);
                M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s: " ITALICS("%s"), mapnum, temp);
            }
            else if (toupper(maptitle[0]) == 'E' && isdigit(maptitle[1])
                && toupper(maptitle[2]) == 'M' && isdigit(maptitle[3]) && isdigit(maptitle[4]))
            {
                memmove(maptitle, maptitle + index, strlen(maptitle) - index + 1);

                if (maptitle[0] == ' ')
                    memmove(maptitle, maptitle + 1, strlen(maptitle));

                temp = titlecase(maptitle);
                M_snprintf(mapnum, sizeof(mapnum), "E%iM%02i", ep, map);
                M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s: " ITALICS("%s"), mapnum, temp);
            }
            else if (M_StringStartsWith(maptitle, "LEVEL")
                || M_StringStartsWith(maptitle, "LV.")
                || M_StringStartsWith(maptitle, "MAP")
                || (toupper(maptitle[0]) == 'E' && isdigit(maptitle[1]) && toupper(maptitle[2]) == 'M' && isdigit(maptitle[3])))
            {
                memmove(maptitle, maptitle + index, strlen(maptitle) - index + 1);

                if (maptitle[0] == ' ')
                    memmove(maptitle, maptitle + 1, strlen(maptitle));

                temp = titlecase(maptitle);
                M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s: " ITALICS("%s"), mapnum, temp);
            }
            else if (toupper(maptitle[0]) == 'L' && isdigit(maptitle[1]) && isdigit(maptitle[2]))
            {
                memmove(maptitle, maptitle + index, strlen(maptitle) - index + 1);

                if (maptitle[0] == ' ')
                    memmove(maptitle, maptitle + 1, strlen(maptitle));

                temp = titlecase(maptitle);
                M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "L%02i: " ITALICS("%s"), map, temp);
            }
            else
            {
                temp = titlecase(maptitle);
                M_StringCopy(mapnumandtitle, temp, sizeof(mapnumandtitle));
                memmove(maptitle, maptitle + index, strlen(maptitle) - index + 1);

                if (maptitle[0] == ' ')
                    memmove(maptitle, maptitle + 1, strlen(maptitle));
            }

            free(temp);
        }
        else if (!M_StringCompare(mapnum, maptitle))
        {
            char    *temp = titlecase(maptitle);

            M_snprintf(mapnumandtitle, sizeof(mapnumandtitle),
                "%s%s" ITALICS("%s"), mapnum, (maptitle[0] ? ": " : ""), temp);
            free(temp);
        }
        else
            M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));

        M_StringCopy(automaptitle, mapnumandtitle, sizeof(automaptitle));
    }
}

static nodeformat_t P_CheckNodeFormat(int lumpnum)
{
    nodeformat_t    format = DOOMBSP;
    byte            *n = NULL;
    int             b = lumpnum + ML_BLOCKMAP + 1;

    if (b < numlumps && !strncasecmp(lumpinfo[b]->name, "BEHAVIOR", 8))
        I_Error("Hexen format maps are not supported.");

    if ((b = lumpnum + ML_NODES) < numlumps && (n = W_CacheLumpNum(b)) && W_LumpLength(b))
    {
        if (!memcmp(n, "xNd4\0\0\0\0", 8))
            format = DEEPBSP;
        else if (!W_LumpLength(lumpnum + ML_SEGS) && W_LumpLength(lumpnum + ML_NODES) >= 12)
        {
            if (!memcmp(n, "XNOD", 4))
                format = ZDBSPX;
            else if (!memcmp(n, "ZNOD", 4))
                format = ZDBSPZ;
        }
    }

    if (n)
        W_ReleaseLumpNum(b);

    return format;
}

//
// P_SetupLevel
//
void P_SetupLevel(int ep, int map)
{
    char        lumpname[6];
    int         lumpnum;
    static int  prevlumpnum = -1;

    id24compatible = false;

    nummappedlines = 0;
    totalkills = 0;
    totalitems = 0;
    totalsecrets = 0;
    totalpickups = 0;
    memset(monstercount, 0, NUMMOBJTYPES * sizeof(int));
    barrelcount = 0;
    player1starts = 0;
    wminfo.partime = 0;
    viewplayer->killcount = 0;
    viewplayer->secretcount = 0;
    viewplayer->itemcount = 0;

    idclev = false;
    idclevtics = 0;

    Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

    if (rejectlump != -1)
    {
        // cph - unlock the reject table
        W_ReleaseLumpNum(rejectlump);
        rejectlump = -1;
    }

    P_InitThinkers();

    secretmap = false;

    // find map name
    if (*speciallumpname)
    {
        lumpnum = W_GetNumForName(speciallumpname);
        M_StringCopy(lumpname, speciallumpname, sizeof(lumpname));
    }
    else
    {
        if (gamemode == commercial)
        {
            M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", map);

            if (map == 31 || map == 32 || (map == 33 && bfgedition) || (gamemission == pack_nerve && map == 9))
                secretmap = true;
        }
        else
        {
            M_snprintf(lumpname, sizeof(lumpname), "E%iM%i", ep, map);

            if (map == 9)
                secretmap = true;
        }

        lumpnum = (nerve && gamemission == doom2 ? W_GetLastNumForName(lumpname) : W_GetNumForName(lumpname));
    }

    if (!secretmap)
        secretmap = mapinfo[ep][map].secret;

    if ((!numconsolestrings
        || (!M_StringStartsWith(console[numconsolestrings - 1].string, "map ")
            && !M_StringStartsWith(console[numconsolestrings - 1].string, "load ")
            && !M_StringStartsWith(console[numconsolestrings - 1].string, "newgame")
            && !M_StringCompare(console[numconsolestrings - 1].string, "restartmap")
            && !M_StringStartsWith(console[numconsolestrings - 1].string, "Warping ")
            && !M_StringStartsWith(console[numconsolestrings - 1].string, "Restarting ")
            && !autostart))
        && ((numconsolestrings == 1
            || (!M_StringStartsWith(console[numconsolestrings - 2].string, "map ")
                && !autostart))))
    {
        if (legacyofrust)
            C_Input("map E%iM%i", maptoepisode[map], P_GetMapInEpisode(map));
        else
            C_Input("map %s", lumpname);
    }

    if (!(samelevel = (lumpnum == prevlumpnum)))
    {
        viewplayer->cheats &= ~(CF_ALLMAP | CF_ALLMAP_THINGS);
        viewplayer->deaths = 0;
        viewplayer->suicides = 0;
    }

    prevlumpnum = lumpnum;

    nodeformat = P_CheckNodeFormat(lumpnum);

    canmodify = ((W_GetNumLumps(lumpname) == 1
        || (sigil && gamemission == doom)
        || (sigil2 && gamemission == doom)
        || gamemission == pack_nerve
        || (nerve && gamemission == doom2))
        && !FREEDOOM
        && !(gamemission == doom && *speciallumpname && (map == 4 || map == 8)));

    speciallumpname[0] = '\0';

    C_AddConsoleDivider();

    if (M_StringCompare(maptitle, mapnumandtitle))
        C_PlayerMessage("%s %s %s.",
            (M_StringCompare(playername, playername_default) ? "You" : playername),
            (samelevel ? "reentered" : "entered"), maptitle);
    else
    {
        char    *temp = titlecase(maptitle);

        C_PlayerMessage("%s %s " ITALICS("%s") "%s",
            (M_StringCompare(playername, playername_default) ? "You" : playername),
            (samelevel ? "reentered" : "entered"), temp,
            (ispunctuation(temp[strlen(temp) - 1]) ? "" : "."));

        free(temp);
    }

    maptime = 0;
    animatedtic = 0;
    animatedliquiddiff = 2 * FRACUNIT + animatedliquiddiffs[M_BigRandom() & (ANIMATEDLIQUIDDIFFS - 1)];
    animatedliquidxdir = M_BigRandomInt(-FRACUNIT, FRACUNIT) / 12;
    animatedliquidydir = M_BigRandomInt(-FRACUNIT, FRACUNIT) / 12;

    animatedliquidxoffs = 0;
    animatedliquidyoffs = 0;

    if (!samelevel)
    {
        free(segs);
        free(nodes);
        free(subsectors);
        free(blocklinks);
        free(blockmaplump);
        free(lines);
        free(sides);
        free(sectors);
        free(vertexes);
    }

    // note: most of this ordering is important
    P_LoadVertexes(lumpnum + ML_VERTEXES);
    P_LoadSectors(lumpnum + ML_SECTORS);
    P_LoadSideDefs(lumpnum + ML_SIDEDEFS);
    P_LoadLineDefs(lumpnum + ML_LINEDEFS);
    P_LoadSideDefs2(lumpnum + ML_SIDEDEFS);

    // killough 01/30/98: Create xref tables for tags
    P_InitTagLists();

    P_LoadLineDefs2();

    if (!samelevel)
        P_LoadBlockMap(lumpnum + ML_BLOCKMAP);
    else
        memset(blocklinks, 0, (size_t)bmapwidth * bmapheight * sizeof(*blocklinks));

    if (nodeformat == ZDBSPX)
        P_LoadZNodes(lumpnum + ML_NODES, false);
    else if (nodeformat == ZDBSPZ)
        P_LoadZNodes(lumpnum + ML_NODES, true);
    else if (nodeformat == DEEPBSP)
    {
        P_LoadSubsectors_V4(lumpnum + ML_SSECTORS);
        P_LoadNodes_V4(lumpnum + ML_NODES);
        P_LoadSegs_V4(lumpnum + ML_SEGS);
    }
    else
    {
        P_LoadSubsectors(lumpnum + ML_SSECTORS);
        P_LoadNodes(lumpnum + ML_NODES);
        P_LoadSegs(lumpnum + ML_SEGS);
    }

    P_GroupLines();
    P_LoadReject(lumpnum);

    P_RemoveSlimeTrails();

    P_CalcSegsLength();

    nummarks = 0;
    maxmarks = 0;
    mark = NULL;

    numbreadcrumbs = 0;
    maxbreadcrumbs = NUMBREADCRUMBS;
    breadcrumb = I_Realloc(breadcrumb, maxbreadcrumbs * sizeof(*breadcrumb));

    massacre = false;

    P_GetMapLiquids(ep, map);
    P_GetMapNoLiquids(ep, map);
    P_SetLiquids();

    P_LoadThings(map, lumpnum + ML_THINGS);

    numfriends = 0;

    P_InitCards();

    prevtouchtype = MT_NULL;

    // set up world state
    P_SpawnSpecials();
    P_FindLifts();
    P_FindSelfReferencingSectors();

    P_MapEnd();

    // preload graphics
    R_PrecacheLevel();

    if (!musinfo.fromsavegame)
        S_Start();
    else
        musinfo.fromsavegame = false;

    S_ParseMusInfo(lumpname);

    compat_corpsegibs = (compat_corpsegibs_global != -1 ? compat_corpsegibs_global : mapinfo[ep][map].compat_corpsegibs);
    compat_floormove = (compat_floormove_global != -1 ? compat_floormove_global : mapinfo[ep][map].compat_floormove);
    compat_light = (compat_light_global != -1 ? compat_light_global : mapinfo[ep][map].compat_light);
    compat_limitpain = (compat_limitpain_global != -1 ? compat_limitpain_global : mapinfo[ep][map].compat_limitpain);
    compat_nopassover = (compat_nopassover_global != -1 ? compat_nopassover_global : mapinfo[ep][map].compat_nopassover);
    compat_stairs = (compat_stairs_global != -1 ? compat_stairs_global : mapinfo[ep][map].compat_stairs);
    compat_useblocking = (compat_useblocking_global != -1 ? compat_useblocking_global : mapinfo[ep][map].compat_useblocking);
    compat_zombie = (compat_zombie_global != -1 ? compat_zombie_global : mapinfo[ep][map].compat_zombie);
    nograduallighting = (nograduallighting_global != -1 ? nograduallighting_global : mapinfo[ep][map].nograduallighting);

    if (gamemission == pack_tnt)
        compat_stairs = true;
}

static int  liquidlumps;
static int  noliquidlumps;

static void P_InitMapInfo(void)
{
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 100; j++)
        {
            mapinfo[i][j].allowmonstertelefrags = -1;
            mapinfo[i][j].author[0] = '\0';
            mapinfo[i][j].cluster = 0;
            mapinfo[i][j].compat_corpsegibs = false;
            mapinfo[i][j].compat_floormove = false;
            mapinfo[i][j].compat_light = false;
            mapinfo[i][j].compat_limitpain = false;
            mapinfo[i][j].compat_nopassover = false;
            mapinfo[i][j].compat_stairs = false;
            mapinfo[i][j].compat_useblocking = false;
            mapinfo[i][j].compat_zombie = false;
            mapinfo[i][j].endbunny = false;
            mapinfo[i][j].endcast = false;
            mapinfo[i][j].endgame = false;
            mapinfo[i][j].endpic = 0;
            mapinfo[i][j].enterpic = 0;
            mapinfo[i][j].exitpic = 0;
            mapinfo[i][j].interbackdrop[0] = '\0';
            mapinfo[i][j].intermusic = 0;
            mapinfo[i][j].intertext[0] = '\0';
            mapinfo[i][j].intertextsecret[0] = '\0';
            mapinfo[i][j].label[0] = '\0';

            for (int k = 0; k < NUMLIQUIDS; k++)
            {
                mapinfo[i][j].liquid[k] = -1;
                mapinfo[i][j].noliquid[k] = -1;
            }

            mapinfo[i][j].music = 0;
            mapinfo[i][j].musicartist[0] = '\0';
            mapinfo[i][j].musictitle[0] = '\0';
            mapinfo[i][j].name[0] = '\0';
            mapinfo[i][j].next = 0;
            mapinfo[i][j].nofreelook = false;
            mapinfo[i][j].nograduallighting = false;
            mapinfo[i][j].nojump = false;
            mapinfo[i][j].numbossactions = 0;
            mapinfo[i][j].par = 0;
            mapinfo[i][j].pistolstart = false;
            mapinfo[i][j].secretnext = 0;
            mapinfo[i][j].secret = false;
            mapinfo[i][j].sky1texture = 0;
            mapinfo[i][j].sky1scrolldelta = 0;
            mapinfo[i][j].titlepatch = 0;
        }
}

static void P_ParseMapString(const char *string, int *map, int *ep)
{
    char    *buffer = uppercase(string);
    int     value1;
    int     value2;

    if (gamemode == commercial)
    {
        if (sscanf(buffer, "MAP0%1i", &value1) == 1
            || sscanf(buffer, "MAP%2i", &value1) == 1)
            *map = value1;
    }
    else
    {
        if (sscanf(buffer, "E%1iM%i", &value1, &value2) == 2)
        {
            *ep = value1;
            *map = value2;
        }
    }

    free(buffer);
}

static int mapinepisode(const int map)
{
    int epi = maptoepisode[map];

    if (epi)
        for (int i = map; i >= 1; i--)
            if (maptoepisode[i] < epi)
                return (map - i);

    return map;
}

static bool P_ParseMapInfo(const char *scriptname)
{
    int         mapmax = 1;
    int         mcmdvalue;
    mapinfo_t   *info;
    char        *temp1;
    char        *temp2;

    for (MAPINFO = numlumps - 1; MAPINFO >= 0; MAPINFO--)
        if (!strncasecmp(lumpinfo[MAPINFO]->name, scriptname, 8))
        {
            char    *file = leafname(lumpinfo[MAPINFO]->wadfile->path);

            if (!D_IsNERVEWAD(file) && !D_IsSIGILWAD(file) && !D_IsSIGIL2WAD(file) && !D_IsFinalDOOMIWAD(file))
                break;
        }

    if (MAPINFO == -1)
        return false;

    mapinfolump = uppercase(scriptname);

    SC_Open(MAPINFO);

    while (SC_GetString())
    {
        int ep = 1;
        int map = 0;

        if (SC_Compare("DEFAULTMAP"))
        {
            while (SC_GetString())
            {
                if ((mcmdvalue = SC_MatchString(mapcmdnames)) >= 0)
                {
                    switch (mapcmdids[mcmdvalue])
                    {
                        case MCMD_COMPAT_CORPSEGIBS:
                        case MCMD_COMPAT_VILEGHOSTS:
                            SC_MustGetNumber();
                            compat_corpsegibs_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_LIMITPAIN:
                            SC_MustGetNumber();
                            compat_limitpain_global = (int)sc_Number;
                            break;

                        case MCMD_NOGRADUALLIGHTING:
                            SC_MustGetNumber();
                            nograduallighting_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_FLOORMOVE:
                            SC_MustGetNumber();
                            compat_floormove_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_LIGHT:
                            SC_MustGetNumber();
                            compat_light_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_NOPASSOVER:
                            SC_MustGetNumber();
                            compat_nopassover_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_STAIRS:
                            SC_MustGetNumber();
                            compat_stairs_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_USEBLOCKING:
                            SC_MustGetNumber();
                            compat_useblocking_global = (int)sc_Number;
                            break;

                        case MCMD_COMPAT_ZOMBIE:
                            SC_MustGetNumber();
                            compat_zombie_global = (int)sc_Number;
                            break;

                        case MCMD_NOJUMP:
                            nojump = true;
                            break;

                        case MCMD_NOFREELOOK:
                        case MCMD_NOMOUSELOOK:
                            nofreelook = true;
                            break;
                    }
                }
                else
                    break;
            }
        }
        else if (SC_Compare("EPISODE"))
        {
            SC_MustGetString();

            if (sscanf(sc_String, "%i", &map) != 1 || map < 0 || map > 99)
            {
                char    *temp = uppercase(sc_String);

                if (gamemode == commercial)
                {
                    if (sscanf(temp, "MAP0%1i", &map) != 1)
                        sscanf(temp, "MAP%2i", &map);
                }
                else
                    sscanf(temp, "E%1iM%i", &ep, &map);

                free(temp);
            }

            if (map >= 0 && map <= 99)
            {
                SC_MustGetString();

                if (SC_Compare("NAME"))
                {
                    SC_MustGetString();
                    M_AddEpisode(map, ep, "", sc_String);
                    SC_MustGetString();
                }

                if (SC_Compare("PICNAME"))
                {
                    SC_MustGetString();
                    M_AddEpisode(map, ep, sc_String, "");
                }
            }
        }
        else if (SC_Compare("MAP"))
        {
            SC_MustGetString();

            if (sscanf(sc_String, "%i", &map) != 1 || map < 0 || map > 99)
            {
                char    *temp = uppercase(sc_String);

                if (gamemode == commercial)
                {
                    if (sscanf(temp, "MAP0%1i", &map) != 1 && sscanf(temp, "MAP%2i", &map) != 1)
                    {
                        free(temp);
                        continue;
                    }
                }
                else
                {
                    if (sscanf(temp, "E%1iM%i", &ep, &map) != 2)
                    {
                        free(temp);
                        continue;
                    }
                }

                free(temp);
            }

            if (map < 0 || map > 99)
            {
                if (M_StringEndsWith(lumpinfo[MAPINFO]->wadfile->path, "NERVE.WAD"))
                {
                    C_Warning(1, "The map markers in PWAD " BOLD("%s") " are invalid.",
                        lumpinfo[MAPINFO]->wadfile->path);
                    nerve = false;
                    NewDef.prevmenu = &MainDef;
                    MAPINFO = -1;
                    break;
                }
                else
                {
                    C_Warning(1, "The " BOLD("MAPINFO") " lump contains an invalid map marker.");
                    continue;
                }
            }

            info = &mapinfo[ep][map];

            if (compat_stairs_global == -1)
                info->compat_stairs = true;

            if (compat_zombie_global == -1)
                info->compat_zombie = true;

            // Process optional tokens
            while (SC_GetString())
            {
                if (SC_Compare("MAP")
                    || SC_Compare("DEFAULTMAP")
                    || SC_Compare("CLUSTERDEF")
                    || SC_Compare("LIQUID")
                    || SC_Compare("NOLIQUID"))
                {
                    SC_UnGet();
                    break;
                }

                if ((mcmdvalue = SC_MatchString(mapcmdnames)) >= 0)
                    switch (mapcmdids[mcmdvalue])
                    {
                        case MCMD_AUTHOR:
                            SC_MustGetString();
                            M_StringCopy(info->author, sc_String, sizeof(info->author));
                            break;

                        case MCMD_BOSSACTION:
                            SC_MustGetString();

                            if (SC_Compare("CLEAR"))
                            {
                                if (info->bossactions)
                                    free(info->bossactions);

                                info->bossactions = NULL;
                                info->numbossactions = -1;
                            }
                            else
                            {
                                int i;

                                for (i = 0; i < nummobjtypes; i++)
                                {
                                    char    *name1 = removenonalpha(mobjinfo[i].name1);
                                    char    *name2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                                    char    *name3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);
                                    char    *temp = removenonalpha(sc_String);

                                    if ((name1 && M_StringCompare(name1, temp))
                                        || (name2 && M_StringCompare(name2, temp))
                                        || (name3 && M_StringCompare(name3, temp)))
                                        break;
                                }

                                if (i < nummobjtypes)
                                {
                                    int special;
                                    int tag;

                                    SC_MustGetNumber();
                                    special = (int)sc_Number;

                                    SC_MustGetNumber();
                                    tag = (int)sc_Number;

                                    if (tag
                                        || special == S1_ExitLevel
                                        || special == S1_ExitLevel_GoesToSecretLevel
                                        || special == W1_ExitLevel
                                        || special == W1_ExitLevel_GoesToSecretLevel)
                                    {
                                        if (info->numbossactions == -1)
                                            info->numbossactions = 1;
                                        else
                                            info->numbossactions++;

                                        info->bossactions = (bossaction_t *)I_Realloc(info->bossactions,
                                            sizeof(bossaction_t) * info->numbossactions);

                                        info->bossactions[info->numbossactions - 1].type = i;
                                        info->bossactions[info->numbossactions - 1].special = special;
                                        info->bossactions[info->numbossactions - 1].tag = tag;
                                    }
                                }
                            }

                            break;

                        case MCMD_CLUSTER:
                            SC_MustGetNumber();
                            info->cluster = (int)sc_Number;
                            break;

                        case MCMD_ENDBUNNY:
                            SC_MustGetString();

                            if (SC_Compare("TRUE"))
                                info->endbunny = true;

                            break;

                        case MCMD_ENDCAST:
                            SC_MustGetString();

                            if (SC_Compare("TRUE"))
                                info->endcast = true;

                            break;

                        case MCMD_ENDGAME:
                            SC_MustGetString();

                            if (SC_Compare("TRUE"))
                                info->endgame = true;

                            break;

                        case MCMD_ENDPIC:
                            SC_MustGetString();
                            info->endpic = W_CheckNumForName(sc_String);
                            break;

                        case MCMD_ENTERANIM:
                            SC_MustGetString();
                            M_StringCopy(info->enteranim, sc_String, sizeof(info->enteranim));
                            break;

                        case MCMD_ENTERPIC:
                            SC_MustGetString();
                            info->enterpic = W_CheckNumForName(sc_String);
                            break;

                        case MCMD_EPISODE:
                        {
                            char    lumpname[9];
                            char    string[128];

                            SC_MustGetString();

                            if (SC_Compare("CLEAR"))
                                break;

                            M_StringCopy(lumpname, sc_String, sizeof(lumpname));
                            SC_MustGetString();

                            if (SC_Compare("NAME"))
                            {
                                SC_MustGetString();
                                M_StringCopy(string, sc_String, sizeof(string));
                                SC_MustGetString();

                                if (SC_Compare("PICNAME"))
                                {
                                    SC_MustGetString();
                                    P_ParseMapString(lumpname, &map, &ep);
                                    M_AddEpisode(map, ep, sc_String, string);
                                }
                            }
                            else if (SC_Compare("PICNAME"))
                            {
                                SC_MustGetString();
                                M_StringCopy(string, sc_String, sizeof(string));
                                SC_MustGetString();

                                if (SC_Compare("NAME"))
                                {
                                    SC_MustGetString();
                                    P_ParseMapString(lumpname, &map, &ep);
                                    M_AddEpisode(map, ep, string, sc_String);
                                }
                            }
                            else
                            {
                                M_StringCopy(string, sc_String, sizeof(string));
                                M_AddEpisode(map, ep, lumpname, string);
                            }

                            break;
                        }

                        case MCMD_EXITANIM:
                            SC_MustGetString();
                            M_StringCopy(info->exitanim, sc_String, sizeof(info->exitanim));
                            break;

                        case MCMD_EXITPIC:
                            SC_MustGetString();
                            info->exitpic = W_CheckNumForName(sc_String);
                            break;

                        case MCMD_INTERBACKDROP:
                            SC_MustGetString();
                            M_StringCopy(info->interbackdrop, sc_String, sizeof(info->interbackdrop));
                            break;

                        case MCMD_INTERMUSIC:
                            SC_MustGetString();
                            info->intermusic = W_CheckNumForName(sc_String);
                            break;

                        case MCMD_LABEL:
                            SC_MustGetString();
                            M_StringCopy(info->label, sc_String, sizeof(info->label));
                            break;

                        case MCMD_INTERTEXTSECRET:
                        {
                            char    buffer[1024] = "";
                            bool    firststring = false;

                            while (SC_GetString())
                            {
                                if (SC_MatchString(mapcmdnames) >= 0 || SC_Compare("MAP"))
                                {
                                    SC_UnGet();
                                    break;
                                }

                                if (!firststring)
                                {
                                    firststring = true;
                                    M_StringCopy(buffer, sc_String, sizeof(buffer));
                                }
                                else
                                {
                                    strcat(buffer, "\n");
                                    strcat(buffer, sc_String);
                                }
                            }

                            M_StringCopy(info->intertextsecret, buffer, sizeof(info->intertextsecret));
                            break;
                        }

                        case MCMD_INTERTEXT:
                        {
                            char    buffer[1024] = "";
                            bool    firststring = false;

                            while (SC_GetString())
                            {
                                if (SC_MatchString(mapcmdnames) >= 0 || SC_Compare("MAP"))
                                {
                                    SC_UnGet();
                                    break;
                                }
                                else if (SC_Compare("CLEAR"))
                                {
                                    M_StringCopy(buffer, sc_String, sizeof(buffer));
                                    break;
                                }

                                if (!firststring)
                                {
                                    firststring = true;
                                    M_StringCopy(buffer, sc_String, sizeof(buffer));
                                }
                                else
                                {
                                    strcat(buffer, "\n");
                                    strcat(buffer, sc_String);
                                }
                            }

                            M_StringCopy(info->intertext, buffer, sizeof(info->intertext));
                            break;
                        }

                        case MCMD_LIQUID:
                        {
                            int lump;

                            SC_MustGetString();

                            if ((lump = R_CheckFlatNumForName(sc_String)) >= 0)
                                info->liquid[liquidlumps++] = lump;

                            break;
                        }

                        case MCMD_LEVELNAME:
                            SC_MustGetString();
                            M_StringCopy(info->name, sc_String, sizeof(info->name));
                            break;

                        case MCMD_MUSIC:
                            SC_MustGetString();

                            if (legacyofrust && extras)
                            {
                                sc_String[0] = 'O';

                                if (W_CheckNumForName(sc_String) == -1)
                                    sc_String[0] = 'D';
                            }

                            info->music = W_CheckNumForName(sc_String);
                            break;

                        case MCMD_MUSICARTIST:
                            SC_MustGetString();
                            M_StringCopy(info->musicartist, sc_String, sizeof(info->musicartist));
                            break;

                        case MCMD_MUSICTITLE:
                            SC_MustGetString();
                            M_StringCopy(info->musictitle, sc_String, sizeof(info->musictitle));
                            break;

                        case MCMD_NEXT:
                        {
                            int nextepisode = 1;
                            int nextmap = 0;

                            SC_MustGetString();

                            if (SC_Compare("ENDGAME1"))
                            {
                                info->endgame = true;
                                break;
                            }

                            if (SC_Compare("ENDGAMEC"))
                            {
                                info->endcast = true;
                                break;
                            }

                            if (sscanf(sc_String, "%i", &nextmap) != 1 || nextmap < 0 || nextmap > 99)
                            {
                                char    *buffer = uppercase(sc_String);

                                if (gamemode == commercial)
                                {
                                    if (sscanf(buffer, "MAP0%1i", &nextmap) != 1 && sscanf(buffer, "MAP%2i", &nextmap) != 1)
                                        continue;
                                }
                                else
                                {
                                    if (sscanf(buffer, "E%1iM%i", &nextepisode, &nextmap) != 2)
                                        continue;
                                }

                                free(buffer);
                            }

                            info->next = nextmap;
                            break;
                        }

                        case MCMD_NOBRIGHTMAP:
                        {
                            int texture;

                            SC_MustGetString();

                            if ((texture = R_TextureNumForName(sc_String)) >= 0)
                                nobrightmap[texture] = true;

                            break;
                        }

                        case MCMD_NOJUMP:
                            info->nojump = true;
                            break;

                        case MCMD_NOLIQUID:
                        {
                            int lump;

                            SC_MustGetString();

                            if ((lump = R_CheckFlatNumForName(sc_String)) >= 0)
                                info->noliquid[noliquidlumps++] = lump;

                            break;
                        }

                        case MCMD_NOFREELOOK:
                        case MCMD_NOMOUSELOOK:
                            info->nofreelook = true;
                            break;

                        case MCMD_PAR:
                        case MCMD_PARTIME:
                            SC_MustGetNumber();
                            info->par = (int)sc_Number;

                            break;

                        case MCMD_PISTOLSTART:
                            info->pistolstart = true;
                            break;

                        case MCMD_NEXTSECRET:
                        case MCMD_SECRETNEXT:
                        {
                            int nextepisode = 1;
                            int nextmap = 0;

                            SC_MustGetString();

                            if (sscanf(sc_String, "%i", &nextmap) != 1 || nextmap < 0 || nextmap > 99)
                            {
                                char    *buffer = uppercase(sc_String);

                                if (gamemode == commercial)
                                {
                                    if (sscanf(buffer, "MAP0%1i", &nextmap) != 1 && sscanf(buffer, "MAP%2i", &nextmap) != 1)
                                        continue;
                                }
                                else
                                {
                                    if (sscanf(buffer, "E%1iM%i", &nextepisode, &nextmap) != 2)
                                        continue;
                                }

                                free(buffer);
                            }

                            info->secretnext = nextmap;

                            if (info->next != info->secretnext)
                                mapinfo[nextepisode][nextmap].secret = true;

                            break;
                        }

                        case MCMD_SKY1:
                            SC_MustGetString();
                            info->sky1texture = R_TextureNumForName(sc_String);

                            if (SC_GetNumber())
                                info->sky1scrolldelta = sc_Number;

                            break;

                        case MCMD_SKYTEXTURE:
                            SC_MustGetString();
                            info->sky1texture = R_TextureNumForName(sc_String);
                            break;

                        case MCMD_LEVELPIC:
                        case MCMD_TITLEPATCH:
                            SC_MustGetString();
                            info->titlepatch = W_CheckNumForName(sc_String);
                            break;

                        case MCMD_ALLOWMONSTERTELEFRAGS:
                            SC_MustGetNumber();
                            info->allowmonstertelefrags = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_CORPSEGIBS:
                        case MCMD_COMPAT_VILEGHOSTS:
                            SC_MustGetNumber();
                            info->compat_corpsegibs = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_LIMITPAIN:
                            SC_MustGetNumber();
                            info->compat_limitpain = !!((int)sc_Number);
                            break;

                        case MCMD_NOGRADUALLIGHTING:
                            SC_MustGetNumber();
                            info->nograduallighting = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_FLOORMOVE:
                            SC_MustGetNumber();
                            info->compat_floormove = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_LIGHT:
                            SC_MustGetNumber();
                            info->compat_light = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_NOPASSOVER:
                            SC_MustGetNumber();
                            info->compat_nopassover = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_STAIRS:
                            SC_MustGetNumber();
                            info->compat_stairs = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_USEBLOCKING:
                            SC_MustGetNumber();
                            info->compat_useblocking = !!((int)sc_Number);
                            break;

                        case MCMD_COMPAT_ZOMBIE:
                            SC_MustGetNumber();
                            info->compat_zombie = !!((int)sc_Number);
                            break;
                    }
            }

            if (REKKR && ep <= 2)
                info->sky1scrolldelta = 0.05f;

            mapmax = MAX(map, mapmax);
        }
        else if (SC_Compare("LIQUID"))
        {
            int lump;

            SC_MustGetString();

            if ((lump = R_CheckFlatNumForName(sc_String)) >= 0)
                terraintypes[lump] = LIQUID;
        }
        else if (SC_Compare("NOLIQUID"))
        {
            int lump;

            SC_MustGetString();

            if ((lump = R_CheckFlatNumForName(sc_String)) >= 0)
                terraintypes[lump] = SOLID;
        }
    }

    SC_Close();

    if (customepisodes)
    {
        for (int i = 0; i < 100; i++)
            mapinfo[1][i].mapinepisode = mapinepisode(i);

        if (EpiDef.laston >= EpiDef.numitems)
        {
            EpiDef.laston = 0;
            episode = 1;
            M_SaveCVARs();
        }
    }

    temp1 = commify(sc_Line);
    temp2 = uppercase(scriptname);
    C_Output("%s line%s have been parsed in the " BOLD("%s") " lump in the %s " BOLD("%s") ".",
        temp1, (sc_Line == 1 ? "" : "s"), temp2, (lumpinfo[MAPINFO]->wadfile->type == IWAD ? "IWAD" : "PWAD"),
        lumpinfo[MAPINFO]->wadfile->path);
    free(temp1);
    free(temp2);

    return true;
}

char *P_GetMapAuthor(const int ep, const int map)
{
    return (MAPINFO >= 0 && mapinfo[ep][map].author[0] ? mapinfo[ep][map].author : (((E1M4B || *speciallumpname) && map == 4)
        || ((E1M8B || *speciallumpname) && map == 8) || (onehumanity && map == 1) ? s_AUTHOR_ROMERO : ""));
}

int P_GetNumBossActions(const int ep, const int map)
{
    return mapinfo[ep][map].numbossactions;
}

bossaction_t *P_GetBossAction(const int ep, const int map, const int i)
{
    return &mapinfo[ep][map].bossactions[i];
}

char *P_GetInterBackrop(const int ep, const int map)
{
    return mapinfo[ep][map].interbackdrop;
}

int P_GetInterMusic(const int ep, const int map)
{
    return mapinfo[ep][map].intermusic;
}

char *P_GetInterText(const int ep, const int map)
{
    return mapinfo[ep][map].intertext;
}

char *P_GetInterSecretText(const int ep, const int map)
{
    return mapinfo[ep][map].intertextsecret;
}

char *P_GetLabel(const int ep, const int map)
{
    return mapinfo[ep][map].label;
}

bool P_GetMapEndBunny(const int ep, const int map)
{
    return mapinfo[ep][map].endbunny;
}

bool P_GetMapEndCast(const int ep, const int map)
{
    return mapinfo[ep][map].endcast;
}

bool P_GetMapEndGame(const int ep, const int map)
{
    return mapinfo[ep][map].endgame;
}

int P_GetMapEndPic(const int ep, const int map)
{
    return mapinfo[ep][map].endpic;
}

char *P_GetMapEnterAnim(const int ep, const int map)
{
    return mapinfo[ep][map].enteranim;
}

int P_GetMapEnterPic(const int ep, const int map)
{
    return mapinfo[ep][map].enterpic;
}

char *P_GetMapExitAnim(const int ep, const int map)
{
    return mapinfo[ep][map].exitanim;
}

int P_GetMapExitPic(const int ep, const int map)
{
    return mapinfo[ep][map].exitpic;
}

void P_GetMapLiquids(const int ep, const int map)
{
    for (int i = 0; i < liquidlumps; i++)
        terraintypes[mapinfo[ep][map].liquid[i]] = LIQUID;
}

int P_GetMapMusic(const int ep, const int map)
{
    return mapinfo[ep][map].music;
}

char *P_GetMapMusicComposer(const int ep, const int map)
{
    return mapinfo[ep][map].musicartist;
}

char *P_GetMapMusicTitle(const int ep, const int map)
{
    return mapinfo[ep][map].musictitle;
}

char *P_GetMapName(const int ep, const int map)
{
    return (MAPINFO >= 0 && !sigil ? mapinfo[ep][map].name : ((E1M4B || *speciallumpname) && map == 4 ? s_HUSTR_E1M4B :
        ((E1M8B || *speciallumpname) && map == 8 ? s_HUSTR_E1M8B : "")));
}

int P_GetMapNext(const int ep, const int map)
{
    return mapinfo[ep][map].next;
}

void P_GetMapNoLiquids(const int ep, const int map)
{
    for (int i = 0; i < noliquidlumps; i++)
        terraintypes[mapinfo[ep][map].noliquid[i]] = SOLID;
}

int P_GetMapPar(const int ep, const int map)
{
    return mapinfo[ep][map].par;
}

bool P_GetMapPistolStart(const int ep, const int map)
{
    return mapinfo[ep][map].pistolstart;
}

int P_GetMapSecretNext(const int ep, const int map)
{
    return mapinfo[ep][map].secretnext;
}

int P_GetMapSky1Texture(const int ep, const int map)
{
    return mapinfo[ep][map].sky1texture;
}

float P_GetMapSky1ScrollDelta(const int ep, const int map)
{
    return mapinfo[ep][map].sky1scrolldelta;
}

int P_GetMapTitlePatch(const int ep, const int map)
{
    return mapinfo[ep][map].titlepatch;
}

int P_GetAllowMonsterTelefrags(const int ep, const int map)
{
    return mapinfo[ep][map].allowmonstertelefrags;
}

bool P_IsSecret(const int ep, const int map)
{
    return mapinfo[ep][map].secret;
}

int P_GetMapInEpisode(const int map)
{
    return mapinfo[1][map].mapinepisode;
}

//
// P_Init
//
void P_Init(void)
{
    P_InitSwitchList();
    P_InitPicAnims();

    if (!M_CheckParm("-nomapinfo"))
    {
        P_InitMapInfo();

        if (!M_CheckParm("-normapinfo") && !P_ParseMapInfo("RMAPINFO"))
            if (!M_CheckParm("-noumapinfo") && !P_ParseMapInfo("UMAPINFO"))
                if (!P_ParseMapInfo("MAPINFO"))
                    if (!M_CheckParm("-nozmapinfo") && !P_ParseMapInfo("ZMAPINFO"))
                        if (!M_CheckParm("-nodmapinfo"))
                            P_ParseMapInfo("DMAPINFO");

        if (nojump && (keyboardjump || keyboardjump2 || mousejump != -1 || controllerjump))
            C_Warning(1, "This %s has disabled use of the " BOLD("+jump") " action.",
                (lumpinfo[MAPINFO]->wadfile->type == IWAD ? "IWAD" : "PWAD"));

        if (nofreelook)
            C_Warning(1, "This %s has disabled use of the " BOLD("freelook") " CVAR and " BOLD("+freelook") " action.",
                (lumpinfo[MAPINFO]->wadfile->type == IWAD ? "IWAD" : "PWAD"));
    }

    R_InitSprites();
    P_CheckSpechits();
    P_CheckIntercepts();

    linespecials[Scroll_ScrollWallWithSameTagUsingSidedefOffsets] =
        "Scroll wall with same tag using sidedef offsets";
    linespecials[Scroll_ScrollWallWithSameTagUsingSidedefOffsetsWhenSectorChangesHeight] =
        "Scroll wall with same tag using sidedef offsets when sector changes height";
    linespecials[Scroll_ScrollWallWithSameTagUsingSidedefOffsetsAcceleratesWhenSectorChangesHeight] =
        "Scroll wall with same tag using sidedef offsets (accelerates when sector changes height)";
}
