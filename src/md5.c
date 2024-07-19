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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "m_misc.h"
#include "md5.h"

#ifdef WORDS_BIGENDIAN
void byteswap(uint32_t *buf, unsigned int words)
{
    byte    *p = (byte *)buf;

    do
    {
        *buf++ = (uint32_t)((unsigned int)p[3] << 8 | p[2]) << 16 | ((unsigned int)p[1] << 8 | p[0]);
        p += 4;
    } while (--words);
}
#else
#define byteswap(buf, words)
#endif

// Start MD5 accumulation. Set bit count to 0 and buffer to mysterious
// initialization constants.
void MD5Init(MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xEFCDAB89;
    ctx->buf[2] = 0x98BADCFE;
    ctx->buf[3] = 0x10325476;

    ctx->bytes[0] = 0;
    ctx->bytes[1] = 0;
}

 // Update context to reflect the concatenation of another buffer full
 // of bytes.
void MD5Update(MD5Context *ctx, const byte *buf, unsigned int len)
{
    uint32_t    t = ctx->bytes[0];

    // Update byte count
    if ((ctx->bytes[0] = t + len) < t)
        ctx->bytes[1]++;    // Carry from low to high

    t = 64 - (t & 0x3F);    // Space available in ctx->in (at least 1)

    if (t > len)
    {
        memcpy((byte *)ctx->in + 64 - t, buf, len);
        return;
    }

    // First chunk is an odd size
    memcpy((byte *)ctx->in + 64 - t, buf, t);
    byteswap(ctx->in, 16);
    MD5Transform(ctx->buf, ctx->in);
    buf += t;
    len -= t;

    // Process data in 64-byte chunks
    while (len >= 64)
    {
        memcpy(ctx->in, buf, 64);
        byteswap(ctx->in, 16);
        MD5Transform(ctx->buf, ctx->in);
        buf += 64;
        len -= 64;
    }

    // Handle any remaining bytes of data.
    memcpy(ctx->in, buf, len);
}

// Final wrap-up - pad to 64-byte boundary with the bit pattern
// 1 0* (64-bit count of bits processed, MSB-first)
void MD5Final(byte digest[16], MD5Context *ctx)
{
    int     count = ctx->bytes[0] & 0x3f;   // Number of bytes in ctx->in
    byte    *p = (byte *)ctx->in + count;

    // Set the first char of padding to 0x80. There is always room.
    *p++ = 0x80;

    // Bytes of padding needed to make 56 bytes (-8..55)
    count = 56 - 1 - count;

    if (count < 0)
    {
        // Padding forces an extra block
        memset(p, 0, count + 8);
        byteswap(ctx->in, 16);
        MD5Transform(ctx->buf, ctx->in);
        p = (byte *)ctx->in;
        count = 56;
    }

    memset(p, 0, count);
    byteswap(ctx->in, 14);

    // Append length in bits and transform
    ctx->in[14] = ctx->bytes[0] << 3;
    ctx->in[15] = (ctx->bytes[1] << 3 | ctx->bytes[0] >> 29);
    MD5Transform(ctx->buf, ctx->in);

    byteswap(ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset(ctx, 0, sizeof(*ctx));   // In case it's sensitive
}

#ifndef ASM_MD5

// The four core functions - F1 is optimized somewhat
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, in, s)   (w += f(x, y, z) + in, w = (w << s | w >> (32 - s)) + x)

// The core of the MD5 algorithm, this alters an existing MD5 hash to
// reflect the addition of 16 longwords of new data. MD5Update blocks
// the data and converts bytes into longwords for this routine.
void MD5Transform(uint32_t buf[4], const uint32_t in[16])
{
    uint32_t    a = buf[0];
    uint32_t    b = buf[1];
    uint32_t    c = buf[2];
    uint32_t    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xD76AA478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xE8C7B756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070DB, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xC1BDCEEE, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xF57C0FAF, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787C62A, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xA8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xFD469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098D8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8B44F7AF, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xFFFF5BB1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895CD7BE, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6B901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xFD987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xA679438E, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49B40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xF61E2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xC040B340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265E5A51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xE9B6C7AA, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xD62F105D, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xD8A1E681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xE7D3FBC8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21E1CDE6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xC33707D6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xF4D50D87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455A14ED, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xA9E3E905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xFCEFA3F8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676F02D9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8D2A4C8A, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xFFFA3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771F681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6D9D6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xFDE5380C, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xA4BEEA44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4BDECFA9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xF6BB4B60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xBEBFBC70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289B7EC6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xEAA127FA, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xD4EF3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881D05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xD9D4D039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xE6DB99E5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1FA27CF8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xC4AC5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xF4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432AFF97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xAB9423A7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xFC93A039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655B59C3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8F0CCC92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xFFEFF47D, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845DD1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6FA87E4F, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xFE2CE6E0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xA3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4E0811A1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xF7537E82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xBD3AF235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2AD7D2BB, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xEB86D391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}
#endif

char *MD5(const char *filename)
{
    char    checksum[33] = "";
    FILE    *file = fopen(filename, "rb");

    if (file)
    {
        MD5Context  md5;
        byte        buffer[8192];
        size_t      len;

        MD5Init(&md5);

        while ((len = fread(buffer, 1, sizeof(buffer), file)) > 0)
            MD5Update(&md5, buffer, (unsigned int)len);

        MD5Final(buffer, &md5);

        for (int i = 0; i < 16; i++)
            M_snprintf(checksum, sizeof(checksum), "%s%02x", checksum, buffer[i]);

        fclose(file);
    }

    return M_StringDuplicate(checksum);
}
