/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"


#include "w_wad.h"

#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"

#include "doomstat.h"
#include "r_sky.h"


#include "r_data.h"

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//



//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef struct
{
    short       originx;
    short       originy;
    short       patch;
    short       stepdir;
    short       colormap;
} PACKEDATTR mappatch_t;


//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct
{
    char        name[8];
    int         masked;
    short       width;
    short       height;
    int         obsolete;
    short       patchcount;
    mappatch_t  patches[1];
} PACKEDATTR maptexture_t;


// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct
{
    // Block origin (allways UL),
    // which has allready accounted
    // for the internal origin of the patch.
    short       originx;
    short       originy;
    int         patch;
} texpatch_t;


// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.

typedef struct texture_s texture_t;

struct texture_s
{
    // Keep name for switch changing, etc.
    char        name[8];
    short       width;
    short       height;

    // Index in textures list

    int         index;

    // Next in hash table chain

    texture_t   *next;

    // All the patches[patchcount]
    //  are drawn back to front into the cached texture.
    short       patchcount;
    texpatch_t  patches[1];
};



int             firstflat;
int             lastflat;
int             numflats;

int             firstpatch;
int             lastpatch;
int             numpatches;

int             firstspritelump;
int             lastspritelump;
int             numspritelumps;

int             numtextures;
texture_t       **textures;
texture_t       **textures_hashtable;


int             *texturewidthmask;
// needed for texture pegging
fixed_t         *textureheight;
byte            **texturefullbright;
int             *texturecompositesize;
short           **texturecolumnlump;
unsigned        **texturecolumnofs;
byte            **texturecomposite;

// for global animation
int             *flattranslation;
int             *texturetranslation;

// needed for pre rendering
fixed_t         *spritewidth;
fixed_t         *spriteheight;
fixed_t         *spriteoffset;
fixed_t         *spritetopoffset;

lighttable_t    *colormaps;

boolean         *lookuptextures;
int             lookupprogress;

byte notgray[256] =
{
    0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

byte notgrayorbrown[256] =
{
    0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

byte redonly[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

byte greenonly1[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

byte greenonly2[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

struct
{
    char         texture[9];
    byte         *colormask;
} fullbright[] = {
    { "COMP2",    notgrayorbrown }, { "COMPSTA1", notgray        }, { "COMPSTA2", notgray        },
    { "COMPUTE1", notgrayorbrown }, { "COMPUTE2", notgrayorbrown }, { "COMPUTE3", notgrayorbrown },
    { "EXITSIGN", notgray        }, { "EXITSTON", notgray        }, { "PLANET1 ", notgray        },
    { "SILVER2",  notgray        }, { "SILVER3",  notgrayorbrown }, { "SLADSKUL", redonly        },
    { "SW1BRCOM", redonly        }, { "SW1BRIK",  redonly        }, { "SW1BRN1",  redonly        },
    { "SW1COMM",  redonly        }, { "SW1DIRT",  redonly        }, { "SW1MET2",  redonly        },
    { "SW1STARG", redonly        }, { "SW1STON1", redonly        }, { "SW1STON2", redonly        },
    { "SW1STONE", redonly        }, { "SW1STRTN", redonly        }, { "SW2BLUE ", redonly        },
    { "SW2BRCOM", greenonly2     }, { "SW2BRIK",  greenonly1     }, { "SW2BRN1",  greenonly1     },
    { "SW2BRN2",  notgray        }, { "SW2BRNGN", notgray        }, { "SW2COMM",  greenonly1     },
    { "SW2COMP",  redonly        }, { "SW2DIRT",  greenonly1     }, { "SW2EXIT",  notgray        },
    { "SW2GRAY",  notgray        }, { "SW2GRAY1", notgray        }, { "SW2GSTON", redonly        },
    { "SW2MARB ", greenonly1     }, { "SW2MET2",  greenonly1     }, { "SW2METAL", greenonly1     },
    { "SW2MOD1",  notgrayorbrown }, { "SW2PANEL", redonly        }, { "SW2ROCK",  redonly        },
    { "SW2SLAD",  redonly        }, { "SW2STARG", greenonly1     }, { "SW2STON1", greenonly1     },
    { "SW2STON2", greenonly1     }, { "SW2STON6", redonly        }, { "SW2STONE", greenonly1     },
    { "SW2STRTN", greenonly1     }, { "SW2TEK",   greenonly1     }, { "SW2VINE",  greenonly1     },
    { "SW2WDMET", redonly        }, { "SW2WOOD",  redonly        }, { "SW2ZIM",   redonly        },
    { "WOOD4",    redonly        }, { "WOODGARG", redonly        }, { "WOODSKUL", redonly        },
    { "ZELDOOR",  redonly        }, { "",        0               }
};


//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//



//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
void R_DrawColumnInCache(const column_t *patch, byte *cache, int originy, int cacheheight,
                         byte *marks)
{
    while (patch->topdelta != 0xFF)
    {
        int     count = patch->length;
        int     position = originy + patch->topdelta;
        byte    *source = (byte *)patch + 3;

        if (position < 0)
        {
            count += position;
            source -= position;
            position = 0;
        }

        if (position + count > cacheheight)
            count = cacheheight - position;

        if (count > 0)
        {
            memcpy(cache + position, source, count);
            memset (marks + position, 0xFF, count);
        }

        patch = (column_t *)((byte *)patch + patch->length + 4);
    }
}



//
// R_GenerateComposite
// Using the texture definition,
//  the composite texture is created from the patches,
//  and each column is cached.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
//
static void R_GenerateComposite(int texnum)
{
    byte *block = (byte *)Z_Malloc(texturecompositesize[texnum], PU_STATIC,
                                   (void **)&texturecomposite[texnum]);
    texture_t *texture = textures[texnum];
    // Composite the columns together.
    texpatch_t *patch = texture->patches;
    short *collump = texturecolumnlump[texnum];
    unsigned *colofs = texturecolumnofs[texnum]; // killough 4/9/98: make 32-bit
    int i = texture->patchcount;
    // killough 4/9/98: marks to identify transparent regions in merged textures
    byte *marks = (byte *)calloc(texture->width, texture->height), *source;

    for (; --i >= 0; patch++)
    {
        patch_t *realpatch = (patch_t *)W_CacheLumpNum(patch->patch, PU_CACHE);
        int x, x1 = patch->originx, x2 = x1 + SHORT(realpatch->width);
        const int *cofs = realpatch->columnofs - x1;

        if (x1 < 0)
            x1 = 0;
        if (x2 > texture->width)
            x2 = texture->width;
        for (x = x1; x < x2 ; x++)
            if (collump[x] == -1)               // Column has multiple patches?
                // killough 1/25/98, 4/9/98: Fix medusa bug.
                R_DrawColumnInCache((column_t *)((byte *)realpatch + LONG(cofs[x])),
                                    block + colofs[x], patch->originy,
                                    texture->height, marks + x * texture->height);
    }

    // killough 4/9/98: Next, convert multipatched columns into true columns,
    // to fix Medusa bug while still allowing for transparent regions.

    source = (byte *)malloc(texture->height);           // temporary column
    for (i = 0; i < texture->width; i++)
        if (collump[i] == -1)                   // process only multipatched columns
        {
            column_t *col = (column_t *)(block + colofs[i] - 3);        // cached column
            const byte *mark = marks + i * texture->height;
            int j = 0;

            // save column in temporary so we can shuffle it around
            memcpy(source, (byte *)col + 3, texture->height);

            for (;;)  // reconstruct the column by scanning transparency marks
            {
                unsigned len;                           // killough 12/98

                while (j < texture->height && !mark[j]) // skip transparent cells
                    j++;

                if (j >= texture->height)               // if at end of column
                {
                    col->topdelta = -1;                 // end-of-column marker
                    break;
                }

                col->topdelta = j;                      // starting offset of post

                // killough 12/98:
                // Use 32-bit len counter, to support tall 1s multipatched textures

                for (len = 0; j < texture->height && mark[j]; j++)
                    len++;                              // count opaque cells

                col->length = len; // killough 12/98: intentionally truncate length

                // copy opaque cells from the temporary back into the column
                memcpy((byte *)col + 3, source + col->topdelta, len);
                col = (column_t *)((byte *)col + len + 4); // next post
            }
        }
    free(source);         // free temporary column
    free(marks);          // free transparency marks

    // Now that the texture has been built in column cache,
    // it is purgable from zone memory.

    Z_ChangeTag(block, PU_CACHE);
}



//
// R_GenerateLookup
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
//
static void R_GenerateLookup(int texnum)
{
    const texture_t *texture = textures[texnum];

    // Composited texture not created yet.

    short *collump = texturecolumnlump[texnum];
    unsigned *colofs = texturecolumnofs[texnum];        // killough 4/9/98: make 32-bit

    // killough 4/9/98: keep count of posts in addition to patches.
    // Part of fix for medusa bug for multipatched 2s normals.

    struct {
        unsigned patches, posts;
    } *count = calloc(sizeof *count, texture->width);

    // killough 12/98: First count the number of patches per column.

    const texpatch_t *patch = texture->patches;
    int i = texture->patchcount;

    while (--i >= 0)
    {
        int pat = patch->patch;
        const patch_t *realpatch = (patch_t *)W_CacheLumpNum(pat, PU_CACHE);
        int x, x1 = (patch++)->originx, x2 = x1 + SHORT(realpatch->width);
        const int *cofs = realpatch->columnofs - x1;

        if (x2 > texture->width)
            x2 = texture->width;
        if (x1 < 0)
            x1 = 0;
        for (x = x1; x < x2; x++)
        {
            count[x].patches++;
            collump[x] = pat;
            colofs[x] = LONG(cofs[x]) + 3;
        }
    }

    // killough 4/9/98: keep a count of the number of posts in column,
    // to fix Medusa bug while allowing for transparent multipatches.
    //
    // killough 12/98:
    // Post counts are only necessary if column is multipatched,
    // so skip counting posts if column comes from a single patch.
    // This allows arbitrarily tall textures for 1s walls.
    //
    // If texture is >= 256 tall, assume it's 1s, and hence it has
    // only one post per column. This avoids crashes while allowing
    // for arbitrarily tall multipatched 1s textures.

    if (texture->patchcount > 1 && texture->height < 256)
    {
        // killough 12/98: Warn about a common column construction bug
        unsigned limit = texture->height * 3 + 3;       // absolute column size limit

        for (i = texture->patchcount, patch = texture->patches; --i >= 0;)
        {
            int pat = patch->patch;
            const patch_t *realpatch = (patch_t *)W_CacheLumpNum(pat, PU_CACHE);
            int x, x1 = patch++->originx, x2 = x1 + SHORT(realpatch->width);
            const int *cofs = realpatch->columnofs - x1;

            if (x2 > texture->width)
                x2 = texture->width;
            if (x1 < 0)
                x1 = 0;

            for (x = x1; x < x2; x++)
                if (count[x].patches > 1)               // Only multipatched columns
                {
                    const column_t *col = (column_t *)((byte *)realpatch + LONG(cofs[x]));
                    const byte *base = (const byte *)col;

                    // count posts
                    for (; col->topdelta != 0xff; count[x].posts++)
                        if ((unsigned)((byte *)col - base) <= limit)
                            col = (column_t *)((byte *)col + col->length + 4);
                }
        }
    }

    // -ES- 1998/08/18 We don't have to init this texnum again
    lookuptextures[texnum] = true;

    // Now count the number of columns
    //  that are covered by more than one patch.
    // Fill in the lump / offset, so columns
    //  with only a single patch are all done.

    texturecomposite[texnum] = 0;

    {
        int x = texture->width;
        int height = texture->height;
        int csize = 0;                                  // killough 10/98

        while (--x >= 0)
        {
            if (count[x].patches > 1)                   // killough 4/9/98
            {
                // killough 1/25/98, 4/9/98:
                //
                // Fix Medusa bug, by adding room for column header
                // and trailer bytes for each post in merged column.
                // For now, just allocate conservatively 4 bytes
                // per post per patch per column, since we don't
                // yet know how many posts the merged column will
                // require, and it's bounded above by this limit.

                collump[x] = -1;                        // mark lump as multipatched
                colofs[x] = csize + 3;                  // three header bytes in a column
                // killough 12/98: add room for one extra post
                csize += 4 * count[x].posts + 5;        // 1 stop byte plus 4 bytes per post
            }
            csize += height;                            // height bytes of texture data
        }

        texturecompositesize[texnum] = csize;
    }
    free(count);                                        // killough 4/9/98
}



//
// R_GetColumn
//
byte *R_GetColumn(int tex, int col)
{
    int         lump;
    int         ofs;

    if (lookuptextures[tex] == false)
        R_GenerateLookup(tex);

    col &= texturewidthmask[tex];
    lump = texturecolumnlump[tex][col];
    ofs = texturecolumnofs[tex][col];

    if (lump > 0)
        return (byte *)W_CacheLumpNum(lump, PU_CACHE) + ofs;

    if (!texturecomposite[tex])
        R_GenerateComposite(tex);

    return texturecomposite[tex] + ofs;
}


static void GenerateTextureHashTable(void)
{
    texture_t   **rover;
    int         i;
    int         key;

    textures_hashtable
        = (texture_t **)Z_Malloc(sizeof(texture_t *) * numtextures, PU_STATIC, 0);

    memset(textures_hashtable, 0, sizeof(texture_t *) * numtextures);

    // Add all textures to hash table

    for (i = 0; i < numtextures; ++i)
    {
        // Store index

        textures[i]->index = i;

        // Vanilla Doom does a linear search of the texures array
        // and stops at the first entry it finds.  If there are two
        // entries with the same name, the first one in the array
        // wins. The new entry must therefore be added at the end
        // of the hash chain, so that earlier entries win.

        key = W_LumpNameHash(textures[i]->name) % numtextures;

        rover = &textures_hashtable[key];

        while (*rover != NULL)
        {
            rover = &(*rover)->next;
        }

        // Hook into hash table

        textures[i]->next = NULL;
        *rover = textures[i];
    }
}


//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
void R_InitTextures(void)
{
    maptexture_t        *mtexture;
    texture_t           *texture;
    mappatch_t          *mpatch;
    texpatch_t          *patch;

    int                 i;
    int                 j;

    int                 *maptex;
    int                 *maptex2;
    int                 *maptex1;

    char                name[9];
    char                *names;
    char                *name_p;

    int                 *patchlookup;

    int                 totalwidth;
    int                 nummappatches;
    int                 offset;
    int                 maxoff;
    int                 maxoff2;
    int                 numtextures1;
    int                 numtextures2;

    int                 *directory;


    // Load the patch names from pnames.lmp.
    name[8] = 0;
    names = (char *)W_CacheLumpName("PNAMES", PU_STATIC);
    nummappatches = LONG(*((int *)names));
    name_p = names + 4;
    patchlookup = (int *)Z_Malloc(nummappatches * sizeof(*patchlookup), PU_STATIC, NULL);

    for (i = 0; i < nummappatches; i++)
    {
        strncpy(name,name_p + i * 8, 8);
        patchlookup[i] = W_CheckNumForName(name);
    }
    W_ReleaseLumpName("PNAMES");

    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex = maptex1 = (int *)W_CacheLumpName("TEXTURE1", PU_STATIC);
    numtextures1 = LONG(*maptex);
    maxoff = W_LumpLength(W_GetNumForName("TEXTURE1"));
    directory = maptex + 1;

    if (W_CheckNumForName("TEXTURE2") != -1)
    {
        maptex2 = (int *)W_CacheLumpName("TEXTURE2", PU_STATIC);
        numtextures2 = LONG(*maptex2);
        maxoff2 = W_LumpLength(W_GetNumForName("TEXTURE2"));
    }
    else
    {
        maptex2 = NULL;
        numtextures2 = 0;
        maxoff2 = 0;
    }
    numtextures = numtextures1 + numtextures2;

    textures = (texture_t **)Z_Malloc(numtextures * sizeof(*textures), PU_STATIC, 0);
    texturecolumnlump = (short **)Z_Malloc(numtextures * sizeof(*texturecolumnlump), PU_STATIC, 0);
    texturecolumnofs = (unsigned **)Z_Malloc(numtextures * sizeof(*texturecolumnofs), PU_STATIC, 0);
    texturecomposite = (byte **)Z_Malloc(numtextures * sizeof(*texturecomposite), PU_STATIC, 0);
    texturecompositesize = (int *)Z_Malloc(numtextures * sizeof(*texturecompositesize), PU_STATIC, 0);
    texturewidthmask = (int *)Z_Malloc(numtextures * sizeof(*texturewidthmask), PU_STATIC, 0);
    textureheight = (fixed_t *)Z_Malloc(numtextures * sizeof(*textureheight), PU_STATIC, 0);
    texturefullbright = (byte **)Z_Malloc(numtextures * sizeof(*texturefullbright), PU_STATIC, 0);

    totalwidth = 0;

    for (i = 0; i < numtextures; i++, directory++)
    {
        if (i == numtextures1)
        {
            // Start looking in second texture file.
            maptex = maptex2;
            maxoff = maxoff2;
            directory = maptex + 1;
        }

        offset = LONG(*directory);

        if (offset > maxoff)
            I_Error ("R_InitTextures: bad texture directory");

        mtexture = (maptexture_t *)((byte *)maptex + offset);

        texture = textures[i] =
            (texture_t *)Z_Malloc(sizeof(texture_t) +
                                  sizeof(texpatch_t) * (SHORT(mtexture->patchcount) - 1),
                                  PU_STATIC, 0);

        texture->width = SHORT(mtexture->width);
        texture->height = SHORT(mtexture->height);
        texture->patchcount = SHORT(mtexture->patchcount);

        memcpy(texture->name, mtexture->name, sizeof(texture->name));
        mpatch = &mtexture->patches[0];
        patch = &texture->patches[0];

        for (j = 0; j < texture->patchcount; j++, mpatch++, patch++)
        {
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);
            patch->patch = patchlookup[SHORT(mpatch->patch)];
            if (patch->patch == -1)
            {
                I_Error ("R_InitTextures: Missing patch in texture %s",
                         texture->name);
            }
        }
        texturecolumnlump[i] = (short *)Z_Malloc(texture->width * sizeof(**texturecolumnlump),
                                                 PU_STATIC, 0);
        texturecolumnofs[i] = (unsigned *)Z_Malloc(texture->width * sizeof(**texturecolumnofs),
                                                   PU_STATIC, 0);

        j = 1;
        while (j * 2 <= texture->width)
            j <<= 1;

        texturewidthmask[i] = j - 1;
        textureheight[i] = texture->height << FRACBITS;

        totalwidth += texture->width;
    }

    Z_Free(patchlookup);

    W_ReleaseLumpName("TEXTURE1");
    if (maptex2)
        W_ReleaseLumpName("TEXTURE2");

    lookuptextures = (bool *)Z_Malloc(numtextures * sizeof(boolean), PU_STATIC, 0);

    for (i = 0; i < numtextures; i++)
        lookuptextures[i] = false;

    lookupprogress = numtextures;

    // Precalculate whatever possible.

    for (i = 0; i < numtextures; i++)
        R_GenerateLookup(i);

    // Create translation table for global animation.
    texturetranslation = (int *)Z_Malloc((numtextures + 1) * sizeof(*texturetranslation),
                                         PU_STATIC, 0);

    for (i = 0; i < numtextures; i++)
        texturetranslation[i] = i;

    GenerateTextureHashTable();

    // [BH] Initialize partially fullbright textures.
    {
        int j = 0;

        memset(texturefullbright, 0, sizeof(byte *) * numtextures);
        while (fullbright[j].colormask)
        {
            int num = R_CheckTextureNumForName(fullbright[j].texture);

            if (num != -1)
              texturefullbright[num] = fullbright[j].colormask;
            j++;
        }
    }
}



//
// R_InitFlats
//
void R_InitFlats(void)
{
    int         i;

    firstflat = W_GetNumForName("F_START") + 1;
    lastflat = W_GetNumForName("F_END") - 1;
    numflats = lastflat - firstflat + 1;

    // Create translation table for global animation.
    flattranslation = (int *)Z_Malloc((numflats + 1) * sizeof(*flattranslation), PU_STATIC, 0);

    for (i = 0; i < numflats; i++)
        flattranslation[i] = i;
}


//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
void R_InitSpriteLumps(void)
{
    int         i;
    patch_t     *patch;

    firstspritelump = W_GetNumForName("S_START") + 1;
    lastspritelump = W_GetNumForName("S_END") - 1;

    numspritelumps = lastspritelump - firstspritelump + 1;
    spritewidth = (fixed_t *)Z_Malloc(numspritelumps * sizeof(*spritewidth), PU_STATIC, 0);
    spriteheight = (fixed_t *)Z_Malloc(numspritelumps * sizeof(*spriteheight), PU_STATIC, 0);
    spriteoffset = (fixed_t *)Z_Malloc(numspritelumps * sizeof(*spriteoffset), PU_STATIC, 0);
    spritetopoffset = (fixed_t *)Z_Malloc(numspritelumps * sizeof(*spritetopoffset), PU_STATIC, 0);

    for (i = 0; i < numspritelumps; i++)
    {
        patch = (patch_t *)W_CacheLumpNum(firstspritelump + i, PU_CACHE);
        spritewidth[i] = SHORT(patch->width) << FRACBITS;
        spriteheight[i] = SHORT(patch->height) << FRACBITS;
        spriteoffset[i] = SHORT(patch->leftoffset) << FRACBITS;
        spritetopoffset[i] = SHORT(patch->topoffset) << FRACBITS;
    }

    // [BH] override sprite offsets in WAD with those in sproffsets[] in info.c
    for (i = 0; i < NUMOFFSETS; i++)
    {
        int j = W_CheckNumForName(sproffsets[i].name);

        if (j >= 0)
        {
            j -= firstspritelump;
            spriteoffset[j] = sproffsets[i].x << FRACBITS;
            spritetopoffset[j] = sproffsets[i].y << FRACBITS;
        }
    }
}



//
// R_InitColormaps
//
extern int FindNearestColor(byte *palette, int red, int green, int blue);

void R_InitColormaps(void)
{
    int         lump;

    // Load in the light tables,
    //  256 byte align tables.
    lump = W_GetNumForName("COLORMAP");
    colormaps = (lighttable_t *)W_CacheLumpNum(lump, PU_STATIC);

    // [BH] There's a typo in dcolors.c, the source code of the utility Id
    // Software used to construct the palettes and colormaps for DOOM (see
    // http://www.doomworld.com/idgames/?id=16644). When constructing colormap
    // 32, which is used for the invulnerability powerup, the traditional
    // Y luminence values are used (see http://en.wikipedia.org/wiki/YIQ), but a
    // value of 0.144 is used when it should be 0.114. So I've grabbed the
    // offending code from dcolor.c, corrected it, put it here, and now colormap
    // 32 is manually calculated rather than grabbing it from the colormap lump.
    // The resulting  differences are minor.
    {
        int             i;
        float           red, green, blue, gray;
        byte            *palsrc, *palette;

        palsrc = palette = (byte *)W_CacheLumpName("PLAYPAL", PU_CACHE);

        for (i = 0; i < 255; i++)
        {
            red = *palsrc++ / 256.0f;
            green = *palsrc++ / 256.0f;
            blue = *palsrc++ / 256.0f;

            gray = red * 0.299f + green * 0.587f + blue * 0.114f/*0.144f*/;
            gray = 1.0f - gray;
            colormaps[32 * 256 + i] = FindNearestColor(palette, (int)(gray * 255.0f),
                                                       (int)(gray * 255.0f), (int)(gray * 255.0f));
        }
    }
}



//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData(void)
{
    R_InitTextures();
    R_InitFlats();
    R_InitSpriteLumps();
    R_InitColormaps();
}



//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
int R_FlatNumForName(char *name)
{
    int         i;
    char        namet[9];

    i = W_RangeCheckNumForName(firstflat, lastflat, name);

    if (i == -1)
    {
        namet[8] = 0;
        memcpy (namet, name, 8);
        I_Error("R_FlatNumForName: %s not found", namet);
    }
    return i - firstflat;
}




//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int R_CheckTextureNumForName(char *name)
{
    texture_t   *texture;
    int         key;

    // "NoTexture" marker.
    if (name[0] == '-')
        return 0;

    key = W_LumpNameHash(name) % numtextures;

    texture = textures_hashtable[key];

    while (texture != NULL)
    {
        if (!strncasecmp(texture->name, name, 8))
            return texture->index;

        texture = texture->next;
    }

    return -1;
}



//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int R_TextureNumForName(char *name)
{
    int     i;

    i = R_CheckTextureNumForName(name);

    if (i == -1)
    {
         I_Error ("R_TextureNumForName: %s not found",
                  name);
    }
    return i;
}




//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
int                     flatmemory;
int                     texturememory;
int                     spritememory;

void R_PrecacheLevel(void)
{
    char                *flatpresent;
    char                *texturepresent;
    char                *spritepresent;

    int                 i;
    int                 j;
    int                 k;
    int                 lump;

    texture_t           *texture;
    thinker_t           *th;
    spriteframe_t       *sf;

    if (demoplayback)
        return;

    // Precache flats.
    flatpresent = (char *)Z_Malloc(numflats, PU_STATIC, NULL);
    memset(flatpresent, 0, numflats);

    for (i = 0; i < numsectors; i++)
    {
        flatpresent[sectors[i].floorpic] = 1;
        flatpresent[sectors[i].ceilingpic] = 1;
    }

    flatmemory = 0;

    for (i = 0; i < numflats; i++)
    {
        if (flatpresent[i])
        {
            lump = firstflat + i;
            flatmemory += lumpinfo[lump].size;
            W_CacheLumpNum(lump, PU_CACHE);
        }
    }

    Z_Free(flatpresent);

    // Precache textures.
    texturepresent = (char *)Z_Malloc(numtextures, PU_STATIC, NULL);
    memset(texturepresent, 0, numtextures);

    for (i = 0; i < numsides; i++)
    {
        texturepresent[sides[i].toptexture] = 1;
        texturepresent[sides[i].midtexture] = 1;
        texturepresent[sides[i].bottomtexture] = 1;
    }

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependend
    //  name.
    texturepresent[skytexture] = 1;

    texturememory = 0;
    for (i = 0; i < numtextures; i++)
    {
        if (!texturepresent[i])
            continue;

        texture = textures[i];

        for (j = 0; j < texture->patchcount; j++)
        {
            lump = texture->patches[j].patch;
            texturememory += lumpinfo[lump].size;
            W_CacheLumpNum(lump, PU_CACHE);
        }
    }

    Z_Free(texturepresent);

    // Precache sprites.
    spritepresent = (char *)Z_Malloc(numsprites, PU_STATIC, NULL);
    memset(spritepresent, 0, numsprites);

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
            spritepresent[((mobj_t *)th)->sprite] = 1;
    }

    spritememory = 0;
    for (i = 0; i < numsprites; i++)
    {
        if (!spritepresent[i])
            continue;

        for (j = 0; j < sprites[i].numframes; j++)
        {
            sf = &sprites[i].spriteframes[j];
            for (k = 0; k < 8; k++)
            {
                lump = firstspritelump + sf->lump[k];
                spritememory += lumpinfo[lump].size;
                W_CacheLumpNum(lump, PU_CACHE);
            }
        }
    }

    Z_Free(spritepresent);
}