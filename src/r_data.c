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

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_local.h"
#include "r_sky.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//

static int  firstcolormaplump;

int         firstflat;
static int  lastflat;
int         numflats;

static int  missingflatnum;

int         firstspritelump;
int         lastspritelump;
int         numspritelumps;

bool        fixspriteoffsets = false;
bool        suppresswarnings = false;

int         numtextures;
texture_t   **textures;

char        berserk[64];

// needed for texture pegging
fixed_t     *textureheight;

byte        **brightmap;
bool        *nobrightmap;
byte        (*masks)[256];
char        (*masknames)[32];

// for global animation
int         *flattranslation;
int         *texturetranslation;

// needed for prerendering
fixed_t     *spritewidth;
fixed_t     *spriteheight;
fixed_t     *spriteoffset;
fixed_t     *spritetopoffset;

fixed_t     *newspriteoffset;
fixed_t     *newspritetopoffset;

byte        grays[256];

//
// R_GetTextureColumn
//
byte *R_GetTextureColumn(const rpatch_t *texpatch, int col)
{
    while (col < 0)
        col += texpatch->width;

    return texpatch->columns[(col & texpatch->widthmask)].pixels;
}

//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
static void R_InitTextures(void)
{
    typedef struct
    {
        void            *names;
        short           nummappatches;
        char            *name_p;
    } pnameslump_t;

    pnameslump_t        *pnameslumps = malloc(sizeof(pnameslump_t));
    int                 maxpnameslumps = 1;
    int                 numpnameslumps = 0;
    const maptexture_t  *mtexture;
    texture_t           *texture;
    int                 maptex_lump[] = { -1, -1 };
    const int           *maptex1;
    const int           *maptex2 = NULL;
    char                name[9];
    int                 *patchlookup;
    int                 nummappatches = 0;
    int                 maxoff;
    int                 maxoff2 = 0;
    int                 numtextures1;
    int                 numtextures2 = 0;
    const int           *directory;

    for (int i = numlumps - 1; i >= 0; i--)
        if (!strncasecmp(lumpinfo[i]->name, "PNAMES", 6))
        {
            if (numpnameslumps == maxpnameslumps)
            {
                maxpnameslumps++;
                pnameslumps = I_Realloc(pnameslumps, maxpnameslumps * sizeof(pnameslump_t));
            }

            pnameslumps[numpnameslumps].names = W_CacheLumpNum(i);
            pnameslumps[numpnameslumps].nummappatches = LONG(*((int *)pnameslumps[numpnameslumps].names));

            // [crispy] accumulated number of patches in the lookup tables excluding the current one
            pnameslumps[numpnameslumps].name_p = (char *)pnameslumps[numpnameslumps].names + 4;

            // [crispy] calculate total number of patches
            nummappatches += pnameslumps[numpnameslumps].nummappatches;
            numpnameslumps++;
        }

    patchlookup = malloc(nummappatches * sizeof(*patchlookup)); // killough

    for (int i = 0, patch = 0; i < numpnameslumps; i++)
        for (int j = 0; j < pnameslumps[i].nummappatches; j++)
        {
            int p1;
            int p2;

            M_StringCopy(name, &pnameslumps[i].name_p[j * 8], sizeof(name));
            p1 = p2 = W_CheckNumForName(name);

            // [crispy] prevent flat lumps from being mistaken as patches
            while (p2 >= firstflat && p2 <= lastflat)
                p2 = W_RangeCheckNumForName(0, p2 - 1, name);

            patchlookup[patch++] = (p2 >= 0 ? p2 : p1);
        }

    free(pnameslumps);

    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex_lump[0] = W_GetNumForName("TEXTURE1");
    maptex1 = W_CacheLumpNum(maptex_lump[0]);
    numtextures1 = LONG(*maptex1);
    maxoff = W_LumpLength(maptex_lump[0]);
    directory = maptex1 + 1;

    if (W_CheckNumForName("TEXTURE2") >= 0)
    {
        maptex_lump[1] = W_GetNumForName("TEXTURE2");
        maptex2 = W_CacheLumpNum(maptex_lump[1]);
        numtextures2 = LONG(*maptex2);
        maxoff2 = W_LumpLength(maptex_lump[1]);
    }

    numtextures = numtextures1 + numtextures2;

    // killough 04/09/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof
    textures = Z_Malloc(numtextures * sizeof(*textures), PU_STATIC, NULL);
    textureheight = Z_Malloc(numtextures * sizeof(*textureheight), PU_STATIC, NULL);

    for (int i = 0; i < numtextures; i++, directory++)
    {
        const mappatch_t    *mpatch;
        texpatch_t          *patch;
        int                 offset;
        short               mask;

        if (i == numtextures1)
        {
            // Start looking in second texture file.
            maptex1 = maptex2;
            maxoff = maxoff2;
            directory = maptex1 + 1;
        }

        offset = LONG(*directory);

        if (offset > maxoff)
            I_Error("R_InitTextures: Bad texture directory");

        mtexture = (const maptexture_t *)((const byte *)maptex1 + offset);

        texture = textures[i] = Z_Malloc(sizeof(texture_t) + ((size_t)SHORT(mtexture->patchcount) - 1) * sizeof(texpatch_t),
            PU_STATIC, 0);

        texture->width = SHORT(mtexture->width);
        texture->height = SHORT(mtexture->height);
        texture->patchcount = SHORT(mtexture->patchcount);

        for (int j = 0; j < sizeof(texture->name); j++)
            texture->name[j] = mtexture->name[j];

        mpatch = mtexture->patches;
        patch = texture->patches;

        for (int j = 0; j < texture->patchcount; j++, mpatch++, patch++)
        {
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);
            patch->patch = patchlookup[SHORT(mpatch->patch)];

            if (patch->patch == -1)
                I_Error("R_InitTextures: Patch %i is missing in the %.8s texture",
                    SHORT(mpatch->patch), uppercase(texture->name));
        }

        for (mask = 1; mask * 2 <= texture->width; mask *= 2);

        texture->widthmask = mask - 1;
        textureheight[i] = texture->height << FRACBITS;
    }

    free(patchlookup);

    if (maptex_lump[0] != -1)
        W_ReleaseLumpNum(maptex_lump[0]);

    if (maptex_lump[1] != -1)
        W_ReleaseLumpNum(maptex_lump[1]);

    // Create translation table for global animation.
    // killough 04/09/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof
    texturetranslation = Z_Malloc(((size_t)numtextures + 1) * sizeof(*texturetranslation), PU_STATIC, NULL);

    for (int i = 0; i < numtextures; i++)
    {
        texturetranslation[i] = i;
        textures[i]->index = -1;    // killough 01/31/98: Initialize texture hash table
    }

    for (int i = numtextures - 1; i >= 0; i--)
    {
        const int   j = W_LumpNameHash(textures[i]->name) % numtextures;

        textures[i]->next = textures[j]->index; // Prepend to chain
        textures[j]->index = i;
    }
}

//
// R_InitBrightmaps
//
static void R_InitBrightmaps(void)
{
    int nummasks = 0;
    int numbrightmaps = 0;

    brightmap = Z_Calloc(numtextures, 256, PU_STATIC, NULL);
    nobrightmap = Z_Calloc(numtextures, sizeof(*nobrightmap), PU_STATIC, NULL);

    if (BTSX || chex || FREEDOOM || hacx || harmony || REKKR)
        return;

    masks = Z_Calloc(numtextures, sizeof(*masks), PU_STATIC, NULL);
    masknames = Z_Calloc(numtextures, sizeof(*masknames), PU_STATIC, NULL);

    SC_Open("BRGHTMPS");

    while (SC_GetString())
        if (SC_Compare("BRIGHTMAP"))
        {
            char    colors[1024];
            char    *p;

            SC_MustGetString();
            M_StringCopy(masknames[nummasks], sc_String, sizeof(masknames[0]));

            SC_MustGetString();
            M_StringCopy(colors, sc_String, sizeof(colors));

            p = strtok(colors, ",");

            while (p)
            {
                int color1, color2;

                if (sscanf(p, "%i-%i", &color1, &color2) == 2)
                {
                    if ((color1 = MIN(color1, 255)) >= 0)
                        while (color1 <= color2)
                            masks[nummasks][color1++] = 1;
                }
                else if (sscanf(p, "%i", &color1) == 1)
                {
                    if (color1 >= 0 && color1 <= 255)
                        masks[nummasks][color1] = 1;
                }

                p = strtok(NULL, ",");
            }

            nummasks++;
        }
        else if (SC_Compare("TEXTURE"))
        {
            int     texture;
            char    maskname[32];

            SC_MustGetString();
            texture = R_CheckTextureNumForName(sc_String);

            SC_MustGetString();
            M_StringCopy(maskname, sc_String, sizeof(maskname));

            SC_GetString();

            if (SC_Compare("TEXTURE") || SC_Compare("SPRITE") || SC_Compare("FLAT") || SC_Compare("STATE"))
            {
                SC_UnGet();
                *sc_String = '\0';
            }

            if (texture >= 0
                && (!*sc_String || SC_Compare("0") || SC_Compare("DOOM|DOOM2") || SC_Compare("DOOM1|DOOM2")
                    || (gamemission == doom && !SC_Compare("2") && !SC_Compare("DOOM2"))
                    || (gamemission != doom && !SC_Compare("1") && !SC_Compare("DOOM") && !SC_Compare("DOOM1"))))
                for (int i = 0; i < nummasks; i++)
                    if (M_StringCompare(maskname, masknames[i]))
                    {
                        brightmap[texture] = masks[i];
                        numbrightmaps++;
                        break;
                    }
        }
        else if (SC_Compare("SPRITE") || SC_Compare("FLAT") || SC_Compare("STATE"))
        {
            SC_MustGetString();
            SC_MustGetString();
            SC_GetString();

            if (SC_Compare("TEXTURE") || SC_Compare("SPRITE") || SC_Compare("FLAT") || SC_Compare("STATE"))
                SC_UnGet();
        }

    SC_Close();

    SC_Open("DRCOMPAT");

    while (SC_GetString())
        if (SC_Compare("NOBRIGHTMAP"))
        {
            int texture;

            SC_MustGetString();
            texture = R_TextureNumForName(sc_String);

            SC_MustGetString();

            if (texture >= 0 && SC_Compare(pwadfile))
            {
                nobrightmap[texture] = true;
                numbrightmaps--;
            }
        }

    SC_Close();

    if (r_brightmaps && numbrightmaps > 0)
    {
        char    *temp = commify(numbrightmaps);

        C_Output("Brightmaps have been applied to %s texture%s.",
            temp, (numbrightmaps == 1 ? "" : "s"));
        free(temp);
    }
}

//
// R_InitFlats
//
static void R_InitFlats(void)
{
    firstflat = W_GetNumForName("F_START") + 1;
    lastflat = W_GetNumForName("F_END") - 1;
    numflats = lastflat - firstflat + 1;

    // Create translation table for global animation.
    flattranslation = Z_Malloc(((size_t)numflats + 1) * sizeof(*flattranslation), PU_STATIC, NULL);

    for (int i = 0; i < numflats; i++)
        flattranslation[i] = firstflat + i;

    missingflatnum = R_FlatNumForName("-N0_TEX-");
}

//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the WAD,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
static void R_InitSpriteLumps(void)
{
    SC_Open("DRCOMPAT");

    while (SC_GetString())
        if (SC_Compare("FIXSPRITEOFFSETS"))
        {
            SC_MustGetString();

            if (SC_Compare(pwadfile))
                fixspriteoffsets = true;
        }
        else if (SC_Compare("NOBLUEGREENBLOOD"))
        {
            SC_MustGetString();

            if (SC_Compare(pwadfile))
            {
                mobjinfo[MT_HEAD].bloodcolor = REDBLOOD;
                mobjinfo[MT_BRUISER].bloodcolor = REDBLOOD;
                mobjinfo[MT_KNIGHT].bloodcolor = REDBLOOD;
            }
        }

    SC_Close();

    firstspritelump = W_GetNumForName("S_START") + 1;
    lastspritelump = W_GetNumForName("S_END") - 1;

    numspritelumps = lastspritelump - firstspritelump + 1;
    spritewidth = Z_Malloc(numspritelumps * sizeof(*spritewidth), PU_STATIC, NULL);
    spriteheight = Z_Malloc(numspritelumps * sizeof(*spriteheight), PU_STATIC, NULL);
    spriteoffset = Z_Malloc(numspritelumps * sizeof(*spriteoffset), PU_STATIC, NULL);
    spritetopoffset = Z_Malloc(numspritelumps * sizeof(*spritetopoffset), PU_STATIC, NULL);

    newspriteoffset = Z_Malloc(numspritelumps * sizeof(*newspriteoffset), PU_STATIC, NULL);
    newspritetopoffset = Z_Malloc(numspritelumps * sizeof(*newspritetopoffset), PU_STATIC, NULL);

    for (int i = 0; i < numspritelumps; i++)
    {
        patch_t *patch = W_CacheLumpNum(firstspritelump + i);

        if (patch)
        {
            spritewidth[i] = SHORT(patch->width) << FRACBITS;
            spriteheight[i] = SHORT(patch->height) << FRACBITS;
            spriteoffset[i] = newspriteoffset[i] = SHORT(patch->leftoffset) << FRACBITS;
            spritetopoffset[i] = newspritetopoffset[i] = SHORT(patch->topoffset) << FRACBITS;

            // [BH] override sprite offsets in WAD with those in sproffsets[] in info.c
            if (!FREEDOOM && !chex && !hacx)
                for (int j = 0; *sproffsets[j].name; j++)
                    if (i == W_CheckNumForName(sproffsets[j].name) - firstspritelump
                        && spritewidth[i] == (SHORT(sproffsets[j].width) << FRACBITS)
                        && spriteheight[i] == (SHORT(sproffsets[j].height) << FRACBITS)
                        && ((!BTSX && !sprfix18) || sproffsets[j].sprfix18)
                        && (fixspriteoffsets || lumpinfo[firstspritelump + i]->wadfile->type == IWAD
                            || M_StringEndsWith(lumpinfo[firstspritelump + i]->wadfile->path, DOOMRETRO_RESOURCEWAD)))
                    {
                        newspriteoffset[i] = SHORT(sproffsets[j].x) << FRACBITS;
                        newspritetopoffset[i] = SHORT(sproffsets[j].y) << FRACBITS;
                        break;
                    }
        }
    }

    M_StringCopy(berserk, M_StringReplace(powerups[pw_strength], " power-up", ""), sizeof(berserk));

    // [BH] compatibility fixes
    if (FREEDOOM)
    {
        s_M_EPISODE1 = M_StringDuplicate("Outpost Outbreak");
        s_M_EPISODE2 = M_StringDuplicate("Military Labs");
        s_M_EPISODE3 = M_StringDuplicate("Event Horizon");
        s_M_EPISODE4 = M_StringDuplicate("Double Impact");

        s_M_SKILLLEVEL1 = M_StringDuplicate("Please don't kill me!");
        s_M_SKILLLEVEL2 = M_StringDuplicate("Will this hurt?");
        s_M_SKILLLEVEL3 = M_StringDuplicate("Bring on the pain.");
        s_M_SKILLLEVEL4 = M_StringDuplicate("Extreme carnage");
        s_M_SKILLLEVEL5 = M_StringDuplicate("Insanity!");

        states[S_BAR1].nextstate = S_BAR2;
        mobjinfo[MT_BARREL].frames = 2;

        mobjinfo[MT_HEAD].bloodcolor = REDBLOOD;
        mobjinfo[MT_BRUISER].bloodcolor = REDBLOOD;
        mobjinfo[MT_KNIGHT].bloodcolor = REDBLOOD;

        weaponinfo[wp_pistol].name = M_StringDuplicate("handgun");
        weaponinfo[wp_shotgun].name = M_StringDuplicate("pump-action shotgun");
        weaponinfo[wp_chaingun].name = M_StringDuplicate("minigun");
        weaponinfo[wp_missile].name = M_StringDuplicate("missile launcher");
        weaponinfo[wp_plasma].name = M_StringDuplicate("polaric energy cannon");
        weaponinfo[wp_bfg].name = M_StringDuplicate("SKAG 1337");
        weaponinfo[wp_chainsaw].name = M_StringDuplicate("angle grinder");
        weaponinfo[wp_supershotgun].name = M_StringDuplicate("double-barreled shotgun");

        M_StringCopy(weaponinfo[wp_missile].ammoname, "missile", sizeof(weaponinfo[0].ammoname));
        M_StringCopy(weaponinfo[wp_missile].ammoplural, "missiles", sizeof(weaponinfo[0].ammoplural));
        M_StringCopy(weaponinfo[wp_plasma].ammoname, "polaric recharge", sizeof(weaponinfo[0].ammoname));
        M_StringCopy(weaponinfo[wp_plasma].ammoplural, "polaric recharges", sizeof(weaponinfo[0].ammoplural));
        M_StringCopy(weaponinfo[wp_bfg].ammoname, "polaric recharge", sizeof(weaponinfo[0].ammoname));
        M_StringCopy(weaponinfo[wp_bfg].ammoplural, "polaric recharges", sizeof(weaponinfo[0].ammoplural));

        M_StringCopy(mobjinfo[MT_MISC0].name1, "light armor vest", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC0].plural1, "light armor vests", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC1].name1, "heavy armor vest", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC1].plural1, "heavy armor vests", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC2].name1, "1% health bonus", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC2].plural1, "1% health bonuses", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC3].name1, "1% armor bonus", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC3].plural1, "1% armor bonuses", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC4].name1, "blue passcard", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC4].plural1, "blue passcards", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC5].name1, "red passcard", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC5].plural1, "red passcards", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC6].name1, "yellow passcard", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC6].plural1, "yellow passcards", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC7].name1, "yellow skeleton key", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC7].plural1, "yellow skeleton keys", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC8].name1, "red skeleton key", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC8].plural1, "red skeleton keys", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC9].name1, "blue skeleton key", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC9].plural1, "blue skeleton keys", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC10].name1, "small health pack", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC10].plural1, "small health packs", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC11].name1, "large health pack", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC11].plural1, "large health packs", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC12].name1, "overdrive sphere", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC12].plural1, "overdrive spheres", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC13].name1, "steroids", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC13].plural1, "steroids", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_INS].name1, "stealth sphere", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_INS].plural1, "stealth spheres", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC14].name1, "hazard suit", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC14].plural1, "hazard suits", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC15].name1, "tactical survey map", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC15].plural1, "tactical survey maps", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC16].name1, "night vision goggles", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC16].plural1, "night vision goggles", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MEGA].name1, "ultra-overdrive sphere", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MEGA].plural1, "ultra-overdrive spheres", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CLIP].name1, "ammo clip", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CLIP].plural1, "ammo clips", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC17].name1, "box of ammo", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC17].plural1, "boxes of ammo", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC18].name1, "missile", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC18].plural1, "missiles", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC19].name1, "crate of missiles", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC19].plural1, "crates of missiles", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC20].name1, "small polaric recharge", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC20].plural1, "small polaric recharges", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC21].name1, "large polaric recharge", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC21].plural1, "large polaric recharges", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC25].name1, "SKAG 1337", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC25].plural1, "SKAG 1337s", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CHAINGUN].name1, "minigun", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CHAINGUN].plural1, "miniguns", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC26].name1, "angle grinder", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC26].plural1, "angle grinders", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC27].name1, "missile launcher", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC27].plural1, "missile launchers", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC28].name1, "polaric energy cannon", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC28].plural1, "polaric energy cannons", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SHOTGUN].name1, "pump-action shotgun", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SHOTGUN].plural1, "pump-action shotguns", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SUPERSHOTGUN].name1, "double-barreled shotgun", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SUPERSHOTGUN].plural1, "double-barreled shotguns", sizeof(mobjinfo[0].plural1));

        M_StringCopy(mobjinfo[MT_POSSESSED].name1, "zombie", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_POSSESSED].plural1, "zombies", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SHOTGUY].name1, "shotgun zombie", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SHOTGUY].plural1, "shotgun zombies", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_VILE].name1, "necromancer", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_VILE].plural1, "necromancers", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_UNDEAD].name1, "dark soldier", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_UNDEAD].plural1, "dark soldiers", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_FATSO].name1, "combat slug", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_FATSO].plural1, "combat slugs", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CHAINGUY].name1, "minigun zombie", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CHAINGUY].plural1, "minigun zombies", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_TROOP].name1, "serpentipede", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_TROOP].plural1, "serpentipedes", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SERGEANT].name1, "flesh worm", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SERGEANT].plural1, "flesh worms", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SHADOWS].name1, "stealth worm", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SHADOWS].plural1, "stealth worms", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_HEAD].name1, "trilobite", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_HEAD].plural1, "trilobites", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_BRUISER].name1, "pain bringer", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_BRUISER].plural1, "pain bringers", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_KNIGHT].name1, "pain lord", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_KNIGHT].plural1, "pain lords", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SKULL].name1, "deadflare", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SKULL].plural1, "deadflares", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SPIDER].name1, "large technospider", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SPIDER].plural1, "large technospiders", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_BABY].name1, "technospider", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_BABY].plural1, "technospiders", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CYBORG].name1, "assault tripod", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CYBORG].plural1, "assault tripods", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_PAIN].name1, "summoner", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_PAIN].plural1, "summoners", sizeof(mobjinfo[0].plural1));
    }
    else if (hacx)
    {
        s_M_SKILLLEVEL1 = M_StringDuplicate("Please don't shoot!");
        s_M_SKILLLEVEL2 = M_StringDuplicate("Arrgh, I need health!");
        s_M_SKILLLEVEL3 = M_StringDuplicate("Let's rip them apart!");
        s_M_SKILLLEVEL4 = M_StringDuplicate("I am immortal");
        s_M_SKILLLEVEL5 = M_StringDuplicate("Insanity!");

        mobjinfo[MT_HEAD].flags2 |= MF2_DONTMAP;
        mobjinfo[MT_INV].flags2 &= ~MF2_TRANSLUCENT_33;
        mobjinfo[MT_INS].flags2 &= ~(MF2_TRANSLUCENT_33 | MF2_FLOATBOB);
        mobjinfo[MT_MISC14].flags2 &= ~MF2_FLOATBOB;
        mobjinfo[MT_BFG].flags2 &= ~MF2_TRANSLUCENT;

        mobjinfo[MT_HEAD].bloodcolor = REDBLOOD;
        mobjinfo[MT_BRUISER].bloodcolor = REDBLOOD;
        mobjinfo[MT_KNIGHT].bloodcolor = REDBLOOD;
    }
    else if (eviternity)
    {
        mobjinfo[MT_BRUISER].bloodcolor = REDBLOOD;
        mobjinfo[MT_DOGS].bloodcolor = GREENBLOOD;
    }
    else if (doom4vanilla)
    {
        mobjinfo[MT_HEAD].bloodcolor = REDBLOOD;
        mobjinfo[MT_KNIGHT].bloodcolor = REDBLOOD;

        mobjinfo[MT_INV].flags2 &= ~(MF2_TRANSLUCENT_33 | MF2_FLOATBOB);
        mobjinfo[MT_MEGA].flags2 &= ~MF2_FLOATBOB;

        M_StringCopy(mobjinfo[MT_POSSESSED].name1, "possessed", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_POSSESSED].plural1, "possessed", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SHOTGUY].name1, "possessed security", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SHOTGUY].plural1, "possessed security", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_VILE].name1, "summoner", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_VILE].plural1, "summoners", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_UNDEAD].name1, "mancubus", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_UNDEAD].plural1, "mancubi", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_FATSO].name1, "revenant", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_FATSO].plural1, "revenants", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CHAINGUY].name1, "hell razer", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CHAINGUY].plural1, "hell razers", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SERGEANT].name1, "pinky", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SERGEANT].plural1, "pinkies", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SERGEANT].name3, "demon", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SERGEANT].plural3, "demons", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SPIDER].name1, "cyberdemon", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SPIDER].plural1, "cyberdemons", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_BABY].name1, "spider mastermind", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_BABY].plural1, "spider masterminds", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CYBORG].name1, "cyber-mancubus", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CYBORG].plural1, "cyber-mancubi", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_PAIN].name1, "gore nest", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_PAIN].plural1, "gore nests", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_WOLFSS].name1, "possessed", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_WOLFSS].plural1, "possessed", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_WOLFSS].name2, "possessed scientist", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_WOLFSS].plural2, "possessed scientists", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_INV].name2, "super chainsaw", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_INV].plural2, "super chainsaws", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MEGA].name2, "mega doll", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MEGA].plural2, "mega dolls", sizeof(mobjinfo[0].plural1));
    }
    else if (REKKR)
    {
        s_M_EPISODE1 = M_StringDuplicate("Homecoming");
        s_M_EPISODE2 = M_StringDuplicate("Downfall");
        s_M_EPISODE3 = M_StringDuplicate("Otherworld");
        s_M_EPISODE4 = M_StringDuplicate(REKKRSL ? "Sunken Land" : "Bonus");

        s_M_SKILLLEVEL1 = M_StringDuplicate("Scrapper");
        s_M_SKILLLEVEL2 = M_StringDuplicate("Brawler");
        s_M_SKILLLEVEL3 = M_StringDuplicate("Fighter");
        s_M_SKILLLEVEL4 = M_StringDuplicate("Wrecker");
        s_M_SKILLLEVEL5 = M_StringDuplicate("Berserker");

        M_StringCopy(berserk, "wode", sizeof(berserk));

        mobjinfo[MT_HEAD].bloodcolor = REDBLOOD;
        mobjinfo[MT_KNIGHT].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC52].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC53].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC54].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC55].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC57].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC58].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC59].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC60].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC61].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC62].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC63].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC64].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC66].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC67].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC68].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC69].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC70].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC71].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC72].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC73].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC74].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC75].bloodcolor = NOBLOOD;
        mobjinfo[MT_MISC79].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC80].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC81].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC82].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC83].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC84].bloodcolor = REDBLOOD;
        mobjinfo[MT_MISC85].bloodcolor = REDBLOOD;

        mobjinfo[MT_SKULL].flags2 &= ~MF2_TRANSLUCENT_REDONLY;
        mobjinfo[MT_CLIP].flags2 |= MF2_TRANSLUCENT_50;

        weaponinfo[wp_pistol].name = M_StringDuplicate("soul bow");
        weaponinfo[wp_shotgun].name = M_StringDuplicate("steel-shot launcher");
        weaponinfo[wp_chaingun].name = M_StringDuplicate("soul gun");
        weaponinfo[wp_missile].name = M_StringDuplicate("runic staff");
        weaponinfo[wp_plasma].name = M_StringDuplicate("holy relic");
        weaponinfo[wp_bfg].name = M_StringDuplicate("blessing of the gods");
        weaponinfo[wp_chainsaw].name = M_StringDuplicate("axe");

        M_StringCopy(weaponinfo[wp_pistol].ammoname, "soul", sizeof(weaponinfo[0].ammoname));
        M_StringCopy(weaponinfo[wp_pistol].ammoplural, "souls", sizeof(weaponinfo[0].ammoplural));
        M_StringCopy(weaponinfo[wp_shotgun].ammoname, "steelshot", sizeof(weaponinfo[0].ammoname));
        M_StringCopy(weaponinfo[wp_shotgun].ammoplural, "steelshots", sizeof(weaponinfo[0].ammoplural));
        M_StringCopy(weaponinfo[wp_missile].ammoname, "rune", sizeof(weaponinfo[0].ammoname));
        M_StringCopy(weaponinfo[wp_missile].ammoplural, "runes", sizeof(weaponinfo[0].ammoplural));
        M_StringCopy(weaponinfo[wp_plasma].ammoname, "mana", sizeof(weaponinfo[0].ammoname));
        M_StringCopy(weaponinfo[wp_plasma].ammoplural, "mana", sizeof(weaponinfo[0].ammoplural));

        M_StringCopy(mobjinfo[MT_POSSESSED].name1, "former human", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_POSSESSED].plural1, "former humans", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SHOTGUY].name1, "jackalope", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SHOTGUY].plural1, "jackalopes", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_VILE].name1, "skeleturret", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_VILE].plural1, "skeleturrets", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_UNDEAD].name1, "mean imp", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_UNDEAD].plural1, "mean imps", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_FATSO].name1, "former duke", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_FATSO].plural1, "former dukes", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CHAINGUY].name1, "former king", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CHAINGUY].plural1, "former kings", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SERGEANT].name1, "husk", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SERGEANT].plural1, "husks", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SHADOWS].name1, "mean husk", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SHADOWS].plural1, "mean husks", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_HEAD].name1, "sorrow", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_HEAD].plural1, "sorrows", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_BRUISER].name1, "tree beast", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_BRUISER].plural1, "tree beasts", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_KNIGHT].name1, "skelly belly", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_KNIGHT].plural1, "skelly bellies", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SKULL].name1, "eyeball", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SKULL].plural1, "eyeballs", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_SPIDER].name1, "large technospider", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_SPIDER].plural1, "large technospiders", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_BABY].name1, "mean jackalope", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_BABY].plural1, "mean jackalopes", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CYBORG].name1, "death raven", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CYBORG].plural1, "death ravens", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_WOLFSS].name1, "former human grotesque", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_WOLFSS].plural1, "former human grotesques", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_KEEN].name1, "health mimic", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_KEEN].plural1, "health mimics", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_BOSSBRAIN].name1, "flammenwerfer", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_BOSSBRAIN].plural1, "flammenwerfers", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_PLASMA].name1, "barrel", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_PLASMA].plural1, "barrels", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_CHAINGUN].name1, "skeletower", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_CHAINGUN].plural1, "skeletowers", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC38].name1, "puppy", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC38].plural1, "puppies", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC65].name1, "eye spawner", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC65].plural1, "eye spawners", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC77].name1, "landmine", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC77].plural1, "landmines", sizeof(mobjinfo[0].plural1));
        M_StringCopy(mobjinfo[MT_MISC78].name1, "skelespider", sizeof(mobjinfo[0].name1));
        M_StringCopy(mobjinfo[MT_MISC78].plural1, "skelespiders", sizeof(mobjinfo[0].plural1));
    }
}

//
// R_InitColormaps
//
// killough 03/20/98: rewritten to allow dynamic colormaps
// and to remove unnecessary 256-byte alignment
//
// killough 04/04/98: Add support for C_START/C_END markers
//
static void R_InitColormaps(void)
{
    const bool  COLORMAP = (W_CheckMultipleLumps("COLORMAP") > 1);
    byte        *palsrc = PLAYPAL;
    wadfile_t   *colormapwad = lumpinfo[W_CheckNumForName("COLORMAP")]->wadfile;

    if (W_CheckNumForName("C_START") >= 0 && W_CheckNumForName("C_END") >= 0)
    {
        firstcolormaplump = W_GetNumForName("C_START");
        numcolormaps = W_GetNumForName("C_END") - firstcolormaplump;

        colormaps = Z_Malloc(numcolormaps * sizeof(*colormaps), PU_STATIC, NULL);

        for (int i = 1; i < numcolormaps; i++)
            colormaps[i] = W_CacheLumpNum(firstcolormaplump + i);
    }
    else
        colormaps = Z_Malloc(sizeof(*colormaps), PU_STATIC, NULL);

    dc_colormap[1] = dc_nextcolormap[1] = colormaps[0] = W_CacheLumpName("COLORMAP");

    if (numcolormaps == 1)
        C_Output("Using the " BOLD("COLORMAP") " lump in the %s " BOLD("%s") ".",
            (colormapwad->type == IWAD ? "IWAD" : "PWAD"), colormapwad->path);
    else
    {
        wadfile_t   *othercolormapwad = lumpinfo[firstcolormaplump]->wadfile;

        if (M_StringCompare(colormapwad->path, othercolormapwad->path))
            C_Output("Using the " BOLD("COLORMAP") " lump and %i more in the %s " BOLD("%s") ".",
                numcolormaps - 1, (colormapwad->type == IWAD ? "IWAD" : "PWAD"), colormapwad->path);
        else
            C_Output("Using the " BOLD("COLORMAP") " lump in the %s " BOLD("%s") " and %i more in the %s " BOLD("%s") ".",
                (colormapwad->type == IWAD ? "IWAD" : "PWAD"), colormapwad->path, numcolormaps - 1,
                (othercolormapwad->type == IWAD ? "IWAD" : "PWAD"), othercolormapwad->path);
    }

    for (int i = 0; i < 255; i++)
    {
        const byte  red = *palsrc++;
        const byte  green = *palsrc++;
        const byte  blue = *palsrc++;
        byte        gray = (byte)(red * 0.2126 + green * 0.7152 + blue * 0.0722);

        grays[i] = FindNearestColor(PLAYPAL, gray, gray, gray);

        if (!COLORMAP)
        {
            gray = 255 - gray;
            colormaps[0][32 * 256 + i] = FindNearestColor(PLAYPAL, gray, gray, gray);
        }
    }
}

// killough 04/04/98: get colormap number from name
// killough 04/11/98: changed to return -1 for illegal names
int R_ColormapNumForName(char *name)
{
    int i = 0;

    if (numcolormaps == 1)
        return -1;

    if (strncasecmp(name, "COLORMAP", 8))     // COLORMAP predefined to return 0
        if ((i = W_CheckNumForName(name)) >= 0)
            i -= firstcolormaplump;

    return (i > numcolormaps ? -1 : i);
}

//
// R_InitData
// Locates all the lumps that will be used by all views
// Must be called after W_Init.
//
void R_InitData(void)
{
    R_InitFlats();
    R_InitTextures();
    R_InitBrightmaps();
    R_InitSpriteLumps();
    R_InitColormaps();

    // [JN] Generate doomednum hash at startup.
    P_FindDoomedNum(0);
}

//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
int R_FlatNumForName(char *name)
{
    const int   i = W_RangeCheckNumForName(firstflat, lastflat, name);

    if (i == -1)
    {
        if (*name && *name != '-')
        {
            char    *temp = uppercase(name);

            C_Warning(1, "The " BOLD("%.8s") " texture can't be found.", temp);
            free(temp);
        }

        return missingflatnum;
    }

    return (i - firstflat);
}

//
// R_CheckFlatNumForName
// Retrieval, get a flat number for a flat name. No error.
//
int R_CheckFlatNumForName(char *name)
{
    for (int i = firstflat; i <= lastflat; i++)
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
            return (i - firstflat);

    return -1;
}

//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int R_CheckTextureNumForName(char *name)
{
    int i = 0;

    if (*name != '-')
    {
        i = textures[W_LumpNameHash(name) % numtextures]->index;

        while (i >= 0 && strncasecmp(textures[i]->name, name, 8))
            i = textures[i]->next;
    }

    return i;
}

//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int R_TextureNumForName(char *name)
{
    const int   i = R_CheckTextureNumForName(name);

    if (i == -1)
    {
        if (*name && *name != '-' && !suppresswarnings)
        {
            char    *temp = uppercase(name);

            C_Warning(1, "The " BOLD("%.8s") " texture can't be found.", temp);
            free(temp);
        }

        return 0;
    }

    return i;
}

//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
// Totally rewritten by Lee Killough to use less memory,
// to avoid using alloca(), and to improve performance.
void R_PrecacheLevel(void)
{
    bool    *hitlist = calloc(MAX(numtextures, numflats), sizeof(bool));

    // Precache flats.
    for (int i = 0; i < numsectors; i++)
    {
        hitlist[sectors[i].floorpic] = true;
        hitlist[sectors[i].ceilingpic] = true;
    }

    for (int i = 0; i < numflats; i++)
        if (hitlist[i])
            W_CacheLumpNum(firstflat + i);

    // Precache textures.
    memset(hitlist, false, sizeof(*hitlist));

    for (int i = 0; i < numsides; i++)
    {
        hitlist[sides[i].toptexture] = true;
        hitlist[sides[i].midtexture] = true;
        hitlist[sides[i].bottomtexture] = true;
    }

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependent
    //  name.
    hitlist[skytexture] = true;

    for (int i = 0; i < numtextures; i++)
        if (hitlist[i])
        {
            texture_t   *texture = textures[i];

            for (int j = 0; j < texture->patchcount; j++)
                W_CacheLumpNum(texture->patches[j].patch);
        }

    free(hitlist);
}
