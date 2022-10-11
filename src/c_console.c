/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_gamecontroller.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_spec.h"
#include "s_sound.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "st_stuff.h"
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
patch_t                 *lsquote;
patch_t                 *rsquote;
patch_t                 *ldquote;
patch_t                 *rdquote;

static patch_t          *brand;
static patch_t          *endash;
static patch_t          *trademark;
static patch_t          *copyright;
static patch_t          *regomark;
static patch_t          *multiply;
static patch_t          *bullet;
static patch_t          *warning;

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

char                    consoleinput[255];
int                     consolestrings = 0;
size_t                  consolestringsmax = 0;

static size_t           undolevels;
static undohistory_t    *undohistory;

static bool             showcaret = true;
static int              caretwait;
int                     caretpos;
int                     selectstart;
int                     selectend;

static bool             wrapbold;
static bool             wrapitalics;
static bool             pathoverlay;

char                    consolecheat[255];
char                    consolecheatparm[3];

static int              inputhistory = -1;
static int              outputhistory = -1;

static int              degreewidth;
static int              suckswidth;
static int              timerwidth;
static int              timewidth;
static int              zerowidth;

static byte             *consoleautomapbevelcolor;
static byte             *consolebackcolor;
static byte             *consolebevelcolor;
static int              consoleboldcolor;
static int              consolebolditalicscolor;
int                     consolebrandingcolor;
static int              consolecaretcolor;
static int              consoledividercolor;
static int              consoleinputcolor;
static int              consoleinputtooutputcolor;
static int              consoleoutputcolor;
static int              consoleoverlaycolor;
static int              consoleoverlaywarningcolor;
static int              consoleplayermessagecolor;
static int              consolescrollbarfacecolor;
static int              consolescrollbartrackcolor;
static int              consoleselectedinputbackgroundcolor;
static int              consoleselectedinputcolor;
static int              consoletimestampcolor;
static int              consolewarningboldcolor;
static int              consolewarningcolor;

static int              consolecolors[STRINGTYPES];
static int              consoleboldcolors[STRINGTYPES];

bool                    scrollbardrawn;

static void (*consoletextfunc)(int, int, patch_t *, int, int, int, bool, byte *);

void C_Input(const char *string, ...)
{
    va_list args;
    char    buffer[CONSOLETEXTMAXLENGTH];

    if (togglingvanilla)
        return;

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[consolestrings].string, buffer, sizeof(console[0].string));
    console[consolestrings].indent = 0;
    console[consolestrings].wrap = 0;
    console[consolestrings++].stringtype = inputstring;
    inputhistory = -1;
    outputhistory = -1;
    consoleinput[0] = '\0';
    caretpos = 0;
    selectstart = 0;
    selectend = 0;
}

void C_Cheat(const char *string)
{
    char        buffer[CONSOLETEXTMAXLENGTH] = "";
    const int   len = (int)strlen(string);

    for (int i = 0; i < len; i++)
        buffer[i] = '\x95';

    buffer[len] = '\0';

    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[consolestrings].string, buffer, sizeof(console[0].string));
    console[consolestrings].indent = 0;
    console[consolestrings].wrap = 0;
    console[consolestrings++].stringtype = cheatstring;
    inputhistory = -1;
    outputhistory = -1;
    consoleinput[0] = '\0';
    caretpos = 0;
    selectstart = 0;
    selectend = 0;
}

void C_IntCVAROutput(const char *cvar, int value)
{
    char    *temp1 = M_StringJoin(cvar, " ", NULL);
    char    *temp2 = commify(value);

    if (consolestrings && M_StringStartsWithExact(console[consolestrings - 1].string, temp1))
        consolestrings--;

    C_Input("%s %s", cvar, temp2);
    free(temp1);
    free(temp2);
}

void C_PctCVAROutput(const char *cvar, int value)
{
    char    *temp1 = M_StringJoin(cvar, " ", NULL);
    char    *temp2 = commify(value);

    if (consolestrings && M_StringStartsWithExact(console[consolestrings - 1].string, temp1))
        consolestrings--;

    C_Input("%s %s%%", cvar, temp2);
    free(temp1);
    free(temp2);
}

void C_StrCVAROutput(const char *cvar, const char *string)
{
    char    *temp = M_StringJoin(cvar, " ", NULL);

    if (consolestrings && M_StringStartsWithExact(console[consolestrings - 1].string, temp))
        consolestrings--;

    C_Input("%s %s", cvar, string);
    free(temp);
}

void C_Output(const char *string, ...)
{
    char    buffer[CONSOLETEXTMAXLENGTH] = "";

    if (string)
    {
        va_list args;

        va_start(args, string);
        M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
        va_end(args);
    }

    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[consolestrings].string, buffer, sizeof(console[0].string));
    console[consolestrings].indent = 0;
    console[consolestrings].wrap = 0;
    console[consolestrings++].stringtype = outputstring;
    outputhistory = -1;
}

bool C_OutputNoRepeat(const char *string, ...)
{
    char    buffer[CONSOLETEXTMAXLENGTH] = "";

    if (string)
    {
        va_list args;

        va_start(args, string);
        M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
        va_end(args);
    }

    if (consolestrings && M_StringCompare(console[consolestrings - 1].string, buffer))
        return true;

    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[consolestrings].string, buffer, sizeof(console[0].string));
    console[consolestrings].indent = 0;
    console[consolestrings].wrap = 0;
    console[consolestrings++].stringtype = outputstring;
    outputhistory = -1;
    return false;
}

void C_TabbedOutput(const int tabs[3], const char *string, ...)
{
    va_list args;
    char    buffer[CONSOLETEXTMAXLENGTH];

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[consolestrings].string, buffer, sizeof(console[0].string));
    console[consolestrings].stringtype = outputstring;
    memcpy(console[consolestrings].tabs, tabs, sizeof(console[0].tabs));
    console[consolestrings].indent = (tabs[2] ? tabs[2] : (tabs[1] ? tabs[1] : tabs[0])) - 10;
    console[consolestrings++].wrap = 0;
    outputhistory = -1;
}

void C_Header(const int tabs[3], patch_t *header, const char *string)
{
    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    console[consolestrings].stringtype = headerstring;
    memcpy(console[consolestrings].tabs, tabs, sizeof(console[0].tabs));
    console[consolestrings].header = header;
    M_StringCopy(console[consolestrings++].string, string, sizeof(console[0].string));
    outputhistory = -1;
}

void C_Warning(const int minwarninglevel, const char *string, ...)
{
    va_list args;
    char    buffer[CONSOLETEXTMAXLENGTH];

    if (warninglevel < minwarninglevel && !devparm)
        return;

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (!consolestrings || !M_StringCompare(console[consolestrings - 1].string, buffer))
    {
        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringCopy(console[consolestrings].string, buffer, sizeof(console[0].string));
        console[consolestrings].line = 1;
        console[consolestrings].indent = WARNINGWIDTH + 2;
        console[consolestrings].wrap = 0;
        console[consolestrings++].stringtype = warningstring;
        outputhistory = -1;
    }
}

void C_PlayerMessage(const char *string, ...)
{
    va_list     args;
    char        buffer[CONSOLETEXTMAXLENGTH];
    const int   i = consolestrings - 1;

    va_start(args, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, args);
    va_end(args);

    if (console[i].stringtype == playermessagestring && M_StringCompare(console[i].string, buffer) && groupmessages)
    {
        console[i].tics = gametime;
        console[i].timestamp[0] = '\0';
        console[i].count++;
    }
    else
    {
        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringCopy(console[consolestrings].string, buffer, sizeof(console[0].string));
        console[consolestrings].stringtype = playermessagestring;
        console[consolestrings].tics = gametime;
        console[consolestrings].timestamp[0] = '\0';
        console[consolestrings].indent = 0;
        console[consolestrings].wrap = 0;
        console[consolestrings++].count = 1;
        viewplayer->prevmessage[0] = '\0';
        viewplayer->prevmessagetics = 0;
    }

    outputhistory = -1;
}

void C_ResetWrappedLines(void)
{
    for (int i = 0; i < consolestrings; i++)
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
    if (console[consolestrings - 1].stringtype != dividerstring || !consolestrings)
    {
        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        console[consolestrings++].stringtype = dividerstring;
    }
}

const kern_t altkern[] =
{
    { ' ',  ' ',  -1 }, { ' ',  'J',  -1 }, { ' ',  'T',  -1 }, { '!',  ' ',   2 }, { '"',  '+',  -1 }, { '"',  ',',  -2 },
    { '"',  '.',  -2 }, { '"',  '4',  -1 }, { '"',  'J',  -2 }, { '"',  'a',  -1 }, { '"',  'c',  -1 }, { '"',  'd',  -1 },
    { '"',  'e',  -1 }, { '"',  'g',  -1 }, { '"',  'j',  -2 }, { '"',  'o',  -1 }, { '"',  'q',  -1 }, { '"',  's',  -1 },
    { '\'',  '4', -1 }, { '\'', 's',  -1 }, { '(',  '(',  -1 }, { '(',  '-',  -1 }, { '(',  '4',  -1 }, { '(',  't',  -1 },
    { ')',  ')',  -1 }, { '+',  'j',  -2 }, { ',',  '-',  -1 }, { ',',  '4',  -1 }, { ',',  '7',  -1 }, { '-',  '3',  -1 },
    { '.',  '"',  -1 }, { '.',  '4',  -1 }, { '.',  '7',  -1 }, { '.',  '\\', -1 }, { '/',  '/',  -2 }, { '/',  '4',  -1 },
    { '/',  'a',  -1 }, { '/',  'c',  -1 }, { '/',  'd',  -1 }, { '/',  'e',  -1 }, { '/',  'g',  -1 }, { '/',  'o',  -1 },
    { '/',  'q',  -1 }, { '/',  's',  -1 }, { '0',  ',',  -1 }, { '0',  ';',  -1 }, { '0',  '4',  -1 }, { '0',  'j',  -2 },
    { '1',  ',',  -1 }, { '1',  '"',  -1 }, { '1',  '\'', -1 }, { '1',  '\\', -1 }, { '1',  '4',  -1 }, { '1',  'j',  -2 },
    { '2',  ',',  -1 }, { '2',  '4',  -1 }, { '2',  'j',  -2 }, { '3',  ',',  -1 }, { '3',  ';',  -1 }, { '3',  '4',  -1 },
    { '3',  'j',  -2 }, { '4',  '"',  -1 }, { '4',  '\'', -1 }, { '4',  '\\', -1 }, { '4',  ')',  -1 }, { '4',  '4',  -1 },
    { '4',  '7',  -1 }, { '4',  'j',  -2 }, { '5',  ',',  -1 }, { '5',  ';',  -1 }, { '5',  '4',  -1 }, { '5',  'j',  -2 },
    { '6',  ',',  -1 }, { '6',  '4',  -1 }, { '6',  'j',  -2 }, { '7',  ',',  -2 }, { '7',  '.',  -1 }, { '7',  ';',  -1 },
    { '7',  '4',  -1 }, { '7',  'j',  -2 }, { '8',  ',',  -1 }, { '8',  ';',  -1 }, { '8',  '4',  -1 }, { '8',  'j',  -2 },
    { '9',  ',',  -1 }, { '9',  ';',  -1 }, { '9',  '4',  -1 }, { '9',  'j',  -2 }, { '?',  ' ',   2 }, { 'F',  ' ',  -1 },
    { 'F',  ',',  -1 }, { 'F',  '.',  -1 }, { 'F',  ';',  -1 }, { 'F',  'J',  -1 }, { 'F',  'a',  -1 }, { 'F',  'e',  -1 },
    { 'F',  'o',  -1 }, { 'L',  ' ',  -1 }, { 'L',  '"',  -1 }, { 'L',  'Y',  -1 }, { 'L',  '\'', -1 }, { 'L',  '\\', -1 },
    { 'P',  ',',  -1 }, { 'P',  '.',  -1 }, { 'P',  ';',  -1 }, { 'P',  '_',  -1 }, { 'P',  'J',  -1 }, { 'T',  ' ',  -1 },
    { 'T',  ',',  -1 }, { 'T',  '.',  -1 }, { 'T',  ';',  -1 }, { 'T',  'a',  -1 }, { 'T',  'e',  -1 }, { 'T',  'o',  -1 },
    { 'V',  ',',  -1 }, { 'V',  '.',  -1 }, { 'V',  ';',  -1 }, { 'V',  'a',  -1 }, { 'Y',  ',',  -1 }, { 'Y',  '.',  -1 },
    { 'Y',  ';',  -1 }, { '\'', 'J',  -2 }, { '\'', 'a',  -1 }, { '\'', 'c',  -1 }, { '\'', 'd',  -1 }, { '\'', 'e',  -1 },
    { '\'', 'g',  -1 }, { '\'', 'j',  -2 }, { '\'', 'o',  -1 }, { '\'', 't',  -1 }, { '\\', 'T',  -1 }, { '\\', 'V',  -2 },
    { '\\', '\\', -2 }, { '\\', 't',  -1 }, { '\\', 'v',  -1 }, { '_',  'T',  -1 }, { '_',  'f',  -1 }, { '_',  't',  -1 },
    { '_',  'v',  -1 }, { 'a',  '"',  -1 }, { 'a',  '\'', -1 }, { 'a',  '\\', -1 }, { 'a',  'j',  -2 }, { 'b',  '"',  -1 },
    { 'b',  ',',  -1 }, { 'b',  ';',  -1 }, { 'b',  '\'', -1 }, { 'b',  '\\', -1 }, { 'b',  'j',  -2 }, { 'c',  '"',  -1 },
    { 'c',  ',',  -1 }, { 'c',  ';',  -1 }, { 'c',  '\'', -1 }, { 'c',  '\\', -1 }, { 'c',  'j',  -2 }, { 'd',  'j',  -2 },
    { 'e',  '"',  -1 }, { 'e',  '.',  -1 }, { 'e',  ',',  -1 }, { 'e',  '/',  -1 }, { 'e',  ';',  -1 }, { 'e',  '\'', -1 },
    { 'e',  '\\', -1 }, { 'e',  '_',  -1 }, { 'e',  ')',  -1 }, { 'e',  'j',  -2 }, { 'f',  ' ',  -1 }, { 'f',  '.',  -1 },
    { 'f',  ',',  -1 }, { 'f',  ';',  -1 }, { 'f',  '_',  -1 }, { 'f',  'a',  -1 }, { 'f',  'f',  -1 }, { 'f',  'j',  -2 },
    { 'f',  't',  -1 }, { 'h',  '\\', -1 }, { 'h',  'j',  -2 }, { 'i',  'j',  -2 }, { 'k',  'j',  -2 }, { 'l',  'j',  -2 },
    { 'm',  '"',  -1 }, { 'm',  '\'', -1 }, { 'm',  '\\', -1 }, { 'm',  'j',  -2 }, { 'n',  '"',  -1 }, { 'n',  '\'', -1 },
    { 'n',  '\\', -1 }, { 'n',  'j',  -2 }, { 'o',  '"',  -1 }, { 'o',  ',',  -1 }, { 'o',  ';',  -1 }, { 'o',  '\'', -1 },
    { 'o',  '\\', -1 }, { 'o',  'j',  -2 }, { 'p',  '"',  -1 }, { 'p',  ',',  -1 }, { 'p',  ';',  -1 }, { 'p',  '\'', -1 },
    { 'p',  '\\', -1 }, { 'p',  'j',  -2 }, { 'r',  ' ',  -1 }, { 'r',  '"',  -1 }, { 'r',  ')',  -1 }, { 'r',  ',',  -2 },
    { 'r',  '.',  -1 }, { 'r',  '/',  -1 }, { 'r',  ';',  -1 }, { 'r',  '\'', -1 }, { 'r',  '\\', -1 }, { 'r',  '_',  -1 },
    { 'r',  'a',  -1 }, { 'r',  'j',  -2 }, { 's',  ',',  -1 }, { 's',  ';',  -1 }, { 's',  'j',  -2 }, { 't',  'j',  -2 },
    { 't',  't',  -1 }, { 'u',  'j',  -2 }, { 'v',  ',',  -1 }, { 'v',  ';',  -1 }, { 'v',  '_',  -1 }, { 'v',  '4',  -1 },
    { 'v',  'a',  -1 }, { 'v',  'j',  -2 }, { 'w',  'j',  -2 }, { 'x',  '7',  -1 }, { 'x',  'j',  -2 }, { 'z',  'j',  -2 },
    { '\0', '\0',  0 }
};

int C_TextWidth(const char *text, const bool formatting, const bool kerning)
{
    bool            italics = false;
    const int       len = (int)strlen(text);
    unsigned char   prevletter = '\0';
    int             width = 0;

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];
        unsigned char       nextletter;

        if (letter == ' ')
            width += spacewidth;
        else if (letter == BOLDTOGGLECHAR)
            continue;
        else if (letter == ITALICSTOGGLECHAR)
        {
            italics = !italics;
            continue;
        }
        else if (letter == 149)
        {
            width += SHORT(bullet->width);
            i++;
        }
        else if (letter == 150)
        {
            width += SHORT(endash->width);
            i++;
        }
        else if (letter == 153)
        {
            width += SHORT(trademark->width);
            i++;
        }
        else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 't'
            && tolower(text[i + 2]) == 'm' && text[i + 3] == ')' && formatting)
        {
            width += SHORT(trademark->width);
            i += 3;
        }
        else if (letter == 169)
        {
            width += SHORT(copyright->width);
            i++;
        }
        else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'c' && text[i + 2] == ')' && formatting)
        {
            width += SHORT(copyright->width);
            i += 2;
        }
        else if (letter == 174)
        {
            width += SHORT(regomark->width);
            i++;
        }
        else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'r' && text[i + 2] == ')' && formatting)
        {
            width += SHORT(regomark->width);
            i += 2;
        }
        else if (letter == 176)
        {
            width += SHORT(degree->width);
            i++;
        }
        else if (letter == 215 || (letter == 'x' && isdigit(prevletter)
            && ((nextletter = (i < len - 1 ? text[i + 1] : '\0')) == '\0' || isdigit(nextletter))))
            width += SHORT(multiply->width);
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
        else if (letter == 176)
            width += degreewidth;
        else if (letter >= CONSOLEFONTSTART)
            width += SHORT(consolefont[letter - CONSOLEFONTSTART]->width);
    }

    return width;
}

static void C_DrawScrollbar(void)
{
    const int   trackend = CONSOLESCROLLBARHEIGHT * SCREENWIDTH;
    const int   facestart = CONSOLESCROLLBARHEIGHT * (outputhistory == -1 ?
                    MAX(0, consolestrings - CONSOLELINES) : outputhistory) / consolestrings;
    const int   faceend = facestart + CONSOLESCROLLBARHEIGHT - CONSOLESCROLLBARHEIGHT
                    * MAX(0, consolestrings - CONSOLELINES) / consolestrings;

    if (!facestart && trackend == faceend * SCREENWIDTH)
        scrollbardrawn = false;
    else
    {
        const int   offset = (CONSOLEHEIGHT - consoleheight) * SCREENWIDTH;
        const int   gripstart = (facestart + (faceend - facestart) / 2 - 2) * SCREENWIDTH;

        // draw scrollbar track
        for (int y = 0; y < trackend; y += SCREENWIDTH)
            if (y - offset >= 0)
                for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                    screens[0][y - offset + x] = tinttab50[screens[0][y - offset + x] + consolescrollbartrackcolor];

        // init scrollbar grip
        if (faceend - facestart > 8)
            for (int y = gripstart; y < gripstart + 6 * SCREENWIDTH; y += 2 * SCREENWIDTH)
                if (y - offset >= 0)
                    for (int x = CONSOLESCROLLBARX + 1; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH - 1; x++)
                        screens[1][y - offset + x] = screens[0][y - offset + x];

        // draw scrollbar face
        for (int y = facestart * SCREENWIDTH; y < faceend * SCREENWIDTH; y += SCREENWIDTH)
            if (y - offset >= 0)
                for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                    screens[0][y - offset + x] = consolescrollbarfacecolor;

        // draw scrollbar grip
        if (faceend - facestart > 8)
            for (int y = gripstart; y < gripstart + 6 * SCREENWIDTH; y += 2 * SCREENWIDTH)
                if (y - offset >= 0)
                    for (int x = CONSOLESCROLLBARX + 1; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH - 1; x++)
                        screens[0][y - offset + x] = screens[1][y - offset + x];

        // draw scrollbar face shadow
        if (faceend * SCREENWIDTH - offset >= 0)
            for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                screens[0][faceend * SCREENWIDTH - offset + x] = tinttab10[screens[0][faceend * SCREENWIDTH - offset + x]];

        scrollbardrawn = true;
    }
}

void C_ClearConsole(void)
{
    consolestrings = 0;
    consolestringsmax = 0;

    for (int i = 1; i <= 8; i++)
        C_Output("");
}

void C_Init(void)
{
    const char  *appdatafolder = M_GetAppDataFolder();
    char        consolefolder[MAX_PATH];

    M_snprintf(consolefolder, sizeof(consolefolder), "%s" DIR_SEPARATOR_S DOOMRETRO_CONSOLEFOLDER, appdatafolder);
    M_MakeDirectory(consolefolder);

    for (int i = 0, j = CONSOLEFONTSTART; i < CONSOLEFONTSIZE; i++)
    {
        char    buffer[9];

        M_snprintf(buffer, sizeof(buffer), "DRFON%03i", j++);
        consolefont[i] = W_CacheLastLumpName(buffer);
    }

    consoleautomapbevelcolor = &tinttab50[nearestcolors[CONSOLEAUTOMAPBEVELCOLOR] << 8];
    consolebackcolor = &tinttab50[nearestcolors[CONSOLEBACKCOLOR] << 8];
    consolebevelcolor = &tinttab50[nearestcolors[CONSOLEBEVELCOLOR] << 8];
    consoleboldcolor = nearestcolors[CONSOLEBOLDCOLOR];
    consolebolditalicscolor = nearestcolors[CONSOLEBOLDITALICSCOLOR];
    consolecaretcolor = nearestcolors[CONSOLECARETCOLOR];
    consoledividercolor = nearestcolors[CONSOLEDIVIDERCOLOR] << 8;
    consoleinputcolor = nearestcolors[CONSOLEINPUTCOLOR];
    consoleinputtooutputcolor = nearestcolors[CONSOLEINPUTTOOUTPUTCOLOR];
    consoleoutputcolor = nearestcolors[CONSOLEOUTPUTCOLOR];
    consoleoverlaycolor = nearestcolors[CONSOLEOVERLAYCOLOR];
    consoleoverlaywarningcolor = nearestcolors[CONSOLEOVERLAYWARNINGCOLOR];
    consoleplayermessagecolor = nearestcolors[CONSOLEPLAYERMESSAGECOLOR];
    consolescrollbarfacecolor = nearestcolors[CONSOLESCROLLBARFACECOLOR];
    consolescrollbartrackcolor = nearestcolors[CONSOLESCROLLBARTRACKCOLOR] << 8;
    consoleselectedinputbackgroundcolor = nearestcolors[CONSOLESELECTEDINPUTBACKGROUNDCOLOR];
    consoleselectedinputcolor = nearestcolors[CONSOLESELECTEDINPUTCOLOR];
    consoletimestampcolor = nearestcolors[CONSOLETIMESTAMPCOLOR];
    consolewarningboldcolor = nearestcolors[CONSOLEWARNINGBOLDCOLOR];
    consolewarningcolor = nearestcolors[CONSOLEWARNINGCOLOR];

    consolecolors[inputstring] = consoleinputtooutputcolor;
    consolecolors[cheatstring] = consoleinputtooutputcolor;
    consolecolors[outputstring] = consoleoutputcolor;
    consolecolors[warningstring] = consolewarningcolor;
    consolecolors[playermessagestring] = consoleplayermessagecolor;

    consoleboldcolors[inputstring] = consoleboldcolor;
    consoleboldcolors[cheatstring] = consoleboldcolor;
    consoleboldcolors[outputstring] = consoleboldcolor;
    consoleboldcolors[warningstring] = consolewarningboldcolor;
    consoleboldcolors[playermessagestring] = consoleplayermessagecolor;

    brand = W_CacheLastLumpName("DRBRAND");
    lsquote = W_CacheLastLumpName("DRFON145");
    rsquote = W_CacheLastLumpName("DRFON039");
    ldquote = W_CacheLastLumpName("DRFON147");
    rdquote = W_CacheLastLumpName("DRFON034");
    bullet = W_CacheLastLumpName("DRFON149");
    endash = W_CacheLastLumpName("DRFON150");
    trademark = W_CacheLastLumpName("DRFON153");
    copyright = W_CacheLastLumpName("DRFON169");
    regomark = W_CacheLastLumpName("DRFON174");
    degree = W_CacheLastLumpName("DRFON176");
    multiply = W_CacheLastLumpName("DRFON215");
    unknownchar = W_CacheLastLumpName("DRFON000");

    warning = W_CacheLastLumpName("DRFONWRN");
    altunderscores = W_CacheLastLumpName("DRFONUND");

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

    suckswidth = C_OverlayWidth(s_STSTR_SUCKS, false);
    timewidth = C_OverlayWidth("00:00", true);
}

void C_ShowConsole(void)
{
    consoleheight = MAX(1, consoleheight);
    consoledirection = 1;
    consoleanim = 0;
    showcaret = true;
    caretwait = 0;
    skipaction = false;

    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++)
        mousebuttons[i] = false;

    S_FadeOutSounds();
    S_LowerMusicVolume();
    SDL_StartTextInput();

    if (gamestate == GS_TITLESCREEN)
    {
        S_StartSound(NULL, sfx_swtchn);
        D_FadeScreen(false);
    }
}

void C_HideConsole(void)
{
    if (!consoleactive)
        return;

    SDL_StopTextInput();

    consoledirection = -1;
    consoleanim = 0;

    if (gamestate == GS_TITLESCREEN)
    {
        consoleheight = 0;
        consoleactive = false;
        S_StartSound(NULL, sfx_swtchx);
        D_FadeScreen(false);
    }

    S_RestoreMusicVolume();
}

void C_HideConsoleFast(void)
{
    if (!consoleactive)
        return;

    SDL_StopTextInput();

    consoledirection = -1;
    consoleanim = 0;
    consoleheight = 0;
    consoleactive = false;

    S_RestoreMusicVolume();
}

static void C_DrawBackground(void)
{
    static byte blurscreen[MAXSCREENAREA];
    const int   height = (consoleheight + 5) * SCREENWIDTH;

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
    for (int i = 0; i < height; i++)
        screens[0][i] = consolebackcolor[blurscreen[i]];

    // apply corrugated glass effect to background
    for (int y = consoleheight % 3; y <= height - 3 * SCREENWIDTH; y += SCREENWIDTH)
    {
        for (int x = y + 2; x < y + SCREENWIDTH - 1; x += 3)
        {
            byte    *dot = *screens + x;

            *dot = colormaps[0][6 * 256 + *(dot + ((x % SCREENWIDTH) ? -1 : 1))];
        }

        for (int x = (y += SCREENWIDTH) + 1; x < y + SCREENWIDTH - 1; x += 3)
        {
            byte    *dot = *screens + x;

            *dot = colormaps[0][6 * 256 + *(dot + ((x % SCREENWIDTH) ? -1 : 1))];
        }

        for (int x = (y += SCREENWIDTH); x < y + SCREENWIDTH - 1; x += 3)
        {
            byte    *dot = *screens + x;

            *dot = colormaps[0][6 * 256 + *(dot + ((x % SCREENWIDTH) ? -1 : 1))];
        }
    }

    // draw branding
    V_DrawConsoleBrandingPatch(SCREENWIDTH - brandwidth + (vid_widescreen ? 0 : 18),
        consoleheight - brandheight + 2, brand);

    // draw bottom edge
    for (int i = height - 3 * SCREENWIDTH; i < height; i++)
    {
        byte    *dot = *screens + i;

        *dot = tinttab50[consolebrandingcolor + *dot];
    }

    // bevel left and right edges
    if (automapactive && am_backcolor == am_backcolor_default)
    {
        for (int i = 0; i < height - 3 * SCREENWIDTH; i += SCREENWIDTH)
            screens[0][i] = consoleautomapbevelcolor[screens[0][i + 1]];

        for (int i = MAX(0, height - 3 * SCREENWIDTH); i < height; i += SCREENWIDTH)
            screens[0][i] = consolebevelcolor[screens[0][i + 1]];

        for (int i = 0; i < height - (brandheight + 3) * SCREENWIDTH; i += SCREENWIDTH)
            screens[0][i + SCREENWIDTH - 1] = consoleautomapbevelcolor[screens[0][i + SCREENWIDTH - 2]];

        for (int i = MAX(0, height - (brandheight + 3) * SCREENWIDTH); i < height; i += SCREENWIDTH)
            screens[0][i + SCREENWIDTH - 1] = consolebevelcolor[screens[0][i + SCREENWIDTH - 2]];
    }
    else
        for (int i = 0; i < height; i += SCREENWIDTH)
        {
            screens[0][i] = consolebevelcolor[screens[0][i + 1]];
            screens[0][i + SCREENWIDTH - 1] = consolebevelcolor[screens[0][i + SCREENWIDTH - 2]];
        }

    // bevel bottom edge
    for (int i = height - SCREENWIDTH + 1; i < height - 1; i++)
        screens[0][i] = consolebevelcolor[screens[0][i]];

    // draw shadow
    if (gamestate != GS_TITLESCREEN)
        for (int i = SCREENWIDTH; i <= 4 * SCREENWIDTH; i += SCREENWIDTH)
            for (int j = height; j < height + i; j++)
                screens[0][j] = colormaps[0][4 * 256 + screens[0][j]];
}

static int C_DrawConsoleText(int x, int y, char *text, const int color1, const int color2, const int boldcolor,
    byte *tinttab, const int tabs[3], const bool formatting, const bool kerning, const int index)
{
    bool            bold = wrapbold;
    bool            italics = wrapitalics;
    int             tab = -1;
    const int       len = (int)strlen(text);
    unsigned char   prevletter = '\0';
    unsigned char   prevletter2 = '\0';
    int             startx = x;

    y -= CONSOLEHEIGHT - consoleheight;

    if (console[index].stringtype == warningstring)
    {
        if (console[index].line == 2)
        {
            if (text[0] == ' ')
                x -= spacewidth;
        }
        else
            V_DrawConsoleTextPatch(x - 1, y, warning, WARNINGWIDTH, color1, color2, false, tinttab);

        x += (text[0] == 'T' ? WARNINGWIDTH : WARNINGWIDTH + 1);
    }

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];
        unsigned char       nextletter = text[i + 1];

        if (letter == BOLDTOGGLECHAR)
            bold = !bold;
        else if (letter == ITALICSTOGGLECHAR)
            italics = !italics;
        else
        {
            patch_t *patch = NULL;

            if (letter == ' ' && formatting)
                x += spacewidth;
            else if (letter == '\t')
            {
                if (vid_widescreen)
                    x = (x > tabs[++tab] + 18 ? x + spacewidth : tabs[tab] + 18);
                else
                    x = (x > tabs[++tab] ? x + spacewidth : tabs[tab]);

                bold = false;
                italics = false;
            }
            else if (letter == 145)
                patch = lsquote;
            else if (letter == 146)
                patch = rsquote;
            else if (letter == 147)
                patch = ldquote;
            else if (letter == 148)
                patch = rdquote;
            else if (letter == 149)
                patch = bullet;
            else if (letter == 150)
                patch = endash;
            else if (letter == 153)
                patch = trademark;
            else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 't' && tolower(text[i + 2]) == 'm' && text[i + 3] == ')'
                && formatting)
            {
                patch = trademark;
                i += 3;
            }
            else if (letter == 169)
                patch = copyright;
            else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'c' && text[i + 2] == ')' && formatting)
            {
                patch = copyright;
                i += 2;
            }
            else if (letter == 174)
                patch = regomark;
            else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'r' && text[i + 2] == ')' && formatting)
            {
                patch = regomark;
                i += 2;
            }
            else if (letter == 176)
                patch = degree;
            else if (letter == 215 || (letter == 'x' && isdigit(prevletter) && (i == len - 1 || isdigit(nextletter))))
                patch = multiply;
            else if (letter == '\n')
                break;
            else
            {
                const int   c = letter - CONSOLEFONTSTART;

                patch = (c >= 0 && c < CONSOLEFONTSIZE ? consolefont[c] : unknownchar);

                if (letter == '\'')
                {
                    if (prevletter == '\0' || prevletter == ' ' || prevletter == '\t' || prevletter == '('
                        || prevletter == '[' || prevletter == '{' || prevletter == '<' || prevletter == '"'
                        || ((prevletter == BOLDTOGGLECHAR || prevletter == ITALICSTOGGLECHAR)
                            && prevletter2 != '.' && nextletter != '.'))
                    {
                        patch = lsquote;

                        if (prevletter == '\0' && formatting)
                            x--;
                    }
                }
                else if (letter == '"')
                {
                    if (prevletter == '\0' || prevletter == ' ' || prevletter == '\t' || prevletter == '('
                        || prevletter == '[' || prevletter == '{' || prevletter == '<' || prevletter == '\''
                        || ((prevletter == BOLDTOGGLECHAR || prevletter == ITALICSTOGGLECHAR)
                            && prevletter2 != '.' && nextletter != '.'))
                    {
                        patch = ldquote;

                        if (prevletter == '\0' && formatting)
                            x--;
                    }
                }
            }

            if (kerning)
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
                    else if (letter == '\'')
                        x--;
                    else if (letter == 'v' && prevletter == '-')
                        x--;

                    if (prevletter == '/')
                        x -= 2;
                    else if (prevletter == '\'')
                        x++;

                    if (letter == 'T' && prevletter == ITALICSTOGGLECHAR && prevletter2 == ' ')
                        x--;
                }
                else if (letter == '-' && prevletter == ITALICSTOGGLECHAR)
                    x++;
                else if (letter == ',' && prevletter == BOLDTOGGLECHAR
                    && (prevletter2 == '"' || prevletter2 == '\'' || prevletter2 == 'r'))
                    x -= 2;
                else if (letter == '(' && prevletter == ' ')
                {
                    if (prevletter2 == '.')
                        x--;
                    else if (prevletter2 == '!')
                        x -= 2;
                }
                else if (prevletter == BOLDTOGGLECHAR && prevletter2 == '\t')
                {
                    if (letter == '"' || letter == '\'' || letter == '(')
                        x -= 2;
                    else if (letter == '4')
                        x--;
                }
                else if (letter == '.' && prevletter == BOLDTOGGLECHAR && prevletter2 == '\"')
                    x--;
            }

            if (patch)
            {
                const int   width = SHORT(patch->width);

                consoletextfunc(x, y, patch, width,
                    (bold && italics ? (color1 == consolewarningcolor ? color1 : consolebolditalicscolor) : (bold ? boldcolor : color1)),
                    color2, (italics && letter != '_' && letter != '-' && letter != '+' && letter != ',' && letter != '/'), tinttab);
                x += width;
            }

            if (x >= CONSOLETEXTPIXELWIDTH + CONSOLETEXTX)
                break;
        }

        prevletter2 = prevletter;
        prevletter = letter;
    }

    console[index].bold = bold;
    console[index].italics = italics;

    return (x - startx);
}

static void C_DrawOverlayText(byte *screen, int screenwidth, int x, int y,
    byte *tinttab, const char *text, const int color, const bool monospaced)
{
    const int   len = (int)strlen(text);

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];

        if (letter == ' ')
            x += spacewidth;
        else if (letter == 176)
        {
            V_DrawOverlayTextPatch(screen, screenwidth, x, y, degree, degreewidth, color, tinttab);
            x += degreewidth;
        }
        else
        {
            patch_t     *patch = consolefont[letter - CONSOLEFONTSTART];
            const int   width = SHORT(patch->width);

            if (isdigit(letter) && monospaced)
            {
                V_DrawOverlayTextPatch(screen, screenwidth,
                    x + (letter == '1' ? 1 : (letter == '4' ? -1 : 0)), y, patch, width, color, tinttab);
                x += zerowidth;
            }
            else
            {
                V_DrawOverlayTextPatch(screen, screenwidth, x, y, patch, width, color, tinttab);
                x += width;
            }
        }
    }
}

char *C_CreateTimeStamp(int index)
{
    int         hours = gamestarttime.tm_hour;
    int         minutes = gamestarttime.tm_min;
    int         seconds = gamestarttime.tm_sec;
    const int   tics = console[index].tics / TICRATE;

    if ((seconds += (tics % 3600) % 60) >= 60)
    {
        minutes += seconds / 60;
        seconds %= 60;
    }

    if ((minutes += (tics % 3600) / 60) >= 60)
    {
        hours += minutes / 60;
        minutes %= 60;
    }

    if ((hours += tics / 3600) > 12)
        hours %= 12;

    M_snprintf(console[index].timestamp, sizeof(console[0].timestamp), "%i:%02i:%02i", (!hours ? 12 : hours), minutes, seconds);
    return console[index].timestamp;
}

static void C_DrawTimeStamp(int x, int y, int index)
{
    char    buffer[9];

    M_StringCopy(buffer, (*console[index].timestamp ? console[index].timestamp : C_CreateTimeStamp(index)), sizeof(buffer));
    y -= CONSOLEHEIGHT - consoleheight;

    for (int i = (int)strlen(buffer) - 1; i >= 0; i--)
    {
        char    ch = buffer[i];
        patch_t *patch = consolefont[ch - CONSOLEFONTSTART];
        int     width = SHORT(patch->width);

        x -= (ch == ':' ? width : zerowidth);
        V_DrawConsoleTextPatch(x + (ch == '1' ? 1 : (ch == '4' ? -1 : 0)), y, patch,
            width, consoletimestampcolor, NOBACKGROUNDCOLOR, false, tinttab33);
    }
}

void C_UpdateFPSOverlay(void)
{
    char    buffer[32];
    char    *temp = commify(framespersecond);
    byte    *tinttab = (r_hud_translucency ? (automapactive ? tinttab70 : tinttab50) : NULL);

    M_snprintf(buffer, sizeof(buffer), s_STSTR_FPS, temp);

    C_DrawOverlayText(screens[0], SCREENWIDTH, SCREENWIDTH - C_OverlayWidth(buffer, true) - OVERLAYTEXTX + 1,
        OVERLAYTEXTY, tinttab, buffer, (framespersecond < (refreshrate && vid_capfps != TICRATE ? refreshrate :
        TICRATE) ? consoleoverlaywarningcolor : (((viewplayer->fixedcolormap == INVERSECOLORMAP) ^ (!r_textures))
        && !automapactive ? nearestblack : consoleoverlaycolor)), true);
    free(temp);
}

void C_UpdateTimerOverlay(void)
{
    static char buffer[10];
    static int  prevtime = -1;
    byte        *tinttab = (r_hud_translucency ? (automapactive ? tinttab70 : tinttab50) : NULL);
    int         y = OVERLAYTEXTY;

    if (vid_showfps && framespersecond)
        y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (timeremaining != prevtime)
    {
        int         seconds = (prevtime = timeremaining) / TICRATE;
        const int   hours = seconds / 3600;
        const int   minutes = ((seconds %= 3600)) / 60;

        if (!hours)
            M_snprintf(buffer, sizeof(buffer), "%02i:%02i", minutes, seconds % 60);
        else
            M_snprintf(buffer, sizeof(buffer), "%i:%02i:%02i", hours, minutes, seconds % 60);

        timerwidth = C_OverlayWidth(buffer, true);
    }

    C_DrawOverlayText(screens[0], SCREENWIDTH, SCREENWIDTH - timerwidth - OVERLAYTEXTX + 1, y, tinttab,
        buffer, (((viewplayer->fixedcolormap == INVERSECOLORMAP) ^ (!r_textures)) && !automapactive ?
        nearestblack : consoleoverlaycolor), true);
}

void C_UpdatePlayerPositionOverlay(void)
{
    const int   x = SCREENWIDTH - OVERLAYTEXTX + 1;
    int         y = OVERLAYTEXTY;
    const int   color = (((viewplayer->fixedcolormap == INVERSECOLORMAP) ^ (!r_textures)) && !automapactive ?
                    nearestblack : consoleoverlaycolor);
    byte        *tinttab = (r_hud_translucency ? (automapactive ? tinttab70 : tinttab50) : NULL);
    static char angle[32];
    static char coordinates[32];

    if (vid_showfps && framespersecond)
        y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (timeremaining)
        y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (automapactive && !am_followmode)
    {
        const mpoint_t  center = am_frame.center;
        const int       xx = center.x >> MAPBITS;
        const int       yy = center.y >> MAPBITS;

        M_snprintf(angle, sizeof(angle), "%i\xB0", direction);
        M_snprintf(coordinates, sizeof(coordinates), "(%i, %i, %i)",
            xx, yy, R_PointInSubsector(xx, yy)->sector->floorheight >> FRACBITS);
    }
    else
    {
        const int   an = (int)(viewangle * 90.0 / ANG90);
        mobj_t      *mo = viewplayer->mo;
        int         z = mo->z;

        if ((mo->flags2 & MF2_FEETARECLIPPED) && r_liquid_lowerview)
            z -= FOOTCLIPSIZE;

        M_snprintf(angle, sizeof(angle), "%i\xB0", (an == 360 ? 0 : an));
        M_snprintf(coordinates, sizeof(coordinates), "(%i, %i, %i)",
            viewx >> FRACBITS, viewy >> FRACBITS, z >> FRACBITS);
    }

    C_DrawOverlayText(screens[0], SCREENWIDTH, x - C_OverlayWidth(angle, true), y, tinttab, angle, color, true);
    C_DrawOverlayText(screens[0], SCREENWIDTH, x - C_OverlayWidth(coordinates, true),
        y + OVERLAYLINEHEIGHT, tinttab, coordinates, color, true);
}

void C_UpdatePathOverlay(void)
{
    static int  prevdistancetraveled = -1;
    static char distance[20];
    static int  width;

    if (viewplayer->distancetraveled != prevdistancetraveled)
    {
        char    *temp = distancetraveled(viewplayer->distancetraveled, false);

        M_StringCopy(distance, temp, sizeof(distance));
        free(temp);

        width = C_OverlayWidth(distance, true);
    }

    if (*distance)
    {
        int y = MAPOVERLAYTEXTY;

        if (!mapwindow)
        {
            if (vid_showfps && framespersecond)
                y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

            if (timeremaining)
                y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

            if (viewplayer->cheats & CF_MYPOS)
                y += OVERLAYLINEHEIGHT * 2 + OVERLAYSPACING;
        }

        C_DrawOverlayText(mapscreen, MAPWIDTH, MAPWIDTH - width - MAPOVERLAYTEXTX + 1, y,
            (r_hud_translucency ? tinttab70 : NULL), distance, consoleoverlaycolor, true);

        pathoverlay = true;
    }
    else
        pathoverlay = false;
}

void C_UpdatePlayerStatsOverlay(void)
{
    const int   x = MAPWIDTH - MAPOVERLAYTEXTX + 1;
    int         y = MAPOVERLAYTEXTY;
    byte        *tinttab = (r_hud_translucency ? tinttab70 : NULL);
    static char time[10];
    static int  prevmaptime = -1;
    static int  width;
    static int  color;

    if (!mapwindow)
    {
        if (vid_showfps && framespersecond)
            y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

        if (timeremaining)
            y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

        if (viewplayer->cheats & CF_MYPOS)
            y += OVERLAYLINEHEIGHT * 2 + OVERLAYSPACING;
    }

    if (pathoverlay)
        y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (maptime != prevmaptime)
    {
        int         seconds = (prevmaptime = maptime) / TICRATE;
        const int   hours = seconds / 3600;
        const int   minutes = ((seconds %= 3600)) / 60;

        if (!hours)
        {
            M_snprintf(time, sizeof(time), "%02i:%02i", minutes, seconds % 60);
            color = consoleoverlaycolor;
            width = timewidth;
        }
        else
        {
            M_StringCopy(time, s_STSTR_SUCKS, sizeof(time));
            color = consoleoverlaywarningcolor;
            width = suckswidth;
        }
    }

    C_DrawOverlayText(mapscreen, MAPWIDTH, x - width, y, tinttab, time, color, true);
    y += OVERLAYLINEHEIGHT + OVERLAYSPACING;

    if (totalkills)
    {
        char    kills[32];
        char    *temp1 = commify(viewplayer->killcount);
        char    *temp2 = commify(totalkills);

        M_snprintf(kills, sizeof(kills), s_STSTR_KILLS, temp1, temp2);
        C_DrawOverlayText(mapscreen, MAPWIDTH, x - C_OverlayWidth(kills, false), y,
            tinttab, kills, consoleoverlaycolor, false);
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
            tinttab, items, consoleoverlaycolor, false);
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
            tinttab, secrets, consoleoverlaycolor, false);
        free(temp1);
        free(temp2);
    }
}

void C_Drawer(void)
{
    int         i;
    int         x = CONSOLEINPUTX;
    int         y = CONSOLELINEHEIGHT * (CONSOLELINES - 1) - CONSOLELINEHEIGHT / 2 + 1;
    const int   bottomline = (outputhistory == -1 ? consolestrings : outputhistory + CONSOLELINES) - 1;
    int         len;
    char        partialinput[255];
    const bool  prevconsoleactive = consoleactive;
    static int  consolewait;
    const int   tics = I_GetTimeMS();
    const int   notabs[3] = { 0 };

    // adjust console height
    if (gamestate == GS_TITLESCREEN)
    {
        consoleheight = CONSOLEHEIGHT;
        consoleactive = true;
    }
    else if (consolewait < tics)
    {
        consolewait = tics + 12;

        if (consoledirection == 1)
        {
            if (consoleheight < CONSOLEHEIGHT)
            {
                const int consoledown[CONSOLEDOWNSIZE] =
                {
                     12,  29,  45,  60,  84,  97, 109, 120, 130, 139, 147, 154, 160, 165,
                    169, 173, 176, 179, 182, 184, 186, 188, 190, 192, 194, 194, 195, 195
                };

                if (consoleheight > consoledown[consoleanim])
                    consolewait = 0;
                else
                    consoleheight = consoledown[consoleanim];

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

                if (consoleheight < consoleup[consoleanim])
                    consolewait = 0;
                else
                    consoleheight = consoleup[consoleanim];

                consoleactive = (consoleanim++ < CONSOLEUPSIZE / 2);
            }
            else
                consoleactive = false;
        }
    }

    if (vid_motionblur && consoleheight < CONSOLEHEIGHT)
        I_SetMotionBlur(0);

    // cancel any controller rumble
    if (!prevconsoleactive && (joy_rumble_damage || joy_rumble_barrels || joy_rumble_weapons))
    {
        if (consoleactive)
        {
            restoredrumblestrength = idlechainsawrumblestrength;
            idlechainsawrumblestrength = 0;
        }
        else
            idlechainsawrumblestrength = restoredrumblestrength;

        I_GameControllerRumble(idlechainsawrumblestrength);
    }

    // cancel any screen shake
    I_UpdateBlitFunc(false);

    // draw background and bottom edge
    C_DrawBackground();

    // draw the scrollbar
    C_DrawScrollbar();

    consoletextfunc = &V_DrawConsoleTextPatch;

    // draw console text
    for (i = bottomline; i >= 0; i--)
    {
        const stringtype_t  stringtype = console[i].stringtype;

        if (stringtype == dividerstring)
        {
            int yy = (y + 5 - (CONSOLEHEIGHT - consoleheight)) * SCREENWIDTH;

            if (yy >= 0)
                for (int xx = yy + CONSOLETEXTX; xx < yy + CONSOLETEXTPIXELWIDTH + CONSOLETEXTX; xx++)
                {
                    byte    *dest = &screens[0][xx];

                    *dest = tinttab50[consoledividercolor + *dest];
                }

            if ((yy += SCREENWIDTH) >= 0)
            {
                byte    *tinttab = (!yy ? tinttab25 : tinttab50);

                for (int xx = yy + CONSOLETEXTX; xx < yy + CONSOLETEXTPIXELWIDTH + CONSOLETEXTX; xx++)
                {
                    byte    *dest = &screens[0][xx];

                    *dest = tinttab[consoledividercolor + *dest];
                }
            }
        }
        else if ((len = (int)strlen(console[i].string)))
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
                    int     width;

                    if (stringtype == warningstring)
                        width = indent + C_TextWidth(temp, true, true);
                    else
                        width = (indent ? indent + C_TextWidth(strrchr(temp, '\t') + 1, true, true) : C_TextWidth(temp, true, true));

                    free(temp);

                    if (width <= CONSOLETEXTPIXELWIDTH && isbreak(console[i].string[wrap]))
                        break;
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
                    C_DrawConsoleText(CONSOLETEXTX, y, buffer, consoleplayermessagecolor,
                        NOBACKGROUNDCOLOR, consoleplayermessagecolor, tinttab66, notabs, true, true, i);
                    free(temp);
                }
                else
                    C_DrawConsoleText(CONSOLETEXTX, y, text, consoleplayermessagecolor,
                        NOBACKGROUNDCOLOR, consoleplayermessagecolor, tinttab66, notabs, true, true, i);

                if (!*console[i].timestamp || !M_StringCompare(console[i].timestamp, console[i - count].timestamp))
                    C_DrawTimeStamp(SCREENWIDTH - CONSOLETEXTX - 10 - CONSOLESCROLLBARWIDTH + 1, y, i);
            }
            else if (stringtype == outputstring)
                C_DrawConsoleText(CONSOLETEXTX, y, text, consoleoutputcolor,
                    NOBACKGROUNDCOLOR, consoleboldcolor, tinttab66, console[i].tabs, true, true, i);
            else if (stringtype == inputstring || stringtype == cheatstring)
                C_DrawConsoleText(CONSOLETEXTX, y, text, consoleinputcolor,
                    NOBACKGROUNDCOLOR, consoleboldcolor, tinttab66, notabs, true, true, i);
            else if (stringtype == warningstring)
                C_DrawConsoleText(CONSOLETEXTX, y, console[i].string, consolewarningcolor,
                    NOBACKGROUNDCOLOR, consolewarningboldcolor, tinttab66, notabs, true, true, i);
            else
                V_DrawConsolePatch(CONSOLETEXTX - 1, y + 4 - (CONSOLEHEIGHT - consoleheight),
                    console[i].header, CONSOLETEXTPIXELWIDTH + 2);

            if (wrap < len && i < bottomline)
            {
                char    *temp = M_SubString(console[i].string, wrap, (size_t)len - wrap);

                wrapbold = console[i].bold;
                wrapitalics = console[i].italics;
                C_DrawConsoleText(CONSOLETEXTX + console[i].indent, y + CONSOLELINEHEIGHT, trimwhitespace(temp),
                    consolecolors[stringtype], NOBACKGROUNDCOLOR, consoleboldcolors[stringtype], tinttab66, notabs, true, true, 0);
                wrapbold = false;
                wrapitalics = false;
                free(temp);
            }

            free(text);
        }

        if ((y -= CONSOLELINEHEIGHT) < -CONSOLELINEHEIGHT)
            break;
    }

    if (quitcmd)
        return;

    if (consoleinput[0] != '\0')
    {
        // draw input text to left of caret
        for (i = 0; i < MIN(selectstart, caretpos); i++)
            partialinput[i] = consoleinput[i];

        partialinput[i] = '\0';

        if (partialinput[0] != '\0')
        {
            consoletextfunc = &V_DrawConsoleTextPatch;
            x += C_DrawConsoleText(x, CONSOLEINPUTY, partialinput, consoleinputcolor,
                NOBACKGROUNDCOLOR, NOBOLDCOLOR, NULL, notabs, false, true, 0);
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
                    int yy = CONSOLEINPUTY + i - (CONSOLEHEIGHT - consoleheight);

                    if (yy >= 0)
                        screens[0][yy * SCREENWIDTH + x - 1] = consoleselectedinputbackgroundcolor;
                }

                consoletextfunc = &V_DrawConsoleSelectedTextPatch;
                x += C_DrawConsoleText(x, CONSOLEINPUTY, partialinput, consoleselectedinputcolor,
                         consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false, true, 0);

                for (i = 1; i < CONSOLELINEHEIGHT - 1; i++)
                {
                    int yy = CONSOLEINPUTY + i - (CONSOLEHEIGHT - consoleheight);

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

            for (int yy = 0; yy < 14 * SCREENWIDTH; yy += SCREENWIDTH)
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

    x += 3;

    // draw any selected text to right of caret
    if (selectend > caretpos)
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

            consoletextfunc = &V_DrawConsoleSelectedTextPatch;
            x += C_DrawConsoleText(x, CONSOLEINPUTY, partialinput, consoleselectedinputcolor,
                consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false, true, 0);

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
        for (i = selectend; i < len; i++)
            partialinput[i - selectend] = consoleinput[i];

        partialinput[i - selectend] = '\0';

        if (partialinput[0] != '\0')
        {
            consoletextfunc = &V_DrawConsoleTextPatch;
            C_DrawConsoleText(x, CONSOLEINPUTY, partialinput, consoleinputcolor,
                NOBACKGROUNDCOLOR, NOBOLDCOLOR, NULL, notabs, false, true, 0);
        }
    }

    I_Sleep(1);
}

bool C_ExecuteInputString(const char *input)
{
    char    *string = M_StringDuplicate(input);
    char    *strings[255];
    int     j = 0;

    M_StripQuotes(string);
    strings[0] = strtok(string, ";");

    while (strings[j])
    {
        if (!C_ValidateInput(trimwhitespace(strings[j])))
            break;

        strings[++j] = strtok(NULL, ";");
    }

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

                    if ((M_StringCompare(cmd, consolecmds[i].name) || M_StringCompare(cmd, consolecmds[i].alternate))
                        && length == strlen(cmd) + 2
                        && consolecmds[i].func1(consolecmds[i].name, consolecheatparm))
                    {
                        if (gamestate == GS_LEVEL)
                            M_StringCopy(consolecheat, cmd, sizeof(consolecheat));

                        return true;
                    }
                }
            }
            else if ((M_StringCompare(input, consolecmds[i].name) || M_StringCompare(input, consolecmds[i].alternate))
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

                if ((M_StringCompare(cmd, consolecmds[i].name) || M_StringCompare(cmd, consolecmds[i].alternate))
                    && consolecmds[i].func1(consolecmds[i].name, temp)
                    && (consolecmds[i].parameters || !*temp))
                {
                    if (!executingalias && !resettingcvar)
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

    if (gamestate == GS_LEVEL)
        for (int i = 0; *actions[i].action; i++)
            if (M_StringCompare(input, actions[i].action))
            {
                C_Input("%s", input);

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
        I_Quit(true);

    if ((consoleheight < CONSOLEHEIGHT && consoledirection == -1) || messagetoprint)
        return false;

    len = (int)strlen(consoleinput);

    if (ev->type == ev_keydown)
    {
        static char         currentinput[255];
        const int           key = ev->data1;
        const SDL_Keymod    modstate = SDL_GetModState();

        if (key == keyboardconsole && !keydown)
        {
            keydown = key;
            C_HideConsole();
            return true;
        }
        else if (key == keyboardscreenshot && keyboardscreenshot == KEY_PRINTSCREEN)
        {
            G_ScreenShot();
            return true;
        }

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
                // confirm input
                if (consoleinput[0] != '\0')
                {
                    bool    result = false;

                    if (M_StringStartsWith(consoleinput, "bind ") || M_StringStartsWith(consoleinput, "unbind "))
                    {
                        if (C_ValidateInput(consoleinput))
                            result = true;
                    }
                    else
                    {
                        char    *string = M_StringDuplicate(consoleinput);
                        char    *strings[255];

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
                        caretpos++;
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;

                        if (selectend >= caretpos)
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
                // move caret to start
                if ((outputhistory != -1 || !caretpos) && outputhistory && consolestrings > CONSOLELINES)
                    outputhistory = 0;
                else if (caretpos > 0)
                {
                    selectend = ((modstate & KMOD_SHIFT) ? caretpos : 0);
                    caretpos = selectstart = 0;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            case KEY_END:
                // move caret to end
                if (outputhistory != -1 && consolestrings > CONSOLELINES)
                    outputhistory = -1;
                else if (caretpos < len)
                {
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
                        if (consoleinput[i] == ';')
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
                        int         spaces2;
                        bool        endspace2;
                        int         len2;
                        int         game;

                        autocomplete += scrolldirection;

                        if (GetCapsLockState())
                        {
                            char    *temp = uppercase(autocompletelist[autocomplete].text);

                            M_StringCopy(output, temp, sizeof(output));
                            free(temp);
                        }
                        else
                            M_StringCopy(output, autocompletelist[autocomplete].text, sizeof(output));

                        if (M_StringCompare(output, input))
                            continue;

                        len2 = (int)strlen(output);
                        spaces2 = numspaces(output);
                        endspace2 = (len2 > 0 && output[len2 - 1] == ' ');
                        game = autocompletelist[autocomplete].game;

                        if ((game == DOOM1AND2
                            || (gamemission == doom && game == DOOM1ONLY)
                            || (gamemission != doom && game == DOOM2ONLY))
                            && M_StringStartsWith(output, input)
                            && input[strlen(input) - 1] != '+'
                            && ((!spaces1 && (!spaces2 || (spaces2 == 1 && endspace2)))
                                || (spaces1 == 1 && !endspace1 && (spaces2 == 1 || (spaces2 == 2 && endspace2)))
                                || (spaces1 == 2 && !endspace1 && (spaces2 == 2 || (spaces2 == 3 && endspace2)))
                                || (spaces1 == 3 && !endspace1)))
                        {
                            char    *temp = M_StringJoin(prefix, M_StringReplace(output, input, input), NULL);

                            M_StringCopy(consoleinput, temp, sizeof(consoleinput));
                            caretpos = selectstart = selectend = len2 + (int)strlen(prefix);
                            caretwait = I_GetTimeMS() + CARETBLINKTIME;
                            showcaret = true;
                            free(temp);

                            return true;
                        }
                    }

                    autocomplete = start;
                }

                break;

            case KEY_UPARROW:
                // scroll output up
                if (modstate & KMOD_CTRL)
                {
                    scrollspeed = MIN(scrollspeed + 4, TICRATE * 8);

                    if (consolestrings > CONSOLELINES)
                        outputhistory = (outputhistory == -1 ? consolestrings - (CONSOLELINES + 1) :
                            MAX(0, outputhistory - scrollspeed / TICRATE));
                }

                // previous input
                else
                {
                    if (inputhistory == -1)
                        M_StringCopy(currentinput, consoleinput, sizeof(currentinput));

                    for (i = (inputhistory == -1 ? consolestrings : inputhistory) - 1; i >= 0; i--)
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
                // scroll output down
                if (modstate & KMOD_CTRL)
                {
                    scrollspeed = MIN(scrollspeed + 4, TICRATE * 8);

                    if (outputhistory != -1 && (outputhistory += scrollspeed / TICRATE) + CONSOLELINES >= consolestrings)
                        outputhistory = -1;
                }

                // next input
                else
                {
                    if (inputhistory != -1)
                    {
                        for (i = inputhistory + 1; i < consolestrings; i++)
                            if (console[i].stringtype == inputstring
                                && !M_StringCompare(consoleinput, console[i].string)
                                && C_TextWidth(console[i].string, false, true) <= CONSOLEINPUTPIXELWIDTH)
                            {
                                inputhistory = i;
                                M_StringCopy(consoleinput, console[i].string, sizeof(consoleinput));
                                break;
                            }

                        if (i == consolestrings)
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
                scrollspeed = MIN(scrollspeed + 4, TICRATE * 8);

                if (consolestrings > CONSOLELINES)
                    outputhistory = (outputhistory == -1 ? consolestrings - (CONSOLELINES + 1) :
                        MAX(0, outputhistory - scrollspeed / TICRATE));

                break;

            case KEY_PAGEDOWN:
                // scroll output down
                scrollspeed = MIN(scrollspeed + 4, TICRATE * 8);

                if (outputhistory != -1 && (outputhistory += scrollspeed / TICRATE) + CONSOLELINES >= consolestrings)
                    outputhistory = -1;

                break;

            case KEY_ESCAPE:
                // close console
                C_HideConsole();
                break;

            case KEY_F1:
            case KEY_F2:
            case KEY_F3:
            case KEY_F4:
            case KEY_F5:
            case KEY_F6:
            case KEY_F7:
            case KEY_F8:
            case KEY_F9:
            case KEY_F10:
            case KEY_F11:
                if (M_Responder(ev))
                    C_HideConsoleFast();

                break;

            case KEY_CAPSLOCK:
                // toggle "always run"
                if (keyboardalwaysrun == KEY_CAPSLOCK)
                    G_ToggleAlwaysRun(ev_keydown);

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
                    M_StringCopy(buffer, M_StringReplace(buffer, "(null)", ""), sizeof(buffer));
                    M_StringCopy(buffer, M_StringReplace(buffer, "(null)", ""), sizeof(buffer));

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
        const char  ch = (char)ev->data1;
        char        *temp = NULL;

        if (ch >= ' ' && ch <= '}' && ch != '`' && C_TextWidth(consoleinput, false, true)
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
    else if (ev->type == ev_mousewheel)
    {
        // scroll output up
        if (ev->data1 > 0)
        {
            if (consolestrings > CONSOLELINES)
                outputhistory = (outputhistory == -1 ? consolestrings - (CONSOLELINES + 1) : MAX(0, outputhistory - 1));
        }

        // scroll output down
        else if (ev->data1 < 0)
        {
            if (outputhistory != -1 && ++outputhistory + CONSOLELINES == consolestrings)
                outputhistory = -1;
        }
    }
    else if (ev->type == ev_controller && (gamecontrollerbuttons & gamecontrollerconsole) && gamecontrollerwait < I_GetTime())
    {
        gamecontrollerwait = I_GetTime() + 8;
        C_HideConsole();
    }

    return true;
}

static const char *dayofweek(int day, int month, int year)
{
    const int   adjustment = (14 - month) / 12;
    const char  *days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    month += 12 * adjustment - 2;
    year -= adjustment;

    return days[(day + (13 * month - 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7];
}

void C_PrintCompileDate(void)
{
    char    mth[4] = "";
    int     minute, hour, day, year;

    if (sscanf(__DATE__, "%3s %2i %4i", mth, &day, &year) == 3 && sscanf(__TIME__, "%2i:%2i:%*i", &hour, &minute) == 2)
    {
        const char  mths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
        const int   month = (int)(strstr(mths, mth) - mths) / 3;

        const char *months[] =
        {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };

        C_Output("This %i-bit " ITALICS("%s") " app of " ITALICS("%s") " was built with love by %s in %s at %i:%02i%s on %s, %s %i, %i.",
            8 * (int)sizeof(intptr_t), WINDOWS, DOOMRETRO_NAMEANDVERSIONSTRING, DOOMRETRO_CREATOR, DOOMRETRO_PLACEOFORIGIN,
            (hour ? hour - 12 * (hour > 12) : 12), minute, (hour < 12 ? "am" : "pm"),
            dayofweek(day, month + 1, year), months[month], day, year);
    }

#if defined(__clang__)
    C_Output("It was compiled using " ITALICS("Clang v%i.%i.%i."), __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(_MSC_FULL_VER)
    if (_MSC_BUILD)
        C_Output("It was compiled using v%i.%02i.%i.%i of the " ITALICS("Microsoft C/C++ Optimizing Compiler."),
            _MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000, _MSC_BUILD);
    else
        C_Output("It was compiled using v%i.%02i.%i of the " ITALICS("Microsoft C/C++ Optimizing Compiler."),
            _MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000);
#endif
}

void C_PrintSDLVersions(void)
{
    C_Output("Using v%i.%i.%i of the " ITALICS("SDL (Simple DirectMedia Layer)") " library.",
        SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

    C_Output("Also using v%i.%i.%i of the " ITALICS("SDL_mixer") " library and v%i.%i.%i of the " ITALICS("SDL_image") " library.",
        SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL,
        SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);
}
