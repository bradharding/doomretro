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

#if !defined(__MMUS2MID_H__)
#define __MMUS2MID_H__

#include "doomtype.h"

// some names for integers of various sizes, all unsigned
typedef uint8_t     UBYTE;  // a one-byte int
typedef uint16_t    UWORD;  // a two-byte int

// proff: changed from unsigned int to unsigned long to avoid warning
typedef uint32_t    ULONG; // a four-byte int (assumes int 4 bytes)

#define MIDI_TRACKS 32

typedef struct                          // a midi file
{
    int                 divisions;      // number of ticks per quarter note

    struct
    {
        unsigned char   *data;          // MIDI message stream
        size_t          len;            // length of the track data
    } track[MIDI_TRACKS];
} MIDI;

dboolean mmuscheckformat(UBYTE *mus, int size);
dboolean mmus2mid(UBYTE *mus, size_t size, MIDI *mididata);
void MIDIToMidi(MIDI *mididata, UBYTE **mid, int *midlen);

#endif
