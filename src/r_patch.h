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

#if !defined(__R_PATCH__)
#define __R_PATCH__

//e6y
typedef enum
{
    PATCH_ISNOTTILEABLE = 0x00000001,
    PATCH_REPEAT        = 0x00000002,
    PATCH_HASHOLES      = 0x00000004
} rpatch_flag_t;

typedef struct
{
    int                 topdelta;
    int                 length;
} rpost_t;

typedef struct
{
    int                 numPosts;
    rpost_t             *posts;
    unsigned char       *pixels;
} rcolumn_t;

typedef struct
{
    int                 width;
    int                 height;
    unsigned            widthmask;

    int                 leftoffset;
    int                 topoffset;

    // this is the single malloc'ed/free'd array 
    // for this patch
    unsigned char       *data;

    // these are pointers into the data array
    unsigned char       *pixels;
    rcolumn_t           *columns;
    rpost_t             *posts;

    unsigned int        locks;
    unsigned int        flags;  //e6y
} rpatch_t;

rpatch_t *R_CacheTextureCompositePatchNum(int id);
void R_UnlockTextureCompositePatchNum(int id);

rcolumn_t *R_GetPatchColumnWrapped(rpatch_t *patch, int columnIndex);
rcolumn_t *R_GetPatchColumnClamped(rpatch_t *patch, int columnIndex);

// returns R_GetPatchColumnWrapped for square, non-holed textures
// and R_GetPatchColumnClamped otherwise
rcolumn_t *R_GetPatchColumn(rpatch_t *patch, int columnIndex);

void R_InitPatches(void);

#endif
