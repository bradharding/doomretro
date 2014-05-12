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

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <XInput.h>

#if (_MSC_PLATFORM_TOOLSET <= 100)
#pragma comment(lib, "XInput.lib")
#else
#pragma comment(lib, "XInput9_1_0.lib")
#endif

#include "d_main.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "m_fixed.h"
#include "SDL.h"
#include "SDL_joystick.h"

static SDL_Joystick *gamepad = NULL;

int gamepadbuttons;
int gamepadthumbLX;
int gamepadthumbLY;
int gamepadthumbRX;

boolean vibrate = false;

void (*gamepadfunc)(void);
void (*gamepadthumbsfunc)(short, short, short, short);

extern int vibrationtics;
extern boolean idclev;
extern boolean idmus;
extern boolean idbehold;

void I_InitGamepad(void)
{
    gamepadfunc = I_PollDirectInputGamepad;
    gamepadthumbsfunc = (gamepadlefthanded ? I_PollThumbs_DirectInput_LeftHanded :
        I_PollThumbs_DirectInput_RightHanded);

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
        return;
    else
    {
        int i;
        int numgamepads = SDL_NumJoysticks();

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
            XINPUT_STATE state;

            if (XInputGetState(0, &state) == ERROR_SUCCESS)
            {
                gamepadfunc = I_PollXInputGamepad;
                gamepadthumbsfunc = (gamepadlefthanded ? I_PollThumbs_XInput_LeftHanded :
                    I_PollThumbs_XInput_RightHanded);
            }

            SDL_JoystickEventState(SDL_ENABLE);
        }
    }
}

void I_ShutdownGamepad(void)
{
    if (gamepad)
    {
        SDL_JoystickClose(gamepad);
        gamepad = NULL;
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }
}

static int __inline clamp(int value, int deadzone)
{
    if (value > -deadzone && value < deadzone)
        return 0;
    return MAX(-32767, value);
}

void I_PollThumbs_DirectInput_RightHanded(short LX, short LY, short RX, short RY)
{
    gamepadthumbLX = clamp(SDL_JoystickGetAxis(gamepad, LX), GAMEPAD_LEFT_THUMB_DEADZONE);
    gamepadthumbLY = clamp(SDL_JoystickGetAxis(gamepad, LY), GAMEPAD_LEFT_THUMB_DEADZONE);
    gamepadthumbRX = clamp(SDL_JoystickGetAxis(gamepad, RX), GAMEPAD_RIGHT_THUMB_DEADZONE);
}

void I_PollThumbs_DirectInput_LeftHanded(short LX, short LY, short RX, short RY)
{
    gamepadthumbLX = clamp(SDL_JoystickGetAxis(gamepad, RX), GAMEPAD_RIGHT_THUMB_DEADZONE);
    gamepadthumbLY = clamp(SDL_JoystickGetAxis(gamepad, RY), GAMEPAD_RIGHT_THUMB_DEADZONE);
    gamepadthumbRX = clamp(SDL_JoystickGetAxis(gamepad, LX), GAMEPAD_LEFT_THUMB_DEADZONE);
}

void I_PollDirectInputGamepad(void)
{
    if (gamepad)
    {
        event_t         ev;
        int             hat = SDL_JoystickGetHat(gamepad, 0);

        gamepadthumbsfunc(0, 1, 2, 3);

        gamepadbuttons = (GAMEPAD_X * SDL_JoystickGetButton(gamepad, 0) |
            GAMEPAD_A * SDL_JoystickGetButton(gamepad, 1) |
            GAMEPAD_B * SDL_JoystickGetButton(gamepad, 2) |
            GAMEPAD_Y * SDL_JoystickGetButton(gamepad, 3) |
            GAMEPAD_LEFT_SHOULDER * SDL_JoystickGetButton(gamepad, 4) |
            GAMEPAD_RIGHT_SHOULDER * SDL_JoystickGetButton(gamepad, 5) |
            GAMEPAD_LEFT_TRIGGER * SDL_JoystickGetButton(gamepad, 6) |
            GAMEPAD_RIGHT_TRIGGER * SDL_JoystickGetButton(gamepad, 7) |
            GAMEPAD_BACK * SDL_JoystickGetButton(gamepad, 8) |
            GAMEPAD_START * SDL_JoystickGetButton(gamepad, 9) |
            GAMEPAD_LEFT_THUMB * SDL_JoystickGetButton(gamepad, 10) |
            GAMEPAD_RIGHT_THUMB * SDL_JoystickGetButton(gamepad, 11) |
            GAMEPAD_DPAD_UP * (hat & SDL_HAT_UP) |
            GAMEPAD_DPAD_RIGHT * (hat & SDL_HAT_RIGHT) |
            GAMEPAD_DPAD_DOWN * (hat & SDL_HAT_DOWN) |
            GAMEPAD_DPAD_LEFT * (hat & SDL_HAT_LEFT));

        if (gamepadbuttons)
        {
            idclev = false;
            idmus = false;
            if (idbehold)
            {
                HU_clearMessages();
                idbehold = false;
            }
        }

        ev.type = ev_gamepad;
        ev.data1 = gamepadbuttons;
        D_PostEvent(&ev);
    }
}

void XInputVibration(int left, int right)
{
    XINPUT_VIBRATION    vibration;

    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    vibration.wLeftMotorSpeed = left;
    vibration.wRightMotorSpeed = right;
    XInputSetState(0, &vibration);
}

void I_PollThumbs_XInput_RightHanded(short LX, short LY, short RX, short RY)
{
    gamepadthumbLX = clamp(LX, GAMEPAD_LEFT_THUMB_DEADZONE);
    gamepadthumbLY = -clamp(LY, GAMEPAD_LEFT_THUMB_DEADZONE);
    gamepadthumbRX = clamp(RX, GAMEPAD_RIGHT_THUMB_DEADZONE);
}

void I_PollThumbs_XInput_LeftHanded(short LX, short LY, short RX, short RY)
{
    gamepadthumbLX = clamp(RX, GAMEPAD_RIGHT_THUMB_DEADZONE);
    gamepadthumbLY = -clamp(RY, GAMEPAD_RIGHT_THUMB_DEADZONE);
    gamepadthumbRX = clamp(LX, GAMEPAD_LEFT_THUMB_DEADZONE);
}

void I_PollXInputGamepad(void)
{
    if (gamepad)
    {
        event_t         ev;
        XINPUT_STATE    state;
        XINPUT_GAMEPAD  Gamepad;

        ZeroMemory(&state, sizeof(XINPUT_STATE));
        XInputGetState(0, &state);
        Gamepad = state.Gamepad;

        gamepadthumbsfunc(Gamepad.sThumbLX, Gamepad.sThumbLY, Gamepad.sThumbRX, Gamepad.sThumbRY);

        gamepadbuttons = (state.Gamepad.wButtons |
            GAMEPAD_LEFT_TRIGGER * (state.Gamepad.bLeftTrigger > GAMEPAD_TRIGGER_THRESHOLD) |
            GAMEPAD_RIGHT_TRIGGER * (state.Gamepad.bRightTrigger > GAMEPAD_TRIGGER_THRESHOLD));

        if (vibrationtics)
            if (!(--vibrationtics))
                XInputVibration(0, 0);

        if (gamepadbuttons)
        {
            vibrate = true;
            idclev = false;
            idmus = false;
            if (idbehold)
            {
                HU_clearMessages();
                idbehold = false;
            }
        }

        ev.type = ev_gamepad;
        ev.data1 = gamepadbuttons;
        D_PostEvent(&ev);
    }
}
