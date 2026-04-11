/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#include <string.h>

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
// and we compose textures from the TEXTURE1/2 lists of patches.
//

// Re-engineered patch support
static rpatch_t *patches;
static rpatch_t *texturecomposites;

static short    BIGDOOR1;
static short    BIGDOOR7;
static short    FIREBLU1;
static short    SKY1;
static short    STEP2;
static short    TEKWALL1;

// Checks if the lump can be a DOOM patch
bool R_CheckIfPatch(const int lump)
{
    const int   size = W_LumpLength(lump);
    bool        result = false;

    if (size >= 13)
    {
        const patch_t       *patch = W_CacheLumpNum(lump);
        const unsigned char *magic = (const unsigned char *)patch;

        if (magic[0] != 0x89 || magic[1] != 'P' || magic[2] != 'N' || magic[3] != 'G')
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
    const int   defaultpatchnum = W_GetNumForName("TNT1A0");
    int         sourcepatchnum = patchnum;
    rpatch_t    *patch = &patches[patchnum];
    bool        usedefault = false;

    if (!R_CheckIfPatch(sourcepatchnum))
    {
        sourcepatchnum = defaultpatchnum;
        usedefault = true;
    }

    memset(patch, 0, sizeof(*patch));

    while (true)
    {
        const patch_t   *oldpatch = W_CacheLumpNum(sourcepatchnum);
        const column_t  *oldcolumn = NULL;
        const byte      *oldpatchdata = (const byte *)oldpatch;
        const size_t    oldpatchsize = (size_t)W_LumpLength(sourcepatchnum);
        const byte      *oldpatchend = oldpatchdata + oldpatchsize;
        int             pixeldatasize;
        int             columnsdatasize;
        int             postsdatasize;
        int             datasize;
        int             *numpostsincolumn = NULL;
        int             numpoststotal = 0;
        bool            badpatch = false;

        patch->width = SHORT(oldpatch->width);
        patch->widthmask = 0;
        patch->height = SHORT(oldpatch->height);
        patch->leftoffset = SHORT(oldpatch->leftoffset);
        patch->topoffset = SHORT(oldpatch->topoffset);

        // count the number of posts in each column
        if (patch->width <= 0 || !(numpostsincolumn = malloc(patch->width * sizeof(int))))
        {
            W_ReleaseLumpNum(sourcepatchnum);
            memset(patch, 0, sizeof(*patch));
            C_Warning(1, "The " BOLD("%.8s") " patch couldn't be created.", lumpinfo[patchnum]->name);
            return;
        }

        // work out how much memory we need to allocate for this patch's data
        pixeldatasize = ((patch->width * patch->height + 4) & ~3);
        columnsdatasize = patch->width * sizeof(rcolumn_t);

        for (int x = 0; x < patch->width && !badpatch; x++)
        {
            const unsigned int  columnoffset = LONG(oldpatch->columnoffset[x]);

            if (columnoffset >= oldpatchsize)
            {
                badpatch = true;
                break;
            }

            oldcolumn = (const column_t *)(oldpatchdata + columnoffset);
            numpostsincolumn[x] = 0;

            while ((const byte *)oldcolumn < oldpatchend && oldcolumn->topdelta != 0xFF)
            {
                const byte  *oldcolumnbytes = (const byte *)oldcolumn;
                const size_t oldcolumnsize = (size_t)oldcolumn->length + 4;

                if ((size_t)(oldpatchend - oldcolumnbytes) < oldcolumnsize)
                {
                    badpatch = true;
                    break;
                }

                numpostsincolumn[x]++;
                numpoststotal++;
                oldcolumn = (const column_t *)(oldcolumnbytes + oldcolumnsize);
            }

            if ((const byte *)oldcolumn >= oldpatchend)
                badpatch = true;
        }

        if (badpatch)
        {
            W_ReleaseLumpNum(sourcepatchnum);
            free(numpostsincolumn);

            if (!usedefault && sourcepatchnum != defaultpatchnum)
            {
                sourcepatchnum = defaultpatchnum;
                usedefault = true;
                memset(patch, 0, sizeof(*patch));
                continue;
            }

            memset(patch, 0, sizeof(*patch));
            C_Warning(1, "The " BOLD("%.8s") " patch couldn't be created.", lumpinfo[patchnum]->name);
            return;
        }

        postsdatasize = numpoststotal * sizeof(rpost_t);

        // allocate our data chunk
        datasize = pixeldatasize + columnsdatasize + postsdatasize;
        patch->data = Z_Calloc(1, datasize, PU_CACHE, (void **)&patch->data);

        if (!patch->data)
        {
            W_ReleaseLumpNum(sourcepatchnum);
            free(numpostsincolumn);

            if (!usedefault && sourcepatchnum != defaultpatchnum)
            {
                sourcepatchnum = defaultpatchnum;
                usedefault = true;
                memset(patch, 0, sizeof(*patch));
                continue;
            }

            memset(patch, 0, sizeof(*patch));
            C_Warning(1, "The " BOLD("%.8s") " patch couldn't be created.", lumpinfo[patchnum]->name);
            return;
        }

        // set out pixel, column, and post pointers into our data array
        patch->pixels = patch->data;
        patch->columns = (rcolumn_t *)((unsigned char *)patch->pixels + pixeldatasize);
        patch->posts = (rpost_t *)((unsigned char *)patch->columns + columnsdatasize);

        // sanity check that we've got all the memory allocated we need
        SDL_assert((((byte *)patch->posts + numpoststotal * sizeof(rpost_t)) - (byte *)patch->data) == datasize);

        // fill in the pixels, posts, and columns
        for (int x = 0, numpostsusedsofar = 0; x < patch->width && !badpatch; x++)
        {
            int                 top = -1;
            const unsigned int  columnoffset = LONG(oldpatch->columnoffset[x]);

            if (columnoffset >= oldpatchsize)
            {
                badpatch = true;
                break;
            }

            oldcolumn = (const column_t *)(oldpatchdata + columnoffset);

            // setup the column's data
            patch->columns[x].pixels = &patch->pixels[x * patch->height];
            patch->columns[x].numposts = numpostsincolumn[x];
            patch->columns[x].posts = patch->posts + numpostsusedsofar;

            while ((const byte *)oldcolumn < oldpatchend && oldcolumn->topdelta != 0xFF)
            {
                const byte          *oldcolumnbytes = (const byte *)oldcolumn;
                const size_t        oldcolumnsize = (size_t)oldcolumn->length + 4;
                const unsigned char *oldcolumnpixeldata;
                int                 len = oldcolumn->length;

                if ((size_t)(oldpatchend - oldcolumnbytes) < oldcolumnsize || numpostsusedsofar >= numpoststotal)
                {
                    badpatch = true;
                    break;
                }

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
                    oldcolumnpixeldata = oldcolumnbytes + 3;

                    for (int y = 0; y < len; y++)
                        patch->pixels[x * patch->height + top + y] = oldcolumnpixeldata[y];
                }

                oldcolumn = (const column_t *)(oldcolumnbytes + oldcolumnsize);
                numpostsusedsofar++;
            }

            if ((const byte *)oldcolumn >= oldpatchend || numpostsusedsofar != patch->columns[x].numposts + (int)(patch->columns[x].posts - patch->posts))
                badpatch = true;
        }

        if (badpatch)
        {
            if (patch->data)
                Z_Free(patch->data);

            W_ReleaseLumpNum(sourcepatchnum);
            free(numpostsincolumn);

            if (!usedefault && sourcepatchnum != defaultpatchnum)
            {
                sourcepatchnum = defaultpatchnum;
                usedefault = true;
                memset(patch, 0, sizeof(*patch));
                continue;
            }

            memset(patch, 0, sizeof(*patch));
            C_Warning(1, "The " BOLD("%.8s") " patch couldn't be created.", lumpinfo[patchnum]->name);
            return;
        }

        W_ReleaseLumpNum(sourcepatchnum);
        free(numpostsincolumn);
        return;
    }
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
            rpost_t         *post1 = &column->posts[i];
            const rpost_t   *post2 = &column->posts[i + 1];

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
    const int           defaultpatchnum = W_GetNumForName("TNT1A0");
    int                 *patchsources;
    int                 patchnum;
    const patch_t       *oldpatch;
    const column_t      *oldcolumn;
    int                 count;
    int                 pixeldatasize;
    int                 columnsdatasize;
    int                 postsdatasize;
    int                 datasize;
    int                 numpoststotal = 0;
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
    patchsources = malloc(texture->patchcount * sizeof(*patchsources));

    if (!countsincolumn || !patchsources)
    {
        free(countsincolumn);
        free(patchsources);
        memset(compositepatch, 0, sizeof(*compositepatch));
        return;
    }

    for (int i = 0; i < texture->patchcount; i++)
    {
        bool    usedefault = false;

        texpatch = &texture->patches[i];
        patchnum = texpatch->patch;
        patchsources[i] = -1;

        while (true)
        {
            const byte  *oldpatchdata;
            const byte  *oldpatchend;
            size_t      oldpatchsize;
            bool        badpatch = false;

            if (!R_CheckIfPatch(patchnum))
            {
                patchnum = defaultpatchnum;
                usedefault = true;
            }

            oldpatch = (const patch_t *)W_CacheLumpNum(patchnum);
            oldpatchdata = (const byte *)oldpatch;
            oldpatchsize = (size_t)W_LumpLength(patchnum);
            oldpatchend = oldpatchdata + oldpatchsize;

            for (int x = 0; x < SHORT(oldpatch->width); x++)
            {
                const int           tx = texpatch->originx + x;
                const unsigned int  columnoffset = LONG(oldpatch->columnoffset[x]);

                if (tx < 0)
                    continue;

                if (tx >= compositepatch->width)
                    break;

                if (columnoffset >= oldpatchsize)
                {
                    badpatch = true;
                    break;
                }

                countsincolumn[tx].patches++;
                oldcolumn = (const column_t *)(oldpatchdata + columnoffset);

                while ((const byte *)oldcolumn < oldpatchend && oldcolumn->topdelta != 0xFF)
                {
                    const byte      *oldcolumnbytes = (const byte *)oldcolumn;
                    const size_t    oldcolumnsize = (size_t)oldcolumn->length + 4;

                    if ((size_t)(oldpatchend - oldcolumnbytes) < oldcolumnsize)
                    {
                        badpatch = true;
                        break;
                    }

                    countsincolumn[tx].posts++;
                    numpoststotal++;
                    oldcolumn = (const column_t *)(oldcolumnbytes + oldcolumnsize);
                }

                if (badpatch || (const byte *)oldcolumn >= oldpatchend)
                {
                    badpatch = true;
                    break;
                }
            }

            W_ReleaseLumpNum(patchnum);

            if (!badpatch)
            {
                patchsources[i] = patchnum;
                break;
            }

            for (int x = 0; x < compositepatch->width; x++)
            {
                countsincolumn[x].patches = 0;
                countsincolumn[x].posts = 0;
            }

            numpoststotal = 0;

            for (int j = 0; j < i; j++)
            {
                const texpatch_t    *prevtexpatch = &texture->patches[j];
                const int           prevpatchnum = patchsources[j];
                const patch_t       *prevpatch;
                const byte          *prevpatchdata;
                const byte          *prevpatchend;
                size_t              prevpatchsize;

                if (prevpatchnum < 0)
                    continue;

                prevpatchsize = (size_t)W_LumpLength(prevpatchnum);
                prevpatch = (const patch_t *)W_CacheLumpNum(prevpatchnum);
                prevpatchdata = (const byte *)prevpatch;
                prevpatchend = prevpatchdata + prevpatchsize;

                for (int x = 0; x < SHORT(prevpatch->width); x++)
                {
                    const int           tx = prevtexpatch->originx + x;
                    const unsigned int  columnoffset = LONG(prevpatch->columnoffset[x]);

                    if (tx < 0)
                        continue;

                    if (tx >= compositepatch->width)
                        break;

                    if (columnoffset >= prevpatchsize)
                        break;

                    countsincolumn[tx].patches++;
                    oldcolumn = (const column_t *)(prevpatchdata + columnoffset);

                    while ((const byte *)oldcolumn < prevpatchend && oldcolumn->topdelta != 0xFF)
                    {
                        const byte  *oldcolumnbytes = (const byte *)oldcolumn;
                        const size_t oldcolumnsize = (size_t)oldcolumn->length + 4;

                        if ((size_t)(prevpatchend - oldcolumnbytes) < oldcolumnsize)
                            break;

                        countsincolumn[tx].posts++;
                        numpoststotal++;
                        oldcolumn = (const column_t *)(oldcolumnbytes + oldcolumnsize);
                    }
                }

                W_ReleaseLumpNum(prevpatchnum);
            }

            if (!usedefault && patchnum != defaultpatchnum)
            {
                patchnum = defaultpatchnum;
                usedefault = true;
                continue;
            }

            patchsources[i] = -1;
            break;
        }
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
    SDL_assert((((byte *)compositepatch->posts + numpoststotal * sizeof(rpost_t)) - (byte *)compositepatch->data) == datasize);

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
        patchnum = patchsources[i];

        if (patchnum < 0)
            continue;

        oldpatch = (const patch_t *)W_CacheLumpNum(patchnum);

        for (int x = 0; x < SHORT(oldpatch->width); x++)
        {
            int         top = -1;
            const int   tx = texpatch->originx + x;
            const byte  *oldpatchdata = (const byte *)oldpatch;
            const size_t oldpatchsize = (size_t)W_LumpLength(patchnum);
            const byte  *oldpatchend = oldpatchdata + oldpatchsize;
            const unsigned int columnoffset = LONG(oldpatch->columnoffset[x]);

            if (tx < 0)
                continue;

            if (tx >= compositepatch->width)
                break;

            if (columnoffset >= oldpatchsize)
                break;

            oldcolumn = (const column_t *)(oldpatchdata + columnoffset);

            while ((const byte *)oldcolumn < oldpatchend && oldcolumn->topdelta != 0xFF)
            {
                int     oy = texpatch->originy;
                const byte  *oldcolumnbytes = (const byte *)oldcolumn;
                const size_t oldcolumnsize = (size_t)oldcolumn->length + 4;
                rpost_t *post;

                if ((size_t)(oldpatchend - oldcolumnbytes) < oldcolumnsize
                    || countsincolumn[tx].postsused >= countsincolumn[tx].posts)
                    break;

                post = &compositepatch->columns[tx].posts[countsincolumn[tx].postsused];

                // e6y: support for DeePsea's true tall patches
                if (oldcolumn->topdelta <= top)
                    top += oldcolumn->topdelta;
                else
                    top = oldcolumn->topdelta;

                oldcolumnpixeldata = oldcolumnbytes + 3;
                count = oldcolumn->length;

                // [BH] use incorrect y-origin for certain textures
                if (id == BIGDOOR7 || id == FIREBLU1 || id == SKY1 || (id == STEP2 && modifiedgame) || id == TEKWALL1)
                    oy = 0;
                else if (id == BIGDOOR1 && gamemission == doom && !modifiedgame)
                    oy += 32;
                else if (countsincolumn[tx].patches > 1)
                {
                    if (!i)
                        for (int y = 0; y < count; y++)
                        {
                            const int   ty = oy + top + y;

                            if (ty < 0)
                                continue;

                            if (ty >= compositepatch->height)
                                break;

                            compositepatch->pixels[tx * compositepatch->height + ty] = oldcolumnpixeldata[y];
                        }

                    if (oy + top < 0)
                    {
                        count += oy;
                        oy = 0;
                    }
                }

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

                oldcolumn = (const column_t *)(oldcolumnbytes + oldcolumnsize);
                countsincolumn[tx].postsused++;
                SDL_assert(countsincolumn[tx].postsused <= countsincolumn[tx].posts);
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

    free(countsincolumn);
    free(patchsources);
}

void R_InitPatches(void)
{
    patches = calloc(numlumps, sizeof(rpatch_t));

    texturecomposites = calloc(numtextures, sizeof(rpatch_t));

    BIGDOOR1 = R_CheckTextureNumForName("BIGDOOR1");
    BIGDOOR7 = R_CheckTextureNumForName("BIGDOOR7");
    FIREBLU1 = R_CheckTextureNumForName("FIREBLU1");
    SKY1 = R_CheckTextureNumForName("SKY1");
    STEP2 = R_CheckTextureNumForName("STEP2");
    TEKWALL1 = R_CheckTextureNumForName("TEKWALL1");

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
