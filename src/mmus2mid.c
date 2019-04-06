/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

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

#include <string.h>

#include "i_system.h"
#include "mmus2mid.h"

// some macros to decode mus event bit fields
#define last(e)         ((UBYTE)((e) & 0x80))
#define event_type(e)   ((UBYTE)(((e) & 0x7F) >> 4))
#define channel(e)      ((UBYTE)((e) & 0x0F))

// event types
enum
{
    RELEASE_NOTE,
    PLAY_NOTE,
    BEND_NOTE,
    SYS_EVENT,
    CNTL_CHANGE,
    UNKNOWN_EVENT1,
    SCORE_END,
    UNKNOWN_EVENT2
};

// MUS format header structure

// haleyjd 04/05/05: this structure is read directly from memory
// and so it should be packed
#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

struct tagMUSheader
{
    char    ID[4];          // identifier "MUS"0x1A
    UWORD   ScoreLength;    // length of music portion
    UWORD   ScoreStart;     // offset of music portion
    UWORD   channels;       // count of primary channels
    UWORD   SecChannels;    // count of secondary channels
    UWORD   InstrCnt;       // number of instruments
};

typedef struct tagMUSheader MUSheader;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

// to keep track of information in a MIDI track
typedef struct
{
    char    velocity;
    int     deltaT;
    UBYTE   lastEvt;
    int     alloced;
} TrackInfo;

// array of info about tracks
static TrackInfo track[MIDI_TRACKS];

// initial track size allocation
static ULONG TRACKBUFFERSIZE = 1024L;

// lookup table MUS -> MID controls
static UBYTE MUS2MIDcontrol[15] =
{
    0,      // Program change - not a MIDI control change
    0x00,   // Bank select
    0x01,   // Modulation pot
    0x07,   // Volume
    0x0A,   // Pan pot
    0x0B,   // Expression pot
    0x5B,   // Reverb depth
    0x5D,   // Chorus depth
    0x40,   // Sustain pedal
    0x43,   // Soft pedal
    0x78,   // All sounds off
    0x7B,   // All notes off
    0x7E,   // Mono
    0x7F,   // Poly
    0x79    // Reset all controllers
};

// some strings of bytes used in the midi format
static UBYTE    midikey[] = { 0x00, 0xFF, 0x59, 0x02, 0x00, 0x00 };                 // C major
static UBYTE    miditempo[] = { 0x00, 0xFF, 0x51, 0x03, 0x09, 0xA3, 0x1A };         // uS/qnote
static UBYTE    midihdr[] = { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 0, 0, 0 };   // header (length 6, format 1)
static UBYTE    trackhdr[] = { 'M', 'T', 'r', 'k' };                                // track header

//
// TWriteByte()
//
// write one byte to the selected MIDItrack, update current position
// if track allocation exceeded, double it
// if track not allocated, initially allocate TRACKBUFFERSIZE bytes
//
// Passed pointer to Allegro MIDI structure, number of the MIDI track being
// written, and the byte to write.
//
// proff: changed type for byte from char to unsigned char to avoid warning
//
static void TWriteByte(MIDI *mididata, int MIDItrack, unsigned char byte)
{
    size_t  pos = mididata->track[MIDItrack].len;

    // proff: Added typecast to avoid warning
    if (pos >= (size_t)track[MIDItrack].alloced)
    {
        // double allocation or set initial TRACKBUFFERSIZE
        track[MIDItrack].alloced = (track[MIDItrack].alloced ? 2 * track[MIDItrack].alloced : TRACKBUFFERSIZE);

        // attempt to reallocate
        mididata->track[MIDItrack].data = (unsigned char *)I_Realloc(mididata->track[MIDItrack].data, track[MIDItrack].alloced);
    }

    mididata->track[MIDItrack].data[pos] = byte;
    mididata->track[MIDItrack].len++;
}

//
// TWriteVarLen()
//
// write the ULONG value to MIDItrack track, in midi format, which is
// big endian, 7 bits per byte, with all bytes but the last flagged by
// bit 8 being set, allowing the length to vary.
//
// Passed the Allegro MIDI structure, the track number to write,
// and the ULONG value to encode in midi format there
//
static void TWriteVarLen(MIDI *mididata, int MIDItrack, ULONG value)
{
    ULONG   buffer = value & 0x7F;

    while ((value >>= 7))               // terminates because value unsigned
    {
        buffer <<= 8;                   // note first value shifted in has bit 8 clear
        buffer |= 0x80;                 // all succeeding values do not
        buffer += (value & 0x7F);
    }

    while (true)                        // write bytes out in opposite order
    {
        TWriteByte(mididata, MIDItrack, (char)(buffer & 0xFF));

        if (buffer & 0x80)
            buffer >>= 8;
        else                            // terminate on the byte with bit 8 clear
            break;
    }
}

//
// ReadTime()
//
// Read a time value from the MUS buffer, advancing the position in it
//
// A time value is a variable length sequence of 8 bit bytes, with all
// but the last having bit 8 set.
//
// Passed a pointer to the pointer to the MUS buffer
// Returns the integer unsigned long time value there and advances the pointer
//
static ULONG ReadTime(UBYTE **musptrp)
{
    ULONG   timeval = 0;
    int     byte;

    // shift each byte read up in the result until a byte with bit 8 clear
    do
    {
        byte = *(*musptrp)++;
        timeval = (timeval << 7) + (byte & 0x7F);
    } while (byte & 0x80);

    return timeval;
}

//
// FirstChannelAvailable()
//
// Return the next unassigned MIDI channel number
//
// The assignment for MUS channel 15 is not counted in the calculation, that
// being percussion and always assigned to MIDI channel 9 (base 0).
//
// Passed the array of MIDI channels assigned to MUS channels
// Returns the maximum channel number unassigned unless that is 9 in which
// case 10 is returned.
//
static char FirstChannelAvailable(signed char MUS2MIDchannel[])
{
    signed char max = -1;

    // find the largest MIDI channel assigned so far
    for (int i = 0; i < 15; i++)
        if (MUS2MIDchannel[i] > max)
            max = MUS2MIDchannel[i];

    return (max == 8 ? 10 : max + 1);   // skip MIDI channel 9 (percussion)
}

//
// MidiEvent()
//
// Constructs a MIDI event code, and writes it to the current MIDI track
// unless its the same as the last event code and compression is enabled
// in which case nothing is written.
//
// Passed the Allegro MIDI structure, the midi event code, the current
// MIDI channel number, the current MIDI track number, and whether compression
// (running status) is enabled.
//
// Returns the new event code if successful, 0 if a memory allocation error
//
static UBYTE MidiEvent(MIDI *mididata, UBYTE midicode, UBYTE MIDIchannel, UBYTE MIDItrack)
{
    UBYTE   newevent = (midicode | MIDIchannel);

    if (newevent != track[MIDItrack].lastEvt)
    {
        TWriteByte(mididata, MIDItrack, newevent);
        track[MIDItrack].lastEvt = newevent;
    }

    return newevent;
}

#define MAX_HEADER_SCAN 32

//
// mmuscheckformat
//
// haleyjd 11/23/12:
// Returns true if the data is a MUS.
//
dboolean mmuscheckformat(UBYTE *mus, int size)
{
    UBYTE   *hptr = mus;

    while (hptr < mus + size - sizeof(MUSheader) && hptr < mus + MAX_HEADER_SCAN && strncmp((const char *)hptr, "MUS\x1a", 4))
        hptr++;

    if (hptr < mus + size - sizeof(MUSheader) && !strncmp((const char *)hptr, "MUS\x1a", 4))
        return true;

    return false;
}

//
// mmus2mid()
//
// Convert a memory buffer containing MUS data to an Allegro MIDI structure
// with specified time division and compression.
//
// Passed a pointer to the buffer containing MUS data, a pointer to the
// Allegro MIDI structure, the divisions, and a flag whether to compress.
//
// Returns 0 if successful, otherwise an error code (see mmus2mid.h).
//
dboolean mmus2mid(UBYTE *mus, size_t size, MIDI *mididata)
{
    UWORD               TrackCnt = 0;
    UBYTE               evt;
    UBYTE               MIDIchannel;
    UBYTE               MIDItrack;
    int                 data;
    UBYTE               *musptr;
    UBYTE               *hptr;
    size_t              muslen;
    static MUSheader    MUSh;
    UBYTE               MIDIchan2track[MIDI_TRACKS];
    signed char         MUS2MIDchannel[MIDI_TRACKS];

    // haleyjd 04/04/10: don't bite off more than you can chew
    if (size < sizeof(MUSheader))
        return false;

    // haleyjd 04/04/10: scan forward for a MUS header. Evidently DMX was
    // capable of doing this, and would skip over any intervening data. That,
    // or DMX doesn't use the MUS header at all somehow.
    hptr = mus;

    while (hptr < mus + size - sizeof(MUSheader) && hptr < mus + MAX_HEADER_SCAN && strncmp((const char *)hptr, "MUS\x1a", 4))
        hptr++;

    // if we found a likely header start, reset the mus pointer to that location,
    // otherwise just leave it alone and pray.
    if (hptr < mus + size - sizeof(MUSheader) && !strncmp((const char *)hptr, "MUS\x1a", 4))
        mus = hptr;

    // copy the MUS header from the MUS buffer to the MUSh header structure
    memcpy(&MUSh, mus, sizeof(MUSheader));

    // check some things and set length of MUS buffer from internal data
    if (!(muslen = (size_t)MUSh.ScoreLength + MUSh.ScoreStart))
        return false;                       // MUS file empty

    if (MUSh.channels > 15)                 // MUSchannels + drum channel > 16
        return false;

    musptr = mus + MUSh.ScoreStart;         // init musptr to start of score

    for (int i = 0; i < MIDI_TRACKS; i++)   // init the track structure's tracks
    {
        MUS2MIDchannel[i] = -1;             // flag for channel not used yet
        track[i].velocity = 64;
        track[i].deltaT = 0;
        track[i].lastEvt = 0;
        free(mididata->track[i].data);      // jff 3/5/98 remove old allocations
        mididata->track[i].data = NULL;
        track[i].alloced = 0;
        mididata->track[i].len = 0;
    }

    // allocate the first track which is a special tempo/key track
    // note multiple tracks means midi format 1

    // set the divisions (ticks per quarter note)
    mididata->divisions = 89;

    // allocate for midi tempo/key track, allow for end of track
    mididata->track[0].data = (unsigned char *)I_Realloc(mididata->track[0].data, sizeof(midikey) + sizeof(miditempo) + 4);

    // key C major
    memcpy(mididata->track[0].data, midikey, sizeof(midikey));

    // tempo uS/qnote
    memcpy(mididata->track[0].data + sizeof(midikey), miditempo, sizeof(miditempo));
    mididata->track[0].len = sizeof(midikey) + sizeof(miditempo);

    TrackCnt++;   // music tracks start at 1

    // process the MUS events in the MUS buffer
    do
    {
        UBYTE   MUSchannel;

        // get a mus event, decode its type and channel fields
        int     event = *musptr++;

        if ((evt = event_type(event)) == SCORE_END)     // jff 1/23/98 use symbol
            break;                                      // if end of score event, leave

        MUSchannel = channel(event);

        // if this channel not initialized, do so
        if (MUS2MIDchannel[MUSchannel] == -1)
        {
            // set MIDIchannel and MIDItrack
            MIDIchannel = MUS2MIDchannel[MUSchannel] = (MUSchannel == 15 ? 9 : FirstChannelAvailable(MUS2MIDchannel));

            // proff: Added typecast to avoid warning
            MIDItrack = MIDIchan2track[MIDIchannel] = (unsigned char)TrackCnt++;

            TWriteByte(mididata, MIDItrack, 0x00);
            TWriteByte(mididata, MIDItrack, (0xB0 | MIDIchannel));
            TWriteByte(mididata, MIDItrack, 0x7B);
            TWriteByte(mididata, MIDItrack, 0x00);
        }
        else    // channel already allocated as a track, use those values
        {
            MIDIchannel = MUS2MIDchannel[MUSchannel];
            MIDItrack = MIDIchan2track[MIDIchannel];
        }

        TWriteVarLen(mididata, MIDItrack, track[MIDItrack].deltaT);

        track[MIDItrack].deltaT = 0;

        switch (evt)
        {
            case RELEASE_NOTE:
                if (!MidiEvent(mididata, 0x90, MIDIchannel, MIDItrack))
                    return false;

                data = *musptr++;

                TWriteByte(mididata, MIDItrack, (unsigned char)(data & 0x7F));
                TWriteByte(mididata, MIDItrack, 0);
                break;

            case PLAY_NOTE:
                if (!MidiEvent(mididata, 0x90, MIDIchannel, MIDItrack))
                    return false;

                data = *musptr++;

                TWriteByte(mididata, MIDItrack, (unsigned char)(data & 0x7F));

                if (data & 0x80)
                    track[MIDItrack].velocity = (*musptr++) & 0x7F;

                TWriteByte(mididata, MIDItrack, track[MIDItrack].velocity);
                break;

            case BEND_NOTE:
                if (!MidiEvent(mididata, 0xE0, MIDIchannel, MIDItrack))
                    return false;

                data = *musptr++;

                TWriteByte(mididata, MIDItrack, (unsigned char)((data & 1) << 6));
                TWriteByte(mididata, MIDItrack, (unsigned char)(data >> 1));
                break;

            case SYS_EVENT:
                if (!MidiEvent(mididata, 0xB0, MIDIchannel, MIDItrack))
                    return false;

                data = *musptr++;

                if (data < 10 || data > 14)
                    return false;

                TWriteByte(mididata, MIDItrack, MUS2MIDcontrol[data]);

                if (data == 12)
                    TWriteByte(mididata, MIDItrack, (unsigned char)(MUSh.channels + 1));
                else
                    TWriteByte(mididata, MIDItrack, 0);

                break;

            case CNTL_CHANGE:
                data = *musptr++;

                if (data > 9)
                    return false;

                if (data)
                {
                    if (!MidiEvent(mididata, 0xB0, MIDIchannel, MIDItrack))
                        return false;

                    TWriteByte(mididata, MIDItrack, MUS2MIDcontrol[data]);
                }
                else if (!MidiEvent(mididata, 0xC0, MIDIchannel, MIDItrack))
                    return false;

                data = *musptr++;

                // Gez: Fix TNT.WAD's D_STALKS, based on Ben Ryves's fix in MUS2MID
                if (data & 0x80)
                    data = 0x7F;

                TWriteByte(mididata, MIDItrack, (unsigned char)data);
                break;

            case SCORE_END:
                break;

            case UNKNOWN_EVENT1:   // mus events 5 and 7
            case UNKNOWN_EVENT2:   // meaning not known
            default:
                return false;
        }

        if (last(event))
        {
            ULONG   DeltaTime = ReadTime(&musptr);

            for (int i = 0; i < MIDI_TRACKS; i++)   // jff 3/13/98 update all tracks
                track[i].deltaT += DeltaTime;       // whether allocated yet or not
        }
    } while (evt != SCORE_END && (size_t)(musptr - mus) < muslen);

    if (evt != SCORE_END)
        return false;

    // Now add an end of track to each mididata track, correct allocation
    for (int i = 0; i < MIDI_TRACKS; i++)
    {
        if (mididata->track[i].len)
        {
            TWriteByte(mididata, i, 0x00);
            TWriteByte(mididata, i, 0xFF);
            TWriteByte(mididata, i, 0x2F);
            TWriteByte(mididata, i, 0x00);

            // jff 1/23/98 fix failure to set data NULL, len 0 for unused tracks
            // shorten allocation to proper length (important for Allegro)
            mididata->track[i].data = (unsigned char *)I_Realloc(mididata->track[i].data, mididata->track[i].len);
        }
        else
        {
            free(mididata->track[i].data);
            mididata->track[i].data = NULL;
        }
    }

    return true;
}

//
// TWriteLength()
//
// Write the length of a MIDI chunk to a midi buffer. The length is four
// bytes and is written byte-reversed for bigendian. The pointer to the
// midi buffer is advanced.
//
// Passed a pointer to the pointer to a midi buffer, and the length to write
// Returns nothing
//
static void TWriteLength(UBYTE **midiptr, size_t length)
{
    // proff: Added typecast to avoid warning
    *(*midiptr)++ = (unsigned char)((length >> 24) & 0xFF);
    *(*midiptr)++ = (unsigned char)((length >> 16) & 0xFF);
    *(*midiptr)++ = (unsigned char)((length >> 8) & 0xFF);
    *(*midiptr)++ = (unsigned char)(length & 0xFF);
}

//
// Frees all midi data allocated
//
void FreeMIDIData(MIDI *mididata)
{
    for (int i = 0; i < arrlen(mididata->track); i++)
        free(mididata->track[i].data);

    memset(mididata, 0, sizeof(*mididata));
}

//
// MIDIToMidi()
//
// This routine converts an Allegro MIDI structure to a midi 1 format file
// in memory. It is used to support memory MUS -> MIDI conversion
//
// Passed a pointer to an Allegro MIDI structure, a pointer to a pointer to
// a buffer containing midi data, and a pointer to a length return.
//
void MIDIToMidi(const MIDI *mididata, UBYTE **mid, int *midlen)
{
    int     ntrks = 0;
    UBYTE   *midiptr;

    // calculate how long the mid buffer must be, and allocate
    size_t  total = sizeof(midihdr);

    for (int i = 0; i < MIDI_TRACKS; i++)
        if (mididata->track[i].len)
        {
            total += 8 + mididata->track[i].len;        // Track hdr + track length
            ntrks++;
        }

    if (!(*mid = (UBYTE *)malloc(total)))
        return;

    // fill in number of tracks and bigendian divisions (ticks/qnote)
    midihdr[10] = 0;
    midihdr[11] = (UBYTE)ntrks;   // set number of tracks in header
    midihdr[12] = (mididata->divisions >> 8) & 0x7F;
    midihdr[13] = mididata->divisions & 0xFF;

    // write the midi header
    midiptr = *mid;
    memcpy(midiptr, midihdr, sizeof(midihdr));
    midiptr += sizeof(midihdr);

    // write the tracks
    for (int i = 0; i < MIDI_TRACKS; i++)
        if (mididata->track[i].len)
        {
            memcpy(midiptr, trackhdr, sizeof(trackhdr));                        // header
            midiptr += sizeof(trackhdr);
            TWriteLength(&midiptr, mididata->track[i].len);                     // track length
            memcpy(midiptr, mididata->track[i].data, mididata->track[i].len);   // data
            midiptr += mididata->track[i].len;
        }

    // return length information
    *midlen = (int)(midiptr - *mid);
}
