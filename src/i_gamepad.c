/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

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

#if defined(WIN32)
#include <Windows.h>
#include <XInput.h>

typedef DWORD(WINAPI *XINPUTGETSTATE)(DWORD, XINPUT_STATE *);
typedef DWORD(WINAPI *XINPUTSETSTATE)(DWORD, XINPUT_VIBRATION *);

static XINPUTGETSTATE pXInputGetState;
static XINPUTSETSTATE pXInputSetState;
#endif

#include "c_console.h"
#include "d_main.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_fixed.h"
#include "m_misc.h"
#include "SDL.h"
#include "SDL_joystick.h"

float                   gp_deadzone_left = gp_deadzone_left_default;
float                   gp_deadzone_right = gp_deadzone_right_default;

static SDL_Joystick     *gamepad;

int                     gamepadbuttons = 0;
short                   gamepadthumbLX = 0;
short                   gamepadthumbLY = 0;
short                   gamepadthumbRX = 0;
float                   gamepadsensitivity;
int                     gamepadleftdeadzone;
int                     gamepadrightdeadzone;

dboolean                vibrate = false;

extern dboolean         idclev;
extern dboolean         idmus;
extern dboolean         idbehold;
extern dboolean         menuactive;
extern dboolean         message_clearable;

#if defined(WIN32)
HMODULE                 pXInputDLL;
#endif

void (*gamepadfunc)(void);
void (*gamepadthumbsfunc)(short, short, short, short);

void I_InitGamepad(void)
{
    gamepadfunc = I_PollDirectInputGamepad;
    gamepadthumbsfunc = (gp_swapthumbsticks ? I_PollThumbs_DirectInput_LeftHanded :
        I_PollThumbs_DirectInput_RightHanded);

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
        return;
    else
    {
        int     i;
        int     numgamepads = SDL_NumJoysticks();

        for (i = 0; i < numgamepads; ++i)
        {
            gamepad = SDL_JoystickOpen(i);
            if (gamepad)
                break;
        }

        if (!gamepad)
        {
            SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
            return;
        }
        else
        {
#if defined(WIN32)
            char        *XInputDLL = malloc(16);
            static int  initcount;

            if ((pXInputDLL = LoadLibrary("XInput1_4.dll")))
                M_StringCopy(XInputDLL, "XINPUT1_4.DLL", 16);
            else if ((pXInputDLL = LoadLibrary("XInput9_1_0.dll")))
                M_StringCopy(XInputDLL, "XINPUT9_1_0.DLL", 16);
            else if ((pXInputDLL = LoadLibrary("XInput1_3.dll")))
                M_StringCopy(XInputDLL, "XINPUT1_3.DLL", 16);

            ++initcount;

            if (pXInputDLL)
            {
                pXInputGetState = (XINPUTGETSTATE)GetProcAddress(pXInputDLL, "XInputGetState");
                pXInputSetState = (XINPUTSETSTATE)GetProcAddress(pXInputDLL, "XInputSetState");

                if (pXInputGetState && pXInputSetState)
                {
                    XINPUT_STATE        state;

                    ZeroMemory(&state, sizeof(XINPUT_STATE));

                    if (pXInputGetState(0, &state) == ERROR_SUCCESS)
                    {
                        gamepadfunc = I_PollXInputGamepad;
                        gamepadthumbsfunc = (gp_swapthumbsticks ? I_PollThumbs_XInput_LeftHanded :
                            I_PollThumbs_XInput_RightHanded);
                        if (initcount == 1)
                            C_Output("XInput gamepad detected. Using %s.", XInputDLL);
                    }
                }
                else
                    FreeLibrary(pXInputDLL);
            }
            else if (initcount == 1)
                C_Output("DirectInput gamepad \"%s\" detected.", SDL_JoystickName(gamepad));

            free(XInputDLL);
#else
            C_Output("DirectInput gamepad \"%s\" detected.", SDL_JoystickName(gamepad));
#endif

            SDL_JoystickEventState(SDL_ENABLE);
        }
    }
}

void I_ShutdownGamepad(void)
{
#if defined(WIN32)
    if (pXInputDLL)
        FreeLibrary(pXInputDLL);
#endif

    if (gamepad)
    {
        SDL_JoystickClose(gamepad);
        gamepad = NULL;
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }
}

static short __inline clamp(short value, short deadzone)
{
    return (ABS(value) < deadzone ? 0 : MAX(-SHRT_MAX, value));
}

void I_PollThumbs_DirectInput_RightHanded(short LX, short LY, short RX, short RY)
{
    gamepadthumbLX = clamp(SDL_JoystickGetAxis(gamepad, LX), gamepadleftdeadzone);
    gamepadthumbLY = clamp(SDL_JoystickGetAxis(gamepad, LY), gamepadleftdeadzone);
    gamepadthumbRX = clamp(SDL_JoystickGetAxis(gamepad, RX), gamepadrightdeadzone);
}

void I_PollThumbs_DirectInput_LeftHanded(short LX, short LY, short RX, short RY)
{
    gamepadthumbLX = clamp(SDL_JoystickGetAxis(gamepad, RX), gamepadrightdeadzone);
    gamepadthumbLY = clamp(SDL_JoystickGetAxis(gamepad, RY), gamepadrightdeadzone);
    gamepadthumbRX = clamp(SDL_JoystickGetAxis(gamepad, LX), gamepadleftdeadzone);
}

void I_PollDirectInputGamepad(void)
{
    if (gamepad)
    {
        int     hat = SDL_JoystickGetHat(gamepad, 0);

        gamepadbuttons = (GAMEPAD_X * SDL_JoystickGetButton(gamepad, 0)
            | GAMEPAD_A * SDL_JoystickGetButton(gamepad, 1)
            | GAMEPAD_B * SDL_JoystickGetButton(gamepad, 2)
            | GAMEPAD_Y * SDL_JoystickGetButton(gamepad, 3)
            | GAMEPAD_LEFT_SHOULDER * SDL_JoystickGetButton(gamepad, 4)
            | GAMEPAD_RIGHT_SHOULDER * SDL_JoystickGetButton(gamepad, 5)
            | GAMEPAD_LEFT_TRIGGER * SDL_JoystickGetButton(gamepad, 6)
            | GAMEPAD_RIGHT_TRIGGER * SDL_JoystickGetButton(gamepad, 7)
            | GAMEPAD_BACK * SDL_JoystickGetButton(gamepad, 8)
            | GAMEPAD_START * SDL_JoystickGetButton(gamepad, 9)
            | GAMEPAD_LEFT_THUMB * SDL_JoystickGetButton(gamepad, 10)
            | GAMEPAD_RIGHT_THUMB * SDL_JoystickGetButton(gamepad, 11)
            | GAMEPAD_DPAD_UP * (hat & SDL_HAT_UP)
            | GAMEPAD_DPAD_RIGHT * (hat & SDL_HAT_RIGHT)
            | GAMEPAD_DPAD_DOWN * (hat & SDL_HAT_DOWN)
            | GAMEPAD_DPAD_LEFT * (hat & SDL_HAT_LEFT));

        if (gamepadbuttons)
        {
            idclev = false;
            idmus = false;
            if (idbehold)
            {
                message_clearable = true;
                HU_ClearMessages();
                idbehold = false;
            }
        }

        if (gp_sensitivity || menuactive || (gamepadbuttons & gamepadmenu))
        {
            event_t     ev;

            ev.type = ev_gamepad;
            D_PostEvent(&ev);

            gamepadthumbsfunc(0, 1, 2, 3);
        }
        else
        {
            gamepadbuttons = 0;
            gamepadthumbLX = 0;
            gamepadthumbLY = 0;
            gamepadthumbRX = 0;
        }
    }
}

int currentmotorspeed = 0;
int idlemotorspeed = 0;
int restoremotorspeed = 0;

void XInputVibration(int motorspeed)
{
#if defined(WIN32)
    if (motorspeed > currentmotorspeed || motorspeed == idlemotorspeed)
    {
        XINPUT_VIBRATION    vibration;

        ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
        vibration.wLeftMotorSpeed = currentmotorspeed = motorspeed;
        pXInputSetState(0, &vibration);
    }
#endif
}

void I_PollThumbs_XInput_RightHanded(short LX, short LY, short RX, short RY)
{
    gamepadthumbLX = clamp(LX, gamepadleftdeadzone);
    gamepadthumbLY = -clamp(LY, gamepadleftdeadzone);
    gamepadthumbRX = clamp(RX, gamepadrightdeadzone);
}

void I_PollThumbs_XInput_LeftHanded(short LX, short LY, short RX, short RY)
{
    gamepadthumbLX = clamp(RX, gamepadrightdeadzone);
    gamepadthumbLY = -clamp(RY, gamepadrightdeadzone);
    gamepadthumbRX = clamp(LX, gamepadleftdeadzone);
}

void I_PollXInputGamepad(void)
{
#if defined(WIN32)
    if (gamepad)
    {
        XINPUT_STATE    state;
        XINPUT_GAMEPAD  Gamepad;

        ZeroMemory(&state, sizeof(XINPUT_STATE));
        pXInputGetState(0, &state);
        Gamepad = state.Gamepad;

        gamepadbuttons = (Gamepad.wButtons
            | GAMEPAD_LEFT_TRIGGER * (Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            | GAMEPAD_RIGHT_TRIGGER * (Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD));

        if (damagevibrationtics)
            if (!--damagevibrationtics && !weaponvibrationtics)
                XInputVibration(idlemotorspeed);

        if (weaponvibrationtics)
            if (!--weaponvibrationtics && !damagevibrationtics)
                XInputVibration(idlemotorspeed);

        if (gamepadbuttons)
        {
            vibrate = true;
            idclev = false;
            idmus = false;
            if (idbehold)
            {
                message_clearable = true;
                HU_ClearMessages();
                idbehold = false;
            }
        }

        if (gp_sensitivity || menuactive || (gamepadbuttons & gamepadmenu))
        {
            event_t      ev;

            ev.type = ev_gamepad;
            D_PostEvent(&ev);

            gamepadthumbsfunc(Gamepad.sThumbLX, Gamepad.sThumbLY, Gamepad.sThumbRX,
                Gamepad.sThumbRY);
        }
        else
        {
            gamepadbuttons = 0;
            gamepadthumbLX = 0;
            gamepadthumbLY = 0;
            gamepadthumbRX = 0;
        }
    }
#endif
}

void I_SetGamepadSensitivity(int value)
{
    gamepadsensitivity = (!value ? 0.0f : GP_SENSITIVITY_OFFSET
        + GP_SENSITIVITY_FACTOR * value / gp_sensitivity_max);
}
