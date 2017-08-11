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

#if !defined(__R_PATCH_H__)
#define __R_PATCH_H__

typedef struct rpost_s
{
    int             topdelta;
    int             length;
} rpost_t;

typedef struct rcolumn_s
{
    int             numposts;
    rpost_t         *posts;
    unsigned char   *pixels;
} rcolumn_t;

typedef struct rpatch_s
{
    int             width;
    int             height;
    unsigned int    widthmask;

    int             leftoffset;
    int             topoffset;

    // this is the single malloc'ed/free'd array
    // for this patch
    unsigned char   *data;

    // these are pointers into the data array
    unsigned char   *pixels;
    rcolumn_t       *columns;
    rpost_t         *posts;

    unsigned int    locks;
} rpatch_t;

const rpatch_t *R_CachePatchNum(int id);
void R_UnlockPatchNum(int id);

const rpatch_t *R_CacheTextureCompositePatchNum(int id);
void R_UnlockTextureCompositePatchNum(int id);

const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnIndex);
const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnIndex);

void R_InitPatches(void);

#endif
