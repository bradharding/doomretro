CFLAGS = -m32 -Wall $(shell sdl-config --cflags)
LDFLAGS = $(shell sdl-config --libs) -lSDL_mixer

SRCS =             \
    am_map.c       \
    d_items.c      \
    d_iwad.c       \
    d_main.c       \
    d_net.c        \
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

