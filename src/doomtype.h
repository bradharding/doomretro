/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#include <inttypes.h>
#include <limits.h>

#ifdef __cplusplus

// Use builtin bool type with C++.
typedef bool boolean;

#else

typedef enum
{
    false,
    true
} bool;

#define boolean bool

#endif

typedef uint8_t byte;

#ifdef WIN32

#define DIR_SEPARATOR   '\\'
#define DIR_SEPARATOR_S "\\"
#define PATH_SEPARATOR  ';'

#else

#define DIR_SEPARATOR   '/'
#define DIR_SEPARATOR_S "/"
#define PATH_SEPARATOR  ':'

#endif

#define arrlen(array) (sizeof(array) / sizeof(*array))

#endif
