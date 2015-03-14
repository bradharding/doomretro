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

#define MAPCMDFORMAT    "E~x~M~y~|MAP~xy~"
#define SPAWNCMDFORMAT  "~type~"

#define NONE_MIN        0
#define NONE_MAX        0
#define NONE_DEFAULT    0

extern boolean  alwaysrun;
extern boolean  animatedliquid;
extern int      bloodsplats;
extern boolean  brightmaps;
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
#if defined(SDL20)
extern int      monitor;
#endif
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
extern char     *pixelsize;
extern int      pixelwidth;
extern int      playerbob;
extern boolean  rotatemode;
extern int      runcount;
extern int      screenheight;
extern char     *screenresolution;
extern int      screenwidth;
extern int      selectedepisode;
extern int      selectedexpansion;
extern int      selectedsavegame;
extern int      selectedskilllevel;
extern boolean  shadows;
extern boolean  smoketrails;
extern int      snd_maxslicetime_ms;
extern boolean  spritefixes;
extern char     *timidity_cfg_path;
extern boolean  translucency;
extern char     *videodriver;
extern boolean  widescreen;
extern int      windowheight;
extern char     *windowposition;
extern char     *windowsize;
extern int      windowwidth;

#if defined(SDL20)
extern char     *scaledriver;
extern char     *scalequality;
extern boolean  vsync;
#endif

control_t controls[] =
{
    { "1",             keyboard, '1'            }, { "2",             keyboard, '2'            },
    { "3",             keyboard, '3'            }, { "4",             keyboard, '4'            },
    { "5",             keyboard, '5'            }, { "6",             keyboard, '6'            },
    { "7",             keyboard, '7'            }, { "8",             keyboard, '8'            },
    { "9",             keyboard, '9'            }, { "0",             keyboard, '0'            },
    { "-",             keyboard, KEY_MINUS      }, { "=",             keyboard, KEY_EQUALS     },
    { "+",             keyboard, KEY_EQUALS     }, { "backspace",     keyboard, KEY_BACKSPACE  },
    { "tab",           keyboard, KEY_TAB        }, { "q",             keyboard, 'q'            },
    { "w",             keyboard, 'w'            }, { "e",             keyboard, 'e'            },
    { "r",             keyboard, 'r'            }, { "t",             keyboard, 't'            },
    { "y",             keyboard, 'y'            }, { "u",             keyboard, 'u'            },
    { "i",             keyboard, 'i'            }, { "o",             keyboard, 'o'            },
    { "p",             keyboard, 'p'            }, { "[",             keyboard, '['            },
    { "]",             keyboard, ']'            }, { "enter",         keyboard, KEY_ENTER      },
    { "ctrl",          keyboard, KEY_RCTRL      }, { "a",             keyboard, 'a'            },
    { "s",             keyboard, 's'            }, { "d",             keyboard, 'd'            },
    { "f",             keyboard, 'f'            }, { "g",             keyboard, 'g'            },
    { "h",             keyboard, 'h'            }, { "j",             keyboard, 'j'            },
    { "k",             keyboard, 'k'            }, { "l",             keyboard, 'l'            },
    { ";",             keyboard, ';'            }, { "\'",            keyboard, '\''           },
    { "shift",         keyboard, KEY_RSHIFT     }, { "\\",            keyboard, '\\'           },
    { "z",             keyboard, 'z'            }, { "x",             keyboard, 'x'            },
    { "c",             keyboard, 'c'            }, { "v",             keyboard, 'v'            },
    { "b",             keyboard, 'b'            }, { "n",             keyboard, 'n'            },
    { "m",             keyboard, 'm'            }, { ",",             keyboard, ','            },
    { ".",             keyboard, '.'            }, { "/",             keyboard, '/'            },
    { "alt",           keyboard, KEY_RALT       }, { "space",         keyboard, ' '            },
    { "numlock",       keyboard, KEY_NUMLOCK    }, { "scrolllock",    keyboard, KEY_SCRLCK     },
    { "home",          keyboard, KEY_HOME       }, { "up",            keyboard, KEY_UPARROW    },
    { "pageup",        keyboard, KEY_PGUP       }, { "left",          keyboard, KEY_LEFTARROW  },
    { "right",         keyboard, KEY_RIGHTARROW }, { "end",           keyboard, KEY_END        },
    { "down",          keyboard, KEY_DOWNARROW  }, { "pagedown",      keyboard, KEY_PGDN       },
    { "insert",        keyboard, KEY_INS        }, { "delete",        keyboard, KEY_DEL        },
    { "mouse1",        mouse,    0              }, { "mouse2",        mouse,    1              },
    { "mouse3",        mouse,    2              }, { "mouse4",        mouse,    3              },
    { "mouse5",        mouse,    4              }, { "mouse6",        mouse,    5              },
    { "mouse7",        mouse,    6              }, { "mouse8",        mouse,    7              },
    { "wheelup",       mouse,    8              }, { "wheeldown",     mouse,    9              },
    { "dpadup",        gamepad,  1              }, { "dpaddown",      gamepad,  2              },
    { "dpadleft",      gamepad,  4              }, { "dpadright",     gamepad,  8              },
    { "start",         gamepad,  16             }, { "back",          gamepad,  32             },
    { "leftthumb",     gamepad,  64             }, { "rightthumb",    gamepad,  128            },
    { "leftshoulder",  gamepad,  256            }, { "rightshoulder", gamepad,  512            },
    { "lefttrigger",   gamepad,  1024           }, { "righttrigger",  gamepad,  2048           },
    { "gamepad1",      gamepad,  4096           }, { "gamepad2",      gamepad,  8192           },
    { "gamepad3",      gamepad,  16384          }, { "gamepad4",      gamepad,  32768          },
    { "",              0,        0              }
};

action_t actions[] = 
{
    { "+automap",     &key_automap,            NULL,              NULL,              &gamepadautomap           },
    { "+back",        &key_down,               &key_down2,        NULL,              NULL                      },
    { "+clearmark",   &key_automap_clearmark,  NULL,              NULL,              &gamepadautomapclearmark  },
    { "+fire",        &key_fire,               NULL,              &mousebfire,       &gamepadfire              },
    { "+followmode",  &key_automap_followmode, NULL,              NULL,              &gamepadautomapfollowmode },
    { "+forward",     &key_up,                 &key_up2,          &mousebforward,    NULL                      },
    { "+grid",        &key_automap_grid,       NULL,              NULL,              &gamepadautomapgrid       },
    { "+left",        &key_left,               NULL,              NULL,              NULL                      },
    { "+mark",        &key_automap_mark,       NULL,              NULL,              &gamepadautomapmark       },
    { "+maxzoom",     &key_automap_maxzoom,    NULL,              NULL,              &gamepadautomapmaxzoom    },
    { "+menu",        NULL,                    NULL,              NULL,              &gamepadmenu              },
    { "+nextweapon",  &key_nextweapon,         NULL,              &mousebnextweapon, &gamepadnextweapon        },
    { "+prevweapon",  &key_prevweapon,         NULL,              &mousebprevweapon, &gamepadprevweapon        },
    { "+right",       &key_right,              NULL,              NULL,              NULL                      },
    { "+rotatemode",  &key_automap_rotatemode, NULL,              NULL,              &gamepadautomaprotatemode },
    { "+run",         &key_run,                NULL,              NULL,              &gamepadrun               },
    { "+strafe",      &key_strafe,             NULL,              &mousebstrafe,     NULL                      },
    { "+strafeleft",  &key_strafeleft,         &key_strafeleft2,  NULL,              NULL                      },
    { "+straferight", &key_straferight,        &key_straferight2, NULL,              NULL                      },
    { "+use",         &key_use,                NULL,              &mousebuse,        &gamepaduse               },
    { "+weapon1",     &key_weapon1,            NULL,              NULL,              &gamepadweapon1           },
    { "+weapon2",     &key_weapon2,            NULL,              NULL,              &gamepadweapon2           },
    { "+weapon3",     &key_weapon3,            NULL,              NULL,              &gamepadweapon3           },
    { "+weapon4",     &key_weapon4,            NULL,              NULL,              &gamepadweapon4           },
    { "+weapon5",     &key_weapon5,            NULL,              NULL,              &gamepadweapon5           },
    { "+weapon6",     &key_weapon6,            NULL,              NULL,              &gamepadweapon6           },
    { "+weapon7",     &key_weapon7,            NULL,              NULL,              &gamepadweapon7           },
    { "+zoomin",      &key_automap_zoomin,     NULL,              NULL,              &gamepadautomapzoomin     },
    { "+zoomout",     &key_automap_zoomout,    NULL,              NULL,              &gamepadautomapzoomout    },
    { "",             NULL,                    NULL,              NULL,              NULL                      }
};

boolean C_BindCondition(char *, char *, char *);
boolean C_BloodSplatsCondition(char *, char *, char *);
boolean C_BoolCondition(char *, char *, char *);
boolean C_CheatCondition(char *, char *, char *);
boolean C_ConBackCondition(char *, char *, char *);
boolean C_DeadZoneCondition(char *, char *, char *);
boolean C_FloatCondition(char *, char *, char *);
boolean C_GameCondition(char *, char *, char *);
boolean C_GammaCondition(char *, char *, char *);
boolean C_GiveCondition(char *, char *, char *);
boolean C_GodCondition(char *, char *, char *);
boolean C_GraphicDetailCondition(char *, char *, char *);
boolean C_IntCondition(char *, char *, char *);
boolean C_KillCondition(char *, char *, char *);
boolean C_MapCondition(char *, char *, char *);
boolean C_NoCondition(char *, char *, char *);
boolean C_SpawnCondition(char *, char *, char *);
boolean C_VolumeCondition(char *, char *, char *);

void C_AlwaysRun(char *, char *, char *);
void C_Bind(char *, char *, char *);
void C_BloodSplats(char *, char *, char *);
void C_Bool(char *, char *, char *);
void C_Clear(char *, char *, char *);
void C_CmdList(char *, char *, char *);
void C_ConBack(char *, char *, char *);
void C_ConDump(char *, char *, char *);
void C_CvarList(char *, char *, char *);
void C_DeadZone(char *, char *, char *);
void C_EndGame(char *, char *, char *);
void C_ExitMap(char *, char *, char *);
void C_Float(char *, char *, char *);
void C_Gamma(char *, char *, char *);
void C_GamepadVibrate(char *, char *, char *);
void C_God(char *, char *, char *);
void C_Give(char *, char *, char *);
void C_GraphicDetail(char *, char *, char *);
void C_Help(char *, char *, char *);
void C_Hud(char *, char *, char *);
void C_Int(char *, char *, char *);
void C_Kill(char *, char *, char *);
void C_Map(char *, char *, char *);
void C_NoClip(char *, char *, char *);
void C_NoTarget(char *, char *, char *);
void C_PixelSize(char *, char *, char *);
void C_Quit(char *, char *, char *);
void C_ScreenSize(char *, char *, char *);
void C_ScreenResolution(char *, char *, char *);
void C_ShowFPS(char *, char *, char *);
void C_Spawn(char *, char *, char *);
void C_Str(char *, char *, char *);
void C_Time(char *, char *, char *);
void C_Volume(char *, char *, char *);
void C_WindowPosition(char *, char *, char *);
void C_WindowSize(char *, char *, char *);

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

#define CMD(name, cond, func, parms, form, desc) \
    { #name, cond, func, parms, CT_CMD, CF_NONE, NULL, 0, 0, 0, 0, form, desc }
#define CMD_CHEAT(name, parms) \
    { #name, C_CheatCondition, NULL, parms, CT_CHEAT, CF_NONE, NULL, 0, 0, 0, 0, "", "" }
#define CVAR_BOOL(name, cond, func, var, val) \
    { #name, cond, func, 1, CT_CVAR, CF_BOOLEAN, &var, 1, false, true, val##_DEFAULT, "", "" }
#define CVAR_INT(name, cond, func, flags, var, aliases, val) \
    { #name, cond, func, 1, CT_CVAR, CF_INTEGER | flags, &var, aliases, val##_MIN, val##_MAX, val##_DEFAULT, "", "" }
#define CVAR_FLOAT(name, cond, func, flags, var) \
    { #name, cond, func, 1, CT_CVAR, CF_FLOAT | flags, &var, 0, 0, 0, 0, "", "" }
#define CVAR_POS(name, cond, func, var) \
    { #name, cond, func, 1, CT_CVAR, CF_POSITION, &var, 0, 0, 0, 0, "", "" }
#define CVAR_SIZE(name, cond, func, var) \
    { #name, cond, func, 1, CT_CVAR, CF_SIZE, &var, 0, 0, 0, 0, "", "" }
#define CVAR_STR(name, cond, func, var) \
    { #name, cond, func, 1, CT_CVAR, CF_STRING, &var, 0, 0, 0, 0, "", "" }
#define CVAR_TIME(name, cond, func, var) \
    { #name, cond, func, 1, CT_CVAR, CF_TIME | CF_READONLY, &var, 0, 0, 0, 0, "", "" }

int numconsolecmds;

consolecmd_t consolecmds[] =
{
    CVAR_BOOL (am_followmode, C_BoolCondition, C_Bool, followmode, NONE),
    CVAR_BOOL (am_grid, C_BoolCondition, C_Bool, grid, GRID),
    CVAR_BOOL (am_rotatemode, C_BoolCondition, C_Bool, rotatemode, ROTATEMODE),
    CMD       (bind, C_BindCondition, C_Bind, 2, "[~control~ [+~action~]]", "Bind an action to a control."),
    CMD       (clear, C_NoCondition, C_Clear, 0, "", "Clear the console."),
    CMD       (cmdlist, C_NoCondition, C_CmdList, 1, "[~pattern~]", "Display a list of console commands."),
    CVAR_STR  (conback, C_ConBackCondition, C_ConBack, conback),
    CMD       (condump, C_NoCondition, C_ConDump, 1, "[~filename~]", "Dump the console to a file."),
    CMD       (cvarlist, C_NoCondition, C_CvarList, 1, "[~pattern~]", "Display a list of console variables."),
    CMD       (endgame, C_GameCondition, C_EndGame, 0, "", "End a game."),
    CVAR_INT  (episode, C_IntCondition, C_Int, CF_NONE, selectedepisode, 0, EPISODE),
    CMD       (exitmap, C_GameCondition, C_ExitMap, 0, "", "Exit the current map."),
    CVAR_INT  (expansion, C_IntCondition, C_Int, CF_NONE, selectedexpansion, 0, EXPANSION),
    CVAR_TIME (gametime, C_NoCondition, C_Time, gametic),
    CMD       (god, C_GodCondition, C_God, 1, "[on|off]", "Toggle god mode on/off."),
    CVAR_FLOAT(gp_deadzone_left, C_DeadZoneCondition, C_DeadZone, CF_PERCENT, gamepadleftdeadzone_percent),
    CVAR_FLOAT(gp_deadzone_right, C_DeadZoneCondition, C_DeadZone, CF_PERCENT, gamepadrightdeadzone_percent),
    CVAR_INT  (gp_sensitivity, C_NoCondition, C_Int, CF_NONE, gamepadsensitivity, 0, GAMEPADSENSITIVITY),
    CVAR_BOOL (gp_swapthumbsticks, C_BoolCondition, C_Bool, gamepadlefthanded, GAMEPADLEFTHANDED),
    CVAR_BOOL (gp_vibrate, C_BoolCondition, C_Bool, gamepadvibrate, GAMEPADVIBRATE),
    CMD       (help, C_NoCondition, C_Help, 0, "", "Display the help screen."),
    CMD_CHEAT (idbeholda, 0),
    CMD_CHEAT (idbeholdl, 0),
    CMD_CHEAT (idbeholdi, 0),
    CMD_CHEAT (idbeholdr, 0),
    CMD_CHEAT (idbeholds, 0),
    CMD_CHEAT (idbeholdv, 0),
    CMD_CHEAT (idchoppers, 0),
    CMD_CHEAT (idclev, 1),
    CMD_CHEAT (idclip, 0),
    CMD_CHEAT (iddqd, 0),
    CMD_CHEAT (iddt, 0),
    CMD_CHEAT (idfa, 0),
    CMD_CHEAT (idkfa, 0),
    CMD_CHEAT (idmus, 1),
    CMD_CHEAT (idmypos, 0),
    CMD_CHEAT (idspispopd, 0),
    CVAR_STR  (iwadfolder, C_NoCondition, C_Str, iwadfolder),
    CMD       (kill, C_KillCondition, C_Kill, 1, "[all|monsters|~type~]", "Kill the player, all monsters or a type of monster."),
    CVAR_FLOAT(m_acceleration, C_FloatCondition, C_Float, CF_NONE, mouse_acceleration),
    CVAR_BOOL (m_doubleclick_use, C_BoolCondition, C_Bool, dclick_use, DCLICKUSE),
    CVAR_BOOL (m_novertical, C_BoolCondition, C_Bool, novert, NOVERT),
    CVAR_INT  (m_sensitivity, C_IntCondition, C_Int, CF_NONE, mousesensitivity, 0, MOUSESENSITIVITY),
    CVAR_INT  (m_threshold, C_IntCondition, C_Int, CF_NONE, mouse_threshold, 0, MOUSETHRESHOLD),
    CMD       (map, C_MapCondition, C_Map, 1, MAPCMDFORMAT, "Warp to a map."),
    CVAR_BOOL (mapfixes, C_BoolCondition, C_Bool, mapfixes, MAPFIXES),
    CVAR_TIME (maptime, C_NoCondition, C_Time, leveltime),
    CVAR_BOOL (messages, C_BoolCondition, C_Bool, messages, MESSAGES),
    CMD       (noclip, C_GameCondition, C_NoClip, 1, "[on|off]", "Toggle no clipping mode on/off."),
    CMD       (notarget, C_GameCondition, C_NoTarget, 1, "[on|off]", "Toggle no target mode on/off."),
    CVAR_BOOL (pm_alwaysrun, C_BoolCondition, C_AlwaysRun, alwaysrun, ALWAYSRUN),
    CVAR_BOOL (pm_centerweapon, C_BoolCondition, C_Bool, centerweapon, CENTERWEAPON),
    CVAR_INT  (pm_walkbob, C_NoCondition, C_Int, CF_PERCENT, playerbob, 0, PLAYERBOB),
    CMD       (quit, C_NoCondition, C_Quit, 0, "", "Quit "PACKAGE_NAME"."),
    CVAR_INT  (r_bloodsplats, C_BloodSplatsCondition, C_BloodSplats, CF_NONE, bloodsplats, 7, BLOODSPLATS),
    CVAR_BOOL (r_brightmaps, C_BoolCondition, C_Bool, brightmaps, BRIGHTMAPS),
    CVAR_BOOL (r_corpses_mirrored, C_BoolCondition, C_Bool, corpses_mirror, CORPSES_MIRROR),
    CVAR_BOOL (r_corpses_moreblood, C_BoolCondition, C_Bool, corpses_moreblood, CORPSES_MOREBLOOD),
    CVAR_BOOL (r_corpses_slide, C_BoolCondition, C_Bool, corpses_slide, CORPSES_SLIDE),
    CVAR_BOOL (r_corpses_smearblood, C_BoolCondition, C_Bool, corpses_smearblood, CORPSES_SMEARBLOOD),
    CVAR_BOOL (r_detail, C_GraphicDetailCondition, C_GraphicDetail, graphicdetail, GRAPHICDETAIL),
    CVAR_BOOL (r_floatbob, C_BoolCondition, C_Bool, floatbob, FLOATBOB),
    CVAR_FLOAT(r_gamma, C_GammaCondition, C_Gamma, CF_NONE, gammalevel),
    CVAR_BOOL (r_homindicator, C_BoolCondition, C_Bool, homindicator, HOMINDICATOR),
    CVAR_BOOL (r_hud, C_BoolCondition, C_Hud, hud, HUD),
    CVAR_BOOL (r_liquid_animatedheight, C_BoolCondition, C_Bool, animatedliquid, ANIMATEDLIQUID),
    CVAR_BOOL (r_liquid_clipsprites, C_BoolCondition, C_Bool, footclip, FOOTCLIP),
    CVAR_SIZE (r_lowpixelsize, C_NoCondition, C_PixelSize, pixelsize),
    CVAR_BOOL (r_mirrorweapons, C_BoolCondition, C_Bool, mirrorweapons, MIRRORWEAPONS),
    CVAR_BOOL (r_rockettrails, C_BoolCondition, C_Bool, smoketrails, SMOKETRAILS),
    CVAR_INT  (r_screensize, C_IntCondition, C_ScreenSize, CF_NONE, screensize, 0, SCREENSIZE),
    CVAR_BOOL (r_shadows, C_BoolCondition, C_Bool, shadows, SHADOWS),
    CVAR_BOOL (r_translucency, C_BoolCondition, C_Bool, translucency, TRANSLUCENCY),
    CVAR_INT  (runcount, C_NoCondition, C_Int, CF_READONLY, runcount, 0, NONE),
    CVAR_INT  (s_maxslicetime, C_NoCondition, C_Int, CF_NONE, snd_maxslicetime_ms, 0, SND_MAXSLICETIME_MS),
    CVAR_INT  (s_musicvolume, C_VolumeCondition, C_Volume, CF_PERCENT, musicvolume_percent, 0, MUSICVOLUME),
    CVAR_INT  (s_sfxvolume, C_VolumeCondition, C_Volume, CF_PERCENT, sfxvolume_percent, 0, SFXVOLUME),
    CVAR_STR  (s_timiditycfgpath, C_NoCondition, C_Str, timidity_cfg_path),
    CMD       (showfps, C_BoolCondition, C_ShowFPS, 1, "[on|off]", "Toggle the display of the average frames per second on/off."),
    CVAR_INT  (skilllevel, C_IntCondition, C_Int, CF_NONE, selectedskilllevel, 0, SKILLLEVEL),
    CMD       (spawn, C_SpawnCondition, C_Spawn, 1, SPAWNCMDFORMAT, "Spawn a monster or object."),
    CVAR_BOOL (spritefixes, C_BoolCondition, C_Bool, spritefixes, SPRITEFIXES),
    CVAR_INT  (totalitems, C_NoCondition, C_Int, CF_READONLY, totalitems, 0, NONE),
    CVAR_INT  (totalkills, C_NoCondition, C_Int, CF_READONLY, totalkills, 0, NONE),
    CVAR_INT  (totalsecrets, C_NoCondition, C_Int, CF_READONLY, totalsecret, 0, NONE),
    CVAR_BOOL (vid_fullscreen, C_BoolCondition, C_Bool, fullscreen, FULLSCREEN),
#if defined(SDL20)
    CVAR_INT  (vid_monitor, C_NoCondition, C_Int, CF_NONE, monitor, 0, MONITOR),
    CVAR_STR  (vid_scaledriver, C_NoCondition, C_Str, scaledriver),
    CVAR_STR  (vid_scalequality, C_NoCondition, C_Str, scalequality),
#endif
    CVAR_SIZE (vid_screenresolution, C_NoCondition, C_ScreenResolution, screenresolution),
    CVAR_STR  (vid_videodriver, C_NoCondition, C_Str, videodriver),
#if defined(SDL20)
    CVAR_BOOL (vid_vsync, C_BoolCondition, C_Bool, vsync, VSYNC),
#endif
    CVAR_BOOL (vid_widescreen, C_BoolCondition, C_Bool, widescreen, WIDESCREEN),
    CVAR_POS  (vid_windowposition, C_NoCondition, C_WindowPosition, windowposition),
    CVAR_SIZE (vid_windowsize, C_NoCondition, C_WindowSize, windowsize),

    { "", C_NoCondition, NULL, 0, 0, CF_NONE, NULL, 0, 0, 0, 0, "", "" }
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
    C_Bool(cmd, parm1, "");
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
    int i = 0;

    while (controls[i].type)
    {
        if (controls[i].type == type && controls[i].value == value)
        {
            char *control = controls[i].control;

            if (strlen(control) == 1)
                C_Output("%i\t\'%s\'\t%s", count, (control[0] == '=' ? "+" : control), action);
            else
                C_Output("%i\t%s\t%s", count, control, action);
            break;
        }
        ++i;
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
        int     i = 0;

        while (controls[i].type)
        {
            if (!strcasecmp(parm1, controls[i].control))
                break;
            ++i;
        }

        if (controls[i].control[0])
        {
            int action = 0;

            if (!parm2[0])
            {
                while (actions[action].action[0])
                {
                    if (controls[i].type == keyboard && actions[action].keyboard1
                        && controls[i].value == *(int *)actions[action].keyboard1)
                    {
                        char *control = controls[i].control;

                        if (strlen(control) == 1)
                            C_Output("The \'%s\' key on the keyboard is bound to the %s action.",
                                (control[0] == '=' ? "+" : control), actions[action].action);
                        else
                            C_Output("The %s key on the keyboard is bound to the %s action.",
                                control, actions[action].action);
                    }
                    else if (controls[i].type == keyboard && actions[action].keyboard2
                        && controls[i].value == *(int *)actions[action].keyboard2)
                    {
                        char *control = controls[i].control;

                        if (strlen(control) == 1)
                            C_Output("The \'%s\' key on the keyboard is bound to the %s action.",
                            (control[0] == '=' ? "+" : control), actions[action].action);
                        else
                            C_Output("The %s key on the keyboard is bound to the %s action.",
                            control, actions[action].action);
                    }
                    else if (controls[i].type == mouse && actions[action].mouse
                        && controls[i].value == *(int *)actions[action].mouse)
                        C_Output("The %s control on the mouse is bound to the %s action.",
                            controls[i].control, actions[action].action);
                    else if (controls[i].type == gamepad && actions[action].gamepad
                        && controls[i].value == *(int *)actions[action].gamepad)
                        C_Output("The %s control on the gamepad is bound to the %s action.",
                            controls[i].control, actions[action].action);
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
                    switch (controls[i].type)
                    {
                        case keyboard:
                            if (controls[i].value != *(int *)actions[action].keyboard1
                                && controls[i].value != *(int *)actions[action].keyboard2)
                            {
                                if (*(int *)actions[action].keyboard1)
                                {
                                    if (actions[action].keyboard2)
                                    {
                                        if (*(int *)actions[action].keyboard2)
                                            *(int *)actions[action].keyboard1
                                                = *(int *)actions[action].keyboard2;
                                        *(int *)actions[action].keyboard2 = controls[i].value;
                                    }
                                    else
                                        *(int *)actions[action].keyboard1 = controls[i].value;
                                }
                                else
                                    *(int *)actions[action].keyboard1 = controls[i].value;
                                if (strlen(parm1) == 1)
                                    C_Output("The \'%s\' key on the keyboard is now bound to the %s action.",
                                        (parm1[0] == '=' ? "+" : parm1), parm2);
                                else
                                    C_Output("The %s key on the keyboard is now bound to the %s action.",
                                        parm1, parm2);
                            }
                            break;
                        case mouse:
                            *(int *)actions[action].mouse = controls[i].value;
                            C_Output("The %s control on the mouse is now bound to the %s action.", parm1, parm2);
                            break;
                        case gamepad:
                            *(int *)actions[action].gamepad = controls[i].value;
                            C_Output("The %s control on the gamepad is now bound to the %s action.", parm1, parm2);
                            break;
                    }
                    M_SaveDefaults();
                }
            }
        }
    }
}

//
// BLOODSPLATS cvar
//
void (*P_BloodSplatSpawner)(fixed_t, fixed_t, int, int);

boolean C_BloodSplatsCondition(char *cmd, char *parm1, char *parm2)
{
    int value = 0;

    return (!parm1[0] || C_LookupValueFromAlias(parm1, 7) >= 0
        || sscanf(parm1, "%i", &value));
}

void C_BloodSplats(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 7);

        if (value < 0)
            sscanf(parm1, "%i", &value);
        if (value >= 0)
        {
            bloodsplats = value;
            M_SaveDefaults();

            if (!bloodsplats)
            {
                P_BloodSplatSpawner = P_NullBloodSplatSpawner;
                C_Output("Blood splats are now off.");
            }
            if (bloodsplats == UNLIMITED)
            {
                P_BloodSplatSpawner = P_SpawnBloodSplat;
                C_Output("There can now be an unlimited number of blood splats.");
            }
            else
            {
                P_BloodSplatSpawner = P_SpawnBloodSplat2;
                C_Output("There can now be a maximum of %s blood splats.", commify(bloodsplats));
            }
        }
    }
    else
    {
        if (!bloodsplats)
        {
            P_BloodSplatSpawner = P_NullBloodSplatSpawner;
            C_Output("Blood splats are off.");
        }
        if (bloodsplats == UNLIMITED)
        {
            P_BloodSplatSpawner = P_SpawnBloodSplat;
            C_Output("There can be an unlimited number of blood splats.");
        }
        else
        {
            P_BloodSplatSpawner = P_SpawnBloodSplat2;
            C_Output("There can be a maximum of %s blood splats.", commify(bloodsplats));
        }
    }
}

//
// Boolean cvars
//
boolean C_BoolCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, 1) >= 0);
}

void C_Bool(char *cmd, char *parm1, char *parm2)
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
                    M_SaveDefaults();
                    C_Output("%s is now %s.", cmd, parm1);
                }
            }
            else
                C_Output("%s is %s.", cmd, (*(boolean *)consolecmds[i].variable ? "on" : "off"));
        }
        ++i;
    }
}

//
// CLEAR cmd
//
extern int      consolestrings;

void C_Clear(char *cmd, char *parm1, char *parm2)
{
    consolestrings = 0;
    C_Output("");
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
        if (consolecmds[i].type == CT_CMD && (!parm1[0] || wildcard(consolecmds[i].name, parm1)))
            C_Output("%i\t%s %s\t\t%s", count++, consolecmds[i].name, consolecmds[i].format,
                consolecmds[i].description);
        ++i;
    }
}

//
// CONBACK cvar
//
boolean C_ConBackCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || R_CheckFlatNumForName(parm1) >= 0);
}

void C_ConBack(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        conback = strdup(parm1);
        consolebackground = W_CacheLumpName(parm1, PU_CACHE);
        defaultconback = ((gamemode == commercial && !strcasecmp(conback, "GRNROCK"))
            || (gamemode != commercial && !strcasecmp(conback, "FLOOR7_2")));
        M_SaveDefaults();
        C_Output("The console's background is now using the \"%s\" flat.", uppercase(conback));
    }
    else
        C_Output("The console's background is using the \"%s\" flat.", uppercase(conback));
}

//
// CONDUMP cmd
//
void C_ConDump(char *cmd, char *parm1, char *parm2)
{
    if (consolestrings)
    {
        FILE        *file = fopen((parm1[0] ? parm1 : "condump.txt"), "wt");
        int         i;

        for (i = 1; i < consolestrings - 1; ++i)
            fprintf(file, "%s\n", (console[i].type == divider ? DIVIDERSTRING : console[i].string));
        fclose(file);
        C_Output("Dumped the console to the file %s.", (parm1[0] ? uppercase(parm1) : "CONDUMP.TXT"));
    }
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
        if (consolecmds[i].type == CT_CVAR && (!parm1[0] || wildcard(consolecmds[i].name, parm1)))
        {
            if (consolecmds[i].flags & CF_BOOLEAN)
                C_Output("%i\t%s\t\t%s", count++, consolecmds[i].name,
                    C_LookupAliasFromValue(*(boolean *)consolecmds[i].variable,
                    consolecmds[i].aliases));
            else if ((consolecmds[i].flags & CF_INTEGER) && (consolecmds[i].flags & CF_PERCENT))
                C_Output("%i\t%s\t\t%i%%", count++, consolecmds[i].name,
                    *(int *)consolecmds[i].variable);
            else if (consolecmds[i].flags & CF_INTEGER)
            {
                char *alias = C_LookupAliasFromValue(*(int *)consolecmds[i].variable,
                              consolecmds[i].aliases);

                C_Output("%i\t%s\t\t%s", count++, consolecmds[i].name,
                    (alias ? alias : commify(*(int *)consolecmds[i].variable)));
            }
            else if (consolecmds[i].flags & CF_FLOAT)
                C_Output("%i\t%s\t\t%s%s", count++, consolecmds[i].name,
                    striptrailingzero(*(float *)consolecmds[i].variable),
                    ((consolecmds[i].flags & CF_PERCENT) ? "%" : ""));
            else if (consolecmds[i].flags & CF_STRING)
                C_Output("%i\t%s\t\t\"%s\"", count++, consolecmds[i].name,
                    *(char **)consolecmds[i].variable);
            else if (consolecmds[i].flags & CF_POSITION)
            {
                if ((*(char **)consolecmds[i].variable)[0])
                    C_Output("%i\t%s\t\t(%s)", count++, consolecmds[i].name,
                        *(char **)consolecmds[i].variable);
                else
                    C_Output("%i\t%s\t\tcenter", count++, consolecmds[i].name);
            }
            else if (consolecmds[i].flags & CF_SIZE)
                C_Output("%i\t%s\t\t%s", count++, consolecmds[i].name,
                    *(char **)consolecmds[i].variable);
            else if (consolecmds[i].flags & CF_TIME)
            {
                int tics = *(int *)consolecmds[i].variable / TICRATE;

                C_Output("%i\t%s\t\t%02i:%02i:%02i", count++, consolecmds[i].name,
                    tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60);
            }

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

        if (value >= 0.0f && value <= 100.0f)
        {
            if (!strcasecmp(cmd, "gp_deadzone_left"))
            {
                gamepadleftdeadzone_percent = value;
                gamepadleftdeadzone = (int)(BETWEENF(GAMEPADLEFTDEADZONE_MIN,
                    gamepadleftdeadzone_percent,
                    GAMEPADLEFTDEADZONE_MAX) * (float)SHRT_MAX / 100.0f);
                C_Output("The dead zone of the gamepad's left thumbstick is now %s%%.",
                    striptrailingzero(value));
            }
            else
            {
                gamepadrightdeadzone_percent = value;
                gamepadrightdeadzone = (int)(BETWEENF(GAMEPADRIGHTDEADZONE_MIN,
                    gamepadrightdeadzone_percent,
                    GAMEPADRIGHTDEADZONE_MAX) * (float)SHRT_MAX / 100.0f);
                C_Output("The dead zone of the gamepad's right thumbstick is now %s%%.",
                    striptrailingzero(value));
            }

            M_SaveDefaults();
        }
    }
    else
    {
        float   value = 0;

        if (!strcasecmp(cmd, "gp_deadzone_left"))
            C_Output("The dead zone of the gamepad's left thumbstick is %s%%.",
                striptrailingzero(gamepadleftdeadzone_percent));
        else
            C_Output("The dead zone of the gamepad's right thumbstick is %s%%.",
                striptrailingzero(gamepadrightdeadzone_percent));
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
// EXITMAP cmd
//
void C_ExitMap(char *cmd, char *parm1, char *parm2)
{
    G_ExitLevel();
}

//
// All float cvars
//
boolean C_FloatCondition(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    if (!parm1[0])
        return true;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_FLOAT))
        {
            int value = -1;

            sscanf(parm1, "%f", &value);

            return (value >= 0);
        }
        ++i;
    }
    return false;
}

void C_Float(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_FLOAT))
        {
            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                float     value = -1;

                sscanf(parm1, "%f", &value);

                if (value >= 0)
                {
                    *(float *)consolecmds[i].variable = value;
                    M_SaveDefaults();
                    C_Output("%s is %.2f.", cmd, value);
                }
            }
            else
                C_Output("%s is %.2f.", cmd, *(float *)consolecmds[i].variable);
        }
        ++i;
    }
}

//
// GAMMACORRECTIONLEVEL cvar
//
extern int      st_palette;

boolean C_GammaCondition(char *cmd, char *parm1, char *parm2)
{
    int value = -1;

    if (!parm1[0] || !strcasecmp(parm1, "off"))
        return true;

    sscanf(parm1, "%f", &value);

    return (value >= 0);
}

void C_Gamma(char *cmd, char *parm1, char *parm2)
{
    static char buffer[128];

    if (parm1[0])
    {
        float   value = -1;

        if (!strcasecmp(parm1, "off"))
            gammalevel = 1.0f;
        else
            sscanf(parm1, "%f", &value);

        if (value >= 0.0f)
        {
            gammalevel = BETWEENF(GAMMALEVEL_MIN, value, GAMMALEVEL_MAX);
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

            I_SetPalette((byte *)W_CacheLumpName("PLAYPAL", PU_CACHE) + st_palette * 768);
            M_SaveDefaults();

            if (gammalevel == 1.0f)
                C_Output("Gamma correction is now off.");
            else
            {
                M_snprintf(buffer, sizeof(buffer), "The gamma correction level is now %.2f.",
                    gammalevel);
                if (buffer[strlen(buffer) - 1] == '0' && buffer[strlen(buffer) - 2] == '0')
                    buffer[strlen(buffer) - 1] = '\0';
                C_Output(buffer);
            }
        }
    }
    else
    {
        if (gammalevel == 1.0f)
            C_Output("Gamma correction is off.");
        else
        {
            M_snprintf(buffer, sizeof(buffer), "The gamma correction level is %.2f.",
                gammalevel);
            if (buffer[strlen(buffer) - 1] == '0' && buffer[strlen(buffer) - 2] == '0')
                buffer[strlen(buffer) - 1] = '\0';
            C_Output(buffer);
        }
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
    player_t    *player = &players[displayplayer];

    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 1);

        if (value == 0)
            player->cheats &= ~CF_GODMODE;
        else if (value == 1)
            player->cheats |= CF_GODMODE;
    }
    else
        player->cheats ^= CF_GODMODE;

    C_Output((player->cheats & CF_GODMODE) ? s_STSTR_GODON : s_STSTR_GODOFF);
}

//
// GRAPHICDETAIL cvar
//
boolean C_GraphicDetailCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, 6) >= 0);
}

void C_GraphicDetail(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int value = C_LookupValueFromAlias(parm1, 6);

        if (value == 0 || value == 1)
        {
            graphicdetail = !!value;
            M_SaveDefaults();
            C_Output("The graphic detail is now %s.", parm1);
        }
    }
    else
        C_Output("The graphic detail is %s.", C_LookupAliasFromValue(graphicdetail, 6));
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
        C_Bool(cmd, parm1, "");
}

//
// Integer cvars
//
boolean C_IntCondition(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    if (!parm1[0])
        return true;

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

void C_Int(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_INTEGER))
        {
            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                int     value = C_LookupValueFromAlias(parm1, consolecmds[i].aliases);

                if (value < 0)
                    sscanf(parm1, "%i", &value);

                if (value >= 0)
                {
                    *(int *)consolecmds[i].variable = value;
                    M_SaveDefaults();
                    C_Output("%s is now %s.", cmd, parm1);
                }
            }
            else
            {
                char    *alias = C_LookupAliasFromValue(*(int *)consolecmds[i].variable,
                                 consolecmds[i].aliases);

                C_Output("%s is %s.", cmd,
                    (alias ? alias : commify(*(int *)consolecmds[i].variable)));
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
        C_Output("Player killed.");
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
            C_Output("No monsters killed.");
        else
            C_Output("%s monster%s killed.", commify(kills), (kills == 1 ? "" : "s"));
    }
}

//
// MAP cmd
//
static int      mapcmdepisode;
static int      mapcmdmap;

extern boolean  samelevel;
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
            sscanf(uppercase(parm1), "E%iM0%i", &mapcmdepisode, &mapcmdmap);
            if (!mapcmdmap)
                sscanf(uppercase(parm1), "E%iM%i", &mapcmdepisode, &mapcmdmap);
            if (mapcmdmap && ((mapcmdepisode == 1 && BTSXE1) || (mapcmdepisode == 2 && BTSXE2)))
            {
                static char     lump[6];

                M_snprintf(lump, sizeof(lump), "MAP%02i", mapcmdmap);
                return (W_CheckMultipleLumps(lump) == 2);
            }
        }
        sscanf(uppercase(parm1), "MAP0%i", &mapcmdmap);
        if (!mapcmdmap)
            sscanf(uppercase(parm1), "MAP%i", &mapcmdmap);
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
        C_Output("map "MAPCMDFORMAT);
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
    C_Output("Warping to %s...", uppercase(parm1));
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

    return ((!strcasecmp(cmd, "snd_musicvolume") && value >= MUSICVOLUME_MIN && value <= MUSICVOLUME_MAX)
        || (!strcasecmp(cmd, "snd_sfxvolume") && value >= SFXVOLUME_MIN && value <= SFXVOLUME_MAX));
}

void C_Volume(char *cmd, char *parm1, char *parm2)
{
    int value = 0;

    if (parm1[0])
    {
        if (parm1[strlen(parm1) - 1] == '%')
            parm1[strlen(parm1) - 1] = 0;
        sscanf(parm1, "%i", &value);

        if (!strcasecmp(cmd, "snd_musicvolume"))
        {
            musicvolume_percent = value;
            musicVolume = (BETWEEN(MUSICVOLUME_MIN, musicvolume_percent,
                MUSICVOLUME_MAX) * 15 + 50) / 100;
            S_SetMusicVolume((int)(musicVolume * (127.0f / 15.0f)));
            C_Output("The music volume is now %i%%.", value);
        }
        else
        {
            sfxvolume_percent = value;
            sfxVolume = (BETWEEN(SFXVOLUME_MIN, sfxvolume_percent, SFXVOLUME_MAX) * 15 + 50) / 100;
            S_SetSfxVolume((int)(sfxVolume * (127.0f / 15.0f)));
            C_Output("The SFX volume is now %i%%.", value);
        }

        M_SaveDefaults();
    }
    else
    {
        if (!strcasecmp(cmd, "snd_musicvolume"))
            C_Output("The music volume is %i%%.", musicvolume_percent);
        else
            C_Output("The music volume is %i%%.", sfxvolume_percent);
    }
}

//
// NOCLIP cmd
//
void C_NoClip(char *cmd, char *parm1, char *parm2)
{
    player_t    *player = &players[displayplayer];

    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 1);

        if (value == 0)
            player->cheats &= ~CF_NOCLIP;
        else if (value == 1)
            player->cheats |= CF_NOCLIP;
    }
    else
        player->cheats ^= CF_NOCLIP;

    C_Output("%s.", ((player->cheats & CF_NOCLIP) ? s_STSTR_NCON : s_STSTR_NCOFF));
}

//
// NOTARGET cmd
//
void C_NoTarget(char *cmd, char *parm1, char *parm2)
{
    player_t    *player = &players[displayplayer];

    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 1);

        if (value == 0)
            player->cheats &= ~CF_NOTARGET;
        else if (value == 1)
            player->cheats |= CF_NOTARGET;
    }
    else
        player->cheats ^= CF_NOTARGET;

    C_Output((player->cheats & CF_NOTARGET) ? s_STSTR_NTON : s_STSTR_NTOFF);
}

void C_PixelSize(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     width = -1;
        int     height = -1;

        sscanf(parm1, "%ix%i", &width, &height);

        if (width >= 0 && height >= 0)
        {
            pixelwidth = BETWEEN(PIXELWIDTH_MIN, width, PIXELWIDTH_MAX);
            while (SCREENWIDTH % pixelwidth)
                --pixelwidth;

            pixelheight = BETWEEN(PIXELHEIGHT_MIN, height, PIXELHEIGHT_MAX);
            while (SCREENHEIGHT % pixelheight)
                --pixelheight;

            M_SaveDefaults();
            C_Output("The size of pixels when graphic detail is low is now %ix%i.",
                pixelwidth, pixelheight);
        }
    }
    else
        C_Output("The size of pixels when graphic detail is low is %ix%i.",
            pixelwidth, pixelheight);
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
            C_Output("The screen size is now %i.", screensize);
        }
    }
    else
        C_Output("The screen size is %i.", screensize);
}

//
// SHOWFPS cmd
//
void C_ShowFPS(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 1);

        if (value == 0 || value == 1)
            showfps = !!value;
    }
    else
        showfps = !showfps;
}

//
// SPAWN cmd
//
static int      spawntype = NUMMOBJTYPES;

boolean C_SpawnCondition(char *cmd, char *parm1, char *parm2)
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
                boolean     spawn = true;

                spawntype = mobjinfo[i].doomednum;
                if (gamemode != commercial)
                {
                    switch (spawntype)
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
                            spawn = false;
                            break;
                    }
                }
                else if (spawntype == WolfensteinSS && bfgedition)
                    spawntype = Zombieman;

                return spawn;
            }
    }
    return false;
}

void C_Spawn(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
    {
        C_Output("spawn "SPAWNCMDFORMAT);
        return;
    }
    else
    {
        mobj_t      *player = players[displayplayer].mo;
        fixed_t     x = player->x;
        fixed_t     y = player->y;
        angle_t     angle = player->angle >> ANGLETOFINESHIFT;
        mobj_t      *thing = P_SpawnMobj(x + 100 * finecosine[angle], y + 100 * finesine[angle],
            ONFLOORZ, P_FindDoomedNum(spawntype));

        thing->angle = R_PointToAngle2(thing->x, thing->y, x, y);
    }
}

//
// String cvars
//
void C_Str(char *cmd, char *parm1, char *parm2)
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
                C_Output("%s is now \"%s\".", cmd, parm1);
            }
            else
                C_Output("%s is \"%s\".", cmd, *(char **)consolecmds[i].variable);
        }
        ++i;
    }
}

extern int      desktopwidth;
extern int      desktopheight;

void C_ScreenResolution(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        if (!strcasecmp(parm1, "desktop"))
        {
            screenwidth = 0;
            screenheight = 0;

            M_SaveDefaults();
            C_Output("The screen resolution is now the desktop resolution of %ix%i.",
                desktopwidth, desktopheight);
        }
        else
        {
            int     width = -1;
            int     height = -1;

            sscanf(parm1, "%ix%i", &width, &height);

            if (width >= 0 && height >= 0)
            {
                screenwidth = width;
                screenheight = height;

                M_SaveDefaults();
                C_Output("The screen resolution is now %ix%i.", screenwidth, screenheight);
            }
        }
    }
    else if (!screenwidth || !screenheight)
        C_Output("The screen resolution is the desktop resolution of %ix%i.",
            desktopwidth, desktopheight);
    else
        C_Output("The screen resolution is %ix%i.", screenwidth, screenheight);
}

void C_Time(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_TIME))
        {
            int tics = *(int *)consolecmds[i].variable / TICRATE;

            C_Output("%s is %02i:%02i:%02i.", cmd,
                tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60);
        }
        ++i;
    }
}

void C_WindowPosition(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    if (parm1[0])
    {
        if (!strcasecmp(parm1, "center"))
        {
            windowposition = "";
            C_Output("The window is now centered on screen.");
        }
        else
        {
            windowposition = strdup(parm1);
            C_Output("The window is now positioned at (%s).", parm1);
        }
        M_SaveDefaults();
    }
    else if (windowposition[0])
        C_Output("The window is centered on screen.");
    else
        C_Output("The window is now positioned at (%s).", windowposition);
}

void C_WindowSize(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     width = -1;
        int     height = -1;

        sscanf(parm1, "%ix%i", &width, &height);

        if (width >= 0 && height >= 0)
        {
            windowwidth = width;
            windowheight = height;

            M_SaveDefaults();
            C_Output("The size of the window is now %ix%i.", windowwidth, windowheight);
        }
    }
    else
        C_Output("The size of the window is %ix%i.", windowwidth, windowheight);
}
