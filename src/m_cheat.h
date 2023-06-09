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

#pragma once

//
// CHEAT SEQUENCE PACKAGE
//

// declaring a cheat
#define CHEAT(value, parameters, longtimeout)   { value, parameters, longtimeout, 0, 0, "", 0 }

typedef struct
{
    // settings for this cheat
    char    *sequence;
    int     parameter_chars;
    bool    longtimeout;

    // state used during the game
    size_t  chars_read;
    int     param_chars_read;
    char    parameter_buf[5];

    int     timeout;

    bool    movekey;
} cheatseq_t;

bool cht_CheckCheat(cheatseq_t *cht, unsigned char key);

void cht_GetParam(cheatseq_t *cht, char *buffer);

extern char         cheatkey;

extern cheatseq_t   cheat_mus;
extern cheatseq_t   cheat_mus_xy;
extern cheatseq_t   cheat_god;
extern cheatseq_t   cheat_ammo;
extern cheatseq_t   cheat_ammonokey;
extern cheatseq_t   cheat_noclip;
extern cheatseq_t   cheat_commercial_noclip;
extern cheatseq_t   cheat_powerup[7];
extern cheatseq_t   cheat_choppers;
extern cheatseq_t   cheat_buddha;
extern cheatseq_t   cheat_clev;
extern cheatseq_t   cheat_clev_xy;
extern cheatseq_t   cheat_mypos;
extern cheatseq_t   cheat_amap;
