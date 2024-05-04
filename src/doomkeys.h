/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ASCII (uppercased).
//
#define KEY_TAB         0x09
#define KEY_ENTER       0x0D
#define KEY_ESCAPE      0x1B
#define KEY_LEFTARROW   0xAC
#define KEY_UPARROW     0xAD
#define KEY_RIGHTARROW  0xAE
#define KEY_DOWNARROW   0xAF
#define KEY_F1          (0x80 + 0x3B)
#define KEY_F2          (0x80 + 0x3C)
#define KEY_F3          (0x80 + 0x3D)
#define KEY_F4          (0x80 + 0x3E)
#define KEY_F5          (0x80 + 0x3F)
#define KEY_F6          (0x80 + 0x40)
#define KEY_F7          (0x80 + 0x41)
#define KEY_F8          (0x80 + 0x42)
#define KEY_F9          (0x80 + 0x43)
#define KEY_F10         (0x80 + 0x44)
#define KEY_F11         (0x80 + 0x57)
#define KEY_F12         (0x80 + 0x58)

#define KEY_BACKSPACE   0x7F
#define KEY_PAUSE       0xFF

#define KEY_CTRL        (0x80 + 0x1D)
#define KEY_SHIFT       (0x80 + 0x36)
#define KEY_ALT         (0x80 + 0x38)

// new keys:
#define KEY_NUMLOCK     (0x80 + 0x39)
#define KEY_CAPSLOCK    (0x80 + 0x3A)
#define KEY_SCROLLLOCK  (0x80 + 0x46)

#define KEY_HOME        (0x80 + 0x47)
#define KEY_PAGEUP      (0x80 + 0x49)
#define KEY_END         (0x80 + 0x4F)
#define KEY_PAGEDOWN    (0x80 + 0x51)
#define KEY_INSERT      (0x80 + 0x52)
#define KEY_PRINTSCREEN (0x80 + 0x59)
#define KEY_DELETE      (0x80 + 0x60)

#define KEYP_0          (0x80 + 0x80)
#define KEYP_1          (0x80 + 0x81)
#define KEYP_2          (0x80 + 0x82)
#define KEYP_3          (0x80 + 0x83)
#define KEYP_4          (0x80 + 0x84)
#define KEYP_5          (0x80 + 0x85)
#define KEYP_6          (0x80 + 0x86)
#define KEYP_7          (0x80 + 0x87)
#define KEYP_8          (0x80 + 0x88)
#define KEYP_9          (0x80 + 0x89)
