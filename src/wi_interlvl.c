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
#include "doomtype.h"
#include "m_array.h"
#include "w_wad.h"
#include "wi_interlvl.h"
#include "yyjson/yyjson.h"

static bool ParseCondition(yyjson_val *json, interlevelcond_t *out)
{
    yyjson_val  *condition;
    yyjson_val  *param;

    if (!yyjson_is_num((condition = yyjson_obj_get(json, "condition"))))
        return false;

    out->condition = yyjson_get_int(condition);

    if (!yyjson_is_num((param = yyjson_obj_get(json, "param"))))
        return false;

    out->param = yyjson_get_int(param);
    return true;
}

static bool ParseFrame(yyjson_val *json, interlevelframe_t *out)
{
    yyjson_val  *imagelump;
    yyjson_val  *type;
    yyjson_val  *duration;
    yyjson_val  *maxduration;

    if (!yyjson_is_str((imagelump = yyjson_obj_get(json, "image"))))
        return false;

    out->imagelump = M_StringDuplicate(yyjson_get_str(imagelump));

    if (!yyjson_is_num((type = yyjson_obj_get(json, "type"))))
        return false;

    out->type = yyjson_get_int(type);

    if (!yyjson_is_num((duration = yyjson_obj_get(json, "duration"))))
        return false;

    out->duration = (int)(yyjson_get_num(duration) * TICRATE);

    if (!yyjson_is_num((maxduration = yyjson_obj_get(json, "maxduration"))))
        return false;

    out->maxduration = (int)(yyjson_get_num(maxduration) * TICRATE);
    return true;
}

static bool ParseAnim(yyjson_val *json, interlevelanim_t *out)
{
    yyjson_val          *js_frames;
    yyjson_val          *js_frame;
    yyjson_val          *xpos;
    yyjson_val          *ypos;
    yyjson_val          *js_conditions;
    yyjson_val          *js_condition;
    interlevelframe_t   *frames = NULL;
    interlevelcond_t    *conditions = NULL;

    js_frames = yyjson_obj_get(json, "frames");

    if (yyjson_is_arr(js_frames))
    {
        size_t  idx;
        size_t  max;

        yyjson_arr_foreach(js_frames, idx, max, js_frame)
        {
            interlevelframe_t   frame = { 0 };

            if (ParseFrame(js_frame, &frame))
                array_push(frames, frame);
        }
    }

    out->frames = frames;

    if (!yyjson_is_num((xpos = yyjson_obj_get(json, "x")))
        || !yyjson_is_num((ypos = yyjson_obj_get(json, "y"))))
        return false;

    out->xpos = yyjson_get_int(xpos);
    out->ypos = yyjson_get_int(ypos);

    js_conditions = yyjson_obj_get(json, "conditions");

    if (yyjson_is_arr(js_conditions))
    {
        size_t  idx;
        size_t  max;

        yyjson_arr_foreach(js_conditions, idx, max, js_condition)
        {
            interlevelcond_t    condition = { 0 };

            if (ParseCondition(js_condition, &condition))
                array_push(conditions, condition);
        }
    }

    out->conditions = conditions;
    return true;
}

static void ParseLevelLayer(yyjson_val *json, interlevellayer_t *out)
{
    yyjson_val          *js_anims;
    yyjson_val          *js_anim;
    yyjson_val          *js_conditions;
    yyjson_val          *js_condition;
    interlevelanim_t    *anims = NULL;
    interlevelcond_t    *conditions = NULL;

    js_anims = yyjson_obj_get(json, "anims");

    if (yyjson_is_arr(js_anims))
    {
        size_t  idx;
        size_t  max;

        yyjson_arr_foreach(js_anims, idx, max, js_anim)
        {
            interlevelanim_t    anim = { 0 };

            if (ParseAnim(js_anim, &anim))
                array_push(anims, anim);
        }
    }

    out->anims = anims;
    js_conditions = yyjson_obj_get(json, "conditions");

    if (yyjson_is_arr(js_conditions))
    {
        size_t  idx;
        size_t  max;

        yyjson_arr_foreach(js_conditions, idx, max, js_condition)
        {
            interlevelcond_t    condition = { 0 };

            if (ParseCondition(js_condition, &condition))
                array_push(conditions, condition);
        }
    }

    out->conditions = conditions;
}

interlevel_t *WI_ParseInterlevel(const char *lumpname)
{
    int                 lumpnum = W_CheckNumForName(lumpname);
    interlevel_t        *out;
    yyjson_doc          *jsondoc;
    yyjson_val          *json;
    yyjson_val          *data;
    yyjson_val          *music;
    yyjson_val          *backgroundimage;
    yyjson_val          *js_layers;
    yyjson_val          *js_layer;
    interlevellayer_t   *layers = NULL;

    if (!(jsondoc = yyjson_read(W_CacheLumpNum(lumpnum), (size_t)W_LumpLength(lumpnum), 0))
        || !yyjson_is_obj((json = yyjson_doc_get_root(jsondoc)))
        || !yyjson_is_obj((data = yyjson_obj_get(json, "data")))
        || !yyjson_is_str((music = yyjson_obj_get(data, "music")))
        || !yyjson_is_str((backgroundimage = yyjson_obj_get(data, "backgroundimage"))))
    {
        yyjson_doc_free(jsondoc);
        C_Warning(1, "The " BOLD("%s") " lump in " BOLD("%s") " couldn't be parsed.",
            lumpname, leafname(lumpinfo[lumpnum]->wadfile->path));
        return NULL;
    }

    if (!(out = calloc(1, sizeof(*out))))
    {
        yyjson_doc_free(jsondoc);
        return NULL;
    }

    out->musiclump = M_StringDuplicate(yyjson_get_str(music));
    out->backgroundlump = M_StringDuplicate(yyjson_get_str(backgroundimage));

    js_layers = yyjson_obj_get(data, "layers");

    if (yyjson_is_arr(js_layers))
    {
        size_t  idx;
        size_t  max;

        yyjson_arr_foreach(js_layers, idx, max, js_layer)
        {
            interlevellayer_t   layer = { 0 };

            ParseLevelLayer(js_layer, &layer);
            array_push(layers, layer);
        }
    }

    out->layers = layers;

    yyjson_doc_free(jsondoc);
    return out;
}

void WI_FreeInterlevel(interlevel_t *interlevel)
{
    if (interlevel)
    {
        interlevellayer_t   *layer;

        free(interlevel->musiclump);
        free(interlevel->backgroundlump);

        array_foreach(layer, interlevel->layers)
        {
            interlevelanim_t    *anim;

            array_foreach(anim, layer->anims)
            {
                interlevelframe_t   *frame;

                array_foreach(frame, anim->frames)
                    free(frame->imagelump);

                array_free(anim->frames);
                array_free(anim->conditions);
            }

            array_free(layer->anims);
            array_free(layer->conditions);
        }

        array_free(interlevel->layers);
        free(interlevel);
    }
}
