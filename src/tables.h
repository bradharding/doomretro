/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

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

#ifndef __TABLES__
#define __TABLES__

#include "m_fixed.h"

#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES - 1)


// 0x100000000 to 0x2000
#define ANGLETOFINESHIFT        19

// Effective size is 10240.
extern const fixed_t finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 phase shift.
extern const fixed_t *finecosine;


// Effective size is 4096.
extern const fixed_t finetangent[FINEANGLES / 2];

// Binary Angle Measument, BAM.
#define ANG45                   0x20000000
#define ANG90                   0x40000000
#define ANG180                  0x80000000
#define ANG270                  0xc0000000


#define SLOPERANGE              2048
#define SLOPEBITS               11
#define DBITS                   (FRACBITS - SLOPEBITS)

typedef unsigned angle_t;


// Effective size is 2049;
// The +1 size is to handle the case when x==y
//  without additional checking.
extern const angle_t tantoangle[SLOPERANGE + 1];

// Utility function, called by R_PointToAngle.
int SlopeDiv(unsigned num, unsigned den);

#endif
