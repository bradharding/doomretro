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

#ifndef __M_CONFIG__
#define __M_CONFIG__

#define ALWAYSRUN_DEFAULT                       false

#define ANIMATEDLIQUID_DEFAULT                  true

#define UNLIMITED                               32768
#define BLOODSPLATS_MIN                         0
#define BLOODSPLATS_DEFAULT                     UNLIMITED
#define BLOODSPLATS_MAX                         UNLIMITED

#define BRIGHTMAPS_DEFAULT                      true

#define CENTERWEAPON_DEFAULT                    true

#define CORPSES_MIRROR_DEFAULT                  true

#define CORPSES_MOREBLOOD_DEFAULT               true

#define CORPSES_SLIDE_DEFAULT                   true

#define CORPSES_SMEARBLOOD_DEFAULT              true

#define DCLICKUSE_DEFAULT                       false

#define EPISODE_MIN                             0
#define EPISODE_DEFAULT                         0
#define EPISODE_MAX                             3

#define EXPANSION_MIN                           0
#define EXPANSION_DEFAULT                       0
#define EXPANSION_MAX                           1

#define FLOATBOB_DEFAULT                        true

#define FOOTCLIP_DEFAULT                        true

#define FULLSCREEN_DEFAULT                      true

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

#define GAMEPADLEFTDEADZONE_MIN                 0.0f
#define GAMEPADLEFTDEADZONE_DEFAULT             GAMEPAD_LEFT_THUMB_DEADZONE / (float)SHRT_MAX * 100.0f
#define GAMEPADLEFTDEADZONE_MAX                 (float)SHRT_MAX

#define GAMEPADRIGHTDEADZONE_MIN                0.0f
#define GAMEPADRIGHTDEADZONE_DEFAULT            GAMEPAD_RIGHT_THUMB_DEADZONE / (float)SHRT_MAX * 100.0f
#define GAMEPADRIGHTDEADZONE_MAX                (float)SHRT_MAX

#define GAMEPADMENU_DEFAULT                     GAMEPAD_START

#define GAMEPADNEXTWEAPON_DEFAULT               GAMEPAD_B

#define GAMEPADPREVWEAPON_DEFAULT               GAMEPAD_Y

#define GAMEPADSENSITIVITY_MIN                  0
#define GAMEPADSENSITIVITY_DEFAULT              32
#define GAMEPADSENSITIVITY_MAX                  128

#define GAMEPADRUN_DEFAULT                      GAMEPAD_LEFT_TRIGGER

#define GAMEPADUSE_DEFAULT                      GAMEPAD_A

#define GAMEPADWEAPON_DEFAULT                   0

#define GAMEPADLEFTHANDED_DEFAULT               false

#define DAMAGE                                  1
#define WEAPONS                                 2
#define GAMEPADVIBRATE_MIN                      0
#define GAMEPADVIBRATE_DEFAULT                  (DAMAGE | WEAPONS)
#define GAMEPADVIBRATE_MAX                      (DAMAGE | WEAPONS)

#define GAMMALEVEL_MIN                          gammalevels[0]
#define GAMMALEVEL_DEFAULT                      0.75
#define GAMMALEVEL_MAX                          gammalevels[GAMMALEVELS - 1]

#define LOW                                     0
#define HIGH                                    1
#define GRAPHICDETAIL_DEFAULT                   HIGH

#define GRID_DEFAULT                            false

#define HOMINDICATOR_DEFAULT                    false

#define HUD_DEFAULT                             true

#define IWADFOLDER_DEFAULT                      "."

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

#define MAPFIXES_DEFAULT                        true

#define MESSAGES_DEFAULT                        false

#define MIRRORWEAPONS_DEFAULT                   false

#define MOUSEACCELERATION_DEFAULT               2.0

#define MOUSEFIRE_DEFAULT                       0

#define MOUSEFORWARD_DEFAULT                    -1

#define MOUSEPREVWEAPON_DEFAULT                 MOUSE_WHEELDOWN

#define MOUSENEXTWEAPON_DEFAULT                 MOUSE_WHEELUP

#define MOUSESENSITIVITY_MIN                    0
#define MOUSESENSITIVITY_DEFAULT                16
#define MOUSESENSITIVITY_MAX                    128

#define MOUSESTRAFE_DEFAULT                     -1

#define MOUSETHRESHOLD_DEFAULT                  10

#define MOUSEUSE_DEFAULT                        -1

#define MUSICVOLUME_MIN                         0
#define MUSICVOLUME_DEFAULT                     100
#define MUSICVOLUME_MAX                         100

#define NOVERT_DEFAULT                          true

#define PIXELWIDTH_MIN                          2
#define PIXELWIDTH_DEFAULT                      2
#define PIXELWIDTH_MAX                          SCREENWIDTH

#define PIXELHEIGHT_MIN                         2
#define PIXELHEIGHT_DEFAULT                     2
#define PIXELHEIGHT_MAX                         SCREENHEIGHT

#define PLAYERBOB_MIN                           0
#define PLAYERBOB_DEFAULT                       75
#define PLAYERBOB_MAX                           100

#define ROTATEMODE_DEFAULT                      true

#define RUNCOUNT_MAX                            32768

#define SAVEGAME_DEFAULT                        0

#define SCALEQUALITY_DEFAULT                    "nearest"

#define SCREENSIZE_MIN                          0
#define SCREENSIZE_DEFAULT                      7
#define SCREENSIZE_MAX                          8

#define SCREENWIDTH_DEFAULT                     0

#define SCREENHEIGHT_DEFAULT                    0

#define SFXVOLUME_MIN                           0
#define SFXVOLUME_DEFAULT                       100
#define SFXVOLUME_MAX                           100

#define SHADOWS_DEFAULT                         true

#define SKILLLEVEL_MIN                          sk_baby
#define SKILLLEVEL_DEFAULT                      sk_medium
#define SKILLLEVEL_MAX                          sk_nightmare

#define SMOKETRAILS_DEFAULT                     true

#define SND_MAXSLICETIME_MS_DEFAULT             28

#define TIMIDITY_CFG_PATH_DEFAULT               ""

#define TRANSLUCENCY_DEFAULT                    true

#ifdef WIN32
#define VIDEODRIVER_DEFAULT                     "windows"
#else
#define VIDEODRIVER_DEFAULT                     ""
#endif

#define VSYNC_DEFAULT                           true

#define WIDESCREEN_DEFAULT                      false

#define WINDOWPOSITION_DEFAULT                  ""

#define WINDOWWIDTH_DEFAULT                     SCREENWIDTH

#define WINDOWHEIGHT_DEFAULT                    (SCREENWIDTH * 3 / 4)

typedef enum
{
    DEFAULT_INT,
    DEFAULT_INT_HEX,
    DEFAULT_INT_PERCENT,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
    DEFAULT_FLOAT_PERCENT,
    DEFAULT_KEY
} default_type_t;

typedef struct
{
    // Name of the variable
    char                *name;

    // Pointer to the location in memory of the variable
    void                *location;

    // Type of the variable
    default_type_t      type;

    // If this is a key value, the original integer scancode we read from
    // the config file before translating it to the internal key value.
    // If zero, we didn't read this value from a config file.
    int                 untranslated;

    // The value we translated the scancode into when we read the
    // config file on startup.  If the variable value is different from
    // this, it has been changed and needs to be converted; otherwise,
    // use the 'untranslated' value.
    int                 original_translated;

    int                 set;
} default_t;

typedef struct
{
    default_t           *defaults;
    int                 numdefaults;
    char                *filename;
} default_collection_t;

extern default_collection_t doom_defaults;

void M_LoadDefaults(void);
void M_SaveDefaults(void);

#endif
