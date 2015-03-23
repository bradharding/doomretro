/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void I_ShutdownWindows32(void);
#else
#include <unistd.h>
#endif

#if !defined(SDL20) && defined(__MACOSX__)
#include <CoreFoundation/CFUserNotification.h>
#endif

#include "c_console.h"
#include "d_net.h"
#include "doomdef.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "s_sound.h"
#include "SDL.h"
#include "version.h"
#include "w_merge.h"
#include "w_wad.h"
#include "z_zone.h"

extern boolean  widescreen;
extern boolean  hud;
extern boolean  returntowidescreen;

#if defined(WIN32)
typedef long(__stdcall *PRTLGETVERSION)(PRTL_OSVERSIONINFOEXW);
typedef BOOL(WINAPI *PGETPRODUCTINFO)(DWORD, DWORD, DWORD, DWORD, PDWORD);

#define PRODUCT_PROFESSIONAL    0x00000030

void I_PrintWindowsVersion(void)
{
    PRTLGETVERSION      pRtlGetVersion = (PRTLGETVERSION)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlGetVersion");
    PGETPRODUCTINFO     pGetProductInfo = (PGETPRODUCTINFO)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProductInfo");
    OSVERSIONINFOEXW    info;
    const char          *infoname;
    DWORD               type;
    const char          *typename = "";

    if (pRtlGetVersion && pGetProductInfo)
    {
        ZeroMemory(&info, sizeof(&info));
        info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

        pRtlGetVersion((PRTL_OSVERSIONINFOEXW)&info);

        pGetProductInfo(info.dwMajorVersion, info.dwMinorVersion, 0, 0, &type);
        switch (type)
        {
            case PRODUCT_ULTIMATE:                     typename = "Ultimate";                      break;
            case PRODUCT_PROFESSIONAL:                 typename = "Professional";                  break;
            case PRODUCT_HOME_PREMIUM:                 typename = "Home Premium";                  break;
            case PRODUCT_HOME_BASIC:                   typename = "Home Basic";                    break;
            case PRODUCT_ENTERPRISE:                   typename = "Enterprise";                    break;
            case PRODUCT_BUSINESS:                     typename = "Business";                      break;
            case PRODUCT_STARTER:                      typename = "Starter";                       break;
            case PRODUCT_CLUSTER_SERVER:               typename = "Cluster Server";                break;
            case PRODUCT_DATACENTER_SERVER:
            case PRODUCT_DATACENTER_SERVER_CORE:       typename = "Datacenter Edition";            break;
            case PRODUCT_ENTERPRISE_SERVER:
            case PRODUCT_ENTERPRISE_SERVER_CORE:
            case PRODUCT_ENTERPRISE_SERVER_IA64:       typename = "Enterprise";                    break;
            case PRODUCT_SMALLBUSINESS_SERVER:         typename = "Small Business Server";         break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM: typename = "Small Business Server Premium"; break;
            case PRODUCT_STANDARD_SERVER:
            case PRODUCT_STANDARD_SERVER_CORE:         typename = "Standard";                      break;
            case PRODUCT_WEB_SERVER:                   typename = "Web Server";                    break;
        }

        if (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            infoname = (info.dwMinorVersion < 10 ? "95" : (info.dwMinorVersion < 90 ? "98" : "Me"));

            C_Output("Running on Microsoft Windows %s%s%s%s%ws%s.",
                infoname, (strlen(typename) ? " " : ""), (strlen(typename) ? typename : ""),
                (wcslen(info.szCSDVersion) ? " (" : ""),
                (wcslen(info.szCSDVersion) ? info.szCSDVersion : L""),
                (wcslen(info.szCSDVersion) ? ")" : ""));
        }
        else if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            infoname = "NT";
            if (info.dwMajorVersion == 5)
            {
                if (info.dwMinorVersion == 0)
                    infoname = "2000";
                else if (info.dwMinorVersion == 1)
                    infoname = "XP";
                else if (info.dwMinorVersion == 2)
                    infoname = "Server 2003";
            }
            else if (info.dwMajorVersion == 6)
            {
                if (info.dwMinorVersion == 0)
                    infoname = (info.wProductType == VER_NT_WORKSTATION ? "Vista" : "Server 2008");
                else if (info.dwMinorVersion == 1)
                    infoname = (info.wProductType == VER_NT_WORKSTATION ? "7" : "Server 2008 R2");
                else if (info.dwMinorVersion == 2)
                    infoname = (info.wProductType == VER_NT_WORKSTATION ? "8" : "Server 2012");
                else if (info.dwMinorVersion == 3)
                    infoname = "8.1";
            }
            else if (info.dwMajorVersion == 10 && info.dwMinorVersion == 0)
                infoname = "10";

            C_Output("Running on Microsoft Windows %s%s%s%s%ws%s.",
                infoname, (strlen(typename) ? " " : ""), (strlen(typename) ? typename : ""),
                (wcslen(info.szCSDVersion) ? " (" : ""),
                (wcslen(info.szCSDVersion) ? info.szCSDVersion : L""),
                (wcslen(info.szCSDVersion) ? ")" : ""));
        }
    }
}
#endif

//
// I_Quit
//
void I_Quit(boolean shutdown)
{
    if (shutdown)
    {
        S_Shutdown();

#if !defined(SDL20)
        I_SaveWindowPosition();
#endif

        if (returntowidescreen)
            widescreen = true;

        M_SaveDefaults();

        I_ShutdownGraphics();

        I_ShutdownKeyboard();

        I_ShutdownGamepad();
    }

#if defined(WIN32)
    I_ShutdownWindows32();
#endif

    SDL_Quit();

    exit(0);
}

void I_WaitVBL(int count)
{
    I_Sleep((count * 1000) / 70);
}

#if !defined(SDL20) && !defined(WIN32) && !defined(__MACOSX__)

#define ZENITY_BINARY "/usr/bin/zenity"

// returns non-zero if zenity is available
static int ZenityAvailable(void)
{
    return system(ZENITY_BINARY " --help >/dev/null 2>&1") == 0;
}

// Escape special characters in the given string so that they can be
// safely enclosed in shell quotes.
static char *EscapeShellString(char *string)
{
    char        *result;
    char        *r, *s;

    // In the worst case, every character might be escaped.
    result = malloc(strlen(string) * 2 + 3);
    r = result;

    // Enclosing quotes.
    *r = '"';
    ++r;

    for (s = string; *s != '\0'; ++s)
    {
        // From the bash manual:
        //
        //  "Enclosing characters in double quotes preserves the literal
        //   value of all characters within the quotes, with the exception
        //   of $, `, \, and, when history expansion is enabled, !."
        //
        // Therefore, escape these characters by prefixing with a backslash.
        if (strchr("$`\\!", *s) != NULL)
        {
            *r = '\\';
            ++r;
        }

        *r = *s;
        ++r;
    }

    // Enclosing quotes.
    *r = '"';
    ++r;
    *r = '\0';

    return result;
}

// Open a native error box with a message using zenity
static int ZenityErrorBox(char *message)
{
    int                 result;
    char                *escaped_message;
    char                *errorboxpath;
    static size_t       errorboxpath_size;

    if (!ZenityAvailable())
        return 0;

    escaped_message = EscapeShellString(message);

    errorboxpath_size = strlen(ZENITY_BINARY) + strlen(escaped_message) + 19;
    errorboxpath = malloc(errorboxpath_size);
    M_snprintf(errorboxpath, errorboxpath_size, "%s --error --text=%s",
        ZENITY_BINARY, escaped_message);

    result = system(errorboxpath);

    free(errorboxpath);
    free(escaped_message);

    return result;
}
#endif

//
// I_Error
//
static boolean already_quitting = false;

void I_Error(char *error, ...)
{
    va_list     argptr;
    char        msgbuf[512];

#if !defined(SDL20) && defined(WIN32)
    wchar_t     wmsgbuf[512];
#endif

    if (already_quitting)
        exit(-1);
    else
        already_quitting = true;

    // Shutdown. Here might be other errors.
    S_Shutdown();

#if !defined(SDL20)
    I_SaveWindowPosition();
#endif

    if (returntowidescreen)
        widescreen = true;

    I_ShutdownGraphics();

    I_ShutdownKeyboard();

    I_ShutdownGamepad();

    va_start(argptr, error);
    memset(msgbuf, 0, sizeof(msgbuf));
    M_vsnprintf(msgbuf, sizeof(msgbuf) - 1, error, argptr);
    va_end(argptr);

#if defined(SDL20)
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PACKAGE_NAME, msgbuf, NULL);
#elif defined(WIN32)
    MultiByteToWideChar(CP_ACP, 0, msgbuf, strlen(msgbuf) + 1, wmsgbuf, sizeof(wmsgbuf));

    MessageBoxW(NULL, wmsgbuf, PACKAGE_NAME_W, MB_ICONERROR | MB_OK);

    I_ShutdownWindows32();

#elif defined(__MACOSX__)
    {
        CFStringRef     message;
        int             i;

        // The CoreFoundation message box wraps text lines, so replace
        // newline characters with spaces so that multiline messages
        // are continuous.

        for (i = 0; msgbuf[i] != '\0'; ++i)
            if (msgbuf[i] == '\n')
                msgbuf[i] = ' ';

        message = CFStringCreateWithCString(NULL, msgbuf, kCFStringEncodingUTF8);

        CFUserNotificationDisplayNotice(0, kCFUserNotificationCautionAlertLevel, NULL, NULL, NULL,
            CFSTR(PACKAGE_NAME), message, NULL);
    }
#else
    ZenityErrorBox(msgbuf);
#endif

    SDL_Quit();

    exit(-1);
}
