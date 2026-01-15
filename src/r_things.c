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

#include "c_cmds.h"
#include "c_console.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_array.h"
#include "m_config.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAXSPRITEFRAMES 29
#define MINZ            (4 * FRACUNIT)
#define MAXZ            (8192 * FRACUNIT)
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

static mobj_t                   **nearby_sprites;

// constant arrays used for psprite clipping and initializing clipping
int                             negonearray[MAXWIDTH];
int                             viewheightarray[MAXWIDTH];
static int                      zeroarray[MAXWIDTH];

static int                      cliptop[MAXWIDTH];
static int                      clipbot[MAXWIDTH];

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up and range check thing_t sprites patches
spritedef_t                     *sprites;

short                           firstbloodsplatlump;
int                             numbloodsplatlumps;

bool                            allowwolfensteinss = true;

static spriteframe_t            sprtemp[MAXSPRITEFRAMES];
static int                      maxframe;

static bool                     drawshadows;
static bool                     interpolatesprites;
static bool                     invulnerable;

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

static lighttable_t *R_GetSectorColormap(sector_t *sector)
{
    if (sector->floorlightsec && sector->floorlightsec->colormap)
        return colormaps[sector->floorlightsec->colormap];
    else if (sector->heightsec && sector->heightsec->colormap)
        return colormaps[sector->heightsec->colormap];
    else if (sector->colormap)
        return colormaps[sector->colormap];
    else
        return fullcolormap;
}

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

                sprtemp[frame].rotate = 0;  // jff 04/24/98 if any subbed, rotless
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

        sprtemp[frame].rotate = 1;          // jff 04/24/98 only change if rot used
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
// 01/25/98, 01/31/98 killough: Rewritten for performance
//
// Empirically verified to have excellent hash properties across standard DOOM sprites:
#define R_SpriteNameHash(s) ((unsigned int)((s)[0] - ((s)[1] * 3 - (s)[3] * 2 - (s)[2]) * 2))

static void R_InitSpriteDefs(void)
{
    struct
    {
        int index;
        int next;
    } *hash;

    sprites = Z_Calloc(numsprites, sizeof(*sprites), PU_STATIC, NULL);

    // Create hash table based on just the first four letters of each sprite
    // killough 01/31/98
    if (!(hash = malloc(numspritelumps * sizeof(*hash))))
    {
        I_Error("R_InitSpriteDefs: Out of memory allocating sprite tables");
        return;
    }

    for (int i = 0; i < numspritelumps; i++)    // initialize hash table as empty
        hash[i].index = -1;

    for (int i = 0; i < numspritelumps; i++)    // Prepend each sprite to hash chain
    {
        const int   j = R_SpriteNameHash(lumpinfo[i + firstspritelump]->name) % numspritelumps;

        hash[i].next = hash[j].index;
        hash[j].index = i;
    }

    // scan all the lump names for each of the names, noting the highest frame letter.
    for (int i = 0; i < numsprites; i++)
    {
        const char  *spritename = sprnames[i];
        int         j;

        if (!spritename)
            continue;

        if ((j = hash[R_SpriteNameHash(spritename) % numspritelumps].index) >= 0)
        {
            memset(sprtemp, -1, sizeof(sprtemp));

            for (int k = 0; k < MAXSPRITEFRAMES; k++)
                sprtemp[k].flip = 0;

            maxframe = -1;

            do
            {
                const int           lumpnum = j + firstspritelump;
                const lumpinfo_t    *lump = lumpinfo[lumpnum];

                if (!lump->size)
                    continue;

                // Fast portable comparison -- killough
                // (using int pointer cast is non-portable):
                if (!((lump->name[0] ^ spritename[0]) | (lump->name[1] ^ spritename[1])
                    | (lump->name[2] ^ spritename[2]) | (lump->name[3] ^ spritename[3])))
                {
                    R_InstallSpriteLump(lumpnum, lump->name[4] - 'A', lump->name[5], false);

                    if (lump->name[6])
                        R_InstallSpriteLump(lumpnum, lump->name[6] - 'A', lump->name[7], true);
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
                                    I_Error("R_InitSprites: Frame %c of sprite %.8s is missing rotations",
                                        frame + 'A', sprnames[i]);

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
    numbloodsplatlumps = sprites[SPR_BLD2].numframes;

    // check if Wolfenstein SS sprites have been changed to zombiemen sprites
    if (bfgedition && gamemode == commercial)
    {
        if (!states[S_SSWV_STND].dehacked)
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
}

//
// GAME FUNCTIONS
//

static vissprite_t  *vissprites;
static vissprite_t  **vissprite_ptrs;
static unsigned int num_vissprite;
static unsigned int num_vissplat;
static unsigned int num_vissprite_alloc = MAXVISSPRITES;

static vissplat_t   vissplats[r_bloodsplats_max_max];

//
// R_InitSprites
// Called at program start.
//
void R_InitSprites(void)
{
    for (int i = 0; i < MAXWIDTH; i++)
    {
        negonearray[i] = -1;
        zeroarray[i] = 0;
    }

    R_InitSpriteDefs();

    vissprites = I_Malloc(num_vissprite_alloc * sizeof(*vissprites));
}

//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites(void)
{
    num_vissprite = 0;
    num_vissplat = 0;
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

int     *mfloorclip;
int     *mceilingclip;

fixed_t spryscale;
int64_t sprtopscreen;

static void (*shadowcolfunc)(void);

//
// R_BlastSpriteColumn
//
static void inline R_BlastSpriteColumn(const rcolumn_t *column)
{
    unsigned char   *pixels = column->pixels;
    const rpost_t   *posts = column->posts;

    while (dc_numposts--)
    {
        const rpost_t   *post = &posts[dc_numposts];
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
    const rpost_t   *posts = column->posts;

    while (dc_numposts--)
    {
        const rpost_t   *post = &posts[dc_numposts];
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
    bool            percolumnlighting;
    fixed_t         pcl_patchoffset = 0;
    fixed_t         pcl_cosine = 0;
    fixed_t         pcl_sine = 0;
    int             pcl_lightindex = 0;

    spryscale = vis->scale;

    dc_colormap[0] = vis->colormap;
    dc_nextcolormap[0] = vis->nextcolormap;
    dc_sectorcolormap = vis->sectorcolormap;
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

        if ((colfunc == bloodcolfunc || colfunc == translatedcolfunc) && mobj->bloodcolor > NOBLOOD)
            dc_translation = colortranslation[mobj->bloodcolor - 1];
    }

    sprtopscreen = (int64_t)centeryfrac - FixedMul(dc_texturemid, spryscale);
    baseclip = (vis->footclip ? (int)(sprtopscreen + vis->footclip) >> FRACBITS : viewheight);
    fuzz1pos = 0;

    if ((percolumnlighting = (r_percolumnlighting && !vis->fullbright && !fixedcolormap
        && (flags & (MF_SHOOTABLE | MF_CORPSE)))))
    {
        const int   angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;

        pcl_patchoffset = SHORT(patch->leftoffset) << FRACBITS;
        pcl_cosine = finecosine[angle];
        pcl_sine = finesine[angle];
        pcl_lightindex = MIN(spryscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1);
    }

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
    {
        const rcolumn_t *column = R_GetPatchColumnClamped(patch, frac >> FRACBITS);

        if ((dc_numposts = column->numposts))
        {
            dc_ceilingclip = mceilingclip[dc_x] + 1;
            dc_floorclip = MIN(baseclip, mfloorclip[dc_x]) - 1;

            if (percolumnlighting)
            {
                const fixed_t   offset = (vis->flipped ? pcl_patchoffset - frac : frac - pcl_patchoffset);
                const fixed_t   gx = vis->gx + FixedMul(offset, pcl_cosine);
                const fixed_t   gy = vis->gy + FixedMul(offset, pcl_sine);
                sector_t        *sector = R_PointInSubsector(gx, gy)->sector;
                sector_t        tempsec;
                int             floorlightlevel;
                int             ceilinglightlevel;
                int             lightnum;

                R_FakeFlat(sector, &tempsec, &floorlightlevel, &ceilinglightlevel, false);

                lightnum = ((floorlightlevel + ceilinglightlevel) >> (LIGHTSEGSHIFT + 1)) + extralight;

                dc_colormap[0] = scalelight[BETWEEN(0, lightnum - 2, LIGHTLEVELS - 1)][pcl_lightindex];
                dc_nextcolormap[0] = scalelight[BETWEEN(0, lightnum + 2, LIGHTLEVELS - 1)][pcl_lightindex];
                dc_sectorcolormap = R_GetSectorColormap(sector);
            }

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
    int             black;
    bool            percolumnlighting;
    fixed_t         pcl_patchoffset = 0;
    fixed_t         pcl_cosine = 0;
    fixed_t         pcl_sine = 0;
    int             pcl_lightindex = 0;
    int64_t         shadowtopscreen;
    int64_t         shadowspryscale;

    spryscale = vis->scale;

    dc_colormap[0] = vis->colormap;
    dc_nextcolormap[0] = vis->nextcolormap;
    dc_sectorcolormap = vis->sectorcolormap;
    dc_z = ((spryscale >> 5) & 255);
    dc_black = dc_colormap[0][nearestblack];
    black = dc_black << 8;

    if (flags & MF_FUZZ)
        dc_black33 = &tinttab15[black];
    else if (vis->fullbright)
    {
        dc_black33 = &tinttab20[black];
        dc_black40 = &tinttab25[black];
    }
    else if ((mobj->flags2 & (MF2_TRANSLUCENT_33 | MF2_EXPLODING)) && r_sprites_translucency)
    {
        dc_black33 = &tinttab10[black];
        dc_black40 = &tinttab25[black];
    }
    else
    {
        dc_black33 = &tinttab33[black];
        dc_black40 = &tinttab40[black];
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

        if (colfunc == translatedcolfunc && mobj->bloodcolor > NOBLOOD)
            dc_translation = colortranslation[mobj->bloodcolor - 1];
    }

    sprtopscreen = (int64_t)centeryfrac - FixedMul(dc_texturemid, spryscale);
    shadowcolfunc = mobj->shadowcolfunc;
    shadowtopscreen = (int64_t)centeryfrac - FixedMul(vis->shadowz, spryscale);
    shadowspryscale = (int64_t)spryscale / 10;

    fuzz1pos = 0;

    if ((percolumnlighting = (r_percolumnlighting && !vis->fullbright && !fixedcolormap
        && (flags & (MF_SHOOTABLE | MF_CORPSE)))))
    {
        const int   angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;

        pcl_patchoffset = SHORT(patch->leftoffset) << FRACBITS;
        pcl_cosine = finecosine[angle];
        pcl_sine = finesine[angle];
        pcl_lightindex = MIN(spryscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1);
    }

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
    {
        const rcolumn_t *column = R_GetPatchColumnClamped(patch, frac >> FRACBITS);

        if ((dc_numposts = column->numposts))
        {
            const rpost_t   *posts = column->posts;

            dc_ceilingclip = mceilingclip[dc_x] + 1;
            dc_floorclip = mfloorclip[dc_x] - 1;

            if (percolumnlighting)
            {
                const fixed_t   offset = (vis->flipped ? pcl_patchoffset - frac : frac - pcl_patchoffset);
                const fixed_t   gx = vis->gx + FixedMul(offset, pcl_cosine);
                const fixed_t   gy = vis->gy + FixedMul(offset, pcl_sine);
                sector_t        *sector = R_PointInSubsector(gx, gy)->sector;
                sector_t        tempsec;
                int             floorlightlevel;
                int             ceilinglightlevel;
                int             lightnum;

                R_FakeFlat(sector, &tempsec, &floorlightlevel, &ceilinglightlevel, false);

                lightnum = ((floorlightlevel + ceilinglightlevel) >> (LIGHTSEGSHIFT + 1)) + extralight;

                dc_colormap[0] = scalelight[BETWEEN(0, lightnum - 2, LIGHTLEVELS - 1)][pcl_lightindex];
                dc_nextcolormap[0] = scalelight[BETWEEN(0, lightnum + 2, LIGHTLEVELS - 1)][pcl_lightindex];
                dc_sectorcolormap = R_GetSectorColormap(sector);
            }

            while (dc_numposts--)
            {
                const rpost_t   *post = &posts[dc_numposts];
                const int64_t   topscreen = shadowtopscreen + shadowspryscale * post->topdelta;

                if ((dc_yh = MIN((int)((topscreen + shadowspryscale * post->length) >> FRACBITS), dc_floorclip)) >= 0)
                    if ((dc_yl = MAX(dc_ceilingclip, (int)((topscreen + FRACUNIT) >> FRACBITS))) <= dc_yh)
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
    dc_sectorcolormap = vis->sectorcolormap;
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
// R_DrawVisSplat
//
static void R_DrawVisSplat(const vissplat_t *vis)
{
    fixed_t         frac = vis->startfrac;
    const fixed_t   xiscale = vis->xiscale;
    const fixed_t   x2 = vis->x2;
    const rcolumn_t *columns = R_CachePatchNum(vis->patch)->columns;
    int64_t         splattopscreen;

    spryscale = vis->scale;
    colfunc = vis->colfunc;
    dc_bloodcolor = &tinttab50[(dc_solidbloodcolor = vis->colormap[vis->color]) << 8];
    dc_sectorcolormap = vis->sectorcolormap;
    splattopscreen = (int64_t)centeryfrac - FixedMul(vis->texturemid, spryscale);

    for (dc_x = vis->x1; dc_x <= x2; dc_x++, frac += xiscale)
    {
        const rcolumn_t *column = &columns[frac >> FRACBITS];

        if (column->numposts)
        {
            const rpost_t   *post = column->posts;
            const int64_t   topscreen = splattopscreen + spryscale * post->topdelta;

            if ((dc_yh = MIN((int)((topscreen + spryscale * post->length) >> FRACBITS), clipbot[dc_x] - 1)) >= 0)
                if ((dc_yl = MAX(cliptop[dc_x], (int)(topscreen >> FRACBITS))) <= dc_yh)
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

    // thing is behind view plane or too far away?
    if (tz < MINZ || tz > MAXZ)
        return;

    // too far off the side?
    // [ceski] [JN] Possibly use an extended value (fovtx) and prevent overflows (int64_t).
    if ((int64_t)(ABS((tx = FixedMul(tr_x, viewsin) - FixedMul(tr_y, viewcos))) >> fovtx) > ((int64_t)tz << 2))
        return;

    // decide which patch to use for sprite relative to player
    frame = thing->frame;
    sprframe = &sprites[thing->sprite].spriteframes[(frame & FF_FRAMEMASK)];

    if (((flags2 = thing->flags2) & MF2_FLOATBOB) && r_floatbob)
        fz += floatbobdiffs[((thing->floatbob + maptime) & 63)];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        const angle_t   ang = R_PointToAngle(fx, fy) - thing->angle + (angle_t)(ANG45 / 2) * 9;

        if (sprframe->lump[0] == sprframe->lump[1])
            rot = ang >> 28;
        else
            rot = (ang - (angle_t)(ANG180 / 16)) >> 28;

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
    if (fz > (int64_t)viewz + FixedDiv(viewheightfrac, xscale)
        || gzt < (int64_t)viewz - FixedDiv(viewheightfrac - viewheight, xscale))
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
        const sector_t  *phs = viewplayer->mo->subsector->sector->heightsec;

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
    vis->gz = thing->subsector->sector->interpfloorheight;
    vis->gzt = gzt;

    vis->flipped = flip;
    vis->fullbright = ((frame & FF_FULLBRIGHT) || thing->info->fullbright);

    if ((flags2 & MF2_CASTSHADOW) && xscale >= FRACUNIT / 4 && drawshadows)
        vis->shadowz = thing->floorz + thing->shadowoffset - viewz;
    else
        vis->shadowz = 1;

    vis->colfunc = (invulnerable && r_textures ? thing->altcolfunc : thing->colfunc);

    // foot clipping
    if ((flags2 & MF2_FEETARECLIPPED) && !heightsec && r_liquid_clipsprites && height >= 4 * FRACUNIT)
    {
        if (topoffset - height <= 4 * FRACUNIT)
        {
            fixed_t clipfeet = MIN((height >> FRACBITS) / 4, 10) << FRACBITS;

            vis->texturemid = gzt - viewz - clipfeet;

            if (r_liquid_bobsprites)
                clipfeet += animatedliquiddiff;

            vis->footclip = FixedMul(height - clipfeet, xscale);
        }
        else
        {
            vis->texturemid = gzt - FOOTCLIPSIZE - viewz;
            vis->footclip = 0;
        }
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
    if (viewplayer->fixedcolormap == INVERSECOLORMAP)
    {
        // fixed map
        vis->colormap = fixedcolormap;
        vis->nextcolormap = fixedcolormap;
        vis->sectorcolormap = fullcolormap;
    }
    else if (((frame & FF_FULLBRIGHT) && (rot <= 5 || rot >= 12 || thing->info->fullbright))
        || viewplayer->fixedcolormap == 1)
    {
        // full bright
        vis->colormap = fullcolormap;
        vis->nextcolormap = fullcolormap;
        vis->sectorcolormap = R_GetSectorColormap(thing->subsector->sector);
    }
    else
    {
        // diminished light
        const int   i = MIN(xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1);

        vis->colormap = spritelights[i];
        vis->nextcolormap = nextspritelights[i];
        vis->sectorcolormap = R_GetSectorColormap(thing->subsector->sector);
    }
}

static void R_ProjectBloodSplat(const bloodsplat_t *splat)
{
    fixed_t         tx;
    fixed_t         xscale;
    int             x1;
    int             x2;
    vissplat_t      *vis;
    const fixed_t   fx = splat->x;
    const fixed_t   fy = splat->y;
    fixed_t         width;
    const fixed_t   tr_x = fx - viewx;
    const fixed_t   tr_y = fy - viewy;
    const fixed_t   tz = FixedMul(tr_x, viewcos) + FixedMul(tr_y, viewsin);

    // splat is behind view plane or too far away?
    if (tz < MINZ || tz > MAXZ / 8)
        return;

    // too far off the side?
    if ((int64_t)ABS((tx = FixedMul(tr_x, viewsin) - FixedMul(tr_y, viewcos))) > ((int64_t)tz << 2))
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

    if (num_vissplat >= r_bloodsplats_max_max)
        return;

    // store information in a vissplat
    vis = &vissplats[num_vissplat++];

    vis->scale = xscale;
    vis->gx = fx;
    vis->gy = fy;
    vis->color = (r_textures ? splat->viscolor : nearestlightgray);
    vis->colfunc = splat->viscolfunc;
    vis->texturemid = splat->sector->interpfloorheight + FRACUNIT - viewz;
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
    if (viewplayer->fixedcolormap == INVERSECOLORMAP)
    {
        vis->colormap = fixedcolormap;
        vis->sectorcolormap = fullcolormap;
    }
    else
    {
        vis->colormap = (viewplayer->fixedcolormap == 1 ? fixedcolormap :
            spritelights[MIN(xscale >> LIGHTSCALESHIFT, MAXLIGHTSCALE - 1)]);
        vis->sectorcolormap = R_GetSectorColormap(splat->sector);
    }
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
// killough 09/18/98: add lightlevel as parameter, fixing underwater lighting
void R_AddSprites(sector_t *sec, int lightlevel)
{
    mobj_t  *thing = sec->thinglist;

    if (!thing)
        return;

    spritelights = scalelight[BETWEEN(0, ((lightlevel - 2) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
    nextspritelights = scalelight[BETWEEN(0, ((lightlevel + 2) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
    drawshadows = (sec->terraintype == SOLID && !fixedcolormap && r_shadows);

    // Handle all things in sector.
    do
    {
        if (!menuactive || ((thing->flags & MF_SOLID) && (!(thing->flags & MF_SHOOTABLE) || thing->type == MT_BARREL)))
            R_ProjectSprite(thing);

        thing = thing->snext;
    } while (thing);
}

void R_AddNearbySprites(sector_t *sec)
{
    for (msecnode_t *n = sec->touching_thinglist; n; n = n->m_snext)
    {
        mobj_t  *thing = n->m_thing;

        if (thing->subsector->sector->validcount != validcount)
            array_push(nearby_sprites, thing);
    }
}

void R_DrawNearbySprites(void)
{
    const int   size = array_size(nearby_sprites);

    for (int i = 0; i < size; i++)
    {
        mobj_t      *thing = nearby_sprites[i];
        sector_t    *sec = thing->subsector->sector;

        // [FG] sprites in sector have already been projected
        if (sec->validcount != validcount)
        {
            const short lightlevel = sec->lightlevel;

            spritelights = scalelight[BETWEEN(0, ((lightlevel - 2) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
            nextspritelights = scalelight[BETWEEN(0, ((lightlevel + 2) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];

            R_ProjectSprite(thing);
        }
    }

    array_clear(nearby_sprites);
}

static void R_AddBloodSplats(void)
{
    const int       radius = (int)((MAXZ / 8 + MAPBLOCKSIZE - 1) / MAPBLOCKSIZE);
    const int       cx = (viewx - bmaporgx) >> MAPBLOCKSHIFT;
    const int       cy = (viewy - bmaporgy) >> MAPBLOCKSHIFT;
    const int       minx = MAX(0, cx - radius);
    const int       maxx = MIN(bmapwidth - 1, cx + radius);
    const int       miny = MAX(0, cy - radius);
    const int       maxy = MIN(bmapheight - 1, cy + radius);

    int             prevlightlevel = INT_MIN;
    lighttable_t    **cachedspritelights[256];
    lighttable_t    **cachednextspritelights[256];
    bool            cached[256];

    memset(cached, 0, sizeof(cached));

    for (int y = miny; y <= maxy; y++)
        for (int x = minx; x <= maxx; x++)
            for (bloodsplat_t *splat = bloodsplat_blocklinks[y * bmapwidth + x]; splat; splat = splat->bnext)
            {
                sector_t    *sector = splat->sector;
                short       lightlevel = (sector->floorlightsec ? sector->floorlightsec->lightlevel : sector->lightlevel);

                if (lightlevel != prevlightlevel)
                {
                    const int   i = BETWEEN(0, lightlevel, 255);

                    if (!cached[i])
                    {
                        cachedspritelights[i] = scalelight[BETWEEN(0, ((lightlevel - 2) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
                        cachednextspritelights[i] = scalelight[BETWEEN(0, ((lightlevel + 2) >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
                        cached[i] = true;
                    }

                    spritelights = cachedspritelights[i];
                    nextspritelights = cachednextspritelights[i];

                    prevlightlevel = lightlevel;
                }

                R_ProjectBloodSplat(splat);
            }
}

//
// R_DrawPlayerSprite
//
static bool muzzleflash;

static void R_DrawPlayerSprite(const pspdef_t *psp, bool invisibility, bool altered)
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
            const int   deltax = x2 - vis->x1;

            vis->x1 = psp_inter.x1 + FixedMul(vis->x1 - psp_inter.x1, fractionaltic);
            vis->x2 = MIN(vis->x1 + deltax, viewwidth);
            vis->texturemid = psp_inter.texturemid + FixedMul(vis->texturemid - psp_inter.texturemid, fractionaltic);
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

    if (freelook && r_screensize < r_screensize_max)
        vis->texturemid -= viewplayer->pitch * 0x0520;

    if (invisibility)
    {
        vis->colfunc = (r_textures ? psprcolfunc : &R_DrawTranslucent50SolidColorColumn);
        vis->colormap = NULL;
        vis->sectorcolormap = fullcolormap;
    }
    else
    {
        sector_t    *sec = viewplayer->mo->subsector->sector;

        if (r_sprites_translucency)
        {
            if (!r_textures)
            {
                vis->colfunc = (psp == &viewplayer->psprites[1] ? &R_DrawTranslucent50SolidColorColumn : &R_DrawSolidColorColumn);
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
            else if (legacyofrust && spr == 283)
                vis->colfunc = tlredwhitecolfunc1;
            else
                vis->colfunc = basecolfunc;
        }
        else
        {
            if (!r_textures)
            {
                vis->colfunc = &R_DrawSolidColorColumn;
                vis->colormap = NULL;
            }
            else
                vis->colfunc = basecolfunc;
        }

        if (fixedcolormap)
        {
            vis->colormap = fixedcolormap;       // fixed color
            vis->sectorcolormap = (viewplayer->fixedcolormap != INVERSECOLORMAP ?
                R_GetSectorColormap(sec) : fullcolormap);
        }
        else
        {
            if (muzzleflash || (frame & FF_FULLBRIGHT))
                vis->colormap = fullcolormap;    // full bright
            else
            {
                const int   lightnum = ((sec->floorlightsec ? sec->floorlightsec : sec)->lightlevel >> OLDLIGHTSEGSHIFT) + extralight;

                vis->colormap = psprscalelight[MIN(lightnum, OLDLIGHTLEVELS - 1)][MIN(lightnum + 16, OLDMAXLIGHTSCALE - 1)];
            }

            vis->sectorcolormap = R_GetSectorColormap(sec);
        }
    }

    R_DrawPlayerVisSprite(vis);
}

//
// R_DrawPlayerSprites
//
static void R_DrawPlayerSprites(void)
{
    const int       invisibility = viewplayer->powers[pw_invisibility];
    const pspdef_t  *weapon = viewplayer->psprites;
    const pspdef_t  *flash = weapon + 1;
    const state_t   *weaponstate = weapon->state;
    const state_t   *flashstate = flash->state;
    bool            altered;

    if (!weaponstate)
        return;

    altered = (weaponinfo[viewplayer->readyweapon].altered || weaponstate->dehacked || !r_fixspriteoffsets);

    // add all active psprites
    if (invisibility && (invisibility > STARTFLASHING || (invisibility & FLASHONTIC)))
    {
        fuzz2pos = 0;

        V_FillRect(1, viewwindowx, viewwindowy, viewwidth, viewheight, PINK, 0, false, false, NULL, NULL);

        if (flashstate)
        {
            altered |= flashstate->dehacked;

            R_DrawPlayerSprite(weapon, true, altered);
            R_DrawPlayerSprite(flash, true, altered);
        }
        else
            R_DrawPlayerSprite(weapon, true, altered);

        R_DrawFuzzColumns();
    }
    else
    {
        muzzleflash = (weaponstate->frame & FF_FULLBRIGHT);

        if (flashstate)
        {
            altered |= flashstate->dehacked;
            muzzleflash |= (flashstate->frame & FF_FULLBRIGHT);

            R_DrawPlayerSprite(weapon, false, altered);
            R_DrawPlayerSprite(flash, false, altered);
        }
        else
            R_DrawPlayerSprite(weapon, false, altered);
    }
}

//
// R_DrawBloodSplatSprite
//
static void R_DrawBloodSplatSprite(const vissplat_t *splat)
{
    int             x1 = splat->x1;
    int             x2 = splat->x2;
    const fixed_t   scale = splat->scale;
    const fixed_t   gx = splat->gx;
    const fixed_t   gy = splat->gy;

    if (x2 < 0 || x1 >= viewwidth)
        return;

    if ((x1 = MAX(0, x1)) > (x2 = MIN(viewwidth - 1, x2)))
        return;

    // initialize the clipping arrays
    memcpy(cliptop + x1, zeroarray + x1, (x2 - x1 + 1) * sizeof(cliptop[0]));
    memcpy(clipbot + x1, viewheightarray + x1, (x2 - x1 + 1) * sizeof(clipbot[0]));

    // Scan drawsegs using the same xrange acceleration as sprites.
    if (drawsegs_xrange_count > 0)
    {
        const drawseg_xrange_item_t *last = &drawsegs_xrange[drawsegs_xrange_count - 1];
        drawseg_xrange_item_t       *curr = &drawsegs_xrange[-1];

        while (++curr <= last)
        {
            drawseg_t   *ds;
            int         silhouette;

            if (curr->x1 > x2 || curr->x2 < x1)
                continue;

            ds = curr->user;
            silhouette = ds->silhouette;

            if (ds->x1 > x2 || ds->x2 < x1 || (!silhouette && !ds->maskedtexturecol))
                continue;

            if (ds->maxscale < scale || (ds->minscale < scale && !R_PointOnSegSide(gx, gy, ds->curline)))
                continue;

            const int r1 = MAX(x1, ds->x1);
            const int r2 = MIN(ds->x2, x2);

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
    R_DrawVisSplat(splat);
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

        // [FG] change '<' to '<=' here and below, so that vissprites with the same scale
        // are reordered, and so that the object with the higher map index appears in front
        if (s[i - 1]->scale <= scale)
        {
            unsigned int    j = i;

            while ((s[j] = s[j - 1])->scale <= scale && --j);

            s[j] = temp;
        }
    }
}

static void R_SortVisSprites(void)
{
    static unsigned int num_vissprite_ptrs;

    if (num_vissprite_ptrs < num_vissprite * 2)
        vissprite_ptrs = I_Realloc(vissprite_ptrs,
            (num_vissprite_ptrs = num_vissprite_alloc * 2) * sizeof(*vissprite_ptrs));

    for (int i = num_vissprite - 1; i >= 0; i--)
        vissprite_ptrs[i] = vissprites + i;

    msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
}

static void R_DrawSprite(const vissprite_t *spr)
{
    const int       x1 = spr->x1;
    const int       x2 = spr->x2;
    const fixed_t   scale = spr->scale;
    const fixed_t   gx = spr->gx;
    const fixed_t   gy = spr->gy;

    // initialize the clipping arrays
    memcpy(cliptop + x1, negonearray + x1, (x2 - x1 + 1) * sizeof(cliptop[0]));
    memcpy(clipbot + x1, viewheightarray + x1, (x2 - x1 + 1) * sizeof(clipbot[0]));

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
                    R_RenderMaskedSegRange(ds, MAX(x1, ds->x1), MIN(ds->x2, x2));

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
        fixed_t         h;
        fixed_t         mh = spr->heightsec->interpfloorheight;
        const sector_t  *phs = viewplayer->mo->subsector->sector->heightsec;

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

    if (spr->shadowz == 1)
        R_DrawVisSprite(spr);
    else
        R_DrawVisSpriteWithShadow(spr);
}

//
// R_DrawMasked
//
void R_DrawMasked(void)
{
    if (consoleactive || paused || freeze)
    {
        M_Fuzz1Seed(maptime);
        M_Fuzz2Seed(maptime);
    }

    interpolatesprites = (vid_capfps != TICRATE && !consoleactive && !freeze);
    invulnerable = (viewplayer->fixedcolormap == INVERSECOLORMAP && r_sprites_translucency);

    if (drawbloodsplats && bloodsplat_blocklinks && !menuactive)
        R_AddBloodSplats();

    if (!num_vissplat && !num_vissprite)
    {
        // render any remaining masked midtextures
        for (drawseg_t *ds = ds_p; ds-- > drawsegs; )
            if (ds->maskedtexturecol)
                R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

        if (r_playersprites && !menuactive)
            R_DrawPlayerSprites();

        return;
    }

    // Prepare drawseg xranges once (used by both splats and sprites).
    if (drawsegs_xrange_size < maxdrawsegs)
    {
        drawsegs_xrange_size = 2 * maxdrawsegs;

        for (int i = 0; i < DS_RANGES_COUNT; i++)
            drawsegs_xranges[i].items = I_Realloc(drawsegs_xranges[i].items,
                drawsegs_xrange_size * sizeof(drawsegs_xranges[i].items[0]));
    }

    for (int i = 0; i < DS_RANGES_COUNT; i++)
        drawsegs_xranges[i].count = 0;

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

    // draw all blood splats
    for (int i = num_vissplat - 1; i >= 0; i--)
    {
        const vissplat_t    *splat = &vissplats[i];

        if (splat->x2 < centerx)
        {
            drawsegs_xrange = drawsegs_xranges[1].items;
            drawsegs_xrange_count = drawsegs_xranges[1].count;
        }
        else if (splat->x1 >= centerx)
        {
            drawsegs_xrange = drawsegs_xranges[2].items;
            drawsegs_xrange_count = drawsegs_xranges[2].count;
        }
        else
        {
            drawsegs_xrange = drawsegs_xranges[0].items;
            drawsegs_xrange_count = drawsegs_xranges[0].count;
        }

        R_DrawBloodSplatSprite(splat);
    }

    if (num_vissprite)
    {
        R_SortVisSprites();

        // draw all other vissprites back to front
        for (int i = num_vissprite - 1; i >= 0; i--)
        {
            const vissprite_t   *spr = vissprite_ptrs[i];

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
    }

    // render any remaining masked midtextures
    for (drawseg_t *ds = ds_p; ds-- > drawsegs; )
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    if (r_playersprites && !menuactive)
        R_DrawPlayerSprites();
}
