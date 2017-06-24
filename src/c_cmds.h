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

#if !defined(__C_CMDS_H__)
#define __C_CMDS_H__

#include "doomtype.h"

#define MAXALIASES          256

#define BINDLISTTITLE       "CONTROL\t+ACTION"
#define CMDLISTTITLE        "CCMD\tDESCRIPTION"
#define CVARLISTTITLE       "CVAR\tDEFAULT\tDESCRIPTION"
#define MAPLISTTITLE        "MAP\tNAME\tWAD"
#define PLAYERSTATSTITLE    "STAT\tCURRENT MAP\tTOTAL"
#define THINGLISTTITLE      "THING\tPOSITION"

typedef enum controltype_e
{
    keyboardcontrol = 1,
    mousecontrol    = 2,
    gamepadcontrol  = 3,
    invalidcontrol  = 4
} controltype_t;

typedef struct control_s
{
    char            *control;
    controltype_t   type;
    int             value;
} control_t;

typedef struct action_s
{
    char    *action;
    void    *keyboard1;
    void    *keyboard2;
    void    *mouse1;
    void    *gamepad1;
    void    *gamepad2;
} action_t;

typedef enum cmdtype_e
{
    CT_CMD   = 1,
    CT_CVAR  = 2,
    CT_CHEAT = 3
} cmdtype_t;

enum cmdflag_e
{
    CF_NONE     =   0,
    CF_BOOLEAN  =   1,
    CF_FLOAT    =   2,
    CF_INTEGER  =   4,
    CF_PERCENT  =   8,
    CF_POSITION =  16,
    CF_READONLY =  32,
    CF_SIZE     =  64,
    CF_STRING   = 128,
    CF_TIME     = 256
};

typedef struct consolecmd_s
{
    char        *name;
    char        *alternate;
    dboolean    (*func1)(char *cmd, char *parms);
    void        (*func2)(char *cmd, char *parms);
    int         parameters;
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

typedef struct alias_s
{
    char    name[128];
    char    string[128];
} alias_t;

extern action_t     actions[];
extern control_t    controls[];
extern consolecmd_t consolecmds[];
extern alias_t      aliases[];

dboolean C_ExecuteAlias(char *alias);

#endif
