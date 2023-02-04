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

#include "w_file.h"

//
// TYPES
//

//
// WADFILE I/O related stuff.
//

#define IWAD    1
#define PWAD    2

 typedef struct
{
    char        name[9];
    int         size;
    void        *cache;

    // killough 01/31/98: hash table fields, used for ultra-fast hash table lookup
    int         index;
    int         next;

    int         position;

    wadfile_t   *wadfile;
} lumpinfo_t;

extern lumpinfo_t   **lumpinfo;
extern int          numlumps;

bool IsUltimateDOOM(const char *iwadname);

char *GetCorrectCase(char *path);

#if defined(_WIN32)
char *W_GuessFilename(char *path, char *string);
#endif

bool W_AddFile(char *filename, bool autoloaded);
bool W_AutoLoadFiles(const char *folder);
int W_WadType(char *filename);

int W_CheckNumForName(const char *name);

int W_RangeCheckNumForName(int min, int max, const char *name);
int W_GetNumForName(const char *name);
int W_GetLastNumForName(const char *name);
int W_GetSecondNumForName(const char *name);
int W_GetWidestNumForName(const char *name);

int W_CheckMultipleLumps(const char *name);

int W_LumpLength(int lump);

void *W_CacheLumpNum(int lumpnum);

#define W_CacheLumpName(name)       W_CacheLumpNum(W_GetNumForName(name))
#define W_CacheSecondLumpName(name) W_CacheLumpNum(W_GetSecondNumForName(name))
#define W_CacheLastLumpName(name)   W_CacheLumpNum(W_GetLastNumForName(name))
#define W_CacheWidestLumpName(name) W_CacheLumpNum(W_GetWidestNumForName(name))

void W_Init(void);

unsigned int W_LumpNameHash(const char *s);

void W_ReleaseLumpNum(int lumpnum);

#define W_ReleaseLumpName(name)     W_ReleaseLumpNum(W_GetNumForName(name))

void W_CloseFiles(void);

gamemission_t IWADRequiredByPWAD(char *pwadname);
bool HasDehackedLump(const char *pwadname);
