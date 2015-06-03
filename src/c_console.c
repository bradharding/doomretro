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

#include <ctype.h>

#if defined(WIN32)
#include <windows.h>
#include <psapi.h>
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

#if defined(WIN32)
#pragma comment(lib, "psapi.lib")
#endif

#define CONSOLESPEED            (CONSOLEHEIGHT / 12)

#define CONSOLEFONTSTART        ' '
#define CONSOLEFONTEND          '~'
#define CONSOLEFONTSIZE         (CONSOLEFONTEND - CONSOLEFONTSTART + 1)

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

#define SPACEWIDTH              3
#define DIVIDER                 "~~~"
#define ITALICS                 '~'

#define CARETWAIT               10

#define NOBACKGROUNDCOLOR       -1

boolean         consoleactive = false;
int             consoleheight = 0;
int             consoledirection = -1;
static int      consolewait = 0;

patch_t         *unknownchar;
patch_t         *consolefont[CONSOLEFONTSIZE];
patch_t         *lsquote;
patch_t         *ldquote;
patch_t         *degree;
patch_t         *multiply;

char            consoleinput[255] = "";
int             consolestrings = 0;

patch_t         *caret;
int             caretpos = 0;
static boolean  showcaret = true;
static int      caretwait = 0;
int             selectstart = 0;
int             selectend = 0;

char            consolecheat[255] = "";
char            consolecheatparm[3] = "";

static int      autocomplete = -1;
static char     autocompletetext[255] = "";

static int      inputhistory = -1;

static int      outputhistory = -1;

static int      notabs[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

#if defined(WIN32)
boolean         showmemory = false;
#endif

extern boolean  translucency;
extern byte     *tinttab75;
extern int      fps;
boolean         alwaysrun;

void G_ToggleAlwaysRun(void);

char *upper =
{
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0 !\"#$%&\"()*+,_>?)!@#$%^&*(:"
    ":<+>?\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0{\\}^_`ABCDEFGHIJKLMNOPQRSTUVWXYZ"
};

byte            *c_tempscreen;
byte            *c_blurredscreen;

int             consolecaretcolor = 4;
int             consolehighfpscolor = 116;
int             consoleinputcolor = 4;
int             consoleselectedinputcolor = 4;
int             consoleselectedinputbackgroundcolor = 161;
int             consoleinputtooutputcolor = 4;
int             consolelowfpscolor = 180;
int             consoletitlecolor = 88;
int             consolememorycolor = 88;
int             consoleplayermessagecolor = 161;
int             consoleoutputcolor = 88;
int             consolebrandingcolor = 100;
int             consolewarningcolor = 180;
int             consoledividercolor = 100;
int             consoletintcolor = 5;
int             consolescrollbartrackcolor = 100;
int             consolescrollbarfacecolor = 88;

int             consolecolors[STRINGTYPES];

void C_Print(stringtype_t type, char *string, ...)
{
    va_list     argptr;
    char        buffer[1024];

    va_start(argptr, string);
    memset(buffer, 0, sizeof(buffer));
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    console = realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(buffer);
    console[consolestrings].type = type;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    ++consolestrings;
    outputhistory = -1;
}

void C_Input(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024];

    va_start(argptr, string);
    memset(buffer, 0, sizeof(buffer));
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    console = realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(buffer);
    console[consolestrings].type = input;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    ++consolestrings;
    outputhistory = -1;
}

void C_Output(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024];

    va_start(argptr, string);
    memset(buffer, 0, sizeof(buffer));
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    console = realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(buffer);
    console[consolestrings].type = output;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    ++consolestrings;
    outputhistory = -1;
}

void C_TabbedOutput(int tabs[8], char *string, ...)
{
    va_list     argptr;
    char        buffer[1024];

    va_start(argptr, string);
    memset(buffer, 0, sizeof(buffer));
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    console = realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(buffer);
    console[consolestrings].type = output;
    memcpy(console[consolestrings].tabs, tabs, sizeof(console[consolestrings].tabs));
    ++consolestrings;
    outputhistory = -1;
}

void C_Warning(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024];

    va_start(argptr, string);
    memset(buffer, 0, sizeof(buffer));
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    if (consolestrings && strcasecmp(console[consolestrings - 1].string, buffer))
    {
        console = realloc(console, (consolestrings + 1) * sizeof(*console));
        console[consolestrings].string = strdup(buffer);
        console[consolestrings].type = warning;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        ++consolestrings;
        outputhistory = -1;
    }
}

void C_PlayerMessage(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024];

    va_start(argptr, string);
    memset(buffer, 0, sizeof(buffer));
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    if (consolestrings && !strcasecmp(console[consolestrings - 1].string, buffer))
    {
        M_snprintf(buffer, sizeof(buffer), "%s (2)", console[consolestrings - 1].string);
        console[consolestrings - 1].string = strdup(buffer);
    }
    else if (consolestrings && M_StringStartsWith(console[consolestrings - 1].string, buffer))
    {
        char    *count = strrchr(console[consolestrings - 1].string, '(') + 1;

        count[strlen(count) - 1] = 0;

        M_snprintf(buffer, sizeof(buffer), "%s (%i)", buffer, atoi(count) + 1);
        console[consolestrings - 1].string = strdup(buffer);
    }
    else
    {
        console = realloc(console, (consolestrings + 1) * sizeof(*console));
        console[consolestrings].string = strdup(buffer);
        console[consolestrings].type = playermessage;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        ++consolestrings;
    }
    outputhistory = -1;
}

void C_AddConsoleDivider(void)
{
    if (!consolestrings || strcasecmp(console[consolestrings - 1].string, DIVIDER))
        C_Print(divider, DIVIDER);
}

static void C_DrawDivider(int y)
{
    int i;

    y *= SCREENWIDTH;
    if (y >= CONSOLETOP * SCREENWIDTH)
        for (i = y + CONSOLETEXTX; i < y + CONSOLETEXTX + CONSOLEDIVIDERWIDTH; ++i)
            screens[0][i] = consoledividercolor;
    if ((y += SCREENWIDTH) >= CONSOLETOP * SCREENWIDTH)
        for (i = y + CONSOLETEXTX; i < y + CONSOLETEXTX + CONSOLEDIVIDERWIDTH; ++i)
            screens[0][i] = consoledividercolor;
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
                screens[0][y - offset + x] = consolescrollbartrackcolor;

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

    caret = consolefont['|' - CONSOLEFONTSTART];

    if (BTSXE1)
        consoleplayermessagecolor = consoleselectedinputbackgroundcolor = 196;
    else if (BTSXE2)
        consoleplayermessagecolor = consoleselectedinputbackgroundcolor = 214;
    else if (chex)
        consoleplayermessagecolor = consoleselectedinputbackgroundcolor = 114;
    else if (hacx)
        consoleplayermessagecolor = consoleselectedinputbackgroundcolor = 198;

    consolecolors[input] = consoleinputtooutputcolor;
    consolecolors[output] = consoleoutputcolor;
    consolecolors[divider] = consoledividercolor;
    consolecolors[title] = consoletitlecolor;
    consolecolors[warning] = consolewarningcolor;
    consolecolors[playermessage] = consoleplayermessagecolor;

    c_tempscreen = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    c_blurredscreen = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
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

static void C_StripQuotes(char *string)
{
    size_t len = strlen(string);

    if (len >= 2 && ((string[0] == '\"' && string[len - 1] == '\"')
        || (string[0] == '\'' && string[len - 1] == '\'')))
    {
        len -= 2;
        memmove(string, string + 1, len);
        string[len] = '\0';
    }
}

static void c_blurscreen(int x1, int y1, int x2, int y2, int i)
{
    int x, y;

    memcpy(c_tempscreen, c_blurredscreen, SCREENWIDTH * (CONSOLEHEIGHT + 5));

    for (y = y1; y < y2; y += SCREENWIDTH)
        for (x = y + x1; x < y + x2; ++x)
            c_blurredscreen[x] = tinttab50[c_tempscreen[x] + (c_tempscreen[x + i] << 8)];
}

static void C_DrawBackground(int height)
{
    static boolean      blurred = false;
    int                 i;

    height = (height + 5) * SCREENWIDTH;

    if (!blurred)
    {
        for (i = 0; i < height; ++i)
            c_blurredscreen[i] = screens[0][i];

        c_blurscreen(0, 0, SCREENWIDTH - 1, height, 1);
        c_blurscreen(1, 0, SCREENWIDTH, height, -1);
        c_blurscreen(0, 0, SCREENWIDTH - 1, height - SCREENWIDTH, SCREENWIDTH + 1);
        c_blurscreen(1, SCREENWIDTH, SCREENWIDTH, height, -(SCREENWIDTH + 1));
        c_blurscreen(0, 0, SCREENWIDTH, height - SCREENWIDTH, SCREENWIDTH);
        c_blurscreen(0, SCREENWIDTH, SCREENWIDTH, height, -SCREENWIDTH);
        c_blurscreen(1, 0, SCREENWIDTH, height - SCREENWIDTH, SCREENWIDTH - 1);
        c_blurscreen(0, SCREENWIDTH, SCREENWIDTH - 1, height, -(SCREENWIDTH - 1));
    }

    blurred = (consoleheight == CONSOLEHEIGHT && !wipe);

    for (i = 0; i < height; ++i)
        screens[0][i] = tinttab50[c_blurredscreen[i] + (consoletintcolor << 8)];

    for (i = height - SCREENWIDTH * 3; i < height - SCREENWIDTH * 2; ++i)
        screens[0][i] = tinttab25[((consolebrandingcolor + 5) << 8) + screens[0][i]];

    for (i = height - SCREENWIDTH * 2; i < height; ++i)
        screens[0][i] = tinttab25[(consolebrandingcolor << 8) + screens[0][i]];
}

static struct
{
    char        char1;
    char        char2;
    int         adjust;
} kern[] = {
    { '\"', '+',  -1 }, { '\"', '.',  -1 }, { '\"', 'a',  -1 }, { '\"', 'c',  -1 },
    { '\"', 'd',  -1 }, { '\"', 'e',  -1 }, { '\"', 'g',  -1 }, { '\"', 'j',  -2 },
    { '\"', 'o',  -1 }, { '\"', 'q',  -1 }, { '\"', 's',  -1 }, { '\'', 'a',  -1 },
    { '\'', 'c',  -1 }, { '\'', 'd',  -1 }, { '\'', 'e',  -1 }, { '\'', 'g',  -1 },
    { '\'', 'j',  -2 }, { '\'', 'o',  -1 }, { '\"', 'q',  -1 }, { '\'', 's',  -1 },
    { '.',  '\\', -1 }, { '.',  '7',  -1 }, { '/',  'o',  -1 }, { ':', '\\',  -1 },
    { '_',  'f',  -1 }, { '0',  ',',  -1 }, { '0',  'j',  -2 }, { '1',  '\"', -1 },
    { '1',  '\'', -1 }, { '1',  'j',  -2 }, { '2',  'j',  -2 }, { '3',  ',',  -1 },
    { '3',  'j',  -2 }, { '4',  'j',  -2 }, { '5',  ',',  -1 }, { '5',  'j',  -2 },
    { '6',  ',',  -1 }, { '6',  'j',  -2 }, { '7',  ',',  -2 }, { '7',  'j',  -2 },
    { '8',  ',',  -1 }, { '8',  'j',  -2 }, { '9',  ',',  -1 }, { '9',  'j',  -2 },
    { 'F',  '.',  -1 }, { 'F',  ',',  -1 }, { 'L',  '\\', -1 }, { 'L',  '\"', -1 },
    { 'L',  '\'', -1 }, { 'P',  '.',  -1 }, { 'P',  ',',  -1 }, { 'T',  '.',  -1 },
    { 'T',  ',',  -1 }, { 'V',  '.',  -1 }, { 'V',  ',',  -1 }, { 'Y',  '.',  -1 },
    { 'Y',  ',',  -1 }, { 'a',  '\"', -1 }, { 'a',  '\'', -1 }, { 'a',  'j',  -2 },
    { 'b',  ',',  -1 }, { 'b',  '\"', -1 }, { 'b',  '\\', -1 }, { 'b',  '\'', -1 },
    { 'b',  'j',  -2 }, { 'c',  '\\', -1 }, { 'c',  ',',  -1 }, { 'c',  '\"', -1 },
    { 'c',  '\'', -1 }, { 'c',  'j',  -2 }, { 'd',  'j',  -2 }, { 'e',  '\\', -1 },
    { 'e',  ',',  -1 }, { 'e',  '\"', -1 }, { 'e',  '\'', -1 }, { 'e',  '_',  -1 },
    { 'e',  'j',  -2 }, { 'f',  ',',  -2 }, { 'f',  '_',  -1 }, { 'f',  'j',  -2 },
    { 'h',  '\\', -1 }, { 'h',  '\"', -1 }, { 'h',  '\'', -1 }, { 'h',  'j',  -2 },
    { 'i',  'j',  -2 }, { 'k',  'j',  -2 }, { 'l',  'j',  -2 }, { 'm',  '\"', -1 },
    { 'm',  '\\', -1 }, { 'm',  '\'', -1 }, { 'm',  'j',  -2 }, { 'n',  '\\', -1 },
    { 'n',  '\"', -1 }, { 'n',  '\'', -1 }, { 'n',  'j',  -2 }, { 'o',  '\\', -1 },
    { 'o',  ',',  -1 }, { 'o',  '\"', -1 }, { 'o',  '\'', -1 }, { 'o',  'j',  -2 },
    { 'p',  '\\', -1 }, { 'p',  ',',  -1 }, { 'p',  '\"', -1 }, { 'p',  '\'', -1 },
    { 'p',  'j',  -2 }, { 'r',  ' ',  -1 }, { 'r',  '\\', -1 }, { 'r',  '.',  -2 },
    { 'r',  ',',  -2 }, { 'r',  '\"', -1 }, { 'r',  '\'', -1 }, { 'r',  '_',  -1 },
    { 'r',  'a',  -1 }, { 'r',  'j',  -2 }, { 's',  ',',  -1 }, { 's',  'j',  -2 },
    { 't',  'j',  -2 }, { 'u',  'j',  -2 }, { 'v',  ',',  -1 }, { 'v',  'j',  -2 },
    { 'w',  'j',  -2 }, { 'x',  'j',  -2 }, { 'z',  'j',  -2 }, {  0 ,   0 ,   0 }
};

static int C_TextWidth(char *text)
{
    size_t      i;
    char        prevletter = '\0';
    int         w = 0;

    for (i = 0; i < strlen(text); ++i)
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

static void C_DrawConsoleText(int x, int y, char *text, int color1, int color2, int translucency,
    int tabs[8])
{
    boolean     italics = false;
    size_t      i;
    int         tab = -1;
    size_t      len = strlen(text);
    char        prevletter = '\0';

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
        char    letter = text[i];
        int     c = letter - CONSOLEFONTSTART;
        char    nextletter = text[i + 1];

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
                x = (x > tabs[++tab] ? x + SPACEWIDTH : tabs[tab]);
            else if (letter == '\xc2' && nextletter == '\xb0')
            {
                patch = degree;
                ++i;
            }
            else
                patch = (c < 0 || c >= CONSOLEFONTSIZE ? unknownchar : consolefont[c]);

            if (isdigit(prevletter) && letter == 'x' && isdigit(nextletter))
                patch = multiply;
            else if (prevletter == ' ' || prevletter == '\t' || !i)
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
                V_DrawConsoleChar(x, y - (CONSOLEHEIGHT - consoleheight), patch, color1, color2,
                    italics, translucency);
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
    char        prevletter = '\0';

    for (i = 0; i < len; ++i)
    {
        char    letter = text[i];
        patch_t *patch = NULL;
        int     k = 0;

        if (letter == ' ')
            x += SPACEWIDTH;
        else
            patch = consolefont[letter - CONSOLEFONTSTART];

        while (kern[k].char1)
        {
            if (prevletter == kern[k].char1 && letter == kern[k].char2)
            {
                x += kern[k].adjust;
                break;
            }
            ++k;
        }

        if (patch)
        {
            V_DrawConsoleChar(x, y, patch, color, NOBACKGROUNDCOLOR, false, 2);
            x += SHORT(patch->width);
        }
        prevletter = letter;
    }
}

void C_Drawer(void)
{
    if (consoleheight)
    {
        int     i;
        int     x = CONSOLETEXTX;
        int     start;
        int     end;
        char    *left = Z_Malloc(512, PU_STATIC, NULL);
        char    *middle = Z_Malloc(512, PU_STATIC, NULL);
        char    *right = Z_Malloc(512, PU_STATIC, NULL);
        boolean prevconsoleactive = consoleactive;

        if (consolewait < I_GetTime())
        {
            consoleheight = BETWEEN(0, consoleheight + CONSOLESPEED * consoledirection,
                CONSOLEHEIGHT);
            consolewait = I_GetTime();
        }

        consoleactive = (consoledirection == 1);

        if (!prevconsoleactive && gamepadvibrate && vibrate)
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

        // draw tiled background and bottom edge
        C_DrawBackground(consoleheight);

        // draw branding
        C_DrawConsoleText(SCREENWIDTH - C_TextWidth(PACKAGE_BRANDINGSTRING) - CONSOLETEXTX + 1,
            CONSOLEHEIGHT - 15, PACKAGE_BRANDINGSTRING, consolebrandingcolor, NOBACKGROUNDCOLOR,
            1, notabs);

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

            if (console[i].type == divider)
                C_DrawDivider(y + 5 - (CONSOLEHEIGHT - consoleheight));
            else
                C_DrawConsoleText(CONSOLETEXTX, y, console[i].string,
                    consolecolors[console[i].type], NOBACKGROUNDCOLOR, 0, console[i].tabs);
        }

        // draw input text to left of caret
        for (i = 0; i < MIN(selectstart, caretpos); ++i)
            left[i] = consoleinput[i];
        left[i] = 0;
        C_DrawConsoleText(x, CONSOLEHEIGHT - 15, left, consoleinputcolor, NOBACKGROUNDCOLOR, 0,
            notabs);
        x += C_TextWidth(left);

        // draw any selected text to left of caret
        if (selectstart < caretpos)
        {
            for (i = selectstart; i < selectend; ++i)
                middle[i - selectstart] = consoleinput[i];
            middle[i - selectstart] = 0;
            if (middle[0])
            {
                C_DrawConsoleText(x, CONSOLEHEIGHT - 15, middle, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, 0, notabs);
                x += C_TextWidth(middle);
            }
        }

        // draw caret
        if (caretwait < I_GetTime())
        {
            showcaret = !showcaret;
            caretwait = I_GetTime() + CARETWAIT;
        }
        if (showcaret)
            V_DrawConsoleChar(x, consoleheight - 15, caret, consolecaretcolor, NOBACKGROUNDCOLOR,
                false, 0);
        x += 3;

        // draw any selected text to right of caret
        if (selectend > caretpos)
        {
            for (i = selectstart; i < selectend; ++i)
                middle[i - selectstart] = consoleinput[i];
            middle[i - selectstart] = 0;
            if (middle[0])
            {
                C_DrawConsoleText(x, CONSOLEHEIGHT - 15, middle, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, 0, notabs);
                x += C_TextWidth(middle);
            }
        }

        // draw input text to right of caret
        if ((unsigned int)caretpos < strlen(consoleinput))
        {
            for (i = selectend; (unsigned int)i < strlen(consoleinput); ++i)
                right[i - selectend] = consoleinput[i];
            right[i - selectend] = 0;
            if (right[0])
                C_DrawConsoleText(x, CONSOLEHEIGHT - 15, right, consoleinputcolor,
                    NOBACKGROUNDCOLOR, 0, notabs);
        }

        Z_Free(left);
        Z_Free(middle);
        Z_Free(right);

        // draw the scrollbar
        C_DrawScrollbar();
    }
    else
        consoleactive = false;

    if (!wipe)
    {
        if (showfps && fps)
        {
            static char     buffer[16];
            byte            color = (fps < TICRATE ? consolelowfpscolor : consolehighfpscolor);
            static int      prevfps = 0;

            M_snprintf(buffer, 16, "%i FPS", fps);

            C_DrawOverlayText(SCREENWIDTH - C_TextWidth(buffer) - CONSOLETEXTX + 2, CONSOLETEXTY,
                buffer, color);

            if (fps != prevfps)
                blurred = false;

            prevfps = fps;
        }

#if defined(WIN32)
        if (showmemory)
        {
            HANDLE                  hProcess = GetCurrentProcess();
            PROCESS_MEMORY_COUNTERS pmc;

            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
            {
                static char buffer[16];

                M_snprintf(buffer, 16, "%s KB", commify(pmc.WorkingSetSize / 1024));

                C_DrawOverlayText(SCREENWIDTH - C_TextWidth(buffer) - CONSOLETEXTX + 2,
                    CONSOLETEXTY + (showfps && fps ? CONSOLELINEHEIGHT : 0), buffer,
                    consolememorycolor);

                blurred = false;
            }

            CloseHandle(hProcess);
        }
#endif
    }
}

boolean C_Responder(event_t *ev)
{
    if (consoleheight < CONSOLEHEIGHT && consoledirection == -1)
        return false;

    if (ev->type == ev_keydown)
    {
        int             key = ev->data1;
        char            ch = (char)ev->data2;
        int             i;

#if defined(SDL20)
        SDL_Keymod      modstate = SDL_GetModState();
#else
        SDLMod          modstate = SDL_GetModState();
#endif

        switch (key)
        {
            case KEY_BACKSPACE:
                if (selectstart < selectend)
                {
                    // delete selected text
                    for (i = selectend; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[selectstart + i - selectend] = consoleinput[i];
                    consoleinput[selectstart + i - selectend] = 0;
                    caretpos = selectend = selectstart;
                    caretwait = I_GetTime() + CARETWAIT;
                    showcaret = true;
                }
                else if (caretpos > 0)
                {
                    // delete character left of caret
                    for (i = caretpos - 1; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[i] = consoleinput[i + 1];
                    --caretpos;
                    caretwait = I_GetTime() + CARETWAIT;
                    showcaret = true;
                }
                break;

            case KEY_DEL:
                if (selectstart < selectend)
                {
                    // delete selected text
                    for (i = selectend; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[selectstart + i - selectend] = consoleinput[i];
                    consoleinput[selectstart + i - selectend] = 0;
                    caretpos = selectend = selectstart;
                    caretwait = I_GetTime() + CARETWAIT;
                    showcaret = true;
                }
                else if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    // delete character right of caret
                    for (i = caretpos; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[i] = consoleinput[i + 1];
                    caretwait = I_GetTime() + CARETWAIT;
                    showcaret = true;
                }
                break;

            // confirm input
            case KEY_ENTER:
                if (consoleinput[0])
                {
                    boolean     validcmd = false;

                    // process cmd
                    i = 0;
                    while (consolecmds[i].name[0])
                    {
                        if (consolecmds[i].parameters == 1)
                        {
                            char        cmd[256] = "";

                            if (consolecmds[i].type == CT_CHEAT)
                            {
                                size_t  length = strlen(consoleinput);

                                if (isdigit(consoleinput[length - 2])
                                    && isdigit(consoleinput[length - 1]))
                                {
                                    consolecheatparm[0] = consoleinput[length - 2];
                                    consolecheatparm[1] = consoleinput[length - 1];
                                    consolecheatparm[2] = 0;

                                    M_StringCopy(cmd, consoleinput, 255);
                                    cmd[length - 2] = 0;

                                    if (!strcasecmp(cmd, consolecmds[i].name)
                                        && length == strlen(cmd) + 2
                                        && consolecmds[i].condition(cmd, consolecheatparm, ""))
                                    {
                                        validcmd = true;
                                        C_Input(consoleinput);
                                        M_StringCopy(consolecheat, cmd, 255);
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                char    parm[256] = "";

                                sscanf(consoleinput, "%255s %255s", cmd, parm);
                                C_StripQuotes(parm);
                                if (!strcasecmp(cmd, consolecmds[i].name)
                                    && consolecmds[i].condition(cmd, parm, ""))
                                {
                                    validcmd = true;
                                    C_Input(consoleinput);
                                    consolecmds[i].function(cmd, parm, "");
                                    break;
                                }
                            }
                        }
                        else if (consolecmds[i].parameters == 2)
                        {
                            char        cmd[256] = "";
                            char        parm1[256] = "";
                            char        parm2[256] = "";

                            sscanf(consoleinput, "%255s %255s %255s", cmd, parm1, parm2);
                            C_StripQuotes(parm1);
                            C_StripQuotes(parm2);
                            if (!strcasecmp(cmd, consolecmds[i].name)
                                && consolecmds[i].condition(cmd, parm1, parm2))
                            {
                                validcmd = true;
                                C_Input(consoleinput);
                                consolecmds[i].function(cmd, parm1, parm2);
                                break;
                            }
                        }
                        else if (!strcasecmp(consoleinput, consolecmds[i].name)
                            && consolecmds[i].condition(consoleinput, "", ""))
                        {
                            validcmd = true;
                            C_Input(consoleinput);
                            if (consolecmds[i].type == CT_CHEAT)
                                M_StringCopy(consolecheat, consoleinput, 255);
                            else
                                consolecmds[i].function(consoleinput, "", "");
                            break;
                        }
                        ++i;
                    }

                    if (validcmd)
                    {
                        // clear input
                        consoleinput[0] = 0;
                        caretpos = 0;
                        caretwait = I_GetTime() + CARETWAIT;
                        showcaret = true;
                    }

                    autocomplete = -1;
                    inputhistory = -1;
                    outputhistory = -1;

                    return !consolecheat[0];
                }
                break;

            // move caret left
            case KEY_LEFTARROW:
                if (caretpos > 0)
                {
                    --caretpos;
                    caretwait = I_GetTime() + CARETWAIT;
                    showcaret = true;
                    if (modstate & KMOD_SHIFT)
                    {
                        if (selectstart <= caretpos)
                            selectend = caretpos;
                        else
                            selectstart = caretpos;
                    }
                    else
                        selectstart = selectend = caretpos;
                }
                break;

            // move caret right
            case KEY_RIGHTARROW:
                if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    ++caretpos;
                    caretwait = I_GetTime() + CARETWAIT;
                    showcaret = true;
                    if (modstate & KMOD_SHIFT)
                    {
                        if (selectend >= caretpos)
                            selectstart = caretpos;
                        else
                            selectend = caretpos;
                    }
                    else
                        selectstart = selectend = caretpos;
                }
                break;

            // move caret to start
            case KEY_HOME:
                if ((outputhistory != -1 || !caretpos) && outputhistory
                    && consolestrings > CONSOLELINES)
                    outputhistory = 0;
                else if (caretpos > 0)
                {
                    caretpos = selectstart = selectend = 0;
                    caretwait = I_GetTime() + CARETWAIT;
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
                    caretwait = I_GetTime() + CARETWAIT;
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
                                consoleinput[length + 1] = 0;
                            }
                            caretpos = selectstart = selectend = strlen(consoleinput);
                            caretwait = I_GetTime() + CARETWAIT;
                            showcaret = true;
                            return true;
                        }
                    }
                    autocomplete = start;
                }
                break;

            // previous input
            case KEY_UPARROW:
                for (i = (inputhistory == -1 ? consolestrings : inputhistory) - 1; i >= 0; --i)
                {
                    if (console[i].type == input && strcasecmp(consoleinput, console[i].string))
                    {
                        inputhistory = i;
                        M_StringCopy(consoleinput, console[i].string, 255);
                        caretpos = selectstart = selectend = strlen(consoleinput);
                        caretwait = I_GetTime() + CARETWAIT;
                        showcaret = true;
                        break;
                    }
                }
                break;

            // next input
            case KEY_DOWNARROW:
                if (inputhistory != -1)
                {
                    for (i = inputhistory + 1; i < consolestrings; ++i)
                        if (console[i].type == input
                            && strcasecmp(consoleinput, console[i].string))
                        {
                            inputhistory = i;
                            M_StringCopy(consoleinput, console[i].string, 255);
                            break;
                        }
                    if (i == consolestrings)
                    {
                        inputhistory = -1;
                        consoleinput[0] = 0;
                    }
                    caretpos = selectstart = selectend = strlen(consoleinput);
                    caretwait = I_GetTime() + CARETWAIT;
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

            case KEY_F11:
                M_ChangeGamma(modstate & KMOD_SHIFT);
                break;

            case KEY_CAPSLOCK:
                G_ToggleAlwaysRun();
                C_Output("%s.", (alwaysrun ? s_ALWAYSRUNON : s_ALWAYSRUNOFF));
                break;

            default:
                if (modstate & KMOD_CTRL)
                {
                    // select all
                    if (ch == 'a')
                    {
                        selectstart = 0;
                        selectend = caretpos = strlen(consoleinput);
                    }
                }
                else
                {
                    if (modstate & KMOD_SHIFT)
                        ch = upper[ch];
                    if (ch >= ' ' && ch < '~' && ch != '`'
                        && C_TextWidth(consoleinput) + (ch == ' ' ? SPACEWIDTH :
                        consolefont[ch - CONSOLEFONTSTART]->width) <= CONSOLEINPUTPIXELWIDTH
                        && !(modstate & KMOD_ALT))
                    {
                        consoleinput[strlen(consoleinput) + 1] = '\0';
                        for (i = strlen(consoleinput); i > caretpos; --i)
                            consoleinput[i] = consoleinput[i - 1];
                        consoleinput[caretpos++] = ch;
                        selectstart = selectend = caretpos;
                        caretwait = I_GetTime() + CARETWAIT;
                        showcaret = true;
                        autocomplete = -1;
                        inputhistory = -1;
                    }
                }
        }
    }
    else if (ev->type == ev_keyup)
        return false;
#if defined(SDL20)
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
#else
    else if (ev->type == ev_mouse)
    {
        // scroll output up
        if (ev->data1 == MOUSE_WHEELUP)
        {
            if (consolestrings > 10)
                outputhistory = (outputhistory == -1 ? consolestrings - 11 :
                    MAX(0, outputhistory - 1));
        }

        // scroll output down
        else if (ev->data1 == MOUSE_WHEELDOWN)
        {
            if (outputhistory != -1)
            {
                ++outputhistory;
                if (outputhistory + 10 == consolestrings)
                    outputhistory = -1;
            }
        }
    }
#endif
    return true;
}

static int dayofweek(int day, int month, int year)
{
    int adjustment = (14 - month) / 12;
    int m = month + 12 * adjustment - 2;
    int y = year - adjustment;

    return (day + (13 * m - 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
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
    static char         mth[4];

    sscanf(__DATE__, "%3s %2d %4d", mth, &day, &year);
    sscanf(__TIME__, "%2d:%2d:%*d", &hour, &minute);
    month = (strstr(mths, mth) - mths) / 3;

    C_Output("");
    C_Output("This %i-bit %s binary of %s was built on %s, %s %i, %i at %i:%02i%s.",
        (sizeof(intptr_t) == 4 ? 32 : 64),
#if defined(WIN32)
        "Windows",
#elif defined(__MACOSX__)
        "OS X",
#else
        "Linux",
#endif
        PACKAGE_NAMEANDVERSIONSTRING, days[dayofweek(day, month + 1, year)], months[month], day,
        year, (hour > 12 ? hour - 12 : hour), minute, (hour < 12 ? "am" : "pm"));
}

void C_PrintSDLVersions(void)
{
    C_Output("Using version %i.%i.%i of %s.",
        SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL,
#if defined(SDL20)
        "SDL2.DLL"
#else
        "SDL.DLL"
#endif
        );

    C_Output("Using version %i.%i.%i of %s.",
        SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL,
#if defined(SDL20)
        "SDL2_MIXER.DLL"
#else
        "SDL_MIXER.DLL"
#endif
        );
}
