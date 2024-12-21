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

#pragma once

#include <stdbool.h>

typedef struct _MEMFILE MEMFILE;

typedef enum
{
    MEM_SEEK_SET,
    MEM_SEEK_CUR,
    MEM_SEEK_END
} mem_rel_t;

MEMFILE *mem_fopen_read(void *buf, size_t buflen);
size_t mem_fread(void *buf, size_t size, size_t nmemb, MEMFILE *stream);
MEMFILE *mem_fopen_write(void);
size_t mem_fwrite(const void *ptr, size_t size, size_t nmemb, MEMFILE *stream);
char *mem_fgets(char *str, int count, MEMFILE *stream);
int mem_fgetc(MEMFILE *stream);
void mem_get_buf(MEMFILE *stream, void **buf, size_t *buflen);
void mem_fclose(MEMFILE *stream);
long mem_ftell(MEMFILE *stream);
int mem_fseek(MEMFILE *stream, long position, mem_rel_t whence);
bool mem_feof(MEMFILE *stream);
