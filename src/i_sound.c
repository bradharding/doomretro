/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <string.h>

#include "c_console.h"
#include "i_system.h"
#include "m_config.h"
#include "s_sound.h"
#include "SDL_mixer.h"
#include "version.h"
#include "w_wad.h"

typedef struct allocated_sound_s
{
    sfxinfo_t                   *sfxinfo;
    Mix_Chunk                   chunk;
    int                         use_count;
    int                         pitch;
    struct allocated_sound_s    *prev;
    struct allocated_sound_s    *next;
} allocated_sound_t;

static bool                     sound_initialized;

static allocated_sound_t        *channels_playing[s_channels_max];

static int                      mixer_freq = MIX_DEFAULT_FREQUENCY;

// Doubly-linked list of allocated sounds.
// When a sound is played, it is moved to the head, so that the oldest sounds not used recently are at the tail.
static allocated_sound_t        *allocated_sounds_head;
static allocated_sound_t        *allocated_sounds_tail;

// Hook a sound into the linked list at the head.
static void AllocatedSoundLink(allocated_sound_t *snd)
{
    snd->prev = NULL;

    snd->next = allocated_sounds_head;
    allocated_sounds_head = snd;

    if (!allocated_sounds_tail)
        allocated_sounds_tail = snd;
    else
        snd->next->prev = snd;
}

// Unlink a sound from the linked list.
static void AllocatedSoundUnlink(allocated_sound_t *snd)
{
    if (!snd->prev)
        allocated_sounds_head = snd->next;
    else
        snd->prev->next = snd->next;

    if (!snd->next)
        allocated_sounds_tail = snd->prev;
    else
        snd->next->prev = snd->prev;
}

static void FreeAllocatedSound(allocated_sound_t *snd)
{
    // Unlink from linked list.
    AllocatedSoundUnlink(snd);
    free(snd);
}

// Search from the tail backwards along the allocated sounds list, find and free a sound that is
// not in use, to free up memory. Return true for success.
static bool FindAndFreeSound(void)
{
    allocated_sound_t   *snd = allocated_sounds_tail;

    while (snd)
    {
        if (!snd->use_count)
        {
            FreeAllocatedSound(snd);
            return true;
        }

        snd = snd->prev;
    }

    // No available sounds to free...
    return false;
}

// Allocate a block for a new sound effect.
static allocated_sound_t *AllocateSound(sfxinfo_t *sfxinfo, const int length)
{
    allocated_sound_t   *snd;

    // Allocate the sound structure and data. The data will immediately follow the structure, which
    // acts as a header.
    do
    {
        // Out of memory? Try to free an old sound, then loop round and try again.
        if (!(snd = calloc(1, sizeof(allocated_sound_t) + length)) && !FindAndFreeSound())
            return NULL;
    } while (!snd);

    // Skip past the chunk structure for the audio buffer
    snd->chunk.abuf = (uint8_t *)(snd + 1);
    snd->chunk.alen = length;
    snd->chunk.allocated = 1;
    snd->chunk.volume = MIX_MAX_VOLUME - 1;
    snd->pitch = NORM_PITCH;
    snd->sfxinfo = sfxinfo;
    snd->use_count = 0;

    AllocatedSoundLink(snd);

    return snd;
}

// Lock a sound, to indicate that it may not be freed.
static void LockAllocatedSound(allocated_sound_t *snd)
{
    // Increase use count, to stop the sound being freed.
    snd->use_count++;

    // When we use a sound, re-link it into the list at the head, so that the oldest sounds fall to
    // the end of the list for freeing.
    AllocatedSoundUnlink(snd);
    AllocatedSoundLink(snd);
}

// Unlock a sound to indicate that it may now be freed.
static void UnlockAllocatedSound(allocated_sound_t *snd)
{
    snd->use_count--;
}

static allocated_sound_t *GetAllocatedSoundBySfxInfoAndPitch(const sfxinfo_t *sfxinfo, const int pitch)
{
    allocated_sound_t   *p = allocated_sounds_head;

    while (p)
    {
        if (p->sfxinfo == sfxinfo && p->pitch == pitch)
            return p;

        p = p->next;
    }

    return NULL;
}

// Allocate a new sound chunk and pitch-shift an existing sound up-or-down into it.
static allocated_sound_t *PitchShift(allocated_sound_t *insnd, const int pitch)
{
    allocated_sound_t   *outsnd;
    int16_t             *srcbuf;
    int16_t             *dstbuf;
    const uint32_t      srclen = insnd->chunk.alen;

    // Determine ratio pitch:NORM_PITCH and apply to srclen, then invert.
    // This is an approximation of vanilla behavior based on measurements.
    uint32_t            dstlen = (uint32_t)((1 + (1 - (float)pitch / NORM_PITCH)) * srclen);

    // ensure that the new buffer is an even length
    if (!(dstlen % 2))
        dstlen++;

    if (!(outsnd = AllocateSound(insnd->sfxinfo, dstlen)))
        return NULL;

    outsnd->pitch = pitch;
    srcbuf = (int16_t *)insnd->chunk.abuf;
    dstbuf = (int16_t *)outsnd->chunk.abuf;

    // Loop over output buffer. Find corresponding input cell, copy over.
    for (int16_t *inp, *outp = dstbuf; outp < dstbuf + dstlen / 2; outp++)
    {
        inp = &srcbuf[(int)((float)(outp - dstbuf) / dstlen * srclen)];
        *outp = *inp;
    }

    return outsnd;
}

// When a sound stops, check if it is still playing. If it is not, we can mark the sound data as
// CACHE to be freed back for other means.
static void ReleaseSoundOnChannel(const int channel)
{
    allocated_sound_t   *snd = channels_playing[channel];

    if (!snd)
        return;

    Mix_HaltChannel(channel);

    channels_playing[channel] = NULL;
    UnlockAllocatedSound(snd);

    // If the sound is a pitch-shift and it's not in use, immediately free it.
    if (snd->pitch != NORM_PITCH && snd->use_count <= 0)
        FreeAllocatedSound(snd);
}

// Generic sound expansion function for any sample rate.
static void ExpandSoundData(sfxinfo_t *sfxinfo, const byte *data, const int samplerate, const int bits, const int length)
{
    const unsigned int  samplecount = length / (bits / 8);
    const unsigned int  expanded_length = (unsigned int)(((uint64_t)samplecount * mixer_freq) / samplerate);
    allocated_sound_t   *snd = AllocateSound(sfxinfo, expanded_length * 4);
    int16_t             *expanded = (int16_t *)(&snd->chunk)->abuf;
    const int           expand_ratio = (samplecount << 8) / expanded_length;
    const double        dt = 1.0 / mixer_freq;
    const double        alpha = dt / (1.0 / (M_PI * samplerate) + dt);

    if (bits == 8)
        for (unsigned int i = 0; i < expanded_length; i++)
        {
            const int   src = data[(i * expand_ratio) >> 8];

            expanded[i * 2] = expanded[i * 2 + 1] = (src | (src << 8)) - 32768;
        }
    else
        for (unsigned int i = 0; i < expanded_length; i++)
        {
            const int   src = ((i * expand_ratio) >> 8) * 2;

            expanded[i * 2] = expanded[i * 2 + 1] = (data[src] | (data[src + 1] << 8));
        }

    // Apply low-pass filter
    for (unsigned int i = 2; i < expanded_length * 2; i++)
        expanded[i] = (int16_t)(alpha * expanded[i] + (1.0 - alpha) * expanded[i - 2]);
}

// Load and convert a sound effect
// Returns true if successful
bool CacheSFX(sfxinfo_t *sfxinfo)
{
    // Need to load the sound
    const int   lumpnum = sfxinfo->lumpnum;
    byte        *data = W_CacheLumpNum(lumpnum);
    const int   lumplen = W_LumpLength(lumpnum);

    // Check the header, and ensure this is a valid sound
    if (lumplen > 44 && !memcmp(data, "RIFF", 4) && !memcmp(data + 8, "WAVEfmt ", 8))
    {
        SDL_RWops       *rwops = SDL_RWFromMem(data, lumplen);
        SDL_AudioSpec   spec;
        uint8_t         *buffer = NULL;
        uint32_t        length;

        if (SDL_LoadWAV_RW(rwops, 1, &spec, &buffer, &length))
        {
            if (spec.channels == 1 && SDL_AUDIO_ISINT(spec.format))
            {
                const int   bits = SDL_AUDIO_BITSIZE(spec.format);

                if (bits == 8 || bits == 16)
                {
                    ExpandSoundData(sfxinfo, buffer, spec.freq, bits, length);
                    return true;
                }
            }

            SDL_FreeWAV(buffer);
        }
    }
    else if (lumplen >= 8 && data[0] == 0x03 && data[1] == 0x00)
    {
        const int   length = (data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24));

        // If the header specifies that the length of the sound is greater than the length of the lump
        // itself, this is an invalid sound lump.

        // We also discard sound lumps that are less than 49 samples long, as this is how DMX behaves -
        // although the actual cut-off length seems to vary slightly depending on the sample rate. This
        // needs further investigation to better understand the correct behavior.
        if (length > 48 && length <= lumplen - 8)
        {
            ExpandSoundData(sfxinfo, data + 16, (data[2] | (data[3] << 8)), 8, length - 32);
            return true;
        }
    }

    return false;
}

void I_UpdateSoundParms(const int channel, const int vol, const int sep)
{
    Mix_SetPanning(channel, (254 - sep) * vol / (MIX_MAX_VOLUME - 1), sep * vol / (MIX_MAX_VOLUME - 1));
}

//
// Starting a sound means adding it to the current list of active sounds in the internal channels.
// As the SFX info struct contains e.g. a pointer to the raw data, it is ignored.
// As our sound handling does not handle priority, it is ignored.
// Pitching (that is, increased speed of playback) is set, but currently not used by mixing.
//
int I_StartSound(sfxinfo_t *sfxinfo, const int channel, const int vol, const int sep, const int pitch)
{
    allocated_sound_t   *snd;

    // Release a sound effect if there is already one playing on this channel.
    ReleaseSoundOnChannel(channel);

    if (!(snd = GetAllocatedSoundBySfxInfoAndPitch(sfxinfo, pitch)))
    {
        // Fetch the base sound effect, un-pitch-shifted.
        if (!(snd = GetAllocatedSoundBySfxInfoAndPitch(sfxinfo, NORM_PITCH)))
            return -1;

        if (s_randompitch && pitch && pitch != NORM_PITCH)
        {
            allocated_sound_t   *newsnd = PitchShift(snd, pitch);

            if (newsnd)
            {
                LockAllocatedSound(newsnd);
                UnlockAllocatedSound(snd);
                snd = newsnd;
            }
        }
    }
    else
        LockAllocatedSound(snd);

    // Play sound
    Mix_PlayChannel(channel, &snd->chunk, 0);

    channels_playing[channel] = snd;

    // Set separation, etc.
    I_UpdateSoundParms(channel, vol, sep);

    return channel;
}

void I_StopSound(const int channel)
{
    // Sound data is no longer needed; release the sound data being used for this channel.
    ReleaseSoundOnChannel(channel);
}

bool I_SoundIsPlaying(const int channel)
{
    return Mix_Playing(channel);
}

bool I_AnySoundStillPlaying(void)
{
    return Mix_Playing(-1);
}

void I_ShutdownSound(void)
{
    if (!sound_initialized)
        return;

    Mix_CloseAudio();
    sound_initialized = false;
}

bool I_InitSound(void)
{
    const SDL_version   *linked = Mix_Linked_Version();
    uint16_t            mixer_format;
    int                 mixer_channels;

    // No sounds yet
    for (int i = 0; i < s_channels_max; i++)
        channels_playing[i] = NULL;

    if (linked->major != SDL_MIXER_MAJOR_VERSION || linked->minor != SDL_MIXER_MINOR_VERSION)
        I_Error("The wrong version of " SDL_MIXER_FILENAME " was found. " DOOMRETRO_NAME " requires v%i.%i.%i.",
            SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL);

    if (linked->patch != SDL_MIXER_PATCHLEVEL)
        C_Warning(1, "The wrong version of " BOLD(SDL_MIXER_FILENAME) " was found. " ITALICS(DOOMRETRO_NAME) " requires v%i.%i.%i.",
            SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL);

    if (Mix_OpenAudioDevice(SAMPLERATE, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS,
        CHUNKSIZE, DEFAULT_DEVICE, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE) < 0)
        return false;

    if (!Mix_QuerySpec(&mixer_freq, &mixer_format, &mixer_channels))
        return false;

    Mix_AllocateChannels(s_channels_max);
    sound_initialized = true;

    return true;
}
