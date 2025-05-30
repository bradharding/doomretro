/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "SDL_gamecontroller.h"

#define CONTROLLER_A                    0x00000001
#define CONTROLLER_B                    0x00000002
#define CONTROLLER_X                    0x00000004
#define CONTROLLER_Y                    0x00000008
#define CONTROLLER_BACK                 0x00000010
#define CONTROLLER_GUIDE                0x00000020
#define CONTROLLER_START                0x00000040
#define CONTROLLER_LEFT_THUMB           0x00000080
#define CONTROLLER_RIGHT_THUMB          0x00000100
#define CONTROLLER_LEFT_SHOULDER        0x00000200
#define CONTROLLER_RIGHT_SHOULDER       0x00000400
#define CONTROLLER_DPAD_UP              0x00000800
#define CONTROLLER_DPAD_DOWN            0x00001000
#define CONTROLLER_DPAD_LEFT            0x00002000
#define CONTROLLER_DPAD_RIGHT           0x00004000
#define CONTROLLER_MISC1                0x00008000
#define CONTROLLER_PADDLE1              0x00010000
#define CONTROLLER_PADDLE2              0x00020000
#define CONTROLLER_PADDLE3              0x00040000
#define CONTROLLER_PADDLE4              0x00080000
#define CONTROLLER_TOUCHPAD             0x00100000
#define CONTROLLER_LEFT_TRIGGER         0x00200000
#define CONTROLLER_RIGHT_TRIGGER        0x00400000

#define CONTROLLER_TRIGGER_THRESHOLD    3855

#define IDLE_CHAINSAW_RUMBLE_STRENGTH   10000
#define PICKUP_RUMBLE_STRENGTH          15000
#define PICKUP_RUMBLE_TICS              10
#define OOF_RUMBLE_STRENGTH             10000
#define OOF_RUMBLE_TICS                 15

extern int      barrelrumbletics;
extern int      damagerumbletics;
extern int      pickuprumbletics;
extern int      weaponrumbletics;

extern int      controllerbuttons;
extern short    controllerthumbLX;
extern short    controllerthumbLY;
extern short    controllerthumbRX;
extern short    controllerthumbRY;
extern int      idlechainsawrumblestrength;
extern int      restoredrumblestrength;
extern float    controllerhorizontalsensitivity;
extern float    controllerverticalsensitivity;
extern short    controllerleftdeadzone;
extern short    controllerrightdeadzone;

extern char     *selectbutton;

void I_InitController(void);
void I_ShutdownController(void);
void I_ControllerRumble(const short low, const short high);
void I_ReadController(void);
void I_StopControllerRumble(void);
void I_SetControllerHorizontalSensitivity(void);
void I_SetControllerVerticalSensitivity(void);
void I_SetControllerLeftDeadZone(void);
void I_SetControllerRightDeadZone(void);
