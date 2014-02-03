/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include <stdio.h>
#include <stdlib.h>

#include "i_system.h"

#include "doomstat.h"
#include "doomdef.h"

#include "sounds.h"
#include "s_sound.h"

#include "m_random.h"
#include "m_argv.h"

#include "p_local.h"
#include "w_wad.h"
#include "z_zone.h"

#include "SDL_mixer.h"

// when to clip out sounds
// Does not fit the large outdoor areas.

#define S_CLIPPING_DIST (1200 << FRACBITS)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// In the source code release: (160*FRACUNIT).  Changed back to the
// Vanilla value of 200 (why was this changed?)

#define S_CLOSE_DIST (200 << FRACBITS)

// The range over which sound attenuates

#define S_ATTENUATOR ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)

// Stereo separation

#define S_STEREO_SWING (96 << FRACBITS)

#define NORM_PRIORITY 64
#define NORM_SEP 128


typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t *sfxinfo;

    // origin of sound
    mobj_t *origin;

    // handle of the sound being played
    int handle;
} channel_t;

// Low-level sound and music modules we are using

static sound_module_t *sound_module;
static music_module_t *music_module;

// The set of channels available

static channel_t *channels;

// Maximum volume of a sound effect.
// Internal default is max out of 0-15.

int sfxVolume = 15;

// Maximum volume of music.

int musicVolume = 15;

// Sound sample rate to use for digital output (Hz)

int snd_samplerate = 44100;

// Internal volume level, ranging from 0-127

static int snd_SfxVolume;

// Whether songs are mus_paused

static boolean mus_paused;

// Music currently being played

static musicinfo_t *mus_playing = NULL;

// Number of channels to use

int numChannels = 32;

int snd_musicdevice = SNDDEVICE_GENMIDI;
int snd_sfxdevice = SNDDEVICE_SB;

// Sound modules

extern sound_module_t sound_sdl_module;
extern music_module_t music_sdl_module;

// Compiled-in sound modules:

static sound_module_t *sound_modules[] =
{
    &sound_sdl_module,
    NULL,
};

// Compiled-in music modules:

static music_module_t *music_modules[] =
{
    &music_sdl_module,
    NULL,
};

// Check if a sound device is in the given list of devices

static boolean SndDeviceInList(snddevice_t device, snddevice_t *list, int len)
{
    int i;

    for (i = 0; i < len; ++i)
        if (device == list[i])
            return true;

    return false;
}

// Find and initialize a sound_module_t appropriate for the setting
// in snd_sfxdevice.

static void InitSfxModule(void)
{
    int i;

    sound_module = NULL;

    for (i = 0; sound_modules[i] != NULL; ++i)
    {
        // Is the sfx device in the list of devices supported by
        // this module?

        if (SndDeviceInList((snddevice_t)snd_sfxdevice,
                            sound_modules[i]->sound_devices,
                            sound_modules[i]->num_sound_devices))
        {
            // Initialize the module

            if (sound_modules[i]->Init())
            {
                sound_module = sound_modules[i];
                return;
            }
        }
    }
}

// Initialize music according to snd_musicdevice.

static void InitMusicModule(void)
{
    int i;

    music_module = NULL;

    for (i = 0; music_modules[i] != NULL; ++i)
    {
        // Is the music device in the list of devices supported
        // by this module?

        if (SndDeviceInList((snddevice_t)snd_musicdevice,
                            music_modules[i]->sound_devices,
                            music_modules[i]->num_sound_devices))
        {
            // Initialize the module

            if (music_modules[i]->Init())
            {
                music_module = music_modules[i];
                return;
            }
        }
    }
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

boolean nosound, nosfx, nomusic;

void S_Init(int sfxVolume, int musicVolume)
{
    int i;

    //!
    // @vanilla
    //
    // Disable all sound output.
    //

    nosound = (M_CheckParm("-nosound") > 0);

    //!
    // @vanilla
    //
    // Disable sound effects.
    //

    nosfx = (M_CheckParm("-nosfx") > 0);

    //!
    // @vanilla
    //
    // Disable music.
    //

    nomusic = (M_CheckParm("-nomusic") > 0);

    // Initialize the sound and music subsystems.

    if (!nosound)
    {
        if (!nosfx)
        {
            InitSfxModule();
            S_SetSfxVolume(sfxVolume);

            // Allocating the internal channels for mixing
            // (the maximum numer of sounds rendered
            // simultaneously) within zone memory.
            channels = (channel_t *)Z_Malloc(numChannels * sizeof(channel_t), PU_STATIC, 0);

            // Free all channels for use
            for (i = 0; i < numChannels; i++)
                channels[i].sfxinfo = 0;
            // Note that sounds have not been cached (yet).
            for (i = 1; i < NUMSFX; i++)
                S_sfx[i].lumpnum = -1;
        }

        if (!nomusic)
        {
            InitMusicModule();
            S_SetMusicVolume(musicVolume);

            // no sounds are playing, and they are not mus_paused
            mus_paused = 0;
        }
    }

}

void S_Shutdown(void)
{
    if (sound_module != NULL)
    {
        sound_module->Shutdown();
    }

    if (music_module != NULL)
    {
        music_module->Shutdown();
    }
}

static void S_StopChannel(int cnum)
{
    int i;
    channel_t *c;

    c = &channels[cnum];

    if (c->sfxinfo)
    {
        // stop the sound playing

        if (sound_module != NULL)
        {
            if (sound_module->SoundIsPlaying(c->handle))
            {
                sound_module->StopSound(c->handle);
            }
        }

        // check to see if other channels are playing the sound

        for (i = 0; i < numChannels; i++)
        {
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
            {
                break;
            }
        }

        c->sfxinfo = NULL;
    }
}


void S_StopSounds(void)
{
    int cnum;

    if (nosfx || nosound)
      return;

    for (cnum = 0; cnum < numChannels; cnum++)
      if (channels[cnum].sfxinfo)
        S_StopChannel(cnum);
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

void S_Start(void)
{
    int mnum;

    S_StopSounds();

    if (nomusic || nosound)
      return;

    // start new music for the level
    mus_paused = 0;

    if (gamemode == commercial)
    {
        if (gamemission == pack_nerve)
        {
          int nmus[] =
          {
            mus_messag,
            mus_ddtblu,
            mus_doom,
            mus_shawn,
            mus_in_cit,
            mus_the_da,
            mus_in_cit,
            mus_shawn,
            mus_ddtblu
          };

          mnum = nmus[gamemap - 1];
        }
        else
          mnum = mus_runnin + gamemap - 1;
    }
    else
    {
        int spmus[]=
        {
            // Song - Who? - Where?

            mus_e3m4,           // American     e4m1
            mus_e3m2,           // Romero       e4m2
            mus_e3m3,           // Shawn        e4m3
            mus_e1m5,           // American     e4m4
            mus_e2m7,           // Tim          e4m5
            mus_e2m4,           // Romero       e4m6
            mus_e2m6,           // J.Anderson   e4m7 CHIRON.WAD
            mus_e2m5,           // Shawn        e4m8
            mus_e1m9,           // Tim          e4m9
        };

        if (gameepisode < 4)
        {
            mnum = mus_e1m1 + (gameepisode-1) * 9 + gamemap - 1;
        }
        else
        {
            mnum = spmus[gamemap-1];
        }
    }

    S_ChangeMusic(mnum, true, false);
}

void S_StopSound(mobj_t *origin)
{
    int cnum;

    if (nosound || nosfx)
        return;

    for (cnum = 0; cnum < numChannels; cnum++)
    {
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }
}

//
// S_GetChannel :
//   If none available, return -1.  Otherwise channel #.
//

static int S_GetChannel(mobj_t *origin, sfxinfo_t *sfxinfo)
{
    // channel number to use
    int cnum;
    channel_t *c;

    // Find an open channel
    for (cnum = 0; cnum < numChannels && channels[cnum].sfxinfo; cnum++)
    {
        if (origin && channels[cnum].origin == origin
            && channels[cnum].sfxinfo->singularity == sfxinfo->singularity)
        {
            S_StopChannel(cnum);
            break;
        }
    }

    // None available
    if (cnum == numChannels)
    {
        // Look for lower priority
        for (cnum = 0; cnum < numChannels; cnum++)
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
            {
                break;
            }

        if (cnum == numChannels)
        {
            // FUCK!  No lower priority.  Sorry, Charlie.
            return -1;
        }
        else
        {
            // Otherwise, kick out lower priority.
            S_StopChannel(cnum);
        }
    }

    c = &channels[cnum];

    // channel is decided to be cnum.
    c->sfxinfo = sfxinfo;
    c->origin = origin;

    return cnum;
}

// Changes volume and stereo-separation variables from the norm of a sound
// effect to be played. If the sound is not audible, returns a 0. Otherwise,
// modifies parameters and returns 1.
static int S_AdjustSoundParams(mobj_t *listener, mobj_t *source, int *vol, int *sep)
{
    fixed_t dist;
    fixed_t adx;
    fixed_t ady;
    angle_t angle;

    if (!listener)
        return 0;

    // calculate the distance to sound origin
    //  and clip it if necessary
    //
    // killough 11/98: scale coordinates down before calculations start
    // killough 12/98: use exact distance formula instead of approximation

    adx = ABS((listener->x >> FRACBITS) - (source->x >> FRACBITS));
    ady = ABS((listener->y >> FRACBITS) - (source->y >> FRACBITS));

    if (ady > adx)
        dist = adx, adx = ady, ady = dist;

    dist = (adx ? FixedDiv(adx, finesine[(tantoangle[FixedDiv(ady, adx) >> DBITS]
        + ANG90) >> ANGLETOFINESHIFT]) : 0);

    if (dist > S_CLIPPING_DIST >> FRACBITS)
        return 0;

    // angle of source to listener
    angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

    if (angle <= listener->angle)
        angle += 0xffffffff;
    angle -= listener->angle;
    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = NORM_SEP - FixedMul(S_STEREO_SWING >> FRACBITS, finesine[angle]);

    // volume calculation
    if (dist < (S_CLOSE_DIST >> FRACBITS))
        *vol = snd_SfxVolume;
    else
        *vol = snd_SfxVolume * ((S_CLIPPING_DIST >> FRACBITS) - dist) / S_ATTENUATOR;

    return (*vol > 0);
}

void S_StartSound(void *origin_p, int sfx_id)
{
    sfxinfo_t *sfx;
    mobj_t *origin;
    int sep;
    int priority;
    int cnum;
    int volume;

    if (nosound || nosfx)
        return;

    origin = (mobj_t *) origin_p;
    volume = snd_SfxVolume;

    sfx = &S_sfx[sfx_id];

    // Initialize sound parameters
    if (sfx->link)
    {
        priority = sfx->priority;
        volume += sfx->volume;

        if (volume < 1)
            return;

        if (volume > snd_SfxVolume)
            volume = snd_SfxVolume;
    }
    else
        priority = NORM_PRIORITY;

    // Check to see if it is audible,
    //  and if not, modify the params
    if (!origin || origin == players[displayplayer].mo)
        sep = NORM_SEP;
    else if (!S_AdjustSoundParams(players[displayplayer].mo, origin, &volume, &sep))
        return;
    else if (origin->x == players[displayplayer].mo->x
             && origin->y == players[displayplayer].mo->y)
             sep = NORM_SEP;

    // kill old sound
    for (cnum = 0; cnum < numChannels; cnum++)
        if (channels[cnum].sfxinfo
            && channels[cnum].sfxinfo->singularity == sfx->singularity
            && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }

    // try to find a channel
    cnum = S_GetChannel(origin, sfx);

    if (cnum < 0)
        return;

    if (sound_module != NULL)
    {
        // Get lumpnum if necessary

        if (sfx->lumpnum < 0)
            sfx->lumpnum = sound_module->GetSfxLumpNum(sfx);

        // Assigns the handle to one of the channels in the
        //  mix/output buffer.

        channels[cnum].handle = sound_module->StartSound(sfx_id, cnum, volume, sep);
    }
}

//
// Stop and resume music, during game PAUSE.
//

void S_PauseSound(void)
{
    if (nosound || nomusic)
        return;

    if (mus_playing && !mus_paused)
    {
        if (music_module != NULL)
        {
            music_module->PauseMusic();
        }
        mus_paused = true;
    }
}

void S_ResumeSound(void)
{
    if (nosound || nomusic)
        return;

    if (mus_playing && mus_paused)
    {
        if (music_module != NULL)
        {
            music_module->ResumeMusic();
        }
        mus_paused = false;
    }
}

//
// Updates music & sounds
//

void S_UpdateSounds(mobj_t *listener)
{
    int cnum;

    if (nosound || nosfx)
        return;

    for (cnum = 0; cnum < numChannels; cnum++)
    {
        channel_t *c = &channels[cnum];
        sfxinfo_t *sfx = c->sfxinfo;

        if (c->sfxinfo)
        {
            if (sound_module != NULL && sound_module->SoundIsPlaying(c->handle))
            {
                // initialize parameters
                int volume = snd_SfxVolume;
                int sep = NORM_SEP;

                if (sfx->link)
                {
                    volume += sfx->volume;
                    if (volume < 1)
                    {
                        S_StopChannel(cnum);
                        continue;
                    }
                    else if (volume > snd_SfxVolume)
                        volume = snd_SfxVolume;
                }

                // check non-local sounds for distance clipping
                //  or modify their params
                if (c->origin && listener != c->origin)
                {
                    if (!S_AdjustSoundParams(listener, c->origin, &volume, &sep))
                        S_StopChannel(cnum);
                    else
                        sound_module->UpdateSoundParams(c->handle, volume, sep);
                }
            }
            else
                // if channel is allocated but sound has stopped, free it
                S_StopChannel(cnum);
        }
    }
}

void S_SetMusicVolume(int volume)
{
    if (nosound || nomusic)
        return;

    if (music_module != NULL)
    {
        music_module->SetMusicVolume(volume);
    }
}

void S_SetSfxVolume(int volume)
{
    if (nosound || nosfx)
        return;

    snd_SfxVolume = volume;
}

//
// Starts some music with the music id found in sounds.h.
//

void S_StartMusic(int m_id)
{
    if (nosound || nomusic)
        return;

    S_ChangeMusic(m_id, false, false);
}

void S_ChangeMusic(int musicnum, int looping, int cheating)
{
    musicinfo_t *music = NULL;
    char namebuf[9];
    void *handle;

    if (nosound || nomusic)
        return;

    // The Doom IWAD file has two versions of the intro music: d_intro
    // and d_introa.  The latter is used for OPL playback.

    if (musicnum == mus_intro && (snd_musicdevice == SNDDEVICE_ADLIB
                                  || snd_musicdevice == SNDDEVICE_SB))
    {
        musicnum = mus_introa;
    }

    music = &S_music[musicnum];

    if (mus_playing == music && !cheating)
    {
        return;
    }

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
        sprintf(namebuf, "d_%s", (music->name));
        music->lumpnum = W_GetNumForName(namebuf);
    }

    if (music_module != NULL)
    {
        // Load & register it

        music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);
        handle = music_module->RegisterSong(music->data,
                                            W_LumpLength(music->lumpnum));

        music->handle = handle;

        // Play it

        music_module->PlaySong(handle, looping);
    }

    mus_playing = music;
}

boolean S_MusicPlaying(void)
{
    if (music_module != NULL)
    {
        return music_module->MusicIsPlaying();
    }
    else
    {
        return false;
    }
}

void S_StopMusic(void)
{
    if (nosound || nomusic)
        return;

    if (mus_playing)
    {
        if (music_module != NULL)
        {
            if (mus_paused)
            {
                music_module->ResumeMusic();
            }

            music_module->StopSong();
            music_module->UnRegisterSong(mus_playing->handle);
            W_ReleaseLumpNum(mus_playing->lumpnum);

            mus_playing->data = NULL;
        }

        mus_playing = NULL;
    }
}