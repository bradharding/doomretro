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

#include <ctype.h>

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_setup.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Locally used constants, shortcuts.
//
#define WHITE           4
#define GRAY            92
#define DARKGRAY        102
#define GREEN           114
#define RED             180
#define BLUE            200
#define YELLOW          231

#define STSTR_BEHOLD2   "inVuln, bSrk, Inviso, Rad, Allmap or Lite-amp?"

patch_t                 *hu_font[HU_FONTSIZE];
static hu_textline_t    w_title;

dboolean                message_on;
dboolean                message_dontfuckwithme;
static dboolean         message_external;
static dboolean         message_nottobefuckedwith;

dboolean                idbehold;
dboolean                s_STSTR_BEHOLD2;

static hu_stext_t       w_message;
static int              message_counter;

static dboolean         headsupactive;

static patch_t          *minuspatch;
static short            minuspatchwidth;
static int              minuspatchy;
static patch_t          *greenarmorpatch;
static patch_t          *bluearmorpatch;

int                     crosshair = crosshair_default;
int                     crosshaircolor = crosshaircolor_default;
char                    *playername = playername_default;
dboolean                r_althud = r_althud_default;
dboolean                r_diskicon = r_diskicon_default;
dboolean                r_hud = r_hud_default;
dboolean                r_hud_translucency = r_hud_translucency_default;

static patch_t          *stdisk;
static short            stdiskwidth;
dboolean                drawdisk;

static int              coloroffset;

extern patch_t          *tallnum[10];
extern patch_t          *tallpercent;
extern short            tallpercentwidth;
extern dboolean         emptytallpercent;
extern int              caretcolor;
extern patch_t          *faces[ST_NUMFACES];
extern int              st_faceindex;

void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);

static void (*hudfunc)(int, int, patch_t *, byte *);
static void (*hudnumfunc)(int, int, patch_t *, byte *);

static void (*althudfunc)(int, int, patch_t *, int, int);
void (*althudtextfunc)(int, int, byte *, patch_t *, int);
static void (*fillrectfunc)(int, int, int, int, int, int, dboolean);
static void (*fillrectfunc2)(int, int, int, int, int, int, dboolean);

static struct
{
    char    *patchname;
    int     mobjnum;
    patch_t *patch;
} ammopic[NUMAMMO] = {
    { "CLIPA0", MT_CLIP   },
    { "SHELA0", MT_MISC22 },
    { "CELLA0", MT_MISC20 },
    { "ROCKA0", MT_MISC18 }
};

static struct
{
    char    *patchnamea;
    char    *patchnameb;
    patch_t *patch;
} keypics[NUMCARDS] = {
    { "BKEYA0", "BKEYB0" },
    { "YKEYA0", "YKEYB0" },
    { "RKEYA0", "RKEYB0" },
    { "BSKUA0", "BSKUB0" },
    { "YSKUA0", "YSKUB0" },
    { "RSKUA0", "RSKUB0" }
};

static void HU_AltInit(void);

static patch_t *HU_LoadHUDAmmoPatch(int ammopicnum)
{
    int lump;

    if ((mobjinfo[ammopic[ammopicnum].mobjnum].flags & MF_SPECIAL) && (lump = W_CheckNumForName(ammopic[ammopicnum].patchname)) >= 0)
        return W_CacheLumpNum(lump);

    return NULL;
}

static patch_t *HU_LoadHUDKeyPatch(int keypicnum)
{
    int lump;

    if (dehacked && (lump = W_CheckNumForName(keypics[keypicnum].patchnamea)) >= 0)
        return W_CacheLumpNum(lump);
    else if ((lump = W_CheckNumForName(keypics[keypicnum].patchnameb)) >= 0)
        return W_CacheLumpNum(lump);

    return NULL;
}

void HU_SetTranslucency(void)
{
    if (r_hud_translucency)
    {
        hudfunc = &V_DrawTranslucentHUDPatch;
        hudnumfunc = &V_DrawTranslucentHUDNumberPatch;
        althudfunc = &V_DrawTranslucentAltHUDPatch;
        althudtextfunc =  &V_DrawTranslucentAltHUDText;
        fillrectfunc = &V_FillSoftTransRect;
        fillrectfunc2 = &V_FillTransRect;
        coloroffset = 0;
    }
    else
    {
        hudfunc = &V_DrawHUDPatch;
        hudnumfunc = &V_DrawHUDPatch;
        althudfunc = &V_DrawAltHUDPatch;
        althudtextfunc = &V_DrawAltHUDText;
        fillrectfunc = &V_FillRect;
        fillrectfunc2 = &V_FillRect;
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

        M_snprintf(buffer, sizeof(buffer), "STCFN%03d", j++);
        hu_font[i] = W_CacheLumpName(buffer);
    }

    caretcolor = FindDominantColor(hu_font['A' - HU_FONTSTART], W_CacheLumpName("PLAYPAL"));

    if (W_CheckNumForName("STTMINUS") >= 0)
        if (W_CheckMultipleLumps("STTMINUS") > 1 || W_CheckMultipleLumps("STTNUM0") == 1)
        {
            patch_t *patch = W_CacheLumpName("STTNUM0");

            minuspatch = W_CacheLumpName("STTMINUS");
            minuspatchwidth = SHORT(minuspatch->width);
            minuspatchy = (SHORT(patch->height) - SHORT(minuspatch->height)) / 2;
        }

    if ((lump = W_CheckNumForName("ARM1A0")) >= 0)
        greenarmorpatch = W_CacheLumpNum(lump);

    if ((lump = W_CheckNumForName("ARM2A0")) >= 0)
        bluearmorpatch = W_CacheLumpNum(lump);

    for (int i = 0; i < NUMAMMO; i++)
        ammopic[i].patch = HU_LoadHUDAmmoPatch(i);

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

    HU_AltInit();
    HU_SetTranslucency();
}

static void HU_Stop(void)
{
    headsupactive = false;
}

void HU_Start(void)
{
    char    *s = M_StringDuplicate(automaptitle);
    int     len = (int)strlen(s);

    if (headsupactive)
        HU_Stop();

    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    message_external = false;

    // create the message widget
    HUlib_InitSText(&w_message, w_message.l->x, w_message.l->y, HU_MSGHEIGHT, hu_font, HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_InitTextLine(&w_title, w_title.x, w_title.y, hu_font, HU_FONTSTART);

    while (M_StringWidth(s) > (vid_widescreen ? SCREENWIDTH - 12 : VANILLAWIDTH - 6))
    {
        if (len >= 2 && s[len - 2] == ' ')
        {
            s[len - 2] = '.';
            s[len - 1] = '.';
            s[len] = '.';
            s[len + 1] = '\0';
        }
        else
        {
            s[len - 1] = '.';
            s[len] = '.';
            s[len + 1] = '.';
            s[len + 2] = '\0';
        }

        len--;
    }

    while (*s)
        HUlib_AddCharToTextLine(&w_title, *(s++));

    headsupactive = true;
}

static void DrawHUDNumber(int *x, int y, int val, byte *translucency, void (*drawhudnumfunc)(int, int, patch_t *, byte *))
{
    patch_t *patch;

    if (val < 0)
    {
        if (minuspatch)
        {
            val = -val;
            drawhudnumfunc(*x, y + minuspatchy, minuspatch, translucency);
            *x += minuspatchwidth;

            if (val == 1 || val == 7 || (val >= 10 && val <= 19) || (val >= 70 && val <= 79) || (val >= 100 && val <= 199))
                (*x)--;
        }
        else
            val = 0;
    }

    if (val >= 100)
    {
        drawhudnumfunc(*x, y, (patch = tallnum[val / 100]), translucency);
        *x += SHORT(patch->width);
        drawhudnumfunc(*x, y, (patch = tallnum[(val %= 100) / 10]), translucency);
        *x += SHORT(patch->width);
        drawhudnumfunc(*x, y, (patch = tallnum[val % 10]), translucency);
        *x += SHORT(patch->width);
    }
    else if (val >= 10)
    {
        drawhudnumfunc(*x, y, (patch = tallnum[val / 10]), translucency);
        *x += SHORT(patch->width);
        drawhudnumfunc(*x, y, (patch = tallnum[val % 10]), translucency);
        *x += SHORT(patch->width);
    }
    else
    {
        drawhudnumfunc(*x, y, (patch = tallnum[val % 10]), translucency);
        *x += SHORT(patch->width);
    }
}

static int HUDNumberWidth(int val)
{
    int width = 0;

    if (val < 0)
    {
        if (minuspatch)
        {
            val = -val;
            width = minuspatchwidth;

            if (val == 1 || val == 7 || (val >= 10 && val <= 19) || (val >= 70 && val <= 79) || (val >= 100 && val <= 199))
                width--;
        }
        else
            val = 0;
    }

    if (val >= 100)
    {
        width += SHORT(tallnum[val / 100]->width) + 2;
        width += SHORT(tallnum[(val %= 100) / 10]->width) + 2;
    }
    else if (val >= 10)
        width = SHORT(tallnum[val / 10]->width) + 2;

    return (width + SHORT(tallnum[val % 10]->width));
}

static void HU_DrawCrosshair(void)
{
    byte    *color = (viewplayer->attackdown ? &tinttab50[nearestcolors[crosshaircolor] << 8] :
                &tinttab40[nearestcolors[crosshaircolor] << 8]);

    if (crosshair == crosshair_cross)
    {
        byte    *dot = *screens + (SCREENHEIGHT - SBARHEIGHT - 3) * SCREENWIDTH / 2 - 1;

        *dot = *(*dot + color);
        dot += SCREENWIDTH;
        *dot = *(*dot + color);
        dot += SCREENWIDTH - 2;
        *dot = *(*dot + color);
        dot++;
        *dot = *(*dot + color);
        dot++;
        *dot = *(*dot + color);
        dot++;
        *dot = *(*dot + color);
        dot++;
        *dot = *(*dot + color);
        dot += SCREENWIDTH - 2;
        *dot = *(*dot + color);
        dot += SCREENWIDTH;
        *dot = *(*dot + color);
    }
    else
    {
        byte    *dot = *screens + (SCREENHEIGHT - SBARHEIGHT - 1) * SCREENWIDTH / 2 - 1;

        *dot = *(*dot + color);
        dot++;
        *dot = *(*dot + color);
        dot += SCREENWIDTH;
        *dot = *(*dot + color);
        dot--;
        *dot = *(*dot + color);
    }
}

static void HU_DrawSolidCrosshair(void)
{
    int color = nearestcolors[crosshaircolor];

    if (crosshair == crosshair_cross)
    {
        byte    *dot = *screens + (SCREENHEIGHT - SBARHEIGHT - 3) * SCREENWIDTH / 2 - 1;

        *dot = color;
        dot += SCREENWIDTH;
        *dot = color;
        dot += SCREENWIDTH - 2;
        *dot++ = color;
        *dot++ = color;
        *dot++ = color;
        *dot++ = color;
        *dot = color;
        dot += SCREENWIDTH - 2;
        *dot = color;
        dot += SCREENWIDTH;
        *dot = color;
    }
    else
    {
        byte    *dot = *screens + (SCREENHEIGHT - SBARHEIGHT - 1) * SCREENWIDTH / 2 - 1;

        *dot++ = color;
        *dot = color;
        dot += SCREENWIDTH;
        *dot-- = color;
        *dot = color;
    }
}

int healthhighlight = 0;
int ammohighlight = 0;
int armorhighlight = 0;

static void HU_DrawHUD(void)
{
    const int           health = MAX(health_min, viewplayer->health);
    const int           armor = viewplayer->armorpoints;
    int                 health_x = HUDNumberWidth(health);
    static dboolean     healthanim;
    const dboolean      gamepaused = (menuactive || paused || consoleactive || freeze);
    byte                *translucency = (health <= 0 || (health <= HUD_HEALTH_MIN && healthanim)
                            || health > HUD_HEALTH_MIN || gamepaused ? tinttab66 : tinttab25);
    patch_t             *patch;
    const int           currenttime = I_GetTimeMS();
    int                 keypic_x = HUD_KEYS_X;
    static int          keywait;
    static dboolean     showkey;

    health_x = HUD_HEALTH_X - (health_x + (health_x & 1) + tallpercentwidth) / 2;

    if ((patch = faces[st_faceindex]))
        hudfunc(HUD_HEALTH_X - SHORT(patch->width) / 2, HUD_HEALTH_Y - SHORT(patch->height) - 2, patch, tinttab66);

    if (r_hud_translucency || !healthanim)
    {
        if (healthhighlight > currenttime)
        {
            DrawHUDNumber(&health_x, HUD_HEALTH_Y, health, translucency, &V_DrawHighlightedHUDNumberPatch);

            if (!emptytallpercent)
                V_DrawHighlightedHUDNumberPatch(health_x, HUD_HEALTH_Y, tallpercent, translucency);
        }
        else
        {
            DrawHUDNumber(&health_x, HUD_HEALTH_Y, health, translucency, hudnumfunc);

            if (!emptytallpercent)
                hudnumfunc(health_x, HUD_HEALTH_Y, tallpercent, translucency);
        }
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

    if (armor)
    {
        int armor_x = HUDNumberWidth(armor);

        armor_x = HUD_ARMOR_X - (armor_x + (armor_x & 1) + tallpercentwidth) / 2;

        if ((patch = (viewplayer->armortype == armortype_green ? greenarmorpatch : bluearmorpatch)))
            hudfunc(HUD_ARMOR_X - SHORT(patch->width) / 2, HUD_ARMOR_Y - SHORT(patch->height) - 3, patch, tinttab66);

        if (armorhighlight > currenttime)
        {
            DrawHUDNumber(&armor_x, HUD_ARMOR_Y, armor, tinttab66, &V_DrawHighlightedHUDNumberPatch);

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

    for (int i = 1; i <= NUMCARDS; i++)
        for (int j = 0; j < NUMCARDS; j++)
            if (viewplayer->cards[j] == i && (patch = keypics[j].patch))
            {
                keypic_x -= SHORT(patch->width);
                hudfunc(keypic_x, HUD_KEYS_Y - (SHORT(patch->height) - 16), patch, tinttab66);
                keypic_x -= 5;
            }

    if (viewplayer->neededcardflash)
    {
        const int   neededcard = viewplayer->neededcard;

        if (neededcard == it_allkeys)
        {
            if (!gamepaused && keywait < currenttime)
            {
                showkey = !showkey;
                keywait = currenttime + HUD_KEY_WAIT;
                viewplayer->neededcardflash--;
            }

            if (showkey || gamepaused)
                for (int i = 0; i < NUMCARDS; i++)
                    if ((patch = keypics[i].patch) && viewplayer->cards[i] != i)
                    {
                        keypic_x -= SHORT(patch->width);
                        hudfunc(keypic_x, HUD_KEYS_Y - (SHORT(patch->height) - 16), patch, tinttab66);
                        keypic_x -= 5;
                    }
        }
        else if ((patch = keypics[neededcard].patch))
        {
            if (!gamepaused && keywait < currenttime)
            {
                showkey = !showkey;
                keywait = currenttime + HUD_KEY_WAIT;
                viewplayer->neededcardflash--;
            }

            if (showkey || gamepaused)
                hudfunc(keypic_x - SHORT(patch->width), HUD_KEYS_Y - (SHORT(patch->height) - 16), patch, tinttab66);
        }
    }
    else
    {
        showkey = false;
        keywait = 0;
    }

    if (health > 0)
    {
        const weapontype_t  pendingweapon = viewplayer->pendingweapon;
        ammotype_t          ammotype = weaponinfo[(pendingweapon != wp_nochange ? pendingweapon : viewplayer->readyweapon)].ammotype;
        int                 ammo;

        if (ammotype != am_noammo && (ammo = viewplayer->ammo[ammotype]))
        {
            int             ammo_x = HUDNumberWidth(ammo);
            static dboolean ammoanim;

            ammo_x = HUD_AMMO_X - (ammo_x + (ammo_x & 1)) / 2;
            translucency = (ammoanim || ammo > HUD_AMMO_MIN || gamepaused ? tinttab66 : tinttab25);

            if ((patch = ammopic[ammotype].patch))
                hudfunc(HUD_AMMO_X - SHORT(patch->width) / 2 - 1, HUD_AMMO_Y - SHORT(patch->height) - 3, patch, tinttab66);

            if (r_hud_translucency || !ammoanim)
                DrawHUDNumber(&ammo_x, HUD_AMMO_Y, ammo, translucency,
                    (ammohighlight > currenttime ? &V_DrawHighlightedHUDNumberPatch : hudnumfunc));

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
    }
}

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

static int      gray;
static int      darkgray;
static int      green;
static int      blue;
static int      red;
static int      yellow;

static void HU_AltInit(void)
{
    char        buffer[9];
    patch_t     *altkeypatch;
    patch_t     *altskullpatch;
    dboolean    weaponschanged = false;

    for (int i = 0; i < 10; i++)
    {
        M_snprintf(buffer, sizeof(buffer), "DRHUD1%iA", i);
        altnum[i] = W_CacheLumpName(buffer);
        M_snprintf(buffer, sizeof(buffer), "DRHUD1%iB", i);
        altnum2[i] = W_CacheLumpName(buffer);
    }

    altminuspatch = W_CacheLumpName("DRHUDNEG");
    altminuspatchwidth = SHORT(altminuspatch->width);

    altarmpatch = W_CacheLumpName("DRHUDARM");

    altendpatch = W_CacheLumpName("DRHUDE");
    altmarkpatch = W_CacheLumpName("DRHUDIA");
    altmark2patch = W_CacheLumpName("DRHUDIB");

    altkeypatch = W_CacheLumpName("DRHUDKEY");
    altskullpatch = W_CacheLumpName("DRHUDSKU");

    for (int i = 0; i < NUMCARDS; i++)
        if (lumpinfo[i]->wadfile->type == PWAD)
        {
            if (keypics[i].patch)
                altkeypics[i].color = FindDominantColor(keypics[i].patch, W_CacheLumpName("SPLSHPAL"));
        }
        else if (!BTSX)
            altkeypics[i].color = nearestcolors[altkeypics[i].color];

    altkeypics[0].patch = altkeypatch;
    altkeypics[1].patch = altkeypatch;
    altkeypics[2].patch = altkeypatch;
    altkeypics[3].patch = altskullpatch;
    altkeypics[4].patch = altskullpatch;
    altkeypics[5].patch = altskullpatch;

    for (int i = 1; i < NUMWEAPONS; i++)
    {
        int lump = W_CheckNumForName(weaponinfo[i].spritename);

        if (lump >= 0 && lumpinfo[lump]->wadfile->type == PWAD)
        {
            weaponschanged = true;
            break;
        }
    }

    if (!weaponschanged || BTSX)
        for (int i = 1; i < NUMWEAPONS; i++)
        {
            M_snprintf(buffer, sizeof(buffer), "DRHUDWP%i", i);
            altweapon[i] = W_CacheLumpName(buffer);
        }

    altleftpatch = W_CacheLumpName("DRHUDL");
    altrightpatch = W_CacheLumpName("DRHUDR");

    gray = nearestcolors[GRAY];
    darkgray = nearestcolors[DARKGRAY];
    green = nearestcolors[GREEN];
    blue = (BTSX ? BLUE : nearestcolors[BLUE]);
    red = nearestcolors[RED];
    yellow = nearestcolors[YELLOW];
}

static void DrawAltHUDNumber(int x, int y, int val, int color)
{
    if (val < 0)
    {
        val = -val;
        althudfunc(x - altminuspatchwidth - (val == 1 || val == 7 || (val >= 10 && val <= 19) || (val >= 70 && val <= 79)
            || (val >= 100 && val <= 199) ? 1 : 2), y, altminuspatch, WHITE, color);
    }

    if (val >= 100)
    {
        patch_t *patch = altnum[val / 100];

        althudfunc(x, y, patch, WHITE, color);
        x += SHORT(patch->width) + 2;
        althudfunc(x, y, (patch = altnum[(val %= 100) / 10]), WHITE, color);
        althudfunc(x + SHORT(patch->width) + 2, y, altnum[val % 10], WHITE, color);
    }
    else if (val >= 10)
    {
        patch_t *patch = altnum[val / 10];

        althudfunc(x, y, patch, WHITE, color);
        althudfunc(x + SHORT(patch->width) + 2, y, altnum[val % 10], WHITE, color);
    }
    else
        althudfunc(x, y, altnum[val % 10], WHITE, color);
}

static int AltHUDNumberWidth(int val)
{
    int width = 0;

    if (val >= 100)
    {
        width = SHORT(altnum[val / 100]->width) + 2;
        width += SHORT(altnum[(val %= 100) / 10]->width) + 2;
    }
    else if (val >= 10)
        width = SHORT(altnum[val / 10]->width) + 2;

    return (width + SHORT(altnum[val % 10]->width));
}

static void DrawAltHUDNumber2(int x, int y, int val, int color)
{
    if (val >= 100)
    {
        patch_t *patch = altnum2[val / 100];

        althudfunc(x, y, patch, WHITE, color);
        x += SHORT(patch->width) + 2;

        patch = altnum2[(val %= 100) / 10];
        althudfunc(x, y, patch, WHITE, color);
        x += SHORT(patch->width) + 2;
    }
    else if (val >= 10)
    {
        patch_t *patch = altnum2[val / 10];

        althudfunc(x, y, patch, WHITE, color);
        x += SHORT(patch->width) + 2;
    }

    althudfunc(x, y, altnum2[val % 10], WHITE, color);
}

static int AltHUDNumber2Width(int val)
{
    int width = 0;

    if (val >= 100)
    {
        width = SHORT(altnum2[val / 100]->width) + 2;
        width += SHORT(altnum2[(val %= 100) / 10]->width) + 2;
    }
    else if (val >= 10)
        width = SHORT(altnum2[val / 10]->width) + 2;

    return (width + SHORT(altnum2[val % 10]->width));
}

static void HU_DrawAltHUD(void)
{
    dboolean        invert = ((viewplayer->fixedcolormap == INVERSECOLORMAP) ^ (!r_textures));
    int             color = (invert ? colormaps[0][32 * 256 + nearestwhite] : nearestwhite);
    int             health = MAX(health_min, viewplayer->health);
    int             armor = viewplayer->armorpoints;
    int             barcolor2 = (health <= 20 ? red : (health >= 100 ? green : color));
    int             barcolor1 = barcolor2;
    int             keypic_x = ALTHUD_RIGHT_X;
    static int      keywait;
    static dboolean showkey;
    int             powerup = 0;
    int             powerupbar = 0;
    int             max = 1;

    if (barcolor1 == green)
        barcolor1 += coloroffset;

    DrawAltHUDNumber(ALTHUD_LEFT_X - AltHUDNumberWidth(ABS(health)), ALTHUD_Y + 12, health, color);

    if ((health = MAX(0, health) * 200 / maxhealth) > 100)
    {
        fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, 101, 8, barcolor1, true);
        fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, MAX(1, health - 100) + (health == 200), 8, barcolor2, (health == 200));
        althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y + 11, altleftpatch, WHITE, color);
        althudfunc(ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, altendpatch, WHITE, barcolor2);
        althudfunc(ALTHUD_LEFT_X + 123, ALTHUD_Y + 13, altmarkpatch, WHITE, barcolor1);
        althudfunc(ALTHUD_LEFT_X + 25 + health - 100 - (health < 200) - 2, ALTHUD_Y + 10, altmark2patch, WHITE, barcolor2);
    }
    else
    {
        fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, MAX(1, health) + (health == 100), 8, barcolor1, true);
        althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y + 11, altleftpatch, WHITE, color);
        althudfunc(ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, altendpatch, WHITE, barcolor1);
        althudfunc(ALTHUD_LEFT_X + 25 + MAX(1, health) - (health < 100) - 2, ALTHUD_Y + 13, altmarkpatch, WHITE, barcolor1);
    }

    if (armor)
    {
        barcolor2 = (viewplayer->armortype == armortype_green ? green : blue);
        barcolor1 = barcolor2 + coloroffset;
        DrawAltHUDNumber2(ALTHUD_LEFT_X - AltHUDNumber2Width(armor), ALTHUD_Y, armor, color);
        althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y, altarmpatch, WHITE, color);

        if ((armor *= 200 / max_armor) > 100)
        {
            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, 100 + 1, 4, barcolor1, true);
            fillrectfunc2(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor - 100 + (armor == 200), 4, barcolor2, false);
        }
        else
            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor + (armor == 100), 4, barcolor1, true);
    }
    else
        althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y, altarmpatch, WHITE, darkgray);

    if (health)
    {
        const weapontype_t  pendingweapon = viewplayer->pendingweapon;
        const weapontype_t  weapon = (pendingweapon != wp_nochange ? pendingweapon : viewplayer->readyweapon);
        const ammotype_t    ammotype = weaponinfo[weapon].ammotype;

        if (ammotype != am_noammo)
        {
            int ammo = viewplayer->ammo[ammotype];

            DrawAltHUDNumber(ALTHUD_RIGHT_X + 101 - AltHUDNumberWidth(ammo), ALTHUD_Y - 1, ammo, color);
            ammo = 100 * ammo / viewplayer->maxammo[ammotype];
            barcolor1 = (ammo <= 15 ? yellow : color);
            fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1, 8, barcolor1, true);
            althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color);
            althudfunc(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, barcolor1);
            althudfunc(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 13, altmarkpatch, WHITE, barcolor1);
        }

        if (altweapon[weapon])
            althudfunc(ALTHUD_RIGHT_X + 107, ALTHUD_Y - 15, altweapon[weapon], WHITE, color);
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
        const dboolean  gamepaused = (menuactive || paused || consoleactive || freeze);
        const int       neededcard = viewplayer->neededcard;

        if (neededcard == it_allkeys)
        {
            if (!gamepaused)
            {
                int currenttime = I_GetTimeMS();

                if (keywait < currenttime)
                {
                    showkey = !showkey;
                    keywait = currenttime + HUD_KEY_WAIT;
                    viewplayer->neededcardflash--;
                }
            }

            if (showkey || gamepaused)
                for (int i = 0; i < NUMCARDS; i++)
                    if (viewplayer->cards[i] != i)
                    {
                        altkeypic_t altkeypic = altkeypics[i];
                        patch_t     *patch = altkeypic.patch;

                        althudfunc(keypic_x, ALTHUD_Y, patch, WHITE, altkeypic.color);
                        keypic_x += SHORT(patch->width) + 4;
                    }
        }
        else
        {
            if (!gamepaused)
            {
                int currenttime = I_GetTimeMS();

                if (keywait < currenttime)
                {
                    showkey = !showkey;
                    keywait = currenttime + HUD_KEY_WAIT;
                    viewplayer->neededcardflash--;
                }
            }

            if (showkey || gamepaused)
            {
                altkeypic_t altkeypic = altkeypics[viewplayer->neededcard];

                althudfunc(keypic_x, ALTHUD_Y, altkeypic.patch, WHITE, altkeypic.color);
            }
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
        powerupbar = (powerup == -1 ? INT_MAX : powerup);
    }

    if ((powerup = viewplayer->powers[pw_invisibility]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
    {
        max = INVISTICS;
        powerupbar = (powerup == -1 ? INT_MAX : powerup);
    }

    if ((powerup = viewplayer->powers[pw_ironfeet]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
    {
        max = IRONTICS;
        powerupbar = (powerup == -1 ? INT_MAX : powerup);
    }

    if ((powerup = viewplayer->powers[pw_infrared]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
    {
        max = INFRATICS;
        powerupbar = (powerup == -1 ? INT_MAX : powerup);
    }

    if (powerupbar == INT_MAX
        || (!powerupbar && viewplayer->powers[pw_strength]
            && ((viewplayer->readyweapon == wp_fist && viewplayer->pendingweapon == wp_nochange)
                || viewplayer->pendingweapon == wp_fist)))
    {
        max = STARTFLASHING + 1;
        powerupbar = STARTFLASHING + 1;
    }

    if (powerupbar > STARTFLASHING || (powerupbar & 8))
    {
        fillrectfunc2(0, ALTHUD_RIGHT_X, ALTHUD_Y + 26, 101, 2, darkgray, false);
        fillrectfunc2(0, ALTHUD_RIGHT_X, ALTHUD_Y + 26, powerupbar * 101 / max, 2, gray, false);
    }
}

void HU_DrawDisk(void)
{
    if (r_diskicon && stdisk)
        V_DrawBigPatch(SCREENWIDTH - HU_MSGX * SCREENSCALE - stdiskwidth, HU_MSGY * SCREENSCALE, stdisk);
}

void HU_Drawer(void)
{
    if (menuactive)
        return;

    if (w_message.l->l[0])
    {
        if (vanilla)
        {
            w_message.l->x = 0;
            w_message.l->y = 0;
        }
        else if (vid_widescreen)
        {
            if (r_althud)
            {
                w_message.l->x = HU_MSGX;
                w_message.l->y = HU_MSGY;
            }
            else
            {
                w_message.l->x = HU_MSGX * SCREENSCALE + 9;
                w_message.l->y = HU_MSGY * SCREENSCALE + 4;
            }
        }
        else
        {
            w_message.l->x = HU_MSGX;
            w_message.l->y = HU_MSGY;
        }

        HUlib_DrawSText(&w_message, message_external);
    }

    if (automapactive)
    {
        if (vid_widescreen)
        {
            w_title.x = HU_TITLEX * SCREENSCALE;
            w_title.y = SCREENHEIGHT - SBARHEIGHT - hu_font[0]->height - 4;

            if (r_althud)
                HUlib_DrawAltAutomapTextLine(&w_title, false);
            else
                HUlib_DrawTextLine(&w_title, false);
        }
        else
        {
            w_title.x = HU_TITLEX;
            w_title.y = VANILLAHEIGHT - VANILLASBARHEIGHT - hu_font[0]->height - 2;

            HUlib_DrawTextLine(&w_title, false);
        }
    }
    else
    {
        if (crosshair != crosshair_none)
        {
            ammotype_t  ammotype = weaponinfo[viewplayer->readyweapon].ammotype;
            actionf_t   action;

            if (ammotype != am_noammo && viewplayer->ammo[ammotype]
                && (action = viewplayer->psprites[ps_weapon].state->action) != &A_Raise && action != &A_Lower)
            {
                if (r_hud_translucency)
                    HU_DrawCrosshair();
                else
                    HU_DrawSolidCrosshair();
            }
        }

        if (vid_widescreen && r_hud)
        {
            if (r_althud)
                HU_DrawAltHUD();
            else
                HU_DrawHUD();
        }

        if (mapwindow)
        {
            w_title.x = HU_TITLEX * SCREENSCALE;
            w_title.y = SCREENHEIGHT - SBARHEIGHT - hu_font[0]->height - 4;

            if (vid_widescreen && r_althud)
                HUlib_DrawAltAutomapTextLine(&w_title, true);
            else
                HUlib_DrawTextLine(&w_title, true);
        }
    }
}

void HU_Erase(void)
{
    if (message_on)
        HUlib_EraseSText(&w_message);

    if (mapwindow || automapactive)
        HUlib_EraseTextLine(&w_title);
}

void HU_Ticker(void)
{
    const dboolean  idmypos = viewplayer->cheats & CF_MYPOS;

    // tic down message counter if message is up
    if (message_counter && !menuactive && !idmypos && !--message_counter)
    {
        message_on = false;
        message_nottobefuckedwith = false;
        message_external = false;
    }

    if (idmypos)
    {
        // [BH] display and constantly update message for IDMYPOS cheat
        char    buffer[80];

        if (!message_counter)
            message_counter = HU_MSGTIMEOUT;
        else if (message_counter > 132)
            message_counter--;

        if (automapactive && !am_followmode)
        {
            char        *temp = striptrailingzero((float)direction, 1);
            mpoint_t    center = am_frame.center;
            int         x = center.x >> MAPBITS;
            int         y = center.y >> MAPBITS;

            M_snprintf(buffer, sizeof(buffer), s_STSTR_MYPOS, temp, x, y, R_PointInSubsector(x, y)->sector->floorheight >> FRACBITS);
            free(temp);
        }
        else
        {
            float   angle = viewangle * 90.0f / ANG90;
            char    *temp = striptrailingzero((angle == 360.0f ? 0.0f : angle), 2);
            mobj_t  *mo = viewplayer->mo;
            int     z = mo->z;

            if ((mo->flags2 & MF2_FEETARECLIPPED) && r_liquid_lowerview)
                z -= FOOTCLIPSIZE;

            M_snprintf(buffer, sizeof(buffer), s_STSTR_MYPOS, temp, viewx >> FRACBITS, viewy >> FRACBITS, z >> FRACBITS);
            free(temp);
        }

        HUlib_AddMessageToSText(&w_message, buffer);
        message_on = true;
    }

    // display message if necessary
    if (viewplayer->message && (!message_nottobefuckedwith || message_dontfuckwithme))
    {
        if ((messages || message_dontfuckwithme) && !idmypos)
        {
            int     len = (int)strlen(viewplayer->message);
            char    message[133];
            int     maxwidth = VANILLAWIDTH - 6;

            if (vid_widescreen)
                maxwidth *= SCREENSCALE;

            M_StringCopy(message, viewplayer->message, sizeof(message));

            while (M_StringWidth(message) > maxwidth)
            {
                if (len >= 2 && message[len - 2] == ' ')
                {
                    message[len - 2] = '.';
                    message[len - 1] = '.';
                    message[len] = '.';
                    message[len + 1] = '\0';
                }
                else if (len >= 1)
                {
                    message[len - 1] = '.';
                    message[len] = '.';
                    message[len + 1] = '.';
                    message[len + 2] = '\0';
                }

                len--;
            }

            HUlib_AddMessageToSText(&w_message, message);
            message_on = true;
            message_counter = (idbehold ? CHEATTIMEOUT : HU_MSGTIMEOUT);
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = false;
        }

        viewplayer->message = NULL;
    }
}

void HU_SetPlayerMessage(char *message, dboolean counter, dboolean external)
{
    if (!counter)
        viewplayer->message = M_StringDuplicate(message);
    else
    {
        static int  messagecount = 1;
        char        buffer[133];

        if (M_StringCompare(message, viewplayer->prevmessage))
            M_snprintf(buffer, sizeof(buffer), "%s (%i)", message, ++messagecount);
        else
        {
            M_StringCopy(buffer, message, sizeof(buffer));
            messagecount = 1;
            M_StringCopy(viewplayer->prevmessage, message, sizeof(viewplayer->prevmessage));
        }

        viewplayer->message = M_StringDuplicate(buffer);
    }

    message_external = (external && mapwindow);
}

void HU_PlayerMessage(char *message, dboolean counter, dboolean external)
{
    char    buffer[133] = "";

    if (!*message)
        return;

    if (message[0] == '%' && message[1] == 's')
        M_snprintf(buffer, sizeof(buffer), message, playername);
    else
    {
        int len = (int)strlen(message);

        for (int i = 0, j = 0; i < len; i++)
        {
            if (message[i] == '%')
                buffer[j++] = '%';

            buffer[j++] = message[i];
        }
    }

    buffer[0] = toupper(buffer[0]);
    C_PlayerMessage(buffer);

    if (gamestate == GS_LEVEL && !consoleactive && !message_dontfuckwithme)
        HU_SetPlayerMessage(buffer, counter, external);
}

void HU_ClearMessages(void)
{
    if (viewplayer->cheats & CF_MYPOS)
        return;

    viewplayer->message = NULL;
    message_counter = 0;
    message_on = false;
    message_nottobefuckedwith = false;
    message_dontfuckwithme = false;
    message_external = false;
}
