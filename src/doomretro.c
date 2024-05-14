/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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
#include <Windows.h>

#include "SDL_syswm.h"
#endif

#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"

#if defined(_WIN32)
static WNDPROC  oldProc;
static HICON    icon;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_SETCURSOR)
    {
        if (LOWORD(lParam) == HTCLIENT && !MouseShouldBeGrabbed())
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return true;
        }
    }
    else if (msg == WM_SYSCOMMAND)
    {
        if ((wParam & 0xFFF0) == SC_MAXIMIZE && !splashscreen)
        {
            I_ToggleFullscreen(true);
            return true;
        }
        else if ((wParam & 0xFFF0) == SC_KEYMENU)
            return false;
    }
    else if (msg == WM_SYSKEYDOWN)
    {
        if (wParam == VK_RETURN && !(lParam & 0x40000000) && !splashscreen)
        {
            I_ToggleFullscreen(true);
            return true;
        }
    }
    else if (msg == WM_SIZE)
    {
        if (!vid_fullscreen)
            I_WindowResizeBlit();
    }
    else if (msg == WM_GETMINMAXINFO)
    {
        LPMINMAXINFO    minmaxinfo = (LPMINMAXINFO)lParam;

        minmaxinfo->ptMinTrackSize.x = (vid_widescreen ? WIDEVANILLAWIDTH : VANILLAWIDTH) + windowborderwidth;
        minmaxinfo->ptMinTrackSize.y = ACTUALVANILLAHEIGHT + windowborderheight;

        return false;
    }

    return CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
}

static HANDLE       hInstanceMutex;

static STICKYKEYS   g_StartupStickyKeys = { sizeof(STICKYKEYS), 0 };
static TOGGLEKEYS   g_StartupToggleKeys = { sizeof(TOGGLEKEYS), 0 };
static FILTERKEYS   g_StartupFilterKeys = { sizeof(FILTERKEYS), 0 };

static void I_AccessibilityShortcutKeys(bool bAllowKeys)
{
    if (bAllowKeys)
    {
        // Restore StickyKeys/etc to original state
        SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
        SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
        SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
    }
    else
    {
        // Disable StickyKeys/etc shortcuts
        STICKYKEYS  skOff = g_StartupStickyKeys;
        TOGGLEKEYS  tkOff = g_StartupToggleKeys;
        FILTERKEYS  fkOff = g_StartupFilterKeys;

        if (!(skOff.dwFlags & SKF_STICKYKEYSON))
        {
            // Disable the hotkey and the confirmation
            skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
            skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

            SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
        }

        if (!(tkOff.dwFlags & TKF_TOGGLEKEYSON))
        {
            // Disable the hotkey and the confirmation
            tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
            tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

            SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
        }

        if (!(fkOff.dwFlags & FKF_FILTERKEYSON))
        {
            // Disable the hotkey and the confirmation
            fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
            fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

            SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
        }
    }
}

void I_InitWindows32(void)
{
    HINSTANCE       handle = GetModuleHandle(NULL);
    SDL_SysWMinfo   info = { 0 };
    HWND            hwnd;

    SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

    SDL_VERSION(&info.version);

    SDL_GetWindowWMInfo(window, &info);
    hwnd = info.info.win.window;

    icon = LoadIcon(handle, "IDI_ICON1");
    SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)icon);

    oldProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    windowborderwidth = (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER)) * 2;
    windowborderheight = (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER)) * 2
        + GetSystemMetrics(SM_CYCAPTION);
}

void I_ShutdownWindows32(void)
{
    DestroyIcon(icon);
    ReleaseMutex(hInstanceMutex);
    CloseHandle(hInstanceMutex);
    I_AccessibilityShortcutKeys(true);
}
#endif

int main(int argc, char **argv)
{
    myargc = argc;

    if ((myargv = (char **)malloc(myargc * sizeof(myargv[0]))))
    {
        memcpy(myargv, argv, myargc * sizeof(myargv[0]));

        for (int i = 0; i < myargc; i++)
            M_NormalizeSlashes(myargv[i]);
    }

    M_FindResponseFile();

#if defined(_WIN32)
    hInstanceMutex = CreateMutex(NULL, true, DOOMRETRO_MUTEX);

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        if (hInstanceMutex)
            CloseHandle(hInstanceMutex);

        SetForegroundWindow(FindWindow(DOOMRETRO_MUTEX, NULL));
        return 1;
    }

    // Save the current sticky/toggle/filter key settings so they can be restored later
    SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
    SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
    SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);

    I_AccessibilityShortcutKeys(false);
#endif

    D_DoomMain();

    return 0;
}
