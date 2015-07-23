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

char    *configfile = PACKAGE_CONFIG;

int     musicvolume_percent = MUSICVOLUME_DEFAULT;
int     sfxvolume_percent = SFXVOLUME_DEFAULT;

extern dboolean am_grid;
extern dboolean am_rotatemode;
extern int      episode;
extern int      expansion;
extern float    gp_deadzone_left;
extern float    gp_deadzone_right;
extern int      gp_sensitivity;
extern dboolean gp_swapthumbsticks;
extern dboolean gp_vibrate;
extern char     *iwadfolder;
extern dboolean messages;
extern float    m_acceleration;
extern dboolean m_doubleclick_use;
extern dboolean m_novertical;
extern int      m_sensitivity;
extern int      m_threshold;
extern char     *playername;
extern dboolean pm_alwaysrun;
extern dboolean pm_centerweapon;
extern int      pm_walkbob;
extern int      r_blood;
extern dboolean r_brightmaps;
extern dboolean r_corpses_mirrored;
extern dboolean r_corpses_moreblood;
extern dboolean r_corpses_nudge;
extern dboolean r_corpses_slide;
extern dboolean r_corpses_smearblood;
extern int      r_detail;
extern dboolean r_fixmaperrors;
extern dboolean r_fixspriteoffsets;
extern dboolean r_floatbob;
extern float    r_gamma;
extern dboolean r_homindicator;
extern dboolean r_hud;
extern dboolean r_liquid_bob;
extern dboolean r_liquid_clipsprites;
extern dboolean r_liquid_ripple;
extern char     *r_lowpixelsize;
extern int      r_maxbloodsplats;
extern dboolean r_mirrorweapons;
extern dboolean r_playersprites;
extern dboolean r_rockettrails;
extern dboolean r_shadows;
extern dboolean randompitch;
extern int      runcount;
extern int      selectedsavegame;
extern int      skilllevel;
extern char     *timidity_cfg_path;
extern dboolean r_translucency;
extern dboolean vid_capfps;
extern int      vid_display;
#if !defined(WIN32)
extern char     *vid_driver;
#endif
extern dboolean vid_fullscreen;
extern char     *vid_scaledriver;
extern char     *vid_scalefilter;
extern char     *vid_screenresolution;
extern dboolean vid_vsync;
extern dboolean vid_widescreen;
extern char     *vid_windowposition;
extern char     *vid_windowsize;

extern int      gamepadleftdeadzone;
extern int      gamepadrightdeadzone;
extern int      pixelwidth;
extern int      pixelheight;
extern dboolean returntowidescreen;

#define CONFIG_VARIABLE_GENERIC(name, variable, type, set) \
    { #name, &variable, type, set }

#define CONFIG_VARIABLE_INT(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_INT, set)
#define CONFIG_VARIABLE_INT_HEX(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_INT_HEX, set)
#define CONFIG_VARIABLE_INT_PERCENT(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_INT_PERCENT, set)
#define CONFIG_VARIABLE_FLOAT(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_FLOAT, set)
#define CONFIG_VARIABLE_FLOAT_PERCENT(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_FLOAT_PERCENT, set)
#define CONFIG_VARIABLE_STRING(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_STRING, set)

static default_t cvars[] =
{
    CONFIG_VARIABLE_INT          (am_grid,                 am_grid,                      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (am_rotatemode,           am_rotatemode,                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (episode,                 episode,                      NOALIAS    ),
    CONFIG_VARIABLE_INT          (expansion,               expansion,                    NOALIAS    ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_left,        gp_deadzone_left,             NOALIAS    ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_right,       gp_deadzone_right,            NOALIAS    ),
    CONFIG_VARIABLE_INT          (gp_sensitivity,          gp_sensitivity,               NOALIAS    ),
    CONFIG_VARIABLE_INT          (gp_swapthumbsticks,      gp_swapthumbsticks,           BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (gp_vibrate,              gp_vibrate,                   BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (iwadfolder,              iwadfolder,                   NOALIAS    ),
    CONFIG_VARIABLE_FLOAT        (m_acceleration,          m_acceleration,               NOALIAS    ),
    CONFIG_VARIABLE_INT          (m_doubleclick_use,       m_doubleclick_use,            BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (m_novertical,            m_novertical,                 BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (m_sensitivity,           m_sensitivity,                NOALIAS    ),
    CONFIG_VARIABLE_INT          (m_threshold,             m_threshold,                  NOALIAS    ),
    CONFIG_VARIABLE_INT          (messages,                messages,                     BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (playername,              playername,                   NOALIAS    ),
    CONFIG_VARIABLE_INT          (pm_alwaysrun,            pm_alwaysrun,                 BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (pm_centerweapon,         pm_centerweapon,              BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (pm_walkbob,              pm_walkbob,                   NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_blood,                 r_blood,                      BLOODALIAS ),
    CONFIG_VARIABLE_INT          (r_brightmaps,            r_brightmaps,                 BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_mirrored,      r_corpses_mirrored,           BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_moreblood,     r_corpses_moreblood,          BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_nudge,         r_corpses_nudge,              BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_slide,         r_corpses_slide,              BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_smearblood,    r_corpses_smearblood,         BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_detail,                r_detail,                     DETAILALIAS),
    CONFIG_VARIABLE_INT          (r_fixmaperrors,          r_fixmaperrors,               BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_fixspriteoffsets,      r_fixspriteoffsets,           BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_floatbob,              r_floatbob,                   BOOLALIAS  ),
    CONFIG_VARIABLE_FLOAT        (r_gamma,                 r_gamma,                      GAMMAALIAS ),
    CONFIG_VARIABLE_INT          (r_homindicator,          r_homindicator,               BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_hud,                   r_hud,                        BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_bob,            r_liquid_bob,                 BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_clipsprites,    r_liquid_clipsprites,         BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_ripple,         r_liquid_ripple,              BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (r_lowpixelsize,          r_lowpixelsize,               NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_maxbloodsplats,        r_maxbloodsplats,             SPLATALIAS ),
    CONFIG_VARIABLE_INT          (r_mirrorweapons,         r_mirrorweapons,              BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_playersprites,         r_playersprites,              BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_rockettrails,          r_rockettrails,               BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_screensize,            r_screensize,                 NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_shadows,               r_shadows,                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_translucency,          r_translucency,               BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (runcount,                runcount,                     NOALIAS    ),
    CONFIG_VARIABLE_INT_PERCENT  (s_musicvolume,           musicvolume_percent,          NOALIAS    ),
    CONFIG_VARIABLE_INT          (s_randompitch,           randompitch,                  BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (s_sfxvolume,             sfxvolume_percent,            NOALIAS    ),
    CONFIG_VARIABLE_STRING       (s_timiditycfgpath,       timidity_cfg_path,            NOALIAS    ),
    CONFIG_VARIABLE_INT          (savegame,                selectedsavegame,             NOALIAS    ),
    CONFIG_VARIABLE_INT          (skilllevel,              skilllevel,                   NOALIAS    ),
    CONFIG_VARIABLE_INT          (vid_capfps,              vid_capfps,                   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (vid_display,             vid_display,                  NOALIAS    ),
#if !defined(WIN32)
    CONFIG_VARIABLE_STRING       (vid_driver,              vid_driver,                   NOALIAS    ),
#endif
    CONFIG_VARIABLE_INT          (vid_fullscreen,          vid_fullscreen,               BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (vid_scaledriver,         vid_scaledriver,              NOALIAS    ),
    CONFIG_VARIABLE_STRING       (vid_scalefilter,         vid_scalefilter,              NOALIAS    ),
    CONFIG_VARIABLE_STRING       (vid_screenresolution,    vid_screenresolution,         NOALIAS    ),
    CONFIG_VARIABLE_INT          (vid_vsync,               vid_vsync,                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (vid_widescreen,          vid_widescreen,               BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (vid_windowposition,      vid_windowposition,           NOALIAS    ),
    CONFIG_VARIABLE_STRING       (vid_windowsize,          vid_windowsize,               NOALIAS    )
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
                    if (v == aliases[j].value && cvars[i].set == aliases[j].set)
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

            case DEFAULT_INT_PERCENT:
            {
                int         j = 0;
                dboolean    flag = false;
                int         v = *(int *)cvars[i].location;

                while (aliases[j].text[0])
                {
                    if (v == aliases[j].value && cvars[i].set == aliases[j].set)
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
                    if (v == aliases[j].value && cvars[i].set == aliases[j].set)
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
                    if (v == aliases[j].value && cvars[i].set == aliases[j].set)
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
static int ParseIntParameter(char *strparm, int set)
{
    int parm = 0;
    int i = 0;

    while (aliases[i].text[0])
    {
        if (!strcasecmp(strparm, aliases[i].text) && set == aliases[i].set)
            return aliases[i].value;
        i++;
    }

    sscanf(strparm, "%10i", &parm);

    return parm;
}

// Parses float values in the configuration file
static float ParseFloatParameter(char *strparm, int set)
{
    int     i = 0;

    while (aliases[i].text[0])
    {
        if (!strcasecmp(strparm, aliases[i].text) && set == aliases[i].set)
            return (float)aliases[i].value;
        i++;
    }

    return (float)atof(strparm);
}

void C_Bind(char *cmd, char *parm1, char *parm2);

static void M_CheckCVARs(void)
{
    if (pm_alwaysrun != false && pm_alwaysrun != true)
        pm_alwaysrun = PM_ALWAYSRUN_DEFAULT;

    if (r_liquid_bob != false && r_liquid_bob != true)
        r_liquid_bob = R_LIQUID_BOB_DEFAULT;

    if (r_blood != NOBLOOD && r_blood != REDBLOODONLY && r_blood != ALLBLOODCOLORS)
        r_blood = R_BLOOD_DEFAULT;

    if (r_brightmaps != false && r_brightmaps != true)
        r_brightmaps = R_BRIGHTMAPS_DEFAULT;

    if (vid_capfps != false && vid_capfps != true)
        vid_capfps = VID_CAPFPS_DEFAULT;

    if (pm_centerweapon != false && pm_centerweapon != true)
        pm_centerweapon = PM_CENTERWEAPON_DEFAULT;

    if (r_corpses_mirrored != false && r_corpses_mirrored != true)
        r_corpses_mirrored = R_CORPSES_MIRRORED_DEFAULT;

    if (r_corpses_moreblood != false && r_corpses_moreblood != true)
        r_corpses_moreblood = R_CORPSES_MOREBLOOD_DEFAULT;

    if (r_corpses_nudge != false && r_corpses_nudge != true)
        r_corpses_nudge = R_CORPSES_NUDGE_DEFAULT;

    if (r_corpses_slide != false && r_corpses_slide != true)
        r_corpses_slide = R_CORPSES_SLIDE_DEFAULT;

    if (r_corpses_smearblood != false && r_corpses_smearblood != true)
        r_corpses_smearblood = R_CORPSES_SMEARBLOOD_DEFAULT;

    if (m_doubleclick_use != false && m_doubleclick_use != true)
        m_doubleclick_use = M_DOUBLECLICK_USE_DEFAULT;

    if (r_floatbob != false && r_floatbob != true)
        r_floatbob = R_FLOATBOB_DEFAULT;

    if (r_liquid_clipsprites != false && r_liquid_clipsprites != true)
        r_liquid_clipsprites = R_LIQUID_CLIPSPRITES_DEFAULT;

    if (vid_fullscreen != false && vid_fullscreen != true)
        vid_fullscreen = VID_FULLSCREEN_DEFAULT;

    gp_deadzone_left = BETWEENF(GP_DEADZONE_LEFT_MIN, gp_deadzone_left, GP_DEADZONE_LEFT_MAX);
    gamepadleftdeadzone = (int)(gp_deadzone_left * (float)SHRT_MAX / 100.0f);

    gp_deadzone_right = BETWEENF(GP_DEADZONE_RIGHT_MIN, gp_deadzone_right, GP_DEADZONE_RIGHT_MAX);
    gamepadrightdeadzone = (int)(gp_deadzone_right * (float)SHRT_MAX / 100.0f);

    if (gp_swapthumbsticks != false && gp_swapthumbsticks != true)
        gp_swapthumbsticks = GP_SWAPTHUMBSTICKS_DEFAULT;

    gp_sensitivity = BETWEEN(GP_SENSITIVITY_MIN, gp_sensitivity, GP_SENSITIVITY_MAX);
    gamepadsensitivityf = (!gp_sensitivity ? 0.0f : GP_SENSITIVITY_OFFSET
        + gp_sensitivity / (float)GP_SENSITIVITY_MAX * GP_SENSITIVITY_FACTOR);

    if (gp_vibrate != false && gp_vibrate != true)
        gp_vibrate = GP_VIBRATE_DEFAULT;

    r_gamma = BETWEENF(R_GAMMA_MIN, r_gamma, R_GAMMA_MAX);
    gammaindex = 0;
    while (gammaindex < GAMMALEVELS)
        if (gammalevels[gammaindex++] == r_gamma)
        break;
    if (gammaindex == GAMMALEVELS)
    {
        gammaindex = 0;
        while (gammalevels[gammaindex++] != R_GAMMA_DEFAULT);
    }
    --gammaindex;

    if (r_detail != LOW && r_detail != HIGH)
        r_detail = R_DETAIL_DEFAULT;

    if (am_grid != false && am_grid != true)
        am_grid = AM_GRID_DEFAULT;

    if (r_homindicator != false && r_homindicator != true)
        r_homindicator = R_HOMINDICATOR_DEFAULT;

    if (r_hud != false && r_hud != true)
        r_hud = R_HUD_DEFAULT;

    if (messages != false && messages != true)
        messages = MESSAGES_DEFAULT;

    if (r_mirrorweapons != false && r_mirrorweapons != true)
        r_mirrorweapons = R_MIRRORWEAPONS_DEFAULT;

    r_maxbloodsplats = BETWEEN(R_MAXBLOODSPLATS_MIN, r_maxbloodsplats, R_MAXBLOODSPLATS_MAX);

    m_sensitivity = BETWEEN(M_SENSITIVITY_MIN, m_sensitivity, M_SENSITIVITY_MAX);

    musicVolume = (BETWEEN(MUSICVOLUME_MIN, musicvolume_percent, MUSICVOLUME_MAX) * 15 + 50) / 100;

    if (m_novertical != false && m_novertical != true)
        m_novertical = M_NOVERTICAL_DEFAULT;

    pm_walkbob = BETWEEN(PM_WALKBOB_MIN, pm_walkbob, PM_WALKBOB_MAX);

    if (r_playersprites != false && r_playersprites != true)
        r_playersprites = R_PLAYERSPRITES_DEFAULT;

    if (randompitch != false && randompitch != true)
        randompitch = RANDOMPITCH_DEFAULT;

    if (am_rotatemode != false && am_rotatemode != true)
        am_rotatemode = AM_ROTATEMODE_DEFAULT;

    runcount = BETWEEN(0, runcount, RUNCOUNT_MAX);

    if (strcasecmp(vid_scaledriver, "direct3d") && strcasecmp(vid_scaledriver, "opengl")
        && strcasecmp(vid_scaledriver, "software"))
        vid_scaledriver = VID_SCALEDRIVER_DEFAULT;

    if (strcasecmp(vid_scalefilter, "nearest") && strcasecmp(vid_scalefilter, "linear"))
        vid_scalefilter = VID_SCALEFILTER_DEFAULT;

    r_screensize = BETWEEN(R_SCREENSIZE_MIN, r_screensize, R_SCREENSIZE_MAX);

    episode = BETWEEN(EPISODE_MIN, episode, EPISODE_MAX - (gamemode == registered));

    expansion = BETWEEN(EXPANSION_MIN, expansion, EXPANSION_MAX);

    selectedsavegame = BETWEEN(0, selectedsavegame, 5);

    skilllevel = BETWEEN(SKILLLEVEL_MIN, skilllevel, SKILLLEVEL_MAX);

    sfxVolume = (BETWEEN(SFXVOLUME_MIN, sfxvolume_percent, SFXVOLUME_MAX) * 15 + 50) / 100;

    if (r_shadows != false && r_shadows != true)
        r_shadows = R_SHADOWS_DEFAULT;

    if (r_rockettrails != false && r_rockettrails != true)
        r_rockettrails = R_ROCKETTRAILS_DEFAULT;

    if (r_fixspriteoffsets != false && r_fixspriteoffsets != true)
        r_fixspriteoffsets = R_FIXSPRITEOFFSETS_DEFAULT;

    if (r_liquid_ripple != false && r_liquid_ripple != true)
        r_liquid_ripple = R_LIQUID_RIPPLE_DEFAULT;

    if (r_translucency != false && r_translucency != true)
        r_translucency = R_TRANSLUCENCY_DEFAULT;

    if (vid_vsync != false && vid_vsync != true)
        vid_vsync = VID_VSYNC_DEFAULT;

    if (vid_widescreen != false && vid_widescreen != true)
        vid_widescreen = VID_WIDESCREEN_DEFAULT;
    if (vid_widescreen || r_screensize == R_SCREENSIZE_MAX)
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
        while (strlen(strparm) > 0 && !isprint(strparm[strlen(strparm) - 1]))
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
                    *(int *)cvars[i].location = ParseIntParameter(strparm, cvars[i].set);
                    break;

                case DEFAULT_INT_PERCENT:
                    s = strdup(strparm);
                    if (strlen(s) >= 1 && s[strlen(s) - 1] == '%')
                        s[strlen(s) - 1] = '\0';
                    *(int *)cvars[i].location = ParseIntParameter(s, cvars[i].set);
                    break;

                case DEFAULT_FLOAT:
                    *(float *)cvars[i].location = ParseFloatParameter(strparm, cvars[i].set);
                    break;

                case DEFAULT_FLOAT_PERCENT:
                    s = strdup(s);
                    if (strlen(s) >= 1 && s[strlen(s) - 1] == '%')
                        s[strlen(s) - 1] = '\0';
                    *(float *)cvars[i].location = ParseFloatParameter(s, cvars[i].set);
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
