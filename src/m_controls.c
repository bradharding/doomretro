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

#include "m_config.h"

int keyboardalwaysrun = KEYALWAYSRUN_DEFAULT;
int keyboardalwaysrun2 = KEYALWAYSRUN2_DEFAULT;
int keyboardautomap = KEYAUTOMAP_DEFAULT;
int keyboardautomap2 = KEYAUTOMAP2_DEFAULT;
int keyboardback = KEYDOWN_DEFAULT;
int keyboardback2 = KEYDOWN2_DEFAULT;
int keyboardbfg9000 = KEYBFG9000_DEFAULT;
int keyboardbfg90002 = KEYBFG90002_DEFAULT;
int keyboardchaingun = KEYCHAINGUN_DEFAULT;
int keyboardchaingun2 = KEYCHAINGUN2_DEFAULT;
int keyboardchainsaw = KEYCHAINSAW_DEFAULT;
int keyboardchainsaw2 = KEYCHAINSAW2_DEFAULT;
int keyboardclearmark = KEYCLEARMARK_DEFAULT;
int keyboardclearmark2 = KEYCLEARMARK2_DEFAULT;
int keyboardconsole = KEYCONSOLE_DEFAULT;
int keyboardconsole2 = KEYCONSOLE2_DEFAULT;
int keyboardfire = KEYFIRE_DEFAULT;
int keyboardfire2 = KEYFIRE2_DEFAULT;
int keyboardfists = KEYFISTS_DEFAULT;
int keyboardfists2 = KEYFISTS2_DEFAULT;
int keyboardfollowmode = KEYFOLLOWMODE_DEFAULT;
int keyboardfollowmode2 = KEYFOLLOWMODE2_DEFAULT;
int keyboardforward = KEYUP_DEFAULT;
int keyboardforward2 = KEYUP2_DEFAULT;
int keyboardfreelook = KEYFREELOOK_DEFAULT;
int keyboardfreelook2 = KEYFREELOOK2_DEFAULT;
int keyboardgrid = KEYGRID_DEFAULT;
int keyboardgrid2 = KEYGRID2_DEFAULT;
int keyboardjump = KEYJUMP_DEFAULT;
int keyboardjump2 = KEYJUMP2_DEFAULT;
int keyboardleft = KEYLEFT_DEFAULT;
int keyboardleft2 = KEYLEFT2_DEFAULT;
int keyboardmark = KEYMARK_DEFAULT;
int keyboardmark2 = KEYMARK2_DEFAULT;
int keyboardmaxzoom = KEYMAXZOOM_DEFAULT;
int keyboardmaxzoom2 = KEYMAXZOOM2_DEFAULT;
int keyboardmenu = KEYMENU_DEFAULT;
int keyboardmenu2 = KEYMENU2_DEFAULT;
int keyboardnextweapon = KEYNEXTWEAPON_DEFAULT;
int keyboardnextweapon2 = KEYNEXTWEAPON2_DEFAULT;
int keyboardpath = KEYPATH_DEFAULT;
int keyboardpath2 = KEYPATH2_DEFAULT;
int keyboardpistol = KEYPISTOL_DEFAULT;
int keyboardpistol2 = KEYPISTOL2_DEFAULT;
int keyboardplasmarifle = KEYPLASMARIFLE_DEFAULT;
int keyboardplasmarifle2 = KEYPLASMARIFLE2_DEFAULT;
int keyboardprevweapon = KEYPREVWEAPON_DEFAULT;
int keyboardprevweapon2 = KEYPREVWEAPON2_DEFAULT;
int keyboardright = KEYRIGHT_DEFAULT;
int keyboardright2 = KEYRIGHT2_DEFAULT;
int keyboardrocketlauncher = KEYROCKETLAUNCHER_DEFAULT;
int keyboardrocketlauncher2 = KEYROCKETLAUNCHER2_DEFAULT;
int keyboardrotatemode = KEYROTATEMODE_DEFAULT;
int keyboardrotatemode2 = KEYROTATEMODE2_DEFAULT;
int keyboardrun = KEYRUN_DEFAULT;
int keyboardrun2 = KEYRUN2_DEFAULT;
int keyboardscreenshot = KEYSCREENSHOT_DEFAULT;
int keyboardscreenshot2 = KEYSCREENSHOT2_DEFAULT;
int keyboardshotgun = KEYSHOTGUN_DEFAULT;
int keyboardshotgun2 = KEYSHOTGUN2_DEFAULT;
int keyboardstrafe = KEYSTRAFE_DEFAULT;
int keyboardstrafe2 = KEYSTRAFE2_DEFAULT;
int keyboardstrafeleft = KEYSTRAFELEFT_DEFAULT;
int keyboardstrafeleft2 = KEYSTRAFELEFT2_DEFAULT;
int keyboardstraferight = KEYSTRAFERIGHT_DEFAULT;
int keyboardstraferight2 = KEYSTRAFERIGHT2_DEFAULT;
int keyboardsupershotgun = KEYSUPERSHOTGUN_DEFAULT;
int keyboardsupershotgun2 = KEYSUPERSHOTGUN2_DEFAULT;
int keyboarduse = KEYUSE_DEFAULT;
int keyboarduse2 = KEYUSE2_DEFAULT;
int keyboardweapon1 = KEYWEAPON1_DEFAULT;
int keyboardweapon1_2 = KEYWEAPON1_2_DEFAULT;
int keyboardweapon2 = KEYWEAPON2_DEFAULT;
int keyboardweapon2_2 = KEYWEAPON2_2_DEFAULT;
int keyboardweapon3 = KEYWEAPON3_DEFAULT;
int keyboardweapon3_2 = KEYWEAPON3_2_DEFAULT;
int keyboardweapon4 = KEYWEAPON4_DEFAULT;
int keyboardweapon4_2 = KEYWEAPON4_2_DEFAULT;
int keyboardweapon5 = KEYWEAPON5_DEFAULT;
int keyboardweapon5_2 = KEYWEAPON5_2_DEFAULT;
int keyboardweapon6 = KEYWEAPON6_DEFAULT;
int keyboardweapon6_2 = KEYWEAPON6_2_DEFAULT;
int keyboardweapon7 = KEYWEAPON7_DEFAULT;
int keyboardweapon7_2 = KEYWEAPON7_2_DEFAULT;
int keyboardzoomin = KEYZOOMIN_DEFAULT;
int keyboardzoomin2 = KEYZOOMIN2_DEFAULT;
int keyboardzoomout = KEYZOOMOUT_DEFAULT;
int keyboardzoomout2 = KEYZOOMOUT2_DEFAULT;

int mousealwaysrun = MOUSEALWAYSRUN_DEFAULT;
int mouseautomap = MOUSEAUTOMAP_DEFAULT;
int mouseback = MOUSEBACK_DEFAULT;
int mousebfg9000 = MOUSEBFG9000_DEFAULT;
int mousechaingun = MOUSECHAINGUN_DEFAULT;
int mousechainsaw = MOUSECHAINSAW_DEFAULT;
int mouseclearmark = MOUSECLEARMARK_DEFAULT;
int mouseconsole = MOUSECONSOLE_DEFAULT;
int mousefire = MOUSEFIRE_DEFAULT;
int mousefists = MOUSEFISTS_DEFAULT;
int mousefollowmode = MOUSEFOLLOWMODE_DEFAULT;
int mouseforward = MOUSEFORWARD_DEFAULT;
int mousefreelook = MOUSEFREELOOK_DEFAULT;
int mousegrid = MOUSEGRID_DEFAULT;
int mousejump = MOUSEJUMP_DEFAULT;
int mouseleft = MOUSELEFT_DEFAULT;
int mousemark = MOUSEMARK_DEFAULT;
int mousemaxzoom = MOUSEMAXZOOM_DEFAULT;
int mousemenu = MOUSEMENU_DEFAULT;
int mousenextweapon = MOUSENEXTWEAPON_DEFAULT;
int mousepath = MOUSEPATH_DEFAULT;
int mousepistol = MOUSEPISTOL_DEFAULT;
int mouseplasmarifle = MOUSEPLASMARIFLE_DEFAULT;
int mouseprevweapon = MOUSEPREVWEAPON_DEFAULT;
int mouseright = MOUSERIGHT_DEFAULT;
int mouserocketlauncher = MOUSEROCKETLAUNCHER_DEFAULT;
int mouserotatemode = MOUSEROTATEMODE_DEFAULT;
int mouserun = MOUSERUN_DEFAULT;
int mousescreenshot = MOUSESCREENSHOT_DEFAULT;
int mouseshotgun = MOUSESHOTGUN_DEFAULT;
int mousestrafe = MOUSESTRAFE_DEFAULT;
int mousestrafeleft = MOUSESTRAFELEFT_DEFAULT;
int mousestraferight = MOUSESTRAFERIGHT_DEFAULT;
int mousesupershotgun = MOUSESUPERSHOTGUN_DEFAULT;
int mouseuse = MOUSEUSE_DEFAULT;
int mouseweapon1 = MOUSEWEAPON1_DEFAULT;
int mouseweapon2 = MOUSEWEAPON2_DEFAULT;
int mouseweapon3 = MOUSEWEAPON3_DEFAULT;
int mouseweapon4 = MOUSEWEAPON4_DEFAULT;
int mouseweapon5 = MOUSEWEAPON5_DEFAULT;
int mouseweapon6 = MOUSEWEAPON6_DEFAULT;
int mouseweapon7 = MOUSEWEAPON7_DEFAULT;
int mousezoomin = MOUSEZOOMIN_DEFAULT;
int mousezoomout = MOUSEZOOMOUT_DEFAULT;

int controlleralwaysrun = CONTROLLERALWAYSRUN_DEFAULT;
int controllerautomap = CONTROLLERAUTOMAP_DEFAULT;
int controllerback = CONTROLLERBACK_DEFAULT;
int controllerbfg9000 = CONTROLLERBFG9000_DEFAULT;
int controllerchaingun = CONTROLLERCHAINGUN_DEFAULT;
int controllerchainsaw = CONTROLLERCHAINSAW_DEFAULT;
int controllerclearmark = CONTROLLERCLEARMARK_DEFAULT;
int controllerconsole = CONTROLLERCONSOLE_DEFAULT;
int controllerfire = CONTROLLERFIRE_DEFAULT;
int controllerfists = CONTROLLERFISTS_DEFAULT;
int controllerfollowmode = CONTROLLERFOLLOWMODE_DEFAULT;
int controllerforward = CONTROLLERFORWARD_DEFAULT;
int controllerfreelook = CONTROLLERFREELOOK_DEFAULT;
int controllergrid = CONTROLLERGRID_DEFAULT;
int controllerjump = CONTROLLERJUMP_DEFAULT;
int controllerleft = CONTROLLERLEFT_DEFAULT;
int controllermark = CONTROLLERMARK_DEFAULT;
int controllermaxzoom = CONTROLLERMAXZOOM_DEFAULT;
int controllermenu = CONTROLLERMENU_DEFAULT;
int controllernextweapon = CONTROLLERNEXTWEAPON_DEFAULT;
int controllerpath = CONTROLLERPATH_DEFAULT;
int controllerpistol = CONTROLLERPISTOL_DEFAULT;
int controllerplasmarifle = CONTROLLERPLASMARIFLE_DEFAULT;
int controllerprevweapon = CONTROLLERPREVWEAPON_DEFAULT;
int controllerright = CONTROLLERRIGHT_DEFAULT;
int controllerrocketlauncher = CONTROLLERROCKETLAUNCHER_DEFAULT;
int controllerrotatemode = CONTROLLERROTATEMODE_DEFAULT;
int controllerrun = CONTROLLERRUN_DEFAULT;
int controllerscreenshot = CONTROLLERSCREENSHOT_DEFAULT;
int controllershotgun = CONTROLLERSHOTGUN_DEFAULT;
int controllerstrafe = CONTROLLERSTRAFE_DEFAULT;
int controllerstrafeleft = CONTROLLERSTRAFELEFT_DEFAULT;
int controllerstraferight = CONTROLLERSTRAFERIGHT_DEFAULT;
int controllersupershotgun = CONTROLLERSUPERSHOTGUN_DEFAULT;
int controlleruse = CONTROLLERUSE_DEFAULT;
int controlleruse2 = CONTROLLERUSE2_DEFAULT;
int controllerweapon1 = CONTROLLERWEAPON1_DEFAULT;
int controllerweapon2 = CONTROLLERWEAPON2_DEFAULT;
int controllerweapon3 = CONTROLLERWEAPON3_DEFAULT;
int controllerweapon4 = CONTROLLERWEAPON4_DEFAULT;
int controllerweapon5 = CONTROLLERWEAPON5_DEFAULT;
int controllerweapon6 = CONTROLLERWEAPON6_DEFAULT;
int controllerweapon7 = CONTROLLERWEAPON7_DEFAULT;
int controllerzoomin = CONTROLLERZOOMIN_DEFAULT;
int controllerzoomout = CONTROLLERZOOMOUT_DEFAULT;
