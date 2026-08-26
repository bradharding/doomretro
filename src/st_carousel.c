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

static const weapontype_t weapon_order[] =
{
    wp_fist,
    wp_chainsaw,
    wp_pistol,
    wp_shotgun,
    wp_supershotgun,
    wp_chaingun,
    wp_missile,
    wp_plasma,
    wp_bfg
};

typedef enum
{
    wpi_none = -1,
    wpi_regular,
    wpi_selected,
    wpi_disabled
} weapon_icon_state_t;

typedef struct
{
    weapontype_t        weapon;
    weapon_icon_state_t state;
} weapon_icon_t;

static weapon_icon_t    *weapon_icons;
static int              selected_index = 0;

static int              last_index = -1;
static uint64_t         last_time;
static int              distance;

static int              duration;
static int              fade;

static bool WeaponSelectable(const weapontype_t weapon)
{
    const ammotype_t    ammotype = weaponinfo[weapon].ammotype;

    return (viewplayer->weaponowned[weapon]
        && (ammotype == am_noammo || infiniteammo
            || viewplayer->ammo[ammotype] >= weaponinfo[weapon].ammopershot));
}

void ST_ResetCarousel(void)
{
    last_index = -1;
    last_time = 0;
    distance = 0;
    duration = 0;
    fade = 0;
    selected_index = 0;
}

static void BuildWeaponIcons(void)
{
    const weapontype_t  selectedweapon = (viewplayer->pendingweapon == wp_nochange ?
                            viewplayer->readyweapon : viewplayer->pendingweapon);

    array_clear(weapon_icons);

    selected_index = 0;

    for (int i = 0; i < arrlen(weapon_order); i++)
    {
        weapontype_t        weapon = weapon_order[i];
        weapon_icon_state_t state = wpi_none;

        if (last_index == -1 && weapon == viewplayer->readyweapon)
            last_index = array_size(weapon_icons);

        if (viewplayer->weaponowned[weapon])
        {
            if (selectedweapon == weapon)
            {
                selected_index = array_size(weapon_icons);
                state = wpi_selected;
            }
            else if (!WeaponSelectable(weapon))
                state = wpi_disabled;
            else
                state = wpi_regular;
        }

        if (state != wpi_none)
        {
            weapon_icon_t   icon = {weapon, state};

            array_push(weapon_icons, icon);
        }
    }
}

void ST_UpdateCarousel(void)
{
    if (!weaponcarousel || automapactive || menuactive || paused || viewplayer->playerstate == PST_DEAD
        || !viewplayer->mo || viewplayer->mo->health <= 0)
    {
        ST_ResetCarousel();
        return;
    }

    BuildWeaponIcons();

    if (last_index == -1)
    {
        last_index = selected_index;
        return;
    }

    if (last_index != selected_index)
    {
        distance = selected_index - last_index;
        distance = 64 * MAX(-2, MIN(distance, 2));
        last_index = selected_index;
        last_time = I_GetTimeMS();
        duration = TICRATE / 2;
        fade = (smoothtransitions ? 0 : 4);
    }
    else if (duration > 0)
    {
        if (viewplayer->pendingweapon == wp_nochange)
            duration--;

        if (smoothtransitions && fade < 4)
            fade++;

        if (!duration && smoothtransitions)
            duration = -4;
    }
    else if (duration < 0)
    {
        fade--;
        duration++;
    }
}

static void CarouselDrawIcon(int x, int y, weapon_icon_t icon)
{
    char        lump[9] = { 0 };
    int         lumpnum;
    M_snprintf(lump, sizeof(lump), "%s%d", weaponinfo[icon.weapon].carouselicon,
        (icon.state == wpi_selected));

    if ((lumpnum = W_CheckNumForName(lump)) >= 0)
    {
        patch_t *patch = W_CacheLumpNum(lumpnum);

        V_DrawDropShadowPatch(x, y, 0, patch,
            (fade == 1 ? black10 : (fade == 2 ? black25 : black40)));

        if (fade == 4 && r_hud_translucency)
            V_DrawTranslucentPatch(x, y, 0, patch, tinttab80);
        else if (fade == 4)
            V_DrawPatch(x, y, 0, patch);
        else if (fade > 0)
            V_DrawTranslucentPatch(x, y, 0, patch,
                (fade == 1 ? tinttab25 : (fade == 2 ? tinttab50 : tinttab75)));
    }
}

static int CalcOffset(void)
{
    if (distance)
    {
        const uint64_t  delta = I_GetTimeMS() - last_time;

        if (delta < 125)
        {
            const float x = 1.0f - delta / 125.0f;

            return lroundf(distance * x * x);
        }

        distance = 0;
    }

    return 0;
}

void ST_DrawCarousel(int x, int y)
{
    int offset;

    if (!weaponcarousel || !duration || !array_size(weapon_icons) || fade <= 0)
        return;

    offset = x + CalcOffset();

    CarouselDrawIcon(offset, y, weapon_icons[selected_index]);

    for (int i = selected_index + 1, k = 1; i < array_size(weapon_icons) && k < 3; i++, k++)
        CarouselDrawIcon(offset + k * 64, y, weapon_icons[i]);

    for (int i = selected_index - 1, k = 1; i >= 0 && k < 3; i--, k++)
        CarouselDrawIcon(offset - k * 64, y, weapon_icons[i]);
}
