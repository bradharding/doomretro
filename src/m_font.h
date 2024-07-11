//
// SLADE - It's a Doom Editor
// Copyright(C) 2008 - 2019 Simon Judd
// Copyright(C) 2024 Roman Fomin
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// DESCRIPTION:
//     Load and draw ZDoom FON2 fonts

#ifndef MN_FONT
#define MN_FONT

#include "doomtype.h"

bool MN_LoadFon2(const byte *gfx_data, int size);

bool MN_DrawFon2String(int x, int y, const char *str);

int MN_GetFon2PixelWidth(const char *str);

#endif
