/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "r_data.h"
#include "w_file.h"

//
// VIDEO
//

#define DX          ((NONWIDEWIDTH << FRACBITS) / VANILLAWIDTH)
#define DXI         ((VANILLAWIDTH << FRACBITS) / NONWIDEWIDTH)
#define DY          ((SCREENHEIGHT << FRACBITS) / VANILLAHEIGHT)
#define DYI         ((VANILLAHEIGHT << FRACBITS) / SCREENHEIGHT)

#define NUMSCREENS  4

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.
extern byte *screens[NUMSCREENS];

extern int  lowpixelwidth;
extern int  lowpixelheight;

extern void (*postprocessfunc)(int, int, int, int, int, int);

enum
{
    CR_RED,
    CR_GRAY,
    CR_GREEN,
    CR_BLUE,
    CR_YELLOW,
    CR_BLACK,
    CR_PURPLE,
    CR_WHITE,
    CR_ORANGE,
    CR_LIMIT
};

extern byte *colortranslation[CR_LIMIT];
extern byte *redtogold;

void V_InitColorTranslation(void);

// Allocates buffer screens, call before R_Init.
void V_Init(void);

void V_FillRect(int screen, int x, int y, int width, int height, int color1, int color2,
    bool left, bool right, const byte *tinttab1, const byte *tinttab2);
void V_FillTransRect(int screen, int x, int y, int width, int height, int color1, int color2,
    bool left, bool right, const byte *tinttab1, const byte *tinttab2);
void V_FillSoftTransRect(int screen, int x, int y, int width, int height, int color1, int color2,
    bool left, bool right, const byte *tinttab1, const byte *tinttab2);

void V_DrawPatch(int x, int y, int screen, patch_t *patch);
void V_DrawWidePatch(int x, int y, int screen, patch_t *patch);
void V_DrawBigPatch(int x, int y, short width, short height, patch_t *patch);
void V_DrawMenuBorderPatch(int x, int y, patch_t *patch);
void V_DrawConsolePatch(int x, int y, patch_t *patch, int maxwidth);
void V_DrawConsoleBrandingPatch(int x, int y, patch_t *patch);
void V_DrawConsoleSelectedTextPatch(const int x, const int y, const patch_t *patch, const int width,
    const int color, const int backgroundcolor, const bool italics, const byte *tinttab);
void V_DrawConsoleTextPatch(const int x, const int y, const patch_t *patch, const int width,
    const int color, const int backgroundcolor, const bool italics, const byte *tinttab);
void V_DrawOverlayTextPatch(byte *screen, int screenwidth, int x, int y, patch_t *patch,
    int width, int color, const byte *tinttab);
void V_DrawShadowPatch(int x, int y, patch_t *patch);
void V_DrawSolidShadowPatch(int x, int y, patch_t *patch);
void V_DrawSpectreShadowPatch(int x, int y, patch_t *patch);
bool V_IsEmptyPatch(patch_t *patch);
void V_DrawMenuPatch(int x, int y, patch_t* patch, bool shadow, bool highlight, bool darken);
void V_DrawFlippedPatch(int x, int y, patch_t *patch);
void V_DrawFlippedShadowPatch(int x, int y, patch_t *patch);
void V_DrawFlippedSolidShadowPatch(int x, int y, patch_t *patch);
void V_DrawFlippedSpectreShadowPatch(int x, int y, patch_t *patch);
void V_DrawFuzzPatch(int x, int y, patch_t *patch);
void V_DrawFlippedFuzzPatch(int x, int y, patch_t *patch);
void V_DrawNoGreenPatchWithShadow(int x, int y, patch_t *patch);
void V_DrawHUDPatch(int x, int y, patch_t *patch, const byte *tinttab);
void V_DrawHighlightedHUDNumberPatch(int x, int y, patch_t *patch, const byte *tinttab);
void V_DrawTranslucentHUDPatch(int x, int y, patch_t *patch, const byte *tinttab);
void V_DrawTranslucentHUDNumberPatch(int x, int y, patch_t *patch, const byte *tinttab);
void V_DrawAltHUDPatch(int x, int y, patch_t *patch, int from, int to, const byte *tinttab);
void V_DrawTranslucentAltHUDPatch(int x, int y, patch_t *patch, int from, int to, const byte *tinttab);
void V_DrawTranslucentNoGreenPatch(int x, int y, patch_t *patch);
void V_DrawTranslucentRedPatch(int x, int y, patch_t *patch);
void V_DrawFlippedTranslucentRedPatch(int x, int y, patch_t *patch);
void V_DrawPatchToTempScreen(int x, int y, patch_t *patch);
void V_DrawHUDText(int x, int y, byte *screen, patch_t *patch, int screenwidth);
void V_DrawTranslucentHUDText(int x, int y, byte *screen, patch_t *patch, int screenwidth);
void V_DrawAltHUDText(int x, int y, byte *screen, patch_t *patch,
    bool italics, int color, int screenwidth, const byte *tinttab);
void V_DrawTranslucentAltHUDText(int x, int y, byte *screen, patch_t *patch,
    bool italics, int color, int screenwidth, const byte *tinttab);
void V_DrawPagePatch(int screen, patch_t *patch);

void V_DrawPixel(int x, int y, byte color, bool highlight, bool shadow, bool darken);

void GetPixelSize(void);
void V_InvertScreen(void);

bool V_ScreenShot(void);
