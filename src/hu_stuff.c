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

#include <ctype.h>

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "m_misc.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_config.h"
#include "p_local.h"
#include "r_main.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Locally used constants, shortcuts.
//
#define HU_TITLEX       3
#define HU_TITLEY       (ORIGINALHEIGHT - ORIGINALSBARHEIGHT - hu_font[0]->height - 2)
#define STSTR_BEHOLD2   "inVuln, bSrk, Inviso, Rad, Allmap or Lite-amp?"

patch_t                 *hu_font[HU_FONTSIZE];
static hu_textline_t    w_title;

dboolean                message_on;
dboolean                message_dontfuckwithme;
dboolean                message_clearable;
static dboolean         message_external;
static dboolean         message_nottobefuckedwith;
static int              message_x;
static int              message_y;

dboolean                idbehold;
dboolean                s_STSTR_BEHOLD2;

static hu_stext_t       w_message;
static int              message_counter;

int M_StringWidth(char *string);

static dboolean         headsupactive;

byte                    *tempscreen;

static patch_t          *minuspatch;
static short            minuspatchwidth;
static patch_t          *greenarmorpatch;
static patch_t          *bluearmorpatch;

char                    *playername = playername_default;
dboolean                r_althud = r_althud_default;
dboolean                r_diskicon = r_diskicon_default;
dboolean                r_hud = r_hud_default;
dboolean                r_hud_translucency = r_hud_translucency_default;
int                     r_messagescale = r_messagescale_default;
char                    *r_messagepos = r_messagepos_default;

static patch_t          *stdisk;
static short            stdiskwidth;
dboolean                drawdisk;

static int              coloroffset;

extern int              cardsfound;
extern patch_t          *tallnum[10];
extern patch_t          *tallpercent;
extern short            tallpercentwidth;
extern dboolean         emptytallpercent;
extern int              caretcolor;
extern patch_t          *faces[ST_NUMFACES];
extern int              st_faceindex;

static void (*hudfunc)(int, int, patch_t *, byte *);
static void (*hudnumfunc)(int, int, patch_t *, byte *);
static void (*godhudfunc)(int, int, patch_t *, byte *);

static void (*althudfunc)(int, int, patch_t *, int, int);
static void (*fillrectfunc)(int, int, int, int, int, int, dboolean);

static struct
{
    char    *patchname;
    int     mobjnum;
    patch_t *patch;
} ammopic[NUMAMMO] = {
    { "CLIPA0", MT_CLIP,   NULL },
    { "SHELA0", MT_MISC22, NULL },
    { "CELLA0", MT_MISC20, NULL },
    { "ROCKA0", MT_MISC18, NULL }
};

static struct
{
    char    *patchnamea;
    char    *patchnameb;
    patch_t *patch;
} keypics[NUMCARDS] = {
    { "BKEYA0", "BKEYB0", NULL },
    { "YKEYA0", "YKEYB0", NULL },
    { "RKEYA0", "RKEYB0", NULL },
    { "BSKUA0", "BSKUB0", NULL },
    { "YSKUA0", "YSKUB0", NULL },
    { "RSKUA0", "RSKUB0", NULL }
};

static void HU_AltInit(void);

static patch_t *HU_LoadHUDAmmoPatch(int ammopicnum)
{
    int lump;

    if ((mobjinfo[ammopic[ammopicnum].mobjnum].flags & MF_SPECIAL)
        && (lump = W_CheckNumForName(ammopic[ammopicnum].patchname)) >= 0)
        return W_CacheLumpNum(lump);
    else
        return NULL;
}

static patch_t *HU_LoadHUDKeyPatch(int keypicnum)
{
    int lump;

    if (dehacked && (lump = W_CheckNumForName(keypics[keypicnum].patchnamea)) >= 0)
        return W_CacheLumpNum(lump);
    else if ((lump = W_CheckNumForName(keypics[keypicnum].patchnameb)) >= 0)
        return W_CacheLumpNum(lump);
    else
        return NULL;
}

void HU_SetTranslucency(void)
{
    if (r_hud_translucency)
    {
        hudfunc = V_DrawTranslucentHUDPatch;
        hudnumfunc = V_DrawTranslucentHUDNumberPatch;
        godhudfunc = V_DrawTranslucentYellowHUDPatch;
        althudfunc = V_DrawTranslucentAltHUDPatch;
        fillrectfunc = V_FillTransRect;
        coloroffset = 0;
    }
    else
    {
        hudfunc = V_DrawHUDPatch;
        hudnumfunc = V_DrawHUDPatch;
        godhudfunc = V_DrawYellowHUDPatch;
        althudfunc = V_DrawAltHUDPatch;
        fillrectfunc = V_FillRect;
        coloroffset = 4;
    }
}

void HU_Init(void)
{
    int lump;

    // load the heads-up font
    for (int i = 0, j = HU_FONTSTART; i < HU_FONTSIZE; i++)
    {
        char    buffer[9];

        M_snprintf(buffer, sizeof(buffer), "STCFN%.3d", j++);
        hu_font[i] = W_CacheLumpName(buffer);
        caretcolor = FindDominantColor(hu_font[i]);
    }

    if (W_CheckNumForName("STTMINUS") >= 0)
        if (W_CheckMultipleLumps("STTMINUS") > 1 || W_CheckMultipleLumps("STTNUM0") == 1)
        {
            minuspatch = W_CacheLumpName("STTMINUS");
            minuspatchwidth = SHORT(minuspatch->width);
        }

    tempscreen = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    if ((lump = W_CheckNumForName("ARM1A0")) >= 0)
        greenarmorpatch = W_CacheLumpNum(lump);

    if ((lump = W_CheckNumForName("ARM2A0")) >= 0)
        bluearmorpatch = W_CacheLumpNum(lump);

    ammopic[am_clip].patch = HU_LoadHUDAmmoPatch(am_clip);
    ammopic[am_shell].patch = HU_LoadHUDAmmoPatch(am_shell);

    if (gamemode != shareware)
        ammopic[am_cell].patch = HU_LoadHUDAmmoPatch(am_cell);

    ammopic[am_misl].patch = HU_LoadHUDAmmoPatch(am_misl);

    keypics[it_bluecard].patch = HU_LoadHUDKeyPatch(it_bluecard);
    keypics[it_yellowcard].patch = HU_LoadHUDKeyPatch(hacx ? it_yellowskull : it_yellowcard);
    keypics[it_redcard].patch = HU_LoadHUDKeyPatch(it_redcard);

    if (gamemode != shareware)
    {
        keypics[it_blueskull].patch = HU_LoadHUDKeyPatch(it_blueskull);
        keypics[it_yellowskull].patch = HU_LoadHUDKeyPatch(it_yellowskull);
        keypics[it_redskull].patch = HU_LoadHUDKeyPatch(it_redskull);
    }

    if ((lump = W_CheckNumForName(M_CheckParm("-cdrom") ? "STCDROM" : "STDISK")) >= 0)
    {
        stdisk = W_CacheLumpNum(lump);
        stdiskwidth = SHORT(stdisk->width);
    }

    s_STSTR_BEHOLD2 = M_StringCompare(s_STSTR_BEHOLD, STSTR_BEHOLD2);

    HU_GetMessagePosition();
    HU_AltInit();
    HU_SetTranslucency();
}

static void HU_Stop(void)
{
    headsupactive = false;
}

void HU_Start(void)
{
    char    *s = strdup(automaptitle);
    int     len = (int)strlen(s);

    if (headsupactive)
        HU_Stop();

    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    message_clearable = false;
    message_external = false;

    // create the message widget
    HUlib_initSText(&w_message, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, hu_font, HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_initTextLine(&w_title, HU_TITLEX, HU_TITLEY, hu_font, HU_FONTSTART);

    while (M_StringWidth(s) > (r_messagescale == r_messagescale_small ? (SCREENWIDTH - 12) : (ORIGINALWIDTH - 6)))
    {
        s[len - 1] = '.';
        s[len] = '.';
        s[len + 1] = '.';
        s[len + 2] = '\0';
        len--;
    }

    while (*s)
        HUlib_addCharToTextLine(&w_title, *(s++));

    headsupactive = true;
}

static void DrawHUDNumber(int *x, int y, int val, byte *tinttab, void (*hudnumfunc)(int, int, patch_t *, byte *))
{
    int     oldval = val;
    patch_t *patch;

    if (val < 0)
    {
        if (minuspatch)
        {
            val = -val;
            hudnumfunc(*x, y + 5, minuspatch, tinttab);
            *x += minuspatchwidth;

            if (val == 1 || (val >= 10 && val <= 19) || (val >= 100 && val <= 199))
                (*x)--;
        }
        else
            val = 0;

        oldval = val;
    }

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

    if (val < 0)
    {
        if (minuspatch)
        {
            val = -val;
            oldval = val;
            width = minuspatchwidth;

            if (val == 1 || (val >= 10 && val <= 19) || (val >= 100 && val <= 199))
                width--;
        }
        else
            val = 0;

        oldval = val;
    }

    if (val > 99)
        width += SHORT(tallnum[val / 100]->width);

    val %= 100;

    if (val > 9 || oldval > 99)
        width += SHORT(tallnum[val / 10]->width);

    val %= 10;
    width += SHORT(tallnum[val]->width);
    return width;
}

int healthhighlight;
int ammohighlight;
int armorhighlight;

static void HU_DrawHUD(void)
{
    const int           health = viewplayer->health;
    const weapontype_t  pendingweapon = viewplayer->pendingweapon;
    const weapontype_t  readyweapon = viewplayer->readyweapon;
    int                 ammotype = weaponinfo[readyweapon].ammotype;
    int                 ammo = viewplayer->ammo[ammotype];
    const int           armor = viewplayer->armorpoints;
    int                 health_x;
    byte                *tinttab;
    static dboolean     healthanim;
    patch_t             *patch;
    const dboolean      gamepaused = (menuactive || paused || consoleactive);
    const int           currenttime = I_GetTimeMS();
    int                 keypic_x = HUD_KEYS_X;
    static int          keywait;
    static dboolean     showkey;

    tinttab = (health <= 0 || (health <= HUD_HEALTH_MIN && healthanim) || health > HUD_HEALTH_MIN ? tinttab66 : tinttab25);

    patch = faces[st_faceindex];
    hudfunc(HUD_HEALTH_X - SHORT(patch->width) / 2, HUD_HEALTH_Y - SHORT(patch->height) - 3, patch, tinttab66);

    health_x = HUD_HEALTH_X - (HUDNumberWidth(health) + tallpercentwidth) / 2;

    if (healthhighlight > currenttime)
    {
        DrawHUDNumber(&health_x, HUD_HEALTH_Y, health, tinttab, V_DrawHighlightedHUDNumberPatch);

        if (!emptytallpercent)
            V_DrawHighlightedHUDNumberPatch(health_x, HUD_HEALTH_Y, tallpercent, tinttab);
    }
    else
    {
        DrawHUDNumber(&health_x, HUD_HEALTH_Y, health, tinttab, hudnumfunc);

        if (!emptytallpercent)
            hudnumfunc(health_x, HUD_HEALTH_Y, tallpercent, tinttab);
    }

    if (!gamepaused)
    {
        static int  healthwait;

        if (health > 0 && health <= HUD_HEALTH_MIN)
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
        ammotype = weaponinfo[pendingweapon].ammotype;
        ammo = viewplayer->ammo[ammotype];
    }

    if (health > 0 && ammo && ammotype != am_noammo)
    {
        int             ammo_x = HUD_AMMO_X - HUDNumberWidth(ammo) / 2;
        static dboolean ammoanim;

        tinttab = (ammoanim || ammo > HUD_AMMO_MIN ? tinttab66 : tinttab25);

        if ((patch = ammopic[ammotype].patch))
            hudfunc(HUD_AMMO_X - SHORT(patch->width) / 2, HUD_AMMO_Y - SHORT(patch->height) - 3, patch, tinttab66);

        DrawHUDNumber(&ammo_x, HUD_AMMO_Y, ammo, tinttab,
            (ammohighlight > currenttime ? V_DrawHighlightedHUDNumberPatch : hudnumfunc));

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

    for (int i = 1; i <= NUMCARDS; i++)
        for (int j = 0; j < NUMCARDS; j++)
            if (viewplayer->cards[j] == i && (patch = keypics[j].patch))
            {
                keypic_x -= SHORT(patch->width);
                hudfunc(keypic_x, HUD_KEYS_Y, patch, tinttab66);
                keypic_x -= 4;
            }

    if (viewplayer->neededcardflash)
    {
        if ((patch = keypics[viewplayer->neededcard].patch))
        {
            if (!gamepaused && keywait < currenttime)
            {
                showkey = !showkey;
                keywait = currenttime + HUD_KEY_WAIT;
                viewplayer->neededcardflash--;
            }

            if (showkey)
                hudfunc(keypic_x - SHORT(patch->width), HUD_KEYS_Y, patch, tinttab66);
        }
    }
    else
    {
        showkey = false;
        keywait = 0;
    }

    if (armor)
    {
        int armor_x = HUD_ARMOR_X - (HUDNumberWidth(armor) + tallpercentwidth) / 2;

        if ((patch = (viewplayer->armortype == GREENARMOR ? greenarmorpatch : bluearmorpatch)))
            hudfunc(HUD_ARMOR_X - SHORT(patch->width) / 2, HUD_ARMOR_Y - SHORT(patch->height) - 3, patch, tinttab66);

        if (armorhighlight > currenttime)
        {
            DrawHUDNumber(&armor_x, HUD_ARMOR_Y, armor, tinttab66, V_DrawHighlightedHUDNumberPatch);

            if (!emptytallpercent)
                V_DrawHighlightedHUDNumberPatch(armor_x, HUD_ARMOR_Y, tallpercent, tinttab66);
        }
        else
        {
            DrawHUDNumber(&armor_x, HUD_ARMOR_Y, armor, tinttab66, hudnumfunc);

            if (!emptytallpercent)
                hudnumfunc(armor_x, HUD_ARMOR_Y, tallpercent, tinttab66);
        }
    }
}

#define ALTHUD_LEFT_X   21
#define ALTHUD_RIGHT_X  (SCREENWIDTH - 179)
#define ALTHUD_Y        (SCREENHEIGHT - SBARHEIGHT - 37)

#define WHITE           4
#define LIGHTGRAY       86
#define GRAY            92
#define DARKGRAY        102
#define GREEN           114
#define RED             180
#define BLUE            200
#define YELLOW          231

typedef struct
{
    int     color;
    patch_t *patch;
} altkeypic_t;

static altkeypic_t altkeypics[NUMCARDS] =
{
    { BLUE   },
    { YELLOW },
    { RED    },
    { BLUE   },
    { YELLOW },
    { RED    }
};

static patch_t  *altnum[10];
static patch_t  *altnum2[10];
static patch_t  *altminuspatch;
static short    altminuspatchwidth;
static patch_t  *altweapon[NUMWEAPONS];
static patch_t  *altendpatch;
static patch_t  *altleftpatch;
static patch_t  *altarmpatch;
static patch_t  *altrightpatch;
static patch_t  *altmarkpatch;
static patch_t  *altmark2patch;
static patch_t  *altkeypatch;
static patch_t  *altskullpatch;

int             white;
static int      lightgray;
static int      gray;
static int      darkgray;
static int      green;
static int      red;
static int      yellow;

static void HU_AltInit(void)
{
    char    buffer[9];

    for (int i = 0; i < 10; i++)
    {
        M_snprintf(buffer, sizeof(buffer), "DRHUD%i", i);
        altnum[i] = W_CacheLumpName(buffer);
        M_snprintf(buffer, sizeof(buffer), "DRHUD%i_2", i);
        altnum2[i] = W_CacheLumpName(buffer);
    }

    altminuspatch = W_CacheLumpName("DRHUDNEG");
    altminuspatchwidth = SHORT(altminuspatch->width);

    for (int i = 1; i < NUMWEAPONS; i++)
    {
        M_snprintf(buffer, sizeof(buffer), "DRHUDWP%i", i);
        altweapon[i] = W_CacheLumpName(buffer);
    }

    altleftpatch = W_CacheLumpName("DRHUDL");
    altarmpatch = W_CacheLumpName("DRHUDARM");
    altrightpatch = W_CacheLumpName("DRHUDR");

    altendpatch = W_CacheLumpName("DRHUDE");
    altmarkpatch = W_CacheLumpName("DRHUDI");
    altmark2patch = W_CacheLumpName("DRHUDI_2");

    altkeypatch = W_CacheLumpName("DRHUDKEY");
    altskullpatch = W_CacheLumpName("DRHUDSKU");

    if (gamemode != shareware)
        for (int i = 0; i < NUMCARDS; i++)
        {
            int lump = W_GetNumForName(keypics[i].patchnamea);

            altkeypics[i].color = (lumpinfo[lump]->wadfile->type == PWAD ?
                FindDominantColor(W_CacheLumpNum(lump)) : nearestcolors[altkeypics[i].color]);
        }

    altkeypics[0].patch = altkeypatch;
    altkeypics[1].patch = altkeypatch;
    altkeypics[2].patch = altkeypatch;
    altkeypics[3].patch = altskullpatch;
    altkeypics[4].patch = altskullpatch;
    altkeypics[5].patch = altskullpatch;

    white = nearestcolors[WHITE];
    lightgray = nearestcolors[LIGHTGRAY];
    gray = nearestcolors[GRAY];
    darkgray = nearestcolors[DARKGRAY];
    green = nearestcolors[GREEN];
    red = nearestcolors[RED];
    yellow = nearestcolors[YELLOW];
}

static void DrawAltHUDNumber(int x, int y, int val)
{
    const int   oldval = ABS(val);
    patch_t     *patch;

    if (val < 0)
    {
        val = -val;
        althudfunc(x - altminuspatchwidth - ((val == 1 || val == 7 || (val >= 10 && val <= 19) || (val >= 70
            && val <= 79) || (val >= 100 && val <= 199)) ? 1 : 2), y, altminuspatch, WHITE, white);
    }

    if (val > 99)
    {
        patch = altnum[val / 100];
        althudfunc(x, y, patch, WHITE, white);
        x += SHORT(patch->width) + 2;
    }

    val %= 100;

    if (val > 9 || oldval > 99)
    {
        patch = altnum[val / 10];
        althudfunc(x, y, patch, WHITE, white);
        x += SHORT(patch->width) + 2;
    }

    althudfunc(x, y, altnum[val % 10], WHITE, white);
}

static int AltHUDNumberWidth(int val)
{
    const int   oldval = val;
    int         width = 0;

    if (val > 99)
        width += SHORT(altnum[val / 100]->width) + 2;

    val %= 100;

    if (val > 9 || oldval > 99)
        width += SHORT(altnum[val / 10]->width) + 2;

    return (width + SHORT(altnum[val % 10]->width));
}

static void DrawAltHUDNumber2(int x, int y, int val, int color)
{
    const int   oldval = val;
    patch_t     *patch;

    if (val > 99)
    {
        patch = altnum2[val / 100];
        althudfunc(x, y, patch, WHITE, color);
        x += SHORT(patch->width) + 1;
    }

    val %= 100;

    if (val > 9 || oldval > 99)
    {
        patch = altnum2[val / 10];
        althudfunc(x, y, patch, WHITE, color);
        x += SHORT(patch->width) + 1;
    }

    althudfunc(x, y, altnum2[val % 10], WHITE, color);
}

static int AltHUDNumber2Width(int val)
{
    const int   oldval = val;
    int         width = 0;

    if (val > 99)
        width += SHORT(altnum2[val / 100]->width) + 1;

    val %= 100;

    if (val > 9 || oldval > 99)
        width += SHORT(altnum2[val / 10]->width) + 1;

    return (width + SHORT(altnum2[val % 10]->width));
}

static void HU_DrawAltHUD(void)
{
    int             health = MAX(health_min, viewplayer->health);
    int             armor = viewplayer->armorpoints;
    int             color2 = (health <= 20 ? red : (health >= 100 ? green : white));
    int             color1 = color2 + (color2 == green ? coloroffset : 0);
    int             keypic_x = ALTHUD_RIGHT_X;
    static int      keywait;
    static dboolean showkey;
    int             powerup = 0;
    int             powerupbar = 0;
    int             max;

    DrawAltHUDNumber(ALTHUD_LEFT_X + 35 - AltHUDNumberWidth(ABS(health)), ALTHUD_Y + 12, health);
    health = MAX(0, health) * 200 / maxhealth;

    if (health > 100)
    {
        fillrectfunc(0, ALTHUD_LEFT_X + 60, ALTHUD_Y + 13, 101, 8, color1, true);
        fillrectfunc(0, ALTHUD_LEFT_X + 60, ALTHUD_Y + 13, MAX(1, health - 100) + (health == 200), 8, color2, (health == 200));
        althudfunc(ALTHUD_LEFT_X + 40, ALTHUD_Y + 1, altleftpatch, WHITE, white);
        althudfunc(ALTHUD_LEFT_X + 60, ALTHUD_Y + 13, altendpatch, WHITE, color2);
        althudfunc(ALTHUD_LEFT_X + 60 + 98, ALTHUD_Y + 13, altmarkpatch, WHITE, color1);
        althudfunc(ALTHUD_LEFT_X + 60 + health - 100 - (health < 200) - 2, ALTHUD_Y + 10, altmark2patch, WHITE, color2);
    }
    else
    {
        fillrectfunc(0, ALTHUD_LEFT_X + 60, ALTHUD_Y + 13, MAX(1, health) + (health == 100), 8, color1, true);
        althudfunc(ALTHUD_LEFT_X + 40, ALTHUD_Y + 1, altleftpatch, WHITE, white);
        althudfunc(ALTHUD_LEFT_X + 60, ALTHUD_Y + 13, altendpatch, WHITE, color1);
        althudfunc(ALTHUD_LEFT_X + 60 + MAX(1, health) - (health < 100) - 2, ALTHUD_Y + 13, altmarkpatch, WHITE, color1);
    }

    if (armor)
    {
        color2 = (viewplayer->armortype == GREENARMOR ? gray : lightgray);
        color1 = color2 + coloroffset;
        althudfunc(ALTHUD_LEFT_X + 43, ALTHUD_Y, altarmpatch, WHITE, color2);
        DrawAltHUDNumber2(ALTHUD_LEFT_X + 35 - AltHUDNumber2Width(armor), ALTHUD_Y, armor, color2);
        armor = armor * 200 / max_armor;

        if (armor > 100)
        {
            fillrectfunc(0, ALTHUD_LEFT_X + 60, ALTHUD_Y + 2, 100 + 1, 4, color1, true);
            fillrectfunc(0, ALTHUD_LEFT_X + 60, ALTHUD_Y + 2, armor - 100 + (armor == 200), 4, color2, (armor == 200));
        }
        else
            fillrectfunc(0, ALTHUD_LEFT_X + 60, ALTHUD_Y + 2, armor + (armor == 100), 4, color1, true);
    }
    else
        althudfunc(ALTHUD_LEFT_X + 43, ALTHUD_Y, altarmpatch, WHITE, darkgray);

    if (health)
    {
        const weapontype_t  pendingweapon = viewplayer->pendingweapon;
        const weapontype_t  weapon = (pendingweapon != wp_nochange ? pendingweapon : viewplayer->readyweapon);
        const ammotype_t    ammotype = weaponinfo[weapon].ammotype;

        if (ammotype != am_noammo)
        {
            int ammo = viewplayer->ammo[ammotype];

            DrawAltHUDNumber(ALTHUD_RIGHT_X + 101 - AltHUDNumberWidth(ammo), ALTHUD_Y - 1, ammo);
            ammo = 100 * ammo / viewplayer->maxammo[ammotype];
            color1 = (ammo <= 15 ? yellow : white);
            fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1, 8, color1, true);
            althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, white);
            althudfunc(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, color1);
            althudfunc(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 13, altmarkpatch, WHITE, color1);
        }

        if (weapon != wp_fist)
            althudfunc(ALTHUD_RIGHT_X + 107, ALTHUD_Y - 15, altweapon[weapon], WHITE, white);
    }

    for (int i = 1; i <= NUMCARDS; i++)
        for (int j = 0; j < NUMCARDS; j++)
            if (viewplayer->cards[j] == i)
            {
                altkeypic_t altkeypic = altkeypics[j];
                patch_t     *patch = altkeypic.patch;

                althudfunc(keypic_x, ALTHUD_Y, patch, WHITE, altkeypic.color);
                keypic_x += SHORT(patch->width) + 4;
            }

    if (viewplayer->neededcardflash)
    {
        if (!(menuactive || paused || consoleactive))
        {
            int currenttime = I_GetTimeMS();

            if (keywait < currenttime)
            {
                showkey = !showkey;
                keywait = currenttime + HUD_KEY_WAIT;
                viewplayer->neededcardflash--;
            }
        }

        if (showkey)
        {
            altkeypic_t altkeypic = altkeypics[viewplayer->neededcard];
            patch_t     *patch = altkeypic.patch;

            althudfunc(keypic_x, ALTHUD_Y, patch, WHITE, altkeypic.color);
        }
    }
    else
    {
        showkey = false;
        keywait = 0;
    }

    if ((powerup = viewplayer->powers[pw_invulnerability]))
    {
        max = INVULNTICS;
        powerupbar = (powerup == -1 ? max : powerup);
    }

    if ((powerup = viewplayer->powers[pw_invisibility]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
    {
        max = INVISTICS;
        powerupbar = (powerup == -1 ? max : powerup);
    }

    if ((powerup = viewplayer->powers[pw_ironfeet]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
    {
        max = IRONTICS;
        powerupbar = (powerup == -1 ? max : powerup);
    }

    if ((powerup = viewplayer->powers[pw_infrared]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
    {
        max = INFRATICS;
        powerupbar = (powerup == -1 ? max : powerup);
    }

    if ((powerup = viewplayer->powers[pw_strength])
        && ((viewplayer->readyweapon == wp_fist && viewplayer->pendingweapon == wp_nochange)
            || viewplayer->pendingweapon == wp_fist)
        && !powerupbar)
    {
        max = STARTFLASHING + 1;
        powerupbar = STARTFLASHING + 1;
    }

    if (powerupbar > STARTFLASHING || (powerupbar & 8))
    {
        fillrectfunc(0, ALTHUD_RIGHT_X, ALTHUD_Y + 26, 101, 2, darkgray, false);
        fillrectfunc(0, ALTHUD_RIGHT_X, ALTHUD_Y + 26, powerupbar * 101 / max, 2, gray, false);
    }
}

void HU_DrawDisk(void)
{
    if (r_diskicon && stdisk)
        V_DrawBigPatch(SCREENWIDTH - HU_MSGX * SCREENSCALE - stdiskwidth, HU_MSGY * SCREENSCALE, 0, stdisk);
}

void HU_GetMessagePosition(void)
{
    if (sscanf(r_messagepos, "(%10i,%10i)", &message_x, &message_y) != 2
        || message_x < 0
        || message_x >= SCREENWIDTH
        || message_y < 0
        || message_y >= SCREENHEIGHT - SBARHEIGHT)
    {
        message_x = HU_MSGX;
        message_y = HU_MSGY;
        r_messagepos = r_messagepos_default;
        M_SaveCVARs();
    }
}

void HU_Drawer(void)
{
    if (!vid_widescreen || !r_althud)
    {
        if (r_messagescale == r_messagescale_small)
        {
            w_message.l->x = BETWEEN(0, message_x * SCREENSCALE, SCREENWIDTH - M_StringWidth(w_message.l->l));
            w_message.l->y = BETWEEN(0, message_y * SCREENSCALE, SCREENHEIGHT - SBARHEIGHT - hu_font[0]->height);
        }
        else
        {
            w_message.l->x = BETWEEN(0, message_x, ORIGINALWIDTH - M_StringWidth(w_message.l->l));
            w_message.l->y = BETWEEN(0, message_y, ORIGINALHEIGHT - ORIGINALSBARHEIGHT - hu_font[0]->height);
        }
    }

    HUlib_drawSText(&w_message, message_external);

    if (automapactive)
    {
        if (r_messagescale == r_messagescale_big)
        {
            w_title.x = HU_TITLEX;
            w_title.y = ORIGINALHEIGHT - ORIGINALSBARHEIGHT - hu_font[0]->height - 2;
        }
        else
        {
            w_title.x = HU_TITLEX * SCREENSCALE;
            w_title.y = SCREENHEIGHT - SBARHEIGHT - hu_font[0]->height - 2 * SCREENSCALE;
        }

        HUlib_drawTextLine(&w_title, false);
    }
    else
    {
        if (vid_widescreen && r_hud)
        {
            if (r_althud)
                HU_DrawAltHUD();
            else
                HU_DrawHUD();
        }

        if (mapwindow)
        {
            if (r_messagescale == r_messagescale_big)
            {
                w_title.x = HU_TITLEX;
                w_title.y = ORIGINALHEIGHT - ORIGINALSBARHEIGHT - hu_font[0]->height - 2;
            }
            else
            {
                w_title.x = HU_TITLEX * SCREENSCALE;
                w_title.y = SCREENHEIGHT - SBARHEIGHT - hu_font[0]->height - 2 * SCREENSCALE;
            }

            HUlib_drawTextLine(&w_title, true);
        }
    }
}

void HU_Erase(void)
{
    if (message_on)
        HUlib_eraseSText(&w_message);

    if (mapwindow || automapactive)
        HUlib_eraseTextLine(&w_title);
}

extern fixed_t  m_x, m_y;
extern fixed_t  m_h, m_w;
extern dboolean message_dontpause;
extern dboolean inhelpscreens;
extern int      direction;

void HU_Ticker(void)
{
    const dboolean  idmypos = !!(viewplayer->cheats & CF_MYPOS);

    // tick down message counter if message is up
    if (message_counter
        && ((!menuactive && !paused && !consoleactive) || inhelpscreens || message_dontpause)
        && !idbehold
        && !idmypos
        && !--message_counter)
    {
        message_on = false;
        message_nottobefuckedwith = false;

        if (message_dontpause)
        {
            message_dontpause = false;
            blurred = false;
        }

        message_external = false;
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
            int x = (m_x + m_w / 2) >> MAPBITS;
            int y = (m_y + m_h / 2) >> MAPBITS;

            M_snprintf(buffer, sizeof(buffer), s_STSTR_MYPOS, direction, x, y,
                R_PointInSubsector(x, y)->sector->floorheight >> FRACBITS);
        }
        else
        {
            int angle = (int)((double)viewangle * 90.0f / ANG90);

            M_snprintf(buffer, sizeof(buffer), s_STSTR_MYPOS, (angle == 360 ? 0 : angle),
                viewx >> FRACBITS, viewy >> FRACBITS, viewplayer->mo->z >> FRACBITS);
        }

        HUlib_addMessageToSText(&w_message, 0, buffer);
        message_on = true;
    }

    // display message if necessary
    if (viewplayer->message && (!message_nottobefuckedwith || message_dontfuckwithme))
    {
        if (!idbehold && !idmypos && (messages || message_dontfuckwithme))
        {
            int     len = (int)strlen(viewplayer->message);
            char    *s = malloc(133);

            strcpy(s, viewplayer->message);

            while (M_StringWidth(s) > (vid_widescreen || r_messagescale == r_messagescale_small ?
                (SCREENWIDTH - 12) : (ORIGINALWIDTH - 6)))
            {
                s[len - 1] = '.';
                s[len] = '.';
                s[len + 1] = '.';
                s[len + 2] = '\0';
                len--;
            }

            HUlib_addMessageToSText(&w_message, 0, s);
            message_on = true;
            message_counter = HU_MSGTIMEOUT;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = false;

            free(s);
        }

        viewplayer->message = NULL;
    }
}

void HU_SetPlayerMessage(char *message, dboolean external)
{
    viewplayer->message = message;
    message_external = (external && mapwindow);
}

void HU_PlayerMessage(char *message, dboolean external)
{
    static char buffer[1024];

    if (message[0] == '%' && message[1] == 's')
        M_snprintf(buffer, sizeof(buffer), message, playername);
    else
        M_StringCopy(buffer, message, sizeof(buffer));

    buffer[0] = toupper(buffer[0]);
    C_PlayerMessage(buffer);

    if (gamestate == GS_LEVEL && !consoleactive && !message_dontfuckwithme)
        HU_SetPlayerMessage(buffer, external);
}

void HU_ClearMessages(void)
{
    if ((idbehold || (viewplayer->cheats & CF_MYPOS)) && !message_clearable)
        return;

    viewplayer->message = NULL;
    message_counter = 0;
    message_on = false;
    message_nottobefuckedwith = false;
    message_dontfuckwithme = false;
    message_dontpause = false;
    message_clearable = false;
    message_external = false;
}
