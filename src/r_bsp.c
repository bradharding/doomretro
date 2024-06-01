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

#include <stdlib.h>
#include <string.h>

#include "doomstat.h"
#include "m_bbox.h"
#include "m_config.h"
#include "r_plane.h"
#include "r_segs.h"
#include "r_things.h"

seg_t       *curline;
line_t      *linedef;
sector_t    *frontsector;
sector_t    *backsector;

drawseg_t   *drawsegs;
drawseg_t   *ds_p;

//
// R_ClearDrawSegs
//
void R_ClearDrawSegs(void)
{
    ds_p = drawsegs;
}

// CPhipps -
// Instead of clipsegs, let's try using an array with one entry for each column,
// indicating whether it's blocked by a solid wall yet or not.
static int  memcmpsize;
byte        *solidcol;

// CPhipps -
// R_ClipWallSegment
//
// Replaces the old R_Clip*WallSegment functions. It draws bits of walls in those
// columns which aren't solid, and updates the solidcol[] array appropriately
static void R_ClipWallSegment(int first, const int last, const bool solid)
{
    while (first < last)
        if (solidcol[first])
        {
            const byte  *p = memchr(solidcol + first, 0, (size_t)last - first);

            if (!p)
                return;

            first = (int)(p - solidcol);
        }
        else
        {
            const byte  *p = memchr(solidcol + first, 1, (size_t)last - first);
            const int   to = (p ? (int)(p - solidcol) : last);

            R_StoreWallRange(first, to - 1);

            if (solid)
                memset(solidcol + first, 1, (size_t)to - first);

            first = to;
        }
}

//
// R_InitClipSegs
//
void R_InitClipSegs(void)
{
    memcmpsize = sizeof(frontsector->floorxoffset) + sizeof(frontsector->flooryoffset)
        + sizeof(frontsector->ceilingxoffset) + sizeof(frontsector->ceilingyoffset)
        + sizeof(*frontsector->floorlightsec) + sizeof(*frontsector->ceilinglightsec)
        + sizeof(frontsector->floorpic) + sizeof(frontsector->ceilingpic)
        + sizeof(frontsector->lightlevel);
    solidcol = calloc(MAXWIDTH, sizeof(*solidcol));
}

//
// R_ClearClipSegs
//
void R_ClearClipSegs(void)
{
    memset(solidcol, 0, MAXWIDTH);
}

// killough 01/18/98 -- This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// cph - converted to R_RecalcLineFlags. This recalculates all the flags for
// a line, including closure and texture tiling.
static void R_RecalcLineFlags(line_t *line)
{
    int c;

    line->r_validcount = gametime;

    if (!(line->flags & ML_TWOSIDED)
        || backsector->interpceilingheight <= frontsector->interpfloorheight
        || backsector->interpfloorheight >= frontsector->interpceilingheight
        || (backsector->interpceilingheight <= backsector->interpfloorheight
            && (backsector->interpceilingheight >= frontsector->interpceilingheight
                || curline->sidedef->toptexture)
            && (backsector->interpfloorheight <= frontsector->interpfloorheight
                || curline->sidedef->bottomtexture)
            && (backsector->ceilingpic != skyflatnum
                || frontsector->ceilingpic != skyflatnum)))
        line->r_flags = RF_CLOSED;
    else
    {
        if (backsector->interpceilingheight != frontsector->interpceilingheight
            || backsector->interpfloorheight != frontsector->interpfloorheight
            || curline->sidedef->midtexture
            || memcmp(&backsector->floorxoffset, &frontsector->floorxoffset, memcmpsize))
        {
            line->r_flags = RF_NONE;
            return;
        }
        else
            line->r_flags = RF_IGNORE;
    }

    if (curline->sidedef->rowoffset)
        return;

    if (line->flags & ML_TWOSIDED)
    {
        // Does top texture need tiling
        if ((c = frontsector->interpceilingheight - backsector->interpceilingheight) > 0
            && textureheight[texturetranslation[curline->sidedef->toptexture]] > c)
            line->r_flags |= RF_TOP_TILE;

        // Does bottom texture need tiling
        if ((c = frontsector->interpfloorheight - backsector->interpfloorheight) > 0
            && textureheight[texturetranslation[curline->sidedef->bottomtexture]] > c)
            line->r_flags |= RF_BOT_TILE;
    }
    else
    {
        // Does middle texture need tiling
        if ((c = frontsector->interpceilingheight - frontsector->interpfloorheight) > 0
            && textureheight[texturetranslation[curline->sidedef->midtexture]] > c)
            line->r_flags |= RF_MID_TILE;
    }
}

// [AM] Interpolate the passed sector.
static void R_InterpolateSector(sector_t *sector)
{
    sector_t    *heightsec = sector->heightsec;

    // Only if we moved the sector last tic
    if (sector->oldgametime == gametime - 1 && vid_capfps != TICRATE)
    {
        // Interpolate between current and last floor/ceiling position
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

        if (heightsec)
        {
            if (heightsec->floorheight != heightsec->oldfloorheight)
                heightsec->interpfloorheight = heightsec->oldfloorheight
                    + FixedMul(heightsec->floorheight - heightsec->oldfloorheight, fractionaltic);
            else
                heightsec->interpfloorheight = heightsec->floorheight;

            if (heightsec->ceilingheight != heightsec->oldceilingheight)
                heightsec->interpceilingheight = heightsec->oldceilingheight
                    + FixedMul(heightsec->ceilingheight - heightsec->oldceilingheight, fractionaltic);
            else
                heightsec->interpceilingheight = heightsec->ceilingheight;
        }
    }
    else
    {
        sector->interpfloorheight = sector->floorheight;
        sector->interpceilingheight = sector->ceilingheight;

        if (heightsec)
        {
            heightsec->interpfloorheight = heightsec->floorheight;
            heightsec->interpceilingheight = heightsec->ceilingheight;
        }
    }
}

//
// killough 03/07/98: Hack floor/ceiling heights for deep water etc.
//
// If player's view height is underneath fake floor, lower the
// drawn ceiling to be just under the floor height, and replace
// the drawn floor and ceiling textures, and light level, with
// the control sector's.
//
// Similar for ceiling, only reflected.
//
// killough 04/11/98, 04/13/98: fix bugs, add 'back' parameter
//
sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec,
    int *floorlightlevel, int *ceilinglightlevel, const bool back)
{
    const sector_t  *s = sec->heightsec;

    if (floorlightlevel)
        *floorlightlevel = (sec->floorlightsec ? sec->floorlightsec->lightlevel : sec->lightlevel);

    if (ceilinglightlevel)
        *ceilinglightlevel = (sec->ceilinglightsec ? sec->ceilinglightsec->lightlevel : sec->lightlevel);

    if (s)
    {
        const sector_t  *heightsec = viewplayer->mo->subsector->sector->heightsec;
        const bool      underwater = (heightsec && viewz <= heightsec->interpfloorheight);

        // Replace sector being drawn, with a copy to be hacked
        *tempsec = *sec;

        // Replace floor and ceiling height with other sector's heights.
        tempsec->interpfloorheight = s->interpfloorheight;
        tempsec->interpceilingheight = s->interpceilingheight;

        // killough 11/98: prevent sudden light changes from non-water sectors:
        if (underwater && ((tempsec->interpfloorheight = sec->interpfloorheight),
            (tempsec->interpceilingheight = s->interpfloorheight - 1), !back))
        {
            // head-below-floor hack
            tempsec->floorpic = s->floorpic;
            tempsec->floorxoffset = s->floorxoffset;
            tempsec->flooryoffset = s->flooryoffset;

            if (s->ceilingpic == skyflatnum)
            {
                tempsec->interpfloorheight = tempsec->interpceilingheight + 1;
                tempsec->ceilingpic = tempsec->floorpic;
                tempsec->ceilingxoffset = tempsec->floorxoffset;
                tempsec->ceilingyoffset = tempsec->flooryoffset;
            }
            else
            {
                tempsec->ceilingpic = s->ceilingpic;
                tempsec->ceilingxoffset = s->ceilingxoffset;
                tempsec->ceilingyoffset = s->ceilingyoffset;
            }

            tempsec->lightlevel = s->lightlevel;

            if (floorlightlevel)
                *floorlightlevel = (s->floorlightsec ? s->floorlightsec->lightlevel : s->lightlevel);

            if (ceilinglightlevel)
                *ceilinglightlevel = (s->ceilinglightsec ? s->ceilinglightsec->lightlevel : s->lightlevel);
        }
        else if (heightsec
            && viewz >= heightsec->interpceilingheight
            && sec->interpceilingheight > s->interpceilingheight)
        {
            // Above-ceiling hack
            tempsec->interpceilingheight = s->interpceilingheight;
            tempsec->interpfloorheight = s->interpceilingheight + 1;

            tempsec->floorpic = tempsec->ceilingpic = s->ceilingpic;
            tempsec->floorxoffset = tempsec->ceilingxoffset = s->ceilingxoffset;
            tempsec->flooryoffset = tempsec->ceilingyoffset = s->ceilingyoffset;

            if (s->floorpic != skyflatnum)
            {
                tempsec->interpceilingheight = sec->interpceilingheight;
                tempsec->floorpic = s->floorpic;
                tempsec->floorxoffset = s->floorxoffset;
                tempsec->flooryoffset = s->flooryoffset;
            }

            tempsec->lightlevel = s->lightlevel;

            if (floorlightlevel)
                *floorlightlevel = (s->floorlightsec ? s->floorlightsec->lightlevel : s->lightlevel);

            if (ceilinglightlevel)
                *ceilinglightlevel = (s->ceilinglightsec ? s->ceilinglightsec->lightlevel : s->lightlevel);
        }

        sec = tempsec;  // Use other sector
    }

    return sec;
}

//
// R_AddLine
// Clips the given segment and adds any visible pieces to the line list.
//
static void R_AddLine(seg_t *line)
{
    int     x1;
    int     x2;
    angle_t angle1;
    angle_t angle2;

    curline = line;

    // Skip this line if it's not facing the camera
    if (R_PointOnSegSide(viewx, viewy, line))
        return;

    angle1 = R_PointToAngleEx(line->v1->x, line->v1->y);
    angle2 = R_PointToAngleEx(line->v2->x, line->v2->y);

    // Back side? I.e. backface culling?
    if (angle1 - angle2 >= ANG180)
        return;

    angle1 -= viewangle;
    angle2 -= viewangle;

    if ((int)angle1 < (int)angle2)
    {
        // Either angle1 or angle2 is behind us, so it doesn't matter if we change it to the correct sign
        if (angle1 >= ANG180 && angle1 < ANG270)
            angle1 = INT_MAX;   // which is ANG180 - 1
        else
            angle2 = INT_MIN;
    }

    if ((int)angle2 >= (int)clipangle)
        return;                 // Both off left edge

    if ((int)angle1 <= -(int)clipangle)
        return;                 // Both off right edge

    if ((int)angle1 >= (int)clipangle)
        angle1 = clipangle;     // Clip at left edge

    if ((int)angle2 <= -(int)clipangle)
        angle2 = 0 - clipangle; // Clip at right edge

    // The seg is in the view range, but not necessarily visible.

    // killough 01/31/98: Here is where "slime trails" can SOMETIMES occur:
    x1 = viewangletox[MIN((angle1 + ANG90) >> ANGLETOFINESHIFT, FINEANGLES / 2 - 1)];
    x2 = viewangletox[MIN((angle2 + ANG90) >> ANGLETOFINESHIFT, FINEANGLES / 2 - 1)];

    // Does not cross a pixel?
    if (x1 >= x2)
        return;

    // Single sided line?
    if ((backsector = line->backsector))
    {
        sector_t    tempsec;    // killough 03/08/98: ceiling/water hack

        // [AM] Interpolate sector movement before running clipping tests.
        // Frontsector should already be interpolated.
        R_InterpolateSector(backsector);

        // killough 03/08/98, 04/04/98: hack for invisible ceilings/deep water
        backsector = R_FakeFlat(backsector, &tempsec, NULL, NULL, true);
    }

    if ((linedef = curline->linedef)->r_validcount != gametime)
        R_RecalcLineFlags(linedef);

    if (linedef->r_flags & RF_IGNORE)
        return;

    R_ClipWallSegment(x1, x2, (linedef->r_flags & RF_CLOSED));
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true if some part of the bbox might be visible.
//
static bool R_CheckBBox(const fixed_t *bspcoord)
{
    const byte checkcoord[12][4] =
    {
        { 3, 0, 2, 1 },
        { 3, 0, 2, 0 },
        { 3, 1, 2, 0 },
        { 0, 0, 0, 0 },
        { 2, 0, 2, 1 },
        { 0, 0, 0, 0 },
        { 3, 1, 3, 0 },
        { 0, 0, 0, 0 },
        { 2, 0, 3, 1 },
        { 2, 1, 3, 1 },
        { 2, 1, 3, 0 }
    };

    byte        boxpos;
    const byte  *check;

    angle_t     angle1;
    angle_t     angle2;

    int         sx1;
    int         sx2;

    // Find the corners of the box that define the edges from current viewpoint.
    boxpos = (viewx <= bspcoord[BOXLEFT] ? 0 : (viewx < bspcoord[BOXRIGHT] ? 1 : 2))
        + (viewy >= bspcoord[BOXTOP] ? 0 : (viewy > bspcoord[BOXBOTTOM] ? 4 : 8));

    if (boxpos == 5)
        return true;

    check = checkcoord[boxpos];

    // check clip list for an open space
    angle1 = R_PointToAngleEx(bspcoord[check[0]], bspcoord[check[1]]) - viewangle;
    angle2 = R_PointToAngleEx(bspcoord[check[2]], bspcoord[check[3]]) - viewangle;

    // cph - replaced old code, which was unclear and badly commented
    // Much more efficient code now
    if ((int)angle1 < (int)angle2)
    {
        // Either angle1 or angle2 is behind us, so it doesn't matter if we change it to the correct sign
        if (angle1 >= ANG180 && angle1 < ANG270)
            angle1 = INT_MAX;           // which is ANG180 - 1
        else
            angle2 = INT_MIN;
    }

    if ((int)angle2 >= (int)clipangle)
        return false;                   // Both off left edge

    if ((int)angle1 <= -(int)clipangle)
        return false;                   // Both off right edge

    if ((int)angle1 >= (int)clipangle)
        angle1 = clipangle;             // Clip at left edge

    if ((int)angle2 <= -(int)clipangle)
        angle2 = 0 - clipangle;         // Clip at right edge

    // Find the first clippost that touches the source post (adjacent pixels are touching).
    sx1 = viewangletox[MIN((angle1 + ANG90) >> ANGLETOFINESHIFT, FINEANGLES / 2 - 1)];
    sx2 = viewangletox[MIN((angle2 + ANG90) >> ANGLETOFINESHIFT, FINEANGLES / 2 - 1)];

    // Does not cross a pixel.
    if (sx1 == sx2)
        return false;

    if (!memchr(solidcol + sx1, 0, (size_t)sx2 - sx1))
        return false;

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
    subsector_t *sub = subsectors + num;
    sector_t    tempsec;                                    // killough 03/07/98: deep water hack
    sector_t    *sector = sub->sector;
    int         floorlightlevel;                            // killough 03/16/98: set floor lightlevel
    int         ceilinglightlevel;                          // killough 04/11/98
    int         count = sub->numlines;
    seg_t       *line = segs + sub->firstline;

    // [AM] Interpolate sector movement. Usually only needed when player is standing inside the sector.
    R_InterpolateSector(sector);

    // killough 03/08/98, 04/04/98: Deep water/fake ceiling effect
    frontsector = R_FakeFlat(sector, &tempsec, &floorlightlevel, &ceilinglightlevel, false);

    floorplane = (frontsector->interpfloorheight < viewz    // killough 03/07/98
        || (frontsector->heightsec && frontsector->heightsec->ceilingpic == skyflatnum) ?
        R_FindPlane(frontsector->interpfloorheight,
            (frontsector->floorpic == skyflatnum            // killough 10/98
                && (frontsector->sky & PL_SKYFLAT) ? frontsector->sky : frontsector->floorpic),
            floorlightlevel,                                // killough 03/16/98
            frontsector->floorxoffset,                      // killough 03/07/98
            frontsector->flooryoffset) : NULL);

    ceilingplane = (frontsector->interpceilingheight > viewz
        || frontsector->ceilingpic == skyflatnum
        || (frontsector->heightsec && frontsector->heightsec->floorpic == skyflatnum) ?
        R_FindPlane(frontsector->interpceilingheight,       // killough 03/08/98
            (frontsector->ceilingpic == skyflatnum          // killough 10/98
                && (frontsector->sky & PL_SKYFLAT) ? frontsector->sky : frontsector->ceilingpic),
            ceilinglightlevel,                              // killough 04/11/98
            frontsector->ceilingxoffset,                    // killough 03/07/98
            frontsector->ceilingyoffset) : NULL);

    // killough 09/18/98: Fix underwater slowdown, by passing real sector
    // instead of fake one. Improve sprite lighting by basing sprite
    // lightlevels on floor and ceiling lightlevels in the surrounding area.
    //
    // 10/98 killough:
    //
    // NOTE: TeamTNT fixed this bug incorrectly, messing up sprite lighting!!!
    // That is part of the 242 effect!!! If you simply pass sub->sector to
    // the old code you will not get correct lighting for underwater sprites!!!
    // Either you must pass the fake sector and handle validcount here, on the
    // real sector, or you must account for the lighting in some other way,
    // like passing it as an argument.
    if (sector->validcount != validcount && !menuactive && !automapactive)
    {
        sector->validcount = validcount;
        R_AddSprites(sector, (sector->heightsec ? (ceilinglightlevel + floorlightlevel) / 2 : floorlightlevel));
        R_AddNearbySprites(sector);
    }

    while (count--)
        R_AddLine(line++);
}

//
// R_RenderBSPNode
// Renders all subsectors below a given node, traversing subtree recursively.
// [BH] Made non-recursive
//
#define MAX_BSP_DEPTH   256

void R_RenderBSPNode(int bspnum)
{
    int bspstack[MAX_BSP_DEPTH] = { 0 };
    int sidestack[MAX_BSP_DEPTH] = { 0 };
    int sp = 0;

    while (true)
    {
        const node_t    *bsp;
        int             side;

        while (!(bspnum & NF_SUBSECTOR))
        {
            if (sp == MAX_BSP_DEPTH)
                break;

            bsp = nodes + bspnum;
            side = (((int64_t)viewy - bsp->y) * bsp->dx + ((int64_t)bsp->x - viewx) * bsp->dy > 0);
            bspstack[sp] = bspnum;
            sidestack[sp++] = side;
            bspnum = bsp->children[side];
        }

        R_Subsector(bspnum == -1 ? 0 : (bspnum & ~NF_SUBSECTOR));

        if (!sp)
            return;

        side = sidestack[--sp] ^ 1;
        bsp = nodes + bspstack[sp];

        while (!R_CheckBBox(bsp->bbox[side]))
        {
            if (!sp)
                return;

            side = sidestack[--sp] ^ 1;
            bsp = nodes + bspstack[sp];
        }

        bspnum = bsp->children[side];
    }
}
