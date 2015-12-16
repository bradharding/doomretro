/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

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

#if !defined(__M_CONFIG__)
#define __M_CONFIG__

typedef enum
{
    am_cheat_off,
    am_cheat_all,
    am_cheat_things
} am_cheat_values_e;

typedef enum
{
    r_blood_none,
    r_blood_red,
    r_blood_all
} r_blood_values_e;

typedef enum
{
    r_detail_low,
    r_detail_high
} r_detail_values_e;

#define alwaysrun_default                       false

#define am_allmapcdwallcolor_min                0
#define am_allmapcdwallcolor_default            106
#define am_allmapcdwallcolor_max                255

#define am_allmapfdwallcolor_min                0
#define am_allmapfdwallcolor_default            110
#define am_allmapfdwallcolor_max                255

#define am_allmapwallcolor_min                  0
#define am_allmapwallcolor_default              108
#define am_allmapwallcolor_max                  255

#define am_backcolor_min                        0
#define am_backcolor_default                    0
#define am_backcolor_max                        255

#define am_cdwallcolor_min                      0
#define am_cdwallcolor_default                  160
#define am_cdwallcolor_max                      255

#define am_cheat_min                            am_cheat_off
#define am_cheat_default                        am_cheat_off
#define am_cheat_max                            am_cheat_things

#define am_external_default                     false

#define am_fdwallcolor_min                      0
#define am_fdwallcolor_default                  64
#define am_fdwallcolor_max                      255

#define am_followmode_default                   true

#define am_grid_default                         false

#define am_gridcolor_min                        0
#define am_gridcolor_default                    5
#define am_gridcolor_max                        255

#define am_markcolor_min                        0
#define am_markcolor_default                    100
#define am_markcolor_max                        255

#define am_playercolor_min                      0
#define am_playercolor_default                  4
#define am_playercolor_max                      255

#define am_rotatemode_default                   true

#define am_teleportercolor_min                  0
#define am_teleportercolor_default              184
#define am_teleportercolor_max                  255

#define am_thingcolor_min                       0
#define am_thingcolor_default                   112
#define am_thingcolor_max                       255

#define am_tswallcolor_min                      0
#define am_tswallcolor_default                  104
#define am_tswallcolor_max                      255

#define am_wallcolor_min                        0
#define am_wallcolor_default                    176
#define am_wallcolor_max                        255

#define am_xhaircolor_min                       0
#define am_xhaircolor_default                   4
#define am_xhaircolor_max                       255

#define centerweapon_default                    true

#define con_obituaries_default                  false

#define con_timestamps_default                  true

#define episode_min                             0
#define episode_default                         0
#define episode_max                             3

#define faceback_min                            0
#define faceback_default                        5
#define faceback_max                            255

#define expansion_min                           0
#define expansion_default                       0
#define expansion_max                           1

#define gp_deadzone_left_min                    0.0f
#define gp_deadzone_left_default                (GAMEPAD_LEFT_THUMB_DEADZONE \
                                                / (float)SHRT_MAX * 100.0f)
#define gp_deadzone_left_max                    100.0f

#define gp_deadzone_right_min                   0.0f
#define gp_deadzone_right_default               (GAMEPAD_RIGHT_THUMB_DEADZONE \
                                                / (float)SHRT_MAX * 100.0f)
#define gp_deadzone_right_max                   100.0f

#define gp_sensitivity_min                      0
#define gp_sensitivity_default                  32
#define gp_sensitivity_max                      128

#define gp_swapthumbsticks_default              false

#define gp_vibrate_default                      true

#define iwadfolder_default                      "C:\\"

#define messages_default                        false

#define m_acceleration_min                      0
#define m_acceleration_default                  2.0
#define m_acceleration_max                      INT_MAX

#define m_doubleclick_use_default               false

#define m_novertical_default                    true

#define m_sensitivity_min                       0
#define m_sensitivity_default                   24
#define m_sensitivity_max                       128

#define m_threshold_min                         0
#define m_threshold_default                     10
#define m_threshold_max                         INT_MAX

#define movebob_min                             0
#define movebob_default                         75
#define movebob_max                             100

#define playername_default                      "you"

#define r_althud_default                        false

#define r_blood_min                             r_blood_none
#define r_blood_default                         r_blood_all
#define r_blood_max                             r_blood_all

#define r_bloodsplats_max_min                   0
#define r_bloodsplats_max_default               32768
#define r_bloodsplats_max_max                   1048576

#define r_bloodsplats_total_min                 0
#define r_bloodsplats_total_default             0
#define r_bloodsplats_total_max                 0

#define r_brightmaps_default                    true

#define r_corpses_color_default                 true

#define r_corpses_mirrored_default              true

#define r_corpses_moreblood_default             true

#define r_corpses_nudge_default                 true

#define r_corpses_slide_default                 true

#define r_corpses_smearblood_default            true

#define r_detail_default                        r_detail_high

#define r_diskicon_default                      true

#define r_fixmaperrors_default                  true

#define r_fixspriteoffsets_default              true

#define r_floatbob_default                      true

#define r_gamma_min                             gammalevels[0]
#define r_gamma_default                         0.75
#define r_gamma_max                             gammalevels[GAMMALEVELS - 1]

#define r_homindicator_default                  false

#define r_hud_default                           true

#define r_liquid_bob_default                    true

#define r_liquid_swirl_default                  true

#define r_liquid_clipsprites_default            true

#define r_liquid_current_default                true

#define r_liquid_lowerview_default              true

#define r_lowpixelsize_default                  "2x2"

#define r_mirroredweapons_default               false

#define r_playersprites_default                 true

#define r_rockettrails_default                  true

#define r_screensize_min                        0
#define r_screensize_default                    7
#define r_screensize_max                        8

#define r_shadows_default                       true

#define r_shakescreen_default                   true

#define r_translucency_default                  true

#define s_musicvolume_min                       0
#define s_musicvolume_default                   100
#define s_musicvolume_max                       100

#define s_randommusic_default                   false

#define s_randompitch_default                   false

#define s_sfxvolume_min                         0
#define s_sfxvolume_default                     100
#define s_sfxvolume_max                         100

#define s_timiditycfgpath_default               ""

#define savegame_min                            0
#define savegame_default                        0
#define savegame_max                            5

#define skilllevel_min                          sk_baby
#define skilllevel_default                      sk_medium
#define skilllevel_max                          sk_nightmare

#define stillbob_min                            0
#define stillbob_default                        0
#define stillbob_max                            100

#define vid_capfps_default                      false

#define vid_display_min                         1
#define vid_display_default                     1
#define vid_display_max                         INT_MAX

#if !defined(win32)
#define vid_driver_default                      ""
#endif

#define vid_fullscreen_default                  true

#define vid_scaledriver_direct3d                "direct3d"
#define vid_scaledriver_opengl                  "opengl"
#define vid_scaledriver_software                "software"
#define vid_scaledriver_default                 ""

#define vid_scalefilter_linear                  "linear"
#define vid_scalefilter_nearest                 "nearest"
#define vid_scalefilter_nearest_linear          "nearest_linear"
#define vid_scalefilter_default                 vid_scalefilter_nearest

#define vid_screenresolution_desktop            "desktop"
#define vid_screenresolution_default            vid_screenresolution_desktop

#define vid_showfps_default                     false

#define vid_vsync_default                       false

#define vid_widescreen_default                  false

#define vid_windowposition_centered             "centered"
#define vid_windowposition_default              vid_windowposition_centered

#define vid_windowsize_default                  "768x480"

#define GAMEPADALWAYSRUN_DEFAULT                0
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

#define MOUSEFIRE_DEFAULT                       0
#define MOUSEFORWARD_DEFAULT                    -1
#define MOUSEPREVWEAPON_DEFAULT                 MOUSE_WHEELUP
#define MOUSENEXTWEAPON_DEFAULT                 MOUSE_WHEELDOWN
#define MOUSESTRAFE_DEFAULT                     -1
#define MOUSEUSE_DEFAULT                        -1

typedef enum
{
    DEFAULT_INT,
    DEFAULT_INT_UNSIGNED,
    DEFAULT_INT_PERCENT,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
    DEFAULT_FLOAT_PERCENT,
    DEFAULT_OTHER
} default_type_t;

typedef enum
{
    NOALIAS,
    BOOLALIAS,
    DETAILALIAS,
    GAMMAALIAS,
    BLOODALIAS,
    CHEATALIAS
} alias_type_t;

typedef struct
{
    // Name of the variable
    char                *name;

    // Pointer to the location in memory of the variable
    void                *location;

    // Type of the variable
    default_type_t      type;

    alias_type_t        aliastype;
} default_t;

typedef struct
{
    char                *text;
    int                 value;
    alias_type_t        type;
} alias_t;

extern alias_t          aliases[];

void M_LoadCVARs(char *filename);
void M_SaveCVARs(void);
char *striptrailingzero(float value, int precision);

#endif