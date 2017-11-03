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
#include "g_game.h"
#include "i_colors.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "s_sound.h"
#include "sc_man.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"

#define CONSOLELINES            (gamestate != GS_TITLESCREEN ? 11 : 27)
#define CONSOLETEXTX            10
#define CONSOLETEXTY            8
#define CONSOLETEXTMAXLENGTH    1024
#define CONSOLELINEHEIGHT       14

#define CONSOLESCROLLBARWIDTH   3
#define CONSOLESCROLLBARHEIGHT  ((CONSOLELINES - 1) * CONSOLELINEHEIGHT - 1)
#define CONSOLESCROLLBARX       (CONSOLEWIDTH - CONSOLETEXTX - CONSOLESCROLLBARWIDTH)
#define CONSOLESCROLLBARY       (CONSOLETEXTY + 1)

#define CONSOLETEXTPIXELWIDTH   (CONSOLEWIDTH - CONSOLETEXTX * 3 - CONSOLESCROLLBARWIDTH + 3 + !scrollbardrawn * 8)

#define CONSOLEINPUTPIXELWIDTH  (CONSOLEWIDTH - CONSOLETEXTX - brandwidth - 2)

#define DIVIDER                 "~~~"

#define CARETBLINKTIME          350

console_t               *console;

dboolean                consoleactive;
int                     consoleheight = 0;
int                     consoledirection = -1;
static int              consoleanim;

dboolean                forceconsoleblurredraw;

patch_t                 *consolefont[CONSOLEFONTSIZE];
patch_t                 *degree;

static patch_t          *trademark;
static patch_t          *copyright;
static patch_t          *regomark;
static patch_t          *multiply;
static patch_t          *warning;
static patch_t          *brand;
static patch_t          *divider;
static patch_t          *bindlist;
static patch_t          *cmdlist;
static patch_t          *cvarlist;
static patch_t          *maplist;
static patch_t          *playerstats;
static patch_t          *thinglist;

static short            brandwidth;
static short            brandheight;
static short            spacewidth;

static char             consoleinput[255];
static int              numautocomplete;
int                     consolestrings;

static int              undolevels;
static undohistory_t    *undohistory;

static patch_t          *caret;
static int              caretpos;
static dboolean         showcaret = true;
static int              caretwait;
static int              selectstart;
static int              selectend;

char                    consolecheat[255];
char                    consolecheatparm[3];

static int              outputhistory = -1;

dboolean                con_timestamps = con_timestamps_default;
static int              timestampx;
static int              zerowidth;

static byte             c_blurscreen[SCREENWIDTH * SCREENHEIGHT];

static int              consolecaretcolor = 4;
static int              consolelowfpscolor = 180;
static int              consolehighfpscolor = 116;
static int              consoleinputcolor = 4;
static int              consoleselectedinputcolor = 4;
static int              consoleselectedinputbackgroundcolor = 100;
static int              consoleinputtooutputcolor = 4;
static int              consoletitlecolor = 88;
static int              consolememorycolor = 88;
static int              consoleplayermessagecolor = 161;
static int              consoletimestampcolor = 100;
static int              consoleoutputcolor = 88;
static int              consoleboldcolor = 4;
static int              consoleitalicscolor = 98;
static int              consoleheadercolor = 180;
static int              consolewarningcolor = 180;
static int              consolewarningboldcolor = 176;
static int              consoledividercolor = 100;
static int              consoletintcolor = 5;
static int              consoleedgecolor = 180;
static int              consolescrollbartrackcolor = 100;
static int              consolescrollbarfacecolor = 94;

static int              consolecolors[STRINGTYPES];

static dboolean         scrollbardrawn;

extern char             autocompletelist[][255];
extern int              fps;
extern int              refreshrate;
extern dboolean         dowipe;
extern dboolean         togglingvanilla;

void C_Print(const stringtype_t type, const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    console = I_Realloc(console, (consolestrings + 1) * sizeof(*console));
    M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
    console[consolestrings].type = type;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings++].timestamp[0] = '\0';
    outputhistory = -1;
}

void C_Input(const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH] = "";

    if (togglingvanilla)
        return;

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    console = I_Realloc(console, (consolestrings + 1) * sizeof(*console));
    M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
    console[consolestrings].type = inputstring;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings++].timestamp[0] = '\0';
    outputhistory = -1;
}

void C_IntCVAROutput(const char *cvar, const int value)
{
    if (consolestrings && M_StringStartsWith(console[consolestrings - 1].string, cvar))
        consolestrings--;

    C_Input("%s %i", cvar, value);
}

void C_PctCVAROutput(const char *cvar, const int value)
{
    if (consolestrings && M_StringStartsWith(console[consolestrings - 1].string, cvar))
        consolestrings--;

    C_Input("%s %i%%", cvar, value);
}

void C_StrCVAROutput(const char *cvar, const char *string)
{
    if (consolestrings && M_StringStartsWith(console[consolestrings - 1].string, cvar))
        consolestrings--;

    C_Input("%s %s", cvar, string);
}

void C_CCMDOutput(const char *ccmd)
{
    if (!consolestrings || !M_StringCompare(console[consolestrings - 1].string, ccmd))
        C_Input(ccmd);
}

void C_Output(const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    console = I_Realloc(console, (consolestrings + 1) * sizeof(*console));
    M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
    console[consolestrings].type = outputstring;
    memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
    console[consolestrings++].timestamp[0] = '\0';
    outputhistory = -1;
}

void C_TabbedOutput(const int tabs[8], const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    console = I_Realloc(console, (consolestrings + 1) * sizeof(*console));
    M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
    console[consolestrings].type = outputstring;
    memcpy(console[consolestrings].tabs, tabs, sizeof(console[consolestrings].tabs));
    console[consolestrings++].timestamp[0] = '\0';
    outputhistory = -1;
}

void C_Warning(const char *string, ...)
{
    va_list argptr;
    char    buffer[CONSOLETEXTMAXLENGTH] = "";

    va_start(argptr, string);
    M_vsnprintf(buffer, CONSOLETEXTMAXLENGTH - 1, string, argptr);
    va_end(argptr);

    if (consolestrings && !M_StringCompare(console[consolestrings - 1].string, buffer))
    {
        console = I_Realloc(console, (consolestrings + 1) * sizeof(*console));
        M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
        console[consolestrings].type = warningstring;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        console[consolestrings++].timestamp[0] = '\0';
        outputhistory = -1;
    }
}

void C_PlayerMessage(const char *string, ...)
{
    va_list         argptr;
    char            buffer[CONSOLETEXTMAXLENGTH] = "";
    const int       i = consolestrings - 1;
    const dboolean  prevplayermessage = (i >= 0 && console[i].type == playermessagestring);
    time_t          rawtime;

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
        console = I_Realloc(console, (consolestrings + 1) * sizeof(*console));
        M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
        console[consolestrings].type = playermessagestring;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        strftime(console[consolestrings++].timestamp, 9, "%H:%M:%S", localtime(&rawtime));
    }

    outputhistory = -1;
}

void C_Obituary(const char *string, ...)
{
    va_list         argptr;
    char            buffer[CONSOLETEXTMAXLENGTH] = "";
    const int       i = consolestrings - 1;
    const dboolean  prevobituary = (i >= 0 && console[i].type == obituarystring);
    time_t          rawtime;

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
        console = I_Realloc(console, (consolestrings + 1) * sizeof(*console));
        M_StringCopy(console[consolestrings].string, buffer, CONSOLETEXTMAXLENGTH);
        console[consolestrings].type = obituarystring;
        memset(console[consolestrings].tabs, 0, sizeof(console[consolestrings].tabs));
        strftime(console[consolestrings++].timestamp, 9, "%H:%M:%S", localtime(&rawtime));
    }

    outputhistory = -1;
}

static void C_AddToUndoHistory(void)
{
    undohistory = I_Realloc(undohistory, (undolevels + 1) * sizeof(*undohistory));
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

const static struct
{
    char    char1;
    char    char2;
    int     adjust;
} kern[] = {
    { ' ',  '(',  -1 }, { ' ',  'T',  -1 }, { '\"', '+',  -1 }, { '\"', '.',  -1 },
    { '\"', 'a',  -1 }, { '\"', 'c',  -1 }, { '\"', 'd',  -1 }, { '\"', 'e',  -1 },
    { '\"', 'g',  -1 }, { '\"', 'j',  -2 }, { '\"', 'o',  -1 }, { '\"', 'q',  -1 },
    { '\"', 's',  -1 }, { '\"', 'J',  -2 }, { '\\', '\\', -2 }, { '\\', 'd',  -1 },
    { '\\', 'V',  -1 }, { '\'', 'a',  -1 }, { '\'', 'a',  -1 }, { '\'', 'c',  -1 },
    { '\'', 'd',  -1 }, { '\'', 'e',  -1 }, { '\'', 'g',  -1 }, { '\'', 'j',  -2 },
    { '\'', 'o',  -1 }, { '\'', 's',  -1 }, { '\'', 'J',  -2 }, { '.',  '\\', -1 },
    { '.',  '4',  -1 }, { '.',  '7',  -1 }, { ',',  '4',  -1 }, { '/',  '/',  -2 },
    { '/',  'd',  -1 }, { '/',  'o',  -1 }, { ':', '\\',  -1 }, { '_',  'f',  -1 },
    { '0',  ',',  -1 }, { '0',  ';',  -1 }, { '0',  'j',  -2 }, { '1',  '\"', -1 },
    { '1',  '\'', -1 }, { '1',  'j',  -2 }, { '2',  'j',  -2 }, { '3',  ',',  -1 },
    { '3',  ';',  -1 }, { '3',  'j',  -2 }, { '4',  'j',  -2 }, { '5',  ',',  -1 },
    { '5',  ';',  -1 }, { '5',  'j',  -2 }, { '6',  ',',  -1 }, { '6',  'j',  -2 },
    { '7',  '.',  -2 }, { '7',  ',',  -2 }, { '7',  ';',  -1 }, { '7',  'j',  -2 },
    { '8',  ',',  -1 }, { '8',  ';',  -1 }, { '8',  'j',  -2 }, { '9',  ',',  -1 },
    { '9',  ';',  -1 }, { '9',  'j',  -2 }, { 'F',  '.',  -1 }, { 'F',  ',',  -1 },
    { 'F',  ';',  -1 }, { 'L',  '\\', -1 }, { 'L',  '\"', -1 }, { 'L',  '\'', -1 },
    { 'P',  '.',  -1 }, { 'P',  ',',  -1 }, { 'P',  ';',  -1 }, { 'T',  '.',  -1 },
    { 'T',  ',',  -1 }, { 'T',  ';',  -1 }, { 'T',  'a',  -1 }, { 'T',  'e',  -1 },
    { 'T',  'o',  -1 }, { 'V',  '.',  -1 }, { 'V',  ',',  -1 }, { 'V',  ';',  -1 },
    { 'Y',  '.',  -1 }, { 'Y',  ',',  -1 }, { 'Y',  ';',  -1 }, { 'a',  '\"', -1 },
    { 'a',  '\'', -1 }, { 'a',  'j',  -2 }, { 'b',  ',',  -1 }, { 'b',  ';',  -1 },
    { 'b',  '\"', -1 }, { 'b',  '\\', -1 }, { 'b',  '\'', -1 }, { 'b',  'j',  -2 },
    { 'c',  '\\', -1 }, { 'c',  ',',  -1 }, { 'c',  ';',  -1 }, { 'c',  '\"', -1 },
    { 'c',  '\'', -1 }, { 'c',  'j',  -2 }, { 'd',  'j',  -2 }, { 'e',  '\\', -1 },
    { 'e',  ',',  -1 }, { 'e',  ';',  -1 }, { 'e',  '\"', -1 }, { 'e',  '\'', -1 },
    { 'e',  '_',  -1 }, { 'e',  'j',  -2 }, { 'f',  ' ',  -1 }, { 'f',  ',',  -2 },
    { 'f',  ';',  -1 }, { 'f',  '_',  -1 }, { 'f',  'a',  -1 }, { 'f',  'j',  -2 },
    { 'h',  '\\', -1 }, { 'h',  'j',  -2 }, { 'i',  'j',  -2 }, { 'k',  'j',  -2 },
    { 'l',  'j',  -2 }, { 'm',  '\"', -1 }, { 'm',  '\\', -1 }, { 'm',  '\'', -1 },
    { 'm',  'j',  -2 }, { 'n',  '\\', -1 }, { 'n',  '\"', -1 }, { 'n',  '\'', -1 },
    { 'n',  'j',  -2 }, { 'o',  '\\', -1 }, { 'o',  ',',  -1 }, { 'o',  ';',  -1 },
    { 'o',  '\"', -1 }, { 'o',  '\'', -1 }, { 'o',  'j',  -2 }, { 'p',  '\\', -1 },
    { 'p',  ',',  -1 }, { 'p',  ';',  -1 }, { 'p',  '\"', -1 }, { 'p',  '\'', -1 },
    { 'p',  'j',  -2 }, { 'r',  ' ',  -1 }, { 'r',  '\\', -1 }, { 'r',  '.',  -2 },
    { 'r',  ',',  -2 }, { 'r',  ';',  -1 }, { 'r',  '\"', -1 }, { 'r',  '\'', -1 },
    { 'r',  '_',  -1 }, { 'r',  'a',  -1 }, { 'r',  'j',  -2 }, { 's',  '\\', -1 },
    { 's',  ',',  -1 }, { 's',  ';',  -1 }, { 's',  'j',  -2 }, { 't',  'j',  -2 },
    { 'u',  'j',  -2 }, { 'v',  ',',  -1 }, { 'v',  ';',  -1 }, { 'v',  'j',  -2 },
    { 'w',  'j',  -2 }, { 'x',  'j',  -2 }, { 'z',  'j',  -2 }, { '\0', '\0',  0 }
};

static int C_TextWidth(const char *text, const dboolean formatting, const dboolean kerning)
{
    const int       len = (int)strlen(text);
    unsigned char   prevletter = '\0';
    int             w = 0;

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];
        const int           c = letter - CONSOLEFONTSTART;
        const unsigned char nextletter = text[i + 1];

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

        if (kerning)
            for (int j = 0; kern[j].char1; j++)
                if (prevletter == kern[j].char1 && letter == kern[j].char2)
                {
                    w += kern[j].adjust;
                    break;
                }

        prevletter = letter;
    }

    return w;
}

static void C_DrawScrollbar(void)
{
    const int   trackstart = CONSOLESCROLLBARY * CONSOLEWIDTH;
    const int   trackend = trackstart + CONSOLESCROLLBARHEIGHT * CONSOLEWIDTH;
    const int   facestart = (CONSOLESCROLLBARY + CONSOLESCROLLBARHEIGHT * (outputhistory == -1 ?
                    MAX(0, consolestrings - CONSOLELINES) : outputhistory) / consolestrings) * CONSOLEWIDTH;
    const int   faceend = facestart + (CONSOLESCROLLBARHEIGHT - CONSOLESCROLLBARHEIGHT
                    * MAX(0, consolestrings - CONSOLELINES) / consolestrings) * CONSOLEWIDTH;

    if (trackstart == facestart && trackend == faceend)
        scrollbardrawn = false;
    else
    {
        const int   offset = (CONSOLEHEIGHT - consoleheight) * CONSOLEWIDTH;

        // Draw scrollbar track
        for (int y = trackstart; y < trackend; y += CONSOLEWIDTH)
            if (y - offset >= 0)
                for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                    screens[0][y - offset + x] = tinttab50[screens[0][y - offset + x]
                        + consolescrollbartrackcolor];

        // Draw scrollbar face
        for (int y = facestart; y < faceend; y += CONSOLEWIDTH)
            if (y - offset >= 0)
                for (int x = CONSOLESCROLLBARX; x < CONSOLESCROLLBARX + CONSOLESCROLLBARWIDTH; x++)
                    screens[0][y - offset + x] = consolescrollbarfacecolor;

        scrollbardrawn = true;
    }
}

void C_Init(void)
{
    int     j = CONSOLEFONTSTART;
    char    buffer[9];

    for (int i = 0; i < CONSOLEFONTSIZE; i++)
    {
        M_snprintf(buffer, sizeof(buffer), "DRFON%03d", j++);
        consolefont[i] = W_CacheLumpName(buffer);
    }

    trademark = W_CacheLumpName("DRFON153");
    copyright = W_CacheLumpName("DRFON169");
    regomark = W_CacheLumpName("DRFON174");
    degree = W_CacheLumpName("DRFON176");
    multiply = W_CacheLumpName("DRFON215");

    brand = W_CacheLumpName("DRBRAND");
    caret = W_CacheLumpName("DRCARET");
    divider = W_CacheLumpName("DRDIVIDE");
    warning = W_CacheLumpName("DRFONWRN");

    bindlist = W_CacheLumpName("DRBNDLST");
    cmdlist = W_CacheLumpName("DRCMDLST");
    cvarlist = W_CacheLumpName("DRCVRLST");
    maplist = W_CacheLumpName("DRMAPLST");
    playerstats = W_CacheLumpName("DRPLYRST");
    thinglist = W_CacheLumpName("DRTHNLST");

    brandwidth = SHORT(brand->width);
    brandheight = SHORT(brand->height);
    spacewidth = SHORT(consolefont[' ' - CONSOLEFONTSTART]->width);
    timestampx = CONSOLEWIDTH - C_TextWidth("00:00:00", false, false) - CONSOLETEXTX * 2 - CONSOLESCROLLBARWIDTH + 1;
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
    consolewarningboldcolor = nearestcolors[consolewarningboldcolor];
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

    while (*autocompletelist[++numautocomplete]);
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
    int len = (int)strlen(string);

    if (len > 2 && ((string[0] == '\"' && string[len - 1] == '\"') || (string[0] == '\''
        && string[len - 1] == '\'')))
    {
        len -= 2;
        memmove(string, string + 1, len);
        string[len] = '\0';
    }
}

static void DoBlurScreen(const int x1, const int y1, const int x2, const int y2, const int i)
{
    static byte c_tempscreen[SCREENWIDTH * SCREENHEIGHT];

    memcpy(c_tempscreen, c_blurscreen, CONSOLEWIDTH * (CONSOLEHEIGHT + 5));

    for (int y = y1; y < y2; y += CONSOLEWIDTH)
        for (int x = y + x1; x < y + x2; x++)
            c_blurscreen[x] = tinttab50[c_tempscreen[x] + (c_tempscreen[x + i] << 8)];
}

static void C_DrawBackground(int height)
{
    static dboolean blurred;

    height = (height + 5) * CONSOLEWIDTH;

    if (!blurred)
    {
        for (int i = 0; i < height; i++)
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

    blurred = (consoleheight == CONSOLEHEIGHT && !dowipe);

    if (forceconsoleblurredraw)
    {
        forceconsoleblurredraw = false;
        blurred = false;
    }

    for (int i = 0; i < height; i++)
        screens[0][i] = tinttab50[(consoletintcolor << 8) + c_blurscreen[i]];

    for (int i = height - 2; i > 1; i -= 3)
    {
        screens[0][i] = colormaps[0][256 * 6 + screens[0][i]];

        if (((i - 1) % CONSOLEWIDTH) < CONSOLEWIDTH - 2)
            screens[0][i + 1] = colormaps[0][256 * 6 + screens[0][i - 1]];
    }

    // draw branding
    V_DrawConsolePatch(CONSOLEWIDTH - brandwidth, consoleheight - brandheight + 2, brand);

    // draw bottom edge
    for (int i = height - CONSOLEWIDTH * 3; i < height; i++)
        screens[0][i] = tinttab50[consoleedgecolor + screens[0][i]];

    // soften edges
    for (int i = 0; i < height; i += CONSOLEWIDTH)
    {
        screens[0][i] = tinttab50[screens[0][i]];
        screens[0][i + CONSOLEWIDTH - 1] = tinttab50[screens[0][i + CONSOLEWIDTH - 1]];
    }

    for (int i = height - CONSOLEWIDTH + 1; i < height - 1; i++)
        screens[0][i] = tinttab25[screens[0][i]];

    // draw shadow
    if (gamestate != GS_TITLESCREEN)
        for (int i = CONSOLEWIDTH; i <= 4 * CONSOLEWIDTH; i += CONSOLEWIDTH)
            for (int j = height; j < height + i; j++)
                screens[0][j] = colormaps[0][256 * 4 + screens[0][j]];
}

static void C_DrawConsoleText(int x, int y, char *text, const int color1, const int color2,
    const int boldcolor, byte *tinttab, const int tabs[8], const dboolean formatting, const dboolean kerning)
{
    int             bold = 0;
    dboolean        italics = false;
    int             tab = -1;
    int             len = (int)strlen(text);
    int             truncate = len;
    unsigned char   prevletter = '\0';
    int             width = 0;
    int             lastcolor1;

    y -= CONSOLEHEIGHT - consoleheight;

    if (color1 == consolewarningcolor)
    {
        V_DrawConsoleTextPatch(x, y, warning, color1, color2, false, tinttab);
        width += SHORT(warning->width) + 2;
        x += width;
    }

    if (len > 80)
        while (C_TextWidth(M_SubString(text, 0, truncate), formatting, kerning) + width + 6 > CONSOLETEXTPIXELWIDTH)
            truncate--;

    for (int i = 0; i < truncate; i++)
    {
        const unsigned char letter = text[i];

        if (letter == '<' && text[i + 1] == 'b' && text[i + 2] == '>' && formatting)
        {
            bold = (italics ? 2 : 1);
            i += 2;
        }
        else if (letter == '<' && text[i + 1] == '/' && text[i + 2] == 'b' && text[i + 3] == '>' && formatting)
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
            const unsigned char nextletter = text[i + 1];
            const int           c = letter - CONSOLEFONTSTART;

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
                for (int j = 0; kern[j].char1; j++)
                    if (prevletter == kern[j].char1 && letter == kern[j].char2)
                    {
                        x += kern[j].adjust;
                        break;
                    }

            if (patch)
            {
                V_DrawConsoleTextPatch(x, y, patch, (lastcolor1 = (bold == 1 ? boldcolor : (bold == 2 ? color1 :
                    (italics ? (color1 == consolewarningcolor ? color1 : consoleitalicscolor) : color1)))),
                    color2, italics, tinttab);
                x += SHORT(patch->width);
            }

            prevletter = letter;
        }
    }

    if (truncate < len)
    {
        patch_t *patch = consolefont['.' - CONSOLEFONTSTART];

        for (int i = 0; i < 3; i++)
        {
            V_DrawConsoleTextPatch(x, y, patch, lastcolor1, color2, false, tinttab);
            x += SHORT(patch->width);
        }
    }
}

static void C_DrawOverlayText(int x, int y, const char *text, const int color)
{
    const int   len = (int)strlen(text);

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = text[i];

        if (letter == ' ')
            x += spacewidth;
        else
        {
            patch_t *patch = consolefont[letter - CONSOLEFONTSTART];

            V_DrawConsoleTextPatch(x, y, patch, color, NOBACKGROUNDCOLOR, false, (r_hud_translucency ?
                tinttab75 : NULL));
            x += SHORT(patch->width);
        }
    }
}

static void C_DrawTimeStamp(int x, int y, const char *text)
{
    const int   len = (int)strlen(text);

    y -= (CONSOLEHEIGHT - consoleheight);

    for (int i = 0; i < len; i++)
    {
        patch_t     *patch = consolefont[text[i] - CONSOLEFONTSTART];
        const int   width = SHORT(patch->width);

        V_DrawConsoleTextPatch(x + (text[i] == '1' ? (zerowidth - width) / 2 : 0), y, patch,
            consoletimestampcolor, NOBACKGROUNDCOLOR, false, tinttab25);
        x += (isdigit(text[i]) ? zerowidth : width);
    }
}

void C_UpdateFPS(void)
{
    if (fps && !dowipe && !paused && !menuactive)
    {
        static char buffer[32];

        M_snprintf(buffer, sizeof(buffer), "%i FPS (%.1fms)", fps, 1000.0 / fps);

        C_DrawOverlayText(CONSOLEWIDTH - C_TextWidth(buffer, false, false) - CONSOLETEXTX + 1, CONSOLETEXTY,
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
        int             len;
        char            lefttext[512];
        char            middletext[512];
        char            righttext[512];
        const dboolean  prevconsoleactive = consoleactive;
        static int      consolewait;

        const int consoledown[] =
        {
            14,  28,  42,  56,  70,  84,  98, 112, 126, 140, 150, 152,
            154, 156, 158, 160, 161, 162, 163, 164, 165, 166, 167, 168
        };

        const int consoleup[] = { 154, 140, 126, 112, 98, 84, 70, 56, 42, 28, 14, 0 };

        const int notabs[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

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
        if (!prevconsoleactive && (gp_vibrate_barrels || gp_vibrate_damage || gp_vibrate_weapons) && vibrate)
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

        // draw the scrollbar
        C_DrawScrollbar();

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
            const int           y = CONSOLELINEHEIGHT * (i - start + MAX(0, CONSOLELINES - consolestrings))
                                    - CONSOLELINEHEIGHT / 2 + 1;
            const stringtype_t  type = console[i].type;

            if (type == dividerstring)
                V_DrawConsoleTextPatch(CONSOLETEXTX, y + 5 - (CONSOLEHEIGHT - consoleheight), divider,
                    consoledividercolor, NOBACKGROUNDCOLOR, false, tinttab50);
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
            else if (M_StringCompare(console[i].string, THINGLISTTITLE))
                V_DrawConsolePatch(CONSOLETEXTX, y + 4 - (CONSOLEHEIGHT - consoleheight), thinglist);
            else
            {
                C_DrawConsoleText(CONSOLETEXTX, y, console[i].string, consolecolors[type],
                    NOBACKGROUNDCOLOR, (type == warningstring ? consolewarningboldcolor : consoleboldcolor),
                    tinttab66, console[i].tabs, true, true);

                if (con_timestamps && *console[i].timestamp)
                    C_DrawTimeStamp(timestampx, y, console[i].timestamp);
            }
        }

        // draw input text to left of caret
        for (i = 0; i < MIN(selectstart, caretpos); i++)
            lefttext[i] = consoleinput[i];

        lefttext[i] = '\0';
        C_DrawConsoleText(x, CONSOLEHEIGHT - 17, lefttext, consoleinputcolor, NOBACKGROUNDCOLOR,
            NOBOLDCOLOR, NULL, notabs, false, true);
        x += C_TextWidth(lefttext, false, true);

        // draw any selected text to left of caret
        if (selectstart < caretpos)
        {
            for (i = selectstart; i < selectend; i++)
                middletext[i - selectstart] = consoleinput[i];

            middletext[i - selectstart] = '\0';

            if (*middletext)
            {
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, middletext, consoleselectedinputcolor,
                    consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false, true);
                x += C_TextWidth(middletext, false, true);
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
                    consoleselectedinputbackgroundcolor, NOBOLDCOLOR, NULL, notabs, false, true);
                x += C_TextWidth(middletext, false, true);
            }
        }

        // draw input text to right of caret
        len = (int)strlen(consoleinput);

        if (caretpos < len)
        {
            for (i = selectend; i < len; i++)
                righttext[i - selectend] = consoleinput[i];

            righttext[i - selectend] = '\0';

            if (*righttext)
                C_DrawConsoleText(x, CONSOLEHEIGHT - 17, righttext, consoleinputcolor, NOBACKGROUNDCOLOR,
                    NOBOLDCOLOR, NULL, notabs, false, true);
        }
    }
    else
        consoleactive = false;
}

dboolean C_ExecuteInputString(const char *input)
{
    char    *string = strdup(input);
    char    *strings[255];
    int     j = 0;

    C_StripQuotes(string);
    strings[0] = strtok(string, ";");

    while (strings[j])
    {
        if (!C_ValidateInput(trimwhitespace(strings[j])))
            break;

        strings[++j] = strtok(NULL, ";");
    }

    return true;
}

dboolean C_ValidateInput(const char *input)
{
    for (int i = 0; *consolecmds[i].name; i++)
    {
        char    cmd[128] = "";

        if (consolecmds[i].type == CT_CHEAT)
        {
            if (consolecmds[i].parameters)
            {
                const int   length = (int)strlen(input);

                if (isdigit(input[length - 2]) && isdigit(input[length - 1]))
                {
                    consolecheatparm[0] = input[length - 2];
                    consolecheatparm[1] = input[length - 1];
                    consolecheatparm[2] = '\0';

                    M_StringCopy(cmd, input, sizeof(cmd));
                    cmd[length - 2] = '\0';

                    if ((M_StringCompare(cmd, consolecmds[i].name)
                        || M_StringCompare(cmd, consolecmds[i].alternate))
                        && length == strlen(cmd) + 2
                        && consolecmds[i].func1(consolecmds[i].name, consolecheatparm))
                    {
                        if (gamestate == GS_LEVEL)
                            M_StringCopy(consolecheat, cmd, sizeof(consolecheat));

                        return true;
                    }
                }
            }
            else if ((M_StringCompare(input, consolecmds[i].name)
                || M_StringCompare(input, consolecmds[i].alternate))
                && consolecmds[i].func1(consolecmds[i].name, ""))
            {
                M_StringCopy(consolecheat, input, sizeof(consolecheat));
                return true;
            }
        }
        else
        {
            char    parms[128] = "";

            sscanf(input, "%127s %127[^\n]", cmd, parms);
            C_StripQuotes(parms);

            if ((M_StringCompare(cmd, consolecmds[i].name)
                || M_StringCompare(cmd, consolecmds[i].alternate))
                && consolecmds[i].func1(consolecmds[i].name, parms)
                && (consolecmds[i].parameters || !*parms))
            {
                if (!executingalias)
                    C_Input((input[strlen(input) - 1] == '%' ? "%s%" : "%s"), input);

                consolecmds[i].func2(consolecmds[i].name, parms);
                return true;
            }
        }
    }

    if (C_ExecuteAlias(input))
        return true;

    if (gamestate == GS_LEVEL)
        for (int i = 0; *actions[i].action; i++)
            if (M_StringCompare(input, actions[i].action))
            {
                C_Input(input);

                if (consoleactive)
                    C_HideConsoleFast();

                if (actions[i].func)
                    actions[i].func();

                return true;
            }

    return false;
}

dboolean C_Responder(event_t *ev)
{
    static const char *shiftxform =
    {
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0 !\"#$%&\"()*+<_>?"
        ")!@#$%^&*(::<+>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}\"_'ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~\0"
    };

    if ((consoleheight < CONSOLEHEIGHT && consoledirection == -1) || messageToPrint)
        return false;

    if (ev->type == ev_keydown)
    {
        static int          autocomplete = -1;
        static int          inputhistory = -1;
        static char         currentinput[255];
        const int           key = ev->data1;
        char                ch = (char)ev->data2;
        int                 i;
        const int           len = (int)strlen(consoleinput);
        const SDL_Keymod    modstate = SDL_GetModState();

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
                if (*consoleinput)
                {
                    char        *string = strdup(consoleinput);
                    char        *strings[255];
                    int         i = 0;
                    dboolean    result = false;

                    strings[0] = strtok(string, ";");

                    while (strings[i])
                    {
                        if (C_ValidateInput(trimwhitespace(strings[i])))
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

                    return !consolecheat[0];
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
                if (*consoleinput && caretpos == len)
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
                        i += 1 + (consoleinput[i + 1] == ' ');
                        M_StringCopy(prefix, M_SubString(consoleinput, 0, i), sizeof(input));

                        if (autocomplete == -1)
                        {
                            M_StringCopy(input, M_SubString(consoleinput, i, len - i), sizeof(input));

                            if (!*input)
                                break;
                        }
                    }

                    spaces1 = numspaces(input);
                    endspace1 = (input[strlen(input) - 1] == ' ');

                    while ((direction == -1 && autocomplete > 0)
                        || (direction == 1 && autocomplete < numautocomplete - 1))
                    {
                        static char output[255];
                        int         spaces2;
                        dboolean    endspace2;
                        int         len2;

                        autocomplete += direction;
                        M_StringCopy(output, autocompletelist[autocomplete], sizeof(output));
                        len2 = (int)strlen(output);
                        spaces2 = numspaces(output);
                        endspace2 = (output[len2 - 1] == ' ');

                        if (M_StringStartsWith(output, input)
                            && ((!spaces1 && (!spaces2 || (spaces2 == 1 && endspace2)))
                                || (spaces1 == 1 && !endspace1 && (spaces2 == 1 || (spaces2 == 2 && endspace2)))
                                || (spaces1 == 2 && !endspace1 && (spaces2 == 2 || (spaces2 == 3 && endspace2)))
                                || (spaces1 == 3 && !endspace1)))
                        {
                            M_StringCopy(consoleinput, M_StringJoin(prefix, output, NULL), sizeof(consoleinput));
                            caretpos = selectstart = selectend = len2 + (int)strlen(prefix);
                            caretwait = I_GetTimeMS() + CARETBLINKTIME;
                            showcaret = true;
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
                    if (consolestrings > CONSOLELINES)
                        outputhistory = (outputhistory == -1 ? consolestrings - (CONSOLELINES + 1) :
                            MAX(0, outputhistory - 1));
                }

                // previous input
                else
                {
                    if (inputhistory == -1)
                        M_StringCopy(currentinput, consoleinput, sizeof(currentinput));

                    for (i = (inputhistory == -1 ? consolestrings : inputhistory) - 1; i >= 0; i--)
                        if (console[i].type == inputstring && !M_StringCompare(consoleinput, console[i].string))
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
                    if (outputhistory != -1)
                        if (++outputhistory + CONSOLELINES == consolestrings)
                            outputhistory = -1;
                }

                // next input
                else
                {
                    if (inputhistory != -1)
                    {
                        for (i = inputhistory + 1; i < consolestrings; i++)
                            if (console[i].type == inputstring && !M_StringCompare(consoleinput, console[i].string))
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
                if (consolestrings > CONSOLELINES)
                    outputhistory = (outputhistory == -1 ? consolestrings - (CONSOLELINES + 1) :
                        MAX(0, outputhistory - 1));

                break;

            case KEY_PAGEDOWN:
                // scroll output down
                if (outputhistory != -1)
                    if (++outputhistory + CONSOLELINES == consolestrings)
                        outputhistory = -1;

                break;

            case KEY_ESCAPE:
                // close console
                C_HideConsole();
                break;

            case KEY_F11:
                // change gamma correction level
                M_ChangeGamma(modstate & KMOD_SHIFT);
                break;

            case KEY_CAPSLOCK:
                // toggle "always run"
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
                        selectend = caretpos = len;
                    }

                    // copy selected text to clipboard
                    else if (ch == 'c')
                    {
                        if (selectstart < selectend)
                            SDL_SetClipboardText(M_SubString(consoleinput, selectstart, selectend - selectstart));
                    }

                    // paste text from clipboard
                    else if (ch == 'v')
                    {
                        char    buffer[255];

                        M_snprintf(buffer, sizeof(buffer), "%s%s%s", M_SubString(consoleinput, 0, selectstart),
                            SDL_GetClipboardText(), M_SubString(consoleinput, selectend, len - selectend));

                        if (C_TextWidth(buffer, false, true) <= CONSOLEINPUTPIXELWIDTH)
                        {
                            C_AddToUndoHistory();
                            M_StringCopy(consoleinput, buffer, sizeof(consoleinput));
                            selectstart += (int)strlen(SDL_GetClipboardText());
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

                            for (i = selectend; i < len; i++)
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

                    if (ch >= ' ' && ch < '~' && ch != '`' && C_TextWidth(consoleinput, false, true)
                        + (ch == ' ' ? spacewidth : SHORT(consolefont[ch - CONSOLEFONTSTART]->width))
                        - (selectstart < selectend ? C_TextWidth(M_SubString(consoleinput, selectstart,
                        selectend - selectstart), false, true) : 0) <= CONSOLEINPUTPIXELWIDTH
                        && !(modstate & KMOD_ALT))
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
                if (++outputhistory + CONSOLELINES == consolestrings)
                    outputhistory = -1;
        }
    }

    return true;
}

static int dayofweek(int d, int m, int y)
{
    const int   adjustment = (14 - m) / 12;

    m += 12 * adjustment - 2;
    y -= adjustment;

    return ((d + (13 * m - 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7);
}

void C_PrintCompileDate(void)
{
    int day, month, year, hour, minute;

    static const char *days[] =
    {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };

    static const char mths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    static const char *months[] =
    {
        "", "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    static char mth[4] = "";

    sscanf(__DATE__, "%3s %2d %4d", mth, &day, &year);
    sscanf(__TIME__, "%2d:%2d:%*d", &hour, &minute);
    month = (int)(strstr(mths, mth) - mths) / 3 + 1;

    C_Output("This %i-bit <i><b>%s</b></i> binary of <i><b>%s</b></i> was built at %i:%02i%s on %s, %s %i, %i.",
        (sizeof(intptr_t) == 4 ? 32 : 64), SDL_GetPlatform(), PACKAGE_NAMEANDVERSIONSTRING,
        (hour > 12 ? hour - 12 : hour), minute, (hour < 12 ? "am" : "pm"), days[dayofweek(day, month, year)],
        months[month], day, year);

#if defined(_MSC_FULL_VER)
    if (_MSC_BUILD)
        C_Output("It was compiled using <i><b>Microsoft C/C++ Optimizing Compiler v%i.%02i.%i.%i</b></i>.",
            _MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000, _MSC_BUILD);
    else
        C_Output("It was compiled using <i><b>Microsoft C/C++ Optimizing Compiler v%i.%02i.%i</b></i>.",
            _MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, _MSC_FULL_VER % 100000);
#endif
}

void C_PrintSDLVersions(void)
{
    const int   revision = SDL_GetRevisionNumber();

    if (revision)
        C_Output("Using version %i.%i.%i (revision %s) of <b>%s</b>.", SDL_MAJOR_VERSION,
            SDL_MINOR_VERSION, SDL_PATCHLEVEL, commify(revision), SDL_FILENAME);
    else
        C_Output("Using version %i.%i.%i of <b>%s</b>.", SDL_MAJOR_VERSION, SDL_MINOR_VERSION,
            SDL_PATCHLEVEL, SDL_FILENAME);

    C_Output("Using version %i.%i.%i of <b>%s</b> and version %i.%i.%i of <b>%s</b>.",
        SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL, SDL_MIXER_FILENAME,
        SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL, SDL_IMAGE_FILENAME);
}
