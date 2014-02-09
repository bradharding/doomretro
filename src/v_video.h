/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

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

#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomtype.h"

#include "doomdef.h"

// Needed because we are refering to patches.
#include "r_data.h"

//
// VIDEO
//

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.
extern byte *screens[5];

extern byte *tinttab33;
extern byte *tinttab50;
extern byte *tinttabred;

#define GAMMALEVELS 9

byte gammatable[GAMMALEVELS][256];
extern int usegamma;
extern double gammalevel[GAMMALEVELS];

fixed_t DX, DY, DXI, DYI, SCALEDWIDTH, SCALEDHEIGHT, X_OFFSET, Y_OFFSET;


// Allocates buffer screens, call before R_Init.
void V_Init(void);


void V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn);

void V_FillRect(int scrn, int x, int y, int width, int height, byte color);

void V_DrawPatch(int x, int y, int scrn, patch_t *patch);
void V_DrawPatchWithShadow(int x, int y, int scrn, patch_t *patch, boolean flag);
void V_DrawPatchFlipped(int x, int y, int scrn, patch_t *patch);
void V_DrawPatchCentered(int y, int scrn, patch_t *patch);
void V_DrawFuzzPatch(int x, int y, int scrn, patch_t *patch);
void V_DrawFuzzPatchFlipped(int x, int y, int scrn, patch_t *patch);
void V_DrawPatchNoGreenWithShadow(int x, int y, int scrn, patch_t *patch);
void V_DrawTranslucentNoGreenPatch(int x, int y, int scrn, patch_t *patch);
void V_DrawTranslucentRedPatch(int x, int y, int scrn, patch_t *patch);

void V_DrawPixel(int x, int y, int screen, int color, boolean shadow);


// Draw a linear block of pixels into the view buffer.
void V_DrawBlock(int x, int y, int scrn, int width, int height, byte *src);

// Reads a linear block of pixels into the view buffer.
void V_GetBlock(int x, int y, int scrn, int width, int height, byte *dest);

boolean V_ScreenShot(void);

#endif