/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __Z_ZONE__
#define __Z_ZONE__

// Include system definitions so that prototypes become
// active before macro replacements below are in effect.

#include "doomtype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//
// ZONE MEMORY
// PU - purge tags.
//
enum
{
    PU_FREE,       // a free block
    PU_STATIC,     // static entire execution time
    PU_SOUND,      // static while playing
    PU_MUSIC,      // static while playing
    PU_LEVEL,      // static until level exited
    PU_LEVSPEC,    // a special thinker in a level

    PU_CACHE,
    PU_MAX         // Must always be last -- killough
};

#define PU_PURGELEVEL    PU_CACHE    // First purgable tag's level

void *Z_Malloc(size_t size, int32_t tag, void **ptr);
void Z_Free(void *ptr);
void Z_FreeTags(int32_t lowtag, int32_t hightag);
void Z_ChangeTag(void *ptr, int32_t tag);
void *Z_Calloc(size_t n1, size_t n2, int32_t tag, void **user);
void *Z_Realloc(void *ptr, size_t n, int32_t tag, void **user);
char *Z_Strdup(const char *s, int32_t tag, void **user);

#endif
