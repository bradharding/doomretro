/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "i_video.h"
#include "r_defs.h"

//
// Globally visible constants.
//
#define HU_FONTSTART                '!' // the first font character
#define HU_FONTEND                  '_' // the last font character

// Calculate # of characters in font.
#define HU_FONTSIZE                 (HU_FONTEND - HU_FONTSTART + 1)

#define HU_MSGX                     (3 * 2)
#define HU_MSGY                     (2 * 2)

#define HU_MSGTIMEOUT               (4 * TICRATE)

#define HUD_NUMBER_MIN              (negativehealth ? -99 : 0)
#define HUD_NUMBER_MAX              999

#define HUD_HEALTH_X                (MAXWIDESCREENDELTA + 2)
#define HUD_HEALTH_Y                (SCREENHEIGHT - 32)
#define HUD_HEALTH_MIN              20
#define HUD_HEALTH_WAIT             250
#define HUD_HEALTH_HIGHLIGHT_WAIT   250

#define HUD_ARMOR_X                 (MAXWIDESCREENDELTA + 71)
#define HUD_ARMOR_Y                 HUD_HEALTH_Y
#define HUD_ARMOR_HIGHLIGHT_WAIT    250

#define HUD_KEYS_X                  (SCREENWIDTH - MAXWIDESCREENDELTA - 35)
#define HUD_KEYS_Y                  HUD_HEALTH_Y

#define HUD_AMMO_X                  (SCREENWIDTH - MAXWIDESCREENDELTA + 2)
#define HUD_AMMO_Y                  HUD_HEALTH_Y
#define HUD_AMMO_MIN                10
#define HUD_AMMO_WAIT               250
#define HUD_AMMO_HIGHLIGHT_WAIT     250

#define HUD_KEY_WAIT                250

#define ALTHUD_LEFT_X               (MAXWIDESCREENDELTA + 9)
#define ALTHUD_RIGHT_X              (SCREENWIDTH - MAXWIDESCREENDELTA - 128)
#define ALTHUD_Y                    (SCREENHEIGHT - 41)

#define DRAWDISKTICS                (12 * TICRATE)

//
// HEADS UP TEXT
//
void HU_Init(void);
void HU_SetTranslucency(void);
void HU_Start(void);

void HU_Ticker(void);
void HU_Drawer(void);
void HU_Erase(void);

void HU_SetPlayerMessage(char *message, bool group, bool external);
void HU_PlayerMessage(char *message, bool group, bool external);

void HU_ClearMessages(void);
void HU_DrawDisk(void);

extern patch_t  *hu_font[HU_FONTSIZE];
extern patch_t  *minuspatch;

extern uint64_t healthhighlight;
extern uint64_t ammohighlight;
extern uint64_t armorhighlight;
extern int      ammodiff[NUMAMMO];
extern int      maxammodiff[NUMAMMO];
extern int      armordiff;
extern int      healthdiff;
extern bool     drawdisk;
extern int      drawdisktics;
extern bool     idbehold;
extern int      message_counter;
extern bool     message_dontfuckwithme;
extern bool     message_secret;
extern bool     message_warning;
extern bool     message_on;
extern bool     message_fadeon;
extern char     prevmessage[133];
extern short    minuspatchtopoffset1;
extern short    minuspatchtopoffset2;
extern short    minuspatchwidth;
