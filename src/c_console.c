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

#include "c_cmds.h"
#include "c_console.h"
#include "d_event.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
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

#define CONSOLESPEED            12

#define CONSOLEFONTSTART        '!'
#define CONSOLEFONTEND          '~'
#define CONSOLEFONTSIZE         (CONSOLEFONTEND - CONSOLEFONTSTART + 1)

#define CONSOLETEXTX            10
#define CONSOLETEXTY            8
#define CONSOLELINEHEIGHT       14

#define CONSOLEINPUTPIXELWIDTH  500

#define SPACEWIDTH              3
#define DIVIDER                 "~~~"
#define ITALICS                 '~'

#define CARETTICS               20

boolean         consoleactive = false;
int             consoleheight = 0;
int             consoledirection = -1;

char            *conback = "";
boolean         defaultconback;

byte            *consolebackground;
patch_t         *consolebottom;
patch_t         *consolefont[CONSOLEFONTSIZE];
patch_t         *lsquote;
patch_t         *ldquote;
patch_t         *multiply;

console_t       *console;
char            consoleinput[255] = "";
int             consolestrings = 0;

patch_t         *caret;
int             caretpos = 0;
static boolean  showcaret = true;
static int      carettics = 0;

char            consolecheat[255] = "";
char            consolecheatparm[3] = "";

static int      autocomplete = -1;
static char     autocompletetext[255] = "";

static int      inputhistory = -1;

static int      outputhistory = -1;

extern boolean  translucency;
extern byte     *tinttab75;
extern int      fps;

char *upper =
{
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0 !\"#$%&\"()*+,_>?)!@#$%^&*(:"
    ":<+>?\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0{\\}^_`ABCDEFGHIJKLMNOPQRSTUVWXYZ"
};

int     consolecaretcolor = 227;
int     consolehighfpscolor = -116;
int     consoleinputcolor = 227;
int     consoleinputtooutputcolor = 227;
int     consolelowfpscolor = -180;
int     consolemaptitlecolor = 227;
int     consoleplayermessagecolor = 180;
int     consoleoutputcolor = 227;
int     consoletitlecolor = 227;
int     consoledividercolor = 227;

int consolecolors[4];

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
    ++consolestrings;
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
    ++consolestrings;
}

void C_PlayerMessage(char *string, ...)
{
    va_list     argptr;
    char        buffer[1024];
    int         len;

    va_start(argptr, string);
    memset(buffer, 0, sizeof(buffer));
    M_vsnprintf(buffer, sizeof(buffer) - 1, string, argptr);
    va_end(argptr);

    len = strlen(buffer);
    if (buffer[len - 1] != '.' && buffer[len - 1] != '!')
    {
        buffer[len] = '.';
        buffer[len + 1] = 0;
    }

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
        ++consolestrings;
    }
}

void C_AddConsoleDivider(void)
{
    if (!consolestrings || strcasecmp(console[consolestrings - 1].string, DIVIDER))
        C_Print(output, DIVIDER);
}

static void C_DrawDivider(int y)
{
    int i;

    y *= SCREENWIDTH;
    if (y > 0)
        for (i = y + CONSOLETEXTX; i <= y + SCREENWIDTH - CONSOLETEXTX; ++i)
            screens[0][i] = consoledividercolor;
    if ((y += SCREENWIDTH) > 0)
        for (i = y + CONSOLETEXTX; i <= y + SCREENWIDTH - CONSOLETEXTX; ++i)
            screens[0][i] = consoledividercolor;
}

void C_Init(void)
{
    int         i;
    int         j = CONSOLEFONTSTART;
    char        buffer[9];

    if (!conback[0] || (gamemode == commercial && !strcasecmp(conback, "FLOOR7_2"))
        || R_CheckFlatNumForName(conback) < 0)
    {
        conback = (gamemode == commercial ? "GRNROCK" : "FLOOR7_2");
        M_SaveDefaults();
    }
    consolebackground = W_CacheLumpName(conback, PU_CACHE);
    consolebottom = W_CacheLumpName("BRDR_B", PU_CACHE);

    defaultconback = ((gamemode == commercial && !strcasecmp(conback, "GRNROCK"))
        || (gamemode != commercial && !strcasecmp(conback, "FLOOR7_2")));

    for (i = 0; i < CONSOLEFONTSIZE; i++)
    {
        M_snprintf(buffer, 9, "DRFON%03d", j++);
        consolefont[i] = W_CacheLumpName(buffer, PU_STATIC);
    }
    lsquote = W_CacheLumpName("DRFON145", PU_STATIC);
    ldquote = W_CacheLumpName("DRFON147", PU_STATIC);
    multiply = W_CacheLumpName("DRFON215", PU_STATIC);

    caret = consolefont['|' - CONSOLEFONTSTART];

    consolecolors[input] = consoleinputtooutputcolor;
    consolecolors[output] = consoleoutputcolor;
    consolecolors[title] = consoletitlecolor;
    consolecolors[playermessage] = consoleplayermessagecolor;
}

void C_HideConsole(void)
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

static void C_DrawBackground(int height)
{
    byte        *dest = screens[0];
    int         x, y;
    int         offset = CONSOLEHEIGHT - height + 5 * !defaultconback;
    int         top = offset;

    for (y = offset; y < height + offset; y += 2)
        for (x = 0; x < SCREENWIDTH / 32; x += 2)
        {
            int i;

            for (i = 0; i < 64; i++)
            {
                int     j = i * 2;

                if (top >= CONSOLETOP * SCREENWIDTH)
                {
                    int     dot = *(consolebackground + (((y / 2) & 63) << 6) + i);

                    if (translucency)
                    {
                        dot <<= 8;
                        *(dest + j) = tinttab75[dot + *(dest + j)];
                        ++j;
                        *(dest + j) = tinttab75[dot + *(dest + j)];
                    }
                    else
                    {
                        *(dest + j) = dot;
                        ++j;
                        *(dest + j) = dot;
                    }
                }
            }
            dest += 128;
            top += 128;
        }

    if (defaultconback)
        for (x = 0; x < ORIGINALWIDTH; x += 8)
            V_DrawTranslucentConsolePatch(x, height / 2, consolebottom);

    y = height / 2 + 3;
    if (y >= CONSOLETOP)
        for (x = 0; x < ORIGINALWIDTH; ++x)
            V_DrawPixel(x, y, 251, true);
}

static struct
{
    char        char1;
    char        char2;
    int         adjust;
} kern[] = {
    { '\"', '+',  -1 }, { '\"', 'a',  -1 }, { '\"', 'c',  -1 }, { '\"', 'd',  -1 },
    { '\"', 'e',  -1 }, { '\"', 'g',  -1 }, { '\"', 'j',  -2 }, { '\"', 'o',  -1 },
    { '\"', 'q',  -1 }, { '\"', 's',  -1 }, { '\'', 'a',  -1 }, { '\'', 'c',  -1 },
    { '\'', 'd',  -1 }, { '\'', 'e',  -1 }, { '\'', 'g',  -1 }, { '\'', 'j',  -2 },
    { '\'', 'o',  -1 }, { '\"', 'q',  -1 }, { '\'', 's',  -1 }, { '.',  '7',  -1 },
    { '/',  'o',  -1 }, { ':', '\\',  -1 }, { '_',  'f',  -1 }, { '0',  ',',  -1 },
    { '0',  'j',  -2 }, { '1',  '\"', -1 }, { '1',  '\'', -1 }, { '1',  'j',  -2 },
    { '2',  'j',  -2 }, { '3',  ',',  -1 }, { '3',  'j',  -2 }, { '4',  'j',  -2 },
    { '5',  ',',  -1 }, { '5',  'j',  -2 }, { '6',  ',',  -1 }, { '6',  'j',  -2 },
    { '7',  ',',  -2 }, { '7',  'j',  -2 }, { '8',  ',',  -1 }, { '8',  'j',  -2 },
    { '9',  ',',  -1 }, { '9',  'j',  -2 }, { 'a',  '\"', -1 }, { 'a',  '\'', -1 },
    { 'a',  'j',  -2 }, { 'b',  ',',  -1 }, { 'b',  '\"', -1 }, { 'b',  '\'', -1 },
    { 'b',  'j',  -2 }, { 'c',  ',',  -1 }, { 'c',  '\"', -1 }, { 'c',  '\'', -1 },
    { 'c',  'j',  -2 }, { 'd',  'j',  -2 }, { 'e',  ',',  -1 }, { 'e',  '\"', -1 },
    { 'e',  '\'', -1 }, { 'e',  '_',  -1 }, { 'e',  'j',  -2 }, { 'f',  ',',  -2 },
    { 'f',  'j',  -2 }, { 'h',  '\"', -1 }, { 'h',  '\'', -1 }, { 'h',  'j',  -2 },
    { 'i',  'j',  -2 }, { 'k',  'j',  -2 }, { 'l',  'j',  -2 }, { 'm',  '\"', -1 },
    { 'm',  '\'', -1 }, { 'm',  'j',  -2 }, { 'n',  '\"', -1 }, { 'n',  '\'', -1 },
    { 'n',  'j',  -2 }, { 'o',  ',',  -1 }, { 'o',  '\"', -1 }, { 'o',  '\'', -1 },
    { 'o',  'j',  -2 }, { 'p',  ',',  -1 }, { 'p',  '\"', -1 }, { 'p',  '\'', -1 },
    { 'p',  'j',  -2 }, { 'r',  '.',  -2 }, { 'r',  ',',  -2 }, { 'r',  '\"', -1 },
    { 'r',  '\'', -1 }, { 'r',  'a',  -1 }, { 'r',  'j',  -2 }, { 's',  ',',  -1 },
    { 's',  'j',  -2 }, { 't',  'j',  -2 }, { 'u',  'j',  -2 }, { 'v',  ',',  -1 },
    { 'v',  'j',  -2 }, { 'w',  'j',  -2 }, { 'x',  'j',  -2 }, { 'z',  'j',  -2 },
    {  0 ,   0 ,   0 }
};

static int C_TextWidth(char *text)
{
    size_t      i;
    char        prev = ' ';
    int         w = 0;

    for (i = 0; i < strlen(text); ++i)
    {
        char    letter = text[i];
        int     c = letter - CONSOLEFONTSTART;
        int     j = 0;

        w += (c < 0 || c >= CONSOLEFONTSIZE ? SPACEWIDTH : SHORT(consolefont[c]->width));

        while (kern[j].char1)
        {
            if (prev == kern[j].char1 && letter == kern[j].char2)
            {
                w += kern[j].adjust;
                break;
            }
            ++j;
        }
        prev = letter;
    }
    return w;
}

static char     prevletter;

static void C_DrawText(int x, int y, char *text, int color)
{
    boolean     italics = false;

    if (!strcasecmp(text, DIVIDER))
        C_DrawDivider(y + 5 - (CONSOLEHEIGHT - consoleheight));
    else
    {
        size_t      i;
        int         tabs = 0;

        for (i = 0; i < strlen(text); ++i)
        {
            char    letter = text[i];
            int     c = letter - CONSOLEFONTSTART;
            char    nextletter = text[i + 1];

            if (letter == ITALICS)
            {
                italics = !italics;
                if (!italics)
                    ++x;
            }
            else
            {
                if (letter == '\t')
                    x = MAX(x, (++tabs == 1 ? 40 : tabs * 65));
                else if (c < 0 || c >= CONSOLEFONTSIZE)
                    x += SPACEWIDTH;
                else
                {
                    patch_t     *patch = consolefont[c];
                    int         k = 0;

                    if (isdigit(prevletter) && letter == 'x' && isdigit(nextletter))
                        patch = multiply;
                    else if (prevletter == ' ' || prevletter == '\t')
                    {
                        if (letter == '\'')
                            patch = lsquote;
                        else if (letter == '\"')
                            patch = ldquote;
                    }

                    if (!italics)
                        while (kern[k].char1)
                        {
                            if (prevletter == kern[k].char1 && letter == kern[k].char2)
                            {
                                x += kern[k].adjust;
                                break;
                            }
                            ++k;
                        }

                    V_DrawConsoleChar(x, y - (CONSOLEHEIGHT - consoleheight), patch, color, italics);
                    x += SHORT(patch->width);
                }
                prevletter = letter;
            }
        }
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
        char    *right = Z_Malloc(512, PU_STATIC, NULL);
        boolean prevconsoleactive = consoleactive;

        // adjust height
        consoleheight = BETWEEN(0, consoleheight + CONSOLESPEED * consoledirection, CONSOLEHEIGHT);

        consoleactive = (consoleheight >= CONSOLEHEIGHT / 2);

        if (consoleactive != prevconsoleactive && gamepadvibrate && vibrate)
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

        // draw title and version
        prevletter = ' ';
        C_DrawText(SCREENWIDTH - C_TextWidth(PACKAGE_NAMEANDVERSIONSTRING) - CONSOLETEXTX,
            CONSOLEHEIGHT - 15, PACKAGE_NAMEANDVERSIONSTRING, consoletitlecolor);

        // draw console text
        if (outputhistory == -1)
        {
            start = MAX(0, consolestrings - 10);
            end = consolestrings;
        }
        else
        {
            start = outputhistory;
            end = outputhistory + 10;
        }
        for (i = start; i < end; ++i)
        {
            prevletter = ' ';
            C_DrawText(CONSOLETEXTX,
                CONSOLETEXTY + CONSOLELINEHEIGHT * (i - start + MAX(0, 10 - consolestrings)),
                console[i].string, consolecolors[console[i].type]);
        }

        // draw input text to left of caret
        prevletter = ' ';
        for (i = 0; i < caretpos; ++i)
            left[i] = consoleinput[i];
        left[i] = 0;
        C_DrawText(x, CONSOLEHEIGHT - 15, left, consoleinputcolor);

        // draw caret
        if (carettics++ == CARETTICS)
        {
            carettics = 0;
            showcaret = !showcaret;
        }
        x += C_TextWidth(left);
        if (showcaret)
            V_DrawConsoleChar(x, consoleheight - 15, caret, consolecaretcolor, false);

        // draw input text to right of caret
        for (i = 0; (unsigned int)i < strlen(consoleinput) - caretpos; ++i)
            right[i] = consoleinput[i + caretpos];
        right[i] = 0;
        if (right[0])
            C_DrawText(x + 3, CONSOLEHEIGHT - 15, right, consoleinputcolor);
    }

    if (showfps && fps)
    {
        static char buffer[16];
        size_t      i;
        size_t      len;
        int         x = SCREENWIDTH - CONSOLETEXTX;
        int         color = (fps < TICRATE ? consolelowfpscolor : consolehighfpscolor);
        static int  prevfps = 0;

        M_snprintf(buffer, 16, "%i FPS", fps);

        len = strlen(buffer);

        for (i = 0; i < len; ++i)
            x -= (buffer[i] == ' ' ? SPACEWIDTH :
                SHORT(consolefont[buffer[i] - CONSOLEFONTSTART]->width));

        for (i = 0; i < len; ++i)
            if (buffer[i] == ' ')
                x += SPACEWIDTH;
            else
            {
                patch_t *patch = consolefont[buffer[i] - CONSOLEFONTSTART];

                V_DrawConsoleChar(x, CONSOLETEXTY, patch, color, false);
                x += SHORT(patch->width);
            }

        if ((menuactive || paused) && fps != prevfps)
            blurred = false;

        prevfps = fps;
    }
}

boolean C_Responder(event_t *ev)
{
    if ((consoleheight < CONSOLEHEIGHT && consoledirection == -1) ||
        (!consoleheight && consoledirection == -1))
        return false;

    if (ev->type == ev_keydown)
    {
        int             key = ev->data1;
        int             ch = ev->data2;
        int             i;

#if defined(SDL20)
        SDL_Keymod      modstate = SDL_GetModState();
#else
        SDLMod          modstate = SDL_GetModState();
#endif

        switch (key)
        {
            // delete character left of caret
            case KEY_BACKSPACE:
                if (caretpos > 0)
                {
                    for (i = caretpos - 1; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[i] = consoleinput[i + 1];
                    --caretpos;
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // delete character right of caret
            case KEY_DEL:
                if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    for (i = caretpos; (unsigned int)i < strlen(consoleinput); ++i)
                        consoleinput[i] = consoleinput[i + 1];
                    carettics = 0;
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
                            char        cmd[255] = "";
                            char        parm[255] = "";

                            if (consolecmds[i].type == CT_CHEAT)
                            {
                                int     length = strlen(consoleinput);

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
                                        C_Print(input, consoleinput);
                                        M_StringCopy(consolecheat, cmd, 255);
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                sscanf(consoleinput, "%s %s", cmd, parm);
                                C_StripQuotes(parm);
                                if (!strcasecmp(cmd, consolecmds[i].name)
                                    && consolecmds[i].condition(cmd, parm, ""))
                                {
                                    validcmd = true;
                                    C_Print(input, consoleinput);
                                    consolecmds[i].function(cmd, parm, "");
                                    break;
                                }
                            }
                        }
                        else if (consolecmds[i].parameters == 2)
                        {
                            char        cmd[255] = "";
                            char        parm1[255] = "";
                            char        parm2[255] = "";

                            sscanf(consoleinput, "%s %s %s", cmd, parm1, parm2);
                            C_StripQuotes(parm1);
                            C_StripQuotes(parm2);
                            if (!strcasecmp(cmd, consolecmds[i].name)
                                && consolecmds[i].condition(cmd, parm1, parm2))
                            {
                                validcmd = true;
                                C_Print(input, consoleinput);
                                consolecmds[i].function(cmd, parm1, parm2);
                                break;
                            }
                        }
                        else if (!strcasecmp(consoleinput, consolecmds[i].name)
                            && consolecmds[i].condition(consoleinput, "", ""))
                        {
                            validcmd = true;
                            C_Print(input, consoleinput);
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
                        carettics = 0;
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
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // move caret right
            case KEY_RIGHTARROW:
                if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    ++caretpos;
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // move caret to start
            case KEY_HOME:
                if (caretpos > 0)
                {
                    caretpos = 0;
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // move caret to end
            case KEY_END:
                if ((unsigned int)caretpos < strlen(consoleinput))
                {
                    caretpos = strlen(consoleinput);
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // autocomplete
            case KEY_TAB:
                if (consoleinput[0])
                {
                    if (autocomplete == -1)
                    {
                        autocomplete = 0;
                        M_StringCopy(autocompletetext, consoleinput, 255);
                    }

                    while (consolecmds[autocomplete].name[0])
                    {
                        if (M_StringStartsWith(consolecmds[autocomplete].name, autocompletetext)
                            && consolecmds[autocomplete].type != CT_CHEAT)
                        {
                            M_StringCopy(consoleinput, consolecmds[autocomplete].name, 255);
                            if (consolecmds[autocomplete].parameters)
                            {
                                int     length = strlen(consoleinput);

                                consoleinput[length] = ' ';
                                consoleinput[length + 1] = 0;
                            }
                            ++autocomplete;
                            break;
                        }
                        ++autocomplete;
                    }
                    caretpos = strlen(consoleinput);
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // previous input
            case KEY_UPARROW:
                for (i = (inputhistory == -1 ? consolestrings - 2 : inputhistory - 1); i >= 0; --i)
                {
                    if (console[i].type == input)
                    {
                        inputhistory = i;
                        M_StringCopy(consoleinput, console[i].string, 255);
                        caretpos = strlen(consoleinput);
                        carettics = 0;
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
                        if (console[i].type == input)
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
                    caretpos = strlen(consoleinput);
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // scroll output up
            case KEY_PGUP:
                if (consolestrings > 10)
                    outputhistory = (outputhistory == -1 ? consolestrings - 11 : MAX(0, outputhistory - 1));
                break;

            // scroll output down
            case KEY_PGDN:
                if (outputhistory != -1)
                {
                    ++outputhistory;
                    if (outputhistory + 10 == consolestrings)
                        outputhistory = -1;
                }
                break;

            // close console:
            case KEY_ESCAPE:
            case KEY_TILDE:
                consoledirection = -1;
                break;

            default:
            {
                if (modstate & KMOD_SHIFT)
                    ch = upper[ch];
                if (ch >= ' ' && ch < '~' && ch != '`'
                    && C_TextWidth(consoleinput) + (ch == ' ' ? SPACEWIDTH :
                    consolefont[ch - CONSOLEFONTSTART]->width) <= CONSOLEINPUTPIXELWIDTH
                    && !(modstate & (KMOD_ALT | KMOD_CTRL)))
                {
                    consoleinput[strlen(consoleinput) + 1] = '\0';
                    for (i = strlen(consoleinput); i > caretpos; --i)
                        consoleinput[i] = consoleinput[i - 1];
                    consoleinput[caretpos++] = ch;
                    carettics = 0;
                    showcaret = true;
                }
            }
        }
        if (autocomplete != -1 && key != KEY_TAB)
            autocomplete = -1;

        if (inputhistory != -1 && key != KEY_UPARROW && key != KEY_DOWNARROW)
            inputhistory = -1;

        if (outputhistory != -1 && key != KEY_PGUP && key != KEY_PGDN)
            outputhistory = -1;
    }
#if defined(SDL20)
    else if (ev->type == ev_mousewheel)
    {
        // scroll output up
        if (ev->data1 > 0)
        {
            if (consolestrings > 10)
                outputhistory = (outputhistory == -1 ? consolestrings - 11 : MAX(0, outputhistory - 1));
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
                outputhistory = (outputhistory == -1 ? consolestrings - 11 : MAX(0, outputhistory - 1));
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

void C_PrintCompileDate(void)
{
    int                 day, year, hour, minute;
    static const char   mths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    static const char   *months[] =
    {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    static char         month[4];

    sscanf(__DATE__, "%s %d %d", month, &day, &year);
    sscanf(__TIME__, "%d:%d:%*d", &hour, &minute);
    C_Output("%s was built on %s %i, %i at %i:%02i%s.",
        uppercase(PACKAGE_EXE), months[(strstr(mths, month) - mths) / 3], day, year,
        (hour > 12 ? hour - 12 : hour), minute, (hour < 12 ? "am" : "pm"));
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

void C_SetBTSXColorScheme(void)
{
    consolecaretcolor = 80;
    consolehighfpscolor = -116;
    consoleinputcolor = 80;
    consoleinputtooutputcolor = 80;
    consolelowfpscolor = -180;
    consolemaptitlecolor = 80;
    consoleplayermessagecolor = (BTSXE1 ? 196 : 214);
    consoleoutputcolor = 80;
    consoletitlecolor = 80;
    consoledividercolor = 80;

    consolecolors[input] = consoleinputtooutputcolor;
    consolecolors[output] = consoleoutputcolor;
    consolecolors[title] = consoletitlecolor;
    consolecolors[playermessage] = consoleplayermessagecolor;
}
