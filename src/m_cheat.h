/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__M_CHEAT__)
#define __M_CHEAT__

#include <stdlib.h>

//
// CHEAT SEQUENCE PACKAGE
//

// declaring a cheat
#define CHEAT(value, parameters) \
    { value, sizeof(value) - 1, parameters, 0, 0, "", 0, false }

#define MAX_CHEAT_LEN           25
#define MAX_CHEAT_PARAMS        5

#define TIMELIMIT               (TICRATE * 2)

typedef struct
{
    // settings for this cheat
    char        *sequence;
    size_t      sequence_len;
    int         parameter_chars;

    // state used during the game
    size_t      chars_read;
    int         param_chars_read;
    char        parameter_buf[MAX_CHEAT_PARAMS];

    int         timeout;

    dboolean    actionkey;
} cheatseq_t;

int cht_CheckCheat(cheatseq_t *cht, unsigned char key);

void cht_GetParam(cheatseq_t *cht, char *buffer);

extern dboolean idbehold;
extern int      leveltime;

extern int      key_right;
extern int      key_left;
extern int      key_up;
extern int      key_up2;
extern int      key_down;
extern int      key_down2;
extern int      key_strafeleft;
extern int      key_straferight;
extern int      key_fire;
extern int      key_use;
extern int      key_strafe;
extern int      key_run;
extern int      key_prevweapon;
extern int      key_nextweapon;
extern int      key_weapon1;
extern int      key_weapon2;
extern int      key_weapon3;
extern int      key_weapon4;
extern int      key_weapon5;
extern int      key_weapon6;
extern int      key_weapon7;

extern cheatseq_t cheat_mus;
extern cheatseq_t cheat_mus_xy;
extern cheatseq_t cheat_god;
extern cheatseq_t cheat_ammo;
extern cheatseq_t cheat_ammonokey;
extern cheatseq_t cheat_noclip;
extern cheatseq_t cheat_commercial_noclip;
extern cheatseq_t cheat_powerup[7];
extern cheatseq_t cheat_choppers;
extern cheatseq_t cheat_clev;
extern cheatseq_t cheat_clev_xy;
extern cheatseq_t cheat_mypos;
extern cheatseq_t cheat_amap;

#endif
