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

#include "m_bbox.h"
#include "p_local.h"

//
// P_CheckSight
//

// killough 4/19/98:
// Convert LOS info to struct for reentrancy and efficiency of data locality
typedef struct
{
    fixed_t     sightzstart, t2x, t2y;  // eye z of looker
    divline_t   strace;                 // from t1 to t2
    fixed_t     topslope, bottomslope;  // slopes to top and bottom of target
    fixed_t     bbox[4];
    fixed_t     maxz, minz;             // cph - z optimizations for 2sided lines
} los_t;

static los_t    los; // cph - made static

//
// P_DivlineSide
// Returns side 0 (front), 1 (back), or 2 (on).
//
static int P_DivlineSide(fixed_t x, fixed_t y, const divline_t *node)
{
    fixed_t left;
    fixed_t right;

    if (!node->dx)
        return (x == node->x ? 2 : (x <= node->x ? node->dy > 0 : node->dy < 0));

    if (!node->dy)
        return (y == node->y ? 2 : (y <= node->y ? node->dx < 0 : node->dx > 0));

    left = (node->dy >> FRACBITS) * ((x - node->x) >> FRACBITS);
    right = ((y - node->y) >> FRACBITS) * (node->dx >> FRACBITS);

    return (right < left ? 0 : (left == right ? 2 : 1));
}

//
// P_InterceptVector2
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings and addlines traversers.
//
static fixed_t P_InterceptVector2(const divline_t *v2, const divline_t *v1)
{
    fixed_t den = FixedMul(v1->dy >> 8, v2->dx) - FixedMul(v1->dx >> 8, v2->dy);

    if (!den)
        return 0;

    return FixedDiv(FixedMul((v1->x - v2->x) >> 8, v1->dy) + FixedMul((v2->y - v1->y) >> 8, v1->dx), den);
}

//
// P_CrossSubsector
// Returns true
//  if strace crosses the given subsector successfully.
//
static dboolean P_CrossSubsector(int num)
{
    subsector_t *sub = subsectors + num;
    seg_t       *seg = segs + sub->firstline;

    for (int count = sub->numlines; count; seg++, count--)
    {
        line_t      *line = seg->linedef;
        fixed_t     frac;
        sector_t    *front;
        sector_t    *back;
        fixed_t     opentop;
        fixed_t     openbottom;
        divline_t   divl;
        vertex_t    *v1;
        vertex_t    *v2;

        if (line->bbox[BOXLEFT] > los.bbox[BOXRIGHT] || line->bbox[BOXRIGHT] < los.bbox[BOXLEFT]
            || line->bbox[BOXBOTTOM] > los.bbox[BOXTOP] || line->bbox[BOXTOP] < los.bbox[BOXBOTTOM])
        {
            line->validcount = validcount;
            continue;
        }

        v1 = line->v1;
        v2 = line->v2;

        // line isn't crossed?
        if (P_DivlineSide(v1->x, v1->y, &los.strace) == P_DivlineSide(v2->x, v2->y, &los.strace))
        {
            line->validcount = validcount;
            continue;
        }

        divl.x = v1->x;
        divl.y = v1->y;
        divl.dx = v2->x - v1->x;
        divl.dy = v2->y - v1->y;

        // line isn't crossed?
        if (P_DivlineSide(los.strace.x, los.strace.y, &divl) == P_DivlineSide(los.t2x, los.t2y, &divl))
        {
            line->validcount = validcount;
            continue;
        }

        // already checked other side?
        if (line->validcount == validcount)
            continue;

        line->validcount = validcount;

        // stop because it is not two sided anyway
        if (!(line->flags & ML_TWOSIDED))
            return false;

        // crosses a two sided line
        front = seg->frontsector;
        back = seg->backsector;

        // no wall to block sight with?
        if (front->floorheight == back->floorheight && front->ceilingheight == back->ceilingheight)
            continue;

        // possible occluder
        // because of ceiling height differences
        opentop = MIN(front->ceilingheight, back->ceilingheight);

        // because of floor height differences
        openbottom = MAX(front->floorheight, back->floorheight);

        // cph - reject if does not intrude in the z-space of the possible LOS
        if (opentop >= los.maxz && openbottom <= los.minz)
            continue;

        // cph - if bottom >= top or top < minz or bottom > maxz then it must be
        // solid wrt this LOS
        if (openbottom >= opentop || opentop < los.minz || openbottom > los.maxz)
            return false;

        // crosses a two sided line
        frac = P_InterceptVector2(&los.strace, &divl);

        if (front->floorheight != back->floorheight)
            los.bottomslope = MAX(los.bottomslope, FixedDiv(openbottom - los.sightzstart, frac));

        if (front->ceilingheight != back->ceilingheight)
            los.topslope = MIN(los.topslope, FixedDiv(opentop - los.sightzstart, frac));

        if (los.topslope <= los.bottomslope)
            return false;   // stop
    }

    // passed the subsector ok
    return true;
}

//
// P_CrossBSPNode
// Returns true
//  if strace crosses the given node successfully.
//
static dboolean P_CrossBSPNode(int bspnum)
{
    while (!(bspnum & NF_SUBSECTOR))
    {
        const node_t    *bsp = nodes + bspnum;
        int             side1 = P_DivlineSide(los.strace.x, los.strace.y, (divline_t *)bsp) & 1;
        int             side2 = P_DivlineSide(los.t2x, los.t2y, (divline_t *)bsp);

        if (side1 == side2)
            bspnum = bsp->children[side1];              // doesn't touch the other side
        else if (!P_CrossBSPNode(bsp->children[side1])) // the partition plane is crossed here
            return false;                               // cross the starting side
        else
            bspnum = bsp->children[side1 ^ 1];          // cross the ending side
    }

    return P_CrossSubsector(bspnum == -1 ? 0 : (bspnum & ~NF_SUBSECTOR));
}

//
// P_CheckSight
// Returns true
//  if a straight line between t1 and t2 is unobstructed.
// Uses REJECT.
//
dboolean P_CheckSight(mobj_t *t1, mobj_t *t2)
{
    const sector_t  *s1 = t1->subsector->sector;
    const sector_t  *s2 = t2->subsector->sector;
    int             pnum = s1->id * numsectors + s2->id;

    // First check for trivial rejection.
    // Determine subsector entries in REJECT table.
    // Check in REJECT table.
    if (rejectmatrix[pnum >> 3] & (1 << (pnum & 7)))
        return false;

    // killough 4/19/98: make fake floors and ceilings block monster view
    if ((s1->heightsec && ((t1->z + t1->height <= s1->heightsec->interpfloorheight &&
        t2->z >= s1->heightsec->interpfloorheight)
        || (t1->z >= s1->heightsec->interpceilingheight &&
        t2->z + t1->height <= s1->heightsec->interpceilingheight))) || (s2->heightsec &&
        ((t2->z + t2->height <= s2->heightsec->interpfloorheight &&
        t1->z >= s2->heightsec->interpfloorheight)
        || (t2->z >= s2->heightsec->interpceilingheight &&
        t1->z + t2->height <= s2->heightsec->interpceilingheight))))
        return false;

    // killough 11/98: shortcut for melee situations
    // same subsector? obviously visible
    if (t1->subsector == t2->subsector)
        return true;

    // An unobstructed LOS is possible.
    // Now look from eyes of t1 to any part of t2.
    validcount++;

    los.sightzstart = t1->z + t1->height - (t1->height >> 2);
    los.bottomslope = t2->z - los.sightzstart;
    los.topslope = los.bottomslope + t2->height;

    los.strace.x = t1->x;
    los.strace.y = t1->y;
    los.t2x = t2->x;
    los.t2y = t2->y;
    los.strace.dx = t2->x - t1->x;
    los.strace.dy = t2->y - t1->y;

    los.bbox[BOXRIGHT] = MAX(t1->x, t2->x);
    los.bbox[BOXLEFT] = MIN(t1->x, t2->x);
    los.bbox[BOXTOP] = MAX(t1->y, t2->y);
    los.bbox[BOXBOTTOM] = MIN(t1->y, t2->y);

    // cph - calculate min and max z of the potential line of sight
    if (los.sightzstart < t2->z)
    {
        los.maxz = t2->z + t2->height;
        los.minz = los.sightzstart;
    }
    else if (los.sightzstart > t2->z + t2->height)
    {
        los.maxz = los.sightzstart;
        los.minz = t2->z;
    }
    else
    {
        los.maxz = t2->z + t2->height;
        los.minz = t2->z;
    }

    // the head node is the last node output
    return P_CrossBSPNode(numnodes - 1);
}
