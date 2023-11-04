/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#if defined(_WIN32)
#pragma warning( disable : 4091 )

#include <Windows.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_MSC_VER)
#include <direct.h>
#endif

#else
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#include "c_console.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "version.h"
#include "w_file.h"

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>

#include <dirent.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <errno.h>
#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#include <sys/sysctl.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#elif defined(__linux__) || defined(__HAIKU__) || defined(__sun)
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>

#if defined(__HAIKU__)
#include <FindDirectory.h>
#endif
#endif

// Create a directory
void M_MakeDirectory(const char *path)
{
#if defined(_WIN32)
    mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

// Check if a file exists
bool M_FileExists(const char *filename)
{
    FILE    *fstream = fopen(filename, "r");

    if (fstream)
    {
        fclose(fstream);
        return true;
    }

    return false;
}

#if !defined(_WIN32) && !defined(__APPLE__)
// Check if a file exists by probing for common case variation of its filename.
// Returns a newly allocated string that the caller is responsible for freeing.
char *M_FileCaseExists(const char *path)
{
    char    *path_dup = M_StringDuplicate(path);
    char    *filename;
    char    *ufilename;
    char    *ext;

    // actual path
    if (M_FileExists(path_dup))
        return path_dup;

    if ((filename = strrchr(path_dup, DIR_SEPARATOR)))
        filename++;
    else
        filename = path_dup;

    // lowercase filename, e.g. doom2.wad
    lowercase(filename);

    if (M_FileExists(path_dup))
        return path_dup;

    // uppercase filename, e.g. DOOM2.WAD
    ufilename = uppercase(filename);

    // uppercase basename with lowercase extension, e.g. DOOM2.wad
    if ((ext = strrchr(path_dup, '.')) && ext > ufilename)
    {
        lowercase(ext + 1);

        if (M_FileExists(path_dup))
        {
            free(ufilename);
            return path_dup;
        }
    }

    // lowercase filename with uppercase first letter, e.g. Doom2.wad
    if (strlen(ufilename) > 1)
    {
        lowercase(ufilename + 1);

        if (M_FileExists(path_dup))
        {
            free(ufilename);
            return path_dup;
        }
    }

    // no luck
    free(ufilename);
    free(path_dup);
    return NULL;
}
#endif

// Check if a folder exists
bool M_FolderExists(const char *folder)
{
    struct stat status;

    return (!stat(folder, &status) && (status.st_mode & S_IFDIR));
}

// Safe string copy function that works like OpenBSD's strlcpy().
void M_StringCopy(char *dest, const char *src, const size_t dest_size)
{
    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
    }
}

char *M_ExtractFolder(const char *path)
{
    char    *pos;
    char    *folder;

    if (!*path)
        return "";

    folder = M_StringDuplicate(path);

    if ((pos = strrchr(folder, DIR_SEPARATOR)))
        *pos = '\0';

    return folder;
}

char *M_GetAppDataFolder(void)
{
    char    *executablefolder = M_GetExecutableFolder();

#if defined(_WIN32)
    return executablefolder;
#else
    // On Linux and macOS, if ../share/doomretro doesn't exist then we're dealing with
    // a portable installation, and we write doomretro.cfg to the executable directory.
    char    *resourcefolder = M_StringJoin(executablefolder,
                DIR_SEPARATOR_S ".." DIR_SEPARATOR_S "share" DIR_SEPARATOR_S DOOMRETRO, NULL);
    DIR     *resourcedir = opendir(resourcefolder);

    free(resourcefolder);

    if (resourcedir)
    {
#if defined(__APPLE__)
        // On macOS, store generated application files in ~/Library/Application Support/DOOM Retro.
        NSFileManager   *manager = [NSFileManager defaultManager];
        NSURL           *baseAppSupportURL = [manager URLsForDirectory : NSApplicationSupportDirectory
                            inDomains : NSUserDomainMask].firstObject;
        NSURL           *appSupportURL = [baseAppSupportURL URLByAppendingPathComponent : @DOOMRETRO_NAME];

        closedir(resourcedir);

        return (char *)appSupportURL.fileSystemRepresentation;
#else
        // On Linux, store generated application files in /home/<username>/.config/doomretro
        char    *buffer = getenv("HOME");

        if (!buffer)
            buffer = getpwuid(getuid())->pw_dir;

        closedir(resourcedir);
        free(executablefolder);

#if defined(__HAIKU__)
        return M_StringJoin(buffer, DIR_SEPARATOR_S "config" DIR_SEPARATOR_S "settings" DIR_SEPARATOR_S DOOMRETRO, NULL);
#else
        return M_StringJoin(buffer, DIR_SEPARATOR_S ".config" DIR_SEPARATOR_S DOOMRETRO, NULL);
#endif
#endif
    }
    else
        return executablefolder;
#endif
}

char *M_GetResourceFolder(void)
{
    char    *executablefolder = M_GetExecutableFolder();

#if !defined(_WIN32)
    // On Linux and macOS, first assume that the executable is in ../bin and
    // try to load resources from ../share/doomretro.
    char    *resourcefolder = M_StringJoin(executablefolder,
                DIR_SEPARATOR_S ".." DIR_SEPARATOR_S "share" DIR_SEPARATOR_S DOOMRETRO, NULL);
    DIR     *resourcedir = opendir(resourcefolder);

    if (resourcedir)
    {
        closedir(resourcedir);
        free(executablefolder);

        return resourcefolder;
    }

#if defined(__APPLE__)
    // On macOS, load resources from the Contents/Resources folder within the application bundle
    // if ../share/doomretro is not available.
    NSURL   *resourceURL = [NSBundle mainBundle].resourceURL;

    return (char *)resourceURL.fileSystemRepresentation;
#else
    free(resourcefolder);
    // And on Linux, fall back to the same folder as the executable.
    return executablefolder;
#endif

#else
    // On Windows, load resources from the same folder as the executable.
    return executablefolder;
#endif
}

char *M_GetExecutableFolder(void)
{
#if defined(_WIN32)
    char    *folder = malloc(MAX_PATH);

    if (folder)
    {
        char    *pos;

        GetModuleFileName(NULL, folder, MAX_PATH);

        if ((pos = strrchr(folder, DIR_SEPARATOR)))
            *pos = '\0';
    }

    return folder;
#elif defined(__linux__) || defined(__NetBSD__) || defined(__sun)
    char    exe[MAX_PATH];

#if defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", exe, MAX_PATH - 1);
#elif defined(__NetBSD__)
    ssize_t len = readlink("/proc/curproc/exe", exe, MAX_PATH - 1);
#elif defined(__sun)
    ssize_t len = readlink("/proc/self/path/a.out", exe, MAX_PATH - 1);
#endif

    if (len == -1)
    {
        strcpy(exe, ".");
        return M_StringDuplicate(exe);
    }
    else
    {
        exe[len] = '\0';
        return M_StringDuplicate(dirname(exe));
    }
#elif defined(__FreeBSD__) || defined(__DragonFly__)
    char    exe[MAX_PATH];
    size_t  len = MAX_PATH;
    int     mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };

    if (!sysctl(mib, 4, exe, &len, NULL, 0))
    {
        exe[len] = '\0';
        return M_StringDuplicate(dirname(exe));
    }
    else
    {
        strcpy(exe, ".");
        return M_StringDuplicate(exe);
    }
#elif defined(__APPLE__)
    char        exe[MAX_PATH];
    uint32_t    len = MAX_PATH;

    if (_NSGetExecutablePath(exe, &len))
    {
        strcpy(exe, ".");
        return M_StringDuplicate(exe);
    }

    return M_StringDuplicate(dirname(exe));
#elif defined(__HAIKU__)
    char    exe[MAX_PATH];

    exe[0] = '\0';

    if (find_path(B_APP_IMAGE_SYMBOL, B_FIND_PATH_IMAGE_PATH, NULL, exe, MAX_PATH) == B_OK)
        return M_StringDuplicate(dirname(exe));

    strcpy(exe, ".");
    return M_StringDuplicate(exe);
#else
    char    *folder = malloc(2);

    strcpy(folder, ".");
    return folder;
#endif
}

char *M_TempFile(char *s)
{
    char    *tempdir;

#if defined(_WIN32)
    tempdir = getenv("TEMP");

    if (!tempdir)
        tempdir = ".";
#else
    tempdir = "/tmp";
#endif

    return M_StringJoin(tempdir, DIR_SEPARATOR_S, s, NULL);
}

// Return a newly-malloced string with all the strings given as arguments
// concatenated together.
char *M_StringJoin(const char *s, ...)
{
    char        *result;
    const char  *v;
    va_list     args;
    size_t      result_len = strlen(s) + 1;

    va_start(args, s);

    while (true)
    {
        if (!(v = va_arg(args, const char *)))
            break;

        result_len += strlen(v);
    }

    va_end(args);

    if (!(result = malloc(result_len)))
        return NULL;

    M_StringCopy(result, s, result_len);

    va_start(args, s);

    while (true)
    {
        if (!(v = va_arg(args, const char *)))
            break;

        strncat(result, v, result_len);
    }

    va_end(args);
    return result;
}

bool M_StrToInt(const char *str, int *result)
{
    return (sscanf(str, " 0x%2x", (unsigned int *)result) == 1
        || sscanf(str, " 0X%2x", (unsigned int *)result) == 1
        || sscanf(str, " 0%3o", (unsigned int *)result) == 1
        || sscanf(str, " %12d", result) == 1);
}

// Case-insensitive version of strstr()
const char *M_StrCaseStr(const char *haystack, const char *needle)
{
    const int   haystack_len = (int)strlen(haystack);
    const int   needle_len = (int)strlen(needle);
    int         len;

    if (haystack_len < needle_len)
        return NULL;

    len = haystack_len - needle_len;

    for (int i = 0; i <= len; i++)
        if (!strncasecmp(haystack + i, needle, needle_len))
            return (haystack + i);

    return NULL;
}

#if !defined(stristr)
static char *stristr(char *ch1, const char *ch2)
{
    char    *chN1 = M_StringDuplicate(ch1);
    char    *chN2 = M_StringDuplicate(ch2);
    char    *chRet = NULL;

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

        if ((chNdx = strstr(chN1, chN2)))
            chRet = ch1 + (chNdx - chN1);
    }

    free(chN1);
    free(chN2);

    return chRet;
}
#endif

// String replace function.
char *M_StringReplaceFirst(char *haystack, const char *needle, const char *replacement)
{
    static char buffer[4096];
    char        *p;

    if (!(p = stristr(haystack, (char *)needle)))
        return haystack;

    strncpy(buffer, haystack, p - haystack);
    buffer[p - haystack] = '\0';
    sprintf(buffer + (p - haystack), "%s%s", replacement, p + strlen(needle));

    return buffer;
}

#if !defined(strrstr)
char *strrstr(const char *haystack, const char *needle)
{
    char    *r = NULL;

    if (!needle[0])
        return (char *)haystack + strlen(haystack);

    while (true)
    {
        char    *p = strstr(haystack, needle);

        if (!p)
            return r;

        r = p;
        haystack = p + 1;
    }
}
#endif

char *M_StringReplaceLast(char *haystack, const char *needle, const char *replacement)
{
    static char buffer[4096];
    char        *p;

    if (!(p = strrstr(haystack, (char *)needle)))
        return haystack;

    strncpy(buffer, haystack, p - haystack);
    buffer[p - haystack] = '\0';
    sprintf(buffer + (p - haystack), "%s%s", replacement, p + strlen(needle));

    return buffer;
}

void M_StringReplaceAll(char *haystack, const char *needle, const char *replacement, bool usecase)
{
    char        buffer[1024] = "";
    char        *insert_point = &buffer[0];
    char        *temp = haystack;
    const int   needle_len = (int)strlen(needle);
    const int   repl_len = (int)strlen(replacement);

    while (true)
    {
        char    *p = (usecase ? strstr(temp, (char *)needle) : stristr(temp, (char *)needle));

        if (!p)
        {
            strcpy(insert_point, temp);
            break;
        }

        memcpy(insert_point, temp, p - temp);
        insert_point += p - temp;

        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;

        temp = p + needle_len;
    }

    strcpy(haystack, buffer);
}

// Safe version of strdup() that checks the string was successfully allocated.
char *M_StringDuplicate(const char *orig)
{
    char    *result = strdup(orig);

    if (!result)
        I_Error("Failed to duplicate string.");

    return result;
}

// Returns true if str1 and str2 are the same.
// (Case-insensitive, return value reverse of strcasecmp() to avoid confusion.
bool M_StringCompare(const char *str1, const char *str2)
{
    return !strcasecmp(str1, str2);
}

// Returns true if string begins with the specified prefix.
bool M_StringStartsWith(const char *s, const char *prefix)
{
    const size_t    len = strlen(prefix);

    return (strlen(s) >= len && !strncasecmp(s, prefix, len));
}

// Returns true if string ends with the specified suffix.
bool M_StringEndsWith(const char *s, const char *suffix)
{
    const size_t    len1 = strlen(s);
    const size_t    len2 = strlen(suffix);

    return (len1 >= len2 && M_StringCompare(s + len1 - len2, suffix));
}

// Safe, portable vsnprintf().
void M_vsnprintf(char *buf, int buf_len, const char *s, va_list args)
{
    if (buf_len >= 1)
    {
        // Windows (and other OSes?) have a vsnprintf() that doesn't always
        // append a trailing \0. So we must do it, and write into a buffer
        // that is one byte shorter; otherwise this function is unsafe.
        const int   result = vsnprintf(buf, buf_len, s, args);

        // If truncated, change the final char in the buffer to a \0.
        // A negative result indicates a truncated buffer on Windows.
        if (result < 0 || result >= buf_len)
            buf[buf_len - 1] = '\0';
    }
}

// Safe, portable snprintf().
void M_snprintf(char *buf, int buf_len, const char *s, ...)
{
    va_list args;

    va_start(args, s);
    M_vsnprintf(buf, buf_len, s, args);
    va_end(args);
}

#if !defined(strndup)
char *strndup(const char *s, size_t n)
{
    const size_t    len = strnlen(s, n);
    char            *new = malloc(len + 1);

    if (!new)
        return NULL;

    new[len] = '\0';
    return memcpy(new, s, len);
}
#endif

char *M_SubString(const char *str, size_t begin, size_t len)
{
    const size_t    length = strlen(str);

    if (!length || length < begin || length < begin + len)
        return 0;

    return strndup(str + begin, len);
}

char *uppercase(const char *str)
{
    char    *newstr;
    char    *p = newstr = M_StringDuplicate(str);

    while ((*p = toupper(*p)))
        p++;

    return newstr;
}

char *lowercase(char *str)
{
    for (char *p = str; *p; p++)
        *p = tolower(*p);

    return str;
}

char *titlecase(const char *str)
{
    char        *newstr = M_StringDuplicate(str);
    const int   len = (int)strlen(newstr);

    if (len > 0)
    {
        newstr[0] = toupper(newstr[0]);

        if (len > 1)
            for (int i = 1; i < len; i++)
                if ((newstr[i - 1] != '\'' || (i >= 2 && newstr[i - 2] == ' '))
                    && !isalnum((unsigned char)newstr[i - 1])
                    && isalnum((unsigned char)newstr[i]))
                    newstr[i] = toupper(newstr[i]);
    }

    return newstr;
}

char *sentencecase(const char *str)
{
    char    *newstr = M_StringDuplicate(str);

    if (newstr[0] != '\0')
        newstr[0] = toupper(newstr[0]);

    return newstr;
}

bool isuppercase(const char *str)
{
    const int   len = (int)strlen(str);

    for (int i = 0; i < len; i++)
        if (!isupper(str[i]))
            return false;

    return true;
}

bool islowercase(const char *str)
{
    const int   len = (int)strlen(str);

    for (int i = 0; i < len; i++)
        if (!islower(str[i]))
            return false;

    return true;
}

char *commify(int64_t value)
{
    char    result[64];

    M_snprintf(result, sizeof(result), "%" PRIi64, value);

    if (value <= -1000 || value >= 1000)
    {
        char    *pt;
        size_t  n;

        for (pt = result; *pt && *pt != '.'; pt++);

        n = result + sizeof(result) - pt;

        while (true)
            if ((pt -= 3) > result)
            {
                memmove(pt + 1, pt, n);
                *pt = ',';
                n += 4;
            }
            else
                break;
    }

    return M_StringDuplicate(result);
}

char *commifystat(uint64_t value)
{
    char    result[64];

    M_snprintf(result, sizeof(result), "%" PRIu64, value);

    if (value >= 1000)
    {
        char    *pt;
        size_t  n;

        for (pt = result; *pt && *pt != '.'; pt++);

        n = result + sizeof(result) - pt;

        while (true)
            if ((pt -= 3) > result)
            {
                memmove(pt + 1, pt, n);
                *pt = ',';
                n += 4;
            }
            else
                break;
    }

    return M_StringDuplicate(result);
}

char *uncommify(const char *input)
{
    char    *p;

    if (!*input)
        return "";

    if ((p = malloc(strlen(input) + 1)))
    {
        char    *p2 = p;

        while (*input != '\0')
            if (*input != ',' || *(input + 1) == '\0')
                *p2++ = *input++;
            else
                input++;

        *p2 = '\0';
    }

    return p;
}

bool wildcard(char *input, char *pattern)
{
    if (!*pattern || M_StringCompare(input, pattern))
        return true;

    for (int i = 0; pattern[i] != '\0'; i++)
    {
        if (pattern[i] == '?')
            continue;
        else if (pattern[i] == '*')
        {
            for (int j = i; input[j] != '\0'; j++)
                if (wildcard(input + j, pattern + i + 1))
                    return true;

            return false;
        }
        else if (pattern[i] != input[i])
            return false;
    }

    return false;
}

int gcd(int a, int b)
{
    return (!b ? a : gcd(b, a % b));
}

int numspaces(char *str)
{
    int         result = 0;
    const int   len = (int)strlen(str);

    for (int i = 0; i < len; i++)
        result += (str[i] == ' ');

    return result;
}

char *removespaces(const char *input)
{
    char    *p;

    if (!*input)
        return "";

    if ((p = malloc(strlen(input) + 1)))
    {
        char    *p2 = p;

        while (*input != '\0')
            if (!isspace((unsigned char)*input))
                *p2++ = *input++;
            else
                input++;

        *p2 = '\0';
    }

    return p;
}

char *removenonalpha(const char *input)
{
    char    *p;

    if (!*input)
        return "";

    if ((p = malloc(strlen(input) + 1)))
    {
        char    *p2 = p;

        while (*input != '\0')
            if (!isspace((unsigned char)*input) && *input != '-' && *input != '(' && *input != ')')
                *p2++ = *input++;
            else
                input++;

        *p2 = '\0';
    }

    return p;
}

char *trimwhitespace(char *input)
{
    char    *end;

    while (isspace((unsigned char)*input))
        input++;

    if (!*input)
        return input;

    end = input + strlen(input) - 1;

    while (end > input && isspace((unsigned char)*end))
        end--;

    *(end + 1) = '\0';

    return input;
}

char *makevalidfilename(const char *input)
{
    char        *newstr = M_StringDuplicate(input);
    const int   len = (int)strlen(newstr);

    for (int i = 0; i < len; i++)
        if (strchr("\\/:?\"<>|", newstr[i]))
            newstr[i] = ' ';

    return newstr;
}

char *leafname(char *path)
{
    char    cc;
    char    *ptr = path;

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
    char    *newstr = M_StringDuplicate(file);
    char    *lastdot = strrchr(newstr, '.');

    *lastdot = '\0';

    return newstr;
}

bool isvowel(const char ch)
{
    return !!strchr("aeiouAEIOU", ch);
}

bool ispunctuation(const char ch)
{
    return !!strchr(".!?", ch);
}

bool isbreak(const char ch)
{
    return !!strchr(" /\\-", ch);
}

char *striptrailingzero(float value, int precision)
{
    char    *result = malloc(100);

    if (result)
    {
        int len;

        M_snprintf(result, sizeof(result), "%.*f",
            (precision == 2 ? 2 : (value != floor(value))), value);
        len = (int)strlen(result);

        if (len >= 4 && result[len - 3] == '.' && result[len - 1] == '0')
            result[len - 1] = '\0';
    }

    return result;
}

void M_StripQuotes(char *str)
{
    int len = (int)strlen(str);

    if (len > 2
        && (((str[0] == '"' || str[0] == '\x93')
            && (str[len - 1] == '"' || str[len - 1] == '\x94'))
            || ((str[0] == '\'' || str[0] == '\x91')
                && (str[len - 1] == '\'' || str[len - 1] == '\x92'))))
    {
        len -= 2;
        memmove(str, str + 1, len);
        str[len] = '\0';
    }
}

void M_NormalizeSlashes(char *str)
{
    char    *p;

    // Convert all slashes/backslashes to DIR_SEPARATOR
    for (p = str; *p; p++)
        if ((*p == '/' || *p == '\\') && *p != DIR_SEPARATOR)
            *p = DIR_SEPARATOR;

    // Remove trailing slashes
    while (p > str && *(--p) == DIR_SEPARATOR)
        *p = 0;

    // Collapse multiple slashes
    for (p = str; (*str++ = *p); )
        if (*p++ == DIR_SEPARATOR)
            while (*p == DIR_SEPARATOR)
                p++;
}

char *preferredpronoun(pronoun_t type)
{
    if (type == personal)
        return (playergender == playergender_male ? "he" :
            (playergender == playergender_female ? "she" : "they"));
    else if (type == possessive)
        return (playergender == playergender_male ? "his" :
            (playergender == playergender_female ? "her" : "their"));
    else
        return (playergender == playergender_male ? "himself" :
            (playergender == playergender_female ? "herself" : "themselves"));
}

const char *words[][2] =
{
    { "agoniz",     "agonis"     }, { "airplane",   "aeroplane"  },
    { "analog",     "analogue"   }, { "armor",      "armour"     },
    { "artifact",   "artefact"   }, { "behavior",   "behaviour"  },
    { "caliber",    "calibre"    }, { "centered",   "centred"    },
    { "centering",  "centring"   }, { "center",     "centre"     },
    { "color",      "colour"     }, { "defense",    "defence"    },
    { "dialog",     "dialogue"   }, { "disk",       "disc"       },
    { "donut",      "doughnut"   }, { "draft",      "draught"    },
    { "endeavor",   "endeavour"  }, { "favor",      "favour"     },
    { "fiber",      "fibre"      }, { "flavor",     "flavour"    },
    { "gray",       "grey"       }, { "harbor",     "harbour"    },
    { "honor",      "honour"     }, { "humor",      "humour"     },
    { "initializ",  "initialis"  }, { "inquir",     "enquir"     },
    { "jewelry",    "jewellery"  }, { "judgment",   "judgement"  },
    { "labor",      "labour"     }, { "license",    "licence"    },
    { "liter",      "litre"      }, { "meter",      "metre"      },
    { "neighbor",   "neighbour"  }, { "offense",    "offence"    },
    { "organiz",    "organis"    }, { "practice",   "practise"   },
    { "program",    "programme"  }, { "realiz",     "realis"     },
    { "randomiz",   "randomis"   }, { "recogniz",   "recognis"   },
    { "refueling",  "refuelling" }, { "rumor",      "rumour"     },
    { "savior",     "saviour"    }, { "savor",      "savour"     },
    { "skeptic",    "sceptic"    }, { "specializ",  "specialis"  },
    { "stabiliz",   "stabilis"   }, { "standardiz", "standardis" },
    { "theater",    "theatre"    }, { "traveled",   "travelled"  },
    { "traveling",  "travelling" }, { "utiliz",     "utilis"     },
    { "vapor",      "vapour"     }, { "whiskey",    "whisky"     },
    { "yogurt",     "yoghurt"    }, { "",           ""           }
};

static void M_Translate(char *string, const char *word1, const char *word2)
{
    char    *temp1 = M_StringDuplicate(word1);
    char    *temp2 = M_StringDuplicate(word2);

    M_StringReplaceAll(string, temp1, temp2, true);

    temp1[0] = toupper(temp1[0]);
    temp2[0] = toupper(temp2[0]);

    M_StringReplaceAll(string, temp1, temp2, true);

    free(temp1);
    free(temp2);

    temp1 = uppercase(word1);
    temp2 = uppercase(word2);

    M_StringReplaceAll(string, temp1, temp2, true);

    free(temp1);
    free(temp2);
}

void M_AmericanToBritishEnglish(char *string)
{
    for (int i = 0; *words[i][0]; i++)
        M_Translate(string, words[i][0], words[i][1]);
}

void M_BritishToAmericanEnglish(char *string)
{
    for (int i = 0; *words[i][0]; i++)
        M_Translate(string, words[i][1], words[i][0]);
}

void M_TranslateAutocomplete(void)
{
    if (english == english_american)
        for (int i = 0; *autocompletelist[i].text; i++)
            M_BritishToAmericanEnglish(autocompletelist[i].text);
    else
        for (int i = 0; *autocompletelist[i].text; i++)
            M_AmericanToBritishEnglish(autocompletelist[i].text);
}

const char *dayofweek(int day, int month, int year)
{
    const int   adjustment = (14 - month) / 12;
    const char  *days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    month += 12 * adjustment - 2;
    year -= adjustment;

    return days[(day + (13 * month - 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7];
}
