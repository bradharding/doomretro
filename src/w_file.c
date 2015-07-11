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

#include <stdio.h>

#include "doomtype.h"
#include "m_argv.h"
#include "w_file.h"

extern wad_file_class_t stdc_wad_file;

#if defined(WIN32)
extern wad_file_class_t win32_wad_file;
#endif

#if defined(HAVE_MMAP)
extern wad_file_class_t posix_wad_file;
#endif 

static wad_file_class_t *wad_file_classes[] =
{
#if defined(WIN32)
    &win32_wad_file,
#endif
#if defined(HAVE_MMAP)
    &posix_wad_file,
#endif
    &stdc_wad_file,
};

wad_file_t *W_OpenFile(char *path)
{
    wad_file_t  *result;
    int         i;

    //!
    // Use the OS's virtual memory subsystem to map WAD files
    // directly into memory.
    //
    if (!M_CheckParm("-mmap"))
        return stdc_wad_file.OpenFile(path);

    // Try all classes in order until we find one that works
    result = NULL;

    for (i = 0; i < arrlen(wad_file_classes); ++i)
    {
        result = wad_file_classes[i]->OpenFile(path);

        if (result)
            break;
    }

    return result;
}

void W_CloseFile(wad_file_t *wad)
{
    wad->file_class->CloseFile(wad);
}

size_t W_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len)
{
    return wad->file_class->Read(wad, offset, buffer, buffer_len);
}
