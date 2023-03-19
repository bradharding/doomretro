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

#include <stdio.h>
#include <string.h>

#include "doomdef.h"
#include "doomtype.h"
#include "i_swap.h"
#include "memio.h"
#include "mus2mid.h"
#include "SDL.h"

#define NUM_CHANNELS            16

#define MIDI_PERCUSSION_CHAN     9
#define MUS_PERCUSSION_CHAN     15

// MUS event codes
typedef enum
{
    mus_releasekey       = 0x00,
    mus_presskey         = 0x10,
    mus_pitchwheel       = 0x20,
    mus_systemevent      = 0x30,
    mus_changecontroller = 0x40,
    mus_scoreend         = 0x60
} musevent;

// MIDI event codes
typedef enum
{
    midi_releasekey        = 0x80,
    midi_presskey          = 0x90,
    midi_aftertouchkey     = 0xA0,
    midi_changecontroller  = 0xB0,
    midi_changepatch       = 0xC0,
    midi_aftertouchchannel = 0xD0,
    midi_pitchwheel        = 0xE0
} midievent;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

// Structure to hold MUS file header
typedef struct
{
    byte            id[4];
    unsigned short  scorelength;
    unsigned short  scorestart;
    unsigned short  primarychannels;
    unsigned short  secondarychannels;
    unsigned short  instrumentcount;
} PACKEDATTR musheader;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

// Standard MIDI type 0 header + track header
static const byte midiheader[] =
{
    'M', 'T', 'h', 'd',     // Main header
    0x00, 0x00, 0x00, 0x06, // Header size
    0x00, 0x00,             // MIDI type (0)
    0x00, 0x01,             // Number of tracks
    0x00, 0x46,             // Resolution
    'M', 'T', 'r', 'k',     // Start of track
    0x00, 0x00, 0x00, 0x00  // Placeholder for track length
};

// Cached channel velocities
static byte channelvelocities[] =
{
    127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127
};

// Timestamps between sequences of MUS events
static unsigned int queuedtime;

// Counter for the length of the track
static unsigned int tracksize;

static const byte controller_map[] =
{
    0x00, 0x20, 0x01, 0x07, 0x0A, 0x0B, 0x5B, 0x5D,
    0x40, 0x43, 0x78, 0x7B, 0x7E, 0x7F, 0x79
};

static int channel_map[NUM_CHANNELS];

// Write timestamp to a MIDI file
static bool WriteTime(unsigned int time, MEMFILE *midioutput)
{
    unsigned int    buffer = (time & 0x7F);

    while ((time >>= 7))
    {
        buffer <<= 8;
        buffer |= ((time & 0x7F) | 0x80);
    }

    while (true)
    {
        byte    writeval = (byte)(buffer & 0xFF);

        if (mem_fwrite(&writeval, 1, 1, midioutput) != 1)
            return true;

        tracksize++;

        if (buffer & 0x80)
            buffer >>= 8;
        else
        {
            queuedtime = 0;
            return false;
        }
    }
}

// Write the end of track marker
static bool WriteEndTrack(MEMFILE *midioutput)
{
    byte    endtrack[] = { 0xFF, 0x2F, 0x00 };

    if (WriteTime(queuedtime, midioutput))
        return true;

    if (mem_fwrite(endtrack, 1, 3, midioutput) != 3)
        return true;

    tracksize += 3;
    return false;
}

// Write a key press event
static bool WritePressKey(byte channel, byte key, byte velocity, MEMFILE *midioutput)
{
    byte    working = (midi_presskey | channel);

    if (WriteTime(queuedtime, midioutput))
        return true;

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    working = (key & 0x7F);

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    working = (velocity & 0x7F);

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    tracksize += 3;
    return false;
}

// Write a key release event
static bool WriteReleaseKey(byte channel, byte key, MEMFILE *midioutput)
{
    byte    working = (midi_releasekey | channel);

    if (WriteTime(queuedtime, midioutput))
        return true;

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    working = (key & 0x7F);

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    working = 0;

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    tracksize += 3;
    return false;
}

// Write a pitch wheel/bend event
static bool WritePitchWheel(byte channel, short wheel, MEMFILE *midioutput)
{
    byte    working = (midi_pitchwheel | channel);

    if (WriteTime(queuedtime, midioutput))
        return true;

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    working = (wheel & 0x7F);

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    working = ((wheel >> 7) & 0x7F);

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    tracksize += 3;
    return false;
}

// Write a patch change event
static bool WriteChangePatch(byte channel, byte patch, MEMFILE *midioutput)
{
    byte    working = (midi_changepatch | channel);

    if (WriteTime(queuedtime, midioutput))
        return true;

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    working = (patch & 0x7F);

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    tracksize += 2;
    return false;
}

// Write a valued controller change event
static bool WriteChangeController_Valued(byte channel, byte control, byte value, MEMFILE *midioutput)
{
    byte    working = (midi_changecontroller | channel);

    if (WriteTime(queuedtime, midioutput))
        return true;

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    working = (control & 0x7F);

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    // Quirk in Vanilla DOOM? MUS controller values should be 7-bit, not 8-bit
    working = value/* & 0x7F*/;

    // Fix on said quirk to stop MIDI players from complaining that the value is out of range
    if (working & 0x80)
        working = 0x7F;

    if (mem_fwrite(&working, 1, 1, midioutput) != 1)
        return true;

    tracksize += 3;
    return false;
}

// Write a valueless controller change event
static bool WriteChangeController_Valueless(byte channel, byte control, MEMFILE *midioutput)
{
    return WriteChangeController_Valued(channel, control, 0, midioutput);
}

// Allocate a free MIDI channel
static int AllocateMIDIChannel(void)
{
    int result;
    int max = -1;

    // Find the current highest-allocated channel.
    for (int i = 0; i < NUM_CHANNELS; i++)
        if (channel_map[i] > max)
            max = channel_map[i];

    // max is now equal to the highest-allocated MIDI channel
    // We can now allocate the next available channel
    // This also works if no channels are currently allocated (max = -1)
    result = max + 1;

    // Don't allocate the MIDI percussion channel!
    if (result == MIDI_PERCUSSION_CHAN)
        result++;

    return result;
}

// Given a MUS channel number, get the MIDI channel number to use
// in the outputted file.
static int GetMIDIChannel(int mus_channel, MEMFILE *midioutput)
{
    // Find the MIDI channel to use for this MUS channel.
    // MUS channel 15 is the percussion channel.
    if (mus_channel == MUS_PERCUSSION_CHAN)
        return MIDI_PERCUSSION_CHAN;
    else
    {
        // If a MIDI channel hasn't been allocated for this MUS channel
        // yet, allocate the next free MIDI channel.
        if (channel_map[mus_channel] == -1)
        {
            channel_map[mus_channel] = AllocateMIDIChannel();

            // First time using the channel, send an "all notes off"
            // event. This fixes "The D_DDTBLU disease" described here:
            // <https://www.doomworld.com/forum/topic/66802/>
            WriteChangeController_Valueless(channel_map[mus_channel], 0x7B, midioutput);
        }

        return channel_map[mus_channel];
    }
}

static bool ReadMusHeader(MEMFILE *file, musheader *header)
{
    if (mem_fread(&header->id, sizeof(byte), 4, file) == 4
        && mem_fread(&header->scorelength, sizeof(short), 1, file) == 1
        && mem_fread(&header->scorestart, sizeof(short), 1, file) == 1
        && mem_fread(&header->primarychannels, sizeof(short), 1, file) == 1
        && mem_fread(&header->secondarychannels, sizeof(short), 1, file) == 1
        && mem_fread(&header->instrumentcount, sizeof(short), 1, file) == 1)
    {
        header->scorelength = SHORT(header->scorelength);
        header->scorestart = SHORT(header->scorestart);
        header->primarychannels = SHORT(header->primarychannels);
        header->secondarychannels = SHORT(header->secondarychannels);
        header->instrumentcount = SHORT(header->instrumentcount);

        return true;
    }

    return false;
}

// Read a MUS file from a stream (musinput) and output a MIDI file to a stream (midioutput).
bool mus2mid(MEMFILE *musinput, MEMFILE *midioutput)
{
    // Header for the MUS file
    musheader   musfileheader;

    // Buffer used for MIDI track size record
    byte        tracksizebuffer[4] = { 0 };

    // Flag for when the score end marker is hit.
    int         hitscoreend = 0;

    // Initialize channel map to mark all channels as unused.
    for (int i = 0; i < NUM_CHANNELS; i++)
        channel_map[i] = -1;

    // Grab the header
    if (!ReadMusHeader(musinput, &musfileheader))
        return false;

    // Seek to where the data is held
    if (mem_fseek(musinput, (long)musfileheader.scorestart, MEM_SEEK_SET))
        return false;

    // So, we can assume the MUS file is faintly legit. Let's start writing MIDI data...
    mem_fwrite(midiheader, 1, sizeof(midiheader), midioutput);
    tracksize = 0;

    // Now, process the MUS file
    while (!hitscoreend)
    {
        // Handle a block of events
        while (!hitscoreend)
        {
            byte    eventdescriptor;
            byte    key;
            byte    controllernumber;
            byte    controllervalue;
            int     channel;

            // Fetch channel number and event code
            if (mem_fread(&eventdescriptor, 1, 1, musinput) != 1)
                return false;

            channel = GetMIDIChannel((eventdescriptor & 0x0F), midioutput);

            switch (eventdescriptor & 0x70)
            {
                case mus_releasekey:
                    if (mem_fread(&key, 1, 1, musinput) != 1)
                        return false;

                    if (WriteReleaseKey(channel, key, midioutput))
                        return false;

                    break;

                case mus_presskey:
                    if (mem_fread(&key, 1, 1, musinput) != 1)
                        return false;

                    if (key & 0x80)
                    {
                        if (mem_fread(&channelvelocities[channel], 1, 1, musinput) != 1)
                            return false;

                        channelvelocities[channel] &= 0x7F;
                    }

                    if (WritePressKey(channel, key, channelvelocities[channel], midioutput))
                        return false;

                    break;

                case mus_pitchwheel:
                    if (mem_fread(&key, 1, 1, musinput) != 1)
                        break;

                    if (WritePitchWheel(channel, (short)(key * 64), midioutput))
                        return false;

                    break;

                case mus_systemevent:
                    if (mem_fread(&controllernumber, 1, 1, musinput) != 1)
                        return false;

                    if (controllernumber < 10 || controllernumber > 14)
                        return false;

                    if (WriteChangeController_Valueless(channel, controller_map[controllernumber], midioutput))
                        return false;

                    break;

                case mus_changecontroller:
                    if (mem_fread(&controllernumber, 1, 1, musinput) != 1)
                        return false;

                    if (mem_fread(&controllervalue, 1, 1, musinput) != 1)
                        return false;

                    if (!controllernumber)
                    {
                        if (WriteChangePatch(channel, controllervalue, midioutput))
                            return false;
                    }
                    else
                    {
                        if (controllernumber > 9)
                            return false;

                        if (WriteChangeController_Valued(channel, controller_map[controllernumber], controllervalue, midioutput))
                            return false;
                    }

                    break;

                case mus_scoreend:
                    hitscoreend = 1;
                    break;

                default:
                    return false;
            }

            if (eventdescriptor & 0x80)
                break;
        }

        // Now we need to read the time code
        if (!hitscoreend)
        {
            unsigned int    timedelay = 0;

            while (true)
            {
                byte    working;

                if (mem_fread(&working, 1, 1, musinput) != 1)
                    return false;

                timedelay = timedelay * 128 + (working & 0x7F);

                if (!(working & 0x80))
                    break;
            }

            queuedtime += timedelay;
        }
    }

    // End of track
    if (WriteEndTrack(midioutput))
        return false;

    // Write the track size into the stream
    if (mem_fseek(midioutput, 18, MEM_SEEK_SET))
        return false;

    tracksizebuffer[0] = ((tracksize >> 24) & 0xFF);
    tracksizebuffer[1] = ((tracksize >> 16) & 0xFF);
    tracksizebuffer[2] = ((tracksize >> 8) & 0xFF);
    tracksizebuffer[3] = (tracksize & 0xFF);

    if (mem_fwrite(tracksizebuffer, 1, 4, midioutput) != 4)
        return false;

    return true;
}
