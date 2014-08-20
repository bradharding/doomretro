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

#ifndef __SHA1_H__
#define __SHA1_H__

#include "doomtype.h"

typedef struct sha1_context_s sha1_context_t;
typedef byte sha1_digest_t[20];

struct sha1_context_s
{
    uint32_t    h0, h1, h2, h3, h4;
    uint32_t    nblocks;
    byte        buf[64];
    int         count;
};

void SHA1_Init(sha1_context_t *context);
void SHA1_Update(sha1_context_t *context, byte *buf, size_t len);
void SHA1_Final(sha1_digest_t digest, sha1_context_t *context);
void SHA1_UpdateInt32(sha1_context_t *context, unsigned int val);
void SHA1_UpdateString(sha1_context_t *context, char *str);

#endif
