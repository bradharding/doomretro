/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (c) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (c) 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__W_FILE__)
#define __W_FILE__

#include <stdio.h>
#include "doomtype.h"

typedef struct _wad_file_s wad_file_t;

typedef struct
{
    // Open a file for reading.
    wad_file_t *(*OpenFile)(char *path);

    // Close the specified file.
    void (*CloseFile)(wad_file_t *file);

    // Read data from the specified position in the file into the
    // provided buffer.  Returns the number of bytes read.
    size_t (*Read)(wad_file_t *file, unsigned int offset, void *buffer, size_t buffer_len);
} wad_file_class_t;

struct _wad_file_s
{
    // Class of this file.
    wad_file_class_t    *file_class;

    // If this is NULL, the file cannot be mapped into memory.  If this
    // is non-NULL, it is a pointer to the mapped file.
    byte                *mapped;

    // Length of the file, in bytes.
    unsigned int        length;

    dboolean            freedoom;

    char                path[260];

    int                 type;
};

// Open the specified file. Returns a pointer to a new wad_file_t
// handle for the WAD file, or NULL if it could not be opened.
wad_file_t *W_OpenFile(char *path);

// Close the specified WAD file.
void W_CloseFile(wad_file_t *wad);

// Read data from the specified file into the provided buffer.  The
// data is read from the specified offset from the start of the file.
// Returns the number of bytes read.
size_t W_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len);

#endif
