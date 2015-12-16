/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include <ctype.h>
#include <time.h>

#if defined(WIN32)
#include <windows.h>
#endif

#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_event.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_cheat.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define CONSOLESPEED            (CONSOLEHEIGHT / 12)

#define CONSOLETEXTX            10
#define CONSOLETEXTY            8
#define CONSOLELINES            11
#define CONSOLELINEHEIGHT       14

#define CONSOLEINPUTPIXELWIDTH  500

#define CONSOLESCROLLBARWIDTH   3
#define CONSOLESCROLLBARHEIGHT  ((CONSOLELINES - 1) * CONSOLELINEHEIGHT - 4)
#define CONSOLESCROLLBARX       (SCREENWIDTH - CONSOLETEXTX - CONSOLESCROLLBARWIDTH)
#define CONSOLESCROLLBARY       (CONSOLETEXTY + 1)

#define CONSOLEDIVIDERWIDTH     (SCREENWIDTH - CONSOLETEXTX * 3 - CONSOLESCROLLBARWIDTH)

#define DIVIDER                 "~~~"
#define ITALICS                 '~'

#define CARETBLINKTIME          530

dboolean        consoleactive = false;
int             consoleheight = 0;
int             consoledirection = -1;
static int      consolewait;

dboolean        forceblurredraw = false;

patch_t         *unknownchar;
patch_t         *consolefont[CONSOLEFONTSIZE];
patch_t         *lsquote;
patch_t         *ldquote;
patch_t         *degree;
patch_t         *multiply;

int             spacewidth;

char            consoleinput[255] = "";
int             consolestrings = 0;

int             undolevels = 0;

patch_t         *caret;
int             caretpos = 0;
static dboolean showcaret = true;
static int      caretwait;
static int      caretblinktime;
int             selectstart = 0;
int             selectend = 0;

char            consolecheat[255] = "";
char            consolecheatparm[3] = "";

static int      autocomplete = -1;
static char     autocompletetext[255] = "";

static int      inputhistory = -1;
char            currentinput[255] = "";

static int      outputhistory = -1;

static int      notabs[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

dboolean        con_timestamps = con_timestamps_default;
int             timestampx;
int             zerowidth;

extern dboolean r_translucency;
extern byte     *tinttab75;
extern int      fps;
extern dboolean alwaysrun;
extern int      key_alwaysrun;

void G_ToggleAlwaysRun(evtype_t type);
int FindNearestColor(byte *palette, int red, int green, int blue);

static const char *shiftxform =
{
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0 !\"#$%&\"()*+<_>?"
    ")!@#$%^&*(::<+>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}\"_'ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~\0"
};

byte            c_tempscreen[SCREENWIDTH * SCREENHEIGHT];
byte            c_blurscreen[SCREENWIDTH * SCREENHEIGHT];

int             consolecaretcolor = 4;
int             consolehighfpscolor = 116;
int             consoleinputcolor = 4;
int             consoleselectedinputcolor = 4;
int             consoleselectedinputbackgroundcolor = 100;
int             consoleinputtooutputcolor = 4;
int             consolelowfpscolor = 180;
int             consoletitlecolor = 88;
int             consolememorycolor = 88;
int             consoleplayermessagecolor = 161;
int             consoletimestampcolor = 100;
int             consoleoutputcolor = 88;
int             consolebrandingcolor = 100;
int             consolewarningcolor = 180;
int             consoledividercolor = 100;
int             consoletintcolor = 5;
int             consoleedgecolor1 = 105;
int             consoleedgecolor2 = 100;
int             consolescrollbartrackcolor = 100;
int             consolescrollbarfacecolor = 88;

int             consolecolors[STRINGTYPES];

void C_DebugOutput(char *string)
{
#if defined(_MSC_VER) && defined (_DEBUG)
    OutputDebugString(M_StringJoin(string, "\n", NULL));
#endif
}

void C_Print(stringtype_t type, char *string, ...)
{
    va_list     argptr;
    char        buffer[1024] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(buffer);
    console[consolestrings].type = type;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings].timestamp = "";
    ++consolestrings;
    outputhistory = -1;
    C_DebugOutput(buffer);
}

void C_Input(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(buffer);
    console[consolestrings].type = inputstring;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings].timestamp = "";
    ++consolestrings;
    outputhistory = -1;
    C_DebugOutput(buffer);
}

void C_Output(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(buffer);
    console[consolestrings].type = outputstring;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings].timestamp = "";
    ++consolestrings;
    outputhistory = -1;
    C_DebugOutput(buffer);
}

void C_TabbedOutput(int tabs[8], char *string, ...)
{
    va_list     argptr;
    char        buffer[1024] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(buffer);
    console[consolestrings].type = outputstring;
    memcpy(console[consolestrings].tabs, tabs, sizeof(console[consolestrings].tabs));
    console[consolestrings].timestamp = "";
    ++consolestrings;
    outputhistory = -1;
    C_DebugOutput(buffer);
}

void C_Warning(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    if (consolestrings && !M_StringCompare(console[consolestrings - 1].string, buffer))
    {
        console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
        console[consolestrings].string = strdup(buffer);
        console[consolestrings].type = warningstring;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        console[consolestrings].timestamp = "";
        ++consolestrings;
        outputhistory = -1;
        C_DebugOutput(buffer);
    }
}

void C_PlayerMessage(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    if (consolestrings && M_StringCompare(console[consolestrings - 1].string, buffer))
    {
        M_snprintf(buffer, sizeof(buffer), "%s (2)", console[consolestrings - 1].string);
        console[consolestrings - 1].string = strdup(buffer);
    }
    else if (consolestrings && M_StringStartsWith(console[consolestrings - 1].string, buffer))
    {
        char    *count = strrchr(console[consolestrings - 1].string, '(') + 1;

        count[strlen(count) - 1] = '\0';

        M_snprintf(buffer, sizeof(buffer), "%s (%i)", buffer, atoi(count) + 1);
        console[consolestrings - 1].string = strdup(buffer);
    }
    else
    {
        time_t          rawtime;
        struct tm       *timeinfo;

        console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
        console[consolestrings].string = strdup(buffer);
        console[consolestrings].type = playermessagestring;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
        console[consolestrings].timestamp = strdup(buffer);

        ++consolestrings;
    }
    outputhistory = -1;
    C_DebugOutput(buffer);
}

static void C_AddToUndoHistory(void)
{
    undohistory = Z_Realloc(undohistory, (undolevels + 1) * sizeof(*undohistory));
    undohistory[undolevels].input = strdup(consoleinput);
    undohistory[undolevels].caretpos = caretpos;
    undohistory[undolevels].selectstart = selectstart;
    undohistory[undolevels].selectend = selectend;
    ++undolevels;
}

void C_AddConsoleDivider(void)
{
    if (!consolestrings || !M_StringCompare(console[consolestrings - 1].string, DIVIDER))
        C_Print(dividerstring, DIVIDER);
}

static void C_DrawDivider(int y)
{
    int i;

    y *= SCREENWIDTH;
    if (y >= CONSOLETOP * SCREENWIDTH)
        for (i = y + CONSOLETEXTX; i < y + CONSOLETEXTX + CONSOLEDIVIDERWIDTH; ++i)
            screens[0][i] = tinttab50[screens[0][i] + consoledividercolor];
    if ((y += SCREENWIDTH) >= CONSOLETOP * SCREENWIDTH)
        for (i = y + CONSOLETEXTX; i < y + CONSOLETEXTX + CONSOLEDIVIDERWIDTH; ++i)
            screens[0][i] = tinttab50[screens[0][i] + consoledividercolor];
    C_DebugOutput(DIVIDERSTRING);
}

static struct
{
    char        char1;
    char        char2;
    int         adjust;
} kern[] = {
    { ' ',  '(',  -1 }, { '\\', 'V',  -1 }, { '\"', '+',  -1 }, { '\"', '.',  -1 },
    { '\"', 'a',  -1 }, { '\"', 'c',  -1 }, { '\"', 'd',  -1 }, { '\"', 'e',  -1 },
    { '\"', 'g',  -1 }, { '\"', 'j',  -2 }, { '\"', 'o',  -1 }, { '\"', 'q',  -1 },
    { '\"', 's',  -1 }, { '\'', 'a',  -1 }, { '\'', 'c',  -1 }, { '\'', 'd',  -1 },
    { '\'', 'e',  -1 }, { '\'', 'g',  -1 }, { '\'', 'j',  -2 }, { '\'', 'o',  -1 },
    { '\"', 'q',  -1 }, { '\'', 's',  -1 }, { '.',  '\\', -1 }, { '.',  '7',  -1 },
    { ',',  '4',  -1 }, { '/',  'o',  -1 }, { ':', '\\',  -1 }, { '_',  'f',  -1 },
    { '0',  ',',  -1 }, { '0',  'j',  -2 }, { '1',  '\"', -1 }, { '1',  '\'', -1 },
    { '1',  'j',  -2 }, { '2',  'j',  -2 }, { '3',  ',',  -1 }, { '3',  'j',  -2 },
    { '4',  'j',  -2 }, { '5',  ',',  -1 }, { '5',  'j',  -2 }, { '6',  ',',  -1 },
    { '6',  'j',  -2 }, { '7',  ',',  -2 }, { '7',  'j',  -2 }, { '8',  ',',  -1 },
    { '8',  'j',  -2 }, { '9',  ',',  -1 }, { '9',  'j',  -2 }, { 'F',  '.',  -1 },
    { 'F',  ',',  -1 }, { 'L',  '\\', -1 }, { 'L',  '\"', -1 }, { 'L',  '\'', -1 },
    { 'P',  '.',  -1 }, { 'P',  ',',  -1 }, { 'T',  '.',  -1 }, { 'T',  ',',  -1 },
    { 'V',  '.',  -1 }, { 'V',  ',',  -1 }, { 'Y',  '.',  -1 }, { 'Y',  ',',  -1 },
    { 'a',  '\"', -1 }, { 'a',  '\'', -1 }, { 'a',  'j',  -2 }, { 'b',  ',',  -1 },
    { 'b',  '\"', -1 }, { 'b',  '\\', -1 }, { 'b',  '\'', -1 }, { 'b',  'j',  -2 },
    { 'c',  '\\', -1 }, { 'c',  ',',  -1 }, { 'c',  '\"', -1 }, { 'c',  '\'', -1 },
    { 'c',  'j',  -2 }, { 'd',  'j',  -2 }, { 'e',  '\\', -1 }, { 'e',  ',',  -1 },
    { 'e',  '\"', -1 }, { 'e',  '\'', -1 }, { 'e',  '_',  -1 }, { 'e',  'j',  -2 },
    { 'f',  ' ',  -1 }, { 'f',  ',',  -2 }, { 'f',  '_',  -1 }, { 'f',  'a',  -1 },
    { 'f',  'j',  -2 }, { 'h',  '\\', -1 }, { 'h',  '\"', -1 }, { 'h',  '\'', -1 },
    { 'h',  'j',  -2 }, { 'i',  'j',  -2 }, { 'k',  'j',  -2 }, { 'l',  'j',  -2 },
    { 'm',  '\"', -1 }, { 'm',  '\\', -1 }, { 'm',  '\'', -1 }, { 'm',  'j',  -2 },
    { 'n',  '\\', -1 }, { 'n',  '\"', -1 }, { 'n',  '\'', -1 }, { 'n',  'j',  -2 },
    { 'o',  '\\', -1 }, { 'o',  ',',  -1 }, { 'o',  '\"', -1 }, { 'o',  '\'', -1 },
    { 'o',  'j',  -2 }, { 'p',  '\\', -1 }, { 'p',  ',',  -1 }, { 'p',  '\"', -1 },
    { 'p',  '\'', -1 }, { 'p',  'j',  -2 }, { 'r',  ' ',  -1 }, { 'r',  '\\', -1 },
    { 'r',  '.',  -2 }, { 'r',  ',',  -2 }, { 'r',  '\"', -1 }, { 'r',  '\'', -1 },
    { 'r',  '_',  -1 }, { 'r',  'a',  -1 }, { 'r',  'j',  -2 }, { 's',  ',',  -1 },
    { 's',  'j',  -2 }, { 't',  'j',  -2 }, { 'u',  'j',  -2 }, { 'v',  ',',  -1 },
    { 'v',  'j',  -2 }, { 'w',  'j',  -2 }, { 'x',  'j',  -2 }, { 'z',  'j',  -2 },
    {  0 ,   0 ,   0 }
};

static int C_TextWidth(char *text)
{
    size_t      i;
    size_t      len = strlen(text);
    char        prevletter = '\0';
    int         w = 0;

    for (i = 0; i < len; ++i)
    {
        char    letter = text[i];
        int     c = letter - CONSOLEFONTSTART;
        char    nextletter = text[i + 1];
        int     j = 0;

        if (letter == '\xc2' && nextletter == '\xb0')
        {
            w += SHORT(degree->width);
            ++i;
        }
        else
            w += SHORT(c < 0 || c >= CONSOLEFONTSIZE ? unknownchar->width : consolefont[c]->width);

        while (kern[j].char1)
        {
            if (prevletter == kern[j].char1 && letter == kern[j].char2)
            {
                w += kern[j].adjust;
                break;
            }
            ++j;
        }
        prevletter = letter;
    }
    return w;
}

static void C_DrawScrollbar(void)
{
    int x, y;
    int trackstart;
    int trackend;
    int facestart;
    int faceend;
    int offset = (CONSOLEHEIGHT - consoleheight) * SCREENWIDTH;

    // Draw scrollbar track
    trackstart = CONSOLESCROLLBARY * SCREENWIDTH;
    trackend = trackstart + CONSOLESCROLLBARHEIGHT * SCREENWIDTH;
    for (y = trackstart; y < trackend; y += SCREENWIDTH)
        if (y - offset >= 0)
            for (x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; ++x)
                screens[0][y - offset + x] = tinttab50[screens[0][y - offset + x]
                    + consolescrollbartrackcolor];

    // Draw scrollbar face
    facestart = (CONSOLESCROLLBARY + CONSOLESCROLLBARHEIGHT * (outputhistory == -1 ?
        MAX(0, consolestrings - CONSOLELINES) : outputhistory) / consolestrings) * SCREENWIDTH;
    faceend = facestart + (CONSOLESCROLLBARHEIGHT - CONSOLESCROLLBARHEIGHT
        * MAX(0, consolestrings - CONSOLELINES) / consolestrings) * SCREENWIDTH;

    for (y = facestart; y < faceend; y += SCREENWIDTH)
        if (y - offset >= 0)
            for (x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; ++x)
                screens[0][y - offset + x] = consolescrollbarfacecolor;
}

void C_Init(void)
{
    int         i;
    int         j = CONSOLEFONTSTART;
    char        buffer[9];

    while (consolecmds[numconsolecmds++].name[0]);

    unknownchar = W_CacheLumpName("DRFON000", PU_STATIC);
    for (i = 0; i < CONSOLEFONTSIZE; i++)
    {
        M_snprintf(buffer, 9, "DRFON%03d", j++);
        consolefont[i] = W_CacheLumpName(buffer, PU_STATIC);
    }
    lsquote = W_CacheLumpName("DRFON145", PU_STATIC);
    ldquote = W_CacheLumpName("DRFON147", PU_STATIC);
    degree = W_CacheLumpName("DRFON176", PU_STATIC);
    multiply = W_CacheLumpName("DRFON215", PU_STATIC);

    caret = W_CacheLumpName("CARET", PU_STATIC);

#if defined(WIN32)
    caretblinktime = GetCaretBlinkTime();
#else
    caretblinktime = CARETBLINKTIME;
#endif

    spacewidth = SHORT(consolefont[' ' - CONSOLEFONTSTART]->width);
    timestampx = SCREENWIDTH - C_TextWidth("00:00:00") - CONSOLETEXTX * 2
        - CONSOLESCROLLBARWIDTH + 1;
    zerowidth = SHORT(consolefont['0' - CONSOLEFONTSTART]->width);

    if (W_CheckMultipleLumps("STCFN065") > 1)
    {
        byte    *playpal = W_CacheLumpName("PLAYPAL", PU_CACHE);
        int     red = 0, green = 0, blue = 0, total = 0;

        V_AverageColorInPatch(W_CacheLumpName("STCFN065", PU_STATIC), &red, &green, &blue, &total);
        V_AverageColorInPatch(W_CacheLumpName("STCFN066", PU_STATIC), &red, &green, &blue, &total);
        V_AverageColorInPatch(W_CacheLumpName("STCFN067", PU_STATIC), &red, &green, &blue, &total);
        if (total > 0)
        {
            int color = FindNearestColor(playpal, red / total, green / total, blue / total);

            if (color < 176 || color > 191)
                consoleplayermessagecolor = color;
        }
    }

    consolecolors[inputstring] = consoleinputtooutputcolor;
    consolecolors[outputstring] = consoleoutputcolor;
    consolecolors[dividerstring] = consoledividercolor;
    consolecolors[titlestring] = consoletitlecolor;
    consolecolors[warningstring] = consolewarningcolor;
    consolecolors[playermessagestring] = consoleplayermessagecolor;

    consoletintcolor <<= 8;
    consoleedgecolor1 <<= 8;
    consoleedgecolor2 <<= 8;
    consolescrollbartrackcolor <<= 8;
    consoledividercolor <<= 8;
}

void C_HideConsole(void)
{
    consoledirection = -1;
}

void C_HideConsoleFast(void)
{
    consoleheight = 0;
    consoledirection = -1;
    consoleactive = false;
}

void C_StripQuotes(char *string)
{
    size_t len = strlen(string);

    if (len > 2 && ((string[0] == '\"' && string[len - 1] == '\"')
        || (string[0] == '\'' && string[len - 1] == '\'')))
    {
        len -= 2;
        memmove(string, string + 1, len);
        string[len] = '\0';
    }
}

static void DoBlurScreen(int x1, int y1, int x2, int y2, int i)
{
    int x, y;

    memcpy(c_tempscreen, c_blurscreen, SCREENWIDTH * (CONSOLEHEIGHT + 5));

    for (y = y1; y < y2; y += SCREENWIDTH)
        for (x = y + x1; x < y + x2; ++x)
            c_blurscreen[x] = tinttab50[c_tempscreen[x] + (c_tempscreen[x + i] << 8)];
}

static void C_DrawBackground(int height)
{
    static dboolean     blurred;
    int                 i, j;

    height = (height + 5) * SCREENWIDTH;

    if (!blurred || forceblurredraw)
    {
        forceblurredraw = false;

        for (i = 0; i < height; ++i)
            c_blurscreen[i] = screens[0][i];

        DoBlurScreen(0, 0, SCREENWIDTH - 1, height, 1);
        DoBlurScreen(1, 0, SCREENWIDTH, height, -1);
        DoBlurScreen(0, 0, SCREENWIDTH - 1, height - SCREENWIDTH, SCREENWIDTH + 1);
        DoBlurScreen(1, SCREENWIDTH, SCREENWIDTH, height, -(SCREENWIDTH + 1));
        DoBlurScreen(0, 0, SCREENWIDTH, height - SCREENWIDTH, SCREENWIDTH);
        DoBlurScreen(0, SCREENWIDTH, SCREENWIDTH, height, -SCREENWIDTH);
        DoBlurScreen(1, 0, SCREENWIDTH, height - SCREENWIDTH, SCREENWIDTH - 1);
        DoBlurScreen(0, SCREENWIDTH, SCREENWIDTH - 1, height, -(SCREENWIDTH - 1));
    }

    blurred = (consoleheight == CONSOLEHEIGHT && !wipe);

    for (i = 0; i < height; ++i)
        screens[0][i] = tinttab50[c_blurscreen[i] + consoletintcolor];

    for (i = height - 2; i > 1; i -= 3)
    {
        screens[0][i] = colormaps[0][256 * 6 + screens[0][i]];
        if (((i - 1) % SCREENWIDTH) < SCREENWIDTH - 2)
            screens[0][i + 1] = colormaps[0][256 * 6 + screens[0][i - 1]];
    }

    for (i = height - SCREENWIDTH * 3; i < height - SCREENWIDTH * 2; ++i)
        screens[0][i] = tinttab25[consoleedgecolor1 + screens[0][i]];

    for (i = height - SCREENWIDTH * 2; i < height; ++i)
        screens[0][i] = tinttab25[consoleedgecolor2 + screens[0][i]];

    for (j = 1; j <= 4; ++j)
        for (i = height; i < height + SCREENWIDTH * j; ++i)
            screens[0][i] = colormaps[0][256 * 4 + screens[0][i]];
}

static void C_DrawConsoleText(int x, int y, char *text, int color1, int color2, byte *tinttab,
    int tabs[8])
{
    dboolean            italics = false;
    size_t              i;
    int                 tab = -1;
    size_t              len = strlen(text);
    unsigned char       prevletter = '\0';

    y -= (CONSOLEHEIGHT - consoleheight);

    if (len > 80)
        while (C_TextWidth(text) > SCREENWIDTH - CONSOLETEXTX * 3 - CONSOLESCROLLBARWIDTH + 2)
        {
            text[len - 1] = '.';
            text[len] = '.';
            text[len + 1] = '.';
            text[len + 2] = '\0';
            --len;
        }

    for (i = 0; i < len; ++i)
    {
        unsigned char   letter = text[i];
        int             c = letter - CONSOLEFONTSTART;
        unsigned char   nextletter = text[i + 1];

        if (letter == ITALICS && prevletter != ITALICS)
        {
            italics = !italics;
            if (!italics)
                ++x;
        }
        else
        {
            patch_t     *patch = NULL;

            if (letter == ITALICS)
                italics = false;
            if (letter == '\t')
                x = (x > tabs[++tab] ? x + spacewidth : tabs[tab]);
            else if (letter == '\xC2' && nextletter == '\xB0')
            {
                patch = degree;
                ++i;
            }
            else
                patch = (c < 0 || c >= CONSOLEFONTSIZE ? unknownchar : consolefont[c]);

            if (letter == 215)
                patch = multiply;
            else if (prevletter == ' ' || prevletter == '\t' || prevletter == '(' || !i)
            {
                if (letter == '\'')
                    patch = lsquote;
                else if (letter == '\"')
                    patch = ldquote;
            }

            if (!italics)
            {
                int     k = 0;

                while (kern[k].char1)
                {
                    if (prevletter == kern[k].char1 && letter == kern[k].char2)
                    {
                        x += kern[k].adjust;
                        break;
                    }
                    ++k;
                }
            }

            if (patch)
            {
                V_DrawConsoleChar(x, y, patch, color1, color2, italics, tinttab);
                x += SHORT(patch->width);
            }
        }
        prevletter = letter;
    }
}

static void C_DrawOverlayText(int x, int y, char *text, int color)
{
    size_t      i;
    size_t      len = strlen(text);

    for (i = 0; i < len; ++i)
    {
        char    letter = text[i];

        if (letter == ' ')
            x += spacewidth;
        else
        {
            patch_t     *patch = consolefont[letter - CONSOLEFONTSTART];

            V_DrawConsoleChar(x, y, patch, color, NOBACKGROUNDCOLOR, false, tinttab75);
            x += SHORT(patch->width);
        }
    }
}

static void C_DrawTimeStamp(int x, int y, char *text)
{
    size_t      i;
    size_t      len = strlen(text);

    y -= (CONSOLEHEIGHT - consoleheight);

    for (i = 0; i < len; ++i)
    {
        patch_t *patch = consolefont[text[i] - CONSOLEFONTSTART];

        V_DrawConsoleChar(x + (text[i] == '1' ? (zerowidth - SHORT(patch->width)) / 2 : 0), y,
            patch, consoletimestampcolor, NOBACKGROUNDCOLOR, false, tinttab25);
        x += (isdigit(text[i]) ? zerowidth : SHORT(patch->width));
    }
}

void C_UpdateFPS(void)
{
    if (fps && !wipe)
    {
        static char     buffer[16];

        M_snprintf(buffer, 16, "%i FPS", fps);

        C_DrawOverlayText(SCREENWIDTH - C_TextWidth(buffer) - CONSOLETEXTX + 1, CONSOLETEXTY,
            buffer, (fps < TICRATE ? consolelowfpscolor : consolehighfpscolor));
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
        char            *lefttext = Z_Malloc(512, PU_STATIC, NULL);
        char            *middletext = Z_Malloc(512, PU_STATIC, NULL);
        char            *righttext = Z_Malloc(512, PU_STATIC, NULL);
        dboolean        prevconsoleactive = consoleactive;

        // adjust console height
        if (consolewait < I_GetTime())
        {
            consoleheight = BETWEEN(0, consoleheight + CONSOLESPEED * consoledirection,
                CONSOLEHEIGHT);
            consolewait = I_GetTime();
        }

        consoleactive = (consoledirection == 1);

        // cancel any gamepad vibrations
        if (!prevconsoleactive && gp_vibrate && vibrate)
        {
            if (consoleactive)
            {
                restoremotorspeed = idlemotorspeed;
                idlemotorspeed = 0;
            }
            else
                idlemotorspeed = restoremotorspeed;
            XInputVibration(idlemotorspeed);
        }

        // cancel any screen shake
        I_UpdateBlitFunc();

        // draw background and bottom edge
        C_DrawBackground(consoleheight);

        // draw branding
        C_DrawConsoleText(SCREENWIDTH - C_TextWidth(PACKAGE_BRANDINGSTRING) - CONSOLETEXTX + 1,
            CONSOLEHEIGHT - 17, PACKAGE_BRANDINGSTRING, consolebrandingcolor, NOBACKGROUNDCOLOR,
            tinttab25, notabs);

        // draw console text
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
        for (i = start; i < end; ++i)
        {
            int y = CONSOLELINEHEIGHT * (i - start + MAX(0, CONSOLELINES - consolestrings))
                    - CONSOLELINEHEIGHT / 2 + 1;

            if (console[i].type == dividerstring)
                C_DrawDivider(y + 5 - (CONSOLEHEIGHT - consoleheight));
            else
            {
                C_DrawConsoleText(CONSOLETEXTX, y, console[i].string,
                    consolecolors[console[i].type], NOBACKGROUNDCOLOR, NULL, console[i].tabs);
                if (con_timestamps && console[i].timestamp[0])
                    C_DrawTimeStamp(timestampx, y, console[i].timestamp);
            }
        }

        // draw input text to left of caret
        for (i = 0; i < MIN(selectstart, caretpos); ++i)
            lefttext[i] = consoleinput[i];
        lefttext[i] = '\0';
        C_DrawConsoleText(x, CONSOLEHEIGHT - 17, lefttext, consoleinputcolor, NOBACKGROUNDCOLOR,
            NULL, notabs);
        x += C_TextWidth(lefttext);

        // draw any selected text to left of caret
        if (selectstart < caretpos)
        {
            for (i = selectstart; i < selectend; ++i)
                middletext[i - selectstart] = consoleinput[i];
            middletext[i - selectstart] = '\0';
            if (middletext[0])
            {
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, middletext, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, NULL, notabs);
                x += C_TextWidth(middletext);
            }
        }

        // draw caret
        if (caretwait < I_GetTimeMS())
        {
            showcaret = !showcaret;
            caretwait = I_GetTimeMS() + caretblinktime;
        }
        if (showcaret)
            if (selectend > caretpos)
                V_DrawConsoleChar(x, consoleheight - 17, caret, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, false, NULL);
            else
                V_DrawConsoleChar(x, consoleheight - 17, caret, consolecaretcolor,
                    NOBACKGROUNDCOLOR, false, NULL);
        x += SHORT(caret->width);

        // draw any selected text to right of caret
        if (selectend > caretpos)
        {
            for (i = selectstart; i < selectend; ++i)
                middletext[i - selectstart] = consoleinput[i];
            middletext[i - selectstart] = '\0';
            if (middletext[0])
            {
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, middletext, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, NULL, notabs);
                x += C_TextWidth(middletext);
            }
        }

        // draw input text to right of caret
        if ((unsigned int)caretpos < strlen(consoleinput))
        {
            for (i = selectend; (unsigned int)i < strlen(consoleinput); ++i)
                righttext[i - selectend] = consoleinput[i];
            righttext[i - selectend] = '\0';
            if (righttext[0])
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, righttext, consoleinputcolor,
                    NOBACKGROUNDCOLOR, NULL, notabs);
        }

        Z_Free(lefttext);
        Z_Free(middletext);
        Z_Free(righttext);

        // draw the scrollbar
        C_DrawScrollbar();
    }
    else
        consoleactive = false;
}

dboolean C_ValidateInput(char *input)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        char    cmd[256] = "";

        if (consolecmds[i].type == CT_CHEAT)
        {
            if (consolecmds[i].parameters)
            {
                size_t  length = strlen(input);

                if (isdigit(input[length - 2]) && isdigit(input[length - 1]))
                {
                    consolecheatparm[0] = input[length - 2];
                    consolecheatparm[1] = input[length - 1];
                    consolecheatparm[2] = '\0';

                    M_StringCopy(cmd, input, 255);
                    cmd[length - 2] = '\0';

                    if ((M_StringCompare(cmd, consolecmds[i].name)
                        || M_StringCompare(cmd, consolecmds[i].alternate))
                        && length == strlen(cmd) + 2
                        && consolecmds[i].func1(consolecmds[i].name, consolecheatparm, "", ""))
                    {
                        M_StringCopy(consolecheat, cmd, 255);
                        return true;
                    }
                }
            }
            else if ((M_StringCompare(input, consolecmds[i].name)
                || M_StringCompare(input, consolecmds[i].alternate))
                && consolecmds[i].func1(consolecmds[i].name, "", "", ""))
            {
                M_StringCopy(consolecheat, input, 255);
                return true;
            }
        }
        else
        {
            char        parm1[256] = "";
            char        parm2[256] = "";
            char        parm3[256] = "";

            sscanf(input, "%255s %255s %255s %255s", cmd, parm1, parm2, parm3);
            C_StripQuotes(parm1);
            C_StripQuotes(parm2);
            C_StripQuotes(parm3);
            if ((M_StringCompare(cmd, consolecmds[i].name)
                || M_StringCompare(cmd, consolecmds[i].alternate))
                && consolecmds[i].func1(consolecmds[i].name, parm1, parm2, parm3))
            {
                C_Input((input[strlen(input) - 1] == '%' ? "%s%" : "%s"), input);
                consolecmds[i].func2(consolecmds[i].name, parm1, parm2, parm3);
                return true;
            }
        }
        ++i;
    }
    return false;
}

dboolean C_Responder(event_t *ev)
{
    if (consoleheight < CONSOLEHEIGHT && consoledirection == -1)
        return false;

    if (ev->type == ev_keydown)
    {
        int             key = ev->data1;
        char            ch = (char)ev->data2;
        int             i;
        SDL_Keymod      modstate = SDL_GetModState();

        switch (key)
        {
            case KEY_BACKSPACE:
                if (selectstart < selectend)
                {
                    // delete selected text
                    C_AddToUndoHistory();
                    for (i = selectend; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[selectstart + i - selectend] = consoleinput[i];
                    consoleinput[selectstart + i - selectend] = '\0';
                    caretpos = selectend = selectstart;
                    caretwait = I_GetTimeMS() + caretblinktime;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }
                else if (caretpos > 0)
                {
                    // delete character left of caret
                    C_AddToUndoHistory();
                    for (i = caretpos - 1; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[i] = consoleinput[i + 1];
                    selectend = selectstart = --caretpos;
                    caretwait = I_GetTimeMS() + caretblinktime;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }
                break;

            case KEY_DEL:
                if (selectstart < selectend)
                {
                    // delete selected text
                    C_AddToUndoHistory();
                    for (i = selectend; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[selectstart + i - selectend] = consoleinput[i];
                    consoleinput[selectstart + i - selectend] = '\0';
                    caretpos = selectend = selectstart;
                    caretwait = I_GetTimeMS() + caretblinktime;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }
                else if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    // delete character right of caret
                    C_AddToUndoHistory();
                    for (i = caretpos; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[i] = consoleinput[i + 1];
                    caretwait = I_GetTimeMS() + caretblinktime;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }
                break;

            // confirm input
            case KEY_ENTER:
                if (consoleinput[0])
                {
                    if (C_ValidateInput(consoleinput))
                    {
                        // clear input
                        consoleinput[0] = '\0';
                        caretpos = selectstart = selectend = 0;
                        caretwait = I_GetTimeMS() + caretblinktime;
                        showcaret = true;
                        undolevels = 0;
                        autocomplete = -1;
                        inputhistory = -1;
                        outputhistory = -1;
                        forceblurredraw = true;
                    }

                    return !consolecheat[0];
                }
                break;

            // move caret left
            case KEY_LEFTARROW:
                if (caretpos > 0)
                {
                    if (modstate & KMOD_SHIFT)
                    {
                        --caretpos;
                        caretwait = I_GetTimeMS() + caretblinktime;
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
                        caretwait = I_GetTimeMS() + caretblinktime;
                        showcaret = true;
                    }
                }
                else if (!(modstate & KMOD_SHIFT))
                    caretpos = selectend = selectstart = 0;
                break;

            // move caret right
            case KEY_RIGHTARROW:
                if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    if (modstate & KMOD_SHIFT)
                    {
                        ++caretpos;
                        caretwait = I_GetTimeMS() + caretblinktime;
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
                        caretwait = I_GetTimeMS() + caretblinktime;
                        showcaret = true;
                    }
                }
                else if (!(modstate & KMOD_SHIFT))
                    caretpos = selectend = selectstart = strlen(consoleinput);
                break;

            // move caret to start
            case KEY_HOME:
                if ((outputhistory != -1 || !caretpos) && outputhistory
                    && consolestrings > CONSOLELINES)
                    outputhistory = 0;
                else if (caretpos > 0)
                {
                    caretpos = selectstart = selectend = 0;
                    caretwait = I_GetTimeMS() + caretblinktime;
                    showcaret = true;
                }
                break;

            // move caret to end
            case KEY_END:
                if (outputhistory != -1 && consolestrings > CONSOLELINES)
                    outputhistory = -1;
                else if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    caretpos = selectstart = selectend = strlen(consoleinput);
                    caretwait = I_GetTimeMS() + caretblinktime;
                    showcaret = true;
                }
                break;

            // autocomplete
            case KEY_TAB:
                if (consoleinput[0])
                {
                    int direction = ((modstate & KMOD_SHIFT) ? -1 : 1);
                    int start = autocomplete;

                    if (autocomplete == -1)
                        M_StringCopy(autocompletetext, consoleinput, sizeof(autocompletetext));

                    while ((direction == -1 && autocomplete > 0)
                        || (direction == 1 && autocomplete < numconsolecmds - 1))
                    {
                        autocomplete += direction;
                        if (M_StringStartsWith(consolecmds[autocomplete].name, autocompletetext)
                            && consolecmds[autocomplete].type != CT_CHEAT
                            && consolecmds[autocomplete].description[0])
                        {
                            M_StringCopy(consoleinput, consolecmds[autocomplete].name,
                                sizeof(consoleinput));
                            if (consolecmds[autocomplete].parameters)
                            {
                                int     length = strlen(consoleinput);

                                consoleinput[length] = ' ';
                                consoleinput[length + 1] = '\0';
                            }
                            caretpos = selectstart = selectend = strlen(consoleinput);
                            caretwait = I_GetTimeMS() + caretblinktime;
                            showcaret = true;
                            return true;
                        }
                    }
                    autocomplete = start;
                }
                break;

            // previous input
            case KEY_UPARROW:
                if (inputhistory == -1)
                    M_StringCopy(currentinput, consoleinput, sizeof(currentinput));
                for (i = (inputhistory == -1 ? consolestrings : inputhistory) - 1; i >= 0; --i)
                    if (console[i].type == inputstring
                        && !M_StringCompare(consoleinput, console[i].string))
                    {
                        inputhistory = i;
                        M_StringCopy(consoleinput, console[i].string, 255);
                        caretpos = selectstart = selectend = strlen(consoleinput);
                        caretwait = I_GetTimeMS() + caretblinktime;
                        showcaret = true;
                        break;
                    }
                break;

            // next input
            case KEY_DOWNARROW:
                if (inputhistory != -1)
                {
                    for (i = inputhistory + 1; i < consolestrings; ++i)
                        if (console[i].type == inputstring
                            && !M_StringCompare(consoleinput, console[i].string))
                        {
                            inputhistory = i;
                            M_StringCopy(consoleinput, console[i].string, 255);
                            break;
                        }
                    if (i == consolestrings)
                    {
                        inputhistory = -1;
                        M_StringCopy(consoleinput, currentinput, sizeof(consoleinput));
                    }
                    caretpos = selectstart = selectend = strlen(consoleinput);
                    caretwait = I_GetTimeMS() + caretblinktime;
                    showcaret = true;
                }
                break;

            // scroll output up
            case KEY_PGUP:
                if (consolestrings > CONSOLELINES)
                    outputhistory = (outputhistory == -1 ? consolestrings - (CONSOLELINES + 1)
                        : MAX(0, outputhistory - 1));
                break;

            // scroll output down
            case KEY_PGDN:
                if (outputhistory != -1)
                {
                    ++outputhistory;
                    if (outputhistory + CONSOLELINES == consolestrings)
                        outputhistory = -1;
                }
                break;

            // close console
            case KEY_ESCAPE:
            case KEY_TILDE:
                consoledirection = -1;
                break;

            // change gamma correction level
            case KEY_F11:
                M_ChangeGamma(modstate & KMOD_SHIFT);
                break;

            // toggle "always run"
            case KEY_CAPSLOCK:
                if (key_alwaysrun == KEY_CAPSLOCK)
                    G_ToggleAlwaysRun(ev_keydown);
                break;

            default:
                if (modstate & KMOD_CTRL)
                {
                    // select all text
                    if (ch == 'a')
                    {
                        selectstart = 0;
                        selectend = caretpos = strlen(consoleinput);
                    }

                    // copy selected text to clipboard
                    else if (ch == 'c')
                    {
                        if (selectstart < selectend)
                            SDL_SetClipboardText(M_SubString(consoleinput, selectstart,
                                selectend - selectstart));
                    }

                    // paste text from clipboard
                    else if (ch == 'v')
                    {
                        char    buffer[255];

                        M_snprintf(buffer, sizeof(buffer), "%s%s%s", M_SubString(consoleinput, 0,
                            selectstart), SDL_GetClipboardText(), M_SubString(consoleinput,
                            selectend, strlen(consoleinput) - selectend));
                        if (C_TextWidth(buffer) <= CONSOLEINPUTPIXELWIDTH)
                        {
                            C_AddToUndoHistory();
                            M_StringCopy(consoleinput, buffer, sizeof(consoleinput));
                            selectstart += strlen(SDL_GetClipboardText());
                            selectend = caretpos = selectstart;
                        }
                    }

                    // cut selected text to clipboard
                    else if (ch == 'x')
                    {
                        if (selectstart < selectend)
                        {
                            C_AddToUndoHistory();
                            SDL_SetClipboardText(M_SubString(consoleinput, selectstart,
                                selectend - selectstart));
                            for (i = selectend; (unsigned int)i < strlen(consoleinput); ++i)
                                consoleinput[selectstart + i - selectend] = consoleinput[i];
                            consoleinput[selectstart + i - selectend] = '\0';
                            caretpos = selectend = selectstart;
                            caretwait = I_GetTimeMS() + caretblinktime;
                            showcaret = true;
                        }
                    }

                    // undo
                    else if (ch == 'z')
                        if (undolevels)
                        {
                            --undolevels;
                            M_StringCopy(consoleinput, undohistory[undolevels].input,
                                sizeof(consoleinput));
                            caretpos = undohistory[undolevels].caretpos;
                            selectstart = undohistory[undolevels].selectstart;
                            selectend = undohistory[undolevels].selectend;
                        }
                }
                else
                {
                    if ((modstate & KMOD_SHIFT)
                        || (key_alwaysrun != KEY_CAPSLOCK && (modstate & KMOD_CAPS)))
                        ch = shiftxform[ch];
                    if (ch >= ' ' && ch < '~' && ch != '`'
                        && C_TextWidth(consoleinput) + (ch == ' ' ? spacewidth :
                        consolefont[ch - CONSOLEFONTSTART]->width) <= CONSOLEINPUTPIXELWIDTH
                        && !(modstate & KMOD_ALT))
                    {
                        C_AddToUndoHistory();
                        if (selectstart < selectend)
                        {
                            // replace selected text with a character
                            consoleinput[selectstart] = ch;
                            for (i = selectend; (unsigned int)i < strlen(consoleinput); ++i)
                                consoleinput[selectstart + i - selectend + 1] = consoleinput[i];
                            consoleinput[selectstart + i - selectend + 1] = '\0';
                            caretpos = selectend = selectstart + 1;
                            caretwait = I_GetTimeMS() + caretblinktime;
                            showcaret = true;
                        }
                        else
                        {
                            // insert a character
                            if (strlen(consoleinput) < 255)
                                consoleinput[strlen(consoleinput) + 1] = '\0';
                            for (i = strlen(consoleinput); i > caretpos; --i)
                                consoleinput[i] = consoleinput[i - 1];
                            consoleinput[caretpos++] = ch;
                        }
                        selectstart = selectend = caretpos;
                        caretwait = I_GetTimeMS() + caretblinktime;
                        showcaret = true;
                        autocomplete = -1;
                        inputhistory = -1;
                    }
                }
        }
    }
    else if (ev->type == ev_keyup)
        return false;
    else if (ev->type == ev_mousewheel)
    {
        // scroll output up
        if (ev->data1 > 0)
        {
            if (consolestrings > 10)
                outputhistory = (outputhistory == -1 ? consolestrings - 11 :
                    MAX(0, outputhistory - 1));
        }

        // scroll output down
        else if (ev->data1 < 0)
        {
            if (outputhistory != -1)
            {
                ++outputhistory;
                if (outputhistory + 10 == consolestrings)
                    outputhistory = -1;
            }
        }
    }
    return true;
}

static int dayofweek(int day, int month, int year)
{
    int adjustment = (14 - month) / 12;
    int m = month + 12 * adjustment - 2;
    int y = year - adjustment;

    return ((day + (13 * m - 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7);
}

void C_PrintCompileDate(void)
{
    int                 day, month, year, hour, minute;
    static const char   *days[] =
    {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };
    static const char   mths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    static const char   *months[] =
    {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    static char         mth[4] = "";

    sscanf(__DATE__, "%3s %2d %4d", mth, &day, &year);
    sscanf(__TIME__, "%2d:%2d:%*d", &hour, &minute);
    month = (strstr(mths, mth) - mths) / 3;

    C_Output("This %i-bit %s binary of ~"PACKAGE_NAMEANDVERSIONSTRING"~ was built on %s, %s %i, "
        "%i at %i:%02i%s.", (sizeof(intptr_t) == 4 ? 32 : 64), SDL_GetPlatform(),
        days[dayofweek(day, month + 1, year)], months[month], day, year,
        (hour > 12 ? hour - 12 : hour), minute, (hour < 12 ? "am" : "pm"));
}

void C_PrintSDLVersions(void)
{
    int revision = SDL_GetRevisionNumber();

    if (revision)
        C_Output("Using version %i.%i.%i (Revision %i) of SDL2.DLL.",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, revision);
    else
        C_Output("Using version %i.%i.%i of SDL2.DLL.",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

    C_Output("Using version %i.%i.%i of SDL2_MIXER.DLL.",
        SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL);
}
