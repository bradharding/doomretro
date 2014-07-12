/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

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
#include <stdarg.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#ifdef _MSC_VER
#include <direct.h>
#endif
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "doomdef.h"
#include "m_misc.h"
#include "i_system.h"
#include "z_zone.h"

//
// Create a directory
//
void M_MakeDirectory(char *path)
{
#ifdef WIN32
    mkdir(path);
#else
    mkdir(path, 0755);
#endif
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
    char *tempdir;

#ifdef WIN32

    // Check the TEMP environment variable to find the location.
    tempdir = getenv("TEMP");

    if (tempdir == NULL)
        tempdir = ".";
#else
    // In Unix, just use /tmp.
    tempdir = "/tmp";
#endif

    return M_StringJoin(tempdir, DIR_SEPARATOR_S, s, NULL);
}

boolean M_StrToInt(const char *str, int *result)
{
    return (sscanf(str, " 0x%x", result) == 1
            || sscanf(str, " 0X%x", result) == 1
            || sscanf(str, " 0%o", result) == 1
            || sscanf(str, " %d", result) == 1);
}

void M_ForceUppercase(char *text)
{
    char        *p;

    for (p = text; *p != '\0'; ++p)
        *p = toupper(*p);
}

//
// M_StrCaseStr
//
// Case-insensitive version of strstr()
//
char *M_StrCaseStr(char *haystack, char *needle)
{
    unsigned int        haystack_len;
    unsigned int        needle_len;
    unsigned int        len;
    unsigned int        i;

    haystack_len = strlen(haystack);
    needle_len = strlen(needle);

    if (haystack_len < needle_len)
        return NULL;

    len = haystack_len - needle_len;

    for (i = 0; i <= len; ++i)
        if (!strncasecmp(haystack + i, needle, needle_len))
            return haystack + i;

    return NULL;
}

//
// String replace function.
//
char *M_StringReplace(char *haystack, char *needle, char *replacement)
{
    char        *result, *p;
    char        *dst;
    size_t      needle_len = strlen(needle);
    size_t      result_len, dst_len;

    // Iterate through occurrences of 'needle' and calculate the size of
    // the new string.
    result_len = strlen(haystack) + 1;
    p = haystack;

    for (;;)
    {
        p = strstr(p, needle);
        if (p == NULL)
            break;

        p += needle_len;
        result_len += strlen(replacement) - needle_len;
    }

    // Construct new string.

    result = malloc(result_len);
    if (result == NULL)
    {
        I_Error("M_StringReplace: Failed to allocate new string");
        return NULL;
    }

    dst = result; dst_len = result_len;
    p = haystack;

    while (*p != '\0')
        if (!strncmp(p, needle, needle_len))
        {
            M_StringCopy(dst, replacement, dst_len);
            p += needle_len;
            dst += strlen(replacement);
            dst_len -= strlen(replacement);
        }
        else
        {
            *dst = *p;
            ++dst;
            --dst_len;
            ++p;
        }

    return result;
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.
boolean M_StringCopy(char *dest, char *src, size_t dest_size)
{
    strncpy(dest, src, dest_size);
    dest[dest_size - 1] = '\0';
    return strlen(dest) == strlen(src);
}

// Returns true if 's' begins with the specified prefix.
boolean M_StringStartsWith(char *s, char *prefix)
{
    return (strlen(s) > strlen(prefix) && strncmp(s, prefix, strlen(prefix)) == 0);
}

// Returns true if 's' ends with the specified suffix.
boolean M_StringEndsWith(char *s, char *suffix)
{
    return (strlen(s) >= strlen(suffix) && strcmp(s + strlen(s) - strlen(suffix), suffix) == 0);
}

// Return a newly-malloced string with all the strings given as arguments
// concatenated together.
char *M_StringJoin(char *s, ...)
{
    char *result, *v;
    va_list args;
    size_t result_len;

    result_len = strlen(s) + 1;

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, char *);
        if (v == NULL)
            break;

        result_len += strlen(v);
    }
    va_end(args);

    result = malloc(result_len);

    if (result == NULL)
    {
        I_Error("M_StringJoin: Failed to allocate new string.");
        return NULL;
    }

    M_StringCopy(result, s, result_len);

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, char *);
        if (v == NULL)
            break;

        strncat(result, v, result_len);
    }
    va_end(args);

    return result;
}

// Safe, portable vsnprintf().
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args)
{
    int result;

    if (buf_len < 1)
    {
        return 0;
    }

    // Windows (and other OSes?) has a vsnprintf() that doesn't always
    // append a trailing \0. So we must do it, and write into a buffer
    // that is one byte shorter; otherwise this function is unsafe.
    result = vsnprintf(buf, buf_len, s, args);

    // If truncated, change the final char in the buffer to a \0.
    // A negative result indicates a truncated buffer on Windows.
    if (result < 0 || (unsigned)result >= buf_len)
    {
        buf[buf_len - 1] = '\0';
        result = buf_len - 1;
    }

    return result;
}

// Safe, portable snprintf().
int M_snprintf(char *buf, size_t buf_len, const char *s, ...)
{
    va_list args;
    int result;
    va_start(args, s);
    result = M_vsnprintf(buf, buf_len, s, args);
    va_end(args);
    return result;
}
