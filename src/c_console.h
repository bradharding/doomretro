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

#if !defined(__C_CONSOLE_H__)
#define __C_CONSOLE_H__

#include "doomtype.h"
#include "d_event.h"

#define CONSOLEFONTSTART    ' '
#define CONSOLEFONTEND      '~'
#define CONSOLEFONTSIZE     (CONSOLEFONTEND - CONSOLEFONTSTART + 1)

#define NOBOLDCOLOR         -1
#define NOBACKGROUNDCOLOR   -1

#define CONSOLEWIDTH        SCREENWIDTH
#define CONSOLEHEIGHT       (gamestate != GS_TITLESCREEN ? (SCREENHEIGHT - SBARHEIGHT) / 2 : SCREENHEIGHT - 5)

#define CONSOLETOP          0

#define DIVIDERSTRING       "==================================================" \
                            "================================================="

#define EMPTYVALUE          "\"\""

#define stringize(x)        #x

typedef enum stringtype_e
{
    inputstring,
    outputstring,
    dividerstring,
    titlestring,
    warningstring,
    playermessagestring,
    obituarystring,
    STRINGTYPES
} stringtype_t;

typedef struct
{
    char            string[1024];
    stringtype_t    type;
    int             tabs[8];
    char            timestamp[9];
} console_t;

extern console_t    *console;

extern dboolean     consoleactive;
extern int          consoleheight;
extern int          consoledirection;

extern int          consolestrings;

extern char         consolecheat[255];
extern char         consolecheatparm[3];
extern char         consolecmdparm[255];

extern dboolean     forceconsoleblurredraw;

typedef struct
{
    char            *input;
    int             caretpos;
    int             selectstart;
    int             selectend;
} undohistory_t;

void C_Print(const stringtype_t type, const char *string, ...);
void C_Input(const char *string, ...);
void C_IntCVAROutput(const char *cvar, const int value);
void C_PctCVAROutput(const char *cvar, const int value);
void C_StrCVAROutput(const char *cvar, const char *string);
void C_Output(const char *string, ...);
void C_TabbedOutput(const int tabs[8], const char *string, ...);
void C_Warning(const char *string, ...);
void C_PlayerMessage(const char *string, ...);
void C_Obituary(const char *string, ...);
void C_AddConsoleDivider(void);
void C_Init(void);
void C_ShowConsole(void);
void C_HideConsole(void);
void C_HideConsoleFast(void);
void C_Drawer(void);
dboolean C_ValidateInput(const char *input);
dboolean C_Responder(event_t *ev);
void C_PrintCompileDate(void);
void C_PrintSDLVersions(void);
void C_StripQuotes(char *string);
void C_UpdateFPS(void);

#endif
