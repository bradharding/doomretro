/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

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

==============================================================================
*/

#include "m_fixed.h"
#include "m_random.h"
#include "tables.h"

unsigned int    seed;
unsigned int    bigseed;

// MBF21: [XA] Common random formulas used by codepointers

//
// P_RandomHitscanAngle
// Outputs a random angle between (-spread, spread), as an int ('cause it can be negative).
//   spread: Maximum angle (degrees, in fixed point -- not BAM!)
//
int P_RandomHitscanAngle(const fixed_t spread)
{
    return (int)(((int64_t)(spread < 0 ? FixedToAngle(-spread) : FixedToAngle(spread)) * M_SubRandom()) / 255);
}

//
// P_RandomHitscanSlope
// Outputs a random angle between (-spread, spread), converted to values used for slope
//   spread: Maximum vertical angle (degrees, in fixed point -- not BAM!)
//
int P_RandomHitscanSlope(const fixed_t spread)
{
    const int   angle = P_RandomHitscanAngle(spread);

    return finetangent[(angle > ANG90 ? 0 : (-angle > ANG90 ? FINEANGLES / 2 - 1 : (ANG90 - angle) >> ANGLETOFINESHIFT))];
}
