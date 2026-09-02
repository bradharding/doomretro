/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <math.h>

#include "c_console.h"
#include "d_items.h"
#include "d_player.h"
#include "doomdef.h"
#include "doomtype.h"
#include "doomstat.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_timer.h"
#include "m_array.h"
#include "m_config.h"
#include "m_misc.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

typedef struct
{
    weapontype_t    weapon;
    bool            selected;
} weaponicon_t;

static weaponicon_t *weaponicons;
static weapontype_t carouselweapons[NUMWEAPONS];
static patch_t      *pickuppatches[NUMWEAPONS];
static patch_t      *carouselpatches[NUMWEAPONS][2];
static int          pickupxoffset[NUMWEAPONS];
static int          pickupyoffset[NUMWEAPONS];
static byte         pickuptint[256];
static int          selectedindex = 0;
static int          tallesticonheight;
static byte         bordercolor;

static int          lastindex = -1;
static uint64_t     lasttime;
static int          distance;

static int          duration;
static int          fade;

static byte ContrastColor(const byte color)
{
    const byte  *palette = &PLAYPAL[color * 3];
    const int   red = BETWEEN(0, 128 + ((int)palette[0] - 128) * 140 / 100, 255);
    const int   green = BETWEEN(0, 128 + ((int)palette[1] - 128) * 140 / 100, 255);
    const int   blue = BETWEEN(0, 128 + ((int)palette[2] - 128) * 140 / 100, 255);

    return I_GetNearestColor(PLAYPAL, red, green, blue);
}

void ST_InitCarousel(void)
{
    tallesticonheight = 0;

    for (int i = 0; i < 256; i++)
        pickuptint[i] = ContrastColor(tinttab60[(black25[grays[i]] << 8) + (consoleedgecolor1 >> 8)]);

    bordercolor = black25[consoleedgecolor1];

    for (int i = 0; i < NUMWEAPONS; i++)
    {
        carouselweapons[i] = wp_nochange;
        pickuppatches[i] = NULL;
        carouselpatches[i][0] = NULL;
        carouselpatches[i][1] = NULL;
    }

    for (int i = 0; i < NUMWEAPONS; i++)
    {
        const int   order = weaponinfo[i].carouselorder;

        if (order >= 0 && order < NUMWEAPONS)
            carouselweapons[order] = (weapontype_t)i;

        if (i <= wp_pistol)
        {
            char    lump[9];
            int     lumpnum;

            M_snprintf(lump, sizeof(lump), "DRHUDWP%d", i);

            if ((lumpnum = W_CheckNumForName(lump)) >= 0)
                pickuppatches[i] = W_CacheLumpNum(lumpnum);
        }
        else
            pickuppatches[i] = weaponinfo[i].weaponpatch;

        if (pickuppatches[i])
        {
            const int   width = LITTLESHORT(pickuppatches[i]->width);
            const int   height = LITTLESHORT(pickuppatches[i]->height);

            tallesticonheight = MAX(tallesticonheight, height);
            pickupxoffset[i] = (64 - (width - (width - 1) / 4)) / 2 - 32;
        }

        if (weaponinfo[i].carouselicon)
            for (int selected = 0; selected < 2; selected++)
            {
                char    lump[9];
                int     lumpnum;

                M_snprintf(lump, sizeof(lump), "%s%d", weaponinfo[i].carouselicon, selected);

                if ((lumpnum = W_CheckNumForName(lump)) >= 0)
                    carouselpatches[i][selected] = W_CacheLumpNum(lumpnum);
            }
    }

    for (int i = 0; i < NUMWEAPONS; i++)
        if (pickuppatches[i])
            pickupyoffset[i] = (tallesticonheight - LITTLESHORT(pickuppatches[i]->height)) / 2 - 16;
}

void ST_ResetCarousel(void)
{
    lastindex = -1;
    lasttime = 0;
    distance = 0;
    duration = 0;
    fade = 0;
    selectedindex = 0;
}

static void BuildWeaponIcons(void)
{
    const weapontype_t  selectedweapon = (viewplayer->pendingweapon == wp_nochange ?
                            viewplayer->readyweapon : viewplayer->pendingweapon);

    array_clear(weaponicons);

    selectedindex = 0;

    for (int i = 0; i < NUMWEAPONS; i++)
    {
        const weapontype_t  weapon = carouselweapons[i];

        if (weapon == wp_nochange)
            continue;

        if (weapon == wp_fist && viewplayer->weaponowned[wp_chainsaw] && !viewplayer->powers[pw_strength])
            continue;

        if (weaponinfo[weapon].ammotype != am_noammo && !infiniteammo
            && !viewplayer->ammo[weaponinfo[weapon].ammotype])
            continue;

        if (lastindex == -1 && weapon == viewplayer->readyweapon)
            lastindex = array_size(weaponicons);

        if (viewplayer->weaponowned[weapon])
        {
            const bool      selected = (selectedweapon == weapon);
            weaponicon_t    icon = { weapon, selected };

            if (selected)
                selectedindex = array_size(weaponicons);

            array_push(weaponicons, icon);
        }
    }
}

void ST_UpdateCarousel(void)
{
    if (!weaponcarousel || automapactive || menuactive || paused
        || viewplayer->playerstate == PST_DEAD || !viewplayer->mo || viewplayer->mo->health <= 0)
        ST_ResetCarousel();
    else
    {
        BuildWeaponIcons();

        if (lastindex == -1)
            lastindex = selectedindex;
        else if (lastindex != selectedindex)
        {
            distance = selectedindex - lastindex;

            if (array_size(weaponicons) > 1)
            {
                const int   weaponcount = array_size(weaponicons);

                if (distance > weaponcount / 2)
                    distance -= weaponcount;
                else if (distance < -weaponcount / 2)
                    distance += weaponcount;
            }

            distance = 64 * MAX(-2, MIN(distance, 2));
            lastindex = selectedindex;
            lasttime = I_GetTimeMS();
            duration = TICRATE / 2;
            fade = (smoothtransitions ? 0 : 4);
        }
        else if (duration > 0)
        {
            if (viewplayer->pendingweapon == wp_nochange)
                duration--;

            if (smoothtransitions)
            {
                if (fade < 4)
                    fade++;

                if (!duration)
                    duration = -4;
            }
        }
        else if (duration < 0)
        {
            fade--;
            duration++;
        }
    }
}

static void CarouselDrawIcon(int x, int y, weaponicon_t icon)
{
    const weapontype_t  weapon = icon.weapon;
    const bool          selected = icon.selected;
    patch_t             *patch = carouselpatches[weapon][selected];

    if (patch)
    {
        if (r_hud_translucency)
            V_DrawDropShadowPatch(x, y, 0, patch,
                (fade == 1 ? black10 : (fade == 2 ? black25 : black40)));

        if (fade == 4 && !r_hud_translucency)
            V_DrawPatch(x, y, 0, patch);
        else if (fade > 0)
            V_DrawTranslucentPatch(x, y, 0, patch,
                (fade == 1 ? tinttab25 : (fade == 2 ? tinttab50 : (fade == 3 ? tinttab75 : tinttab80))));
    }
    else if ((patch = pickuppatches[weapon]))
    {
        x += pickupxoffset[weapon];
        y += pickupyoffset[weapon];

        if (r_hud_translucency)
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                    V_DrawSmallDropShadowPatch(x + dx, y + dy, 0, patch, black40);

        if (selected)
        {
            const int   fadepercent = fade * 25;
            const byte  darkgold = I_GetNearestColor(PLAYPAL, 128 * fadepercent / 100, 96 * fadepercent / 100, 0);

            V_DrawSmallColoredPatch(x - 1, y - 1, 0, patch, darkgold);
            V_DrawSmallColoredPatch(x, y - 1, 0, patch, darkgold);
            V_DrawSmallColoredPatch(x + 1, y - 1, 0, patch, darkgold);
            V_DrawSmallColoredPatch(x - 1, y, 0, patch, darkgold);
            V_DrawSmallColoredPatch(x + 1, y, 0, patch, darkgold);
            V_DrawSmallColoredPatch(x - 1, y + 1, 0, patch, darkgold);
            V_DrawSmallColoredPatch(x, y + 1, 0, patch, darkgold);
            V_DrawSmallColoredPatch(x + 1, y + 1, 0, patch, darkgold);
        }
        else
        {
            V_DrawSmallColoredPatch(x - 1, y - 1, 0, patch, bordercolor);
            V_DrawSmallColoredPatch(x, y - 1, 0, patch, bordercolor);
            V_DrawSmallColoredPatch(x + 1, y - 1, 0, patch, bordercolor);
            V_DrawSmallColoredPatch(x - 1, y, 0, patch, bordercolor);
            V_DrawSmallColoredPatch(x + 1, y, 0, patch, bordercolor);
            V_DrawSmallColoredPatch(x - 1, y + 1, 0, patch, bordercolor);
            V_DrawSmallColoredPatch(x, y + 1, 0, patch, bordercolor);
            V_DrawSmallColoredPatch(x + 1, y + 1, 0, patch, bordercolor);
        }

        V_DrawSmallTintedPatch(x, y, 0, patch, pickuptint);
    }
}

void ST_DrawCarousel(int x, int y)
{
    int offset = x;
    int weaponcount;

    if (!weaponcarousel || !duration || fade <= 0 || !(weaponcount = array_size(weaponicons)))
        return;

    if (distance)
    {
        const uint64_t  delta = I_GetTimeMS() - lasttime;

        if (delta < 125)
        {
            const float fx = 1.0f - delta / 125.0f;

            offset += lroundf(distance * fx * fx);
        }
    }

    CarouselDrawIcon(offset, y, weaponicons[selectedindex]);

    if (weaponcount < 3)
    {
        for (int i = selectedindex + 1, j = 1; i < weaponcount; i++, j++)
            CarouselDrawIcon(offset + j * 64, y, weaponicons[i]);

        for (int i = selectedindex - 1, j = 1; i >= 0; i--, j++)
            CarouselDrawIcon(offset - j * 64, y, weaponicons[i]);
    }
    else
    {
        const int   direction = (distance >= 0 ? -1 : 1);
        const int   firstcount = (weaponcount == 3 ? 1 : 2);
        const int   secondcount = (weaponcount == 4 ? 1 : firstcount);

        for (int i = 1; i <= firstcount; i++)
            CarouselDrawIcon(offset + direction * i * 64, y,
                weaponicons[((selectedindex + direction * i) % weaponcount + weaponcount) % weaponcount]);

        for (int i = 1; i <= secondcount; i++)
            CarouselDrawIcon(offset - direction * i * 64, y,
                weaponicons[((selectedindex - direction * i) % weaponcount + weaponcount) % weaponcount]);
    }
}
