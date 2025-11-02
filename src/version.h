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

#define DOOMRETRO_VERSION               5,8,1,0
#define DOOMRETRO_VERSIONSTRING         "5.8.1"
#define DOOMRETRO_NAMEANDVERSIONSTRING  "DOOM Retro v5.8.1"

#define DOOMRETRO_VERSIONSTRING_5_7_2   "5.7.2"

#define DOOMRETRO_SAVEGAMEVERSION_3_6   "DOOM Retro v3.6"
#define DOOMRETRO_SAVEGAMEVERSION_5_7   "DOOM Retro v5.7"
#define DOOMRETRO_SAVEGAMEVERSION_5_7_1 "DOOM Retro v5.7.1"
#define DOOMRETRO_SAVEGAMEVERSION_5_7_2 "DOOM Retro v5.7.2"
#define DOOMRETRO_SAVEGAMEVERSIONSTRING DOOMRETRO_SAVEGAMEVERSION_5_7_2

#define DOOMRETRO                       "doomretro"
#define DOOMRETRO_AUTOLOADFOLDER        "autoload"
#define DOOMRETRO_BLOGURL               "www.doomretro.com"
#define DOOMRETRO_CONFIGFILE            "doomretro.cfg"
#define DOOMRETRO_CONSOLEFOLDER         "console"
#define DOOMRETRO_COPYRIGHT             "Copyright \xA9 2013\x962025 by Brad Harding. All rights reserved."
#define DOOMRETRO_CREATOR               "Brad Harding"
#define DOOMRETRO_CREATORANDEMAIL       "Brad Harding (brad@doomretro.com)"
#define DOOMRETRO_FILENAME              "doomretro.exe"
#define DOOMRETRO_HOMEOFCREATOR         "Western Sydney, Australia"
#define DOOMRETRO_ICONPATH              "..\\res\\doomretro.ico"
#define DOOMRETRO_LATESTRELEASEPATH     L"/repos/bradharding/doomretro/releases/latest"
#define DOOMRETRO_LICENSE               "GNU General Public License v3.0"
#define DOOMRETRO_LICENSEURL            "https://github.com/bradharding/doomretro/wiki/License"
#define DOOMRETRO_MUTEX                 "DOOMRETRO-CC4F1071-8B24-4E91-A207-D792F39636CD"
#define DOOMRETRO_NAME                  "DOOM Retro"
#define DOOMRETRO_RELEASENOTESURL       "https://github.com/bradharding/doomretro/releases/tag/v" \
                                        DOOMRETRO_VERSIONSTRING
#define DOOMRETRO_RESOURCEWAD           "doomretro.wad"
#define DOOMRETRO_SAVEGAME              "doomretro%i.save"
#define DOOMRETRO_SAVEGAMESFOLDER       "savegames"
#define DOOMRETRO_SCREENSHOTSFOLDER     "screenshots"
#define DOOMRETRO_TRADEMARKS            "DOOM is a registered trademark of id Software LLC, a ZeniMax " \
                                        "Media company, in the US and/or other countries, and is used " \
                                        "without permission. All other trademarks are the property of " \
                                        "their respective holders. DOOM Retro is in no way affiliated " \
                                        "with nor endorsed by id Software."
#define DOOMRETRO_WIKINAME              "DOOM Retro Wiki"
#define DOOMRETRO_WIKIURL               "https://github.com/bradharding/doomretro/wiki"

#if defined(_WIN32)
void D_CheckForNewVersion(void);
void D_OpenURLInBrowser(const char *url, const char *warning);
#endif
