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

#if defined(WIN32)
#pragma warning( disable : 4091 )
#include <ShlObj.h>
#include <Xinput.h>
#endif

#include "c_cmds.h"
#include "c_console.h"
#include "doomstat.h"
#include "i_gamepad.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "version.h"

char                    *configfile = PACKAGE_CONFIG;

extern dboolean         am_grid;
extern dboolean         am_rotatemode;
extern dboolean         con_timestamps;
extern int              episode;
extern int              expansion;
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
extern char             *playername;
extern dboolean         pm_alwaysrun;
extern dboolean         pm_centerweapon;
extern int              pm_walkbob;
extern dboolean         r_altlighting;
extern int              r_blood;
extern int              r_bloodsplats_max;
extern dboolean         r_brightmaps;
extern dboolean         r_corpses_mirrored;
extern dboolean         r_corpses_moreblood;
extern dboolean         r_corpses_nudge;
extern dboolean         r_corpses_slide;
extern dboolean         r_corpses_smearblood;
extern int              r_detail;
extern dboolean         r_fixmaperrors;
extern dboolean         r_fixspriteoffsets;
extern dboolean         r_floatbob;
extern float            r_gamma;
extern dboolean         r_homindicator;
extern dboolean         r_hud;
extern dboolean         r_liquid_bob;
extern dboolean         r_liquid_clipsprites;
extern dboolean         r_liquid_lowerview;
extern dboolean         r_liquid_swirl;
extern char             *r_lowpixelsize;
extern dboolean         r_mirroredweapons;
extern dboolean         r_playersprites;
extern dboolean         r_rockettrails;
extern dboolean         r_shadows;
extern dboolean         r_translucency;
extern int              runcount;
extern int              s_musicvolume;
extern dboolean         s_randompitch;
extern int              s_sfxvolume;
extern char             *s_timiditycfgpath;
extern int              savegame;
extern int              skilllevel;
extern unsigned int     stat_cheated;
extern unsigned int     stat_damageinflicted;
extern unsigned int     stat_damagereceived;
extern unsigned int     stat_deaths;
extern unsigned int     stat_itemspickedup;
extern unsigned int     stat_monsterskilled;
extern unsigned int     stat_secretsrevealed;
extern unsigned int     stat_time;
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

extern int              gamepadleftdeadzone;
extern int              gamepadrightdeadzone;
extern int              pixelwidth;
extern int              pixelheight;
extern dboolean         returntowidescreen;

#define CONFIG_VARIABLE_GENERIC(name, type, set) \
    { #name, &name, type, set }

#define CONFIG_VARIABLE_INT(name, set) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_INT, set)
#define CONFIG_VARIABLE_INT_UNSIGNED(name, set) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_INT_UNSIGNED, set)
#define CONFIG_VARIABLE_INT_PERCENT(name, set) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_INT_PERCENT, set)
#define CONFIG_VARIABLE_FLOAT(name, set) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_FLOAT, set)
#define CONFIG_VARIABLE_FLOAT_PERCENT(name, set) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_FLOAT_PERCENT, set)
#define CONFIG_VARIABLE_STRING(name, set) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_STRING, set)
#define CONFIG_VARIABLE_OTHER(name, set) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_OTHER, set)

static default_t cvars[] =
{
    CONFIG_VARIABLE_INT          (am_grid,              BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (am_rotatemode,        BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (con_timestamps,       BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (episode,              NOALIAS    ),
    CONFIG_VARIABLE_INT          (expansion,            NOALIAS    ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_left,     NOALIAS    ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_right,    NOALIAS    ),
    CONFIG_VARIABLE_INT          (gp_sensitivity,       NOALIAS    ),
    CONFIG_VARIABLE_INT          (gp_swapthumbsticks,   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (gp_vibrate,           BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (iwadfolder,           NOALIAS    ),
    CONFIG_VARIABLE_FLOAT        (m_acceleration,       NOALIAS    ),
    CONFIG_VARIABLE_INT          (m_doubleclick_use,    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (m_novertical,         BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (m_sensitivity,        NOALIAS    ),
    CONFIG_VARIABLE_INT          (m_threshold,          NOALIAS    ),
    CONFIG_VARIABLE_INT          (messages,             BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (playername,           NOALIAS    ),
    CONFIG_VARIABLE_INT          (pm_alwaysrun,         BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (pm_centerweapon,      BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (pm_walkbob,           NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_altlighting,        BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_blood,              BLOODALIAS ),
    CONFIG_VARIABLE_INT          (r_bloodsplats_max,    SPLATALIAS ),
    CONFIG_VARIABLE_INT          (r_brightmaps,         BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_mirrored,   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_moreblood,  BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_nudge,      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_slide,      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_smearblood, BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_detail,             DETAILALIAS),
    CONFIG_VARIABLE_INT          (r_fixmaperrors,       BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_fixspriteoffsets,   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_floatbob,           BOOLALIAS  ),
    CONFIG_VARIABLE_FLOAT        (r_gamma,              GAMMAALIAS ),
    CONFIG_VARIABLE_INT          (r_homindicator,       BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_hud,                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_bob,         BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_clipsprites, BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_lowerview,   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_swirl,       BOOLALIAS  ),
    CONFIG_VARIABLE_OTHER        (r_lowpixelsize,       NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_mirroredweapons,    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_playersprites,      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_rockettrails,       BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_screensize,         NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_shadows,            BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_translucency,       BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (runcount,             NOALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (s_musicvolume,        NOALIAS    ),
    CONFIG_VARIABLE_INT          (s_randompitch,        BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (s_sfxvolume,          NOALIAS    ),
    CONFIG_VARIABLE_STRING       (s_timiditycfgpath,    NOALIAS    ),
    CONFIG_VARIABLE_INT          (savegame,             NOALIAS    ),
    CONFIG_VARIABLE_INT          (skilllevel,           NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_cheated,         NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_damageinflicted, NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_damagereceived,  NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_deaths,          NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_itemspickedup,   NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_monsterskilled,  NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_secretsrevealed, NOALIAS    ),
    CONFIG_VARIABLE_INT_UNSIGNED (stat_time,            NOALIAS    ),
    CONFIG_VARIABLE_INT          (vid_capfps,           BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (vid_display,          NOALIAS    ),
#if !defined(WIN32)
    CONFIG_VARIABLE_STRING       (vid_driver,           NOALIAS    ),
#endif
    CONFIG_VARIABLE_INT          (vid_fullscreen,       BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (vid_scaledriver,      NOALIAS    ),
    CONFIG_VARIABLE_STRING       (vid_scalefilter,      NOALIAS    ),
    CONFIG_VARIABLE_OTHER        (vid_screenresolution, NOALIAS    ),
    CONFIG_VARIABLE_INT          (vid_vsync,            BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (vid_widescreen,       BOOLALIAS  ),
    CONFIG_VARIABLE_OTHER        (vid_windowposition,   NOALIAS    ),
    CONFIG_VARIABLE_OTHER        (vid_windowsize,       NOALIAS    )
};

alias_t aliases[] =
{
    { "off",           0, BOOLALIAS   }, { "on",            1, BOOLALIAS   },
    { "0",             0, BOOLALIAS   }, { "1",             1, BOOLALIAS   },
    { "no",            0, BOOLALIAS   }, { "yes",           1, BOOLALIAS   },
    { "false",         0, BOOLALIAS   }, { "true",          1, BOOLALIAS   },
    { "low",           0, DETAILALIAS }, { "high",          1, DETAILALIAS },
    { "unlimited", 32768, SPLATALIAS  }, { "off",           1, GAMMAALIAS  },
    { "none",          0, BLOODALIAS  }, { "red",           1, BLOODALIAS  },
    { "all",           2, BLOODALIAS  }, { "",              0, NOALIAS     }
};

char *striptrailingzero(float value, int precision)
{
    size_t      len;
    static char result[100];

    M_snprintf(result, sizeof(result), "%.*f",
        (precision == 2 ? 2 : (value != floor(value))), value);
    len = strlen(result);
    if (len >= 4 && result[len - 3] == '.' && result[len - 1] == '0')
        result[len - 1] = '\0';
    return result;
}

static void SaveBind(FILE *file, char *action, int value, controltype_t type)
{
    int i = 0;

    while (controls[i].type)
    {
        if (controls[i].type == type && controls[i].value == value)
        {
            char *control = controls[i].control;

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
    FILE        *file = fopen(configfile, "w");

    if (!file)
        return; // can't write the file, but don't complain

    if (returntowidescreen)
        vid_widescreen = true;

    for (i = 0; i < arrlen(cvars); i++)
    {
        // Print the name and line up all values at 30 characters
        fprintf(file, "%s ", cvars[i].name);

        // Print the value
        switch (cvars[i].type)
        {
            case DEFAULT_INT:
            {
                int         j = 0;
                dboolean    flag = false;
                int         v = *(int *)cvars[i].location;

                while (aliases[j].text[0])
                {
                    if (v == aliases[j].value && cvars[i].aliastype == aliases[j].type)
                    {
                        fprintf(file, "%s", aliases[j].text);
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
                int         j = 0;
                dboolean    flag = false;
                int         v = *(int *)cvars[i].location;

                while (aliases[j].text[0])
                {
                    if (v == aliases[j].value && cvars[i].aliastype == aliases[j].type)
                    {
                        fprintf(file, "%s", aliases[j].text);
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

                while (aliases[j].text[0])
                {
                    if (v == aliases[j].value && cvars[i].aliastype == aliases[j].type)
                    {
                        fprintf(file, "%s", aliases[j].text);
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

                while (aliases[j].text[0])
                {
                    if (v == aliases[j].value && cvars[i].aliastype == aliases[j].type)
                    {
                        fprintf(file, "%s", aliases[j].text);
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
                fprintf(file, "%s", *(char **)cvars[i].location);
                break;
        }

        fprintf(file, "\n");
    }

    fprintf(file, "\n");

    i = 0;
    while (actions[i].action[0])
    {
        if (actions[i].keyboard)
            SaveBind(file, actions[i].action, *(int *)actions[i].keyboard, keyboard);
        if (actions[i].mouse)
            SaveBind(file, actions[i].action, *(int *)actions[i].mouse, mouse);
        if (actions[i].gamepad)
            SaveBind(file, actions[i].action, *(int *)actions[i].gamepad, gamepad);
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

    while (aliases[i].text[0])
    {
        if (!strcasecmp(strparm, aliases[i].text) && aliastype == aliases[i].type)
            return aliases[i].value;
        i++;
    }

    sscanf(strparm, "%10i", &parm);

    return parm;
}

// Parses float values in the configuration file
static float ParseFloatParameter(char *strparm, int aliastype)
{
    int     i = 0;

    while (aliases[i].text[0])
    {
        if (!strcasecmp(strparm, aliases[i].text) && aliastype == aliases[i].type)
            return (float)aliases[i].value;
        i++;
    }

    return (float)atof(strparm);
}

void C_Bind(char *cmd, char *parm1, char *parm2);

static void M_CheckCVARs(void)
{
    if (pm_alwaysrun != false && pm_alwaysrun != true)
        pm_alwaysrun = pm_alwaysrun_default;

    if (r_liquid_bob != false && r_liquid_bob != true)
        r_liquid_bob = r_liquid_bob_default;

    if (r_blood != noblood && r_blood != redbloodonly && r_blood != allbloodcolors)
        r_blood = r_blood_default;

    if (r_brightmaps != false && r_brightmaps != true)
        r_brightmaps = r_brightmaps_default;

    if (vid_capfps != false && vid_capfps != true)
        vid_capfps = vid_capfps_default;

    if (pm_centerweapon != false && pm_centerweapon != true)
        pm_centerweapon = pm_centerweapon_default;

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

    if (m_doubleclick_use != false && m_doubleclick_use != true)
        m_doubleclick_use = m_doubleclick_use_default;

    if (r_floatbob != false && r_floatbob != true)
        r_floatbob = r_floatbob_default;

    if (r_liquid_clipsprites != false && r_liquid_clipsprites != true)
        r_liquid_clipsprites = r_liquid_clipsprites_default;

    if (vid_fullscreen != false && vid_fullscreen != true)
        vid_fullscreen = vid_fullscreen_default;

    gp_deadzone_left = BETWEENF(gp_deadzone_left_min, gp_deadzone_left, gp_deadzone_left_max);
    gamepadleftdeadzone = (int)(gp_deadzone_left * (float)SHRT_MAX / 100.0f);

    gp_deadzone_right = BETWEENF(gp_deadzone_right_min, gp_deadzone_right, gp_deadzone_right_max);
    gamepadrightdeadzone = (int)(gp_deadzone_right * (float)SHRT_MAX / 100.0f);

    if (gp_swapthumbsticks != false && gp_swapthumbsticks != true)
        gp_swapthumbsticks = gp_swapthumbsticks_default;

    gp_sensitivity = BETWEEN(gp_sensitivity_min, gp_sensitivity, gp_sensitivity_max);
    gamepadsensitivityf = (!gp_sensitivity ? 0.0f : GP_SENSITIVITY_OFFSET
        + gp_sensitivity / (float)gp_sensitivity_max * GP_SENSITIVITY_FACTOR);

    if (gp_vibrate != false && gp_vibrate != true)
        gp_vibrate = gp_vibrate_default;

    r_gamma = BETWEENF(r_gamma_min, r_gamma, r_gamma_max);
    gammaindex = 0;
    while (gammaindex < GAMMALEVELS)
        if (gammalevels[gammaindex++] == r_gamma)
        break;
    if (gammaindex == GAMMALEVELS)
    {
        gammaindex = 0;
        while (gammalevels[gammaindex++] != r_gamma_default);
    }
    --gammaindex;

    if (r_detail != low && r_detail != high)
        r_detail = r_detail_default;

    if (am_grid != false && am_grid != true)
        am_grid = am_grid_default;

    if (r_homindicator != false && r_homindicator != true)
        r_homindicator = r_homindicator_default;

    if (r_hud != false && r_hud != true)
        r_hud = r_hud_default;

    if (messages != false && messages != true)
        messages = messages_default;

    if (r_mirroredweapons != false && r_mirroredweapons != true)
        r_mirroredweapons = r_mirroredweapons_default;

    r_bloodsplats_max = BETWEEN(r_bloodsplats_max_min, r_bloodsplats_max, r_bloodsplats_max_max);

    m_sensitivity = BETWEEN(m_sensitivity_min, m_sensitivity, m_sensitivity_max);

    musicVolume = (BETWEEN(s_musicvolume_min, s_musicvolume, s_musicvolume_max) * 15 + 50) / 100;

    if (m_novertical != false && m_novertical != true)
        m_novertical = m_novertical_default;

    pm_walkbob = BETWEEN(pm_walkbob_min, pm_walkbob, pm_walkbob_max);

    if (r_playersprites != false && r_playersprites != true)
        r_playersprites = r_playersprites_default;

    if (s_randompitch != false && s_randompitch != true)
        s_randompitch = s_randompitch_default;

    if (am_rotatemode != false && am_rotatemode != true)
        am_rotatemode = am_rotatemode_default;

    runcount = BETWEEN(0, runcount, runcount_max);

    if (strcasecmp(vid_scaledriver, vid_scaledriver_direct3d)
        && strcasecmp(vid_scaledriver, vid_scaledriver_opengl)
        && strcasecmp(vid_scaledriver, vid_scaledriver_software))
        vid_scaledriver = vid_scaledriver_default;

    if (strcasecmp(vid_scalefilter, vid_scalefilter_nearest)
        && strcasecmp(vid_scalefilter, vid_scalefilter_linear))
        vid_scalefilter = vid_scalefilter_default;

    r_screensize = BETWEEN(r_screensize_min, r_screensize, r_screensize_max);

    episode = BETWEEN(episode_min, episode, episode_max - (gamemode == registered));

    expansion = BETWEEN(expansion_min, expansion, expansion_max);

    savegame = BETWEEN(0, savegame, 5);

    skilllevel = BETWEEN(skilllevel_min, skilllevel, skilllevel_max);

    sfxVolume = (BETWEEN(s_sfxvolume_min, s_sfxvolume, s_sfxvolume_max) * 15 + 50) / 100;

    if (r_shadows != false && r_shadows != true)
        r_shadows = r_shadows_default;

    if (r_rockettrails != false && r_rockettrails != true)
        r_rockettrails = r_rockettrails_default;

    if (r_fixspriteoffsets != false && r_fixspriteoffsets != true)
        r_fixspriteoffsets = r_fixspriteoffsets_default;

    if (r_liquid_swirl != false && r_liquid_swirl != true)
        r_liquid_swirl = r_liquid_swirl_default;

    if (r_translucency != false && r_translucency != true)
        r_translucency = r_translucency_default;

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
    char        defname[32];
    char        strparm[256];

    configfile = strdup(filename);

    // read the file in, overriding any set defaults
    file = fopen(filename, "r");

    if (!file)
    {
        C_Output("%s not found. Using defaults for all CVARs and creating %s.",
            uppercase(filename), uppercase(PACKAGE_CONFIG));
        M_CheckCVARs();
        return;
    }

    while (!feof(file))
    {
        if (fscanf(file, "bind %31s %31[^\n]\n", control, action) == 2)
        {
            C_StripQuotes(control);
            C_StripQuotes(action);
            C_Bind("", control, action);
            continue;
        }
        else if (fscanf(file, "%31s %255[^\n]\n", defname, strparm) != 2)
            // This line doesn't match
            continue;

        // Strip off trailing non-printable characters (\r characters from DOS text files)
        while (strlen(strparm) > 0 && !isprint((unsigned char)strparm[strlen(strparm) - 1]))
            strparm[strlen(strparm) - 1] = '\0';

        // Find the setting in the list
        for (i = 0; i < arrlen(cvars); ++i)
        {
            char        *s;

            if (strcasecmp(defname, cvars[i].name))
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

    C_Output("Loaded CVARs from %s.", uppercase(filename));
    M_CheckCVARs();
}
