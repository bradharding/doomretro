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
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "c_console.h"
#include "doomstat.h"
#include "i_colors.h"
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

static voxelbinding_t  **bindings;
static voxel_t         **modelsbylump;
static visvoxel_t      *visvoxels;
static int             numvisvoxels;
static int             maxvisvoxels;
static fixed_t         eyex, eyey;
static uint32_t        *shadowstamps;
static uint32_t        shadowstamp;

static voxeldepth_t    *voxeldepth;
static bool            voxeldepthcleared;

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

static const byte *VX_TintForThing(const mobj_t *thing)
{
    if (!r_sprites_translucency)
        return NULL;

    if (thing->state && thing->state->tranmap && !(thing->flags & MF_FUZZ))
        return thing->state->tranmap;

    return R_GetColumnTint(viewplayer && ISINVULNERABILITYCOLORMAP(viewplayer->fixedcolormap) && r_textures ?
        thing->altcolfunc : thing->colfunc);
}

static byte VX_LitColor(const vissprite_t *spr, byte color, const byte *dither, const int row,
    const byte *tint, const byte under)
{
    byte    lit;

    if (!r_textures)
        color = nearestwhite;
    else if (spr->mobj->colfunc == bloodcolfunc)
        color = colortranslation[spr->mobj->bloodcolor - 1][color];
    else
    {
        const int   flags = spr->mobj->flags;

        if (flags & MF_TRANSLATION)
            color = translationtables[((flags & MF_TRANSLATION) >> (MF_TRANSLATIONSHIFT - 8)) - 256 + color];
    }

    lit = (dither && dither[R_GetDitherRow(row)] ? spr->nextcolormap : spr->colormap)[color];

    return spr->sectorcolormap[(tint ? tint[(under << 8) + lit] : lit)];
}

static bool VX_DepthPass(const int pixel, const fixed_t depth, const int owner)
{
    voxeldepth_t    *d = &voxeldepth[pixel];

    if (d->owner && d->owner != owner && d->depth > depth)
        return false;

    d->depth = depth;
    d->owner = owner;
    return true;
}

static uint32_t VX_ReadU32(const byte *p)
{
    return ((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24);
}

static fixed_t VX_ReadPivot(const byte *p)
{
    return (fixed_t)((int64_t)(int32_t)VX_ReadU32(p) * (FRACUNIT >> 8));
}

static voxel_t *VX_Decode(const byte *source, int length)
{
    const byte  *p = source;
    const byte  *end = source + length;
    byte        *playpal;
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
    v->xsize = (int)VX_ReadU32(p);
    p += 4;
    v->ysize = (int)VX_ReadU32(p);
    p += 4;
    v->zsize = (int)VX_ReadU32(p);
    p += 4;

    if (v->xsize <= 0 || v->xsize > 256
        || v->ysize <= 0 || v->ysize > 256
        || v->zsize <= 0 || v->zsize > 256)
        goto badvoxel;

    v->xpivot = VX_ReadPivot(p);
    p += 4;
    v->ypivot = VX_ReadPivot(p);
    p += 4;
    v->zpivot = VX_ReadPivot(p);
    p += 4;

    if (p + (v->xsize + 1) * 4 > end - 768)
        goto badvoxel;

    for (int x = 0; x <= v->xsize; x++, p += 4)
        xoffsets[x] = (int)VX_ReadU32(p);

    numoffsets = v->xsize * (v->ysize + 1);

    if (p + numoffsets * 2 > end - 768)
        goto badvoxel;

    v->offsets = I_Malloc(numoffsets * sizeof(*v->offsets));

    for (int x = 0; x < v->xsize; x++)
        for (int y = 0; y <= v->ysize; y++, p += 2)
        {
            const int offset = (p[0] | p[1] << 8) + xoffsets[x];

            v->offsets[y * v->xsize + x] = offset;
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
        remap[i] = I_GetNearestColor(playpal, (source[length - 768 + i * 3] << 2),
            (source[length - 767 + i * 3] << 2), (source[length - 766 + i * 3] << 2));

    for (int x = 0; x < v->xsize; x++)
        for (int y = 0; y < v->ysize; y++)
        {
            byte    *slab = v->data + v->offsets[y * v->xsize + x];
            byte    *slabend = v->data + v->offsets[(y + 1) * v->xsize + x];

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

    if (!modelsbylump[lump])
    {
        const byte  *data = W_CacheLumpNum(lump);

        modelsbylump[lump] = VX_Decode(data, W_LumpLength(lump));
        W_ReleaseLumpNum(lump);
    }

    return modelsbylump[lump];
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
                    bindings[spr][frame].angleoffset = (angle_t)((uint64_t)(angle % 360) * ANG1);
                }
            }
        }

        p = lineend;
    }
}

void VX_Init(void)
{
    bindings = I_Malloc(numsprites * sizeof(*bindings));
    modelsbylump = I_Calloc(numlumps, sizeof(*modelsbylump));

    for (int spr = 0; spr < numsprites; spr++)
    {
        bindings[spr] = I_Calloc(VX_MAX_FRAMES, sizeof(**bindings));

        for (int frame = 0; frame < VX_MAX_FRAMES; frame++)
        {
            char    name[9] = { 0 };
            char    framechar = (frame == 26 ? '[' : frame == 27 ? '^' : frame == 28 ? ']' : 'A' + frame);

            if (!sprnames[spr])
                continue;

            M_snprintf(name, sizeof(name), "%.4s%c", sprnames[spr], framechar);
            bindings[spr][frame].model = VX_ModelForName(name);
        }
    }

    for (int i = 0; i < numlumps; i++)
        if (lumpinfo[i]->namespace == ns_global && !strncasecmp(lumpinfo[i]->name, "VOXELDEF", 8))
            VX_ParseVoxelDef(W_CacheLumpNum(i), W_LumpLength(i));
}

void VX_ClearVoxels(void)
{
    numvisvoxels = 0;
    voxeldepthcleared = false;
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
    fixed_t         relative;
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

    if (!(thing->flags & MF_SPECIAL))
    {
        angle = thing->angle + binding->angleoffset + thing->info->voxelangle;
        relative = (ANG180 - viewangle + angle) >> ANGLETOFINESHIFT;
    }
    else if (r_sprites_tilt)
    {
        angle = R_PointToAngle(gx, gy);
        relative = (angle - viewangle + binding->angleoffset + thing->info->voxelangle) >> ANGLETOFINESHIFT;
        angle += ANG180 + binding->angleoffset + thing->info->voxelangle;
    }
    else
    {
        angle = viewangle + ANG180 + binding->angleoffset + thing->info->voxelangle;
        relative = (binding->angleoffset + thing->info->voxelangle) >> ANGLETOFINESHIFT;
    }

    c = finecosine[relative];
    s = finesine[relative];

    tlx = tx - FixedMul(v->xpivot, c) - FixedMul(v->ypivot, s);
    tly = ty - FixedMul(v->xpivot, s) + FixedMul(v->ypivot, c);

    cornersx[0] = tlx;
    cornersy[0] = tly;
    cornersx[1] = tlx + v->ysize * s;
    cornersy[1] = tly - v->ysize * c;
    cornersx[2] = cornersx[1] + v->xsize * c;
    cornersy[2] = cornersy[1] + v->xsize * s;
    cornersx[3] = tlx + v->xsize * c;
    cornersy[3] = tly + v->xsize * s;

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
    vv->tlx = tlx;
    vv->tly = tly;
    vv->c = c;
    vv->s = s;
    vv->liquidclip = false;
    vv->shadow = ((thing->flags2 & MF2_CASTSHADOW) && r_shadows && !fixedcolormap && xscale >= FRACUNIT / 4);
    vv->tint = VX_TintForThing(thing);

    vis = R_NewVisSprite();
    memset(vis, 0, sizeof(*vis));
    vis->voxelindex = index;
    vis->drawfunc = VX_DrawVoxel;
    vis->mobj = thing;
    vis->heightsec = thing->subsector->sector->heightsec;
    vis->scale = xscale;
    vis->gx = gx;
    vis->gy = gy;
    vis->gz = gz;
    vis->gzt = gz + v->zpivot;

    // Match DOOM Retro's sprite foot clipping in liquid sectors. Sink the
    // model slightly, then clip it against the animated liquid surface.
    if ((thing->flags2 & MF2_FEETARECLIPPED) && !thing->subsector->sector->heightsec
        && r_liquid_clipsprites && v->zsize >= 4)
    {
        vis->gzt -= (MIN(v->zsize / 4, 10) << FRACBITS);
        vv->liquidclip = true;
        vv->liquidclipz = thing->subsector->sector->interpfloorheight
            + (r_liquid_bobsprites ? animatedliquiddiff : 0);
    }

    vis->x1 = x1;
    vis->x2 = x2;
    vis->fullbright = !!((thing->frame & FF_FULLBRIGHT) || thing->info->fullbright);
    vis->sectorcolormap = R_GetSectorColormap(thing->subsector->sector);

    if (fixedcolormap)
        vis->colormap = vis->nextcolormap = fixedcolormap;
    else if (vis->fullbright)
        vis->colormap = vis->nextcolormap = fullcolormap;
    else
    {
        const short lightlevel = thing->subsector->sector->lightlevel;
        const int   light = BETWEEN(0, ((lightlevel - 2) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1);
        const int   nextlight = BETWEEN(0, ((lightlevel + 2) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1);
        const int   scaleindex = MIN(xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1);

        vis->colormap = scalelight[light][scaleindex];
        vis->nextcolormap = scalelight[nextlight][scaleindex];
    }

    return true;
}

static void VX_DrawFuzz(const int x, const int yl, const int yh)
{
    if (yl > yh)
        return;

    dc_x = x;
    dc_yl = MAX(0, yl);
    dc_yh = MIN(viewheight - 1, yh);

    // R_DrawFuzzColumn writes 2x2 blocks. Keep its final block
    // inside the view at the bottom and right edges. Normal sprite
    // posts satisfy these assumptions implicitly, but an enlarged
    // or near-plane-clipped voxel slab can reach the last row.
    if (!(dc_yl & 1))
        dc_yh = MIN(dc_yh, viewheight - 2);

    if (dc_x + 1 < viewwidth && dc_yh - dc_yl >= 2)
        R_DrawFuzzColumn();
}

static void VX_DrawColumn(const vissprite_t *spr, int x, int y)
{
    const visvoxel_t    *vv = &visvoxels[spr->voxelindex];
    const voxel_t       *v = vv->model;
    const int           offsets1 = v->offsets[y * v->xsize + x];
    const int           offsets2 = v->offsets[(y + 1) * v->xsize + x];
    const int           owner = spr->voxelindex + 1;
    int                 qux, quy;
    int                 quadrant;
    int                 idx;
    fixed_t             px[4], py[4];
    fixed_t             ax, ay;
    fixed_t             bx, by;
    fixed_t             cx, cy;
    fixed_t             dx, dy;
    fixed_t             ascale;
    fixed_t             bscale;
    fixed_t             cscale;
    fixed_t             dscale;
    byte                aface;
    byte                bface;
    byte                *dest = screens[0] + viewwindowy * SCREENWIDTH + viewwindowx;
    static const int    acorners[9] = { 3, 3, 2, 0, -1, 2, 0, 1, 1 };
    static const byte   afaces[9] = { F_BACK, F_BACK, F_RIGHT, F_LEFT, 0, F_RIGHT, F_LEFT, F_FRONT, F_FRONT };
    static const byte   bfaces[9] = { F_LEFT, 0, F_BACK, 0, 0, 0, F_FRONT, 0, F_RIGHT };
    fixed_t             uxstart, uxend;

    if (offsets1 >= offsets2)
        return;

    qux = (eyex < (x << FRACBITS) ? 0 : eyex < ((x + 1) << FRACBITS) ? 1 : 2);
    quy = (eyey < (y << FRACBITS) ? 0 : eyey < ((y + 1) << FRACBITS) ? 1 : 2);
    quadrant = quy * 3 + qux;

    if (quadrant == 4)
        return;

    px[0] = vv->tlx + x * vv->c + y * vv->s;
    py[0] = vv->tly + x * vv->s - y * vv->c;
    px[1] = px[0] + vv->s;
    py[1] = py[0] - vv->c;
    px[2] = px[1] + vv->c;
    py[2] = py[1] + vv->s;
    px[3] = px[0] + vv->c;
    py[3] = py[0] + vv->s;

    idx = acorners[quadrant];
    ax = px[idx];
    ay = py[idx];
    idx = (idx + 1) & 3;
    bx = px[idx];
    by = py[idx];
    idx = (idx + 1) & 3;
    cx = px[idx];
    cy = py[idx];
    idx = (idx + 1) & 3;
    dx = px[idx];
    dy = py[idx];

    if (ay < VX_MINZ && by < VX_MINZ && cy < VX_MINZ && dy < VX_MINZ)
        return;

    // Clip cells which straddle the view plane instead of dropping the
    // entire cell. Dropped cells were the source of the alternating
    // screen-height bands seen when the camera approached a voxel.
    ay = MAX(ay, VX_MINZ);
    by = MAX(by, VX_MINZ);
    cy = MAX(cy, VX_MINZ);
    dy = MAX(dy, VX_MINZ);

    ascale = FixedDiv(projection, ay);
    bscale = FixedDiv(projection, by);
    cscale = FixedDiv(projection, cy);
    dscale = FixedDiv(projection, dy);
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
        fixed_t     scale;
        fixed_t     iscale;
        const byte  *dither = (r_ditheredlighting ? R_GetDitherColumn(ux >> FRACBITS, (spr->scale >> 5) & 255) : NULL);
        const int   screenx = ux >> FRACBITS;
        const byte  *slab = v->data + offsets1;
        const byte  *slabend = v->data + offsets2;
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
            clipbottom = MIN(clipbottom, centeryfrac - FixedMul(vv->liquidclipz - viewz, scale) - 1);

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
            const fixed_t   originaluy2 = uy2;
            const bool      side = (!!(face & (ux > bx ? bface : aface))
                                && originaluy2 >= cliptop && originaluy1 <= clipbottom);
            fixed_t         widescale = 0;

            if (!len || slab + len > slabend)
                break;

            // Clamp both ends. The top and bottom face loops also use these
            // values, so one-sided clipping can otherwise write past the
            // framebuffer when a voxel crosses a floor or ceiling edge.
            uy1 = BETWEEN(cliptop, uy1, clipbottom);
            uy2 = BETWEEN(cliptop, uy2, clipbottom);

            if ((face & F_TOP) || (face & F_BOTTOM))
            {
                if (ux > cx && bx != cx)
                    widescale = cscale + FixedMul(bscale - cscale, FixedDiv(ux - cx, bx - cx));
                else if (ux > dx && cx != dx)
                    widescale = dscale + FixedMul(cscale - dscale, FixedDiv(ux - dx, cx - dx));
                else if (dx != ax)
                    widescale = ascale + FixedMul(dscale - ascale, FixedDiv(ux - ax, dx - ax));
            }

            if (spr->mobj->flags & MF_FUZZ)
            {
                fixed_t yl = uy1;
                fixed_t yh = uy2;

                if ((face & F_TOP) && topz < 0)
                    yl = MIN(yl, MAX(((VX_ProjectScreenY(topz, widescale) - 1) | FRACMASK) + 1, cliptop));
                else if ((face & F_BOTTOM) && topz > (len << FRACBITS))
                    yh = MAX(yh, MIN(VX_ProjectScreenY(topz - (len << FRACBITS), widescale), clipbottom));

                if (side)
                    VX_DrawFuzz(screenx, yl >> FRACBITS, yh >> FRACBITS);
                else
                {
                    if (yl < uy1)
                        VX_DrawFuzz(screenx, yl >> FRACBITS, (uy1 - 1) >> FRACBITS);

                    if (yh > uy2)
                        VX_DrawFuzz(screenx, (uy2 >> FRACBITS) + 1, yh >> FRACBITS);
                }

                slab += len;
                continue;
            }

            if ((face & F_TOP) && topz < 0)
            {
                // Start on an exact screen row. Without this ceil operation,
                // the fractional top-face walk can stop one row before the
                // side-face walk starts, leaving horizontal background-colored
                // seams between otherwise adjacent voxel faces.
                fixed_t uy = MAX(((VX_ProjectScreenY(topz, widescale) - 1) | FRACMASK) + 1, cliptop);

                for (; uy < uy1; uy += FRACUNIT)
                {
                    const int   pixel = (uy >> FRACBITS) * SCREENWIDTH + screenx;

                    if (VX_DepthPass(pixel, widescale, owner))
                        dest[pixel] = VX_LitColor(spr, slab[0], dither, uy >> FRACBITS, vv->tint, dest[pixel]);
                }
            }
            else if ((face & F_BOTTOM) && topz > (len << FRACBITS))
            {
                fixed_t uy = MIN(VX_ProjectScreenY(topz - (len << FRACBITS), widescale), clipbottom);

                for (; uy > uy2; uy -= FRACUNIT)
                {
                    const int   pixel = (uy >> FRACBITS) * SCREENWIDTH + screenx;

                    if (VX_DepthPass(pixel, widescale, owner))
                        dest[pixel] = VX_LitColor(spr, slab[len - 1], dither, uy >> FRACBITS, vv->tint, dest[pixel]);
                }
            }

            if (side)
                for (fixed_t uy = ((uy1 - 1) | FRACMASK) + 1; uy <= uy2; uy += FRACUNIT)
                {
                    const int   pixel = (uy >> FRACBITS) * SCREENWIDTH + screenx;

                    if (VX_DepthPass(pixel, scale, owner))
                    {
                        const int   source = BETWEEN(0, (int)(((int64_t)((uy - originaluy1) >> FRACBITS) * iscale) >> FRACBITS),
                                        len - 1);

                        dest[pixel] = VX_LitColor(spr, slab[source], dither, uy >> FRACBITS, vv->tint, dest[pixel]);
                    }
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

        if (eyex < ((x * 2 + width) << (FRACBITS - 1)))
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

        if (eyey < ((y * 2 + height) << (FRACBITS - 1)))
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

static int VX_ClipNear(const vxpoint_t *in, vxpoint_t *out)
{
    int n = 0;

    for (int i = 0; i < 4; i++)
    {
        const vxpoint_t a = in[i];
        const vxpoint_t b = in[(i + 1) & 3];
        const bool      ain = (a.y >= VX_MINZ);
        const bool      bin = (b.y >= VX_MINZ);

        if (ain)
            out[n++] = a;

        if (ain != bin)
        {
            const int64_t   t = ((int64_t)(VX_MINZ - a.y) << FRACBITS) / (b.y - a.y);

            out[n].x = a.x + (fixed_t)(((int64_t)(b.x - a.x) * t) >> FRACBITS);
            out[n].y = VX_MINZ;
            n++;
        }
    }

    return n;
}

// Projects each column of voxels straight down onto the floor beneath it, so
// the shadow of a voxel hanging over a ledge is split across different heights.
static void VX_DrawShadow(const vissprite_t *spr)
{
    const visvoxel_t    *vv = &visvoxels[spr->voxelindex];
    const voxel_t       *v = vv->model;
    const mobj_t        *mobj = spr->mobj;
    byte                *dest = screens[0] + viewwindowy * SCREENWIDTH + viewwindowx;
    const byte          black = spr->colormap[nearestblack];
    const byte          *tint = NULL;

    if (r_shadows_translucency)
    {
        if (mobj->flags & MF_FUZZ)
            tint = &tinttab15[black << 8];
        else if (spr->fullbright)
            tint = &tinttab25[black << 8];
        else if ((mobj->flags2 & (MF2_TRANSLUCENT_33 | MF2_EXPLODING)) && r_sprites_translucency)
            tint = &tinttab25[black << 8];
        else
            tint = &tinttab40[black << 8];
    }

    if (!shadowstamps)
        shadowstamps = I_Calloc(MAXSCREENAREA, sizeof(*shadowstamps));

    if (!(++shadowstamp))
    {
        memset(shadowstamps, 0, MAXSCREENAREA * sizeof(*shadowstamps));
        shadowstamp = 1;
    }

    for (int x = 0; x < v->xsize; x++)
        for (int y = 0; y < v->ysize; y++)
        {
            const int       offset1 = v->offsets[y * v->xsize + x];
            const int       offset2 = v->offsets[(y + 1) * v->xsize + x];
            const byte      *slab = v->data + offset1;
            const byte      *slabend = v->data + offset2;
            int             bottom = 0;
            int             top = INT_MAX;
            fixed_t         px[4], py[4];
            fixed_t         cx, cy;
            fixed_t         wx, wy;
            fixed_t         floorz;
            fixed_t         relz;
            const sector_t  *sector;
            vxpoint_t       quad[4];
            vxpoint_t       clipped[8];
            int64_t         sx[8], sy[8];
            int64_t         minx = INT64_MAX, maxx = INT64_MIN;
            int             n;

            if (offset1 >= offset2)
                continue;

            while (slab + 3 <= slabend)
            {
                const int   len = slab[1];

                if (!len || slab + 3 + len > slabend)
                    break;

                bottom = MAX(bottom, slab[0] + len);
                top = MIN(top, slab[0]);
                slab += 3 + len;
            }

            if (top == INT_MAX || (vv->liquidclip && spr->gzt - (top << FRACBITS) <= vv->liquidclipz))
                continue;

            px[0] = vv->tlx + x * vv->c + y * vv->s;
            py[0] = vv->tly + x * vv->s - y * vv->c;
            px[1] = px[0] + vv->s;
            py[1] = py[0] - vv->c;
            px[2] = px[1] + vv->c;
            py[2] = py[1] + vv->s;
            px[3] = px[0] + vv->c;
            py[3] = py[0] + vv->s;

            cx = px[0] + ((vv->s + vv->c) >> 1);
            cy = py[0] + ((vv->s - vv->c) >> 1);
            wx = viewx + FixedMul(cx, viewsin) + FixedMul(cy, viewcos);
            wy = viewy - FixedMul(cx, viewcos) + FixedMul(cy, viewsin);
            sector = R_PointInSubsector(wx, wy)->sector;
            floorz = (sector->heightsec ? sector->heightsec->interpfloorheight : sector->interpfloorheight);

            if (vv->liquidclip && sector == mobj->subsector->sector)
                floorz = vv->liquidclipz;

            if ((relz = floorz - viewz) >= 0 || floorz > spr->gzt - (bottom << FRACBITS) + 8 * FRACUNIT)
                continue;

            for (int i = 0; i < 4; i++)
            {
                quad[i].x = px[i];
                quad[i].y = py[i];
            }

            if ((n = VX_ClipNear(quad, clipped)) < 3)
                continue;

            for (int i = 0; i < n; i++)
            {
                const fixed_t   scale = FixedDiv(projection, clipped[i].y);

                sx[i] = VX_ProjectScreenX(clipped[i].x, scale);
                sy[i] = VX_ProjectScreenY(relz, scale);
                minx = MIN64(minx, sx[i]);
                maxx = MAX64(maxx, sx[i]);
            }

            for (int screenx = MAX(spr->x1, (int)((minx + FRACMASK) >> FRACBITS));
                screenx <= MIN(spr->x2, (int)(maxx >> FRACBITS)); screenx++)
            {
                const int64_t   ux = (int64_t)screenx << FRACBITS;
                int64_t         lo = INT64_MAX;
                int64_t         hi = INT64_MIN;
                int             yl;
                int             yh;

                for (int i = 0; i < n; i++)
                {
                    const int   j = (i + 1) % n;
                    int64_t     x0 = sx[i], y0 = sy[i];
                    int64_t     x1 = sx[j], y1 = sy[j];

                    if (x0 > x1)
                    {
                        int64_t temp = x0;

                        x0 = x1;
                        x1 = temp;
                        temp = y0;
                        y0 = y1;
                        y1 = temp;
                    }

                    if (ux < x0 || ux > x1)
                        continue;

                    if (x0 == x1)
                    {
                        lo = MIN64(lo, MIN64(y0, y1));
                        hi = MAX64(hi, MAX64(y0, y1));
                    }
                    else
                    {
                        int64_t uy = y0 + (y1 - y0) * (ux - x0) / (x1 - x0);

                        lo = MIN64(lo, uy);
                        hi = MAX64(hi, uy);
                    }
                }

                if (lo > hi)
                    continue;

                yl = (int)MAX64(MAX(0, mceilingclip[screenx] + 1), (lo + FRACMASK) >> FRACBITS);
                yh = (int)MIN64(MIN(viewheight - 1, mfloorclip[screenx] - 1), hi >> FRACBITS);

                for (int row = yl; row <= yh; row++)
                {
                    const int   pixel = row * SCREENWIDTH + screenx;

                    if (shadowstamps[pixel] != shadowstamp && !voxeldepth[pixel].owner)
                    {
                        shadowstamps[pixel] = shadowstamp;
                        dest[pixel] = (tint ? tint[dest[pixel]] : black);
                    }
                }
            }
        }
}

void VX_DrawVoxel(const vissprite_t *spr)
{
    const visvoxel_t    *vv = &visvoxels[spr->voxelindex];
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

    if (!voxeldepthcleared)
    {
        if (!voxeldepth)
            voxeldepth = I_Calloc(MAXSCREENAREA, sizeof(*voxeldepth));
        else
            memset(voxeldepth, 0, (size_t)viewheight * SCREENWIDTH * sizeof(*voxeldepth));

        voxeldepthcleared = true;
    }

    if (vv->shadow)
        VX_DrawShadow(spr);

    eyex = v->xpivot + FixedMul(dx, c) + FixedMul(dy, s);
    eyey = v->ypivot + FixedMul(dx, s) - FixedMul(dy, c);
    VX_RecursiveDraw(spr, 0, 0, v->xsize, v->ysize);
}
