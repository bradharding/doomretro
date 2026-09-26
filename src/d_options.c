/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

    This file is a part of DOOM Retro.

    DOOM Retro is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the license, or (at
    your option) any later version.

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

#include "am_map.h"
#include "c_console.h"
#include "d_options.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
#include "st_carousel.h"
#include "w_wad.h"

int am_allmapcdwallcolor_options = -1;
int am_allmapfdwallcolor_options = -1;
int am_allmapwallcolor_options = -1;
int am_bluedoorcolor_options = -1;
int am_bluekeycolor_options = -1;
int am_cdwallcolor_options = -1;
int am_crosshaircolor_options = -1;
int am_fdwallcolor_options = -1;
int am_gridcolor_options = -1;
int am_reddoorcolor_options = -1;
int am_redkeycolor_options = -1;
int am_secretcolor_options = -1;
int am_teleportercolor_options = -1;
int am_thingcolor_options = -1;
int am_tswallcolor_options = -1;
int am_wallcolor_options = -1;
int am_yellowdoorcolor_options = -1;
int am_yellowkeycolor_options = -1;
int weaponcarouselbordercolor_options = -1;
int weaponcarouselcolor_options = -1;
int weaponcarouselhighlightcolor_options = -1;

static void D_ProcessOptionsLump(const int lump)
{
    const int   length = W_LumpLength(lump);
    char        *data = I_Malloc(length + 1);
    char        *line = data;

    memcpy(data, W_CacheLumpNum(lump), length);
    data[length] = '\0';

    while (line < data + length)
    {
        char        name[64] = "";
        int         value;
        char        *end = memchr(line, '\n', data + length - line);
        const bool  newline = (end != NULL);

        if (!end)
            end = data + length;

        *end = '\0';

        if (sscanf(line, "%63s %i", name, &value) == 2 && value >= 0 && value <= 255)
        {
            if (M_StringCompare(name, "mapcolor_grid"))
            {
                am_gridcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_wall"))
            {
                am_wallcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_fchg"))
            {
                am_fdwallcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_cchg"))
            {
                am_cdwallcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_clsd"))
            {
                am_bluedoorcolor_options = value;
                am_reddoorcolor_options = value;
                am_yellowdoorcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_rkey"))
            {
                am_redkeycolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_bkey"))
            {
                am_bluekeycolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_ykey"))
            {
                am_yellowkeycolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_rdor"))
            {
                am_reddoorcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_bdor"))
            {
                am_bluedoorcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_ydor"))
            {
                am_yellowdoorcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_unsn"))
            {
                am_allmapwallcolor_options = value;
                am_allmapfdwallcolor_options = value;
                am_allmapcdwallcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_tele"))
            {
                am_teleportercolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_flat"))
            {
                am_tswallcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_sprt"))
            {
                am_thingcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_hair"))
            {
                am_crosshaircolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, "mapcolor_secr"))
            {
                am_secretcolor_options = value;
                AM_SetColors();
            }
            else if (M_StringCompare(name, stringize(weaponcarouselbordercolor)))
            {
                weaponcarouselbordercolor_options = value;
                ST_SetCarouselColors();
            }
            else if (M_StringCompare(name, stringize(weaponcarouselcolor)))
            {
                weaponcarouselcolor_options = value;
                ST_SetCarouselColors();
            }
            else if (M_StringCompare(name, stringize(weaponcarouselhighlightcolor)))
            {
                weaponcarouselhighlightcolor_options = value;
                ST_SetCarouselColors();
            }
        }

        line = (newline ? end + 1 : end);
    }

    W_ReleaseLumpNum(lump);
    free(data);
}

void D_ProcessOptionsInWad(void)
{
    for (int i = 0; i < numlumps; i++)
        if (M_StringCompare(lumpinfo[i]->name, "OPTIONS"))
            D_ProcessOptionsLump(i);
}

void D_ResetOptionsColor(const char *name)
{
    int value = -1;

    if (M_StringCompare(name, stringize(am_allmapcdwallcolor)))
    {
        value = am_allmapcdwallcolor_options;
        am_allmapcdwallcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_allmapfdwallcolor)))
    {
        value = am_allmapfdwallcolor_options;
        am_allmapfdwallcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_allmapwallcolor)))
    {
        value = am_allmapwallcolor_options;
        am_allmapwallcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_bluedoorcolor)))
    {
        value = am_bluedoorcolor_options;
        am_bluedoorcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_bluekeycolor)))
    {
        value = am_bluekeycolor_options;
        am_bluekeycolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_cdwallcolor)))
    {
        value = am_cdwallcolor_options;
        am_cdwallcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_crosshaircolor)))
    {
        value = am_crosshaircolor_options;
        am_crosshaircolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_fdwallcolor)))
    {
        value = am_fdwallcolor_options;
        am_fdwallcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_gridcolor)))
    {
        value = am_gridcolor_options;
        am_gridcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_reddoorcolor)))
    {
        value = am_reddoorcolor_options;
        am_reddoorcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_redkeycolor)))
    {
        value = am_redkeycolor_options;
        am_redkeycolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_secretcolor)))
    {
        value = am_secretcolor_options;
        am_secretcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_teleportercolor)))
    {
        value = am_teleportercolor_options;
        am_teleportercolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_thingcolor)))
    {
        value = am_thingcolor_options;
        am_thingcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_tswallcolor)))
    {
        value = am_tswallcolor_options;
        am_tswallcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_wallcolor)))
    {
        value = am_wallcolor_options;
        am_wallcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_yellowdoorcolor)))
    {
        value = am_yellowdoorcolor_options;
        am_yellowdoorcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(am_yellowkeycolor)))
    {
        value = am_yellowkeycolor_options;
        am_yellowkeycolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(weaponcarouselbordercolor)))
    {
        value = weaponcarouselbordercolor_options;
        weaponcarouselbordercolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(weaponcarouselcolor)))
    {
        value = weaponcarouselcolor_options;
        weaponcarouselcolor_options = -1;
    }
    else if (M_StringCompare(name, stringize(weaponcarouselhighlightcolor)))
    {
        value = weaponcarouselhighlightcolor_options;
        weaponcarouselhighlightcolor_options = -1;
    }

    if (value != -1)
        C_Warning(0, "It is no longer being overridden by {%i} from an " BOLD("OPTIONS") " lump.",
            value);
}

int *D_GetOptionsColor(const char *name)
{
    if (M_StringCompare(name, stringize(am_allmapcdwallcolor)))
        return &am_allmapcdwallcolor_options;
    else if (M_StringCompare(name, stringize(am_allmapfdwallcolor)))
        return &am_allmapfdwallcolor_options;
    else if (M_StringCompare(name, stringize(am_allmapwallcolor)))
        return &am_allmapwallcolor_options;
    else if (M_StringCompare(name, stringize(am_bluedoorcolor)))
        return &am_bluedoorcolor_options;
    else if (M_StringCompare(name, stringize(am_bluekeycolor)))
        return &am_bluekeycolor_options;
    else if (M_StringCompare(name, stringize(am_cdwallcolor)))
        return &am_cdwallcolor_options;
    else if (M_StringCompare(name, stringize(am_crosshaircolor)))
        return &am_crosshaircolor_options;
    else if (M_StringCompare(name, stringize(am_fdwallcolor)))
        return &am_fdwallcolor_options;
    else if (M_StringCompare(name, stringize(am_gridcolor)))
        return &am_gridcolor_options;
    else if (M_StringCompare(name, stringize(am_reddoorcolor)))
        return &am_reddoorcolor_options;
    else if (M_StringCompare(name, stringize(am_redkeycolor)))
        return &am_redkeycolor_options;
    else if (M_StringCompare(name, stringize(am_secretcolor)))
        return &am_secretcolor_options;
    else if (M_StringCompare(name, stringize(am_teleportercolor)))
        return &am_teleportercolor_options;
    else if (M_StringCompare(name, stringize(am_thingcolor)))
        return &am_thingcolor_options;
    else if (M_StringCompare(name, stringize(am_tswallcolor)))
        return &am_tswallcolor_options;
    else if (M_StringCompare(name, stringize(am_wallcolor)))
        return &am_wallcolor_options;
    else if (M_StringCompare(name, stringize(am_yellowdoorcolor)))
        return &am_yellowdoorcolor_options;
    else if (M_StringCompare(name, stringize(am_yellowkeycolor)))
        return &am_yellowkeycolor_options;
    else if (M_StringCompare(name, stringize(weaponcarouselbordercolor)))
        return &weaponcarouselbordercolor_options;
    else if (M_StringCompare(name, stringize(weaponcarouselcolor)))
        return &weaponcarouselcolor_options;
    else if (M_StringCompare(name, stringize(weaponcarouselhighlightcolor)))
        return &weaponcarouselhighlightcolor_options;

    return NULL;
}
