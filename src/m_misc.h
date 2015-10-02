/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

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
  id Software LLC.

========================================================================
*/

#if !defined(__M_MISC__)
#define __M_MISC__

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"

dboolean M_WriteFile(char *name, void *source, int length);
int M_ReadFile(char *name, byte **buffer);
void M_MakeDirectory(char *dir);
char *M_TempFile(char *s);
dboolean M_FileExists(char *file);
long M_FileLength(FILE *handle);
char *M_ExtractFolder(char *path);
char *M_GetExecutableFolder(void);
dboolean M_StrToInt(const char *str, int *result);
char *M_StrCaseStr(char *haystack, char *needle);
dboolean M_StringCopy(char *dest, char *src, size_t dest_size);
char *M_StringReplace(char *haystack, char *needle, char *replacement);
char *M_StringJoin(char *s, ...);
dboolean M_StringStartsWith(char *s, char *prefix);
dboolean M_StringEndsWith(char *s, char *suffix);
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args);
int M_snprintf(char *buf, size_t buf_len, const char *s, ...);
char *M_SubString(const char *str, size_t begin, size_t len);
char *uppercase(const char *str);
char *titlecase(const char *str);
char *commify(int value);
dboolean wildcard(char *input, char *pattern);
int gcd(int a, int b);
char *removespaces(const char *input);
char *removenewlines(const char *input);
char *makevalidfilename(const char *input);
const char *leafname(const char *path);
dboolean isvowel(const char ch);
char *convertsize(const int size);

#endif
