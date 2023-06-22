/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "c_console.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_misc.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

typedef enum
{
    SECTION_NORMAL,
    SECTION_FLATS,
    SECTION_SPRITES,
    SECTION_HIDEF
} section_t;

typedef struct
{
    lumpinfo_t          **lumps;
    int                 numlumps;
} searchlist_t;

typedef struct
{
    char                sprname[4];
    char                frame;
    lumpinfo_t          *angle_lumps[8];
} sprite_frame_t;

static searchlist_t     iwad;
static searchlist_t     iwad_sprites;
static searchlist_t     pwad;

static searchlist_t     iwad_flats;
static searchlist_t     pwad_sprites;
static searchlist_t     pwad_flats;

// lumps with these sprites must be replaced in the IWAD
static sprite_frame_t   *sprite_frames;
static int              num_sprite_frames;
static int              sprite_frames_alloced = 128;

// Search in a list to find a lump with a particular name
// Linear search (slow!)
//
// Returns -1 if not found
static int FindInList(searchlist_t *list, char *name)
{
    for (int i = 0; i < list->numlumps; i++)
        if (!strncasecmp(list->lumps[i]->name, name, 8))
            return i;

    return -1;
}

static bool SetupList(searchlist_t *list, searchlist_t *src_list,
    char *startname, char *endname, char *startname2, char *endname2)
{
    int startlump = FindInList(src_list, startname);

    list->numlumps = 0;

    if (startname2 && startlump < 0)
        startlump = FindInList(src_list, startname2);

    if (startlump >= 0)
    {
        int endlump = FindInList(src_list, endname);

        if (endname2 && endlump < 0)
            endlump = FindInList(src_list, endname2);

        if (endlump > startlump)
        {
            list->lumps = src_list->lumps + startlump + 1;
            list->numlumps = endlump - startlump - 1;
            return true;
        }
    }

    return false;
}

// Sets up the sprite/flat search lists
static void SetupLists(void)
{
    // IWAD
    if (!SetupList(&iwad_flats, &iwad, "F_START", "F_END", NULL, NULL))
        I_Error("Flats section not found in IWAD");

    if (!SetupList(&iwad_sprites, &iwad, "S_START", "S_END", NULL, NULL))
        I_Error("Sprites section not found in IWAD");

    // PWAD
    SetupList(&pwad_flats, &pwad, "F_START", "F_END", "FF_START", "FF_END");
    SetupList(&pwad_sprites, &pwad, "S_START", "S_END", "SS_START", "SS_END");
}

// Initialize the replace list
static void InitSpriteList(void)
{
    sprite_frames = Z_Malloc(sprite_frames_alloced * sizeof(*sprite_frames), PU_STATIC, NULL);
    num_sprite_frames = 0;
}

static bool ValidSpriteLumpName(const char *name)
{
    if (name[0] == '\0' || name[1] == '\0' || name[2] == '\0' || name[3] == '\0')
        return false;

    // First frame:
    if (name[4] == '\0' || !isdigit((int)name[5]))
        return false;

    // Second frame (optional):
    if (name[6] != '\0' && !isdigit((int)name[7]))
        return false;

    return true;
}

// Find a sprite frame
static sprite_frame_t *FindSpriteFrame(char *name, char frame)
{
    sprite_frame_t  *result;

    // Search the list and try to find the frame
    for (int i = 0; i < num_sprite_frames; i++)
    {
        sprite_frame_t  *cur = &sprite_frames[i];

        if (!strncasecmp(cur->sprname, name, 4) && cur->frame == frame)
            return cur;
    }

    // Not found in list; Need to add to the list

    // Grow list?
    if (num_sprite_frames >= sprite_frames_alloced)
    {
        sprite_frame_t  *newframes = Z_Malloc((size_t)sprite_frames_alloced * 2 * sizeof(*sprite_frames), PU_STATIC, NULL);

        memcpy(newframes, sprite_frames, sprite_frames_alloced * sizeof(*sprite_frames));
        Z_Free(sprite_frames);
        sprite_frames_alloced *= 2;
        sprite_frames = newframes;
    }

    // Add to end of list
    result = &sprite_frames[num_sprite_frames];
    strncpy(result->sprname, name, 4);
    result->frame = frame;

    for (int i = 0; i < 8; i++)
        result->angle_lumps[i] = NULL;

    num_sprite_frames++;

    return result;
}

// Check if sprite lump is needed in the new WAD
static bool SpriteLumpNeeded(lumpinfo_t *lump)
{
    sprite_frame_t  *sprite;
    int             angle_num;

    if (!ValidSpriteLumpName(lump->name))
        return true;

    // check the first frame
    sprite = FindSpriteFrame(lump->name, lump->name[4]);

    if (!(angle_num = lump->name[5] - '0'))
    {
        // must check all frames
        for (int i = 0; i < 8; i++)
            if (sprite->angle_lumps[i] == lump)
                return true;
    }
    else
    {
        // check if this lump is being used for this frame
        if (sprite->angle_lumps[angle_num - 1] == lump)
            return true;
    }

    // second frame if any

    // no second frame?
    if (lump->name[6] == '\0')
        return false;

    sprite = FindSpriteFrame(lump->name, lump->name[6]);

    if (!(angle_num = lump->name[7] - '0'))
    {
        // must check all frames
        for (int i = 0; i < 8; i++)
            if (sprite->angle_lumps[i] == lump)
                return true;
    }
    else
    {
        // check if this lump is being used for this frame
        if (sprite->angle_lumps[angle_num - 1] == lump)
            return true;
    }

    return false;
}

static void AddSpriteLump(lumpinfo_t *lump)
{
    static struct
    {
        const char  *spr1;
        const char  *spr2;
    } weaponsprites[] = {
        { "PUNG", ""     }, { "PISG", "PISF" }, { "SHTG", "SHTF" }, { "CHGG", "CHGF" }, { "MISG", "MISF" },
        { "PLSG", "PLSF" }, { "BFGG", "BFGF" }, { "SAWG", ""     }, { "SHT2", "SHT2" }, { "",     ""     }
    };

    sprite_frame_t  *sprite;
    int             angle_num;
    static int      MISFA0;
    static int      MISFB0;
    static int      SHT2A0;
    static int      SHT2E0;
    bool            isresourcewad = M_StringCompare(leafname(lump->wadfile->path), DOOMRETRO_RESOURCEWAD);

    if (!ValidSpriteLumpName(lump->name))
        return;

    if (lump->wadfile->type == PWAD)
    {
        if (!isresourcewad)
        {
            int i = 0;

            MISFA0 += M_StringCompare(lump->name, "MISFA0");
            MISFB0 += M_StringCompare(lump->name, "MISFB0");
            SHT2A0 += M_StringCompare(lump->name, "SHT2A0");
            SHT2E0 += M_StringCompare(lump->name, "SHT2E0");

            while (*weaponsprites[i].spr1)
            {
                if (M_StringStartsWith(lump->name, weaponsprites[i].spr1)
                    || (*weaponsprites[i].spr2 && M_StringStartsWith(lump->name, weaponsprites[i].spr2)))
                    weaponinfo[i].altered = true;

                i++;
            }
        }

        if (M_StringCompare(lump->name, "BAR1A0") || M_StringCompare(lump->name, "BAR1B0"))
        {
            states[S_BAR1].nextstate = S_BAR2;
            mobjinfo[MT_BARREL].frames = 2;
        }
    }

    if (isresourcewad)
    {
        if (M_StringStartsWith(lump->name, "MISF") && ((MISFA0 >= 2 || MISFB0 >= 2) || hacx || FREEDOOM))
            return;

        if (M_StringCompare(lump->name, "SHT2A0") && (SHT2A0 >= 2 || hacx || FREEDOOM))
            return;

        if (M_StringCompare(lump->name, "SHT2E0") && (SHT2E0 >= 2 || hacx || FREEDOOM))
            return;

        if (chex && (M_StringCompare(lump->name, "MEDIA0") || M_StringCompare(lump->name, "STIMA0")))
            return;
    }

    // first angle
    sprite = FindSpriteFrame(lump->name, lump->name[4]);

    if (!(angle_num = lump->name[5] - '0'))
        for (int i = 0; i < 8; i++)
            sprite->angle_lumps[i] = lump;
    else
        sprite->angle_lumps[angle_num - 1] = lump;

    // no second angle?
    if (lump->name[6] == '\0')
        return;

    // second angle
    sprite = FindSpriteFrame(lump->name, lump->name[6]);

    if (!(angle_num = lump->name[7] - '0'))
        for (int i = 0; i < 8; i++)
            sprite->angle_lumps[i] = lump;
    else
        sprite->angle_lumps[angle_num - 1] = lump;
}

// Generate the list. Run at the start, before merging
static void GenerateSpriteList(void)
{
    InitSpriteList();

    // Add all sprites from the IWAD
    for (int i = 0; i < iwad_sprites.numlumps; i++)
        AddSpriteLump(iwad_sprites.lumps[i]);

    // Add all sprites from the PWAD
    // (replaces IWAD sprites)
    for (int i = 0; i < pwad_sprites.numlumps; i++)
    {
        AddSpriteLump(pwad_sprites.lumps[i]);

        if (M_StringCompare(pwad_sprites.lumps[i]->name, "PUFFA0"))
            PUFFA0 = true;
    }
}

// Perform the merge.
//
// The merge code creates a new lumpinfo list, adding entries from the
// IWAD first followed by the PWAD.
//
// For the IWAD:
//  * Flats are added. If a flat with the same name is in the PWAD,
//    it is ignored (deleted). At the end of the section, all flats in the
//    PWAD are inserted. This is consistent with the behavior of
//    deutex/deusf.
//  * Sprites are added. The "replace list" is generated before the merge
//    from the list of sprites in the PWAD. Any sprites in the IWAD found
//    to match the replace list are removed. At the end of the section,
//    the sprites from the PWAD are inserted.
//
// For the PWAD:
//  * All sprites and flats are ignored, with the assumption they have
//    already been merged into the IWAD's sections.
static void DoMerge(void)
{
    int         num_newlumps = 0;
    lumpinfo_t  **newlumps = calloc(numlumps, sizeof(lumpinfo_t *));
    section_t   current_section = SECTION_NORMAL;

    // Add IWAD lumps
    for (int i = 0; i < iwad.numlumps; i++)
    {
        lumpinfo_t  *lump = iwad.lumps[i];

        switch (current_section)
        {
            case SECTION_NORMAL:
                if (!strncasecmp(lump->name, "F_START", 8))
                    current_section = SECTION_FLATS;
                else if (!strncasecmp(lump->name, "S_START", 8))
                    current_section = SECTION_SPRITES;

                newlumps[num_newlumps++] = lump;
                break;

            case SECTION_FLATS:
                // Have we reached the end of the section?
                if (!strncasecmp(lump->name, "F_END", 8))
                {
                    // Add all new flats from the PWAD to the end of the section
                    for (int n = 0; n < pwad_flats.numlumps; n++)
                        newlumps[num_newlumps++] = pwad_flats.lumps[n];

                    newlumps[num_newlumps++] = lump;

                    // Back to normal reading
                    current_section = SECTION_NORMAL;
                }
                else
                {
                    // If there is a flat in the PWAD with the same name,
                    // do not add it now. All PWAD flats are added to the
                    // end of the section. Otherwise, if it is only in the
                    // IWAD, add it now
                    if (FindInList(&pwad_flats, lump->name) < 0)
                        newlumps[num_newlumps++] = lump;
                }

                break;

            case SECTION_SPRITES:
                // Have we reached the end of the section?
                if (!strncasecmp(lump->name, "S_END", 8))
                {
                    // add all the PWAD sprites
                    for (int n = 0; n < pwad_sprites.numlumps; n++)
                        if (SpriteLumpNeeded(pwad_sprites.lumps[n]))
                            newlumps[num_newlumps++] = pwad_sprites.lumps[n];

                    // Copy the ending
                    newlumps[num_newlumps++] = lump;

                    // Back to normal reading
                    current_section = SECTION_NORMAL;
                }
                else
                {
                    // Is this lump holding a sprite to be replaced in the
                    // PWAD? If so, wait until the end to add it.
                    if (SpriteLumpNeeded(lump))
                        newlumps[num_newlumps++] = lump;
                }

                break;

            case SECTION_HIDEF:
                break;
        }
    }

    // Add PWAD lumps
    current_section = SECTION_NORMAL;

    for (int i = 0, histart = -1; i < pwad.numlumps; i++)
    {
        lumpinfo_t  *lump = pwad.lumps[i];

        switch (current_section)
        {
            case SECTION_NORMAL:
                if (!strncasecmp(lump->name, "F_START", 8) || !strncasecmp(lump->name, "FF_START", 8))
                    current_section = SECTION_FLATS;
                else if (!strncasecmp(lump->name, "S_START", 8) || !strncasecmp(lump->name, "SS_START", 8))
                    current_section = SECTION_SPRITES;
                else if (!strncasecmp(lump->name, "HI_START", 8))
                {
                    current_section = SECTION_HIDEF;
                    histart = i;
                }
                else
                    // Don't include the headers of sections
                    newlumps[num_newlumps++] = lump;

                break;

            case SECTION_FLATS:
                // PWAD flats are ignored (already merged)
                if (!strncasecmp(lump->name, "FF_END", 8) || !strncasecmp(lump->name, "F_END", 8))
                    current_section = SECTION_NORMAL;   // end of section

                break;

            case SECTION_SPRITES:
                // PWAD sprites are ignored (already merged)
                if (!strncasecmp(lump->name, "SS_END", 8) || !strncasecmp(lump->name, "S_END", 8))
                    current_section = SECTION_NORMAL;   // end of section

                break;

            case SECTION_HIDEF:
                if (!strncasecmp(lump->name, "HI_END", 8) && histart != -1)
                {
                    const int   patches = i - histart - 1;

                    if (patches)
                    {
                        char    *temp = commify(patches);

                        C_Warning(1, "The %s%s between the " BOLD("HI_START") " and " BOLD("HI_END") " markers will be ignored.",
                            (patches > 1 ? temp : ""), (patches > 1 ? " patches" : "patch"));
                        free(temp);
                    }

                    current_section = SECTION_NORMAL;   // end of section
                }

                break;
        }
    }

    // Switch to the new lumpinfo, and free the old one
    free(lumpinfo);
    lumpinfo = newlumps;
    numlumps = num_newlumps;
}

// Merge in a file by name
bool W_MergeFile(char *filename, bool autoloaded)
{
    const int   old_numlumps = numlumps;

    // Load PWAD
    if (!W_AddFile(filename, autoloaded))
        return false;

    // IWAD is at the start, PWAD was appended to the end
    iwad.lumps = lumpinfo;
    iwad.numlumps = old_numlumps;

    pwad.lumps = lumpinfo + old_numlumps;
    pwad.numlumps = numlumps - old_numlumps;

    // Setup sprite/flat lists
    SetupLists();

    // Generate list of sprites to be replaced by the PWAD
    GenerateSpriteList();

    // Perform the merge
    DoMerge();

    return true;
}
