/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#if !defined(__W_WAD__)
#define __W_WAD__

#include <stdio.h>

#include "doomdef.h"
#include "doomtype.h"
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
typedef int lumpindex_t;

struct lumpinfo_s
{
    char        name[8];
    wad_file_t  *wad_file;
    int         position;
    int         size;
    void        *cache;

    // Used for hash table lookups
    lumpindex_t next;
};

extern lumpinfo_t       **lumpinfo;
extern int              numlumps;

wad_file_t *W_AddFile(char *filename, dboolean automatic);
int W_WadType(char *filename);

lumpindex_t W_CheckNumForName(char *name);
lumpindex_t W_RangeCheckNumForName(lumpindex_t min, lumpindex_t max, char *name);
lumpindex_t W_GetNumForName(char *name);
lumpindex_t W_GetNumForName2(char *name);
lumpindex_t W_GetNumForNameX(char *name, unsigned int count);

int W_CheckMultipleLumps(char *name);

int W_LumpLength(lumpindex_t lump);
void W_ReadLump(lumpindex_t lump, void *dest);

void *W_CacheLumpNum(lumpindex_t lump, int tag);
void *W_CacheLumpName(char *name, int tag);

void W_GenerateHashTable(void);

extern unsigned int W_LumpNameHash(const char *s);

void W_ReleaseLumpNum(lumpindex_t lump);
void W_ReleaseLumpName(char *name);

int IWADRequiredByPWAD(const char *pwadname);
dboolean IsFreedoom(const char *iwadname);
dboolean HasDehackedLump(const char *pwadname);

#endif
