/*
========================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.
Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

For a complete list of credits, see the accompanying AUTHORS file.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __M_ARGV__
#define __M_ARGV__

//
// MISC
//
extern int      myargc;
extern char     **myargv;

// Returns the position of the given parameter
// in the arg list (0 if not found).
int M_CheckParm(char *check);

// Same as M_CheckParm, but checks that num_args arguments are available
// following the specified argument.
int M_CheckParmWithArgs(char *check, int num_args);
int M_CheckParmsWithArgs(char *check1, char *check2, int num_args);

void M_FindResponseFile(void);

#endif
