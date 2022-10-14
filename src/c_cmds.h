/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#pragma once

#include "doomtype.h"
#include "m_config.h"

#define MAXALIASES          256

#define DIVIDERSTRING       "===================================================================================================="

#define UNITSPERFOOT        16
#define FEETPERMETER        3.28084f
#define METERSPERKILOMETER  1000
#define FEETPERMILE         5280

typedef enum
{
    keyboardcontrol       = 1,
    mousecontrol          = 2,
    gamecontrollercontrol = 3
} controltype_t;

typedef struct
{
    char            *control;
    controltype_t   type;
    int             value;
} control_t;

typedef struct
{
    char    *action;
    bool    hideconsole;
    void    (*func)(void);
    void    *keyboard1;
    void    *keyboard2;
    void    *mouse1;
    void    *gamecontroller1;
    void    *gamecontroller2;
} action_t;

typedef enum
{
    CT_CCMD  = 1,
    CT_CVAR  = 2,
    CT_CHEAT = 3
} cmdtype_t;

enum
{
    CF_NONE         =    0,
    CF_BOOLEAN      =    1,
    CF_FLOAT        =    2,
    CF_INTEGER      =    4,
    CF_PERCENT      =    8,
    CF_STRING       =   16,
    CF_TIME         =   32,
    CF_OTHER        =   64,
    CF_READONLY     =  128,
    CF_STARTUPRESET =  256,
    CF_MAPRESET     =  512,
    CF_NEXTMAP      = 1024,
    CF_PISTOLSTART  = 2048
};

typedef struct
{
    char        *name;
    char        *altspelling;
    char        *alternate;
    bool        (*func1)(char *cmd, char *parms);
    void        (*func2)(char *cmd, char *parms);
    bool        parameters;
    cmdtype_t   type;
    int         flags;
    void        *variable;
    int         aliases;
    int         minimumvalue;
    int         maximumvalue;
    char        *format;
    char        *description;
    float       defaultnumber;
    char        *defaultstring;
} consolecmd_t;

typedef struct
{
    char    name[128];
    char    string[128];
} alias_t;

extern action_t         actions[];
extern const control_t  controls[];
extern consolecmd_t     consolecmds[];
extern alias_t          aliases[MAXALIASES];
extern bool             executingalias;
extern bool             healthcvar;
extern bool             resettingcvar;
extern bool             vanilla;
extern bool             togglingvanilla;
extern bool             massacre;
extern bool             nobindoutput;
extern bool             quitcmd;

void alias_cmd_func2(char *cmd, char *parms);
void bind_cmd_func2(char *cmd, char *parms);

char *C_LookupAliasFromValue(const int value, const valuealiastype_t valuealiastype);
int C_GetIndex(const char *cmd);
bool C_ExecuteAlias(const char *alias);
char *distancetraveled(uint64_t value, bool allowzero);
