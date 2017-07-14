/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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

#include <stdlib.h>
#include <string.h>

#include "doomstat.h"
#include "m_bbox.h"
#include "r_main.h"
#include "r_plane.h"
#include "r_things.h"

seg_t           *curline;
side_t          *sidedef;
line_t          *linedef;
sector_t        *frontsector;
sector_t        *backsector;

dboolean        doorclosed;

drawseg_t       *drawsegs;
unsigned int    maxdrawsegs;
drawseg_t       *ds_p;

void R_StoreWallRange(int start, int stop);

//
// R_ClearDrawSegs
//
void R_ClearDrawSegs(void)
{
    ds_p = drawsegs;
}

//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
typedef struct cliprange_s
{
    int first;
    int last;
} cliprange_t;

// 1/11/98: Lee Killough
//
// This fixes many strange venetian blinds crashes, which occurred when a scan
// line had too many "posts" of alternating non-transparent and transparent
// regions. Using a doubly-linked list to represent the posts is one way to
// do it, but it has increased overhead and poor spatial locality, which hurts
// cache performance on modern machines. Since the maximum number of posts
// theoretically possible is a function of screen width, a static limit is
// okay in this case. It used to be 32, which was way too small.
//
// This limit was frequently mistaken for the visplane limit in some DOOM
// editing FAQs, where visplanes were said to "double" if a pillar or other
// object split the view's space into two pieces horizontally. That did not
// have anything to do with visplanes, but it had everything to do with these
// clip posts.

#define MAXSEGS (SCREENWIDTH / 2 + 1)

// newend is one past the last valid seg
static cliprange_t  *newend;
static cliprange_t  solidsegs[MAXSEGS];

//
// R_ClipSolidWallSegment
// Does handle solid walls,
//  e.g. single sided LineDefs (middle texture)
//  that entirely block the view.
//
static void R_ClipSolidWallSegment(int first, int last)
{
    cliprange_t *next;
    cliprange_t *start = solidsegs;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    while (start->last < first - 1)
        start++;

    if (first < start->first)
    {
        if (last < start->first - 1)
        {
            // Post is entirely visible (above start), so insert a new clippost.
            R_StoreWallRange(first, last);

            // 1/11/98 killough: performance tuning using fast memmove
            memmove(start + 1, start, (++newend - start) * sizeof(*start));
            start->first = first;
            start->last = last;
            return;
        }

        // There is a fragment above *start.
        R_StoreWallRange(first, start->first - 1);

        // Now adjust the clip size.
        start->first = first;
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;

    next = start;

    while (last >= (next + 1)->first - 1)
    {
        // There is a fragment between two posts.
        R_StoreWallRange(next->last + 1, (next + 1)->first - 1);
        next++;

        if (last <= next->last)
        {
            // Bottom is contained in next. Adjust the clip size.
            start->last = next->last;
            goto crunch;
        }
    }

    // There is a fragment after *next.
    R_StoreWallRange(next->last + 1, last);

    // Adjust the clip size.
    start->last = last;

    // Remove start + 1 to next from the clip list,
    // because start now covers their area.
crunch:
    if (next == start)
        return;                 // Post just extended past the bottom of one post.

    while (next++ != newend)
        *++start = *next;       // Remove a post.

    newend = start + 1;
}

//
// R_ClipPassWallSegment
// Clips the given range of columns,
//  but does not includes it in the clip list.
// Does handle windows,
//  e.g. LineDefs with upper and lower texture.
//
static void R_ClipPassWallSegment(int first, int last)
{
    cliprange_t *start = solidsegs;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    while (start->last < first - 1)
        start++;

    if (first < start->first)
    {
        if (last < start->first - 1)
        {
            // Post is entirely visible (above start).
            R_StoreWallRange(first, last);
            return;
        }

        // There is a fragment above *start.
        R_StoreWallRange(first, start->first - 1);
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;

    while (last >= (start + 1)->first - 1)
    {
        // There is a fragment between two posts.
        R_StoreWallRange(start->last + 1, (start + 1)->first - 1);
        start++;

        if (last <= start->last)
            return;
    }

    // There is a fragment after *next.
    R_StoreWallRange(start->last + 1, last);
}

//
// R_ClearClipSegs
//
void R_ClearClipSegs(void)
{
    solidsegs[0].first = INT_MIN + 1;
    solidsegs[0].last = -1;
    solidsegs[1].first = viewwidth;
    solidsegs[1].last = INT_MAX - 1;
    newend = solidsegs + 2;
}

// killough 1/18/98 -- This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// It assumes that DOOM has already ruled out a door being closed because
// of front-back closure (e.g. front floor is taller than back ceiling).
static dboolean R_DoorClosed(void)
{
    return
        // if door is closed because back is shut:
        (backsector->interpceilingheight <= backsector->interpfloorheight

            // preserve a kind of transparent door/lift special effect:
            && (backsector->interpceilingheight >= frontsector->interpceilingheight
                || curline->sidedef->toptexture)
            && (backsector->interpfloorheight <= frontsector->interpfloorheight
                || curline->sidedef->bottomtexture)

            // properly render skies (consider door "open" if both ceilings are sky):
            && (backsector->ceilingpic != skyflatnum || frontsector->ceilingpic != skyflatnum));
}

// [AM] Interpolate the passed sector, if prudent.
static void R_MaybeInterpolateSector(sector_t *sector)
{
    if (vid_capfps != TICRATE
        // Only if we moved the sector last tic.
        && sector->oldgametic == gametic - 1)
    {
        // Interpolate between current and last floor/ceiling position.
        if (sector->floorheight != sector->oldfloorheight)
            sector->interpfloorheight = sector->oldfloorheight
                + FixedMul(sector->floorheight - sector->oldfloorheight, fractionaltic);
        else
            sector->interpfloorheight = sector->floorheight;

        if (sector->ceilingheight != sector->oldceilingheight)
            sector->interpceilingheight = sector->oldceilingheight
                + FixedMul(sector->ceilingheight - sector->oldceilingheight, fractionaltic);
        else
            sector->interpceilingheight = sector->ceilingheight;
    }
    else
    {
        sector->interpfloorheight = sector->floorheight;
        sector->interpceilingheight = sector->ceilingheight;
    }
}

//
// killough 3/7/98: Hack floor/ceiling heights for deep water etc.
//
// If player's view height is underneath fake floor, lower the
// drawn ceiling to be just under the floor height, and replace
// the drawn floor and ceiling textures, and light level, with
// the control sector's.
//
// Similar for ceiling, only reflected.
//
// killough 4/11/98, 4/13/98: fix bugs, add 'back' parameter
//
sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec, int *floorlightlevel, int *ceilinglightlevel,
    dboolean back)
{
    if (floorlightlevel)
        *floorlightlevel = (sec->floorlightsec == -1 ? sec->lightlevel :
            sectors[sec->floorlightsec].lightlevel);

    if (ceilinglightlevel)
        *ceilinglightlevel = (sec->ceilinglightsec == -1 ? sec->lightlevel :
            sectors[sec->ceilinglightsec].lightlevel);

    if (sec->heightsec != -1)
    {
        const sector_t  *s = &sectors[sec->heightsec];
        int             heightsec = viewplayer->mo->subsector->sector->heightsec;
        int             underwater = (heightsec != -1 && viewz <= sectors[heightsec].interpfloorheight);

        // Replace sector being drawn, with a copy to be hacked
        *tempsec = *sec;

        // Replace floor and ceiling height with other sector's heights.
        tempsec->interpfloorheight = s->interpfloorheight;
        tempsec->interpceilingheight = s->interpceilingheight;

        // killough 11/98: prevent sudden light changes from non-water sectors:
        if (underwater && (tempsec->interpfloorheight = sec->interpfloorheight,
            tempsec->interpceilingheight = s->interpfloorheight - 1, !back))
        {
            // head-below-floor hack
            tempsec->floorpic = s->floorpic;
            tempsec->floor_xoffs = s->floor_xoffs;
            tempsec->floor_yoffs = s->floor_yoffs;

            if (underwater)
            {
                if (s->ceilingpic == skyflatnum)
                {
                    tempsec->interpfloorheight = tempsec->interpceilingheight + 1;
                    tempsec->ceilingpic = tempsec->floorpic;
                    tempsec->ceiling_xoffs = tempsec->floor_xoffs;
                    tempsec->ceiling_yoffs = tempsec->floor_yoffs;
                }
                else
                {
                    tempsec->ceilingpic = s->ceilingpic;
                    tempsec->ceiling_xoffs = s->ceiling_xoffs;
                    tempsec->ceiling_yoffs = s->ceiling_yoffs;
                }
            }

            tempsec->lightlevel = s->lightlevel;

            if (floorlightlevel)
                *floorlightlevel = (s->floorlightsec == -1 ? s->lightlevel :
                    sectors[s->floorlightsec].lightlevel);              // killough 3/16/98

            if (ceilinglightlevel)
                *ceilinglightlevel = (s->ceilinglightsec == -1 ? s->lightlevel :
                    sectors[s->ceilinglightsec].lightlevel);            // killough 4/11/98
        }
        else if (heightsec != -1 && viewz >= sectors[heightsec].interpceilingheight
            && sec->interpceilingheight > s->interpceilingheight)
        {
            // Above-ceiling hack
            tempsec->interpceilingheight = s->interpceilingheight;
            tempsec->interpfloorheight = s->interpceilingheight + 1;

            tempsec->floorpic = tempsec->ceilingpic = s->ceilingpic;
            tempsec->floor_xoffs = tempsec->ceiling_xoffs = s->ceiling_xoffs;
            tempsec->floor_yoffs = tempsec->ceiling_yoffs = s->ceiling_yoffs;

            if (s->floorpic != skyflatnum)
            {
                tempsec->interpceilingheight = sec->interpceilingheight;
                tempsec->floorpic = s->floorpic;
                tempsec->floor_xoffs = s->floor_xoffs;
                tempsec->floor_yoffs = s->floor_yoffs;
            }

            tempsec->lightlevel = s->lightlevel;

            if (floorlightlevel)
                *floorlightlevel = (s->floorlightsec == -1 ? s->lightlevel :
                    sectors[s->floorlightsec].lightlevel);              // killough 3/16/98

            if (ceilinglightlevel)
                *ceilinglightlevel = (s->ceilinglightsec == -1 ? s->lightlevel :
                    sectors[s->ceilinglightsec].lightlevel);            // killough 4/11/98
        }

        sec = tempsec;        // Use other sector
    }

    return sec;
}

//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//
static void R_AddLine(seg_t *line)
{
    int             x1;
    int             x2;
    angle_t         angle1 = R_PointToAngleEx(line->v1->x, line->v1->y);
    angle_t         angle2 = R_PointToAngleEx(line->v2->x, line->v2->y);
    static sector_t tempsec;        // killough 3/8/98: ceiling/water hack

    curline = line;

    // Back side? I.e. backface culling?
    if (angle1 - angle2 >= ANG180)
        return;

    // Global angle needed by segcalc.
    angle1 -= viewangle;
    angle2 -= viewangle;

    if ((signed int)angle1 < (signed int)angle2)
    {
        // Either angle1 or angle2 is behind us, so it doesn't matter if we
        // change it to the correct sign
        if (angle1 >= ANG180 && angle1 < ANG270)
            angle1 = INT_MAX;           // which is ANG180 - 1
        else
            angle2 = INT_MIN;
    }

    if ((signed int)angle2 >= (signed int)clipangle)
        return;                         // Both off left edge

    if ((signed int)angle1 <= -(signed int)clipangle)
        return;                         // Both off right edge

    if ((signed int)angle1 >= (signed int)clipangle)
        angle1 = clipangle;             // Clip at left edge

    if ((signed int)angle2 <= -(signed int)clipangle)
        angle2 = 0 - clipangle;         // Clip at right edge

    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
    angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;

    // killough 1/31/98: Here is where "slime trails" can SOMETIMES occur:
    x1 = viewangletox[angle1];
    x2 = viewangletox[angle2];

    // Does not cross a pixel?
    if (x1 >= x2)
        return;

    // Single sided line?
    if (!(backsector = line->backsector))
        goto clipsolid;

    // [AM] Interpolate sector movement before
    //      running clipping tests. Frontsector
    //      should already be interpolated.
    R_MaybeInterpolateSector(backsector);

    // killough 3/8/98, 4/4/98: hack for invisible ceilings / deep water
    backsector = R_FakeFlat(backsector, &tempsec, NULL, NULL, true);

    doorclosed = false; // killough 4/16/98

    // Closed door.
    if (backsector->interpceilingheight <= frontsector->interpfloorheight
        || backsector->interpfloorheight >= frontsector->interpceilingheight)
        goto clipsolid;

    // This fixes the automap floor height bug -- killough 1/18/98:
    // killough 4/7/98: optimize: save result in doorclosed for use in r_segs.c
    if ((doorclosed = R_DoorClosed()))
        goto clipsolid;

    // Window.
    if (backsector->interpceilingheight != frontsector->interpceilingheight
        || backsector->interpfloorheight != frontsector->interpfloorheight)
        goto clippass;

    // Reject empty lines used for triggers
    //  and special events.
    // Identical floor and ceiling on both sides,
    // identical light levels on both sides,
    // and no middle texture.
    if (backsector->ceilingpic == frontsector->ceilingpic
        && backsector->floorpic == frontsector->floorpic
        && backsector->lightlevel == frontsector->lightlevel
        && !curline->sidedef->midtexture

        // killough 3/7/98: Take flats offsets into account:
        && backsector->floor_xoffs == frontsector->floor_xoffs
        && backsector->floor_yoffs == frontsector->floor_yoffs
        && backsector->ceiling_xoffs == frontsector->ceiling_xoffs
        && backsector->ceiling_yoffs == frontsector->ceiling_yoffs

        // killough 4/16/98: consider altered lighting
        && backsector->floorlightsec == frontsector->floorlightsec
        && backsector->ceilinglightsec == frontsector->ceilinglightsec)
        return;

clippass:
    R_ClipPassWallSegment(x1, x2 - 1);
    return;

clipsolid:
    R_ClipSolidWallSegment(x1, x2 - 1);
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
static dboolean R_CheckBBox(const fixed_t *bspcoord)
{
    const int checkcoord[12][4] =
    {
        { 3, 0, 2, 1 },
        { 3, 0, 2, 0 },
        { 3, 1, 2, 0 },
        { 0 },
        { 2, 0, 2, 1 },
        { 0, 0, 0, 0 },
        { 3, 1, 3, 0 },
        { 0 },
        { 2, 0, 3, 1 },
        { 2, 1, 3, 1 },
        { 2, 1, 3, 0 }
    };

    int         boxpos;
    const int   *check;

    angle_t     angle1;
    angle_t     angle2;

    cliprange_t *start;

    int         sx1;
    int         sx2;

    // Find the corners of the box
    // that define the edges from current viewpoint.
    boxpos = (viewx <= bspcoord[BOXLEFT] ? 0 : viewx < bspcoord[BOXRIGHT] ? 1 : 2) +
        (viewy >= bspcoord[BOXTOP] ? 0 : viewy > bspcoord[BOXBOTTOM] ? 4 : 8);

    if (boxpos == 5)
        return true;

    check = checkcoord[boxpos];

    // check clip list for an open space
    angle1 = R_PointToAngle(bspcoord[check[0]], bspcoord[check[1]]) - viewangle;
    angle2 = R_PointToAngle(bspcoord[check[2]], bspcoord[check[3]]) - viewangle;

    // cph - replaced old code, which was unclear and badly commented
    // Much more efficient code now
    if ((signed int)angle1 < (signed int)angle2)
    {
        // Either angle1 or angle2 is behind us, so it doesn't matter if we
        // change it to the correct sign
        if (angle1 >= ANG180 && angle1 < ANG270)
            angle1 = INT_MAX;           // which is ANG180 - 1
        else
            angle2 = INT_MIN;
    }

    if ((signed int)angle2 >= (signed int)clipangle)
        return false;                   // Both off left edge

    if ((signed int)angle1 <= -(signed int)clipangle)
        return false;                   // Both off right edge

    if ((signed int)angle1 >= (signed int)clipangle)
        angle1 = clipangle;             // Clip at left edge

    if ((signed int)angle2 <= -(signed int)clipangle)
        angle2 = 0 - clipangle;         // Clip at right edge

    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    sx1 = viewangletox[(angle1 + ANG90) >> ANGLETOFINESHIFT];
    sx2 = viewangletox[(angle2 + ANG90) >> ANGLETOFINESHIFT];

    if (sx1 > 0)
        sx1--;

    if (sx2 < viewwidth - 1)
        sx2++;

    start = solidsegs;

    while (start->last < sx2)
        start++;

    if (sx1 >= start->first && sx2 <= start->last)
        return false;                   // The clippost contains the new span.

    return true;
}

//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//
static void R_Subsector(int num)
{
    subsector_t *sub = &subsectors[num];
    sector_t    tempsec;              // killough 3/7/98: deep water hack
    int         floorlightlevel;      // killough 3/16/98: set floor lightlevel
    int         ceilinglightlevel;    // killough 4/11/98
    int         count = sub->numlines;
    seg_t       *line = &segs[sub->firstline];

    frontsector = sub->sector;

    // [AM] Interpolate sector movement. Usually only needed
    //      when you're standing inside the sector.
    R_MaybeInterpolateSector(frontsector);

    // killough 3/8/98, 4/4/98: Deep water / fake ceiling effect
    frontsector = R_FakeFlat(frontsector, &tempsec, &floorlightlevel, &ceilinglightlevel, false);

    floorplane = (frontsector->interpfloorheight < viewz        // killough 3/7/98
        || (frontsector->heightsec != -1 && sectors[frontsector->heightsec].ceilingpic == skyflatnum) ?
        R_FindPlane(frontsector->interpfloorheight,
            (frontsector->floorpic == skyflatnum                // killough 10/98
                && (frontsector->sky & PL_SKYFLAT) ? frontsector->sky : frontsector->floorpic),
            floorlightlevel,                                    // killough 3/16/98
            frontsector->floor_xoffs,                           // killough 3/7/98
            frontsector->floor_yoffs) : NULL);

    ceilingplane = (frontsector->interpceilingheight > viewz || frontsector->ceilingpic == skyflatnum
        || (frontsector->heightsec != -1 && sectors[frontsector->heightsec].floorpic == skyflatnum) ?
        R_FindPlane(frontsector->interpceilingheight,           // killough 3/8/98
            (frontsector->ceilingpic == skyflatnum              // killough 10/98
                && (frontsector->sky & PL_SKYFLAT) ? frontsector->sky : frontsector->ceilingpic),
            ceilinglightlevel,                                  // killough 4/11/98
            frontsector->ceiling_xoffs,                         // killough 3/7/98
            frontsector->ceiling_yoffs) : NULL);

    // killough 9/18/98: Fix underwater slowdown, by passing real sector
    // instead of fake one. Improve sprite lighting by basing sprite
    // lightlevels on floor & ceiling lightlevels in the surrounding area.
    //
    // 10/98 killough:
    //
    // NOTE: TeamTNT fixed this bug incorrectly, messing up sprite lighting!!!
    // That is part of the 242 effect!!! If you simply pass sub->sector to
    // the old code you will not get correct lighting for underwater sprites!!!
    // Either you must pass the fake sector and handle validcount here, on the
    // real sector, or you must account for the lighting in some other way,
    // like passing it as an argument.
    if (sub->sector->validcount != validcount)
    {
        sub->sector->validcount = validcount;
        R_AddSprites(sub->sector, (frontsector->ceilingpic == skyflatnum
            && !(frontsector->sky & PL_SKYFLAT) ? (ceilinglightlevel + floorlightlevel) / 2 :
            floorlightlevel));
    }

    while (count--)
        R_AddLine(line++);
}

//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
void R_RenderBSPNode(int bspnum)
{
    while (!(bspnum & NF_SUBSECTOR))    // Found a subsector?
    {
        const node_t    *bsp = &nodes[bspnum];

        // Decide which side the view point is on.
        int             side = R_PointOnSide(viewx, viewy, bsp);

        // Recursively divide front space.
        R_RenderBSPNode(bsp->children[side]);

        // Possibly divide back space.
        if (!R_CheckBBox(bsp->bbox[side ^= 1]))
            return;

        bspnum = bsp->children[side];
    }

    R_Subsector(bspnum == -1 ? 0 : (bspnum & ~NF_SUBSECTOR));
}
