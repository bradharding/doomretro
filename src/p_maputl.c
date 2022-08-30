/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

#include "i_system.h"
#include "m_bbox.h"
#include "p_local.h"
#include "p_setup.h"

//
// P_ApproxDistance
// Gives an estimation of distance (not exact)
//
fixed_t P_ApproxDistance(fixed_t dx, fixed_t dy)
{
    dx = ABS(dx);
    dy = ABS(dy);

    return (dx + dy - (MIN(dx, dy) >> 1));
}

//
// P_PointOnLineSide
// Returns 0 or 1
//
int P_PointOnLineSide(const fixed_t x, const fixed_t y, const line_t *line)
{
    return (!line->dx ? (x <= line->v1->x ? line->dy > 0 : line->dy < 0) :
        (!line->dy ? (y <= line->v1->y ? line->dx < 0 : line->dx > 0) :
        ((int64_t)y - line->v1->y) * line->dx >= line->dy * ((int64_t)x - line->v1->x)));
}

//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
// killough 05/03/98: reformatted, cleaned up
//
int P_BoxOnLineSide(const fixed_t *tmbox, const line_t *ld)
{
    switch (ld->slopetype)
    {
        int p;

        default:
        case ST_HORIZONTAL:
            p = (tmbox[BOXTOP] > ld->v1->y);
            return ((tmbox[BOXBOTTOM] > ld->v1->y) == p ? p ^ (ld->dx < 0) : -1);

        case ST_VERTICAL:
            p = (tmbox[BOXRIGHT] < ld->v1->x);
            return ((tmbox[BOXLEFT] < ld->v1->x) == p ? p ^ (ld->dy < 0) : -1);

        case ST_POSITIVE:
            p = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld);
            return (P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld) == p ? p : -1);

        case ST_NEGATIVE:
            p = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld);
            return ((P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld)) == p ? p : -1);
    }
}

//
// P_PointOnDivlineSide
// Returns 0 or 1
//
static int P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t *line)
{
    return (!line->dx ? (x <= line->x ? line->dy > 0 : line->dy < 0) :
        (!line->dy ? (y <= line->y ? line->dx < 0 : line->dx > 0) :
        (line->dy ^ line->dx ^ (x -= line->x) ^ (y -= line->y)) < 0 ? (line->dy ^ x) < 0 :
        FixedMul(y >> 8, line->dx >> 8) >= FixedMul(line->dy >> 8, x >> 8)));
}

//
// P_InterceptVector
// Returns the fractional intercept point along the first divline.
// This is only called by the addthings and addlines traversers.
//
fixed_t P_InterceptVector(divline_t *v2, divline_t *v1)
{
    const int64_t   den = ((int64_t)v1->dy * v2->dx - (int64_t)v1->dx * v2->dy) >> FRACBITS;

    if (!den)
        return 0;

    return (fixed_t)((((int64_t)v1->x - v2->x) * v1->dy - ((int64_t)v1->y - v2->y) * v1->dx) / den);
}

//
// P_LineOpening
// Sets opentop and openbottom to the window through a two sided line.
// OPTIMIZE: keep this precalculated
//
fixed_t opentop;
fixed_t openbottom;
fixed_t openrange;
fixed_t lowfloor;

void P_LineOpening(line_t *line)
{
    sector_t    *front;
    sector_t    *back;
    fixed_t     frontfloor;
    fixed_t     backfloor;

    if (line->sidenum[1] == NO_INDEX)
    {
        // single sided line
        openrange = 0;
        return;
    }

    front = line->frontsector;
    back = line->backsector;
    opentop = MIN(front->ceilingheight, back->ceilingheight);
    frontfloor = front->floorheight;
    backfloor = back->floorheight;

    if (frontfloor > backfloor)
    {
        openbottom = frontfloor;
        lowfloor = backfloor;
    }
    else
    {
        openbottom = backfloor;
        lowfloor = frontfloor;
    }

    openrange = opentop - openbottom;
}

//
// THING POSITION SETTING
//

//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors. On each position change, BLOCKMAP and other
// lookups maintaining lists of things inside these structures need to be updated.
//
void P_UnsetThingPosition(mobj_t *thing)
{
    if (!(thing->flags & MF_NOSECTOR))
    {
        // invisible things don't need to be in sector list
        // unlink from subsector

        // killough 08/11/98: simpler scheme using pointers-to-pointers for prev
        // pointers, allows head node pointers to be treated like everything else
        mobj_t  **sprev = thing->sprev;
        mobj_t  *snext = thing->snext;

        if ((*sprev = snext))                           // unlink from sector list
            snext->sprev = sprev;

        // phares 03/14/98
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
        sector_list = thing->touching_sectorlist;
        thing->touching_sectorlist = NULL;              // to be restored by P_SetThingPosition
    }

    if (!(thing->flags & MF_NOBLOCKMAP))
    {
        // inert things don't need to be in blockmap
        //
        // killough 08/11/98: simpler scheme using pointers-to-pointers for prev
        // pointers, allows head node pointers to be treated like everything else
        //
        // Also more robust, since it doesn't depend on current position for
        // unlinking. Old method required computing head node based on position
        // at time of unlinking, assuming it was the same position as during
        // linking.
        mobj_t  *bnext;
        mobj_t  **bprev = thing->bprev;

        if (bprev && (*bprev = bnext = thing->bnext))   // unlink from block map
            bnext->bprev = bprev;
    }
}

//
// P_UnsetBloodSplatPosition
//
void P_UnsetBloodSplatPosition(bloodsplat_t *splat)
{
    bloodsplat_t    **prev = splat->prev;
    bloodsplat_t    *next = splat->next;

    if ((*prev = next))
        next->prev = prev;

    free(splat);
}

//
// P_SetThingPosition
// Links a thing into both a block and a subsector based on its x y.
// Sets thing->subsector properly
//
void P_SetThingPosition(mobj_t *thing)
{
    // link into subsector
    subsector_t *subsector = thing->subsector = R_PointInSubsector(thing->x, thing->y);

    if (!(thing->flags & MF_NOSECTOR))
    {
        // invisible things don't go into the sector links

        // killough 08/11/98: simpler scheme using pointer-to-pointer prev
        // pointers, allows head nodes to be treated like everything else
        mobj_t  **link = &subsector->sector->thinglist;
        mobj_t  *snext = *link;

        if ((thing->snext = snext))
            snext->sprev = &thing->snext;

        thing->sprev = link;
        *link = thing;

        // phares 03/16/98
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
        sector_list = NULL;                             // clear for next time
    }

    // link into blockmap
    if (!(thing->flags & MF_NOBLOCKMAP))
    {
        // inert things don't need to be in blockmap
        int blockx = P_GetSafeBlockX(thing->x - bmaporgx);
        int blocky = P_GetSafeBlockY(thing->y - bmaporgy);

        if (blockx >= 0 && blockx < bmapwidth && blocky >= 0 && blocky < bmapheight)
        {
            // killough 08/11/98: simpler scheme using pointer-to-pointer prev
            // pointers, allows head nodes to be treated like everything else
            mobj_t  **link = &blocklinks[blocky * bmapwidth + blockx];
            mobj_t  *bnext = *link;

            if ((thing->bnext = bnext))
                bnext->bprev = &thing->bnext;

            thing->bprev = link;
            *link = thing;
        }
        else
        {
            // thing is off the map
            thing->bnext = NULL;
            thing->bprev = NULL;
        }
    }
}

//
// P_SetBloodSplatPosition
//
void P_SetBloodSplatPosition(bloodsplat_t *splat)
{
    bloodsplat_t    **link = &splat->sector->splatlist;
    bloodsplat_t    *next = *link;

    if ((splat->next = next))
        next->prev = &splat->next;

    splat->prev = link;
    *link = splat;
}

//
// BLOCK MAP ITERATORS
// For each line/thing in the given mapblock, call the passed PIT_* function.
// If the function returns false, exit with false without checking anything else.
//

//
// P_BlockLinesIterator
// The validcount flags are used to avoid checking lines that are marked in multiple mapblocks, so
// increment validcount before the first call to P_BlockLinesIterator, then make one or more calls to it.
//
bool P_BlockLinesIterator(const int x, const int y, bool func(line_t *))
{
    if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
        return true;
    else
    {
        const int   *list = &blockmaplump[blockmap[y * bmapwidth + x]];

        if (skipblstart)
            list++;

        for (; *list != -1; list++)
        {
            line_t  *ld = lines + *list;

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
bool P_BlockThingsIterator(const int x, const int y, bool func(mobj_t *))
{
    if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
        return true;

    for (mobj_t *mobj = blocklinks[y * bmapwidth + x]; mobj; mobj = mobj->bnext)
        if (!func(mobj))
            return false;

    if (func == &PIT_RadiusAttack)
        return true;

    // Blockmap bug fix by Terry Hearst

    // (-1, -1)
    if (x > 0 && y > 0)
        for (mobj_t *mobj = blocklinks[(y - 1) * bmapwidth + x - 1]; mobj; mobj = mobj->bnext)
            if (x == (mobj->x + mobj->radius - bmaporgx) >> MAPBLOCKSHIFT
                && y == (mobj->y + mobj->radius - bmaporgy) >> MAPBLOCKSHIFT
                && !func(mobj))
                    return false;

    // (0, -1)
    if (y > 0)
        for (mobj_t *mobj = blocklinks[(y - 1) * bmapwidth + x]; mobj; mobj = mobj->bnext)
            if (y == (mobj->y + mobj->radius - bmaporgy) >> MAPBLOCKSHIFT && !func(mobj))
                    return false;

    // (1, -1)
    if (x < bmapwidth - 1 && y > 0)
        for (mobj_t *mobj = blocklinks[(y - 1) * bmapwidth + x + 1]; mobj; mobj = mobj->bnext)
            if (x == (mobj->x - mobj->radius - bmaporgx) >> MAPBLOCKSHIFT
                && y == (mobj->y + mobj->radius - bmaporgy) >> MAPBLOCKSHIFT
                && !func(mobj))
                    return false;

    // (1, 0)
    if (x < bmapwidth - 1)
        for (mobj_t *mobj = blocklinks[y * bmapwidth + x + 1]; mobj; mobj = mobj->bnext)
            if (x == (mobj->x - mobj->radius - bmaporgx) >> MAPBLOCKSHIFT && !func(mobj))
                    return false;

    // (1, 1)
    if (x < bmapwidth - 1 && y < bmapheight - 1)
        for (mobj_t *mobj = blocklinks[(y + 1) * bmapwidth + x + 1]; mobj; mobj = mobj->bnext)
            if (x == (mobj->x - mobj->radius - bmaporgx) >> MAPBLOCKSHIFT
                && y == (mobj->y - mobj->radius - bmaporgy) >> MAPBLOCKSHIFT
                && !func(mobj))
                    return false;

    // (0, 1)
    if (y < bmapheight - 1)
        for (mobj_t *mobj = blocklinks[(y + 1) * bmapwidth + x]; mobj; mobj = mobj->bnext)
            if (y == (mobj->y - mobj->radius - bmaporgy) >> MAPBLOCKSHIFT && !func(mobj))
                    return false;

    // (-1, 1)
    if (x > 0 && y < bmapheight - 1)
        for (mobj_t *mobj = blocklinks[(y + 1) * bmapwidth + x - 1]; mobj; mobj = mobj->bnext)
            if (x == (mobj->x + mobj->radius - bmaporgx) >> MAPBLOCKSHIFT
                && y == (mobj->y - mobj->radius - bmaporgy) >> MAPBLOCKSHIFT
                && !func(mobj))
                    return false;

    // (-1, 0)
    if (x > 0)
        for (mobj_t *mobj = blocklinks[y * bmapwidth + x - 1]; mobj; mobj = mobj->bnext)
            if (x == (mobj->x + mobj->radius - bmaporgx) >> MAPBLOCKSHIFT && !func(mobj))
                    return false;

    return true;
}

//
// INTERCEPT ROUTINES
//

// killough 01/11/98: Intercept limit removed
static intercept_t  *intercepts;
static intercept_t  *intercept_p;

// Check for limit and double size if necessary -- killough
void P_CheckIntercepts(void)
{
    static size_t   num_intercepts;
    const size_t    offset = intercept_p - intercepts;

    if (offset >= num_intercepts)
    {
        num_intercepts = (num_intercepts ? num_intercepts * 2 : 128);
        intercepts = I_Realloc(intercepts, num_intercepts * sizeof(*intercepts));
        intercept_p = intercepts + offset;
    }
}

divline_t   dltrace;

//
// PIT_AddLineIntercepts
// Looks for lines in the given block that intercept the given trace to add to the intercepts list.
// A line is crossed if its endpoints are on opposite sides of the trace.
//
static bool PIT_AddLineIntercepts(line_t *ld)
{
    int         s1;
    int         s2;
    fixed_t     frac;
    divline_t   dl;

    // avoid precision problems with two routines
    if (dltrace.dx < -16 * FRACUNIT || dltrace.dx > 16 * FRACUNIT || dltrace.dy < -16 * FRACUNIT || dltrace.dy > 16 * FRACUNIT)
    {
        s1 = P_PointOnDivlineSide(ld->v1->x, ld->v1->y, &dltrace);
        s2 = P_PointOnDivlineSide(ld->v2->x, ld->v2->y, &dltrace);
    }
    else
    {
        s1 = P_PointOnLineSide(dltrace.x, dltrace.y, ld);
        s2 = P_PointOnLineSide(dltrace.x + dltrace.dx, dltrace.y + dltrace.dy, ld);
    }

    if (s1 == s2)
        return true;        // line isn't crossed

    // hit the line
    dl.x = ld->v1->x;
    dl.y = ld->v1->y;
    dl.dx = ld->dx;
    dl.dy = ld->dy;

    if ((frac = P_InterceptVector(&dltrace, &dl)) < 0)
        return true;        // behind source

    P_CheckIntercepts();    // killough

    intercept_p->frac = frac;
    intercept_p->isaline = true;
    intercept_p->d.line = ld;
    intercept_p++;

    return true;            // continue
}

//
// PIT_AddThingIntercepts
//
static bool PIT_AddThingIntercepts(mobj_t *thing)
{
    int             numfronts = 0;
    divline_t       dl;
    const fixed_t   radius = thing->radius;
    const fixed_t   x = thing->x;
    const fixed_t   y = thing->y;

    // [RH] Don't check a corner to corner crosssection for hit.
    // Instead, check against the actual bounding box.

    // There's probably a smarter way to determine which two sides
    // of the thing face the trace than by trying all four sides...
    for (int i = 0; i < 4; i++)
    {
        switch (i)
        {
            case 0:     // Top edge
                dl.x = x + radius;
                dl.y = y + radius;
                dl.dx = -radius * 2;
                dl.dy = 0;
                break;

            case 1:     // Right edge
                dl.x = x + radius;
                dl.y = y - radius;
                dl.dx = 0;
                dl.dy = radius * 2;
                break;

            case 2:     // Bottom edge
                dl.x = x - radius;
                dl.y = y - radius;
                dl.dx = radius * 2;
                dl.dy = 0;
                break;

            case 3:     // Left edge
                dl.x = x - radius;
                dl.y = y + radius;
                dl.dx = 0;
                dl.dy = radius * -2;
                break;
        }

        // Check if this side is facing the trace origin
        if (!P_PointOnDivlineSide(dltrace.x, dltrace.y, &dl))
        {
            numfronts++;

            // If it is, see if the trace crosses it
            if (P_PointOnDivlineSide(dl.x, dl.y, &dltrace) != P_PointOnDivlineSide(dl.x + dl.dx, dl.y + dl.dy, &dltrace))
            {
                // It's a hit
                fixed_t frac = P_InterceptVector(&dltrace, &dl);

                // behind source
                if (frac < 0)
                    continue;

                P_CheckIntercepts();    // killough

                intercept_p->frac = frac;
                intercept_p->isaline = false;
                intercept_p->d.thing = thing;
                intercept_p++;

                continue;
            }
        }
    }

    // If none of the sides was facing the trace, then the trace
    // must have started inside the box, so add it as an intercept.
    if (!numfronts)
    {
        P_CheckIntercepts();    // killough

        intercept_p->frac = 0;
        intercept_p->isaline = false;
        intercept_p->d.thing = thing;
        intercept_p++;
    }

    return true;
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true for all lines.
//
static bool P_TraverseIntercepts(traverser_t func, const fixed_t maxfrac)
{
    size_t      count = intercept_p - intercepts;
    intercept_t *in = NULL;

    while (count--)
    {
        fixed_t dist = FIXED_MAX;

        for (intercept_t *scan = intercepts; scan < intercept_p; scan++)
            if (scan->frac < dist)
            {
                dist = scan->frac;
                in = scan;
            }

        if (dist > maxfrac)
            return true;        // checked everything in range

        if (!func(in))
            return false;       // don't bother going farther

        in->frac = FIXED_MAX;
    }

    return true;                // everything was traversed
}

//
// P_PathTraverse
// Traces a line from (x1,y1) to (x2,y2), calling the traverser function for each.
// Returns true if the traverser function returns true for all lines.
//
#define MAX_SIGHT_COUNT 64

bool P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, const int flags, traverser_t trav)
{
    fixed_t xt1, yt1;
    fixed_t xt2, yt2;
    int64_t _x1, _y1;
    int64_t _x2, _y2;
    fixed_t xstep = 256 * FRACUNIT, ystep = 256 * FRACUNIT;
    fixed_t partial = FRACUNIT;
    fixed_t xintercept, yintercept;
    int     mapx, mapy;
    int     mapx1, mapy1;
    int     mapxstep = 0, mapystep = 0;

    validcount++;
    intercept_p = intercepts;

    if (!((x1 - bmaporgx) & (MAPBLOCKSIZE - 1)))
        x1 += FRACUNIT;         // don't side exactly on a line

    if (!((y1 - bmaporgy) & (MAPBLOCKSIZE - 1)))
        y1 += FRACUNIT;         // don't side exactly on a line

    dltrace.x = x1;
    dltrace.y = y1;
    dltrace.dx = x2 - x1;
    dltrace.dy = y2 - y1;

    _x1 = (int64_t)x1 - bmaporgx;
    _y1 = (int64_t)y1 - bmaporgy;
    xt1 = (int)(_x1 >> MAPBLOCKSHIFT);
    yt1 = (int)(_y1 >> MAPBLOCKSHIFT);

    mapx1 = (int)(_x1 >> MAPBTOFRAC);
    mapy1 = (int)(_y1 >> MAPBTOFRAC);

    _x2 = (int64_t)x2 - bmaporgx;
    _y2 = (int64_t)y2 - bmaporgy;
    xt2 = (int)(_x2 >> MAPBLOCKSHIFT);
    yt2 = (int)(_y2 >> MAPBLOCKSHIFT);

    x1 -= bmaporgx;
    y1 -= bmaporgy;
    x2 -= bmaporgx;
    y2 -= bmaporgy;

    if (xt2 > xt1)
    {
        mapxstep = 1;
        partial = FRACUNIT - (mapx1 & (FRACUNIT - 1));
        ystep = FixedDiv(y2 - y1, ABS(x2 - x1));
    }
    else if (xt2 < xt1)
    {
        mapxstep = -1;
        partial = (mapx1 & (FRACUNIT - 1));
        ystep = FixedDiv(y2 - y1, ABS(x2 - x1));
    }

    yintercept = mapy1 + FixedMul(partial, ystep);

    if (yt2 > yt1)
    {
        mapystep = 1;
        partial = FRACUNIT - (mapy1 & (FRACUNIT - 1));
        xstep = FixedDiv(x2 - x1, ABS(y2 - y1));
    }
    else if (yt2 < yt1)
    {
        mapystep = -1;
        partial = (mapy1 & (FRACUNIT - 1));
        xstep = FixedDiv(x2 - x1, ABS(y2 - y1));
    }

    xintercept = mapx1 + FixedMul(partial, xstep);

    // Step through map blocks.
    // Count is present to prevent a round off error from skipping the break.
    mapx = xt1;
    mapy = yt1;

    for (int count = 0; count < MAX_SIGHT_COUNT; count++)
    {
        if (flags & PT_ADDLINES)
            if (!P_BlockLinesIterator(mapx, mapy, &PIT_AddLineIntercepts))
                return false;   // early out

        if (flags & PT_ADDTHINGS)
            if (!P_BlockThingsIterator(mapx, mapy, &PIT_AddThingIntercepts))
                return false;   // early out

        if (mapx == xt2 && mapy == yt2)
            break;

        // [RH] Handle corner cases properly instead of pretending they don't exist.
        switch ((((yintercept >> FRACBITS) == mapy) << 1) | ((xintercept >> FRACBITS) == mapx))
        {
            case 0:
                // neither xintercept nor yintercept match!
                count = MAX_SIGHT_COUNT;
                break;

            case 1:
                // xintercept matches
                xintercept += xstep;
                mapy += mapystep;
                break;

            case 2:
                // yintercept matches
                yintercept += ystep;
                mapx += mapxstep;
                break;

            case 3:
                // xintercept and yintercept both match
                // The trace is exiting a block through its corner. Not only does the block
                // being entered need to be checked (which will happen when this loop
                // continues), but the other two blocks adjacent to the corner also need to
                // be checked.
                if (flags & PT_ADDLINES)
                {
                    P_BlockLinesIterator(mapx + mapxstep, mapy, &PIT_AddLineIntercepts);
                    P_BlockLinesIterator(mapx, mapy + mapystep, &PIT_AddLineIntercepts);
                }

                if (flags & PT_ADDTHINGS)
                {
                    P_BlockThingsIterator(mapx + mapxstep, mapy, &PIT_AddThingIntercepts);
                    P_BlockThingsIterator(mapx, mapy + mapystep, &PIT_AddThingIntercepts);
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

// MAES: support 512x512 blockmaps.
int P_GetSafeBlockX(int coord)
{
    // If x is LE than those special values, interpret as positive.
    // Otherwise, leave it as it is.
    if ((coord >>= MAPBLOCKSHIFT) <= blockmapxneg)
        return (coord & 0x01FF);    // Broke width boundary

    return coord;
}

// MAES: support 512x512 blockmaps.
int P_GetSafeBlockY(int coord)
{
    // If y is LE than those special values, interpret as positive.
    // Otherwise, leave it as it is.
    if ((coord >>= MAPBLOCKSHIFT) <= blockmapyneg)
        return (coord & 0x01FF);    // Broke height boundary

    return coord;
}

//
// MBF21: RoughBlockCheck
// [XA] adapted from Hexen -- used by P_RoughTargetSearch
//
static mobj_t *RoughBlockCheck(mobj_t *mo, const int index, const angle_t fov)
{
    mobj_t  *link = blocklinks[index];

    while (link)
    {
        // skip non-shootable actors
        if (!(link->flags & MF_SHOOTABLE))
        {
            link = link->bnext;
            continue;
        }

        // skip the projectile's owner
        if (link == mo->target)
        {
            link = link->bnext;
            continue;
        }

        // skip actors on the same "team", unless infighting
        if (mo->target && !((link->flags ^ mo->target->flags) & MF_FRIEND)
            && mo->target->target != link && !(link->player && mo->target->player))
        {
            link = link->bnext;
            continue;
        }

        // skip actors outside of specified FOV
        if (fov > 0 && !P_CheckFOV(mo, link, fov))
        {
            link = link->bnext;
            continue;
        }

        // skip actors not in line of sight
        if (!P_CheckSight(mo, link))
        {
            link = link->bnext;
            continue;
        }

        // all good! return it.
        return link;
    }

    // couldn't find a valid target
    return NULL;
}

//
// MBF21: P_RoughTargetSearch
// Searches though the surrounding mapblocks for monsters/players
// based on Hexen's P_RoughMonsterSearch
//
// distance is in MAPBLOCKUNITS
mobj_t *P_RoughTargetSearch(mobj_t *mo, const angle_t fov, const int distance)
{
    const int   startx = (mo->x - bmaporgx) >> MAPBLOCKSHIFT;
    const int   starty = (mo->y - bmaporgy) >> MAPBLOCKSHIFT;
    mobj_t      *target;

    if (startx >= 0 && startx < bmapwidth && starty >= 0 && starty < bmapheight
        && (target = RoughBlockCheck(mo, starty * bmapwidth + startx, fov)))
        return target;  // found a target right away

    for (int count = 1; count <= distance; count++)
    {
        int blockx, blocky;
        int blockindex;
        int firststop = startx + count;
        int secondstop;
        int thirdstop;
        int finalstop;

        if (firststop < 0)
            continue;

        if ((secondstop = starty + count) < 0)
            continue;

        if (firststop >= bmapwidth)
            firststop = bmapwidth - 1;

        if (secondstop >= bmapheight)
            secondstop = bmapheight - 1;

        blockx = BETWEEN(0, startx - count, bmapwidth - 1);
        blocky = BETWEEN(0, starty - count, bmapheight - 1);
        blockindex = blocky * bmapwidth + blockx;

        thirdstop = secondstop * bmapwidth + blockx;
        secondstop = secondstop * bmapwidth + firststop;
        firststop += blocky * bmapwidth;
        finalstop = blockindex;

        // Trace the first block section (along the top)
        for (; blockindex <= firststop; blockindex++)
            if ((target = RoughBlockCheck(mo, blockindex, fov)))
                return target;

        // Trace the second block section (right edge)
        for (blockindex--; blockindex <= secondstop; blockindex += bmapwidth)
            if ((target = RoughBlockCheck(mo, blockindex, fov)))
                return target;

        // Trace the third block section (bottom edge)
        for (blockindex -= bmapwidth; blockindex >= thirdstop; blockindex--)
            if ((target = RoughBlockCheck(mo, blockindex, fov)))
                return target;

        // Trace the final block section (left edge)
        for (blockindex++; blockindex > finalstop; blockindex -= bmapwidth)
            if ((target = RoughBlockCheck(mo, blockindex, fov)))
                return target;
    }

    return NULL;
}
