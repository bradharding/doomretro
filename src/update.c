/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <ctype.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include <ShellAPI.h>
#include <winhttp.h>
#endif

#include "SDL.h"

#include "i_system.h"
#include "m_misc.h"
#include "update.h"
#include "version.h"

static void GetVersionToken(const char *src, char *out, size_t outlen)
{
    const char  *p = src;
    size_t      i = 0;

    if (!src || !out || !outlen)
        return;

    while (*p && !isdigit(*p))
        ++p;

    if (!*p)
        if ((p = strchr(src, 'v')) && isdigit(*(p + 1)))
            p++;
        else
            p = src;

    while (*p && i + 1 < outlen)
        if (isdigit(*p) || *p == '.' || *p == '-' || *p == '+' || isalpha(*p))
            out[i++] = *p++;
        else
            break;

    if (!i)
        out[0] = '\0';
    else
        out[i] = '\0';
}

#if defined(_WIN32)
static char *WinHttpReadResponse(HINTERNET hRequest)
{
    DWORD   dwSize = 0;
    DWORD   dwDownloaded = 0;
    char    *buffer = NULL;
    size_t  total = 0;

    while (true)
    {
        char    *chunk;
        char    *newbuf;

        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)
            || !dwSize
            || !(chunk = (char *)malloc(dwSize + 1)))
            break;

        if (!WinHttpReadData(hRequest, (LPVOID)chunk, dwSize, &dwDownloaded))
        {
            free(chunk);
            break;
        }

        chunk[dwDownloaded] = '\0';

        if (!(newbuf = (char *)realloc(buffer, total + dwDownloaded + 1)))
        {
            free(chunk);
            free(buffer);
            buffer = NULL;
            break;
        }

        buffer = newbuf;
        memcpy(buffer + total, chunk, dwDownloaded + 1);
        total += dwDownloaded;
        buffer[total] = '\0';
        free(chunk);
    }

    return buffer;
}
#endif

void D_CheckForNewReleaseDialog(void)
{
#if defined(_WIN32)
    char        localVersion[128] = { 0 };
    char        *response = NULL;
    const char  *needle = "\"tag_name\"";
    char        *p;
    char        latestTag[128] = { 0 };
    HINTERNET   hSession = NULL;
    HINTERNET   hConnect = NULL;
    HINTERNET   hRequest = NULL;
    wchar_t     host[] = L"api.github.com";
    LPCWSTR     accepttypes[] = { L"*/*", NULL };
    LPCWSTR     path = DOOMRETRO_LATESTRELEASEPATH;

    do
    {
        char    *striplatest;
        char    *striplocal;

        GetVersionToken(DOOMRETRO_NAMEANDVERSIONSTRING, localVersion, sizeof(localVersion));

        if (localVersion[0] == '\0')
            break;

        if (!(hSession = WinHttpOpen(L"DoomRetroUpdateChecker/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)))
            break;

        if (!(hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0)))
            break;

        if (!(hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER,
            accepttypes, WINHTTP_FLAG_SECURE)))
            break;

        WinHttpAddRequestHeaders(hRequest, L"User-Agent: DoomRetroUpdateChecker/1.0\r\n"
            "Accept: application/vnd.github.v3+json\r\n", (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);

        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
            break;

        if (!WinHttpReceiveResponse(hRequest, NULL))
            break;

        if (!(response = WinHttpReadResponse(hRequest)))
            break;

        if ((p = strstr(response, needle)))
        {
            p += strlen(needle);

            if ((p = strchr(p, ':')) && (p = strchr(p, '\"')))
            {
                size_t  i = 0;

                ++p;

                while (*p && *p != '\"' && i + 1 < sizeof(latestTag))
                    latestTag[i++] = *p++;

                latestTag[i] = '\0';
            }
        }

        free(response);
        response = NULL;

        if (latestTag[0] == '\0')
            break;

        striplatest = (toupper(latestTag[0]) == 'V' ? latestTag + 1 : latestTag);
        striplocal = (toupper(localVersion[0]) == 'V' ? localVersion + 1 : localVersion);

        if (strncmp(striplatest, striplocal, 127))
        {
            char    buffer[512] = "\nA newer version of DOOM Retro is now available!\n"
                        "Do you want to visit www.doomretro.com to download it?\n";
            int     buttonid;

            const SDL_MessageBoxButtonData buttons[] =
            {
                { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No"  },
                { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 2, "Yes" }
            };

            const SDL_MessageBoxData messageboxdata =
            {
                SDL_MESSAGEBOX_INFORMATION,
                NULL,
                DOOMRETRO_NAME,
                buffer,
                SDL_arraysize(buttons),
                buttons,
                NULL
            };

            if (SDL_ShowMessageBox(&messageboxdata, &buttonid) >= 0 && buttonid == 2)
            {
                ShellExecute(GetActiveWindow(), "open", DOOMRETRO_BLOGURL, NULL, NULL, SW_SHOWNORMAL);
                I_Quit(false);
            }
        }

    } while (false);

    if (response)
        free(response);

    if (hRequest)
        WinHttpCloseHandle(hRequest);

    if (hConnect)
        WinHttpCloseHandle(hConnect);

    if (hSession)
        WinHttpCloseHandle(hSession);
#endif
}
