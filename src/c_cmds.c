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
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <XInput.h>
#endif

#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_event.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_inter.h"
#include "p_local.h"
#include "s_sound.h"
#include "SDL.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAPCMDFORMAT    "map E~x~M~y~|MAP~xy~"
#define SUMMONCMDFORMAT "summon ~type~"

extern boolean  alwaysrun;
extern boolean  animatedliquid;
extern int      bloodsplats;
extern int      brightmaps;
extern boolean  centerweapon;
extern char     *conback;
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
extern float    gamepadleftdeadzone_percent;
extern int      gamepadrightdeadzone;
extern float    gamepadrightdeadzone_percent;
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
    { "1",             keyboard, '1'            }, { "2",             keyboard, '2'            }, { "3",             keyboard, '3'            },
    { "4",             keyboard, '4'            }, { "5",             keyboard, '5'            }, { "6",             keyboard, '6'            },
    { "7",             keyboard, '7'            }, { "8",             keyboard, '8'            }, { "9",             keyboard, '9'            },
    { "0",             keyboard, '0'            }, { "-",             keyboard, KEY_MINUS      }, { "=",             keyboard, KEY_EQUALS     },
    { "+",             keyboard, KEY_EQUALS     }, { "backspace",     keyboard, KEY_BACKSPACE  }, { "tab",           keyboard, KEY_TAB        },
    { "q",             keyboard, 'q'            }, { "w",             keyboard, 'w'            }, { "e",             keyboard, 'e'            },
    { "r",             keyboard, 'r'            }, { "t",             keyboard, 't'            }, { "y",             keyboard, 'y'            },
    { "u",             keyboard, 'u'            }, { "i",             keyboard, 'i'            }, { "o",             keyboard, 'o'            },
    { "p",             keyboard, 'p'            }, { "[",             keyboard, '['            }, { "]",             keyboard, ']'            },
    { "enter",         keyboard, KEY_ENTER      }, { "ctrl",          keyboard, KEY_RCTRL      }, { "a",             keyboard, 'a'            },
    { "s",             keyboard, 's'            }, { "d",             keyboard, 'd'            }, { "f",             keyboard, 'f'            },
    { "g",             keyboard, 'g'            }, { "h",             keyboard, 'h'            }, { "j",             keyboard, 'j'            },
    { "k",             keyboard, 'k'            }, { "l",             keyboard, 'l'            }, { ";",             keyboard, ';'            },
    { "\'",            keyboard, '\''           }, { "shift",         keyboard, KEY_RSHIFT     }, { "\\",            keyboard, '\\'           },
    { "z",             keyboard, 'z'            }, { "x",             keyboard, 'x'            }, { "c",             keyboard, 'c'            },
    { "v",             keyboard, 'v'            }, { "b",             keyboard, 'b'            }, { "n",             keyboard, 'n'            },
    { "m",             keyboard, 'm'            }, { ",",             keyboard, ','            }, { ".",             keyboard, '.'            },
    { "/",             keyboard, '/'            }, { "alt",           keyboard, KEY_RALT       }, { "space",         keyboard, ' '            },
    { "numlock",       keyboard, KEY_NUMLOCK    }, { "scrolllock",    keyboard, KEY_SCRLCK     }, { "home",          keyboard, KEY_HOME       },
    { "up",            keyboard, KEY_UPARROW    }, { "pageup",        keyboard, KEY_PGUP       }, { "left",          keyboard, KEY_LEFTARROW  },
    { "right",         keyboard, KEY_RIGHTARROW }, { "end",           keyboard, KEY_END        }, { "down",          keyboard, KEY_DOWNARROW  },
    { "pagedown",      keyboard, KEY_PGDN       }, { "insert",        keyboard, KEY_INS        }, { "delete",        keyboard, KEY_DEL        },
    { "mouse1",        mouse,    0              }, { "mouse2",        mouse,    1              }, { "mouse3",        mouse,    2              },
    { "mouse4",        mouse,    3              }, { "mouse5",        mouse,    4              }, { "mouse6",        mouse,    5              },
    { "mouse7",        mouse,    6              }, { "mouse8",        mouse,    7              }, { "wheelup",       mouse,    8              },
    { "wheeldown",     mouse,    9              }, { "dpadup",        gamepad,  1              }, { "dpaddown",      gamepad,  2              },
    { "dpadleft",      gamepad,  4              }, { "dpadright",     gamepad,  8              }, { "start",         gamepad,  16             },
    { "back",          gamepad,  32             }, { "leftthumb",     gamepad,  64             }, { "rightthumb",    gamepad,  128            },
    { "leftshoulder",  gamepad,  256            }, { "rightshoulder", gamepad,  512            }, { "lefttrigger",   gamepad,  1024           },
    { "righttrigger",  gamepad,  2048           }, { "gamepad1",      gamepad,  4096           }, { "gamepad2",      gamepad,  8192           },
    { "gamepad3",      gamepad,  16384          }, { "gamepad4",      gamepad,  32768          }, { "",              0,        0              }
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
boolean C_DeadZoneCondition(char *, char *, char *);
boolean C_GameCondition(char *, char *, char *);
boolean C_GammaCondition(char *, char *, char *);
boolean C_GiveCondition(char *, char *, char *);
boolean C_GodCondition(char *, char *, char *);
boolean C_GraphicDetailCondition(char *, char *, char *);
boolean C_IntegerCondition(char *, char *, char *);
boolean C_KillCondition(char *, char *, char *);
boolean C_MapCondition(char *, char *, char *);
boolean C_NoCondition(char *, char *, char *);
boolean C_SummonCondition(char *, char *, char *);
boolean C_VolumeCondition(char *, char *, char *);

void C_AlwaysRun(char *, char *, char *);
void C_Bind(char *, char *, char *);
void C_BloodSplats(char *, char *, char *);
void C_Boolean(char *, char *, char *);
void C_Clear(char *, char *, char *);
void C_CmdList(char *, char *, char *);
void C_ConBack(char *, char *, char *);
void C_CvarList(char *, char *, char *);
void C_DeadZone(char *, char *, char *);
void C_EndGame(char *, char *, char *);
void C_Gamma(char *, char *, char *);
void C_GamepadVibrate(char *, char *, char *);
void C_God(char *, char *, char *);
void C_Give(char *, char *, char *);
void C_GraphicDetail(char *, char *, char *);
void C_Help(char *, char *, char *);
void C_Hud(char *, char *, char *);
void C_Integer(char *, char *, char *);
void C_Kill(char *, char *, char *);
void C_Map(char *, char *, char *);
void C_NoClip(char *, char *, char *);
void C_NoTarget(char *, char *, char *);
void C_Quit(char *, char *, char *);
void C_ScreenSize(char *, char *, char *);
void C_ShowFPS(char *, char *, char *);
void C_String(char *, char *, char *);
void C_Summon(char *, char *, char *);
void C_Volume(char *, char *, char *);

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
    { "1.95",         29, 4 }, { "2.0",          30, 4 }, { "desktop",       0, 5 }, { "",              0, 0 }
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
    {
        /* name        */ "alwaysrun",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_AlwaysRun,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &alwaysrun,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ ALWAYSRUN_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "animatedliquid",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &animatedliquid,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ ANIMATEDLIQUID_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "bind",
        /* condition   */ C_BindCondition,
        /* function    */ C_Bind,
        /* parameters  */ 2,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "bind [~control~ [+~action~]]",
        /* description */ "Bind an action to a control."
    },

    {
        /* name        */ "bloodsplats",
        /* condition   */ C_BloodSplatsCondition,
        /* function    */ C_BloodSplats,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &bloodsplats,
        /* aliases     */ 2,
        /* minimum     */ BLOODSPLATS_MIN,
        /* maximum     */ BLOODSPLATS_MAX,
        /* default     */ BLOODSPLATS_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "brightmaps",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &brightmaps,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ BRIGHTMAPS_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "centerweapon",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &centerweapon,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ CENTERWEAPON_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "clear",
        /* condition   */ C_NoCondition,
        /* function    */ C_Clear,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "clear",
        /* description */ "Clear the console."
    },

    {
        /* name        */ "cmdlist",
        /* condition   */ C_NoCondition,
        /* function    */ C_CmdList,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "cmdlist",
        /* description */ "Display a list of console commands."
    },

    {
        /* name        */ "conback",
        /* condition   */ C_NoCondition,
        /* function    */ C_ConBack,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_STRING,
        /* variable    */ &conback,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "corpses_mirror",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &corpses_mirror,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ CORPSES_MIRROR_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "corpses_moreblood",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &corpses_moreblood,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ CORPSES_MOREBLOOD_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "corpses_slide",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &corpses_slide,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ CORPSES_SLIDE_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "corpses_smearblood",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &corpses_smearblood,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ CORPSES_SMEARBLOOD_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "cvarlist",
        /* condition   */ C_NoCondition,
        /* function    */ C_CvarList,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "cvarlist",
        /* description */ "Display a list of console variables."
    },

    {
        /* name        */ "dclick_use",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &dclick_use,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ DCLICKUSE_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "endgame",
        /* condition   */ C_GameCondition,
        /* function    */ C_EndGame,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "endgame",
        /* description */ "End a game."
    },

    {
        /* name        */ "episode",
        /* condition   */ C_IntegerCondition,
        /* function    */ C_Integer,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &selectedepisode,
        /* aliases     */ 1,
        /* minimum     */ EPISODE_MIN,
        /* maximum     */ EPISODE_MAX,
        /* default     */ EPISODE_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "expansion",
        /* condition   */ C_IntegerCondition,
        /* function    */ C_Integer,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &selectedexpansion,
        /* aliases     */ 1,
        /* minimum     */ EXPANSION_MIN,
        /* maximum     */ EXPANSION_MAX,
        /* default     */ EXPANSION_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "floatbob",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &floatbob,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ FLOATBOB_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "followmode",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN | CF_NOTSAVED,
        /* variable    */ &followmode,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "footclip",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &footclip,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ FOOTCLIP_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "fullscreen",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &fullscreen,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ FULLSCREEN_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "gamepad_leftdeadzone",
        /* condition   */ C_DeadZoneCondition,
        /* function    */ C_DeadZone,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_FLOAT_PERCENT,
        /* variable    */ &gamepadleftdeadzone_percent,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "gamepad_lefthanded",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &gamepadlefthanded,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ GAMEPADLEFTHANDED_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "gamepad_rightdeadzone",
        /* condition   */ C_DeadZoneCondition,
        /* function    */ C_DeadZone,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_FLOAT_PERCENT,
        /* variable    */ &gamepadrightdeadzone_percent,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "gamepad_sensitivity",
        /* condition   */ C_NoCondition,
        /* function    */ C_Integer,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &gamepadsensitivity,
        /* aliases     */ 0,
        /* minimum     */ GAMEPADSENSITIVITY_MIN,
        /* maximum     */ GAMEPADSENSITIVITY_MAX,
        /* default     */ GAMEPADSENSITIVITY_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "gamepad_vibrate",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &gamepadvibrate,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ GAMEPADVIBRATE_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "gammacorrectionlevel",
        /* condition   */ C_GammaCondition,
        /* function    */ C_Gamma,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &gammaindex,
        /* aliases     */ 4,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "god",
        /* condition   */ C_GodCondition,
        /* function    */ C_God,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "god",
        /* description */ "Toggle god mode on/off."
    },

    {
        /* name        */ "graphicdetail",
        /* condition   */ C_GraphicDetailCondition,
        /* function    */ C_GraphicDetail,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &graphicdetail,
        /* aliases     */ 3,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ GRAPHICDETAIL_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "grid",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &grid,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ GRID_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "help",
        /* condition   */ C_NoCondition,
        /* function    */ C_Help,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "help",
        /* description */ "Display the help screen."
    },

    {
        /* name        */ "homindicator",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &homindicator,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ HOMINDICATOR_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "hud",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Hud,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &hud,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ HUD_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idbeholda",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idbeholdl",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idbeholdi",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idbeholdr",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* description */ ""
    },

    {
        /* name        */ "idbeholds",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idbeholdv",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idchoppers",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idclev",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 1,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idclip",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "iddqd",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "iddt",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idfa",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idkfa",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idmus",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 1,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idmypos",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "idspispopd",
        /* condition   */ C_CheatCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ CT_CHEAT,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "iwadfolder",
        /* condition   */ C_NoCondition,
        /* function    */ C_String,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_STRING,
        /* variable    */ &iwadfolder,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "kill",
        /* condition   */ C_KillCondition,
        /* function    */ C_Kill,
        /* parameters  */ 1,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "kill [all|monsters|~type~]",
        /* description */ "Kill the player, all monsters or a type of monster."
    },

    {
        /* name        */ "map",
        /* condition   */ C_MapCondition,
        /* function    */ C_Map,
        /* parameters  */ 1,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ MAPCMDFORMAT,
        /* description */ "Warp to a map."
    },

    {
        /* name        */ "mapfixes",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &mapfixes,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ MAPFIXES_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "messages",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &messages,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ MESSAGES_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "mirrorweapons",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &mirrorweapons,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ MIRRORWEAPONS_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "mouse_sensitivity",
        /* condition   */ C_NoCondition,
        /* function    */ C_Integer,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &mousesensitivity,
        /* aliases     */ 0,
        /* minimum     */ MOUSESENSITIVITY_MIN,
        /* maximum     */ MOUSESENSITIVITY_MAX,
        /* default     */ MOUSESENSITIVITY_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "musicvolume",
        /* condition   */ C_VolumeCondition,
        /* function    */ C_Volume,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER_PERCENT,
        /* variable    */ &musicvolume_percent,
        /* aliases     */ 0,
        /* minimum     */ MUSICVOLUME_MIN,
        /* maximum     */ MUSICVOLUME_MAX,
        /* default     */ MUSICVOLUME_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "noclip",
        /* condition   */ C_GameCondition,
        /* function    */ C_NoClip,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "noclip",
        /* description */ "Toggle no clipping mode on/off."
    },

    {
        /* name        */ "notarget",
        /* condition   */ C_GameCondition,
        /* function    */ C_NoTarget,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "notarget",
        /* description */ "Toggle no target mode on/off."
    },

    {
        /* name        */ "novert",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &novert,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ NOVERT_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "quit",
        /* condition   */ C_NoCondition,
        /* function    */ C_Quit,
        /* parameters  */ 0,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "quit",
        /* description */ "Quit DOOM RETRO."
    },

    {
        /* name        */ "rotatemode",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &rotatemode,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ ROTATEMODE_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

#if defined(SDL20)
    {
        /* name        */ "scalequality",
        /* condition   */ C_NoCondition,
        /* function    */ C_String,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_STRING,
        /* variable    */ &scalequality,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },
#endif

    {
        /* name        */ "screenwidth",
        /* condition   */ C_IntegerCondition,
        /* function    */ C_Integer,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &screenwidth,
        /* aliases     */ 5,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ SCREENWIDTH_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "screenheight",
        /* condition   */ C_IntegerCondition,
        /* function    */ C_Integer,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &screenheight,
        /* aliases     */ 5,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ SCREENHEIGHT_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "screensize",
        /* condition   */ C_IntegerCondition,
        /* function    */ C_ScreenSize,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &screensize,
        /* aliases     */ 0,
        /* minimum     */ SCREENSIZE_MIN,
        /* maximum     */ SCREENSIZE_MAX,
        /* default     */ SCREENSIZE_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "sfxvolume",
        /* condition   */ C_VolumeCondition,
        /* function    */ C_Volume,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER_PERCENT,
        /* variable    */ &sfxvolume_percent,
        /* aliases     */ 0,
        /* minimum     */ SFXVOLUME_MIN,
        /* maximum     */ SFXVOLUME_MAX,
        /* default     */ SFXVOLUME_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "shadows",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &shadows,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ SHADOWS_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "showfps",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_ShowFPS,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN | CF_NOTSAVED,
        /* variable    */ &showfps,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ false,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "skilllevel",
        /* condition   */ C_IntegerCondition,
        /* function    */ C_Integer,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_INTEGER,
        /* variable    */ &selectedskilllevel,
        /* aliases     */ 1,
        /* minimum     */ SKILLLEVEL_MIN,
        /* maximum     */ SKILLLEVEL_MAX,
        /* default     */ SKILLLEVEL_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "smoketrails",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &smoketrails,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ SMOKETRAILS_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "summon",
        /* condition   */ C_SummonCondition,
        /* function    */ C_Summon,
        /* parameters  */ 1,
        /* type        */ CT_CMD,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ SUMMONCMDFORMAT,
        /* description */ "Summon a monster or object."
    },

    {
        /* name        */ "timidity_cfg_path",
        /* condition   */ C_NoCondition,
        /* function    */ C_String,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_STRING,
        /* variable    */ &timidity_cfg_path,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "translucency",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &translucency,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ TRANSLUCENCY_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "videodriver",
        /* condition   */ C_NoCondition,
        /* function    */ C_String,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_STRING,
        /* variable    */ &videodriver,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

#if defined(SDL20)
    {
        /* name        */ "vsync",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &vsync,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ VSYNC_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },
#endif

    {
        /* name        */ "widescreen",
        /* condition   */ C_BooleanCondition,
        /* function    */ C_Boolean,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_BOOLEAN,
        /* variable    */ &widescreen,
        /* aliases     */ 1,
        /* minimum     */ false,
        /* maximum     */ true,
        /* default     */ TRANSLUCENCY_DEFAULT,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "windowposition",
        /* condition   */ C_NoCondition,
        /* function    */ C_String,
        /* parameters  */ 1,
        /* type        */ CT_CVAR,
        /* flags       */ CF_STRING,
        /* variable    */ &windowposition,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    },

    {
        /* name        */ "",
        /* condition   */ C_NoCondition,
        /* function    */ NULL,
        /* parameters  */ 0,
        /* type        */ 0,
        /* flags       */ CF_NONE,
        /* variable    */ NULL,
        /* aliases     */ 0,
        /* minimum     */ 0,
        /* maximum     */ 0,
        /* default     */ 0,
        /* format      */ "",
        /* description */ ""
    }
};

//
// Cheat cmds
//
boolean C_CheatCondition(char *cmd, char *parm1, char *parm2)
{
    if (gamestate != GS_LEVEL)
        return false;
    if (!strcasecmp(cmd, "iddt") && !automapactive)
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
    int control = 0;

    while (controls[control].type)
    {
        if (controls[control].type == type && controls[control].value == value)
        {
            C_Print(output, CONSOLEOUTPUTCOLOR, "%i\t%s\t%s", count, controls[control].control,
                action);
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
                C_DisplayBinds(actions[action].action, *(int *)actions[action].keyboard1, keyboard,
                    count++);
            if (actions[action].keyboard2)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].keyboard2, keyboard,
                    count++);
            if (actions[action].mouse)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].mouse, mouse,
                    count++);
            if (actions[action].gamepad)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].gamepad, gamepad,
                    count++);
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
            int action = 0;

            if (!parm2[0])
            {
                while (actions[action].action[0])
                {
                    if (controls[control].type == keyboard && actions[action].keyboard1
                        && controls[control].value == *(int *)actions[action].keyboard1)
                        C_Print(output, CONSOLEOUTPUTCOLOR, "%s \"%s\"", controls[control].control,
                            actions[action].action);
                    else if (controls[control].type == keyboard && actions[action].keyboard2
                        && controls[control].value == *(int *)actions[action].keyboard2)
                        C_Print(output, CONSOLEOUTPUTCOLOR, "%s \"%s\"", controls[control].control,
                            actions[action].action);
                    else if (controls[control].type == mouse && actions[action].mouse
                        && controls[control].value == *(int *)actions[action].mouse)
                        C_Print(output, CONSOLEOUTPUTCOLOR, "%s \"%s\"", controls[control].control,
                            actions[action].action);
                    else if (controls[control].type == gamepad && actions[action].gamepad
                        && controls[control].value == *(int *)actions[action].gamepad)
                        C_Print(output, CONSOLEOUTPUTCOLOR, "%s \"%s\"", controls[control].control,
                            actions[action].action);
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
                            if (controls[control].value != *(int *)actions[action].keyboard1
                                && controls[control].value != *(int *)actions[action].keyboard2)
                            {
                                if (*(int *)actions[action].keyboard1)
                                {
                                    if (actions[action].keyboard2)
                                    {
                                        if (*(int *)actions[action].keyboard2)
                                            *(int *)actions[action].keyboard1
                                                = *(int *)actions[action].keyboard2;
                                        *(int *)actions[action].keyboard2 = controls[control].value;
                                    }
                                    else
                                        *(int *)actions[action].keyboard1 = controls[control].value;
                                }
                                else
                                    *(int *)actions[action].keyboard1 = controls[control].value;
                            }
                            break;
                        case mouse:
                            *(int *)actions[action].mouse = controls[control].value;
                            break;
                        case gamepad:
                            *(int *)actions[action].gamepad = controls[control].value;
                            break;
                    }
                    M_SaveDefaults();
                    C_Print(output, CONSOLEOUTPUTCOLOR, "%s \"%s\"", parm1, parm2);
                }
            }
        }
    }
}

//
// Boolean cvars
//
boolean C_BooleanCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, 1) >= 0);
}

void C_Boolean(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_BOOLEAN))
        {
            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                int     value = C_LookupValueFromAlias(parm1, 1);

                if (value == 0 || value == 1)
                {
                    *(boolean *)consolecmds[i].variable = !!value;
                    if (!(consolecmds[i].flags & CF_NOTSAVED))
                        M_SaveDefaults();
                    C_Print(output, CONSOLEOUTPUTCOLOR, "%s is now %s.", cmd, parm1);
                }
            }
            else
                C_Print(output, CONSOLEOUTPUTCOLOR, "%s is %s.", cmd,
                    (*(boolean *)consolecmds[i].variable ? "on" : "off"));
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
    int value = 0;

    return (!parm1[0] || C_LookupValueFromAlias(parm1, 2) >= 0 || sscanf(parm1, "%i", &value));
}

void C_BloodSplats(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 2);

        if (value < 0)
            sscanf(parm1, "%i", &value);
        if (value >= 0)
        {
            bloodsplats = value;
            M_SaveDefaults();
            P_BloodSplatSpawner = ((bloodsplats == UNLIMITED ? P_SpawnBloodSplat :
                (bloodsplats ? P_SpawnBloodSplat2 : P_NullBloodSplatSpawner)));
            C_Print(output, CONSOLEOUTPUTCOLOR, "bloodsplats is now %s.", parm1);
        }
    }
    else
    {
        char    *alias = C_LookupAliasFromValue(bloodsplats, 2);

        if (alias)
            C_Print(output, CONSOLEOUTPUTCOLOR, "bloodsplats is %s.", alias);
        else
            C_Print(output, CONSOLEOUTPUTCOLOR, "bloodsplats is %i.", bloodsplats);
    }
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

    while (consolecmds[i].name[0])
    {
        if (consolecmds[i].type == CT_CMD)
            C_Print(output, CONSOLEOUTPUTCOLOR, "%i\t%s\t\t%s", count++, consolecmds[i].format,
                 consolecmds[i].description);
        ++i;
    }
}

//
// CONBACK cvar
//
void C_ConBack(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        if (R_CheckFlatNumForName(parm1) >= 0)
        {
            conback = strdup(parm1);
            consolebackground = W_CacheLumpName(parm1, PU_CACHE);
            M_SaveDefaults();
            C_Print(output, CONSOLEOUTPUTCOLOR, "conback is now \"%s\".", conback);
        }
    }
    else
        C_Print(output, CONSOLEOUTPUTCOLOR, "conback is \"%s\".", conback);
}

//
// CVARLIST cmd
//
void C_CvarList(char *cmd, char *parm1, char *parm2)
{
    int i = 0;
    int count = 1;

    while (consolecmds[i].name[0])
    {
        if (consolecmds[i].type == CT_CVAR)
        {
            if (consolecmds[i].flags & CF_BOOLEAN)
                C_Print(output, CONSOLEOUTPUTCOLOR, "%i\t%s\t\t%s", count++, consolecmds[i].name,
                    C_LookupAliasFromValue(*(boolean *)consolecmds[i].variable,
                    consolecmds[i].aliases));
            else if (consolecmds[i].flags & CF_INTEGER)
            {
                char *alias = C_LookupAliasFromValue(*(int *)consolecmds[i].variable,
                              consolecmds[i].aliases);

                if (alias)
                    C_Print(output, CONSOLEOUTPUTCOLOR, "%i\t%s\t\t%s", count++,
                        consolecmds[i].name, alias);
                else
                    C_Print(output, CONSOLEOUTPUTCOLOR, "%i\t%s\t\t%i", count++,
                        consolecmds[i].name, *(int *)consolecmds[i].variable);
            }
            else if (consolecmds[i].flags & CF_INTEGER_PERCENT)
                C_Print(output, CONSOLEOUTPUTCOLOR, "%i\t%s\t\t%i%%", count++,
                    consolecmds[i].name, *(int *)consolecmds[i].variable);
            else if (consolecmds[i].flags & CF_STRING)
                C_Print(output, CONSOLEOUTPUTCOLOR, "%i\t%s\t\t\"%s\"", count++,
                    consolecmds[i].name, *(char **)consolecmds[i].variable);
            else if (consolecmds[i].flags & CF_FLOAT_PERCENT)
                C_Print(output, CONSOLEOUTPUTCOLOR, "%i\t%s\t\t%s%%", count++, consolecmds[i].name,
                    striptrailingzero(*(float *)consolecmds[i].variable));
        }
        ++i;
    }
}

//
// GAMEPAD_LEFTDEADZONE and GAMEPAD_RIGHTDEADZONE cvars
//
boolean C_DeadZoneCondition(char *cmd, char *parm1, char *parm2)
{
    float value;

    if (!parm1[0])
        return true;
    if (parm1[strlen(parm1) - 1] == '%')
        parm1[strlen(parm1) - 1] = 0;
    return sscanf(parm1, "%f", &value);
}

void C_DeadZone(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        float   value = 0;

        if (parm1[strlen(parm1) - 1] == '%')
            parm1[strlen(parm1) - 1] = 0;
        sscanf(parm1, "%f", &value);

        if (!strcasecmp(cmd, "gamepad_leftdeadzone"))
        {
            gamepadleftdeadzone_percent = value;
            gamepadleftdeadzone = (int)(BETWEENF(GAMEPADLEFTDEADZONE_MIN,
                gamepadleftdeadzone_percent,
                GAMEPADLEFTDEADZONE_MAX) * (float)SHRT_MAX / 100.0f);
        }
        else
        {
            gamepadrightdeadzone_percent = value;
            gamepadrightdeadzone = (int)(BETWEENF(GAMEPADRIGHTDEADZONE_MIN,
                gamepadrightdeadzone_percent,
                GAMEPADRIGHTDEADZONE_MAX) * (float)SHRT_MAX / 100.0f);
        }

        M_SaveDefaults();
        C_Print(output, CONSOLEOUTPUTCOLOR, "%s is now %s%%.", cmd, striptrailingzero(value));
    }
    else
    {
        float   value = 0;

        if (!strcasecmp(cmd, "gamepad_leftdeadzone"))
            value = gamepadleftdeadzone_percent;
        else
            value = gamepadrightdeadzone_percent;

        C_Print(output, CONSOLEOUTPUTCOLOR, "%s is %s%%.", cmd, striptrailingzero(value));
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
    if (parm1[0])
    {
        int value = C_LookupValueFromAlias(parm1, 4);

        if (value != -1)
        {
            gammaindex = value;

            I_SetPalette((byte *)W_CacheLumpName("PLAYPAL", PU_CACHE) + st_palette * 768);

            M_SaveDefaults();

            if (gammaindex == 10)
                C_Print(output, CONSOLEOUTPUTCOLOR, "\"gammacorrectionlevel\" is now off.");
            else
                C_Print(output, CONSOLEOUTPUTCOLOR, "\"gammacorrectionlevel\" is now %.2f.",
                    gammalevels[gammaindex]);
        }
    }
    else
    {
        if (gammaindex == 10)
            C_Print(output, CONSOLEOUTPUTCOLOR, "\"gammacorrectionlevel\" is off.");
        else
            C_Print(output, CONSOLEOUTPUTCOLOR, "\"gammacorrectionlevel\" is %.2f.",
                gammalevels[gammaindex]);
    }
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
    C_Print(output, CONSOLEOUTPUTCOLOR,
        ((player->cheats & CF_GODMODE) ? s_STSTR_GODON : s_STSTR_GODOFF));
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
    if (parm1[0])
    {
        int value = C_LookupValueFromAlias(parm1, 3);

        if (value == 0 || value == 1)
        {
            graphicdetail = !!value;
            M_SaveDefaults();
            C_Print(output, CONSOLEOUTPUTCOLOR, "graphicdetail is now %s.", parm1);
        }
    }
    else
        C_Print(output, CONSOLEOUTPUTCOLOR, "graphicdetail is %s.",
            C_LookupAliasFromValue(graphicdetail, 3));
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
// Integer cvars
//
boolean C_IntegerCondition(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_INTEGER))
        {
            int value = -1;
            
            sscanf(parm1, "%i", &value);

            return (value >= consolecmds[i].minimumvalue && value <= consolecmds[i].maximumvalue);
        }
        ++i;
    }
    return false;
}

void C_Integer(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_INTEGER))
        {
            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                int     value = C_LookupValueFromAlias(parm1, 1);

                if (value < 0)
                    sscanf(parm1, "%i", &value);

                if (value >= 0)
                {
                    *(int *)consolecmds[i].variable = value;
                    if (!(consolecmds[i].flags & CF_NOTSAVED))
                        M_SaveDefaults();
                    C_Print(output, CONSOLEOUTPUTCOLOR, "%s is now %s.", cmd, parm1);
                }
            }
            else
            {
                char    *alias = C_LookupAliasFromValue(*(int *)consolecmds[i].variable,
                                 consolecmds[i].aliases);

                if (alias)
                    C_Print(output, CONSOLEOUTPUTCOLOR, "%s is %s.", parm1, alias);
                else
                    C_Print(output, CONSOLEOUTPUTCOLOR, "%s is %i.", cmd,
                        *(int *)consolecmds[i].variable);
            }
        }
        ++i;
    }
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
        C_Print(output, CONSOLEOUTPUTCOLOR, "Player killed.");
    }
    else
    {
        int             i, j;
        int             kills = 0;

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
            int type = P_FindDoomedNum(killcmdtype);

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
        if (!kills)
            C_Print(output, CONSOLEOUTPUTCOLOR, "No monsters killed.");
        else
            C_Print(output, CONSOLEOUTPUTCOLOR, "%i monster%s killed.", kills,
                kills == 1 ? "" : "s");
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
        return true;

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
    if (!parm1[0])
    {
        C_Print(output, CONSOLEOUTPUTCOLOR, MAPCMDFORMAT);
        return;
    }

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
// MUSICVOLUME and SFXVOLUME cvars
//
boolean C_VolumeCondition(char *cmd, char *parm1, char *parm2)
{
    int value = -1;

    if (!parm1[0])
        return true;
    if (parm1[strlen(parm1) - 1] == '%')
        parm1[strlen(parm1) - 1] = 0;

    sscanf(parm1, "%i", &value);

    return ((!strcasecmp(cmd, "musicvolume") && value >= MUSICVOLUME_MIN && value <= MUSICVOLUME_MAX)
        || (!strcasecmp(cmd, "sfxvolume") && value >= SFXVOLUME_MIN && value <= SFXVOLUME_MAX));
}

void C_Volume(char *cmd, char *parm1, char *parm2)
{
    int value = 0;

    if (parm1[0])
    {
        if (parm1[strlen(parm1) - 1] == '%')
            parm1[strlen(parm1) - 1] = 0;
        sscanf(parm1, "%i", &value);

        if (!strcasecmp(cmd, "musicvolume"))
        {
            musicvolume_percent = value;
            musicVolume = (BETWEEN(MUSICVOLUME_MIN, musicvolume_percent,
                MUSICVOLUME_MAX) * 15 + 50) / 100;
            S_SetMusicVolume((int)(musicVolume * (127.0f / 15.0f)));
        }
        else
        {
            sfxvolume_percent = value;
            sfxVolume = (BETWEEN(SFXVOLUME_MIN, sfxvolume_percent, SFXVOLUME_MAX) * 15 + 50) / 100;
            S_SetSfxVolume((int)(sfxVolume * (127.0f / 15.0f)));
        }

        M_SaveDefaults();
        C_Print(output, CONSOLEOUTPUTCOLOR, "%s is now %i%%.", cmd, value);
    }
    else
    {
        if (!strcasecmp(cmd, "musicvolume"))
            value = musicvolume_percent;
        else
            value = sfxvolume_percent;

        C_Print(output, CONSOLEOUTPUTCOLOR, "%s is %i%%.", cmd, value);
    }
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
    C_Print(output, CONSOLEOUTPUTCOLOR,
        ((players[displayplayer].cheats & CF_NOTARGET) ? s_STSTR_NTON : s_STSTR_NTOFF));
}

//
// QUIT cmd
//
void C_Quit(char *cmd, char *parm1, char *parm2)
{
    I_Quit(true);
}

//
// SCREENSIZE cvar
//
void C_ScreenSize(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = -1;

        sscanf(parm1, "%i", &value);

        if (value >= SCREENSIZE_MIN && value <= SCREENSIZE_MAX)
        {
            if (widescreen || (returntowidescreen && gamestate != GS_LEVEL))
            {
                if (value == SCREENSIZE_MAX)
                    --value;
                else if (value <= SCREENSIZE_MAX - 1)
                {
                    hud = true;
                    ToggleWidescreen(false);
                }
            }
            else
            {
                if (value >= SCREENSIZE_MAX - 1)
                {
                    if (gamestate != GS_LEVEL)
                    {
                        returntowidescreen = true;
                        hud = true;
                    }
                    else
                    {
                        ToggleWidescreen(true);
                        if (widescreen)
                            value = SCREENSIZE_MAX - 1;
                    }
                }
            }
            screensize = value;
            M_SaveDefaults();
            R_SetViewSize(screensize);
            C_Print(output, CONSOLEOUTPUTCOLOR, "screensize is now %i.", screensize);
        }
    }
    else
        C_Print(output, CONSOLEOUTPUTCOLOR, "screensize is %i.", screensize);
}

//
// SHOWFPS cvar
//
void C_ShowFPS(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 1);

        if (value == 0 || value == 1)
        {
            showfps = !!value;
            C_Print(output, CONSOLEOUTPUTCOLOR, "showfps is now %s.", parm1);
        }
    }
    else
        C_Print(output, CONSOLEOUTPUTCOLOR, "showfps is %s.", (showfps ? "on" : "off"));
}

//
// String cvars
//
void C_String(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_STRING))
        {
            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                *(char **)consolecmds[i].variable = strdup(parm1);
                M_SaveDefaults();
                C_Print(output, CONSOLEOUTPUTCOLOR, "%s is now \"%s\".", cmd, parm1);
            }
            else
                C_Print(output, CONSOLEOUTPUTCOLOR, "%s is \"%s\".", cmd,
                    *(char **)consolecmds[i].variable);
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
        return true;

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
    if (!parm1[0])
    {
        C_Print(output, CONSOLEOUTPUTCOLOR, SUMMONCMDFORMAT);
        return;
    }
    else
    {
        mobj_t      *player = players[displayplayer].mo;
        fixed_t     x = player->x;
        fixed_t     y = player->y;
        angle_t     angle = player->angle >> ANGLETOFINESHIFT;
        mobj_t      *thing = P_SpawnMobj(x + 100 * finecosine[angle], y + 100 * finesine[angle],
            ONFLOORZ, P_FindDoomedNum(summoncmdtype));

        thing->angle = R_PointToAngle2(thing->x, thing->y, x, y);
    }
}
