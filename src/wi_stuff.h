/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#pragma once

// States for the intermission
typedef enum
{
    NoState = -1,
    StatCount,
    ShowNextLoc
} stateenum_t;

// Called by main loop, animate the intermission.
void WI_Ticker(void);

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer(void);

// Setup for an intermission screen.
void WI_Start(wbstartstruct_t *wbstartstruct);

// Shut down the intermission screen
void WI_End(void);

void WI_CheckForAccelerate(void);

extern bool acceleratestage;    // accelerate intermission screens
