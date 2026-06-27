/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <SDL3_mixer/SDL_mixer.h>

#include "c_console.h"
#include "doomstat.h"
#include "m_config.h"
#include "memio.h"
#include "mus2mid.h"
#include "s_sound.h"
#include "w_wad.h"

typedef struct
{
    byte    *data;
    int     size;
} registered_song_t;

bool        midimusictype;
bool        musmusictype;

#if defined(_WIN32)
bool        windowsmidi = false;
#endif

static bool music_initialized;
static MIX_Track *music_track;

int         current_music_volume = 0;

// Shutdown music
void I_ShutdownMusic(void)
{
    if (!music_initialized)
        return;

    if (mus_playing)
    {
        I_StopSong();
        I_UnregisterSong(mus_playing->handle);
    }

    if (music_track)
    {
        MIX_DestroyTrack(music_track);
        music_track = NULL;
    }

    music_initialized = false;
    I_ReleaseMixer();
}

// Initialize music subsystem
bool I_InitMusic(void)
{
#if defined(_WIN32)
    windowsmidi = false;
#endif

    if (!I_AcquireMixer())
        return false;

    if (!(music_track = MIX_CreateTrack(I_GetMixer())))
    {
        I_ReleaseMixer();
        return false;
    }

    music_initialized = true;
    I_SetMusicVolume(current_music_volume);

    return true;
}

// Set music volume (0 - 127)
void I_SetMusicVolume(const int volume)
{
    current_music_volume = volume;

    if (music_track)
        MIX_SetTrackGain(music_track, (float)current_music_volume / 127.0f);
}

// Start playing a mid
void I_PlaySong(void *handle, const bool looping)
{
    registered_song_t    *song = handle;
    SDL_IOStream         *io;

    if (!music_initialized || !music_track || !song)
        return;

    if (!(io = SDL_IOFromMem(song->data, song->size)))
        return;

    if (!MIX_SetTrackIOStream(music_track, io, true))
    {
        SDL_CloseIO(io);
        return;
    }

    MIX_SetTrackLoops(music_track, (looping ? -1 : 0));
    MIX_SetTrackGain(music_track, (float)current_music_volume / 127.0f);
    MIX_PlayTrack(music_track, 0);
}

void I_PauseSong(void)
{
    if (!music_initialized || !music_track)
        return;

    MIX_PauseTrack(music_track);
}

void I_ResumeSong(void)
{
    if (!music_initialized || !music_track)
        return;

    MIX_ResumeTrack(music_track);
}

void I_StopSong(void)
{
    if (!music_initialized || !music_track)
        return;

    MIX_StopTrack(music_track, 0);
}

void I_UnregisterSong(void *handle)
{
    registered_song_t    *song = handle;

    if (!song)
        return;

    free(song->data);
    free(song);
}

void *I_RegisterSong(void *data, int size)
{
    registered_song_t    *song;
    const byte           *src = data;
    size_t               outsize = (size_t)size;

    if (!music_initialized)
        return NULL;

    midimusictype = false;
    musmusictype = false;

    if (size >= 14)
    {
        if (!memcmp(data, "MThd", 4))
            midimusictype = true;
        else if (!memcmp(data, "MUS\x1A", 4))
        {
            MEMFILE *instream = mem_fopen_read(data, size);
            MEMFILE *outstream = mem_fopen_write();
            void    *outbuf = NULL;
            size_t  midlen = 0;

            musmusictype = true;

            if (!mus2mid(instream, outstream))
            {
                mem_fclose(instream);
                mem_fclose(outstream);
                return NULL;
            }

            mem_get_buf(outstream, &outbuf, &midlen);
            src = outbuf;
            outsize = midlen;
            midimusictype = true;

            mem_fclose(instream);
            mem_fclose(outstream);
        }
    }

    if (!(song = malloc(sizeof(*song))))
        return NULL;

    if (!(song->data = malloc(outsize)))
    {
        free(song);
        return NULL;
    }

    memcpy(song->data, src, outsize);
    song->size = (int)outsize;

    return song;
}
