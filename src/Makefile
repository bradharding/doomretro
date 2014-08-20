#
#========================================================================
#
#  DOOM RETRO
#  The classic, refined DOOM source port. For Windows PC.
#  Copyright (C) 2013-2014 Brad Harding.
#
#  This file is part of DOOM RETRO.
#
#  DOOM RETRO is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  DOOM RETRO is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.
#
#========================================================================
#

CFLAGS = -m32 -Wall $(shell sdl-config --cflags)
LDFLAGS = $(shell sdl-config --libs) -lSDL_mixer

SRCS =             \
    am_map.c       \
    d_items.c      \
    d_iwad.c       \
    d_main.c       \
    d_net.c        \
    deh_ammo.c     \
    deh_cheat.c    \
    deh_doom.c     \
    deh_frame.c    \
    deh_io.c       \
    deh_main.c     \
    deh_mapping.c  \
    deh_main.c     \
    deh_ptr.c      \
    deh_sound.c    \
    deh_str.c      \
    deh_text.c     \
    deh_thing.c    \
    deh_weapon.c   \
    doomstat.c     \
    dstrings.c     \
    f_finale.c     \
    f_wipe.c       \
    g_game.c       \
    hu_lib.c       \
    hu_stuff.c     \
    i_gamepad.c    \
    i_main.c       \
    i_sdlmusic.c   \
    i_sdlsound.c   \
    i_system.c     \
    i_timer.c      \
    i_tinttab.c    \
    i_video.c      \
    info.c         \
    m_argv.c       \
    m_bbox.c       \
    m_cheat.c      \
    m_config.c     \
    m_fixed.c      \
    m_menu.c       \
    m_misc.c       \
    m_random.c     \
    memio.c        \
    midifile.c     \
    mus2mid.c      \
    p_ceilng.c     \
    p_doors.c      \
    p_enemy.c      \
    p_fix.c        \
    p_floor.c      \
    p_inter.c      \
    p_lights.c     \
    p_map.c        \
    p_maputl.c     \
    p_mobj.c       \
    p_plats.c      \
    p_pspr.c       \
    p_saveg.c      \
    p_setup.c      \
    p_sight.c      \
    p_spec.c       \
    p_switch.c     \
    p_telept.c     \
    p_tick.c       \
    p_user.c       \
    r_bsp.c        \
    r_data.c       \
    r_draw.c       \
    r_main.c       \
    r_plane.c      \
    r_segs.c       \
    r_sky.c        \
    r_things.c     \
    s_sound.c      \
    sha1.c         \
    sounds.c       \
    st_lib.c       \
    st_stuff.c     \
    tables.c       \
    v_data.c       \
    v_video.c      \
    w_file.c       \
    w_file_stdc.c  \
    w_merge.c      \
    w_wad.c        \
    wi_stuff.c     \
    z_zone.c

OBJS = $(patsubst %.c,%.o,$(SRCS))

doomretro : $(OBJS)
    $(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o : %.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f $(OBJS) doomretro
