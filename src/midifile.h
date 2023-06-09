/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

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

==============================================================================
*/

#if defined(_WIN32)

#pragma once

#include "doomdef.h"
#include "doomtype.h"
#include "SDL.h"

#define HEADER_CHUNK_ID         "MThd"
#define TRACK_CHUNK_ID          "MTrk"
#define MIDI_CHANNELS_PER_TRACK 16

typedef enum
{
    MIDI_EVENT_NOTE_OFF        = 0x80,
    MIDI_EVENT_NOTE_ON         = 0x90,
    MIDI_EVENT_AFTERTOUCH      = 0xA0,
    MIDI_EVENT_CONTROLLER      = 0xB0,
    MIDI_EVENT_PROGRAM_CHANGE  = 0xC0,
    MIDI_EVENT_CHAN_AFTERTOUCH = 0xD0,
    MIDI_EVENT_PITCH_BEND      = 0xE0,

    MIDI_EVENT_SYSEX           = 0xF0,
    MIDI_EVENT_SYSEX_SPLIT     = 0xF7,
    MIDI_EVENT_META            = 0xFF
} midi_event_type_t;

typedef enum
{
    MIDI_CONTROLLER_BANK_SELECT    = 0x00,
    MIDI_CONTROLLER_MODULATION     = 0x01,
    MIDI_CONTROLLER_BREATH_CONTROL = 0x02,
    MIDI_CONTROLLER_FOOT_CONTROL   = 0x03,
    MIDI_CONTROLLER_PORTAMENTO     = 0x04,
    MIDI_CONTROLLER_DATA_ENTRY     = 0x05,

    MIDI_CONTROLLER_MAIN_VOLUME    = 0x07,
    MIDI_CONTROLLER_PAN            = 0x0A,

    MIDI_CONTROLLER_ALL_NOTES_OFF  = 0x7B
} midi_controller_t;

typedef enum
{
    MIDI_META_SEQUENCE_NUMBER    = 0x00,

    MIDI_META_TEXT               = 0x01,
    MIDI_META_COPYRIGHT          = 0x02,
    MIDI_META_TRACK_NAME         = 0x03,
    MIDI_META_INSTR_NAME         = 0x04,
    MIDI_META_LYRICS             = 0x05,
    MIDI_META_MARKER             = 0x06,
    MIDI_META_CUE_POINT          = 0x07,

    MIDI_META_CHANNEL_PREFIX     = 0x20,
    MIDI_META_END_OF_TRACK       = 0x2F,

    MIDI_META_SET_TEMPO          = 0x51,
    MIDI_META_SMPTE_OFFSET       = 0x54,
    MIDI_META_TIME_SIGNATURE     = 0x58,
    MIDI_META_KEY_SIGNATURE      = 0x59,
    MIDI_META_SEQUENCER_SPECIFIC = 0x7F
} midi_meta_event_type_t;

typedef struct
{
    unsigned int        type;       // Meta event type
    unsigned int        length;     // Length
    byte                *data;      // Meta event data
} midi_meta_event_data_t;

typedef struct
{
    unsigned int        length;     // Length
    byte                *data;      // Event data
} midi_sysex_event_data_t;

typedef struct
{
    unsigned int        channel;    // The channel number to which this applies
    unsigned int        param1;     // Extra parameter
    unsigned int        param2;     // Extra parameter
} midi_channel_event_data_t;

typedef struct
{
    unsigned int        delta_time; // Time between the previous event and this event
    midi_event_type_t   event_type; // Type of event

    union
    {
        midi_channel_event_data_t   channel;
        midi_meta_event_data_t      meta;
        midi_sysex_event_data_t     sysex;
    } data;
} midi_event_t;

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
    unsigned int    num_events_mem;
} midi_track_t;

typedef struct
{
    midi_track_t    *track;
    unsigned int    position;
} midi_track_iter_t;

typedef struct
{
    midi_header_t   header;

    // All tracks in this file
    midi_track_t    *tracks;
    unsigned int    num_tracks;

    // Data buffer used to store data read for SysEx or meta events
    byte            *buffer;
    unsigned int    buffer_size;
} midi_file_t;

// Load a MIDI file.
midi_file_t *MIDI_LoadFile(SDL_RWops *stream);

// Free a MIDI file.
void MIDI_FreeFile(midi_file_t *file);

// Get the time division value from the MIDI header.
unsigned int MIDI_GetFileTimeDivision(midi_file_t *file);

// Get the number of tracks in a MIDI file.
unsigned int MIDI_NumTracks(midi_file_t *file);

// Get the number of events in a MIDI file.
unsigned int MIDI_NumEvents(midi_file_t *file);

// Start iterating over the events in a track.
midi_track_iter_t *MIDI_IterateTrack(midi_file_t *file, unsigned int track);

// Get the time until the next MIDI event in a track.
unsigned int MIDI_GetDeltaTime(midi_track_iter_t *iter);

// Get a pointer to the next MIDI event.
bool MIDI_GetNextEvent(midi_track_iter_t *iter, midi_event_t **event);

#endif
