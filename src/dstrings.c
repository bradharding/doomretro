/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#include "d_deh.h"

char **endmsg[] =
{
    // DOOM1
    &s_QUITMSG,
    &s_QUITMSG1,
    &s_QUITMSG2,
    &s_QUITMSG3,
    &s_QUITMSG4,
    &s_QUITMSG5,
    &s_QUITMSG6,
    &s_QUITMSG7,

    // QuitDOOM II messages
    &s_QUITMSG,
    &s_QUITMSG8,
    &s_QUITMSG9,
    &s_QUITMSG10,
    &s_QUITMSG11,
    &s_QUITMSG12,
    &s_QUITMSG13,
    &s_QUITMSG14
};

char *devendmsg[] =
{
    // UNUSED messages included in the source release
    "Fuck you, pussy!\nGet the fuck out!",
    "You quit now and I'll jizz\nin your cysthole!",
    "If you leave, I'll make\nthe lord drink my jizz.",
    "Hey, Ron! Can we say\n\"fuck\" in this game?",
    "I'd leave. This is just more\nmonsters and levels. What a load!",
    "Suck it down, asshole!\nYou're a fucking wimp!",
    "Don't quit now!\nWe're still spending your money!",

    // Internal debug. Different style, too.
    "THIS IS NO MESSAGE!\nPage intentionally left blank."
};
