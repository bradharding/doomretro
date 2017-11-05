/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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

#if !defined(__P_SAVEG_H__)
#define __P_SAVEG_H__

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
    tc_elevator,        // jff 2/22/98 new elevator type thinker
    tc_scroll,          // killough 3/7/98: new scroll effect thinker
    tc_pusher,          // phares 3/22/98:  new push/pull effect thinker
    tc_fireflicker,     // killough 10/4/98
    tc_button,
    tc_endspecials
};

// temporary filename to use while saving.
char *P_TempSaveGameFile(void);

// filename to use for a savegame slot
char *P_SaveGameFile(int slot);

// Savegame file header read/write functions
dboolean P_ReadSaveGameHeader(char *description);
void P_WriteSaveGameHeader(char *description);

// Savegame end-of-file read/write functions
dboolean P_ReadSaveGameEOF(void);
void P_WriteSaveGameEOF(void);

// Persistent storage/archiving.
// These are the load / save game routines.
void P_ArchivePlayer(void);
void P_UnArchivePlayer(void);
void P_ArchiveWorld(void);
void P_UnArchiveWorld(void);
void P_ArchiveThinkers(void);
void P_UnArchiveThinkers(void);
void P_ArchiveSpecials(void);
void P_UnArchiveSpecials(void);
void P_ArchiveMap(void);
void P_UnArchiveMap(void);

void P_RestoreTargets(void);

extern FILE *save_stream;

#endif
