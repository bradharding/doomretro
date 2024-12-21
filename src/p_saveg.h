/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

// maximum size of a savegame description
#define SAVESTRINGSIZE          256

#define SAVESTRINGPIXELWIDTH    186
#define VERSIONSIZE             24

enum
{
    tc_end,
    tc_mobj,
    tc_bloodsplat
};

enum
{
    tc_ceiling,
    tc_door,
    tc_floor,
    tc_plat,
    tc_flash,
    tc_strobe,
    tc_glow,
    tc_elevator,    // jff 02/22/98 new elevator type thinker
    tc_scroll,      // killough 03/07/98: new scroll effect thinker
    tc_pusher,      // phares 03/22/98: new push/pull effect thinker
    tc_fireflicker, // killough 10/04/98
    tc_button,
    tc_endspecials
};

// temporary filename to use while saving
char *P_TempSaveGameFile(void);

// filename to use for a savegame slot
char *P_SaveGameFile(int slot);

// Savegame file header read/write functions
bool P_ReadSaveGameHeader(char *description);
void P_WriteSaveGameHeader(const char *description);

// Savegame end-of-file read/write functions
bool P_ReadSaveGameEOF(void);
void P_WriteSaveGameEOF(void);

// Savegame file footer read/write functions
void P_ReadSaveGameFooter(void);
void P_WriteSaveGameFooter(void);

// Persistent storage/archiving.
// These are the load/save game routines.
void P_ArchivePlayer(void);
void P_UnarchivePlayer(void);
void P_ArchiveWorld(void);
void P_UnarchiveWorld(void);
void P_ArchiveThinkers(void);
void P_UnarchiveThinkers(void);
void P_ArchiveSpecials(void);
void P_UnarchiveSpecials(void);
void P_ArchiveMap(void);
void P_UnarchiveMap(void);

void P_RestoreTargets(void);

extern FILE *save_stream;
