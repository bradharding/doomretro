/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

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

#include "c_console.h"
#include "i_gamepad.h"
#include "m_config.h"
#include "m_fixed.h"
#include "m_misc.h"

dboolean                    gp_analog = gp_analog_default;
float                       gp_deadzone_left = gp_deadzone_left_default;
float                       gp_deadzone_right = gp_deadzone_right_default;
dboolean                    gp_invertyaxis = gp_invertyaxis_default;
int                         gp_sensitivity_horizontal = gp_sensitivity_horizontal_default;
int                         gp_sensitivity_vertical = gp_sensitivity_vertical_default;
dboolean                    gp_swapthumbsticks = gp_swapthumbsticks_default;
int                         gp_thumbsticks = gp_thumbsticks_default;
int                         gp_vibrate_barrels = gp_vibrate_barrels_default;
int                         gp_vibrate_damage = gp_vibrate_damage_default;
int                         gp_vibrate_weapons = gp_vibrate_weapons_default;

static SDL_Joystick         *joystick;
static SDL_GameController   *gamecontroller;
static SDL_Haptic           *haptic;

int                         gamepadbuttons = 0;
short                       gamepadthumbLX = 0;
short                       gamepadthumbLY = 0;
short                       gamepadthumbRX = 0;
short                       gamepadthumbRY = 0;
float                       gamepadhorizontalsensitivity;
float                       gamepadverticalsensitivity;
short                       gamepadleftdeadzone;
short                       gamepadrightdeadzone;

int                         barrelvibrationtics = 0;
int                         damagevibrationtics = 0;
int                         weaponvibrationtics = 0;
int                         idlevibrationstrength;
int                         restorevibrationstrength;

void I_InitGamepad(void)
{
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0)
        C_Warning(1, "Gamepad support couldn't be initialized.");
    else
    {
        for (int i = 0, numjoysticks = SDL_NumJoysticks(); i < numjoysticks; i++)
            if ((joystick = SDL_JoystickOpen(i)) && SDL_IsGameController(i))
            {
                gamecontroller = SDL_GameControllerOpen(i);
                break;
            }

        if (!gamecontroller)
            SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
        else
        {
            const char  *name = SDL_GameControllerName(gamecontroller);

            if (*name)
            {
                if (M_StrCaseStr(name, "xinput"))
                    C_OutputNoRepeat("An <i><b>XInput</b></i> gamepad is connected.");
                else
                    C_OutputNoRepeat("A <i><b>DirectInput</b></i> gamepad called \"%s\" is connected.", name);
            }
            else
                C_OutputNoRepeat("A gamepad is connected.");

            if (!(haptic = SDL_HapticOpenFromJoystick(joystick)) || SDL_HapticRumbleInit(haptic) < 0)
            {
                haptic = NULL;
                C_Warning(1, "This gamepad doesn't support vibration.");
            }

            SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1", SDL_HINT_OVERRIDE);
        }
    }
}

void I_ShutdownGamepad(void)
{
    if (!gamecontroller)
        return;

    if (haptic)
    {
        SDL_HapticClose(haptic);
        haptic = NULL;
        barrelvibrationtics = 0;
        damagevibrationtics = 0;
        weaponvibrationtics = 0;
    }

    SDL_GameControllerClose(gamecontroller);
    gamecontroller = NULL;

    SDL_JoystickClose(joystick);
    joystick = NULL;

    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
}

void I_GamepadVibration(int strength)
{
    static int  currentstrength;

    if (!haptic)
        return;

    if (!strength || (lasteventtype == ev_gamepad && (strength == idlevibrationstrength || strength >= currentstrength)))
    {
        currentstrength = MIN(strength, UINT16_MAX);
        SDL_HapticRumblePlay(haptic, (float)strength / MAXVIBRATIONSTRENGTH, 600000);
    }
}

void I_UpdateGamepadVibration(void)
{
    if (!haptic)
        return;

    if (weaponvibrationtics && !--weaponvibrationtics && !damagevibrationtics && !barrelvibrationtics)
        I_GamepadVibration(idlevibrationstrength);
    else if (damagevibrationtics && !--damagevibrationtics && !weaponvibrationtics && !barrelvibrationtics)
        I_GamepadVibration(idlevibrationstrength);
    else if (barrelvibrationtics && !--barrelvibrationtics && !weaponvibrationtics && !damagevibrationtics)
        I_GamepadVibration(idlevibrationstrength);
}

void I_StopGamepadVibration(void)
{
    if (haptic)
        SDL_HapticRumbleStop(haptic);
}

void I_SetGamepadHorizontalSensitivity(void)
{
    gamepadhorizontalsensitivity = (!gp_sensitivity_horizontal ? 0.0f :
        4.0f * gp_sensitivity_horizontal / gp_sensitivity_horizontal_max + 0.2f);
}

void I_SetGamepadVerticalSensitivity(void)
{
    gamepadverticalsensitivity = (!gp_sensitivity_vertical ? 0.0f :
        4.0f * gp_sensitivity_vertical / gp_sensitivity_vertical_max + 0.2f);
}

void I_SetGamepadLeftDeadZone(void)
{
    gamepadleftdeadzone = (short)(gp_deadzone_left * SHRT_MAX / 100.0f);
}

void I_SetGamepadRightDeadZone(void)
{
    gamepadrightdeadzone = (short)(gp_deadzone_right * SHRT_MAX / 100.0f);
}
