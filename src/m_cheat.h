/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
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

#ifndef __M_CHEAT__
#define __M_CHEAT__

#include <stdlib.h>

//
// CHEAT SEQUENCE PACKAGE
//

// declaring a cheat
#define CHEAT(value, parameters) \
    { value, sizeof(value) - 1, parameters, 0, 0, "", 0 }

#define MAX_CHEAT_LEN           25
#define MAX_CHEAT_PARAMS        5

#define TIMELIMIT               (TICRATE * 2)

typedef struct
{
    // settings for this cheat
    char        sequence[MAX_CHEAT_LEN];
    size_t      sequence_len;
    int         parameter_chars;

    // state used during the game
    size_t      chars_read;
    int         param_chars_read;
    char        parameter_buf[MAX_CHEAT_PARAMS];

    int         timeout;
} cheatseq_t;

int cht_CheckCheat(cheatseq_t *cht, char key);

void cht_GetParam(cheatseq_t *cht, char *buffer);

extern boolean idbehold;
extern int leveltime;

#endif
