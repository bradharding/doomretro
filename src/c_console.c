/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <ctype.h>
#include <time.h>

#include "c_cmds.h"
#include "c_console.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_timer.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define CONSOLESPEED            (CONSOLEHEIGHT / 12)

#define CONSOLETEXTX            10
#define CONSOLETEXTY            8
#define CONSOLETEXTMAXLENGTH    1024
#define CONSOLETEXTPIXELWIDTH   (CONSOLEWIDTH - CONSOLETEXTX * 3 - CONSOLESCROLLBARWIDTH + 3)
#define CONSOLELINES            (gamestate != GS_TITLESCREEN ? 11 : 27)
#define CONSOLELINEHEIGHT       14

#define CONSOLEINPUTPIXELWIDTH  (CONSOLEWIDTH - CONSOLETEXTX - brandwidth - 2)

#define CONSOLESCROLLBARWIDTH   3
#define CONSOLESCROLLBARHEIGHT  ((CONSOLELINES - 1) * CONSOLELINEHEIGHT - 1)
#define CONSOLESCROLLBARX       (CONSOLEWIDTH - CONSOLETEXTX - CONSOLESCROLLBARWIDTH)
#define CONSOLESCROLLBARY       (CONSOLETEXTY + 1)

#define CONSOLEDIVIDERWIDTH     (CONSOLETEXTPIXELWIDTH - CONSOLETEXTX + 1)

#define DIVIDER                 "~~~"

#define CARETBLINKTIME          350

dboolean        consoleactive;
int             consoleheight;
int             consoledirection = -1;
int             consoleanim;

dboolean        forceconsoleblurredraw;

patch_t         *consolefont[CONSOLEFONTSIZE];
patch_t         *degree;

static patch_t  *trademark;
static patch_t  *copyright;
static patch_t  *regomark;
static patch_t  *multiply;
static patch_t  *warning;
static patch_t  *brand;
static patch_t  *divider;
static patch_t  *bindlist;
static patch_t  *cmdlist;
static patch_t  *cvarlist;
static patch_t  *maplist;
static patch_t  *playerstats;

static short    brandwidth;
static short    brandheight;
static short    spacewidth;

static char     consoleinput[255];
int             consolestrings;
int             numconsolecmds;

static int      undolevels;

static patch_t  *caret;
static int      caretpos;
static dboolean showcaret = true;
static int      caretwait;
static int      selectstart;
static int      selectend;

char            consolecheat[255];
char            consolecheatparm[3];

static int      autocomplete = -1;
static char     autocompletetext[255];

static int      inputhistory = -1;
static char     currentinput[255];

static int      outputhistory = -1;

static int      notabs[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

dboolean        con_timestamps = con_timestamps_default;
static int      timestampx;
static int      zerowidth;

static const char *shiftxform =
{
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0 !\"#$%&\"()*+<_>?"
    ")!@#$%^&*(::<+>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}\"_'ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~\0"
};

static byte     c_tempscreen[SCREENWIDTH * SCREENHEIGHT];
static byte     c_blurscreen[SCREENWIDTH * SCREENHEIGHT];

static int      consolecaretcolor = 4;
static int      consolelowfpscolor = 180;
static int      consolehighfpscolor = 116;
static int      consoleinputcolor = 4;
static int      consoleselectedinputcolor = 4;
static int      consoleselectedinputbackgroundcolor = 100;
static int      consoleinputtooutputcolor = 4;
static int      consoletitlecolor = 88;
static int      consolememorycolor = 88;
static int      consoleplayermessagecolor = 161;
static int      consoletimestampcolor = 100;
static int      consoleoutputcolor = 88;
static int      consoleboldcolor = 4;
static int      consoleitalicscolor = 98;
static int      consoleheadercolor = 180;
static int      consolewarningcolor = 180;
static int      consoledividercolor = 100;
static int      consoletintcolor = 5;
static int      consoleedgecolor = 180;
static int      consolescrollbartrackcolor = 100;
static int      consolescrollbarfacecolor = 94;

static int      consolecolors[STRINGTYPES];

extern int      fps;
extern int      refreshrate;
extern dboolean r_althud;
extern dboolean r_translucency;
extern dboolean vanilla;
extern dboolean togglingvanilla;
extern dboolean message_external;
extern dboolean message_dontfuckwithme;

void G_ToggleAlwaysRun(evtype_t type);

void C_Print(stringtype_t type, char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
    M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
    console[consolestrings].type = type;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings++].timestamp[0] = '\0';
    outputhistory = -1;
}

void C_Input(char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH] = "";

    if (togglingvanilla)
        return;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
    M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
    console[consolestrings].type = inputstring;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings++].timestamp[0] = '\0';
    outputhistory = -1;
}

void C_IntCVAROutput(char *cvar, int value)
{
    if (consolestrings && M_StringStartsWith(console[consolestrings - 1].string, cvar))
        consolestrings--;

    C_Input("%s %i", cvar, value);
}

void C_PctCVAROutput(char *cvar, int value)
{
    if (consolestrings && M_StringStartsWith(console[consolestrings - 1].string, cvar))
        consolestrings--;

    C_Input("%s %i%%", cvar, value);
}

void C_StrCVAROutput(char *cvar, char *string)
{
    if (consolestrings && M_StringStartsWith(console[consolestrings - 1].string, cvar))
        consolestrings--;

    C_Input("%s %s", cvar, string);
}

void C_Output(char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
    M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
    console[consolestrings].type = outputstring;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings++].timestamp[0] = '\0';
    outputhistory = -1;
}

void C_TabbedOutput(int tabs[8], char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
    M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
    console[consolestrings].type = outputstring;
    memcpy(console[consolestrings].tabs, tabs, sizeof(console[consolestrings].tabs));
    console[consolestrings++].timestamp[0] = '\0';
    outputhistory = -1;
}

void C_Warning(char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (consolestrings && !M_StringCompare(console[consolestrings - 1].string, buffer))
    {
        console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
        M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
        console[consolestrings].type = warningstring;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        console[consolestrings++].timestamp[0] = '\0';
        outputhistory = -1;
    }
}

void C_PlayerMessage(char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH] = "";
    int         i = consolestrings - 1;
    dboolean    prevplayermessage = (i >= 0 && console[i].type == playermessagestring);
    time_t      rawtime;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    time(&rawtime);

    if (prevplayermessage && M_StringCompare(console[i].string, buffer))
    {
        M_snprintf(console[i].string, CONSOLETEXTMAXLENGTH, "%s (2)", buffer);
        strftime(console[i].timestamp, 9, "%H:%M:%S", localtime(&rawtime));
    }
    else if (prevplayermessage && M_StringStartsWith(console[i].string, buffer))
    {
        char    *count = strrchr(console[i].string, '(') + 1;

        count[strlen(count) - 1] = '\0';
        M_snprintf(console[i].string, CONSOLETEXTMAXLENGTH, "%s (%i)", buffer, atoi(count) + 1);
        strftime(console[i].timestamp, 9, "%H:%M:%S", localtime(&rawtime));
    }
    else
    {
        console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
        M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
        console[consolestrings].type = playermessagestring;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        strftime(console[consolestrings++].timestamp, 9, "%H:%M:%S", localtime(&rawtime));
    }
    outputhistory = -1;
}

void C_Obituary(char *string, ...)
{
    va_list     argptr;
    char        buffer[CONSOLETEXTMAXLENGTH] = "";
    int         i = consolestrings - 1;
    dboolean    prevobituary = (i >= 0 && console[i].type == obituarystring);
    time_t      rawtime;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    time(&rawtime);

    if (prevobituary && M_StringCompare(console[i].string, buffer))
    {
        M_snprintf(console[i].string, CONSOLETEXTMAXLENGTH, "%s (2)", buffer);
        strftime(console[i].timestamp, 9, "%H:%M:%S", localtime(&rawtime));
    }
    else if (prevobituary && M_StringStartsWith(console[i].string, buffer))
    {
        char    *count = strrchr(console[i].string, '(') + 1;

        count[strlen(count) - 1] = '\0';
        M_snprintf(console[i].string, CONSOLETEXTMAXLENGTH, "%s (%i)", buffer, atoi(count) + 1);
        strftime(console[i].timestamp, 9, "%H:%M:%S", localtime(&rawtime));
    }
    else
    {
        console = Z_Realloc(console, (consolestrings + 1) * sizeof(*console));
        M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
        console[consolestrings].type = obituarystring;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        strftime(console[consolestrings++].timestamp, 9, "%H:%M:%S", localtime(&rawtime));
    }
    outputhistory = -1;
}

static void C_AddToUndoHistory(void)
{
    undohistory = Z_Realloc(undohistory, (undolevels + 1) * sizeof(*undohistory));
    undohistory[undolevels].input = strdup(consoleinput);
    undohistory[undolevels].caretpos = caretpos;
    undohistory[undolevels].selectstart = selectstart;
    undohistory[undolevels].selectend = selectend;
    undolevels++;
}

void C_AddConsoleDivider(void)
{
    if (!consolestrings || !M_StringCompare(console[consolestrings - 1].string, DIVIDER))
        C_Print(dividerstring, DIVIDER);
}

static struct
{
    char        char1;
    char        char2;
    int         adjust;
} kern[] = {
    { ' ',  '(',  -1 }, { ' ',  'T',  -1 }, { '\"', '+',  -1 }, { '\"', '.',  -1 },
    { '\"', 'a',  -1 }, { '\"', 'c',  -1 }, { '\"', 'd',  -1 }, { '\"', 'e',  -1 },
    { '\"', 'g',  -1 }, { '\"', 'j',  -2 }, { '\"', 'o',  -1 }, { '\"', 'q',  -1 },
    { '\"', 's',  -1 }, { '\\', '\\', -2 }, { '\\', 'd',  -1 }, { '\\', 'V',  -1 },
    { '\'', 'a',  -1 }, { '\'', 'a',  -1 }, { '\'', 'c',  -1 }, { '\'', 'd',  -1 },
    { '\'', 'e',  -1 }, { '\'', 'g',  -1 }, { '\'', 'j',  -2 }, { '\'', 'o',  -1 },
    { '\'', 's',  -1 }, { '.',  '\\', -1 }, { '.',  '4',  -1 }, { '.',  '7',  -1 },
    { ',',  '4',  -1 }, { '/',  '/',  -2 }, { '/',  'd',  -1 }, { '/',  'o',  -1 },
    { ':', '\\',  -1 }, { '_',  'f',  -1 }, { '0',  ',',  -1 }, { '0',  ';',  -1 },
    { '0',  'j',  -2 }, { '1',  '\"', -1 }, { '1',  '\'', -1 }, { '1',  'j',  -2 },
    { '2',  'j',  -2 }, { '3',  ',',  -1 }, { '3',  ';',  -1 }, { '3',  'j',  -2 },
    { '4',  'j',  -2 }, { '5',  ',',  -1 }, { '5',  ';',  -1 }, { '5',  'j',  -2 },
    { '6',  ',',  -1 }, { '6',  'j',  -2 }, { '7',  '.',  -2 }, { '7',  ',',  -2 },
    { '7',  ';',  -1 }, { '7',  'j',  -2 }, { '8',  ',',  -1 }, { '8',  ';',  -1 },
    { '8',  'j',  -2 }, { '9',  ',',  -1 }, { '9',  ';',  -1 }, { '9',  'j',  -2 },
    { 'F',  '.',  -1 }, { 'F',  ',',  -1 }, { 'F',  ';',  -1 }, { 'L',  '\\', -1 },
    { 'L',  '\"', -1 }, { 'L',  '\'', -1 }, { 'P',  '.',  -1 }, { 'P',  ',',  -1 },
    { 'P',  ';',  -1 }, { 'T',  '.',  -1 }, { 'T',  ',',  -1 }, { 'T',  ';',  -1 },
    { 'T',  'a',  -1 }, { 'T',  'e',  -1 }, { 'T',  'o',  -1 }, { 'V',  '.',  -1 },
    { 'V',  ',',  -1 }, { 'V',  ';',  -1 }, { 'Y',  '.',  -1 }, { 'Y',  ',',  -1 },
    { 'Y',  ';',  -1 }, { 'a',  '\"', -1 }, { 'a',  '\'', -1 }, { 'a',  'j',  -2 },
    { 'b',  ',',  -1 }, { 'b',  ';',  -1 }, { 'b',  '\"', -1 }, { 'b',  '\\', -1 },
    { 'b',  '\'', -1 }, { 'b',  'j',  -2 }, { 'c',  '\\', -1 }, { 'c',  ',',  -1 },
    { 'c',  ';',  -1 }, { 'c',  '\"', -1 }, { 'c',  '\'', -1 }, { 'c',  'j',  -2 },
    { 'd',  'j',  -2 }, { 'e',  '\\', -1 }, { 'e',  ',',  -1 }, { 'e',  ';',  -1 },
    { 'e',  '\"', -1 }, { 'e',  '\'', -1 }, { 'e',  '_',  -1 }, { 'e',  'j',  -2 },
    { 'f',  ' ',  -1 }, { 'f',  ',',  -2 }, { 'f',  ';',  -1 }, { 'f',  '_',  -1 },
    { 'f',  'a',  -1 }, { 'f',  'j',  -2 }, { 'h',  '\\', -1 }, { 'h',  'j',  -2 },
    { 'i',  'j',  -2 }, { 'k',  'j',  -2 }, { 'l',  'j',  -2 }, { 'm',  '\"', -1 },
    { 'm',  '\\', -1 }, { 'm',  '\'', -1 }, { 'm',  'j',  -2 }, { 'n',  '\\', -1 },
    { 'n',  '\"', -1 }, { 'n',  '\'', -1 }, { 'n',  'j',  -2 }, { 'o',  '\\', -1 },
    { 'o',  ',',  -1 }, { 'o',  ';',  -1 }, { 'o',  '\"', -1 }, { 'o',  '\'', -1 },
    { 'o',  'j',  -2 }, { 'p',  '\\', -1 }, { 'p',  ',',  -1 }, { 'p',  ';',  -1 },
    { 'p',  '\"', -1 }, { 'p',  '\'', -1 }, { 'p',  'j',  -2 }, { 'r',  ' ',  -1 },
    { 'r',  '\\', -1 }, { 'r',  '.',  -2 }, { 'r',  ',',  -2 }, { 'r',  ';',  -1 },
    { 'r',  '\"', -1 }, { 'r',  '\'', -1 }, { 'r',  '_',  -1 }, { 'r',  'a',  -1 },
    { 'r',  'j',  -2 }, { 's',  '\\', -1 }, { 's',  ',',  -1 }, { 's',  ';',  -1 },
    { 's',  'j',  -2 }, { 't',  'j',  -2 }, { 'u',  'j',  -2 }, { 'v',  ',',  -1 },
    { 'v',  ';',  -1 }, { 'v',  'j',  -2 }, { 'w',  'j',  -2 }, { 'x',  'j',  -2 },
    { 'z',  'j',  -2 }, {  0 ,   0 ,   0 }
};

static int C_TextWidth(char *text, dboolean formatting)
{
    size_t              i;
    size_t              len = strlen(text);
    unsigned char       prevletter = '\0';
    int                 w = 0;

    for (i = 0; i < len; i++)
    {
        unsigned char   letter = text[i];
        int             c = letter - CONSOLEFONTSTART;
        unsigned char   nextletter = text[i + 1];
        int             j = 0;

        if (letter == '<' && i < len - 2 && (text[i + 1] == 'b' || text[i + 1] == 'i')
            && text[i + 2] == '>' && formatting)
            i += 2;
        else if (letter == '<' && i < len - 3 && text[i + 1] == '/' && (text[i + 2] == 'b'
            || text[i + 2] == 'i') && text[i + 3] == '>' && formatting)
            i += 3;
        else if (letter == 153)
        {
            w += SHORT(trademark->width);
            i++;
        }
        else if (letter == 169)
        {
            w += SHORT(copyright->width);
            i++;
        }
        else if (letter == 174)
        {
            w += SHORT(regomark->width);
            i++;
        }
        else if (letter == 194 && nextletter == 176)
        {
            w += SHORT(degree->width);
            i++;
        }
        else if (letter == 215)
        {
            w += SHORT(multiply->width);
            i++;
        }
        else
            w += SHORT(c < 0 || c >= CONSOLEFONTSIZE ? 0 : consolefont[c]->width);

        while (kern[j].char1)
        {
            if (prevletter == kern[j].char1 && letter == kern[j].char2)
            {
                w += kern[j].adjust;
                break;
            }

            j++;
        }
        prevletter = letter;
    }

    return w;
}

static void C_DrawScrollbar(void)
{

    int trackstart = CONSOLESCROLLBARY * CONSOLEWIDTH;
    int trackend = trackstart + CONSOLESCROLLBARHEIGHT * CONSOLEWIDTH;
    int facestart = (CONSOLESCROLLBARY + CONSOLESCROLLBARHEIGHT * (outputhistory == -1 ?
            MAX(0, consolestrings - CONSOLELINES) : outputhistory) / consolestrings) * CONSOLEWIDTH;
    int faceend = facestart + (CONSOLESCROLLBARHEIGHT - CONSOLESCROLLBARHEIGHT
            * MAX(0, consolestrings - CONSOLELINES) / consolestrings) * CONSOLEWIDTH;

    if (trackstart == facestart && trackend == faceend)
        return;
    else
    {
        int     x, y;
        int     offset = (CONSOLEHEIGHT - consoleheight) * CONSOLEWIDTH;

        // Draw scrollbar track
        for (y = trackstart; y < trackend; y += CONSOLEWIDTH)
            if (y - offset >= 0)
                for (x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                    screens[0][y - offset + x] = tinttab50[screens[0][y - offset + x]
                        + consolescrollbartrackcolor];

        // Draw scrollbar face
        for (y = facestart; y < faceend; y += CONSOLEWIDTH)
            if (y - offset >= 0)
                for (x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                    screens[0][y - offset + x] = consolescrollbarfacecolor;
    }
}

void C_Init(void)
{
    int         i;
    int         j = CONSOLEFONTSTART;
    char        buffer[9];

    while (*consolecmds[numconsolecmds++].name);

    for (i = 0; i < CONSOLEFONTSIZE; i++)
    {
        M_snprintf(buffer, 9, "DRFON%03d", j++);
        consolefont[i] = W_CacheLumpName(buffer, PU_STATIC);
    }

    trademark = W_CacheLumpName("DRFON153", PU_STATIC);
    copyright = W_CacheLumpName("DRFON169", PU_STATIC);
    regomark = W_CacheLumpName("DRFON174", PU_STATIC);
    degree = W_CacheLumpName("DRFON176", PU_STATIC);
    multiply = W_CacheLumpName("DRFON215", PU_STATIC);

    brand = W_CacheLumpName("DRBRAND", PU_STATIC);
    caret = W_CacheLumpName("DRCARET", PU_STATIC);
    divider = W_CacheLumpName("DRDIVIDE", PU_STATIC);
    warning = W_CacheLumpName("DRFONWRN", PU_STATIC);

    bindlist = W_CacheLumpName("DRBNDLST", PU_STATIC);
    cmdlist = W_CacheLumpName("DRCMDLST", PU_STATIC);
    cvarlist = W_CacheLumpName("DRCVRLST", PU_STATIC);
    maplist = W_CacheLumpName("DRMAPLST", PU_STATIC);
    playerstats = W_CacheLumpName("DRPLYRST", PU_STATIC);

    brandwidth = SHORT(brand->width);
    brandheight = SHORT(brand->height);
    spacewidth = SHORT(consolefont[' ' - CONSOLEFONTSTART]->width);
    timestampx = CONSOLEWIDTH - C_TextWidth("00:00:00", false) - CONSOLETEXTX * 2 - CONSOLESCROLLBARWIDTH + 1;
    zerowidth = SHORT(consolefont['0' - CONSOLEFONTSTART]->width);

    consolecaretcolor = nearestcolors[consolecaretcolor];
    consolelowfpscolor = nearestcolors[consolelowfpscolor];
    consolehighfpscolor = nearestcolors[consolehighfpscolor];
    consoleinputcolor = nearestcolors[consoleinputcolor];
    consoleselectedinputcolor = nearestcolors[consoleselectedinputcolor];
    consoleselectedinputbackgroundcolor = nearestcolors[consoleselectedinputbackgroundcolor];
    consoleinputtooutputcolor = nearestcolors[consoleinputtooutputcolor];
    consoletitlecolor = nearestcolors[consoletitlecolor];
    consolememorycolor = nearestcolors[consolememorycolor];
    consoleplayermessagecolor = nearestcolors[consoleplayermessagecolor];
    consoletimestampcolor = nearestcolors[consoletimestampcolor];
    consoleoutputcolor = nearestcolors[consoleoutputcolor];
    consoleboldcolor = nearestcolors[consoleboldcolor];
    consoleitalicscolor = nearestcolors[consoleitalicscolor];
    consoleheadercolor = nearestcolors[consoleheadercolor];
    consolewarningcolor = nearestcolors[consolewarningcolor];
    consoledividercolor = nearestcolors[consoledividercolor];
    consoletintcolor = nearestcolors[consoletintcolor];
    consoleedgecolor = nearestcolors[consoleedgecolor] << 8;
    consolescrollbartrackcolor = nearestcolors[consolescrollbartrackcolor] << 8;
    consolescrollbarfacecolor = nearestcolors[consolescrollbarfacecolor];

    consolecolors[inputstring] = consoleinputtooutputcolor;
    consolecolors[outputstring] = consoleoutputcolor;
    consolecolors[dividerstring] = consoledividercolor;
    consolecolors[titlestring] = consoletitlecolor;
    consolecolors[warningstring] = consolewarningcolor;
    consolecolors[playermessagestring] = consoleplayermessagecolor;
    consolecolors[obituarystring] = consoleplayermessagecolor;
}

void C_ShowConsole(void)
{
    consoleheight = MAX(1, consoleheight);
    consoledirection = 1;
    consoleanim = 0;
    showcaret = true;
    caretwait = 0;
    if (gamestate == GS_TITLESCREEN && !devparm)
        S_StartSound(NULL, sfx_swtchn);
}

void C_HideConsole(void)
{
    consoledirection = -1;
    consoleanim = 0;
    if (gamestate == GS_TITLESCREEN)
    {
        consoleheight = 0;
        consoleactive = false;
        S_StartSound(NULL, sfx_swtchx);
    }
}

void C_HideConsoleFast(void)
{
    consoledirection = -1;
    consoleanim = 0;
    consoleheight = 0;
    consoleactive = false;
}

void C_StripQuotes(char *string)
{
    size_t      len = strlen(string);

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

    memcpy(c_tempscreen, c_blurscreen, CONSOLEWIDTH * (CONSOLEHEIGHT + 5));

    for (y = y1; y < y2; y += CONSOLEWIDTH)
        for (x = y + x1; x < y + x2; x++)
            c_blurscreen[x] = tinttab50[c_tempscreen[x] + (c_tempscreen[x + i] << 8)];
}

static void C_DrawBackground(int height)
{
    static dboolean     blurred;
    int                 i;

    height = (height + 5) * CONSOLEWIDTH;

    if (r_translucency)
    {
        if (!blurred)
        {
            for (i = 0; i < height; i++)
                c_blurscreen[i] = screens[0][i];

            DoBlurScreen(0, 0, CONSOLEWIDTH - 1, height, 1);
            DoBlurScreen(1, 0, CONSOLEWIDTH, height, -1);
            DoBlurScreen(0, 0, CONSOLEWIDTH - 1, height - CONSOLEWIDTH, CONSOLEWIDTH + 1);
            DoBlurScreen(1, CONSOLEWIDTH, CONSOLEWIDTH, height, -(CONSOLEWIDTH + 1));
            DoBlurScreen(0, 0, CONSOLEWIDTH, height - CONSOLEWIDTH, CONSOLEWIDTH);
            DoBlurScreen(0, CONSOLEWIDTH, CONSOLEWIDTH, height, -CONSOLEWIDTH);
            DoBlurScreen(1, 0, CONSOLEWIDTH, height - CONSOLEWIDTH, CONSOLEWIDTH - 1);
            DoBlurScreen(0, CONSOLEWIDTH, CONSOLEWIDTH - 1, height, -(CONSOLEWIDTH - 1));
        }

        blurred = (consoleheight == CONSOLEHEIGHT && !wipe);

        if (forceconsoleblurredraw)
        {
            forceconsoleblurredraw = false;
            blurred = false;
        }

        for (i = 0; i < height; i++)
            screens[0][i] = tinttab50[(consoletintcolor << 8) + c_blurscreen[i]];

        for (i = height - 2; i > 1; i -= 3)
        {
            screens[0][i] = colormaps[0][256 * 6 + screens[0][i]];

            if (((i - 1) % CONSOLEWIDTH) < CONSOLEWIDTH - 2)
                screens[0][i + 1] = colormaps[0][256 * 6 + screens[0][i - 1]];
        }
    }
    else
    {
        for (i = 0; i < height; i++)
            screens[0][i] = consoletintcolor;

        for (i = height - 2; i > 1; i -= 3)
            screens[0][i] = colormaps[0][256 * 6 + screens[0][i]];
    }

    // draw branding
    V_DrawConsolePatch(CONSOLEWIDTH - brandwidth, consoleheight - brandheight + 2, brand);

    // draw bottom edge
    for (i = height - CONSOLEWIDTH * 3; i < height; i++)
        screens[0][i] = tinttab50[consoleedgecolor + screens[0][i]];

    // soften edges
    if (r_translucency)
    {
        for (i = 0; i < height; i += CONSOLEWIDTH)
        {
            screens[0][i] = tinttab50[screens[0][i]];
            screens[0][i + CONSOLEWIDTH - 1] = tinttab50[screens[0][i + CONSOLEWIDTH - 1]];
        }

        for (i = height - CONSOLEWIDTH + 1; i < height - 1; i++)
            screens[0][i] = tinttab25[screens[0][i]];
    }

    // draw shadow
    if (gamestate != GS_TITLESCREEN)
    {
        if (r_translucency)
        {
            int j;

            for (j = CONSOLEWIDTH; j <= 4 * CONSOLEWIDTH; j += CONSOLEWIDTH)
                for (i = height; i < height + j; i++)
                    screens[0][i] = colormaps[0][256 * 4 + screens[0][i]];
        }
        else
            for (i = height; i < height + CONSOLEWIDTH; i++)
                screens[0][i] = 0;
    }
}

static void C_DrawConsoleText(int x, int y, char *text, int color1, int color2, int boldcolor,
    byte *tinttab, int tabs[8], dboolean formatting)
{
    int                 bold = 0;
    dboolean            italics = false;
    size_t              i;
    int                 tab = -1;
    size_t              len = strlen(text);
    unsigned char       prevletter = '\0';
    int                 width = 0;

    y -= CONSOLEHEIGHT - consoleheight;

    if (color1 == consolewarningcolor)
    {
        V_DrawConsoleTextPatch(x, y, warning, color1, color2, false, tinttab);
        width = SHORT(warning->width) + 2;
        x += width;
    }

    if (len > 80)
        while (C_TextWidth(text, formatting) + width > CONSOLETEXTPIXELWIDTH)
        {
            text[len - 1] = '.';
            text[len] = '.';
            text[len + 1] = '.';
            text[len + 2] = '\0';
            len--;
        }

    for (i = 0; i < len; i++)
    {
        unsigned char   letter = text[i];

        if (letter == '<' && text[i + 1] == 'b' && text[i + 2] == '>' && formatting)
        {
            bold = (italics ? 2 : 1);
            i += 2;
        }
        else if (letter == '<' && text[i + 1] == '/' && text[i + 2] == 'b' && text[i + 3] == '>'
            && formatting)
        {
            bold = 0;
            i += 3;
        }
        else if (letter == '<' && text[i + 1] == 'i' && text[i + 2] == '>' && formatting)
        {
            italics = true;
            i += 2;
        }
        else if (letter == '<' && text[i + 1] == '/' && text[i + 2] == 'i' && text[i + 3] == '>'
            && formatting)
        {
            italics = false;
            i += 3;
            x++;
        }
        else
        {
            patch_t             *patch = NULL;
            unsigned char       nextletter = text[i + 1];
            int                 c = letter - CONSOLEFONTSTART;

            if (letter == '\t')
                x = (x > tabs[++tab] ? x + spacewidth : tabs[tab]);
            else if (letter == 153)
                patch = trademark;
            else if (letter == 169)
                patch = copyright;
            else if (letter == 174)
                patch = regomark;
            else if (letter == 194 && nextletter == 176)
            {
                patch = degree;
                i++;
            }
            else if (letter == 215)
                patch = multiply;
            else if (c >= 0 && c < CONSOLEFONTSIZE)
                patch = consolefont[c];
            else
                continue;

            if (!italics)
            {
                int     j = 0;

                while (kern[j].char1)
                {
                    if (prevletter == kern[j].char1 && letter == kern[j].char2)
                    {
                        x += kern[j].adjust;
                        break;
                    }
                    j++;
                }
            }

            if (patch)
            {
                V_DrawConsoleTextPatch(x, y, patch, (bold == 1 ? boldcolor : (bold == 2 ? color1 :
                    (italics ? (color1 == consolewarningcolor ? color1 : consoleitalicscolor) :
                    color1))), color2, italics, tinttab);
                x += SHORT(patch->width);
            }

            prevletter = letter;
        }
    }
}

static void C_DrawOverlayText(int x, int y, char *text, int color)
{
    size_t      i;
    size_t      len = strlen(text);

    for (i = 0; i < len; i++)
    {
        char    letter = text[i];

        if (letter == ' ')
            x += spacewidth;
        else
        {
            patch_t     *patch = consolefont[letter - CONSOLEFONTSTART];

            V_DrawConsoleTextPatch(x, y, patch, color, NOBACKGROUNDCOLOR, false,
                (r_translucency ? tinttab75 : NULL));
            x += SHORT(patch->width);
        }
    }
}

static void C_DrawTimeStamp(int x, int y, char *text)
{
    size_t      i;
    size_t      len = strlen(text);

    y -= (CONSOLEHEIGHT - consoleheight);

    for (i = 0; i < len; i++)
    {
        patch_t *patch = consolefont[text[i] - CONSOLEFONTSTART];
        int     width = SHORT(patch->width);

        V_DrawConsoleTextPatch(x + (text[i] == '1' ? (zerowidth - width) / 2 : 0), y, patch,
            consoletimestampcolor, NOBACKGROUNDCOLOR, false, (r_translucency ? tinttab25 : NULL));
        x += (isdigit(text[i]) ? zerowidth : width);
    }
}

void C_UpdateFPS(void)
{
    if (fps && !wipe && !paused && !menuactive)
    {
        static char     buffer[16];

        M_snprintf(buffer, 16, "%i FPS", fps);

        C_DrawOverlayText(CONSOLEWIDTH - C_TextWidth(buffer, false) - CONSOLETEXTX + 1, CONSOLETEXTY,
            buffer, (fps < (refreshrate && vid_capfps != TICRATE ? refreshrate : TICRATE) ?
            consolelowfpscolor : consolehighfpscolor));
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
        char            *lefttext = malloc(512);
        char            *middletext = malloc(512);
        char            *righttext = malloc(512);
        dboolean        prevconsoleactive = consoleactive;
        static int      consolewait;

        int             consoledown[] =
        {
             14,  28,  42,  56,  70,  84,  98, 112, 126, 140, 150, 152,
            154, 156, 158, 160, 161, 162, 163, 164, 165, 166, 167, 168
        };

        int             consoleup[] =
        {
            154, 140, 126, 112,  98,  84,  70,  56,  42,  28,  14,   0
        };

        // adjust console height
        if (gamestate == GS_TITLESCREEN)
            consoleheight = CONSOLEHEIGHT;
        else if (consolewait < I_GetTimeMS())
        {
            consolewait = I_GetTimeMS() + 10;
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
                if (consoleheight > -4)
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
        if (!prevconsoleactive && (gp_vibrate_damage || gp_vibrate_weapons) && vibrate)
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
        I_UpdateBlitFunc(false);

        // draw background and bottom edge
        C_DrawBackground(consoleheight);

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
        for (i = start; i < end; i++)
        {
            int                 y = CONSOLELINEHEIGHT * (i - start + MAX(0, CONSOLELINES
                                    - consolestrings)) - CONSOLELINEHEIGHT / 2 + 1;
            stringtype_t        type = console[i].type;

            if (type == dividerstring)
                V_DrawConsoleTextPatch(CONSOLETEXTX, y + 5 - (CONSOLEHEIGHT - consoleheight),
                    divider, consoledividercolor, NOBACKGROUNDCOLOR, false, tinttab50);
            else if (M_StringCompare(console[i].string, BINDLISTTITLE))
                V_DrawConsolePatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight), bindlist);
            else if (M_StringCompare(console[i].string, CMDLISTTITLE))
                V_DrawConsolePatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight), cmdlist);
            else if (M_StringCompare(console[i].string, CVARLISTTITLE))
                V_DrawConsolePatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight), cvarlist);
            else if (M_StringCompare(console[i].string, MAPLISTTITLE))
                V_DrawConsolePatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight), maplist);
            else if (M_StringCompare(console[i].string, PLAYERSTATSTITLE))
                V_DrawConsolePatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight), playerstats);
            else
            {
                C_DrawConsoleText(CONSOLETEXTX, y, console[i].string, consolecolors[type],
                    NOBACKGROUNDCOLOR, (type == warningstring ? consolewarningcolor :
                    consoleboldcolor), tinttab66, console[i].tabs, true);
                if (con_timestamps && *console[i].timestamp)
                    C_DrawTimeStamp(timestampx, y, console[i].timestamp);
            }
        }

        // draw input text to left of caret
        for (i = 0; i < MIN(selectstart, caretpos); i++)
            lefttext[i] = consoleinput[i];
        lefttext[i] = '\0';
        C_DrawConsoleText(x, CONSOLEHEIGHT - 17, lefttext, consoleinputcolor, NOBACKGROUNDCOLOR,
            NOBOLDCOLOR, NULL, notabs, false);
        x += C_TextWidth(lefttext, false);

        // draw any selected text to left of caret
        if (selectstart < caretpos)
        {
            for (i = selectstart; i < selectend; i++)
                middletext[i - selectstart] = consoleinput[i];
            middletext[i - selectstart] = '\0';
            if (*middletext)
            {
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, middletext, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false);
                x += C_TextWidth(middletext, false);
            }
        }

        // draw caret
        if (consoledirection == 1 && windowfocused)
        {
            if (caretwait < I_GetTimeMS())
            {
                showcaret = !showcaret;
                caretwait = I_GetTimeMS() + CARETBLINKTIME;
            }
            if (showcaret)
            {
                if (selectend > caretpos)
                    V_DrawConsoleTextPatch(x, consoleheight - 17, caret, consoleselectedinputcolor,
                        consoleselectedinputbackgroundcolor, false, NULL);
                else
                    V_DrawConsoleTextPatch(x, consoleheight - 17, caret, consolecaretcolor,
                        NOBACKGROUNDCOLOR, false, NULL);
            }
        }
        else
        {
            showcaret = false;
            caretwait = 0;
        }
        x += SHORT(caret->width);

        // draw any selected text to right of caret
        if (selectend > caretpos)
        {
            for (i = selectstart; i < selectend; i++)
                middletext[i - selectstart] = consoleinput[i];
            middletext[i - selectstart] = '\0';
            if (*middletext)
            {
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, middletext, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false);
                x += C_TextWidth(middletext, false);
            }
        }

        // draw input text to right of caret
        if ((unsigned int)caretpos < strlen(consoleinput))
        {
            for (i = selectend; (unsigned int)i < strlen(consoleinput); i++)
                righttext[i - selectend] = consoleinput[i];
            righttext[i - selectend] = '\0';
            if (*righttext)
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, righttext, consoleinputcolor,
                    NOBACKGROUNDCOLOR, NOBOLDCOLOR, NULL, notabs, false);
        }

        free(lefttext);
        free(middletext);
        free(righttext);

        // draw the scrollbar
        C_DrawScrollbar();
    }
    else
        consoleactive = false;
}

dboolean C_ValidateInput(char *input)
{
    int i = 0;

    while (*consolecmds[i].name)
    {
        char    cmd[128] = "";

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

                    M_StringCopy(cmd, input, 127);
                    cmd[length - 2] = '\0';

                    if ((M_StringCompare(cmd, consolecmds[i].name)
                        || M_StringCompare(cmd, consolecmds[i].alternate))
                        && length == strlen(cmd) + 2
                        && consolecmds[i].func1(consolecmds[i].name, consolecheatparm))
                    {
                        M_StringCopy(consolecheat, cmd, 127);
                        return true;
                    }
                }
            }
            else if ((M_StringCompare(input, consolecmds[i].name)
                || M_StringCompare(input, consolecmds[i].alternate))
                && consolecmds[i].func1(consolecmds[i].name, ""))
            {
                M_StringCopy(consolecheat, input, 127);
                return true;
            }
        }
        else
        {
            char        parms[128] = "";

            sscanf(input, "%127s %127[^\n]", cmd, parms);
            C_StripQuotes(parms);
            if ((M_StringCompare(cmd, consolecmds[i].name)
                || M_StringCompare(cmd, consolecmds[i].alternate))
                && consolecmds[i].func1(consolecmds[i].name, parms)
                && (consolecmds[i].parameters || !*parms))
            {
                C_Input((input[strlen(input) - 1] == '%' ? "%s%" : "%s"), input);
                consolecmds[i].func2(consolecmds[i].name, uncommify(parms));
                return true;
            }
        }
        i++;
    }

    return C_ExecuteAlias(input);
}

dboolean C_Responder(event_t *ev)
{
    if ((consoleheight < CONSOLEHEIGHT && consoledirection == -1) || messageToPrint)
        return false;

    if (ev->type == ev_keydown)
    {
        int             key = ev->data1;
        char            ch = (char)ev->data2;
        int             i;
        SDL_Keymod      modstate = SDL_GetModState();

        if (key == keyboardconsole)
        {
            C_HideConsole();
            return true;
        }

        switch (key)
        {
            case KEY_BACKSPACE:
                if (selectstart < selectend)
                {
                    // delete selected text
                    C_AddToUndoHistory();

                    for (i = selectend; (unsigned int)i < strlen(consoleinput); i++)
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

                    for (i = caretpos - 1; (unsigned int)i < strlen(consoleinput); i++)
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

                    for (i = selectend; (unsigned int)i < strlen(consoleinput); i++)
                        consoleinput[selectstart + i - selectend] = consoleinput[i];

                    consoleinput[selectstart + i - selectend] = '\0';
                    caretpos = selectend = selectstart;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }
                else if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    // delete character right of caret
                    C_AddToUndoHistory();

                    for (i = caretpos; (unsigned int)i < strlen(consoleinput); i++)
                        consoleinput[i] = consoleinput[i + 1];

                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                    autocomplete = -1;
                    inputhistory = -1;
                }
                break;

            // confirm input
            case KEY_ENTER:
                if (*consoleinput)
                {
                    if (C_ValidateInput(consoleinput))
                    {
                        // clear input
                        consoleinput[0] = '\0';
                        caretpos = selectstart = selectend = 0;
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;
                        undolevels = 0;
                        autocomplete = -1;
                        inputhistory = -1;
                        outputhistory = -1;
                        forceconsoleblurredraw = true;
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

            // move caret right
            case KEY_RIGHTARROW:
                if ((unsigned int)caretpos < strlen(consoleinput))
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
                    caretpos = selectend = selectstart = strlen(consoleinput);

                break;

            // move caret to start
            case KEY_HOME:
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

            // move caret to end
            case KEY_END:
                if (outputhistory != -1 && consolestrings > CONSOLELINES)
                    outputhistory = -1;
                else if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    selectstart = ((modstate & KMOD_SHIFT) ? caretpos : strlen(consoleinput));
                    caretpos = selectend = strlen(consoleinput);
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }
                break;

            // autocomplete
            case KEY_TAB:
                if (*consoleinput)
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
                            && *consolecmds[autocomplete].description)
                        {
                            M_StringCopy(consoleinput, consolecmds[autocomplete].name, sizeof(consoleinput));

                            if (consolecmds[autocomplete].parameters)
                            {
                                int     length = strlen(consoleinput);

                                consoleinput[length] = ' ';
                                consoleinput[length + 1] = '\0';
                            }
                            caretpos = selectstart = selectend = strlen(consoleinput);
                            caretwait = I_GetTimeMS() + CARETBLINKTIME;
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

                for (i = (inputhistory == -1 ? consolestrings : inputhistory) - 1; i >= 0; i--)
                {
                    if (console[i].type == inputstring && !M_StringCompare(consoleinput, console[i].string))
                    {
                        inputhistory = i;
                        M_StringCopy(consoleinput, console[i].string, 255);
                        caretpos = selectstart = selectend = strlen(consoleinput);
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;
                        break;
                    }
                }

                break;

            // next input
            case KEY_DOWNARROW:
                if (inputhistory != -1)
                {
                    for (i = inputhistory + 1; i < consolestrings; i++)
                    {
                        if (console[i].type == inputstring && !M_StringCompare(consoleinput, console[i].string))
                        {
                            inputhistory = i;
                            M_StringCopy(consoleinput, console[i].string, 255);
                            break;
                        }
                    }

                    if (i == consolestrings)
                    {
                        inputhistory = -1;
                        M_StringCopy(consoleinput, currentinput, sizeof(consoleinput));
                    }

                    caretpos = selectstart = selectend = strlen(consoleinput);
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            // scroll output up
            case KEY_PAGEUP:
                if (consolestrings > CONSOLELINES)
                    outputhistory = (outputhistory == -1 ? consolestrings - (CONSOLELINES + 1) :
                        MAX(0, outputhistory - 1));

                break;

            // scroll output down
            case KEY_PAGEDOWN:
                if (outputhistory != -1)
                {
                    if (++outputhistory + CONSOLELINES == consolestrings)
                        outputhistory = -1;
                }

                break;

            // close console
            case KEY_ESCAPE:
                C_HideConsole();
                break;

            // change gamma correction level
            case KEY_F11:
                M_ChangeGamma(modstate & KMOD_SHIFT);
                break;

            // toggle "always run"
            case KEY_CAPSLOCK:
                if (keyboardalwaysrun == KEY_CAPSLOCK)
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

                        if (C_TextWidth(buffer, false) <= CONSOLEINPUTPIXELWIDTH)
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

                            for (i = selectend; (unsigned int)i < strlen(consoleinput); i++)
                                consoleinput[selectstart + i - selectend] = consoleinput[i];

                            consoleinput[selectstart + i - selectend] = '\0';
                            caretpos = selectend = selectstart;
                            caretwait = I_GetTimeMS() + CARETBLINKTIME;
                            showcaret = true;
                        }
                    }

                    // undo
                    else if (ch == 'z')
                    {
                        if (undolevels)
                        {
                            undolevels--;
                            M_StringCopy(consoleinput, undohistory[undolevels].input, sizeof(consoleinput));
                            caretpos = undohistory[undolevels].caretpos;
                            selectstart = undohistory[undolevels].selectstart;
                            selectend = undohistory[undolevels].selectend;
                        }
                    }
                }
                else
                {
                    if ((modstate & KMOD_SHIFT) || (keyboardalwaysrun != KEY_CAPSLOCK && (modstate & KMOD_CAPS)))
                        ch = shiftxform[ch];

                    if (ch >= ' ' && ch < '~' && ch != '`' && C_TextWidth(consoleinput, false)
                        + (ch == ' ' ? spacewidth : SHORT(consolefont[ch
                        - CONSOLEFONTSTART]->width)) - (selectstart < selectend ?
                        C_TextWidth(M_SubString(consoleinput, selectstart, selectend - selectstart),
                        false) : 0) <= CONSOLEINPUTPIXELWIDTH && !(modstate & KMOD_ALT))
                    {
                        C_AddToUndoHistory();

                        if (selectstart < selectend)
                        {
                            // replace selected text with a character
                            consoleinput[selectstart] = ch;

                            for (i = selectend; (unsigned int)i < strlen(consoleinput); i++)
                                consoleinput[selectstart + i - selectend + 1] = consoleinput[i];

                            consoleinput[selectstart + i - selectend + 1] = '\0';
                            caretpos = selectend = selectstart + 1;
                            caretwait = I_GetTimeMS() + CARETBLINKTIME;
                            showcaret = true;
                        }
                        else
                        {
                            // insert a character
                            if (strlen(consoleinput) < 255)
                                consoleinput[strlen(consoleinput) + 1] = '\0';

                            for (i = strlen(consoleinput); i > caretpos; i--)
                                consoleinput[i] = consoleinput[i - 1];

                            consoleinput[caretpos++] = ch;
                        }
                        selectstart = selectend = caretpos;
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
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
            if (consolestrings > CONSOLELINES)
                outputhistory = (outputhistory == -1 ? consolestrings - (CONSOLELINES + 1) :
                    MAX(0, outputhistory - 1));
        }

        // scroll output down
        else if (ev->data1 < 0)
        {
            if (outputhistory != -1)
            {
                if (++outputhistory + CONSOLELINES == consolestrings)
                    outputhistory = -1;
            }
        }
    }
    return true;
}

static int dayofweek(int d, int m, int y)
{
    int adjustment = (14 - m) / 12;

    m += 12 * adjustment - 2;
    y -= adjustment;

    return ((d + (13 * m - 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7);
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

    C_Output("This %i-bit <i><b>%s</b></i> binary of <i><b>"PACKAGE_NAMEANDVERSIONSTRING"</b></i> "
        "was built at %i:%02i%s on %s, %s %i, %i.", (sizeof(intptr_t) == 4 ? 32 : 64),
        SDL_GetPlatform(), (hour > 12 ? hour - 12 : hour), minute, (hour < 12 ? "am" : "pm"),
        days[dayofweek(day, month + 1, year)], months[month], day, year);

#if defined(_MSC_FULL_VER)
    if (_MSC_BUILD)
        C_Output("It was compiled using <i><b>Microsoft C/C++ Optimizing Compiler "
            "v%i.%02i.%i.%i</b></i>.", _MSC_FULL_VER / 10000000,
            (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000, _MSC_BUILD);
    else
        C_Output("It was compiled using <i><b>Microsoft C/C++ Optimizing Compiler "
            "v%i.%02i.%i</b></i>.", _MSC_FULL_VER / 10000000,
            (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000);
#endif
}

void C_PrintSDLVersions(void)
{
    int revision = SDL_GetRevisionNumber();

    if (revision)
        C_Output("Using version %i.%i.%i (revision %s) of <b>sdl2.dll</b>.",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, commify(revision));
    else
        C_Output("Using version %i.%i.%i of <b>sdl2.dll</b>.",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

    C_Output("Using version %i.%i.%i of <b>sdl2_mixer.dll</b> and version %i.%i.%i of "
        "<b>sdl2_image.dll</b>.", SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION,
        SDL_MIXER_PATCHLEVEL, SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION,
        SDL_IMAGE_PATCHLEVEL);
}
