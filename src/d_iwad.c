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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "c_console.h"
#include "d_deh.h"
#include "d_iwad.h"
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

// Array of locations to search for IWAD files
//
// "128 IWAD search directories should be enough for anybody".
#define MAX_IWAD_DIRS   128

static dboolean iwad_dirs_built;
static char     *iwad_dirs[MAX_IWAD_DIRS];
static int      num_iwad_dirs;

static void AddIWADDir(char *dir)
{
    if (num_iwad_dirs < MAX_IWAD_DIRS)
    {
        iwad_dirs[num_iwad_dirs] = dir;
        ++num_iwad_dirs;
    }
}

#if defined(WIN32)
// This is Windows-specific code that automatically finds the location
// of installed IWAD files. The registry is inspected to find special
// keys installed by the Windows installers for various CD versions
// of DOOM. From these keys we can deduce where to find an IWAD.

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

typedef struct
{
    HKEY        root;
    char        *path;
    char        *value;
} registry_value_t;

#define UNINSTALLER_STRING "\\uninstl.exe /S "

// Keys installed by the various CD editions. These are actually the
// commands to invoke the uninstaller and look like this:
//
// C:\Program Files\Path\uninstl.exe /S C:\Program Files\Path
//
// With some munging we can find where DOOM was installed.

// [AlexMax] From the perspective of a 64-bit executable, 32-bit registry
// keys are located in a different spot.
#if _WIN64
#define SOFTWARE_KEY "Software\\Wow6432Node"
#else
#define SOFTWARE_KEY "Software"
#endif

static registry_value_t uninstall_values[] =
{
    // Ultimate DOOM, CD version (Depths of DOOM trilogy)
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Ultimate Doom for Windows 95",
        "UninstallString",
    },

    // DOOM II, CD version (Depths of DOOM trilogy)
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Doom II for Windows 95",
        "UninstallString",
    },

    // Final DOOM
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Final Doom for Windows 95",
        "UninstallString",
    },

    // Shareware version
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Doom Shareware for Windows 95",
        "UninstallString",
    },
};

// Values installed by the GOG.com and Collector's Edition versions

static registry_value_t root_path_keys[] =
{
    // Doom Collector's Edition
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\Activision\\DOOM Collector's Edition\\v1.0",
        "INSTALLPATH",
    },

    // Ultimate Doom

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1435827232",
        "PATH",
    },

    // Doom II

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1435848814",
        "PATH",
    },

    // Final Doom

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1435848742",
        "PATH",
    },
};

// Subdirectories of the above install path, where IWADs are installed.
static char *root_path_subdirs[] =
{
    ".",
    "Doom2",
    "Final Doom",
    "Ultimate Doom",
    "TNT",
    "Plutonia"
};

// Location where Steam is installed
static registry_value_t steam_install_location =
{
    HKEY_LOCAL_MACHINE,
    "Software\\Valve\\Steam",
    "InstallPath",
};

// Subdirs of the steam install directory where IWADs are found
static char *steam_install_subdirs[] =
{
    "steamapps\\common\\doom 2\\base",
    "steamapps\\common\\final doom\\base",
    "steamapps\\common\\ultimate doom\\base",
    "steamapps\\common\\DOOM 3 BFG Edition\\base\\wads"
};

static char *GetRegistryString(registry_value_t *reg_val)
{
    HKEY        key;
    DWORD       len;
    DWORD       valtype;
    char        *result = NULL;

    // Open the key (directory where the value is stored)
    if (RegOpenKeyEx(reg_val->root, reg_val->path, 0, KEY_READ, &key) != ERROR_SUCCESS)
        return NULL;

    // Find the type and length of the string, and only accept strings.
    if (RegQueryValueEx(key, reg_val->value, NULL, &valtype, NULL, &len) == ERROR_SUCCESS
        && valtype == REG_SZ)
    {
        // Allocate a buffer for the value and read the value
        result = (char *)malloc(len);

        if (RegQueryValueEx(key, reg_val->value, NULL, &valtype, (unsigned char *)result,
            &len) != ERROR_SUCCESS)
        {
            free(result);
            result = NULL;
        }
    }

    // Close the key
    RegCloseKey(key);

    return result;
}

// Check for the uninstall strings from the CD versions
static void CheckUninstallStrings(void)
{
    unsigned int        i;

    for (i = 0; i < arrlen(uninstall_values); ++i)
    {
        char    *val = GetRegistryString(&uninstall_values[i]);
        char    *unstr;

        if (!val)
            continue;

        unstr = strstr(val, UNINSTALLER_STRING);

        if (!unstr)
            free(val);
        else
        {
            char        *path = unstr + strlen(UNINSTALLER_STRING);

            AddIWADDir(path);
        }
    }
}

// Check for GOG.com and Doom: Collector's Edition

static void CheckInstallRootPaths(void)
{
    size_t      i;

    for (i = 0; i < arrlen(root_path_keys); ++i)
    {
        char    *install_path = GetRegistryString(&root_path_keys[i]);
        size_t  j;

        if (!install_path)
            continue;

        for (j = 0; j < arrlen(root_path_subdirs); ++j)
            AddIWADDir(M_StringJoin(install_path, DIR_SEPARATOR_S, root_path_subdirs[j], NULL));

        free(install_path);
    }
}

// Check for DOOM downloaded via Steam
static void CheckSteamEdition(void)
{
    char        *install_path = GetRegistryString(&steam_install_location);
    size_t      i;

    if (!install_path)
        return;

    for (i = 0; i < arrlen(steam_install_subdirs); ++i)
        AddIWADDir(M_StringJoin(install_path, DIR_SEPARATOR_S, steam_install_subdirs[i], NULL));

    free(install_path);
}

// Default install directories for DOS DOOM
static void CheckDOSDefaults(void)
{
    // These are the default install directories used by the deice
    // installer program:
    AddIWADDir("\\doom2");              // DOOM II
    AddIWADDir("\\plutonia");           // Final DOOM
    AddIWADDir("\\tnt");
    AddIWADDir("\\doom_se");            // Ultimate DOOM
    AddIWADDir("\\doom");               // Shareware / Registered DOOM
    AddIWADDir("\\dooms");              // Shareware versions
    AddIWADDir("\\doomsw");
}

#endif

static struct
{
    char                *name;
    GameMission_t       mission;
} iwads[] = {
    { "doom2.wad",    doom2      },
    { "doom2.wad",    pack_nerve },
    { "plutonia.wad", pack_plut  },
    { "tnt.wad",      pack_tnt   },
    { "doom.wad",     doom       },
    { "doom1.wad",    doom       },
    { "hacx.wad",     doom2      }
};

// When given an IWAD with the '-iwad' parameter,
// attempt to identify it by its name.
void IdentifyIWADByName(char *name)
{
    size_t      i;
    char        *p;

    // Trim down the name to just the filename, ignoring the path.
    p = strrchr(name, '\\');
    if (!p)
        p = strrchr(name, '/');
    if (p)
        name = p + 1;
    gamemission = none;

    for (i = 0; i < arrlen(iwads); ++i)
    {
        // Check if the filename is this IWAD name.
        if (M_StringCompare(name, iwads[i].name))
        {
            gamemission = iwads[i].mission;
            break;
        }
    }
}

//
// Add directories from the list in the DOOMWADPATH environment variable.
//
static void AddDoomWadPath(void)
{
    char        *doomwadpath = getenv("DOOMWADPATH");
    char        *p;

    if (!doomwadpath)
        return;

    // Add the initial directory
    AddIWADDir(doomwadpath);

    // Split into individual dirs within the list.
    p = doomwadpath;

    for (;;)
    {
        p = strchr(p, PATH_SEPARATOR);

        if (p)
        {
            // Break at the separator and store the right hand side
            // as another iwad dir
            *p = '\0';
            p += 1;

            AddIWADDir(p);
        }
        else
            break;
    }
}

//
// Build a list of IWAD files
//
static void BuildIWADDirList(void)
{
    char        *doomwaddir;

    if (iwad_dirs_built)
        return;

    // Look in the current directory. DOOM always does this.
    AddIWADDir(".");

    // Add DOOMWADDIR if it is in the environment
    doomwaddir = getenv("DOOMWADDIR");

    if (doomwaddir)
        AddIWADDir(doomwaddir);

    // Add dirs from DOOMWADPATH
    AddDoomWadPath();

#if defined(WIN32)
    // Search the registry and find where IWADs have been installed.
    CheckUninstallStrings();
    CheckInstallRootPaths();
    CheckSteamEdition();
    CheckDOSDefaults();
#endif

    // Don't run this function again.
    iwad_dirs_built = true;
}

//
// Searches WAD search paths for an WAD with a specific filename.
//
char *D_FindWADByName(char *name)
{
    int         i;

    // Absolute path?
    if (M_FileExists(name))
        return name;

    BuildIWADDirList();

    // Search through all IWAD paths for a file with the given name.
    for (i = 0; i < num_iwad_dirs; ++i)
    {
        char        *path;

        // As a special case, if this is in DOOMWADDIR or DOOMWADPATH,
        // the "directory" may actually refer directly to an IWAD
        // file.
        if (M_StringCompare(leafname(iwad_dirs[i]), name) && M_FileExists(iwad_dirs[i]))
            return strdup(iwad_dirs[i]);

        // Construct a string for the full path
        path = M_StringJoin(iwad_dirs[i], DIR_SEPARATOR_S, name, NULL);

        if (M_FileExists(path))
            return path;

        free(path);
    }

    // File not found
    return NULL;
}

//
// D_TryWADByName
//
// Searches for a WAD by its filename, or passes through the filename
// if not found.
//
char *D_TryFindWADByName(char *filename)
{
    char        *result = D_FindWADByName(filename);

    return (result ? result : filename);
}

//
// FindIWAD
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWADs).
//
char *D_FindIWAD(void)
{
    char        *result = NULL;
    int         iwadparm = M_CheckParmWithArgs("-iwad", 1, 1);

    if (iwadparm)
    {
        char        *iwadfile;

        // Search through IWAD dirs for an IWAD with the given name.
        iwadfile = myargv[iwadparm + 1];

        result = D_FindWADByName(iwadfile);

        if (!result)
            I_Error("The IWAD file \"%s\" wasn't found!", iwadfile);

        IdentifyIWADByName(result);
    }

    return result;
}

//
// Get the IWAD name used for savegames.
//
static char *SaveGameIWADName(void)
{
    size_t      i;

    // Find what subdirectory to use for savegames
    //
    // The directory depends on the IWAD, so that savegames for
    // different IWADs are kept separate.
    //
    // Note that we match on gamemission rather than on IWAD name.
    // This ensures that doom1.wad and doom.wad saves are stored
    // in the same place.
    for (i = 0; i < arrlen(iwads); ++i)
        if (gamemission == iwads[i].mission)
            return iwads[i].name;

    return NULL;
}

extern char     *pwadfile;

//
// SetSaveGameFolder
//
// Chooses the directory used to store saved games.
//
void D_SetSaveGameFolder(void)
{
    char *iwad_name = SaveGameIWADName();

    if (!iwad_name)
        iwad_name = "unknown.wad";

    savegamefolder = M_StringJoin(M_GetExecutableFolder(), DIR_SEPARATOR_S, "savegames",
        DIR_SEPARATOR_S, NULL);
    M_MakeDirectory(savegamefolder);

    savegamefolder = M_StringJoin(savegamefolder, (pwadfile[0] ? pwadfile : iwad_name),
        DIR_SEPARATOR_S, NULL);
    M_MakeDirectory(savegamefolder);

    C_Output("Savegames will be saved and loaded in %s.", uppercase(savegamefolder));
}

//
// Find out what version of DOOM is playing.
//
void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD function. But if
    // we specify '-iwad', we have to identify using
    // IdentifyIWADByName. However, if the iwad does not match
    // any known IWAD name, we may have a dilemma. Try to
    // identify by its contents.
    if (gamemission == none)
    {
        int     i;

        for (i = 0; i < numlumps; ++i)
        {
            if (!strncasecmp(lumpinfo[i]->name, "MAP01", 8))
            {
                gamemission = doom2;
                break;
            }
            else if (!strncasecmp(lumpinfo[i]->name, "E1M1", 8))
            {
                gamemission = doom;
                break;
            }
        }

        if (gamemission == none)
            // Still no idea. I don't think this is going to work.
            I_Error("Unknown or invalid IWAD file.");
    }

    // Make sure gamemode is set up correctly
    if (gamemission == doom)
    {
        // DOOM 1. But which version?
        if (W_CheckNumForName("E4M1") > 0)
            // Ultimate DOOM
            gamemode = retail;
        else if (W_CheckNumForName("E3M1") > 0)
            gamemode = registered;
        else
            gamemode = shareware;
    }
    else
        // DOOM 2 of some kind.
        gamemode = commercial;
}

// Set the gamedescription string
void D_SetGameDescription(void)
{
    gamedescription = (char *)malloc(64);
    gamedescription = PACKAGE_NAME;

    if (chex)
        gamedescription = s_CAPTION_CHEX;
    else if (hacx)
        gamedescription = s_CAPTION_HACX;
    else if (BTSXE1)
        gamedescription = s_CAPTION_BTSXE1;
    else if (BTSXE2)
        gamedescription = s_CAPTION_BTSXE2;
    else if (BTSXE3)
        gamedescription = s_CAPTION_BTSXE3;
    else if (gamemission == doom)
    {
        // DOOM 1. But which version?
        if (FREEDOOM)
            gamedescription = s_CAPTION_FREEDOOM1;
        else if (W_CheckMultipleLumps("TITLEPIC") > 1)
            gamedescription = uppercase(leafname(
                lumpinfo[W_GetNumForName("TITLEPIC")]->wad_file->path));
        else if (W_CheckMultipleLumps("M_DOOM") > 1)
            gamedescription = uppercase(leafname(
                lumpinfo[W_GetNumForName("M_DOOM")]->wad_file->path));
        else if (gamemode == retail)
            gamedescription = s_CAPTION_ULTIMATE;
        else if (gamemode == registered)
            gamedescription = s_CAPTION_REGISTERED;
        else if (gamemode == shareware)
            gamedescription = s_CAPTION_SHAREWARE;
    }
    else
    {
        // DOOM 2 of some kind. But which mission?
        if (FREEDOOM)
        {
            if (FREEDM)
                gamedescription = s_CAPTION_FREEDM;
            else
                gamedescription = s_CAPTION_FREEDOOM2;
        }
        else if (nerve)
            gamedescription = s_CAPTION_DOOM2;
        else if (W_CheckMultipleLumps("TITLEPIC") > 1)
            gamedescription = uppercase(leafname(
                lumpinfo[W_GetNumForName("TITLEPIC")]->wad_file->path));
        else if (W_CheckMultipleLumps("M_DOOM") > 1)
            gamedescription = uppercase(leafname(
                lumpinfo[W_GetNumForName("M_DOOM")]->wad_file->path));
        else if (gamemission == doom2)
            gamedescription = M_StringJoin(s_CAPTION_DOOM2, ": ", s_CAPTION_HELLONEARTH, NULL);
        else if (gamemission == pack_plut)
            gamedescription = s_CAPTION_PLUTONIA;
        else if (gamemission == pack_tnt)
            gamedescription = s_CAPTION_TNT;
    }

    if (nerve)
    {
        if (bfgedition)
            C_Output("Playing \"%s: %s (%s)\" and \"%s: %s (%s)\".",
                s_CAPTION_DOOM2, s_CAPTION_HELLONEARTH, s_CAPTION_BFGEDITION,
                s_CAPTION_DOOM2, s_CAPTION_NERVE, s_CAPTION_BFGEDITION);
        else
            C_Output("Playing \"%s: %s\" and \"%s: %s\".",
                s_CAPTION_DOOM2, s_CAPTION_HELLONEARTH, s_CAPTION_DOOM2, s_CAPTION_NERVE);
    }
    else
    {
        if (bfgedition)
            C_Output("Playing \"%s (%s)\".", gamedescription, s_CAPTION_BFGEDITION);
        else
            C_Output("Playing \"%s\".", gamedescription);
    }
}
