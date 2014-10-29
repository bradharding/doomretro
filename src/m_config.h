/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __M_CONFIG__
#define __M_CONFIG__

#define ALWAYSRUN_DEFAULT               false

#define BLOODSPLATS_MIN                 0
#define BLOODSPLATS_DEFAULT             UNLIMITED
#define BLOODSPLATS_MAX                 UNLIMITED
#define UNLIMITED                       32768

#define BRIGHTMAPS_DEFAULT              true

#define CENTEREDWEAPON_DEFAULT          true

#define CORPSES_MIN                     0
#define CORPSES_DEFAULT                 (MIRROR | SLIDE | SMEARBLOOD | MOREBLOOD)
#define CORPSES_MAX                     CORPSES_DEFAULT
#define MIRROR                          1
#define SLIDE                           2
#define SMEARBLOOD                      4
#define MOREBLOOD                       8

#define DCLICKUSE_DEFAULT               false

#define EPISODE_MIN                     0
#define EPISODE_DEFAULT                 0
#define EPISODE_MAX                     3

#define EXPANSION_MIN                   0
#define EXPANSION_DEFAULT               0
#define EXPANSION_MAX                   1

#define FOOTCLIP_DEFAULT                true

#define FULLSCREEN_DEFAULT              true

#define GAMEPADAUTOMAP_DEFAULT          GAMEPAD_BACK

#define GAMEPADFIRE_DEFAULT             GAMEPAD_RIGHT_TRIGGER

#define GAMEPADMENU_DEFAULT             GAMEPAD_START

#define GAMEPADNEXTWEAPON_DEFAULT       GAMEPAD_B

#define GAMEPADPREVWEAPON_DEFAULT       GAMEPAD_Y

#define GAMEPADSENSITIVITY_MIN          0
#define GAMEPADSENSITIVITY_DEFAULT      16
#define GAMEPADSENSITIVITY_MAX          128

#define GAMEPADSPEED_DEFAULT            GAMEPAD_LEFT_TRIGGER

#define GAMEPADUSE_DEFAULT              GAMEPAD_A

#define GAMEPADWEAPON_DEFAULT           0

#define GAMEPADLEFTHANDED_DEFAULT       false

#define GAMEPADVIBRATE_DEFAULT          true

#define GAMMA_MIN                       gammalevels[0]
#define GAMMA_DEFAULT                   0.75
#define GAMMA_MAX                       gammalevels[GAMMALEVELS - 1]

#define GRAPHICDETAIL_DEFAULT           HIGH
#define LOW                             0
#define HIGH                            1

#define GRID_DEFAULT                    false

#define HOMINDICATOR_DEFAULT            false

#define HUD_DEFAULT                     true

#define IWADFOLDER_DEFAULT              "."

#define KEYDOWN_DEFAULT                 KEY_DOWNARROW

#define KEYDOWN2_DEFAULT                's'

#define KEYFIRE_DEFAULT                 KEY_RCTRL

#define KEYLEFT_DEFAULT                 KEY_LEFTARROW

#define KEYNEXTWEAPON_DEFAULT           0

#define KEYPREVWEAPON_DEFAULT           0

#define KEYRIGHT_DEFAULT                KEY_RIGHTARROW

#define KEYSPEED_DEFAULT                KEY_RSHIFT

#define KEYSTRAFE_DEFAULT               KEY_RALT

#define KEYSTRAFELEFT_DEFAULT           'a'

#define KEYSTRAFERIGHT_DEFAULT          'd'

#define KEYUP_DEFAULT                   KEY_UPARROW

#define KEYUP2_DEFAULT                  'w'

#define KEYUSE_DEFAULT                  ' '

#define MESSAGES_DEFAULT                false

#define MIRRORWEAPONS_DEFAULT           false

#define MOUSEACCELERATION_DEFAULT       2.0

#define MOUSEFIRE_DEFAULT               0

#define MOUSEFORWARD_DEFAULT            -1

#define MOUSEPREVWEAPON_DEFAULT         3

#define MOUSENEXTWEAPON_DEFAULT         4

#define MOUSESENSITIVITY_MIN            0
#define MOUSESENSITIVITY_DEFAULT        16
#define MOUSESENSITIVITY_MAX            128

#define MOUSESTRAFE_DEFAULT             -1

#define MOUSETHRESHOLD_DEFAULT          10

#define MOUSEUSE_DEFAULT                -1

#define MUSICVOLUME_MIN                 0
#define MUSICVOLUME_DEFAULT             15
#define MUSICVOLUME_MAX                 15

#define NOVERT_DEFAULT                  true

#define PIXELWIDTH_MIN                  2
#define PIXELWIDTH_DEFAULT              2
#define PIXELWIDTH_MAX                  SCREENWIDTH

#define PIXELHEIGHT_MIN                 2
#define PIXELHEIGHT_DEFAULT             2
#define PIXELHEIGHT_MAX                 SCREENHEIGHT

#define PLAYERBOB_MIN                   0
#define PLAYERBOB_DEFAULT               75
#define PLAYERBOB_MAX                   100

#define ROTATE_DEFAULT                  true

#define RUNCOUNT_MAX                    32768

#define SATURATION_MIN                  0.0
#define SATURATION_DEFAULT              0.75
#define SATURATION_MAX                  1.0

#define SAVEGAME_DEFAULT                0

#define SCREENSIZE_MIN                  0
#define SCREENSIZE_DEFAULT              7
#define SCREENSIZE_MAX                  8

#define SCREENWIDTH_DEFAULT             0

#define SCREENHEIGHT_DEFAULT            0

#define SFXVOLUME_MIN                   0
#define SFXVOLUME_DEFAULT               15
#define SFXVOLUME_MAX                   15

#define SHADOWS_DEFAULT                 true

#define SKILLLEVEL_MIN                  sk_baby
#define SKILLLEVEL_DEFAULT              sk_medium
#define SKILLLEVEL_MAX                  sk_nightmare

#define SMOKETRAILS_DEFAULT             true

#define SND_MAXSLICETIME_MS_DEFAULT     120

#define TIMIDITY_CFG_PATH_DEFAULT       ""

#define TRANSLUCENCY_DEFAULT            true

#ifdef WIN32
#ifdef SDL20
#define VIDEODRIVER_DEFAULT             "windows"
#else
#define VIDEODRIVER_DEFAULT             "directx"
#endif
#else
#define VIDEODRIVER_DEFAULT             ""
#endif

#define WIDESCREEN_DEFAULT              false

#define WINDOWPOSITION_DEFAULT          ""

#define WINDOWWIDTH_DEFAULT             SCREENWIDTH

#define WINDOWHEIGHT_DEFAULT            (SCREENWIDTH * 3 / 4)

void M_LoadDefaults(void);
void M_SaveDefaults(void);

#endif
