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

#ifndef __P_SAVEG__
#define __P_SAVEG__

#include <stdio.h>

// maximum size of a savegame description
#define SAVESTRINGSIZE          256
#define SAVESTRINGPIXELWIDTH    185
#define VERSIONSIZE             16

// temporary filename to use while saving.
char *P_TempSaveGameFile(void);

// filename to use for a savegame slot
char *P_SaveGameFile(int slot);

// Savegame file header read/write functions
boolean P_ReadSaveGameHeader(void);
void P_WriteSaveGameHeader(char *description);

// Savegame end-of-file read/write functions
boolean P_ReadSaveGameEOF(void);
void P_WriteSaveGameEOF(void);

// Persistent storage/archiving.
// These are the load / save game routines.
void P_ArchivePlayers(void);
void P_UnArchivePlayers(void);
void P_ArchiveWorld(void);
void P_UnArchiveWorld(void);
void P_ArchiveThinkers(void);
void P_UnArchiveThinkers(void);
void P_ArchiveSpecials(void);
void P_UnArchiveSpecials(void);

uint32_t P_ThinkerToIndex(thinker_t *thinker);
thinker_t *P_IndexToThinker(uint32_t index);
void P_RestoreTargets(void);

extern FILE *save_stream;
extern boolean savegame_error;

#endif
