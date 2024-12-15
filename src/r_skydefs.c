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

#include "c_console.h"
#include "cJSON/cJSON.h"
#include "m_array.h"
#include "r_skydefs.h"
#include "w_wad.h"

static bool ParseFire(cJSON *json, fire_t *out)
{
    cJSON   *updatetime;
    cJSON   *palette;

    if (!cJSON_IsNumber((updatetime = cJSON_GetObjectItemCaseSensitive(json, "updatetime"))))
        return false;

    out->updatetime = (int)(updatetime->valuedouble * TICRATE);

    if (!cJSON_IsArray((palette = cJSON_GetObjectItemCaseSensitive(json, "palette"))))
        return false;

    for (int i = 0, size = cJSON_GetArraySize(palette); i < size; i++)
        array_push(out->palette, cJSON_GetArrayItem(palette, i)->valueint);

    return true;
}

static bool ParseSkyTex(cJSON *json, skytex_t *out)
{
    cJSON   *name;
    cJSON   *mid;
    cJSON   *scrollx;
    cJSON   *scrolly;
    cJSON   *scalex;
    cJSON   *scaley;

    if (!cJSON_IsString((name = cJSON_GetObjectItemCaseSensitive(json, "name"))))
        return false;

    out->name = M_StringDuplicate(name->valuestring);

    if (!cJSON_IsNumber((mid = cJSON_GetObjectItemCaseSensitive(json, "mid")))
        || !cJSON_IsNumber((scrollx = cJSON_GetObjectItemCaseSensitive(json, "scrollx")))
        || !cJSON_IsNumber((scrolly = cJSON_GetObjectItemCaseSensitive(json, "scrolly")))
        || !cJSON_IsNumber((scalex = cJSON_GetObjectItemCaseSensitive(json, "scalex")))
        || !cJSON_IsNumber((scaley = cJSON_GetObjectItemCaseSensitive(json, "scaley"))))
        return false;

    out->mid = mid->valuedouble;
    out->scrollx = (fixed_t)(scrollx->valuedouble * (1.0 / TICRATE) * FRACUNIT);
    out->scrolly = (fixed_t)(scrolly->valuedouble * (1.0 / TICRATE) * FRACUNIT);
    out->scalex = (fixed_t)(scalex->valuedouble * FRACUNIT);
    out->scaley = (scaley->valuedouble ? (fixed_t)(1.0 / scaley->valuedouble * FRACUNIT) : FRACUNIT);
    return true;
}

static bool ParseSky(cJSON *json, sky_t *out)
{
    cJSON       *type;
    cJSON       *js_fire;
    cJSON       *js_foreground;
    skytex_t    background = { 0 };
    fire_t      fire = { 0 };
    skytex_t    foreground = { 0 };

    if (!cJSON_IsNumber((type = cJSON_GetObjectItemCaseSensitive(json, "type"))))
        return false;

    out->type = type->valueint;

    if (!ParseSkyTex(json, &background))
        return false;

    out->skytex = background;

    if (!cJSON_IsNull((js_fire = cJSON_GetObjectItemCaseSensitive(json, "fire"))))
        ParseFire(js_fire, &fire);

    out->fire = fire;

    if (!cJSON_IsNull((js_foreground = cJSON_GetObjectItemCaseSensitive(json, "foregroundtex"))))
        ParseSkyTex(js_foreground, &foreground);

    out->foreground = foreground;
    return true;
}

static bool ParseFlatMap(cJSON *json, flatmap_t *out)
{
    cJSON   *flat = cJSON_GetObjectItemCaseSensitive(json, "flat");
    cJSON   *sky = cJSON_GetObjectItemCaseSensitive(json, "sky");

    out->flat = M_StringDuplicate(flat->valuestring);
    out->sky = M_StringDuplicate(sky->valuestring);
    return true;
}

skydefs_t *R_ParseSkyDefs(void)
{
    int         lumpnum = W_CheckNumForName("SKYDEFS");
    cJSON       *json;
    cJSON       *data;
    cJSON       *js_skies;
    cJSON       *js_sky = NULL;
    cJSON       *js_flatmapping;
    cJSON       *js_flatmap = NULL;
    skydefs_t   *out;

    if (lumpnum == -1)
        return NULL;

    if (!((json = cJSON_ParseWithLength(W_CacheLumpNum(lumpnum), W_LumpLength(lumpnum)))))
    {
        cJSON_Delete(json);
        C_Warning(1, "The " BOLD("SKYDEFS") " lump in " BOLD("%s") " couldn't be parsed.",
            leafname(lumpinfo[lumpnum]->wadfile->path));
        return NULL;
    }

    if (!cJSON_IsObject((data = cJSON_GetObjectItemCaseSensitive(json, "data"))))
    {
        cJSON_Delete(json);
        return NULL;
    }

    out = calloc(1, sizeof(*out));
    js_skies = cJSON_GetObjectItemCaseSensitive(data, "skies");

    cJSON_ArrayForEach(js_sky, js_skies)
    {
        sky_t   sky = { 0 };

        if (ParseSky(js_sky, &sky))
            array_push(out->skies, sky);
    }

    js_flatmapping = cJSON_GetObjectItemCaseSensitive(data, "flatmapping");

    cJSON_ArrayForEach(js_flatmap, js_flatmapping)
    {
        flatmap_t   flatmap = { 0 };

        if (ParseFlatMap(js_flatmap, &flatmap))
            array_push(out->flatmapping, flatmap);
    }

    cJSON_Delete(json);
    return out;
}
