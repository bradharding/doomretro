/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

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
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__SC_MAN_H__)
#define __SC_MAN_H__

extern char     *sc_String;
extern int      sc_Number;
extern int      sc_Line;
extern dboolean sc_End;
extern dboolean sc_FileScripts;
extern char     *sc_ScriptsDir;

void SC_Open(char *name);
void SC_Close(void);
dboolean SC_GetString(void);
void SC_MustGetString(void);
dboolean SC_GetNumber(void);
void SC_MustGetNumber(void);
void SC_UnGet(void);
dboolean SC_Compare(char *text);
int SC_MatchString(char **strings);
void SC_ScriptError(char *message);

#endif
