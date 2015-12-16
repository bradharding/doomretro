/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

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

#include "i_system.h"
#include "z_zone.h"

// Minimum chunk size at which blocks are allocated
#define CHUNK_SIZE      32

typedef struct memblock
{
    struct memblock     *next;
    struct memblock     *prev;
    size_t              size;
    void                **user;
    unsigned char       tag;
} memblock_t;

// size of block header
// cph - base on sizeof(memblock_t), which can be larger than CHUNK_SIZE on
// 64bit architectures
static const size_t     HEADER_SIZE = (sizeof(memblock_t) + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1);

static memblock_t       *blockbytag[PU_MAX];

//
// Z_Malloc
// You can pass a NULL user if the tag is < PU_PURGELEVEL.
//
// cph - the algorithm here was a very simple first-fit round-robin
//  one - just keep looping around, freeing everything we can until
//  we get a large enough space
//
// This has been changed now; we still do the round-robin first-fit,
// but we only free the blocks we actually end up using; we don't
// free all the stuff we just pass on the way.
//
void *Z_Malloc(size_t size, int32_t tag, void **user)
{
    memblock_t  *block = NULL;

    if (!size)
        return (user ? (*user = NULL) : NULL);          // malloc(0) returns NULL

    size = (size + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1); // round to chunk size

    while (!(block = malloc(size + HEADER_SIZE)))
    {
        if (!blockbytag[PU_CACHE])
            I_Error("Z_Malloc: Failure trying to allocate %lu bytes", (unsigned long)size);
        Z_FreeTags(PU_CACHE, PU_CACHE);
    }

    if (!blockbytag[tag])
    {
        blockbytag[tag] = block;
        block->next = block->prev = block;
    }
    else
    {
        blockbytag[tag]->prev->next = block;
        block->prev = blockbytag[tag]->prev;
        block->next = blockbytag[tag];
        blockbytag[tag]->prev = block;
    }

    block->size = size;

    block->tag = tag;                                   // tag
    block->user = user;                                 // user
    block = (memblock_t *)((char *)block + HEADER_SIZE);
    if (user)                                           // if there is a user
        *user = block;                                  // set user to point to new block

    return block;
}

void *Z_Calloc(size_t n1, size_t n2, int32_t tag, void **user)
{
    return ((n1 *= n2) ? memset(Z_Malloc(n1, tag, user), 0, n1) : NULL);
}

void *Z_Realloc(void *ptr, size_t size)
{
    void        *newp = realloc(ptr, size);

    if (!newp && size)
        I_Error("Z_Realloc: Failure trying to reallocate %i bytes", size);
    else
        ptr = newp;

    return ptr;
}

void Z_Free(void *p)
{
    memblock_t  *block = (memblock_t *)((char *)p - HEADER_SIZE);

    if (!p)
        return;

    if (block->user)                                    // Nullify user if one exists
        *block->user = NULL;

    if (block == block->next)
        blockbytag[block->tag] = NULL;
    else if (blockbytag[block->tag] == block)
        blockbytag[block->tag] = block->next;
    block->prev->next = block->next;
    block->next->prev = block->prev;

    free(block);
}

void Z_FreeTags(int32_t lowtag, int32_t hightag)
{
    if (lowtag <= PU_FREE)
        lowtag = PU_FREE + 1;
    if (hightag > PU_CACHE)
        hightag = PU_CACHE;

    for (; lowtag <= hightag; ++lowtag)
    {
        memblock_t      *block;
        memblock_t      *end_block;

        block = blockbytag[lowtag];
        if (!block)
            continue;
        end_block = block->prev;
        while (1)
        {
            memblock_t  *next = block->next;

            Z_Free((char *)block + HEADER_SIZE);
            if (block == end_block)
                break;
            block = next;                               // Advance to next block
        }
    }
}

void Z_ChangeTag(void *ptr, int32_t tag)
{
    memblock_t  *block = (memblock_t *)((char *)ptr - HEADER_SIZE);

    // proff - added sanity check, this can happen when an empty lump is locked
    if (!ptr)
        return;

    // proff - do nothing if tag doesn't differ
    if (tag == block->tag)
        return;

    if (block == block->next)
        blockbytag[block->tag] = NULL;
    else if (blockbytag[block->tag] == block)
        blockbytag[block->tag] = block->next;
    block->prev->next = block->next;
    block->next->prev = block->prev;

    if (!blockbytag[tag])
    {
        blockbytag[tag] = block;
        block->next = block->prev = block;
    }
    else
    {
        blockbytag[tag]->prev->next = block;
        block->prev = blockbytag[tag]->prev;
        block->next = blockbytag[tag];
        blockbytag[tag]->prev = block;
    }

    block->tag = tag;
}

void Z_ChangeUser(void *ptr, void **user)
{
    memblock_t  *block;

    block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));

    block->user = user;
    *user = ptr;
}
