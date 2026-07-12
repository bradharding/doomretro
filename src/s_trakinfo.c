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

#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "sha1.h"
#include "s_trakinfo.h"
#include "w_wad.h"
#include "yyjson/yyjson.h"

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
    yyjson_doc  *jsondoc;
    yyjson_val  *json;

    W_ReleaseLumpNum(lumpnum);

    jsondoc = yyjson_read(data, (size_t)lumplength, YYJSON_READ_ALLOW_COMMENTS);
    json = yyjson_doc_get_root(jsondoc);

    if (yyjson_is_obj(json))
    {
        size_t      idx;
        size_t      max;
        yyjson_val  *key;
        yyjson_val  *item;

        yyjson_obj_foreach(json, idx, max, key, item)
            if (yyjson_is_obj(item) && yyjson_is_str(key))
            {
                byte        digest[SHA1_DIGEST_SIZE];
                trakinfo_t  *entry;
                yyjson_val  *midi;
                yyjson_val  *remixed;
                yyjson_val  *volume;
                const char  *digeststr = yyjson_get_str(key);

                if (!M_StringToDigest(digeststr, digest, sizeof(digest)))
                    continue;

                entry = S_GetOrCreateTrakInfo(digest);
                midi = yyjson_obj_get(item, "MIDI");
                remixed = yyjson_obj_get(item, "Remixed");
                volume = yyjson_obj_get(item, "volume");

                if (yyjson_is_str(midi) && *yyjson_get_str(midi))
                {
                    free(entry->midi);
                    entry->midi = M_StringDuplicate(yyjson_get_str(midi));
                }

                if (yyjson_is_str(remixed) && *yyjson_get_str(remixed))
                {
                    free(entry->remixed);
                    entry->remixed = M_StringDuplicate(yyjson_get_str(remixed));
                }

                if (yyjson_is_num(volume))
                {
                    entry->volume = (float)yyjson_get_num(volume);
                    entry->hasvolume = true;
                }
                else if (yyjson_is_str(volume) && *yyjson_get_str(volume))
                {
                    char        *endptr;
                    const char  *volumestr = yyjson_get_str(volume);
                    float       value = strtof(volumestr, &endptr);

                    if (endptr != volumestr)
                    {
                        entry->volume = value;
                        entry->hasvolume = true;
                    }
                }
            }
    }

    yyjson_doc_free(jsondoc);
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
