/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __R_DRAW__
#define __R_DRAW__

extern lighttable_t     *dc_colormap;
extern int              dc_x;
extern int              dc_yl;
extern int              dc_yh;
extern fixed_t          dc_iscale;
extern fixed_t          dc_texturemid;
extern fixed_t          dc_texheight;
extern fixed_t          dc_texturefrac;
extern boolean          dc_topsparkle;
extern boolean          dc_bottomsparkle;
extern fixed_t          dc_blood;

// first pixel in a column
extern byte             *dc_source;

extern byte             *tinttab;
extern byte             *tinttab33;
extern byte             *tinttab40;
extern byte             *tinttab50;
extern byte             *tinttab66;
extern byte             *tinttabred;
extern byte             *tinttabredwhite;
extern byte             *tinttabgreen;
extern byte             *tinttabblue;
extern byte             *tinttabred50;
extern byte             *tinttabredwhite50;
extern byte             *tinttabgreen50;
extern byte             *tinttabblue50;

// The span blitting interface.
// Hook in assembler or system specific BLT
//  here.
void R_DrawColumn(void);
void R_DrawWallColumn(void);
void R_DrawFullbrightWallColumn(byte *);
void R_DrawSkyColumn(void);
void R_DrawFlippedSkyColumn(void);
void R_DrawTranslucentColumn(void);
void R_DrawTranslucent50Column(void);
void R_DrawTranslucent33Column(void);
void R_DrawTranslucentGreenColumn(void);
void R_DrawTranslucentRedColumn(void);
void R_DrawTranslucentRedWhiteColumn(void);
void R_DrawTranslucentRedWhite50Column(void);
void R_DrawTranslucentBlueColumn(void);
void R_DrawTranslucentGreen50Column(void);
void R_DrawTranslucentRed50Column(void);
void R_DrawTranslucentBlue50Column(void);
void R_DrawRedToBlueColumn(void);
void R_DrawTranslucentRedToBlue33Column(void);
void R_DrawRedToGreenColumn(void);
void R_DrawTranslucentRedToGreen33Column(void);
void R_DrawPlayerSpriteColumn(void);
void R_DrawSuperShotgunColumn(void);
void R_DrawShadowColumn(void);
void R_DrawBloodSplatColumn(void);
void R_DrawMegaSphereColumn(void);

// The Spectre/Invisibility effect.
void R_DrawFuzzColumn(void);
void R_DrawFuzzColumns(void);

void R_VideoErase(unsigned int ofs, int count);

extern int              ds_y;
extern int              ds_x1;
extern int              ds_x2;

extern lighttable_t     *ds_colormap;

extern fixed_t          ds_xfrac;
extern fixed_t          ds_yfrac;
extern fixed_t          ds_xstep;
extern fixed_t          ds_ystep;

// start of a 64*64 tile image
extern byte             *ds_source;

// Span blitting for rows, floor/ceiling.
// No Spectre effect needed.
void R_DrawSpan(void);

void R_InitBuffer(int width, int height);

// Rendering function.
void R_FillBackScreen(void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder(void);

#endif
