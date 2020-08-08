/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

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
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__M_FIXED_H__)
#define __M_FIXED_H__

#include "doomtype.h"

#undef ABS
#undef MIN
#undef MAX
#undef SWAP

//
// Fixed point, 32bit as 16.16.
//
#define FRACBITS        16
#define FRACUNIT        65536
#define FIXED2DOUBLE(a) ((a) / (double)FRACUNIT)
#define FIXED_MIN       INT32_MIN
#define FIXED_MAX       INT32_MAX
#define SWAP(a, b)      (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b)))

typedef int32_t fixed_t;

static inline int ABS(int a)
{
    int b = a >> 31;

    return ((a ^ b) - b);
}

static inline int MAX(int a, int b)
{
    b = a - b;
    return (a - (b & (b >> 31)));
}

static inline int MIN(int a, int b)
{
    a -= b;
    return (b + (a & (a >> 31)));
}

static inline int BETWEEN(int a, int b, int c)
{
    b -= c;
    c = a - c - (b & (b >> 31));
    return (a - (c & (c >> 31)));
}

static inline float BETWEENF(float a, float b, float c)
{
    return (b < a ? a : (b > c ? c : b));
}

static inline int SIGN(int a)
{
    return (1 | (a >> 31));
}

static inline fixed_t FixedMul(fixed_t a, fixed_t b)
{
    return (((int64_t)a * b) >> FRACBITS);
}

static inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    return ((ABS(a) >> 15) >= ABS(b) ? (((a ^ b) >> 31) ^ FIXED_MAX) : (fixed_t)(((int64_t)a << FRACBITS) / b));
}

static inline fixed_t FixedMod(fixed_t a, fixed_t b)
{
    return ((b & (b - 1)) ? ((a %= b) < 0 ? a + b : a) : (a & (b - 1)));
}

static inline uint64_t SafeAdd(uint64_t a, uint64_t b)
{
    return (b > UINT64_MAX - a ? a : a + b);
}

#endif
