/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

#include "doomtype.h"
#include "m_config.h"
#include "r_defs.h"

#define BLACK           0
#define BLUE1         198
#define BLUE2         199
#define BLUE3         203
#define DARKBLUE      244
#define DARKGRAY      102
#define GRAY           92
#define GREEN1        114
#define GREEN2        118
#define GREEN3        125
#define LIGHTGRAY1     80
#define LIGHTGRAY2     82
#define LIGHTGRAY3     86
#define PINK          251
#define RED1          176
#define RED2          180
#define WHITE           4
#define YELLOW1       161
#define YELLOW2       231

extern byte *tinttab5;
extern byte *tinttab10;
extern byte *tinttab15;
extern byte *tinttab20;
extern byte *tinttab25;
extern byte *tinttab30;
extern byte *tinttab33;
extern byte *tinttab40;
extern byte *tinttab50;
extern byte *tinttab60;
extern byte *tinttab66;
extern byte *tinttab70;
extern byte *tinttab75;
extern byte *tinttab80;
extern byte *tinttab90;

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

extern byte nearestblack;
extern byte nearestdarkblue;
extern byte nearestgold;
extern byte nearestgreen;
extern byte nearestlightgray;
extern byte nearestred;
extern byte nearestwhite;

extern byte *black25;
extern byte *black40;
extern byte *gold15;
extern byte *white25;
extern byte *white33;

void I_InitTintTables(byte *palette);
int FindNearestColor(byte *palette, const byte red, const byte green, const byte blue);
void FindNearestColors(byte *palette);

int FindBrightDominantColor(patch_t *patch);
int FindDominantEdgeColor(patch_t *patch);
