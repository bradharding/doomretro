/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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

#include <errno.h>
#include <stdarg.h>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#if defined(_MSC_VER)
#include <direct.h>
#endif
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "doomdef.h"
#include "m_fixed.h"
#include "m_misc.h"
#include "i_system.h"
#include "z_zone.h"

#if defined(__MACOSX__)
#include "version.h"
#import <Cocoa/Cocoa.h>
#endif

#if !defined(MAX_PATH)
#define MAX_PATH        260
#endif

//
// Create a directory
//
void M_MakeDirectory(const char *path)
{
#if defined(WIN32)
    mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

// Check if a file exists
dboolean M_FileExists(const char *filename)
{
    FILE        *fstream;

    fstream = fopen(filename, "r");

    if (fstream)
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

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.
dboolean M_StringCopy(char *dest, char *src, size_t dest_size)
{
    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
        return (src[strlen(dest)] == '\0');
    }
    else
        return false;
}

char *M_ExtractFolder(char *path)
{
    char        *pos;
    char        *folder;

    if (!path[0])
        return "";

    folder = (char *)malloc(MAX_PATH);

    M_StringCopy(folder, path, MAX_PATH);

    pos = strrchr(folder, DIR_SEPARATOR);
    if (pos)
        *pos = '\0';

    return folder;
}

char *M_GetAppDataFolder(void)
{
    //On OSX, store generated application files in ~/Library/Application Support/DOOM Retro.
#if defined(__MACOSX__)
    NSFileManager      *manager = [NSFileManager defaultManager];
    NSURL              *baseAppSupportURL = [manager URLsForDirectory: NSApplicationSupportDirectory
                           inDomains: NSUserDomainMask].firstObject;
    NSURL              *appSupportURL = [baseAppSupportURL URLByAppendingPathComponent:
                           @PACKAGE_NAME];

    return appSupportURL.fileSystemRepresentation;
#else
    // On Windows and Linux, store generated files in the same folder as the executable.
    // TODO: on Windows this should probably use %APPDATA%/PACKAGE_NAME,
    // and on Unix it should probably use somewhere within ~.
    return M_GetExecutableFolder();
#endif
}

char *M_GetResourceFolder(void)
{
#if defined(__MACOSX__)
    // On OSX, load resources from the Contents/Resources folder within the application bundle.
    NSURL       *resourceURL = [NSBundle mainBundle].resourceURL;

    return resourceURL.fileSystemRepresentation;
#else
    // On Windows and Linux, load resources from the same folder as the executable.
    return M_GetExecutableFolder();
#endif
}

char *M_GetExecutableFolder(void)
{
#if defined(WIN32)
    char        *pos;
    char        *folder = (char *)malloc(MAX_PATH);
    TCHAR       buffer[MAX_PATH];

    GetModuleFileName(NULL, buffer, MAX_PATH);
    M_StringCopy(folder, buffer, MAX_PATH);

    pos = strrchr(folder, '\\');
    if (pos)
        *pos = '\0';

    // FIXME: the string returned here is allocated on the heap,
    // but that is inconsistent with the non-windows branch below
    // and no caller of this method bothers to free the returned
    // string anyway.
    // Instead this method should return an address on the stack,
    // and make it the caller's responsibility to copy the string
    // if it's needed beyond the stack scope.
    return folder;
#else
    return ".";
#endif
}

//
// M_WriteFile
//
dboolean M_WriteFile(char *name, void *source, int length)
{
    FILE        *handle = fopen(name, "wb");
    int         count;

    if (!handle)
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
    FILE        *handle = fopen(name, "rb");
    int         count;
    int         length;
    byte        *buf;

    if (!handle)
    {
        I_Error("Couldn't read file %s", name);
        return 0;
    }

    // find the size of the file by seeking to the end and
    // reading the current position
    length = M_FileLength(handle);

    buf = Z_Malloc(length, PU_STATIC, NULL);
    count = fread(buf, 1, length, handle);
    fclose(handle);

    if (count < length)
        I_Error("Couldn't read file %s", name);

    *buffer = buf;
    return length;
}

// Return a newly-malloced string with all the strings given as arguments
// concatenated together.
char *M_StringJoin(char *s, ...)
{
    char        *result;
    char        *v;
    va_list     args;
    size_t      result_len = strlen(s) + 1;

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, char *);
        if (!v)
            break;

        result_len += strlen(v);
    }
    va_end(args);

    result = malloc(result_len);

    if (!result)
    {
        I_Error("M_StringJoin: Failed to allocate new string.");
        return NULL;
    }

    M_StringCopy(result, s, result_len);

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, char *);
        if (!v)
            break;

        strncat(result, v, result_len);
    }
    va_end(args);

    return result;
}

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.
//
// The returned value must be freed with Z_Free after use.
char *M_TempFile(char *s)
{
    char *tempdir;

#if defined(WIN32)
    // Check the TEMP environment variable to find the location.
    tempdir = getenv("TEMP");

    if (!tempdir)
        tempdir = ".";
#else
    // In Unix, just use /tmp.
    tempdir = "/tmp";
#endif

    return M_StringJoin(tempdir, DIR_SEPARATOR_S, s, NULL);
}

dboolean M_StrToInt(const char *str, int *result)
{
    return (sscanf(str, " 0x%2x", result) == 1 || sscanf(str, " 0X%2x", result) == 1
        || sscanf(str, " 0%3o", result) == 1 || sscanf(str, " %10d", result) == 1);
}

//
// M_StrCaseStr
//
// Case-insensitive version of strstr()
//
char *M_StrCaseStr(char *haystack, char *needle)
{
    unsigned int        haystack_len = strlen(haystack);
    unsigned int        needle_len = strlen(needle);
    unsigned int        len;
    unsigned int        i;

    if (haystack_len < needle_len)
        return NULL;

    len = haystack_len - needle_len;

    for (i = 0; i <= len; ++i)
        if (!strncasecmp(haystack + i, needle, needle_len))
            return haystack + i;

    return NULL;
}

char *stristr(char *ch1, char *ch2)
{
    char        *chN1 = strdup(ch1);
    char        *chN2 = strdup(ch2);
    char        *chRet = NULL;

    if (chN1 && chN2)
    {
        char    *chNdx = chN1;

        while (*chNdx)
        {
            *chNdx = (char)tolower(*chNdx);
            chNdx++;
        }
        chNdx = chN2;
        while (*chNdx)
        {
            *chNdx = (char)tolower(*chNdx);
            chNdx++;
        }
        chNdx = strstr(chN1, chN2);
        if (chNdx)
            chRet = ch1 + (chNdx - chN1);
    }

    free(chN1);
    free(chN2);

    return chRet;
}

//
// String replace function.
//
char *M_StringReplace(char *haystack, char *needle, char *replacement)
{
    static char buffer[4096];
    char        *p;

    if (!(p = stristr(haystack, needle)))
        return haystack;

    strncpy(buffer, haystack, p - haystack);
    buffer[p - haystack] = '\0';

    sprintf(buffer + (p - haystack), "%s%s", replacement, p + strlen(needle));

    return buffer;
}

// Returns true if 'str1' and 'str2' are the same.
// (Case-insensitive, return value reverse of strcasecmp() to avoid confusion.
dboolean M_StringCompare(const char *str1, const char *str2)
{
    return !strcasecmp(str1, str2);
}

// Returns true if 's' begins with the specified prefix.
dboolean M_StringStartsWith(char *s, char *prefix)
{
    return (strlen(s) > strlen(prefix) && strncasecmp(s, prefix, strlen(prefix)) == 0);
}

// Returns true if 's' ends with the specified suffix.
dboolean M_StringEndsWith(char *s, char *suffix)
{
    return (strlen(s) >= strlen(suffix) && !M_StringCompare(s + strlen(s) - strlen(suffix),
        suffix) == 0);
}

// Safe, portable vsnprintf().
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args)
{
    int result;

    if (buf_len < 1)
        return 0;

    // Windows (and other OSes?) has a vsnprintf() that doesn't always
    // append a trailing \0. So we must do it, and write into a buffer
    // that is one byte shorter; otherwise this function is unsafe.
    result = vsnprintf(buf, buf_len, s, args);

    // If truncated, change the final char in the buffer to a \0.
    // A negative result indicates a truncated buffer on Windows.
    if (result < 0 || (unsigned int)result >= buf_len)
    {
        buf[buf_len - 1] = '\0';
        result = buf_len - 1;
    }

    return result;
}

// Safe, portable snprintf().
int M_snprintf(char *buf, size_t buf_len, const char *s, ...)
{
    va_list     args;
    int         result;

    va_start(args, s);
    result = M_vsnprintf(buf, buf_len, s, args);
    va_end(args);
    return result;
}

#if !defined(strndup)
char *strndup(const char *s, size_t n)
{
    size_t      len = strnlen(s, n);
    char        *new = malloc(len + 1);

    if (!new)
        return NULL;

    new[len] = '\0';
    return memcpy(new, s, len);
}
#endif

char *M_SubString(const char *str, size_t begin, size_t len)
{
    if (!str|| !strlen(str) || strlen(str) < begin || strlen(str) < begin + len)
        return 0;

    return strndup(str + begin, len);
}

char *uppercase(const char *str)
{
    char        *newstr;
    char        *p;

    p = newstr = strdup(str);
    while ((*p = toupper(*p)))
        ++p;

    return newstr;
}

char *lowercase(const char *str)
{
    char        *newstr;
    char        *p;

    p = newstr = strdup(str);
    while ((*p = tolower(*p)))
        ++p;

    return newstr;
}

char *titlecase(const char *str)
{
    char        *newstr = strdup(str);
    size_t      len = strlen(newstr);

    if (len > 1)
    {
        size_t  i;

        newstr[0] = toupper(newstr[0]);
        for (i = 1; i < len; ++i)
            if (!isalnum((unsigned char)newstr[i - 1]) && isalnum((unsigned char)newstr[i]))
                newstr[i] = toupper(newstr[i]);
    }

    return newstr;
}

char *formatsize(const char *str)
{
    char        *newstr = strdup(str);
    size_t      len = strlen(newstr);

    if (len > 1)
    {
        size_t  i;

        for (i = 1; i < len; ++i)
            if (newstr[i] == 'x')
            {
                newstr[i] = 215;
                break;
            }
    }

    return newstr;
}

char *commify(int value)
{
    char        result[64];

    M_snprintf(result, sizeof(result), "%i", value);
    if (ABS(value) >= 1000)
    {
        char        *pt;
        int         n;

        for (pt = result; *pt && *pt != '.'; pt++);
        n = result + sizeof(result) - pt;
        do
        {
            pt -= 3;
            if (pt > result)
            {
                memmove(pt + 1, pt, n);
                *pt = ',';
                n += 4;
            }
            else
                break;
        } while (1);
    }
    return strdup(result);
}

dboolean wildcard(char *input, char *pattern)
{
    int i, z;

    if (pattern[0] == '\0')
        return true;

    for (i = 0; pattern[i] != '\0'; i++)
    {
        if (pattern[i] == '\0')
            return false;
        else if (pattern[i] == '?')
            continue;
        else if (pattern[i] == '*')
        {
            for (z = i; input[z] != '\0'; z++)
                if (wildcard(input + z, pattern + i + 1))
                    return true;
            return false;
        }
        else if (pattern[i] != input[i])
            return false;
    }
    return true;
}

int gcd(int a, int b)
{
    return (!b ? a : gcd(b, a % b));
}

char *removespaces(const char *input)
{
    char        *p = malloc(strlen(input) + 1);

    if (p)
    {
        char    *p2 = p;

        while (*input != '\0')
            if (isalnum((unsigned char)*input))
                *p2++ = *input++;
            else
                ++input;
        *p2 = '\0';
    }

    return p;
}

char *removenewlines(const char *input)
{
    char        *p = malloc(strlen(input) + 1);

    if (p)
    {
        char    *p2 = p;

        while (*input != '\0')
            if (*input != '\n')
                *p2++ = *input++;
            else
                ++input;
        *p2 = '\0';
    }

    return p;
}

char *makevalidfilename(const char *input)
{
    char        *newstr = strdup(input);
    size_t      len = strlen(newstr);
    size_t      i;

    for (i = 0; i < len; ++i)
        if (strchr("\\/:?\"<>|", newstr[i]))
            newstr[i] = ' ';

    return newstr;
}

const char *leafname(const char *path)
{
    char        cc;
    const char  *ptr = path;

    do
    {
        cc = *ptr++;
        if (cc == '\\' || cc == '/')
            path = ptr;
    } while (cc);

    return path;
}

char *removeext(const char *file)
{
    char        *newstr = strdup(file);
    char        *lastdot = strrchr(newstr, '.');

    *lastdot = '\0';

    return newstr;
}

dboolean isvowel(const char ch)
{
    return (!!strchr("aeiou", ch));
}

static const char       *sizes[] = { "MB", "KB", " bytes" };

char *convertsize(const int size)
{
    char        *result = (char *)malloc(sizeof(char) * 20);
    int         multiplier = 1024ULL * 1024ULL;
    int         i;

    for (i = 0; i < sizeof(sizes) / sizeof(*sizes); i++, multiplier /= 1024)
    {
        if (size < multiplier)
            continue;
        if (!(size % multiplier))
            M_snprintf(result, 20, "%s%s", commify(size / multiplier), sizes[i]);
        else
            M_snprintf(result, 20, "%.2f%s", (float)size / multiplier, sizes[i]);
        return result;
    }
    strcpy(result, "0");

    return result;
}
