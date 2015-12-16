/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__VERSION__)
#define __VERSION__

#define PACKAGE_VERSION                 2,0,0,0
#define PACKAGE_VERSIONSTRING           "2.0"
#if defined (_DEBUG)
#define PACKAGE_BRANDINGSTRING          "DOOM Retro v2.0 (Debug Build)"
#else
#define PACKAGE_BRANDINGSTRING          "DOOM Retro v2.0"
#endif
#define PACKAGE_NAMEANDVERSIONSTRING    "DOOM Retro v2.0"
#define PACKAGE_SAVEGAMEVERSIONSTRING   "DOOM Retro v2.0"

#define PACKAGE                         "doomretro"
#define PACKAGE_CONFIG                  "doomretro.cfg"
#define PACKAGE_COPYRIGHT               "� 2013-2016 Brad Harding. All rights reserved."
#define PACKAGE_EMAIL                   "brad@doomretro.com"
#define PACKAGE_ICON_PATH               "..\\res\\doomretro.ico"
#define PACKAGE_MUTEX                   "DOOMRETRO-CC4F1071-8B24-4E91-A207-D792F39636CD"
#define PACKAGE_NAME                    "DOOM Retro"
#define PACKAGE_SAVE                    "doomretro%d.save"
#define PACKAGE_WAD                     "doomretro.wad"
#define PACKAGE_WIKI_URL                "http://wiki.doomretro.com/Starting-a-Game"

#endif
