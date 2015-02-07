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

#include "d_event.h"
#include "doomstat.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "SDL.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define CONSOLESPEED            8
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

char            **consolestring = NULL;
char            consoleinput[255] = "";
int             consolestrings = 0;

patch_t         *caret;
int             caretpos = 0;
static boolean  showcaret = true;
static int      carettics = 0;

char            consolecheat[255] = "";

void C_CmdList(void);
void C_Quit(void);

typedef struct
{
    char        *command;
    boolean     cheat;

    void(*func)(void);
} consolecommand_t;

consolecommand_t consolecommands[] =
{
    { "cmdlist",    false, C_CmdList },
    { "idbeholda",  true,  NULL      },
    { "idbeholdl",  true,  NULL      },
    { "idbeholdi",  true,  NULL      },
    { "idbeholdr",  true,  NULL      },
    { "idbeholds",  true,  NULL      },
    { "idbeholdv",  true,  NULL      },
    { "idchoppers", true,  NULL      },
  //{ "idclev",     true,  NULL      },
    { "idclip",     true,  NULL      },
    { "iddqd",      true,  NULL      },
    { "iddt",       true,  NULL      },
    { "idfa",       true,  NULL      },
    { "idkfa",      true,  NULL      },
  //{ "idmus",      true,  NULL      },
    { "idmypos",    true,  NULL      },
    { "idspispopd", true,  NULL      },
    { "quit",       false, C_Quit    },
    { "",           false, NULL      }
};

extern byte     *tinttab75;

void C_AddConsoleString(char *string)
{
    consolestring = (char **)realloc(consolestring, (consolestrings + 1) * sizeof(char *));
    consolestring[consolestrings++] = strdup(string);
}

void C_AddConsoleDivider(void)
{
    C_AddConsoleString(DIVIDER);
}

static void C_DrawDivider(int y)
{
    int x;

    for (x = 0; x < ORIGINALWIDTH; x += 8)
        V_DrawTranslucentPatch(x, y / 2, 0, divider);
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

    caret = consolefont['|' - CONSOLEFONTSTART];
}

static void C_DrawBackground(int height)
{
    byte        *dest = screens[0];
    int         x, y;
    int         offset = CONSOLEHEIGHT - height;

    for (y = offset; y < height + offset; y += 2)
        for (x = 0; x < SCREENWIDTH / 32; x += 2)
        {
            int i;

            for (i = 0; i < 64; i++)
            {
                int     j = i * 2;
                int     dot = *(background + (((y / 2) & 63) << 6) + i) << 8;

                *(dest + j) = tinttab75[dot + *(dest + j)];
                ++j;
                *(dest + j) = tinttab75[dot + *(dest + j)];
            }
            dest += 128;
        }

    C_DrawDivider(height);

    for (x = 0; x < ORIGINALWIDTH; ++x)
        V_DrawPixel(x, height / 2 + 3, 251, true);
}

static int C_TextWidth(char *text)
{
    size_t      i;
    int         w = 0;

    for (i = 0; i < strlen(text); ++i)
    {
        int     c = text[i] - CONSOLEFONTSTART;

        w += (c < 0 || c >= CONSOLEFONTSIZE ? SPACEWIDTH : SHORT(consolefont[c]->width));
    }
    return w;
}

static void C_DrawText(int x, int y, char *text)
{
    if (!strcasecmp(text, DIVIDER))
        C_DrawDivider(y + 4 - (CONSOLEHEIGHT - consoleheight));
    else
    {
        size_t      i;

        for (i = 0; i < strlen(text); ++i)
        {
            int     c = text[i] - CONSOLEFONTSTART;

            if (c < 0 || c >= CONSOLEFONTSIZE)
                x += SPACEWIDTH;
            else
            {
                patch_t     *patch = consolefont[c];

                V_DrawBigPatch(x, y - (CONSOLEHEIGHT - consoleheight), 0, patch);
                x += SHORT(patch->width);
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
        char    *left = Z_Malloc(512, PU_STATIC, NULL);
        char    *right = Z_Malloc(512, PU_STATIC, NULL);

        // adjust height
        consoleheight = BETWEEN(0, consoleheight + CONSOLESPEED * consoledirection, CONSOLEHEIGHT);

        // draw tiled background and bottom edge
        C_DrawBackground(consoleheight);

        // draw title and version
        C_DrawText(SCREENWIDTH - C_TextWidth(PACKAGE_VERSIONSTRING) - CONSOLETEXTX,
            CONSOLEHEIGHT - 15, PACKAGE_VERSIONSTRING);

        // draw console text
        start = MAX(0, consolestrings - 10);
        for (i = start; i < consolestrings; ++i)
        {
            int pos = i - start;

            if (consolestrings < 10)
                pos += 10 - consolestrings;
            C_DrawText(CONSOLETEXTX, CONSOLETEXTY + CONSOLELINEHEIGHT * pos, consolestring[i]);
        }

        // draw input text to left of caret
        for (i = 0; i < caretpos; ++i)
            left[i] = consoleinput[i];
        left[i] = 0;
        C_DrawText(CONSOLETEXTX, CONSOLEHEIGHT - 15, left);

        // draw input text to right of caret
        for (i = 0; (unsigned int)i < strlen(consoleinput) - caretpos; ++i)
            right[i] = consoleinput[i + caretpos];
        right[i] = 0;
        C_DrawText(CONSOLETEXTX + C_TextWidth(left) + 3, CONSOLEHEIGHT - 15, right);

        // draw caret
        if (carettics++ == CARETTICS)
        {
            carettics = 0;
            showcaret = !showcaret;
        }
        if (showcaret)
            V_DrawBigPatch(CONSOLETEXTX + C_TextWidth(left), consoleheight - 15, 0, caret);
    }
}

boolean C_Responder(event_t *ev)
{
    if (!consoleheight)
        return false;

    if (ev->type == ev_keydown)
    {
        int             key = ev->data1;
        int             ch = ev->data2;
        size_t          i;

#ifdef SDL20
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
                if (strlen(consoleinput))
                {
                    boolean     validcommand = false;

                    // add input to output
                    C_AddConsoleString(consoleinput);

                    // process input
                    i = 0;
                    while (consolecommands[i].command[0])
                    {
                        if (!strcasecmp(consoleinput, consolecommands[i].command))
                        {
                            validcommand = true;
                            if (consolecommands[i].cheat)
                                M_StringCopy(consolecheat, consoleinput, 255);
                            else
                                consolecommands[i].func();
                            break;
                        }
                        ++i;
                    }

                    // display error if invalid
                    if (!validcommand)
                    {
                        static char     error[1024];

                        M_snprintf(error, 1024, "Unknown command \"%s\"", consoleinput);
                        C_AddConsoleString(error);
                    }

                    // clear input
                    consoleinput[0] = 0;
                    caretpos = 0;

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

            default:
                if (modstate & KMOD_SHIFT)
                    ch = toupper(ch);
                if (ch >= ' ' && ch < '~' && ch != '`'
                    && C_TextWidth(consoleinput) + (ch == ' ' ? SPACEWIDTH :
                    consolefont[ch - CONSOLEFONTSTART]->width) <= CONSOLEINPUTPIXELWIDTH
                    && !(modstate & (KMOD_ALT | KMOD_CTRL)))
                {
                    consoleinput[strlen(consoleinput) + 1] = '\0';
                    for (i = strlen(consoleinput); i > (unsigned int)caretpos; --i)
                        consoleinput[i] = consoleinput[i - 1];
                    consoleinput[caretpos++] = ch;
                    carettics = 0;
                    showcaret = true;
                }
        }
    }
    return true;
}

void C_CmdList(void)
{
    int i = 0;

    while (consolecommands[i].command[0])
    {
        if (!consolecommands[i].cheat)
        {
            static char     buffer[1024];

            M_snprintf(buffer, 1024, "    %s", consolecommands[i].command);
            C_AddConsoleString(buffer);
        }
        ++i;
    }
}

void C_Quit(void)
{
    I_Quit(true);
}
