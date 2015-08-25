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
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "s_sound.h"
#include "SDL.h"
#include "sounds.h"
#include "st_stuff.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#if !defined(MAX_PATH)
#define MAX_PATH        260
#endif

#define GIVECMDFORMAT   "~items~"
#define MAPCMDFORMAT    "E~x~M~y~|MAP~xy~"
#define SPAWNCMDFORMAT  "~monster~|~item~"

extern dboolean         am_grid;
extern dboolean         am_rotatemode;
extern dboolean         con_timestamps;
extern char             *configfile;
extern int              episode;
extern int              expansion;
extern int              gamepadautomap;
extern int              gamepadautomapclearmark;
extern int              gamepadautomapfollowmode;
extern int              gamepadautomapgrid;
extern int              gamepadautomapmark;
extern int              gamepadautomapmaxzoom;
extern int              gamepadautomaprotatemode;
extern int              gamepadautomapzoomin;
extern int              gamepadautomapzoomout;
extern int              gamepadfire;
extern int              gamepadleftdeadzone;
extern int              gamepadrightdeadzone;
extern int              gamepadmenu;
extern int              gamepadnextweapon;
extern int              gamepadprevweapon;
extern int              gamepadrun;
extern int              gamepaduse;
extern int              gamepadweapon1;
extern int              gamepadweapon2;
extern int              gamepadweapon3;
extern int              gamepadweapon4;
extern int              gamepadweapon5;
extern int              gamepadweapon6;
extern int              gamepadweapon7;
extern int              gametime;
extern float            gp_deadzone_left;
extern float            gp_deadzone_right;
extern int              gp_sensitivity;
extern dboolean         gp_swapthumbsticks;
extern dboolean         gp_vibrate;
extern char             *iwadfolder;
extern int              key_alwaysrun;
extern int              key_automap;
extern int              key_automap_clearmark;
extern int              key_automap_followmode;
extern int              key_automap_grid;
extern int              key_automap_mark;
extern int              key_automap_maxzoom;
extern int              key_automap_rotatemode;
extern int              key_automap_zoomin;
extern int              key_automap_zoomout;
extern int              key_down;
extern int              key_down2;
extern int              key_fire;
extern int              key_left;
extern int              key_nextweapon;
extern int              key_prevweapon;
extern int              key_right;
extern int              key_run;
extern int              key_strafe;
extern int              key_strafeleft;
extern int              key_strafeleft2;
extern int              key_straferight;
extern int              key_straferight2;
extern int              key_up;
extern int              key_up2;
extern int              key_use;
extern int              key_weapon1;
extern int              key_weapon2;
extern int              key_weapon3;
extern int              key_weapon4;
extern int              key_weapon5;
extern int              key_weapon6;
extern int              key_weapon7;
extern dboolean         messages;
extern float            m_acceleration;
extern dboolean         m_doubleclick_use;
extern dboolean         m_novertical;
extern int              m_sensitivity;
extern int              m_threshold;
extern int              mousebfire;
extern int              mousebforward;
extern int              mousebprevweapon;
extern int              mousebnextweapon;
extern int              mousebstrafe;
extern int              mousebuse;
extern char             *playername;
extern dboolean         pm_alwaysrun;
extern dboolean         pm_centerweapon;
extern int              pm_walkbob;
extern dboolean         r_altlighting;
extern int              r_blood;
extern int              r_bloodsplats_max;
extern int              r_bloodsplats_total;
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
extern int              r_screensize;
extern dboolean         r_shadows;
extern dboolean         r_translucency;
extern int              runcount;
extern char             *savegamefolder;
extern int              s_musicvolume;
extern dboolean         s_randompitch;
extern int              s_sfxvolume;
extern char             *s_timiditycfgpath;
extern int              savegame;
extern int              skilllevel;
extern unsigned int     stat_damageinflicted;
extern unsigned int     stat_damagereceived;
extern unsigned int     stat_deaths;
extern unsigned int     stat_itemspickedup;
extern unsigned int     stat_monsterskilled;
extern unsigned int     stat_secretsrevealed;
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

extern int              pixelwidth;
extern int              pixelheight;
extern int              screenheight;
extern int              screenwidth;

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
    { "numlock",       keyboard, KEY_NUMLOCK    }, { "capslock",      keyboard, KEY_CAPSLOCK   },
    { "scrolllock",    keyboard, KEY_SCRLCK     }, { "home",          keyboard, KEY_HOME       },
    { "up",            keyboard, KEY_UPARROW    }, { "pageup",        keyboard, KEY_PGUP       },
    { "left",          keyboard, KEY_LEFTARROW  }, { "right",         keyboard, KEY_RIGHTARROW },
    { "end",           keyboard, KEY_END        }, { "down",          keyboard, KEY_DOWNARROW  },
    { "pagedown",      keyboard, KEY_PGDN       }, { "insert",        keyboard, KEY_INS        },
    { "delete",        keyboard, KEY_DEL        }, { "mouse1",        mouse,    0              },
    { "mouse2",        mouse,    1              }, { "mouse3",        mouse,    2              },
    { "mouse4",        mouse,    3              }, { "mouse5",        mouse,    4              },
    { "mouse6",        mouse,    5              }, { "mouse7",        mouse,    6              },
    { "mouse8",        mouse,    7              }, { "wheelup",       mouse,    8              },
    { "wheeldown",     mouse,    9              }, { "dpadup",        gamepad,  1              },
    { "dpaddown",      gamepad,  2              }, { "dpadleft",      gamepad,  4              },
    { "dpadright",     gamepad,  8              }, { "start",         gamepad,  16             },
    { "back",          gamepad,  32             }, { "leftthumb",     gamepad,  64             },
    { "rightthumb",    gamepad,  128            }, { "leftshoulder",  gamepad,  256            },
    { "rightshoulder", gamepad,  512            }, { "lefttrigger",   gamepad,  1024           },
    { "righttrigger",  gamepad,  2048           }, { "gamepad1",      gamepad,  4096           },
    { "gamepad2",      gamepad,  8192           }, { "gamepad3",      gamepad,  16384          },
    { "gamepad4",      gamepad,  32768          }, { "",              0,        0              }
};

action_t actions[] = 
{
    { "+alwaysrun",    &key_alwaysrun,          NULL,              NULL                      },
    { "+automap",      &key_automap,            NULL,              &gamepadautomap           },
    { "+back",         &key_down,               NULL,              NULL                      },
    { "+back2",        &key_down2,              NULL,              NULL                      },
    { "+clearmark",    &key_automap_clearmark,  NULL,              &gamepadautomapclearmark  },
    { "+fire",         &key_fire,               &mousebfire,       &gamepadfire              },
    { "+followmode",   &key_automap_followmode, NULL,              &gamepadautomapfollowmode },
    { "+forward",      &key_up,                 &mousebforward,    NULL                      },
    { "+forward2",     &key_up2,                &mousebforward,    NULL                      },
    { "+grid",         &key_automap_grid,       NULL,              &gamepadautomapgrid       },
    { "+left",         &key_left,               NULL,              NULL                      },
    { "+mark",         &key_automap_mark,       NULL,              &gamepadautomapmark       },
    { "+maxzoom",      &key_automap_maxzoom,    NULL,              &gamepadautomapmaxzoom    },
    { "+menu",         NULL,                    NULL,              &gamepadmenu              },
    { "+nextweapon",   &key_nextweapon,         &mousebnextweapon, &gamepadnextweapon        },
    { "+prevweapon",   &key_prevweapon,         &mousebprevweapon, &gamepadprevweapon        },
    { "+right",        &key_right,              NULL,              NULL                      },
    { "+rotatemode",   &key_automap_rotatemode, NULL,              &gamepadautomaprotatemode },
    { "+run",          &key_run,                NULL,              &gamepadrun               },
    { "+strafe",       &key_strafe,             &mousebstrafe,     NULL                      },
    { "+strafeleft",   &key_strafeleft,         NULL,              NULL                      },
    { "+strafeleft2",  &key_strafeleft2,        NULL,              NULL                      },
    { "+straferight",  &key_straferight,        NULL,              NULL                      },
    { "+straferight2", &key_straferight2,       NULL,              NULL                      },
    { "+use",          &key_use,                &mousebuse,        &gamepaduse               },
    { "+weapon1",      &key_weapon1,            NULL,              &gamepadweapon1           },
    { "+weapon2",      &key_weapon2,            NULL,              &gamepadweapon2           },
    { "+weapon3",      &key_weapon3,            NULL,              &gamepadweapon3           },
    { "+weapon4",      &key_weapon4,            NULL,              &gamepadweapon4           },
    { "+weapon5",      &key_weapon5,            NULL,              &gamepadweapon5           },
    { "+weapon6",      &key_weapon6,            NULL,              &gamepadweapon6           },
    { "+weapon7",      &key_weapon7,            NULL,              &gamepadweapon7           },
    { "+zoomin",       &key_automap_zoomin,     NULL,              &gamepadautomapzoomin     },
    { "+zoomout",      &key_automap_zoomout,    NULL,              &gamepadautomapzoomout    },
    { "",              NULL,                    NULL,              NULL                      }
};

static dboolean C_BloodCondition(char *, char *, char *);
static dboolean C_BoolCondition(char *, char *, char *);
static dboolean C_CheatCondition(char *, char *, char *);
static dboolean C_DeadZoneCondition(char *, char *, char *);
static dboolean C_FloatCondition(char *, char *, char *);
static dboolean C_GameCondition(char *, char *, char *);
static dboolean C_GammaCondition(char *, char *, char *);
static dboolean C_GiveCondition(char *, char *, char *);
static dboolean C_GodCondition(char *, char *, char *);
static dboolean C_GraphicDetailCondition(char *, char *, char *);
static dboolean C_IntCondition(char *, char *, char *);
static dboolean C_KillCondition(char *, char *, char *);
static dboolean C_LoadCondition(char *, char *, char *);
static dboolean C_MapCondition(char *, char *, char *);
static dboolean C_MaxBloodSplatsCondition(char *, char *, char *);
static dboolean C_NoCondition(char *, char *, char *);
static dboolean C_SaveCondition(char *, char *, char *);
static dboolean C_SpawnCondition(char *, char *, char *);
static dboolean C_ResurrectCondition(char *, char *, char *);
static dboolean C_VolumeCondition(char *, char *, char *);

void C_Bind(char *, char *, char *);

static void C_AlwaysRun(char *, char *, char *);
static void C_Blood(char *, char *, char *);
static void C_Bool(char *, char *, char *);
static void C_Clear(char *, char *, char *);
static void C_CmdList(char *, char *, char *);
static void C_ConDump(char *, char *, char *);
static void C_CvarList(char *, char *, char *);
static void C_DeadZone(char *, char *, char *);
static void C_Display(char *, char *, char *);
static void C_EndGame(char *, char *, char *);
static void C_ExitMap(char *, char *, char *);
static void C_Float(char *, char *, char *);
static void C_Fullscreen(char *, char *, char *);
static void C_Gamma(char *, char *, char *);
static void C_GamepadVibrate(char *, char *, char *);
static void C_God(char *, char *, char *);
static void C_Give(char *, char *, char *);
static void C_GraphicDetail(char *, char *, char *);
static void C_Help(char *, char *, char *);
static void C_Hud(char *, char *, char *);
static void C_Int(char *, char *, char *);
static void C_Kill(char *, char *, char *);
static void C_Load(char *, char *, char *);
static void C_Map(char *, char *, char *);
static void C_MapList(char *, char *, char *);
static void C_MapStats(char *, char *, char *);
static void C_MaxBloodSplats(char *, char *, char *);
static void C_NoClip(char *, char *, char *);
static void C_NoTarget(char *, char *, char *);
static void C_PixelSize(char *, char *, char *);
static void C_PlayerName(char *, char *, char *);
static void C_PlayerStats(char *, char *, char *);
static void C_Quit(char *, char *, char *);
static void C_Resurrect(char *, char *, char *);
static void C_Save(char *, char *, char *);
static void C_ScaleDriver(char *, char *, char *);
static void C_ScaleFilter(char *, char *, char *);
static void C_ScreenSize(char *, char *, char *);
static void C_ScreenResolution(char *, char *, char *);
static void C_ShowFPS(char *, char *, char *);
static void C_Spawn(char *, char *, char *);
static void C_Str(char *, char *, char *);
static void C_ThingList(char *, char *, char *);
static void C_Time(char *, char *, char *);
static void C_UnBind(char *, char *, char *);
static void C_Volume(char *, char *, char *);
static void C_Vsync(char *, char *, char *);
static void C_Widescreen(char *, char *, char *);
static void C_WindowPosition(char *, char *, char *);
static void C_WindowSize(char *, char *, char *);

static int C_LookupValueFromAlias(char *text, int aliastype)
{
    int i = 0;

    while (aliases[i].text[0])
    {
        if (aliastype == aliases[i].type && !strcasecmp(text, aliases[i].text))
            return aliases[i].value;
        ++i;
    }
    return -1;
}

static char *C_LookupAliasFromValue(int value, int aliastype)
{
    int         i = 0;

    while (aliases[i].text[0])
    {
        if (aliastype == aliases[i].type && value == aliases[i].value)
            return aliases[i].text;
        ++i;
    }
    return commify(value);
}

#define CMD(name, cond, func, parms, form, desc) \
    { #name, cond, func, parms, CT_CMD, CF_NONE, NULL, 0, 0, 0, 0, form, desc }
#define CMD_CHEAT(name, parms) \
    { #name, C_CheatCondition, NULL, parms, CT_CHEAT, CF_NONE, NULL, 0, 0, 0, 0, "", "" }
#define CVAR_BOOL(name, cond, func, desc) \
    { #name, cond, func, 1, CT_CVAR, CF_BOOLEAN, &name, 1, false, true, name##_default, "", desc }
#define CVAR_INT(name, cond, func, flags, aliases, desc) \
    { #name, cond, func, 1, CT_CVAR, (CF_INTEGER | flags), &name, aliases, name##_min, \
      name##_max, name##_default, "", desc }
#define CVAR_FLOAT(name, cond, func, flags, desc) \
    { #name, cond, func, 1, CT_CVAR, (CF_FLOAT | flags), &name, 0, 0, 0, 0, "", desc }
#define CVAR_POS(name, cond, func, desc) \
    { #name, cond, func, 1, CT_CVAR, CF_POSITION, &name, 0, 0, 0, 0, "", desc }
#define CVAR_SIZE(name, cond, func, desc) \
    { #name, cond, func, 1, CT_CVAR, CF_SIZE, &name, 0, 0, 0, 0, "", desc }
#define CVAR_STR(name, cond, func, desc) \
    { #name, cond, func, 1, CT_CVAR, CF_STRING, &name, 0, 0, 0, 0, "", desc }
#define CVAR_TIME(name, cond, func, desc) \
    { #name, cond, func, 1, CT_CVAR, (CF_TIME | CF_READONLY), &name, 0, 0, 0, 0, "", desc }

int     numconsolecmds;

consolecmd_t consolecmds[] =
{
    CVAR_BOOL (am_followmode, C_BoolCondition, C_Bool, "Toggles follow mode in the automap."),
    CVAR_BOOL (am_grid, C_BoolCondition, C_Bool, "Toggles the grid in the automap."),
    CVAR_BOOL (am_rotatemode, C_BoolCondition, C_Bool, "Toggles rotate mode in the automap."),
    CMD       (bind, C_NoCondition, C_Bind, 2, "[~control~ [+~action~]]", "Binds an action to a control."),
    CMD       (clear, C_NoCondition, C_Clear, 0, "", "Clears the console."),
    CMD       (cmdlist, C_NoCondition, C_CmdList, 1, "[~searchstring~]", "Shows a list of console commands."),
    CVAR_BOOL (con_timestamps, C_BoolCondition, C_Bool, "Toggles timestamps for player messages in the console."),
    CMD       (condump, C_NoCondition, C_ConDump, 1, "[~filename~.txt]", "Dumps the console to a file."),
    CVAR_STR  (configfile, C_NoCondition, C_Str, "The path of the configuration file."),
    CMD       (cvarlist, C_NoCondition, C_CvarList, 1, "[~searchstring~]", "Shows a list of console variables."),
    CMD       (endgame, C_GameCondition, C_EndGame, 0, "", "Ends a game."),
    CVAR_INT  (episode, C_IntCondition, C_Int, CF_NONE, NOALIAS, "The currently selected episode in the menu."),
    CMD       (exit, C_NoCondition, C_Quit, 0, "", ""),
    CMD       (exitmap, C_GameCondition, C_ExitMap, 0, "", "Exits the current map."),
    CVAR_INT  (expansion, C_IntCondition, C_Int, CF_NONE, NOALIAS, "The currently selected expansion in the menu."),
    CVAR_TIME (gametime, C_NoCondition, C_Time, "The amount of time since "PACKAGE_NAME" started."),
    CMD       (give, C_GiveCondition, C_Give, 1, GIVECMDFORMAT, "Gives items to the player."),
    CMD       (god, C_GodCondition, C_God, 1, "[on|off]", "Toggles god mode."),
    CVAR_FLOAT(gp_deadzone_left, C_DeadZoneCondition, C_DeadZone, CF_PERCENT, "The dead zone of the gamepad's left thumbstick."),
    CVAR_FLOAT(gp_deadzone_right, C_DeadZoneCondition, C_DeadZone, CF_PERCENT, "The dead zone of the gamepad's right thumbstick."),
    CVAR_INT  (gp_sensitivity, C_NoCondition, C_Int, CF_NONE, NOALIAS, "The gamepad's sensitivity."),
    CVAR_BOOL (gp_swapthumbsticks, C_BoolCondition, C_Bool, "Toggles swapping the gamepad's left and right thumbsticks."),
    CVAR_BOOL (gp_vibrate, C_BoolCondition, C_Bool, "Toggles vibration for XInput gamepads."),
    CMD       (help, C_NoCondition, C_Help, 0, "", "Shows the help screen."),
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
    CVAR_STR  (iwadfolder, C_NoCondition, C_Str, "The folder where an IWAD file was last opened."),
    CMD       (kill, C_KillCondition, C_Kill, 1, "[all|~type~]", "Kills the player, all monsters or a type of monster."),
    CMD       (load, C_LoadCondition, C_Load, 1, "~filename~.save", "Loads a game from a file."),
    CVAR_FLOAT(m_acceleration, C_FloatCondition, C_Float, CF_NONE, "The amount the mouse accelerates."),
    CVAR_BOOL (m_doubleclick_use, C_BoolCondition, C_Bool, "Toggles double-clicking a mouse button for the +use action."),
    CVAR_BOOL (m_novertical, C_BoolCondition, C_Bool, "Toggles no vertical movement of the mouse."),
    CVAR_INT  (m_sensitivity, C_IntCondition, C_Int, CF_NONE, NOALIAS, "The mouse's sensitivity."),
    CVAR_INT  (m_threshold, C_IntCondition, C_Int, CF_NONE, NOALIAS, "The mouse's acceleration threshold."),
    CMD       (map, C_MapCondition, C_Map, 1, MAPCMDFORMAT, "Warps to a map."),
    CMD       (maplist, C_NoCondition, C_MapList, 0, "", "Shows a list of the available maps."),
    CMD       (mapstats, C_GameCondition, C_MapStats, 0, "", "Shows stats on the current map."),
    CVAR_BOOL (messages, C_BoolCondition, C_Bool, "Toggles messages."),
    CMD       (noclip, C_GameCondition, C_NoClip, 1, "[on|off]", "Toggles collision detection for the player."),
    CMD       (notarget, C_GameCondition, C_NoTarget, 1, "[on|off]", "Toggles the player as a target."),
    CVAR_STR  (playername, C_NoCondition, C_PlayerName, "The name of the player used in messages."),
    CMD       (playerstats, C_GameCondition, C_PlayerStats, 0, "", "Shows the player's stats."),
    CVAR_BOOL (pm_alwaysrun, C_BoolCondition, C_AlwaysRun, "Toggles always run."),
    CVAR_BOOL (pm_centerweapon, C_BoolCondition, C_Bool, "Toggles the centering of the player's weapon when firing."),
    CVAR_INT  (pm_walkbob, C_NoCondition, C_Int, CF_PERCENT, NOALIAS, "The amount the player bobs when walking."),
    CMD       (quit, C_NoCondition, C_Quit, 0, "", "Quits "PACKAGE_NAME"."),
    CVAR_BOOL (r_altlighting, C_BoolCondition, C_Bool, "Toggles alternate lighting on and around the player."),
    CVAR_INT  (r_blood, C_BloodCondition, C_Blood, CF_NONE, BLOODALIAS, "The color of the blood of the player and monsters."),
    CVAR_INT  (r_bloodsplats_max, C_MaxBloodSplatsCondition, C_MaxBloodSplats, CF_NONE, SPLATALIAS, "The maximum number of blood splats allowed in a map."),
    CVAR_INT  (r_bloodsplats_total, C_IntCondition, C_Int, CF_READONLY, NOALIAS, "The total number of blood splats in the current map."),
    CVAR_BOOL (r_brightmaps, C_BoolCondition, C_Bool, "Toggles brightmaps on certain wall textures."),
    CVAR_BOOL (r_corpses_mirrored, C_BoolCondition, C_Bool, "Toggles corpses being randomly mirrored."),
    CVAR_BOOL (r_corpses_moreblood, C_BoolCondition, C_Bool, "Toggles blood splats around corpses when a map is loaded."),
    CVAR_BOOL (r_corpses_nudge, C_BoolCondition, C_Bool, "Toggles corpses being nudged when monsters walk over them."),
    CVAR_BOOL (r_corpses_slide, C_BoolCondition, C_Bool, "Toggles corpses reacting to barrel and rocket explosions."),
    CVAR_BOOL (r_corpses_smearblood, C_BoolCondition, C_Bool, "Toggles corpses producing blood splats as they slide."),
    CVAR_BOOL (r_detail, C_GraphicDetailCondition, C_GraphicDetail, "Toggles the graphic detail."),
    CVAR_BOOL (r_fixmaperrors, C_BoolCondition, C_Bool, "Toggles the fixing of mapping errors in the DOOM IWADs."),
    CVAR_BOOL (r_fixspriteoffsets, C_BoolCondition, C_Bool, "Toggles the fixing of sprite offsets."),
    CVAR_BOOL (r_floatbob, C_BoolCondition, C_Bool, "Toggles powerups bobbing up and down."),
    CVAR_FLOAT(r_gamma, C_GammaCondition, C_Gamma, CF_NONE, "The gamma correction level."),
    CVAR_BOOL (r_homindicator, C_BoolCondition, C_Bool, "Toggles the flashing \"Hall of Mirrors\" indicator."),
    CVAR_BOOL (r_hud, C_BoolCondition, C_Hud, "Toggles the heads-up display when in widescreen mode."),
    CVAR_BOOL (r_liquid_bob, C_BoolCondition, C_Bool, "Toggles the bobbing effect of liquid sectors."),
    CVAR_BOOL (r_liquid_clipsprites, C_BoolCondition, C_Bool, "Toggles the bottom of sprites being clipped in liquid sectors."),
    CVAR_BOOL (r_liquid_lowerview, C_BoolCondition, C_Bool, "Toggles lowering the player's view in liquid sectors."),
    CVAR_BOOL (r_liquid_swirl, C_BoolCondition, C_Bool, "Toggles the swirl effect of liquid sectors."),
    CVAR_SIZE (r_lowpixelsize, C_NoCondition, C_PixelSize, "The size of pixels when the graphic detail is low."),
    CVAR_BOOL (r_mirroredweapons, C_BoolCondition, C_Bool, "Toggles randomly mirroring weapons dropped by monsters."),
    CVAR_BOOL (r_playersprites, C_BoolCondition, C_Bool,"Toggles the display of the player's weapon."),
    CVAR_BOOL (r_rockettrails, C_BoolCondition, C_Bool, "Toggles rocket trails behind player and Cyberdemon rockets."),
    CVAR_INT  (r_screensize, C_IntCondition, C_ScreenSize, CF_NONE, NOALIAS, "The screen size."),
    CVAR_BOOL (r_shadows, C_BoolCondition, C_Bool, "Toggles sprites casting shadows."),
    CVAR_BOOL (r_translucency, C_BoolCondition, C_Bool, "Toggles translucency in sprites and textures."),
    CMD       (resurrect, C_ResurrectCondition, C_Resurrect, 0, "", "Resurrects the player."),
    CVAR_INT  (runcount, C_NoCondition, C_Int, CF_READONLY, NOALIAS, "The number of times "PACKAGE_NAME" has been run."),
    CVAR_INT  (s_musicvolume, C_VolumeCondition, C_Volume, CF_PERCENT,  NOALIAS, "The music volume."),
    CVAR_BOOL (s_randompitch, C_BoolCondition, C_Bool, "Toggles randomizing the pitch of monster sound effects."),
    CVAR_INT  (s_sfxvolume, C_VolumeCondition, C_Volume, CF_PERCENT, NOALIAS, "The sound effects volume."),
    CVAR_STR  (s_timiditycfgpath, C_NoCondition, C_Str, "The path of Timidity's configuration file."),
    CMD       (save, C_SaveCondition, C_Save, 1, "~filename~.save", "Saves the game to a file."),
    CVAR_STR  (savegamefolder, C_NoCondition, C_Str, "The folder where savegames are saved."),
    CVAR_INT  (skilllevel, C_IntCondition, C_Int, CF_NONE, NOALIAS, "The currently selected skill level in the menu."),
    CMD       (spawn, C_SpawnCondition, C_Spawn, 1, SPAWNCMDFORMAT, "Spawns a monster or item."),
    CMD       (summon, C_SpawnCondition, C_Spawn, 1, "", ""),
    CMD       (thinglist, C_GameCondition, C_ThingList, 0, "", "Shows a list of things in the current map."),
    CMD       (unbind, C_NoCondition, C_UnBind, 1, "~control~", "Unbinds an action from a control."),
    CVAR_BOOL (vid_capfps, C_BoolCondition, C_Bool, "Toggles capping of the framerate at 35 FPS."),
    CVAR_INT  (vid_display, C_IntCondition, C_Display, CF_NONE, NOALIAS, "The display used to render the game."),
#if !defined(WIN32)
    CVAR_STR  (vid_driver, C_NoCondition, C_Str, "The video driver used to render the game."),
#endif
    CVAR_BOOL (vid_fullscreen, C_BoolCondition, C_Fullscreen, "Toggles between fullscreen and a window."),
    CVAR_STR  (vid_scaledriver, C_NoCondition, C_ScaleDriver, "The driver used to scale the display."),
    CVAR_STR  (vid_scalefilter, C_NoCondition, C_ScaleFilter, "The filter used to scale the display."),
    CVAR_SIZE (vid_screenresolution, C_NoCondition, C_ScreenResolution, "The screen's resolution when fullscreen."),
    CVAR_BOOL (vid_showfps, C_BoolCondition, C_ShowFPS, "Toggles the display of the average frames per second."),
    CVAR_BOOL (vid_vsync, C_BoolCondition, C_Vsync, "Toggles vertical synchronization with display's refresh rate."),
    CVAR_BOOL (vid_widescreen, C_BoolCondition, C_Widescreen, "Toggles widescreen mode."),
    CVAR_POS  (vid_windowposition, C_NoCondition, C_WindowPosition, "The position of the window on the desktop."),
    CVAR_SIZE (vid_windowsize, C_NoCondition, C_WindowSize, "The size of the window on the desktop."),
    CMD       (warp, C_MapCondition, C_Map, 1, "", ""),

    { "", C_NoCondition, NULL, 0, 0, CF_NONE, NULL, 0, 0, 0, 0, "", "" }
};

static dboolean C_CheatCondition(char *cmd, char *parm1, char *parm2)
{
    if (gamestate != GS_LEVEL)
        return false;
    else if (!strcasecmp(cmd, cheat_god.sequence))
        return (gameskill != sk_nightmare);
    else if (!strcasecmp(cmd, cheat_ammonokey.sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_ammo.sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_mus.sequence))
        return (!nomusic && musicVolume);
    else if (!strcasecmp(cmd, cheat_noclip.sequence))
        return (gamemode != commercial && gameskill != sk_nightmare);
    else if (!strcasecmp(cmd, cheat_commercial_noclip.sequence))
        return (gamemode == commercial && gameskill != sk_nightmare);
    else if (!strcasecmp(cmd, cheat_powerup[0].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_powerup[1].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_powerup[2].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_powerup[3].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_powerup[4].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_powerup[5].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_powerup[6].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_choppers.sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (!strcasecmp(cmd, cheat_mypos.sequence))
        return true;
    else if (!strcasecmp(cmd, cheat_clev.sequence))
    {
        char   lump[6];
        int    map;

        if (gamemode == commercial)
        {
            map = (parm1[0] - '0') * 10 + parm1[1] - '0';
            M_snprintf(lump, sizeof(lump), "MAP%c%c", parm1[0], parm1[1]);
        }
        else
        {
            map = parm1[1] - '0';
            M_snprintf(lump, sizeof(lump), "E%cM%c", parm1[0], parm1[1]);
        }

        return (W_CheckNumForName(lump) >= 0 && (gamemission != pack_nerve || map <= 9)
            && (!BTSX || W_CheckMultipleLumps(lump) > 1));
    }
    else if (!strcasecmp(cmd, cheat_amap.sequence))
        return automapactive;
    return false;
}

static dboolean C_GameCondition(char *cmd, char *parm1, char *parm2)
{
    return (gamestate == GS_LEVEL);
}

static dboolean C_NoCondition(char *cmd, char *parm1, char *parm2)
{
    return true;
}

static void C_AlwaysRun(char *cmd, char *parm1, char *parm2)
{
    C_Bool(cmd, parm1, "");
    I_InitKeyboard();
}

static void C_DisplayBinds(char *action, int value, controltype_t type, int count)
{
    int i = 0;
    int tabs[8] = { 40, 130, 0, 0, 0, 0, 0, 0 };

    while (controls[i].type)
    {
        if (controls[i].type == type && controls[i].value == value)
        {
            char *control = controls[i].control;

            if (strlen(control) == 1)
                C_TabbedOutput(tabs, "%i.\t\'%s\'\t%s", count, (control[0] == '=' ? "+" : control),
                    action);
            else
                C_TabbedOutput(tabs, "%i.\t%s\t%s", count, control, action);
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
            if (actions[action].keyboard)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].keyboard, keyboard,
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
                    if (controls[i].type == keyboard && actions[action].keyboard
                        && controls[i].value == *(int *)actions[action].keyboard)
                            C_Output(actions[action].action);
                    else if (controls[i].type == mouse && actions[action].mouse
                        && controls[i].value == *(int *)actions[action].mouse)
                        C_Output(actions[action].action);
                    else if (controls[i].type == gamepad && actions[action].gamepad
                        && controls[i].value == *(int *)actions[action].gamepad)
                        C_Output(actions[action].action);
                    ++action;
                }
            }
            else if (!strcasecmp(parm2, "none"))
            {
                while (actions[action].action[0])
                {
                    switch (controls[i].type)
                    {
                        case keyboard:
                            if (actions[action].keyboard
                                && controls[i].value == *(int *)actions[action].keyboard)
                            {
                                *(int *)actions[action].keyboard = 0;
                                M_SaveCVARs();
                            }
                            break;
                        case mouse:
                            if (actions[action].mouse
                                && controls[i].value == *(int *)actions[action].mouse)
                            {
                                *(int *)actions[action].mouse = 0;
                                M_SaveCVARs();
                            }
                            break;
                        case gamepad:
                            if (actions[action].gamepad
                                && controls[i].value == *(int *)actions[action].gamepad)
                            {
                                *(int *)actions[action].gamepad = 0;
                                M_SaveCVARs();
                            }
                            break;
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
                    switch (controls[i].type)
                    {
                        case keyboard:
                            *(int *)actions[action].keyboard = controls[i].value;
                            break;
                        case mouse:
                            *(int *)actions[action].mouse = controls[i].value;
                            break;
                        case gamepad:
                            *(int *)actions[action].gamepad = controls[i].value;
                            break;
                    }

                    if (cmd[0])
                        M_SaveCVARs();
                }
            }
        }
    }
}

static dboolean C_BloodCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, BLOODALIAS) >= 0);
}

void (*P_BloodSplatSpawner)(fixed_t, fixed_t, int, int, mobj_t *);

static void C_Blood(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BLOODALIAS);

        if (value >= 0)
        {
            r_blood = value;
            P_BloodSplatSpawner = (r_blood == noblood ? P_NullBloodSplatSpawner :
                (r_bloodsplats_max == unlimited ? P_SpawnBloodSplat : P_SpawnBloodSplat2));
            M_SaveCVARs();
        }
    }
    else
        C_Output(C_LookupAliasFromValue(r_blood, BLOODALIAS));
}

static dboolean C_BoolCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, BOOLALIAS) >= 0);
}

static void C_Bool(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_BOOLEAN))
        {
            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                int     value = C_LookupValueFromAlias(parm1, BOOLALIAS);

                if (value == 0 || value == 1)
                {
                    *(dboolean *)consolecmds[i].variable = !!value;
                    M_SaveCVARs();
                }
            }
            else
                C_Output(C_LookupAliasFromValue(*(dboolean *)consolecmds[i].variable, BOOLALIAS));
        }
        ++i;
    }
}

extern int      consolestrings;

static void C_Clear(char *cmd, char *parm1, char *parm2)
{
    consolestrings = 0;
    C_Output("");
}

static void C_CmdList(char *cmd, char *parm1, char *parm2)
{
    int i = 0;
    int count = 1;
    int tabs[8] = { 40, 192, 0, 0, 0, 0, 0, 0 };

    while (consolecmds[i].name[0])
    {
        if (consolecmds[i].type == CT_CMD && consolecmds[i].description[0]
            && (!parm1[0] || wildcard(consolecmds[i].name, parm1)))
            C_TabbedOutput(tabs, "%i.\t%s %s\t%s", count++, consolecmds[i].name,
                consolecmds[i].format, consolecmds[i].description);
        ++i;
    }
}

static void C_ConDump(char *cmd, char *parm1, char *parm2)
{
    if (consolestrings)
    {
        char    filename[MAX_PATH];
        FILE    *file;
        char    *exefolder = M_GetExecutableFolder();

        if (!parm1[0])
        {
            int count = 0;

            M_snprintf(filename, sizeof(filename), "%s"DIR_SEPARATOR_S"condump.txt", exefolder);
            while (M_FileExists(filename))
                M_snprintf(filename, sizeof(filename), "%s"DIR_SEPARATOR_S"condump (%i).txt",
                    exefolder, ++count);
        }
        else
            M_snprintf(filename, sizeof(filename), "%s"DIR_SEPARATOR_S"%s", exefolder, parm1);

        file = fopen(filename, "wt");

        if (file)
        {
            int i;

            for (i = 1; i < consolestrings - 1; ++i)
                if (console[i].type == divider)
                    fprintf(file, "%s\n", DIVIDERSTRING);
                else
                {
                    unsigned int        inpos;
                    unsigned int        spaces;
                    unsigned int        len = strlen(console[i].string);
                    unsigned int        outpos = 0;
                    int                 tabcount = 0;

                    for (inpos = 0; inpos < len; ++inpos)
                    {
                        char    ch = console[i].string[inpos];

                        if (ch != '\n')
                            if (ch == '\t')
                            {
                                unsigned int    tabstop = console[i].tabs[tabcount] / 5;

                                if (outpos < tabstop)
                                {
                                    for (spaces = 0; spaces < tabstop - outpos; ++spaces)
                                        fputc(' ', file);
                                    outpos = tabstop;
                                    ++tabcount;
                                }
                                else
                                {
                                    fputc(' ', file);
                                    ++outpos;
                                }
                            }
                            else
                            {
                                fputc(ch, file);
                                ++outpos;
                            }
                    }

                    if (con_timestamps && console[i].timestamp[0])
                    {
                        for (spaces = 0; spaces < 91 - outpos; ++spaces)
                            fputc(' ', file);
                        fputs(console[i].timestamp, file);
                    }

                    fputc('\n', file);
                }

            fclose(file);

            C_Output("Dumped the console to the file %s.", uppercase(filename));
        }
    }
}

static void C_CvarList(char *cmd, char *parm1, char *parm2)
{
    int i = 0;
    int count = 1;
    int tabs[8] = { 35, 179, 257, 0, 0, 0, 0, 0 };

    while (consolecmds[i].name[0])
    {
        if (consolecmds[i].type == CT_CVAR && (!parm1[0] || wildcard(consolecmds[i].name, parm1)))
        {
            if (consolecmds[i].flags & CF_BOOLEAN)
                C_TabbedOutput(tabs, "%i.\t%s\t%s\t%s", count++, consolecmds[i].name,
                    C_LookupAliasFromValue(*(dboolean *)consolecmds[i].variable,
                    consolecmds[i].aliases), consolecmds[i].description);
            else if ((consolecmds[i].flags & CF_INTEGER) && (consolecmds[i].flags & CF_PERCENT))
                C_TabbedOutput(tabs, "%i.\t%s\t%i%%\t%s", count++, consolecmds[i].name,
                    *(int *)consolecmds[i].variable, consolecmds[i].description);
            else if (consolecmds[i].flags & CF_INTEGER)
                C_TabbedOutput(tabs, "%i.\t%s\t%s\t%s", count++, consolecmds[i].name,
                    C_LookupAliasFromValue(*(int *)consolecmds[i].variable,
                    consolecmds[i].aliases), consolecmds[i].description);
            else if (consolecmds[i].flags & CF_FLOAT)
                C_TabbedOutput(tabs, "%i.\t%s\t%s%s\t%s", count++, consolecmds[i].name,
                    striptrailingzero(*(float *)consolecmds[i].variable,
                    ((consolecmds[i].flags & CF_PERCENT) ? 1 : 2)),
                    ((consolecmds[i].flags & CF_PERCENT) ? "%" : ""), consolecmds[i].description);
            else if (consolecmds[i].flags & CF_STRING)
                C_TabbedOutput(tabs, "%i.\t%s\t\"%.9s%s\"\t%s", count++, consolecmds[i].name,
                    *(char **)consolecmds[i].variable,
                    (strlen(*(char **)consolecmds[i].variable) > 9 ? "..." : ""),
                    consolecmds[i].description);
            else if ((consolecmds[i].flags & CF_POSITION) || (consolecmds[i].flags & CF_SIZE))
                C_TabbedOutput(tabs, "%i.\t%s\t%s\t%s", count++, consolecmds[i].name,
                    *(char **)consolecmds[i].variable, consolecmds[i].description);
            else if (consolecmds[i].flags & CF_TIME)
            {
                int tics = *(int *)consolecmds[i].variable / TICRATE;

                C_TabbedOutput(tabs, "%i.\t%s\t%02i:%02i:%02i\t%s", count++, consolecmds[i].name,
                    tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60,
                    consolecmds[i].description);
            }

        }
        ++i;
    }
}

static dboolean C_DeadZoneCondition(char *cmd, char *parm1, char *parm2)
{
    float value;

    if (!parm1[0])
        return true;
    if (parm1[strlen(parm1) - 1] == '%')
        parm1[strlen(parm1) - 1] = 0;
    return sscanf(parm1, "%10f", &value);
}

static void C_DeadZone(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        float   value = 0;

        if (parm1[strlen(parm1) - 1] == '%')
            parm1[strlen(parm1) - 1] = 0;
        sscanf(parm1, "%10f", &value);

        if (!strcasecmp(cmd, stringize(gp_deadzone_left)))
        {
            gp_deadzone_left = BETWEENF(gp_deadzone_left_min, value, gp_deadzone_left_max);
            gamepadleftdeadzone = (int)(gp_deadzone_left * (float)SHRT_MAX / 100.0f);
        }
        else
        {
            gp_deadzone_right = BETWEENF(gp_deadzone_right_min, value, gp_deadzone_right_max);
            gamepadrightdeadzone = (int)(gp_deadzone_right * (float)SHRT_MAX / 100.0f);
        }
        M_SaveCVARs();
    }
    else if (!strcasecmp(cmd, stringize(gp_deadzone_left)))
        C_Output("%s %s%%", cmd, striptrailingzero(gp_deadzone_left, 1));
    else
        C_Output("%s %s%%", cmd, striptrailingzero(gp_deadzone_right, 1));
}

static void C_Display(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = -1;

        sscanf(parm1, "%10i", &value);

        if (value >= vid_display_min && value <= vid_display_max && value != vid_display)
        {
            vid_display = value;
            M_SaveCVARs();
            I_RestartGraphics();
        }
    }
    else
        C_Output("%i", vid_display);
}

static void C_EndGame(char *cmd, char *parm1, char *parm2)
{
    M_EndingGame();
    C_HideConsoleFast();
}

static void C_ExitMap(char *cmd, char *parm1, char *parm2)
{
    G_ExitLevel();
    C_HideConsoleFast();
}

static dboolean C_FloatCondition(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    if (!parm1[0])
        return true;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_FLOAT))
        {
            float       value = -1.0f;

            sscanf(parm1, "%10f", &value);

            return (value >= 0.0f);
        }
        ++i;
    }
    return false;
}

static void C_Float(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_FLOAT))
        {
            if (parm1[0] && !(consolecmds[i].flags & CF_READONLY))
            {
                float     value = -1.0f;

                sscanf(parm1, "%10f", &value);

                if (value >= 0.0f)
                {
                    *(float *)consolecmds[i].variable = value;
                    M_SaveCVARs();
                }
            }
            else
                C_Output(striptrailingzero(*(float *)consolecmds[i].variable, 2));
        }
        ++i;
    }
}

static void C_Fullscreen(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BOOLALIAS);

        if ((value == 0 || value == 1) && value != vid_fullscreen)
            ToggleFullscreen();
    }
    else
        C_Output(C_LookupAliasFromValue(vid_fullscreen, BOOLALIAS));
}

extern int      st_palette;

static dboolean C_GammaCondition(char *cmd, char *parm1, char *parm2)
{
    float       value = -1.0f;

    if (!parm1[0] || !strcasecmp(parm1, "off"))
        return true;

    sscanf(parm1, "%10f", &value);

    return (value >= 0.0f);
}

static void C_Gamma(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        float   value = -1.0f;

        if (!strcasecmp(parm1, "off"))
            r_gamma = 1.0f;
        else
            sscanf(parm1, "%10f", &value);

        if (value >= 0.0f)
        {
            r_gamma = BETWEENF(r_gamma_min, value, r_gamma_max);
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

            I_SetPalette((byte *)W_CacheLumpName("PLAYPAL", PU_CACHE) + st_palette * 768);
            M_SaveCVARs();
        }
    }
    else
        C_Output(r_gamma == 1.0f ? "off" : striptrailingzero(r_gamma, 2));
}

extern int      cardsfound;

static dboolean C_GiveCondition(char *cmd, char *parm1, char *parm2)
{
    if (gamestate != GS_LEVEL)
        return false;

    if (!parm1[0])
        return true;

    if (!strcasecmp(parm1, "all")
        || !strcasecmp(parm1, "backpack")
        || !strcasecmp(parm1, "health")
        || !strcasecmp(parm1, "weapons")
        || !strcasecmp(parm1, "ammo")
        || !strcasecmp(parm1, "armor")
        || !strcasecmp(parm1, "keys"))
        return true;
    
    return false;
}

static void C_Give(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
        C_Output("%s %s", cmd, GIVECMDFORMAT);
    else
    {
        player_t    *player = &players[0];

        if (!strcasecmp(parm1, "all"))
        {
            P_GiveBackpack(player, false);
            P_GiveMegaHealth(player);
            P_GiveAllWeapons(player);
            P_GiveFullAmmo(player);
            P_GiveArmor(player, blue_armor_class);
            P_GiveAllCards(player);
        }
        else if (!strcasecmp(parm1, "backpack"))
            P_GiveBackpack(player, true);
        else if (!strcasecmp(parm1, "health"))
            P_GiveMegaHealth(player);
        else if (!strcasecmp(parm1, "weapons"))
            P_GiveAllWeapons(player);
        else if (!strcasecmp(parm1, "ammo"))
            P_GiveFullAmmo(player);
        else if (!strcasecmp(parm1, "armor"))
            P_GiveArmor(player, blue_armor_class);
        else if (!strcasecmp(parm1, "keys"))
            P_GiveAllCards(player);
    }
}

static dboolean C_GodCondition(char *cmd, char *parm1, char *parm2)
{
    return (gamestate == GS_LEVEL && players[0].playerstate == PST_LIVE);
}

static void C_God(char *cmd, char *parm1, char *parm2)
{
    player_t    *player = &players[0];

    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BOOLALIAS);

        if (value == 0)
            player->cheats &= ~CF_GODMODE;
        else if (value == 1)
            player->cheats |= CF_GODMODE;
    }
    else
        player->cheats ^= CF_GODMODE;

    C_Output((player->cheats & CF_GODMODE) ? s_STSTR_GODON : s_STSTR_GODOFF);
}

static dboolean C_GraphicDetailCondition(char *cmd, char *parm1, char *parm2)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, DETAILALIAS) >= 0);
}

static void C_GraphicDetail(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int value = C_LookupValueFromAlias(parm1, DETAILALIAS);

        if (value == 0 || value == 1)
        {
            r_detail = !!value;
            M_SaveCVARs();
        }
    }
    else
        C_Output(C_LookupAliasFromValue(r_detail, DETAILALIAS));
}

static void C_Help(char *cmd, char *parm1, char *parm2)
{
    C_HideConsoleFast();
    M_ShowHelp();
}

static void C_Hud(char *cmd, char *parm1, char *parm2)
{
    if (vid_widescreen || r_screensize == r_screensize_max)
        C_Bool(cmd, parm1, "");
}

static dboolean C_IntCondition(char *cmd, char *parm1, char *parm2)
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
            
            sscanf(parm1, "%10i", &value);

            return (value >= consolecmds[i].minimumvalue && value <= consolecmds[i].maximumvalue);
        }
        ++i;
    }
    return false;
}

static void C_Int(char *cmd, char *parm1, char *parm2)
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
                    sscanf(parm1, "%10i", &value);

                if (value >= 0)
                {
                    *(int *)consolecmds[i].variable = value;
                    M_SaveCVARs();
                }
            }
            else
                C_Output(C_LookupAliasFromValue(*(int *)consolecmds[i].variable,
                    consolecmds[i].aliases));
        }
        ++i;
    }
}

static int      killcmdtype = NUMMOBJTYPES;

void A_Fall(mobj_t *actor);

static dboolean C_KillCondition(char *cmd, char *parm1, char *parm2)
{
    if (gamestate == GS_LEVEL)
    {
        int i;

        if (!parm1[0] || !strcasecmp(parm1, "player"))
            return players[0].mo->health;

        if (!strcasecmp(parm1, "monsters") || !strcasecmp(parm1, "all"))
            return true;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (!strcasecmp(parm1, removespaces(mobjinfo[i].name1))
                || !strcasecmp(parm1, removespaces(mobjinfo[i].plural1))
                || (mobjinfo[i].name2[0] && !strcasecmp(parm1, removespaces(mobjinfo[i].name2)))
                || (mobjinfo[i].plural2[0] && !strcasecmp(parm1,
                removespaces(mobjinfo[i].plural2))))
            {
                dboolean    kill = true;

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

static void C_Kill(char *cmd, char *parm1, char *parm2)
{
    static char buffer[1024];

    if (!parm1[0] || !strcasecmp(parm1, "player"))
    {
        players[0].health = 0;
        P_KillMobj(players[0].mo, players[0].mo);
        M_snprintf(buffer, sizeof(buffer), "%s killed %s", playername,
            (!strcasecmp(playername, "you") ? "yourself" : "themselves"));
        buffer[0] = toupper(buffer[0]);
        C_Output("%s.", buffer);
        players[0].message = buffer;
        message_dontfuckwithme = true;
        C_HideConsole();
    }
    else
    {
        int     i;
        int     kills = 0;

        if (!strcasecmp(parm1, "monsters") || !strcasecmp(parm1, "all"))
        {
            for (i = 0; i < numsectors; ++i)
            {
                mobj_t      *thing = sectors[i].thinglist;

                while (thing)
                {
                    if (thing->health > 0)
                    {
                        mobjtype_t      type = thing->type;

                        if (type == MT_PAIN)
                        {
                            A_Fall(thing);
                            P_SetMobjState(thing, S_PAIN_DIE6);
                            players[0].killcount++;
                            stat_monsterskilled++;
                            kills++;
                        }
                        else if ((thing->flags & MF_SHOOTABLE) && type != MT_PLAYER
                            && type != MT_BARREL && type != MT_BOSSBRAIN)
                        {
                            P_DamageMobj(thing, NULL, NULL, thing->health);
                            thing->momx += FRACUNIT * M_RandomInt(-1, 1);
                            thing->momy += FRACUNIT * M_RandomInt(-1, 1);
                            kills++;
                        }
                    }
                    thing = thing->snext;
                }
            }

            if (kills)
            {
                M_snprintf(buffer, sizeof(buffer), "%s monster%s killed", commify(kills),
                    (kills == 1 ? "" : "s"));
                C_Output("%s.", buffer);
                players[0].message = buffer;
                message_dontfuckwithme = true;
                C_HideConsole();
            }
            else
                C_Output("No monsters %s kill.", (!totalkills ? "to" : "left to"));
        }
        else
        {
            int type = P_FindDoomedNum(killcmdtype);
            int dead = 0;

            for (i = 0; i < numsectors; ++i)
            {
                mobj_t      *thing = sectors[i].thinglist;

                while (thing)
                {
                    if (type == thing->type)
                        if (type == MT_PAIN)
                        {
                            if (thing->health > 0)
                            {
                                A_Fall(thing);
                                P_SetMobjState(thing, S_PAIN_DIE6);
                                players[0].killcount++;
                                stat_monsterskilled++;
                                kills++;
                            }
                            else
                                dead++;
                        }
                        else if ((thing->flags & MF_SHOOTABLE) && thing->health > 0)
                        {
                            P_DamageMobj(thing, NULL, NULL, thing->health);
                            thing->momx += FRACUNIT * M_RandomInt(-1, 1);
                            thing->momy += FRACUNIT * M_RandomInt(-1, 1);
                            kills++;
                        }
                        else if (thing->flags & MF_CORPSE)
                            dead++;
                    thing = thing->snext;
                }
            }

            if (kills)
            {
                M_snprintf(buffer, sizeof(buffer), "%s %s %s", commify(kills),
                    (kills == 1 ? mobjinfo[type].name1 : mobjinfo[type].plural1),
                    (type == MT_BARREL ? "exploded" : "killed"));
                C_Output("%s.", buffer);
                players[0].message = buffer;
                message_dontfuckwithme = true;
                C_HideConsole();
            }
            else
                C_Output("No %s %s %s.", mobjinfo[type].plural1, (dead ? "left to" : "to"),
                    (type == MT_BARREL ? "explode" : "kill"));
        }
    }
}

static dboolean C_LoadCondition(char *cmd, char *parm1, char *parm2)
{
    return (parm1[0] != '\0');
}

static void C_Load(char *cmd, char *parm1, char *parm2)
{
    G_LoadGame(M_StringJoin(savegamefolder, parm1,
        (M_StringEndsWith(parm1, ".save") ? "" : ".save"), NULL));
}

static int      mapcmdepisode;
static int      mapcmdmap;

extern dboolean samelevel;
extern menu_t   EpiDef;
extern int      idclevtics;

static dboolean C_MapCondition(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
        return true;

    parm1 = uppercase(parm1);
    mapcmdepisode = 0;
    mapcmdmap = 0;

    if (gamemode == commercial)
    {
        if (BTSX)
        {
            if (sscanf(parm1, "MAP%02iC", &mapcmdmap) == 1)
                return (W_CheckNumForName(parm1) >= 0);
            else
            {
                if (sscanf(parm1, "E%1iM0%1i", &mapcmdepisode, &mapcmdmap) != 2)
                    sscanf(parm1, "E%1iM%2i", &mapcmdepisode, &mapcmdmap);
                if (mapcmdmap && ((mapcmdepisode == 1 && BTSXE1) || (mapcmdepisode == 2 && BTSXE2)
                    || (mapcmdepisode == 3 && BTSXE3)))
                {
                    static char     lump[6];

                    M_snprintf(lump, sizeof(lump), "MAP%02i", mapcmdmap);
                    return (W_CheckMultipleLumps(lump) == 2);
                }
            }
        }
        if (sscanf(parm1, "MAP0%1i", &mapcmdmap) != 1)
            if (sscanf(parm1, "MAP%2i", &mapcmdmap) != 1)
                return false;
        if (BTSX && W_CheckMultipleLumps(parm1) == 1)
            return false;
        mapcmdepisode = 1;
        if (gamestate != GS_LEVEL && gamemission == pack_nerve)
            gamemission = doom2;
    }
    else if (sscanf(parm1, "E%1iM%1i", &mapcmdepisode, &mapcmdmap) != 2)
        return false;

    return (W_CheckNumForName(parm1) >= 0);
}

static void C_Map(char *cmd, char *parm1, char *parm2)
{
    static char buffer[1024];

    if (!parm1[0])
    {
        C_Output("%s %s", cmd, MAPCMDFORMAT);
        return;
    }
    samelevel = (gameepisode == mapcmdepisode && gamemap == mapcmdmap);
    gameepisode = mapcmdepisode;
    if (gamemission == doom && gameepisode <= 4)
    {
        episode = gameepisode - 1;
        EpiDef.lastOn = episode;
    }
    gamemap = mapcmdmap;
    M_snprintf(buffer, sizeof(buffer), (samelevel ? "Restarting %s..." : "Warping to %s..."),
        uppercase(parm1));
    C_Output(buffer);
    players[0].message = buffer;
    message_dontfuckwithme = true;
    if (gamestate == GS_LEVEL)
    {
        idclevtics = MAPCHANGETICS;
        C_HideConsole();
    }
    else
    {
        G_DeferredInitNew(skilllevel, gameepisode, gamemap);
        C_HideConsoleFast();
    }
}

extern int      dehcount;
extern char     **mapnames[];
extern char     **mapnames2[];
extern char     **mapnames2_bfg[];
extern char     **mapnamesp[];
extern char     **mapnamest[];
extern char     **mapnamesn[];

static void C_MapList(char *cmd, char *parm1, char *parm2)
{
    int         i, j;
    int         count = 0;
    int         tabs[8] = { 40, 90, 350, 0, 0, 0, 0, 0 };
    char        **maplist;

    // initialize map list
    maplist = malloc(numlumps * sizeof(char *));
    for (i = 0; i < numlumps; ++i)
        maplist[i] = malloc(256 * sizeof(char));

    // search through lumps for maps
    for (i = 0; i < numlumps; ++i)
    {
        int             ep = 0;
        int             map = 0;
        char            lump[8];
        char            wad[MAX_PATH];
        dboolean        replaced;
        dboolean        pwad;

        M_StringCopy(lump, uppercase(lumpinfo[i]->name), 8);

        if (gamemode == commercial)
        {
            ep = 1;
            sscanf(lump, "MAP0%1i", &map);
            if (!map)
                sscanf(lump, "MAP%2i", &map);
        }
        else
            sscanf(lump, "E%1iM%1i", &ep, &map);

        if (!ep-- || !map--)
            continue;

        M_StringCopy(wad, uppercase(M_ExtractFilename(lumpinfo[i]->wad_file->path)), MAX_PATH);
        replaced = (W_CheckMultipleLumps(lump) > 1 && !chex && !FREEDOOM);
        pwad = (lumpinfo[i]->wad_file->type == PWAD);

        switch (gamemission)
        {
            case doom:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t%s\t%s", lump,
                        (replaced && dehcount == 1 ? "-" : *mapnames[ep * 9 + map]),
                        (modifiedgame ? wad : ""));
                break;
            case doom2:
                if (strcasecmp(wad, "nerve.wad") && (!replaced || pwad || nerve)
                    && (pwad || !BTSX))
                    if (BTSX)
                    {
                        if (strchr(*mapnames2[map], ':'))
                            M_snprintf(maplist[count++], 256, "%s",
                                M_StringReplace(*mapnames2[map], ": ", "\t"));
                    }
                    else
                        M_snprintf(maplist[count++], 256, "%s\t%s\t%s", lump, (replaced
                            && dehcount == 1 && !nerve ? "  -" : (bfgedition ? *mapnames2_bfg[map]
                            : *mapnames2[map])), (modifiedgame && !nerve ? wad : ""));
                break;
            case pack_nerve:
                if (!strcasecmp(wad, "nerve.wad"))
                    M_snprintf(maplist[count++], 256, "%s\t%s", lump, *mapnamesn[map]);
                break;
            case pack_plut:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t%s\t%s", lump,
                        (replaced && dehcount == 1 ? "-" : *mapnamesp[map]),
                        (modifiedgame ? wad : ""));
                break;
            case pack_tnt:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t%s\t%s", lump,
                        (replaced && dehcount == 1 ? "-" : *mapnamest[map]),
                        (modifiedgame ? wad : ""));
                break;
        }
    }

    // sort the map list
    for (i = 0; i < count; i++)
        for (j = i + 1; j < count; j++)
        {
            if (strcmp(maplist[i], maplist[j]) > 0)
            {
                char    temp[256];

                strcpy(temp, maplist[i]);
                strcpy(maplist[i], maplist[j]);
                strcpy(maplist[j], temp);
            }
        }

    // display the map list
    for (i = 0; i < count; ++i)
        C_TabbedOutput(tabs, "%i.\t%s", i + 1, maplist[i]);

    free(maplist);
}

#define AA      "Andre Arsenault"
#define AD      "Andrew Dowswell"
#define AM      "American McGee"
#define BK      "Brian Kidby"
#define CB      "Christopher Buteau"
#define DC      "Dario Casali"
#define DJ      "Dean Johnson"
#define DO      "Drake O'Brien"
#define JA      "John Anderson"
#define JD      "Jim Dethlefsen"
#define JL      "Jim Lowell"
#define JM      "Jim Mentzer"
#define JM2     "John Minadeo"
#define JR      "John Romero"
#define JS      "Jimmy Sieben"
#define JW      "John Wakelin"
#define MB      "Michael Bukowski"
#define MC      "Milo Casali"
#define MS      "Mark Snell"
#define PT      "Paul Turnbull"
#define RH      "Richard Heath"
#define RM      "Russell Meakim"
#define RP      "Robin Patenall"
#define SG      "Shawn Green"
#define SP      "Sandy Petersen"
#define TH      "Tom Hall"
#define TH2     "Ty Halderman"
#define TM      "Tom Mustaine"
#define TW      "Tim Willits"
#define WW      "William D. Whitaker"
#define AMSP    AM" and "SP
#define BKTH2   BK" and "TH2
#define DCMC    DC" and "MC
#define DCTH2   DC" and "TH2
#define JRTH    JR" and "TH
#define JSTH2   JS" and "TH2
#define MSJL    MS" and "JL
#define RPJM2   RP" and "JM2
#define SPTH    SP" and "TH

char *authors[][6] =
{
    /* xy      doom   doom2  tnt    plut   nerve */
    /* 00 */ { "",    "",    "",    DCMC,  ""    },
    /* 01 */ { "",    SP,    TM,    DCMC,  RM    },
    /* 02 */ { "",    AM,    JW,    DCMC,  RH    },
    /* 03 */ { "",    AM,    RPJM2, DCMC,  RM    },
    /* 04 */ { "",    AM,    TH2,   DCMC,  RM    },
    /* 05 */ { "",    AM,    JD,    DCMC,  RH    },
    /* 06 */ { "",    AM,    JSTH2, DCMC,  RH    },
    /* 07 */ { "",    AMSP,  AD,    DCMC,  RH    },
    /* 08 */ { "",    SP,    JM2,   DCMC,  RH    },
    /* 09 */ { "",    SP,    JSTH2, DCMC,  RM    },
    /* 10 */ { "",    SPTH,  TM,    DCMC,  ""    },
    /* 11 */ { JR,    JR,    DJ,    DCMC,  ""    },
    /* 12 */ { JR,    SP,    JL,    DCMC,  ""    },
    /* 13 */ { JR,    SP,    BKTH2, DCMC,  ""    },
    /* 14 */ { JRTH,  AM,    RP,    DCMC,  ""    },
    /* 15 */ { JR,    JR,    WW,    DCMC,  ""    },
    /* 16 */ { JR,    SP,    AA,    DCMC,  ""    },
    /* 17 */ { JR,    JR,    TM,    DCMC,  ""    },
    /* 18 */ { SPTH,  SP,    DCTH2, DCMC,  ""    },
    /* 19 */ { JR,    SP,    TH2,   DCMC,  ""    },
    /* 20 */ { "",    JR,    DO,    DCMC,  ""    },
    /* 21 */ { SPTH,  SP,    DO,    DCMC,  ""    },
    /* 22 */ { SPTH,  AM,    CB,    DCMC,  ""    },
    /* 23 */ { SPTH,  SP,    PT,    DCMC,  ""    },
    /* 24 */ { SPTH,  SP,    DJ,    DCMC,  ""    },
    /* 25 */ { SP,    SG,    JM,    DCMC,  ""    },
    /* 26 */ { SP,    JR,    MSJL,  DCMC,  ""    },
    /* 27 */ { SPTH,  SP,    DO,    DCMC,  ""    },
    /* 28 */ { SP,    SP,    MC,    DCMC,  ""    },
    /* 29 */ { SP,    JR,    JS,    DCMC,  ""    },
    /* 30 */ { "",    SP,    JS,    DCMC,  ""    },
    /* 31 */ { "",    SP,    DC,    DCMC,  ""    },
    /* 32 */ { "",    SP,    DC,    DCMC,  ""    },
    /* 33 */ { SPTH,  MB,    "",    "",    ""    },
    /* 34 */ { "",    "",    "",    "",    ""    },
    /* 35 */ { "",    "",    "",    "",    ""    },
    /* 36 */ { "",    "",    "",    "",    ""    },
    /* 37 */ { "",    "",    "",    "",    ""    },
    /* 38 */ { "",    "",    "",    "",    ""    },
    /* 39 */ { "",    "",    "",    "",    ""    },
    /* 40 */ { "",    "",    "",    "",    ""    },
    /* 41 */ { AM,    "",    "",    "",    ""    },
    /* 42 */ { JR,    "",    "",    "",    ""    },
    /* 43 */ { SG,    "",    "",    "",    ""    },
    /* 44 */ { AM,    "",    "",    "",    ""    },
    /* 45 */ { TW,    "",    "",    "",    ""    },
    /* 46 */ { JR,    "",    "",    "",    ""    },
    /* 47 */ { JA,    "",    "",    "",    ""    },
    /* 48 */ { SG,    "",    "",    "",    ""    },
    /* 49 */ { TW,    "",    "",    "",    ""    }
};

static void C_MapStats(char *cmd, char *parm1, char *parm2)
{
    int tabs[8] = { 160, 0, 0, 0, 0, 0, 0, 0 };

    C_TabbedOutput(tabs, "Title\t%s", mapnumandtitle);

    {
        int     i = (gamemission == doom ? gameepisode * 10 : 0) + gamemap;

        if (canmodify && authors[i][gamemission])
            C_TabbedOutput(tabs, "Author\t%s", authors[i][gamemission]);
    }

    {
        static char     lumpname[6];
        int             i;

        if (gamemode == commercial)
            M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", startmap);
        else
            M_snprintf(lumpname, sizeof(lumpname), "E%iM%i", startepisode, startmap);
        i = W_CheckNumForName(lumpname);
        C_TabbedOutput(tabs, "%s\t%s", (lumpinfo[i]->wad_file->type == IWAD ? "IWAD" : "PWAD"),
            uppercase(lumpinfo[i]->wad_file->path));
    }

    C_TabbedOutput(tabs, "Node format\t%s", (mapformat == DOOMBSP ? "Regular nodes" :
        (mapformat == DEEPBSP ? "DeePBSP v4 extended nodes" :
        "ZDoom uncompressed extended nodes")));

    if (blockmaprecreated)
        C_TabbedOutput(tabs, "Blockmap\tRecreated");

    C_TabbedOutput(tabs, "Total vertices\t%s", commify(numvertexes));

    C_TabbedOutput(tabs, "Total sides\t%s", commify(numsides));

    C_TabbedOutput(tabs, "Total lines\t%s", commify(numlines));

    C_TabbedOutput(tabs, "Line specials\t%s-compatible", (boomlinespecials ? "Boom" : "Vanilla"));

    C_TabbedOutput(tabs, "Total sectors\t%s", commify(numsectors));

    C_TabbedOutput(tabs, "Total things\t%s", commify(numthings));

    {
        int i, min_x = INT_MAX, max_x = INT_MIN, min_y = INT_MAX, max_y = INT_MIN;

        for (i = 0; i < numvertexes; ++i)
        {
            fixed_t x = vertexes[i].x;
            fixed_t y = vertexes[i].y;

            if (x < min_x)
                min_x = x;
            else if (x > max_x)
                max_x = x;
            if (y < min_y)
                min_y = y;
            else if (y > max_y)
                max_y = y;
        }
        C_TabbedOutput(tabs, "Size\t%sx%s",
            commify((max_x - min_x) >> FRACBITS), commify((max_y - min_y) >> FRACBITS));
    }

    if (mus_playing && !nomusic)
    {
        int     lumps = W_CheckMultipleLumps(mus_playing->name);

        if (((gamemode == commercial || gameepisode > 1) && lumps == 1)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2))
            C_TabbedOutput(tabs, "Music title\t%s", mus_playing->title);
    }
}

static dboolean C_MaxBloodSplatsCondition(char *cmd, char *parm1, char *parm2)
{
    int value = 0;

    return (!parm1[0] || C_LookupValueFromAlias(parm1, SPLATALIAS) >= 0
        || sscanf(parm1, "%10i", &value));
}

static void C_MaxBloodSplats(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, SPLATALIAS);

        if (value < 0)
            sscanf(parm1, "%10i", &value);
        if (value >= 0)
        {
            r_bloodsplats_max = value;
            M_SaveCVARs();

            if (!r_bloodsplats_max)
                P_BloodSplatSpawner = P_NullBloodSplatSpawner;
            else if (r_bloodsplats_max == unlimited)
                P_BloodSplatSpawner = P_SpawnBloodSplat;
            else
                P_BloodSplatSpawner = P_SpawnBloodSplat2;
        }
    }
    else
        C_Output(C_LookupAliasFromValue(r_bloodsplats_max, SPLATALIAS));
}

static void C_NoClip(char *cmd, char *parm1, char *parm2)
{
    player_t    *player = &players[0];

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

    HU_PlayerMessage(((player->cheats & CF_NOCLIP) ? s_STSTR_NCON : s_STSTR_NCOFF), false);
}

static void C_NoTarget(char *cmd, char *parm1, char *parm2)
{
    player_t    *player = &players[0];

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

    if (player->cheats & CF_NOTARGET)
    {
        int     i;

        for (i = 0; i < numsectors; ++i)
        {
            mobj_t   *mo = sectors[i].thinglist;

            while (mo)
            {
                if (mo->target && mo->target->player)
                    P_SetTarget(&mo->target, NULL);
                if (mo->tracer && mo->tracer->player)
                    P_SetTarget(&mo->tracer, NULL);
                if (mo->lastenemy && mo->lastenemy->player)
                    P_SetTarget(&mo->lastenemy, NULL);
                mo = mo->snext;
            }

            P_SetTarget(&sectors[i].soundtarget, NULL);
        }
        HU_PlayerMessage(s_STSTR_NTON, false);
    }
    else
        HU_PlayerMessage(s_STSTR_NTOFF, false);
}

static void C_PixelSize(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        r_lowpixelsize = strdup(parm1);

        GetPixelSize();

        if (strcasecmp(r_lowpixelsize, parm1))
            M_SaveCVARs();
    }
    else
        C_Output(r_lowpixelsize);
}

static void C_PlayerName(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        if (strcasecmp(parm1, EMPTYVALUE))
        {
            playername = strdup(parm1);
            M_SaveCVARs();
        }
    }
    else
        C_Output("\"%s\"", playername);
}

static void C_PlayerStats(char *cmd, char *parm1, char *parm2)
{
    int tabs[8] = { 170, 280, 0, 0, 0, 0, 0, 0 };
    int tics = leveltime / TICRATE;

    C_TabbedOutput(tabs, "\t~Current Map~\t~Total~");

    if ((players[0].cheats & CF_ALLMAP) || (players[0].cheats & CF_ALLMAP_THINGS))
        C_TabbedOutput(tabs, "Amount of map revealed\t100%%\t-");
    else
    {
        int i = 0;
        int totallines = numlines;
        int totallinesmapped = 0;

        while (i < numlines)
            totallines -= !!(lines[i++].flags & ML_DONTDRAW);
        i = 0;
        while (i < numlines)
            totallinesmapped += !!(lines[i++].flags & ML_MAPPED);
        C_TabbedOutput(tabs, "Amount of map revealed\t%i%%\t-",
            totallinesmapped * 100 / totallines);
    }

    C_TabbedOutput(tabs, "Monsters killed\t%s of %s (%i%%)\t%s",
        commify(players[0].killcount), commify(totalkills),
        (totalkills ? players[0].killcount * 100 / totalkills : 0), commify(stat_monsterskilled));

    C_TabbedOutput(tabs, "Items picked up\t%s of %s (%i%%)\t%s",
        commify(players[0].itemcount), commify(totalitems),
        (totalitems ? players[0].itemcount * 100 / totalitems : 0), commify(stat_itemspickedup));

    C_TabbedOutput(tabs, "Secrets revealed\t%s of %s (%i%%)\t%s",
        commify(players[0].secretcount), commify(totalsecret),
        (totalsecret ? players[0].secretcount * 100 / totalsecret : 0),
        commify(stat_secretsrevealed));

    C_TabbedOutput(tabs, "Time spent in map\t%02i:%02i:%02i\t-",
        tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60);

    C_TabbedOutput(tabs, "Damage inflicted\t%s\t%s",
        commify(players[0].damageinflicted), commify(stat_damageinflicted));

    C_TabbedOutput(tabs, "Damage received\t%s\t%s",
        commify(players[0].damagereceived), commify(stat_damagereceived));

    C_TabbedOutput(tabs, "Deaths\t-\t%s", commify(stat_deaths));
}

static void C_Quit(char *cmd, char *parm1, char *parm2)
{
    I_Quit(true);
}

static dboolean C_ResurrectCondition(char *cmd, char *parm1, char *parm2)
{
    return (gamestate == GS_LEVEL && players[0].playerstate == PST_DEAD);
}

static void C_Resurrect(char *cmd, char *parm1, char *parm2)
{
    P_ResurrectPlayer(&players[0]);
    C_HideConsole();
}

static dboolean C_SaveCondition(char *cmd, char *parm1, char *parm2)
{
    return (parm1[0] != '\0' && gamestate == GS_LEVEL && players[0].playerstate == PST_LIVE);
}

static void C_Save(char *cmd, char *parm1, char *parm2)
{
    G_SaveGame(-1, "", M_StringJoin(savegamefolder, parm1,
        (M_StringEndsWith(parm1, ".save") ? "" : ".save"), NULL));
}

static void C_ScaleDriver(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        if (!strcasecmp(parm1, EMPTYVALUE))
        {
            vid_scaledriver = "";
            M_SaveCVARs();
            I_RestartGraphics();
        }
        else if ((!strcasecmp(parm1, vid_scaledriver_direct3d)
            || !strcasecmp(parm1, vid_scaledriver_opengl)
            || !strcasecmp(parm1, vid_scaledriver_software))
            && strcasecmp(parm1, vid_scaledriver))
        {
            vid_scaledriver = strdup(parm1);
            M_SaveCVARs();
            I_RestartGraphics();
        }
    }
    else
        C_Output("\"%s\"", vid_scaledriver);
}

static void C_ScaleFilter(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        if ((!strcasecmp(parm1, vid_scalefilter_nearest)
            || !strcasecmp(parm1, vid_scalefilter_linear))
            && strcasecmp(parm1, vid_scalefilter))
        {
            vid_scalefilter = strdup(parm1);
            M_SaveCVARs();
            I_RestartGraphics();
        }
    }
    else
        C_Output("\"%s\"", vid_scalefilter);
}

static void C_ScreenResolution(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        vid_screenresolution = strdup(parm1);

        GetScreenResolution();

        if (strcasecmp(vid_screenresolution, parm1))
        {
            M_SaveCVARs();

            if (vid_fullscreen)
                I_RestartGraphics();
        }
    }
    else
        C_Output(vid_screenresolution);
}

static void C_ScreenSize(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = -1;

        sscanf(parm1, "%10i", &value);

        if (value >= r_screensize_min && value <= r_screensize_max)
        {
            if (vid_widescreen || (returntowidescreen && gamestate != GS_LEVEL))
            {
                if (value == r_screensize_max)
                    --value;
                else if (value <= r_screensize_max - 1)
                {
                    r_hud = true;
                    ToggleWidescreen(false);
                }
            }
            else
            {
                if (value >= r_screensize_max - 1)
                {
                    if (gamestate != GS_LEVEL)
                    {
                        returntowidescreen = true;
                        r_hud = true;
                    }
                    else
                    {
                        ToggleWidescreen(true);
                        if (vid_widescreen)
                            value = r_screensize_max - 1;
                    }
                }
            }
            r_screensize = value;
            M_SaveCVARs();
            R_SetViewSize(r_screensize);
        }
    }
    else
        C_Output("%i", r_screensize);
}

static void C_ShowFPS(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BOOLALIAS);

        if ((value == 0 || value == 1) && value != vid_showfps)
        {
            vid_showfps = !!value;
            updatefunc = (vid_showfps ? I_FinishUpdateShowFPS : I_FinishUpdate);
        }
    }
    else
        C_Output(C_LookupAliasFromValue(vid_showfps, BOOLALIAS));
}

static int      spawntype = NUMMOBJTYPES;

static dboolean C_SpawnCondition(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
        return true;

    if (gamestate == GS_LEVEL)
    {
        int i;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (!strcasecmp(parm1, removespaces(mobjinfo[i].name1))
                || (mobjinfo[i].name2[0] && !strcasecmp(parm1, removespaces(mobjinfo[i].name2))))
            {
                dboolean    spawn = true;

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

static void C_Spawn(char *cmd, char *parm1, char *parm2)
{
    if (!parm1[0])
    {
        C_Output("%s %s", cmd, SPAWNCMDFORMAT);
        return;
    }
    else
    {
        mobj_t      *player = players[0].mo;
        fixed_t     x = player->x;
        fixed_t     y = player->y;
        angle_t     angle = player->angle >> ANGLETOFINESHIFT;
        mobj_t      *thing = P_SpawnMobj(x + 100 * finecosine[angle], y + 100 * finesine[angle],
                        ONFLOORZ, P_FindDoomedNum(spawntype));
        int         flags = thing->flags;

        thing->angle = R_PointToAngle2(thing->x, thing->y, x, y);

        if (flags & MF_COUNTKILL)
            ++totalkills;
        else if (flags & MF_COUNTITEM)
            ++totalitems;
    }
}

static void C_Str(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_STRING))
        {
            if (!strcasecmp(parm1, EMPTYVALUE))
            {
                *(char **)consolecmds[i].variable = "";
                M_SaveCVARs();
            }
            else if (parm1[0])
            {
                *(char **)consolecmds[i].variable = strdup(parm1);
                M_SaveCVARs();
            }
            else
                C_Output("\"%s\"", *(char **)consolecmds[i].variable);
            break;
        }
        ++i;
    }
}

static void C_ThingList(char *cmd, char *parm1, char *parm2)
{
    thinker_t   *th;
    int         count = 0;
    int         tabs[8] = { 45, 250, 0, 0, 0, 0, 0, 0 };

    for (th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
    {
        mobj_t      *mobj = (mobj_t *)th;

        C_TabbedOutput(tabs, "%i.\t%s\t(%i,%i,%i)", ++count, mobjinfo[mobj->type].name1,
            mobj->x >> FRACBITS, mobj->y >> FRACBITS, mobj->z >> FRACBITS);
    }
}

static void C_Time(char *cmd, char *parm1, char *parm2)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (!strcasecmp(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_TIME))
        {
            int tics = *(int *)consolecmds[i].variable / TICRATE;

            C_Output("%02i:%02i:%02i", tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60);
        }
        ++i;
    }
}

static void C_UnBind(char *cmd, char *parm1, char *parm2)
{
    C_Bind(cmd, parm1, "none");
}

static dboolean C_VolumeCondition(char *cmd, char *parm1, char *parm2)
{
    int value = -1;

    if (!parm1[0])
        return true;
    if (parm1[strlen(parm1) - 1] == '%')
        parm1[strlen(parm1) - 1] = 0;

    sscanf(parm1, "%10i", &value);

    return ((!strcasecmp(cmd, stringize(s_musicvolume)) && value >= s_musicvolume_min
        && value <= s_musicvolume_max) || (!strcasecmp(cmd, stringize(s_sfxvolume))
        && value >= s_sfxvolume_min && value <= s_sfxvolume_max));
}

static void C_Volume(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int value = 0;

        if (parm1[strlen(parm1) - 1] == '%')
            parm1[strlen(parm1) - 1] = 0;
        sscanf(parm1, "%10i", &value);

        if (!strcasecmp(cmd, stringize(s_musicvolume)))
        {
            s_musicvolume = value;
            musicVolume = (BETWEEN(s_musicvolume_min, s_musicvolume,
                s_musicvolume_max) * 15 + 50) / 100;
            S_SetMusicVolume((int)(musicVolume * (127.0f / 15.0f)));
        }
        else
        {
            s_sfxvolume = value;
            sfxVolume = (BETWEEN(s_sfxvolume_min, s_sfxvolume, s_sfxvolume_max) * 15 + 50) / 100;
            S_SetSfxVolume((int)(sfxVolume * (127.0f / 15.0f)));
        }

        M_SaveCVARs();
    }
    else
        C_Output("%i%%", (!strcasecmp(cmd, stringize(s_musicvolume)) ? s_musicvolume :
            s_sfxvolume));
}

static void C_Vsync(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BOOLALIAS);

        if ((value == 0 || value == 1) && value != vid_vsync)
        {
            vid_vsync = !!value;
            M_SaveCVARs();
            I_RestartGraphics();
        }
    }
    else
        C_Output(C_LookupAliasFromValue(vid_vsync, BOOLALIAS));
}

static void C_Widescreen(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BOOLALIAS);

        if ((value == 0 || value == 1) && value != vid_widescreen)
        {
            vid_widescreen = !!value;
            if (vid_widescreen)
            {
                if (gamestate == GS_LEVEL)
                {
                    ToggleWidescreen(true);
                    if (vid_widescreen)
                        S_StartSound(NULL, sfx_stnmov);
                }
                else
                {
                    returntowidescreen = true;
                    r_hud = true;
                }
            }
            else
            {
                ToggleWidescreen(false);
                if (!vid_widescreen)
                    S_StartSound(NULL, sfx_stnmov);
            }
            M_SaveCVARs();
        }
    }
    else
        C_Output(C_LookupAliasFromValue(vid_widescreen, BOOLALIAS));
}

extern SDL_Window       *window;

static void C_WindowPosition(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        vid_windowposition = strdup(parm1);
 
        GetWindowPosition();

        if (strcasecmp(vid_windowposition, parm1))
        {
            M_SaveCVARs();

            if (!vid_fullscreen)
                SDL_SetWindowPosition(window, windowx, windowy);
        }
    }
    else
        C_Output("%s", vid_windowposition);
}

static void C_WindowSize(char *cmd, char *parm1, char *parm2)
{
    if (parm1[0])
    {
        vid_windowsize = strdup(parm1);

        GetWindowSize();

        if (strcasecmp(vid_windowsize, parm1))
        {
            M_SaveCVARs();

            if (!vid_fullscreen)
                SDL_SetWindowSize(window, windowwidth, windowheight);
        }
    }
    else
        C_Output(vid_windowsize);
}
