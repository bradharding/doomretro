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

#include "c_console.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_tick.h"
#include "r_sky.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//

// killough 4/17/98: make firstcolormaplump, lastcolormaplump external
static int  firstcolormaplump;

int         firstflat;
static int  lastflat;
int         numflats;

int         firstspritelump;
int         lastspritelump;

dboolean    notranslucency;
dboolean    telefragonmap30;

int         numtextures;
texture_t   **textures;

// needed for texture pegging
fixed_t     *textureheight;
byte        **texturefullbright;
dboolean    *nobrightmap;

// for global animation
int         *flattranslation;
int         *texturetranslation;

// needed for prerendering
fixed_t     *spritewidth;
fixed_t     *spriteheight;
fixed_t     *spriteoffset;
fixed_t     *spritetopoffset;

fixed_t     *newspriteoffset;
fixed_t     *newspritetopoffset;

dboolean    r_fixspriteoffsets = r_fixspriteoffsets_default;

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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
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

static byte greenonly3[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define DOOM1AND2   0
#define DOOM1ONLY   1
#define DOOM2ONLY   2

static struct
{
    char    texture[9];
    int     game;
    byte    *colormask;
} fullbright[] = {
    { "COMP2",    DOOM1AND2, notgrayorbrown }, { "COMPSTA1", DOOM1AND2, notgray        },
    { "COMPSTA2", DOOM1AND2, notgray        }, { "COMPUTE1", DOOM1AND2, notgrayorbrown },
    { "COMPUTE2", DOOM1AND2, notgrayorbrown }, { "COMPUTE3", DOOM1AND2, notgrayorbrown },
    { "EXITSIGN", DOOM1AND2, notgray        }, { "EXITSTON", DOOM1AND2, notgray        },
    { "M_TEC",    DOOM2ONLY, greenonly2     }, { "PLANET1",  DOOM1AND2, notgray        },
    { "PNK4EXIT", DOOM2ONLY, redonly        }, { "SILVER2",  DOOM1AND2, notgray        },
    { "SILVER3",  DOOM1AND2, notgrayorbrown }, { "SLAD2",    DOOM2ONLY, notgrayorbrown },
    { "SLAD3",    DOOM2ONLY, notgrayorbrown }, { "SLAD4",    DOOM2ONLY, notgrayorbrown },
    { "SLAD5",    DOOM2ONLY, notgrayorbrown }, { "SLAD6",    DOOM2ONLY, notgrayorbrown },
    { "SLAD7",    DOOM2ONLY, notgrayorbrown }, { "SLAD8",    DOOM2ONLY, notgrayorbrown },
    { "SLAD9",    DOOM2ONLY, notgrayorbrown }, { "SLAD10",   DOOM2ONLY, notgrayorbrown },
    { "SLAD11",   DOOM2ONLY, notgrayorbrown }, { "SLADRIP1", DOOM2ONLY, notgrayorbrown },
    { "SLADRIP3", DOOM2ONLY, notgrayorbrown }, { "SLADSKUL", DOOM1AND2, redonly        },
    { "SW1BRCOM", DOOM1AND2, redonly        }, { "SW1BRIK",  DOOM1AND2, redonly        },
    { "SW1BRN1",  DOOM2ONLY, redonly        }, { "SW1COMM",  DOOM1AND2, redonly        },
    { "SW1DIRT",  DOOM1AND2, redonly        }, { "SW1MET2",  DOOM1AND2, redonly        },
    { "SW1STARG", DOOM2ONLY, redonly        }, { "SW1STON1", DOOM1AND2, redonly        },
    { "SW1STON2", DOOM2ONLY, redonly        }, { "SW1STONE", DOOM1AND2, redonly        },
    { "SW1STRTN", DOOM1AND2, redonly        }, { "SW2BLUE",  DOOM1AND2, redonly        },
    { "SW2BRCOM", DOOM1AND2, greenonly2     }, { "SW2BRIK",  DOOM1AND2, greenonly1     },
    { "SW2BRN1",  DOOM1AND2, greenonly1     }, { "SW2BRN2",  DOOM1AND2, greenonly1     },
    { "SW2BRNGN", DOOM1AND2, greenonly2     }, { "SW2COMM",  DOOM1AND2, greenonly1     },
    { "SW2COMP",  DOOM1AND2, redonly        }, { "SW2DIRT",  DOOM1AND2, greenonly1     },
    { "SW2EXIT",  DOOM1AND2, notgray        }, { "SW2GRAY",  DOOM1AND2, notgray        },
    { "SW2GRAY1", DOOM1AND2, notgray        }, { "SW2GSTON", DOOM1AND2, redonly        },
    { "SW2MARB",  DOOM2ONLY, redonly        }, { "SW2MET2",  DOOM1AND2, greenonly1     },
    { "SW2METAL", DOOM1AND2, greenonly3     }, { "SW2MOD1",  DOOM1AND2, notgrayorbrown },
    { "SW2PANEL", DOOM1AND2, redonly        }, { "SW2ROCK",  DOOM1AND2, redonly        },
    { "SW2SLAD",  DOOM1AND2, redonly        }, { "SW2STARG", DOOM2ONLY, greenonly1     },
    { "SW2STON1", DOOM1AND2, greenonly1     }, { "SW2STON2", DOOM1ONLY, redonly        },
    { "SW2STON2", DOOM2ONLY, greenonly1     }, { "SW2STON6", DOOM1AND2, redonly        },
    { "SW2STONE", DOOM1AND2, greenonly1     }, { "SW2STRTN", DOOM1AND2, greenonly1     },
    { "SW2TEK",   DOOM1AND2, greenonly1     }, { "SW2VINE",  DOOM1AND2, greenonly1     },
    { "SW2WOOD",  DOOM1AND2, redonly        }, { "SW2ZIM",   DOOM1AND2, redonly        },
    { "WOOD4",    DOOM1AND2, redonly        }, { "WOODGARG", DOOM1AND2, redonly        },
    { "WOODSKUL", DOOM1AND2, redonly        }, { "ZELDOOR",  DOOM1AND2, redonly        },
    { "",         0,         0              }
};

extern char *pwadfile;

//
// R_GetTextureColumn
//
byte *R_GetTextureColumn(const rpatch_t *texpatch, int col)
{
    while (col < 0)
        col += texpatch->width;

    return texpatch->columns[col & texpatch->widthmask].pixels;
}

//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
static void R_InitTextures(void)
{
    const maptexture_t  *mtexture;
    texture_t           *texture;
    int                 i;
    int                 j;
    int                 maptex_lump[2] = { -1, -1 };
    const int           *maptex1;
    const int           *maptex2;
    char                name[9];
    int                 names_lump;     // cph - new wad lump handling
    const char          *names;         // cph -
    const char          *name_p;        // const*'s
    int                 *patchlookup;
    int                 nummappatches;
    int                 maxoff, maxoff2;
    int                 numtextures1;
    int                 numtextures2;
    const int           *directory;

    // Load the patch names from pnames.lmp.
    name[8] = '\0';
    names = W_CacheLumpNum((names_lump = W_GetNumForName("PNAMES")));
    nummappatches = LONG(*((const int *)names));
    name_p = names + 4;
    patchlookup = malloc(nummappatches * sizeof(*patchlookup));   // killough

    for (i = 0; i < nummappatches; i++)
    {
        strncpy(name, name_p + i * 8, 8);
        patchlookup[i] = W_CheckNumForName(name);
    }

    W_UnlockLumpNum(names_lump);       // cph - release the lump

    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex_lump[0] = W_GetNumForName("TEXTURE1");
    maptex1 = W_CacheLumpNum(maptex_lump[0]);
    numtextures1 = LONG(*maptex1);
    maxoff = W_LumpLength(maptex_lump[0]);
    directory = maptex1 + 1;

    if (W_CheckNumForName("TEXTURE2") != -1)
    {
        maptex_lump[1] = W_GetNumForName("TEXTURE2");
        maptex2 = W_CacheLumpNum(maptex_lump[1]);
        numtextures2 = LONG(*maptex2);
        maxoff2 = W_LumpLength(maptex_lump[1]);
    }
    else
    {
        maptex2 = NULL;
        numtextures2 = 0;
        maxoff2 = 0;
    }

    numtextures = numtextures1 + numtextures2;

    // killough 4/9/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof
    textures = Z_Malloc(numtextures * sizeof(*textures), PU_STATIC, NULL);
    textureheight = Z_Malloc(numtextures * sizeof(*textureheight), PU_STATIC, NULL);

    for (i = 0; i < numtextures; i++, directory++)
    {
        const mappatch_t    *mpatch;
        texpatch_t          *patch;
        int                 offset;

        if (i == numtextures1)
        {
            // Start looking in second texture file.
            maptex1 = maptex2;
            maxoff = maxoff2;
            directory = maptex1 + 1;
        }

        offset = LONG(*directory);

        if (offset > maxoff)
            I_Error("R_InitTextures: Bad texture directory");

        mtexture = (const maptexture_t *)((const byte *)maptex1 + offset);

        texture = textures[i] = Z_Malloc(sizeof(texture_t) + sizeof(texpatch_t)
            * (SHORT(mtexture->patchcount) - 1), PU_STATIC, 0);

        texture->width = SHORT(mtexture->width);
        texture->height = SHORT(mtexture->height);
        texture->patchcount = SHORT(mtexture->patchcount);

        for (j = 0; j < sizeof(texture->name); j++)
            texture->name[j] = mtexture->name[j];

        mpatch = mtexture->patches;
        patch = texture->patches;

        for (j = 0; j < texture->patchcount; j++, mpatch++, patch++)
        {
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);
            patch->patch = patchlookup[SHORT(mpatch->patch)];

            if (patch->patch == -1)
                C_Warning("Patch %i is missing in the <b>%.8s</b> texture.", SHORT(mpatch->patch), texture->name);
        }

        for (j = 1; j * 2 <= texture->width; j <<= 1);

        texture->widthmask = j - 1;
        textureheight[i] = texture->height << FRACBITS;
    }

    free(patchlookup);          // killough

    for (i = 0; i < 2; i++)     // cph - release the TEXTUREx lumps
        if (maptex_lump[i] != -1)
            W_UnlockLumpNum(maptex_lump[i]);

    // Create translation table for global animation.
    // killough 4/9/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof
    texturetranslation = Z_Malloc((numtextures + 1) * sizeof(*texturetranslation), PU_STATIC, NULL);

    for (i = 0; i < numtextures; i++)
        texturetranslation[i] = i;

    // killough 1/31/98: Initialize texture hash table
    for (i = 0; i < numtextures; i++)
        textures[i]->index = -1;

    while (--i >= 0)
    {
        j = W_LumpNameHash(textures[i]->name) % (unsigned int)numtextures;

        textures[i]->next = textures[j]->index; // Prepend to chain
        textures[j]->index = i;
    }

    // [BH] Initialize partially fullbright textures.
    texturefullbright = Z_Calloc(numtextures, sizeof(*texturefullbright), PU_STATIC, NULL);
    nobrightmap = Z_Calloc(numtextures, sizeof(*nobrightmap), PU_STATIC, NULL);

    i = 0;

    while (fullbright[i].colormask)
    {
        int game = fullbright[i].game;

        if (fullbright[i].texture[0] != '\0' && (game == DOOM1AND2
            || (gamemission == doom && game == DOOM1ONLY) || (gamemission != doom && game == DOOM2ONLY)))
        {
            int num = R_CheckTextureNumForName(fullbright[i].texture);

            if (num != -1)
                texturefullbright[num] = fullbright[i].colormask;
        }

        i++;
    }
}

//
// R_InitFlats
//
static void R_InitFlats(void)
{
    firstflat = W_GetNumForName("F_START") + 1;
    lastflat = W_GetNumForName("F_END") - 1;
    numflats = lastflat - firstflat + 1;

    // Create translation table for global animation.
    flattranslation = Z_Malloc((numflats + 1) * sizeof(*flattranslation), PU_STATIC, NULL);

    for (int i = 0; i < numflats; i++)
        flattranslation[i] = i;
}

//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
static void R_InitSpriteLumps(void)
{
    int numspritelumps;

    firstspritelump = W_GetNumForName("S_START") + 1;
    lastspritelump = W_GetNumForName("S_END") - 1;

    numspritelumps = lastspritelump - firstspritelump + 1;
    spritewidth = Z_Malloc(numspritelumps * sizeof(*spritewidth), PU_STATIC, NULL);
    spriteheight = Z_Malloc(numspritelumps * sizeof(*spriteheight), PU_STATIC, NULL);
    spriteoffset = Z_Malloc(numspritelumps * sizeof(*spriteoffset), PU_STATIC, NULL);
    spritetopoffset = Z_Malloc(numspritelumps * sizeof(*spritetopoffset), PU_STATIC, NULL);

    newspriteoffset = Z_Malloc(numspritelumps * sizeof(*newspriteoffset), PU_STATIC, NULL);
    newspritetopoffset = Z_Malloc(numspritelumps * sizeof(*newspritetopoffset), PU_STATIC, NULL);

    for (int i = 0; i < numspritelumps; i++)
    {
        patch_t *patch = W_CacheLumpNum(firstspritelump + i);

        if (patch)
        {
            spritewidth[i] = SHORT(patch->width) << FRACBITS;
            spriteheight[i] = SHORT(patch->height) << FRACBITS;
            spriteoffset[i] = newspriteoffset[i] = SHORT(patch->leftoffset) << FRACBITS;
            spritetopoffset[i] = newspritetopoffset[i] = SHORT(patch->topoffset) << FRACBITS;

            // [BH] override sprite offsets in WAD with those in sproffsets[] in info.c
            if (!FREEDOOM && !hacx)
            {
                int j = 0;

                while (*sproffsets[j].name)
                {
                    if (i == W_CheckNumForName(sproffsets[j].name) - firstspritelump
                        && spritewidth[i] == (SHORT(sproffsets[j].width) << FRACBITS)
                        && spriteheight[i] == (SHORT(sproffsets[j].height) << FRACBITS)
                        && ((!BTSX && !sprfix18) || sproffsets[j].sprfix18))
                    {
                        newspriteoffset[i] = SHORT(sproffsets[j].x) << FRACBITS;
                        newspritetopoffset[i] = SHORT(sproffsets[j].y) << FRACBITS;
                        break;
                    }

                    j++;
                }
            }
        }
    }

    // [BH] compatibility fixes
    if (FREEDOOM)
    {
        states[S_BAR1].tics = 0;
        mobjinfo[MT_BARREL].spawnstate = S_BAR2;
        mobjinfo[MT_BARREL].frames = 0;

        mobjinfo[MT_HEAD].blood = MT_BLOOD;
        mobjinfo[MT_BRUISER].blood = MT_BLOOD;
        mobjinfo[MT_KNIGHT].blood = MT_BLOOD;
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
        mobjinfo[MT_HEAD].flags2 |= MF2_DONTMAP;
        mobjinfo[MT_INV].flags2 &= ~MF2_TRANSLUCENT_33;
        mobjinfo[MT_INS].flags2 &= ~(MF2_TRANSLUCENT_33 | MF2_FLOATBOB | MF2_NOFOOTCLIP);
        mobjinfo[MT_MISC14].flags2 &= ~(MF2_FLOATBOB | MF2_NOFOOTCLIP);
        mobjinfo[MT_BFG].flags2 &= ~MF2_TRANSLUCENT;
        mobjinfo[MT_HEAD].blood = MT_BLOOD;
        mobjinfo[MT_BRUISER].blood = MT_BLOOD;
        mobjinfo[MT_KNIGHT].blood = MT_BLOOD;
    }

    SC_Open("DRCOMPAT");

    while (SC_GetString())
    {
        if (M_StringCompare(sc_String, "NOTRANSLUCENCY"))
        {
            SC_MustGetString();

            if (M_StringCompare(pwadfile, removeext(sc_String)))
                notranslucency = true;
        }
        else if (M_StringCompare(sc_String, "TELEFRAGONMAP30"))
        {
            SC_MustGetString();

            if (M_StringCompare(pwadfile, removeext(sc_String)))
                telefragonmap30 = true;
        }
    }

    SC_Close();
}

//
// R_InitColormaps
//
// killough 3/20/98: rewritten to allow dynamic colormaps
// and to remove unnecessary 256-byte alignment
//
// killough 4/4/98: Add support for C_START/C_END markers
//
byte grays[256];

static void R_InitColormaps(void)
{
    dboolean    COLORMAP = (W_CheckMultipleLumps("COLORMAP") > 1);
    byte        *palsrc;
    byte        *palette;
    wadfile_t   *colormapwad;

    if (W_CheckNumForName("C_START") >= 0 && W_CheckNumForName("C_END") >= 0)
    {
        firstcolormaplump = W_GetNumForName("C_START");
        numcolormaps = W_GetNumForName("C_END") - firstcolormaplump;

        colormaps = Z_Malloc(sizeof(*colormaps) * numcolormaps, PU_STATIC, NULL);

        colormaps[0] = W_CacheLumpName("COLORMAP");

        for (int i = 1; i < numcolormaps; i++)
            colormaps[i] = W_CacheLumpNum(i + firstcolormaplump);
    }
    else
    {
        colormaps = Z_Malloc(sizeof(*colormaps), PU_STATIC, NULL);
        colormaps[0] = W_CacheLumpName("COLORMAP");
    }

    colormapwad = lumpinfo[W_CheckNumForName("COLORMAP")]->wadfile;
    C_Output("Using %s colormap%s from the <b>COLORMAP</b> lump in %s <b>%s</b>.",
        (numcolormaps == 1 ? "the" : commify(numcolormaps)), (numcolormaps == 1 ? "" : "s"),
        (colormapwad->type == IWAD ? "IWAD" : "PWAD"), colormapwad->path);

    // [BH] There's a typo in dcolors.c, the source code of the utility Id
    // Software used to construct the palettes and colormaps for DOOM (see
    // http://www.doomworld.com/idgames/?id=16644). When constructing colormap
    // 32, which is used for the invulnerability power-up, the traditional
    // Y luminance values are used (see http://en.wikipedia.org/wiki/YIQ), but a
    // value of 0.144 is used when it should be 0.114. So I've grabbed the
    // offending code from dcolor.c, corrected it, put it here, and now colormap
    // 32 is manually calculated rather than grabbing it from the colormap lump.
    // The resulting differences are minor.
    palsrc = palette = W_CacheLumpName("PLAYPAL");

    for (int i = 0; i < 255; i++)
    {
        double  red = *palsrc++ / 256.0;
        double  green = *palsrc++ / 256.0;
        double  blue = *palsrc++ / 256.0;
        double  gray = red * 0.299 + green * 0.587 + blue * 0.114/*0.144*/;
        int     color = (int)(gray * 255.0);

        grays[i] = FindNearestColor(palette, color, color, color);

        if (!COLORMAP)
        {
            color = (int)((1.0 - gray) * 255.0);
            colormaps[0][32 * 256 + i] = FindNearestColor(palette, color, color, color);
        }
    }
}

// killough 4/4/98: get colormap number from name
// killough 4/11/98: changed to return -1 for illegal names
int R_ColormapNumForName(char *name)
{
    int i = 0;

    if (numcolormaps == 1)
        return -1;

    if (strncasecmp(name, "COLORMAP", 8))     // COLORMAP predefined to return 0
        if ((i = W_CheckNumForName(name)) != -1)
            i -= firstcolormaplump;

    return i;
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
    int i = W_RangeCheckNumForName(firstflat, lastflat, name);

    if (i == -1)
    {
        C_Warning("The <b>%.8s</b> flat texture can't be found.", uppercase(name));
        return skyflatnum;
    }

    return (i - firstflat);
}

//
// R_CheckFlatNumForName
// Retrieval, get a flat number for a flat name. No error.
//
int R_CheckFlatNumForName(char *name)
{
    for (int i = firstflat; i <= lastflat; i++)
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
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
    int i = 0;

    if (*name != '-')
    {
        i = textures[W_LumpNameHash(name) % (unsigned int)numtextures]->index;

        while (i >= 0 && strncasecmp(textures[i]->name, name, 8))
            i = textures[i]->next;
    }

    return i;
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
    {
        C_Warning("The <b>%.8s</b> texture can't be found.", uppercase(name));
        return 0;
    }

    return i;
}

//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
// Totally rewritten by Lee Killough to use less memory,
// to avoid using alloca(), and to improve performance.
// cph - new wad lump handling, calls cache functions but acquires no locks

static inline void precache_lump(int l)
{
    W_CacheLumpNum(l);
    W_UnlockLumpNum(l);
}

void R_PrecacheLevel(void)
{
    byte    *hitlist = malloc(MAX(numtextures, MAX(numflats, NUMSPRITES)));

    if (!hitlist)
        return;

    // Precache flats.
    memset(hitlist, 0, numflats);

    for (int i = 0; i < numsectors; i++)
    {
        hitlist[sectors[i].floorpic] = 1;
        hitlist[sectors[i].ceilingpic] = 1;
    }

    for (int i = 0; i < numflats; i++)
        if (hitlist[i])
            precache_lump(firstflat + i);

    // Precache textures.
    memset(hitlist, 0, numtextures);

    for (int i = 0; i < numsides; i++)
    {
        hitlist[sides[i].toptexture] = 1;
        hitlist[sides[i].midtexture] = 1;
        hitlist[sides[i].bottomtexture] = 1;
    }

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependent
    //  name.
    hitlist[skytexture] = 1;

    for (int i = 0; i < numtextures; i++)
        if (hitlist[i])
        {
            texture_t   *texture = textures[i];

            for (int j = 0; j < texture->patchcount; j++)
                precache_lump(texture->patches[j].patch);
        }

    // Precache sprites.
    memset(hitlist, 0, NUMSPRITES);

    for (thinker_t *th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
        hitlist[((mobj_t *)th)->sprite] = 1;

    for (int i = 0; i < NUMSPRITES; i++)
        if (hitlist[i])
            for (int j = 0; j < sprites[i].numframes; j++)
            {
                short   *lump = sprites[i].spriteframes[j].lump;

                for (int k = 0; k < 8; k++)
                    precache_lump(firstspritelump + lump[k]);
            }

    free(hitlist);
}
