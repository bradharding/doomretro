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

#include "doomdef.h"
#include "SDL.h"

#if SDL_MAJOR_VERSION < 2 || (SDL_MAJOR_VERSION == 2 && SDL_MINOR_VERSION < 18)
#define SDL_GetTicks64  SDL_GetTicks
#endif

//
// I_GetTime
// returns time in 1/35th second tics
//
uint64_t I_GetTime(void)
{
    return (SDL_GetTicks64() * TICRATE / 1000);
}

//
// Same as I_GetTime(), but returns time in milliseconds
//
uint64_t I_GetTimeMS(void)
{
    return SDL_GetTicks64();
}

// [crispy] Get time in microseconds
uint64_t I_GetTimeUS(void)
{
    return ((SDL_GetPerformanceCounter() - performancecounter) * 1000000) / performancefrequency;
}

//
// Sleep for a specified number of milliseconds
//
void I_Sleep(int ms)
{
    SDL_Delay(ms);
}
