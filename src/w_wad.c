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

#include <ctype.h>

#include "c_console.h"
#include "doomstat.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

typedef struct
{
    // Should be "IWAD" or "PWAD".
    char        identification[4];
    int         numlumps;
    int         infotableofs;
} PACKEDATTR wadinfo_t;

typedef struct
{
    int         filepos;
    int         size;
    char        name[8];
} PACKEDATTR filelump_t;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t      **lumpinfo;
int             numlumps = 0;

// Hash table for fast lookups
static lumpindex_t      *lumphash;

void ExtractFileBase(char *path, char *dest)
{
    char        *src = path + strlen(path) - 1;
    char        *filename;
    int         length = 0;

    // back up until a \ or the start
    while (src != path && *(src - 1) != '\\' && *(src - 1) != '/')
        src--;

    // copy up to eight characters
    filename = src;
    memset(dest, 0, 8);

    while (*src != '\0' && *src != '.')
        dest[length++] = (char)toupper((int)*src++);
}

// Hash function used for lump names.
unsigned int W_LumpNameHash(const char *s)
{
    // This is the djb2 string hash function, modded to work on strings
    // that have a maximum length of 8.
    unsigned int        result = 5381;
    unsigned int        i;

    for (i = 0; i < 8 && s[i] != '\0'; ++i)
        result = ((result << 5) ^ result) ^ toupper(s[i]);

    return result;
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
// Other files are single lumps with the base filename
//  for the lump name.
wad_file_t *W_AddFile(char *filename, dboolean automatic)
{
    wadinfo_t   header;
    lumpindex_t i;
    int         startlump;
    filelump_t  *fileinfo;
    filelump_t  *filerover;
    lumpinfo_t  *filelumps;
    int         numfilelumps;

    // open the file and add to directory
    wad_file_t  *wad_file = W_OpenFile(filename);

    if (!wad_file)
        return NULL;

    M_StringCopy(wad_file->path, filename, sizeof(wad_file->path));

    wad_file->freedoom = IsFreedoom(filename);

    if (!M_StringCompare(filename + strlen(filename) - 3, "wad"))
    {
        // single lump file

        // fraggle: Swap the filepos and size here. The WAD directory
        // parsing code expects a little-endian directory, so will swap
        // them back. Effectively we're constructing a "fake WAD directory"
        // here, as it would appear on disk.
        fileinfo = Z_Malloc(sizeof(filelump_t), PU_STATIC, NULL);
        fileinfo->filepos = LONG(0);
        fileinfo->size = LONG(wad_file->length);

        // Name the lump after the base of the filename (without the
        // extension).
        ExtractFileBase(filename, fileinfo->name);
        numfilelumps = 1;
    }
    else
    {
        int     length;

        // WAD file
        W_Read(wad_file, 0, &header, sizeof(header));

        // Homebrew levels?
        if (strncmp(header.identification, "IWAD", 4)
            && strncmp(header.identification, "PWAD", 4))
            I_Error("Wad file %s doesn't have IWAD or PWAD id\n", filename);

        wad_file->type = (!strncmp(header.identification, "IWAD", 4) ? IWAD : PWAD);

        header.numlumps = LONG(header.numlumps);
        header.infotableofs = LONG(header.infotableofs);
        length = header.numlumps * sizeof(filelump_t);
        fileinfo = Z_Malloc(length, PU_STATIC, NULL);

        W_Read(wad_file, header.infotableofs, fileinfo, length);
        numfilelumps = header.numlumps;
    }

    // Increase size of numlumps array to accommodate the new file.
    filelumps = calloc(numfilelumps, sizeof(lumpinfo_t));
    if (!filelumps)
        I_Error("Failed to allocate array for lumps from new file.");

    startlump = numlumps;
    numlumps += numfilelumps;
    lumpinfo = Z_Realloc(lumpinfo, numlumps * sizeof(lumpinfo_t *));
    if (!lumpinfo)
        I_Error("Failed to increase lumpinfo[] array size.");

    filerover = fileinfo;

    for (i = startlump; i < numlumps; ++i)
    {
        lumpinfo_t      *lump_p = &filelumps[i - startlump];

        lump_p->wad_file = wad_file;
        lump_p->position = LONG(filerover->filepos);
        lump_p->size = LONG(filerover->size);
        lump_p->cache = NULL;
        strncpy(lump_p->name, filerover->name, 8);
        lumpinfo[i] = lump_p;

        ++filerover;
    }

    Z_Free(fileinfo);

    if (lumphash)
    {
        Z_Free(lumphash);
        lumphash = NULL;
    }

    C_Output("%s %s lump%s from %.4s file <b>%s</b>.", (automatic ? "Automatically added" :
        "Added"), commify(numlumps - startlump), (numlumps - startlump == 1 ? "" : "s"),
        header.identification, filename);

    return wad_file;
}

dboolean IsFreedoom(const char *iwadname)
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
        for (header.numlumps = LONG(header.numlumps);
            header.numlumps && fread(&lump, sizeof(lump), 1, fp); header.numlumps--)
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
        for (header.numlumps = LONG(header.numlumps);
            header.numlumps && fread(&lump, sizeof(lump), 1, fp); header.numlumps--)
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

    for (header.numlumps = LONG(header.numlumps);
        header.numlumps && fread(&lump, sizeof(lump), 1, fp); header.numlumps--)
        if (*n == 'E' && n[2] == 'M' && !n[4])
            result = doom;
        else if (*n == 'M' && n[1] == 'A' && n[2] == 'P' && !n[5])
            result = doom2;

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
    wad_file_t  *wad_file = W_OpenFile(filename);

    if (!wad_file)
        return 0;

    W_Read(wad_file, 0, &header, sizeof(header));

    W_CloseFile(wad_file);

    if (!strncmp(header.identification, "IWAD", 4))
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
lumpindex_t W_CheckNumForName(char *name)
{
    lumpindex_t i;

    // Do we have a hash table yet?
    if (lumphash)
    {
        int     hash;

        // We do! Excellent.
        hash = W_LumpNameHash(name) % numlumps;

        for (i = lumphash[hash]; i != -1; i = lumpinfo[i]->next)
            if (!strncasecmp(lumpinfo[i]->name, name, 8))
                return i;
    }
    else
    {
        // We don't have a hash table generate yet. Linear search :-(
        // scan backwards so patch lump files take precedence
        for (i = numlumps - 1; i >= 0; --i)
            if (!strncasecmp(lumpinfo[i]->name, name, 8))
                return i;
    }

    // TFB. Not found.
    return -1;
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

    for (i = numlumps - 1; i >= 0; --i)
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
            ++count;

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

lumpindex_t W_GetNumForNameX(char *name, unsigned int count)
{
    lumpindex_t         i;
    unsigned int        j = 0;

    for (i = 0; i < numlumps; i++)
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
            if (++j == count)
                break;

    if (i == numlumps && j != count)
        I_Error("W_GetNumForNameX: %s not found!", name);

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
    lumpinfo_t  *l;

    if (lump >= numlumps)
        I_Error("W_ReadLump: %i >= numlumps", lump);

    l = lumpinfo[lump];

    c = W_Read(l->wad_file, l->position, dest, l->size);

    if (c < l->size)
        I_Error("W_ReadLump: only read %i of %i on lump %i", c, l->size, lump);
}

//
// W_CacheLumpNum
//
// Load a lump into memory and return a pointer to a buffer containing
// the lump data.
//
// 'tag' is the type of zone memory buffer to allocate for the lump
// (usually PU_STATIC or PU_CACHE). If the lump is loaded as
// PU_STATIC, it should be released back using W_ReleaseLumpNum
// when no longer needed (do not use Z_ChangeTag).
//
void *W_CacheLumpNum(lumpindex_t lumpnum, int tag)
{
    byte        *result;
    lumpinfo_t  *lump;

    if (lumpnum >= numlumps)
        I_Error("W_CacheLumpNum: %i >= numlumps", lumpnum);

    lump = lumpinfo[lumpnum];

    // Get the pointer to return. If the lump is in a memory-mapped
    // file, we can just return a pointer to within the memory-mapped
    // region. If the lump is in an ordinary file, we may already
    // have it cached; otherwise, load it into memory.
    if (lump->wad_file->mapped)
    {
        // Memory mapped file, return from the mmapped region.
        result = lump->wad_file->mapped + lump->position;
    }
    else if (lump->cache)
    {
        // Already cached, so just switch the zone tag.
        result = (byte *)lump->cache;
        Z_ChangeTag(lump->cache, tag);
    }
    else
    {
        // Not yet loaded, so load it now
        lump->cache = Z_Malloc(W_LumpLength(lumpnum), tag, &lump->cache);
        W_ReadLump(lumpnum, lump->cache);
        result = (byte *)lump->cache;
    }

    return result;
}

//
// W_CacheLumpName
//
void *W_CacheLumpName(char *name, int tag)
{
    return W_CacheLumpNum(W_GetNumForName(name), tag);
}

//
// Release a lump back to the cache, so that it can be reused later
// without having to read from disk again, or alternatively, discarded
// if we run out of memory.
//
// Back in Vanilla DOOM, this was just done using Z_ChangeTag
// directly, but now that we have WAD mmap, things are a bit more
// complicated...
//
void W_ReleaseLumpNum(lumpindex_t lumpnum)
{
    lumpinfo_t  *lump;

    if (lumpnum >= numlumps)
        I_Error("W_ReleaseLumpNum: %i >= numlumps", lumpnum);

    lump = lumpinfo[lumpnum];

    if (!lump->wad_file->mapped)
        Z_ChangeTag(lump->cache, PU_CACHE);
}

void W_ReleaseLumpName(char *name)
{
    W_ReleaseLumpNum(W_GetNumForName(name));
}

// Generate a hash table for fast lookups
void W_GenerateHashTable(void)
{
    // Free the old hash table, if there is one
    if (lumphash)
        Z_Free(lumphash);

    // Generate hash table
    if (numlumps > 0)
    {
        lumpindex_t     i;

        lumphash = Z_Malloc(sizeof(lumpindex_t) * numlumps, PU_STATIC, NULL);

        for (i = 0; i < numlumps; ++i)
            lumphash[i] = -1;

        for (i = 0; i < numlumps; ++i)
        {
            unsigned int        hash;

            hash = W_LumpNameHash(lumpinfo[i]->name) % numlumps;

            // Hook into the hash table
            lumpinfo[i]->next = lumphash[hash];
            lumphash[hash] = i;
        }
    }

    // All done!
}
