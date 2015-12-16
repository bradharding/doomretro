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

#if !defined(__I_GAMEPAD__)
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

#if defined(WIN32)
#define GAMEPAD_LEFT_THUMB_DEADZONE     XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
#define GAMEPAD_RIGHT_THUMB_DEADZONE    XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
#else
#define GAMEPAD_LEFT_THUMB_DEADZONE     7849
#define GAMEPAD_RIGHT_THUMB_DEADZONE    8689
#endif

#define gamepadthumbLXleft              (float)(-gamepadthumbLX - gamepadleftdeadzone) /\
                                        ((float)SHRT_MAX - gamepadleftdeadzone)
#define gamepadthumbLXright             (float)(gamepadthumbLX - gamepadleftdeadzone) /\
                                        ((float)SHRT_MAX - gamepadleftdeadzone)
#define gamepadthumbLYup                (float)(-gamepadthumbLY - gamepadleftdeadzone) /\
                                        ((float)SHRT_MAX - gamepadleftdeadzone)
#define gamepadthumbLYdown              (float)(gamepadthumbLY - gamepadleftdeadzone) /\
                                        ((float)SHRT_MAX - gamepadleftdeadzone)
#define gamepadthumbRXleft              pow((-gamepadthumbRX - gamepadrightdeadzone) /\
                                        ((float)SHRT_MAX - gamepadrightdeadzone), 3.0f)
#define gamepadthumbRXright             pow((gamepadthumbRX - gamepadrightdeadzone) /\
                                        ((float)SHRT_MAX - gamepadrightdeadzone), 3.0f)

#define GP_SENSITIVITY_OFFSET           1.0f
#define GP_SENSITIVITY_FACTOR           3.0f

int damagevibrationtics;
int weaponvibrationtics;

extern int      gamepadbuttons;
extern short    gamepadthumbLX;
extern short    gamepadthumbLY;
extern short    gamepadthumbRX;
extern dboolean vibrate;
extern int      currentmotorspeed;
extern int      idlemotorspeed;
extern int      restoremotorspeed;

extern int      gamepadalwaysrun;
extern int      gamepadautomap;
extern int      gamepadfire;
extern int      gamepadleftdeadzone;
extern int      gamepadrightdeadzone;
extern int      gamepadmenu;
extern int      gamepadnextweapon;
extern int      gamepadprevweapon;
extern int      gamepadrun;
extern int      gamepaduse;
extern int      gamepadweapon1;
extern int      gamepadweapon2;
extern int      gamepadweapon3;
extern int      gamepadweapon4;
extern int      gamepadweapon5;
extern int      gamepadweapon6;
extern int      gamepadweapon7;

extern int      gp_sensitivity;
extern dboolean gp_swapthumbsticks;
extern dboolean gp_vibrate;

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
