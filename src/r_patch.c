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
#include "r_patch.h"
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
static rpatch_t         *patches = 0;
static rpatch_t         *texture_composites = 0;

extern int              numtextures;
extern texture_t        **textures;

void R_InitPatches(void)
{
    if (!patches)
        patches = calloc(numlumps, sizeof(rpatch_t));

    if (!texture_composites)
        texture_composites = calloc(numtextures, sizeof(rpatch_t));
}

static int getPatchIsNotTileable(const patch_t *patch)
{
    int                 x = 0;
    int                 lastColumnDelta = 0;
    const column_t      *column;
    int                 cornerCount = 0;
    int                 hasAHole = 0;

    for (x = 0; x < SHORT(patch->width); ++x)
    {
        int             numPosts = 0;

        column = (const column_t *)((const byte *)patch + LONG(patch->columnofs[x]));

        if (!x)
            lastColumnDelta = column->topdelta;
        else if (lastColumnDelta != column->topdelta)
            hasAHole = 1;

        while (column->topdelta != 0xFF)
        {
            // check to see if a corner pixel filled
            if (!x && !column->topdelta)
                cornerCount++;
            else if (!x && column->topdelta + column->length >= SHORT(patch->height))
                cornerCount++;
            else if (x == SHORT(patch->width) - 1 && !column->topdelta)
                cornerCount++;
            else if (x == SHORT(patch->width) - 1
                && column->topdelta + column->length >= SHORT(patch->height))
                cornerCount++;

            if (numPosts++)
                hasAHole = 1;
            column = (const column_t *)((const byte *)column + column->length + 4);
        }
    }

    if (cornerCount == 4)
        return 0;

    return hasAHole;
}

static int getIsSolidAtSpot(const column_t *column, int spot)
{
    if (!column)
        return 0;

    while (column->topdelta != 0xFF)
    {
        if (spot < column->topdelta)
            return 0;
        if (spot >= column->topdelta && spot <= column->topdelta + column->length)
            return 1;
        column = (const column_t *)((const byte *)column + 3 + column->length + 1);
    }

    return 0;
}

// Checks if the lump can be a Doom patch
static dboolean CheckIfPatch(int lump)
{
    int                 size;
    int                 width, height;
    const patch_t       *patch;
    dboolean             result;

    size = W_LumpLength(lump);

    // minimum length of a valid Doom patch
    if (size < 13)
        return false;

    patch = (const patch_t *)W_CacheLumpNum(lump, PU_STATIC);

    width = SHORT(patch->width);
    height = SHORT(patch->height);

    result = (height > 0 && height <= 16384 && width > 0 && width <= 16384 && width < size / 4);

    if (result)
    {
        // The dimensions seem like they might be valid for a patch, so
        // check the column directory for extra security. All columns 
        // must begin after the column directory, and none of them must
        // point past the end of the patch.
        int     x;

        for (x = 0; x < width; ++x)
        {
            unsigned int        ofs = LONG(patch->columnofs[x]);

            // Need one byte for an empty column (but there's patches that don't know that!)
            if (ofs < (unsigned int)width * 4 + 8 || ofs >= (unsigned int)size)
            {
                result = false;
                break;
            }
        }
    }

    W_ReleaseLumpNum(lump);
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

    oldPatch = (const patch_t*)W_CacheLumpNum(patchNum, PU_STATIC);

    patch = &patches[id];
    patch->width = SHORT(oldPatch->width);
    patch->widthmask = 0;
    patch->height = SHORT(oldPatch->height);
    patch->leftoffset = SHORT(oldPatch->leftoffset);
    patch->topoffset = SHORT(oldPatch->topoffset);
    patch->flags = 0;
    if (getPatchIsNotTileable(oldPatch))
        patch->flags |= PATCH_ISNOTTILEABLE;

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
    patch->data = (unsigned char *)Z_Malloc(dataSize, PU_CACHE, (void **)&patch->data);
    memset(patch->data, 0, dataSize);

    // set out pixel, column, and post pointers into our data array
    patch->pixels = patch->data;
    patch->columns = (rcolumn_t *)((unsigned char *)patch->pixels + pixelDataSize);
    patch->posts = (rpost_t *)((unsigned char *)patch->columns + columnsDataSize);

    // sanity check that we've got all the memory allocated we need
    assert((((byte*)patch->posts + numPostsTotal * sizeof(rpost_t))
        - (byte *)patch->data) == dataSize);

    memset(patch->pixels, 0xFF, (patch->width*patch->height));

    // fill in the pixels, posts, and columns
    numPostsUsedSoFar = 0;
    for (x = 0; x < patch->width; ++x)
    {
        int             top = -1;
        const column_t  *oldPrevColumn;
        const column_t  *oldNextColumn;

        oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));

        if (patch->flags & PATCH_ISNOTTILEABLE)
        {
            // non-tiling
            if (!x)
                oldPrevColumn = 0;
            else
                oldPrevColumn = (const column_t *)((const byte *)oldPatch
                    + LONG(oldPatch->columnofs[x - 1]));
            if (x == patch->width - 1)
                oldNextColumn = 0;
            else
                oldNextColumn = (const column_t *)((const byte *)oldPatch
                    + LONG(oldPatch->columnofs[x + 1]));
        }
        else
        {
            // tiling
            int prevColumnIndex = x - 1;
            int nextColumnIndex = x + 1;

            while (prevColumnIndex < 0)
                prevColumnIndex += patch->width;
            while (nextColumnIndex >= patch->width)
                nextColumnIndex -= patch->width;
            oldPrevColumn = (const column_t *)((const byte *)oldPatch
                + LONG(oldPatch->columnofs[prevColumnIndex]));
            oldNextColumn = (const column_t *)((const byte *)oldPatch
                + LONG(oldPatch->columnofs[nextColumnIndex]));
        }

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
                // e6y: marking of all patches with holes
                patch->flags |= PATCH_HASHOLES;

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
                //if (getIsSolidAtSpot(oldColumn, y)) continue;
                if (column->pixels[y] != 0xFF) continue;

                // this pixel is a hole

                // e6y: marking of all patches with holes
                patch->flags |= PATCH_HASHOLES;

                if (x && prevColumn->pixels[y - 1] != 0xFF)
                    column->pixels[y] = prevColumn->pixels[y];  // copy the color from the left
                else
                    column->pixels[y] = column->pixels[y - 1];  // copy the color from above
            }
        }
    }

    W_ReleaseLumpNum(patchNum);
    free(numPostsInColumn);
}

typedef struct
{
    unsigned short      patches;
    unsigned short      posts;
    unsigned short      posts_used;
}
count_t;

static void switchPosts(rpost_t *post1, rpost_t *post2)
{
    rpost_t     dummy;

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
        int     i;

        for (i = post; i < column->numPosts - 1; ++i)
        {
            rpost_t     *post1 = &column->posts[i];
            rpost_t     *post2 = &column->posts[i + 1];

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
    composite_patch->flags = 0;

    // work out how much memory we need to allocate for this patch's data
    pixelDataSize = (composite_patch->width * composite_patch->height + 4) & ~3;
    columnsDataSize = sizeof(rcolumn_t) * composite_patch->width;

    // count the number of posts in each column
    countsInColumn = (count_t *)calloc(sizeof(count_t), composite_patch->width);
    numPostsTotal = 0;

    for (i = 0; i < texture->patchcount; ++i)
    {
        texpatch = &texture->patches[i];
        patchNum = texpatch->patch;
        oldPatch = (const patch_t *)W_CacheLumpNum(patchNum, PU_STATIC);

        for (x = 0; x < SHORT(oldPatch->width); ++x)
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

        W_ReleaseLumpNum(patchNum);
    }

    postsDataSize = numPostsTotal * sizeof(rpost_t);

    // allocate our data chunk
    dataSize = pixelDataSize + columnsDataSize + postsDataSize;
    composite_patch->data = (unsigned char *)Z_Malloc(dataSize, PU_STATIC,
        (void **)&composite_patch->data);
    memset(composite_patch->data, 0, dataSize);

    // set out pixel, column, and post pointers into our data array
    composite_patch->pixels = composite_patch->data;
    composite_patch->columns = (rcolumn_t*)((unsigned char*)composite_patch->pixels
        + pixelDataSize);
    composite_patch->posts = (rpost_t*)((unsigned char*)composite_patch->columns
        + columnsDataSize);

    // sanity check that we've got all the memory allocated we need
    assert((((byte *)composite_patch->posts + numPostsTotal * sizeof(rpost_t))
        - (byte *)composite_patch->data) == dataSize);

    memset(composite_patch->pixels, 0xFF, (composite_patch->width * composite_patch->height));

    numPostsUsedSoFar = 0;

    for (x = 0; x < texture->width; ++x)
    {
        // setup the column's data
        composite_patch->columns[x].pixels = composite_patch->pixels
            + (x * composite_patch->height);
        composite_patch->columns[x].numPosts = countsInColumn[x].posts;
        composite_patch->columns[x].posts = composite_patch->posts + numPostsUsedSoFar;
        numPostsUsedSoFar += countsInColumn[x].posts;
    }

    // fill in the pixels, posts, and columns
    for (i = 0; i < texture->patchcount; ++i)
    {
        texpatch = &texture->patches[i];
        patchNum = texpatch->patch;
        oldPatch = (const patch_t *)W_CacheLumpNum(patchNum, PU_STATIC);

        for (x = 0; x < SHORT(oldPatch->width); ++x)
        {
            int                 top = -1;
            int                 tx = texpatch->originx + x;
            const column_t      *oldPrevColumn;
            const column_t      *oldNextColumn;

            if (tx < 0)
                continue;
            if (tx >= composite_patch->width)
                break;

            oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));

            {
                // tiling
                int     prevColumnIndex = x - 1;
                int     nextColumnIndex = x + 1;

                while (prevColumnIndex < 0)
                    prevColumnIndex += SHORT(oldPatch->width);
                while (nextColumnIndex >= SHORT(oldPatch->width))
                    nextColumnIndex -= SHORT(oldPatch->width);
                oldPrevColumn = (const column_t *)((const byte *)oldPatch
                    + LONG(oldPatch->columnofs[prevColumnIndex]));
                oldNextColumn = (const column_t *)((const byte *)oldPatch
                    + LONG(oldPatch->columnofs[nextColumnIndex]));
            }

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
                // the original renderer had several bugs which we reproduce here
                if (countsInColumn[tx].patches > 1)
                {
                    // when there are multiple patches, then we need to handle the
                    // column differently
                    if (!i)
                    {
                        // draw first patch at original position, it will be partly
                        // overdrawn below
                        for (y = 0; y < count; ++y)
                        {
                            int ty = oy + top + y;

                            if (ty < 0)
                                continue;
                            if (ty >= composite_patch->height)
                                break;
                            composite_patch->pixels[tx * composite_patch->height + ty]
                                = oldColumnPixelData[y];
                        }
                    }
                    // do the buggy clipping
                    if (oy + top < 0)
                    {
                        count += oy;
                        oy = 0;
                    }
                }
                else
                    oy = 0;     // with a single patch only negative y origins are wrong

                // set up the post's data
                post->topdelta = top + oy;
                post->length = count;
                if ((post->topdelta + post->length) > composite_patch->height)
                {
                    if (post->topdelta > composite_patch->height)
                        post->length = 0;
                    else
                        post->length = composite_patch->height - post->topdelta;
                }
                if (post->topdelta < 0)
                {
                    if ((post->topdelta + post->length) <= 0)
                        post->length = 0;
                    else
                        post->length -= post->topdelta;
                    post->topdelta = 0;
                }

                // fill in the post's pixels
                for (y = 0; y < count; ++y)
                {
                    int ty = oy + top + y;

                    if (ty < 0)
                        continue;
                    if (ty >= composite_patch->height)
                        break;
                    composite_patch->pixels[tx * composite_patch->height + ty]
                        = oldColumnPixelData[y];
                }

                oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
                countsInColumn[tx].posts_used++;
                assert(countsInColumn[tx].posts_used <= countsInColumn[tx].posts);
            }
        }

        W_ReleaseLumpNum(patchNum);
    }

    for (x = 0; x < texture->width; ++x)
    {
        rcolumn_t       *column;

        if (countsInColumn[x].patches <= 1)
            continue;

        // cleanup posts on multipatch columns
        column = &composite_patch->columns[x];

        i = 0;
        while (i < column->numPosts - 1)
        {
            rpost_t     *post1 = &column->posts[i];
            rpost_t     *post2 = &column->posts[i + 1];

            if (post2->topdelta - post1->topdelta < 0)
                switchPosts(post1, post2);

            if (post1->topdelta + post1->length >= post2->topdelta)
            {
                int     length = (post1->length + post2->length) - ((post1->topdelta
                    + post1->length) - post2->topdelta);

                if (post1->length < length)
                    post1->length = length;
                removePostFromColumn(column, i + 1);
                i = 0;
                continue;
            }
            i++;
        }
    }

    {
        const rcolumn_t *column;
        const rcolumn_t *prevColumn;

        // copy the patch image down and to the right where there are
        // holes to eliminate the black halo from bilinear filtering
        for (x = 0; x < composite_patch->width; ++x)
        {
            column = R_GetPatchColumnClamped(composite_patch, x);
            prevColumn = R_GetPatchColumnClamped(composite_patch, x - 1);

            if (column->pixels[0] == 0xFF)
            {
                // e6y: marking of all patches with holes
                composite_patch->flags |= PATCH_HASHOLES;

                // force the first pixel (which is a hole), to use
                // the color from the next solid spot in the column
                for (y = 0; y < composite_patch->height; ++y)
                {
                    if (column->pixels[y] != 0xFF)
                    {
                        column->pixels[0] = column->pixels[y];
                        break;
                    }
                }
            }

            // copy from above or to the left
            for (y = 1; y < composite_patch->height; ++y)
            {
                if (column->pixels[y] != 0xFF)
                    continue;

                // this pixel is a hole

                // e6y: marking of all patches with holes
                composite_patch->flags |= PATCH_HASHOLES;

                if (x && prevColumn->pixels[y - 1] != 0xFF)
                    column->pixels[y] = prevColumn->pixels[y];  // copy the color from the left
                else
                    column->pixels[y] = column->pixels[y - 1];  // copy the color from above
            }
        }
    }

    free(countsInColumn);
}

rpatch_t *R_CacheTextureCompositePatchNum(int id)
{
    if (!texture_composites)
        I_Error("R_CacheTextureCompositePatchNum: Composite patches not initialized");

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

rcolumn_t *R_GetPatchColumnWrapped(rpatch_t *patch, int columnIndex)
{
    while (columnIndex < 0)
        columnIndex += patch->width;
    columnIndex %= patch->width;
    return &patch->columns[columnIndex];
}

rcolumn_t *R_GetPatchColumnClamped(rpatch_t *patch, int columnIndex)
{
    return &patch->columns[BETWEEN(0, columnIndex, patch->width - 1)];
}

rcolumn_t *R_GetPatchColumn(rpatch_t *patch, int columnIndex)
{
    if (patch->flags & PATCH_ISNOTTILEABLE)
        return R_GetPatchColumnClamped(patch, columnIndex);
    else
        return R_GetPatchColumnWrapped(patch, columnIndex);
}