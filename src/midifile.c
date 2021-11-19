/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2021 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2021 by Brad Harding <mailto:brad@doomretro.com>.

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

#if defined(_WIN32)

#include <assert.h>
#include <string.h>

#include "doomtype.h"
#include "i_swap.h"
#include "i_system.h"
#include "midifile.h"

#define HEADER_CHUNK_ID "MThd"
#define TRACK_CHUNK_ID  "MTrk"

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

typedef struct
{
    byte            chunk_id[4];
    unsigned int    chunk_size;
} PACKEDATTR chunk_header_t;

typedef struct
{
    chunk_header_t  chunk_header;
    unsigned short  format_type;
    unsigned short  num_tracks;
    unsigned short  time_division;
} PACKEDATTR midi_header_t;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

typedef struct
{
    // Length in bytes
    unsigned int    data_len;

    // Events in this track
    midi_event_t    *events;
    unsigned int    num_events;
} midi_track_t;

struct midi_track_iter_s
{
    midi_track_t    *track;
    unsigned int    position;
};

struct midi_file_s
{
    midi_header_t   header;

    // All tracks in this file
    midi_track_t    *tracks;
    unsigned int    num_tracks;

    // Data buffer used to store data read for SysEx or meta events
    byte            *buffer;
    unsigned int    buffer_size;
};

// Check the header of a chunk
static dboolean CheckChunkHeader(chunk_header_t *chunk, const char *expected_id)
{
    return !memcmp((char*)chunk->chunk_id, expected_id, 4);
}

// Read a single byte. Return false on error.
static dboolean ReadByte(byte *result, FILE *stream)
{
    int c = fgetc(stream);

    if (c == EOF)
        return false;
    else
    {
        *result = (byte)c;
        return true;
    }
}

// Read a variable-length value.
static dboolean ReadVariableLength(unsigned int *result, FILE *stream)
{
    byte    b = 0;

    *result = 0;

    for (int i = 0; i < 4; i++)
    {
        if (!ReadByte(&b, stream))
            return false;

        // Insert the bottom seven bits from this byte.
        *result <<= 7;
        *result |= b & 0x7F;

        // If the top bit is not set, this is the end.
        if (!(b & 0x80))
            return true;
    }

    return false;
}

// Read a byte sequence into the data buffer.
static void *ReadByteSequence(unsigned int num_bytes, FILE *stream)
{
    // Allocate a buffer. Allocate one extra byte, as malloc(0) is non-portable.

    byte    *result = malloc(num_bytes + 1);

    if (!result)
        return NULL;

    // Read the data:

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
static dboolean ReadChannelEvent(midi_event_t *event, byte event_type, dboolean two_param, FILE *stream)
{
    byte    b = 0;

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
static dboolean ReadSysExEvent(midi_event_t *event, int event_type, FILE *stream)
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
static dboolean ReadMetaEvent(midi_event_t *event, FILE *stream)
{
    byte    b = 0;

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

static dboolean ReadEvent(midi_event_t *event, unsigned int *last_event_type, FILE *stream)
{
    byte    event_type = 0;

    if (!ReadVariableLength(&event->delta_time, stream))
        return false;

    if (!ReadByte(&event_type, stream))
        return false;

    // All event types have their top bit set.  Therefore, if
    // the top bit is not set, it is because we are using the "same
    // as previous event type" shortcut to save a byte.  Skip back
    // a byte so that we read this byte again.
    if (!(event_type & 0x80))
    {
        event_type = *last_event_type;

        if (fseek(stream, -1, SEEK_CUR) < 0)
            return false;
    }
    else
        *last_event_type = event_type;

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
            // Nothing to do.
            break;
    }
}

// Read and check the track chunk header
static dboolean ReadTrackHeader(midi_track_t *track, FILE *stream)
{
    chunk_header_t  chunk_header;
    size_t          records_read = fread(&chunk_header, sizeof(chunk_header_t), 1, stream);

    if (records_read < 1)
        return false;

    if (!CheckChunkHeader(&chunk_header, TRACK_CHUNK_ID))
        return false;

    track->data_len = SDL_SwapBE32(chunk_header.chunk_size);

    return true;
}

static dboolean ReadTrack(midi_track_t *track, FILE *stream)
{
    midi_event_t    *new_events;
    unsigned int    last_event_type;

    track->num_events = 0;
    track->events = NULL;

    // Read the header
    if (!ReadTrackHeader(track, stream))
        return false;

    // Then the events
    last_event_type = 0;

    while (true)
    {
        midi_event_t    *event;

        // Resize the track slightly larger to hold another event
        new_events = I_Realloc(track->events, (track->num_events + 1) * sizeof(midi_event_t));
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

static dboolean ReadAllTracks(midi_file_t *file, FILE *stream)
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
static dboolean ReadFileHeader(midi_file_t *file, FILE *stream)
{
    unsigned int    format_type;

    if (fread(&file->header, sizeof(midi_header_t), 1, stream) < 1)
        return false;

    if (!CheckChunkHeader(&file->header.chunk_header, HEADER_CHUNK_ID)
        || SDL_SwapBE32(file->header.chunk_header.chunk_size) != 6)
        return false;

    format_type = SDL_SwapBE16(file->header.format_type);
    file->num_tracks = SDL_SwapBE16(file->header.num_tracks);

    if ((format_type != 0 && format_type != 1) || file->num_tracks < 1)
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

midi_file_t *MIDI_LoadFile(char *filename)
{
    midi_file_t *file = malloc(sizeof(midi_file_t));
    FILE        *stream;

    if (!file)
        return NULL;

    file->tracks = NULL;
    file->num_tracks = 0;
    file->buffer = NULL;
    file->buffer_size = 0;

    // Open file
    stream = fopen(filename, "rb");

    if (!stream)
    {
        MIDI_FreeFile(file);
        return NULL;
    }

    // Read MIDI file header
    if (!ReadFileHeader(file, stream))
    {
        fclose(stream);
        MIDI_FreeFile(file);

        return NULL;
    }

    // Read all tracks
    if (!ReadAllTracks(file, stream))
    {
        fclose(stream);
        MIDI_FreeFile(file);

        return NULL;
    }

    fclose(stream);

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

    iter = malloc(sizeof(*iter));
    iter->track = &file->tracks[track];
    iter->position = 0;

    return iter;
}

// Get the time until the next MIDI event in a track.
unsigned int MIDI_GetDeltaTime(midi_track_iter_t *iter)
{
    if (iter->position < iter->track->num_events)
    {
        midi_event_t    *next_event = &iter->track->events[iter->position];

        return next_event->delta_time;
    }
    else
        return 0;
}

// Get a pointer to the next MIDI event.
dboolean MIDI_GetNextEvent(midi_track_iter_t *iter, midi_event_t **event)
{
    if (iter->position < iter->track->num_events)
    {
        *event = &iter->track->events[iter->position];
        ++iter->position;

        return true;
    }
    else
        return false;
}

unsigned int MIDI_GetFileTimeDivision(midi_file_t *file)
{
    short   result = SDL_SwapBE16(file->header.time_division);

    // Negative time division indicates SMPTE time and must be handled differently.
    if (result < 0)
        return ((signed int)(-result / 256) * (signed int)(result & 0xFF));
    else
        return result;
}

#endif
