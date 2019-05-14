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

#include "c_console.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_local.h"
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

static int  missingflatnum;

int         firstspritelump;
int         lastspritelump;
int         numspritelumps;

dboolean    notranslucency = false;
dboolean    telefragonmap30 = false;

int         numtextures;
texture_t   **textures;

// needed for texture pegging
fixed_t     *textureheight;
byte        **brightmap;
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

static byte blueandgreen[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static byte brighttan[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0,
    1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static struct
{
    char    texture[9];
    int     game;
    byte    *mask;
} brightmaps[] = {
    { "COMP2",    DOOM1AND2, blueandgreen   }, { "COMPSTA1", DOOM1AND2, notgray        },
    { "COMPSTA2", DOOM1AND2, notgray        }, { "COMPUTE1", DOOM1AND2, notgrayorbrown },
    { "COMPUTE2", DOOM1AND2, notgrayorbrown }, { "COMPUTE3", DOOM1AND2, notgrayorbrown },
    { "EXITSIGN", DOOM1AND2, notgray        }, { "EXITSTON", DOOM1AND2, redonly        },
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
    { "SW2BRN1",  DOOM1AND2, greenonly2     }, { "SW2BRN2",  DOOM1AND2, greenonly1     },
    { "SW2BRNGN", DOOM1AND2, greenonly3     }, { "SW2COMM",  DOOM1AND2, greenonly1     },
    { "SW2COMP",  DOOM1AND2, redonly        }, { "SW2DIRT",  DOOM1AND2, greenonly2     },
    { "SW2EXIT",  DOOM1AND2, notgray        }, { "SW2GARG",  DOOM1AND2, brighttan      },
    { "SW2GRAY",  DOOM1AND2, notgray        }, { "SW2GRAY1", DOOM1AND2, notgray        },
    { "SW2GSTON", DOOM1AND2, redonly        }, { "SW2LION",  DOOM1AND2, brighttan      },
    { "SW2MARB",  DOOM2ONLY, redonly        }, { "SW2MET2",  DOOM1AND2, greenonly1     },
    { "SW2METAL", DOOM1AND2, greenonly3     }, { "SW2MOD1",  DOOM1AND2, greenonly1     },
    { "SW2PANEL", DOOM1AND2, redonly        }, { "SW2ROCK",  DOOM1AND2, redonly        },
    { "SW2SATYR", DOOM1AND2, brighttan      }, { "SW2SLAD",  DOOM1AND2, redonly        },
    { "SW2STARG", DOOM2ONLY, greenonly2     }, { "SW2STON1", DOOM1AND2, greenonly3     },
    { "SW2STON2", DOOM1ONLY, redonly        }, { "SW2STON2", DOOM2ONLY, greenonly2     },
    { "SW2STON6", DOOM1AND2, redonly        }, { "SW2STONE", DOOM1AND2, greenonly2     },
    { "SW2STRTN", DOOM1AND2, greenonly1     }, { "SW2TEK",   DOOM1AND2, greenonly1     },
    { "SW2VINE",  DOOM1AND2, greenonly1     }, { "SW2WOOD",  DOOM1AND2, redonly        },
    { "SW2ZIM",   DOOM1AND2, redonly        }, { "WOOD4",    DOOM1AND2, redonly        },
    { "WOODGARG", DOOM1AND2, redonly        }, { "WOODSKUL", DOOM1AND2, redonly        },
    { "ZELDOOR",  DOOM1AND2, redonly        }, { "",         0,         0              }
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
    const int           *maptex2 = NULL;
    char                name[9];
    int                 names_lump;     // cph - new wad lump handling
    const char          *names;         // cph -
    const char          *name_p;        // const*'s
    int                 *patchlookup;
    int                 nummappatches;
    int                 maxoff;
    int                 maxoff2 = 0;
    int                 numtextures1;
    int                 numtextures2 = 0;
    const int           *directory;

    // Load the patch names from pnames.lmp.
    name[8] = '\0';
    names = W_CacheLumpNum((names_lump = W_GetNumForName("PNAMES")));
    nummappatches = LONG(*((const int *)names));
    name_p = names + 4;
    patchlookup = malloc(nummappatches * sizeof(*patchlookup)); // killough

    for (i = 0; i < nummappatches; i++)
    {
        int p1;
        int p2;

        M_StringCopy(name, &name_p[i * 8], sizeof(name));
        p1 = p2 = W_CheckNumForName(name);

        // [crispy] prevent flat lumps from being mistaken as patches
        while (p2 >= firstflat && p2 <= lastflat)
            p2 = W_RangeCheckNumForName(0, p2 - 1, name);

        patchlookup[i] = (p2 != -1 ? p2 : p1);
    }

    W_ReleaseLumpNum(names_lump);                               // cph - release the lump

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

        texture = textures[i] = Z_Malloc(sizeof(texture_t) + sizeof(texpatch_t) * ((size_t)SHORT(mtexture->patchcount) - 1),
            PU_STATIC, 0);

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
                C_Warning("Patch %i is missing in the <b>%.8s</b> texture.", SHORT(mpatch->patch), uppercase(texture->name));
        }

        for (j = 1; j * 2 <= texture->width; j <<= 1);

        texture->widthmask = j - 1;
        textureheight[i] = texture->height << FRACBITS;
    }

    free(patchlookup);                                          // killough

    for (i = 0; i < 2; i++)                                     // cph - release the TEXTUREx lumps
        if (maptex_lump[i] != -1)
            W_ReleaseLumpNum(maptex_lump[i]);

    // Create translation table for global animation.
    // killough 4/9/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof
    texturetranslation = Z_Malloc(((size_t)numtextures + 1) * sizeof(*texturetranslation), PU_STATIC, NULL);

    for (i = 0; i < numtextures; i++)
        texturetranslation[i] = i;

    // killough 1/31/98: Initialize texture hash table
    for (i = 0; i < numtextures; i++)
        textures[i]->index = -1;

    while (--i >= 0)
    {
        j = W_LumpNameHash(textures[i]->name) % numtextures;

        textures[i]->next = textures[j]->index;                 // Prepend to chain
        textures[j]->index = i;
    }
}

//
// R_InitBrightmaps
//
static void R_InitBrightmaps(void)
{
    int i = 0;

    brightmap = Z_Calloc(numtextures, 256, PU_STATIC, NULL);
    nobrightmap = Z_Calloc(numtextures, sizeof(*nobrightmap), PU_STATIC, NULL);

    while (brightmaps[i].mask)
    {
        int game = brightmaps[i].game;

        if (*brightmaps[i].texture
            && (game == DOOM1AND2 || (gamemission == doom && game == DOOM1ONLY) || (gamemission != doom && game == DOOM2ONLY)))
        {
            int num = R_CheckTextureNumForName(brightmaps[i].texture);

            if (num != -1)
                brightmap[num] = brightmaps[i].mask;
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
    flattranslation = Z_Malloc(((size_t)numflats + 1) * sizeof(*flattranslation), PU_STATIC, NULL);

    for (int i = 0; i < numflats; i++)
        flattranslation[i] = firstflat + i;

    missingflatnum = R_FlatNumForName("-N0_TEX-");
}

//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
static void R_InitSpriteLumps(void)
{
    dboolean    fixspriteoffsets = false;

    SC_Open("DRCOMPAT");

    while (SC_GetString())
    {
        if (M_StringCompare(sc_String, "FIXSPRITEOFFSETS"))
        {
            char    *sc_String_free;

            SC_MustGetString();
            sc_String_free = removeext(sc_String);

            if (M_StringCompare(pwadfile, sc_String_free))
                fixspriteoffsets = true;

            free(sc_String_free);
        }
        else if (M_StringCompare(sc_String, "NOTRANSLUCENCY"))
        {
            char    *sc_String_free;

            SC_MustGetString();
            sc_String_free = removeext(sc_String);

            if (M_StringCompare(pwadfile, sc_String_free))
                notranslucency = true;

            free(sc_String_free);
        }
        else if (M_StringCompare(sc_String, "TELEFRAGONMAP30"))
        {
            char    *sc_String_free;

            SC_MustGetString();
            sc_String_free = removeext(sc_String);

            if (M_StringCompare(pwadfile, sc_String_free))
                telefragonmap30 = true;

            free(sc_String_free);
        }
    }

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
                        && ((!BTSX && !sprfix18) || sproffsets[j].sprfix18)
                        && (BTSX || fixspriteoffsets || lumpinfo[firstspritelump + i]->wadfile->type != PWAD))
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
        states[S_BAR3].nextstate = S_BAR2;
        mobjinfo[MT_BARREL].frames = 2;
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
        mobjinfo[MT_INS].flags2 &= ~(MF2_TRANSLUCENT_33 | MF2_FLOATBOB);
        mobjinfo[MT_MISC14].flags2 &= ~MF2_FLOATBOB;
        mobjinfo[MT_BFG].flags2 &= ~MF2_TRANSLUCENT;
        mobjinfo[MT_HEAD].blood = MT_BLOOD;
        mobjinfo[MT_BRUISER].blood = MT_BLOOD;
        mobjinfo[MT_KNIGHT].blood = MT_BLOOD;
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
byte    grays[256];

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

        for (int i = 1; i < numcolormaps; i++)
            colormaps[i] = W_CacheLumpNum(i + firstcolormaplump);
    }
    else
        colormaps = Z_Malloc(sizeof(*colormaps), PU_STATIC, NULL);

    dc_colormap[1] = colormaps[0] = W_CacheLumpName("COLORMAP");

    colormapwad = lumpinfo[W_CheckNumForName("COLORMAP")]->wadfile;

    if (numcolormaps == 1)
        C_Output("Using the <b>COLORMAP</b> lump in %s <b>%s</b>.", (colormapwad->type == IWAD ? "IWAD" : "PWAD"), colormapwad->path);
    else
        C_Output("Using %s colormaps from the <b>COLORMAP</b> lump in %s <b>%s</b>.",
            commify(numcolormaps), (colormapwad->type == IWAD ? "IWAD" : "PWAD"), colormapwad->path);

    // [BH] There's a typo in dcolors.c, the source code of the utility id
    // Software used to construct the palettes and colormaps for DOOM (see
    // <https://www.doomworld.com/idgames/?id=16644>). When constructing colormap
    // 32, which is used for the invulnerability power-up, the traditional
    // Y luminance values are used (see <https://en.wikipedia.org/wiki/YIQ>),
    // but a value of 0.144 is used when it should be 0.114. So I've grabbed the
    // offending code from dcolor.c, corrected it, put it here, and now colormap
    // 32 is manually calculated rather than grabbing it from the colormap lump.
    // The resulting differences are minor.
    palsrc = palette = PLAYPAL;

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

    return (i > numcolormaps ? -1 : i);
}

//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData(void)
{
    R_InitFlats();
    R_InitTextures();
    R_InitBrightmaps();
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
        if (*name != '-')
            C_Warning("The <b>%.8s</b> flat texture can't be found.", uppercase(name));

        return missingflatnum;
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
        i = textures[W_LumpNameHash(name) % numtextures]->index;

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
        if (*name != '-')
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
void R_PrecacheLevel(void)
{
    dboolean    *hitlist = malloc(sizeof(dboolean) * MAX(numtextures, numflats));

    if (!hitlist)
        return;

    // Precache flats.
    memset(hitlist, false, numflats);

    for (int i = 0; i < numsectors; i++)
    {
        hitlist[sectors[i].floorpic] = true;
        hitlist[sectors[i].ceilingpic] = true;
    }

    for (int i = 0; i < numflats; i++)
        if (hitlist[i])
            W_CacheLumpNum(firstflat + i);

    // Precache textures.
    memset(hitlist, false, numtextures);

    for (int i = 0; i < numsides; i++)
    {
        hitlist[sides[i].toptexture] = true;
        hitlist[sides[i].midtexture] = true;
        hitlist[sides[i].bottomtexture] = true;
    }

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependent
    //  name.
    hitlist[skytexture] = true;

    for (int i = 0; i < numtextures; i++)
        if (hitlist[i])
        {
            texture_t   *texture = textures[i];

            for (int j = 0; j < texture->patchcount; j++)
                W_CacheLumpNum(texture->patches[j].patch);
        }

    free(hitlist);
}
