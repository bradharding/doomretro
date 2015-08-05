#
#========================================================================
#
#                               DOOM RETRO
#         The classic, refined DOOM source port. For Windows PC.
#
#========================================================================
#
#  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
#  Copyright (C) 2013-2015 Brad Harding.
#
#  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
#  For a complete list of credits, see the accompanying AUTHORS file.
#
#  This file is part of DOOM RETRO.
#
#  DOOM RETRO is free software: you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  DOOM RETRO is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.
#
#  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
#  company, in the US and/or other countries and is used without
#  permission. All other trademarks are the property of their respective
#  holders. DOOM RETRO is in no way affiliated with nor endorsed by
#  id Software LLC.
#
#========================================================================
#

OPTFLAGS = -O2
CFLAGS = $(OPTFLAGS) -g `sdl2-config --cflags`
LDFLAGS = -g `sdl2-config --libs` -lSDL2_mixer -lm

SRCS := $(shell find . -name '*.c')

OBJS = $(patsubst %.c,%.o,$(SRCS))

doomretro : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) doomretro
