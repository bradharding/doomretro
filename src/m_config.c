/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifdef WIN32
#include <ShlObj.h>
#endif

#include "doomstat.h"
#include "m_argv.h"
#include "m_menu.h"
#include "m_misc.h"

//
// DEFAULTS
//

// Location where all configuration data is stored -
// doomretro.cfg, savegames, etc.
char            *configdir;

extern int      bloodsplats;
extern boolean  brightmaps;
extern int      corpses;
extern boolean  dclick_use;
extern int      fullscreen;
extern int      gamepadautomap;
extern int      gamepadfire;
extern int      gamepadlefthanded;
extern int      gamepadmenu;
extern int      gamepadnextweapon;
extern int      gamepadprevweapon;
extern int      gamepadspeed;
extern int      gamepaduse;
extern int      gamepadvibrate;
extern int      gamepadweapon1;
extern int      gamepadweapon2;
extern int      gamepadweapon3;
extern int      gamepadweapon4;
extern int      gamepadweapon5;
extern int      gamepadweapon6;
extern int      gamepadweapon7;
extern float    gamma;
extern int      graphicdetail;
extern boolean  grid;
extern boolean  homindicator;
extern int      hud;
extern char     *iwadfolder;
extern int      key_down;
extern int      key_down2;
extern int      key_fire;
extern int      key_left;
extern int      key_nextweapon;
extern int      key_prevweapon;
extern int      key_right;
extern int      key_speed;
extern int      key_strafe;
extern int      key_strafeleft;
extern int      key_straferight;
extern int      key_up;
extern int      key_up2;
extern int      key_use;
extern boolean  messages;
extern boolean  mirrorweapons;
extern int      mouseSensitivity;
extern float    mouse_acceleration;
extern int      mouse_threshold;
extern int      mousebfire;
extern int      mousebforward;
extern int      mousebstrafe;
extern boolean  novert;
extern int      pixelheight;
extern int      pixelwidth;
extern int      playerbob;
extern boolean  rotate;
extern int      runcount;
extern float    saturation;
extern int      screenheight;
extern int      screenwidth;
extern int      selectedepisode;
extern int      selectedexpansion;
extern int      selectedsavegame;
extern int      selectedskilllevel;
extern boolean  smoketrails;
extern int      snd_maxslicetime_ms;
extern char     *timidity_cfg_path;
extern boolean  translucency;
extern char     *videodriver;
extern boolean  widescreen;
extern int      windowheight;
extern char     *windowposition;
extern int      windowwidth;
extern char     *xinput;

extern boolean  returntowidescreen;

typedef enum
{
    DEFAULT_INT,
    DEFAULT_INT_HEX,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
    DEFAULT_KEY
} default_type_t;

typedef struct
{
    // Name of the variable
    char                *name;

    // Pointer to the location in memory of the variable
    void                *location;

    // Type of the variable
    default_type_t      type;

    // If this is a key value, the original integer scancode we read from
    // the config file before translating it to the internal key value.
    // If zero, we didn't read this value from a config file.
    int                 untranslated;

    // The value we translated the scancode into when we read the
    // config file on startup.  If the variable value is different from
    // this, it has been changed and needs to be converted; otherwise,
    // use the 'untranslated' value.
    int                 original_translated;

    int                 set;
} default_t;

typedef struct
{
    default_t           *defaults;
    int                 numdefaults;
    char                *filename;
} default_collection_t;

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
#define CONFIG_VARIABLE_FLOAT(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_FLOAT, set)
#define CONFIG_VARIABLE_STRING(name, variable, set) \
    CONFIG_VARIABLE_GENERIC(name, variable, DEFAULT_STRING, set)

static default_t doom_defaults_list[] =
{
    CONFIG_VARIABLE_INT   (bloodsplats,         bloodsplats,          7),
    CONFIG_VARIABLE_INT   (brightmaps,          brightmaps,           1),
    CONFIG_VARIABLE_INT   (corpses,             corpses,             11),
    CONFIG_VARIABLE_INT   (dclick_use,          dclick_use,           1),
    CONFIG_VARIABLE_INT   (episode,             selectedepisode,      8),
    CONFIG_VARIABLE_INT   (expansion,           selectedexpansion,    9),
    CONFIG_VARIABLE_INT   (fullscreen,          fullscreen,           1),
    CONFIG_VARIABLE_INT   (gamepad_automap,     gamepadautomap,       2),
    CONFIG_VARIABLE_INT   (gamepad_fire,        gamepadfire,          2),
    CONFIG_VARIABLE_INT   (gamepad_lefthanded,  gamepadlefthanded,    1),
    CONFIG_VARIABLE_INT   (gamepad_menu,        gamepadmenu,          2),
    CONFIG_VARIABLE_INT   (gamepad_nextweapon,  gamepadnextweapon,    2),
    CONFIG_VARIABLE_INT   (gamepad_prevweapon,  gamepadprevweapon,    2),
    CONFIG_VARIABLE_INT   (gamepad_speed,       gamepadspeed,         2),
    CONFIG_VARIABLE_INT   (gamepad_use,         gamepaduse,           2),
    CONFIG_VARIABLE_INT   (gamepad_vibrate,     gamepadvibrate,       1),
    CONFIG_VARIABLE_INT   (gamepad_weapon1,     gamepadweapon1,       2),
    CONFIG_VARIABLE_INT   (gamepad_weapon2,     gamepadweapon2,       2),
    CONFIG_VARIABLE_INT   (gamepad_weapon3,     gamepadweapon3,       2),
    CONFIG_VARIABLE_INT   (gamepad_weapon4,     gamepadweapon4,       2),
    CONFIG_VARIABLE_INT   (gamepad_weapon5,     gamepadweapon5,       2),
    CONFIG_VARIABLE_INT   (gamepad_weapon6,     gamepadweapon6,       2),
    CONFIG_VARIABLE_INT   (gamepad_weapon7,     gamepadweapon7,       2),
    CONFIG_VARIABLE_FLOAT (gamma,               gamma,                0),
    CONFIG_VARIABLE_INT   (graphicdetail,       graphicdetail,        6),
    CONFIG_VARIABLE_INT   (grid,                grid,                 1),
    CONFIG_VARIABLE_INT   (homindicator,        homindicator,         1),
    CONFIG_VARIABLE_INT   (hud,                 hud,                  1),
    CONFIG_VARIABLE_STRING(iwadfolder,          iwadfolder,           0),
    CONFIG_VARIABLE_KEY   (key_down,            key_down,             3),
    CONFIG_VARIABLE_KEY   (key_down2,           key_down2,            3),
    CONFIG_VARIABLE_KEY   (key_fire,            key_fire,             3),
    CONFIG_VARIABLE_KEY   (key_left,            key_left,             3),
    CONFIG_VARIABLE_KEY   (key_nextweapon,      key_nextweapon,       3),
    CONFIG_VARIABLE_KEY   (key_prevweapon,      key_prevweapon,       3),
    CONFIG_VARIABLE_KEY   (key_right,           key_right,            3),
    CONFIG_VARIABLE_KEY   (key_speed,           key_speed,            3),
    CONFIG_VARIABLE_KEY   (key_strafe,          key_strafe,           3),
    CONFIG_VARIABLE_KEY   (key_strafeleft,      key_strafeleft,       3),
    CONFIG_VARIABLE_KEY   (key_straferight,     key_straferight,      3),
    CONFIG_VARIABLE_KEY   (key_up,              key_up,               3),
    CONFIG_VARIABLE_KEY   (key_up2,             key_up2,              3),
    CONFIG_VARIABLE_KEY   (key_use,             key_use,              3),
    CONFIG_VARIABLE_INT   (messages,            messages,             1),
    CONFIG_VARIABLE_INT   (mirrorweapons,       mirrorweapons,        1),
    CONFIG_VARIABLE_FLOAT (mouse_acceleration,  mouse_acceleration,   0),
    CONFIG_VARIABLE_INT   (mouse_fire,          mousebfire,           4),
    CONFIG_VARIABLE_INT   (mouse_forward,       mousebforward,        4),
    CONFIG_VARIABLE_INT   (mouse_sensitivity,   mouseSensitivity,     0),
    CONFIG_VARIABLE_INT   (mouse_strafe,        mousebstrafe,         4),
    CONFIG_VARIABLE_INT   (mouse_threshold,     mouse_threshold,      0),
    CONFIG_VARIABLE_INT   (music_volume,        musicVolume,          0),
    CONFIG_VARIABLE_INT   (novert,              novert,               1),
    CONFIG_VARIABLE_INT   (pixelwidth,          pixelwidth,           0),
    CONFIG_VARIABLE_INT   (pixelheight,         pixelheight,          0),
    CONFIG_VARIABLE_INT   (playerbob,           playerbob,           12),
    CONFIG_VARIABLE_INT   (rotate,              rotate,               1),
    CONFIG_VARIABLE_INT   (runcount,            runcount,             0),
    CONFIG_VARIABLE_FLOAT (saturation,          saturation,           0),
    CONFIG_VARIABLE_INT   (savegame,            selectedsavegame,     0),
    CONFIG_VARIABLE_INT   (screensize,          screensize,           0),
    CONFIG_VARIABLE_INT   (screenwidth,         screenwidth,          5),
    CONFIG_VARIABLE_INT   (screenheight,        screenheight,         5),
    CONFIG_VARIABLE_INT   (sfx_volume,          sfxVolume,            0),
    CONFIG_VARIABLE_INT   (skilllevel,          selectedskilllevel,  10),
    CONFIG_VARIABLE_INT   (smoketrails,         smoketrails,          1),
    CONFIG_VARIABLE_INT   (snd_maxslicetime_ms, snd_maxslicetime_ms,  0),
    CONFIG_VARIABLE_STRING(timidity_cfg_path,   timidity_cfg_path,    0),
    CONFIG_VARIABLE_INT   (translucency,        translucency,         1),
    CONFIG_VARIABLE_STRING(videodriver,         videodriver,          0),
    CONFIG_VARIABLE_INT   (widescreen,          widescreen,           1),
    CONFIG_VARIABLE_STRING(windowposition,      windowposition,       0),
    CONFIG_VARIABLE_INT   (windowwidth,         windowwidth,          0),
    CONFIG_VARIABLE_INT   (windowheight,        windowheight,         0),
    CONFIG_VARIABLE_STRING(xinput,              xinput,               0)
};

static default_collection_t doom_defaults =
{
    doom_defaults_list,
    arrlen(doom_defaults_list),
    NULL
};

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
    KEY_RALT,      ' ',            KEY_CAPSLOCK,  KEY_F1,
    KEY_F2,        KEY_F3,         KEY_F4,        KEY_F5,
    KEY_F6,        KEY_F7,         KEY_F8,        KEY_F9,
    KEY_F10,       KEY_PAUSE,      KEY_SCRLCK,    KEY_HOME,
    KEY_UPARROW,   KEY_PGUP,       KEY_MINUS,     KEY_LEFTARROW,
    KEYP_5,        KEY_RIGHTARROW, KEYP_PLUS,     KEY_END,
    KEY_DOWNARROW, KEY_PGDN,       KEY_INS,       KEY_DEL,
    0,             0,              0,             KEY_F11,
    KEY_F12,       0,              0,             0,
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
    { "false",                                  0,  1 },
    { "true",                                   1,  1 },
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
    { "rightshoulder",                        512,  2 },
    { "lefttrigger",                         1024,  2 },
    { "righttrigger",                        2048,  2 },
    { "a",                                   4096,  2 },
    { "b",                                   8192,  2 },
    { "x",                                  16384,  2 },
    { "y",                                  32768,  2 },
    { "none",                                   0,  3 },
    { "backspace",                             14,  3 },
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
    { "del",                                   83,  3 },
    { "insert",                                82,  3 },
    { "none",                                  -1,  4 },
    { "left",                                   0,  4 },
    { "middle",                                 1,  4 },
    { "right",                                  2,  4 },
    { "desktop",                                0,  5 },
    { "low",                                    0,  6 },
    { "high",                                   1,  6 },
    { "unlimited",                          32768,  7 },
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
    { "mirror",                                 1, 11 },
    { "slide",                                  2, 11 },
    { "mirror|slide",                           3, 11 },
    { "slide|mirror",                           3, 11 },
    { "smearblood",                             4, 11 },
    { "mirror|smearblood",                      5, 11 },
    { "smearblood|mirror",                      5, 11 },
    { "slide|smearblood",                       6, 11 },
    { "smearblood|slide",                       6, 11 },
    { "mirror|slide|smearblood",                7, 11 },
    { "slide|mirror|smearblood",                7, 11 },
    { "slide|smearblood|mirror",                7, 11 },
    { "mirror|smearblood|slide",                7, 11 },
    { "smearblood|mirror|slide",                7, 11 },
    { "smearblood|slide|mirror",                7, 11 },
    { "moreblood",                              8, 11 },
    { "mirror|moreblood",                       9, 11 },
    { "moreblood|mirror",                       9, 11 },
    { "slide|moreblood",                       10, 11 },
    { "moreblood|slide",                       10, 11 },
    { "mirror|slide|moreblood",                11, 11 },
    { "slide|mirror|moreblood",                11, 11 },
    { "slide|moreblood|mirror",                11, 11 },
    { "mirror|moreblood|slide",                11, 11 },
    { "moreblood|mirror|slide",                11, 11 },
    { "moreblood|slide|mirror",                11, 11 },
    { "smearblood|moreblood",                  12, 11 },
    { "moreblood|smearblood",                  12, 11 },
    { "mirror|smearblood|moreblood",           13, 11 },
    { "smearblood|mirror|moreblood",           13, 11 },
    { "smearblood|moreblood|mirror",           13, 11 },
    { "mirror|moreblood|smearblood",           13, 11 },
    { "moreblood|mirror|smearblood",           13, 11 },
    { "moreblood|smearblood|mirror",           13, 11 },
    { "slide|smearblood|moreblood",            14, 11 },
    { "smearblood|slide|moreblood",            14, 11 },
    { "smearblood|moreblood|slide",            14, 11 },
    { "slide|moreblood|smearblood",            14, 11 },
    { "moreblood|slide|smearblood",            14, 11 },
    { "moreblood|smearblood|slide",            14, 11 },
    { "mirror|slide|smearblood|moreblood",     15, 11 },
    { "mirror|slide|moreblood|smearblood",     15, 11 },
    { "mirror|smearblood|slide|moreblood",     15, 11 },
    { "mirror|smearblood|moreblood|slide",     15, 11 },
    { "mirror|moreblood|slide|smearblood",     15, 11 },
    { "mirror|moreblood|smearblood|slide",     15, 11 },
    { "slide|mirror|smearblood|moreblood",     15, 11 },
    { "slide|mirror|moreblood|smearblood",     15, 11 },
    { "slide|smearblood|mirror|moreblood",     15, 11 },
    { "slide|smearblood|moreblood|mirror",     15, 11 },
    { "slide|moreblood|mirror|smearblood",     15, 11 },
    { "slide|moreblood|smearblood|mirror",     15, 11 },
    { "smearblood|mirror|slide|moreblood",     15, 11 },
    { "smearblood|mirror|moreblood|slide",     15, 11 },
    { "smearblood|slide|mirror|moreblood",     15, 11 },
    { "smearblood|slide|moreblood|mirror",     15, 11 },
    { "smearblood|moreblood|mirror|slide",     15, 11 },
    { "smearblood|moreblood|slide|mirror",     15, 11 },
    { "moreblood|mirror|slide|smearblood",     15, 11 },
    { "moreblood|mirror|smearblood|slide",     15, 11 },
    { "moreblood|slide|mirror|smearblood",     15, 11 },
    { "moreblood|slide|smearblood|mirror",     15, 11 },
    { "moreblood|smearblood|mirror|slide",     15, 11 },
    { "moreblood|smearblood|slide|mirror",     15, 11 },
    { "0%",                                     0, 12 },
    { "75%",                                   75, 12 },
    { "100%",                                 100, 12 },
    { "",                                       0,  0 }
};

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
        int     v;

        // Print the name and line up all values at 30 characters
        chars_written = fprintf(f, "%s ", defaults[i].name);

        for (; chars_written < 30; ++chars_written)
            fprintf(f, " ");

        // Print the value
        switch (defaults[i].type)
        {
            case DEFAULT_KEY:

                // use the untranslated version if we can, to reduce
                // the possibility of screwing up the user's config
                // file
                v = *(int *)defaults[i].location;

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

            case DEFAULT_INT:
                {
                    int         j = 0;
                    boolean     flag = false;

                    v = *(int *)defaults[i].location;
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

            case DEFAULT_FLOAT:
                fprintf(f, "%.2f", *(float *)defaults[i].location);
                break;

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
    int         parm;

    if (strparm[0] == '\'' && strparm[2] == '\'')
    {
        int     s;

        for (s = 0; s < 128; ++s)
            if (tolower(strparm[1]) == scantokey[s])
                return s;
    }
    else
    {
        int     i = 0;

        while (alias[i].text[0])
        {
            if (!strcasecmp(strparm, alias[i].text) && set == alias[i].set)
                return alias[i].value;
            i++;
        }
    }

    if (strparm[0] == '0' && strparm[1] == 'x')
        sscanf(strparm + 2, "%x", &parm);
    else
        sscanf(strparm, "%i", &parm);

    return parm;
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
            {
                // not this one
                continue;
            }

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

                case DEFAULT_KEY:

                    // translate scancodes read from config
                    // file (save the old value in untranslated)
                    intparm = ParseIntParameter(strparm, def->set);
                    defaults[i].untranslated = intparm;
                    intparm = (intparm >= 0 && intparm < 128 ? scantokey[intparm] : 0);

                    defaults[i].original_translated = intparm;
                    *(int *)def->location = intparm;
                    break;

                case DEFAULT_FLOAT:
                    *(float *)def->location = (float)atof(strparm);
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
        doom_defaults.filename = M_StringJoin(configdir, "doomretro.cfg", NULL);

    LoadDefaultCollection(&doom_defaults);
}

//
// SetConfigDir:
//
// Sets the location of the configuration directory, where configuration
// files are stored - doomretro.cfg, savegames, etc.
//
void M_SetConfigDir(void)
{
    configdir = strdup("");
}
