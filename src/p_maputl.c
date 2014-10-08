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

#include <stdlib.h>
#include "m_bbox.h"
#include "p_local.h"

extern msecnode_t *sector_list; // phares 3/16/98

void P_CreateSecNodeList(mobj_t *thing, fixed_t x, fixed_t y);

//
// P_ApproxDistance
// Gives an estimation of distance (not exact)
//
fixed_t P_ApproxDistance(fixed_t dx, fixed_t dy)
{
    dx = ABS(dx);
    dy = ABS(dy);
    if (dx < dy)
        return (dx + dy - (dx >> 1));
    return (dx + dy - (dy >> 1));
}

//
// P_PointOnLineSide
// Returns 0 or 1
// killough 5/3/98: reformatted, cleaned up
//
int P_PointOnLineSide(fixed_t x, fixed_t y, line_t *line)
{
    return (!line->dx ? x <= line->v1->x ? line->dy > 0 : line->dy < 0 :
        !line->dy ? y <= line->v1->y ? line->dx < 0 : line->dx > 0 :
        FixedMul(y-line->v1->y, line->dx >> FRACBITS) >=
        FixedMul(line->dy >> FRACBITS, x-line->v1->x));
}

//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
// killough 5/3/98: reformatted, cleaned up
//
int P_BoxOnLineSide(fixed_t *tmbox, line_t *ld)
{
    switch (ld->slopetype)
    {
        int     p;

        default:
        case ST_HORIZONTAL:
            return ((tmbox[BOXBOTTOM] > ld->v1->y) == (p = tmbox[BOXTOP] > ld->v1->y) ?
                p ^ (ld->dx < 0) : -1);
        case ST_VERTICAL:
            return ((tmbox[BOXLEFT] < ld->v1->x) == (p = tmbox[BOXRIGHT] < ld->v1->x) ?
                p ^ (ld->dy < 0) : -1);
        case ST_POSITIVE:
            return (P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld) ==
                (p = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld)) ? p : -1);
        case ST_NEGATIVE:
            return ((P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld)) ==
                (p = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld)) ? p : -1);
    }
}

//
// P_PointOnDivlineSide
// Returns 0 or 1.
// killough 5/3/98: reformatted, cleaned up
//
int P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t *line)
{
    return (!line->dx ? x <= line->x ? line->dy > 0 : line->dy < 0 :
        !line->dy ? y <= line->y ? line->dx < 0 : line->dx > 0 :
        (line->dy ^ line->dx ^ (x -= line->x) ^ (y -= line->y)) < 0 ? (line->dy ^ x) < 0 :
        FixedMul(y >> 8, line->dx >> 8) >= FixedMul(line->dy >> 8, x >> 8));
}

//
// P_MakeDivline
//
void P_MakeDivline(line_t *li, divline_t *dl)
{
    dl->x = li->v1->x;
    dl->y = li->v1->y;
    dl->dx = li->dx;
    dl->dy = li->dy;
}

//
// P_InterceptVector
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings
// and addlines traversers.
// killough 5/3/98: reformatted, cleaned up
//
fixed_t P_InterceptVector(divline_t *v2, divline_t *v1)
{
    fixed_t     den = FixedMul(v1->dy >> 8, v2->dx) - FixedMul(v1->dx >> 8, v2->dy);

    return (den ? FixedDiv((FixedMul((v1->x - v2->x) >> 8, v1->dy) +
        FixedMul((v2->y - v1->y) >> 8, v1->dx)), den) : 0);
}

//
// P_LineOpening
// Sets opentop and openbottom to the window
// through a two sided line.
// OPTIMIZE: keep this precalculated
//
fixed_t opentop;
fixed_t openbottom;
fixed_t openrange;
fixed_t lowfloor;

void P_LineOpening(line_t *linedef)
{
    sector_t    *front;
    sector_t    *back;

    if (linedef->sidenum[1] == NO_INDEX)
    {
        // single sided line
        openrange = 0;
        return;
    }

    front = linedef->frontsector;
    back = linedef->backsector;

    if (front->ceilingheight < back->ceilingheight)
        opentop = front->ceilingheight;
    else
        opentop = back->ceilingheight;

    if (front->floorheight > back->floorheight)
    {
        openbottom = front->floorheight;
        lowfloor = back->floorheight;
    }
    else
    {
        openbottom = back->floorheight;
        lowfloor = front->floorheight;
    }

    openrange = opentop - openbottom;
}

//
// THING POSITION SETTING
//

//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors.
// On each position change, BLOCKMAP and other
// lookups maintaining lists ot things inside
// these structures need to be updated.
//
void P_UnsetThingPosition(mobj_t *thing)
{
    if (!(thing->flags & MF_NOSECTOR))
    {
        // inert things don't need to be in blockmap?
        // unlink from subsector
        if (thing->snext)
            thing->snext->sprev = thing->sprev;

        if (thing->sprev)
            thing->sprev->snext = thing->snext;
        else
            thing->subsector->sector->thinglist = thing->snext;

        // phares 3/14/98
        //
        // Save the sector list pointed to by touching_sectorlist.
        // In P_SetThingPosition, we'll keep any nodes that represent
        // sectors the Thing still touches. We'll add new ones then, and
        // delete any nodes for sectors the Thing has vacated. Then we'll
        // put it back into touching_sectorlist. It's done this way to
        // avoid a lot of deleting/creating for nodes, when most of the
        // time you just get back what you deleted anyway.
        //
        // If this Thing is being removed entirely, then the calling
        // routine will clear out the nodes in sector_list.
        thing->old_sectorlist = thing->touching_sectorlist;
        thing->touching_sectorlist = NULL;      // to be restored by P_SetThingPosition
    }

    if (!(thing->flags & MF_NOBLOCKMAP))
    {
        // inert things don't need to be in blockmap
        // unlink from block map
        if (thing->bnext)
            thing->bnext->bprev = thing->bprev;

        if (thing->bprev)
            thing->bprev->bnext = thing->bnext;
        else
        {
            int blockx = (thing->x - bmaporgx) >> MAPBLOCKSHIFT;
            int blocky = (thing->y - bmaporgy) >> MAPBLOCKSHIFT;

            if (blockx >= 0 && blockx < bmapwidth && blocky >= 0 && blocky < bmapheight)
                blocklinks[blocky * bmapwidth + blockx] = thing->bnext;
        }
    }
}

//
// P_SetThingPosition
// Links a thing into both a block and a subsector
// based on its x y.
// Sets thing->subsector properly
//
void P_SetThingPosition(mobj_t *thing)
{
    subsector_t *ss;

    // link into subsector
    ss = R_PointInSubsector(thing->x, thing->y);
    thing->subsector = ss;

    if (!(thing->flags & MF_NOSECTOR))
    {
        // invisible things don't go into the sector links
        sector_t *sec = ss->sector;

        thing->sprev = NULL;
        thing->snext = sec->thinglist;

        if (sec->thinglist)
            sec->thinglist->sprev = thing;

        sec->thinglist = thing;

        // phares 3/16/98
        //
        // If sector_list isn't NULL, it has a collection of sector
        // nodes that were just removed from this Thing.

        // Collect the sectors the object will live in by looking at
        // the existing sector_list and adding new nodes and deleting
        // obsolete ones.

        // When a node is deleted, its sector links (the links starting
        // at sector_t->touching_thinglist) are broken. When a node is
        // added, new sector links are created.
        P_CreateSecNodeList(thing, thing->x, thing->y);
        thing->touching_sectorlist = sector_list;       // Attach to Thing's mobj_t
        thing->old_sectorlist = NULL;                             // clear for next time
    }

    // link into blockmap
    if (!(thing->flags & MF_NOBLOCKMAP))
    {
        // inert things don't need to be in blockmap
        int     blockx = (thing->x - bmaporgx) >> MAPBLOCKSHIFT;
        int     blocky = (thing->y - bmaporgy) >> MAPBLOCKSHIFT;

        if (blockx >= 0 && blockx < bmapwidth && blocky >= 0 && blocky < bmapheight)
        {
            mobj_t      **link = &blocklinks[blocky * bmapwidth + blockx];

            thing->bprev = NULL;
            thing->bnext = *link;
            if (*link)
                (*link)->bprev = thing;

            *link = thing;
        }
        else
        {
            // thing is off the map
            thing->bnext = thing->bprev = NULL;
        }
    }
}

//
// BLOCK MAP ITERATORS
// For each line/thing in the given mapblock,
// call the passed PIT_* function.
// If the function returns false,
// exit with false without checking anything else.
//

//
// P_BlockLinesIterator
// The validcount flags are used to avoid checking lines
// that are marked in multiple mapblocks,
// so increment validcount before the first call
// to P_BlockLinesIterator, then make one or more calls
// to it.
//
boolean P_BlockLinesIterator(int x, int y, boolean (*func)(line_t *))
{
    if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
        return true;
    else
    {
        int             offset = blockmapindex[y * bmapwidth + x];
        const uint32_t  *list;

        for (list = &blockmaphead[offset]; *list != (uint32_t)(-1); list++)
        {
            line_t          *ld = &lines[*list];

            if (ld->validcount == validcount)
                continue;       // line has already been checked

            ld->validcount = validcount;

            if (!func(ld))
                return false;
        }
        return true;            // everything was checked
    }
}

//
// P_BlockThingsIterator
//
boolean P_BlockThingsIterator(int x, int y, boolean (*func)(mobj_t *))
{
    mobj_t      *mobj;

    if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
        return true;

    for (mobj = blocklinks[y * bmapwidth + x]; mobj; mobj = mobj->bnext)
        if (!func(mobj))
            return false;
    return true;
}

//
// INTERCEPT ROUTINES
//

// 1/11/98 killough: Intercept limit removed
static intercept_t      *intercepts;
static intercept_t      *intercept_p;

// Check for limit and double size if necessary -- killough
static void check_intercept(void)
{
    static size_t       num_intercepts;
    size_t              offset = intercept_p - intercepts;

    if (offset >= num_intercepts)
    {
        num_intercepts = (num_intercepts ? num_intercepts * 2 : 128);
        intercepts = (intercept_t *)realloc(intercepts, sizeof(*intercepts) * num_intercepts);
        intercept_p = intercepts + offset;
    }
}

divline_t       trace;
boolean         earlyout;
int             ptflags;

//
// PIT_AddLineIntercepts.
// Looks for lines in the given block
// that intercept the given trace
// to add to the intercepts list.
//
// A line is crossed if its endpoints
// are on opposite sides of the trace.
// Returns true if earlyout and a solid line hit.
//
boolean PIT_AddLineIntercepts(line_t *ld)
{
    int         s1;
    int         s2;
    fixed_t     frac;
    divline_t   dl;

    // avoid precision problems with two routines
    if (trace.dx > FRACUNIT * 16 || trace.dy > FRACUNIT * 16
        || trace.dx < -FRACUNIT * 16 || trace.dy < -FRACUNIT * 16)
    {
        s1 = P_PointOnDivlineSide(ld->v1->x, ld->v1->y, &trace);
        s2 = P_PointOnDivlineSide(ld->v2->x, ld->v2->y, &trace);
    }
    else
    {
        s1 = P_PointOnLineSide(trace.x, trace.y, ld);
        s2 = P_PointOnLineSide(trace.x + trace.dx, trace.y + trace.dy, ld);
    }

    if (s1 == s2)
        return true;    // line isn't crossed

    // hit the line
    P_MakeDivline(ld, &dl);
    frac = P_InterceptVector(&trace, &dl);

    if (frac < 0)
        return true;    // behind source

    // try to early out the check
    if (earlyout && frac < FRACUNIT && !ld->backsector)
        return false;   // stop checking

    check_intercept();

    intercept_p->frac = frac;
    intercept_p->isaline = true;
    intercept_p->d.line = ld;
    intercept_p++;

    return true;        // continue
}

//
// PIT_AddThingIntercepts
//
boolean PIT_AddThingIntercepts(mobj_t *thing)
{
    // Taken from ZDoom:
    // [RH] Don't check a corner to corner crossection for hit.
    // Instead, check against the actual bounding box.

    // There's probably a smarter way to determine which two sides
    // of the thing face the trace than by trying all four sides...
    int         numfronts = 0;
    divline_t   line;
    int         i;

    for (i = 0; i < 4; ++i)
    {
        switch (i)
        {
            case 0:     // Top edge
                line.x = thing->x + thing->radius;
                line.y = thing->y + thing->radius;
                line.dx = -thing->radius * 2;
                line.dy = 0;
                break;

            case 1:     // Right edge
                line.x = thing->x + thing->radius;
                line.y = thing->y - thing->radius;
                line.dx = 0;
                line.dy = thing->radius * 2;
                break;

            case 2:     // Bottom edge
                line.x = thing->x - thing->radius;
                line.y = thing->y - thing->radius;
                line.dx = thing->radius * 2;
                line.dy = 0;
                break;

            case 3:     // Left edge
                line.x = thing->x - thing->radius;
                line.y = thing->y + thing->radius;
                line.dx = 0;
                line.dy = thing->radius * -2;
                break;
        }

        // Check if this side is facing the trace origin
        if (P_PointOnDivlineSide(trace.x, trace.y, &line) == 0)
        {
            numfronts++;

            // If it is, see if the trace crosses it
            if (P_PointOnDivlineSide(line.x, line.y, &trace) !=
                P_PointOnDivlineSide(line.x + line.dx, line.y + line.dy, &trace))
            {
                // It's a hit
                fixed_t frac = P_InterceptVector(&trace, &line);

                if (frac < 0)
                    return true;        // behind source

                check_intercept();

                intercept_p->frac = frac;
                intercept_p->isaline = false;
                intercept_p->d.thing = thing;
                intercept_p++;
                return true;
            }
        }
    }

    // If none of the sides were facing the trace, then the trace
    // must have started inside the box, so add it as an intercept.
    if (numfronts == 0)
    {
        check_intercept();

        intercept_p->frac = 0;
        intercept_p->isaline = false;
        intercept_p->d.thing = thing;
        intercept_p++;
    }
    return true;
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true
// for all lines.
//
boolean P_TraverseIntercepts(traverser_t func, fixed_t maxfrac)
{
    int         count = intercept_p - intercepts;
    intercept_t *in = NULL;

    while (count--)
    {
        fixed_t     dist = INT_MAX;
        intercept_t *scan;

        for (scan = intercepts; scan < intercept_p; scan++)
        {
            if (scan->frac < dist)
            {
                dist = scan->frac;
                in = scan;
            }
        }

        if (dist > maxfrac)
            return true;        // checked everything in range

        if (!func(in))
            return false;       // don't bother going farther

        in->frac = INT_MAX;
    }

    return true;                // everything was traversed
}

//
// P_PathTraverse
// Traces a line from x1,y1 to x2,y2,
// calling the traverser function for each.
// Returns true if the traverser function returns true
// for all lines.
//
boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                       int flags, boolean (*trav)(intercept_t *))
{
    fixed_t     xt1, yt1;
    fixed_t     xt2, yt2;
    fixed_t     xstep, ystep;
    fixed_t     partial;
    fixed_t     xintercept, yintercept;
    int         mapx, mapy;
    int         mapxstep, mapystep;
    int         count;

    earlyout = (flags & PT_EARLYOUT);

    validcount++;
    intercept_p = intercepts;

    if (!((x1 - bmaporgx) & (MAPBLOCKSIZE - 1)))
        x1 += FRACUNIT;         // don't side exactly on a line

    if (!((y1 - bmaporgy) & (MAPBLOCKSIZE - 1)))
        y1 += FRACUNIT;         // don't side exactly on a line

    trace.x = x1;
    trace.y = y1;
    trace.dx = x2 - x1;
    trace.dy = y2 - y1;

    x1 -= bmaporgx;
    y1 -= bmaporgy;
    xt1 = x1 >> MAPBLOCKSHIFT;
    yt1 = y1 >> MAPBLOCKSHIFT;

    x2 -= bmaporgx;
    y2 -= bmaporgy;
    xt2 = x2 >> MAPBLOCKSHIFT;
    yt2 = y2 >> MAPBLOCKSHIFT;

    if (xt2 > xt1)
    {
        mapxstep = 1;
        partial = FRACUNIT - ((x1 >> MAPBTOFRAC) & (FRACUNIT - 1));
        ystep = FixedDiv(y2 - y1, ABS(x2 - x1));
    }
    else if (xt2 < xt1)
    {
        mapxstep = -1;
        partial = (x1 >> MAPBTOFRAC) & (FRACUNIT - 1);
        ystep = FixedDiv(y2 - y1, ABS(x2 - x1));
    }
    else
    {
        mapxstep = 0;
        partial = FRACUNIT;
        ystep = 256 * FRACUNIT;
    }

    yintercept = (y1 >> MAPBTOFRAC) + FixedMul(partial, ystep);

    if (yt2 > yt1)
    {
        mapystep = 1;
        partial = FRACUNIT - ((y1 >> MAPBTOFRAC) & (FRACUNIT - 1));
        xstep = FixedDiv(x2 - x1, ABS(y2 - y1));
    }
    else if (yt2 < yt1)
    {
        mapystep = -1;
        partial = (y1 >> MAPBTOFRAC) & (FRACUNIT - 1);
        xstep = FixedDiv(x2 - x1, ABS(y2 - y1));
    }
    else
    {
        mapystep = 0;
        partial = FRACUNIT;
        xstep = 256 * FRACUNIT;
    }

    xintercept = (x1 >> MAPBTOFRAC) + FixedMul(partial, xstep);

    // Step through map blocks.
    // Count is present to prevent a round off error
    // from skipping the break.
    mapx = xt1;
    mapy = yt1;

    for (count = 0; count < 100; count++)
    {
        if (flags & PT_ADDLINES)
            if (!P_BlockLinesIterator(mapx, mapy, PIT_AddLineIntercepts))
                return false;           // early out

        if (flags & PT_ADDTHINGS)
            if (!P_BlockThingsIterator(mapx, mapy, PIT_AddThingIntercepts))
                return false;           // early out

        if (mapx == xt2 && mapy == yt2)
            break;

        switch ((((yintercept >> FRACBITS) == mapy) << 1) | ((xintercept >> FRACBITS) == mapx))
        {
            case 0:
                count = 100;
                break;

            case 1:
                xintercept += xstep;
                mapy += mapystep;
                break;

            case 2:
                yintercept += ystep;
                mapx += mapxstep;
                break;

            case 3:
                if (flags & PT_ADDLINES)
                {
                    if (!P_BlockLinesIterator(mapx + mapxstep, mapy, PIT_AddLineIntercepts))
                        return false;
                    if (!P_BlockLinesIterator(mapx, mapy + mapystep, PIT_AddLineIntercepts))
                        return false;
                }

                if (flags & PT_ADDTHINGS)
                {
                    if (!P_BlockThingsIterator(mapx + mapxstep, mapy, PIT_AddThingIntercepts))
                        return false;
                    if (!P_BlockThingsIterator(mapx, mapy + mapystep, PIT_AddThingIntercepts))
                        return false;
                }
                xintercept += xstep;
                yintercept += ystep;
                mapx += mapxstep;
                mapy += mapystep;
                break;
        }
    }

    // go through the sorted list
    return P_TraverseIntercepts(trav, FRACUNIT);
}
