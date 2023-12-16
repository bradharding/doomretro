/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#pragma once

#include <stdio.h>

#if defined(_WIN32)
#include <io.h>
#include <sys/stat.h>
#include <direct.h>
#endif

#if !defined(MAX_PATH)
#define MAX_PATH    260
#endif

typedef struct
{
    FILE    *fstream;
    bool    freedoom;
    char    path[MAX_PATH];
    int     type;
} wadfile_t;

// Open the specified file. Returns a pointer to a new wadfile_t
// handle for the WAD file, or NULL if it could not be opened.
wadfile_t *W_OpenFile(char *path);

// Close the specified WAD file.
void W_CloseFile(wadfile_t *wad);

// Read data from the specified file into the provided buffer. The
// data is read from the specified offset from the start of the file.
// Returns the number of bytes read.
size_t W_Read(wadfile_t *wad, unsigned int offset, void *buffer, size_t buffer_len);

bool W_WriteFile(char const *name, const void *source, size_t length);
size_t W_FileLength(FILE *handle);
