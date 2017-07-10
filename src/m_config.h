/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__M_CONFIG_H__)
#define __M_CONFIG_H__

typedef enum r_blood_values_e
{
    r_blood_none,
    r_blood_red,
    r_blood_all
} r_blood_values_t;

typedef enum r_messagescale_values_e
{
    r_messagescale_small,
    r_messagescale_big
} r_messagescale_values_t;

typedef enum r_detail_values_e
{
    r_detail_low,
    r_detail_high
} r_detail_values_t;

typedef enum units_values_e
{
    units_imperial,
    units_metric
} units_values_t;

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

#define am_crosshaircolor_min                   0
#define am_crosshaircolor_default               4
#define am_crosshaircolor_max                   255

#define am_external_default                     false

#define am_fdwallcolor_min                      0
#define am_fdwallcolor_default                  64
#define am_fdwallcolor_max                      255

#define am_followmode_default                   true

#define am_grid_default                         false

#define am_gridsize_default                     "128x128"

#define am_gridcolor_min                        0
#define am_gridcolor_default                    7
#define am_gridcolor_max                        255

#define am_markcolor_min                        0
#define am_markcolor_default                    95
#define am_markcolor_max                        255

#define am_path_default                         false

#define am_pathcolor_min                        0
#define am_pathcolor_default                    95
#define am_pathcolor_max                        255

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

#define ammo_min                                0
#define ammo_default                            50
#define ammo_max                                INT_MAX

#define armor_min                               0
#define armor_default                           0
#define armor_max                               INT_MAX

#define autoload_default                        true

#define centerweapon_default                    true

#define con_obituaries_default                  true

#define con_timestamps_default                  true

#define episode_min                             1
#define episode_default                         1
#define episode_max                             4

#define expansion_min                           1
#define expansion_default                       1
#define expansion_max                           2

#define facebackcolor_min                       0
#define facebackcolor_default                   5
#define facebackcolor_max                       255

#define gp_deadzone_left_min                    0.0f
#define gp_deadzone_left_default                24.0f
#define gp_deadzone_left_max                    100.0f

#define gp_deadzone_right_min                   0.0f
#define gp_deadzone_right_default               26.5f
#define gp_deadzone_right_max                   100.0f

#define gp_invertyaxis_default                  false

#define gp_sensitivity_min                      0
#define gp_sensitivity_default                  64
#define gp_sensitivity_max                      128

#define gp_swapthumbsticks_default              false

#define gp_vibrate_damage_min                   0
#define gp_vibrate_damage_default               100
#define gp_vibrate_damage_max                   200

#define gp_vibrate_weapons_min                  0
#define gp_vibrate_weapons_default              100
#define gp_vibrate_weapons_max                  200

#define health_min                              -200
#define health_default                          100
#define health_max                              INT_MAX

#define iwadfolder_default                      "C:\\"

#define m_acceleration_min                      0
#define m_acceleration_default                  2.0
#define m_acceleration_max                      INT_MAX

#define m_doubleclick_use_default               false

#define m_invertyaxis_default                   false

#define m_novertical_default                    true

#define m_sensitivity_min                       0
#define m_sensitivity_default                   32
#define m_sensitivity_max                       128

#define m_threshold_min                         0
#define m_threshold_default                     10
#define m_threshold_max                         INT_MAX

#define messages_default                        false

#define mouselook_default                       false

#define movebob_min                             0
#define movebob_default                         75
#define movebob_max                             100

#define playername_default                      "you"

#define r_althud_default                        true

#define r_berserkintensity_min                  0
#define r_berserkintensity_default              2
#define r_berserkintensity_max                  8

#define r_blood_min                             r_blood_none
#define r_blood_default                         r_blood_all
#define r_blood_max                             r_blood_all

#define r_bloodsplats_max_min                   0
#define r_bloodsplats_max_default               65536
#define r_bloodsplats_max_max                   1048576

#define r_bloodsplats_total_min                 0
#define r_bloodsplats_total_default             0
#define r_bloodsplats_total_max                 0

#define r_bloodsplats_translucency_default      true

#define r_brightmaps_default                    true

#define r_corpses_color_default                 true

#define r_corpses_mirrored_default              true

#define r_corpses_moreblood_default             true

#define r_corpses_nudge_default                 true

#define r_corpses_slide_default                 true

#define r_corpses_smearblood_default            true

#define r_detail_default                        r_detail_high

#define r_diskicon_default                      false

#define r_dither_default                        false

#define r_fixmaperrors_default                  true

#define r_fixspriteoffsets_default              true

#define r_floatbob_default                      true

#define r_gamma_min                             gammalevels[0]
#define r_gamma_default                         0.75f
#define r_gamma_max                             gammalevels[GAMMALEVELS - 1]

#define r_homindicator_default                  false

#define r_hud_default                           true

#define r_hud_translucency_default              true

#define r_liquid_bob_default                    true

#define r_liquid_clipsprites_default            true

#define r_liquid_current_default                true

#define r_liquid_lowerview_default              true

#define r_liquid_swirl_default                  true

#define r_lowpixelsize_default                  "2x2"

#define r_messagescale_default                  r_messagescale_big

#define r_mirroredweapons_default               false

#define r_playersprites_default                 true

#define r_rockettrails_default                  true

#define r_screensize_min                        0
#define r_screensize_default                    7
#define r_screensize_max                        7

#define r_shadows_default                       true

#define r_shadows_translucency_default          true

#define r_shake_barrels_default                 true

#define r_shake_damage_min                      0
#define r_shake_damage_default                  50
#define r_shake_damage_max                      100

#define r_skycolor_none                         -1
#define r_skycolor_min                          0
#define r_skycolor_default                      r_skycolor_none
#define r_skycolor_max                          255

#define r_textures_default                      true

#define r_translucency_default                  true

#define s_channels_min                          8
#define s_channels_default                      32
#define s_channels_max                          256

#define s_musicvolume_min                       0
#define s_musicvolume_default                   67
#define s_musicvolume_max                       100

#define s_randommusic_default                   false

#define s_randompitch_default                   false

#define s_sfxvolume_min                         0
#define s_sfxvolume_default                     100
#define s_sfxvolume_max                         100

#define savegame_min                            1
#define savegame_default                        1
#define savegame_max                            6

#define skilllevel_min                          1
#define skilllevel_default                      3
#define skilllevel_max                          5

#define stillbob_min                            0
#define stillbob_default                        0
#define stillbob_max                            100

#define turbo_min                               10
#define turbo_default                           100
#define turbo_max                               400

#define units_default                           units_imperial

#define version_default                         PACKAGE_VERSIONSTRING

#define vid_capfps_min                          35
#define vid_capfps_default                      200
#define vid_capfps_max                          1000

#define vid_display_min                         1
#define vid_display_default                     1
#define vid_display_max                         INT_MAX

#if !defined(_WIN32)
#define vid_driver_default                      ""
#endif

#define vid_fullscreen_default                  true

#define vid_motionblur_min                      0
#define vid_motionblur_default                  0
#define vid_motionblur_max                      100

#define vid_pillarboxes_default                 false

#define vid_scaleapi_direct3d                   "direct3d"
#define vid_scaleapi_opengl                     "opengl"
#if !defined(_WIN32)
#define vid_scaleapi_opengles                   "opengles"
#define vid_scaleapi_opengles2                  "opengles2"
#endif
#define vid_scaleapi_software                   "software"
#define vid_scaleapi_default                    vid_scaleapi_opengl

#define vid_scalefilter_linear                  "linear"
#define vid_scalefilter_nearest                 "nearest"
#define vid_scalefilter_nearest_linear          "nearest_linear"
#define vid_scalefilter_default                 vid_scalefilter_nearest

#define vid_screenresolution_desktop            "desktop"
#define vid_screenresolution_default            vid_screenresolution_desktop

#define vid_showfps_default                     false

#define vid_vsync_default                       true

#define vid_widescreen_default                  false

#define vid_windowposition_centered             "centered"
#define vid_windowposition_centred              "centred"
#define vid_windowposition_default              vid_windowposition_centered

#define vid_windowsize_default                  "768x480"

#if defined(_WIN32) || defined(__MACOSX__)
#define wad_default                             ""
#endif

#define weaponbob_min                           0
#define weaponbob_default                       75
#define weaponbob_max                           100

#define weaponrecoil_default                    false

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
#define GAMEPADBACK_DEFAULT                     0
#define GAMEPADFIRE_DEFAULT                     GAMEPAD_RIGHT_TRIGGER
#define GAMEPADFORWARD_DEFAULT                  0
#define GAMEPADLEFT_DEFAULT                     0
#define GAMEPADMENU_DEFAULT                     GAMEPAD_START
#define GAMEPADMOUSELOOK_DEFAULT                0
#define GAMEPADNEXTWEAPON_DEFAULT               GAMEPAD_B
#define GAMEPADPREVWEAPON_DEFAULT               GAMEPAD_Y
#define GAMEPADRIGHT_DEFAULT                    0
#define GAMEPADRUN_DEFAULT                      GAMEPAD_LEFT_TRIGGER
#define GAMEPADSTRAFE_DEFAULT                   0
#define GAMEPADSTRAFELEFT_DEFAULT               0
#define GAMEPADSTRAFERIGHT_DEFAULT              0
#define GAMEPADUSE_DEFAULT                      GAMEPAD_A
#define GAMEPADUSE2_DEFAULT                     GAMEPAD_RIGHT_THUMB
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
#define KEYCONSOLE_DEFAULT                      '`'
#define KEYDOWN_DEFAULT                         KEY_DOWNARROW
#define KEYDOWN2_DEFAULT                        's'
#define KEYFIRE_DEFAULT                         KEY_CTRL
#define KEYLEFT_DEFAULT                         KEY_LEFTARROW
#define KEYMOUSELOOK_DEFAULT                    0
#define KEYNEXTWEAPON_DEFAULT                   0
#define KEYPREVWEAPON_DEFAULT                   0
#define KEYRIGHT_DEFAULT                        KEY_RIGHTARROW
#define KEYRUN_DEFAULT                          KEY_SHIFT
#if defined(_WIN32)
#define KEYSCREENSHOT_DEFAULT                   KEY_PRINTSCREEN
#else
#define KEYSCREENSHOT_DEFAULT                   0
#endif
#define KEYSTRAFE_DEFAULT                       KEY_ALT
#define KEYSTRAFELEFT_DEFAULT                   'a'
#define KEYSTRAFELEFT2_DEFAULT                  ','
#define KEYSTRAFERIGHT_DEFAULT                  'd'
#define KEYSTRAFERIGHT2_DEFAULT                  '.'
#define KEYUP_DEFAULT                           KEY_UPARROW
#define KEYUP2_DEFAULT                          'w'
#define KEYUSE_DEFAULT                          ' '
#define KEYUSE2_DEFAULT                         'e'
#define KEYWEAPON1_DEFAULT                      '1'
#define KEYWEAPON2_DEFAULT                      '2'
#define KEYWEAPON3_DEFAULT                      '3'
#define KEYWEAPON4_DEFAULT                      '4'
#define KEYWEAPON5_DEFAULT                      '5'
#define KEYWEAPON6_DEFAULT                      '6'
#define KEYWEAPON7_DEFAULT                      '7'

#define MOUSEFIRE_DEFAULT                       0
#define MOUSEFORWARD_DEFAULT                    -1
#define MOUSEMOUSELOOK_DEFAULT                  -1
#define MOUSENEXTWEAPON_DEFAULT                 MOUSE_WHEELDOWN
#define MOUSEPREVWEAPON_DEFAULT                 MOUSE_WHEELUP
#define MOUSESTRAFE_DEFAULT                     -1
#define MOUSERUN_DEFAULT                        -1
#define MOUSEUSE_DEFAULT                        -1

typedef enum default_type_e
{
    DEFAULT_INT,
    DEFAULT_INT_UNSIGNED,
    DEFAULT_INT_PERCENT,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
    DEFAULT_FLOAT_PERCENT,
    DEFAULT_OTHER
} default_type_t;

typedef enum valuealias_type_e
{
    NOVALUEALIAS,
    BOOLVALUEALIAS,
    DETAILVALUEALIAS,
    GAMMAVALUEALIAS,
    BLOODVALUEALIAS,
    UNITSVALUEALIAS,
    CAPVALUEALIAS,
    SKYVALUEALIAS,
    SCALEVALUEALIAS
} valuealias_type_t;

typedef struct default_s
{
    // Name of the variable
    char                *name;

    // Pointer to the location in memory of the variable
    void                *location;

    // Type of the variable
    default_type_t      type;

    valuealias_type_t   valuealiastype;
} default_t;

typedef struct valuealias_s
{
    char                *text;
    int                 value;
    valuealias_type_t   type;
} valuealias_t;

extern valuealias_t     valuealiases[];

void M_LoadCVARs(char *filename);
void M_SaveCVARs(void);

#endif
