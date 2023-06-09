/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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
int keyboardautomap = KEYAUTOMAP_DEFAULT;
int keyboardback = KEYDOWN_DEFAULT;
int keyboardback2 = KEYDOWN2_DEFAULT;
int keyboardclearmark = KEYCLEARMARK_DEFAULT;
int keyboardconsole = KEYCONSOLE_DEFAULT;
int keyboardfire = KEYFIRE_DEFAULT;
int keyboardfollowmode = KEYFOLLOWMODE_DEFAULT;
int keyboardforward = KEYUP_DEFAULT;
int keyboardforward2 = KEYUP2_DEFAULT;
int keyboardgrid = KEYGRID_DEFAULT;
int keyboardjump = KEYJUMP_DEFAULT;
int keyboardleft = KEYLEFT_DEFAULT;
int keyboardmark = KEYMARK_DEFAULT;
int keyboardmaxzoom = KEYMAXZOOM_DEFAULT;
int keyboardmenu = KEYMENU_DEFAULT;
int keyboardmouselook = KEYMOUSELOOK_DEFAULT;
int keyboardnextweapon = KEYNEXTWEAPON_DEFAULT;
int keyboardprevweapon = KEYPREVWEAPON_DEFAULT;
int keyboardright = KEYRIGHT_DEFAULT;
int keyboardrotatemode = KEYROTATEMODE_DEFAULT;
int keyboardrun = KEYRUN_DEFAULT;
int keyboardscreenshot = KEYSCREENSHOT_DEFAULT;
int keyboardstrafe = KEYSTRAFE_DEFAULT;
int keyboardstrafeleft = KEYSTRAFELEFT_DEFAULT;
int keyboardstrafeleft2 = KEYSTRAFELEFT2_DEFAULT;
int keyboardstraferight = KEYSTRAFERIGHT_DEFAULT;
int keyboardstraferight2 = KEYSTRAFERIGHT2_DEFAULT;
int keyboarduse = KEYUSE_DEFAULT;
int keyboarduse2 = KEYUSE2_DEFAULT;
int keyboardweapon1 = KEYWEAPON1_DEFAULT;
int keyboardweapon2 = KEYWEAPON2_DEFAULT;
int keyboardweapon3 = KEYWEAPON3_DEFAULT;
int keyboardweapon4 = KEYWEAPON4_DEFAULT;
int keyboardweapon5 = KEYWEAPON5_DEFAULT;
int keyboardweapon6 = KEYWEAPON6_DEFAULT;
int keyboardweapon7 = KEYWEAPON7_DEFAULT;
int keyboardzoomin = KEYZOOMIN_DEFAULT;
int keyboardzoomout = KEYZOOMOUT_DEFAULT;

int mouseback = MOUSEBACK_DEFAULT;
int mousefire = MOUSEFIRE_DEFAULT;
int mouseforward = MOUSEFORWARD_DEFAULT;
int mousejump = MOUSEJUMP_DEFAULT;
int mouseleft = MOUSELEFT_DEFAULT;
int mousemouselook = MOUSEMOUSELOOK_DEFAULT;
int mousenextweapon = MOUSENEXTWEAPON_DEFAULT;
int mouseprevweapon = MOUSEPREVWEAPON_DEFAULT;
int mouseright = MOUSERIGHT_DEFAULT;
int mouserun = MOUSERUN_DEFAULT;
int mousescreenshot = MOUSESCREENSHOT_DEFAULT;
int mousestrafe = MOUSESTRAFE_DEFAULT;
int mousestrafeleft = MOUSESTRAFELEFT_DEFAULT;
int mousestraferight = MOUSESTRAFERIGHT_DEFAULT;
int mouseuse = MOUSEUSE_DEFAULT;
int mouseweapon1 = MOUSEWEAPON1_DEFAULT;
int mouseweapon2 = MOUSEWEAPON2_DEFAULT;
int mouseweapon3 = MOUSEWEAPON3_DEFAULT;
int mouseweapon4 = MOUSEWEAPON4_DEFAULT;
int mouseweapon5 = MOUSEWEAPON5_DEFAULT;
int mouseweapon6 = MOUSEWEAPON6_DEFAULT;
int mouseweapon7 = MOUSEWEAPON7_DEFAULT;

int gamecontrolleralwaysrun = GAMECONTROLLERALWAYSRUN_DEFAULT;
int gamecontrollerautomap = GAMECONTROLLERAUTOMAP_DEFAULT;
int gamecontrollerback = GAMECONTROLLERBACK_DEFAULT;
int gamecontrollerclearmark = GAMECONTROLLERCLEARMARK_DEFAULT;
int gamecontrollerconsole = GAMECONTROLLERCONSOLE_DEFAULT;
int gamecontrollerfire = GAMECONTROLLERFIRE_DEFAULT;
int gamecontrollerfollowmode = GAMECONTROLLERFOLLOWMODE_DEFAULT;
int gamecontrollerforward = GAMECONTROLLERFORWARD_DEFAULT;
int gamecontrollergrid = GAMECONTROLLERGRID_DEFAULT;
int gamecontrollerjump = GAMECONTROLLERJUMP_DEFAULT;
int gamecontrollerleft = GAMECONTROLLERLEFT_DEFAULT;
int gamecontrollermark = GAMECONTROLLERMARK_DEFAULT;
int gamecontrollermaxzoom = GAMECONTROLLERMAXZOOM_DEFAULT;
int gamecontrollermenu = GAMECONTROLLERMENU_DEFAULT;
int gamecontrollermouselook = GAMECONTROLLERMOUSELOOK_DEFAULT;
int gamecontrollernextweapon = GAMECONTROLLERNEXTWEAPON_DEFAULT;
int gamecontrollerprevweapon = GAMECONTROLLERPREVWEAPON_DEFAULT;
int gamecontrollerright = GAMECONTROLLERRIGHT_DEFAULT;
int gamecontrollerrotatemode = GAMECONTROLLERROTATEMODE_DEFAULT;
int gamecontrollerrun = GAMECONTROLLERRUN_DEFAULT;
int gamecontrollerstrafe = GAMECONTROLLERSTRAFE_DEFAULT;
int gamecontrollerstrafeleft = GAMECONTROLLERSTRAFELEFT_DEFAULT;
int gamecontrollerstraferight = GAMECONTROLLERSTRAFERIGHT_DEFAULT;
int gamecontrolleruse = GAMECONTROLLERUSE_DEFAULT;
int gamecontrolleruse2 = GAMECONTROLLERUSE2_DEFAULT;
int gamecontrollerweapon1 = GAMECONTROLLERWEAPON_DEFAULT;
int gamecontrollerweapon2 = GAMECONTROLLERWEAPON_DEFAULT;
int gamecontrollerweapon3 = GAMECONTROLLERWEAPON_DEFAULT;
int gamecontrollerweapon4 = GAMECONTROLLERWEAPON_DEFAULT;
int gamecontrollerweapon5 = GAMECONTROLLERWEAPON_DEFAULT;
int gamecontrollerweapon6 = GAMECONTROLLERWEAPON_DEFAULT;
int gamecontrollerweapon7 = GAMECONTROLLERWEAPON_DEFAULT;
int gamecontrollerzoomin = GAMECONTROLLERZOOMIN_DEFAULT;
int gamecontrollerzoomout = GAMECONTROLLERZOOMOUT_DEFAULT;
