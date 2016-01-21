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

#include <ctype.h>

#include "am_map.h"
#include "c_console.h"
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
#define HU_TITLEX       (vid_fullscreen && !vid_widescreen ? 0 : 3)
#define HU_TITLEY       (ORIGINALHEIGHT - 32 * (r_screensize < r_screensize_max) - 9)

static player_t         *plr;
patch_t                 *hu_font[HU_FONTSIZE];
static hu_textline_t    w_title;

dboolean                message_on;
dboolean                message_dontfuckwithme;
dboolean                message_clearable = false;
static dboolean         message_nottobefuckedwith;

#define STSTR_BEHOLD2   "inVuln, bSrk, Inviso, Rad, Allmap or Lite-amp?"

dboolean                idbehold = false;
dboolean                s_STSTR_BEHOLD2;

static hu_stext_t       w_message;
int                     message_counter;

char                    *playername = playername_default;

int M_StringWidth(char *string);

static dboolean         headsupactive;

byte                    *tempscreen;
int                     hudnumoffset;

static patch_t          *healthpatch;
static patch_t          *berserkpatch;
static patch_t          *greenarmorpatch;
static patch_t          *bluearmorpatch;

dboolean                r_althud = r_althud_default;
dboolean                r_diskicon = r_diskicon_default;
dboolean                r_hud = r_hud_default;

static patch_t          *stdisk;
dboolean                drawdisk = false;

extern dboolean         messages;
extern dboolean         vid_widescreen;
extern dboolean         r_translucency;
extern int              cardsfound;
extern patch_t          *tallnum[10];
extern patch_t          *tallpercent;
extern dboolean         emptytallpercent;

void (*hudfunc)(int, int, patch_t *, byte *);
void (*hudnumfunc)(int, int, patch_t *, byte *);
void (*godhudfunc)(int, int, patch_t *, byte *);

static struct
{
    char        *patchname;
    int         mobjnum;
    int         x;
    int         y;
    patch_t     *patch;
} ammopic[NUMAMMO] = {
    { "CLIPA0", MT_CLIP,   8,  2, NULL },
    { "SHELA0", MT_MISC22, 5,  5, NULL },
    { "CELLA0", MT_MISC20, 0,  2, NULL },
    { "ROCKA0", MT_MISC18, 8, -6, NULL }
};

static struct
{
    char        *patchnamea;
    char        *patchnameb;
    patch_t     *patch;
} keypic[NUMCARDS] = {
    { "BKEYA0", "BKEYB0", NULL },
    { "YKEYA0", "YKEYB0", NULL },
    { "RKEYA0", "RKEYB0", NULL },
    { "BSKUA0", "BSKUB0", NULL },
    { "YSKUA0", "YSKUB0", NULL },
    { "RSKUA0", "RSKUB0", NULL }
};

void HU_AltInit(void);

patch_t *HU_LoadHUDAmmoPatch(int ammopicnum)
{
    int lump;

    if ((mobjinfo[ammopic[ammopicnum].mobjnum].flags & MF_SPECIAL)
        && (lump = W_CheckNumForName(ammopic[ammopicnum].patchname)) >= 0)
        return W_CacheLumpNum(lump, PU_CACHE);
    else
        return NULL;
}

patch_t *HU_LoadHUDKeyPatch(int keypicnum)
{
    int lump;

    if (dehacked && (lump = W_CheckNumForName(keypic[keypicnum].patchnamea)) >= 0)
        return W_CacheLumpNum(lump, PU_CACHE);
    else if ((lump = W_CheckNumForName(keypic[keypicnum].patchnameb)) >= 0)
        return W_CacheLumpNum(lump, PU_CACHE);
    else
        return NULL;
}

void HU_Init(void)
{
    int         i;
    int         j;
    int         lump;
    char        buffer[9];

    // load the heads-up font
    j = HU_FONTSTART;
    for (i = 0; i < HU_FONTSIZE; i++)
    {
        M_snprintf(buffer, 9, "STCFN%.3d", j++);
        hu_font[i] = W_CacheLumpName(buffer, PU_STATIC);
    }

    tempscreen = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    if ((lump = W_CheckNumForName("MEDIA0")) >= 0)
        healthpatch = W_CacheLumpNum(lump, PU_CACHE);
    if ((lump = W_CheckNumForName("PSTRA0")) >= 0)
        berserkpatch = W_CacheLumpNum(lump, PU_CACHE);
    else
        berserkpatch = healthpatch;
    if ((lump = W_CheckNumForName("ARM1A0")) >= 0)
        greenarmorpatch = W_CacheLumpNum(lump, PU_CACHE);
    if ((lump = W_CheckNumForName("ARM2A0")) >= 0)
        bluearmorpatch = W_CacheLumpNum(lump, PU_CACHE);

    ammopic[am_clip].patch = HU_LoadHUDAmmoPatch(am_clip);
    ammopic[am_shell].patch = HU_LoadHUDAmmoPatch(am_shell);
    if (gamemode != shareware)
        ammopic[am_cell].patch = HU_LoadHUDAmmoPatch(am_cell);
    ammopic[am_misl].patch = HU_LoadHUDAmmoPatch(am_misl);

    keypic[it_bluecard].patch = HU_LoadHUDKeyPatch(it_bluecard);
    keypic[it_yellowcard].patch = HU_LoadHUDKeyPatch(hacx ? it_yellowskull : it_yellowcard);
    keypic[it_redcard].patch = HU_LoadHUDKeyPatch(it_redcard);
    if (gamemode != shareware)
    {
        keypic[it_blueskull].patch = HU_LoadHUDKeyPatch(it_blueskull);
        keypic[it_yellowskull].patch = HU_LoadHUDKeyPatch(it_yellowskull);
        keypic[it_redskull].patch = HU_LoadHUDKeyPatch(it_redskull);
    }

    if (r_translucency)
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

    if ((lump = W_CheckNumForName("STDISK")) >= 0)
        stdisk = W_CacheLumpNum(lump, PU_CACHE);

    s_STSTR_BEHOLD2 = M_StringCompare(s_STSTR_BEHOLD, STSTR_BEHOLD2);

    if (!M_StringCompare(playername, playername_default))
        s_GOTMEDINEED = s_GOTMEDINEED2;

    HU_AltInit();
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

    plr = &players[0];
    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;

    // create the message widget
    HUlib_initSText(&w_message, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, hu_font, HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_initTextLine(&w_title, HU_TITLEX, HU_TITLEY, hu_font, HU_FONTSTART);

    s = Z_Malloc(133, PU_STATIC, NULL);
    strcpy(s, automaptitle);

    len = strlen(s);
    while (M_StringWidth(s) > ORIGINALWIDTH - 6)
    {
        s[len - 1] = s[len] = s[len + 1] = '.';
        s[len + 2] = '\0';
        --len;
    }

    while (*s)
        HUlib_addCharToTextLine(&w_title, *(s++));

    headsupactive = true;

    hudnumoffset = (16 - SHORT(tallnum[0]->height)) / 2;
}

static void DrawHUDNumber(int *x, int y, int val, byte *tinttab,
                          void (*hudnumfunc)(int, int, patch_t *, byte *))
{
    int         oldval = val;
    patch_t     *patch;

    if (val > 99)
    {
        patch = tallnum[val / 100];
        hudnumfunc(*x, y, patch, tinttab);
        *x += SHORT(patch->width);
    }
    val %= 100;
    if (val > 9 || oldval > 99)
    {
        patch = tallnum[val / 10];
        hudnumfunc(*x, y, patch, tinttab);
        *x += SHORT(patch->width);
    }
    val %= 10;
    patch = tallnum[val];
    hudnumfunc(*x, y, patch, tinttab);
    *x += SHORT(patch->width);
}

static int HUDNumberWidth(int val)
{
    int oldval = val;
    int width = 0;

    if (val > 99)
        width += SHORT(tallnum[val / 100]->width);
    val %= 100;
    if (val > 9 || oldval > 99)
        width += SHORT(tallnum[val / 10]->width);
    val %= 10;
    width += SHORT(tallnum[val]->width);
    return width;
}

int     healthhighlight = 0;
int     ammohighlight = 0;
int     armorhighlight = 0;

static void HU_DrawHUD(void)
{
    int                 health = MAX(0, plr->health);
    weapontype_t        pendingweapon = plr->pendingweapon;
    weapontype_t        readyweapon = plr->readyweapon;
    int                 ammotype = weaponinfo[readyweapon].ammo;
    int                 ammo = plr->ammo[ammotype];
    int                 armor = plr->armorpoints;
    int                 health_x = HUD_HEALTH_X;
    int                 keys = 0;
    int                 i = 0;
    byte                *tinttab;
    int                 invulnerability = plr->powers[pw_invulnerability];
    static dboolean     healthanim;
    patch_t             *patch;
    dboolean            gamepaused = (menuactive || paused || consoleactive);
    int                 currenttime = I_GetTimeMS();

    tinttab = (!health || (health <= HUD_HEALTH_MIN && healthanim) || health > HUD_HEALTH_MIN ?
        tinttab66 : tinttab25);

    patch = (((readyweapon == wp_fist && pendingweapon == wp_nochange) || pendingweapon == wp_fist)
        && plr->powers[pw_strength] ? berserkpatch : healthpatch);

    if (patch)
    {
        if ((plr->cheats & CF_GODMODE) || invulnerability > 128 || (invulnerability & 8))
            godhudfunc(health_x, HUD_HEALTH_Y - (SHORT(patch->height) - 17), patch, tinttab);
        else
            hudfunc(health_x, HUD_HEALTH_Y - (SHORT(patch->height) - 17), patch, tinttab);

        health_x += SHORT(patch->width) + 8;
    }

    if (healthhighlight > currenttime)
    {
        DrawHUDNumber(&health_x, HUD_HEALTH_Y + hudnumoffset, health, tinttab,
            V_DrawHighlightedHUDNumberPatch);

        if (!emptytallpercent)
            V_DrawHighlightedHUDNumberPatch(health_x, HUD_HEALTH_Y + hudnumoffset, tallpercent,
                tinttab);
    }
    else
    {
        DrawHUDNumber(&health_x, HUD_HEALTH_Y + hudnumoffset, health, tinttab, hudnumfunc);

        if (!emptytallpercent)
            hudnumfunc(health_x, HUD_HEALTH_Y + hudnumoffset, tallpercent, tinttab);
    }

    if (!gamepaused)
    {
        static int      healthwait;

        if (health <= HUD_HEALTH_MIN)
        {
            if (healthwait < currenttime)
            {
                healthanim = !healthanim;
                healthwait = currenttime + HUD_HEALTH_WAIT * health / HUD_HEALTH_MIN + 115;
            }
        }
        else
        {
            healthanim = false;
            healthwait = 0;
        }
    }

    if (pendingweapon != wp_nochange)
    {
        ammotype = weaponinfo[pendingweapon].ammo;
        ammo = plr->ammo[ammotype];
    }

    if (health && ammo && ammotype != am_noammo)
    {
        int                 ammo_x = HUD_AMMO_X + ammopic[ammotype].x;
        static dboolean     ammoanim;

        tinttab = ((ammo <= HUD_AMMO_MIN && ammoanim) || ammo > HUD_AMMO_MIN ? tinttab66 :
            tinttab25);

        patch = ammopic[ammotype].patch;
        if (patch)
        {
            hudfunc(ammo_x, HUD_AMMO_Y + ammopic[ammotype].y, patch, tinttab);
            ammo_x += SHORT(patch->width) + 8;
        }

        if (ammohighlight > currenttime)
            DrawHUDNumber(&ammo_x, HUD_AMMO_Y + hudnumoffset, ammo, tinttab,
                V_DrawHighlightedHUDNumberPatch);
        else
            DrawHUDNumber(&ammo_x, HUD_AMMO_Y + hudnumoffset, ammo, tinttab, hudnumfunc);

        if (!gamepaused)
        {
            static int  ammowait;

            if (ammo <= HUD_AMMO_MIN)
            {
                if (ammowait < currenttime)
                {
                    ammoanim = !ammoanim;
                    ammowait = currenttime + HUD_AMMO_WAIT * ammo / HUD_AMMO_MIN + 115;
                }
            }
            else
            {
                ammoanim = false;
                ammowait = 0;
            }
        }
    }

    while (i < NUMCARDS)
        if (plr->cards[i++] > 0)
            keys++;

    if (keys || plr->neededcardflash)
    {
        int                 keypic_x = HUD_KEYS_X - 20 * (keys - 1);
        static int          keywait;
        static dboolean     showkey;

        if (!armor)
            keypic_x += 114;
        else
        {
            if (emptytallpercent)
                keypic_x += SHORT(tallpercent->width);

            if (armor < 10)
                keypic_x += 26;
            else if (armor < 100)
                keypic_x += 12;
        }

        if (plr->neededcardflash)
        {
            patch_t     *patch = keypic[plr->neededcard].patch;

            if (patch)
            {
                if (!gamepaused && keywait < currenttime)
                {
                    showkey = !showkey;
                    keywait = currenttime + HUD_KEY_WAIT;
                    plr->neededcardflash--;
                }

                if (showkey)
                    hudfunc(keypic_x - (SHORT(patch->width) + 6), HUD_KEYS_Y, patch, tinttab66);
            }
        }
        else
        {
            showkey = false;
            keywait = 0;
        }

        for (i = 0; i < NUMCARDS; i++)
            if (plr->cards[i] > 0)
            {
                patch_t     *patch = keypic[i].patch;

                if (patch)
                    hudfunc(keypic_x + (SHORT(patch->width) + 6) * (cardsfound - plr->cards[i]),
                        HUD_KEYS_Y, patch, tinttab66);
            }
    }

    if (armor)
    {
        patch_t     *patch = (plr->armortype == GREENARMOR ? greenarmorpatch : bluearmorpatch);
        int         armor_x = HUD_ARMOR_X;

        if (patch)
        {
            armor_x -= SHORT(patch->width);
            hudfunc(armor_x, HUD_ARMOR_Y - (SHORT(patch->height) - 16), patch, tinttab66);
            armor_x -= 7;
        }

        if (armorhighlight > currenttime)
        {
            if (emptytallpercent)
            {
                armor_x -= HUDNumberWidth(armor);
                DrawHUDNumber(&armor_x, HUD_ARMOR_Y + hudnumoffset, armor, tinttab66,
                    V_DrawHighlightedHUDNumberPatch);
            }
            else
            {
                armor_x -= SHORT(tallpercent->width);
                V_DrawHighlightedHUDNumberPatch(armor_x, HUD_ARMOR_Y + hudnumoffset, tallpercent,
                    tinttab66);
                armor_x -= HUDNumberWidth(armor);
                DrawHUDNumber(&armor_x, HUD_ARMOR_Y + hudnumoffset, armor, tinttab66,
                    V_DrawHighlightedHUDNumberPatch);
            }
        }
        else if (emptytallpercent)
        {
            armor_x -= HUDNumberWidth(armor);
            DrawHUDNumber(&armor_x, HUD_ARMOR_Y + hudnumoffset, armor, tinttab66, hudnumfunc);
        }
        else
        {
            armor_x -= SHORT(tallpercent->width);
            hudnumfunc(armor_x, HUD_ARMOR_Y + hudnumoffset, tallpercent, tinttab66);
            armor_x -= HUDNumberWidth(armor);
            DrawHUDNumber(&armor_x, HUD_ARMOR_Y + hudnumoffset, armor, tinttab66, hudnumfunc);
        }
    }
}

#define ALTHUD_LEFT_X   22
#define ALTHUD_RIGHT_X  459
#define ALTHUD_Y        300

#define WHITE           4
#define GRAY            98
#define GREEN           114
#define RED             180
#define BLUE            200
#define YELLOW          231

typedef struct
{
    int         color;
    patch_t     *patch;
} altkeypic_t;

altkeypic_t altkeypics[NUMCARDS] =
{
    { BLUE   },
    { YELLOW },
    { RED    },
    { BLUE   },
    { YELLOW },
    { RED    }
};

static patch_t  *altnum[10];
static patch_t  *altweapon[NUMWEAPONS];
static patch_t  *altendpatch;
static patch_t  *altleftpatch;
static patch_t  *altrightpatch;
static patch_t  *altmarkpatch;
static patch_t  *altkeypatch;
static patch_t  *altskullpatch;

void HU_AltInit(void)
{
    int         i;
    char        buffer[9];

    for (i = 0; i < 10; i++)
    {
        M_snprintf(buffer, 9, "DRHUD%i", i);
        altnum[i] = W_CacheLumpName(buffer, PU_STATIC);
    }

    for (i = 1; i < NUMWEAPONS; i++)
    {
        M_snprintf(buffer, 9, "DRHUDWP%i", i);
        altweapon[i] = W_CacheLumpName(buffer, PU_STATIC);
    }

    altleftpatch = W_CacheLumpName("DRHUDL", PU_CACHE);
    altrightpatch = W_CacheLumpName("DRHUDR", PU_CACHE);

    altendpatch = W_CacheLumpName("DRHUDE", PU_CACHE);
    altmarkpatch = W_CacheLumpName("DRHUDI", PU_CACHE);

    altkeypatch = W_CacheLumpName("DRHUDKEY", PU_CACHE);
    altskullpatch = W_CacheLumpName("DRHUDSKU", PU_CACHE);
    altkeypics[0].patch = altkeypatch;
    altkeypics[1].patch = altkeypatch;
    altkeypics[2].patch = altkeypatch;
    altkeypics[3].patch = altskullpatch;
    altkeypics[4].patch = altskullpatch;
    altkeypics[5].patch = altskullpatch;
}

static void DrawAltHUDNumber(int x, int y, int val)
{
    int         oldval = val;
    patch_t     *patch;

    if (val > 99)
    {
        patch = altnum[val / 100];
        V_DrawAltHUDPatch(x, y, patch, 0, 0);
        x += SHORT(patch->width) + 2;
    }
    val %= 100;
    if (val > 9 || oldval > 99)
    {
        patch = altnum[val / 10];
        V_DrawAltHUDPatch(x, y, patch, 0, 0);
        x += SHORT(patch->width) + 2;
    }
    V_DrawAltHUDPatch(x, y, altnum[val % 10], 0, 0);
}

static int AltHUDNumberWidth(int val)
{
    int oldval = val;
    int width = 0;

    if (val > 99)
        width += SHORT(altnum[val / 100]->width) + 2;
    val %= 100;
    if (val > 9 || oldval > 99)
        width += SHORT(altnum[val / 10]->width) + 2;
    return (width + SHORT(altnum[val % 10]->width));
}

static void HU_DrawAltHUD(void)
{
    int health = MAX(0, plr->health);
    int armor = plr->armorpoints;
    int color = (health <= 20 ? RED : (health >= 100 ? GREEN : WHITE));
    int keys = 0;
    int i = 0;

    DrawAltHUDNumber(ALTHUD_LEFT_X + 34 - AltHUDNumberWidth(health), ALTHUD_Y + 12, health);
    health = MIN(health, 100);
    V_FillTransRect(ALTHUD_LEFT_X + 58, ALTHUD_Y + 13, health + 1, 8, color);
    V_DrawAltHUDPatch(ALTHUD_LEFT_X + 40, ALTHUD_Y + 1, altleftpatch, 0, 0);
    V_DrawAltHUDPatch(ALTHUD_LEFT_X + 58, ALTHUD_Y + 13, altendpatch, WHITE, color);
    V_DrawAltHUDPatch(ALTHUD_LEFT_X + 58 + health - 2, ALTHUD_Y + 13, altmarkpatch, WHITE, color);

    if (health < 100)
        V_DrawAltHUDPatch(ALTHUD_LEFT_X + 158, ALTHUD_Y + 13, altendpatch, 0, 0);

    if (armor)
        if (plr->armortype == GREENARMOR)
            V_FillTransRect(ALTHUD_LEFT_X + 58, ALTHUD_Y + 2, MIN(armor, 100) + 1, 6, GRAY);
        else
            V_FillTransRect(ALTHUD_LEFT_X + 58, ALTHUD_Y + 2, armor / 2 + 1, 6, GRAY);

    if (health)
    {
        weapontype_t    pendingweapon = plr->pendingweapon;
        weapontype_t    weapon = (pendingweapon != wp_nochange ? pendingweapon : plr->readyweapon);
        ammotype_t      ammotype = weaponinfo[weapon].ammo;

        if (ammotype != am_noammo)
        {
            int ammo = plr->ammo[ammotype];

            DrawAltHUDNumber(ALTHUD_RIGHT_X + 101 - AltHUDNumberWidth(ammo), ALTHUD_Y - 1, ammo);
            ammo = 100 * ammo / plr->maxammo[ammotype];
            color = (ammo <= 15 ? YELLOW : WHITE);
            V_FillTransRect(ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1, 8, color);
            V_DrawAltHUDPatch(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, 0, 0);
            V_DrawAltHUDPatch(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, color);
            V_DrawAltHUDPatch(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 13, altmarkpatch, WHITE,
                color);

            if (ammo < 100)
                V_DrawAltHUDPatch(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altendpatch, 0, 0);
        }
        else
        {
            V_DrawAltHUDPatch(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, 0, 0);
            V_DrawAltHUDPatch(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, 0, 0);
            V_DrawAltHUDPatch(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altendpatch, 0, 0);
        }

        if (weapon)
            V_DrawAltHUDPatch(ALTHUD_RIGHT_X + 107, ALTHUD_Y - 15, altweapon[weapon], 0, 0);
    }
    else
    {
        V_DrawAltHUDPatch(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, 0, 0);
        V_DrawAltHUDPatch(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, 0, 0);
        V_DrawAltHUDPatch(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altendpatch, 0, 0);
    }

    while (i < NUMCARDS)
        if (plr->cards[i++] > 0)
            ++keys;

    if (keys || plr->neededcardflash)
    {
        static int          keywait;
        static dboolean     showkey;

        if (plr->neededcardflash)
        {
            if (!(menuactive || paused || consoleactive))
            {
                int currenttime = I_GetTimeMS();

                if (keywait < currenttime)
                {
                    showkey = !showkey;
                    keywait = currenttime + HUD_KEY_WAIT;
                    plr->neededcardflash--;
                }
            }

            if (showkey)
            {
                altkeypic_t     altkeypic = altkeypics[plr->neededcard];

                V_DrawAltHUDPatch(ALTHUD_RIGHT_X + 10 * cardsfound, ALTHUD_Y, altkeypic.patch,
                    WHITE, altkeypic.color);
            }
        }
        else
        {
            showkey = false;
            keywait = 0;
        }

        for (i = 0; i < NUMCARDS; i++)
        {
            int card = plr->cards[i];

            if (card > 0)
            {
                altkeypic_t    altkeypic = altkeypics[i];

                V_DrawAltHUDPatch(ALTHUD_RIGHT_X + 10 * (card - 1), ALTHUD_Y, altkeypic.patch,
                    WHITE, altkeypic.color);
            }
        }
    }
}

void HU_DrawDisk(void)
{
    if (r_diskicon && stdisk)
    {
        V_DrawBigPatch(SCREENWIDTH - HU_MSGX * SCREENSCALE - SHORT(stdisk->width),
            HU_MSGY * SCREENSCALE, 0, stdisk);
        if (!consoleheight)
            blitfunc();
    }
}

void HU_Drawer(void)
{
    w_message.l->x = HU_MSGX;
    w_message.l->y = HU_MSGY;
    HUlib_drawSText(&w_message);
    if (automapactive)
    {
        w_title.x = HU_TITLEX;
        w_title.y = HU_TITLEY;
        HUlib_drawTextLine(&w_title, false);
    }
    else
    {
        if (vid_widescreen && r_hud)
            if (r_althud)
                HU_DrawAltHUD();
            else
                HU_DrawHUD();

        if (mapwindow && realframe)
            HUlib_drawTextLine(&w_title, true);
    }

    if (drawdisk)
        HU_DrawDisk();
}

void HU_Erase(void)
{
    HUlib_eraseSText(&w_message);
    HUlib_eraseTextLine(&w_title);
}

extern fixed_t  m_x, m_y, m_h, m_w;
extern dboolean message_dontpause;
extern dboolean inhelpscreens;
extern int      direction;

void HU_Ticker(void)
{
    dboolean    idmypos = (players[0].cheats & CF_MYPOS);

    // tick down message counter if message is up
    if (((!menuactive && !paused) || inhelpscreens || message_dontpause) && !idbehold && !idmypos
        && message_counter && !--message_counter)
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
    else if (idmypos)
    {
        // [BH] display and constantly update message for IDMYPOS cheat
        char    buffer[80];

        if (!message_counter)
            message_counter = HU_MSGTIMEOUT;
        else if (message_counter > 132)
            message_counter--;

        if (automapactive && !am_followmode)
        {
            int         x = (m_x + m_w / 2) >> MAPBITS;
            int         y = (m_y + m_h / 2) >> MAPBITS;
            sector_t    *sector = R_PointInSubsector(x, y)->sector;

            M_snprintf(buffer, sizeof(buffer), s_STSTR_MYPOS, direction, x, y,
                sector->floorheight / FRACUNIT);
        }
        else
        {
            int         angle = (int)((double)viewangle * (90.0f / ANG90));

            M_snprintf(buffer, sizeof(buffer), s_STSTR_MYPOS, (angle == 360 ? 0 : angle),
                viewx / FRACUNIT, viewy / FRACUNIT, plr->mo->z / FRACUNIT);
        }

        HUlib_addMessageToSText(&w_message, 0, buffer);
        message_on = true;
    }

    // display message if necessary
    if ((plr->message && !message_nottobefuckedwith)
        || (plr->message && message_dontfuckwithme))
    {
        if (!idbehold && !idmypos && (messages || message_dontfuckwithme))
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
            message_dontfuckwithme = false;

            Z_Free(s);
        }
        plr->message = 0;
    }
}

void HU_PlayerMessage(char *message, dboolean ingame)
{
    static char buffer[1024];
    char        lastchar;

    if (message[0] == '%' && message[1] == 's')
        M_snprintf(buffer, sizeof(buffer), message, playername);
    else
        M_StringCopy(buffer, message, sizeof(buffer));

    buffer[0] = toupper(buffer[0]);
    lastchar = buffer[strlen(buffer) - 1];

    if (plr && !consoleactive && !message_dontfuckwithme)
        plr->message = buffer;

    if (ingame)
        C_PlayerMessage("%s%s", buffer, (lastchar == '.' || lastchar == '!' ? "" : "."));
    else
        C_Output("%s%s", buffer, (lastchar == '.' || lastchar == '!' ? "" : "."));
}

void HU_ClearMessages(void)
{
    if ((idbehold || (plr->cheats & CF_MYPOS)) && !message_clearable)
        return;

    plr->message = 0;
    message_counter = 0;
    message_on = false;
    message_nottobefuckedwith = false;
    message_dontfuckwithme = false;
    message_dontpause = false;
    message_clearable = false;
}
