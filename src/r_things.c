/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

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

#define MINZ                    (FRACUNIT * 4)
#define BASEYCENTER             (ORIGINALHEIGHT / 2)

//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t                         pspritexscale;
fixed_t                         pspriteyscale;
fixed_t                         pspriteiscale;

static lighttable_t             **spritelights;         // killough 1/25/98 made static

// constant arrays
//  used for psprite clipping and initializing clipping
int                             negonearray[SCREENWIDTH];
int                             screenheightarray[SCREENWIDTH];

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up and range check thing_t sprites patches
spritedef_t                     *sprites;
int                             numsprites;

#define MAX_SPRITE_FRAMES       29

static spriteframe_t            sprtemp[MAX_SPRITE_FRAMES];
static int                      maxframe;

boolean                         footclip = FOOTCLIP_DEFAULT;

extern boolean                  inhelpscreens;
extern boolean                  translucency;
extern boolean                  dehacked;
extern boolean                  shadows;

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void R_InstallSpriteLump(lumpinfo_t *lump, int lumpnum, unsigned int frame,
                         unsigned int rotation, boolean flipped)
{
    if (frame >= MAX_SPRITE_FRAMES || rotation > 8)
        I_Error("R_InstallSpriteLump: Bad frame characters in lump %s", lump->name);

    if ((int)frame > maxframe)
        maxframe = frame;

    if (rotation == 0)
    {
        int r;

        // the lump should be used for all rotations
        for (r = 0; r < 8; r++)
        {
            if (sprtemp[frame].lump[r] == -1)
            {
                sprtemp[frame].lump[r] = lumpnum - firstspritelump;
                sprtemp[frame].flip[r] = (byte)flipped;
                sprtemp[frame].rotate = false;
            }
        }
        return;
    }

    // the lump is only used for one rotation
    if (sprtemp[frame].lump[--rotation] == -1)
    {
        sprtemp[frame].lump[rotation] = lumpnum - firstspritelump;
        sprtemp[frame].flip[rotation] = (byte)flipped;
        sprtemp[frame].rotate = true;
    }
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
// (4 chars exactly) to be used.
//
// Builds the sprite rotation matrixes to account
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
// properties across standard Doom sprites:
#define R_SpriteNameHash(s) ((unsigned int)((s)[0] - ((s)[1] * 3 - (s)[3] * 2 - (s)[2]) * 2))

void R_InitSpriteDefs(char **namelist)
{
    size_t              numentries = lastspritelump - firstspritelump + 1;
    unsigned int        i;

    struct {
        int     index;
        int     next;
    } *hash;

    if (!numentries || !*namelist)
        return;

    // count the number of sprite names
    for (i = 0; namelist[i]; i++);

    numsprites = (signed int)i;

    sprites = Z_Malloc(numsprites * sizeof(*sprites), PU_STATIC, NULL);

    // Create hash table based on just the first four letters of each sprite
    // killough 1/31/98
    hash = malloc(sizeof(*hash) * numentries);  // allocate hash table

    for (i = 0; i < numentries; i++)            // initialize hash table as empty
        hash[i].index = -1;

    for (i = 0; i < numentries; i++)            // Prepend each sprite to hash chain
    {                                           // prepend so that later ones win
        int     j = R_SpriteNameHash(lumpinfo[i + firstspritelump].name) % numentries;

        hash[i].next = hash[j].index;
        hash[j].index = i;
    }

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    for (i = 0; i < (unsigned int)numsprites; i++)
    {
        const char      *spritename = namelist[i];
        int             j = hash[R_SpriteNameHash(spritename) % numentries].index;

        if (j >= 0)
        {
            memset(sprtemp, -1, sizeof(sprtemp));
            maxframe = -1;
            do
            {
                lumpinfo_t      *lump = &lumpinfo[j + firstspritelump];

                // Fast portable comparison -- killough
                // (using int pointer cast is nonportable):
                if (!((lump->name[0] ^ spritename[0]) |
                      (lump->name[1] ^ spritename[1]) |
                      (lump->name[2] ^ spritename[2]) |
                      (lump->name[3] ^ spritename[3])))
                {
                    R_InstallSpriteLump(lump, j + firstspritelump, lump->name[4] - 'A', 
                        lump->name[5] - '0', false);
                    if (lump->name[6])
                        R_InstallSpriteLump(lump, j + firstspritelump, lump->name[6] - 'A',
                           lump->name[7] - '0', true);
                }
            } while ((j = hash[j].next) >= 0);

            // check the frames that were found for completeness
            if ((sprites[i].numframes = ++maxframe))  // killough 1/31/98
            {
                int     frame;

                for (frame = 0; frame < maxframe; frame++)
                    switch ((int)sprtemp[frame].rotate)
                    {
                        case -1:
                            // no rotations were found for that frame at all
                            break;

                        case 0:
                            // only the first rotation is needed
                            break;

                        case 1:
                            // must have all 8 frames
                        {
                            int rotation;

                            for (rotation = 0; rotation < 8; rotation++)
                                if (sprtemp[frame].lump[rotation] == -1)
                                    I_Error("R_InitSprites: Sprite %.8s frame %c is missing rotations",
                                        namelist[i], frame + 'A');
                            break;
                        }
                    }
                    // allocate space for the frames present and copy sprtemp to it
                    sprites[i].spriteframes = Z_Malloc(maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
                    memcpy(sprites[i].spriteframes, sprtemp, maxframe * sizeof(spriteframe_t));
            }
        }
    }
    free(hash);             // free hash table
}

//
// GAME FUNCTIONS
//
static vissprite_t      *vissprites, **vissprite_ptrs;          // killough
static int              num_vissprite, num_vissprite_alloc, num_vissprite_ptrs;

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
}

//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites(void)
{
    num_vissprite = 0;          // killough
}

//
// R_NewVisSprite
//
vissprite_t *R_NewVisSprite(void)
{
    if (num_vissprite >= num_vissprite_alloc)           // killough
    {
        num_vissprite_alloc = (num_vissprite_alloc ? num_vissprite_alloc * 2 : 128);
        vissprites = realloc(vissprites, num_vissprite_alloc * sizeof(*vissprites));
    }
    return (vissprites + num_vissprite++);
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
    while (column->topdelta != 0xff)
    {
        int     topdelta = column->topdelta;
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
    while (column->topdelta != 0xff)
    {
        int     length = column->length;

        // calculate unclipped screen coordinates for post
        int64_t topscreen = sprtopscreen + spryscale * column->topdelta + 1;

        dc_yl = MAX((int)((topscreen + FRACUNIT) >> FRACBITS), mceilingclip[dc_x] + 1);
        dc_yh = MIN((int)((topscreen + spryscale * length) >> FRACBITS), mfloorclip[dc_x] - 1);

        if (dc_yl <= dc_yh && dc_yh < viewheight)
        {
            dc_source = (byte *)column + 3;
            colfunc();
        }
        column = (column_t *)((byte *)column + length + 4);
    }
}

static void R_DrawMaskedShadowColumn(column_t *column)
{
    while (column->topdelta != 0xff)
    {
        int     length = column->length;

        // calculate unclipped screen coordinates for post
        int64_t topscreen = sprtopscreen + spryscale * column->topdelta + 1;

        dc_yl = MAX((int)(((topscreen + FRACUNIT) >> FRACBITS) / 10 + shift), mceilingclip[dc_x] + 1);
        dc_yh = MIN((int)(((topscreen + spryscale * length) >> FRACBITS) / 10 + shift),
            mfloorclip[dc_x] - 1);

        if (dc_yl <= dc_yh && dc_yh < viewheight)
        {
            dc_source = (byte *)column + 3;
            colfunc();
        }
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

    if (viewplayer->fixedcolormap == INVERSECOLORMAP && translucency)
    {
        if (colfunc == tlcolfunc)
            colfunc = tl50colfunc;
        else if (colfunc == tlredcolfunc)
            colfunc = tlred50colfunc;
        else if (colfunc == tlgreencolfunc)
            colfunc = tlgreen50colfunc;
        else if (colfunc == tlbluecolfunc)
            colfunc = tlblue50colfunc;
        else if (colfunc == tlredwhitecolfunc)
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

    dc_blood = vis->colormap[vis->blood] << 8;

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
    fixed_t             gzt;
    fixed_t             tx;

    fixed_t             xscale;

    int                 x1;
    int                 x2;

    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;

    boolean             flip;

    vissprite_t         *vis;

    int                 flags = thing->flags;
    int                 flags2 = thing->flags2;
    int                 frame = thing->frame;
    int                 type = thing->type;

    // transform the origin point
    fixed_t             tr_x;
    fixed_t             tr_y;

    fixed_t             gxt;
    fixed_t             gyt;

    fixed_t             tz;

    unsigned int        rot = 0;

    sector_t            *sector = thing->subsector->sector;

    fixed_t             interpx;
    fixed_t             interpy;
    fixed_t             interpz;
    fixed_t             interpangle;

    // [AM] Interpolate between current and last position, if prudent.
    if (!capfps
        // Don't interpolate if the mobj did something 
        // that would necessitate turning it off for a tic.
        && thing->interp
        // Don't interpolate during a paused state.
        && !paused && !menuactive && !consoleactive)
    {
        interpx = thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic);
        interpy = thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic);
        interpz = thing->oldz + FixedMul(thing->z - thing->oldz, fractionaltic);
        interpangle = R_InterpolateAngle(thing->oldangle, thing->angle, fractionaltic);
    }
    else
    {
        interpx = thing->x;
        interpy = thing->y;
        interpz = thing->z;
        interpangle = thing->angle;
    }

    tr_x = interpx - viewx;
    tr_y = interpy - viewy;

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

    // choose a different rotation based on player view
    if (sprframe->rotate)
        rot = (R_PointToAngle(interpx, interpy) - interpangle + (unsigned int)(ANG45 / 2) * 9) >> 29;

    lump = sprframe->lump[rot];
    flip = (!!sprframe->flip[rot] || (flags2 & MF2_MIRRORED));

    // calculate edges of the shape
    tx -= (flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump]);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;

    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx + spritewidth[lump], xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    gzt = interpz + spritetopoffset[lump];

    if (interpz > viewz + FixedDiv(viewheight << FRACBITS, xscale)
        || gzt < viewz - FixedDiv((viewheight << FRACBITS) - viewheight, xscale))
        return;

    // store information in a vissprite
    vis = R_NewVisSprite();
    vis->mobjflags = flags;
    vis->mobjflags2 = flags2;
    vis->type = type;
    vis->scale = xscale;
    vis->gx = interpx;
    vis->gy = interpy;
    vis->gz = interpz;
    vis->gzt = gzt;
    vis->blood = thing->blood;

    if ((flags & MF_FUZZ) && (menuactive || paused || consoleactive))
        vis->colfunc = R_DrawPausedFuzzColumn;
    else
        vis->colfunc = thing->colfunc;

    // foot clipping
    if ((flags2 & MF2_FEETARECLIPPED) && interpz <= sector->interpfloorheight + FRACUNIT && footclip)
    {
        fixed_t clipfeet = MIN((spriteheight[lump] >> FRACBITS) / 4, 10) << FRACBITS;

        vis->texturemid = gzt - viewz - clipfeet;

        if ((flags2 & MF2_NOFLOATBOB) && sector->animate != INT_MAX)
            clipfeet += sector->animate;

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
        vis->colormap = colormaps;              // full bright
    else                                        // diminished light
        vis->colormap = spritelights[BETWEEN(0, xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)];
}

void R_ProjectBloodSplat(mobj_t *thing)
{
    fixed_t             gzt;
    fixed_t             tx;

    fixed_t             xscale;

    int                 x1;
    int                 x2;

    int                 lump;

    vissprite_t         *vis;

    fixed_t             fx = thing->x;
    fixed_t             fy = thing->y;
    fixed_t             fz = thing->subsector->sector->interpfloorheight;

    int                 flags = thing->flags;
    int                 flags2 = thing->flags2;

    boolean             flip = (flags2 & MF2_MIRRORED);

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

    if (xscale < FRACUNIT / 3)
        return;

    gxt = -FixedMul(tr_x, viewsin);
    gyt = FixedMul(tr_y, viewcos);
    tx = -(gyt + gxt);

    // too far off the side?
    if (ABS(tx) > (tz << 2))
        return;

    // decide which patch to use for sprite relative to player
    lump = sprites[SPR_BLD2].spriteframes[thing->frame].lump[0];

    // calculate edges of the shape
    tx -= (flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump]);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;

    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx + spritewidth[lump], xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    gzt = fz + spritetopoffset[lump];

    // store information in a vissprite
    vis = R_NewVisSprite();
    vis->mobjflags = flags;
    vis->mobjflags2 = flags2;
    vis->type = MT_BLOODSPLAT;
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

    vis->texturemid = gzt - viewz;

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

    boolean             flip;

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

    unsigned int        rot = 0;

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    xscale = FixedDiv(projection, tz);

    if (xscale < FRACUNIT / 3)
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

    // choose a different rotation based on player view
    if (sprframe->rotate)
        rot = (R_PointToAngle(fx, fy) - thing->angle + (unsigned int)(ANG45 / 2) * 9) >> 29;

    lump = sprframe->lump[rot];
    flip = (!!sprframe->flip[rot] || (thing->flags2 & MF2_MIRRORED));

    // calculate edges of the shape
    tx -= (flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump]);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;

    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx + spritewidth[lump], xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    // store information in a vissprite
    vis = R_NewVisSprite();
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
void R_AddSprites(sector_t *sec)
{
    mobj_t      *thing;
    short       floorpic = sec->floorpic;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
        return;

    // Well, now it will be done.
    sec->validcount = validcount;

    spritelights = scalelight[BETWEEN(0, (sec->lightlevel >> LIGHTSEGSHIFT)
        + extralight * LIGHTBRIGHT, LIGHTLEVELS - 1)];

    // Handle all things in sector.
    if (fixedcolormap || isliquid[floorpic] || floorpic == skyflatnum || !shadows)
    {
        for (thing = sec->thinglist; thing; thing = thing->snext)
        {
            mobjtype_t  type = thing->type;

            if (type == MT_BLOODSPLAT)
                R_ProjectBloodSplat(thing);
            else if (type != MT_SHADOW)
                R_ProjectSprite(thing);
        }
    }
    else
    {
        for (thing = sec->thinglist; thing; thing = thing->snext)
        {
            mobjtype_t  type = thing->type;

            if (type == MT_BLOODSPLAT)
                R_ProjectBloodSplat(thing); 
            else if (type == MT_SHADOW)
                R_ProjectShadow(thing);
            else
                R_ProjectSprite(thing);
        }
    }
}

//
// R_DrawPSprite
//
static boolean  bflash;

static void R_DrawPSprite(pspdef_t *psp, boolean invisibility)
{
    fixed_t             tx;
    int                 x1, x2;
    spritenum_t         spr;
    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;
    boolean             flip;
    vissprite_t         *vis;
    vissprite_t         avis;
    state_t             *state;

    // decide which patch to use
    state = psp->state;
    spr = state->sprite;
    sprdef = &sprites[spr];
    sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];

    // calculate edges of the shape
    tx = psp->sx - ORIGINALWIDTH / 2 * FRACUNIT - spriteoffset[lump];
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
        if (state == &states[S_DSGUN])
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
                /* SPR_SHT2 */ tlredwhitecolfunc,
                /* SPR_CHGG */ basecolfunc,
                /* SPR_CHGF */ tlcolfunc,
                /* SPR_MISG */ basecolfunc,
                /* SPR_MISF */ tlcolfunc,
                /* SPR_SAWG */ basecolfunc,
                /* SPR_PLSG */ basecolfunc,
                /* SPR_PLSF */ tlcolfunc,
                /* SPR_BFGG */ basecolfunc,
                /* SPR_BFGF */ tlcolfunc
            };

            vis->colfunc = (bflash && spr <= SPR_BFGF && !dehacked ? colfuncs[spr] : basecolfunc);
        }
        if (fixedcolormap)
            vis->colormap = fixedcolormap;      // fixed color
        else
        {
            if (bflash || (state->frame & FF_FULLBRIGHT))
                vis->colormap = colormaps;      // full bright
            else
            {
                // local light
                int lightnum = (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT)
                    + extralight * LIGHTBRIGHT;

                vis->colormap = psprscalelight[BETWEEN(0, lightnum, LIGHTLEVELS - 1)]
                    [BETWEEN(0, lightnum + 8, MAXLIGHTSCALE - 1)];
            }
        }
    }

    R_DrawVisSprite(vis);
}

//
// R_DrawPlayerSprites
//
static void R_DrawPlayerSprites(void)
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
// R_SortVisSprites
//
// Rewritten by Lee Killough to avoid using unnecessary
// linked lists, and to use faster sorting algorithm.
//
#define bcopyp(d, s, n) memcpy(d, s, (n) * sizeof(void *))

// killough 9/2/98: merge sort
static void msort(vissprite_t **s, vissprite_t **t, int n)
{
    if (n >= 16)
    {
        int             n1 = n / 2;
        int             n2 = n - n1;
        vissprite_t     **s1 = s;
        vissprite_t     **s2 = s + n1;
        vissprite_t     **d = t;

        msort(s1, t, n1);
        msort(s2, t, n2);

        while ((*s1)->scale > (*s2)->scale ? (*d++ = *s1++, --n1) : (*d++ = *s2++, --n2));

        if (n2)
            bcopyp(d, s2, n2);
        else
            bcopyp(d, s1, n1);

        bcopyp(s, t, n);
    }
    else
    {
        int     i;

        for (i = 1; i < n; i++)
        {
            vissprite_t *temp = s[i];

            if (s[i - 1]->scale < temp->scale)
            {
                int     j = i;

                while ((s[j] = s[j - 1])->scale < temp->scale && --j);
                s[j] = temp;
            }
        }
    }
}

void R_SortVisSprites(void)
{
    if (num_vissprite)
    {
        int     i;

        // If we need to allocate more pointers for the vissprites,
        // allocate as many as were allocated for sprites -- killough
        // killough 9/22/98: allocate twice as many
        if (num_vissprite_ptrs < num_vissprite * 2)
        {
            free(vissprite_ptrs);
            vissprite_ptrs = (vissprite_t **)malloc((num_vissprite_ptrs = num_vissprite_alloc * 2)
                * sizeof(*vissprite_ptrs));
        }

        for (i = num_vissprite; --i >= 0;)
        {
            vissprite_t     *spr = vissprites + i;

            spr->drawn = false;
            vissprite_ptrs[i] = spr;
        }

        // killough 9/22/98: replace qsort with merge sort, since the keys
        // are roughly in order to begin with, due to BSP rendering.
        msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

//
// R_DrawBloodSprite
//
void R_DrawBloodSprite(vissprite_t *spr)
{
    if (spr->x1 > spr->x2)
        return;
    else
    {
        drawseg_t       *ds;
        int             clipbot[SCREENWIDTH];
        int             cliptop[SCREENWIDTH];
        int             x;
        int             r1;
        int             r2;

        for (x = spr->x1; x <= spr->x2; x++)
            clipbot[x] = cliptop[x] = -2;

        // Scan drawsegs from end to start for obscuring segs.
        // The first drawseg that has a greater scale
        //  is the clip seg.
        for (ds = ds_p - 1; ds >= drawsegs; ds--)
        {
            // determine if the drawseg obscures the sprite
            if (ds->x1 > spr->x2 || ds->x2 < spr->x1 || (!ds->silhouette && !ds->maskedtexturecol))
                continue;       // does not cover sprite

            if (MAX(ds->scale1, ds->scale2) < spr->scale
                || (MIN(ds->scale1, ds->scale2) < spr->scale
                && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
                continue;       // seg is behind sprite

            r1 = MAX(ds->x1, spr->x1);
            r2 = MIN(ds->x2, spr->x2);

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
        for (x = spr->x1; x <= spr->x2; x++)
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
}

//
// R_DrawShadowSprite
//
void R_DrawShadowSprite(vissprite_t *spr)
{
    if (spr->x1 > spr->x2)
        return;
    else
    {
        drawseg_t       *ds;
        int             clipbot[SCREENWIDTH];
        int             cliptop[SCREENWIDTH];
        int             x;
        int             r1;
        int             r2;

        for (x = spr->x1; x <= spr->x2; x++)
            clipbot[x] = cliptop[x] = -2;

        // Scan drawsegs from end to start for obscuring segs.
        // The first drawseg that has a greater scale
        //  is the clip seg.
        for (ds = ds_p - 1; ds >= drawsegs; ds--)
        {
            // determine if the drawseg obscures the sprite
            if (ds->x1 > spr->x2 || ds->x2 < spr->x1 || (!ds->silhouette && !ds->maskedtexturecol))
                continue;       // does not cover sprite

            if (MAX(ds->scale1, ds->scale2) < spr->scale
                || (MIN(ds->scale1, ds->scale2) < spr->scale
                && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
                continue;       // seg is behind sprite

            r1 = MAX(ds->x1, spr->x1);
            r2 = MIN(ds->x2, spr->x2);

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
        for (x = spr->x1; x <= spr->x2; x++)
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
}

void R_DrawSprite(vissprite_t *spr)
{
    if (spr->x1 > spr->x2)
        return;
    else
    {
        drawseg_t       *ds;
        int             clipbot[SCREENWIDTH];
        int             cliptop[SCREENWIDTH];
        int             x;
        int             r1;
        int             r2;

        for (x = spr->x1; x <= spr->x2; x++)
            clipbot[x] = cliptop[x] = -2;

        // Scan drawsegs from end to start for obscuring segs.
        // The first drawseg that has a greater scale is the clip seg.
        for (ds = ds_p - 1; ds >= drawsegs; ds--)
        {
            // determine if the drawseg obscures the sprite
            if (ds->x1 > spr->x2 || ds->x2 < spr->x1 || (!ds->silhouette && !ds->maskedtexturecol))
                continue;       // does not cover sprite

            if (MAX(ds->scale1, ds->scale2) < spr->scale
                || (MIN(ds->scale1, ds->scale2) < spr->scale
                && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
            {
                // masked mid texture?
                if (ds->maskedtexturecol)
                {
                    r1 = MAX(ds->x1, spr->x1);
                    r2 = MIN(ds->x2, spr->x2);
                    R_RenderMaskedSegRange(ds, r1, r2);
                }

                // seg is behind sprite
                continue;
            }

            r1 = MAX(ds->x1, spr->x1);
            r2 = MIN(ds->x2, spr->x2);

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
        for (x = spr->x1; x <= spr->x2; x++)
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
}

//
// R_DrawMasked
//
void R_DrawMasked(void)
{
    drawseg_t   *ds;
    int         i;

    R_SortVisSprites();

    // draw all sprites with MF2_DRAWFIRST flag (blood splats and pools of blood)
    for (i = 0; i < num_vissprite; i++)
    {
        vissprite_t     *spr = vissprite_ptrs[i];

        if (spr->mobjflags2 & MF2_DRAWFIRST)
        {
            spr->drawn = true;
            R_DrawBloodSprite(spr);
        }
    }

    // draw all shadows
    for (i = num_vissprite; --i >= 0;)
    {
        vissprite_t     *spr = vissprite_ptrs[i];

        if (spr->type == MT_SHADOW)
        {
            spr->drawn = true;
            R_DrawShadowSprite(spr);
        }
    }

    // draw all other vissprites, back to front
    for (i = num_vissprite; --i >= 0;)
    {
        vissprite_t     *spr = vissprite_ptrs[i];

        if (!spr->drawn)
            R_DrawSprite(spr);
    }

    // render any remaining masked mid textures
    for (ds = ds_p; ds-- > drawsegs;)
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    if (!inhelpscreens)
        R_DrawPlayerSprites();
}
