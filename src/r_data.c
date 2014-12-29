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

#include "d_deh.h"
#include "doomstat.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_misc.h"
#include "p_local.h"
#include "r_sky.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//

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
unsigned int    **texturecolumnofs;
byte            **texturecomposite;

// for global animation
int             *flattranslation;
int             *texturetranslation;
byte            **flatfullbright;

// needed for pre rendering
fixed_t         *spritewidth;
fixed_t         *spriteheight;
fixed_t         *spriteoffset;
fixed_t         *spritetopoffset;

lighttable_t    *colormaps;

boolean         *lookuptextures;
int             lookupprogress;

static byte notgray[256] =
{
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static byte notgrayorbrown[256] =
{
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static byte redonly[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static byte greenonly1[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static byte greenonly2[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static byte whiteonly[256] =
{
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static struct
{
    char        texture[9];
    char        flat[9];
    byte        *colormask;
} fullbright[] = {

    // textures
    { "COMP2",    "XXXXXXXX", notgrayorbrown }, { "COMPSTA1", "",         notgray        },
    { "COMPSTA2", "",         notgray        }, { "COMPUTE1", "",         notgrayorbrown },
    { "COMPUTE2", "",         notgrayorbrown }, { "COMPUTE3", "",         notgrayorbrown },
    { "EXITSIGN", "",         notgray        }, { "EXITSTON", "",         notgray        },
    { "PLANET1",  "",         notgray        }, { "SILVER2",  "",         notgray        },
    { "SILVER3",  "",         notgrayorbrown }, { "SLADSKUL", "",         redonly        },
    { "SW1BRCOM", "",         redonly        }, { "SW1BRIK",  "",         redonly        },
    { "SW1BRN1",  "",         redonly        }, { "SW1COMM",  "",         redonly        },
    { "SW1DIRT",  "",         redonly        }, { "SW1MET2",  "",         redonly        },
    { "SW1STARG", "",         redonly        }, { "SW1STON1", "",         redonly        },
    { "SW1STON2", "",         redonly        }, { "SW1STONE", "",         redonly        },
    { "SW1STRTN", "",         redonly        }, { "SW2BLUE",  "",         redonly        },
    { "SW2BRCOM", "",         greenonly2     }, { "SW2BRIK",  "",         greenonly1     },
    { "SW2BRN1",  "",         greenonly1     }, { "SW2BRN2",  "",         notgray        },
    { "SW2BRNGN", "",         notgray        }, { "SW2COMM",  "",         greenonly1     },
    { "SW2COMP",  "",         redonly        }, { "SW2DIRT",  "",         greenonly1     },
    { "SW2EXIT",  "",         notgray        }, { "SW2GRAY",  "",         notgray        },
    { "SW2GRAY1", "",         notgray        }, { "SW2GSTON", "",         redonly        },
    { "SW2MARB",  "",         greenonly1     }, { "SW2MET2",  "",         greenonly1     },
    { "SW2METAL", "",         greenonly1     }, { "SW2MOD1",  "",         notgrayorbrown },
    { "SW2PANEL", "",         redonly        }, { "SW2ROCK",  "",         redonly        },
    { "SW2SLAD",  "",         redonly        }, { "SW2STARG", "",         greenonly1     },
    { "SW2STON1", "",         greenonly1     }, { "SW2STON2", "",         greenonly1     },
    { "SW2STON6", "",         redonly        }, { "SW2STONE", "",         greenonly1     },
    { "SW2STRTN", "",         greenonly1     }, { "SW2TEK",   "",         greenonly1     },
    { "SW2VINE",  "",         greenonly1     }, { "SW2WDMET", "",         redonly        },
    { "SW2WOOD",  "",         redonly        }, { "SW2ZIM",   "",         redonly        },
    { "WOOD4",    "",         redonly        }, { "WOODGARG", "",         redonly        },
    { "WOODSKUL", "",         redonly        }, { "ZELDOOR",  "",         redonly        },

    // flats
    { "",         "CEIL1_2",  whiteonly      }, { "",         "CEIL1_3",  whiteonly      },
    { "",         "",         0              }
};

extern boolean  brightmaps;

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
void R_DrawColumnInCache(const column_t *patch, byte *cache, int originy,
                         int cacheheight, byte *marks, boolean oldmethod)
{
    while (patch->topdelta != 0xff)
    {
        int     count = patch->length;
        int     position = originy + patch->topdelta;
        byte    *source = (byte *)patch + 3;

        if (position < 0)
        {
            count += position;
            if (!oldmethod)
                source -= position;
            position = 0;
        }

        if (position + count > cacheheight)
            count = cacheheight - position;

        if (count > 0)
        {
            memcpy(cache + position, source, count);

            // killough 4/9/98: remember which cells in column have been drawn,
            // so that column can later be converted into a series of posts, to
            // fix the Medusa bug.
            memset(marks + position, 0xff, count);
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
    byte                *block = Z_Malloc(texturecompositesize[texnum], PU_STATIC,
                                          (void **)&texturecomposite[texnum]);
    texture_t           *texture = textures[texnum];

    // Composite the columns together.
    texpatch_t          *patch = texture->patches;
    short               *collump = texturecolumnlump[texnum];
    unsigned int        *colofs = texturecolumnofs[texnum];
    int                 i = texture->patchcount;

    // killough 4/9/98: marks to identify transparent regions in merged textures
    byte                *marks = calloc(texture->width, texture->height), *source;
    boolean             tekwall1 = (texnum == R_CheckTextureNumForName("TEKWALL1"));

    for (; --i >= 0; patch++)
    {
        patch_t         *realpatch = W_CacheLumpNum(patch->patch, PU_CACHE);
        int             x1 = MAX(0, patch->originx);
        int             x2 = MIN(x1 + SHORT(realpatch->width), texture->width);
        const int       *cofs = realpatch->columnofs - x1;

        for (; x1 < x2 ; x1++)
            if (collump[x1] == -1)      // Column has multiple patches?
                // killough 1/25/98, 4/9/98: Fix medusa bug.
                R_DrawColumnInCache((column_t *)((byte *)realpatch + LONG(cofs[x1])),
                    block + colofs[x1], patch->originy, texture->height,
                    marks + x1 * texture->height, tekwall1);
    }

    // killough 4/9/98: Next, convert multipatched columns into true columns,
    // to fix Medusa bug while still allowing for transparent regions.
    source = (byte *)malloc(texture->height);   // temporary column
    for (i = 0; i < texture->width; i++)
        if (collump[i] == -1)                   // process only multipatched columns
        {
            column_t    *col = (column_t *)(block + colofs[i] - 3);     // cached column
            const byte  *mark = marks + i * texture->height;
            int         j = 0;

            // save column in temporary so we can shuffle it around
            memcpy(source, (byte *)col + 3, texture->height);

            for (;;)  // reconstruct the column by scanning transparency marks
            {
                unsigned int    len;                    // killough 12/98

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
    free(source);       // free temporary column
    free(marks);        // free transparency marks

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
    const texture_t     *texture = textures[texnum];

    // Composited texture not created yet.
    short               *collump = texturecolumnlump[texnum];
    unsigned int        *colofs = texturecolumnofs[texnum];

    // killough 4/9/98: keep count of posts in addition to patches.
    // Part of fix for medusa bug for multipatched 2s normals.
    struct {
        unsigned int    patches, posts;
    } *count = calloc(sizeof(*count), texture->width);

    // killough 12/98: First count the number of patches per column.
    const texpatch_t   *patch = texture->patches;
    int                i = texture->patchcount;

    while (--i >= 0)
    {
        int             pat = patch->patch;
        const patch_t   *realpatch = (patch_t *)W_CacheLumpNum(pat, PU_CACHE);
        int             x, x1 = MAX(0, (patch++)->originx);
        int             x2 = MIN(x1 + SHORT(realpatch->width), texture->width);
        const int       *cofs = realpatch->columnofs - x1;

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
        unsigned int            limit = texture->height * 3 + 3;   // absolute column size limit

        for (i = texture->patchcount, patch = texture->patches; --i >= 0;)
        {
            int                 pat = patch->patch;
            const patch_t       *realpatch = (patch_t *)W_CacheLumpNum(pat, PU_CACHE);
            int                 x, x1 = MAX(0, (patch++)->originx);
            int                 x2 = MIN(x1 + SHORT(realpatch->width), texture->width);
            const int           *cofs = realpatch->columnofs - x1;

            for (x = x1; x < x2; x++)
                if (count[x].patches > 1)               // Only multipatched columns
                {
                    const column_t      *col = (column_t *)((byte *)realpatch + LONG(cofs[x]));
                    const byte          *base = (const byte *)col;

                    // count posts
                    for (; col->topdelta != 0xff; count[x].posts++)
                        if ((unsigned int)((byte *)col - base) <= limit)
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
        int     x = texture->width;
        int     height = texture->height;
        int     csize = 0;                              // killough 10/98

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
        return ((byte *)W_CacheLumpNum(lump, PU_CACHE) + ofs);

    if (!texturecomposite[tex])
        R_GenerateComposite(tex);

    return (texturecomposite[tex] + ofs);
}

static void GenerateTextureHashTable(void)
{
    texture_t   **rover;
    int         i;
    int         key;

    textures_hashtable = (texture_t **)Z_Malloc(sizeof(texture_t *) * numtextures, PU_STATIC, 0);

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
            rover = &(*rover)->next;

        // Hook into hash table
        textures[i]->next = NULL;
        *rover = textures[i];
    }
}

//
// R_DoomTextureHacks
//
void R_DoomTextureHacks(texture_t *t)
{
    if (t->height == 128 &&
        t->patches[0].originy == -8 &&
        t->name[0] == 'S' &&
        t->name[1] == 'K' &&
        t->name[2] == 'Y' &&
        t->name[3] == '1' &&
        t->name[4] == 0)
    {
        t->patches[0].originy = 0;
    }

    if (t->height == 128 &&
        t->patches[0].originy == -4 &&
        t->patches[1].originy == -4 &&
        t->name[0] == 'B' &&
        t->name[1] == 'I' &&
        t->name[2] == 'G' &&
        t->name[3] == 'D' &&
        t->name[4] == 'O' &&
        t->name[5] == 'O' &&
        t->name[6] == 'R' &&
        t->name[7] == '7')
    {
        t->patches[0].originy = t->patches[1].originy = 0;
    }
}

//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
static void R_InitTextures(void)
{
    maptexture_t *mtexture;
    texture_t    *texture;
    mappatch_t   *mpatch;
    texpatch_t   *patch;

    int          i;
    int          j;

    int          *maptex;
    int          *maptex2;
    int          *maptex1;

    char         name[9];
    char         *names;
    char         *name_p;

    int          *patchlookup;

    int          totalwidth;
    int          nummappatches;
    int          offset;
    int          maxoff;
    int          maxoff2;
    int          numtextures1;
    int          numtextures2;

    int          *directory;

    // Load the patch names from pnames.lmp.
    name[8] = 0;
    names = (char *)W_CacheLumpName("PNAMES", PU_STATIC);
    nummappatches = LONG(*((int *)names));
    name_p = names + 4;
    patchlookup = (int *)Z_Malloc(nummappatches * sizeof(*patchlookup), PU_STATIC, NULL);

    for (i = 0; i < nummappatches; i++)
    {
        M_StringCopy(name, name_p + i * 8, sizeof(name));
        patchlookup[i] = W_CheckNumForName(name);
    }
    W_ReleaseLumpName("PNAMES");

    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex = maptex1 = W_CacheLumpName("TEXTURE1", PU_STATIC);
    numtextures1 = LONG(*maptex);
    maxoff = W_LumpLength(W_GetNumForName("TEXTURE1"));
    directory = maptex + 1;

    if (W_CheckNumForName("TEXTURE2") != -1)
    {
        maptex2 = W_CacheLumpName("TEXTURE2", PU_STATIC);
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

    textures = Z_Malloc(numtextures * sizeof(*textures), PU_STATIC, 0);
    texturecolumnlump = Z_Malloc(numtextures * sizeof(*texturecolumnlump), PU_STATIC, 0);
    texturecolumnofs = Z_Malloc(numtextures * sizeof(*texturecolumnofs), PU_STATIC, 0);
    texturecomposite = Z_Malloc(numtextures * sizeof(*texturecomposite), PU_STATIC, 0);
    texturecompositesize = Z_Malloc(numtextures * sizeof(*texturecompositesize), PU_STATIC, 0);
    texturewidthmask = Z_Malloc(numtextures * sizeof(*texturewidthmask), PU_STATIC, 0);
    textureheight = Z_Malloc(numtextures * sizeof(*textureheight), PU_STATIC, 0);
    texturefullbright = Z_Malloc(numtextures * sizeof(*texturefullbright), PU_STATIC, 0);

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
            I_Error("R_InitTextures: bad texture directory");

        mtexture = (maptexture_t *)((byte *)maptex + offset);

        texture = textures[i] = Z_Malloc(sizeof(texture_t) +
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
                patch->patch = 0;       // [crispy] make non-fatal
        }
        texturecolumnlump[i] = Z_Malloc(texture->width * sizeof(**texturecolumnlump),  PU_STATIC, 0);
        texturecolumnofs[i] = Z_Malloc(texture->width * sizeof(**texturecolumnofs), PU_STATIC, 0);

        for (j = 1; j * 2 <= texture->width; j <<= 1);

        texturewidthmask[i] = j - 1;
        textureheight[i] = texture->height << FRACBITS;

        totalwidth += texture->width;

        R_DoomTextureHacks(texture);
    }

    Z_Free(patchlookup);

    W_ReleaseLumpName("TEXTURE1");
    if (maptex2)
        W_ReleaseLumpName("TEXTURE2");

    lookuptextures = Z_Malloc(numtextures * sizeof(boolean), PU_STATIC, 0);

    for (i = 0; i < numtextures; i++)
        lookuptextures[i] = false;

    lookupprogress = numtextures;

    // Precalculate whatever possible.

    for (i = 0; i < numtextures; i++)
        R_GenerateLookup(i);

    // Create translation table for global animation.
    texturetranslation = Z_Malloc((numtextures + 1) * sizeof(*texturetranslation), PU_STATIC, 0);

    for (i = 0; i < numtextures; i++)
        texturetranslation[i] = i;

    GenerateTextureHashTable();

    // [BH] Initialize partially fullbright textures.
    if (brightmaps)
    {
        memset(texturefullbright, 0, numtextures * sizeof(*texturefullbright));
        i = 0;
        while (fullbright[i].colormask)
        {
            if (fullbright[i].texture)
            {
                int num = R_CheckTextureNumForName(fullbright[i].texture);

                if (num != -1)
                    texturefullbright[num] = fullbright[i].colormask;
                i++;
            }
        }
    }
}

//
// R_InitFlats
//
void R_InitFlats(void)
{
    int i;

    firstflat = W_GetNumForName("F_START") + 1;
    lastflat = W_GetNumForName("F_END") - 1;
    numflats = lastflat - firstflat + 1;

    flatfullbright = Z_Malloc(numflats * sizeof(*flatfullbright), PU_STATIC, 0);

    // Create translation table for global animation.
    flattranslation = Z_Malloc((numflats + 1) * sizeof(*flattranslation), PU_STATIC, 0);

    for (i = 0; i < numflats; i++)
        flattranslation[i] = i;

    // [BH] Initialize partially fullbright flats.
    if (brightmaps)
    {
        memset(flatfullbright, 0, numflats * sizeof(*flatfullbright));
        i = 0;
        while (fullbright[i].colormask)
        {
            if (fullbright[i].flat)
            {
                int num = R_CheckFlatNumForName(fullbright[i].flat);

                if (num != -1)
                    flatfullbright[num] = fullbright[i].colormask;
                i++;
            }
        }
    }
}

//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
void R_InitSpriteLumps(void)
{
    int i;

    firstspritelump = W_GetNumForName("S_START") + 1;
    lastspritelump = W_GetNumForName("S_END") - 1;

    numspritelumps = lastspritelump - firstspritelump + 1;
    spritewidth = Z_Malloc(numspritelumps * sizeof(*spritewidth), PU_STATIC, 0);
    spriteheight = Z_Malloc(numspritelumps * sizeof(*spriteheight), PU_STATIC, 0);
    spriteoffset = Z_Malloc(numspritelumps * sizeof(*spriteoffset), PU_STATIC, 0);
    spritetopoffset = Z_Malloc(numspritelumps * sizeof(*spritetopoffset), PU_STATIC, 0);

    for (i = 0; i < numspritelumps; i++)
    {
        patch_t *patch = W_CacheLumpNum(firstspritelump + i, PU_CACHE);

        spritewidth[i] = SHORT(patch->width) << FRACBITS;
        spriteheight[i] = SHORT(patch->height) << FRACBITS;
        spriteoffset[i] = SHORT(patch->leftoffset) << FRACBITS;
        spritetopoffset[i] = SHORT(patch->topoffset) << FRACBITS;

        // [BH] override sprite offsets in WAD with those in sproffsets[] in info.c
        if (!FREEDOOM && !hacx && !dehacked)
        {
            int j = 0;

            while (sproffsets[j].name[0])
            {
                if (sproffsets[j].canmodify || BTSX)
                {
                    if (i == W_CheckNumForName(sproffsets[j].name) - firstspritelump)
                    {
                        spriteoffset[i] = SHORT(sproffsets[j].x) << FRACBITS;
                        spritetopoffset[i] = SHORT(sproffsets[j].y) << FRACBITS;
                        break;
                    }
                }
                j++;
            }
        }
    }

    if (FREEDOOM)
    {
        states[S_BAR1].tics = 0;
        mobjinfo[MT_BARREL].spawnstate = S_BAR2;
        mobjinfo[MT_BARREL].frames = 2;
    }
    else if (chex)
    {
        states[S_POSS_DIE5].tics = 0;
        states[S_POSS_XDIE9].tics = 0;
        states[S_SPOS_DIE5].tics = 0;
        states[S_SPOS_XDIE9].tics = 0;
        states[S_TROO_DIE5].tics = 0;
        states[S_TROO_XDIE8].tics = 0;
        states[S_SARG_DIE6].tics = 0;
        states[S_BOSS_DIE7].tics = 0;
    }
    else if (hacx)
    {
        mobjinfo[MT_HEAD].flags2 |= MF2_DONOTMAP;
        mobjinfo[MT_INV].flags2 &= ~MF2_TRANSLUCENT_33;
        mobjinfo[MT_INS].flags2 &= ~(MF2_TRANSLUCENT_33 | MF2_FLOATBOB | MF2_NOFOOTCLIP);
        mobjinfo[MT_MISC14].flags2 &= ~(MF2_FLOATBOB | MF2_NOFOOTCLIP);
    }
    else if (dehacked)
    {
        for (i = 0; i < NUMMOBJTYPES; i++)
        {
            if ((mobjinfo[i].flags & MF_SHOOTABLE))
                mobjinfo[i].projectilepassheight = mobjinfo[i].height;
            if (mobjinfo[i].flags2 & MF2_BLOOD)
                mobjinfo[i].flags2 = MF2_BLOOD;
            else
                mobjinfo[i].flags2 = 0;
        }
    }

    if (!BTSX)
    {
        if (mergedcacodemon)
            mobjinfo[MT_HEAD].blood = MT_BLOOD;

        if (mergednoble)
        {
            mobjinfo[MT_BRUISER].blood = MT_BLOOD;
            mobjinfo[MT_KNIGHT].blood = MT_BLOOD;
        }
    }
}

//
// R_InitColormaps
//
extern int FindNearestColor(byte *palette, int red, int green, int blue);

byte grays[256];

void R_InitColormaps(void)
{
    int         lump;
    boolean     COLORMAP = (W_CheckMultipleLumps("COLORMAP") > 1);

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
    // The resulting differences are minor.
    {
        int     i;
        float   red, green, blue, gray;
        byte    *palsrc, *palette;

        palsrc = palette = W_CacheLumpName("PLAYPAL", PU_CACHE);

        for (i = 0; i < 255; i++)
        {
            red = *palsrc++ / 256.0f;
            green = *palsrc++ / 256.0f;
            blue = *palsrc++ / 256.0f;

            gray = red * 0.299f + green * 0.587f + blue * 0.114f/*0.144f*/;
            grays[i] = FindNearestColor(palette, (int)(gray * 255.0f),
                                        (int)(gray * 255.0f), (int)(gray * 255.0f));
            if (!COLORMAP)
            {
                gray = (1.0f - gray) * 255.0f;
                colormaps[32 * 256 + i] = FindNearestColor(palette, (int)gray, (int)gray, (int)gray);
            }
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
    int  i;

    i = W_RangeCheckNumForName(firstflat, lastflat, name);

    if (i == -1)
        return 0;
    return (i - firstflat);
}

//
// R_CheckFlatNumForName
// Retrieval, get a flat number for a flat name. No error.
//
int R_CheckFlatNumForName(char *name)
{
    int  i;

    for (i = firstflat; i <= lastflat; i++)
        if (!strncasecmp(lumpinfo[i].name, name, 8))
            return (i - firstflat);

    return -1;
}

//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int R_CheckTextureNumForName(char *name)
{
    texture_t *texture;
    int       key;

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
    int i = R_CheckTextureNumForName(name);

    if (i == -1)
        return 0;       // [crispy] make non-fatal
    return i;
}

//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
int flatmemory;
int texturememory;
int spritememory;

void R_PrecacheLevel(void)
{
    char          *flatpresent;
    char          *texturepresent;
    char          *spritepresent;

    int           i;
    int           j;
    int           k;
    int           lump;

    texture_t     *texture;
    thinker_t     *th;
    spriteframe_t *sf;

    // Precache flats.
    flatpresent = Z_Malloc(numflats, PU_STATIC, NULL);
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
    texturepresent = Z_Malloc(numtextures, PU_STATIC, NULL);
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
    spritepresent = Z_Malloc(numsprites, PU_STATIC, NULL);
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
