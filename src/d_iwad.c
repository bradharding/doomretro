/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "c_console.h"
#include "d_deh.h"
#include "d_main.h"
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

char        screenshotfolder[MAX_PATH];

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

// [AlexMax] From the perspective of a 64-bit executable, 32-bit registry
// keys are located in a different spot.
#if defined(_WIN64)
#define SOFTWARE_KEY    "SOFTWARE\\WOW6432Node\\"
#else
#define SOFTWARE_KEY    "SOFTWARE\\"
#endif

static registryvalue_t uninstall_values[] =
{
    // Ultimate DOOM, CD version (Depths of DOOM trilogy)
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ultimate Doom for Windows 95",
        "UninstallString"
    },

    // DOOM II, CD version (Depths of DOOM trilogy)
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "Microsoft\\Windows\\CurrentVersion\\Uninstall\\Doom II for Windows 95",
        "UninstallString"
    },

    // Final DOOM
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "Microsoft\\Windows\\CurrentVersion\\Uninstall\\Final Doom for Windows 95",
        "UninstallString"
    },

    // Shareware version
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "Microsoft\\Windows\\CurrentVersion\\Uninstall\\Doom Shareware for Windows 95",
        "UninstallString"
    }
};

// Values installed by the GOG.com and Collector's Edition versions
static registryvalue_t root_path_keys[] =
{
    // DOOM Collector's Edition
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "Activision\\DOOM Collector's Edition\\v1.0",
        "INSTALLPATH"
    },

    // DOOM
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "GOG.com\\Games\\2015545325",
        "INSTALLPATH"
    },

    // DOOM II
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "GOG.com\\Games\\1435848814",
        "PATH"
    },

    // DOOM II
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "GOG.com\\Games\\1426071866",
        "PATH"
    },

    // DOOM 3: BFG Edition
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "GOG.com\\Games\\1135892318",
        "PATH"
    },

    // Final DOOM
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "GOG.com\\Games\\1435848742",
        "PATH"
    },

    // Ultimate DOOM
    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "GOG.com\\Games\\1435827232",
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

// Location where the Bethesda.net Launcher is installed
static registryvalue_t bethesda_install_location =
{
    HKEY_LOCAL_MACHINE, SOFTWARE_KEY "Bethesda Softworks\\Bethesda.net", "installLocation"
};

// Subdirs of the Bethesda.net Launcher install directory where IWADs are found
static const char *bethesda_install_subdirs[] =
{
    "games\\DOOM_II_Classic_2019\\base",
    "games\\DOOM_II_Classic_2019\\rerelease\\DOOM II_Data\\StreamingAssets",
    "games\\DOOM_Classic_2019\\base",
    "games\\DOOM_Classic_2019\\rerelease\\DOOM_Data\\StreamingAssets",
    "games\\DOOM 3 BFG Edition\\base\\wads",
    "games\\DOOM Eternal\\base\\classicwads"
};

// Locations where Steam is installed
static registryvalue_t steam_install_locations[] =
{
    { HKEY_CURRENT_USER,  "SOFTWARE\\Valve\\Steam",    "SteamPath"   },
    { HKEY_LOCAL_MACHINE, SOFTWARE_KEY "Valve\\Steam", "InstallPath" }
};

// Subdirs of the Steam install directory where IWADs are found
static const char *steam_install_subdirs[] =
{
    "steamapps\\common\\Doom 2\\rerelease\\DOOM II_Data\\StreamingAssets",
    "steamapps\\common\\Doom 2\\base",
    "steamapps\\common\\Doom 2\\finaldoombase",
    "steamapps\\common\\Ultimate Doom\\rerelease\\DOOM_Data\\StreamingAssets",
    "steamapps\\common\\Ultimate Doom\\base"
    "steamapps\\common\\DOOM 3 BFG Edition\\base\\wads",
    "steamapps\\common\\Final Doom\\base",
    "steamapps\\common\\DOOMEternal\\base\\classicwads"
};

static char *GetRegistryString(registryvalue_t *reg_val)
{
    HKEY    key;
    DWORD   len = 0;
    DWORD   valtype;
    char    *result = NULL;

    // Open the key (directory where the value is stored)
    if (RegOpenKeyEx(reg_val->root, reg_val->path, 0, KEY_READ, &key) != ERROR_SUCCESS)
        return NULL;

    // Find the type and length of the string, and only accept strings.
    if (RegQueryValueEx(key, reg_val->value, NULL, &valtype, NULL, &len) == ERROR_SUCCESS && valtype == REG_SZ && len > 0)
    {
        // Allocate a buffer for the value and read the value
        result = malloc((size_t)len + 1);

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
    const int   len = (int)strlen(UNINSTALLER_STRING);

    for (size_t i = 0; i < arrlen(uninstall_values); i++)
    {
        char    *val = GetRegistryString(&uninstall_values[i]);
        char    *unstr;

        if (!val)
            continue;

        if ((unstr = strstr(val, UNINSTALLER_STRING)))
        {
            char    *path = unstr + len;

            AddIWADDir(path);
        }
        else
            free(val);
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
        {
            char    *path = M_StringJoin(install_path, DIR_SEPARATOR_S, root_path_subdirs[j], NULL);

            AddIWADDir(path);
        }

        free(install_path);
    }
}

// Check for DOOM downloaded via the Bethesda.net Launcher
static void CheckBethesdaEdition(void)
{
    char    *install_path = GetRegistryString(&bethesda_install_location);

    if (!install_path)
        return;

    for (size_t j = 0; j < arrlen(bethesda_install_subdirs); j++)
    {
        char    *path = M_StringJoin(install_path, DIR_SEPARATOR_S, bethesda_install_subdirs[j], NULL);

        AddIWADDir(path);
    }

    free(install_path);
}

// Check for DOOM downloaded via Steam
static void CheckSteamEdition(void)
{
    for (size_t i = 0; i < arrlen(steam_install_locations); i++)
    {
        char    *install_path = GetRegistryString(&steam_install_locations[i]);

        if (!install_path)
            continue;

        for (size_t j = 0; j < arrlen(steam_install_subdirs); j++)
        {
            char    *path = M_StringJoin(install_path, DIR_SEPARATOR_S, steam_install_subdirs[j], NULL);

            AddIWADDir(path);
        }

        free(install_path);
    }
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
#else
// Add IWAD directories parsed from splitting a path string containing
// paths separated by PATH_SEPARATOR. 'suffix' is a string to concatenate
// to the end of the paths before adding them.
static void AddIWADPath(const char *path, const char *suffix)
{
    char    *p;
    char    *dup_path = M_StringDuplicate(path);

    // Split into individual directories within the list.
    char    *left = dup_path;

    while (true)
        if ((p = strchr(left, PATH_SEPARATOR)))
        {
            // Break at the separator and use the left hand side
            // as another IWAD dir
            *p = '\0';

            AddIWADDir(M_StringJoin(left, suffix, NULL));
            left = p + 1;
        }
        else
            break;

    AddIWADDir(M_StringJoin(left, suffix, NULL));
    free(dup_path);
}

// Add standard directories where IWADs are located on Unix systems.
// To respect the freedesktop.org specification we support overriding
// using standard environment variables. See the XDG Base Directory
// Specification:
// <http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>
static void AddXdgDirs(void)
{
    // Quote:
    // > $XDG_DATA_HOME defines the base directory relative to which
    // > user specific data files should be stored. If $XDG_DATA_HOME
    // > is either not set or empty, a default equal to
    // > $HOME/.local/share should be used.
    char    *env = getenv("XDG_DATA_HOME");
    char    *tmp_env = NULL;

    if (!env)
    {
        char    *homedir = getenv("HOME");

        if (!homedir)
            homedir = "/";

        tmp_env = M_StringJoin(homedir, "/.local/share", NULL);
        env = tmp_env;
    }

    // We support $XDG_DATA_HOME/games/doom (which will usually be
    // ~/.local/share/games/doom) as a user-writable extension to
    // the usual /usr/share/games/doom location.
    AddIWADDir(M_StringJoin(env, "/games/doom", NULL));
    free(tmp_env);

    // Quote:
    // > $XDG_DATA_DIRS defines the preference-ordered set of base
    // > directories to search for data files in addition to the
    // > $XDG_DATA_HOME base directory. The directories in $XDG_DATA_DIRS
    // > should be seperated with a colon ':'.
    // >
    // > If $XDG_DATA_DIRS is either not set or empty, a value equal to
    // > /usr/local/share/:/usr/share/ should be used.

    if (!(env = getenv("XDG_DATA_DIRS")))
        // (Trailing / omitted from paths, as it is added below)
        env = "/usr/local/share:/usr/share";

    // The "standard" location for IWADs on Unix that is supported by most
    // source ports is /usr/share/games/doom - we support this through the
    // XDG_DATA_DIRS mechanism, through which it can be overridden.
    AddIWADPath(env, "/games/doom");
    AddIWADPath(env, "/doom");

    // The convention set by RBDOOM-3-BFG is to install DOOM 3: BFG
    // Edition into this directory, under which includes the DOOM
    // Classic WADs.
    AddIWADPath(env, "/games/doom3bfg/base/wads");
}

#if !defined(__APPLE__)
// Steam on Linux allows installing some select Windows games,
// including the classic DOOM series (running DOSBox via Wine). We
// could parse *.vdf files to more accurately detect installation
// locations, but the defaults are likely to be good enough for just
// about everyone.
static void AddSteamDirs(void)
{
    char    *homedir = getenv("HOME");
    char    *steampath;

    if (!homedir)
        homedir = "/";

    steampath = M_StringJoin(homedir, "/.steam/root/steamapps/common", NULL);

    AddIWADPath(steampath, "/Doom 2/base");
    AddIWADPath(steampath, "/Ultimate Doom/base");
    AddIWADPath(steampath, "/Final Doom/base");
    AddIWADPath(steampath, "/DOOM 3 BFG Edition/base/wads");
    free(steampath);
}
#endif
#endif

typedef struct
{
    char            *name;
    gamemission_t   mission;
} iwads_t;

static const iwads_t iwads[] =
{
    { "doom2",    doom2      },
    { "nerve",    pack_nerve },
    { "plutonia", pack_plut  },
    { "tnt",      pack_tnt   },
    { "doom",     doom       },
    { "doom1",    doom       },
    { "hacx",     doom2      },
    { "",         0          }
};

#if !defined(_WIN32) && !defined(__APPLE__)
// Returns true if the specified path is a path to a file
// of the specified name.
static bool DirIsFile(char *path, char *filename)
{
    return (strchr(path, DIR_SEPARATOR) && !strcasecmp(leafname(path), filename));
}

// Check if the specified directory contains the specified IWAD
// file, returning the full path to the IWAD if found, or NULL
// if not found.
static char *CheckDirectoryHasIWAD(char *dir, char *iwadname)
{
    char    *filename;
    char    *probe = M_FileCaseExists(dir);

    // As a special case, the "directory" may refer directly to an
    // IWAD file if the path comes from DOOMWADDIR or DOOMWADPATH.
    if (DirIsFile(dir, iwadname) && probe)
        return probe;

    // Construct the full path to the IWAD if it is located in
    // this directory, and check if it exists.
    if (!strcmp(dir, "."))
        filename = M_StringDuplicate(iwadname);
    else
        filename = M_StringJoin(dir, DIR_SEPARATOR_S, iwadname, ".wad", NULL);

    free(probe);
    probe = M_FileCaseExists(filename);
    free(filename);

    return probe;
}

// Search a directory to try to find an IWAD
// Returns the location of the IWAD if found, otherwise NULL.
static char *SearchDirectoryForIWAD(char *dir)
{
    for (size_t i = 0; i < arrlen(iwads); i++)
    {
        char    *filename = CheckDirectoryHasIWAD(dir, iwads[i].name);

        if (filename)
        {
            gamemission = iwads[i].mission;
            return filename;
        }
    }

    return NULL;
}
#endif

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

    for (size_t i = 0; iwads[i].name[0]; i++)
    {
        char    *iwad = M_StringJoin(iwads[i].name, ".WAD", NULL);

        // Check if the filename is this IWAD name.
        if (M_StringCompare(name, iwad))
        {
            gamemission = iwads[i].mission;
            free(iwad);
            break;
        }

        free(iwad);
    }

    if (M_StringCompare(name, "HACX.WAD"))
        hacx = true;
    else if (M_StringCompare(name, "harmony.wad") || M_StringCompare(name, "harm1.wad"))
        harmony = true;
}

//
// Add directories from the list in the DOOMWADPATH environment variable.
//
static void AddDoomWADPath(void)
{
    char    *doomwadpath = getenv("DOOMWADPATH");
    char    *p;

    if (!doomwadpath)
        return;

    // Add the initial directory
    AddIWADDir(doomwadpath);

    // Split into individual directories within the list.
    p = doomwadpath;

    while (true)
    {
        if ((p = strchr(p, PATH_SEPARATOR)))
        {
            // Break at the separator and store the right hand side
            // as another IWAD dir
            *p++ = '\0';

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
    static bool iwad_dirs_built;

    if (iwad_dirs_built)
        return;

    // Add DOOMWADDIR if it is in the environment
    if ((doomwaddir = getenv("DOOMWADDIR")))
        AddIWADDir(doomwaddir);

    // Add directories from DOOMWADPATH
    AddDoomWADPath();

#if defined(_WIN32)
    // Search the registry and find where IWADs have been installed.
    CheckBethesdaEdition();
    CheckSteamEdition();
    CheckInstallRootPaths();
    CheckUninstallStrings();
    CheckDOSDefaults();
#else
    AddXdgDirs();

#if !defined(__APPLE__)
    AddSteamDirs();
#endif
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
        if (M_StringEndsWith(iwad_dirs[i], filename) && M_FileExists(iwad_dirs[i]))
            return M_StringDuplicate(iwad_dirs[i]);

        // Construct a string for the full path
        free(path);
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
#if defined(_WIN32)
    char    path[MAX_PATH];
#endif

    BuildIWADDirList();

    for (int i = 0; i < num_iwad_dirs; i++)
        if (M_FolderExists(iwad_dirs[i]))
        {
            iwadfolder = M_StringDuplicate(iwad_dirs[i]);
            break;
        }

#if defined(_WIN32)
    M_snprintf(path, sizeof(path), "%s" DIR_SEPARATOR_S "DOOM.WAD", iwadfolder);

    if (M_FileExists(path))
        wad = "DOOM.WAD";
    else
    {
        M_snprintf(path, sizeof(path), "%s" DIR_SEPARATOR_S "DOOM2.WAD", iwadfolder);

        if (M_FileExists(path))
            wad = "DOOM2.WAD";
    }
#endif
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
        // Search through IWAD directories for an IWAD with the given name.
        char    *iwadfile = myargv[iwadparm + 1];

        if (!(result = D_FindWADByName(iwadfile)))
            I_Error("The IWAD file \"%s\" wasn't found!", iwadfile);

        D_IdentifyIWADByName(result);
    }
#if !defined(_WIN32) && !defined(__APPLE__)
    else
    {
        // Search through the list and look for an IWAD
        BuildIWADDirList();

        for (int i = 0; !result && i < num_iwad_dirs; i++)
            result = SearchDirectoryForIWAD(iwad_dirs[i]);
    }
#endif

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
    // This ensures that DOOM1.WAD and DOOM.WAD saves are stored
    // in the same place.
    if (FREEDOOM)
        return (gamemode == commercial ? "freedoom2" : "freedoom");
    else if (hacx)
        return "hacx";

    for (size_t i = 0; iwads[i].name[0]; i++)
        if (gamemission == iwads[i].mission)
            return iwads[i].name;

    return "unknown";
}

//
// SetSaveGameFolder
//
// Chooses the directory used to store saved games.
//
void D_SetSaveGameFolder(bool output)
{
    const int   p = M_CheckParmsWithArgs("-save", "-savedir", "", 1, 1);

    if (p)
    {
        if (myargv[p + 1][strlen(myargv[p + 1]) - 1] != DIR_SEPARATOR)
            savegamefolder = M_StringJoin(myargv[p + 1], DIR_SEPARATOR_S, NULL);
    }
    else
    {
        char    *appdatafolder = M_GetAppDataFolder();
        char    *tmpsavegamefolder;

        M_MakeDirectory(appdatafolder);
        tmpsavegamefolder = M_StringJoin(appdatafolder, DIR_SEPARATOR_S DOOMRETRO_SAVEGAMESFOLDER DIR_SEPARATOR_S, NULL);
        M_MakeDirectory(savegamefolder);

        if (*pwadfile)
        {
            char    *temp = removeext(GetCorrectCase(pwadfile));

            savegamefolder = M_StringJoin(tmpsavegamefolder, temp, DIR_SEPARATOR_S, NULL);
            free(temp);
        }
        else
            savegamefolder = M_StringJoin(tmpsavegamefolder, SaveGameIWADName(), DIR_SEPARATOR_S, NULL);

        free(appdatafolder);
	free(tmpsavegamefolder);
    }

    M_MakeDirectory(savegamefolder);

    if (output)
    {
        int numsavegames = M_CountSaveGames();

        if (!numsavegames)
            C_Output("All savegames will be placed in " BOLD("%s") ".", savegamefolder);
        else if (numsavegames == 1)
            C_Output("There is 1 savegame in " BOLD("%s") ".", savegamefolder);
        else
            C_Output("There are %i savegames in " BOLD("%s") ".", numsavegames, savegamefolder);
    }
}

void D_SetAutoLoadFolder(void)
{
    const int   p = M_CheckParmsWithArgs("-autoload", "-autoloaddir", "", 1, 1);

    if (p)
        M_StringCopy(autoloadfolder, myargv[p + 1], sizeof(autoloadfolder));
    else
    {
        char    *appdatafolder = M_GetAppDataFolder();

        M_MakeDirectory(appdatafolder);
        autoloadfolder = M_StringJoin(appdatafolder, DIR_SEPARATOR_S DOOMRETRO_AUTOLOADFOLDER DIR_SEPARATOR_S, NULL);
        free(appdatafolder);
    }

    M_MakeDirectory(autoloadfolder);

    autoloadiwadsubfolder = M_StringJoin(autoloadfolder, SaveGameIWADName(), DIR_SEPARATOR_S, NULL);
    M_MakeDirectory(autoloadiwadsubfolder);

    if (*pwadfile)
    {
        char    *temp = removeext(GetCorrectCase(pwadfile));

        autoloadpwadsubfolder = M_StringJoin(autoloadfolder, temp, DIR_SEPARATOR_S, NULL);
        M_MakeDirectory(autoloadpwadsubfolder);
        free(temp);
    }
}

void D_SetScreenshotsFolder(void)
{
    const int   p = M_CheckParmsWithArgs("-shot", "-shotdir", "", 1, 1);

    if (p)
        M_StringCopy(screenshotfolder, myargv[p + 1], sizeof(screenshotfolder));
    else
    {
        char    *appdatafolder = M_GetAppDataFolder();

        M_snprintf(screenshotfolder, sizeof(screenshotfolder),
            "%s" DIR_SEPARATOR_S DOOMRETRO_SCREENSHOTSFOLDER DIR_SEPARATOR_S, appdatafolder);

        free(appdatafolder);
    }

    M_MakeDirectory(screenshotfolder);

    C_Output("All screenshots taken will be placed in " BOLD("%s") ".", screenshotfolder);
}

//
// Find out what version of DOOM is playing.
//
void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD() function. But if
    // we specify '-iwad', we have to identify using
    // D_IdentifyIWADByName(). However, if the IWAD does not match
    // any known IWAD name, we may have a dilemma. Try to
    // identify by its contents.
    if (gamemission == none)
    {
        if (W_CheckNumForName("MAP01") >= 0)
            gamemission = doom2;
        else if (W_CheckNumForName("E1M1") >= 0)
            gamemission = doom;
        else
            // Still no idea. I don't think this is going to work.
            I_Error("Unknown or invalid IWAD file.");
    }

    // Make sure gamemode is set up correctly
    if (gamemission == doom)
    {
        // DOOM 1. But which version?
        if (W_CheckNumForName("E4M1") >= 0)
            // Ultimate DOOM
            gamemode = retail;
        else if (W_CheckNumForName("E3M1") >= 0)
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
    bool    defaultname = M_StringCompare(playername, playername_default);

    if (chex1)
        M_StringCopy(gamedescription, s_CAPTION_CHEX, sizeof(gamedescription));
    else if (chex2)
        M_StringCopy(gamedescription, s_CAPTION_CHEX2, sizeof(gamedescription));
    else if (hacx)
        M_StringCopy(gamedescription, s_CAPTION_HACX, sizeof(gamedescription));
    else if (BTSXE1)
        M_StringCopy(gamedescription, s_CAPTION_BTSXE1, sizeof(gamedescription));
    else if (BTSXE2)
        M_StringCopy(gamedescription, s_CAPTION_BTSXE2, sizeof(gamedescription));
    else if (REKKRSL)
        M_StringCopy(gamedescription, s_CAPTION_REKKRSL, sizeof(gamedescription));
    else if (REKKR)
        M_StringCopy(gamedescription, s_CAPTION_REKKR, sizeof(gamedescription));
    else if (anomalyreport)
        M_StringCopy(gamedescription, s_CAPTION_ANOMALYREPORT, sizeof(gamedescription));
    else if (arrival)
        M_StringCopy(gamedescription, s_CAPTION_ARRIVAL, sizeof(gamedescription));
    else if (dbimpact)
        M_StringCopy(gamedescription, s_CAPTION_DBIMPACT, sizeof(gamedescription));
    else if (deathless)
        M_StringCopy(gamedescription, s_CAPTION_DEATHLESS, sizeof(gamedescription));
    else if (doomzero)
        M_StringCopy(gamedescription, s_CAPTION_DOOMZERO, sizeof(gamedescription));
    else if (earthless)
        M_StringCopy(gamedescription, s_CAPTION_EARTHLESS, sizeof(gamedescription));
    else if (harmony)
        M_StringCopy(gamedescription, s_CAPTION_HARMONY, sizeof(gamedescription));
    else if (KDIKDIZD)
        M_StringCopy(gamedescription, s_CAPTION_KDIKDIZD, sizeof(gamedescription));
    else if (neis)
        M_StringCopy(gamedescription, s_CAPTION_NEIS, sizeof(gamedescription));
    else if (revolution)
        M_StringCopy(gamedescription, s_CAPTION_REVOLUTION, sizeof(gamedescription));
    else if (syringe)
        M_StringCopy(gamedescription, s_CAPTION_SYRINGE, sizeof(gamedescription));
    else if (gamemission == doom)
    {
        // DOOM 1. But which version?
        if (modifiedgame && *pwadfile)
        {
            M_StringCopy(gamedescription, GetCorrectCase(pwadfile), sizeof(gamedescription));
            return;
        }
        else if (FREEDOOM)
            M_StringCopy(gamedescription, s_CAPTION_FREEDOOM1, sizeof(gamedescription));
        else
            M_StringCopy(gamedescription, s_CAPTION_DOOM, sizeof(gamedescription));
    }
    else
    {
        // DOOM 2 of some kind. But which mission?
        if (modifiedgame && *pwadfile)
        {
            if (M_StringCompare(pwadfile, "nerve.wad"))
                M_StringCopy(gamedescription, s_CAPTION_DOOM2, sizeof(gamedescription));
            else
            {
                M_StringCopy(gamedescription, GetCorrectCase(pwadfile), sizeof(gamedescription));
                return;
            }
        }
        else if (FREEDOOM)
            M_StringCopy(gamedescription, (FREEDM ? s_CAPTION_FREEDM : s_CAPTION_FREEDOOM2), sizeof(gamedescription));
        else if (nerve)
            M_StringCopy(gamedescription, s_CAPTION_DOOM2, sizeof(gamedescription));
        else if (gamemission == doom2)
            M_snprintf(gamedescription, sizeof(gamedescription), "%s: %s", s_CAPTION_DOOM2, s_CAPTION_HELLONEARTH);
        else if (gamemission == pack_plut)
            M_StringCopy(gamedescription, s_CAPTION_PLUTONIA, sizeof(gamedescription));
        else if (gamemission == pack_tnt)
            M_StringCopy(gamedescription, s_CAPTION_TNT, sizeof(gamedescription));
    }

    if (nerve)
            C_Output("%s %s playing " ITALICS("%s: %s") " and " ITALICS("%s: %s."),
                (defaultname ? "You" : playername), (defaultname ? "are" : "is"),
                s_CAPTION_DOOM2, s_CAPTION_HELLONEARTH, s_CAPTION_DOOM2, s_CAPTION_NERVE);
    else if (modifiedgame && !sigil)
        C_Output("%s %s playing " ITALICS("%s%s"),
            (defaultname ? "You" : playername), (defaultname ? "are" : "is"),
            gamedescription, (ispunctuation(gamedescription[strlen(gamedescription) - 1]) ? "" : "."));
    else
            C_Output("%s %s playing " ITALICS("%s%s"),
                (defaultname ? "You" : playername), (defaultname ? "are" : "is"),
                gamedescription, (ispunctuation(gamedescription[strlen(gamedescription) - 1]) ? "" : "."));
}
