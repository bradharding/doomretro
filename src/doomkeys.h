/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#ifndef __DOOMKEYS__
#define __DOOMKEYS__

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEY_RIGHTARROW  0xae
#define KEY_LEFTARROW   0xac
#define KEY_UPARROW     0xad
#define KEY_DOWNARROW   0xaf
#define KEY_ESCAPE      0x1b
#define KEY_ENTER       0x0d
#define KEY_TAB         0x09
#define KEY_F1          (0x80 + 0x3b)
#define KEY_F2          (0x80 + 0x3c)
#define KEY_F3          (0x80 + 0x3d)
#define KEY_F4          (0x80 + 0x3e)
#define KEY_F5          (0x80 + 0x3f)
#define KEY_F6          (0x80 + 0x40)
#define KEY_F7          (0x80 + 0x41)
#define KEY_F8          (0x80 + 0x42)
#define KEY_F9          (0x80 + 0x43)
#define KEY_F10         (0x80 + 0x44)
#define KEY_F11         (0x80 + 0x57)
#define KEY_F12         (0x80 + 0x58)

#define KEY_BACKSPACE   0x7f
#define KEY_PAUSE       0xff

#define KEY_EQUALS      0x3d
#define KEY_MINUS       0x2d

#define KEY_RSHIFT      (0x80 + 0x36)
#define KEY_RCTRL       (0x80 + 0x1d)
#define KEY_RALT        (0x80 + 0x38)

#define KEY_LALT        KEY_RALT

// new keys:

#define KEY_NUMLOCK     (0x80 + 0x39)
#define KEY_CAPSLOCK    (0x80 + 0x3a)
#define KEY_SCRLCK      (0x80 + 0x46)

#define KEY_HOME        (0x80 + 0x47)
#define KEY_END         (0x80 + 0x4f)
#define KEY_PGUP        (0x80 + 0x49)
#define KEY_PGDN        (0x80 + 0x51)
#define KEY_INS         (0x80 + 0x52)
#define KEY_DEL         (0x80 + 0x60)

#define KEYP_0          0x100
#define KEYP_1          KEY_END
#define KEYP_2          KEY_DOWNARROW
#define KEYP_3          KEY_PGDN
#define KEYP_4          KEY_LEFTARROW
#define KEYP_5          0x105
#define KEYP_6          KEY_RIGHTARROW
#define KEYP_7          KEY_HOME
#define KEYP_8          KEY_UPARROW
#define KEYP_9          KEY_PGUP

#define KEYP_DIVIDE     '/'
#define KEYP_PLUS       KEY_EQUALS
#define KEYP_MINUS      KEY_MINUS
#define KEYP_MULTIPLY   '*'
#define KEYP_PERIOD     0x10a
#define KEYP_EQUALS     KEY_EQUALS
#define KEYP_ENTER      KEY_ENTER

#endif          // __DOOMKEYS__