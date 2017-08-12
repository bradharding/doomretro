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

#if !defined(__TABLES_H__)
#define __TABLES_H__

#include "m_fixed.h"

#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES - 1)

// 0x100000000 to 0x2000
#define ANGLETOFINESHIFT        19

// Effective size is 10240.
extern fixed_t  finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 phase shift.
extern fixed_t  *finecosine;

// Effective size is 4096.
fixed_t         finetangent[FINEANGLES / 2];

// Binary Angle Measurement, BAM.
#define ANG5                    (ANG90 / 18)
#define ANG45                   0x20000000
#define ANG90                   0x40000000
#define ANG180                  0x80000000
#define ANG270                  0xC0000000

#define SLOPERANGE              2048
#define SLOPEBITS               11
#define DBITS                   (FRACBITS - SLOPEBITS)

typedef unsigned angle_t;

// Effective size is 2049;
// The +1 size is to handle the case when x==y
//  without additional checking.
extern angle_t  tantoangle[SLOPERANGE + 1];

#endif
