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

#include "SDL.h"
#include "SDL_joystick.h"
#include "d_main.h"
#include "i_gamepad.h"
#include "hu_stuff.h"

#include "i_system.h"

static SDL_Joystick *gamepad = NULL;

int gamepadbuttons;
int gamepadthumbLX;
int gamepadthumbLY;
int gamepadthumbRX;

boolean vibrate = false;

void (*gamepadfunc)(void);

extern int vibrationtics;
extern boolean idclev;
extern boolean idmus;
extern boolean idbehold;

void I_InitGamepad(void)
{
    gamepadfunc = I_PollDirectInputGamepad;

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
                gamepadfunc = I_PollXInputGamepad;

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
    if (value == -32768)
        value = -32767;
    return value;
}

void I_PollDirectInputGamepad(void)
{
    if (gamepad)
    {
        event_t         ev;
        int             hat;

        if (gamepadlefthanded)
        {
            gamepadthumbLX = clamp(SDL_JoystickGetAxis(gamepad, 2),
                                   GAMEPAD_RIGHT_THUMB_DEADZONE);
            gamepadthumbLY = clamp(SDL_JoystickGetAxis(gamepad, 3),
                                   GAMEPAD_RIGHT_THUMB_DEADZONE);
            gamepadthumbRX = clamp(SDL_JoystickGetAxis(gamepad, 0),
                                   GAMEPAD_LEFT_THUMB_DEADZONE);
        }
        else
        {
            gamepadthumbLX = clamp(SDL_JoystickGetAxis(gamepad, 0),
                                   GAMEPAD_LEFT_THUMB_DEADZONE);
            gamepadthumbLY = clamp(SDL_JoystickGetAxis(gamepad, 1),
                                   GAMEPAD_LEFT_THUMB_DEADZONE);
            gamepadthumbRX = clamp(SDL_JoystickGetAxis(gamepad, 2),
                                   GAMEPAD_RIGHT_THUMB_DEADZONE);
        }

        gamepadbuttons = 0;
        if (SDL_JoystickGetButton(gamepad, 0))
            gamepadbuttons |= GAMEPAD_X;
        if (SDL_JoystickGetButton(gamepad, 1))
            gamepadbuttons |= GAMEPAD_A;
        if (SDL_JoystickGetButton(gamepad, 2))
            gamepadbuttons |= GAMEPAD_B;
        if (SDL_JoystickGetButton(gamepad, 3))
            gamepadbuttons |= GAMEPAD_Y;
        if (SDL_JoystickGetButton(gamepad, 4))
            gamepadbuttons |= GAMEPAD_LEFT_SHOULDER;
        if (SDL_JoystickGetButton(gamepad, 5))
            gamepadbuttons |= GAMEPAD_RIGHT_SHOULDER;
        if (SDL_JoystickGetButton(gamepad, 6))
            gamepadbuttons |= GAMEPAD_LEFT_TRIGGER;
        if (SDL_JoystickGetButton(gamepad, 7))
            gamepadbuttons |= GAMEPAD_RIGHT_TRIGGER;
        if (SDL_JoystickGetButton(gamepad, 8))
            gamepadbuttons |= GAMEPAD_BACK;
        if (SDL_JoystickGetButton(gamepad, 9))
            gamepadbuttons |= GAMEPAD_START;
        if (SDL_JoystickGetButton(gamepad, 10))
            gamepadbuttons |= GAMEPAD_LEFT_THUMB;
        if (SDL_JoystickGetButton(gamepad, 11))
            gamepadbuttons |= GAMEPAD_RIGHT_THUMB;

        hat = SDL_JoystickGetHat(gamepad, 0);
        if (hat & SDL_HAT_UP)
            gamepadbuttons |= GAMEPAD_DPAD_UP;
        if (hat & SDL_HAT_RIGHT)
            gamepadbuttons |= GAMEPAD_DPAD_RIGHT;
        if (hat & SDL_HAT_DOWN)
            gamepadbuttons |= GAMEPAD_DPAD_DOWN;
        if (hat & SDL_HAT_LEFT)
            gamepadbuttons |= GAMEPAD_DPAD_LEFT;

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

void I_PollXInputGamepad(void)
{
    if (gamepad)
    {
        event_t         ev;
        XINPUT_STATE    state;

        ZeroMemory(&state, sizeof(XINPUT_STATE));
        XInputGetState(0, &state);

        if (gamepadlefthanded)
        {
            gamepadthumbLX = clamp(state.Gamepad.sThumbRX, GAMEPAD_RIGHT_THUMB_DEADZONE);
            gamepadthumbLY = -clamp(state.Gamepad.sThumbRY, GAMEPAD_RIGHT_THUMB_DEADZONE);
            gamepadthumbRX = clamp(state.Gamepad.sThumbLX, GAMEPAD_LEFT_THUMB_DEADZONE);
        }
        else
        {
            gamepadthumbLX = clamp(state.Gamepad.sThumbLX, GAMEPAD_LEFT_THUMB_DEADZONE);
            gamepadthumbLY = -clamp(state.Gamepad.sThumbLY, GAMEPAD_LEFT_THUMB_DEADZONE);
            gamepadthumbRX = clamp(state.Gamepad.sThumbRX, GAMEPAD_RIGHT_THUMB_DEADZONE);
        }

        gamepadbuttons = state.Gamepad.wButtons;

        if (state.Gamepad.bLeftTrigger > GAMEPAD_TRIGGER_THRESHOLD)
            gamepadbuttons |= GAMEPAD_LEFT_TRIGGER;
        if (state.Gamepad.bRightTrigger > GAMEPAD_TRIGGER_THRESHOLD)
            gamepadbuttons |= GAMEPAD_RIGHT_TRIGGER;

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
