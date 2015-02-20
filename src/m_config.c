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

#include "doomstat.h"
#include "i_gamepad.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "version.h"

float           gamepadleftdeadzone_percent = GAMEPADLEFTDEADZONE_DEFAULT;
float           gamepadrightdeadzone_percent = GAMEPADRIGHTDEADZONE_DEFAULT;
int             musicvolume_percent = MUSICVOLUME_DEFAULT;
int             sfxvolume_percent = SFXVOLUME_DEFAULT;

//
// DEFAULTS
//
extern boolean  alwaysrun;
extern boolean  animatedliquid;
extern int      bloodsplats;
extern int      brightmaps;
extern boolean  centerweapon;
extern boolean  corpses_mirror;
extern boolean  corpses_moreblood;
extern boolean  corpses_slide;
extern boolean  corpses_smearblood;
extern boolean  dclick_use;
extern boolean  floatbob;
extern boolean  footclip;
extern boolean  fullscreen;
extern int      gamepadautomap;
extern int      gamepadautomapclearmark;
extern int      gamepadautomapfollowmode;
extern int      gamepadautomapgrid;
extern int      gamepadautomapmark;
extern int      gamepadautomapmaxzoom;
extern int      gamepadautomaprotatemode;
extern int      gamepadautomapzoomin;
extern int      gamepadautomapzoomout;
extern int      gamepadfire;
extern int      gamepadleftdeadzone;
extern int      gamepadrightdeadzone;
extern boolean  gamepadlefthanded;
extern int      gamepadmenu;
extern int      gamepadnextweapon;
extern int      gamepadprevweapon;
extern int      gamepadrun;
extern int      gamepadsensitivity;
extern int      gamepaduse;
extern boolean  gamepadvibrate;
extern int      gamepadweapon1;
extern int      gamepadweapon2;
extern int      gamepadweapon3;
extern int      gamepadweapon4;
extern int      gamepadweapon5;
extern int      gamepadweapon6;
extern int      gamepadweapon7;
extern float    gammalevel;
extern int      graphicdetail;
extern boolean  grid;
extern boolean  homindicator;
extern boolean  hud;
extern char     *iwadfolder;
extern int      key_automap;
extern int      key_automap_clearmark;
extern int      key_automap_followmode;
extern int      key_automap_grid;
extern int      key_automap_mark;
extern int      key_automap_maxzoom;
extern int      key_automap_rotatemode;
extern int      key_automap_zoomin;
extern int      key_automap_zoomout;
extern int      key_down;
extern int      key_down2;
extern int      key_fire;
extern int      key_left;
extern int      key_nextweapon;
extern int      key_prevweapon;
extern int      key_right;
extern int      key_run;
extern int      key_strafe;
extern int      key_strafeleft;
extern int      key_strafeleft2;
extern int      key_straferight;
extern int      key_straferight2;
extern int      key_up;
extern int      key_up2;
extern int      key_use;
extern int      key_weapon1;
extern int      key_weapon2;
extern int      key_weapon3;
extern int      key_weapon4;
extern int      key_weapon5;
extern int      key_weapon6;
extern int      key_weapon7;
extern boolean  mapfixes;
extern boolean  messages;
extern boolean  mirrorweapons;
extern int      mousesensitivity;
extern float    mouse_acceleration;
extern int      mouse_threshold;
extern int      mousebfire;
extern int      mousebforward;
extern int      mousebprevweapon;
extern int      mousebnextweapon;
extern int      mousebstrafe;
extern int      mousebuse;
extern boolean  novert;
extern int      pixelheight;
extern int      pixelwidth;
extern int      playerbob;
extern boolean  rotatemode;
extern int      runcount;
#if defined(SDL20)
extern char     *scalequality;
#endif
extern int      screenheight;
extern int      screenwidth;
extern int      selectedepisode;
extern int      selectedexpansion;
extern int      selectedsavegame;
extern int      selectedskilllevel;
extern boolean  shadows;
extern boolean  smoketrails;
extern int      snd_maxslicetime_ms;
extern char     *timidity_cfg_path;
extern boolean  translucency;
extern char     *videodriver;
#if defined(SDL20)
extern boolean  vsync;
#endif
extern boolean  widescreen;
extern int      windowheight;
extern char     *windowposition;
extern int      windowwidth;

extern boolean  returntowidescreen;

typedef struct
{
    char                *text;
    int                 value;
    int                 set;
} alias_t;

#define CONFIG_VARIABLE_GENERIC(name, variable, type, set) \
    { #name, &variable, type, 0, 0, set }

#define CONFIG_VARIABLE_KEY(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_KEY, set)
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

static default_t doom_defaults_list[] =
{
    CONFIG_VARIABLE_INT          (alwaysrun,                  alwaysrun,                     1),
    CONFIG_VARIABLE_INT          (animatedliquid,             animatedliquid,                1),
    CONFIG_VARIABLE_INT          (bloodsplats,                bloodsplats,                   7),
    CONFIG_VARIABLE_INT          (brightmaps,                 brightmaps,                    1),
    CONFIG_VARIABLE_INT          (centerweapon,               centerweapon,                  1),
    CONFIG_VARIABLE_INT          (corpses_mirror,             corpses_mirror,                1),
    CONFIG_VARIABLE_INT          (corpses_moreblood,          corpses_moreblood,             1),
    CONFIG_VARIABLE_INT          (corpses_slide,              corpses_slide,                 1),
    CONFIG_VARIABLE_INT          (corpses_smearblood,         corpses_smearblood,            1),
    CONFIG_VARIABLE_INT          (dclick_use,                 dclick_use,                    1),
    CONFIG_VARIABLE_INT          (episode,                    selectedepisode,               8),
    CONFIG_VARIABLE_INT          (expansion,                  selectedexpansion,             9),
    CONFIG_VARIABLE_INT          (floatbob,                   floatbob,                      1),
    CONFIG_VARIABLE_INT          (footclip,                   footclip,                      1),
    CONFIG_VARIABLE_INT          (fullscreen,                 fullscreen,                    1),
    CONFIG_VARIABLE_INT          (gamepad_automap,            gamepadautomap,                2),
    CONFIG_VARIABLE_INT          (gamepad_automap_clearmark,  gamepadautomapclearmark,       2),
    CONFIG_VARIABLE_INT          (gamepad_automap_followmode, gamepadautomapfollowmode,      2),
    CONFIG_VARIABLE_INT          (gamepad_automap_grid,       gamepadautomapgrid,            2),
    CONFIG_VARIABLE_INT          (gamepad_automap_mark,       gamepadautomapmark,            2),
    CONFIG_VARIABLE_INT          (gamepad_automap_maxzoom,    gamepadautomapmaxzoom,         2),
    CONFIG_VARIABLE_INT          (gamepad_automap_rotatemode, gamepadautomaprotatemode,      2),
    CONFIG_VARIABLE_INT          (gamepad_automap_zoomin,     gamepadautomapzoomin,          2),
    CONFIG_VARIABLE_INT          (gamepad_automap_zoomout,    gamepadautomapzoomout,         2),
    CONFIG_VARIABLE_INT          (gamepad_fire,               gamepadfire,                   2),
    CONFIG_VARIABLE_FLOAT_PERCENT(gamepad_leftdeadzone,       gamepadleftdeadzone_percent,   0),
    CONFIG_VARIABLE_FLOAT_PERCENT(gamepad_rightdeadzone,      gamepadrightdeadzone_percent,  0),
    CONFIG_VARIABLE_INT          (gamepad_lefthanded,         gamepadlefthanded,             1),
    CONFIG_VARIABLE_INT          (gamepad_menu,               gamepadmenu,                   2),
    CONFIG_VARIABLE_INT          (gamepad_prevweapon,         gamepadprevweapon,             2),
    CONFIG_VARIABLE_INT          (gamepad_nextweapon,         gamepadnextweapon,             2),
    CONFIG_VARIABLE_INT          (gamepad_run,                gamepadrun,                    2),
    CONFIG_VARIABLE_INT          (gamepad_sensitivity,        gamepadsensitivity,            0),
    CONFIG_VARIABLE_INT          (gamepad_use,                gamepaduse,                    2),
    CONFIG_VARIABLE_INT          (gamepad_vibrate,            gamepadvibrate,               15),
    CONFIG_VARIABLE_INT          (gamepad_weapon1,            gamepadweapon1,                2),
    CONFIG_VARIABLE_INT          (gamepad_weapon2,            gamepadweapon2,                2),
    CONFIG_VARIABLE_INT          (gamepad_weapon3,            gamepadweapon3,                2),
    CONFIG_VARIABLE_INT          (gamepad_weapon4,            gamepadweapon4,                2),
    CONFIG_VARIABLE_INT          (gamepad_weapon5,            gamepadweapon5,                2),
    CONFIG_VARIABLE_INT          (gamepad_weapon6,            gamepadweapon6,                2),
    CONFIG_VARIABLE_INT          (gamepad_weapon7,            gamepadweapon7,                2),
    CONFIG_VARIABLE_FLOAT        (gammacorrectionlevel,       gammalevel,                   12),
    CONFIG_VARIABLE_INT          (graphicdetail,              graphicdetail,                 6),
    CONFIG_VARIABLE_INT          (grid,                       grid,                          1),
    CONFIG_VARIABLE_INT          (homindicator,               homindicator,                  1),
    CONFIG_VARIABLE_INT          (hud,                        hud,                           1),
    CONFIG_VARIABLE_STRING       (iwadfolder,                 iwadfolder,                    0),
    CONFIG_VARIABLE_KEY          (key_automap,                key_automap,                   3),
    CONFIG_VARIABLE_KEY          (key_automap_clearmark,      key_automap_clearmark,         3),
    CONFIG_VARIABLE_KEY          (key_automap_followmode,     key_automap_followmode,        3),
    CONFIG_VARIABLE_KEY          (key_automap_grid,           key_automap_grid,              3),
    CONFIG_VARIABLE_KEY          (key_automap_mark,           key_automap_mark,              3),
    CONFIG_VARIABLE_KEY          (key_automap_maxzoom,        key_automap_maxzoom,           3),
    CONFIG_VARIABLE_KEY          (key_automap_rotatemode,     key_automap_rotatemode,        3),
    CONFIG_VARIABLE_KEY          (key_automap_zoomin,         key_automap_zoomin,            3),
    CONFIG_VARIABLE_KEY          (key_automap_zoomout,        key_automap_zoomout,           3),
    CONFIG_VARIABLE_KEY          (key_down,                   key_down,                      3),
    CONFIG_VARIABLE_KEY          (key_down2,                  key_down2,                     3),
    CONFIG_VARIABLE_KEY          (key_fire,                   key_fire,                      3),
    CONFIG_VARIABLE_KEY          (key_left,                   key_left,                      3),
    CONFIG_VARIABLE_KEY          (key_prevweapon,             key_prevweapon,                3),
    CONFIG_VARIABLE_KEY          (key_nextweapon,             key_nextweapon,                3),
    CONFIG_VARIABLE_KEY          (key_right,                  key_right,                     3),
    CONFIG_VARIABLE_KEY          (key_run,                    key_run,                       3),
    CONFIG_VARIABLE_KEY          (key_strafe,                 key_strafe,                    3),
    CONFIG_VARIABLE_KEY          (key_strafeleft,             key_strafeleft,                3),
    CONFIG_VARIABLE_KEY          (key_strafeleft2,            key_strafeleft2,               3),
    CONFIG_VARIABLE_KEY          (key_straferight,            key_straferight,               3),
    CONFIG_VARIABLE_KEY          (key_straferight2,           key_straferight2,              3),
    CONFIG_VARIABLE_KEY          (key_up,                     key_up,                        3),
    CONFIG_VARIABLE_KEY          (key_up2,                    key_up2,                       3),
    CONFIG_VARIABLE_KEY          (key_use,                    key_use,                       3),
    CONFIG_VARIABLE_KEY          (key_weapon1,                key_weapon1,                   3),
    CONFIG_VARIABLE_KEY          (key_weapon2,                key_weapon2,                   3),
    CONFIG_VARIABLE_KEY          (key_weapon3,                key_weapon3,                   3),
    CONFIG_VARIABLE_KEY          (key_weapon4,                key_weapon4,                   3),
    CONFIG_VARIABLE_KEY          (key_weapon5,                key_weapon5,                   3),
    CONFIG_VARIABLE_KEY          (key_weapon6,                key_weapon6,                   3),
    CONFIG_VARIABLE_KEY          (key_weapon7,                key_weapon7,                   3),
    CONFIG_VARIABLE_INT          (mapfixes,                   mapfixes,                      1),
    CONFIG_VARIABLE_INT          (messages,                   messages,                      1),
    CONFIG_VARIABLE_INT          (mirrorweapons,              mirrorweapons,                 1),
    CONFIG_VARIABLE_FLOAT        (mouse_acceleration,         mouse_acceleration,            0),
    CONFIG_VARIABLE_INT          (mouse_fire,                 mousebfire,                    4),
    CONFIG_VARIABLE_INT          (mouse_forward,              mousebforward,                 4),
    CONFIG_VARIABLE_INT          (mouse_prevweapon,           mousebprevweapon,              4),
    CONFIG_VARIABLE_INT          (mouse_nextweapon,           mousebnextweapon,              4),
    CONFIG_VARIABLE_INT          (mouse_sensitivity,          mousesensitivity,              0),
    CONFIG_VARIABLE_INT          (mouse_strafe,               mousebstrafe,                  4),
    CONFIG_VARIABLE_INT          (mouse_threshold,            mouse_threshold,               0),
    CONFIG_VARIABLE_INT          (mouse_use,                  mousebuse,                     4),
    CONFIG_VARIABLE_INT_PERCENT  (musicvolume,                musicvolume_percent,           0),
    CONFIG_VARIABLE_INT          (novert,                     novert,                        1),
    CONFIG_VARIABLE_INT          (pixelwidth,                 pixelwidth,                    0),
    CONFIG_VARIABLE_INT          (pixelheight,                pixelheight,                   0),
    CONFIG_VARIABLE_INT_PERCENT  (playerbob,                  playerbob,                     0),
    CONFIG_VARIABLE_INT          (rotatemode,                 rotatemode,                    1),
    CONFIG_VARIABLE_INT          (runcount,                   runcount,                      0),
    CONFIG_VARIABLE_INT          (savegame,                   selectedsavegame,              0),
#if defined(SDL20)
    CONFIG_VARIABLE_STRING       (scalequality,               scalequality,                  0),
#endif
    CONFIG_VARIABLE_INT          (screensize,                 screensize,                    0),
    CONFIG_VARIABLE_INT          (screenwidth,                screenwidth,                   5),
    CONFIG_VARIABLE_INT          (screenheight,               screenheight,                  5),
    CONFIG_VARIABLE_INT_PERCENT  (sfxvolume,                  sfxvolume_percent,             0),
    CONFIG_VARIABLE_INT          (shadows,                    shadows,                       1),
    CONFIG_VARIABLE_INT          (skilllevel,                 selectedskilllevel,           10),
    CONFIG_VARIABLE_INT          (smoketrails,                smoketrails,                   1),
    CONFIG_VARIABLE_INT          (snd_maxslicetime_ms,        snd_maxslicetime_ms,           0),
    CONFIG_VARIABLE_STRING       (timidity_cfg_path,          timidity_cfg_path,             0),
    CONFIG_VARIABLE_INT          (translucency,               translucency,                  1),
    CONFIG_VARIABLE_STRING       (videodriver,                videodriver,                   0),
#if defined(SDL20)
    CONFIG_VARIABLE_INT          (vsync,                      vsync,                         1),
#endif
    CONFIG_VARIABLE_INT          (widescreen,                 widescreen,                    1),
    CONFIG_VARIABLE_STRING       (windowposition,             windowposition,                0),
    CONFIG_VARIABLE_INT          (windowwidth,                windowwidth,                   0),
    CONFIG_VARIABLE_INT          (windowheight,               windowheight,                  0)
};

default_collection_t doom_defaults =
{
    doom_defaults_list,
    arrlen(doom_defaults_list),
    NULL
};

#define INVALIDKEY      -1

static const int scantokey[128] =
{
    0,             27,             '1',           '2',
    '3',           '4',            '5',           '6',
    '7',           '8',            '9',           '0',
    '-',           '=',            KEY_BACKSPACE, 9,
    'q',           'w',            'e',           'r',
    't',           'y',            'u',           'i',
    'o',           'p',            '[',           ']',
    13,            KEY_RCTRL,      'a',           's',
    'd',           'f',            'g',           'h',
    'j',           'k',            'l',           ';',
    '\'',          '`',            KEY_RSHIFT,    '\\',
    'z',           'x',            'c',           'v',
    'b',           'n',            'm',           ',',
    '.',           '/',            KEY_RSHIFT,    KEYP_MULTIPLY,
    KEY_RALT,      ' ',            INVALIDKEY,    INVALIDKEY,
    INVALIDKEY,    INVALIDKEY,     INVALIDKEY,    INVALIDKEY,
    INVALIDKEY,    INVALIDKEY,     INVALIDKEY,    INVALIDKEY,
    INVALIDKEY,    KEY_PAUSE,      KEY_SCRLCK,    KEY_HOME,
    KEY_UPARROW,   KEY_PGUP,       KEY_MINUS,     KEY_LEFTARROW,
    KEYP_5,        KEY_RIGHTARROW, KEYP_PLUS,     KEY_END,
    KEY_DOWNARROW, KEY_PGDN,       KEY_INS,       KEY_DEL,
    0,             0,              0,             INVALIDKEY,
    INVALIDKEY,    0,              0,             0,
    0,             0,              0,             0,
    0,             0,              0,             0,
    0,             0,              0,             0,
    0,             0,              0,             0,
    0,             0,              0,             0,
    0,             0,              0,             0,
    0,             0,              0,             0,
    0,             0,              0,             0,
    0,             0,              0,             0
};

static alias_t alias[] =
{
    { "off",                                    0,  1 },
    { "on",                                     1,  1 },
    { "no",                                     0,  1 },
    { "yes",                                    1,  1 },
    { "false",                                  0,  1 },
    { "true",                                   1,  1 },
    { "-",                                      0,  2 },
    { "none",                                   0,  2 },
    { "dpadup",                                 1,  2 },
    { "dpaddown",                               2,  2 },
    { "dpadleft",                               4,  2 },
    { "dpadright",                              8,  2 },
    { "start",                                 16,  2 },
    { "back",                                  32,  2 },
    { "leftthumb",                             64,  2 },
    { "rightthumb",                           128,  2 },
    { "leftshoulder",                         256,  2 },
    { "LS",                                   256,  2 },
    { "leftbutton",                           256,  2 },
    { "LB",                                   256,  2 },
    { "rightshoulder",                        512,  2 },
    { "RS",                                   512,  2 },
    { "rightbutton",                          512,  2 },
    { "RB",                                   512,  2 },
    { "lefttrigger",                         1024,  2 },
    { "LT",                                  1024,  2 },
    { "righttrigger",                        2048,  2 },
    { "RT",                                  2048,  2 },
    { "A",                                   4096,  2 },
    { "B",                                   8192,  2 },
    { "X",                                  16384,  2 },
    { "Y",                                  32768,  2 },
    { "-",                                      0,  3 },
    { "none",                                   0,  3 },
    { "\'+\'",                                 13,  3 },
    { "backspace",                             14,  3 },
    { "tab",                                   15,  3 },
    { "enter",                                 28,  3 },
    { "ctrl",                                  29,  3 },
    { "shift",                                 42,  3 },
    { "alt",                                   56,  3 },
    { "space",                                 57,  3 },
    { "home",                                  71,  3 },
    { "up",                                    72,  3 },
    { "pageup",                                73,  3 },
    { "left",                                  75,  3 },
    { "right",                                 77,  3 },
    { "end",                                   79,  3 },
    { "down",                                  80,  3 },
    { "pagedown",                              81,  3 },
    { "insert",                                82,  3 },
    { "del",                                   83,  3 },
    { "-",                                     -1,  4 },
    { "none",                                  -1,  4 },
    { "left",                                   0,  4 },
    { "mouse1",                                 0,  4 },
    { "middle",                                 1,  4 },
    { "mouse2",                                 1,  4 },
    { "right",                                  2,  4 },
    { "mouse3",                                 2,  4 },
#if !defined(SDL20)
    { "wheelup",                                3,  4 },
    { "wheeldown",                              4,  4 },
#endif
    { "mouse4",                                 3,  4 },
    { "mouse5",                                 4,  4 },
    { "mouse6",                                 5,  4 },
    { "mouse7",                                 6,  4 },
    { "mouse8",                                 7,  4 },
#if defined(SDL20)
    { "wheelup",                                8,  4 },
    { "wheeldown",                              9,  4 },
#endif
    { "desktop",                                0,  5 },
    { "low",                                    0,  6 },
    { "high",                                   1,  6 },
    { "-",                                      0,  7 },
    { "none",                                   0,  7 },
    { "off",                                    0,  7 },
    { "no",                                     0,  7 },
    { "false",                                  0,  7 },
    { "unlimited",                          32768,  7 },
    { "on",                                 32768,  7 },
    { "yes",                                32768,  7 },
    { "true",                               32768,  7 },
    { "\"Knee-Deep in the Dead\"",              0,  8 },
    { "\"The Shores of Hell\"",                 1,  8 },
    { "\"Inferno\"",                            2,  8 },
    { "\"Thy Flesh Consumed\"",                 3,  8 },
    { "\"Hell on Earth\"",                      0,  9 },
    { "\"No Rest for the Living\"",             1,  9 },
    { "\"I\'m too young to die.\"",             0, 10 },
    { "\"Hey, not too rough.\"",                1, 10 },
    { "\"Hurt me plenty.\"",                    2, 10 },
    { "\"Ultra-Violence.\"",                    3, 10 },
    { "\"Nightmare!\"",                         4, 10 },
    { "off",                                    1, 12 },
    { "-",                                      0, 15 },
    { "none",                                   0, 15 },
    { "damage",                                 1, 15 },
    { "weapons",                                2, 15 },
    { "damage|weapons",                         3, 15 },
    { "weapons|damage",                         3, 15 },
    { "",                                       0,  0 }
};

char *striptrailingzero(float value)
{
    size_t      len;
    static char result[100];
    
    M_snprintf(result, 100, "%.1f", value);
    len = strlen(result);
    if (len > 2 && result[len - 2] == '.' && result[len - 1] == '0')
        result[len - 2] = '\0';
    return result;
}

static void SaveDefaultCollection(default_collection_t *collection)
{
    default_t   *defaults;
    int         i;
    FILE        *f = fopen(collection->filename, "w");

    if (!f)
        return; // can't write the file, but don't complain

    defaults = collection->defaults;

    for (i = 0; i < collection->numdefaults; i++)
    {
        int     chars_written;

        // Print the name and line up all values at 30 characters
        chars_written = fprintf(f, "%s ", defaults[i].name);

        for (; chars_written < 30; ++chars_written)
            fprintf(f, " ");

        // Print the value
        switch (defaults[i].type)
        {
            case DEFAULT_KEY:
            {
                // use the untranslated version if we can, to reduce
                // the possibility of screwing up the user's config
                // file
                int     v = *(int *)defaults[i].location;

                if (defaults[i].untranslated && v == defaults[i].original_translated)
                {
                    // Has not been changed since the last time we
                    // read the config file.
                    int         j = 0;
                    boolean     flag = false;

                    v = defaults[i].untranslated;
                    while (alias[j].text[0])
                    {
                        if (v == alias[j].value && defaults[i].set == alias[j].set)
                        {
                            fprintf(f, "%s", alias[j].text);
                            flag = true;
                            break;
                        }
                        j++;
                    }
                    if (flag)
                        break;

                    if (isprint(scantokey[v]))
                        fprintf(f, "\'%c\'", scantokey[v]);
                    else
                        fprintf(f, "%i", v);
                }
                else
                {
                    // search for a reverse mapping back to a scancode
                    // in the scantokey table
                    int         s;

                    for (s = 0; s < 128; ++s)
                    {
                        if (scantokey[s] == v)
                        {
                            int         j = 0;
                            boolean     flag = false;

                            v = s;
                            while (alias[j].text[0])
                            {
                                if (v == alias[j].value && defaults[i].set == alias[j].set)
                                {
                                    fprintf(f, "%s", alias[j].text);
                                    flag = true;
                                    break;
                                }
                                j++;
                            }
                            if (flag)
                                break;

                            if (isprint(scantokey[v]))
                                fprintf(f, "\'%c\'", scantokey[v]);
                            else
                                fprintf(f, "%i", v);
                            break;
                        }
                    }
                }

                break;
            }

            case DEFAULT_INT:
            {
                int         j = 0;
                boolean     flag = false;
                int         v = *(int *)defaults[i].location;

                while (alias[j].text[0])
                {
                    if (v == alias[j].value && defaults[i].set == alias[j].set)
                    {
                        fprintf(f, "%s", alias[j].text);
                        flag = true;
                        break;
                    }
                    j++;
                }
                if (!flag)
                    fprintf(f, "%i", *(int *)defaults[i].location);
                break;
            }

            case DEFAULT_INT_HEX:
                fprintf(f, "0x%x", *(int *)defaults[i].location);
                break;

            case DEFAULT_INT_PERCENT:
            {
                int         j = 0;
                boolean     flag = false;
                int         v = *(int *)defaults[i].location;

                while (alias[j].text[0])
                {
                    if (v == alias[j].value && defaults[i].set == alias[j].set)
                    {
                        fprintf(f, "%s", alias[j].text);
                        flag = true;
                        break;
                    }
                    j++;
                }
                if (!flag)
                    fprintf(f, "%i%%", *(int *)defaults[i].location);
                break;
            }

            case DEFAULT_FLOAT:
            {
                int         j = 0;
                boolean     flag = false;
                float       v = *(float *)defaults[i].location;

                while (alias[j].text[0])
                {
                    if (v == alias[j].value && defaults[i].set == alias[j].set)
                    {
                        fprintf(f, "%s", alias[j].text);
                        flag = true;
                        break;
                    }
                    j++;
                }
                if (!flag)
                    fprintf(f, "%.2f", *(float *)defaults[i].location);
                break;
            }

            case DEFAULT_FLOAT_PERCENT:
            {
                int         j = 0;
                boolean     flag = false;
                float       v = *(float *)defaults[i].location;

                while (alias[j].text[0])
                {
                    if (v == alias[j].value && defaults[i].set == alias[j].set)
                    {
                        fprintf(f, "%s", alias[j].text);
                        flag = true;
                        break;
                    }
                    j++;
                }
                if (!flag)
                    fprintf(f, "%s%%", striptrailingzero(*(float *)defaults[i].location));
                break;
            }

            case DEFAULT_STRING:
                fprintf(f, "\"%s\"", *(char **)defaults[i].location);
                break;
        }

        fprintf(f, "\n");
    }

    fclose(f);
}

// Parses integer values in the configuration file
static int ParseIntParameter(char *strparm, int set)
{
    int parm;
    int i = 0;

    while (alias[i].text[0])
    {
        if (!strcasecmp(strparm, alias[i].text) && set == alias[i].set)
            return alias[i].value;
        i++;
    }

    if (strparm[0] == '\'' && strparm[2] == '\'')
        for (i = 0; i < 128; ++i)
            if (tolower(strparm[1]) == scantokey[i])
                return i;

    if (strparm[0] == '0' && strparm[1] == 'x')
        sscanf(strparm + 2, "%x", &parm);
    else
        sscanf(strparm, "%i", &parm);

    return parm;
}

// Parses float values in the configuration file
static float ParseFloatParameter(char *strparm, int set)
{
    int     i = 0;

    while (alias[i].text[0])
    {
        if (!strcasecmp(strparm, alias[i].text) && set == alias[i].set)
            return (float)alias[i].value;
        i++;
    }

    return (float)atof(strparm);
}

static void LoadDefaultCollection(default_collection_t *collection)
{
    default_t   *defaults = collection->defaults;
    int         i;
    FILE        *f;
    char        defname[80];
    char        strparm[100];

    // read the file in, overriding any set defaults
    f = fopen(collection->filename, "r");

    if (!f)
        // File not opened, but don't complain
        return;

    while (!feof(f))
    {
        if (fscanf(f, "%79s %[^\n]\n", defname, strparm) != 2)
            // This line doesn't match
            continue;

        // Strip off trailing non-printable characters (\r characters
        // from DOS text files)
        while (strlen(strparm) > 0 && !isprint(strparm[strlen(strparm) - 1]))
            strparm[strlen(strparm) - 1] = '\0';

        // Find the setting in the list
        for (i = 0; i < collection->numdefaults; ++i)
        {
            default_t   *def = &collection->defaults[i];
            char        *s;
            int         intparm;

            if (strcmp(defname, def->name) != 0)
                continue;       // not this one

            // parameter found
            switch (def->type)
            {
                case DEFAULT_STRING:
                    s = strdup(strparm + 1);
                    s[strlen(s) - 1] = '\0';
                    *(char **)def->location = s;
                    break;

                case DEFAULT_INT:
                case DEFAULT_INT_HEX:
                    *(int *)def->location = ParseIntParameter(strparm, def->set);
                    break;

                case DEFAULT_INT_PERCENT:
                    s = strdup(strparm);
                    if (s[strlen(s) - 1] == '%')
                        s[strlen(s) - 1] = '\0';
                    *(int *)def->location = ParseIntParameter(s, def->set);
                    break;

                case DEFAULT_KEY:
                    // translate scancodes read from config
                    // file (save the old value in untranslated)
                    intparm = ParseIntParameter(strparm, def->set);
                    defaults[i].untranslated = intparm;
                    intparm = (intparm >= 0 && intparm < 128 ? scantokey[intparm] : INVALIDKEY);

                    defaults[i].original_translated = intparm;
                    *(int *)def->location = intparm;
                    break;

                case DEFAULT_FLOAT:
                    *(float *)def->location = ParseFloatParameter(strparm, def->set);
                    break;

                case DEFAULT_FLOAT_PERCENT:
                    s = strdup(strparm);
                    if (s[strlen(s) - 1] == '%')
                        s[strlen(s) - 1] = '\0';
                    *(float *)def->location = ParseFloatParameter(strparm, def->set);
                    break;
            }

            // finish
            break;
        }
    }

    fclose(f);
}

//
// M_SaveDefaults
//
void M_SaveDefaults(void)
{
    if (returntowidescreen)
        widescreen = true;
    SaveDefaultCollection(&doom_defaults);
    if (returntowidescreen)
        widescreen = false;
}

static void M_CheckDefaults(void)
{
    if (alwaysrun != false && alwaysrun != true)
        alwaysrun = ALWAYSRUN_DEFAULT;

    if (animatedliquid != false && animatedliquid != true)
        animatedliquid = ANIMATEDLIQUID_DEFAULT;

    bloodsplats = BETWEEN(BLOODSPLATS_MIN, bloodsplats, BLOODSPLATS_MAX);

    if (brightmaps != false && brightmaps != true)
        brightmaps = BRIGHTMAPS_DEFAULT;

    if (centerweapon != false && centerweapon != true)
        centerweapon = CENTERWEAPON_DEFAULT;

    if (corpses_mirror != false && corpses_mirror != true)
        corpses_mirror = CORPSES_MIRROR_DEFAULT;

    if (corpses_moreblood != false && corpses_moreblood != true)
        corpses_moreblood = CORPSES_MOREBLOOD_DEFAULT;

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

    if (fullscreen != false && fullscreen != true)
        fullscreen = FULLSCREEN_DEFAULT;

    if (gamepadautomap < 0 || gamepadautomap > GAMEPAD_Y
        || (gamepadautomap & (gamepadautomap - 1)))
        gamepadautomap = GAMEPADAUTOMAP_DEFAULT;

    if (gamepadautomapclearmark < 0 || gamepadautomapclearmark > GAMEPAD_Y
        || (gamepadautomapclearmark & (gamepadautomapclearmark - 1)))
        gamepadautomapclearmark = GAMEPADAUTOMAPCLEARMARK_DEFAULT;

    if (gamepadautomapfollowmode < 0 || gamepadautomapfollowmode > GAMEPAD_Y
        || (gamepadautomapfollowmode & (gamepadautomapfollowmode - 1)))
        gamepadautomapfollowmode = GAMEPADAUTOMAPFOLLOWMODE_DEFAULT;

    if (gamepadautomapgrid < 0 || gamepadautomapgrid > GAMEPAD_Y
        || (gamepadautomapgrid & (gamepadautomapgrid - 1)))
        gamepadautomapgrid = GAMEPADAUTOMAPGRID_DEFAULT;

    if (gamepadautomapmark < 0 || gamepadautomapmark > GAMEPAD_Y
        || (gamepadautomapmark & (gamepadautomapmark - 1)))
        gamepadautomapmark = GAMEPADAUTOMAPMARK_DEFAULT;

    if (gamepadautomapmaxzoom < 0 || gamepadautomapmaxzoom > GAMEPAD_Y
        || (gamepadautomapmaxzoom & (gamepadautomapmaxzoom - 1)))
        gamepadautomapmaxzoom = GAMEPADAUTOMAPMAXZOOM_DEFAULT;

    if (gamepadautomaprotatemode < 0 || gamepadautomaprotatemode > GAMEPAD_Y
        || (gamepadautomaprotatemode & (gamepadautomaprotatemode - 1)))
        gamepadautomaprotatemode = GAMEPADAUTOMAPROTATEMODE_DEFAULT;

    if (gamepadautomapzoomin < 0 || gamepadautomapzoomin > GAMEPAD_Y
        || (gamepadautomapzoomin & (gamepadautomapzoomin - 1)))
        gamepadautomapzoomin = GAMEPADAUTOMAPZOOMIN_DEFAULT;

    if (gamepadautomapzoomout < 0 || gamepadautomapzoomout > GAMEPAD_Y
        || (gamepadautomapzoomout & (gamepadautomapzoomout - 1)))
        gamepadautomapzoomout = GAMEPADAUTOMAPZOOMOUT_DEFAULT;

    if (gamepadfire < 0 || gamepadfire > GAMEPAD_Y || (gamepadfire & (gamepadfire - 1)))
        gamepadfire = GAMEPADFIRE_DEFAULT;

    gamepadleftdeadzone = (int)(BETWEENF(GAMEPADLEFTDEADZONE_MIN, gamepadleftdeadzone_percent,
        GAMEPADLEFTDEADZONE_MAX) * (float)SHRT_MAX / 100.0f);

    gamepadrightdeadzone = (int)(BETWEENF(GAMEPADRIGHTDEADZONE_MIN, gamepadrightdeadzone_percent,
        GAMEPADRIGHTDEADZONE_MAX) * (float)SHRT_MAX / 100.0f);

    if (gamepadlefthanded != false && gamepadlefthanded != true)
        gamepadlefthanded = GAMEPADLEFTHANDED_DEFAULT;

    if (gamepadmenu < 0 || gamepadmenu > GAMEPAD_Y || (gamepadmenu & (gamepadmenu - 1)))
        gamepadmenu = GAMEPADMENU_DEFAULT;

    if (gamepadnextweapon < 0 || gamepadnextweapon > GAMEPAD_Y
        || (gamepadnextweapon & (gamepadnextweapon - 1)))
        gamepadnextweapon = GAMEPADNEXTWEAPON_DEFAULT;

    if (gamepadprevweapon < 0 || gamepadprevweapon > GAMEPAD_Y
        || (gamepadprevweapon & (gamepadprevweapon - 1)))
        gamepadprevweapon = GAMEPADPREVWEAPON_DEFAULT;

    gamepadsensitivity = BETWEEN(GAMEPADSENSITIVITY_MIN, gamepadsensitivity,
        GAMEPADSENSITIVITY_MAX);
    gamepadsensitivityf = (!gamepadsensitivity ? 0.0f :
        GAMEPADSENSITIVITY_OFFSET + gamepadsensitivity / (float)GAMEPADSENSITIVITY_MAX *
        GAMEPADSENSITIVITY_FACTOR);

    if (gamepadrun < 0 || gamepadrun > GAMEPAD_Y || (gamepadrun & (gamepadrun - 1)))
        gamepadrun = GAMEPADRUN_DEFAULT;

    if (gamepaduse < 0 || gamepaduse > GAMEPAD_Y || (gamepaduse & (gamepaduse - 1)))
        gamepaduse = GAMEPADUSE_DEFAULT;

    if (gamepadvibrate != false && gamepadvibrate != true)
        gamepadvibrate = GAMEPADVIBRATE_DEFAULT;

    if (gamepadweapon1 < 0 || gamepadweapon1 > GAMEPAD_Y || (gamepadweapon1 & (gamepadweapon1 - 1)))
        gamepadweapon1 = GAMEPADWEAPON_DEFAULT;

    if (gamepadweapon2 < 0 || gamepadweapon2 > GAMEPAD_Y || (gamepadweapon2 & (gamepadweapon2 - 1)))
        gamepadweapon2 = GAMEPADWEAPON_DEFAULT;

    if (gamepadweapon3 < 0 || gamepadweapon3 > GAMEPAD_Y || (gamepadweapon3 & (gamepadweapon3 - 1)))
        gamepadweapon3 = GAMEPADWEAPON_DEFAULT;

    if (gamepadweapon4 < 0 || gamepadweapon4 > GAMEPAD_Y || (gamepadweapon4 & (gamepadweapon4 - 1)))
        gamepadweapon4 = GAMEPADWEAPON_DEFAULT;

    if (gamepadweapon5 < 0 || gamepadweapon5 > GAMEPAD_Y || (gamepadweapon5 & (gamepadweapon5 - 1)))
        gamepadweapon5 = GAMEPADWEAPON_DEFAULT;

    if (gamepadweapon6 < 0 || gamepadweapon6 > GAMEPAD_Y || (gamepadweapon6 & (gamepadweapon6 - 1)))
        gamepadweapon6 = GAMEPADWEAPON_DEFAULT;

    if (gamepadweapon7 < 0 || gamepadweapon7 > GAMEPAD_Y || (gamepadweapon7 & (gamepadweapon7 - 1)))
        gamepadweapon7 = GAMEPADWEAPON_DEFAULT;

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

    if (grid != false && grid != true)
        grid = GRID_DEFAULT;

    if (homindicator != false && homindicator != true)
        homindicator = HOMINDICATOR_DEFAULT;

    if (hud != false && hud != true)
        hud = HUD_DEFAULT;

    if (key_automap == INVALIDKEY)
        key_automap = KEYAUTOMAP_DEFAULT;

    if (key_automap_clearmark == INVALIDKEY)
        key_automap_clearmark = KEYAUTOMAPCLEARMARK_DEFAULT;

    if (key_automap_followmode == INVALIDKEY)
        key_automap_followmode = KEYAUTOMAPFOLLOWMODE_DEFAULT;

    if (key_automap_grid == INVALIDKEY)
        key_automap_grid = KEYAUTOMAPGRID_DEFAULT;

    if (key_automap_mark == INVALIDKEY)
        key_automap_mark = KEYAUTOMAPMARK_DEFAULT;

    if (key_automap_maxzoom == INVALIDKEY)
        key_automap_maxzoom = KEYAUTOMAPMAXZOOM_DEFAULT;

    if (key_automap_rotatemode == INVALIDKEY)
        key_automap_rotatemode = KEYAUTOMAPROTATEMODE_DEFAULT;

    if (key_automap_zoomin == INVALIDKEY)
        key_automap_zoomin = KEYAUTOMAPZOOMIN_DEFAULT;

    if (key_automap_zoomout == INVALIDKEY)
        key_automap_zoomout = KEYAUTOMAPZOOMOUT_DEFAULT;

    if (key_down == INVALIDKEY)
        key_down = KEYDOWN_DEFAULT;

    if (key_down2 == INVALIDKEY)
        key_down2 = KEYDOWN2_DEFAULT;

    if (key_fire == INVALIDKEY)
        key_fire = KEYFIRE_DEFAULT;

    if (key_left == INVALIDKEY)
        key_left = KEYLEFT_DEFAULT;

    if (key_nextweapon == INVALIDKEY)
        key_nextweapon = KEYNEXTWEAPON_DEFAULT;

    if (key_prevweapon == INVALIDKEY)
        key_prevweapon = KEYPREVWEAPON_DEFAULT;

    if (key_right == INVALIDKEY)
        key_right = KEYRIGHT_DEFAULT;

    if (key_run == INVALIDKEY)
        key_run = KEYRUN_DEFAULT;

    if (key_strafe == INVALIDKEY)
        key_strafe = KEYSTRAFE_DEFAULT;

    if (key_strafeleft == INVALIDKEY)
        key_strafeleft = KEYSTRAFELEFT_DEFAULT;

    if (key_strafeleft2 == INVALIDKEY)
        key_strafeleft2 = KEYSTRAFELEFT2_DEFAULT;

    if (key_straferight == INVALIDKEY)
        key_straferight = KEYSTRAFERIGHT_DEFAULT;

    if (key_straferight2 == INVALIDKEY)
        key_straferight2 = KEYSTRAFERIGHT2_DEFAULT;

    if (key_up == INVALIDKEY)
        key_up = KEYUP_DEFAULT;

    if (key_up2 == INVALIDKEY)
        key_up2 = KEYUP2_DEFAULT;

    if (key_use == INVALIDKEY)
        key_use = KEYUSE_DEFAULT;

    if (key_weapon1 == INVALIDKEY)
        key_weapon1 = KEYWEAPON1_DEFAULT;

    if (key_weapon2 == INVALIDKEY)
        key_weapon2 = KEYWEAPON2_DEFAULT;

    if (key_weapon3 == INVALIDKEY)
        key_weapon3 = KEYWEAPON3_DEFAULT;

    if (key_weapon4 == INVALIDKEY)
        key_weapon4 = KEYWEAPON4_DEFAULT;

    if (key_weapon5 == INVALIDKEY)
        key_weapon5 = KEYWEAPON5_DEFAULT;

    if (key_weapon6 == INVALIDKEY)
        key_weapon6 = KEYWEAPON6_DEFAULT;

    if (key_weapon7 == INVALIDKEY)
        key_weapon7 = KEYWEAPON7_DEFAULT;

    if (mapfixes != false && mapfixes != true)
        mapfixes = MAPFIXES_DEFAULT;

    if (messages != false && messages != true)
        messages = MESSAGES_DEFAULT;

    if (mirrorweapons != false && mirrorweapons != true)
        mirrorweapons = MIRRORWEAPONS_DEFAULT;

    if (mousebfire < -1 || mousebfire > MAX_MOUSE_BUTTONS)
        mousebfire = MOUSEFIRE_DEFAULT;

    if (mousebforward < -1 || mousebforward > MAX_MOUSE_BUTTONS || mousebforward == mousebfire)
        mousebforward = MOUSEFORWARD_DEFAULT;

#if defined(SDL20)
    if (mousebprevweapon < -1 || mousebprevweapon > MAX_MOUSE_BUTTONS + 2
#else
    if (mousebprevweapon < -1 || mousebprevweapon > MAX_MOUSE_BUTTONS
#endif
        || mousebprevweapon == mousebfire || mousebprevweapon == mousebforward)
        mousebprevweapon = MOUSEPREVWEAPON_DEFAULT;

#if defined(SDL20)
    if (mousebnextweapon < -1 || mousebnextweapon > MAX_MOUSE_BUTTONS + 2
#else
    if (mousebnextweapon < -1 || mousebnextweapon > MAX_MOUSE_BUTTONS
#endif
        || mousebnextweapon == mousebfire || mousebnextweapon == mousebforward
        || mousebnextweapon == mousebprevweapon)
        mousebnextweapon = MOUSENEXTWEAPON_DEFAULT;

    mousesensitivity = BETWEEN(MOUSESENSITIVITY_MIN, mousesensitivity, MOUSESENSITIVITY_MAX);

    if (mousebstrafe < -1 || mousebstrafe > MAX_MOUSE_BUTTONS || mousebstrafe == mousebfire
        || mousebstrafe == mousebforward || mousebstrafe == mousebprevweapon
        || mousebstrafe == mousebnextweapon)
        mousebstrafe = MOUSESTRAFE_DEFAULT;

    if (mousebuse < -1 || mousebuse > MAX_MOUSE_BUTTONS || mousebuse == mousebfire
        || mousebuse == mousebforward || mousebuse == mousebprevweapon
        || mousebuse == mousebnextweapon || mousebuse == mousebstrafe)
        mousebuse = MOUSEUSE_DEFAULT;

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

    if (rotatemode != false && rotatemode != true)
        rotatemode = ROTATEMODE_DEFAULT;

    runcount = BETWEEN(0, runcount, RUNCOUNT_MAX);

    screensize = BETWEEN(SCREENSIZE_MIN, screensize, SCREENSIZE_MAX);

    if (screenwidth && screenheight
        && (screenwidth < SCREENWIDTH || screenheight < SCREENHEIGHT * 3 / 4))
    {
        screenwidth = SCREENWIDTH_DEFAULT;
        screenheight = SCREENHEIGHT_DEFAULT;
    }

    selectedepisode = BETWEEN(EPISODE_MIN, selectedepisode, EPISODE_MAX - (gamemode == registered));

    selectedexpansion = BETWEEN(EXPANSION_MIN, selectedexpansion, EXPANSION_MAX);

    selectedsavegame = BETWEEN(0, selectedsavegame, 5);

    selectedskilllevel = BETWEEN(SKILLLEVEL_MIN, selectedskilllevel, SKILLLEVEL_MAX);

    sfxVolume = (BETWEEN(SFXVOLUME_MIN, sfxvolume_percent, SFXVOLUME_MAX) * 15 + 50) / 100;

    if (shadows != false && shadows != true)
        shadows = SHADOWS_DEFAULT;

    if (smoketrails != false && smoketrails != true)
        smoketrails = SMOKETRAILS_DEFAULT;

    if (translucency != false && translucency != true)
        translucency = TRANSLUCENCY_DEFAULT;

#if defined(SDL20)
    if (vsync != false && vsync != true)
        vsync = VSYNC_DEFAULT;
#endif

    if (widescreen != false && widescreen != true)
        widescreen = WIDESCREEN_DEFAULT;
    if (widescreen || screensize == SCREENSIZE_MAX)
    {
        returntowidescreen = true;
        widescreen = false;
    }
    else
        hud = true;

    if (windowwidth < SCREENWIDTH || windowheight < SCREENWIDTH * 3 / 4)
        windowheight = WINDOWHEIGHT_DEFAULT;
    windowwidth = windowheight * 4 / 3;

    M_SaveDefaults();
}

//
// M_LoadDefaults
//
void M_LoadDefaults(void)
{
    int i;

    // check for a custom default file
    i = M_CheckParmWithArgs("-config", 1);

    if (i)
        doom_defaults.filename = myargv[i + 1];
    else
        doom_defaults.filename = PACKAGE_CONFIG;

    LoadDefaultCollection(&doom_defaults);
    M_CheckDefaults();
}
