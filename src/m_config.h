/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#pragma once

#include "i_controller.h"
#include "p_local.h"
#include "version.h"

extern bool     alwaysrun;
extern int      am_allmapcdwallcolor;
extern int      am_allmapfdwallcolor;
extern int      am_allmapwallcolor;
extern bool     am_antialiasing;
extern bool     am_author;
extern int      am_backcolor;
extern int      am_bloodsplatcolor;
extern int      am_bluedoorcolor;
extern int      am_bluekeycolor;
extern int      am_cdwallcolor;
extern int      am_corpsecolor;
extern bool     am_correctaspectratio;
extern int      am_crosshaircolor;
extern int      am_display;
extern bool     am_dynamic;
extern bool     am_external;
extern int      am_fdwallcolor;
extern bool     am_followmode;
extern bool     am_grid;
extern int      am_gridcolor;
extern char     *am_gridsize;
extern int      am_markcolor;
extern bool     am_mousepanning;
extern bool     am_path;
extern int      am_pathcolor;
extern int      am_pathlength;
extern int      am_playercolor;
extern bool     am_playerstats;
extern int      am_playerstatscolor;
extern int      am_reddoorcolor;
extern int      am_redkeycolor;
extern bool     am_rotatemode;
extern int      am_teleportercolor;
extern int      am_thingcolor;
extern int      am_tswallcolor;
extern int      am_wallcolor;
extern int      am_yellowdoorcolor;
extern int      am_yellowkeycolor;
extern bool     animatedstats;
extern bool     autoaim;
extern bool     autoload;
extern bool     autosave;
extern bool     autoswitch;
extern bool     autotilt;
extern bool     autouse;
extern bool     centerweapon;
extern int      con_edgecolor;
extern int      con_timestampformat;
extern bool     con_timestamps;
extern int      con_warninglevel;
extern int      crosshair;
extern int      crosshaircolor;
extern int      english;
extern int      episode;
extern int      expansion;
extern int      facebackcolor;
extern bool     fade;
extern bool     flashkeys;
extern bool     freelook;
extern bool     groupmessages;
extern bool     infighting;
extern bool     infiniteheight;
extern bool     joy_analog;
extern float    joy_deadzone_left;
extern float    joy_deadzone_right;
extern bool     joy_invertyaxis;
extern int      joy_rumble_barrels;
extern int      joy_rumble_damage;
extern bool     joy_rumble_fall;
extern bool     joy_rumble_pickup;
extern int      joy_rumble_weapons;
extern float    joy_sensitivity_horizontal;
extern float    joy_sensitivity_vertical;
extern bool     joy_swapthumbsticks;
extern int      joy_thumbsticks;
extern bool     m_acceleration;
extern bool     m_doubleclick_use;
extern bool     m_invertyaxis;
extern bool     m_novertical;
extern bool     m_pointer;
extern float    m_sensitivity;
extern bool     melt;
extern bool     menuhighlight;
extern bool     menushadow;
extern bool     menuspin;
extern bool     messages;
extern int      movebob;
extern bool     negativehealth;
extern bool     obituaries;
extern int      playergender;
extern char     *playername;
extern bool     r_althud;
extern bool     r_althudfont;
extern bool     r_antialiasing;
extern int      r_berserkeffect;
extern int      r_blood;
extern bool     r_blood_gibs;
extern bool     r_blood_melee;
extern int      r_bloodsplats_max;
extern int      r_bloodsplats_total;
extern bool     r_bloodsplats_translucency;
extern bool     r_brightmaps;
extern bool     r_corpses_color;
extern bool     r_corpses_gib;
extern bool     r_corpses_mirrored;
extern bool     r_corpses_moreblood;
extern bool     r_corpses_nudge;
extern bool     r_corpses_slide;
extern bool     r_corpses_smearblood;
extern bool     r_damageeffect;
extern int      r_detail;
extern bool     r_diskicon;
extern bool     r_ditheredlighting;
extern bool     r_fixmaperrors;
extern bool     r_fixspriteoffsets;
extern bool     r_floatbob;
extern int      r_fov;
extern float    r_gamma;
extern bool     r_graduallighting;
extern bool     r_homindicator;
extern bool     r_hud;
extern bool     r_hud_translucency;
extern int      r_levelbrightness;
extern bool     r_linearskies;
extern bool     r_liquid_bob;
extern bool     r_liquid_bobsprites;
extern bool     r_liquid_clipsprites;
extern bool     r_liquid_current;
extern bool     r_liquid_lowerview;
extern bool     r_liquid_swirl;
extern char     *r_lowpixelsize;
extern bool     r_mirroredweapons;
extern bool     r_percolumnlighting;
extern bool     r_pickupeffect;
extern bool     r_playersprites;
extern bool     r_radsuiteffect;
extern bool     r_randomstartframes;
extern bool     r_rockettrails;
extern bool     r_rockettrails_translucency;
extern int      r_screensize;
extern bool     r_shadows;
extern bool     r_shadows_translucency;
extern bool     r_shake_barrels;
extern bool     r_shake_berserk;
extern bool     r_shake_damage;
extern bool     r_sprites_translucency;
extern bool     r_textures;
extern bool     r_textures_translucency;
extern int      s_channels;
extern bool     s_fullsfx;
extern bool     s_lowermenumusic;
extern bool     s_musicinbackground;
extern int      s_musicvolume;
extern bool     s_randommusic;
extern bool     s_randompitch;
extern int      s_sfxvolume;
extern bool     s_stereo;
extern int      savegame;
extern bool     secretmessages;
extern int      skilllevel;
extern int      stillbob;
extern int      sucktime;
extern bool     tossdrop;
extern int      turbo;
extern int      units;
extern char     *version;
extern int      vid_aspectratio;
extern int      vid_blue;
extern bool     vid_borderlesswindow;
extern int      vid_brightness;
extern int      vid_capfps;
extern int      vid_contrast;
extern int      vid_display;
#if !defined(_WIN32)
extern char     *vid_driver;
#endif
extern bool     vid_fullscreen;
extern int      vid_green;
extern int      vid_motionblur;
extern bool     vid_pillarboxes;
extern int      vid_red;
extern int      vid_saturation;
extern char     *vid_scaleapi;
extern char     *vid_scalefilter;
extern char     *vid_screenresolution;
extern bool     vid_showfps;
extern int      vid_vsync;
extern bool     vid_widescreen;
extern char     *vid_windowpos;
extern char     *vid_windowsize;
#if defined(_WIN32)
extern char     *wad;
#endif
extern char     *wadfolder;
extern int      weaponbob;
extern bool     weaponbounce;
extern bool     weaponrecoil;

extern uint64_t stat_automapopened;
extern uint64_t stat_barrelsexploded;
extern uint64_t stat_cheatsentered;
extern uint64_t stat_damageinflicted;
extern uint64_t stat_damagereceived;
extern uint64_t stat_deaths;
extern uint64_t stat_distancetraveled;
extern uint64_t stat_firstrun;
extern uint64_t stat_gamesloaded;
extern uint64_t stat_gamessaved;
extern uint64_t stat_itemspickedup;
extern uint64_t stat_itemspickedup_ammo[NUMAMMO];
extern uint64_t stat_itemspickedup_ammo_fuel;
extern uint64_t stat_itemspickedup_armor;
extern uint64_t stat_itemspickedup_health;
extern uint64_t stat_itemspickedup_keys;
extern uint64_t stat_itemspickedup_powerups;
extern uint64_t stat_mapsfinished;
extern uint64_t stat_mapsstarted;
extern uint64_t stat_monstersgibbed;
extern uint64_t stat_monsterskilled_total;
extern uint64_t stat_monsterskilled_infighting;
extern uint64_t stat_monsterskilled[NUMMOBJTYPES];
extern uint64_t stat_monstersrespawned;
extern uint64_t stat_monstersresurrected;
extern uint64_t stat_monsterstelefragged;
extern uint64_t stat_runs;
extern uint64_t stat_secretsfound;
extern uint64_t stat_shotsfired[NUMWEAPONS];
extern uint64_t stat_shotsfired_incinerator;
extern uint64_t stat_shotsfired_calamityblade;
extern uint64_t stat_shotssuccessful[NUMWEAPONS];
extern uint64_t stat_shotssuccessful_incinerator;
extern uint64_t stat_shotssuccessful_calamityblade;
extern uint64_t stat_skilllevel_imtooyoungtodie;
extern uint64_t stat_skilllevel_heynottoorough;
extern uint64_t stat_skilllevel_hurtmeplenty;
extern uint64_t stat_skilllevel_ultraviolence;
extern uint64_t stat_skilllevel_nightmare;
extern uint64_t stat_suicides;
extern uint64_t stat_timeplayed;

enum
{
    am_pathlength_short,
    am_pathlength_medium,
    am_pathlength_long,
    am_pathlength_endless
};

enum
{
    con_timestampformat_military,
    con_timestampformat_standard
};

enum
{
    crosshair_none,
    crosshair_cross,
    crosshair_angle,
    crosshair_dot,
    crosshair_bigcross,
    crosshair_circle,
    crosshair_bigcircle,
    crosshair_chevron,
    crosshair_chevrons,
    crosshair_arcs
};

enum
{
    english_american,
    english_british
};

enum
{
    playergender_other,
    playergender_male,
    playergender_female
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
    vid_aspectratio_auto,
    vid_aspectratio_16_9,
    vid_aspectratio_16_10,
    vid_aspectratio_21_9,
    vid_aspectratio_32_9
};

enum
{
#if !defined(__APPLE__)
    vid_vsync_adaptive = -1,
#endif
    vid_vsync_off,
    vid_vsync_on
};

#define alwaysrun_default                   false

#define am_allmapcdwallcolor_min            0
#define am_allmapcdwallcolor_default        109
#define am_allmapcdwallcolor_max            255

#define am_allmapfdwallcolor_min            0
#define am_allmapfdwallcolor_default        110
#define am_allmapfdwallcolor_max            255

#define am_allmapwallcolor_min              0
#define am_allmapwallcolor_default          108
#define am_allmapwallcolor_max              255

#define am_antialiasing_default             false

#define am_author_default                   true

#define am_backcolor_min                    0
#define am_backcolor_default                0
#define am_backcolor_max                    255

#define am_bloodsplatcolor_min              0
#define am_bloodsplatcolor_default          124
#define am_bloodsplatcolor_max              255

#define am_bluedoorcolor_min                0
#define am_bluedoorcolor_default            am_cdwallcolor_default
#define am_bluedoorcolor_max                255

#define am_bluekeycolor_min                 0
#define am_bluekeycolor_default             am_thingcolor_default
#define am_bluekeycolor_max                 255

#define am_cdwallcolor_min                  0
#define am_cdwallcolor_default              160
#define am_cdwallcolor_max                  255

#define am_corpsecolor_min                  0
#define am_corpsecolor_default              116
#define am_corpsecolor_max                  255

#define am_correctaspectratio_default       true

#define am_crosshaircolor_min               0
#define am_crosshaircolor_default           112
#define am_crosshaircolor_max               255

#define am_display_min                      1
#define am_display_default                  2
#define am_display_max                      INT_MAX

#define am_dynamic_default                  true

#define am_external_default                 false

#define am_fdwallcolor_min                  0
#define am_fdwallcolor_default              64
#define am_fdwallcolor_max                  255

#define am_followmode_default               true

#define am_grid_default                     false

#define am_gridcolor_min                    0
#define am_gridcolor_default                6
#define am_gridcolor_max                    255

#define am_gridsize_default                 "128x128"

#define am_gridsize_width_min               4
#define am_gridsize_height_min              4
#define am_gridsize_width_default           128
#define am_gridsize_height_default          128
#define am_gridsize_width_max               4096
#define am_gridsize_height_max              4096

#define am_markcolor_min                    0
#define am_markcolor_default                89
#define am_markcolor_max                    255

#define am_mousepanning_default             true

#define am_path_default                     false

#define am_pathcolor_min                    0
#define am_pathcolor_default                89
#define am_pathcolor_max                    255

#define am_pathlength_min                   am_pathlength_short
#define am_pathlength_default               am_pathlength_medium
#define am_pathlength_max                   am_pathlength_endless

#define am_playercolor_min                  0
#define am_playercolor_default              4
#define am_playercolor_max                  255

#define am_playerstats_default              false

#define am_playerstatscolor_min             0
#define am_playerstatscolor_default         4
#define am_playerstatscolor_max             255

#define am_reddoorcolor_min                 0
#define am_reddoorcolor_default             am_cdwallcolor_default
#define am_reddoorcolor_max                 255

#define am_redkeycolor_min                  0
#define am_redkeycolor_default              am_thingcolor_default
#define am_redkeycolor_max                  255

#define am_rotatemode_default               true

#define am_teleportercolor_min              0
#define am_teleportercolor_default          184
#define am_teleportercolor_max              255

#define am_thingcolor_min                   0
#define am_thingcolor_default               112
#define am_thingcolor_max                   255

#define am_tswallcolor_min                  0
#define am_tswallcolor_default              104
#define am_tswallcolor_max                  255

#define am_wallcolor_min                    0
#define am_wallcolor_default                176
#define am_wallcolor_max                    255

#define am_yellowdoorcolor_min              0
#define am_yellowdoorcolor_default          am_cdwallcolor_default
#define am_yellowdoorcolor_max              255

#define am_yellowkeycolor_min               0
#define am_yellowkeycolor_default           am_thingcolor_default
#define am_yellowkeycolor_max               255

#define ammo_min                            0
#define ammo_default                        50
#define ammo_max                            INT_MAX

#define animatedstats_default               true

#define armor_min                           0
#define armor_default                       0
#define armor_max                           INT_MAX

#define armortype_min                       armortype_none
#define armortype_default                   armortype_none
#define armortype_max                       armortype_blue

#define autoaim_default                     true

#define autoload_default                    true

#define autosave_default                    true

#define autoswitch_default                  true

#define autotilt_default                    false

#define autouse_default                     false

#define centerweapon_default                true

#define con_edgecolor_auto                  -1
#define con_edgecolor_min                   con_edgecolor_auto
#define con_edgecolor_default               con_edgecolor_auto
#define con_edgecolor_max                   255

#define con_timestampformat_default         con_timestampformat_standard

#define con_timestamps_default              true

#define con_warninglevel_min                0
#define con_warninglevel_default            1
#define con_warninglevel_max                2

#define crosshair_min                       crosshair_none
#define crosshair_default                   crosshair_none
#define crosshair_max                       crosshair_arcs

#define crosshaircolor_min                  0
#define crosshaircolor_default              4
#define crosshaircolor_max                  255

#define english_default                     english_american

#define episode_min                         1
#define episode_default                     1
#define episode_max                         6

#define expansion_min                       1
#define expansion_default                   1
#define expansion_max                       2

#define facebackcolor_min                   0
#define facebackcolor_default               5
#define facebackcolor_max                   255

#define fade_default                        true

#define flashkeys_default                   true

#define freelook_default                    false

#define groupmessages_default               true

#define health_min                          INT_MIN
#define health_default                      100
#define health_max                          INT_MAX

#define infighting_default                  true

#define infiniteheight_default              false

#define joy_analog_default                  true

#define joy_deadzone_left_min               0.0f
#define joy_deadzone_left_default           15.0f
#define joy_deadzone_left_max               30.0f

#define joy_deadzone_right_min              0.0f
#define joy_deadzone_right_default          15.0f
#define joy_deadzone_right_max              30.0f

#define joy_invertyaxis_default             false

#define joy_rumble_fall_default             true

#define joy_rumble_pickup_default           true

#define joy_rumble_damage_min               0
#define joy_rumble_damage_default           100
#define joy_rumble_damage_max               200

#define joy_rumble_barrels_min              0
#define joy_rumble_barrels_default          100
#define joy_rumble_barrels_max              200

#define joy_rumble_weapons_min              0
#define joy_rumble_weapons_default          100
#define joy_rumble_weapons_max              200

#define joy_sensitivity_horizontal_min      0.0f
#define joy_sensitivity_horizontal_default  64.0f
#define joy_sensitivity_horizontal_max      128.0f

#define joy_sensitivity_vertical_min        0.0f
#define joy_sensitivity_vertical_default    64.0f
#define joy_sensitivity_vertical_max        128.0f

#define joy_swapthumbsticks_default         false

#define joy_thumbsticks_min                 1
#define joy_thumbsticks_default             2
#define joy_thumbsticks_max                 2

#define m_acceleration_default              true

#define m_doubleclick_use_default           false

#define m_invertyaxis_default               false

#define m_novertical_default                true

#define m_pointer_default                   true

#define m_sensitivity_min                   0.0f
#define m_sensitivity_default               16.0f
#define m_sensitivity_max                   128.0f

#define melt_default                        true

#define menuhighlight_default               true

#define menushadow_default                  true

#define menuspin_default                    true

#define messages_default                    true

#define movebob_min                         0
#define movebob_default                     75
#define movebob_max                         100

#define negativehealth_default              true

#define obituaries_default                  true

#define playergender_min                    playergender_other
#define playergender_default                playergender_male
#define playergender_max                    playergender_female

#define playername_default                  ""

#define r_althud_default                    false

#define r_althudfont_default                true

#define r_antialiasing_default              false

#define r_berserkeffect_min                 0
#define r_berserkeffect_default             3
#define r_berserkeffect_max                 8

#define r_blood_min                         r_blood_none
#define r_blood_default                     r_blood_nofuzz
#define r_blood_max                         r_blood_nofuzz

#define r_blood_gibs_default                true

#define r_blood_melee_default               true

#define r_bloodsplats_max_min               0
#define r_bloodsplats_max_default           131072
#define r_bloodsplats_max_max               1048576

#define r_bloodsplats_total_min             0
#define r_bloodsplats_total_default         0
#define r_bloodsplats_total_max             0

#define r_bloodsplats_translucency_default  true

#define r_brightmaps_default                true

#define r_corpses_color_default             true

#define r_corpses_gib_default               true

#define r_corpses_mirrored_default          true

#define r_corpses_moreblood_default         true

#define r_corpses_nudge_default             true

#define r_corpses_slide_default             true

#define r_corpses_smearblood_default        true

#define r_damageeffect_default              true

#define r_detail_default                    r_detail_high

#define r_diskicon_default                  false

#define r_ditheredlighting_default          true

#define r_fixmaperrors_default              true

#define r_fixspriteoffsets_default          true

#define r_floatbob_default                  true

#define r_fov_min                           45
#define r_fov_default                       90
#define r_fov_max                           135

#define r_gamma_min                         gammalevels[0]
#define r_gamma_default                     0.90f
#define r_gamma_max                         gammalevels[GAMMALEVELS - 1]

#define r_graduallighting_default           true

#define r_homindicator_default              false

#define r_hud_default                       false

#define r_hud_translucency_default          true

#define r_levelbrightness_min               0
#define r_levelbrightness_default           0
#define r_levelbrightness_max               100

#define r_linearskies_default               false

#define r_liquid_bob_default                true

#define r_liquid_bobsprites_default         true

#define r_liquid_clipsprites_default        true

#define r_liquid_current_default            true

#define r_liquid_lowerview_default          true

#define r_liquid_swirl_default              true

#define r_lowpixelsize_default              "2x2"

#define r_mirroredweapons_default           false

#define r_percolumnlighting_default         true

#define r_pickupeffect_default              true

#define r_playersprites_default             true

#define r_radsuiteffect_default             true

#define r_randomstartframes_default         true

#define r_rockettrails_default              true

#define r_rockettrails_translucency_default true

#define r_screensize_min                    0
#define r_screensize_default                7
#define r_screensize_max                    8

#define r_shadows_default                   true

#define r_shadows_translucency_default      true

#define r_shake_barrels_default             true

#define r_shake_berserk_default             true

#define r_shake_damage_default              true

#define r_sprites_translucency_default      true

#define r_textures_default                  true

#define r_textures_translucency_default     true

#define s_channels_min                      8
#define s_channels_default                  32
#define s_channels_max                      64

#define s_fullsfx_default                true

#define s_lowermenumusic_default            true

#define s_musicinbackground_default         false

#define s_musicvolume_min                   0
#define s_musicvolume_default               100
#define s_musicvolume_max                   100

#define s_randommusic_default               false

#define s_randompitch_default               false

#define s_sfxvolume_min                     0
#define s_sfxvolume_default                 100
#define s_sfxvolume_max                     100

#define s_stereo_default                    true

#define savegame_min                        1
#define savegame_default                    1
#define savegame_max                        8

#define secretmessages_default              true

#define skilllevel_min                      1
#define skilllevel_default                  3
#define skilllevel_max                      5

#define stillbob_min                        0
#define stillbob_default                    0
#define stillbob_max                        100

#define sucktime_min                        0
#define sucktime_default                    1
#define sucktime_max                        24

#define tossdrop_default                    true

#define turbo_min                           10
#define turbo_default                       100
#define turbo_max                           400

#define units_default                       units_imperial

#define version_default                     DOOMRETRO_VERSIONSTRING

#define vid_aspectratio_min                 vid_aspectratio_auto
#define vid_aspectratio_default             vid_aspectratio_auto
#define vid_aspectratio_max                 vid_aspectratio_32_9

#define vid_blue_min                        -100
#define vid_blue_default                    0
#define vid_blue_max                        100

#define vid_borderlesswindow_default        true

#define vid_brightness_min                  -100
#define vid_brightness_default              0
#define vid_brightness_max                  100

#define vid_capfps_min                      0
#define vid_capfps_default                  200
#define vid_capfps_max                      1000

#define vid_contrast_min                    -100
#define vid_contrast_default                0
#define vid_contrast_max                    100

#define vid_display_min                     1
#define vid_display_default                 1
#define vid_display_max                     8

#if !defined(_WIN32)
#define vid_driver_default                  ""
#endif

#define vid_fullscreen_default              true

#define vid_green_min                       -100
#define vid_green_default                   0
#define vid_green_max                       100

#define vid_motionblur_min                  0
#define vid_motionblur_default              0
#define vid_motionblur_max                  100

#define vid_pillarboxes_default             false

#define vid_red_min                         -100
#define vid_red_default                     0
#define vid_red_max                         100

#define vid_saturation_min                  -100
#define vid_saturation_default              0
#define vid_saturation_max                  100

#if defined(_WIN32)
#define vid_scaleapi_direct3d               "direct3d"
#endif
#define vid_scaleapi_opengl                 "opengl"
#if !defined(_WIN32)
#define vid_scaleapi_opengles               "opengles"
#define vid_scaleapi_opengles2              "opengles2"
#endif
#define vid_scaleapi_software               "software"
#if defined(_WIN32)
#define vid_scaleapi_default                vid_scaleapi_direct3d
#else
#define vid_scaleapi_default                vid_scaleapi_opengl
#endif

#define vid_scalefilter_linear              "linear"
#define vid_scalefilter_nearest             "nearest"
#define vid_scalefilter_nearest_linear      "nearest_linear"
#define vid_scalefilter_default             vid_scalefilter_nearest_linear

#define vid_screenresolution_desktop        "desktop"
#define vid_screenresolution_default        vid_screenresolution_desktop

#define vid_showfps_default                 false

#if defined(__APPLE__)
#define vid_vsync_min                       vid_vsync_off
#else
#define vid_vsync_min                       vid_vsync_adaptive
#endif
#define vid_vsync_default                   vid_vsync_off
#define vid_vsync_max                       vid_vsync_on

#define vid_widescreen_default              false

#define vid_windowpos_centered              "centered"
#define vid_windowpos_centred               "centred"
#define vid_windowpos_default               vid_windowpos_centered

#define vid_windowsize_default              "854x480"

#if defined(_WIN32)
#define wad_default                         ""
#endif

#if defined(_WIN32)
#define wadfolder_default                   "C:\\"
#else
#define wadfolder_default                   "/"
#endif

#define weapon_min                          wp_fist
#define weapon_default                      wp_pistol
#define weapon_max                          wp_supershotgun

#define weaponbob_min                       0
#define weaponbob_default                   75
#define weaponbob_max                       100

#define weaponbounce_default                true

#define weaponrecoil_default                false

#define KEYALWAYSRUN_DEFAULT                KEY_CAPSLOCK
#define KEYALWAYSRUN2_DEFAULT               0
#define KEYAUTOMAP_DEFAULT                  KEY_TAB
#define KEYAUTOMAP2_DEFAULT                 0
#define KEYBFG9000_DEFAULT                  0
#define KEYBFG90002_DEFAULT                 0
#define KEYCHAINGUN_DEFAULT                 0
#define KEYCHAINGUN2_DEFAULT                0
#define KEYCHAINSAW_DEFAULT                 0
#define KEYCHAINSAW2_DEFAULT                0
#define KEYCLEARMARK_DEFAULT                'c'
#define KEYCLEARMARK2_DEFAULT               0
#define KEYCONSOLE_DEFAULT                  '`'
#define KEYCONSOLE2_DEFAULT                 0
#define KEYDOWN_DEFAULT                     KEY_DOWNARROW
#define KEYDOWN2_DEFAULT                    's'
#define KEYFIRE_DEFAULT                     KEY_CTRL
#define KEYFIRE2_DEFAULT                    0
#define KEYFISTS_DEFAULT                    0
#define KEYFISTS2_DEFAULT                   0
#define KEYFOLLOWMODE_DEFAULT               'f'
#define KEYFOLLOWMODE2_DEFAULT              0
#define KEYFREELOOK_DEFAULT                 0
#define KEYFREELOOK2_DEFAULT                0
#define KEYGRID_DEFAULT                     'g'
#define KEYGRID2_DEFAULT                    0
#define KEYJUMP_DEFAULT                     0
#define KEYJUMP2_DEFAULT                    0
#define KEYLEFT_DEFAULT                     KEY_LEFTARROW
#define KEYLEFT2_DEFAULT                    0
#define KEYMARK_DEFAULT                     'm'
#define KEYMARK2_DEFAULT                    0
#define KEYMAXZOOM_DEFAULT                  '0'
#define KEYMAXZOOM2_DEFAULT                 0
#define KEYMENU_DEFAULT                     KEY_ESCAPE
#define KEYMENU2_DEFAULT                    0
#define KEYNEXTWEAPON_DEFAULT               0
#define KEYNEXTWEAPON2_DEFAULT              0
#define KEYPATH_DEFAULT                     'p'
#define KEYPATH2_DEFAULT                    0
#define KEYPISTOL_DEFAULT                   0
#define KEYPISTOL2_DEFAULT                  0
#define KEYPLASMARIFLE_DEFAULT              0
#define KEYPLASMARIFLE2_DEFAULT             0
#define KEYPREVWEAPON_DEFAULT               0
#define KEYPREVWEAPON2_DEFAULT              0
#define KEYRIGHT_DEFAULT                    KEY_RIGHTARROW
#define KEYRIGHT2_DEFAULT                   0
#define KEYROCKETLAUNCHER_DEFAULT           0
#define KEYROCKETLAUNCHER2_DEFAULT          0
#define KEYROTATEMODE_DEFAULT               'r'
#define KEYROTATEMODE2_DEFAULT              0
#define KEYRUN_DEFAULT                      KEY_SHIFT
#define KEYRUN2_DEFAULT                     0
#if defined(_WIN32)
#define KEYSCREENSHOT_DEFAULT               KEY_PRINTSCREEN
#else
#define KEYSCREENSHOT_DEFAULT               0
#endif
#define KEYSCREENSHOT2_DEFAULT              0
#define KEYSHOTGUN_DEFAULT                  0
#define KEYSHOTGUN2_DEFAULT                 0
#define KEYSTRAFE_DEFAULT                   KEY_ALT
#define KEYSTRAFE2_DEFAULT                  0
#define KEYSTRAFELEFT_DEFAULT               ','
#define KEYSTRAFELEFT2_DEFAULT              'a'
#define KEYSTRAFERIGHT_DEFAULT              '.'
#define KEYSTRAFERIGHT2_DEFAULT             'd'
#define KEYSUPERSHOTGUN_DEFAULT             0
#define KEYSUPERSHOTGUN2_DEFAULT            0
#define KEYUP_DEFAULT                       KEY_UPARROW
#define KEYUP2_DEFAULT                      'w'
#define KEYUSE_DEFAULT                      KEY_SPACE
#define KEYUSE2_DEFAULT                     'e'
#define KEYWEAPON1_DEFAULT                  '1'
#define KEYWEAPON1_2_DEFAULT                0
#define KEYWEAPON2_DEFAULT                  '2'
#define KEYWEAPON2_2_DEFAULT                0
#define KEYWEAPON3_DEFAULT                  '3'
#define KEYWEAPON3_2_DEFAULT                0
#define KEYWEAPON4_DEFAULT                  '4'
#define KEYWEAPON4_2_DEFAULT                0
#define KEYWEAPON5_DEFAULT                  '5'
#define KEYWEAPON5_2_DEFAULT                0
#define KEYWEAPON6_DEFAULT                  '6'
#define KEYWEAPON6_2_DEFAULT                0
#define KEYWEAPON7_DEFAULT                  '7'
#define KEYWEAPON7_2_DEFAULT                0
#define KEYZOOMIN_DEFAULT                   '='
#define KEYZOOMIN2_DEFAULT                  0
#define KEYZOOMOUT_DEFAULT                  '-'
#define KEYZOOMOUT2_DEFAULT                 0

#define MOUSEALWAYSRUN_DEFAULT              -1
#define MOUSEAUTOMAP_DEFAULT                -1
#define MOUSEBACK_DEFAULT                   -1
#define MOUSEBFG9000_DEFAULT                -1
#define MOUSECHAINGUN_DEFAULT               -1
#define MOUSECHAINSAW_DEFAULT               -1
#define MOUSECLEARMARK_DEFAULT              -1
#define MOUSECONSOLE_DEFAULT                -1
#define MOUSEFIRE_DEFAULT                   0
#define MOUSEFISTS_DEFAULT                  -1
#define MOUSEFOLLOWMODE_DEFAULT             -1
#define MOUSEFORWARD_DEFAULT                -1
#define MOUSEFREELOOK_DEFAULT               -1
#define MOUSEGRID_DEFAULT                   -1
#define MOUSEJUMP_DEFAULT                   -1
#define MOUSELEFT_DEFAULT                   -1
#define MOUSEMARK_DEFAULT                   -1
#define MOUSEMAXZOOM_DEFAULT                -1
#define MOUSEMENU_DEFAULT                   -1
#define MOUSENEXTWEAPON_DEFAULT             MOUSE_WHEELDOWN
#define MOUSEPATH_DEFAULT                   -1
#define MOUSEPISTOL_DEFAULT                 -1
#define MOUSEPLASMARIFLE_DEFAULT            -1
#define MOUSEPREVWEAPON_DEFAULT             MOUSE_WHEELUP
#define MOUSERIGHT_DEFAULT                  -1
#define MOUSEROCKETLAUNCHER_DEFAULT         -1
#define MOUSEROTATEMODE_DEFAULT             -1
#define MOUSERUN_DEFAULT                    -1
#define MOUSESCREENSHOT_DEFAULT             -1
#define MOUSESHOTGUN_DEFAULT                -1
#define MOUSESTRAFE_DEFAULT                 -1
#define MOUSESTRAFELEFT_DEFAULT             -1
#define MOUSESTRAFERIGHT_DEFAULT            -1
#define MOUSESUPERSHOTGUN_DEFAULT           -1
#define MOUSEUSE_DEFAULT                    -1
#define MOUSEWEAPON1_DEFAULT                -1
#define MOUSEWEAPON2_DEFAULT                -1
#define MOUSEWEAPON3_DEFAULT                -1
#define MOUSEWEAPON4_DEFAULT                -1
#define MOUSEWEAPON5_DEFAULT                -1
#define MOUSEWEAPON6_DEFAULT                -1
#define MOUSEWEAPON7_DEFAULT                -1
#define MOUSEZOOMIN_DEFAULT                 -1
#define MOUSEZOOMOUT_DEFAULT                -1

#define CONTROLLERALWAYSRUN_DEFAULT         0
#define CONTROLLERAUTOMAP_DEFAULT           CONTROLLER_BACK
#define CONTROLLERBACK_DEFAULT              0
#define CONTROLLERBFG9000_DEFAULT           0
#define CONTROLLERCHAINGUN_DEFAULT          0
#define CONTROLLERCHAINSAW_DEFAULT          0
#define CONTROLLERCLEARMARK_DEFAULT         0
#define CONTROLLERCONSOLE_DEFAULT           0
#define CONTROLLERFIRE_DEFAULT              CONTROLLER_RIGHT_TRIGGER
#define CONTROLLERFISTS_DEFAULT             0
#define CONTROLLERFOLLOWMODE_DEFAULT        0
#define CONTROLLERFORWARD_DEFAULT           0
#define CONTROLLERFREELOOK_DEFAULT          0
#define CONTROLLERGRID_DEFAULT              0
#define CONTROLLERJUMP_DEFAULT              0
#define CONTROLLERLEFT_DEFAULT              0
#define CONTROLLERMARK_DEFAULT              0
#define CONTROLLERMAXZOOM_DEFAULT           0
#define CONTROLLERMENU_DEFAULT              CONTROLLER_START
#define CONTROLLERNEXTWEAPON_DEFAULT        CONTROLLER_RIGHT_SHOULDER
#define CONTROLLERPATH_DEFAULT              0
#define CONTROLLERPISTOL_DEFAULT            0
#define CONTROLLERPLASMARIFLE_DEFAULT       0
#define CONTROLLERPREVWEAPON_DEFAULT        CONTROLLER_LEFT_SHOULDER
#define CONTROLLERRIGHT_DEFAULT             0
#define CONTROLLERROCKETLAUNCHER_DEFAULT    0
#define CONTROLLERROTATEMODE_DEFAULT        0
#define CONTROLLERRUN_DEFAULT               CONTROLLER_LEFT_TRIGGER
#define CONTROLLERSCREENSHOT_DEFAULT        0
#define CONTROLLERSHOTGUN_DEFAULT           0
#define CONTROLLERSTRAFE_DEFAULT            0
#define CONTROLLERSTRAFELEFT_DEFAULT        0
#define CONTROLLERSTRAFERIGHT_DEFAULT       0
#define CONTROLLERSUPERSHOTGUN_DEFAULT      0
#define CONTROLLERUSE_DEFAULT               CONTROLLER_A
#define CONTROLLERUSE2_DEFAULT              CONTROLLER_RIGHT_THUMB
#define CONTROLLERWEAPON1_DEFAULT           0
#define CONTROLLERWEAPON2_DEFAULT           0
#define CONTROLLERWEAPON3_DEFAULT           0
#define CONTROLLERWEAPON4_DEFAULT           0
#define CONTROLLERWEAPON5_DEFAULT           0
#define CONTROLLERWEAPON6_DEFAULT           0
#define CONTROLLERWEAPON7_DEFAULT           0
#define CONTROLLERZOOMIN_DEFAULT            CONTROLLER_RIGHT_SHOULDER
#define CONTROLLERZOOMOUT_DEFAULT           CONTROLLER_LEFT_SHOULDER

typedef enum
{
    DEFAULT_BOOL,
    DEFAULT_INT32,
    DEFAULT_UINT64,
    DEFAULT_INT32_PERCENT,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
    DEFAULT_FLOAT_PERCENT,
    DEFAULT_OTHER
} defaulttype_t;

typedef enum
{
    NOVALUEALIAS,
    ARMORTYPEVALUEALIAS,
    BLOODVALUEALIAS,
    BOOLVALUEALIAS,
    CAPVALUEALIAS,
    CROSSHAIRVALUEALIAS,
    DETAILVALUEALIAS,
    EDGECOLORVALUEALIAS,
    ENGLISHVALUEALIAS,
    GAMMAVALUEALIAS,
    GENDERVALUEALIAS,
    PATHLENGTHVALUEALIAS,
    RATIOVALUEALIAS,
    SCALEVALUEALIAS,
    SUCKSVALUEALIAS,
    TIMESTAMPVALUEALIAS,
    UNITSVALUEALIAS,
    VSYNCVALUEALIAS,
    WEAPONVALUEALIAS
} valuealiastype_t;

typedef struct
{
    // Name of the variable
    char                *name;
    char                *oldname;

    // Pointer to the location in memory of the variable
    void                *location;

    // Type of the variable
    defaulttype_t       type;

    valuealiastype_t    valuealiastype;
} default_t;

typedef struct
{
    char                *text;
    int                 value;
    valuealiastype_t    type;
} valuealias_t;

extern valuealias_t     valuealiases[];

void M_LoadCVARs(const char *filename);
void M_SaveCVARs(void);
