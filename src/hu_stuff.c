/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "m_misc.h"
#include "i_swap.h"
#include "i_timer.h"
#include "m_config.h"
#include "r_main.h"
#include "s_sound.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Locally used constants, shortcuts.
//
#define HU_TITLEX       0
#define HU_TITLEY       ((SCREENHEIGHT - SBARHEIGHT) / 2 - SHORT(hu_font[0]->height) - 2)

char                    chat_char;
static player_t         *plr;
patch_t                 *hu_font[HU_FONTSIZE];
static hu_textline_t    w_title;
boolean                 chat_on;
static boolean          always_off = false;
static hu_itext_t       w_inputbuffer[MAXPLAYERS];

boolean                 message_on;
boolean                 message_dontfuckwithme;
static boolean          message_nottobefuckedwith;

boolean                 idbehold = false;

static hu_stext_t       w_message;
int                     message_counter;

int M_StringWidth(char *string);

extern boolean          messages;
extern boolean          widescreen;
extern boolean          hud;
extern int              translucency;
extern int              cardsfound;
extern patch_t          *tallnum[10];
extern patch_t          *tallpercent;
extern boolean          emptytallpercent;

static boolean          headsupactive = false;

byte                    *tempscreen;
int                     hud_y;

static patch_t          *healthpatch;
static patch_t          *berserkpatch;
static patch_t          *greenarmorpatch;
static patch_t          *bluearmorpatch;

void (*hudfunc)(int, int, patch_t *, boolean);
void (*hudnumfunc)(int, int, patch_t *, boolean);
void (*godhudfunc)(int, int, patch_t *, boolean);

#define HUD_X           24 * SCREENSCALE / 2
#define HUD_Y           311 * SCREENSCALE / 2

#define HUD_HEALTH_X    HUD_X
#define HUD_HEALTH_Y    hud_y
#define HUD_HEALTH_MIN  20
#define HUD_HEALTH_TICS 20

#define HUD_AMMO_X      (HUD_HEALTH_X + 96 * SCREENSCALE / 2)
#define HUD_AMMO_Y      HUD_HEALTH_Y
#define HUD_AMMO_MIN    20
#define HUD_AMMO_TICS   20

#define HUD_KEYS_X      (HUD_HEALTH_X + 479 * SCREENSCALE / 2)
#define HUD_KEYS_Y      HUD_HEALTH_Y

#define HUD_ARMOR_X     (HUD_HEALTH_X + 506 * SCREENSCALE / 2)
#define HUD_ARMOR_Y     HUD_HEALTH_Y

#define HUD_MIN_TICS    6
#define HUD_KEY_TICS    12

static struct
{
    char        *patchname;
    int         x;
    int         y;
    patch_t     *patch;
}
ammopic[NUMAMMO] =
{
    { "CLIPA0",  0,  2, NULL },
    { "SHELA0", -5,  5, NULL },
    { "CELLA0", -8,  2, NULL },
    { "ROCKA0", -2, -6, NULL }
};

static struct
{
    char        *patchname;
    patch_t     *patch;
}
keypic[NUMCARDS] =
{
    { "BKEYB0", NULL },
    { "YKEYB0", NULL },
    { "RKEYB0", NULL },
    { "BSKUB0", NULL },
    { "YSKUB0", NULL },
    { "RSKUB0", NULL }
};

void HU_Init(void)
{
    int         i;
    int         j;
    char        buffer[9];

    // load the heads-up font
    j = HU_FONTSTART;
    for (i = 0; i < HU_FONTSIZE; i++)
    {
        M_snprintf(buffer, 9, "STCFN%.3d", j++);
        hu_font[i] = W_CacheLumpName(buffer, PU_STATIC);
    }
}

void HU_Stop(void)
{
    headsupactive = false;
}

void HU_Start(void)
{
    int         len;
    char        *s = "";

    if (headsupactive)
        HU_Stop();

    plr = &players[consoleplayer];
    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    chat_on = false;

    // create the message widget
    HUlib_initSText(&w_message, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, hu_font, HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_initTextLine(&w_title, HU_TITLEX, HU_TITLEY, hu_font, HU_FONTSTART);

    s = Z_Malloc(133, PU_STATIC, NULL);
    strcpy(s, automaptitle);

    len = strlen(s);
    while (M_StringWidth(s) > ORIGINALWIDTH - 6)
    {
        s[len - 1] = '.';
        s[len] = '.';
        s[len + 1] = '.';
        s[len + 2] = '\0';
        --len;
    }

    while (*s)
        HUlib_addCharToTextLine(&w_title, *(s++));

    headsupactive = true;

    tempscreen = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    if (translucency)
    {
        hudfunc = V_DrawTranslucentHUDPatch;
        hudnumfunc = V_DrawTranslucentHUDNumberPatch;
        godhudfunc = V_DrawTranslucentYellowHUDPatch;
    }
    else
    {
        hudfunc = V_DrawHUDPatch;
        hudnumfunc = V_DrawHUDPatch;
        godhudfunc = V_DrawYellowHUDPatch;
    }

    healthpatch = W_CacheLumpNum(W_GetNumForName("MEDIA0"), PU_CACHE);
    berserkpatch = W_CacheLumpNum(W_GetNumForName(gamemode != shareware ? "PSTRA0" : "MEDIA0"), PU_CACHE);
    greenarmorpatch = W_CacheLumpNum(W_GetNumForName("ARM1A0"), PU_CACHE);
    bluearmorpatch = W_CacheLumpNum(W_GetNumForName("ARM2A0"), PU_CACHE);

    ammopic[am_clip].patch = W_CacheLumpNum(W_GetNumForName(ammopic[am_clip].patchname), PU_CACHE);
    ammopic[am_shell].patch = W_CacheLumpNum(W_GetNumForName(ammopic[am_shell].patchname), PU_CACHE);
    if (gamemode != shareware)
        ammopic[am_cell].patch = W_CacheLumpNum(W_GetNumForName(ammopic[am_cell].patchname), PU_CACHE);
    ammopic[am_misl].patch = W_CacheLumpNum(W_GetNumForName(ammopic[am_misl].patchname), PU_CACHE);

    keypic[it_bluecard].patch = W_CacheLumpNum(W_GetNumForName(keypic[it_bluecard].patchname), PU_CACHE);
    keypic[it_yellowcard].patch = W_CacheLumpNum(W_GetNumForName(keypic[it_yellowcard].patchname), PU_CACHE);
    keypic[it_redcard].patch = W_CacheLumpNum(W_GetNumForName(keypic[it_redcard].patchname), PU_CACHE);
    if (gamemode != shareware)
    {
        keypic[it_blueskull].patch = W_CacheLumpNum(W_GetNumForName(keypic[it_blueskull].patchname), PU_CACHE);
        keypic[it_yellowskull].patch = W_CacheLumpNum(W_GetNumForName(keypic[it_yellowskull].patchname), PU_CACHE);
        keypic[it_redskull].patch = W_CacheLumpNum(W_GetNumForName(keypic[it_redskull].patchname), PU_CACHE);
    }
}

static void DrawHUDNumber(int x, int y, signed int val, boolean invert,
                          void (*hudnumfunc)(int, int, patch_t *, boolean))
{
    int         xpos = x + 8;
    int         oldval = val;

    if (val > 99)
        hudnumfunc(xpos, y, tallnum[val / 100], invert);
    val %= 100;
    xpos += 14;
    if (val > 9 || oldval > 99)
        hudnumfunc(xpos, y, tallnum[val / 10], invert);
    val %= 10;
    xpos += 14;
    hudnumfunc(xpos, y, tallnum[val], invert);
}

static void HU_DrawHUD(void)
{
    int         health = plr->mo->health;

    if (health > 0)
    {
        int             ammotype = weaponinfo[plr->readyweapon].ammo;
        int             ammo = plr->ammo[ammotype];
        int             armor = plr->armorpoints;
        int             health_x = HUD_HEALTH_X + 12;
        int             keys = 0;
        int             i = 0;
        static int      healthanimtics = 1;
        static int      ammoanimtics = 1;
        boolean         invert;
        int             invulnerability = plr->powers[pw_invulnerability];
        static boolean  healthanim = false;
        static boolean  ammoanim = false;
        patch_t         *patch;

        if (((plr->readyweapon == wp_fist && plr->pendingweapon == wp_nochange)
            || plr->pendingweapon == wp_fist) && plr->powers[pw_strength])
            patch = berserkpatch;
        else
            patch = healthpatch;

        if (health < 10)
            health_x -= 14;
        if (health < 100)
            health_x -= 14;

        invert = ((health <= HUD_HEALTH_MIN && healthanim) || health > HUD_HEALTH_MIN
            || menuactive || paused);
        if ((plr->cheats & CF_GODMODE) || invulnerability > 128 || (invulnerability & 8))
            godhudfunc(HUD_HEALTH_X - 14, HUD_HEALTH_Y - (SHORT(patch->height) - 17), patch, invert);
        else
            hudfunc(HUD_HEALTH_X - 14, HUD_HEALTH_Y - (SHORT(patch->height) - 17), patch, invert);
        DrawHUDNumber(health_x, HUD_HEALTH_Y, health, invert, hudnumfunc);
        if (!emptytallpercent)
            hudnumfunc(health_x + 50, HUD_HEALTH_Y, tallpercent, invert);

        if (health <= HUD_HEALTH_MIN && !menuactive && !paused)
        {
            if (!--healthanimtics)
            {
                healthanim = !healthanim;
                healthanimtics = MAX(HUD_MIN_TICS,
                    (int)(HUD_HEALTH_TICS * (float)health / HUD_HEALTH_MIN));
            }
        }
        else
        {
            healthanim = false;
            healthanimtics = 1;
        }

        if (plr->pendingweapon != wp_nochange)
        {
            ammotype = weaponinfo[plr->pendingweapon].ammo;
            ammo = plr->ammo[ammotype];
        }

        if (ammo && ammotype != am_noammo)
        {
            int ammopic_x = HUD_AMMO_X + ammopic[ammotype].x;
            int ammonum_x = HUD_AMMO_X + 8;

            if (ammo < 100)
            {
                ammopic_x += 7;
                ammonum_x -= 7;
            }
            if (ammo < 10)
            {
                ammopic_x += 7;
                ammonum_x -= 7;
            }

            invert = ((ammo <= HUD_AMMO_MIN && ammoanim) || ammo > HUD_AMMO_MIN
                || menuactive || paused);
            hudfunc(ammopic_x, HUD_AMMO_Y + ammopic[ammotype].y, ammopic[ammotype].patch, invert);
            DrawHUDNumber(ammonum_x, HUD_AMMO_Y, ammo, invert, hudnumfunc);

            if (ammo <= HUD_AMMO_MIN && !menuactive && !paused)
            {
                if (!--ammoanimtics)
                {
                    ammoanim = !ammoanim;
                    ammoanimtics = MAX(HUD_MIN_TICS,
                        (int)(HUD_AMMO_TICS * (float)ammo / HUD_AMMO_MIN));
                }
            }
            else
            {
                ammoanim = false;
                ammoanimtics = 1;
            }
        }

        while (i < NUMCARDS)
            if (plr->cards[i++] > 0)
                keys++;

        if (keys || plr->neededcardtics)
        {
            int                 keypic_x = HUD_KEYS_X - 20 * (keys - 1);
            static int          keyanimcounter = HUD_KEY_TICS;
            static boolean      showkey = true;

            if (!armor)
                keypic_x += 111;
            else
            {
                if (emptytallpercent)
                    keypic_x += SHORT(tallpercent->width);
                if (armor < 10)
                    keypic_x += 26;
                else if (armor < 100)
                    keypic_x += 12;
            }

            if (plr->neededcardtics)
            {
                if (gametic)
                {
                    patch_t     *patch = keypic[plr->neededcard].patch;

                    if (!menuactive && !paused)
                    {
                        plr->neededcardtics--;
                        if (!--keyanimcounter)
                        {
                            showkey = !showkey;
                            keyanimcounter = HUD_KEY_TICS;
                        }
                    }
                    if (showkey)
                        hudfunc(keypic_x - (SHORT(patch->width) + 6), HUD_KEYS_Y, patch, true);
                }
            }
            else
            {
                showkey = true;
                keyanimcounter = HUD_KEY_TICS;
            }

            for (i = 0; i < NUMCARDS; i++)
                if (plr->cards[i] > 0)
                {
                    patch_t     *patch = keypic[i].patch;

                    hudfunc(keypic_x + (SHORT(patch->width) + 6) * (cardsfound - plr->cards[i]),
                        HUD_KEYS_Y, patch, true);
                }
        }

        if (armor)
        {
            patch_t     *patch = (plr->armortype == 1 ? greenarmorpatch : bluearmorpatch);

            if (emptytallpercent)
                DrawHUDNumber(HUD_ARMOR_X + SHORT(tallpercent->width), HUD_ARMOR_Y, armor, true,
                    hudnumfunc);
            else
            {
                DrawHUDNumber(HUD_ARMOR_X, HUD_ARMOR_Y, armor, true, hudnumfunc);
                hudnumfunc(HUD_ARMOR_X + 50, HUD_ARMOR_Y, tallpercent, true);
            }
            hudfunc(HUD_ARMOR_X + 70, HUD_ARMOR_Y - (SHORT(patch->height) - 16), patch, true);
        }
    }
}

void HU_Drawer(void)
{
    w_message.l->x = (automapactive && fullscreen && !widescreen ? 0 : 3);
    w_message.l->y = HU_MSGY;
    HUlib_drawSText(&w_message);
    if (automapactive)
    {
        w_title.x = (fullscreen && !widescreen ? 0 : 3);
        HUlib_drawTextLine(&w_title);
    }
    else if ((widescreen || screensize == SCREENSIZE_MAX) && hud)
    {
        hud_y = (widescreen ? HUD_Y : HUD_Y + SBARHEIGHT);
        HU_DrawHUD();
    }
}

void HU_Erase(void)
{
    HUlib_eraseSText(&w_message);
    HUlib_eraseTextLine(&w_title);
}

extern fixed_t  m_x, m_y, m_h, m_w;
extern boolean  message_dontpause;
extern boolean  blurred;
extern boolean  inhelpscreens;
extern int      direction;

void HU_Ticker(void)
{
    static int  lasttic = -1;
    int         tic, fps;
    static char fps_str[8] = "";

    // tick down message counter if message is up
    if (((!menuactive && !paused) || inhelpscreens || message_dontpause) &&
        !idbehold && !(players[consoleplayer].cheats & CF_MYPOS) && !devparm && message_counter &&
        !--message_counter)
    {
        message_on = false;
        message_nottobefuckedwith = false;
        if (message_dontpause)
        {
            message_dontpause = false;
            blurred = false;
        }
    }

    if (idbehold)
    {
        // [BH] display message for IDBEHOLDx cheat
        if (!message_counter)
            message_counter = HU_MSGTIMEOUT;
        else if (message_counter > 132)
            message_counter--;

        HUlib_addMessageToSText(&w_message, 0, s_STSTR_BEHOLD);
        message_on = true;
    }
    else if (players[consoleplayer].cheats & CF_MYPOS)
    {
        // [BH] display and constantly update message for IDMYPOS cheat
        char    buffer[80];
        int     angle;
        int     x, y, z;

        if (!message_counter)
            message_counter = HU_MSGTIMEOUT;
        else if (message_counter > 132)
            message_counter--;

        if (automapactive && !followplayer)
        {
            angle = direction;
            x = (m_x + (m_w >> 1)) / FRACUNIT;
            y = (m_y + (m_h >> 1)) / FRACUNIT;
            z = R_PointInSubsector(m_x + (m_w >> 1), m_y + (m_h >> 1))->sector->floorheight / FRACUNIT;
        }
        else
        {
            angle = (int)((double)plr->mo->angle * (90.0f / ANG90));
            if (angle == 360)
                angle = 0;
            x = plr->mo->x / FRACUNIT;
            y = plr->mo->y / FRACUNIT;
            z = plr->mo->z / FRACUNIT;
        }

        M_snprintf(buffer, sizeof(buffer), s_STSTR_MYPOS, angle, STCFN034 ? ' ' : '*', x, y, z);
        HUlib_addMessageToSText(&w_message, 0, buffer);
        message_on = true;
    }
    else if (devparm)
    {
        // [BH] display and constantly update FPS for -DEVPARM
        tic = I_GetTime();
        fps = (lasttic == -1 ? TICRATE : TICRATE - (tic - lasttic) + 1);
        if (fps < 1)
            fps = 1;
        if (fps > TICRATE)
            fps = TICRATE;
        lasttic = tic;
        M_snprintf(fps_str, sizeof(fps_str), "%i FPS", fps);
        HUlib_addMessageToSText(&w_message, 0, fps_str);
        message_on = true;
    }
    else
    {
        // display message if necessary
        if ((plr->message && !message_nottobefuckedwith)
            || (plr->message && message_dontfuckwithme))
        {
            printf("%s\n", plr->message);
            if (messages || message_dontfuckwithme)
            {
                char    *s = Z_Malloc(133, PU_STATIC, NULL);
                int     len;

                strcpy(s, plr->message);

                len = strlen(s);
                while (M_StringWidth(s) > ORIGINALWIDTH - 6)
                {
                    s[len - 1] = '.';
                    s[len] = '.';
                    s[len + 1] = '.';
                    s[len + 2] = '\0';
                    --len;
                }

                HUlib_addMessageToSText(&w_message, 0, s);
                message_on = true;
                message_counter = HU_MSGTIMEOUT;
                message_nottobefuckedwith = message_dontfuckwithme;
                message_dontfuckwithme = 0;
            }
            plr->message = 0;
        }
    }
}

void HU_clearMessages(void)
{
    plr->message = 0;
    message_counter = 0;
    message_on = false;
    message_nottobefuckedwith = false;
    message_dontpause = false;
}
