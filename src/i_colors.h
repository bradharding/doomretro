/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#if !defined(__I_COLORS_H__)
#define __I_COLORS_H__

#include "doomtype.h"
#include "r_defs.h"

extern byte *tinttab20;
extern byte *tinttab25;
extern byte *tinttab33;
extern byte *tinttab40;
extern byte *tinttab50;
extern byte *tinttab60;
extern byte *tinttab66;
extern byte *tinttab75;

extern byte *tranmap;
extern byte *tinttabadditive;
extern byte *tinttabred;
extern byte *tinttabredwhite1;
extern byte *tinttabredwhite2;
extern byte *tinttabgreen;
extern byte *tinttabblue;
extern byte *tinttabred33;
extern byte *tinttabredwhite50;
extern byte *tinttabgreen33;
extern byte *tinttabblue25;

extern byte nearestcolors[256];

void I_InitTintTables(byte *palette);
int FindNearestColor(byte *palette, int red, int green, int blue);
void FindNearestColors(byte *palette);

int FindDominantColor(patch_t *patch);

#endif
