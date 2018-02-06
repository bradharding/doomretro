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

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_swap.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "s_sound.h"
#include "st_lib.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS        1
#define STARTBONUSPALS      9
#define NUMREDPALS          8
#define NUMBONUSPALS        4

// Radiation suit, green shift.
#define RADIATIONPAL        13

// Location of status bar
#define ST_X                0

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
#define ST_ARMSBGY          168
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

// lump number for PLAYPAL
static int                  lu_palette;

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
static patch_t              *armsbg2;

// weapon ownership patches
static patch_t              *arms[6][2];
static int                  armsnum;

// ready-weapon widget
static st_number_t          w_ready;

// health widget
static st_percent_t         w_health;

// arms background
static st_binicon_t         w_armsbg;
static st_binicon_t         w_armsbg2;

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

// count until face changes
int                         st_facecount;

// current face index, used by w_faces
int                         st_faceindex;

// holds key-type for each key box on bar
static int                  keyboxes[3];

// a random number per tick
static int                  st_randomnumber;

int                         oldhealth = 100;

dboolean                    idclev;

int                         idclevtics;

dboolean                    idmus;

dboolean                    samelevel;

int                         facebackcolor = facebackcolor_default;
int                         r_berserkintensity = r_berserkintensity_default;

unsigned int                stat_cheated;

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
    return (key == keyboardright || key == keyboardleft
        || key == keyboardforward || key == keyboardforward2
        || key == keyboardback || key == keyboardback2
        || key == keyboardstrafeleft || key == keyboardstraferight);
}

static void ST_initCheats(void)
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
#define IDMUS_MAX   50

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
    /* 49 */ { NONE,        NONE,        NONE,        mus_e1m9,   NONE,        NONE       }
};

//
// STATUS BAR CODE
//
static int ST_calcPainOffset(void);

static void ST_refreshBackground(void)
{
    if (st_statusbaron)
    {
        if (STBAR >= 3 || r_detail == r_detail_low)
            V_DrawPatch(ST_X, 0, 4, sbar);
        else
            V_DrawBigPatch(ST_X, 0, 4, sbar2);

        V_CopyRect(ST_X, 0, 4, ST_WIDTH, SBARHEIGHT, ST_X, ST_Y, 0);
    }
}

//
// ST_AutomapEvent
//
// haleyjd 09/29/04: Replaces the weird hack Dave Taylor put into
// ST_Responder to toggle the status bar when the automap is toggled.
// The automap now calls this function instead of sending fake events
// to ST_Responder, allowing that function to be minimized.
//
void ST_AutomapEvent(int type)
{
    if (type == AM_MSGENTERED)
        st_firsttime = true;
}

extern char     cheatkey;
extern int      episode;
extern menu_t   EpiDef;

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
                // [BH] if player is dead, resurrect them first
                if (viewplayer->health <= 0)
                    P_ResurrectPlayer(initial_health);

                viewplayer->cheats ^= CF_GODMODE;

                if (viewplayer->cheats & CF_GODMODE)
                {
                    // [BH] remember player's current health,
                    //  and only set to 100% if less than 100%
                    oldhealth = viewplayer->health;
                    P_GiveBody(god_health, false);

                    C_Input(cheat_god.sequence);

                    HU_PlayerMessage(s_STSTR_DQDON, false);

                    // [BH] always display message
                    if (!consoleactive)
                        message_dontfuckwithme = true;

                    // [BH] play sound
                    S_StartSound(NULL, sfx_getpow);

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
                else
                {
                    C_Input(cheat_god.sequence);

                    HU_PlayerMessage(s_STSTR_DQDOFF, false);

                    // [BH] always display message
                    if (!consoleactive)
                        message_dontfuckwithme = true;

                    // [BH] restore player's health
                    viewplayer->health = oldhealth;
                    viewplayer->mo->health = oldhealth;

                    if (!oldhealth)
                    {
                        // [BH] The player originally used this cheat to
                        // resurrect themselves, and now they have the audacity
                        // to disable it. Kill them!
                        viewplayer->attacker = NULL;
                        P_KillMobj(viewplayer->mo, NULL, viewplayer->mo);
                    }
                    else
                    {
                        // [BH] play sound
                        S_StartSound(NULL, sfx_getpow);
                    }
                }
            }

            // 'fa' cheat for killer fucking arsenal
            else if (cht_CheckCheat(&cheat_ammonokey, ev->data2) && gameskill != sk_nightmare
                     // [BH] can only enter cheat while player is alive
                     && viewplayer->health > 0)
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

                ammogiven = P_GiveFullAmmo(false);

                // [BH] show evil grin if player was given any new weapons
                if (weaponsgiven && !(viewplayer->cheats & CF_GODMODE) && !viewplayer->powers[pw_invulnerability])
                {
                    st_facecount = ST_EVILGRINCOUNT;
                    st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
                }

                // [BH] only acknowledge cheat if player was given something
                if (ammogiven || armorgiven || berserkgiven || weaponsgiven)
                {
                    // [BH] flash screen
                    P_AddBonus();

                    C_Input(cheat_ammonokey.sequence);

                    HU_PlayerMessage(s_STSTR_FAADDED, false);

                    // [BH] always display message
                    if (!consoleactive)
                        message_dontfuckwithme = true;

                    // [BH] play sound
                    S_StartSound(NULL, sfx_getpow);

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

                ammogiven = P_GiveFullAmmo(false);

                // [BH] only give the player the keycards or skull keys from the
                //  current level, and note if any keys given
                keysgiven = P_GiveAllCards();

                // [BH] show evil grin if player was given any new weapons
                if (weaponsgiven && !(viewplayer->cheats & CF_GODMODE) && !viewplayer->powers[pw_invulnerability])
                {
                    st_facecount = ST_EVILGRINCOUNT;
                    st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
                }

                // [BH] only acknowledge cheat if player was given something
                if (ammogiven || armorgiven || berserkgiven || weaponsgiven || keysgiven)
                {
                    // [BH] flash screen
                    P_AddBonus();

                    C_Input(cheat_ammo.sequence);

                    HU_PlayerMessage(s_STSTR_KFAADDED, false);

                    // [BH] always display message
                    if (!consoleactive)
                        message_dontfuckwithme = true;

                    // [BH] play sound
                    S_StartSound(NULL, sfx_getpow);

                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
            }

            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus_xy, ev->data2)
                     // [BH] can only enter cheat if music is playing
                     && !nomusic && musicVolume)
            {
                char   buf[3];

                // [BH] only display message if parameter is valid
                cht_GetParam(&cheat_mus_xy, buf);

                // [BH] rewritten to use mus[] LUT
                // [BH] fix crash if IDMUS0y and IDMUSx0 entered in DOOM,
                //  IDMUS21 to IDMUS39 entered in shareware, and IDMUS00
                //  entered in DOOM II
                if (buf[0] >= '0' && buf[0] <= '9' && buf[1] >= '0' && buf[1] <= '9')
                {
                    int musnum = (buf[0] - '0') * 10 + (buf[1] - '0');

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

                            S_ChangeMusic(musnum, 1, true, false);

                            C_Input("%s%c%c", cheat_mus_xy.sequence, buf[0], buf[1]);

                            M_snprintf(msg, sizeof(msg), s_STSTR_MUS, uppercase(S_music[musnum].name));
                            HU_PlayerMessage(msg, false);

                            // [BH] always display message
                            if (!consoleactive)
                                message_dontfuckwithme = true;

                            // [BH] play sound
                            S_StartSound(NULL, sfx_getpow);

                            stat_cheated = SafeAdd(stat_cheated, 1);
                            viewplayer->cheated++;
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
                viewplayer->cheats ^= CF_NOCLIP;

                C_Input(gamemode == commercial ? cheat_commercial_noclip.sequence : cheat_noclip.sequence);

                HU_PlayerMessage(((viewplayer->cheats & CF_NOCLIP) ? s_STSTR_NCON : s_STSTR_NCOFF), false);

                // [BH] always display message
                if (!consoleactive)
                    message_dontfuckwithme = true;

                // [BH] play sound
                S_StartSound(NULL, sfx_getpow);

                if (viewplayer->cheats & CF_NOCLIP)
                {
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    viewplayer->cheated++;
                }
            }

            // 'behold?' power-up cheats
            for (int i = 0; i < 6; i++)
            {
                if (cht_CheckCheat(&cheat_powerup[i], ev->data2) && gameskill != sk_nightmare
                    // [BH] can only enter cheat while player is alive
                    && viewplayer->health > 0)
                {
                    if ((i != pw_strength && viewplayer->powers[i] >= 0 && viewplayer->powers[i] <= STARTFLASHING)
                        || (i == pw_strength && !viewplayer->powers[i]))
                    {
                        P_GivePower(i);

                        // [BH] set to -1 so power-up won't run out, but can
                        //  still be toggled off using cheat
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
                                viewplayer->pendingweapon = wp_fist;

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

                        C_Input(cheat_powerup[i].sequence);

                        HU_PlayerMessage((!M_StringCompare(s_STSTR_BEHOLDX, STSTR_BEHOLDX) ?
                            s_STSTR_BEHOLDX : s_STSTR_BEHOLDON), false);

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
                                viewplayer->pendingweapon = wp_chainsaw;
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
                                    viewplayer->pendingweapon = viewplayer->weaponbeforechoppers;

                                viewplayer->weaponowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                                oldweaponsowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                            }

                            // [BH] start flashing palette to indicate power-up about to run out
                            viewplayer->powers[i] = STARTFLASHING * (i != pw_allmap);
                        }

                        C_Input(cheat_powerup[i].sequence);

                        HU_PlayerMessage((!M_StringCompare(s_STSTR_BEHOLDX, STSTR_BEHOLDX) ?
                            s_STSTR_BEHOLDX : s_STSTR_BEHOLDOFF), false);
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

                    for (int i = 0; i < 7; i++)
                        cheat_powerup[i].chars_read = 0;

                    cheat_choppers.chars_read = 0;
                    cheat_clev.chars_read = 0;
                    cheat_clev_xy.chars_read = 0;
                    cheat_clev_xy.param_chars_read = 0;
                    cheat_mypos.chars_read = 0;
                    cheat_amap.chars_read = 0;
                    cheat_buddha.chars_read = 0;
                    cheatkey = '\0';

                    // [BH] always display message
                    if (!consoleactive)
                        message_dontfuckwithme = true;

                    // [BH] play sound
                    S_StartSound(NULL, sfx_getpow);

                    idbehold = false;
                    return true;
                }
            }

            // 'behold' power-up menu
            if (cht_CheckCheat(&cheat_powerup[6], ev->data2) && gameskill != sk_nightmare
                // [BH] can only enter cheat while player is alive
                && viewplayer->health > 0)
            {
                // [BH] message stays on screen until parameter entered or another key
                //  pressed to cancel. Code is in hu_stuff.c.
                idbehold = true;
            }

            // 'choppers' invulnerability & chainsaw
            else if (cht_CheckCheat(&cheat_choppers, ev->data2) && gameskill != sk_nightmare
                     // [BH] can only enter cheat while player is alive
                     && viewplayer->health > 0)
            {
                if (!(viewplayer->cheats & CF_CHOPPERS))
                {
                    // [BH] flash screen
                    P_AddBonus();

                    // [BH] note if has chainsaw and/or invulnerability already
                    viewplayer->invulnbeforechoppers = viewplayer->powers[pw_invulnerability];
                    viewplayer->chainsawbeforechoppers = viewplayer->weaponowned[wp_chainsaw];

                    // [BH] note weapon before switching to chainsaw
                    viewplayer->weaponbeforechoppers = viewplayer->readyweapon;

                    if (viewplayer->weaponbeforechoppers != wp_chainsaw)
                    {
                        viewplayer->weaponowned[wp_chainsaw] = true;
                        oldweaponsowned[wp_chainsaw] = true;
                        viewplayer->pendingweapon = wp_chainsaw;
                    }

                    viewplayer->weaponowned[wp_chainsaw] = true;

                    // [BH] fixed bug where invulnerability was never given, and now
                    //  needs to be toggled off with cheat or switch from chainsaw
                    P_GivePower(pw_invulnerability);
                    viewplayer->powers[pw_invulnerability] = -1;

                    C_Input(cheat_choppers.sequence);

                    HU_PlayerMessage(s_STSTR_CHOPPERS, false);

                    // [BH] always display message
                    if (!consoleactive)
                        message_dontfuckwithme = true;

                    // [BH] play sound
                    S_StartSound(NULL, sfx_getpow);

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
                        viewplayer->pendingweapon = viewplayer->weaponbeforechoppers;

                    viewplayer->weaponowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
                    oldweaponsowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;

                    C_Input(cheat_choppers.sequence);
                }
            }

            // 'mypos' for player position
            else if (cht_CheckCheat(&cheat_mypos, ev->data2))
            {
                C_Input(cheat_mypos.sequence);

                // [BH] message stays on screen until toggled off again using
                //  cheat. Code is in hu_stuff.c.
                viewplayer->cheats ^= CF_MYPOS;

                if (!(viewplayer->cheats & CF_MYPOS))
                {
                    message_clearable = true;
                    HU_ClearMessages();
                }

                // [BH] play sound
                S_StartSound(NULL, sfx_getpow);

                stat_cheated = SafeAdd(stat_cheated, 1);
                viewplayer->cheated++;
            }

            else if (cht_CheckCheat(&cheat_buddha, ev->data2) && gameskill != sk_nightmare
                && viewplayer->health > 0)
            {
                viewplayer->cheats ^= CF_BUDDHA;

                C_Input(cheat_buddha.sequence);

                if (viewplayer->cheats & CF_BUDDHA)
                {
                    HU_PlayerMessage(s_STSTR_BUDDHA, false);

                    // [BH] always display message
                    if (!consoleactive)
                        message_dontfuckwithme = true;
                }

                // [BH] play sound
                S_StartSound(NULL, sfx_getpow);

                stat_cheated = SafeAdd(stat_cheated, 1);
                viewplayer->cheated++;
            }

            else if ((automapactive || mapwindow) && cht_CheckCheat(&cheat_amap, ev->data2))
            {
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

                S_StartSound(NULL, sfx_getpow);
            }
        }

        // 'clev' change-level cheat
        if (!menuactive && !paused)
        {
            if (!consolecheat[0] && cht_CheckCheat(&cheat_clev, ev->data2))
                idclev = true;

            if (cht_CheckCheat(&cheat_clev_xy, ev->data2))
            {
                char   buf[3];
                char   lump[6];
                int    epsd;
                int    map;

                cht_GetParam(&cheat_clev_xy, buf);

                if (gamemode == commercial)
                {
                    epsd = 1;
                    map = (buf[0] - '0') * 10 + buf[1] - '0';
                    M_snprintf(lump, sizeof(lump), "MAP%c%c", buf[0], buf[1]);
                }
                else
                {
                    epsd = buf[0] - '0';
                    map = buf[1] - '0';
                    M_snprintf(lump, sizeof(lump), "E%cM%c", buf[0], buf[1]);
                }

                if (chex)
                    epsd = 1;

                // Catch invalid maps.
                // [BH] simplified by checking if lump for map exists in WAD
                // [BH] only allow MAP01 to MAP09 when NERVE.WAD loaded
                if (W_CheckNumForName(lump) < 0 || (gamemission == pack_nerve && map > 9)
                    || (BTSX && W_CheckMultipleLumps(lump) == 1))
                    idclev = false;
                else
                {
                    static char message[128];

                    C_Input("%s%c%c", cheat_clev_xy.sequence, buf[0], buf[1]);

                    if (BTSX)
                        M_snprintf(lump, sizeof(lump), "E%iM%c%c", (BTSXE1 ? 1 : 2), buf[0], buf[1]);
                    else if (FREEDOOM && gamemode != commercial)
                        M_snprintf(lump, sizeof(lump), "C%cM%c", buf[0], buf[1]);

                    if (epsd == gameepisode && map == gamemap)
                        M_snprintf(message, sizeof(message), s_STSTR_CLEVSAME, lump);
                    else
                        M_snprintf(message, sizeof(message), s_STSTR_CLEV, lump);

                    HU_PlayerMessage(message, false);

                    // [BH] always display message
                    viewplayer->message = message;
                    message_dontfuckwithme = true;

                    // [BH] play sound
                    S_StartSound(NULL, sfx_getpow);

                    // [BH] delay map change by 1 second to allow message to be displayed
                    samelevel = (gameepisode == epsd && gamemap == map);
                    gameepisode = epsd;

                    if (gamemission == doom && epsd <= 4)
                    {
                        episode = gameepisode;
                        EpiDef.lastOn = episode - 1;
                        M_SaveCVARs();
                    }

                    gamemap = map;
                    idclevtics = MAPCHANGETICS;
                    drawdisk = true;
                    C_HideConsole();
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
    }

    return false;
}

static int ST_calcPainOffset(void)
{
    int         health = MIN(viewplayer->health, 100);
    static int  lastcalc;
    static int  oldhealth = -1;

    if (health != oldhealth)
    {
        lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
        oldhealth = health;
    }

    return lastcalc;
}

//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
static void ST_updateFaceWidget(void)
{
    static int  priority;

    // [crispy] fix status bar face hysteresis
    int         painoffset = ST_calcPainOffset();
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
        if (viewplayer->damagecount && viewplayer->attacker)
        {
            // [BH] fix ouch-face when damage > 20
            if (st_oldhealth - viewplayer->health > ST_MUCHPAIN)
            {
                priority = 8;   // [BH] keep ouch-face visible
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_OUCHOFFSET;
            }
            else
            {
                angle_t     badguyangle = R_PointToAngle2(viewx, viewy, viewplayer->attacker->x, viewplayer->attacker->y);
                angle_t     diffang;
                dboolean    turn;

                if (badguyangle > viewangle)
                {
                    // whether right or left
                    diffang = badguyangle - viewangle;
                    turn = (diffang > ANG180);
                }
                else
                {
                    // whether left or right
                    diffang = viewangle - badguyangle;
                    turn = (diffang <= ANG180);
                }

                priority = 7;
                st_facecount = ST_TURNCOUNT;
                faceindex = (diffang < ANG45 ? ST_RAMPAGEOFFSET : ST_TURNOFFSET + !turn);
            }
        }
    }

    if (priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (viewplayer->damagecount)
        {
            // [BH] fix ouch-face when damage > 20
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
        faceindex = st_randomnumber % 3;
        st_facecount = ST_STRAIGHTFACECOUNT;
    }

    st_facecount--;

    // [crispy] fix status bar face hysteresis
    st_faceindex = painoffset + faceindex;
}

static void ST_updateWidgets(void)
{
    static int  largeammo = 1994;   // means "n/a"
    ammotype_t  ammotype = weaponinfo[viewplayer->readyweapon].ammotype;

    w_ready.num = (ammotype == am_noammo || viewplayer->health <= 0 ? &largeammo : &viewplayer->ammo[ammotype]);
    w_ready.data = viewplayer->readyweapon;

    // update keycard multiple widgets
    for (int i = 0; i < 3; i++)
    {
        keyboxes[i] = (viewplayer->cards[i] > 0 ? i : -1);

        if (viewplayer->cards[i + 3] > 0 && viewplayer->cards[i + 3] < viewplayer->cards[i])
            keyboxes[i] = i + 3;
    }

    // refresh everything if this is him coming back to life
    // [BH] but only if not paused and no menu
    if (!paused && !menuactive && !consoleactive)
        ST_updateFaceWidget();
}

void ST_Ticker(void)
{
    st_randomnumber = M_Random();

    if (!vid_widescreen)
    {
        ST_updateWidgets();
        st_oldhealth = viewplayer->health;
    }
    else if (r_hud && !r_althud && !paused && !menuactive && !consoleactive)
    {
        ST_updateFaceWidget();
        st_oldhealth = viewplayer->health;
    }

    // [BH] action the IDCLEV cheat after a small delay to allow its player message to display
    if (idclevtics)
        if (!--idclevtics)
        {
            if (!samelevel)
                S_StopMusic();

            G_DeferredLoadLevel(gameskill, gameepisode, gamemap);
        }
}

int st_palette = 0;

static void ST_doPaletteStuff(void)
{
    int palette = 0;
    int count = viewplayer->damagecount;

    if (viewplayer->powers[pw_strength] && (viewplayer->pendingweapon == wp_fist || (viewplayer->readyweapon == wp_fist
        && viewplayer->pendingweapon == wp_nochange)) && viewplayer->health > 0 && r_berserkintensity)
    {
        if (viewplayer->bonuscount)
            palette = STARTBONUSPALS - 1 + MIN((viewplayer->bonuscount + 7) >> 3, NUMBONUSPALS);
        else
            palette = MIN((count >> 3) + r_berserkintensity, NUMREDPALS);
    }
    else if (count)
        palette = STARTREDPALS + MIN((count + 7) >> 3, NUMREDPALS - 1);
    else if (viewplayer->health > 0)
    {
        if (viewplayer->bonuscount)
            palette = STARTBONUSPALS - 1 + MIN((viewplayer->bonuscount + 7) >> 3, NUMBONUSPALS);
        else if (viewplayer->powers[pw_ironfeet] > STARTFLASHING || (viewplayer->powers[pw_ironfeet] & 8))
            palette = RADIATIONPAL;
    }

    // [BH] show green instead of red palette in Chex Quest
    if (chex && palette >= STARTREDPALS && palette < STARTREDPALS + NUMREDPALS)
        palette = RADIATIONPAL;

    if (palette != st_palette)
    {
        st_palette = palette;
        I_SetPalette((byte *)W_CacheLumpNum(lu_palette) + palette * 768);
    }
}

static void ST_drawWidgets(dboolean refresh)
{
    STlib_updateBigNum(&w_ready);

    for (int i = 0; i < 4; i++)
    {
        STlib_updateSmallNum(&w_ammo[i]);
        STlib_updateSmallNum(&w_maxammo[i]);
    }

    STlib_updatePercent(&w_health, refresh);
    STlib_updatePercent(&w_armor, refresh);

    if (STBAR >= 3 || r_detail == r_detail_low)
        STlib_updateBinIcon(&w_armsbg, refresh);
    else
        STlib_updateBigBinIcon(&w_armsbg2, refresh);

    // [BH] manually draw arms numbers
    //  changes:
    //    arms 3 highlighted when player has super shotgun but no shotgun
    //    arms 6 and 7 not visible in shareware
    for (int i = 0; i < armsnum; i++)
        STlib_updateArmsIcon(&w_arms[i], refresh, i);

    if (facebackcolor != facebackcolor_default)
        V_FillRect(0, ST_FACEBACKX, ST_FACEBACKY, ST_FACEBACKWIDTH, ST_FACEBACKHEIGHT, facebackcolor);

    STlib_updateMultIcon(&w_faces, refresh);

    for (int i = 0; i < 3; i++)
        STlib_updateMultIcon(&w_keyboxes[i], refresh);
}

void ST_doRefresh(void)
{
    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground();

    // and refresh all widgets
    ST_drawWidgets(true);
}

static void ST_diffDraw(void)
{
    // update all widgets
    ST_drawWidgets(false);
}

void ST_Drawer(dboolean fullscreen, dboolean refresh)
{
    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    if (vid_widescreen)
        return;

    st_statusbaron = (!fullscreen || automapactive);
    st_firsttime = (st_firsttime || refresh);

    // If just after ST_Start(), refresh all
    if (st_firsttime)
        ST_doRefresh();
    // Otherwise, update as little as possible
    else
        ST_diffDraw();
}

typedef void (*load_callback_t)(char *lumpname, patch_t **variable);

static void ST_loadUnloadGraphics(load_callback_t callback)
{
    int     facenum;
    char    namebuf[9];

    // Load the numbers, tall and short
    for (int i = 0; i < 10; i++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "STTNUM%i", i);
        callback(namebuf, &tallnum[i]);
        M_snprintf(namebuf, sizeof(namebuf), "STYSNUM%i", i);
        callback(namebuf, &shortnum[i]);
    }

    // Load percent key.
    callback("STTPRCNT", &tallpercent);
    emptytallpercent = V_EmptyPatch(tallpercent);
    tallpercentwidth = (emptytallpercent ? 0 : SHORT(tallpercent->width));

    // key cards
    for (int i = 0; i < NUMCARDS; i++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "STKEYS%i", i);
        callback(namebuf, &keys[i]);
    }

    // arms background
    callback("STARMS", &armsbg);
    callback("STARMS2", &armsbg2);

    armsbg->leftoffset = 0;
    armsbg->topoffset = 0;
    armsbg2->leftoffset = 0;
    armsbg2->topoffset = 0;

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
    facenum = 0;

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
    callback("STFDEAD0", &faces[facenum++]);

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

static void ST_loadCallback(char *lumpname, patch_t **variable)
{
    if (M_StringCompare(lumpname, "STARMS"))
        *variable = (FREEDOOM || hacx ? W_CacheLumpName2("STARMS") : W_CacheLumpName("STARMS"));
    else if (M_StringCompare(lumpname, "STBAR"))
        *variable = (FREEDOOM || hacx ? W_CacheLumpName2("STBAR") : W_CacheLumpName("STBAR"));
    else
        *variable = W_CacheLumpName(lumpname);
}

static void ST_loadGraphics(void)
{
    ST_loadUnloadGraphics(ST_loadCallback);
}

static void ST_loadData(void)
{
    lu_palette = W_GetNumForName("PLAYPAL");
    ST_loadGraphics();
}

static void ST_initData(void)
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

static void ST_createWidgets(void)
{
    // ready weapon ammo
    STlib_initNum(&w_ready, ST_AMMOX, ST_AMMOY + (STBAR != 2 && !BTSX), tallnum,
        &viewplayer->ammo[weaponinfo[viewplayer->readyweapon].ammotype], &st_statusbaron, ST_AMMOWIDTH);

    // the last weapon type
    w_ready.data = viewplayer->readyweapon;

    // health percentage
    STlib_initPercent(&w_health, ST_HEALTHX, ST_HEALTHY + (STBAR != 2 && !BTSX), tallnum, &viewplayer->health,
        &st_statusbaron, tallpercent);

    // arms background
    STlib_initBinIcon(&w_armsbg, ST_ARMSBGX + hacx * 4, ST_ARMSBGY, armsbg, &st_statusbaron, &st_statusbaron);
    STlib_initBinIcon(&w_armsbg2, ST_ARMSBGX * 2, ST_ARMSBGY * 2, armsbg2, &st_statusbaron, &st_statusbaron);

    // weapons owned
    armsnum = (gamemode == shareware ? 4 : 6);

    for (int i = 0; i < armsnum; i++)
        STlib_initMultIcon(&w_arms[i], ST_ARMSX + (i % 3) * ST_ARMSXSPACE, ST_ARMSY + i / 3 * ST_ARMSYSPACE,
            arms[i], (i == 1 ? &viewplayer->shotguns : &viewplayer->weaponowned[i + 1]), &st_statusbaron);

    // faces
    STlib_initMultIcon(&w_faces, ST_FACESX, ST_FACESY, faces, &st_faceindex, &st_statusbaron);

    // armor percentage
    STlib_initPercent(&w_armor, ST_ARMORX, ST_ARMORY + (STBAR != 2 && !BTSX), tallnum, &viewplayer->armorpoints,
        &st_statusbaron, tallpercent);

    // keyboxes 0-2
    STlib_initMultIcon(&w_keyboxes[0], ST_KEY0X + (STBAR >= 3), ST_KEY0Y, keys, &keyboxes[0], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes[1], ST_KEY1X + (STBAR >= 3), ST_KEY1Y, keys, &keyboxes[1], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes[2], ST_KEY2X + (STBAR >= 3), ST_KEY2Y, keys, &keyboxes[2], &st_statusbaron);

    usesmallnums = ((!STYSNUM0 && STBAR == 2) || gamemode == shareware);

    // ammo count (all four kinds)
    STlib_initNum(&w_ammo[0], ST_AMMO0X, ST_AMMO0Y, shortnum, &viewplayer->ammo[0], &st_statusbaron, ST_AMMO0WIDTH);
    STlib_initNum(&w_ammo[1], ST_AMMO1X, ST_AMMO1Y, shortnum, &viewplayer->ammo[1], &st_statusbaron, ST_AMMO1WIDTH);
    STlib_initNum(&w_ammo[2], ST_AMMO2X, ST_AMMO2Y, shortnum, &viewplayer->ammo[2], &st_statusbaron, ST_AMMO2WIDTH);
    STlib_initNum(&w_ammo[3], ST_AMMO3X, ST_AMMO3Y, shortnum, &viewplayer->ammo[3], &st_statusbaron, ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_initNum(&w_maxammo[0], ST_MAXAMMO0X, ST_MAXAMMO0Y, shortnum, &viewplayer->maxammo[0], &st_statusbaron,
        ST_MAXAMMO0WIDTH);
    STlib_initNum(&w_maxammo[1], ST_MAXAMMO1X, ST_MAXAMMO1Y, shortnum, &viewplayer->maxammo[1], &st_statusbaron,
        ST_MAXAMMO1WIDTH);
    STlib_initNum(&w_maxammo[2], ST_MAXAMMO2X, ST_MAXAMMO2Y, shortnum, &viewplayer->maxammo[2], &st_statusbaron,
        ST_MAXAMMO2WIDTH);
    STlib_initNum(&w_maxammo[3], ST_MAXAMMO3X, ST_MAXAMMO3Y, shortnum, &viewplayer->maxammo[3], &st_statusbaron,
        ST_MAXAMMO3WIDTH);
}

static dboolean st_stopped = true;

static void ST_Stop(void)
{
    if (st_stopped)
        return;

    I_SetPalette(W_CacheLumpNum(lu_palette));
    st_stopped = true;
}

void ST_Start(void)
{
    if (!st_stopped)
        ST_Stop();

    ST_initData();
    ST_createWidgets();
    st_stopped = false;
}

void ST_Init(void)
{
    ST_loadData();
    screens[4] = malloc(ST_WIDTH * SBARHEIGHT);

    // [BH] fix evil grin being displayed when picking up first item after
    // loading save game or entering IDFA/IDKFA cheat
    for (int i = 0; i < NUMWEAPONS; i++)
        oldweaponsowned[i] = false;

    ST_initCheats();
}
