/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

  This file is a part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the license, or (at your
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
#include "i_swap.h"
#include "r_main.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists  of patches.
//

// Re-engineered patch support
static rpatch_t *patches;
static rpatch_t *texturecomposites;

static short    BIGDOOR7;
static short    FIREBLU1;
static short    SKY1;
static short    STEP2;

static bool IsSolidAtSpot(const column_t *column, const int spot)
{
    if (!column)
        return false;

    while (column->topdelta != 0xFF)
    {
        if (spot < column->topdelta)
            return false;

        if (spot <= column->topdelta + column->length)
            return true;

        column = (const column_t *)((const byte *)column + 3 + column->length + 1);
    }

    return false;
}

// Checks if the lump can be a DOOM patch
static bool CheckIfPatch(const int lump)
{
    const int   size = W_LumpLength(lump);
    bool        result = false;

    if (size >= 13)
    {
        const patch_t       *patch = W_CacheLumpNum(lump);
        const unsigned char *magic = (const unsigned char *)patch;

        if (magic[0] == 0x89 && magic[1] == 'P' && magic[2] == 'N' && magic[3] == 'G')
            C_Warning(1, "The " BOLD("%.8s") " patch is an unsupported PNG lump.", lumpinfo[lump]->name);
        else
        {
            const short width = SHORT(patch->width);
            const short height = SHORT(patch->height);

            if ((result = (width > 0 && width <= 16384 && width < size / 4 && height > 0 && height <= 16384)))
                // The dimensions seem like they might be valid for a patch, so
                // check the column directory for extra security. All columns
                // must begin after the column directory, and none of them must
                // point past the end of the patch.
                for (int x = 0; x < width; x++)
                {
                    const unsigned int  offset = LONG(patch->columnoffset[x]);

                    // Need one byte for an empty column (but there's patches that don't know that!)
                    if (offset < (unsigned int)width * 4 + 8 || offset >= (unsigned int)size)
                    {
                        result = false;

                        if (lumpinfo[lump]->size > 0)
                            C_Warning(1, "The " BOLD("%.8s") " patch is in an unknown format.", lumpinfo[lump]->name);

                        break;
                    }
                }
        }

        W_ReleaseLumpNum(lump);
    }

    return result;
}

static void CreatePatch(int patchnum)
{
    rpatch_t            *patch;
    const patch_t       *oldpatch;
    const column_t      *oldcolumn;
    int                 pixeldatasize;
    int                 columnsdatasize;
    int                 postsdatasize;
    int                 datasize;
    int                 *numpostsincolumn;
    int                 numpoststotal;
    const unsigned char *oldcolumnpixeldata;

    if (!CheckIfPatch(patchnum))
        patchnum = W_GetNumForName("TNT1A0");

    oldpatch = W_CacheLumpNum(patchnum);
    patch = &patches[patchnum];
    patch->width = SHORT(oldpatch->width);
    patch->widthmask = 0;
    patch->height = SHORT(oldpatch->height);
    patch->leftoffset = SHORT(oldpatch->leftoffset);
    patch->topoffset = SHORT(oldpatch->topoffset);

    // count the number of posts in each column
    if (patch->width <= 0 || !(numpostsincolumn = malloc(patch->width * sizeof(int))))
    {
        C_Warning(1, "The " BOLD("%.8s") " patch couldn't be created.", lumpinfo[patchnum]->name);
        return;
    }

    // work out how much memory we need to allocate for this patch's data
    pixeldatasize = ((patch->width * patch->height + 4) & ~3);
    columnsdatasize = patch->width * sizeof(rcolumn_t);

    numpoststotal = 0;

    for (int x = 0; x < patch->width; x++)
    {
        oldcolumn = (const column_t *)((const byte *)oldpatch + LONG(oldpatch->columnoffset[x]));
        numpostsincolumn[x] = 0;

        while (oldcolumn->topdelta != 0xFF)
        {
            numpostsincolumn[x]++;
            numpoststotal++;
            oldcolumn = (const column_t *)((const byte *)oldcolumn + oldcolumn->length + 4);
        }
    }

    postsdatasize = numpoststotal * sizeof(rpost_t);

    // allocate our data chunk
    datasize = pixeldatasize + columnsdatasize + postsdatasize;
    patch->data = Z_Calloc(1, datasize, PU_CACHE, (void **)&patch->data);

    // set out pixel, column, and post pointers into our data array
    patch->pixels = patch->data;
    patch->columns = (rcolumn_t *)((unsigned char *)patch->pixels + pixeldatasize);
    patch->posts = (rpost_t *)((unsigned char *)patch->columns + columnsdatasize);

    // sanity check that we've got all the memory allocated we need
    assert((((byte *)patch->posts + numpoststotal * sizeof(rpost_t)) - (byte *)patch->data) == datasize);

    memset(patch->pixels, 0xFF, (size_t)patch->width * patch->height);

    // fill in the pixels, posts, and columns
    for (int x = 0, numpostsusedsofar = 0; x < patch->width; x++)
    {
        int top = -1;

        oldcolumn = (const column_t *)((const byte *)oldpatch + LONG(oldpatch->columnoffset[x]));

        // setup the column's data
        patch->columns[x].pixels = &patch->pixels[x * patch->height];
        patch->columns[x].numposts = numpostsincolumn[x];
        patch->columns[x].posts = patch->posts + numpostsusedsofar;

        while (oldcolumn->topdelta != 0xFF)
        {
            int len = oldcolumn->length;

            // e6y: support for DeePsea's true tall patches
            if (oldcolumn->topdelta <= top)
                top += oldcolumn->topdelta;
            else
                top = oldcolumn->topdelta;

            // Clip posts that extend past the bottom
            if (top + oldcolumn->length > patch->height)
                len = patch->height - top;

            if (len > 0)
            {
                // set up the post's data
                patch->posts[numpostsusedsofar].topdelta = top;
                patch->posts[numpostsusedsofar].length = len;

                // fill in the post's pixels
                oldcolumnpixeldata = (const byte *)oldcolumn + 3;

                for (int y = 0; y < len; y++)
                    patch->pixels[x * patch->height + top + y] = oldcolumnpixeldata[y];
            }

            oldcolumn = (const column_t *)((const byte *)oldcolumn + oldcolumn->length + 4);
            numpostsusedsofar++;
        }
    }

    // copy the patch image down and to the right where there are
    // holes to eliminate the black halo from bilinear filtering
    for (int x = 0; x < patch->width; x++)
    {
        const rcolumn_t *column = R_GetPatchColumnClamped(patch, x);
        const rcolumn_t *prevcolumn = R_GetPatchColumnClamped(patch, x - 1);

        if (column->pixels[0] == 0xFF)
        {
            // force the first pixel (which is a hole), to use
            // the color from the next solid spot in the column
            for (int y = 0; y < patch->height; y++)
                if (column->pixels[y] != 0xFF)
                {
                    column->pixels[0] = column->pixels[y];
                    break;
                }
        }

        // copy from above or to the left
        for (int y = 1; y < patch->height; y++)
        {
            if (IsSolidAtSpot(oldcolumn, y))
                continue;

            if (column->pixels[y] != 0xFF)
                continue;

            // this pixel is a hole
            if (x && prevcolumn->pixels[y - 1] != 0xFF)
                column->pixels[y] = prevcolumn->pixels[y];  // copy the color from the left
            else
                column->pixels[y] = column->pixels[y - 1];  // copy the color from above
        }
    }

    W_ReleaseLumpNum(patchnum);
    free(numpostsincolumn);
}

typedef struct
{
    unsigned short  patches;
    unsigned short  posts;
    unsigned short  postsused;
} count_t;

static void SwitchPosts(rpost_t *post1, rpost_t *post2)
{
    rpost_t dummy = { 0 };

    dummy.topdelta = post1->topdelta;
    dummy.length = post1->length;
    post1->topdelta = post2->topdelta;
    post1->length = post2->length;
    post2->topdelta = dummy.topdelta;
    post2->length = dummy.length;
}

static void RemovePostFromColumn(rcolumn_t *column, int post)
{
    if (post < column->numposts)
        for (int i = post; i < column->numposts - 1; i++)
        {
            rpost_t *post1 = &column->posts[i];
            rpost_t *post2 = &column->posts[i + 1];

            post1->topdelta = post2->topdelta;
            post1->length = post2->length;
        }

    column->numposts--;
}

static void CreateTextureCompositePatch(const int id)
{
    rpatch_t            *compositepatch = &texturecomposites[id];
    const texture_t     *texture = textures[id];
    const texpatch_t    *texpatch;
    int                 patchnum;
    const patch_t       *oldpatch;
    const column_t      *oldcolumn;
    int                 count;
    int                 pixeldatasize;
    int                 columnsdatasize;
    int                 postsdatasize;
    int                 datasize;
    int                 numpoststotal;
    const unsigned char *oldcolumnpixeldata;
    count_t             *countsincolumn;

    compositepatch->width = texture->width;
    compositepatch->height = texture->height;
    compositepatch->widthmask = texture->widthmask;
    compositepatch->leftoffset = 0;
    compositepatch->topoffset = 0;

    // work out how much memory we need to allocate for this patch's data
    pixeldatasize = ((compositepatch->width * compositepatch->height + 4) & ~3);
    columnsdatasize = compositepatch->width * sizeof(rcolumn_t);

    // count the number of posts in each column
    countsincolumn = (count_t *)calloc(compositepatch->width, sizeof(count_t));
    numpoststotal = 0;

    for (int i = 0; i < texture->patchcount; i++)
    {
        texpatch = &texture->patches[i];
        patchnum = texpatch->patch;

        if (!CheckIfPatch(patchnum))
            patchnum = W_GetNumForName("TNT1A0");

        oldpatch = (const patch_t *)W_CacheLumpNum(patchnum);

        for (int x = 0; x < SHORT(oldpatch->width); x++)
        {
            int tx = texpatch->originx + x;

            if (tx < 0)
                continue;

            if (tx >= compositepatch->width)
                break;

            countsincolumn[tx].patches++;

            oldcolumn = (const column_t *)((const byte *)oldpatch + LONG(oldpatch->columnoffset[x]));

            while (oldcolumn->topdelta != 0xFF)
            {
                countsincolumn[tx].posts++;
                numpoststotal++;
                oldcolumn = (const column_t *)((const byte *)oldcolumn + oldcolumn->length + 4);
            }
        }

        W_ReleaseLumpNum(patchnum);
    }

    postsdatasize = numpoststotal * sizeof(rpost_t);

    // allocate our data chunk
    datasize = pixeldatasize + columnsdatasize + postsdatasize;
    compositepatch->data = Z_Calloc(1, datasize, PU_STATIC, (void **)&compositepatch->data);

    // set out pixel, column, and post pointers into our data array
    compositepatch->pixels = compositepatch->data;
    compositepatch->columns = (rcolumn_t *)((unsigned char *)compositepatch->pixels + pixeldatasize);
    compositepatch->posts = (rpost_t *)((unsigned char *)compositepatch->columns + columnsdatasize);

    // sanity check that we've got all the memory allocated we need
    assert((((byte *)compositepatch->posts + numpoststotal * sizeof(rpost_t)) - (byte *)compositepatch->data) == datasize);

    memset(compositepatch->pixels, 0xFF, (size_t)compositepatch->width * compositepatch->height);

    for (int x = 0, numpostsusedsofar = 0; x < texture->width; x++)
    {
        // setup the column's data
        compositepatch->columns[x].pixels = &compositepatch->pixels[x * compositepatch->height];
        compositepatch->columns[x].numposts = countsincolumn[x].posts;
        compositepatch->columns[x].posts = compositepatch->posts + numpostsusedsofar;
        numpostsusedsofar += countsincolumn[x].posts;
    }

    // fill in the pixels, posts, and columns
    for (int i = 0; i < texture->patchcount; i++)
    {
        texpatch = &texture->patches[i];
        patchnum = texpatch->patch;

        if (!CheckIfPatch(patchnum))
            patchnum = W_GetNumForName("TNT1A0");

        oldpatch = (const patch_t *)W_CacheLumpNum(patchnum);

        for (int x = 0; x < SHORT(oldpatch->width); x++)
        {
            int         top = -1;
            const int   tx = texpatch->originx + x;

            if (tx < 0)
                continue;

            if (tx >= compositepatch->width)
                break;

            oldcolumn = (const column_t *)((const byte *)oldpatch + LONG(oldpatch->columnoffset[x]));

            while (oldcolumn->topdelta != 0xFF)
            {
                int     oy = texpatch->originy;
                rpost_t *post = &compositepatch->columns[tx].posts[countsincolumn[tx].postsused];

                // e6y: support for DeePsea's true tall patches
                if (oldcolumn->topdelta <= top)
                    top += oldcolumn->topdelta;
                else
                    top = oldcolumn->topdelta;

                oldcolumnpixeldata = (const byte *)oldcolumn + 3;
                count = oldcolumn->length;

                // [BH] use incorrect y-origin for certain textures
                if (id == BIGDOOR7 || id == FIREBLU1 || id == SKY1 || (id == STEP2 && modifiedgame))
                    oy = 0;

                // set up the post's data
                post->topdelta = top + oy;
                post->length = count;

                if (post->topdelta + post->length > compositepatch->height)
                {
                    if (post->topdelta > compositepatch->height)
                        post->length = 0;
                    else
                        post->length = compositepatch->height - post->topdelta;
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
                for (int y = 0; y < count; y++)
                {
                    const int   ty = oy + top + y;

                    if (ty < 0)
                        continue;

                    if (ty >= compositepatch->height)
                        break;

                    compositepatch->pixels[tx * compositepatch->height + ty] = oldcolumnpixeldata[y];
                }

                oldcolumn = (const column_t *)((const byte *)oldcolumn + oldcolumn->length + 4);
                countsincolumn[tx].postsused++;
                assert(countsincolumn[tx].postsused <= countsincolumn[tx].posts);
            }
        }

        W_ReleaseLumpNum(patchnum);
    }

    for (int x = 0; x < texture->width; x++)
    {
        rcolumn_t   *column;
        int         i = 0;

        if (countsincolumn[x].patches <= 1)
            continue;

        // cleanup posts on multipatch columns
        column = &compositepatch->columns[x];

        while (i < column->numposts - 1)
        {
            rpost_t *post1 = &column->posts[i];
            rpost_t *post2 = &column->posts[i + 1];

            if (post2->topdelta - post1->topdelta < 0)
                SwitchPosts(post1, post2);

            if (post1->topdelta + post1->length >= post2->topdelta)
            {
                const int   length = post1->length + post2->length - (post1->topdelta + post1->length - post2->topdelta);

                if (post1->length < length)
                    post1->length = length;

                RemovePostFromColumn(column, i + 1);
                i = 0;

                continue;
            }

            i++;
        }
    }

    // copy the patch image down and to the right where there are
    // holes to eliminate the black halo from bilinear filtering
    for (int x = 0; x < compositepatch->width; x++)
    {
        const rcolumn_t *column = R_GetPatchColumnClamped(compositepatch, x);
        const rcolumn_t *prevcolumn = R_GetPatchColumnClamped(compositepatch, x - 1);

        if (column->pixels[0] == 0xFF)
        {
            // force the first pixel (which is a hole), to use
            // the color from the next solid spot in the column
            for (int y = 0; y < compositepatch->height; y++)
                if (column->pixels[y] != 0xFF)
                {
                    column->pixels[0] = column->pixels[y];
                    break;
                }
        }

        // copy from above or to the left
        for (int y = 1; y < compositepatch->height; y++)
        {
            if (column->pixels[y] != 0xFF)
                continue;

            // this pixel is a hole
            if (x && prevcolumn->pixels[y - 1] != 0xFF)
                column->pixels[y] = prevcolumn->pixels[y];  // copy the color from the left
            else
                column->pixels[y] = column->pixels[y - 1];  // copy the color from above
        }
    }

    free(countsincolumn);
}

void R_InitPatches(void)
{
    patches = calloc(numlumps, sizeof(rpatch_t));

    texturecomposites = calloc(numtextures, sizeof(rpatch_t));

    BIGDOOR7 = R_CheckTextureNumForName("BIGDOOR7");
    FIREBLU1 = R_CheckTextureNumForName("FIREBLU1");
    SKY1 = R_CheckTextureNumForName("SKY1");
    STEP2 = R_CheckTextureNumForName("STEP2");

    for (int i = 0; i < numspritelumps; i++)
        CreatePatch(firstspritelump + i);

    for (int i = 0; i < numtextures; i++)
        CreateTextureCompositePatch(i);
}

const rpatch_t *R_CachePatchNum(const int id)
{
    return &patches[id];
}

const rpatch_t *R_CacheTextureCompositePatchNum(const int id)
{
    return &texturecomposites[id];
}

const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnindex)
{
    while (columnindex < 0)
        columnindex += patch->width;

    return &patch->columns[columnindex % patch->width];
}

const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnindex)
{
    return &patch->columns[BETWEEN(0, columnindex, patch->width - 1)];
}
