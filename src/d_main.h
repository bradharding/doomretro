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

#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"

#define MOUSESENSITIVITY_MIN            0
#define MOUSESENSITIVITY_DEFAULT        12
#define MOUSESENSITIVITY_MAX            64

#define MOUSEACCELERATION_DEFAULT       2.0

#define MOUSETHRESHOLD_DEFAULT          10

#define SFXVOLUME_MIN                   0
#define SFXVOLUME_DEFAULT               15
#define SFXVOLUME_MAX                   15

#define MUSICVOLUME_MIN                 0
#define MUSICVOLUME_DEFAULT             15
#define MUSICVOLUME_MAX                 15

#define LOW                             0
#define HIGH                            1
#define GRAPHICDETAIL_DEFAULT           HIGH

#define SCREENSIZE_MIN                  0
#define SCREENSIZE_DEFAULT              7
#define SCREENSIZE_MAX                  8

#define GAMMALEVEL_MIN                  gammalevels[0]
#define GAMMALEVEL_DEFAULT              0.75
#define GAMMALEVEL_MAX                  gammalevels[GAMMALEVELS - 1]

#define UNLIMITED                       32768

#define BLOODSPLATSTOTAL_MIN            0
#define BLOODSPLATSTOTAL_DEFAULT        UNLIMITED
#define BLOODSPLATSTOTAL_MAX            32768

#define BLOODSPLATSVISIBLE_MIN          0
#define BLOODSPLATSVISIBLE_DEFAULT      1024
#define BLOODSPLATSVISIBLE_MAX          32768

#define PIXELWIDTH_MIN                  2
#define PIXELWIDTH_DEFAULT              2
#define PIXELWIDTH_MAX                  SCREENWIDTH

#define PIXELHEIGHT_MIN                 2
#define PIXELHEIGHT_DEFAULT             2
#define PIXELHEIGHT_MAX                 SCREENHEIGHT

#define RUNCOUNT_MAX                    32768

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
// If not overrided by user input, calls D_AdvanceDemo.
//
void D_DoomMain(void);

// Called by IO functions when input is detected.
void D_PostEvent(event_t *ev);

// Read an event from the event queue

event_t *D_PopEvent(void);

// Read events from all input devices

void D_ProcessEvents(void);


//
// BASE LEVEL
//
void D_PageTicker(void);
void D_PageDrawer(void);
void D_AdvanceDemo(void);
void D_DoAdvanceDemo(void);
void D_StartTitle(void);


#endif
