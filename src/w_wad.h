/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
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

#if !defined(__W_WAD_H__)
#define __W_WAD_H__

#include "doomdef.h"
#include "w_file.h"

//
// TYPES
//

//
// WADFILE I/O related stuff.
//

#define IWAD 1
#define PWAD 2

typedef struct lumpinfo_s lumpinfo_t;

struct lumpinfo_s
{
    char        name[9];
    int         size;
    void        *cache;

    // killough 1/31/98: hash table fields, used for ultra-fast hash table lookup
    int         index;
    int         next;

    int         position;

    wadfile_t   *wadfile;
};

extern lumpinfo_t   **lumpinfo;
extern int          numlumps;

dboolean IsBFGEdition(const char *iwadname);

char *GetCorrectCase(char *path);
dboolean W_AddFile(char *filename, dboolean automatic);
int W_WadType(char *filename);

int W_CheckNumForName(const char *name);

int W_RangeCheckNumForName(int min, int max, const char *name);
int W_GetNumForName(const char *name);
int W_GetLastNumForName(const char *name);
int W_GetSecondNumForName(const char* name);

int W_CheckMultipleLumps(const char *name);

int W_LumpLength(int lump);
void W_ReadLump(int lump, void *dest);

void *W_CacheLumpNum(int lumpnum);

#define W_CacheLumpName(name)   W_CacheLumpNum(W_GetNumForName(name))
#define W_CacheLastLumpName(name)  W_CacheLumpNum(W_GetLastNumForName(name))

void W_Init(void);

unsigned int W_LumpNameHash(const char *s);

void W_ReleaseLumpNum(int lumpnum);

#define W_ReleaseLumpName(name) W_ReleaseLumpNum(W_GetNumForName(name))

GameMission_t IWADRequiredByPWAD(char *pwadname);
dboolean HasDehackedLump(const char *pwadname);

#endif
