/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

wadfile_t *W_OpenFile(const char *path)
{
    wadfile_t   *result;
    FILE        *fstream = fopen(path, "rb");

    if (!fstream)
        return NULL;

    // Create a new wadfile_t to hold the file handle.
    result = Z_Malloc(sizeof(wadfile_t), PU_STATIC, NULL);
    result->fstream = fstream;

    return result;
}

void W_CloseFile(wadfile_t *wad)
{
    fclose(wad->fstream);
    Z_Free(wad);
}

// Read data from the specified position in the file into the
// provided buffer. Returns the number of bytes read.
size_t W_Read(wadfile_t *wad, unsigned int offset, void *buffer, size_t buffer_len)
{
    // Jump to the specified position in the file.
    fseek(wad->fstream, offset, SEEK_SET);

    // Read into the buffer.
    return fread(buffer, 1, buffer_len, wad->fstream);
}

bool W_WriteFile(char const *name, const void *source, size_t length)
{
    FILE    *fstream = fopen(name, "wb");

    if (!fstream)
        return false;

    length = (fwrite(source, 1, length, fstream) == length);
    fclose(fstream);

    if (!length)
        remove(name);

    return !!length;
}

size_t W_FileLength(FILE *handle)
{
    long savedpos;
    long length;

    // save the current position in the file
    savedpos = ftell(handle);

    // jump to the end and find the length
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);

    // go back to the old location
    fseek(handle, savedpos, SEEK_SET);

    return length;
}
