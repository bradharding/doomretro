/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "c_console.h"
#include "doomstat.h"
#include "i_swap.h"
#include "i_system.h"
#include "p_local.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAX_SPRITE_FRAMES       29
#define MINZ                    (FRACUNIT * 4)
#define BASEYCENTER             (ORIGINALHEIGHT / 2)

//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t                 pspritexscale;
fixed_t                 pspriteyscale;
fixed_t                 pspriteiscale;

static lighttable_t     **spritelights;         // killough 1/25/98 made static

// constant arrays
//  used for psprite clipping and initializing clipping
int                     negonearray[SCREENWIDTH];
int                     screenheightarray[SCREENWIDTH];

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up and range check thing_t sprites patches
spritedef_t             *sprites;
int                     numsprites;

static spriteframe_t    sprtemp[MAX_SPRITE_FRAMES];
static int              maxframe;

dboolean                r_liquid_clipsprites = r_liquid_clipsprites_default;

dboolean                r_playersprites = r_playersprites_default;

extern fixed_t          animatedliquiddiff;
extern dboolean         inhelpscreens;
extern dboolean         r_liquid_bob;
extern dboolean         r_shadows;
extern dboolean         r_translucency;
extern dboolean         skippsprinterp;

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
static void R_InstallSpriteLump(lumpinfo_t *lump, int lumpnum, unsigned int frame, char rot,
    dboolean flipped)
{
    unsigned int        rotation = (rot >= '0' && rot <= '9' ? rot - '0' : (rot >= 'A' ?
        rot - 'A' + 10 : 17));

    if (frame >= MAX_SPRITE_FRAMES || rotation > 16)
        I_Error("R_InstallSpriteLump: Bad frame characters in lump %s", lump->name);

    if ((int)frame > maxframe)
        maxframe = frame;

    if (rotation == 0)
    {
        int r;

        // the lump should be used for all rotations
        for (r = 14; r >= 0; r -= 2)
        {
            if (sprtemp[frame].lump[r] == -1)
            {
                sprtemp[frame].lump[r] = lumpnum - firstspritelump;
                if (flipped)
                    sprtemp[frame].flip |= (1 << r);
                sprtemp[frame].rotate = false;  // jff 4/24/98 if any subbed, rotless
            }
        }
        return;
    }

    // the lump is only used for one rotation
    rotation = (rotation <= 8 ? (rotation - 1) * 2 : (rotation - 9) * 2 + 1);

    if (sprtemp[frame].lump[rotation] == -1)
    {
        sprtemp[frame].lump[rotation] = lumpnum - firstspritelump;
        if (flipped)
            sprtemp[frame].flip |= (1 << rotation);
        sprtemp[frame].rotate = true;           //jff 4/24/98 only change if rot used
    }
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
// (4 chars exactly) to be used.
//
// Builds the sprite rotation matrices to account
// for horizontally flipped sprites.
//
// Will report an error if the lumps are inconsistent.
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
//
// A sprite that is flippable will have an additional
//  letter/number appended.
//
// The rotation character can be 0 to signify no rotations.
//
// 1/25/98, 1/31/98 killough : Rewritten for performance
//
// Empirically verified to have excellent hash
// properties across standard DOOM sprites:
#define R_SpriteNameHash(s) ((unsigned int)((s)[0] - ((s)[1] * 3 - (s)[3] * 2 - (s)[2]) * 2))

static void R_InitSpriteDefs(const char *const *namelist)
{
    size_t              numentries = lastspritelump - firstspritelump + 1;
    unsigned int        i;

    struct
    {
        int     index;
        int     next;
    } *hash;

    if (!numentries || !*namelist)
        return;

    // count the number of sprite names
    for (i = 0; namelist[i]; ++i);

    numsprites = (signed int)i;

    sprites = Z_Calloc(numsprites, sizeof(*sprites), PU_STATIC, NULL);

    // Create hash table based on just the first four letters of each sprite
    // killough 1/31/98
    hash = malloc(sizeof(*hash) * numentries);  // allocate hash table

    for (i = 0; i < numentries; i++)            // initialize hash table as empty
        hash[i].index = -1;

    for (i = 0; i < numentries; i++)            // Prepend each sprite to hash chain
    {                                           // prepend so that later ones win
        int     j = R_SpriteNameHash(lumpinfo[i + firstspritelump]->name) % numentries;

        hash[i].next = hash[j].index;
        hash[j].index = i;
    }

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    for (i = 0; i < (unsigned int)numsprites; ++i)
    {
        const char      *spritename = namelist[i];
        int             j = hash[R_SpriteNameHash(spritename) % numentries].index;

        if (j >= 0)
        {
            int k;

            memset(sprtemp, -1, sizeof(sprtemp));
            for (k = 0; k < MAX_SPRITE_FRAMES; ++k)
                sprtemp[k].flip = 0;

            maxframe = -1;
            do
            {
                lumpinfo_t      *lump = lumpinfo[j + firstspritelump];

                // Fast portable comparison -- killough
                // (using int pointer cast is nonportable):
                if (!((lump->name[0] ^ spritename[0]) | (lump->name[1] ^ spritename[1])
                    | (lump->name[2] ^ spritename[2]) | (lump->name[3] ^ spritename[3])))
                {
                    R_InstallSpriteLump(lump, j + firstspritelump, lump->name[4] - 'A',
                        lump->name[5], false);
                    if (lump->name[6])
                        R_InstallSpriteLump(lump, j + firstspritelump, lump->name[6] - 'A',
                           lump->name[7], true);
                }
            } while ((j = hash[j].next) >= 0);

            // check the frames that were found for completeness
            if ((sprites[i].numframes = ++maxframe))  // killough 1/31/98
            {
                int     frame;
                int     rot;

                for (frame = 0; frame < maxframe; ++frame)
                    switch (sprtemp[frame].rotate)
                    {
                        case -1:
                            // no rotations were found for that frame at all
                            break;

                        case 0:
                            // only the first rotation is needed
                            for (rot = 1; rot < 16; ++rot)
                                sprtemp[frame].lump[rot] = sprtemp[frame].lump[0];

                            // If the frame is flipped, they all should be
                            if (sprtemp[frame].flip & 1)
                                sprtemp[frame].flip = 0xFFFF;
                            break;

                        case 1:
                            // must have all 8 frames
                            for (rot = 0; rot < 16; rot += 2)
                            {
                                if (sprtemp[frame].lump[rot + 1] == -1)
                                {
                                    sprtemp[frame].lump[rot + 1] = sprtemp[frame].lump[rot];
                                    if (sprtemp[frame].flip & (1 << rot))
                                        sprtemp[frame].flip |= 1 << (rot + 1);
                                }
                                if (sprtemp[frame].lump[rot] == -1)
                                {
                                    sprtemp[frame].lump[rot] = sprtemp[frame].lump[rot + 1];
                                    if (sprtemp[frame].flip & (1 << (rot + 1)))
                                        sprtemp[frame].flip |= 1 << rot;
                                }
                            }
                            for (rot = 0; rot < 16; ++rot)
                                if (sprtemp[frame].lump[rot] == -1)
                                    I_Error("R_InitSprites: Frame %c of sprite %.8s frame %c is "
                                        "missing rotations", frame + 'A', namelist[i]);
                            break;
                    }

                for (frame = 0; frame < maxframe; ++frame)
                    if (sprtemp[frame].rotate == -1)
                    {
                        memset(&sprtemp[frame].lump, 0, sizeof(sprtemp[0].lump));
                        sprtemp[frame].flip = 0;
                        sprtemp[frame].rotate = 0;
                    }

                // allocate space for the frames present and copy sprtemp to it
                sprites[i].spriteframes = Z_Malloc(maxframe * sizeof(spriteframe_t), PU_STATIC,
                    NULL);
                memcpy(sprites[i].spriteframes, sprtemp, maxframe * sizeof(spriteframe_t));
            }
        }
    }
    free(hash); // free hash table
}

//
// GAME FUNCTIONS
//

static vissprite_t      *vissprites;
static vissprite_t      **vissprite_ptrs;
static unsigned int     num_vissprite;
static unsigned int     num_vissprite_alloc;

static vissprite_t      bloodvissprites[NUMVISSPRITES];
static int              num_bloodvissprite;

static vissprite_t      shadowvissprites[NUMVISSPRITES];
static int              num_shadowvissprite;

//
// R_InitSprites
// Called at program start.
//
void R_InitSprites(char **namelist)
{
    int i;

    for (i = 0; i < SCREENWIDTH; i++)
        negonearray[i] = -1;

    R_InitSpriteDefs(namelist);

    num_vissprite = 0;
    num_vissprite_alloc = 128;
    vissprites = malloc(num_vissprite_alloc * sizeof(vissprite_t));
    vissprite_ptrs = malloc(num_vissprite_alloc * sizeof(vissprite_t *));
}

//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites(void)
{
    if (num_vissprite >= num_vissprite_alloc)
    {
        num_vissprite_alloc += 128;
        vissprites = Z_Realloc(vissprites, num_vissprite_alloc * sizeof(vissprite_t));
        vissprite_ptrs = Z_Realloc(vissprite_ptrs, num_vissprite_alloc * sizeof(vissprite_t *));
    }

    num_vissprite = 0;
    num_bloodvissprite = 0;
    num_shadowvissprite = 0;
}

//
// R_NewVisSprite
//
static vissprite_t *R_NewVisSprite(fixed_t scale)
{
    unsigned int        pos;
    unsigned int        pos2;
    unsigned int        step;
    unsigned int        count;
    vissprite_t         *rc;
    vissprite_t         *vis;

    switch (num_vissprite)
    {
        case 0:
            rc = &vissprites[0];
            vissprite_ptrs[0] = rc;
            num_vissprite = 1;
            return rc;

        case 1:
            vis = &vissprites[0];
            rc = &vissprites[1];
            if (scale > vis->scale)
            {
                vissprite_ptrs[0] = rc;
                vissprite_ptrs[1] = vis;
            }
            else
                vissprite_ptrs[1] = rc;
            num_vissprite = 2;
            return rc;
    }

    pos = (num_vissprite + 1) >> 1;
    step = (pos + 1) >> 1;
    count = (pos << 1);
    do
    {
        fixed_t d1;
        fixed_t d2;

        vis = vissprite_ptrs[pos];
        d1 = INT_MAX;
        d2 = vis->scale;

        if (scale >= d2)
        {
            if (!pos)
                break;

            vis = vissprite_ptrs[pos - 1];
            d1 = vis->scale;

            if (scale <= d1)
                break;
        }

        pos = (scale > d1 ? MAX(0, pos - step) : MIN(pos + step, num_vissprite - 1));
        step = (step + 1) >> 1;
        count >>= 1;

        if (!count)
        {
            pos = num_vissprite;
            break;
        }
    } while (1);

    if (num_vissprite >= num_vissprite_alloc)
    {
        if (pos >= num_vissprite)
            return NULL;

        rc = vissprite_ptrs[num_vissprite - 1];
    }
    else
        rc = &vissprites[num_vissprite++];

    pos2 = num_vissprite - 1;
    do
    {
        vissprite_ptrs[pos2] = vissprite_ptrs[pos2 - 1];
    } while (--pos2 > pos);

    vissprite_ptrs[pos] = rc;

    return rc;
}

//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
int     *mfloorclip;
int     *mceilingclip;

fixed_t spryscale;
int64_t sprtopscreen;
int64_t shift;

static void R_DrawMaskedSpriteColumn(column_t *column)
{
    byte        topdelta;

    while ((topdelta = column->topdelta) != 0xFF)
    {
        int     length = column->length;

        // calculate unclipped screen coordinates for post
        int64_t topscreen = sprtopscreen + spryscale * topdelta + 1;

        dc_yl = MAX((int)((topscreen + FRACUNIT) >> FRACBITS), mceilingclip[dc_x] + 1);
        dc_yh = MIN((int)((topscreen + spryscale * length) >> FRACBITS), mfloorclip[dc_x] - 1);

        if (dc_baseclip != -1)
            dc_yh = MIN(dc_baseclip, dc_yh);

        dc_texturefrac = dc_texturemid - (topdelta << FRACBITS)
            + FixedMul((dc_yl - centery) << FRACBITS, dc_iscale);

        if (dc_texturefrac < 0)
        {
            int cnt = (FixedDiv(-dc_texturefrac, dc_iscale) + FRACUNIT - 1) >> FRACBITS;

            dc_yl += cnt;
            dc_texturefrac += cnt * dc_iscale;
        }

        {
            const fixed_t       endfrac = dc_texturefrac + (dc_yh - dc_yl) * dc_iscale;
            const fixed_t       maxfrac = length << FRACBITS;

            if (endfrac >= maxfrac)
                dc_yh -= (FixedDiv(endfrac - maxfrac - 1, dc_iscale) + FRACUNIT - 1) >> FRACBITS;
        }

        if (dc_yl <= dc_yh && dc_yh < viewheight)
        {
            dc_source = (byte *)column + 3;
            colfunc();
        }
        column = (column_t *)((byte *)column + length + 4);
    }
}

static void R_DrawMaskedBloodSplatColumn(column_t *column)
{
    byte        topdelta;

    while ((topdelta = column->topdelta) != 0xFF)
    {
        int     length = column->length;

        // calculate unclipped screen coordinates for post
        int64_t topscreen = sprtopscreen + spryscale * topdelta;

        dc_yl = MAX((int)(topscreen >> FRACBITS) + 1, mceilingclip[dc_x] + 1);
        dc_yh = MIN((int)((topscreen + spryscale * length) >> FRACBITS), mfloorclip[dc_x] - 1);

        if (dc_yl <= dc_yh && dc_yh < viewheight)
            colfunc();
        column = (column_t *)((byte *)column + length + 4);
    }
}

static void R_DrawMaskedShadowColumn(column_t *column)
{
    byte        topdelta;

    while ((topdelta = column->topdelta) != 0xFF)
    {
        int     length = column->length;

        // calculate unclipped screen coordinates for post
        int64_t topscreen = sprtopscreen + spryscale * topdelta;

        dc_yl = MAX((int)(((topscreen >> FRACBITS) + 1) / 10 + shift), mceilingclip[dc_x] + 1);
        dc_yh = MIN((int)(((topscreen + spryscale * length) >> FRACBITS) / 10 + shift),
            mfloorclip[dc_x] - 1);

        if (dc_yl <= dc_yh && dc_yh < viewheight)
            colfunc();
        column = (column_t *)((byte *)column + length + 4);
    }
}

int     fuzzpos;

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
void R_DrawVisSprite(vissprite_t *vis)
{
    fixed_t     frac = vis->startfrac;
    fixed_t     xiscale = vis->xiscale;
    fixed_t     x2 = vis->x2;
    patch_t     *patch = W_CacheLumpNum(vis->patch + firstspritelump, PU_CACHE);

    dc_colormap = vis->colormap;
    colfunc = vis->colfunc;

    dc_iscale = ABS(xiscale);
    dc_texturemid = vis->texturemid;
    if (vis->mobjflags & MF_TRANSLATION)
    {
        colfunc = transcolfunc;
        dc_translation = translationtables - 256
            + ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
    }

    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);

    if (viewplayer->fixedcolormap == INVERSECOLORMAP && r_translucency)
    {
        if (colfunc == tlcolfunc)
            colfunc = tl50colfunc;
        else if (colfunc == tlredcolfunc)
            colfunc = tlred33colfunc;
        else if (colfunc == tlgreencolfunc)
            colfunc = tlgreen33colfunc;
        else if (colfunc == tlbluecolfunc)
            colfunc = tlblue33colfunc;
        else if (colfunc == tlredwhitecolfunc1 || colfunc == tlredwhitecolfunc2)
            colfunc = tlredwhite50colfunc;
    }

    if (vis->footclip)
        dc_baseclip = ((int)sprtopscreen + FixedMul(SHORT(patch->height) << FRACBITS, spryscale)
            - FixedMul(vis->footclip, spryscale)) >> FRACBITS;
    else
        dc_baseclip = -1;

    fuzzpos = 0;

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
        R_DrawMaskedSpriteColumn((column_t *)((byte *)patch
            + LONG(patch->columnofs[frac >> FRACBITS])));

    colfunc = basecolfunc;
}

void R_DrawBloodSplatVisSprite(vissprite_t *vis)
{
    fixed_t     frac = vis->startfrac;
    fixed_t     xiscale = vis->xiscale;
    fixed_t     x2 = vis->x2;
    patch_t     *patch = W_CacheLumpNum(vis->patch + firstspritelump, PU_CACHE);

    colfunc = vis->colfunc;

    dc_blood = tinttab75 + (vis->colormap[vis->blood] << 8);

    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(vis->texturemid, spryscale);

    fuzzpos = 0;

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
        R_DrawMaskedBloodSplatColumn((column_t *)((byte *)patch
            + LONG(patch->columnofs[frac >> FRACBITS])));

    colfunc = basecolfunc;
}

void R_DrawShadowVisSprite(vissprite_t *vis)
{
    fixed_t     frac = vis->startfrac;
    fixed_t     xiscale = vis->xiscale;
    fixed_t     x2 = vis->x2;
    patch_t     *patch = W_CacheLumpNum(vis->patch + firstspritelump, PU_CACHE);

    colfunc = vis->colfunc;

    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(vis->texturemid, spryscale);
    shift = (sprtopscreen * 9 / 10) >> FRACBITS;

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
        R_DrawMaskedShadowColumn((column_t *)((byte *)patch
            + LONG(patch->columnofs[frac >> FRACBITS])));

    colfunc = basecolfunc;
}

//
// R_ProjectSprite
// Generates a vissprite for a thing
//  if it might be visible.
//
void R_ProjectSprite(mobj_t *thing)
{
    fixed_t             tx;

    fixed_t             xscale;

    int                 x1;
    int                 x2;

    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;

    dboolean            flip;

    vissprite_t         *vis;

    int                 heightsec;

    int                 flags = thing->flags;
    int                 flags2 = thing->flags2;
    int                 frame = thing->frame;
    int                 type = thing->type;

    // transform the origin point
    fixed_t             tr_x;
    fixed_t             tr_y;

    fixed_t             gxt;
    fixed_t             gyt;
    fixed_t             gzt;

    fixed_t             tz;

    angle_t             rot = 0;

    sector_t            *sector = thing->subsector->sector;

    fixed_t             fx;
    fixed_t             fy;
    fixed_t             fz;
    fixed_t             fangle;

    fixed_t             offset;
    fixed_t             topoffset;

    // [AM] Interpolate between current and last position, if prudent.
    if (!vid_capfps
        // Don't interpolate if the mobj did something
        // that would necessitate turning it off for a tic.
        && thing->interp
        // Don't interpolate during a paused state.
        && !paused && !menuactive && !consoleactive)
    {
        fx = thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic);
        fy = thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic);
        fz = thing->oldz + FixedMul(thing->z - thing->oldz, fractionaltic);
        fangle = R_InterpolateAngle(thing->oldangle, thing->angle, fractionaltic);
    }
    else
    {
        fx = thing->x;
        fy = thing->y;
        fz = thing->z;
        fangle = thing->angle;
    }

    tr_x = fx - viewx;
    tr_y = fy - viewy;

    gxt = FixedMul(tr_x, viewcos);
    gyt = -FixedMul(tr_y, viewsin);

    tz = gxt - gyt;

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    xscale = FixedDiv(projection, tz);

    gxt = -FixedMul(tr_x, viewsin);
    gyt = FixedMul(tr_y, viewcos);
    tx = -(gyt + gxt);

    // too far off the side?
    if (ABS(tx) > (tz << 2))
        return;

    // decide which patch to use for sprite relative to player
    sprdef = &sprites[thing->sprite];
    sprframe = &sprdef->spriteframes[frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        angle_t ang = R_PointToAngle(fx, fy);

        if (sprframe->lump[0] == sprframe->lump[1])
            rot = (ang - fangle + (angle_t)(ANG45 / 2) * 9) >> 28;
        else
            rot = (ang - fangle + (angle_t)(ANG45 / 2) * 9 - (angle_t)(ANG180 / 16)) >> 28;
        lump = sprframe->lump[rot];
        flip = ((dboolean)(sprframe->flip & (1 << rot)) || (flags2 & MF2_MIRRORED));
    }
    else
    {
        // use single rotation for all views
        lump = sprframe->lump[0];
        flip = ((dboolean)(sprframe->flip & 1) || (flags2 & MF2_MIRRORED));
    }

    if (thing->state->dehacked)
    {
        offset = spriteoffset[lump];
        topoffset = spritetopoffset[lump];
    }
    else
    {
        offset = newspriteoffset[lump];
        topoffset = newspritetopoffset[lump];
    }

    // calculate edges of the shape
    tx -= (flip ? spritewidth[lump] - offset : offset);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;

    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx + spritewidth[lump], xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    gzt = fz + topoffset;

    if (fz > viewz + FixedDiv(viewheight << FRACBITS, xscale)
        || gzt < viewz - FixedDiv((viewheight << FRACBITS) - viewheight, xscale))
        return;

    // killough 3/27/98: exclude things totally separated
    // from the viewer, by either water or fake ceilings
    // killough 4/11/98: improve sprite clipping for underwater/fake ceilings
    heightsec = sector->heightsec;

    if (heightsec != -1)   // only clip things which are in special sectors
    {
        int     phs = viewplayer->mo->subsector->sector->heightsec;

        if (phs != -1 && viewz < sectors[phs].floorheight ?
            thing->z >= sectors[heightsec].floorheight :
            gzt < sectors[heightsec].floorheight)
            return;
        if (phs != -1 && viewz > sectors[phs].ceilingheight ?
            gzt < sectors[heightsec].ceilingheight &&
            viewz >= sectors[heightsec].ceilingheight :
            thing->z >= sectors[heightsec].ceilingheight)
            return;
    }

    // store information in a vissprite
    if (!(vis = R_NewVisSprite(xscale)))
        return;

    // killough 3/27/98: save sector for special clipping later
    vis->heightsec = heightsec;

    vis->mobjflags = flags;
    vis->mobjflags2 = flags2;
    vis->type = type;
    vis->scale = xscale;
    vis->gx = fx;
    vis->gy = fy;
    vis->gz = fz;
    vis->gzt = gzt;
    vis->blood = thing->blood;

    if ((flags & MF_FUZZ) && (menuactive || paused || consoleactive))
        vis->colfunc = R_DrawPausedFuzzColumn;
    else
        vis->colfunc = thing->colfunc;

    // foot clipping
    if ((flags2 & MF2_FEETARECLIPPED) && fz <= sector->interpfloorheight + FRACUNIT
        && heightsec == -1 && r_liquid_clipsprites)
    {
        fixed_t clipfeet = MIN((spriteheight[lump] >> FRACBITS) / 4, 10) << FRACBITS;

        vis->texturemid = gzt - viewz - clipfeet;

        if ((flags2 & MF2_NOLIQUIDBOB) && r_liquid_bob && isliquid[sector->floorpic])
            clipfeet += animatedliquiddiff;

        vis->footclip = clipfeet;
    }
    else
    {
        vis->footclip = 0;
        vis->texturemid = gzt - viewz;
    }

    vis->x1 = MAX(0, x1);
    vis->x2 = MIN(x2, viewwidth - 1);

    if (flip)
    {
        vis->startfrac = spritewidth[lump] - 1;
        vis->xiscale = -FixedDiv(FRACUNIT, xscale);
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = FixedDiv(FRACUNIT, xscale);
    }

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale * (vis->x1 - x1);
    vis->patch = lump;

    // get light level
    if (fixedcolormap)
        vis->colormap = fixedcolormap;          // fixed map
    else if ((frame & FF_FULLBRIGHT) && (rot <= 3 || rot >= 7))
        vis->colormap = fullcolormap;           // full bright
    else                                        // diminished light
        vis->colormap = spritelights[BETWEEN(0, xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)];
}

void R_ProjectBloodSplat(mobj_t *thing)
{
    fixed_t             tx;

    fixed_t             xscale;

    int                 x1;
    int                 x2;

    int                 lump;

    vissprite_t         *vis;

    fixed_t             fx = thing->x;
    fixed_t             fy = thing->y;
    fixed_t             fz;

    fixed_t             width;

    // transform the origin point
    fixed_t             tr_x = fx - viewx;
    fixed_t             tr_y = fy - viewy;

    fixed_t             gxt = FixedMul(tr_x, viewcos);
    fixed_t             gyt = -FixedMul(tr_y, viewsin);

    fixed_t             tz = gxt - gyt;

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    xscale = FixedDiv(projection, tz);

    if (xscale < FRACUNIT / 2)
        return;

    gxt = -FixedMul(tr_x, viewsin);
    gyt = FixedMul(tr_y, viewcos);
    tx = -(gyt + gxt);

    // too far off the side?
    if (ABS(tx) > (tz << 2))
        return;

    // decide which patch to use for sprite relative to player
    lump = sprites[SPR_BLD2].spriteframes[thing->frame].lump[0];
    width = spritewidth[lump];

    // calculate edges of the shape
    tx -= (width >> 1);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;

    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx + width, xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    // store information in a vissprite
    vis = &bloodvissprites[num_bloodvissprite++];

    vis->type = MT_BLOODSPLAT;
    vis->scale = xscale;
    vis->gx = fx;
    vis->gy = fy;
    fz = thing->subsector->sector->interpfloorheight;
    vis->gz = fz;
    vis->gzt = fz + 1;
    vis->blood = thing->blood;

    if ((thing->flags & MF_FUZZ) && (menuactive || paused || consoleactive))
        vis->colfunc = R_DrawPausedFuzzColumn;
    else
        vis->colfunc = thing->colfunc;

    vis->texturemid = fz + 1 - viewz;

    vis->x1 = MAX(0, x1);
    vis->x2 = MIN(x2, viewwidth - 1);

    vis->startfrac = 0;
    vis->xiscale = FixedDiv(FRACUNIT, xscale);

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale * (vis->x1 - x1);
    vis->patch = lump;

    // get light level
    if (fixedcolormap)
        vis->colormap = fixedcolormap;          // fixed map
    else                                        // diminished light
        vis->colormap = spritelights[BETWEEN(0, xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)];
}

void R_ProjectShadow(mobj_t *thing)
{
    fixed_t             tx;

    fixed_t             xscale;

    int                 x1;
    int                 x2;

    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;

    dboolean            flip;

    vissprite_t         *vis;

    fixed_t             fx = thing->x;
    fixed_t             fy = thing->y;
    fixed_t             fz = thing->subsector->sector->interpfloorheight
                             + thing->shadow->info->shadowoffset;

    // transform the origin point
    fixed_t             tr_x = fx - viewx;
    fixed_t             tr_y = fy - viewy;

    fixed_t             gxt = FixedMul(tr_x, viewcos);
    fixed_t             gyt = -FixedMul(tr_y, viewsin);

    fixed_t             tz = gxt - gyt;

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    xscale = FixedDiv(projection, tz);

    if (xscale < FRACUNIT / 2)
        return;

    gxt = -FixedMul(tr_x, viewsin);
    gyt = FixedMul(tr_y, viewcos);
    tx = -(gyt + gxt);

    // too far off the side?
    if (ABS(tx) > (tz << 2))
        return;

    // decide which patch to use for sprite relative to player
    sprdef = &sprites[thing->sprite];
    sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        angle_t rot;
        angle_t ang = R_PointToAngle(fx, fy);

        if (sprframe->lump[0] == sprframe->lump[1])
            rot = (ang - thing->angle + (angle_t)(ANG45 / 2) * 9) >> 28;
        else
            rot = (ang - thing->angle + (angle_t)(ANG45 / 2) * 9 - (angle_t)(ANG180 / 16)) >> 28;
        lump = sprframe->lump[rot];
        flip = ((dboolean)(sprframe->flip & (1 << rot)) || (thing->flags2 & MF2_MIRRORED));
    }
    else
    {
        // use single rotation for all views
        lump = sprframe->lump[0];
        flip = ((dboolean)(sprframe->flip & 1) || (thing->flags2 & MF2_MIRRORED));
    }

    // calculate edges of the shape
    tx -= (flip ? spritewidth[lump] - newspriteoffset[lump] : newspriteoffset[lump]);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;

    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx + spritewidth[lump], xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    // store information in a vissprite
    vis = &shadowvissprites[num_shadowvissprite++];

    vis->mobjflags = 0;
    vis->mobjflags2 = 0;
    vis->type = MT_SHADOW;
    vis->scale = xscale;
    vis->gx = fx;
    vis->gy = fy;
    vis->gz = fz;
    vis->gzt = fz;
    vis->colfunc = thing->colfunc;
    vis->texturemid = fz - viewz;

    vis->x1 = MAX(0, x1);
    vis->x2 = MIN(x2, viewwidth - 1);

    if (flip)
    {
        vis->startfrac = spritewidth[lump] - 1;
        vis->xiscale = -FixedDiv(FRACUNIT, xscale);
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = FixedDiv(FRACUNIT, xscale);
    }

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale * (vis->x1 - x1);
    vis->patch = lump;
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
// killough 9/18/98: add lightlevel as parameter, fixing underwater lighting
void R_AddSprites(sector_t *sec, int lightlevel)
{
    mobj_t      *thing;
    short       floorpic = sec->floorpic;

    spritelights = scalelight[BETWEEN(0, (lightlevel >> LIGHTSEGSHIFT) + extralight * LIGHTBRIGHT,
        LIGHTLEVELS - 1)];

    // Handle all things in sector.
    if (fixedcolormap || isliquid[floorpic] || floorpic == skyflatnum || !r_shadows)
        for (thing = sec->thinglist; thing; thing = thing->snext)
        {
            if (thing->type != MT_SHADOW)
                thing->projectfunc(thing);
        }
    else
        for (thing = sec->thinglist; thing; thing = thing->snext)
            thing->projectfunc(thing);
}

//
// R_DrawPSprite
//
static dboolean bflash;

static void R_DrawPSprite(pspdef_t *psp, dboolean invisibility)
{
    fixed_t             tx;
    int                 x1, x2;
    spritenum_t         spr;
    spritedef_t         *sprdef;
    long                frame;
    spriteframe_t       *sprframe;
    int                 lump;
    dboolean            flip;
    vissprite_t         *vis;
    vissprite_t         avis;
    state_t             *state;

    // decide which patch to use
    state = psp->state;
    spr = state->sprite;
    sprdef = &sprites[spr];
    frame = state->frame;
    sprframe = &sprdef->spriteframes[frame & FF_FRAMEMASK];

    lump = sprframe->lump[0];
    flip = (dboolean)(sprframe->flip & 1);

    // calculate edges of the shape
    tx = psp->sx - ORIGINALWIDTH / 2 * FRACUNIT - (state->dehacked ? spriteoffset[lump] :
        newspriteoffset[lump]);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, pspritexscale)) >> FRACBITS;

    // off the right side
    if (x1 > viewwidth)
        return;

    tx += spritewidth[lump];
    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx, pspritexscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    // store information in a vissprite
    vis = &avis;
    vis->mobjflags = 0;
    vis->mobjflags2 = 0;
    vis->texturemid = (BASEYCENTER << FRACBITS) + FRACUNIT / 4 - (psp->sy - spritetopoffset[lump]);
    vis->x1 = MAX(0, x1);
    vis->x2 = MIN(x2, viewwidth - 1);
    vis->scale = pspriteyscale;
    vis->blood = 0;
    vis->footclip = 0;

    if (flip)
    {
        vis->xiscale = -pspriteiscale;
        vis->startfrac = spritewidth[lump] - 1;
    }
    else
    {
        vis->xiscale = pspriteiscale;
        vis->startfrac = 0;
    }

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale * (vis->x1 - x1);

    vis->patch = lump;

    if (invisibility)
    {
        // shadow draw
        vis->colfunc = psprcolfunc;
        vis->colormap = NULL;
    }
    else
    {
        if (spr == SPR_SHT2 && (!frame || frame >= 8))
            vis->colfunc = R_DrawSuperShotgunColumn;
        else
        {
            void (*colfuncs[])(void) =
            {
                /* n/a      */ NULL,
                /* SPR_SHTG */ basecolfunc,
                /* SPR_PUNG */ basecolfunc,
                /* SPR_PISG */ basecolfunc,
                /* SPR_PISF */ tlcolfunc,
                /* SPR_SHTF */ tlcolfunc,
                /* SPR_SHT2 */ tlredwhitecolfunc1,
                /* SPR_CHGG */ basecolfunc,
                /* SPR_CHGF */ tlredwhitecolfunc2,
                /* SPR_MISG */ basecolfunc,
                /* SPR_MISF */ tlcolfunc,
                /* SPR_SAWG */ basecolfunc,
                /* SPR_PLSG */ basecolfunc,
                /* SPR_PLSF */ tlcolfunc,
                /* SPR_BFGG */ basecolfunc,
                /* SPR_BFGF */ tlcolfunc
            };

            vis->colfunc = (bflash && spr <= SPR_BFGF && !state->dehacked ? colfuncs[spr] :
                basecolfunc);
        }
        if (fixedcolormap)
            vis->colormap = fixedcolormap;      // fixed color
        else
        {
            if (bflash || (frame & FF_FULLBRIGHT))
                vis->colormap = fullcolormap;   // full bright
            else
            {
                int lightnum = (viewplayer->mo->subsector->sector->lightlevel >> OLDLIGHTSEGSHIFT)
                    + extralight * OLDLIGHTBRIGHT;

                vis->colormap = psprscalelight[BETWEEN(0, lightnum, OLDLIGHTLEVELS - 1)]
                    [BETWEEN(0, lightnum + 16, OLDMAXLIGHTSCALE - 1)];
            }
        }
    }

    //e6y: interpolation for weapon bobbing
    if (!vid_capfps)
    {
        typedef struct interpolate_s
        {
            int x1;
            int x1_prev;
            int texturemid;
            int texturemid_prev;
            int lump;
        } psp_interpolate_t;

        static psp_interpolate_t        psp_inter;

        if (realframe)
        {
            psp_inter.x1 = psp_inter.x1_prev;
            psp_inter.texturemid = psp_inter.texturemid_prev;
        }

        psp_inter.x1_prev = vis->x1;
        psp_inter.texturemid_prev = vis->texturemid;

        if (lump == psp_inter.lump && !skippsprinterp)
        {
            int deltax = vis->x2 - vis->x1;

            vis->x1 = psp_inter.x1 + FixedMul(fractionaltic, vis->x1 - psp_inter.x1);
            vis->x2 = vis->x1 + deltax;
            vis->texturemid = psp_inter.texturemid
                + FixedMul(fractionaltic, vis->texturemid - psp_inter.texturemid);
        }
        else
        {
            skippsprinterp = false;
            psp_inter.x1 = vis->x1;
            psp_inter.texturemid = vis->texturemid;
            psp_inter.lump = lump;
        }
    }

    R_DrawVisSprite(vis);
}

//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites(void)
{
    int         i;
    int         invisibility = viewplayer->powers[pw_invisibility];
    pspdef_t    *psp;

    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;

    // add all active psprites
    if (invisibility > 128 || (invisibility & 8))
    {
        V_FillRect(1, viewwindowx, viewwindowy, viewwidth, viewheight, 251);
        for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
            if (psp->state)
                R_DrawPSprite(psp, true);
        if (menuactive || paused || consoleactive)
            R_DrawPausedFuzzColumns();
        else
            R_DrawFuzzColumns();
    }
    else
    {
        bflash = false;
        for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
            if (psp->state && (psp->state->frame & FF_FULLBRIGHT))
                bflash = true;
        for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
            if (psp->state)
                R_DrawPSprite(psp, false);
    }
}

//
// R_DrawBloodSprite
//
static void R_DrawBloodSprite(vissprite_t *spr)
{
    drawseg_t   *ds;
    int         clipbot[SCREENWIDTH];
    int         cliptop[SCREENWIDTH];
    int         x;
    int         x1 = spr->x1;
    int         x2 = spr->x2;

    // [RH] Quickly reject sprites with bad x ranges.
    if (x1 > x2)
        return;

    for (x = x1; x <= x2; x++)
        clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.
    for (ds = ds_p; ds-- > drawsegs;)
    {
        int     r1;
        int     r2;

        // determine if the drawseg obscures the sprite
        if (ds->x1 > x2 || ds->x2 < x1 || (!ds->silhouette && !ds->maskedtexturecol))
            continue;       // does not cover sprite

        if (MAX(ds->scale1, ds->scale2) < spr->scale
            || (MIN(ds->scale1, ds->scale2) < spr->scale
            && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
            continue;       // seg is behind sprite

        r1 = MAX(ds->x1, x1);
        r2 = MIN(ds->x2, x2);

        // clip this piece of the sprite
        // killough 3/27/98: optimized and made much shorter
        if ((ds->silhouette & SIL_BOTTOM) && spr->gz < ds->bsilheight)  // bottom sil
            for (x = r1; x <= r2; x++)
                if (clipbot[x] == -2)
                    clipbot[x] = ds->sprbottomclip[x];

        if ((ds->silhouette & SIL_TOP) && spr->gzt > ds->tsilheight)    // top sil
            for (x = r1; x <= r2; x++)
                if (cliptop[x] == -2)
                    cliptop[x] = ds->sprtopclip[x];
    }

    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = x1; x <= x2; x++)
    {
        if (clipbot[x] == -2)
            clipbot[x] = viewheight;

        if (cliptop[x] == -2)
            cliptop[x] = -1;
    }

    mfloorclip = clipbot;
    mceilingclip = cliptop;
    if (spr->type == MT_BLOODSPLAT)
        R_DrawBloodSplatVisSprite(spr);
    else
        R_DrawVisSprite(spr);
}

//
// R_DrawShadowSprite
//
static void R_DrawShadowSprite(vissprite_t *spr)
{
    drawseg_t   *ds;
    int         clipbot[SCREENWIDTH];
    int         cliptop[SCREENWIDTH];
    int         x;
    int         x1 = spr->x1;
    int         x2 = spr->x2;

    // [RH] Quickly reject sprites with bad x ranges.
    if (x1 > x2)
        return;

    for (x = x1; x <= x2; x++)
        clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.
    for (ds = ds_p; ds-- > drawsegs;)
    {
        int     r1;
        int     r2;

        // determine if the drawseg obscures the sprite
        if (ds->x1 > x2 || ds->x2 < x1 || (!ds->silhouette && !ds->maskedtexturecol))
            continue;       // does not cover sprite

        if (MAX(ds->scale1, ds->scale2) < spr->scale
            || (MIN(ds->scale1, ds->scale2) < spr->scale
            && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
            continue;       // seg is behind sprite

        r1 = MAX(ds->x1, x1);
        r2 = MIN(ds->x2, x2);

        // clip this piece of the sprite
        // killough 3/27/98: optimized and made much shorter
        if ((ds->silhouette & SIL_BOTTOM) && spr->gz < ds->bsilheight)  // bottom sil
            for (x = r1; x <= r2; x++)
                if (clipbot[x] == -2)
                    clipbot[x] = ds->sprbottomclip[x];

        if ((ds->silhouette & SIL_TOP) && spr->gzt > ds->tsilheight)    // top sil
            for (x = r1; x <= r2; x++)
                if (cliptop[x] == -2)
                    cliptop[x] = ds->sprtopclip[x];
    }

    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = x1; x <= x2; x++)
    {
        if (clipbot[x] == -2)
            clipbot[x] = viewheight;

        if (cliptop[x] == -2)
            cliptop[x] = -1;
    }

    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawShadowVisSprite(spr);
}

static void R_DrawSprite(vissprite_t *spr)
{
    drawseg_t   *ds;
    int         clipbot[SCREENWIDTH];
    int         cliptop[SCREENWIDTH];
    int         x;
    int         x1 = spr->x1;
    int         x2 = spr->x2;

    if (x1 > x2)
        return;

    for (x = x1; x <= x2; x++)
        clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale is the clip seg.
    for (ds = ds_p; ds-- > drawsegs;)
    {
        int     r1;
        int     r2;

        // determine if the drawseg obscures the sprite
        if (ds->x1 > x2 || ds->x2 < x1 || (!ds->silhouette && !ds->maskedtexturecol))
            continue;       // does not cover sprite

        if (MAX(ds->scale1, ds->scale2) < spr->scale
            || (MIN(ds->scale1, ds->scale2) < spr->scale
            && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
        {
            // masked mid texture?
            if (ds->maskedtexturecol)
            {
                r1 = MAX(ds->x1, x1);
                r2 = MIN(ds->x2, x2);
                R_RenderMaskedSegRange(ds, r1, r2);
            }

            // seg is behind sprite
            continue;
        }

        r1 = MAX(ds->x1, x1);
        r2 = MIN(ds->x2, x2);

        // clip this piece of the sprite
        // killough 3/27/98: optimized and made much shorter
        if ((ds->silhouette & SIL_BOTTOM) && spr->gz < ds->bsilheight)  // bottom sil
            for (x = r1; x <= r2; x++)
                if (clipbot[x] == -2)
                    clipbot[x] = ds->sprbottomclip[x];

        if ((ds->silhouette & SIL_TOP) && spr->gzt > ds->tsilheight)    // top sil
            for (x = r1; x <= r2; x++)
                if (cliptop[x] == -2)
                    cliptop[x] = ds->sprtopclip[x];
    }

    // killough 3/27/98:
    // Clip the sprite against deep water and/or fake ceilings.
    // killough 4/9/98: optimize by adding mh
    // killough 4/11/98: improve sprite clipping for underwater/fake ceilings
    // killough 11/98: fix disappearing sprites
    if (spr->heightsec != -1)  // only things in specially marked sectors
    {
        fixed_t     h, mh;
        int         phs = viewplayer->mo->subsector->sector->heightsec;

        if ((mh = sectors[spr->heightsec].interpfloorheight) > spr->gz
            && (h = centeryfrac - FixedMul(mh -= viewz, spr->scale)) >= 0
            && (h >>= FRACBITS) < viewheight)
            if (mh <= 0 || (phs != -1 && viewz > sectors[phs].interpfloorheight))
            {                          // clip bottom
                for (x = x1; x <= x2; x++)
                    if (clipbot[x] == -2 || h < clipbot[x])
                        clipbot[x] = h;
            }
            else                        // clip top
                if (phs != -1 && viewz <= sectors[phs].interpfloorheight)       // killough 11/98
                    for (x = x1; x <= x2; x++)
                        if (cliptop[x] == -2 || h > cliptop[x])
                            cliptop[x] = h;

        if ((mh = sectors[spr->heightsec].ceilingheight) < spr->gzt
            && (h = centeryfrac - FixedMul(mh - viewz, spr->scale)) >= 0
            && (h >>= FRACBITS) < viewheight)
            if (phs != -1 && viewz >= sectors[phs].ceilingheight)
            {                         // clip bottom
                for (x = x1; x <= x2; x++)
                    if (clipbot[x] == -2 || h < clipbot[x])
                        clipbot[x] = h;
            }
            else                       // clip top
                for (x = x1; x <= x2; x++)
                    if (cliptop[x] == -2 || h > cliptop[x])
                        cliptop[x] = h;
    }

    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1; x <= x2; x++)
    {
        if (clipbot[x] == -2)
            clipbot[x] = viewheight;

        if (cliptop[x] == -2)
            cliptop[x] = -1;
    }

    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite(spr);
}

//
// R_DrawMasked
//
void R_DrawMasked(void)
{
    drawseg_t   *ds;
    int         i;

    // draw all blood splats
    for (i = num_bloodvissprite; --i >= 0;)
        R_DrawBloodSprite(&bloodvissprites[i]);

    // draw all shadows
    for (i = num_shadowvissprite; --i >= 0;)
        R_DrawShadowSprite(&shadowvissprites[i]);

    // draw all other vissprites back to front
    for (i = num_vissprite; --i >= 0;)
        R_DrawSprite(vissprite_ptrs[i]);

    // render any remaining masked mid textures
    for (ds = ds_p; ds-- > drawsegs;)
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    if (r_playersprites && !inhelpscreens)
        R_DrawPlayerSprites();
}
