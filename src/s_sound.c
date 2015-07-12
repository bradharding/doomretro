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

#include "c_console.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "w_wad.h"
#include "s_sound.h"
#include "z_zone.h"

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1200 << FRACBITS)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// In the source code release: (160*FRACUNIT).  Changed back to the
// Vanilla value of 200 (why was this changed?)
#define S_CLOSE_DIST    (200 << FRACBITS)

// The range over which sound attenuates
#define S_ATTENUATOR    ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)

// Stereo separation
#define S_STEREO_SWING  (96 << FRACBITS)

#define NORM_PRIORITY   64
#define NORM_SEP        128

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t           *sfxinfo;

    // origin of sound
    mobj_t              *origin;

    // handle of the sound being played
    int                 handle;

    int                 pitch;
}
channel_t;

// The set of channels available
static channel_t        *channels;

// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int                     sfxVolume = SFXVOLUME_DEFAULT;

// Maximum volume of music.
int                     musicVolume = MUSICVOLUME_DEFAULT;

// Sound sample rate to use for digital output (Hz)
int                     snd_samplerate = 44100;

// Internal volume level, ranging from 0-127
static int              snd_SfxVolume;

// Whether songs are mus_paused
static dboolean         mus_paused;

// Music currently being played
musicinfo_t             *mus_playing = NULL;

// Number of channels to use
int                     numChannels = 32;

dboolean                randompitch = RANDOMPITCH_DEFAULT;

// Find and initialize a sound_module_t appropriate for the setting
// in snd_sfxdevice.
static void InitSfxModule(void)
{
    if (I_SDL_InitSound())
    {
        C_Output("SFX playing at a sample rate of %.1fkHz on %i channels.",
            snd_samplerate / 1000.0f, numChannels);
        return;
    }

    C_Output("The initialization of SFX failed.");
}

// Initialize music according to snd_musicdevice.
static void InitMusicModule(void)
{
    if (I_SDL_InitMusic())
    {
        C_Output("Using General MIDI for music.");

        CheckTimidityConfig();
        return;
    }

    C_Warning("The initialization of music failed.");
}

dboolean nosound = false;
dboolean nosfx = false;
dboolean nomusic = false;

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxvol, int musicvol)
{
    if (M_CheckParm("-nosound") > 0)
    {
        C_Output("Found -NOSOUND parameter on command-line. Music and SFX have been disabled.");
        nosound = true;
        nomusic = true;
        nosfx = true;
    }
    if (M_CheckParm("-nomusic") > 0)
    {
        C_Output("Found -NOMUSIC parameter on command-line. Music has been disabled.");
        nomusic = true;
    }
    if (M_CheckParm("-nosfx") > 0)
    {
        C_Output("Found -NOSFX parameter on command-line. SFX have been disabled.");
        nosfx = true;
    }

    // Initialize the sound and music subsystems.
    if (!nosound)
    {
        // This is kind of a hack. If native MIDI is enabled, set up
        // the TIMIDITY_CFG environment variable here before SDL_mixer
        // is opened.
        if (!nomusic)
            I_InitTimidityConfig();

        if (!nosfx)
        {
            int i;

            InitSfxModule();
            S_SetSfxVolume(sfxvol);

            // Allocating the internal channels for mixing
            // (the maximum numer of sounds rendered
            // simultaneously) within zone memory.
            channels = Z_Malloc(numChannels * sizeof(channel_t), PU_STATIC, 0);

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
            S_SetMusicVolume(musicvol);

            // no sounds are playing, and they are not mus_paused
            mus_paused = false;
        }
    }
}

void S_Shutdown(void)
{
    I_SDL_ShutdownSound();
    I_SDL_ShutdownMusic();
}

static void S_StopChannel(int cnum)
{
    channel_t   *c = &channels[cnum];

    if (c->sfxinfo)
    {
        int     i;

        // stop the sound playing
        if (I_SDL_SoundIsPlaying(c->handle))
            I_SDL_StopSound(c->handle);

        // check to see if other channels are playing the sound
        for (i = 0; i < numChannels; ++i)
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
                break;

        c->sfxinfo = NULL;
        c->origin = NULL;
    }
}

void S_StopSounds(void)
{
    int cnum;

    if (nosfx)
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

    // kill all playing sounds at start of level
    //  (trust me - a good idea)
    S_StopSounds();

    // start new music for the level
    mus_paused = false;

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
            mus_e1m9            // Tim          e4m9
        };

        if (gameepisode < 4)
            mnum = mus_e1m1 + (gameepisode - 1) * 9 + gamemap - 1;
        else
            mnum = spmus[gamemap - 1];
    }

    S_ChangeMusic(mnum, true, false);
}

//
// S_GetChannel :
//   If none available, return -1.  Otherwise channel #.
//
static int S_GetChannel(mobj_t *origin, sfxinfo_t *sfxinfo)
{
    // channel number to use
    int         cnum;
    channel_t   *c;

    // Find an open channel
    for (cnum = 0; cnum < numChannels && channels[cnum].sfxinfo; ++cnum)
        if (origin && channels[cnum].origin == origin
            && channels[cnum].sfxinfo->singularity == sfxinfo->singularity)
        {
            S_StopChannel(cnum);
            break;
        }

    // None available
    if (cnum == numChannels)
    {
        // Look for lower priority
        for (cnum = 0; cnum < numChannels; ++cnum)
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
                break;

        if (cnum == numChannels)
            return -1;                  // FUCK!  No lower priority.  Sorry, Charlie.
        else
            S_StopChannel(cnum);        // Otherwise, kick out lower priority.
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
    fixed_t     dist;
    fixed_t     adx;
    fixed_t     ady;
    angle_t     angle;

    if (nosfx || !listener)
        return 0;

    // calculate the distance to sound origin
    //  and clip it if necessary
    // killough 11/98: scale coordinates down before calculations start
    // killough 12/98: use exact distance formula instead of approximation
    adx = ABS((listener->x >> FRACBITS) - (source->x >> FRACBITS));
    ady = ABS((listener->y >> FRACBITS) - (source->y >> FRACBITS));

    if (ady > adx)
    {
        dist = adx;
        adx = ady;
        ady = dist;
    }

    dist = (adx ? FixedDiv(adx, finesine[(tantoangle[FixedDiv(ady, adx) >> DBITS]
        + ANG90) >> ANGLETOFINESHIFT]) : 0);

    if (dist > (S_CLIPPING_DIST >> FRACBITS))
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
    sfxinfo_t   *sfx = &S_sfx[sfx_id];
    mobj_t      *origin = (mobj_t *)origin_p;
    mobj_t      *player = players[0].mo;
    int         sep;
    int         pitch = (origin ? origin->pitch : NORM_PITCH);
    int         cnum;
    int         volume = snd_SfxVolume;
    int         handle;

    // Initialize sound parameters
    if (sfx->link)
    {
        volume += sfx->volume;

        if (volume < 1)
            return;

        if (volume > snd_SfxVolume)
            volume = snd_SfxVolume;
    }

    // Check to see if it is audible,
    //  and if not, modify the params
    if (!origin || origin == player)
        sep = NORM_SEP;
    else if (!S_AdjustSoundParams(player, origin, &volume, &sep))
        return;
    else if (origin->x == player->x && origin->y == player->y)
        sep = NORM_SEP;

    // kill old sound
    for (cnum = 0; cnum < numChannels; cnum++)
        if (channels[cnum].sfxinfo && channels[cnum].sfxinfo->singularity == sfx->singularity
            && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }

    // try to find a channel
    cnum = S_GetChannel(origin, sfx);

    if (cnum < 0)
        return;

    // Get lumpnum if necessary
    // killough 2/28/98: make missing sounds non-fatal
    if (sfx->lumpnum < 0 && (sfx->lumpnum = I_SDL_GetSfxLumpNum(sfx)) < 0)
        return;

    // Assigns the handle to one of the channels in the
    //  mix/output buffer.
    // e6y: [Fix] Crash with zero-length sounds.
    if ((handle = I_SDL_StartSound(sfx, cnum, volume, sep, pitch)) != -1)
    {
        channels[cnum].handle = handle;
        channels[cnum].pitch = pitch;
    }
}

void S_StartMapSound(void *origin_p, int sfx_id)
{
    mobj_t      *origin = (mobj_t *)origin_p;

    origin->pitch = NORM_PITCH;
    S_StartSound(origin, sfx_id);
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
    if (mus_playing && !mus_paused)
    {
        I_SDL_PauseSong();
        mus_paused = true;
    }
}

void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_SDL_ResumeSong();
        mus_paused = false;
    }
}

//
// Updates music & sounds
//
void S_UpdateSounds(mobj_t *listener)
{
    int cnum;

    I_SDL_UpdateSound();

    for (cnum = 0; cnum < numChannels; ++cnum)
    {
        channel_t       *c = &channels[cnum];
        sfxinfo_t       *sfx = c->sfxinfo;

        if (sfx)
        {
            if (I_SDL_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                int     volume = snd_SfxVolume;
                int     sep = NORM_SEP;

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
                        I_SDL_UpdateSoundParams(c->handle, volume, sep);
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
    I_SDL_SetMusicVolume(volume);
}

void S_SetSfxVolume(int volume)
{
    snd_SfxVolume = volume;
}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
    S_ChangeMusic(m_id, false, false);
}

void S_ChangeMusic(int musicnum, int looping, int cheating)
{
    musicinfo_t *music = &S_music[musicnum];
    void        *handle;

    if (mus_playing == music && !cheating)
        return;

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
        char    namebuf[9];

        M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name);
        music->lumpnum = W_GetNumForName(namebuf);
    }

    // Load & register it
    music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);
    handle = I_SDL_RegisterSong(music->data, W_LumpLength(music->lumpnum));

    music->handle = handle;

    // Play it
    I_SDL_PlaySong(handle, looping);

    mus_playing = music;
}

void S_StopMusic(void)
{
    if (mus_playing)
    {
        if (mus_paused)
            I_SDL_ResumeSong();

        I_SDL_StopSong();
        I_SDL_UnRegisterSong(mus_playing->handle);
        W_ReleaseLumpNum(mus_playing->lumpnum);
        mus_playing->data = NULL;
        mus_playing = NULL;
    }
}
