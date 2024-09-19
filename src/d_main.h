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

#pragma once

#include "d_event.h"
#include "doomdef.h"
#include "r_defs.h"

#define PAGETICS    (20 * TICRATE)

extern patch_t  *titlelump;
extern patch_t  *creditlump;
extern char     **episodes[];
extern char     **expansions[];
extern char     **skilllevels[];
extern char     *configfile;
extern char     *resourcewad;
extern char     *pwadfile;
extern bool     advancetitle;
extern bool     splashscreen;
extern bool     dowipe;
extern int      logotic;
extern int      pagetic;
extern int      titlesequence;
extern int      fadecount;

#if defined(_WIN32)
extern char     *previouswad;
#endif

void D_Display(void);

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//
void D_DoomMain(void);

// Called by IO functions when input is detected.
void D_PostEvent(event_t *ev);

//
// BASE LEVEL
//
void D_PageTicker(void);
void D_PageDrawer(void);
void D_SplashDrawer(void);
void D_DoAdvanceTitle(void);
void D_StartTitle(int page);
void D_FadeScreenToBlack(void);
void D_FadeScreen(bool screenshot);
bool D_IsDOOMIWAD(char *filename);
bool D_IsDOOM1IWAD(char *filename);
bool D_IsDOOM2IWAD(char *filename);
bool D_IsNERVEWAD(char *filename);
bool D_IsEXTRASWAD(char *filename);
bool D_IsSIGILWAD(char *filename);
bool D_IsSIGILSHREDSWAD(char *filename);
bool D_IsSIGIL2WAD(char *filename);
void D_CheckSupportedPWAD(char *filename);
