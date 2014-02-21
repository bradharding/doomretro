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

#ifndef __R_PLANE__
#define __R_PLANE__


#include "r_data.h"



// Visplane related.
extern  int             *lastopening;


typedef void (*planefunction_t)(int top, int bottom);

extern planefunction_t  floorfunc;
extern planefunction_t  ceilingfunc_t;

extern int              floorclip[SCREENWIDTH];
extern int              ceilingclip[SCREENWIDTH];

extern fixed_t          yslope[SCREENHEIGHT];
extern fixed_t          distscale[SCREENWIDTH];

extern boolean          flipsky;

void R_ClearPlanes(void);

void R_MakeSpans(int x, int t1, int b1, int t2, int b2);

void R_DrawPlanes(void);

visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel);

visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop);



#endif