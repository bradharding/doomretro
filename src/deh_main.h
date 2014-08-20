/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

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

#ifndef DEH_MAIN_H
#define DEH_MAIN_H

#include "doomtype.h"
#include "deh_str.h"
#include "sha1.h"

// These are the limits that dehacked uses (from dheinit.h in the dehacked
// source).  If these limits are exceeded, it does not generate an error, but
// a warning is displayed.
#define DEH_VANILLA_NUMSTATES   966
#define DEH_VANILLA_NUMSFX      107

void DEH_Init(void);
int DEH_LoadFile(char *filename);
int DEH_LoadLump(int lumpnum);
int DEH_LoadLumpByName(char *name);

boolean DEH_ParseAssignment(char *line, char **variable_name, char **value);

void DEH_Checksum(sha1_digest_t digest);

extern boolean deh_allow_long_strings;
extern boolean deh_allow_long_cheats;
extern boolean deh_apply_cheats;

#endif
