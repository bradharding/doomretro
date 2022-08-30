/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#include "d_items.h"
#include "info.h"
#include "states.h"

weaponinfo_t weaponinfo[NUMWEAPONS] =
{
    {
        /* name        */ "fists",
        /* ammotype    */ am_noammo,
        /* ammoname    */ "",
        /* ammoplural  */ "",
        /* ammopershot */ 0,
        /* upstate     */ S_PUNCHUP,
        /* downstate   */ S_PUNCHDOWN,
        /* readystate  */ S_PUNCH,
        /* atkstate    */ S_PUNCH1,
        /* flashstate  */ S_NULL,
        /* recoil      */ 0,
        /* strength    */ 37500,
        /* tics        */ 10,
        /* prev        */ wp_bfg,
        /* next        */ wp_chainsaw,
        /* spritename  */ "",
        /* flags       */ (WPF_FLEEMELEE | WPF_AUTOSWITCHFROM | WPF_NOAUTOSWITCHTO)
    },

    {
        /* name        */ "pistol",
        /* ammotype    */ am_clip,
        /* ammoname    */ "bullet",
        /* ammoplural  */ "bullets",
        /* ammopershot */ 1,
        /* upstate     */ S_PISTOLUP,
        /* downstate   */ S_PISTOLDOWN,
        /* readystate  */ S_PISTOL,
        /* atkstate    */ S_PISTOL1,
        /* flashstate  */ S_PISTOLFLASH,
        /* recoil      */ 4,
        /* strength    */ 45000,
        /* tics        */ 10,
        /* prev        */ wp_chainsaw,
        /* next        */ wp_shotgun,
        /* spritename  */ "",
        /* flags       */ WPF_AUTOSWITCHFROM
    },

    {
        /* name        */ "shotgun",
        /* ammotype    */ am_shell,
        /* ammoname    */ "shell",
        /* ammoplural  */ "shells",
        /* ammopershot */ 1,
        /* upstate     */ S_SGUNUP,
        /* downstate   */ S_SGUNDOWN,
        /* readystate  */ S_SGUN,
        /* atkstate    */ S_SGUN1,
        /* flashstate  */ S_SGUNFLASH1,
        /* recoil      */ 8,
        /* strength    */ 52500,
        /* tics        */ 10,
        /* prev        */ wp_pistol,
        /* next        */ wp_supershotgun,
        /* spritename  */ "SHOTA0",
        /* flags       */ WPF_NOFLAG
    },

    {
        /* name        */ "chaingun",
        /* ammotype    */ am_clip,
        /* ammoname    */ "bullet",
        /* ammoplural  */ "bullets",
        /* ammopershot */ 1,
        /* upstate     */ S_CHAINUP,
        /* downstate   */ S_CHAINDOWN,
        /* readystate  */ S_CHAIN,
        /* atkstate    */ S_CHAIN1,
        /* flashstate  */ S_CHAINFLASH1,
        /* recoil      */ 4,
        /* strength    */ 45000,
        /* tics        */ 10,
        /* prev        */ wp_supershotgun,
        /* next        */ wp_missile,
        /* spritename  */ "MGUNA0",
        /* flags       */ WPF_NOFLAG
    },

    {
        /* name        */ "rocket launcher",
        /* ammotype    */ am_misl,
        /* ammoname    */ "rocket",
        /* ammoplural  */ "rockets",
        /* ammopershot */ 1,
        /* upstate     */ S_MISSILEUP,
        /* downstate   */ S_MISSILEDOWN,
        /* readystate  */ S_MISSILE,
        /* atkstate    */ S_MISSILE1,
        /* flashstate  */ S_MISSILEFLASH1,
        /* recoil      */ 16,
        /* strength    */ 60000,
        /* tics        */ 20,
        /* prev        */ wp_chaingun,
        /* next        */ wp_plasma,
        /* spritename  */ "LAUNA0",
        /* flags       */ WPF_NOAUTOFIRE
    },

    {
        /* name        */ "plasma rifle",
        /* ammotype    */ am_cell,
        /* ammoname    */ "cell",
        /* ammoplural  */ "cells",
        /* ammopershot */ 1,
        /* upstate     */ S_PLASMAUP,
        /* downstate   */ S_PLASMADOWN,
        /* readystate  */ S_PLASMA,
        /* atkstate    */ S_PLASMA1,
        /* flashstate  */ S_PLASMAFLASH1,
        /* recoil      */ 4,
        /* strength    */ 52500,
        /* tics        */ 10,
        /* prev        */ wp_missile,
        /* next        */ wp_bfg,
        /* spritename  */ "PLASA0",
        /* flags       */ WPF_NOFLAG
    },

    {
        /* name        */ "BFG-9000",
        /* ammotype    */ am_cell,
        /* ammoname    */ "cell",
        /* ammoplural  */ "cells",
        /* ammopershot */ BFGCELLS,
        /* upstate     */ S_BFGUP,
        /* downstate   */ S_BFGDOWN,
        /* readystate  */ S_BFG,
        /* atkstate    */ S_BFG1,
        /* flashstate  */ S_BFGFLASH1,
        /* recoil      */ 16,
        /* strength    */ 45000,
        /* tics        */ 30,
        /* prev        */ wp_plasma,
        /* next        */ wp_fist,
        /* spritename  */ "BFUGA0",
        /* flags       */ WPF_NOAUTOFIRE
    },

    {
        /* name        */ "chainsaw",
        /* ammotype    */ am_noammo,
        /* ammoname    */ "",
        /* ammoplural  */ "",
        /* ammopershot */ 0,
        /* upstate     */ S_SAWUP,
        /* downstate   */ S_SAWDOWN,
        /* readystate  */ S_SAW,
        /* atkstate    */ S_SAW1,
        /* flashstate  */ S_NULL,
        /* recoil      */ -2,
        /* strength    */ 45000,
        /* tics        */ 10,
        /* prev        */ wp_fist,
        /* next        */ wp_pistol,
        /* spritename  */ "CSAWA0",
        /* flags       */ (WPF_NOTHRUST | WPF_FLEEMELEE | WPF_NOAUTOSWITCHTO)
    },

    {
        /* name        */ "super shotgun",
        /* ammotype    */ am_shell,
        /* ammoname    */ "shell",
        /* ammoplural  */ "shells",
        /* ammopershot */ 2,
        /* upstate     */ S_DSGUNUP,
        /* downstate   */ S_DSGUNDOWN,
        /* readystate  */ S_DSGUN,
        /* atkstate    */ S_DSGUN1,
        /* flashstate  */ S_DSGUNFLASH1,
        /* recoil      */ 16,
        /* strength    */ 60000,
        /* tics        */ 10,
        /* prev        */ wp_shotgun,
        /* next        */ wp_chaingun,
        /* spritename  */ "SGN2A0",
        /* flags       */ WPF_NOFLAG
    }
};

char *powerupnames[NUMPOWERS] =
{
    "",
    (char *)mobjinfo[MT_INV].name1,
    (char *)mobjinfo[MT_MISC13].name1,
    (char *)mobjinfo[MT_INS].name1,
    (char *)mobjinfo[MT_MISC14].name1,
    (char *)mobjinfo[MT_MISC15].name1,
    (char *)mobjinfo[MT_MISC16].name1
};
