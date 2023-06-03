﻿/*
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
#include "i_system.h"
#include "p_local.h"

//
// States
//
state_t     *states;
int         numstates;
byte        *defined_codeptr_args;
actionf_t   *deh_codeptr;

static void InitStates(void)
{
    numstates = NUMSTATES;
    states = original_states;
    deh_codeptr = malloc(numstates * sizeof(*deh_codeptr));

    for (int i = 0; i < numstates; i++)
        deh_codeptr[i] = states[i].action;

    defined_codeptr_args = calloc(numstates, sizeof(*defined_codeptr_args));

    // MBF21
    for (int i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
        states[i].flags |= STATEF_SKILL5FAST;
}

static void FreeStates(void)
{
    free(defined_codeptr_args);
    free(deh_codeptr);
}

void dsdh_EnsureStatesCapacity(int limit)
{
    static bool first_allocation = true;

    while (limit >= numstates)
    {
        const int   old_numstates = numstates;

        numstates *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            states = malloc(numstates * sizeof(*states));
            memcpy(states, original_states, old_numstates * sizeof(*states));
        }
        else
            states = I_Realloc(states, numstates * sizeof(*states));

        memset(states + old_numstates, 0, (numstates - old_numstates) * sizeof(*states));

        deh_codeptr = I_Realloc(deh_codeptr, numstates * sizeof(*deh_codeptr));
        memset(deh_codeptr + old_numstates, 0, (numstates - old_numstates) * sizeof(*deh_codeptr));

        defined_codeptr_args = I_Realloc(defined_codeptr_args, numstates * sizeof(*defined_codeptr_args));
        memset(defined_codeptr_args + old_numstates, 0,
            (numstates - old_numstates) * sizeof(*defined_codeptr_args));

        for (int i = old_numstates; i < numstates; i++)
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
int         numsprites;
static char **deh_spritenames;
static int  deh_spritenames_size;
static byte *sprnames_state;

static void InitSprites(void)
{
    sprnames = original_sprnames;
    numsprites = NUMSPRITES;
    deh_spritenames_size = numsprites + 1;
    deh_spritenames = malloc(deh_spritenames_size * sizeof(*deh_spritenames));

    for (int i = 0; i < numsprites; i++)
        deh_spritenames[i] = strdup(sprnames[i]);

    deh_spritenames[numsprites] = NULL;
    sprnames_state = calloc(numsprites, sizeof(*sprnames_state));
}

static void EnsureSpritesCapacity(int limit)
{
    static bool first_allocation = true;

    while (limit >= numsprites)
    {
        const int   old_numsprites = numsprites;

        numsprites *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            sprnames = malloc(numsprites * sizeof(*sprnames));
            memcpy(sprnames, original_sprnames, old_numsprites * sizeof(*sprnames));
        }
        else
            sprnames = I_Realloc(sprnames, numsprites * sizeof(*sprnames));

        memset(sprnames + old_numsprites, 0, (numsprites - old_numsprites) * sizeof(*sprnames));

        sprnames_state = I_Realloc(sprnames_state, numsprites * sizeof(*sprnames_state));
        memset(sprnames_state + old_numsprites, 0,
            (numsprites - old_numsprites) * sizeof(*sprnames_state));
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

int dsdh_GetDehSpriteIndex(const char *key)
{
    for (int i = 0; i < numsprites; i++)
        if (sprnames[i] && !strncasecmp(sprnames[i], key, 4) && !sprnames_state[i])
        {
            sprnames_state[i] = true;   // sprite has been edited
            return i;
        }

    return -1;
}

int dsdh_GetOriginalSpriteIndex(const char *key)
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
sfxinfo_t   *s_sfx;
int         numsfx;
static char **deh_soundnames;
static int  deh_soundnames_size;
static byte *sfx_state;

static void InitSFX(void)
{
    s_sfx = original_s_sfx;
    numsfx = NUMSFX;
    deh_soundnames_size = numsfx + 1;
    deh_soundnames = malloc(deh_soundnames_size * sizeof(*deh_soundnames));

    for (int i = 1; i < numsfx; i++)
        if (s_sfx[i].name1)
            deh_soundnames[i] = strdup(s_sfx[i].name1);
        else
            deh_soundnames[i] = NULL;

    deh_soundnames[0] = NULL;
    deh_soundnames[numsfx] = NULL;
    sfx_state = calloc(numsfx, sizeof(*sfx_state));
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

    while (limit >= numsfx)
    {
        const int   old_numsfx = numsfx;

        numsfx *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            s_sfx = malloc(numsfx * sizeof(*s_sfx));
            memcpy(s_sfx, original_s_sfx, old_numsfx * sizeof(*s_sfx));
        }
        else
            s_sfx = I_Realloc(s_sfx, numsfx * sizeof(*s_sfx));

        memset(s_sfx + old_numsfx, 0, (numsfx - old_numsfx) * sizeof(*s_sfx));

        sfx_state = I_Realloc(sfx_state, numsfx * sizeof(*sfx_state));
        memset(sfx_state + old_numsfx, 0,
            (numsfx - old_numsfx) * sizeof(*sfx_state));

        for (int i = old_numsfx; i < numsfx; i++)
        {
            s_sfx[i].priority = 127;
            s_sfx[i].lumpnum = -1;
        }
    }
}

int dsdh_GetDehSFXIndex(const char *key, size_t length)
{
    for (int i = 1; i < numsfx; i++)
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

int dsdh_GetOriginalSFXIndex(const char *key)
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
// Things
//
mobjinfo_t  *mobjinfo;
int         nummobjtypes;

static void InitMobjInfo(void)
{
  nummobjtypes = NUMMOBJTYPES;
  mobjinfo = original_mobjinfo;
}

void dsdh_EnsureMobjInfoCapacity(int limit)
{
    static bool first_allocation = true;

    while (limit >= nummobjtypes)
    {
        const int   old_nummobjtypes = nummobjtypes;

        nummobjtypes *= 2;

        if (first_allocation)
        {
            first_allocation = false;
            mobjinfo = malloc(nummobjtypes * sizeof(*mobjinfo));
            memcpy(mobjinfo, original_mobjinfo, old_nummobjtypes * sizeof(*mobjinfo));
        }
        else
            mobjinfo = I_Realloc(mobjinfo, nummobjtypes * sizeof(*mobjinfo));

        memset(mobjinfo + old_nummobjtypes, 0,
            (nummobjtypes - old_nummobjtypes) * sizeof(*mobjinfo));

        for (int i = old_nummobjtypes; i < nummobjtypes; i++)
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