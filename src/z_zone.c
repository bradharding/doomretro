/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "i_system.h"
#include "z_zone.h"

// Minimum chunk size at which blocks are allocated
#define CHUNKSIZE   32

typedef struct memblock_s
{
    struct memblock_s   *next;
    struct memblock_s   *prev;
    size_t              size;
    void                **user;
    unsigned char       tag;
} memblock_t;

// size of block header
// cph - base on sizeof(memblock_t), which can be larger than CHUNKSIZE on 64-bit architectures
static const size_t headersize = ((sizeof(memblock_t) + CHUNKSIZE - 1) & ~(CHUNKSIZE - 1));

static memblock_t   *blockbytag[PU_MAX];

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
void *Z_Malloc(size_t size, unsigned char tag, void **user)
{
    memblock_t  *block = NULL;

    if (!size)
        return (user ? (*user = NULL) : NULL);          // malloc(0) returns NULL

    size = ((size + CHUNKSIZE - 1) & ~(CHUNKSIZE - 1)); // round to chunk size

    while (!(block = malloc(size + headersize)))
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

    block->tag = tag;
    block->user = user;
    block = (memblock_t *)((char *)block + headersize);

    if (user)           // if there is a user
        *user = block;  // set user to point to new block

    return block;
}

void *Z_Calloc(size_t size1, size_t size2, unsigned char tag, void **user)
{
    return ((size1 *= size2) ? memset(Z_Malloc(size1, tag, user), 0, size1) : NULL);
}

char *Z_StringDuplicate(const char *s, unsigned char tag, void **user)
{
    char    *d = Z_Malloc(strlen(s) + 1, tag, user);

    if (d)
        strcpy(d, s);

    return d;
}

void Z_Free(void *ptr)
{
    memblock_t  *block;

    if (!ptr)
        return;

    block = (memblock_t *)((char *)ptr - headersize);

    // Nullify user if one exists
    if (block->user)
        *block->user = NULL;

    if (block == block->next)
        blockbytag[block->tag] = NULL;
    else if (blockbytag[block->tag] == block)
        blockbytag[block->tag] = block->next;

    block->prev->next = block->next;
    block->next->prev = block->prev;

    free(block);
}

void Z_FreeTags(unsigned char lowtag, unsigned char hightag)
{
    for (; lowtag <= hightag; lowtag++)
    {
        memblock_t  *block = blockbytag[lowtag];
        memblock_t  *endblock;

        if (!block)
            continue;

        endblock = block->prev;

        while (true)
        {
            memblock_t  *next = block->next;

            Z_Free((char *)block + headersize);

            if (block == endblock)
                break;

            block = next;   // Advance to next block
        }
    }
}

void Z_ChangeTag(void *ptr, unsigned char tag)
{
    memblock_t  *block;

    if (!ptr)
        return;

    block = (memblock_t *)((char *)ptr - headersize);

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
