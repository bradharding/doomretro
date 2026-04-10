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

#pragma warning(push)
#pragma warning(disable: 4091)
#include <DbgHelp.h>
#pragma warning(pop)

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

static const char *I_GetExceptionName(DWORD exceptioncode)
{
    switch (exceptioncode)
    {
        case EXCEPTION_ACCESS_VIOLATION:
            return "Access Violation";

        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            return "Array Bounds Exceeded";

        case EXCEPTION_BREAKPOINT:
            return "Breakpoint";

        case EXCEPTION_DATATYPE_MISALIGNMENT:
            return "Datatype Misalignment";

        case EXCEPTION_FLT_DENORMAL_OPERAND:
            return "Floating Point Denormal Operand";

        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            return "Floating Point Division by Zero";

        case EXCEPTION_FLT_INEXACT_RESULT:
            return "Floating Point Inexact Result";

        case EXCEPTION_FLT_INVALID_OPERATION:
            return "Floating Point Invalid Operation";

        case EXCEPTION_FLT_OVERFLOW:
            return "Floating Point Overflow";

        case EXCEPTION_FLT_STACK_CHECK:
            return "Floating Point Stack Check";

        case EXCEPTION_FLT_UNDERFLOW:
            return "Floating Point Underflow";

        case EXCEPTION_GUARD_PAGE:
            return "Guard Page Violation";

        case EXCEPTION_ILLEGAL_INSTRUCTION:
            return "Illegal Instruction";

        case EXCEPTION_IN_PAGE_ERROR:
            return "Page Error";

        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            return "Integer Division by Zero";

        case EXCEPTION_INT_OVERFLOW:
            return "Integer Overflow";

        case EXCEPTION_INVALID_DISPOSITION:
            return "Invalid Disposition";

        case EXCEPTION_INVALID_HANDLE:
            return "Invalid Handle";

        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            return "Non-Continuable Exception";

        case EXCEPTION_PRIV_INSTRUCTION:
            return "Privileged Instruction";

        case EXCEPTION_SINGLE_STEP:
            return "Single Step";

        case EXCEPTION_STACK_OVERFLOW:
            return "Stack Overflow";

        default:
            return "Unknown Exception";
    }
}

static void I_LogAddressDetails(FILE *logfile, HANDLE process, DWORD64 address, const char *label)
{
    char                symbolbuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
    PSYMBOL_INFO        symbol = (PSYMBOL_INFO)symbolbuffer;
    DWORD64             displacement64 = 0;
    DWORD               displacement = 0;
    BOOL                hasline;
    IMAGEHLP_LINE64     line;
    IMAGEHLP_MODULE64   module;
    char                moduleprefix[64] = "";
    char                symboltext[512];

    ZeroMemory(symbolbuffer, sizeof(symbolbuffer));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME - 1;

    ZeroMemory(&line, sizeof(line));
    line.SizeOfStruct = sizeof(line);

    ZeroMemory(&module, sizeof(module));
    module.SizeOfStruct = sizeof(module);

    if (SymGetModuleInfo64(process, address, &module) && module.ModuleName[0])
        M_snprintf(moduleprefix, sizeof(moduleprefix), "%s!", module.ModuleName);

    if (SymFromAddr(process, address, &displacement64, symbol))
        M_snprintf(symboltext, sizeof(symboltext), "%s%s + 0x%I64X", moduleprefix, symbol->Name, displacement64);
    else if (module.ModuleName[0])
        M_snprintf(symboltext, sizeof(symboltext), "%s+0x%I64X", module.ModuleName, address - module.BaseOfImage);
    else
        M_snprintf(symboltext, sizeof(symboltext), "0x%p", (void *)(uintptr_t)address);

    fprintf(logfile, "%s%s", label, symboltext);

    hasline = SymGetLineFromAddr64(process, address, &displacement, &line);

    if (hasline)
        fprintf(logfile, " (%s:%lu", line.FileName, line.LineNumber);

    if (hasline && displacement)
        fprintf(logfile, "+%lu", displacement);

    if (hasline)
        fprintf(logfile, ")");

    fputc('\n', logfile);
}

static void I_LogExceptionParameters(FILE *logfile, const EXCEPTION_RECORD *record)
{
    if (record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || record->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
    {
        const char  *operation = "accessing";

        if (record->NumberParameters >= 2)
        {
            if (record->ExceptionInformation[0] == 0)
                operation = "reading";
            else if (record->ExceptionInformation[0] == 1)
                operation = "writing";
            else if (record->ExceptionInformation[0] == 8)
                operation = "executing";

            fprintf(logfile, "Fault Details:     Tried %s address 0x%p\n", operation,
                (void *)(uintptr_t)record->ExceptionInformation[1]);
        }

        if (record->ExceptionCode == EXCEPTION_IN_PAGE_ERROR && record->NumberParameters >= 3)
            fprintf(logfile, "In-Page Status:    0x%08lX\n", (DWORD)record->ExceptionInformation[2]);
    }
}

static void I_LogRegisters(FILE *logfile, const CONTEXT *context)
{
    fprintf(logfile, "Registers:\n");

#if defined(_M_X64)
    fprintf(logfile, "    RIP=0x%016I64X RSP=0x%016I64X RBP=0x%016I64X\n",
        context->Rip, context->Rsp, context->Rbp);
    fprintf(logfile, "    RAX=0x%016I64X RBX=0x%016I64X RCX=0x%016I64X\n",
        context->Rax, context->Rbx, context->Rcx);
    fprintf(logfile, "    RDX=0x%016I64X RSI=0x%016I64X RDI=0x%016I64X\n",
        context->Rdx, context->Rsi, context->Rdi);
    fprintf(logfile, "    R8 =0x%016I64X R9 =0x%016I64X R10=0x%016I64X\n",
        context->R8, context->R9, context->R10);
    fprintf(logfile, "    R11=0x%016I64X R12=0x%016I64X R13=0x%016I64X\n",
        context->R11, context->R12, context->R13);
    fprintf(logfile, "    R14=0x%016I64X R15=0x%016I64X EFlags=0x%08lX\n",
        context->R14, context->R15, context->EFlags);
#elif defined(_M_IX86)
    fprintf(logfile, "    EIP=0x%08lX ESP=0x%08lX EBP=0x%08lX\n",
        context->Eip, context->Esp, context->Ebp);
    fprintf(logfile, "    EAX=0x%08lX EBX=0x%08lX ECX=0x%08lX EDX=0x%08lX\n",
        context->Eax, context->Ebx, context->Ecx, context->Edx);
    fprintf(logfile, "    ESI=0x%08lX EDI=0x%08lX EFlags=0x%08lX\n",
        context->Esi, context->Edi, context->EFlags);
#else
    fprintf(logfile, "    Register logging isn't implemented for this architecture.\n");
#endif
}

static void I_LogCallStack(FILE *logfile, HANDLE process, HANDLE thread, const CONTEXT *contextrecord)
{
    CONTEXT         context = *contextrecord;
    STACKFRAME64    stackframe;
    DWORD64         address;
    DWORD64         lastaddress = 0;
    DWORD           machinetype;
    int             frame = 0;

    ZeroMemory(&stackframe, sizeof(stackframe));

#if defined(_M_X64)
    machinetype = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrFrame.Offset = context.Rbp;
    stackframe.AddrStack.Offset = context.Rsp;
#elif defined(_M_IX86)
    machinetype = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrStack.Offset = context.Esp;
#else
    fprintf(logfile, "Call Stack:        Unavailable on this architecture.\n");
    return;
#endif

    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Mode = AddrModeFlat;

    fprintf(logfile, "Call Stack:\n");

    address = stackframe.AddrPC.Offset;

    if (address)
    {
        fprintf(logfile, "    #%02d ", frame);
        I_LogAddressDetails(logfile, process, address, "");
        lastaddress = address;
        frame++;
    }

    while (frame < 64)
    {
        if (!StackWalk64(machinetype, process, thread, &stackframe, &context, NULL,
            SymFunctionTableAccess64, SymGetModuleBase64, NULL) || !stackframe.AddrPC.Offset)
            break;

        address = stackframe.AddrPC.Offset;

        if (address == lastaddress)
            break;

        fprintf(logfile, "    #%02d ", frame);
        I_LogAddressDetails(logfile, process, address, "");
        lastaddress = address;
        frame++;
    }

    if (!frame)
        fprintf(logfile, "    <unavailable>\n");
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
    M_snprintf(logpath, sizeof(logpath), "%s" DIR_SEPARATOR_S "%s.txt", crashfolder, timestamp);

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
        HANDLE      process = GetCurrentProcess();
        HANDLE      thread = GetCurrentThread();
        DWORD       exceptioncode = exceptionInfo->ExceptionRecord->ExceptionCode;
        const char  *exceptionname = I_GetExceptionName(exceptioncode);
        BOOL        symsinitialized;

        GetModuleFileName(NULL, exepath, sizeof(exepath));
        SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
        symsinitialized = SymInitialize(process, NULL, TRUE);

        fprintf(logfile, "File:              %s\n", exepath);
        fprintf(logfile, "Version:           %s\n", DOOMRETRO_VERSIONSTRING);
        fprintf(logfile, "Architecture:      %i-bit\n", (int)(sizeof(void *) * 8));
        fprintf(logfile, "Thread ID:         %lu\n", GetCurrentThreadId());

        M_snprintf(readabletimestamp, sizeof(readabletimestamp), "%d:%02d:%02d%s on %s, %s %d, %d",
            (!(tm_info->tm_hour % 12) ? 12 : (tm_info->tm_hour % 12)),
            tm_info->tm_min,
            tm_info->tm_sec,
            (tm_info->tm_hour < 12 ? "am" : "pm"),
            daynames[tm_info->tm_wday],
            monthnames[tm_info->tm_mon],
            tm_info->tm_mday,
            1900 + tm_info->tm_year);
        fprintf(logfile, "Time/Date:         %s\n", readabletimestamp);

        fprintf(logfile, "Exception:         %s (0x%08lX)\n", exceptionname, exceptioncode);

        fprintf(logfile, "Exception Address: 0x%p\n",
            exceptionInfo->ExceptionRecord->ExceptionAddress);

        I_LogExceptionParameters(logfile, exceptionInfo->ExceptionRecord);

        if (symsinitialized)
            I_LogAddressDetails(logfile, process, (DWORD64)(uintptr_t)exceptionInfo->ExceptionRecord->ExceptionAddress,
                "Exception Symbol:  ");

        if (windowsname[0] && windowsbuild[0])
            fprintf(logfile, "Operating System:  %s %s\n", windowsname, windowsbuild);

        if (wadsloaded[0])
            fprintf(logfile, "WAD%s              %s\n",
                (strchr(wadsloaded, ',') ? "s:" : ": "), M_StringReplaceLast(wadsloaded, ",", " and"));

        if (gamestate == GS_LEVEL)
        {
            if (mapnumandtitle[0])
                fprintf(logfile, "Map:               %s\n", removenonprintable(mapnumandtitle));

            if (viewplayer && viewplayer->mo)
                fprintf(logfile, "Player Position:   (%d, %d, %d)\n",
                    viewplayer->mo->x >> FRACBITS, viewplayer->mo->y >> FRACBITS, viewplayer->mo->z >> FRACBITS);
        }

        I_LogRegisters(logfile, exceptionInfo->ContextRecord);

        if (symsinitialized)
            I_LogCallStack(logfile, process, thread, exceptionInfo->ContextRecord);
        else
            fprintf(logfile, "Call Stack:        Unavailable (symbol initialization failed).\n");

        if (symsinitialized)
            SymCleanup(process);

        fclose(logfile);
    }

    // Show user dialog
    M_snprintf(message, sizeof(message),
        "%s has crashed unexpectedly.\n\n"
        "Important information about this crash has been saved as:\n"
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

        SDL_StopTextInput();

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

    SDL_StopTextInput();

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
