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

#include <stdlib.h>

#include "SDL_mixer.h"

#include "c_console.h"
#include "doomstat.h"
#include "i_winmusic.h"
#include "m_config.h"
#include "memio.h"
#if defined(_WIN32)
#include "midifile.h"
#endif
#include "mus2mid.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

bool        midimusictype;
bool        musmusictype;

#if defined(_WIN32)
bool        windowsmidi = false;
static bool windowsmidisong;
#else
static int  paused_midi_volume;
#endif

static bool music_initialized;

int         current_music_volume = 0;

#if defined(_WIN32)
typedef struct
{
    unsigned int    tick;
    unsigned int    tempo;
} tempochange_t;

static int TempoChangeCompare(const void *p1, const void *p2)
{
    const tempochange_t  *tempo1 = (const tempochange_t *)p1;
    const tempochange_t  *tempo2 = (const tempochange_t *)p2;

    if (tempo1->tick < tempo2->tick)
        return -1;
    else if (tempo1->tick > tempo2->tick)
        return 1;
    else
        return 0;
}

static double GetMIDIDuration(void *data, const int size)
{
    midi_file_t      *file = MIDI_LoadFile(SDL_RWFromMem(data, size));
    tempochange_t    *tempochanges = NULL;
    unsigned int     numtempochanges = 0;
    unsigned int     maxtick = 0;
    unsigned int     timedivision;
    double           duration = 0.0;

    if (!file)
        return 0.0;

    if (!(timedivision = MIDI_GetFileTimeDivision(file)))
    {
        MIDI_FreeFile(file);
        return 0.0;
    }

    for (unsigned int i = 0; i < MIDI_NumTracks(file); i++)
    {
        midi_track_iter_t   *iter = MIDI_IterateTrack(file, i);
        midi_event_t        *event;
        unsigned int        tick = 0;

        if (!iter)
            continue;

        while (MIDI_GetNextEvent(iter, &event))
        {
            tick += event->delta_time;

            if (event->event_type == MIDI_EVENT_META
                && event->data.meta.type == MIDI_META_SET_TEMPO
                && event->data.meta.length == 3)
            {
                tempochange_t   *newtempochanges = realloc(tempochanges,
                                    (numtempochanges + 1) * sizeof(*tempochanges));

                if (newtempochanges)
                {
                    tempochanges = newtempochanges;
                    tempochanges[numtempochanges].tick = tick;
                    tempochanges[numtempochanges].tempo = (event->data.meta.data[0] << 16)
                        | (event->data.meta.data[1] << 8) | event->data.meta.data[2];
                    numtempochanges++;
                }
            }
        }

        if (tick > maxtick)
            maxtick = tick;

        free(iter);
    }

    if (tempochanges)
    {
        unsigned int    currenttick = 0;
        unsigned int    currenttempo = 500000;

        qsort(tempochanges, numtempochanges, sizeof(*tempochanges), TempoChangeCompare);

        for (unsigned int i = 0; i < numtempochanges; i++)
        {
            if (tempochanges[i].tick > maxtick)
                break;

            if (tempochanges[i].tick > currenttick)
            {
                duration += (tempochanges[i].tick - currenttick) * currenttempo / (double)timedivision / 1000000.0;
                currenttick = tempochanges[i].tick;
            }

            currenttempo = tempochanges[i].tempo;
        }

        if (maxtick > currenttick)
            duration += (maxtick - currenttick) * currenttempo / (double)timedivision / 1000000.0;

        free(tempochanges);
    }
    else
        duration = maxtick * 500000.0 / (double)timedivision / 1000000.0;

    MIDI_FreeFile(file);

    return duration;
}
#endif

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

    music_initialized = false;

#if defined(_WIN32)
    windowsmidisong = false;

    if (windowsmidi)
    {
        I_Windows_ShutdownMusic();
        windowsmidi = false;
    }
#endif
}

// Initialize music subsystem
bool I_InitMusic(void)
{
    int         freq = MIX_DEFAULT_FREQUENCY;
    int         channels;
    uint16_t    format;

#if defined(_WIN32)
    // Never let SDL Mixer use native MIDI on Windows. Avoids SDL Mixer bug
    // where music volume affects global application volume.
    SDL_setenv("SDL_MIXER_DISABLE_NATIVEMIDI", "1", true);
#endif

    // If SDL_mixer is not initialized, we have to initialize it and have the
    // responsibility to shut it down later on.
    if (!Mix_QuerySpec(&freq, &format, &channels))
        if (Mix_OpenAudioDevice(SAMPLERATE, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS,
            CHUNKSIZE, DEFAULT_DEVICE, SDL_AUDIO_ALLOW_ANY_CHANGE) < 0)
            return false;

    music_initialized = true;

#if defined(_WIN32)
    if (!(windowsmidi = I_Windows_InitMusic()))
        C_Warning(1, "Music couldn't be completely %s. Volume adjustment could be affected.",
            (english == english_american ? "initialized" : "initialised"));

    if (extras && W_CheckNumForName((s_remix ? "H_INTRO" : "O_INTRO")) >= 0 && (!sigil || buckethead) && (!sigil2 || thorr) && !legacyofrust)
        return true;
#endif

    return music_initialized;
}

// Set music volume (0 - 127)
void I_SetMusicVolume(const int volume)
{
    // Internal state variable.
    current_music_volume = volume;

#if defined(_WIN32)
    if (windowsmidisong)
        I_Windows_SetMusicVolume(current_music_volume);
    else
        Mix_VolumeMusic(current_music_volume);
#else
    Mix_VolumeMusic(current_music_volume);
#endif
}

// Start playing a mid
void I_PlaySong(void *handle, const bool looping)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (windowsmidisong)
        I_Windows_PlaySong(looping);
    else if (handle)
        Mix_PlayMusic(handle, (looping ? -1 : 1));
#else
    Mix_PlayMusic(handle, (looping ? -1 : 1));
#endif
}

void I_PauseSong(void)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (windowsmidisong)
        I_Windows_PauseSong();
    else
        Mix_PauseMusic();
#else
    if (midimusictype)
    {
        paused_midi_volume = current_music_volume;
        Mix_VolumeMusic(0);
    }
    else
        Mix_PauseMusic();
#endif
}

void I_ResumeSong(void)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (windowsmidisong)
        I_Windows_ResumeSong();
    else
        Mix_ResumeMusic();
#else
    if (midimusictype)
        Mix_VolumeMusic(paused_midi_volume);
    else
        Mix_ResumeMusic();
#endif
}

void I_StopSong(void)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (windowsmidisong)
        I_Windows_StopSong();
#endif

    Mix_HaltMusic();
}

double I_GetMusicDuration(void *handle, void *data, int size)
{
    double  duration = 0.0;

    if (handle)
        duration = Mix_MusicDuration(handle);

    if (duration > 0.0 || size < 14)
        return duration;

#if defined(_WIN32)
    if (!memcmp(data, "MThd", 4))
        return GetMIDIDuration(data, size);
    else if (!memcmp(data, "MUS\x1A", 4))
    {
        MEMFILE *instream = mem_fopen_read(data, size);
        MEMFILE *outstream = mem_fopen_write();
        double  musduration = 0.0;

        if (mus2mid(instream, outstream))
        {
            void    *middata;
            size_t  midsize;

            mem_get_buf(outstream, &middata, &midsize);
            musduration = GetMIDIDuration(middata, (int)midsize);
        }

        mem_fclose(instream);
        mem_fclose(outstream);

        return musduration;
    }
#endif

    return duration;
}

void I_UnregisterSong(void *handle)
{
    if (!music_initialized)
        return;

#if defined(_WIN32)
    if (windowsmidisong)
    {
        I_Windows_UnregisterSong();
        windowsmidisong = false;
    }
    else if (handle)
        Mix_FreeMusic(handle);
#else
    if (handle)
        Mix_FreeMusic(handle);
#endif
}

void *I_RegisterSong(void *data, int size)
{
    if (!music_initialized)
        return NULL;
    else
    {
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

                if (mus2mid(instream, outstream))
                {
                    void    *outbuf;
                    byte    *mid;
                    size_t  midlen;

                    mem_get_buf(outstream, &outbuf, &midlen);

                    if ((mid = Z_Malloc(midlen, PU_LEVEL, NULL)))
                    {
                        memcpy(mid, outbuf, midlen);
                        data = mid;
                        size = (int)midlen;
                    }
                }

                mem_fclose(instream);
                mem_fclose(outstream);

                midimusictype = true;               // now it's a MIDI
            }
        }

#if defined(_WIN32)
        windowsmidisong = false;

        if (midimusictype && windowsmidi)
        {
            if (I_Windows_RegisterSong(data, size))
            {
                windowsmidisong = true;
                return NULL;
            }
        }
#endif

        if (!(rwops = SDL_RWFromMem(data, size)))
            return NULL;

        return Mix_LoadMUS_RW(rwops, 1);
    }
}
