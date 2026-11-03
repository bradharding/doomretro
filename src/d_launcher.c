/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

#if defined(_WIN32)
#pragma comment(lib, "winmm.lib")
#include <Windows.h>
#include <commdlg.h>
#endif

#include "c_console.h"
#include "d_deh.h"
#include "d_iwad.h"
#include "d_launcher.h"
#include "d_main.h"
#include "d_options.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_setup.h"
#include "w_merge.h"
#include "w_wad.h"

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#endif

#if defined(_WIN32) || defined(__APPLE__)
static char *invalidwad;
static char *collected_wads = NULL;
static int  numcollectedwads;

static void AddToWadList(const char *filename)
{
    char    *wadname;

    if (M_StringEndsWith(filename, ".lmp"))
        return;

    wadname = GetCorrectCase(M_StringDuplicate(filename));
    numcollectedwads++;

    if (strchr(wadname, ' '))
    {
        char    *quoted = M_StringJoin("\"", wadname, "\"", NULL);

        free(wadname);
        wadname = quoted;
    }

    if (!collected_wads)
        collected_wads = wadname;
    else
    {
        char    *temp = M_StringJoin(collected_wads, " ", wadname, NULL);

        free(collected_wads);
        free(wadname);
        collected_wads = temp;
    }
}

int D_OpenWADLauncher(void)
{
    int             iwadfound = -1;
    bool            fileopenedok;

#if defined(_WIN32)
    OPENFILENAME    ofn;
    char            szFile[4096] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = HWND_DESKTOP;

    if (invalidwad)
        M_StringCopy(szFile, invalidwad, sizeof(szFile));
    else if (wad)
    {
        char    *tokens[100];
        int     numtokens = D_SplitWADList(wad, tokens, 100);

        for (int i = 0; i < numtokens; i++)
        {
            char    *temp = M_StringJoin(szFile, (i ? " " : ""), tokens[i], NULL);

            M_StringCopy(szFile, temp, sizeof(szFile));
            free(temp);
            free(tokens[i]);
        }
    }

    ofn.lpstrFile = szFile;

    if (collected_wads)
    {
        free(collected_wads);
        collected_wads = NULL;
    }

    numcollectedwads = 0;

    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "IWAD and/or PWAD(s) (*.wad)\0*.wad;*.iwad;*.pwad;*.lmp;*.deh;*.bex;*.cfg;*.pk3;*.zip\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = wadfolder;
    ofn.Flags = (OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_EXPLORER);
    ofn.lpstrTitle = "Where\u2019s All the Data?\0";

    fileopenedok = GetOpenFileName(&ofn);
#elif defined(__APPLE__)
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:YES];
    [panel setTitle:@"Where's All the Data?"];

    NSInteger   clicked = [panel runModal];

    fileopenedok = (clicked == NSModalResponseOK);
#endif

    if (fileopenedok)
    {
        bool    onlyoneselected;
        bool    guess = false;

#if defined(__APPLE__)
        NSArray *urls = [panel URLs];
#endif

        iwadfound = 0;

#if defined(_WIN32)
        if (wad)
            D_SetString(&previouswad, wad);

        D_SetString(&wad, "");

        if ((onlyoneselected = !ofn.lpstrFile[strlen(ofn.lpstrFile) + 1])
            && D_IsMultiWADSelection(ofn.lpstrFile))
        {
            char    tempbuf[4096];
            char    *filenames[100];
            int     filecount = D_SplitWADList(ofn.lpstrFile, filenames, 100);

            if (filecount > 1)
            {
                char    *basedir;
                char    *dest;
                size_t  remaining;

                if (strchr(filenames[0], '\\'))
                    basedir = M_ExtractFolder(filenames[0]);
                else
                    basedir = M_StringDuplicate(wadfolder);

                M_StringCopy(tempbuf, basedir, sizeof(tempbuf));
                dest = tempbuf + strlen(tempbuf) + 1;
                remaining = sizeof(tempbuf) - strlen(tempbuf) - 1;

                for (int i = 0; i < filecount && remaining > 0; i++)
                {
                    const char  *filename = leafname(filenames[i]);
                    size_t      len = strlen(filename);

                    if (len + 1 < remaining)
                    {
                        M_StringCopy(dest, filename, remaining);
                        dest += len + 1;
                        remaining -= len + 1;
                    }
                }

                if (remaining > 0)
                    *dest = '\0';

                free(basedir);

                memcpy(szFile, tempbuf, sizeof(tempbuf));

                onlyoneselected = false;
            }

            for (int i = 0; i < filecount; i++)
                free(filenames[i]);
        }
#elif defined(__APPLE__)
        onlyoneselected = ([urls count] == 1);
#endif

        if (onlyoneselected)
        {
#if defined(_WIN32)
            char    *file = (char *)ofn.lpstrFile;
#elif defined(__APPLE__)
            NSURL   *url = [urls objectAtIndex:0];
            char    *file = (char *)[url fileSystemRepresentation];
#endif
            char    *folder = M_ExtractFolder(file);

            if (!D_IsWADFile(file) && !D_IsDEHFile(file) && !D_IsCFGFile(file) && !strchr(file, '.'))
            {
                char    *temp = M_StringDuplicate(file);

                file = M_StringJoin(temp, ".wad", NULL);

                if (!M_FileExists(file))
                    file = M_StringJoin(temp, ".iwad", NULL);

                if (!M_FileExists(file))
                    file = M_StringJoin(temp, ".pwad", NULL);

                free(temp);
            }

#if defined(_WIN32)
            // if WAD doesn't exist, it was entered manually, there may be a typo, so guess what was intended
            if (!M_FileExists(file) && strlen(leafname(file)) > 2)
            {
                char    *temp = W_GuessFilename(folder, leafname(file));

                if (temp)
                {
                    guess = true;

                    if (!M_StringEndsWith(temp, leafname(file)))
                        C_Warning(0, BOLD("%s%s") " wasn't found so " BOLD("%s") " was loaded instead.",
                            (char *)ofn.lpstrFile, (M_StringEndsWith((char *)ofn.lpstrFile, ".wad") ? "" : ".wad"),
                            temp);

                    file = M_StringDuplicate(temp);
                    AddToWadList(leafname(temp));
                    free(temp);
                }
                else
                {
                    d_startuperror = true;
                    D_SetString(&invalidwad, (char *)ofn.lpstrFile);
                }
            }
#endif

            // check if the file exists but isn't actually a WAD
            if (D_IsWADFile(file) && M_FileExists(file) && !D_IsDOOMIWAD(file) && !W_WadType(file))
            {
                char    buffer[MAX_PATH + 64];

                M_snprintf(buffer, sizeof(buffer), "%s isn't a valid WAD.\n", leafname(file));
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, DOOMRETRO_NAME, buffer, NULL);

#if defined(_WIN32)
                if (previouswad)
                    wad = M_StringDuplicate(previouswad);
#endif

                d_startuperror = true;
            }
            // check if it's a valid and supported IWAD
            else if (D_IsDOOMIWAD(file) || (W_WadType(file) == IWAD && !D_IsUnsupportedWAD(file)))
            {
                D_IdentifyIWADByName(file);

                if (W_AddFile(file, false))
                {
                    iwadfound = 1;
                    wadfolder = M_StringDuplicate(folder);

#if defined(_WIN32)
                    // Add IWAD to list here instead
                    if (!guess)
                        AddToWadList(leafname(file));
#endif

                    // if DOOM.WAD is selected, load SIGIL.WAD automatically if present
                    if (D_IsDOOM1IWAD(file) && IsUltimateDOOM(file))
                    {
                        D_AutoloadSIGILWAD();
                        D_AutoloadSIGIL2WAD();
                    }
                    // if DOOM2.WAD is selected, load NERVE.WAD automatically if present
                    else if (D_IsDOOM2IWAD(file))
                    {
                        D_AutoloadNerveWAD();
                        D_AutoloadMasterLevelsWAD();
                    }
                }
            }

            // if it's a PWAD, determine the IWAD required and try loading that as well
            else if (W_WadType(file) == PWAD && !D_IsUnsupportedPWAD(file))
            {
                gamemission_t   iwadrequired = IWADRequiredByPWAD(file);
                char            fullpath[MAX_PATH];
                char            *iwadname;

                if (iwadrequired == none)
                    iwadrequired = doom2;

                iwadname = D_RequiredIWAD(file,
                    (M_StringCompare(leafname(file), "chex2.wad") ? "chex.wad" : iwadsrequired[iwadrequired]));

#if defined(_WIN32)
                if (!guess)
                    AddToWadList(leafname(file));
#endif

                // try the current folder first
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, iwadname);
                D_IdentifyIWADByName(fullpath);

                if (W_AddFile(fullpath, true))
                {
                    iwadfound = 1;
                    wadfolder = M_StringDuplicate(folder);
                    D_CheckSupportedPWAD(file);

                    if (D_IsSIGIL2WAD(file))
                        D_AutoloadSIGILWAD();

                    if (W_MergeFile(file, false))
                    {
                        modifiedgame = true;

                        if (legacyofrust)
                            D_AutoloadExtrasWAD();

                        if (PWADHasMaps(file) && IWADRequiredByPWAD(file) != none)
                            D_SetString(&pwadfile, leafname(file));

                        LoadCfgFile(file);

                        if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex") && !D_IsDEHFile(file))
                            LoadDEHFile(file, true);

                        if (W_GetNumLumps("M_DOOM") == 2 && !BTSX && !PWADHasMaps(file))
                        {
                            if (D_IsDOOM1IWAD(fullpath) && W_GetNumLumps("E1M1") == 1)
                            {
                                if (IsUltimateDOOM(fullpath))
                                {
                                    W_Init();

                                    if (W_CheckNumForName("M_EPI5") < 0 && W_CheckNumForName("E5M1") < 0)
                                        D_AutoloadSIGILWAD();

                                    if (W_CheckNumForName("M_EPI6") < 0 && W_CheckNumForName("E6M1") < 0)
                                        D_AutoloadSIGIL2WAD();
                                }
                            }
                            else if (D_IsDOOM2IWAD(fullpath) && W_GetNumLumps("MAP01") == 1)
                            {
                                D_AutoloadNerveWAD();
                                D_AutoloadMasterLevelsWAD();
                            }
                        }
                    }
                }
                else
                {
                    // otherwise try the wadfolder CVAR
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", wadfolder, iwadname);
                    D_IdentifyIWADByName(fullpath);

                    if (W_AddFile(fullpath, true))
                    {
                        iwadfound = 1;
                        wadfolder = M_StringDuplicate(folder);
                        D_CheckSupportedPWAD(file);

                        if (W_MergeFile(file, false))
                        {
                            modifiedgame = true;

                            if (legacyofrust)
                                D_AutoloadExtrasWAD();

                            if (PWADHasMaps(file) && IWADRequiredByPWAD(file) != none)
                                D_SetString(&pwadfile, leafname(file));

                            LoadCfgFile(file);

                            if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex") && !D_IsDEHFile(file))
                                LoadDEHFile(file, true);

                            if (W_GetNumLumps("M_DOOM") == 2 && !PWADHasMaps(file))
                            {
                                if (D_IsDOOM1IWAD(fullpath) && W_GetNumLumps("E1M1") == 1)
                                {
                                    if (IsUltimateDOOM(fullpath))
                                    {
                                        W_Init();

                                        if (W_CheckNumForName("M_EPI5") < 0 && W_CheckNumForName("E5M1") < 0)
                                            D_AutoloadSIGILWAD();

                                        if (W_CheckNumForName("M_EPI6") < 0 && W_CheckNumForName("E6M1") < 0)
                                            D_AutoloadSIGIL2WAD();
                                    }
                                }
                                else if (D_IsDOOM2IWAD(fullpath) && W_GetNumLumps("MAP01") == 1)
                                {
                                    D_AutoloadNerveWAD();
                                    D_AutoloadMasterLevelsWAD();
                                }
                            }
                        }
                    }
                    else
                    {
                        // still nothing? try some common installation folders
                        if (W_AddFile(D_FindWADByName(iwadname), true))
                        {
                            iwadfound = 1;
                            wadfolder = M_StringDuplicate(folder);
                            D_CheckSupportedPWAD(file);

                            if (W_MergeFile(file, false))
                            {
                                modifiedgame = true;

                                if (legacyofrust)
                                    D_AutoloadExtrasWAD();

                                if (IWADRequiredByPWAD(file) != none)
                                    D_SetString(&pwadfile, leafname(file));

                                LoadCfgFile(file);

                                if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex") && !D_IsDEHFile(file))
                                    LoadDEHFile(file, true);

                                if (W_GetNumLumps("M_DOOM") == 2 && W_GetNumLumps("E1M1") == 1)
                                {
                                    if (D_IsDOOM1IWAD(fullpath))
                                    {
                                        if (IsUltimateDOOM(fullpath))
                                        {
                                            W_Init();

                                            if (W_CheckNumForName("M_EPI5") < 0 && W_CheckNumForName("E5M1") < 0)
                                                D_AutoloadSIGILWAD();

                                            if (W_CheckNumForName("M_EPI6") < 0 && W_CheckNumForName("E6M1") < 0)
                                                D_AutoloadSIGIL2WAD();
                                        }
                                    }
                                    else if (D_IsDOOM2IWAD(fullpath) && W_GetNumLumps("MAP01") == 1)
                                    {
                                        D_AutoloadNerveWAD();
                                        D_AutoloadMasterLevelsWAD();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (BTSX)
                D_AutoloadOtherBTSXWAD();
            else if (KDIKDIZD)
                D_AutoloadOtherKDIKDIZDWAD();

            free(folder);
        }
        else
        {
            // more than one file was selected
            bool    isDOOM2 = false;
            bool    isUltimateDOOM = false;
            bool    sharewareiwad = false;

#if defined(_WIN32)
            LPSTR   iwadpass1 = ofn.lpstrFile;
            LPSTR   iwadpass2 = ofn.lpstrFile;
            LPSTR   pwadpass1 = ofn.lpstrFile;
            LPSTR   pwadpass2 = ofn.lpstrFile;
            LPSTR   cfgpass = ofn.lpstrFile;
            LPSTR   dehpass = ofn.lpstrFile;

            iwadpass1 = &iwadpass1[strlen(iwadpass1) + 1];

            // find and add IWAD first
            while (*iwadpass1)
            {
                char    fullpath[MAX_PATH];

                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, iwadpass1);

#elif defined(__APPLE__)
            char    *szFile;

            for (NSURL *url in urls)
            {
                char    *fullpath = (char *)[url fileSystemRepresentation];
                char    *iwadpass1 = (char *)[[url lastPathComponent] UTF8String];

                szFile = (char *)[[url URLByDeletingLastPathComponent] fileSystemRepresentation];
#endif

                if (D_IsDOOMIWAD(fullpath))
                {
                    D_IdentifyIWADByName(fullpath);

                    if (W_AddFile(fullpath, false))
                    {
                        iwadfound = 1;
                        sharewareiwad = M_StringCompare(iwadpass1, "DOOM1.WAD");
                        isDOOM2 = D_IsDOOM2IWAD(iwadpass1);
                        isUltimateDOOM = (D_IsDOOM1IWAD(fullpath) && IsUltimateDOOM(fullpath));

#if defined(_WIN32)
                        AddToWadList(leafname(fullpath));
#endif

                        wadfolder = M_ExtractFolder(fullpath);
                        break;
                    }
                }

#if defined(_WIN32)
                iwadpass1 = &iwadpass1[strlen(iwadpass1) + 1];
#endif
            }

#if defined(_WIN32)
            iwadpass2 = &iwadpass2[strlen(iwadpass2) + 1];

            // find and add IWAD first
            while (*iwadpass2)
            {
                char    fullpath[MAX_PATH];

                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, iwadpass2);

#elif defined(__APPLE__)
            for (NSURL *url in urls)
            {
                char    *fullpath = (char *)[url fileSystemRepresentation];
                char    *iwadpass2 = (char *)[[url lastPathComponent] UTF8String];

                szFile = (char *)[[url URLByDeletingLastPathComponent] fileSystemRepresentation];
#endif

                if (W_WadType(fullpath) == IWAD && !D_IsUnsupportedWAD(fullpath))
                {
                    if (!iwadfound)
                    {
                        D_IdentifyIWADByName(fullpath);

                        if (W_AddFile(fullpath, false))
                        {
                            iwadfound = 1;
                            sharewareiwad = M_StringCompare(iwadpass2, "DOOM1.WAD");
                            isDOOM2 = D_IsDOOM2IWAD(iwadpass2);
                            isUltimateDOOM = (D_IsDOOM1IWAD(fullpath) && IsUltimateDOOM(fullpath));

#if defined(_WIN32)
                            AddToWadList(leafname(fullpath));
#endif

                            wadfolder = M_ExtractFolder(fullpath);
                            break;
                        }
                    }
                    else if (!D_IsDOOMIWAD(fullpath))
                    {
                        if (W_MergeFile(fullpath, false))
                        {
                            modifiedgame = true;
                            break;
                        }
                    }
                }

#if defined(_WIN32)
                iwadpass2 = &iwadpass2[strlen(iwadpass2) + 1];
#endif
            }

            // merge any PWADs
            if (!sharewareiwad)
            {
                // if no IWAD has been selected, check each PWAD to determine the IWAD required
                // and then try to load it first
#if defined(_WIN32)
                pwadpass1 = &pwadpass1[strlen(pwadpass1) + 1];

                while (!iwadfound && *pwadpass1)
                {
                    char    fullpath[MAX_PATH];

                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, pwadpass1);

#elif defined(__APPLE__)
                for (NSURL *url in urls)
                {
                    char    *fullpath = (char *)[url fileSystemRepresentation];
                    char    *pwadpass1 = (char *)[[url lastPathComponent] UTF8String];

                    if (iwadfound)
                        break;
#endif

                    if (W_WadType(fullpath) == PWAD && !D_IsUnsupportedPWAD(fullpath) && !D_IsDEHFile(fullpath))
                    {
                        gamemission_t   iwadrequired = IWADRequiredByPWAD(fullpath);
                        char            *iwadname = D_RequiredIWAD(fullpath, NULL);

                        if (!iwadname && iwadrequired != none)
                            iwadname = iwadsrequired[iwadrequired];

                        if (iwadname)
                        {
                            char    fullpath2[MAX_PATH];

                            // try the current folder first
                            M_snprintf(fullpath2, sizeof(fullpath2), "%s" DIR_SEPARATOR_S "%s", szFile, iwadname);
                            D_IdentifyIWADByName(fullpath2);

                            if (W_AddFile(fullpath2, true))
                            {
                                iwadfound = 1;
                                wadfolder = M_ExtractFolder(fullpath2);
                            }
                            else
                            {
                                // otherwise try the wadfolder CVAR
                                M_snprintf(fullpath2, sizeof(fullpath2), "%s" DIR_SEPARATOR_S "%s", wadfolder, iwadname);
                                D_IdentifyIWADByName(fullpath2);

                                if (W_AddFile(fullpath2, true))
                                    iwadfound = 1;
                                else
                                {
                                    // still nothing? try some common installation folders
                                    if (W_AddFile(D_FindWADByName(iwadname), true))
                                        iwadfound = 1;
                                }
                            }
                        }
                    }

#if defined(_WIN32)
                    pwadpass1 = &pwadpass1[strlen(pwadpass1) + 1];
#endif
                }

                // if still no IWAD found, then try DOOM2.WAD
                if (!iwadfound)
                {
                    // try the current folder first
                    D_IdentifyIWADByName("DOOM2.WAD");

                    if (W_AddFile("DOOM2.WAD", true))
                        iwadfound = 1;
                    else
                    {
                        char    fullpath2[MAX_PATH];

                        // otherwise try the wadfolder CVAR
                        M_snprintf(fullpath2, sizeof(fullpath2), "%s" DIR_SEPARATOR_S "DOOM2.WAD", wadfolder);
                        D_IdentifyIWADByName(fullpath2);

                        if (W_AddFile(fullpath2, true))
                            iwadfound = 1;
                        else
                        {
                            // still nothing? try some common installation folders
                            if (W_AddFile(D_FindWADByName("DOOM2.WAD"), true))
                                iwadfound = 1;
                        }
                    }
                }

                // if an IWAD has now been found, make second pass through the PWADs to merge them
                if (iwadfound)
                {
                    bool    mapspresent = false;

#if defined(_WIN32)
                    pwadpass2 = &pwadpass2[strlen(pwadpass2) + 1];

                    while (*pwadpass2)
                    {
                        char    fullpath[MAX_PATH];

                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, pwadpass2);

#elif defined(__APPLE__)
                    for (NSURL *url in urls)
                    {
                        char    *fullpath = (char *)[url fileSystemRepresentation];
#endif

                        if (W_WadType(fullpath) == PWAD && !D_IsUnsupportedPWAD(fullpath) && !D_IsDEHFile(fullpath))
                        {
                            D_CheckSupportedPWAD(fullpath);

                            if (W_MergeFile(fullpath, false))
                            {
#if defined(_WIN32)
                                AddToWadList(leafname(fullpath));
#endif

                                modifiedgame = true;

                                if (legacyofrust)
                                    D_AutoloadExtrasWAD();

                                LoadCfgFile(fullpath);

                                if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex") && !D_IsDEHFile(fullpath))
                                    LoadDEHFile(fullpath, true);

                                if (PWADHasMaps(fullpath))
                                {
                                    mapspresent = true;

                                    if (IWADRequiredByPWAD(fullpath) != none)
                                        D_SetString(&pwadfile, leafname(fullpath));
                                }
                            }
                        }

#if defined(_WIN32)
                        pwadpass2 = &pwadpass2[strlen(pwadpass2) + 1];
#endif
                    }

                    if (!mapspresent)
                    {
                        if (isUltimateDOOM)
                        {
                            D_AutoloadSIGILWAD();
                            D_AutoloadSIGIL2WAD();
                        }
                        else if (isDOOM2)
                        {
                            D_AutoloadNerveWAD();
                            D_AutoloadMasterLevelsWAD();
                        }
                    }

#if defined(_WIN32)
                    // process any config files
                    cfgpass = &cfgpass[strlen(cfgpass) + 1];

                    while (*cfgpass)
                    {
                        char    fullpath[MAX_PATH];

                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, cfgpass);

#elif defined(__APPLE__)
                    for (NSURL *url in urls)
                    {
                        char    *fullpath = (char *)[url fileSystemRepresentation];
#endif

                        if (D_IsCFGFile(fullpath))
                            M_LoadCVARs(fullpath);

#if defined(_WIN32)
                        cfgpass = &cfgpass[strlen(cfgpass) + 1];
#endif
                    }

#if defined(_WIN32)
                    // process any DeHackEd files last of all
                    dehpass = &dehpass[strlen(dehpass) + 1];

                    while (*dehpass)
                    {
                        char    fullpath[MAX_PATH];

                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, dehpass);

#elif defined(__APPLE__)
                    for (NSURL *url in urls)
                    {
                        char    *fullpath = (char *)[url fileSystemRepresentation];
#endif

                        if (D_IsDEHFile(fullpath))
                        {
#if defined(_WIN32)
                            AddToWadList(leafname(fullpath));
#endif
                            LoadDEHFile(fullpath, false);
                        }

#if defined(_WIN32)
                        dehpass = &dehpass[strlen(dehpass) + 1];
#endif
                    }

                    if (BTSX)
                        D_AutoloadOtherBTSXWAD();
                    else if (KDIKDIZD)
                        D_AutoloadOtherKDIKDIZDWAD();
                }
            }
            else
                I_Error("Other files can’t be loaded with the shareware version of DOOM.");
        }
    }

#if defined(_WIN32)
    if (collected_wads)
    {
        if (numcollectedwads == 1)
            M_StripQuotes(collected_wads);

        D_SetString(&wad, collected_wads);
        free(collected_wads);
        collected_wads = NULL;
        numcollectedwads = 0;
    }
#endif

    return iwadfound;
}
#endif
