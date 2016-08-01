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

#include <ctype.h>
#include <string.h>

#include "c_cmds.h"
#include "c_console.h"
#include "doomstat.h"
#include "i_gamepad.h"
#include "m_misc.h"
#include "p_local.h"
#include "version.h"

extern dboolean         alwaysrun;
extern int              am_allmapcdwallcolor;
extern int              am_allmapfdwallcolor;
extern int              am_allmapwallcolor;
extern int              am_backcolor;
extern int              am_cdwallcolor;
extern dboolean         am_external;
extern int              am_fdwallcolor;
extern dboolean         am_grid;
extern int              am_gridcolor;
extern int              am_markcolor;
extern int              am_playercolor;
extern dboolean         am_rotatemode;
extern int              am_teleportercolor;
extern int              am_thingcolor;
extern int              am_tswallcolor;
extern int              am_wallcolor;
extern int              am_xhaircolor;
extern dboolean         autoload;
extern dboolean         centerweapon;
extern dboolean         con_obituaries;
extern dboolean         con_timestamps;
extern int              episode;
extern int              expansion;
extern int              facebackcolor;
extern float            gp_deadzone_left;
extern float            gp_deadzone_right;
extern int              gp_sensitivity;
extern dboolean         gp_swapthumbsticks;
extern dboolean         gp_vibrate;
extern char             *iwadfolder;
extern dboolean         messages;
extern float            m_acceleration;
extern dboolean         m_doubleclick_use;
extern dboolean         m_novertical;
extern int              m_sensitivity;
extern int              m_threshold;
extern int              movebob;
extern char             *playername;
extern dboolean         r_althud;
extern int              r_berserkintensity;
extern int              r_blood;
extern int              r_bloodsplats_max;
extern dboolean         r_brightmaps;
extern dboolean         r_corpses_color;
extern dboolean         r_corpses_mirrored;
extern dboolean         r_corpses_moreblood;
extern dboolean         r_corpses_nudge;
extern dboolean         r_corpses_slide;
extern dboolean         r_corpses_smearblood;
extern int              r_detail;
extern dboolean         r_diskicon;
extern dboolean         r_fixmaperrors;
extern dboolean         r_fixspriteoffsets;
extern dboolean         r_floatbob;
extern float            r_gamma;
extern dboolean         r_homindicator;
extern dboolean         r_hud;
extern dboolean         r_liquid_bob;
extern dboolean         r_liquid_clipsprites;
extern dboolean         r_liquid_current;
extern dboolean         r_liquid_lowerview;
extern dboolean         r_liquid_swirl;
extern char             *r_lowpixelsize;
extern dboolean         r_mirroredweapons;
extern dboolean         r_playersprites;
extern dboolean         r_rockettrails;
extern dboolean         r_shadows;
extern int              r_shakescreen;
extern dboolean         r_translucency;
extern int              s_musicvolume;
extern dboolean         s_randommusic;
extern dboolean         s_randompitch;
extern int              s_sfxvolume;
extern char             *s_timiditycfgpath;
extern int              savegame;
extern int              skilllevel;
extern int              stillbob;
extern unsigned int     stat_cheated;
extern unsigned int     stat_damageinflicted;
extern unsigned int     stat_damagereceived;
extern unsigned int     stat_deaths;
extern unsigned int     stat_distancetravelled;
extern unsigned int     stat_itemspickedup;
extern unsigned int     stat_monsterskilled;
extern unsigned int     stat_monsterskilled_arachnotrons;
extern unsigned int     stat_monsterskilled_archviles;
extern unsigned int     stat_monsterskilled_baronsofhell;
extern unsigned int     stat_monsterskilled_cacodemons;
extern unsigned int     stat_monsterskilled_cyberdemons;
extern unsigned int     stat_monsterskilled_demons;
extern unsigned int     stat_monsterskilled_heavyweapondudes;
extern unsigned int     stat_monsterskilled_hellknights;
extern unsigned int     stat_monsterskilled_imps;
extern unsigned int     stat_monsterskilled_lostsouls;
extern unsigned int     stat_monsterskilled_mancubi;
extern unsigned int     stat_monsterskilled_painelementals;
extern unsigned int     stat_monsterskilled_revenants;
extern unsigned int     stat_monsterskilled_shotgunguys;
extern unsigned int     stat_monsterskilled_spectres;
extern unsigned int     stat_monsterskilled_spidermasterminds;
extern unsigned int     stat_monsterskilled_zombiemen;
extern unsigned int     stat_runs;
extern unsigned int     stat_secretsrevealed;
extern unsigned int     stat_shotsfired;
extern unsigned int     stat_shotshit;
extern unsigned int     stat_time;
extern int              turbo;
extern dboolean         vid_capfps;
extern int              vid_display;
#if !defined(WIN32)
extern char             *vid_driver;
#endif
extern dboolean         vid_fullscreen;
extern char             *vid_scaledriver;
extern char             *vid_scalefilter;
extern char             *vid_screenresolution;
extern dboolean         vid_vsync;
extern dboolean         vid_widescreen;
extern char             *vid_windowposition;
extern char             *vid_windowsize;
extern int              weaponbob;

extern char             *packageconfig;
extern dboolean         returntowidescreen;

#define CONFIG_VARIABLE_INT(name, set) \
    { #name,          &name, DEFAULT_INT,           set     }
#define CONFIG_VARIABLE_INT_UNSIGNED(name, set) \
    { #name,          &name, DEFAULT_INT_UNSIGNED,  set     }
#define CONFIG_VARIABLE_INT_PERCENT(name, set) \
    { #name,          &name, DEFAULT_INT_PERCENT,   set     }
#define CONFIG_VARIABLE_FLOAT(name, set) \
    { #name,          &name, DEFAULT_FLOAT,         set     }
#define CONFIG_VARIABLE_FLOAT_PERCENT(name, set) \
    { #name,          &name, DEFAULT_FLOAT_PERCENT, set     }
#define CONFIG_VARIABLE_STRING(name, set) \
    { #name,          &name, DEFAULT_STRING,        set     }
#define CONFIG_VARIABLE_OTHER(name, set) \
    { #name,          &name, DEFAULT_OTHER,         set     }
#define BLANKLINE \
    { "",             "",    DEFAULT_OTHER,         NOALIAS }
#define COMMENT(text) \
    { "; "##text"\n", "",    DEFAULT_OTHER,         NOALIAS }

static default_t cvars[] =
{
    COMMENT("console variables"),
    CONFIG_VARIABLE_INT          (alwaysrun,                                         BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (am_allmapcdwallcolor,                              NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_allmapfdwallcolor,                              NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_allmapwallcolor,                                NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_backcolor,                                      NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_cdwallcolor,                                    NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_external,                                       BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (am_fdwallcolor,                                    NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_grid,                                           BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (am_gridcolor,                                      NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_markcolor,                                      NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_playercolor,                                    NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_rotatemode,                                     BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (am_teleportercolor,                                NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_thingcolor,                                     NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_tswallcolor,                                    NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_wallcolor,                                      NOALIAS    ),
    CONFIG_VARIABLE_INT          (am_xhaircolor,                                     NOALIAS    ),
    CONFIG_VARIABLE_INT          (autoload,                                          BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (centerweapon,                                      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (con_obituaries,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (con_timestamps,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (episode,                                           NOALIAS    ),
    CONFIG_VARIABLE_INT          (expansion,                                         NOALIAS    ),
    CONFIG_VARIABLE_INT          (facebackcolor,                                     NOALIAS    ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_left,                                  NOALIAS    ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_right,                                 NOALIAS    ),
    CONFIG_VARIABLE_INT          (gp_sensitivity,                                    NOALIAS    ),
    CONFIG_VARIABLE_INT          (gp_swapthumbsticks,                                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (gp_vibrate,                                        BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (iwadfolder,                                        NOALIAS    ),
    CONFIG_VARIABLE_FLOAT        (m_acceleration,                                    NOALIAS    ),
    CONFIG_VARIABLE_INT          (m_doubleclick_use,                                 BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (m_novertical,                                      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (m_sensitivity,                                     NOALIAS    ),
    CONFIG_VARIABLE_INT          (m_threshold,                                       NOALIAS    ),
    CONFIG_VARIABLE_INT          (messages,                                          BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (movebob,                                           NOALIAS    ),
    CONFIG_VARIABLE_STRING       (playername,                                        NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_althud,                                          BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_berserkintensity,                                NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_blood,                                           BLOODALIAS ),
    CONFIG_VARIABLE_INT          (r_bloodsplats_max,                                 NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_brightmaps,                                      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_color,                                   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_mirrored,                                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_moreblood,                               BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_nudge,                                   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_slide,                                   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_smearblood,                              BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_detail,                                          DETAILALIAS),
    CONFIG_VARIABLE_INT          (r_diskicon,                                        BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_fixmaperrors,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_fixspriteoffsets,                                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_floatbob,                                        BOOLALIAS  ),
    CONFIG_VARIABLE_FLOAT        (r_gamma,                                           GAMMAALIAS ),
    CONFIG_VARIABLE_INT          (r_homindicator,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_hud,                                             BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_bob,                                      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_clipsprites,                              BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_current,                                  BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_lowerview,                                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_swirl,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_OTHER        (r_lowpixelsize,                                    NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_mirroredweapons,                                 BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_playersprites,                                   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_rockettrails,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_screensize,                                      NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_shadows,                                         BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (r_shakescreen,                                     NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_translucency,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (s_musicvolume,                                     NOALIAS    ),
    CONFIG_VARIABLE_INT          (s_randommusic,                                     BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (s_randompitch,                                     BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (s_sfxvolume,                                       NOALIAS    ),
    CONFIG_VARIABLE_STRING       (s_timiditycfgpath,                                 NOALIAS    ),
    CONFIG_VARIABLE_INT          (savegame,                                          NOALIAS    ),
    CONFIG_VARIABLE_INT          (skilllevel,                                        NOALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (stillbob,                                          NOALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (turbo,                                             NOALIAS    ),
    CONFIG_VARIABLE_INT          (vid_capfps,                                        BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (vid_display,                                       NOALIAS    ),
#if !defined(WIN32)
    CONFIG_VARIABLE_STRING       (vid_driver,                                        NOALIAS    ),
#endif
    CONFIG_VARIABLE_INT          (vid_fullscreen,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (vid_scaledriver,                                   NOALIAS    ),
    CONFIG_VARIABLE_STRING       (vid_scalefilter,                                   NOALIAS    ),
    CONFIG_VARIABLE_OTHER        (vid_screenresolution,                              NOALIAS    ),
    CONFIG_VARIABLE_INT          (vid_vsync,                                         BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (vid_widescreen,                                    BOOLALIAS  ),
    CONFIG_VARIABLE_OTHER        (vid_windowposition,                                NOALIAS    ),
    CONFIG_VARIABLE_OTHER        (vid_windowsize,                                    NOALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (weaponbob,                                         NOALIAS    ),
    BLANKLINE,
    COMMENT("player statistics"),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_cheated,                                      NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_damageinflicted,                              NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_damagereceived,                               NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_deaths,                                       NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_distancetravelled,                            NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup,                                NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled,                               NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_arachnotrons,                  NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_archviles,                     NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_baronsofhell,                  NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_cacodemons,                    NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_heavyweapondudes,              NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_cyberdemons,                   NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_demons,                        NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_hellknights,                   NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_imps,                          NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_lostsouls,                     NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_mancubi,                       NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_painelementals,                NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_revenants,                     NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_shotgunguys,                   NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_spectres,                      NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_spidermasterminds,             NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled_zombiemen,                     NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_runs,                                         NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_secretsrevealed,                              NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_shotsfired,                                   NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_shotshit,                                     NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_time,                                         NOALIAS    )
};

alias_t aliases[] =
{
    { "off",   0, BOOLALIAS   }, { "on",    1, BOOLALIAS   }, { "0",     0, BOOLALIAS   },
    { "1",     1, BOOLALIAS   }, { "no",    0, BOOLALIAS   }, { "yes",   1, BOOLALIAS   },
    { "false", 0, BOOLALIAS   }, { "true",  1, BOOLALIAS   }, { "low",   0, DETAILALIAS },
    { "high",  1, DETAILALIAS }, { "off",   1, GAMMAALIAS  }, { "none",  0, BLOODALIAS  },
    { "red",   1, BLOODALIAS  }, { "all",   2, BLOODALIAS  }, { "",      0, NOALIAS     }
};

static void SaveBind(FILE *file, char *action, int value, controltype_t type)
{
    int i = 0;

    while (controls[i].type)
    {
        if (controls[i].type == type && controls[i].value == value)
        {
            char        *control = controls[i].control;

            if (strlen(control) == 1)
                fprintf(file, "bind '%s' %s\n", (control[0] == '=' ? "+" : control), action);
            else
                fprintf(file, "bind %s %s\n", control, action);
            break;
        }
        ++i;
    }
}

//
// M_SaveCVARs
//
void M_SaveCVARs(void)
{
    int         i;
    FILE        *file = fopen(packageconfig, "w");

    if (!file)
        return; // can't write the file, but don't complain

    if (returntowidescreen)
        vid_widescreen = true;

    for (i = 0; i < arrlen(cvars); i++)
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
                int             j = 0;
                dboolean        flag = false;
                int             v = *(int *)cvars[i].location;

                while (*aliases[j].text)
                {
                    if (v == aliases[j].value && cvars[i].aliastype == aliases[j].type)
                    {
                        fputs(aliases[j].text, file);
                        flag = true;
                        break;
                    }
                    j++;
                }
                if (!flag)
                    fprintf(file, "%i", *(int *)cvars[i].location);
                break;
            }

            case DEFAULT_INT_UNSIGNED:
            {
                fprintf(file, "%u", *(unsigned int *)cvars[i].location);
                break;
            }

            case DEFAULT_INT_PERCENT:
            {
                int             j = 0;
                dboolean        flag = false;
                int             v = *(int *)cvars[i].location;

                while (*aliases[j].text)
                {
                    if (v == aliases[j].value && cvars[i].aliastype == aliases[j].type)
                    {
                        fputs(aliases[j].text, file);
                        flag = true;
                        break;
                    }
                    j++;
                }
                if (!flag)
                    fprintf(file, "%i%%", *(int *)cvars[i].location);
                break;
            }

            case DEFAULT_FLOAT:
            {
                int         j = 0;
                dboolean    flag = false;
                float       v = *(float *)cvars[i].location;

                while (*aliases[j].text)
                {
                    if (v == aliases[j].value && cvars[i].aliastype == aliases[j].type)
                    {
                        fputs(aliases[j].text, file);
                        flag = true;
                        break;
                    }
                    j++;
                }
                if (!flag)
                    fprintf(file, "%s", striptrailingzero(*(float *)cvars[i].location, 2));
                break;
            }

            case DEFAULT_FLOAT_PERCENT:
            {
                int         j = 0;
                dboolean    flag = false;
                float       v = *(float *)cvars[i].location;

                while (*aliases[j].text)
                {
                    if (v == aliases[j].value && cvars[i].aliastype == aliases[j].type)
                    {
                        fputs(aliases[j].text, file);
                        flag = true;
                        break;
                    }
                    j++;
                }
                if (!flag)
                    fprintf(file, "%s%%", striptrailingzero(*(float *)cvars[i].location, 1));
                break;
            }

            case DEFAULT_STRING:
                fprintf(file, "\"%s\"", *(char **)cvars[i].location);
                break;

            case DEFAULT_OTHER:
                fputs(*(char **)cvars[i].location, file);
                break;
        }

        fputs("\n", file);
    }

    fputs("\n", file);

    fputs("; bound controls\n", file);

    i = 0;
    while (*actions[i].action)
    {
        if (actions[i].keyboard)
            SaveBind(file, actions[i].action, *(int *)actions[i].keyboard, keyboardcontrol);
        if (actions[i].mouse)
            SaveBind(file, actions[i].action, *(int *)actions[i].mouse, mousecontrol);
        if (actions[i].gamepad)
            SaveBind(file, actions[i].action, *(int *)actions[i].gamepad, gamepadcontrol);
        ++i;
    }

    fclose(file);

    if (returntowidescreen)
        vid_widescreen = false;
}

// Parses integer values in the configuration file
static int ParseIntParameter(char *strparm, int aliastype)
{
    int parm = 0;
    int i = 0;

    while (*aliases[i].text)
    {
        if (M_StringCompare(strparm, aliases[i].text) && aliastype == aliases[i].type)
            return aliases[i].value;
        ++i;
    }

    sscanf(strparm, "%10i", &parm);

    return parm;
}

// Parses float values in the configuration file
static float ParseFloatParameter(char *strparm, int aliastype)
{
    int i = 0;

    while (*aliases[i].text)
    {
        if (M_StringCompare(strparm, aliases[i].text) && aliastype == aliases[i].type)
            return (float)aliases[i].value;
        ++i;
    }

    return (float)atof(strparm);
}

void C_Bind(char *cmd, char *parm1, char *parm2, char *parm3);

static void M_CheckCVARs(void)
{
    if (alwaysrun != false && alwaysrun != true)
        alwaysrun = alwaysrun_default;

    if (am_allmapcdwallcolor < am_allmapcdwallcolor_min
        || am_allmapcdwallcolor > am_allmapcdwallcolor_max)
        am_allmapcdwallcolor = am_allmapcdwallcolor_default;

    if (am_allmapfdwallcolor < am_allmapfdwallcolor_min
        || am_allmapfdwallcolor > am_allmapfdwallcolor_max)
        am_allmapfdwallcolor = am_allmapfdwallcolor_default;

    if (am_allmapwallcolor < am_allmapwallcolor_min || am_allmapwallcolor > am_allmapwallcolor_max)
        am_allmapwallcolor = am_allmapwallcolor_default;

    if (am_backcolor < am_backcolor_min || am_backcolor > am_backcolor_max)
        am_backcolor = am_backcolor_default;

    if (am_cdwallcolor < am_cdwallcolor_min || am_cdwallcolor > am_cdwallcolor_max)
        am_cdwallcolor = am_cdwallcolor_default;

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

    if (am_xhaircolor < am_xhaircolor_min || am_xhaircolor > am_xhaircolor_max)
        am_xhaircolor = am_xhaircolor_default;

    if (autoload != false && autoload != true)
        autoload = autoload_default;

    if (centerweapon != false && centerweapon != true)
        centerweapon = centerweapon_default;

    if (con_obituaries != false && con_obituaries != true)
        con_obituaries = con_obituaries_default;

    if (con_timestamps != false && con_timestamps != true)
        con_timestamps = con_timestamps_default;

    episode = BETWEEN(episode_min, episode, episode_max - (gamemode == registered));

    expansion = BETWEEN(expansion_min, expansion, expansion_max);

    if (facebackcolor < facebackcolor_min || facebackcolor > facebackcolor_max)
        facebackcolor = facebackcolor_default;

    gp_deadzone_left = BETWEENF(gp_deadzone_left_min, gp_deadzone_left, gp_deadzone_left_max);
    gamepadleftdeadzone = (short)(gp_deadzone_left * (float)SHRT_MAX / 100.0f);

    gp_deadzone_right = BETWEENF(gp_deadzone_right_min, gp_deadzone_right, gp_deadzone_right_max);
    gamepadrightdeadzone = (short)(gp_deadzone_right * (float)SHRT_MAX / 100.0f);

    gp_sensitivity = BETWEEN(gp_sensitivity_min, gp_sensitivity, gp_sensitivity_max);
    I_SetGamepadSensitivity(gp_sensitivity);

    if (gp_swapthumbsticks != false && gp_swapthumbsticks != true)
        gp_swapthumbsticks = gp_swapthumbsticks_default;

    if (gp_vibrate != false && gp_vibrate != true)
        gp_vibrate = gp_vibrate_default;

    if (m_doubleclick_use != false && m_doubleclick_use != true)
        m_doubleclick_use = m_doubleclick_use_default;

    if (m_novertical != false && m_novertical != true)
        m_novertical = m_novertical_default;

    m_sensitivity = BETWEEN(m_sensitivity_min, m_sensitivity, m_sensitivity_max);

    if (messages != false && messages != true)
        messages = messages_default;

    movebob = BETWEEN(movebob_min, movebob, movebob_max);

    if (!*playername)
        playername = strdup(playername_default);

    if (r_althud != false && r_althud != true)
        r_althud = r_althud_default;

    if (r_berserkintensity < r_berserkintensity_min || r_berserkintensity > r_berserkintensity_max)
        r_berserkintensity = r_berserkintensity_default;

    if (r_blood != r_blood_none && r_blood != r_blood_red && r_blood != r_blood_all)
        r_blood = r_blood_default;

    r_bloodsplats_max = BETWEEN(r_bloodsplats_max_min, r_bloodsplats_max, r_bloodsplats_max_max);

    if (r_brightmaps != false && r_brightmaps != true)
        r_brightmaps = r_brightmaps_default;

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

    if (r_fixmaperrors != false && r_fixmaperrors != true)
        r_fixmaperrors = r_fixmaperrors_default;

    if (r_fixspriteoffsets != false && r_fixspriteoffsets != true)
        r_fixspriteoffsets = r_fixspriteoffsets_default;

    if (r_floatbob != false && r_floatbob != true)
        r_floatbob = r_floatbob_default;

    r_gamma = BETWEENF(r_gamma_min, r_gamma, r_gamma_max);
    I_SetGamma(r_gamma);

    if (r_homindicator != false && r_homindicator != true)
        r_homindicator = r_homindicator_default;

    if (r_hud != false && r_hud != true)
        r_hud = r_hud_default;

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

    r_shakescreen = BETWEEN(r_shakescreen_min, r_shakescreen, r_shakescreen_max);

    if (r_translucency != false && r_translucency != true)
        r_translucency = r_translucency_default;

    s_musicvolume = BETWEEN(s_musicvolume_min, s_musicvolume, s_musicvolume_max);
    musicVolume = (s_musicvolume * 15 + 50) / 100;

    if (s_randommusic != false && s_randommusic != true)
        s_randommusic = s_randommusic_default;

    if (s_randompitch != false && s_randompitch != true)
        s_randompitch = s_randompitch_default;

    s_sfxvolume = BETWEEN(s_sfxvolume_min, s_sfxvolume, s_sfxvolume_max);
    sfxVolume = (s_sfxvolume * 15 + 50) / 100;

    savegame = BETWEEN(savegame_min, savegame, savegame_max);

    skilllevel = BETWEEN(skilllevel_min, skilllevel, skilllevel_max);

    stillbob = BETWEEN(stillbob_min, stillbob, stillbob_max);

    turbo = BETWEEN(turbo_min, turbo, turbo_max);

    if (vid_capfps != false && vid_capfps != true)
        vid_capfps = vid_capfps_default;

    if (vid_fullscreen != false && vid_fullscreen != true)
        vid_fullscreen = vid_fullscreen_default;

    if (!M_StringCompare(vid_scaledriver, vid_scaledriver_direct3d)
        && !M_StringCompare(vid_scaledriver, vid_scaledriver_opengl)
        && !M_StringCompare(vid_scaledriver, vid_scaledriver_software))
        vid_scaledriver = vid_scaledriver_default;

    if (!M_StringCompare(vid_scalefilter, vid_scalefilter_linear)
        && !M_StringCompare(vid_scalefilter, vid_scalefilter_nearest)
        && !M_StringCompare(vid_scalefilter, vid_scalefilter_nearest_linear))
        vid_scalefilter = vid_scalefilter_default;

    if (vid_vsync != false && vid_vsync != true)
        vid_vsync = vid_vsync_default;

    if (vid_widescreen != false && vid_widescreen != true)
        vid_widescreen = vid_widescreen_default;
    if (vid_widescreen || r_screensize == r_screensize_max)
    {
        returntowidescreen = true;
        vid_widescreen = false;
    }
    else
        r_hud = true;

    weaponbob = BETWEEN(weaponbob_min, weaponbob, weaponbob_max);

    M_SaveCVARs();
}

//
// M_LoadCVARs
//
void M_LoadCVARs(char *filename)
{
    int         i;
    FILE        *file;
    char        control[32];
    char        action[32];
    char        defname[32] = "";
    char        strparm[256] = "";

    // read the file in, overriding any set defaults
    file = fopen(filename, "r");

    if (!file)
    {
        if (stat_runs)
            C_Output("<b>%s</b> wasn't found. Using the defaults for all CVARs and recreating "
                "<b>%s</b>.", filename, PACKAGE_CONFIG);
        else
            C_Output("Created <b>%s</b>.", filename);
        M_CheckCVARs();
        return;
    }

    while (!feof(file))
    {
        if (fscanf(file, "bind %31s %31[^\n]\n", control, action) == 2)
        {
            C_StripQuotes(control);
            C_StripQuotes(action);
            C_Bind("", control, action, "");
            continue;
        }
        else if (fscanf(file, "%31s %255[^\n]\n", defname, strparm) != 2)
            // This line doesn't match
            continue;

        if (defname[0] == ';')
            continue;

        // Strip off trailing non-printable characters (\r characters from DOS text files)
        while (strlen(strparm) > 0 && !isprint((unsigned char)strparm[strlen(strparm) - 1]))
            strparm[strlen(strparm) - 1] = '\0';

        // Find the setting in the list
        for (i = 0; i < arrlen(cvars); ++i)
        {
            char        *s;

            if (!M_StringCompare(defname, cvars[i].name))
                continue;       // not this one

            // parameter found
            switch (cvars[i].type)
            {
                case DEFAULT_STRING:
                    s = strdup(strparm + 1);
                    s[strlen(s) - 1] = '\0';
                    *(char **)cvars[i].location = s;
                    break;

                case DEFAULT_INT:
                    *(int *)cvars[i].location = ParseIntParameter(strparm, cvars[i].aliastype);
                    break;

                case DEFAULT_INT_UNSIGNED:
                    sscanf(strparm, "%10u", (unsigned int *)cvars[i].location);
                    break;

                case DEFAULT_INT_PERCENT:
                    s = strdup(strparm);
                    if (strlen(s) >= 1 && s[strlen(s) - 1] == '%')
                        s[strlen(s) - 1] = '\0';
                    *(int *)cvars[i].location = ParseIntParameter(s, cvars[i].aliastype);
                    break;

                case DEFAULT_FLOAT:
                    *(float *)cvars[i].location = ParseFloatParameter(strparm, cvars[i].aliastype);
                    break;

                case DEFAULT_FLOAT_PERCENT:
                    s = strdup(strparm);
                    if (strlen(s) >= 1 && s[strlen(s) - 1] == '%')
                        s[strlen(s) - 1] = '\0';
                    *(float *)cvars[i].location = ParseFloatParameter(s, cvars[i].aliastype);
                    break;

                case DEFAULT_OTHER:
                    *(char **)cvars[i].location = strdup(strparm);
                    break;
            }

            // finish
            break;
        }
    }

    fclose(file);

    C_Output("Loaded CVARs from <b>%s</b>.", filename);
    M_CheckCVARs();
}
