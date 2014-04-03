/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

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

#include "i_system.h"
#include "mus2mid.h"
#include "m_misc.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "s_sound.h"
#include "z_zone.h"

#define MAXMIDLENGTH (96 * 1024)

static boolean  music_initialized = false;

// If this is true, this module initialized SDL sound and has the
// responsibility to shut it down
static boolean  sdl_was_initialized = false;

static boolean  musicpaused = false;
static int      current_music_volume;

// Shutdown music
static void I_SDL_ShutdownMusic(void)
{
    if (music_initialized)
    {
        Mix_HaltMusic();
        music_initialized = false;

        if (sdl_was_initialized)
        {
            Mix_CloseAudio();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            sdl_was_initialized = false;
        }
    }
}

static boolean SDLIsInitialized(void)
{
    int         freq, channels;
    Uint16      format;

    return (Mix_QuerySpec(&freq, &format, &channels) != 0);
}

// Initialize music subsystem
static boolean I_SDL_InitMusic(void)
{
    // If SDL_mixer is not initialized, we have to initialize it
    // and have the responsibility to shut it down later on.
    if (SDLIsInitialized())
        music_initialized = true;
    else
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
            I_Error("Unable to set up sound: %s", SDL_GetError());
        else if (Mix_OpenAudio(snd_samplerate, AUDIO_S16SYS, 2, 1024) < 0)
        {
            I_Error("Error initializing SDL_mixer: %s", Mix_GetError());
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }
        else
        {
            SDL_PauseAudio(0);

            sdl_was_initialized = true;
            music_initialized = true;
        }
    }

    return music_initialized;
}

//
// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
//
static void UpdateMusicVolume(void)
{
    Mix_VolumeMusic((current_music_volume * MIX_MAX_VOLUME) / 127 * musicpaused);
}

// Set music volume (0 - 127)
static void I_SDL_SetMusicVolume(int volume)
{
    // Internal state variable.
    current_music_volume = volume;

    UpdateMusicVolume();
}

// Start playing a mid
static void I_SDL_PlaySong(void *handle, int looping)
{
    Mix_Music *music = (Mix_Music *)handle;
    int loops;

    if (!music_initialized)
        return;

    if (handle == NULL)
        return;

    if (looping)
        loops = -1;
    else
        loops = 1;

    Mix_PlayMusic(music, loops);
}

static void I_SDL_PauseSong(void)
{
    if (!music_initialized)
        return;

    musicpaused = true;

    UpdateMusicVolume();
}

static void I_SDL_ResumeSong(void)
{
    if (!music_initialized)
        return;

    musicpaused = false;

    UpdateMusicVolume();
}

static void I_SDL_StopSong(void)
{
    if (!music_initialized)
        return;

    Mix_HaltMusic();
}

static void I_SDL_UnRegisterSong(void *handle)
{
    Mix_Music *music = (Mix_Music *) handle;

    if (!music_initialized)
        return;

    if (handle == NULL)
        return;

    Mix_FreeMusic(music);
}

// Determine whether memory block is a .mid file
static boolean IsMid(byte *mem, int len)
{
    return (len > 4 && !memcmp(mem, "MThd", 4));
}

static boolean ConvertMus(byte *musdata, int len, char *filename)
{
    MEMFILE     *instream;
    MEMFILE     *outstream;
    void        *outbuf;
    size_t      outbuf_len;
    int         result;

    instream = mem_fopen_read(musdata, len);
    outstream = mem_fopen_write();

    result = mus2mid(instream, outstream);

    if (result == 0)
    {
        mem_get_buf(outstream, &outbuf, &outbuf_len);

        M_WriteFile(filename, outbuf, outbuf_len);
    }

    mem_fclose(instream);
    mem_fclose(outstream);

    return result;
}

static void *I_SDL_RegisterSong(void *data, int len)
{
    char        *filename;
    Mix_Music   *music;

    if (!music_initialized)
        return NULL;

    // MUS files begin with "MUS"
    // Reject anything which doesn't have this signature
    filename = M_TempFile("doom.mid");

    if (IsMid(data, len) && len < MAXMIDLENGTH)
        M_WriteFile(filename, data, len);
    else
        // Assume a MUS file and try to convert
        ConvertMus(data, len, filename);

    // Load the MIDI

    music = Mix_LoadMUS(filename);

    if (music == NULL)
        // Failed to load
        I_Error("Error loading midi: %s", Mix_GetError());

    // remove file now
    remove(filename);

    Z_Free(filename);

    return music;
}

// Is the song playing?
static boolean I_SDL_MusicIsPlaying(void)
{
    if (!music_initialized)
        return false;

    return Mix_PlayingMusic();
}

static snddevice_t music_sdl_devices[] =
{
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32
};

music_module_t music_sdl_module =
{
    music_sdl_devices,
    arrlen(music_sdl_devices),
    I_SDL_InitMusic,
    I_SDL_ShutdownMusic,
    I_SDL_SetMusicVolume,
    I_SDL_PauseSong,
    I_SDL_ResumeSong,
    I_SDL_RegisterSong,
    I_SDL_UnRegisterSong,
    I_SDL_PlaySong,
    I_SDL_StopSong,
    I_SDL_MusicIsPlaying
};
