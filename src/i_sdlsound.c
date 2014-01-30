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

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "SDL.h"
#include "SDL_mixer.h"

#include "i_system.h"
#include "i_swap.h"
#include "s_sound.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"

#include "doomdef.h"

#define MAX_SOUND_SLICE_TIME 70 /* ms */
#define NUM_CHANNELS 32

static boolean sound_initialized = false;

static Mix_Chunk sound_chunks[NUMSFX];
static int channels_playing[NUM_CHANNELS];

static int mixer_freq;
static Uint16 mixer_format;
static int mixer_channels;

// When a sound stops, check if it is still playing.  If it is not,
// we can mark the sound data as CACHE to be freed back for other
// means.

static void ReleaseSoundOnChannel(int channel)
{
    int i;
    int id = channels_playing[channel];

    if (!id)
    {
        return;
    }

    channels_playing[channel] = sfx_None;

    for (i = 0; i < NUM_CHANNELS; ++i)
    {
        // Playing on this channel? if so, don't release.

        if (channels_playing[i] == id)
            return;
    }

    // Not used on any channel, and can be safely released

    Z_ChangeTag(sound_chunks[id].abuf, PU_CACHE);
}


static boolean ConvertibleRatio(int freq1, int freq2)
{
    int ratio;

    if (freq1 > freq2)
    {
        return ConvertibleRatio(freq2, freq1);
    }
    else if ((freq2 % freq1) != 0)
    {
        // Not in a direct ratio

        return false;
    }
    else
    {
        // Check the ratio is a power of 2

        ratio = freq2 / freq1;

        while ((ratio & 1) == 0)
        {
            ratio = ratio >> 1;
        }

        return ratio == 1;
    }
}

// Generic sound expansion function for any sample rate.

static void ExpandSoundData_SDL(byte *data, int samplerate, uint32_t length, Mix_Chunk *destination)
{
    SDL_AudioCVT convertor;
    uint32_t expanded_length;

    // Calculate the length of the expanded version of the sample.

    expanded_length = (uint32_t)((((uint64_t) length) * mixer_freq) / samplerate);

    // Double up twice: 8 -> 16 bit and mono -> stereo

    expanded_length *= 4;
    destination->alen = expanded_length;
    destination->abuf
        = (Uint8 *)Z_Malloc(expanded_length, PU_STATIC, (void **)&destination->abuf);

    // If we can, use the standard / optimized SDL conversion routines.

    if (samplerate <= mixer_freq
        && ConvertibleRatio(samplerate, mixer_freq)
        && SDL_BuildAudioCVT(&convertor,
                             AUDIO_U8, 1, samplerate,
                             mixer_format, mixer_channels, mixer_freq))
    {
        convertor.buf = destination->abuf;
        convertor.len = length;
        memcpy(convertor.buf, data, length);

        SDL_ConvertAudio(&convertor);
    }
    else
    {
        Sint16 *expanded = (Sint16 *)destination->abuf;
        int expanded_length;
        int expand_ratio;
        int i;

        // Generic expansion if conversion does not work:
        //
        // SDL's audio conversion only works for rate conversions that are
        // powers of 2; if the two formats are not in a direct power of 2
        // ratio, do this naive conversion instead.

        // number of samples in the converted sound

        expanded_length = ((uint64_t) length * mixer_freq) / samplerate;
        expand_ratio = (length << 8) / expanded_length;

        for (i = 0; i < expanded_length; ++i)
        {
            Sint16 sample;
            int src;

            src = (i * expand_ratio) >> 8;

            sample = data[src] | (data[src] << 8);
            sample -= 32768;

            // expand 8->16 bits, mono->stereo

            expanded[i * 2] = expanded[i * 2 + 1] = sample;
        }

        {
            float rc, dt, alpha;

            // Low-pass filter for cutoff frequency f:
            //
            // For sampling rate r, dt = 1 / r
            // rc = 1 / 2*pi*f
            // alpha = dt / (rc + dt)

            // Filter to the half sample rate of the original sound effect
            // (maximum frequency, by nyquist)

            dt = 1.0f / mixer_freq;
            rc = 1.0f / (float)(M_PI * samplerate);
            alpha = dt / (rc + dt);

            // Both channels are processed in parallel, hence [i-2]:

            for (i=2; i<expanded_length * 2; ++i)
            {
                expanded[i] = (Sint16) (alpha * expanded[i]
                                      + (1 - alpha) * expanded[i-2]);
            }
        }
    }
}


// Load and validate a sound effect lump.
// Preconditions:
//     S_sfx[sound].lumpnum has been set
// Postconditions if sound is valid:
//     returns true
//     starred parameters are set, with data_ref pointing to start of sound
//     caller is responsible for releasing the identified lump
// Postconditions if sound is invalid:
//     returns false
//     starred parameters are garbage
//     lump already released

static boolean LoadSoundLump(int sound, int *lumpnum, int *samplerate, uint32_t *length, byte **data_ref)
{
    int lumplen;
    byte *data;

    // Load the sound

    *lumpnum = S_sfx[sound].lumpnum;
    *data_ref = (byte *)W_CacheLumpNum(*lumpnum, PU_STATIC);
    lumplen = W_LumpLength(*lumpnum);
    data  = *data_ref;

    // Ensure this is a valid sound

    if (lumplen < 8 || data[0] != 0x03 || data[1] != 0x00)
    {
        // Invalid sound
        W_ReleaseLumpNum(*lumpnum);
        return false;
    }

    // 16 bit sample rate field, 32 bit length field

    *samplerate = (data[3] << 8) | data[2];
    *length = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4];

    // If the header specifies that the length of the sound is
    // greater than the length of the lump itself, this is an invalid
    // sound lump.

    // We also discard sound lumps that are less than 49 samples long,
    // as this is how DMX behaves - although the actual cut-off length
    // seems to vary slightly depending on the sample rate.  This needs
    // further investigation to better understand the correct
    // behavior.

    if (*length > (unsigned)lumplen - 8 || *length <= 48)
    {
        W_ReleaseLumpNum(*lumpnum);
        return false;
    }

    // Prune header
    *data_ref += 8;

    // The DMX sound library seems to skip the first 16 and last 16
    // bytes of the lump - reason unknown.
    *data_ref += 16;
    *length -= 32;

    return true;
}


// Load and convert a sound effect
// Returns true if successful

static boolean CacheSFX_SDL(int sound)
{
    int lumpnum;
    int samplerate;
    uint32_t length;
    byte *data;

    if (!LoadSoundLump(sound, &lumpnum, &samplerate, &length, &data))
        return false;

    // Sample rate conversion
    // sound_chunks[sound].alen and abuf are determined by ExpandSoundData.

    sound_chunks[sound].allocated = 1;
    sound_chunks[sound].volume = MIX_MAX_VOLUME;

    ExpandSoundData_SDL(data,
                        samplerate,
                        length,
                        &sound_chunks[sound]);


    // don't need the original lump any more

    W_ReleaseLumpNum(lumpnum);

    return true;
}




static Mix_Chunk *GetSFXChunk(int sound_id)
{
    if (sound_chunks[sound_id].abuf == NULL)
    {
        if (!CacheSFX_SDL(sound_id))
            return NULL;
    }
    else
    {
        // don't free the sound while it is playing!

        Z_ChangeTag(sound_chunks[sound_id].abuf, PU_STATIC);
    }

    return &sound_chunks[sound_id];
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//

static int I_SDL_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char namebuf[9];

    sprintf(namebuf, "ds%s", sfx->name);

    return W_GetNumForName(namebuf);
}

static void I_SDL_UpdateSoundParams(int handle, int vol, int sep)
{
    int left, right;

    if (!sound_initialized)
        return;

    left = (254 - sep) * vol / 127;
    right = sep * vol / 127;

    Mix_SetPanning(handle, left, right);
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//

static int I_SDL_StartSound(int id, int channel, int vol, int sep)
{
    Mix_Chunk *chunk;

    if (!sound_initialized)
        return -1;

    // Release a sound effect if there is already one playing
    // on this channel

    ReleaseSoundOnChannel(channel);

    // Get the sound data

    chunk = GetSFXChunk(id);

    if (chunk == NULL)
        return -1;

    // play sound

    Mix_PlayChannelTimed(channel, chunk, 0, -1);

    channels_playing[channel] = id;

    // set separation, etc.

    I_SDL_UpdateSoundParams(channel, vol, sep);

    return channel;
}

static void I_SDL_StopSound (int handle)
{
    if (!sound_initialized)
        return;

    Mix_HaltChannel(handle);

    // Sound data is no longer needed; release the
    // sound data being used for this channel

    ReleaseSoundOnChannel(handle);
}


static boolean I_SDL_SoundIsPlaying(int handle)
{
    if (handle < 0)
        return false;

    return Mix_Playing(handle);
}

//
// Periodically called to update the sound system
//

static void I_SDL_UpdateSound(void)
{
    int i;

    // Check all channels to see if a sound has finished

    for (i = 0; i < NUM_CHANNELS; ++i)
        if (channels_playing[i] && !I_SDL_SoundIsPlaying(i))
            // Sound has finished playing on this channel,
            // but sound data has not been released to cache

            ReleaseSoundOnChannel(i);
}

static void I_SDL_ShutdownSound(void)
{
    if (!sound_initialized)
        return;

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    sound_initialized = false;
}

// Calculate slice size, based on MAX_SOUND_SLICE_TIME.
// The result must be a power of two.

static int GetSliceSize(void)
{
    int limit;
    int n;

    limit = snd_samplerate * MAX_SOUND_SLICE_TIME / 1000;

    // Try all powers of two, not exceeding the limit.

    for (n = 0; ; ++n)
        // 2^n <= limit < 2^n+1 ?

        if ((1 << (n + 1)) > limit)
            return (1 << n);

    // Should never happen?

    return 1024;
}

static boolean I_SDL_InitSound(void)
{
    int i;

    // No sounds yet

    for (i = 0; i < NUMSFX; ++i)
        sound_chunks[i].abuf = NULL;

    for (i = 0; i < NUM_CHANNELS; ++i)
        channels_playing[i] = sfx_None;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Unable to set up sound.\n");
        return false;
    }

    if (Mix_OpenAudio(snd_samplerate, AUDIO_S16SYS, 2, GetSliceSize()) < 0)
    {
        fprintf(stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
        return false;
    }

    Mix_QuerySpec(&mixer_freq, &mixer_format, &mixer_channels);

    // precache sounds even when not using libsamplerate to avoid slowdown
    // inside game
    for (i = 0; i < NUMSFX; i++)
        CacheSFX_SDL(i);

    Mix_AllocateChannels(NUM_CHANNELS);

    SDL_PauseAudio(0);

    sound_initialized = true;

    return true;
}

static snddevice_t sound_sdl_devices[] =
{
    SNDDEVICE_SB,
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_AWE32,
};

sound_module_t sound_sdl_module =
{
    sound_sdl_devices,
    arrlen(sound_sdl_devices),
    I_SDL_InitSound,
    I_SDL_ShutdownSound,
    I_SDL_GetSfxLumpNum,
    I_SDL_UpdateSound,
    I_SDL_UpdateSoundParams,
    I_SDL_StartSound,
    I_SDL_StopSound,
    I_SDL_SoundIsPlaying,
};