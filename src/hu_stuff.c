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
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_setup.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"

#define STSTR_BEHOLD2   "inVuln, bSrk, Inviso, Rad, Allmap or Lite-amp?"

patch_t                 *hu_font[HU_FONTSIZE];
static hu_textline_t    w_title;
static hu_textline_t    w_author;

bool                    message_fadeon;
bool                    message_dontfuckwithme;
bool                    message_secret;
bool                    message_warning;
bool                    message_on;
static bool             message_external;
static bool             message_nottobefuckedwith;

char                    prevmessage[133];

bool                    idbehold = false;
bool                    s_STSTR_BEHOLD2;

static hu_stext_t       w_message;
int                     message_counter;

static bool             headsupactive;

patch_t                 *minuspatch = NULL;
patch_t                 *buddha;
short                   minuspatchtopoffset1 = 0;
short                   minuspatchtopoffset2 = 0;
short                   minuspatchwidth = 0;
static patch_t          *greenarmorpatch;
static patch_t          *bluearmorpatch;

static patch_t          *stdisk;
static short            stdiskwidth;
static short            stdiskheight;
bool                    drawdisk = false;
int                     drawdisktics;

static int              coloroffset;

void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);

static void (*hudfunc)(int, int, patch_t *, const byte *);
static void (*hudnumfunc)(int, int, patch_t *, const byte *);
static void (*hudnumfunc2)(int, int, patch_t *, const byte *);

static void (*althudfunc)(int, int, patch_t *, int, int, const byte *, int);
void (*althudtextfunc)(int, int, byte *, patch_t *, bool, int, int, int, const byte *);
static void (*althudweaponfunc)(int, int, patch_t *, int, int, const byte *);
static void (*fillrectfunc)(int, int, int, int, int, int, int, bool, bool, const byte *, const byte *);
static void (*fillrectfunc2)(int, int, int, int, int, int, int, bool, bool, const byte *, const byte *);

static struct
{
    const char  *patchnamea;
    const char  *patchnameb;
    patch_t     *patch;
} keypics[] = {
    { "BKEYA0", "BKEYB0", NULL },
    { "YKEYA0", "YKEYB0", NULL },
    { "RKEYA0", "RKEYB0", NULL },
    { "BSKUA0", "BSKUB0", NULL },
    { "YSKUA0", "YSKUB0", NULL },
    { "RSKUA0", "RSKUB0", NULL }
};

static void HU_AltInit(void);

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
        hudnumfunc2 = &V_DrawTranslucentHighlightedHUDNumberPatch;
        althudfunc = &V_DrawTranslucentAltHUDPatch;
        althudtextfunc = &V_DrawTranslucentAltHUDText;
        althudweaponfunc = &V_DrawTranslucentAltHUDWeaponPatch;
        fillrectfunc = &V_FillSoftTransRect;
        fillrectfunc2 = &V_FillTransRect;
        coloroffset = 0;
    }
    else
    {
        hudfunc = &V_DrawHUDPatch;
        hudnumfunc = &V_DrawHUDPatch;
        hudnumfunc2 = &V_DrawHighlightedHUDNumberPatch;
        althudfunc = &V_DrawAltHUDPatch;
        althudtextfunc = &V_DrawAltHUDText;
        althudweaponfunc = &V_DrawAltHUDWeaponPatch;
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

        M_snprintf(buffer, sizeof(buffer), "STCFN%03i", j++);
        hu_font[i] = W_CacheLumpName(buffer);

        if (W_GetNumLumps(buffer) > 1)
            STCFNxxx = true;
    }

    buddha = W_CacheLumpNameFromResourceWAD("DRBUDDH1");

    if ((lump = W_CheckNumForName("STTMINUS")) >= 0
        && (W_GetNumLumps("STTMINUS") > 1 || W_GetNumLumps("STTNUM0") == 1))
    {
        minuspatch = W_CacheLumpNum(lump);
        minuspatchwidth = SHORT(minuspatch->width);

        if (SHORT(minuspatch->height) == 6 && !SHORT(minuspatch->topoffset))
        {
            minuspatchtopoffset1 = -5;
            minuspatchtopoffset2 = -5;
        }
        else
        {
            minuspatchtopoffset1 = 0;
            minuspatchtopoffset2 = SHORT(minuspatch->topoffset);
        }
    }

    if ((lump = W_CheckNumForName("ARM1A0")) >= 0)
        greenarmorpatch = W_CacheLumpNum(lump);

    if ((lump = W_CheckNumForName("ARM2A0")) >= 0)
        bluearmorpatch = W_CacheLumpNum(lump);

    for (int i = 0; i < NUMWEAPONS; i++)
    {
        const spritenum_t   sprite = weaponinfo[i].ammosprite;

        if (!sprite
            || (gamemode == shareware && (i == wp_plasma || i == wp_bfg))
            || (gamemode != commercial && i == wp_supershotgun))
            weaponinfo[i].ammopatch = NULL;
        else
        {
            weaponinfo[i].ammopatch = W_CacheLumpNum(firstspritelump + sprite);

            for (int j = numstates - 1; j >= 0; j--)
            {
                state_t *state = &states[j];

                if (state->sprite == sprite)
                {
                    spriteframe_t   *frame = &sprites[sprite].spriteframes[state->frame & FF_FRAMEMASK];

                    if (frame)
                        weaponinfo[i].ammopatch = W_CacheLumpNum(firstspritelump + frame->lump[0]);

                    break;
                }
            }
        }
    }

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
        stdiskheight = SHORT(stdisk->height);
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
    char    *title = M_StringDuplicate(automaptitle);
    char    *author = M_StringJoin("By ", P_GetMapAuthor(gameepisode, gamemap), NULL);

    if (headsupactive)
        HU_Stop();

    message_on = false;
    message_dontfuckwithme = false;
    message_secret = false;
    message_warning = false;
    message_nottobefuckedwith = false;
    message_external = false;

    // create the message widget
    HUlib_InitSText(&w_message, w_message.l.x, w_message.l.y, hu_font, HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_InitTextLine(&w_title, w_title.x, w_title.y, hu_font, HU_FONTSTART);
    HUlib_InitTextLine(&w_author, w_author.x, w_author.y, hu_font, HU_FONTSTART);

    while (*title && *title != '\r' && *title != '\n')
        HUlib_AddCharToTextLine(&w_title, *(title++));

    while (*author && *author != '\r' && *author != '\n')
        HUlib_AddCharToTextLine(&w_author, *(author++));

    headsupactive = true;
}

static void DrawHUDNumber(int *x, int y, int val, const byte *tinttab,
    void (*drawhudnumfunc)(int, int, patch_t *, const byte *))
{
    int i;

    if (val < 0)
    {
        if (negativehealth && minuspatch)
        {
            val = -val;
            drawhudnumfunc(*x, y - minuspatchtopoffset2, minuspatch, tinttab);
            *x += minuspatchwidth;

            if (val == 1 || val == 7 || (val >= 10 && val <= 19) || (val >= 70 && val <= 79))
                (*x)--;
        }
        else
            val = 0;
    }

    if (val >= 100)
    {
        if ((i = val / 100) == 1)
        {
            (*x)++;
            drawhudnumfunc(*x, y, tallnum[i], tinttab);
            (*x)++;
        }
        else
            drawhudnumfunc(*x, y, tallnum[i], tinttab);

        *x += SHORT(tallnum[i]->width);

        if ((i = (val %= 100) / 10) == 1)
        {
            (*x)++;
            drawhudnumfunc(*x, y, tallnum[i], tinttab);
            (*x) += 2;
        }
        else
            drawhudnumfunc(*x, y, tallnum[i], tinttab);

        *x += SHORT(tallnum[i]->width);

        if ((i = val % 10) == 1)
        {
            (*x)++;
            drawhudnumfunc(*x, y, tallnum[i], tinttab);
            (*x) += 2;
        }
        else
            drawhudnumfunc(*x, y, tallnum[i], tinttab);

        *x += SHORT(tallnum[i]->width);
    }
    else if (val >= 10)
    {
        if ((i = val / 10) == 1)
        {
            (*x)++;
            drawhudnumfunc(*x, y, tallnum[i], tinttab);
            (*x)++;
        }
        else
            drawhudnumfunc(*x, y, tallnum[i], tinttab);

        *x += SHORT(tallnum[i]->width);

        if ((i = val % 10) == 1)
        {
            (*x)++;
            drawhudnumfunc(*x, y, tallnum[i], tinttab);
            (*x) += 2;
        }
        else
            drawhudnumfunc(*x, y, tallnum[i], tinttab);

        *x += SHORT(tallnum[i]->width);
    }
    else
    {
        if ((i = val % 10) == 1)
        {
            (*x)++;
            drawhudnumfunc(*x, y, tallnum[i], tinttab);
            (*x)++;
        }
        else
            drawhudnumfunc(*x, y, tallnum[i], tinttab);

        *x += SHORT(tallnum[i]->width);
    }
}

static int HUDNumberWidth(int val)
{
    int width = 0;

    if (val < 0)
    {
        if (negativehealth && minuspatch)
        {
            val = -val;
            width = minuspatchwidth;

            if (val == 1 || val == 7 || (val >= 10 && val <= 19) || (val >= 70 && val <= 79))
                width--;
        }
        else
            val = 0;
    }

    if (val >= 200)
        return (width + tallnum0width * 3);
    else if (val >= 100)
        return (width + tallnum0width * 3 - 1);
    else if (val >= 20)
        return (width + tallnum0width * 2);
    else if (val >= 10)
        return (width + tallnum0width * 2 - 1);

    return (width + tallnum0width);
}

static inline void HU_DrawScaledPixel(const int x, const int y, byte *color)
{
    byte    *dest = &screens[0][(y * 2 - 1) * SCREENWIDTH + x * 2 - 1];

    *dest = *(*dest + color);
    dest++;
    *dest = *(*dest + color);
    dest += SCREENWIDTH;
    *dest = *(*dest + color);
    dest--;
    *dest = *(*dest + color);
}

static inline void HU_DrawSolidScaledPixel(const int x, const int y, byte color)
{
    byte    *dest = &screens[0][(y * 2 - 1) * SCREENWIDTH + x * 2 - 1];

    *(dest++) = color;
    *dest = color;
    *(dest += SCREENWIDTH) = color;
    *(--dest) = color;
}

#define CENTERX (WIDESCREENDELTA + VANILLAWIDTH / 2)
#define CENTERY ((VANILLAHEIGHT - VANILLASBARHEIGHT * (r_screensize < r_screensize_max)) / 2)

static void HU_DrawCrosshair(void)
{
    byte    *color = (viewplayer->attackdown ? &tinttab50[nearestcolors[crosshaircolor] << 8] :
                &tinttab40[nearestcolors[crosshaircolor] << 8]);

    if (r_detail == r_detail_low)
    {
        if (crosshair == crosshair_cross)
        {
            HU_DrawScaledPixel(CENTERX - 2, CENTERY, color);
            HU_DrawScaledPixel(CENTERX - 1, CENTERY, color);
            HU_DrawScaledPixel(CENTERX, CENTERY, color);
            HU_DrawScaledPixel(CENTERX + 1, CENTERY, color);
            HU_DrawScaledPixel(CENTERX + 2, CENTERY, color);
            HU_DrawScaledPixel(CENTERX, CENTERY - 2, color);
            HU_DrawScaledPixel(CENTERX, CENTERY - 1, color);
            HU_DrawScaledPixel(CENTERX, CENTERY + 1, color);
            HU_DrawScaledPixel(CENTERX, CENTERY + 2, color);
        }
        else
            HU_DrawScaledPixel(CENTERX, CENTERY, color);
    }
    else
    {
        if (crosshair == crosshair_cross)
        {
            byte    *dot = *screens + (SCREENHEIGHT - SBARHEIGHT * (r_screensize < r_screensize_max) - 3) * SCREENWIDTH / 2 - 1;

            *dot = *(*dot + color);
            dot += SCREENWIDTH;
            *dot = *(*dot + color);
            dot += SCREENWIDTH;
            *dot = *(*dot + color);
            dot += (size_t)SCREENWIDTH - 3;
            *dot = *(*dot + color);
            dot++;
            *dot = *(*dot + color);
            dot++;
            *dot = *(*dot + color);
            dot++;
            *dot = *(*dot + color);
            dot++;
            *dot = *(*dot + color);
            dot++;
            *dot = *(*dot + color);
            dot++;
            *dot = *(*dot + color);
            dot += (size_t)SCREENWIDTH - 3;
            *dot = *(*dot + color);
            dot += SCREENWIDTH;
            *dot = *(*dot + color);
            dot += SCREENWIDTH;
            *dot = *(*dot + color);
        }
        else
        {
            byte    *dot = *screens + (SCREENHEIGHT - SBARHEIGHT * (r_screensize < r_screensize_max) - 1) * SCREENWIDTH / 2 - 1;

            *dot = *(*dot + color);
        }
    }
}

static void HU_DrawSolidCrosshair(void)
{
    const int   color = (viewplayer->attackdown ? nearestcolors[crosshaircolor] : black25[nearestcolors[crosshaircolor]]);

    if (r_detail == r_detail_low)
    {
        if (crosshair == crosshair_cross)
        {
            HU_DrawSolidScaledPixel(CENTERX - 2, CENTERY, color);
            HU_DrawSolidScaledPixel(CENTERX - 1, CENTERY, color);
            HU_DrawSolidScaledPixel(CENTERX, CENTERY, color);
            HU_DrawSolidScaledPixel(CENTERX + 1, CENTERY, color);
            HU_DrawSolidScaledPixel(CENTERX + 2, CENTERY, color);
            HU_DrawSolidScaledPixel(CENTERX, CENTERY - 2, color);
            HU_DrawSolidScaledPixel(CENTERX, CENTERY - 1, color);
            HU_DrawSolidScaledPixel(CENTERX, CENTERY + 1, color);
            HU_DrawSolidScaledPixel(CENTERX, CENTERY + 2, color);
        }
        else
            HU_DrawSolidScaledPixel(CENTERX, CENTERY, color);
    }
    else
    {
        if (crosshair == crosshair_cross)
        {
            byte    *dot = *screens + (SCREENHEIGHT - SBARHEIGHT * (r_screensize < r_screensize_max) - 3) * SCREENWIDTH / 2 - 1;

            *dot = color;
            dot += SCREENWIDTH;
            *dot = color;
            dot += SCREENWIDTH;
            *dot = color;
            dot += (size_t)SCREENWIDTH - 3;
            *dot++ = color;
            *dot++ = color;
            *dot++ = color;
            *dot++ = color;
            *dot++ = color;
            *dot++ = color;
            *dot = color;
            dot += (size_t)SCREENWIDTH - 3;
            *dot = color;
            dot += SCREENWIDTH;
            *dot = color;
            dot += SCREENWIDTH;
            *dot = color;
        }
        else
        {
            byte    *dot = *screens + (SCREENHEIGHT - SBARHEIGHT * (r_screensize < r_screensize_max) - 1) * SCREENWIDTH / 2 - 1;

            *dot = color;
        }
    }
}

uint64_t    ammohighlight = 0;
uint64_t    armorhighlight = 0;
uint64_t    healthhighlight = 0;

int         ammodiff[NUMAMMO] = { 0 };
int         maxammodiff[NUMAMMO] = { 0 };
int         armordiff = 0;
int         healthdiff = 0;

static void HU_DrawHUD(void)
{
    const int           health = BETWEEN(HUD_NUMBER_MIN, (negativehealth && minuspatch && !viewplayer->health ?
                            viewplayer->negativehealth : viewplayer->health) + healthdiff, HUD_NUMBER_MAX);
    int                 armor = MIN(viewplayer->armor, HUD_NUMBER_MAX);
    static bool         healthanim;
    const bool          gamepaused = (consoleactive || paused || freeze);
    byte                *tinttab = (health >= HUD_HEALTH_MIN || healthanim || health <= 0
                            || (viewplayer->cheats & CF_BUDDHA) || gamepaused ? tinttab80 : tinttab25);
    patch_t             *patch = faces[st_faceindex];
    const uint64_t      currenttime = I_GetTimeMS();
    int                 keypic_x = HUD_KEYS_X;
    static uint64_t     keywait;
    static bool         showkey;
    const weapontype_t  pendingweapon = viewplayer->pendingweapon;
    const weapontype_t  weapon = (pendingweapon == wp_nochange ? viewplayer->readyweapon : pendingweapon);
    const ammotype_t    ammotype = weaponinfo[weapon].ammotype;

    if (patch)
        hudfunc(HUD_HEALTH_X - SHORT(patch->width) / 2 - 1, HUD_HEALTH_Y - SHORT(patch->height) - 2, patch, tinttab80);

    if (r_hud_translucency || !healthanim)
    {
        int health_x = HUDNumberWidth(health);

        health_x = HUD_HEALTH_X - (health_x + (health_x & 1) + tallpercentwidth) / 2;

        if (healthhighlight > currenttime)
        {
            if (emptytallpercent)
            {
                health_x -= 4;
                DrawHUDNumber(&health_x, HUD_HEALTH_Y, health, tinttab, hudnumfunc2);
            }
            else
            {
                DrawHUDNumber(&health_x, HUD_HEALTH_Y, health, tinttab, hudnumfunc2);
                hudnumfunc2(health_x, HUD_HEALTH_Y, tallpercent, tinttab);
            }
        }
        else
        {
            if (emptytallpercent)
            {
                health_x -= 4;
                DrawHUDNumber(&health_x, HUD_HEALTH_Y, health, tinttab, hudnumfunc);
            }
            else
            {
                DrawHUDNumber(&health_x, HUD_HEALTH_Y, health, tinttab, hudnumfunc);
                hudnumfunc(health_x, HUD_HEALTH_Y, tallpercent, tinttab);
            }
        }
    }

    if (!gamepaused)
    {
        static uint64_t healthwait;

        if (health > 0 && health < HUD_HEALTH_MIN)
        {
            if (healthwait < currenttime)
            {
                healthanim = !healthanim;
                healthwait = currenttime + HUD_HEALTH_WAIT;
            }
        }
        else
        {
            healthanim = false;
            healthwait = 0;
        }
    }

    if ((armor += armordiff))
    {
        int armor_x = HUDNumberWidth(armor);

        armor_x = HUD_ARMOR_X - (armor_x + (armor_x & 1) + tallpercentwidth) / 2;

        if ((patch = (viewplayer->armortype == blue_armor_class ? bluearmorpatch : greenarmorpatch)))
            hudfunc(HUD_ARMOR_X - SHORT(patch->width) / 2, HUD_ARMOR_Y - SHORT(patch->height) - 3, patch, tinttab80);

        if (armorhighlight > currenttime)
        {
            DrawHUDNumber(&armor_x, HUD_ARMOR_Y, armor, tinttab80, hudnumfunc2);

            if (!emptytallpercent)
                hudnumfunc2(armor_x, HUD_ARMOR_Y, tallpercent, tinttab80);
        }
        else
        {
            DrawHUDNumber(&armor_x, HUD_ARMOR_Y, armor, tinttab80, hudnumfunc);

            if (!emptytallpercent)
                hudnumfunc(armor_x, HUD_ARMOR_Y, tallpercent, tinttab80);
        }
    }

    for (int i = 1; i <= NUMCARDS; i++)
        for (int j = 0; j < NUMCARDS; j++)
            if (viewplayer->cards[j] == i && (patch = keypics[j].patch))
            {
                keypic_x -= SHORT(patch->width);
                hudfunc(keypic_x, HUD_KEYS_Y - (SHORT(patch->height) - 16), patch, tinttab80);
                keypic_x -= 5;
            }

    if (viewplayer->neededcardflash)
    {
        const card_t    neededcard = viewplayer->neededcard;

        if (neededcard == it_allkeys)
        {
            if (!gamepaused && keywait < currenttime)
            {
                showkey = !showkey;
                keywait = currenttime + HUD_KEY_WAIT;
                viewplayer->neededcardflash--;
            }

            if (flashkeys && (showkey || gamepaused))
                for (int i = 0; i < NUMCARDS; i++)
                    if ((patch = keypics[i].patch) && viewplayer->cards[i] != i)
                    {
                        keypic_x -= SHORT(patch->width);
                        hudfunc(keypic_x, HUD_KEYS_Y - (SHORT(patch->height) - 16), patch, tinttab80);
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

            if (flashkeys && (showkey || gamepaused))
                hudfunc(keypic_x - SHORT(patch->width), HUD_KEYS_Y - (SHORT(patch->height) - 16), patch, tinttab80);
        }
    }
    else
    {
        showkey = false;
        keywait = 0;
    }

    if (ammotype != am_noammo)
    {
        int         ammo = viewplayer->ammo[ammotype] + ammodiff[ammotype];
        int         ammo_x = HUDNumberWidth(ammo);
        static bool ammoanim;

        ammo_x = HUD_AMMO_X - (ammo_x + (ammo_x & 1)) / 2;
        tinttab = (!ammoanim || ammo >= HUD_AMMO_MIN || gamepaused ? tinttab80 : tinttab25);

        if ((patch = weaponinfo[weapon].ammopatch))
            hudfunc(HUD_AMMO_X - SHORT(patch->width) / 2 - 1, HUD_AMMO_Y - SHORT(patch->height) - 3, patch, tinttab80);

        if (r_hud_translucency || !ammoanim)
            DrawHUDNumber(&ammo_x, HUD_AMMO_Y, ammo, tinttab,
                (ammohighlight > currenttime ? hudnumfunc2 : hudnumfunc));

        if (!gamepaused)
        {
            static uint64_t ammowait;

            if (ammo < HUD_AMMO_MIN)
            {
                if (ammowait < currenttime)
                {
                    ammoanim = !ammoanim;
                    ammowait = currenttime + HUD_AMMO_WAIT;
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

typedef struct
{
    int     color;
    byte    *tinttab;
    patch_t *patch;
    short   width;
    short   height;
} altkeypic_t;

static altkeypic_t altkeypics[NUMCARDS] =
{
    { BLUE3,   NULL, NULL },
    { YELLOW1, NULL, NULL },
    { RED2,    NULL, NULL },
    { BLUE3,   NULL, NULL },
    { YELLOW1, NULL, NULL },
    { RED2,    NULL, NULL }
};

static patch_t  *altnumpatch[10];
static patch_t  *altnumpatch2[10];
static patch_t  *altminuspatch;
static patch_t  *altendpatch;
static patch_t  *altleftpatch1;
static patch_t  *altleftpatch2;
static patch_t  *altrightpatch;
static patch_t  *altmarkpatch;
static patch_t  *altmark2patch;

static short    altminuspatchwidth;

static int      gray;
static int      darkgray;
static int      green1;
static int      green2;
static int      green3;
static int      blue2;
static int      blue3;
static int      red2;
static int      yellow1;
static int      yellow2;

typedef struct
{
    short   width;
    short   height;
    short   leftoffset;
    short   topoffset1;
    short   topoffset2;
} pistol_t;

static pistol_t pistol1[] =
{
    {  57,  62, -126, -106, -106 },
    {  79,  82, -104,  -86, -109 },
    {  66,  81, -119,  -87, -111 },
    {  61,  81, -125,  -87,  -87 },
    {  78, 103, -106,  -65,  -65 }
};

static pistol_t pistol2[] =
{
    { 107,  94,  -76, -106, -106 },
    { 116, 114,  -67,  -86,  -86 },
    { 111, 113,  -74,  -87,  -87 },
    { 109, 113,  -77,  -87,  -87 },
    { 113, 135,  -71,  -65,  -65 }
};

static bool HU_DefaultPistolSprites(void)
{
    bool    result1 = true;
    bool    result2 = true;

    for (int i = 0; i < 5; i++)
    {
        spriteframe_t   *frame = sprites[SPR_PISG].spriteframes;

        if (frame)
        {
            patch_t *patch = W_CacheLumpNum(firstspritelump + frame[i & FF_FRAMEMASK].lump[0]);

            if (SHORT(patch->width) != pistol1[i].width
                || SHORT(patch->height) != pistol1[i].height
                || SHORT(patch->leftoffset) != pistol1[i].leftoffset
                || (SHORT(patch->topoffset) != pistol1[i].topoffset1
                    && SHORT(patch->topoffset) != pistol1[i].topoffset2))
            {
                result1 = false;
                break;
            }
        }
    }

    if (!result1)
        for (int i = 0; i < 5; i++)
        {
            spriteframe_t   *frame = sprites[SPR_PISG].spriteframes;

            if (frame)
            {
                patch_t *patch = W_CacheLumpNum(firstspritelump + frame[i & FF_FRAMEMASK].lump[0]);

                if (SHORT(patch->width) != pistol2[i].width
                    || SHORT(patch->height) != pistol2[i].height
                    || SHORT(patch->leftoffset) != pistol2[i].leftoffset
                    || (SHORT(patch->topoffset) != pistol2[i].topoffset1
                        && SHORT(patch->topoffset) != pistol2[i].topoffset2))
                {
                    result2 = false;
                    break;
                }
            }
        }

    return (result1 || result2);
}

static void HU_AltInit(void)
{
    char    buffer[9];
    patch_t *altkeypatch;
    patch_t *altskullpatch;

    for (int i = 0; i <= 9; i++)
    {
        M_snprintf(buffer, sizeof(buffer), "DRHUD%iA", i);
        altnumpatch[i] = W_CacheLumpName(buffer);
        M_snprintf(buffer, sizeof(buffer), "DRHUD%iB", i);
        altnumpatch2[i] = W_CacheLumpName(buffer);
    }

    altminuspatch = W_CacheLumpName("DRHUDNEG");
    altminuspatchwidth = SHORT(altminuspatch->width);

    altleftpatch1 = W_CacheLumpName("DRHUDL1");
    altleftpatch2 = W_CacheLumpName("DRHUDL2");
    altrightpatch = W_CacheLumpName("DRHUDR");

    altendpatch = W_CacheLumpName("DRHUDE");
    altmarkpatch = W_CacheLumpName("DRHUDIA");
    altmark2patch = W_CacheLumpName("DRHUDIB");

    altkeypatch = W_CacheLumpName("DRHUDKEY");
    altskullpatch = W_CacheLumpName("DRHUDSKU");

    for (int i = 0; i < NUMCARDS; i++)
        if (lumpinfo[i]->wadfile->type == PWAD)
        {
            if (keypics[i].patch)
                altkeypics[i].color = FindBrightDominantColor(keypics[i].patch);
        }
        else if (!BTSX)
            altkeypics[i].color = nearestcolors[altkeypics[i].color];

    altkeypics[it_bluecard].patch = altkeypatch;
    altkeypics[it_bluecard].tinttab = tinttab90;
    altkeypics[it_yellowcard].patch = altkeypatch;
    altkeypics[it_yellowcard].tinttab = tinttab60;
    altkeypics[it_redcard].patch = altkeypatch;
    altkeypics[it_redcard].tinttab = tinttab60;
    altkeypics[it_blueskull].patch = altskullpatch;
    altkeypics[it_blueskull].tinttab = tinttab90;
    altkeypics[it_yellowskull].patch = altskullpatch;
    altkeypics[it_yellowskull].tinttab = tinttab60;
    altkeypics[it_redskull].patch = altskullpatch;
    altkeypics[it_redskull].tinttab = tinttab60;

    for (int i = 0; i < NUMCARDS; i++)
    {
        altkeypics[i].width = SHORT(altkeypics[i].patch->width);
        altkeypics[i].height = SHORT(altkeypics[i].patch->height);
    }

    for (int i = 0; i < NUMWEAPONS; i++)
    {
        M_snprintf(buffer, sizeof(buffer), "DRHUDWP%i", i);

        if (W_CheckNumForName(buffer) >= 0)
        {
            weaponinfo[i].weaponpatch = W_CacheLumpName(buffer);
            weaponinfo[i].weapony = ALTHUD_Y + 11 - SHORT(weaponinfo[i].weaponpatch->height) / 2;
        }
        else
        {
            const spritenum_t   sprite = weaponinfo[i].weaponsprite;

            if (!sprite
                || (gamemode == shareware && (i == wp_plasma || i == wp_bfg))
                || (gamemode != commercial && i == wp_supershotgun))
                weaponinfo[i].weaponpatch = NULL;
            else
                for (int j = numstates - 1; j >= 0; j--)
                {
                    state_t *state = &states[j];

                    if (state->sprite == sprite)
                    {
                        weaponinfo[i].weaponpatch = W_CacheLumpNum(firstspritelump
                            + sprites[state->sprite].spriteframes[state->frame & FF_FRAMEMASK].lump[0]);
                        weaponinfo[i].weapony = ALTHUD_Y + 11 - SHORT(weaponinfo[i].weaponpatch->height) / 2;
                        break;
                    }
                }
        }
    }

    if (!HU_DefaultPistolSprites())
        weaponinfo[wp_pistol].weaponpatch = NULL;

    gray = nearestcolors[GRAY1];
    darkgray = nearestcolors[DARKGRAY1];
}

static void DrawAltHUDNumber(int x, int y, int val, int color, const byte *tinttab, int shadow)
{
    if (val < 0)
    {
        if (negativehealth)
        {
            val = -val;
            althudfunc(x - altminuspatchwidth - (val == 1 || val == 7 || (val >= 10 && val <= 19)
                || (val >= 70 && val <= 79) || (val >= 100 && val <= 199) ? 1 : 2), y,
                altminuspatch, WHITE, color, tinttab, shadow);
        }
        else
            val = 0;
    }

    if (val == 1 || val % 10 == 1)
        x++;

    if (val >= 100)
    {
        patch_t *patch = altnumpatch[val / 100];

        althudfunc(x, y, patch, WHITE, color, tinttab, shadow);
        x += SHORT(patch->width) + 2;
        althudfunc(x, y, (patch = altnumpatch[(val %= 100) / 10]), WHITE, color, tinttab, shadow);
        althudfunc(x + SHORT(patch->width) + 2, y, altnumpatch[val % 10], WHITE, color, tinttab, shadow);
    }
    else if (val >= 10)
    {
        patch_t *patch = altnumpatch[val / 10];

        althudfunc(x, y, patch, WHITE, color, tinttab, shadow);
        althudfunc(x + SHORT(patch->width) + 2, y, altnumpatch[val % 10], WHITE, color, tinttab, shadow);
    }
    else
        althudfunc(x, y, altnumpatch[val % 10], WHITE, color, tinttab, shadow);
}

static int AltHUDNumberWidth(int val)
{
    int width = 0;

    if (val >= 100)
    {
        width = SHORT(altnumpatch[val / 100]->width) + 2;
        width += SHORT(altnumpatch[(val %= 100) / 10]->width) + 2;
    }
    else if (val >= 10)
        width = SHORT(altnumpatch[val / 10]->width) + 2;

    return (width + SHORT(altnumpatch[val % 10]->width));
}

static void DrawAltHUDNumber2(int x, int y, int val, int color, const byte *tinttab, int shadow)
{
    if (val == 1 || val % 10 == 1)
        x++;

    if (val >= 100)
    {
        patch_t *patch = altnumpatch2[val / 100];

        althudfunc(x, y, patch, WHITE, color, tinttab, shadow);
        x += SHORT(patch->width) + 2;

        patch = altnumpatch2[(val %= 100) / 10];

        althudfunc(x, y, patch, WHITE, color, tinttab, shadow);
        x += SHORT(patch->width) + 2;
    }
    else if (val >= 10)
    {
        patch_t *patch = altnumpatch2[val / 10];

        althudfunc(x, y, patch, WHITE, color, tinttab, shadow);
        x += SHORT(patch->width) + 2;
    }

    althudfunc(x, y, altnumpatch2[val % 10], WHITE, color, tinttab, shadow);
}

static int AltHUDNumber2Width(int val)
{
    int width = 0;

    if (val >= 100)
    {
        width = SHORT(altnumpatch2[val / 100]->width) + 2;
        width += SHORT(altnumpatch2[(val %= 100) / 10]->width) + 2;
    }
    else if (val >= 10)
        width = SHORT(altnumpatch2[val / 10]->width) + 2;

    return (width + SHORT(altnumpatch2[val % 10]->width));
}

static void HU_DrawAltHUD(void)
{
    const bool      inverted = ((viewplayer->fixedcolormap == INVERSECOLORMAP) != !r_textures);
    const int       color = (inverted ? nearestblack : (r_hud_translucency ? nearestwhite : nearestlightgray));
    int             health = BETWEEN(HUD_NUMBER_MIN, (negativehealth && minuspatch && !viewplayer->health ?
                        viewplayer->negativehealth : viewplayer->health) + healthdiff, HUD_NUMBER_MAX);
    int             armor = BETWEEN(0, viewplayer->armor + armordiff, HUD_NUMBER_MAX);
    int             keypic_x = ALTHUD_RIGHT_X;
    const uint64_t  currenttime = I_GetTimeMS();
    int             shadowcolor;

    if (inverted)
    {
        blue2 = nearestblack;
        blue3 = nearestblack;
        green1 = nearestblack;
        green2 = nearestblack;
        green3 = nearestblack;
        red2 = nearestblack;
        yellow1 = nearestblack;
        yellow2 = nearestblack;
        shadowcolor = -1;
    }
    else
    {
        if (BTSX)
        {
            blue2 = BLUE2;
            blue3 = BLUE3;
        }
        else
        {
            blue2 = nearestcolors[BLUE2];
            blue3 = nearestcolors[BLUE3];
        }

        green1 = nearestcolors[GREEN1];
        green2 = nearestcolors[GREEN2];
        green3 = nearestcolors[GREEN3];
        red2 = nearestcolors[RED2];
        yellow1 = nearestcolors[YELLOW1];
        yellow2 = nearestcolors[YELLOW2];
        shadowcolor = nearestdarkgray;
    }

    if (r_hud_translucency)
        DrawAltHUDNumber(ALTHUD_LEFT_X - AltHUDNumberWidth(ABS(health)), ALTHUD_Y + 12, health,
            color, (healthhighlight > currenttime ? tinttab80 : tinttab60), shadowcolor);
    else
        DrawAltHUDNumber(ALTHUD_LEFT_X - AltHUDNumberWidth(ABS(health)), ALTHUD_Y + 12, health,
            (healthhighlight > currenttime ? (inverted ? nearestdarkgray : nearestwhite) : color), NULL, shadowcolor);

    if (health == 100)
    {
        if (r_hud_translucency)
        {
            const int   color2 = tinttab10[green1];

            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, MAX(1, health) + 1,
                8, green2, green2, true, true, tinttab60, tinttab25);
            althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y + 11, altleftpatch1, WHITE, color, tinttab60, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
            althudfunc(ALTHUD_LEFT_X + 123, ALTHUD_Y + 22, altmarkpatch, WHITE, color2, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 125, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
        }
        else
        {
            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, MAX(1, health) + 1, 8,
                green2, green2, true, true, NULL, NULL);
            althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y + 11, altleftpatch1, WHITE, color, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, altendpatch, WHITE, green1, NULL, -1);
            althudfunc(ALTHUD_LEFT_X + 123, ALTHUD_Y + 22, altmarkpatch, WHITE, green1, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 125, ALTHUD_Y + 13, altendpatch, WHITE, green1, NULL, -1);
        }
    }
    else if (health < 100)
    {
        const int   barcolor = (health < HUD_HEALTH_MIN && !(viewplayer->cheats & CF_BUDDHA) && healthdiff != -initial_health ? red2 : color);

        health = MAX(1, health);

        if (r_hud_translucency)
        {
            const int   color2 = tinttab10[barcolor];

            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, health,
                8, barcolor, barcolor, true, true, tinttab60, tinttab25);
            althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y + 11, altleftpatch1, WHITE, color, tinttab60, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
            althudfunc(ALTHUD_LEFT_X + 25 + health - 3, ALTHUD_Y + 22, altmarkpatch, WHITE, color2, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25 + health - 1, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
        }
        else
        {
            const int   color2 = (inverted ? nearestblack : nearestwhite);

            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, health, 8,
                barcolor, barcolor, true, true, NULL, NULL);
            althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y + 11, altleftpatch1, WHITE, color, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
            althudfunc(ALTHUD_LEFT_X + 25 + health - 3, ALTHUD_Y + 22, altmarkpatch, WHITE, color2, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25 + health - 1, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
        }
    }
    else
    {
        health = MAX(0, health) * 200 / maxhealth;

        if (r_hud_translucency)
        {
            const int   color2 = tinttab10[green1];

            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, 101, 8, green2, green2, true, true, tinttab60, tinttab25);
            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, MAX(1, health - 99),
                8, green2, green2, true, (health == 200), tinttab60, tinttab40);
            althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y + 11, altleftpatch1, WHITE, color, tinttab60, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
            althudfunc(ALTHUD_LEFT_X + 123, ALTHUD_Y + 22, altmarkpatch, WHITE, color2, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 125, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
            althudfunc(ALTHUD_LEFT_X + 25 + health - 102, ALTHUD_Y + 9, altmark2patch, WHITE, green1, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25 + health - 100, ALTHUD_Y + 13, altendpatch, WHITE, green1, NULL, -1);
        }
        else
        {
            fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, 101, 8, green3, green3, true, true, NULL, NULL);

            if (inverted)
                fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, MAX(1, health - 99),
                    8, nearestdarkgray, nearestdarkgray, true, (health == 200), NULL, NULL);
            else
                fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, MAX(1, health - 99),
                    8, green2, green2, true, (health == 200), NULL, NULL);

            althudfunc(ALTHUD_LEFT_X + 5, ALTHUD_Y + 11, altleftpatch1, WHITE, color, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25, ALTHUD_Y + 13, altendpatch, WHITE, green1, NULL, -1);
            althudfunc(ALTHUD_LEFT_X + 123, ALTHUD_Y + 22, altmarkpatch, WHITE, green1, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 125, ALTHUD_Y + 13, altendpatch, WHITE, green1, NULL, -1);
            althudfunc(ALTHUD_LEFT_X + 25 + health - 102, ALTHUD_Y + 9, altmark2patch, WHITE, green1, NULL, shadowcolor);
            althudfunc(ALTHUD_LEFT_X + 25 + health - 100, ALTHUD_Y + 13, altendpatch, WHITE, green1, NULL, -1);
        }
    }

    if (armor)
    {
        if (r_hud_translucency)
            DrawAltHUDNumber2(ALTHUD_LEFT_X - AltHUDNumber2Width(armor), ALTHUD_Y, armor,
                color, (armorhighlight > currenttime ? tinttab80 : tinttab60), shadowcolor);
        else
            DrawAltHUDNumber2(ALTHUD_LEFT_X - AltHUDNumber2Width(armor), ALTHUD_Y, armor,
                (armorhighlight > currenttime ? (inverted ? nearestdarkgray : nearestwhite) : color), NULL, shadowcolor);

        althudfunc(ALTHUD_LEFT_X + 8, ALTHUD_Y, altleftpatch2, WHITE, color, tinttab60, shadowcolor);

        if ((armor *= 200 / max_armor) > 100)
        {
            if (r_hud_translucency)
            {
                if (viewplayer->armortype == green_armor_class)
                {
                    fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, 101,
                        4, green2, green2, true, true, tinttab60, tinttab25);
                    fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor - 100 + (armor == 200),
                        4, green2, green2, true, (armor == 200), tinttab60, tinttab40);
                }
                else
                {
                    fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, 101,
                        4, blue3, gray, true, true, tinttab90, tinttab5);
                    fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor - 100 + (armor == 200),
                        4, blue2, blue2, true, (armor == 200), tinttab90, tinttab10);
                }
            }
            else
            {
                if (viewplayer->armortype == green_armor_class)
                {
                    fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, 101,
                        4, green3, green3, true, true, NULL, NULL);

                    if (inverted)
                        fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor - 100 + (armor == 200),
                            4, nearestdarkgray, nearestdarkgray, true, (armor == 200), NULL, NULL);
                    else
                        fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor - 100 + (armor == 200),
                            4, green2, green2, true, (armor == 200), NULL, NULL);
                }
                else
                {
                    fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, 101,
                        4, blue3, blue3, true, true, NULL, NULL);

                    if (inverted)
                        fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor - 100 + (armor == 200),
                            4, nearestdarkgray, nearestdarkgray, true, (armor == 200), NULL, NULL);
                    else
                        fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor - 100 + (armor == 200),
                            4, blue2, blue2, true, (armor == 200), NULL, NULL);
                }
            }
        }
        else
        {
            if (r_hud_translucency)
            {
                if (viewplayer->armortype == green_armor_class)
                    fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor + (armor == 100),
                        4, green2, green2, true, true, tinttab60, tinttab25);
                else
                    fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor + (armor == 100),
                        4, blue3, gray, true, true, tinttab90, tinttab5);
            }
            else
            {
                const int   color2 = (viewplayer->armortype == blue_armor_class ? blue3 : green3);

                fillrectfunc(0, ALTHUD_LEFT_X + 25, ALTHUD_Y + 2, armor + (armor == 100),
                    4, color2, color2, true, true, NULL, NULL);
            }
        }
    }
    else
    {
        if (r_hud_translucency)
            althudfunc(ALTHUD_LEFT_X + 8, ALTHUD_Y, altleftpatch2, -1, 0, tinttab60, shadowcolor);
        else
            althudfunc(ALTHUD_LEFT_X + 8, ALTHUD_Y, altleftpatch2, WHITE,
                (inverted ? nearestblack : darkgray), NULL, shadowcolor);
    }

    if (health)
    {
        const weapontype_t  pendingweapon = viewplayer->pendingweapon;
        const weapontype_t  weapon = (pendingweapon == wp_nochange ? viewplayer->readyweapon : pendingweapon);
        const ammotype_t    ammotype = weaponinfo[weapon].ammotype;
        static uint64_t     keywait;
        static bool         showkey;
        int                 powerup = 0;
        int                 powerupbar = 0;
        int                 powertics = 1;
        patch_t             *patch;

        if (ammotype != am_noammo)
        {
            int ammo = viewplayer->ammo[ammotype] + ammodiff[ammotype];

            if (r_hud_translucency)
                DrawAltHUDNumber(ALTHUD_RIGHT_X + 101 - AltHUDNumberWidth(ammo), ALTHUD_Y - 2, ammo,
                    color, (ammohighlight > currenttime ? tinttab80 : tinttab60), shadowcolor);
            else
                DrawAltHUDNumber(ALTHUD_RIGHT_X + 101 - AltHUDNumberWidth(ammo), ALTHUD_Y - 2, ammo,
                    (ammohighlight > currenttime ? (inverted ? nearestdarkgray : nearestwhite) : color), NULL, shadowcolor);

            if (ammo)
            {
                int max = viewplayer->maxammo[ammotype];

                if (viewplayer->backpack && ammo > (max /= 2))
                {
                    ammo = MIN(100 * (ammo - max) / max, 100);

                    if (r_hud_translucency)
                    {
                        const int   color2 = tinttab10[color];

                        fillrectfunc(0, ALTHUD_RIGHT_X, ALTHUD_Y + 13, 101, 8, color, color, true, true, tinttab60, tinttab25);
                        fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1,
                            8, color, color, false, true, tinttab60, tinttab25);
                        althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, tinttab60, shadowcolor);
                        althudfunc(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
                        althudfunc(ALTHUD_RIGHT_X - 2, ALTHUD_Y + 22, altmarkpatch, WHITE, color2, NULL, shadowcolor);
                        althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
                        althudfunc(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 9, altmark2patch, WHITE, color, NULL, shadowcolor);
                        althudfunc(ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, altendpatch, WHITE, color, NULL, -1);
                    }
                    else
                    {
                        fillrectfunc(0, ALTHUD_RIGHT_X, ALTHUD_Y + 13, 101, 8, color, color, true, true, NULL, NULL);

                        if (inverted)
                        {
                            fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1,
                                8, nearestdarkgray, nearestdarkgray, false, true, NULL, NULL);
                            althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, NULL, shadowcolor);
                            althudfunc(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, nearestblack, NULL, -1);
                            althudfunc(ALTHUD_RIGHT_X - 2, ALTHUD_Y + 22, altmarkpatch, WHITE, nearestblack, NULL, shadowcolor);
                            althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altendpatch, WHITE, nearestblack, NULL, -1);
                            althudfunc(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 9, altmark2patch, WHITE, nearestblack, NULL, shadowcolor);
                            althudfunc(ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, altendpatch, WHITE, nearestblack, NULL, -1);
                        }
                        else
                        {
                            fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1,
                                8, nearestwhite, nearestwhite, false, true, NULL, NULL);
                            althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, NULL, shadowcolor);
                            althudfunc(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, nearestwhite, NULL, -1);
                            althudfunc(ALTHUD_RIGHT_X - 2, ALTHUD_Y + 22, altmarkpatch, WHITE, nearestwhite, NULL, shadowcolor);
                            althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altendpatch, WHITE, nearestwhite, NULL, -1);
                            althudfunc(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 9, altmark2patch, WHITE, nearestwhite, NULL, shadowcolor);
                            althudfunc(ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, altendpatch, WHITE, nearestwhite, NULL, -1);
                        }
                    }
                }
                else if ((ammo = 100 * ammo / max) < HUD_AMMO_MIN && ammodiff[ammotype] >= 0)
                {
                    if (r_hud_translucency)
                    {
                        fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1,
                            8, yellow2, yellow2, true, true, tinttab60, tinttab25);
                        althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, tinttab60, shadowcolor);
                    }
                    else
                    {
                        fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1,
                            8, yellow2, yellow2, true, true, NULL, NULL);
                        althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, NULL, shadowcolor);
                    }

                    althudfunc(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, yellow1, NULL, -1);
                    althudfunc(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 22, altmarkpatch, WHITE, yellow1, NULL, shadowcolor);
                    althudfunc(ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, altendpatch, WHITE, yellow1, NULL, -1);
                }
                else
                {
                    if (r_hud_translucency)
                    {
                        const int   color2 = tinttab10[color];

                        fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1,
                            8, color, color, true, true, tinttab60, tinttab25);
                        althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, tinttab60, shadowcolor);
                        althudfunc(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
                        althudfunc(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 22, altmarkpatch, WHITE, color2, NULL, shadowcolor);
                        althudfunc(ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
                    }
                    else
                    {
                        const int   color2 = (inverted ? nearestblack : nearestwhite);

                        fillrectfunc(0, ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, ammo + 1,
                            8, color, color, true, true, NULL, NULL);
                        althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, NULL, shadowcolor);
                        althudfunc(ALTHUD_RIGHT_X + 100, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
                        althudfunc(ALTHUD_RIGHT_X + 100 - ammo - 2, ALTHUD_Y + 22, altmarkpatch, WHITE, color2, NULL, shadowcolor);
                        althudfunc(ALTHUD_RIGHT_X + 100 - ammo, ALTHUD_Y + 13, altendpatch, WHITE, color2, NULL, -1);
                    }
                }
            }
            else
                althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, tinttab60, shadowcolor);
        }
        else
            althudfunc(ALTHUD_RIGHT_X, ALTHUD_Y + 13, altrightpatch, WHITE, color, tinttab60, shadowcolor);

        if ((patch = weaponinfo[weapon].weaponpatch))
            althudweaponfunc(ALTHUD_RIGHT_X + 108, weaponinfo[weapon].weapony, patch, color, shadowcolor, tinttab60);

        for (int i = 1; i <= NUMCARDS; i++)
            for (int j = 0; j < NUMCARDS; j++)
                if (viewplayer->cards[j] == i)
                {
                    const altkeypic_t   altkeypic = altkeypics[j];

                    patch = altkeypic.patch;

                    if (inverted)
                        althudfunc(keypic_x, ALTHUD_Y - altkeypic.height + 7, patch,
                            WHITE, nearestblack, tinttab60, shadowcolor);
                    else
                        althudfunc(keypic_x, ALTHUD_Y - altkeypic.height + 7, patch,
                            WHITE, altkeypic.color, altkeypic.tinttab, shadowcolor);

                    keypic_x += altkeypic.width + 4;
                }

        if (viewplayer->neededcardflash)
        {
            const bool      gamepaused = (consoleactive || paused || freeze);
            const card_t    neededcard = viewplayer->neededcard;

            if (neededcard == it_allkeys)
            {
                if (!gamepaused && keywait < currenttime)
                {
                    showkey = !showkey;
                    keywait = currenttime + HUD_KEY_WAIT;
                    viewplayer->neededcardflash--;
                }

                if (flashkeys && (showkey || gamepaused))
                    for (int i = 0; i < NUMCARDS; i++)
                        if (viewplayer->cards[i] != i)
                        {
                            const altkeypic_t   altkeypic = altkeypics[i];

                            patch = altkeypic.patch;

                            if (inverted)
                                althudfunc(keypic_x, ALTHUD_Y - altkeypic.height + 7, patch,
                                    WHITE, nearestblack, tinttab60, shadowcolor);
                            else
                                althudfunc(keypic_x, ALTHUD_Y - altkeypic.height + 7, patch,
                                    WHITE, altkeypic.color, altkeypic.tinttab, shadowcolor);

                            keypic_x += altkeypic.width + 4;
                        }
            }
            else
            {
                if (!gamepaused && keywait < currenttime)
                {
                    showkey = !showkey;
                    keywait = currenttime + HUD_KEY_WAIT;
                    viewplayer->neededcardflash--;
                }

                if (flashkeys && (showkey || gamepaused))
                {
                    const altkeypic_t   altkeypic = altkeypics[neededcard];

                    patch = altkeypic.patch;

                    if (inverted)
                        althudfunc(keypic_x, ALTHUD_Y - altkeypic.height + 7, patch,
                            WHITE, nearestblack, tinttab60, shadowcolor);
                    else
                        althudfunc(keypic_x, ALTHUD_Y - altkeypic.height + 7, patch,
                            WHITE, altkeypic.color, altkeypic.tinttab, shadowcolor);
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
            powertics = INVULNTICS;
            powerupbar = (powerup == -1 ? INT_MAX : powerup);
        }

        if ((powerup = viewplayer->powers[pw_invisibility]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
        {
            powertics = INVISTICS;
            powerupbar = (powerup == -1 ? INT_MAX : powerup);
        }

        if ((powerup = viewplayer->powers[pw_ironfeet]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
        {
            powertics = IRONTICS;
            powerupbar = (powerup == -1 ? INT_MAX : powerup);
        }

        if ((powerup = viewplayer->powers[pw_infrared]) && (!powerupbar || (powerup >= 0 && powerup < powerupbar)))
        {
            powertics = INFRATICS;
            powerupbar = (powerup == -1 ? INT_MAX : powerup);
        }

        if (powertics && (powerupbar = (powerupbar == INT_MAX ? 101 : (int)(powerupbar * 101.0 / powertics + 0.5))))
        {
            const int   color2 = (r_hud_translucency ? color : darkgray);

            fillrectfunc2(0, ALTHUD_RIGHT_X, ALTHUD_Y + 27, 101 - powerupbar,
                3, color2, color2, false, false, tinttab20, NULL);
            fillrectfunc2(0, ALTHUD_RIGHT_X + 101 - powerupbar, ALTHUD_Y + 27, powerupbar,
                3, color, color, false, false, tinttab60, NULL);
        }
    }
}

void HU_DrawDisk(void)
{
    if (r_diskicon && stdisk && drawdisktics)
    {
        V_DrawPatch(SCREENWIDTH / 2 - HU_MSGX * (vid_widescreen ? 2 : 1) - stdiskwidth - WIDESCREENDELTA,
            HU_MSGY - 1, 0, stdisk);

        if (!--drawdisktics || dowipe)
            drawdisk = false;
    }
}

void HU_Drawer(void)
{
    if (menuactive)
        return;

    if (*w_message.l.l)
    {
        if (vanilla && !vid_widescreen)
        {
            w_message.l.x = 0;
            w_message.l.y = 0;
        }
        else if ((r_screensize == r_screensize_max && !(r_althud && r_althudfont)) || message_external)
        {
            w_message.l.x = MAXWIDESCREENDELTA / 2 - HU_MSGX - 3;
            w_message.l.y = HU_MSGY + 4;
        }
        else if (vid_widescreen && r_screensize == r_screensize_max - 1)
        {
            w_message.l.x = WIDESCREENDELTA + HU_MSGX * !automapactive;
            w_message.l.y = HU_MSGY + 1;
        }
        else
        {
            w_message.l.x = (!vid_widescreen && automapactive ? 0 : HU_MSGX);
            w_message.l.y = HU_MSGY + 1;
        }

        HUlib_DrawSText(&w_message, message_external);
    }

    if (automapactive)
    {
        if (r_althud && r_althudfont && r_screensize == r_screensize_max)
        {
            if (P_GetMapAuthor(gameepisode, gamemap))
            {
                w_author.y = SCREENHEIGHT - 30;
                HUlib_DrawAltAutomapTextLine(&w_author, false);

                w_title.y = SCREENHEIGHT - 44;
            }
            else
                w_title.y = SCREENHEIGHT - 30;

            HUlib_DrawAltAutomapTextLine(&w_title, false);
        }
        else
        {
            if (P_GetMapAuthor(gameepisode, gamemap))
            {
                if (vid_widescreen)
                {
                    w_author.x = (r_screensize == r_screensize_max - 1 ? WIDESCREENDELTA * 2 : OVERLAYTEXTX);
                    w_author.y = MAPHEIGHT - SHORT(hu_font[0]->height) * 2 - (r_screensize == r_screensize_max - 1 ? 6 : 20);

                    w_title.x = (r_screensize == r_screensize_max - 1 ? WIDESCREENDELTA * 2 : OVERLAYTEXTX);
                    w_title.y = MAPHEIGHT - SHORT(hu_font[0]->height) * 2 - (r_screensize == r_screensize_max - 1 ? 10 : 38);
                }
                else
                {
                    w_author.x = 0;
                    w_author.y = MAPHEIGHT - SHORT(hu_font[0]->height) * 2 - 6;

                    w_title.x = 0;
                    w_title.y = MAPHEIGHT - SHORT(hu_font[0]->height) * 2 - 10;
                }

                HUlib_DrawAutomapTextLine(&w_author, false);
            }
            else
            {
                if (vid_widescreen)
                {
                    w_title.x = (r_screensize == r_screensize_max - 1 ? WIDESCREENDELTA * 2 : OVERLAYTEXTX);
                    w_title.y = MAPHEIGHT - SHORT(hu_font[0]->height) * 2 - (r_screensize == r_screensize_max - 1 ? 6 : 20);
                }
                else
                {
                    w_title.x = 0;
                    w_title.y = MAPHEIGHT - SHORT(hu_font[0]->height) * 2 - 6;
                }
            }

            HUlib_DrawAutomapTextLine(&w_title, false);
        }
    }
    else
    {
        if (crosshair != crosshair_none && !consoleactive)
        {
            const ammotype_t    ammotype = weaponinfo[viewplayer->readyweapon].ammotype;

            if (ammotype != am_noammo && viewplayer->ammo[ammotype])
            {
                const actionf_t action = viewplayer->psprites[ps_weapon].state->action;

                if (action != &A_Raise && action != &A_Lower)
                {
                    if (r_hud_translucency)
                        HU_DrawCrosshair();
                    else
                        HU_DrawSolidCrosshair();
                }
            }
        }

        if (r_hud)
        {
            if (r_althud)
                HU_DrawAltHUD();
            else
                HU_DrawHUD();
        }

        if (mapwindow)
        {
            if (r_althud && r_althudfont && r_screensize == r_screensize_max)
                HUlib_DrawAltAutomapTextLine(&w_title, true);
            else
            {
                w_title.x = 25;
                w_title.y = MAPHEIGHT - SHORT(hu_font[0]->height) * 2 - 16;
                HUlib_DrawAutomapTextLine(&w_title, true);
            }
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
    // tic down message counter if message is up
    if (message_counter && !menuactive && !--message_counter)
    {
        message_on = false;
        message_nottobefuckedwith = false;
        message_secret = false;
        message_warning = false;
    }

    // display message if necessary
    else if (viewplayer->message && (!message_nottobefuckedwith || message_dontfuckwithme)
        && (r_hud || r_screensize < r_screensize_max))
    {
        if (messages || message_dontfuckwithme || message_secret || message_warning)
        {
            HUlib_AddMessageToSText(&w_message, viewplayer->message);

            message_fadeon = (!message_on || message_counter <= 5);
            message_on = true;
            message_counter = HU_MSGTIMEOUT;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = false;

            if (viewplayer->message)
                free(viewplayer->message);

            viewplayer->message = NULL;
        }
    }
}

void HU_SetPlayerMessage(char *message, bool group, bool external)
{
    if (message_secret || message_warning || !viewplayer || togglingvanilla)
        return;

    M_StringReplaceAll(message, "%%", "%", false);

    if (!group)
        viewplayer->message = sentencecase(message);
    else
    {
        static int  messagecount = 1;

        if (gametime - viewplayer->prevmessagetics < HU_MSGTIMEOUT
            && M_StringCompare(message, viewplayer->prevmessage) && groupmessages)
        {
            char    buffer[133];
            char    *temp = commify(++messagecount);

            M_snprintf(buffer, sizeof(buffer), "%s (%s)", message, temp);
            free(viewplayer->message);
            viewplayer->message = sentencecase(buffer);
            free(temp);
        }
        else
        {
            messagecount = 1;
            viewplayer->message = sentencecase(message);
            M_StringCopy(viewplayer->prevmessage, message, sizeof(viewplayer->prevmessage));
        }

        viewplayer->prevmessagetics = gametime;
    }

    M_StringCopy(prevmessage, viewplayer->message, sizeof(prevmessage));
    message_external = (external && mapwindow);
}

void HU_PlayerMessage(char *message, bool group, bool external)
{
    char        buffer[133] = "";
    const int   len = (int)strlen(message);

    if (!len || (len == 1 && !isalnum(message[0])))
        return;

    M_StringReplaceAll(message, " _", " " ITALICSON, false);
    M_StringReplaceAll(message, "_ ", ITALICSOFF " ", false);

    if (len >= 2 && message[0] == '%' && message[1] == 's')
        M_snprintf(buffer, sizeof(buffer), message,
            M_StringCompare(playername, playername_default) ? "you" : playername);
    else
        for (int i = 0, j = 0; i < len; i++)
        {
            if (message[i] == '%')
                buffer[j++] = '%';

            buffer[j++] = message[i];
        }

    buffer[0] = toupper(buffer[0]);
    C_PlayerMessage(buffer);

    if (gamestate == GS_LEVEL && !message_dontfuckwithme)
        HU_SetPlayerMessage(buffer, group, external);

    viewplayer->prevmessagetics = gametime;
}

void HU_ClearMessages(void)
{
    if (viewplayer->message)
        free(viewplayer->message);

    viewplayer->message = NULL;
    message_counter = 7;
    message_on = false;
    message_nottobefuckedwith = false;
    message_dontfuckwithme = false;
    message_secret = false;
    message_warning = false;
}
