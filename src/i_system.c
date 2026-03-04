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
#include <Windows.h>
#include <DbgHelp.h>
#else
#include <unistd.h>
#endif

#include "c_console.h"
#include "d_main.h"
#include "doomstat.h"
#include "i_controller.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_setup.h"
#include "s_sound.h"
#include "version.h"
#include "w_wad.h"

#if defined(_WIN32)
#pragma comment(lib, "Dbghelp.lib")

#define PRODUCT_CORE        0x00000065
#define PRODUCT_EDUCATION   0x00000079

typedef long    (__stdcall *PRTLGETVERSION)(PRTL_OSVERSIONINFOEXW);
typedef BOOL    (WINAPI *PGETPRODUCTINFO)(DWORD, DWORD, DWORD, DWORD, PDWORD);
typedef BOOL    (WINAPI *PISWOW64PROCESS)(HANDLE, PBOOL);

static char windowsname[128];
static char windowsbuild[32];

void I_PrintWindowsVersion(void)
{
    PRTLGETVERSION  pRtlGetVersion = (PRTLGETVERSION)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlGetVersion");
    PGETPRODUCTINFO pGetProductInfo = (PGETPRODUCTINFO)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProductInfo");
    PISWOW64PROCESS pIsWow64Process = (PISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32.dll"), "IsWow64Process");

    if (pRtlGetVersion && pGetProductInfo)
    {
        int                 bits = 32;
        char                typename[32] = "";
        OSVERSIONINFOEXW    info;
        DWORD               type;

        if (pIsWow64Process)
        {
            BOOL    Wow64Process = FALSE;

            pIsWow64Process(GetCurrentProcess(), &Wow64Process);

            if (Wow64Process || sizeof(intptr_t) == 8)
                bits = 64;
        }

        ZeroMemory(&info, sizeof(OSVERSIONINFOEXW));
        info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

        pRtlGetVersion((PRTL_OSVERSIONINFOEXW)&info);

        pGetProductInfo(info.dwMajorVersion, info.dwMinorVersion, 0, 0, &type);

        switch (type)
        {
            case PRODUCT_ULTIMATE:
                M_StringCopy(typename, "Ultimate", sizeof(typename));
                break;

            case PRODUCT_PROFESSIONAL:
                M_StringCopy(typename, "Professional", sizeof(typename));
                break;

            case PRODUCT_HOME_PREMIUM:
                M_StringCopy(typename, "Home Premium", sizeof(typename));
                break;

            case PRODUCT_HOME_BASIC:
                M_StringCopy(typename, "Home Basic", sizeof(typename));
                break;

            case PRODUCT_ENTERPRISE:
            case PRODUCT_ENTERPRISE_SERVER:
            case PRODUCT_ENTERPRISE_SERVER_CORE:
            case PRODUCT_ENTERPRISE_SERVER_IA64:
                M_StringCopy(typename, "Enterprise", sizeof(typename));
                break;

            case PRODUCT_BUSINESS:
                M_StringCopy(typename, "Business", sizeof(typename));
                break;

            case PRODUCT_STARTER:
                M_StringCopy(typename, "Starter", sizeof(typename));
                break;

            case PRODUCT_CLUSTER_SERVER:
                M_StringCopy(typename, "Cluster Server", sizeof(typename));
                break;

            case PRODUCT_DATACENTER_SERVER:
            case PRODUCT_DATACENTER_SERVER_CORE:
                M_StringCopy(typename, "Datacenter Edition", sizeof(typename));
                break;

            case PRODUCT_SMALLBUSINESS_SERVER:
                M_StringCopy(typename, "Small Business Server", sizeof(typename));
                break;

            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                M_StringCopy(typename, "Small Business Server Premium", sizeof(typename));
                break;

            case PRODUCT_STANDARD_SERVER:
            case PRODUCT_STANDARD_SERVER_CORE:
                M_StringCopy(typename, "Standard", sizeof(typename));
                break;

            case PRODUCT_WEB_SERVER:
                M_StringCopy(typename, "Web Server", sizeof(typename));
                break;

            case PRODUCT_CORE:
                M_StringCopy(typename, "Home", sizeof(typename));
                break;
        }

        if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            char    infoname[32] = "NT";
            char    *build = commify(info.dwBuildNumber);

            M_snprintf(windowsbuild, sizeof(windowsbuild), "(Build %s)", build);

            if (info.dwMajorVersion == 5)
            {
                if (info.dwMinorVersion == 0)
                    M_StringCopy(infoname, "2000", sizeof(infoname));
                else if (info.dwMinorVersion == 1)
                    M_StringCopy(infoname, "XP", sizeof(infoname));
                else if (info.dwMinorVersion == 2)
                    M_StringCopy(infoname, "Server 2003", sizeof(infoname));
            }
            else if (info.dwMajorVersion == 6)
            {
                if (info.dwMinorVersion == 0)
                    M_StringCopy(infoname, (info.wProductType == VER_NT_WORKSTATION ? "Vista" : "Server 2008"), sizeof(infoname));
                else if (info.dwMinorVersion == 1)
                    M_StringCopy(infoname, (info.wProductType == VER_NT_WORKSTATION ? "7" : "Server 2008 R2"), sizeof(infoname));
                else if (info.dwMinorVersion == 2)
                    M_StringCopy(infoname, (info.wProductType == VER_NT_WORKSTATION ? "8" : "Server 2012"), sizeof(infoname));
                else if (info.dwMinorVersion == 3)
                    M_StringCopy(infoname, "8.1", sizeof(infoname));
            }
            else if (info.dwMajorVersion == 10)
            {
                if (info.dwBuildNumber < 22000)
                    M_StringCopy(infoname, (info.wProductType == VER_NT_WORKSTATION ? "10" : "Server 2016"), sizeof(infoname));
                else
                {
                    M_StringCopy(infoname, (info.wProductType == VER_NT_WORKSTATION ? "11" : "Server 2022"), sizeof(infoname));

                    if (!*typename)
                        switch (type)
                        {
                            case PRODUCT_PROFESSIONAL:
                                M_StringCopy(typename, "Pro", sizeof(typename));
                                break;

                            case PRODUCT_ENTERPRISE:
                                M_StringCopy(typename, "Enterprise", sizeof(typename));
                                break;

                            case PRODUCT_EDUCATION:
                                M_StringCopy(typename, "Education", sizeof(typename));
                                break;

                            case PRODUCT_CORE:
                                M_StringCopy(typename, "Home", sizeof(typename));
                                break;
                        }
                }
            }

            if (wcslen(info.szCSDVersion) > 0)
            {
                M_snprintf(windowsname, sizeof(windowsname), "Microsoft Windows %s%s%s (%ws)",
                    infoname, (*typename ? " " : ""), typename, info.szCSDVersion);
                C_Output("It is running on the %i-bit edition of " ITALICS("%s") " %s.",
                    bits, windowsname, windowsbuild);
            }
            else if (info.dwMajorVersion < 10 || info.dwBuildNumber < 22000)
            {
                M_snprintf(windowsname, sizeof(windowsname), "Microsoft Windows %s%s%s",
                    infoname, (*typename ? " " : ""), typename);
                C_Output("It is running on the %i-bit edition of " ITALICS("%s") " %s.",
                    bits, windowsname, windowsbuild);
            }
            else
            {
                M_snprintf(windowsname, sizeof(windowsname), "Microsoft Windows %s%s%s",
                    infoname, (*typename ? " " : ""), typename);
                C_Output("It is running on " ITALICS("%s") " %s.", windowsname, windowsbuild);
            }

            free(build);
        }

        if (bits == 64 && sizeof(intptr_t) == 4)
            C_Warning(1, "The 64-bit version of " ITALICS(DOOMRETRO_NAME) " is recommended on this PC.");
    }
}

static LONG WINAPI I_ExceptionHandler(EXCEPTION_POINTERS *exceptionInfo)
{
    char        crashfolder[MAX_PATH];
    char        dumppath[MAX_PATH];
    char        logpath[MAX_PATH];
    char        message[1024];
    HANDLE      hDumpFile;
    time_t      now = time(NULL);
    struct tm   *tm_info = localtime(&now);
    char        timestamp[64];
    char        readabletimestamp[64];
    FILE        *logfile;

    // Create timestamp
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", tm_info);

    // Create crash dump filename
    M_snprintf(crashfolder, sizeof(crashfolder), "%s" DIR_SEPARATOR_S DOOMRETRO_CRASHFOLDER,
        M_GetAppDataFolder());
    M_MakeDirectory(crashfolder);
    M_snprintf(dumppath, sizeof(dumppath), "%s" DIR_SEPARATOR_S "%s.dmp", crashfolder, timestamp);
    M_snprintf(logpath, sizeof(logpath), "%s" DIR_SEPARATOR_S "%s.txt",
        crashfolder, timestamp);

    // Write minidump
    hDumpFile = CreateFile(dumppath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION  mdei;

        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = exceptionInfo;
        mdei.ClientPointers = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile,
            MiniDumpWithFullMemory, &mdei, NULL, NULL);
        CloseHandle(hDumpFile);
    }

    // Write crash log with system info
    if ((logfile = fopen(logpath, "w")))
    {
        char        exepath[MAX_PATH];
        DWORD       exceptioncode = exceptionInfo->ExceptionRecord->ExceptionCode;
        const char  *exceptionname = "Unknown Exception";

        GetModuleFileName(NULL, exepath, sizeof(exepath));
        fprintf(logfile, "File:              %s\n", exepath);
        fprintf(logfile, "Version:           %s\n", DOOMRETRO_VERSIONSTRING);

        M_snprintf(readabletimestamp, sizeof(readabletimestamp), "%s, %s %d, %d at %d:%02d:%02d%s",
            daynames[tm_info->tm_wday],
            monthnames[tm_info->tm_mon],
            tm_info->tm_mday,
            1900 + tm_info->tm_year,
            (!(tm_info->tm_hour % 12) ? 12 : (tm_info->tm_hour % 12)),
            tm_info->tm_min,
            tm_info->tm_sec,
            (tm_info->tm_hour < 12 ? "am" : "pm"));
        fprintf(logfile, "Date/Time:         %s\n", readabletimestamp);

        switch (exceptioncode)
        {
            case EXCEPTION_ACCESS_VIOLATION:
                exceptionname = "Access Violation";
                break;

            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
                exceptionname = "Array Bounds Exceeded";
                break;

            case EXCEPTION_BREAKPOINT:
                exceptionname = "Breakpoint";
                break;

            case EXCEPTION_DATATYPE_MISALIGNMENT:
                exceptionname = "Datatype Misalignment";
                break;

            case EXCEPTION_FLT_DIVIDE_BY_ZERO:
                exceptionname = "Floating Point Division by Zero";
                break;

            case EXCEPTION_FLT_OVERFLOW:
                exceptionname = "Floating Point Overflow";
                break;

            case EXCEPTION_ILLEGAL_INSTRUCTION:
                exceptionname = "Illegal Instruction";
                break;

            case EXCEPTION_INT_DIVIDE_BY_ZERO:
                exceptionname = "Integer Division by Zero";
                break;

            case EXCEPTION_INT_OVERFLOW:
                exceptionname = "Integer Overflow";
                break;

            case EXCEPTION_STACK_OVERFLOW:
                exceptionname = "Stack Overflow";
                break;
        }

        fprintf(logfile, "Exception:         %s (0x%08lX)\n", exceptionname, exceptioncode);

        fprintf(logfile, "Exception Address: 0x%p\n",
            exceptionInfo->ExceptionRecord->ExceptionAddress);

        if (windowsname[0] && windowsbuild[0])
            fprintf(logfile, "Operating System:  %s %s\n", windowsname, windowsbuild);

        if (wadsloaded[0])
            fprintf(logfile, "WAD%s              %s\n",
                (strchr(wadsloaded, ',') ? "s:" : ": "), wadsloaded);

        if (gamestate == GS_LEVEL)
        {
            if (mapnum[0])
                fprintf(logfile, "Map                %s\n", mapnum);

            if (viewplayer && viewplayer->mo)
                fprintf(logfile, "Player Position    (%d, %d, %d)\n",
                    viewplayer->mo->x >> FRACBITS, viewplayer->mo->y >> FRACBITS, viewplayer->mo->z >> FRACBITS);
        }

        fclose(logfile);
    }

    // Show user dialog
    M_snprintf(message, sizeof(message),
        "%s has crashed unexpectedly.\n\n"
        "Information about this crash has been saved to:\n"
        "%s\n"
        "%s\n\n"
        "Please send these files to %s.\n",
        DOOMRETRO_NAME, dumppath, logpath, DOOMRETRO_CREATORANDEMAIL);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, DOOMRETRO_NAME " crashed!", message, NULL);

    return EXCEPTION_EXECUTE_HANDLER;
}

void I_InitCrashHandler(void)
{
    SetUnhandledExceptionFilter(I_ExceptionHandler);
}
#endif

void I_PrintSystemInfo(void)
{
    const int   cores = SDL_GetCPUCount();
    char        *RAM = commify(SDL_GetSystemRAM() / 1000);

    C_Output("There %s %i core%s and %sGB of RAM on this " DEVICE ".",
        (cores == 1 ? "is" : "are"), cores, (cores == 1 ? "" : "s"), RAM);
    free(RAM);
}

//
// I_Quit
//
void I_Quit(bool shutdown)
{
    if (shutdown)
    {
        D_FadeScreenToBlack();

        S_Shutdown();

        M_SaveCVARs();

        I_ShutdownKeyboard();
        I_ShutdownController();
        I_ShutdownGraphics();
        SDL_Quit();
    }

    W_CloseFiles();

#if defined(_WIN32)
    I_ShutdownWindows();
#endif

    exit(0);
}

//
// I_Error
//
void I_Error(const char *error, ...)
{
    va_list     args;
    char        buffer[512];
    static bool already_quitting;

    if (already_quitting)
        exit(-1);

    already_quitting = true;

    // Shutdown. Here might be other errors.
    S_Shutdown();

#if defined(_WIN32)
    if (previouswad && gamestate <= GS_TITLESCREEN)
        wad = M_StringDuplicate(previouswad);
#endif

    M_SaveCVARs();

    I_ShutdownKeyboard();
    I_ShutdownController();
    I_ShutdownGraphics();

    W_CloseFiles();

#if defined(_WIN32)
    I_ShutdownWindows();
#endif

    va_start(args, error);
    vfprintf(stderr, error, args);
    fprintf(stderr, "\n\n");
    va_end(args);
    fflush(stderr);

    va_start(args, error);
    memset(buffer, 0, sizeof(buffer));
    M_vsnprintf(buffer, sizeof(buffer) - 1, error, args);
    va_end(args);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, DOOMRETRO_NAME, buffer, NULL);

    SDL_Quit();

    exit(-1);
}

//
// I_Malloc
//
void *I_Malloc(size_t size)
{
    void    *newp = malloc(size);

    if (!newp && size)
        I_Error("I_Malloc: Failure trying to allocate %zu bytes", size);

    return newp;
}

//
// I_Calloc
//
void *I_Calloc(size_t count, size_t size)
{
    void    *newp = calloc(count, size);

    if (!newp && size)
        I_Error("I_Calloc: Failure trying to allocate %zu bytes", size);

    return newp;
}

//
// I_Realloc
//
void *I_Realloc(void *block, size_t size)
{
    void    *newp = realloc(block, size);

    if (!newp && size)
        I_Error("I_Realloc: Failure trying to reallocate %zu bytes", size);

    block = newp;
    return block;
}
