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

#pragma once

#include "m_fixed.h"

#define FINEANGLES          8192
#define FINEMASK            (FINEANGLES - 1)

#define ANGLETOFINESHIFT    19

#define ANGLEMULTIPLIER     0x002C62C6

// Binary Angle Measurement, BAM.
#define ANG1                (ANG45 / 45)
#define ANG5                (ANG90 / 18)
#define ANG30               (ANG90 / 3)
#define ANG45               0x20000000
#define ANG60               (ANG180 / 3)
#define ANG90               0x40000000
#define ANG180              0x80000000
#define ANG270              0xC0000000
#define ANGLE_MAX           0xFFFFFFFF

#define SLOPERANGE          2048
#define SLOPEBITS           11
#define DBITS               (FRACBITS - SLOPEBITS)

typedef unsigned int    angle_t;

// Effective size is 10240.
extern fixed_t  finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 phase shift.
extern fixed_t  *finecosine;

// Effective size is 4096.
extern fixed_t  finetangent[FINEANGLES / 2];

// Effective size is 2049;
// The +1 size is to handle the case when x == y without additional checking.
extern angle_t  tantoangle[SLOPERANGE + 1];

// MBF21: More utility functions, courtesy of Quasar (James Haley).
// These are straight from Eternity so demos stay in sync.
CONSTATTR inline static angle_t FixedToAngle(fixed_t a)
{
    return (angle_t)(((uint64_t)a * ANG1) >> FRACBITS);
}

// [XA] Clamped angle->slope, for convenience
CONSTATTR inline static fixed_t AngleToSlope(int a)
{
    return finetangent[(a > ANG90 ? 0 : (-a > ANG90 ? FINEANGLES / 2 - 1 : (ANG90 - a) >> ANGLETOFINESHIFT))];
}

// [XA] Ditto, using fixed-point-degrees input
CONSTATTR inline static fixed_t DegToSlope(fixed_t a)
{
    return AngleToSlope(a >= 0 ? FixedToAngle(a) : -(int)FixedToAngle(-a));
}

CONSTATTR inline static angle_t AngleDiff(angle_t a, angle_t b)
{
    return (b > a ? AngleDiff(b, a) : (a - b < ANG180 ? a - b : b - a));
}
