/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

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
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__M_CONFIG_H__)
#define __M_CONFIG_H__

extern dboolean     alwaysrun;
extern int          am_allmapcdwallcolor;
extern int          am_allmapfdwallcolor;
extern int          am_allmapwallcolor;
extern int          am_backcolor;
extern int          am_cdwallcolor;
extern int          am_crosshaircolor;
extern dboolean     am_external;
extern int          am_fdwallcolor;
extern dboolean     am_followmode;
extern dboolean     am_grid;
extern int          am_gridcolor;
extern char         *am_gridsize;
extern int          am_markcolor;
extern dboolean     am_path;
extern int          am_pathcolor;
extern int          am_playercolor;
extern dboolean     am_rotatemode;
extern int          am_teleportercolor;
extern int          am_thingcolor;
extern int          am_tswallcolor;
extern int          am_wallcolor;
extern dboolean     autoaim;
extern dboolean     autoload;
extern dboolean     autosave;
extern dboolean     autotilt;
extern dboolean     autouse;
extern dboolean     centerweapon;
extern int          con_backcolor;
extern int          con_edgecolor;
extern dboolean     con_obituaries;
extern int          crosshair;
extern int          crosshaircolor;
extern int          episode;
extern int          expansion;
extern int          facebackcolor;
extern dboolean     fade;
extern dboolean     gp_analog;
extern float        gp_deadzone_left;
extern float        gp_deadzone_right;
extern dboolean     gp_invertyaxis;
extern int          gp_sensitivity_horizontal;
extern int          gp_sensitivity_vertical;
extern dboolean     gp_swapthumbsticks;
extern int          gp_thumbsticks;
extern int          gp_vibrate_barrels;
extern int          gp_vibrate_damage;
extern int          gp_vibrate_weapons;
extern dboolean     infighting;
extern dboolean     infiniteheight;
extern char         *iwadfolder;
extern dboolean     m_acceleration;
extern dboolean     m_doubleclick_use;
extern dboolean     m_invertyaxis;
extern dboolean     m_novertical;
extern int          m_sensitivity;
extern dboolean     melt;
extern dboolean     messages;
extern dboolean     mouselook;
extern int          movebob;
extern char         *playername;
extern dboolean     r_althud;
extern int          r_berserkintensity;
extern int          r_blood;
extern int          r_bloodsplats_max;
extern int          r_bloodsplats_total;
extern dboolean     r_bloodsplats_translucency;
extern dboolean     r_brightmaps;
extern int          r_color;
extern dboolean     r_corpses_color;
extern dboolean     r_corpses_gib;
extern dboolean     r_corpses_mirrored;
extern dboolean     r_corpses_moreblood;
extern dboolean     r_corpses_nudge;
extern dboolean     r_corpses_slide;
extern dboolean     r_corpses_smearblood;
extern int          r_detail;
extern dboolean     r_diskicon;
extern dboolean     r_dither;
extern dboolean     r_fixmaperrors;
extern dboolean     r_fixspriteoffsets;
extern dboolean     r_floatbob;
extern int          r_fov;
extern float        r_gamma;
extern dboolean     r_graduallighting;
extern dboolean     r_homindicator;
extern dboolean     r_hud;
extern dboolean     r_hud_translucency;
extern dboolean     r_liquid_bob;
extern dboolean     r_liquid_clipsprites;
extern dboolean     r_liquid_current;
extern dboolean     r_liquid_lowerview;
extern dboolean     r_liquid_swirl;
extern char         *r_lowpixelsize;
extern dboolean     r_mirroredweapons;
extern dboolean     r_playersprites;
extern dboolean     r_rockettrails;
extern int          r_screensize;
extern dboolean     r_shadows;
extern dboolean     r_shadows_translucency;
extern dboolean     r_shake_barrels;
extern int          r_shake_damage;
extern int          r_skycolor;
extern dboolean     r_supersampling;
extern dboolean     r_textures;
extern dboolean     r_translucency;
extern int          s_channels;
extern int          s_musicvolume;
extern dboolean     s_randommusic;
extern dboolean     s_randompitch;
extern int          s_sfxvolume;
extern dboolean     s_stereo;
extern int          savegame;
extern int          skilllevel;
extern uint64_t     stat_barrelsexploded;
extern uint64_t     stat_cheated;
extern uint64_t     stat_damageinflicted;
extern uint64_t     stat_damagereceived;
extern uint64_t     stat_deaths;
extern uint64_t     stat_distancetraveled;
extern uint64_t     stat_gamessaved;
extern uint64_t     stat_itemspickedup;
extern uint64_t     stat_itemspickedup_ammo_bullets;
extern uint64_t     stat_itemspickedup_ammo_cells;
extern uint64_t     stat_itemspickedup_ammo_rockets;
extern uint64_t     stat_itemspickedup_ammo_shells;
extern uint64_t     stat_itemspickedup_armor;
extern uint64_t     stat_itemspickedup_health;
extern uint64_t     stat_mapscompleted;
extern uint64_t     stat_mapsstarted;
extern uint64_t     stat_monsterskilled;
extern uint64_t     stat_monsterskilled_arachnotrons;
extern uint64_t     stat_monsterskilled_archviles;
extern uint64_t     stat_monsterskilled_baronsofhell;
extern uint64_t     stat_monsterskilled_cacodemons;
extern uint64_t     stat_monsterskilled_cyberdemons;
extern uint64_t     stat_monsterskilled_demons;
extern uint64_t     stat_monsterskilled_heavyweapondudes;
extern uint64_t     stat_monsterskilled_hellknights;
extern uint64_t     stat_monsterskilled_imps;
extern uint64_t     stat_monsterskilled_lostsouls;
extern uint64_t     stat_monsterskilled_mancubi;
extern uint64_t     stat_monsterskilled_painelementals;
extern uint64_t     stat_monsterskilled_revenants;
extern uint64_t     stat_monsterskilled_shotgunguys;
extern uint64_t     stat_monsterskilled_spectres;
extern uint64_t     stat_monsterskilled_spidermasterminds;
extern uint64_t     stat_monsterskilled_zombiemen;
extern uint64_t     stat_runs;
extern uint64_t     stat_secretsfound;
extern uint64_t     stat_shotsfired_pistol;
extern uint64_t     stat_shotsfired_shotgun;
extern uint64_t     stat_shotsfired_supershotgun;
extern uint64_t     stat_shotsfired_chaingun;
extern uint64_t     stat_shotsfired_rocketlauncher;
extern uint64_t     stat_shotsfired_plasmarifle;
extern uint64_t     stat_shotsfired_bfg9000;
extern uint64_t     stat_shotssuccessful_pistol;
extern uint64_t     stat_shotssuccessful_shotgun;
extern uint64_t     stat_shotssuccessful_supershotgun;
extern uint64_t     stat_shotssuccessful_chaingun;
extern uint64_t     stat_shotssuccessful_rocketlauncher;
extern uint64_t     stat_shotssuccessful_plasmarifle;
extern uint64_t     stat_shotssuccessful_bfg9000;
extern uint64_t     stat_skilllevel_imtooyoungtodie;
extern uint64_t     stat_skilllevel_heynottoorough;
extern uint64_t     stat_skilllevel_hurtmeplenty;
extern uint64_t     stat_skilllevel_ultraviolence;
extern uint64_t     stat_skilllevel_nightmare;
extern uint64_t     stat_suicides;
extern uint64_t     stat_time;
extern int          stillbob;
extern dboolean     tossdrop;
extern int          turbo;
extern int          units;
extern char         *version;
extern dboolean     vid_borderlesswindow;
extern int          vid_capfps;
extern int          vid_display;
#if !defined(_WIN32)
extern char         *vid_driver;
#endif
extern dboolean     vid_fullscreen;
extern int          vid_motionblur;
extern dboolean     vid_pillarboxes;
extern char         *vid_scaleapi;
extern char         *vid_scalefilter;
extern char         *vid_screenresolution;
extern dboolean     vid_showfps;
extern int          vid_vsync;
extern dboolean     vid_widescreen;
extern char         *vid_windowpos;
extern char         *vid_windowsize;
#if defined(_WIN32)
extern char         *wad;
#endif
extern int          warninglevel;
extern int          weaponbob;
extern dboolean     weaponbounce;
extern dboolean     weaponrecoil;

enum
{
    crosshair_none,
    crosshair_cross,
    crosshair_dot
};

enum
{
    r_blood_none,
    r_blood_red,
    r_blood_all,
    r_blood_green,
    r_blood_nofuzz
};

enum
{
    r_detail_low,
    r_detail_high
};

enum
{
    units_imperial,
    units_metric
};

enum
{
    vid_vsync_adaptive = -1,
    vid_vsync_off,
    vid_vsync_on
};

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

#define am_gridcolor_min                        0
#define am_gridcolor_default                    111
#define am_gridcolor_max                        255

#define am_gridsize_default                     "128x128"

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

#define armortype_min                           armortype_none
#define armortype_default                       armortype_none
#define armortype_max                           armortype_blue

#define autoaim_default                         true

#define autoload_default                        true

#define autosave_default                        true

#define autotilt_default                        false

#define autouse_default                         false

#define centerweapon_default                    true

#define con_backcolor_min                       0
#define con_backcolor_default                   12
#define con_backcolor_max                       255

#define con_edgecolor_min                       0
#define con_edgecolor_default                   180
#define con_edgecolor_max                       255

#define con_obituaries_default                  true

#define crosshair_min                           crosshair_none
#define crosshair_default                       crosshair_none
#define crosshair_max                           crosshair_dot

#define crosshaircolor_min                      0
#define crosshaircolor_default                  4
#define crosshaircolor_max                      255

#define episode_min                             1
#define episode_default                         1
#define episode_max                             5

#define expansion_min                           1
#define expansion_default                       1
#define expansion_max                           2

#define facebackcolor_min                       0
#define facebackcolor_default                   5
#define facebackcolor_max                       255

#define fade_default                            true

#define gp_analog_default                       true

#define gp_deadzone_left_min                    0.0f
#define gp_deadzone_left_default                24.0f
#define gp_deadzone_left_max                    100.0f

#define gp_deadzone_right_min                   0.0f
#define gp_deadzone_right_default               26.5f
#define gp_deadzone_right_max                   100.0f

#define gp_invertyaxis_default                  false

#define gp_sensitivity_horizontal_min           0
#define gp_sensitivity_horizontal_default       64
#define gp_sensitivity_horizontal_max           128

#define gp_sensitivity_vertical_min             0
#define gp_sensitivity_vertical_default         64
#define gp_sensitivity_vertical_max             128

#define gp_swapthumbsticks_default              false

#define gp_thumbsticks_min                      1
#define gp_thumbsticks_default                  2
#define gp_thumbsticks_max                      2

#define gp_vibrate_barrels_min                  0
#define gp_vibrate_barrels_default              100
#define gp_vibrate_barrels_max                  200

#define gp_vibrate_damage_min                   0
#define gp_vibrate_damage_default               100
#define gp_vibrate_damage_max                   200

#define gp_vibrate_weapons_min                  0
#define gp_vibrate_weapons_default              100
#define gp_vibrate_weapons_max                  200

#define health_min                              -100
#define health_default                          100
#define health_max                              INT_MAX

#define infighting_default                      true

#define infiniteheight_default                  false

#if defined(_WIN32)
#define iwadfolder_default                      "C:\\"
#else
#define iwadfolder_default                      "/"
#endif

#define m_acceleration_default                  true

#define m_doubleclick_use_default               false

#define m_invertyaxis_default                   false

#define m_novertical_default                    true

#define m_sensitivity_min                       0
#define m_sensitivity_default                   16
#define m_sensitivity_max                       128

#define melt_default                            true

#define messages_default                        false

#define mouselook_default                       false

#define movebob_min                             0
#define movebob_default                         75
#define movebob_max                             100

#define playername_default                      "you"

#define r_althud_default                        false

#define r_berserkintensity_min                  0
#define r_berserkintensity_default              2
#define r_berserkintensity_max                  8

#define r_blood_min                             r_blood_none
#define r_blood_default                         r_blood_all
#define r_blood_max                             r_blood_nofuzz

#define r_bloodsplats_max_min                   0
#define r_bloodsplats_max_default               65536
#define r_bloodsplats_max_max                   1048576

#define r_bloodsplats_total_min                 0
#define r_bloodsplats_total_default             0
#define r_bloodsplats_total_max                 0

#define r_bloodsplats_translucency_default      true

#define r_brightmaps_default                    true

#define r_color_min                             0
#define r_color_default                         100
#define r_color_max                             100

#define r_corpses_color_default                 true

#define r_corpses_gib_default                   true

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

#define r_fov_min                               45
#define r_fov_default                           90
#define r_fov_max                               135

#define r_gamma_min                             gammalevels[0]
#define r_gamma_default                         0.90f
#define r_gamma_max                             gammalevels[GAMMALEVELS - 1]

#define r_graduallighting_default               true

#define r_homindicator_default                  false

#define r_hud_default                           true

#define r_hud_translucency_default              true

#define r_liquid_bob_default                    true

#define r_liquid_clipsprites_default            true

#define r_liquid_current_default                true

#define r_liquid_lowerview_default              true

#define r_liquid_swirl_default                  true

#define r_lowpixelsize_default                  "2x2"

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

#define r_supersampling_default                 true

#define r_textures_default                      true

#define r_translucency_default                  true

#define s_channels_min                          8
#define s_channels_default                      32
#define s_channels_max                          64

#define s_musicvolume_min                       0
#define s_musicvolume_default                   100
#define s_musicvolume_max                       100

#define s_randommusic_default                   false

#define s_randompitch_default                   false

#define s_sfxvolume_min                         0
#define s_sfxvolume_default                     100
#define s_sfxvolume_max                         100

#define s_stereo_default                        true

#define savegame_min                            1
#define savegame_default                        1
#define savegame_max                            6

#define skilllevel_min                          1
#define skilllevel_default                      3
#define skilllevel_max                          5

#define stillbob_min                            0
#define stillbob_default                        0
#define stillbob_max                            100

#define tossdrop_default                        true

#define turbo_min                               10
#define turbo_default                           100
#define turbo_max                               400

#define units_default                           units_imperial

#define version_default                         PACKAGE_VERSIONSTRING

#define vid_borderlesswindow_default            true

#define vid_capfps_min                          0
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

#if defined(_WIN32)
#define vid_scaleapi_direct3d                   "direct3d"
#elif defined(__APPLE__)
#define vid_scaleapi_metal                      "metal"
#endif
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
#define vid_scalefilter_default                 vid_scalefilter_nearest_linear

#define vid_screenresolution_desktop            "desktop"
#define vid_screenresolution_default            vid_screenresolution_desktop

#define vid_showfps_default                     false

#define vid_vsync_min                           vid_vsync_adaptive
#define vid_vsync_default                       vid_vsync_on
#define vid_vsync_max                           vid_vsync_on

#define vid_widescreen_default                  false

#define vid_windowpos_centered                  "centered"
#define vid_windowpos_centred                   "centred"
#define vid_windowpos_default                   vid_windowpos_centered

#define vid_windowsize_default                  "768x480"

#if defined(_WIN32)
#define wad_default                             ""
#endif

#define warninglevel_min                        0
#define warninglevel_default                    1
#define warninglevel_max                        2

#define weaponbob_min                           0
#define weaponbob_default                       75
#define weaponbob_max                           100

#define weaponbounce_default                    true

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
#define GAMEPADJUMP_DEFAULT                     0
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
#define KEYJUMP_DEFAULT                         0
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
#define KEYSTRAFELEFT_DEFAULT                   ','
#define KEYSTRAFELEFT2_DEFAULT                  'a'
#define KEYSTRAFERIGHT_DEFAULT                  '.'
#define KEYSTRAFERIGHT2_DEFAULT                 'd'
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
#define MOUSEJUMP_DEFAULT                       -1
#define MOUSEMOUSELOOK_DEFAULT                  -1
#define MOUSENEXTWEAPON_DEFAULT                 MOUSE_WHEELDOWN
#define MOUSEPREVWEAPON_DEFAULT                 MOUSE_WHEELUP
#define MOUSERUN_DEFAULT                        -1
#define MOUSESCREENSHOT_DEFAULT                 -1
#define MOUSESTRAFE_DEFAULT                     -1
#define MOUSEUSE_DEFAULT                        -1

typedef enum
{
    DEFAULT_INT32,
    DEFAULT_UINT64,
    DEFAULT_INT32_PERCENT,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
    DEFAULT_FLOAT_PERCENT,
    DEFAULT_OTHER
} default_type_t;

typedef enum
{
    NOVALUEALIAS,
    BOOLVALUEALIAS,
    DETAILVALUEALIAS,
    GAMMAVALUEALIAS,
    BLOODVALUEALIAS,
    UNITSVALUEALIAS,
    CAPVALUEALIAS,
    SKYVALUEALIAS,
    SCALEVALUEALIAS,
    FACEBACKVALUEALIAS,
    ARMORTYPEVALUEALIAS,
    CROSSHAIRVALUEALIAS,
    VSYNCVALUEALIAS
} valuealias_type_t;

typedef struct
{
    // Name of the variable
    char                *name;
    char                *oldname;

    // Pointer to the location in memory of the variable
    void                *location;

    // Type of the variable
    default_type_t      type;

    valuealias_type_t   valuealiastype;
} default_t;

typedef struct
{
    char                *text;
    int                 value;
    valuealias_type_t   type;
} valuealias_t;

extern valuealias_t     valuealiases[];

void M_LoadCVARs(char *filename);
void M_SaveCVARs(void);

#endif
