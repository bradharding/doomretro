/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include <math.h>

#include "z_zone.h"

#include "i_swap.h"
#include "m_argv.h"
#include "m_bbox.h"

#include "g_game.h"

#include "i_system.h"
#include "w_wad.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

#include "doomstat.h"

#include "p_fix.h"


void P_SpawnMapThing(mapthing_t *mthing);


//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int             numvertexes;
vertex_t        *vertexes;

int             numsegs;
seg_t           *segs;

int             numsectors;
sector_t        *sectors;

int             numsubsectors;
subsector_t     *subsectors;

int             numnodes;
node_t          *nodes;

int             numlines;
line_t          *lines;

int             numsides;
side_t          *sides;

static int      totallines;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int             bmapwidth;
int             bmapheight;     // size in mapblocks
short           *blockmap;      // int for larger maps
// offsets in blockmap are from here
short           *blockmaplump;
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
byte            *rejectmatrix;


// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS   10

mapthing_t      deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t      *deathmatch_p;
mapthing_t      playerstarts[MAXPLAYERS];


boolean         canmodify;

#define DEFAULT 0x7fff


//
// P_LoadVertexes
//
void P_LoadVertexes(int lump)
{
    byte        *data;
    int         i;

    // Determine number of lumps:
    // total lump length / vertex record length.
    numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = (vertex_t *)Z_Malloc(numvertexes * sizeof(vertex_t), PU_LEVEL, 0);

    // Load data into cache.
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i = 0; i < numvertexes; i++)
    {
        vertexes[i].x = SHORT(((mapvertex_t *)data)[i].x) << FRACBITS;
        vertexes[i].y = SHORT(((mapvertex_t *)data)[i].y) << FRACBITS;

        // Apply any level-specific fixes.
        if (canmodify)
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
    byte        *data;
    int         i;
    int         sidenum;

    numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
    segs = (seg_t *)Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
    memset(segs, 0, numsegs * sizeof(seg_t));
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    for (i = 0; i < numsegs; i++)
    {
        seg_t *li = segs + i;
        mapseg_t *ml = (mapseg_t *)data + i;

        int side, linedef;
        line_t *ldef;

        short v = SHORT(ml->v1);

        if (v < 0 || v >= numvertexes)
            I_Error("P_LoadSegs: invalid vertex %d", v);
        else
            li->v1 = &vertexes[v];

        v = SHORT(ml->v2);

        if (v < 0 || v >= numvertexes)
            I_Error("P_LoadSegs: invalid vertex %d", v);
        else
            li->v2 = &vertexes[v];

        li->angle = SHORT(ml->angle) << 16;
        linedef = SHORT(ml->linedef);

        if (linedef < 0 || linedef >= numlines)
            I_Error("P_LoadSegs: invalid linedef %d", linedef);

        ldef = &lines[linedef];
        li->linedef = ldef;

        side = SHORT(ml->side);

        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;

        if (ldef-> flags & ML_TWOSIDED)
        {
            sidenum = ldef->sidenum[side ^ 1];

            // If the sidenum is out of range, this may be a "glass hack"
            // impassible window.  Point at side #0 (this may not be
            // the correct Vanilla behavior; however, it seems to work for
            // OTTAWAU.WAD, which is the one place I've seen this trick
            // used).

            if (sidenum < 0 || sidenum >= numsides)
                sidenum = 0;

            li->backsector = sides[sidenum].sector;
        }
        else
        {
            li->backsector = 0;
        }

        // From Odamex:
        {
            // Recalculate seg offsets. Values in wads are untrustworthy.
            vertex_t *from = (side == 0)
                ? ldef->v1         // right side: offset is from start of linedef
                : ldef->v2;        // left side: offset is from end of linedef
            vertex_t *to = li->v1; // end point is start of seg, in both cases

            float dx = (float)(to->x - from->x);
            float dy = (float)(to->y - from->y);
            li->offset = (fixed_t)sqrt(dx * dx + dy * dy);
        }

        // Apply any level-specific fixes.
        if (canmodify)
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
                        li->offset = SHORT(linefix[j].offset)<<FRACBITS;
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


//
// P_LoadSubsectors
//
void P_LoadSubsectors(int lump)
{
    byte        *data;
    int         i;

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_t);
    subsectors = (subsector_t *)Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    memset(subsectors, 0, numsubsectors * sizeof(subsector_t));

    for (i = 0; i < numsubsectors; i++)
    {
        subsectors[i].numlines = SHORT(((mapsubsector_t *)data)[i].numsegs);
        subsectors[i].firstline = SHORT(((mapsubsector_t *)data)[i].firstseg);
    }

    W_ReleaseLumpNum(lump);
}



//
// P_LoadSectors
//
void P_LoadSectors(int lump)
{
    byte        *data;
    int         i;
    mapsector_t *ms;
    sector_t    *ss;

    numsectors = W_LumpLength(lump) / sizeof(mapsector_t);
    sectors = (sector_t *)Z_Malloc(numsectors * sizeof(sector_t), PU_LEVEL, 0);
    memset(sectors, 0, numsectors * sizeof(sector_t));
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    ms = (mapsector_t *)data;
    ss = sectors;
    for (i = 0; i < numsectors; i++, ss++, ms++)
    {
        ss->floorheight = SHORT(ms->floorheight) << FRACBITS;
        ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
        ss->floorpic = R_FlatNumForName(ms->floorpic);
        ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
        ss->lightlevel = SHORT(ms->lightlevel);
        ss->special = SHORT(ms->special);
        ss->tag = SHORT(ms->tag);
        ss->thinglist = NULL;

        // Apply any level-specific fixes.
        if (canmodify)
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
    }

    W_ReleaseLumpNum(lump);
}


//
// P_LoadNodes
//
void P_LoadNodes(int lump)
{
    byte        *data;
    int         i;
    int         j;
    int         k;
    mapnode_t   *mn;
    node_t      *no;

    numnodes = W_LumpLength(lump) / sizeof(mapnode_t);
    nodes = (node_t *)Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    mn = (mapnode_t *)data;
    no = nodes;

    for (i = 0; i < numnodes; i++, no++, mn++)
    {
        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;
        for (j = 0; j < 2; j++)
        {
            no->children[j] = SHORT(mn->children[j]);
            for (k = 0; k < 4; k++)
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
    byte        *data;
    int         i;
    mapthing_t  *mt;
    mapthing_t  spawnthing;
    int         numthings;
    boolean     spawn;

    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_t);

    mt = (mapthing_t *)data;
    for (i = 0; i < numthings; i++, mt++)
    {
        spawn = true;

        // Do not spawn cool, new monsters if !commercial
        if (gamemode != commercial)
        {
            switch (SHORT(mt->type))
            {
                case Arachnotron:
                case ArchVile:
                case BossBrain:
                case MonstersSpawner:
                case HellKnight:
                case Mancubus:
                case PainElemental:
                case Chaingunner:
                case Revenant:
                case WolfensteinSS:
                    spawn = false;
                    break;
            }
        }
        if (spawn == false)
            break;

        // Do spawn all other stuff.
        spawnthing.x = SHORT(mt->x);
        spawnthing.y = SHORT(mt->y);
        spawnthing.angle = SHORT(mt->angle);
        spawnthing.type = SHORT(mt->type);
        spawnthing.options = SHORT(mt->options);

        // Apply any level-specific fixes.
        if (canmodify)
        {
            int j = 0;

            while (thingfix[j].mission != -1)
            {
                if (gamemission == thingfix[j].mission
                    && gameepisode == thingfix[j].epsiode
                    && gamemap == thingfix[j].map
                    && i == thingfix[j].thing
                    && spawnthing.type == thingfix[j].type
                    && spawnthing.x == SHORT(thingfix[j].oldx)
                    && spawnthing.y == SHORT(thingfix[j].oldy))
                {
                    if (thingfix[j].newx == REMOVE && thingfix[j].newy == REMOVE)
                        spawn = false;
                    else
                    {
                        spawnthing.x = SHORT(thingfix[j].newx);
                        spawnthing.y = SHORT(thingfix[j].newy);
                    }
                    if (thingfix[j].angle != DEFAULT)
                        spawnthing.angle = SHORT(thingfix[j].angle);
                    if (thingfix[j].options != DEFAULT)
                        spawnthing.options = thingfix[j].options;
                    break;
                }
                j++;
            }
        }

        // Change each WolfensteinSS into Zombiemen in BFG Edition
        if (spawnthing.type == WolfensteinSS && bfgedition)
            spawnthing.type = FormerHuman;

        if (spawn)
            P_SpawnMapThing(&spawnthing);
    }

    W_ReleaseLumpNum(lump);
}


//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs(int lump)
{
    byte                *data;
    int                 i;
    maplinedef_t        *mld;
    line_t              *ld;
    vertex_t            *v1;
    vertex_t            *v2;

    numlines = W_LumpLength(lump) / sizeof(maplinedef_t);
    lines = (line_t *)Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
    memset(lines, 0, numlines * sizeof(line_t));
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    mld = (maplinedef_t *)data;
    ld = lines;
    for (i = 0; i < numlines; i++, mld++, ld++)
    {
        ld->flags = SHORT(mld->flags);
        ld->hidden = false;
        ld->special = SHORT(mld->special);
        ld->tag = SHORT(mld->tag);
        v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
        v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
        ld->dx = v2->x - v1->x;
        ld->dy = v2->y - v1->y;

        if (!ld->dx)
            ld->slopetype = ST_VERTICAL;
        else if (!ld->dy)
            ld->slopetype = ST_HORIZONTAL;
        else
        {
            if (FixedDiv(ld->dy , ld->dx) > 0)
                ld->slopetype = ST_POSITIVE;
            else
                ld->slopetype = ST_NEGATIVE;
        }

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
        ld->soundorg.x = (ld->bbox[BOXLEFT] + ld->bbox[BOXRIGHT] ) / 2;
        ld->soundorg.y = (ld->bbox[BOXTOP] + ld->bbox[BOXBOTTOM]) / 2;

        ld->sidenum[0] = SHORT(mld->sidenum[0]);
        ld->sidenum[1] = SHORT(mld->sidenum[1]);

        if (ld->sidenum[0] != -1)
            ld->frontsector = sides[ld->sidenum[0]].sector;
        else
            ld->frontsector = 0;

        if (ld->sidenum[1] != -1)
            ld->backsector = sides[ld->sidenum[1]].sector;
        else
            ld->backsector = 0;
    }

    W_ReleaseLumpNum(lump);
}


//
// P_LoadSideDefs
//
void P_LoadSideDefs(int lump)
{
    byte                *data;
    int                 i;
    mapsidedef_t        *msd;
    side_t              *sd;

    numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
    sides = (side_t *)Z_Malloc(numsides * sizeof(side_t), PU_LEVEL, 0);
    memset(sides, 0, numsides * sizeof(side_t));
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i = 0; i < numsides; i++, msd++, sd++)
    {
        sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;
        sd->toptexture = R_TextureNumForName(msd->toptexture);
        sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
        sd->midtexture = R_TextureNumForName(msd->midtexture);
        sd->sector = &sectors[SHORT(msd->sector)];
    }

    W_ReleaseLumpNum(lump);
}


//
// P_LoadBlockMap
//
void P_LoadBlockMap(int lump)
{
    int i;
    int count;
    int lumplen;

    lumplen = W_LumpLength(lump);
    count = lumplen / 2;

    blockmaplump = (short *)Z_Malloc(lumplen, PU_LEVEL, NULL);
    W_ReadLump(lump, blockmaplump);
    blockmap = blockmaplump + 4;

    // Swap all short integers to native byte ordering.
    for (i = 0; i < count; i++)
    {
        blockmaplump[i] = SHORT(blockmaplump[i]);
    }

    // Read the header
    bmaporgx = blockmaplump[0] << FRACBITS;
    bmaporgy = blockmaplump[1] << FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];

    // Clear out mobj chains
    count = sizeof(*blocklinks) * bmapwidth * bmapheight;
    blocklinks = (mobj_t **)Z_Malloc(count, PU_LEVEL, 0);
    memset(blocklinks, 0, count);
}


//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines(void)
{
    line_t      **linebuffer;
    int         i;
    int         j;
    line_t      *li;
    sector_t    *sector;
    subsector_t *ss;
    seg_t       *seg;
    fixed_t     bbox[4];
    int         block;

    // look up sector number for each subsector
    ss = subsectors;
    for (i = 0; i < numsubsectors; i++, ss++)
    {
        seg = &segs[ss->firstline];
        ss->sector = seg->sidedef->sector;
    }

    // count number of lines in each sector
    li = lines;
    totallines = 0;
    for (i = 0; i < numlines; i++, li++)
    {
        totallines++;

        if (!li->frontsector && li->backsector)
        {
            li->frontsector = li->backsector;
            li->backsector = NULL;
        }

        if (li->frontsector)
            li->frontsector->linecount++;

        if (li->backsector && li->backsector != li->frontsector)
        {
            li->backsector->linecount++;
            totallines++;
        }
    }

    // build line tables for each sector
    linebuffer = (line_t **)Z_Malloc(totallines * sizeof(line_t *), PU_LEVEL, 0);

    for (i = 0; i < numsectors; ++i)
    {
        // Assign the line buffer for this sector

        sectors[i].lines = linebuffer;
        linebuffer += sectors[i].linecount;

        // Reset linecount to zero so in the next stage we can count
        // lines into the list.

        sectors[i].linecount = 0;
    }

    // Assign lines to sectors

    for (i = 0; i < numlines; ++i)
    {
        li = &lines[i];

        if (li->frontsector != NULL)
        {
            sector = li->frontsector;

            sector->lines[sector->linecount] = li;
            ++sector->linecount;
        }

        if (li->backsector != NULL && li->frontsector != li->backsector)
        {
            sector = li->backsector;

            sector->lines[sector->linecount] = li;
            ++sector->linecount;
        }
    }

    // Generate bounding boxes for sectors
    sector = sectors;
    for (i = 0 ; i<numsectors ; i++, sector++)
    {
        M_ClearBox(bbox);

        for (j = 0; j < sector->linecount; j++)
        {
            li = sector->lines[j];

            M_AddToBox(bbox, li->v1->x, li->v1->y);
            M_AddToBox(bbox, li->v2->x, li->v2->y);
        }

        // set the degenmobj_t to the middle of the bounding box
        sector->soundorg.x = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
        sector->soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;

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
}

//
// killough 10/98
//
// Remove slime trails.
//
// Slime trails are inherent to Doom's coordinate system -- i.e. there is
// nothing that a node builder can do to prevent slime trails ALL of the time,
// because it's a product of the integer coodinate system, and just because
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
// Please note: This section of code is not interchangable with TeamTNT's
// code which attempts to fix the same problem.
//
// Firelines (TM) is a Rezistered Trademark of MBF Productions
//

static void P_RemoveSlimeTrails(void)           // killough 10/98
{
    byte *hit = (byte *)calloc(1, numvertexes); // Hitlist for vertices
    int i;

    for (i = 0; i < numsegs; i++)               // Go through each seg
    {
        const line_t *l = segs[i].linedef;      // The parent linedef

        if (l->dx && l->dy)                     // We can ignore orthogonal lines
        {
            vertex_t *v = segs[i].v1;

            do
                if (!hit[v - vertexes])         // If we haven't processed vertex
                {
                    hit[v - vertexes] = 1;          // Mark this vertex as processed

                    if (v != l->v1 && v != l->v2)   // Exclude endpoints of linedefs
                    {
                        // Project the vertex back onto the parent linedef
                        long long dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
                        long long dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
                        long long dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
                        long long s = dx2 + dy2;
                        int x0 = v->x, y0 = v->y, x1 = l->v1->x, y1 = l->v1->y;

                        v->x = (int)((dx2 * x0 + dy2 * x1 + dxy * (y0 - y1)) / s);
                        v->y = (int)((dy2 * y0 + dx2 * y1 + dxy * (x0 - x1)) / s);
                    }
                }  // Obsfucated C contest entry:   :)
            while ((v != segs[i].v2) && (v = segs[i].v2));
        }
    }
    free(hit);
}

char            mapnum[6];
char            maptitle[128];
char            mapnumandtitle[133];

extern char     *mapnames[][6];

// Determine map name to use
void P_MapName(int episode, int map)
{
    switch (gamemission)
    {
        case doom:
            sprintf(mapnum, "E%iM%i", episode, map);
            if (W_CheckMultipleLumps(mapnum))
            {
                strcpy(maptitle, mapnum);
                strcpy(mapnumandtitle, mapnum);
            }
            else
            {
                strcpy(maptitle, mapnames[(episode - 1) * 9 + map - 1][doom]);
                sprintf(mapnumandtitle, "%s: %s", mapnum, maptitle);
            }
            break;

        case doom2:
            sprintf(mapnum, "MAP%02i", map);
            if (W_CheckMultipleLumps(mapnum) && (!nerve || map > 9))
            {
                strcpy(maptitle, mapnum);
                strcpy(mapnumandtitle, mapnum);
            }
            else
            {
                strcpy(maptitle, mapnames[map - 1][bfgedition ? doom2bfg : doom2]);
                sprintf(mapnumandtitle, "%s: %s", mapnum, maptitle);
            }
            break;

        case pack_nerve:
            sprintf(mapnum, "MAP%02i", map);
            strcpy(maptitle, mapnames[map - 1][pack_nerve]);
            sprintf(mapnumandtitle, "%s: %s", mapnum, maptitle);
            break;

        case pack_plut:
        case pack_tnt:
            sprintf(mapnum, "MAP%02i", map);
            if (W_CheckMultipleLumps(mapnum))
            {
                strcpy(maptitle, mapnum);
                strcpy(mapnumandtitle, mapnum);
            }
            else
            {
                strcpy(maptitle, mapnames[map - 1][gamemission]);
                sprintf(mapnumandtitle, "%s: %s", mapnum, maptitle);
            }
            break;

        default:
            break;
    }
}


extern boolean idclev;
extern int oldweaponsowned[];

//
// P_SetupLevel
//
void P_SetupLevel(int episode, int map, int playermask, skill_t skill)
{
    int         i;
    char        lumpname[9];
    int         lumpnum;

    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 0;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        players[i].killcount = players[i].secretcount = players[i].itemcount = 0;
    }

    // Initial height of PointOfView
    // will be set by player think.
    players[consoleplayer].viewz = 1;

    idclev = false;

    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start();

    Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

    P_InitThinkers();

    // find map name
    if (gamemode == commercial)
    {
        if (map < 10)
            snprintf(lumpname, 9, "map0%i", map);
        else
            snprintf(lumpname, 9, "map%i", map);
    }
    else
    {
        lumpname[0] = 'E';
        lumpname[1] = '0' + episode;
        lumpname[2] = 'M';
        lumpname[3] = '0' + map;
        lumpname[4] = 0;
    }

    if (nerve && gamemission == doom2)
        lumpnum = W_GetNumForName2(lumpname);
    else
        lumpnum = W_GetNumForName(lumpname);

    canmodify = (!W_CheckMultipleLumps(lumpname)
                 || gamemission == pack_nerve
                 || (nerve && gamemission == doom2));

    leveltime = 0;

    P_MapName(gameepisode, gamemap);

    // note: most of this ordering is important
    P_LoadBlockMap(lumpnum + ML_BLOCKMAP);
    P_LoadVertexes(lumpnum + ML_VERTEXES);
    P_LoadSectors(lumpnum + ML_SECTORS);
    P_LoadSideDefs(lumpnum + ML_SIDEDEFS);

    P_LoadLineDefs(lumpnum + ML_LINEDEFS);
    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes(lumpnum + ML_NODES);
    P_LoadSegs(lumpnum + ML_SEGS);

    rejectmatrix = (byte *)W_CacheLumpNum(lumpnum + ML_REJECT, PU_LEVEL);
    P_GroupLines();

    P_RemoveSlimeTrails();

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;
    P_LoadThings(lumpnum + ML_THINGS);

    // if deathmatch, randomly spawn the active players
    if (deathmatch)
    {
        for (i = 0; i < MAXPLAYERS; i++)
            if (playeringame[i])
            {
                players[i].mo = NULL;
                G_DeathMatchSpawnPlayer(i);
            }

    }

    // clear special respawning queue
    iquehead = iquetail = 0;

    // set up world state
    P_SpawnSpecials();

    // preload graphics
    R_PrecacheLevel();
}



//
// P_Init
//
void P_Init(void)
{
    P_InitSwitchList();
    P_InitPicAnims();
    R_InitSprites(sprnames);
}