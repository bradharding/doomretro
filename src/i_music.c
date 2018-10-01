/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is part of DOOM Retro.

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
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "SDL_mixer.h"

#include "c_console.h"
#include "i_midirpc.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "mmus2mid.h"
#include "s_sound.h"
#include "version.h"
#include "z_zone.h"

dboolean        midimusictype;
dboolean        musmusictype;

static dboolean music_initialized;

// If this is true, this module initialized SDL sound and has the
// responsibility to shut it down
static dboolean sdl_was_initialized;

static int      current_music_volume;
static int      paused_midi_volume;

#if defined(_WIN32)
static dboolean haveMidiServer;
dboolean        serverMidiPlaying;
#endif

// Shutdown music
void I_ShutdownMusic(void)
{
    if (!music_initialized)
        return;

    Mix_HaltMusic();
    music_initialized = false;

    if (sdl_was_initialized)
    {
        Mix_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        sdl_was_initialized = false;
    }

#if defined(_WIN32)
    I_MidiRPCClientShutDown();
#endif
}

static dboolean SDLIsInitialized(void)
{
    int     freq;
    int     channels;
    Uint16  format;

    return !!Mix_QuerySpec(&freq, &format, &channels);
}

// Initialize music subsystem
dboolean I_InitMusic(void)
{
    // If SDL_mixer is not initialized, we have to initialize it
    // and have the responsibility to shut it down later on.
    if (!SDLIsInitialized())
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
            return false;

        if (Mix_OpenAudio(SAMPLERATE, MIX_DEFAULT_FORMAT, CHANNELS, CHUNKSIZE) < 0)
        {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return false;
        }
    }

    SDL_PauseAudio(0);

    sdl_was_initialized = true;
    music_initialized = true;

#if defined(_WIN32)
    // Initialize RPC server
    haveMidiServer = I_MidiRPCInitServer();
#endif

    return music_initialized;
}

//
// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
//
static void UpdateMusicVolume(void)
{
#if defined(_WIN32)
    // adjust server volume
    if (serverMidiPlaying)
    {
        I_MidiRPCSetVolume(current_music_volume);
        return;
    }
#endif

    Mix_VolumeMusic(current_music_volume);
}

// Set music volume (0 - MAX_MUSIC_VOLUME)
void I_SetMusicVolume(int volume)
{
    // Internal state variable.
    current_music_volume = volume;

    UpdateMusicVolume();
}

// Start playing a mid
void I_PlaySong(void *handle, dboolean looping)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (serverMidiPlaying)
    {
        I_MidiRPCPlaySong(looping);
        I_MidiRPCSetVolume(current_music_volume);
        return;
    }
#endif

    if (handle)
        Mix_PlayMusic(handle, (looping ? -1 : 1));
}

void I_PauseSong(void)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (serverMidiPlaying)
    {
        I_MidiRPCPauseSong();
        return;
    }
#endif

    if (!midimusictype)
        Mix_PauseMusic();
    else
    {
        paused_midi_volume = Mix_VolumeMusic(-1);
        Mix_VolumeMusic(0);
    }
}

void I_ResumeSong(void)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (serverMidiPlaying)
    {
        I_MidiRPCResumeSong();
        return;
    }
#endif

    if (!midimusictype)
        Mix_ResumeMusic();
    else
        Mix_VolumeMusic(paused_midi_volume);
}

void I_StopSong(void)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (serverMidiPlaying)
    {
        I_MidiRPCStopSong();
        serverMidiPlaying = false;
        return;
    }
#endif

    Mix_HaltMusic();
}

void I_UnRegisterSong(void *handle)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (serverMidiPlaying)
    {
        I_MidiRPCStopSong();
        serverMidiPlaying = false;
        return;
    }
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
        SDL_RWops   *rwops;

        midimusictype = false;
        musmusictype = false;

        // Check for MIDI or MUS format first:
        if (size >= 14)
        {
            if (!memcmp(data, "MThd", 4))                       // is it a MIDI?
                midimusictype = true;
            else if (mmuscheckformat((UBYTE *)data, size))      // is it a MUS?
            {
                MIDI    mididata;
                UBYTE   *mid;
                int     midlen;

                musmusictype = true;

                memset(&mididata, 0, sizeof(MIDI));

                if (!mmus2mid((UBYTE *)data, (size_t)size, &mididata))
                    return NULL;

                // Hurrah! Let's make it a mid and give it to SDL_mixer
                MIDIToMidi(&mididata, &mid, &midlen);

                data = mid;
                size = midlen;
                midimusictype = true;                           // now it's a MIDI
            }
        }

#if defined(_WIN32)
        // Check for option to invoke RPC server if isMIDI
        if (midimusictype && haveMidiServer)
        {
            static dboolean haveMidiClient;

            if (!haveMidiClient)
                if (!(haveMidiClient = I_MidiRPCInitClient()))
                    C_Warning("The RPC client couldn't be initialized.");

            if (haveMidiClient && I_MidiRPCRegisterSong(data, size))
            {
                serverMidiPlaying = true;
                return NULL;        // server will play this song
            }
        }
#endif

        if ((rwops = SDL_RWFromMem(data, size)))
            music = Mix_LoadMUS_RW(rwops, SDL_FALSE);

        return music;
    }
}
