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
#include "d_options.h"
#include "i_system.h"
#include "m_config.h"
#include "m_misc.h"
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

bool D_IsOptionsColorOverridden(const char *name)
{
    if (M_StringCompare(name, "am_allmapcdwallcolor"))
        return (am_allmapcdwallcolor_options >= 0);
    else if (M_StringCompare(name, "am_allmapfdwallcolor"))
        return (am_allmapfdwallcolor_options >= 0);
    else if (M_StringCompare(name, "am_allmapwallcolor"))
        return (am_allmapwallcolor_options >= 0);
    else if (M_StringCompare(name, "am_bluedoorcolor"))
        return (am_bluedoorcolor_options >= 0);
    else if (M_StringCompare(name, "am_bluekeycolor"))
        return (am_bluekeycolor_options >= 0);
    else if (M_StringCompare(name, "am_cdwallcolor"))
        return (am_cdwallcolor_options >= 0);
    else if (M_StringCompare(name, "am_crosshaircolor"))
        return (am_crosshaircolor_options >= 0);
    else if (M_StringCompare(name, "am_fdwallcolor"))
        return (am_fdwallcolor_options >= 0);
    else if (M_StringCompare(name, "am_gridcolor"))
        return (am_gridcolor_options >= 0);
    else if (M_StringCompare(name, "am_reddoorcolor"))
        return (am_reddoorcolor_options >= 0);
    else if (M_StringCompare(name, "am_redkeycolor"))
        return (am_redkeycolor_options >= 0);
    else if (M_StringCompare(name, "am_secretcolor"))
        return (am_secretcolor_options >= 0);
    else if (M_StringCompare(name, "am_teleportercolor"))
        return (am_teleportercolor_options >= 0);
    else if (M_StringCompare(name, "am_thingcolor"))
        return (am_thingcolor_options >= 0);
    else if (M_StringCompare(name, "am_tswallcolor"))
        return (am_tswallcolor_options >= 0);
    else if (M_StringCompare(name, "am_wallcolor"))
        return (am_wallcolor_options >= 0);
    else if (M_StringCompare(name, "am_yellowdoorcolor"))
        return (am_yellowdoorcolor_options >= 0);
    else if (M_StringCompare(name, "am_yellowkeycolor"))
        return (am_yellowkeycolor_options >= 0);

    return false;
}
