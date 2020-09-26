/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

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
#endif

#include "SDL_image.h"
#include "SDL_mixer.h"

#include "c_cmds.h"
#include "c_console.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_colors.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_spec.h"
#include "r_main.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"

console_t               *console;

dboolean                consoleactive;
int                     consoleheight = 0;
int                     consoledirection = -1;
static int              consoleanim;

dboolean                forceconsoleblurredraw;

patch_t                 *consolefont[CONSOLEFONTSIZE];
patch_t                 *degree;
patch_t                 *unknownchar;
patch_t                 *altunderscores;
patch_t                 *brand;
patch_t                 *lsquote;
patch_t                 *ldquote;

static patch_t          *dot;
static patch_t          *trademark;
static patch_t          *copyright;
static patch_t          *regomark;
static patch_t          *multiply;
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

static char             consoleinput[255];
static int              numautocomplete;
int                     consolestrings = 0;
size_t                  consolestringsmax = 0;

static size_t           undolevels;
static undohistory_t    *undohistory;

static int              caretpos;
static dboolean         showcaret = true;
static int              caretwait;
static int              selectstart;
static int              selectend;

char                    consolecheat[255];
char                    consolecheatparm[3];

static int              outputhistory = -1;

int                     con_backcolor = con_backcolor_default;
int                     con_edgecolor = con_edgecolor_default;
int                     warninglevel = warninglevel_default;

static int              timerx;
static int              zerowidth;
static int              warningwidth;
static int              dotwidth;

static int              consolecaretcolor = 4;
static int              consolelowfpscolor = 180;
static int              consolehighfpscolor = 4;
static int              consoletimercolor = 4;
static int              consoleinputcolor = 4;
static int              consoleselectedinputcolor = 4;
static int              consoleselectedinputbackgroundcolor = 100;
static int              consoleinputtooutputcolor = 4;
static int              consoleplayermessagecolor = 161;
static int              consoletimestampcolor = 161;
static int              consoleoutputcolor = 88;
static int              consoleboldcolor = 4;
static int              consoleitalicscolor = 98;
static int              consolewarningcolor = 180;
static int              consolewarningboldcolor = 176;
static int              consoledividercolor = 100;
static int              consolescrollbartrackcolor = 100;
static int              consolescrollbarfacecolor = 94;
static int              consolescrollbargripcolor = 104;

static byte             *consolebevel;
static byte             *consoleautomapbevel;

static int              consolecolors[STRINGTYPES];

dboolean                scrollbardrawn;

static void (*consoletextfunc)(int, int, patch_t *, int, int, int, dboolean, byte *);

extern int              framespersecond;
extern int              refreshrate;
extern dboolean         quitcmd;

void C_Input(const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH];

    if (togglingvanilla)
        return;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
    C_DumpConsoleStringToFile(consolestrings);
    console[consolestrings].truncate = 0;
    console[consolestrings++].stringtype = inputstring;
    outputhistory = -1;
    consoleinput[0] = '\0';
    caretpos = 0;
    selectstart = 0;
    selectend = 0;
}

void C_InputNoRepeat(const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH];

    if (togglingvanilla)
        return;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (!consolestrings || !M_StringStartsWith(console[consolestrings - 1].string, buffer))
    {
        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
        C_DumpConsoleStringToFile(consolestrings);
        console[consolestrings].truncate = 0;
        console[consolestrings++].stringtype = inputstring;
        outputhistory = -1;
        consoleinput[0] = '\0';
        caretpos = 0;
        selectstart = 0;
        selectend = 0;
    }
}

void C_IntCVAROutput(char *cvar, int value)
{
    char    *temp = M_StringJoin(cvar, " ", NULL);

    if (consolestrings && M_StringStartsWithExact(console[consolestrings - 1].string, temp))
        consolestrings--;

    C_Input("%s %i", cvar, value);
    free(temp);
}

void C_PctCVAROutput(char *cvar, int value)
{
    char    *temp = M_StringJoin(cvar, " ", NULL);

    if (consolestrings && M_StringStartsWithExact(console[consolestrings - 1].string, temp))
        consolestrings--;

    C_Input("%s %i%%", cvar, value);
    free(temp);
}

void C_StrCVAROutput(char *cvar, char *string)
{
    char    *temp = M_StringJoin(cvar, " ", NULL);

    if (consolestrings && M_StringStartsWithExact(console[consolestrings - 1].string, temp))
        consolestrings--;

    C_Input("%s %s", cvar, string);
    free(temp);
}

void C_Output(const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH];

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
    C_DumpConsoleStringToFile(consolestrings);
    console[consolestrings].truncate = 0;
    console[consolestrings++].stringtype = outputstring;
    outputhistory = -1;
}

void C_OutputWrap(const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH];

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (!consolestrings || !M_StringCompare(console[consolestrings - 1].string, buffer))
    {
        int len = (int)strlen(buffer);

        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        if (len <= 100)
        {
            M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
            console[consolestrings].line = 1;
            console[consolestrings].truncate = 0;
            C_DumpConsoleStringToFile(consolestrings);
            console[consolestrings++].stringtype = outputstring;
        }
        else
        {
            int     truncate = len;
            char    *temp;

            do
            {
                int width;

                temp = M_SubString(buffer, 0, truncate);
                width = C_TextWidth(temp, true, true);
                free(temp);

                if (width <= CONSOLETEXTPIXELWIDTH)
                    break;
            } while (truncate-- > 0);

            while (truncate > 0 && !isbreak(buffer[truncate]))
                truncate--;

            temp = M_SubString(buffer, 0, truncate);
            M_StringCopy(console[consolestrings].string, temp, sizeof(console[consolestrings].string));
            free(temp);
            console[consolestrings].line = 1;
            C_DumpConsoleStringToFile(consolestrings);
            console[consolestrings].truncate = 0;
            console[consolestrings++].stringtype = outputstring;

            if (consolestrings >= (int)consolestringsmax)
                console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

            temp = M_SubString(buffer, truncate, (size_t)len - truncate);

            if (*temp)
            {
                M_StringCopy(console[consolestrings].string, trimwhitespace(temp), sizeof(console[consolestrings].string));
                free(temp);
                console[consolestrings].line = 2;
                C_DumpConsoleStringToFile(consolestrings);
                console[consolestrings].truncate = 0;
                console[consolestrings++].stringtype = outputstring;
            }
        }

        outputhistory = -1;
    }
}

void C_OutputNoRepeat(const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH];

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (!consolestrings || !M_StringStartsWith(console[consolestrings - 1].string, buffer))
    {
        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
        C_DumpConsoleStringToFile(consolestrings);
        console[consolestrings].truncate = 0;
        console[consolestrings++].stringtype = outputstring;
        outputhistory = -1;
    }
}

void C_TabbedOutput(const int tabs[4], const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH];

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
    console[consolestrings].truncate = 0;
    console[consolestrings].stringtype = outputstring;
    memcpy(console[consolestrings].tabs, tabs, sizeof(console[consolestrings].tabs));
    C_DumpConsoleStringToFile(consolestrings);
    consolestrings++;
    outputhistory = -1;
}

void C_Header(const int tabs[4], patch_t *header, const char *string)
{
    if (consolestrings >= (int)consolestringsmax)
        console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

    console[consolestrings].truncate = 0;
    console[consolestrings].stringtype = headerstring;
    memcpy(console[consolestrings].tabs, tabs, sizeof(console[consolestrings].tabs));
    console[consolestrings].header = header;
    M_StringCopy(console[consolestrings].string, string, sizeof(console[consolestrings].string));
    C_DumpConsoleStringToFile(consolestrings);
    consolestrings++;
    outputhistory = -1;
}

void C_Warning(const int minwarninglevel, const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH];

    if (warninglevel < minwarninglevel)
        return;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (!consolestrings || !M_StringCompare(console[consolestrings - 1].string, buffer))
    {
        int len = (int)strlen(buffer);

        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        if (len <= 100 || !warningwidth)
        {
            M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
            console[consolestrings].line = 1;
            C_DumpConsoleStringToFile(consolestrings);
            console[consolestrings].truncate = 0;
            console[consolestrings++].stringtype = warningstring;
        }
        else
        {
            int     truncate = len;
            char    *temp;

            do
            {
                int width;

                temp = M_SubString(buffer, 0, truncate);
                width = C_TextWidth(temp, true, true) + warningwidth + 12;
                free(temp);

                if (width <= CONSOLETEXTPIXELWIDTH - 8)
                    break;
            } while (truncate-- > 0);

            while (truncate > 0 && !isbreak(buffer[truncate]))
                truncate--;

            temp = M_SubString(buffer, 0, truncate);
            M_StringCopy(console[consolestrings].string, temp, sizeof(console[consolestrings].string));
            free(temp);
            console[consolestrings].line = 1;
            C_DumpConsoleStringToFile(consolestrings);
            console[consolestrings].truncate = 0;
            console[consolestrings++].stringtype = warningstring;

            if (consolestrings >= (int)consolestringsmax)
                console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

            temp = M_SubString(buffer, truncate, (size_t)len - truncate);

            if (*temp)
            {
                M_StringCopy(console[consolestrings].string, trimwhitespace(temp), sizeof(console[consolestrings].string));
                free(temp);
                console[consolestrings].line = 2;
                C_DumpConsoleStringToFile(consolestrings);
                console[consolestrings].truncate = 0;
                console[consolestrings++].stringtype = warningstring;
            }
        }

        outputhistory = -1;
    }
}

void C_PlayerMessage(const char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH];
    const int   i = consolestrings - 1;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (i >= 0 && console[i].stringtype == playermessagestring && M_StringCompare(console[i].string, buffer))
    {
        console[i].tics = gametime;
        console[i].timestamp[0] = '\0';
        console[i].count++;
    }
    else
    {
        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
        console[consolestrings].truncate = 0;
        console[consolestrings].stringtype = playermessagestring;
        console[consolestrings].tics = gametime;
        console[consolestrings].timestamp[0] = '\0';
        C_DumpConsoleStringToFile(consolestrings);
        console[consolestrings++].count = 1;
    }

    outputhistory = -1;
}

void C_Obituary(const char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH];
    const int   i = consolestrings - 1;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (i >= 0 && console[i].stringtype == obituarystring && M_StringCompare(console[i].string, buffer))
    {
        console[i].tics = gametime;
        console[i].timestamp[0] = '\0';
        console[i].count++;
    }
    else
    {
        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        M_StringCopy(console[consolestrings].string, buffer, sizeof(console[consolestrings].string));
        console[consolestrings].truncate = 0;
        console[consolestrings].stringtype = obituarystring;
        console[consolestrings].tics = gametime;
        console[consolestrings].timestamp[0] = '\0';
        C_DumpConsoleStringToFile(consolestrings);
        console[consolestrings++].count = 1;
    }

    outputhistory = -1;
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
    if (!consolestrings || console[consolestrings - 1].stringtype != dividerstring)
    {
        if (consolestrings >= (int)consolestringsmax)
            console = I_Realloc(console, (consolestringsmax += CONSOLESTRINGSMAX) * sizeof(*console));

        C_DumpConsoleStringToFile(consolestrings);
        console[consolestrings].truncate = 0;
        console[consolestrings++].stringtype = dividerstring;
    }
}

const kern_t altkern[] =
{
    { ' ',  'J',  -1 }, { ' ',  'T',  -1 }, { '!',  ' ',   2 }, { '"',  '+',  -1 }, { '"',  ',',  -2 }, { '"',  '.',  -2 },
    { '"',  '4',  -1 }, { '"',  'J',  -2 }, { '"',  'a',  -1 }, { '"',  'c',  -1 }, { '"',  'd',  -1 }, { '"',  'e',  -1 },
    { '"',  'g',  -1 }, { '"',  'j',  -2 }, { '"',  'o',  -1 }, { '"',  'q',  -1 }, { '"',  's',  -1 }, { '\'',  '4', -1 },
    { '\'', 's',  -2 }, { '(',  '(',  -1 }, { '(',  '-',  -1 }, { '(',  't',  -1 }, { ')',  ')',  -1 }, { '+',  'j',  -2 },
    { ',',  '-',  -1 }, { ',',  '4',  -1 }, { ',',  '7',  -1 }, { '-',  '3',  -1 }, { '.',  '"',  -1 }, { '.',  '4',  -1 },
    { '.',  '7',  -1 }, { '.',  '\\', -1 }, { '/',  '/',  -2 }, { '/',  'a',  -2 }, { '/',  'd',  -1 }, { '/',  'o',  -1 },
    { '0',  ',',  -1 }, { '0',  ';',  -1 }, { '0',  'j',  -2 }, { '1',  ',',  -1 }, { '1',  '"',  -1 }, { '1',  '\'', -1 },
    { '1',  '\\', -1 }, { '1',  'j',  -2 }, { '2',  ',',  -1 }, { '2',  'j',  -2 }, { '3',  ',',  -1 }, { '3',  ';',  -1 },
    { '3',  'j',  -2 }, { '4',  '.',  -1 }, { '4',  ',',  -1 }, { '4',  '"',  -1 }, { '4',  '\'', -1 }, { '4',  '\\', -2 },
    { '4',  ')',  -1 }, { '4',  '4',  -1 }, { '4',  '7',  -1 }, { '4',  'j',  -2 }, { '5',  ',',  -1 }, { '5',  ';',  -1 },
    { '5',  'j',  -2 }, { '6',  ',',  -1 }, { '6',  'j',  -2 }, { '7',  ',',  -3 }, { '7',  '.',  -2 }, { '7',  ';',  -1 },
    { '7',  '4',  -1 }, { '7',  'j',  -2 }, { '8',  ',',  -1 }, { '8',  ';',  -1 }, { '8',  'j',  -2 }, { '9',  ',',  -1 },
    { '9',  ';',  -1 }, { '9',  'j',  -2 }, { '?',  ' ',   2 }, { 'F',  ' ',  -1 }, { 'F',  ',',  -1 }, { 'F',  '.',  -1 },
    { 'F',  ';',  -1 }, { 'F',  'J',  -1 }, { 'F',  'a',  -1 }, { 'F',  'e',  -1 }, { 'F',  'o',  -1 }, { 'L',  ' ',  -1 },
    { 'L',  '"',  -1 }, { 'L',  'Y',  -1 }, { 'L',  '\'', -1 }, { 'L',  '\\', -1 }, { 'P',  ',',  -1 }, { 'P',  '.',  -1 },
    { 'P',  ';',  -1 }, { 'P',  '_',  -1 }, { 'P',  'J',  -1 }, { 'T',  ' ',  -1 }, { 'T',  ',',  -1 }, { 'T',  '.',  -1 },
    { 'T',  ';',  -1 }, { 'T',  'a',  -1 }, { 'T',  'e',  -1 }, { 'T',  'o',  -1 }, { 'V',  ',',  -1 }, { 'V',  '.',  -1 },
    { 'V',  ';',  -1 }, { 'V',  'a',  -1 }, { 'Y',  ',',  -1 }, { 'Y',  '.',  -1 }, { 'Y',  ';',  -1 }, { '\'', 'J',  -2 },
    { '\'', 'a',  -1 }, { '\'', 'c',  -1 }, { '\'', 'd',  -1 }, { '\'', 'e',  -1 }, { '\'', 'g',  -1 }, { '\'', 'j',  -2 },
    { '\'', 'o',  -1 }, { '\'', 't',  -1 }, { '\\', 'T',  -1 }, { '\\', 'V',  -2 }, { '\\', '\\', -2 }, { '\\', 't',  -1 },
    { '\\', 'v',  -1 }, { '_',  'f',  -1 }, { '_',  't',  -1 }, { '_',  'v',  -2 }, { 'a',  '"',  -1 }, { 'a',  '\'', -1 },
    { 'a',  '\\', -1 }, { 'a',  'j',  -2 }, { 'b',  '"',  -1 }, { 'b',  ',',  -1 }, { 'b',  ';',  -1 }, { 'b',  '\'', -1 },
    { 'b',  '\\', -1 }, { 'b',  'j',  -2 }, { 'c',  '"',  -1 }, { 'c',  ',',  -1 }, { 'c',  ';',  -1 }, { 'c',  '\'', -1 },
    { 'c',  '\\', -1 }, { 'c',  'j',  -2 }, { 'd',  'j',  -2 }, { 'e',  '"',  -1 }, { 'e',  '.',  -1 }, { 'e',  ',',  -1 },
    { 'e',  '/',  -1 }, { 'e',  ';',  -1 }, { 'e',  '\'', -1 }, { 'e',  '\\', -1 }, { 'e',  '_',  -1 }, { 'e',  ')',  -1 },
    { 'e',  'j',  -2 }, { 'f',  ' ',  -1 }, { 'f',  ',',  -2 }, { 'f',  ';',  -1 }, { 'f',  '_',  -1 }, { 'f',  'a',  -1 },
    { 'f',  'f',  -1 }, { 'f',  'j',  -2 }, { 'f',  't',  -1 }, { 'h',  '\\', -1 }, { 'h',  'j',  -2 }, { 'i',  'j',  -2 },
    { 'k',  'j',  -2 }, { 'l',  'j',  -2 }, { 'm',  '"',  -1 }, { 'm',  '\'', -1 }, { 'm',  '\\', -1 }, { 'm',  'j',  -2 },
    { 'n',  '"',  -1 }, { 'n',  '\'', -1 }, { 'n',  '\\', -1 }, { 'n',  'j',  -2 }, { 'o',  '"',  -1 }, { 'o',  ',',  -1 },
    { 'o',  ';',  -1 }, { 'o',  '\'', -1 }, { 'o',  '\\', -1 }, { 'o',  'j',  -2 }, { 'p',  '"',  -1 }, { 'p',  ',',  -1 },
    { 'p',  ';',  -1 }, { 'p',  '\'', -1 }, { 'p',  '\\', -1 }, { 'p',  'j',  -2 }, { 'r',  ' ',  -1 }, { 'r',  '"',  -1 },
    { 'r',  ')',  -1 }, { 'r',  ',',  -2 }, { 'r',  '.',  -1 }, { 'r',  '/',  -1 }, { 'r',  ';',  -1 }, { 'r',  '\'', -1 },
    { 'r',  '\\', -1 }, { 'r',  '_',  -2 }, { 'r',  'a',  -1 }, { 'r',  'j',  -2 }, { 's',  ',',  -1 }, { 's',  ';',  -1 },
    { 's',  'j',  -2 }, { 't',  'j',  -2 }, { 't',  't',  -1 }, { 'u',  'j',  -2 }, { 'v',  ',',  -1 }, { 'v',  ';',  -1 },
    { 'v',  'a',  -1 }, { 'v',  'j',  -2 }, { 'w',  'j',  -2 }, { 'x',  'j',  -2 }, { 'z',  'j',  -2 }, { '\0', '\0',  0 }
};

int C_TextWidth(const char *text, const dboolean formatting, const dboolean kerning)
{
    int             bold = 0;
    dboolean        italics = false;
    const int       len = (int)strlen(text);
    unsigned char   prevletter = '\0';
    int             w = 0;

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];
        unsigned char       nextletter;

        if (letter == ' ')
            w += spacewidth;
        else if (letter == '<' && i < len - 2 && tolower(text[i + 1]) == 'b' && text[i + 2] == '>' && formatting)
        {
            bold = (italics ? 2 : 1);
            i += 2;
        }
        else if (letter == '<' && i < len - 3 && text[i + 1] == '/' && tolower(text[i + 2]) == 'b' && text[i + 3] == '>' && formatting)
        {
            bold = 0;
            i += 3;
        }
        else if (letter == '<' && i < len - 2 && tolower(text[i + 1]) == 'i' && text[i + 2] == '>' && formatting)
        {
            italics = true;
            i += 2;
        }
        else if (letter == '<' && i < len - 3 && text[i + 1] == '/' && tolower(text[i + 2]) == 'i' && text[i + 3] == '>' && formatting)
        {
            italics = false;
            i += 3;
            w++;
        }
        else if (letter == 153)
        {
            w += SHORT(trademark->width);
            i++;
        }
        else if (letter == '(' && i < len - 3 && tolower(text[i + 1]) == 't' && tolower(text[i + 2]) == 'm' && text[i + 3] == ')'
            && formatting)
        {
            w += SHORT(trademark->width);
            i += 3;
        }
        else if (letter == 169)
        {
            w += SHORT(copyright->width);
            i++;
        }
        else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'c' && text[i + 2] == ')' && formatting)
        {
            w += SHORT(copyright->width);
            i += 2;
        }
        else if (letter == 174)
        {
            w += SHORT(regomark->width);
            i++;
        }
        else if (letter == '(' && i < len - 2 && tolower(text[i + 1]) == 'r' && text[i + 2] == ')' && formatting)
        {
            w += SHORT(regomark->width);
            i += 2;
        }
        else if (letter == 176)
        {
            w += SHORT(degree->width);
            i++;
        }
        else if (letter == 215 || (letter == 'x' && isdigit(prevletter)
            && ((nextletter = (i < len - 1 ? text[i + 1] : '\0')) == '\0' || isdigit(nextletter))))
            w += SHORT(multiply->width);
        else if (!i || prevletter == ' ' || prevletter == '(' || prevletter == '[' || prevletter == '\t')
        {
            if (letter == '\'')
                w += SHORT(lsquote->width);
            else if (letter == '\"')
                w += SHORT(ldquote->width);
            else
            {
                const int   c = letter - CONSOLEFONTSTART;

                w += SHORT((c >= 0 && c < CONSOLEFONTSIZE ? consolefont[c] : unknownchar)->width);
            }
        }
        else
        {
            const int   c = letter - CONSOLEFONTSTART;

            w += SHORT((c >= 0 && c < CONSOLEFONTSIZE ? consolefont[c] : unknownchar)->width);

            if (letter == '-' && italics)
                w++;
        }

        if (kerning)
        {
            for (int j = 0; altkern[j].char1; j++)
                if (prevletter == altkern[j].char1 && letter == altkern[j].char2)
                {
                    w += altkern[j].adjust;
                    break;
                }

            if (prevletter == '/' && italics)
                w -= 2;
            else if (prevletter == '.' && letter == ' ' && !bold && !italics)
                w++;
        }

        prevletter = letter;
    }

    return w;
}

static void C_DrawScrollbar(void)
{
    const int   trackend = CONSOLESCROLLBARHEIGHT * CONSOLEWIDTH;
    const int   facestart = CONSOLESCROLLBARHEIGHT * (outputhistory == -1 ?
                    MAX(0, consolestrings - CONSOLELINES) : outputhistory) / consolestrings;
    const int   faceend = facestart + CONSOLESCROLLBARHEIGHT - CONSOLESCROLLBARHEIGHT
                    * MAX(0, consolestrings - CONSOLELINES) / consolestrings;

    if (!facestart && trackend == faceend * CONSOLEWIDTH)
        scrollbardrawn = false;
    else
    {
        const int   offset = (CONSOLEHEIGHT - consoleheight) * CONSOLEWIDTH;
        const int   gripstart = (facestart + (faceend - facestart) / 2 - 2) * CONSOLEWIDTH;

        // draw scrollbar track
        for (int y = 0; y < trackend; y += CONSOLEWIDTH)
            if (y - offset >= CONSOLETOP)
                for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                    screens[0][y - offset + x] = tinttab50[screens[0][y - offset + x] + consolescrollbartrackcolor];

        // draw scrollbar face
        for (int y = facestart * CONSOLEWIDTH; y < faceend * CONSOLEWIDTH; y += CONSOLEWIDTH)
            if (y - offset >= CONSOLETOP)
                for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                    screens[0][y - offset + x] = consolescrollbarfacecolor;

        // draw scrollbar grip
        if (faceend - facestart > 8)
            for (int y = gripstart; y < gripstart + CONSOLEWIDTH * 6; y += CONSOLEWIDTH * 2)
                if (y - offset >= CONSOLETOP)
                    for (int x = CONSOLESCROLLBARX + 1; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH - 1; x++)
                        screens[0][y - offset + x] = consolescrollbargripcolor;

        // draw scrollbar face shadow
        if (faceend * CONSOLEWIDTH - offset >= CONSOLETOP)
            for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                screens[0][faceend * CONSOLEWIDTH - offset + x] = tinttab20[screens[0][faceend * CONSOLEWIDTH - offset + x]];

        scrollbardrawn = true;
    }
}

void C_Init(void)
{
    for (int i = 0, j = CONSOLEFONTSTART; i < CONSOLEFONTSIZE; i++)
    {
        char    buffer[9];

        M_snprintf(buffer, sizeof(buffer), "DRFON%03d", j++);
        consolefont[i] = W_CacheLumpName(buffer);
    }

    consolecaretcolor = nearestcolors[consolecaretcolor];
    consolelowfpscolor = nearestcolors[consolelowfpscolor];
    consolehighfpscolor = nearestcolors[consolehighfpscolor];
    consoletimercolor = nearestcolors[consoletimercolor];
    consoleinputcolor = nearestcolors[consoleinputcolor];
    consoleselectedinputcolor = nearestcolors[consoleselectedinputcolor];
    consoleselectedinputbackgroundcolor = nearestcolors[consoleselectedinputbackgroundcolor];
    consoleinputtooutputcolor = nearestcolors[consoleinputtooutputcolor];
    consoleplayermessagecolor = nearestcolors[consoleplayermessagecolor];
    consoletimestampcolor = nearestcolors[consoletimestampcolor];
    consoleoutputcolor = nearestcolors[consoleoutputcolor];
    consoleboldcolor = nearestcolors[consoleboldcolor];
    consoleitalicscolor = nearestcolors[consoleitalicscolor];
    consolewarningcolor = nearestcolors[consolewarningcolor];
    consolewarningboldcolor = nearestcolors[consolewarningboldcolor];
    consoledividercolor = nearestcolors[consoledividercolor] << 8;
    consolescrollbartrackcolor = nearestcolors[consolescrollbartrackcolor] << 8;
    consolescrollbarfacecolor = nearestcolors[consolescrollbarfacecolor];
    consolescrollbargripcolor = nearestcolors[consolescrollbargripcolor];

    consolecolors[inputstring] = consoleinputtooutputcolor;
    consolecolors[outputstring] = consoleoutputcolor;
    consolecolors[dividerstring] = consoledividercolor;
    consolecolors[warningstring] = consolewarningcolor;
    consolecolors[playermessagestring] = consoleplayermessagecolor;
    consolecolors[obituarystring] = consoleplayermessagecolor;

    consolebevel = &tinttab50[nearestblack << 8];
    consoleautomapbevel = &tinttab50[nearestcolors[5] << 8];

    brand = W_CacheLumpName("DRBRAND");
    dot = W_CacheLumpName("DRFON046");
    lsquote = W_CacheLumpName("DRFON145");
    ldquote = W_CacheLumpName("DRFON147");
    trademark = W_CacheLumpName("DRFON153");
    copyright = W_CacheLumpName("DRFON169");
    regomark = W_CacheLumpName("DRFON174");
    degree = W_CacheLumpName("DRFON176");
    multiply = W_CacheLumpName("DRFON215");
    unknownchar = W_CacheLumpName("DRFON000");

    warning = W_CacheLumpName("DRFONWRN");
    altunderscores = W_CacheLumpName("DRFONUND");

    bindlist = W_CacheLumpName("DRBNDLST");
    cmdlist = W_CacheLumpName("DRCMDLST");
    cvarlist = W_CacheLumpName("DRCVRLST");
    maplist = W_CacheLumpName("DRMAPLST");
    mapstats = W_CacheLumpName("DRMAPST");
    playerstats = W_CacheLumpName("DRPLYRST");
    thinglist = W_CacheLumpName("DRTHNLST");

    brandwidth = SHORT(brand->width);
    brandheight = SHORT(brand->height);
    spacewidth = SHORT(consolefont[' ' - CONSOLEFONTSTART]->width);
    timerx = CONSOLEWIDTH - C_TextWidth("00:00:00", false, false) - CONSOLETEXTX + 1;
    zerowidth = SHORT(consolefont['0' - CONSOLEFONTSTART]->width);
    warningwidth = SHORT(warning->width);
    dotwidth = SHORT(dot->width);

    while (*autocompletelist[++numautocomplete].text);
}

void C_ShowConsole(void)
{
    consoleheight = MAX(1, consoleheight);
    consoledirection = 1;
    consoleanim = 0;
    showcaret = true;
    caretwait = 0;
    skipaction = false;

    if (viewplayer)
        viewplayer->damagecount = MIN(viewplayer->damagecount, (NUMREDPALS - 1) << 3);

    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++)
        mousebuttons[i] = false;

    if (gamestate == GS_TITLESCREEN && !devparm)
    {
        S_StartSound(NULL, sfx_swtchn);
        D_FadeScreen();
    }

    S_LowerMusicVolume();
    SDL_StartTextInput();
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
        D_FadeScreen();
    }

    S_SetMusicVolume(musicVolume * MIX_MAX_VOLUME / 31);
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

    S_SetMusicVolume(musicVolume * MIX_MAX_VOLUME / 31);
}

static void C_DrawBackground(void)
{
    static dboolean blurred;
    static byte     blurscreen[SCREENAREA];
    int             consolebackcolor = nearestcolors[con_backcolor] << 8;
    int             consoleedgecolor = nearestcolors[con_edgecolor] << 8;
    int             height = (consoleheight + 5) * CONSOLEWIDTH;

    if (!blurred)
    {
        // blur background
        memcpy(blurscreen, screens[0], height);

        for (int y = 0; y <= height - CONSOLEWIDTH; y += CONSOLEWIDTH)
            for (int x = y; x <= y + CONSOLEWIDTH - 2; x++)
                blurscreen[x] = tinttab50[(blurscreen[x + 1] << 8) + blurscreen[x]];

        for (int y = 0; y <= height - CONSOLEWIDTH; y += CONSOLEWIDTH)
            for (int x = y + CONSOLEWIDTH - 2; x >= y; x--)
                blurscreen[x] = tinttab50[(blurscreen[x - 1] << 8) + blurscreen[x]];

        for (int y = CONSOLEWIDTH; y <= height - CONSOLEWIDTH * 2; y += CONSOLEWIDTH)
            for (int x = y; x <= y + CONSOLEWIDTH - 2; x++)
                blurscreen[x] = tinttab50[(blurscreen[x] << 8) + blurscreen[x]];

        for (int y = height - CONSOLEWIDTH; y >= CONSOLEWIDTH; y -= CONSOLEWIDTH)
            for (int x = y + CONSOLEWIDTH - 1; x >= y + 1; x--)
                blurscreen[x] = tinttab50[(blurscreen[x - CONSOLEWIDTH - 1] << 8) + blurscreen[x]];

        for (int y = 0; y <= height - CONSOLEWIDTH * 2; y += CONSOLEWIDTH)
            for (int x = y; x <= y + CONSOLEWIDTH - 1; x++)
                blurscreen[x] = tinttab50[(blurscreen[x + CONSOLEWIDTH] << 8) + blurscreen[x]];

        for (int y = height - CONSOLEWIDTH; y >= CONSOLEWIDTH; y -= CONSOLEWIDTH)
            for (int x = y; x <= y + CONSOLEWIDTH - 1; x++)
                blurscreen[x] = tinttab50[(blurscreen[x - CONSOLEWIDTH] << 8) + blurscreen[x]];

        for (int y = 0; y <= height - CONSOLEWIDTH * 2; y += CONSOLEWIDTH)
            for (int x = y + CONSOLEWIDTH - 1; x >= y + 1; x--)
                blurscreen[x] = tinttab50[(blurscreen[x + CONSOLEWIDTH - 1] << 8) + blurscreen[x]];

        for (int y = height - CONSOLEWIDTH; y >= CONSOLEWIDTH; y -= CONSOLEWIDTH)
            for (int x = y; x <= y + CONSOLEWIDTH - 2; x++)
                blurscreen[x] = tinttab50[(blurscreen[x - CONSOLEWIDTH + 1] << 8) + blurscreen[x]];
    }

    if (forceconsoleblurredraw)
    {
        forceconsoleblurredraw = false;
        blurred = false;
    }
    else
        blurred = (consoleheight == CONSOLEHEIGHT && !dowipe);

    // tint background using con_backcolor CVAR
    for (int i = 0; i < height; i++)
        screens[0][i] = tinttab50[consolebackcolor + blurscreen[i]];

    // apply corrugated glass effect to background
    for (int i = height - 2; i > 1; i -= 3)
        screens[0][i + 1] = colormaps[0][6 * 256 + screens[0][i + ((i % CONSOLEWIDTH) && (i + 1) % CONSOLEWIDTH ? -1 : 1)]];

    // draw branding
    V_DrawConsoleBrandingPatch(CONSOLEWIDTH - brandwidth, consoleheight - brandheight + 2, brand, consoleedgecolor);

    // draw bottom edge
    for (int i = height - CONSOLEWIDTH * 3; i < height; i++)
        screens[0][i] = tinttab50[consoleedgecolor + screens[0][i]];

    // bevel left and right edges
    if (automapactive && am_backcolor == am_backcolor_default)
    {
        for (int i = 0; i < height - 3 * CONSOLEWIDTH; i += CONSOLEWIDTH)
            screens[0][i] = consoleautomapbevel[screens[0][i + 1]];

        for (int i = MAX(0, height - 3 * CONSOLEWIDTH); i < height; i += CONSOLEWIDTH)
            screens[0][i] = consolebevel[screens[0][i + 1]];

        for (int i = 0; i < height - (brandheight + 3) * CONSOLEWIDTH; i += CONSOLEWIDTH)
            screens[0][i + CONSOLEWIDTH - 1] = consoleautomapbevel[screens[0][i + CONSOLEWIDTH - 2]];

        for (int i = MAX(0, height - (brandheight + 3) * CONSOLEWIDTH); i < height; i += CONSOLEWIDTH)
            screens[0][i + CONSOLEWIDTH - 1] = consolebevel[screens[0][i + CONSOLEWIDTH - 2]];
    }
    else
        for (int i = 0; i < height; i += CONSOLEWIDTH)
        {
            screens[0][i] = consolebevel[screens[0][i + 1]];
            screens[0][i + CONSOLEWIDTH - 1] = consolebevel[screens[0][i + CONSOLEWIDTH - 2]];
        }

    // bevel bottom edge
    for (int i = height - CONSOLEWIDTH + 1; i < height - 1; i++)
        screens[0][i] = consolebevel[screens[0][i]];

    // draw shadow
    if (gamestate != GS_TITLESCREEN)
        for (int i = CONSOLEWIDTH; i <= 4 * CONSOLEWIDTH; i += CONSOLEWIDTH)
            for (int j = height; j < height + i; j++)
                screens[0][j] = colormaps[0][4 * 256 + screens[0][j]];
}

static int C_DrawConsoleText(int x, int y, char *text, const int color1, const int color2, const int boldcolor,
    byte *translucency, const int tabs[4], const dboolean formatting, const dboolean kerning, const int index)
{
    int             bold = 0;
    dboolean        italics = false;
    int             tab = -1;
    int             len = (int)strlen(text);
    int             truncate = len;
    unsigned char   prevletter = '\0';
    int             width = 0;
    int             lastcolor1 = color1;
    int             startx = x;

    y -= CONSOLEHEIGHT - consoleheight;

    if (console[index].stringtype == warningstring)
    {
        if (console[index].line == 2)
        {
            if (text[0] == ' ')
                width -= spacewidth;
        }
        else
            V_DrawConsoleOutputTextPatch(x, y, warning, warningwidth, color1, color2, false, translucency);

        width += warningwidth + 1;
        x += width;
    }

    if (console[index].truncate)
        truncate = console[index].truncate;
    else if (len > 100)
    {
        do
        {
            char    *temp = M_SubString(text, 0, truncate);
            int     width2 = C_TextWidth(temp, formatting, kerning) + width + 6;

            free(temp);

            if (width2 <= CONSOLETEXTPIXELWIDTH)
                break;
        } while (truncate-- > 0);

        if (truncate == len - 1 && text[truncate] == '.')
            truncate++;

        if (text[truncate - 1] == ' ')
            truncate--;

        console[index].truncate = truncate;
    }

    for (int i = 0; i < truncate; i++)
    {
        const unsigned char letter = text[i];

        if (letter == '<' && i < len - 2 && tolower(text[i + 1]) == 'b' && text[i + 2] == '>' && formatting)
        {
            bold = (italics ? 2 : 1);
            i += 2;
        }
        else if (letter == '<' && i < len - 3 && text[i + 1] == '/' && tolower(text[i + 2]) == 'b' && text[i + 3] == '>' && formatting)
        {
            bold = 0;
            i += 3;
        }
        else if (letter == '<' && i < len - 2 && tolower(text[i + 1]) == 'i' && text[i + 2] == '>' && formatting)
        {
            italics = true;
            i += 2;
        }
        else if (letter == '<' && i < len - 3 && text[i + 1] == '/' && tolower(text[i + 2]) == 'i' && text[i + 3] == '>' && formatting)
        {
            italics = false;
            i += 3;
            x++;
        }
        else
        {
            patch_t         *patch = NULL;
            unsigned char   nextletter;

            if (letter == ' ' && formatting)
                x += spacewidth;
            else if (letter == '\t')
                x = (x > tabs[++tab] ? x + spacewidth : tabs[tab]);
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
            else if (letter == 215 || (letter == 'x' && isdigit(prevletter)
                && ((nextletter = (i < len - 1 ? text[i + 1] : '\0')) == '\0' || isdigit(nextletter))))
                patch = multiply;
            else
            {
                const int   c = letter - CONSOLEFONTSTART;

                patch = (c >= 0 && c < CONSOLEFONTSIZE ? consolefont[c] : unknownchar);

                if (!i || prevletter == ' ' || prevletter == '(' || prevletter == '[' || prevletter == '\t')
                {
                    if (letter == '\'')
                        patch = lsquote;
                    else if (letter == '\"')
                        patch = ldquote;
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

                    if (prevletter == '/')
                        x -= 2;
                    else if (prevletter == '\'')
                        x++;
                }
                else if (prevletter == '.' && letter == ' ' && !bold)
                    x++;
            }

            if (patch)
            {
                int patchwidth = SHORT(patch->width);

                consoletextfunc(x, y, patch, patchwidth, (lastcolor1 = (bold == 1 ? boldcolor : (bold == 2 ? color1 : (italics ?
                    (color1 == consolewarningcolor ? color1 : consoleitalicscolor) : color1)))), color2,
                    (italics && letter != '_' && letter != '-' && letter != '+' && letter != ',' && letter != '/'), translucency);
                x += patchwidth;
            }

            prevletter = letter;
        }
    }

    if (truncate < len)
    {
        if (kerning)
            for (int j = 0; altkern[j].char1; j++)
                if (prevletter == altkern[j].char1 && altkern[j].char2 == '.')
                {
                    x += altkern[j].adjust;
                    break;
                }

        consoletextfunc(x, y, dot, dotwidth, lastcolor1, color2, false, translucency);
        x += dotwidth;
        consoletextfunc(x, y, dot, dotwidth, lastcolor1, color2, false, translucency);

        if (text[truncate - 1] != '.')
        {
            x += dotwidth;
            consoletextfunc(x, y, dot, dotwidth, lastcolor1, color2, false, translucency);
        }
    }

    return (x - startx);
}

int C_OverlayWidth(const char *text)
{
    const int   len = (int)strlen(text);
    int         w = 0;

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];

        if (letter == ' ')
            w += spacewidth;
        else if (isdigit(letter))
            w += zerowidth;
        else
            w += SHORT(consolefont[letter - CONSOLEFONTSTART]->width);
    }

    return w;
}

static void C_DrawOverlayText(int x, int y, const char *text, const int color, dboolean monospaced)
{
    const int   len = (int)strlen(text);
    byte        *tinttab = (r_hud_translucency ? (consoleactive ? tinttab75 : tinttab50) : NULL);

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];

        if (letter == ' ')
            x += spacewidth;
        else
        {
            patch_t *patch = consolefont[letter - CONSOLEFONTSTART];
            int     width = SHORT(patch->width);

            if (isdigit(letter))
            {
                V_DrawConsoleOutputTextPatch(x + (letter == '1') - (letter == '4'),
                    y, patch, width, color, NOBACKGROUNDCOLOR, false, tinttab);
                x += zerowidth;
            }
            else
            {
                V_DrawConsoleOutputTextPatch(x, y, patch, width, color, NOBACKGROUNDCOLOR, false, tinttab);
                x += width;
            }
        }
    }
}

char *C_CreateTimeStamp(int index)
{
    int hours = gamestarttime.tm_hour;
    int minutes = gamestarttime.tm_min;
    int seconds = gamestarttime.tm_sec;
    int tics = console[index].tics / TICRATE;

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

    M_snprintf(console[index].timestamp, 9, "%i:%02i:%02i", hours, minutes, seconds);
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

        x -= (i && ch != ':' ? zerowidth : width);
        V_DrawConsoleOutputTextPatch(x + (i && ch == '1'), y, patch, width, consoletimestampcolor, NOBACKGROUNDCOLOR, false, tinttab33);
    }
}

void C_UpdateFPS(void)
{
    if (!dowipe && !splashscreen)
    {
        char    buffer[32];

        M_snprintf(buffer, sizeof(buffer), "%i FPS (%.1fms)", framespersecond, 1000.0f / framespersecond);

        C_DrawOverlayText(CONSOLEWIDTH - C_OverlayWidth(buffer) - CONSOLETEXTX + 1, CONSOLETEXTY, buffer,
            (framespersecond < (refreshrate && vid_capfps != TICRATE ? refreshrate : TICRATE) ? consolelowfpscolor :
            consolehighfpscolor), false);
    }
}

void C_UpdateTimer(void)
{
    if (!paused && !menuactive && !vanilla)
    {
        static char buffer[9];
        int         tics = countdown;
        static int  prevtics;

        if (tics != prevtics)
        {
            int hours = (tics = (prevtics = tics) / TICRATE) / 3600;
            int minutes = ((tics %= 3600)) / 60;
            int seconds = tics % 60;

            M_snprintf(buffer, 9, "%02i:%02i:%02i", hours, minutes, seconds);
        }

        C_DrawOverlayText(timerx, (vid_showfps ? CONSOLETEXTY + CONSOLELINEHEIGHT : CONSOLETEXTY), buffer, consoletimercolor, true);
    }
}

void C_Drawer(void)
{
    if (consoleheight)
    {
        int             i;
        int             x = CONSOLETEXTX;
        int             start;
        int             end;
        int             len;
        char            partialinput[255];
        const dboolean  prevconsoleactive = consoleactive;
        static int      consolewait;
        int             tics = I_GetTimeMS();

        const int consoledown[] =
        {
             14,  28,  42,  56,  70,  84,  98, 112, 126, 140, 150, 152,
            154, 156, 158, 160, 161, 162, 163, 164, 165, 166, 167, 168
        };

        const int consoleup[] =
        {
            154, 140, 126, 112,  98,  84,  70,  56,  42,  28,  14,   0
        };

        const int notabs[4] = { 0 };

        // adjust console height
        if (gamestate == GS_TITLESCREEN)
            consoleheight = CONSOLEHEIGHT;
        else if (consolewait < tics)
        {
            consolewait = tics + 10;

            if (consoledirection == 1)
            {
                if (consoleheight < CONSOLEHEIGHT)
                {
                    if (consoleheight > consoledown[consoleanim])
                        consolewait = 0;
                    else
                        consoleheight = consoledown[consoleanim];

                    consoleanim++;
                }
            }
            else
            {
                if (consoleheight)
                {
                    if (consoleheight < consoleup[consoleanim])
                        consolewait = 0;
                    else
                        consoleheight = consoleup[consoleanim];

                    consoleanim++;
                }
            }
        }

        if (vid_motionblur && consoleheight < CONSOLEHEIGHT)
            I_SetMotionBlur(0);

        consoleactive = (consoledirection == 1);

        // cancel any gamepad vibrations
        if (!prevconsoleactive && (gp_vibrate_barrels || gp_vibrate_damage || gp_vibrate_weapons))
        {
            if (consoleactive)
            {
                restorevibrationstrength = idlevibrationstrength;
                idlevibrationstrength = 0;
            }
            else
                idlevibrationstrength = restorevibrationstrength;

            I_GamepadVibration(idlevibrationstrength);
        }

        // cancel any screen shake
        I_UpdateBlitFunc(false);

        // draw background and bottom edge
        C_DrawBackground();

        // draw the scrollbar
        C_DrawScrollbar();

        // draw console text
        consoletextfunc = &V_DrawConsoleOutputTextPatch;

        if (outputhistory == -1)
        {
            start = MAX(0, consolestrings - CONSOLELINES);
            end = consolestrings;
        }
        else
        {
            start = outputhistory;
            end = outputhistory + CONSOLELINES;
        }

        for (i = start; i < end; i++)
        {
            int                 y = CONSOLELINEHEIGHT * (i - start + MAX(0, CONSOLELINES - consolestrings)) - CONSOLELINEHEIGHT / 2 + 1;
            const stringtype_t  stringtype = console[i].stringtype;

            if (stringtype == playermessagestring || stringtype == obituarystring)
            {
                int width = C_DrawConsoleText(CONSOLETEXTX, y, console[i].string, consoleplayermessagecolor,
                                NOBACKGROUNDCOLOR, consoleplayermessagecolor, tinttab66, notabs, true, true, i);

                if (console[i].count > 1)
                {
                    char    buffer[CONSOLETEXTMAXLENGTH];
                    char    *temp = commify(console[i].count);

                    M_snprintf(buffer, sizeof(buffer), "(%s)", temp);
                    C_DrawConsoleText(CONSOLETEXTX + width + 2, y, buffer, consoleplayermessagecolor,
                        NOBACKGROUNDCOLOR, consoleplayermessagecolor, tinttab66, notabs, false, false, i);
                    free(temp);
                }

                C_DrawTimeStamp(CONSOLEWIDTH - CONSOLETEXTX * 2 - CONSOLESCROLLBARWIDTH + 3, y, i);
            }
            else if (stringtype == outputstring)
                C_DrawConsoleText(CONSOLETEXTX, y, console[i].string, consolecolors[stringtype],
                    NOBACKGROUNDCOLOR, consoleboldcolor, tinttab66, console[i].tabs, true, true, i);
            else if (stringtype == dividerstring)
            {
                if ((y += 5 - (CONSOLEHEIGHT - consoleheight)) >= CONSOLETOP)
                    for (int xx = CONSOLETEXTX; xx < CONSOLETEXTX + CONSOLETEXTPIXELWIDTH + 2; xx++)
                        screens[0][y * CONSOLEWIDTH + xx] = tinttab50[consoledividercolor + screens[0][y * CONSOLEWIDTH + xx]];

                if (++y >= CONSOLETOP)
                    for (int xx = CONSOLETEXTX; xx < CONSOLETEXTX + CONSOLETEXTPIXELWIDTH + 2; xx++)
                        screens[0][y * CONSOLEWIDTH + xx] = tinttab50[consoledividercolor + screens[0][y * CONSOLEWIDTH + xx]];
            }
            else if (stringtype == headerstring)
                V_DrawConsolePatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight),
                    console[i].header, nearestcolors[con_edgecolor] << 8);
            else if (stringtype == warningstring)
                C_DrawConsoleText(CONSOLETEXTX, y, console[i].string, consolecolors[stringtype], NOBACKGROUNDCOLOR,
                    consolewarningboldcolor, tinttab66, notabs, true, true, i);
            else
                C_DrawConsoleText(CONSOLETEXTX, y, console[i].string, consolecolors[stringtype], NOBACKGROUNDCOLOR,
                    consoleboldcolor, tinttab66, notabs, true, true, i);
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
                consoletextfunc = &V_DrawConsoleOutputTextPatch;
                x += C_DrawConsoleText(x, CONSOLEHEIGHT - 17, partialinput, consoleinputcolor,
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
                        int y = CONSOLEHEIGHT - 17 + i - (CONSOLEHEIGHT - consoleheight);

                        if (y >= CONSOLETOP)
                            screens[0][y * CONSOLEWIDTH + x - 1] = consoleselectedinputbackgroundcolor;
                    }

                    consoletextfunc = &V_DrawConsoleInputTextPatch;
                    x += C_DrawConsoleText(x, CONSOLEHEIGHT - 17, partialinput, consoleselectedinputcolor,
                             consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false, true, 0);

                    for (i = 1; i < CONSOLELINEHEIGHT - 1; i++)
                    {
                        int y = CONSOLEHEIGHT - 17 + i - (CONSOLEHEIGHT - consoleheight);

                        if (y >= CONSOLETOP)
                            screens[0][y * CONSOLEWIDTH + x] = consoleselectedinputbackgroundcolor;
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
                byte    *dest = &screens[0][(consoleheight - 17) * CONSOLEWIDTH + x];

                for (int y = 0; y < 14 * CONSOLEWIDTH; y += CONSOLEWIDTH)
                {
                    *(dest + y) = consolecaretcolor;
                    *(dest + y + 1) = consolecaretcolor;
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
                    int y = CONSOLEHEIGHT - 17 + i - (CONSOLEHEIGHT - consoleheight);

                    if (y >= CONSOLETOP)
                        screens[0][y * CONSOLEWIDTH + x - 1] = consoleselectedinputbackgroundcolor;
                }

                consoletextfunc = &V_DrawConsoleInputTextPatch;
                x += C_DrawConsoleText(x, CONSOLEHEIGHT - 17, partialinput, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false, true, i);

                for (i = 1; i < CONSOLELINEHEIGHT - 1; i++)
                {
                    int y = CONSOLEHEIGHT - 17 + i - (CONSOLEHEIGHT - consoleheight);

                    if (y >= CONSOLETOP)
                        screens[0][y * CONSOLEWIDTH + x] = consoleselectedinputbackgroundcolor;
                }
            }
        }

        // draw input text to right of caret
        len = (int)strlen(consoleinput);

        if (caretpos < len)
        {
            for (i = selectend; i < len; i++)
                partialinput[i - selectend] = consoleinput[i];

            partialinput[i - selectend] = '\0';

            if (partialinput[0] != '\0')
            {
                consoletextfunc = &V_DrawConsoleOutputTextPatch;
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, partialinput, consoleinputcolor,
                    NOBACKGROUNDCOLOR, NOBOLDCOLOR, NULL, notabs, false, true, i);
            }
        }

        I_Sleep(1);
    }
    else
        consoleactive = false;
}

dboolean C_ExecuteInputString(const char *input)
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

dboolean C_ValidateInput(char *input)
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
                M_StripQuotes(parms);

                if ((M_StringCompare(cmd, consolecmds[i].name) || M_StringCompare(cmd, consolecmds[i].alternate))
                    && consolecmds[i].func1(consolecmds[i].name, parms)
                    && (consolecmds[i].parameters || !*parms))
                {
                    if (!executingalias && !resettingcvar)
                    {
                        if (parms[0] != '\0')
                            C_Input((input[length - 1] == '%' ? "%s %s%" : "%s %s"), cmd, parms);
                        else
                            C_Input("%s%s", cmd, (input[length - 1] == ' ' ? " " : ""));
                    }

                    consolecmds[i].func2(consolecmds[i].name, parms);
                    return true;
                }
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

dboolean C_Responder(event_t *ev)
{
    static int  autocomplete = -1;
    static int  inputhistory = -1;
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

        if (key == keyboardconsole)
        {
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
                    char        *string = M_StringDuplicate(consoleinput);
                    char        *strings[255];
                    dboolean    result = false;

                    strings[0] = strtok(string, ";");
                    i = 0;

                    while (strings[i])
                    {
                        if (C_ValidateInput(strings[i]))
                            result = true;

                        strings[++i] = strtok(NULL, ";");
                    }

                    free(string);

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
                        forceconsoleblurredraw = true;
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
                    const int   direction = ((modstate & KMOD_SHIFT) ? -1 : 1);
                    const int   start = autocomplete;
                    static char input[255];
                    char        prefix[255] = "";
                    int         spaces1;
                    dboolean    endspace1;

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

                    while ((direction == -1 && autocomplete > 0) || (direction == 1 && autocomplete <= numautocomplete))
                    {
                        static char output[255];
                        int         spaces2;
                        dboolean    endspace2;
                        int         len2;
                        int         game;

                        autocomplete += direction;

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
                    undolevels--;
                    M_StringCopy(consoleinput, undohistory[undolevels].input, sizeof(consoleinput));
                    caretpos = undohistory[undolevels].caretpos;
                    selectstart = undohistory[undolevels].selectstart;
                    selectend = undohistory[undolevels].selectend;
                }

                break;
        }
    }
    else if (ev->type == ev_keyup)
    {
        scrollspeed = TICRATE;
        return false;
    }
    else if (ev->type == ev_textinput)
    {
        char    ch = (char)ev->data1;
        char    *temp = NULL;

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

    return true;
}

static const char *dayofweek(int d, int m, int y)
{
    const int   adjustment = (14 - m) / 12;
    const char  *days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    m += 12 * adjustment - 2;
    y -= adjustment;

    return days[(d + (13 * m - 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7];
}

void C_PrintCompileDate(void)
{
    char    mth[4] = "";
    int     day, year, hour, minute;

    if (sscanf(__DATE__, "%3s %2d %4d", mth, &day, &year) == 3 && sscanf(__TIME__, "%2d:%2d:%*d", &hour, &minute) == 2)
    {
        const char  mths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
        int         month = (int)(strstr(mths, mth) - mths) / 3;

        const char *months[] =
        {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };

        C_Output("This %i-bit <i><b>%s</b></i> %s of <i><b>%s</b></i> was built at %i:%02i%s on %s, %s %i, %i.",
            (int)sizeof(intptr_t) * 8, WINDOWS, EXECUTABLE, PACKAGE_NAMEANDVERSIONSTRING, hour - 12 * (hour > 12), minute,
            (hour < 12 ? "am" : "pm"), dayofweek(day, month + 1, year), months[month], day, year);
    }

#if defined(_MSC_FULL_VER)
    if (_MSC_BUILD)
        C_Output("It was compiled using v%i.%02i.%i.%i of the <i><b>Microsoft C/C++ Optimizing Compiler.</b></i>",
            _MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000, _MSC_BUILD);
    else
        C_Output("It was compiled using v%i.%02i.%i of the <i><b>Microsoft C/C++ Optimizing Compiler.</b></i>",
            _MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000);
#endif
}

void C_PrintSDLVersions(void)
{
    const int   revision = SDL_GetRevisionNumber();

    if (revision)
    {
        char    *temp = commify(revision);

        C_Output("Using v%i.%i.%i (revision %s) of the <i><b>SDL (Simple DirectMedia Layer)</b></i> library.",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, temp);
        free(temp);
    }
    else
        C_Output("Using v%i.%i.%i of the <i><b>SDL (Simple DirectMedia Layer)</b></i> library.",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

    C_Output("Using v%i.%i.%i of the <i><b>SDL_mixer</b></i> library and v%i.%i.%i of the <i><b>SDL_image</b></i> library.",
        SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL,
        SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);
}
