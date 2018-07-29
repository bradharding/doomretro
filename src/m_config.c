/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#include <ctype.h>
#include <string.h>

#include "c_cmds.h"
#include "c_console.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_fixed.h"
#include "m_misc.h"
#include "p_local.h"
#include "version.h"

static dboolean cvarsloaded;

extern char     *packageconfig;
extern dboolean returntowidescreen;
extern dboolean vanilla;
extern dboolean togglingvanilla;

void alias_cmd_func2(char *cmd, char *parms);
void bind_cmd_func2(char *cmd, char *parms);

#define CONFIG_VARIABLE_INT(name, set)              { #name, &name, DEFAULT_INT,           set          }
#define CONFIG_VARIABLE_INT_UNSIGNED(name, set)     { #name, &name, DEFAULT_INT_UNSIGNED,  set          }
#define CONFIG_VARIABLE_INT_PERCENT(name, set)      { #name, &name, DEFAULT_INT_PERCENT,   set          }
#define CONFIG_VARIABLE_FLOAT(name, set)            { #name, &name, DEFAULT_FLOAT,         set          }
#define CONFIG_VARIABLE_FLOAT_PERCENT(name, set)    { #name, &name, DEFAULT_FLOAT_PERCENT, set          }
#define CONFIG_VARIABLE_STRING(name, set)           { #name, &name, DEFAULT_STRING,        set          }
#define CONFIG_VARIABLE_OTHER(name, set)            { #name, &name, DEFAULT_OTHER,         set          }
#define BLANKLINE                                   { "",    "",    DEFAULT_OTHER,         NOVALUEALIAS }
#define COMMENT(text)                               { text,  "",    DEFAULT_OTHER,         NOVALUEALIAS }

static default_t cvars[] =
{
    COMMENT("; CVARs\n"),
    CONFIG_VARIABLE_INT          (alwaysrun,                                         BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (am_allmapcdwallcolor,                              NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_allmapfdwallcolor,                              NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_allmapwallcolor,                                NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_backcolor,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_cdwallcolor,                                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_crosshaircolor,                                 NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_external,                                       BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (am_fdwallcolor,                                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_grid,                                           BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (am_gridcolor,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_OTHER        (am_gridsize,                                       NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_markcolor,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_path,                                           BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (am_pathcolor,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_playercolor,                                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_rotatemode,                                     BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (am_teleportercolor,                                NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_thingcolor,                                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_tswallcolor,                                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (am_wallcolor,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (autoaim,                                           BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (autoload,                                          BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (autouse,                                           BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (centerweapon,                                      BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (con_backcolor,                                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (con_obituaries,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (con_timestamps,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (episode,                                           NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (expansion,                                         NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (facebackcolor,                                     FACEBACKVALUEALIAS),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_left,                                  NOVALUEALIAS      ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_right,                                 NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (gp_invertyaxis,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (gp_sensitivity,                                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (gp_swapthumbsticks,                                BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (gp_vibrate_barrels,                                NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_PERCENT  (gp_vibrate_damage,                                 NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_PERCENT  (gp_vibrate_weapons,                                NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (infighting,                                        BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (infiniteheight,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_STRING       (iwadfolder,                                        NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (m_doubleclick_use,                                 BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (m_invertyaxis,                                     BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (m_novertical,                                      BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (m_sensitivity,                                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (messages,                                          BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (mouselook,                                         BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (movebob,                                           NOVALUEALIAS      ),
    CONFIG_VARIABLE_STRING       (playername,                                        NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (r_althud,                                          BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_berserkintensity,                                NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (r_blood,                                           BLOODVALUEALIAS   ),
    CONFIG_VARIABLE_INT          (r_bloodsplats_max,                                 NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (r_bloodsplats_translucency,                        BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_brightmaps,                                      BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (r_color,                                           NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (r_corpses_color,                                   BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_corpses_mirrored,                                BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_corpses_moreblood,                               BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_corpses_nudge,                                   BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_corpses_slide,                                   BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_corpses_smearblood,                              BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_detail,                                          DETAILVALUEALIAS  ),
    CONFIG_VARIABLE_INT          (r_diskicon,                                        BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_dither,                                          BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_fixmaperrors,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_fixspriteoffsets,                                BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_floatbob,                                        BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_fov,                                             NOVALUEALIAS      ),
    CONFIG_VARIABLE_FLOAT        (r_gamma,                                           GAMMAVALUEALIAS   ),
    CONFIG_VARIABLE_INT          (r_homindicator,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_hud,                                             BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_hud_translucency,                                BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_liquid_bob,                                      BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_liquid_clipsprites,                              BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_liquid_current,                                  BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_liquid_lowerview,                                BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_liquid_swirl,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_OTHER        (r_lowpixelsize,                                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_OTHER        (r_messagepos,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (r_messagescale,                                    SCALEVALUEALIAS   ),
    CONFIG_VARIABLE_INT          (r_mirroredweapons,                                 BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_playersprites,                                   BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_rockettrails,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_screensize,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (r_shadows,                                         BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_shadows_translucency,                            BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_shake_barrels,                                   BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (r_shake_damage,                                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (r_skycolor,                                        SKYVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (r_textures,                                        BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (r_translucency,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (s_channels,                                        NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_PERCENT  (s_musicvolume,                                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (s_randommusic,                                     BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (s_randompitch,                                     BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (s_sfxvolume,                                       NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (s_stereo,                                          BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (savegame,                                          NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (skilllevel,                                        NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_PERCENT  (stillbob,                                          NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (tossdrop,                                          BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (turbo,                                             NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (units,                                             UNITSVALUEALIAS   ),
    CONFIG_VARIABLE_STRING       (version,                                           NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (vid_capfps,                                        CAPVALUEALIAS     ),
    CONFIG_VARIABLE_INT          (vid_display,                                       NOVALUEALIAS      ),
#if !defined(_WIN32)
    CONFIG_VARIABLE_STRING       (vid_driver,                                        NOVALUEALIAS      ),
#endif
    CONFIG_VARIABLE_INT          (vid_fullscreen,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (vid_motionblur,                                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (vid_pillarboxes,                                   BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_STRING       (vid_scaleapi,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_STRING       (vid_scalefilter,                                   NOVALUEALIAS      ),
    CONFIG_VARIABLE_OTHER        (vid_screenresolution,                              NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (vid_vsync,                                         BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (vid_widescreen,                                    BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_OTHER        (vid_windowpos,                                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_OTHER        (vid_windowsize,                                    NOVALUEALIAS      ),
#if defined(_WIN32)
    CONFIG_VARIABLE_STRING       (wad,                                               NOVALUEALIAS      ),
#endif
    CONFIG_VARIABLE_INT_PERCENT  (weaponbob,                                         NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT          (weaponrecoil,                                      BOOLVALUEALIAS    ),
    CONFIG_VARIABLE_INT          (wipe,                                              BOOLVALUEALIAS    ),
    BLANKLINE,
    COMMENT("; player stats\n"),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_barrelsexploded,                              NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_cheated,                                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_damageinflicted,                              NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_damagereceived,                               NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_deaths,                                       NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_distancetraveled,                             NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup,                                NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup_ammo_bullets,                   NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup_ammo_cells,                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup_ammo_rockets,                   NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup_ammo_shells,                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup_armor,                          NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup_health,                         NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_mapscompleted,                                NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled,                               NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_arachnotrons,                  NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_archviles,                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_baronsofhell,                  NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_cacodemons,                    NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_cyberdemons,                   NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_demons,                        NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_heavyweapondudes,              NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_hellknights,                   NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_imps,                          NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_lostsouls,                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_mancubi,                       NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_painelementals,                NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_revenants,                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_shotgunguys,                   NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_spectres,                      NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_spidermasterminds,             NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_zombiemen,                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_runs,                                         NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_secretsrevealed,                              NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_shotsfired,                                   NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_shotshit,                                     NOVALUEALIAS      ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_time,                                         NOVALUEALIAS      )
};

valuealias_t valuealiases[] =
{
    { "off",       0, BOOLVALUEALIAS     }, { "on",      1, BOOLVALUEALIAS     },
    { "0",         0, BOOLVALUEALIAS     }, { "1",       1, BOOLVALUEALIAS     },
    { "no",        0, BOOLVALUEALIAS     }, { "yes",     1, BOOLVALUEALIAS     },
    { "false",     0, BOOLVALUEALIAS     }, { "true",    1, BOOLVALUEALIAS     },
    { "low",       0, DETAILVALUEALIAS   }, { "high",    1, DETAILVALUEALIAS   },
    { "off",       1, GAMMAVALUEALIAS    }, { "none",    0, BLOODVALUEALIAS    },
    { "red",       1, BLOODVALUEALIAS    }, { "all",     2, BLOODVALUEALIAS    },
    { "imperial",  0, UNITSVALUEALIAS    }, { "metric",  1, UNITSVALUEALIAS    },
    { "off",       0, CAPVALUEALIAS      }, { "none",   -1, SKYVALUEALIAS      },
    { "off",      -1, SKYVALUEALIAS      }, { "small",   0, SCALEVALUEALIAS    },
    { "big",       1, SCALEVALUEALIAS    }, { "none",    5, FACEBACKVALUEALIAS },
    { "off",       5, FACEBACKVALUEALIAS }, { "",        0, NOVALUEALIAS       }
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
    int     p;
    FILE    *file;

    if (!cvarsloaded || vanilla)
        return;

    p = M_CheckParmWithArgs("-config", 1, 1);

    if (!(file = fopen((p ? myargv[p + 1] : packageconfig), "w")))
        return; // can't write the file, but don't complain

    if (returntowidescreen)
        vid_widescreen = true;

    for (int i = 0; i < arrlen(cvars); i++)
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
            case DEFAULT_INT:
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
                    fputs(commify(v), file);

                break;
            }

            case DEFAULT_INT_UNSIGNED:
                fputs(commify(*(unsigned int *)cvars[i].location), file);
                break;

            case DEFAULT_INT_PERCENT:
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
                    fprintf(file, "%s%%", commify(v));

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
                    static char buf[128];
                    int         len;

                    M_snprintf(buf, sizeof(buf), "%.2f", v);
                    len = (int)strlen(buf);

                    if (len >= 2 && buf[len - 1] == '0' && buf[len - 2] == '0')
                        buf[len - 1] = '\0';

                    fputs(buf, file);
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
                    fprintf(file, "%s%%", striptrailingzero(v, 1));

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
    int parm = 0;

    for (int i = 0; *valuealiases[i].text; i++)
        if (M_StringCompare(strparm, valuealiases[i].text) && valuealiastype == valuealiases[i].type)
            return valuealiases[i].value;

    sscanf(strparm, "%10i", &parm);
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

    if (autouse != false && autouse != true)
        autouse = autouse_default;

    if (centerweapon != false && centerweapon != true)
        centerweapon = centerweapon_default;

    if (con_backcolor < con_backcolor_min || con_backcolor > con_backcolor_max)
        con_backcolor = con_backcolor_default;

    if (con_obituaries != false && con_obituaries != true)
        con_obituaries = con_obituaries_default;

    if (con_timestamps != false && con_timestamps != true)
        con_timestamps = con_timestamps_default;

    episode = BETWEEN(episode_min, episode, episode_max);
    expansion = BETWEEN(expansion_min, expansion, expansion_max);

    if (facebackcolor < facebackcolor_min || facebackcolor > facebackcolor_max)
        facebackcolor = facebackcolor_default;

    gp_deadzone_left = BETWEENF(gp_deadzone_left_min, gp_deadzone_left, gp_deadzone_left_max);
    I_SetGamepadLeftDeadZone();
    gp_deadzone_right = BETWEENF(gp_deadzone_right_min, gp_deadzone_right, gp_deadzone_right_max);
    I_SetGamepadRightDeadZone();

    if (gp_invertyaxis != false && gp_invertyaxis != true)
        gp_invertyaxis = gp_invertyaxis_default;

    gp_sensitivity = BETWEEN(gp_sensitivity_min, gp_sensitivity, gp_sensitivity_max);
    I_SetGamepadSensitivity();

    if (gp_swapthumbsticks != false && gp_swapthumbsticks != true)
        gp_swapthumbsticks = gp_swapthumbsticks_default;

    gp_vibrate_barrels = BETWEEN(gp_vibrate_barrels_min, gp_vibrate_barrels, gp_vibrate_barrels_max);
    gp_vibrate_damage = BETWEEN(gp_vibrate_damage_min, gp_vibrate_damage, gp_vibrate_damage_max);
    gp_vibrate_weapons = BETWEEN(gp_vibrate_weapons_min, gp_vibrate_damage, gp_vibrate_weapons_max);

    if (infighting != false && infighting != true)
        infighting = infighting_default;

    if (infiniteheight != false && infiniteheight != true)
        infiniteheight = infiniteheight_default;

    if (m_doubleclick_use != false && m_doubleclick_use != true)
        m_doubleclick_use = m_doubleclick_use_default;

    if (m_invertyaxis != false && m_invertyaxis != true)
        m_invertyaxis = m_invertyaxis_default;

    if (m_novertical != false && m_novertical != true)
        m_novertical = m_novertical_default;

    m_sensitivity = BETWEEN(m_sensitivity_min, m_sensitivity, m_sensitivity_max);

    if (messages != false && messages != true)
        messages = messages_default;

    if (mouselook != false && mouselook != true)
        mouselook = mouselook_default;

    movebob = BETWEEN(movebob_min, movebob, movebob_max);

    if (!*playername)
        playername = strdup(playername_default);

    if (r_althud != false && r_althud != true)
        r_althud = r_althud_default;

    r_berserkintensity = BETWEEN(r_berserkintensity_min, r_berserkintensity, r_berserkintensity_max);

    if (r_blood != r_blood_none && r_blood != r_blood_red && r_blood != r_blood_all)
        r_blood = r_blood_default;

    r_bloodsplats_max = BETWEEN(r_bloodsplats_max_min, r_bloodsplats_max, r_bloodsplats_max_max);

    if (r_bloodsplats_translucency != false && r_bloodsplats_translucency != true)
        r_bloodsplats_translucency = r_bloodsplats_translucency_default;

    if (r_brightmaps != false && r_brightmaps != true)
        r_brightmaps = r_brightmaps_default;

    r_color = BETWEEN(r_color_min, r_color, r_color_max);

    if (r_corpses_color != false && r_corpses_color != true)
        r_corpses_color = r_corpses_color_default;

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

    if (r_fov < r_fov_min || r_fov > r_fov_max)
        r_fov = r_fov_default;

    r_gamma = BETWEENF(r_gamma_min, r_gamma, r_gamma_max);
    I_SetGamma(r_gamma);

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

    if (r_messagescale != r_messagescale_small && r_messagescale != r_messagescale_big)
        r_messagescale = r_mirroredweapons_default;

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

    vid_capfps = (vid_capfps < vid_capfps_min ? 0 : BETWEEN(vid_capfps_min, vid_capfps, vid_capfps_max));
    vid_display = MAX(vid_display_min, vid_display);

    if (vid_fullscreen != false && vid_fullscreen != true)
        vid_fullscreen = vid_fullscreen_default;

    vid_motionblur = BETWEEN(vid_motionblur_min, vid_motionblur, vid_motionblur_max);

    if (!M_StringCompare(vid_scaleapi, vid_scaleapi_direct3d)
#if defined(__MACOSX__)
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_metal)
#endif
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_opengl)
#if !defined(_WIN32)
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_opengles)
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_opengles2)
#endif
        && !M_StringCompare(vid_scaleapi, vid_scaleapi_software))
        vid_scaleapi = vid_scaleapi_default;

    if (!M_StringCompare(vid_scalefilter, vid_scalefilter_linear)
        && !M_StringCompare(vid_scalefilter, vid_scalefilter_nearest)
        && !M_StringCompare(vid_scalefilter, vid_scalefilter_nearest_linear))
        vid_scalefilter = vid_scalefilter_default;

    if (vid_vsync != false && vid_vsync != true)
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

    weaponbob = BETWEEN(weaponbob_min, weaponbob, weaponbob_max);

    if (weaponrecoil != false && weaponrecoil != true)
        weaponrecoil = weaponrecoil_default;

    if (wipe != false && wipe != true)
        wipe = wipe_default;
}

//
// M_LoadCVARs
//
void M_LoadCVARs(char *filename)
{
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
            if (!togglingvanilla)
                bind_cmd_func2("bind", value);

            continue;
        }
        else if (M_StringCompare(cvar, "alias"))
        {
            if (!togglingvanilla)
                alias_cmd_func2("alias", value);

            continue;
        }

        // Strip off trailing non-printable characters (\r characters from DOS text files)
        while (value[0] != '\0' && !isprint((unsigned char)value[strlen(value) - 1]))
            value[strlen(value) - 1] = '\0';

        if (togglingvanilla)
        {
            C_ValidateInput(M_StringJoin(cvar, " ", uncommify(value), NULL));
            continue;
        }

        // Find the setting in the list
        for (int i = 0; i < arrlen(cvars); i++)
        {
            char    *s;

            if (!M_StringCompare(cvar, cvars[i].name))
                continue;       // not this one

            if (M_StringStartsWith(cvar, "stat_"))
                statcount++;
            else
                cvarcount++;

            // parameter found
            switch (cvars[i].type)
            {
                case DEFAULT_STRING:
                    s = strdup(value + 1);
                    s[strlen(s) - 1] = '\0';
                    *(char **)cvars[i].location = s;
                    break;

                case DEFAULT_INT:
                    M_StringCopy(value, uncommify(value), sizeof(value));
                    *(int *)cvars[i].location = ParseIntParameter(value, cvars[i].valuealiastype);
                    break;

                case DEFAULT_INT_UNSIGNED:
                    M_StringCopy(value, uncommify(value), sizeof(value));
                    sscanf(value, "%10u", (unsigned int *)cvars[i].location);
                    break;

                case DEFAULT_INT_PERCENT:
                    M_StringCopy(value, uncommify(value), sizeof(value));
                    s = strdup(value);

                    if (s[0] != '\0' && s[strlen(s) - 1] == '%')
                        s[strlen(s) - 1] = '\0';

                    *(int *)cvars[i].location = ParseIntParameter(s, cvars[i].valuealiastype);
                    break;

                case DEFAULT_FLOAT:
                    M_StringCopy(value, uncommify(value), sizeof(value));
                    *(float *)cvars[i].location = ParseFloatParameter(value, cvars[i].valuealiastype);
                    break;

                case DEFAULT_FLOAT_PERCENT:
                    M_StringCopy(value, uncommify(value), sizeof(value));
                    s = strdup(value);

                    if (s[0] != '\0' && s[strlen(s) - 1] == '%')
                        s[strlen(s) - 1] = '\0';

                    *(float *)cvars[i].location = ParseFloatParameter(s, cvars[i].valuealiastype);
                    break;

                case DEFAULT_OTHER:
                    *(char **)cvars[i].location = strdup(value);
                    break;
            }

            // finish
            break;
        }
    }

    fclose(file);

    if (!togglingvanilla)
    {
        C_Output("Loaded %i CVARs and %i player stats from <b>%s</b>.", cvarcount, statcount, filename);
        M_CheckCVARs();
        cvarsloaded = true;
    }
}
