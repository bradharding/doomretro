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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "c_console.h"
#include "d_main.h"
#include "doomstat.h"
#include "i_discord.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_setup.h"
#include "version.h"
#include "yyjson/yyjson.h"

#if defined(_WIN32)
static HANDLE   discordpipe = INVALID_HANDLE_VALUE;
static time_t   activitystart;
static uint64_t nextconnectattempt;
static uint64_t nextupdate;
static uint64_t nonce;
static char     cachedappid[33];
static char     lastdetails[128];
static char     laststate[128];

static void I_ResetDiscordRPCState(void)
{
    activitystart = 0;
    nextupdate = 0;
    lastdetails[0] = '\0';
    laststate[0] = '\0';
}

static bool I_DiscordSendFrame(const uint32_t opcode, const char *payload)
{
    const uint32_t  length = (uint32_t)strlen(payload);
    uint8_t         header[8];
    DWORD           written;

    memcpy(header, &opcode, sizeof(opcode));
    memcpy(header + sizeof(opcode), &length, sizeof(length));

    return (WriteFile(discordpipe, header, sizeof(header), &written, NULL)
        && written == sizeof(header)
        && (!length
            || (WriteFile(discordpipe, payload, length, &written, NULL) && written == length)));
}

static void I_DiscordDrainPipe(void)
{
    uint8_t header[8];
    DWORD   read;

    while (ReadFile(discordpipe, header, sizeof(header), &read, NULL) && read == sizeof(header))
    {
        uint32_t    length;

        memcpy(&length, header + 4, sizeof(length));

        while (length)
        {
            char        buffer[512];
            const DWORD chunk = (DWORD)(length < sizeof(buffer) ? length : sizeof(buffer));

            if (!ReadFile(discordpipe, buffer, chunk, &read, NULL) || !read)
                return;

            length -= read;
        }
    }
}

static void I_BuildDiscordActivity(char *details, const size_t detailssize,
    char *state, const size_t statesize)
{
    char    *temp = removenonprintable(*mapnumandtitle ? mapnumandtitle : mapnum);

    details[0] = '\0';
    state[0] = '\0';

    if (gamestate == GS_LEVEL)
    {
        if (paused)
            M_StringCopy(details, "Paused", detailssize);
        else if (menuactive || consoleactive)
            M_StringCopy(details, "In menu", detailssize);
        else if (!windowfocused)
            M_StringCopy(details, "In background", detailssize);
        else
        {
            M_snprintf(details, (int)detailssize, "Playing %s", gamedescription);
            M_StringCopy(state, temp, statesize);
            free(temp);
            return;
        }

        M_StringCopy(state, temp, statesize);
    }
    else if (gamestate == GS_INTERMISSION)
    {
        M_StringCopy(details, "In intermission", detailssize);
        M_StringCopy(state, temp, statesize);
    }
    else if (gamestate == GS_FINALE)
    {
        M_StringCopy(details, "Watching finale", detailssize);
        M_StringCopy(state, gamedescription, statesize);
    }
    else
    {
        M_StringCopy(details, (splashscreen ? "Starting up" : "On title screen"), detailssize);
        M_StringCopy(state, gamedescription, statesize);
    }

    free(temp);
}

static bool I_SetDiscordActivity(const char *details, const char *state)
{
    yyjson_mut_doc  *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val  *root;
    yyjson_mut_val  *args;
    yyjson_mut_val  *activity;
    yyjson_mut_val  *timestamps;
    char            buffer[32];
    char            *payload;

    if (!doc)
        return false;

    if (!(root = yyjson_mut_obj(doc))
        || !(args = yyjson_mut_obj_add_obj(doc, root, "args"))
        || !(activity = yyjson_mut_obj_add_obj(doc, args, "activity")))
    {
        yyjson_mut_doc_free(doc);
        return false;
    }

    yyjson_mut_doc_set_root(doc, root);

    if (*details && !yyjson_mut_obj_add_strcpy(doc, activity, "details", details))
    {
        yyjson_mut_doc_free(doc);
        return false;
    }

    if (*state && !yyjson_mut_obj_add_strcpy(doc, activity, "state", state))
    {
        yyjson_mut_doc_free(doc);
        return false;
    }

    if (activitystart)
    {
        if (!(timestamps = yyjson_mut_obj_add_obj(doc, activity, "timestamps"))
            || !yyjson_mut_obj_add_sint(doc, timestamps, "start", (int64_t)activitystart))
        {
            yyjson_mut_doc_free(doc);
            return false;
        }
    }

    if (!yyjson_mut_obj_add_uint(doc, args, "pid", (uint64_t)GetCurrentProcessId())
        || !yyjson_mut_obj_add_str(doc, root, "cmd", "SET_ACTIVITY"))
    {
        yyjson_mut_doc_free(doc);
        return false;
    }

    M_snprintf(buffer, sizeof(buffer), "%llu", ++nonce);

    if (!yyjson_mut_obj_add_strcpy(doc, root, "nonce", buffer))
    {
        yyjson_mut_doc_free(doc);
        return false;
    }

    if (!(payload = yyjson_mut_write(doc, 0, NULL)))
    {
        yyjson_mut_doc_free(doc);
        return false;
    }

    if (!I_DiscordSendFrame(1, payload))
    {
        yyjson_mut_doc_free(doc);
        free(payload);
        return false;
    }

    yyjson_mut_doc_free(doc);
    free(payload);
    return true;
}

static bool I_OpenDiscordRPC(void)
{
    char    pipename[32];

    for (int i = 0; i < DISCORD_RPC_MAX_IPC_PIPES; i++)
    {
        DWORD           mode = (PIPE_READMODE_BYTE | PIPE_NOWAIT);
        yyjson_mut_doc  *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val  *json;
        char            *payload;

        M_snprintf(pipename, sizeof(pipename), "\\\\.\\pipe\\discord-ipc-%i", i);
        discordpipe = CreateFileA(pipename, (GENERIC_READ | GENERIC_WRITE), 0, NULL, OPEN_EXISTING, 0, NULL);

        if (discordpipe == INVALID_HANDLE_VALUE)
            continue;

        SetNamedPipeHandleState(discordpipe, &mode, NULL, NULL);

        if (!doc || !(json = yyjson_mut_obj(doc)))
        {
            yyjson_mut_doc_free(doc);
            break;
        }

        yyjson_mut_doc_set_root(doc, json);

        if (!yyjson_mut_obj_add_int(doc, json, "v", 1)
            || !yyjson_mut_obj_add_str(doc, json, "client_id", DOOMRETRO_DISCORDAPPID))
        {
            yyjson_mut_doc_free(doc);
            break;
        }

        payload = yyjson_mut_write(doc, 0, NULL);
        yyjson_mut_doc_free(doc);

        if (!payload)
            break;

        if (I_DiscordSendFrame(0, payload))
        {
            free(payload);
            I_DiscordDrainPipe();
            return true;
        }

        free(payload);
        CloseHandle(discordpipe);
        discordpipe = INVALID_HANDLE_VALUE;
    }

    if (discordpipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(discordpipe);
        discordpipe = INVALID_HANDLE_VALUE;
    }

    return false;
}
#endif

void I_InitDiscordRPC(void)
{
#if defined(_WIN32)
    cachedappid[0] = '\0';
    nextconnectattempt = 0;
    nonce = 0;
    I_ResetDiscordRPCState();
#endif
}

void I_ShutdownDiscordRPC(void)
{
#if defined(_WIN32)
    if (discordpipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(discordpipe);
        discordpipe = INVALID_HANDLE_VALUE;
    }
#endif
}

void I_UpdateDiscordRPC(void)
{
#if defined(_WIN32)
    char        details[128];
    char        state[128];
    uint64_t    now;

    if (!discordpresence)
    {
        I_ShutdownDiscordRPC();
        cachedappid[0] = '\0';
        I_ResetDiscordRPCState();
        return;
    }

    now = I_GetTimeMS();

    if (discordpipe == INVALID_HANDLE_VALUE)
    {
        if (now < nextconnectattempt)
            return;

        if (!I_OpenDiscordRPC())
        {
            nextconnectattempt = now + DISCORD_RPC_UPDATE_WAIT;
            return;
        }
    }

    I_DiscordDrainPipe();
    I_BuildDiscordActivity(details, sizeof(details), state, sizeof(state));

    if (!activitystart || !M_StringCompare(details, lastdetails) || !M_StringCompare(state, laststate))
        activitystart = time(NULL);

    if (!nextupdate || now >= nextupdate
        || !M_StringCompare(details, lastdetails) || !M_StringCompare(state, laststate))
    {
        if (!I_SetDiscordActivity(details, state))
        {
            I_ShutdownDiscordRPC();
            nextconnectattempt = now + DISCORD_RPC_UPDATE_WAIT;
            return;
        }

        M_StringCopy(lastdetails, details, sizeof(lastdetails));
        M_StringCopy(laststate, state, sizeof(laststate));
        nextupdate = now + DISCORD_RPC_UPDATE_WAIT;
    }
#endif
}
