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

#include <ctype.h>
#include <string.h>

#include "c_cmds.h"
#include "c_console.h"
#include "d_iwad.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "version.h"

static dboolean cvarsloaded;

#define NUMCVARS                                                197

#define CONFIG_VARIABLE_INT(name, oldname, cvar, set)           { #name, #oldname, &cvar, DEFAULT_INT32,         set          }
#define CONFIG_VARIABLE_INT_UNSIGNED(name, oldname, cvar, set)  { #name, #oldname, &cvar, DEFAULT_UINT64,        set          }
#define CONFIG_VARIABLE_INT_PERCENT(name, oldname, cvar, set)   { #name, #oldname, &cvar, DEFAULT_INT32_PERCENT, set          }
#define CONFIG_VARIABLE_FLOAT(name, oldname, cvar, set)         { #name, #oldname, &cvar, DEFAULT_FLOAT,         set          }
#define CONFIG_VARIABLE_FLOAT_PERCENT(name, oldname, cvar, set) { #name, #oldname, &cvar, DEFAULT_FLOAT_PERCENT, set          }
#define CONFIG_VARIABLE_STRING(name, oldname, cvar, set)        { #name, #oldname, &cvar, DEFAULT_STRING,        set          }
#define CONFIG_VARIABLE_OTHER(name, oldname, cvar, set)         { #name, #oldname, &cvar, DEFAULT_OTHER,         set          }
#define BLANKLINE                                               { "",     "",      NULL,  DEFAULT_OTHER,         NOVALUEALIAS }
#define COMMENT(text)                                           { text,   "",      NULL,  DEFAULT_OTHER,         NOVALUEALIAS }

static default_t cvars[NUMCVARS] =
{
    COMMENT("; CVARs\n"),
    CONFIG_VARIABLE_INT          (alwaysrun,                        alwaysrun,                             alwaysrun,                             BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (am_allmapcdwallcolor,             am_allmapcdwallcolor,                  am_allmapcdwallcolor,                  NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_allmapfdwallcolor,             am_allmapfdwallcolor,                  am_allmapfdwallcolor,                  NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_allmapwallcolor,               am_allmapwallcolor,                    am_allmapwallcolor,                    NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_backcolor,                     am_backcolor,                          am_backcolor,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_cdwallcolor,                   am_cdwallcolor,                        am_cdwallcolor,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_crosshaircolor,                am_crosshaircolor,                     am_crosshaircolor,                     NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_external,                      am_external,                           am_external,                           BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (am_fdwallcolor,                   am_fdwallcolor,                        am_fdwallcolor,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_followmode,                    am_followmode,                         am_followmode,                         BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (am_grid,                          am_grid,                               am_grid,                               BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (am_gridcolor,                     am_gridcolor,                          am_gridcolor,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_OTHER        (am_gridsize,                      am_gridsize,                           am_gridsize,                           NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_markcolor,                     am_markcolor,                          am_markcolor,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_path,                          am_path,                               am_path,                               BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (am_pathcolor,                     am_pathcolor,                          am_pathcolor,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_playercolor,                   am_playercolor,                        am_playercolor,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_rotatemode,                    am_rotatemode,                         am_rotatemode,                         BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (am_teleportercolor,               am_teleportercolor,                    am_teleportercolor,                    NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_thingcolor,                    am_thingcolor,                         am_thingcolor,                         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_tswallcolor,                   am_tswallcolor,                        am_tswallcolor,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (am_wallcolor,                     am_wallcolor,                          am_wallcolor,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (autoaim,                          autoaim,                               autoaim,                               BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (autoload,                         autoload,                              autoload,                              BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (autosave,                         autosave,                              autosave,                              BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (autotilt,                         autotilt,                              autotilt,                              BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (autouse,                          autouse,                               autouse,                               BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (centerweapon,                     centerweapon,                          centerweapon,                          BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (con_backcolor,                    con_backcolor,                         con_backcolor,                         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (con_edgecolor,                    con_edgecolor,                         con_edgecolor,                         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (con_obituaries,                   con_obituaries,                        con_obituaries,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (crosshair,                        crosshair,                             crosshair,                             CROSSHAIRVALUEALIAS),
    CONFIG_VARIABLE_INT          (crosshaircolor,                   crosshaircolor,                        crosshaircolor,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (episode,                          episode,                               episode,                               NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (expansion,                        expansion,                             expansion,                             NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (facebackcolor,                    facebackcolor,                         facebackcolor,                         FACEBACKVALUEALIAS ),
    CONFIG_VARIABLE_INT          (fade,                             fade,                                  fade,                                  BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (gp_analog,                        gp_analog,                             gp_analog,                             BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_left,                 gp_deadzone_left,                      gp_deadzone_left,                      NOVALUEALIAS       ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_right,                gp_deadzone_right,                     gp_deadzone_right,                     NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (gp_invertyaxis,                   gp_invertyaxis,                        gp_invertyaxis,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (gp_sensitivity_horizontal,        gp_sensitivity_horizontal,             gp_sensitivity_horizontal,             NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (gp_sensitivity_vertical,          gp_sensitivity_vertical,               gp_sensitivity_vertical,               NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (gp_swapthumbsticks,               gp_swapthumbsticks,                    gp_swapthumbsticks,                    BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (gp_thumbsticks,                   gp_thumbsticks,                        gp_thumbsticks,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_PERCENT  (gp_vibrate_barrels,               gp_vibrate_barrels,                    gp_vibrate_barrels,                    NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_PERCENT  (gp_vibrate_damage,                gp_vibrate_damage,                     gp_vibrate_damage,                     NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_PERCENT  (gp_vibrate_weapons,               gp_vibrate_weapons,                    gp_vibrate_weapons,                    NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (infighting,                       infighting,                            infighting,                            BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (infiniteheight,                   infiniteheight,                        infiniteheight,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_STRING       (iwadfolder,                       iwadfolder,                            iwadfolder,                            NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (m_acceleration,                   m_acceleration,                        m_acceleration,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (m_doubleclick_use,                m_doubleclick_use,                     m_doubleclick_use,                     BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (m_invertyaxis,                    m_invertyaxis,                         m_invertyaxis,                         BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (m_novertical,                     m_novertical,                          m_novertical,                          BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (m_sensitivity,                    m_sensitivity,                         m_sensitivity,                         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (melt,                             wipe,                                  melt,                                  BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (messages,                         messages,                              messages,                              BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (mouselook,                        mouselook,                             mouselook,                             BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT_PERCENT  (movebob,                          movebob,                               movebob,                               NOVALUEALIAS       ),
    CONFIG_VARIABLE_STRING       (playername,                       playername,                            playername,                            NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (r_althud,                         r_althud,                              r_althud,                              BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_berserkintensity,               r_berserkintensity,                    r_berserkintensity,                    NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (r_blood,                          r_blood,                               r_blood,                               BLOODVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_bloodsplats_max,                r_bloodsplats_max,                     r_bloodsplats_max,                     NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (r_bloodsplats_translucency,       r_bloodsplats_translucency,            r_bloodsplats_translucency,            BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_brightmaps,                     r_brightmaps,                          r_brightmaps,                          BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT_PERCENT  (r_color,                          r_color,                               r_color,                               NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (r_corpses_color,                  r_corpses_color,                       r_corpses_color,                       BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_corpses_gib,                    r_corpses_gib,                         r_corpses_gib,                         BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_corpses_mirrored,               r_corpses_mirrored,                    r_corpses_mirrored,                    BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_corpses_moreblood,              r_corpses_moreblood,                   r_corpses_moreblood,                   BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_corpses_nudge,                  r_corpses_nudge,                       r_corpses_nudge,                       BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_corpses_slide,                  r_corpses_slide,                       r_corpses_slide,                       BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_corpses_smearblood,             r_corpses_smearblood,                  r_corpses_smearblood,                  BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_detail,                         r_detail,                              r_detail,                              DETAILVALUEALIAS   ),
    CONFIG_VARIABLE_INT          (r_diskicon,                       r_diskicon,                            r_diskicon,                            BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_dither,                         r_dither,                              r_dither,                              BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_fixmaperrors,                   r_fixmaperrors,                        r_fixmaperrors,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_fixspriteoffsets,               r_fixspriteoffsets,                    r_fixspriteoffsets,                    BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_floatbob,                       r_floatbob,                            r_floatbob,                            BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_fov,                            r_fov,                                 r_fov,                                 NOVALUEALIAS       ),
    CONFIG_VARIABLE_FLOAT        (r_gamma,                          r_gamma,                               r_gamma,                               GAMMAVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_graduallighting,                r_graduallighting,                     r_graduallighting,                     BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_homindicator,                   r_homindicator,                        r_homindicator,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_hud,                            r_hud,                                 r_hud,                                 BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_hud_translucency,               r_hud_translucency,                    r_hud_translucency,                    BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_liquid_bob,                     r_liquid_bob,                          r_liquid_bob,                          BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_liquid_clipsprites,             r_liquid_clipsprites,                  r_liquid_clipsprites,                  BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_liquid_current,                 r_liquid_current,                      r_liquid_current,                      BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_liquid_lowerview,               r_liquid_lowerview,                    r_liquid_lowerview,                    BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_liquid_swirl,                   r_liquid_swirl,                        r_liquid_swirl,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_OTHER        (r_lowpixelsize,                   r_lowpixelsize,                        r_lowpixelsize,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (r_mirroredweapons,                r_mirroredweapons,                     r_mirroredweapons,                     BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_playersprites,                  r_playersprites,                       r_playersprites,                       BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_rockettrails,                   r_rockettrails,                        r_rockettrails,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_screensize,                     r_screensize,                          r_screensize,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (r_shadows,                        r_shadows,                             r_shadows,                             BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_shadows_translucency,           r_shadows_translucency,                r_shadows_translucency,                BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_shake_barrels,                  r_shake_barrels,                       r_shake_barrels,                       BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT_PERCENT  (r_shake_damage,                   r_shake_damage,                        r_shake_damage,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (r_skycolor,                       r_skycolor,                            r_skycolor,                            SKYVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (r_supersampling,                  r_supersampling,                       r_supersampling,                       BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_textures,                       r_textures,                            r_textures,                            BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_translucency,                   r_translucency,                        r_translucency,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (s_channels,                       s_channels,                            s_channels,                            NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_PERCENT  (s_musicvolume,                    s_musicvolume,                         s_musicvolume,                         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (s_randommusic,                    s_randommusic,                         s_randommusic,                         BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (s_randompitch,                    s_randompitch,                         s_randompitch,                         BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT_PERCENT  (s_sfxvolume,                      s_sfxvolume,                           s_sfxvolume,                           NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (s_stereo,                         s_stereo,                              s_stereo,                              BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (savegame,                         savegame,                              savegame,                              NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (skilllevel,                       skilllevel,                            skilllevel,                            NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_PERCENT  (stillbob,                         stillbob,                              stillbob,                              NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (tossdrop,                         tossdrop,                              tossdrop,                              BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT_PERCENT  (turbo,                            turbo,                                 turbo,                                 NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (units,                            units,                                 units,                                 UNITSVALUEALIAS    ),
    CONFIG_VARIABLE_STRING       (version,                          version,                               version,                               NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (vid_borderlesswindow,             vid_borderlesswindow,                  vid_borderlesswindow,                  BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (vid_capfps,                       vid_capfps,                            vid_capfps,                            CAPVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (vid_display,                      vid_display,                           vid_display,                           NOVALUEALIAS       ),
#if !defined(_WIN32)
    CONFIG_VARIABLE_STRING       (vid_driver,                       vid_driver,                            vid_driver,                            NOVALUEALIAS       ),
#endif
    CONFIG_VARIABLE_INT          (vid_fullscreen,                   vid_fullscreen,                        vid_fullscreen,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT_PERCENT  (vid_motionblur,                   vid_motionblur,                        vid_motionblur,                        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (vid_pillarboxes,                  vid_pillarboxes,                       vid_pillarboxes,                       BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_STRING       (vid_scaleapi,                     vid_scaleapi,                          vid_scaleapi,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_STRING       (vid_scalefilter,                  vid_scalefilter,                       vid_scalefilter,                       NOVALUEALIAS       ),
    CONFIG_VARIABLE_OTHER        (vid_screenresolution,             vid_screenresolution,                  vid_screenresolution,                  NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (vid_showfps,                      vid_showfps,                           vid_showfps,                           BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (vid_vsync,                        vid_vsync,                             vid_vsync,                             VSYNCVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (vid_widescreen,                   vid_widescreen,                        vid_widescreen,                        BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_OTHER        (vid_windowpos,                    vid_windowpos,                         vid_windowpos,                         NOVALUEALIAS       ),
    CONFIG_VARIABLE_OTHER        (vid_windowsize,                   vid_windowsize,                        vid_windowsize,                        NOVALUEALIAS       ),
#if defined(_WIN32)
    CONFIG_VARIABLE_STRING       (wad,                              wad,                                   wad,                                   NOVALUEALIAS       ),
#endif
    CONFIG_VARIABLE_INT          (warninglevel,                     warninglevel,                          warninglevel,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_PERCENT  (weaponbob,                        weaponbob,                             weaponbob,                             NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT          (weaponbounce,                     weaponbounce,                          weaponbounce,                          BOOLVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (weaponrecoil,                     weaponrecoil,                          weaponrecoil,                          BOOLVALUEALIAS     ),
    BLANKLINE,
    COMMENT("; player stats\n"),
    CONFIG_VARIABLE_INT_UNSIGNED (barrelsexploded,                  stat_barrelsexploded,                  stat_barrelsexploded,                  NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (cheated,                          stat_cheated,                          stat_cheated,                          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (damageinflicted,                  stat_damageinflicted,                  stat_damageinflicted,                  NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (damagereceived,                   stat_damagereceived,                   stat_damagereceived,                   NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (deaths,                           stat_deaths,                           stat_deaths,                           NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (distancetraveled,                 stat_distancetraveled,                 stat_distancetraveled,                 NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (gamessaved,                       stat_gamessaved,                       stat_gamessaved,                       NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (itemspickedup,                    stat_itemspickedup,                    stat_itemspickedup,                    NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (itemspickedup_ammo_bullets,       stat_itemspickedup_ammo_bullets,       stat_itemspickedup_ammo_bullets,       NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (itemspickedup_ammo_cells,         stat_itemspickedup_ammo_cells,         stat_itemspickedup_ammo_cells,         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (itemspickedup_ammo_rockets,       stat_itemspickedup_ammo_rockets,       stat_itemspickedup_ammo_rockets,       NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (itemspickedup_ammo_shells,        stat_itemspickedup_ammo_shells,        stat_itemspickedup_ammo_shells,        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (itemspickedup_armor,              stat_itemspickedup_armor,              stat_itemspickedup_armor,              NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (itemspickedup_health,             stat_itemspickedup_health,             stat_itemspickedup_health,             NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (mapscompleted,                    stat_mapscompleted,                    stat_mapscompleted,                    NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (mapsstarted,                      stat_mapsstarted,                      stat_mapsstarted,                      NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled,                   stat_monsterskilled,                   stat_monsterskilled,                   NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_arachnotrons,      stat_monsterskilled_arachnotrons,      stat_monsterskilled_arachnotrons,      NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_archviles,         stat_monsterskilled_archviles,         stat_monsterskilled_archviles,         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_baronsofhell,      stat_monsterskilled_baronsofhell,      stat_monsterskilled_baronsofhell,      NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_cacodemons,        stat_monsterskilled_cacodemons,        stat_monsterskilled_cacodemons,        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_cyberdemons,       stat_monsterskilled_cyberdemons,       stat_monsterskilled_cyberdemons,       NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_demons,            stat_monsterskilled_demons,            stat_monsterskilled_demons,            NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_heavyweapondudes,  stat_monsterskilled_heavyweapondudes,  stat_monsterskilled_heavyweapondudes,  NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_hellknights,       stat_monsterskilled_hellknights,       stat_monsterskilled_hellknights,       NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_imps,              stat_monsterskilled_imps,              stat_monsterskilled_imps,              NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_lostsouls,         stat_monsterskilled_lostsouls,         stat_monsterskilled_lostsouls,         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_mancubi,           stat_monsterskilled_mancubi,           stat_monsterskilled_mancubi,           NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_painelementals,    stat_monsterskilled_painelementals,    stat_monsterskilled_painelementals,    NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_revenants,         stat_monsterskilled_revenants,         stat_monsterskilled_revenants,         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_shotgunguys,       stat_monsterskilled_shotgunguys,       stat_monsterskilled_shotgunguys,       NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_spectres,          stat_monsterskilled_spectres,          stat_monsterskilled_spectres,          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_spidermasterminds, stat_monsterskilled_spidermasterminds, stat_monsterskilled_spidermasterminds, NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (monsterskilled_zombiemen,         stat_monsterskilled_zombiemen,         stat_monsterskilled_zombiemen,         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (runs,                             stat_runs,                             stat_runs,                             NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (secretsfound,                     stat_secretsrevealed,                  stat_secretsfound,                     NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotsfired_pistol,                stat_shotsfired_pistol,                stat_shotsfired_pistol,                NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotsfired_shotgun,               stat_shotsfired_shotgun,               stat_shotsfired_shotgun,               NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotsfired_supershotgun,          stat_shotsfired_supershotgun,          stat_shotsfired_supershotgun,          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotsfired_chaingun,              stat_shotsfired_chaingun,              stat_shotsfired_chaingun,              NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotsfired_rocketlauncher,        stat_shotsfired_rocketlauncher,        stat_shotsfired_rocketlauncher,        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotsfired_plasmarifle,           stat_shotsfired_plasmarifle,           stat_shotsfired_plasmarifle,           NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotsfired_bfg9000,               stat_shotsfired_bfg9000,               stat_shotsfired_bfg9000,               NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotssuccessful_pistol,           stat_shotssuccessful_pistol,           stat_shotssuccessful_pistol,           NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotssuccessful_shotgun,          stat_shotssuccessful_shotgun,          stat_shotssuccessful_shotgun,          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotssuccessful_supershotgun,     stat_shotssuccessful_supershotgun,     stat_shotssuccessful_supershotgun,     NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotssuccessful_chaingun,         stat_shotssuccessful_chaingun,         stat_shotssuccessful_chaingun,         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotssuccessful_rocketlauncher,   stat_shotssuccessful_rocketlauncher,   stat_shotssuccessful_rocketlauncher,   NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotssuccessful_plasmarifle,      stat_shotssuccessful_plasmarifle,      stat_shotssuccessful_plasmarifle,      NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (shotssuccessful_bfg9000,          stat_shotssuccessful_bfg9000,          stat_shotssuccessful_bfg9000,          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (skilllevel_heynottoorough,        stat_skilllevel_heynottoorough,        stat_skilllevel_heynottoorough,        NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (skilllevel_hurtmeplenty,          stat_skilllevel_hurtmeplenty,          stat_skilllevel_hurtmeplenty,          NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (skilllevel_imtooyoungtodie,       stat_skilllevel_imtooyoungtodie,       stat_skilllevel_imtooyoungtodie,       NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (skilllevel_nightmare,             stat_skilllevel_nightmare,             stat_skilllevel_nightmare,             NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (skilllevel_ultraviolence,         stat_skilllevel_ultraviolence,         stat_skilllevel_ultraviolence,         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (suicides,                         stat_suicides,                         stat_suicides,                         NOVALUEALIAS       ),
    CONFIG_VARIABLE_INT_UNSIGNED (time,                             stat_time,                             stat_time,                             NOVALUEALIAS       )
};

valuealias_t valuealiases[] =
{
    { "off",       0, BOOLVALUEALIAS      }, { "on",      1, BOOLVALUEALIAS      },
    { "0",         0, BOOLVALUEALIAS      }, { "1",       1, BOOLVALUEALIAS      },
    { "no",        0, BOOLVALUEALIAS      }, { "yes",     1, BOOLVALUEALIAS      },
    { "false",     0, BOOLVALUEALIAS      }, { "true",    1, BOOLVALUEALIAS      },
    { "low",       0, DETAILVALUEALIAS    }, { "high",    1, DETAILVALUEALIAS    },
    { "off",       1, GAMMAVALUEALIAS     }, { "none",    0, BLOODVALUEALIAS     },
    { "red",       1, BLOODVALUEALIAS     }, { "all",     2, BLOODVALUEALIAS     },
    { "green",     3, BLOODVALUEALIAS     }, { "nofuzz",  4, BLOODVALUEALIAS     },
    { "imperial",  0, UNITSVALUEALIAS     }, { "metric",  1, UNITSVALUEALIAS     },
    { "off",       0, CAPVALUEALIAS       }, { "none",   -1, SKYVALUEALIAS       },
    { "off",      -1, SKYVALUEALIAS       }, { "none",    5, FACEBACKVALUEALIAS  },
    { "off",       5, FACEBACKVALUEALIAS  }, { "none",    0, ARMORTYPEVALUEALIAS },
    { "green",     1, ARMORTYPEVALUEALIAS }, { "blue",    2, ARMORTYPEVALUEALIAS },
    { "none",      0, CROSSHAIRVALUEALIAS }, { "off",     0, CROSSHAIRVALUEALIAS },
    { "cross",     1, CROSSHAIRVALUEALIAS }, { "dot",     2, CROSSHAIRVALUEALIAS },
    { "adaptive", -1, VSYNCVALUEALIAS     }, { "off",     0, VSYNCVALUEALIAS     },
    { "on",        1, VSYNCVALUEALIAS     }, { "",        0, NOVALUEALIAS        }
};

static void SaveBind(FILE *file, char *control, char *string)
{
    if (strlen(control) == 1)
        fprintf(file, "bind '%s' %s\n", (control[0] == '=' ? "+" : control), string);
    else
        fprintf(file, "bind %s %s\n", control, string);
}

static void SaveBindByValue(FILE *file, char *action, int value, controltype_t type)
{
    for (int i = 0; controls[i].type; i++)
        if (controls[i].type == type && controls[i].value == value)
        {
            SaveBind(file, controls[i].control, action);
            break;
        }
}

//
// M_SaveCVARs
//
void M_SaveCVARs(void)
{
    int     numaliases = 0;
    FILE    *file;

    if (!cvarsloaded || vanilla || togglingvanilla)
        return;

    if (!(file = fopen(packageconfig, "w")))
    {
        static dboolean warning;

        if (!warning)
        {
            warning = true;
            C_Warning(1, "<b>%s</b> couldn't be saved.", packageconfig);
        }

        return;
    }

    if (returntowidescreen)
        vid_widescreen = true;

    for (int i = 0; i < NUMCVARS; i++)
    {
        if (!*cvars[i].name)
        {
            fputs("\n", file);
            continue;
        }

        if (cvars[i].name[0] == ';')
        {
            fputs(cvars[i].name, file);
            continue;
        }

        // Print the name
        fprintf(file, "%s ", cvars[i].name);

        // Print the value
        switch (cvars[i].type)
        {
            case DEFAULT_INT32:
            {
                dboolean    flag = false;
                int         v = *(int *)cvars[i].location;

                for (int j = 0; *valuealiases[j].text; j++)
                    if (v == valuealiases[j].value && cvars[i].valuealiastype == valuealiases[j].type)
                    {
                        fputs(valuealiases[j].text, file);
                        flag = true;
                        break;
                    }

                if (!flag)
                {
                    char    *temp = commify(v);

                    fputs(temp, file);
                    free(temp);
                }

                break;
            }

            case DEFAULT_UINT64:
            {
                char    *temp = commify(*(uint64_t *)cvars[i].location);

                fputs(temp, file);
                free(temp);
                break;
            }

            case DEFAULT_INT32_PERCENT:
            {
                dboolean    flag = false;
                int         v = *(int *)cvars[i].location;

                for (int j = 0; *valuealiases[j].text; j++)
                    if (v == valuealiases[j].value && cvars[i].valuealiastype == valuealiases[j].type)
                    {
                        fputs(valuealiases[j].text, file);
                        flag = true;
                        break;
                    }

                if (!flag)
                {
                    char    *temp = commify(v);

                    fprintf(file, "%s%%", temp);
                    free(temp);
                }

                break;
            }

            case DEFAULT_FLOAT:
            {
                dboolean    flag = false;
                float       v = *(float *)cvars[i].location;

                for (int j = 0; *valuealiases[j].text; j++)
                    if (v == valuealiases[j].value && cvars[i].valuealiastype == valuealiases[j].type)
                    {
                        fputs(valuealiases[j].text, file);
                        flag = true;
                        break;
                    }

                if (!flag)
                {
                    static char buffer[128];
                    int         len;

                    M_snprintf(buffer, sizeof(buffer), "%.2f", v);
                    len = (int)strlen(buffer);

                    if (len >= 2 && buffer[len - 1] == '0' && buffer[len - 2] == '0')
                        buffer[len - 1] = '\0';

                    fputs(buffer, file);
                }

                break;
            }

            case DEFAULT_FLOAT_PERCENT:
            {
                dboolean    flag = false;
                float       v = *(float *)cvars[i].location;

                for (int j = 0; *valuealiases[j].text; j++)
                    if (v == valuealiases[j].value && cvars[i].valuealiastype == valuealiases[j].type)
                    {
                        fputs(valuealiases[j].text, file);
                        flag = true;
                        break;
                    }

                if (!flag)
                {
                    char    *temp = striptrailingzero(v, 1);

                    fprintf(file, "%s%%", temp);
                    free(temp);
                }

                break;
            }

            case DEFAULT_STRING:
                if (M_StringCompare(*(char **)cvars[i].location, EMPTYVALUE))
                    fputs(*(char **)cvars[i].location, file);
                else
                    fprintf(file, "%s%s%s", (M_StringCompare(cvars[i].name, "version") ? "" : "\""),
                        *(char **)cvars[i].location, (M_StringCompare(cvars[i].name, "version") ? "" : "\""));

                break;

            case DEFAULT_OTHER:
                fputs(*(char **)cvars[i].location, file);
                break;
        }

        fputs("\n", file);
    }

    fputs("\n; bound controls\n", file);

    for (int i = 0; *actions[i].action; i++)
    {
        if (actions[i].keyboard1)
            SaveBindByValue(file, actions[i].action, *(int *)actions[i].keyboard1, keyboardcontrol);

        if (actions[i].keyboard2)
            SaveBindByValue(file, actions[i].action, *(int *)actions[i].keyboard2, keyboardcontrol);

        if (actions[i].mouse1)
            SaveBindByValue(file, actions[i].action, *(int *)actions[i].mouse1, mousecontrol);

        if (actions[i].gamepad1)
            SaveBindByValue(file, actions[i].action, *(int *)actions[i].gamepad1, gamepadcontrol);

        if (actions[i].gamepad2)
            SaveBindByValue(file, actions[i].action, *(int *)actions[i].gamepad2, gamepadcontrol);
    }

    for (int i = 0; controls[i].type; i++)
        if (controls[i].type == keyboardcontrol && keyactionlist[controls[i].value][0])
            SaveBind(file, controls[i].control, keyactionlist[controls[i].value]);
        else if (controls[i].type == mousecontrol && mouseactionlist[controls[i].value][0])
            SaveBind(file, controls[i].control, mouseactionlist[controls[i].value]);

    for (int i = 0; i < MAXALIASES; i++)
        if (*aliases[i].name)
            numaliases++;

    if (numaliases)
    {
        fputs("\n; aliases\n", file);

        for (int i = 0; i < MAXALIASES; i++)
            if (*aliases[i].name)
                fprintf(file, "alias %s \"%s\"\n", aliases[i].name, aliases[i].string);
    }

    fclose(file);

    if (returntowidescreen)
        vid_widescreen = false;
}

// Parses integer values in the configuration file
static int ParseIntParameter(char *strparm, int valuealiastype)
{
    int parm = INT_MAX;

    for (int i = 0; *valuealiases[i].text; i++)
        if (M_StringCompare(strparm, valuealiases[i].text) && valuealiastype == valuealiases[i].type)
            return valuealiases[i].value;

    sscanf(strparm, "%10d", &parm);
    return parm;
}

// Parses float values in the configuration file
static float ParseFloatParameter(char *strparm, int valuealiastype)
{
    for (int i = 0; *valuealiases[i].text; i++)
        if (M_StringCompare(strparm, valuealiases[i].text) && valuealiastype == valuealiases[i].type)
            return (float)valuealiases[i].value;

    return (float)atof(strparm);
}

static void M_CheckCVARs(void)
{
    if (alwaysrun != false && alwaysrun != true)
        alwaysrun = alwaysrun_default;

    if (am_allmapcdwallcolor < am_allmapcdwallcolor_min || am_allmapcdwallcolor > am_allmapcdwallcolor_max)
        am_allmapcdwallcolor = am_allmapcdwallcolor_default;

    if (am_allmapfdwallcolor < am_allmapfdwallcolor_min || am_allmapfdwallcolor > am_allmapfdwallcolor_max)
        am_allmapfdwallcolor = am_allmapfdwallcolor_default;

    if (am_allmapwallcolor < am_allmapwallcolor_min || am_allmapwallcolor > am_allmapwallcolor_max)
        am_allmapwallcolor = am_allmapwallcolor_default;

    if (am_backcolor < am_backcolor_min || am_backcolor > am_backcolor_max)
        am_backcolor = am_backcolor_default;

    if (am_cdwallcolor < am_cdwallcolor_min || am_cdwallcolor > am_cdwallcolor_max)
        am_cdwallcolor = am_cdwallcolor_default;

    if (am_crosshaircolor < am_crosshaircolor_min || am_crosshaircolor > am_crosshaircolor_max)
        am_crosshaircolor = am_crosshaircolor_default;

    if (am_external != false && am_external != true)
        am_external = am_external_default;

    if (am_fdwallcolor < am_fdwallcolor_min || am_fdwallcolor > am_fdwallcolor_max)
        am_fdwallcolor = am_fdwallcolor_default;

    if (am_followmode != false && am_followmode != true)
        am_followmode = am_followmode_default;

    if (am_grid != false && am_grid != true)
        am_grid = am_grid_default;

    if (am_gridcolor < am_gridcolor_min || am_gridcolor > am_gridcolor_max)
        am_gridcolor = am_gridcolor_default;

    if (am_markcolor < am_markcolor_min || am_markcolor > am_markcolor_max)
        am_markcolor = am_markcolor_default;

    if (am_path != false && am_path != true)
        am_path = am_path_default;

    if (am_pathcolor < am_pathcolor_min || am_pathcolor > am_pathcolor_max)
        am_pathcolor = am_pathcolor_default;

    if (am_playercolor < am_playercolor_min || am_playercolor > am_playercolor_max)
        am_playercolor = am_playercolor_default;

    if (am_rotatemode != false && am_rotatemode != true)
        am_rotatemode = am_rotatemode_default;

    if (am_teleportercolor < am_teleportercolor_min || am_teleportercolor > am_teleportercolor_max)
        am_teleportercolor = am_teleportercolor_default;

    if (am_thingcolor < am_thingcolor_min || am_thingcolor > am_thingcolor_max)
        am_thingcolor = am_thingcolor_default;

    if (am_tswallcolor < am_tswallcolor_min || am_tswallcolor > am_tswallcolor_max)
        am_tswallcolor = am_tswallcolor_default;

    if (am_wallcolor < am_wallcolor_min || am_wallcolor > am_wallcolor_max)
        am_wallcolor = am_wallcolor_default;

    if (autoaim != false && autoaim != true)
        autoaim = autoaim_default;

    if (autoload != false && autoload != true)
        autoload = autoload_default;

    if (autosave != false && autosave != true)
        autosave = autosave_default;

    if (autotilt != false && autotilt != true)
        autotilt = autotilt_default;

    if (autouse != false && autouse != true)
        autouse = autouse_default;

    if (centerweapon != false && centerweapon != true)
        centerweapon = centerweapon_default;

    if (con_backcolor < con_backcolor_min || con_backcolor > con_backcolor_max)
        con_backcolor = con_backcolor_default;

    if (con_edgecolor < con_edgecolor_min || con_edgecolor > con_edgecolor_max)
        con_edgecolor = con_edgecolor_default;

    if (con_obituaries != false && con_obituaries != true)
        con_obituaries = con_obituaries_default;

    if (crosshair != crosshair_none && crosshair != crosshair_cross && crosshair != crosshair_dot)
        crosshair = crosshair_default;

    if (crosshaircolor < crosshaircolor_min || crosshaircolor > crosshaircolor_max)
        crosshaircolor = crosshaircolor_default;

    episode = BETWEEN(episode_min, episode, episode_max);

    expansion = BETWEEN(expansion_min, expansion, expansion_max);

    if (facebackcolor < facebackcolor_min || facebackcolor > facebackcolor_max)
        facebackcolor = facebackcolor_default;

    if (fade != false && fade != true)
        fade = fade_default;

    if (gp_analog != false && gp_analog != true)
        gp_analog = gp_analog_default;

    gp_deadzone_left = BETWEENF(gp_deadzone_left_min, gp_deadzone_left, gp_deadzone_left_max);
    I_SetGamepadLeftDeadZone();

    gp_deadzone_right = BETWEENF(gp_deadzone_right_min, gp_deadzone_right, gp_deadzone_right_max);
    I_SetGamepadRightDeadZone();

    if (gp_invertyaxis != false && gp_invertyaxis != true)
        gp_invertyaxis = gp_invertyaxis_default;

    gp_sensitivity_horizontal = BETWEEN(gp_sensitivity_horizontal_min, gp_sensitivity_horizontal, gp_sensitivity_horizontal_max);
    I_SetGamepadHorizontalSensitivity();

    gp_sensitivity_vertical = BETWEEN(gp_sensitivity_vertical_min, gp_sensitivity_vertical, gp_sensitivity_vertical_max);
    I_SetGamepadVerticalSensitivity();

    if (gp_swapthumbsticks != false && gp_swapthumbsticks != true)
        gp_swapthumbsticks = gp_swapthumbsticks_default;

    if (gp_thumbsticks < gp_thumbsticks_min || gp_thumbsticks > gp_thumbsticks_max)
        gp_thumbsticks = gp_thumbsticks_default;

    gp_vibrate_barrels = BETWEEN(gp_vibrate_barrels_min, gp_vibrate_barrels, gp_vibrate_barrels_max);

    gp_vibrate_damage = BETWEEN(gp_vibrate_damage_min, gp_vibrate_damage, gp_vibrate_damage_max);

    gp_vibrate_weapons = BETWEEN(gp_vibrate_weapons_min, gp_vibrate_damage, gp_vibrate_weapons_max);

    if (infighting != false && infighting != true)
        infighting = infighting_default;

    if (infiniteheight != false && infiniteheight != true)
        infiniteheight = infiniteheight_default;

    if (!*iwadfolder || M_StringCompare(iwadfolder, iwadfolder_default) || !M_FolderExists(iwadfolder))
        D_InitIWADFolder();

    if (m_acceleration != false && m_acceleration != true)
        m_acceleration = m_acceleration_default;

    if (m_doubleclick_use != false && m_doubleclick_use != true)
        m_doubleclick_use = m_doubleclick_use_default;

    if (m_invertyaxis != false && m_invertyaxis != true)
        m_invertyaxis = m_invertyaxis_default;

    if (m_novertical != false && m_novertical != true)
        m_novertical = m_novertical_default;

    m_sensitivity = BETWEEN(m_sensitivity_min, m_sensitivity, m_sensitivity_max);

    if (melt != false && melt != true)
        melt = melt_default;

    if (messages != false && messages != true)
        messages = messages_default;

    if (mouselook != false && mouselook != true)
        mouselook = mouselook_default;

    movebob = BETWEEN(movebob_min, movebob, movebob_max);

    if (!*playername)
        playername = M_StringDuplicate(playername_default);

    if (r_althud != false && r_althud != true)
        r_althud = r_althud_default;

    r_berserkintensity = BETWEEN(r_berserkintensity_min, r_berserkintensity, r_berserkintensity_max);

    if (r_blood != r_blood_none && r_blood != r_blood_red && r_blood != r_blood_all && r_blood != r_blood_green
        && r_blood != r_blood_nofuzz)
        r_blood = r_blood_default;

    r_bloodsplats_max = BETWEEN(r_bloodsplats_max_min, r_bloodsplats_max, r_bloodsplats_max_max);

    if (r_bloodsplats_translucency != false && r_bloodsplats_translucency != true)
        r_bloodsplats_translucency = r_bloodsplats_translucency_default;

    if (r_brightmaps != false && r_brightmaps != true)
        r_brightmaps = r_brightmaps_default;

    r_color = BETWEEN(r_color_min, r_color, r_color_max);

    if (r_corpses_color != false && r_corpses_color != true)
        r_corpses_color = r_corpses_color_default;

    if (r_corpses_gib != false && r_corpses_gib != true)
        r_corpses_gib = r_corpses_gib_default;

    if (r_corpses_mirrored != false && r_corpses_mirrored != true)
        r_corpses_mirrored = r_corpses_mirrored_default;

    if (r_corpses_moreblood != false && r_corpses_moreblood != true)
        r_corpses_moreblood = r_corpses_moreblood_default;

    if (r_corpses_nudge != false && r_corpses_nudge != true)
        r_corpses_nudge = r_corpses_nudge_default;

    if (r_corpses_slide != false && r_corpses_slide != true)
        r_corpses_slide = r_corpses_slide_default;

    if (r_corpses_smearblood != false && r_corpses_smearblood != true)
        r_corpses_smearblood = r_corpses_smearblood_default;

    if (r_detail != r_detail_low && r_detail != r_detail_high)
        r_detail = r_detail_default;

    if (r_diskicon != false && r_diskicon != true)
        r_diskicon = r_diskicon_default;

    if (r_dither != false && r_dither != true)
        r_dither = r_dither_default;

    if (r_fixmaperrors != false && r_fixmaperrors != true)
        r_fixmaperrors = r_fixmaperrors_default;

    if (r_fixspriteoffsets != false && r_fixspriteoffsets != true)
        r_fixspriteoffsets = r_fixspriteoffsets_default;

    if (r_floatbob != false && r_floatbob != true)
        r_floatbob = r_floatbob_default;

    r_fov = BETWEEN(r_fov_min, r_fov, r_fov_max);

    r_gamma = BETWEENF(r_gamma_min, r_gamma, r_gamma_max);
    I_SetGamma(r_gamma);

    if (r_graduallighting != false && r_graduallighting != true)
        r_graduallighting = r_graduallighting_default;

    if (r_homindicator != false && r_homindicator != true)
        r_homindicator = r_homindicator_default;

    if (r_hud != false && r_hud != true)
        r_hud = r_hud_default;

    if (r_hud_translucency != false && r_hud_translucency != true)
        r_hud_translucency = r_hud_translucency_default;

    if (r_liquid_bob != false && r_liquid_bob != true)
        r_liquid_bob = r_liquid_bob_default;

    if (r_liquid_clipsprites != false && r_liquid_clipsprites != true)
        r_liquid_clipsprites = r_liquid_clipsprites_default;

    if (r_liquid_current != false && r_liquid_current != true)
        r_liquid_current = r_liquid_current_default;

    if (r_liquid_lowerview != false && r_liquid_lowerview != true)
        r_liquid_lowerview = r_liquid_lowerview_default;

    if (r_liquid_swirl != false && r_liquid_swirl != true)
        r_liquid_swirl = r_liquid_swirl_default;

    if (r_mirroredweapons != false && r_mirroredweapons != true)
        r_mirroredweapons = r_mirroredweapons_default;

    if (r_playersprites != false && r_playersprites != true)
        r_playersprites = r_playersprites_default;

    if (r_rockettrails != false && r_rockettrails != true)
        r_rockettrails = r_rockettrails_default;

    r_screensize = BETWEEN(r_screensize_min, r_screensize, r_screensize_max);

    if (r_shadows != false && r_shadows != true)
        r_shadows = r_shadows_default;

    if (r_shadows_translucency != false && r_shadows_translucency != true)
        r_shadows_translucency = r_shadows_translucency_default;

    if (r_shake_barrels != false && r_shake_barrels != true)
        r_shake_barrels = r_shake_barrels_default;

    r_shake_damage = BETWEEN(r_shake_damage_min, r_shake_damage, r_shake_damage_max);

    if (r_skycolor != r_skycolor_none && (r_skycolor < r_skycolor_min || r_skycolor > r_skycolor_max))
        r_skycolor = r_skycolor_default;

    if (r_supersampling != false && r_supersampling != true)
        r_supersampling = r_supersampling_default;

    if (r_textures != false && r_textures != true)
        r_textures = r_textures_default;

    if (r_translucency != false && r_translucency != true)
        r_translucency = r_translucency_default;

    s_channels = BETWEEN(s_channels_min, s_channels, s_channels_max);

    s_musicvolume = BETWEEN(s_musicvolume_min, s_musicvolume, s_musicvolume_max);
    musicVolume = (s_musicvolume * 31 + 50) / 100;

    if (s_randommusic != false && s_randommusic != true)
        s_randommusic = s_randommusic_default;

    if (s_randompitch != false && s_randompitch != true)
        s_randompitch = s_randompitch_default;

    s_sfxvolume = BETWEEN(s_sfxvolume_min, s_sfxvolume, s_sfxvolume_max);
    sfxVolume = (s_sfxvolume * 31 + 50) / 100;

    if (s_stereo != false && s_stereo != true)
        s_stereo = s_stereo_default;

    savegame = BETWEEN(savegame_min, savegame, savegame_max);

    skilllevel = BETWEEN(skilllevel_min, skilllevel, skilllevel_max);

    stillbob = BETWEEN(stillbob_min, stillbob, stillbob_max);

    if (tossdrop != false && tossdrop != true)
        tossdrop = tossdrop_default;

    turbo = BETWEEN(turbo_min, turbo, turbo_max);

    if (units != units_imperial && units != units_metric)
        units = units_default;

    version = version_default;

    if (vid_borderlesswindow != false && vid_borderlesswindow != true)
        vid_borderlesswindow = vid_borderlesswindow_default;

    vid_capfps = BETWEEN(vid_capfps_min, vid_capfps, vid_capfps_max);

    vid_display = MAX(vid_display_min, vid_display);

    if (vid_fullscreen != false && vid_fullscreen != true)
        vid_fullscreen = vid_fullscreen_default;

    vid_motionblur = BETWEEN(vid_motionblur_min, vid_motionblur, vid_motionblur_max);

    if (vid_pillarboxes != false && vid_pillarboxes != true)
        vid_pillarboxes = vid_pillarboxes_default;

    if (!M_StringCompare(vid_scaleapi, vid_scaleapi_software)
#if defined(_WIN32)
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_direct3d)
#endif
#if defined(__APPLE__)
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_metal)
#endif
#if !defined(_WIN32)
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_opengles)
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_opengles2)
#endif
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_opengl))
        vid_scaleapi = vid_scaleapi_default;

    if (!M_StringCompare(vid_scalefilter, vid_scalefilter_linear)
        && !M_StringCompare(vid_scalefilter, vid_scalefilter_nearest)
        && !M_StringCompare(vid_scalefilter, vid_scalefilter_nearest_linear))
        vid_scalefilter = vid_scalefilter_default;

    vid_showfps = vid_showfps_default;

    if (vid_vsync != vid_vsync_adaptive && vid_vsync != vid_vsync_off && vid_vsync != vid_vsync_on)
        vid_vsync = vid_vsync_default;

    if (vid_widescreen != false && vid_widescreen != true)
        vid_widescreen = vid_widescreen_default;

    if (vid_widescreen)
    {
        returntowidescreen = true;
        vid_widescreen = false;
        r_screensize = r_screensize_max;
    }
    else
        r_hud = true;

    warninglevel = BETWEEN(warninglevel_min, warninglevel, warninglevel_max);

    weaponbob = BETWEEN(weaponbob_min, weaponbob, weaponbob_max);

    if (weaponbounce != false && weaponbounce != true)
        weaponbounce = weaponbounce_default;

    if (weaponrecoil != false && weaponrecoil != true)
        weaponrecoil = weaponrecoil_default;
}

//
// M_LoadCVARs
//
void M_LoadCVARs(char *filename)
{
    int     bindcount = 0;
    int     cvarcount = 0;
    int     statcount = 0;

    // read the file in, overriding any set defaults
    FILE    *file = fopen(filename, "r");

    if (!file)
    {
        M_CheckCVARs();
        M_SaveCVARs();
        C_Output("Created <b>%s</b>.", filename);
        cvarsloaded = true;
        return;
    }

    for (int i = 0; i < MAXALIASES; i++)
    {
        aliases[i].name[0] = '\0';
        aliases[i].string[0] = '\0';
    }

    // Clear all default controls before reading them from config file
    if (!togglingvanilla && M_StringEndsWith(filename, PACKAGE_CONFIG))
    {
        for (int i = 0; *actions[i].action; i++)
        {
            if (actions[i].keyboard1)
                *(int *)actions[i].keyboard1 = 0;

            if (actions[i].keyboard2)
                *(int *)actions[i].keyboard2 = 0;

            if (actions[i].mouse1)
                *(int *)actions[i].mouse1 = -1;

            if (actions[i].gamepad1)
                *(int *)actions[i].gamepad1 = 0;

            if (actions[i].gamepad2)
                *(int *)actions[i].gamepad2 = 0;
        }

        for (int i = 0; i < NUMKEYS; i++)
            keyactionlist[i][0] = '\0';

        for (int i = 0; i < MAX_MOUSE_BUTTONS + 2; i++)
            mouseactionlist[i][0] = '\0';
    }

    while (!feof(file))
    {
        char    cvar[64] = "";
        char    value[256] = "";

        if (fscanf(file, "%63s %255[^\n]\n", cvar, value) != 2)
            continue;

        if (cvar[0] == ';')
            continue;

        if (M_StringCompare(cvar, "bind"))
        {
            bind_cmd_func2("bind", value);
            bindcount++;
            continue;
        }
        else if (M_StringCompare(cvar, "alias"))
        {
            if (!togglingvanilla)
                alias_cmd_func2("alias", value);

            continue;
        }

        // Strip off trailing non-printable characters (\r characters from DOS text files)
        while (*value && !isprint((unsigned char)value[strlen(value) - 1]))
            value[strlen(value) - 1] = '\0';

        if (togglingvanilla)
        {
            char    *temp = uncommify(value);

            C_ValidateInput(M_StringJoin(cvar, " ", temp, NULL));
            free(temp);
            continue;
        }

        // Find the setting in the list
        for (int i = 0; i < arrlen(cvars); i++)
        {
            char    *s;

            if (!M_StringCompare(cvar, cvars[i].name) && !M_StringCompare(cvar, cvars[i].oldname))
                continue;       // not this one

            // parameter found
            switch (cvars[i].type)
            {
                case DEFAULT_STRING:
                    s = M_StringDuplicate(value + 1);
                    s[strlen(s) - 1] = '\0';
                    *(char **)cvars[i].location = s;
                    cvarcount++;
                    break;

                case DEFAULT_INT32:
                {
                    char    *temp = uncommify(value);

                    M_StringCopy(value, temp, sizeof(value));
                    *(int *)cvars[i].location = ParseIntParameter(value, cvars[i].valuealiastype);
                    free(temp);
                    cvarcount++;
                    break;
                }

                case DEFAULT_UINT64:
                {
                    char    *temp = uncommify(value);

                    M_StringCopy(value, temp, sizeof(value));
                    sscanf(value, "%10" PRIu64, (uint64_t *)cvars[i].location);
                    free(temp);
                    statcount++;
                    break;
                }

                case DEFAULT_INT32_PERCENT:
                {
                    char    *temp = uncommify(value);

                    M_StringCopy(value, temp, sizeof(value));

                    if (value[strlen(value) - 1] == '%')
                        value[strlen(value) - 1] = '\0';

                    *(int *)cvars[i].location = ParseIntParameter(value, cvars[i].valuealiastype);
                    free(temp);
                    cvarcount++;
                    break;
                }

                case DEFAULT_FLOAT:
                {
                    char    *temp = uncommify(value);

                    M_StringCopy(value, temp, sizeof(value));
                    *(float *)cvars[i].location = ParseFloatParameter(value, cvars[i].valuealiastype);
                    free(temp);
                    cvarcount++;
                    break;
                }

                case DEFAULT_FLOAT_PERCENT:
                {
                    char    *temp = uncommify(value);

                    M_StringCopy(value, temp, sizeof(value));

                    if (value[strlen(value) - 1] == '%')
                        value[strlen(value) - 1] = '\0';

                    *(float *)cvars[i].location = ParseFloatParameter(value, cvars[i].valuealiastype);
                    free(temp);
                    cvarcount++;
                    break;
                }

                case DEFAULT_OTHER:
                    *(char **)cvars[i].location = M_StringDuplicate(value);
                    cvarcount++;
                    break;
            }

            // finish
            break;
        }
    }

    fclose(file);

    if (!togglingvanilla)
    {
        char    *temp1 = commify(cvarcount);
        char    *temp2 = commify(statcount);
        char    *temp3 = commify(bindcount);

        C_Output("Loaded %s CVARs and %s player stats from <b>%s</b>.", temp1, temp2, filename);
        C_Output("Bound %s actions to the keyboard, mouse and gamepad.", temp3);
        M_CheckCVARs();
        cvarsloaded = true;

        free(temp1);
        free(temp2);
        free(temp3);
    }
}
