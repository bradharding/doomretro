/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 1999 Lee Killough.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

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

typedef struct drawseg_xrange_item_s
{
    short                       x1, x2;
    drawseg_t                   *user;
} drawseg_xrange_item_t;

typedef struct drawsegs_xrange_s
{
    drawseg_xrange_item_t       *items;
    int                         count;
} drawsegs_xrange_t;

#define DS_RANGES_COUNT         3

static drawsegs_xrange_t        drawsegs_xranges[DS_RANGES_COUNT];

static drawseg_xrange_item_t    *drawsegs_xrange;
static unsigned int             drawsegs_xrange_size = 0;
static int                      drawsegs_xrange_count = 0;

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
static char                     *spritename;

extern int                      screensize;
extern boolean                  inhelpscreens;

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void R_InstallSpriteLump(int lump, unsigned int frame, unsigned int rotation, boolean flipped)
{
    if (frame >= MAX_SPRITE_FRAMES || rotation > 8)
        I_Error("R_InstallSpriteLump: Bad frame characters in lump %i", lump);

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
                sprtemp[frame].lump[r] = lump - firstspritelump;
                sprtemp[frame].flip[r] = (byte)flipped;
                sprtemp[frame].rotate = false;
            }
        }
        return;
    }

    // the lump is only used for one rotation
    if (sprtemp[frame].lump[--rotation] == -1)
    {
        sprtemp[frame].lump[rotation] = lump - firstspritelump;
        sprtemp[frame].flip[rotation] = (byte)flipped;
        sprtemp[frame].rotate = true;
    }
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant.
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
void R_InitSpriteDefs(char **namelist)
{
    char        **check;
    int         i;
    int         l;
    int         frame;
    int         rotation;
    int         start;
    int         end;
    int         patched;

    // count the number of sprite names
    check = namelist;
    while (*check != NULL)
        check++;

    numsprites = check - namelist;

    if (!numsprites)
        return;

    sprites = (spritedef_t *)Z_Malloc(numsprites * sizeof(*sprites), PU_STATIC, NULL);

    start = firstspritelump - 1;
    end = lastspritelump + 1;

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    for (i = 0; i < numsprites; i++)
    {
        spritename = namelist[i];
        memset(sprtemp, -1, sizeof(sprtemp));

        maxframe = -1;

        // scan the lumps,
        //  filling in the frames for whatever is found
        for (l = start + 1; l < end; l++)
        {
            if (!strncasecmp(lumpinfo[l].name, spritename, 4))
            {
                frame = lumpinfo[l].name[4] - 'A';
                rotation = lumpinfo[l].name[5] - '0';

                patched = (modifiedgame ? W_GetNumForName(lumpinfo[l].name) : l);

                R_InstallSpriteLump(patched, frame, rotation, false);

                if (lumpinfo[l].name[6])
                {
                    frame = lumpinfo[l].name[6] - 'A';
                    rotation = lumpinfo[l].name[7] - '0';
                    R_InstallSpriteLump(l, frame, rotation, true);
                }
            }
        }

        // check the frames that were found for completeness
        if (maxframe == -1)
        {
            sprites[i].numframes = 0;
            continue;
        }

        maxframe++;

        for (frame = 0; frame < maxframe; frame++)
        {
            switch ((int)sprtemp[frame].rotate)
            {
                case -1:
                    // no rotations were found for that frame at all
                    I_Error("R_InitSprites: No patches found "
                            "for %s frame %c", spritename, frame + 'A');
                    break;

                case 0:
                    // only the first rotation is needed
                    break;

                case 1:
                    // must have all 8 frames
                    for (rotation = 0; rotation < 8; rotation++)
                        if (sprtemp[frame].lump[rotation] == -1)
                            I_Error("R_InitSprites: Sprite %s frame %c "
                                    "is missing rotations",
                                    spritename, frame + 'A');
                    break;
            }
        }

        // allocate space for the frames present and copy sprtemp to it
        sprites[i].numframes = maxframe;
        sprites[i].spriteframes =
            (spriteframe_t *)Z_Malloc(maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
        memcpy(sprites[i].spriteframes, sprtemp, maxframe * sizeof(spriteframe_t));
    }
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
        size_t  num_vissprite_alloc_prev = num_vissprite_alloc;

        num_vissprite_alloc = (num_vissprite_alloc ? num_vissprite_alloc * 2 : 128);
        vissprites = realloc(vissprites, num_vissprite_alloc * sizeof(*vissprites));

        //e6y: set all fields to zero
        memset(vissprites + num_vissprite_alloc_prev, 0,
            (num_vissprite_alloc - num_vissprite_alloc_prev) * sizeof(*vissprites));
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
fixed_t sprtopscreen;

void R_DrawMaskedColumn(column_t *column)
{
    while (column->topdelta != 0xff)
    {
        int topscreen;

        if (column->length == 0)
        {
            column = (column_t *)((byte *)column + 4);
            continue;
        }

        // calculate unclipped screen coordinates for post
        topscreen = sprtopscreen + spryscale * column->topdelta + 1;

        dc_yl = (topscreen + FRACUNIT) >> FRACBITS;
        dc_yh = (topscreen + spryscale * column->length) >> FRACBITS;

        if (dc_yh >= mfloorclip[dc_x])
            dc_yh = mfloorclip[dc_x] - 1;
        if (dc_yl <= mceilingclip[dc_x])
            dc_yl = mceilingclip[dc_x] + 1;

        dc_texturefrac = dc_texturemid - (column->topdelta << FRACBITS)
                         + FixedMul((dc_yl - centery) << FRACBITS, dc_iscale);

        if (dc_texturefrac < 0)
        {
            int cnt = (FixedDiv(-dc_texturefrac, dc_iscale) + FRACUNIT - 1) >> FRACBITS;

            dc_yl += cnt;
            dc_texturefrac += cnt * dc_iscale;
        }

        {
            const fixed_t endfrac = dc_texturefrac + (dc_yh - dc_yl) * dc_iscale;
            const fixed_t maxfrac = column->length << FRACBITS;

            if (endfrac >= maxfrac)
                dc_yh -= (FixedDiv(endfrac - maxfrac - 1, dc_iscale) + FRACUNIT - 1) >> FRACBITS;
        }

        dc_source = (byte *)column + 3;

        if (dc_yl >= 0 && dc_yh < viewheight && dc_yl <= dc_yh)
            colfunc();

        column = (column_t *)((byte *)column + column->length + 4);
    }
}

boolean megasphere;
int     fuzzpos;

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
void R_DrawVisSprite(vissprite_t *vis)
{
    column_t    *column;
    int         texturecolumn;
    fixed_t     frac;
    patch_t     *patch = (patch_t *)W_CacheLumpNum(vis->patch + firstspritelump, PU_CACHE);

    dc_colormap = vis->colormap;
    colfunc = vis->colfunc;
    fuzzpos = 0;

    dc_iscale = FixedDiv(FRACUNIT, vis->scale);
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);

    if (viewplayer->fixedcolormap == INVERSECOLORMAP)
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

    megasphere = (vis->type == MT_MEGA);

    for (dc_x = vis->x1; dc_x <= vis->x2; dc_x++, frac += vis->xiscale)
    {
        texturecolumn = frac >> FRACBITS;
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[texturecolumn]));
        R_DrawMaskedColumn(column);
    }

    colfunc = basecolfunc;
}

//
// R_ProjectSprite
// Generates a vissprite for a thing
//  if it might be visible.
//
void R_ProjectSprite(mobj_t *thing)
{
    fixed_t             tr_x;
    fixed_t             tr_y;

    fixed_t             gxt;
    fixed_t             gyt;
    fixed_t             gzt;

    fixed_t             tx;
    fixed_t             tz;

    fixed_t             xscale;

    int                 x1;
    int                 x2;

    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;

    unsigned int        rot;
    boolean             flip;

    vissprite_t         *vis;

    angle_t             ang;
    fixed_t             iscale;

    // transform the origin point
    tr_x = thing->x - viewx;
    tr_y = thing->y - viewy;

    gxt = FixedMul(tr_x, viewcos);
    gyt = -FixedMul(tr_y, viewsin);

    tz = gxt - gyt;

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    xscale = FixedDiv(projection, tz);

    if (thing->type == MT_BLOODSPLAT && xscale < FRACUNIT / 3)
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
        ang = R_PointToAngle(thing->x, thing->y);
        rot = (ang - thing->angle + (unsigned)(ANG45 / 2) * 9) >> 29;
        lump = sprframe->lump[rot];
        flip = (boolean)sprframe->flip[rot];
    }
    else
    {
        // use single rotation for all views
        lump = sprframe->lump[0];
        flip = (boolean)sprframe->flip[0];
    }

    if (thing->flags2 & MF2_MIRRORED)
        flip = true;

    // calculate edges of the shape
    tx -= (flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump]);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;

    tx += spritewidth[lump];
    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx, xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    gzt = thing->z + spritetopoffset[lump];

    if (thing->z > viewz + FixedDiv(viewheight << FRACBITS, xscale)
        || gzt < viewz - FixedDiv((viewheight << FRACBITS) - viewheight, xscale))
        return;

    // store information in a vissprite
    vis = R_NewVisSprite();
    vis->mobjflags = thing->flags;
    vis->mobjflags2 = thing->flags2;
    vis->colfunc = thing->colfunc;
    vis->type = thing->type;
    vis->scale = xscale;
    vis->gx = thing->x;
    vis->gy = thing->y;
    vis->gz = thing->z;
    vis->gzt = gzt;
    vis->texturemid = vis->gzt - viewz;
    vis->x1 = MAX(0, x1);
    vis->x2 = (x2 >= viewwidth ? viewwidth - 1 : x2);
    iscale = FixedDiv(FRACUNIT, xscale);

    if (flip)
    {
        vis->startfrac = spritewidth[lump] - 1;
        vis->xiscale = -iscale;
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale * (vis->x1 - x1);
    vis->patch = lump;

    // get light level
    if (fixedcolormap)
        vis->colormap = fixedcolormap;                          // fixed map
    else if (thing->frame & FF_FULLBRIGHT)
        vis->colormap = colormaps;                              // full bright
    else
        vis->colormap = spritelights[MIN(((xscale * 160 / centerx) >> LIGHTSCALESHIFT),
                                         MAXLIGHTSCALE - 1)];   // diminished light
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites(sector_t *sec)
{
    mobj_t      *thing;
    int         lightnum;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
        return;

    // Well, now it will be done.
    sec->validcount = validcount;

    lightnum = (sec->lightlevel >> LIGHTSEGSHIFT) + extralight * LIGHTBRIGHT;
    spritelights = scalelight[lightnum >= LIGHTLEVELS ? LIGHTLEVELS - 1 : MAX(0, lightnum)];

    // Handle all things in sector.
    for (thing = sec->thinglist; thing; thing = thing->snext)
        R_ProjectSprite(thing);
}

//
// R_DrawPSprite
//
static boolean flash;

void R_DrawPSprite(pspdef_t *psp)
{
    fixed_t             tx;
    int                 x1, x2;
    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;
    boolean             flip;
    vissprite_t         *vis;
    vissprite_t         avis;
    state_t             *state;
    int                 invisibility;

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

    // decide which patch to use
    state = psp->state;
    sprdef = &sprites[state->sprite];
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

    invisibility = viewplayer->powers[pw_invisibility];
    if (invisibility > 128 || (invisibility & 8))
        vis->colfunc = psprcolfunc;                             // shadow draw
    else
        vis->colfunc = (flash ? colfuncs[state->sprite] : basecolfunc);

    if (fixedcolormap)
        vis->colormap = fixedcolormap;                          // fixed color
    else
    {
        if (flash || (state->frame & FF_FULLBRIGHT))
            vis->colormap = colormaps;                          // full bright
        else
        {
            // local light
            int lightnum = (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT)
                           + extralight * LIGHTBRIGHT + 8;

            vis->colormap = spritelights[MIN(lightnum, MAXLIGHTSCALE)];
        }
    }

    if (state == &states[S_DSGUN])
        vis->colfunc = R_DrawSuperShotgunColumn;

    R_DrawVisSprite(vis);
}

//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites(void)
{
    int         i;
    int         lightnum;
    int         invisibility = viewplayer->powers[pw_invisibility];
    pspdef_t    *psp;

    // get light level
    lightnum = (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT) + extralight * LIGHTBRIGHT;
    spritelights = scalelight2[lightnum >= LIGHTLEVELS ? LIGHTLEVELS - 1 : MAX(0, lightnum)];

    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;

    // add all active psprites
    flash = false;
    for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
        if (psp->state && (psp->state->frame & FF_FULLBRIGHT))
            flash = true;
    if (invisibility > 128 || (invisibility & 8))
    {
        V_FillRect(1, viewwindowx, viewwindowy, viewwidth, viewheight, 251);
        for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
            if (psp->state)
                R_DrawPSprite(psp);
        R_DrawFuzzColumns();
    }
    else
    {
        for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
            if (psp->state)
                R_DrawPSprite(psp);
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
        int             i;

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
        int     i = num_vissprite;

        // If we need to allocate more pointers for the vissprites,
        // allocate as many as were allocated for sprites -- killough
        // killough 9/22/98: allocate twice as many
        if (num_vissprite_ptrs < num_vissprite * 2)
        {
            free(vissprite_ptrs);
            vissprite_ptrs = (vissprite_t **)malloc((num_vissprite_ptrs = num_vissprite_alloc * 2)
                * sizeof *vissprite_ptrs);
        }

        while (--i >= 0)
            vissprite_ptrs[i] = vissprites + i;

        // killough 9/22/98: replace qsort with merge sort, since the keys
        // are roughly in order to begin with, due to BSP rendering.
        msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

//
// R_DrawSprite
//
void R_DrawSprite(vissprite_t *spr, boolean drawmaskedtextures)
{
    drawseg_t   *ds;
    int         clipbot[MAXWIDTH];
    int         cliptop[MAXWIDTH];
    int         x;
    int         r1;
    int         r2;
    fixed_t     scale;
    fixed_t     lowscale;

    if (spr->x1 > spr->x2)
        return;

    for (x = spr->x1; x <= spr->x2; x++)
        clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.

    // e6y: optimization

    if (drawsegs_xrange_size)
    {
        const drawseg_xrange_item_t     *last = &drawsegs_xrange[drawsegs_xrange_count - 1];
        drawseg_xrange_item_t           *curr = &drawsegs_xrange[-1];

        while (++curr <= last)
        {
            // determine if the drawseg obscures the sprite
            if (curr->x1 > spr->x2 || curr->x2 < spr->x1)
                continue;           // does not cover sprite

            ds = curr->user;

            r1 = MAX(ds->x1, spr->x1);
            r2 = MIN(ds->x2, spr->x2);

            lowscale = MIN(ds->scale1, ds->scale2);
            scale = MAX(ds->scale1, ds->scale2);

            if (scale < spr->scale ||
                (lowscale < spr->scale &&  !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
            {
                // masked mid texture?
                if (drawmaskedtextures && ds->maskedtexturecol)
                    R_RenderMaskedSegRange(ds, r1, r2);
                // seg is behind sprite
                continue;
            }

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
    }
    else
    {
        for (ds = ds_p; ds-- > drawsegs;)
        {
            // determine if the drawseg obscures the sprite
            if (ds->x1 > spr->x2 || ds->x2 < spr->x1 || (!ds->silhouette && !ds->maskedtexturecol))
                continue;           // does not cover sprite

            r1 = MAX(ds->x1, spr->x1);
            r2 = MIN(ds->x2, spr->x2);

            lowscale = MIN(ds->scale1, ds->scale2);
            scale = MAX(ds->scale1, ds->scale2);

            if (scale < spr->scale ||
                (lowscale < spr->scale && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
            {
                // masked mid texture?
                if (drawmaskedtextures && ds->maskedtexturecol)
                    R_RenderMaskedSegRange(ds, r1, r2);
                // seg is behind sprite
                continue;
            }

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
    }

    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1; x <= spr->x2; x++)
    {
        if (clipbot[x] == -2 || clipbot[x] > viewheight)
            clipbot[x] = viewheight;

        if (cliptop[x] == -2 || cliptop[x] < 0)
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
    int         cx = SCREENWIDTH / 2;

    R_SortVisSprites();

    // e6y
    // Reducing of cache misses in the following R_DrawSprite()
    // Makes sense for scenes with huge amount of drawsegs.
    // ~12% of speed improvement on epic.wad map05
    for (i = 0; i < DS_RANGES_COUNT; i++)
        drawsegs_xranges[i].count = 0;

    if (num_vissprite > 0)
    {
        int i;

        if (drawsegs_xrange_size < maxdrawsegs)
        {
            drawsegs_xrange_size = 2 * maxdrawsegs;
            for (i = 0; i < DS_RANGES_COUNT; i++)
            {
                drawsegs_xranges[i].items = realloc(
                    drawsegs_xranges[i].items,
                    drawsegs_xrange_size * sizeof(drawsegs_xranges[i].items[0]));
            }
        }
        for (ds = ds_p; ds-- > drawsegs;)
        {
            if (ds->silhouette || ds->maskedtexturecol)
            {
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].x1 = ds->x1;
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].x2 = ds->x2;
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].user = ds;

                // e6y: ~13% of speed improvement on sunder.wad map10
                if (ds->x1 < cx)
                {
                    drawsegs_xranges[1].items[drawsegs_xranges[1].count] =
                        drawsegs_xranges[0].items[drawsegs_xranges[0].count];
                    drawsegs_xranges[1].count++;
                }
                if (ds->x2 >= cx)
                {
                    drawsegs_xranges[2].items[drawsegs_xranges[2].count] =
                        drawsegs_xranges[0].items[drawsegs_xranges[0].count];
                    drawsegs_xranges[2].count++;
                }

                drawsegs_xranges[0].count++;
            }
        }
    }

    // draw all blood splats first, front to back
    for (i = 0; i < num_vissprite; i++)
    {
        vissprite_t *spr = vissprite_ptrs[i];

        if (spr->type == MT_BLOODSPLAT)
        {
            if (spr->x2 < cx)
            {
                drawsegs_xrange = drawsegs_xranges[1].items;
                drawsegs_xrange_count = drawsegs_xranges[1].count;
            }
            else if (spr->x1 >= cx)
            {
                drawsegs_xrange = drawsegs_xranges[2].items;
                drawsegs_xrange_count = drawsegs_xranges[2].count;
            }
            else
            {
                drawsegs_xrange = drawsegs_xranges[0].items;
                drawsegs_xrange_count = drawsegs_xranges[0].count;
            }

            R_DrawSprite(spr, false);
        }
    }

    // draw all other vissprites, back to front
    for (i = num_vissprite; --i >= 0;)
    {
        vissprite_t *spr = vissprite_ptrs[i];

        if (spr->type != MT_BLOODSPLAT)
        {
            if (spr->x2 < cx)
            {
                drawsegs_xrange = drawsegs_xranges[1].items;
                drawsegs_xrange_count = drawsegs_xranges[1].count;
            }
            else if (spr->x1 >= cx)
            {
                drawsegs_xrange = drawsegs_xranges[2].items;
                drawsegs_xrange_count = drawsegs_xranges[2].count;
            }
            else
            {
                drawsegs_xrange = drawsegs_xranges[0].items;
                drawsegs_xrange_count = drawsegs_xranges[0].count;
            }

            R_DrawSprite(spr, true);
        }
    }

    // render any remaining masked mid textures
    for (ds = ds_p; ds-- > drawsegs;)
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    if (!inhelpscreens)
        R_DrawPlayerSprites();
}
