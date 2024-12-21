/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#pragma once

#include "doomdef.h"
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
extern char         *wadsloaded;

bool IsUltimateDOOM(const char *iwadname);

char *GetCorrectCase(char *path);

#if defined(_WIN32)
char *W_GuessFilename(char *path, const char *string);
#endif

bool W_AddFile(char *filename, bool autoloaded);
bool W_AutoloadFile(const char *filename, const char *folder, const bool nonerveorsigil);
bool W_AutoloadFiles(const char *folder, const bool nonerveorsigil);
int W_WadType(char *filename);

int W_CheckNumForName(const char *name);

int W_RangeCheckNumForName(int min, int max, const char *name);
int W_GetNumForName(const char *name);
int W_GetLastNumForName(const char *name);
int W_GetXNumForName(const char *name, const int x);
int W_GetNumForNameFromResourceWAD(const char *name);

int W_GetNumLumps(const char *name);

int W_LumpLength(int lump);

void *W_CacheLumpNum(int lumpnum);

#define W_CacheLumpName(name)                   W_CacheLumpNum(W_GetNumForName(name))
#define W_CacheLastLumpName(name)               W_CacheLumpNum(W_GetLastNumForName(name))
#define W_CacheXLumpName(name, x)               W_CacheLumpNum(W_GetXNumForName(name, x))
#define W_CacheLumpNameFromResourceWAD(name)    W_CacheLumpNum(W_GetNumForNameFromResourceWAD(name))

void W_Init(void);
void W_CheckForPNGLumps(void);

unsigned int W_LumpNameHash(const char *s);

void W_ReleaseLumpNum(int lumpnum);

#define W_ReleaseLumpName(name)     W_ReleaseLumpNum(W_GetNumForName(name))

void W_CloseFiles(void);

gamemission_t IWADRequiredByPWAD(char *pwadname);
bool HasDehackedLump(const char *pwadname);
