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

/*
**---------------------------------------------------------------------------
** Copyright 2004-2006 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
*/

#include "i_swap.h"
#include "i_system.h"
#include "r_main.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.
//

// Re-engineered patch support
static rpatch_t     *patches;
static rpatch_t     *texture_composites;

static short        BIGDOOR7;
static short        FIREBLU1;
static short        SKY1;

extern int          numtextures;
extern texture_t    **textures;

void R_InitPatches(void)
{
    if (!patches)
        patches = calloc(numlumps, sizeof(rpatch_t));

    if (!texture_composites)
        texture_composites = calloc(numtextures, sizeof(rpatch_t));

    BIGDOOR7 = R_CheckTextureNumForName("BIGDOOR7");
    FIREBLU1 = R_CheckTextureNumForName("FIREBLU1");
    SKY1 = R_CheckTextureNumForName("SKY1");
}

static dboolean getIsSolidAtSpot(const column_t *column, int spot)
{
    if (!column)
        return false;

    while (column->topdelta != 0xFF)
    {
        if (spot < column->topdelta)
            return false;

        if (spot >= column->topdelta && spot <= column->topdelta + column->length)
            return true;

        column = (const column_t*)((const byte*)column + 3 + column->length + 1);
    }

    return false;
}

// Checks if the lump can be a DOOM patch
static dboolean CheckIfPatch(int lump)
{
    int             size = W_LumpLength(lump);
    int             width;
    int             height;
    const patch_t   *patch;
    dboolean        result;

    // minimum length of a valid DOOM patch
    if (size < 13)
        return false;

    patch = W_CacheLumpNum(lump);

    width = SHORT(patch->width);
    height = SHORT(patch->height);

    result = (height > 0 && height <= 16384 && width > 0 && width <= 16384 && width < size / 4);

    if (result)
    {
        // The dimensions seem like they might be valid for a patch, so
        // check the column directory for extra security. All columns
        // must begin after the column directory, and none of them must
        // point past the end of the patch.
        int x;

        for (x = 0; x < width; ++x)
        {
            unsigned int    ofs = LONG(patch->columnofs[x]);

            // Need one byte for an empty column (but there's patches that don't know that!)
            if (ofs < (unsigned int)width * 4 + 8 || ofs >= (unsigned int)size)
            {
                result = false;
                break;
            }
        }
    }

    W_UnlockLumpNum(lump);
    return result;
}

static void createPatch(int id)
{
    rpatch_t            *patch;
    const int           patchNum = id;
    const patch_t       *oldPatch;
    const column_t      *oldColumn;
    int                 x, y;
    int                 pixelDataSize;
    int                 columnsDataSize;
    int                 postsDataSize;
    int                 dataSize;
    int                 *numPostsInColumn;
    int                 numPostsTotal;
    const unsigned char *oldColumnPixelData;
    int                 numPostsUsedSoFar;

    if (!CheckIfPatch(patchNum))
        I_Error("createPatch: Unknown patch format %s.",
            (patchNum < numlumps ? lumpinfo[patchNum]->name : NULL));

    oldPatch = (const patch_t *)W_CacheLumpNum(patchNum);

    patch = &patches[id];
    patch->width = SHORT(oldPatch->width);
    patch->widthmask = 0;
    patch->height = SHORT(oldPatch->height);
    patch->leftoffset = SHORT(oldPatch->leftoffset);
    patch->topoffset = SHORT(oldPatch->topoffset);

    // work out how much memory we need to allocate for this patch's data
    pixelDataSize = (patch->width * patch->height + 4) & ~3;
    columnsDataSize = sizeof(rcolumn_t) * patch->width;

    // count the number of posts in each column
    numPostsInColumn = malloc(sizeof(int) * patch->width);
    numPostsTotal = 0;

    for (x = 0; x < patch->width; ++x)
    {
        oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));
        numPostsInColumn[x] = 0;

        while (oldColumn->topdelta != 0xFF)
        {
            numPostsInColumn[x]++;
            numPostsTotal++;
            oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
        }
    }

    postsDataSize = numPostsTotal * sizeof(rpost_t);

    // allocate our data chunk
    dataSize = pixelDataSize + columnsDataSize + postsDataSize;
    patch->data = Z_Malloc(dataSize, PU_CACHE, (void **)&patch->data);
    memset(patch->data, 0, dataSize);

    // set out pixel, column, and post pointers into our data array
    patch->pixels = patch->data;
    patch->columns = (rcolumn_t *)((unsigned char *)patch->pixels + pixelDataSize);
    patch->posts = (rpost_t *)((unsigned char *)patch->columns + columnsDataSize);

    // sanity check that we've got all the memory allocated we need
    assert((((byte *)patch->posts + numPostsTotal * sizeof(rpost_t)) - (byte *)patch->data) == dataSize);

    memset(patch->pixels, 0xFF, patch->width * patch->height);

    // fill in the pixels, posts, and columns
    numPostsUsedSoFar = 0;

    for (x = 0; x < patch->width; ++x)
    {
        int top = -1;

        oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));

        // setup the column's data
        patch->columns[x].pixels = patch->pixels + x * patch->height;
        patch->columns[x].numPosts = numPostsInColumn[x];
        patch->columns[x].posts = patch->posts + numPostsUsedSoFar;

        while (oldColumn->topdelta != 0xFF)
        {
            int len = oldColumn->length;

            //e6y: support for DeePsea's true tall patches
            if (oldColumn->topdelta <= top)
                top += oldColumn->topdelta;
            else
                top = oldColumn->topdelta;

            // Clip posts that extend past the bottom
            if (top + oldColumn->length > patch->height)
                len = patch->height - top;

            if (len > 0)
            {
                // set up the post's data
                patch->posts[numPostsUsedSoFar].topdelta = top;
                patch->posts[numPostsUsedSoFar].length = len;

                // fill in the post's pixels
                oldColumnPixelData = (const byte *)oldColumn + 3;

                for (y = 0; y < len; y++)
                    patch->pixels[x * patch->height + top + y] = oldColumnPixelData[y];
            }

            oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
            numPostsUsedSoFar++;
        }
    }

    {
        const rcolumn_t *column;
        const rcolumn_t *prevColumn;

        // copy the patch image down and to the right where there are
        // holes to eliminate the black halo from bilinear filtering
        for (x = 0; x < patch->width; ++x)
        {
            column = R_GetPatchColumnClamped(patch, x);
            prevColumn = R_GetPatchColumnClamped(patch, x - 1);

            if (column->pixels[0] == 0xFF)
            {
                // force the first pixel (which is a hole), to use
                // the color from the next solid spot in the column
                for (y = 0; y < patch->height; y++)
                    if (column->pixels[y] != 0xFF)
                    {
                        column->pixels[0] = column->pixels[y];
                        break;
                    }
            }

            // copy from above or to the left
            for (y = 1; y < patch->height; ++y)
            {
                if (getIsSolidAtSpot(oldColumn, y))
                    continue;

                if (column->pixels[y] != 0xFF)
                    continue;

                // this pixel is a hole
                if (x && prevColumn->pixels[y - 1] != 0xFF)
                    column->pixels[y] = prevColumn->pixels[y];  // copy the color from the left
                else
                    column->pixels[y] = column->pixels[y - 1];  // copy the color from above
            }
        }

        // verify that the patch truly is non-rectangular since
        // this determines tiling later on
    }

    W_UnlockLumpNum(patchNum);
    free(numPostsInColumn);
}

typedef struct
{
    unsigned short  patches;
    unsigned short  posts;
    unsigned short  posts_used;
} count_t;

static void switchPosts(rpost_t *post1, rpost_t *post2)
{
    rpost_t dummy;

    dummy.topdelta = post1->topdelta;
    dummy.length = post1->length;
    post1->topdelta = post2->topdelta;
    post1->length = post2->length;
    post2->topdelta = dummy.topdelta;
    post2->length = dummy.length;
}

static void removePostFromColumn(rcolumn_t *column, int post)
{
    if (post < column->numPosts)
    {
        int i;

        for (i = post; i < column->numPosts - 1; i++)
        {
            rpost_t *post1 = &column->posts[i];
            rpost_t *post2 = &column->posts[i + 1];

            post1->topdelta = post2->topdelta;
            post1->length = post2->length;
        }
    }
    column->numPosts--;
}

static void createTextureCompositePatch(int id)
{
    rpatch_t            *composite_patch = &texture_composites[id];
    texture_t           *texture = textures[id];
    texpatch_t          *texpatch;
    int                 patchNum;
    const patch_t       *oldPatch;
    const column_t      *oldColumn;
    int                 i, x, y;
    int                 oy, count;
    int                 pixelDataSize;
    int                 columnsDataSize;
    int                 postsDataSize;
    int                 dataSize;
    int                 numPostsTotal;
    const unsigned char *oldColumnPixelData;
    int                 numPostsUsedSoFar;
    count_t             *countsInColumn;

    composite_patch->width = texture->width;
    composite_patch->height = texture->height;
    composite_patch->widthmask = texture->widthmask;
    composite_patch->leftoffset = 0;
    composite_patch->topoffset = 0;

    // work out how much memory we need to allocate for this patch's data
    pixelDataSize = (composite_patch->width * composite_patch->height + 4) & ~3;
    columnsDataSize = sizeof(rcolumn_t) * composite_patch->width;

    // count the number of posts in each column
    countsInColumn = (count_t *)calloc(sizeof(count_t), composite_patch->width);
    numPostsTotal = 0;

    for (i = 0; i < texture->patchcount; i++)
    {
        texpatch = &texture->patches[i];
        patchNum = texpatch->patch;
        oldPatch = (const patch_t *)W_CacheLumpNum(patchNum);

        for (x = 0; x < SHORT(oldPatch->width); x++)
        {
            int tx = texpatch->originx + x;

            if (tx < 0)
                continue;

            if (tx >= composite_patch->width)
                break;

            countsInColumn[tx].patches++;

            oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));

            while (oldColumn->topdelta != 0xFF)
            {
                countsInColumn[tx].posts++;
                numPostsTotal++;
                oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
            }
        }

        W_UnlockLumpNum(patchNum);
    }

    postsDataSize = numPostsTotal * sizeof(rpost_t);

    // allocate our data chunk
    dataSize = pixelDataSize + columnsDataSize + postsDataSize;
    composite_patch->data = Z_Calloc(1, dataSize, PU_STATIC, (void **)&composite_patch->data);

    // set out pixel, column, and post pointers into our data array
    composite_patch->pixels = composite_patch->data;
    composite_patch->columns = (rcolumn_t *)((unsigned char *)composite_patch->pixels + pixelDataSize);
    composite_patch->posts = (rpost_t *)((unsigned char *)composite_patch->columns + columnsDataSize);

    // sanity check that we've got all the memory allocated we need
    assert((((byte *)composite_patch->posts + numPostsTotal * sizeof(rpost_t))
        - (byte *)composite_patch->data) == dataSize);

    memset(composite_patch->pixels, 0xFF, composite_patch->width * composite_patch->height);

    numPostsUsedSoFar = 0;

    for (x = 0; x < texture->width; x++)
    {
        // setup the column's data
        composite_patch->columns[x].pixels = composite_patch->pixels + x * composite_patch->height;
        composite_patch->columns[x].numPosts = countsInColumn[x].posts;
        composite_patch->columns[x].posts = composite_patch->posts + numPostsUsedSoFar;
        numPostsUsedSoFar += countsInColumn[x].posts;
    }

    // fill in the pixels, posts, and columns
    for (i = 0; i < texture->patchcount; i++)
    {
        texpatch = &texture->patches[i];
        patchNum = texpatch->patch;
        oldPatch = (const patch_t *)W_CacheLumpNum(patchNum);

        for (x = 0; x < SHORT(oldPatch->width); x++)
        {
            int top = -1;
            int tx = texpatch->originx + x;

            if (tx < 0)
                continue;

            if (tx >= composite_patch->width)
                break;

            oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));

            while (oldColumn->topdelta != 0xFF)
            {
                rpost_t *post = &composite_patch->columns[tx].posts[countsInColumn[tx].posts_used];

                // e6y: support for DeePsea's true tall patches
                if (oldColumn->topdelta <= top)
                    top += oldColumn->topdelta;
                else
                    top = oldColumn->topdelta;

                oldColumnPixelData = (const byte *)oldColumn + 3;
                oy = texpatch->originy;
                count = oldColumn->length;

                // [BH] use incorrect y-origin for certain textures
                if (id == BIGDOOR7 || id == FIREBLU1 || id == SKY1)
                    oy = 0;

                // set up the post's data
                post->topdelta = top + oy;
                post->length = count;

                if (post->topdelta + post->length > composite_patch->height)
                {
                    if (post->topdelta > composite_patch->height)
                        post->length = 0;
                    else
                        post->length = composite_patch->height - post->topdelta;
                }

                if (post->topdelta < 0)
                {
                    if (post->topdelta + post->length <= 0)
                        post->length = 0;
                    else
                        post->length -= post->topdelta;

                    post->topdelta = 0;
                }

                // fill in the post's pixels
                for (y = 0; y < count; y++)
                {
                    int ty = oy + top + y;

                    if (ty < 0)
                        continue;

                    if (ty >= composite_patch->height)
                        break;

                    composite_patch->pixels[tx * composite_patch->height + ty] = oldColumnPixelData[y];
                }

                oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
                countsInColumn[tx].posts_used++;
                assert(countsInColumn[tx].posts_used <= countsInColumn[tx].posts);
            }
        }

        W_UnlockLumpNum(patchNum);
    }

    for (x = 0; x < texture->width; x++)
    {
        rcolumn_t   *column;

        if (countsInColumn[x].patches <= 1)
            continue;

        // cleanup posts on multipatch columns
        column = &composite_patch->columns[x];

        i = 0;

        while (i < column->numPosts - 1)
        {
            rpost_t *post1 = &column->posts[i];
            rpost_t *post2 = &column->posts[i + 1];

            if (post2->topdelta - post1->topdelta < 0)
                switchPosts(post1, post2);

            if (post1->topdelta + post1->length >= post2->topdelta)
            {
                int length = post1->length + post2->length - (post1->topdelta + post1->length
                        - post2->topdelta);

                if (post1->length < length)
                    post1->length = length;

                removePostFromColumn(column, i + 1);
                i = 0;
                continue;
            }

            i++;
        }
    }

    // copy the patch image down and to the right where there are
    // holes to eliminate the black halo from bilinear filtering
    for (x = 0; x < composite_patch->width; x++)
    {
        const rcolumn_t *column = R_GetPatchColumnClamped(composite_patch, x);
        const rcolumn_t *prevColumn = R_GetPatchColumnClamped(composite_patch, x - 1);

        if (column->pixels[0] == 0xFF)
        {
            // force the first pixel (which is a hole), to use
            // the color from the next solid spot in the column
            for (y = 0; y < composite_patch->height; y++)
                if (column->pixels[y] != 0xFF)
                {
                    column->pixels[0] = column->pixels[y];
                    break;
                }
        }

        // copy from above or to the left
        for (y = 1; y < composite_patch->height; y++)
        {
            if (column->pixels[y] != 0xFF)
                continue;

            // this pixel is a hole
            if (x && prevColumn->pixels[y - 1] != 0xFF)
                column->pixels[y] = prevColumn->pixels[y];      // copy the color from the left
            else
                column->pixels[y] = column->pixels[y - 1];      // copy the color from above
        }
    }

    free(countsInColumn);
}

const rpatch_t *R_CachePatchNum(int id)
{
    if (!patches[id].data)
        createPatch(id);

    if (!patches[id].locks)
        Z_ChangeTag(patches[id].data, PU_STATIC);

    patches[id].locks++;

    return &patches[id];
}

void R_UnlockPatchNum(int id)
{
    if (!--patches[id].locks)
        Z_ChangeTag(patches[id].data, PU_CACHE);
}

const rpatch_t *R_CacheTextureCompositePatchNum(int id)
{
    if (!texture_composites[id].data)
        createTextureCompositePatch(id);

    // cph - if wasn't locked but now is, tell z_zone to hold it
    if (!texture_composites[id].locks)
        Z_ChangeTag(texture_composites[id].data, PU_STATIC);

    texture_composites[id].locks++;

    return &texture_composites[id];
}

void R_UnlockTextureCompositePatchNum(int id)
{
    if (!--texture_composites[id].locks)
        Z_ChangeTag(texture_composites[id].data, PU_CACHE);
}

const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnIndex)
{
    while (columnIndex < 0)
        columnIndex += patch->width;

    return &patch->columns[columnIndex % patch->width];
}

const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnIndex)
{
    return &patch->columns[BETWEEN(0, columnIndex, patch->width - 1)];
}
