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
  Free Software Foundation, either version 3 of the License, or (at your
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

#include <string.h>

#include "SDL_mixer.h"

#include "c_console.h"
#include "i_winmusic.h"
#include "m_misc.h"
#include "memio.h"
#include "mus2mid.h"
#include "s_sound.h"
#include "version.h"

dboolean        midimusictype;
dboolean        musmusictype;

#if defined(_WIN32)
dboolean        windowsmidi = false;
#endif

static dboolean music_initialized;

int             current_music_volume;
static int      paused_midi_volume;

// Shutdown music
void I_ShutdownMusic(void)
{
    if (!music_initialized)
        return;

    music_initialized = false;

    if (mus_playing)
        I_UnregisterSong(mus_playing->handle);

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

#if defined(_WIN32)
    if (windowsmidi)
    {
        I_Windows_ShutdownMusic();
        windowsmidi = false;
    }
#endif
}

// Initialize music subsystem
dboolean I_InitMusic(void)
{
    int         freq = MIX_DEFAULT_FREQUENCY;
    int         channels;
    uint16_t    format;

    // If SDL_mixer is not initialized, we have to initialize it and have the responsibility to shut it down later on.
    if (!Mix_QuerySpec(&freq, &format, &channels))
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
            return false;

        if (Mix_OpenAudioDevice(SAMPLERATE, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS,
            CHUNKSIZE, DEFAULT_DEVICE, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE) < 0)
        {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return false;
        }
    }

    SDL_PauseAudio(0);

    music_initialized = true;

#if defined(_WIN32)
    windowsmidi = I_Windows_InitMusic();
#endif

    return music_initialized;
}

// Set music volume (0 - MIX_MAX_VOLUME)
void I_SetMusicVolume(int volume)
{
    // Internal state variable.
    current_music_volume = volume;

#if defined(_WIN32)
    if (midimusictype && windowsmidi)
        I_Windows_SetMusicVolume(current_music_volume);
    else
#endif
        Mix_VolumeMusic(current_music_volume / 2);
}

// Start playing a mid
void I_PlaySong(void *handle, dboolean looping)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (midimusictype && windowsmidi)
        I_Windows_PlaySong(looping);
    else if (handle)
#endif
        Mix_PlayMusic(handle, (looping ? -1 : 1));
}

void I_PauseSong(void)
{
    if (!music_initialized)
        return;

    if (midimusictype)
    {
        paused_midi_volume = current_music_volume;

#if defined(_WIN32)
        I_Windows_SetMusicVolume(0);
#else
        Mix_VolumeMusic(0);
#endif
    }
    else
        Mix_PauseMusic();
}

void I_ResumeSong(void)
{
    if (!music_initialized)
        return;

    if (midimusictype)
#if defined(_WIN32)
        I_Windows_SetMusicVolume(paused_midi_volume);
#else
        Mix_VolumeMusic(paused_midi_volume);
#endif
    else
        Mix_ResumeMusic();
}

void I_StopSong(void)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (windowsmidi)
        I_Windows_StopSong();
#endif

    Mix_HaltMusic();
}

void I_UnregisterSong(void *handle)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (windowsmidi)
        I_Windows_UnregisterSong();
    else
#endif
        if (handle)
            Mix_FreeMusic(handle);
}

void *I_RegisterSong(void *data, int size)
{
    if (!music_initialized)
        return NULL;
    else
    {
        Mix_Music   *music = NULL;
        SDL_RWops   *rwops = NULL;

        midimusictype = false;
        musmusictype = false;

        // Check for MIDI or MUS format first:
        if (size >= 14)
        {
            if (!memcmp(data, "MThd", 4))           // is it a MIDI?
                midimusictype = true;
            else if (!memcmp(data, "MUS\x1A", 4))   // is it a MUS?
            {
                MEMFILE *instream = mem_fopen_read(data, size);
                MEMFILE *outstream = mem_fopen_write();

                musmusictype = true;

                if (!(mus2mid(instream, outstream)))
                {
                    void    *outbuf;
                    byte    *mid;
                    size_t  midlen;

                    mem_get_buf(outstream, &outbuf, &midlen);

                    mid = malloc(midlen);
                    memcpy(mid, outbuf, midlen);
                    data = mid;
                    size = (int)midlen;
                }

                mem_fclose(instream);
                mem_fclose(outstream);

                midimusictype = true;               // now it's a MIDI
            }
        }

#if defined(_WIN32)
        if (midimusictype && windowsmidi)
        {
            I_Windows_RegisterSong(data, size);
            return NULL;
        }
#endif

        if ((rwops = SDL_RWFromMem(data, size)))
            music = Mix_LoadMUS_RW(rwops, SDL_FALSE);

        return music;
    }
}
