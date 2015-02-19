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

#if !defined(__V_VIDEO__)
#define __V_VIDEO__

#include "r_data.h"

//
// VIDEO
//

fixed_t DX, DY, DXI, DYI;

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.
extern byte     *screens[5];

extern byte     redtoyellow[];

extern byte     *tinttab25;
extern byte     *tinttab33;
extern byte     *tinttab40;
extern byte     *tinttab50;
extern byte     *tinttab75;
extern byte     *tinttabred;

// Allocates buffer screens, call before R_Init.
void V_Init(void);

void V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn);

void V_FillRect(int scrn, int x, int y, int width, int height, byte color);

void V_DrawPatch(int x, int y, int scrn, patch_t *patch);
void V_DrawTranslucentPatch(int x, int y, int scrn, patch_t *patch);
void V_DrawBigPatch(int x, int y, int scrn, patch_t *patch);
void V_DrawConsoleChar(int x, int y, patch_t *patch, byte color);
void V_DrawTranslucentConsolePatch(int x, int y, patch_t *patch);
void V_DrawShadowPatch(int x, int y, patch_t *patch);
void V_DrawSolidShadowPatch(int x, int y, patch_t *patch);
void V_DrawSpectreShadowPatch(int x, int y, patch_t *patch);
boolean V_EmptyPatch(patch_t *patch);
void V_DrawPatchWithShadow(int x, int y, patch_t *patch, boolean flag);
void V_DrawFlippedPatch(int x, int y, patch_t *patch);
void V_DrawFlippedShadowPatch(int x, int y, patch_t *patch);
void V_DrawFlippedSolidShadowPatch(int x, int y, patch_t *patch);
void V_DrawFlippedSpectreShadowPatch(int x, int y, patch_t *patch);
void V_DrawCenteredPatch(int y, patch_t *patch);
void V_DrawFuzzPatch(int x, int y, patch_t *patch);
void V_DrawFlippedFuzzPatch(int x, int y, patch_t *patch);
void V_DrawNoGreenPatchWithShadow(int x, int y, patch_t *patch);
void V_DrawHUDPatch(int x, int y, patch_t *patch, boolean invert);
void V_DrawYellowHUDPatch(int x, int y, patch_t *patch, boolean invert);
void V_DrawTranslucentHUDPatch(int x, int y, patch_t *patch, boolean invert);
void V_DrawTranslucentHUDNumberPatch(int x, int y, patch_t *patch, boolean invert);
void V_DrawTranslucentYellowHUDPatch(int x, int y, patch_t *patch, boolean invert);
void V_DrawTranslucentNoGreenPatch(int x, int y, patch_t *patch);
void V_DrawTranslucentRedPatch(int x, int y, patch_t *patch);
void V_DrawFlippedTranslucentRedPatch(int x, int y, patch_t *patch);
void V_DrawPatchToTempScreen(int x, int y, patch_t *patch);

void V_DrawPixel(int x, int y, byte color, boolean shadow);

void V_LowGraphicDetail(int height);

// Draw a linear block of pixels into the view buffer.
void V_DrawBlock(int x, int y, int width, int height, byte *src);

boolean V_ScreenShot(void);

#endif
