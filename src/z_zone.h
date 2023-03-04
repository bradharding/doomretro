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

// Include system definitions so that prototypes become
// active before macro replacements below are in effect.
#include <string.h>
#include <assert.h>

#include "doomdef.h"

//
// ZONE MEMORY
// PU - purge tags.
//
enum
{
    PU_FREE,       // a free block
    PU_STATIC,     // static entire execution time
    PU_LEVEL,      // static until level exited
    PU_LEVSPEC,    // a special thinker in a level
    PU_CACHE,
    PU_MAX         // Must always be last -- killough
};

#define PU_PURGELEVEL    PU_CACHE    // First purgeable tag's level

void *Z_Malloc(size_t size, int tag, void **user) ALLOCATTR(1);
void *Z_Calloc(size_t n1, size_t n2, int tag, void **user) ALLOCSATTR(1, 2);
char *Z_Strdup(const char *, int tag, void **user);
void Z_Free(void *ptr);
void Z_FreeTags(int lowtag, int hightag);
void Z_ChangeTag(void *ptr, int tag);
