/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is part of DOOM Retro.

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
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__V_VIDEO_H__)
#define __V_VIDEO_H__

#include "r_data.h"

//
// VIDEO
//

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.
extern byte *screens[5];

extern byte *tinttab20;
extern byte *tinttab25;
extern byte *tinttab33;
extern byte *tinttab40;
extern byte *tinttab50;
extern byte *tinttab60;
extern byte *tinttab66;
extern byte *tinttab75;
extern byte *tinttabred;

// Allocates buffer screens, call before R_Init.
void V_Init(void);

void V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn);

void V_FillRect(int scrn, int x, int y, int width, int height, int color, dboolean right);
void V_FillTransRect(int scrn, int x, int y, int width, int height, int color, dboolean right);

void V_DrawPatch(int x, int y, int scrn, patch_t *patch);
void V_DrawBigPatch(int x, int y, int scrn, patch_t *patch);
void V_DrawConsoleTextPatch(int x, int y, patch_t *patch, int color, int backgroundcolor, dboolean italics, byte *tinttab);
void V_DrawConsolePatch(int x, int y, patch_t *patch);
void V_DrawShadowPatch(int x, int y, patch_t *patch);
void V_DrawSolidShadowPatch(int x, int y, patch_t *patch);
void V_DrawSpectreShadowPatch(int x, int y, patch_t *patch);
void V_DrawSolidSpectreShadowPatch(int x, int y, patch_t *patch);
dboolean V_EmptyPatch(patch_t *patch);
void V_DrawPatchWithShadow(int x, int y, patch_t *patch, dboolean flag);
void V_DrawFlippedPatch(int x, int y, patch_t *patch);
void V_DrawFlippedShadowPatch(int x, int y, patch_t *patch);
void V_DrawFlippedSolidShadowPatch(int x, int y, patch_t *patch);
void V_DrawFlippedSpectreShadowPatch(int x, int y, patch_t *patch);
void V_DrawFlippedSolidSpectreShadowPatch(int x, int y, patch_t *patch);
void V_DrawFuzzPatch(int x, int y, patch_t *patch);
void V_DrawFlippedFuzzPatch(int x, int y, patch_t *patch);
void V_DrawNoGreenPatchWithShadow(int x, int y, patch_t *patch);
void V_DrawHUDPatch(int x, int y, patch_t *patch, byte *tinttab);
void V_DrawYellowHUDPatch(int x, int y, patch_t *patch, byte *tinttab);
void V_DrawHighlightedHUDNumberPatch(int x, int y, patch_t *patch, byte *tinttab);
void V_DrawTranslucentHUDPatch(int x, int y, patch_t *patch, byte *tinttab);
void V_DrawTranslucentHUDNumberPatch(int x, int y, patch_t *patch, byte *tinttab);
void V_DrawTranslucentYellowHUDPatch(int x, int y, patch_t *patch, byte *tinttab);
void V_DrawAltHUDPatch(int x, int y, patch_t *patch, int from, int to);
void V_DrawTranslucentAltHUDPatch(int x, int y, patch_t *patch, int from, int to);
void V_DrawTranslucentNoGreenPatch(int x, int y, patch_t *patch);
void V_DrawTranslucentRedPatch(int x, int y, patch_t *patch);
void V_DrawFlippedTranslucentRedPatch(int x, int y, patch_t *patch);
void V_DrawPatchToTempScreen(int x, int y, patch_t *patch);
void V_DrawBigPatchToTempScreen(int x, int y, patch_t *patch);
void V_DrawAltHUDText(int x, int y, patch_t *patch, int color);
void V_DrawTranslucentAltHUDText(int x, int y, patch_t *patch, int color);
void V_DrawPagePatch(patch_t *patch);

void V_DrawPixel(int x, int y, byte color, dboolean shadow);

void GetPixelSize(dboolean reset);
void V_LowGraphicDetail(void);
void V_InvertScreen(void);

dboolean V_ScreenShot(void);

#endif
