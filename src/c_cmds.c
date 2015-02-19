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

#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_event.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_inter.h"
#include "p_local.h"
#include "SDL.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

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
extern int      gamepadvibrate;
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
extern boolean  widescreen;
extern int      windowheight;
extern char     *windowposition;
extern int      windowwidth;

#if defined(SDL20)
extern char     *scalequality;
extern boolean  vsync;
#endif

control_t controls[] =
{
    { "esc",           keyboard, KEY_ESCAPE }, { "1",             keyboard, '1'           }, { "2",             keyboard, '2'            },
    { "3",             keyboard, '3'        }, { "4",             keyboard, '4'           }, { "5",             keyboard, '5'            },
    { "6",             keyboard, '6'        }, { "7",             keyboard, '7'           }, { "8",             keyboard, '8'            },
    { "9",             keyboard, '9'        }, { "0",             keyboard, '0'           }, { "-",             keyboard, KEY_MINUS      },
    { "=",             keyboard, KEY_EQUALS }, { "+",             keyboard, KEY_EQUALS    }, { "backspace",     keyboard, KEY_BACKSPACE  },
    { "tab",           keyboard, KEY_TAB    }, { "q",             keyboard, 'q'           }, { "w",             keyboard, 'w'            },
    { "e",             keyboard, 'e'        }, { "r",             keyboard, 'r'           }, { "t",             keyboard, 't'            },
    { "y",             keyboard, 'y'        }, { "u",             keyboard, 'u'           }, { "i",             keyboard, 'i'            },
    { "o",             keyboard, 'o'        }, { "p",             keyboard, 'p'           }, { "[",             keyboard, '['            },
    { "]",             keyboard, ']'        }, { "enter",         keyboard, KEY_ENTER     }, { "ctrl",          keyboard, KEY_RCTRL      },
    { "a",             keyboard, 'a'        }, { "s",             keyboard, 's'           }, { "d",             keyboard, 'd'            },
    { "f",             keyboard, 'f'        }, { "g",             keyboard, 'g'           }, { "h",             keyboard, 'h'            },
    { "j",             keyboard, 'j'        }, { "k",             keyboard, 'k'           }, { "l",             keyboard, 'l'            },
    { ";",             keyboard, ';'        }, { "\'",            keyboard, '\''          }, { "`",             keyboard, KEY_TILDE      },
    { "shift",         keyboard, KEY_RSHIFT }, { "\\",            keyboard, '\\'          }, { "z",             keyboard, 'z'            },
    { "x",             keyboard, 'x'        }, { "c",             keyboard, 'c'           }, { "v",             keyboard, 'v'            },
    { "b",             keyboard, 'b'        }, { "n",             keyboard, 'n'           }, { "m",             keyboard, 'm'            },
    { ",",             keyboard, ','        }, { ".",             keyboard, '.'           }, { "/",             keyboard, '/'            },
    { "alt",           keyboard, KEY_RALT   }, { "space",         keyboard, ' '           }, { "numlock",       keyboard, KEY_NUMLOCK    },
    { "scrolllock",    keyboard, KEY_SCRLCK }, { "home",          keyboard, KEY_HOME      }, { "up",            keyboard, KEY_UPARROW    },
    { "pageup",        keyboard, KEY_PGUP   }, { "left",          keyboard, KEY_LEFTARROW }, { "right",         keyboard, KEY_RIGHTARROW },
    { "end",           keyboard, KEY_END    }, { "down",          keyboard, KEY_DOWNARROW }, { "pagedown",      keyboard, KEY_PGDN       },
    { "insert",        keyboard, KEY_INS    }, { "del",           keyboard, KEY_DEL       }, { "left",          mouse,    0              },
    { "mouse1",        mouse,    0          }, { "middle",        mouse,    1             }, { "mouse2",        mouse,    1              },
    { "right",         mouse,    2          }, { "mouse3",        mouse,    2             }, { "mouse4",        mouse,    3              },
    { "mouse5",        mouse,    4          }, { "mouse6",        mouse,    5             }, { "mouse7",        mouse,    6              },
    { "mouse8",        mouse,    7          }, { "wheelup",       mouse,    8             }, { "wheeldown",     mouse,    9              },
    { "dpadup",        gamepad,  1          }, { "dpaddown",      gamepad,  2             }, { "dpadleft",      gamepad,  4              },
    { "dpadright",     gamepad,  8          }, { "start",         gamepad,  16            }, { "back",          gamepad,  32             },
    { "leftthumb",     gamepad,  64         }, { "rightthumb",    gamepad,  128           }, { "leftshoulder",  gamepad,  256            },
    { "LS",            gamepad,  256        }, { "leftbutton",    gamepad,  256           }, { "LB",            gamepad,  256            },
    { "rightshoulder", gamepad,  512        }, { "RS",            gamepad,  512           }, { "rightbutton",   gamepad,  512            },
    { "RB",            gamepad,  512        }, { "lefttrigger",   gamepad,  1024          }, { "LT",            gamepad,  1024           },
    { "righttrigger",  gamepad,  2048       }, { "RT",            gamepad,  2048          }, { "buttona",       gamepad,  4096           },
    { "buttonb",       gamepad,  8192       }, { "buttonx",       gamepad,  16384         }, { "buttony",       gamepad,  32768          },
    { "",              0,        0          }
};

action_t actions[] = 
{
    { "+automap",       &key_automap,            NULL,              NULL,              &gamepadautomap           },
    { "+back",          &key_down,               &key_down2,        NULL,              NULL                      },
    { "+clearmark",     &key_automap_clearmark,  NULL,              NULL,              &gamepadautomapclearmark  },
    { "+fire",          &key_fire,               NULL,              &mousebfire,       &gamepadfire              },
    { "+followmode",    &key_automap_followmode, NULL,              NULL,              &gamepadautomapfollowmode },
    { "+forward",       &key_up,                 &key_up2,          &mousebforward,    NULL                      },
    { "+grid",          &key_automap_grid,       NULL,              NULL,              &gamepadautomapgrid       },
    { "+left",          &key_left,               NULL,              NULL,              NULL                      },
    { "+mark",          &key_automap_mark,       NULL,              NULL,              &gamepadautomapmark       },
    { "+maxzoom",       &key_automap_maxzoom,    NULL,              NULL,              &gamepadautomapmaxzoom    },
    { "+menu",          NULL,                    NULL,              NULL,              &gamepadmenu              },
    { "+nextweapon",    &key_nextweapon,         NULL,              &mousebnextweapon, &gamepadnextweapon        },
    { "+prevweapon",    &key_prevweapon,         NULL,              &mousebprevweapon, &gamepadprevweapon        },
    { "+right",         &key_right,              NULL,              NULL,              NULL                      },
    { "+rotatemode",    &key_automap_rotatemode, NULL,              NULL,              &gamepadautomaprotatemode },
    { "+run",           &key_run,                NULL,              NULL,              &gamepadrun               },
    { "+strafe",        &key_strafe,             NULL,              &mousebstrafe,     NULL                      },
    { "+strafeleft",    &key_strafeleft,         &key_strafeleft2,  NULL,              NULL                      },
    { "+straferight",   &key_straferight,        &key_straferight2, NULL,              NULL                      },
    { "+use",           &key_use,                NULL,              &mousebuse,        &gamepaduse               },
    { "+weapon1",       &key_weapon1,            NULL,              NULL,              &gamepadweapon1           },
    { "+weapon2",       &key_weapon2,            NULL,              NULL,              &gamepadweapon2           },
    { "+weapon3",       &key_weapon3,            NULL,              NULL,              &gamepadweapon3           },
    { "+weapon4",       &key_weapon4,            NULL,              NULL,              &gamepadweapon4           },
    { "+weapon5",       &key_weapon5,            NULL,              NULL,              &gamepadweapon5           },
    { "+weapon6",       &key_weapon6,            NULL,              NULL,              &gamepadweapon6           },
    { "+weapon7",       &key_weapon7,            NULL,              NULL,              &gamepadweapon7           },
    { "+zoomin",        &key_automap_zoomin,     NULL,              NULL,              &gamepadautomapzoomin     },
    { "+zoomout",       &key_automap_zoomout,    NULL,              NULL,              &gamepadautomapzoomout    },
    { "",               NULL,                    NULL,              NULL,              NULL                      }
};

boolean C_BindCondition(char *, char *, char *);
boolean C_BloodSplatsCondition(char *, char *, char *);
boolean C_BooleanCondition(char *, char *, char *);
boolean C_CheatCondition(char *, char *, char *);
boolean C_GameCondition(char *, char *, char *);
boolean C_GammaCondition(char *, char *, char *);
boolean C_GiveCondition(char *, char *, char *);
boolean C_GodCondition(char *, char *, char *);
boolean C_GraphicDetailCondition(char *, char *, char *);
boolean C_KillCondition(char *, char *, char *);
boolean C_MapCondition(char *, char *, char *);
boolean C_NoCondition(char *, char *, char *);
boolean C_SummonCondition(char *, char *, char *);

void C_AlwaysRun(char *, char *, char *);
void C_Bind(char *, char *, char *);
void C_BloodSplats(char *, char *, char *);
void C_Boolean(char *, char *, char *);
void C_Clear(char *, char *, char *);
void C_CmdList(char *, char *, char *);
void C_CvarList(char *, char *, char *);
void C_EndGame(char *, char *, char *);
void C_Gamma(char *, char *, char *);
void C_God(char *, char *, char *);
void C_Give(char *, char *, char *);
void C_GraphicDetail(char *, char *, char *);
void C_Help(char *, char *, char *);
void C_Hud(char *, char *, char *);
void C_Kill(char *, char *, char *);
void C_Map(char *, char *, char *);
void C_NoClip(char *, char *, char *);
void C_NoTarget(char *, char *, char *);
void C_Quit(char *, char *, char *);
void C_ShowFPS(char *, char *, char *);
void C_String(char *, char *, char *);
void C_Summon(char *, char *, char *);

alias_t aliases[] =
{
    { "off",           0, 1 }, { "on",            1, 1 }, { "0",             0, 1 }, { "1",             1, 1 },
    { "no",            0, 1 }, { "yes",           1, 1 }, { "false",         0, 1 }, { "true",          1, 1 },
    { "none",          0, 2 }, { "off",           0, 2 }, { "no",            0, 2 }, { "false",         0, 2 },
    { "unlimited", 32768, 2 }, { "on",        32768, 2 }, { "yes",       32768, 2 }, { "true",      32768, 2 },
    { "low",           0, 3 }, { "high",          1, 3 }, { "0.5",           0, 4 }, { "0.55",          1, 4 },
    { "0.6",           2, 4 }, { "0.65",          3, 4 }, { "0.7",           4, 4 }, { "0.75",          5, 4 },
    { "0.8",           6, 4 }, { "0.85",          7, 4 }, { "0.9",           8, 4 }, { "0.95",          9, 4 },
    { "off",          10, 4 }, { "1.0",          10, 4 }, { "1.05",         11, 4 }, { "1.1",          12, 4 },
    { "1.15",         13, 4 }, { "1.2",          14, 4 }, { "1.25",         15, 4 }, { "1.3",          16, 4 },
    { "1.35",         17, 4 }, { "1.4",          18, 4 }, { "1.45",         19, 4 }, { "1.5",          20, 4 },
    { "1.55",         21, 4 }, { "1.6",          22, 4 }, { "1.65",         23, 4 }, { "1.7",          24, 4 },
    { "1.75",         25, 4 }, { "1.8",          26, 4 }, { "1.85",         27, 4 }, { "1.9",          28, 4 },
    { "1.95",         29, 4 }, { "2.0",          30, 4 }, { "",              0, 0 }
};

int C_LookupValueFromAlias(char *text, int set)
{
    int i = 0;

    while (aliases[i].text[0])
    {
        if (set == aliases[i].set && !strcasecmp(text, aliases[i].text))
            return aliases[i].value;
        ++i;
    }
    return -1;
}

char *C_LookupAliasFromValue(int value, int set)
{
    int         i = 0;
    static char buffer[1024];

    while (aliases[i].text[0])
    {
        if (set == aliases[i].set && value == aliases[i].value)
            return aliases[i].text;
        ++i;
    }
    return NULL;
}

consolecmd_t consolecmds[] =
{
    { "alwaysrun",            C_BooleanCondition,       C_AlwaysRun,     1, CT_CVAR,  CF_BOOLEAN,               &alwaysrun,          1, ""                                     },
    { "animatedliquid",       C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &animatedliquid,     1, ""                                     },
    { "bind",                 C_BindCondition,          C_Bind,          2, CT_CMD,   CF_NONE,                  NULL,                0, "Bind an action to a control."         },
    { "bloodsplats",          C_BloodSplatsCondition,   C_BloodSplats,   1, CT_CVAR,  CF_INTEGER,               &bloodsplats,        2, ""                                     },
    { "brightmaps",           C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &brightmaps,         1, ""                                     },
    { "centerweapon",         C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &centerweapon,       1, ""                                     },
    { "clear",                C_NoCondition,            C_Clear,         0, CT_CMD,   CF_NONE,                  NULL,                0, "Clear the console."                   },
    { "cmdlist",              C_NoCondition,            C_CmdList,       0, CT_CMD,   CF_NONE,                  NULL,                0, "Display a list of console commands."  },
    { "corpses_mirror",       C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &corpses_mirror,     1, ""                                     },
    { "corpses_moreblood",    C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &corpses_moreblood,  1, ""                                     },
    { "corpses_slide",        C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &corpses_slide,      1, ""                                     },
    { "corpses_smearblood",   C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &corpses_smearblood, 1, ""                                     },
    { "cvarlist",             C_NoCondition,            C_CvarList,      0, CT_CMD,   CF_NONE,                  NULL,                0, "Display a list of console variables." },
    { "dclick_use",           C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &dclick_use,         1, ""                                     },
    { "endgame",              C_GameCondition,          C_EndGame,       0, CT_CMD,   CF_NONE,                  NULL,                0, "End a game."                          },
    { "floatbob",             C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &floatbob,           1, ""                                     },
    { "followmode",           C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN | CF_NOTSAVED, &followmode,         1, ""                                     },
    { "footclip",             C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &footclip,           1, ""                                     },
    { "fullscreen",           C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &fullscreen,         1, ""                                     },
    { "gammacorrectionlevel", C_GammaCondition,         C_Gamma,         1, CT_CVAR,  CF_BOOLEAN,               &graphicdetail,      4, ""                                     },
    { "god",                  C_GodCondition,           C_God,           0, CT_CMD,   CF_NONE,                  NULL,                0, "Toggle god mode on/off."              },
    { "graphicdetail",        C_GraphicDetailCondition, C_GraphicDetail, 1, CT_CVAR,  CF_BOOLEAN,               &graphicdetail,      3, ""                                     },
    { "grid",                 C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &grid,               1, ""                                     },
    { "help",                 C_NoCondition,            C_Help,          0, CT_CMD,   CF_NONE,                  NULL,                0, "Display the help screen."             },
    { "homindicator",         C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &homindicator,       1, ""                                     },
    { "hud",                  C_BooleanCondition,       C_Hud,           1, CT_CVAR,  CF_BOOLEAN,               &hud,                1, ""                                     },
    { "idbeholda",            C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idbeholdl",            C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idbeholdi",            C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idbeholdr",            C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idbeholds",            C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idbeholdv",            C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idchoppers",           C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idclev",               C_CheatCondition,         NULL,            1, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idclip",               C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "iddqd",                C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "iddt",                 C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idfa",                 C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idkfa",                C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idmus",                C_CheatCondition,         NULL,            1, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idmypos",              C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "idspispopd",           C_CheatCondition,         NULL,            0, CT_CHEAT, CF_NONE,                  NULL,                0, ""                                     },
    { "iwadfolder",           C_NoCondition,            C_String,        1, CT_CVAR,  CF_STRING,                &iwadfolder,         0, ""                                     },
    { "kill",                 C_KillCondition,          C_Kill,          1, CT_CMD,   CF_NONE,                  NULL,                0, "Kill the player or monsters."         },
    { "map",                  C_MapCondition,           C_Map,           1, CT_CMD,   CF_NONE,                  NULL,                0, "Warp to a map."                       },
    { "mapfixes",             C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &mapfixes,           1, ""                                     },
    { "messages",             C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &messages,           1, ""                                     },
    { "mirrorweapons",        C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &mirrorweapons,      1, ""                                     },
    { "noclip",               C_GameCondition,          C_NoClip,        0, CT_CMD,   CF_NONE,                  NULL,                0, "Toggle no clipping mode on/off."      },
    { "notarget",             C_GameCondition,          C_NoTarget,      0, CT_CMD,   CF_NONE,                  NULL,                0, "Toggle no target mode on/off."        },
    { "novert",               C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &novert,             1, ""                                     },
    { "quit",                 C_NoCondition,            C_Quit,          0, CT_CMD,   CF_NONE,                  NULL,                0, "Quit DOOM RETRO."                     },
    { "rotatemode",           C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &rotatemode,         1, ""                                     },
#if defined(SDL20)
    { "scalequality",         C_NoCondition,            C_String,        1, CT_CVAR,  CF_STRING,                &scalequality,       0, ""                                     },
#endif
    { "shadows",              C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &shadows,            1, ""                                     },
    { "showfps",              C_BooleanCondition,       C_ShowFPS,       1, CT_CVAR,  CF_BOOLEAN,               &showfps,            1, ""                                     },
    { "smoketrails",          C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &smoketrails,        1, ""                                     },
    { "summon",               C_SummonCondition,        C_Summon,        1, CT_CMD,   CF_NONE,                  NULL,                0, "Summon a monster or map decoration."  },
    { "timidity_cfg_path",    C_NoCondition,            C_String,        1, CT_CVAR,  CF_STRING,                &timidity_cfg_path,  0, ""                                     },
    { "translucency",         C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &translucency,       1, ""                                     },
    { "videodriver",          C_NoCondition,            C_String,        1, CT_CVAR,  CF_STRING,                &videodriver,        0, ""                                     },
#if defined(SDL20)
    { "vsync",                C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &vsync,              1, ""                                     },
#endif
    { "widescreen",           C_BooleanCondition,       C_Boolean,       1, CT_CVAR,  CF_BOOLEAN,               &widescreen,         1, ""                                     },
    { "windowposition",       C_NoCondition,            C_String,        1, CT_CVAR,  CF_STRING,                &windowposition,     0, ""                                     },
    { "",                     C_NoCondition,            NULL,            0, 0,        CF_NONE,                  NULL,                0, ""                                     }
};

//
// All cheat cmds
//
boolean C_CheatCondition(char *cmd, char *parm1, char *parm2)
{
    if (gamestate != GS_LEVEL)
        return false;
    if (!strcasecmp(cmd, "idclip") && gamemode != commercial)
        return false;
    if (!strcasecmp(cmd, "idspispopd") && gamemode == commercial)
        return false;
    return true;
}

//
// Cmd is only valid when game is running
//
boolean C_GameCondition(char *cmd, char *parm1, char *parm2)
{
    return (gamestate == GS_LEVEL);
}

//
// Cmd is always valid
//
boolean C_NoCondition(char *cmd, char *parm1, char *parm2)
{
    return true;
}

//
// ALWAYSRUN cvar
//
void C_AlwaysRun(char *cmd, char *parm1, char *parm2)
{
    C_Boolean(cmd, parm1, "");
    I_InitKeyboard();
}

//
// BIND cmd
//
boolean C_BindCondition(char *cmd, char *parm1, char *parm2)
{
    return true;
}

void C_DisplayBinds(char *action, int value, controltype_t type, int count)
{
    int         control = 0;
    static char buffer[1024];

    while (controls[control].type)
    {
        if (controls[control].type == type && controls[control].value == value)
        {
            M_snprintf(buffer, sizeof(buffer), "%i\t%s\t%s", count,
                controls[control].control, action);
            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
            break;
        }
        ++control;
    }
}

void C_Bind(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
    {
        int     action = 0;
        int     count = 1;

        while (actions[action].action[0])
        {
            if (actions[action].keyboard1)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].keyboard1, keyboard, count++);
            if (actions[action].keyboard2)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].keyboard2, keyboard, count++);
            if (actions[action].mouse)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].mouse, mouse, count++);
            if (actions[action].gamepad)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].gamepad, gamepad, count++);
            ++action;
        }
    }
    else
    {
        int     control = 0;

        while (controls[control].type)
        {
            if (!strcasecmp(parm1, controls[control].control))
                break;
            ++control;
        }

        if (controls[control].control[0])
        {
            int         action = 0;
            static char buffer[1024];

            if (!parm2[0])
            {
                int     action = 0;

                while (actions[action].action[0])
                {
                    if (controls[control].type == keyboard && actions[action].keyboard1
                        && controls[control].value == *(int *)actions[action].keyboard1)
                    {
                        M_snprintf(buffer, sizeof(buffer), "%s \"%s\"", controls[control].control,
                            actions[action].action);
                        C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
                    }
                    else if (controls[control].type == keyboard && actions[action].keyboard2
                        && controls[control].value == *(int *)actions[action].keyboard2)
                    {
                        M_snprintf(buffer, sizeof(buffer), "%s \"%s\"", controls[control].control,
                            actions[action].action);
                        C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
                    }
                    else if (controls[control].type == mouse && actions[action].mouse
                        && controls[control].value == *(int *)actions[action].mouse)
                    {
                        M_snprintf(buffer, sizeof(buffer), "%s \"%s\"", controls[control].control,
                            actions[action].action);
                        C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
                    }
                    else if (controls[control].type == gamepad && actions[action].gamepad
                        && controls[control].value == *(int *)actions[action].gamepad)
                    {
                        M_snprintf(buffer, sizeof(buffer), "%s \"%s\"", controls[control].control,
                            actions[action].action);
                        C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
                    }
                    ++action;
                }
            }
            else
            {
                while (actions[action].action[0])
                {
                    if (!strcasecmp(parm2, actions[action].action))
                        break;
                    ++action;
                }

                if (actions[action].action[0])
                {
                    switch (controls[control].type)
                    {
                        case keyboard:
                            *(int *)actions[action].keyboard1 = controls[control].value;
                            break;
                        case mouse:
                            *(int *)actions[action].mouse = controls[control].value;
                            break;
                        case gamepad:
                            *(int *)actions[action].gamepad = controls[control].value;
                            break;
                    }

                    M_SaveDefaults();

                    M_snprintf(buffer, sizeof(buffer), "%s \"%s\"", parm1, parm2);
                    C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
                }
            }
        }
    }
}

//
// All boolean cvars
//
boolean C_BooleanCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, 1) >= 0);
}

void C_Boolean(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].cmd[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].cmd) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_BOOLEAN))
        {
            static char buffer[1024];

            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                int     value = C_LookupValueFromAlias(parm1, 1);

                if (value == 0 || value == 1)
                {
                    *(boolean *)consolecmds[i].value = !!value;

                    if (!(consolecmds[i].flags & CF_NOTSAVED))
                        M_SaveDefaults();

                    M_snprintf(buffer, sizeof(buffer), "%s is now %s.", cmd, parm1);
                }
            }
            else
                M_snprintf(buffer, sizeof(buffer), "%s is %s.", cmd,
                    (*(boolean *)consolecmds[i].value ? "on" : "off"));

            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

//
// BLOODSPLATS cvar
//
void (*P_BloodSplatSpawner)(fixed_t, fixed_t, int, int);

boolean C_BloodSplatsCondition(char *cmd, char *parm1, char *parm2)
{
    int integer = 0;

    return (!parm1[0] || C_LookupValueFromAlias(parm1, 2) >= 0 || sscanf(parm1, "%i", &integer));
}

void C_BloodSplats(char *cmd, char *parm1, char *parm2)
{
    static char buffer[1024];

    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 2);

        if (value == -1)
            sscanf(parm1, "%i", &value);

        if (value >= 0)
        {
            bloodsplats = value;

            M_SaveDefaults();

            P_BloodSplatSpawner = ((bloodsplats == UNLIMITED ? P_SpawnBloodSplat :
                (bloodsplats ? P_SpawnBloodSplat2 : P_NullBloodSplatSpawner)));

            M_snprintf(buffer, sizeof(buffer), "bloodsplats is now %s.", parm1);
        }
    }
    else
    {
        char    *alias = C_LookupAliasFromValue(bloodsplats, 2);

        if (alias)
            M_snprintf(buffer, sizeof(buffer), "bloodsplats is %s.", alias);
        else
            M_snprintf(buffer, sizeof(buffer), "bloodsplats is %i.", bloodsplats);
    }

    C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
}

//
// CLEAR cmd
//
extern int      consolestrings;

void C_Clear(char *cmd, char *parm1, char *parm2)
{
    consolestrings = 0;
}

//
// CMDLIST cmd
//
void C_CmdList(char *cmd, char *parm1, char *parm2)
{
    int i = 0;
    int count = 1;

    while (consolecmds[i].cmd[0])
    {
        if (consolecmds[i].type == CT_CMD)
        {
            static char     buffer[1024];

            M_snprintf(buffer, 1024, "%i\t%s\t%s", count++, consolecmds[i].cmd,
                consolecmds[i].description);
            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

//
// CVARLIST cmd
//
void C_CvarList(char *cmd, char *parm1, char *parm2)
{
    int i = 0;
    int count = 1;

    while (consolecmds[i].cmd[0])
    {
        if (consolecmds[i].type == CT_CVAR)
        {
            static char     buffer[1024];

            if (consolecmds[i].flags & CF_BOOLEAN)
                M_snprintf(buffer, sizeof(buffer), "%i\t%s\t\t%s", count++, consolecmds[i].cmd,
                    C_LookupAliasFromValue(*(boolean *)consolecmds[i].value, consolecmds[i].set));
            else if (consolecmds[i].flags & CF_INTEGER)
            {
                char *alias = C_LookupAliasFromValue(*(int *)consolecmds[i].value,
                    consolecmds[i].set);

                if (alias)
                    M_snprintf(buffer, sizeof(buffer), "%i\t%s\t\t%s", count++, consolecmds[i].cmd,
                        alias);
                else
                    M_snprintf(buffer, sizeof(buffer), "%i\t%s\t\t%i", count++, consolecmds[i].cmd,
                        *(int *)consolecmds[i].value);
            }
            else if (consolecmds[i].flags & CF_STRING)
                M_snprintf(buffer, sizeof(buffer), "%i\t%s\t\t\"%s\"", count++, consolecmds[i].cmd,
                    *(char **)consolecmds[i].value);
            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

//
// ENDGAME cmd
//
void C_EndGame(char *cmd, char *parm1, char *parm2)
{
    M_EndingGame();
}

//
// GAMMACORRECTIONLEVEL cvar
//
extern int      st_palette;

boolean C_GammaCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, 4) >= 0);
}

void C_Gamma(char *cmd, char *parm1, char *parm2)
{
    static char buffer[1024];

    if (parm1[0])
    {
        int value = C_LookupValueFromAlias(parm1, 4);

        if (value != -1)
        {
            gammaindex = value;

            I_SetPalette((byte *)W_CacheLumpName("PLAYPAL", PU_CACHE) + st_palette * 768);

            M_SaveDefaults();

            if (gammaindex == 10)
                M_StringCopy(buffer, "\"gammacorrectionlevel\" is now off.", sizeof(buffer));
            else
                M_snprintf(buffer, sizeof(buffer), "\"gammacorrectionlevel\" is now %.2f.",
                    gammalevels[gammaindex]);
        }
    }
    else
    {
        if (gammaindex == 10)
            M_StringCopy(buffer, "\"gammacorrectionlevel\" is off.", sizeof(buffer));
        else
            M_snprintf(buffer, sizeof(buffer), "\"gammacorrectionlevel\" is %.2f.",
                gammalevels[gammaindex]);
    }

    C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
}

//
// GOD cmd
//
boolean C_GodCondition(char *cmd, char *parm1, char *parm2)
{
    return (gamestate == GS_LEVEL && players[displayplayer].playerstate == PST_LIVE);
}

void C_God(char *cmd, char *parm1, char *parm2)
{
    player_t      *player = &players[displayplayer];

    player->cheats ^= CF_GODMODE;
    C_AddConsoleString(((player->cheats & CF_GODMODE) ? s_STSTR_GODON : s_STSTR_GODOFF), output,
        CONSOLEOUTPUTCOLOR);
}

//
// GRAPHICDETAIL cvar
//
boolean C_GraphicDetailCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, 3) >= 0);
}

void C_GraphicDetail(char *cmd, char *parm1, char *parm2)
{
    static char buffer[1024];

    if (parm1[0])
    {
        int value = C_LookupValueFromAlias(parm1, 3);

        if (value == 0 || value == 1)
        {
            graphicdetail = !!value;

            M_SaveDefaults();

            M_snprintf(buffer, sizeof(buffer), "graphicdetail is now %s.", parm1);
        }
    }
    else
        M_snprintf(buffer, sizeof(buffer), "graphicdetail is %s.",
            C_LookupAliasFromValue(graphicdetail, 3));

    C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
}

//
// HELP cmd
//
void C_Help(char *cmd, char *parm1, char *parm2)
{
    C_HideConsole();
    M_ShowHelp();
}

//
// HUD cvar
//
void C_Hud(char *cmd, char *parm1, char *parm2)
{
    if (widescreen || screensize == 8)
        C_Boolean(cmd, parm1, "");
}

//
// KILL cmd
//
static int      killcmdtype = NUMMOBJTYPES;

void A_Fall(mobj_t *actor);

boolean C_KillCondition(char *cmd, char *parm1, char *parm2)
{
    if (gamestate == GS_LEVEL)
    {
        int i;

        if (!parm1[0] || !strcasecmp(parm1, "all") || !strcasecmp(parm1, "monsters"))
            return true;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (!strcasecmp(parm1, mobjinfo[i].name1)
                || !strcasecmp(parm1, mobjinfo[i].plural1)
                || (mobjinfo[i].name2[0] && !strcasecmp(parm1, mobjinfo[i].name2))
                || (mobjinfo[i].plural2[0] && !strcasecmp(parm1, mobjinfo[i].plural2)))
            {
                boolean     kill = true;

                killcmdtype = mobjinfo[i].doomednum;
                if (gamemode != commercial)
                {
                    switch (killcmdtype)
                    {
                        case Arachnotron:
                        case ArchVile:
                        case BossBrain:
                        case HellKnight:
                        case Mancubus:
                        case PainElemental:
                        case HeavyWeaponDude:
                        case Revenant:
                        case WolfensteinSS:
                            kill = false;
                            break;
                    }
                }
                else if (killcmdtype == WolfensteinSS && bfgedition)
                    killcmdtype = Zombieman;

                if (!(mobjinfo[i].flags & MF_SHOOTABLE))
                    kill = false;

                return kill;
            }
    }
    return false;
}

void C_Kill(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
    {
        P_KillMobj(NULL, players[displayplayer].mo);
        C_AddConsoleString("Player killed.", output, CONSOLEOUTPUTCOLOR);
    }
    else
    {
        int             i, j;
        int             kills = 0;
        static char     buffer[1024];

        if (!strcasecmp(parm1, "all") || !strcasecmp(parm1, "monsters"))
        {
            for (i = 0; i < numsectors; ++i)
            {
                mobj_t      *thing = sectors[i].thinglist;

                while (thing)
                {
                    if (thing->health > 0)
                    {
                        if (thing->type == MT_PAIN)
                        {
                            A_Fall(thing);
                            P_SetMobjState(thing, S_PAIN_DIE6);
                            kills++;
                        }
                        else if (thing->flags & MF_COUNTKILL)
                        {
                            P_DamageMobj(thing, NULL, NULL, thing->health);
                            kills++;
                        }
                    }
                    thing = thing->snext;
                }
            }
        }
        else
        {
            int type = P_FindDoomedNum(P_FindDoomedNum(killcmdtype));

            for (j = 0; j < numsectors; ++j)
            {
                mobj_t      *thing = sectors[j].thinglist;

                while (thing)
                {
                    if (thing->health > 0)
                    {
                        if (type == thing->type)
                            if (type == MT_PAIN)
                            {
                                A_Fall(thing);
                                P_SetMobjState(thing, S_PAIN_DIE6);
                                kills++;
                            }
                            else if (thing->flags & MF_COUNTKILL)
                            {
                                P_DamageMobj(thing, NULL, NULL, thing->health);
                                kills++;
                            }
                    }
                    thing = thing->snext;
                }
            }
        }
        M_snprintf(buffer, sizeof(buffer), "%i monster%s killed.", kills, kills == 1 ? "" : "s");
        C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
    }
}

//
// MAP cmd
//
static int      mapcmdepisode;
static int      mapcmdmap;

extern boolean  samelevel;
extern int      selectedepisode;
extern int      selectedskilllevel;
extern menu_t   EpiDef;

boolean C_MapCondition(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
        return false;

    mapcmdepisode = 0;
    mapcmdmap = 0;

    if (gamemode == commercial)
    {
        if (BTSX)
        {
            sscanf(uppercase(parm1), "E%iM%02i", &mapcmdepisode, &mapcmdmap);
            if (mapcmdmap && ((mapcmdepisode == 1 && BTSXE1) || (mapcmdepisode == 2 && BTSXE2)))
            {
                M_snprintf(parm1, sizeof(parm1), "MAP%02i", mapcmdmap);
                return (W_CheckMultipleLumps(parm1) == 2);
            }
        }
        sscanf(uppercase(parm1), "MAP%02i", &mapcmdmap);
        if (!mapcmdmap)
            return false;
        if (BTSX && (W_CheckMultipleLumps(parm1) == 1))
            return false;
        if (gamestate != GS_LEVEL && gamemission == pack_nerve)
            gamemission = doom2;
    }
    else
    {
        sscanf(uppercase(parm1), "E%iM%i", &mapcmdepisode, &mapcmdmap);
        if (!mapcmdepisode || !mapcmdmap)
            return false;
    }

    return (W_CheckNumForName(parm1) >= 0);
}

void C_Map(char *cmd, char *parm1, char *parm2)
{
    samelevel = (gameepisode == mapcmdepisode && gamemap == mapcmdmap);
    gameepisode = mapcmdepisode;
    if (gamemission == doom && gameepisode <= 4)
    {
        selectedepisode = gameepisode - 1;
        EpiDef.lastOn = selectedepisode;
    }
    gamemap = mapcmdmap;
    if (gamestate == GS_LEVEL)
        G_DeferredLoadLevel((gamestate == GS_LEVEL ? gameskill : selectedskilllevel), gameepisode,
            gamemap);
    else
        G_DeferredInitNew((gamestate == GS_LEVEL ? gameskill : selectedskilllevel), gameepisode,
            gamemap);
}

//
// NOCLIP cmd
//
void C_NoClip(char *cmd, char *parm1, char *parm2)
{
    M_StringCopy(consolecheat, (gamemode == commercial ? "idclip" : "idspispopd"),
        sizeof(consolecheat));
}

//
// NOTARGET cmd
//
void C_NoTarget(char *cmd, char *parm1, char *parm2)
{
    players[displayplayer].cheats ^= CF_NOTARGET;
    C_AddConsoleString(players[displayplayer].cheats & CF_NOTARGET ? s_STSTR_NTON : s_STSTR_NTOFF,
        output, CONSOLEOUTPUTCOLOR);
}

//
// QUIT cmd
//
void C_Quit(char *cmd, char *parm1, char *parm2)
{
    I_Quit(true);
}

//
// SHOWFPS cvar
//
void C_ShowFPS(char *cmd, char *parm1, char *parm2)
{
    static char buffer[1024];

    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 1);

        if (value == 0 || value == 1)
        {
            showfps = false;
            HU_clearMessages();

            M_snprintf(buffer, sizeof(buffer), "showfps is now %s.", parm1);
        }
    }
    else
        M_snprintf(buffer, sizeof(buffer), "showfps is %s.", (showfps ? "on" : "off"));

    C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
}

//
// All string cvars
//
void C_String(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].cmd[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].cmd) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_STRING))
        {
            static char     buffer[1024];

            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                *(char **)consolecmds[i].value = strdup(parm1);

                M_SaveDefaults();

                M_snprintf(buffer, sizeof(buffer), "%s is now \"%s\".", cmd, parm1);
            }
            else
                M_snprintf(buffer, sizeof(buffer), "%s is \"%s\".", cmd,
                    *(char **)consolecmds[i].value);

            C_AddConsoleString(buffer, output, CONSOLEOUTPUTCOLOR);
        }
        ++i;
    }
}

//
// SUMMON cmd
//
static int      summoncmdtype = NUMMOBJTYPES;

boolean C_SummonCondition(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
        return false;

    if (gamestate == GS_LEVEL)
    {
        int i;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (!strcasecmp(parm1, mobjinfo[i].name1)
                || (mobjinfo[i].name2[0] && !strcasecmp(parm1, mobjinfo[i].name2)))
            {
                boolean     summon = true;

                summoncmdtype = mobjinfo[i].doomednum;
                if (gamemode != commercial)
                {
                    switch (summoncmdtype)
                    {
                        case Arachnotron:
                        case ArchVile:
                        case BossBrain:
                        case HellKnight:
                        case Mancubus:
                        case PainElemental:
                        case HeavyWeaponDude:
                        case Revenant:
                        case WolfensteinSS:
                            summon = false;
                            break;
                    }
                }
                else if (summoncmdtype == WolfensteinSS && bfgedition)
                    summoncmdtype = Zombieman;

                return summon;
            }
    }
    return false;
}

void C_Summon(char *cmd, char *parm1, char *parm2)
{
    mobj_t      *player = players[displayplayer].mo;
    fixed_t     x = player->x;
    fixed_t     y = player->y;
    angle_t     angle = player->angle >> ANGLETOFINESHIFT;
    mobj_t      *thing = P_SpawnMobj(x + 100 * finecosine[angle], y + 100 * finesine[angle],
        ONFLOORZ, P_FindDoomedNum(summoncmdtype));

    thing->angle = R_PointToAngle2(thing->x, thing->y, x, y);
}
