/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

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

#if !defined(__D_ITEMS_H__)
#define __D_ITEMS_H__

#include "doomdef.h"
#include "doomtype.h"

#define BFGCELLS    40

// Weapon info: sprite frames, ammunition use.
typedef struct
{
    char            *description;
    ammotype_t      ammotype;
    int             minammo;
    int             upstate;
    int             downstate;
    int             readystate;
    int             atkstate;
    int             flashstate;
    int             recoil;
    int             strength;
    int             tics;
    int             prev;
    int             next;
    dboolean        altered;
} weaponinfo_t;

extern weaponinfo_t weaponinfo[NUMWEAPONS];

#endif
