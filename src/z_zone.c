/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#include "z_zone.h"
#include "i_system.h"

// Tunables

// Alignment of zone memory (benefit may be negated by HEADER_SIZE, CHUNK_SIZE)
#define CACHE_ALIGN     32

// Minimum chunk size at which blocks are allocated
#define CHUNK_SIZE      32

// Minimum size a block must be to become part of a split
#define MIN_BLOCK_SPLIT 1024

// How much RAM to leave aside for other libraries
#define LEAVE_ASIDE     (128 * 1024)

// Amount to subtract when retrying failed attempts to allocate initial pool
#define RETRY_AMOUNT    (256 * 1024)

// signature for block header
#define ZONEID          0x931d4a11

// Number of mallocs & frees kept in history buffer (must be a power of 2)
#define ZONE_HISTORY    4

// End Tunables

typedef struct memblock
{
    struct memblock     *next;
    struct memblock     *prev;
    size_t              size;
    void                **user;
    int32_t             tag;
} memblock_t;

//
// size of block header
// cph - base on sizeof(memblock_t), which can be larger than CHUNK_SIZE on
// 64bit architectures
//
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

    while (!(block = (memblock_t *)malloc(size + HEADER_SIZE)))
    {
        if (!blockbytag[PU_CACHE])
            I_Error("Z_Malloc: Failure trying to allocate %lu bytes", (uint32_t)size);
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
            memblock_t *next = block->next;
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

void *Z_Realloc(void *ptr, size_t n, int32_t tag, void **user)
{
    void        *p = Z_Malloc(n, tag, user);

    if (ptr)
    {
        memblock_t      *block = (memblock_t *)((char *)ptr - HEADER_SIZE);

        if (p)
            memcpy(p, ptr, n <= block->size ? n : block->size);
        Z_Free(ptr);
        if (user)                                       // in case Z_Free nullified same user
            *user = p;
    }
    return p;
}

void *Z_Calloc(size_t n1, size_t n2, int32_t tag, void **user)
{
    return ((n1 *= n2) ? memset(Z_Malloc(n1, tag, user), 0, n1) : NULL);
}

char *Z_Strdup(const char *s, int32_t tag, void **user)
{
    return strcpy((char *)Z_Malloc(strlen(s) + 1, tag, user), s);
}
