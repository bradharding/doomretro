/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#pragma once

#include "d_event.h"
#include "doomdef.h"
#include "doomtype.h"
#include "hu_lib.h"
#include "r_defs.h"

#define CONSOLESTRINGSMAX                   1024

#define CONSOLEFONTSTART                    32
#define CONSOLEFONTEND                      255
#define CONSOLEFONTSIZE                     (CONSOLEFONTEND - CONSOLEFONTSTART + 1)

#define CONSOLEAUTOMAPBEVELCOLOR            6
#define CONSOLEBACKCOLOR                    12
#define CONSOLEBEVELCOLOR                   0
#define CONSOLEBOLDCOLOR                    80
#define CONSOLEBOLDITALICSCOLOR             93
#define CONSOLEBRANDINGCOLOR1               176
#define CONSOLEBRANDINGCOLOR2               182
#define CONSOLECARETCOLOR                   4
#define CONSOLEDIVIDERCOLOR                 100
#define CONSOLEINPUTCOLOR                   4
#define CONSOLEOUTPUTCOLOR                  88
#define CONSOLEOVERLAYWARNINGCOLOR          176
#define CONSOLEPLAYERMESSAGECOLOR           161
#define CONSOLESCROLLBARFACECOLOR           96
#define CONSOLESCROLLBARTRACKCOLOR          100
#define CONSOLESELECTEDINPUTBACKGROUNDCOLOR 100
#define CONSOLESELECTEDINPUTCOLOR           4
#define CONSOLETIMESTAMPCOLOR               161
#define CONSOLEWARNINGBOLDCOLOR             176
#define CONSOLEWARNINGCOLOR                 180

#define NOBOLDCOLOR                         -1
#define NOBACKGROUNDCOLOR                   -1

#define CONSOLEDOWNSIZE                     28
#define CONSOLEUPSIZE                       12

#define CONSOLEHEIGHT                       ((gamestate == GS_TITLESCREEN ? SCREENHEIGHT : SCREENHEIGHT / 2) - 5)

#define CONSOLELINES                        (gamestate == GS_TITLESCREEN ? 27 : 13)
#define CONSOLETEXTX                        (vid_widescreen ? MAX(MAXWIDESCREENDELTA - 18, 10) : 10)
#define CONSOLETEXTY                        8
#define CONSOLETEXTMAXLENGTH                1024
#define CONSOLELINEHEIGHT                   14
#define CONSOLEBLANKLINES                   12

#define CONSOLESCROLLBARWIDTH               5
#define CONSOLESCROLLBARHEIGHT              (CONSOLEHEIGHT - (gamestate == GS_TITLESCREEN ? 26 : 22))
#define CONSOLESCROLLBARX                   (SCREENWIDTH - CONSOLETEXTX - CONSOLESCROLLBARWIDTH)

#define CONSOLETEXTPIXELWIDTH               (SCREENWIDTH - CONSOLETEXTX * 2 - (scrollbardrawn ? CONSOLESCROLLBARWIDTH + 15 : 10))

#define CONSOLEINPUTX                       CONSOLETEXTX
#define CONSOLEINPUTY                       (CONSOLEHEIGHT - 16)

#define CONSOLEINPUTPIXELWIDTH              (SCREENWIDTH - CONSOLETEXTX * 2 - brandwidth - 2)

#define OVERLAYTEXTX                        (vid_widescreen ? MAXWIDESCREENDELTA - 18 : 14)
#define OVERLAYTEXTY                        (vid_widescreen && r_screensize == r_screensize_max ? 17 : 11)
#define OVERLAYLINEHEIGHT                   14
#define OVERLAYSPACING                      7
#define MAPOVERLAYTEXTX                     18
#define MAPOVERLAYTEXTY                     18

#define WARNINGWIDTH                        13

#define EMPTYVALUE                          "\"\""

#define stringize(text)                     #text

#define BOLDON                              "\x1A"
#define BOLDONCHAR                          '\x1A'
#define BOLDOFF                             "\x1B"
#define BOLDOFFCHAR                         '\x1B'
#define ITALICSON                           "\x1C"
#define ITALICSONCHAR                       '\x1C'
#define ITALICSOFF                          "\x1D"
#define ITALICSOFFCHAR                      '\x1D'
#define MONOSPACEDON                        "\x1E"
#define MONOSPACEDONCHAR                    '\x1E'
#define MONOSPACEDOFF                       "\x1F"
#define MONOSPACEDOFFCHAR                   '\x1F'

#define BOLD(text)                          BOLDON text BOLDOFF
#define BOLDER(text)                        BOLDON BOLDON text BOLDOFF BOLDOFF
#define ITALICS(text)                       ITALICSON text ITALICSOFF
#define BOLDITALICS(text)                   ITALICS(BOLD(text))
#define MONOSPACED(text)                    MONOSPACEDON text MONOSPACEDOFF

#if defined(_WIN32)
#define SDL_FILENAME                        "SDL2.dll"
#define SDL_MIXER_FILENAME                  "SDL2_mixer.dll"
#define SDL_IMAGE_FILENAME                  "SDL2_image.dll"
#elif defined(__APPLE__)
#define SDL_FILENAME                        "SDL2"
#define SDL_MIXER_FILENAME                  "SDL2_mixer"
#define SDL_IMAGE_FILENAME                  "SDL2_image"
#else
#define SDL_FILENAME                        "SDL2.so"
#define SDL_MIXER_FILENAME                  "SDL2_mixer.so"
#define SDL_IMAGE_FILENAME                  "SDL2_image.so"
#endif

#define MAXTABS                             10

#define BINDLISTHEADER                      "CONTROL\t+ACTION/COMMAND(S)"
#define CMDLISTHEADER                       "CCMD\tDESCRIPTION"
#define CVARLISTHEADER                      "CVAR\tVALUE\tDESCRIPTION"
#define MAPLISTHEADER                       "\tMAP\tTITLE\tWAD"
#define MAPSTATSHEADER                      "STAT\tVALUE"
#define PLAYERSTATSHEADER                   "STAT\tCURRENT MAP\tTOTAL"
#define THINGLISTHEADER                     "\tTHING\tPOSITION\tANGLE\tFLAGS"

typedef enum
{
    inputstring,
    cheatstring,
    outputstring,
    dividerstring,
    warningstring,
    playerwarningstring,
    playermessagestring,
    headerstring,
    STRINGTYPES
} stringtype_t;

typedef struct
{
    char            string[1024];
    int             count;
    stringtype_t    stringtype;
    int             wrap;
    int             indent;
    patch_t         *header;
    int             tabs[MAXTABS];
    int             tics;
    char            timestamp[9];
} console_t;

typedef struct
{
    char            *input;
    int             caretpos;
    int             selectstart;
    int             selectend;
} undohistory_t;

typedef struct
{
    const char      char1;
    const char      char2;
    const int       adjust;
} kern_t;

typedef struct
{
    char            text[255];
    const int       game;
} autocomplete_t;

extern patch_t          *consolefont[CONSOLEFONTSIZE];
extern patch_t          *degree;
extern patch_t          *lsquote;
extern patch_t          *ldquote;
extern patch_t          *unknownchar;
extern patch_t          *altunderscores;

extern patch_t          *bindlist;
extern patch_t          *cmdlist;
extern patch_t          *cvarlist;
extern patch_t          *maplist;
extern patch_t          *mapstats;
extern patch_t          *playerstats;
extern patch_t          *thinglist;

extern console_t        *console;

extern bool             consoleactive;
extern int              consoleheight;
extern int              consoledirection;

extern char             consoleinput[255];
extern int              numconsolestrings;
extern size_t           consolestringsmax;

extern int              caretpos;
extern int              selectstart;
extern int              selectend;

extern bool             pathoverlay;

extern char             consolecheat[255];
extern char             consolecheatparm[3];
extern char             consolecmdparm[255];

extern int              consolebrandingcolor1;
extern int              consolebrandingcolor2;

extern bool             scrollbardrawn;

extern const kern_t     kern[];
extern const kern_t     altkern[];

extern autocomplete_t   autocompletelist[];

void C_Input(const char *string, ...);
void C_Cheat(const char *string);
void C_IntegerCVAROutput(const char *cvar, const int value);
void C_IntegerCVAROutputNoRepeat(const char *cvar, const int value);
void C_PercentCVAROutput(const char *cvar, const int value);
void C_StringCVAROutput(const char *cvar, const char *string);
void C_Output(const char *string, ...);
void C_TabbedOutput(const int tabs[MAXTABS], const char *string, ...);
void C_Header(const int tabs[MAXTABS], patch_t *header, const char *string);
void C_Warning(const int minwarninglevel, const char *string, ...);
void C_PlayerMessage(const char *string, ...);
void C_PlayerObituary(const char *string, ...);
void C_PlayerWarning(const char *string, ...);
void C_ResetWrappedLines(void);
void C_AddConsoleDivider(void);
void C_ClearConsole(void);
void C_Init(void);
void C_ShowConsole(bool reset);
void C_HideConsole(void);
void C_HideConsoleFast(void);
void C_Drawer(void);
bool C_ExecuteInputString(const char *input);
bool C_ValidateInput(char *input);
bool C_Responder(event_t *ev);
void C_PrintSDLVersions(void);
void C_UpdateFPSOverlay(void);
void C_UpdateTimerOverlay(void);
void C_UpdatePathOverlay(void);
void C_UpdatePlayerStatsOverlay(void);
void C_UpdatePlayerPositionOverlay(void);
char *C_CreateTimeStamp(const int index);
int C_TextWidth(const char *text, const bool formatting, const bool kerning);

#if defined(_WIN32)
void C_PrintCompileDate(void);
#endif
