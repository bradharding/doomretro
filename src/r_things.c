/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#include "c_cmds.h"
#include "c_console.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_system.h"
#include "m_config.h"
#include "m_menu.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAXSPRITEFRAMES 29
#define MINZ            (4 * FRACUNIT)
#define BASEYCENTER     (VANILLAHEIGHT / 2)

#define MAXVISSPRITES   256
#define DS_RANGES_COUNT 3

//
// Sprite rotation 0 is facing the viewer, rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle, which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t                         pspritescale;
fixed_t                         pspriteiscale;

static lighttable_t             **spritelights;         // killough 01/25/98 made static
static lighttable_t             **nextspritelights;

typedef struct
{
    short                       x1, x2;
    drawseg_t                   *user;
} drawseg_xrange_item_t;

typedef struct
{
    drawseg_xrange_item_t       *items;
    int                         count;
} drawsegs_xrange_t;

static drawsegs_xrange_t        drawsegs_xranges[DS_RANGES_COUNT];

static drawseg_xrange_item_t    *drawsegs_xrange;
static unsigned int             drawsegs_xrange_size;
static int                      drawsegs_xrange_count;

// constant arrays used for psprite clipping and initializing clipping
int                             negonearray[MAXWIDTH];
int                             viewheightarray[MAXWIDTH];

static int                      cliptop[MAXWIDTH];
static int                      clipbot[MAXWIDTH];

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up and range check thing_t sprites patches
spritedef_t                     *sprites;

short                           firstbloodsplatlump;

bool                            allowwolfensteinss = true;

static spriteframe_t            sprtemp[MAXSPRITEFRAMES];
static int                      maxframe;

static bool                     drawshadows;
static bool                     interpolatesprites;
static bool                     invulnerable;
static fixed_t                  floorheight;

static const fixed_t floatbobdiffs[64] =
{
     205560,  205560,  203576,  199640,  193776,  186048,  176528,  165304,
     152496,  138216,  122600,  105808,   87992,   69336,   50008,   30200,
      10096,  -10096,  -30200,  -50008,  -69336,  -87992, -105808, -122600,
    -138216, -152496, -165304, -176528, -186048, -193776, -199640, -203576,
    -205560, -205560, -203576, -199640, -193776, -186048, -176528, -165304,
    -152496, -138216, -122600, -105808,  -88000,  -69336,  -50008,  -30200,
     -10096,   10096,   30200,   50008,   69336,   87992,  105808,  122600,
     138216,  152496,  165304,  176528,  186048,  193776,  199640,  203576
};

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
static void R_InstallSpriteLump(const int lump, const int frame, const char rot, const bool flipped)
{
    unsigned int    rotation = (rot >= 'A' ? rot - 'A' + 10 : (rot >= '0' ? rot - '0' : 17));

    if (frame >= MAXSPRITEFRAMES || rotation > 16)
    {
        I_Error("R_InstallSpriteLump: Bad frame characters in lump %s", lumpinfo[lump]->name);
        return;
    }

    if (frame > maxframe)
        maxframe = frame;

    if (!rotation)
    {
        // the lump should be used for all rotations
        for (int r = 14; r >= 0; r -= 2)
            if (sprtemp[frame].lump[r] == -1)
            {
                sprtemp[frame].lump[r] = lump - firstspritelump;

                if (flipped)
                    sprtemp[frame].flip |= (1 << r);

                sprtemp[frame].rotate = 0;  // jff 4/24/98 if any subbed, rotless
            }

        return;
    }

    // the lump is only used for one rotation
    rotation = (rotation <= 8 ? (rotation - 1) * 2 : (rotation - 9) * 2 + 1);

    if (sprtemp[frame].lump[rotation] == -1)
    {
        sprtemp[frame].lump[rotation] = lump - firstspritelump;

        if (flipped)
            sprtemp[frame].flip |= (1 << rotation);

        sprtemp[frame].rotate = 1;          // jff 4/24/98 only change if rot used
    }
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names (4 chars exactly) to be used.
//
// Builds the sprite rotation matrices to account for horizontally flipped sprites.
//
// Will report an error if the lumps are inconsistent. Only called at startup.
//
// Sprite lump names are 4 characters for the actor, a letter for the frame, and a number for the rotation.
//
// A sprite that is flippable will have an additional letter/number appended.
//
// The rotation character can be 0 to signify no rotations.
//
// 01/25/98, 01/31/98 killough : Rewritten for performance
//
// Empirically verified to have excellent hash properties across standard DOOM sprites:
#define R_SpriteNameHash(s) ((unsigned int)((s)[0] - ((s)[1] * 3 - (s)[3] * 2 - (s)[2]) * 2))

static void R_InitSpriteDefs(void)
{
    const size_t    numentries = (size_t)lastspritelump - firstspritelump + 1;

    struct
    {
        int index;
        int next;
    } *hash;

    sprites = Z_Calloc(NUMSPRITES, sizeof(*sprites), PU_STATIC, NULL);

    // Create hash table based on just the first four letters of each sprite
    // killough 01/31/98
    hash = malloc(numentries * sizeof(*hash));      // allocate hash table

    for (unsigned int i = 0; i < numentries; i++)   // initialize hash table as empty
        hash[i].index = -1;

    for (unsigned int i = 0; i < numentries; i++)   // Prepend each sprite to hash chain
    {
        const int   j = R_SpriteNameHash(lumpinfo[i + firstspritelump]->name) % numentries;

        hash[i].next = hash[j].index;
        hash[j].index = i;
    }

    // scan all the lump names for each of the names, noting the highest frame letter.
    for (unsigned int i = 0; i < NUMSPRITES; i++)
    {
        const char  *spritename = sprnames[i];
        int         j = hash[R_SpriteNameHash(spritename) % numentries].index;

        if (j >= 0)
        {
            memset(sprtemp, -1, sizeof(sprtemp));

            for (int k = 0; k < MAXSPRITEFRAMES; k++)
                sprtemp[k].flip = 0;

            maxframe = -1;

            do
            {
                const lumpinfo_t    *lump = lumpinfo[j + firstspritelump];

                // Fast portable comparison -- killough
                // (using int pointer cast is nonportable):
                if (!((lump->name[0] ^ spritename[0]) | (lump->name[1] ^ spritename[1])
                    | (lump->name[2] ^ spritename[2]) | (lump->name[3] ^ spritename[3])))
                {
                    R_InstallSpriteLump(j + firstspritelump, lump->name[4] - 'A', lump->name[5], false);

                    if (lump->name[6])
                        R_InstallSpriteLump(j + firstspritelump, lump->name[6] - 'A', lump->name[7], true);
                }
            } while ((j = hash[j].next) >= 0);

            // check the frames that were found for completeness
            if ((sprites[i].numframes = ++maxframe))  // killough 01/31/98
            {
                for (int frame = 0; frame < maxframe; frame++)
                    switch (sprtemp[frame].rotate)
                    {
                        case -1:
                            // no rotations were found for that frame at all
                            break;

                        case 0:
                            // only the first rotation is needed
                            for (int rot = 1; rot < 16; rot++)
                                sprtemp[frame].lump[rot] = sprtemp[frame].lump[0];

                            // If the frame is flipped, they all should be
                            if (sprtemp[frame].flip & 1)
                                sprtemp[frame].flip = 0xFFFF;

                            break;

                        case 1:
                            // must have all 8 frames
                            for (int rot = 0; rot < 16; rot += 2)
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

                            for (int rot = 0; rot < 16; rot++)
                                if (sprtemp[frame].lump[rot] == -1)
                                    I_Error("R_InitSprites: Frame %c of sprite %.8s is missing rotations", frame + 'A', sprnames[i]);

                            break;
                    }

                for (int frame = 0; frame < maxframe; frame++)
                    if (sprtemp[frame].rotate == -1)
                    {
                        memset(&sprtemp[frame].lump, 0, sizeof(sprtemp[0].lump));
                        sprtemp[frame].flip = 0;
                        sprtemp[frame].rotate = 0;
                    }

                // allocate space for the frames present and copy sprtemp to it
                sprites[i].spriteframes = Z_Malloc(maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
                memcpy(sprites[i].spriteframes, sprtemp, maxframe * sizeof(spriteframe_t));
            }
        }
    }

    free(hash); // free hash table

    firstbloodsplatlump = sprites[SPR_BLD2].spriteframes[0].lump[0];

    // check if Wolfenstein SS sprites have been changed to zombiemen sprites
    if (gamemode != commercial || (bfgedition && !states[S_SSWV_STND].dehacked))
        allowwolfensteinss = false;
    else
    {
        const short poss = sprites[SPR_POSS].spriteframes[0].lump[0];
        const short sswv = sprites[SPR_SSWV].spriteframes[0].lump[0];

        if (spritewidth[poss] == spritewidth[sswv]
            && spriteheight[poss] == spriteheight[sswv]
            && spriteoffset[poss] == spriteoffset[sswv]
            && spritetopoffset[poss] == spritetopoffset[sswv])
            allowwolfensteinss = false;
    }
}

//
// GAME FUNCTIONS
//

static vissprite_t              *vissprites;
static vissprite_t              **vissprite_ptrs;
static unsigned int             num_vissprite;
static unsigned int             num_bloodsplatvissprite;
static unsigned int             num_vissprite_alloc = MAXVISSPRITES;

static bloodsplatvissprite_t    bloodsplatvissprites[r_bloodsplats_max_max];

//
// R_InitSprites
// Called at program start.
//
void R_InitSprites(void)
{
    for (int i = 0; i < MAXWIDTH; i++)
        negonearray[i] = -1;

    R_InitSpriteDefs();

    vissprites = malloc(num_vissprite_alloc * sizeof(*vissprites));
}

//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites(void)
{
    num_vissprite = 0;
    num_bloodsplatvissprite = 0;
}

//
// R_NewVisSprite
//
static vissprite_t *R_NewVisSprite(void)
{
    if (num_vissprite >= num_vissprite_alloc)
    {
        num_vissprite_alloc = (num_vissprite_alloc ? num_vissprite_alloc * 2 : MAXVISSPRITES);
        vissprites = I_Realloc(vissprites, num_vissprite_alloc * sizeof(*vissprites));
    }

    return (vissprites + num_vissprite++);
}

int         *mfloorclip;
int         *mceilingclip;

fixed_t     spryscale;
int64_t     sprtopscreen;
static int  shadowtopscreen;
static int  shadowshift;
static int  splattopscreen;

static void (*shadowcolfunc)(void);

//
// R_BlastSpriteColumn
//
static void inline R_BlastSpriteColumn(const rcolumn_t *column)
{
    unsigned char   *pixels = column->pixels;

    while (dc_numposts--)
    {
        const rpost_t   *post = &column->posts[dc_numposts];
        const int       topdelta = post->topdelta;
        const int64_t   topscreen = sprtopscreen + (int64_t)spryscale * topdelta;

        if ((dc_yh = MIN((int)((topscreen + (int64_t)spryscale * post->length - 256) >> FRACBITS), dc_floorclip)) >= 0)
            if ((dc_yl = MAX(dc_ceilingclip, (int)((topscreen + FRACUNIT + 512) >> FRACBITS))) <= dc_yh)
            {
                dc_texturefrac = dc_texturemid - (topdelta << FRACBITS) + FixedMul((dc_yl - centery) << FRACBITS, dc_iscale);
                dc_source = pixels + topdelta;
                colfunc();
            }
    }
}

//
// R_BlastPlayerSpriteColumn
//
static void inline R_BlastPlayerSpriteColumn(const rcolumn_t *column)
{
    unsigned char   *pixels = column->pixels;

    while (dc_numposts--)
    {
        const rpost_t   *post = &column->posts[dc_numposts];
        const int       topdelta = post->topdelta;
        const int64_t   topscreen = sprtopscreen + (int64_t)pspritescale * topdelta + 1;

        if ((dc_yh = MIN((int)((topscreen + (int64_t)pspritescale * post->length) >> FRACBITS), viewheight - 1)) >= 0)
            if ((dc_yl = MAX(0, (int)((topscreen + FRACUNIT) >> FRACBITS))) <= dc_yh)
            {
                dc_texturefrac = dc_texturemid - (topdelta << FRACBITS) + FixedMul((dc_yl - centery) << FRACBITS, dc_iscale);
                dc_source = pixels + topdelta;
                colfunc();
            }
    }
}

//
// R_DrawVisSprite
//
static void R_DrawVisSprite(const vissprite_t *vis)
{
    fixed_t         frac = vis->startfrac;
    const fixed_t   xiscale = vis->xiscale;
    const fixed_t   x2 = vis->x2;
    const rpatch_t  *patch = R_CachePatchNum(vis->patch + firstspritelump);
    const mobj_t    *mobj = vis->mobj;
    const int       flags = mobj->flags;
    const int       translation = (flags & MF_TRANSLATION);
    int             baseclip;

    spryscale = vis->scale;

    dc_colormap[0] = vis->colormap;
    dc_nextcolormap[0] = vis->nextcolormap;
    dc_z = ((spryscale >> 5) & 255);
    dc_iscale = FixedDiv(FRACUNIT, spryscale);
    dc_texturemid = vis->texturemid;

    if (translation && (r_corpses_color || !(flags & MF_CORPSE)))
    {
        colfunc = translatedcolfunc;
        dc_translation = &translationtables[(translation >> (MF_TRANSLATIONSHIFT - 8)) - 256];
    }
    else
    {
        colfunc = vis->colfunc;

        if (colfunc == bloodcolfunc || colfunc == translatedcolfunc)
            dc_translation = colortranslation[mobj->bloodcolor - 1];
    }

    sprtopscreen = (int64_t)centeryfrac - FixedMul(dc_texturemid, spryscale);
    baseclip = (vis->footclip ? (int)(sprtopscreen + vis->footclip) >> FRACBITS : viewheight);
    fuzzpos = 0;

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
    {
        const rcolumn_t *column = R_GetPatchColumnClamped(patch, frac >> FRACBITS);

        if ((dc_numposts = column->numposts))
        {
            dc_ceilingclip = mceilingclip[dc_x] + 1;
            dc_floorclip = MIN(baseclip, mfloorclip[dc_x]) - 1;
            R_BlastSpriteColumn(column);
        }
    }
}

//
// R_DrawVisSpriteWithShadow
//
static void R_DrawVisSpriteWithShadow(const vissprite_t *vis)
{
    fixed_t         frac = vis->startfrac;
    const fixed_t   xiscale = vis->xiscale;
    const fixed_t   x2 = vis->x2;
    const rpatch_t  *patch = R_CachePatchNum(vis->patch + firstspritelump);
    const mobj_t    *mobj = vis->mobj;
    const int       flags = mobj->flags;
    const int       translation = (flags & MF_TRANSLATION);

    spryscale = vis->scale;

    dc_colormap[0] = vis->colormap;
    dc_nextcolormap[0] = vis->nextcolormap;
    dc_z = ((spryscale >> 5) & 255);
    dc_black = dc_colormap[0][nearestblack];

    if (flags & MF_FUZZ)
        dc_black33 = &tinttab15[dc_black << 8];
    else if ((mobj->flags2 & MF2_TRANSLUCENT_33) && r_sprites_translucency)
    {
        dc_black33 = &tinttab10[dc_black << 8];
        dc_black40 = &tinttab25[dc_black << 8];
    }
    else
    {
        dc_black33 = &tinttab33[dc_black << 8];
        dc_black40 = &tinttab40[dc_black << 8];
    }

    dc_iscale = FixedDiv(FRACUNIT, spryscale);
    dc_texturemid = vis->texturemid;

    if (translation && (r_corpses_color || !(flags & MF_CORPSE)))
    {
        colfunc = translatedcolfunc;
        dc_translation = &translationtables[(translation >> (MF_TRANSLATIONSHIFT - 8)) - 256];
    }
    else
    {
        colfunc = vis->colfunc;

        if (colfunc == translatedcolfunc)
            dc_translation = colortranslation[mobj->bloodcolor - 1];
    }

    sprtopscreen = (int64_t)centeryfrac - FixedMul(dc_texturemid, spryscale);
    shadowcolfunc = mobj->shadowcolfunc;
    shadowtopscreen = centeryfrac - FixedMul(vis->shadowpos, spryscale);
    shadowshift = (shadowtopscreen * 9 / 10) >> FRACBITS;
    fuzzpos = 0;

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
    {
        const rcolumn_t *column = R_GetPatchColumnClamped(patch, frac >> FRACBITS);

        if ((dc_numposts = column->numposts))
        {
            const rpost_t   *posts = column->posts;

            dc_ceilingclip = mceilingclip[dc_x] + 1;
            dc_floorclip = mfloorclip[dc_x] - 1;

            while (dc_numposts--)
            {
                const rpost_t   *post = &posts[dc_numposts];
                const int       topscreen = shadowtopscreen + spryscale * post->topdelta;

                if ((dc_yh = MIN((((topscreen + spryscale * post->length) >> FRACBITS) / 10 + shadowshift), dc_floorclip)) >= 0)
                    if ((dc_yl = MAX(dc_ceilingclip, ((topscreen + FRACUNIT) >> FRACBITS) / 10 + shadowshift)) <= dc_yh)
                        shadowcolfunc();
            }

            dc_numposts = column->numposts;
            R_BlastSpriteColumn(column);
        }
    }
}

//
// R_DrawPlayerVisSprite
//
static void R_DrawPlayerVisSprite(const vissprite_t *vis)
{
    fixed_t         frac = vis->startfrac;
    const fixed_t   x2 = vis->x2;
    const rpatch_t  *patch = R_CachePatchNum(vis->patch + firstspritelump);

    colfunc = vis->colfunc;
    dc_colormap[0] = vis->colormap;
    dc_nextcolormap[0] = vis->colormap;
    dc_iscale = pspriteiscale;
    dc_texturemid = vis->texturemid;
    sprtopscreen = (int64_t)centeryfrac - FixedMul(dc_texturemid, pspritescale);

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += pspriteiscale)
    {
        const rcolumn_t *column = R_GetPatchColumnClamped(patch, frac >> FRACBITS);

        if ((dc_numposts = column->numposts))
            R_BlastPlayerSpriteColumn(column);
    }
}

//
// R_DrawBloodSplatVisSprite
//
static void R_DrawBloodSplatVisSprite(const bloodsplatvissprite_t *vis)
{
    fixed_t         frac = vis->startfrac;
    const fixed_t   xiscale = vis->xiscale;
    const fixed_t   x2 = vis->x2;
    const rcolumn_t *columns = R_CachePatchNum(vis->patch)->columns;

    spryscale = vis->scale;
    colfunc = vis->colfunc;
    dc_bloodcolor = &tinttab50[(dc_solidbloodcolor = vis->colormap[vis->color]) << 8];
    splattopscreen = centeryfrac - FixedMul(vis->texturemid, spryscale);

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
    {
        const rcolumn_t *column = &columns[frac >> FRACBITS];

        if (column->numposts)
        {
            const rpost_t   *post = column->posts;
            const int       topscreen = splattopscreen + spryscale * post->topdelta;

            if ((dc_yh = MIN((topscreen + spryscale * post->length) >> FRACBITS, clipbot[dc_x] - 1)) >= 0)
                if ((dc_yl = MAX(cliptop[dc_x], topscreen >> FRACBITS)) <= dc_yh)
                    colfunc();
        }
    }
}

//
// R_ProjectSprite
// Generates a vissprite for a thing
//  if it might be visible.
//
static void R_ProjectSprite(mobj_t *thing)
{
    fixed_t         tx;
    fixed_t         xscale;
    int             x1;
    int             x2;
    spriteframe_t   *sprframe;
    int             lump;
    fixed_t         width;
    bool            flip;
    vissprite_t     *vis;
    sector_t        *heightsec;
    int             flags2;
    int             frame;
    fixed_t         tr_x, tr_y;
    fixed_t         gzt;
    fixed_t         tz;
    angle_t         rot = 0;
    fixed_t         fx, fy, fz;
    fixed_t         offset;
    fixed_t         topoffset;
    fixed_t         height;

    if (thing->player && thing->player->mo == thing)
        return;

    // [AM] Interpolate between current and last position, if prudent.
    if (thing->interpolate && interpolatesprites)
    {
        fx = thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic);
        fy = thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic);
        fz = thing->oldz + FixedMul(thing->z - thing->oldz, fractionaltic);
    }
    else
    {
        fx = thing->x;
        fy = thing->y;
        fz = thing->z;
    }

    tr_x = fx - viewx;
    tr_y = fy - viewy;
    tz = FixedMul(tr_x, viewcos) + FixedMul(tr_y, viewsin);

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    // too far off the side?
    if (ABS((tx = FixedMul(tr_x, viewsin) - FixedMul(tr_y, viewcos))) > (tz << 2))
        return;

    // decide which patch to use for sprite relative to player
    frame = thing->frame;
    sprframe = &sprites[thing->sprite].spriteframes[(frame & FF_FRAMEMASK)];

    if (((flags2 = thing->flags2) & MF2_FLOATBOB) && r_floatbob)
        fz += floatbobdiffs[((thing->floatbob + maptime) & 63)];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        const angle_t   ang = R_PointToAngle(fx, fy);

        if (sprframe->lump[0] == sprframe->lump[1])
            rot = (ang - thing->angle + (angle_t)(ANG45 / 2) * 9) >> 28;
        else
            rot = (ang - thing->angle + (angle_t)(ANG45 / 2) * 9 - (angle_t)(ANG180 / 16)) >> 28;

        lump = sprframe->lump[rot];
        flip = ((sprframe->flip & (1 << rot)) || (flags2 & MF2_MIRRORED));
    }
    else
    {
        // use single rotation for all views
        lump = sprframe->lump[0];
        flip = ((sprframe->flip & 1) || (flags2 & MF2_MIRRORED));
    }

    height = spriteheight[lump];

    if (thing->info->dehacked || !r_fixspriteoffsets)
    {
        offset = spriteoffset[lump];
        topoffset = spritetopoffset[lump];
    }
    else
    {
        offset = newspriteoffset[lump];
        topoffset = newspritetopoffset[lump];
    }

    gzt = fz + topoffset;
    xscale = FixedDiv(projection, tz);

    // killough 04/09/98: clip things which are out of view due to height
    if (fz > (int64_t)(viewz + FixedDiv(viewheightfrac, xscale))
        || gzt < (int64_t)(viewz - FixedDiv(viewheightfrac - viewheight, xscale)))
        return;

    // calculate edges of the shape
    width = spritewidth[lump];
    tx -= (flip ? width - offset : offset);

    // off the right side?
    if ((x1 = (centerxfrac + FixedMul(tx, xscale)) >> FRACBITS) >= viewwidth)
        return;

    // off the left side?
    if ((x2 = ((centerxfrac + FixedMul(tx + width, xscale) - FRACUNIT / 2) >> FRACBITS)) < 0)
        return;

    // quickly reject sprites with bad x ranges
    if (x1 >= x2)
        return;

    // killough 03/27/98: exclude things totally separated
    // from the viewer, by either water or fake ceilings
    // killough 04/11/98: improve sprite clipping for underwater/fake ceilings
    if ((heightsec = thing->subsector->sector->heightsec))
    {
        sector_t    *phs = viewplayer->mo->subsector->sector->heightsec;

        if (phs)
        {
            if (viewz < phs->interpfloorheight ? fz >= heightsec->interpfloorheight : gzt < heightsec->interpfloorheight)
                return;

            if (viewz > phs->interpceilingheight ?
                gzt < heightsec->interpceilingheight && viewz >= heightsec->interpceilingheight :
                fz >= heightsec->interpceilingheight)
                return;
        }
    }

    // store information in a vissprite
    vis = R_NewVisSprite();

    // killough 03/27/98: save sector for special clipping later
    vis->heightsec = heightsec;

    vis->mobj = thing;
    vis->scale = xscale;
    vis->gx = fx;
    vis->gy = fy;
    vis->gz = floorheight;
    vis->gzt = gzt;

    if ((flags2 & MF2_CASTSHADOW) && xscale >= FRACUNIT / 4 && drawshadows)
        vis->shadowpos = floorheight + thing->shadowoffset - viewz;
    else
        vis->shadowpos = 1;

    vis->colfunc = (invulnerable && r_textures ? thing->altcolfunc : thing->colfunc);

    // foot clipping
    if ((flags2 & MF2_FEETARECLIPPED) && !heightsec && r_liquid_clipsprites && topoffset - height <= 4 * FRACUNIT)
    {
        fixed_t clipfeet = MIN((height >> FRACBITS) / 4, 10) << FRACBITS;

        vis->texturemid = gzt - viewz - clipfeet;

        if (r_liquid_bob)
            clipfeet += animatedliquiddiff;

        vis->footclip = FixedMul(height - clipfeet, xscale);
    }
    else
    {
        vis->texturemid = gzt - viewz;
        vis->footclip = 0;
    }

    if (flip)
    {
        vis->xiscale = -FixedDiv(FRACUNIT, xscale);

        if (x1 < 0)
        {
            vis->x1 = 0;
            vis->startfrac = width - 1 - vis->xiscale * x1;
        }
        else
        {
            vis->x1 = x1;
            vis->startfrac = width - 1;
        }
    }
    else
    {
        vis->xiscale = FixedDiv(FRACUNIT, xscale);

        if (x1 < 0)
        {
            vis->x1 = 0;
            vis->startfrac = -vis->xiscale * x1;
        }
        else
        {
            vis->x1 = x1;
            vis->startfrac = 0;
        }
    }

    vis->x2 = MIN(x2, viewwidth - 1);
    vis->patch = lump;

    // get light level
    if (fixedcolormap)
    {
        // fixed map
        vis->colormap = fixedcolormap;
        vis->nextcolormap = fixedcolormap;
    }
    else if ((frame & FF_FULLBRIGHT) && (rot <= 5 || rot >= 12 || thing->info->fullbright))
    {
        // full bright
        vis->colormap = fullcolormap;
        vis->nextcolormap = fullcolormap;
    }
    else
    {
        // diminished light
        const int   i = MIN(xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1);

        vis->colormap = spritelights[i];
        vis->nextcolormap = nextspritelights[i];
    }
}

static int  skipsplat[3];

static void R_ProjectBloodSplat(const bloodsplat_t *splat)
{
    fixed_t                 tx;
    fixed_t                 xscale;
    int                     x1;
    int                     x2;
    bloodsplatvissprite_t   *vis;
    const fixed_t           fx = splat->x;
    const fixed_t           fy = splat->y;
    fixed_t                 width;
    const fixed_t           tr_x = fx - viewx;
    const fixed_t           tr_y = fy - viewy;
    const fixed_t           tz = FixedMul(tr_x, viewcos) + FixedMul(tr_y, viewsin);
    fixed_t                 splatdist;
    mobj_t                  *mo = viewplayer->mo;

    // splat is behind view plane?
    if (tz < MINZ)
        return;

    if ((splatdist = P_ApproxDistance(splat->x - mo->x, splat->y - mo->y)) > (5000 << FRACBITS)
        || (splatdist > (2500 << FRACBITS) && (skipsplat[0]++ % 2))
        || (splatdist > (1250 << FRACBITS) && (skipsplat[1]++ % 3))
        || (splatdist > (625 << FRACBITS) && (skipsplat[2]++ % 4)))
        return;

    // too far off the side?
    if (ABS((tx = FixedMul(tr_x, viewsin) - FixedMul(tr_y, viewcos))) > (tz << 2))
        return;

    // calculate edges of the shape
    tx -= ((width = splat->width) >> 1);
    xscale = FixedDiv(projection, tz);

    // off the right side?
    if ((x1 = (centerxfrac + FixedMul(tx, xscale)) >> FRACBITS) >= viewwidth)
        return;

    // off the left side?
    if ((x2 = ((centerxfrac + FixedMul(tx + width, xscale)) >> FRACBITS) - 1) < 0)
        return;

    // quickly reject splats with bad x ranges
    if (x1 >= x2)
        return;

    // store information in a vissprite
    vis = &bloodsplatvissprites[num_bloodsplatvissprite++];

    vis->scale = xscale;
    vis->gx = fx;
    vis->gy = fy;
    vis->color = (r_textures ? splat->viscolor : nearestlightgray);
    vis->colfunc = splat->viscolfunc;
    vis->texturemid = floorheight + FRACUNIT - viewz;
    vis->xiscale = FixedDiv(FRACUNIT, xscale);

    if (x1 < 0)
    {
        vis->x1 = 0;
        vis->startfrac = -vis->xiscale * x1;
    }
    else
    {
        vis->x1 = x1;
        vis->startfrac = 0;
    }

    vis->x2 = MIN(x2, viewwidth - 1);
    vis->patch = splat->patch;

    // get light level
    vis->colormap = (fixedcolormap ? fixedcolormap : spritelights[MIN(xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)]);
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
// killough 09/18/98: add lightlevel as parameter, fixing underwater lighting
void R_AddSprites(sector_t *sec, int lightlevel)
{
    mobj_t  *thing = sec->thinglist;

    if ((floorheight = sec->interpfloorheight) - FRACUNIT <= viewz)
    {
        bloodsplat_t    *splat = sec->splatlist;

        if (splat && drawbloodsplats)
        {
            spritelights = scalelight[BETWEEN(0, (lightlevel >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
            nextspritelights = (thing ?
                scalelight[BETWEEN(0, ((lightlevel + 4) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)] : spritelights);

            skipsplat[0] = 1;
            skipsplat[1] = 1;
            skipsplat[2] = 1;

            do
            {
                R_ProjectBloodSplat(splat);
                splat = splat->next;
            } while (splat);

            if (!thing)
                return;
        }
        else if (thing)
        {
            spritelights = scalelight[BETWEEN(0, (lightlevel >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
            nextspritelights = scalelight[BETWEEN(0, ((lightlevel + 4) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
        }
        else
            return;

        drawshadows = (sec->terraintype == SOLID && !fixedcolormap && r_shadows);
    }
    else if (thing)
    {
        spritelights = scalelight[BETWEEN(0, (lightlevel >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
        nextspritelights = scalelight[BETWEEN(0, ((lightlevel + 4) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];

        drawshadows = false;
    }
    else
        return;

    // Handle all things in sector.
    do
    {
        R_ProjectSprite(thing);
        thing = thing->snext;
    } while (thing);
}

//
// R_DrawPlayerSprite
//
static bool muzzleflash;

static void R_DrawPlayerSprite(pspdef_t *psp, bool invisibility, bool altered)
{
    fixed_t             tx;
    int                 x1, x2;
    vissprite_t         tempvis = { 0 };
    vissprite_t         *vis = &tempvis;
    const state_t       *state = psp->state;
    const spritenum_t   spr = state->sprite;
    const int           frame = state->frame;
    const spriteframe_t *sprframe = &sprites[spr].spriteframes[frame & FF_FRAMEMASK];
    const int           lump = sprframe->lump[0];

    // calculate edges of the shape
    tx = psp->sx - VANILLAWIDTH / 2 * FRACUNIT
        - (!r_fixspriteoffsets || (altered && !vanilla) ? spriteoffset[lump] : newspriteoffset[lump]);
    x1 = (centerxfrac + FRACUNIT / 2 + FixedMul(tx, pspritescale)) >> FRACBITS;
    x2 = ((centerxfrac + FRACUNIT / 2 + FixedMul(tx + spritewidth[lump], pspritescale)) >> FRACBITS) - 1;

    // store information in a vissprite
    vis->texturemid = (BASEYCENTER << FRACBITS) + FRACUNIT / 4 - (psp->sy + ABS(viewplayer->bounce) - spritetopoffset[lump]);

    vis->x1 = MAX(0, x1);
    vis->x2 = MIN(x2, viewwidth - 1);
    vis->startfrac = (vis->x1 > x1 ? pspriteiscale * (vis->x1 - x1) : 0);
    vis->patch = lump;

    // interpolation for weapon bobbing
    if (interpolatesprites)
    {
        typedef struct
        {
            int x1;
            int x1_prev;
            int texturemid;
            int texturemid_prev;
            int lump;
        } psp_interpolate_t;

        static psp_interpolate_t    psp_inter;

        if (realframe && !skippsprinterp)
        {
            psp_inter.x1 = psp_inter.x1_prev;
            psp_inter.texturemid = psp_inter.texturemid_prev;
        }

        psp_inter.x1_prev = vis->x1;
        psp_inter.texturemid_prev = vis->texturemid;

        if (lump == psp_inter.lump && !skippsprinterp)
        {
            const int   deltax = vis->x2 - vis->x1;

            vis->x1 = psp_inter.x1 + FixedMul(fractionaltic, vis->x1 - psp_inter.x1);
            vis->x2 = vis->x1 + deltax;
            vis->texturemid = psp_inter.texturemid + FixedMul(fractionaltic, vis->texturemid - psp_inter.texturemid);
        }
        else
        {
            psp_inter.x1 = vis->x1;
            psp_inter.texturemid = vis->texturemid;
            psp_inter.lump = lump;

            skippsprinterp = false;
        }
    }

    vis->texturemid += FixedMul(((centery - viewheight / 2) << FRACBITS), pspriteiscale);

    if (mouselook && r_screensize < r_screensize_max)
        vis->texturemid -= viewplayer->lookdir * 0x05C0;

    if (invisibility && r_textures)
    {
        vis->colfunc = psprcolfunc;
        vis->colormap = NULL;
    }
    else
    {
        if (r_sprites_translucency)
        {
            if (!r_textures)
            {
                vis->colfunc = (psp == &viewplayer->psprites[1] || invisibility ? &R_DrawTranslucent50ColorColumn : &R_DrawColorColumn);
                vis->colormap = NULL;
            }
            else if (spr == SPR_SHT2)
                vis->colfunc = ((frame & FF_FRAMEMASK) && (frame & FF_FULLBRIGHT)
                    && (!altered || state->translucent || BTSX) ? tlredwhitecolfunc1 : basecolfunc);
            else if (muzzleflash && spr >= SPR_SHTG && spr <= SPR_BFGF && (!altered || state->translucent || BTSX))
            {
                void (*colfuncs[])(void) =
                {
                                   NULL,               NULL,
                    /* SPR_SHTG */ basecolfunc,        basecolfunc,
                    /* SPR_PUNG */ basecolfunc,        basecolfunc,
                    /* SPR_PISG */ basecolfunc,        basecolfunc,
                    /* SPR_PISF */ tlcolfunc,          tl50colfunc,
                    /* SPR_SHTF */ tlcolfunc,          tl50colfunc,
                    /* SPR_SHT2 */ tlredwhitecolfunc1, tlredwhite50colfunc,
                    /* SPR_CHGG */ basecolfunc,        basecolfunc,
                    /* SPR_CHGF */ tlredwhitecolfunc2, tlredwhite50colfunc,
                    /* SPR_MISG */ basecolfunc,        basecolfunc,
                    /* SPR_MISF */ tlredwhitecolfunc2, tlredwhite50colfunc,
                    /* SPR_SAWG */ basecolfunc,        basecolfunc,
                    /* SPR_PLSG */ basecolfunc,        basecolfunc,
                    /* SPR_PLSF */ tlbluecolfunc,      tlbluecolfunc,
                    /* SPR_BFGG */ basecolfunc,        basecolfunc,
                    /* SPR_BFGF */ tlcolfunc,          tl50colfunc
                };

                vis->colfunc = colfuncs[(invulnerable ? spr * 2 + 1 : spr * 2)];
            }
            else
                vis->colfunc = basecolfunc;
        }
        else
        {
            if (!r_textures)
            {
                vis->colfunc = &R_DrawColorColumn;
                vis->colormap = NULL;
            }
            else
                vis->colfunc = basecolfunc;
        }

        if (fixedcolormap)
            vis->colormap = fixedcolormap;       // fixed color
        else
        {
            if (muzzleflash || (frame & FF_FULLBRIGHT))
                vis->colormap = fullcolormap;    // full bright
            else
            {
                sector_t    *sec = viewplayer->mo->subsector->sector;
                const int   lightnum = ((sec->floorlightsec ? sec->floorlightsec : sec)->lightlevel >> OLDLIGHTSEGSHIFT) + extralight;

                vis->colormap = psprscalelight[MIN(lightnum, OLDLIGHTLEVELS - 1)][MIN(lightnum + 16, OLDMAXLIGHTSCALE - 1)];
            }
        }
    }

    R_DrawPlayerVisSprite(vis);
}

//
// R_DrawPlayerSprites
//
static void R_DrawPlayerSprites(void)
{
    const int   invisibility = viewplayer->powers[pw_invisibility];
    const bool  altered = (weaponinfo[viewplayer->readyweapon].altered || !r_fixspriteoffsets);
    pspdef_t    *weapon = viewplayer->psprites;
    pspdef_t    *flash = weapon + 1;
    state_t     *weaponstate = weapon->state;
    state_t     *flashstate = flash->state;

    if (!weaponstate)
        return;

    // add all active psprites
    if (invisibility && (invisibility > STARTFLASHING || (invisibility & FLASHONTIC)) && r_textures)
    {
        fuzzpos = 0;

        V_FillRect(1, viewwindowx, viewwindowy, viewwidth, viewheight, PINK, 0, false, false, NULL, NULL);
        R_DrawPlayerSprite(weapon, true, (weaponstate->dehacked || altered));

        if (flashstate)
            R_DrawPlayerSprite(flash, true, (flashstate->dehacked || altered));

        if (consoleactive)
            R_DrawPausedFuzzColumns();
        else
            R_DrawFuzzColumns();
    }
    else
    {
        muzzleflash = (weaponstate->frame & FF_FULLBRIGHT);

        if (flashstate)
        {
            muzzleflash |= (flashstate->frame & FF_FULLBRIGHT);

            R_DrawPlayerSprite(weapon, false, (weaponstate->dehacked || altered));
            R_DrawPlayerSprite(flash, false, (flashstate->dehacked || altered));
        }
        else
            R_DrawPlayerSprite(weapon, false, (weaponstate->dehacked || altered));
    }
}

//
// R_DrawBloodSplatSprite
//
static void R_DrawBloodSplatSprite(const bloodsplatvissprite_t *splat)
{
    const int       x1 = splat->x1;
    const int       x2 = splat->x2;
    const fixed_t   scale = splat->scale;
    const fixed_t   gx = splat->gx;
    const fixed_t   gy = splat->gy;

    // initialize the clipping arrays
    for (int i = x1; i <= x2; i++)
    {
        cliptop[i] = -1;
        clipbot[i] = viewheight;
    }

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale is the clip seg.
    for (drawseg_t *ds = ds_p; ds-- > drawsegs; )
    {
        const int   silhouette = ds->silhouette;

        // determine if the drawseg obscures the blood splat
        if (ds->x1 > x2 || ds->x2 < x1 || (!silhouette && !ds->maskedtexturecol))
            continue;

        if (ds->maxscale < scale || (ds->minscale < scale && !R_PointOnSegSide(gx, gy, ds->curline)))
            continue;
        else
        {
            // clip this piece of the blood splat
            const int   r1 = MAX(x1, ds->x1);
            const int   r2 = MIN(ds->x2, x2);

            if (silhouette & SIL_TOP)
                for (int i = r1; i <= r2; i++)
                    if (cliptop[i] < ds->sprtopclip[i])
                        cliptop[i] = ds->sprtopclip[i];

            if (silhouette & SIL_BOTTOM)
                for (int i = r1; i <= r2; i++)
                    if (clipbot[i] > ds->sprbottomclip[i])
                        clipbot[i] = ds->sprbottomclip[i];
        }
    }

    // all clipping has been performed, so draw the blood splat
    R_DrawBloodSplatVisSprite(splat);
}

static void msort(vissprite_t **s, vissprite_t **t, unsigned int n)
{
    if (n >= 16)
    {
        unsigned int    n1 = n / 2;
        unsigned int    n2 = n - n1;
        vissprite_t     **s1 = s;
        vissprite_t     **s2 = s + n1;
        vissprite_t     **d = t;

        msort(s1, t, n1);
        msort(s2, t, n2);

        while ((*s1)->scale > (*s2)->scale ? (*d++ = *s1++, --n1) : (*d++ = *s2++, --n2));

        if (n2)
            memcpy(d, s2, n2 * sizeof(void *));
        else
            memcpy(d, s1, n1 * sizeof(void *));

        memcpy(s, t, n * sizeof(void *));
        return;
    }

    for (unsigned int i = 1; i < n; i++)
    {
        vissprite_t     *temp = s[i];
        const fixed_t   scale = temp->scale;

        if (s[i - 1]->scale < scale)
        {
            unsigned int    j = i;

            while ((s[j] = s[j - 1])->scale < scale && --j);

            s[j] = temp;
        }
    }
}

static void R_SortVisSprites(void)
{
    if (num_vissprite)
    {
        static unsigned int num_vissprite_ptrs;

        if (num_vissprite_ptrs < num_vissprite * 2)
            vissprite_ptrs = I_Realloc(vissprite_ptrs,
                (num_vissprite_ptrs = num_vissprite_alloc * 2) * sizeof(*vissprite_ptrs));

        for (int i = num_vissprite - 1; i >= 0; i--)
            vissprite_ptrs[i] = vissprites + i;

        msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

static void R_DrawSprite(const vissprite_t *spr)
{
    const int       x1 = spr->x1;
    const int       x2 = spr->x2;
    const fixed_t   scale = spr->scale;
    const fixed_t   gx = spr->gx;
    const fixed_t   gy = spr->gy;

    // initialize the clipping arrays
    for (int i = x1; i <= x2; i++)
    {
        cliptop[i] = -1;
        clipbot[i] = viewheight;
    }

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale is the clip seg.
    if (drawsegs_xrange_size)
    {
        const drawseg_xrange_item_t *last = &drawsegs_xrange[drawsegs_xrange_count - 1];
        drawseg_xrange_item_t       *curr = &drawsegs_xrange[-1];

        while (++curr <= last)
        {
            drawseg_t   *ds;
            int         silhouette;

            // determine if the drawseg obscures the sprite
            if (curr->x1 > spr->x2 || curr->x2 < spr->x1)
                continue;      // does not cover sprite

            ds = curr->user;
            silhouette = ds->silhouette;

            // determine if the drawseg obscures the sprite
            if (ds->x1 > x2 || ds->x2 < x1 || (!silhouette && !ds->maskedtexturecol))
                continue;

            if (ds->maxscale < scale || (ds->minscale < scale && !R_PointOnSegSide(gx, gy, ds->curline)))
            {
                // masked midtexture?
                if (ds->maskedtexturecol)
                    R_RenderMaskedSegRange(ds, MAX(ds->x1, x1), MIN(ds->x2, x2));

                // seg is behind sprite
                continue;
            }
            else
            {
                // clip this piece of the sprite
                const int   r1 = MAX(x1, ds->x1);
                const int   r2 = MIN(ds->x2, x2);

                if (silhouette & SIL_TOP)
                    for (int i = r1; i <= r2; i++)
                        if (cliptop[i] < ds->sprtopclip[i])
                            cliptop[i] = ds->sprtopclip[i];

                if (silhouette & SIL_BOTTOM)
                    for (int i = r1; i <= r2; i++)
                        if (clipbot[i] > ds->sprbottomclip[i])
                            clipbot[i] = ds->sprbottomclip[i];
            }
        }
    }

    // killough 03/27/98:
    // Clip the sprite against deep water and/or fake ceilings.
    // killough 04/09/98: optimize by adding mh
    // killough 04/11/98: improve sprite clipping for underwater/fake ceilings
    // killough 11/98: fix disappearing sprites
    if (spr->heightsec) // only things in specially marked sectors
    {
        fixed_t     h;
        fixed_t     mh = spr->heightsec->interpfloorheight;
        sector_t    *phs = viewplayer->mo->subsector->sector->heightsec;

        if (mh > spr->gz && (h = centeryfrac - FixedMul((mh -= viewz), scale)) >= 0 && (h >>= FRACBITS) < viewheight)
        {
            if (mh <= 0 || (phs && viewz > phs->interpfloorheight))
            {
                // clip bottom
                for (int i = x1; i <= x2; i++)
                    if (h < clipbot[i])
                        clipbot[i] = h;
            }
            else
                // clip top
                if (phs && viewz <= phs->interpfloorheight)
                    for (int i = x1; i <= x2; i++)
                        if (h > cliptop[i])
                            cliptop[i] = h;
        }

        if ((mh = spr->heightsec->interpceilingheight) < spr->gzt
            && (h = centeryfrac - FixedMul(mh - viewz, scale)) >= 0 && (h >>= FRACBITS) < viewheight)
        {
            if (phs && viewz >= phs->interpceilingheight)
            {
                // clip bottom
                for (int i = x1; i <= x2; i++)
                    if (h < clipbot[i])
                        clipbot[i] = h;
            }
            else
                // clip top
                for (int i = x1; i <= x2; i++)
                    if (h > cliptop[i])
                        cliptop[i] = h;
        }
    }

    // all clipping has been performed, so draw the sprite
    mceilingclip = cliptop;
    mfloorclip = clipbot;

    if (spr->shadowpos <= 0)
        R_DrawVisSpriteWithShadow(spr);
    else
        R_DrawVisSprite(spr);
}

//
// R_DrawMasked
//
void R_DrawMasked(void)
{
    interpolatesprites = (vid_capfps != TICRATE && !consoleactive && !freeze);
    invulnerable = (viewplayer->fixedcolormap == INVERSECOLORMAP && r_sprites_translucency);

    // draw all blood splats
    for (int i = num_bloodsplatvissprite - 1; i >= 0; i--)
        R_DrawBloodSplatSprite(&bloodsplatvissprites[i]);

    R_SortVisSprites();

    for (int i = 0; i < DS_RANGES_COUNT; i++)
        drawsegs_xranges[i].count = 0;

    if (num_vissprite > 0)
    {
        if (drawsegs_xrange_size < maxdrawsegs)
        {
            drawsegs_xrange_size = 2 * maxdrawsegs;

            for (int i = 0; i < DS_RANGES_COUNT; i++)
                drawsegs_xranges[i].items = I_Realloc(drawsegs_xranges[i].items,
                    drawsegs_xrange_size * sizeof(drawsegs_xranges[i].items[0]));
        }

        for (drawseg_t *ds = ds_p; ds-- > drawsegs; )
            if (ds->silhouette || ds->maskedtexturecol)
            {
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].x1 = ds->x1;
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].x2 = ds->x2;
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].user = ds;

                // e6y: ~13% of speed improvement on sunder.wad map10
                if (ds->x1 < centerx)
                {
                    drawsegs_xranges[1].items[drawsegs_xranges[1].count] = drawsegs_xranges[0].items[drawsegs_xranges[0].count];
                    drawsegs_xranges[1].count++;
                }

                if (ds->x2 >= centerx)
                {
                    drawsegs_xranges[2].items[drawsegs_xranges[2].count] = drawsegs_xranges[0].items[drawsegs_xranges[0].count];
                    drawsegs_xranges[2].count++;
                }

                drawsegs_xranges[0].count++;
            }
    }

    // draw all other vissprites back to front
    for (int i = num_vissprite - 1; i >= 0; i--)
    {
        vissprite_t *spr = vissprite_ptrs[i];

        if (spr->x2 < centerx)
        {
            drawsegs_xrange = drawsegs_xranges[1].items;
            drawsegs_xrange_count = drawsegs_xranges[1].count;
        }
        else if (spr->x1 >= centerx)
        {
            drawsegs_xrange = drawsegs_xranges[2].items;
            drawsegs_xrange_count = drawsegs_xranges[2].count;
        }
        else
        {
            drawsegs_xrange = drawsegs_xranges[0].items;
            drawsegs_xrange_count = drawsegs_xranges[0].count;
        }

        R_DrawSprite(spr);
    }

    // render any remaining masked midtextures
    for (drawseg_t *ds = ds_p; ds-- > drawsegs; )
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    if (r_playersprites && !menuactive)
        R_DrawPlayerSprites();
}
