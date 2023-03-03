/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

#if defined(_WIN32)

#include <assert.h>
#include <string.h>

#include "doomtype.h"
#include "i_system.h"
#include "midifile.h"

// Check the header of a chunk
static bool CheckChunkHeader(chunk_header_t *chunk, const char *expected_id)
{
    return !memcmp((char *)chunk->chunk_id, expected_id, 4);
}

// Read a single byte. Return false on error.
static bool ReadByte(byte *result, SDL_RWops *stream)
{
    int c;

    if (SDL_RWread(stream, &c, 1, 1))
    {
        *result = (byte)c;
        return true;
    }

    return false;
}

// Read a variable-length value.
static bool ReadVariableLength(unsigned int *result, SDL_RWops *stream)
{
    *result = 0;

    for (int i = 0; i < 4; i++)
    {
        byte    b;

        if (!ReadByte(&b, stream))
            return false;

        // Insert the bottom seven bits from this byte.
        *result <<= 7;
        *result |= (b & 0x7F);

        // If the top bit is not set, this is the end.
        if (!(b & 0x80))
            return true;
    }

    return false;
}

// Read a byte sequence into the data buffer.
static void *ReadByteSequence(size_t num_bytes, SDL_RWops *stream)
{
    // Allocate a buffer. Allocate one extra byte, as malloc(0) is non-portable.
    byte    *result = malloc(num_bytes + 1);

    if (!result)
        return NULL;

    // Read the data
    for (unsigned int i = 0; i < num_bytes; i++)
        if (!ReadByte(&result[i], stream))
        {
            free(result);
            return NULL;
        }

    return result;
}

// Read a MIDI channel event.
// two_param indicates that the event type takes two parameters
// (three byte) otherwise it is single parameter (two byte)
static bool ReadChannelEvent(midi_event_t *event, byte event_type, bool two_param, SDL_RWops *stream)
{
    byte    b;

    // Set basics
    event->event_type = (event_type & 0xF0);
    event->data.channel.channel = (event_type & 0x0F);

    // Read parameters
    if (!ReadByte(&b, stream))
        return false;

    event->data.channel.param1 = b;

    // Second parameter
    if (two_param)
    {
        if (!ReadByte(&b, stream))
            return false;

        event->data.channel.param2 = b;
    }

    return true;
}

// Read sysex event
static bool ReadSysExEvent(midi_event_t *event, int event_type, SDL_RWops *stream)
{
    event->event_type = event_type;

    if (!ReadVariableLength(&event->data.sysex.length, stream))
        return false;

    // Read the byte sequence
    event->data.sysex.data = ReadByteSequence(event->data.sysex.length, stream);

    if (!event->data.sysex.data)
        return false;

    return true;
}

// Read meta event
static bool ReadMetaEvent(midi_event_t *event, SDL_RWops *stream)
{
    byte    b;

    event->event_type = MIDI_EVENT_META;

    // Read meta event type
    if (!ReadByte(&b, stream))
        return false;

    event->data.meta.type = b;

    // Read length of meta event data
    if (!ReadVariableLength(&event->data.meta.length, stream))
        return false;

    // Read the byte sequence
    event->data.meta.data = ReadByteSequence(event->data.meta.length, stream);

    if (!event->data.meta.data)
        return false;

    return true;
}

static bool ReadEvent(midi_event_t *event, unsigned int *last_event_type, SDL_RWops *stream)
{
    byte    event_type = 0;

    if (!ReadVariableLength(&event->delta_time, stream))
        return false;

    if (!ReadByte(&event_type, stream))
        return false;

    // All event types have their top bit set. Therefore, if
    // the top bit is not set, it is because we are using the "same
    // as previous event type" shortcut to save a byte. Skip back
    // a byte so that we read this byte again.
    if (event_type & 0x80)
        *last_event_type = event_type;
    else
    {
        event_type = *last_event_type;

        if (SDL_RWseek(stream, -1, RW_SEEK_CUR) == -1)
            return false;
    }

    // Check event type
    switch (event_type & 0xF0)
    {
        // Two parameter channel events
        case MIDI_EVENT_NOTE_OFF:
        case MIDI_EVENT_NOTE_ON:
        case MIDI_EVENT_AFTERTOUCH:
        case MIDI_EVENT_CONTROLLER:
        case MIDI_EVENT_PITCH_BEND:
            return ReadChannelEvent(event, event_type, true, stream);

        // Single parameter channel events
        case MIDI_EVENT_PROGRAM_CHANGE:
        case MIDI_EVENT_CHAN_AFTERTOUCH:
            return ReadChannelEvent(event, event_type, false, stream);

        default:
            break;
    }

    // Specific value?
    switch (event_type)
    {
        case MIDI_EVENT_SYSEX:
        case MIDI_EVENT_SYSEX_SPLIT:
            return ReadSysExEvent(event, event_type, stream);

        case MIDI_EVENT_META:
            return ReadMetaEvent(event, stream);

        default:
            break;
    }

    return false;
}

// Free an event
static void FreeEvent(midi_event_t *event)
{
    // Some event types have dynamically allocated buffers assigned to them that must be freed
    switch (event->event_type)
    {
        case MIDI_EVENT_SYSEX:
        case MIDI_EVENT_SYSEX_SPLIT:
            free(event->data.sysex.data);
            break;

        case MIDI_EVENT_META:
            free(event->data.meta.data);
            break;

        default:
            break;
    }
}

// Read and check the track chunk header
static bool ReadTrackHeader(midi_track_t *track, SDL_RWops *stream)
{
    chunk_header_t  chunk_header;

    if (!SDL_RWread(stream, &chunk_header, sizeof(chunk_header_t), 1))
        return false;

    if (!CheckChunkHeader(&chunk_header, TRACK_CHUNK_ID))
        return false;

    track->data_len = SDL_SwapBE32(chunk_header.chunk_size);

    return true;
}

static bool ReadTrack(midi_track_t *track, SDL_RWops *stream)
{
    midi_event_t    *new_events = NULL;
    unsigned int    last_event_type = 0;

    track->num_events = 0;
    track->num_events_mem = 0;
    track->events = NULL;

    // Read the header
    if (!ReadTrackHeader(track, stream))
        return false;

    // Then the events
    while (true)
    {
        midi_event_t    *event;

        // Resize the track slightly larger to hold another event
        if (track->num_events == track->num_events_mem)
            new_events = I_Realloc(track->events, (track->num_events_mem += 100) * sizeof(midi_event_t));

        track->events = new_events;

        // Read the next event
        event = &track->events[track->num_events];

        if (!ReadEvent(event, &last_event_type, stream))
            return false;

        track->num_events++;

        // End of track?
        if (event->event_type == MIDI_EVENT_META && event->data.meta.type == MIDI_META_END_OF_TRACK)
            break;
    }

    return true;
}

// Free a track
static void FreeTrack(midi_track_t *track)
{
    for (unsigned int i = 0; i < track->num_events; i++)
        FreeEvent(&track->events[i]);

    free(track->events);
}

static bool ReadAllTracks(midi_file_t *file, SDL_RWops *stream)
{
    // Allocate list of tracks and read each track
    if (!(file->tracks = malloc(file->num_tracks * sizeof(midi_track_t))))
        return false;

    memset(file->tracks, 0, file->num_tracks * sizeof(midi_track_t));

    // Read each track
    for (unsigned int i = 0; i < file->num_tracks; i++)
        if (!ReadTrack(&file->tracks[i], stream))
            return false;

    return true;
}

// Read and check the header chunk
static bool ReadFileHeader(midi_file_t *file, SDL_RWops *stream)
{
    if (!SDL_RWread(stream, &file->header, sizeof(midi_header_t), 1))
        return false;

    if (!CheckChunkHeader(&file->header.chunk_header, HEADER_CHUNK_ID)
        || SDL_SwapBE32(file->header.chunk_header.chunk_size) != 6)
        return false;

    if ((file->num_tracks = SDL_SwapBE16(file->header.num_tracks)) < 1
        || SDL_SwapBE16(file->header.format_type) > 1)
        return false;

    return true;
}

void MIDI_FreeFile(midi_file_t *file)
{
    if (file->tracks)
    {
        for (unsigned int i = 0; i < file->num_tracks; i++)
            FreeTrack(&file->tracks[i]);

        free(file->tracks);
    }

    free(file);
}

midi_file_t *MIDI_LoadFile(SDL_RWops *stream)
{
    midi_file_t *file = calloc(1, sizeof(midi_file_t));

    if (!file)
        return NULL;

    // Open file
    if (!stream)
    {
        MIDI_FreeFile(file);
        return NULL;
    }

    // Read MIDI file header
    if (!ReadFileHeader(file, stream))
    {
        SDL_RWclose(stream);
        MIDI_FreeFile(file);

        return NULL;
    }

    // Read all tracks
    if (!ReadAllTracks(file, stream))
    {
        SDL_RWclose(stream);
        MIDI_FreeFile(file);

        return NULL;
    }

    SDL_RWclose(stream);
    return file;
}

// Get the number of tracks in a MIDI file.
unsigned int MIDI_NumTracks(midi_file_t *file)
{
    return file->num_tracks;
}

// Get the number of events in a MIDI file.
unsigned int MIDI_NumEvents(midi_file_t *file)
{
    unsigned int    num_events = 0;

    for (unsigned int i = 0; i < file->num_tracks; i++)
        num_events += file->tracks[i].num_events;

    return num_events;
}

// Start iterating over the events in a track.
midi_track_iter_t *MIDI_IterateTrack(midi_file_t *file, unsigned int track)
{
    midi_track_iter_t   *iter;

    assert(track < file->num_tracks);

    if ((iter = malloc(sizeof(*iter))))
    {
        iter->track = &file->tracks[track];
        iter->position = 0;
    }

    return iter;
}

// Get the time until the next MIDI event in a track.
unsigned int MIDI_GetDeltaTime(midi_track_iter_t *iter)
{
    if (iter->position < iter->track->num_events)
        return iter->track->events[iter->position].delta_time;

    return 0;
}

// Get a pointer to the next MIDI event.
bool MIDI_GetNextEvent(midi_track_iter_t *iter, midi_event_t **event)
{
    if (iter->position < iter->track->num_events)
    {
        *event = &iter->track->events[iter->position++];
        return true;
    }

    return false;
}

unsigned int MIDI_GetFileTimeDivision(midi_file_t *file)
{
    short   result = SDL_SwapBE16(file->header.time_division);

    // Negative time division indicates SMPTE time and must be handled differently.
    if (result < 0)
        return (-result / 256 * (result & 0xFF));

    return result;
}

#endif
