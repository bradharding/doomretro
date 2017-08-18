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

#if !defined(__M_FIXED_H__)
#define __M_FIXED_H__

#include <inttypes.h>

//
// Fixed point, 32bit as 16.16.
//
#define FRACBITS        16
#define FRACUNIT        (1 << FRACBITS)
#define FIXED2DOUBLE(x) (x / (double)FRACUNIT)

#ifdef ABS
#undef ABS
#undef MAX
#undef MIN
#endif

typedef int32_t fixed_t;

int32_t ABS(int32_t a);
int32_t MAX(int32_t a, int32_t b);
int32_t MIN(int32_t a, int32_t b);
int32_t BETWEEN(int32_t a, int32_t b, int32_t c);
float BETWEENF(float a, float b, float c);
int32_t SIGN(int32_t a);
fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);
fixed_t FixedMod(fixed_t a, fixed_t b);
uint32_t SafeAdd(uint32_t a, int32_t b);

#endif
