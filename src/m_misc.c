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

#include <errno.h>
#include <direct.h>

#include "doomdef.h"
#include "i_system.h"
#include "z_zone.h"

//
// Create a directory
//
void M_MakeDirectory(char *path)
{
    mkdir(path);
}

// Check if a file exists
boolean M_FileExists(char *filename)
{
    FILE        *fstream;

    fstream = fopen(filename, "r");

    if (fstream != NULL)
    {
        fclose(fstream);
        return true;
    }
    else
        // If we can't open because the file is a directory, the
        // "file" exists at least!
        return (errno == EISDIR);
}

//
// Determine the length of an open file.
//
long M_FileLength(FILE *handle)
{
    long        savedpos;
    long        length;

    // save the current position in the file
    savedpos = ftell(handle);

    // jump to the end and find the length
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);

    // go back to the old location
    fseek(handle, savedpos, SEEK_SET);

    return length;
}

char *M_ExtractFolder(char *path)
{
    char        *pos;
    char        *folder = (char *)malloc(strlen(path));

    strcpy(folder, path);

    pos = strrchr(folder, '\\');
    if (pos)
        *pos = '\0';

    return folder;
}

//
// M_WriteFile
//
boolean M_WriteFile(char *name, void *source, int length)
{
    FILE        *handle;
    int         count;

    handle = fopen(name, "wb");

    if (handle == NULL)
        return false;

    count = fwrite(source, 1, length, handle);
    fclose(handle);

    if (count < length)
        return false;

    return true;
}

//
// M_ReadFile
//
int M_ReadFile(char *name, byte **buffer)
{
    FILE        *handle;
    int         count, length;
    byte        *buf;

    handle = fopen(name, "rb");
    if (handle == NULL)
        I_Error("Couldn't read file %s", name);

    // find the size of the file by seeking to the end and
    // reading the current position
    length = M_FileLength(handle);

    buf = (byte *)Z_Malloc(length, PU_STATIC, NULL);
    count = fread(buf, 1, length, handle);
    fclose(handle);

    if (count < length)
        I_Error("Couldn't read file %s", name);

    *buffer = buf;
    return length;
}

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.
//
// The returned value must be freed with Z_Free after use.
char *M_TempFile(char *s)
{
    char *result;
    char *tempdir;

    // Check the TEMP environment variable to find the location.
    tempdir = getenv("TEMP");

    if (tempdir == NULL)
        tempdir = ".";

    result = (char *)Z_Malloc(strlen(tempdir) + strlen(s) + 2, PU_STATIC, 0);
    sprintf(result, "%s%c%s", tempdir, DIR_SEPARATOR, s);

    return result;
}

boolean M_StrToInt(const char *str, int *result)
{
    return (sscanf(str, " 0x%x", result) == 1
            || sscanf(str, " 0X%x", result) == 1
            || sscanf(str, " 0%o", result) == 1
            || sscanf(str, " %d", result) == 1);
}
