/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "SDL.h"

#include "doomdef.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_main.h"

typedef BOOL (WINAPI *SetAffinityFunc)(HANDLE hProcess, DWORD mask);

// This is a bit more complicated than it really needs to be.  We really
// just need to call the SetProcessAffinityMask function, but that
// function doesn't exist on systems before Windows 2000.  Instead,
// dynamically look up the function and call the pointer to it.  This
// way, the program will run on older versions of Windows (Win9x, etc.)

static void LockCPUAffinity(void)
{
    HMODULE kernel32_dll;
    SetAffinityFunc SetAffinity;

    // Find the kernel interface DLL.

    kernel32_dll = LoadLibrary("kernel32.dll");

    if (kernel32_dll == NULL)
    {
        // This should never happen...

        fprintf(stderr, "Failed to load kernel32.dll\n");
        return;
    }

    // Find the SetProcessAffinityMask function.

    SetAffinity = (SetAffinityFunc)GetProcAddress(kernel32_dll, "SetProcessAffinityMask");

    // If the function was not found, we are on an old (Win9x) system
    // that doesn't have this function.  That's no problem, because
    // those systems don't support SMP anyway.

    if (SetAffinity != NULL)
    {
        if (!SetAffinity(GetCurrentProcess(), 1))
        {
            fprintf(stderr, "Failed to set process affinity (%d)\n",
                            (int) GetLastError());
        }
    }
}

extern int fullscreen;
extern boolean window_focused;
HHOOK g_hKeyboardHook;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    bool bEatKeystroke = false;
    KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

    if (nCode < 0 || nCode != HC_ACTION)
        return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);

    switch (wParam)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            bEatKeystroke = (fullscreen && window_focused &&
                ((p->vkCode == VK_LWIN) || (p->vkCode == VK_RWIN)));
            break;
        }
    }

    if (bEatKeystroke)
        return 1;
    else
        return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}


WNDPROC oldProc;
HICON icon;
HWND hwnd;

boolean MouseShouldBeGrabbed(void);
void ToggleFullScreen(void);
void I_InitGamepad(void);

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
    else if (msg == WM_SYSCOMMAND && (wParam & 0xfff0) == SC_KEYMENU)
        return false;
    else if (msg == WM_SYSKEYDOWN && wParam == VK_RETURN && !(lParam & 0x40000000))
    {
        ToggleFullScreen();
        return true;
    }
    else if (msg == WM_DEVICECHANGE)
        I_InitGamepad();

    return CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
}

#include "SDL_syswm.h"

void init_win32(LPCTSTR lpIconName)
{
    HINSTANCE handle = GetModuleHandle(NULL);

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version)
    if (SDL_GetWMInfo(&wminfo) != 1)
    {
        // error: wrong SDL version
    }
    hwnd = wminfo.window;

    icon = LoadIcon(handle, lpIconName);
    SetClassLong(hwnd, GCL_HICON, (LONG) icon);

    oldProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)WndProc);
}

void done_win32(void)
{
    DestroyIcon(icon);
    UnhookWindowsHookEx(g_hKeyboardHook);
}

int main(int argc, char **argv)
{
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

    // save arguments

    myargc = argc;
    myargv = argv;

    // Only schedule on a single core, if we have multiple
    // cores.  This is to work around a bug in SDL_mixer.

    LockCPUAffinity();

    // start doom

    D_DoomMain();
}
