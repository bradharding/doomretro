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

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "c_console.h"
#include "doomstat.h"
#include "i_system.h"
#include "i_video.h"
#include "m_config.h"
#include "m_fixed.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_state.h"
#include "r_things.h"
#include "r_voxels.h"
#include "sprites.h"
#include "tables.h"
#include "v_video.h"
#include "w_wad.h"

#define VX_MAX_FRAMES   29
#define VX_MINZ         (4 * FRACUNIT)
#define VX_MAX_DIST     (8192 * FRACUNIT)
#define FRACMASK        (FRACUNIT - 1)

enum
{
    F_LEFT   = 0x01,
    F_RIGHT  = 0x02,
    F_BACK   = 0x04,
    F_FRONT  = 0x08,
    F_TOP    = 0x10,
    F_BOTTOM = 0x20
};

typedef struct
{
    int     x_size, y_size, z_size;
    fixed_t x_pivot, y_pivot, z_pivot;
    int     *offsets;
    byte    *data;
} voxel_t;

typedef struct
{
    voxel_t *model;
    angle_t angle_offset;
} voxelbinding_t;

typedef struct
{
    voxel_t *model;
    angle_t angle;
    fixed_t tl_x, tl_y;
    fixed_t c, s;
    fixed_t liquidclipz;
    bool    liquidclip;
} visvoxel_t;

static voxelbinding_t  **bindings;
static voxel_t         **models_by_lump;
static visvoxel_t      *visvoxels;
static int             numvisvoxels;
static int             maxvisvoxels;
static fixed_t         eye_x, eye_y;

static fixed_t VX_ProjectScreenX(fixed_t x, fixed_t scale)
{
    const int64_t   limit = (int64_t)MAXWIDTH * FRACUNIT;
    const int64_t   projected = (int64_t)centerxfrac + (((int64_t)x * scale) >> FRACBITS);

    return (fixed_t)(projected < -limit ? -limit : (projected > limit * 2 ? limit * 2 : projected));
}

static fixed_t VX_ProjectScreenY(fixed_t z, fixed_t scale)
{
    const int64_t   limit = (int64_t)MAXHEIGHT * FRACUNIT;
    const int64_t   projected = (int64_t)centeryfrac - (((int64_t)z * scale) >> FRACBITS);

    return (fixed_t)(projected < -limit ? -limit : (projected > limit * 2 ? limit * 2 : projected));
}

static byte VX_LitColor(const vissprite_t *spr, byte color)
{
    const int   flags = spr->mobj->flags;

    // Blood mobjs inherit the wounded actor's blood color. The sprite path
    // applies the corresponding CR* translation through bloodcolfunc; do the
    // same for voxelized BLUD frames so cacodemon blood is blue, baron/knight
    // blood is green, and custom blood colors continue to work automatically.
    if (spr->mobj->colfunc == bloodcolfunc && spr->mobj->bloodcolor > NOBLOOD)
        color = colortranslation[spr->mobj->bloodcolor - 1][color];

    // DOOM Retro gives preplaced player corpses a random player-color
    // translation. That works for the original sprite's exact green ramp,
    // but a KVX model contains many nearby custom shades. Translating only
    // the exact ramp produces the red/dark horizontal streaks seen across
    // Voxel Doom corpses, so keep authored corpse colors intact.
    else if ((flags & MF_TRANSLATION) && !(flags & MF_CORPSE))
        color = translationtables[((flags & MF_TRANSLATION) >> (MF_TRANSLATIONSHIFT - 8)) - 256 + color];

    return spr->sectorcolormap[spr->colormap[color]];
}

static int VX_PaletteIndex(const byte *palette, int r, int g, int b)
{
    int best = 0;
    int bestdist = INT32_MAX;

    for (int i = 0; i < 256; i++)
    {
        const int   dr = r - palette[i * 3];
        const int   dg = g - palette[i * 3 + 1];
        const int   db = b - palette[i * 3 + 2];
        const int   dist = dr * dr + dg * dg + db * db;

        if (dist < bestdist)
        {
            best = i;
            bestdist = dist;
        }
    }

    return best;
}

static uint32_t VX_ReadU32(const byte *p)
{
    return ((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24);
}

static fixed_t VX_ReadPivot(const byte *p)
{
    // KVX stores signed pivots as 8.8 fixed point. Convert them to the
    // engine's 16.16 fixed-point coordinates before projecting the model.
    return (fixed_t)((int64_t)(int32_t)VX_ReadU32(p) * (FRACUNIT >> 8));
}

static voxel_t *VX_Decode(const byte *source, int length)
{
    const byte  *p = source;
    const byte  *end = source + length;
    const byte  *playpal;
    voxel_t     *v;
    int         xoffsets[257];
    int         minoffset = INT32_MAX;
    int         maxoffset = 0;
    int         numoffsets;
    byte        remap[256];

    if (length < 40 + 768)
        return NULL;

    v = I_Calloc(1, sizeof(*v));
    p += 4;
    v->x_size = (int)VX_ReadU32(p); p += 4;
    v->y_size = (int)VX_ReadU32(p); p += 4;
    v->z_size = (int)VX_ReadU32(p); p += 4;

    if (v->x_size <= 0 || v->x_size > 256
        || v->y_size <= 0 || v->y_size > 256
        || v->z_size <= 0 || v->z_size > 256)
        goto badvoxel;

    v->x_pivot = VX_ReadPivot(p); p += 4;
    v->y_pivot = VX_ReadPivot(p); p += 4;
    v->z_pivot = VX_ReadPivot(p); p += 4;

    if (p + (v->x_size + 1) * 4 > end - 768)
        goto badvoxel;

    for (int x = 0; x <= v->x_size; x++, p += 4)
        xoffsets[x] = (int)VX_ReadU32(p);

    numoffsets = v->x_size * (v->y_size + 1);

    if (p + numoffsets * 2 > end - 768)
        goto badvoxel;

    v->offsets = I_Malloc(numoffsets * sizeof(*v->offsets));

    for (int x = 0; x < v->x_size; x++)
        for (int y = 0; y <= v->y_size; y++, p += 2)
        {
            const int offset = (p[0] | p[1] << 8) + xoffsets[x];

            v->offsets[y * v->x_size + x] = offset;
            minoffset = MIN(minoffset, offset);
            maxoffset = MAX(maxoffset, offset);
        }

    if (minoffset < 0 || maxoffset <= minoffset || 7 * 4 + maxoffset > length - 768)
        goto badvoxel;

    v->data = I_Malloc(maxoffset - minoffset);
    memcpy(v->data, source + 7 * 4 + minoffset, maxoffset - minoffset);

    for (int i = 0; i < numoffsets; i++)
        v->offsets[i] -= minoffset;

    playpal = W_CacheLumpName("PLAYPAL");

    for (int i = 0; i < 256; i++)
        remap[i] = VX_PaletteIndex(playpal, source[length - 768 + i * 3] << 2,
            source[length - 767 + i * 3] << 2, source[length - 766 + i * 3] << 2);

    for (int x = 0; x < v->x_size; x++)
        for (int y = 0; y < v->y_size; y++)
        {
            byte    *slab = v->data + v->offsets[y * v->x_size + x];
            byte    *slabend = v->data + v->offsets[(y + 1) * v->x_size + x];

            while (slab + 3 <= slabend)
            {
                const int   len = slab[1];

                slab += 3;

                if (slab + len > slabend)
                    break;

                for (int i = 0; i < len; i++)
                    slab[i] = remap[slab[i]];

                slab += len;
            }
        }

    return v;

badvoxel:
    free(v->offsets);
    free(v->data);
    free(v);

    return NULL;
}

static voxel_t *VX_ModelForName(const char *name)
{
    const int   lump = W_CheckNumForNameInNamespace(name, ns_voxels);

    if (lump < 0)
        return NULL;

    if (!models_by_lump[lump])
    {
        const byte  *data = W_CacheLumpNum(lump);

        models_by_lump[lump] = VX_Decode(data, W_LumpLength(lump));
        W_ReleaseLumpNum(lump);
    }

    return models_by_lump[lump];
}

static int VX_FindSprite(const char *name)
{
    // VoxelDoom uses alternate sprite families for the four spheres so
    // GZDoom can toggle them. Map those families back to vanilla actors.
    static const struct
    {
        const char  *voxel;
        const char  *doom;
    } aliases[] = {
        { "VINV", "PINV" },
        { "VOUL", "SOUL" },
        { "VINS", "PINS" },
        { "VEGA", "MEGA" }
    };

    for (int alias = 0; alias < arrlen(aliases); alias++)
        if (!strncasecmp(name, aliases[alias].voxel, 4))
        {
            name = aliases[alias].doom;
            break;
        }

    for (int i = 0; i < numsprites; i++)
        if (sprnames[i] && !strncasecmp(sprnames[i], name, 4))
            return i;

    return -1;
}

static void VX_ParseVoxelDef(const byte *data, int length)
{
    char        *p = (char *)data;
    const char  *end = p + length;

    while (p < end)
    {
        char    key[9] = { 0 };
        char    model[9] = { 0 };
        char    *lineend;
        int     keylen = 0;
        int     modellen = 0;
        int     angle = 0;

        while (p < end && isspace((unsigned char)*p))
            p++;

        if (p + 1 < end && p[0] == '/' && p[1] == '/')
        {
            while (p < end && *p != '\n')
                p++;

            continue;
        }

        if (p + 1 < end && p[0] == '/' && p[1] == '*')
        {
            p += 2;

            while (p + 1 < end && !(p[0] == '*' && p[1] == '/'))
                p++;

            p += (p + 1 < end ? 2 : 0);
            continue;
        }

        if (p >= end || !(isalnum((unsigned char)*p) || strchr("[]^\\", *p)))
        {
            p++;
            continue;
        }

        while (p < end && (isalnum((unsigned char)*p) || strchr("[]^\\", *p)))
        {
            if (keylen < 8)
                key[keylen++] = *p;

            p++;
        }

        while (p < end && isspace((unsigned char)*p))
            p++;

        if (p >= end || *p++ != '=')
            continue;

        while (p < end && isspace((unsigned char)*p))
            p++;

        if (p >= end || *p++ != '"')
            continue;

        while (p < end && *p != '"')
        {
            if (modellen < 8)
                model[modellen++] = *p;

            p++;
        }

        if (p < end)
            p++;

        lineend = p;

        while (lineend < end && *lineend != '\n')
            lineend++;

        for (const char *a = p; a + 11 < lineend; a++)
            if (!strncasecmp(a, "AngleOffset", 11))
            {
                a += 11;

                while (a < lineend && (isspace((unsigned char)*a) || *a == '='))
                    a++;

                angle = (int)strtol(a, NULL, 10);
                break;
            }

        if (keylen >= 5)
        {
            const int   spr = VX_FindSprite(key);
            int         frame = key[4] - 'A';

            if (key[4] == '^' || key[4] == '\\')
                frame = 27;
            else if (key[4] == '[')
                frame = 26;
            else if (key[4] == ']')
                frame = 28;

            if (spr >= 0 && frame >= 0 && frame < VX_MAX_FRAMES)
            {
                voxel_t *v = VX_ModelForName(model);

                if (v)
                {
                    bindings[spr][frame].model = v;
                    bindings[spr][frame].angle_offset = (angle_t)((uint64_t)(angle % 360) * ANG1);
                }
            }
        }

        p = lineend;
    }
}

void VX_Init(void)
{
    int found = 0;

    bindings = I_Malloc(numsprites * sizeof(*bindings));
    models_by_lump = I_Calloc(numlumps, sizeof(*models_by_lump));

    for (int spr = 0; spr < numsprites; spr++)
    {
        bindings[spr] = I_Calloc(VX_MAX_FRAMES, sizeof(**bindings));

        for (int frame = 0; frame < VX_MAX_FRAMES; frame++)
        {
            char    name[9] = { 0 };
            char    framechar = (frame == 26 ? '[' : frame == 27 ? '^' : frame == 28 ? ']' : 'A' + frame);

            if (!sprnames[spr])
                continue;

            snprintf(name, sizeof(name), "%.4s%c", sprnames[spr], framechar);
            bindings[spr][frame].model = VX_ModelForName(name);
        }
    }

    for (int i = 0; i < numlumps; i++)
        if (lumpinfo[i]->namespace == ns_global && !strncasecmp(lumpinfo[i]->name, "VOXELDEF", 8))
            VX_ParseVoxelDef(W_CacheLumpNum(i), W_LumpLength(i));

    for (int spr = 0; spr < numsprites; spr++)
        for (int frame = 0; frame < VX_MAX_FRAMES; frame++)
            found += !!bindings[spr][frame].model;

    if (found)
        C_Output("%i voxel model mapping%s initialized.", found, (found == 1 ? "" : "s"));
}

void VX_ClearVoxels(void)
{
    numvisvoxels = 0;
}

static int VX_NewVisVoxel(void)
{
    if (numvisvoxels >= maxvisvoxels)
    {
        maxvisvoxels = (maxvisvoxels ? maxvisvoxels * 2 : 128);
        visvoxels = I_Realloc(visvoxels, maxvisvoxels * sizeof(*visvoxels));
    }

    return numvisvoxels++;
}

bool VX_ProjectVoxel(mobj_t *thing, fixed_t gx, fixed_t gy, fixed_t gz)
{
    if (!r_voxels)
        return false;

    const int       spr = thing->sprite;
    const int       frame = thing->frame & FF_FRAMEMASK;
    voxelbinding_t  *binding;
    voxel_t         *v;
    fixed_t         dx, dy, tx, ty;
    fixed_t         c, s;
    fixed_t         cornersx[4], cornersy[4];
    fixed_t         xscale;
    angle_t         angle;
    int             x1 = viewwidth - 1;
    int             x2 = 0;
    int             index;
    visvoxel_t      *vv;
    vissprite_t     *vis;
    angle_t         relative;
    fixed_t         tlx, tly;

    if (!bindings || spr < 0 || spr >= numsprites || frame >= VX_MAX_FRAMES)
        return false;

    binding = &bindings[spr][frame];

    if (!(v = binding->model))
        return false;

    dx = gx - viewx;
    dy = gy - viewy;

    if (ABS(dx) > VX_MAX_DIST || ABS(dy) > VX_MAX_DIST)
        return true;

    tx = FixedMul(dx, viewsin) - FixedMul(dy, viewcos);
    ty = FixedMul(dx, viewcos) + FixedMul(dy, viewsin);

    if (ty < -64 * FRACUNIT)
        return true;

    xscale = (ty < VX_MINZ ? 15000 * FRACUNIT - ty : FixedDiv(projection, ty));

    // Voxels are real 3D objects, so their orientation normally remains fixed
    // in the world. Voxel Doom's four spheres are the exception: their models
    // have a deliberately presented face and look wrong edge-on, so aim that
    // face at the viewer without making ordinary pickups rotate in place.
    switch (spr)
    {
        case SPR_PINV:
        case SPR_PINS:
        case SPR_SOUL:
        case SPR_MEGA:
            angle = R_PointToAngle(gx, gy) + ANG180 + binding->angle_offset;
            break;

        default:
            angle = thing->angle + binding->angle_offset;
            break;
    }

    relative = ANG180 - viewangle + angle;

    c = finecosine[relative >> ANGLETOFINESHIFT];
    s = finesine[relative >> ANGLETOFINESHIFT];

    tlx = tx - FixedMul(v->x_pivot, c) - FixedMul(v->y_pivot, s);
    tly = ty - FixedMul(v->x_pivot, s) + FixedMul(v->y_pivot, c);

    cornersx[0] = tlx;
    cornersy[0] = tly;
    cornersx[1] = tlx + v->y_size * s;
    cornersy[1] = tly - v->y_size * c;
    cornersx[2] = cornersx[1] + v->x_size * c;
    cornersy[2] = cornersy[1] + v->x_size * s;
    cornersx[3] = tlx + v->x_size * c;
    cornersy[3] = tly + v->x_size * s;

    for (int i = 0; i < 4; i++)
    {
        int x;

        // A footprint crossing the near plane may cover the whole view.
        // Keep it as a voxel; individual cells are clipped while drawing.
        if (cornersy[i] < VX_MINZ)
        {
            x1 = 0;
            x2 = viewwidth - 1;
            break;
        }

        x = VX_ProjectScreenX(cornersx[i], FixedDiv(projection, cornersy[i])) >> FRACBITS;
        x1 = MIN(x1, x);
        x2 = MAX(x2, x);
    }

    x1 = MAX(0, x1);
    x2 = MIN(viewwidth - 1, x2);

    if (x1 > x2)
        return true;

    index = VX_NewVisVoxel();
    vv = &visvoxels[index];
    vv->model = v;
    vv->angle = angle;
    vv->tl_x = tlx;
    vv->tl_y = tly;
    vv->c = c;
    vv->s = s;
    vv->liquidclip = false;

    vis = R_NewVisSprite();
    memset(vis, 0, sizeof(*vis));
    vis->voxel_index = index;
    vis->drawfunc = VX_DrawVoxel;
    vis->mobj = thing;
    vis->heightsec = thing->subsector->sector->heightsec;
    vis->scale = xscale;
    vis->gx = gx;
    vis->gy = gy;
    vis->gz = gz;
    vis->gzt = gz + v->z_pivot;

    // Match DOOM Retro's sprite foot clipping in liquid sectors. Sink the
    // model slightly, then clip it against the animated liquid surface.
    if ((thing->flags2 & MF2_FEETARECLIPPED) && !thing->subsector->sector->heightsec
        && r_liquid_clipsprites && v->z_size >= 4)
    {
        const fixed_t   clipfeet = MIN(v->z_size / 4, 10) << FRACBITS;

        vis->gzt -= clipfeet;
        vv->liquidclip = true;
        vv->liquidclipz = thing->subsector->sector->interpfloorheight
            + (r_liquid_bobsprites ? animatedliquiddiff : 0);
    }

    vis->x1 = x1;
    vis->x2 = x2;
    vis->fullbright = !!((thing->frame & FF_FULLBRIGHT) || thing->info->fullbright);
    vis->sectorcolormap = R_GetSectorColormap(thing->subsector->sector);

    if (fixedcolormap)
        vis->colormap = fixedcolormap;
    else if (vis->fullbright)
        vis->colormap = fullcolormap;
    else
    {
        const int light = BETWEEN(0,
            (thing->subsector->sector->lightlevel >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1);

        vis->colormap = scalelight[light][MIN(xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)];
    }

    return true;
}

static void VX_DrawColumn(const vissprite_t *spr, int x, int y)
{
    const visvoxel_t    *vv = &visvoxels[spr->voxel_index];
    const voxel_t       *v = vv->model;
    const int           ofs1 = v->offsets[y * v->x_size + x];
    const int           ofs2 = v->offsets[(y + 1) * v->x_size + x];
    int                 qux, quy, quadrant, idx;
    fixed_t             px[4], py[4];
    fixed_t             ax, ay, bx, by, cx, cy, dx, dy;
    fixed_t             ascale, bscale, cscale, dscale;
    byte                aface, bface;
    byte                *dest = screens[0] + viewwindowy * SCREENWIDTH + viewwindowx;
    static const int    acorners[9] = { 3, 3, 2, 0, -1, 2, 0, 1, 1 };
    static const byte   afaces[9] = { F_BACK, F_BACK, F_RIGHT, F_LEFT, 0, F_RIGHT, F_LEFT, F_FRONT, F_FRONT };
    static const byte   bfaces[9] = { F_LEFT, 0, F_BACK, 0, 0, 0, F_FRONT, 0, F_RIGHT };
    fixed_t             uxstart, uxend;

    if (ofs1 >= ofs2)
        return;

    qux = (eye_x < (x << FRACBITS) ? 0 : eye_x < ((x + 1) << FRACBITS) ? 1 : 2);
    quy = (eye_y < (y << FRACBITS) ? 0 : eye_y < ((y + 1) << FRACBITS) ? 1 : 2);
    quadrant = quy * 3 + qux;

    if (quadrant == 4)
        return;

    px[0] = vv->tl_x + x * vv->c + y * vv->s;
    py[0] = vv->tl_y + x * vv->s - y * vv->c;
    px[1] = px[0] + vv->s; py[1] = py[0] - vv->c;
    px[2] = px[1] + vv->c; py[2] = py[1] + vv->s;
    px[3] = px[0] + vv->c; py[3] = py[0] + vv->s;

    idx = acorners[quadrant];
    ax = px[idx]; ay = py[idx]; idx = (idx + 1) & 3;
    bx = px[idx]; by = py[idx]; idx = (idx + 1) & 3;
    cx = px[idx]; cy = py[idx]; idx = (idx + 1) & 3;
    dx = px[idx]; dy = py[idx];

    if (ay < VX_MINZ && by < VX_MINZ && cy < VX_MINZ && dy < VX_MINZ)
        return;

    // Clip cells which straddle the view plane instead of dropping the
    // entire cell. Dropped cells were the source of the alternating
    // screen-height bands seen when the camera approached a voxel.
    ay = MAX(ay, VX_MINZ);
    by = MAX(by, VX_MINZ);
    cy = MAX(cy, VX_MINZ);
    dy = MAX(dy, VX_MINZ);

    ascale = FixedDiv(projection, ay); bscale = FixedDiv(projection, by);
    cscale = FixedDiv(projection, cy); dscale = FixedDiv(projection, dy);
    ax = VX_ProjectScreenX(ax, ascale);
    bx = VX_ProjectScreenX(bx, bscale);
    cx = VX_ProjectScreenX(cx, cscale);
    dx = VX_ProjectScreenX(dx, dscale);
    aface = afaces[quadrant];
    bface = bfaces[quadrant];

    uxstart = MAX(((ax - 1) | FRACMASK) + 1, spr->x1 << FRACBITS);
    uxend = MIN(MAX(cx, bx), (spr->x2 + 1) << FRACBITS);

    for (fixed_t ux = uxstart; ux < uxend; ux += FRACUNIT)
    {
        fixed_t     scale, iscale;
        const int   screenx = ux >> FRACBITS;
        const byte  *slab = v->data + ofs1;
        const byte  *slabend = v->data + ofs2;
        fixed_t     cliptop;
        fixed_t     clipbottom;

        if (screenx > spr->x2)
            break;

        if (screenx < spr->x1)
            continue;

        cliptop = BETWEEN(0, (mceilingclip[screenx] + 1) << FRACBITS, (viewheight << FRACBITS));

        clipbottom = BETWEEN(-1, (mfloorclip[screenx] << FRACBITS) - 1, (viewheight << FRACBITS) - 1);

        if (cliptop > clipbottom)
            continue;

        if (ux > bx && cx != bx)
            scale = bscale + FixedMul(cscale - bscale, FixedDiv(ux - bx, cx - bx));
        else if (bx != ax)
            scale = ascale + FixedMul(bscale - ascale, FixedDiv(ux - ax, bx - ax));
        else
            scale = bscale;

        if (scale <= 0)
            continue;

        if (vv->liquidclip)
        {
            const fixed_t   liquidclip = centeryfrac - FixedMul(vv->liquidclipz - viewz, scale) - 1;

            clipbottom = MIN(clipbottom, liquidclip);

            if (cliptop > clipbottom)
                continue;
        }

        iscale = FixedDiv(FRACUNIT, scale);

        while (slab + 3 <= slabend)
        {
            const int       top = *slab++;
            const int       len = *slab++;
            const int       face = *slab++;
            const fixed_t   topz = spr->gzt - viewz - (top << FRACBITS);
            fixed_t         uy1 = VX_ProjectScreenY(topz, scale);
            fixed_t         uy2 = VX_ProjectScreenY(topz - (len << FRACBITS), scale);
            const fixed_t   originaluy1 = uy1;
            const bool      side = !!(face & (ux > bx ? bface : aface));
            fixed_t         widescale = 0;

            if (!len || slab + len > slabend)
                break;

            // Clamp both ends. The top and bottom face loops also use these
            // values, so one-sided clipping can otherwise write past the
            // framebuffer when a voxel crosses a floor or ceiling edge.
            uy1 = BETWEEN(cliptop, uy1, clipbottom);
            uy2 = BETWEEN(cliptop, uy2, clipbottom);

            if ((spr->mobj->flags & MF_FUZZ) && side && uy1 <= uy2)
            {
                dc_x = screenx;
                dc_yl = MAX(0, uy1 >> FRACBITS);
                dc_yh = MIN(viewheight - 1, uy2 >> FRACBITS);

                // R_DrawFuzzColumn writes 2x2 blocks. Keep its final block
                // inside the view at the bottom and right edges. Normal sprite
                // posts satisfy these assumptions implicitly, but an enlarged
                // or near-plane-clipped voxel slab can reach the last row.
                if (!(dc_yl & 1))
                    dc_yh = MIN(dc_yh, viewheight - 2);

                if (dc_x + 1 < viewwidth && dc_yh - dc_yl >= 2)
                    R_DrawFuzzColumn();

                slab += len;
                continue;
            }

            if ((face & F_TOP) || (face & F_BOTTOM))
            {
                if (ux > cx && bx != cx)
                    widescale = cscale + FixedMul(bscale - cscale, FixedDiv(ux - cx, bx - cx));
                else if (ux > dx && cx != dx)
                    widescale = dscale + FixedMul(cscale - dscale, FixedDiv(ux - dx, cx - dx));
                else if (dx != ax)
                    widescale = ascale + FixedMul(dscale - ascale, FixedDiv(ux - ax, dx - ax));
            }

            if ((face & F_TOP) && topz < 0)
            {
                fixed_t     uy = VX_ProjectScreenY(topz, widescale);
                const byte  color = VX_LitColor(spr, slab[0]);

                // Start on an exact screen row. Without this ceil operation,
                // the fractional top-face walk can stop one row before the
                // side-face walk starts, leaving horizontal background-colored
                // seams between otherwise adjacent voxel faces.
                uy = MAX(((uy - 1) | FRACMASK) + 1, cliptop);

                for (; uy < uy1; uy += FRACUNIT)
                    dest[(uy >> FRACBITS) * SCREENWIDTH + screenx] = color;
            }
            else if ((face & F_BOTTOM) && topz > (len << FRACBITS))
            {
                fixed_t     uy = MIN(VX_ProjectScreenY(topz - (len << FRACBITS), widescale), clipbottom);
                const byte  color = VX_LitColor(spr, slab[len - 1]);

                for (; uy > uy2; uy -= FRACUNIT)
                    dest[(uy >> FRACBITS) * SCREENWIDTH + screenx] = color;
            }

            if (side)
                for (fixed_t uy = ((uy1 - 1) | FRACMASK) + 1; uy <= uy2; uy += FRACUNIT)
                {
                    int source = (int)(((int64_t)((uy - originaluy1) >> FRACBITS) * iscale) >> FRACBITS);

                    source = BETWEEN(0, source, len - 1);
                    dest[(uy >> FRACBITS) * SCREENWIDTH + screenx] = VX_LitColor(spr, slab[source]);
                }

            slab += len;
        }
    }
}

static void VX_RecursiveDraw(const vissprite_t *spr, int x, int y, int width, int height)
{
    if (width == 1 && height == 1)
    {
        VX_DrawColumn(spr, x, y);
        return;
    }

    if (width >= height)
    {
        const int   left = width / 2;
        const int   right = width - left;

        if (eye_x < ((x * 2 + width) << (FRACBITS - 1)))
        {
            VX_RecursiveDraw(spr, x + left, y, right, height);
            VX_RecursiveDraw(spr, x, y, left, height);
        }
        else
        {
            VX_RecursiveDraw(spr, x, y, left, height);
            VX_RecursiveDraw(spr, x + left, y, right, height);
        }
    }
    else
    {
        const int   top = height / 2;
        const int   bottom = height - top;

        if (eye_y < ((y * 2 + height) << (FRACBITS - 1)))
        {
            VX_RecursiveDraw(spr, x, y + top, width, bottom);
            VX_RecursiveDraw(spr, x, y, width, top);
        }
        else
        {
            VX_RecursiveDraw(spr, x, y, width, top);
            VX_RecursiveDraw(spr, x, y + top, width, bottom);
        }
    }
}

void VX_DrawVoxel(const vissprite_t *spr)
{
    const visvoxel_t    *vv = &visvoxels[spr->voxel_index];
    const voxel_t       *v = vv->model;
    const unsigned int  angle = (vv->angle + ANG90) >> ANGLETOFINESHIFT;
    const fixed_t       c = finecosine[angle];
    const fixed_t       s = finesine[angle];
    const fixed_t       dx = viewx - spr->gx;
    const fixed_t       dy = viewy - spr->gy;

    // The regular sprite renderer starts a fresh fuzz sequence per sprite.
    // Do the same here: a voxel may contain thousands of independently drawn
    // slabs and must not inherit an index left by another object.
    if (spr->mobj->flags & MF_FUZZ)
        fuzz1pos = 0;

    eye_x = v->x_pivot + FixedMul(dx, c) + FixedMul(dy, s);
    eye_y = v->y_pivot + FixedMul(dx, s) - FixedMul(dy, c);
    VX_RecursiveDraw(spr, 0, 0, v->x_size, v->y_size);
}
