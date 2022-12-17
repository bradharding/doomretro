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

#include "r_defs.h"

//
// Typedefs of widgets
//

// Number widget
typedef struct
{
    // upper right-hand corner
    //  of the number (right-justified)
    int         x, y;

    // max # of digits in number
    int         width;

    // pointer to current value
    int         *num;

    // list of patches for 0-9
    patch_t     **p;

    // user data
    int         data;
} st_number_t;

// Percent widget ("child" of number widget,
//  or, more precisely, contains a number widget.)
typedef struct
{
    // number information
    st_number_t n;

    // percent sign graphic
    patch_t     *p;
} st_percent_t;

// Multiple Icon widget
typedef struct
{
    // center-justified location of icons
    int         x, y;

    // last icon number
    int         oldinum;

    // pointer to current icon
    int         *inum;

    // list of icons
    patch_t     **patch;
} st_multicon_t;

extern bool usesmallnums;

//
// Widget creation, access, and update routines
//

// Initializes widget library.
// More precisely, initialize STMINUS,
//  everything else is done somewhere else.
//
// Number widget routines
void STlib_InitNum(st_number_t *n, int x, int y, patch_t **pl, int *num, int width);

void STlib_UpdateBigAmmoNum(st_number_t *n);
void STlib_UpdateBigArmorNum(st_number_t *n);
void STlib_UpdateBigHealthNum(st_number_t *n);
void STlib_UpdateSmallNum(st_number_t *n);

// Percent widget routines
void STlib_InitPercent(st_percent_t *p, int x, int y, patch_t **pl, int *num, patch_t *percent);

void STlib_UpdateHealthPercent(st_percent_t *per, int refresh);
void STlib_UpdateArmorPercent(st_percent_t *per, int refresh);

// Multiple Icon widget routines
void STlib_InitMultIcon(st_multicon_t *mi, int x, int y, patch_t **il, int *inum);

void STlib_UpdateMultIcon(st_multicon_t *mi, bool refresh);

void STlib_UpdateArmsIcon(st_multicon_t *mi, bool refresh, int i);

void STLib_Init(void);
