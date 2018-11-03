/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "version.h"
#include "w_wad.h"

// Array of locations to search for IWAD files
//
// "128 IWAD search directories should be enough for anybody".
#define MAX_IWAD_DIRS   128

static char *iwad_dirs[MAX_IWAD_DIRS];
static int  num_iwad_dirs;

static void AddIWADDir(char *dir)
{
    if (num_iwad_dirs < MAX_IWAD_DIRS)
        iwad_dirs[num_iwad_dirs++] = dir;
}

#if defined(_WIN32)
// This is Windows-specific code that automatically finds the location
// of installed IWAD files. The registry is inspected to find special
// keys installed by the Windows installers for various CD versions
// of DOOM. From these keys we can deduce where to find an IWAD.
typedef struct
{
    HKEY    root;
    char    *path;
    char    *value;
} registryvalue_t;

#define UNINSTALLER_STRING  "\\uninstl.exe /S "

// Keys installed by the various CD editions. These are actually the
// commands to invoke the uninstaller and look like this:
//
// C:\Program Files\Path\uninstl.exe /S C:\Program Files\Path
//
// With some munging we can find where DOOM was installed.
static registryvalue_t uninstall_values[] =
{
    // Ultimate DOOM, CD version (Depths of DOOM trilogy)
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ultimate Doom for Windows 95",
        "UninstallString"
    },

    // DOOM II, CD version (Depths of DOOM trilogy)
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Doom II for Windows 95",
        "UninstallString"
    },

    // Final DOOM
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Final Doom for Windows 95",
        "UninstallString"
    },

    // Shareware version
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Doom Shareware for Windows 95",
        "UninstallString"
    },
};

// Values installed by the GOG.com and Collector's Edition versions
static registryvalue_t root_path_keys[] =
{
    // DOOM Collector's Edition
    {
        HKEY_LOCAL_MACHINE,
        "Software\\Activision\\DOOM Collector's Edition\\v1.0",
        "INSTALLPATH"
    },

    // DOOM II
    {
        HKEY_LOCAL_MACHINE,
        "Software\\GOG.com\\Games\\1435848814",
        "PATH"
    },

    // DOOM 3: BFG Edition
    {
        HKEY_LOCAL_MACHINE,
        "Software\\GOG.com\\Games\\1135892318",
        "PATH"
    },

    // Final DOOM
    {
        HKEY_LOCAL_MACHINE,
        "Software\\GOG.com\\Games\\1435848742",
        "PATH"
    },

    // Ultimate DOOM
    {
        HKEY_LOCAL_MACHINE,
        "Software\\GOG.com\\Games\\1435827232",
        "PATH"
    }
};

// Subdirectories of the above install path, where IWADs are installed.
static const char *root_path_subdirs[] =
{
    ".",
    "Doom2",
    "Final Doom",
    "Ultimate Doom",
    "TNT",
    "Plutonia",
    "base\\wads"
};

// Location where Steam is installed
static registryvalue_t steam_install_location =
{
    HKEY_CURRENT_USER,
    "Software\\Valve\\Steam",
    "SteamPath"
};

// Subdirs of the steam install directory where IWADs are found
static const char *steam_install_subdirs[] =
{
    "steamapps\\common\\doom 2\\base",
    "steamapps\\common\\DOOM 3 BFG Edition\\base\\wads",
    "steamapps\\common\\final doom\\base",
    "steamapps\\common\\ultimate doom\\base"
};

static char *GetRegistryString(registryvalue_t *reg_val)
{
    HKEY    key;
    DWORD   len;
    DWORD   valtype;
    char    *result = NULL;

    // Open the key (directory where the value is stored)
    if (RegOpenKeyEx(reg_val->root, reg_val->path, 0, KEY_READ, &key) != ERROR_SUCCESS)
        return NULL;

    // Find the type and length of the string, and only accept strings.
    if (RegQueryValueEx(key, reg_val->value, NULL, &valtype, NULL, &len) == ERROR_SUCCESS && valtype == REG_SZ)
    {
        // Allocate a buffer for the value and read the value
        result = malloc(len);

        if (RegQueryValueEx(key, reg_val->value, NULL, &valtype, (unsigned char *)result, &len) != ERROR_SUCCESS)
        {
            free(result);
            result = NULL;
        }
        else
            // Ensure the value is null-terminated
            result[len] = '\0';
    }

    // Close the key
    RegCloseKey(key);

    return result;
}

// Check for the uninstall strings from the CD versions
static void CheckUninstallStrings(void)
{
    int len = (int)strlen(UNINSTALLER_STRING);

    for (size_t i = 0; i < arrlen(uninstall_values); i++)
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
            char    *path = unstr + len;

            AddIWADDir(path);
        }
    }
}

// Check for GOG.com and DOOM: Collector's Edition
static void CheckInstallRootPaths(void)
{
    for (size_t i = 0; i < arrlen(root_path_keys); i++)
    {
        char    *install_path = GetRegistryString(&root_path_keys[i]);

        if (!install_path)
            continue;

        for (size_t j = 0; j < arrlen(root_path_subdirs); j++)
            AddIWADDir(M_StringJoin(install_path, DIR_SEPARATOR_S, root_path_subdirs[j], NULL));

        free(install_path);
    }
}

// Check for DOOM downloaded via Steam
static void CheckSteamEdition(void)
{
    char    *install_path = GetRegistryString(&steam_install_location);

    if (!install_path)
        return;

    for (size_t i = 0; i < arrlen(steam_install_subdirs); i++)
        AddIWADDir(M_StringJoin(install_path, DIR_SEPARATOR_S, steam_install_subdirs[i], NULL));

    free(install_path);
}

// Default install directories for DOS DOOM
static void CheckDOSDefaults(void)
{
    // These are the default install directories used by the deice
    // installer program:
    AddIWADDir("\\doom2");      // DOOM II
    AddIWADDir("\\plutonia");   // Final DOOM
    AddIWADDir("\\tnt");
    AddIWADDir("\\doom_se");    // Ultimate DOOM
    AddIWADDir("\\doom");       // Shareware/Registered DOOM
    AddIWADDir("\\dooms");      // Shareware versions
    AddIWADDir("\\doomsw");
}

#endif

static struct
{
    char            *name;
    GameMission_t   mission;
} iwads[] = {
    { "doom2",    doom2      },
    { "nerve",    pack_nerve },
    { "plutonia", pack_plut  },
    { "tnt",      pack_tnt   },
    { "doom",     doom       },
    { "doom1",    doom       },
    { "hacx",     doom2      }
};

// When given an IWAD with the '-iwad' parameter,
// attempt to identify it by its name.
void D_IdentifyIWADByName(char *name)
{
    // Trim down the name to just the filename, ignoring the path.
    char    *p = strrchr(name, '\\');

    if (!p)
        p = strrchr(name, '/');

    if (p)
        name = p + 1;

    gamemission = none;

    for (size_t i = 0; i < arrlen(iwads); i++)
    {
        char    *iwad = M_StringJoin(iwads[i].name, ".WAD", NULL);

        // Check if the filename is this IWAD name.
        if (M_StringCompare(name, iwad))
        {
            gamemission = iwads[i].mission;
            break;
        }
    }

    if (M_StringCompare(name, "HACX.WAD"))
        hacx = true;
}

//
// Add directories from the list in the DOOMWADPATH environment variable.
//
static void AddDoomWADPath(void)
{
    char    *doomwadpath = SDL_getenv("DOOMWADPATH");
    char    *p;

    if (!doomwadpath)
        return;

    // Add the initial directory
    AddIWADDir(doomwadpath);

    // Split into individual dirs within the list.
    p = doomwadpath;

    for (;;)
    {
        if ((p = strchr(p, PATH_SEPARATOR)))
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
    char            *doomwaddir;
    static dboolean iwad_dirs_built;

    if (iwad_dirs_built)
        return;

    // Add DOOMWADDIR if it is in the environment
    if ((doomwaddir = SDL_getenv("DOOMWADDIR")))
        AddIWADDir(doomwaddir);

    // Add dirs from DOOMWADPATH
    AddDoomWADPath();

#if defined(_WIN32)
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
char *D_FindWADByName(char *filename)
{
    char    *path;

    // Absolute path?
    if (M_FileExists(filename))
        return filename;

    path = M_StringJoin(filename, ".WAD", NULL);

    if (M_FileExists(path))
        return path;

    BuildIWADDirList();

    // Search through all IWAD paths for a file with the given name.
    for (int i = 0; i < num_iwad_dirs; i++)
    {
        // As a special case, if this is in DOOMWADDIR or DOOMWADPATH,
        // the "directory" may actually refer directly to an IWAD file.
        if (M_StringCompare(leafname(iwad_dirs[i]), filename) && M_FileExists(iwad_dirs[i]))
            return strdup(iwad_dirs[i]);

        // Construct a string for the full path
        path = M_StringJoin(iwad_dirs[i], DIR_SEPARATOR_S, filename, NULL);

        if (M_FileExists(path))
            return path;
    }

    free(path);

    // File not found
    return NULL;
}

void D_InitIWADFolder(void)
{
    BuildIWADDirList();

    for (int i = 0; i < num_iwad_dirs; i++)
        if (M_FolderExists(iwad_dirs[i]))
        {
            iwadfolder = strdup(iwad_dirs[i]);
            strreplace(iwadfolder, "/", "\\");
            break;
        }
}

//
// D_TryWADByName
//
// Searches for a WAD by its filename, or passes through the filename
// if not found.
//
char *D_TryFindWADByName(char *filename)
{
    char    *result = D_FindWADByName(filename);

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
    char    *result = NULL;
    int     iwadparm = M_CheckParmWithArgs("-iwad", 1, 1);

    if (iwadparm)
    {
        // Search through IWAD dirs for an IWAD with the given name.
        char    *iwadfile = myargv[iwadparm + 1];

        if (!(result = D_FindWADByName(iwadfile)))
            I_Error("The IWAD file \"%s\" wasn't found!", iwadfile);

        D_IdentifyIWADByName(result);
    }

    return result;
}

//
// Get the IWAD name used for savegames.
//
static char *SaveGameIWADName(void)
{
    // Find what subdirectory to use for savegames
    //
    // The directory depends on the IWAD, so that savegames for
    // different IWADs are kept separate.
    //
    // Note that we match on gamemission rather than on IWAD name.
    // This ensures that doom1.wad and doom.wad saves are stored
    // in the same place.
    if (FREEDOOM)
        return (gamemode == commercial ? "freedoom2" : "freedoom");
    else if (hacx)
        return "hacx";

    for (size_t i = 0; i < arrlen(iwads); i++)
        if (gamemission == iwads[i].mission)
            return iwads[i].name;

    return NULL;
}

extern char *pwadfile;

//
// SetSaveGameFolder
//
// Chooses the directory used to store saved games.
//
void D_SetSaveGameFolder(dboolean output)
{
    int p = M_CheckParmsWithArgs("-save", "-savedir", 1, 1);

    if (p)
    {
        if (myargv[p + 1][strlen(myargv[p + 1]) - 1] != DIR_SEPARATOR)
            savegamefolder = M_StringJoin(myargv[p + 1], DIR_SEPARATOR_S, NULL);
    }
    else
    {
        char    *iwad_name = SaveGameIWADName();
        char    *appdatafolder = M_GetAppDataFolder();

        if (!iwad_name)
            iwad_name = "unknown";

        M_MakeDirectory(appdatafolder);
        savegamefolder = M_StringJoin(appdatafolder, DIR_SEPARATOR_S, "savegames", DIR_SEPARATOR_S, NULL);
        M_MakeDirectory(savegamefolder);
        savegamefolder = M_StringJoin(savegamefolder, (*pwadfile ? pwadfile : iwad_name), DIR_SEPARATOR_S, NULL);
    }

    M_MakeDirectory(savegamefolder);

    if (output)
    {
        int numsavegames = M_CountSaveGames();

        if (!numsavegames)
            C_Output("Savegames will be saved in <b>%s</b>.", savegamefolder);
        else if (numsavegames == 1)
            C_Output("There is 1 savegame in <b>%s</b>.", savegamefolder);
        else
            C_Output("There are %i savegames in <b>%s</b>.", numsavegames, savegamefolder);
    }
}

//
// Find out what version of DOOM is playing.
//
void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD function. But if
    // we specify '-iwad', we have to identify using
    // D_IdentifyIWADByName. However, if the iwad does not match
    // any known IWAD name, we may have a dilemma. Try to
    // identify by its contents.
    if (gamemission == none)
    {
        for (int i = 0; i < numlumps; i++)
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
    gamedescription = PACKAGE_NAME;

    if (chex1)
        gamedescription = s_CAPTION_CHEX;
    else if (chex2)
        gamedescription = s_CAPTION_CHEX2;
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
        if (modifiedgame && *pwadfile)
            gamedescription = M_StringJoin(uppercase(pwadfile), ".WAD", NULL);
        else if (FREEDOOM)
            gamedescription = s_CAPTION_FREEDOOM1;
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
        if (modifiedgame && *pwadfile)
            gamedescription = M_StringJoin(uppercase(pwadfile), ".WAD", NULL);
        else if (FREEDOOM)
            gamedescription = (FREEDM ? s_CAPTION_FREEDM : s_CAPTION_FREEDOOM2);
        else if (nerve)
            gamedescription = s_CAPTION_DOOM2;
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
            C_Output("Playing <i><b>%s: %s (%s)</b></i> and <i><b>%s: %s (%s)</b></i>.", s_CAPTION_DOOM2, s_CAPTION_HELLONEARTH,
                s_CAPTION_BFGEDITION, s_CAPTION_DOOM2, s_CAPTION_NERVE, s_CAPTION_BFGEDITION);
        else
            C_Output("Playing <i><b>%s: %s</b></i> and <i><b>%s: %s</b></i>.", s_CAPTION_DOOM2, s_CAPTION_HELLONEARTH,
                s_CAPTION_DOOM2, s_CAPTION_NERVE);
    }
    else if (modifiedgame)
        C_Output("Playing <b>%s</b>.", gamedescription);
    else
    {
        if (bfgedition)
            C_Output("Playing <i><b>%s (%s)</b></i>.", gamedescription, s_CAPTION_BFGEDITION);
        else
            C_Output("Playing <i><b>%s</b></i>.", gamedescription);
    }
}
