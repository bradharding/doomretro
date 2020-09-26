/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include <float.h>

#if defined(_WIN32)
#include <Windows.h>
#include <ShellAPI.h>
#endif

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_iwad.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_pspr.h"
#include "p_setup.h"
#include "p_tick.h"
#include "r_sky.h"
#include "s_sound.h"
#include "sc_man.h"
#include "sounds.h"
#include "st_lib.h"
#include "st_stuff.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define ALIASCMDFORMAT              "<i>alias</i> [[<b>\"</b>]<i>command</i>[<b>;</b> <i>command</i> ...<b>\"</b>]]"
#define BINDCMDFORMAT               "<i>control</i> [<b>+</b><i>action</i>|[<b>\"</b>]<i>command</i>[<b>;</b> <i>command</i> ...<b>\"</b>]]"
#define EXECCMDFORMAT               "<i>filename</i><b>.cfg</b>"
#define GIVECMDFORMAT               "<b>ammo</b>|<b>armor</b>|<b>health</b>|<b>keys</b>|<b>weapons</b>|<b>all</b>|<i>item</i>"
#define IFCMDFORMAT                 "<i>CVAR</i> <i>value</i> <b>then</b> [<b>\"</b>]<i>command</i>[<b>;</b> <i>command</i> ...<b>\"</b>]"
#define KILLCMDFORMAT               "<b>player</b>|<b>all</b>|<i>monster</i>|<b>barrels</b>|<b>missiles</b>"
#define LOADCMDFORMAT               "<i>filename</i><b>.save</b>"
#define MAPCMDFORMAT1               "<b>E</b><i>x</i><b>M</b><i>y</i>[<b>B</b>]|<i>title</i>|<b>first</b>|<b>previous</b>|<b>next</b>|<b>last</b>|<b>random</b>"
#define MAPCMDFORMAT2               "<b>MAP</b><i>xy</i>|<i>title</i>|<b>first</b>|<b>previous</b>|<b>next</b>|<b>last</b>|<b>random</b>"
#define PLAYCMDFORMAT               "<i>soundeffect</i>|<i>music</i>"
#define NAMECMDFORMAT               "[<b>friendly</b> ]<i>monster</i> <i>name</i>"
#define PRINTCMDFORMAT              "<b>\"</b><i>message</i><b>\"</b>"
#define RESETCMDFORMAT              "<i>CVAR</i>"
#define RESURRECTCMDFORMAT          "<b>player</b>|<b>all</b>|<i>monster</i>"
#define SAVECMDFORMAT               "<i>filename</i><b>.save</b>"
#define SPAWNCMDFORMAT              "<i>item</i>|[<b>friendly</b> ]<i>monster</i>"
#define TAKECMDFORMAT               "<b>ammo</b>|<b>armor</b>|<b>health</b>|<b>keys</b>|<b>weapons</b>|<b>all</b>|<i>item</i>"
#define TELEPORTCMDFORMAT           "<i>x</i> <i>y</i>[ <i>z</i>]"
#define TIMERCMDFORMAT              "<i>minutes</i>"
#define UNBINDCMDFORMAT             "<i>control</i>|<b>+</b><i>action</i>"

#define PENDINGCHANGE               "This change won't be effective until the next map."

#define INTEGERCVARWITHDEFAULT      "It is <b>%s</b> and is <b>%s</b> by default."
#define INTEGERCVARWITHNODEFAULT    "It is <b>%s</b>."
#define INTEGERCVARISDEFAULT        "It is its default of <b>%s</b>."
#define INTEGERCVARISREADONLY       "It is <b>%s</b> and is read-only."
#define PERCENTCVARWITHDEFAULT      "It is <b>%s%%</b> and is <b>%s%%</b> by default."
#define PERCENTCVARWITHNODEFAULT    "It is <b>%s%%</b>."
#define PERCENTCVARISDEFAULT        "It is its default of <b>%s%%</b>."
#define PERCENTCVARISREADONLY       "It is <b>%s%%</b> and is read-only."
#define STRINGCVARWITHDEFAULT       "It is <b>\"%s\"</b> and is <b>\"%s\"</b> by default."
#define STRINGCVARISDEFAULT         "It is its default of <b>\"%s\"</b>."
#define STRINGCVARISREADONLY        "It is <b>%s%s%s</b> and is read-only."
#define TIMECVARISREADONLY          "It is <b>%02i:%02i:%02i</b> and is read-only."

#define UNITSPERFOOT                16
#define FEETPERMETER                3.28084f
#define METERSPERKILOMETER          1000.0f
#define FEETPERMILE                 5280

alias_t             aliases[MAXALIASES];

static int          ammo;
static int          armor;
static int          armortype;
static int          health;

static int          mapcmdepisode;
static int          mapcmdmap;
static char         mapcmdlump[7];

dboolean            executingalias = false;
dboolean            healthcvar = false;
dboolean            quitcmd = false;
dboolean            resettingcvar = false;
dboolean            togglingvanilla = false;
dboolean            vanilla = false;

char                *version = version_default;

const control_t controls[] =
{
    { "1",             keyboardcontrol, '1'                    }, { "2",             keyboardcontrol, '2'                    },
    { "3",             keyboardcontrol, '3'                    }, { "4",             keyboardcontrol, '4'                    },
    { "5",             keyboardcontrol, '5'                    }, { "6",             keyboardcontrol, '6'                    },
    { "7",             keyboardcontrol, '7'                    }, { "8",             keyboardcontrol, '8'                    },
    { "9",             keyboardcontrol, '9'                    }, { "0",             keyboardcontrol, '0'                    },
    { "-",             keyboardcontrol, KEY_MINUS              }, { "=",             keyboardcontrol, KEY_EQUALS             },
    { "+",             keyboardcontrol, KEY_EQUALS             }, { "backspace",     keyboardcontrol, KEY_BACKSPACE          },
    { "tab",           keyboardcontrol, KEY_TAB                }, { "q",             keyboardcontrol, 'q'                    },
    { "w",             keyboardcontrol, 'w'                    }, { "e",             keyboardcontrol, 'e'                    },
    { "r",             keyboardcontrol, 'r'                    }, { "t",             keyboardcontrol, 't'                    },
    { "y",             keyboardcontrol, 'y'                    }, { "u",             keyboardcontrol, 'u'                    },
    { "i",             keyboardcontrol, 'i'                    }, { "o",             keyboardcontrol, 'o'                    },
    { "p",             keyboardcontrol, 'p'                    }, { "[",             keyboardcontrol, '['                    },
    { "]",             keyboardcontrol, ']'                    }, { "enter",         keyboardcontrol, KEY_ENTER              },
    { "ctrl",          keyboardcontrol, KEY_CTRL               }, { "a",             keyboardcontrol, 'a'                    },
    { "s",             keyboardcontrol, 's'                    }, { "d",             keyboardcontrol, 'd'                    },
    { "f",             keyboardcontrol, 'f'                    }, { "g",             keyboardcontrol, 'g'                    },
    { "h",             keyboardcontrol, 'h'                    }, { "j",             keyboardcontrol, 'j'                    },
    { "k",             keyboardcontrol, 'k'                    }, { "l",             keyboardcontrol, 'l'                    },
    { ";",             keyboardcontrol, ';'                    }, { "'",             keyboardcontrol, '\''                   },
    { "shift",         keyboardcontrol, KEY_SHIFT              }, { "\\",            keyboardcontrol, '\\'                   },
    { "z",             keyboardcontrol, 'z'                    }, { "x",             keyboardcontrol, 'x'                    },
    { "c",             keyboardcontrol, 'c'                    }, { "v",             keyboardcontrol, 'v'                    },
    { "b",             keyboardcontrol, 'b'                    }, { "n",             keyboardcontrol, 'n'                    },
    { "m",             keyboardcontrol, 'm'                    }, { ",",             keyboardcontrol, ','                    },
    { ".",             keyboardcontrol, '.'                    }, { "/",             keyboardcontrol, '/'                    },
    { "tilde",         keyboardcontrol, '`'                    }, { "alt",           keyboardcontrol, KEY_ALT                },
    { "space",         keyboardcontrol, ' '                    }, { "numlock",       keyboardcontrol, KEY_NUMLOCK            },
    { "capslock",      keyboardcontrol, KEY_CAPSLOCK           }, { "scrolllock",    keyboardcontrol, KEY_SCROLLLOCK         },
    { "home",          keyboardcontrol, KEY_HOME               }, { "up",            keyboardcontrol, KEY_UPARROW            },
    { "pageup",        keyboardcontrol, KEY_PAGEUP             }, { "left",          keyboardcontrol, KEY_LEFTARROW          },
    { "right",         keyboardcontrol, KEY_RIGHTARROW         }, { "end",           keyboardcontrol, KEY_END                },
    { "down",          keyboardcontrol, KEY_DOWNARROW          }, { "pagedown",      keyboardcontrol, KEY_PAGEDOWN           },
    { "insert",        keyboardcontrol, KEY_INSERT             }, { "printscreen",   keyboardcontrol, KEY_PRINTSCREEN        },
    { "delete",        keyboardcontrol, KEY_DELETE             }, { "escape",        keyboardcontrol, KEY_ESCAPE             },
    { "F12",           keyboardcontrol, KEY_F12                }, { "mouse1",        mousecontrol,    0                      },
    { "mouse2",        mousecontrol,    1                      }, { "mouse3",        mousecontrol,    2                      },
    { "mouse4",        mousecontrol,    3                      }, { "mouse5",        mousecontrol,    4                      },
    { "mouse6",        mousecontrol,    5                      }, { "mouse7",        mousecontrol,    6                      },
    { "mouse8",        mousecontrol,    7                      }, { "wheelup",       mousecontrol,    MOUSE_WHEELUP          },
    { "wheeldown",     mousecontrol,    MOUSE_WHEELDOWN        }, { "dpadup",        gamepadcontrol,  GAMEPAD_DPAD_UP        },
    { "dpaddown",      gamepadcontrol,  GAMEPAD_DPAD_DOWN      }, { "dpadleft",      gamepadcontrol,  GAMEPAD_DPAD_LEFT      },
    { "dpadright",     gamepadcontrol,  GAMEPAD_DPAD_RIGHT     }, { "start",         gamepadcontrol,  GAMEPAD_START          },
    { "back",          gamepadcontrol,  GAMEPAD_BACK           }, { "leftthumb",     gamepadcontrol,  GAMEPAD_LEFT_THUMB     },
    { "rightthumb",    gamepadcontrol,  GAMEPAD_RIGHT_THUMB    }, { "leftshoulder",  gamepadcontrol,  GAMEPAD_LEFT_SHOULDER  },
    { "rightshoulder", gamepadcontrol,  GAMEPAD_RIGHT_SHOULDER }, { "lefttrigger",   gamepadcontrol,  GAMEPAD_LEFT_TRIGGER   },
    { "righttrigger",  gamepadcontrol,  GAMEPAD_RIGHT_TRIGGER  }, { "gamepad1",      gamepadcontrol,  GAMEPAD_A              },
    { "gamepad2",      gamepadcontrol,  GAMEPAD_B              }, { "gamepad3",      gamepadcontrol,  GAMEPAD_X              },
    { "gamepad4",      gamepadcontrol,  GAMEPAD_Y              }, { "guide",         gamepadcontrol,  GAMEPAD_GUIDE          },
    { "",              0,               0                      }
};

static void alwaysrun_action_func(void);
static void automap_action_func(void);
static void back_action_func(void);
static void clearmark_action_func(void);
static void console_action_func(void);
static void fire_action_func(void);
static void followmode_action_func(void);
static void forward_action_func(void);
static void grid_action_func(void);
static void jump_action_func(void);
static void left_action_func(void);
static void mark_action_func(void);
static void maxzoom_action_func(void);
static void menu_action_func(void);
static void nextweapon_action_func(void);
static void prevweapon_action_func(void);
static void right_action_func(void);
static void rotatemode_action_func(void);
static void screenshot_action_func(void);
static void strafeleft_action_func(void);
static void straferight_action_func(void);
static void use_action_func(void);
static void weapon1_action_func(void);
static void weapon2_action_func(void);
static void weapon3_action_func(void);
static void weapon4_action_func(void);
static void weapon5_action_func(void);
static void weapon6_action_func(void);
static void weapon7_action_func(void);

action_t actions[] =
{
    { "+alwaysrun",   true,  alwaysrun_action_func,   &keyboardalwaysrun,         NULL,                  NULL,             &gamepadalwaysrun,         NULL         },
    { "+automap",     false, automap_action_func,     &keyboardautomap,           NULL,                  NULL,             &gamepadautomap,           NULL         },
    { "+back",        true,  back_action_func,        &keyboardback,              &keyboardback2,        NULL,             &gamepadback,              NULL         },
    { "+clearmark",   true,  clearmark_action_func,   &keyboardautomapclearmark,  NULL,                  NULL,             &gamepadautomapclearmark,  NULL         },
    { "+console",     false, console_action_func,     &keyboardconsole,           NULL,                  NULL,             NULL,                      NULL         },
    { "+fire",        true,  fire_action_func,        &keyboardfire,              NULL,                  &mousefire,       &gamepadfire,              NULL         },
    { "+followmode",  true,  followmode_action_func,  &keyboardautomapfollowmode, NULL,                  NULL,             &gamepadautomapfollowmode, NULL         },
    { "+forward",     true,  forward_action_func,     &keyboardforward,           &keyboardforward2,     &mouseforward,    &gamepadforward,           NULL         },
    { "+grid",        true,  grid_action_func,        &keyboardautomapgrid,       NULL,                  NULL,             &gamepadautomapgrid,       NULL         },
    { "+jump",        true,  jump_action_func,        &keyboardjump,              NULL,                  &mousejump,       &gamepadjump,              NULL         },
    { "+left",        true,  left_action_func,        &keyboardleft,              NULL,                  NULL,             &gamepadleft,              NULL         },
    { "+mark",        true,  mark_action_func,        &keyboardautomapmark,       NULL,                  NULL,             &gamepadautomapmark,       NULL         },
    { "+maxzoom",     true,  maxzoom_action_func,     &keyboardautomapmaxzoom,    NULL,                  NULL,             &gamepadautomapmaxzoom,    NULL         },
    { "+menu",        true,  menu_action_func,        &keyboardmenu,              NULL,                  NULL,             &gamepadmenu,              NULL         },
    { "+mouselook",   true,  NULL,                    &keyboardmouselook,         NULL,                  &mousemouselook,  &gamepadmouselook,         NULL         },
    { "+nextweapon",  true,  nextweapon_action_func,  &keyboardnextweapon,        NULL,                  &mousenextweapon, &gamepadnextweapon,        NULL         },
    { "+prevweapon",  true,  prevweapon_action_func,  &keyboardprevweapon,        NULL,                  &mouseprevweapon, &gamepadprevweapon,        NULL         },
    { "+right",       true,  right_action_func,       &keyboardright,             NULL,                  NULL,             &gamepadright,             NULL         },
    { "+rotatemode",  true,  rotatemode_action_func,  &keyboardautomaprotatemode, NULL,                  NULL,             &gamepadautomaprotatemode, NULL         },
    { "+run",         true,  NULL,                    &keyboardrun,               NULL,                  &mouserun,        &gamepadrun,               NULL         },
    { "+screenshot",  false, screenshot_action_func,  &keyboardscreenshot,        NULL,                  &mousescreenshot, NULL,                      NULL         },
    { "+strafe",      true,  NULL,                    &keyboardstrafe,            NULL,                  &mousestrafe,     &gamepadstrafe,            NULL         },
    { "+strafeleft",  true,  strafeleft_action_func,  &keyboardstrafeleft,        &keyboardstrafeleft2,  NULL,             &gamepadstrafeleft,        NULL         },
    { "+straferight", true,  straferight_action_func, &keyboardstraferight,       &keyboardstraferight2, NULL,             &gamepadstraferight,       NULL         },
    { "+use",         true,  use_action_func,         &keyboarduse,               &keyboarduse2,         &mouseuse,        &gamepaduse,               &gamepaduse2 },
    { "+weapon1",     true,  weapon1_action_func,     &keyboardweapon1,           NULL,                  NULL,             &gamepadweapon1,           NULL         },
    { "+weapon2",     true,  weapon2_action_func,     &keyboardweapon2,           NULL,                  NULL,             &gamepadweapon2,           NULL         },
    { "+weapon3",     true,  weapon3_action_func,     &keyboardweapon3,           NULL,                  NULL,             &gamepadweapon3,           NULL         },
    { "+weapon4",     true,  weapon4_action_func,     &keyboardweapon4,           NULL,                  NULL,             &gamepadweapon4,           NULL         },
    { "+weapon5",     true,  weapon5_action_func,     &keyboardweapon5,           NULL,                  NULL,             &gamepadweapon5,           NULL         },
    { "+weapon6",     true,  weapon6_action_func,     &keyboardweapon6,           NULL,                  NULL,             &gamepadweapon6,           NULL         },
    { "+weapon7",     true,  weapon7_action_func,     &keyboardweapon7,           NULL,                  NULL,             &gamepadweapon7,           NULL         },
    { "+zoomin",      true,  NULL,                    &keyboardautomapzoomin,     NULL,                  NULL,             &gamepadautomapzoomin,     NULL         },
    { "+zoomout",     true,  NULL,                    &keyboardautomapzoomout,    NULL,                  NULL,             &gamepadautomapzoomout,    NULL         },
    { "",             false, NULL,                    NULL,                       NULL,                  NULL,             NULL,                      NULL         }
};

static dboolean alive_func1(char *cmd, char *parms);
static dboolean cheat_func1(char *cmd, char *parms);
static dboolean game_func1(char *cmd, char *parms);
static dboolean null_func1(char *cmd, char *parms);

static void bindlist_cmd_func2(char *cmd, char *parms);
static void clear_cmd_func2(char *cmd, char *parms);
static void cmdlist_cmd_func2(char *cmd, char *parms);
static dboolean condump_cmd_func1(char *cmd, char *parms);
static void condump_cmd_func2(char *cmd, char *parms);
static void cvarlist_cmd_func2(char *cmd, char *parms);
static void endgame_cmd_func2(char *cmd, char *parms);
static void exec_cmd_func2(char *cmd, char *parms);
static void exitmap_cmd_func2(char *cmd, char *parms);
static dboolean fastmonsters_cmd_func1(char *cmd, char *parms);
static void fastmonsters_cmd_func2(char *cmd, char *parms);
static void freeze_cmd_func2(char *cmd, char *parms);
static dboolean give_cmd_func1(char *cmd, char *parms);
static void give_cmd_func2(char *cmd, char *parms);
static void god_cmd_func2(char *cmd, char *parms);
static void help_cmd_func2(char *cmd, char *parms);
static void if_cmd_func2(char *cmd, char *parms);
static dboolean kill_cmd_func1(char *cmd, char *parms);
static void kill_cmd_func2(char *cmd, char *parms);
static void license_cmd_func2(char *cmd, char *parms);
static void load_cmd_func2(char *cmd, char *parms);
static dboolean map_cmd_func1(char *cmd, char *parms);
static void map_cmd_func2(char *cmd, char *parms);
static void maplist_cmd_func2(char *cmd, char *parms);
static void mapstats_cmd_func2(char *cmd, char *parms);
static dboolean name_cmd_func1(char *cmd, char *parms);
static void name_cmd_func2(char *cmd, char *parms);
static void newgame_cmd_func2(char *cmd, char *parms);
static void noclip_cmd_func2(char *cmd, char *parms);
static void nomonsters_cmd_func2(char *cmd, char *parms);
static void notarget_cmd_func2(char *cmd, char *parms);
static void pistolstart_cmd_func2(char *cmd, char *parms);
static dboolean play_cmd_func1(char *cmd, char *parms);
static void play_cmd_func2(char *cmd, char *parms);
static void playerstats_cmd_func2(char *cmd, char *parms);
static void print_cmd_func2(char *cmd, char *parms);
static void quit_cmd_func2(char *cmd, char *parms);
static void regenhealth_cmd_func2(char *cmd, char *parms);
static void reset_cmd_func2(char *cmd, char *parms);
static void resetall_cmd_func2(char *cmd, char *parms);
static void respawnitems_cmd_func2(char *cmd, char *parms);
static dboolean respawnmonsters_cmd_func1(char *cmd, char *parms);
static void respawnmonsters_cmd_func2(char *cmd, char *parms);
static void restartmap_cmd_func2(char *cmd, char *parms);
static dboolean resurrect_cmd_func1(char *cmd, char *parms);
static void resurrect_cmd_func2(char *cmd, char *parms);
static void save_cmd_func2(char *cmd, char *parms);
static dboolean spawn_cmd_func1(char *cmd, char *parms);
static void spawn_cmd_func2(char *cmd, char *parms);
static dboolean take_cmd_func1(char *cmd, char *parms);
static void take_cmd_func2(char *cmd, char *parms);
static dboolean teleport_cmd_func1(char *cmd, char *parms);
static void teleport_cmd_func2(char *cmd, char *parms);
static void thinglist_cmd_func2(char *cmd, char *parms);
static void timer_cmd_func2(char *cmd, char *parms);
static void unbind_cmd_func2(char *cmd, char *parms);
static void vanilla_cmd_func2(char *cmd, char *parms);

static dboolean bool_cvars_func1(char *cmd, char *parms);
static void bool_cvars_func2(char *cmd, char *parms);
static dboolean color_cvars_func1(char *cmd, char *parms);
static void color_cvars_func2(char *cmd, char *parms);
static dboolean float_cvars_func1(char *cmd, char *parms);
static dboolean int_cvars_func1(char *cmd, char *parms);
static void int_cvars_func2(char *cmd, char *parms);
static void str_cvars_func2(char *cmd, char *parms);
static void time_cvars_func2(char *cmd, char *parms);

static void alwaysrun_cvar_func2(char *cmd, char *parms);
static void am_external_cvar_func2(char *cmd, char *parms);
static dboolean am_followmode_cvar_func1(char *cmd, char *parms);
static void am_gridsize_cvar_func2(char *cmd, char *parms);
static dboolean armortype_cvar_func1(char *cmd, char *parms);
static void armortype_cvar_func2(char *cmd, char *parms);
static void autotilt_cvar_func2(char *cmd, char *parms);
static dboolean crosshair_cvar_func1(char *cmd, char *parms);
static void crosshair_cvar_func2(char *cmd, char *parms);
static void episode_cvar_func2(char *cmd, char *parms);
static void expansion_cvar_func2(char *cmd, char *parms);
static dboolean gp_deadzone_cvars_func1(char *cmd, char *parms);
static void gp_deadzone_cvars_func2(char *cmd, char *parms);
static void gp_sensitivity_cvars_func2(char *cmd, char *parms);
static void mouselook_cvar_func2(char *cmd, char *parms);
static dboolean player_cvars_func1(char *cmd, char *parms);
static void player_cvars_func2(char *cmd, char *parms);
static void playername_cvar_func2(char *cmd, char *parms);
static dboolean r_blood_cvar_func1(char *cmd, char *parms);
static void r_blood_cvar_func2(char *cmd, char *parms);
static void r_bloodsplats_translucency_cvar_func2(char *cmd, char *parms);
static void r_color_cvar_func2(char *cmd, char *parms);
static dboolean r_detail_cvar_func1(char *cmd, char *parms);
static void r_detail_cvar_func2(char *cmd, char *parms);
static void r_dither_cvar_func2(char *cmd, char *parms);
static void r_fixmaperrors_cvar_func2(char *cmd, char *parms);
static void r_fov_cvar_func2(char *cmd, char *parms);
static dboolean r_gamma_cvar_func1(char *cmd, char *parms);
static void r_gamma_cvar_func2(char *cmd, char *parms);
static void r_hud_cvar_func2(char *cmd, char *parms);
static void r_hud_translucency_cvar_func2(char *cmd, char *parms);
static void r_lowpixelsize_cvar_func2(char *cmd, char *parms);
static void r_screensize_cvar_func2(char *cmd, char *parms);
static void r_shadows_translucency_cvar_func2(char *cmd, char *parms);
static dboolean r_skycolor_cvar_func1(char *cmd, char *parms);
static void r_skycolor_cvar_func2(char *cmd, char *parms);
static void r_textures_cvar_func2(char *cmd, char *parms);
static void r_translucency_cvar_func2(char *cmd, char *parms);
static dboolean s_volume_cvars_func1(char *cmd, char *parms);
static void s_volume_cvars_func2(char *cmd, char *parms);
static void savegame_cvar_func2(char *cmd, char *parms);
static void skilllevel_cvar_func2(char *cmd, char *parms);
static dboolean turbo_cvar_func1(char *cmd, char *parms);
static void turbo_cvar_func2(char *cmd, char *parms);
static dboolean units_cvar_func1(char *cmd, char *parms);
static void units_cvar_func2(char *cmd, char *parms);
static void vid_borderlesswindow_cvar_func2(char *cmd, char *parms);
static dboolean vid_capfps_cvar_func1(char *cmd, char *parms);
static void vid_capfps_cvar_func2(char *cmd, char *parms);
static void vid_display_cvar_func2(char *cmd, char *parms);
static void vid_fullscreen_cvar_func2(char *cmd, char *parms);
static void vid_pillarboxes_cvar_func2(char *cmd, char *parms);
static dboolean vid_scaleapi_cvar_func1(char *cmd, char *parms);
static void vid_scaleapi_cvar_func2(char *cmd, char *parms);
static dboolean vid_scalefilter_cvar_func1(char *cmd, char *parms);
static void vid_scalefilter_cvar_func2(char *cmd, char *parms);
static void vid_screenresolution_cvar_func2(char *cmd, char *parms);
static void vid_showfps_cvar_func2(char *cmd, char *parms);
static dboolean vid_vsync_cvar_func1(char *cmd, char *parms);
static void vid_vsync_cvar_func2(char *cmd, char *parms);
static void vid_widescreen_cvar_func2(char *cmd, char *parms);
static void vid_windowpos_cvar_func2(char *cmd, char *parms);
static void vid_windowsize_cvar_func2(char *cmd, char *parms);

static int C_LookupValueFromAlias(const char *text, const valuealias_type_t valuealiastype)
{
    for (int i = 0; *valuealiases[i].text; i++)
        if (valuealiastype == valuealiases[i].type && M_StringCompare(text, valuealiases[i].text))
            return valuealiases[i].value;

    return INT_MIN;
}

static char *C_LookupAliasFromValue(const int value, const valuealias_type_t valuealiastype)
{
    for (int i = 0; *valuealiases[i].text; i++)
        if (valuealiastype == valuealiases[i].type && value == valuealiases[i].value)
            return M_StringDuplicate(valuealiases[i].text);

    return commify(value);
}

#define CCMD(name, alt, cond, func, parms, form, desc) \
    { #name, #alt, cond, func, parms, CT_CCMD, CF_NONE, NULL, 0, 0, 0, form, desc, 0, 0 }
#define CMD_CHEAT(name, parms) \
    { #name, "", cheat_func1, NULL, parms, CT_CHEAT, CF_NONE, NULL, 0, 0, 0, "", "", 0, 0 }
#define CVAR_BOOL(name, alt, cond, func, aliases, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, CF_BOOLEAN, &name, aliases, false, true, "", desc, name##_default, 0 }
#define CVAR_INT(name, alt, cond, func, flags, aliases, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, (CF_INTEGER | flags), &name, aliases, name##_min, name##_max, "", desc, name##_default, 0 }
#define CVAR_FLOAT(name, alt, cond, func, flags, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, (CF_FLOAT | flags), &name, 0, 0, 0, "", desc, name##_default, 0 }
#define CVAR_STR(name, alt, cond, func, flags, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, (CF_STRING | flags), &name, 0, 0, 0, "", desc, 0, name##_default }
#define CVAR_TIME(name, alt, cond, func, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, (CF_TIME | CF_READONLY), &name, 0, 0, 0, "", desc, 0, "" }
#define CVAR_OTHER(name, alt, cond, func, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, CF_OTHER, &name, 0, 0, 0, "", desc, 0, name##_default }

consolecmd_t consolecmds[] =
{
    CCMD(alias, "", null_func1, alias_cmd_func2, true, ALIASCMDFORMAT,
        "Creates an <i>alias</i> that executes a string of\n<i>commands</i>."),
    CVAR_BOOL(alwaysrun, "", bool_cvars_func1, alwaysrun_cvar_func2, BOOLVALUEALIAS,
        "Toggles the player to always run when they move."),
    CVAR_INT(am_allmapcdwallcolor, am_allmapcdwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped lines in the automap that\nindicate a change in ceiling height while the player\n"
        "has a computer area map power-up (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_allmapfdwallcolor, am_allmapfdwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped lines in the automap that\nindicate a change in floor height while the player\n"
        "has a computer area map power-up (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_allmapwallcolor, am_allmapwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped solid walls in the automap\nwhile the player has a computer area map power-up\n"
        "(<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_backcolor, am_backcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the automap's background (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_cdwallcolor, am_cdwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap that indicate a\nchange in ceiling height (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_crosshaircolor, am_crosshaircolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the crosshair in the automap (<b>0</b> to\n<b>255</b>)."),
    CVAR_BOOL(am_external, "", bool_cvars_func1, am_external_cvar_func2, BOOLVALUEALIAS,
        "Toggles showing the automap on an external\ndisplay."),
    CVAR_INT(am_fdwallcolor, am_fdwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap that indicate a\nchange in floor height (<b>0</b> to <b>255</b>)."),
    CVAR_BOOL(am_followmode, "", am_followmode_cvar_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles follow mode in the automap."),
    CVAR_BOOL(am_grid, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the grid in the automap."),
    CVAR_INT(am_gridcolor, am_gridcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the grid in the automap (<b>0</b> to <b>255</b>)."),
    CVAR_OTHER(am_gridsize, "", null_func1, am_gridsize_cvar_func2,
        "The size of the grid in the automap (<i>width</i><b>\xD7</b><i>height</i>)."),
    CVAR_INT(am_markcolor, am_markcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of marks in the automap (<b>0</b> to <b>255</b>)."),
    CVAR_BOOL(am_path, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the player's path in the automap."),
    CVAR_INT(am_pathcolor, am_pathcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the player's path in the automap (<b>0</b> to\n<b>255</b>)."),
    CVAR_INT(am_playercolor, am_playercolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the player in the automap (<b>0</b> to <b>255</b>)."),
    CVAR_BOOL(am_rotatemode, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles rotate mode in the automap."),
    CVAR_INT(am_teleportercolor, am_teleportercolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of teleporters in the automap (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_thingcolor, am_thingcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of things in the automap (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_tswallcolor, am_tswallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap with no change in\nheight (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_wallcolor, am_wallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of solid walls in the automap (<b>0</b> to <b>255</b>)."),
    CVAR_INT(ammo, "", player_cvars_func1, player_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The amount of ammo for the player's currently\nequipped weapon."),
    CVAR_INT(armor, armour, player_cvars_func1, player_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The player's armor (<b>0%</b> to <b>200%</b>)."),
    CVAR_INT(armortype, armourtype, armortype_cvar_func1, armortype_cvar_func2, CF_NONE, ARMORTYPEVALUEALIAS,
        "The player's armor type (<b>none</b>, <b>green</b> or <b>blue</b>)."),
    CVAR_BOOL(autoaim, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles vertical autoaiming as the player fires\ntheir weapon while using mouselook."),
    CVAR_BOOL(autoload, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles automatically loading the last savegame\nafter the player dies."),
    CVAR_BOOL(autosave, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles automatically saving the game at the start\nof each map."),
    CVAR_BOOL(autotilt, "", bool_cvars_func1, autotilt_cvar_func2, BOOLVALUEALIAS,
        "Toggles automatically tilting the player's view\nwhile going up or down flights of stairs."),
    CVAR_BOOL(autouse, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles automatically using doors and switches in\nfront of the player"),
    CCMD(bind, "", null_func1, bind_cmd_func2, true, BINDCMDFORMAT,
        "Binds an <i>action</i> or string of <i>commands</i> to a\n<i>control</i>."),
    CCMD(bindlist, "", null_func1, bindlist_cmd_func2, false, "",
        "Lists all bound controls."),
    CVAR_BOOL(centerweapon, centreweapon, bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles centering the player's weapon when firing."),
    CCMD(clear, "", null_func1, clear_cmd_func2, false, "",
        "Clears the console."),
    CCMD(cmdlist, ccmdlist, null_func1, cmdlist_cmd_func2, true, "[<i>searchstring</i>]",
        "Lists all console commands."),
    CVAR_INT(con_backcolor, con_backcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the console's background (<b>0</b> to <b>255</b>)."),
    CVAR_INT(con_edgecolor, con_edgecolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the console's bottom edge (<b>0</b> to <b>255</b>)."),
    CVAR_BOOL(con_obituaries, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles obituaries in the console when the player\nor monsters are killed."),
    CCMD(condump, "", condump_cmd_func1, condump_cmd_func2, true, "[<i>filename</i><b>.txt</b>]",
        "Dumps the contents of the console to a file."),
    CVAR_INT(crosshair, "", crosshair_cvar_func1, crosshair_cvar_func2, CF_NONE, CROSSHAIRVALUEALIAS,
        "Toggles a crosshair (<b>none</b>, <b>cross</b> or <b>dot</b>)."),
    CVAR_INT(crosshaircolor, crosshaircolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the crosshair (<b>0</b> to <b>255</b>)."),
    CCMD(cvarlist, "", null_func1, cvarlist_cmd_func2, true, "[<i>searchstring</i>]",
        "Lists all console variables."),
    CCMD(endgame, "", game_func1, endgame_cmd_func2, false, "",
        "Ends a game."),
    CVAR_INT(episode, "", int_cvars_func1, episode_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected <i><b>DOOM</b></i> episode in the menu\n(<b>1</b> to <b>5</b>)."),
    CCMD(exec, "", null_func1, exec_cmd_func2, true, EXECCMDFORMAT,
        "Executes a series of commands stored in a\nfile."),
    CCMD(exitmap, "", game_func1, exitmap_cmd_func2, false, "",
        "Exits the current map."),
    CVAR_INT(expansion, "", int_cvars_func1, expansion_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected <i><b>DOOM II</b></i> expansion in the\nmenu (<b>1</b> or <b>2</b>)."),
    CVAR_INT(facebackcolor, facebackcolour, color_cvars_func1, color_cvars_func2, CF_NONE, FACEBACKVALUEALIAS,
        "The color behind the player's face in the status bar\n(<b>none</b>, <b>0</b> to <b>255</b>)."),
    CVAR_BOOL(fade, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles a fading effect when transitioning between\nsome screens."),
        CCMD(fastmonsters, "", fastmonsters_cmd_func1, fastmonsters_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles fast monsters."),
    CCMD(freeze, "", alive_func1, freeze_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles freeze mode."),
    CVAR_TIME(gametime, "", null_func1, time_cvars_func2,
        "The amount of time <i><b>" PACKAGE_NAME "</b></i> has been running."),
    CCMD(give, "", give_cmd_func1, give_cmd_func2, true, GIVECMDFORMAT,
        "Gives <b>ammo</b>, <b>armor</b>, <b>health</b>, <b>keys</b>, <b>weapons</b>, or\n"
        "<b>all</b> or certain <i>items</i> to the player."),
    CCMD(god, "", alive_func1, god_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles god mode."),
    CVAR_BOOL(gp_analog, gp_analogue, bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles whether movement using the gamepad's\nthumbsticks is analog or digital."),
    CVAR_FLOAT(gp_deadzone_left, "", gp_deadzone_cvars_func1, gp_deadzone_cvars_func2, CF_PERCENT,
        "The dead zone of the gamepad's left thumbstick\n(<b>0%</b> to <b>100%</b>)."),
    CVAR_FLOAT(gp_deadzone_right, "", gp_deadzone_cvars_func1, gp_deadzone_cvars_func2, CF_PERCENT,
        "The dead zone of the gamepad's right thumbstick\n(<b>0%</b> to <b>100%</b>)."),
    CVAR_BOOL(gp_invertyaxis, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles inverting the vertical axis of the\ngamepad's right thumbstick when looking up or\ndown."),
    CVAR_INT(gp_sensitivity_horizontal, "", int_cvars_func1, gp_sensitivity_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The horizontal sensitivity of the gamepad's\nthumbsticks (<b>0</b> to <b>128</b>)."),
    CVAR_INT(gp_sensitivity_vertical, "", int_cvars_func1, gp_sensitivity_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The vertical sensitivity of the gamepad's\nthumbsticks (<b>0</b> to <b>128</b>)."),
    CVAR_BOOL(gp_swapthumbsticks, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles swapping the gamepad's left and right\nthumbsticks."),
    CVAR_INT(gp_thumbsticks, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The number of thumbsticks used on the gamepad\n(<b>1</b> or <b>2</b>)."),
    CVAR_INT(gp_vibrate_barrels, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount <i><b>XInput</b></i> gamepads vibrate when the\nplayer is near an exploding barrel (<b>0%</b> to <b>200%</b>)."),
    CVAR_INT(gp_vibrate_damage, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount <i><b>XInput</b></i> gamepads vibrate when the\nplayer receives damage (<b>0%</b> to <b>200%</b>)."),
    CVAR_INT(gp_vibrate_weapons, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount <i><b>XInput</b></i> gamepads vibrate when the\nplayer fires their weapon (<b>0%</b> to <b>200%</b>)."),
    CVAR_INT(health, "", player_cvars_func1, player_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The player's health (<b>0%</b> to <b>200%</b>)."),
    CCMD(help, "", null_func1, help_cmd_func2, false, "",
        "Opens the <i><b>" PACKAGE_NAME " Wiki.</b></i>"),
    CMD_CHEAT(idbehold, false),
    CMD_CHEAT(idbeholda, false),
    CMD_CHEAT(idbeholdi, false),
    CMD_CHEAT(idbeholdl, false),
    CMD_CHEAT(idbeholdr, false),
    CMD_CHEAT(idbeholds, false),
    CMD_CHEAT(idbeholdv, false),
    CMD_CHEAT(idchoppers, false),
    CMD_CHEAT(idclev, true),
    CMD_CHEAT(idclip, false),
    CMD_CHEAT(iddqd, false),
    CMD_CHEAT(iddt, false),
    CMD_CHEAT(idfa, false),
    CMD_CHEAT(idkfa, false),
    CMD_CHEAT(idmus, true),
    CMD_CHEAT(idmypos, false),
    CMD_CHEAT(idspispopd, false),
    CCMD(if, "", null_func1, if_cmd_func2, true, IFCMDFORMAT,
        "Executes a string of <i>commands</i> if a <i>CVAR</i>\nequals a <i>value</i>."),
    CVAR_BOOL(infighting, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles infighting among monsters once the player\ndies."),
    CVAR_BOOL(infiniteheight, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles giving the player and monsters infinite\nheight."),
    CVAR_STR(iwadfolder, "", null_func1, str_cvars_func2, CF_NONE,
        "The folder where an IWAD was last opened."),
    CCMD(kill, explode, kill_cmd_func1, kill_cmd_func2, true, KILLCMDFORMAT,
        "Kills the <b>player</b>, <b>all</b> monsters, a type of\n<i>monster</i>, or explodes all <b>barrels</b> or <b>missiles</b>."),
    CCMD(license, "", null_func1, license_cmd_func2, false, "",
        "Displays the <i><b>" PACKAGE_LICENSE ".</b></i>"),
    CCMD(load, "", null_func1, load_cmd_func2, true, LOADCMDFORMAT,
        "Loads a game from a file."),
    CVAR_BOOL(m_acceleration, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the acceleration of mouse movement."),
    CVAR_BOOL(m_doubleclick_use, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles double-clicking a mouse button for the\n<b>+use</b> action."),
    CVAR_BOOL(m_invertyaxis, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles inverting the mouse's vertical axis when\nusing mouselook."),
    CVAR_BOOL(m_novertical, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles no vertical movement of the mouse."),
    CVAR_INT(m_sensitivity, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The mouse's sensitivity (<b>0</b> to <b>128</b>)."),
    CCMD(map, warp, map_cmd_func1, map_cmd_func2, true, MAPCMDFORMAT1,
        "Warps the player to another map."),
    CCMD(maplist, "", null_func1, maplist_cmd_func2, false, "",
        "Lists all maps in the currently loaded WADs."),
    CCMD(mapstats, "", game_func1, mapstats_cmd_func2, false, "",
        "Shows stats about the current map."),
    CVAR_BOOL(melt, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles a melting effect when transitioning\nbetween some screens."),
    CVAR_BOOL(messages, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles player messages."),
    CVAR_BOOL(mouselook, "", bool_cvars_func1, mouselook_cvar_func2, BOOLVALUEALIAS,
        "Toggles mouselook."),
    CVAR_INT(movebob, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's view bobs up and down\nwhen they move (<b>0%</b> to <b>100%</b>)."),
    CMD_CHEAT(mumu, false),
    CCMD(name, "", name_cmd_func1, name_cmd_func2, true, NAMECMDFORMAT,
        "Gives a <i>name</i> to the <i>monster</i> nearest to the\nplayer."),
    CCMD(newgame, "", null_func1, newgame_cmd_func2, true, "",
        "Starts a new game."),
    CCMD(noclip, "", game_func1, noclip_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles no clipping mode."),
    CCMD(nomonsters, "", null_func1, nomonsters_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles the presence of monsters in maps."),
    CCMD(notarget, "", game_func1, notarget_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles monsters not seeing the player as a\ntarget."),
    CCMD(pistolstart, "", null_func1, pistolstart_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles the player starting each map with only\na pistol."),
    CCMD(play, "", play_cmd_func1, play_cmd_func2, true, PLAYCMDFORMAT,
        "Plays a <i>sound effect</i> or <i>music</i> lump."),
    CVAR_STR(playername, "", null_func1, playername_cvar_func2, CF_NONE,
        "The name of the player used in player messages."),
    CCMD(playerstats, "", null_func1, playerstats_cmd_func2, false, "",
        "Shows stats about the player."),
    CCMD(print, "", null_func1, print_cmd_func2, true, PRINTCMDFORMAT,
        "Prints a player \"<i>message</i>\"."),
    CCMD(quit, exit, null_func1, quit_cmd_func2, false, "",
        "Quits <i><b>" PACKAGE_NAME ".</b></i>"),
    CVAR_BOOL(r_althud, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles an alternate heads-up display when in\nwidescreen mode."),
    CVAR_INT(r_berserkintensity, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The intensity of the effect when the player has a\nberserk power-up and their fists equipped (<b>0</b> to <b>8</b>)."),
    CVAR_INT(r_blood, "", r_blood_cvar_func1, r_blood_cvar_func2, CF_NONE, BLOODVALUEALIAS,
        "The colors of the blood spilled by the player and\n"
        "monsters (<b>all</b>, <b>none</b>, <b>red</b>, <b>green</b> or <b>nofuzz</b>)."),
    CVAR_INT(r_bloodsplats_max, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The maximum number of blood splats allowed in a\nmap (<b>0</b> to <b>1,048,576</b>)."),
    CVAR_INT(r_bloodsplats_total, "", int_cvars_func1, int_cvars_func2, CF_READONLY, NOVALUEALIAS,
        "The total number of blood splats in the current\nmap."),
    CVAR_BOOL(r_bloodsplats_translucency, "", bool_cvars_func1, r_bloodsplats_translucency_cvar_func2, BOOLVALUEALIAS,
        "Toggles the translucency of blood splats."),
    CVAR_BOOL(r_brightmaps, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles brightmaps on certain wall textures."),
    CVAR_INT(r_color, "", int_cvars_func1, r_color_cvar_func2, CF_PERCENT, NOVALUEALIAS,
        "The intensity of color on the screen (<b>0%</b> to <b>100%</b>)."),
    CVAR_BOOL(r_corpses_color, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles randomly colored marine corpses."),
    CVAR_BOOL(r_corpses_gib, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles some corpses gibbing in reaction to\nnearby barrel and rocket explosions."),
    CVAR_BOOL(r_corpses_mirrored, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles randomly mirrored corpses."),
    CVAR_BOOL(r_corpses_moreblood, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles blood splats around corpses spawned when\na map is loaded."),
    CVAR_BOOL(r_corpses_nudge, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles corpses being nudged when the player and\nmonsters walk over them."),
    CVAR_BOOL(r_corpses_slide, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles corpses sliding in reaction to nearby\nbarrel and rocket explosions."),
    CVAR_BOOL(r_corpses_smearblood, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles corpses leaving blood splats as they slide."),
    CVAR_BOOL(r_detail, "", r_detail_cvar_func1, r_detail_cvar_func2, DETAILVALUEALIAS,
        "Toggles the graphic detail (<b>high</b> or <b>low</b>)."),
    CVAR_BOOL(r_diskicon, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles showing a disk icon when loading and\nsaving."),
    CVAR_BOOL(r_dither, "", bool_cvars_func1, r_dither_cvar_func2, BOOLVALUEALIAS,
        "Toggles dithering of <i><b>BOOM</b></i>-compatible translucent\nwall textures."),
    CVAR_BOOL(r_fixmaperrors, "", bool_cvars_func1, r_fixmaperrors_cvar_func2, BOOLVALUEALIAS,
        "Toggles fixing the mapping errors in the <i><b>DOOM</b></i> and\n<i><b>DOOM II</b></i> IWADs."),
    CVAR_BOOL(r_fixspriteoffsets, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles fixing sprite offsets."),
    CVAR_BOOL(r_floatbob, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles some power-ups bobbing up and down."),
    CVAR_INT(r_fov, "", int_cvars_func1, r_fov_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The player's field of view (<b>45</b>\xB0 to <b>135</b>\xB0)."),
    CVAR_FLOAT(r_gamma, "", r_gamma_cvar_func1, r_gamma_cvar_func2, CF_NONE,
        "The screen's gamma correction level (<b>off</b>, or <b>0.50</b>\nto <b>2.0</b>)."),
    CVAR_BOOL(r_graduallighting, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles gradual lighting under doors and crushing\nsectors."),
    CVAR_BOOL(r_homindicator, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the flashing \"Hall Of Mirrors\" indicator."),
    CVAR_BOOL(r_hud, "", bool_cvars_func1, r_hud_cvar_func2, BOOLVALUEALIAS,
        "Toggles the heads-up display when in widescreen\nmode."),
    CVAR_BOOL(r_hud_translucency, "", bool_cvars_func1, r_hud_translucency_cvar_func2, BOOLVALUEALIAS,
        "Toggles the translucency of the heads-up display\nwhen in widescreen mode."),
    CVAR_BOOL(r_liquid_bob, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the bob of liquid sectors and the sprites\nin them."),
    CVAR_BOOL(r_liquid_clipsprites, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles clipping the bottom of sprites in liquid\nsectors."),
    CVAR_BOOL(r_liquid_current, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles a slight current being applied to liquid\nsectors."),
    CVAR_BOOL(r_liquid_lowerview, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles lowering the player's view when in a liquid\nsector."),
    CVAR_BOOL(r_liquid_swirl, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the swirl of liquid sectors."),
    CVAR_OTHER(r_lowpixelsize, "", null_func1, r_lowpixelsize_cvar_func2,
        "The size of each pixel when the graphic detail is low\n(<i>width</i><b>\xD7</b><i>height</i>)."),
    CVAR_BOOL(r_mirroredweapons, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles randomly mirroring the weapons dropped\nby monsters."),
    CVAR_BOOL(r_playersprites, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles showing the player's weapon."),
    CVAR_BOOL(r_rockettrails, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the trails of smoke behind rockets fired by\nthe player and cyberdemons."),
    CVAR_INT(r_screensize, "", int_cvars_func1, r_screensize_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The screen size (<b>0</b> to <b>7</b>)."),
    CVAR_BOOL(r_shadows, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles sprites casting shadows."),
    CVAR_BOOL(r_shadows_translucency, "", bool_cvars_func1, r_shadows_translucency_cvar_func2, BOOLVALUEALIAS,
        "Toggles the translucency of shadows cast by\nsprites."),
    CVAR_BOOL(r_shake_barrels, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles shaking the screen when the player is near\nan exploding barrel."),
    CVAR_INT(r_shake_damage, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the screen shakes when the player is\nattacked (<b>0%</b> to <b>100%</b>)."),
    CVAR_INT(r_skycolor, r_skycolour, r_skycolor_cvar_func1, r_skycolor_cvar_func2, CF_NONE, SKYVALUEALIAS,
        "The color of the sky (<b>none</b>, or <b>0</b> to <b>255</b>)."),
    CVAR_BOOL(r_supersampling, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles SSAA (supersampling anti-aliasing) when\nthe graphic detail is low."),
    CVAR_BOOL(r_textures, "", bool_cvars_func1, r_textures_cvar_func2, BOOLVALUEALIAS,
        "Toggles displaying all textures."),
    CVAR_BOOL(r_translucency, "", bool_cvars_func1, r_translucency_cvar_func2, BOOLVALUEALIAS,
        "Toggles the translucency of sprites and <i><b>BOOM</b></i>-\ncompatible wall textures."),
    CCMD(regenhealth, "", null_func1, regenhealth_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles regenerating the player's health when\nbelow 100%."),
    CCMD(reset, "", null_func1, reset_cmd_func2, true, RESETCMDFORMAT,
        "Resets a <i>CVAR</i> to its default."),
    CCMD(resetall, "", null_func1, resetall_cmd_func2, false, "",
        "Resets all CVARs to their defaults."),
    CCMD(respawnitems, "", null_func1, respawnitems_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles respawning items."),
    CCMD(respawnmonsters, "", respawnmonsters_cmd_func1, respawnmonsters_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles respawning monsters."),
    CCMD(restartmap, "", game_func1, restartmap_cmd_func2, false, "",
        "Restarts the current map."),
    CCMD(resurrect, "", resurrect_cmd_func1, resurrect_cmd_func2, true, RESURRECTCMDFORMAT,
        "Resurrects the <b>player</b>, <b>all</b> monsters or a type\nof <i>monster</i>."),
    CVAR_INT(s_channels, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The number of sound effects that can be played at\nthe same time (<b>8</b> to <b>64</b>)."),
    CVAR_INT(s_musicvolume, "", s_volume_cvars_func1, s_volume_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The volume level of music (<b>0%</b> to <b>100%</b>)."),
    CVAR_BOOL(s_randommusic, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles randomizing the music at the start of each\nmap."),
    CVAR_BOOL(s_randompitch, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles randomizing the pitch of monster sound\neffects."),
    CVAR_INT(s_sfxvolume, "", s_volume_cvars_func1, s_volume_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The volume level of sound effects (<b>0%</b> to <b>100%</b>)."),
    CVAR_BOOL(s_stereo, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles playing sound effects in mono or stereo."),
    CCMD(save, "", alive_func1, save_cmd_func2, true, SAVECMDFORMAT,
        "Saves the game to a file."),
    CVAR_INT(savegame, "", int_cvars_func1, savegame_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected savegame in the menu\n(<b>1</b> to <b>6</b>)."),
    CVAR_INT(skilllevel, "", int_cvars_func1, skilllevel_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected skill level in the menu\n(<b>1</b> to <b>5</b>)."),
    CCMD(spawn, summon, spawn_cmd_func1, spawn_cmd_func2, true, SPAWNCMDFORMAT,
        "Spawns an <i>item</i> or <i>monster</i> in front of the\nplayer."),
    CVAR_INT(stillbob, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's view and weapon bob up\nand down when they stand still (<b>0%</b> to <b>100%</b>)."),
    CCMD(take, "", take_cmd_func1, take_cmd_func2, true, TAKECMDFORMAT,
        "Takes <b>ammo</b>, <b>armor</b>, <b>health</b>, <b>keys</b>, <b>weapons</b>, or\n"
        "<b>all</b> or certain <i>items</i> from the player."),
    CCMD(teleport, "", teleport_cmd_func1, teleport_cmd_func2, true, TELEPORTCMDFORMAT,
        "Teleports the player to (<i>x</i>,<i>y</i>,<i>z</i>) in the current\nmap."),
    CCMD(thinglist, "", game_func1, thinglist_cmd_func2, false, "",
        "Lists all things in the current map."),
    CCMD(timer, "", null_func1, timer_cmd_func2, true, TIMERCMDFORMAT,
        "Sets a timer for each map."),
    CVAR_BOOL(tossdrop, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles tossing items dropped by monsters when\nthey die."),
    CVAR_INT(turbo, "", turbo_cvar_func1, turbo_cvar_func2, CF_PERCENT, NOVALUEALIAS,
        "The speed the player moves (<b>10%</b> to <b>400%</b>)."),
    CCMD(unbind, "", null_func1, unbind_cmd_func2, true, UNBINDCMDFORMAT,
        "Unbinds the +<i>action</i> from a <i>control</i>."),
    CVAR_BOOL(units, "", units_cvar_func1, units_cvar_func2, UNITSVALUEALIAS,
        "The units used in the <b>mapstats</b> and <b>playerstats</b>\nCCMDs (<b>imperial</b> or <b>metric</b>)."),
    CCMD(vanilla, "", null_func1, vanilla_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles vanilla mode."),
    CVAR_STR(version, "", null_func1, str_cvars_func2, CF_READONLY,
        "<i><b>" PACKAGE_NAME "'s</b></i> version."),
    CVAR_BOOL(vid_borderlesswindow, "", bool_cvars_func1, vid_borderlesswindow_cvar_func2, BOOLVALUEALIAS,
        "Toggles using a borderless window when fullscreen."),
    CVAR_INT(vid_capfps, "", vid_capfps_cvar_func1, vid_capfps_cvar_func2, CF_NONE, CAPVALUEALIAS,
        "The number of frames per second at which to cap\nthe framerate (<b>off</b>, or <b>10</b> to <b>1,000</b>). Interpolation is\n"
        "disabled when this CVAR is <b>35</b>."),
    CVAR_INT(vid_display, "", int_cvars_func1, vid_display_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The display used to render the game."),
#if !defined(_WIN32)
    CVAR_STR(vid_driver, "", null_func1, str_cvars_func2, CF_NONE,
        "The video driver used to render the game."),
#endif
    CVAR_BOOL(vid_fullscreen, "", bool_cvars_func1, vid_fullscreen_cvar_func2, BOOLVALUEALIAS,
        "Toggles between fullscreen and a window."),
    CVAR_INT(vid_motionblur, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount of motion blur when the player turns\nquickly (<b>0%</b> to <b>100%</b>)."),
    CVAR_BOOL(vid_pillarboxes, "", bool_cvars_func1, vid_pillarboxes_cvar_func2, BOOLVALUEALIAS,
        "Toggles using the pillarboxes either side of the\nscreen for palette effects."),
#if defined(_WIN32)
    CVAR_STR(vid_scaleapi, "", vid_scaleapi_cvar_func1, vid_scaleapi_cvar_func2, CF_NONE,
        "The API used to scale each frame (<b>\"direct3d\"</b>,\n<b>\"opengl\"</b> or <b>\"software\"</b>)."),
#elif defined(__APPLE__)
    CVAR_STR(vid_scaleapi, "", vid_scaleapi_cvar_func1, vid_scaleapi_cvar_func2, CF_NONE,
        "The API used to scale each frame (<b>\"metal\"</b>,\n"
        "<b>\"opengl\"</b>, <b>\"opengles\"</b>, <b>\"opengles2\"</b> or <b>\"software\"</b>)."),
#else
    CVAR_STR(vid_scaleapi, "", vid_scaleapi_cvar_func1, vid_scaleapi_cvar_func2, CF_NONE,
        "The API used to scale each frame (<b>\"opengl\"</b>,\n<b>\"opengles\"</b>, <b>\"opengles2\"</b> or <b>\"software\"</b>)."),
#endif
    CVAR_STR(vid_scalefilter, "", vid_scalefilter_cvar_func1, vid_scalefilter_cvar_func2, CF_NONE,
        "The filter used to scale each frame (<b>\"nearest\"</b>,\n<b>\"linear\"</b> or <b>\"nearest_linear\"</b>)."),
    CVAR_OTHER(vid_screenresolution, "", null_func1, vid_screenresolution_cvar_func2,
        "The screen's resolution when fullscreen (<b>desktop</b>\nor <i>width</i><b>\xD7</b><i>height</i>)."),
    CVAR_BOOL(vid_showfps, "", bool_cvars_func1, vid_showfps_cvar_func2, BOOLVALUEALIAS,
        "Toggles showing the number of frames per second."),
    CVAR_INT(vid_vsync, "", vid_vsync_cvar_func1, vid_vsync_cvar_func2, CF_NONE, VSYNCVALUEALIAS,
        "Toggles vertical sync with the display's refresh\nrate (<b>on</b>, <b>off</b> or <b>adaptive</b>)."),
    CVAR_BOOL(vid_widescreen, "", bool_cvars_func1, vid_widescreen_cvar_func2, BOOLVALUEALIAS,
        "Toggles widescreen mode."),
    CVAR_OTHER(vid_windowpos, vid_windowposition, null_func1, vid_windowpos_cvar_func2,
        "The position of the window on the desktop\n(<b>centered</b> or <b>(</b><i>x</i><b>,</b><i>y</i><b>)</b>)."),
    CVAR_OTHER(vid_windowsize, "", null_func1, vid_windowsize_cvar_func2,
        "The size of the window on the desktop\n(<i>width</i><b>\xD7</b><i>height</i>)."),
#if defined(_WIN32)
    CVAR_STR(wad, "", null_func1, str_cvars_func2, CF_READONLY,
        "The last WAD to be opened by the WAD launcher."),
#endif
    CVAR_INT(warninglevel, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The console's warning level (<b>0</b>, <b>1</b> or <b>2</b>)."),
    CVAR_INT(weaponbob, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's weapon bobs when they\nmove (<b>0%</b> to <b>100%</b>)."),
    CVAR_BOOL(weaponbounce, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the bouncing of the player's weapon when\ndropping from a greater height."),
    CVAR_BOOL(weaponrecoil, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the recoiling of the player's weapon when\nfired."),

    { "", "", null_func1, NULL, 0, 0, CF_NONE, NULL, 0, 0, 0, "", "" }
};

static dboolean run(void)
{
    return (gamekeydown[keyboardrun] ^ (!!mousebuttons[mouserun]) ^ (!!(gamepadbuttons & gamepadrun)) ^ alwaysrun);
}

static dboolean strafe(void)
{
    return (gamekeydown[keyboardstrafe] || mousebuttons[mousestrafe] || (gamepadbuttons & gamepadstrafe));
}

static void alwaysrun_action_func(void)
{
    G_ToggleAlwaysRun(ev_none);
    I_InitKeyboard();
}

static void automap_action_func(void)
{
    if (mapwindow)
        return;

    if (!automapactive)
        AM_Start(true);
    else
        AM_Stop();

    D_FadeScreen();
}

static void back_action_func(void)
{
    viewplayer->cmd.forwardmove -= forwardmove[run()];
    P_MovePlayer();
}

static void clearmark_action_func(void)
{
    if (automapactive || mapwindow)
        AM_ClearMarks();
}

static void console_action_func(void)
{
    if (consoleactive)
        C_HideConsole();
    else
        C_ShowConsole();
}

static void fire_action_func(void)
{
    P_FireWeapon();
}

static void followmode_action_func(void)
{
    if (automapactive)
        AM_ToggleFollowMode();
}

static void forward_action_func(void)
{
    viewplayer->cmd.forwardmove += forwardmove[run()];
    P_MovePlayer();
}

static void grid_action_func(void)
{
    if (automapactive || mapwindow)
        AM_ToggleGrid();
}

static void jump_action_func(void)
{
    viewplayer->cmd.buttons |= BT_JUMP;
}

static void left_action_func(void)
{
    if (strafe())
        viewplayer->cmd.sidemove -= sidemove[run()];
    else
        viewplayer->cmd.angleturn -= angleturn[run()];
}

static void mark_action_func(void)
{
    if (automapactive || mapwindow)
        AM_AddMark();
}

static void maxzoom_action_func(void)
{
    if (automapactive || mapwindow)
        AM_ToggleMaxZoom();
}

static void menu_action_func(void)
{
    M_StartControlPanel();
}

static void nextweapon_action_func(void)
{
    G_NextWeapon();
}

static void prevweapon_action_func(void)
{
    G_PrevWeapon();
}

static void right_action_func(void)
{
    if (strafe())
        viewplayer->cmd.sidemove += sidemove[run()];
    else
        viewplayer->cmd.angleturn += angleturn[run()];
}

static void rotatemode_action_func(void)
{
    if (automapactive || mapwindow)
        AM_ToggleRotateMode();
}

static void screenshot_action_func(void)
{
    G_ScreenShot();
}

static void strafeleft_action_func(void)
{
    viewplayer->cmd.sidemove -= sidemove[run()];
}

static void straferight_action_func(void)
{
    viewplayer->cmd.sidemove += sidemove[run()];
}

static void use_action_func(void)
{
    P_UseLines();
}

static void weapon1_action_func(void)
{
    P_ChangeWeapon(wp_fist);
}

static void weapon2_action_func(void)
{
    P_ChangeWeapon(wp_pistol);
}

static void weapon3_action_func(void)
{
    P_ChangeWeapon(wp_shotgun);
}

static void weapon4_action_func(void)
{
    P_ChangeWeapon(wp_chaingun);
}

static void weapon5_action_func(void)
{
    P_ChangeWeapon(wp_missile);
}

static void weapon6_action_func(void)
{
    P_ChangeWeapon(wp_plasma);
}

static void weapon7_action_func(void)
{
    P_ChangeWeapon(wp_bfg);
}

static int C_GetIndex(const char *cmd)
{
    int i = 0;

    while (*consolecmds[i].name)
    {
        if (M_StringCompare(cmd, consolecmds[i].name) || M_StringCompare(cmd, consolecmds[i].alternate))
            break;

        i++;
    }

    return i;
}

static void C_ShowDescription(int index)
{
    char    description[255];
    char    description1[255];
    char    *p;

    M_StringCopy(description, consolecmds[index].description, sizeof(description));
    description[0] = tolower(description[0]);

    if (consolecmds[index].type == CT_CCMD)
        M_snprintf(description1, sizeof(description1), "This CCMD %s", description);
    else
        M_snprintf(description1, sizeof(description1), "This CVAR %s%s",
            (M_StringStartsWith(description, "toggles") ? "" : ((consolecmds[index].flags & CF_READONLY) ? "is " : "changes ")),
            description);

    if ((p = strchr(description1, '\n')))
    {
        char    description2[255];

        *p++ = '\0';
        M_StringCopy(description2, p, sizeof(description2));

        if ((p = strchr(description2, '\n')))
        {
            *p++ = '\0';
            C_OutputWrap("%s %s %s", description1, description2, p);
        }
        else
            C_OutputWrap("%s %s", description1, description2);
    }
    else
        C_Output("%s", description1);
}

static dboolean alive_func1(char *cmd, char *parms)
{
    return (gamestate == GS_LEVEL && viewplayer->health > 0);
}

static dboolean cheat_func1(char *cmd, char *parms)
{
    if (M_StringCompare(cmd, cheat_clev.sequence))
    {
        dboolean    result;

        if (gamemode == commercial)
        {
            mapcmdepisode = 1;
            mapcmdmap = (parms[0] - '0') * 10 + parms[1] - '0';
            M_snprintf(mapcmdlump, sizeof(mapcmdlump), "MAP%c%c", parms[0], parms[1]);
        }
        else
        {
            mapcmdepisode = parms[0] - '0';
            mapcmdmap = parms[1] - '0';
            M_snprintf(mapcmdlump, sizeof(mapcmdlump), "E%cM%c", parms[0], parms[1]);
        }

        result = (W_CheckNumForName(mapcmdlump) >= 0
            && (gamemission != pack_nerve || mapcmdmap <= 9)
            && (!BTSX || W_CheckMultipleLumps(mapcmdlump) > 1));

        if (gamestate == GS_LEVEL)
            return result;
        else if (result)
        {
            C_Input("%s%s", cmd, parms);
            map_cmd_func2("map", mapcmdlump);
            return true;
        }
    }
    else if (gamestate != GS_LEVEL)
        return false;
    else if (M_StringCompare(cmd, cheat_god.sequence))
        return (gameskill != sk_nightmare);
    else if (M_StringCompare(cmd, cheat_ammonokey.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_ammo.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_mus.sequence))
        return (!nomusic && musicVolume);
    else if (M_StringCompare(cmd, cheat_noclip.sequence))
        return (gamemode != commercial && gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_commercial_noclip.sequence))
        return (gamemode == commercial && gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[0].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[1].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[2].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[3].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[4].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[5].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[6].sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_choppers.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_buddha.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_mypos.sequence))
        return true;
    else if (M_StringCompare(cmd, cheat_amap.sequence))
        return (automapactive || mapwindow);

    return false;
}

static dboolean game_func1(char *cmd, char *parms)
{
    return (gamestate == GS_LEVEL);
}

static dboolean null_func1(char *cmd, char *parms)
{
    return true;
}

//
// alias CCMD
//
dboolean C_ExecuteAlias(const char *alias)
{
    if (!executingalias)
        for (int i = 0; i < MAXALIASES; i++)
            if (M_StringCompare(alias, aliases[i].name))
            {
                char    *string = M_StringDuplicate(aliases[i].string);
                char    *strings[255];
                int     j = 0;

                strings[0] = strtok(string, ";");
                executingalias = true;

                while (strings[j])
                {
                    if (!C_ValidateInput(trimwhitespace(strings[j])))
                        break;

                    strings[++j] = strtok(NULL, ";");
                }

                executingalias = false;
                C_Input("%s", alias);
                free(string);
                return true;
            }

    return false;
}

void alias_cmd_func2(char *cmd, char *parms)
{
    char    parm1[128] = "";
    char    parm2[128] = "";

    if (sscanf(parms, "%127s %127[^\n]", parm1, parm2) <= 0)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, ALIASCMDFORMAT);
        return;
    }

    M_StripQuotes(parm1);

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(parm1, consolecmds[i].name))
            return;

    if (!*parm2)
    {
        for (int i = 0; i < MAXALIASES; i++)
            if (*aliases[i].name && M_StringCompare(parm1, aliases[i].name))
            {
                aliases[i].name[0] = '\0';
                aliases[i].string[0] = '\0';
                M_SaveCVARs();
                return;
            }

        return;
    }

    M_StripQuotes(parm2);

    for (int i = 0; i < MAXALIASES; i++)
        if (*aliases[i].name && M_StringCompare(parm1, aliases[i].name))
        {
            M_StringCopy(aliases[i].string, parm2, sizeof(aliases[i].string));
            M_SaveCVARs();
            return;
        }

    for (int i = 0; i < MAXALIASES; i++)
        if (!*aliases[i].name)
        {
            M_StringCopy(aliases[i].name, parm1, sizeof(aliases[i].name));
            M_StringCopy(aliases[i].string, parm2, sizeof(aliases[i].string));
            M_SaveCVARs();
            return;
        }
}

//
// bind CCMD
//
static void C_UnbindDuplicates(const int keep, const controltype_t type, const int value)
{
    for (int i = 0; *actions[i].action; i++)
        if (i != keep)
        {
            if (type == keyboardcontrol)
            {
                if (actions[i].keyboard1 && value == *(int *)actions[i].keyboard1)
                    *(int *)actions[i].keyboard1 = 0;

                if (actions[i].keyboard2 && value == *(int *)actions[i].keyboard2)
                    *(int *)actions[i].keyboard2 = 0;
            }
            else if (type == mousecontrol)
            {
                if (actions[i].mouse1 && value == *(int *)actions[i].mouse1)
                    *(int *)actions[i].mouse1 = -1;
            }
            else if (type == gamepadcontrol)
            {
                if (actions[i].gamepad1 && value == *(int *)actions[i].gamepad1)
                    *(int *)actions[i].gamepad1 = 0;

                if (actions[i].gamepad2 && value == *(int *)actions[i].gamepad2)
                    *(int *)actions[i].gamepad2 = 0;
            }
        }
}

void bind_cmd_func2(char *cmd, char *parms)
{
    int             i = 0;
    int             action = 0;
    char            parm1[128] = "";
    char            parm2[128] = "";
    const dboolean  mouselookcontrols = (keyboardmouselook || gamepadmouselook || mousemouselook != -1);

    if (sscanf(parms, "%127s %127[^\n]", parm1, parm2) <= 0)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, BINDCMDFORMAT);
        return;
    }

    M_StripQuotes(parm1);

    while (controls[i].type)
    {
        if (M_StringCompare(parm1, controls[i].control))
            break;

        i++;
    }

    if (*controls[i].control)
    {
        if (M_StringCompare(cmd, "unbind"))
        {
            while (*actions[action].action)
            {
                switch (controls[i].type)
                {
                    case keyboardcontrol:
                        if (actions[action].keyboard1 && controls[i].value == *(int *)actions[action].keyboard1)
                        {
                            *(int *)actions[action].keyboard1 = 0;
                            M_SaveCVARs();
                        }

                        if (actions[action].keyboard2 && controls[i].value == *(int *)actions[action].keyboard2)
                        {
                            *(int *)actions[action].keyboard2 = 0;
                            M_SaveCVARs();
                        }

                        break;

                    case mousecontrol:
                        if (actions[action].mouse1 && controls[i].value == *(int *)actions[action].mouse1)
                        {
                            *(int *)actions[action].mouse1 = -1;
                            M_SaveCVARs();
                        }

                        break;

                    case gamepadcontrol:
                        if (actions[action].gamepad1 && controls[i].value == *(int *)actions[action].gamepad1)
                        {
                            *(int *)actions[action].gamepad1 = 0;
                            M_SaveCVARs();
                        }

                        if (actions[action].gamepad2 && controls[i].value == *(int *)actions[action].gamepad2)
                        {
                            *(int *)actions[action].gamepad2 = 0;
                            M_SaveCVARs();
                        }

                        break;

                    default:
                        break;
                }

                action++;
            }

            if (controls[i].type == keyboardcontrol)
                keyactionlist[controls[i].value][0] = '\0';
            else if (controls[i].type == mousecontrol)
                mouseactionlist[controls[i].value][0] = '\0';
        }
        else if (!*parm2)
        {
            while (*actions[action].action)
            {
                if (controls[i].type == keyboardcontrol)
                {
                    if (actions[action].keyboard1 && controls[i].value == *(int *)actions[action].keyboard1)
                        C_Output(actions[action].action);
                    else if (actions[action].keyboard2 && controls[i].value == *(int *)actions[action].keyboard2)
                        C_Output(actions[action].action);
                }
                else if (controls[i].type == mousecontrol)
                {
                    if (actions[action].mouse1 && controls[i].value == *(int *)actions[action].mouse1)
                        C_Output(actions[action].action);
                }
                else if (controls[i].type == gamepadcontrol)
                {
                    if (actions[action].gamepad1 && controls[i].value == *(int *)actions[action].gamepad1)
                        C_Output(actions[action].action);
                    else if (actions[action].gamepad2 && controls[i].value == *(int *)actions[action].gamepad2)
                        C_Output(actions[action].action);
                }

                action++;
            }
        }
        else
        {
            while (*actions[action].action)
            {
                if (M_StringCompare(parm2, actions[action].action))
                    break;

                action++;
            }

            if (*actions[action].action)
            {
                dboolean    bound = false;

                switch (controls[i].type)
                {
                    case keyboardcontrol:
                        if (actions[action].keyboard1)
                        {
                            if (actions[action].keyboard2
                                && *(int *)actions[action].keyboard1
                                && *(int *)actions[action].keyboard1 != controls[i].value)
                            {
                                if (*(int *)actions[action].keyboard2)
                                {
                                    *(int *)actions[action].keyboard2 = *(int *)actions[action].keyboard1;
                                    *(int *)actions[action].keyboard1 = controls[i].value;
                                }
                                else
                                    *(int *)actions[action].keyboard2 = controls[i].value;
                            }
                            else
                                *(int *)actions[action].keyboard1 = controls[i].value;

                            bound = true;
                            C_UnbindDuplicates(action, keyboardcontrol, controls[i].value);

#if defined(_WIN32)
                            if (M_StringCompare(actions[action].action, "+screenshot"))
                            {
                                if (keyboardscreenshot == KEY_PRINTSCREEN)
                                {
                                    RegisterHotKey(NULL, 1, MOD_ALT, VK_SNAPSHOT);
                                    RegisterHotKey(NULL, 2, 0, VK_SNAPSHOT);
                                }
                                else
                                {
                                    UnregisterHotKey(NULL, 1);
                                    UnregisterHotKey(NULL, 2);
                                }
                            }
#endif
                        }

                        break;

                    case mousecontrol:
                        if (actions[action].mouse1)
                        {
                            *(int *)actions[action].mouse1 = controls[i].value;
                            bound = true;
                            C_UnbindDuplicates(action, mousecontrol, controls[i].value);
                        }

                        break;

                    case gamepadcontrol:
                        if (actions[action].gamepad1)
                        {
                            if (actions[action].gamepad2
                                && *(int *)actions[action].gamepad1
                                && *(int *)actions[action].gamepad1 != controls[i].value)
                            {
                                if (*(int *)actions[action].gamepad2)
                                {
                                    *(int *)actions[action].gamepad2 = *(int *)actions[action].gamepad1;
                                    *(int *)actions[action].gamepad1 = controls[i].value;
                                }
                                else
                                    *(int *)actions[action].gamepad2 = controls[i].value;
                            }
                            else
                                *(int *)actions[action].gamepad1 = controls[i].value;

                            bound = true;
                            C_UnbindDuplicates(action, gamepadcontrol, controls[i].value);
                        }

                        break;

                    default:
                        break;
                }

                M_SaveCVARs();

                if (!bound)
                {
                    if (strlen(controls[i].control) == 1)
                        C_Warning(0, "The <b>%s</b> action can't be bound to '<b>%s</b>'.", parm2, controls[i].control);
                    else
                        C_Warning(0, "The <b>%s</b> action can't be bound to <b>%s</b>.", parm2, controls[i].control);
                }
            }
            else
            {
                if (controls[i].type == keyboardcontrol)
                {
                    M_StringCopy(keyactionlist[controls[i].value], parm2, sizeof(keyactionlist[0]));
                    M_SaveCVARs();
                }
                else if (controls[i].type == mousecontrol)
                {
                    M_StringCopy(mouseactionlist[controls[i].value], parm2, sizeof(mouseactionlist[0]));
                    M_SaveCVARs();
                }
            }
        }
    }
    else if (M_StringCompare(cmd, "unbind"))
    {
        while (*actions[action].action)
        {
            if (M_StringCompare(parms, actions[action].action))
            {
                if (actions[action].keyboard1)
                    *(int *)actions[action].keyboard1 = 0;

                if (actions[action].keyboard2)
                    *(int *)actions[action].keyboard2 = 0;

                if (actions[action].mouse1)
                    *(int *)actions[action].mouse1 = -1;

                if (actions[action].gamepad1)
                    *(int *)actions[action].gamepad1 = 0;

                if (actions[action].gamepad2)
                    *(int *)actions[action].gamepad2 = 0;

                break;
            }

            action++;
        }
    }
    else
        C_Warning(0, "<b>%s</b> isn't a valid control.", parm1);

    if (mouselookcontrols != (keyboardmouselook || gamepadmouselook || mousemouselook != -1))
    {
        if (gamestate == GS_LEVEL)
            R_InitSkyMap();

        R_InitColumnFunctions();
    }
}

//
// bindlist CCMD
//
static void C_DisplayBinds(const char *action, const int value, const controltype_t type, int *count)
{
    const int   tabs[4] = { 40, 130, 0, 0 };

    for (int i = 0; controls[i].type; i++)
    {
        const char  *control = controls[i].control;

        if (controls[i].type == type && controls[i].value == value)
        {
            if (strlen(control) == 1)
                C_TabbedOutput(tabs, "%i.\t'%s'\t%s", (*count)++, (control[0] == '=' ? "+" : control), action);
            else
                C_TabbedOutput(tabs, "%i.\t%s\t%s", (*count)++, control, action);

            break;
        }
    }
}

static void bindlist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[4] = { 40, 131, 0, 0 };
    int         count = 1;

    C_Header(tabs, bindlist, BINDLISTHEADER);

    for (int i = 0; *actions[i].action; i++)
    {
        if (actions[i].keyboard1)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].keyboard1, keyboardcontrol, &count);

        if (actions[i].keyboard2)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].keyboard2, keyboardcontrol, &count);

        if (actions[i].mouse1)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].mouse1, mousecontrol, &count);

        if (actions[i].gamepad1)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].gamepad1, gamepadcontrol, &count);

        if (actions[i].gamepad2)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].gamepad2, gamepadcontrol, &count);
    }

    for (int i = 0; controls[i].type; i++)
    {
        const char  *control = controls[i].control;
        const int   value = controls[i].value;

        if (controls[i].type == keyboardcontrol && keyactionlist[value][0])
        {
            if (strlen(control) == 1)
                C_TabbedOutput(tabs, "%i.\t'%s'\t%s", count++, (control[0] == '=' ? "+" : control), keyactionlist[value]);
            else
                C_TabbedOutput(tabs, "%i.\t%s\t%s", count++, control, keyactionlist[value]);
        }
        else if (controls[i].type == mousecontrol && mouseactionlist[value][0])
            C_TabbedOutput(tabs, "%i.\t%s\t%s", count++, control, mouseactionlist[value]);
    }
}

//
// clear CCMD
//
static void clear_cmd_func2(char *cmd, char *parms)
{
    consolestrings = 0;
    consolestringsmax = 0;
    C_Output("");
}

//
// cmdlist CCMD
//
static void cmdlist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[4] = { 40, 346, 0, 0 };
    int         count = 0;

    for (int i = 0; *consolecmds[i].name; i++)
        if (consolecmds[i].type == CT_CCMD)
        {
            char    description1[255];
            char    format[255];
            char    *p;

            if (*parms && !wildcard(consolecmds[i].name, parms))
                continue;

            if (++count == 1)
                C_Header(tabs, cmdlist, CMDLISTHEADER);

            M_StringCopy(description1, consolecmds[i].description, sizeof(description1));

            if (M_StringCompare(consolecmds[i].name, "map"))
                M_StringCopy(format, (gamemission == doom ? MAPCMDFORMAT1 : MAPCMDFORMAT2), sizeof(format));
            else
                M_StringCopy(format, consolecmds[i].format, sizeof(format));

            if ((p = strchr(description1, '\n')))
            {
                char    description2[255] = "";

                *p++ = '\0';
                M_StringCopy(description2, p, sizeof(description2));

                C_TabbedOutput(tabs, "%i.\t<b>%s</b> %s\t%s", count, consolecmds[i].name, format, description1);
                C_TabbedOutput(tabs, "\t\t%s", description2);
            }
            else
                C_TabbedOutput(tabs, "%i.\t<b>%s</b> %s\t%s", count, consolecmds[i].name, format, description1);
        }
}

static FILE *condumpfile = NULL;

//
// condump CCMD
//
void C_DumpConsoleStringToFile(int index)
{
    if (!condumpfile)
        return;

    if (console[index].stringtype == dividerstring)
        fprintf(condumpfile, "%s\n", DIVIDERSTRING);
    else
    {
        char            *string = M_StringDuplicate(console[index].string);
        int             len;
        unsigned int    outpos = 0;
        int             tabcount = 0;

        strreplace(string, "<b>", "");
        strreplace(string, "</b>", "");
        strreplace(string, "<i>", "");
        strreplace(string, "</i>", "");
        len = (int)strlen(string);

        if (console[index].stringtype == warningstring)
            fputs((console[index].line == 1 ? "! " : (string[0] == ' ' ? " " : "  ")), condumpfile);

        for (int inpos = 0; inpos < len; inpos++)
        {
            const unsigned char letter = string[inpos];

            if (letter != '\n')
            {
                if (letter == '\t')
                {
                    const unsigned int  tabstop = console[index].tabs[tabcount] / 5;

                    if (outpos < tabstop)
                    {
                        for (unsigned int spaces = 0; spaces < tabstop - outpos; spaces++)
                            fputc(' ', condumpfile);

                        outpos = tabstop;
                        tabcount++;
                    }
                    else
                    {
                        fputc(' ', condumpfile);
                        outpos++;
                    }
                }
                else
                {
                    fputc(letter, condumpfile);
                    outpos++;
                }
            }
        }

        if (console[index].stringtype == playermessagestring || console[index].stringtype == obituarystring)
        {
            char    buffer[9];

            for (unsigned int spaces = 0; spaces < 92 - outpos; spaces++)
                fputc(' ', condumpfile);

            M_StringCopy(buffer, C_CreateTimeStamp(index), sizeof(buffer));

            if (strlen(buffer) == 7)
                fputc(' ', condumpfile);

            fputs(C_CreateTimeStamp(index), condumpfile);
        }

        fputc('\n', condumpfile);
        free(string);
    }
}

static dboolean condump_cmd_func1(char *cmd, char *parms)
{
    return (consolestrings > 1);
}

static void condump_cmd_func2(char *cmd, char *parms)
{
    char        filename[MAX_PATH];
    const char  *appdatafolder = M_GetAppDataFolder();

    if (!*parms)
    {
        char    consolefolder[MAX_PATH];
        int     count = 0;

        M_snprintf(consolefolder, sizeof(consolefolder), "%s" DIR_SEPARATOR_S "console", appdatafolder);
        M_MakeDirectory(consolefolder);
        M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "condump.txt", consolefolder);

        while (M_FileExists(filename))
        {
            char    *temp = commify(++count);

            M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "condump (%s).txt", consolefolder, temp);
            free(temp);
        }
    }
    else
        M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "%s", appdatafolder, parms);

    if ((condumpfile = fopen(filename, "wt")))
    {
        char    *temp = commify((int64_t)consolestrings - 2);

        for (int i = 1; i < consolestrings; i++)
            C_DumpConsoleStringToFile(i);

        C_Output("Dumped %s lines from the console to <b>%s</b>.", temp, filename);
        free(temp);
    }
}

//
// cvarlist CCMD
//
static void cvarlist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[4] = { 40, 209, 318, 0 };
    int         count = 0;

    for (int i = 0; *consolecmds[i].name; i++)
        if (consolecmds[i].type == CT_CVAR)
        {
            char    description1[255];
            char    description2[255] = "";
            char    description3[255] = "";
            char    *p;

            if (*parms && !wildcard(consolecmds[i].name, parms))
                continue;

            if (++count == 1)
                C_Header(tabs, cvarlist, CVARLISTHEADER);

            M_StringCopy(description1, consolecmds[i].description, sizeof(description1));

            if ((p = strchr(description1, '\n')))
            {
                *p++ = '\0';
                M_StringCopy(description2, p, sizeof(description2));

                if ((p = strchr(description2, '\n')))
                {
                    *p++ = '\0';
                    M_StringCopy(description3, p, sizeof(description3));
                }
            }

            if (M_StringCompare(consolecmds[i].name, stringize(ammo)))
            {
                if (gamestate == GS_LEVEL)
                    C_TabbedOutput(tabs, "%i.\t<b>%s\t%i</b>\t%s", count, consolecmds[i].name,
                        viewplayer->ammo[weaponinfo[viewplayer->readyweapon].ammotype], description1);
                else
                    C_TabbedOutput(tabs, "%i.\t<b>%s</b>\tn/a\t%s", count, consolecmds[i].name, description1);
            }
            else if (M_StringCompare(consolecmds[i].name, stringize(armor)))
            {
                if (gamestate == GS_LEVEL)
                    C_TabbedOutput(tabs, "%i.\t<b>%s\t%i%%</b>\t%s", count, consolecmds[i].name, viewplayer->armorpoints,
                        description1);
                else
                    C_TabbedOutput(tabs, "%i.\t<b>%s</b>\tn/a\t%s", count, consolecmds[i].name, description1);
            }
            else if (M_StringCompare(consolecmds[i].name, stringize(armortype)))
            {
                if (gamestate == GS_LEVEL)
                {
                    char    *temp = C_LookupAliasFromValue(viewplayer->armortype, ARMORTYPEVALUEALIAS);

                    C_TabbedOutput(tabs, "%i.\t<b>%s\t%s</b>\t%s", count, consolecmds[i].name, temp, description1);
                    free(temp);
                }
                else
                    C_TabbedOutput(tabs, "%i.\t<b>%s</b>\tn/a\t%s", count, consolecmds[i].name, description1);
            }
            else if (M_StringCompare(consolecmds[i].name, stringize(health)))
            {
                if (gamestate == GS_LEVEL)
                    C_TabbedOutput(tabs, "%i.\t<b>%s\t%i%%</b>\t%s", count, consolecmds[i].name, viewplayer->health, description1);
                else
                    C_TabbedOutput(tabs, "%i.\t<b>%s</b>\tn/a\t%s", count, consolecmds[i].name, description1);
            }
            else if (consolecmds[i].flags & CF_BOOLEAN)
            {
                char    *temp = C_LookupAliasFromValue(*(dboolean *)consolecmds[i].variable, consolecmds[i].aliases);

                C_TabbedOutput(tabs, "%i.\t<b>%s\t%s</b>\t%s", count, consolecmds[i].name, temp, description1);
                free(temp);
            }
            else if ((consolecmds[i].flags & CF_INTEGER) && (consolecmds[i].flags & CF_PERCENT))
                C_TabbedOutput(tabs, "%i.\t<b>%s\t%i%%</b>\t%s", count, consolecmds[i].name,
                    *(int *)consolecmds[i].variable, description1);
            else if (consolecmds[i].flags & CF_INTEGER)
            {
                char    *temp = C_LookupAliasFromValue(*(int *)consolecmds[i].variable, consolecmds[i].aliases);

                C_TabbedOutput(tabs, "%i.\t<b>%s\t%s</b>\t%s", count, consolecmds[i].name, temp, description1);
                free(temp);
            }
            else if (consolecmds[i].flags & CF_FLOAT)
            {
                if (consolecmds[i].flags & CF_PERCENT)
                {
                    char    *temp = striptrailingzero(*(float *)consolecmds[i].variable, 1);

                    C_TabbedOutput(tabs, "%i.\t<b>%s\t%s%%</b>\t%s", count, consolecmds[i].name, temp, description1);
                    free(temp);
                }
                else
                {
                    char    buffer[128];
                    int     len;

                    M_snprintf(buffer, sizeof(buffer), "%.2f", *(float *)consolecmds[i].variable);
                    len = (int)strlen(buffer);

                    if (len >= 2 && buffer[len - 1] == '0' && buffer[len - 2] == '0')
                        buffer[len - 1] = '\0';

                    C_TabbedOutput(tabs, "%i.\t<b>%s\t%s</b>\t%s", count, consolecmds[i].name, buffer, description1);
                }
            }
            else if (consolecmds[i].flags & CF_STRING)
                C_TabbedOutput(tabs, "%i.\t<b>%s\t%s%.14s%s%s</b>\t%s", count, consolecmds[i].name,
                    (M_StringCompare(consolecmds[i].name, stringize(version)) ? "" : "\""), *(char **)consolecmds[i].variable,
                    (M_StringCompare(consolecmds[i].name, stringize(version)) ? "" : "\""),
                    (strlen(*(char **)consolecmds[i].variable) > 14 ? "..." : ""), description1);
            else if (consolecmds[i].flags & CF_TIME)
            {
                const int   tics = *(int *)consolecmds[i].variable / TICRATE;

                C_TabbedOutput(tabs, "%i.\t<b>%s\t%02i:%02i:%02i</b>\t%s", count, consolecmds[i].name,
                    tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60, description1);
            }
            else if (consolecmds[i].flags & CF_OTHER)
                C_TabbedOutput(tabs, "%i.\t<b>%s\t%s</b>\t%s", count, consolecmds[i].name,
                    *(char **)consolecmds[i].variable, description1);

            if (!M_StringCompare(description2, ""))
            {
                C_TabbedOutput(tabs, "\t\t\t%s", description2);

                if (!M_StringCompare(description3, ""))
                    C_TabbedOutput(tabs, "\t\t\t%s", description3);
            }
        }
}

//
// endgame CCMD
//
static void endgame_cmd_func2(char *cmd, char *parms)
{
    M_EndingGame();
    C_HideConsoleFast();
}

//
// exec CCMD
//
static void exec_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, EXECCMDFORMAT);
    }
    else
    {
        FILE    *file = fopen(parms, "r");
        char    strparm[256] = "";

        if (!file)
            return;

        while (fgets(strparm, 256, file) != NULL)
        {
            if (strparm[0] == ';')
                continue;

            C_ValidateInput(strparm);
        }

        fclose(file);
    }
}

//
// exitmap CCMD
//
static void exitmap_cmd_func2(char *cmd, char *parms)
{
    G_ExitLevel();
    C_HideConsoleFast();
}

//
// fastmonsters CCMD
//
static dboolean fastmonsters_cmd_func1(char *cmd, char *parms)
{
    return (gameskill != sk_nightmare);
}

static void fastmonsters_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && fastparm)
            fastparm = false;
        else if (value == 1 && !fastparm)
            fastparm = true;
        else
            return;
    }
    else
        fastparm = !fastparm;

    G_SetFastParms(fastparm);

    if (fastparm)
    {
        C_Output(s_STSTR_FMON);
        HU_SetPlayerMessage(s_STSTR_FMON, false, false);
    }
    else
    {
        C_Output(s_STSTR_FMOFF);
        HU_SetPlayerMessage(s_STSTR_FMOFF, false, false);
    }

    message_dontfuckwithme = true;
}

//
// freeze CCMD
//
static void freeze_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && freeze)
            freeze = false;
        else if (value == 1 && !freeze)
            freeze = true;
        else
            return;
    }
    else
        freeze = !freeze;

    if (freeze)
    {
        C_Output(s_STSTR_FON);
        HU_SetPlayerMessage(s_STSTR_FON, false, false);
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_FOFF);
        HU_SetPlayerMessage(s_STSTR_FOFF, false, false);
    }

    message_dontfuckwithme = true;
    C_HideConsole();
}

//
// give CCMD
//
static dboolean give_cmd_func1(char *cmd, char *parms)
{
    dboolean    result = false;
    char        *parm;

    if (gamestate != GS_LEVEL)
        return false;

    parm = removenonalpha(parms);

    if (!*parm)
        return true;

    if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything")
        || M_StringCompare(parm, "health") || M_StringCompare(parm, "fullhealth")
        || M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons")
        || M_StringCompare(parm, "ammo") || M_StringCompare(parm, "fullammo")
        || M_StringCompare(parm, "armor") || M_StringCompare(parm, "fullarmor")
        || M_StringCompare(parm, "armour") || M_StringCompare(parm, "fullarmour")
        || M_StringCompare(parm, "keys") || M_StringCompare(parm, "allkeys")
        || M_StringCompare(parm, "keycards") || M_StringCompare(parm, "allkeycards")
        || M_StringCompare(parm, "skullkeys") || M_StringCompare(parm, "allskullkeys")
        || M_StringCompare(parm, "pistol"))
        result = true;
    else
    {
        for (int i = 0, num = -1; i < NUMMOBJTYPES; i++)
        {
            char    *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
            char    *temp2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
            char    *temp3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);

            if ((mobjinfo[i].flags & MF_SPECIAL)
                && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                    || (*mobjinfo[i].name2 && M_StringCompare(parm, temp2))
                    || (*mobjinfo[i].name3 && M_StringCompare(parm, temp3))
                    || (sscanf(parm, "%10d", &num) == 1 && num == mobjinfo[i].doomednum && num != -1)))
                result = true;

            if (temp1)
                free(temp1);

            if (temp2)
                free(temp2);

            if (temp3)
                free(temp3);

            if (result)
                break;
        }
    }

    free(parm);
    return result;
}

static void give_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, GIVECMDFORMAT);
    }
    else
    {
        if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything"))
        {
            dboolean    result = false;

            if (P_GiveBackpack(false, false))
                result = true;

            if (P_GiveMegaHealth(false))
                result = true;

            if (P_GiveAllWeapons())
                result = true;

            if (P_GiveFullAmmo())
                result = true;

            if (P_GiveArmor(blue_armor_class, false))
                result = true;

            if (P_GiveAllCardsInMap())
                result = true;

            for (int i = 0; i < NUMPOWERS; i++)
                if (P_GivePower(i))
                    result = true;

            if (result)
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);
                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have everything.");
                else
                    C_Warning(0, "%s already has everything.", playername);

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "health") || M_StringCompare(parm, "fullhealth"))
        {
            if (P_GiveMegaHealth(false))
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);
                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have full health.");
                else
                    C_Warning(0, "%s already has full health.", playername);

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons"))
        {
            if (P_GiveAllWeapons())
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);
                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all the weapons.");
                else
                    C_Warning(0, "%s already has all the weapons.", playername);

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "ammo") || M_StringCompare(parm, "fullammo"))
        {
            if (P_GiveFullAmmo())
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);
                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have full ammo.");
                else
                    C_Warning(0, "%s already has full ammo.", playername);

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "armor") || M_StringCompare(parm, "fullarmor")
            || M_StringCompare(parm, "armour") || M_StringCompare(parm, "fullarmour"))
        {
            if (P_GiveArmor(blue_armor_class, false))
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);
                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have full armor.");
                else
                    C_Warning(0, "%s already has full armor.", playername);

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "keys") || M_StringCompare(parm, "allkeys"))
        {
            if (P_GiveAllCards())
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);
                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all the keycards and skull keys.");
                else
                    C_Warning(0, "%s already has all the keycards and skull keys.", playername);

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "keycards") || M_StringCompare(parm, "allkeycards"))
        {
            if (P_GiveAllKeyCards())
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);
                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all the keycards.");
                else
                    C_Warning(0, "%s already has all the keycards.", playername);

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "skullkeys") || M_StringCompare(parm, "allskullkeys"))
        {
            if (P_GiveAllSkullKeys())
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);
                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all the skull keys.");
                else
                    C_Warning(0, "%s already has all the skull keys.", playername);

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "pistol"))
        {
            if (viewplayer->weaponowned[wp_pistol])
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have a pistol.");
                else
                    C_Warning(0, "%s already has a pistol.", playername);

                free(parm);
                return;
            }

            viewplayer->weaponowned[wp_pistol] = true;
            oldweaponsowned[wp_pistol] = true;
            P_EquipWeapon(wp_pistol);
            C_HideConsole();
            free(parm);
            return;
        }
        else
        {
            for (int i = 0, num = -1; i < NUMMOBJTYPES; i++)
            {
                dboolean    result = false;
                char        *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
                char        *temp2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char        *temp3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);

                if ((mobjinfo[i].flags & MF_SPECIAL)
                    && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, temp2))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, temp3))
                        || (sscanf(parm, "%10d", &num) == 1 && num == mobjinfo[i].doomednum && num != -1)))
                {
                    dboolean    old_freeze = freeze;

                    if (gamemode != commercial && (i == MT_SUPERSHOTGUN || i == MT_MEGA))

                        C_Warning(0, "%s can't get %s in <i><b>%s.</b></i>",
                            (M_StringCompare(playername, playername_default) ? "You" : playername),
                            mobjinfo[i].plural1, gamedescription);
                    else if (gamemode == shareware && (i == MT_MISC7 || i == MT_MISC8 || i == MT_MISC9
                        || i == MT_MISC20 || i == MT_MISC21 || i == MT_MISC25 || i == MT_MISC28))
                        C_Warning(0, "%s can't get %s in <i><b>%s.</b></i>",
                            (M_StringCompare(playername, playername_default) ? "You" : playername),
                            mobjinfo[i].plural1, gamedescription);
                    else
                    {
                        freeze = false;
                        P_TouchSpecialThing(P_SpawnMobj(viewx, viewy, viewz, i), viewplayer->mo, false, false);
                        freeze = old_freeze;
                        C_HideConsole();
                        result = true;
                    }
                }

                if (temp1)
                    free(temp1);

                if (temp2)
                    free(temp2);

                if (temp3)
                    free(temp3);

                if (result)
                    break;
            }
        }

        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
        free(parm);
    }
}

//
// god CCMD
//
static void god_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && (viewplayer->cheats & CF_GODMODE))
            viewplayer->cheats &= ~CF_GODMODE;
        else if (value == 1 && !(viewplayer->cheats & CF_GODMODE))
            viewplayer->cheats |= CF_GODMODE;
        else
            return;
    }
    else
        viewplayer->cheats ^= CF_GODMODE;

    if (viewplayer->cheats & CF_GODMODE)
    {
        C_Output(s_STSTR_GODON);
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }
    else
        C_Output(s_STSTR_GODOFF);
}

//
// help CCMD
//
static void help_cmd_func2(char *cmd, char *parms)
{
#if defined(_WIN32)
    ShellExecute(NULL, "open", PACKAGE_WIKIURL, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__linux__)
    system("xdg-open " PACKAGE_WIKIURL);
#elif defined(__APPLE__)
    system("open " PACKAGE_WIKIURL);
#endif
}

//
// if CCMD
//
static dboolean match(dboolean value, char *toggle)
{
    return ((value && M_StringCompare(toggle, "on")) || (!value && M_StringCompare(toggle, "off")));
}

static void if_cmd_func2(char *cmd, char *parms)
{
    char    parm1[64] = "";
    char    parm2[64] = "";
    char    parm3[128] = "";

    if (sscanf(parms, "%63s %63s then %127[^\n]", parm1, parm2, parm3) != 3)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, IFCMDFORMAT);
        return;
    }

    M_StripQuotes(parm1);

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(parm1, consolecmds[i].name))
        {
            dboolean    condition = false;

            M_StripQuotes(parm2);

            if (consolecmds[i].type == CT_CVAR)
            {
                if (consolecmds[i].flags & (CF_BOOLEAN | CF_INTEGER))
                {
                    int value = C_LookupValueFromAlias(parm2, consolecmds[i].aliases);

                    if (value != INT_MIN || sscanf(parms, "%10d", &value) == 1)
                        condition = (value != INT_MIN && value == *(int *)consolecmds[i].variable);
                }
                else if (consolecmds[i].flags & CF_FLOAT)
                {
                    float value = FLT_MIN;

                    if (sscanf(parms, "%10f", &value) == 1)
                        condition = (value != FLT_MIN && value == *(float *)consolecmds[i].variable);
                }
                else
                    condition = M_StringCompare(parm2, *(char **)consolecmds[i].variable);
            }
            else if (M_StringCompare(parm1, "fastmonsters"))
                condition = match(fastparm, parm2);
            else if (M_StringCompare(parm1, "freeze"))
                condition = match(freeze, parm2);
            else if (M_StringCompare(parm1, "god"))
                condition = match((gamestate == GS_LEVEL && (viewplayer->cheats & CF_GODMODE)), parm2);
            else if (M_StringCompare(parm1, "noclip"))
                condition = match((gamestate == GS_LEVEL && (viewplayer->cheats & CF_NOCLIP)), parm2);
            else if (M_StringCompare(parm1, "nomonsters"))
                condition = match(nomonsters, parm2);
            else if (M_StringCompare(parm1, "notarget"))
                condition = match((gamestate == GS_LEVEL && (viewplayer->cheats & CF_NOTARGET)), parm2);
            else if (M_StringCompare(parm1, "pistolstart"))
                condition = match(pistolstart, parm2);
            else if (M_StringCompare(parm1, "regenhealth"))
                condition = match(regenhealth, parm2);
            else if (M_StringCompare(parm1, "respawnitems"))
                condition = match(respawnitems, parm2);
            else if (M_StringCompare(parm1, "respawnmonsters"))
                condition = match(respawnmonsters, parm2);
            else if (M_StringCompare(parm1, "vanilla"))
                condition = match(vanilla, parm2);

            if (condition)
            {
                char    *strings[255];
                int     j = 0;

                M_StripQuotes(parm3);
                strings[0] = strtok(parm3, ";");

                while (strings[j])
                {
                    if (!C_ValidateInput(trimwhitespace(strings[j])))
                        break;

                    strings[++j] = strtok(NULL, ";");
                }
            }

            break;
        }
}

//
// kill CCMD
//
static int      killcmdtype = NUMMOBJTYPES;
static mobj_t   *killcmdmobj;
dboolean        massacre;

static dboolean kill_cmd_func1(char *cmd, char *parms)
{
    dboolean    result = false;
    char        *parm;

    if (gamestate != GS_LEVEL)
        return false;

    parm = removenonalpha(parms);

    if (!*parm)
        return true;

    killcmdmobj = NULL;

    if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        result = (viewplayer->health > 0);
    else if (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters") || M_StringCompare(parm, "all")
        || M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
        || M_StringCompare(parm, "friendly monster") || M_StringCompare(parm, "friendly monsters")
        || M_StringCompare(parm, "missile") || M_StringCompare(parm, "missiles"))
        result = true;
    else
    {
        for (int i = 0, num = -1; i < NUMMOBJTYPES; i++)
            if (*mobjinfo[i].name1)
            {
                char    *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
                char    *temp2 = (*mobjinfo[i].plural1 ? removenonalpha(mobjinfo[i].plural1) : NULL);
                char    *temp3 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char    *temp4 = (*mobjinfo[i].plural2 ? removenonalpha(mobjinfo[i].plural2) : NULL);
                char    *temp5 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);
                char    *temp6 = (*mobjinfo[i].plural3 ? removenonalpha(mobjinfo[i].plural3) : NULL);

                if (M_StringStartsWith(parm, "all"))
                    strreplace(parm, "all", "");

                killcmdtype = mobjinfo[i].doomednum;

                if (killcmdtype >= 0
                    && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                        || (*mobjinfo[i].plural1 && M_StringCompare(parm, temp2))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, temp3))
                        || (*mobjinfo[i].plural2 && M_StringCompare(parm, temp4))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, temp5))
                        || (*mobjinfo[i].plural3 && M_StringCompare(parm, temp6))
                        || (sscanf(parm, "%10d", &num) == 1 && num == killcmdtype && num != -1)))
                {
                    if (killcmdtype == WolfensteinSS && !allowwolfensteinss && !states[S_SSWV_STND].dehacked)
                        result = false;
                    else
                        result = mobjinfo[i].flags & MF_SHOOTABLE;
                }

                if (temp1)
                    free(temp1);

                if (temp2)
                    free(temp2);

                if (temp3)
                    free(temp3);

                if (temp4)
                    free(temp4);

                if (temp5)
                    free(temp5);

                if (temp6)
                    free(temp6);

                if (result)
                    break;
            }

        if (!result)
            for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
            {
                mobj_t  *mobj = (mobj_t *)th;

                if (*mobj->name)
                {
                    char    *temp = removenonalpha(mobj->name);

                    if (M_StringCompare(parm, temp))
                    {
                        killcmdmobj = mobj;
                        result = true;
                    }

                    free(temp);

                    if (result)
                        break;
                }
            }
    }

    free(parm);
    return result;
}

void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);

void kill_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, KILLCMDFORMAT);
    }
    else
    {
        char    buffer[1024];

        if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        {
            viewplayer->health = 0;
            viewplayer->mo->health = 0;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            viewplayer->attacker = NULL;

            if (viewplayer->fixedcolormap == INVERSECOLORMAP)
                viewplayer->fixedcolormap = 0;

            viewplayer->mo->flags2 |= MF2_MASSACRE;
            P_KillMobj(viewplayer->mo, NULL, viewplayer->mo);
            M_snprintf(buffer, sizeof(buffer), "%s killed %s.",
                playername, (M_StringCompare(playername, playername_default) ? "yourself" : "themselves"));
            buffer[0] = toupper(buffer[0]);
            C_Obituary(buffer);
            C_HideConsole();
            HU_SetPlayerMessage(buffer, false, false);
            message_dontfuckwithme = true;
        }
        else
        {
            dboolean    friends = (M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
                            || M_StringCompare(parm, "friendly monster") || M_StringCompare(parm, "friendly monsters"));
            dboolean    enemies = (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters"));
            dboolean    all = M_StringCompare(parm, "all");
            int         kills = 0;
            int         prevkills = totalkills;

            if (friends || enemies || all)
            {
                massacre = true;

                for (int i = 0; i < numsectors; i++)
                {
                    mobj_t  *thing = sectors[i].thinglist;

                    while (thing)
                    {
                        const int   flags = thing->flags;

                        if (all || !!(flags & MF_FRIEND) == friends)
                        {
                            if (thing->flags2 & MF2_MONSTERMISSILE)
                            {
                                thing->flags2 |= MF2_MASSACRE;
                                P_ExplodeMissile(thing);
                            }
                            else if (thing->health > 0)
                            {
                                const mobjtype_t    type = thing->type;

                                if (type == MT_PAIN)
                                {
                                    A_Fall(thing, NULL, NULL);
                                    P_SetMobjState(thing, S_PAIN_DIE6);
                                    viewplayer->mobjcount[MT_PAIN]++;
                                    stat_monsterskilled_painelementals = SafeAdd(stat_monsterskilled_painelementals, 1);
                                    viewplayer->killcount++;
                                    stat_monsterskilled = SafeAdd(stat_monsterskilled, 1);
                                    kills++;
                                }
                                else if ((flags & MF_SHOOTABLE) && type != MT_PLAYER && type != MT_BARREL && (type != MT_HEAD || !hacx))
                                {
                                    thing->flags2 |= MF2_MASSACRE;
                                    P_DamageMobj(thing, NULL, NULL, thing->health, false);

                                    if (!(flags & MF_NOBLOOD))
                                        P_SpawnMoreBlood(thing);

                                    kills++;
                                }
                            }
                        }

                        thing = thing->snext;
                    }
                }

                if (kills)
                {
                    char    *temp = commify(kills);

                    M_snprintf(buffer, sizeof(buffer), "%s %s %smonster%s in this map %s been killed.", (kills == 1 ? "The" : "All"),
                        temp, (kills < prevkills ? "remaining " : ""), (kills == 1 ? "" : "s"), (kills == 1 ? "has" : "have"));
                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                    viewplayer->cheated++;
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    M_SaveCVARs();
                    free(temp);
                }
                else
                    C_Warning(0, "There are no monsters in this map %s kill.", (!totalkills ? "to" : "left to"));
            }
            else if (M_StringCompare(parm, "missile") || M_StringCompare(parm, "missiles"))
            {
                for (int i = 0; i < numsectors; i++)
                {
                    mobj_t  *thing = sectors[i].thinglist;

                    while (thing)
                    {
                        if (thing->flags2 & MF2_MONSTERMISSILE)
                        {
                            P_ExplodeMissile(thing);
                            kills++;
                        }

                        thing = thing->snext;
                    }
                }

                if (kills)
                {
                    char    *temp = commify(kills);

                    M_snprintf(buffer, sizeof(buffer), "%s %s missile%s %s exploded.", (kills == 1 ? "The" : "All"), temp,
                        (kills == 1 ? "" : "s"), (kills == 1 ? "has" : "have"));
                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                    viewplayer->cheated++;
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    M_SaveCVARs();
                    free(temp);
                }
                else
                    C_Warning(0, "There are no missiles to explode.");
            }
            else if (killcmdmobj)
            {
                char    *temp = sentencecase(parm);

                killcmdmobj->flags2 |= MF2_MASSACRE;
                P_DamageMobj(killcmdmobj, NULL, NULL, killcmdmobj->health, false);

                if (!(killcmdmobj->flags & MF_NOBLOOD))
                {
                    const int   r = M_RandomInt(-1, 1);

                    killcmdmobj->momx += r;
                    killcmdmobj->momy += (!r ? M_RandomIntNoRepeat(-1, 1, 0) : M_RandomInt(-1, 1)) * FRACUNIT;
                }

                M_snprintf(buffer, sizeof(buffer), "%s was killed.", temp);
                C_Output(buffer);
                C_HideConsole();
                HU_SetPlayerMessage(buffer, false, false);
                message_dontfuckwithme = true;
                viewplayer->cheated++;
                stat_cheated = SafeAdd(stat_cheated, 1);
                M_SaveCVARs();
                free(temp);
            }
            else
            {
                const mobjtype_t    type = P_FindDoomedNum(killcmdtype);
                int                 dead = 0;

                for (int i = 0; i < numsectors; i++)
                {
                    mobj_t  *thing = sectors[i].thinglist;

                    while (thing)
                    {
                        if (type == thing->type)
                        {
                            if (type == MT_PAIN)
                            {
                                if (thing->health > 0)
                                {
                                    A_Fall(thing, NULL, NULL);
                                    P_SetMobjState(thing, S_PAIN_DIE6);
                                    viewplayer->mobjcount[MT_PAIN]++;
                                    stat_monsterskilled_painelementals = SafeAdd(stat_monsterskilled_painelementals, 1);
                                    viewplayer->killcount++;
                                    stat_monsterskilled = SafeAdd(stat_monsterskilled, 1);
                                    kills++;
                                }
                                else
                                    dead++;
                            }
                            else if ((thing->flags & MF_SHOOTABLE) && thing->health > 0)
                            {
                                thing->flags2 |= MF2_MASSACRE;
                                P_DamageMobj(thing, NULL, NULL, thing->health, false);

                                if (!(thing->flags & MF_NOBLOOD))
                                {
                                    const int   r = M_RandomInt(-1, 1);

                                    thing->momx += r * FRACUNIT;
                                    thing->momy += (!r ? M_RandomIntNoRepeat(-1, 1, 0) : M_RandomInt(-1, 1)) * FRACUNIT;
                                }

                                kills++;
                            }
                            else if (thing->flags & MF_CORPSE)
                                dead++;
                        }

                        thing = thing->snext;
                    }
                }

                if (kills)
                {
                    char    *temp = commify(kills);

                    M_snprintf(buffer, sizeof(buffer), "%s %s %s in this map %s %s.", (kills == 1 ? "The" : "All"), temp,
                        (kills == 1 ? mobjinfo[type].name1 : mobjinfo[type].plural1), (kills == 1 ? "has" : "have"),
                        (type == MT_BARREL ? "exploded" : "been killed"));
                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                    viewplayer->cheated++;
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    M_SaveCVARs();
                    free(temp);
                }
                else
                {
                    if (gamemode != commercial)
                    {
                        if (killcmdtype >= ArchVile && killcmdtype <= MonstersSpawner)
                        {
                            C_Warning(0, "There are no %s in <i><b>%s.</b></i>", mobjinfo[type].plural1, gamedescription);
                            return;
                        }
                        else if (gamemode == shareware && (killcmdtype == Cyberdemon || killcmdtype == SpiderMastermind))
                        {
                            C_Warning(0, "There are no %s in <i><b>%s.</b></i>", mobjinfo[type].plural1, gamedescription);
                            return;
                        }
                    }

                    C_Warning(0, "There are no %s %s %s.", mobjinfo[type].plural1, (dead ? "left to" : "to"),
                        (type == MT_BARREL ? "explode" : "kill"));
                }
            }
        }

        free(parm);
    }
}

//
// license CCMD
//
static void license_cmd_func2(char *cmd, char *parms)
{
#if defined(_WIN32)
    ShellExecute(NULL, "open", PACKAGE_WIKILICENSEURL, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__linux__)
    system("xdg-open " PACKAGE_WIKILICENSEURL);
#elif defined(__APPLE__)
    system("open " PACKAGE_WIKILICENSEURL);
#endif
}

//
// load CCMD
//
static void load_cmd_func2(char *cmd, char *parms)
{
    char    buffer[1024];

    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, LOADCMDFORMAT);
        return;
    }

    M_snprintf(buffer, sizeof(buffer), "%s%s%s",
        (M_StringStartsWith(parms, savegamefolder) ? "" : savegamefolder), parms, (M_StringEndsWith(parms, ".save") ? "" : ".save"));
    G_LoadGame(buffer);
}

//
// map CCMD
//
static dboolean map_cmd_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;
    else
    {
        dboolean    result = false;
        char        *temp1 = removenonalpha(parms);
        char        *parm = uppercase(temp1);

        mapcmdepisode = 0;
        mapcmdmap = 0;

        if (M_StringCompare(parm, "first"))
        {
            if (gamemode == commercial)
            {
                if (gamemap != 1)
                {
                    mapcmdepisode = gameepisode;
                    mapcmdmap = 1;
                    M_StringCopy(mapcmdlump, "MAP01", sizeof(mapcmdlump));
                    result = true;
                }
            }
            else
            {
                if (!(gameepisode == 1 && gamemap == 1))
                {
                    mapcmdepisode = 1;
                    mapcmdmap = 1;
                    M_StringCopy(mapcmdlump, "E1M1", sizeof(mapcmdlump));
                    result = true;
                }
            }
        }
        else if ((M_StringCompare(parm, "previous") || M_StringCompare(parm, "prev")) && gamestate != GS_TITLESCREEN)
        {
            if (gamemode == commercial)
            {
                if (gamemap != 1)
                {
                    mapcmdepisode = gameepisode;
                    mapcmdmap = gamemap - 1;
                    M_snprintf(mapcmdlump, sizeof(mapcmdlump), "MAP%02i", mapcmdmap);
                    result = true;
                }
            }
            else
            {
                if (gamemap == 1)
                {
                    if (gameepisode != 1)
                    {
                        mapcmdepisode = gameepisode - 1;
                        mapcmdmap = 8;
                        result = true;
                    }
                }
                else
                {
                    mapcmdepisode = gameepisode;
                    mapcmdmap = gamemap - 1;
                    result = true;
                }

                M_snprintf(mapcmdlump, sizeof(mapcmdlump), "E%iM%i", mapcmdepisode, mapcmdmap);
            }
        }
        else if (M_StringCompare(parm, "next") && gamestate != GS_TITLESCREEN)
        {
            if (gamemode == commercial)
            {
                if (gamemap != (gamemission == pack_nerve ? 8 : 30))
                {
                    mapcmdepisode = gameepisode;
                    mapcmdmap = gamemap + 1;
                    M_snprintf(mapcmdlump, sizeof(mapcmdlump), "MAP%02i", mapcmdmap);
                    result = true;
                }
            }
            else
            {
                if (gamemap == 8)
                {
                    if (gameepisode != (gamemode == retail ? (sigil ? 5 : 4) : (gamemode == shareware || chex ? 1 : 3)))
                    {
                        mapcmdepisode = gameepisode + 1;
                        mapcmdmap = 1;
                        result = true;
                    }
                }
                else
                {
                    mapcmdepisode = gameepisode;
                    mapcmdmap = gamemap + 1;
                    result = true;
                }

                M_snprintf(mapcmdlump, sizeof(mapcmdlump), "E%iM%i", mapcmdepisode, mapcmdmap);
            }
        }
        else if (M_StringCompare(parm, "last"))
        {
            if (gamemode == commercial)
            {
                if (gamemission == pack_nerve)
                {
                    if (gamemap != 8)
                    {
                        mapcmdepisode = gameepisode;
                        mapcmdmap = 8;
                        M_StringCopy(mapcmdlump, "MAP08", sizeof(mapcmdlump));
                        result = true;
                    }
                }
                else
                {
                    if (gamemap != 30)
                    {
                        mapcmdepisode = gameepisode;
                        mapcmdmap = 30;
                        M_StringCopy(mapcmdlump, "MAP30", sizeof(mapcmdlump));
                        result = true;
                    }
                }
            }
            else if (gamemode == shareware || chex)
            {
                if (!(gameepisode == 1 && gamemap == 8))
                {
                    mapcmdepisode = 1;
                    mapcmdmap = 8;
                    M_StringCopy(mapcmdlump, "E1M8", sizeof(mapcmdlump));
                    result = true;
                }
            }
            else if (gamemode == retail)
            {
                if (sigil)
                {
                    if (!(gameepisode == 5 && gamemap == 8))
                    {
                        mapcmdepisode = 5;
                        mapcmdmap = 8;
                        M_StringCopy(mapcmdlump, "E5M8", sizeof(mapcmdlump));
                        result = true;
                    }
                }
                else if (!(gameepisode == 4 && gamemap == 8))
                {
                    mapcmdepisode = 4;
                    mapcmdmap = 8;
                    M_StringCopy(mapcmdlump, "E4M8", sizeof(mapcmdlump));
                    result = true;
                }
            }
            else
            {
                if (!(gameepisode == 3 && gamemap == 8))
                {
                    mapcmdepisode = 3;
                    mapcmdmap = 8;
                    M_StringCopy(mapcmdlump, "E3M8", sizeof(mapcmdlump));
                    result = true;
                }
            }
        }
        else if (M_StringCompare(parm, "random"))
        {
            if (gamemode == commercial)
            {
                mapcmdepisode = gameepisode;
                mapcmdmap = M_RandomIntNoRepeat(1, (gamemission == pack_nerve ? 8 : 30), gamemap);
                M_snprintf(mapcmdlump, sizeof(mapcmdlump), "MAP%02i", mapcmdmap);
                result = true;
            }
            else
            {
                mapcmdepisode = (gamemode == shareware || chex ? 1 :
                    M_RandomIntNoRepeat(1, (gamemode == retail ? (sigil ? 5 : 4) : 3), gameepisode));
                mapcmdmap = M_RandomIntNoRepeat(1, (chex ? 5 : 8), gamemap);
                M_snprintf(mapcmdlump, sizeof(mapcmdlump), "E%iM%i", mapcmdepisode, mapcmdmap);
                result = true;
            }
        }
        else if (M_StringCompare(parm, "E1M4B") && gamemission == doom && gamemode != shareware && !chex)
        {
            mapcmdepisode = 1;
            mapcmdmap = 4;
            M_StringCopy(speciallumpname, "E1M4B", sizeof(speciallumpname));
            M_StringCopy(mapcmdlump, "E1M4B", sizeof(mapcmdlump));
            result = true;
        }
        else if (M_StringCompare(parm, "E1M8B") && gamemission == doom && gamemode != shareware && !chex)
        {
            mapcmdepisode = 1;
            mapcmdmap = 8;
            M_StringCopy(speciallumpname, "E1M8B", sizeof(speciallumpname));
            M_StringCopy(mapcmdlump, "E1M8B", sizeof(mapcmdlump));
            result = true;
        }
        else
        {
            M_StringCopy(mapcmdlump, parm, sizeof(mapcmdlump));

            if (gamemode == commercial)
            {
                mapcmdepisode = 1;

                if (sscanf(parm, "MAP0%1i", &mapcmdmap) == 1 || sscanf(parm, "MAP%2i", &mapcmdmap) == 1)
                {
                    if (!((BTSX && W_CheckMultipleLumps(parm) == 1) || (gamemission == pack_nerve && mapcmdmap > 9)))
                    {
                        if (gamestate != GS_LEVEL && gamemission == pack_nerve)
                        {
                            gamemission = doom2;
                            expansion = 1;
                        }

                        result = (W_CheckNumForName(parm) >= 0);
                    }
                }
                else if (BTSX)
                {
                    if (sscanf(parm, "MAP%02iC", &mapcmdmap) == 1)
                        result = (W_CheckNumForName(parm) >= 0);
                    else
                    {
                        if (sscanf(parm, "E%1iM0%1i", &mapcmdepisode, &mapcmdmap) != 2)
                            sscanf(parm, "E%1iM%2i", &mapcmdepisode, &mapcmdmap);

                        if (mapcmdmap && ((mapcmdepisode == 1 && BTSXE1) || (mapcmdepisode == 2 && BTSXE2)
                            || (mapcmdepisode == 3 && BTSXE3)))
                        {
                            char    lump[6];

                            M_snprintf(lump, sizeof(lump), "MAP%02i", mapcmdmap);
                            result = (W_CheckMultipleLumps(lump) == 2);
                        }
                    }
                }
            }
            else if (sscanf(parm, "E%1iM%1i", &mapcmdepisode, &mapcmdmap) == 2)
                result = (chex && mapcmdepisode > 1 ? false : (W_CheckNumForName(parm) >= 0));
            else if (FREEDOOM && sscanf(parm, "C%1iM%1i", &mapcmdepisode, &mapcmdmap) == 2)
            {
                char    lump[5];

                M_snprintf(lump, sizeof(lump), "E%iM%i", mapcmdepisode, mapcmdmap);
                result = (W_CheckNumForName(lump) >= 0);
            }
        }

        if (!result)
        {
            for (int i = 0; i < numlumps; i++)
            {
                char        wadname[MAX_PATH];
                dboolean    replaced;
                dboolean    pwad;
                char        mapinfoname[128];
                char        *temp2 = uppercase(lumpinfo[i]->name);

                M_StringCopy(mapcmdlump, temp2, sizeof(mapcmdlump));
                free(temp2);

                mapcmdepisode = -1;
                mapcmdmap = -1;

                if (gamemode == commercial)
                {
                    mapcmdepisode = 1;
                    sscanf(mapcmdlump, "MAP0%1i", &mapcmdmap);

                    if (mapcmdmap == -1)
                        sscanf(mapcmdlump, "MAP%2i", &mapcmdmap);
                }
                else
                {
                    sscanf(mapcmdlump, "E%1iM%1iB", &mapcmdepisode, &mapcmdmap);

                    if (gamemode != shareware && strlen(mapcmdlump) == 5 && mapcmdepisode != -1 && mapcmdmap != -1)
                        M_StringCopy(speciallumpname, mapcmdlump, sizeof(speciallumpname));
                    else
                        sscanf(mapcmdlump, "E%1iM%1i", &mapcmdepisode, &mapcmdmap);
                }

                if (mapcmdepisode == -1 || mapcmdmap == -1)
                    continue;

                M_StringCopy(wadname, leafname(lumpinfo[i]->wadfile->path), sizeof(wadname));
                replaced = (W_CheckMultipleLumps(mapcmdlump) > 1 && !chex && !FREEDOOM);
                pwad = (lumpinfo[i]->wadfile->type == PWAD);
                M_StringCopy(mapinfoname, P_GetMapName((mapcmdepisode - 1) * 10 + mapcmdmap), sizeof(mapinfoname));

                switch (gamemission)
                {
                    case doom:
                        if (!replaced || pwad)
                        {
                            temp2 = removenonalpha(*mapinfoname ? mapinfoname : *mapnames[(mapcmdepisode - 1) * 9 + mapcmdmap - 1]);

                            if (M_StringCompare(parm, temp2))
                                result = true;

                            free(temp2);
                        }

                        break;

                    case doom2:
                        if ((!M_StringCompare(wadname, "NERVE.WAD") && ((!replaced || pwad || nerve) && (pwad || !BTSX))) || hacx)
                        {
                            if (BTSX)
                            {
                                if (!M_StringCompare(wadname, "DOOM2.WAD"))
                                {
                                    temp2 = removenonalpha(*mapnames2[mapcmdmap - 1]);

                                    if (M_StringCompare(parm, temp2))
                                        result = true;

                                    free(temp2);
                                }
                            }
                            else
                            {
                                temp2 = removenonalpha(*mapinfoname ? mapinfoname :
                                    (bfgedition ? *mapnames2_bfg[mapcmdmap - 1] : *mapnames2[mapcmdmap - 1]));

                                if (M_StringCompare(parm, temp2))
                                    result = true;

                                free(temp2);
                            }
                        }

                        break;

                    case pack_nerve:
                        if (M_StringCompare(wadname, "NERVE.WAD"))
                        {
                            temp2 = removenonalpha(*mapnamesn[mapcmdmap - 1]);

                            if (M_StringCompare(parm, temp2))
                                result = true;

                            free(temp2);
                        }

                        break;

                    case pack_plut:
                        if (!replaced || pwad)
                        {
                            temp2 = removenonalpha(*mapnamesp[mapcmdmap - 1]);

                            if (M_StringCompare(parm, temp2))
                                result = true;

                            free(temp2);
                        }

                        break;

                    case pack_tnt:
                        if (!replaced || pwad)
                        {
                            temp2 = removenonalpha(*mapnamest[mapcmdmap - 1]);

                            if (M_StringCompare(parm, temp2))
                                result = true;

                            free(temp2);
                        }

                        break;

                    default:
                        break;
                }

                if (result)
                    break;
            }

        }

        free(temp1);
        free(parm);

        return result;
    }
}

static void map_cmd_func2(char *cmd, char *parms)
{
    char    buffer[1024];

    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, (gamemission == doom ? MAPCMDFORMAT1 : MAPCMDFORMAT2));
        return;
    }

    M_snprintf(buffer, sizeof(buffer), (M_StringCompare(mapcmdlump, mapnum) ? s_STSTR_CLEVSAME : s_STSTR_CLEV), mapcmdlump);
    C_Output(buffer);
    HU_SetPlayerMessage(buffer, false, false);
    message_dontfuckwithme = true;
    samelevel = (gameepisode == mapcmdepisode && gamemap == mapcmdmap);

    gameepisode = mapcmdepisode;

    if (gamemission == doom)
    {
        episode = gameepisode;
        EpiDef.lastOn = episode - 1;
    }

    gamemap = mapcmdmap;

    if (gamestate == GS_LEVEL)
    {
        idclevtics = MAPCHANGETICS;
        drawdisk = true;
        C_HideConsole();
    }
    else
    {
        G_DeferredInitNew(skilllevel - 1, gameepisode, gamemap);
        C_HideConsoleFast();
    }

    viewplayer->cheated++;
    stat_cheated = SafeAdd(stat_cheated, 1);
    M_SaveCVARs();
}

//
// maplist CCMD
//
static void removemapnum(char *title)
{
    char *pos = strchr(title, ':');

    if (pos)
    {
        int index = (int)(pos - title) + 1;

        memmove(title, title + index, strlen(title) - index + 1);

        if (title[0] == ' ')
            memmove(title, title + 1, strlen(title));
    }
}

static void maplist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[4] = { 40, 93, 370, 0 };
    int         count = 0;
    char        (*maps)[256] = malloc(numlumps * sizeof(char *));
    dboolean    mapfound[50] = { false };

    C_Header(tabs, maplist, MAPLISTHEADER);

    // search through lumps for maps
    for (int i = numlumps - 1; i >= 0; i--)
    {
        int         ep = -1;
        int         map = -1;
        char        lump[9];
        char        wadname[MAX_PATH];
        dboolean    replaced;
        dboolean    pwad;
        char        mapinfoname[128];
        char        *temp = uppercase(lumpinfo[i]->name);

        M_StringCopy(lump, temp, sizeof(lump));
        free(temp);

        speciallumpname[0] = '\0';

        if (gamemode == commercial)
        {
            ep = 1;
            sscanf(lump, "MAP0%1i", &map);

            if (map == -1)
                sscanf(lump, "MAP%2i", &map);
        }
        else
        {
            sscanf(lump, "E%1iM%1iB", &ep, &map);

            if (gamemode != shareware && strlen(lump) == 5 && ep != -1 && map != -1)
                M_StringCopy(speciallumpname, lump, sizeof(speciallumpname));
            else
                sscanf(lump, "E%1iM%1i", &ep, &map);
        }

        if (ep-- == -1 || map-- == -1 || mapfound[ep * 10 + map + 1])
            continue;

        if (!*speciallumpname)
            mapfound[ep * 10 + map + 1] = true;

        M_StringCopy(wadname, leafname(lumpinfo[i]->wadfile->path), sizeof(wadname));
        replaced = (W_CheckMultipleLumps(lump) > 1 && !chex && !FREEDOOM);
        pwad = (lumpinfo[i]->wadfile->type == PWAD);
        M_StringCopy(mapinfoname, P_GetMapName(ep * 10 + map + 1), sizeof(mapinfoname));
        speciallumpname[0] = '\0';

        switch (gamemission)
        {
            case doom:
                if (!replaced || pwad)
                {
                    temp = titlecase(*mapinfoname ? mapinfoname : *mapnames[ep * 9 + map]);
                    removemapnum(temp);
                    M_snprintf(maps[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump,
                        (replaced && dehcount == 1 && !*mapinfoname ? "-" : temp), wadname);
                    free(temp);
                }

                break;

            case doom2:
                if ((!M_StringCompare(wadname, "NERVE.WAD") && (!replaced || pwad || nerve)) || hacx)
                {
                    if (BTSX)
                    {
                        if (!M_StringCompare(wadname, "DOOM2.WAD"))
                        {
                            temp = titlecase(M_StringReplace(*mapnames2[map], ": ", "\t<i><b>"));
                            removemapnum(temp);
                            M_snprintf(maps[count++], 256, "%s</b></i>\t%s", temp, wadname);
                            free(temp);
                        }
                    }
                    else
                    {
                        temp = titlecase(*mapinfoname ? mapinfoname : (bfgedition ? *mapnames2_bfg[map] : *mapnames2[map]));
                        removemapnum(temp);
                        M_snprintf(maps[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump,
                            (replaced && dehcount == 1 && !nerve && !*mapinfoname ? "-" : temp), wadname);
                        free(temp);
                    }
                }

                break;

            case pack_nerve:
                if (M_StringCompare(wadname, "NERVE.WAD"))
                {
                    temp = titlecase(*mapinfoname ? mapinfoname : *mapnamesn[map]);
                    removemapnum(temp);
                    M_snprintf(maps[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump, temp, wadname);
                    free(temp);
                }

                break;

            case pack_plut:
                if (!replaced || pwad)
                {
                    temp = titlecase(*mapinfoname ? mapinfoname : *mapnamesp[map]);
                    removemapnum(temp);
                    M_snprintf(maps[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump,
                        (replaced && dehcount == 1 && !*mapinfoname ? "-" : temp), wadname);
                    free(temp);
                }

                break;

            case pack_tnt:
                if (!replaced || pwad)
                {
                    temp = titlecase(*mapinfoname ? mapinfoname : *mapnamest[map]);
                    removemapnum(temp);
                    M_snprintf(maps[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump,
                        (replaced && dehcount == 1 && !*mapinfoname ? "-" : temp), wadname);
                    free(temp);
                }

                break;

            default:
                break;
        }
    }

    // sort the map list
    for (int i = 0; i < count; i++)
        for (int j = i + 1; j < count; j++)
            if (strcmp(maps[i], maps[j]) > 0)
            {
                char    temp[256];

                M_StringCopy(temp, maps[i], sizeof(temp));
                M_StringCopy(maps[i], maps[j], sizeof(maps[i]));
                M_StringCopy(maps[j], temp, sizeof(maps[j]));
            }

    // display the map list
    for (int i = 0; i < count; i++)
        C_TabbedOutput(tabs, "%i.\t%s", i + 1, maps[i]);

    free(maps);
}

//
// mapstats CCMD
//
#define AA      "Andre Arsenault"
#define AD      "Andrew Dowswell"
#define AI      "Arya Iwakura"
#define AM      "American McGee"
#define BK      "Brian Kidby"
#define CB      "Christopher Buteau"
#define DB      "David Blanshine"
#define DC      "Dario Casali"
#define DC2     "David Calvin"
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
#define RM      "Russell Meakim"
#define RP      "Robin Patenall"
#define SG      "Shawn Green"
#define SP      "Sandy Petersen"
#define TH      "Tom Hall"
#define TH2     "Ty Halderman"
#define TM      "Tom Mustaine"
#define TW      "Tim Willits"
#define WW      "William D. Whitaker"
#define AMSP    AM " and " SP
#define BKTH2   BK " and " TH2
#define DC2DB   DC2 " and " DB
#define DCMC    DC " and " MC
#define DCTH2   DC " and " TH2
#define JRTH    JR " and " TH
#define JSTH2   JS " and " TH2
#define MSJL    MS " and " JL
#define RPJM2   RP " and " JM2
#define SPTH    SP " and " TH

static void mapstats_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[4] = { 120, 240, 0, 0 };
    char        *temp;

    C_Header(tabs, mapstats, MAPSTATSHEADER);

    if (gamemode == commercial)
    {
        if (gamemission == pack_nerve)
        {
            temp = titlecase(*expansions[1]);
            C_TabbedOutput(tabs, "Expansion\t<b><i>%s</i> (2 of 2)</b>", temp);
            free(temp);

            C_TabbedOutput(tabs, "Map\t<b>%i of %i%s</b>", gamemap, (gamemap == 9 ? 9 : 8), (gamemap == 9 ? " (secret)" : ""));
        }
        else if (nerve)
        {
            temp = titlecase(*expansions[0]);
            C_TabbedOutput(tabs, "Expansion\t<b><i>%s</i> (1 of 2)</b>", temp);
            free(temp);

            C_TabbedOutput(tabs, "Map\t<b>%i of %i%s</b>", gamemap, (gamemap >= 31 ? gamemap : 30), (gamemap >= 31 ? " (secret)" : ""));
        }
        else
            C_TabbedOutput(tabs, "Map\t<b>%i of %i%s</b>", gamemap, (gamemap >= 31 ? gamemap : 30), (gamemap >= 31 ? " (secret)" : ""));
    }
    else
    {
        temp = titlecase(*episodes[gameepisode - 1]);
        C_TabbedOutput(tabs, "Episode\t<b><i>%s</i> (%i of %i)</b>", temp, gameepisode, (gamemode == retail ? (sigil ? 5 : 4) : 3));
        free(temp);

        C_TabbedOutput(tabs, "Map\t<b>%i of %i%s</b>", gamemap, (gamemap == 9 ? 9 : 8), (gamemap == 9 ? " (secret)" : ""));
    }

    if (!M_StringCompare(maptitle, mapnum))
    {
        temp = titlecase(maptitle);
        C_TabbedOutput(tabs, "Title\t<b><i>%s</i></b>", temp);
        free(temp);

        if (gamemode == commercial)
        {
            if (gamemap == 11)
            {
                if (M_StringCompare(maptitle, s_HUSTR_11))
                    C_TabbedOutput(tabs, "Alternative Title\t<b><i>%s</i></b>", s_HUSTR_11_ALT);
            }
            else if (gamemap == 31)
            {
                if (M_StringCompare(maptitle, s_HUSTR_31))
                    C_TabbedOutput(tabs, "Alternative Title\t<b><i>%s</i></b>", s_HUSTR_31_BFG);
                else if (M_StringCompare(maptitle, s_HUSTR_31_BFG))
                    C_TabbedOutput(tabs, "Alternative Title\t<b><i>%s</i></b>", s_HUSTR_31);
            }
            else if (gamemap == 32)
            {
                if (M_StringCompare(maptitle, s_HUSTR_32))
                    C_TabbedOutput(tabs, "Alternative Title\t<b><i>%s</i></b>", s_HUSTR_32_BFG);
                else if (M_StringCompare(maptitle, s_HUSTR_32_BFG))
                    C_TabbedOutput(tabs, "Alternative Title\t<b><i>%s</i></b>", s_HUSTR_32);
            }
        }
        else if (gameepisode == 3 && gamemap == 7)
        {
            if (M_StringCompare(maptitle, s_HUSTR_E3M7))
                C_TabbedOutput(tabs, "Alternate Title\t<b><i>%s</i></b>", s_HUSTR_E3M7_ALT);
        }
    }

    {
        const char *authors[][6] =
        {
            /* xy      doom   doom2 tnt    plut  nerve */
            /* 00 */ { "",    "",   "",    DCMC, "" },
            /* 01 */ { "",    SP,   TM,    DCMC, RM },
            /* 02 */ { "",    AM,   JW,    DCMC, AI },
            /* 03 */ { "",    AM,   RPJM2, DCMC, RM },
            /* 04 */ { "",    AM,   TH2,   DCMC, RM },
            /* 05 */ { "",    AM,   JD,    DCMC, AI },
            /* 06 */ { "",    AM,   JSTH2, DCMC, AI },
            /* 07 */ { "",    AMSP, AD,    DCMC, AI },
            /* 08 */ { "",    SP,   JM2,   DCMC, AI },
            /* 09 */ { "",    SP,   JSTH2, DCMC, RM },
            /* 10 */ { "",    SPTH, TM,    DCMC, "" },
            /* 11 */ { JR,    JR,   DJ,    DCMC, "" },
            /* 12 */ { JR,    SP,   JL,    DCMC, "" },
            /* 13 */ { JR,    SP,   BKTH2, DCMC, "" },
            /* 14 */ { JRTH,  AM,   RP,    DCMC, "" },
            /* 15 */ { JR,    JR,   WW,    DCMC, "" },
            /* 16 */ { JR,    SP,   AA,    DCMC, "" },
            /* 17 */ { JR,    JR,   TM,    DCMC, "" },
            /* 18 */ { SPTH,  SP,   DCTH2, DCMC, "" },
            /* 19 */ { JR,    SP,   TH2,   DCMC, "" },
            /* 20 */ { DC2DB, JR,   DO,    DCMC, "" },
            /* 21 */ { SPTH,  SP,   DO,    DCMC, "" },
            /* 22 */ { SPTH,  AM,   CB,    DCMC, "" },
            /* 23 */ { SPTH,  SP,   PT,    DCMC, "" },
            /* 24 */ { SPTH,  SP,   DJ,    DCMC, "" },
            /* 25 */ { SP,    SG,   JM,    DCMC, "" },
            /* 26 */ { SP,    JR,   MSJL,  DCMC, "" },
            /* 27 */ { SPTH,  SP,   DO,    DCMC, "" },
            /* 28 */ { SP,    SP,   MC,    DCMC, "" },
            /* 29 */ { SP,    JR,   JS,    DCMC, "" },
            /* 30 */ { "",    SP,   JS,    DCMC, "" },
            /* 31 */ { SP,    SP,   DC,    DCMC, "" },
            /* 32 */ { SP,    SP,   DC,    DCMC, "" },
            /* 33 */ { SPTH,  MB,   "",    "",   "" },
            /* 34 */ { SP,    "",   "",    "",   "" },
            /* 35 */ { SP,    "",   "",    "",   "" },
            /* 36 */ { SP,    "",   "",    "",   "" },
            /* 37 */ { SPTH,  "",   "",    "",   "" },
            /* 38 */ { SP,    "",   "",    "",   "" },
            /* 39 */ { SP,    "",   "",    "",   "" },
            /* 40 */ { "",    "",   "",    "",   "" },
            /* 41 */ { AM,    "",   "",    "",   "" },
            /* 42 */ { JR,    "",   "",    "",   "" },
            /* 43 */ { SG,    "",   "",    "",   "" },
            /* 44 */ { AM,    "",   "",    "",   "" },
            /* 45 */ { TW,    "",   "",    "",   "" },
            /* 46 */ { JR,    "",   "",    "",   "" },
            /* 47 */ { JA,    "",   "",    "",   "" },
            /* 48 */ { SG,    "",   "",    "",   "" },
            /* 49 */ { TW,    "",   "",    "",   "" },
            /* 50 */ { "",    "",   "",    "",   "" },
            /* 51 */ { JR,    "",   "",    "",   "" },
            /* 52 */ { JR,    "",   "",    "",   "" },
            /* 53 */ { JR,    "",   "",    "",   "" },
            /* 54 */ { JR,    "",   "",    "",   "" },
            /* 55 */ { JR,    "",   "",    "",   "" },
            /* 56 */ { JR,    "",   "",    "",   "" },
            /* 57 */ { JR,    "",   "",    "",   "" },
            /* 58 */ { JR,    "",   "",    "",   "" },
            /* 59 */ { JR,    "",   "",    "",   "" }
        };

        const int   i = (gamemission == doom ? gameepisode * 10 : 0) + gamemap;
        const char  *author = P_GetMapAuthor(i);

        if (*author)
            C_TabbedOutput(tabs, "Author\t<b>%s</b>", author);
        else if (canmodify && *authors[i][gamemission])
            C_TabbedOutput(tabs, "Author\t<b>%s</b>", authors[i][gamemission]);
    }

    {
        int     i = (nerve && gamemission == doom2 ? W_GetLastNumForName(mapnum) : W_CheckNumForName(mapnum));
        char    wadname[MAX_PATH];
        int     wadtype = lumpinfo[i]->wadfile->type;

        M_StringCopy(wadname, leafname(lumpinfo[i]->wadfile->path), sizeof(wadname));

        C_TabbedOutput(tabs, "%s\t<b>%s%s</b>", (wadtype == IWAD ? "IWAD" : "PWAD"), wadname,
            (wadtype == IWAD && bfgedition ? " <i>(BFG Edition)</i>" : ""));

        if (M_StringCompare(wadname, "DOOM.WAD"))
        {
            if (bfgedition)
                C_TabbedOutput(tabs, "Release date\t<b>October 16, 2012</b>");
            else if (gameepisode == 4)
                C_TabbedOutput(tabs, "Release date\t<b>April 30, 1995</b>");
            else
                C_TabbedOutput(tabs, "Release date\t<b>December 10, 1993</b>");
        }
        else if (M_StringCompare(wadname, "SIGIL_v1_21.wad")
            || M_StringCompare(wadname, "SIGIL_v1_2.wad")
            || M_StringCompare(wadname, "SIGIL_v1_1.wad")
            || M_StringCompare(wadname, "SIGIL.wad"))
            C_TabbedOutput(tabs, "Release date\t<b>May 22, 2019</b>");
        else if (M_StringCompare(wadname, "DOOM2.WAD"))
        {
            if (bfgedition)
                C_TabbedOutput(tabs, "Release date\t<b>October 16, 2012</b>");
            else
                C_TabbedOutput(tabs, "Release date\t<b>September 30, 1994</b>");
        }
        else if (M_StringCompare(wadname, "NERVE.WAD"))
            C_TabbedOutput(tabs, "Release date\t<b>May 26, 2010</b>");
        else if (M_StringCompare(wadname, "PLUTONIA.WAD") || M_StringCompare(wadname, "TNT.WAD"))
            C_TabbedOutput(tabs, "Release date\t<b>June 17, 1996</b>");

        if (wadtype == PWAD)
            C_TabbedOutput(tabs, "IWAD\t<b>%s%s</b>", leafname(lumpinfo[W_GetLastNumForName("PLAYPAL")]->wadfile->path),
                (bfgedition ? " <i>(BFG Edition)</i>" : ""));
    }

    C_TabbedOutput(tabs, "Compatibility\t<b>%s</b>",
        (mbfcompatible ? "<i>MBF</i>" : (boomcompatible ? "<i>BOOM</i>" : (numsegs < 32768 ? "Vanilla" : "Limit removing"))));

    {
        int partime = G_GetParTime();

        if (partime)
            C_TabbedOutput(tabs, "Par time\t<b>%02i:%02i</b>", partime / 60, partime % 60);
    }

    temp = commify(numspawnedthings);
    C_TabbedOutput(tabs, "Things\t<b>%s</b>", temp);
    free(temp);

    temp = commify(totalkills);
    C_TabbedOutput(tabs, "   Monsters\t<b>%s</b>", temp);
    free(temp);

    temp = commify(totalpickups);
    C_TabbedOutput(tabs, "   Pickups\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numdecorations);
    C_TabbedOutput(tabs, "   Decorations\t<b>%s</b>", temp);
    free(temp);

    temp = commify(barrelcount);
    C_TabbedOutput(tabs, "   Barrels\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numlines);
    C_TabbedOutput(tabs, "Linedefs\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numsides);
    C_TabbedOutput(tabs, "Sidedefs\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numvertexes);
    C_TabbedOutput(tabs, "Vertexes\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numsegs);
    C_TabbedOutput(tabs, "Segments\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numsubsectors);
    C_TabbedOutput(tabs, "Subsectors\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numnodes);
    C_TabbedOutput(tabs, "Nodes\t<b>%s</b>", temp);
    free(temp);

    C_TabbedOutput(tabs, "   Format\t<b>%s</b>", mapformats[mapformat]);

    temp = commify(numsectors);
    C_TabbedOutput(tabs, "Sectors\t<b>%s</b>", temp);
    free(temp);

    {
        int outside = 0;

        for (int i = 0; i < numsectors; i++)
        {
            short   picnum = sectors[i].ceilingpic;

            if (picnum == skyflatnum || (picnum & PL_SKYFLAT))
                outside++;
        }

        outside = outside * 100 / numsectors;
        C_TabbedOutput(tabs, "   Inside/outside\t<b>%i%%/%i%%</b>", 100 - outside, outside);
    }

    temp = commify(totalsecret);
    C_TabbedOutput(tabs, "   Secret\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numliquid);
    C_TabbedOutput(tabs, "   Liquid\t<b>%s</b>", temp);
    free(temp);

    temp = commify(numdamaging);
    C_TabbedOutput(tabs, "   Damaging\t<b>%s</b>", temp);
    free(temp);

    if (blockmaprebuilt)
        C_TabbedOutput(tabs, "Blockmap\t<b>Rebuilt</b>");

    {
        int min_x = INT_MAX;
        int max_x = INT_MIN;
        int min_y = INT_MAX;
        int max_y = INT_MIN;
        int max_c = INT_MIN;
        int min_f = INT_MAX;
        int width;
        int height;
        int depth;

        for (int i = 0; i < numvertexes; i++)
        {
            const fixed_t   x = vertexes[i].x;
            const fixed_t   y = vertexes[i].y;

            if (x < min_x)
                min_x = x;
            else if (x > max_x)
                max_x = x;

            if (y < min_y)
                min_y = y;
            else if (y > max_y)
                max_y = y;
        }

        width = ((max_x >> FRACBITS) - (min_x >> FRACBITS)) / UNITSPERFOOT;
        height = ((max_y >> FRACBITS) - (min_y >> FRACBITS)) / UNITSPERFOOT;

        for (int i = 0; i < numsectors; i++)
        {
            if (max_c < sectors[i].ceilingheight)
                max_c = sectors[i].ceilingheight;

            if (min_f > sectors[i].floorheight)
                min_f = sectors[i].floorheight;
        }

        depth = ((max_c >> FRACBITS) - (min_f >> FRACBITS)) / UNITSPERFOOT;

        if (units == units_metric)
        {
            const float metricwidth = width / FEETPERMETER;
            const float metricheight = height / FEETPERMETER;
            const float metricdepth = depth / FEETPERMETER;

            if (metricwidth < METERSPERKILOMETER && metricheight < METERSPERKILOMETER && metricdepth < METERSPERKILOMETER)
            {
                char    *temp1 = striptrailingzero(metricwidth, 1);
                char    *temp2 = striptrailingzero(metricheight, 1);
                char    *temp3 = striptrailingzero(metricdepth, 1);

                C_TabbedOutput(tabs, "Dimensions\t<b>%sx%sx%s meters</b>", temp1, temp2, temp3);
                free(temp1);
                free(temp2);
                free(temp3);
            }
            else
            {
                char    *temp1 = striptrailingzero(metricwidth / METERSPERKILOMETER, 2);
                char    *temp2 = striptrailingzero(metricheight / METERSPERKILOMETER, 2);
                char    *temp3 = striptrailingzero(metricdepth / METERSPERKILOMETER, 2);

                C_TabbedOutput(tabs, "Dimensions\t<b>%sx%sx%s kilometers</b>", temp1, temp2, temp3);
                free(temp1);
                free(temp2);
                free(temp3);
            }
        }
        else
        {
            if (width < FEETPERMILE && height < FEETPERMILE)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);
                char    *temp3 = commify(depth);

                C_TabbedOutput(tabs, "Dimensions\t<b>%sx%sx%s feet</b>", temp1, temp2, temp3);
                free(temp1);
                free(temp2);
                free(temp3);
            }
            else
            {
                char    *temp1 = striptrailingzero((float)width / FEETPERMILE, 2);
                char    *temp2 = striptrailingzero((float)height / FEETPERMILE, 2);
                char    *temp3 = striptrailingzero((float)depth / FEETPERMILE, 2);

                C_TabbedOutput(tabs, "Dimensions\t<b>%sx%sx%s miles</b>", temp1, temp2, temp3);
                free(temp1);
                free(temp2);
                free(temp3);
            }
        }
    }

    if (mus_playing && !nomusic)
    {
        char                lumpname[9];
        int                 lumps;
        const char          *musiccomposer = P_GetMapMusicComposer((gameepisode - 1) * 10 + gamemap);
        const char          *musictitle = P_GetMapMusicTitle((gameepisode - 1) * 10 + gamemap);
        const Mix_MusicType musictype = Mix_GetMusicType(NULL);

        M_snprintf(lumpname, sizeof(lumpname), "d_%s", mus_playing->name1);
        temp = uppercase(lumpname);
        lumps = W_CheckMultipleLumps(lumpname);

        C_TabbedOutput(tabs, "Music lump\t<b>%s%s</b>",
            temp, ((((gamemode == commercial || gameepisode > 1) && lumps == 1)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2)) ? "" : " (replaced by lump in PWAD)"));
        free(temp);

        if (*musictitle)
            C_TabbedOutput(tabs, "   Title\t<b><i>%s</i></b>", musictitle);
        else if (sigil && gameepisode == 5)
            C_TabbedOutput(tabs, "   Title\t<b><i>%s</i></b>", (buckethead ? mus_playing->title2 : mus_playing->title1));
        else if (((gamemode == commercial || gameepisode > 1) && lumps == 1)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2))
            C_TabbedOutput(tabs, "   Title\t<b><i>%s</i></b>", mus_playing->title1);

        if (*musiccomposer)
            C_TabbedOutput(tabs, "   Composer\t<b>%s</b>", musiccomposer);
        else if (sigil && gameepisode == 5)
            C_TabbedOutput(tabs, "   Composer\t<b>%s</b>", (buckethead ? "Buckethead" : "James Paddock"));
        else if (((gamemode == commercial || gameepisode > 1) && lumps == 1)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2))
            C_TabbedOutput(tabs, "   Composer\t<b>%s</b>", "Bobby Prince");

        if (musmusictype)
            C_TabbedOutput(tabs, "   Format\t<b>MUS</b>");
        else if (midimusictype || musictype == MUS_MID)
            C_TabbedOutput(tabs, "   Format\t<b>MIDI</b>");
        else if (musictype == MUS_OGG)
            C_TabbedOutput(tabs, "   Format\t<b>Ogg Vorbis</b>");
        else if (musictype == MUS_MP3)
            C_TabbedOutput(tabs, "   Format\t<b>MP3</b>");
        else if (musictype == MUS_WAV)
            C_TabbedOutput(tabs, "   Format\t<b>WAV</b>");
        else if (musictype == MUS_FLAC)
            C_TabbedOutput(tabs, "   Format\t<b>FLAC</b>");
        else if (musictype == MUS_MOD)
            C_TabbedOutput(tabs, "   Format\t<b>MOD</b>");
    }
}

//
// name CCMD
//
static dboolean namecmdfriendly;
static dboolean namecmdanymonster;
static char     namecmdnew[128];
static char     namecmdold[128];
static int      namecmdtype = NUMMOBJTYPES;

static dboolean name_cmd_func1(char *cmd, char *parms)
{
    char    *parm = M_StringDuplicate(parms);

    if (!*parm)
        return true;

    if (M_StringStartsWith(parm, "player"))
    {
        M_StringCopy(namecmdold, "player", sizeof(namecmdold));
        strreplace(parm, "player", "");
        M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
        return (namecmdnew[0] != '\0' && strlen(namecmdnew) < 33);
    }

    if (gamestate == GS_LEVEL)
    {
        if ((namecmdfriendly = M_StringStartsWith(parm, "friendly ")))
            strreplace(parm, "friendly ", "");
        else if ((namecmdfriendly = M_StringStartsWith(parm, "friendly")))
            strreplace(parm, "friendly", "");

        if (M_StringStartsWith(parm, "monster"))
        {
            M_StringCopy(namecmdold, "monster", sizeof(namecmdold));
            strreplace(parm, "monster", "");
            M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
            namecmdanymonster = true;
            return (namecmdnew[0] != '\0' && strlen(namecmdnew) < 33);
        }
        else
            namecmdanymonster = false;

        for (int i = 0; i < NUMMOBJTYPES; i++)
            if ((mobjinfo[i].flags & MF_SHOOTABLE) && i != MT_PLAYER && i != MT_BARREL)
            {
                dboolean    result = false;
                char        *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
                char        *temp2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char        *temp3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);

                if (*mobjinfo[i].name1 && M_StringStartsWith(parm, temp1))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name1, sizeof(namecmdold));
                    strreplace(parm, temp1, "");
                    M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
                    namecmdtype = i;
                    result = true;
                }
                else if (*mobjinfo[i].name2 && M_StringStartsWith(parm, temp2))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name2, sizeof(namecmdold));
                    strreplace(parm, temp2, "");
                    M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
                    namecmdtype = i;
                    result = true;
                }
                else if (*mobjinfo[i].name3 && M_StringStartsWith(parm, temp3))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name3, sizeof(namecmdold));
                    strreplace(parm, temp3, "");
                    M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
                    namecmdtype = i;
                    result = true;
                }

                if (temp1)
                    free(temp1);

                if (temp2)
                    free(temp2);

                if (temp3)
                    free(temp3);

                if (result)
                    return (namecmdnew[0] != '\0' && strlen(namecmdnew) < 33);
            }
    }

    return false;
}

static void name_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, NAMECMDFORMAT);
    }
    else if (M_StringCompare(namecmdold, "player"))
    {
        C_Output("The player has been %s %s.", (M_StringCompare(playername, playername_default) ? "named" : "renamed"), namecmdnew);
        M_StripQuotes(namecmdnew);
        playername = M_StringDuplicate(namecmdnew);
        M_SaveCVARs();
    }
    else
    {
        mobj_t  *bestmobj = NULL;
        fixed_t bestdist = FIXED_MAX;

        for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
        {
            mobj_t      *mobj = (mobj_t *)th;
            int         flags = mobj->flags;
            mobjtype_t  type = mobj->type;

            if (((namecmdanymonster && (flags & MF_SHOOTABLE) && type != MT_BARREL && type != MT_PLAYER) || type == namecmdtype)
                && ((namecmdfriendly && (flags & MF_FRIEND)) || !namecmdfriendly)
                && mobj->health > 0
                && P_CheckSight(viewplayer->mo, mobj))
            {
                fixed_t dist = P_ApproxDistance(mobj->x - viewx, mobj->y - viewy);

                if (dist < bestdist)
                {
                    bestdist = dist;
                    bestmobj = mobj;

                    if (namecmdanymonster)
                        M_StringCopy(namecmdold, mobj->info->name1, sizeof(namecmdold));
                }
            }
        }

        if (bestmobj)
        {
            M_StripQuotes(namecmdnew);

            if (monstercount[bestmobj->type] == 1)
                C_Output("The %s%s has been %s %s.",
                    (namecmdfriendly ? "friendly " : ""), namecmdold, (*bestmobj->name ? "renamed" : "named"), namecmdnew);
            else
                C_Output("The %s%s nearest to %s has been %s %s.",
                    (namecmdfriendly ? "friendly " : ""), namecmdold, playername, (*bestmobj->name ? "renamed" : "named"), namecmdnew);

            M_StringCopy(bestmobj->name, namecmdnew, sizeof(bestmobj->name));
        }
        else
            C_Warning(0, "%s %s%s couldn't be found nearby.",
                (isvowel(namecmdold[0]) ? "An" : "A"), (namecmdfriendly ? "friendly " : ""), namecmdold);
    }
}

//
// newgame CCMD
//
static void newgame_cmd_func2(char *cmd, char *parms)
{
    C_HideConsoleFast();

    if (viewplayer)
        viewplayer->cheats = 0;

    G_DeferredInitNew((skill_t)(skilllevel - 1), (gamemode == commercial ? expansion : episode), 1);
}

//
// noclip CCMD
//
static void noclip_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && (viewplayer->cheats & CF_NOCLIP))
            viewplayer->cheats &= ~CF_NOCLIP;
        else if (value == 1 && !(viewplayer->cheats & CF_NOCLIP))
            viewplayer->cheats |= CF_NOCLIP;
        else
            return;
    }
    else
        viewplayer->cheats ^= CF_NOCLIP;

    if (viewplayer->cheats & CF_NOCLIP)
    {
        C_Output(s_STSTR_NCON);
        HU_SetPlayerMessage(s_STSTR_NCON, false, false);
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_NCOFF);
        HU_SetPlayerMessage(s_STSTR_NCOFF, false, false);
    }

    message_dontfuckwithme = true;
}

//
// nomonsters CCMD
//
static void nomonsters_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && nomonsters)
            nomonsters = false;
        else if (value == 1 && !nomonsters)
            nomonsters = true;
        else
            return;
    }
    else
        nomonsters = !nomonsters;

    if (nomonsters)
    {
        if (gamestate == GS_LEVEL)
            for (int i = 0; i < numsectors; i++)
            {
                mobj_t  *thing = sectors[i].thinglist;

                while (thing)
                {
                    const mobjtype_t    type = thing->type;
                    const int           flags = thing->flags;

                    if (((flags & MF_SHOOTABLE) || (flags & MF_CORPSE) || (thing->flags2 & MF2_MONSTERMISSILE))
                        && type != MT_PLAYER && type != MT_BARREL && type != MT_BOSSBRAIN)
                        P_RemoveMobj(thing);

                    thing = thing->snext;
                }
            }

        C_Output(s_STSTR_NMON);
        HU_SetPlayerMessage(s_STSTR_NMON, false, false);
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_NMOFF);
        HU_SetPlayerMessage(s_STSTR_NMOFF, false, false);

        message_dontfuckwithme = true;

        if (gamestate == GS_LEVEL)
            C_Warning(0, PENDINGCHANGE);
    }
}

//
// notarget CCMD
//
static void notarget_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && (viewplayer->cheats & CF_NOTARGET))
            viewplayer->cheats &= ~CF_NOTARGET;
        else if (value == 1 && !(viewplayer->cheats & CF_NOTARGET))
            viewplayer->cheats |= CF_NOTARGET;
        else
            return;
    }
    else
        viewplayer->cheats ^= CF_NOTARGET;

    if (viewplayer->cheats & CF_NOTARGET)
    {
        for (int i = 0; i < numsectors; i++)
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

        C_Output(s_STSTR_NTON);
        HU_SetPlayerMessage(s_STSTR_NTON, false, false);
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_NTOFF);
        HU_SetPlayerMessage(s_STSTR_NTOFF, false, false);
    }

    message_dontfuckwithme = true;
}

//
// pistolstart CCMD
//
static void pistolstart_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && pistolstart)
            pistolstart = false;
        else if (value == 1 && !pistolstart)
            pistolstart = true;
        else
            return;
    }
    else
        pistolstart = !pistolstart;

    if (pistolstart)
    {
        C_Output(s_STSTR_PSON);
        HU_SetPlayerMessage(s_STSTR_PSON, false, false);
    }
    else
    {
        C_Output(s_STSTR_PSOFF);
        HU_SetPlayerMessage(s_STSTR_PSOFF, false, false);
    }

    message_dontfuckwithme = true;

    if (gamestate == GS_LEVEL)
        C_Warning(0, PENDINGCHANGE);
}

//
// play CCMD
//
static int  playcmdid;
static int  playcmdtype;

static dboolean play_cmd_func1(char *cmd, char *parms)
{
    char    namebuf[9];

    if (!*parms)
        return true;

    for (int i = 1; i < NUMSFX; i++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "ds%s", S_sfx[i].name2);

        if (M_StringCompare(parms, namebuf) && W_CheckNumForName(namebuf) >= 0)
        {
            playcmdid = i;
            playcmdtype = 1;
            return true;
        }
    }

    for (int i = 1; i < NUMMUSIC; i++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "d_%s", S_music[i].name2);

        if (M_StringCompare(parms, namebuf) && W_CheckNumForName(namebuf) >= 0)
        {
            playcmdid = i;
            playcmdtype = 2;
            return true;
        }
    }

    return false;
}

static void play_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, PLAYCMDFORMAT);
    }
    else if (playcmdtype == 1)
        S_StartSound(NULL, playcmdid);
    else
        S_ChangeMusic(playcmdid, true, true, false);
}

static skill_t favoriteskilllevel(void)
{
    uint64_t    skilllevelstat = 0;
    skill_t     favorite = skilllevel - 1;

    if (skilllevelstat < stat_skilllevel_imtooyoungtodie)
    {
        skilllevelstat = stat_skilllevel_imtooyoungtodie;
        favorite = sk_baby;
    }

    if (skilllevelstat < stat_skilllevel_heynottoorough)
    {
        skilllevelstat = stat_skilllevel_heynottoorough;
        favorite = sk_easy;
    }

    if (skilllevelstat < stat_skilllevel_hurtmeplenty)
    {
        skilllevelstat = stat_skilllevel_hurtmeplenty;
        favorite = sk_medium;
    }

    if (skilllevelstat < stat_skilllevel_ultraviolence)
    {
        skilllevelstat = stat_skilllevel_ultraviolence;
        favorite = sk_hard;
    }

    if (skilllevelstat < stat_skilllevel_nightmare)
        favorite = sk_nightmare;

    return favorite;
}

static weapontype_t favoriteweapon(dboolean total)
{
    weapontype_t    favorite = wp_nochange;

    if (total)
    {
        uint64_t    shotsfiredstat = 0;

        if (shotsfiredstat < stat_shotsfired_pistol)
        {
            shotsfiredstat = stat_shotsfired_pistol;
            favorite = wp_pistol;
        }

        if (shotsfiredstat < stat_shotsfired_shotgun)
        {
            shotsfiredstat = stat_shotsfired_shotgun;
            favorite = wp_shotgun;
        }

        if (shotsfiredstat < stat_shotsfired_supershotgun)
        {
            shotsfiredstat = stat_shotsfired_supershotgun;
            favorite = wp_supershotgun;
        }

        if (shotsfiredstat < stat_shotsfired_chaingun)
        {
            shotsfiredstat = stat_shotsfired_chaingun;
            favorite = wp_chaingun;
        }

        if (shotsfiredstat < stat_shotsfired_rocketlauncher)
        {
            shotsfiredstat = stat_shotsfired_rocketlauncher;
            favorite = wp_missile;
        }

        if (shotsfiredstat < stat_shotsfired_plasmarifle)
        {
            shotsfiredstat = stat_shotsfired_plasmarifle;
            favorite = wp_plasma;
        }

        if (shotsfiredstat < stat_shotsfired_bfg9000)
            favorite = wp_bfg;
    }
    else
        for (int i = 0, shotsfiredstat = 0; i < NUMWEAPONS; i++)
            if (shotsfiredstat < viewplayer->shotsfired[i])
            {
                shotsfiredstat = viewplayer->shotsfired[i];
                favorite = i;
            }

    return favorite;
}

static char *distance(uint64_t value, dboolean showunits)
{
    char    *result = malloc(20);

    value /= UNITSPERFOOT;

    if (units == units_metric)
    {
        const float meters = value / FEETPERMETER;

        if (!meters)
            M_StringCopy(result, (showunits ? "0 meters" : "0"), 20);
        else if (meters < METERSPERKILOMETER)
        {
            char    *temp = striptrailingzero(meters, 1);

            M_snprintf(result, 20, "%s%s%s", temp, (showunits ? " meter" : ""), (meters == 1.0f || !showunits ? "" : "s"));
            free(temp);
        }
        else
        {
            char    *temp = striptrailingzero(meters / METERSPERKILOMETER, 2);

            M_snprintf(result, 20, "%s%s%s", temp, (showunits ? " kilometer" : ""),
                (meters == METERSPERKILOMETER || !showunits ? "" : "s"));
            free(temp);
        }
    }
    else
    {
        if (value < FEETPERMILE)
        {
            char    *temp = commify(value);

            M_snprintf(result, 20, "%s%s", temp, (showunits ? (value == 1 ? " foot" : " feet") : ""));
            free(temp);
        }
        else
        {
            char    *temp = striptrailingzero((float)value / FEETPERMILE, 2);

            M_snprintf(result, 20, "%s%s%s", temp, (showunits ? " mile" : ""), (value == FEETPERMILE || !showunits ? "" : "s"));
            free(temp);
        }
    }

    return result;
}

//
// playerstats CCMD
//
static void C_PlayerStats_Game(void)
{
    const int       tabs[4] = { 160, 281, 0, 0 };
    skill_t         favoriteskilllevel1 = favoriteskilllevel();
    weapontype_t    favoriteweapon1 = favoriteweapon(false);
    weapontype_t    favoriteweapon2 = favoriteweapon(true);
    const int       time1 = leveltime / TICRATE;
    const int       time2 = (int)(stat_time / TICRATE);
    char            *temp1;
    char            *temp2;
    char            *temp3;
    char            *temp4;
    char            *temp5;
    int             killcount = 0;
    int             shotsfired1 = 0;
    uint64_t        shotsfired2 = 0;
    int             shotssuccessful1 = 0;
    uint64_t        shotssuccessful2 = 0;

    C_Header(tabs, playerstats, PLAYERSTATSHEADER);

    if (viewplayer->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS))
        C_TabbedOutput(tabs, "Map explored\t<b>100%%</b>\t-");
    else
    {
        int mappedwalls = 0;
        int totalwalls = 0;

        for (int i = 0; i < numlines; i++)
        {
            const line_t            line = lines[i];
            const unsigned short    flags = line.flags;

            if (!(flags & ML_DONTDRAW))
            {
                const sector_t  *back = line.backsector;
                const sector_t  *front = line.frontsector;

                if (!back || back->floorheight != front->floorheight || back->ceilingheight != front->ceilingheight)
                {
                    totalwalls++;

                    if (flags & ML_MAPPED)
                        mappedwalls++;
                }
            }
        }

        temp1 = striptrailingzero((totalwalls ? mappedwalls * 100.0f / totalwalls : 0.0f), 1);
        C_TabbedOutput(tabs, "Map explored\t<b>%s%%</b>\t-", temp1);
        free(temp1);
    }

    temp1 = commify(stat_mapsstarted);
    C_TabbedOutput(tabs, "Maps started\t<b>1</b>\t<b>%s</b>", temp1);
    free(temp1);

    temp1 = commify(stat_mapscompleted);
    C_TabbedOutput(tabs, "Maps completed\t-\t<b>%s</b>", temp1);
    free(temp1);

    temp1 = commify(viewplayer->gamessaved);
    temp2 = commify(stat_gamessaved);
    C_TabbedOutput(tabs, "Games saved\t<b>%s</b>\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    if (favoriteskilllevel1 == sk_none)
        C_TabbedOutput(tabs, "Favorite skill level\t-\t-");
    else
    {
        temp1 = titlecase(*skilllevels[skilllevel - 1]);

        if (temp1[strlen(temp1) - 1] == '.')
            temp1[strlen(temp1) - 1] = '\0';

        temp2 = titlecase(*skilllevels[favoriteskilllevel1]);

        if (temp2[strlen(temp2) - 1] == '.')
            temp2[strlen(temp2) - 1] = '\0';

        C_TabbedOutput(tabs, "Favorite skill level\t<b><i>%s</i></b>\t<b><i>%s</i></b>", temp1, temp2);
        free(temp1);
        free(temp2);
    }

    for (int i = 0; i < NUMMOBJTYPES; i++)
        killcount += viewplayer->mobjcount[i];

    temp1 = commify(killcount);
    temp2 = commify(totalkills);
    temp3 = commify(stat_monsterskilled);
    C_TabbedOutput(tabs, "Monsters killed\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, (totalkills ? killcount * 100 / totalkills : 0), temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(mobjinfo[MT_BABY].plural1);
        temp2 = commify(viewplayer->mobjcount[MT_BABY]);
        temp3 = commify(monstercount[MT_BABY]);
        temp4 = commify(stat_monsterskilled_arachnotrons);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            temp1, temp2, temp3,
            (monstercount[MT_BABY] ? viewplayer->mobjcount[MT_BABY] * 100 / monstercount[MT_BABY] : 0), temp4);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);

        temp1 = sentencecase(mobjinfo[MT_VILE].plural1);
        temp2 = commify(viewplayer->mobjcount[MT_VILE]);
        temp3 = commify(monstercount[MT_VILE]);
        temp4 = commify(stat_monsterskilled_archviles);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            temp1, temp2, temp3,
            (monstercount[MT_VILE] ? viewplayer->mobjcount[MT_VILE] * 100 / monstercount[MT_VILE] : 0), temp4);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
    }

    temp1 = sentencecase(mobjinfo[MT_BRUISER].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_BRUISER]);
    temp3 = commify(monstercount[MT_BRUISER]);
    temp4 = commify(stat_monsterskilled_baronsofhell);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_BRUISER] ? viewplayer->mobjcount[MT_BRUISER] * 100 / monstercount[MT_BRUISER] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = sentencecase(mobjinfo[MT_HEAD].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_HEAD]);
    temp3 = commify(monstercount[MT_HEAD]);
    temp4 = commify(stat_monsterskilled_cacodemons);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_HEAD] ? viewplayer->mobjcount[MT_HEAD] * 100 / monstercount[MT_HEAD] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(mobjinfo[MT_CYBORG].plural1);
        temp2 = commify(viewplayer->mobjcount[MT_CYBORG]);
        temp3 = commify(monstercount[MT_CYBORG]);
        temp4 = commify(stat_monsterskilled_cyberdemons);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            temp1, temp2, temp3,
            (monstercount[MT_CYBORG] ? viewplayer->mobjcount[MT_CYBORG] * 100 / monstercount[MT_CYBORG] : 0), temp4);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
    }

    temp1 = sentencecase(mobjinfo[MT_SERGEANT].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_SERGEANT]);
    temp3 = commify(monstercount[MT_SERGEANT]);
    temp4 = commify(stat_monsterskilled_demons);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_SERGEANT] ? viewplayer->mobjcount[MT_SERGEANT] * 100 / monstercount[MT_SERGEANT] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(mobjinfo[MT_CHAINGUY].plural1);
        temp2 = commify(viewplayer->mobjcount[MT_CHAINGUY]);
        temp3 = commify(monstercount[MT_CHAINGUY]);
        temp4 = commify(stat_monsterskilled_heavyweapondudes);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            temp1, temp2, temp3,
            (monstercount[MT_CHAINGUY] ? viewplayer->mobjcount[MT_CHAINGUY] * 100 / monstercount[MT_CHAINGUY] : 0), temp4);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);

        temp1 = sentencecase(mobjinfo[MT_KNIGHT].plural1);
        temp2 = commify(viewplayer->mobjcount[MT_KNIGHT]);
        temp3 = commify(monstercount[MT_KNIGHT]);
        temp4 = commify(stat_monsterskilled_hellknights);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            temp1, temp2, temp3,
            (monstercount[MT_KNIGHT] ? viewplayer->mobjcount[MT_KNIGHT] * 100 / monstercount[MT_KNIGHT] : 0), temp4);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
    }

    temp1 = sentencecase(mobjinfo[MT_TROOP].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_TROOP]);
    temp3 = commify(monstercount[MT_TROOP]);
    temp4 = commify(stat_monsterskilled_imps);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_TROOP] ? viewplayer->mobjcount[MT_TROOP] * 100 / monstercount[MT_TROOP] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = sentencecase(mobjinfo[MT_SKULL].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_SKULL]);
    temp3 = commify(monstercount[MT_SKULL]);
    temp4 = commify(stat_monsterskilled_lostsouls);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_SKULL] ? viewplayer->mobjcount[MT_SKULL] * 100 / monstercount[MT_SKULL] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(mobjinfo[MT_FATSO].plural1);
        temp2 = commify(viewplayer->mobjcount[MT_FATSO]);
        temp3 = commify(monstercount[MT_FATSO]);
        temp4 = commify(stat_monsterskilled_mancubi);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            temp1, temp2, temp3,
            (monstercount[MT_FATSO] ? viewplayer->mobjcount[MT_FATSO] * 100 / monstercount[MT_FATSO] : 0), temp4);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);

        temp1 = sentencecase(mobjinfo[MT_PAIN].plural1);
        temp2 = commify(viewplayer->mobjcount[MT_PAIN]);
        temp3 = commify(monstercount[MT_PAIN]);
        temp4 = commify(stat_monsterskilled_painelementals);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            temp1, temp2, temp3,
            (monstercount[MT_PAIN] ? viewplayer->mobjcount[MT_PAIN] * 100 / monstercount[MT_PAIN] : 0), temp4);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
    }

    temp1 = sentencecase(mobjinfo[MT_UNDEAD].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_UNDEAD]);
    temp3 = commify(monstercount[MT_UNDEAD]);
    temp4 = commify(stat_monsterskilled_revenants);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_UNDEAD] ? viewplayer->mobjcount[MT_UNDEAD] * 100 / monstercount[MT_UNDEAD] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = sentencecase(mobjinfo[MT_SHOTGUY].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_SHOTGUY]);
    temp3 = commify(monstercount[MT_SHOTGUY]);
    temp4 = commify(stat_monsterskilled_shotgunguys);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_SHOTGUY] ? viewplayer->mobjcount[MT_SHOTGUY] * 100 / monstercount[MT_SHOTGUY] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = sentencecase(mobjinfo[MT_SHADOWS].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_SHADOWS]);
    temp3 = commify(monstercount[MT_SHADOWS]);
    temp4 = commify(stat_monsterskilled_spectres);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_SHADOWS] ? viewplayer->mobjcount[MT_SHADOWS] * 100 / monstercount[MT_SHADOWS] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(mobjinfo[MT_SPIDER].plural1);
        temp2 = commify(viewplayer->mobjcount[MT_SPIDER]);
        temp3 = commify(monstercount[MT_SPIDER]);
        temp4 = commify(stat_monsterskilled_spidermasterminds);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            temp1, temp2, temp3,
            (monstercount[MT_SPIDER] ? viewplayer->mobjcount[MT_SPIDER] * 100 / monstercount[MT_SPIDER] : 0), temp4);
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
    }

    temp1 = sentencecase(mobjinfo[MT_POSSESSED].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_POSSESSED]);
    temp3 = commify(monstercount[MT_POSSESSED]);
    temp4 = commify(stat_monsterskilled_zombiemen);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (monstercount[MT_POSSESSED] ? viewplayer->mobjcount[MT_POSSESSED] * 100 / monstercount[MT_POSSESSED] : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = sentencecase(mobjinfo[MT_BARREL].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_BARREL]);
    temp3 = commify(barrelcount);
    temp4 = commify(stat_barrelsexploded);
    C_TabbedOutput(tabs, "%s exploded\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, temp3,
        (barrelcount ? viewplayer->mobjcount[MT_BARREL] * 100 / barrelcount : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = commify(viewplayer->itemcount);
    temp2 = commify(totalitems);
    temp3 = commify(stat_itemspickedup);
    C_TabbedOutput(tabs, "Items picked up\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, (totalitems ? viewplayer->itemcount * 100 / totalitems : 0), temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = commify(viewplayer->itemspickedup_ammo_bullets);
    temp2 = commify(stat_itemspickedup_ammo_bullets);
    C_TabbedOutput(tabs, "   Ammo\t<b>%s bullet%s</b>\t<b>%s bullet%s</b>",
        temp1, (viewplayer->itemspickedup_ammo_bullets == 1 ? "" : "s"), temp2, (stat_itemspickedup_ammo_bullets == 1 ? "" : "s"));
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_ammo_cells);
    temp2 = commify(stat_itemspickedup_ammo_cells);
    C_TabbedOutput(tabs, "\t<b>%s cell%s</b>\t<b>%s cell%s</b>",
        temp1, (viewplayer->itemspickedup_ammo_cells == 1 ? "" : "s"), temp2, (stat_itemspickedup_ammo_cells == 1 ? "" : "s"));
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_ammo_rockets);
    temp2 = commify(stat_itemspickedup_ammo_rockets);
    C_TabbedOutput(tabs, "\t<b>%s rocket%s</b>\t<b>%s rocket%s</b>",
        temp1, (viewplayer->itemspickedup_ammo_rockets == 1 ? "" : "s"), temp2, (stat_itemspickedup_ammo_rockets == 1 ? "" : "s"));
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_ammo_shells);
    temp2 = commify(stat_itemspickedup_ammo_shells);
    C_TabbedOutput(tabs, "\t<b>%s shell%s</b>\t<b>%s shell%s</b>",
        temp1, (viewplayer->itemspickedup_ammo_shells == 1 ? "" : "s"), temp2, (stat_itemspickedup_ammo_shells == 1 ? "" : "s"));
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_armor);
    temp2 = commify(stat_itemspickedup_armor);
    C_TabbedOutput(tabs, "   Armor\t<b>%s%%</b>\t<b>%s%%</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_health);
    temp2 = commify(stat_itemspickedup_health);
    C_TabbedOutput(tabs, "   Health\t<b>%s%%</b>\t<b>%s%%</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->secretcount);
    temp2 = commify(totalsecret);
    temp3 = commify(stat_secretsfound);
    C_TabbedOutput(tabs, "Secrets found\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        temp1, temp2, (totalsecret ? viewplayer->secretcount * 100 / totalsecret : 0), temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    C_TabbedOutput(tabs, "Time played\t<b>%02i:%02i:%02i</b>\t<b>%02i:%02i:%02i</b>",
        time1 / 3600, (time1 % 3600) / 60, (time1 % 3600) % 60, time2 / 3600, (time2 % 3600) / 60, (time2 % 3600) % 60);

    temp1 = commify(viewplayer->damageinflicted);
    temp2 = commify(stat_damageinflicted);
    C_TabbedOutput(tabs, "Damage inflicted\t<b>%s%%</b>\t<b>%s%%</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->damagereceived);
    temp2 = commify(stat_damagereceived);
    C_TabbedOutput(tabs, "Damage received\t<b>%s%%</b>\t<b>%s%%</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->deaths);
    temp2 = commify(stat_deaths);
    C_TabbedOutput(tabs, "Deaths\t<b>%s</b>\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->suicides);
    temp2 = commify(stat_suicides);
    C_TabbedOutput(tabs, "Suicides\t<b>%s</b>\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->cheated);
    temp2 = commify(stat_cheated);
    C_TabbedOutput(tabs, "Cheated\t<b>%s</b>\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    for (int i = 0; i < NUMWEAPONS; i++)
        shotsfired1 += viewplayer->shotsfired[i];

    for (int i = 0; i < NUMWEAPONS; i++)
        shotssuccessful1 += viewplayer->shotssuccessful[i];

    temp1 = commify(shotssuccessful1);
    temp2 = commify(shotsfired1);
    temp3 = commify((shotssuccessful2 = stat_shotssuccessful_pistol + stat_shotssuccessful_shotgun + stat_shotssuccessful_supershotgun
        + stat_shotssuccessful_chaingun + stat_shotssuccessful_rocketlauncher + stat_shotssuccessful_plasmarifle
        + stat_shotssuccessful_bfg9000));
    temp4 = commify((shotsfired2 = stat_shotsfired_pistol + stat_shotsfired_shotgun + stat_shotsfired_supershotgun
        + stat_shotsfired_chaingun + stat_shotsfired_rocketlauncher + stat_shotsfired_plasmarifle + stat_shotsfired_bfg9000));
    C_TabbedOutput(tabs, "Shots successful/fired\t<b>%s of %s (%i%%)</b>\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, (shotsfired1 ? shotssuccessful1 * 100 / shotsfired1 : 0), temp3, temp4,
        (shotsfired2 ? (int)(shotssuccessful2 * 100 / shotsfired2) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = sentencecase(weaponinfo[wp_pistol].description);
    temp2 = commify(viewplayer->shotssuccessful[wp_pistol]);
    temp3 = commify(viewplayer->shotsfired[wp_pistol]);
    temp4 = commify(stat_shotssuccessful_pistol);
    temp5 = commify(stat_shotsfired_pistol);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, temp3,
        (viewplayer->shotsfired[wp_pistol] ? viewplayer->shotssuccessful[wp_pistol] * 100 / viewplayer->shotsfired[wp_pistol] : 0),
        temp4, temp5,
        (stat_shotsfired_pistol ? (int)(stat_shotssuccessful_pistol * 100 / stat_shotsfired_pistol) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);

    temp1 = sentencecase(weaponinfo[wp_shotgun].description);
    temp2 = commify(viewplayer->shotssuccessful[wp_shotgun]);
    temp3 = commify(viewplayer->shotsfired[wp_shotgun]);
    temp4 = commify(stat_shotssuccessful_shotgun);
    temp5 = commify(stat_shotsfired_shotgun);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, temp3,
        (viewplayer->shotsfired[wp_shotgun] ? viewplayer->shotssuccessful[wp_shotgun] * 100 / viewplayer->shotsfired[wp_shotgun] : 0),
        temp4, temp5,
        (stat_shotsfired_shotgun ? (int)(stat_shotssuccessful_shotgun * 100 / stat_shotsfired_shotgun) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(weaponinfo[wp_supershotgun].description);
        temp2 = commify(viewplayer->shotssuccessful[wp_supershotgun]);
        temp3 = commify(viewplayer->shotsfired[wp_supershotgun]);
        temp4 = commify(stat_shotssuccessful_supershotgun);
        temp5 = commify(stat_shotsfired_supershotgun);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s of %s (%i%%)</b>",
            temp1, temp2, temp3,
            (viewplayer->shotsfired[wp_supershotgun] ?
                viewplayer->shotssuccessful[wp_supershotgun] * 100 / viewplayer->shotsfired[wp_supershotgun] : 0),
            temp4, temp5,
            (stat_shotsfired_supershotgun ? (int)(stat_shotssuccessful_supershotgun * 100 / stat_shotsfired_supershotgun) : 0));
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
        free(temp5);
    }

    temp1 = sentencecase(weaponinfo[wp_chaingun].description);
    temp2 = commify(viewplayer->shotssuccessful[wp_chaingun]);
    temp3 = commify(viewplayer->shotsfired[wp_chaingun]);
    temp4 = commify(stat_shotssuccessful_chaingun);
    temp5 = commify(stat_shotsfired_chaingun);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, temp3,
        (viewplayer->shotsfired[wp_chaingun] ? viewplayer->shotssuccessful[wp_chaingun] * 100 / viewplayer->shotsfired[wp_chaingun] : 0),
        temp4, temp5,
        (stat_shotsfired_chaingun ? (int)(stat_shotssuccessful_chaingun * 100 / stat_shotsfired_chaingun) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);

    temp1 = sentencecase(weaponinfo[wp_missile].description);
    temp2 = commify(viewplayer->shotssuccessful[wp_missile]);
    temp3 = commify(viewplayer->shotsfired[wp_missile]);
    temp4 = commify(stat_shotssuccessful_rocketlauncher);
    temp5 = commify(stat_shotsfired_rocketlauncher);
    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, temp3,
        (viewplayer->shotsfired[wp_missile] ? viewplayer->shotssuccessful[wp_missile] * 100 / viewplayer->shotsfired[wp_missile] : 0),
        temp4, temp5,
        (stat_shotsfired_rocketlauncher ? (int)(stat_shotssuccessful_rocketlauncher * 100 / stat_shotsfired_rocketlauncher) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(weaponinfo[wp_plasma].description);
        temp2 = commify(viewplayer->shotssuccessful[wp_plasma]);
        temp3 = commify(viewplayer->shotsfired[wp_plasma]);
        temp4 = commify(stat_shotssuccessful_plasmarifle);
        temp5 = commify(stat_shotsfired_plasmarifle);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s of %s (%i%%)</b>",
            temp1, temp2, temp3,
            (viewplayer->shotsfired[wp_plasma] ? viewplayer->shotssuccessful[wp_plasma] * 100 / viewplayer->shotsfired[wp_plasma] : 0),
            temp4, temp5,
            (stat_shotsfired_plasmarifle ? (int)(stat_shotssuccessful_plasmarifle * 100 / stat_shotsfired_plasmarifle) : 0));
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
        free(temp5);

        temp1 = sentencecase(weaponinfo[wp_bfg].description);
        temp2 = commify(viewplayer->shotssuccessful[wp_bfg]);
        temp3 = commify(viewplayer->shotsfired[wp_bfg]);
        temp4 = commify(stat_shotssuccessful_bfg9000);
        temp5 = commify(stat_shotsfired_bfg9000);
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s of %s (%i%%)</b>",
            temp1, temp2, temp3,
            (viewplayer->shotsfired[wp_bfg] ? viewplayer->shotssuccessful[wp_bfg] * 100 / viewplayer->shotsfired[wp_bfg] : 0),
            temp4, temp5,
            (stat_shotsfired_bfg9000 ? (int)(stat_shotssuccessful_bfg9000 * 100 / stat_shotsfired_bfg9000) : 0));
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
        free(temp5);
    }

    if (favoriteweapon1 == wp_nochange && favoriteweapon2 == wp_nochange)
    {
        temp1 = sentencecase(weaponinfo[wp_pistol].description);
        C_TabbedOutput(tabs, "Favorite weapon\t%s\t%s", temp1, temp1);
        free(temp1);
    }
    else if (favoriteweapon1 == wp_nochange)
    {
        temp1 = sentencecase(weaponinfo[wp_pistol].description);
        temp2 = sentencecase(weaponinfo[favoriteweapon2].description);
        C_TabbedOutput(tabs, "Favorite weapon\t<b>%s</b>\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);
    }
    else
    {
        temp1 = sentencecase(weaponinfo[favoriteweapon1].description);
        temp2 = sentencecase(weaponinfo[favoriteweapon2].description);
        C_TabbedOutput(tabs, "Favorite weapon\t<b>%s</b>\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);
    }

    C_TabbedOutput(tabs, "Distance traveled\t<b>%s</b>\t<b>%s</b>",
        distance(viewplayer->distancetraveled, true), distance(stat_distancetraveled, true));
}

static void C_PlayerStats_NoGame(void)
{
    const int       tabs[4] = { 160, 281, 0, 0 };
    skill_t         favoriteskilllevel1 = favoriteskilllevel();
    weapontype_t    favoriteweapon1 = favoriteweapon(true);
    const int       time2 = (int)(stat_time / TICRATE);
    char            *temp1;
    char            *temp2;
    char            *temp3;
    uint64_t        shotsfired = 0;
    uint64_t        shotssuccessful = 0;

    C_Header(tabs, playerstats, PLAYERSTATSHEADER);

    temp1 = commify(stat_mapsstarted);
    C_TabbedOutput(tabs, "Maps started\t-\t<b>%s</b>", temp1);
    free(temp1);

    temp1 = commify(stat_mapscompleted);
    C_TabbedOutput(tabs, "Maps completed\t-\t<b>%s</b>", temp1);
    free(temp1);

    temp1 = commify(stat_gamessaved);
    C_TabbedOutput(tabs, "Games saved\t-\t<b>%s</b>", temp1);
    free(temp1);

    if (favoriteskilllevel1 == sk_none)
        C_TabbedOutput(tabs, "Favorite skill level\t-\t-");
    else
    {
        temp1 = titlecase(*skilllevels[favoriteskilllevel1]);

        if (temp1[strlen(temp1) - 1] == '.')
            temp1[strlen(temp1) - 1] = '\0';

        C_TabbedOutput(tabs, "Favorite skill level\t-\t<b><i>%s</i></b>", temp1);
        free(temp1);
    }

    temp1 = commify(stat_monsterskilled);
    C_TabbedOutput(tabs, "Monsters killed\t-\t<b>%s</b>", temp1);
    free(temp1);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(mobjinfo[MT_BABY].plural1);
        temp2 = commify(stat_monsterskilled_arachnotrons);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);

        temp1 = sentencecase(mobjinfo[MT_VILE].plural1);
        temp2 = commify(stat_monsterskilled_archviles);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);
    }

    temp1 = sentencecase(mobjinfo[MT_BRUISER].plural1);
    temp2 = commify(stat_monsterskilled_baronsofhell);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(mobjinfo[MT_HEAD].plural1);
    temp2 = commify(stat_monsterskilled_cacodemons);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(mobjinfo[MT_CYBORG].plural1);
        temp2 = commify(stat_monsterskilled_cyberdemons);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);
    }

    temp1 = sentencecase(mobjinfo[MT_SERGEANT].plural1);
    temp2 = commify(stat_monsterskilled_demons);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(mobjinfo[MT_CHAINGUY].plural1);
        temp2 = commify(stat_monsterskilled_heavyweapondudes);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);

        temp1 = sentencecase(mobjinfo[MT_KNIGHT].plural1);
        temp2 = commify(stat_monsterskilled_hellknights);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);
    }

    temp1 = sentencecase(mobjinfo[MT_TROOP].plural1);
    temp2 = commify(stat_monsterskilled_imps);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(mobjinfo[MT_SKULL].plural1);
    temp2 = commify(stat_monsterskilled_lostsouls);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(mobjinfo[MT_FATSO].plural1);
        temp2 = commify(stat_monsterskilled_mancubi);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);

        temp1 = sentencecase(mobjinfo[MT_PAIN].plural1);
        temp2 = commify(stat_monsterskilled_painelementals);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);
    }

    temp1 = sentencecase(mobjinfo[MT_UNDEAD].plural1);
    temp2 = commify(stat_monsterskilled_revenants);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(mobjinfo[MT_SHOTGUY].plural1);
    temp2 = commify(stat_monsterskilled_shotgunguys);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(mobjinfo[MT_SHADOWS].plural1);
    temp2 = commify(stat_monsterskilled_spectres);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(mobjinfo[MT_SPIDER].plural1);
        temp2 = commify(stat_monsterskilled_spidermasterminds);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
        free(temp1);
        free(temp2);
    }

    temp1 = sentencecase(mobjinfo[MT_POSSESSED].plural1);
    temp2 = commify(stat_monsterskilled_zombiemen);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(mobjinfo[MT_BARREL].plural1);
    temp2 = commify(stat_barrelsexploded);
    C_TabbedOutput(tabs, "%s exploded\t-\t<b>%s</b>", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(stat_itemspickedup);
    C_TabbedOutput(tabs, "Items picked up\t-\t<b>%s</b>", temp1);
    free(temp1);

    temp1 = commify(stat_itemspickedup_ammo_bullets);
    C_TabbedOutput(tabs, "   Ammo\t-\t<b>%s bullet%s</b>", temp1, (stat_itemspickedup_ammo_bullets == 1 ? "" : "s"));
    free(temp1);

    temp1 = commify(stat_itemspickedup_ammo_cells);
    C_TabbedOutput(tabs, "\t-\t<b>%s cell%s</b>", temp1 , (stat_itemspickedup_ammo_cells == 1 ? "" : "s"));
    free(temp1);

    temp1 = commify(stat_itemspickedup_ammo_rockets);
    C_TabbedOutput(tabs, "\t-\t<b>%s rocket%s</b>", temp1, (stat_itemspickedup_ammo_rockets == 1 ? "" : "s"));
    free(temp1);

    temp1 = commify(stat_itemspickedup_ammo_shells);
    C_TabbedOutput(tabs, "\t-\t<b>%s shell%s</b>", temp1, (stat_itemspickedup_ammo_shells == 1 ? "" : "s"));
    free(temp1);

    temp1 = commify(stat_itemspickedup_armor);
    C_TabbedOutput(tabs, "   Armor\t-\t<b>%s%%</b>", temp1);
    free(temp1);

    temp1 = commify(stat_itemspickedup_health);
    C_TabbedOutput(tabs, "   Health\t-\t<b>%s%%</b>", temp1);
    free(temp1);

    temp1 = commify(stat_secretsfound);
    C_TabbedOutput(tabs, "Secrets found\t-\t<b>%s</b>", temp1);
    free(temp1);

    C_TabbedOutput(tabs, "Time played\t-\t<b>%02i:%02i:%02i</b>", time2 / 3600, (time2 % 3600) / 60, (time2 % 3600) % 60);

    temp1 = commify(stat_damageinflicted);
    C_TabbedOutput(tabs, "Damage inflicted\t-\t<b>%s%%</b>", temp1);
    free(temp1);

    temp1 = commify(stat_damagereceived);
    C_TabbedOutput(tabs, "Damage received\t-\t<b>%s%%</b>", temp1);
    free(temp1);

    temp1 = commify(stat_deaths);
    C_TabbedOutput(tabs, "Deaths\t-\t<b>%s</b>", temp1);
    free(temp1);

    temp1 = commify(stat_suicides);
    C_TabbedOutput(tabs, "Suicides\t-\t<b>%s</b>", temp1);
    free(temp1);

    temp1 = commify(stat_cheated);
    C_TabbedOutput(tabs, "Cheated\t-\t<b>%s</b>", temp1);
    free(temp1);

    temp1 = commify((shotssuccessful = stat_shotssuccessful_pistol + stat_shotssuccessful_shotgun + stat_shotssuccessful_supershotgun
        + stat_shotssuccessful_chaingun + stat_shotssuccessful_rocketlauncher + stat_shotssuccessful_plasmarifle
        + stat_shotssuccessful_bfg9000));
    temp2 = commify((shotsfired = stat_shotsfired_pistol + stat_shotsfired_shotgun + stat_shotsfired_supershotgun
        + stat_shotsfired_chaingun + stat_shotsfired_rocketlauncher + stat_shotsfired_plasmarifle + stat_shotsfired_bfg9000));
    C_TabbedOutput(tabs, "Shots successful/fired\t-\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, (shotsfired ? (int)(shotssuccessful * 100 / shotsfired) : 0));
    free(temp1);
    free(temp2);

    temp1 = sentencecase(weaponinfo[wp_pistol].description);
    temp2 = commify(stat_shotssuccessful_pistol);
    temp3 = commify(stat_shotsfired_pistol);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, temp3, (stat_shotsfired_pistol ? (int)(stat_shotssuccessful_pistol * 100 / stat_shotsfired_pistol) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = sentencecase(weaponinfo[wp_shotgun].description);
    temp2 = commify(stat_shotssuccessful_shotgun);
    temp3 = commify(stat_shotsfired_shotgun);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, temp3, (stat_shotsfired_shotgun ? (int)(stat_shotssuccessful_shotgun * 100 / stat_shotsfired_shotgun) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(weaponinfo[wp_supershotgun].description);
        temp2 = commify(stat_shotssuccessful_supershotgun);
        temp3 = commify(stat_shotsfired_supershotgun);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s of %s (%i%%)</b>",
            temp1, temp2, temp3,
            (stat_shotsfired_supershotgun ? (int)(stat_shotssuccessful_supershotgun * 100 / stat_shotsfired_supershotgun) : 0));
        free(temp1);
        free(temp2);
        free(temp3);
    }

    temp1 = sentencecase(weaponinfo[wp_chaingun].description);
    temp2 = commify(stat_shotssuccessful_chaingun);
    temp3 = commify(stat_shotsfired_chaingun);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, temp3, (stat_shotsfired_chaingun ? (int)(stat_shotssuccessful_chaingun * 100 / stat_shotsfired_chaingun) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = sentencecase(weaponinfo[wp_missile].description);
    temp2 = commify(stat_shotssuccessful_rocketlauncher);
    temp3 = commify(stat_shotsfired_rocketlauncher);
    C_TabbedOutput(tabs, "   %s\t-\t<b>%s of %s (%i%%)</b>",
        temp1, temp2, temp3,
        (stat_shotsfired_rocketlauncher ? (int)(stat_shotssuccessful_rocketlauncher * 100 / stat_shotsfired_rocketlauncher) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(weaponinfo[wp_plasma].description);
        temp2 = commify(stat_shotssuccessful_plasmarifle);
        temp3 = commify(stat_shotsfired_plasmarifle);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s of %s (%i%%)</b>",
            temp1, temp2, temp3,
            (stat_shotsfired_plasmarifle ? (int)(stat_shotssuccessful_plasmarifle * 100 / stat_shotsfired_plasmarifle) : 0));
        free(temp1);
        free(temp2);
        free(temp3);

        temp1 = sentencecase(weaponinfo[wp_bfg].description);
        temp2 = commify(stat_shotssuccessful_bfg9000);
        temp3 = commify(stat_shotsfired_bfg9000);
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s of %s (%i%%)</b>",
            temp1, temp2, temp3, (stat_shotsfired_bfg9000 ? (int)(stat_shotssuccessful_bfg9000 * 100 / stat_shotsfired_bfg9000) : 0));
        free(temp1);
        free(temp2);
        free(temp3);
    }

    if (favoriteweapon1 == wp_nochange)
        C_TabbedOutput(tabs, "Favorite weapon\t-\t-");
    else
    {
        temp1 = sentencecase(weaponinfo[favoriteweapon1].description);
        C_TabbedOutput(tabs, "Favorite weapon\t-\t<b>%s</b>", temp1);
        free(temp1);
    }

    C_TabbedOutput(tabs, "Distance traveled\t-\t<b>%s</b>", distance(stat_distancetraveled, true));
}

static void playerstats_cmd_func2(char *cmd, char *parms)
{
    if (gamestate == GS_LEVEL)
        C_PlayerStats_Game();
    else
        C_PlayerStats_NoGame();
}

//
// print CCMD
//
static void print_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, PRINTCMDFORMAT);
    }
    else
    {
        C_PlayerMessage(parms);

        if (gamestate == GS_LEVEL && !message_dontfuckwithme)
            HU_SetPlayerMessage(parms, false, false);
    }
}

//
// quit CCMD
//
static void quit_cmd_func2(char *cmd, char *parms)
{
    quitcmd = true;
}

//
// regenhealth CCMD
//
static void regenhealth_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && regenhealth)
            regenhealth = false;
        else if (value == 1 && !regenhealth)
            regenhealth = true;
        else
            return;
    }
    else
        regenhealth = !regenhealth;

    if (regenhealth)
    {
        C_Output(s_STSTR_RHON);
        HU_SetPlayerMessage(s_STSTR_RHON, false, false);
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_RHOFF);
        HU_SetPlayerMessage(s_STSTR_RHOFF, false, false);
    }

    message_dontfuckwithme = true;
}

//
// reset CCMD
//
static void reset_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, RESETCMDFORMAT);
        return;
    }

    if (M_StringCompare(parms, "all"))
    {
        resetall_cmd_func2("resetall", "");
        return;
    }

    if (M_StringCompare(parms, "ammo")
        || M_StringCompare(parms, "armor") || M_StringCompare(parms, "armour")
        || M_StringCompare(parms, "armortype") || M_StringCompare(parms, "armourtype")
        || M_StringCompare(parms, "health"))
        return;

    resettingcvar = true;

    for (int i = 0; *consolecmds[i].name; i++)
    {
        const int   flags = consolecmds[i].flags;

        if (consolecmds[i].type == CT_CVAR && M_StringCompare(parms, consolecmds[i].name) && !(flags & CF_READONLY))
        {
            if (flags & (CF_BOOLEAN | CF_INTEGER))
            {
                char    *temp1 = C_LookupAliasFromValue((int)consolecmds[i].defaultnumber, consolecmds[i].aliases);
                char    *temp2 = uncommify(temp1);
                char    *temp3 = M_StringJoin(parms, " ", temp2, NULL);

                C_ValidateInput(temp3);
                free(temp1);
                free(temp2);
                free(temp3);
            }
            else if (flags & CF_FLOAT)
            {
                char    *temp1 = striptrailingzero(consolecmds[i].defaultnumber, 1);
                char    *temp2 = M_StringJoin(parms, " ", temp1, NULL);

                C_ValidateInput(temp2);
                free(temp1);
                free(temp2);
            }
            else
            {
                char    *temp = M_StringJoin(parms, " ", (*consolecmds[i].defaultstring ? consolecmds[i].defaultstring : EMPTYVALUE),
                            NULL);

                C_ValidateInput(temp);
                free(temp);
            }

#if defined(_WIN32)
            if (M_StringCompare(parms, stringize(iwadfolder)))
            {
                wad = "";
                M_SaveCVARs();
            }
#endif

            break;
        }
    }

    resettingcvar = false;
}

//
// resetall CCMD
//
static void C_VerifyResetAll(const int key)
{
    messagetoprint = false;
    SDL_StartTextInput();

    if (key == 'y')
    {
        resettingcvar = true;

        // reset all CVARs to default values
        for (int i = 0; *consolecmds[i].name; i++)
        {
            const int   flags = consolecmds[i].flags;

            if (consolecmds[i].type == CT_CVAR && !(flags & CF_READONLY))
            {
                if (flags & (CF_BOOLEAN | CF_INTEGER))
                {
                    char    *temp1 = C_LookupAliasFromValue((int)consolecmds[i].defaultnumber, consolecmds[i].aliases);
                    char    *temp2 = uncommify(temp1);

                    consolecmds[i].func2(consolecmds[i].name, temp2);
                    free(temp1);
                    free(temp2);
                }
                else if (flags & CF_FLOAT)
                {
                    char    *temp = striptrailingzero(consolecmds[i].defaultnumber, 2);

                    consolecmds[i].func2(consolecmds[i].name, temp);
                    free(temp);
                }
                else
                    consolecmds[i].func2(consolecmds[i].name,
                        (*consolecmds[i].defaultstring ? consolecmds[i].defaultstring : EMPTYVALUE));
            }
        }

#if defined(_WIN32)
        wad = wad_default;
#endif

        resettingcvar = false;

        // unbind all controls
        for (int i = 0; *actions[i].action; i++)
        {
            if (actions[i].keyboard1)
                *(int *)actions[i].keyboard1 = 0;

            if (actions[i].keyboard2)
                *(int *)actions[i].keyboard2 = 0;

            if (actions[i].mouse1)
                *(int *)actions[i].mouse1 = -1;

            if (actions[i].gamepad1)
                *(int *)actions[i].gamepad1 = 0;

            if (actions[i].gamepad2)
                *(int *)actions[i].gamepad2 = 0;
        }

        for (int i = 0; i < NUMKEYS; i++)
            keyactionlist[i][0] = '\0';

        for (int i = 0; i < MAX_MOUSE_BUTTONS + 2; i++)
            mouseactionlist[i][0] = '\0';

        // reset stretched sky
        if (gamestate == GS_LEVEL)
        {
            R_InitSkyMap();
            R_InitColumnFunctions();
        }

        // set all controls to defaults
        keyboardalwaysrun = KEYALWAYSRUN_DEFAULT;
        keyboardautomap = KEYAUTOMAP_DEFAULT;
        keyboardautomapclearmark = KEYAUTOMAPCLEARMARK_DEFAULT;
        keyboardautomapfollowmode = KEYAUTOMAPFOLLOWMODE_DEFAULT;
        keyboardautomapgrid = KEYAUTOMAPGRID_DEFAULT;
        keyboardautomapmark = KEYAUTOMAPMARK_DEFAULT;
        keyboardautomapmaxzoom = KEYAUTOMAPMAXZOOM_DEFAULT;
        keyboardautomaprotatemode = KEYAUTOMAPROTATEMODE_DEFAULT;
        keyboardautomapzoomin = KEYAUTOMAPZOOMIN_DEFAULT;
        keyboardautomapzoomout = KEYAUTOMAPZOOMOUT_DEFAULT;
        keyboardback = KEYDOWN_DEFAULT;
        keyboardback2 = KEYDOWN2_DEFAULT;
        keyboardconsole = KEYCONSOLE_DEFAULT;
        keyboardfire = KEYFIRE_DEFAULT;
        keyboardforward = KEYUP_DEFAULT;
        keyboardforward2 = KEYUP2_DEFAULT;
        keyboardjump = KEYJUMP_DEFAULT;
        keyboardleft = KEYLEFT_DEFAULT;
        keyboardmenu = KEY_ESCAPE;
        keyboardmouselook = KEYMOUSELOOK_DEFAULT;
        keyboardnextweapon = KEYNEXTWEAPON_DEFAULT;
        keyboardprevweapon = KEYPREVWEAPON_DEFAULT;
        keyboardright = KEYRIGHT_DEFAULT;
        keyboardrun = KEYRUN_DEFAULT;
        keyboardscreenshot = KEYSCREENSHOT_DEFAULT;
        keyboardstrafe = KEYSTRAFE_DEFAULT;
        keyboardstrafeleft = KEYSTRAFELEFT_DEFAULT;
        keyboardstrafeleft2 = KEYSTRAFELEFT2_DEFAULT;
        keyboardstraferight = KEYSTRAFERIGHT_DEFAULT;
        keyboardstraferight2 = KEYSTRAFERIGHT2_DEFAULT;
        keyboarduse = KEYUSE_DEFAULT;
        keyboarduse2 = KEYUSE2_DEFAULT;
        keyboardweapon1 = KEYWEAPON1_DEFAULT;
        keyboardweapon2 = KEYWEAPON2_DEFAULT;
        keyboardweapon3 = KEYWEAPON3_DEFAULT;
        keyboardweapon4 = KEYWEAPON4_DEFAULT;
        keyboardweapon5 = KEYWEAPON5_DEFAULT;
        keyboardweapon6 = KEYWEAPON6_DEFAULT;
        keyboardweapon7 = KEYWEAPON7_DEFAULT;

        mousefire = MOUSEFIRE_DEFAULT;
        mouseforward = MOUSEFORWARD_DEFAULT;
        mousejump = MOUSEJUMP_DEFAULT;
        mousemouselook = MOUSEMOUSELOOK_DEFAULT;
        mousenextweapon = MOUSENEXTWEAPON_DEFAULT;
        mouseprevweapon = MOUSEPREVWEAPON_DEFAULT;
        mouserun = MOUSERUN_DEFAULT;
        mousescreenshot = MOUSESCREENSHOT_DEFAULT;
        mousestrafe = MOUSESTRAFE_DEFAULT;
        mouseuse = MOUSEUSE_DEFAULT;

        gamepadalwaysrun = GAMEPADALWAYSRUN_DEFAULT;
        gamepadautomap = GAMEPADAUTOMAP_DEFAULT;
        gamepadautomapclearmark = GAMEPADAUTOMAPCLEARMARK_DEFAULT;
        gamepadautomapfollowmode = GAMEPADAUTOMAPFOLLOWMODE_DEFAULT;
        gamepadautomapgrid = GAMEPADAUTOMAPGRID_DEFAULT;
        gamepadautomapmark = GAMEPADAUTOMAPMARK_DEFAULT;
        gamepadautomapmaxzoom = GAMEPADAUTOMAPMAXZOOM_DEFAULT;
        gamepadautomaprotatemode = GAMEPADAUTOMAPROTATEMODE_DEFAULT;
        gamepadautomapzoomin = GAMEPADAUTOMAPZOOMIN_DEFAULT;
        gamepadautomapzoomout = GAMEPADAUTOMAPZOOMOUT_DEFAULT;
        gamepadback = GAMEPADBACK_DEFAULT;
        gamepadfire = GAMEPADFIRE_DEFAULT;
        gamepadforward = GAMEPADFORWARD_DEFAULT;
        gamepadjump = GAMEPADJUMP_DEFAULT;
        gamepadleft = GAMEPADLEFT_DEFAULT;
        gamepadmenu = GAMEPADMENU_DEFAULT;
        gamepadmouselook = GAMEPADMOUSELOOK_DEFAULT;
        gamepadnextweapon = GAMEPADNEXTWEAPON_DEFAULT;
        gamepadprevweapon = GAMEPADPREVWEAPON_DEFAULT;
        gamepadright = GAMEPADRIGHT_DEFAULT;
        gamepadrun = GAMEPADRUN_DEFAULT;
        gamepadstrafe = GAMEPADSTRAFE_DEFAULT;
        gamepadstrafeleft = GAMEPADSTRAFELEFT_DEFAULT;
        gamepadstraferight = GAMEPADSTRAFERIGHT_DEFAULT;
        gamepaduse = GAMEPADUSE_DEFAULT;
        gamepaduse2 = GAMEPADUSE2_DEFAULT;
        gamepadweapon1 = GAMEPADWEAPON_DEFAULT;
        gamepadweapon2 = GAMEPADWEAPON_DEFAULT;
        gamepadweapon3 = GAMEPADWEAPON_DEFAULT;
        gamepadweapon4 = GAMEPADWEAPON_DEFAULT;
        gamepadweapon5 = GAMEPADWEAPON_DEFAULT;
        gamepadweapon6 = GAMEPADWEAPON_DEFAULT;
        gamepadweapon7 = GAMEPADWEAPON_DEFAULT;

        // clear all aliases
        for (int i = 0; i < MAXALIASES; i++)
        {
            aliases[i].name[0] = '\0';
            aliases[i].string[0] = '\0';
        }

        M_SaveCVARs();

        C_Output("All CVARs and bound controls have been reset to their defaults.");
    }
}

static void resetall_cmd_func2(char *cmd, char *parms)
{
    static char buffer[128];

    M_snprintf(buffer, sizeof(buffer), "Are you sure you want to reset all CVARs\nand bound controls to their defaults?\n\n%s",
        s_PRESSYN);
    M_StartMessage(buffer, &C_VerifyResetAll, true);
    SDL_StopTextInput();
    S_StartSound(NULL, sfx_swtchn);
}

//
// respawnitems CCMD
//
static void respawnitems_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && respawnitems)
            respawnitems = false;
        else if (value == 1 && !respawnitems)
            respawnitems = true;
        else
            return;
    }
    else
        respawnitems = !respawnitems;

    if (respawnitems)
    {
        C_Output(s_STSTR_RION);
        HU_SetPlayerMessage(s_STSTR_RION, false, false);
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_RIOFF);
        HU_SetPlayerMessage(s_STSTR_RIOFF, false, false);
    }

    message_dontfuckwithme = true;
}

//
// respawnmonsters CCMD
//
static dboolean respawnmonsters_cmd_func1(char *cmd, char *parms)
{
    return (gameskill != sk_nightmare);
}

static void respawnmonsters_cmd_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && respawnmonsters)
            respawnmonsters = false;
        else if (value == 1 && !respawnmonsters)
            respawnmonsters = true;
        else
            return;
    }
    else
        respawnmonsters = !respawnmonsters;

    if (respawnmonsters)
    {
        C_Output(s_STSTR_RMON);
        HU_SetPlayerMessage(s_STSTR_RMON, false, false);
    }
    else
    {
        C_Output(s_STSTR_RMOFF);
        HU_SetPlayerMessage(s_STSTR_RMOFF, false, false);
    }

    message_dontfuckwithme = true;
}

//
// restartmap CCMD
//
static void restartmap_cmd_func2(char *cmd, char *parms)
{
    viewplayer->playerstate = PST_REBORN;

    if (M_StringCompare(mapnum, "E1M4B") || M_StringCompare(mapnum, "E1M8B"))
        M_StringCopy(speciallumpname, mapnum, sizeof(speciallumpname));

    G_DoLoadLevel();
    C_HideConsoleFast();
}

//
// resurrect CCMD
//
static int      resurrectcmdtype = NUMMOBJTYPES;
static mobj_t   *resurrectcmdmobj;

static dboolean resurrect_cmd_func1(char *cmd, char *parms)
{
    dboolean    result = false;
    char        *parm;

    if (gamestate != GS_LEVEL)
        return false;

    parm = removenonalpha(parms);

    if (!*parm)
        return true;

    resurrectcmdmobj = NULL;

    if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        result = (viewplayer->health <= 0);
    else if (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters") || M_StringCompare(parm, "all")
        || M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
        || M_StringCompare(parm, "friendly monster") || M_StringCompare(parm, "friendly monsters"))
        result = true;
    else
    {
        for (int i = 0, num = -1; i < NUMMOBJTYPES; i++)
            if (*mobjinfo[i].name1)
            {
                char    *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
                char    *temp2 = (*mobjinfo[i].plural1 ? removenonalpha(mobjinfo[i].plural1) : NULL);
                char    *temp3 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char    *temp4 = (*mobjinfo[i].plural2 ? removenonalpha(mobjinfo[i].plural2) : NULL);
                char    *temp5 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);
                char    *temp6 = (*mobjinfo[i].plural3 ? removenonalpha(mobjinfo[i].plural3) : NULL);

                if (M_StringStartsWith(parm, "all"))
                    strreplace(parm, "all", "");

                resurrectcmdtype = mobjinfo[i].doomednum;

                if (resurrectcmdtype >= 0
                    && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                        || (*mobjinfo[i].plural1 && M_StringCompare(parm, temp2))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, temp3))
                        || (*mobjinfo[i].plural2 && M_StringCompare(parm, temp4))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, temp5))
                        || (*mobjinfo[i].plural3 && M_StringCompare(parm, temp6))
                        || (sscanf(parm, "%10d", &num) == 1 && num == resurrectcmdtype && num != -1)))
                {
                    if (resurrectcmdtype == WolfensteinSS && !allowwolfensteinss && !states[S_SSWV_STND].dehacked)
                        result = false;
                    else
                        result = mobjinfo[i].flags & MF_SHOOTABLE;
                }

                if (temp1)
                    free(temp1);

                if (temp2)
                    free(temp2);

                if (temp3)
                    free(temp3);

                if (temp4)
                    free(temp4);

                if (temp5)
                    free(temp5);

                if (temp6)
                    free(temp6);

                if (result)
                    break;
            }

        if (!result)
            for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
            {
                mobj_t  *mobj = (mobj_t *)th;

                if (*mobj->name)
                {
                    char    *temp = removenonalpha(mobj->name);

                    if (M_StringCompare(parm, temp))
                    {
                        resurrectcmdmobj = mobj;
                        result = true;
                    }

                    free(temp);

                    if (result)
                        break;
                }
            }
    }

    free(parm);
    return result;
}

static void resurrect_cmd_func2(char *cmd, char *parms)
{
    char        *parm = removenonalpha(parms);
    dboolean    cheated = false;

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, RESURRECTCMDFORMAT);
    }
    else
    {
        char    buffer[1024];

        if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        {
            P_ResurrectPlayer(initial_health);
            M_snprintf(buffer, sizeof(buffer), "%s resurrected %s.",
                playername, (M_StringCompare(playername, playername_default) ? "yourself" : "themselves"));
            buffer[0] = toupper(buffer[0]);
            C_Obituary(buffer);
            C_HideConsole();
            HU_SetPlayerMessage(buffer, false, false);
            cheated = true;
        }
        else
        {
            dboolean    friends = (M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
                            || M_StringCompare(parm, "friendly monster") || M_StringCompare(parm, "friendly monsters"));
            dboolean    enemies = (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters"));
            dboolean    all = M_StringCompare(parm, "all");
            int         resurrected = 0;

            if (friends || enemies || all)
            {
                for (int i = 0; i < numsectors; i++)
                {
                    mobj_t  *thing = sectors[i].thinglist;

                    while (thing)
                    {
                        const int   flags = thing->flags;

                        if (all || !!(flags & MF_FRIEND) == friends)
                            if ((flags & MF_CORPSE) && !(thing->flags2 & MF2_DECORATION)
                                && thing->type != MT_PLAYER && thing->info->raisestate != S_NULL)
                            {
                                P_ResurrectMobj(thing);
                                resurrected++;

                                if (flags & MF_FRIEND)
                                    cheated = true;
                            }

                        thing = thing->snext;
                    }
                }

                if (resurrected)
                {
                    char    *temp = commify(resurrected);

                    M_snprintf(buffer, sizeof(buffer), "%s%s monster%s in this map %s been resurrected.",
                        (resurrected == 1 ? "The " : "All "), temp, (resurrected == 1 ? "" : "s"), (resurrected == 1 ? "has" : "have"));
                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                    free(temp);
                }
                else
                    C_Warning(0, "There are no monsters in this map to resurrect.");
            }
            else if (resurrectcmdmobj)
            {
                char    *temp = sentencecase(parm);

                if ((resurrectcmdmobj->flags & MF_CORPSE) && !(resurrectcmdmobj->flags2 & MF2_DECORATION)
                    && resurrectcmdmobj->type != MT_PLAYER && resurrectcmdmobj->info->raisestate != S_NULL)
                    P_ResurrectMobj(resurrectcmdmobj);

                if (resurrectcmdmobj->flags & MF_FRIEND)
                    cheated = true;

                M_snprintf(buffer, sizeof(buffer), "%s was resurrected.", temp);
                C_Output(buffer);
                C_HideConsole();
                HU_SetPlayerMessage(buffer, false, false);
                message_dontfuckwithme = true;
                free(temp);
            }
            else
            {
                const mobjtype_t    type = P_FindDoomedNum(resurrectcmdtype);

                for (int i = 0; i < numsectors; i++)
                {
                    mobj_t  *thing = sectors[i].thinglist;

                    while (thing)
                    {
                        if (type == thing->type && (thing->flags & MF_CORPSE) && !(thing->flags2 & MF2_DECORATION)
                            && type != MT_PLAYER && thing->info->raisestate != S_NULL)
                        {
                            P_ResurrectMobj(thing);
                            resurrected++;

                            if (thing->flags & MF_FRIEND)
                                cheated = true;
                        }

                        thing = thing->snext;
                    }
                }

                if (resurrected)
                {
                    char    *temp = commify(resurrected);

                    M_snprintf(buffer, sizeof(buffer), "%s %s %s in this map %s been resurrected.",
                        (resurrected == 1 ? "The" : "All"), temp, (resurrected == 1 ? mobjinfo[type].name1 : mobjinfo[type].plural1),
                        (resurrected == 1 ? "has" : "have"));
                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                    free(temp);
                }
                else
                {
                    if (gamemode != commercial)
                    {
                        if (resurrectcmdtype >= ArchVile && resurrectcmdtype <= MonstersSpawner)
                        {
                            C_Warning(0, "There are no %s in <i><b>%s.</b></i>", mobjinfo[type].plural1, gamedescription);
                            return;
                        }
                        else if (gamemode == shareware && (resurrectcmdtype == Cyberdemon || resurrectcmdtype == SpiderMastermind))
                        {
                            C_Warning(0, "There are no %s in <i><b>%s.</b></i>", mobjinfo[type].plural1, gamedescription);
                            return;
                        }
                    }

                    C_Warning(0, "There are no dead %s to resurrect.", mobjinfo[type].plural1);
                }
            }
        }

        free(parm);

        if (cheated)
        {
            viewplayer->cheated++;
            stat_cheated = SafeAdd(stat_cheated, 1);
            M_SaveCVARs();
        }
    }
}

//
// save CCMD
//
static void save_cmd_func2(char *cmd, char *parms)
{
    char    buffer[1024];

    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, SAVECMDFORMAT);
        return;
    }

    M_snprintf(buffer, sizeof(buffer), "%s%s%s",
        (M_StringStartsWith(parms, savegamefolder) ? "" : savegamefolder), parms, (M_StringEndsWith(parms, ".save") ? "" : ".save"));
    G_SaveGame(-1, "", buffer);
}

//
// spawn CCMD
//
static int      spawncmdtype = NUMMOBJTYPES;
static dboolean spawncmdfriendly;

static dboolean spawn_cmd_func1(char *cmd, char *parms)
{
    dboolean    result = false;
    char        *parm = removenonalpha(parms);

    if (!*parm)
        return true;

    if (gamestate == GS_LEVEL)
    {
        int num = -1;

        if ((spawncmdfriendly = M_StringStartsWith(parm, "friendly")))
            strreplace(parm, "friendly", "");

        for (int i = 0; i < NUMMOBJTYPES; i++)
        {
            char    *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
            char    *temp2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
            char    *temp3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);

            spawncmdtype = mobjinfo[i].doomednum;

            if (spawncmdtype >= 0
                && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                || (*mobjinfo[i].name2 && M_StringCompare(parm, temp2))
                || (*mobjinfo[i].name3 && M_StringCompare(parm, temp3))
                || (sscanf(parm, "%10d", &num) == 1 && num == spawncmdtype && num != -1)))
                result = true;

            if (temp1)
                free(temp1);

            if (temp2)
                free(temp2);

            if (temp3)
                free(temp3);

            if (result)
                break;
        }
    }

    free(parm);
    return result;
}

static void spawn_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, SPAWNCMDFORMAT);
    }
    else
    {
        dboolean    spawn = true;
        mobjtype_t  type = P_FindDoomedNum(spawncmdtype);

        if (gamemode != commercial)
        {
            char    buffer[128];

            if (spawncmdtype >= ArchVile && spawncmdtype <= MonstersSpawner)
            {
                M_StringCopy(buffer, mobjinfo[type].plural1, sizeof(buffer));

                if (!*buffer)
                    M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[type].name1);

                buffer[0] = toupper(buffer[0]);
                C_Warning(0, "%s can't be spawned in <i><b>%s.</b></i>", buffer, gamedescription);
                spawn = false;
            }

            if (gamemode == shareware && (spawncmdtype == Cyberdemon || spawncmdtype == SpiderMastermind || spawncmdtype == Berserk))
            {
                M_StringCopy(buffer, mobjinfo[type].plural1, sizeof(buffer));

                if (!*buffer)
                    M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[type].name1);

                buffer[0] = toupper(buffer[0]);
                C_Warning(0, "%s can't be spawned in <i><b>%s.</b></i>", buffer, gamedescription);
                spawn = false;
            }
        }
        else if (spawncmdtype == WolfensteinSS && (!allowwolfensteinss || spawncmdfriendly) && !states[S_SSWV_STND].dehacked)
        {
            C_Warning(0, "%s Wolfenstein SS can't be spawned in %s<i><b>%s.</b></i>",
                (spawncmdfriendly ? "Friendly " : ""), (bfgedition || spawncmdfriendly ? "" : "this version of "), gamedescription);
            spawn = false;
        }

        if (spawn)
        {
            fixed_t x = viewx + 100 * viewcos;
            fixed_t y = viewy + 100 * viewsin;

            if (P_CheckLineSide(viewplayer->mo, x, y))
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You are too close to that wall.");
                else
                    C_Warning(0, "%s is too close to that wall.", playername);
            }
            else
            {
                mapthing_t  mthing;
                mobj_t      *thing;

                mthing.x = x >> FRACBITS;
                mthing.y = y >> FRACBITS;
                mthing.type = spawncmdtype;
                mthing.options = (MTF_EASY | MTF_NORMAL | MTF_HARD);

                if ((thing = P_SpawnMapThing(&mthing, true)))
                {
                    angle_t angle = R_PointToAngle2(x, y, viewx, viewy);
                    int     flags = thing->flags;

                    thing->angle = angle;
                    thing->id = thingid++;

                    if (flags & MF_SHOOTABLE)
                    {
                        if (spawncmdfriendly)
                        {
                            thing->flags |= MF_FRIEND;
                            stat_cheated = SafeAdd(stat_cheated, 1);
                            M_SaveCVARs();
                        }

                        thing->flags3 |= MF3_SPAWNEDBYPLAYER;

                        if (flags & MF_NOGRAVITY)
                        {
                            thing->z = 32 * FRACUNIT;
                            thing = P_SpawnMobj(x, y, 32 * FRACUNIT, MT_TFOG);
                        }
                        else
                            thing = P_SpawnMobj(x, y, ONFLOORZ, MT_TFOG);

                        S_StartSound(thing, sfx_telept);
                    }
                    else
                    {
                        if (flags & MF_COUNTITEM)
                        {
                            stat_cheated = SafeAdd(stat_cheated, 1);
                            M_SaveCVARs();
                        }

                        thing = P_SpawnMobj(x, y, ((flags & MF_SPAWNCEILING) ? ONCEILINGZ :
                            ((thing->flags2 & MF2_FLOATBOB) ? 14 * FRACUNIT : ONFLOORZ)), MT_IFOG);

                        S_StartSound(thing, sfx_itmbk);
                    }

                    thing->angle = ANG45 * (angle / 45);

                    C_HideConsole();
                }
            }
        }

        free(parm);
    }
}

//
// take CCMD
//
static dboolean take_cmd_func1(char *cmd, char *parms)
{
    dboolean    result = false;
    char        *parm;

    if (gamestate != GS_LEVEL)
        return false;

    parm = removenonalpha(parms);

    if (!*parm)
        return true;

    if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything")
        || M_StringCompare(parm, "health") || M_StringCompare(parm, "fullhealth")
        || M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons")
        || M_StringCompare(parm, "ammo") || M_StringCompare(parm, "fullammo")
        || M_StringCompare(parm, "armor") || M_StringCompare(parm, "fullarmor")
        || M_StringCompare(parm, "armour") || M_StringCompare(parm, "fullarmour")
        || M_StringCompare(parm, "keys") || M_StringCompare(parm, "allkeys")
        || M_StringCompare(parm, "keycards") || M_StringCompare(parm, "allkeycards")
        || M_StringCompare(parm, "skullkeys") || M_StringCompare(parm, "allskullkeys")
        || M_StringCompare(parm, "pistol"))
        result = true;
    else
        for (int i = 0, num = -1; i < NUMMOBJTYPES; i++)
        {
            char    *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
            char    *temp2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
            char    *temp3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);

            if ((mobjinfo[i].flags & MF_SPECIAL)
                && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                    || (*mobjinfo[i].name2 && M_StringCompare(parm, temp2))
                    || (*mobjinfo[i].name3 && M_StringCompare(parm, temp3))
                    || (sscanf(parm, "%10d", &num) == 1 && num == mobjinfo[i].doomednum && num != -1)))
                result = true;

            if (temp1)
                free(temp1);

            if (temp2)
                free(temp2);

            if (temp3)
                free(temp3);

            if (result)
                break;
        }

    free(parm);
    return result;
}

static void take_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, TAKECMDFORMAT);
    }
    else
    {
        dboolean    result = false;

        if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything"))
        {
            if (viewplayer->backpack)
            {
                for (ammotype_t i = 0; i < NUMAMMO; i++)
                {
                    viewplayer->maxammo[i] /= 2;
                    viewplayer->ammo[i] = MIN(viewplayer->ammo[i], viewplayer->maxammo[i]);
                }

                viewplayer->backpack = false;
                result = true;
            }

            if (viewplayer->health > initial_health)
            {
                P_DamageMobj(viewplayer->mo, viewplayer->mo, NULL, viewplayer->health - initial_health, false);
                result = true;
            }

            for (weapontype_t i = wp_shotgun; i < NUMWEAPONS; i++)
                if (viewplayer->weaponowned[i])
                {
                    viewplayer->weaponowned[i] = oldweaponsowned[i] = false;
                    result = true;
                }

            P_EquipWeapon(wp_fist);

            for (ammotype_t i = 0; i < NUMAMMO; i++)
                if (viewplayer->ammo[i])
                {
                    viewplayer->ammo[i] = 0;
                    result = true;
                }

            if (viewplayer->armorpoints)
            {
                viewplayer->armorpoints = 0;
                viewplayer->armortype = armortype_none;
                result = true;
            }

            for (int i = 0; i < NUMCARDS; i++)
                if (viewplayer->cards[i] > 0)
                {
                    viewplayer->cards[i] = 0;
                    result = true;
                }

            for (int i = 0; i < NUMPOWERS; i++)
                if (viewplayer->powers[i])
                {
                    viewplayer->powers[i] = (i == pw_allmap || i == pw_strength ? 0 : STARTFLASHING);
                    result = true;
                }

            if (result)
                C_HideConsole();
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have anything.");
            else
                C_Warning(0, "%s doesn't have anything.", playername);
        }
        else if (M_StringCompare(parm, "health") || M_StringCompare(parm, "allhealth"))
        {
            if (viewplayer->health > 0 && !(viewplayer->cheats & CF_GODMODE) && !viewplayer->powers[pw_invulnerability])
            {
                healthcvar = true;
                P_DamageMobj(viewplayer->mo, viewplayer->mo, NULL, viewplayer->health - !!(viewplayer->cheats & CF_BUDDHA), false);
                healthcvar = false;
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You are already dead.");
            else
                C_Warning(0, "%s is already dead.", playername);
        }
        else if (M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons"))
        {
            for (weapontype_t i = wp_pistol; i < NUMWEAPONS; i++)
                if (viewplayer->weaponowned[i])
                {
                    viewplayer->weaponowned[i] = false;
                    oldweaponsowned[i] = false;
                    result = true;
                }

            P_EquipWeapon(wp_fist);

            if (result)
                C_HideConsole();
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any weapons.");
            else
                C_Warning(0, "%s doesn't have any weapons.", playername);
        }
        else if (M_StringCompare(parm, "ammo") || M_StringCompare(parm, "allammo"))
        {
            for (ammotype_t i = 0; i < NUMAMMO; i++)
                if (viewplayer->ammo[i])
                {
                    viewplayer->ammo[i] = 0;
                    result = true;
                }

            P_EquipWeapon(wp_fist);

            if (result)
                C_HideConsole();
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any ammo.");
            else
                C_Warning(0, "%s doesn't have any ammo.", playername);
        }
        else if (M_StringCompare(parm, "armor") || M_StringCompare(parm, "allarmor")
                || M_StringCompare(parm, "armour") || M_StringCompare(parm, "allarmour"))
        {
            if (viewplayer->armorpoints)
            {
                viewplayer->armorpoints = 0;
                viewplayer->armortype = armortype_none;
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any armor.");
            else
                C_Warning(0, "%s doesn't have any armor.", playername);
        }
        else if (M_StringCompare(parm, "keys") || M_StringCompare(parm, "allkeys"))
        {
            for (int i = 0; i < NUMCARDS; i++)
                if (viewplayer->cards[i] > 0)
                {
                    viewplayer->cards[i] = 0;
                    result = true;
                }

            if (result)
                C_HideConsole();
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any keycards or skull keys.");
            else
                C_Warning(0, "%s doesn't have any keycards or skull keys.", playername);
        }
        else if (M_StringCompare(parm, "keycards") || M_StringCompare(parm, "allkeycards"))
        {
            if (viewplayer->cards[it_bluecard] > 0 || viewplayer->cards[it_redcard] > 0 || viewplayer->cards[it_yellowcard] > 0)
            {
                viewplayer->cards[it_bluecard] = 0;
                viewplayer->cards[it_redcard] = 0;
                viewplayer->cards[it_yellowcard] = 0;
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any keycards.");
            else
                C_Warning(0, "%s doesn't have any keycards.", playername);
        }
        else if (M_StringCompare(parm, "skullkeys") || M_StringCompare(parm, "allskullkeys"))
        {
            if (viewplayer->cards[it_blueskull] > 0 || viewplayer->cards[it_redskull] > 0 || viewplayer->cards[it_yellowskull] > 0)
            {
                viewplayer->cards[it_blueskull] = 0;
                viewplayer->cards[it_redskull] = 0;
                viewplayer->cards[it_yellowskull] = 0;
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any skull keys.");
            else
                C_Warning(0, "%s doesn't have any skull keys.", playername);
        }
        else if (M_StringCompare(parm, "pistol"))
        {
            if (viewplayer->weaponowned[wp_pistol])
            {
                viewplayer->weaponowned[wp_pistol] = false;
                oldweaponsowned[wp_pistol] = false;

                if (viewplayer->readyweapon == wp_pistol)
                    C_HideConsole();

                P_CheckAmmo(viewplayer->readyweapon);
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have a pistol.");
            else
                C_Warning(0, "%s doesn't have a pistol.", playername);
        }
        else
        {
            for (int i = 0, num = -1; i < NUMMOBJTYPES; i++)
            {
                char    *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
                char    *temp2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char    *temp3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);

                if ((mobjinfo[i].flags & MF_SPECIAL)
                    && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, temp2))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, temp3))
                        || (sscanf(parm, "%10d", &num) == 1 && num == mobjinfo[i].doomednum && num != -1)))
                {
                    if (P_TakeSpecialThing(i))
                        result = true;
                    else if (M_StringCompare(playername, playername_default))
                        C_Warning(0, "You don't have %s %s.",
                            (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                    else
                        C_Warning(0, "%s doesn't have %s %s.",
                            (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), playername, mobjinfo[i].name1);
                }

                if (temp1)
                    free(temp1);

                if (temp2)
                    free(temp2);

                if (temp3)
                    free(temp3);

                if (result)
                    break;
            }
        }

        free(parm);
    }
}

//
// teleport CCMD
//
static dboolean teleport_cmd_func1(char *cmd, char *parms)
{
    if (gamestate != GS_LEVEL)
        return false;
    else if (!*parms)
        return true;
    else
    {
        fixed_t x = FIXED_MIN;
        fixed_t y = FIXED_MIN;

        sscanf(parms, "%10d %10d", &x, &y);

        return (x != FIXED_MIN && y != FIXED_MIN);
    }
}

static void teleport_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, TELEPORTCMDFORMAT);
        return;
    }
    else
    {
        fixed_t x = FIXED_MIN;
        fixed_t y = FIXED_MIN;
        fixed_t z = FIXED_MIN;

        sscanf(parms, "%10d %10d %10d", &x, &y, &z);

        if (x != FIXED_MIN && y != FIXED_MIN)
        {
            mobj_t          *mo = viewplayer->mo;
            const fixed_t   oldx = viewx;
            const fixed_t   oldy = viewy;
            const fixed_t   oldz = viewz;

            x <<= FRACBITS;
            y <<= FRACBITS;

            if (z != ONFLOORZ)
                z <<= FRACBITS;

            if (P_TeleportMove(mo, x, y, z, false))
            {
                // spawn teleport fog at source
                mobj_t  *fog = P_SpawnMobj(oldx, oldy, oldz, MT_TFOG);

                fog->angle = viewangle;
                S_StartSound(fog, sfx_telept);
                C_HideConsole();

                // spawn teleport fog at destination
                viewplayer->viewz = mo->z + viewplayer->viewheight;
                fog = P_SpawnMobj(x + 20 * viewcos, y + 20 * viewsin, z, MT_TFOG);
                fog->angle = viewangle;
                S_StartSound(fog, sfx_telept);

                mo->reactiontime = 18;
                viewplayer->psprites[ps_weapon].sx = 0;
                viewplayer->psprites[ps_weapon].sy = WEAPONTOP;
                viewplayer->momx = 0;
                viewplayer->momy = 0;
                mo->momx = 0;
                mo->momy = 0;
                mo->momz = 0;
                viewplayer->lookdir = 0;
                viewplayer->oldlookdir = 0;
                viewplayer->recoil = 0;
                viewplayer->oldrecoil = 0;

                viewplayer->cheated++;
                stat_cheated = SafeAdd(stat_cheated, 1);
                M_SaveCVARs();
            }
        }
    }
}

//
// thinglist CCMD
//
static void thinglist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[4] = { 50, 300, 0, 0 };

    C_Header(tabs, thinglist, THINGLISTHEADER);

    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        mobj_t  *mobj = (mobj_t *)th;
        char    name[100];
        char    *temp1 = commify(mobj->id);
        char    *temp2;

        if (*mobj->name)
            M_StringCopy(name, mobj->name, sizeof(name));
        else
            M_snprintf(name, sizeof(name), "%s%s", ((mobj->flags & MF_CORPSE) && !(mobj->flags2 & MF2_DECORATION) ? "dead " :
                ((mobj->flags & MF_FRIEND) && mobj->type != MT_PLAYER ? "friendly " : ((mobj->flags & MF_DROPPED) ? "dropped " : ""))),
                (mobj->type == MT_PLAYER && mobj != viewplayer->mo ? "voodoo doll" : (*mobj->info->name1 ? mobj->info->name1 : "-")));

        temp2 = sentencecase(name);
        C_TabbedOutput(tabs, "%s%s\t%s\t(%i,%i,%i)", (mobj->id >= 0 ? temp1 : "-"), (mobj->id >= 0 ? "." : ""),
            temp2, mobj->x >> FRACBITS, mobj->y >> FRACBITS, mobj->z >> FRACBITS);
        free(temp1);
        free(temp2);
    }
}

//
// timer CCMD
//
static void timer_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, TIMERCMDFORMAT);
        return;
    }
    else
    {
        int value = INT_MAX;

        sscanf(parms, "%10d", &value);

        if (value != INT_MAX)
        {
            char    *temp = commify(value);

            value = BETWEEN(0, value, TIMERMAXMINUTES);

            if (value == timer)
                C_Output("The timer for each map has been reset to %s minute%s.", temp, (value == 1 ? "" : "s"));
            else if (value)
                C_Output("The timer for each map is now %s minute%s.", temp, (value == 1 ? "" : "s"));
            else
                C_Output("The timer for each map has been cleared.");

            P_SetTimer(value);
            free(temp);
        }
    }
}

//
// unbind CCMD
//
static void unbind_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output("<b>%s</b> %s", cmd, UNBINDCMDFORMAT);
        return;
    }

    bind_cmd_func2(cmd, parms);
}

//
// vanilla CCMD
//
static void vanilla_cmd_func2(char *cmd, char *parms)
{
    static dboolean buddha;

    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && vanilla)
            vanilla = false;
        else if (value == 1 && !vanilla)
            vanilla = true;
        else
            return;
    }
    else
        vanilla = !vanilla;

    togglingvanilla = true;

    for (int i = 0; *actions[i].action; i++)
    {
        if (actions[i].keyboard1)
            *(int *)actions[i].keyboard1 = 0;

        if (actions[i].keyboard2)
            *(int *)actions[i].keyboard2 = 0;

        if (actions[i].mouse1)
            *(int *)actions[i].mouse1 = -1;

        if (actions[i].gamepad1)
            *(int *)actions[i].gamepad1 = 0;

        if (actions[i].gamepad2)
            *(int *)actions[i].gamepad2 = 0;
    }

    if (vanilla)
    {
        SC_Open("VANILLA");

        while (SC_GetString())
        {
            char    *cvar = M_StringDuplicate(sc_String);

            if (M_StringCompare(cvar, "bind"))
            {
                if (SC_GetString())
                {
                    char    *control = M_StringDuplicate(sc_String);

                    if (SC_GetString())
                    {
                        char    *temp = M_StringJoin(control, " ", sc_String, NULL);

                        bind_cmd_func2("bind", temp);
                        free(temp);
                    }

                    free(control);
                }
            }
            else if (SC_GetString())
            {
                char    *temp = M_StringJoin(cvar, " ", sc_String, NULL);

                C_ValidateInput(temp);
                free(temp);
            }

            free(cvar);
        }

        SC_Close();

        buddha = viewplayer->cheats & CF_BUDDHA;
        viewplayer->cheats &= ~CF_BUDDHA;

        if (s_sfxvolume < s_musicvolume)
        {
            char    parm[4];

            M_snprintf(parm, sizeof(parm), "%i", s_sfxvolume);
            s_volume_cvars_func2(stringize(s_musicvolume), parm);
        }
        else if (s_sfxvolume > s_musicvolume)
        {
            char    parm[4];

            M_snprintf(parm, sizeof(parm), "%i", s_musicvolume);
            s_volume_cvars_func2(stringize(s_sfxvolume), parm);
        }

        C_Output(s_STSTR_VON);
        HU_SetPlayerMessage(s_STSTR_VON, false, false);
        C_Warning(0, "Any changes to CVARs won't be saved while vanilla mode is on.");
    }
    else
    {
        if (buddha)
            viewplayer->cheats |= CF_BUDDHA;

        M_LoadCVARs(packageconfig);
        C_Output(s_STSTR_VOFF);
        HU_SetPlayerMessage(s_STSTR_VOFF, false, false);
    }

    message_dontfuckwithme = true;
    togglingvanilla = false;

    if (gamestate == GS_LEVEL)
        C_HideConsole();
}

//
// boolean CVARs
//
static dboolean bool_cvars_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, BOOLVALUEALIAS) != INT_MIN);
}

static void bool_cvars_func2(char *cmd, char *parms)
{
    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_BOOLEAN) && !(consolecmds[i].flags & CF_READONLY))
        {
            if (*parms)
            {
                const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

                if ((value == 0 || value == 1) && value != *(dboolean *)consolecmds[i].variable)
                {
                    *(dboolean *)consolecmds[i].variable = value;
                    M_SaveCVARs();
                }
            }
            else
            {
                char    *temp1 = C_LookupAliasFromValue(*(dboolean *)consolecmds[i].variable, BOOLVALUEALIAS);

                C_ShowDescription(i);

                if (*(dboolean *)consolecmds[i].variable == (dboolean)consolecmds[i].defaultnumber)
                    C_Output(INTEGERCVARISDEFAULT, temp1);
                else
                {
                    char    *temp2 = C_LookupAliasFromValue((dboolean)consolecmds[i].defaultnumber, BOOLVALUEALIAS);

                    C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
                    free(temp2);
                }

                free(temp1);
            }

            break;
        }
}

//
// color CVARs
//

static struct
{
    char    *name;
    int     value;
} color[] = {
    { "black",       0 }, { "blue",      207 }, { "brick",      32 }, { "brown",      64 },
    { "cream",      48 }, { "darkbrown",  79 }, { "darkgray",  111 }, { "darkgrey",  111 },
    { "darkgreen", 127 }, { "darkred",   191 }, { "gold",      163 }, { "gray",       95 },
    { "grey",       95 }, { "green",     112 }, { "lightblue", 193 }, { "olive",     152 },
    { "orange",    216 }, { "purple",    254 }, { "red",       176 }, { "tan",       144 },
    { "white",       4 }, { "yellow",    231 }, { "",            0 }
};

static dboolean color_cvars_func1(char *cmd, char *parms)
{
    char        *temp;
    dboolean    result = false;

    for (int i = 0; *color[i].name; i++)
        if (M_StringCompare(parms, color[i].name))
            return true;
    temp = M_SubString(parms, 1, 6);
    result = ((strlen(parms) == 7 && parms[0] == '#' && hextodec(temp) >= 0) || int_cvars_func1(cmd, parms));
    free(temp);
    return result;
}

static void color_cvars_func2(char *cmd, char *parms)
{
    char    buffer[8];

    for (int i = 0; *color[i].name; i++)
        if (M_StringCompare(parms, color[i].name))
        {
            M_snprintf(buffer, sizeof(buffer), "%i", nearestcolors[color[i].value]);
            int_cvars_func2(cmd, buffer);
            AM_SetColors();
            return;
        }

    if (strlen(parms) == 7 && parms[0] == '#')
    {
        char    *temp1 = M_SubString(parms, 1, 2);
        char    *temp2 = M_SubString(parms, 3, 2);
        char    *temp3 = M_SubString(parms, 5, 2);

        M_snprintf(buffer, sizeof(buffer), "%i", FindNearestColor(PLAYPAL, hextodec(temp1), hextodec(temp2), hextodec(temp3)));
        int_cvars_func2(cmd, buffer);
        free(temp1);
        free(temp2);
        free(temp3);
    }
    else
        int_cvars_func2(cmd, parms);

    if (*parms)
        AM_SetColors();
}

//
// float CVARs
//
static dboolean float_cvars_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_FLOAT))
        {
            float   value = FLT_MIN;

            sscanf(parms, "%10f", &value);
            return (value != FLT_MIN);
        }

    return false;
}

//
// integer CVARs
//
static dboolean int_cvars_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_INTEGER))
        {
            int value = C_LookupValueFromAlias(parms, consolecmds[i].aliases);

            if (value == INT_MIN)
                sscanf(parms, "%10d", &value);

            return (value >= consolecmds[i].minimumvalue && value <= consolecmds[i].maximumvalue);
        }

    return false;
}

static void int_cvars_func2(char *cmd, char *parms)
{
    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_INTEGER))
        {
            if (*parms && !(consolecmds[i].flags & CF_READONLY))
            {
                int value = C_LookupValueFromAlias(parms, consolecmds[i].aliases);

                if (value == INT_MIN)
                    sscanf(parms, "%10d", &value);

                if (value != INT_MIN && value != *(int *)consolecmds[i].variable)
                {
                    *(int *)consolecmds[i].variable = value;
                    M_SaveCVARs();
                }
            }
            else
            {
                C_ShowDescription(i);

                if (consolecmds[i].flags & CF_PERCENT)
                {
                    char    *temp1 = commify(*(int *)consolecmds[i].variable);

                    if (consolecmds[i].flags & CF_READONLY)
                        C_Output(PERCENTCVARISREADONLY, temp1);
                    else if (*(int *)consolecmds[i].variable == (int)consolecmds[i].defaultnumber)
                        C_Output(PERCENTCVARISDEFAULT, temp1);
                    else
                    {
                        char    *temp2 = commify((int)consolecmds[i].defaultnumber);

                        C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
                        free(temp2);
                    }

                    free(temp1);
                }
                else
                {
                    char    *temp1 = C_LookupAliasFromValue(*(int *)consolecmds[i].variable, consolecmds[i].aliases);

                    if (consolecmds[i].flags & CF_READONLY)
                        C_Output(INTEGERCVARISREADONLY, temp1);
                    else if (*(int *)consolecmds[i].variable == (int)consolecmds[i].defaultnumber)
                        C_Output(INTEGERCVARISDEFAULT, temp1);
                    else
                    {
                        char    *temp2 = C_LookupAliasFromValue((int)consolecmds[i].defaultnumber, consolecmds[i].aliases);

                        C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
                        free(temp2);
                    }

                    free(temp1);
                }
            }

            break;
        }
}

//
// string CVARs
//
static void str_cvars_func2(char *cmd, char *parms)
{
    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_STRING))
        {
            if (M_StringCompare(parms, EMPTYVALUE) && **(char **)consolecmds[i].variable && !(consolecmds[i].flags & CF_READONLY))
            {
                *(char **)consolecmds[i].variable = "";
                M_SaveCVARs();
            }
            else if (*parms)
            {
                if (!M_StringCompare(parms, *(char **)consolecmds[i].variable) && !(consolecmds[i].flags & CF_READONLY))
                {
                    M_StripQuotes(parms);
                    *(char **)consolecmds[i].variable = M_StringDuplicate(parms);
                    M_SaveCVARs();
                }
            }
            else
            {
                C_ShowDescription(i);

                if (consolecmds[i].flags & CF_READONLY)
                    C_Output(STRINGCVARISREADONLY,
                        (M_StringCompare(consolecmds[i].name, "version") ? "" : "\""), *(char **)consolecmds[i].variable,
                        (M_StringCompare(consolecmds[i].name, "version") ? "" : "\""));
                else if (M_StringCompare(*(char **)consolecmds[i].variable, consolecmds[i].defaultstring))
                    C_Output(STRINGCVARISDEFAULT, *(char **)consolecmds[i].variable);
                else
                    C_Output(STRINGCVARWITHDEFAULT, *(char **)consolecmds[i].variable, consolecmds[i].defaultstring);
            }

            break;
        }
}

//
// time CVARs
//
static void time_cvars_func2(char *cmd, char *parms)
{
    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_TIME))
        {
            const int   tics = *(int *)consolecmds[i].variable / TICRATE;

            C_ShowDescription(i);

            C_Output(TIMECVARISREADONLY, tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60);
            break;
        }
}

//
// alwaysrun CVAR
//
static void alwaysrun_cvar_func2(char *cmd, char *parms)
{
    bool_cvars_func2(cmd, parms);
    I_InitKeyboard();
}

//
// am_external CVAR
//
static void am_external_cvar_func2(char *cmd, char *parms)
{
    const dboolean  am_external_old = am_external;

    bool_cvars_func2(cmd, parms);

    if (am_external != am_external_old)
    {
        if (am_external)
        {
            I_CreateExternalAutomap(1);
            am_followmode = true;

            if (gamestate == GS_LEVEL)
                AM_Start(false);
        }
        else
        {
            I_DestroyExternalAutomap();
            mapscreen = *screens;

            if (gamestate == GS_LEVEL)
                AM_Stop();
        }
    }
}

//
// am_followmode CVAR
//
static dboolean am_followmode_cvar_func1(char *cmd, char *parms)
{
    return (!mapwindow && gamestate == GS_LEVEL);
}

//
// am_gridsize CVAR
//
static void am_gridsize_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        am_gridsize = M_StringDuplicate(parms);
        AM_GetGridSize();

        if (!M_StringCompare(am_gridsize, parms))
            M_SaveCVARs();
    }
    else
    {
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(am_gridsize, am_gridsize_default))
            C_Output(INTEGERCVARISDEFAULT, am_gridsize);
        else
            C_Output(INTEGERCVARWITHDEFAULT, am_gridsize, am_gridsize_default);
    }
}

//
// armortype CVAR
//
static dboolean armortype_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || (C_LookupValueFromAlias(parms, ARMORTYPEVALUEALIAS) != INT_MIN && gamestate == GS_LEVEL));
}

static void armortype_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, ARMORTYPEVALUEALIAS);

        if (value != INT_MIN && viewplayer->armorpoints)
        {
            viewplayer->armortype = value;

            if (value == armortype_none)
                viewplayer->armorpoints = 0;
        }
    }
    else
    {
        C_ShowDescription(C_GetIndex(cmd));

        if (gamestate == GS_LEVEL)
        {
            char    *temp = C_LookupAliasFromValue(viewplayer->armortype, ARMORTYPEVALUEALIAS);

            C_Output(INTEGERCVARWITHNODEFAULT, temp);
            free(temp);
        }
    }
}

//
// autotilt CVAR
//
static void autotilt_cvar_func2(char *cmd, char *parms)
{
    const dboolean  autotilt_old = autotilt;

    bool_cvars_func2(cmd, parms);

    if (autotilt != autotilt_old && gamestate == GS_LEVEL)
    {
        R_InitSkyMap();
        R_InitColumnFunctions();
    }
}

//
// crosshair CVAR
//
static dboolean crosshair_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, CROSSHAIRVALUEALIAS) != INT_MIN);
}

static void crosshair_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, CROSSHAIRVALUEALIAS);

        if (value != INT_MIN && crosshair != value)
        {
            crosshair = value;
            M_SaveCVARs();
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(crosshair, CROSSHAIRVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (crosshair == crosshair_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(crosshair_default, CROSSHAIRVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// episode CVAR
//
static void episode_cvar_func2(char *cmd, char *parms)
{
    const int   episode_old = episode;

    int_cvars_func2(cmd, parms);

    if (episode != episode_old && gamemode != commercial)
        EpiDef.lastOn = (gamemode == registered ? MIN(episode, episode_max - 1) : (gamemode == retail ? episode : 1)) - 1;
}

//
// expansion CVAR
//
static void expansion_cvar_func2(char *cmd, char *parms)
{
    const int   expansion_old = expansion;

    int_cvars_func2(cmd, parms);

    if (expansion != expansion_old && gamemode == commercial)
    {
        ExpDef.lastOn = (nerve ? expansion : 1) - 1;

        if (gamestate != GS_LEVEL)
            gamemission = (expansion == 2 && nerve ? pack_nerve : doom2);
    }
}

//
// gp_deadzone_left and gp_deadzone_right CVARs
//
static dboolean gp_deadzone_cvars_func1(char *cmd, char *parms)
{
    float   value;
    int     result;

    if (!*parms)
        return true;

    if ((result = sscanf(parms, "%10f%%", &value)) != 1)
        result = sscanf(parms, "%10f", &value);

    return result;
}

static void gp_deadzone_cvars_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        float   value = 0;

        if (sscanf(parms, "%10f%%", &value) != 1)
            sscanf(parms, "%10f", &value);

        if (M_StringCompare(cmd, stringize(gp_deadzone_left)))
        {
            if (gp_deadzone_left != value)
            {
                gp_deadzone_left = BETWEENF(gp_deadzone_left_min, value, gp_deadzone_left_max);
                I_SetGamepadLeftDeadZone();
                M_SaveCVARs();
            }
        }
        else if (gp_deadzone_right != value)
        {
            gp_deadzone_right = BETWEENF(gp_deadzone_right_min, value, gp_deadzone_right_max);
            I_SetGamepadRightDeadZone();
            M_SaveCVARs();
        }
    }
    else if (M_StringCompare(cmd, stringize(gp_deadzone_left)))
    {
        char    *temp1 = striptrailingzero(gp_deadzone_left, 1);

        C_ShowDescription(C_GetIndex(cmd));

        if (gp_deadzone_left == gp_deadzone_left_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = striptrailingzero(gp_deadzone_left_default, 1);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
        }
    }
    else
    {
        char    *temp1 = striptrailingzero(gp_deadzone_right, 1);

        C_ShowDescription(C_GetIndex(cmd));

        if (gp_deadzone_right == gp_deadzone_right_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = striptrailingzero(gp_deadzone_right_default, 1);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
        }
    }
}

//
// gp_sensitivity_horizontal and gp_sensitivity_vertical CVARs
//
static void gp_sensitivity_cvars_func2(char *cmd, char *parms)
{
    const int   gp_sensitivity_horizontal_old = gp_sensitivity_horizontal;
    const int   gp_sensitivity_vertical_old = gp_sensitivity_vertical;

    int_cvars_func2(cmd, parms);

    if (gp_sensitivity_horizontal != gp_sensitivity_horizontal_old)
        I_SetGamepadHorizontalSensitivity();
    else if (gp_sensitivity_vertical != gp_sensitivity_vertical_old)
        I_SetGamepadVerticalSensitivity();
}

//
// mouselook CVAR
//
static void mouselook_cvar_func2(char *cmd, char *parms)
{
    const dboolean  mouselook_old = mouselook;

    bool_cvars_func2(cmd, parms);

    if (mouselook != mouselook_old)
    {
        if (gamestate == GS_LEVEL)
        {
            R_InitSkyMap();
            R_InitColumnFunctions();

            if (!mouselook)
            {
                viewplayer->lookdir = 0;
                viewplayer->oldlookdir = 0;
                viewplayer->recoil = 0;
                viewplayer->oldrecoil = 0;
            }
        }
    }
}

//
// ammo, armor and health CVARs
//
static dboolean player_cvars_func1(char *cmd, char *parms)
{
    return (!*parms || (int_cvars_func1(cmd, parms) && gamestate == GS_LEVEL));
}

static void player_cvars_func2(char *cmd, char *parms)
{
    int value;

    if (resettingcvar)
        return;

    if (M_StringCompare(cmd, stringize(ammo)))
    {
        weapontype_t    readyweapon = viewplayer->readyweapon;
        ammotype_t      ammotype = weaponinfo[readyweapon].ammotype;

        if (*parms)
        {
            sscanf(parms, "%10d", &value);

            if (ammotype != am_noammo && value != viewplayer->ammo[ammotype] && viewplayer->health > 0)
            {
                if (value > viewplayer->ammo[ammotype])
                {
                    P_UpdateAmmoStat(ammotype, value - viewplayer->ammo[ammotype]);
                    ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;
                    P_AddBonus();
                    S_StartSound(NULL, sfx_itemup);
                }

                viewplayer->ammo[ammotype] = MIN(value, viewplayer->maxammo[ammotype]);
                P_CheckAmmo(readyweapon);
                C_HideConsole();
            }
        }
        else
        {
            C_ShowDescription(C_GetIndex(cmd));

            if (gamestate == GS_LEVEL)
            {
                char    *temp = commify(ammotype == am_noammo ? 0 : viewplayer->ammo[ammotype]);

                C_Output(INTEGERCVARWITHNODEFAULT, temp);
                free(temp);
            }
        }
    }
    else if (M_StringCompare(cmd, stringize(armor)))
    {
        if (*parms)
        {
            sscanf(parms, "%10d", &value);

            if (value != viewplayer->armorpoints)
            {
                if (value > viewplayer->armorpoints)
                {
                    P_UpdateArmorStat(value - viewplayer->armorpoints);
                    armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;
                    P_AddBonus();
                    S_StartSound(NULL, sfx_itemup);
                }

                viewplayer->armorpoints = MIN(value, max_armor);

                if (!viewplayer->armortype)
                    viewplayer->armortype = armortype_green;

                C_HideConsole();
            }
        }
        else
        {
            C_ShowDescription(C_GetIndex(cmd));

            if (gamestate == GS_LEVEL)
            {
                char    *temp = commify(viewplayer->armorpoints);

                C_Output(PERCENTCVARWITHNODEFAULT, temp);
                free(temp);
            }
        }
    }
    else if (M_StringCompare(cmd, stringize(health)) && !(viewplayer->cheats & CF_GODMODE) && !viewplayer->powers[pw_invulnerability])
    {
        if (*parms)
        {
            sscanf(parms, "%10d", &value);
            value = BETWEEN(health_min, value, maxhealth);

            if (value != viewplayer->health)
            {
                if (viewplayer->health <= 0)
                {
                    if (value <= 0)
                    {
                        if (value < viewplayer->health)
                            viewplayer->damagecount = viewplayer->health - value;
                        else
                            S_StartSound(NULL, sfx_itemup);

                        viewplayer->health = value;
                        viewplayer->mo->health = value;
                        healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
                    }
                    else
                    {
                        P_ResurrectPlayer(value);
                        P_AddBonus();
                        S_StartSound(NULL, sfx_itemup);
                    }
                }
                else
                {
                    if (value < viewplayer->health)
                    {
                        healthcvar = true;
                        P_DamageMobj(viewplayer->mo, viewplayer->mo, viewplayer->mo, viewplayer->health - value, false);
                        healthcvar = false;
                    }
                    else
                    {
                        P_UpdateHealthStat(value - viewplayer->health);
                        viewplayer->health = value;
                        viewplayer->mo->health = value;
                        healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
                        P_AddBonus();
                        S_StartSound(NULL, sfx_itemup);
                    }

                    C_HideConsole();
                }
            }
        }
        else
        {
            C_ShowDescription(C_GetIndex(cmd));

            if (gamestate == GS_LEVEL)
            {
                char    *temp = commify(viewplayer->health);

                C_Output(PERCENTCVARWITHNODEFAULT, temp);
                free(temp);
            }
        }
    }
}

//
// playername CVAR
//
static void playername_cvar_func2(char *cmd, char *parms)
{
    str_cvars_func2(cmd, (M_StringCompare(parms, EMPTYVALUE) ? playername_default : parms));
}

//
// r_blood CVAR
//
static dboolean r_blood_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, BLOODVALUEALIAS) != INT_MIN);
}

static void r_blood_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BLOODVALUEALIAS);

        if (value != INT_MIN && r_blood != value)
        {
            r_blood = value;
            M_SaveCVARs();
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_blood, BLOODVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_blood == r_blood_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_blood_default, BLOODVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_bloodsplats_translucency CVAR
//
static void r_bloodsplats_translucency_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_bloodsplats_translucency)
        {
            r_bloodsplats_translucency = value;
            M_SaveCVARs();
            R_InitColumnFunctions();

            for (int i = 0; i < numsectors; i++)
            {
                bloodsplat_t    *splat = sectors[i].splatlist;

                while (splat)
                {
                    splat->colfunc = (splat->blood == FUZZYBLOOD ? fuzzcolfunc : bloodsplatcolfunc);
                    splat = splat->snext;
                }
            }
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_bloodsplats_translucency, BOOLVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_bloodsplats_translucency == r_bloodsplats_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_bloodsplats_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_color CVAR
//
static void r_color_cvar_func2(char *cmd, char *parms)
{
    const int   r_color_old = r_color;

    int_cvars_func2(cmd, parms);

    if (r_color != r_color_old)
    {
        I_SetPalette(&PLAYPAL[st_palette * 768]);
        M_SaveCVARs();
    }
}

//
// r_detail CVAR
//
static dboolean r_detail_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, DETAILVALUEALIAS) != INT_MIN);
}

static void r_detail_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, DETAILVALUEALIAS);

        if ((value == r_detail_low || value == r_detail_high) && r_detail != value)
        {
            r_detail = value;
            STLib_Init();
            M_SaveCVARs();
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_detail, DETAILVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_detail == r_detail_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_detail_default, DETAILVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_dither CVAR
//
static void r_dither_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_dither)
        {
            r_dither = value;
            M_SaveCVARs();
            R_InitColumnFunctions();
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_dither, BOOLVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_dither == r_dither_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_dither_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_fixmaperrors CVAR
//
static void r_fixmaperrors_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_fixmaperrors)
        {
            r_fixmaperrors = value;
            M_SaveCVARs();

            if (gamestate == GS_LEVEL && !togglingvanilla && !resettingcvar)
                C_Warning(0, PENDINGCHANGE);
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_fixmaperrors, BOOLVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_fixmaperrors == r_fixmaperrors_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_fixmaperrors_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_fov CVAR
//
static void r_fov_cvar_func2(char *cmd, char *parms)
{
    const int   r_fov_old = r_fov;

    int_cvars_func2(cmd, parms);

    if (r_fov != r_fov_old)
    {
        setsizeneeded = true;
        R_InitLightTables();
        S_StartSound(NULL, sfx_stnmov);
    }
}

//
// r_gamma CVAR
//
static dboolean r_gamma_cvar_func1(char *cmd, char *parms)
{
    return (C_LookupValueFromAlias(parms, GAMMAVALUEALIAS) != INT_MIN || float_cvars_func1(cmd, parms));
}

static void r_gamma_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        float   value = (float)C_LookupValueFromAlias(parms, GAMMAVALUEALIAS);

        if (value == INT_MIN)
            sscanf(parms, "%10f", &value);

        if (value != INT_MIN && value != r_gamma)
        {
            r_gamma = BETWEENF(r_gamma_min, value, r_gamma_max);
            I_SetGamma(r_gamma);
            I_SetPalette(&PLAYPAL[st_palette * 768]);
            M_SaveCVARs();
        }
    }
    else
    {
        char    buffer1[128];
        int     len;

        M_snprintf(buffer1, sizeof(buffer1), "%.2f", r_gamma);
        len = (int)strlen(buffer1);

        if (len >= 2 && buffer1[len - 1] == '0' && buffer1[len - 2] == '0')
            buffer1[len - 1] = '\0';

        C_ShowDescription(C_GetIndex(cmd));

        if (r_gamma == r_gamma_default)
            C_Output(INTEGERCVARISDEFAULT, (r_gamma == 1.0f ? "off" : buffer1));
        else
        {
            char    buffer2[128];

            M_snprintf(buffer2, sizeof(buffer2), "%.2f", r_gamma_default);
            len = (int)strlen(buffer2);

            if (len >= 2 && buffer2[len - 1] == '0' && buffer2[len - 2] == '0')
                buffer2[len - 1] = '\0';

            C_Output(INTEGERCVARWITHDEFAULT, (r_gamma == 1.0f ? "off" : buffer1), (r_gamma_default == 1.0f ? "off" : buffer2));
        }
    }
}

//
// r_hud CVAR
//
static void r_hud_cvar_func2(char *cmd, char *parms)
{
    if (vid_widescreen || !*parms || resettingcvar)
        bool_cvars_func2(cmd, parms);
}

//
// r_hud_translucency CVAR
//
static void r_hud_translucency_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_hud_translucency)
        {
            r_hud_translucency = value;
            M_SaveCVARs();
            HU_SetTranslucency();
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_hud_translucency, BOOLVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_hud_translucency == r_hud_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_hud_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_lowpixelsize CVAR
//
static void r_lowpixelsize_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        r_lowpixelsize = M_StringDuplicate(parms);
        GetPixelSize(false);

        if (!M_StringCompare(r_lowpixelsize, parms))
            M_SaveCVARs();
    }
    else
    {
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(r_lowpixelsize, r_lowpixelsize_default))
            C_Output(INTEGERCVARISDEFAULT, r_lowpixelsize);
        else
            C_Output(INTEGERCVARWITHDEFAULT, r_lowpixelsize, r_lowpixelsize_default);
    }
}

//
// r_screensize CVAR
//
static void r_screensize_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = parms[0] - '0';

        if (strlen(parms) == 1 && value >= r_screensize_min && value <= r_screensize_max && value != r_screensize)
        {
            if (vid_widescreen || (returntowidescreen && gamestate != GS_LEVEL))
            {
                if (value < r_screensize_max)
                {
                    r_hud = true;
                    I_ToggleWidescreen(false);
                }
            }

            r_screensize = value;
            M_SaveCVARs();
            R_SetViewSize(r_screensize);

            if (r_playersprites)
                skippsprinterp = true;
        }
    }
    else
    {
        char    *temp1 = commify(r_screensize);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_screensize == r_screensize_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = commify(r_screensize_default);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_shadows_translucency CVAR
//
static void r_shadows_translucency_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_shadows_translucency)
        {
            r_shadows_translucency = value;
            M_SaveCVARs();

            for (int i = 0; i < numsectors; i++)
            {
                mobj_t  *mo = sectors[i].thinglist;

                while (mo)
                {
                    P_SetShadowColumnFunction(mo);
                    mo = mo->snext;
                }
            }
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_shadows_translucency, BOOLVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_shadows_translucency == r_shadows_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_shadows_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_skycolor CVAR
//
static dboolean r_skycolor_cvar_func1(char *cmd, char *parms)
{
    return (C_LookupValueFromAlias(parms, SKYVALUEALIAS) == r_skycolor_none || color_cvars_func1(cmd, parms));
}

static void r_skycolor_cvar_func2(char *cmd, char *parms)
{
    const int   value = C_LookupValueFromAlias(parms, SKYVALUEALIAS);

    if (value != INT_MIN)
    {
        if (value != r_skycolor)
        {
            r_skycolor = r_skycolor_none;
            M_SaveCVARs();
            R_InitColumnFunctions();
        }
    }
    else
    {
        const int   r_skycolor_old = r_skycolor;

        color_cvars_func2(cmd, parms);

        if (r_skycolor != r_skycolor_old)
            R_InitColumnFunctions();
    }
}

//
// r_textures CVAR
//
static void r_textures_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_textures)
        {
            r_textures = value;
            M_SaveCVARs();
            R_InitColumnFunctions();

            for (int i = 0; i < numsectors; i++)
            {
                mobj_t          *mo = sectors[i].thinglist;
                bloodsplat_t    *splat = sectors[i].splatlist;

                while (mo)
                {
                    mo->colfunc = mo->info->colfunc;
                    P_SetShadowColumnFunction(mo);
                    mo = mo->snext;
                }

                while (splat)
                {
                    splat->colfunc = (splat->blood == FUZZYBLOOD ? fuzzcolfunc : bloodsplatcolfunc);
                    splat = splat->snext;
                }
            }
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_textures, BOOLVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_textures == r_textures_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_textures_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// r_translucency CVAR
//
static void r_translucency_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_translucency)
        {
            r_translucency = value;
            M_SaveCVARs();
            R_InitColumnFunctions();

            for (int i = 0; i < numsectors; i++)
            {
                mobj_t  *mo = sectors[i].thinglist;

                while (mo)
                {
                    mo->colfunc = mo->info->colfunc;
                    mo = mo->snext;
                }
            }
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(r_translucency, BOOLVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (r_translucency == r_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// s_musicvolume and s_sfxvolume CVARs
//
static dboolean s_volume_cvars_func1(char *cmd, char *parms)
{
    int value = INT_MIN;

    if (!*parms)
        return true;

    if (sscanf(parms, "%10d%%", &value) != 1)
        sscanf(parms, "%10d", &value);

    return ((M_StringCompare(cmd, stringize(s_musicvolume)) && value >= s_musicvolume_min
        && value <= s_musicvolume_max) || (M_StringCompare(cmd, stringize(s_sfxvolume))
        && value >= s_sfxvolume_min && value <= s_sfxvolume_max));
}

static void s_volume_cvars_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        int value = INT_MIN;

        if (sscanf(parms, "%10d%%", &value) != 1)
            sscanf(parms, "%10d", &value);

        if (M_StringCompare(cmd, stringize(s_musicvolume)) && s_musicvolume != value)
        {
            s_musicvolume = value;
            musicVolume = (s_musicvolume * 31 + 50) / 100;
            S_LowerMusicVolume();
            M_SaveCVARs();
        }
        else if (s_sfxvolume != value)
        {
            s_sfxvolume = value;
            sfxVolume = (s_sfxvolume * 31 + 50) / 100;
            S_SetSfxVolume(sfxVolume * MIX_MAX_VOLUME / 31);
            M_SaveCVARs();
        }
    }
    else if (M_StringCompare(cmd, stringize(s_musicvolume)))
    {
        char    *temp1 = commify(s_musicvolume);

        C_ShowDescription(C_GetIndex(cmd));

        if (s_musicvolume == s_musicvolume_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = commify(s_musicvolume_default);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
    else
    {
        char    *temp1 = commify(s_sfxvolume);

        C_ShowDescription(C_GetIndex(cmd));

        if (s_sfxvolume == s_sfxvolume_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = commify(s_sfxvolume_default);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// savegame CVAR
//
static void savegame_cvar_func2(char *cmd, char *parms)
{
    const int   savegame_old = savegame;

    int_cvars_func2(cmd, parms);

    if (savegame != savegame_old)
    {
        SaveDef.lastOn = savegame - 1;
        LoadDef.lastOn = savegame - 1;
    }
}

//
// skilllevel CVAR
//
static void skilllevel_cvar_func2(char *cmd, char *parms)
{
    const int   skilllevel_old = skilllevel;

    int_cvars_func2(cmd, parms);

    if (skilllevel != skilllevel_old)
    {
        pendinggameskill = skilllevel;
        NewDef.lastOn = skilllevel - 1;

        if (gamestate == GS_LEVEL && !resettingcvar)
            C_Warning(0, PENDINGCHANGE);
    }
}

//
// turbo CVAR
//
static dboolean turbo_cvar_func1(char *cmd, char *parms)
{
    int value = INT_MIN;

    if (!*parms)
        return true;

    sscanf(parms, "%10d", &value);
    return (value >= turbo_min && value <= turbo_max);
}

static void turbo_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        int value = INT_MIN;

        sscanf(parms, "%10d", &value);

        if (value >= turbo_min && value <= turbo_max && value != turbo)
        {
            turbo = value;

            if (turbo > turbo_default)
            {
                viewplayer->cheated++;
                stat_cheated = SafeAdd(stat_cheated, 1);
            }

            M_SaveCVARs();
            G_SetMovementSpeed(turbo);
        }
    }
    else
    {
        char    *temp1 = commify(turbo);

        C_ShowDescription(C_GetIndex(cmd));

        if (turbo == turbo_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = commify(turbo_default);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// units CVAR
//
static dboolean units_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, UNITSVALUEALIAS) != INT_MIN);
}

static void units_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, UNITSVALUEALIAS);

        if ((value == units_imperial || value == units_metric) && value != units)
        {
            units = value;
            M_SaveCVARs();
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(units, UNITSVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (units == units_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(units_default, UNITSVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// vid_borderlesswindow CVAR
//
static void vid_borderlesswindow_cvar_func2(char *cmd, char *parms)
{
    const dboolean  vid_borderlesswindow_old = vid_borderlesswindow;

    bool_cvars_func2(cmd, parms);

    if (vid_borderlesswindow != vid_borderlesswindow_old && vid_fullscreen)
        I_RestartGraphics();
}

//
// vid_capfps CVAR
//
static dboolean vid_capfps_cvar_func1(char *cmd, char *parms)
{
    return (C_LookupValueFromAlias(parms, CAPVALUEALIAS) != INT_MIN || int_cvars_func1(cmd, parms));
}

static void vid_capfps_cvar_func2(char *cmd, char *parms)
{
    const int   value = C_LookupValueFromAlias(parms, CAPVALUEALIAS);

    if (value != INT_MIN)
    {
        if (value != vid_capfps)
        {
            vid_capfps = value;
            M_SaveCVARs();
            I_CapFPS(vid_capfps);
        }
    }
    else
    {
        const int   vid_capfps_old = vid_capfps;

        int_cvars_func2(cmd, parms);

        if (vid_capfps != vid_capfps_old)
        {
            if (vid_capfps < 10)
            {
                vid_capfps = vid_capfps_old;
                M_SaveCVARs();
            }
            else
                I_CapFPS(vid_capfps);
        }
    }
}

//
// vid_display CVAR
//
static void vid_display_cvar_func2(char *cmd, char *parms)
{
    const int   vid_display_old = vid_display;

    int_cvars_func2(cmd, parms);

    if (vid_display != vid_display_old)
        I_RestartGraphics();
}

//
// vid_fullscreen CVAR
//
static void vid_fullscreen_cvar_func2(char *cmd, char *parms)
{
    const dboolean  vid_fullscreen_old = vid_fullscreen;

    bool_cvars_func2(cmd, parms);

    if (vid_fullscreen != vid_fullscreen_old)
        I_ToggleFullscreen();
}

//
// vid_pillarboxes CVAR
//
static void vid_pillarboxes_cvar_func2(char *cmd, char *parms)
{
    const dboolean  vid_pillarboxes_old = vid_pillarboxes;

    bool_cvars_func2(cmd, parms);

    if (vid_pillarboxes != vid_pillarboxes_old)
        I_SetPillarboxes();
}

//
// vid_scaleapi CVAR
//
static dboolean vid_scaleapi_cvar_func1(char *cmd, char *parms)
{
    return (!*parms
#if defined(_WIN32)
        || M_StringCompare(parms, vid_scaleapi_direct3d)
#elif defined(__APPLE__)
        || M_StringCompare(parms, vid_scaleapi_metal)
#endif
        || M_StringCompare(parms, vid_scaleapi_opengl)
#if !defined(_WIN32)
        || M_StringCompare(parms, vid_scaleapi_opengles)
        || M_StringCompare(parms, vid_scaleapi_opengles2)
#endif
        || M_StringCompare(parms, vid_scaleapi_software));
}

static void vid_scaleapi_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        if (!M_StringCompare(parms, vid_scaleapi))
        {
            vid_scaleapi = M_StringDuplicate(parms);
            M_SaveCVARs();
            I_RestartGraphics();
        }
    }
    else
    {
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(vid_scaleapi, vid_scaleapi_default))
            C_Output(STRINGCVARISDEFAULT, vid_scaleapi);
        else
            C_Output(STRINGCVARWITHDEFAULT, vid_scaleapi, vid_scaleapi_default);
    }
}

//
// vid_scalefilter CVAR
//
static dboolean vid_scalefilter_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || M_StringCompare(parms, vid_scalefilter_nearest)
        || M_StringCompare(parms, vid_scalefilter_linear)
        || M_StringCompare(parms, vid_scalefilter_nearest_linear));
}

static void vid_scalefilter_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        if (!M_StringCompare(parms, vid_scalefilter))
        {
            vid_scalefilter = M_StringDuplicate(parms);
            M_SaveCVARs();
            I_RestartGraphics();
        }
    }
    else
    {
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(vid_scalefilter, vid_scalefilter_default))
            C_Output(STRINGCVARISDEFAULT, vid_scalefilter);
        else
            C_Output(STRINGCVARWITHDEFAULT, vid_scalefilter, vid_scalefilter_default);
    }
}

//
// vid_screenresolution CVAR
//
static void vid_screenresolution_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        if (!M_StringCompare(vid_screenresolution, parms))
        {
            vid_screenresolution = M_StringDuplicate(parms);
            GetScreenResolution();
            M_SaveCVARs();

            if (vid_fullscreen)
                I_RestartGraphics();
        }
    }
    else
    {
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(vid_screenresolution, vid_screenresolution_default))
            C_Output(INTEGERCVARISDEFAULT, vid_screenresolution);
        else
            C_Output(INTEGERCVARWITHDEFAULT, vid_screenresolution, vid_screenresolution_default);
    }
}

//
// vid_showfps CVAR
//
extern uint64_t starttime;
extern int      frames;

static void vid_showfps_cvar_func2(char *cmd, char *parms)
{
    const dboolean  vid_showfps_old = vid_showfps;

    bool_cvars_func2(cmd, parms);

    if (vid_showfps != vid_showfps_old)
    {
        I_UpdateBlitFunc(viewplayer->damagecount);

        if (vid_showfps)
            starttime = SDL_GetPerformanceCounter();
        else
            frames = -1;
    }
}

//
// vid_vsync CVAR
//
static dboolean vid_vsync_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, VSYNCVALUEALIAS) != INT_MIN);
}

static void vid_vsync_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, VSYNCVALUEALIAS);

        if (value != INT_MIN && vid_vsync != value)
        {
            vid_vsync = value;
            I_RestartGraphics();
            M_SaveCVARs();
        }
    }
    else
    {
        char    *temp1 = C_LookupAliasFromValue(vid_vsync, VSYNCVALUEALIAS);

        C_ShowDescription(C_GetIndex(cmd));

        if (vid_vsync == vid_vsync_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(vid_vsync_default, VSYNCVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
    }
}

//
// vid_widescreen CVAR
//
static void vid_widescreen_cvar_func2(char *cmd, char *parms)
{
    const dboolean  vid_widescreen_old = vid_widescreen;

    bool_cvars_func2(cmd, parms);

    if (vid_widescreen != vid_widescreen_old)
    {
        if (vid_widescreen)
        {
            if (gamestate == GS_LEVEL)
            {
                I_ToggleWidescreen(true);

                if (vid_widescreen && !togglingvanilla)
                    S_StartSound(NULL, sfx_stnmov);
            }
            else
            {
                r_hud = true;
                M_SaveCVARs();
                returntowidescreen = true;
            }
        }
        else
        {
            I_ToggleWidescreen(false);

            if (!vid_widescreen && !togglingvanilla)
                S_StartSound(NULL, sfx_stnmov);
        }
    }

    if (!vid_widescreen)
        returntowidescreen = false;
}

//
// vid_windowpos CVAR
//
static void vid_windowpos_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        char    *parm = removespaces(parms);

        if (!M_StringCompare(vid_windowpos, parm))
        {
            vid_windowpos = M_StringDuplicate(parm);
            GetWindowPosition();
            M_SaveCVARs();

            if (!vid_fullscreen)
                SDL_SetWindowPosition(window, windowx, windowy);
        }

        free(parm);
    }
    else
    {
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(vid_windowpos, vid_windowpos_default))
            C_Output(INTEGERCVARISDEFAULT, vid_windowpos);
        else
            C_Output(INTEGERCVARWITHDEFAULT, vid_windowpos, vid_windowpos_default);
    }
}

//
// vid_windowsize CVAR
//
static void vid_windowsize_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        if (!M_StringCompare(vid_windowsize, parms))
        {
            vid_windowsize = M_StringDuplicate(parms);
            GetWindowSize();
            M_SaveCVARs();

            if (!vid_fullscreen)
                SDL_SetWindowSize(window, windowwidth, windowheight);
        }
    }
    else
    {
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(vid_windowsize, vid_windowsize_default))
            C_Output(INTEGERCVARISDEFAULT, vid_windowsize);
        else
            C_Output(INTEGERCVARWITHDEFAULT, vid_windowsize, vid_windowsize_default);
    }
}
