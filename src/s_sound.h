/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#if !defined(__S_SOUND__)
#define __S_SOUND__

#include "p_mobj.h"
#include "sounds.h"

extern int snd_samplerate;

boolean I_SDL_InitSound(void);
void I_SDL_ShutdownSound(void);
int I_SDL_GetSfxLumpNum(sfxinfo_t *sfx);
void I_SDL_UpdateSoundParams(int handle, int vol, int sep);
int I_SDL_StartSound(int id, int channel, int vol, int sep);
void I_SDL_StopSound(int handle);
boolean I_SDL_SoundIsPlaying(int handle);

boolean I_SDL_InitMusic(void);
void I_SDL_ShutdownMusic(void);
void I_SDL_SetMusicVolume(int volume);
void I_SDL_PauseSong(void);
void I_SDL_ResumeSong(void);
void *I_SDL_RegisterSong(void *data, int len);
void I_SDL_UnRegisterSong(void *handle);
void I_SDL_PlaySong(void *handle, int looping);
void I_SDL_StopSong(void);
boolean I_SDL_MusicIsPlaying(void);

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxVolume, int musicVolume);

// Shut down sound
void S_Shutdown(void);

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void);

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void S_StartSound(void *origin, int sound_id);

// Stop sound for thing at <origin>
void S_StopSound(mobj_t *origin);
void S_StopSounds(void);

// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h,
//  and set whether looping
void S_ChangeMusic(int music_id, int looping, int cheating);

// query if music is playing
boolean S_MusicPlaying(void);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);

//
// Updates music & sounds
//
void S_UpdateSounds(mobj_t *listener);

void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);

void I_InitTimidityConfig(void);
void CheckTimidityConfig(void);

boolean I_AnySoundStillPlaying(void);

#endif
