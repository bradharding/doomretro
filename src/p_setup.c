/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#include <math.h>

#include "doomstat.h"
#include "g_game.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_bbox.h"
#include "m_misc.h"
#include "p_fix.h"
#include "p_local.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

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
uint32_t        *blockmapindex;

// offsets in blockmap are from here
uint32_t        *blockmaphead;

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
int             rejectmatrixsize;

// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS   10

mapthing_t      deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t      *deathmatch_p;
mapthing_t      playerstarts[MAXPLAYERS];

boolean         canmodify;

int             mapfixes = MAPFIXES_DEFAULT;

static int      current_episode = -1;
static int      current_map = -1;
static int      samelevel = false;

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

#define DEFAULT 0x7fff

//
// P_LoadVertexes
//
void P_LoadVertexes(int lump)
{
    const mapvertex_t   *data;
    int                 i;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = calloc_IfSameLevel(vertexes, numvertexes, sizeof(vertex_t));

    // Load data into cache.
    data = (const mapvertex_t *)W_CacheLumpNum(lump, PU_STATIC);

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i = 0; i < numvertexes; i++)
    {
        vertexes[i].x = SHORT(data[i].x) << FRACBITS;
        vertexes[i].y = SHORT(data[i].y) << FRACBITS;

        // Apply any map-specific fixes.
        if (canmodify && (mapfixes & VERTEXES))
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

    numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
    segs = calloc_IfSameLevel(segs, numsegs, sizeof(seg_t));
    memset(segs, 0, numsegs * sizeof(seg_t));
    data = (const mapseg_t *)W_CacheLumpNum(lump, PU_STATIC);

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
            I_Error("P_LoadSegs: invalid linedef %d", linedef);

        ldef = &lines[linedef];
        li->linedef = ldef;

        side = SHORT(ml->side);

        // e6y: fix wrong side index
        if (side != 0 && side != 1)
            side = 1;

        li->sidedef = &sides[ldef->sidenum[side]];

        // cph 2006/09/30 - our frontsector can be the second side of the
        // linedef, so must check for NO_INDEX in case we are incorrectly
        // referencing the back of a 1S line
        if (ldef->sidenum[side] != NO_INDEX)
            li->frontsector = sides[ldef->sidenum[side]].sector;
        else
            li->frontsector = 0;

        if (ldef-> flags & ML_TWOSIDED)
        {
            int sidenum = ldef->sidenum[side ^ 1];

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
            li->backsector = 0;

        // e6y
        // check and fix wrong references to non-existent vertexes
        // see e1m9 @ NIVELES.WAD
        // http://www.doomworld.com/idgames/index.php?id=12647
        if (v1 >= numvertexes || v2 >= numvertexes)
        {

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

        // Apply any map-specific fixes.
        if (canmodify && (mapfixes & LINEDEFS))
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

//
// P_LoadSubsectors
//
void P_LoadSubsectors(int lump)
{
    const mapsubsector_t *data;
    int                  i;

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_t);
    subsectors = calloc_IfSameLevel(subsectors, numsubsectors, sizeof(subsector_t));
    data = (const mapsubsector_t *)W_CacheLumpNum(lump, PU_STATIC);

    memset(subsectors, 0, numsubsectors * sizeof(subsector_t));

    for (i = 0; i < numsubsectors; i++)
    {
        subsectors[i].numlines = (unsigned short)SHORT(data[i].numsegs);
        subsectors[i].firstline = (unsigned short)SHORT(data[i].firstseg);
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

    numsectors = W_LumpLength(lump) / sizeof(mapsector_t);
    sectors = calloc_IfSameLevel(sectors, numsectors, sizeof(sector_t));
    memset(sectors, 0, numsectors * sizeof(sector_t));
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

        // Apply any level-specific fixes.
        if (canmodify && (mapfixes & SECTORS))
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
    const byte  *data;
    int         i;

    numnodes = W_LumpLength(lump) / sizeof(mapnode_t);
    nodes = malloc_IfSameLevel(nodes, numnodes * sizeof(node_t));
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

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
                    no->children[j] = 0;

                no->children[j] |= NF_SUBSECTOR;
            }

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
    const mapthing_t    *data;
    int                 i;
    int                 numthings;

    data = (const mapthing_t *)W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_t);

    for (i = 0; i < numthings; i++)
    {
        mapthing_t      mt = data[i];
        boolean         spawn = true;

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
        if (canmodify && (mapfixes & THINGS))
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
            P_SpawnMapThing(&mt);
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs(int lump)
{
    const byte  *data = W_CacheLumpNum(lump, PU_STATIC);
    int         i;

    numlines = W_LumpLength(lump) / sizeof(maplinedef_t);
    lines = calloc_IfSameLevel(lines, numlines, sizeof(line_t));
    memset(lines, 0, numlines * sizeof(line_t));

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
        ld->soundorg.x = (ld->bbox[BOXLEFT] + ld->bbox[BOXRIGHT]) / 2;
        ld->soundorg.y = (ld->bbox[BOXTOP] + ld->bbox[BOXBOTTOM]) / 2;

        ld->sidenum[0] = SHORT(mld->sidenum[0]);
        ld->sidenum[1] = SHORT(mld->sidenum[1]);

        {
            // cph 2006/09/30 - fix sidedef errors right away
            int j;

            for (j = 0; j < 2; j++)
                if (ld->sidenum[j] != NO_INDEX && ld->sidenum[j] >= numsides) 
                    ld->sidenum[j] = NO_INDEX;

            // killough 11/98: fix common wad errors (missing sidedefs):
            if (ld->sidenum[0] == NO_INDEX)
                ld->sidenum[0] = 0;  // Substitute dummy sidedef for missing right side

            if (ld->sidenum[1] == NO_INDEX && (ld->flags & ML_TWOSIDED))
                ld->flags &= ~ML_TWOSIDED;  // Clear 2s flag for missing left side
        }

        ld->frontsector = (ld->sidenum[0] == NO_INDEX ? 0 : sides[ld->sidenum[0]].sector);
        ld->backsector = (ld->sidenum[1] == NO_INDEX ? 0 : sides[ld->sidenum[1]].sector);
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadSideDefs
//
void P_LoadSideDefs(int lump)
{
    const byte  *data;
    int         i;

    numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
    sides = calloc_IfSameLevel(sides, numsides, sizeof(side_t));
    memset(sides, 0, numsides * sizeof(side_t));
    data = (byte *)W_CacheLumpNum(lump, PU_STATIC);

    for (i = 0; i < numsides; i++)
    {
        mapsidedef_t    *msd = (mapsidedef_t *)data + i;
        side_t          *sd = sides + i;
        unsigned short  sector_num = SHORT(msd->sector);

        sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;

        // cph 2006/09/30 - catch out-of-range sector numbers; use sector 0 instead
        if (sector_num >= numsectors)
            sector_num = 0;
        sd->sector = &sectors[sector_num];

        sd->toptexture = R_TextureNumForName(msd->toptexture);
        sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
        sd->midtexture = R_TextureNumForName(msd->midtexture);
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadBlockMap
//
// Read wad blockmap using int16_t wadblockmaplump[].
// Expand from 16bit wad to internal 32bit blockmap.
// (Taken from Doom Legacy)
//
void P_LoadBlockMap(int lump)
{
    unsigned int        count = W_LumpLength(lump) / 2;                    // number of 16 bit blockmap entries
    uint16_t            *wadblockmaplump = W_CacheLumpNum(lump, PU_LEVEL); // blockmap lump temp
    uint32_t            firstlist, lastlist;  // blockmap block list bounds
    uint32_t            overflow_corr = 0;
    uint32_t            prev_bme = 0;  // for detecting overflow wrap
    unsigned int        i;

    // [WDJ] when zennode has not been run, this code will corrupt Zone memory.
    // It assumes a minimum size blockmap.
    if (count < 5)
        I_Error("Missing blockmap, node builder has not been run.\n");

    // [WDJ] Do endian as read from blockmap lump temp
    blockmaphead = malloc_IfSameLevel(blockmaphead, sizeof(*blockmaphead) * count);

    // killough 3/1/98: Expand wad blockmap into larger internal one,
    // by treating all offsets except -1 as unsigned and zero-extending
    // them. This potentially doubles the size of blockmaps allowed,
    // because Doom originally considered the offsets as always signed.
    // [WDJ] They are unsigned in Unofficial Doom Spec.
    blockmaphead[0] = LE_SWAP16(wadblockmaplump[0]);            // map orgin_x
    blockmaphead[1] = LE_SWAP16(wadblockmaplump[1]);            // map orgin_y
    blockmaphead[2] = LE_SWAP16(wadblockmaplump[2]);            // number columns (x size)
    blockmaphead[3] = LE_SWAP16(wadblockmaplump[3]);            // number rows (y size)

    bmaporgx = blockmaphead[0] << FRACBITS;
    bmaporgy = blockmaphead[1] << FRACBITS;
    bmapwidth = blockmaphead[2];
    bmapheight = blockmaphead[3];
    blockmapindex = &blockmaphead[4];
    firstlist = 4 + bmapwidth * bmapheight;
    lastlist = count - 1;

    if (firstlist >= lastlist || bmapwidth < 1 || bmapheight < 1)
        I_Error("Blockmap corrupt, must run node builder on wad.\n");

    // read blockmap index array
    for (i = 4; i < firstlist; i++)                             // for all entries in wad offset index
    {
        uint32_t        bme = LE_SWAP16(wadblockmaplump[i]);    // offset

        // upon overflow, the bme will wrap to low values
        if (bme < firstlist                                     // too small to be valid
            && bme < 0x1000 && prev_bme > 0xf000)               // wrapped
        {
            // first or repeated overflow
            overflow_corr += 0x00010000;
        }
        prev_bme = bme;                                         // uncorrected

        // correct for overflow, or else try without correction
        if (overflow_corr)
        {
            uint32_t    bmec = bme + overflow_corr;

            // First entry of list is 0, but high odds of hitting one randomly.
            // Check for valid blockmap offset, and offset overflow
            if (bmec <= lastlist
                && wadblockmaplump[bmec] == 0                   // valid start list
                && bmec - blockmaphead[i - 1] < 1000)           // reasonably close sequentially
            {
                bme = bmec;
            }
        }

        if (bme > lastlist)
            I_Error("Blockmap offset[%i]= %i, exceeds bounds.\n", i, bme);
        if (bme < firstlist
            || wadblockmaplump[bme] != 0)                       // not start list
            I_Error("Bad blockmap offset[%i]= %i.\n", i, bme);
        blockmaphead[i] = bme;
    }

    // read blockmap lists
    for (i = firstlist; i < count; i++)                 // for all list entries in wad blockmap
    {
        // killough 3/1/98
        // keep -1 (0xffff), but other values are unsigned
        uint16_t        bme = LE_SWAP16(wadblockmaplump[i]);

        blockmaphead[i] = (bme == 0xffff ? (uint32_t)(-1) : (uint32_t)bme);
    }

    // clear out mobj chains
    blocklinks = calloc_IfSameLevel(blocklinks, bmapwidth * bmapheight, sizeof(*blocklinks));
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
// killough 5/3/98: reformatted, cleaned up
// cph 18/8/99: rewritten to avoid O(numlines * numsectors) section
// It makes things more complicated, but saves seconds on big levels
// figgi 09/18/00 -- adapted for gl-nodes

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
        if (subsectors[i].sector == NULL)
            I_Error("P_GroupLines: Subsector a part of no sector!");
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

        // e6y: REJECT overrun emulation code
        // moved to P_LoadReject
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

static void P_RemoveSlimeTrails(void)                   // killough 10/98
{
    byte        *hit = (byte *)calloc(1, numvertexes);  // Hitlist for vertices
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

                        v->x = (int)((dx2 * x0 + dy2 * x1 + dxy * (y0 - y1)) / s);
                        v->y = (int)((dy2 * y0 + dx2 * y1 + dxy * (x0 - x1)) / s);
                    }
                }  // Obsfucated C contest entry:   :)
            }
            while (v != segs[i].v2 && (v = segs[i].v2));
        }
    }
    free(hit);
}

// precalc values for use later in long wall error fix in R_StoreWallRange()
static void P_CalcSegsLength(void)
{
    int i;

    for (i = 0; i < numsegs; i++)
    {
        seg_t   *li = segs + i;
        fixed_t dx = li->v2->x - li->v1->x;
        fixed_t dy = li->v2->y - li->v1->y;

        li->length = (fixed_t)sqrt((double)dx * dx + (double)dy * dy);
    }
}

char            mapnum[6];
char            maptitle[128];
char            mapnumandtitle[133];
char            automaptitle[133];

extern char     **mapnames[];
extern char     **mapnames2[];
extern char     **mapnames2_bfg[];
extern char     **mapnamesp[];
extern char     **mapnamest[];
extern char     **mapnamesn[];

extern int      dehcount;

void ExtractFileBase(char *path, char *dest);
char *uppercase(char *str);

// Determine map name to use
void P_MapName(int episode, int map)
{
    char        *pos;
    char        wad[260];
    int         i;
    boolean     mapnumonly = false;

    switch (gamemission)
    {
        case doom:
            M_snprintf(mapnum, sizeof(mapnum), "E%iM%i", episode, map);
            i = (episode - 1) * 9 + map - 1;
            if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1 && !chex)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                ExtractFileBase(lumpinfo[W_GetNumForName(mapnum)].wad_file->path, wad);
                M_snprintf(automaptitle, 133, "%s.wad's %s", wad, mapnum);
            }
            else
                M_StringCopy(maptitle, *mapnames[i], sizeof(maptitle));
            break;

        case doom2:
            i = map - 1;
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);
            if (W_CheckMultipleLumps(mapnum) > 1 && (!nerve || map > 9) && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                ExtractFileBase(lumpinfo[W_GetNumForName(mapnum)].wad_file->path, wad);
                M_snprintf(automaptitle, 133, "%s.wad's %s", wad, mapnum);
            }
            else
                M_StringCopy(maptitle, (bfgedition ? *mapnames2_bfg[i] : *mapnames2[i]),
                    sizeof(maptitle));
            break;

        case pack_nerve:
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);
            M_StringCopy(maptitle, *mapnamesn[map - 1], sizeof(maptitle));
            break;

        case pack_plut:
            i = map - 1;
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);
            if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                ExtractFileBase(lumpinfo[W_GetNumForName(mapnum)].wad_file->path, wad);
                M_snprintf(automaptitle, 133, "%s.wad's %s", wad, mapnum);
            }
            else
                M_StringCopy(maptitle, *mapnamesp[map - 1], sizeof(maptitle));
            break;

        case pack_tnt:
            i = map - 1;
            M_snprintf(mapnum, sizeof(mapnum), "MAP%02i", map);
            if (W_CheckMultipleLumps(mapnum) > 1 && dehcount == 1)
            {
                mapnumonly = true;
                M_StringCopy(maptitle, mapnum, sizeof(maptitle));
                M_StringCopy(mapnumandtitle, mapnum, sizeof(mapnumandtitle));
                ExtractFileBase(lumpinfo[W_GetNumForName(mapnum)].wad_file->path, wad);
                M_snprintf(automaptitle, 133, "%s.wad's %s", wad, mapnum);
            }
            else
                M_StringCopy(maptitle, *mapnamest[map - 1], sizeof(maptitle));
            break;
            
        default:
            break;
    }

    if (!mapnumonly)
    {
        if ((pos = strchr(maptitle, ':')))
        {
            if (M_StringStartsWith(uppercase(maptitle), "LEVEL"))
            {
                strcpy(maptitle, pos + 1);
                if (maptitle[0] == ' ')
                    strcpy(maptitle, &maptitle[1]);
                M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s: %s", mapnum, maptitle);
            }
            else
            {
                M_StringCopy(mapnumandtitle, maptitle, sizeof(mapnumandtitle));
                strcpy(maptitle, pos + 1);
                if (maptitle[0] == ' ')
                    strcpy(maptitle, &maptitle[1]);
            }
        }
        else
            M_snprintf(mapnumandtitle, sizeof(mapnumandtitle), "%s: %s", mapnum, maptitle);
        M_StringCopy(automaptitle, mapnumandtitle, sizeof(automaptitle));
    }
}

extern boolean idclev;
extern boolean oldweaponsowned[];

//
// P_SetupLevel
//
void P_SetupLevel(int episode, int map)
{
    int  i;
    char lumpname[6];
    int  lumpnum;

    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 0;
    for (i = 0; i < MAXPLAYERS; i++)
        players[i].killcount = players[i].secretcount = players[i].itemcount = 0;

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
        M_snprintf(lumpname, 6, "MAP%02i", map);
    else
        M_snprintf(lumpname, 5, "E%iM%i", episode, map);

    if (nerve && gamemission == doom2)
        lumpnum = W_GetNumForName2(lumpname);
    else
        lumpnum = W_GetNumForName(lumpname);

    canmodify = (W_CheckMultipleLumps(lumpname) == 1
                 || gamemission == pack_nerve
                 || (nerve && gamemission == doom2));

    leveltime = 0;

    // e6y: speedup of level reloading
    // Most of level's structures now are allocated with PU_STATIC instead of PU_LEVEL
    samelevel = (map == current_map && episode == current_episode);

    current_episode = episode;
    current_map = map;

    if (!samelevel)
    {
        free(segs);
        free(nodes);
        free(subsectors);
        free(blocklinks);
        free(blockmaphead);
        free(lines);
        free(sides);
        free(sectors);
        free(vertexes);
    }

    P_MapName(gameepisode, gamemap);

    // note: most of this ordering is important
    if (!samelevel)
        P_LoadBlockMap(lumpnum + ML_BLOCKMAP);
    else
        memset(blocklinks, 0, bmapwidth * bmapheight * sizeof(*blocklinks));

    P_LoadVertexes(lumpnum + ML_VERTEXES);
    P_LoadSectors(lumpnum + ML_SECTORS);
    P_LoadSideDefs(lumpnum + ML_SIDEDEFS);

    P_LoadLineDefs(lumpnum + ML_LINEDEFS);
    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes(lumpnum + ML_NODES);
    P_LoadSegs(lumpnum + ML_SEGS);

    rejectmatrix = (byte *)W_CacheLumpNum(lumpnum + ML_REJECT, PU_LEVEL);
    rejectmatrixsize = W_LumpLength(lumpnum + ML_REJECT);
    P_GroupLines();

    P_RemoveSlimeTrails();

    P_CalcSegsLength();

    deathmatch_p = deathmatchstarts;

    bloodSplatQueueSlot = 0;
    memset(bloodSplatQueue, 0, sizeof(mobj_t *) * bloodsplats);

    P_LoadThings(lumpnum + ML_THINGS);

    P_InitCards(&players[0]);
    P_InitAnimatedLiquids();

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
    iqueuehead = iqueuetail = 0;

    // set up world state
    P_SpawnSpecials();

    P_MapEnd();

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
