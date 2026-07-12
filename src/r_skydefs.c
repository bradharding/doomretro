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

#include "c_console.h"
#include "m_array.h"
#include "r_skydefs.h"
#include "w_wad.h"
#include "yyjson/yyjson.h"

static bool ParseFire(yyjson_val *json, fire_t *out)
{
    yyjson_val  *updatetime;
    yyjson_val  *palette;

    if (!yyjson_is_num((updatetime = yyjson_obj_get(json, "updatetime"))))
        return false;

    out->updatetime = (int)(yyjson_get_num(updatetime) * TICRATE);

    if (!yyjson_is_arr((palette = yyjson_obj_get(json, "palette"))))
        return false;

    for (size_t i = 0, size = yyjson_arr_size(palette); i < size; i++)
        array_push(out->palette, yyjson_get_int(yyjson_arr_get(palette, i)));

    return true;
}

static bool ParseSkyTex(yyjson_val *json, skytex_t *out)
{
    yyjson_val  *name;
    yyjson_val  *mid;
    yyjson_val  *scrollx;
    yyjson_val  *scrolly;
    yyjson_val  *scalex;
    yyjson_val  *scaley;

    if (!yyjson_is_str((name = yyjson_obj_get(json, "name"))))
        return false;

    out->name = M_StringDuplicate(yyjson_get_str(name));

    if (!yyjson_is_num((mid = yyjson_obj_get(json, "mid")))
        || !yyjson_is_num((scrollx = yyjson_obj_get(json, "scrollx")))
        || !yyjson_is_num((scrolly = yyjson_obj_get(json, "scrolly")))
        || !yyjson_is_num((scalex = yyjson_obj_get(json, "scalex")))
        || !yyjson_is_num((scaley = yyjson_obj_get(json, "scaley"))))
        return false;

    out->mid = yyjson_get_num(mid);
    out->scrollx = (fixed_t)(yyjson_get_num(scrollx) * (1.0 / TICRATE) * (double)FRACUNIT);
    out->scrolly = (fixed_t)(yyjson_get_num(scrolly) * (1.0 / TICRATE) * (double)FRACUNIT);
    out->scalex = (yyjson_get_num(scalex) ? (fixed_t)(1.0 / yyjson_get_num(scalex) * (double)FRACUNIT) : FRACUNIT);
    out->scaley = (yyjson_get_num(scaley) ? (fixed_t)(1.0 / yyjson_get_num(scaley) * (double)FRACUNIT) : FRACUNIT);
    return true;
}

static bool ParseSky(yyjson_val *json, sky_t *out)
{
    yyjson_val  *type;
    yyjson_val  *js_fire;
    yyjson_val  *js_foreground;
    skytex_t    background = { 0 };
    fire_t      fire = { 0 };
    skytex_t    foreground = { 0 };

    if (!yyjson_is_num((type = yyjson_obj_get(json, "type"))))
        return false;

    out->type = yyjson_get_int(type);

    if (!ParseSkyTex(json, &background))
        return false;

    out->skytex = background;

    if ((js_fire = yyjson_obj_get(json, "fire")) && !yyjson_is_null(js_fire))
        ParseFire(js_fire, &fire);

    out->fire = fire;

    if ((js_foreground = yyjson_obj_get(json, "foregroundtex")) && !yyjson_is_null(js_foreground))
        ParseSkyTex(js_foreground, &foreground);

    out->foreground = foreground;
    return true;
}

static bool ParseFlatMap(yyjson_val *json, flatmap_t *out)
{
    yyjson_val  *flat;
    yyjson_val  *sky;

    if (!yyjson_is_str((flat = yyjson_obj_get(json, "flat"))))
        return false;

    out->flat = M_StringDuplicate(yyjson_get_str(flat));

    if (!yyjson_is_str((sky = yyjson_obj_get(json, "sky"))))
        return false;

    out->sky = M_StringDuplicate(yyjson_get_str(sky));
    return true;
}

skydefs_t *R_ParseSkyDefs(void)
{
    int         lumpnum = W_CheckNumForName("SKYDEFS");
    yyjson_doc  *jsondoc;
    yyjson_val  *json;
    yyjson_val  *data;
    yyjson_val  *js_skies;
    yyjson_val  *js_sky;
    yyjson_val  *js_flatmapping;
    yyjson_val  *js_flatmap;
    skydefs_t   *out;

    if (lumpnum == -1)
        return NULL;

    if (!((jsondoc = yyjson_read(W_CacheLumpNum(lumpnum), (size_t)W_LumpLength(lumpnum), 0)))
        || !yyjson_is_obj((json = yyjson_doc_get_root(jsondoc))))
    {
        yyjson_doc_free(jsondoc);
        C_Warning(1, "The " BOLD("SKYDEFS") " lump in " BOLD("%s") " couldn't be parsed.",
            leafname(lumpinfo[lumpnum]->wadfile->path));
        return NULL;
    }

    if (!yyjson_is_obj((data = yyjson_obj_get(json, "data"))))
    {
        yyjson_doc_free(jsondoc);
        return NULL;
    }

    if (!(out = calloc(1, sizeof(*out))))
    {
        yyjson_doc_free(jsondoc);
        return NULL;
    }

    js_skies = yyjson_obj_get(data, "skies");

    if (yyjson_is_arr(js_skies))
    {
        size_t  idx;
        size_t  max;

        yyjson_arr_foreach(js_skies, idx, max, js_sky)
        {
            sky_t   sky = { 0 };

            if (ParseSky(js_sky, &sky))
                array_push(out->skies, sky);
        }
    }

    js_flatmapping = yyjson_obj_get(data, "flatmapping");

    if (yyjson_is_arr(js_flatmapping))
    {
        size_t  idx;
        size_t  max;

        yyjson_arr_foreach(js_flatmapping, idx, max, js_flatmap)
        {
            flatmap_t   flatmap = { 0 };

            if (ParseFlatMap(js_flatmap, &flatmap))
                array_push(out->flatmapping, flatmap);
        }
    }

    yyjson_doc_free(jsondoc);
    return out;
}
