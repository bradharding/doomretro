/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

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

#include "doomtype.h"
#include "m_fixed.h"

int ABS(int a)
{
    int b = a >> 31;

    return ((a ^ b) - b);
}

int MAX(int a, int b)
{
    b = a - b;
    return (a - (b & (b >> 31)));
}

int MIN(int a, int b)
{
    a -= b;
    return (b + (a & (a >> 31)));
}

int BETWEEN(int a, int b, int c)
{
    return MAX(a, MIN(b, c));
}

float BETWEENF(float a, float b, float c)
{
    return (b < a ? a : (b > c ? c : b));
}

int SIGN(int a)
{
    return (1 | (a >> 31));
}

fixed_t FixedMul(fixed_t a, fixed_t b)
{
    return (((int64_t)a * b) >> FRACBITS);
}

fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((ABS(a) >> 14) >= ABS(b))
        return ((a ^ b) >> 31) ^ INT_MAX;
    else
        return (fixed_t)(((int64_t)a << FRACBITS) / b);
}

fixed_t FixedMod(fixed_t a, fixed_t b)
{
    if (b & (b - 1))
    {
        fixed_t r = a % b;

        return (r < 0 ? r + b : r);
    }
    else
        return (a & (b - 1));
}

unsigned int SafeAdd(unsigned int a, int b)
{
    return (b > 0 && (unsigned int)b > UINT_MAX - a ? a : a + b);
}
