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

#define _USE_MATH_DEFINES

#include <math.h>

#include <SDL3_mixer/SDL_mixer.h>

#include "c_console.h"
#include "m_config.h"
#include "s_sound.h"
#include <SDL3/SDL.h>
#include "version.h"
#include "w_wad.h"

#define DMXPADSIZE  16

typedef struct allocated_sound_s
{
    sfxinfo_t                   *sfxinfo;
    MIX_Audio                   *audio;
    SDL_AudioSpec               spec;
    size_t                      datalen;
    int                         use_count;
    int                         pitch;
    struct allocated_sound_s    *prev;
    struct allocated_sound_s    *next;
} allocated_sound_t;

static bool                     sound_initialized;
static MIX_Mixer                *mixer;
static int                      mixer_refcount;
static MIX_Track                *channel_tracks[s_channels_max];
static allocated_sound_t        *channels_playing[s_channels_max];
static int                      mixer_freq = SAMPLERATE;

// Doubly-linked list of allocated sounds.
// When a sound is played, it is moved to the head, so that the oldest sounds not used recently are at the tail.
static allocated_sound_t        *allocated_sounds_head;
static allocated_sound_t        *allocated_sounds_tail;

static inline uint8_t *SoundData(allocated_sound_t *snd)
{
    return (uint8_t *)(snd + 1);
}

bool I_AcquireMixer(void)
{
    SDL_AudioSpec spec = { 0 };

    if (mixer)
    {
        mixer_refcount++;
        return true;
    }

    if (!MIX_Init())
        return false;

    spec.freq = SAMPLERATE;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;

    if (!(mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec)))
    {
        MIX_Quit();
        return false;
    }

    if (!MIX_GetMixerFormat(mixer, &spec))
    {
        MIX_DestroyMixer(mixer);
        mixer = NULL;
        MIX_Quit();
        return false;
    }

    mixer_freq = spec.freq;
    mixer_refcount = 1;

    return true;
}

void I_ReleaseMixer(void)
{
    if (!mixer_refcount)
        return;

    if (--mixer_refcount == 0)
    {
        MIX_DestroyMixer(mixer);
        mixer = NULL;
        MIX_Quit();
    }
}

MIX_Mixer *I_GetMixer(void)
{
    return mixer;
}

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
    AllocatedSoundUnlink(snd);

    if (snd->audio)
        MIX_DestroyAudio(snd->audio);

    free(snd);
}

// Search from the tail backwards along the allocated sounds list, find and free a sound that is
// not in use, to free up memory. Return true for success.
static bool FindAndFreeSound(void)
{
    allocated_sound_t   *snd = allocated_sounds_tail;

    while (snd)
    {
        if (snd->use_count <= 0)
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

    snd->spec.freq = mixer_freq;
    snd->spec.format = SDL_AUDIO_S16;
    snd->spec.channels = 2;
    snd->datalen = (size_t)length;
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

// Allocate a new sound buffer and pitch-shift an existing sound up-or-down into it.
static allocated_sound_t *PitchShift(allocated_sound_t *insnd, const int pitch)
{
    allocated_sound_t   *outsnd;
    int16_t             *srcbuf;
    int16_t             *dstbuf;
    const uint32_t      srclen_samples = (uint32_t)(insnd->datalen / sizeof(int16_t));

    // vanilla-ish behavior: pitch is around NORM_PITCH, treat as semitone-ish ratio
    const uint32_t      dstlen_samples = (uint32_t)(srclen_samples * ((float)NORM_PITCH / (float)pitch));
    const uint32_t      dstlen_bytes = dstlen_samples * sizeof(int16_t);
    const uint32_t      alloc_bytes = ((dstlen_bytes & 1) ? dstlen_bytes + 1 : dstlen_bytes);

    if (!(outsnd = AllocateSound(insnd->sfxinfo, alloc_bytes)))
        return NULL;

    outsnd->pitch = pitch;
    srcbuf = (int16_t *)SoundData(insnd);
    dstbuf = (int16_t *)SoundData(outsnd);

    // linear interpolation resampling
    for (uint32_t i = 0; i < dstlen_samples; i++)
    {
        const float  src_pos = (float)i / (float)(dstlen_samples - 1) * (srclen_samples - 1);
        const int    idx = (int)src_pos;
        const float  frac = src_pos - (float)idx;

        const int16_t s0 = srcbuf[idx];
        const int16_t s1 = (idx + 1 < (int)srclen_samples ? srcbuf[idx + 1] : s0);

        const float   samp = (1.0f - frac) * (float)s0 + frac * (float)s1;

        dstbuf[i] = (int16_t)samp;
    }

    if (!(outsnd->audio = MIX_LoadRawAudioNoCopy(mixer, SoundData(outsnd), outsnd->datalen, &outsnd->spec, false)))
    {
        FreeAllocatedSound(outsnd);
        return NULL;
    }

    return outsnd;
}

// When a sound stops, check if it is still playing. If it is not, we can mark the sound data as
// CACHE to be freed back for other means.
static void ReleaseSoundOnChannel(const int channel)
{
    allocated_sound_t   *snd;

    if (channel < 0 || channel >= s_channels_max)
        return;

    snd = channels_playing[channel];

    if (!snd)
        return;

    MIX_StopTrack(channel_tracks[channel], 0);
    channels_playing[channel] = NULL;
    UnlockAllocatedSound(snd);

    // If the sound is a pitch-shift and it's not in use, immediately free it.
    if (snd->pitch != NORM_PITCH && snd->use_count <= 0)
        FreeAllocatedSound(snd);
}

// Generic sound expansion function for any sample rate.
static bool ExpandSoundData(sfxinfo_t *sfxinfo, const byte *data,
    const int samplerate, const int bits, const int length)
{
    const unsigned int  samplecount = length / (bits / 8);
    const unsigned int  expanded_length = (unsigned int)(((uint64_t)samplecount * mixer_freq) / samplerate);
    allocated_sound_t   *snd = AllocateSound(sfxinfo, expanded_length * 4);

    if (!snd)
        return false;

    {
        int16_t         *expanded = (int16_t *)SoundData(snd);
        const double    dt = 1.0 / mixer_freq;
        const double    alpha = dt / (1.0 / (M_PI * samplerate) + dt);
        const float     src_last = (float)(samplecount - 1);

        if (bits == 8)
        {
            for (unsigned int i = 0; i < expanded_length; i++)
            {
                const float src_pos = (float)i / (float)(expanded_length - 1) * src_last;
                const int   idx = (int)src_pos;
                const float frac = src_pos - (float)idx;
                const int   s0 = (int)data[idx];
                const int   s1 = (idx + 1 < (int)samplecount ? (int)data[idx + 1] : s0);
                const float lin = (1.0f - frac) * (float)s0 + frac * (float)s1;
                const int   sample = ((int)lin | ((int)lin << 8)) - 32768;

                expanded[i * 2] = expanded[i * 2 + 1] = (int16_t)sample;
            }
        }
        else
        {
            const int16_t   *src16 = (const int16_t *)data;

            for (unsigned int i = 0; i < expanded_length; i++)
            {
                const float     src_pos = (float)i / (float)(expanded_length - 1) * src_last;
                const int       idx = (int)src_pos;
                const float     frac = src_pos - (float)idx;
                const int16_t   s0 = src16[idx];
                const int16_t   s1 = (idx + 1 < (int)samplecount ? src16[idx + 1] : s0);
                const float     sample = (1.0f - frac) * (float)s0 + frac * (float)s1;

                expanded[i * 2] = expanded[i * 2 + 1] = (int16_t)sample;
            }
        }

        // Apply low-pass filter
        for (unsigned int i = 2; i < expanded_length * 2; i++)
            expanded[i] = (int16_t)(alpha * expanded[i] + (1.0 - alpha) * expanded[i - 2]);
    }

    if (!(snd->audio = MIX_LoadRawAudioNoCopy(mixer, SoundData(snd), snd->datalen, &snd->spec, false)))
    {
        FreeAllocatedSound(snd);
        return false;
    }

    return true;
}

// Load and convert a sound effect
// Returns true if successful
bool CacheSFX(sfxinfo_t *sfxinfo)
{
    const int   lumpnum = sfxinfo->lumpnum;
    byte        *data = W_CacheLumpNum(lumpnum);
    const int   lumplen = W_LumpLength(lumpnum);

    // Check the header, and ensure this is a valid sound
    if (lumplen > 44 && !memcmp(data, "RIFF", 4) && !memcmp(data + 8, "WAVEfmt ", 8))
    {
        SDL_IOStream    *rwops = SDL_IOFromMem(data, lumplen);
        SDL_AudioSpec   spec;
        uint8_t         *buffer = NULL;
        uint32_t        length;

        if (rwops && SDL_LoadWAV_IO(rwops, true, &spec, &buffer, &length))
        {
            if (spec.channels == 1 && SDL_AUDIO_ISINT(spec.format))
            {
                const int   bits = SDL_AUDIO_BITSIZE(spec.format);

                if ((bits == 8 || bits == 16) && ExpandSoundData(sfxinfo, buffer, spec.freq, bits, length))
                {
                    SDL_free(buffer);
                    return true;
                }
            }

            SDL_free(buffer);
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
            return ExpandSoundData(sfxinfo, data + DMXPADSIZE, (data[2] | (data[3] << 8)), 8, length - DMXPADSIZE * 2);
    }

    return false;
}

void I_UpdateSoundParms(const int channel, const int vol, const int sep)
{
    const float         pan = (float)sep / 254.0f;
    const float         gain = (float)vol / 127.0f;
    const MIX_StereoGains gains =
    {
        cosf(pan * (float)M_PI_2) * gain,
        sinf(pan * (float)M_PI_2) * gain
    };

    if (channel >= 0 && channel < s_channels_max)
        MIX_SetTrackStereo(channel_tracks[channel], &gains);
}

//
// Starting a sound means adding it to the current list of active sounds in the internal channels.
// As the SFX info struct contains e.g. a pointer to the raw data, it is ignored.
// As our sound handling does not handle priority, it is ignored.
// Pitching (that is, increased speed of playback) is set, but currently not used by mixing.
//
int I_StartSound(const sfxinfo_t *sfxinfo, const int channel, const int vol, const int sep, const int pitch)
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
        else
            LockAllocatedSound(snd);
    }
    else
        LockAllocatedSound(snd);

    if (!MIX_SetTrackAudio(channel_tracks[channel], snd->audio) || !MIX_PlayTrack(channel_tracks[channel], 0))
    {
        UnlockAllocatedSound(snd);
        return -1;
    }

    channels_playing[channel] = snd;
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
    return (channel >= 0 && channel < s_channels_max && MIX_TrackPlaying(channel_tracks[channel]));
}

bool I_AnySoundStillPlaying(void)
{
    for (int i = 0; i < s_channels_max; i++)
        if (channel_tracks[i] && MIX_TrackPlaying(channel_tracks[i]))
            return true;

    return false;
}

void I_ShutdownSound(void)
{
    allocated_sound_t   *snd = allocated_sounds_head;

    if (!sound_initialized)
        return;

    for (int i = 0; i < s_channels_max; i++)
    {
        if (channel_tracks[i])
        {
            MIX_DestroyTrack(channel_tracks[i]);
            channel_tracks[i] = NULL;
        }

        channels_playing[i] = NULL;
    }

    while (snd)
    {
        allocated_sound_t   *next = snd->next;

        if (snd->audio)
            MIX_DestroyAudio(snd->audio);

        free(snd);
        snd = next;
    }

    allocated_sounds_head = NULL;
    allocated_sounds_tail = NULL;
    sound_initialized = false;

    I_ReleaseMixer();
}

bool I_InitSound(void)
{
    if (!I_AcquireMixer())
        return false;

    for (int i = 0; i < s_channels_max; i++)
    {
        channels_playing[i] = NULL;

        if (!(channel_tracks[i] = MIX_CreateTrack(mixer)))
        {
            for (int j = 0; j < i; j++)
            {
                MIX_DestroyTrack(channel_tracks[j]);
                channel_tracks[j] = NULL;
            }

            I_ReleaseMixer();
            return false;
        }
    }

    sound_initialized = true;
    return true;
}
