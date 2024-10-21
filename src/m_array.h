/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <stddef.h>
#include <stdlib.h>

#include "i_system.h"

#define M_ARRAY_INIT_CAPACITY   128

typedef struct
{
    int     capacity;
    int     size;
    char    buffer[];
} m_array_buffer_t;

inline static m_array_buffer_t *array_ptr(const void *v)
{
    return (m_array_buffer_t *)((char *)v - offsetof(m_array_buffer_t, buffer));
}

inline static int array_size(const void *v)
{
    return (v ? array_ptr(v)->size : 0);
}

inline static void array_clear(const void *v)
{
    if (v)
        array_ptr(v)->size = 0;
}

#define array_push(v, e)                                                    \
    {                                                                       \
        if (!(v))                                                           \
            (v) = M_ArrayGrow((v), sizeof(*(v)), M_ARRAY_INIT_CAPACITY);    \
        else if (array_ptr((v))->size == array_ptr((v))->capacity)          \
            (v) = M_ArrayGrow((v), sizeof(*(v)), array_ptr((v))->capacity); \
                                                                            \
        (v)[array_ptr((v))->size++] = (e);                                  \
    }

#define array_free(v)         \
    if (v)                    \
    {                         \
        free(array_ptr((v))); \
        (v) = NULL;           \
    }

#define array_foreach(ptr, v) \
    for (ptr = (v); ptr != &(v)[array_size((v))]; ptr++)

inline static void *M_ArrayGrow(void *v, size_t esize, int n)
{
    m_array_buffer_t    *p;

    if (v)
    {
        p = array_ptr(v);
        p = I_Realloc(p, sizeof(m_array_buffer_t) + ((size_t)p->capacity + n) * esize);
        p->capacity += n;
    }
    else
    {
        p = I_Malloc(sizeof(m_array_buffer_t) + n * esize);
        p->capacity = n;
        p->size = 0;
    }

    return p->buffer;
}
