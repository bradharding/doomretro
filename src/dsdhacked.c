/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

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

========================================================================
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "dsdhacked.h"
#include "p_local.h"

//
// States
//
state_t     *states;
int         num_states;
byte        *defined_codeptr_args;
actionf_t   *deh_codeptr;

static void InitStates(void)
{
    num_states = NUMSTATES;
    states = original_states;
    deh_codeptr = malloc(num_states * sizeof(*deh_codeptr));

    for (int i = 0; i < num_states; i++)
        deh_codeptr[i] = states[i].action;

    defined_codeptr_args = calloc(num_states, sizeof(*defined_codeptr_args));
}

static void FreeStates(void)
{
    free(defined_codeptr_args);
    free(deh_codeptr);
}

void dsdh_EnsureStatesCapacity(int limit)
{
    static bool first_allocation = true;

    while (limit >= num_states)
    {
        int old_num_states = num_states;

        num_states *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            states = malloc(num_states * sizeof(*states));
            memcpy(states, original_states, old_num_states * sizeof(*states));
        }
        else
            states = realloc(states, num_states * sizeof(*states));

        memset(states + old_num_states, 0, (num_states - old_num_states) * sizeof(*states));

        deh_codeptr = realloc(deh_codeptr, num_states * sizeof(*deh_codeptr));
        memset(deh_codeptr + old_num_states, 0, (num_states - old_num_states) * sizeof(*deh_codeptr));

        defined_codeptr_args = realloc(defined_codeptr_args, num_states * sizeof(*defined_codeptr_args));
        memset(defined_codeptr_args + old_num_states, 0,
            (num_states - old_num_states) * sizeof(*defined_codeptr_args));

        for (int i = old_num_states; i < num_states; i++)
        {
            states[i].sprite = SPR_TNT1;
            states[i].tics = -1;
            states[i].nextstate = i;
        }
    }
}

//
// Sprites
//
char        **sprnames;
int         num_sprites;
static char **deh_spritenames;
static int  deh_spritenames_size;
static byte *sprnames_state;

static void InitSprites(void)
{
    sprnames = original_sprnames;
    num_sprites = NUMSPRITES;
    deh_spritenames_size = num_sprites + 1;
    deh_spritenames = malloc(deh_spritenames_size * sizeof(*deh_spritenames));

    for (int i = 0; i < num_sprites; i++)
        deh_spritenames[i] = strdup(sprnames[i]);

    deh_spritenames[num_sprites] = NULL;
    sprnames_state = calloc(num_sprites, sizeof(*sprnames_state));
}

static void EnsureSpritesCapacity(int limit)
{
  static bool   first_allocation = true;

    while (limit >= num_sprites)
    {
        int old_num_sprites = num_sprites;

        num_sprites *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            sprnames = malloc(num_sprites * sizeof(*sprnames));
            memcpy(sprnames, original_sprnames, old_num_sprites * sizeof(*sprnames));
        }
        else
            sprnames = realloc(sprnames, num_sprites * sizeof(*sprnames));

        memset(sprnames + old_num_sprites, 0, (num_sprites - old_num_sprites) * sizeof(*sprnames));

        sprnames_state = realloc(sprnames_state, num_sprites * sizeof(*sprnames_state));
        memset(sprnames_state + old_num_sprites, 0,
            (num_sprites - old_num_sprites) * sizeof(*sprnames_state));
    }
}

static void FreeSprites(void)
{
    for (int i = 0; i < deh_spritenames_size; i++)
        if (deh_spritenames[i])
            free(deh_spritenames[i]);

    free(deh_spritenames);
    free(sprnames_state);
}

int dsdh_GetDehSpriteIndex(const char* key)
{
    for (int i = 0; i < num_sprites; i++)
        if (sprnames[i] && !strncasecmp(sprnames[i], key, 4) && !sprnames_state[i])
        {
            sprnames_state[i] = true;   // sprite has been edited
            return i;
        }

    return -1;
}

int dsdh_GetOriginalSpriteIndex(const char* key)
{
    int limit;

    for (int i = 0; deh_spritenames[i]; i++)
        if (!strncasecmp(deh_spritenames[i], key, 4))
            return i;

    // is it a number?
    for (const char *c = key; *c; c++)
        if (!isdigit(*c))
            return -1;

    limit = atoi(key);
    EnsureSpritesCapacity(limit);

    return limit;
}

//
// SFX
//
sfxinfo_t       *s_sfx;
int             num_sfx;
static char     **deh_soundnames;
static int      deh_soundnames_size;
static byte     *sfx_state;

static void InitSFX(void)
{
    s_sfx = original_s_sfx;
    num_sfx = NUMSFX;
    deh_soundnames_size = num_sfx + 1;
    deh_soundnames = malloc(deh_soundnames_size * sizeof(*deh_soundnames));

    for (int i = 1; i < num_sfx; i++)
        if (s_sfx[i].name1)
            deh_soundnames[i] = strdup(s_sfx[i].name1);
        else
            deh_soundnames[i] = NULL;

    deh_soundnames[0] = NULL;
    deh_soundnames[num_sfx] = NULL;
    sfx_state = calloc(num_sfx, sizeof(*sfx_state));
}

static void FreeSFX(void)
{
    for (int i = 1; i < deh_soundnames_size; i++)
        if (deh_soundnames[i])
            free(deh_soundnames[i]);

    free(deh_soundnames);
    free(sfx_state);
}

void dsdh_EnsureSFXCapacity(int limit)
{
    static int  first_allocation = true;

    while (limit >= num_sfx)
    {
        int old_num_sfx = num_sfx;

        num_sfx *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            s_sfx = malloc(num_sfx * sizeof(*s_sfx));
            memcpy(s_sfx, original_s_sfx, old_num_sfx * sizeof(*s_sfx));
        }
        else
            s_sfx = realloc(s_sfx, num_sfx * sizeof(*s_sfx));

        memset(s_sfx + old_num_sfx, 0, (num_sfx - old_num_sfx) * sizeof(*s_sfx));

        sfx_state = realloc(sfx_state, num_sfx * sizeof(*sfx_state));
        memset(sfx_state + old_num_sfx, 0,
            (num_sfx - old_num_sfx) * sizeof(*sfx_state));

        for (int i = old_num_sfx; i < num_sfx; i++)
        {
            s_sfx[i].priority = 127;
            s_sfx[i].lumpnum = -1;
        }
    }
}

int dsdh_GetDehSFXIndex(const char* key, size_t length)
{
    for (int i = 1; i < num_sfx; i++)
        if (s_sfx[i].name1
            && strlen(s_sfx[i].name1) == length
            && !strncasecmp(s_sfx[i].name1, key, length)
            && !sfx_state[i])
        {
            sfx_state[i] = true;    // sfx has been edited
            return i;
        }

    return -1;
}

int dsdh_GetOriginalSFXIndex(const char* key)
{
    int limit;

    for (int i = 1; deh_soundnames[i]; i++)
        if (!strncasecmp(deh_soundnames[i], key, 6))
            return i;

    // is it a number?
    for (const char *c = key; *c; c++)
        if (!isdigit(*c))
            return -1;

    limit = atoi(key);
    dsdh_EnsureSFXCapacity(limit);

    return limit;
}

//
//  Things
//
mobjinfo_t  *mobjinfo;
int         num_mobj_types;

static void InitMobjInfo(void)
{
  num_mobj_types = NUMMOBJTYPES;
  mobjinfo = original_mobjinfo;
}

void dsdh_EnsureMobjInfoCapacity(int limit)
{
    static bool first_allocation = true;

    while (limit >= num_mobj_types)
    {
        int old_num_mobj_types = num_mobj_types;

        num_mobj_types *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            mobjinfo = malloc(num_mobj_types * sizeof(*mobjinfo));
            memcpy(mobjinfo, original_mobjinfo, old_num_mobj_types * sizeof(*mobjinfo));
        }
        else
            mobjinfo = realloc(mobjinfo, num_mobj_types * sizeof(*mobjinfo));

        memset(mobjinfo + old_num_mobj_types, 0,
            (num_mobj_types - old_num_mobj_types) * sizeof(*mobjinfo));

        for (int i = old_num_mobj_types; i < num_mobj_types; i++)
        {
            mobjinfo[i].droppeditem = MT_NULL;
            mobjinfo[i].infightinggroup = IG_DEFAULT;
            mobjinfo[i].projectilegroup = PG_DEFAULT;
            mobjinfo[i].splashgroup = SG_DEFAULT;
            mobjinfo[i].altspeed = NO_ALTSPEED;
            mobjinfo[i].meleerange = MELEERANGE;
        }
    }
}

void dsdh_InitTables(void)
{
    InitStates();
    InitSprites();
    InitSFX();
    InitMobjInfo();
}

void dsdh_FreeTables(void)
{
    FreeStates();
    FreeSprites();
    FreeSFX();
}
