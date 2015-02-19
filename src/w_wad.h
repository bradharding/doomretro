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

struct lumpinfo_s
{
    char        name[8];
    wad_file_t  *wad_file;
    int         position;
    int         size;
    void        *cache;

    // Used for hash table lookups
    lumpinfo_t  *next;
};

extern lumpinfo_t *lumpinfo;
extern unsigned int numlumps;

wad_file_t *W_AddFile(char *filename);
int W_WadType(char *filename);

int W_CheckNumForName(char *name);
int W_RangeCheckNumForName(int min, int max, char *name);
int W_GetNumForName(char *name);
int W_GetNumForName2(char *name);
int W_GetNumForNameX(char *name, unsigned int count);

int W_CheckMultipleLumps(char *name);

int W_LumpLength(unsigned int lump);
void W_ReadLump(unsigned int lump, void *dest);

void *W_CacheLumpNum(int lump, int tag);
void *W_CacheLumpName(char *name, int tag);

void W_GenerateHashTable(void);

extern unsigned int W_LumpNameHash(const char *s);

void W_ReleaseLumpNum(int lump);
void W_ReleaseLumpName(char *name);

int IWADRequiredByPWAD(const char *pwadname);
boolean IsFreedoom(const char *iwadname);
boolean HasDehackedLump(const char *pwadname);

#endif
