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

#pragma once

typedef enum
{
    AnimCondition_None,
    AnimCondition_MapNumGreater,
    AnimCondition_MapNumEqual,
    AnimCondition_MapVisited,
    AnimCondition_MapNotSecret,
    AnimCondition_SecretVisited,
    AnimCondition_Tally,
    AnimCondition_IsEntering
} animcondition_t;

typedef enum
{
    Frame_None           = 0x0000,
    Frame_Infinite       = 0x0001,
    Frame_FixedDuration  = 0x0002,
    Frame_RandomDuration = 0x0004,
    Frame_RandomStart    = 0x1000
} frametype_t;

typedef struct
{
    animcondition_t     condition;
    int                 param;
} interlevelcond_t;

typedef struct
{
    char                *imagelump;
    frametype_t         type;
    int                 duration;
    int                 maxduration;
} interlevelframe_t;

typedef struct
{
    interlevelframe_t   *frames;
    interlevelcond_t    *conditions;
    int                 xpos;
    int                 ypos;
} interlevelanim_t;

typedef struct
{
    interlevelanim_t    *anims;
    interlevelcond_t    *conditions;
} interlevellayer_t;

typedef struct
{
    char                *musiclump;
    char                *backgroundlump;
    interlevellayer_t   *layers;
} interlevel_t;

interlevel_t *WI_ParseInterlevel(const char *lumpname);
