/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

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
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_fix.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sc_man.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"

#define RMAPINFO_SCRIPT_NAME    "RMAPINFO"
#define MAPINFO_SCRIPT_NAME     "MAPINFO"
#define UMAPINFO_SCRIPT_NAME    "UMAPINFO"

#define MAXMAPINFO              100

#define NUMLIQUIDS              256

#define MCMD_AUTHOR             1
#define MCMD_CLUSTER            2
#define MCMD_ENDBUNNY           3
#define MCMD_ENDCAST            4
#define MCMD_ENDGAME            5
#define MCMD_ENDPIC             6
#define MCMD_EPISODE            7
#define MCMD_ENTERPIC           8
#define MCMD_EXITPIC            9
#define MCMD_INTERBACKDROP      10
#define MCMD_INTERMUSIC         11
#define MCMD_INTERTEXT          12
#define MCMD_INTERTEXTSECRET    13
#define MCMD_LEVELNAME          14
#define MCMD_LEVELPIC           15
#define MCMD_LIQUID             16
#define MCMD_MUSIC              17
#define MCMD_MUSICCOMPOSER      18
#define MCMD_MUSICTITLE         19
#define MCMD_NEXT               20
#define MCMD_NEXTSECRET         21
#define MCMD_NOBRIGHTMAP        22
#define MCMD_NOFREELOOK         23
#define MCMD_NOJUMP             24
#define MCMD_NOLIQUID           25
#define MCMD_NOMOUSELOOK        26
#define MCMD_PAR                27
#define MCMD_PARTIME            28
#define MCMD_PISTOLSTART        29
#define MCMD_SECRETNEXT         30
#define MCMD_SKY1               31
#define MCMD_SKYTEXTURE         32
#define MCMD_TITLEPATCH         33

typedef struct mapinfo_s mapinfo_t;

struct mapinfo_s
{
    char        author[128];
    int         cluster;
    dboolean    endbunny;
    dboolean    endcast;
    dboolean    endgame;
    int         endpic;
    int         enterpic;
    int         exitpic;
    char        interbackdrop[9];
    int         intermusic;
    char        intertext[1024];
    char        intertextsecret[1024];
    int         liquid[NUMLIQUIDS];
    int         music;
    char        musiccomposer[128];
    char        musictitle[128];
    char        name[128];
    int         next;
    dboolean    nojump;
    int         noliquid[NUMLIQUIDS];
    dboolean    nomouselook;
    int         par;
    dboolean    pistolstart;
    int         secretnext;
    int         sky1texture;
    int         sky1scrolldelta;
    int         titlepatch;
};

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
static int          mapcount;

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

// for large maps, wad is 16bit
int                 *blockmap;

// offsets in blockmap are from here
int                 *blockmaplump;

// origin of block map
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

dboolean            skipblstart;            // MaxW: Skip initial blocklist short

// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
static int          rejectlump = -1;        // cph - store reject lump num if cached
const byte          *rejectmatrix;          // cph - const*

static mapinfo_t    mapinfo[MAXMAPINFO];

static char *mapcmdnames[] =
{
    "AUTHOR",
    "ENDBUNNY",
    "ENDCAST",
    "ENDGAME",
    "ENDPIC",
    "ENTERPIC",
    "EXITPIC",
    "EPISODE",
    "INTERBACKDROP",
    "INTERMUSIC",
    "INTERTEXT",
    "INTERTEXTSECRET",
    "LIQUID",
    "LEVELNAME",
    "LEVELPIC",
    "MUSIC",
    "MUSICCOMPOSER",
    "MUSICTITLE",
    "NEXT",
    "NEXTSECRET",
    "NOBRIGHTMAP",
    "NOFREELOOK",
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
    MCMD_AUTHOR,
    MCMD_ENDBUNNY,
    MCMD_ENDCAST,
    MCMD_ENDGAME,
    MCMD_ENDPIC,
    MCMD_ENTERPIC,
    MCMD_EXITPIC,
    MCMD_EPISODE,
    MCMD_INTERBACKDROP,
    MCMD_INTERMUSIC,
    MCMD_INTERTEXT,
    MCMD_INTERTEXTSECRET,
    MCMD_LIQUID,
    MCMD_LEVELNAME,
    MCMD_LEVELPIC,
    MCMD_MUSIC,
    MCMD_MUSICCOMPOSER,
    MCMD_MUSICTITLE,
    MCMD_NEXT,
    MCMD_NEXTSECRET,
    MCMD_NOBRIGHTMAP,
    MCMD_NOFREELOOK,
    MCMD_NOJUMP,
    MCMD_NOLIQUID,
    MCMD_NOMOUSELOOK,
    MCMD_PAR,
    MCMD_PARTIME,
    MCMD_PISTOLSTART,
    MCMD_SECRETNEXT,
    MCMD_SKY1,
    MCMD_SKYTEXTURE,
    MCMD_TITLEPATCH,
};

dboolean        canmodify;
dboolean        transferredsky;
static int      UMAPINFO;
static int      RMAPINFO;
static int      MAPINFO;

dboolean        r_fixmaperrors = r_fixmaperrors_default;

dboolean        samelevel;

mapformat_t     mapformat;

const char *mapformats[] =
{
    "Regular",
    "<i>DeeP</i>",
    "<i>ZDoom</i> extended (uncompressed)"
};

dboolean        boomcompatible;
dboolean        mbfcompatible;
dboolean        blockmaprebuilt;
dboolean        nojump = false;
dboolean        nomouselook = false;

const char *linespecials[] =
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

static fixed_t GetOffset(vertex_t *v1, vertex_t *v2)
{
    fixed_t dx = (v1->x - v2->x) >> FRACBITS;
    fixed_t dy = (v1->y - v2->y) >> FRACBITS;

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
            if (canmodify && r_fixmaperrors)
                for (int j = 0; vertexfix[j].mission != -1; j++)
                    if (gamemission == vertexfix[j].mission && gameepisode == vertexfix[j].episode && gamemap == vertexfix[j].map
                        && i == vertexfix[j].vertex && vertexes[i].x == vertexfix[j].oldx << FRACBITS
                        && vertexes[i].y == vertexfix[j].oldy << FRACBITS)
                    {
                        char    *temp = commify(vertexfix[j].vertex);

                        C_Warning(2, "Vertex %s has been moved from (%i,%i) to (%i,%i).",
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

    for (int i = numlines; i--; ld++)
        if (!ld->special)
        {
            if (ld->tag)
            {
                char    *temp1 = commify(ld->id);
                char    *temp2 = commify(ld->tag);

                if (ld->tag < 0 || P_FindSectorFromLineTag(ld, -1) == -1)
                    C_Warning(2, "Linedef %s has an unknown tag of %s and no line special.", temp1, temp2);
                else
                    C_Warning(2, "Linedef %s has a tag of %s but no line special.", temp1, temp2);

                free(temp1);
                free(temp2);
            }
        }
        else if (ld->special <= NUMLINESPECIALS)
        {
            if (!P_CheckTag(ld))
            {
                char    *temp = commify(ld->id);

                C_Warning(2, "Linedef %s has the %sline special %i (\"%s\") but no tag.",
                    temp, (ld->special < BOOMLINESPECIALS ? "" : (ld->special < MBFLINESPECIALS ? "<i>MBF</i>-compatible " :
                    "<i>BOOM</i>-compatible ")), ld->special, linespecials[ld->special]);
                free(temp);
            }
            else if (ld->tag < 0 || P_FindSectorFromLineTag(ld, -1) == -1)
            {
                char    *temp1 = commify(ld->id);
                char    *temp2 = commify(ld->tag);

                C_Warning(2, "Linedef %s has the %sline special %i (\"%s\") but an unknown tag of %s.",
                    temp1, (ld->special < BOOMLINESPECIALS ? "" : (ld->special < MBFLINESPECIALS ? "<i>MBF</i>-compatible " :
                    "<i>BOOM</i>-compatible ")), ld->special, linespecials[ld->special], temp2);
                free(temp1);
                free(temp2);
            }
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

    for (int i = 0; i < numsegs; i++)
    {
        seg_t           *li = segs + i;
        const mapseg_t  *ml = data + i;
        unsigned short  v1;
        unsigned short  v2;
        int             side;
        int             linedefnum;
        line_t          *ldef;

        v1 = (unsigned short)SHORT(ml->v1);
        v2 = (unsigned short)SHORT(ml->v2);
        linedefnum = (unsigned short)SHORT(ml->linedef);

        if (linedefnum >= numlines)
        {
            char    *temp1 = commify(i);
            char    *temp2 = commify(linedefnum);

            I_Error("Seg %s references an invalid linedef of %s.", temp1, temp2);
            free(temp1);
            free(temp2);
        }

        ldef = lines + linedefnum;
        li->linedef = ldef;
        side = SHORT(ml->side);

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            char    *temp1 = commify(i);
            char    *temp2 = commify(side);

            C_Warning(2, "Seg %s has a wrong side index of %s. It has been changed to 1.", temp1, temp2);
            side = 1;
            free(temp1);
            free(temp2);
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
        {
            char    *temp1 = commify(linedefnum);
            char    *temp2 = commify(i);
            char    *temp3 = commify(ldef->sidenum[side]);

            I_Error("Linedef %s for seg %s references an invalid sidedef of %s.", temp1, temp2, temp3);
            free(temp1);
            free(temp2);
            free(temp3);
        }

        li->sidedef = sides + ldef->sidenum[side];

        // cph 2006/09/30 - our frontsector can be the second side of the
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
        // see e1m9 @ NIVELES.WAD
        // <https://www.doomworld.com/idgames/index.php?id=12647>
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

        li->offset = GetOffset(li->v1, (side ? ldef->v2 : ldef->v1));

        // [BH] Apply any map-specific fixes.
        if (canmodify && r_fixmaperrors)
            for (int j = 0; linefix[j].mission != -1; j++)
                if (gamemission == linefix[j].mission && gameepisode == linefix[j].episode && gamemap == linefix[j].map
                    && linedefnum == linefix[j].linedef && side == linefix[j].side)
                {
                    if (*linefix[j].toptexture)
                    {
                        int     texture = R_TextureNumForName(linefix[j].toptexture);
                        char    *temp = commify(linedefnum);

                        if (!texture)
                            C_Warning(2, "The unused top texture of linedef %s has been removed.", temp);
                        else
                        {
                            if (!li->sidedef->toptexture)
                                C_Warning(2, "The missing top texture of linedef %s is now <b>%.8s</b>.",
                                    temp, linefix[j].toptexture);
                            else
                                C_Warning(2, "The top texture of linedef %s has been changed from <b>%.8s</b> to <b>%.8s</b>.",
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
                                C_Warning(2, "The missing middle texture of linedef %s is now <b>%.8s</b>.",
                                    temp, linefix[j].middletexture);
                            else
                                C_Warning(2, "The middle texture of linedef %s has been changed from <b>%.8s</b> to <b>%.8s</b>.",
                                    temp, textures[li->sidedef->midtexture]->name, linefix[j].middletexture);
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
                                C_Warning(2, "The missing bottom texture of linedef %s is now <b>%.8s</b>.",
                                    temp, linefix[j].bottomtexture);
                            else
                                C_Warning(2, "The bottom texture of linedef %s has been changed from <b>%.8s</b> to <b>%.8s</b>.",
                                    temp, textures[li->sidedef->bottomtexture]->name, linefix[j].bottomtexture);
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
                        char    *temp = commify(linedefnum);

                        if (linefix[j].special)
                            C_Warning(2, "The %sline special of linedef %s has been changed from %i (\"%s\") to %i (\"%s\").",
                                (li->linedef->special < BOOMLINESPECIALS ? "" : (li->linedef->special < MBFLINESPECIALS ?
                                "<i>MBF</i>-compatible " : "<i>BOOM</i>-compatible ")), temp, li->linedef->special,
                                linespecials[li->linedef->special], linefix[j].special, linespecials[linefix[j].special]);
                        else
                            C_Warning(2, "The %sline special of linedef %s has been removed.",
                                (li->linedef->special < BOOMLINESPECIALS ? "" : (li->linedef->special < MBFLINESPECIALS ?
                                "<i>MBF</i>-compatible " : "<i>BOOM</i>-compatible ")), temp);

                        li->linedef->special = linefix[j].special;
                        free(temp);

                    }

                    if (linefix[j].tag != DEFAULT)
                    {
                        char    *temp1 = commify(linedefnum);

                        if (linefix[j].tag)
                        {
                            char    *temp2 = commify(li->linedef->tag);
                            char    *temp3 = commify(linefix[j].tag);

                            C_Warning(2, "The tag of linedef %s has been changed from %s to %s.", temp1, temp2, temp3);
                            free(temp2);
                            free(temp3);
                        }
                        else
                            C_Warning(2, "The tag of linedef %s has been removed.", temp1);

                        li->linedef->tag = linefix[j].tag;
                        free(temp1);
                    }

                    break;
                }

        if (li->linedef->special >= MBFLINESPECIALS)
            mbfcompatible = true;
        else if (li->linedef->special >= BOOMLINESPECIALS)
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
        int                 v1;
        int                 v2;
        int                 side;
        int                 linedefnum;
        line_t              *ldef;

        v1 = ml->v1;
        v2 = ml->v2;
        linedefnum = (unsigned short)SHORT(ml->linedef);

        // e6y: check for wrong indexes
        if (linedefnum >= numlines)
        {
            char    *temp1 = commify(i);
            char    *temp2 = commify(linedefnum);

            I_Error("Seg %s references an invalid linedef of %s.", temp1, temp2);
            free(temp1);
            free(temp2);
        }

        ldef = lines + linedefnum;
        li->linedef = ldef;
        side = SHORT(ml->side);

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            char    *temp1 = commify(i);
            char    *temp2 = commify(side);

            C_Warning(2, "Seg %s has a wrong side index of %s. It has been changed to 1.", temp1, temp2);
            side = 1;
            free(temp1);
            free(temp2);
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
        {
            char    *temp1 = commify(linedefnum);
            char    *temp2 = commify(i);
            char    *temp3 = commify(ldef->sidenum[side]);

            I_Error("Linedef %s for seg %s references an invalid sidedef of %s.", temp1, temp2, temp3);
            free(temp1);
            free(temp2);
            free(temp3);
        }

        li->sidedef = sides + ldef->sidenum[side];

        // cph 2006/09/30 - our frontsector can be the second side of the
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
        // see e1m9 @ NIVELES.WAD
        // <https://www.doomworld.com/idgames/index.php?id=12647>
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

        li->offset = GetOffset(li->v1, (side ? ldef->v2 : ldef->v1));

        if (li->linedef->special >= MBFLINESPECIALS)
            mbfcompatible = true;
        else if (li->linedef->special >= BOOMLINESPECIALS)
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

        // [BH] Apply any level-specific fixes.
        if (canmodify && r_fixmaperrors)
            for (int j = 0; sectorfix[j].mission != -1; j++)
                if (gamemission == sectorfix[j].mission && gameepisode == sectorfix[j].episode && gamemap == sectorfix[j].map
                    && i == sectorfix[j].sector)
                {
                    if (*sectorfix[j].floorpic)
                    {
                        char    *temp = commify(sectorfix[j].sector);

                        C_Warning(2, "The floor texture of sector %s has been changed from <b>%.8s</b> to <b>%.8s</b>.",
                            temp, lumpinfo[ss->floorpic + firstflat]->name, sectorfix[j].floorpic);

                        ss->floorpic = R_FlatNumForName(sectorfix[j].floorpic);
                        free(temp);
                    }

                    if (*sectorfix[j].ceilingpic)
                    {
                        char    *temp = commify(sectorfix[j].sector);

                        C_Warning(2, "The ceiling texture of sector %s has been changed from <b>%.8s</b> to <b>%.8s</b>.",
                            temp, lumpinfo[ss->ceilingpic + firstflat]->name, sectorfix[j].ceilingpic);

                        ss->ceilingpic = R_FlatNumForName(sectorfix[j].ceilingpic);
                        free(temp);
                    }

                    if (sectorfix[j].floorheight != DEFAULT)
                    {
                        char    *temp1 = commify(sectorfix[j].sector);
                        char    *temp2 = commify(ss->floorheight);
                        char    *temp3 = commify(sectorfix[j].floorheight);

                        C_Warning(2, "The floor height of sector %s has been changed from %s to %s.", temp1, temp2, temp3);

                        ss->floorheight = sectorfix[j].floorheight << FRACBITS;
                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    if (sectorfix[j].ceilingheight != DEFAULT)
                    {
                        char    *temp1 = commify(sectorfix[j].sector);
                        char    *temp2 = commify(ss->ceilingheight);
                        char    *temp3 = commify(sectorfix[j].ceilingheight);

                        C_Warning(2, "The ceiling height of sector %s has been changed from %s to %s.", temp1, temp2, temp3);

                        ss->ceilingheight = sectorfix[j].ceilingheight << FRACBITS;
                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    if (sectorfix[j].special != DEFAULT)
                    {
                        char    *temp = commify(sectorfix[j].sector);

                        C_Warning(2, "The special of sector %s has been changed from %i (\"%s\") to %i (\"%s\").",
                            temp, ss->special, sectorspecials[ss->special],
                            sectorfix[j].special, sectorspecials[sectorfix[j].special]);

                        ss->special = sectorfix[j].special;
                        free(temp);
                    }

                    if (sectorfix[j].tag != DEFAULT)
                    {
                        char    *temp1 = commify(sectorfix[j].sector);
                        char    *temp2 = commify(ss->tag);
                        char    *temp3 = commify(sectorfix[j].tag);

                        C_Warning(2, "The tag of sector %s has been changed from %s to %s.", temp1, temp2, temp3);

                        ss->tag = sectorfix[j].tag;
                        free(temp1);
                        free(temp2);
                        free(temp3);
                    }

                    break;
                }

        // [AM] Sector interpolation. Even if we're
        //      not running uncapped, the renderer still
        //      uses this data.
        ss->oldfloorheight = ss->floorheight;
        ss->interpfloorheight = ss->floorheight;
        ss->oldceilingheight = ss->ceilingheight;
        ss->interpceilingheight = ss->ceilingheight;

        switch (ss->special)
        {
            case DamageNegative10Or20PercentHealthAndLightBlinks_2Hz:
            case DamageNegative5Or10PercentHealth:
            case DamageNegative2Or5PercentHealth:
            case DamageNegative10Or20PercentHealthAndEndLevel:
            case DamageNegative10Or20PercentHealth:
                numdamaging++;

            default:
                if ((ss->special & DAMAGE_MASK) >> DAMAGE_SHIFT)
                    numdamaging++;

                break;
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
        {
            char    *temp1 = commify(i);
            char    *temp2 = commify(linedefnum);

            I_Error("Seg %s references an invalid linedef of %s.", temp1, temp2);
            free(temp1);
            free(temp2);
        }

        ldef = lines + linedefnum;
        li->linedef = ldef;
        side = ml->side;

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            char    *temp1 = commify(i);
            char    *temp2 = commify(side);

            C_Warning(2, "Seg %s has a wrong side index of %s. It has been changed to 1.", temp1, temp2);
            side = 1;
            free(temp1);
            free(temp2);
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
        {
            char    *temp1 = commify(linedefnum);
            char    *temp2 = commify(i);
            char    *temp3 = commify(ldef->sidenum[side]);

            I_Error("Linedef %s for seg %s references an invalid sidedef of %s.", temp1, temp2, temp3);
            free(temp1);
            free(temp2);
            free(temp3);
        }

        li->sidedef = sides + ldef->sidenum[side];

        // cph 2006/09/30 - our frontsector can be the second side of the
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

        li->offset = GetOffset(li->v1, (side ? ldef->v2 : ldef->v1));

        if (li->linedef->special >= MBFLINESPECIALS)
            mbfcompatible = true;
        else if (li->linedef->special >= BOOMLINESPECIALS)
            boomcompatible = true;
    }
}

// MB 2020-03-01: Fix endianness for 32-bit ZDoom nodes
// <https://zdoom.org/wiki/Node#ZDoom_extended_nodes>
static void P_LoadZNodes(int lump)
{
    byte            *data = W_CacheLumpNum(lump);
    unsigned int    orgVerts;
    unsigned int    newVerts;
    unsigned int    numSubs;
    unsigned int    currSeg = 0;
    unsigned int    numSegs;
    unsigned int    numNodes;
    vertex_t        *newvertarray = NULL;

    // skip header
    data += 4;

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

    W_ReleaseLumpNum(lump);

    P_CheckLinedefs();
}

//
// P_LoadThings
//
static void P_LoadThings(int lump)
{
    const mapthing_t    *data = (const mapthing_t *)W_CacheLumpNum(lump);
    int                 numthings;

    if (!data || !(numthings = W_LumpLength(lump) / sizeof(mapthing_t)))
        I_Error("There are no things in this map.");

    M_BigSeed(numthings);
    numspawnedthings = 0;
    numdecorations = 0;

    prevthingx = 0;
    prevthingy = 0;
    prevthingbob = 0;

    for (thingid = 0; thingid < numthings; thingid++)
    {
        mapthing_t  mt = data[thingid];
        dboolean    spawn = true;
        short       type = SHORT(mt.type);

        if (gamemode != commercial && type >= ArchVile && type <= MonstersSpawner && W_CheckMultipleLumps("DEHACKED") == 1)
        {
            int         doomednum = P_FindDoomedNum(type);
            static char buffer[128];

            M_StringCopy(buffer, mobjinfo[doomednum].plural1, sizeof(buffer));

            if (!*buffer)
                M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[doomednum].name1);

            buffer[0] = toupper(buffer[0]);
            C_Warning(2, "%s can't be spawned in <i><b>%s.</b></i>", buffer, gamedescription);
            continue;
        }

        // Do spawn all other stuff.
        mt.x = SHORT(mt.x);
        mt.y = SHORT(mt.y);
        mt.angle = SHORT(mt.angle);
        mt.type = type;
        mt.options = SHORT(mt.options);

        // [BH] Apply any level-specific fixes.
        if (canmodify && r_fixmaperrors)
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
                        C_Warning(2, "The position of thing %s has been changed from (%i,%i) to (%i,%i).",
                            temp, mt.x, mt.y, thingfix[j].newx, thingfix[j].newy);

                        mt.x = thingfix[j].newx;
                        mt.y = thingfix[j].newy;

                        if (thingfix[j].angle != DEFAULT)
                        {
                            C_Warning(2, "The angle of thing %s has been changed from %i\xB0 to %i\xB0.",
                                temp, mt.angle, thingfix[j].angle);

                            mt.angle = thingfix[j].angle;
                        }

                        if (thingfix[j].options != DEFAULT)
                        {
                            C_Warning(2, "The flags of thing %s have been changed from %i to %i.",
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
            if (mt.type == WolfensteinSS && !allowwolfensteinss && !states[S_SSWV_STND].dehacked)
                mt.type = Zombieman;

            if ((thing = P_SpawnMapThing(&mt, !nomonsters)))
            {
                int flags = thing->flags;

                if ((flags & MF_TOUCHY) || (flags & MF_BOUNCES) || (flags & MF_FRIEND))
                    mbfcompatible = true;

                thing->id = thingid;
            }
        }
    }

    M_Seed((unsigned int)time(NULL));
    M_BigSeed((unsigned int)time(NULL));
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
            sides[*ld->sidenum].special = ld->special;
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
        // cph 2006/09/30 - fix sidedef errors right away
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

        // killough 11/98: fix common wad errors (missing sidedefs):
        if (ld->sidenum[0] == NO_INDEX)
        {
            char    *temp = commify(ld->id);

            C_Warning(2, "Linedef %s is missing its first sidedef.", temp);
            ld->sidenum[0] = 0;                         // Substitute dummy sidedef for missing right side
            free(temp);
        }

        if (ld->sidenum[1] == NO_INDEX && (ld->flags & ML_TWOSIDED))
        {
            char    *temp = commify(ld->id);

            C_Warning(2, "Linedef %s has the two-sided flag set but no second sidedef.", temp);
            ld->flags &= ~ML_TWOSIDED;                  // Clear 2s flag for missing left side
            free(temp);
        }

        ld->frontsector = (ld->sidenum[0] != NO_INDEX ? sides[ld->sidenum[0]].sector : NULL);
        ld->backsector = (ld->sidenum[1] != NO_INDEX ? sides[ld->sidenum[1]].sector : NULL);

        // killough 04/11/98: handle special types
        switch (ld->special)
        {
            case Translucent_MiddleTexture:             // killough 04/11/98: translucent 2s textures
            {
                int lump = sides[*ld->sidenum].special; // translucency from sidedef

                if (!ld->tag)                           // if tag == 0,
                    ld->tranlump = lump;                // affect this linedef only
                else
                    for (int j = 0; j < numlines; j++)  // if tag != 0,
                        if (lines[j].tag == ld->tag)    // affect all matching linedefs
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

        sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;

        // cph 2006/09/30 - catch out-of-range sector numbers; use sector 0 instead
        if (sector_num >= numsectors)
        {
            char    *temp1 = commify(i);
            char    *temp2 = commify(sector_num);

            C_Warning(2, "Sidedef %s references an invalid sector of %s.", temp1, temp2);
            sector_num = 0;
            free(temp1);
            free(temp2);
        }

        sd->sector = sec = sectors + sector_num;

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
static dboolean P_VerifyBlockMap(int count)
{
    dboolean    isvalid = true;
    int         *maxoffs = blockmaplump + count;

    skipblstart = true;

    for (int y = 0; y < bmapheight; y++)
    {
        for (int x = 0; x < bmapwidth; x++)
        {
            int offset = y * bmapwidth + x;
            int *list;
            int *blockoffset = blockmaplump + offset + 4;

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
    int     i;
    fixed_t minx = FIXED_MAX;
    fixed_t miny = FIXED_MAX;
    fixed_t maxx = FIXED_MIN;
    fixed_t maxy = FIXED_MIN;

    blockmaprebuilt = true;

    for (i = 0; i < numvertexes; i++)
    {
        if ((vertexes[i].x >> FRACBITS) < minx)
            minx = vertexes[i].x >> FRACBITS;
        else if ((vertexes[i].x >> FRACBITS) > maxx)
            maxx = vertexes[i].x >> FRACBITS;

        if ((vertexes[i].y >> FRACBITS) < miny)
            miny = vertexes[i].y >> FRACBITS;
        else if ((vertexes[i].y >> FRACBITS) > maxy)
            maxy = vertexes[i].y >> FRACBITS;
    }

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

    // Compute blockmap, which is stored as a 2d array of variable-sized lists.
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
            int n;
            int nalloc;
            int *list;
        } bmap_t;

        unsigned int    tot = bmapwidth * bmapheight;           // size of blockmap
        bmap_t          *bmap = calloc(tot, sizeof(*bmap));     // array of blocklists

        for (i = 0; i < numlines; i++)
        {
            // starting coordinates
            int x = (lines[i].v1->x >> FRACBITS) - minx;
            int y = (lines[i].v1->y >> FRACBITS) - miny;

            // x - y deltas
            int adx = lines[i].dx >> FRACBITS;
            int dx = SIGN(adx);
            int ady = lines[i].dy >> FRACBITS;
            int dy = SIGN(ady);

            // difference in preferring to move across y (> 0) instead of x (< 0)
            int diff = (!adx ? 1 : (!ady ? -1 : (((x >> MAPBTOFRAC) << MAPBTOFRAC)
                    + (dx > 0 ? MAPBLOCKUNITS - 1 : 0) - x) * (ady = ABS(ady)) * dx
                    - (((y >> MAPBTOFRAC) << MAPBTOFRAC) + (dy > 0 ? MAPBLOCKUNITS - 1 : 0) - y)
                    * (adx = ABS(adx)) * dy));

            // starting block, and pointer to its blocklist structure
            int b = (y >> MAPBTOFRAC) * bmapwidth + (x >> MAPBTOFRAC);

            // ending block
            int bend = (((lines[i].v2->y >> FRACBITS) - miny) >> MAPBTOFRAC) * bmapwidth
                    + (((lines[i].v2->x >> FRACBITS) - minx) >> MAPBTOFRAC);

            // delta for pointer when moving across y
            dy *= bmapwidth;

            // deltas for diff inside the loop
            adx <<= MAPBTOFRAC;
            ady <<= MAPBTOFRAC;

            // Now we simply iterate block-by-block until we reach the end block.
            while ((unsigned int)b < tot)       // failsafe -- should ALWAYS be true
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
            int count = tot + 6;                // we need at least 1 word per block, plus reserved's

            for (i = 0; (unsigned int)i < tot; i++)
                if (bmap[i].n)
                    count += bmap[i].n + 2;     // 1 header word + 1 trailer word + blocklist

            // Allocate blockmap lump with computed count
            blockmaplump = malloc_IfSameLevel(blockmaplump, sizeof(*blockmaplump) * count);
        }

        // Now compress the blockmap.
        {
            int     ndx = (tot += 4);           // Advance index to start of linedef lists
            bmap_t  *bp = bmap;                 // Start of uncompressed blockmap

            blockmaplump[ndx++] = 0;            // Store an empty blockmap list at start
            blockmaplump[ndx++] = -1;           // (Used for compression)

            for (i = 4; (unsigned int)i < tot; i++, bp++)
                if (bp->n)                                              // Non-empty blocklist
                {
                    blockmaplump[(blockmaplump[i] = ndx++)] = 0;        // Store index & header

                    do
                        blockmaplump[ndx++] = bp->list[--bp->n];        // Copy linedef list
                    while (bp->n);

                    blockmaplump[ndx++] = -1;                           // Store trailer
                    free(bp->list);                                     // Free linedef list
                }
                else
                    // Empty blocklist: point to reserved empty blocklist
                    blockmaplump[i] = tot;

            free(bmap);                         // Free uncompressed blockmap
        }
    }

    skipblstart = true;
}

//
// P_LoadBlockMap
//
// killough 03/01/98: substantially modified to work
// towards removing blockmap limit (a wad limitation)
//
// killough 03/30/98: Rewritten to remove blockmap limit,
// though current algorithm is brute-force and non-optimal.
//
static void P_LoadBlockMap(int lump)
{
    int count;
    int lumplen = 1;

    blockmaprebuilt = false;

    if (lump >= numlumps || (lumplen = W_LumpLength(lump)) < 8 || (count = lumplen / 2) >= 0x10000)
    {
        P_CreateBlockMap();
        C_Warning(2, "The <b>BLOCKMAP</b> lump was rebuilt.");
    }
    else if (M_CheckParm("-blockmap"))
    {
        P_CreateBlockMap();
        C_Warning(2, "A <b>-blockmap</b> parameter was found on the command-line. The <b>BLOCKMAP</b> lump was rebuilt.");
    }
    else
    {
        short   *wadblockmaplump = W_CacheLumpNum(lump);

        blockmaplump = malloc_IfSameLevel(blockmaplump, sizeof(*blockmaplump) * count);

        // killough 03/01/98: Expand wad blockmap into larger internal one,
        // by treating all offsets except -1 as unsigned and zero-extending
        // them. This potentially doubles the size of blockmaps allowed,
        // because DOOM originally considered the offsets as always signed.
        blockmaplump[0] = SHORT(wadblockmaplump[0]);
        blockmaplump[1] = SHORT(wadblockmaplump[1]);
        blockmaplump[2] = (unsigned int)(SHORT(wadblockmaplump[2])) & 0xFFFF;
        blockmaplump[3] = (unsigned int)(SHORT(wadblockmaplump[3])) & 0xFFFF;

        // Swap all short integers to native byte ordering.
        for (int i = 4; i < count; i++)
        {
            short   t = SHORT(wadblockmaplump[i]);

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
            C_Warning(2, "The <b>BLOCKMAP</b> lump was rebuilt.");
        }
    }

    // Clear out mobj chains
    blocklinks = calloc_IfSameLevel(blocklinks, (size_t)bmapwidth * bmapheight, sizeof(*blocklinks));
    blockmap = blockmaplump + 4;

    // MAES: set blockmapxneg and blockmapyneg
    // E.g. for a full 512x512 map, they should be both
    // -1. For a 257*257, they should be both -255 etc.
    blockmapxneg = (bmapwidth > 255 ? bmapwidth - 512 : -257);
    blockmapyneg = (bmapheight > 255 ? bmapheight - 512 : -257);
}

//
// reject overrun emulation
//
static void RejectOverrun(int lump, const byte **matrix)
{
    unsigned int    required = (numsectors * numsectors + 7) / 8;
    unsigned int    length = W_LumpLength(lump);

    if (length < required)
    {
        // allocate a new block and copy the reject table into it; zero the rest
        // PU_LEVEL => will be freed on level exit
        byte    *newreject = Z_Malloc(required, PU_LEVEL, NULL);

        *matrix = memmove(newreject, *matrix, length);
        memset(newreject + length, 0, required - length);

        // unlock the original lump, it is no longer needed
        W_ReleaseLumpNum(lump);
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
// cph 18/8/99: rewritten to avoid O(numlines * numsectors) section
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
        {
            char    *temp = commify(i);

            I_Error("Subsector %s is not a part of any sector.", temp);
            free(temp);
        }
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
    {
        line_t  **linebuffer = Z_Malloc(total * sizeof(line_t *), PU_LEVEL, NULL);

        for (i = 0, sector = sectors; i < numsectors; i++, sector++)
        {
            sector->lines = linebuffer;
            linebuffer += sector->linecount;
            sector->linecount = 0;
            M_ClearBox(sector->blockbox);
        }
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
        fixed_t *bbox = (void *)sector->blockbox;

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
// A wad file does not allow fractional coordinates, so node builders are out
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
// (x0,y0) is the vertex being moved, and (x1,y1)-(x1+dx,y1+dy) is the
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
                        int64_t dx2 = (int64_t)(l->dx >> FRACBITS) * (l->dx >> FRACBITS);
                        int64_t dy2 = (int64_t)(l->dy >> FRACBITS) * (l->dy >> FRACBITS);
                        int64_t dxy = (int64_t)(l->dx >> FRACBITS) * (l->dy >> FRACBITS);
                        int64_t s = dx2 + dy2;
                        int     x0 = v->x, y0 = v->y;
                        int     x1 = l->v1->x, y1 = l->v1->y;

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

// Precalc values for use later in long wall error fix in R_StoreWallRange()
static void P_CalcSegsLength(void)
{
    for (int i = 0; i < numsegs; i++)
    {
        seg_t   *li = segs + i;

        li->dx = (int64_t)li->v2->x - li->v1->x;
        li->dy = (int64_t)li->v2->y - li->v1->y;

        li->length = (int64_t)sqrt((double)li->dx * li->dx + (double)li->dy * li->dy) / 2;

        // [BH] recalculate angle used for rendering. Fixes <https://doomwiki.org/wiki/Bad_seg_angle>.
        li->angle = R_PointToAngleEx2(li->v1->x, li->v1->y, li->v2->x, li->v2->y);

        li->fakecontrast = (!li->dy ? -LIGHTBRIGHT : (!li->dx ? LIGHTBRIGHT : 0));

        li->dx /= 2;
        li->dy /= 2;
    }
}

char    mapnum[6];
char    maptitle[256];
char    mapnumandtitle[512];
char    automaptitle[512];

// Determine map name to use
void P_MapName(int ep, int map)
{
    dboolean    mapnumonly = false;
    char        *mapinfoname = trimwhitespace(P_GetMapName((ep - 1) * 10 + map));

    switch (gamemission)
    {
        case doom:
            M_snprintf(mapnum, sizeof(mapnum), "E%iM%i%s", ep, map, (((E1M4B || *speciallumpname) && ep == 1 && map == 4)
                || ((E1M8B || *speciallumpname) && ep == 1 && map == 8) ? "B" : ""));

            if (*mapinfoname)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_StringCopy(automaptitle, mapnum, sizeof(mapnumandtitle));
            }
            else
                M_StringCopy(maptitle, trimwhitespace(*mapnames[(ep - 1) * 9 + map - 1]), sizeof(maptitle));

            break;

        case doom2:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);

            if (*mapinfoname && !BTSX)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (W_CheckMultipleLumps(mapnum) > 1 && (!nerve || map > 9) && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_StringCopy(automaptitle, mapnum, sizeof(mapnumandtitle));
            }
            else
                M_StringCopy(maptitle, trimwhitespace(bfgedition && (!modifiedgame || nerve) ?
                    *mapnames2_bfg[map - 1] : *mapnames2[map - 1]), sizeof(maptitle));

            break;

        case pack_nerve:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);

            if (*mapinfoname)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else
                M_StringCopy(maptitle, trimwhitespace(*mapnamesn[map - 1]), sizeof(maptitle));

            break;

        case pack_plut:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);

            if (*mapinfoname)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_StringCopy(automaptitle, mapnum, sizeof(mapnumandtitle));
            }
            else
                M_StringCopy(maptitle, trimwhitespace(*mapnamesp[map - 1]), sizeof(maptitle));

            break;

        case pack_tnt:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);

            if (*mapinfoname)
                M_StringCopy(maptitle, mapinfoname, sizeof(maptitle));
            else if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_StringCopy(automaptitle, mapnum, sizeof(mapnumandtitle));
            }
            else
                M_StringCopy(maptitle, trimwhitespace(*mapnamest[map - 1]), sizeof(maptitle));

            break;

        default:
            break;
    }

    if (strlen(maptitle) >= 4)
    {
        if (toupper(maptitle[0]) == 'M' && toupper(maptitle[1]) == 'A' && toupper(maptitle[2]) == 'P'
            && isdigit((int)maptitle[3]) && isdigit((int)maptitle[4]))
        {
            maptitle[0] = 'M';
            maptitle[1] = 'A';
            maptitle[2] = 'P';
        }
        else if (toupper(maptitle[0]) == 'E' && isdigit((int)maptitle[1]) && toupper(maptitle[2]) == 'M' && isdigit((int)maptitle[3]))
        {
            maptitle[0] = 'E';
            maptitle[2] = 'M';
        }
    }

    if (!mapnumonly)
    {
        char    *pos = strchr(maptitle, ':');

        if (pos)
        {
            int     index = (int)(pos - maptitle) + 1;
            char    *temp;

            if (M_StringStartsWith(maptitle, "LEVEL"))
            {
                memmove(maptitle, maptitle + index, strlen(maptitle) - index + 1);

                if (maptitle[0] == ' ')
                    memmove(maptitle, maptitle + 1, strlen(maptitle));

                temp = titlecase(maptitle);
                M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s: %s", mapnum, temp);
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

            M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s%s%s", mapnum, (maptitle[0] ? ": " : ""), temp);
            free(temp);
        }
        else
            M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));

        M_StringCopy(automaptitle, mapnumandtitle, sizeof(automaptitle));
    }
}

static mapformat_t P_CheckMapFormat(int lumpnum)
{
    mapformat_t format = DOOMBSP;
    byte        *n = NULL;
    int         b;

    if ((b = lumpnum + ML_BLOCKMAP + 1) < numlumps && !strncasecmp(lumpinfo[b]->name, "BEHAVIOR", 8))
        I_Error("Hexen format maps are not supported.");

    if ((b = lumpnum + ML_NODES) < numlumps && (n = W_CacheLumpNum(b)) && W_LumpLength(b))
    {
        if (!memcmp(n, "xNd4\0\0\0\0", 8))
            format = DEEPBSP;
        else if (!memcmp(n, "XNOD", 4) && !W_LumpLength(lumpnum + ML_SEGS) && W_LumpLength(lumpnum + ML_NODES) >= 12)
            format = ZDBSPX;
        else if (!memcmp(n, "ZNOD", 4))
            I_Error("Compressed ZDBSP nodes are not supported.");
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
    char        *temp;

    boomcompatible = false;
    mbfcompatible = false;

    totalkills = 0;
    totalitems = 0;
    totalsecret = 0;
    totalpickups = 0;
    memset(monstercount, 0, sizeof(int) * NUMMOBJTYPES);
    barrelcount = 0;
    wminfo.partime = 0;
    viewplayer->killcount = 0;
    viewplayer->secretcount = 0;
    viewplayer->itemcount = 0;

    // Initial height of PointOfView
    // will be set by player think.
    viewplayer->viewz = 1;

    idclev = false;

    Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

    if (rejectlump != -1)
    {
        // cph - unlock the reject table
        W_ReleaseLumpNum(rejectlump);
        rejectlump = -1;
    }

    P_InitThinkers();

    // find map name
    if (*speciallumpname)
    {
        lumpnum = W_GetNumForName(speciallumpname);
        M_StringCopy(lumpname, speciallumpname, sizeof(lumpname));
        speciallumpname[0] = '\0';
    }
    else
    {
        if (gamemode == commercial)
            M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", map);
        else
            M_snprintf(lumpname, sizeof(lumpname), "E%iM%i", ep, map);

        lumpnum = (nerve && gamemission == doom2 ? W_GetLastNumForName(lumpname) : W_GetNumForName(lumpname));
    }

    if ((!consolestrings
        || (!M_StringStartsWith(console[consolestrings - 1].string, "map ")
            && !M_StringStartsWith(console[consolestrings - 1].string, "load ")
            && !M_StringStartsWith(console[consolestrings - 1].string, "newgame")
            && !M_StringStartsWith(console[consolestrings - 1].string, "idclev")
            && !M_StringCompare(console[consolestrings - 1].string, "restartmap")))
        && ((consolestrings == 1
            || (!M_StringStartsWith(console[consolestrings - 2].string, "map ")
                && !M_StringStartsWith(console[consolestrings - 2].string, "idclev")))))
        C_Input("map %s", lumpname);

    if (!(samelevel = (lumpnum == prevlumpnum)))
    {
        viewplayer->cheats &= ~CF_ALLMAP;
        viewplayer->cheats &= ~CF_ALLMAP_THINGS;
        viewplayer->deaths = 0;
        viewplayer->suicides = 0;
    }

    prevlumpnum = lumpnum;

    mapformat = P_CheckMapFormat(lumpnum);

    canmodify = ((W_CheckMultipleLumps(lumpname) == 1 || (sigil && gamemission == doom) || gamemission == pack_nerve
        || (nerve && gamemission == doom2)) && !FREEDOOM);

    C_AddConsoleDivider();

    temp = titlecase(maptitle);

    if (M_StringCompare(playername, playername_default))
        C_PlayerMessage("You have %s <b><i>%s</i></b>%s",
            (samelevel ? "reentered": "entered"), temp, (ispunctuation(temp[strlen(temp) - 1]) ? "" : "."));
    else
        C_PlayerMessage("%s has %s <b><i>%s</i></b>%s",
            playername, (samelevel ? "reentered" : "entered"), temp, (ispunctuation(temp[strlen(temp) - 1]) ? "" : "."));

    free(temp);

    leveltime = 0;
    animatedliquiddiff = 2 * FRACUNIT;
    animatedliquidxdir = M_BigRandomInt(-FRACUNIT / 12, FRACUNIT / 12);
    animatedliquidydir = M_BigRandomInt(-FRACUNIT / 12, FRACUNIT / 12);

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

    if (mapformat == ZDBSPX)
        P_LoadZNodes(lumpnum + ML_NODES);
    else if (mapformat == DEEPBSP)
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

    r_bloodsplats_total = 0;

    markpointnum = 0;
    markpointnum_max = 0;

    pathpointnum = 0;
    pathpointnum_max = 0;

    massacre = false;

    P_GetMapLiquids((ep - 1) * 10 + map);
    P_GetMapNoLiquids((ep - 1) * 10 + map);
    P_SetLiquids();

    P_LoadThings(lumpnum + ML_THINGS);

    P_InitCards();

    // set up world state
    P_SpawnSpecials();
    P_SetLifts();

    P_MapEnd();

    // preload graphics
    R_PrecacheLevel();

    S_Start();

    if (gamemode != shareware)
        S_ParseMusInfo(lumpname);
}

static int  liquidlumps;
static int  noliquidlumps;

static void P_InitMapInfo(void)
{
    int         mapmax = 1;
    int         mcmdvalue;
    mapinfo_t   *info;
    char        *temp;

    if (sigil || M_CheckParm("-nomapinfo"))
        return;

    if ((RMAPINFO = MAPINFO = W_CheckNumForName(RMAPINFO_SCRIPT_NAME)) < 0)
        if ((UMAPINFO = MAPINFO = W_CheckNumForName(UMAPINFO_SCRIPT_NAME)) < 0)
            if ((MAPINFO = W_CheckNumForName(MAPINFO_SCRIPT_NAME)) < 0)
                return;

    for (int i = 0; i < MAXMAPINFO; i++)
    {
        mapinfo[i].author[0] = '\0';
        mapinfo[i].endbunny = false;
        mapinfo[i].endcast = false;
        mapinfo[i].endgame = false;
        mapinfo[i].endpic = 0;
        mapinfo[i].enterpic = 0;
        mapinfo[i].exitpic = 0;
        mapinfo[i].cluster = 0;
        mapinfo[i].interbackdrop[0] = '\0';
        mapinfo[i].intermusic = 0;
        mapinfo[i].intertext[0] = '\0';
        mapinfo[i].intertextsecret[0] = '\0';

        for (int j = 0; j < NUMLIQUIDS; j++)
        {
            mapinfo[i].liquid[j] = -1;
            mapinfo[i].noliquid[j] = -1;
        }

        mapinfo[i].music = 0;
        mapinfo[i].musiccomposer[0] = '\0';
        mapinfo[i].musictitle[0] = '\0';
        mapinfo[i].name[0] = '\0';
        mapinfo[i].next = 0;
        mapinfo[i].nojump = false;
        mapinfo[i].nomouselook = false;
        mapinfo[i].par = 0;
        mapinfo[i].pistolstart = false;
        mapinfo[i].secretnext = 0;
        mapinfo[i].sky1texture = 0;
        mapinfo[i].sky1scrolldelta = 0;
        mapinfo[i].titlepatch = 0;
    }

    SC_Open(RMAPINFO >= 0 ? RMAPINFO_SCRIPT_NAME : (UMAPINFO >= 0 ? UMAPINFO_SCRIPT_NAME : MAPINFO_SCRIPT_NAME));

    while (SC_GetString())
    {
        int ep = -1;
        int map = -1;

        if (SC_Compare("MAP"))
        {
            SC_MustGetString();
            sscanf(sc_String, "%i", &map);

            if (map < 0 || map > 99)
            {
                char    *buffer = uppercase(sc_String);

                if (gamemode == commercial)
                {
                    ep = 1;
                    sscanf(buffer, "MAP0%1i", &map);

                    if (map == -1)
                        sscanf(buffer, "MAP%2i", &map);
                }
                else
                {
                    sscanf(buffer, "E%1iM%1i", &ep, &map);

                    if (ep != -1 && map != -1)
                        map += (ep - 1) * 10;
                }

                free(buffer);
            }

            if (map < 0 || map > 99)
            {
                if (M_StringEndsWith(lumpinfo[MAPINFO]->wadfile->path, "NERVE.WAD"))
                {
                    C_Warning(0, "The map markers in PWAD <b>%s</b> are invalid.", lumpinfo[MAPINFO]->wadfile->path);
                    nerve = false;
                    NewDef.prevMenu = &MainDef;
                    MAPINFO = -1;
                    break;
                }
                else
                {
                    C_Warning(0, "The <b>MAPINFO</b> lump contains an invalid map marker.");
                    continue;
                }
            }

            info = &mapinfo[map];

            if (SC_GetString() && !SC_Compare("LOOKUP"))
                M_StringCopy(info->name, sc_String, sizeof(info->name));

            // Process optional tokens
            while (SC_GetString())
            {
                if (SC_Compare("MAP") || SC_Compare("DEFAULTMAP") || SC_Compare("CLUSTERDEF"))
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

                        case MCMD_CLUSTER:
                            SC_MustGetNumber();
                            info->cluster = sc_Number;
                            break;

                        case MCMD_ENDBUNNY:
                            SC_MustGetString();

                            if (SC_Compare("true"))
                                info->endbunny = true;

                            break;

                        case MCMD_ENDCAST:
                            SC_MustGetString();

                            if (SC_Compare("true"))
                                info->endcast = true;

                            break;

                        case MCMD_ENDGAME:
                            SC_MustGetString();

                            if (SC_Compare("true"))
                                info->endgame = true;

                            break;

                        case MCMD_ENDPIC:
                            SC_MustGetString();
                            info->endpic = W_GetNumForName(sc_String);
                            break;

                        case MCMD_ENTERPIC:
                            SC_MustGetString();
                            info->enterpic = W_GetNumForName(sc_String);
                            break;

                        case MCMD_EXITPIC:
                            SC_MustGetString();
                            info->exitpic = W_GetNumForName(sc_String);
                            break;

                        case MCMD_EPISODE:
                        {
                            char    lumpname[9];
                            char    string[128];

                            SC_MustGetString();

                            if (SC_Compare("clear"))
                            {
                                M_AddEpisode(map, ep, "", "");
                                break;
                            }

                            M_StringCopy(lumpname, sc_String, sizeof(lumpname));
                            SC_MustGetString();
                            M_StringCopy(string, sc_String, sizeof(string));
                            SC_MustGetString(); // skip key

                            M_AddEpisode(map, ep, lumpname, string);
                            break;
                        }

                        case MCMD_INTERBACKDROP:
                            SC_MustGetString();
                            M_StringCopy(info->interbackdrop, sc_String, sizeof(info->interbackdrop));
                            break;

                        case MCMD_INTERMUSIC:
                            SC_MustGetString();
                            info->intermusic = W_CheckNumForName(sc_String);
                            break;

                        case MCMD_INTERTEXTSECRET:
                        {
                            char    buf[1024] = "";

                            while (SC_GetString())
                            {
                                if (SC_MatchString(mapcmdnames) >= 0 || SC_Compare("MAP"))
                                {
                                    SC_UnGet();
                                    break;
                                }

                                if (!buf[0])
                                    M_StringCopy(buf, sc_String, sizeof(buf));
                                else
                                {
                                    strcat(buf, "\n");
                                    strcat(buf, sc_String);
                                }
                            }

                            M_StringCopy(info->intertextsecret, buf, sizeof(info->intertextsecret));
                            break;
                        }

                        case MCMD_INTERTEXT:
                        {
                            char    buf[1024] = "";

                            while (SC_GetString())
                            {
                                if (SC_MatchString(mapcmdnames) >= 0 || SC_Compare("MAP"))
                                {
                                    SC_UnGet();
                                    break;
                                }

                                if (!buf[0])
                                    M_StringCopy(buf, sc_String, sizeof(buf));
                                else
                                {
                                    strcat(buf, "\n");
                                    strcat(buf, sc_String);
                                }
                            }

                            M_StringCopy(info->intertext, buf, sizeof(info->intertext));
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
                            info->music = W_CheckNumForName(sc_String);
                            break;

                        case MCMD_MUSICCOMPOSER:
                            SC_MustGetString();
                            M_StringCopy(info->musiccomposer, sc_String, sizeof(info->musiccomposer));
                            break;

                        case MCMD_MUSICTITLE:
                            SC_MustGetString();
                            M_StringCopy(info->musictitle, sc_String, sizeof(info->musictitle));
                            break;

                        case MCMD_NEXT:
                        {
                            int nextepisode = 1;
                            int nextmap = -1;

                            SC_MustGetString();

                            if (SC_Compare("ENDGAMEC"))
                            {
                                info->endcast = true;
                                break;
                            }

                            sscanf(sc_String, "%i", &nextmap);

                            if (nextmap < 0 || nextmap > 99)
                            {
                                char    *buffer = uppercase(sc_String);

                                if (gamemode == commercial)
                                {
                                    sscanf(buffer, "MAP0%1i", &nextmap);

                                    if (nextmap == -1)
                                        sscanf(buffer, "MAP%2i", &nextmap);
                                }
                                else
                                    sscanf(buffer, "E%1iM%1i", &nextepisode, &nextmap);

                                free(buffer);
                            }

                            info->next = (nextepisode - 1) * 10 + nextmap;
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
                            info->nomouselook = true;
                            break;

                        case MCMD_PAR:
                        case MCMD_PARTIME:
                            SC_MustGetNumber();
                            info->par = sc_Number;
                            break;

                        case MCMD_PISTOLSTART:
                            info->pistolstart = true;
                            break;

                        case MCMD_NEXTSECRET:
                        case MCMD_SECRETNEXT:
                        {
                            int nextepisode = 1;
                            int nextmap = -1;

                            SC_MustGetString();
                            sscanf(sc_String, "%i", &nextmap);

                            if (nextmap < 0 || nextmap > 99)
                            {
                                char    *buffer = uppercase(sc_String);

                                if (gamemode == commercial)
                                {
                                    sscanf(buffer, "MAP0%1i", &nextmap);

                                    if (nextmap == -1)
                                        sscanf(buffer, "MAP%2i", &nextmap);
                                }
                                else
                                    sscanf(buffer, "E%1iM%1i", &nextepisode, &nextmap);

                                free(buffer);
                            }

                            info->secretnext = (nextepisode - 1) * 10 + nextmap;
                            break;
                        }

                        case MCMD_SKY1:
                            SC_MustGetString();
                            info->sky1texture = R_TextureNumForName(sc_String);

                            if (SC_GetNumber())
                            {
                                info->sky1scrolldelta = sc_Number << 8;
                                SC_UnGet();
                            }

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
                    }
            }

            mapmax = MAX(map, mapmax);
        }
        else if (SC_Compare("NOJUMP"))
        {
            if (!autosigil)
                nojump = true;
        }
        else if (SC_Compare("NOMOUSELOOK") || SC_Compare("NOFREELOOK"))
            nomouselook = true;
    }

    SC_Close();
    mapcount = mapmax;

    temp = commify(sc_Line);
    C_Output("Parsed %s line%s in the <b>%sMAPINFO</b> lump in %s <b>%s</b>.",
        temp, (sc_Line > 1 ? "s" : ""), (RMAPINFO >= 0 ? "R" : (UMAPINFO >= 0 ? "U" : "")),
        (lumpinfo[MAPINFO]->wadfile->type == IWAD ? "IWAD" : "PWAD"), lumpinfo[MAPINFO]->wadfile->path);
    free(temp);

    if (nojump)
        C_Warning(1, "This PWAD has disabled use of the <b>+jump</b> action.");

    if (nomouselook)
        C_Warning(1, "This PWAD has disabled use of the <b>mouselook</b> CVAR and <b>+mouselook</b> action.");
}

char *P_GetMapAuthor(int map)
{
    return (MAPINFO >= 0 && mapinfo[map].author[0] ? mapinfo[map].author :
        (((E1M4B || *speciallumpname) && map == 4) || ((E1M8B || *speciallumpname) && map == 8) ? s_AUTHOR_ROMERO : ""));
}

char *P_GetInterBackrop(int map)
{
    return mapinfo[map].interbackdrop;
}

int P_GetInterMusic(int map)
{
    return mapinfo[map].intermusic;
}

char *P_GetInterText(int map)
{
    return mapinfo[map].intertext;
}

char *P_GetInterSecretText(int map)
{
    return mapinfo[map].intertextsecret;
}

dboolean P_GetMapEndBunny(int map)
{
    return mapinfo[map].endbunny;
}

dboolean P_GetMapEndCast(int map)
{
    return mapinfo[map].endcast;
}

dboolean P_GetMapEndGame(int map)
{
    return mapinfo[map].endgame;
}

int P_GetMapEndPic(int map)
{
    return mapinfo[map].endpic;
}

int P_GetMapEnterPic(int map)
{
    return mapinfo[map].enterpic;
}

void P_GetMapLiquids(int map)
{
    for (int i = 0; i < liquidlumps; i++)
        terraintypes[mapinfo[map].liquid[i]] = LIQUID;
}

int P_GetMapMusic(int map)
{
    return mapinfo[map].music;
}

char *P_GetMapMusicComposer(int map)
{
    return mapinfo[map].musiccomposer;
}

char *P_GetMapMusicTitle(int map)
{
    return mapinfo[map].musictitle;
}

char *P_GetMapName(int map)
{
    return (MAPINFO >= 0 && !sigil ? mapinfo[map].name : ((E1M4B || *speciallumpname) && map == 4 ? s_CAPTION_E1M4B :
        ((E1M8B || *speciallumpname) && map == 8 ? s_CAPTION_E1M8B : "")));
}

int P_GetMapNext(int map)
{
    return mapinfo[map].next;
}

dboolean P_GetMapNoJump(int map)
{
    return (MAPINFO >= 0 ? mapinfo[map].nojump : nojump);
}

void P_GetMapNoLiquids(int map)
{
    for (int i = 0; i < noliquidlumps; i++)
        terraintypes[mapinfo[map].liquid[i]] = SOLID;
}

dboolean P_GetMapNoMouselook(int map)
{
    return (MAPINFO >= 0 ? mapinfo[map].nomouselook : nomouselook);
}

int P_GetMapPar(int map)
{
    return mapinfo[map].par;
}

dboolean P_GetMapPistolStart(int map)
{
    return mapinfo[map].pistolstart;
}

int P_GetMapSecretNext(int map)
{
    return mapinfo[map].secretnext;
}

int P_GetMapSky1Texture(int map)
{
    return mapinfo[map].sky1texture;
}

int P_GetMapSky1ScrollDelta(int map)
{
    return mapinfo[map].sky1scrolldelta;
}

int P_GetMapTitlePatch(int map)
{
    return mapinfo[map].titlepatch;
}

//
// P_Init
//
void P_Init(void)
{
    P_InitSwitchList();
    P_InitPicAnims();
    P_InitMapInfo();
    R_InitSprites();
    P_CheckSpechits();
    P_CheckIntercepts();
}
