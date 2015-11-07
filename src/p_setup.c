/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (c) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (c) 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include <math.h>
#include <time.h>

#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_fix.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAPINFO_SCRIPT_NAME     "MAPINFO"

#define MCMD_AUTHOR             1
#define MCMD_MUSIC              2
#define MCMD_NEXT               3

typedef struct mapinfo_s mapinfo_t;

struct mapinfo_s
{
    char        author[255];
    int         musiclump;
    char        name[255];
    int         nextmap;
    int         nextepisode;
};

void P_SpawnMapThing(mapthing_t *mthing, int index);

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int             MapCount;

int             numvertexes;
int             sizevertexes;
vertex_t        *vertexes;

int             numsegs;
int             sizesegs;
seg_t           *segs;

int             numsectors;
int             sizesectors;
sector_t        *sectors;

int             numsubsectors;
int             sizesubsectors;
subsector_t     *subsectors;

int             numnodes;
int             sizenodes;
node_t          *nodes;

int             numlines;
int             sizelines;
line_t          *lines;

int             numsides;
int             sizesides;
side_t          *sides;

int             numthings;
int             sizethings;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int             bmapwidth;
int             bmapheight;

// for large maps, wad is 16bit
int             *blockmap;

// offsets in blockmap are from here
int             *blockmaplump;

// origin of block map
fixed_t         bmaporgx;
fixed_t         bmaporgy;

// for thing chains
mobj_t          **blocklinks;

// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
static int      rejectlump = -1;        // cph - store reject lump num if cached
const byte      *rejectmatrix;          // cph - const*

static mapinfo_t mapinfo[99];

static char *mapcmdnames[] =
{
    "AUTHOR",
    "MUSIC",
    "NEXT",
    NULL
};

static int mapcmdids[] =
{
    MCMD_AUTHOR,
    MCMD_MUSIC,
    MCMD_NEXT
};

dboolean        canmodify;
dboolean        transferredsky;

dboolean        r_fixmaperrors = r_fixmaperrors_default;

static int      current_episode = -1;
static int      current_map = -1;
static int      samelevel;

mapformat_t     mapformat;

dboolean        boomlinespecials;
dboolean        blockmaprecreated;

extern fixed_t  animatedliquiddiff;
extern fixed_t  animatedliquidxdir;
extern fixed_t  animatedliquidydir;
extern fixed_t  animatedliquidxoffs;
extern fixed_t  animatedliquidyoffs;

static fixed_t GetOffset(vertex_t *v1, vertex_t *v2)
{
    fixed_t     dx = (v1->x - v2->x) >> FRACBITS;
    fixed_t     dy = (v1->y - v2->y) >> FRACBITS;

    return ((fixed_t)(sqrt(dx * dx + dy * dy)) << FRACBITS);
}

// e6y: Smart malloc
// Used by P_SetupLevel() for smart data loading
// Do nothing if level is the same
static void *malloc_IfSameLevel(void *p, size_t size)
{
    if (!samelevel || !p)
        return malloc(size);
    return p;
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
void P_LoadVertexes(int lump)
{
    const mapvertex_t   *data;
    int                 i;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    sizevertexes = W_LumpLength(lump);
    numvertexes = sizevertexes / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = calloc_IfSameLevel(vertexes, numvertexes, sizeof(vertex_t));

    // Load data into cache.
    data = (const mapvertex_t *)W_CacheLumpNum(lump, PU_STATIC);

    if (!data || !numvertexes)
        I_Error("There are no vertices in this map.");

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i = 0; i < numvertexes; i++)
    {
        vertexes[i].x = SHORT(data[i].x) << FRACBITS;
        vertexes[i].y = SHORT(data[i].y) << FRACBITS;

        // Apply any map-specific fixes.
        if (canmodify && r_fixmaperrors)
        {
            int j = 0;

            while (vertexfix[j].mission != -1)
            {
                if (i == vertexfix[j].vertex
                    && gamemission == vertexfix[j].mission
                    && gameepisode == vertexfix[j].epsiode
                    && gamemap == vertexfix[j].map
                    && vertexes[i].x == SHORT(vertexfix[j].oldx) << FRACBITS
                    && vertexes[i].y == SHORT(vertexfix[j].oldy) << FRACBITS)
                {
                    vertexes[i].x = SHORT(vertexfix[j].newx) << FRACBITS;
                    vertexes[i].y = SHORT(vertexfix[j].newy) << FRACBITS;
                    break;
                }
                j++;
            }
        }
    }

    // Free buffer memory.
    W_ReleaseLumpNum(lump);
}

//
// P_LoadSegs
//
void P_LoadSegs(int lump)
{
    const mapseg_t      *data;
    int                 i;

    sizesegs = W_LumpLength(lump);
    numsegs = sizesegs / sizeof(mapseg_t);
    segs = calloc_IfSameLevel(segs, numsegs, sizeof(seg_t));
    data = (const mapseg_t *)W_CacheLumpNum(lump, PU_STATIC);

    if (!data || !numsegs)
        I_Error("There are no segs in this map.");

    boomlinespecials = false;

    for (i = 0; i < numsegs; i++)
    {
        seg_t           *li = segs + i;
        const mapseg_t  *ml = data + i;
        unsigned short  v1, v2;
        int             side, linedef;
        line_t          *ldef;

        v1 = (unsigned short)SHORT(ml->v1);
        v2 = (unsigned short)SHORT(ml->v2);
        li->angle = SHORT(ml->angle) << 16;
        linedef = (unsigned short)SHORT(ml->linedef);

        if (linedef < 0 || linedef >= numlines)
            I_Error("Seg %s references an invalid linedef of %s.", commify(i), commify(linedef));

        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            C_Warning("Seg %s has a wrong side index of %s. It has been replaced with 1.",
                commify(i), commify(side));
            side = 1;
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
            I_Error("Linedef %s for seg %s references an invalid sidedef of %s.",
                commify(linedef), commify(i), commify(ldef->sidenum[side]));

        li->sidedef = &sides[ldef->sidenum[side]];

        // cph 2006/09/30 - our frontsector can be the second side of the
        // linedef, so must check for NO_INDEX in case we are incorrectly
        // referencing the back of a 1S line
        if (ldef->sidenum[side] != NO_INDEX)
            li->frontsector = sides[ldef->sidenum[side]].sector;
        else
        {
            C_Warning("The front of seg %s has no sidedef.", commify(i));
            li->frontsector = NULL;
        }

        // killough 5/3/98: ignore 2s flag if second sidedef missing:
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
        // http://www.doomworld.com/idgames/index.php?id=12647
        if (v1 >= numvertexes || v2 >= numvertexes)
        {
            char buffer[] = "Seg %s references an invalid vertex of %s.";

            if (v1 >= numvertexes)
                C_Warning(buffer, commify(i), commify(v1));
            if (v2 >= numvertexes)
                C_Warning(buffer, commify(i), commify(v2));

            if (li->sidedef == &sides[li->linedef->sidenum[0]])
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

        li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

        if (li->linedef->special >= BOOMLINESPECIALS)
            boomlinespecials = true;

        // Apply any map-specific fixes.
        if (canmodify && r_fixmaperrors)
        {
            int j = 0;

            while (linefix[j].mission != -1)
            {
                if (linedef == linefix[j].linedef
                    && gamemission == linefix[j].mission
                    && gameepisode == linefix[j].epsiode
                    && gamemap == linefix[j].map
                    && side == linefix[j].side)
                {
                    if (linefix[j].toptexture[0] != '\0')
                        li->sidedef->toptexture = R_TextureNumForName(linefix[j].toptexture);
                    if (linefix[j].middletexture[0] != '\0')
                        li->sidedef->midtexture = R_TextureNumForName(linefix[j].middletexture);
                    if (linefix[j].bottomtexture[0] != '\0')
                        li->sidedef->bottomtexture = R_TextureNumForName(linefix[j].bottomtexture);
                    if (linefix[j].offset != DEFAULT)
                    {
                        li->offset = SHORT(linefix[j].offset) << FRACBITS;
                        li->sidedef->textureoffset = 0;
                    }
                    if (linefix[j].rowoffset != DEFAULT)
                        li->sidedef->rowoffset = SHORT(linefix[j].rowoffset) << FRACBITS;
                    if (linefix[j].flags & ML_DONTDRAW)
                        li->linedef->hidden = true;
                    if (linefix[j].flags != DEFAULT)
                    {
                        if (li->linedef->flags & linefix[j].flags)
                            li->linedef->flags &= ~linefix[j].flags;
                        else
                            li->linedef->flags |= linefix[j].flags;
                    }
                    if (linefix[j].special != DEFAULT)
                        li->linedef->special = linefix[j].special;
                    if (linefix[j].tag != DEFAULT)
                        li->linedef->tag = linefix[j].tag;
                    break;
                }
                j++;
            }
        }
    }

    W_ReleaseLumpNum(lump);
}

static void P_LoadSegs_V4(int lump)
{
    const mapseg_v4_t   *data;
    int                 i;

    sizesegs = W_LumpLength(lump);
    numsegs = sizesegs / sizeof(mapseg_v4_t);
    segs = calloc_IfSameLevel(segs, numsegs, sizeof(seg_t));
    data = (const mapseg_v4_t *)W_CacheLumpNum(lump, PU_STATIC);

    if (!data || !numsegs)
        I_Error("This map has no segs.");

    boomlinespecials = false;

    for (i = 0; i < numsegs; i++)
    {
        seg_t                   *li = segs + i;
        const mapseg_v4_t       *ml = data + i;
        int                     v1, v2;
        int                     side, linedef;
        line_t                  *ldef;

        v1 = ml->v1;
        v2 = ml->v2;

        li->angle = SHORT(ml->angle) << FRACBITS;
        li->offset = SHORT(ml->offset) << FRACBITS;
        linedef = (unsigned short)SHORT(ml->linedef);

        //e6y: check for wrong indexes
        if (linedef < 0 || linedef >= numlines)
            I_Error("Seg %s references an invalid linedef of %s.", commify(i), commify(linedef));

        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            C_Warning("Seg %s has a wrong side index of %s. It has been replaced with 1.",
                commify(i), commify(side));
            side = 1;
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
            I_Error("Linedef %s for seg %s references an invalid sidedef of %s.",
                commify(linedef), commify(i), commify(ldef->sidenum[side]));

        li->sidedef = &sides[ldef->sidenum[side]];

        // cph 2006/09/30 - our frontsector can be the second side of the
        // linedef, so must check for NO_INDEX in case we are incorrectly
        // referencing the back of a 1S line
        if (ldef->sidenum[side] != NO_INDEX)
            li->frontsector = sides[ldef->sidenum[side]].sector;
        else
        {
            C_Warning("The front of seg %s has no sidedef.", commify(i));
            li->frontsector = NULL;
        }

        // killough 5/3/98: ignore 2s flag if second sidedef missing:
        if ((ldef->flags & ML_TWOSIDED) && ldef->sidenum[side ^ 1] != -1)
            li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
        else
        {
            li->backsector = NULL;
            ldef->flags &= ~ML_TWOSIDED;
        }

        // e6y
        // check and fix wrong references to non-existent vertexes
        // see e1m9 @ NIVELES.WAD
        // http://www.doomworld.com/idgames/index.php?id=12647
        if (v1 >= numvertexes || v2 >= numvertexes)
        {
            char buffer[] = "Seg %s references an invalid vertex of %s.";

            if (v1 >= numvertexes)
                C_Warning(buffer, commify(i), commify(v1));
            if (v2 >= numvertexes)
                C_Warning(buffer, commify(i), commify(v2));

            if (li->sidedef == &sides[li->linedef->sidenum[0]])
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

        li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

        if (li->linedef->special >= BOOMLINESPECIALS)
            boomlinespecials = true;
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadSubsectors
//
void P_LoadSubsectors(int lump)
{
    const mapsubsector_t        *data;
    int                         i;

    sizesubsectors = W_LumpLength(lump);
    numsubsectors = sizesubsectors / sizeof(mapsubsector_t);
    subsectors = calloc_IfSameLevel(subsectors, numsubsectors, sizeof(subsector_t));
    data = (const mapsubsector_t *)W_CacheLumpNum(lump, PU_STATIC);

    if (!data || !numsubsectors)
        I_Error("This map has no subsectors.");

    for (i = 0; i < numsubsectors; i++)
    {
        subsectors[i].numlines = (unsigned short)SHORT(data[i].numsegs);
        subsectors[i].firstline = (unsigned short)SHORT(data[i].firstseg);
    }

    W_ReleaseLumpNum(lump);
}

static void P_LoadSubsectors_V4(int lump)
{
    const mapsubsector_v4_t     *data;
    int                         i;

    sizesubsectors = W_LumpLength(lump);
    numsubsectors = sizesubsectors / sizeof(mapsubsector_v4_t);
    subsectors = calloc_IfSameLevel(subsectors, numsubsectors, sizeof(subsector_t));
    data = (const mapsubsector_v4_t *)W_CacheLumpNum(lump, PU_STATIC);

    if (!data || !numsubsectors)
        I_Error("This map has no subsectors.");

    for (i = 0; i < numsubsectors; i++)
    {
        subsectors[i].numlines = (int)data[i].numsegs;
        subsectors[i].firstline = (int)data[i].firstseg;
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadSectors
//
void P_LoadSectors(int lump)
{
    const byte  *data;
    int         i;

    sizesectors = W_LumpLength(lump);
    numsectors = sizesectors / sizeof(mapsector_t);
    sectors = calloc_IfSameLevel(sectors, numsectors, sizeof(sector_t));
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    for (i = 0; i < numsectors; i++)
    {
        sector_t    *ss = sectors + i;
        mapsector_t *ms = (mapsector_t *)data + i;

        ss->floorheight = SHORT(ms->floorheight) << FRACBITS;
        ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
        ss->floorpic = R_FlatNumForName(ms->floorpic);
        ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
        ss->lightlevel = SHORT(ms->lightlevel);
        ss->special = SHORT(ms->special);
        ss->tag = SHORT(ms->tag);

        ss->nextsec = -1;       // jff 2/26/98 add fields to support locking out
        ss->prevsec = -1;       // stair retriggering until build completes
        ss->heightsec = -1;     // sector used to get floor and ceiling height
        ss->floorlightsec = -1; // sector used to get floor lighting
        ss->ceilinglightsec = -1;

        // Apply any level-specific fixes.
        if (canmodify && r_fixmaperrors)
        {
            int j = 0;

            while (sectorfix[j].mission != -1)
            {
                if (i == sectorfix[j].sector
                    && gamemission == sectorfix[j].mission
                    && gameepisode == sectorfix[j].epsiode
                    && gamemap == sectorfix[j].map)
                {
                    if (sectorfix[j].floorpic[0] != '\0')
                        ss->floorpic = R_FlatNumForName(sectorfix[j].floorpic);
                    if (sectorfix[j].ceilingpic[0] != '\0')
                        ss->ceilingpic = R_FlatNumForName(sectorfix[j].ceilingpic);
                    if (sectorfix[j].floorheight != DEFAULT)
                        ss->floorheight = SHORT(sectorfix[j].floorheight) << FRACBITS;
                    if (sectorfix[j].ceilingheight != DEFAULT)
                        ss->ceilingheight = SHORT(sectorfix[j].ceilingheight) << FRACBITS;
                    if (sectorfix[j].special != DEFAULT)
                        ss->special = SHORT(sectorfix[j].special) << FRACBITS;
                    if (sectorfix[j].tag != DEFAULT)
                        ss->tag = SHORT(sectorfix[j].tag) << FRACBITS;
                    break;
                }
                j++;
            }
        }

        // [AM] Sector interpolation. Even if we're
        //      not running uncapped, the renderer still
        //      uses this data.
        ss->oldfloorheight = ss->floorheight;
        ss->interpfloorheight = ss->floorheight;
        ss->oldceilingheight = ss->ceilingheight;
        ss->interpceilingheight = ss->ceilingheight;
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadNodes
//
void P_LoadNodes(int lump)
{
    const byte  *data;
    int         i;

    sizenodes = W_LumpLength(lump);
    numnodes = sizenodes / sizeof(mapnode_t);
    nodes = malloc_IfSameLevel(nodes, numnodes * sizeof(node_t));
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    if (!data || !numnodes)
        if (numsubsectors == 1)
            C_Warning("This map has no nodes and only one subsector.");
        else
            I_Error("This map has no nodes.");

    for (i = 0; i < numnodes; i++)
    {
        node_t          *no = nodes + i;
        const mapnode_t *mn = (const mapnode_t *)data + i;
        int             j;

        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;

        for (j = 0; j < 2; j++)
        {
            int k;

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
                    C_Warning("Node %s references an invalid subsector of %s.",
                        commify(i), commify(no->children[j]));
                    no->children[j] = 0;
                }

                no->children[j] |= NF_SUBSECTOR;
            }

            for (k = 0; k < 4; k++)
                no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);
}

static void P_LoadNodes_V4(int lump)
{
    const byte  *data;
    int         i;

    sizenodes = W_LumpLength(lump);
    numnodes = (sizenodes - 8) / sizeof(mapnode_v4_t);
    nodes = malloc_IfSameLevel(nodes, numnodes * sizeof(node_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    // skip header
    data = data + 8;

    if (!data || !numnodes)
        if (numsubsectors == 1)
            C_Warning("This map has no nodes and only one subsector.");
        else
            I_Error("This map has no nodes.");

    for (i = 0; i < numnodes; i++)
    {
        node_t                  *no = nodes + i;
        const mapnode_v4_t      *mn = (const mapnode_v4_t *)data + i;
        int                     j;

        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;

        for (j = 0; j < 2; j++)
        {
            int k;
            no->children[j] = (unsigned int)(mn->children[j]);

            for (k = 0; k<4; k++)
                no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);
}

static void P_LoadZSegs(const byte *data)
{
    int i;

    boomlinespecials = false;

    for (i = 0; i < numsegs; i++)
    {
        line_t                  *ldef;
        unsigned int            v1, v2;
        unsigned int            linedef;
        unsigned char           side;
        seg_t                   *li = segs + i;
        const mapseg_znod_t     *ml = (const mapseg_znod_t *)data + i;

        v1 = ml->v1;
        v2 = ml->v2;

        linedef = (unsigned short)SHORT(ml->linedef);

        // e6y: check for wrong indexes
        if (linedef >= (unsigned int)numlines)
            I_Error("Seg %s references an invalid linedef of %s.", commify(i), commify(linedef));

        ldef = &lines[linedef];
        li->linedef = ldef;
        side = ml->side;

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
        {
            C_Warning("Seg %s has a wrong side index of %s. It has been replaced with 1.",
                commify(i), commify(side));
            side = 1;
        }

        // e6y: check for wrong indexes
        if ((unsigned int)ldef->sidenum[side] >= (unsigned int)numsides)
            C_Warning("Linedef %s for seg %s references an invalid sidedef of %s.",
                commify(linedef), commify(i), commify(ldef->sidenum[side]));

        li->sidedef = &sides[ldef->sidenum[side]];

        // cph 2006/09/30 - our frontsector can be the second side of the
        // linedef, so must check for NO_INDEX in case we are incorrectly
        // referencing the back of a 1S line
        if (ldef->sidenum[side] != NO_INDEX)
            li->frontsector = sides[ldef->sidenum[side]].sector;
        else
        {
            C_Warning("The front of seg %s has no sidedef.", commify(i));
            li->frontsector = NULL;
        }

        if ((ldef->flags & ML_TWOSIDED) && (ldef->sidenum[side ^ 1] != NO_INDEX))
            li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
        else
        {
            li->backsector = NULL;
            ldef->flags &= ~ML_TWOSIDED;
        }

        li->v1 = &vertexes[v1];
        li->v2 = &vertexes[v2];

        li->offset = GetOffset(li->v1, (side ? ldef->v2 : ldef->v1));
        li->angle = R_PointToAngle2(segs[i].v1->x, segs[i].v1->y, segs[i].v2->x, segs[i].v2->y);

        if (li->linedef->special >= BOOMLINESPECIALS)
            boomlinespecials = true;
    }
}

static void P_LoadZNodes(int lump)
{
    byte                *data = W_CacheLumpNum(lump, PU_STATIC);
    unsigned int        i;
    unsigned int        orgVerts, newVerts;
    unsigned int        numSubs, currSeg;
    unsigned int        numSegs;
    unsigned int        numNodes;
    vertex_t            *newvertarray = NULL;

    // skip header
    data += 4;

    // Read extra vertices added during node building
    orgVerts = *((const unsigned int *)data);
    data += sizeof(orgVerts);

    newVerts = *((const unsigned int *)data);
    data += sizeof(newVerts);

    if (!samelevel)
    {
        if (orgVerts + newVerts == (unsigned int)numvertexes)
            newvertarray = vertexes;
        else
        {
            newvertarray = calloc(orgVerts + newVerts, sizeof(vertex_t));
            memcpy(newvertarray, vertexes, orgVerts * sizeof(vertex_t));
        }

        for (i = 0; i < newVerts; i++)
        {
            newvertarray[i + orgVerts].x = *((const unsigned int *)data);
            data += sizeof(newvertarray[0].x);

            newvertarray[i + orgVerts].y = *((const unsigned int *)data);
            data += sizeof(newvertarray[0].y);
        }

        if (vertexes != newvertarray)
        {
            for (i = 0; i < (unsigned int)numlines; i++)
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
        int     size = newVerts * (sizeof(newvertarray[0].x) + sizeof(newvertarray[0].y));

        data += size;

        // P_LoadVertexes reset numvertexes, need to increase it again
        numvertexes = orgVerts + newVerts;
    }

    // Read the subsectors
    numSubs = *((const unsigned int*)data);
    data += sizeof(numSubs);

    numsubsectors = numSubs;
    if (numsubsectors <= 0)
        I_Error("There are no subsectors in this map.");

    subsectors = calloc_IfSameLevel(subsectors, numsubsectors, sizeof(subsector_t));

    for (i = currSeg = 0; i < numSubs; i++)
    {
        const mapsubsector_znod_t       *mseg = (const mapsubsector_znod_t *)data + i;

        subsectors[i].firstline = currSeg;
        subsectors[i].numlines = mseg->numsegs;
        currSeg += mseg->numsegs;
    }
    data += numSubs * sizeof(mapsubsector_znod_t);

    // Read the segs
    numSegs = *((const unsigned int*)data);
    data += sizeof(numSegs);

    // The number of segs stored should match the number of
    // segs used by subsectors.
    if (numSegs != currSeg)
        I_Error("There are an incorrect number of segs in the nodes.");

    numsegs = numSegs;
    segs = calloc_IfSameLevel(segs, numsegs, sizeof(seg_t));

    P_LoadZSegs(data);
    data += numsegs * sizeof(mapseg_znod_t);

    // Read nodes
    numNodes = *((const unsigned int*)data);
    data += sizeof(numNodes);

    numnodes = numNodes;
    nodes = calloc_IfSameLevel(nodes, numNodes, sizeof(node_t));

    for (i = 0; i < numNodes; i++)
    {
        int                     j;
        node_t                  *no = nodes + i;
        const mapnode_znod_t    *mn = (const mapnode_znod_t *)data + i;

        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;

        for (j = 0; j < 2; ++j)
        {
            int k;

            no->children[j] = (unsigned int)(mn->children[j]);

            for (k = 0; k < 4; ++k)
                no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadThings
//
void P_LoadThings(int lump)
{
    const mapthing_t    *data = (const mapthing_t *)W_CacheLumpNum(lump, PU_STATIC);
    int                 i;

    sizethings = W_LumpLength(lump);
    numthings = sizethings / sizeof(mapthing_t);

    srand(numthings);

    for (i = 0; i < numthings; i++)
    {
        mapthing_t      mt = data[i];
        dboolean        spawn = true;

        // Do not spawn cool, new monsters if !commercial
        if (gamemode != commercial)
        {
            switch (SHORT(mt.type))
            {
                case Arachnotron:
                case ArchVile:
                case BossBrain:
                case MonstersSpawner:
                case HellKnight:
                case Mancubus:
                case PainElemental:
                case HeavyWeaponDude:
                case Revenant:
                case WolfensteinSS:
                    spawn = false;
                    break;
            }
        }
        if (!spawn)
            break;

        // Do spawn all other stuff.
        mt.x = SHORT(mt.x);
        mt.y = SHORT(mt.y);
        mt.angle = SHORT(mt.angle);
        mt.type = SHORT(mt.type);
        mt.options = SHORT(mt.options);

        // Apply any level-specific fixes.
        if (canmodify && r_fixmaperrors)
        {
            int j = 0;

            while (thingfix[j].mission != -1)
            {
                if (gamemission == thingfix[j].mission
                    && gameepisode == thingfix[j].epsiode
                    && gamemap == thingfix[j].map
                    && i == thingfix[j].thing
                    && mt.type == thingfix[j].type
                    && mt.x == SHORT(thingfix[j].oldx)
                    && mt.y == SHORT(thingfix[j].oldy))
                {
                    if (thingfix[j].newx == REMOVE && thingfix[j].newy == REMOVE)
                        spawn = false;
                    else
                    {
                        mt.x = SHORT(thingfix[j].newx);
                        mt.y = SHORT(thingfix[j].newy);
                    }
                    if (thingfix[j].angle != DEFAULT)
                        mt.angle = SHORT(thingfix[j].angle);
                    if (thingfix[j].options != DEFAULT)
                        mt.options = thingfix[j].options;
                    break;
                }
                j++;
            }
        }

        // Change each WolfensteinSS into Zombiemen in BFG Edition
        if (mt.type == WolfensteinSS && bfgedition)
            mt.type = Zombieman;

        if (spawn)
            P_SpawnMapThing(&mt, i);
    }

    srand((unsigned int)time(NULL));

    W_ReleaseLumpNum(lump);
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
// killough 4/4/98: split into two functions, to allow sidedef overloading
//
static void P_LoadLineDefs(int lump)
{
    const byte  *data = W_CacheLumpNum(lump, PU_STATIC);
    int         i;

    sizelines = W_LumpLength(lump);
    numlines = sizelines / sizeof(maplinedef_t);
    lines = calloc_IfSameLevel(lines, numlines, sizeof(line_t));

    for (i = 0; i < numlines; i++)
    {
        const maplinedef_t      *mld = (const maplinedef_t *)data + i;
        line_t                  *ld = lines + i;
        vertex_t                *v1, *v2;

        ld->flags = (unsigned short)SHORT(mld->flags);
        ld->hidden = false;

        ld->special = SHORT(mld->special);

        ld->tag = SHORT(mld->tag);
        v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)];
        v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)];
        ld->dx = v2->x - v1->x;
        ld->dy = v2->y - v1->y;

        ld->tranlump = -1;   // killough 4/11/98: no translucency by default

        ld->slopetype = !ld->dx ? ST_VERTICAL : !ld->dy ? ST_HORIZONTAL :
            FixedDiv(ld->dy, ld->dx) > 0 ? ST_POSITIVE : ST_NEGATIVE;

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

        // killough 4/4/98: support special sidedef interpretation below
        if (ld->sidenum[0] != NO_INDEX && ld->special)
            sides[*ld->sidenum].special = ld->special;
    }

    W_ReleaseLumpNum(lump);
}

// killough 4/4/98: delay using sidedefs until they are loaded
static void P_LoadLineDefs2(int lump)
{
    int         i = numlines;
    line_t      *ld = lines;

    transferredsky = false;

    for (; i--; ld++)
    {
        {
            // cph 2006/09/30 - fix sidedef errors right away
            int j;

            for (j = 0; j < 2; j++)
                if (ld->sidenum[j] != NO_INDEX && ld->sidenum[j] >= numsides)
                {
                    C_Warning("Linedef %s references an invalid sidedef of %s.",
                        commify(i), commify(ld->sidenum[j]));
                    ld->sidenum[j] = NO_INDEX;
                }

            // killough 11/98: fix common wad errors (missing sidedefs):
            if (ld->sidenum[0] == NO_INDEX)
            {
                ld->sidenum[0] = 0;  // Substitute dummy sidedef for missing right side
                C_Warning("Linedef %s is missing its first sidedef.", commify(i));
            }

            if (ld->sidenum[1] == NO_INDEX && (ld->flags & ML_TWOSIDED))
            {
                ld->flags &= ~ML_TWOSIDED;  // Clear 2s flag for missing left side
                C_Warning("Linedef %s has the two-sided flag set but has no second sidedef.",
                    commify(i));
            }
        }

        ld->frontsector = (ld->sidenum[0] != NO_INDEX ? sides[ld->sidenum[0]].sector : 0);
        ld->backsector = (ld->sidenum[1] != NO_INDEX ? sides[ld->sidenum[1]].sector : 0);

        // killough 4/11/98: handle special types
        switch (ld->special)
        {
            int lump;

            case Translucent_MiddleTexture:            // killough 4/11/98: translucent 2s textures
                lump = sides[*ld->sidenum].special;    // translucency from sidedef
                if (!ld->tag)                          // if tag==0,
                    ld->tranlump = lump;               // affect this linedef only
                else
                {
                    int j;

                    for (j = 0; j < numlines; j++)     // if tag!=0,
                        if (lines[j].tag == ld->tag)   // affect all matching linedefs
                            lines[j].tranlump = lump;
                }
                break;

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
// killough 4/4/98: split into two functions
static void P_LoadSideDefs(int lump)
{
    sizesides = W_LumpLength(lump);
    numsides = sizesides / sizeof(mapsidedef_t);
    sides = calloc_IfSameLevel(sides, numsides, sizeof(side_t));
}

// killough 4/4/98: delay using texture names until after linedefs are loaded, to allow overloading
static void P_LoadSideDefs2(int lump)
{
    const byte  *data = (byte *)W_CacheLumpNum(lump, PU_STATIC);
    int         i;

    for (i = 0; i < numsides; i++)
    {
        mapsidedef_t    *msd = (mapsidedef_t *)data + i;
        side_t          *sd = sides + i;
        sector_t        *sec;
        unsigned short  sector_num = SHORT(msd->sector);

        sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;

        // cph 2006/09/30 - catch out-of-range sector numbers; use sector 0 instead
        if (sector_num >= numsectors)
        {
            C_Warning("Sidedef %s references an invalid sector of %s.",
                commify(i), commify(sector_num));
            sector_num = 0;
        }
        sd->sector = sec = &sectors[sector_num];

        // killough 4/4/98: allow sidedef texture names to be overloaded
        switch (sd->special)
        {
            case CreateFakeCeilingAndFloor:
                // variable colormap via 242 linedef
                sd->bottomtexture =
                    (sec->bottommap = R_ColormapNumForName(msd->bottomtexture)) < 0 ?
                    sec->bottommap = 0, R_TextureNumForName(msd->bottomtexture) : 0;
                sd->midtexture =
                    (sec->midmap = R_ColormapNumForName(msd->midtexture)) < 0 ?
                    sec->midmap = 0, R_TextureNumForName(msd->midtexture) : 0;
                sd->toptexture =
                    (sec->topmap = R_ColormapNumForName(msd->toptexture)) < 0 ?
                    sec->topmap = 0, R_TextureNumForName(msd->toptexture) : 0;
                break;

            case Translucent_MiddleTexture:
                // killough 4/11/98: apply translucency to 2s normal texture
                sd->midtexture = strncasecmp("TRANMAP", msd->midtexture, 8) ?
                    (sd->special = W_CheckNumForName(msd->midtexture)) < 0 ||
                    W_LumpLength(sd->special) != 65536 ?
                    sd->special = 0, R_TextureNumForName(msd->midtexture) :
                    (sd->special++, 0) : (sd->special = 0);
                sd->toptexture = R_TextureNumForName(msd->toptexture);
                sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
                break;

            default:
                // normal cases
                sd->midtexture = R_TextureNumForName(msd->midtexture);
                sd->toptexture = R_TextureNumForName(msd->toptexture);
                sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
                break;
        }
    }

    W_ReleaseLumpNum(lump);
}

//
// killough 10/98:
//
// Rewritten to use faster algorithm.
//
// New procedure uses Bresenham-like algorithm on the linedefs, adding the
// linedef to each block visited from the beginning to the end of the linedef.
//
// The algorithm's complexity is on the order of nlines*total_linedef_length.
//
// Please note: This section of code is not interchangeable with TeamTNT's
// code which attempts to fix the same problem.
//
static void P_CreateBlockMap(void)
{
    int         i;
    fixed_t     minx = INT_MAX;
    fixed_t     miny = INT_MAX;
    fixed_t     maxx = INT_MIN;
    fixed_t     maxy = INT_MIN;
    vertex_t    *vertex;

    // First find limits of map
    vertex = vertexes;
    i = numvertexes;
    do
    {
        fixed_t j = vertex->x >> FRACBITS;

        if (j < minx)
            minx = j;
        if (j > maxx)
            maxx = j;
        j = vertex->y >> FRACBITS;
        if (j < miny)
            miny = j;
        if (j > maxy)
            maxy = j;
        ++vertex;
    } while (--i);

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
    //   Map the starting and ending vertices to blocks.
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
            int n, nalloc, *list;
        } bmap_t;

        unsigned int    tot = bmapwidth * bmapheight;           // size of blockmap
        bmap_t          *bmap = calloc(sizeof(*bmap), tot);     // array of blocklists

        if (!bmap)
            I_Error("Unable to create blockmap.");

        for (i = 0; i < numlines; i++)
        {
            // starting coordinates
            int x = (lines[i].v1->x >> FRACBITS) - minx;
            int y = (lines[i].v1->y >> FRACBITS) - miny;

            // x - y deltas
            int adx = lines[i].dx >> FRACBITS;
            int dx = (adx < 0 ? -1 : 1);
            int ady = lines[i].dy >> FRACBITS;
            int dy = (ady < 0 ? -1 : 1);

            // difference in preferring to move across y (>0) instead of x (<0)
            int diff = !adx ? 1 : !ady ? -1 :
                (((x >> MAPBTOFRAC) << MAPBTOFRAC)
                + (dx > 0 ? MAPBLOCKUNITS - 1 : 0) - x) * (ady = abs(ady)) * dx
                - (((y >> MAPBTOFRAC) << MAPBTOFRAC)
                + (dy > 0 ? MAPBLOCKUNITS - 1 : 0) - y) * (adx = abs(adx)) * dy;

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
                if (bp->n >= bp->nalloc && !(bp->list = Z_Realloc(bp->list,
                    (bp->nalloc = bp->nalloc ? bp->nalloc * 2 : 8) * sizeof(*bp->list))))
                    I_Error("Unable to create blockmap.");

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
        // at tot and tot+1.
        //
        // 4 words, unused if this routine is called, are reserved at the start.
        {
            int count = tot + 6;  // we need at least 1 word per block, plus reserved's

            for (i = 0; (unsigned int)i < tot; i++)
                if (bmap[i].n)
                    count += bmap[i].n + 2;     // 1 header word + 1 trailer word + blocklist

            // Allocate blockmap lump with computed count
            blockmaplump = malloc_IfSameLevel(blockmaplump, sizeof(*blockmaplump) * count);
        }

        // Now compress the blockmap.
        {
            int         ndx = tot += 4; // Advance index to start of linedef lists
            bmap_t      *bp = bmap;     // Start of uncompressed blockmap

            blockmaplump[ndx++] = 0;    // Store an empty blockmap list at start
            blockmaplump[ndx++] = -1;   // (Used for compression)

            for (i = 4; (unsigned int)i < tot; i++, bp++)
                if (bp->n)                                              // Non-empty blocklist
                {
                    blockmaplump[blockmaplump[i] = ndx++] = 0;          // Store index & header
                    do
                        blockmaplump[ndx++] = bp->list[--bp->n];        // Copy linedef list
                    while (bp->n);
                    blockmaplump[ndx++] = -1;                           // Store trailer
                    free(bp->list);                                     // Free linedef list
                }
                else
                    // Empty blocklist: point to reserved empty blocklist
                    blockmaplump[i] = tot;

            free(bmap);                 // Free uncompressed blockmap
        }
    }
}

//
// P_LoadBlockMap
//
// killough 3/1/98: substantially modified to work
// towards removing blockmap limit (a wad limitation)
//
// killough 3/30/98: Rewritten to remove blockmap limit,
// though current algorithm is brute-force and non-optimal.
//
void P_LoadBlockMap(int lump)
{
    int count;
    int lumplen;

    blockmaprecreated = false;
    if (lump >= numlumps || (lumplen = W_LumpLength(lump)) < 8 || (count = lumplen / 2) >= 0x10000)
    {
        P_CreateBlockMap();
        blockmaprecreated = true;
    }
    else
    {
        short   *wadblockmaplump = W_CacheLumpNum(lump, PU_LEVEL);
        int      i;

        blockmaplump = malloc_IfSameLevel(blockmaplump, sizeof(*blockmaplump) * count);

        // killough 3/1/98: Expand wad blockmap into larger internal one,
        // by treating all offsets except -1 as unsigned and zero-extending
        // them. This potentially doubles the size of blockmaps allowed,
        // because DOOM originally considered the offsets as always signed.
        blockmaplump[0] = SHORT(wadblockmaplump[0]);
        blockmaplump[1] = SHORT(wadblockmaplump[1]);
        blockmaplump[2] = (uint32_t)(SHORT(wadblockmaplump[2])) & 0xFFFF;
        blockmaplump[3] = (uint32_t)(SHORT(wadblockmaplump[3])) & 0xFFFF;

        // Swap all short integers to native byte ordering.
        for (i = 4; i < count; i++)
        {
            short   t = SHORT(wadblockmaplump[i]);

            blockmaplump[i] = (t == -1 ? -1l : ((uint32_t)t & 0xFFFF));
        }

        Z_Free(wadblockmaplump);

        // Read the header
        bmaporgx = blockmaplump[0] << FRACBITS;
        bmaporgy = blockmaplump[1] << FRACBITS;
        bmapwidth = blockmaplump[2];
        bmapheight = blockmaplump[3];
    }

    // Clear out mobj chains
    blocklinks = calloc_IfSameLevel(blocklinks, bmapwidth * bmapheight, sizeof(*blocklinks));
    blockmap = blockmaplump + 4;
}

//
// reject overrun emulation
//
void RejectOverrun(int rejectlump, const byte **rejectmatrix, int totallines)
{
    unsigned int        required = (numsectors * numsectors + 7) / 8;
    unsigned int        length = W_LumpLength(rejectlump);

    if (length < required)
    {
        // allocate a new block and copy the reject table into it; zero the rest
        // PU_LEVEL => will be freed on level exit
        byte    *newreject = Z_Malloc(required, PU_LEVEL, NULL);

        *rejectmatrix = memmove(newreject, *rejectmatrix, length);

        memset(newreject + length, 0, required - length);

        // unlock the original lump, it is no longer needed
        W_ReleaseLumpNum(rejectlump);
    }
}
//
// P_LoadReject - load the reject table
//
static void P_LoadReject(int lumpnum, int totallines)
{
    // dump any old cached reject lump, then cache the new one
    if (rejectlump != -1)
        W_ReleaseLumpNum(rejectlump);
    rejectlump = lumpnum + ML_REJECT;
    rejectmatrix = W_CacheLumpNum(rejectlump, PU_STATIC);

    //e6y: check for overflow
    RejectOverrun(rejectlump, &rejectmatrix, totallines);
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
// killough 5/3/98: reformatted, cleaned up
// cph 18/8/99: rewritten to avoid O(numlines * numsectors) section
// It makes things more complicated, but saves seconds on big levels

// cph - convenient sub-function
static void P_AddLineToSector(line_t *li, sector_t *sector)
{
    fixed_t     *bbox = (void *)sector->blockbox;

    sector->lines[sector->linecount++] = li;
    M_AddToBox(bbox, li->v1->x, li->v1->y);
    M_AddToBox(bbox, li->v2->x, li->v2->y);
}

// modified to return totallines (needed by P_LoadReject)
static int P_GroupLines(void)
{
    line_t      *li;
    sector_t    *sector;
    int         i, j, total = numlines;

    // figgi
    for (i = 0; i < numsubsectors; i++)
    {
        seg_t   *seg = &segs[subsectors[i].firstline];

        subsectors[i].sector = NULL;
        for (j = 0; j < subsectors[i].numlines; j++)
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
    {
        line_t  **linebuffer = Z_Malloc(total * sizeof(line_t *), PU_LEVEL, 0);

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
        fixed_t *bbox = (void*)sector->blockbox; // cph - For convenience, so
        int     block;                           // I can use the old code unchanged

        //e6y: fix sound origin for large levels
        sector->soundorg.x = bbox[BOXRIGHT] / 2 + bbox[BOXLEFT] / 2;
        sector->soundorg.y = bbox[BOXTOP] / 2 + bbox[BOXBOTTOM] / 2;

        // adjust bounding box to map blocks
        block = (bbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
        block = (block >= bmapheight ? bmapheight - 1 : block);
        sector->blockbox[BOXTOP] = block;

        block = (bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
        block = (block < 0 ? 0 : block);
        sector->blockbox[BOXBOTTOM] = block;

        block = (bbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
        block = (block >= bmapwidth ? bmapwidth - 1 : block);
        sector->blockbox[BOXRIGHT] = block;

        block = (bbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
        block = (block < 0 ? 0 : block);
        sector->blockbox[BOXLEFT] = block;
    }

    return total;       // this value is needed by the reject overrun emulation code
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
    byte        *hit = calloc(1, numvertexes);          // Hitlist for vertices
    int         i;

    for (i = 0; i < numsegs; i++)                       // Go through each seg
    {
        const line_t    *l = segs[i].linedef;              // The parent linedef

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
                        int64_t dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
                        int64_t dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
                        int64_t dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
                        int64_t s = dx2 + dy2;
                        int     x0 = v->x, y0 = v->y, x1 = l->v1->x, y1 = l->v1->y;

                        v->x = (fixed_t)((dx2 * x0 + dy2 * x1 + dxy * (y0 - y1)) / s);
                        v->y = (fixed_t)((dy2 * y0 + dx2 * y1 + dxy * (x0 - x1)) / s);

                        // [crispy] wait a minute... moved more than 8 map units?
                        // maybe that's a linguortal then, back to the original coordinates
                        if (ABS(v->x - x0) > 8 * FRACUNIT || ABS(v->y - y0) > 8 * FRACUNIT)
                        {
                            v->x = x0;
                            v->y = y0;
                        }
                    }
                }  // Obfuscated C contest entry:   :)
            }
            while (v != segs[i].v2 && (v = segs[i].v2));
        }
    }
    free(hit);
}

// Precalc values for use later in long wall error fix in R_StoreWallRange()
static void P_CalcSegsLength(void)
{
    int i;

    for (i = 0; i < numsegs; i++)
    {
        seg_t   *li = segs + i;
        fixed_t dx = li->v2->x - li->v1->x;
        fixed_t dy = li->v2->y - li->v1->y;

        li->length = (fixed_t)sqrt((double)dx * dx + (double)dy * dy);

        // [crispy] re-calculate angle used for rendering
        li->angle = R_PointToAngle2(li->v1->x, li->v1->y, li->v2->x, li->v2->y);
    }
}

char            mapnum[6];
char            maptitle[256];
char            mapnumandtitle[512];
char            automaptitle[512];

extern char     **mapnames[];
extern char     **mapnames2[];
extern char     **mapnames2_bfg[];
extern char     **mapnamesp[];
extern char     **mapnamest[];
extern char     **mapnamesn[];

extern int      dehcount;

// Determine map name to use
void P_MapName(int ep, int map)
{
    dboolean    mapnumonly = false;

    switch (gamemission)
    {
        case doom:
            M_snprintf(mapnum, sizeof(mapnum), "E%iM%i", ep, map);
            if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1 && !chex)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_snprintf(automaptitle, sizeof(automaptitle), "%s: %s",
                    leafname(lumpinfo[W_GetNumForName(mapnum)]->wad_file->path), mapnum);
            }
            else
                M_StringCopy(maptitle, *mapnames[(ep - 1) * 9 + map - 1], sizeof(maptitle));
            break;

        case doom2:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);
            if (W_CheckMultipleLumps(mapnum) > 1 && (!nerve || map > 9) && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_snprintf(automaptitle, sizeof(automaptitle), "%s: %s",
                    leafname(lumpinfo[W_GetNumForName(mapnum)]->wad_file->path), mapnum);
            }
            else
                M_StringCopy(maptitle, (bfgedition ? *mapnames2_bfg[map - 1] :
                    *mapnames2[map - 1]), sizeof(maptitle));
            break;

        case pack_nerve:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);
            M_StringCopy(maptitle, *mapnamesn[map - 1], sizeof(maptitle));
            break;

        case pack_plut:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);
            if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_snprintf(automaptitle, sizeof(automaptitle), "%s: %s",
                    leafname(lumpinfo[W_GetNumForName(mapnum)]->wad_file->path), mapnum);
            }
            else
                M_StringCopy(maptitle, *mapnamesp[map - 1], sizeof(maptitle));
            break;

        case pack_tnt:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);
            if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                M_snprintf(automaptitle, sizeof(automaptitle), "%s: %s",
                    leafname(lumpinfo[W_GetNumForName(mapnum)]->wad_file->path), mapnum);
            }
            else
                M_StringCopy(maptitle, *mapnamest[map - 1], sizeof(maptitle));
            break;

        default:
            break;
    }

    if (!mapnumonly)
    {
        char    *pos;

        if ((pos = strchr(maptitle, ':')))
        {
            if (M_StringStartsWith(uppercase(maptitle), "LEVEL"))
            {
                strcpy(maptitle, pos + 1);
                if (maptitle[0] == ' ')
                    strcpy(maptitle, &maptitle[1]);
                M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s: %s", mapnum,
                    titlecase(maptitle));
            }
            else
            {
                M_StringCopy(mapnumandtitle, titlecase(maptitle), sizeof(mapnumandtitle));
                strcpy(maptitle, pos + 1);
                if (maptitle[0] == ' ')
                    strcpy(maptitle, &maptitle[1]);
            }
        }
        else if (strcasecmp(mapnum, maptitle))
            M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s: %s", mapnum,
                titlecase(maptitle));
        else
            M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
        M_StringCopy(automaptitle, mapnumandtitle, sizeof(automaptitle));
    }
}

static mapformat_t P_CheckMapFormat(int lumpnum)
{
    mapformat_t format = DOOMBSP;
    byte        *nodes = NULL;
    int         b;

    if ((b = lumpnum + ML_NODES) < numlumps && (nodes = W_CacheLumpNum(b, PU_CACHE))
        && W_LumpLength(b) > 0)
    {
        if (!memcmp(nodes, "xNd4\0\0\0\0", 8))
            format = DEEPBSP;
        else if (!memcmp(nodes, "XNOD", 4))
                format = ZDBSPX;
        else if (!memcmp(nodes, "ZNOD", 4))
            I_Error("Compressed ZDoom nodes are not supported.");
    }

    if (nodes)
        W_ReleaseLumpNum(b);

    return format;
}

extern dboolean idclev;
extern dboolean oldweaponsowned[];

//
// P_SetupLevel
//
void P_SetupLevel(int ep, int map)
{
    char        lumpname[6];
    int         lumpnum;

    totalkills = totalitems = totalsecret = 0;
    wminfo.partime = 0;
    players[0].killcount = players[0].secretcount = players[0].itemcount = 0;

    // Initial height of PointOfView
    // will be set by player think.
    players[0].viewz = 1;

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
    if (gamemode == commercial)
        M_snprintf(lumpname, 6, "MAP%02i", map);
    else
        M_snprintf(lumpname, 5, "E%iM%i", ep, map);

    if (nerve && gamemission == doom2)
        lumpnum = W_GetNumForName2(lumpname);
    else
        lumpnum = W_GetNumForName(lumpname);

    mapformat = P_CheckMapFormat(lumpnum);

    canmodify = ((W_CheckMultipleLumps(lumpname) == 1 || gamemission == pack_nerve
        || (nerve && gamemission == doom2)) && !FREEDOOM);

    leveltime = 0;
    animatedliquiddiff = FRACUNIT;
    animatedliquidxdir = M_RandomInt(-1, 1) * FRACUNIT / 12;
    animatedliquidydir = M_RandomInt(-1, 1) * FRACUNIT / 12;
    if (!animatedliquidxdir && !animatedliquidydir)
    {
        animatedliquidxdir = FRACUNIT / 12;
        animatedliquidydir = FRACUNIT / 12;
    }
    animatedliquidxoffs = 0;
    animatedliquidyoffs = 0;

    // e6y: speedup of level reloading
    // Most of level's structures now are allocated with PU_STATIC instead of PU_LEVEL
    samelevel = (map == current_map && ep == current_episode);

    current_episode = ep;
    current_map = map;

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
    P_LoadLineDefs2(lumpnum + ML_LINEDEFS);

    if (!samelevel)
        P_LoadBlockMap(lumpnum + ML_BLOCKMAP);
    else
        memset(blocklinks, 0, bmapwidth * bmapheight * sizeof(*blocklinks));

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

    // reject loading and underflow padding separated out into new function
    // P_GroupLines modified to return a number the underflow padding needs
    P_LoadReject(lumpnum, P_GroupLines());

    P_RemoveSlimeTrails();

    P_CalcSegsLength();

    r_bloodsplats_total = 0;
    memset(bloodsplats, 0, sizeof(mobj_t *) * r_bloodsplats_max);

    P_LoadThings(lumpnum + ML_THINGS);

    P_InitCards(&players[0]);

    // set up world state
    P_SpawnSpecials();

    P_MapEnd();

    // preload graphics
    R_PrecacheLevel();

    S_Start();
}

static void InitMapInfo(void)
{
    int         episode;
    int         map;
    int         mapMax;
    int         mcmdValue;
    mapinfo_t   *info;

    mapMax = 1;

    // Put defaults into MapInfo[0]
    info = mapinfo;
    info->name[0] = '\0';
    info->author[0] = '\0';

    SC_Open(MAPINFO_SCRIPT_NAME);
    while (SC_GetString())
    {
        if (!SC_Compare("MAP"))
            SC_ScriptError(NULL);
        SC_GetNumber();
        if (sc_Number >= 1 && sc_Number <= 99)
            map = sc_Number;
        else
        {
            SC_MustGetString();
            if (gamemode == commercial)
            {
                episode = 1;
                sscanf(sc_String, "MAP0%1i", &map);
                if (!map)
                    sscanf(sc_String, "MAP%2i", &map);
            }
            else
                sscanf(sc_String, "E%1iM%1i", &episode, &map);
        }

        info = &mapinfo[map];

        // Copy defaults to current map definition
        memcpy(info, &mapinfo[0], sizeof(*info));

        // Map name must follow the number
        SC_MustGetString();
        M_StringCopy(info->name, sc_String, sizeof(info->name));

        // Process optional tokens
        while (SC_GetString())
        {
            int i;

            if (SC_Compare("MAP"))
            {
                SC_UnGet();
                break;
            }
            mcmdValue = mapcmdids[SC_MatchString(mapcmdnames)];
            switch (mcmdValue)
            {
                case MCMD_AUTHOR:
                    SC_MustGetString();
                    M_StringCopy(info->author, sc_String, sizeof(info->author));
                    break;

                case MCMD_MUSIC:
                    SC_MustGetString();
                    i = 1;
                    while (S_music[i].name[0])
                    {
                        if (strcasecmp(sc_String, S_music[i].name))
                        {
                            info->musiclump = i;
                            break;
                        }
                        ++i;
                    }
                    break;

                case MCMD_NEXT:
                    SC_GetNumber();
                    if (sc_Number >= 1 && sc_Number <= 99)
                        info->nextmap = sc_Number;
                    else
                    {
                        SC_MustGetString();
                        if (gamemode == commercial)
                        {
                            episode = 1;
                            sscanf(sc_String, "MAP0%1i", &info->nextmap);
                            if (!info->nextmap)
                                sscanf(sc_String, "MAP%2i", &info->nextmap);
                        }
                        else
                            sscanf(sc_String, "E%1iM%1i", &episode, &info->nextmap);
                    }
                    break;
            }
        }
        mapMax = (map > mapMax ? map : mapMax);
    }
    SC_Close();
    MapCount = mapMax;
}

static int QualifyMap(int map)
{
    return (map < 1 || map > MapCount ? 0 : map);
}

char *P_GetMapAuthor(int map)
{
    return mapinfo[QualifyMap(map)].author;
}

int P_GetMapMusicLump(int map)
{
    return mapinfo[QualifyMap(map)].musiclump;
}

int P_GetMapNextMap(int map)
{
    return mapinfo[QualifyMap(map)].nextmap;
}


//
// P_Init
//
void P_Init(void)
{
    InitMapInfo();
    P_InitSwitchList();
    P_InitPicAnims();
    R_InitSprites(sprnames);
}
