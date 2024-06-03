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

#include "c_console.h"
#include "doomstat.h"
#include "i_gamecontroller.h"
#include "m_config.h"
#include "m_misc.h"

static SDL_GameController   *gamecontroller;
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

static char *GetGameControllerName(void)
{
    const char  *name = SDL_GameControllerName(gamecontroller);

    if (name)
        return M_StringJoin("A controller called \"", name, "\" is connected.", NULL);
    else
        return "A controller is connected.";
}

#if SDL_VERSION_ATLEAST(2, 12, 0)
static char *GetGameControllerType(void)
{
    SDL_GameControllerType  type = SDL_GameControllerGetType(gamecontroller);

    if (type == SDL_CONTROLLER_TYPE_XBOX360)
        return "An " ITALICS("Xbox 360") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_XBOXONE)
        return "An " ITALICS("Xbox One") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_PS3)
        return "A " ITALICS("PlayStation 3 DualShock") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_PS4)
        return "A " ITALICS("PlayStation 4 DualShock") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO)
        return "A " ITALICS("Nintendo Switch Pro") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_VIRTUAL)
        return "A virtual controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_PS5)
        return "A " ITALICS("PlayStation 5 DualSense") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_AMAZON_LUNA)
        return "An " ITALICS("Amazon Luna") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_GOOGLE_STADIA)
        return "A " ITALICS("Google Stadia") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_NVIDIA_SHIELD)
        return "An " ITALICS("Nvidia Shield") " controller is connected.";
    else if (type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT)
        return "A " ITALICS("Nintendo Switch's") " left joycon is connected.";
    else if (type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT)
        return "A " ITALICS("Nintendo Switch's") " right joycon is connected.";
    else if (type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR)
        return "A " ITALICS("Nintendo Switch's") " left and right joycons are connected.";
    else
        return GetGameControllerName();
}
#endif

void I_InitGameController(void)
{
    if (gamecontroller)
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
#if SDL_VERSION_ATLEAST(2, 12, 0)
            C_Output(GetGameControllerType());
#else
            C_Output(GetGameControllerName());
#endif

#if SDL_VERSION_ATLEAST(2, 18, 0)
            if (SDL_GameControllerHasRumble(gamecontroller))
                gamecontrollerrumbles = true;
            else
#endif
                if (joy_rumble_barrels || joy_rumble_damage || joy_rumble_pickup || joy_rumble_weapons)
                    C_Warning(1, "This controller doesn't rumble!");

            I_SetGameControllerLeftDeadZone();
            I_SetGameControllerRightDeadZone();
            I_SetGameControllerHorizontalSensitivity();
            I_SetGameControllerVerticalSensitivity();

#if SDL_VERSION_ATLEAST(2, 14, 0)
            SDL_GameControllerSetLED(gamecontroller, 255, 0, 0);
#endif

            return;
        }
}

void I_ShutdownGameController(void)
{
    if (!gamecontroller)
        return;

    C_Warning(1, "The controller was disconnected!");

#if SDL_VERSION_ATLEAST(2, 14, 0)
    SDL_GameControllerSetLED(gamecontroller, 0, 0, 255);
#endif

    SDL_GameControllerClose(gamecontroller);
    gamecontroller = NULL;
}

void I_GameControllerRumble(const short low, const short high)
{
    if (!gamecontrollerrumbles || !usinggamecontroller)
        return;

    SDL_GameControllerRumble(gamecontroller, MIN(low, USHRT_MAX), MIN(high, USHRT_MAX), UINT_MAX);
}

void I_ReadGameController(void)
{
    if (gamecontroller)
    {
        short       LX, LY;
        short       RX, RY;

        static int  prevgamecontrollerbuttons;

        if (joy_swapthumbsticks)
        {
            LX = SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_RIGHTX);
            LY = SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_RIGHTY);
            RX = SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_LEFTX);
            RY = SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_LEFTY);
        }
        else
        {
            LX = SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_LEFTX);
            LY = SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_LEFTY);
            RX = SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_RIGHTX);
            RY = SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_RIGHTY);
        }

        if (joy_analog)
        {
            float   magnitude;
            float   normalizedmagnitude;

            if ((magnitude = sqrtf((float)LX * LX + LY * LY)) > gamecontrollerleftdeadzone)
            {
                if (magnitude > SDL_JOYSTICK_AXIS_MAX)
                    magnitude = SDL_JOYSTICK_AXIS_MAX;

                magnitude = (magnitude - gamecontrollerleftdeadzone)
                    / (SDL_JOYSTICK_AXIS_MAX - gamecontrollerleftdeadzone);
                normalizedmagnitude = powf(magnitude, 3.0f);

                gamecontrollerthumbLX = (short)(normalizedmagnitude * LX / magnitude);
                gamecontrollerthumbLY = (short)(normalizedmagnitude * LY / magnitude);
            }
            else
            {
                gamecontrollerthumbLX = 0;
                gamecontrollerthumbLY = 0;
            }

            if ((magnitude = sqrtf((float)RX * RX + RY * RY)) > gamecontrollerrightdeadzone)
            {
                if (magnitude > SDL_JOYSTICK_AXIS_MAX)
                    magnitude = SDL_JOYSTICK_AXIS_MAX;

                magnitude = (magnitude - gamecontrollerrightdeadzone)
                    / (SDL_JOYSTICK_AXIS_MAX - gamecontrollerrightdeadzone);
                normalizedmagnitude = powf(magnitude, 3.0f);

                gamecontrollerthumbRX = (short)(normalizedmagnitude * RX / magnitude);
                gamecontrollerthumbRY = (short)(normalizedmagnitude * RY / magnitude);
            }
            else
            {
                gamecontrollerthumbRX = 0;
                gamecontrollerthumbRY = 0;
            }
        }
        else
        {
            gamecontrollerthumbLX = (ABS(LX) > gamecontrollerleftdeadzone ? SIGN(LX) * SDL_JOYSTICK_AXIS_MAX : 0);
            gamecontrollerthumbLY = (ABS(LY) > gamecontrollerleftdeadzone ? SIGN(LY) * SDL_JOYSTICK_AXIS_MAX : 0);
            gamecontrollerthumbRX = (ABS(RX) > gamecontrollerrightdeadzone ? SIGN(RX) * SDL_JOYSTICK_AXIS_MAX : 0);
            gamecontrollerthumbRY = (ABS(RY) > gamecontrollerrightdeadzone ? SIGN(RY) * SDL_JOYSTICK_AXIS_MAX : 0);
        }

        prevgamecontrollerbuttons = gamecontrollerbuttons;

        if (SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > GAMECONTROLLER_TRIGGER_THRESHOLD)
        {
            gamecontrollerbuttons = GAMECONTROLLER_LEFT_TRIGGER;
            usinggamecontroller = true;
        }
        else
            gamecontrollerbuttons = 0;

        if (SDL_GameControllerGetAxis(gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > GAMECONTROLLER_TRIGGER_THRESHOLD)
        {
            gamecontrollerbuttons |= GAMECONTROLLER_RIGHT_TRIGGER;
            usinggamecontroller = true;
        }

        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
            if (SDL_GameControllerGetButton(gamecontroller, i))
            {
                gamecontrollerbuttons |= (1 << i);
                usinggamecontroller = true;
            }

        if (gamecontrollerthumbLX
            || gamecontrollerthumbLY
            || gamecontrollerthumbRX
            || gamecontrollerthumbRY
            || gamecontrollerbuttons != prevgamecontrollerbuttons)
        {
            event_t ev = { ev_controller, 0, 0, 0 };

            if (gamestate != GS_LEVEL && usingmouse)
            {
                I_SaveMousePointerPosition();
                usingmouse = false;
            }

            keydown = 0;
            usinggamecontroller = true;
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
    gamecontrollerhorizontalsensitivity = 2.0f * joy_sensitivity_horizontal / joy_sensitivity_horizontal_max;
}

void I_SetGameControllerVerticalSensitivity(void)
{
    gamecontrollerverticalsensitivity = 2.0f * joy_sensitivity_vertical / joy_sensitivity_vertical_max;
}

void I_SetGameControllerLeftDeadZone(void)
{
    gamecontrollerleftdeadzone = (short)(joy_deadzone_left * SDL_JOYSTICK_AXIS_MAX / 100.0f);
}

void I_SetGameControllerRightDeadZone(void)
{
    gamecontrollerrightdeadzone = (short)(joy_deadzone_right * SDL_JOYSTICK_AXIS_MAX / 100.0f);
}
