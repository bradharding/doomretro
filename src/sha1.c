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

#include "sha1.h"

#define SHA1ROTL(bits, word)    (((word) << (bits)) | ((word) >> (32 - (bits))))

static void SHA1Transform(uint32_t state[5], const byte buffer[64])
{
    uint32_t    a = state[0];
    uint32_t    b = state[1];
    uint32_t    c = state[2];
    uint32_t    d = state[3];
    uint32_t    e = state[4];
    uint32_t    words[80];

    for (int i = 0; i < 16; i++)
        words[i] = (((uint32_t)buffer[i * 4] << 24) | ((uint32_t)buffer[i * 4 + 1] << 16)
            | ((uint32_t)buffer[i * 4 + 2] << 8) | (uint32_t)buffer[i * 4 + 3]);

    for (int i = 16; i < 80; i++)
        words[i] = SHA1ROTL(1, words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16]);

    for (int i = 0; i < 80; i++)
    {
        uint32_t    f;
        uint32_t    k;
        uint32_t    temp;

        if (i < 20)
        {
            f = ((b & c) | (~b & d));
            k = 0x5A827999;
        }
        else if (i < 40)
        {
            f = (b ^ c ^ d);
            k = 0x6ED9EBA1;
        }
        else if (i < 60)
        {
            f = ((b & c) | (b & d) | (c & d));
            k = 0x8F1BBCDC;
        }
        else
        {
            f = (b ^ c ^ d);
            k = 0xCA62C1D6;
        }

        temp = SHA1ROTL(5, a) + f + e + k + words[i];
        e = d;
        d = c;
        c = SHA1ROTL(30, b);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

void SHA1Init(SHA1Context *context)
{
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count = 0;
}

void SHA1Update(SHA1Context *context, const byte *data, size_t length)
{
    size_t  index = (size_t)((context->count >> 3) & 63);

    context->count += (uint64_t)length << 3;

    if (index)
    {
        const size_t    partlen = 64 - index;

        if (length < partlen)
        {
            memcpy(context->buffer + index, data, length);
            return;
        }

        memcpy(context->buffer + index, data, partlen);
        SHA1Transform(context->state, context->buffer);
        data += partlen;
        length -= partlen;
    }

    while (length >= 64)
    {
        SHA1Transform(context->state, data);
        data += 64;
        length -= 64;
    }

    if (length)
        memcpy(context->buffer, data, length);
}

void SHA1Final(byte digest[SHA1_DIGEST_SIZE], SHA1Context *context)
{
    static const byte    padding[64] = { 0x80 };
    byte                 finalcount[8];

    for (int i = 0; i < 8; i++)
        finalcount[i] = (byte)(context->count >> ((7 - i) * 8));

    SHA1Update(context, padding, (((context->count >> 3) & 63) < 56 ?
        56 - ((context->count >> 3) & 63) : 120 - ((context->count >> 3) & 63)));
    SHA1Update(context, finalcount, sizeof(finalcount));

    for (int i = 0; i < SHA1_DIGEST_SIZE; i++)
        digest[i] = (byte)(context->state[i / 4] >> ((3 - (i & 3)) * 8));

    memset(context, 0, sizeof(*context));
}
