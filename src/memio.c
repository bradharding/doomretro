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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memio.h"
#include "z_zone.h"

typedef enum
{
    MODE_READ,
    MODE_WRITE
} memfile_mode_t;

struct _MEMFILE
{
    unsigned char   *buf;
    size_t          buflen;
    size_t          alloced;
    unsigned int    position;
    bool            readeof;
    bool            eof;
    memfile_mode_t  mode;
};

// Open a memory area for reading
MEMFILE *mem_fopen_read(void *buf, size_t buflen)
{
    MEMFILE *file = Z_Malloc(sizeof(MEMFILE), PU_STATIC, NULL);

    file->buf = (unsigned char *)buf;
    file->buflen = buflen;
    file->position = 0;
    file->readeof = false;
    file->eof = false;
    file->mode = MODE_READ;

    return file;
}

// Read bytes
size_t mem_fread(void *buf, size_t size, size_t nmemb, MEMFILE *stream)
{
    size_t  items = nmemb;

    if (stream->mode != MODE_READ)
        return -1;

    if (!size || !nmemb)
        return 0;

    // Trying to read more bytes than we have left?
    if (items * size > stream->buflen - stream->position)
    {
        if (stream->readeof)
            stream->eof = true;
        else
            stream->readeof = true;

        items = (stream->buflen - stream->position) / size;
    }

    // Copy bytes to buffer
    memcpy(buf, stream->buf + stream->position, items * size);

    // Update position
    stream->position += (unsigned int)(items * size);

    return items;
}

// Open a memory area for writing
MEMFILE *mem_fopen_write(void)
{
    MEMFILE *file = Z_Malloc(sizeof(MEMFILE), PU_STATIC, NULL);

    file->alloced = 1024;
    file->buf = Z_Malloc(file->alloced, PU_STATIC, NULL);
    file->buflen = 0;
    file->position = 0;
    file->readeof = false;
    file->eof = false;
    file->mode = MODE_WRITE;

    return file;
}

// Write bytes to stream
size_t mem_fwrite(const void *ptr, size_t size, size_t nmemb, MEMFILE *stream)
{
    size_t  bytes;

    if (stream->mode != MODE_WRITE)
        return -1;

    // More bytes than can fit in the buffer? If so, reallocate bigger.
    bytes = size * nmemb;

    while (stream->alloced - stream->position < bytes)
    {
        unsigned char   *newbuf = Z_Malloc(stream->alloced * 2, PU_STATIC, NULL);

        memcpy(newbuf, stream->buf, stream->alloced);
        Z_Free(stream->buf);
        stream->buf = newbuf;
        stream->alloced *= 2;
    }

    // Copy into buffer
    memcpy(stream->buf + stream->position, ptr, bytes);
    stream->position += (unsigned int)bytes;

    if (stream->position > stream->buflen)
        stream->buflen = stream->position;

    return nmemb;
}

char *mem_fgets(char *str, int count, MEMFILE *stream)
{
    int i;

    if (!str || count < 0)
        return NULL;

    for (i = 0; i < count - 1; i++)
    {
        byte    ch;

        if (mem_fread(&ch, 1, 1, stream) != 1)
        {
            if (mem_feof(stream))
                return NULL;

            break;
        }

        str[i] = ch;

        if (ch == '\0')
            return str;

        if (ch == '\n')
        {
            i++;
            break;
        }
    }

    str[i] = '\0';
    return str;
}

int mem_fgetc(MEMFILE *stream)
{
    byte    ch;

    if (mem_fread(&ch, 1, 1, stream) == 1)
        return (int)ch;

    return -1;  // EOF
}

void mem_get_buf(MEMFILE *stream, void **buf, size_t *buflen)
{
    *buf = stream->buf;
    *buflen = stream->buflen;
}

void mem_fclose(MEMFILE *stream)
{
    if (stream->mode == MODE_WRITE)
        Z_Free(stream->buf);

    Z_Free(stream);
}

long mem_ftell(MEMFILE *stream)
{
    return stream->position;
}

int mem_fseek(MEMFILE *stream, long position, mem_rel_t whence)
{
    unsigned int    newpos;

    switch (whence)
    {
        case MEM_SEEK_SET:
            newpos = (int)position;
            break;

        case MEM_SEEK_CUR:
            newpos = (int)(stream->position + position);
            break;

        case MEM_SEEK_END:
            newpos = (int)(stream->buflen + position);
            break;

        default:
            return -1;
    }

    if (newpos < stream->buflen)
    {
        stream->position = newpos;
        stream->readeof = false;
        stream->eof = false;
        return 0;
    }

    return -1;
}

bool mem_feof(MEMFILE *stream)
{
    return stream->eof;
}
