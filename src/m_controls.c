/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "i_gamepad.h"
#include "m_config.h"
#include "p_local.h"

int     key_alwaysrun = KEYALWAYSRUN_DEFAULT;
int     key_automap = KEYAUTOMAP_DEFAULT;
int     key_automap_clearmark = KEYAUTOMAPCLEARMARK_DEFAULT;
int     key_automap_followmode = KEYAUTOMAPFOLLOWMODE_DEFAULT;
int     key_automap_grid = KEYAUTOMAPGRID_DEFAULT;
int     key_automap_mark = KEYAUTOMAPMARK_DEFAULT;
int     key_automap_maxzoom = KEYAUTOMAPMAXZOOM_DEFAULT;
int     key_automap_rotatemode = KEYAUTOMAPROTATEMODE_DEFAULT;
int     key_automap_zoomin = KEYAUTOMAPZOOMIN_DEFAULT;
int     key_automap_zoomout = KEYAUTOMAPZOOMOUT_DEFAULT;
int     key_console = KEYCONSOLE_DEFAULT;
int     key_down = KEYDOWN_DEFAULT;
int     key_down2 = KEYDOWN2_DEFAULT;
int     key_fire = KEYFIRE_DEFAULT;
int     key_left = KEYLEFT_DEFAULT;
int     key_menu = KEY_ESCAPE;
int     key_nextweapon = KEYNEXTWEAPON_DEFAULT;
int     key_prevweapon = KEYPREVWEAPON_DEFAULT;
int     key_right = KEYRIGHT_DEFAULT;
int     key_run = KEYRUN_DEFAULT;
int     key_screenshot = KEYSCREENSHOT_DEFAULT;
int     key_strafe = KEYSTRAFE_DEFAULT;
int     key_strafeleft = KEYSTRAFELEFT_DEFAULT;
int     key_strafeleft2 = KEYSTRAFELEFT2_DEFAULT;
int     key_straferight = KEYSTRAFERIGHT_DEFAULT;
int     key_straferight2 = KEYSTRAFERIGHT2_DEFAULT;
int     key_up = KEYUP_DEFAULT;
int     key_up2 = KEYUP2_DEFAULT;
int     key_use = KEYUSE_DEFAULT;
int     key_use2 = KEYUSE2_DEFAULT;
int     key_weapon1 = KEYWEAPON1_DEFAULT;
int     key_weapon2 = KEYWEAPON2_DEFAULT;
int     key_weapon3 = KEYWEAPON3_DEFAULT;
int     key_weapon4 = KEYWEAPON4_DEFAULT;
int     key_weapon5 = KEYWEAPON5_DEFAULT;
int     key_weapon6 = KEYWEAPON6_DEFAULT;
int     key_weapon7 = KEYWEAPON7_DEFAULT;

int     mousebfire = MOUSEFIRE_DEFAULT;
int     mousebforward = MOUSEFORWARD_DEFAULT;
int     mousebnextweapon = MOUSENEXTWEAPON_DEFAULT;
int     mousebprevweapon = MOUSEPREVWEAPON_DEFAULT;
int     mousebrun = MOUSERUN_DEFAULT;
int     mousebstrafe = MOUSESTRAFE_DEFAULT;
int     mousebuse = MOUSEUSE_DEFAULT;

int     gamepadalwaysrun = GAMEPADALWAYSRUN_DEFAULT;
int     gamepadautomap = GAMEPADAUTOMAP_DEFAULT;
int     gamepadautomapclearmark = GAMEPADAUTOMAPCLEARMARK_DEFAULT;
int     gamepadautomapfollowmode = GAMEPADAUTOMAPFOLLOWMODE_DEFAULT;
int     gamepadautomapgrid = GAMEPADAUTOMAPGRID_DEFAULT;
int     gamepadautomapmark = GAMEPADAUTOMAPMARK_DEFAULT;
int     gamepadautomapmaxzoom = GAMEPADAUTOMAPMAXZOOM_DEFAULT;
int     gamepadautomaprotatemode = GAMEPADAUTOMAPROTATEMODE_DEFAULT;
int     gamepadautomapzoomin = GAMEPADAUTOMAPZOOMIN_DEFAULT;
int     gamepadautomapzoomout = GAMEPADAUTOMAPZOOMOUT_DEFAULT;
int     gamepadfire = GAMEPADFIRE_DEFAULT;
int     gamepadmenu = GAMEPADMENU_DEFAULT;
int     gamepadnextweapon = GAMEPADNEXTWEAPON_DEFAULT;
int     gamepadprevweapon = GAMEPADPREVWEAPON_DEFAULT;
int     gamepadrun = GAMEPADRUN_DEFAULT;
int     gamepaduse = GAMEPADUSE_DEFAULT;
int     gamepaduse2 = GAMEPADUSE2_DEFAULT;
int     gamepadweapon1 = GAMEPADWEAPON_DEFAULT;
int     gamepadweapon2 = GAMEPADWEAPON_DEFAULT;
int     gamepadweapon3 = GAMEPADWEAPON_DEFAULT;
int     gamepadweapon4 = GAMEPADWEAPON_DEFAULT;
int     gamepadweapon5 = GAMEPADWEAPON_DEFAULT;
int     gamepadweapon6 = GAMEPADWEAPON_DEFAULT;
int     gamepadweapon7 = GAMEPADWEAPON_DEFAULT;
