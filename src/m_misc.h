/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>

#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
#include <stdarg.h>
#endif

#include "doomtype.h"

typedef enum
{
    personal,
    possessive,
    reflexive
} pronountype_t;

void M_MakeDirectory(const char *path);
bool M_FileExists(const char *filename);
bool M_FolderExists(const char *folder);
char *M_ExtractFolder(char *path);

#if !defined(_WIN32) && !defined(__APPLE__)
char *M_FileCaseExists(const char *path);
#endif

// Returns the file system location where application resource files are located.
// On Windows and Linux, this is the folder in which doomretro.exe is located;
// on macOS, this is the Contents/Resources folder within the application bundle.
char *M_GetResourceFolder(void);

// Returns the file system location where generated application
// data (configuration files, logs, savegames etc.) should be saved.
// On Windows and Linux, this is the folder in which doomretro.exe is located;
// on macOS, this is ~/Library/Application Support/DOOM Retro/.
char *M_GetAppDataFolder(void);

char *M_GetExecutableFolder(void);
bool M_StrToInt(const char *str, int *result);
const char *M_StrCaseStr(const char *haystack, const char *needle);
void M_StringCopy(char *dest, const char *src, const size_t dest_size);
char *M_StringReplace(char *haystack, const char *needle, const char *replacement);
void M_StringReplaceAll(char *haystack, const char *needle, const char *replacement, bool usecase);
char *M_TempFile(char *s);
char *M_StringJoin(const char *s, ...);
bool M_StringStartsWith(const char *s, const char *prefix);
bool M_StringStartsWithExact(const char *s, const char *prefix);
bool M_StringEndsWith(const char *s, const char *suffix);
void M_vsnprintf(char *buf, int buf_len, const char *s, va_list args);
void M_snprintf(char *buf, int buf_len, const char *s, ...);
char *M_SubString(const char *str, size_t begin, size_t len);
char *M_StringDuplicate(const char *orig);
bool M_StringCompare(const char *str1, const char *str2);
char *uppercase(const char *str);
char *lowercase(char *str);
char *titlecase(const char *str);
char *sentencecase(const char *str);
char *commify(int64_t value);
char *commifystat(uint64_t value);
char *uncommify(const char *input);
bool wildcard(char *input, char *pattern);
int gcd(int a, int b);
int numspaces(char *str);
char *removespaces(const char *input);
char *removenonalpha(const char *input);
char *trimwhitespace(char *input);
char *makevalidfilename(const char *input);
char *leafname(char *path);
char *removeext(const char *file);
bool isvowel(const char ch);
bool ispunctuation(const char ch);
bool isbreak(const char ch);
char *striptrailingzero(float value, int precision);
void M_StripQuotes(char *str);
void M_NormalizeSlashes(char *str);
char *pronoun(pronountype_t type);
void M_AmericanToInternationalEnglish(char *string);
void M_InternationalToAmericanEnglish(char *string);
