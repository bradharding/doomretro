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

#if !defined(__C_CMDS__)
#define __C_CMDS__

#include "doomtype.h"

typedef enum
{
    keyboard = 1,
    mouse    = 2,
    gamepad  = 3,
    invalid  = 4
} controltype_t;

typedef struct
{
    char                *control;
    controltype_t       type;
    int                 value;
} control_t;

extern control_t        controls[];

typedef struct
{
    char                *action;
    void                *keyboard1;
    void                *keyboard2;
    void                *mouse;
    void                *gamepad;
} action_t;

extern action_t         actions[];

typedef enum
{
    CT_CMD   = 1,
    CT_CVAR  = 2,
    CT_CHEAT = 3
} cmdtype_t;

typedef enum
{
    CF_NONE            =   0,
    CF_BOOLEAN         =   1,
    CF_FLOAT           =   2,
    CF_INTEGER         =   4,
    CF_PERCENT         =   8,
    CF_POSITION        =  16,
    CF_READONLY        =  32,
    CF_SIZE            =  64,
    CF_STRING          = 128,
    CF_TIME            = 256,
} cmdflags_t;

typedef struct
{
    char                *name;
    boolean             (*condition)(char *cmd, char *parm1, char *parm2);
    void                (*function)(char *cmd, char *parm1, char *parm2);
    int                 parameters;
    cmdtype_t           type;
    cmdflags_t          flags;
    void                *variable;
    int                 aliases;
    int                 minimumvalue;
    int                 maximumvalue;
    int                 defaultvalue;
    char                *format;
    char                *description;
} consolecmd_t;

extern consolecmd_t     consolecmds[];
extern int              numconsolecmds;

#endif
