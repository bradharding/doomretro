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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <XInput.h>

typedef DWORD(WINAPI *XINPUTGETSTATE)(DWORD, XINPUT_STATE *);
typedef DWORD(WINAPI *XINPUTSETSTATE)(DWORD, XINPUT_VIBRATION *);

static XINPUTGETSTATE pXInputGetState;
static XINPUTSETSTATE pXInputSetState;
#endif

#include "d_main.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "m_config.h"
#include "m_fixed.h"
#include "SDL.h"
#include "SDL_joystick.h"

static SDL_Joystick     *gamepad = NULL;

int                     gamepadbuttons = 0;
short                   gamepadthumbLX;
short                   gamepadthumbLY;
short                   gamepadthumbRX;

boolean                 vibrate = false;

extern boolean          idclev;
extern boolean          idmus;
extern boolean          idbehold;
extern boolean          menuactive;

void (*gamepadfunc)(void);
void (*gamepadthumbsfunc)(short, short, short, short);

void I_InitGamepad(void)
{
    gamepadfunc = I_PollDirectInputGamepad;
    gamepadthumbsfunc = (gamepadlefthanded ? I_PollThumbs_DirectInput_LeftHanded :
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
#ifdef WIN32
            HMODULE     pXInputDLL;

            if (!(pXInputDLL = LoadLibrary("XInput1_4.dll")))
                if (!(pXInputDLL = LoadLibrary("XInput9_1_0.dll")))
                    pXInputDLL = LoadLibrary("XInput1_3.dll");

            if (pXInputDLL)
            {
                M_SaveDefaults();

                pXInputGetState = (XINPUTGETSTATE)GetProcAddress(pXInputDLL, "XInputGetState");
                pXInputSetState = (XINPUTSETSTATE)GetProcAddress(pXInputDLL, "XInputSetState");

                if (pXInputGetState && pXInputSetState)
                {
                    XINPUT_STATE        state;

                    ZeroMemory(&state, sizeof(XINPUT_STATE));

                    if (pXInputGetState(0, &state) == ERROR_SUCCESS)
                    {
                        gamepadfunc = I_PollXInputGamepad;
                        gamepadthumbsfunc = (gamepadlefthanded ? I_PollThumbs_XInput_LeftHanded :
                            I_PollThumbs_XInput_RightHanded);
                    }
                }
            }
#endif

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
                HU_clearMessages();
                idbehold = false;
            }
        }

        if (gamepadsensitivity || menuactive || (gamepadbuttons & gamepadmenu))
        {
            event_t     ev;

            ev.type = ev_gamepad;
            ev.data1 = gamepadbuttons;
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
#ifdef WIN32
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
#ifdef WIN32
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
                HU_clearMessages();
                idbehold = false;
            }
        }

        if (gamepadsensitivity || menuactive || (gamepadbuttons & gamepadmenu))
        {
            event_t      ev;

            ev.type = ev_gamepad;
            ev.data1 = gamepadbuttons;
            D_PostEvent(&ev);

            gamepadthumbsfunc(Gamepad.sThumbLX, Gamepad.sThumbLY, Gamepad.sThumbRX, Gamepad.sThumbRY);
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
