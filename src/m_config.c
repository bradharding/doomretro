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

float   gamepadleftdeadzone_percent = GAMEPADLEFTDEADZONE_DEFAULT;
float   gamepadrightdeadzone_percent = GAMEPADRIGHTDEADZONE_DEFAULT;
int     musicvolume_percent = MUSICVOLUME_DEFAULT;
int     sfxvolume_percent = SFXVOLUME_DEFAULT;

//
// DEFAULTS
//
extern dboolean alwaysrun;
extern dboolean am_grid;
extern dboolean am_rotatemode;
extern dboolean animatedliquid;
extern int      blood;
extern dboolean brightmaps;
extern dboolean centerweapon;
extern dboolean corpses_mirror;
extern dboolean corpses_moreblood;
extern dboolean corpses_nudge;
extern dboolean corpses_slide;
extern dboolean corpses_smearblood;
extern dboolean dclick_use;
extern int      episode;
extern int      expansion;
extern dboolean floatbob;
extern dboolean footclip;
extern int      gamepadleftdeadzone;
extern int      gamepadrightdeadzone;
extern dboolean gamepadlefthanded;
extern int      gamepadsensitivity;
extern dboolean gamepadvibrate;
extern float    gammalevel;
extern int      graphicdetail;
extern dboolean homindicator;
extern dboolean hud;
extern char     *iwadfolder;
extern dboolean mapfixes;
extern int      maxbloodsplats;
extern dboolean messages;
extern dboolean mirrorweapons;
extern int      mousesensitivity;
extern float    mouse_acceleration;
extern int      mouse_threshold;
extern dboolean novert;
extern int      pixelheight;
extern char     *pixelsize;
extern int      pixelwidth;
extern int      playerbob;
extern char     *playername;
extern dboolean playersprites;
extern dboolean randompitch;
extern int      runcount;
extern int      screenheight;
extern int      screenwidth;
extern int      selectedsavegame;
extern dboolean shadows;
extern int      skilllevel;
extern dboolean smoketrails;
extern dboolean spritefixes;
extern dboolean swirlingliquid;
extern char     *timidity_cfg_path;
extern dboolean translucency;
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
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_left,        gamepadleftdeadzone_percent,  NOALIAS    ),
    CONFIG_VARIABLE_FLOAT_PERCENT(gp_deadzone_right,       gamepadrightdeadzone_percent, NOALIAS    ),
    CONFIG_VARIABLE_INT          (gp_sensitivity,          gamepadsensitivity,           NOALIAS    ),
    CONFIG_VARIABLE_INT          (gp_swapthumbsticks,      gamepadlefthanded,            BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (gp_vibrate,              gamepadvibrate,               BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (iwadfolder,              iwadfolder,                   NOALIAS    ),
    CONFIG_VARIABLE_FLOAT        (m_acceleration,          mouse_acceleration,           NOALIAS    ),
    CONFIG_VARIABLE_INT          (m_doubleclick_use,       dclick_use,                   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (m_novertical,            novert,                       BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (m_sensitivity,           mousesensitivity,             NOALIAS    ),
    CONFIG_VARIABLE_INT          (m_threshold,             mouse_threshold,              NOALIAS    ),
    CONFIG_VARIABLE_INT          (messages,                messages,                     BOOLALIAS  ),
    CONFIG_VARIABLE_STRING       (playername,              playername,                   NOALIAS    ),
    CONFIG_VARIABLE_INT          (pm_alwaysrun,            alwaysrun,                    BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (pm_centerweapon,         centerweapon,                 BOOLALIAS  ),
    CONFIG_VARIABLE_INT_PERCENT  (pm_walkbob,              playerbob,                    NOALIAS    ),
    CONFIG_VARIABLE_INT          (runcount,                runcount,                     NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_blood,                 blood,                        BLOODALIAS ),
    CONFIG_VARIABLE_INT          (r_brightmaps,            brightmaps,                   BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_mirrored,      corpses_mirror,               BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_moreblood,     corpses_moreblood,            BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_nudge,         corpses_nudge,                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_slide,         corpses_slide,                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_corpses_smearblood,    corpses_smearblood,           BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_detail,                graphicdetail,                DETAILALIAS),
    CONFIG_VARIABLE_INT          (r_fixmaperrors,          mapfixes,                     BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_fixspriteoffsets,      spritefixes,                  BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_floatbob,              floatbob,                     BOOLALIAS  ),
    CONFIG_VARIABLE_FLOAT        (r_gamma,                 gammalevel,                   GAMMAALIAS ),
    CONFIG_VARIABLE_INT          (r_homindicator,          homindicator,                 BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_hud,                   hud,                          BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_bob,            animatedliquid,               BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_clipsprites,    footclip,                     BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_liquid_ripple,         swirlingliquid,               BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_lowpixelheight,        pixelheight,                  NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_lowpixelwidth,         pixelwidth,                   NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_maxbloodsplats,        maxbloodsplats,               SPLATALIAS ),
    CONFIG_VARIABLE_INT          (r_mirrorweapons,         mirrorweapons,                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_playersprites,         playersprites,                BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_rockettrails,          smoketrails,                  BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_screensize,            screensize,                   NOALIAS    ),
    CONFIG_VARIABLE_INT          (r_shadows,               shadows,                      BOOLALIAS  ),
    CONFIG_VARIABLE_INT          (r_translucency,          translucency,                 BOOLALIAS  ),
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
    CONFIG_VARIABLE_INT          (vid_screenheight,        screenheight,                 SCREENALIAS),
    CONFIG_VARIABLE_INT          (vid_screenwidth,         screenwidth,                  SCREENALIAS),
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
    { "desktop",       0, SCREENALIAS }, { "low",           0, DETAILALIAS },
    { "high",          1, DETAILALIAS }, { "unlimited", 32768, SPLATALIAS  },
    { "off",           1, GAMMAALIAS  }, { "none",          0, BLOODALIAS  },
    { "red",           1, BLOODALIAS  }, { "all",           2, BLOODALIAS  }, 
    { "",              0, NOALIAS     }
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
    if (alwaysrun != false && alwaysrun != true)
        alwaysrun = ALWAYSRUN_DEFAULT;

    if (animatedliquid != false && animatedliquid != true)
        animatedliquid = ANIMATEDLIQUID_DEFAULT;

    if (blood != NOBLOOD && blood != REDBLOODONLY && blood != ALLBLOODCOLORS)
        blood = BLOOD_DEFAULT;

    if (brightmaps != false && brightmaps != true)
        brightmaps = BRIGHTMAPS_DEFAULT;

    if (vid_capfps != false && vid_capfps != true)
        vid_capfps = VID_CAPFPS_DEFAULT;

    if (centerweapon != false && centerweapon != true)
        centerweapon = CENTERWEAPON_DEFAULT;

    if (corpses_mirror != false && corpses_mirror != true)
        corpses_mirror = CORPSES_MIRROR_DEFAULT;

    if (corpses_moreblood != false && corpses_moreblood != true)
        corpses_moreblood = CORPSES_MOREBLOOD_DEFAULT;

    if (corpses_nudge != false && corpses_nudge != true)
        corpses_nudge = CORPSES_NUDGE_DEFAULT;

    if (corpses_slide != false && corpses_slide != true)
        corpses_slide = CORPSES_SLIDE_DEFAULT;

    if (corpses_smearblood != false && corpses_smearblood != true)
        corpses_smearblood = CORPSES_SMEARBLOOD_DEFAULT;

    if (dclick_use != false && dclick_use != true)
        dclick_use = DCLICKUSE_DEFAULT;

    if (floatbob != false && floatbob != true)
        floatbob = FLOATBOB_DEFAULT;

    if (footclip != false && footclip != true)
        footclip = FOOTCLIP_DEFAULT;

    if (vid_fullscreen != false && vid_fullscreen != true)
        vid_fullscreen = VID_FULLSCREEN_DEFAULT;

    gamepadleftdeadzone = (int)(BETWEENF(GAMEPADLEFTDEADZONE_MIN, gamepadleftdeadzone_percent,
        GAMEPADLEFTDEADZONE_MAX) * (float)SHRT_MAX / 100.0f);

    gamepadrightdeadzone = (int)(BETWEENF(GAMEPADRIGHTDEADZONE_MIN, gamepadrightdeadzone_percent,
        GAMEPADRIGHTDEADZONE_MAX) * (float)SHRT_MAX / 100.0f);

    if (gamepadlefthanded != false && gamepadlefthanded != true)
        gamepadlefthanded = GAMEPADLEFTHANDED_DEFAULT;

    gamepadsensitivity = BETWEEN(GAMEPADSENSITIVITY_MIN, gamepadsensitivity,
        GAMEPADSENSITIVITY_MAX);
    gamepadsensitivityf = (!gamepadsensitivity ? 0.0f :
        GAMEPADSENSITIVITY_OFFSET + gamepadsensitivity / (float)GAMEPADSENSITIVITY_MAX *
        GAMEPADSENSITIVITY_FACTOR);

    if (gamepadvibrate != false && gamepadvibrate != true)
        gamepadvibrate = GAMEPADVIBRATE_DEFAULT;

    gammalevel = BETWEENF(GAMMALEVEL_MIN, gammalevel, GAMMALEVEL_MAX);
    gammaindex = 0;
    while (gammaindex < GAMMALEVELS)
        if (gammalevels[gammaindex++] == gammalevel)
        break;
    if (gammaindex == GAMMALEVELS)
    {
        gammaindex = 0;
        while (gammalevels[gammaindex++] != GAMMALEVEL_DEFAULT);
    }
    --gammaindex;

    if (graphicdetail != LOW && graphicdetail != HIGH)
        graphicdetail = GRAPHICDETAIL_DEFAULT;

    if (am_grid != false && am_grid != true)
        am_grid = GRID_DEFAULT;

    if (homindicator != false && homindicator != true)
        homindicator = HOMINDICATOR_DEFAULT;

    if (hud != false && hud != true)
        hud = HUD_DEFAULT;

    if (messages != false && messages != true)
        messages = MESSAGES_DEFAULT;

    if (mirrorweapons != false && mirrorweapons != true)
        mirrorweapons = MIRRORWEAPONS_DEFAULT;

    maxbloodsplats = BETWEEN(MAXBLOODSPLATS_MIN, maxbloodsplats, MAXBLOODSPLATS_MAX);

    mousesensitivity = BETWEEN(MOUSESENSITIVITY_MIN, mousesensitivity, MOUSESENSITIVITY_MAX);

    musicVolume = (BETWEEN(MUSICVOLUME_MIN, musicvolume_percent, MUSICVOLUME_MAX) * 15 + 50) / 100;

    if (novert != false && novert != true)
        novert = NOVERT_DEFAULT;

    pixelwidth = BETWEEN(PIXELWIDTH_MIN, pixelwidth, PIXELWIDTH_MAX);
    while (SCREENWIDTH % pixelwidth)
        --pixelwidth;

    pixelheight = BETWEEN(PIXELHEIGHT_MIN, pixelheight, PIXELHEIGHT_MAX);
    while (SCREENHEIGHT % pixelheight)
        --pixelheight;

    playerbob = BETWEEN(PLAYERBOB_MIN, playerbob, PLAYERBOB_MAX);

    if (playersprites != false && playersprites != true)
        playersprites = PLAYERSPRITES_DEFAULT;

    if (randompitch != false && randompitch != true)
        randompitch = RANDOMPITCH_DEFAULT;

    if (am_rotatemode != false && am_rotatemode != true)
        am_rotatemode = ROTATEMODE_DEFAULT;

    runcount = BETWEEN(0, runcount, RUNCOUNT_MAX);

    if (strcasecmp(vid_scaledriver, "direct3d") && strcasecmp(vid_scaledriver, "opengl")
        && strcasecmp(vid_scaledriver, "software"))
        vid_scaledriver = VID_SCALEDRIVER_DEFAULT;

    if (strcasecmp(vid_scalefilter, "nearest") && strcasecmp(vid_scalefilter, "linear"))
        vid_scalefilter = VID_SCALEFILTER_DEFAULT;

    screensize = BETWEEN(SCREENSIZE_MIN, screensize, SCREENSIZE_MAX);

    if (screenwidth && screenheight
        && (screenwidth < SCREENWIDTH || screenheight < SCREENHEIGHT * 3 / 4))
    {
        screenwidth = SCREENWIDTH_DEFAULT;
        screenheight = SCREENHEIGHT_DEFAULT;
    }

    episode = BETWEEN(EPISODE_MIN, episode, EPISODE_MAX - (gamemode == registered));

    expansion = BETWEEN(EXPANSION_MIN, expansion, EXPANSION_MAX);

    selectedsavegame = BETWEEN(0, selectedsavegame, 5);

    skilllevel = BETWEEN(SKILLLEVEL_MIN, skilllevel, SKILLLEVEL_MAX);

    sfxVolume = (BETWEEN(SFXVOLUME_MIN, sfxvolume_percent, SFXVOLUME_MAX) * 15 + 50) / 100;

    if (shadows != false && shadows != true)
        shadows = SHADOWS_DEFAULT;

    if (smoketrails != false && smoketrails != true)
        smoketrails = SMOKETRAILS_DEFAULT;

    if (spritefixes != false && spritefixes != true)
        spritefixes = SPRITEFIXES_DEFAULT;

    if (swirlingliquid != false && swirlingliquid != true)
        swirlingliquid = SWIRLINGLIQUID_DEFAULT;

    if (translucency != false && translucency != true)
        translucency = TRANSLUCENCY_DEFAULT;

    if (vid_vsync != false && vid_vsync != true)
        vid_vsync = VID_VSYNC_DEFAULT;

    if (vid_widescreen != false && vid_widescreen != true)
        vid_widescreen = VID_WIDESCREEN_DEFAULT;
    if (vid_widescreen || screensize == SCREENSIZE_MAX)
    {
        returntowidescreen = true;
        vid_widescreen = false;
    }
    else
        hud = true;

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
