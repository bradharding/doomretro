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

#include <stdlib.h>
#include <string.h>

#include "cJSON/cJSON.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "sha1.h"
#include "s_trakinfo.h"
#include "w_wad.h"
#include "z_zone.h"

typedef struct
{
    byte    digest[SHA1_DIGEST_SIZE];
    char    *midi;
    char    *remixed;
    float   volume;
    bool    hasvolume;
} trakinfo_t;

static trakinfo_t   *trakinfo;
static int          numtrakinfo;

void S_ClearTrakInfo(void)
{
    for (int i = 0; i < numtrakinfo; i++)
    {
        free(trakinfo[i].midi);
        free(trakinfo[i].remixed);
    }

    free(trakinfo);

    trakinfo = NULL;
    numtrakinfo = 0;
}

static char *S_StripTrakInfoComments(const char *data, const int length)
{
    char    *result = Z_Malloc((size_t)length + 1, PU_STATIC, NULL);
    int     j = 0;
    bool    instring = false;
    bool    escaped = false;

    for (int i = 0; i < length; i++)
    {
        const char  c = data[i];

        if (instring)
        {
            result[j++] = c;

            if (escaped)
                escaped = false;
            else if (c == '\\')
                escaped = true;
            else if (c == '"')
                instring = false;

            continue;
        }

        if (c == '"')
        {
            instring = true;
            result[j++] = c;
            continue;
        }

        if (c == '/' && i + 1 < length && data[i + 1] == '/')
        {
            while (i < length && data[i] != '\n')
                i++;

            if (i < length)
                result[j++] = data[i];

            continue;
        }

        result[j++] = c;
    }

    result[j] = '\0';
    return result;
}

static trakinfo_t *S_FindTrakInfo(const byte digest[SHA1_DIGEST_SIZE])
{
    for (int i = 0; i < numtrakinfo; i++)
        if (!memcmp(trakinfo[i].digest, digest, sizeof(trakinfo[i].digest)))
            return &trakinfo[i];

    return NULL;
}

static trakinfo_t *S_GetOrCreateTrakInfo(const byte digest[SHA1_DIGEST_SIZE])
{
    trakinfo_t   *entry = S_FindTrakInfo(digest);

    if (entry)
        return entry;

    trakinfo = I_Realloc(trakinfo, (numtrakinfo + 1) * sizeof(*trakinfo));
    entry = &trakinfo[numtrakinfo++];
    memset(entry, 0, sizeof(*entry));
    memcpy(entry->digest, digest, sizeof(entry->digest));
    entry->volume = 1.0f;

    return entry;
}

static void S_ParseTrakInfoLump(const int lumpnum)
{
    const int   lumplength = W_LumpLength(lumpnum);
    const char  *data = W_CacheLumpNum(lumpnum);
    char        *buffer = S_StripTrakInfoComments(data, lumplength);
    cJSON       *json;
    cJSON       *item;

    W_ReleaseLumpNum(lumpnum);

    json = cJSON_ParseWithLength(buffer, strlen(buffer));

    if (cJSON_IsObject(json))
        cJSON_ArrayForEach(item, json)
            if (cJSON_IsObject(item) && item->string)
            {
                byte        digest[SHA1_DIGEST_SIZE];
                trakinfo_t  *entry;
                cJSON       *midi;
                cJSON       *remixed;
                cJSON       *volume;

                if (!M_StringToDigest(item->string, digest, sizeof(digest)))
                    continue;

                entry = S_GetOrCreateTrakInfo(digest);
                midi = cJSON_GetObjectItemCaseSensitive(item, "MIDI");
                remixed = cJSON_GetObjectItemCaseSensitive(item, "Remixed");
                volume = cJSON_GetObjectItemCaseSensitive(item, "volume");

                if (cJSON_IsString(midi) && *midi->valuestring)
                {
                    free(entry->midi);
                    entry->midi = M_StringDuplicate(midi->valuestring);
                }

                if (cJSON_IsString(remixed) && *remixed->valuestring)
                {
                    free(entry->remixed);
                    entry->remixed = M_StringDuplicate(remixed->valuestring);
                }

                if (cJSON_IsNumber(volume))
                {
                    entry->volume = (float)volume->valuedouble;
                    entry->hasvolume = true;
                }
                else if (cJSON_IsString(volume) && *volume->valuestring)
                {
                    char    *endptr;
                    float   value = strtof(volume->valuestring, &endptr);

                    if (endptr != volume->valuestring)
                    {
                        entry->volume = value;
                        entry->hasvolume = true;
                    }
                }
            }

    cJSON_Delete(json);
    Z_Free(buffer);
}

void S_ParseTrakInfo(void)
{
    S_ClearTrakInfo();

    for (int i = 0; i < numlumps; i++)
        if (M_StringCompare(lumpinfo[i]->name, "TRAKINFO"))
            S_ParseTrakInfoLump(i);
}

static void S_GetTrakInfoDigest(const int lumpnum, byte digest[SHA1_DIGEST_SIZE])
{
    SHA1Context context;
    const byte  *data = W_CacheLumpNum(lumpnum);

    SHA1Init(&context);
    SHA1Update(&context, data, W_LumpLength(lumpnum));
    SHA1Final(digest, &context);
    W_ReleaseLumpNum(lumpnum);
}

int S_ResolveTrakInfoMusic(const int lumpnum, float *volume)
{
    int result = lumpnum;

    for (int i = 0; i < 4; i++)
    {
        byte                digest[SHA1_DIGEST_SIZE];
        const trakinfo_t    *entry;

        S_GetTrakInfoDigest(result, digest);

        if (!(entry = S_FindTrakInfo(digest)))
            break;

        if (entry->hasvolume)
            *volume = entry->volume;

        if ((s_remix && entry->remixed) || (!s_remix && entry->midi))
        {
            const int   preferred = W_CheckNumForName(s_remix ? entry->remixed : entry->midi);

            if (preferred >= 0 && preferred != result)
            {
                result = preferred;
                continue;
            }
        }

        break;
    }

    return result;
}
