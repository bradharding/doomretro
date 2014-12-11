/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#include "doomdef.h"
#include "i_timer.h"
#include "SDL.h"


//
// I_GetTime
// returns time in 1/35th second tics
//
static Uint32 basetime = 0;

int I_GetTime(void)
{
    Uint32      ticks = SDL_GetTicks();

    if (!basetime)
        basetime = ticks;

    return ((ticks - basetime) * TICRATE) / 1000;
}

//
// Same as I_GetTime, but returns time in milliseconds
//
int I_GetTimeMS(void)
{
    Uint32      ticks = SDL_GetTicks();

    if (!basetime)
        basetime = ticks;

    return (ticks - basetime);
}

//
// Sleep for a specified number of ms
//
void I_Sleep(int ms)
{
    SDL_Delay(ms);
}

void I_InitTimer(void)
{
    // initialize timer
    SDL_InitSubSystem(SDL_INIT_TIMER);
}
