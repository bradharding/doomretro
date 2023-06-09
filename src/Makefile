#
#============================================================================
#
#                                 DOOM Retro
#           The classic, refined DOOM source port. For Windows PC.
#
#============================================================================
#
#    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
#    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.
#
#    This file is a part of DOOM Retro.
#
#    DOOM Retro is free software: you can redistribute it and/or modify it
#    under the terms of the GNU General Public License as published by the
#    Free Software Foundation, either version 3 of the license, or (at your
#    option) any later version.
#
#    DOOM Retro is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.
#
#    DOOM is a registered trademark of id Software LLC, a ZeniMax Media
#    company, in the US and/or other countries, and is used without
#    permission. All other trademarks are the property of their respective
#    holders. DOOM Retro is in no way affiliated with nor endorsed by
#    id Software.
#
#============================================================================
#

OPTFLAGS = -O2 -fsigned-char
CFLAGS = $(OPTFLAGS) -g `sdl2-config --cflags`
LDFLAGS = -g `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lm

# If you don't want to compile with X11 bindings,
# set the NOX11 variable, e.g. make NOX11=1
ifndef NOX11
CFLAGS += -DX11
LDFLAGS += -lX11
endif

SRCS := $(shell find . -name '*.c')

OBJS = $(patsubst %.c,%.o,$(SRCS))

doomretro : $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) doomretro
