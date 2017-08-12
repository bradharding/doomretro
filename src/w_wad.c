/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <ctype.h>

#include "c_console.h"
#include "doomstat.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_misc.h"
#include "version.h"
#include "w_merge.h"
#include "w_wad.h"
#include "z_zone.h"

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

typedef struct wadinfo_s
{
    // Should be "IWAD" or "PWAD".
    char            identification[4];
    int             numlumps;
    int             infotableofs;
} PACKEDATTR wadinfo_t;

typedef struct filelump_s
{
    int             filepos;
    int             size;
    char            name[8];
} PACKEDATTR filelump_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

static struct cachelump_s
{
    void            *cache;
    unsigned int    locks;
} *cachelump;

// Location of each lump on disk.
lumpinfo_t          **lumpinfo;
int                 numlumps;

extern char *packagewad;

static dboolean IsFreedoom(const char *iwadname)
{
    FILE        *fp = fopen(iwadname, "rb");
    filelump_t  lump;
    wadinfo_t   header;
    const char  *n = lump.name;
    int         result = false;

    if (!fp)
        I_Error("Can't open IWAD: %s\n", iwadname);

    // read IWAD header
    if (fread(&header, 1, sizeof(header), fp) == sizeof(header))
    {
        fseek(fp, LONG(header.infotableofs), SEEK_SET);

        // Determine game mode from levels present
        // Must be a full set for whichever mode is present
        for (header.numlumps = LONG(header.numlumps); header.numlumps && fread(&lump, sizeof(lump), 1, fp);
            header.numlumps--)
        {
            if (*n == 'F' && n[1] == 'R' && n[2] == 'E' && n[3] == 'E' &&
                n[4] == 'D' && n[5] == 'O' && n[6] == 'O' && n[7] == 'M')
            {
                result = true;
                break;
            }
        }
    }

    fclose(fp);

    return result;
}

char *GetCorrectCase(char *path)
{
#if defined(_WIN32)
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFile = FindFirstFile(path, &FindFileData);

    if (hFile == INVALID_HANDLE_VALUE)
        return path;
    else
    {
        FindClose(hFile);
        strreplace(path, FindFileData.cFileName, FindFileData.cFileName);
    }
#endif

    return path;
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
wadfile_t *W_AddFile(char *filename, dboolean automatic)
{
    static dboolean packagewadadded;
    wadinfo_t       header;
    lumpindex_t     i;
    int             length;
    int             startlump;
    filelump_t      *fileinfo;
    filelump_t      *filerover;
    lumpinfo_t      *filelumps;

    // open the file and add to directory
    wadfile_t   *wadfile = W_OpenFile(filename);

    if (!wadfile)
        return NULL;

    M_StringCopy(wadfile->path, GetCorrectCase(filename), sizeof(wadfile->path));

    if ((wadfile->freedoom = IsFreedoom(filename)))
        FREEDOOM = true;

    // WAD file
    W_Read(wadfile, 0, &header, sizeof(header));

    // Homebrew levels?
    if (strncmp(header.identification, "IWAD", 4) && strncmp(header.identification, "PWAD", 4))
        I_Error("Wad file %s doesn't have an IWAD or PWAD id.", filename);

    wadfile->type = (!strncmp(header.identification, "IWAD", 4)
        || M_StringCompare(leafname(filename), "DOOM2.WAD") ? IWAD : PWAD);

    header.numlumps = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);
    length = header.numlumps * sizeof(filelump_t);
    fileinfo = malloc(length);

    W_Read(wadfile, header.infotableofs, fileinfo, length);

    // Increase size of numlumps array to accommodate the new file.
    filelumps = calloc(header.numlumps, sizeof(lumpinfo_t));

    startlump = numlumps;
    numlumps += header.numlumps;
    lumpinfo = Z_Realloc(lumpinfo, numlumps * sizeof(lumpinfo_t *));

    filerover = fileinfo;

    for (i = startlump; i < numlumps; i++)
    {
        lumpinfo_t *lump_p = &filelumps[i - startlump];

        lump_p->wadfile = wadfile;
        lump_p->position = LONG(filerover->filepos);
        lump_p->size = LONG(filerover->size);
        lump_p->data = NULL;
        strncpy(lump_p->name, filerover->name, 8);
        lumpinfo[i] = lump_p;

        filerover++;
    }

    free(fileinfo);

    C_Output("%s %s lump%s from %s <b>%s</b>.", (automatic ? "Automatically added" : "Added"),
        commify(numlumps - startlump), (numlumps - startlump == 1 ? "" : "s"),
        (wadfile->type == IWAD ? "IWAD" : "PWAD"), wadfile->path);

    if (!packagewadadded)
    {
        packagewadadded = true;

        if (!W_MergeFile(packagewad, true))
            I_Error("%s is invalid.", packagewad);
    }

    return wadfile;
}

// Hash function used for lump names.
// Must be mod'ed with table size.
// Can be used for any 8-character names.
// by Lee Killough
unsigned W_LumpNameHash(const char *s)
{
    unsigned int        hash;

    (void)((hash = toupper(s[0]), s[1])
        && (hash = hash * 3 + toupper(s[1]), s[2])
        && (hash = hash * 2 + toupper(s[2]), s[3])
        && (hash = hash * 2 + toupper(s[3]), s[4])
        && (hash = hash * 2 + toupper(s[4]), s[5])
        && (hash = hash * 2 + toupper(s[5]), s[6])
        && (hash = hash * 2 + toupper(s[6]),
            hash = hash * 2 + toupper(s[7])));

    return hash;
}

dboolean HasDehackedLump(const char *pwadname)
{
    FILE        *fp = fopen(pwadname, "rb");
    filelump_t  lump;
    wadinfo_t   header;
    const char  *n = lump.name;
    int         result = false;

    if (!fp)
        return false;

    // read IWAD header
    if (fread(&header, 1, sizeof(header), fp) == sizeof(header))
    {
        fseek(fp, LONG(header.infotableofs), SEEK_SET);

        // Determine game mode from levels present
        // Must be a full set for whichever mode is present
        for (header.numlumps = LONG(header.numlumps); header.numlumps && fread(&lump, sizeof(lump), 1, fp);
            header.numlumps--)
        {
            if (*n == 'D' && n[1] == 'E' && n[2] == 'H' && n[3] == 'A' &&
                n[4] == 'C' && n[5] == 'K' && n[6] == 'E' && n[7] == 'D')
            {
                result = true;
                break;
            }
        }
    }

    fclose(fp);

    return result;
}

int IWADRequiredByPWAD(const char *pwadname)
{
    FILE        *fp = fopen(pwadname, "rb");
    filelump_t  lump;
    wadinfo_t   header;
    const char  *n = lump.name;
    int         result = indetermined;

    if (!fp)
        I_Error("Can't open PWAD: %s\n", pwadname);

    if (fread(&header, 1, sizeof(header), fp) != sizeof(header) ||
        header.identification[0] != 'P' || header.identification[1] != 'W' ||
        header.identification[2] != 'A' || header.identification[3] != 'D')
        I_Error("PWAD tag not present: %s\n", pwadname);

    fseek(fp, LONG(header.infotableofs), SEEK_SET);

    for (header.numlumps = LONG(header.numlumps); header.numlumps && fread(&lump, sizeof(lump), 1, fp);
        header.numlumps--)
    {
        if (*n == 'E' && n[2] == 'M' && !n[4])
            result = doom;
        else if (*n == 'M' && n[1] == 'A' && n[2] == 'P' && !n[5])
            result = doom2;
    }

    fclose(fp);

    return result;
}

//
// W_WadType
// Returns IWAD, PWAD or 0.
//
int W_WadType(char *filename)
{
    wadinfo_t   header;
    wadfile_t  *wadfile = W_OpenFile(filename);

    if (!wadfile)
        return 0;

    W_Read(wadfile, 0, &header, sizeof(header));

    W_CloseFile(wadfile);

    if (!strncmp(header.identification, "IWAD", 4) || M_StringCompare(leafname(filename), "DOOM2.WAD"))
        return IWAD;
    else if (!strncmp(header.identification, "PWAD", 4))
        return PWAD;
    else
        return 0;
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//
// Rewritten by Lee Killough to use hash table for performance. Significantly
// cuts down on time -- increases Doom performance over 300%. This is the
// single most important optimization of the original Doom sources, because
// lump name lookup is used so often, and the original Doom used a sequential
// search. For large wads with > 1000 lumps this meant an average of over
// 500 were probed during every search. Now the average is under 2 probes per
// search. There is no significant benefit to packing the names into longwords
// with this new hashing algorithm, because the work to do the packing is
// just as much work as simply doing the string comparisons with the new
// algorithm, which minimizes the expected number of comparisons to under 2.
//
lumpindex_t W_CheckNumForName(char *name)
{
    // Hash function maps the name to one of possibly numlump chains.
    // It has been tuned so that the average chain length never exceeds 2.
    int i = lumpinfo[W_LumpNameHash(name) % (unsigned int)numlumps]->index;

    while (i >= 0 && strncasecmp(lumpinfo[i]->name, name, 8))
        i = lumpinfo[i]->next;

    // Return the matching lump, or -1 if none found.
    return i;
}

//
// W_CheckMultipleLumps
// Check if there's more than one of the same lump.
//
int W_CheckMultipleLumps(char *name)
{
    int i;
    int count = 0;

    if (FREEDOOM || hacx)
        return 3;

    for (i = numlumps - 1; i >= 0; i--)
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
            count++;

    return count;
}

//
// W_RangeCheckNumForName
// Linear Search that checks for a lump number ONLY
// inside a range, not all lumps.
//
lumpindex_t W_RangeCheckNumForName(lumpindex_t min, lumpindex_t max, char *name)
{
    lumpindex_t i;

    for (i = min; i <= max; i++)
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
            return i;

    return -1;
}

void W_Init(void)
{
    int i;

    for (i = 0; i < numlumps; i++)
        lumpinfo[i]->index = -1;                       // mark slots empty

    // Insert nodes to the beginning of each chain, in first-to-last
    // lump order, so that the last lump of a given name appears first
    // in any chain, observing pwad ordering rules. killough
    for (i = 0; i < numlumps; i++)
    {
        // hash function:
        int j = W_LumpNameHash(lumpinfo[i]->name) % (unsigned int)numlumps;

        lumpinfo[i]->next = lumpinfo[j]->index;       // Prepend to list
        lumpinfo[j]->index = i;
    }

    // set up caching
    cachelump = calloc(sizeof(*cachelump), numlumps);

    if (!cachelump)
        I_Error ("W_Init: Couldn't allocate lumpcache");
}

//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
lumpindex_t W_GetNumForName(char *name)
{
    lumpindex_t i = W_CheckNumForName(name);

    if (i < 0)
        I_Error("W_GetNumForName: %s not found!", name);

    return i;
}

// Go forwards rather than backwards so we get lump from IWAD and not PWAD
lumpindex_t W_GetNumForName2(char *name)
{
    lumpindex_t i;

    for (i = 0; i < numlumps; i++)
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
            break;

    if (i == numlumps)
        I_Error("W_GetNumForName: %s not found!", name);

    return i;
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength(lumpindex_t lump)
{
    if (lump >= numlumps)
        I_Error("W_LumpLength: %i >= numlumps", lump);

    return lumpinfo[lump]->size;
}

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(lumpindex_t lump, void *dest)
{
    int         c;
    lumpinfo_t  *l = lumpinfo[lump];

    if (!l->size || !dest)
        return;

    c = W_Read(l->wadfile, l->position, dest, l->size);

    if (c < l->size)
        I_Error("W_ReadLump: only read %i of %i on lump %i", c, l->size, lump);
}

void *W_CacheLumpNum(lumpindex_t lump)
{
    const int locks = 1;

    if (!cachelump[lump].cache)         // read the lump in
        W_ReadLump(lump, Z_Malloc(W_LumpLength(lump), PU_CACHE, &cachelump[lump].cache));

    // cph - if wasn't locked but now is, tell z_zone to hold it
    if (!cachelump[lump].locks && locks)
        Z_ChangeTag(cachelump[lump].cache, PU_STATIC);

    cachelump[lump].locks += locks;

    return cachelump[lump].cache;
}

void W_UnlockLumpNum(lumpindex_t lump)
{
    const int unlocks = 1;

    cachelump[lump].locks -= unlocks;

    if (unlocks && !cachelump[lump].locks)
        Z_ChangeTag(cachelump[lump].cache, PU_CACHE);
}
