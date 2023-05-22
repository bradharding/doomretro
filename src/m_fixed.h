/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#pragma once

#include "doomdef.h"
#include "doomtype.h"

#undef ABS
#undef MIN
#undef MAX
#undef BETWEEN
#undef SIGN
#undef SWAP

//
// Fixed point, 32-bit as 16.16.
//
#define FRACBITS        16
#define FRACUNIT        (1 << FRACBITS)
#define FIXED2DOUBLE(a) ((a) / (double)FRACUNIT)
#define FIXED_MIN       INT32_MIN
#define FIXED_MAX       INT32_MAX

#define SWAP(a, b)      (a) ^= (b), (b) ^= (a), (a) ^= (b)

typedef int32_t fixed_t;

CONSTATTR static inline int ABS(int a)
{
    return (a < 0 ? -a : a);
}

CONSTATTR static inline int MAX(int a, int b)
{
    return (a > b ? a : b);
}

CONSTATTR static inline int MIN(int a, int b)
{
    return (a < b ? a : b);
}

CONSTATTR static inline int BETWEEN(int a, int b, int c)
{
    return (b < a ? a : (b > c ? c : b));
}

CONSTATTR static inline float BETWEENF(float a, float b, float c)
{
    return (b < a ? a : (b > c ? c : b));
}

CONSTATTR static inline int SIGN(int a)
{
    return (a < 0 ? -1 : 1);
}

CONSTATTR static inline fixed_t FixedMul(fixed_t a, fixed_t b)
{
    return (((int64_t)a * b) >> FRACBITS);
}

CONSTATTR static inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    return (((int64_t)ABS(a) >> 15) >= ABS(b) ? (((int64_t)(a ^ b) >> 31) ^ FIXED_MAX) : (((int64_t)a << FRACBITS) / b));
}

CONSTATTR static inline fixed_t FixedMod(fixed_t a, fixed_t b)
{
    return ((b & (b - 1)) ? ((a %= b) < 0 ? a + b : a) : (a & (b - 1)));
}

CONSTATTR static inline uint64_t SafeAdd(uint64_t a, int b)
{
    return (b > UINT64_MAX - a ? UINT64_MAX : a + b);
}
