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

#include "d_items.h"
#include "info.h"
#include "sounds.h"
#include "states.h"

weaponinfo_t weaponinfo[NUMWEAPONS] =
{
    // fists
    {
        /* name        */ "fists",
        /* ammotype    */ am_noammo,
        /* ammoname    */ "",
        /* ammoplural  */ "",
        /* ammothing   */ MT_NULL,
        /* ammopershot */ 0,
        /* upstate     */ S_PUNCHUP,
        /* downstate   */ S_PUNCHDOWN,
        /* readystate  */ S_PUNCH,
        /* atkstate    */ S_PUNCH1,
        /* flashstate  */ S_NULL,
        /* recoil      */ 0,
        /* lowrumble   */ 37500,
        /* highrumble  */ 37500,
        /* tics        */ 10,
        /* prevweapon  */ wp_bfg,
        /* nextweapon  */ wp_chainsaw,
        /* spritename  */ "",
        /* sound       */ sfx_none,
        /* flags       */ (WPF_FLEEMELEE | WPF_AUTOSWITCHFROM | WPF_NOAUTOSWITCHTO),
        /* key         */ '1'
    },

    // pistol
    {
        /* name        */ "pistol",
        /* ammotype    */ am_clip,
        /* ammoname    */ "bullet",
        /* ammoplural  */ "bullets",
        /* ammothing   */ MT_CLIP,
        /* ammopershot */ 1,
        /* upstate     */ S_PISTOLUP,
        /* downstate   */ S_PISTOLDOWN,
        /* readystate  */ S_PISTOL,
        /* atkstate    */ S_PISTOL1,
        /* flashstate  */ S_PISTOLFLASH,
        /* recoil      */ 4,
        /* lowrumble   */ 35000,
        /* highrumble  */ 35000,
        /* tics        */ 8,
        /* prevweapon  */ wp_chainsaw,
        /* nextweapon  */ wp_shotgun,
        /* spritename  */ "",
        /* sound       */ sfx_pistol,
        /* flags       */ WPF_AUTOSWITCHFROM,
        /* key         */ '2'
    },

    // shotgun
    {
        /* name        */ original_mobjinfo[MT_SHOTGUN].name1,
        /* ammotype    */ am_shell,
        /* ammoname    */ "shell",
        /* ammoplural  */ "shells",
        /* ammothing   */ MT_MISC22,
        /* ammopershot */ 1,
        /* upstate     */ S_SGUNUP,
        /* downstate   */ S_SGUNDOWN,
        /* readystate  */ S_SGUN,
        /* atkstate    */ S_SGUN1,
        /* flashstate  */ S_SGUNFLASH1,
        /* recoil      */ 8,
        /* lowrumble   */ 45000,
        /* highrumble  */ 45000,
        /* tics        */ 14,
        /* prevweapon  */ wp_pistol,
        /* nextweapon  */ wp_supershotgun,
        /* spritename  */ "SHOTA0",
        /* sound       */ sfx_shotgn,
        /* flags       */ WPF_NOFLAG,
        /* key         */ '3'
    },

    // chaingun
    {
        /* name        */ original_mobjinfo[MT_CHAINGUN].name1,
        /* ammotype    */ am_clip,
        /* ammoname    */ "bullet",
        /* ammoplural  */ "bullets",
        /* ammothing   */ MT_CLIP,
        /* ammopershot */ 1,
        /* upstate     */ S_CHAINUP,
        /* downstate   */ S_CHAINDOWN,
        /* readystate  */ S_CHAIN,
        /* atkstate    */ S_CHAIN1,
        /* flashstate  */ S_CHAINFLASH1,
        /* recoil      */ 4,
        /* lowrumble   */ 35000,
        /* highrumble  */ 35000,
        /* tics        */ 8,
        /* prevweapon  */ wp_supershotgun,
        /* nextweapon  */ wp_missile,
        /* spritename  */ "MGUNA0",
        /* sound       */ sfx_pistol,
        /* flags       */ WPF_NOFLAG,
        /* key         */ '4'
    },

    // rocket launcher
    {
        /* name        */ original_mobjinfo[MT_MISC27].name1,
        /* ammotype    */ am_misl,
        /* ammoname    */ "rocket",
        /* ammoplural  */ "rockets",
        /* ammothing   */ MT_MISC18,
        /* ammopershot */ 1,
        /* upstate     */ S_MISSILEUP,
        /* downstate   */ S_MISSILEDOWN,
        /* readystate  */ S_MISSILE,
        /* atkstate    */ S_MISSILE1,
        /* flashstate  */ S_MISSILEFLASH1,
        /* recoil      */ 16,
        /* lowrumble   */ 60000,
        /* highrumble  */ 60000,
        /* tics        */ 20,
        /* prevweapon  */ wp_chaingun,
        /* nextweapon  */ wp_plasma,
        /* spritename  */ "LAUNA0",
        /* sound       */ sfx_rlaunc,
        /* flags       */ WPF_NOAUTOFIRE,
        /* key         */ '5'
    },

    // plasma rifle
    {
        /* name        */ original_mobjinfo[MT_MISC28].name1,
        /* ammotype    */ am_cell,
        /* ammoname    */ "cell",
        /* ammoplural  */ "cells",
        /* ammothing   */ MT_MISC20,
        /* ammopershot */ 1,
        /* upstate     */ S_PLASMAUP,
        /* downstate   */ S_PLASMADOWN,
        /* readystate  */ S_PLASMA,
        /* atkstate    */ S_PLASMA1,
        /* flashstate  */ S_PLASMAFLASH1,
        /* recoil      */ 4,
        /* lowrumble   */ 50000,
        /* highrumble  */ 50000,
        /* tics        */ 10,
        /* prevweapon  */ wp_missile,
        /* nextweapon  */ wp_bfg,
        /* spritename  */ "PLASA0",
        /* sound       */ sfx_plasma,
        /* flags       */ WPF_NOFLAG,
        /* key         */ '6'
    },

    // BFG-9000
    {
        /* name        */ original_mobjinfo[MT_MISC25].name1,
        /* ammotype    */ am_cell,
        /* ammoname    */ "cell",
        /* ammoplural  */ "cells",
        /* ammothing   */ MT_MISC20,
        /* ammopershot */ BFGCELLS,
        /* upstate     */ S_BFGUP,
        /* downstate   */ S_BFGDOWN,
        /* readystate  */ S_BFG,
        /* atkstate    */ S_BFG1,
        /* flashstate  */ S_BFGFLASH1,
        /* recoil      */ 16,
        /* lowrumble   */ 50000,
        /* highrumble  */ 50000,
        /* tics        */ 30,
        /* prevweapon  */ wp_plasma,
        /* nextweapon  */ wp_fist,
        /* spritename  */ "BFUGA0",
        /* sound       */ sfx_bfg,
        /* flags       */ WPF_NOAUTOFIRE,
        /* key         */ '7'
    },

    // chainsaw
    {
        /* name        */ original_mobjinfo[MT_MISC26].name1,
        /* ammotype    */ am_noammo,
        /* ammoname    */ "",
        /* ammoplural  */ "",
        /* ammothing   */ MT_NULL,
        /* ammopershot */ 0,
        /* upstate     */ S_SAWUP,
        /* downstate   */ S_SAWDOWN,
        /* readystate  */ S_SAW,
        /* atkstate    */ S_SAW1,
        /* flashstate  */ S_NULL,
        /* recoil      */ -2,
        /* lowrumble   */ 50000,
        /* highrumble  */ 50000,
        /* tics        */ 10,
        /* prevweapon  */ wp_fist,
        /* nextweapon  */ wp_pistol,
        /* spritename  */ "CSAWA0",
        /* sound       */ sfx_none,
        /* flags       */ (WPF_NOTHRUST | WPF_FLEEMELEE | WPF_NOAUTOSWITCHTO),
        /* key         */ '1'
    },

    // super shotgun
    {
        /* name        */ original_mobjinfo[MT_SUPERSHOTGUN].name1,
        /* ammotype    */ am_shell,
        /* ammoname    */ "shell",
        /* ammoplural  */ "shells",
        /* ammothing   */ MT_MISC22,
        /* ammopershot */ 2,
        /* upstate     */ S_DSGUNUP,
        /* downstate   */ S_DSGUNDOWN,
        /* readystate  */ S_DSGUN,
        /* atkstate    */ S_DSGUN1,
        /* flashstate  */ S_DSGUNFLASH1,
        /* recoil      */ 16,
        /* lowrumble   */ 60000,
        /* highrumble  */ 60000,
        /* tics        */ 14,
        /* prevweapon  */ wp_shotgun,
        /* nextweapon  */ wp_chaingun,
        /* spritename  */ "SGN2A0",
        /* sound       */ sfx_dshtgn,
        /* flags       */ WPF_NOFLAG,
        /* key         */ '3'
    }
};

char *powerups[NUMPOWERS] =
{
    "",
    original_mobjinfo[MT_INV].name1,     // invincibility
    original_mobjinfo[MT_MISC13].name1,  // berserk
    original_mobjinfo[MT_INS].name1,     // partial invisibility
    original_mobjinfo[MT_MISC14].name1,  // radiation shielding suit
    original_mobjinfo[MT_MISC15].name1,  // computer area map
    original_mobjinfo[MT_MISC16].name1   // light amplification visor
};
