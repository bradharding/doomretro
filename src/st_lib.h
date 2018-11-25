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

#if !defined(__ST_LIB_H__)
#define __ST_LIB_H__

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
    patch_t     **p;
} st_multicon_t;

extern dboolean usesmallnums;

//
// Widget creation, access, and update routines
//

// Initializes widget library.
// More precisely, initialize STMINUS,
//  everything else is done somewhere else.
//
// Number widget routines
void STlib_initNum(st_number_t *n, int x, int y, patch_t **pl, int *num, int width);

void STlib_updateBigNum(st_number_t *n);
void STlib_updateSmallNum(st_number_t *n);

// Percent widget routines
void STlib_initPercent(st_percent_t *p, int x, int y, patch_t **pl, int *num, patch_t *percent);

void STlib_updatePercent(st_percent_t *per, int refresh);

// Multiple Icon widget routines
void STlib_initMultIcon(st_multicon_t *mi, int x, int y, patch_t **il, int *inum);

void STlib_updateMultIcon(st_multicon_t *mi, dboolean refresh);

void STlib_updateArmsIcon(st_multicon_t *mi, dboolean refresh, int i);

#endif
