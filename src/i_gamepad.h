/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
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

#if !defined(__I_GAMEPAD_H__)
#define __I_GAMEPAD_H__

#include "doomtype.h"

#define GAMEPAD_A                       0x00000001
#define GAMEPAD_B                       0x00000002
#define GAMEPAD_X                       0x00000004
#define GAMEPAD_Y                       0x00000008
#define GAMEPAD_BACK                    0x00000010
#define GAMEPAD_GUIDE                   0x00000020
#define GAMEPAD_START                   0x00000040
#define GAMEPAD_LEFT_THUMB              0x00000080
#define GAMEPAD_RIGHT_THUMB             0x00000100
#define GAMEPAD_LEFT_SHOULDER           0x00000200
#define GAMEPAD_RIGHT_SHOULDER          0x00000400
#define GAMEPAD_DPAD_UP                 0x00000800
#define GAMEPAD_DPAD_DOWN               0x00001000
#define GAMEPAD_DPAD_LEFT               0x00002000
#define GAMEPAD_DPAD_RIGHT              0x00004000
#define GAMEPAD_LEFT_TRIGGER            0x00010000
#define GAMEPAD_RIGHT_TRIGGER           0x00020000

#define GAMEPAD_TRIGGER_THRESHOLD       3855

#define MAXVIBRATIONSTRENGTH            65535
#define CHAINSAWIDLEVIBRATIONSTRENGTH   15000

#define gamepadthumbLXleft              (float)(-gamepadthumbLX - gamepadleftdeadzone) / (SHRT_MAX - gamepadleftdeadzone)
#define gamepadthumbLXright             (float)(gamepadthumbLX - gamepadleftdeadzone) / (SHRT_MAX - gamepadleftdeadzone)
#define gamepadthumbLYup                (float)(-gamepadthumbLY - gamepadleftdeadzone) / (SHRT_MAX - gamepadleftdeadzone)
#define gamepadthumbLYdown              (float)(gamepadthumbLY - gamepadleftdeadzone) / (SHRT_MAX - gamepadleftdeadzone)
#define gamepadthumbRXleft              powf((float)(-gamepadthumbRX - gamepadrightdeadzone) / (SHRT_MAX - gamepadrightdeadzone), 3.0f)
#define gamepadthumbRXright             powf((float)(gamepadthumbRX - gamepadrightdeadzone) / (SHRT_MAX - gamepadrightdeadzone), 3.0f)
#define gamepadthumbRYup                (-(float)(-gamepadthumbRY - gamepadrightdeadzone) / (SHRT_MAX - gamepadrightdeadzone))
#define gamepadthumbRYdown              (float)(gamepadthumbRY - gamepadrightdeadzone) / (SHRT_MAX - gamepadrightdeadzone)

extern int      barrelvibrationtics;
extern int      damagevibrationtics;
extern int      weaponvibrationtics;

extern int      gamepadbuttons;
extern short    gamepadthumbLX;
extern short    gamepadthumbLY;
extern short    gamepadthumbRX;
extern short    gamepadthumbRY;
extern int      idlevibrationstrength;
extern int      restorevibrationstrength;
extern float    gamepadsensitivity;
extern short    gamepadleftdeadzone;
extern short    gamepadrightdeadzone;

void I_InitGamepad(void);
void I_ShutdownGamepad(void);
void I_GamepadVibration(int strength);
void I_UpdateGamepadVibration(void);
void I_StopGamepadVibration(void);
void I_SetGamepadSensitivity(void);
void I_SetGamepadLeftDeadZone(void);
void I_SetGamepadRightDeadZone(void);

#endif
