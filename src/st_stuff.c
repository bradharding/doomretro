/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

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

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_swap.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_setup.h"
#include "s_sound.h"
#include "st_lib.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"

//
// STATUS BAR DATA
//

// Radiation suit, green shift.
#define RADIATIONPAL        13

// Location of status bar
#define ST_X                0
#define ST_Y                (SCREENHEIGHT - SBARHEIGHT)

#define ST_TURNOFFSET       ST_NUMSTRAIGHTFACES
#define ST_OUCHOFFSET       (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET   (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET    (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE          (ST_NUMPAINFACES * ST_FACESTRIDE)
#define ST_DEADFACE         (ST_GODFACE + 1)

#define ST_FACESX           (143 + chex)
#define ST_FACESY           168

#define ST_FACEBACKX        (143 * SCREENSCALE)
#define ST_FACEBACKY        (168 * SCREENSCALE)
#define ST_FACEBACKWIDTH    (34 * SCREENSCALE)
#define ST_FACEBACKHEIGHT   (32 * SCREENSCALE)

#define ST_EVILGRINCOUNT    (2 * TICRATE)
#define ST_TURNCOUNT        (1 * TICRATE)
#define ST_RAMPAGEDELAY     (2 * TICRATE)

#define ST_MUCHPAIN         20

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH        3
#define ST_AMMOX            44
#define ST_AMMOY            171

// HEALTH number pos.
#define ST_HEALTHX          90
#define ST_HEALTHY          171

// Weapon pos.
#define ST_ARMSX            111
#define ST_ARMSY            172
#define ST_ARMSBGX          104
#define ST_ARMSXSPACE       12
#define ST_ARMSYSPACE       10

// ARMOR number pos.
#define ST_ARMORX           220
#define ST_ARMORY           171

// Key icon positions.
#define ST_KEY0X            238
#define ST_KEY0Y            171
#define ST_KEY1X            238
#define ST_KEY1Y            181
#define ST_KEY2X            238
#define ST_KEY2Y            191

// Ammunition counter.
#define ST_AMMO0WIDTH       3
#define ST_AMMO0X           288
#define ST_AMMO0Y           173
#define ST_AMMO1WIDTH       ST_AMMO0WIDTH
#define ST_AMMO1X           288
#define ST_AMMO1Y           179
#define ST_AMMO2WIDTH       ST_AMMO0WIDTH
#define ST_AMMO2X           288
#define ST_AMMO2Y           191
#define ST_AMMO3WIDTH       ST_AMMO0WIDTH
#define ST_AMMO3X           288
#define ST_AMMO3Y           185

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH    3
#define ST_MAXAMMO0X        314
#define ST_MAXAMMO0Y        173
#define ST_MAXAMMO1WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X        314
#define ST_MAXAMMO1Y        179
#define ST_MAXAMMO2WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X        314
#define ST_MAXAMMO2Y        191
#define ST_MAXAMMO3WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X        314
#define ST_MAXAMMO3Y        185

// ST_Start() has just been called
static dboolean             st_firsttime;

// whether left-side main status bar is active
static dboolean             st_statusbaron;

// main bar left
static patch_t              *sbar;
static patch_t              *sbar2;

// 0-9, tall numbers
patch_t                     *tallnum[10];

// tall % sign
patch_t                     *tallpercent;
short                       tallpercentwidth;
dboolean                    emptytallpercent;

// 0-9, short, yellow (,different!) numbers
static patch_t              *shortnum[10];

// 3 key-cards, 3 skulls
static patch_t              *keys[NUMCARDS];

// face status patches
patch_t                     *faces[ST_NUMFACES];

// main bar right
static patch_t              *armsbg;

// weapon ownership patches
static patch_t              *arms[6][2];

// ready-weapon widget
static st_number_t          w_ready;

// health widget
static st_percent_t         w_health;

// weapon ownership widgets
static st_multicon_t        w_arms[6];

// face status widget
static st_multicon_t        w_faces;

// keycard widgets
static st_multicon_t        w_keyboxes[3];

// armor widget
static st_percent_t         w_armor;

// ammo widgets
static st_number_t          w_ammo[4];

// max ammo widgets
static st_number_t          w_maxammo[4];

patch_t                     *grnrock;
patch_t                     *brdr_t;
patch_t                     *brdr_b;
patch_t                     *brdr_l;
patch_t                     *brdr_r;
patch_t                     *brdr_tl;
patch_t                     *brdr_tr;
patch_t                     *brdr_bl;
patch_t                     *brdr_br;

// used to use appropriately pained face
static int                  st_oldhealth = -1;

// used for evil grin
dboolean                    oldweaponsowned[NUMWEAPONS];

int                         st_palette = 0;

// count until face changes
int                         st_facecount;

// current face index, used by w_faces
int                         st_faceindex;

static int                  st_shotguns;

// holds key-type for each key box on bar
static int                  keyboxes[3];

int                         oldhealth = 100;

dboolean                    idclev;
int                         idclevtics;

dboolean                    idmus;

int                         facebackcolor = facebackcolor_default;
int                         r_berserkintensity = r_berserkintensity_default;

uint64_t                    stat_cheated = 0;

cheatseq_t cheat_mus = CHEAT("idmus", 0);
cheatseq_t cheat_mus_xy = CHEAT("idmus", 2);
cheatseq_t cheat_god = CHEAT("iddqd", 0);
cheatseq_t cheat_ammo = CHEAT("idkfa", 0);
cheatseq_t cheat_ammonokey = CHEAT("idfa", 0);
cheatseq_t cheat_noclip = CHEAT("idspispopd", 0);
cheatseq_t cheat_commercial_noclip = CHEAT("idclip", 0);

cheatseq_t cheat_powerup[7] =
{
    CHEAT("idbeholdv", 0),
    CHEAT("idbeholds", 0),
    CHEAT("idbeholdi", 0),
    CHEAT("idbeholdr", 0),
    CHEAT("idbeholda", 0),
    CHEAT("idbeholdl", 0),
    CHEAT("idbehold",  0)
};

cheatseq_t cheat_choppers = CHEAT("idchoppers", 0);
cheatseq_t cheat_clev = CHEAT("idclev", 0);
cheatseq_t cheat_clev_xy = CHEAT("idclev", 2);
cheatseq_t cheat_mypos = CHEAT("idmypos", 0);
cheatseq_t cheat_amap = CHEAT("iddt", 0);
cheatseq_t cheat_buddha = CHEAT("mumu", 0);

static dboolean movekey(char key)
{
    return (key == keyboardright || key == keyboardleft || key == keyboardforward || key == keyboardforward2
        || key == keyboardback || key == keyboardback2 || key == keyboardstrafeleft || key == keyboardstraferight);
}

static void ST_InitCheats(void)
{
    cheat_mus.movekey = movekey(cheat_mus.sequence[0]);
    cheat_mus_xy.movekey = movekey(cheat_mus_xy.sequence[0]);
    cheat_god.movekey = movekey(cheat_god.sequence[0]);
    cheat_ammo.movekey = movekey(cheat_ammo.sequence[0]);
    cheat_ammonokey.movekey = movekey(cheat_ammonokey.sequence[0]);
    cheat_noclip.movekey = movekey(cheat_noclip.sequence[0]);
    cheat_commercial_noclip.movekey = movekey(cheat_commercial_noclip.sequence[0]);
    cheat_powerup[0].movekey = movekey(cheat_powerup[0].sequence[0]);
    cheat_powerup[1].movekey = movekey(cheat_powerup[1].sequence[0]);
    cheat_powerup[2].movekey = movekey(cheat_powerup[2].sequence[0]);
    cheat_powerup[3].movekey = movekey(cheat_powerup[3].sequence[0]);
    cheat_powerup[4].movekey = movekey(cheat_powerup[4].sequence[0]);
    cheat_powerup[5].movekey = movekey(cheat_powerup[5].sequence[0]);
    cheat_powerup[6].movekey = movekey(cheat_powerup[6].sequence[0]);
    cheat_choppers.movekey = movekey(cheat_choppers.sequence[0]);
    cheat_buddha.movekey = movekey(cheat_buddha.sequence[0]);
    cheat_clev.movekey = movekey(cheat_clev.sequence[0]);
    cheat_clev_xy.movekey = movekey(cheat_clev_xy.sequence[0]);
    cheat_mypos.movekey = movekey(cheat_mypos.sequence[0]);
    cheat_amap.movekey = movekey(cheat_amap.sequence[0]);
}

#define NONE        -1
#define IDMUS_MAX   60

static const int mus[IDMUS_MAX][6] =
{
    /* xy      shareware    registered   commercial   retail      bfgedition   nerve      */
    /* 00 */ { NONE,        NONE,        NONE,        NONE,       NONE,        NONE       },
    /* 01 */ { NONE,        NONE,        mus_runnin,  NONE,       mus_runnin,  mus_messag },
    /* 02 */ { NONE,        NONE,        mus_stalks,  NONE,       mus_stalks,  mus_ddtblu },
    /* 03 */ { NONE,        NONE,        mus_countd,  NONE,       mus_countd,  mus_doom   },
    /* 04 */ { NONE,        NONE,        mus_betwee,  NONE,       mus_betwee,  mus_shawn  },
    /* 05 */ { NONE,        NONE,        mus_doom,    NONE,       mus_doom,    mus_in_cit },
    /* 06 */ { NONE,        NONE,        mus_the_da,  NONE,       mus_the_da,  mus_the_da },
    /* 07 */ { NONE,        NONE,        mus_shawn,   NONE,       mus_shawn,   mus_in_cit },
    /* 08 */ { NONE,        NONE,        mus_ddtblu,  NONE,       mus_ddtblu,  mus_shawn  },
    /* 09 */ { NONE,        NONE,        mus_in_cit,  NONE,       mus_in_cit,  mus_ddtblu },
    /* 10 */ { NONE,        NONE,        mus_dead,    NONE,       mus_dead,    NONE       },
    /* 11 */ { mus_e1m1,    mus_e1m1,    mus_stlks2,  mus_e1m1,   mus_stlks2,  NONE       },
    /* 12 */ { mus_e1m2,    mus_e1m2,    mus_theda2,  mus_e1m2,   mus_theda2,  NONE       },
    /* 13 */ { mus_e1m3,    mus_e1m3,    mus_doom2,   mus_e1m3,   mus_doom2,   NONE       },
    /* 14 */ { mus_e1m4,    mus_e1m4,    mus_ddtbl2,  mus_e1m4,   mus_ddtbl2,  NONE       },
    /* 15 */ { mus_e1m5,    mus_e1m5,    mus_runni2,  mus_e1m5,   mus_runni2,  NONE       },
    /* 16 */ { mus_e1m6,    mus_e1m6,    mus_dead2,   mus_e1m6,   mus_dead2,   NONE       },
    /* 17 */ { mus_e1m7,    mus_e1m7,    mus_stlks3,  mus_e1m7,   mus_stlks3,  NONE       },
    /* 18 */ { mus_e1m8,    mus_e1m8,    mus_romero,  mus_e1m8,   mus_romero,  NONE       },
    /* 19 */ { mus_e1m9,    mus_e1m9,    mus_shawn2,  mus_e1m9,   mus_shawn2,  NONE       },
    /* 20 */ { NONE,        NONE,        mus_messag,  NONE,       mus_messag,  NONE       },
    /* 21 */ { NONE,        mus_e2m1,    mus_count2,  mus_e2m1,   mus_count2,  NONE       },
    /* 22 */ { NONE,        mus_e2m2,    mus_ddtbl3,  mus_e2m2,   mus_ddtbl3,  NONE       },
    /* 23 */ { NONE,        mus_e2m3,    mus_ampie,   mus_e2m3,   mus_ampie,   NONE       },
    /* 24 */ { NONE,        mus_e2m4,    mus_theda3,  mus_e2m4,   mus_theda3,  NONE       },
    /* 25 */ { NONE,        mus_e2m5,    mus_adrian,  mus_e2m5,   mus_adrian,  NONE       },
    /* 26 */ { NONE,        mus_e2m6,    mus_messg2,  mus_e2m6,   mus_messg2,  NONE       },
    /* 27 */ { NONE,        mus_e2m7,    mus_romer2,  mus_e2m7,   mus_romer2,  NONE       },
    /* 28 */ { NONE,        mus_e2m8,    mus_tense,   mus_e2m8,   mus_tense,   NONE       },
    /* 29 */ { NONE,        mus_e2m9,    mus_shawn3,  mus_e2m9,   mus_shawn3,  NONE       },
    /* 30 */ { NONE,        NONE,        mus_openin,  NONE,       mus_openin,  NONE       },
    /* 31 */ { NONE,        mus_e3m1,    mus_evil,    mus_e3m1,   mus_evil,    NONE       },
    /* 32 */ { NONE,        mus_e3m2,    mus_ultima,  mus_e3m2,   mus_ultima,  NONE       },
    /* 33 */ { NONE,        mus_e3m3,    NONE,        mus_e3m3,   mus_read_m,  NONE       },
    /* 34 */ { NONE,        mus_e3m4,    NONE,        mus_e3m4,   NONE,        NONE       },
    /* 35 */ { NONE,        mus_e3m5,    NONE,        mus_e3m5,   NONE,        NONE       },
    /* 36 */ { NONE,        mus_e3m6,    NONE,        mus_e3m6,   NONE,        NONE       },
    /* 37 */ { NONE,        mus_e3m7,    NONE,        mus_e3m7,   NONE,        NONE       },
    /* 38 */ { NONE,        mus_e3m8,    NONE,        mus_e3m8,   NONE,        NONE       },
    /* 39 */ { NONE,        mus_e3m9,    NONE,        mus_e3m9,   NONE,        NONE       },
    /* 40 */ { NONE,        NONE,        NONE,        NONE,       NONE,        NONE       },
    /* 41 */ { NONE,        NONE,        NONE,        mus_e3m4,   NONE,        NONE       },
    /* 42 */ { NONE,        NONE,        NONE,        mus_e3m2,   NONE,        NONE       },
    /* 43 */ { NONE,        NONE,        NONE,        mus_e3m3,   NONE,        NONE       },
    /* 44 */ { NONE,        NONE,        NONE,        mus_e1m5,   NONE,        NONE       },
    /* 45 */ { NONE,        NONE,        NONE,        mus_e2m7,   NONE,        NONE       },
    /* 46 */ { NONE,        NONE,        NONE,        mus_e2m4,   NONE,        NONE       },
    /* 47 */ { NONE,        NONE,        NONE,        mus_e2m6,   NONE,        NONE       },
    /* 48 */ { NONE,        NONE,        NONE,        mus_e2m5,   NONE,        NONE       },
    /* 49 */ { NONE,        NONE,        NONE,        mus_e1m9,   NONE,        NONE       },
    /* 50 */ { NONE,        NONE,        NONE,        NONE,       NONE,        NONE       },
    /* 51 */ { NONE,        NONE,        NONE,        mus_e5m1,   NONE,        NONE       },
    /* 52 */ { NONE,        NONE,        NONE,        mus_e5m2,   NONE,        NONE       },
    /* 53 */ { NONE,        NONE,        NONE,        mus_e5m3,   NONE,        NONE       },
    /* 54 */ { NONE,        NONE,        NONE,        mus_e5m4,   NONE,        NONE       },
    /* 55 */ { NONE,        NONE,        NONE,        mus_e5m5,   NONE,        NONE       },
    /* 56 */ { NONE,        NONE,        NONE,        mus_e5m6,   NONE,        NONE       },
    /* 57 */ { NONE,        NONE,        NONE,        mus_e5m7,   NONE,        NONE       },
    /* 58 */ { NONE,        NONE,        NONE,        mus_e5m8,   NONE,        NONE       },
    /* 59 */ { NONE,        NONE,        NONE,        mus_e5m9,   NONE,        NONE       }
};

//
// STATUS BAR CODE
//
static void ST_RefreshBackground(void)
{
    if (st_statusbaron)
    {
#if SCREENSCALE == 1
        if (STBAR >= 3)
        {
            V_DrawSTBARPatch(ST_X, VANILLAHEIGHT - VANILLASBARHEIGHT, sbar);
            V_DrawPatch(ST_ARMSBGX + hacx * 4, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, armsbg);
        }
        else
            V_DrawSTBARPatch(ST_X, VANILLAHEIGHT - VANILLASBARHEIGHT, sbar);
#else
        if (STBAR >= 3)
        {
            V_DrawSTBARPatch(ST_X, VANILLAHEIGHT - VANILLASBARHEIGHT, sbar);
            V_DrawPatch(ST_ARMSBGX + hacx * 4, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, armsbg);
        }
        else if (r_detail == r_detail_low)
            V_DrawSTBARPatch(ST_X, VANILLAHEIGHT - VANILLASBARHEIGHT, sbar);
        else
            V_DrawBigPatch(ST_X, ST_Y, sbar2);
#endif
    }
}

static int ST_CalcPainOffset(void);

// Respond to keyboard input events,
//  intercept cheats.
dboolean ST_Responder(event_t *ev)
{
    // if a user keypress...
    if (ev->type == ev_keydown || *consolecheat)
    {
        if (!menuactive && !paused)     // [BH] no cheats when in menu or paused
        {
            if (!*consolecheat && cht_CheckCheat(&cheat_mus, ev->data2) && !nomusic && musicVolume)
                idmus = true;

            // 'dqd' cheat for toggleable god mode
            if (cht_CheckCheat(&cheat_god, ev->data2) && gameskill != sk_nightmare)
            {
                S_StartSound(NULL, sfx_getpow);
                C_Input(cheat_god.sequence);

                // [BH] if player is dead, resurrect them first
                if (viewplayer->health <= 0)
                    P_ResurrectPlayer(initial_health);

                viewplayer->cheats ^= CF_GODMODE;

                if (viewplayer->cheats & CF_GODMODE)
                {
                    viewplayer->cheats &= ~CF_BUDDHA;

                    if (viewplayer->powers[pw_invulnerability] > STARTFLASHING)
                        viewplayer->powers[pw_invulnerability] = STARTFLASHING;

                    // [BH] remember player's current health,
                    //  and only set to 100% if less than 100%
                    oldhealth = viewplayer->health;
                    P_GiveBody(god_health, god_health, false);

                    if (oldhealth < initial_health)
                        P_AddBonus();

                    C_Output(s_STSTR_DQDON);
                    HU_SetPlayerMessage(s_STSTR_DQDON, false, false);

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
                else
                {
                    C_Output(s_STSTR_DQDOFF);
                    HU_SetPlayerMessage(s_STSTR_DQDOFF, false, false);

                    // [BH] restore player's health
                    viewplayer->health = oldhealth;
                    viewplayer->mo->health = oldhealth;
                }

                message_dontfuckwithme = true;
            }

            // 'fa' cheat for killer fucking arsenal
            else if (cht_CheckCheat(&cheat_ammonokey, ev->data2) && gameskill != sk_nightmare && viewplayer->health > 0)
            {
                dboolean    ammogiven = false;
                dboolean    armorgiven = false;
                dboolean    berserkgiven = false;
                dboolean    weaponsgiven = false;

                // [BH] note if doesn't have full armor before giving it
                if (viewplayer->armorpoints < idfa_armor || viewplayer->armortype < idfa_armor_class)
                {
                    armorgiven = true;
                    viewplayer->armorpoints = idfa_armor;
                    viewplayer->armortype = idfa_armor_class;
                }

                // [BH] note if any weapons given that player didn't have already
                weaponsgiven = P_GiveAllWeapons();

                // [BH] give player a berserk power-up so they can still use fists
                berserkgiven = P_GivePower(pw_strength);

                // [BH] give player a backpack if they don't have one
                P_GiveBackpack(false, false);

                ammogiven = P_GiveFullAmmo();

                // [BH] show evil grin if player was given any new weapons
                if (weaponsgiven && !(viewplayer->cheats & CF_GODMODE) && !viewplayer->powers[pw_invulnerability]
                    && (!vid_widescreen || (r_hud && !r_althud)))
                {
                    st_facecount = ST_EVILGRINCOUNT;
                    st_faceindex = ST_CalcPainOffset() + ST_EVILGRINOFFSET;
                }

                // [BH] only acknowledge cheat if player was given something
                if (ammogiven || armorgiven || berserkgiven || weaponsgiven)
                {
                    S_StartSound(NULL, sfx_getpow);
                    C_Input(cheat_ammonokey.sequence);

                    // [BH] flash screen
                    P_AddBonus();

                    C_Output(s_STSTR_FAADDED);
                    HU_SetPlayerMessage(s_STSTR_FAADDED, false, false);
                    message_dontfuckwithme = true;

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
            }

            // 'kfa' cheat for key full ammo
            else if (cht_CheckCheat(&cheat_ammo, ev->data2) && gameskill != sk_nightmare
                     // [BH] can only enter cheat while player is alive
                     && viewplayer->health > 0)
            {
                dboolean    ammogiven = false;
                dboolean    armorgiven = false;
                dboolean    berserkgiven = false;
                dboolean    keysgiven = false;
                dboolean    weaponsgiven = false;

                // [BH] note if doesn't have full armor before giving it
                if (viewplayer->armorpoints < idkfa_armor || viewplayer->armortype < idkfa_armor_class)
                {
                    armorgiven = true;
                    viewplayer->armorpoints = idkfa_armor;
                    viewplayer->armortype = idkfa_armor_class;
                }

                // [BH] note if any weapons given that player didn't have already
                weaponsgiven = P_GiveAllWeapons();

                // [BH] give player a berserk power-up so they can still use fists
                berserkgiven = P_GivePower(pw_strength);

                // [BH] give player a backpack if they don't have one
                P_GiveBackpack(false, false);

                ammogiven = P_GiveFullAmmo();

                // [BH] only give the player the keycards or skull keys from the
                //  current level, and note if any keys given
                keysgiven = P_GiveAllCardsInMap();

                // [BH] show evil grin if player was given any new weapons
                if (weaponsgiven && !(viewplayer->cheats & CF_GODMODE) && !viewplayer->powers[pw_invulnerability]
                    && (!vid_widescreen || (r_hud && !r_althud)))
                {
                    st_facecount = ST_EVILGRINCOUNT;
                    st_faceindex = ST_CalcPainOffset() + ST_EVILGRINOFFSET;
                }

                // [BH] only acknowledge cheat if player was given something
                if (ammogiven || armorgiven || berserkgiven || weaponsgiven || keysgiven)
                {
                    S_StartSound(NULL, sfx_getpow);
                    C_Input(cheat_ammo.sequence);

                    // [BH] flash screen
                    P_AddBonus();

                    C_Output(s_STSTR_KFAADDED);
                    HU_SetPlayerMessage(s_STSTR_KFAADDED, false, false);
                    message_dontfuckwithme = true;

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
            }

            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus_xy, ev->data2) && !nomusic && musicVolume)
            {
                char   buffer[3];

                // [BH] only display message if parameter is valid
                cht_GetParam(&cheat_mus_xy, buffer);

                // [BH] rewritten to use mus[] LUT
                // [BH] fix crash if IDMUS0y and IDMUSx0 entered in DOOM,
                //  IDMUS21 to IDMUS39 entered in shareware, and IDMUS00
                //  entered in DOOM II
                if (buffer[0] >= '0' && buffer[0] <= '9' && buffer[1] >= '0' && buffer[1] <= '9')
                {
                    int musnum = (buffer[0] - '0') * 10 + (buffer[1] - '0');

                    S_StartSound(NULL, sfx_getpow);
                    C_Input("%s%c%c", cheat_mus_xy.sequence, buffer[0], buffer[1]);

                    if (musnum < IDMUS_MAX)
                    {
                        if (gamemission == pack_nerve)
                            musnum = mus[musnum][5];
                        else if (bfgedition && gamemission == doom2)
                            musnum = mus[musnum][4];
                        else
                            musnum = mus[musnum][gamemode];

                        if (musnum != NONE)
                        {
                            static char msg[80];
                            char        *temp = uppercase(S_music[musnum].name1);

                            S_ChangeMusic(musnum, 1, true, false);

                            M_snprintf(msg, sizeof(msg), s_STSTR_MUS, temp);
                            C_Output(msg);
                            HU_SetPlayerMessage(msg, false, false);
                            message_dontfuckwithme = true;
                            free(temp);
                        }
                        else
                            idmus = false;
                    }
                    else
                        idmus = false;
                }
            }

            // no clipping mode cheat
            else if (((cht_CheckCheat(&cheat_noclip, ev->data2) && gamemode != commercial)
                || (cht_CheckCheat(&cheat_commercial_noclip, ev->data2) && gamemode == commercial))
                && gameskill != sk_nightmare
                // [BH] can only enter cheat while player is alive
                && viewplayer->health > 0)
            {
                S_StartSound(NULL, sfx_getpow);
                C_Input(gamemode == commercial ? cheat_commercial_noclip.sequence : cheat_noclip.sequence);

                viewplayer->cheats ^= CF_NOCLIP;

                if (viewplayer->cheats & CF_NOCLIP)
                {
                    C_Output(s_STSTR_NCON);
                    HU_SetPlayerMessage(s_STSTR_NCON, false, false);
                }
                else
                {
                    C_Output(s_STSTR_NCOFF);
                    HU_SetPlayerMessage(s_STSTR_NCOFF, false, false);
                }

                message_dontfuckwithme = true;

                if (viewplayer->cheats & CF_NOCLIP)
                {
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
            }

            // 'behold?' power-up cheats
            for (int i = 1; i < 7; i++)
            {
                if (cht_CheckCheat(&cheat_powerup[i - 1], ev->data2) && gameskill != sk_nightmare
                    // [BH] can only enter cheat while player is alive
                    && viewplayer->health > 0)
                {
                    S_StartSound(NULL, sfx_getpow);
                    C_Input(cheat_powerup[i - 1].sequence);
                    C_Output(s_STSTR_BEHOLD);

                    if ((i != pw_strength && viewplayer->powers[i] >= 0 && viewplayer->powers[i] <= STARTFLASHING)
                        || (i == pw_strength && !viewplayer->powers[i]))
                    {
                        if (i == pw_invulnerability)
                        {
                            viewplayer->cheats &= ~CF_BUDDHA;
                            viewplayer->cheats &= ~CF_GODMODE;
                        }

                        P_GivePower(i);

                        // [BH] set to -1 so power-up won't run out, but can still be toggled off using cheat
                        if (i != pw_strength)
                        {
                            viewplayer->powers[i] = -1;

                            // [BH] flash screen
                            P_AddBonus();
                        }
                        else
                        {
                            // [BH] switch to fists if 'idbeholds' cheat is entered
                            if (viewplayer->readyweapon != wp_fist)
                                P_EquipWeapon(wp_fist);

                            viewplayer->fistorchainsaw = wp_fist;

                            // [BH] cancel 'idchoppers' cheat if necessary
                            if (viewplayer->cheats & CF_CHOPPERS)
                            {
                                viewplayer->cheats &= ~CF_CHOPPERS;

                                viewplayer->powers[pw_invulnerability] = (viewplayer->invulnbeforechoppers ? 1 : STARTFLASHING);
                                viewplayer->weaponowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                                oldweaponsowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                            }
                        }

                        if (!M_StringCompare(s_STSTR_BEHOLDX, STSTR_BEHOLDX))
                        {
                            C_Output(s_STSTR_BEHOLDX);
                            HU_SetPlayerMessage(s_STSTR_BEHOLDX, false, false);
                        }
                        else
                        {
                            C_Output(s_STSTR_BEHOLDON);
                            HU_SetPlayerMessage(s_STSTR_BEHOLDON, false, false);
                        }

                        stat_cheated = SafeAdd(stat_cheated, 1);
                        viewplayer->cheated++;
                    }
                    else
                    {
                        // [BH] toggle berserk off
                        if (i == pw_strength)
                        {
                            viewplayer->powers[i] = 0;

                            if (viewplayer->readyweapon == wp_fist && viewplayer->weaponowned[wp_chainsaw])
                            {
                                P_EquipWeapon(wp_chainsaw);
                                viewplayer->fistorchainsaw = wp_chainsaw;
                            }
                        }
                        else
                        {
                            // [BH] cancel 'idchoppers' cheat if necessary
                            if (i == pw_invulnerability && (viewplayer->cheats & CF_CHOPPERS))
                            {
                                viewplayer->cheats &= ~CF_CHOPPERS;

                                if (viewplayer->weaponbeforechoppers != wp_chainsaw)
                                    P_EquipWeapon(viewplayer->weaponbeforechoppers);

                                viewplayer->weaponowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                                oldweaponsowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                            }

                            // [BH] start flashing palette to indicate power-up about to run out
                            viewplayer->powers[i] = STARTFLASHING * (i != pw_allmap);
                        }

                        if (!M_StringCompare(s_STSTR_BEHOLDX, STSTR_BEHOLDX))
                        {
                            C_Output(s_STSTR_BEHOLDX);
                            HU_SetPlayerMessage(s_STSTR_BEHOLDX, false, false);
                        }
                        else
                        {
                            C_Output(s_STSTR_BEHOLDOFF);
                            HU_SetPlayerMessage(s_STSTR_BEHOLDOFF, false, false);
                        }
                    }

                    // [BH] reset all cheat sequences
                    cheat_mus.chars_read = 0;
                    cheat_mus_xy.chars_read = 0;
                    cheat_mus_xy.param_chars_read = 0;
                    cheat_god.chars_read = 0;
                    cheat_ammo.chars_read = 0;
                    cheat_ammonokey.chars_read = 0;
                    cheat_noclip.chars_read = 0;
                    cheat_commercial_noclip.chars_read = 0;

                    for (int j = 0; j < 6; j++)
                        cheat_powerup[j].chars_read = 0;

                    cheat_choppers.chars_read = 0;
                    cheat_clev.chars_read = 0;
                    cheat_clev_xy.chars_read = 0;
                    cheat_clev_xy.param_chars_read = 0;
                    cheat_mypos.chars_read = 0;
                    cheat_amap.chars_read = 0;
                    cheat_buddha.chars_read = 0;
                    cheatkey = '\0';

                    message_dontfuckwithme = true;
                    idbehold = false;

                    C_HideConsole();

                    return true;
                }
            }

            // 'behold' power-up menu
            if (cht_CheckCheat(&cheat_powerup[6], ev->data2) && gameskill != sk_nightmare
                // [BH] can only enter cheat while player is alive
                && viewplayer->health > 0)
            {
                idbehold = true;

                HU_SetPlayerMessage(s_STSTR_BEHOLD, false, false);
                message_dontfuckwithme = true;
            }

            // 'choppers' invulnerability & chainsaw
            else if (cht_CheckCheat(&cheat_choppers, ev->data2) && gameskill != sk_nightmare
                     // [BH] can only enter cheat while player is alive
                     && viewplayer->health > 0)
            {
                S_StartSound(NULL, sfx_getpow);
                C_Input(cheat_choppers.sequence);

                if (!(viewplayer->cheats & CF_CHOPPERS))
                {
                    // [BH] flash screen
                    P_AddBonus();

                    // [BH] note if has chainsaw and/or invulnerability already
                    viewplayer->invulnbeforechoppers = viewplayer->powers[pw_invulnerability];
                    viewplayer->chainsawbeforechoppers = viewplayer->weaponowned[wp_chainsaw];

                    // [BH] note weapon before switching to chainsaw
                    if ((viewplayer->weaponbeforechoppers = viewplayer->readyweapon) != wp_chainsaw)
                    {
                        viewplayer->weaponowned[wp_chainsaw] = true;
                        oldweaponsowned[wp_chainsaw] = true;
                        P_EquipWeapon(wp_chainsaw);
                    }

                    viewplayer->cheats &= ~CF_BUDDHA;
                    viewplayer->cheats &= ~CF_GODMODE;

                    // [BH] fixed bug where invulnerability was never given, and now
                    //  needs to be toggled off with cheat or switch from chainsaw
                    P_GivePower(pw_invulnerability);
                    viewplayer->powers[pw_invulnerability] = -1;

                    C_Output(s_STSTR_CHOPPERS);
                    HU_SetPlayerMessage(s_STSTR_CHOPPERS, false, false);
                    message_dontfuckwithme = true;

                    viewplayer->cheats |= CF_CHOPPERS;

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
                else
                {
                    // [BH] can be toggled off
                    viewplayer->cheats &= ~CF_CHOPPERS;

                    viewplayer->powers[pw_invulnerability] = (viewplayer->invulnbeforechoppers ? 1 : STARTFLASHING);

                    if (viewplayer->weaponbeforechoppers != wp_chainsaw)
                        P_EquipWeapon(viewplayer->weaponbeforechoppers);

                    viewplayer->weaponowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                    oldweaponsowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                }
            }

            // 'mypos' for player position
            else if (cht_CheckCheat(&cheat_mypos, ev->data2))
            {
                S_StartSound(NULL, sfx_getpow);
                C_Input(cheat_mypos.sequence);

                // [BH] message stays on screen until toggled off again using
                //  cheat. Code is in hu_stuff.c.
                viewplayer->cheats ^= CF_MYPOS;

                if (viewplayer->cheats & CF_MYPOS)
                {
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
                else
                    HU_ClearMessages();
            }

            else if (cht_CheckCheat(&cheat_buddha, ev->data2) && gameskill != sk_nightmare && viewplayer->health > 0)
            {
                S_StartSound(NULL, sfx_getpow);
                C_Input(cheat_buddha.sequence);

                viewplayer->cheats ^= CF_BUDDHA;

                if (viewplayer->cheats & CF_BUDDHA)
                {
                    viewplayer->cheats &= ~CF_GODMODE;

                    if (viewplayer->powers[pw_invulnerability] > STARTFLASHING)
                        viewplayer->powers[pw_invulnerability] = STARTFLASHING;

                    C_Output(s_STSTR_BUDDHAON);
                    HU_SetPlayerMessage(s_STSTR_BUDDHAON, false, false);

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
                else
                {
                    C_Output(s_STSTR_BUDDHAOFF);
                    HU_SetPlayerMessage(s_STSTR_BUDDHAOFF, false, false);
                }

                message_dontfuckwithme = true;
            }

            else if ((automapactive || mapwindow) && cht_CheckCheat(&cheat_amap, ev->data2))
            {
                S_StartSound(NULL, sfx_getpow);
                C_Input(cheat_amap.sequence);

                if (viewplayer->cheats & CF_ALLMAP)
                {
                    viewplayer->cheats ^= CF_ALLMAP;
                    viewplayer->cheats ^= CF_ALLMAP_THINGS;

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
                else if (viewplayer->cheats & CF_ALLMAP_THINGS)
                    viewplayer->cheats ^= CF_ALLMAP_THINGS;
                else
                {
                    viewplayer->cheats ^= CF_ALLMAP;

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
            }

            // 'clev' change-level cheat
            if (!*consolecheat && cht_CheckCheat(&cheat_clev, ev->data2))
                idclev = true;

            if (cht_CheckCheat(&cheat_clev_xy, ev->data2))
            {
                char   buffer[3];
                char   lump[6];
                int    epsd;
                int    map;

                cht_GetParam(&cheat_clev_xy, buffer);

                if (gamemode == commercial)
                {
                    epsd = 1;
                    map = (buffer[0] - '0') * 10 + buffer[1] - '0';
                    M_snprintf(lump, sizeof(lump), "MAP%c%c", buffer[0], buffer[1]);
                }
                else
                {
                    epsd = buffer[0] - '0';
                    map = buffer[1] - '0';
                    M_snprintf(lump, sizeof(lump), "E%cM%c", buffer[0], buffer[1]);

                    if (chex && epsd != 1)
                        return false;
                }

                // Catch invalid maps.
                // [BH] simplified by checking if lump for map exists in WAD
                // [BH] only allow MAP01 to MAP09 when NERVE.WAD loaded
                if (W_CheckNumForName(lump) < 0 || (gamemission == pack_nerve && map > 9) || (BTSX && W_CheckMultipleLumps(lump) == 1))
                    idclev = false;
                else
                {
                    static char message[128];

                    S_StartSound(NULL, sfx_getpow);
                    C_Input("%s%c%c", cheat_clev_xy.sequence, buffer[0], buffer[1]);

                    if (BTSX)
                        M_snprintf(lump, sizeof(lump), "E%iM%c%c", (BTSXE1 ? 1 : (BTSXE2 ? 2 : 3)), buffer[0], buffer[1]);
                    else if (FREEDOOM && gamemode != commercial)
                        M_snprintf(lump, sizeof(lump), "C%cM%c", buffer[0], buffer[1]);

                    M_snprintf(message, sizeof(message), (M_StringCompare(lump, mapnum) ? s_STSTR_CLEVSAME : s_STSTR_CLEV), lump);

                    C_Output(message);
                    HU_SetPlayerMessage(message, false, false);

                    // [BH] always display message
                    viewplayer->message = message;
                    message_dontfuckwithme = true;

                    // [BH] delay map change by 1 second to allow message to be displayed
                    samelevel = (gameepisode == epsd && gamemap == map);
                    gameepisode = epsd;

                    if (gamemission == doom)
                    {
                        episode = gameepisode;
                        EpiDef.lastOn = episode - 1;
                        M_SaveCVARs();
                    }

                    gamemap = map;
                    idclevtics = MAPCHANGETICS;
                    drawdisk = true;
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
            }
        }

        if (cheatkey)
        {
            cheatkey = '\0';
            return true;
        }

        C_HideConsole();
    }

    return false;
}

static int ST_CalcPainOffset(void)
{
    int         newhealth = MIN(viewplayer->health, 100);
    static int  lastcalc;
    static int  health = -1;

    if (newhealth != health)
    {
        lastcalc = ST_FACESTRIDE * (((100 - newhealth) * ST_NUMPAINFACES) / 101);
        health = newhealth;
    }

    return lastcalc;
}

//
// This is a not-very-pretty routine which handles the face states and their timing.
// The precedence of expressions is: dead > evil grin > turned head > straight ahead
//
static void ST_UpdateFaceWidget(void)
{
    static int  priority;

    // [crispy] fix status bar face hysteresis
    int         painoffset = ST_CalcPainOffset();
    static int  faceindex;

    dboolean    invulnerable = ((viewplayer->cheats & CF_GODMODE) || viewplayer->powers[pw_invulnerability]);

    if (priority < 10)
    {
        // dead
        if (viewplayer->health <= 0)
        {
            priority = 9;
            painoffset = 0;
            faceindex = ST_DEADFACE;
            st_facecount = 1;
        }
    }

    if (priority < 9)
    {
        if (viewplayer->bonuscount)
        {
            // picking up bonus
            dboolean    doevilgrin = false;

            for (int i = 0; i < NUMWEAPONS; i++)
                // [BH] no evil grin when invulnerable
                if (oldweaponsowned[i] != viewplayer->weaponowned[i] && !invulnerable)
                {
                    doevilgrin = true;
                    oldweaponsowned[i] = viewplayer->weaponowned[i];
                }

            if (doevilgrin)
            {
                // evil grin if just picked up weapon
                priority = 8;
                st_facecount = ST_EVILGRINCOUNT;
                faceindex = ST_EVILGRINOFFSET;
            }
        }
    }

    if (priority < 8)
    {
        if (viewplayer->damagecount && viewplayer->attacker && viewplayer->attacker != viewplayer->mo)
        {
            // [BH] Fix <https://doomwiki.org/wiki/Ouch_face>.
            if (st_oldhealth - viewplayer->health > ST_MUCHPAIN)
            {
                priority = 8;   // [BH] keep ouch-face visible
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_OUCHOFFSET;
            }
            else
            {
                angle_t     badguyangle = R_PointToAngle(viewplayer->attacker->x, viewplayer->attacker->y);
                angle_t     diffang;

                if (badguyangle > viewangle)
                {
                    // whether right or left
                    diffang = badguyangle - viewangle;
                    faceindex = (diffang < ANG45 ? ST_RAMPAGEOFFSET : ST_TURNOFFSET + (diffang <= ANG180));
                }
                else
                {
                    // whether left or right
                    diffang = viewangle - badguyangle;
                    faceindex = (diffang < ANG45 ? ST_RAMPAGEOFFSET : ST_TURNOFFSET + (diffang > ANG180));
                }

                priority = 7;
                st_facecount = ST_TURNCOUNT;
            }
        }
    }

    if (priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (viewplayer->damagecount)
        {
            // [BH] Fix <https://doomwiki.org/wiki/Ouch_face>.
            if (st_oldhealth - viewplayer->health > ST_MUCHPAIN)
            {
                priority = 7;
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_OUCHOFFSET;
            }
            else
            {
                priority = 6;
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_RAMPAGEOFFSET;
            }
        }
    }

    if (priority < 6)
    {
        static int  lastattackdown = -1;

        // [BH] no rampage face when invulnerable
        if (viewplayer->attackdown && !invulnerable)
        {
            if (lastattackdown == -1)
                lastattackdown = ST_RAMPAGEDELAY;
            else if (!--lastattackdown)
            {
                priority = 5;
                faceindex = ST_RAMPAGEOFFSET;
                st_facecount = 1;
                lastattackdown = 1;
            }
        }
        else
            lastattackdown = -1;
    }

    if (priority < 5)
    {
        // invulnerability
        if (invulnerable)
        {
            priority = 4;
            painoffset = 0;
            faceindex = ST_GODFACE;
            st_facecount = 1;
        }
    }

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
        priority = 0;
        faceindex = M_Random() % 3;
        st_facecount = ST_STRAIGHTFACECOUNT;
    }

    st_facecount--;

    // [crispy] fix status bar face hysteresis
    st_faceindex = painoffset + faceindex;
}

static void ST_UpdateWidgets(void)
{
    static int      largeammo = 1994;   // means "n/a"
    weapontype_t    readyweapon = viewplayer->readyweapon;
    ammotype_t      ammotype = weaponinfo[readyweapon].ammotype;

    w_ready.num = (ammotype == am_noammo || viewplayer->health <= 0 ? &largeammo : &viewplayer->ammo[ammotype]);
    w_ready.data = readyweapon;

    // update keycard multiple widgets
    keyboxes[0] = (viewplayer->cards[it_blueskull] > 0 ? it_blueskull : (viewplayer->cards[it_bluecard] > 0 ? it_bluecard : -1));
    keyboxes[1] = (viewplayer->cards[it_yellowskull] > 0 ? it_yellowskull : (viewplayer->cards[it_yellowcard] > 0 ? it_yellowcard : -1));
    keyboxes[2] = (viewplayer->cards[it_redskull] > 0 ? it_redskull : (viewplayer->cards[it_redcard] > 0 ? it_redcard : -1));

    // refresh everything if this is him coming back to life
    ST_UpdateFaceWidget();
}

void ST_Ticker(void)
{
    if (!vid_widescreen)
    {
        if (!freeze && !paused && !menuactive && !consoleactive)
        {
            ST_UpdateWidgets();
            st_oldhealth = viewplayer->health;
        }
    }
    else if (r_hud && !r_althud)
    {
        if (!freeze && !paused && !menuactive && !consoleactive)
        {
            ST_UpdateFaceWidget();
            st_oldhealth = viewplayer->health;
        }
    }

    // [BH] action the IDCLEV cheat after a small delay to allow its player message to display
    if (idclevtics && !--idclevtics)
    {
        if (!samelevel)
            S_StopMusic();

        G_DeferredLoadLevel(gameskill, gameepisode, gamemap);
    }
}

static void ST_DoPaletteStuff(void)
{
    int palette = 0;

    if (viewplayer->powers[pw_strength]
        && (viewplayer->pendingweapon == wp_fist || (viewplayer->readyweapon == wp_fist && viewplayer->pendingweapon == wp_nochange))
        && viewplayer->health > 0 && r_berserkintensity)
    {
        int bonuscount = viewplayer->bonuscount;

        if (bonuscount)
            palette = STARTBONUSPALS + MIN((bonuscount + 7) >> 3, NUMBONUSPALS) - 1;
        else
            palette = MIN((viewplayer->damagecount >> 3) + r_berserkintensity + 3 * doom4vanilla, NUMREDPALS);
    }
    else
    {
        int damagecount = viewplayer->damagecount;

        if (damagecount && !(viewplayer->cheats & CF_GODMODE))
            palette = (chex ? RADIATIONPAL : STARTREDPALS + MIN((damagecount + 7) >> 3, NUMREDPALS - 1));
        else if (viewplayer->health > 0)
        {
            int bonuscount = viewplayer->bonuscount;

            if (bonuscount)
                palette = STARTBONUSPALS + MIN((bonuscount + 7) >> 3, NUMBONUSPALS) - 1;
            else if (viewplayer->powers[pw_ironfeet] > STARTFLASHING || (viewplayer->powers[pw_ironfeet] & 8))
                palette = RADIATIONPAL;
        }
    }

    if (palette != st_palette)
    {
        st_palette = palette;
        I_SetPalette(&PLAYPAL[st_palette * 768]);
    }
}

static void ST_DrawWidgets(dboolean refresh)
{
    STlib_UpdateBigNum(&w_ready);

    STlib_UpdateSmallNum(&w_ammo[0]);
    STlib_UpdateSmallNum(&w_ammo[1]);
    STlib_UpdateSmallNum(&w_ammo[2]);
    STlib_UpdateSmallNum(&w_ammo[3]);

    STlib_UpdateSmallNum(&w_maxammo[0]);
    STlib_UpdateSmallNum(&w_maxammo[1]);
    STlib_UpdateSmallNum(&w_maxammo[2]);
    STlib_UpdateSmallNum(&w_maxammo[3]);

    STlib_UpdatePercent(&w_health, refresh);
    STlib_UpdatePercent(&w_armor, refresh);

    st_shotguns = (viewplayer->weaponowned[wp_shotgun] || viewplayer->weaponowned[wp_supershotgun]);

    STlib_UpdateArmsIcon(&w_arms[0], refresh, 0);
    STlib_UpdateArmsIcon(&w_arms[1], refresh, 1);
    STlib_UpdateArmsIcon(&w_arms[2], refresh, 2);
    STlib_UpdateArmsIcon(&w_arms[3], refresh, 3);

    if (gamemode != shareware)
    {
        STlib_UpdateArmsIcon(&w_arms[4], refresh, 4);
        STlib_UpdateArmsIcon(&w_arms[5], refresh, 5);
    }

    if (facebackcolor != facebackcolor_none)
        V_FillRect(0, ST_FACEBACKX, ST_FACEBACKY, ST_FACEBACKWIDTH, ST_FACEBACKHEIGHT, nearestcolors[facebackcolor], false);

    STlib_UpdateMultIcon(&w_faces, refresh);

    STlib_UpdateMultIcon(&w_keyboxes[0], refresh);
    STlib_UpdateMultIcon(&w_keyboxes[1], refresh);
    STlib_UpdateMultIcon(&w_keyboxes[2], refresh);
}

void ST_DoRefresh(void)
{
    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_RefreshBackground();

    // and refresh all widgets
    ST_DrawWidgets(true);
}

static void ST_DiffDraw(void)
{
    // update all widgets
    ST_DrawWidgets(false);
}

void ST_Drawer(dboolean fullscreen, dboolean refresh)
{
    // Do red-/gold-shifts from damage/items
    ST_DoPaletteStuff();

    if (vid_widescreen || (menuactive && !consoleactive) || inhelpscreens)
        return;

    st_statusbaron = (!fullscreen || automapactive);
    st_firsttime = (st_firsttime || refresh);

    // If just after ST_Start(), refresh all
    if (st_firsttime)
        ST_DoRefresh();
    // Otherwise, update as little as possible
    else
        ST_DiffDraw();
}

typedef void (*load_callback_t)(char *lumpname, patch_t **variable);

static void ST_LoadUnloadGraphics(load_callback_t callback)
{
    int     facenum = 0;
    char    namebuf[9];

    // Load the numbers, tall and short
    for (int i = 0; i < 10; i++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "STTNUM%i", i);
        callback(namebuf, &tallnum[i]);
        M_snprintf(namebuf, sizeof(namebuf), "STYSNUM%i", i);
        callback(namebuf, &shortnum[i]);
    }

    callback("STTPRCNT", &tallpercent);
    emptytallpercent = V_IsEmptyPatch(tallpercent);
    tallpercentwidth = (emptytallpercent ? 0 : SHORT(tallpercent->width));

    // key cards
    for (int i = 0; i < 6; i++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "STKEYS%i", i);
        callback(namebuf, &keys[i]);
    }

    // arms background
    callback("STARMS", &armsbg);

    armsbg->leftoffset = 0;
    armsbg->topoffset = 0;

    // arms ownership widgets
    // [BH] now manually drawn
    for (int i = 0; i < 6; i++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "STGNUM%i", i + 2);

        // gray #
        callback(namebuf, &arms[i][0]);

        // yellow #
        arms[i][1] = shortnum[i + 2];
    }

    // status bar background bits
    callback("STBAR", &sbar);
    callback("STBAR2", &sbar2); // [BH] double resolution

    sbar->leftoffset = 0;
    sbar->topoffset = 0;
    sbar2->leftoffset = 0;
    sbar2->topoffset = 0;

    // face states
    for (int i = 0; i < ST_NUMPAINFACES; i++)
    {
        for (int j = 0; j < ST_NUMSTRAIGHTFACES; j++)
        {
            M_snprintf(namebuf, sizeof(namebuf), "STFST%i%i", i, j);
            callback(namebuf, &faces[facenum++]);
        }

        M_snprintf(namebuf, sizeof(namebuf), "STFTR%i0", i);          // turn right
        callback(namebuf, &faces[facenum++]);
        M_snprintf(namebuf, sizeof(namebuf), "STFTL%i0", i);          // turn left
        callback(namebuf, &faces[facenum++]);
        M_snprintf(namebuf, sizeof(namebuf), "STFOUCH%i", i);         // ouch!
        callback(namebuf, &faces[facenum++]);
        M_snprintf(namebuf, sizeof(namebuf), "STFEVL%i", i);          // evil grin ;)
        callback(namebuf, &faces[facenum++]);
        M_snprintf(namebuf, sizeof(namebuf), "STFKILL%i", i);         // pissed off
        callback(namebuf, &faces[facenum++]);
    }

    callback("STFGOD0", &faces[facenum++]);
    callback("STFDEAD0", &faces[facenum]);

    // back screen
    callback((gamemode == commercial ? "GRNROCK" : "FLOOR7_2"), &grnrock);
    callback("BRDR_T", &brdr_t);
    callback("BRDR_B", &brdr_b);
    callback("BRDR_L", &brdr_l);
    callback("BRDR_R", &brdr_r);
    callback("BRDR_TL", &brdr_tl);
    callback("BRDR_TR", &brdr_tr);
    callback("BRDR_BL", &brdr_bl);
    callback("BRDR_BR", &brdr_br);

    // [BH] fix display of viewborder for wads that have these patches without offsets
    brdr_t->leftoffset = 0;
    brdr_t->topoffset = -5;
    brdr_b->leftoffset = 0;
    brdr_b->topoffset = 0;
    brdr_l->leftoffset = -5;
    brdr_l->topoffset = 0;
    brdr_r->leftoffset = 0;
    brdr_r->topoffset = 0;
    brdr_tl->leftoffset = -5;
    brdr_tl->topoffset = -5;
    brdr_tr->leftoffset = 0;
    brdr_tr->topoffset = -5;
    brdr_bl->leftoffset = -5;
    brdr_bl->topoffset = 0;
    brdr_br->leftoffset = 0;
    brdr_br->topoffset = 0;
}

static void ST_LoadCallback(char *lumpname, patch_t **variable)
{
    if (M_StringCompare(lumpname, "STARMS") || M_StringCompare(lumpname, "STBAR") || M_StringCompare(lumpname, "STFGOD0"))
        *variable = ((FREEDOOM && !modifiedgame) || hacx ? W_CacheLastLumpName(lumpname) : W_CacheLumpName(lumpname));
    else
        *variable = W_CacheLumpName(lumpname);
}

static void ST_InitData(void)
{
    st_firsttime = true;
    st_statusbaron = true;
    st_faceindex = 0;
    st_palette = -1;
    st_oldhealth = -1;

    for (int i = 0; i < NUMWEAPONS; i++)
        oldweaponsowned[i] = viewplayer->weaponowned[i];

    for (int i = 0; i < 3; i++)
        keyboxes[i] = -1;
}

static void ST_CreateWidgets(void)
{
    // ready weapon ammo
    STlib_InitNum(&w_ready, ST_AMMOX, ST_AMMOY + (STBAR != 2 && !BTSX), tallnum,
        &viewplayer->ammo[weaponinfo[viewplayer->readyweapon].ammotype], ST_AMMOWIDTH);

    // the last weapon type
    w_ready.data = viewplayer->readyweapon;

    // health percentage
    STlib_InitPercent(&w_health, ST_HEALTHX, ST_HEALTHY + (STBAR != 2 && !BTSX), tallnum, &viewplayer->health, tallpercent);

    // weapons owned
    STlib_InitMultIcon(&w_arms[0], ST_ARMSX, ST_ARMSY, arms[0], &viewplayer->weaponowned[1]);
    STlib_InitMultIcon(&w_arms[1], ST_ARMSX + ST_ARMSXSPACE, ST_ARMSY, arms[1], &st_shotguns);
    STlib_InitMultIcon(&w_arms[2], ST_ARMSX + 2 * ST_ARMSXSPACE, ST_ARMSY, arms[2], &viewplayer->weaponowned[3]);
    STlib_InitMultIcon(&w_arms[3], ST_ARMSX, ST_ARMSY + ST_ARMSYSPACE, arms[3], &viewplayer->weaponowned[4]);

    if (gamemode != shareware)
    {
        STlib_InitMultIcon(&w_arms[4], ST_ARMSX + ST_ARMSXSPACE, ST_ARMSY + ST_ARMSYSPACE, arms[4], &viewplayer->weaponowned[5]);
        STlib_InitMultIcon(&w_arms[5], ST_ARMSX + 2 * ST_ARMSXSPACE, ST_ARMSY + ST_ARMSYSPACE, arms[5], &viewplayer->weaponowned[6]);
    }

    // faces
    STlib_InitMultIcon(&w_faces, ST_FACESX, ST_FACESY, faces, &st_faceindex);

    // armor percentage
    STlib_InitPercent(&w_armor, ST_ARMORX, ST_ARMORY + (STBAR != 2 && !BTSX), tallnum, &viewplayer->armorpoints, tallpercent);

    // keyboxes 0-2
    STlib_InitMultIcon(&w_keyboxes[0], ST_KEY0X + (STBAR >= 3), ST_KEY0Y, keys, &keyboxes[0]);
    STlib_InitMultIcon(&w_keyboxes[1], ST_KEY1X + (STBAR >= 3), ST_KEY1Y, keys, &keyboxes[1]);
    STlib_InitMultIcon(&w_keyboxes[2], ST_KEY2X + (STBAR >= 3), ST_KEY2Y, keys, &keyboxes[2]);

    // ammo count (all four kinds)
    STlib_InitNum(&w_ammo[am_clip], ST_AMMO0X, ST_AMMO0Y, shortnum, &viewplayer->ammo[am_clip], ST_AMMO0WIDTH);
    STlib_InitNum(&w_ammo[am_shell], ST_AMMO1X, ST_AMMO1Y, shortnum, &viewplayer->ammo[am_shell], ST_AMMO1WIDTH);
    STlib_InitNum(&w_ammo[am_cell], ST_AMMO2X, ST_AMMO2Y, shortnum, &viewplayer->ammo[am_cell], ST_AMMO2WIDTH);
    STlib_InitNum(&w_ammo[am_misl], ST_AMMO3X, ST_AMMO3Y, shortnum, &viewplayer->ammo[am_misl], ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_InitNum(&w_maxammo[am_clip], ST_MAXAMMO0X, ST_MAXAMMO0Y, shortnum, &viewplayer->maxammo[am_clip], ST_MAXAMMO0WIDTH);
    STlib_InitNum(&w_maxammo[am_shell], ST_MAXAMMO1X, ST_MAXAMMO1Y, shortnum, &viewplayer->maxammo[am_shell], ST_MAXAMMO1WIDTH);
    STlib_InitNum(&w_maxammo[am_cell], ST_MAXAMMO2X, ST_MAXAMMO2Y, shortnum, &viewplayer->maxammo[am_cell], ST_MAXAMMO2WIDTH);
    STlib_InitNum(&w_maxammo[am_misl], ST_MAXAMMO3X, ST_MAXAMMO3Y, shortnum, &viewplayer->maxammo[am_misl], ST_MAXAMMO3WIDTH);
}

void ST_Start(void)
{
    ST_InitData();
    ST_CreateWidgets();
}

void ST_Init(void)
{
    ST_LoadUnloadGraphics(&ST_LoadCallback);

    screens[4] = malloc(ST_WIDTH * SBARHEIGHT);

    // [BH] fix evil grin being displayed when picking up first item after
    // loading save game or entering IDFA/IDKFA cheat
    for (int i = 0; i < NUMWEAPONS; i++)
        oldweaponsowned[i] = false;

    // [BH] no plasma cells in shareware
    if (gamemode == shareware)
        maxammo[am_cell] = 0;

#if SCREENSCALE == 1
    usesmallnums = false;
#else
    usesmallnums = ((!STYSNUM0 && STBAR == 2) || gamemode == shareware);
#endif

    STLib_Init();
    ST_InitCheats();
}
