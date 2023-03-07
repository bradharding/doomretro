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

#pragma once

#include "SDL_mixer.h"
#include "sounds.h"

#define CHUNKSIZE                   1024
#define SAMPLERATE                  44100

#if !defined(__HAIKU__)
#define DEFAULT_DEVICE              NULL
#if defined(__sun)
#define Mix_OpenAudioDevice(freq, format, channels, chunk, dev, chgs)   Mix_OpenAudio(freq, format, channels, chunk)
#endif
#else
// Triggers a segfault if no name is provided even though the default device is empty
#define DEFAULT_DEVICE              ""
#endif

#define LOWER_MUSIC_VOLUME_FACTOR   2.5f

bool I_InitSound(void);
void I_ShutdownSound(void);
bool CacheSFX(sfxinfo_t *sfxinfo);
void I_UpdateSoundParms(const int channel, const int vol, const int sep);
int I_StartSound(sfxinfo_t *sfxinfo, const int channel, const int vol, const int sep, const int pitch);
void I_StopSound(const int channel);
void I_FadeOutSound(const int channel);
bool I_SoundIsPlaying(const int channel);

bool I_InitMusic(void);
void I_ShutdownMusic(void);
void I_SetMusicVolume(int volume);
void I_PauseSong(void);
void I_ResumeSong(void);
void *I_RegisterSong(void *data, int size);
void I_UnregisterSong(void *handle);
void I_PlaySong(void *handle, bool looping);
void I_StopSong(void);
bool I_AnySoundStillPlaying(void);

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(void);

// Shut down sound
void S_Shutdown(void);

void S_StopSounds(void);

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void);

//
// Start sound for thing at <origin_p>
//  using <sfx_id> from sounds.h
//
void S_StartSound(mobj_t *mobj, int sfx_id);
void S_StartSectorSound(degenmobj_t *degenmobj, int sfx_id);
void S_UnlinkSound(mobj_t *origin);

// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h,
//  and set whether looping
void S_ChangeMusic(int music_id, bool looping, bool allowrestart, bool mapstart);

// Stops the music for sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseMusic(void);
void S_ResumeMusic(void);

//
// Updates music and sounds
//
void S_UpdateSounds(void);

void S_SetMusicVolume(int volume);
void S_LowerMusicVolume(void);
void S_RestoreMusicVolume(void);
void S_SetSfxVolume(int volume);

#define MAX_MUS_ENTRIES 64

typedef struct
{
    mobj_t  *mapthing;
    mobj_t  *lastmapthing;
    int     tics;
    int     current_item;
    int     items[MAX_MUS_ENTRIES];
} musinfo_t;

extern musinfo_t    musinfo;

void S_ChangeMusInfoMusic(int lumpnum, int looping);
void S_ParseMusInfo(char *mapid);
void MusInfoThinker(mobj_t *thing);
void T_MAPMusic(void);
