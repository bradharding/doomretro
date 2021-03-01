/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2021 by Brad Harding.

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

#if defined(_WIN32)
#include <Windows.h>
#include <ShellAPI.h>

#include "SDL_syswm.h"
#endif

#include "d_main.h"
#include "i_gamepad.h"
#include "m_argv.h"
#include "m_config.h"
#include "version.h"

int windowborderwidth = 0;
int windowborderheight = 0;

#if defined(_WIN32)

#if !defined(SM_CXPADDEDBORDER)
#define SM_CXPADDEDBORDER   92
#endif

static void I_SetProcessDPIAware(void)
{
    HMODULE hLibrary = LoadLibrary("user32.dll");

    if (hLibrary)
    {
        typedef BOOL    (*SETPROCESSDPIAWARE)(void);

        SETPROCESSDPIAWARE  pSetProcessDPIAware = (SETPROCESSDPIAWARE)GetProcAddress(hLibrary, "SetProcessDPIAware");

        if (pSetProcessDPIAware)
            pSetProcessDPIAware();

        FreeLibrary(hLibrary);
    }
}

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
        if ((wParam & 0xFFF0) == SC_MAXIMIZE)
        {
            I_ToggleFullscreen();
            return true;
        }
        else if ((wParam & 0xFFF0) == SC_KEYMENU)
            return false;
    }
    else if (msg == WM_SYSKEYDOWN)
    {
        if (wParam == VK_RETURN && !(lParam & 0x40000000))
        {
            I_ToggleFullscreen();
            return true;
        }
    }
    else if (msg == WM_DEVICECHANGE)
    {
        I_ShutdownGamepad();
        I_InitGamepad();
    }
    else if (msg == WM_SIZE)
    {
        if (!vid_fullscreen)
            I_WindowResizeBlit();
    }
    else if (msg == WM_GETMINMAXINFO)
    {
        LPMINMAXINFO    minmaxinfo = (LPMINMAXINFO)lParam;

        minmaxinfo->ptMinTrackSize.x = VANILLAWIDTH + windowborderwidth;
        minmaxinfo->ptMinTrackSize.y = VANILLAWIDTH * 3 / 4 + windowborderheight;

        return false;
    }

    return CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
}

static HANDLE   hInstanceMutex;

void I_InitWindows32(void)
{
    HINSTANCE       handle = GetModuleHandle(NULL);
    SDL_SysWMinfo   info;
    HWND            hwnd;

    SDL_VERSION(&info.version);

    SDL_GetWindowWMInfo(window, &info);
    hwnd = info.info.win.window;

    icon = LoadIcon(handle, "IDI_ICON1");
    SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)icon);

    oldProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    windowborderwidth = (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER)) * 2;
    windowborderheight = (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER)) * 2 + GetSystemMetrics(SM_CYCAPTION);
}

void I_ShutdownWindows32(void)
{
    DestroyIcon(icon);
    ReleaseMutex(hInstanceMutex);
    CloseHandle(hInstanceMutex);
}
#endif

int main(int argc, char **argv)
{
    myargc = argc;
    myargv = argv;

#if defined(_WIN32)
    hInstanceMutex = CreateMutex(NULL, true, PACKAGE_MUTEX);

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        if (hInstanceMutex)
            CloseHandle(hInstanceMutex);

        SetForegroundWindow(FindWindow(PACKAGE_MUTEX, NULL));
        return 1;
    }

    I_SetProcessDPIAware();
#endif

    D_DoomMain();

    return 0;
}
