/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__S_SOUND__)
#define __S_SOUND__

#include "p_mobj.h"
#include "r_defs.h"
#include "sounds.h"

extern int      snd_samplerate;
extern dboolean s_randompitch;

dboolean I_SDL_InitSound(void);
void I_SDL_ShutdownSound(void);
int I_SDL_GetSfxLumpNum(sfxinfo_t *sfx);
void I_SDL_UpdateSoundParams(int handle, int vol, int sep);
int I_SDL_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep, int pitch);
void I_SDL_StopSound(int handle);
dboolean I_SDL_SoundIsPlaying(int handle);
void I_SDL_UpdateSound(void);

dboolean I_SDL_InitMusic(void);
void I_SDL_ShutdownMusic(void);
void I_SDL_SetMusicVolume(int volume);
void I_SDL_PauseSong(void);
void I_SDL_ResumeSong(void);
void *I_SDL_RegisterSong(void *data, int len);
void I_SDL_UnRegisterSong(void *handle);
void I_SDL_PlaySong(void *handle, int looping);
void I_SDL_StopSong(void);
dboolean I_SDL_MusicIsPlaying(void);

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxvol, int musicvol);

// Shut down sound
void S_Shutdown(void);

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

void S_StopSounds(void);

// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h,
//  and set whether looping
void S_ChangeMusic(int music_id, dboolean looping, dboolean cheating, dboolean mapstart);

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

dboolean I_AnySoundStillPlaying(void);

#endif
