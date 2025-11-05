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

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "SDL_image.h"
#include "SDL_mixer.h"

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_controller.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_spec.h"
#include "s_sound.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"

console_t               *console = NULL;

bool                    consoleactive = false;
int                     consoleheight = 0;
int                     consoledirection = -1;
static int              consoleanim;

patch_t                 *consolefont[CONSOLEFONTSIZE];
patch_t                 *degree;
patch_t                 *unknownchar;
patch_t                 *altunderscores;
patch_t                 *altbuddha;
patch_t                 *lsquote;
patch_t                 *ldquote;

static patch_t          *brand;
static patch_t          *endash;
static patch_t          *trademark;
static patch_t          *copyright;
static patch_t          *regomark;
static patch_t          *multiply;
static patch_t          *warning;
static patch_t          *fps;
static patch_t          *ampm[2];

patch_t                 *bindlist;
patch_t                 *cmdlist;
patch_t                 *cvarlist;
patch_t                 *maplist;
patch_t                 *mapstats;
patch_t                 *playerstats;
patch_t                 *thinglist;

static short            brandwidth;
static short            brandheight;
static short            spacewidth;
static short            altbuddhawidth;
static short            fpswidth;
static short            ampmwidth;

char                    consoleinput[255] = "";
int                     numconsolestrings = 0;
size_t                  consolestringsmax = 0;

static size_t           undolevels;
static undohistory_t    *undohistory;

static bool             showcaret = true;
static uint64_t         caretwait;
int                     caretpos;
int                     selectstart;
int                     selectend;

bool                    pathoverlay;

char                    consolecheat[255];
char                    consolecheatparm[3];

static int              inputhistory = -1;
static int              outputhistory = -1;
static bool             topofconsole;
static bool             cheatsequence;

static int              degreewidth;
static int              suckswidth;
static int              timerwidth;
static int              timewidth;
static int              zerowidth;

static byte             *consoleautomapbevelcolor;
static byte             *consolebackcolor1;
static byte             *consolebackcolor2;
static byte             *consolebevelcolor1;
static byte             *consolebevelcolor2;
static int              consoleboldcolor;
static int              consolebolditalicscolor;
int                     consoleedgecolor1;
int                     consoleedgecolor2;
static int              consolecaretcolor;
static int              consoledividercolor;
static int              consoleinputcolor;
static int              consoleoutputcolor;
static int              consoleplayermessagecolor;
static int              consolescrollbarfacecolor;
static int              consolescrollbartrackcolor;
static int              consoleselectedinputbackgroundcolor;
static int              consoleselectedinputcolor;
static int              consolewarningboldcolor;
static int              consolewarningcolor;

static int              consolecolors[STRINGTYPES];
static int              consoleboldcolors[STRINGTYPES];

bool                    scrollbardrawn;
int                     scrollbarfacestart;
int                     scrollbarfaceend;

static void C_CreateTimeStamp(const int index)
{
    const time_t    now = time(NULL);
    struct tm       *currenttime = localtime(&now);

    console[index].timestamp = *currenttime;
}

void C_Input(const char *string, ...)
{
    va_list args;
    char    buffer[CONSOLETEXTMAXLENGTH];

    if (togglingvanilla)
        return;

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (numconsolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[numconsolestrings].string, buffer, sizeof(console[0].string));
    console[numconsolestrings].indent = 0;
    console[numconsolestrings].wrap = 0;
    console[numconsolestrings++].stringtype = inputstring;
    inputhistory = -1;
    outputhistory = -1;
    consoleinput[0] = '\0';
    caretpos = 0;
    selectstart = 0;
    selectend = 0;
}

void C_Cheat(const char *string)
{
    char        buffer[CONSOLETEXTMAXLENGTH];
    const int   len = (int)strlen(string);

    for (int i = 0; i < len; i++)
        buffer[i] = '\x95';

    buffer[len] = '\0';

    if (numconsolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[numconsolestrings].string, buffer, sizeof(console[0].string));
    console[numconsolestrings].indent = 0;
    console[numconsolestrings].wrap = 0;
    console[numconsolestrings++].stringtype = cheatstring;
    inputhistory = -1;
    outputhistory = -1;
    consoleinput[0] = '\0';
    caretpos = 0;
    selectstart = 0;
    selectend = 0;
}

void C_IntegerCVAROutput(const char *cvar, const int value)
{
    char    *temp = commify(value);

    C_Input("%s %s", cvar, temp);
    free(temp);
}

void C_IntegerCVAROutputNoRepeat(const char *cvar, const int value)
{
    char    buffer[CONSOLETEXTMAXLENGTH];
    char    *temp = commify(value);

    M_snprintf(buffer, sizeof(buffer), "%s %s", cvar, temp);

    if (numconsolestrings && M_StringStartsWith(console[numconsolestrings - 1].string, cvar))
        M_StringCopy(console[numconsolestrings - 1].string, buffer, sizeof(console[0].string));
    else
        C_Input(buffer);

    free(temp);
}

void C_PercentCVAROutput(const char *cvar, const int value)
{
    char    *temp = commify(value);

    C_Input("%s %s%%", cvar, temp);
    free(temp);
}

void C_StringCVAROutput(const char *cvar, const char *string)
{
    C_Input("%s %s", cvar, string);
}

void C_Output(const char *string, ...)
{
    va_list args;
    char    buffer[CONSOLETEXTMAXLENGTH];

    if (!*string || togglingvanilla)
        return;

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (numconsolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[numconsolestrings].string, buffer, sizeof(console[0].string));
    console[numconsolestrings].string[0] = toupper(console[numconsolestrings].string[0]);
    console[numconsolestrings].indent = 0;
    console[numconsolestrings].wrap = 0;
    console[numconsolestrings++].stringtype = outputstring;
    outputhistory = -1;
}

void C_TabbedOutput(const int tabs[MAXTABS], const char *string, ...)
{
    va_list args;
    char    buffer[CONSOLETEXTMAXLENGTH];

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (numconsolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[numconsolestrings].string, buffer, sizeof(console[0].string));
    console[numconsolestrings].stringtype = outputstring;
    memcpy(console[numconsolestrings].tabs, tabs, sizeof(console[0].tabs));
    console[numconsolestrings].indent = (tabs[2] ? tabs[2] : (tabs[1] ? tabs[1] : tabs[0])) - 10;
    console[numconsolestrings++].wrap = 0;
    outputhistory = -1;
}

void C_Header(const int tabs[MAXTABS], patch_t *header, const char *string)
{
    if (numconsolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    console[numconsolestrings].stringtype = headerstring;
    memcpy(console[numconsolestrings].tabs, tabs, sizeof(console[0].tabs));
    console[numconsolestrings].header = header;
    console[numconsolestrings].wrap = 0;
    M_StringCopy(console[numconsolestrings++].string, string, sizeof(console[0].string));
    outputhistory = -1;
}

void C_Warning(const int minwarninglevel, const char *string, ...)
{
    va_list     args;
    char        buffer[CONSOLETEXTMAXLENGTH];
    const int   i = (numconsolestrings > 0 ? numconsolestrings - 1 : 0);

    if (con_warninglevel < minwarninglevel && !devparm)
        return;

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (numconsolestrings > 0 && console[i].stringtype == warningstring && M_StringCompare(console[i].string, buffer))
        console[i].count++;
    else
    {
        if (numconsolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringCopy(console[numconsolestrings].string, buffer, sizeof(console[0].string));
        console[numconsolestrings].indent = WARNINGWIDTH + 1;
        console[numconsolestrings].wrap = 0;
        console[numconsolestrings].stringtype = warningstring;
        console[numconsolestrings++].count = 1;
    }

    outputhistory = -1;
}

void C_PlayerMessage(const char *string, ...)
{
    va_list     args;
    char        buffer[CONSOLETEXTMAXLENGTH];
    const int   i = (numconsolestrings > 0 ? numconsolestrings - 1 : 0);

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (numconsolestrings > 0 && console[i].stringtype == playermessagestring && M_StringCompare(console[i].string, buffer) && groupmessages)
    {
        C_CreateTimeStamp(i);
        console[i].count++;
    }
    else
    {
        if (numconsolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringReplaceAll(buffer, "\n", " ", false);
        M_StringCopy(console[numconsolestrings].string, buffer, sizeof(console[0].string));
        console[numconsolestrings].stringtype = playermessagestring;
        C_CreateTimeStamp(numconsolestrings);
        console[numconsolestrings].string[0] = toupper(console[numconsolestrings].string[0]);
        console[numconsolestrings].indent = 0;
        console[numconsolestrings].wrap = 0;
        console[numconsolestrings++].count = 1;
    }

    outputhistory = -1;
}

void C_PlayerObituary(const char *string, ...)
{
    va_list args;
    char    buffer[CONSOLETEXTMAXLENGTH];

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (numconsolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[numconsolestrings].string, buffer, sizeof(console[0].string));
    console[numconsolestrings].stringtype = playerwarningstring;
    C_CreateTimeStamp(numconsolestrings);
    console[numconsolestrings].string[0] = toupper(console[numconsolestrings].string[0]);
    console[numconsolestrings].indent = WARNINGWIDTH + 2;
    console[numconsolestrings].wrap = 0;
    console[numconsolestrings++].count = 1;

    outputhistory = -1;

    if (obituaries)
    {
        HU_SetPlayerMessage(buffer, false, false);
        message_warning = true;
    }
}

void C_PlayerWarning(const char *string, ...)
{
    va_list args;
    char    buffer[CONSOLETEXTMAXLENGTH];

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (numconsolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[numconsolestrings].string, buffer, sizeof(console[0].string));
    console[numconsolestrings].stringtype = playerwarningstring;
    C_CreateTimeStamp(numconsolestrings);
    console[numconsolestrings].string[0] = toupper(console[numconsolestrings].string[0]);
    console[numconsolestrings].indent = WARNINGWIDTH + 2;
    console[numconsolestrings].wrap = 0;
    console[numconsolestrings++].count = 1;

    outputhistory = -1;
}

void C_ResetWrappedLines(void)
{
    for (int i = 0; i < numconsolestrings; i++)
        console[i].wrap = 0;
}

static void C_AddToUndoHistory(void)
{
    undohistory = I_Realloc(undohistory, (undolevels + 1) * sizeof(*undohistory));
    undohistory[undolevels].input = M_StringDuplicate(consoleinput);
    undohistory[undolevels].caretpos = caretpos;
    undohistory[undolevels].selectstart = selectstart;
    undohistory[undolevels].selectend = selectend;
    undolevels++;
}

void C_AddConsoleDivider(void)
{
    if (!numconsolestrings || console[numconsolestrings - 1].stringtype != dividerstring)
    {
        if (numconsolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringCopy(console[numconsolestrings].string, DIVIDERSTRING, sizeof(console[0].string));
        console[numconsolestrings++].stringtype = dividerstring;
    }
}

const kern_t altkern[] =
{
    { ' ',  ' ',  -1 }, { ' ',  'J',  -1 }, { ' ',  'T',  -1 }, { '!',  ' ',   2 },
    { '"',  '+',  -1 }, { '"',  ',',  -2 }, { '"',  '.',  -2 }, { '"',  '4',  -1 },
    { '"',  'a',  -1 }, { '"',  'c',  -1 }, { '"',  'd',  -1 }, { '"',  'e',  -1 },
    { '"',  'g',  -1 }, { '"',  'J',  -2 }, { '"',  'j',  -2 }, { '"',  'o',  -1 },
    { '"',  'q',  -1 }, { '"',  's',  -1 }, { '(',  '(',  -1 }, { '(',  '-',  -1 },
    { '(',  '4',  -1 }, { '(',  't',  -1 }, { ')',  ')',  -1 }, { '+',  'j',  -2 },
    { ',',  '-',  -1 }, { ',',  '4',  -1 }, { ',',  '7',  -1 }, { '.',  '"',  -1 },
    { '.',  '4',  -1 }, { '.',  '7',  -1 }, { '.',  '\\', -1 }, { '/',  '/',  -2 },
    { '/',  '4',  -1 }, { '/',  'a',  -1 }, { '/',  'c',  -1 }, { '/',  'd',  -1 },
    { '/',  'e',  -1 }, { '/',  'g',  -1 }, { '/',  'q',  -1 }, { '/',  's',  -1 },
    { '0',  ',',  -1 }, { '0',  '4',  -1 }, { '0',  ';',  -1 }, { '0',  'j',  -2 },
    { '1',  '"',  -1 }, { '1',  ',',  -1 }, { '1',  '\'', -1 }, { '1',  '\\', -1 },
    { '1',  'j',  -2 }, { '2',  ',',  -1 }, { '2',  '4',  -1 }, { '2',  'j',  -2 },
    { '3',  ',',  -1 }, { '3',  '4',  -1 }, { '3',  ';',  -1 }, { '3',  'j',  -2 },
    { '4',  '"',  -1 }, { '4',  ')',  -1 }, { '4',  ',',  -1 }, { '4',  '4',  -1 },
    { '4',  '7',  -1 }, { '4',  '\'', -1 }, { '4',  '\\', -1 }, { '4',  'j',  -2 },
    { '5',  ',',  -1 }, { '5',  '4',  -1 }, { '5',  ';',  -1 }, { '5',  'j',  -2 },
    { '6',  ',',  -1 }, { '6',  '4',  -1 }, { '6',  'j',  -2 }, { '7',  ',',  -2 },
    { '7',  '.',  -1 }, { '7',  '4',  -1 }, { '7',  ';',  -1 }, { '7',  'j',  -2 },
    { '8',  ',',  -1 }, { '8',  '4',  -1 }, { '8',  ';',  -1 }, { '8',  'j',  -2 },
    { '9',  ',',  -1 }, { '9',  '4',  -1 }, { '9',  ';',  -1 }, { '9',  'j',  -2 },
    { '?',  ' ',   2 }, { '\'', 'a',  -1 }, { '\'', 'c',  -1 }, { '\'', 'd',  -1 },
    { '\'', 'e',  -1 }, { '\'', 'g',  -1 }, { '\'', 'J',  -2 }, { '\'', 'j',  -2 },
    { '\'', 'o',  -1 }, { '\'', 's',  -1 }, { '\'', 't',  -1 }, { '\\', '\\', -2 },
    { '\\', 'T',  -1 }, { '\\', 't',  -1 }, { '\\', 'V',  -1 }, { '\\', 'v',  -1 },
    { '\t', '-',   1 }, { '\t', '4',  -1 }, { '_',  'f',  -1 }, { '_',  'T',  -1 },
    { '_',  't',  -1 }, { '_',  'V',  -1 }, { '_',  'v',  -1 }, { 'a',  '"',  -1 },
    { 'a',  '\\', -1 }, { 'a',  'j',  -2 }, { 'b',  '"',  -1 }, { 'b',  ',',  -1 },
    { 'b',  ';',  -1 }, { 'b',  '\\', -1 }, { 'b',  'j',  -2 }, { 'c',  '"',  -1 },
    { 'c',  ',',  -1 }, { 'c',  ';',  -1 }, { 'c',  '\\', -1 }, { 'c',  'j',  -2 },
    { 'D',  'j',  -1 }, { 'd',  'j',  -2 }, { 'e',  '"',  -1 }, { 'e',  ')',  -1 },
    { 'e',  ',',  -1 }, { 'e',  ';',  -1 }, { 'e',  '\\', -1 }, { 'e',  '_',  -1 },
    { 'e',  'j',  -2 }, { 'F',  ' ',  -1 }, { 'f',  ' ',  -1 }, { 'F',  ',',  -1 },
    { 'f',  ',',  -1 }, { 'F',  '.',  -1 }, { 'f',  '.',  -1 }, { 'F',  ';',  -1 },
    { 'f',  ';',  -1 }, { 'f',  '_',  -1 }, { 'F',  '_',  -2 }, { 'F',  'a',  -1 },
    { 'f',  'a',  -1 }, { 'F',  'e',  -1 }, { 'f',  'f',  -1 }, { 'F',  'J',  -1 },
    { 'f',  'j',  -2 }, { 'F',  'o',  -1 }, { 'f',  't',  -1 }, { 'h',  '\\', -1 },
    { 'h',  'j',  -2 }, { 'i',  'j',  -2 }, { 'k',  'j',  -2 }, { 'L',  ' ',  -1 },
    { 'L',  '"',  -1 }, { 'L',  '-',  -1 }, { 'L',  '\'', -1 }, { 'L',  '\\', -1 },
    { 'l',  'j',  -2 }, { 'L',  'Y',  -1 }, { 'm',  '"',  -1 }, { 'm',  '\\', -1 },
    { 'm',  'j',  -2 }, { 'n',  '"',  -1 }, { 'n',  '\'', -1 }, { 'n',  '\\', -1 },
    { 'n',  'j',  -2 }, { 'o',  '"',  -1 }, { 'o',  ',',  -1 }, { 'o',  ';',  -1 },
    { 'o',  '\\', -1 }, { 'o',  'j',  -2 }, { 'p',  '"',  -1 }, { 'P',  ',',  -1 },
    { 'p',  ',',  -1 }, { 'P',  '.',  -1 }, { 'P',  ';',  -1 }, { 'p',  ';',  -1 },
    { 'p',  '\\', -1 }, { 'P',  '_',  -1 }, { 'P',  'J',  -1 }, { 'p',  'j',  -2 },
    { 'r',  ' ',  -1 }, { 'r',  '"',  -1 }, { 'r',  ')',  -1 }, { 'r',  ',',  -2 },
    { 'r',  '.',  -2 }, { 'r',  '/',  -1 }, { 'r',  ';',  -1 }, { 'r',  '\\', -1 },
    { 'r',  '_',  -1 }, { 'r',  'a',  -1 }, { 'r',  'j',  -2 }, { 's',  ',',  -1 },
    { 's',  ';',  -1 }, { 's',  'j',  -2 }, { 'T',  ' ',  -1 }, { 'T',  ',',  -1 },
    { 'T',  '.',  -1 }, { 'T',  ';',  -1 }, { 'T',  'a',  -1 }, { 'T',  'e',  -1 },
    { 't',  'j',  -2 }, { 'T',  'o',  -1 }, { 't',  't',  -1 }, { 'u',  'j',  -2 },
    { 'V',  ',',  -1 }, { 'v',  ',',  -1 }, { 'V',  '.',  -1 }, { 'v',  '4',  -1 },
    { 'V',  ';',  -1 }, { 'v',  ';',  -1 }, { 'v',  '_',  -1 }, { 'V',  'a',  -1 },
    { 'v',  'a',  -1 }, { 'v',  'j',  -2 }, { 'w',  'j',  -2 }, { 'x',  '7',  -1 },
    { 'x',  'j',  -2 }, { 'Y',  ',',  -1 }, { 'Y',  '.',  -1 }, { 'Y',  ';',  -1 },
    { 'z',  'j',  -2 }, { '\0', '\0',  0 }
};

int C_TextWidth(const char *text, const bool formatting, const bool kerning)
{
    bool            italics = false;
    bool            monospaced = false;
    const int       len = (int)strlen(text);
    unsigned char   prevletter = '\0';
    int             width = 0;

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];
        unsigned char       nextletter = text[i + 1];

        if (letter == ' ')
            width += spacewidth;
        else if (letter == BOLDONCHAR || letter == BOLDOFFCHAR)
            continue;
        else if (letter == ITALICSONCHAR)
        {
            italics = true;
            continue;
        }
        else if (letter == ITALICSOFFCHAR)
        {
            italics = false;
            continue;
        }
        else if (letter == MONOSPACEDONCHAR)
        {
            monospaced = true;
            continue;
        }
        else if (letter == MONOSPACEDOFFCHAR)
        {
            monospaced = false;
            continue;
        }
        else if (monospaced)
            width += zerowidth;
        else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 't'
            && tolower(text[i + 2]) == 'm' && text[i + 3] == ')' && formatting)
        {
            width += SHORT(trademark->width);
            i += 3;
        }
        else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 'a'
            && tolower(text[i + 2]) == 'm' && text[i + 3] == ')' && formatting)
        {
            width += ampmwidth;
            i += 3;
        }
        else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 'p'
            && tolower(text[i + 2]) == 'm' && text[i + 3] == ')' && formatting)
        {
            width += ampmwidth;
            i += 3;
        }
        else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'c' && text[i + 2] == ')' && formatting)
        {
            width += SHORT(copyright->width);
            i += 2;
        }
        else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'r' && text[i + 2] == ')' && formatting)
        {
            width += SHORT(regomark->width);
            i += 2;
        }
        else if (letter == 215 || (letter == 'x' && isdigit(prevletter) && (nextletter == '\0' || isdigit(nextletter))))
            width += SHORT(multiply->width);
        else if (letter == '-' && prevletter == ' ' && !isdigit(nextletter))
            width += SHORT(endash->width);
        else if (!i || prevletter == ' ' || prevletter == '(' || prevletter == '[' || prevletter == '\t')
        {
            if (letter == '\'')
                width += SHORT(lsquote->width);
            else if (letter == '"')
                width += SHORT(ldquote->width);
            else
            {
                const int   c = letter - CONSOLEFONTSTART;

                width += SHORT((c >= 0 && c < CONSOLEFONTSIZE ? consolefont[c] : unknownchar)->width);
            }
        }
        else
        {
            const int   c = letter - CONSOLEFONTSTART;

            width += SHORT((c >= 0 && c < CONSOLEFONTSIZE ? consolefont[c] : unknownchar)->width);

            if (letter == '-' && italics)
                width++;
        }

        if (kerning)
        {
            for (int j = 0; altkern[j].char1; j++)
                if (prevletter == altkern[j].char1 && letter == altkern[j].char2)
                {
                    width += altkern[j].adjust;
                    break;
                }

            if (prevletter == '/' && italics)
                width -= 2;
        }

        prevletter = letter;
    }

    return width;
}

static int C_OverlayWidth(const char *text, const bool monospaced)
{
    const int   len = (int)strlen(text);
    int         width = 0;

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];

        if (letter == ' ')
            width += spacewidth;
        else if (isdigit(letter))
            width += (monospaced ? zerowidth : SHORT(consolefont[letter - CONSOLEFONTSTART]->width));
        else if (letter >= CONSOLEFONTSTART)
            width += (SHORT(consolefont[letter - CONSOLEFONTSTART]->width) - (letter == ','));
    }

    return width;
}

static void C_DrawScrollbar(void)
{
    scrollbarfacestart = CONSOLESCROLLBARHEIGHT * MAX(0, (outputhistory == -1 ?
        numconsolestrings - CONSOLEBLANKLINES - CONSOLELINES : outputhistory - CONSOLEBLANKLINES)) / numconsolestrings;
    scrollbarfaceend = scrollbarfacestart + CONSOLESCROLLBARHEIGHT - CONSOLESCROLLBARHEIGHT
        * MAX(0, numconsolestrings - CONSOLEBLANKLINES - CONSOLELINES) / numconsolestrings;

    if (!scrollbarfacestart && scrollbarfaceend == CONSOLESCROLLBARHEIGHT)
        scrollbardrawn = false;
    else
    {
        const int   offset = (CONSOLEHEIGHT - consoleheight) * SCREENWIDTH;
        const int   gripstart = (scrollbarfacestart + (scrollbarfaceend - scrollbarfacestart) / 2 - 2) * SCREENWIDTH - offset;
        const int   trackend = MAX(0, CONSOLESCROLLBARHEIGHT * SCREENWIDTH - offset);

        // draw scrollbar track
        for (int y = 0; y < trackend; y += SCREENWIDTH)
            for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
            {
                byte    *dot = *screens + y + x;

                *dot = tinttab50[*dot + consolescrollbartrackcolor];
            }

        if (scrollbarfaceend - scrollbarfacestart > 8)
        {
            const int   gripend = gripstart + 6 * SCREENWIDTH;

            // init scrollbar grip
            for (int y = gripstart; y < gripend; y += 2 * SCREENWIDTH)
                if (y >= 0)
                    for (int x = CONSOLESCROLLBARX + 1; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH - 1; x++)
                        tempscreen[y + x] = screens[0][y + x];

            // draw scrollbar face
            const int   end = scrollbarfaceend * SCREENWIDTH - offset;

            for (int y = scrollbarfacestart * SCREENWIDTH - offset; y < end; y += SCREENWIDTH)
                if (y >= 0)
                    for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                        screens[0][y + x] = consolescrollbarfacecolor;

            // draw scrollbar grip
            for (int y = gripstart; y < gripend; y += 2 * SCREENWIDTH)
                if (y >= 0)
                    for (int x = CONSOLESCROLLBARX + 1; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH - 1; x++)
                        screens[0][y + x] = tempscreen[y + x];
        }
        else
        {
            const int   end = scrollbarfaceend * SCREENWIDTH - offset;

            // draw scrollbar face
            for (int y = scrollbarfacestart * SCREENWIDTH - offset; y < end; y += SCREENWIDTH)
                if (y >= 0)
                    for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                        screens[0][y + x] = consolescrollbarfacecolor;
        }

        // draw scrollbar face shadow
        if (scrollbarfaceend >= 0)
            for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
            {
                byte    *dot = *screens + scrollbarfaceend + x;

                *dot = tinttab10[*dot];
            }

        scrollbardrawn = true;
    }
}

void C_ClearConsole(void)
{
    numconsolestrings = 0;
    consolestringsmax = CONSOLESTRINGSMAX;
    console = I_Realloc(console, consolestringsmax * sizeof(*console));
    memset(console, 0, consolestringsmax * sizeof(*console));

    for (int i = 0; i < CONSOLEBLANKLINES; i++)
    {
        console[numconsolestrings].string[0] = '\0';
        console[numconsolestrings].indent = 0;
        console[numconsolestrings].wrap = 0;
        console[numconsolestrings++].stringtype = outputstring;
    }
}

static void C_InitEdgeColors(void)
{
    consoleedgecolor1 = FindBrightDominantColor(W_CacheLumpName("STTNUM0"));

    if (W_GetNumLumps("STTNUM0") >= 2
        && !(consoleedgecolor1 >= nearestcolors[LIGHTGRAY1] && consoleedgecolor1 <= nearestcolors[DARKGRAY4])
        && consoleedgecolor1 != nearestcolors[WHITE])
    {
        consoleedgecolor2 = black25[consoleedgecolor1] << 8;
        consoleedgecolor1 <<= 8;
    }
    else
    {
        consoleedgecolor1 = FindBrightDominantColor(W_CacheLumpName("M_NGAME"));

        if (W_GetNumLumps("M_NGAME") >= 2
            && !(consoleedgecolor1 >= nearestcolors[LIGHTGRAY1] && consoleedgecolor1 <= nearestcolors[DARKGRAY4])
            && consoleedgecolor1 != nearestcolors[WHITE])
        {
            consoleedgecolor2 = black25[consoleedgecolor1] << 8;
            consoleedgecolor1 <<= 8;
        }
        else
        {
            consoleedgecolor1 = nearestcolors[CONSOLEEDGECOLOR1] << 8;
            consoleedgecolor2 = nearestcolors[CONSOLEEDGECOLOR2] << 8;
        }
    }
}

void C_Init(void)
{
    char    *appdatafolder = M_GetAppDataFolder();
    char    consolefolder[MAX_PATH];
    char    buffer[9];

    M_snprintf(consolefolder, sizeof(consolefolder), "%s" DIR_SEPARATOR_S DOOMRETRO_CONSOLEFOLDER, appdatafolder);
    M_MakeDirectory(consolefolder);
    free(appdatafolder);

    for (int i = 0, j = CONSOLEFONTSTART; i < CONSOLEFONTSIZE; i++)
    {
        M_snprintf(buffer, sizeof(buffer), "DRFON%03i", j++);
        consolefont[i] = W_CacheLastLumpName(W_CheckNumForName(buffer) >= 0 ? buffer : "DRFON000");
    }

    consoleautomapbevelcolor = &tinttab50[nearestcolors[CONSOLEAUTOMAPBEVELCOLOR] << 8];
    consolebackcolor1 = &tinttab50[nearestcolors[CONSOLEBACKCOLOR] << 8];
    consolebackcolor2 = &tinttab60[nearestblack << 8];
    consolebevelcolor1 = &tinttab50[nearestcolors[CONSOLEBEVELCOLOR] << 8];
    consolebevelcolor2 = &tinttab20[nearestcolors[CONSOLEBEVELCOLOR] << 8];
    consoleboldcolor = nearestcolors[CONSOLEBOLDCOLOR];
    consolebolditalicscolor = nearestcolors[CONSOLEBOLDITALICSCOLOR];
    consolecaretcolor = nearestcolors[CONSOLECARETCOLOR];
    consoledividercolor = nearestcolors[CONSOLEDIVIDERCOLOR] << 8;
    consoleinputcolor = nearestcolors[CONSOLEINPUTCOLOR];
    consoleoutputcolor = nearestcolors[CONSOLEOUTPUTCOLOR];
    consoleplayermessagecolor = (harmony ? 226 : nearestcolors[CONSOLEPLAYERMESSAGECOLOR]);
    consolescrollbarfacecolor = nearestcolors[CONSOLESCROLLBARFACECOLOR];
    consolescrollbartrackcolor = nearestcolors[CONSOLESCROLLBARTRACKCOLOR] << 8;
    consoleselectedinputbackgroundcolor = nearestcolors[CONSOLESELECTEDINPUTBACKGROUNDCOLOR];
    consoleselectedinputcolor = nearestcolors[CONSOLESELECTEDINPUTCOLOR];
    consolewarningboldcolor = nearestcolors[CONSOLEWARNINGBOLDCOLOR];
    consolewarningcolor = nearestcolors[CONSOLEWARNINGCOLOR];

    C_InitEdgeColors();

    consolecolors[inputstring] = consoleinputcolor;
    consolecolors[cheatstring] = consoleinputcolor;
    consolecolors[outputstring] = consoleoutputcolor;
    consolecolors[warningstring] = consolewarningcolor;
    consolecolors[playermessagestring] = consoleplayermessagecolor;
    consolecolors[playerwarningstring] = consolewarningcolor;

    consoleboldcolors[inputstring] = consoleboldcolor;
    consoleboldcolors[cheatstring] = consoleboldcolor;
    consoleboldcolors[outputstring] = consoleboldcolor;
    consoleboldcolors[warningstring] = consolewarningboldcolor;
    consoleboldcolors[playermessagestring] = consoleplayermessagecolor;
    consoleboldcolors[playerwarningstring] = consolewarningboldcolor;

    brand = W_CacheLastLumpName("DRBRAND");

    unknownchar = W_CacheLastLumpName("DRFON000");
    lsquote = consolefont[0x91 - CONSOLEFONTSTART];
    ldquote = consolefont[0x93 - CONSOLEFONTSTART];
    endash = consolefont[0x96 - CONSOLEFONTSTART];
    trademark = consolefont[0x99 - CONSOLEFONTSTART];
    copyright = consolefont[0xA9 - CONSOLEFONTSTART];
    regomark = consolefont[0xAE - CONSOLEFONTSTART];
    degree = consolefont[0xB0 - CONSOLEFONTSTART];
    multiply = consolefont[0xD7 - CONSOLEFONTSTART];

    warning = W_CacheLastLumpName("DRFONWRN");
    altunderscores = W_CacheLastLumpName("DRFONUND");
    altbuddha = W_CacheLumpNameFromResourceWAD("DRBUDDH2");
    fps = W_CacheLastLumpName("DRFONFPS");
    ampm[0] = W_CacheLastLumpName("DRFONAM");
    ampm[1] = W_CacheLastLumpName("DRFONPM");

    bindlist = W_CacheLastLumpName("DRBNDLST");
    cmdlist = W_CacheLastLumpName("DRCMDLST");
    cvarlist = W_CacheLastLumpName("DRCVRLST");
    maplist = W_CacheLastLumpName("DRMAPLST");
    mapstats = W_CacheLastLumpName("DRMAPST");
    playerstats = W_CacheLastLumpName("DRPLYRST");
    thinglist = W_CacheLastLumpName("DRTHNLST");

    brandwidth = SHORT(brand->width);
    brandheight = SHORT(brand->height);
    degreewidth = SHORT(degree->width);
    spacewidth = SHORT(consolefont[' ' - CONSOLEFONTSTART]->width);
    zerowidth = SHORT(consolefont['0' - CONSOLEFONTSTART]->width);
    altbuddhawidth = SHORT(altbuddha->width);
    fpswidth = SHORT(fps->width);
    ampmwidth = SHORT(ampm[0]->width);

    suckswidth = C_OverlayWidth(s_STSTR_SUCKS, false);
    timewidth = C_OverlayWidth("00:00.00", true);

    M_TranslateAutocomplete();
}

void C_ShowConsole(bool reset)
{
    consoleheight = MAX(1, consoleheight);
    consoledirection = 1;
    consoleanim = 0;
    showcaret = true;
    caretwait = 0;

    if (reset)
    {
        consoleinput[0] = '\0';
        caretpos = 0;
        selectstart = 0;
        selectend = 0;
        undolevels = 0;
        inputhistory = -1;
        outputhistory = -1;
    }

    for (int i = 0; i < MAXMOUSEBUTTONS + 2; i++)
        mousebuttons[i] = false;

    if (gamestate == GS_LEVEL)
        I_RestoreMousePointerPosition();

    if (keyboardalwaysrun == KEY_CAPSLOCK && alwaysrun && GetCapsLockState())
#if defined(_WIN32)
    {
        ToggleCapsLockState();
        nokeyevent = true;
    }
#elif defined(X11)
        SetCapsLockState(false);
#endif

    S_StopSounds();
    S_LowerMusicVolume();
    SDL_StartTextInput();
    S_StartSound(NULL, sfx_consol);
}

void C_HideConsole(void)
{
    if (!consoleactive)
        return;

    if (keyboardalwaysrun == KEY_CAPSLOCK && alwaysrun && !GetCapsLockState())
#if defined(_WIN32)
    {
        ToggleCapsLockState();
        nokeyevent = true;
    }
#elif defined(X11)
        SetCapsLockState(true);
#endif

    SDL_StopTextInput();

    consoledirection = -1;
    consoleanim = 0;

    I_SaveMousePointerPosition();

    S_StartSound(viewplayer->mo, sfx_consol);
    S_RestoreMusicVolume();
}

void C_HideConsoleFast(void)
{
    if (!consoleactive)
        return;

    if (keyboardalwaysrun == KEY_CAPSLOCK && alwaysrun && !GetCapsLockState())
#if defined(_WIN32)
    {
        ToggleCapsLockState();
        nokeyevent = true;
    }
#elif defined(X11)
        SetCapsLockState(true);
#endif

    SDL_StopTextInput();

    consoledirection = -1;
    consoleanim = 0;
    consoleheight = 0;
    consoleactive = false;

    I_SaveMousePointerPosition();

    S_RestoreMusicVolume();
}

static void C_DrawBackground(void)
{
    const bool  inverted = ((viewplayer->fixedcolormap == INVERSECOLORMAP) != !r_textures);
    const int   height = (consoleheight + 5) * SCREENWIDTH;
    static byte blurscreen[MAXSCREENAREA];

    // apply random noise to background
    M_BigSeed(666);

    for (int i = height; i >= 0; i--)
        screens[0][i] = colormaps[0][M_BigRandomInt(0, 7) * 256 + screens[0][i]];

    M_BigSeed((unsigned int)time(NULL));

    // blur background
    memcpy(blurscreen, screens[0], height);

    for (int y = 0; y <= height - SCREENWIDTH; y += SCREENWIDTH)
        for (int x = y; x <= y + SCREENWIDTH - 2; x++)
            blurscreen[x] = tinttab50[(blurscreen[x + 1] << 8) + blurscreen[x]];

    for (int y = 0; y <= height - SCREENWIDTH; y += SCREENWIDTH)
        for (int x = y + SCREENWIDTH - 2; x > y; x--)
            blurscreen[x] = tinttab50[(blurscreen[x - 1] << 8) + blurscreen[x]];

    for (int y = height - SCREENWIDTH; y >= SCREENWIDTH; y -= SCREENWIDTH)
        for (int x = y + SCREENWIDTH - 1; x >= y + 1; x--)
            blurscreen[x] = tinttab50[(blurscreen[x - SCREENWIDTH - 1] << 8) + blurscreen[x]];

    for (int y = 0; y <= height - 2 * SCREENWIDTH; y += SCREENWIDTH)
        for (int x = y; x <= y + SCREENWIDTH - 1; x++)
            blurscreen[x] = tinttab50[(blurscreen[x + SCREENWIDTH] << 8) + blurscreen[x]];

    for (int y = height - SCREENWIDTH; y >= SCREENWIDTH; y -= SCREENWIDTH)
        for (int x = y; x <= y + SCREENWIDTH - 1; x++)
            blurscreen[x] = tinttab50[(blurscreen[x - SCREENWIDTH] << 8) + blurscreen[x]];

    for (int y = 0; y <= height - 2 * SCREENWIDTH; y += SCREENWIDTH)
        for (int x = y + SCREENWIDTH - 1; x >= y + 1; x--)
            blurscreen[x] = tinttab50[(blurscreen[x + SCREENWIDTH - 1] << 8) + blurscreen[x]];

    for (int y = height - SCREENWIDTH; y >= SCREENWIDTH; y -= SCREENWIDTH)
        for (int x = y; x <= y + SCREENWIDTH - 2; x++)
            blurscreen[x] = tinttab50[(blurscreen[x - SCREENWIDTH + 1] << 8) + blurscreen[x]];

    // tint background
    if (inverted)
        for (int i = 0; i < height; i++)
            screens[0][i] = consolebackcolor2[blurscreen[i]];
    else
        for (int i = 0; i < height; i++)
            screens[0][i] = consolebackcolor1[blurscreen[i]];

    // apply corrugated glass effect to background
    for (int y = consoleheight % 3; y <= height - 3 * SCREENWIDTH; y += SCREENWIDTH)
    {
        for (int x = y + 2; x < y + SCREENWIDTH - 1; x += 3)
            screens[0][x] = colormaps[0][6 * 256 + screens[0][x + ((x % SCREENWIDTH) ? -1 : 1)]];

        for (int x = (y += SCREENWIDTH) + 1; x < y + SCREENWIDTH - 1; x += 3)
            screens[0][x] = colormaps[0][6 * 256 + screens[0][x + ((x % SCREENWIDTH) ? -1 : 1)]];

        for (int x = (y += SCREENWIDTH); x < y + SCREENWIDTH - 1; x += 3)
            screens[0][x] = colormaps[0][6 * 256 + screens[0][x + ((x % SCREENWIDTH) ? -1 : 1)]];
    }

    // draw bottom edge
    if (con_edgecolor == con_edgecolor_auto)
    {
        V_DrawConsoleBrandingPatch(SCREENWIDTH - MAXWIDESCREENDELTA - brandwidth + (vid_widescreen ? 19 : 44),
            consoleheight - brandheight + 2, brand, consoleedgecolor1, consoleedgecolor2,
            (luminance[consoleedgecolor1 >> 8] <= 128 ? nearestwhite : nearestblack));

        for (int i = height - 3 * SCREENWIDTH; i < height; i++)
            screens[0][i] = tinttab60[consoleedgecolor1 + screens[0][i]];
    }
    else
    {
        const int   color1 = nearestcolors[con_edgecolor] << 8;
        const int   color2 = black25[color1 >> 8] << 8;

        V_DrawConsoleBrandingPatch(SCREENWIDTH - MAXWIDESCREENDELTA - brandwidth + (vid_widescreen ? 19 : 44),
            consoleheight - brandheight + 2, brand, color1, color2,
            (luminance[color1 >> 8] <= 128 ? nearestwhite : nearestblack));

        for (int i = height - 3 * SCREENWIDTH; i < height; i++)
            screens[0][i] = tinttab60[color1 + screens[0][i]];
    }

    // bevel left and right edges
    if (automapactive && am_backcolor == am_backcolor_default)
    {
        for (int i = 0; i < height - 3 * SCREENWIDTH; i += SCREENWIDTH)
            screens[0][i] = consoleautomapbevelcolor[screens[0][i + 1]];

        for (int i = MAX(0, height - 3 * SCREENWIDTH); i < height; i += SCREENWIDTH)
            screens[0][i] = consolebevelcolor1[screens[0][i + 1]];

        for (int i = 0; i < height - (brandheight + 3) * SCREENWIDTH; i += SCREENWIDTH)
            screens[0][i + SCREENWIDTH - 1] = consoleautomapbevelcolor[screens[0][i + SCREENWIDTH - 2]];

        for (int i = MAX(0, height - (brandheight + 3) * SCREENWIDTH); i < height; i += SCREENWIDTH)
            screens[0][i + SCREENWIDTH - 1] = consolebevelcolor1[screens[0][i + SCREENWIDTH - 2]];
    }
    else
        for (int i = 0; i < height; i += SCREENWIDTH)
        {
            screens[0][i] = consolebevelcolor1[screens[0][i + 1]];
            screens[0][i + SCREENWIDTH - 1] = consolebevelcolor1[screens[0][i + SCREENWIDTH - 2]];
        }

    // bevel bottom edge
    if (inverted)
        for (int i = height - SCREENWIDTH + 1; i < height - 1; i++)
            screens[0][i] = consolebevelcolor2[screens[0][i]];
    else
        for (int i = height - SCREENWIDTH + 1; i < height - 1; i++)
            screens[0][i] = consolebevelcolor1[screens[0][i]];

    // draw shadow
    for (int i = SCREENWIDTH; i <= 4 * SCREENWIDTH; i += SCREENWIDTH)
        for (int j = height; j < height + i && j < SCREENAREA; j++)
            screens[0][j] = colormaps[0][4 * 256 + screens[0][j]];
}

static int C_DrawConsoleText(int x, int y, char *text, const int color1, const int color2,
    const int boldcolor, const byte *tinttab, const int tabs[MAXTABS], const bool formatting,
    const bool kerning, const bool wrapped, const int index, unsigned char prevletter,
    unsigned char prevletter2, void consoletextfunc(const int, const int, const patch_t *,
        const int, const int, const int, const bool, const byte *))
{
    bool            bold = false;
    bool            bolder = false;
    bool            italics = false;
    bool            monospaced = false;
    int             tab = -1;
    const int       len = (int)strlen(text);
    int             startx = x;
    unsigned char   prevletter3 = '\0';

    y -= CONSOLEHEIGHT - consoleheight;

    if (console[index].stringtype == warningstring
        || console[index].stringtype == playerwarningstring)
    {
        V_DrawConsoleTextPatch(x - 1, y, warning, WARNINGWIDTH, color1, color2, false, tinttab);
        x += (text[0] == 'T' ? WARNINGWIDTH : WARNINGWIDTH + 1);
    }

    if (M_StringCompare(text, s_STSTR_BUDDHA))
    {
        V_DrawConsoleTextPatch(x, y, altbuddha, altbuddhawidth, color1, color2, false, tinttab);
        return 0;
    }

    for (int i = 0; i < len; i++)
    {
        unsigned char   letter = text[i];
        unsigned char   nextletter = text[i + 1];

        if (cheatsequence)
        {
            letter = '\x95';
            nextletter = '\x95';
        }

        if (letter == BOLDONCHAR)
        {
            if (bold)
                bolder = true;
            else
                bold = true;
        }
        else if (letter == BOLDOFFCHAR)
        {
            if (bolder)
                bolder = false;
            else
                bold = false;
        }
        else if (letter == ITALICSONCHAR)
            italics = true;
        else if (letter == ITALICSOFFCHAR)
            italics = false;
        else if (letter == MONOSPACEDONCHAR)
            monospaced = true;
        else if (letter == MONOSPACEDOFFCHAR)
            monospaced = false;
        else
        {
            patch_t *patch = NULL;

            if (letter == ' ' && formatting)
                x += (monospaced ? zerowidth : spacewidth);
            else if (letter == '\t')
                x = (x > tabs[++tab] + CONSOLETEXTX - 10 ? x + spacewidth : tabs[tab] + CONSOLETEXTX - 10);
            else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 't' && tolower(text[i + 2]) == 'm' && text[i + 3] == ')'
                && formatting)
            {
                patch = trademark;
                i += 3;
            }
            else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 'a' && tolower(text[i + 2]) == 'm' && text[i + 3] == ')'
                && formatting)
            {
                patch = ampm[0];
                i += 3;
            }
            else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 'p' && tolower(text[i + 2]) == 'm' && text[i + 3] == ')'
                && formatting)
            {
                patch = ampm[1];
                i += 3;
            }
            else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'c' && text[i + 2] == ')' && formatting)
            {
                patch = copyright;
                i += 2;
            }
            else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'r' && text[i + 2] == ')' && formatting)
            {
                patch = regomark;
                i += 2;
            }
            else if (letter == 'x' && isdigit(prevletter) && (i == len - 1 || isdigit(nextletter)))
                patch = multiply;
            else if (letter == '-' && (prevletter == ' ' || (prevletter == BOLDONCHAR && prevletter2 == ' '))
                && !isdigit(nextletter))
                patch = endash;
            else if (letter == '\n')
                break;
            else
            {
                const int   c = letter - CONSOLEFONTSTART;

                patch = (c >= 0 && c < CONSOLEFONTSIZE ? consolefont[c] : unknownchar);

                if (letter == '\'')
                {
                    if (prevletter == '\0' || (prevletter == ' ' && prevletter2 != '\'')
                        || prevletter == '\t' || prevletter == '(' || prevletter == '['
                        || prevletter == '{' || prevletter == '<' || prevletter == '"'
                        || ((prevletter == BOLDONCHAR || prevletter == ITALICSONCHAR)
                            && prevletter2 != '.' && nextletter != '.'))
                    {
                        patch = lsquote;

                        if (prevletter == '\0' && formatting)
                            x--;
                    }
                    else if (prevletter == 'I' && italics)
                        x++;
                }
                else if (letter == '"')
                {
                    if (prevletter == '\0' || (prevletter == ' ' && prevletter2 != '"')
                        || prevletter == '\t' || prevletter == '(' || prevletter == '['
                        || prevletter == '{' || prevletter == '<' || prevletter == '\''
                        || ((prevletter == BOLDONCHAR || prevletter == ITALICSONCHAR)
                            && prevletter2 != '.' && nextletter != '.'))
                    {
                        patch = ldquote;

                        if (prevletter == '\0' && formatting)
                            x--;
                    }
                }
            }

            if (kerning && !monospaced)
            {
                for (int j = 0; altkern[j].char1; j++)
                    if (prevletter == altkern[j].char1 && letter == altkern[j].char2)
                    {
                        x += altkern[j].adjust;
                        break;
                    }

                if (italics)
                {
                    if (letter == '-')
                        x++;
                    else if (letter == '\'' && prevletter != 'M')
                        x--;
                    else if (letter == 'v' && prevletter == '-')
                        x--;

                    if (prevletter == '/')
                        x--;
                    else if (prevletter == '\'')
                        x++;

                    if (letter == 'T' && prevletter == ITALICSONCHAR && prevletter2 == ' ')
                        x--;

                    if (prevletter == ITALICSONCHAR && prevletter2 == '\t')
                        x--;
                }
                else if ((letter == '-' || letter == '|' || letter == '[' || letter == ']' || letter == 215)
                    && prevletter == ITALICSOFFCHAR)
                    x++;
                else if (letter == '(' && prevletter == ' ')
                {
                    if (prevletter2 == '.')
                        x--;
                    else if (prevletter2 == '!')
                        x -= 2;
                }
                else if (prevletter == BOLDONCHAR
                    && (prevletter2 == '\t' || (prevletter2 == BOLDONCHAR && prevletter3 == '\t')))
                {
                    if (letter == '"' || letter == '\'' || letter == '(' || letter == '4')
                        x--;
                }
                else if (prevletter == BOLDOFFCHAR)
                {
                    if ((letter == ' ' || letter == ')' || letter == '.') && prevletter2 == 'r')
                        x--;
                    else if (letter == 'f' && prevletter2 == '[')
                        x--;
                    else if (letter == ',' && (prevletter2 == '"' || prevletter2 == '\'' || prevletter2 == 'r'))
                        x -= 2;
                    else if (letter == ',' && (prevletter2 == 'e' || prevletter2 == 'f'))
                        x--;
                    else if (letter == '.' && prevletter2 == '\"')
                        x--;
                }
            }

            if (patch)
            {
                int width = SHORT(patch->width);

                consoletextfunc(x + (monospaced && width <= zerowidth ? (zerowidth - width) / 2 : 0), y,
                    patch, width, (bold && italics ? (color1 == consolewarningcolor ? color1 :
                        consolebolditalicscolor) : (bold ? boldcolor : color1)),
                    color2, (italics && letter != '_' && letter != '-' && letter != '+' && letter != ','
                        && letter != '/' && patch != unknownchar), (bolder ? NULL : tinttab));
                x += (monospaced && width < zerowidth ? zerowidth : width) - (monospaced && letter == '4');

                if (x >= CONSOLETEXTPIXELWIDTH && wrapped)
                {
                    for (int j = 1; j <= 3; j++)
                    {
                        patch = consolefont['.' - CONSOLEFONTSTART];
                        width = SHORT(patch->width);
                        consoletextfunc(x, y, patch, width, (bold && italics ? (color1 == consolewarningcolor ?
                            color1 : consolebolditalicscolor) : (bold ? boldcolor : color1)), color2, false,
                            (bolder ? NULL : tinttab));
                        x += (monospaced ? zerowidth : width);
                    }

                    break;
                }
            }
        }

        prevletter3 = prevletter2;
        prevletter2 = prevletter;
        prevletter = letter;
    }

    return (x - startx);
}

static void C_DrawOverlayText(byte *screen, const int screenwidth, int x,
    const int y, const byte *tinttab, const char *text, const int color,
    const bool monospaced, const int shadowcolor)
{
    const int   len = (int)strlen(text);

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];

        if (letter == ' ')
            x += spacewidth;
        else
        {
            patch_t     *patch = consolefont[letter - CONSOLEFONTSTART];
            const int   width = SHORT(patch->width);

            if (isdigit(letter) && monospaced)
            {
                V_DrawOverlayTextPatch(screen, screenwidth, x + (letter == '1') - (letter == '4'),
                    y, patch, width - 1, color, shadowcolor, tinttab);
                x += zerowidth;
            }
            else
            {
                V_DrawOverlayTextPatch(screen, screenwidth, x - (letter == ','),
                    y, patch, width - 1, color, shadowcolor, tinttab);
                x += (width - (letter == ','));
            }
        }
    }
}

static void C_DrawTimeStamp(int x, const int y, const int index, const int color)
{
    char        buffer[9];
    struct tm   timestamp = console[index].timestamp;

    if (con_timestampformat == con_timestampformat_standard)
    {
        const int   hour = timestamp.tm_hour;

        V_DrawConsoleTextPatch((x -= ampmwidth), y, ampm[hour >= 12], ampmwidth,
            color, NOBACKGROUNDCOLOR, false, tinttab33);

        M_snprintf(buffer, sizeof(buffer), "%i:%02i:%02i",
            (hour ? hour - 12 * (hour > 12) : 12), timestamp.tm_min, timestamp.tm_sec);
    }
    else
        M_snprintf(buffer, sizeof(buffer), "%02i:%02i:%02i",
            timestamp.tm_hour, timestamp.tm_min, timestamp.tm_sec);

    for (int i = (int)strlen(buffer) - 1; i >= 0; i--)
    {
        const char  ch = buffer[i];
        patch_t     *patch = consolefont[ch - CONSOLEFONTSTART];
        const int   width = SHORT(patch->width);

        x -= (ch == ':' ? width : zerowidth);

        V_DrawConsoleTextPatch(x + (ch == '1') - (ch == '4'), y, patch,
            width, color, NOBACKGROUNDCOLOR, false, tinttab33);
    }
}

static byte C_GetOverlayTextColor(void)
{
    if (((viewplayer->fixedcolormap == INVERSECOLORMAP) != !r_textures) && !automapactive && gamestate == GS_LEVEL)
        return nearestblack;
    else if (automapactive)
        return nearestcolors[am_playerstatscolor];
    else
        return nearestwhite;
}

static int C_GetOverlayTextShadowColor(void)
{
    return ((viewplayer->fixedcolormap == INVERSECOLORMAP) != !r_textures && !r_hud_translucency ? -1 : nearestdarkgray);
}

void C_UpdateFPSOverlay(void)
{
    const int       x = SCREENWIDTH - fpswidth - OVERLAYTEXTX;
    const int       color = C_GetOverlayTextColor();
    int             shadowcolor = C_GetOverlayTextShadowColor();
    const byte      *tinttab = (r_hud_translucency ? (automapactive ? tinttab70 : tinttab50) : NULL);
    char            *temp = commify(framespersecond);
    static int      fpshistory[OVERLAYFPSGRAPHWIDTH] = { 0 };
    static int      fpshistoryindex = 0;
    static uint64_t lastupdate = 0;
    static int      maxfpsseen = 60;
    uint64_t        now = I_GetTimeMS();
    int             graphx = SCREENWIDTH - OVERLAYFPSGRAPHWIDTH - OVERLAYTEXTX;
    int             graphy = OVERLAYTEXTY + 1;
    int             prevpx = -1;
    int             prevpy = -1;
    int             pyvals[OVERLAYFPSGRAPHWIDTH];

    if (now - lastupdate >= 1000)
    {
        fpshistory[fpshistoryindex] = framespersecond;
        fpshistoryindex = (fpshistoryindex + 1) % OVERLAYFPSGRAPHWIDTH;
        lastupdate = now;
        maxfpsseen = 0;

        for (int i = 0; i < OVERLAYFPSGRAPHWIDTH; i++)
            if (fpshistory[i] > maxfpsseen)
                maxfpsseen = fpshistory[i];

        if (maxfpsseen > 10)
        {
            int curmax = 0;

            for (int i = 0; i < OVERLAYFPSGRAPHWIDTH; i++)
                if (fpshistory[i] > curmax)
                    curmax = fpshistory[i];

            if (curmax < maxfpsseen)
                maxfpsseen = curmax;
        }
    }

    for (int i = 0; i < OVERLAYFPSGRAPHWIDTH; i++)
    {
        int py = graphy + OVERLAYFPSGRAPHHEIGHT - 1;

        if (maxfpsseen > 0)
        {
            int offset = (fpshistory[(fpshistoryindex + i) % OVERLAYFPSGRAPHWIDTH] * OVERLAYFPSGRAPHHEIGHT) / maxfpsseen;

            if (offset > OVERLAYFPSGRAPHHEIGHT - 1)
                offset = OVERLAYFPSGRAPHHEIGHT - 1;

            py -= offset;

            if (py < graphy)
                py = graphy;
            else if (py > graphy + OVERLAYFPSGRAPHHEIGHT - 1)
                py = graphy + OVERLAYFPSGRAPHHEIGHT - 1;
        }

        pyvals[i] = py;
    }

    for (int yy = graphy; yy < graphy + OVERLAYFPSGRAPHHEIGHT + 1; yy++)
        for (int xx = graphx; xx < graphx + OVERLAYFPSGRAPHWIDTH; xx++)
            tempscreen[yy * SCREENWIDTH + xx] = PINK;

    for (int i = 0; i < OVERLAYFPSGRAPHWIDTH; i++)
    {
        const int   px = graphx + i;
        const int   py = pyvals[i];
        byte        *dest = &tempscreen[py * SCREENWIDTH + px];

        *dest = color;
        dest += SCREENWIDTH;

        if (*dest == PINK)
            *dest = BLUE1;

        if (prevpx >= 0 && prevpy >= 0)
        {
            int dx = px - prevpx;
            int dy = py - prevpy;
            int sx = SIGN(dx);
            int sy = SIGN(dy);
            int err;
            int x0 = prevpx;
            int y0 = prevpy;
            int n;

            dx = ABS(dx);
            dy = ABS(dy);

            if (dx > dy)
            {
                err = dx / 2;
                n = dx;
            }
            else
            {
                err = -dy / 2;
                n = dy;
            }

            for (int j = 0; j < n; j++)
            {
                int e2 = err;

                dest = &tempscreen[y0 * SCREENWIDTH + x0];
                *dest = color;
                dest += SCREENWIDTH;

                if (*dest == PINK)
                    *dest = BLUE1;

                if (e2 > -dx)
                {
                    err -= dy;
                    x0 += sx;
                }

                if (e2 < dy)
                {
                    err += dx;
                    y0 += sy;
                }
            }
        }

        prevpx = px;
        prevpy = py;
    }

    if (tinttab)
    {
        for (int yy = graphy; yy < graphy + OVERLAYFPSGRAPHHEIGHT + 1; yy++)
            for (int xx = graphx; xx < graphx + OVERLAYFPSGRAPHWIDTH; xx++)
            {
                const int   i = yy * SCREENWIDTH + xx;
                const byte  dot = tempscreen[i];

                if (dot == BLUE1)
                {
                    byte    *dest = &screens[0][i];

                    *dest = black10[*dest];
                }
                else if (dot != PINK)
                {
                    byte    *dest = &screens[0][i];

                    *dest = tinttab[(dot << 8) + *dest];
                }
            }
    }
    else
    {
        for (int yy = graphy; yy < graphy + OVERLAYFPSGRAPHHEIGHT + 1; yy++)
            for (int xx = graphx; xx < graphx + OVERLAYFPSGRAPHWIDTH; xx++)
            {
                const int   i = yy * SCREENWIDTH + xx;
                const byte  dot = tempscreen[i];

                if (dot == BLUE1)
                {
                    if (color != shadowcolor && shadowcolor != -1)
                        screens[0][i] = shadowcolor;
                }
                else if (dot != PINK)
                    screens[0][i] = dot;
            }
    }

    V_DrawOverlayTextPatch(screens[0], SCREENWIDTH, x,
        (graphy += OVERLAYFPSGRAPHHEIGHT + 2) + 3, fps, fpswidth, color, shadowcolor, tinttab);
    C_DrawOverlayText(screens[0], SCREENWIDTH, x - C_OverlayWidth(temp, true) - 3,
        graphy, tinttab, temp, color, true, shadowcolor);
    free(temp);
}

void C_UpdateTimerOverlay(void)
{
    static char buffer[16];
    static int  prevtime = -1;
    int         y = OVERLAYTEXTY;

    if (vid_showfps && framespersecond)
        y += OVERLAYFPSGRAPHHEIGHT + 3 + OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (timeremaining != prevtime)
    {
        int         seconds = (prevtime = timeremaining) / TICRATE;
        const int   millseconds = (prevtime % TICRATE) * (1000 / TICRATE);
        const int   hours = seconds / 3600;
        const int   minutes = ((seconds %= 3600)) / 60;

        if (hours)
            M_snprintf(buffer, sizeof(buffer), "%i:%02i:%02i.%02i",
                hours, minutes, seconds % 60, millseconds / 10);
        else
            M_snprintf(buffer, sizeof(buffer), "%02i:%02i.%02i",
                minutes, seconds % 60, millseconds / 10);

        timerwidth = C_OverlayWidth(buffer, true);
    }

    C_DrawOverlayText(screens[0], SCREENWIDTH, SCREENWIDTH - timerwidth - OVERLAYTEXTX + 1, y,
        (r_hud_translucency ? (automapactive ? tinttab70 : tinttab50) : NULL), buffer,
        C_GetOverlayTextColor(), true, C_GetOverlayTextShadowColor());
}

void C_UpdatePlayerPositionOverlay(void)
{
    const int   x = SCREENWIDTH - OVERLAYTEXTX + 1;
    int         y = OVERLAYTEXTY;
    const byte  color = C_GetOverlayTextColor();
    const byte  shadowcolor = C_GetOverlayTextShadowColor();
    const byte  *tinttab = (r_hud_translucency ? (automapactive ? tinttab70 : tinttab50) : NULL);
    static char angle[32];
    static char coordinates[32];

    if (vid_showfps && framespersecond)
        y += OVERLAYFPSGRAPHHEIGHT + 3 + OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (timer)
        y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (automapactive && !am_followmode)
    {
        const mpoint_t  center = am_frame.center;
        const int       xx = center.x >> MAPBITS;
        const int       yy = center.y >> MAPBITS;

        M_snprintf(angle, sizeof(angle), "%i\xB0", am_frame.angle);
        M_snprintf(coordinates, sizeof(coordinates), "(%i, %i, %i)", xx, yy,
            R_PointInSubsector(xx << FRACBITS, yy << FRACBITS)->sector->floorheight >> FRACBITS);
    }
    else
    {
        const int       an = (int)(viewangle * 90.0 / ANG90);
        const mobj_t    *mo = viewplayer->mo;
        fixed_t         z = MAX(mo->floorz, mo->z);

        if ((mo->flags2 & MF2_FEETARECLIPPED) && r_liquid_lowerview)
            z -= FOOTCLIPSIZE;

        M_snprintf(angle, sizeof(angle), "%i\xB0", (an == 360 ? 0 : an));
        M_snprintf(coordinates, sizeof(coordinates), "(%i, %i, %i)",
            viewx >> FRACBITS, viewy >> FRACBITS, z >> FRACBITS);
    }

    C_DrawOverlayText(screens[0], SCREENWIDTH, x - C_OverlayWidth(angle, true), y,
        tinttab, angle, color, true, shadowcolor);
    C_DrawOverlayText(screens[0], SCREENWIDTH, x - C_OverlayWidth(coordinates, true),
        y + OVERLAYLINEHEIGHT, tinttab, coordinates, color, true, shadowcolor);
}

void C_UpdatePathOverlay(void)
{
    static int  prevdistancetraveled = -1;
    static char distance[20];
    static int  width;

    if (viewplayer->distancetraveled != prevdistancetraveled)
    {
        char    *temp = C_DistanceTraveled(viewplayer->distancetraveled, false);

        M_StringCopy(distance, temp, sizeof(distance));
        free(temp);

        width = C_OverlayWidth(distance, true);
    }

    if (*distance)
    {
        int x;
        int y;

        if (mapwindow)
        {
            x = MAPWIDTH - MAPOVERLAYTEXTX + 1;
            y = MAPOVERLAYTEXTY;
        }
        else
        {
            x = MAPWIDTH - OVERLAYTEXTX + 1;
            y = OVERLAYTEXTY;

            if (vid_showfps && framespersecond)
                y += OVERLAYFPSGRAPHHEIGHT + 3 + OVERLAYLINEHEIGHT + OVERLAYSPACING;

            if (timer)
                y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

            if (viewplayer->cheats & CF_MYPOS)
                y += OVERLAYLINEHEIGHT * 2 + OVERLAYSPACING;
        }

        C_DrawOverlayText(mapscreen, MAPWIDTH, x - width, y, (r_hud_translucency ? tinttab70 : NULL),
            distance, C_GetOverlayTextColor(), true, C_GetOverlayTextShadowColor());

        pathoverlay = true;
    }
    else
        pathoverlay = false;
}

void C_UpdatePlayerStatsOverlay(void)
{
    int         x;
    int         y;
    const byte  color = C_GetOverlayTextColor();
    const byte  shadowcolor = C_GetOverlayTextShadowColor();
    const byte  *tinttab = (r_hud_translucency ? tinttab70 : NULL);
    static char buffer[16];
    static int  prevmaptime = -1;
    static int  width;

    if (mapwindow)
    {
        x = MAPWIDTH - MAPOVERLAYTEXTX + 1;
        y = MAPOVERLAYTEXTY;
    }
    else
    {
        x = MAPWIDTH - OVERLAYTEXTX + 1;
        y = OVERLAYTEXTY;

        if (vid_showfps && framespersecond)
            y += OVERLAYFPSGRAPHHEIGHT + 2 + OVERLAYLINEHEIGHT + OVERLAYSPACING;

        if (timer)
            y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

        if (viewplayer->cheats & CF_MYPOS)
            y += OVERLAYLINEHEIGHT * 2 + OVERLAYSPACING;
    }

    if (pathoverlay)
        y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (maptime != prevmaptime)
    {
        int         seconds = (prevmaptime = maptime) / TICRATE;
        const int   millseconds = (prevmaptime % TICRATE) * (1000 / TICRATE);
        const int   hours = seconds / 3600;
        const int   minutes = ((seconds %= 3600)) / 60;

        if (hours)
        {
            if (sucktime && hours >= sucktime)
            {
                M_StringCopy(buffer, s_STSTR_SUCKS, sizeof(buffer));
                width = suckswidth;
            }
            else
            {
                M_snprintf(buffer, sizeof(buffer), "%i:%02i:%02i.%02i",
                    hours, minutes, seconds % 60, millseconds / 10);
                width = C_OverlayWidth(buffer, true);
            }
        }
        else
        {
            M_snprintf(buffer, sizeof(buffer), "%02i:%02i.%02i", minutes, seconds % 60, millseconds / 10);
            width = timewidth;
        }
    }

    C_DrawOverlayText(mapscreen, MAPWIDTH, x - width, y, tinttab, buffer, color, true, shadowcolor);
    y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (totalkills)
    {
        char    kills[32];
        char    *temp1 = commify(viewplayer->killcount);
        char    *temp2 = commify(totalkills);

        M_snprintf(kills, sizeof(kills), s_STSTR_KILLS, temp1, temp2);
        C_DrawOverlayText(mapscreen, MAPWIDTH, x - C_OverlayWidth(kills, false), y,
            tinttab, kills, color, false, shadowcolor);
        free(temp1);
        free(temp2);

        y += OVERLAYLINEHEIGHT;
    }

    if (totalitems)
    {
        char    items[32];
        char    *temp1 = commify(viewplayer->itemcount);
        char    *temp2 = commify(totalitems);

        M_snprintf(items, sizeof(items), s_STSTR_ITEMS, temp1, temp2);
        C_DrawOverlayText(mapscreen, MAPWIDTH, x - C_OverlayWidth(items, false), y,
            tinttab, items, color, false, shadowcolor);
        free(temp1);
        free(temp2);

        y += OVERLAYLINEHEIGHT;
    }

    if (totalsecrets)
    {
        char    secrets[32];
        char    *temp1 = commify(viewplayer->secretcount);
        char    *temp2 = commify(totalsecrets);

        M_snprintf(secrets, sizeof(secrets), s_STSTR_SECRETS, temp1, temp2);
        C_DrawOverlayText(mapscreen, MAPWIDTH, x - C_OverlayWidth(secrets, false), y,
            tinttab, secrets, color, false, shadowcolor);
        free(temp1);
        free(temp2);
    }
}

static bool IsCheatSequence(char *string)
{
    const int   length = (int)strlen(string);

    if (M_StringStartsWith(string, cheat_clev_xy.sequence)
        && length == strlen(cheat_clev_xy.sequence) + cheat_clev_xy.parameter_chars
        && isdigit(string[length - 1])
        && isdigit(string[length - 2]))
    {
        static char lump[7];

        if (legacyofrust)
        {
            int ep = string[length - 2] - '0';
            int map = string[length - 1] - '0';

            if (ep <= 2 && map <= 7)
                map = (ep - 1) * 7 + map;
            else if (ep <= 2 && map == 8)
                map = 14 + ep;
            else
                map = ep * 10 + map;

            M_snprintf(lump, sizeof(lump), "MAP%02i", map);
        }
        else if (gamemode == commercial)
            M_snprintf(lump, sizeof(lump), "MAP%c%c", string[length - 2], string[length - 1]);
        else
            M_snprintf(lump, sizeof(lump), "E%cM%c", string[length - 2], string[length - 1]);

        return (W_CheckNumForName(lump) >= 0);
    }
    else if (M_StringStartsWith(string, cheat_mus_xy.sequence)
        && length == strlen(cheat_mus_xy.sequence) + cheat_mus_xy.parameter_chars
        && isdigit(string[length - 1])
        && isdigit(string[length - 2]))
        return (!nomusic && musicvolume);
    else if (gamestate != GS_LEVEL)
        return false;
    else if (M_StringCompare(string, cheat_god.sequence))
        return (gameskill != sk_nightmare);
    else if (M_StringCompare(string, cheat_ammonokey.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_ammo.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_noclip.sequence))
        return (gamemode != commercial && gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_commercial_noclip.sequence))
        return (gamemode == commercial && gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_powerup[0].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_powerup[1].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_powerup[2].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_powerup[3].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_powerup[4].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_powerup[5].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_choppers.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_buddha.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(string, cheat_mypos.sequence))
        return true;
    else if (M_StringCompare(string, cheat_amap.sequence))
        return (gameskill != sk_nightmare && (automapactive || mapwindow));

    return false;
}

void C_Drawer(void)
{
    int             i;
    int             x = CONSOLEINPUTX;
    int             y = CONSOLELINEHEIGHT * (CONSOLELINES - 1) - CONSOLELINEHEIGHT / 2 + 1;
    const int       bottomline = (outputhistory == -1 ? numconsolestrings : outputhistory + CONSOLELINES) - 1;
    int             len;
    const bool      prevconsoleactive = consoleactive;
    static uint64_t consolewait;
    const uint64_t  tics = I_GetTimeMS();
    const int       notabs[MAXTABS] = { 0 };
    unsigned char   prevletter = '\0';
    unsigned char   prevletter2 = '\0';

    cheatsequence = false;

    // adjust console height
    if (consolewait < tics)
    {
        consolewait = tics + (gamestate == GS_TITLESCREEN ? 4 : 8);

        if (consoledirection == 1)
        {
            if (consoleheight < CONSOLEHEIGHT)
            {
                const int consoledown[CONSOLEDOWNSIZE] =
                {
                     12,  29,  45,  60,  84,  97, 109, 120, 130, 139, 147, 154, 160, 165,
                    169, 173, 176, 179, 182, 184, 186, 188, 190, 192, 194, 194, 195, 195
                };

                const int   height = (gamestate == GS_TITLESCREEN ? consoledown[consoleanim] * 2 + 5 :
                                consoledown[consoleanim]);

                if (consoleheight > height)
                    consolewait = 0;
                else
                    consoleheight = height;

                consoleactive = (consoleanim++ > CONSOLEDOWNSIZE / 2);
            }
            else
                consoleactive = true;
        }
        else
        {
            if (consoleheight)
            {
                const int consoleup[CONSOLEUPSIZE] =
                {
                    183, 167, 150, 133, 117, 100,  83,  67,  50,  33,  17,   0
                };

                const int   height = consoleup[consoleanim] * (gamestate == GS_TITLESCREEN ? 2 : 1);

                if (consoleheight < height)
                    consolewait = 0;
                else
                    consoleheight = height;

                consoleactive = (consoleanim++ < CONSOLEUPSIZE / 2);
            }
            else
                consoleactive = false;
        }
    }

    if (vid_motionblur && consoleheight < CONSOLEHEIGHT)
        I_SetMotionBlur(0);

    // cancel any controller rumble
    if (!prevconsoleactive
        && (joy_rumble_barrels || joy_rumble_damage || joy_rumble_fall || joy_rumble_pickup || joy_rumble_weapons))
    {
        if (consoleactive)
        {
            restoredrumblestrength = idlechainsawrumblestrength;
            idlechainsawrumblestrength = 0;
        }
        else
            idlechainsawrumblestrength = restoredrumblestrength;

        I_ControllerRumble(idlechainsawrumblestrength, idlechainsawrumblestrength);
    }

    // cancel any screen shake
    I_UpdateBlitFunc(false);

    // draw background and bottom edge
    C_DrawBackground();

    // draw the scrollbar
    C_DrawScrollbar();

    topofconsole = false;

    // draw console text
    for (i = bottomline; i >= 0; i--)
    {
        const stringtype_t  stringtype = console[i].stringtype;

        if (stringtype == dividerstring)
        {
            int yy = (y + 5 - (CONSOLEHEIGHT - consoleheight)) * SCREENWIDTH;

            if (yy >= 0)
                for (int xx = yy + CONSOLETEXTX; xx < yy + CONSOLETEXTPIXELWIDTH + CONSOLETEXTX + 7; xx++)
                {
                    byte    *dest = &screens[0][xx];

                    *dest = tinttab50[consoledividercolor + *dest];
                }

            if ((yy += SCREENWIDTH) >= 0)
            {
                const byte  *tinttab = (!yy ? tinttab40 : tinttab50);

                for (int xx = yy + CONSOLETEXTX; xx < yy + CONSOLETEXTPIXELWIDTH + CONSOLETEXTX + 7; xx++)
                {
                    byte    *dest = &screens[0][xx];

                    *dest = tinttab[consoledividercolor + *dest];
                }
            }
        }
        else if (!(topofconsole = !((len = (int)strlen(console[i].string)))))
        {
            int     wrap = len;
            char    *text;

            if (console[i].wrap)
                wrap = console[i].wrap;
            else
            {
                const int   indent = console[i].indent;

                do
                {
                    char    *temp = M_SubString(console[i].string, 0, wrap);
                    int     width = indent;

                    if (stringtype == warningstring || stringtype == playerwarningstring || !indent)
                        width += C_TextWidth(temp, true, true);
                    else
                        width += C_TextWidth(strrchr(temp, '\t') + 1, true, true);

                    free(temp);

                    if (width <= CONSOLETEXTPIXELWIDTH && isbreak(console[i].string[wrap]))
                    {
                        if (console[i].string[wrap] == '-')
                            wrap++;

                        break;
                    }
                } while (wrap-- > 0);

                console[i].wrap = wrap;
            }

            if (wrap < len)
            {
                text = M_SubString(console[i].string, 0, wrap);

                if (i < bottomline)
                    y -= CONSOLELINEHEIGHT;
            }
            else
                text = M_StringDuplicate(console[i].string);

            if (stringtype == playermessagestring)
            {
                const int   count = console[i].count;

                if (count > 1)
                {
                    char    buffer[CONSOLETEXTMAXLENGTH];
                    char    *temp = commify(count);

                    M_snprintf(buffer, sizeof(buffer), "%s (%s)", text, temp);
                    C_DrawConsoleText(CONSOLETEXTX, y, buffer, consoleplayermessagecolor, NOBACKGROUNDCOLOR,
                        consoleplayermessagecolor, tinttab66, notabs, true, true, false, i, '\0', '\0',
                        &V_DrawConsoleTextPatch);
                    free(temp);
                }
                else
                    C_DrawConsoleText(CONSOLETEXTX, y, text, consoleplayermessagecolor, NOBACKGROUNDCOLOR,
                        consoleplayermessagecolor, tinttab66, notabs, true, true, false, i, '\0', '\0',
                        &V_DrawConsoleTextPatch);

                if (con_timestamps)
                    C_DrawTimeStamp(SCREENWIDTH - CONSOLETEXTX - CONSOLESCROLLBARWIDTH - 7,
                        y - (CONSOLEHEIGHT - consoleheight), i, consoleplayermessagecolor);

            }
            else if (stringtype == outputstring)
                C_DrawConsoleText(CONSOLETEXTX, y, text, consoleoutputcolor, NOBACKGROUNDCOLOR,
                    consoleboldcolor, tinttab66, console[i].tabs, true, true, false, i, '\0', '\0',
                    &V_DrawConsoleTextPatch);
            else if (stringtype == inputstring || stringtype == cheatstring)
                C_DrawConsoleText(CONSOLETEXTX, y, text, consoleinputcolor, NOBACKGROUNDCOLOR,
                    consoleboldcolor, tinttab75, notabs, true, true, false, i, '\0', '\0',
                    &V_DrawConsoleTextPatch);
            else if (stringtype == warningstring)
            {
                const int   count = console[i].count;

                if (count > 1)
                {
                    char    buffer[CONSOLETEXTMAXLENGTH];
                    char    *temp = commify(count);

                    M_snprintf(buffer, sizeof(buffer), "%s (%s)", text, temp);
                    C_DrawConsoleText(CONSOLETEXTX, y, buffer, consolewarningcolor, NOBACKGROUNDCOLOR,
                        consolewarningboldcolor, tinttab66, notabs, true, true, false, i, '\0', '\0',
                        &V_DrawConsoleTextPatch);
                    free(temp);
                }
                else
                    C_DrawConsoleText(CONSOLETEXTX, y, text, consolewarningcolor, NOBACKGROUNDCOLOR,
                        consolewarningboldcolor, tinttab66, notabs, true, true, false, i, '\0', '\0',
                        &V_DrawConsoleTextPatch);
            }
            else if (stringtype == playerwarningstring)
            {
                const int   count = console[i].count;

                if (count > 1)
                {
                    char    buffer[CONSOLETEXTMAXLENGTH];
                    char    *temp = commify(count);

                    M_snprintf(buffer, sizeof(buffer), "%s (%s)", text, temp);
                    C_DrawConsoleText(CONSOLETEXTX, y, buffer, consolewarningcolor, NOBACKGROUNDCOLOR,
                        consolewarningboldcolor, tinttab66, notabs, true, true, false, i, '\0', '\0',
                        &V_DrawConsoleTextPatch);
                    free(temp);
                }
                else
                    C_DrawConsoleText(CONSOLETEXTX, y, text, consolewarningcolor, NOBACKGROUNDCOLOR,
                        consolewarningboldcolor, tinttab66, notabs, true, true, false, i, '\0', '\0',
                        &V_DrawConsoleTextPatch);

                if (con_timestamps)
                    C_DrawTimeStamp(SCREENWIDTH - CONSOLETEXTX - CONSOLESCROLLBARWIDTH - 7,
                        y - (CONSOLEHEIGHT - consoleheight), i, consolewarningboldcolor);
            }
            else if (con_edgecolor == con_edgecolor_auto)
                V_DrawConsoleHeaderPatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight),
                    console[i].header, CONSOLETEXTPIXELWIDTH + 7, consoleedgecolor1,
                    (luminance[consoleedgecolor1 >> 8] <= 128 ? nearestwhite : nearestblack));
            else
                V_DrawConsoleHeaderPatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight),
                    console[i].header, CONSOLETEXTPIXELWIDTH + 7, (nearestcolors[con_edgecolor] << 8),
                    (luminance[nearestcolors[con_edgecolor]] <= 128 ? nearestwhite : nearestblack));

            if (wrap < len && i < bottomline)
            {
                char    *temp = M_SubString(console[i].string, wrap, (size_t)len - wrap);
                bool    bold = false;
                bool    italics = false;

                for (int j = 1; j < (int)strlen(temp); j++)
                    if (temp[j] == BOLDONCHAR)
                        break;
                    else if (temp[j] == BOLDOFFCHAR)
                    {
                        bold = true;
                        break;
                    }

                if (bold)
                {
                    char *oldtemp = temp;
                    temp = M_StringJoin(BOLDON, temp, NULL);
                    free(oldtemp);
                }

                for (int j = 1; j < (int)strlen(temp); j++)
                    if (temp[j] == ITALICSONCHAR)
                        break;
                    else if (temp[j] == ITALICSOFFCHAR)
                    {
                        italics = true;
                        break;
                    }

                if (italics)
                {
                    char *oldtemp = temp;
                    temp = M_StringJoin(ITALICSON, temp, NULL);
                    free(oldtemp);
                }

                C_DrawConsoleText(CONSOLETEXTX + console[i].indent, y + CONSOLELINEHEIGHT,
                    trimwhitespace(temp), consolecolors[stringtype], NOBACKGROUNDCOLOR,
                    consoleboldcolors[stringtype], tinttab66, notabs, true, true, true, 0, '\0', '\0',
                    &V_DrawConsoleTextPatch);
                free(temp);
            }

            free(text);
        }

        if ((y -= CONSOLELINEHEIGHT) < -CONSOLELINEHEIGHT)
        {
            while (i + 1 < numconsolestrings && !strlen(console[++i].string))
                outputhistory++;

            break;
        }
    }

    if (quitcmd)
        return;

    if (consoleinput[0] != '\0')
    {
        char    partialinput[255] = "";

        // draw input text to left of caret
        for (i = 0; i < MIN(selectstart, caretpos); i++)
            partialinput[i] = consoleinput[i];

        cheatsequence = IsCheatSequence(consoleinput);
        partialinput[i] = '\0';

        if (partialinput[0] != '\0')
        {
            x += C_DrawConsoleText(x, CONSOLEINPUTY, partialinput, consoleinputcolor,
                NOBACKGROUNDCOLOR, NOBOLDCOLOR, NULL, notabs, false, true, false, 0, '\0', '\0',
                &V_DrawConsoleTextPatch);

            if (strlen(partialinput) > 0)
                prevletter = partialinput[strlen(partialinput) - 1];

            if (strlen(partialinput) > 1)
                prevletter2 = partialinput[strlen(partialinput) - 2];
        }

        // draw any selected text to left of caret
        if (selectstart < caretpos)
        {
            for (i = selectstart; i < selectend; i++)
                partialinput[i - selectstart] = consoleinput[i];

            partialinput[i - selectstart] = '\0';

            if (partialinput[0] != '\0')
            {
                for (i = 1; i < CONSOLELINEHEIGHT - 1; i++)
                {
                    const int   yy = CONSOLEINPUTY + i - (CONSOLEHEIGHT - consoleheight);

                    if (yy >= 0)
                        screens[0][yy * SCREENWIDTH + x - 1] = consoleselectedinputbackgroundcolor;
                }

                x += C_DrawConsoleText(x, CONSOLEINPUTY, partialinput, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false,
                    true, false, 0, prevletter, prevletter2, &V_DrawConsoleSelectedTextPatch);

                for (i = 1; i < CONSOLELINEHEIGHT - 1; i++)
                {
                    const int   yy = CONSOLEINPUTY + i - (CONSOLEHEIGHT - consoleheight);

                    if (yy >= 0)
                        screens[0][yy * SCREENWIDTH + x] = consoleselectedinputbackgroundcolor;
                }
            }
        }
    }

    // draw caret
    if (consoleheight == CONSOLEHEIGHT && windowfocused && !messagetoprint)
    {
        if (caretwait < tics)
        {
            showcaret = !showcaret;
            caretwait = tics + CARETBLINKTIME;
        }

        if (showcaret)
        {
            byte    *dest = &screens[0][CONSOLEINPUTY * SCREENWIDTH + x];

            for (int yy = 0; yy < CONSOLELINEHEIGHT * SCREENWIDTH; yy += SCREENWIDTH)
            {
                *(dest + yy) = consolecaretcolor;
                *(dest + yy + 1) = consolecaretcolor;
            }
        }
    }
    else
    {
        showcaret = false;
        caretwait = 0;
    }

    if (consoleinput[0] != '\0')
    {
        x += 3;

        // draw any selected text to right of caret
        if (selectend > caretpos)
        {
            char    partialinput[255] = "";

            for (i = selectstart; i < selectend; i++)
                partialinput[i - selectstart] = consoleinput[i];

            partialinput[i - selectstart] = '\0';

            if (partialinput[0] != '\0')
            {
                for (i = 1; i < CONSOLELINEHEIGHT - 1; i++)
                {
                    const int   yy = CONSOLEINPUTY + i - (CONSOLEHEIGHT - consoleheight);

                    if (yy >= 0)
                        screens[0][yy * SCREENWIDTH + x - 1] = consoleselectedinputbackgroundcolor;
                }

                x += C_DrawConsoleText(x, CONSOLEINPUTY, partialinput, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false, true,
                    false, 0, prevletter, prevletter2, &V_DrawConsoleSelectedTextPatch);

                for (i = 1; i < CONSOLELINEHEIGHT - 1; i++)
                {
                    const int   yy = CONSOLEINPUTY + i - (CONSOLEHEIGHT - consoleheight);

                    if (yy >= 0)
                        screens[0][yy * SCREENWIDTH + x] = consoleselectedinputbackgroundcolor;
                }
            }
        }

        // draw input text to right of caret
        if (caretpos < (len = (int)strlen(consoleinput)))
        {
            char    partialinput[255] = "";

            for (i = selectend; i < len; i++)
                partialinput[i - selectend] = consoleinput[i];

            partialinput[i - selectend] = '\0';

            if (partialinput[0] != '\0')
                C_DrawConsoleText(x, CONSOLEINPUTY, partialinput, consoleinputcolor, NOBACKGROUNDCOLOR,
                    NOBOLDCOLOR, NULL, notabs, false, true, false, 0, prevletter, prevletter2,
                    &V_DrawConsoleTextPatch);
        }
    }

    I_Sleep(1);
}

bool C_ExecuteInputString(const char *input)
{
    char    *string = M_StringDuplicate(input);
    char    *strings[255] = { "" };
    int     j = 0;
    bool    makesound = false;

    M_StripQuotes(string);
    strings[0] = strtok(string, ";");

    while (strings[j])
    {
        if (!C_ValidateInput(trimwhitespace(strings[j])))
            break;

        if (M_StringStartsWith(strings[j], "toggle"))
            makesound = true;

        strings[++j] = strtok(NULL, ";");
    }

    if (makesound)
        S_StartSound(NULL, sfx_swtchn);

    free(string);
    return true;
}

bool C_ValidateInput(char *input)
{
    const int   length = (int)strlen(input);

    for (int i = 0; *consolecmds[i].name; i++)
    {
        char    cmd[128] = "";

        if (consolecmds[i].type == CT_CHEAT)
        {
            if (consolecmds[i].parameters)
            {
                if (isdigit((int)input[length - 2]) && isdigit((int)input[length - 1]))
                {
                    consolecheatparm[0] = input[length - 2];
                    consolecheatparm[1] = input[length - 1];
                    consolecheatparm[2] = '\0';

                    M_StringCopy(cmd, input, sizeof(cmd));
                    cmd[length - 2] = '\0';

                    if (M_StringCompare(cmd, consolecmds[i].name)
                        && length == strlen(cmd) + 2
                        && consolecmds[i].func1(consolecmds[i].name, consolecheatparm))
                    {
                        if (gamestate == GS_LEVEL)
                            M_StringCopy(consolecheat, cmd, sizeof(consolecheat));

                        return true;
                    }
                }
            }
            else if (M_StringCompare(input, consolecmds[i].name)
                && consolecmds[i].func1(consolecmds[i].name, ""))
            {
                M_StringCopy(consolecheat, input, sizeof(consolecheat));
                return true;
            }
        }
        else
        {
            char    parms[128] = "";

            if (sscanf(input, "%127s %127[^\n]", cmd, parms) > 0)
            {
                char    *temp = M_StringDuplicate(parms);

                M_StripQuotes(temp);

                if ((M_StringCompare(cmd, consolecmds[i].name)
                    || M_StringCompare(cmd, consolecmds[i].altspelling)
                    || M_StringCompare(cmd, consolecmds[i].alternate))
                    && consolecmds[i].func1(consolecmds[i].name, temp)
                    && (consolecmds[i].parameters || !*temp))
                {
                    if (!executingalias && !resettingcvar && !togglingcvar && !parsingcfgfile)
                    {
                        if (temp[0] != '\0')
                            C_Input((input[length - 1] == '%' ? "%s %s%" : "%s %s"), cmd, parms);
                        else
                            C_Input("%s%s", cmd, (input[length - 1] == ' ' ? " " : ""));
                    }

                    consolecmds[i].func2(consolecmds[i].name, temp);
                    free(temp);

                    return true;
                }

                free(temp);
            }
        }
    }

    if (C_ExecuteAlias(input))
        return true;

    for (int i = 0; *actions[i].action; i++)
        if (M_StringCompare(input, actions[i].action))
        {
            C_Input(input);

            if (actions[i].func)
            {
                if (consoleactive && actions[i].hideconsole)
                    C_HideConsoleFast();

                actions[i].func();
            }

            return true;
        }

    return false;
}

bool C_Responder(event_t *ev)
{
    static int  autocomplete = -1;
    static int  scrollspeed = TICRATE;
    int         i;
    int         len;

    if (quitcmd)
        M_QuitResponse('y');

    if ((consoleheight < CONSOLEHEIGHT && consoledirection == -1) || messagetoprint)
        return false;

    len = (int)strlen(consoleinput);

    if (ev->type == ev_keydown)
    {
        static char         currentinput[255];
        const int           key = ev->data1;
        const SDL_Keymod    modstate = SDL_GetModState();

        switch (key)
        {
            case KEY_BACKSPACE:
                if (selectstart < selectend)
                {
                    // delete selected text
                    C_AddToUndoHistory();

                    for (i = selectend; i < len; i++)
                        consoleinput[selectstart + i - selectend] = consoleinput[i];

                    consoleinput[selectstart + i - selectend] = '\0';
                    caretpos = selectend = selectstart;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }
                else if (caretpos > 0)
                {
                    // delete character left of caret
                    C_AddToUndoHistory();

                    for (i = caretpos - 1; i < len; i++)
                        consoleinput[i] = consoleinput[i + 1];

                    selectend = selectstart = --caretpos;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }

                break;

            case KEY_DELETE:
                if (selectstart < selectend)
                {
                    // delete selected text
                    C_AddToUndoHistory();

                    for (i = selectend; i < len; i++)
                        consoleinput[selectstart + i - selectend] = consoleinput[i];

                    consoleinput[selectstart + i - selectend] = '\0';
                    caretpos = selectend = selectstart;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }
                else if (caretpos < len)
                {
                    // delete character right of caret
                    C_AddToUndoHistory();

                    for (i = caretpos; i < len; i++)
                        consoleinput[i] = consoleinput[i + 1];

                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }

                break;

            case KEY_ENTER:
                // validate input
                if (consoleinput[0] != '\0')
                {
                    bool    result = false;

                    if (M_StringStartsWith(consoleinput, "bind ")
                        || M_StringStartsWith(consoleinput, "unbind ")
                        || M_StringStartsWith(consoleinput, "alias "))
                    {
                        if (C_ValidateInput(consoleinput))
                            result = true;
                    }
                    else
                    {
                        char    *string = M_StringDuplicate(consoleinput);
                        char    *strings[255] = { "" };

                        strings[0] = strtok(string, ";");
                        i = 0;

                        while (strings[i])
                        {
                            if (C_ValidateInput(strings[i]))
                                result = true;

                            strings[++i] = strtok(NULL, ";");
                        }

                        free(string);
                    }

                    if (result)
                    {
                        // clear input
                        consoleinput[0] = '\0';
                        caretpos = 0;
                        selectstart = 0;
                        selectend = 0;
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;
                        undolevels = 0;
                        autocomplete = -1;
                        inputhistory = -1;
                        outputhistory = -1;
                    }
                }
                else
                    C_HideConsole();

                break;

            case KEY_LEFTARROW:
                // move caret left
                if (caretpos > 0)
                {
                    if (modstate & KMOD_SHIFT)
                    {
                        caretpos--;
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;

                        if (selectstart <= caretpos)
                            selectend = caretpos;
                        else
                            selectstart = caretpos;
                    }
                    else
                    {
                        if (selectstart < selectend)
                            caretpos = selectend = selectstart;
                        else
                            selectstart = selectend = --caretpos;

                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;
                    }
                }
                else if (!(modstate & KMOD_SHIFT))
                    caretpos = selectend = selectstart = 0;

                break;

            case KEY_RIGHTARROW:
                // move caret right
                if (caretpos < len)
                {
                    if (modstate & KMOD_SHIFT)
                    {
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;

                        if (selectend >= ++caretpos)
                            selectstart = caretpos;
                        else
                            selectend = caretpos;
                    }
                    else
                    {
                        if (selectstart < selectend)
                            caretpos = selectstart = selectend;
                        else
                            selectstart = selectend = ++caretpos;

                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;
                    }
                }
                else if (!(modstate & KMOD_SHIFT))
                    caretpos = selectend = selectstart = len;

                break;

            case KEY_HOME:
                if ((outputhistory != -1 || !caretpos) && outputhistory && numconsolestrings > CONSOLELINES)
                    outputhistory = 0;  // scroll to top
                else if (caretpos > 0)
                {
                    // move caret to start
                    selectend = ((modstate & KMOD_SHIFT) ? caretpos : 0);
                    caretpos = selectstart = 0;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            case KEY_END:
                if (outputhistory != -1 && numconsolestrings > CONSOLELINES)
                    outputhistory = -1; // scroll to bottom
                else if (caretpos < len)
                {
                    // move caret to end
                    selectstart = ((modstate & KMOD_SHIFT) ? caretpos : len);
                    caretpos = selectend = len;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            case KEY_TAB:
                // autocomplete
                if (consoleinput[0] != '\0' && caretpos == len)
                {
                    const int   scrolldirection = ((modstate & KMOD_SHIFT) ? -1 : 1);
                    const int   start = autocomplete;
                    static char input[255];
                    char        prefix[255] = "";
                    int         spaces1;
                    bool        endspace1;

                    for (i = len - 1; i >= 0; i--)
                        if ((consoleinput[i] == ';'
                            && M_StringStartsWith(consoleinput, "bind")
                            && M_StringStartsWith(consoleinput, "unbind")
                            && M_StringStartsWith(consoleinput, "alias"))
                            || (consoleinput[i] == '"'
                                && M_StringStartsWith(consoleinput, "bind")))
                            break;

                    if (i == len)
                    {
                        if (autocomplete == -1)
                            M_StringCopy(input, consoleinput, sizeof(input));
                    }
                    else
                    {
                        char    *temp1;

                        i += 1 + (consoleinput[i + 1] == ' ');
                        temp1 = M_SubString(consoleinput, 0, i);
                        M_StringCopy(prefix, temp1, sizeof(prefix));
                        free(temp1);

                        if (autocomplete == -1)
                        {
                            char    *temp2 = M_SubString(consoleinput, i, (size_t)len - i);

                            M_StringCopy(input, temp2, sizeof(input));
                            free(temp2);

                            if (!*input)
                                break;
                        }
                    }

                    spaces1 = numspaces(input);
                    endspace1 = (input[strlen(input) - 1] == ' ');

                    while ((scrolldirection == -1 && autocomplete > 0)
                        || (scrolldirection == 1 && (autocomplete == -1 || *autocompletelist[autocomplete].text)))
                    {
                        static char output[255];

                        autocomplete += scrolldirection;
                        M_StringCopy(output, autocompletelist[autocomplete].text, sizeof(output));

                        if (!M_StringCompare(output, input))
                        {
                            const int   game = autocompletelist[autocomplete].game;
                            const int   len2 = (int)strlen(output);
                            const int   spaces2 = numspaces(output);
                            const bool  endspace2 = (len2 > 0 && output[len2 - 1] == ' ');

                            if ((game == DOOM1AND2
                                || (gamemission == pack_plut && game == PLUTONIAONLY)
                                || (gamemission == pack_tnt && game == TNTONLY)
                                || (gamemission == pack_nerve && game == NERVEONLY)
                                || (masterlevels && game == MASTERLEVELSONLY)
                                || (legacyofrust && game == LEGACYOFRUSTONLY)
                                || (gamemission == doom && game == DOOM1ONLY)
                                || (gamemission != doom && game == DOOM2ONLY))
                                && M_StringStartsWith(output, input)
                                && input[strlen(input) - 1] != '+'
                                && ((!spaces1 && (!spaces2 || (spaces2 == 1 && endspace2)))
                                    || (spaces1 == 1 && !endspace1 && (spaces2 == 1 || (spaces2 == 2 && endspace2)))
                                    || (spaces1 == 2 && !endspace1 && (spaces2 == 2 || (spaces2 == 3 && endspace2)))
                                    || (spaces1 == 3 && !endspace1)))
                            {
                                char    *temp;

                                if (isuppercase(input))
                                    temp = M_StringJoin(prefix, M_StringReplaceFirst(uppercase(output), input, input), NULL);
                                else if (islowercase(input))
                                    temp = M_StringJoin(prefix, M_StringReplaceFirst(lowercase(output), input, input), NULL);
                                else
                                    temp = M_StringJoin(prefix, M_StringReplaceFirst(output, input, input), NULL);

                                C_AddToUndoHistory();
                                M_StringCopy(consoleinput, temp, sizeof(consoleinput));
                                caretpos = selectstart = selectend = len2 + (int)strlen(prefix);
                                caretwait = I_GetTimeMS() + CARETBLINKTIME;
                                showcaret = true;
                                free(temp);
                                return true;
                            }
                        }
                    }

                    autocomplete = start;
                }

                break;

            case KEY_UPARROW:
                if ((modstate & KMOD_CTRL) && !topofconsole && numconsolestrings > CONSOLELINES && scrollbardrawn)
                {
                    // scroll output up
                    scrollspeed = MIN(scrollspeed + 4, TICRATE * 8);
                    outputhistory = (outputhistory == -1 ? numconsolestrings - (CONSOLELINES + 1) :
                        MAX(0, outputhistory - scrollspeed / TICRATE));
                }
                else
                {
                    // previous input
                    if (inputhistory == -1)
                        M_StringCopy(currentinput, consoleinput, sizeof(currentinput));

                    for (i = (inputhistory == -1 ? numconsolestrings : inputhistory) - 1; i >= 0; i--)
                        if (console[i].stringtype == inputstring
                            && !M_StringCompare(consoleinput, console[i].string)
                            && C_TextWidth(console[i].string, false, true) <= CONSOLEINPUTPIXELWIDTH)
                        {
                            inputhistory = i;
                            M_StringCopy(consoleinput, console[i].string, sizeof(consoleinput));
                            caretpos = selectstart = selectend = (int)strlen(consoleinput);
                            caretwait = I_GetTimeMS() + CARETBLINKTIME;
                            showcaret = true;
                            break;
                        }
                }

                break;

            case KEY_DOWNARROW:
                if ((modstate & KMOD_CTRL) && outputhistory != -1 && scrollbardrawn)
                {
                    // scroll output down
                    scrollspeed = MIN(scrollspeed + 4, TICRATE * 8);

                    if ((outputhistory += scrollspeed / TICRATE) + CONSOLELINES >= numconsolestrings)
                        outputhistory = -1;
                }
                else
                {
                    // next input
                    if (inputhistory != -1)
                    {
                        for (i = inputhistory + 1; i < numconsolestrings; i++)
                            if (console[i].stringtype == inputstring
                                && !M_StringCompare(consoleinput, console[i].string)
                                && C_TextWidth(console[i].string, false, true) <= CONSOLEINPUTPIXELWIDTH)
                            {
                                inputhistory = i;
                                M_StringCopy(consoleinput, console[i].string, sizeof(consoleinput));
                                break;
                            }

                        if (i == numconsolestrings)
                        {
                            inputhistory = -1;
                            M_StringCopy(consoleinput, currentinput, sizeof(consoleinput));
                        }

                        caretpos = selectstart = selectend = (int)strlen(consoleinput);
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;
                    }
                }

                break;

            case KEY_PAGEUP:
                // scroll output up
                if (!topofconsole && numconsolestrings > CONSOLELINES && scrollbardrawn)
                {
                    scrollspeed = MIN(scrollspeed + 4, TICRATE * 8);
                    outputhistory = (outputhistory == -1 ? numconsolestrings - (CONSOLELINES + 1) :
                        MAX(0, outputhistory - scrollspeed / TICRATE));
                }

                break;

            case KEY_PAGEDOWN:
                // scroll output down
                if (outputhistory != -1 && scrollbardrawn)
                {
                    scrollspeed = MIN(scrollspeed + 4, TICRATE * 8);

                    if ((outputhistory += scrollspeed / TICRATE) + CONSOLELINES >= numconsolestrings)
                        outputhistory = -1;
                }

                break;

            case KEY_ESCAPE:
                // close console
                C_HideConsole();
                break;

            case 'a':
                // select all text
                if (modstate & KMOD_CTRL)
                {
                    selectstart = 0;
                    selectend = caretpos = len;
                }

                break;

            case 'c':
                // copy selected text to clipboard
                if ((modstate & KMOD_CTRL) && selectstart < selectend)
                {
                    char    *temp = M_SubString(consoleinput, selectstart, (size_t)selectend - selectstart);

                    SDL_SetClipboardText(temp);
                    free(temp);
                }

                break;

            case 'v':
                // paste text from clipboard
                if (modstate & KMOD_CTRL)
                {
                    char    buffer[255];
                    char    *temp1 = M_SubString(consoleinput, 0, selectstart);
                    char    *temp2 = M_SubString(consoleinput, selectend, (size_t)len - selectend);

                    M_snprintf(buffer, sizeof(buffer), "%s%s%s", temp1, SDL_GetClipboardText(), temp2);
                    M_StringCopy(buffer, M_StringReplaceFirst(buffer, "(null)", ""), sizeof(buffer));
                    M_StringCopy(buffer, M_StringReplaceFirst(buffer, "(null)", ""), sizeof(buffer));

                    if (C_TextWidth(buffer, false, true) <= CONSOLEINPUTPIXELWIDTH)
                    {
                        C_AddToUndoHistory();
                        M_StringCopy(consoleinput, buffer, sizeof(consoleinput));
                        selectstart += (int)strlen(SDL_GetClipboardText());
                        selectend = caretpos = selectstart;
                    }

                    free(temp1);
                    free(temp2);
                }

                break;

            case 'x':
                // cut selected text to clipboard
                if ((modstate & KMOD_CTRL) && selectstart < selectend)
                {
                    char    *temp = M_SubString(consoleinput, selectstart, (size_t)selectend - selectstart);

                    C_AddToUndoHistory();
                    SDL_SetClipboardText(temp);
                    free(temp);

                    for (i = selectend; i < len; i++)
                        consoleinput[selectstart + i - selectend] = consoleinput[i];

                    consoleinput[selectstart + i - selectend] = '\0';
                    caretpos = selectend = selectstart;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            case 'z':
                // undo
                if ((modstate & KMOD_CTRL) && undolevels)
                {
                    M_StringCopy(consoleinput, undohistory[--undolevels].input, sizeof(consoleinput));
                    caretpos = undohistory[undolevels].caretpos;
                    selectstart = undohistory[undolevels].selectstart;
                    selectend = undohistory[undolevels].selectend;
                }

                break;
        }
    }
    else if (ev->type == ev_keyup)
    {
        keydown = 0;
        scrollspeed = TICRATE;
        return false;
    }
    else if (ev->type == ev_textinput)
    {
        const unsigned char ch = (unsigned char)ev->data1;
        char                *temp = NULL;

        if (ch >= CONSOLEFONTSTART
            && ch != keyboardconsole
            && ch != keyboardconsole2
            && C_TextWidth(consoleinput, false, true)
                + (ch == ' ' ? spacewidth : SHORT(consolefont[ch - CONSOLEFONTSTART]->width))
                - (selectstart < selectend ? C_TextWidth((temp = M_SubString(consoleinput, selectstart,
                (size_t)selectend - selectstart)), false, true) : 0) <= CONSOLEINPUTPIXELWIDTH)
        {
            C_AddToUndoHistory();

            if (selectstart < selectend)
            {
                // replace selected text with a character
                consoleinput[selectstart] = ch;

                for (i = selectend; i < len; i++)
                    consoleinput[selectstart + i - selectend + 1] = consoleinput[i];

                consoleinput[selectstart + i - selectend + 1] = '\0';
                caretpos = selectstart + 1;
            }
            else
            {
                // insert a character
                consoleinput[len + 1] = '\0';

                for (i = len; i > caretpos; i--)
                    consoleinput[i] = consoleinput[i - 1];

                consoleinput[caretpos++] = ch;
            }

            selectstart = selectend = caretpos;
            caretwait = I_GetTimeMS() + CARETBLINKTIME;
            showcaret = true;
            autocomplete = -1;
            inputhistory = -1;
        }

        if (temp)
            free(temp);
    }
    else if (ev->type == ev_mouse)
    {
        if ((ev->data1 & MOUSE_LEFTBUTTON) && usingmouse)
        {
            const int   x = (ev->data2 - (vid_widescreen ? 0 : MAXWIDESCREENDELTA)) * 2;
            const int   y = ev->data3 * 2;

            // hide console
            if (y >= SCREENHEIGHT / 2 && gamestate == GS_LEVEL)
                C_HideConsole();

            // move caret
            else if (len && y >= CONSOLEINPUTY - 2 && y < CONSOLEINPUTY + CONSOLELINEHEIGHT)
            {
                for (i = 0; i < len; i++)
                {
                    char    *temp1 = M_SubString(consoleinput, 0, i);
                    char    *temp2 = M_SubString(consoleinput, i, 1);

                    if (x <= CONSOLEINPUTX + C_TextWidth(temp1, false, true) + C_TextWidth(temp2, false, true) / 2)
                    {
                        free(temp1);
                        free(temp2);
                        break;
                    }

                    free(temp1);
                    free(temp2);
                }

                if (caretpos != i)
                {
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                caretpos = selectstart = selectend = i;
            }
            else if (scrollbardrawn && x >= CONSOLESCROLLBARX && x <= CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH)
            {
                // scroll output up
                if (y < scrollbarfacestart)
                    outputhistory = (outputhistory == -1 ? numconsolestrings - (CONSOLELINES + 1) :
                        MAX(0, outputhistory - scrollspeed / TICRATE));

                // scroll output down
                else if (y > scrollbarfaceend && y < CONSOLESCROLLBARHEIGHT - (CONSOLEHEIGHT - consoleheight))
                {
                    if ((outputhistory += scrollspeed / TICRATE) + CONSOLELINES >= numconsolestrings)
                        outputhistory = -1;
                }
            }
        }

        // hide console
        else if (ev->data1 & MOUSE_RIGHTBUTTON)
            C_HideConsole();
    }
    else if (ev->type == ev_mousewheel && scrollbardrawn)
    {
        // scroll output up
        if (ev->data1 > 0)
        {
            if (!topofconsole && numconsolestrings > CONSOLELINES)
                outputhistory = (outputhistory == -1 ? numconsolestrings - (CONSOLELINES + 1) :
                    MAX(0, outputhistory - 1));
        }

        // scroll output down
        else if (ev->data1 < 0)
        {
            if (outputhistory != -1 && ++outputhistory + CONSOLELINES == numconsolestrings)
                outputhistory = -1;
        }
    }
    else if (ev->type == ev_controller)
    {
        // hide console
        if (((controllerbuttons & controllerconsole)
            || (controllerbuttons & CONTROLLER_B)) && controllerwait < I_GetTime())
        {
            controllerwait = I_GetTime() + 2;
            C_HideConsole();
        }

        // scroll output up
        else if ((controllerthumbLY < 0
            || controllerthumbRY < 0
            || (controllerbuttons & CONTROLLER_DPAD_UP)) && controllerwait < I_GetTime())
        {
            controllerwait = I_GetTime() + 2;

            if (!topofconsole && numconsolestrings > CONSOLELINES)
                outputhistory = (outputhistory == -1 ? numconsolestrings - (CONSOLELINES + 1) :
                    MAX(0, outputhistory - 1));
        }

        // scroll output down
        else if ((controllerthumbLY > 0
            || controllerthumbRY > 0
            || (controllerbuttons & CONTROLLER_DPAD_DOWN)) && controllerwait < I_GetTime())
        {
            controllerwait = I_GetTime() + 2;

            if (outputhistory != -1 && ++outputhistory + CONSOLELINES == numconsolestrings)
                outputhistory = -1;
        }
    }

    return true;
}

#if defined(_WIN32)
void C_PrintCompileDate(void)
{
    char    shortmonthname[4] = "";
    int     minute, hour, day, year;

    if (sscanf(__DATE__, "%3s %2d %4d", shortmonthname, &day, &year) == 3
        && sscanf(__TIME__, "%02d:%02d:%*d", &hour, &minute) == 2)
    {
        const char  shortmonthnames[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
        const int   month = (int)(strstr(shortmonthnames, shortmonthname) - shortmonthnames) / 3;

        C_Output("This %i-bit " ITALICS("%s") " app of " ITALICS("%s")
            " was built with love by %s at %i:%02i%s on %s, %s %i, %i somewhere in %s.",
            8 * (int)sizeof(intptr_t), WINDOWS, DOOMRETRO_NAMEANDVERSIONSTRING, DOOMRETRO_CREATOR,
            (hour ? hour - 12 * (hour > 12) : 12), minute, (hour < 12 ? "(AM)" : "(PM)"),
            dayofweek(day, month + 1, year), monthnames[month], day, year, DOOMRETRO_HOMEOFCREATOR);
    }

#if defined(__clang__)
    C_Output("It was compiled using " ITALICS("Clang v%i.%i.%i."),
        __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__INTEL_COMPILER)
    C_Output("It was compiled using the " ITALICS("Intel C++ Compiler Classic."));
#elif defined(__INTEL_LLVM_COMPILER)
    C_Output("It was compiled using the " ITALICS("Intel C++ Compiler."));
#elif defined(_MSC_FULL_VER) && defined(_MSC_BUILD)
    if (_MSC_BUILD)
        C_Output("It was compiled using v%i.%02i.%i.%i of the " ITALICS("Microsoft C/C++ Optimizing Compiler."),
            _MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000, _MSC_BUILD);
    else
        C_Output("It was compiled using v%i.%02i.%i of the " ITALICS("Microsoft C/C++ Optimizing Compiler."),
            _MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000);
#endif
}
#endif

void C_PrintSDLVersions(void)
{
    C_Output(ITALICS("%s") " is using v%i.%i.%i of the " ITALICS("SDL (Simple DirectMedia Layer)") " library.",
        DOOMRETRO_NAME, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

    C_Output("It is also using v%i.%i.%i of the " ITALICS("SDL_mixer")
        " library and v%i.%i.%i of the " ITALICS("SDL_image") " library.",
        SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL,
        SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);
}
