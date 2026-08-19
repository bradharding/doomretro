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

#pragma once

extern int  am_allmapcdwallcolor_options;
extern int  am_allmapfdwallcolor_options;
extern int  am_allmapwallcolor_options;
extern int  am_bluedoorcolor_options;
extern int  am_bluekeycolor_options;
extern int  am_cdwallcolor_options;
extern int  am_crosshaircolor_options;
extern int  am_fdwallcolor_options;
extern int  am_gridcolor_options;
extern int  am_reddoorcolor_options;
extern int  am_redkeycolor_options;
extern int  am_secretcolor_options;
extern int  am_teleportercolor_options;
extern int  am_thingcolor_options;
extern int  am_tswallcolor_options;
extern int  am_wallcolor_options;
extern int  am_yellowdoorcolor_options;
extern int  am_yellowkeycolor_options;

void D_ProcessOptionsInWad(void);
