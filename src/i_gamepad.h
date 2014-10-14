/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __I_GAMEPAD__
#define __I_GAMEPAD__

#include <math.h>

#define GAMEPAD_DPAD_UP                 0x0001
#define GAMEPAD_DPAD_DOWN               0x0002
#define GAMEPAD_DPAD_LEFT               0x0004
#define GAMEPAD_DPAD_RIGHT              0x0008
#define GAMEPAD_START                   0x0010
#define GAMEPAD_BACK                    0x0020
#define GAMEPAD_LEFT_THUMB              0x0040
#define GAMEPAD_RIGHT_THUMB             0x0080
#define GAMEPAD_LEFT_SHOULDER           0x0100
#define GAMEPAD_RIGHT_SHOULDER          0x0200
#define GAMEPAD_LEFT_TRIGGER            0x0400
#define GAMEPAD_RIGHT_TRIGGER           0x0800
#define GAMEPAD_A                       0x1000
#define GAMEPAD_B                       0x2000
#define GAMEPAD_X                       0x4000
#define GAMEPAD_Y                       0x8000

#ifdef WIN32
#define GAMEPAD_LEFT_THUMB_DEADZONE     XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
#define GAMEPAD_RIGHT_THUMB_DEADZONE    XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
#define GAMEPAD_TRIGGER_THRESHOLD       XINPUT_GAMEPAD_TRIGGER_THRESHOLD
#else
#define GAMEPAD_LEFT_THUMB_DEADZONE     0
#define GAMEPAD_RIGHT_THUMB_DEADZONE    0
#define GAMEPAD_TRIGGER_THRESHOLD       0
#endif

#define gamepadthumbLXleft              (float)(-gamepadthumbLX - GAMEPAD_LEFT_THUMB_DEADZONE) /\
                                        (32767.0f - GAMEPAD_LEFT_THUMB_DEADZONE)
#define gamepadthumbLXright             (float)(gamepadthumbLX - GAMEPAD_LEFT_THUMB_DEADZONE) /\
                                        (32767.0f - GAMEPAD_LEFT_THUMB_DEADZONE)
#define gamepadthumbLYup                (float)(-gamepadthumbLY - GAMEPAD_LEFT_THUMB_DEADZONE) /\
                                        (32767.0f - GAMEPAD_LEFT_THUMB_DEADZONE)
#define gamepadthumbLYdown              (float)(gamepadthumbLY - GAMEPAD_LEFT_THUMB_DEADZONE) /\
                                        (32767.0f - GAMEPAD_LEFT_THUMB_DEADZONE)
#define gamepadthumbRXleft              pow((-gamepadthumbRX - GAMEPAD_RIGHT_THUMB_DEADZONE) /\
                                        (32767.0f - GAMEPAD_RIGHT_THUMB_DEADZONE), 3.0f)
#define gamepadthumbRXright             pow((gamepadthumbRX - GAMEPAD_RIGHT_THUMB_DEADZONE) /\
                                        (32767.0f - GAMEPAD_RIGHT_THUMB_DEADZONE), 3.0f)

#define GAMEPADSENSITIVITY_OFFSET       0.5f

int damagevibrationtics;
int weaponvibrationtics;

extern int      gamepadbuttons;
extern int      gamepadthumbLX;
extern int      gamepadthumbLY;
extern int      gamepadthumbRX;
extern boolean  vibrate;
extern int      currentmotorspeed;
extern int      idlemotorspeed;
extern int      restoremotorspeed;

extern int      gamepadautomap;
extern int      gamepadfire;
extern boolean  gamepadlefthanded;
extern int      gamepadmenu;
extern int      gamepadnextweapon;
extern int      gamepadprevweapon;
extern int      gamepadspeed;
extern int      gamepaduse;
extern boolean  gamepadvibrate;
extern int      gamepadweapon1;
extern int      gamepadweapon2;
extern int      gamepadweapon3;
extern int      gamepadweapon4;
extern int      gamepadweapon5;
extern int      gamepadweapon6;
extern int      gamepadweapon7;

void I_InitGamepad(void);
void I_ShutdownGamepad(void);
void I_PollDirectInputGamepad(void);
void I_PollXInputGamepad(void);
void I_PollThumbs_DirectInput_LeftHanded(short LX, short LY, short RX, short RY);
void I_PollThumbs_DirectInput_RightHanded(short LX, short LY, short RX, short RY);
void I_PollThumbs_XInput_LeftHanded(short LX, short LY, short RX, short RY);
void I_PollThumbs_XInput_RightHanded(short LX, short LY, short RX, short RY);
void XInputVibration(int motorspeed);
void (*gamepadfunc)(void);

#endif
