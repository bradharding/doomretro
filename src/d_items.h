/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

#pragma once

#include "doomdef.h"
#include "doomtype.h"
#include "r_defs.h"
#include "sounds.h"

#define BFGCELLS    40

//
// MBF21: haleyjd 09/11/07: weapon flags
//
enum
{
    WPF_NOFLAG         = 0x00000000,    // no flag
    WPF_NOTHRUST       = 0x00000001,    // doesn't thrust Mobj's
    WPF_SILENT         = 0x00000002,    // weapon is silent
    WPF_NOAUTOFIRE     = 0x00000004,    // weapon won't autofire in A_WeaponReady
    WPF_FLEEMELEE      = 0x00000008,    // monsters consider it a melee weapon
    WPF_AUTOSWITCHFROM = 0x00000010,    // can be switched away from when ammo is picked up
    WPF_NOAUTOSWITCHTO = 0x00000020     // cannot be switched to when ammo is picked up
};

// Weapon info: sprite frames, ammunition use.
typedef struct
{
    char            *name;
    ammotype_t      ammotype;
    char            ammoname[255];
    char            ammoplural[255];
    int             ammothing;
    int             ammopershot;
    int             upstate;
    int             downstate;
    int             readystate;
    int             atkstate;
    int             flashstate;
    int             recoil;
    int             lowrumble;
    int             highrumble;
    int             tics;
    weapontype_t    prevweapon;
    weapontype_t    nextweapon;
    char            spritename[9];
    sfxnum_t        sound;
    int             flags;  // MBF21
    char            key;
    bool            altered;
    patch_t         *ammopatch;
} weaponinfo_t;

extern weaponinfo_t weaponinfo[NUMWEAPONS];
extern char         *powerups[NUMPOWERS];
