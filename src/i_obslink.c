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
#include <windows.h>
#include <bcrypt.h>
#include <winhttp.h>
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "c_console.h"
#include "doomstat.h"
#include "i_obslink.h"
#include "r_state.h"
#include "yyjson/yyjson.h"

#define OBSLINK_SCENE           "In Game Doom 16:9"
#define OBSLINK_MAPNAMESOURCE   "MAPNAME"
#define OBSLINK_AUTHORSOURCE    "AUTHOR"

#define OBSLINK_DEFAULTPORT     4455
#define OBSLINK_RECONNECTWAIT   5000
#define OBSLINK_MAXQUEUE        256
#define OBSLINK_MAXCOMMANDS     8
#define OBSLINK_MAXCOMMAND      255
#define OBSLINK_MAXMESSAGE      65536
#define OBSLINK_MAXTEXT         1024

void I_OBSLinkEscape(const char *in, char *out, const size_t size)
{
    size_t  j = 0;

    for (; *in && j + 2 < size; in++)
    {
        const unsigned char c = (unsigned char)*in;

        if (c == '"' || c == '\\')
        {
            out[j++] = '\\';
            out[j++] = (char)c;
        }
        else if (c >= 32 && c < 127)
            out[j++] = (char)c;
    }

    out[j] = '\0';
}

#if defined(_WIN32)
typedef enum
{
    status_none,
    status_connected,
    status_badpassword
} linkstatus_t;

typedef struct message_s
{
    struct message_s    *next;
    char                *text;
} message_t;

static const char   *allowedcommands[] = { "give", "spawn" };

static SDL_Thread   *linkthread;
static SDL_mutex    *mutex;
static SDL_cond     *cond;
static SDL_atomic_t quitting;
static SDL_atomic_t connected;
static SDL_atomic_t status;
static SDL_atomic_t inlevel;
static SDL_atomic_t pendingvisibility;

static HINTERNET    websocket;
static message_t    *queuehead;
static message_t    *queuetail;
static int          queuecount;

static char         commands[OBSLINK_MAXCOMMANDS][OBSLINK_MAXCOMMAND + 1];
static int          commandcount;

static char         mapnametext[OBSLINK_MAXTEXT];
static char         authortext[OBSLINK_MAXTEXT];
static bool         havemap;

static void I_QueueMessage(const char *text)
{
    message_t   *message;

    if (!SDL_AtomicGet(&connected) || !(message = malloc(sizeof(*message))))
        return;

    if (!(message->text = SDL_strdup(text)))
    {
        free(message);
        return;
    }

    message->next = NULL;
    SDL_LockMutex(mutex);

    if (queuecount < OBSLINK_MAXQUEUE)
    {
        if (queuetail)
            queuetail->next = message;
        else
            queuehead = message;

        queuetail = message;
        queuecount++;
        message = NULL;
    }

    SDL_CondSignal(cond);
    SDL_UnlockMutex(mutex);

    if (message)
    {
        SDL_free(message->text);
        free(message);
    }
}

static void I_ClearQueue(void)
{
    SDL_LockMutex(mutex);

    while (queuehead)
    {
        message_t   *next = queuehead->next;

        SDL_free(queuehead->text);
        free(queuehead);
        queuehead = next;
    }

    queuetail = NULL;
    queuecount = 0;
    SDL_UnlockMutex(mutex);
}

static void I_QueueRequest(const char *type, const char *id, const char *data)
{
    char    buffer[OBSLINK_MAXTEXT * 3];
    int     length = snprintf(buffer, sizeof(buffer),
                "{\"op\":6,\"d\":{\"requestType\":\"%s\",\"requestId\":\"%s\",\"requestData\":%s}}", type, id, data);

    if (length > 0 && length < (int)sizeof(buffer))
        I_QueueMessage(buffer);
}

static void I_QueueSetText(const char *source, const char *text)
{
    char    data[OBSLINK_MAXTEXT * 2];

    snprintf(data, sizeof(data), "{\"inputName\":\"%s\",\"inputSettings\":{\"text\":\"%s\"}}", source, text);
    I_QueueRequest("SetInputSettings", "text", data);
}

static void I_QueueSourceVisibility(const char *source, const bool visible)
{
    char    id[64];
    char    data[256];

    if (!SDL_AtomicGet(&connected))
        return;

    // The reply to this request turns the source on or off.
    snprintf(id, sizeof(id), "%s:%s", (visible ? "show" : "hide"), source);
    snprintf(data, sizeof(data), "{\"sceneName\":\"" OBSLINK_SCENE "\",\"sourceName\":\"%s\"}", source);
    SDL_AtomicAdd(&pendingvisibility, 1);
    I_QueueRequest("GetSceneItemId", id, data);
}

static void I_QueueVisibility(const bool visible)
{
    I_QueueSourceVisibility(OBSLINK_MAPNAMESOURCE, visible);
    I_QueueSourceVisibility(OBSLINK_AUTHORSOURCE, visible);
}

static void I_QueueMapUpdate(void)
{
    char    mapname[OBSLINK_MAXTEXT];
    char    author[OBSLINK_MAXTEXT];
    bool    send;

    SDL_LockMutex(mutex);
    send = havemap;
    memcpy(mapname, mapnametext, sizeof(mapname));
    memcpy(author, authortext, sizeof(author));
    SDL_UnlockMutex(mutex);

    if (send)
    {
        I_QueueSetText(OBSLINK_MAPNAMESOURCE, mapname);
        I_QueueSetText(OBSLINK_AUTHORSOURCE, author);
    }

    I_QueueVisibility(SDL_AtomicGet(&inlevel));
}

static bool I_CommandAllowed(const char *command)
{
    const size_t    length = strlen(command);

    if (!length || length > OBSLINK_MAXCOMMAND)
        return false;

    for (size_t i = 0; i < length; i++)
        if (command[i] == ';' || (unsigned char)command[i] < 32 || (unsigned char)command[i] > 126)
            return false;

    for (size_t i = 0; i < sizeof(allowedcommands) / sizeof(allowedcommands[0]); i++)
    {
        const size_t    word = strlen(allowedcommands[i]);

        if (!SDL_strncasecmp(command, allowedcommands[i], word) && (command[word] == ' ' || !command[word]))
            return true;
    }

    return false;
}

static void I_HandleResponse(yyjson_val *data)
{
    const char  *id = yyjson_get_str(yyjson_obj_get(data, "requestId"));
    yyjson_val  *item;
    bool        visible;

    if (!id || (strncmp(id, "show:", 5) && strncmp(id, "hide:", 5)))
        return;

    visible = (*id == 's');

    if ((item = yyjson_obj_get(yyjson_obj_get(data, "responseData"), "sceneItemId")))
    {
        char    request[256];

        snprintf(request, sizeof(request),
            "{\"sceneName\":\"" OBSLINK_SCENE "\",\"sceneItemId\":%i,\"sceneItemEnabled\":%s}",
            yyjson_get_int(item), (visible ? "true" : "false"));
        I_QueueRequest("SetSceneItemEnabled", "visibility", request);
    }

    SDL_AtomicAdd(&pendingvisibility, -1);
}

static void I_HandleEvent(yyjson_val *data)
{
    const char  *type = yyjson_get_str(yyjson_obj_get(data, "eventType"));
    const char  *command;

    if (!type || strcmp(type, "CustomEvent"))
        return;

    command = yyjson_get_str(yyjson_obj_get(yyjson_obj_get(data, "eventData"), "doomcommand"));

    if (command && I_CommandAllowed(command))
    {
        SDL_LockMutex(mutex);

        if (commandcount < OBSLINK_MAXCOMMANDS)
            SDL_strlcpy(commands[commandcount++], command, sizeof(commands[0]));

        SDL_UnlockMutex(mutex);
    }
}

// Reads one complete text message. Returns its length, or -1 if the connection ended.
static int I_ReceiveMessage(HINTERNET socket, char *buffer, const int size)
{
    int     length = 0;
    bool    toolong = false;

    while (true)
    {
        DWORD                           read = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE  type;
        char                            discard[1024];
        char                            *target = (toolong ? discard : buffer + length);
        const DWORD                     space = (toolong ? sizeof(discard) : (DWORD)(size - length - 1));

        if (WinHttpWebSocketReceive(socket, target, space, &read, &type) != NO_ERROR
            || type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
            return -1;

        if (!toolong)
        {
            length += read;
            toolong = (length >= size - 1);
        }

        if (type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE || type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE)
        {
            buffer[length] = '\0';
            return (toolong ? 0 : length);
        }
    }
}

static int SDLCALL I_ReaderThread(void *data)
{
    HINTERNET   socket = data;
    char        *buffer = malloc(OBSLINK_MAXMESSAGE);

    while (buffer && !SDL_AtomicGet(&quitting))
    {
        const int   length = I_ReceiveMessage(socket, buffer, OBSLINK_MAXMESSAGE);
        yyjson_doc  *doc;

        if (length < 0)
            break;

        if ((doc = yyjson_read(buffer, length, 0)))
        {
            yyjson_val  *root = yyjson_doc_get_root(doc);
            const int   op = yyjson_get_int(yyjson_obj_get(root, "op"));
            yyjson_val  *messagedata = yyjson_obj_get(root, "d");

            if (op == 5)
                I_HandleEvent(messagedata);
            else if (op == 7)
                I_HandleResponse(messagedata);

            yyjson_doc_free(doc);
        }
    }

    free(buffer);
    SDL_AtomicSet(&connected, 0);
    SDL_LockMutex(mutex);
    SDL_CondSignal(cond);
    SDL_UnlockMutex(mutex);
    return 0;
}

static void I_Base64(const unsigned char *in, const int length, char *out)
{
    static const char   table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int                 j = 0;

    for (int i = 0; i < length; i += 3)
    {
        const int   remaining = length - i;
        const int   value = (in[i] << 16) | ((remaining > 1 ? in[i + 1] : 0) << 8) | (remaining > 2 ? in[i + 2] : 0);

        out[j++] = table[(value >> 18) & 63];
        out[j++] = table[(value >> 12) & 63];
        out[j++] = (remaining > 1 ? table[(value >> 6) & 63] : '=');
        out[j++] = (remaining > 2 ? table[value & 63] : '=');
    }

    out[j] = '\0';
}

static bool I_HashToBase64(const char *first, const char *second, char *out)
{
    char            input[1024];
    unsigned char   hash[32];
    const int       length = snprintf(input, sizeof(input), "%s%s", first, second);

    if (length <= 0 || length >= (int)sizeof(input)
        || !BCRYPT_SUCCESS(BCryptHash(BCRYPT_SHA256_ALG_HANDLE, NULL, 0, (PUCHAR)input, length, hash, sizeof(hash))))
        return false;

    I_Base64(hash, sizeof(hash), out);
    SecureZeroMemory(input, sizeof(input));
    return true;
}

// Uses the settings OBS saved for its WebSocket server, so the link needs no setup.
static void I_ReadOBSSettings(int *port, char *password, const size_t size)
{
    const char  *appdata = getenv("APPDATA");
    const char  *variable = getenv("OBS_WS_PASSWORD");
    char        path[MAX_PATH * 2];
    yyjson_doc  *doc;

    *port = OBSLINK_DEFAULTPORT;
    *password = '\0';

    if (variable)
        SDL_strlcpy(password, variable, size);

    if (!appdata)
        return;

    snprintf(path, sizeof(path), "%s\\obs-studio\\plugin_config\\obs-websocket\\config.json", appdata);

    if ((doc = yyjson_read_file(path, 0, NULL, NULL)))
    {
        yyjson_val  *root = yyjson_doc_get_root(doc);
        const char  *saved = yyjson_get_str(yyjson_obj_get(root, "server_password"));
        const int   savedport = yyjson_get_int(yyjson_obj_get(root, "server_port"));

        if (saved && *saved)
            SDL_strlcpy(password, saved, size);

        if (savedport > 0)
            *port = savedport;

        yyjson_doc_free(doc);
    }
}

static HINTERNET I_OpenWebSocket(const int port)
{
    HINTERNET   session;
    HINTERNET   connection = NULL;
    HINTERNET   request = NULL;
    HINTERNET   socket = NULL;

    if (!(session = WinHttpOpen(L"DoomRetroOBSLink/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)))
        return NULL;

    WinHttpSetTimeouts(session, 1000, 1000, 2000, 2000);

    if ((connection = WinHttpConnect(session, L"127.0.0.1", (INTERNET_PORT)port, 0))
        && (request = WinHttpOpenRequest(connection, L"GET", L"/", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0))
        && WinHttpSetOption(request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0)
        && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
        && WinHttpReceiveResponse(request, NULL))
        socket = WinHttpWebSocketCompleteUpgrade(request, 0);

    if (request)
        WinHttpCloseHandle(request);

    if (connection)
        WinHttpCloseHandle(connection);

    WinHttpCloseHandle(session);
    return socket;
}

static bool I_SendText(HINTERNET socket, const char *text)
{
    return (WinHttpWebSocketSend(socket, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
        (PVOID)text, (DWORD)strlen(text)) == NO_ERROR);
}

static bool I_Identify(HINTERNET socket, const char *password)
{
    char        *buffer = malloc(OBSLINK_MAXMESSAGE);
    char        identify[256] = "{\"op\":1,\"d\":{\"rpcVersion\":1,\"eventSubscriptions\":1}}";
    yyjson_doc  *doc;
    int         length;
    bool        result = false;

    if (!buffer)
        return false;

    if ((length = I_ReceiveMessage(socket, buffer, OBSLINK_MAXMESSAGE)) <= 0 || !(doc = yyjson_read(buffer, length, 0)))
    {
        free(buffer);
        return false;
    }

    {
        yyjson_val  *auth = yyjson_obj_get(yyjson_obj_get(yyjson_doc_get_root(doc), "d"), "authentication");
        const char  *salt = yyjson_get_str(yyjson_obj_get(auth, "salt"));
        const char  *challenge = yyjson_get_str(yyjson_obj_get(auth, "challenge"));

        if (salt && challenge)
        {
            char    secret[64];
            char    response[64];

            if (I_HashToBase64(password, salt, secret) && I_HashToBase64(secret, challenge, response))
                snprintf(identify, sizeof(identify),
                    "{\"op\":1,\"d\":{\"rpcVersion\":1,\"authentication\":\"%s\",\"eventSubscriptions\":1}}", response);

            SecureZeroMemory(secret, sizeof(secret));
        }
    }

    yyjson_doc_free(doc);

    if (I_SendText(socket, identify) && (length = I_ReceiveMessage(socket, buffer, OBSLINK_MAXMESSAGE)) > 0
        && (doc = yyjson_read(buffer, length, 0)))
    {
        result = (yyjson_get_int(yyjson_obj_get(yyjson_doc_get_root(doc), "op")) == 2);
        yyjson_doc_free(doc);
    }

    if (!result)
        SDL_AtomicSet(&status, status_badpassword);

    free(buffer);
    return result;
}

static void I_RunConnection(HINTERNET socket)
{
    SDL_Thread  *reader;

    SDL_LockMutex(mutex);
    websocket = socket;
    SDL_UnlockMutex(mutex);

    SDL_AtomicSet(&connected, 1);
    SDL_AtomicSet(&status, status_connected);

    if ((reader = SDL_CreateThread(I_ReaderThread, "OBSLinkReader", socket)))
    {
        I_QueueMapUpdate();

        while (SDL_AtomicGet(&connected) && !SDL_AtomicGet(&quitting))
        {
            message_t   *message;

            SDL_LockMutex(mutex);

            if (!queuehead)
                SDL_CondWaitTimeout(cond, mutex, 200);

            if ((message = queuehead))
            {
                if (!(queuehead = message->next))
                    queuetail = NULL;

                queuecount--;
            }

            SDL_UnlockMutex(mutex);

            if (message)
            {
                if (!I_SendText(socket, message->text))
                    SDL_AtomicSet(&connected, 0);

                SDL_free(message->text);
                free(message);
            }
        }
    }

    SDL_AtomicSet(&connected, 0);

    SDL_LockMutex(mutex);
    websocket = NULL;
    SDL_UnlockMutex(mutex);

    WinHttpWebSocketShutdown(socket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
    WinHttpCloseHandle(socket);

    if (reader)
        SDL_WaitThread(reader, NULL);

    I_ClearQueue();
    SDL_AtomicSet(&pendingvisibility, 0);
}

static int SDLCALL I_LinkThread(void *data)
{
    while (!SDL_AtomicGet(&quitting))
    {
        char        password[256];
        int         port;
        HINTERNET   socket;

        I_ReadOBSSettings(&port, password, sizeof(password));

        if ((socket = I_OpenWebSocket(port)))
        {
            if (I_Identify(socket, password))
                I_RunConnection(socket);
            else
                WinHttpCloseHandle(socket);
        }

        SecureZeroMemory(password, sizeof(password));

        for (int waited = 0; waited < OBSLINK_RECONNECTWAIT && !SDL_AtomicGet(&quitting); waited += 100)
            SDL_Delay(100);
    }

    return 0;
}
#endif

void I_InitOBSLink(void)
{
#if defined(_WIN32)
    if (!(mutex = SDL_CreateMutex()) || !(cond = SDL_CreateCond())
        || !(linkthread = SDL_CreateThread(I_LinkThread, "OBSLink", NULL)))
        C_Warning(0, "The OBS link failed to start.");
#endif
}

void I_ShutdownOBSLink(void)
{
#if defined(_WIN32)
    if (!linkthread)
        return;

    // Give OBS a moment to turn the sources off before the link closes.
    if (SDL_AtomicGet(&connected))
    {
        SDL_AtomicSet(&inlevel, 0);
        I_QueueVisibility(false);

        for (int i = 0; i < 50; i++)
        {
            bool    finished;

            SDL_LockMutex(mutex);
            finished = (!queuecount && !SDL_AtomicGet(&pendingvisibility));
            SDL_UnlockMutex(mutex);

            if (finished)
                break;

            SDL_Delay(10);
        }

        SDL_Delay(50);
    }

    SDL_AtomicSet(&quitting, 1);
    SDL_LockMutex(mutex);
    SDL_CondSignal(cond);
    SDL_UnlockMutex(mutex);
    SDL_WaitThread(linkthread, NULL);
    linkthread = NULL;
#endif
}

void I_OBSLinkMap(const char *mapnum, const char *title, const char *author)
{
#if defined(_WIN32)
    char    escaped[256];

    if (!linkthread)
        return;

    I_OBSLinkEscape(title, escaped, sizeof(escaped));

    for (char *c = escaped; *c; c++)
        *c = (char)toupper((unsigned char)*c);

    SDL_LockMutex(mutex);
    snprintf(mapnametext, sizeof(mapnametext), "%s - %s", mapnum, escaped);
    SDL_strlcpy(authortext, author, sizeof(authortext));
    havemap = true;
    SDL_UnlockMutex(mutex);

    SDL_AtomicSet(&inlevel, 1);
    I_QueueMapUpdate();
#endif
}

void I_OBSLinkEvent(const char *event, const char *name, const char *format, ...)
{
#if defined(_WIN32)
    char    buffer[OBSLINK_MAXTEXT];
    char    escaped[256] = "";
    int     length;

    if (!SDL_AtomicGet(&connected))
        return;

    if (name)
        I_OBSLinkEscape(name, escaped, sizeof(escaped));

    length = snprintf(buffer, sizeof(buffer),
        "{\"event\":\"%s\",\"name\":\"%s\",\"episode\":%i,\"map\":%i,\"health\":%i,\"armor\":%i",
        event, escaped, gameepisode, gamemap,
        (viewplayer && viewplayer->health > 0 ? viewplayer->health : 0), (viewplayer ? viewplayer->armor : 0));

    if (length <= 0 || length >= (int)sizeof(buffer) - 2)
        return;

    if (format)
    {
        va_list args;
        int     extra;

        buffer[length++] = ',';
        va_start(args, format);
        extra = vsnprintf(buffer + length, sizeof(buffer) - length - 1, format, args);
        va_end(args);

        if (extra < 0 || extra >= (int)sizeof(buffer) - length - 1)
            return;

        length += extra;
    }

    buffer[length++] = '}';
    buffer[length] = '\0';

    {
        char    data[OBSLINK_MAXTEXT + 32];

        snprintf(data, sizeof(data), "{\"eventData\":%s}", buffer);
        I_QueueRequest("BroadcastCustomEvent", "event", data);
    }
#endif
}

void I_PollOBSLink(void)
{
#if defined(_WIN32)
    const int   current = SDL_AtomicSet(&status, status_none);
    char        pending[OBSLINK_MAXCOMMANDS][OBSLINK_MAXCOMMAND + 1];
    int         count;

    if (!linkthread)
        return;

    if ((gamestate == GS_LEVEL) != (SDL_AtomicGet(&inlevel) != 0))
    {
        SDL_AtomicSet(&inlevel, (gamestate == GS_LEVEL));
        I_QueueVisibility(gamestate == GS_LEVEL);
    }

    if (current == status_connected)
        C_Output("Connected to OBS.");
    else if (current == status_badpassword)
        C_Warning(0, "OBS rejected the WebSocket password.");

    SDL_LockMutex(mutex);
    count = commandcount;
    memcpy(pending, commands, count * sizeof(commands[0]));
    commandcount = 0;
    SDL_UnlockMutex(mutex);

    for (int i = 0; i < count; i++)
    {
        C_ExecuteInputString(pending[i]);
        I_OBSLinkEvent("command", pending[i], NULL);
    }
#endif
}
