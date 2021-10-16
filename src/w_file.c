/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2021 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2021 by Brad Harding <mailto:brad@doomretro.com>.

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

#if defined(_WIN32)
#include <windows.h>
#include <stdlib.h>
#endif

#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

#if defined(_WIN32)
static wchar_t *ConvertToUTF8(const char *str)
{
    int     wlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    wchar_t *wstr = (wchar_t *)malloc(wlen * sizeof(wchar_t));

    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, wlen);

    return wstr;
}

FILE *D_fopen(const char *filename, const char *mode)
{
    wchar_t *wname = ConvertToUTF8(filename);
    wchar_t *wmode = ConvertToUTF8(mode);
    FILE    *file = _wfopen(wname, wmode);

    if (wname)
        free(wname);

    if (wmode)
        free(wmode);

    return file;
}

int D_remove(const char *path)
{
    wchar_t *wpath = ConvertToUTF8(path);
    int     result = _wremove(wpath);

    if (wpath)
        free(wpath);

    return result;
}

int D_stat(const char *path, struct stat *buffer)
{
    wchar_t         *wpath = ConvertToUTF8(path);
    struct _stat    wbuffer;
    int             result = _wstat(wpath, &wbuffer);

    buffer->st_mode = wbuffer.st_mode;
    buffer->st_mtime = wbuffer.st_mtime;
    buffer->st_size = wbuffer.st_size;

    if (wpath)
        free(wpath);

    return result;
}

int D_mkdir(const char *dirname)
{
    wchar_t *wpath = ConvertToUTF8(dirname);
    int     result = _wmkdir(wpath);

    if (wpath)
        free(wpath);

    return result;
}
#endif

wadfile_t *W_OpenFile(char *path)
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

dboolean W_WriteFile(char const *name, const void *source, size_t length)
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
