/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#if !defined(__M_CONFIG__)
#define __M_CONFIG__

#define AM_GRID_DEFAULT                         false

#define AM_ROTATEMODE_DEFAULT                   true

#define EPISODE_MIN                             0
#define EPISODE_DEFAULT                         0
#define EPISODE_MAX                             3

#define EXPANSION_MIN                           0
#define EXPANSION_DEFAULT                       0
#define EXPANSION_MAX                           1

#define GAMEPADAUTOMAP_DEFAULT                  GAMEPAD_BACK

#define GAMEPADAUTOMAPCLEARMARK_DEFAULT         0

#define GAMEPADAUTOMAPFOLLOWMODE_DEFAULT        0

#define GAMEPADAUTOMAPGRID_DEFAULT              0

#define GAMEPADAUTOMAPMARK_DEFAULT              0

#define GAMEPADAUTOMAPMAXZOOM_DEFAULT           0

#define GAMEPADAUTOMAPROTATEMODE_DEFAULT        0

#define GAMEPADAUTOMAPZOOMIN_DEFAULT            GAMEPAD_RIGHT_SHOULDER

#define GAMEPADAUTOMAPZOOMOUT_DEFAULT           GAMEPAD_LEFT_SHOULDER

#define GAMEPADFIRE_DEFAULT                     GAMEPAD_RIGHT_TRIGGER

#define GAMEPADMENU_DEFAULT                     GAMEPAD_START

#define GAMEPADNEXTWEAPON_DEFAULT               GAMEPAD_B

#define GAMEPADPREVWEAPON_DEFAULT               GAMEPAD_Y

#define GAMEPADRUN_DEFAULT                      GAMEPAD_LEFT_TRIGGER

#define GAMEPADUSE_DEFAULT                      GAMEPAD_A

#define GAMEPADWEAPON_DEFAULT                   0

#define GP_DEADZONE_LEFT_MIN                    0.0f
#define GP_DEADZONE_LEFT_DEFAULT                (GAMEPAD_LEFT_THUMB_DEADZONE / (float)SHRT_MAX * 100.0f)
#define GP_DEADZONE_LEFT_MAX                    (float)SHRT_MAX

#define GP_DEADZONE_RIGHT_MIN                   0.0f
#define GP_DEADZONE_RIGHT_DEFAULT               (GAMEPAD_LEFT_THUMB_DEADZONE / (float)SHRT_MAX * 100.0f)
#define GP_DEADZONE_RIGHT_MAX                   (float)SHRT_MAX

#define GP_SENSITIVITY_MIN                      0
#define GP_SENSITIVITY_DEFAULT                  32
#define GP_SENSITIVITY_MAX                      128

#define GP_SWAPTHUMBSTICKS_DEFAULT              false

#define GP_VIBRATE_DEFAULT                      true

#define IWADFOLDER_DEFAULT                      "C:\\"

#define KEYALWAYSRUN_DEFAULT                    KEY_CAPSLOCK

#define KEYAUTOMAP_DEFAULT                      KEY_TAB

#define KEYAUTOMAPCLEARMARK_DEFAULT             'c'

#define KEYAUTOMAPFOLLOWMODE_DEFAULT            'f'

#define KEYAUTOMAPGRID_DEFAULT                  'g'

#define KEYAUTOMAPMARK_DEFAULT                  'm'

#define KEYAUTOMAPMAXZOOM_DEFAULT               '0'

#define KEYAUTOMAPROTATEMODE_DEFAULT            'r'

#define KEYAUTOMAPZOOMIN_DEFAULT                KEY_EQUALS

#define KEYAUTOMAPZOOMOUT_DEFAULT               KEY_MINUS

#define KEYDOWN_DEFAULT                         KEY_DOWNARROW

#define KEYDOWN2_DEFAULT                        's'

#define KEYFIRE_DEFAULT                         KEY_RCTRL

#define KEYLEFT_DEFAULT                         KEY_LEFTARROW

#define KEYNEXTWEAPON_DEFAULT                   0

#define KEYPREVWEAPON_DEFAULT                   0

#define KEYRIGHT_DEFAULT                        KEY_RIGHTARROW

#define KEYRUN_DEFAULT                          KEY_RSHIFT

#define KEYSTRAFE_DEFAULT                       KEY_RALT

#define KEYSTRAFELEFT_DEFAULT                   'a'

#define KEYSTRAFELEFT2_DEFAULT                  ','

#define KEYSTRAFERIGHT_DEFAULT                  'd'

#define KEYSTRAFERIGHT2_DEFAULT                  '.'

#define KEYUP_DEFAULT                           KEY_UPARROW

#define KEYUP2_DEFAULT                          'w'

#define KEYUSE_DEFAULT                          ' '

#define KEYWEAPON1_DEFAULT                      '1'

#define KEYWEAPON2_DEFAULT                      '2'

#define KEYWEAPON3_DEFAULT                      '3'

#define KEYWEAPON4_DEFAULT                      '4'

#define KEYWEAPON5_DEFAULT                      '5'

#define KEYWEAPON6_DEFAULT                      '6'

#define KEYWEAPON7_DEFAULT                      '7'

#define MESSAGES_DEFAULT                        false

#define M_ACCELERATION_MIN                      0
#define M_ACCELERATION_DEFAULT                  2.0
#define M_ACCELERATION_MAX                      INT_MAX

#define M_DOUBLECLICK_USE_DEFAULT               false

#define M_NOVERTICAL_DEFAULT                    true

#define M_SENSITIVITY_MIN                       0
#define M_SENSITIVITY_DEFAULT                   16
#define M_SENSITIVITY_MAX                       128

#define M_THRESHOLD_MIN                         0
#define M_THRESHOLD_DEFAULT                     10
#define M_THRESHOLD_MAX                         INT_MAX

#define MOUSEFIRE_DEFAULT                       0

#define MOUSEFORWARD_DEFAULT                    -1

#define MOUSEPREVWEAPON_DEFAULT                 MOUSE_WHEELUP

#define MOUSENEXTWEAPON_DEFAULT                 MOUSE_WHEELDOWN

#define MOUSESTRAFE_DEFAULT                     -1

#define MOUSEUSE_DEFAULT                        -1

#define PLAYERNAME_DEFAULT                      "you"

#define PM_ALWAYSRUN_DEFAULT                    false

#define PM_CENTERWEAPON_DEFAULT                 true

#define PM_WALKBOB_MIN                          0
#define PM_WALKBOB_DEFAULT                      75
#define PM_WALKBOB_MAX                          100

#define NOBLOOD                                 0
#define REDBLOODONLY                            1
#define ALLBLOODCOLORS                          2
#define R_BLOOD_DEFAULT                         ALLBLOODCOLORS

#define R_BRIGHTMAPS_DEFAULT                    true

#define R_CORPSES_MIRRORED_DEFAULT              true

#define R_CORPSES_MOREBLOOD_DEFAULT             true

#define R_CORPSES_NUDGE_DEFAULT                 true

#define R_CORPSES_SLIDE_DEFAULT                 true

#define R_CORPSES_SMEARBLOOD_DEFAULT            true

#define LOW                                     0
#define HIGH                                    1
#define R_DETAIL_DEFAULT                        HIGH

#define R_FIXMAPERRORS_DEFAULT                  true

#define R_FIXSPRITEOFFSETS_DEFAULT              true

#define R_FLOATBOB_DEFAULT                      true

#define R_GAMMA_MIN                             gammalevels[0]
#define R_GAMMA_DEFAULT                         0.75
#define R_GAMMA_MAX                             gammalevels[GAMMALEVELS - 1]

#define R_HOMINDICATOR_DEFAULT                  false

#define R_HUD_DEFAULT                           true

#define R_LIQUID_BOB_DEFAULT                    true

#define R_LIQUID_RIPPLE_DEFAULT                 true

#define R_LIQUID_CLIPSPRITES_DEFAULT            true

#define R_LOWPIXELSIZE_DEFAULT                  "2x2"

#define UNLIMITED                               32768
#define R_MAXBLOODSPLATS_MIN                    0
#define R_MAXBLOODSPLATS_DEFAULT                UNLIMITED
#define R_MAXBLOODSPLATS_MAX                    UNLIMITED

#define R_MIRRORWEAPONS_DEFAULT                 false

#define R_PLAYERSPRITES_DEFAULT                 true

#define R_ROCKETTRAILS_DEFAULT                  true

#define R_SCREENSIZE_MIN                        0
#define R_SCREENSIZE_DEFAULT                    7
#define R_SCREENSIZE_MAX                        8

#define R_SHADOWS_DEFAULT                       true

#define R_TRANSLUCENCY_DEFAULT                  true

#define RUNCOUNT_MAX                            32768

#define S_MUSICVOLUME_MIN                       0
#define S_MUSICVOLUME_DEFAULT                   100
#define S_MUSICVOLUME_MAX                       100

#define S_RANDOMPITCH_DEFAULT                   false

#define S_SFXVOLUME_MIN                         0
#define S_SFXVOLUME_DEFAULT                     100
#define S_SFXVOLUME_MAX                         100

#define S_TIMIDITYCFGPATH_DEFAULT               ""

#define SAVEGAME_DEFAULT                        0

#define SKILLLEVEL_MIN                          sk_baby
#define SKILLLEVEL_DEFAULT                      sk_medium
#define SKILLLEVEL_MAX                          sk_nightmare

#define VID_CAPFPS_DEFAULT                      false

#define VID_DISPLAY_MIN                         1
#define VID_DISPLAY_DEFAULT                     1
#define VID_DISPLAY_MAX                         INT_MAX

#if !defined(WIN32)
#define VID_DRIVER_DEFAULT                      ""
#endif

#define VID_FULLSCREEN_DEFAULT                  true

#define VID_SCALEDRIVER_DEFAULT                 ""

#define VID_SCALEFILTER_DEFAULT                 "nearest"

#define VID_SCREENRESOLUTION_DEFAULT            "desktop"

#define VID_VSYNC_DEFAULT                       false

#define VID_WIDESCREEN_DEFAULT                  false

#define VID_WINDOWPOSITION_DEFAULT              ""

#define VID_WINDOWSIZE_DEFAULT                  "640x480"

typedef enum
{
    DEFAULT_INT,
    DEFAULT_INT_PERCENT,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
    DEFAULT_FLOAT_PERCENT
} default_type_t;

typedef struct
{
    // Name of the variable
    char                *name;

    // Pointer to the location in memory of the variable
    void                *location;

    // Type of the variable
    default_type_t      type;

    int                 set;
} default_t;

#define NOALIAS         0
#define BOOLALIAS       1
#define SCREENALIAS     2
#define DETAILALIAS     3
#define SPLATALIAS      4
#define GAMMAALIAS      5
#define BLOODALIAS      6

typedef struct
{
    char                *text;
    int                 value;
    int                 set;
} alias_t;

extern alias_t          aliases[];

void M_LoadCVARs(char *filename);
void M_SaveCVARs(void);
char *striptrailingzero(float value, int precision);

#endif
