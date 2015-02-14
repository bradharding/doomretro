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
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_cheat.h"
#include "m_menu.h"
#include "m_misc.h"
#include "SDL.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define CONSOLESPEED            12
#define CONSOLEHEIGHT           ((SCREENHEIGHT - SBARHEIGHT) / 2)

#define CONSOLEFONTSTART        '!'
#define CONSOLEFONTEND          '~'
#define CONSOLEFONTSIZE         (CONSOLEFONTEND - CONSOLEFONTSTART + 1)

#define CONSOLETEXTX            10
#define CONSOLETEXTY            8
#define CONSOLELINEHEIGHT       14

#define CONSOLEINPUTPIXELWIDTH  500

#define SPACEWIDTH              3
#define DIVIDER                 "~~~"

#define CARETTICS               20

int             consoleheight = 0;
int             consoledirection = 1;

byte            *background;
patch_t         *divider;
patch_t         *consolefont[CONSOLEFONTSIZE];
patch_t         *lsquote;
patch_t         *ldquote;

console_t       *console;
char            consoleinput[255] = "";
int             consolestrings = 0;

patch_t         *caret;
int             caretpos = 0;
static boolean  showcaret = true;
static int      carettics = 0;

char            consolecheat[255] = "";
char            consolecheatparm[3] = "";
char            consolecmdparm[255] = "";

static int      autocomplete = -1;
static char     autocompletetext[255] = "";

static int      inputhistory = -1;

static int      outputhistory = -1;

extern byte     *tinttab75;

void C_AddConsoleString(char *string, stringtype_t type, byte color)
{
    console = realloc(console, (consolestrings + 1) * sizeof(*console));
    console[consolestrings].string = strdup(string);
    console[consolestrings].type = type;
    console[consolestrings].color = color;
    ++consolestrings;
}

void C_AddConsoleDivider(void)
{
    C_AddConsoleString(DIVIDER, output, 0);
}

static void C_DrawDivider(int y)
{
    int x;

    for (x = 0; x < ORIGINALWIDTH; x += 8)
        V_DrawTranslucentConsolePatch(x, y / 2, divider);
}

void C_Init(void)
{
    int         i;
    int         j = CONSOLEFONTSTART;
    char        buffer[9];

    background = W_CacheLumpName((gamemode == commercial ? "GRNROCK" : "FLOOR7_2"), PU_CACHE);
    divider = W_CacheLumpName("BRDR_B", PU_CACHE);

    for (i = 0; i < CONSOLEFONTSIZE; i++)
    {
        M_snprintf(buffer, 9, "DRFON%03d", j++);
        consolefont[i] = W_CacheLumpName(buffer, PU_STATIC);
    }
    lsquote = W_CacheLumpName("DRFON145", PU_STATIC);
    ldquote = W_CacheLumpName("DRFON147", PU_STATIC);

    caret = consolefont['|' - CONSOLEFONTSTART];
}

static void C_DrawBackground(int height)
{
    byte        *dest = screens[0];
    int         x, y;
    int         offset = CONSOLEHEIGHT - height;
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
                    int     dot = *(background + (((y / 2) & 63) << 6) + i) << 8;

                    *(dest + j) = tinttab75[dot + *(dest + j)];
                    ++j;
                    *(dest + j) = tinttab75[dot + *(dest + j)];
                }
            }
            dest += 128;
            top += 128;
        }

    C_DrawDivider(height);

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
    { '_', 'f', -1 }, { '/', 'o', -1 }, { '.', '7', -1 }, { '0', 'j', -2 },
    { '1', 'j', -2 }, { '2', 'j', -2 }, { '3', 'j', -2 }, { '4', 'j', -2 },
    { '5', 'j', -2 }, { '6', 'j', -2 }, { '7', 'j', -2 }, { '8', 'j', -2 },
    { '9', 'j', -2 }, { 'a', 'j', -2 }, { 'b', 'j', -2 }, { 'c', 'j', -2 },
    { 'd', 'j', -2 }, { 'e', '_', -1 }, { 'e', 'j', -2 }, { 'f', 'j', -2 },
    { 'h', 'j', -2 }, { 'i', 'j', -2 }, { 'k', 'j', -2 }, { 'l', 'j', -2 },
    { 'm', 'j', -2 }, { 'n', 'j', -2 }, { 'o', 'j', -2 }, { 'p', 'j', -2 },
    { 'r', 'a', -1 }, { 'r', 'j', -2 }, { 'r', '.', -1 }, { 's', 'j', -2 },
    { 't', 'j', -2 }, { 'u', 'j', -2 }, { 'v', 'j', -2 }, { 'w', 'j', -2 },
    { 'x', 'j', -2 }, { 'z', 'j', -2 }, {  0 ,  0 ,  0 }
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

static void C_DrawText(int x, int y, char *text, byte color)
{
    if (!strcasecmp(text, DIVIDER))
        C_DrawDivider(y + 4 - (CONSOLEHEIGHT - consoleheight));
    else
    {
        size_t      i;
        char        prev = ' ';
        int         tabs = 0;

        for (i = 0; i < strlen(text); ++i)
        {
            char    letter = text[i];
            int     c = letter - CONSOLEFONTSTART;

            if (letter == '\t')
                x = MAX(x, (++tabs == 1 ? 40 : tabs * 50));
            else if (c < 0 || c >= CONSOLEFONTSIZE)
                x += SPACEWIDTH;
            else
            {
                patch_t     *patch = consolefont[c];
                int         k = 0;

                if (prev == ' ')
                {
                    if (letter == '\'')
                        patch = lsquote;
                    else if (letter == '\"')
                        patch = ldquote;
                }

                while (kern[k].char1)
                {
                    if (prev == kern[k].char1 && letter == kern[k].char2)
                    {
                        x += kern[k].adjust;
                        break;
                    }
                    ++k;
                }

                V_DrawConsoleChar(x, y - (CONSOLEHEIGHT - consoleheight), patch, color);
                x += SHORT(patch->width);
                prev = letter;
            }
        }
    }
}

void C_Drawer(void)
{
    if (!consoleheight)
        return;
    else
    {
        int     i;
        int     start;
        int     end;
        char    *left = Z_Malloc(512, PU_STATIC, NULL);
        char    *right = Z_Malloc(512, PU_STATIC, NULL);

        // adjust height
        consoleheight = BETWEEN(0, consoleheight + CONSOLESPEED * consoledirection, CONSOLEHEIGHT);

        // draw tiled background and bottom edge
        C_DrawBackground(consoleheight);

        // draw title and version
        C_DrawText(SCREENWIDTH - C_TextWidth(PACKAGE_NAMEANDVERSIONSTRING) - CONSOLETEXTX,
            CONSOLEHEIGHT - 15, PACKAGE_NAMEANDVERSIONSTRING, CONSOLETITLECOLOR);

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
            C_DrawText(CONSOLETEXTX, 
                CONSOLETEXTY + CONSOLELINEHEIGHT * (i - start + MAX(0, 10 - consolestrings)),
                console[i].string, console[i].color);

        // draw input text to left of caret
        for (i = 0; i < caretpos; ++i)
            left[i] = consoleinput[i];
        left[i] = 0;
        C_DrawText(CONSOLETEXTX, CONSOLEHEIGHT - 15, left, CONSOLEINPUTCOLOR);

        // draw caret
        if (carettics++ == CARETTICS)
        {
            carettics = 0;
            showcaret = !showcaret;
        }
        if (showcaret)
            V_DrawConsoleChar(CONSOLETEXTX + C_TextWidth(left), consoleheight - 15, caret,
                CONSOLECARETCOLOR);

        // draw input text to right of caret
        for (i = 0; (unsigned int)i < strlen(consoleinput) - caretpos; ++i)
            right[i] = consoleinput[i + caretpos];
        right[i] = 0;
        C_DrawText(CONSOLETEXTX + C_TextWidth(left) + 3, CONSOLEHEIGHT - 15, right, CONSOLEINPUTCOLOR);
    }
}

boolean C_Responder(event_t *ev)
{
    if (consoleheight != CONSOLEHEIGHT)
        return false;

    if (ev->type == ev_keydown)
    {
        int             key = ev->data1;
        int             ch = ev->data2;
        int             i;
        SDL_Keymod      modstate = SDL_GetModState();

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
                    while (consolecmds[i].cmd[0])
                    {
                        if (consolecmds[i].parms)
                        {
                            char        cmd[255] = "";

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

                                    if (!strcasecmp(cmd, consolecmds[i].cmd)
                                        && length == strlen(cmd) + 2
                                        && consolecmds[i].condition(cmd))
                                    {
                                        validcmd = true;
                                        C_AddConsoleString(consoleinput, input,
                                            CONSOLEINPUTTOOUTPUTCOLOR);
                                        M_StringCopy(consolecheat, cmd, 255);
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                sscanf(consoleinput, "%s %s", cmd, consolecmdparm);
                                if (!strcasecmp(cmd, consolecmds[i].cmd)
                                    && consolecmds[i].condition(cmd))
                                {
                                    validcmd = true;
                                    C_AddConsoleString(consoleinput, input,
                                        CONSOLEINPUTTOOUTPUTCOLOR);
                                    consolecmds[i].func();
                                    consolecmdparm[0] = 0;
                                    break;
                                }
                            }
                        }
                        else if (!strcasecmp(consoleinput, consolecmds[i].cmd)
                            && consolecmds[i].condition(consoleinput))
                        {
                            validcmd = true;
                            C_AddConsoleString(consoleinput, input,
                                CONSOLEINPUTTOOUTPUTCOLOR);
                            if (consolecmds[i].type == CT_CHEAT)
                                M_StringCopy(consolecheat, consoleinput, 255);
                            else
                                consolecmds[i].func();
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

                    while (consolecmds[autocomplete].cmd[0])
                    {
                        if (M_StringStartsWith(consolecmds[autocomplete].cmd, autocompletetext)
                            && consolecmds[autocomplete].type != CT_CHEAT)
                        {
                            M_StringCopy(consoleinput, consolecmds[autocomplete].cmd, 255);
                            if (consolecmds[autocomplete].parms)
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

            default:
                if (modstate & KMOD_SHIFT)
                    ch = toupper(ch);
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
        if (autocomplete != -1 && key != KEY_TAB)
            autocomplete = -1;

        if (inputhistory != -1 && key != KEY_UPARROW && key != KEY_DOWNARROW)
            inputhistory = -1;

        if (outputhistory != -1 && key != KEY_PGUP && key != KEY_PGDN)
            outputhistory = -1;
    }

    return true;
}
