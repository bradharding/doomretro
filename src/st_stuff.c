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
#include <string.h>

#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_timer.h"
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

// Location of status bar
#define ST_X                0
#define ST_Y                (SCREENHEIGHT - SBARHEIGHT)

#define ST_TURNOFFSET       ST_NUMSTRAIGHTFACES
#define ST_OUCHOFFSET       (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET   (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET    (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE          (ST_NUMPAINFACES * ST_FACESTRIDE)
#define ST_DEADFACE         (ST_GODFACE + 1)
#define ST_XDTHFACE         (ST_DEADFACE + 1)

#define ST_FACESX           (chex ? 144 : 143)
#define ST_FACESY           168

#define ST_FACEBACKX        (144 * 2 + WIDESCREENDELTA * 2)
#define ST_FACEBACKY        (170 * 2)
#define ST_FACEBACKWIDTH    (32 * 2)
#define ST_FACEBACKHEIGHT   (29 * 2)

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
#define ST_KEY1X            ST_KEY0X
#define ST_KEY1Y            181
#define ST_KEY2X            ST_KEY0X
#define ST_KEY2Y            191

// Ammunition counter.
#define ST_AMMOBGX          (248 + WIDESCREENDELTA)
#define ST_AMMO0WIDTH       3
#define ST_AMMO0X           288
#define ST_AMMO0Y           173
#define ST_AMMO1WIDTH       ST_AMMO0WIDTH
#define ST_AMMO1X           ST_AMMO0X
#define ST_AMMO1Y           179
#define ST_AMMO2WIDTH       ST_AMMO0WIDTH
#define ST_AMMO2X           ST_AMMO0X
#define ST_AMMO2Y           185
#define ST_AMMO3WIDTH       ST_AMMO0WIDTH
#define ST_AMMO3X           ST_AMMO0X
#define ST_AMMO3Y           191

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH    3
#define ST_MAXAMMO0X        (BTSX ? 313 : 314)
#define ST_MAXAMMO0Y        173
#define ST_MAXAMMO1WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X        ST_MAXAMMO0X
#define ST_MAXAMMO1Y        179
#define ST_MAXAMMO2WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X        ST_MAXAMMO0X
#define ST_MAXAMMO2Y        185
#define ST_MAXAMMO3WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X        ST_MAXAMMO0X
#define ST_MAXAMMO3Y        191

// ST_Start() has just been called
static bool             st_firsttime;

// whether left-side main status bar is active
static bool             st_statusbaron;

// main bar left
static patch_t          *sbar;
static patch_t          *sbar2;
static short            sbarwidth;
static short            sbar2width;

// 0-9, tall numbers
patch_t                 *tallnum[10];
short                   tallnum0width;
short                   tallnum1width;

// tall % sign
patch_t                 *tallpercent;
short                   tallpercentwidth;
bool                    emptytallpercent;

// 0-9, short, yellow, (different!) numbers
static patch_t          *shortnum[10];

// 3 key-cards, 3 skulls
static patch_t          *keys[NUMCARDS + 3];

// face status patches
patch_t                 *faces[ST_NUMFACES];
static int              xdthfaces;

// main bar right
static patch_t          *armsbg;

static patch_t          *ammobg;
static patch_t          *ammobg2;
static short            ammobg2width;

// weapon ownership patches
static patch_t          *arms[6][2];

// ready-weapon widget
static st_number_t      w_ready;

// health widget
static st_percent_t     w_health;

// weapon ownership widgets
static st_multicon_t    w_arms[6];

// face status widget
static st_multicon_t    w_faces;

// keycard widgets
static st_multicon_t    w_keyboxes[3];

// armor widget
static st_percent_t     w_armor;

// ammo widgets
static st_number_t      w_ammo[4];

// max ammo widgets
static st_number_t      w_maxammo[4];

byte                    *grnrock;
patch_t                 *brdr_t;
patch_t                 *brdr_b;
patch_t                 *brdr_l;
patch_t                 *brdr_r;
patch_t                 *brdr_tl;
patch_t                 *brdr_tr;
patch_t                 *brdr_bl;
patch_t                 *brdr_br;

bool                    st_drawbrdr;

// used to use appropriately pained face
static int              st_oldhealth = -1;

// used for evil grin
bool                    oldweaponsowned[NUMWEAPONS];

int                     st_palette = 0;

// count until face changes
int                     st_facecount;

// current face index, used by w_faces
int                     st_faceindex;

static int              st_shotguns;

// holds key-type for each key box on bar
static int              keyboxes[3];

int                     oldhealth = 100;

bool                    idclev;
int                     idclevtics = 0;

bool                    idmus;

cheatseq_t              cheat_mus = CHEAT("idmus", 0, false);
cheatseq_t              cheat_mus_xy = CHEAT("idmus", 2, false);
cheatseq_t              cheat_god = CHEAT("iddqd", 0, false);
cheatseq_t              cheat_ammo = CHEAT("idkfa", 0, false);
cheatseq_t              cheat_ammonokey = CHEAT("idfa", 0, false);
cheatseq_t              cheat_noclip = CHEAT("idspispopd", 0, false);
cheatseq_t              cheat_commercial_noclip = CHEAT("idclip", 0, false);

cheatseq_t              cheat_choppers = CHEAT("idchoppers", 0, false);
cheatseq_t              cheat_clev = CHEAT("idclev", 0, false);
cheatseq_t              cheat_clev_xy = CHEAT("idclev", 2, false);
cheatseq_t              cheat_mypos = CHEAT("idmypos", 0, false);
cheatseq_t              cheat_amap = CHEAT("iddt", 0, false);
cheatseq_t              cheat_buddha = CHEAT("ryhan", 0, false);

cheatseq_t cheat_powerup[7] =
{
    CHEAT("idbeholdv", 0, true),
    CHEAT("idbeholds", 0, true),
    CHEAT("idbeholdi", 0, true),
    CHEAT("idbeholdr", 0, true),
    CHEAT("idbeholda", 0, true),
    CHEAT("idbeholdl", 0, true),
    CHEAT("idbehold",  0, true)
};

static bool movekey(char key)
{
    return (key == keyboardright || key == keyboardright2
        || key == keyboardleft || key == keyboardleft2
        || key == keyboardforward || key == keyboardforward2
        || key == keyboardback || key == keyboardback2
        || key == keyboardstrafeleft || key == keyboardstrafeleft2
        || key == keyboardstraferight || key == keyboardstraferight2);
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
    cheat_clev.movekey = movekey(cheat_clev.sequence[0]);
    cheat_clev_xy.movekey = movekey(cheat_clev_xy.sequence[0]);
    cheat_mypos.movekey = movekey(cheat_mypos.sequence[0]);
    cheat_amap.movekey = movekey(cheat_amap.sequence[0]);
    cheat_buddha.movekey = movekey(cheat_buddha.sequence[0]);
}

const int mus[IDMUS_MAX][6] =
{
    /* xy      shareware   registered  commercial  retail      bfgedition  nerve      */
    /* 00 */ { mus_none,   mus_none,   mus_none,   mus_none,   mus_none,   mus_none   },
    /* 01 */ { mus_none,   mus_none,   mus_runnin, mus_none,   mus_runnin, mus_messag },
    /* 02 */ { mus_none,   mus_none,   mus_stalks, mus_none,   mus_stalks, mus_ddtblu },
    /* 03 */ { mus_none,   mus_none,   mus_countd, mus_none,   mus_countd, mus_doom   },
    /* 04 */ { mus_none,   mus_none,   mus_betwee, mus_none,   mus_betwee, mus_shawn  },
    /* 05 */ { mus_none,   mus_none,   mus_doom,   mus_none,   mus_doom,   mus_in_cit },
    /* 06 */ { mus_none,   mus_none,   mus_the_da, mus_none,   mus_the_da, mus_the_da },
    /* 07 */ { mus_none,   mus_none,   mus_shawn,  mus_none,   mus_shawn,  mus_in_cit },
    /* 08 */ { mus_none,   mus_none,   mus_ddtblu, mus_none,   mus_ddtblu, mus_shawn  },
    /* 09 */ { mus_none,   mus_none,   mus_in_cit, mus_none,   mus_in_cit, mus_ddtblu },
    /* 10 */ { mus_none,   mus_none,   mus_dead,   mus_none,   mus_dead,   mus_none   },
    /* 11 */ { mus_e1m1,   mus_e1m1,   mus_stlks2, mus_e1m1,   mus_stlks2, mus_none   },
    /* 12 */ { mus_e1m2,   mus_e1m2,   mus_theda2, mus_e1m2,   mus_theda2, mus_none   },
    /* 13 */ { mus_e1m3,   mus_e1m3,   mus_doom2,  mus_e1m3,   mus_doom2,  mus_none   },
    /* 14 */ { mus_e1m4,   mus_e1m4,   mus_ddtbl2, mus_e1m4,   mus_ddtbl2, mus_none   },
    /* 15 */ { mus_e1m5,   mus_e1m5,   mus_runni2, mus_e1m5,   mus_runni2, mus_none   },
    /* 16 */ { mus_e1m6,   mus_e1m6,   mus_dead2,  mus_e1m6,   mus_dead2,  mus_none   },
    /* 17 */ { mus_e1m7,   mus_e1m7,   mus_stlks3, mus_e1m7,   mus_stlks3, mus_none   },
    /* 18 */ { mus_e1m8,   mus_e1m8,   mus_romero, mus_e1m8,   mus_romero, mus_none   },
    /* 19 */ { mus_e1m9,   mus_e1m9,   mus_shawn2, mus_e1m9,   mus_shawn2, mus_none   },
    /* 20 */ { mus_none,   mus_none,   mus_messag, mus_none,   mus_messag, mus_none   },
    /* 21 */ { mus_none,   mus_e2m1,   mus_count2, mus_e2m1,   mus_count2, mus_none   },
    /* 22 */ { mus_none,   mus_e2m2,   mus_ddtbl3, mus_e2m2,   mus_ddtbl3, mus_none   },
    /* 23 */ { mus_none,   mus_e2m3,   mus_ampie,  mus_e2m3,   mus_ampie,  mus_none   },
    /* 24 */ { mus_none,   mus_e2m4,   mus_theda3, mus_e2m4,   mus_theda3, mus_none   },
    /* 25 */ { mus_none,   mus_e2m5,   mus_adrian, mus_e2m5,   mus_adrian, mus_none   },
    /* 26 */ { mus_none,   mus_e2m6,   mus_messg2, mus_e2m6,   mus_messg2, mus_none   },
    /* 27 */ { mus_none,   mus_e2m7,   mus_romer2, mus_e2m7,   mus_romer2, mus_none   },
    /* 28 */ { mus_none,   mus_e2m8,   mus_tense,  mus_e2m8,   mus_tense,  mus_none   },
    /* 29 */ { mus_none,   mus_e2m9,   mus_shawn3, mus_e2m9,   mus_shawn3, mus_none   },
    /* 30 */ { mus_none,   mus_none,   mus_openin, mus_none,   mus_openin, mus_none   },
    /* 31 */ { mus_none,   mus_e3m1,   mus_evil,   mus_e3m1,   mus_evil,   mus_none   },
    /* 32 */ { mus_none,   mus_e3m2,   mus_ultima, mus_e3m2,   mus_ultima, mus_none   },
    /* 33 */ { mus_none,   mus_e3m3,   mus_none,   mus_e3m3,   mus_read_m, mus_none   },
    /* 34 */ { mus_none,   mus_e3m4,   mus_none,   mus_e3m4,   mus_none,   mus_none   },
    /* 35 */ { mus_none,   mus_e3m5,   mus_none,   mus_e3m5,   mus_none,   mus_none   },
    /* 36 */ { mus_none,   mus_e3m6,   mus_none,   mus_e3m6,   mus_none,   mus_none   },
    /* 37 */ { mus_none,   mus_e3m7,   mus_none,   mus_e3m7,   mus_none,   mus_none   },
    /* 38 */ { mus_none,   mus_e3m8,   mus_none,   mus_e3m8,   mus_none,   mus_none   },
    /* 39 */ { mus_none,   mus_e3m9,   mus_none,   mus_e3m9,   mus_none,   mus_none   },
    /* 40 */ { mus_none,   mus_none,   mus_none,   mus_none,   mus_none,   mus_none   },
    /* 41 */ { mus_none,   mus_none,   mus_none,   mus_e3m4,   mus_none,   mus_none   },
    /* 42 */ { mus_none,   mus_none,   mus_none,   mus_e3m2,   mus_none,   mus_none   },
    /* 43 */ { mus_none,   mus_none,   mus_none,   mus_e3m3,   mus_none,   mus_none   },
    /* 44 */ { mus_none,   mus_none,   mus_none,   mus_e1m5,   mus_none,   mus_none   },
    /* 45 */ { mus_none,   mus_none,   mus_none,   mus_e2m7,   mus_none,   mus_none   },
    /* 46 */ { mus_none,   mus_none,   mus_none,   mus_e2m4,   mus_none,   mus_none   },
    /* 47 */ { mus_none,   mus_none,   mus_none,   mus_e2m6,   mus_none,   mus_none   },
    /* 48 */ { mus_none,   mus_none,   mus_none,   mus_e2m5,   mus_none,   mus_none   },
    /* 49 */ { mus_none,   mus_none,   mus_none,   mus_e1m9,   mus_none,   mus_none   },
    /* 50 */ { mus_none,   mus_none,   mus_none,   mus_none,   mus_none,   mus_none   },
    /* 51 */ { mus_none,   mus_none,   mus_none,   mus_e5m1,   mus_none,   mus_none   },
    /* 52 */ { mus_none,   mus_none,   mus_none,   mus_e5m2,   mus_none,   mus_none   },
    /* 53 */ { mus_none,   mus_none,   mus_none,   mus_e5m3,   mus_none,   mus_none   },
    /* 54 */ { mus_none,   mus_none,   mus_none,   mus_e5m4,   mus_none,   mus_none   },
    /* 55 */ { mus_none,   mus_none,   mus_none,   mus_e5m5,   mus_none,   mus_none   },
    /* 56 */ { mus_none,   mus_none,   mus_none,   mus_e5m6,   mus_none,   mus_none   },
    /* 57 */ { mus_none,   mus_none,   mus_none,   mus_e5m7,   mus_none,   mus_none   },
    /* 58 */ { mus_none,   mus_none,   mus_none,   mus_e5m8,   mus_none,   mus_none   },
    /* 59 */ { mus_none,   mus_none,   mus_none,   mus_e5m9,   mus_none,   mus_none   },
    /* 60 */ { mus_none,   mus_none,   mus_none,   mus_none,   mus_none,   mus_none   },
    /* 61 */ { mus_none,   mus_none,   mus_none,   mus_e6m1,   mus_none,   mus_none   },
    /* 62 */ { mus_none,   mus_none,   mus_none,   mus_e6m2,   mus_none,   mus_none   },
    /* 63 */ { mus_none,   mus_none,   mus_none,   mus_e6m3,   mus_none,   mus_none   },
    /* 64 */ { mus_none,   mus_none,   mus_none,   mus_e6m4,   mus_none,   mus_none   },
    /* 65 */ { mus_none,   mus_none,   mus_none,   mus_e6m5,   mus_none,   mus_none   },
    /* 66 */ { mus_none,   mus_none,   mus_none,   mus_e6m6,   mus_none,   mus_none   },
    /* 67 */ { mus_none,   mus_none,   mus_none,   mus_e6m7,   mus_none,   mus_none   },
    /* 68 */ { mus_none,   mus_none,   mus_none,   mus_e6m8,   mus_none,   mus_none   },
    /* 69 */ { mus_none,   mus_none,   mus_none,   mus_e6m9,   mus_none,   mus_none   }
};

//
// STATUS BAR CODE
//
static void ST_RefreshBackground(void)
{
    if (STBARs < 3 || legacyofrust)
    {
        if (r_detail == r_detail_high)
        {
            if (vid_widescreen)
            {
                if (sbar2width < SCREENWIDTH)
                    R_FillBezel();

                V_DrawBigPatch((SCREENWIDTH - sbar2width) / 2, ST_Y, sbar2width, SBARHEIGHT, sbar2);
            }
            else
                V_DrawBigPatch(ST_X, ST_Y, sbar2width, SBARHEIGHT, sbar2);

            if (ammobg2)
                V_DrawBigPatch(ST_AMMOBGX * 2, ST_Y, ammobg2width, SBARHEIGHT, ammobg2);
        }
        else
        {
            if (sbarwidth < SCREENWIDTH)
                R_FillBezel();

            V_DrawWidePatch((SCREENWIDTH / 2 - sbarwidth) / 2, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, sbar);

            if (ammobg)
                V_DrawWidePatch(ST_AMMOBGX, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, ammobg);
        }
    }
    else
    {
        if (sbarwidth < SCREENWIDTH)
            R_FillBezel();

        V_DrawWidePatch((SCREENWIDTH / 2 - sbarwidth) / 2, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, sbar);
        V_DrawPatch((hacx ? ST_ARMSBGX + 4 : ST_ARMSBGX), VANILLAHEIGHT - VANILLASBARHEIGHT, 0, armsbg);
    }
}

void ST_PlayerCheated(const char *cheat, const char *parm, const char *output, const bool warning)
{
    char    *temp = M_StringJoin(cheat, parm, NULL);

    C_Cheat(temp);
    free(temp);

    if (output)
        C_Output(output);

    if (warning)
    {
        C_PlayerWarning("%s cheated!",
            (M_StringCompare(playername, playername_default) ? "You" : playername));
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        viewplayer->cheated++;
        M_SaveCVARs();
    }
}

static int ST_CalcPainOffset(void)
{
    const int   newhealth = MIN(viewplayer->health, 100);
    static int  lastcalc;
    static int  health = -1;

    if (newhealth != health)
    {
        lastcalc = ST_FACESTRIDE * (((100 - newhealth) * ST_NUMPAINFACES) / 101);
        health = newhealth;
    }

    return lastcalc;
}

// Respond to keyboard input events, intercept cheats.
bool ST_Responder(const event_t *ev)
{
    // if a user keypress...
    if (ev->type == ev_keydown || *consolecheat)
    {
        if (!menuactive && !paused)     // [BH] no cheats when in menu or paused
        {
            bool    cheatfailed = false;

            if (!*consolecheat && cht_CheckCheat(&cheat_mus, ev->data2) && !nomusic && musicvolume)
                idmus = true;

            // 'dqd' cheat for toggleable god mode
            if (cht_CheckCheat(&cheat_god, ev->data2) && gameskill != sk_nightmare)
            {
                S_StartSound(NULL, sfx_getpow);

                // [BH] if player is dead, resurrect them first
                if (viewplayer->health <= 0)
                    P_ResurrectPlayer(initial_health);

                if ((viewplayer->cheats ^= CF_GODMODE) & CF_GODMODE)
                {
                    if (viewplayer->cheats & CF_BUDDHA)
                    {
                        viewplayer->cheats &= ~CF_BUDDHA;
                        C_Output(s_STSTR_BUDDHAOFF);
                    }

                    if (viewplayer->powers[pw_invulnerability] > STARTFLASHING)
                        viewplayer->powers[pw_invulnerability] = STARTFLASHING;

                    // [BH] remember player's current health,
                    //  and only set to 100% if less than 100%
                    oldhealth = viewplayer->health;
                    P_GiveHealth(god_health, god_health, false);

                    if (oldhealth < initial_health)
                        P_AddBonus();

                    ST_PlayerCheated(cheat_god.sequence, "", NULL, true);
                    C_Output(s_STSTR_DQDON);
                    HU_SetPlayerMessage(s_STSTR_DQDON, false, false);
                }
                else
                {
                    ST_PlayerCheated(cheat_god.sequence, "", NULL, false);
                    C_Output(s_STSTR_DQDOFF);
                    HU_SetPlayerMessage(s_STSTR_DQDOFF, false, false);

                    // [BH] restore player's health
                    P_AnimateHealth(viewplayer->health - oldhealth);
                    viewplayer->health = oldhealth;
                    viewplayer->mo->health = oldhealth;
                }

                message_dontfuckwithme = true;
            }

            // 'fa' cheat for killer fucking arsenal
            else if (cht_CheckCheat(&cheat_ammonokey, ev->data2) && gameskill != sk_nightmare && viewplayer->health > 0)
            {
                bool    ammogiven;
                bool    armorgiven = false;
                bool    berserkgiven = P_GivePower(pw_strength, false);
                bool    weaponsgiven = P_GiveAllWeapons();

                // [BH] note if player doesn't have full armor before giving it
                if (viewplayer->armor < idfa_armor || viewplayer->armortype < idfa_armor_class)
                {
                    armorgiven = true;
                    P_AnimateArmor(viewplayer->armor - idfa_armor);
                    viewplayer->armor = idfa_armor;
                    viewplayer->armortype = idfa_armor_class;
                }

                // [BH] give player a backpack if they don't have one
                P_GiveBackpack(false, false);

                // [BH] give player full ammo (which is double now player has backpack)
                ammogiven = P_GiveFullAmmo();

                // [BH] only acknowledge cheat if player was given something
                if (ammogiven || armorgiven || berserkgiven || weaponsgiven)
                {
                    S_StartSound(NULL, sfx_getpow);

                    // [BH] flash screen
                    P_AddBonus();

                    ST_PlayerCheated(cheat_ammonokey.sequence, "", NULL, true);
                    C_Output(s_STSTR_FAADDED);
                    HU_SetPlayerMessage(s_STSTR_FAADDED, false, false);
                    message_dontfuckwithme = true;
                }
                else
                    cheatfailed = true;
            }

            // 'kfa' cheat for key full ammo
            else if (cht_CheckCheat(&cheat_ammo, ev->data2) && gameskill != sk_nightmare
                // [BH] can only enter cheat while player is alive
                && viewplayer->health > 0)
            {
                bool    ammogiven;
                bool    armorgiven = false;
                bool    berserkgiven = P_GivePower(pw_strength, false);
                bool    weaponsgiven = P_GiveAllWeapons();
                bool    keysgiven = P_GiveAllCardsInMap();

                // [BH] note if player doesn't have full armor before giving it
                if (viewplayer->armor < idkfa_armor || viewplayer->armortype < idkfa_armor_class)
                {
                    armorgiven = true;
                    P_AnimateArmor(viewplayer->armor - idkfa_armor);
                    viewplayer->armor = idkfa_armor;
                    viewplayer->armortype = idkfa_armor_class;
                }

                // [BH] give player a backpack if they don't have one
                P_GiveBackpack(false, false);

                // [BH] give player full ammo (which is double now player has backpack)
                ammogiven = P_GiveFullAmmo();

                // [BH] only acknowledge cheat if player was given something
                if (ammogiven || armorgiven || berserkgiven || weaponsgiven || keysgiven)
                {
                    S_StartSound(NULL, sfx_getpow);

                    // [BH] flash screen
                    P_AddBonus();

                    ST_PlayerCheated(cheat_ammo.sequence, "", NULL, true);
                    C_Output(s_STSTR_KFAADDED);
                    HU_SetPlayerMessage(s_STSTR_KFAADDED, false, false);
                    message_dontfuckwithme = true;
                }
                else
                    cheatfailed = true;
            }

            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus_xy, ev->data2) && !nomusic && musicvolume)
            {
                char   buffer[3];

                // [BH] only display message if parameter is valid
                cht_GetParam(&cheat_mus_xy, buffer);

                // [BH] Fix <https://doomwiki.org/wiki/IDMUS_requests_invalid_music>.
                if (buffer[0] >= '0' && buffer[0] <= '9' && buffer[1] >= '0' && buffer[1] <= '9')
                {
                    int musnum = (buffer[0] - '0') * 10 + (buffer[1] - '0');

                    if (musnum < IDMUS_MAX)
                    {
                        if (gamemission == pack_nerve)
                            musnum = mus[musnum][5];
                        else if (bfgedition && gamemission == doom2)
                            musnum = mus[musnum][4];
                        else
                            musnum = mus[musnum][gamemode];

                        if (musnum != mus_none)
                        {
                            char    lump[9];
                            char    *temp = uppercase(s_music[musnum].name1);

                            M_snprintf(lump, sizeof(lump), "D_%s", temp);

                            if (W_CheckNumForName(lump) >= 0)
                            {
                                static char msg[80];

                                S_StartSound(NULL, sfx_getpow);

                                S_ChangeMusic(musnum, 1, true, false);
                                ST_PlayerCheated(cheat_mus.sequence, "xy", NULL, false);
                                M_snprintf(msg, sizeof(msg), s_STSTR_MUS, temp);
                                C_Output(msg);
                                HU_SetPlayerMessage(msg, false, false);
                                message_dontfuckwithme = true;
                            }

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

                viewplayer->cheats ^= CF_NOCLIP;

                if (viewplayer->cheats & CF_NOCLIP)
                {
                    ST_PlayerCheated((gamemode == commercial ? cheat_commercial_noclip.sequence : cheat_noclip.sequence), "", NULL, true);
                    C_Output(s_STSTR_NCON);
                    HU_SetPlayerMessage(s_STSTR_NCON, false, false);
                }
                else
                {
                    ST_PlayerCheated((gamemode == commercial ? cheat_commercial_noclip.sequence : cheat_noclip.sequence), "", NULL, false);
                    C_Output(s_STSTR_NCOFF);
                    HU_SetPlayerMessage(s_STSTR_NCOFF, false, false);
                }

                message_dontfuckwithme = true;
            }

            // 'behold?' power-up cheats
            for (int i = 1; i < NUMPOWERS; i++)
            {
                if (cht_CheckCheat(&cheat_powerup[i - 1], ev->data2) && gameskill != sk_nightmare
                    // [BH] can only enter cheat while player is alive
                    && viewplayer->health > 0)
                {
                    static char buffer[128];

                    S_StartSound(NULL, sfx_getpow);

                    if ((i != pw_strength && viewplayer->powers[i] >= 0 && viewplayer->powers[i] <= STARTFLASHING)
                        || (i == pw_strength && !viewplayer->powers[i]))
                    {
                        if (i == pw_invulnerability)
                        {
                            if (viewplayer->cheats & CF_GODMODE)
                            {
                                viewplayer->cheats &= ~CF_GODMODE;
                                C_Output(s_STSTR_GODOFF);
                            }
                            else if (viewplayer->cheats & CF_BUDDHA)
                            {
                                viewplayer->cheats &= ~CF_BUDDHA;
                                C_Output(s_STSTR_BUDDHAOFF);
                            }
                        }

                        P_GivePower(i, false);

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

                        M_snprintf(buffer, sizeof(buffer), "%s " BOLD("%c"),
                            s_STSTR_BEHOLD, cheat_powerup[i - 1].sequence[strlen(cheat_powerup[i - 1].sequence) - 1]);
                        ST_PlayerCheated(cheat_powerup[i - 1].sequence, "", buffer, true);

                        if (!M_StringCompare(s_STSTR_BEHOLDX, STSTR_BEHOLDX))
                        {
                            C_Output(s_STSTR_BEHOLDX);
                            HU_SetPlayerMessage(s_STSTR_BEHOLDX, false, false);
                        }
                        else
                        {
                            static char message[128];

                            M_snprintf(message, sizeof(message), s_STSTR_BEHOLDON, powerups[i]);
                            C_Output(message);
                            HU_SetPlayerMessage(message, false, false);
                        }
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
                            if (freeze)
                            {
                                viewplayer->powers[i] = 0;

                                if (i == pw_invulnerability)
                                {
                                    viewplayer->fixedcolormap = 0;
                                    st_faceindex = ST_STRAIGHTFACE;
                                    st_facecount = ST_STRAIGHTFACECOUNT;
                                }
                            }
                            else
                                viewplayer->powers[i] = STARTFLASHING * (i != pw_allmap);
                        }

                        M_snprintf(buffer, sizeof(buffer), "%s " BOLD("%c"),
                            s_STSTR_BEHOLD, cheat_powerup[i - 1].sequence[strlen(cheat_powerup[i - 1].sequence) - 1]);
                        ST_PlayerCheated(cheat_powerup[i - 1].sequence, "", buffer, false);

                        if (!M_StringCompare(s_STSTR_BEHOLDX, STSTR_BEHOLDX))
                        {
                            C_Output(s_STSTR_BEHOLDX);
                            HU_SetPlayerMessage(s_STSTR_BEHOLDX, false, false);
                        }
                        else
                        {
                            static char message[128];

                            M_snprintf(message, sizeof(message), s_STSTR_BEHOLDOFF, powerups[i]);
                            C_Output(message);
                            HU_SetPlayerMessage(message, false, false);
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

                    if (!cheatfailed)
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

            // 'choppers' invulnerability and chainsaw
            else if (cht_CheckCheat(&cheat_choppers, ev->data2) && gameskill != sk_nightmare
                // [BH] can only enter cheat while player is alive
                && viewplayer->health > 0)
            {
                S_StartSound(NULL, sfx_getpow);

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
                        viewplayer->pendingweapon = wp_chainsaw;
                    }

                    if (viewplayer->cheats & CF_GODMODE)
                    {
                        viewplayer->cheats &= ~CF_GODMODE;
                        C_Output(s_STSTR_GODOFF);
                    }
                    else if (viewplayer->cheats & CF_BUDDHA)
                    {
                        viewplayer->cheats &= ~CF_BUDDHA;
                        C_Output(s_STSTR_BUDDHAOFF);
                    }

                    // [BH] fixed bug where invulnerability was never given, and now
                    //  needs to be toggled off with cheat or switch from chainsaw
                    P_GivePower(pw_invulnerability, false);
                    viewplayer->powers[pw_invulnerability] = -1;

                    ST_PlayerCheated(cheat_choppers.sequence, "", NULL, true);
                    C_Output(s_STSTR_CHOPPERS);
                    HU_SetPlayerMessage(s_STSTR_CHOPPERS, false, false);

                    message_dontfuckwithme = true;
                    viewplayer->cheats |= CF_CHOPPERS;
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

                    ST_PlayerCheated(cheat_choppers.sequence, "", NULL, false);
                }
            }

            // 'mypos' for player position
            else if (cht_CheckCheat(&cheat_mypos, ev->data2))
            {
                S_StartSound(NULL, sfx_getpow);

                // [BH] message stays on screen until toggled off again using cheat.
                viewplayer->cheats ^= CF_MYPOS;
                ST_PlayerCheated(cheat_mypos.sequence, "", NULL, (viewplayer->cheats & CF_MYPOS));
            }

            else if (cht_CheckCheat(&cheat_buddha, ev->data2) && gameskill != sk_nightmare && viewplayer->health > 0)
            {
                S_StartSound(NULL, sfx_getpow);

                if ((viewplayer->cheats ^= CF_BUDDHA) & CF_BUDDHA)
                {
                    if (viewplayer->cheats & CF_GODMODE)
                    {
                        viewplayer->cheats &= ~CF_GODMODE;
                        C_Output(s_STSTR_GODOFF);
                    }

                    if (viewplayer->powers[pw_invulnerability] > STARTFLASHING)
                        viewplayer->powers[pw_invulnerability] = STARTFLASHING;

                    ST_PlayerCheated(cheat_buddha.sequence, "", NULL, true);
                    C_Output(s_STSTR_BUDDHAON);
                    HU_SetPlayerMessage(s_STSTR_BUDDHAON, false, false);
                }
                else
                {
                    ST_PlayerCheated(cheat_buddha.sequence, "", NULL, false);
                    C_Output(s_STSTR_BUDDHAOFF);
                    HU_SetPlayerMessage(s_STSTR_BUDDHAOFF, false, false);
                }

                message_dontfuckwithme = true;
            }

            else if ((automapactive || mapwindow) && cht_CheckCheat(&cheat_amap, ev->data2))
            {
                S_StartSound(NULL, sfx_getpow);

                if (viewplayer->cheats & CF_ALLMAP_THINGS)
                {
                    viewplayer->cheats &= ~CF_ALLMAP_THINGS;
                    ST_PlayerCheated(cheat_amap.sequence, "", NULL, false);
                }
                else
                {
                    if (viewplayer->cheats & CF_ALLMAP)
                    {
                        viewplayer->cheats &= ~CF_ALLMAP;
                        viewplayer->cheats |= CF_ALLMAP_THINGS;
                    }
                    else
                        viewplayer->cheats |= CF_ALLMAP;

                    ST_PlayerCheated(cheat_amap.sequence, "", NULL, true);
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

                if (legacyofrust)
                {
                    epsd = buffer[0] - '0';
                    map = buffer[1] - '0';

                    if (epsd <= 2 && map <= 7)
                        map = (epsd - 1) * 7 + map;
                    else if (epsd <= 2 && map == 8)
                        map = 14 + epsd;
                    else
                        map = epsd * 10 + map;

                    M_snprintf(lump, sizeof(lump), "MAP%02i", map);
                }
                else if (gamemode == commercial)
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
                if (W_CheckNumForName(lump) < 0 || (gamemission == pack_nerve && map > 9) || (BTSX && W_GetNumLumps(lump) == 1))
                    return false;
                else
                {
                    static char message[128];

                    S_StartSound(NULL, sfx_getpow);
                    ST_PlayerCheated(cheat_clev_xy.sequence, "xy", NULL, true);

                    if (legacyofrust && map != 99)
                        M_snprintf(lump, sizeof(lump), "E%cM%c", buffer[0], buffer[1]);
                    else if (BTSX)
                        M_snprintf(lump, sizeof(lump), "E%iM%c%c", (BTSXE1 ? 1 : (BTSXE2 ? 2 : 3)), buffer[0], buffer[1]);

                    if (M_StringCompare(lump, mapnum))
                        M_snprintf(message, sizeof(message), s_STSTR_CLEVSAME, lump);
                    else
                        M_snprintf(message, sizeof(message), s_STSTR_CLEV,
                            (M_StringCompare(playername, playername_default) ? "you" : playername), lump);

                    C_Output(message);
                    HU_SetPlayerMessage(message, false, false);

                    if (legacyofrust)
                        epsd = 1;

                    if (P_IsSecret(epsd, map))
                        message_secret = true;
                    else if (gamemode == commercial)
                    {
                        if (map == 31 || map == 32 || (map == 33 && bfgedition) || (gamemission == pack_nerve && map == 9))
                            message_secret = true;
                    }
                    else
                    {
                        if (map == 9)
                            message_secret = true;
                    }

                    // [BH] always display message
                    message_dontfuckwithme = true;

                    // [BH] delay map change by 1 second to allow message to be displayed
                    samelevel = (gameepisode == epsd && gamemap == map);
                    gameepisode = epsd;

                    if (gamemission == doom)
                    {
                        episode = gameepisode;
                        EpiDef.laston = episode - 1;
                        M_SaveCVARs();
                    }

                    gamemap = map;
                    idclevtics = TICRATE;
                    quicksaveslot = -1;

                    if (r_diskicon)
                    {
                        drawdisk = true;
                        drawdisktics = DRAWDISKTICS;
                    }
                }
            }
        }

        if (cheatkey)
        {
            cheatkey = '\0';
            return true;
        }

        if (!messagetoprint)
            C_HideConsole();
    }

    return false;
}

//
// This is a not-very-pretty routine which handles the face states and their timing.
// The precedence of expressions is: dead > evil grin > turned head > straight ahead
//
static void ST_UpdateFaceWidget(void)
{
    static int  priority;
    int         painoffset;
    static int  faceindex;

    if (paused || menuactive)
        return;

    // invulnerability
    if (((viewplayer->cheats & CF_GODMODE) || viewplayer->powers[pw_invulnerability]))
    {
        priority = 5;
        st_faceindex = ST_GODFACE;
        st_facecount = 0;
        return;
    }

    if (freeze)
        return;

    if (priority < 10)
    {
        // dead
        if (viewplayer->health <= 0)
        {
            priority = 9;
            painoffset = 0;
            st_facecount = 1;

            // [FG] support face gib animations as in the 3DO/Jaguar/PSX ports
            if (xdthfaces)
            {
                const int   state = (int)(viewplayer->mo->state - states) - mobjinfo[MT_PLAYER].xdeathstate;

                faceindex = (state >= 0 ? ST_XDTHFACE + MIN(state, xdthfaces - 1) : ST_XDTHFACE);
            }
            else
                faceindex = ST_DEADFACE;
        }
        else
            painoffset = ST_CalcPainOffset();
    }
    else
        painoffset = ST_CalcPainOffset();

    if (priority < 9)
    {
        if (viewplayer->bonuscount)
        {
            // picking up bonus
            bool    doevilgrin = false;

            for (int i = 0; i < NUMWEAPONS; i++)
                if (oldweaponsowned[i] != viewplayer->weaponowned[i])
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
                const angle_t   badguyangle = R_PointToAngle(viewplayer->attacker->x, viewplayer->attacker->y);
                angle_t         diffang;

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

        if (viewplayer->attackdown && !consoleactive)
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

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
        priority = 0;
        faceindex = (consoleactive ? ST_STRAIGHTFACE : M_BigRandom() % ST_NUMSTRAIGHTFACES);
        st_facecount = ST_STRAIGHTFACECOUNT;
    }

    st_facecount--;

    // [crispy] fix status bar face hysteresis
    st_faceindex = faceindex + painoffset;
}

static void ST_UpdateWidgets(void)
{
    static int          largeammo = 1994;   // means "n/a"
    const weapontype_t  readyweapon = viewplayer->readyweapon;
    const ammotype_t    ammotype = weaponinfo[readyweapon].ammotype;

    w_ready.num = (ammotype == am_noammo ? &largeammo : &viewplayer->ammo[ammotype]);
    w_ready.data = readyweapon;

    // update keycard multiple widgets
    keyboxes[0] = (viewplayer->cards[it_bluecard] > 0 ?
        (viewplayer->cards[it_blueskull] > 0 && !vanilla ? it_blueskull + 3 : it_bluecard) :
        (viewplayer->cards[it_blueskull] > 0 ? it_blueskull : -1));
    keyboxes[1] = (viewplayer->cards[it_yellowcard] > 0 ?
        (viewplayer->cards[it_yellowskull] > 0 && !vanilla ? it_yellowskull + 3 : it_yellowcard) :
        (viewplayer->cards[it_yellowskull] > 0 ? it_yellowskull : -1));
    keyboxes[2] = (viewplayer->cards[it_redcard] > 0 ?
        (viewplayer->cards[it_redskull] > 0 && !vanilla ? it_redskull + 3 : it_redcard) :
        (viewplayer->cards[it_redskull] > 0 ? it_redskull : -1));

    // refresh everything if this is him coming back to life
    ST_UpdateFaceWidget();
}

void ST_Ticker(void)
{
    if (r_screensize < r_screensize_max)
    {
        ST_UpdateWidgets();
        st_oldhealth = viewplayer->health;
    }
    else if (r_hud && !r_althud)
    {
        ST_UpdateFaceWidget();
        st_oldhealth = viewplayer->health;
    }

    // [BH] action the IDCLEV cheat after a small delay to allow its player message to display
    if (idclevtics && !--idclevtics)
    {
        if (!samelevel)
            S_StopMusic();

        if (vanilla)
            G_DeferredInitNew(gameskill, gameepisode, gamemap);
        else
            G_DeferredLoadLevel(gameskill, gameepisode, gamemap);
    }
}

static void ST_DoPaletteStuff(void)
{
    int palette = 0;

    if (viewplayer->powers[pw_strength]
        && (viewplayer->pendingweapon == wp_fist
            || (viewplayer->readyweapon == wp_fist && viewplayer->pendingweapon == wp_nochange))
        && viewplayer->health > 0)
    {
        const int   bonuscount = viewplayer->bonuscount;

        if (bonuscount && r_pickupeffect)
            palette = STARTBONUSPALS + MIN((bonuscount + 7) >> 3, NUMBONUSPALS) - 1;
        else
        {
            const int   ironfeet = viewplayer->powers[pw_ironfeet];

            if (ironfeet <= STARTFLASHING && (ironfeet & FLASHONTIC) && r_radsuiteffect)
                palette = RADIATIONPAL;
            else if (viewplayer->cheats & CF_GODMODE)
                palette = r_berserkeffect * (PLAYPALs > 2 ? 1 : 2);
            else
                palette = MIN((viewplayer->damagecount >> 3) + r_berserkeffect * (PLAYPALs > 2 ? 1 : 2), NUMREDPALS);
        }
    }
    else
    {
        const int   damagecount = viewplayer->damagecount;

        if (damagecount && !(viewplayer->cheats & CF_GODMODE) && r_damageeffect)
            palette = (chex || r_blood == r_blood_green ? RADIATIONPAL :
                STARTREDPALS + MIN((damagecount + 8) / (PLAYPALs > 2 ? 8 : 5), NUMREDPALS) - 1);
        else if (viewplayer->health > 0)
        {
            const int   bonuscount = viewplayer->bonuscount;

            if (bonuscount && r_pickupeffect)
                palette = STARTBONUSPALS + MIN((bonuscount + 7) >> 3, NUMBONUSPALS) - 1;
            else
            {
                const int   ironfeet = viewplayer->powers[pw_ironfeet];

                if (r_radsuiteffect)
                {
                    if (ironfeet > STARTFLASHING || (ironfeet & FLASHONTIC))
                        palette = RADIATIONPAL;
                }
                else
                {
                    // [BH] flash radsuit effect when r_radsuiteffect CVAR is off
                    // and radsuit just picked up or is running out
                    if ((ironfeet <= STARTFLASHING && (ironfeet & FLASHONTIC))
                        || (ironfeet >= IRONTICS - 6 && r_pickupeffect))
                        palette = RADIATIONPAL;
                }
            }
        }
    }

    if (palette != st_palette)
    {
        st_palette = palette;
        I_SetPalette(&PLAYPAL[st_palette * 768]);
    }
}

static void ST_DrawWidgets(bool refresh)
{
    static bool togglekey = true;

    STlib_UpdateBigAmmoNum(&w_ready);

    STlib_UpdateSmallAmmoNum(&w_ammo[am_clip], am_clip);
    STlib_UpdateSmallMaxAmmoNum(&w_maxammo[am_clip], am_clip);

    STlib_UpdateSmallAmmoNum(&w_ammo[am_shell], am_shell);
    STlib_UpdateSmallMaxAmmoNum(&w_maxammo[am_shell], am_shell);

    STlib_UpdateSmallAmmoNum(&w_ammo[am_misl], am_misl);
    STlib_UpdateSmallMaxAmmoNum(&w_maxammo[am_misl], am_misl);

    STlib_UpdateSmallAmmoNum(&w_ammo[am_cell], am_cell);
    STlib_UpdateSmallMaxAmmoNum(&w_maxammo[am_cell], am_cell);

    STlib_UpdateBigHealth(&w_health, refresh);
    STlib_UpdateBigArmor(&w_armor, refresh);

    st_shotguns = (viewplayer->weaponowned[wp_shotgun] || viewplayer->weaponowned[wp_supershotgun]);

    STlib_UpdateSmallWeaponNum(&w_arms[0], refresh, 0);
    STlib_UpdateSmallWeaponNum(&w_arms[1], refresh, 1);
    STlib_UpdateSmallWeaponNum(&w_arms[2], refresh, 2);
    STlib_UpdateSmallWeaponNum(&w_arms[3], refresh, 3);

    if (gamemode != shareware)
    {
        STlib_UpdateSmallWeaponNum(&w_arms[4], refresh, 4);
        STlib_UpdateSmallWeaponNum(&w_arms[5], refresh, 5);
    }

    if (facebackcolor != facebackcolor_default)
        V_FillRect(0, ST_FACEBACKX, ST_FACEBACKY, ST_FACEBACKWIDTH,
            ST_FACEBACKHEIGHT, nearestcolors[facebackcolor], 0, false, false, NULL, NULL);
    else if (solonet)
        V_FillRect(0, ST_FACEBACKX, ST_FACEBACKY, ST_FACEBACKWIDTH,
            ST_FACEBACKHEIGHT, nearestgreen, 0, false, false, NULL, NULL);

    STlib_UpdateMultIcon(&w_faces, refresh);

    STlib_UpdateMultIcon(&w_keyboxes[0], refresh);
    STlib_UpdateMultIcon(&w_keyboxes[1], refresh);
    STlib_UpdateMultIcon(&w_keyboxes[2], refresh);

    if (viewplayer->neededcardflash)
    {
        static bool showkey;
        const bool  gamepaused = (consoleactive || freeze);

        if (!gamepaused)
        {
            static uint64_t keywait;
            const uint64_t  currenttime = I_GetTimeMS();

            if (keywait < currenttime)
            {
                showkey = !showkey;
                keywait = currenttime + HUD_KEY_WAIT;
                viewplayer->neededcardflash--;

                if (showkey)
                    togglekey = !togglekey;
            }
        }

        if (flashkeys && (showkey || gamepaused))
        {
            const int   neededcard = viewplayer->neededcard;

            if (neededcard == it_allkeys)
            {
                for (int i = 0; i < NUMCARDS / 2; i++)
                    if (viewplayer->cards[i] <= 0 || viewplayer->cards[i + 3] <= 0)
                    {
                        const st_multicon_t *keybox = &w_keyboxes[i];

                        V_DrawPatch(keybox->x, keybox->y, 0, keybox->patch[i + 6]);
                    }
            }
            else
            {
                if (neededcard <= it_redcard)
                {
                    const st_multicon_t *keybox = &w_keyboxes[neededcard];

                    V_DrawPatch(keybox->x, keybox->y, 0,
                        keybox->patch[(viewplayer->cards[neededcard + 3] > 0 ? neededcard + 6 : neededcard)]);
                }
                else
                {
                    const st_multicon_t *keybox = &w_keyboxes[neededcard - 3];

                    V_DrawPatch(keybox->x, keybox->y, 0,
                        keybox->patch[(viewplayer->cards[neededcard - 3] > 0 ? neededcard + 3 : neededcard)]);
                }
            }
        }
    }
    else
        togglekey = true;
}

static void ST_DoRefresh(void)
{
    st_firsttime = false;

    // draw status bar background to off-screen buff
    if (st_statusbaron)
        ST_RefreshBackground();

    // and refresh all widgets
    ST_DrawWidgets(true);
}

static void ST_DiffDraw(void)
{
    // update all widgets
    ST_DrawWidgets(false);
}

void ST_Drawer(bool fullscreen, bool refresh)
{
    // Do red/gold-shifts from damage/items
    ST_DoPaletteStuff();

    if (r_screensize == r_screensize_max
        || (menuactive && ((messagetoprint && !consoleactive) || !messagetoprint)))
        return;

    st_statusbaron = !fullscreen;
    st_firsttime = (st_firsttime || refresh);

    // If just after ST_Start(), refresh all
    if (st_firsttime)
        ST_DoRefresh();
    else
        // Otherwise, update as little as possible
        ST_DiffDraw();
}

void ST_InitStatBar(void)
{
    // status bar background bits
    if (english == english_american)
    {
        sbar = ((FREEDOOM && !modifiedgame) || chex || hacx || harmony || REKKRSA ?
            W_CacheLastLumpName("STBAR") : (legacyofrust ? W_CacheLumpNameFromResourceWAD("STBAR") : W_CacheLumpName("STBAR")));
        sbar2 = W_CacheLumpName("STBAR2");
    }
    else
    {
        sbar = ((FREEDOOM && !modifiedgame) || chex || hacx || harmony || REKKRSA ? W_CacheLastLumpName("STBAR") :
            (W_GetNumLumps("STBAR") > 2 ? W_CacheLumpName("STBAR") : W_CacheLumpName("STBAR3")));
        sbar2 = W_CacheLumpName("STBAR4");
    }

    sbarwidth = SHORT(sbar->width);
    sbar2width = SHORT(sbar2->width);

    sbar->leftoffset = 0;
    sbar->topoffset = 0;
    sbar2->leftoffset = 0;
    sbar2->topoffset = 0;

    if (legacyofrust)
    {
        ammobg = W_CacheLumpName("STAMMO");
        ammobg2 = W_CacheLumpName("STAMMO2");

        ammobg2width = SHORT(ammobg2->width);
    }
}

static void ST_LoadUnloadGraphics(void callback(const char *, patch_t **))
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

    tallnum0width = SHORT(tallnum[0]->width);
    tallnum1width = SHORT(tallnum[1]->width);

    callback("STTPRCNT", &tallpercent);

    emptytallpercent = V_IsEmptyPatch(tallpercent);
    tallpercentwidth = (emptytallpercent ? 0 : SHORT(tallpercent->width));

    // key cards
    for (int i = 0; i < NUMCARDS + 3; i++)
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

    ST_InitStatBar();

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
    callback("STFDEAD0", &faces[facenum++]);

    // [FG] support face gib animations as in the 3DO/Jaguar/PSX ports
    for (xdthfaces = 0; xdthfaces <= ST_NUMXDTHFACES; xdthfaces++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "STFXDTH%i", xdthfaces);

        if (W_CheckNumForName(namebuf) != -1)
            faces[facenum++] = W_CacheLumpName(namebuf);
        else
            break;
    }

    // back screen
    callback((gamemode == commercial ? "GRNROCK" : "FLOOR7_2"), (patch_t **)&grnrock);
    callback("BRDR_T", &brdr_t);
    callback("BRDR_B", &brdr_b);
    callback("BRDR_L", &brdr_l);
    callback("BRDR_R", &brdr_r);
    callback("BRDR_TL", &brdr_tl);
    callback("BRDR_TR", &brdr_tr);
    callback("BRDR_BL", &brdr_bl);
    callback("BRDR_BR", &brdr_br);

    // [BH] fix display of viewborder for WADs that have these patches without offsets
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

static void ST_LoadCallback(const char *lumpname, patch_t **variable)
{
    if (M_StringCompare(lumpname, "STARMS") || M_StringCompare(lumpname, "STFGOD0"))
        *variable = ((FREEDOOM && !modifiedgame) || chex || hacx || harmony || REKKRSA ?
            W_CacheLastLumpName(lumpname) : W_CacheLumpName(lumpname));
    else
        *variable = W_CacheLumpName(lumpname);
}

static void ST_InitData(void)
{
    st_firsttime = true;
    st_faceindex = 0;
    st_palette = -1;
    st_oldhealth = -1;

    for (int i = 0; i < NUMWEAPONS; i++)
        oldweaponsowned[i] = viewplayer->weaponowned[i];

    keyboxes[0] = -1;
    keyboxes[1] = -1;
    keyboxes[2] = -1;
}

static void ST_CreateWidgets(void)
{
    // ready weapon ammo
    STlib_InitNum(&w_ready, ST_AMMOX, ST_AMMOY, tallnum,
        &viewplayer->ammo[weaponinfo[viewplayer->readyweapon].ammotype], ST_AMMOWIDTH);

    // the last weapon type
    w_ready.data = viewplayer->readyweapon;

    // health percentage
    STlib_InitPercent(&w_health, ST_HEALTHX, ST_HEALTHY, tallnum,
        &viewplayer->health, tallpercent);

    // weapons owned
    STlib_InitMultIcon(&w_arms[0], ST_ARMSX, ST_ARMSY,
        arms[0], &viewplayer->weaponowned[wp_pistol]);
    STlib_InitMultIcon(&w_arms[1], ST_ARMSX + ST_ARMSXSPACE, ST_ARMSY,
        arms[1], &st_shotguns);
    STlib_InitMultIcon(&w_arms[2], ST_ARMSX + 2 * ST_ARMSXSPACE, ST_ARMSY,
        arms[2], &viewplayer->weaponowned[wp_chaingun]);
    STlib_InitMultIcon(&w_arms[3], ST_ARMSX, ST_ARMSY + ST_ARMSYSPACE,
        arms[3], &viewplayer->weaponowned[wp_missile]);

    if (gamemode != shareware)
    {
        STlib_InitMultIcon(&w_arms[4], ST_ARMSX + ST_ARMSXSPACE, ST_ARMSY + ST_ARMSYSPACE,
            arms[4], &viewplayer->weaponowned[wp_plasma]);
        STlib_InitMultIcon(&w_arms[5], ST_ARMSX + 2 * ST_ARMSXSPACE, ST_ARMSY + ST_ARMSYSPACE,
            arms[5], &viewplayer->weaponowned[wp_bfg]);
    }

    // faces
    STlib_InitMultIcon(&w_faces, ST_FACESX, ST_FACESY, faces, &st_faceindex);

    // armor percentage
    STlib_InitPercent(&w_armor, ST_ARMORX + (int)KDIKDIZD, ST_ARMORY, tallnum,
        &viewplayer->armor, tallpercent);

    // keyboxes 0-2
    STlib_InitMultIcon(&w_keyboxes[0], ST_KEY0X + (STBARs >= 3 && !legacyofrust), ST_KEY0Y, keys, &keyboxes[0]);
    STlib_InitMultIcon(&w_keyboxes[1], ST_KEY1X + (STBARs >= 3 && !legacyofrust), ST_KEY1Y, keys, &keyboxes[1]);
    STlib_InitMultIcon(&w_keyboxes[2], ST_KEY2X + (STBARs >= 3 && !legacyofrust), ST_KEY2Y, keys, &keyboxes[2]);

    // ammo count (all four kinds)
    STlib_InitNum(&w_ammo[am_clip], ST_AMMO0X, ST_AMMO0Y,
        shortnum, &viewplayer->ammo[am_clip], ST_AMMO0WIDTH);
    STlib_InitNum(&w_ammo[am_shell], ST_AMMO1X, ST_AMMO1Y,
        shortnum, &viewplayer->ammo[am_shell], ST_AMMO1WIDTH);
    STlib_InitNum(&w_ammo[am_misl], ST_AMMO2X, ST_AMMO2Y,
        shortnum, &viewplayer->ammo[am_misl], ST_AMMO2WIDTH);
    STlib_InitNum(&w_ammo[am_cell], ST_AMMO3X, ST_AMMO3Y,
        shortnum, &viewplayer->ammo[am_cell], ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_InitNum(&w_maxammo[am_clip], ST_MAXAMMO0X, ST_MAXAMMO0Y,
        shortnum, &viewplayer->maxammo[am_clip], ST_MAXAMMO0WIDTH);
    STlib_InitNum(&w_maxammo[am_shell], ST_MAXAMMO1X, ST_MAXAMMO1Y,
        shortnum, &viewplayer->maxammo[am_shell], ST_MAXAMMO1WIDTH);
    STlib_InitNum(&w_maxammo[am_misl], ST_MAXAMMO2X, ST_MAXAMMO2Y,
        shortnum, &viewplayer->maxammo[am_misl], ST_MAXAMMO2WIDTH);
    STlib_InitNum(&w_maxammo[am_cell], ST_MAXAMMO3X, ST_MAXAMMO3Y,
        shortnum, &viewplayer->maxammo[am_cell], ST_MAXAMMO3WIDTH);
}

void ST_Start(void)
{
    ST_InitData();
    ST_CreateWidgets();
}

void ST_Init(void)
{
    ST_LoadUnloadGraphics(&ST_LoadCallback);

    st_drawbrdr = (lumpinfo[W_GetNumForName("BRDR_B")]->wadfile->type == PWAD
        || lumpinfo[W_GetNumForName(gamemode == commercial ? "GRNROCK" : "FLOOR7_2")]->wadfile->type == IWAD);

    // [BH] fix evil grin being displayed when picking up first item after
    // loading save game or entering IDFA/IDKFA cheat
    for (int i = 0; i < NUMWEAPONS; i++)
        oldweaponsowned[i] = false;

    // [BH] no plasma cells in shareware
    if (gamemode == shareware)
        maxammo[am_cell] = 0;

    usesmallnums = ((!STYSNUM0 && STBARs == 2 && !harmony) || gamemode == shareware || legacyofrust);

    STLib_Init();
    ST_InitCheats();
}
