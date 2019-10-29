/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

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

#if !defined(__C_CONSOLE_H__)
#define __C_CONSOLE_H__

#include "doomdef.h"
#include "doomtype.h"
#include "d_event.h"

#define CONSOLESTRINGSMAX       256

#define CONSOLEFONTSTART        ' '
#define CONSOLEFONTEND          '~'
#define CONSOLEFONTSIZE         (CONSOLEFONTEND - CONSOLEFONTSTART + 1)

#define NOBOLDCOLOR             -1
#define NOBACKGROUNDCOLOR       -1

#define CONSOLEWIDTH            SCREENWIDTH
#define CONSOLEHEIGHT           (gamestate != GS_TITLESCREEN ? (SCREENHEIGHT - SBARHEIGHT) / 2 : SCREENHEIGHT - 5)

#define CONSOLELINES            (gamestate != GS_TITLESCREEN ? 11 : 27)
#define CONSOLETEXTX            10
#define CONSOLETEXTY            8
#define CONSOLETEXTMAXLENGTH    1024
#define CONSOLELINEHEIGHT       14

#define CONSOLESCROLLBARWIDTH   4
#define CONSOLESCROLLBARHEIGHT  (gamestate != GS_TITLESCREEN ? 136 : 363)
#define CONSOLESCROLLBARX       (CONSOLEWIDTH - CONSOLETEXTX - CONSOLESCROLLBARWIDTH)
#define CONSOLESCROLLBARY       (CONSOLETEXTY + 1)

#define CONSOLETEXTPIXELWIDTH   (CONSOLEWIDTH - CONSOLETEXTX * 2 - (scrollbardrawn ? CONSOLESCROLLBARWIDTH + CONSOLETEXTX : 0))

#define CONSOLEINPUTPIXELWIDTH  (CONSOLEWIDTH - CONSOLETEXTX - brandwidth - 2)

#define CONSOLETOP              0

#define DIVIDERSTRING           "==================================================================================================="

#define EMPTYVALUE              "\"\""

#define stringize(x)            #x

#if defined(_WIN32)
#define SDL_FILENAME            "SDL2.dll"
#define SDL_MIXER_FILENAME      "SDL2_mixer.dll"
#define SDL_IMAGE_FILENAME      "SDL2_image.dll"
#else
#define SDL_FILENAME            "SDL2"
#define SDL_MIXER_FILENAME      "SDL2_mixer"
#define SDL_IMAGE_FILENAME      "SDL2_image"
#endif

typedef enum
{
    inputstring,
    outputstring,
    dividerstring,
    warningstring,
    playermessagestring,
    obituarystring,
    headerstring,
    STRINGTYPES
} stringtype_t;

typedef enum
{
    bindlistheader,
    cmdlistheader,
    cvarlistheader,
    maplistheader,
    mapstatsheader,
    playerstatsheader,
    thinglistheader
} headertype_t;

typedef struct
{
    char                string[1024];
    unsigned int        count;
    stringtype_t        stringtype;
    headertype_t        headertype;
    int                 tabs[8];
    unsigned int        tics;
} console_t;

extern console_t        *console;

extern dboolean         consoleactive;
extern int              consoleheight;
extern int              consoledirection;

extern int              consolestrings;
extern size_t           consolestringsmax;

extern char             consolecheat[255];
extern char             consolecheatparm[3];
extern char             consolecmdparm[255];

extern dboolean         forceconsoleblurredraw;

extern dboolean         scrollbardrawn;

typedef struct
{
    char                *input;
    int                 caretpos;
    int                 selectstart;
    int                 selectend;
} undohistory_t;

typedef struct
{
    char                char1;
    char                char2;
    int                 adjust;
} kern_t;

extern const kern_t     kern[];
extern const kern_t     altkern[];

typedef struct
{
    char                text[255];
    int                 game;
} autocomplete_t;

extern autocomplete_t   autocompletelist[];

void C_Input(const char *string, ...);
void C_InputNoRepeat(const char *string, ...);
void C_IntCVAROutput(char *cvar, int value);
void C_PctCVAROutput(char *cvar, int value);
void C_StrCVAROutput(char *cvar, char *string);
void C_Output(const char *string, ...);
void C_OutputNoRepeat(const char *string, ...);
void C_TabbedOutput(const int tabs[8], const char *string, ...);
void C_Header(const headertype_t headertype);
void C_Warning(const char *string, ...);
void C_PlayerMessage(const char *string, ...);
void C_Obituary(const char *string, ...);
void C_AddConsoleDivider(void);
int C_TextWidth(const char *text, const dboolean formatting, const dboolean kerning);
void C_Init(void);
void C_ShowConsole(void);
void C_HideConsole(void);
void C_HideConsoleFast(void);
void C_Drawer(void);
dboolean C_ExecuteInputString(const char *input);
dboolean C_ValidateInput(const char *input);
dboolean C_Responder(event_t *ev);
void C_PrintCompileDate(void);
void C_PrintSDLVersions(void);
void C_UpdateFPS(void);
char *C_GetTimeStamp(unsigned int tics);

#endif
