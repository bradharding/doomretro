/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <ctype.h>

#include "c_console.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_setup.h"
#include "s_sound.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST 1200

// Distance to origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// In the source code release: (160 * FRACUNIT). Changed back to the
// Vanilla value of 200 (why was this changed?)
#define S_CLOSE_DIST    200

// The range over which sound attenuates
#define S_ATTENUATOR    (S_CLIPPING_DIST - S_CLOSE_DIST)

// Stereo separation
#define S_STEREO_SWING  96

#define NORM_SEP        127

#define TIDNUM(x)       (int)(x->musicid & 0xFFFF)  // thing identifier

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t       *sfxinfo;

    // origin of sound
    mobj_t          *origin;

    // handle of the sound being played
    int             handle;
} channel_t;

// [crispy] "sound objects" hold the coordinates of removed map objects
typedef struct
{
    thinker_t       dummy;
    fixed_t         x, y, z;
} sobj_t;

// The set of channels available
static channel_t    *channels;
static sobj_t       *sobjs;

// Maximum volume of a sound effect.
// Internal default is max out of 0-31.
int                 sfxvolume;

// Maximum volume of music.
int                 musicvolume;

// Internal volume level, ranging from 0 to 127
static int          snd_sfxvolume;

// Whether songs are mus_paused
static bool         mus_paused;

// Music currently being played
musicinfo_t         *mus_playing;

bool                nosfx;
bool                nomusic;

musinfo_t           musinfo;

const int spmus[] =
{
    // Song - Who? - Where?
    mus_e3m4,   // American     E4M1
    mus_e3m2,   // Romero       E4M2
    mus_e3m3,   // Shawn        E4M3
    mus_e1m5,   // American     E4M4
    mus_e2m7,   // Tim          E4M5
    mus_e2m4,   // Romero       E4M6
    mus_e2m6,   // J.Anderson   E4M7 CHIRON.WAD
    mus_e2m5,   // Shawn        E4M8
    mus_e1m9    // Tim          E4M9
};

const int nmus[] =
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

// Initialize sound effects.
static void InitSfxModule(void)
{
    if (I_InitSound())
    {
        const char  *audiodriver = SDL_GetCurrentAudioDriver();

        C_Output("Sound effects are playing at %i%% volume and a sample rate of %.1fkHz over %i channels%s",
            s_sfxvolume, SAMPLERATE / 1000.0f, s_channels,
            (M_StringCompare(audiodriver, "wasapi") ? " using the " ITALICS("WASAPI.") :
            (M_StringCompare(audiodriver, "directsound") ? " using the " ITALICS("DirectSound") " API." : "")));
    }
    else
    {
        C_Warning(1, "Sound effects couldn't be %s.", (english == english_american ? "initialized" : "initialised"));
        nosfx = true;
    }
}

// Initialize music.
static void InitMusicModule(void)
{
    if (!I_InitMusic())
    {
        C_Warning(1, "Music couldn't be %s.", (english == english_american ? "initialized" : "initialised"));
        nomusic = true;
    }
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume, allocates channel buffer, sets s_sfx lookup.
//
void S_Init(void)
{
    if (M_CheckParm("-nosound"))
    {
        C_Warning(1, "A " BOLD("-nosound") " parameter was found on the command-line. No sound effects or music will be played.");
        nomusic = true;
        nosfx = true;
    }
    else
    {
        if (M_CheckParm("-nomusic"))
        {
            C_Warning(1, "A " BOLD("-nomusic") " parameter was found on the command-line. No music will be played.");
            nomusic = true;
        }

        if (M_CheckParm("-nosfx"))
        {
            C_Warning(1, "A " BOLD("-nosfx") " parameter was found on the command-line. No sound effects will be played.");
            nosfx = true;
        }
    }

    if (!nosfx)
    {
#if defined(_WIN32)
        char    *audiodriver = getenv("SDL_AUDIODRIVER");

        if (audiodriver)
        {
            C_Warning(1, "The " BOLD("SDL_AUDIODRIVER") " environment variable has been set to " BOLD("\"%s\"") ".", audiodriver);
            free(audiodriver);
        }
#endif

        InitSfxModule();
        sfxvolume = (s_sfxvolume * 31 + 50) / 100;
        S_SetSfxVolume(sfxvolume * (MIX_MAX_VOLUME - 1) / 31);

        // Allocating the internal channels for mixing (the maximum number of sounds played simultaneously) within zone memory.
        channels = Z_Calloc(s_channels_max, sizeof(channel_t), PU_STATIC, NULL);
        sobjs = Z_Calloc(s_channels_max, sizeof(sobj_t), PU_STATIC, NULL);

        // [BH] precache all SFX
        for (int i = 1; i < numsfx; i++)
        {
            sfxinfo_t   *sfx = &s_sfx[i];

            if (*s_sfx[i].name1)
            {
                char    namebuf[9];

                M_snprintf(namebuf, sizeof(namebuf), "ds%s", sfx->name1);

                if (extras && M_StringCompare(namebuf, "dssecret"))
                {
                    sfx->lumpnum = W_GetLastNumForName(namebuf);
                    CacheSFX(sfx);
                }
                else if ((sfx->lumpnum = W_CheckNumForName(namebuf)) >= 0 && !CacheSFX(sfx))
                {
                    char    *temp = uppercase(namebuf);

                    sfx->lumpnum = -1;
                    C_Warning(1, "The " BOLD("%s") " sound effect lump won't be played.", temp);
                    free(temp);
                }
            }
        }
    }

    if (!nomusic)
    {
        InitMusicModule();
        musicvolume = (s_musicvolume * 31 + 50) / 100;
        S_RestoreMusicVolume();

        // no sounds are playing, and they are not mus_paused
        mus_paused = false;

        musinfo.mapthing = NULL;

        for (int i = mus_e4m1, j = 0; i <= mus_e4m9; i++, j++)
        {
            musicinfo_t *music = &s_music[i];
            char        namebuf[9];

            M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name1);

            if (W_CheckNumForName(namebuf) == -1)
            {
                M_StringCopy(music->name1, s_music[spmus[j]].name1, sizeof(music->name1));
                M_StringCopy(music->title1, s_music[spmus[j]].title1, sizeof(music->title1));
            }
        }
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

void S_StopSoundEffect(const sfxnum_t sfxnum)
{
    sfxinfo_t   *sfx;

    if (nosfx)
        return;

    sfx = &s_sfx[sfxnum];

    for (int cnum = 0; cnum < s_channels; cnum++)
        if (channels[cnum].sfxinfo == sfx)
        {
            S_StopChannel(cnum);
            break;
        }
}

void S_StopSound(const mobj_t *origin)
{
    if (nosfx)
        return;

    for (int cnum = 0; cnum < s_channels; cnum++)
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
}

void S_StopSounds(void)
{
    if (nosfx)
        return;

    for (int cnum = 0; cnum < s_channels; cnum++)
        S_StopChannel(cnum);
}

static int S_GetMapNum(void)
{
    if (!s_randommusic)
        return ((gameepisode - 1) * 9 + gamemap);

    if (gamemode == commercial)
        return M_RandomIntNoRepeat(1, (gamemission == pack_nerve ? 9 : 32), gamemap);
    else if ((gameepisode == 5 && sigil) || (gameepisode == 6 && sigil2))
            return M_RandomIntNoRepeat(1, 9, gamemap);
    else
        return M_RandomIntNoRepeat(1, 4 * 9, (gameepisode - 1) * 9 + gamemap);
}

static int S_GetMusicNum(void)
{
    const int   map = S_GetMapNum() - 1;

    return (gamemode == commercial ? (gamemission == pack_nerve ? nmus[map] : mus_runnin + map) :
        (gameepisode == 5 && sigil ? mus_e5m1 : (gameepisode == 6 && sigil2 ? mus_e6m1 : mus_e1m1)) + map);
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void)
{
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
// original implementation idea: <https://www.doomworld.com/forum/post/1585325>
void S_UnlinkSound(const mobj_t *origin)
{
    if (!origin->madesound || nosfx)
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
// S_GetChannel
// If none available, return -1. Otherwise channel #.
//
static int S_GetChannel(mobj_t *origin, sfxinfo_t *sfxinfo)
{
    // channel number to use
    int         cnum = 0;
    channel_t   *c;

    // Find an open channel
    if (origin)
        for (; cnum < s_channels && channels[cnum].sfxinfo; cnum++)
            if (channels[cnum].origin == origin
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
            return -1;              // FUCK! No lower priority. Sorry, Charlie.
        else
            S_StopChannel(cnum);    // Otherwise, kick out lower priority.
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
static bool S_AdjustSoundParms(const mobj_t *origin, int *vol, int *sep)
{
    fixed_t         dist = 0;
    const mobj_t    *listener = viewplayer->mo;
    const fixed_t   x = origin->x;
    const fixed_t   y = origin->y;

    // calculate the distance to sound origin and clip it if necessary
    // killough 11/98: scale coordinates down before calculations start
    // killough 12/98: use exact distance formula instead of approximation
    fixed_t adx = ABS((listener->x >> FRACBITS) - (x >> FRACBITS));
    fixed_t ady = ABS((listener->y >> FRACBITS) - (y >> FRACBITS));

    if (ady > adx)
        SWAP(adx, ady);

    if (adx)
        dist = FixedDiv(adx, finesine[(tantoangle[FixedDiv(ady, adx) >> DBITS] + ANG90) >> ANGLETOFINESHIFT]);

    // killough 11/98: handle zero-distance as special case
    if (!dist)
    {
        *sep = NORM_SEP;
        *vol = snd_sfxvolume;

        return (*vol > 0);
    }

    if (dist > S_CLIPPING_DIST)
        return false;

    // stereo separation
    if (s_stereo)
    {
        // angle of source to player
        angle_t angle = R_PointToAngle(x, y);

        if (angle <= viewangle)
            angle += 0xFFFFFFFF;

        *sep = NORM_SEP - FixedMul(S_STEREO_SWING, finesine[(angle - viewangle) >> ANGLETOFINESHIFT]);
    }

    // volume calculation
    *vol = (dist < S_CLOSE_DIST ? snd_sfxvolume : snd_sfxvolume * (S_CLIPPING_DIST - dist) / S_ATTENUATOR);

    return (*vol > 0);
}

static void S_StartSoundAtVolume(mobj_t *origin, sfxnum_t sfxnum, int pitch)
{
    sfxinfo_t   *sfx = &s_sfx[sfxnum];
    int         sep = NORM_SEP;
    int         cnum;
    int         handle;
    int         volume = snd_sfxvolume;

    if (sfx->lumpnum == -1 || nosfx)
        return;

    // Check to see if it is audible, and if not, modify the parms
    if (origin && origin != viewplayer->mo && !S_AdjustSoundParms(origin, &volume, &sep))
        return;

    // try to find a channel
    if ((cnum = S_GetChannel(origin, sfx)) < 0)
        return;

    // Assigns the handle to one of the channels in the mix/output buffer.
    // e6y: [Fix] Crash with zero-length sounds.
    if ((handle = I_StartSound(sfx, cnum, volume, sep, pitch)) != -1)
        channels[cnum].handle = handle;
}

void S_StartSound(mobj_t *mobj, const sfxnum_t sfxnum)
{
    if (mobj)
    {
        mobj->madesound = true;
        S_StartSoundAtVolume(mobj, sfxnum, mobj->pitch);
    }
    else
        S_StartSoundAtVolume(NULL, sfxnum, NORM_PITCH);
}

void S_StartSectorSound(degenmobj_t *degenmobj, const sfxnum_t sfxnum)
{
    S_StartSoundAtVolume((mobj_t *)degenmobj, sfxnum, NORM_PITCH);
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseMusic(void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong();
        mus_paused = true;
    }
}

void S_ResumeMusic(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong();
        mus_paused = false;
    }
}

//
// Updates sounds
//
void S_UpdateSounds(void)
{
    if (nosfx)
        return;

    for (int cnum = 0; cnum < s_channels; cnum++)
    {
        channel_t       *c = &channels[cnum];
        const sfxinfo_t *sfx = c->sfxinfo;

        if (sfx)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                const mobj_t *origin = c->origin;

                // check non-local sounds for distance clipping or modify their parms
                if (origin && origin != viewplayer->mo)
                {
                    int sep = NORM_SEP;
                    int volume = snd_sfxvolume;

                    if (S_AdjustSoundParms(origin, &volume, &sep))
                        I_UpdateSoundParms(c->handle, volume, sep);
                    else
                        S_StopChannel(cnum);
                }
            }
            else
                // if channel is allocated but sound has stopped, free it
                S_StopChannel(cnum);
        }
    }
}

void S_LowerMusicVolume(void)
{
    I_SetMusicVolume((int)(musicvolume * (MIX_MAX_VOLUME - 1) / 31.0f
        / (s_lowermenumusic ? LOWER_MUSIC_VOLUME_FACTOR : 1.0f)));
}

void S_RestoreMusicVolume(void)
{
    I_SetMusicVolume(musicvolume * (MIX_MAX_VOLUME - 1) / 31);
}

void S_SetSfxVolume(const int volume)
{
    snd_sfxvolume = volume;
}

void S_StartMusic(const musicnum_t musicnum)
{
    S_ChangeMusic(musicnum, false, false, false);
}

void S_ChangeMusic(const musicnum_t musicnum, const bool looping,
    const bool allowrestart, const bool mapstart)
{
    musicinfo_t *music = &s_music[musicnum];
    char        namebuf[9];
    void        *handle;
    int         mapinfomusic;

    // current music which should play
    musinfo.currentitem = -1;
    s_music[mus_musinfo].lumpnum = -1;

    if (nomusic || (mus_playing == music && !allowrestart))
        return;

    // shutdown old music
    S_StopMusic();

    if (M_StringStartsWith(music->name1, "d_"))
        M_StringCopy(namebuf, music->name1, sizeof(namebuf));
    else
    {
        if (*music->IDKFA && !legacyofrust)
        {
            M_StringCopy(namebuf, music->IDKFA, sizeof(namebuf));

            if (W_CheckNumForName(namebuf) == -1)
                M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name1);
        }
        else
            M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name1);
    }

    // get lumpnum if necessary
    if (autosigil)
    {
        if (musicnum == mus_intro)
            music->lumpnum = W_GetLastNumForName(namebuf);
        else if (musicnum == mus_inter && sigil && sigil2)
        {
            if (gameepisode == 5)
            {
                M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name1);
                music->lumpnum = W_GetXNumForName(namebuf, (buckethead ? 4 : 2));
            }
            else if (gameepisode == 6)
            {
                M_snprintf(namebuf, sizeof(namebuf), "d_%s", music->name1);
                music->lumpnum = W_GetXNumForName(namebuf,
                    (registeredsigil ? 4 : (thorr && buckethead ? 5 : 3)));
            }
            else
                music->lumpnum = W_GetLastNumForName(namebuf);
        }
        else
            music->lumpnum = W_CheckNumForName(namebuf);
    }
    else if (mapstart && (mapinfomusic = P_GetMapMusic(gameepisode, S_GetMapNum())) > 0)
    {
        music->lumpnum = mapinfomusic;
        M_StringCopy(music->name1, lumpinfo[mapinfomusic]->name, sizeof(music->name1));
    }
    else if (!music->lumpnum)
        music->lumpnum = W_CheckNumForName(namebuf);

    if (music->lumpnum == -1)
    {
        char    *temp = uppercase(namebuf);

        C_Warning(1, "The " BOLD("%s") " music lump can't be found.", temp);
        free(temp);

        return;
    }

    // load and register it
    music->data = W_CacheLumpNum(music->lumpnum);

    if (!(handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum))))
#if defined(_WIN32)
        if (!midimusictype || !windowsmidi)
#endif
        {
            char    *filename = M_TempFile(DOOMRETRO ".mp3");

            if (W_WriteFile(filename, music->data, W_LumpLength(music->lumpnum)))
                handle = Mix_LoadMUS(filename);

            free(filename);

            if (!handle)
            {
                char    *temp = uppercase(namebuf);

                C_Warning(1, "The " BOLD("%s") " music lump can't be played.", temp);
                free(temp);

                return;
            }
        }

    music->handle = handle;

    // Play it
    I_PlaySong(handle, looping);

    S_RestoreMusicVolume();

    mus_playing = music;

    // [crispy] musinfo.items[0] is reserved for the map's default music
    if (!musinfo.items[0])
    {
        musinfo.items[0] = music->lumpnum;
        s_music[mus_musinfo].lumpnum = -1;
    }
}

void S_StopMusic(void)
{
    if (!mus_playing)
        return;

    if (mus_paused)
        I_ResumeSong();

    I_StopSong();
    I_UnregisterSong(mus_playing->handle);

    if (mus_playing->lumpnum >= 0)
        W_ReleaseLumpNum(mus_playing->lumpnum);

    mus_playing->data = NULL;
    mus_playing = NULL;
}

void S_ChangeMusInfoMusic(const int lumpnum, const bool looping)
{
    musicinfo_t *music;
    void        *handle;

    if (nomusic || (mus_playing && mus_playing->lumpnum == lumpnum))
        return;

    music = &s_music[mus_musinfo];

    if (music->lumpnum == lumpnum)
        return;

    // shutdown old music
    S_StopMusic();

    // save lumpnum
    music->lumpnum = lumpnum;

    // load and register it
    music->data = W_CacheLumpNum(lumpnum);

    if (!(handle = I_RegisterSong(music->data, W_LumpLength(lumpnum))))
#if defined(_WIN32)
        if (!midimusictype || !windowsmidi)
#endif
        {
            char    *filename = M_TempFile(DOOMRETRO ".mp3");

            if (W_WriteFile(filename, music->data, W_LumpLength(lumpnum)))
                handle = Mix_LoadMUS(filename);

            free(filename);

            if (!handle)
                return;
        }

    music->handle = handle;

    S_RestoreMusicVolume();

    // play it
    I_PlaySong(handle, looping);

    mus_playing = music;
    M_StringCopy(mus_playing->name1, lumpinfo[lumpnum]->name, sizeof(mus_playing->name1));

    musinfo.currentitem = lumpnum;
}

//
// S_ParseMusInfo
// Parses MUSINFO lump.
//
void S_ParseMusInfo(const char *lumpname)
{
    memset(&musinfo, 0, sizeof(musinfo));
    musinfo.currentitem = -1;

    s_music[NUMMUSIC].lumpnum = -1;

    if (W_CheckNumForName("MUSINFO") >= 0)
    {
        bool    inmap = false;

        SC_Open(W_CheckNumForName("MUSINFO"));

        while (SC_GetString())
            if (inmap || SC_Compare(lumpname))
            {
                unsigned int    num = 0;

                if (!inmap)
                {
                    SC_GetString();
                    inmap = true;
                }

                if (toupper(sc_String[0]) == 'E' || toupper(sc_String[0]) == 'M')
                    break;

                // Check number in range
                if (M_StrToInt(sc_String, (int *)&num) && num > 0 && num < MAX_MUS_ENTRIES && SC_GetString())
                {
                    const int   lumpnum = W_CheckNumForName(sc_String);

                    if (lumpnum >= 0)
                    {
                        musinfo.items[num] = lumpnum;
                        W_CacheLumpNum(lumpnum);
                    }
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
        musinfo.tics = (maptime ? 30 : 0);
    }
}

void T_MAPMusic(void)
{
    if (!musinfo.mapthing || musinfo.tics < 0)
        return;

    if (musinfo.tics > 0)
        musinfo.tics--;
    else if (musinfo.lastmapthing != musinfo.mapthing)
    {
        const int   arraypt = TIDNUM(musinfo.mapthing);

        if (arraypt >= 0 && arraypt < MAX_MUS_ENTRIES)
        {
            const int   lumpnum = musinfo.items[arraypt];

            if (lumpnum > 0 && lumpnum < numlumps)
                S_ChangeMusInfoMusic(lumpnum, true);
        }

        musinfo.tics = -1;
    }
}
