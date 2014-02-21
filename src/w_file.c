/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.
Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "doomtype.h"
#include "m_argv.h"

#include "i_system.h"
#include "w_file.h"
#include "z_zone.h"

typedef struct
{
    wad_file_t wad;
    HANDLE handle;
    HANDLE handle_map;
} win32_wad_file_t;

wad_file_class_t win32_wad_file;

static void MapFile(win32_wad_file_t *wad)
{
    wad->handle_map = CreateFileMapping(wad->handle,
                                        NULL,
                                        PAGE_WRITECOPY,
                                        0,
                                        0,
                                        NULL);

    wad->wad.mapped = MapViewOfFile(wad->handle_map,
                                    FILE_MAP_COPY,
                                    0, 0, 0);
}

unsigned int GetFileLength(HANDLE handle)
{
    DWORD result;

    result = SetFilePointer(handle, 0, NULL, FILE_END);

    if (result == INVALID_SET_FILE_POINTER)
    {
        I_Error("W_Win32_OpenFile: Failed to read file length");
    }

    return result;
}

wad_file_t *W_OpenFile(char *path)
{
    win32_wad_file_t *result;
    wchar_t wpath[MAX_PATH + 1];
    HANDLE handle;

    // Open the file:

    MultiByteToWideChar(CP_OEMCP, 0,
                        path, strlen(path) + 1,
                        wpath, sizeof(wpath));

    handle = CreateFileW(wpath,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    // Create a new win32_wad_file_t to hold the file handle.

    result = (win32_wad_file_t *)Z_Malloc(sizeof(win32_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &win32_wad_file;
    result->wad.length = GetFileLength(handle);
    result->handle = handle;

    // Try to map the file into memory with mmap:

    MapFile(result);

    return &result->wad;
}

void W_CloseFile(wad_file_t *wad)
{
    win32_wad_file_t *win32_wad;

    win32_wad = (win32_wad_file_t *)wad;

    // If mapped, unmap it.

    if (win32_wad->wad.mapped != NULL)
    {
        UnmapViewOfFile(win32_wad->wad.mapped);
    }

    if (win32_wad->handle_map != NULL)
    {
        CloseHandle(win32_wad->handle_map);
    }

    // Close the file

    if (win32_wad->handle != NULL)
    {
        CloseHandle(win32_wad->handle);
    }

    Z_Free(win32_wad);
}


size_t W_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len)
{
    win32_wad_file_t *win32_wad;
    DWORD bytes_read;
    DWORD result;

    win32_wad = (win32_wad_file_t *)wad;

    // Jump to the specified position in the file.

    result = SetFilePointer(win32_wad->handle, offset, NULL, FILE_BEGIN);

    if (result == INVALID_SET_FILE_POINTER)
    {
        I_Error("W_Win32_Read: Failed to set file pointer to %i", offset);
    }

    // Read into the buffer.

    if (!ReadFile(win32_wad->handle, buffer, buffer_len, &bytes_read, NULL))
    {
        I_Error("W_Win32_Read: Error reading from file");
    }

    return bytes_read;
}