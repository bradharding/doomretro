/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

// All important printed strings.
#include "d_englsh.h"

// Misc. other strings.
#define SAVEGAMENAME        "doomsav"

// Quit DOOM messages
// 8 per each game type
#define NUM_QUITMESSAGES    8

// Quit DOOM messages
#define QUITMSG1            "please don't leave, there's more\ndemons to toast!"
#define QUITMSG2            "let's beat it -- this is turning\ninto a bloodbath!"
#define QUITMSG3            "i wouldn't leave if i were you.\ndos is much worse."
#define QUITMSG4            "you're trying to say you like dos\nbetter than me, right?"
#define QUITMSG5            "don't leave yet -- there's a\ndemon around that corner!"
#define QUITMSG6            "ya know, next time you come in here\ni'm gonna toast ya."
#define QUITMSG7            "go ahead and leave. see if i care."

// Quit DOOM II messages
#define QUITMSG8            "you want to quit?\nthen, thou hast lost an eighth!"
#define QUITMSG9            "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!"
#define QUITMSG10           "get outta here and go back\nto your boring programs."
#define QUITMSG11           "if i were your boss, i'd \n deathmatch ya in a minute!"
#define QUITMSG12           "look, bud. you leave now\nand you forfeit your body count!"
#define QUITMSG13           "just leave. when you come\nback, i'll be waiting with a bat."
#define QUITMSG14           "you're lucky i don't smack\nyou for thinking about leaving."

// UNUSED messages included in the source release
#define DEVQUITMSG1         "Fuck you, pussy!\nGet the fuck out!"
#define DEVQUITMSG2         "You quit now and I'll\njizz in your cysthole!"
#define DEVQUITMSG3         "If you leave, I'll make\nthe lord drink my jizz."
#define DEVQUITMSG4         "Hey, Ron! Can we say\n\"fuck\" in this game?"
#define DEVQUITMSG5         "I'd leave. There's just more\nmonsters and maps. What a load!"
#define DEVQUITMSG6         "Suck it down, asshole!\nYou're a fucking wimp!"
#define DEVQUITMSG7         "Don't quit now!\nWe're still spending your money!"

// Internal debug. Different style, too.
#define DEVQUITMSG8         "THIS IS NO MESSAGE!\nPage intentionally left blank."

extern char **endmsg[];
extern char *devendmsg[];
