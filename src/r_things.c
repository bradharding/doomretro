/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

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

#include <stdio.h>
#include <stdlib.h>


#include "doomdef.h"

#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

#include "doomstat.h"

#include "v_video.h"



#define MINZ            (FRACUNIT * 4)
#define BASEYCENTER     (ORIGINALHEIGHT / 2)



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

lighttable_t            **spritelights;

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

spriteframe_t           sprtemp[29];
int                     maxframe;
char                    *spritename;




//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void R_InstallSpriteLump(int lump, unsigned frame, unsigned rotation, boolean flipped)
{
    int         r;

    if (frame >= 29 || rotation > 8)
        I_Error("R_InstallSpriteLump: "
                "Bad frame characters in lump %i", lump);

    if ((int)frame > maxframe)
        maxframe = frame;

    if (rotation == 0)
    {
        // the lump should be used for all rotations
        if (sprtemp[frame].rotate == false)
            I_Error("R_InitSprites: Sprite %s frame %c has "
                    "multip rot=0 lump", spritename, 'A' + frame);

        if (sprtemp[frame].rotate == true)
            I_Error("R_InitSprites: Sprite %s frame %c has rotations "
                    "and a rot=0 lump", spritename, 'A' + frame);

        sprtemp[frame].rotate = false;
        for (r = 0; r < 8; r++)
        {
            sprtemp[frame].lump[r] = lump - firstspritelump;
            sprtemp[frame].flip[r] = (byte)flipped;
        }
        return;
    }

    // the lump is only used for one rotation
    if (sprtemp[frame].rotate == false)
        I_Error("R_InitSprites: Sprite %s frame %c has rotations "
                "and a rot=0 lump", spritename, 'A' + frame);

    sprtemp[frame].rotate = true;

    // make 0 based
    rotation--;
    if (sprtemp[frame].lump[rotation] != -1)
        I_Error("R_InitSprites: Sprite %s : %c : %c "
                "has two lumps mapped to it",
                spritename, 'A' + frame, '1' + rotation);

    sprtemp[frame].lump[rotation] = lump - firstspritelump;
    sprtemp[frame].flip[rotation] = (byte)flipped;
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

                if (modifiedgame)
                    patched = W_GetNumForName(lumpinfo[l].name);
                else
                    patched = l;

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
vissprite_t             vissprites[MAXVISSPRITES];
vissprite_t             *vissprite_p;
int                     newvissprite;



//
// R_InitSprites
// Called at program start.
//
void R_InitSprites(char **namelist)
{
    int         i;

    for (i = 0; i < SCREENWIDTH; i++)
    {
        negonearray[i] = -1;
    }

    R_InitSpriteDefs(namelist);
}



//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites(void)
{
    vissprite_p = vissprites;
}


//
// R_NewVisSprite
//
vissprite_t             overflowsprite;

vissprite_t *R_NewVisSprite(void)
{
    if (vissprite_p == &vissprites[MAXVISSPRITES])
        return &overflowsprite;

    vissprite_p++;
    return vissprite_p - 1;
}



//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
int                     *mfloorclip;
int                     *mceilingclip;

fixed_t                 spryscale;
fixed_t                 sprtopscreen;

void R_DrawMaskedColumn(column_t *column)
{
    while (column->topdelta != 0xff)
    {
        // calculate unclipped screen coordinates for post
        int topscreen = sprtopscreen + spryscale * column->topdelta + 1;

        dc_yl = (topscreen + FRACUNIT) >> FRACBITS;
        dc_yh = (topscreen + spryscale * column->length) >> FRACBITS;

        if (dc_yh >= mfloorclip[dc_x])
            dc_yh = mfloorclip[dc_x] - 1;
        if (dc_yl <= mceilingclip[dc_x])
        {
            int oldyl = dc_yl;

            dc_yl = mceilingclip[dc_x] + 1;
            dc_texturefrac = (dc_yl - oldyl) * dc_iscale;
        }
        else
            dc_texturefrac = 0;

        if (dc_yl >= 0 && dc_yh < viewheight && dc_yl <= dc_yh)
        {
            dc_source = (byte *)column + 3;

            colfunc();
        }
        column = (column_t *)((byte *)column + column->length + 4);
    }
}

void R_DrawMaskedColumn2(column_t *column)
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
            {
                dc_yh -= (FixedDiv(endfrac - maxfrac - 1, dc_iscale) + FRACUNIT - 1) >> FRACBITS;
            }
        }

        dc_source = (byte *)column + 3;

        if (dc_yl >= 0 && dc_yh < viewheight && dc_yl <= dc_yh)
            colfunc();

        column = (column_t *)((byte *)column + column->length + 4);
    }
}

boolean                 colors9and159to142;
int                     fuzzpos;

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
void R_DrawVisSprite(vissprite_t *vis, int x1, int x2, boolean psprite)
{
    column_t            *column;
    int                 texturecolumn;
    fixed_t             frac;
    patch_t             *patch;

    patch = (patch_t *)W_CacheLumpNum(vis->patch + firstspritelump, PU_CACHE);

    dc_colormap = vis->colormap;

    if (vis->mobjflags2 & MF2_FUZZYWEAPON)
        colfunc = psprcolfunc;
    else if (vis->mobjflags2 & MF2_TRANSLUCENT)
        colfunc = (viewplayer->fixedcolormap == INVERSECOLORMAP ? tl50colfunc : tlcolfunc);
    else if (vis->mobjflags2 & MF2_TRANSLUCENT_REDTOGREEN_50)
        colfunc = tlredtogreen50colfunc;
    else if (vis->mobjflags2 & MF2_TRANSLUCENT_REDTOBLUE_50)
        colfunc = tlredtoblue50colfunc;
    else if (vis->mobjflags2 & MF2_TRANSLUCENT_50)
        colfunc = tl50colfunc;
    else if (vis->mobjflags2 & MF2_TRANSLUCENT_33)
        colfunc = tl33colfunc;
    else if (vis->mobjflags2 & MF2_TRANSLUCENT_GREENONLY)
        colfunc = (viewplayer->fixedcolormap == INVERSECOLORMAP ? tlgreen50colfunc : tlgreencolfunc);
    else if (vis->mobjflags2 & MF2_TRANSLUCENT_REDONLY)
        colfunc = (viewplayer->fixedcolormap == INVERSECOLORMAP ? tlred50colfunc : tlredcolfunc);
    else if (vis->mobjflags2 & MF2_TRANSLUCENT_REDWHITEONLY)
        colfunc = (viewplayer->fixedcolormap == INVERSECOLORMAP ? tlredwhite50colfunc : tlredwhitecolfunc);
    else if (vis->mobjflags2 & MF2_TRANSLUCENT_BLUEONLY)
        colfunc = (viewplayer->fixedcolormap == INVERSECOLORMAP ? tlblue50colfunc : tlbluecolfunc);
    else if (vis->mobjflags2 & MF2_REDTOGREEN)
        colfunc = redtogreencolfunc;
    else if (vis->mobjflags2 & MF2_REDTOBLUE)
        colfunc = redtobluecolfunc;
    else if (!dc_colormap)
    {
        // NULL colormap = shadow draw
        fuzzpos = 0;
        colfunc = fuzzcolfunc;
    }
    else if (vis->mobjflags & MF_TRANSLATION)
    {
        colfunc = transcolfunc;
        dc_translation = translationtables - 256 +
            ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
    }
    //if (viewplayer->fixedcolormap == INVERSECOLORMAP)
    //    colfunc = basecolfunc;

    dc_iscale = FixedDiv(FRACUNIT, vis->scale);
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);

    colors9and159to142 = (vis->type == MT_MEGA);

    for (dc_x = vis->x1; dc_x <= vis->x2; dc_x++, frac += vis->xiscale)
    {
        texturecolumn = frac >> FRACBITS;
        column = (column_t *)((byte *)patch +
                              LONG(patch->columnofs[texturecolumn]));
        if (psprite)
            R_DrawMaskedColumn(column);
        else
            R_DrawMaskedColumn2(column);
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

    unsigned            rot;
    boolean             flip;

    int                 index;

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

    if (thing->flags2 & MF2_FLIPPEDCORPSE)
        flip = true;

    // calculate edges of the shape
    tx -= (flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump]);
    x1 = (centerxfrac + FRACUNIT/2 + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;

    tx += spritewidth[lump];
    x2 = ((centerxfrac + FRACUNIT/2 + FixedMul(tx, xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;

    gzt = thing->z + spritetopoffset[lump];

    if (thing->z > viewz + FixedDiv(viewheight << FRACBITS, xscale) ||
        gzt < viewz - FixedDiv((viewheight << FRACBITS) - viewheight, xscale))
        return;

    // store information in a vissprite
    vis = R_NewVisSprite();
    vis->mobjflags = thing->flags;
    vis->mobjflags2 = thing->flags2;
    vis->type = thing->type;
    vis->scale = FixedDiv(projectiony, tz);
    vis->gx = thing->x;
    vis->gy = thing->y;
    vis->gz = thing->z;
    vis->gzt = gzt;
    vis->texturemid = vis->gzt - viewz;
    vis->x1 = (x1 < 0 ? 0 : x1);
    vis->x2 = (x2 >= viewwidth ? viewwidth-1 : x2);
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
    if (thing->flags & MF_SHADOW)
    {
        // shadow draw
        vis->colormap = NULL;
    }
    else if (fixedcolormap)
    {
        // fixed map
        vis->colormap = fixedcolormap;
    }
    else if (thing->frame & FF_FULLBRIGHT)
    {
        // full bright
        vis->colormap = colormaps;
    }
    else
    {
        // diminished light
        index = xscale >> LIGHTSCALESHIFT;
        if (index >= MAXLIGHTSCALE)
            index = MAXLIGHTSCALE - 1;

        vis->colormap = spritelights[index];
    }
}




//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites(sector_t *sec)
{
    mobj_t              *thing;
    int                 lightnum;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
        return;

    // Well, now it will be done.
    sec->validcount = validcount;

    lightnum = (sec->lightlevel >> LIGHTSEGSHIFT) + extralight;

    if (lightnum < 0)
        spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
        spritelights = scalelight[LIGHTLEVELS - 1];
    else
        spritelights = scalelight[lightnum];

    // Handle all things in sector.
    for (thing = sec->thinglist; thing; thing = thing->snext)
        R_ProjectSprite(thing);
}


//
// R_DrawPSprite
//
static boolean          flash;

void R_DrawPSprite(pspdef_t *psp)
{
    fixed_t             tx;
    int                 x1;
    int                 x2;
    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;
    boolean             flip;
    vissprite_t         *vis;
    vissprite_t         avis;
    state_t             *state;

    int flags2[16] =
    {
        0, 0, 0, 0, MF2_TRANSLUCENT, MF2_TRANSLUCENT, MF2_TRANSLUCENT_REDWHITEONLY, 0,
        MF2_TRANSLUCENT, 0, MF2_TRANSLUCENT, 0, 0, MF2_TRANSLUCENT, 0, MF2_TRANSLUCENT
    };

    // decide which patch to use
    state = psp->state;
    sprdef = &sprites[state->sprite];
    sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];

    // calculate edges of the shape
    tx = psp->sx - ORIGINALWIDTH / 2 * FRACUNIT;

    tx -= spriteoffset[lump];
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
    vis->texturemid = (BASEYCENTER << FRACBITS) + FRACUNIT / 2 - (psp->sy - spritetopoffset[lump]);
    vis->x1 = (x1 < 0 ? 0 : x1);
    vis->x2 = (x2 >= viewwidth ? viewwidth - 1 : x2);
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

    if (viewplayer->powers[pw_invisibility] > 128
        || (viewplayer->powers[pw_invisibility] & 8))
    {
        // shadow draw
        vis->mobjflags2 |= MF2_FUZZYWEAPON;
    }
    if (fixedcolormap)
    {
        // fixed color
        vis->colormap = fixedcolormap;
    }
    else
    {
        if (flash || psp->state->frame & FF_FULLBRIGHT)
        {
            // full bright
            vis->colormap = colormaps;
        }
        else
        {
            // local light
            int lightnum = (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT)
                            + extralight + 16;

            if (lightnum > MAXLIGHTSCALE)
                lightnum = MAXLIGHTSCALE;
            vis->colormap = spritelights[lightnum];
        }
    }

    if (flash)
        vis->mobjflags2 |= flags2[psp->state->sprite];

    R_DrawVisSprite(vis, vis->x1, vis->x2, true);
}



//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites(void)
{
    int                 i;
    int                 lightnum;
    pspdef_t            *psp;

    // get light level
    lightnum =
        (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT)
        + extralight;

    if (lightnum < 0)
        spritelights = scalelight2[0];
    else if (lightnum >= LIGHTLEVELS)
        spritelights = scalelight2[LIGHTLEVELS - 1];
    else
        spritelights = scalelight2[lightnum];

    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;

    // add all active psprites
    flash = false;
    for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
        if (psp->state && (psp->state->frame & FF_FULLBRIGHT))
            flash = true;
    if (viewplayer->powers[pw_invisibility] > 128
        || (viewplayer->powers[pw_invisibility] & 8))
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
vissprite_t             vsprsortedhead;

void R_SortVisSprites(void)
{
    int                 i;
    int                 count;
    vissprite_t         *ds;
    vissprite_t         *best;
    vissprite_t         unsorted;
    fixed_t             bestscale;

    count = vissprite_p - vissprites;

    unsorted.next = unsorted.prev = &unsorted;

    if (!count)
        return;

    for (ds = vissprites; ds < vissprite_p; ds++)
    {
        ds->next = ds + 1;
        ds->prev = ds - 1;
    }

    vissprites[0].prev = &unsorted;
    unsorted.next = &vissprites[0];
    (vissprite_p-1)->next = &unsorted;
    unsorted.prev = vissprite_p - 1;

    // pull the vissprites out by scale

    vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;
    for (i = 0; i < count; i++)
    {
        bestscale = INT_MAX;
        best = unsorted.next;
        for (ds = unsorted.next; ds != &unsorted; ds = ds->next)
        {
            if (ds->scale < bestscale)
            {
                bestscale = ds->scale;
                best = ds;
            }
        }
        best->next->prev = best->prev;
        best->prev->next = best->next;
        best->next = &vsprsortedhead;
        best->prev = vsprsortedhead.prev;
        vsprsortedhead.prev->next = best;
        vsprsortedhead.prev = best;
    }
}



//
// R_DrawSprite
//
void R_DrawSprite(vissprite_t *spr)
{
    drawseg_t           *ds;
    int                 clipbot[MAXWIDTH];
    int                 cliptop[MAXWIDTH];
    int                 x;
    int                 r1;
    int                 r2;
    fixed_t             scale;
    fixed_t             lowscale;

    if (spr->x1 > spr->x2)
        return;

    for (x = spr->x1; x <= spr->x2; x++)
        clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.
    for (ds = ds_p; ds-- > drawsegs; )
    {
        // determine if the drawseg obscures the sprite
        if (ds->x1 > spr->x2
            || ds->x2 < spr->x1
            || (!ds->silhouette
                && !ds->maskedtexturecol) )
        {
            // does not cover sprite
            continue;
        }

        r1 = (ds->x1 < spr->x1 ? spr->x1 : ds->x1);
        r2 = (ds->x2 > spr->x2 ? spr->x2 : ds->x2);

        if (ds->scale1 > ds->scale2)
        {
            lowscale = ds->scale2;
            scale = ds->scale1;
        }
        else
        {
            lowscale = ds->scale1;
            scale = ds->scale2;
        }

        if (scale < spr->scale
            || (lowscale < spr->scale
                && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
        {
            // masked mid texture?
            if (ds->maskedtexturecol)
                R_RenderMaskedSegRange(ds, r1, r2);
            // seg is behind sprite
            continue;
        }

        // clip this piece of the sprite
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
        if (clipbot[x] == -2 || clipbot[x] > viewheight)
            clipbot[x] = viewheight;

        if (cliptop[x] == -2 || cliptop[x] < 0)
            cliptop[x] = -1;
    }

    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite(spr, spr->x1, spr->x2, false);
}




//
// R_DrawMasked
//
void R_DrawMasked(void)
{
    vissprite_t         *spr;
    drawseg_t           *ds;

    R_SortVisSprites();

    if (vissprite_p > vissprites)
    {
        // draw all vissprites back to front
        for (spr = vsprsortedhead.next;
             spr != &vsprsortedhead;
             spr = spr->next)
        {
            R_DrawSprite (spr);
        }
    }

    // render any remaining masked mid textures
    for (ds = ds_p; ds-- > drawsegs; )
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    R_DrawPlayerSprites ();
}