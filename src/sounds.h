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

#if !defined(__SOUNDS_H__)
#define __SOUNDS_H__

#include "doomtype.h"

// so that the individual game logic and sound driver code agree
#define NORM_PITCH  127

#define SFX_BDCLS   (gamemission == heretic ? hsfx_doropn : sfx_bdcls)
#define SFX_BDOPN   (gamemission == heretic ? hsfx_doropn : sfx_bdopn)
#define SFX_DORCLS  (gamemission == heretic ? hsfx_doropn : sfx_dorcls)
#define SFX_DOROPN  (gamemission == heretic ? hsfx_doropn : sfx_doropn)
#define SFX_ITEMUP  (gamemission == heretic ? hsfx_itemup : sfx_itemup)
#define SFX_NOWAY   (gamemission == heretic ? hsfx_plroof : sfx_noway)
#define SFX_OOF     (gamemission == heretic ? hsfx_plroof : sfx_oof)
#define SFX_PSTOP   (gamemission == heretic ? hsfx_pstop : sfx_pstop)
#define SFX_STNMOV  (gamemission == heretic ? hsfx_dormov : sfx_stnmov)
#define SFX_SWTCHN  (gamemission == heretic ? hsfx_switch : sfx_swtchn)

//
// SoundFX struct.
//
typedef struct sfxinfo_struct sfxinfo_t;

enum
{
    sg_none,
    sg_itemup,
    sg_wpnup,
    sg_oof,
    sg_getpow,
    sg_stnmov,
    sg_saw
};

struct sfxinfo_struct
{
    // up to 6-character name
    char        name[9];

    // SFX singularity (only one at a time)
    int         singularity;

    // SFX priority
    int         priority;

    // referenced sound if a link
    sfxinfo_t   *link;

    // volume if a link
    int         volume;

    // lump number of SFX
    int         lumpnum;
};

//
// MusicInfo struct.
//
typedef struct
{
    // up to 6-character name
    char        name[9];

    char        title[32];

    // lump number of music
    int         lumpnum;

    // music data
    void        *data;

    // music handle once registered
    void        *handle;
} musicinfo_t;

// the complete set of sound effects
extern sfxinfo_t    S_sfx[];
extern sfxinfo_t    HS_sfx[];

// the complete set of music
extern musicinfo_t  S_music[];
extern musicinfo_t  HS_music[];

extern musicinfo_t  *mus_playing;

extern dboolean     midimusictype;
extern dboolean     musmusictype;

//
// Identifiers for all music in game.
//
enum
{
    mus_None,
    mus_e1m1,
    mus_e1m2,
    mus_e1m3,
    mus_e1m4,
    mus_e1m5,
    mus_e1m6,
    mus_e1m7,
    mus_e1m8,
    mus_e1m9,
    mus_e2m1,
    mus_e2m2,
    mus_e2m3,
    mus_e2m4,
    mus_e2m5,
    mus_e2m6,
    mus_e2m7,
    mus_e2m8,
    mus_e2m9,
    mus_e3m1,
    mus_e3m2,
    mus_e3m3,
    mus_e3m4,
    mus_e3m5,
    mus_e3m6,
    mus_e3m7,
    mus_e3m8,
    mus_e3m9,
    mus_inter,
    mus_intro,
    mus_bunny,
    mus_victor,
    mus_introa,
    mus_runnin,
    mus_stalks,
    mus_countd,
    mus_betwee,
    mus_doom,
    mus_the_da,
    mus_shawn,
    mus_ddtblu,
    mus_in_cit,
    mus_dead,
    mus_stlks2,
    mus_theda2,
    mus_doom2,
    mus_ddtbl2,
    mus_runni2,
    mus_dead2,
    mus_stlks3,
    mus_romero,
    mus_shawn2,
    mus_messag,
    mus_count2,
    mus_ddtbl3,
    mus_ampie,
    mus_theda3,
    mus_adrian,
    mus_messg2,
    mus_romer2,
    mus_tense,
    mus_shawn3,
    mus_openin,
    mus_evil,
    mus_ultima,
    mus_read_m,
    mus_dm2ttl,
    mus_dm2int,
    NUMMUSIC
};

enum
{
    hmus_e1m1,
    hmus_e1m2,
    hmus_e1m3,
    hmus_e1m4,
    hmus_e1m5,
    hmus_e1m6,
    hmus_e1m7,
    hmus_e1m8,
    hmus_e1m9,

    hmus_e2m1,
    hmus_e2m2,
    hmus_e2m3,
    hmus_e2m4,
    hmus_e2m5,
    hmus_e2m6,
    hmus_e2m7,
    hmus_e2m8,
    hmus_e2m9,

    hmus_e3m1,
    hmus_e3m2,
    hmus_e3m3,
    hmus_e3m4,
    hmus_e3m5,
    hmus_e3m6,
    hmus_e3m7,
    hmus_e3m8,
    hmus_e3m9,

    hmus_e4m1,
    hmus_e4m2,
    hmus_e4m3,
    hmus_e4m4,
    hmus_e4m5,
    hmus_e4m6,
    hmus_e4m7,
    hmus_e4m8,
    hmus_e4m9,

    hmus_e5m1,
    hmus_e5m2,
    hmus_e5m3,
    hmus_e5m4,
    hmus_e5m5,
    hmus_e5m6,
    hmus_e5m7,
    hmus_e5m8,
    hmus_e5m9,

    hmus_e6m1,
    hmus_e6m2,
    hmus_e6m3,

    hmus_titl,
    hmus_intr,
    hmus_cptd,

    NUMHMUSIC
};

//
// Identifiers for all sfx in game.
//
enum
{
    sfx_None,
    sfx_pistol,
    sfx_shotgn,
    sfx_sgcock,
    sfx_dshtgn,
    sfx_dbopn,
    sfx_dbcls,
    sfx_dbload,
    sfx_plasma,
    sfx_bfg,
    sfx_sawup,
    sfx_sawidl,
    sfx_sawful,
    sfx_sawhit,
    sfx_rlaunc,
    sfx_rxplod,
    sfx_firsht,
    sfx_firxpl,
    sfx_pstart,
    sfx_pstop,
    sfx_doropn,
    sfx_dorcls,
    sfx_stnmov,
    sfx_swtchn,
    sfx_swtchx,
    sfx_plpain,
    sfx_dmpain,
    sfx_popain,
    sfx_vipain,
    sfx_mnpain,
    sfx_pepain,
    sfx_slop,
    sfx_itemup,
    sfx_wpnup,
    sfx_oof,
    sfx_telept,
    sfx_posit1,
    sfx_posit2,
    sfx_posit3,
    sfx_bgsit1,
    sfx_bgsit2,
    sfx_sgtsit,
    sfx_cacsit,
    sfx_brssit,
    sfx_cybsit,
    sfx_spisit,
    sfx_bspsit,
    sfx_kntsit,
    sfx_vilsit,
    sfx_mansit,
    sfx_pesit,
    sfx_sklatk,
    sfx_sgtatk,
    sfx_skepch,
    sfx_vilatk,
    sfx_claw,
    sfx_skeswg,
    sfx_pldeth,
    sfx_pdiehi,
    sfx_podth1,
    sfx_podth2,
    sfx_podth3,
    sfx_bgdth1,
    sfx_bgdth2,
    sfx_sgtdth,
    sfx_cacdth,
    sfx_skldth,
    sfx_brsdth,
    sfx_cybdth,
    sfx_spidth,
    sfx_bspdth,
    sfx_vildth,
    sfx_kntdth,
    sfx_pedth,
    sfx_skedth,
    sfx_posact,
    sfx_bgact,
    sfx_dmact,
    sfx_bspact,
    sfx_bspwlk,
    sfx_vilact,
    sfx_noway,
    sfx_barexp,
    sfx_punch,
    sfx_hoof,
    sfx_metal,
    sfx_chgun,
    sfx_tink,
    sfx_bdopn,
    sfx_bdcls,
    sfx_itmbk,
    sfx_flame,
    sfx_flamst,
    sfx_getpow,
    sfx_bospit,
    sfx_boscub,
    sfx_bossit,
    sfx_bospn,
    sfx_bosdth,
    sfx_manatk,
    sfx_mandth,
    sfx_sssit,
    sfx_ssdth,
    sfx_keenpn,
    sfx_keendt,
    sfx_skeact,
    sfx_skesit,
    sfx_skeatk,
    sfx_radio,

    // killough 11/98: dog sounds
    sfx_dgsit,
    sfx_dgatk,
    sfx_dgact,
    sfx_dgdth,
    sfx_dgpain,

    sfx_secret,

    NUMSFX
};

enum
{
    hsfx_None,
    hsfx_gldhit,
    hsfx_gntful,
    hsfx_gnthit,
    hsfx_gntpow,
    hsfx_gntact,
    hsfx_gntuse,
    hsfx_phosht,
    hsfx_phohit,
    hsfx_phopow,
    hsfx_lobsht,
    hsfx_lobhit,
    hsfx_lobpow,
    hsfx_hrnsht,
    hsfx_hrnhit,
    hsfx_hrnpow,
    hsfx_ramphit,
    hsfx_ramrain,
    hsfx_bowsht,
    hsfx_stfhit,
    hsfx_stfpow,
    hsfx_stfcrk,
    hsfx_impsit,
    hsfx_impat1,
    hsfx_impat2,
    hsfx_impdth,
    hsfx_impact,
    hsfx_imppai,
    hsfx_mumsit,
    hsfx_mumat1,
    hsfx_mumat2,
    hsfx_mumdth,
    hsfx_mumact,
    hsfx_mumpai,
    hsfx_mumhed,
    hsfx_bstsit,
    hsfx_bstatk,
    hsfx_bstdth,
    hsfx_bstact,
    hsfx_bstpai,
    hsfx_clksit,
    hsfx_clkatk,
    hsfx_clkdth,
    hsfx_clkact,
    hsfx_clkpai,
    hsfx_snksit,
    hsfx_snkatk,
    hsfx_snkdth,
    hsfx_snkact,
    hsfx_snkpai,
    hsfx_kgtsit,
    hsfx_kgtatk,
    hsfx_kgtat2,
    hsfx_kgtdth,
    hsfx_kgtact,
    hsfx_kgtpai,
    hsfx_wizsit,
    hsfx_wizatk,
    hsfx_wizdth,
    hsfx_wizact,
    hsfx_wizpai,
    hsfx_minsit,
    hsfx_minat1,
    hsfx_minat2,
    hsfx_minat3,
    hsfx_mindth,
    hsfx_minact,
    hsfx_minpai,
    hsfx_hedsit,
    hsfx_hedat1,
    hsfx_hedat2,
    hsfx_hedat3,
    hsfx_heddth,
    hsfx_hedact,
    hsfx_hedpai,
    hsfx_sorzap,
    hsfx_sorrise,
    hsfx_sorsit,
    hsfx_soratk,
    hsfx_soract,
    hsfx_sorpai,
    hsfx_sordsph,
    hsfx_sordexp,
    hsfx_sordbon,
    hsfx_sbtsit,
    hsfx_sbtatk,
    hsfx_sbtdth,
    hsfx_sbtact,
    hsfx_sbtpai,
    hsfx_plroof,
    hsfx_plrpai,
    hsfx_plrdth,
    hsfx_gibdth,
    hsfx_plrwdth,
    hsfx_plrcdth,
    hsfx_itemup,
    hsfx_wpnup,
    hsfx_telept,
    hsfx_doropn,
    hsfx_dorcls,
    hsfx_dormov,
    hsfx_artiup,
    hsfx_switch,
    hsfx_pstart,
    hsfx_pstop,
    hsfx_stnmov,
    hsfx_chicpai,
    hsfx_chicatk,
    hsfx_chicdth,
    hsfx_chicact,
    hsfx_chicpk1,
    hsfx_chicpk2,
    hsfx_chicpk3,
    hsfx_keyup,
    hsfx_ripslop,
    hsfx_newpod,
    hsfx_podexp,
    hsfx_bounce,
    hsfx_volsht,
    hsfx_volhit,
    hsfx_burn,
    hsfx_splash,
    hsfx_gloop,
    hsfx_respawn,
    hsfx_blssht,
    hsfx_blshit,
    hsfx_chat,
    hsfx_artiuse,
    hsfx_gfrag,
    hsfx_waterfl,
    hsfx_wind,
    hsfx_amb1,
    hsfx_amb2,
    hsfx_amb3,
    hsfx_amb4,
    hsfx_amb5,
    hsfx_amb6,
    hsfx_amb7,
    hsfx_amb8,
    hsfx_amb9,
    hsfx_amb10,
    hsfx_amb11,

    NUMHSFX
};

#endif
