//
//  Copyright(C) 2021 Roman Fomin
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
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//      DSDHacked support

#ifndef __DSDHACKED__
#define __DSDHACKED__

#include "info.h"

void dsdh_InitTables(void);
void dsdh_FreeTables(void);

void dsdh_EnsureStatesCapacity(int limit);
void dsdh_EnsureSFXCapacity(int limit);
void dsdh_EnsureMobjInfoCapacity(int limit);
int dsdh_GetDehSpriteIndex(const char* key);
int dsdh_GetOriginalSpriteIndex(const char* key);
int dsdh_GetDehSFXIndex(const char* key, size_t length);
int dsdh_GetOriginalSFXIndex(const char* key);

#endif
