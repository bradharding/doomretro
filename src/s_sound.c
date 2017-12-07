/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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

#include <ctype.h>

#include "c_console.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_setup.h"
#include "w_wad.h"
#include "s_sound.h"
#include "sc_man.h"
#include "z_zone.h"

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST 1200

// Distance to origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// In the source code release: (160*FRACUNIT). Changed back to the
// Vanilla value of 200 (why was this changed?)
#define S_CLOSE_DIST    200

// The range over which sound attenuates
#define S_ATTENUATOR    (S_CLIPPING_DIST - S_CLOSE_DIST)

// Stereo separation
#define S_STEREO_SWING  96

#define NORM_SEP        128

#define TIDNUM(x)       ((int)(x->id & 0xFFFF)) // thing identifier

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t       *sfxinfo;

    // origin of sound
    mobj_t          *origin;

    // handle of the sound being played
    int             handle;

    int             pitch;
} channel_t;

// [crispy] "sound objects" hold the coordinates of removed map objects
typedef struct
{
    thinker_t       dummy;
    fixed_t         x;
    fixed_t         y;
    fixed_t         z;
} sobj_t;

// The set of channels available
static channel_t    *channels;
static sobj_t       *sobjs;

int                 s_channels = s_channels_default;
int                 s_musicvolume = s_musicvolume_default;
dboolean            s_randommusic = s_randommusic_default;
dboolean            s_randompitch = s_randompitch_default;
int                 s_sfxvolume = s_sfxvolume_default;

// Maximum volume of a sound effect.
// Internal default is max out of 0-31.
int                 sfxVolume;

// Maximum volume of music.
int                 musicVolume;

// Internal volume level, ranging from 0-MAX_SFX_VOLUME
static int          snd_SfxVolume;

// Whether songs are mus_paused
static dboolean     mus_paused;

// Music currently being played
musicinfo_t         *mus_playing;

dboolean            nosfx;
dboolean            nomusic;

#if defined(_WIN32)
extern dboolean     serverMidiPlaying;
#endif

// Find and initialize a sound_module_t appropriate for the setting
// in snd_sfxdevice.
static void InitSfxModule(void)
{
    if (I_InitSound())
    {
        C_Output("Sound effects will play at a sample rate of %.1fkHz on %i channels%s.", SAMPLERATE / 1000.0f,
            s_channels, (M_StringCompare(SDL_GetCurrentAudioDriver(), "directsound") ?
            " using the <i><b>DirectSound</b></i> API" : ""));
        return;
    }

    C_Warning("Sound effects couldn't be initialized.");
    nosfx = true;
}

// Initialize music according to snd_musicdevice.
static void InitMusicModule(void)
{
    if (I_InitMusic())
        return;

    C_Warning("Music couldn't be initialized.");
    nomusic = true;
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(void)
{
    if (M_CheckParm("-nosound"))
    {
        C_Output("A <b>-nosound</b> parameter was found on the command-line. Both sound effects and music "
            "have been disabled.");
        nomusic = true;
        nosfx = true;
    }

    if (M_CheckParm("-nomusic"))
    {
        C_Output("A <b>-nomusic</b> parameter was found on the command-line. Music has been disabled.");
        nomusic = true;
    }

    if (M_CheckParm("-nosfx"))
    {
        C_Output("A <b>-nosfx</b> parameter was found on the command-line. Sound effects have been disabled.");
        nosfx = true;
    }

#if defined(WIN32)
    putenv("SDL_AUDIODRIVER=DirectSound");
#endif

    if (!nosfx)
    {
        InitSfxModule();
        S_SetSfxVolume(sfxVolume * MAX_SFX_VOLUME / 31);

        // Allocating the internal channels for mixing
        // (the maximum number of sounds rendered
        // simultaneously) within zone memory.
        channels = Z_Calloc(s_channels, sizeof(channel_t), PU_STATIC, NULL);
        sobjs = Z_Malloc(s_channels * sizeof(sobj_t), PU_STATIC, NULL);

        // Note that sounds have not been cached (yet).
        for (int i = 1; i < NUMSFX; i++)
            S_sfx[i].lumpnum = -1;
    }

    if (!nomusic)
    {
        InitMusicModule();
        S_SetMusicVolume(musicVolume * MAX_MUSIC_VOLUME / 31);

        // no sounds are playing, and they are not mus_paused
        mus_paused = false;
    }
}

void S_Shutdown(void)
{
    I_ShutdownSound();
    I_ShutdownMusic();
}

static void S_StopChannel(int cnum)
{
    channel_t   *c = &channels[cnum];

    if (c->sfxinfo)
    {
        // stop the sound playing
        if (I_SoundIsPlaying(c->handle))
            I_StopSound(c->handle);

        // check to see if other channels are playing the sound
        for (int i = 0; i < s_channels; i++)
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
                break;

        c->sfxinfo = NULL;
        c->origin = NULL;
    }
}

void S_StopSounds(void)
{
    if (nosfx)
        return;

    for (int cnum = 0; cnum < s_channels; cnum++)
        if (channels[cnum].sfxinfo)
            S_StopChannel(cnum);
}

static int S_GetMusicNum(void)
{
    static int mnum;

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

            mnum = nmus[(s_randommusic ? M_RandomIntNoRepeat(1, 9, mnum) : gamemap) - 1];
        }
        else
            mnum = mus_runnin + (s_randommusic ? M_RandomIntNoRepeat(1, 32, mnum) : gamemap) - 1;
    }
    else
    {
        int spmus[] =
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
            mnum = mus_e1m1 + (s_randommusic ? M_RandomIntNoRepeat(1, 21, mnum) :
                (gameepisode - 1) * 9 + gamemap) - 1;
        else
            mnum = spmus[(s_randommusic ? M_RandomIntNoRepeat(1, 28, mnum) : gamemap) - 1];
    }

    return mnum;
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void)
{
    // kill all playing sounds at start of level
    //  (trust me - a good idea)
    S_StopSounds();

    // start new music for the level
    mus_paused = false;

    S_ChangeMusic(S_GetMusicNum(), true, false, true);
}

// [crispy] removed map objects may finish their sounds
// When map objects are removed from the map by P_RemoveMobj(), instead of
// stopping their sounds, their coordinates are transfered to "sound objects"
// so stereo positioning and distance calculations continue to work even after
// the corresponding map object has already disappeared.
// Thanks to jeff-d and kb1 for discussing this feature and the former for the
// original implementation idea: https://www.doomworld.com/vb/post/1585325
void S_UnlinkSound(mobj_t *origin)
{
    if (nosfx)
        return;

    for (int cnum = 0; cnum < s_channels; cnum++)
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            sobj_t  *sobj = &sobjs[cnum];

            sobj->x = origin->x;
            sobj->y = origin->y;
            sobj->z = origin->z;
            channels[cnum].origin = (mobj_t *)sobj;
            break;
        }
}

//
// S_GetChannel :
//   If none available, return -1. Otherwise channel #.
//
static int S_GetChannel(mobj_t *origin, sfxinfo_t *sfxinfo)
{
    // channel number to use
    int         cnum;
    channel_t   *c;

    // Find an open channel
    for (cnum = 0; cnum < s_channels && channels[cnum].sfxinfo; cnum++)
        if (origin && channels[cnum].origin == origin
            && channels[cnum].sfxinfo->singularity == sfxinfo->singularity)
        {
            S_StopChannel(cnum);
            break;
        }

    // None available
    if (cnum == s_channels)
    {
        // Look for lower priority
        for (cnum = 0; cnum < s_channels; cnum++)
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
                break;

        if (cnum == s_channels)
            return -1;                  // FUCK! No lower priority. Sorry, Charlie.
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
// effect to be played. If the sound is not audible, returns false. Otherwise,
// modifies parameters and returns true.
static dboolean S_AdjustSoundParams(mobj_t *listener, fixed_t x, fixed_t y, int *vol, int *sep)
{
    fixed_t dist = 0;
    fixed_t adx;
    fixed_t ady;
    angle_t angle;

    if (nosfx || !listener)
        return false;

    // calculate the distance to sound origin and clip it if necessary
    // killough 11/98: scale coordinates down before calculations start
    // killough 12/98: use exact distance formula instead of approximation
    adx = ABS((listener->x >> FRACBITS) - (x >> FRACBITS));
    ady = ABS((listener->y >> FRACBITS) - (y >> FRACBITS));

    if (ady > adx)
        SWAP(adx, ady);

    if (adx)
        dist = FixedDiv(adx, finesine[(tantoangle[FixedDiv(ady, adx) >> DBITS] + ANG90) >> ANGLETOFINESHIFT]);

    if (dist > S_CLIPPING_DIST)
        return false;

    // angle of source to listener
    angle = R_PointToAngle2(listener->x, listener->y, x, y);

    if (angle <= listener->angle)
        angle += 0xFFFFFFFF;

    angle -= listener->angle;
    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = NORM_SEP - FixedMul(S_STEREO_SWING, finesine[angle]);

    // volume calculation
    *vol = (dist < S_CLOSE_DIST ? snd_SfxVolume : snd_SfxVolume * (S_CLIPPING_DIST - dist) / S_ATTENUATOR);

    return (*vol > 0);
}

static void S_StartSoundAtVolume(mobj_t *origin, int sfx_id, int pitch, int volume)
{
    sfxinfo_t   *sfx = &S_sfx[sfx_id];
    mobj_t      *mo = viewplayer->mo;
    int         sep;
    int         cnum;
    int         handle;

    if (nosfx)
        return;

    // Initialize sound parameters
    if (sfx->link)
    {
        volume += sfx->volume;

        if (volume < 1)
            return;

        if (volume > snd_SfxVolume)
            volume = snd_SfxVolume;
    }

    // Check to see if it is audible, and if not, modify the parms
    if (!origin || origin == mo)
        sep = NORM_SEP;
    else if (!S_AdjustSoundParams(mo, origin->x, origin->y, &volume, &sep))
        return;
    else if (origin->x == mo->x && origin->y == mo->y)
        sep = NORM_SEP;

    // kill old sound
    for (cnum = 0; cnum < s_channels; cnum++)
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
    if (sfx->lumpnum < 0 && (sfx->lumpnum = I_GetSfxLumpNum(sfx)) < 0)
        return;

    // Assigns the handle to one of the channels in the
    //  mix/output buffer.
    // e6y: [Fix] Crash with zero-length sounds.
    if ((handle = I_StartSound(sfx, cnum, volume, sep, pitch)) != -1)
    {
        channels[cnum].handle = handle;
        channels[cnum].pitch = pitch;
    }
}

void S_StartSound(mobj_t *mobj, int sfx_id)
{
    S_StartSoundAtVolume(mobj, sfx_id, (mobj ? mobj->pitch : NORM_PITCH), snd_SfxVolume);
}

void S_StartSectorSound(degenmobj_t *degenmobj, int sfx_id)
{
    S_StartSoundAtVolume((mobj_t *)degenmobj, sfx_id, NORM_PITCH, snd_SfxVolume);
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong();
        mus_paused = true;
    }
}

void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong();
        mus_paused = false;
    }
}

//
// Updates music & sounds
//
void S_UpdateSounds(mobj_t *listener)
{
    if (nosfx)
        return;

    I_UpdateSound();

    for (int cnum = 0; cnum < s_channels; cnum++)
    {
        channel_t   *c = &channels[cnum];
        sfxinfo_t   *sfx = c->sfxinfo;

        if (sfx)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                int     volume = snd_SfxVolume;
                int     sep = NORM_SEP;
                mobj_t  *origin = c->origin;

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
                //  or modify their parms
                if (origin && listener != origin)
                {
                    if (!S_AdjustSoundParams(listener, origin->x, origin->y, &volume, &sep))
                        S_StopChannel(cnum);
                    else
                        I_UpdateSoundParams(c->handle, volume, sep);
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
    I_SetMusicVolume(volume);
}

void S_SetSfxVolume(int volume)
{
    snd_SfxVolume = volume;
}

void S_StartMusic(int music_id)
{
    S_ChangeMusic(music_id, false, false, false);
}

void S_ChangeMusic(int music_id, dboolean looping, dboolean cheating, dboolean mapstart)
{
    musicinfo_t *music = &S_music[music_id];
    void        *handle = NULL;
    int         mapinfomusic;

    // current music which should play
    musinfo.current_item = -1;

    if (nomusic || (mus_playing == music && !cheating))
        return;

    // shutdown old music
    S_StopMusic();

    // get lumpnum if necessary
    if (mapstart && (mapinfomusic = P_GetMapMusic((gameepisode - 1) * 10 + gamemap)) > 0)
        music->lumpnum = mapinfomusic;
    else if (!music->lumpnum)
    {
        char    namebuf[9];

        M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name);
        music->lumpnum = W_CheckNumForName(namebuf);
    }

    if (music->lumpnum != -1)
    {
        // Load & register it
        music->data = W_CacheLumpNum(music->lumpnum);
        handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));
    }

    if (!handle)
#if defined(_WIN32)
        if (!serverMidiPlaying)
#endif
        {
            if (*music->name)
                C_Warning("The <b>D_%s</b> music lump can't be played.", uppercase(music->name));

            return;
        }

    music->handle = handle;

    // Play it
    I_PlaySong(handle, looping);

    mus_playing = music;
}

void S_StopMusic(void)
{
    if (mus_playing)
    {
        if (mus_paused)
            I_ResumeSong();

        I_StopSong();
        I_UnRegisterSong(mus_playing->handle);
        W_UnlockLumpNum(mus_playing->lumpnum);
        mus_playing->data = NULL;
        mus_playing = NULL;
    }
}

void S_ChangeMusInfoMusic(int lumpnum, int looping)
{
    musicinfo_t *music;

    if (nomusic)
        return;

    if (mus_playing && mus_playing->lumpnum == lumpnum)
        return;

    music = &S_music[NUMMUSIC];

    if (music->lumpnum == lumpnum)
        return;

    // shutdown old music
    S_StopMusic();

    // save lumpnum
    music->lumpnum = lumpnum;

    // load & register it
    music->data = W_CacheLumpNum(music->lumpnum);
    music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

    // play it
    I_PlaySong(music->handle, looping);

    mus_playing = music;

    musinfo.current_item = lumpnum;
}

musinfo_t musinfo;

//
// S_ParseMusInfo
// Parses MUSINFO lump.
//
void S_ParseMusInfo(char *mapid)
{
    memset(&musinfo, 0, sizeof(musinfo));
    musinfo.current_item = -1;

    S_music[NUMMUSIC].lumpnum = -1;

    if (W_CheckNumForName("MUSINFO") != -1)
    {
        unsigned int    num;
        int             inMap = false;

        SC_Open("MUSINFO");

        while (SC_GetString())
            if (inMap || SC_Compare(mapid))
            {
                if (!inMap)
                {
                    SC_GetString();
                    inMap = true;
                }

                if (toupper(sc_String[0]) == 'E' || toupper(sc_String[0]) == 'M')
                    break;

                // Check number in range
                if (M_StrToInt(sc_String, &num) && num > 0 && num < MAX_MUS_ENTRIES)
                    if (SC_GetString())
                    {
                        int lumpnum = W_CheckNumForName(sc_String);

                        if (lumpnum >= 0)
                            musinfo.items[num] = lumpnum;
                    }
            }

        SC_Close();
    }
}

void MusInfoThinker(mobj_t *thing)
{
    if (musinfo.mapthing != thing && thing->subsector->sector == viewplayer->mo->subsector->sector)
    {
        musinfo.lastmapthing = musinfo.mapthing;
        musinfo.mapthing = thing;
        musinfo.tics = 30;
    }
}

void T_MAPMusic(void)
{
    if (musinfo.tics < 0 || !musinfo.mapthing)
        return;

    if (musinfo.tics > 0)
        musinfo.tics--;
    else if (musinfo.lastmapthing != musinfo.mapthing)
    {
        int arraypt = TIDNUM(musinfo.mapthing);

        if (arraypt < MAX_MUS_ENTRIES)
        {
            int lumpnum = musinfo.items[arraypt];

            if (lumpnum >= 0 && lumpnum < numlumps)
                S_ChangeMusInfoMusic(lumpnum, true);
        }

        musinfo.tics = -1;
    }
}
