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

#ifndef __R_BSP__
#define __R_BSP__



extern seg_t            *curline;
extern side_t           *sidedef;
extern line_t           *linedef;
extern sector_t         *frontsector;
extern sector_t         *backsector;

extern int              rw_x;
extern int              rw_stopx;

extern boolean          segtextured;

// false if the back side is the same plane
extern boolean          markfloor;
extern boolean          markceiling;

extern boolean          skymap;

extern drawseg_t        *drawsegs;
extern unsigned int     maxdrawsegs;

extern drawseg_t        *ds_p;

extern lighttable_t     **hscalelight;
extern lighttable_t     **vscalelight;
extern lighttable_t     **dscalelight;


typedef void (*drawfunc_t)(int start, int stop);


// BSP?
void R_ClearClipSegs(void);
void R_ClearDrawSegs(void);


void R_RenderBSPNode(int bspnum);
int R_DoorClosed(void);


#endif