/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "c_console.h"
#include "doomstat.h"
#include "i_gamecontroller.h"
#include "m_config.h"

static SDL_GameController   *gamecontroller;
static bool                 gamecontrollerconnected;
static bool                 gamecontrollerrumbles;

int                         gamecontrollerbuttons = 0;
short                       gamecontrollerthumbLX = 0;
short                       gamecontrollerthumbLY = 0;
short                       gamecontrollerthumbRX = 0;
short                       gamecontrollerthumbRY = 0;
float                       gamecontrollerhorizontalsensitivity;
float                       gamecontrollerverticalsensitivity;
short                       gamecontrollerleftdeadzone;
short                       gamecontrollerrightdeadzone;

int                         barrelrumbletics = 0;
int                         damagerumbletics = 0;
int                         pickuprumbletics = 0;
int                         weaponrumbletics = 0;
int                         idlechainsawrumblestrength;
int                         restoredrumblestrength;

void I_InitGameController(void)
{
    if (gamecontrollerconnected)
        return;

#if defined(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE)
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1", SDL_HINT_OVERRIDE);
#endif

#if defined(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE)
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1", SDL_HINT_OVERRIDE);
#endif

    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1", SDL_HINT_OVERRIDE);

    for (int i = 0, numjoysticks = SDL_NumJoysticks(); i < numjoysticks; i++)
        if (SDL_IsGameController(i) && (gamecontroller = SDL_GameControllerOpen(i)))
        {
            const char  *name = SDL_GameControllerName(gamecontroller);
            bool        repeated;

            gamecontrollerconnected = true;

            if (name)
                repeated = C_OutputNoRepeat("A controller called \"%s\" is connected.", name);
            else
                repeated = C_OutputNoRepeat("A controller is connected.");

            if (SDL_GameControllerHasRumble(gamecontroller))
                gamecontrollerrumbles = true;
            else if (!repeated && (joy_rumble_barrels || joy_rumble_damage || joy_rumble_pickup || joy_rumble_weapons))
                C_Warning(1, "This controller won't rumble.");

            I_SetGameControllerLeftDeadZone();
            I_SetGameControllerRightDeadZone();
            I_SetGameControllerHorizontalSensitivity();
            I_SetGameControllerVerticalSensitivity();

            SDL_GameControllerSetLED(gamecontroller, 255, 0, 0);
            return;
        }
}

void I_ShutdownGameController(void)
{
    if (!gamecontroller)
        return;

    SDL_GameControllerSetLED(gamecontroller, 0, 0, 255);
    SDL_GameControllerClose(gamecontroller);
}

void I_GameControllerRumble(const int low, const int high)
{
    if (!gamecontrollerrumbles)
        return;

    SDL_GameControllerRumble(gamecontroller, MIN(low, UINT16_MAX), MIN(high, UINT16_MAX), UINT32_MAX);
}

static short inline GetAxis(short value, const short deadzone)
{
    value = SDL_GameControllerGetAxis(gamecontroller, value);

    return (ABS(value) < deadzone ? 0 : (joy_analog ? value : SIGN(value) * SDL_JOYSTICK_AXIS_MAX));
}

void I_ReadGameController(void)
{
    if (gamecontroller)
    {
        static int  prevgamecontrollerbuttons;

        if (joy_swapthumbsticks)
        {
            gamecontrollerthumbLX = GetAxis(SDL_CONTROLLER_AXIS_RIGHTX, gamecontrollerrightdeadzone);
            gamecontrollerthumbLY = GetAxis(SDL_CONTROLLER_AXIS_RIGHTY, gamecontrollerrightdeadzone);
            gamecontrollerthumbRX = GetAxis(SDL_CONTROLLER_AXIS_LEFTX, gamecontrollerleftdeadzone);
            gamecontrollerthumbRY = GetAxis(SDL_CONTROLLER_AXIS_LEFTY, gamecontrollerleftdeadzone);
        }
        else
        {
            gamecontrollerthumbLX = GetAxis(SDL_CONTROLLER_AXIS_LEFTX, gamecontrollerleftdeadzone);
            gamecontrollerthumbLY = GetAxis(SDL_CONTROLLER_AXIS_LEFTY, gamecontrollerleftdeadzone);
            gamecontrollerthumbRX = GetAxis(SDL_CONTROLLER_AXIS_RIGHTX, gamecontrollerrightdeadzone);
            gamecontrollerthumbRY = GetAxis(SDL_CONTROLLER_AXIS_RIGHTY, gamecontrollerrightdeadzone);
        }

        prevgamecontrollerbuttons = gamecontrollerbuttons;
        gamecontrollerbuttons = 0;

        if (SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) >= GAMECONTROLLER_TRIGGER_THRESHOLD)
            gamecontrollerbuttons = GAMECONTROLLER_LEFT_TRIGGER;

        if (SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) >= GAMECONTROLLER_TRIGGER_THRESHOLD)
            gamecontrollerbuttons |= GAMECONTROLLER_RIGHT_TRIGGER;

        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
            if (SDL_GameControllerGetButton(gamecontroller, i))
                gamecontrollerbuttons |= (1 << i);

        if (gamecontrollerthumbLX
            || gamecontrollerthumbLY
            || gamecontrollerthumbRX
            || gamecontrollerthumbRY
            || gamecontrollerbuttons != prevgamecontrollerbuttons)
        {
            event_t ev = { 0 };

            if (gamestate != GS_LEVEL)
                I_SaveMousePointerPosition();

            keydown = 0;
            usingmouse = false;
            ev.type = ev_controller;
            D_PostEvent(&ev);
        }

        if (gamecontrollerrumbles)
        {
            if (weaponrumbletics)
                weaponrumbletics--;

            if (damagerumbletics)
                damagerumbletics--;

            if (barrelrumbletics)
                barrelrumbletics--;

            if (pickuprumbletics)
                pickuprumbletics--;

            if (!weaponrumbletics
                && !damagerumbletics
                && !barrelrumbletics
                && !pickuprumbletics
                && !idlechainsawrumblestrength)
                SDL_GameControllerRumble(gamecontroller, 0, 0, 0);
        }
    }
}

void I_StopGameControllerRumble(void)
{
    if (!gamecontrollerrumbles)
        return;

    SDL_GameControllerRumble(gamecontroller, 0, 0, 0);
}

void I_SetGameControllerHorizontalSensitivity(void)
{
    gamecontrollerhorizontalsensitivity = (!joy_sensitivity_horizontal ? 0.0f :
        4.0f * joy_sensitivity_horizontal / joy_sensitivity_horizontal_max + 0.2f);
}

void I_SetGameControllerVerticalSensitivity(void)
{
    gamecontrollerverticalsensitivity = (!joy_sensitivity_vertical ? 0.0f :
        4.0f * joy_sensitivity_vertical / joy_sensitivity_vertical_max + 0.2f);
}

void I_SetGameControllerLeftDeadZone(void)
{
    gamecontrollerleftdeadzone = (short)(joy_deadzone_left * SHRT_MAX / 100.0f);
}

void I_SetGameControllerRightDeadZone(void)
{
    gamecontrollerrightdeadzone = (short)(joy_deadzone_right * SHRT_MAX / 100.0f);
}
