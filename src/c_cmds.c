/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

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
#define MAPCMDFORMAT                "<b>E</b><i>x</i><b>M</b><i>y</i>|<b>MAP</b><i>xy</i>|<b>first</b>|<b>previous</b>|<b>next</b>|<b>last</b>|<b>random</b>"
#define PLAYCMDFORMAT               "<i>soundeffect</i>|<i>music</i>"
#define NAMECMDFORMAT               "[<b>friendly</b> ]<i>monster</i> <i>name</i>"
#define PRINTCMDFORMAT              "<b>\"</b><i>message</i><b>\"</b>"
#define RESETCMDFORMAT              "<i>CVAR</i>"
#define SAVECMDFORMAT               "<i>filename</i><b>.save</b>"
#define SPAWNCMDFORMAT              "<i>item</i>|[<b>friendly</b> ]<i>monster</i>"
#define TAKECMDFORMAT               GIVECMDFORMAT
#define TELEPORTCMDFORMAT           "<i>x</i> <i>y</i>"
#define TIMERCMDFORMAT              "<i>minutes</i>"
#define UNBINDCMDFORMAT             "<i>control</i>|<b>+</b><i>action</i>"

#define PENDINGCHANGE               "This change won't be effective until the player warps to the next map."

#define INTEGERCVARWITHDEFAULT      "It is currently <b>%s</b> and its default is <b>%s</b>."
#define INTEGERCVARWITHNODEFAULT    "It is currently <b>%s</b>."
#define INTEGERCVARISDEFAULT        "It is currently its default of <b>%s</b>."
#define INTEGERCVARISREADONLY       "It is currently <b>%s</b> and is read-only."
#define PERCENTCVARWITHDEFAULT      "It is currently <b>%s%%</b> and its default is <b>%s%%</b>."
#define PERCENTCVARWITHNODEFAULT    "It is currently <b>%s%%</b>."
#define PERCENTCVARISDEFAULT        "It is currently its default of <b>%s%%</b>."
#define PERCENTCVARISREADONLY       "It is currently <b>%s%%</b> and is read-only."
#define STRINGCVARWITHDEFAULT       "It is currently <b>\"%s\"</b> and its default is <b>\"%s\"</b>."
#define STRINGCVARISDEFAULT         "It is currently its default of <b>\"%s\"</b>."
#define STRINGCVARISREADONLY        "It is currently <b>%s%s%s</b> and is read-only."
#define TIMECVARISREADONLY          "It is currently <b>%02i:%02i:%02i</b> and is read-only."

#define UNITSPERFOOT                16
#define FEETPERMETER                3.28084f
#define METERSPERKILOMETER          1000
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

extern dboolean     setsizeneeded;
extern char         *packageconfig;
extern int          st_palette;

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
    { "+alwaysrun",   alwaysrun_action_func,   &keyboardalwaysrun,         NULL,                  NULL,             &gamepadalwaysrun,         NULL         },
    { "+automap",     automap_action_func,     &keyboardautomap,           NULL,                  NULL,             &gamepadautomap,           NULL         },
    { "+back",        back_action_func,        &keyboardback,              &keyboardback2,        NULL,             &gamepadback,              NULL         },
    { "+clearmark",   clearmark_action_func,   &keyboardautomapclearmark,  NULL,                  NULL,             &gamepadautomapclearmark,  NULL         },
    { "+console",     console_action_func,     &keyboardconsole,           NULL,                  NULL,             NULL,                      NULL         },
    { "+fire",        fire_action_func,        &keyboardfire,              NULL,                  &mousefire,       &gamepadfire,              NULL         },
    { "+followmode",  followmode_action_func,  &keyboardautomapfollowmode, NULL,                  NULL,             &gamepadautomapfollowmode, NULL         },
    { "+forward",     forward_action_func,     &keyboardforward,           &keyboardforward2,     &mouseforward,    &gamepadforward,           NULL         },
    { "+grid",        grid_action_func,        &keyboardautomapgrid,       NULL,                  NULL,             &gamepadautomapgrid,       NULL         },
    { "+jump",        jump_action_func,        &keyboardjump,              NULL,                  &mousejump,       &gamepadjump,              NULL         },
    { "+left",        left_action_func,        &keyboardleft,              NULL,                  NULL,             &gamepadleft,              NULL         },
    { "+mark",        mark_action_func,        &keyboardautomapmark,       NULL,                  NULL,             &gamepadautomapmark,       NULL         },
    { "+maxzoom",     maxzoom_action_func,     &keyboardautomapmaxzoom,    NULL,                  NULL,             &gamepadautomapmaxzoom,    NULL         },
    { "+menu",        menu_action_func,        &keyboardmenu,              NULL,                  NULL,             &gamepadmenu,              NULL         },
    { "+mouselook",   NULL,                    &keyboardmouselook,         NULL,                  &mousemouselook,  &gamepadmouselook,         NULL         },
    { "+nextweapon",  nextweapon_action_func,  &keyboardnextweapon,        NULL,                  &mousenextweapon, &gamepadnextweapon,        NULL         },
    { "+prevweapon",  prevweapon_action_func,  &keyboardprevweapon,        NULL,                  &mouseprevweapon, &gamepadprevweapon,        NULL         },
    { "+right",       right_action_func,       &keyboardright,             NULL,                  NULL,             &gamepadright,             NULL         },
    { "+rotatemode",  rotatemode_action_func,  &keyboardautomaprotatemode, NULL,                  NULL,             &gamepadautomaprotatemode, NULL         },
    { "+run",         NULL,                    &keyboardrun,               NULL,                  &mouserun,        &gamepadrun,               NULL         },
    { "+screenshot",  screenshot_action_func,  &keyboardscreenshot,        NULL,                  &mousescreenshot, NULL,                      NULL         },
    { "+strafe",      NULL,                    &keyboardstrafe,            NULL,                  &mousestrafe,     &gamepadstrafe,            NULL         },
    { "+strafeleft",  strafeleft_action_func,  &keyboardstrafeleft,        &keyboardstrafeleft2,  NULL,             &gamepadstrafeleft,        NULL         },
    { "+straferight", straferight_action_func, &keyboardstraferight,       &keyboardstraferight2, NULL,             &gamepadstraferight,       NULL         },
    { "+use",         use_action_func,         &keyboarduse,               &keyboarduse2,         &mouseuse,        &gamepaduse,               &gamepaduse2 },
    { "+weapon1",     weapon1_action_func,     &keyboardweapon1,           NULL,                  NULL,             &gamepadweapon1,           NULL         },
    { "+weapon2",     weapon2_action_func,     &keyboardweapon2,           NULL,                  NULL,             &gamepadweapon2,           NULL         },
    { "+weapon3",     weapon3_action_func,     &keyboardweapon3,           NULL,                  NULL,             &gamepadweapon3,           NULL         },
    { "+weapon4",     weapon4_action_func,     &keyboardweapon4,           NULL,                  NULL,             &gamepadweapon4,           NULL         },
    { "+weapon5",     weapon5_action_func,     &keyboardweapon5,           NULL,                  NULL,             &gamepadweapon5,           NULL         },
    { "+weapon6",     weapon6_action_func,     &keyboardweapon6,           NULL,                  NULL,             &gamepadweapon6,           NULL         },
    { "+weapon7",     weapon7_action_func,     &keyboardweapon7,           NULL,                  NULL,             &gamepadweapon7,           NULL         },
    { "+zoomin",      NULL,                    &keyboardautomapzoomin,     NULL,                  NULL,             &gamepadautomapzoomin,     NULL         },
    { "+zoomout",     NULL,                    &keyboardautomapzoomout,    NULL,                  NULL,             &gamepadautomapzoomout,    NULL         },
    { "",             NULL,                    NULL,                       NULL,                  NULL,             NULL,                      NULL         }
};

static dboolean alive_func1(char *cmd, char *parms);
static dboolean cheat_func1(char *cmd, char *parms);
static dboolean game_func1(char *cmd, char *parms);
static dboolean null_func1(char *cmd, char *parms);

void alias_cmd_func2(char *cmd, char *parms);
void bind_cmd_func2(char *cmd, char *parms);
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
static void am_path_cvar_func2(char *cmd, char *parms);
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
            return valuealiases[i].text;

    return commify(value);
}

#define CMD(name, alt, cond, func, parms, form, desc) \
    { #name, #alt, cond, func, parms, CT_CMD, CF_NONE, NULL, 0, 0, 0, form, desc, 0, 0 }
#define CMD_CHEAT(name, parms) \
    { #name, "", cheat_func1, NULL, parms, CT_CHEAT, CF_NONE, NULL, 0, 0, 0, "", "", 0, 0 }
#define CVAR_BOOL(name, alt, cond, func, valuealiases, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, CF_BOOLEAN, &name, valuealiases, false, true, "", desc, name##_default, 0 }
#define CVAR_INT(name, alt, cond, func, flags, valuealiases, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, (CF_INTEGER | flags), &name, valuealiases, name##_min, name##_max, "", desc, name##_default, 0 }
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
    CMD(alias, "", null_func1, alias_cmd_func2, true, ALIASCMDFORMAT,
        "Creates an <i>alias</i> that executes a string of\n<i>commands</i>."),
    CVAR_BOOL(alwaysrun, "", bool_cvars_func1, alwaysrun_cvar_func2, BOOLVALUEALIAS,
        "Toggles the player to always run when they move."),
    CVAR_INT(am_allmapcdwallcolor, am_allmapcdwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating a\nchange in ceiling height and when the player has a\ncomputer area map power-up "
        "(<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_allmapfdwallcolor, am_allmapfdwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating a\nchange in floor height and when the player has a\ncomputer area map power-up "
        "(<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_allmapwallcolor, am_allmapwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of solid walls in the automap when the\nplayer has a computer area map power-up (<b>0</b> to\n<b>255</b>)."),
    CVAR_INT(am_backcolor, am_backcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the automap's background (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_cdwallcolor, am_cdwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating a\nchange in ceiling height (<b>0</b> to <b>255</b>)."),
    CVAR_INT(am_crosshaircolor, am_crosshaircolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the crosshair in the automap (<b>0</b> to\n<b>255</b>)."),
    CVAR_BOOL(am_external, "", bool_cvars_func1, am_external_cvar_func2, BOOLVALUEALIAS,
        "Toggles showing the automap on an external\ndisplay."),
    CVAR_INT(am_fdwallcolor, am_fdwallcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating a\nchange in floor height (<b>0</b> to <b>255</b>)."),
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
    CVAR_BOOL(am_path, "", bool_cvars_func1, am_path_cvar_func2, BOOLVALUEALIAS,
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
        "Toggles automatically using a door or switch when\nthe player is nearby."),
    CMD(bind, "", null_func1, bind_cmd_func2, true, BINDCMDFORMAT,
        "Binds an <i>action</i> or string of <i>commands</i> to a\n<i>control</i>."),
    CMD(bindlist, "", null_func1, bindlist_cmd_func2, false, "",
        "Lists all bound controls."),
    CVAR_BOOL(centerweapon, centreweapon, bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles centering the player's weapon when firing."),
    CMD(clear, "", null_func1, clear_cmd_func2, false, "",
        "Clears the console."),
    CMD(cmdlist, ccmdlist, null_func1, cmdlist_cmd_func2, true, "[<i>searchstring</i>]",
        "Lists all console commands."),
    CVAR_INT(con_backcolor, con_backcolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the console's background (<b>0</b> to <b>255</b>)."),
    CVAR_BOOL(con_obituaries, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles obituaries in the console when monsters\nare killed or resurrected."),
    CVAR_BOOL(con_timestamps, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles timestamps next to player messages and\nobituaries in the console."),
    CMD(condump, "", condump_cmd_func1, condump_cmd_func2, true, "[<i>filename</i><b>.txt</b>]",
        "Dumps the console to a file."),
    CVAR_INT(crosshair, "", crosshair_cvar_func1, crosshair_cvar_func2, CF_NONE, CROSSHAIRVALUEALIAS,
        "Toggles a crosshair (<b>none</b>, <b>cross</b> or <b>dot</b>)."),
    CVAR_INT(crosshaircolor, crosshaircolour, color_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the crosshair (<b>0</b> to <b>255</b>)."),
    CMD(cvarlist, "", null_func1, cvarlist_cmd_func2, true, "[<i>searchstring</i>]",
        "Lists all console variables."),
    CMD(endgame, "", game_func1, endgame_cmd_func2, false, "",
        "Ends a game."),
    CVAR_INT(episode, "", int_cvars_func1, episode_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected <i><b>DOOM</b></i> episode in the menu\n(<b>1</b> to <b>5</b>)."),
    CMD(exec, "", null_func1, exec_cmd_func2, true, EXECCMDFORMAT,
        "Executes a series of commands stored in a file."),
    CMD(exitmap, "", game_func1, exitmap_cmd_func2, false, "",
        "Exits the current map."),
    CVAR_INT(expansion, "", int_cvars_func1, expansion_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected <i><b>DOOM II</b></i> expansion in the\nmenu (<b>1</b> or <b>2</b>)."),
    CVAR_INT(facebackcolor, facebackcolour, color_cvars_func1, color_cvars_func2, CF_NONE, FACEBACKVALUEALIAS,
        "The color behind the player's face in the status bar\n(<b>none</b>, <b>0</b> to <b>255</b>)."),
    CMD(fastmonsters, "", fastmonsters_cmd_func1, fastmonsters_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles fast monsters."),
    CMD(freeze, "", alive_func1, freeze_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles freeze mode."),
    CVAR_TIME(gametime, "", null_func1, time_cvars_func2,
        "The amount of time <i><b>" PACKAGE_NAME "</b></i> has been running."),
    CMD(give, "", give_cmd_func1, give_cmd_func2, true, GIVECMDFORMAT,
        "Gives <b>ammo</b>, <b>armor</b>, <b>health</b>, <b>keys</b>, <b>weapons</b>, or <b>all</b>\nor certain <i>items</i> to the "
        "player."),
    CMD(god, "", alive_func1, god_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles god mode."),
    CVAR_BOOL(gp_analog, gp_analogue, bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles whether movement using the gamepad's\nthumbsticks is analog or digital."),
    CVAR_FLOAT(gp_deadzone_left, "", gp_deadzone_cvars_func1, gp_deadzone_cvars_func2, CF_PERCENT,
        "The dead zone of the gamepad's left thumbstick\n(<b>0%</b> to <b>100%</b>)."),
    CVAR_FLOAT(gp_deadzone_right, "", gp_deadzone_cvars_func1, gp_deadzone_cvars_func2, CF_PERCENT,
        "The dead zone of the gamepad's right thumbstick\n(<b>0%</b> to <b>100%</b>)."),
    CVAR_BOOL(gp_invertyaxis, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles inverting the vertical axis of the\ngamepad's right thumbstick when looking up and\ndown."),
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
    CMD(help, "", null_func1, help_cmd_func2, false, "",
        "Opens the <i><b>" PACKAGE_NAME " Wiki</b></i>."),
    CMD_CHEAT(idbeholda, false),
    CMD_CHEAT(idbeholdl, false),
    CMD_CHEAT(idbeholdi, false),
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
    CMD(if, "", null_func1, if_cmd_func2, true, IFCMDFORMAT,
        "If a <i>CVAR</i> equals a <i>value</i> then execute a string\nof <i>commands</i>."),
    CVAR_BOOL(infighting, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles infighting among monsters once the player\ndies."),
    CVAR_BOOL(infiniteheight, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles giving the player and monsters infinite\nheight."),
    CVAR_STR(iwadfolder, "", null_func1, str_cvars_func2, CF_NONE,
        "The folder where an IWAD was last opened."),
    CMD(kill, explode, kill_cmd_func1, kill_cmd_func2, true, KILLCMDFORMAT,
        "Kills the <b>player</b>, <b>all</b> monsters, a type of <i>monster</i>,\nor explodes all <b>barrels</b> or <b>missiles</b>."),
    CMD(load, "", null_func1, load_cmd_func2, true, LOADCMDFORMAT,
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
    CMD(map, warp, map_cmd_func1, map_cmd_func2, true, MAPCMDFORMAT,
        "Warps the player to another map."),
    CMD(maplist, "", null_func1, maplist_cmd_func2, false, "",
        "Lists all maps in the currently loaded WADs."),
    CMD(mapstats, "", game_func1, mapstats_cmd_func2, false, "",
        "Shows statistics about the current map."),
    CVAR_BOOL(messages, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles player messages."),
    CVAR_BOOL(mouselook, "", bool_cvars_func1, mouselook_cvar_func2, BOOLVALUEALIAS,
        "Toggles mouselook."),
    CVAR_INT(movebob, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's view bobs up and down\nwhen they move (<b>0%</b> to <b>100%</b>)."),
    CMD_CHEAT(mumu, false),
    CMD(name, "", name_cmd_func1, name_cmd_func2, true, NAMECMDFORMAT,
        "Gives a <i>name</i> to the <i>monster</i> nearest to the\nplayer."),
    CMD(newgame, "", null_func1, newgame_cmd_func2, true, "",
        "Starts a new game."),
    CMD(noclip, "", game_func1, noclip_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles no clipping mode."),
    CMD(nomonsters, "", null_func1, nomonsters_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles the presence of monsters in maps."),
    CMD(notarget, "", game_func1, notarget_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles monsters not seeing the player as a\ntarget."),
    CMD(pistolstart, "", null_func1, pistolstart_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles the player starting each map with only\na pistol."),
    CMD(play, "", play_cmd_func1, play_cmd_func2, true, PLAYCMDFORMAT,
        "Plays a <i>sound effect</i> or <i>music</i> lump."),
    CVAR_STR(playername, "", null_func1, playername_cvar_func2, CF_NONE,
        "The name of the player used in player messages."),
    CMD(playerstats, "", null_func1, playerstats_cmd_func2, false, "",
        "Shows statistics about the player."),
    CMD(print, "", null_func1, print_cmd_func2, true, PRINTCMDFORMAT,
        "Prints a player <i>message</i>."),
    CMD(quit, exit, null_func1, quit_cmd_func2, false, "",
        "Quits <i><b>" PACKAGE_NAME "</b></i>."),
    CVAR_BOOL(r_althud, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles an alternate heads-up display when in\nwidescreen mode."),
    CVAR_INT(r_berserkintensity, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The intensity of the effect when the player has a\nberserk power-up and their fists equipped (<b>0</b> to <b>8</b>)."),
    CVAR_INT(r_blood, "", r_blood_cvar_func1, r_blood_cvar_func2, CF_NONE, BLOODVALUEALIAS,
        "The colors of the blood of the player and monsters\n(<b>all</b>, <b>none</b> or <b>red</b>)."),
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
    CVAR_BOOL(r_corpses_mirrored, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles randomly mirrored corpses."),
    CVAR_BOOL(r_corpses_moreblood, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles blood splats around corpses that are\nspawned when a map is loaded."),
    CVAR_BOOL(r_corpses_nudge, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles corpses being nudged when the player and\nmonsters walk over them."),
    CVAR_BOOL(r_corpses_slide, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles corpses sliding in reaction to barrel and\nrocket explosions."),
    CVAR_BOOL(r_corpses_smearblood, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles corpses leaving blood splats as they slide."),
    CVAR_BOOL(r_detail, "", r_detail_cvar_func1, r_detail_cvar_func2, DETAILVALUEALIAS,
        "Toggles the graphic detail (<b>low</b> or <b>high</b>)."),
    CVAR_BOOL(r_diskicon, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles showing a disk icon when loading and\nsaving."),
    CVAR_BOOL(r_dither, "", bool_cvars_func1, r_dither_cvar_func2, BOOLVALUEALIAS,
        "Toggles dithering of <i><b>BOOM</b></i>-compatible translucent\nwall textures."),
    CVAR_BOOL(r_fixmaperrors, "", bool_cvars_func1, r_fixmaperrors_cvar_func2, BOOLVALUEALIAS,
        "Toggles the fixing of mapping errors in the <i><b>DOOM</b></i>\nand <i><b>DOOM II</b></i> IWADs."),
    CVAR_BOOL(r_fixspriteoffsets, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the fixing of sprite offsets."),
    CVAR_BOOL(r_floatbob, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles some power-ups bobbing up and down."),
    CVAR_INT(r_fov, "", int_cvars_func1, r_fov_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The player's horizontal field of view (<b>45</b>\xB0 to <b>135</b>\xB0)."),
    CVAR_FLOAT(r_gamma, "", r_gamma_cvar_func1, r_gamma_cvar_func2, CF_NONE,
        "The screen's gamma correction level (<b>off</b>, or <b>0.50</b>\nto <b>2.0</b>)."),
    CVAR_BOOL(r_homindicator, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the flashing \"Hall Of Mirrors\" indicator."),
    CVAR_BOOL(r_hud, "", bool_cvars_func1, r_hud_cvar_func2, BOOLVALUEALIAS,
        "Toggles the heads-up display when in widescreen\nmode."),
    CVAR_BOOL(r_hud_translucency, "", bool_cvars_func1, r_hud_translucency_cvar_func2, BOOLVALUEALIAS,
        "Toggles the translucency of the heads-up display\nwhen in widescreen mode."),
    CVAR_BOOL(r_liquid_bob, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the bobbing effect of liquid sectors and\nthe sprites in them."),
    CVAR_BOOL(r_liquid_clipsprites, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles clipping the bottom of sprites in liquid\nsectors."),
    CVAR_BOOL(r_liquid_current, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles a slight current being applied to liquid\nsectors."),
    CVAR_BOOL(r_liquid_lowerview, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles lowering the player's view when in a liquid\nsector."),
    CVAR_BOOL(r_liquid_swirl, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the swirl effect of liquid sectors."),
    CVAR_OTHER(r_lowpixelsize, "", null_func1, r_lowpixelsize_cvar_func2,
        "The size of pixels when the graphic detail is low\n(<i>width</i><b>\xD7</b><i>height</i>)."),
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
    CVAR_BOOL(r_textures, "", bool_cvars_func1, r_textures_cvar_func2, BOOLVALUEALIAS,
        "Toggles displaying all textures."),
    CVAR_BOOL(r_translucency, "", bool_cvars_func1, r_translucency_cvar_func2, BOOLVALUEALIAS,
        "Toggles the translucency of sprites and <i><b>BOOM</b></i>-\ncompatible wall textures."),
    CMD(regenhealth, "", null_func1, regenhealth_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles regenerating the player's health when\nbelow 100%."),
    CMD(reset, "", null_func1, reset_cmd_func2, true, RESETCMDFORMAT,
        "Resets a <i>CVAR</i> to its default."),
    CMD(resetall, "", null_func1, resetall_cmd_func2, false, "",
        "Resets all CVARs to their defaults."),
    CMD(respawnitems, "", null_func1, respawnitems_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles respawning items."),
    CMD(respawnmonsters, "", respawnmonsters_cmd_func1, respawnmonsters_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles respawning monsters."),
    CMD(restartmap, "", game_func1, restartmap_cmd_func2, false, "",
        "Restarts the current map."),
    CMD(resurrect, "", resurrect_cmd_func1, resurrect_cmd_func2, false, "",
        "Resurrects the player."),
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
    CMD(save, "", alive_func1, save_cmd_func2, true, SAVECMDFORMAT,
        "Saves the game to a file."),
    CVAR_INT(savegame, "", int_cvars_func1, savegame_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected savegame in the menu\n(<b>1</b> to <b>6</b>)."),
    CVAR_INT(skilllevel, "", int_cvars_func1, skilllevel_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected skill level in the menu\n(<b>1</b> to <b>5</b>)."),
    CMD(spawn, summon, spawn_cmd_func1, spawn_cmd_func2, true, SPAWNCMDFORMAT,
        "Spawns an <i>item</i> or <i>monster</i> in front of the\nplayer."),
    CVAR_INT(stillbob, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's view and weapon bob up\nand down when they stand still (<b>0%</b> to <b>100%</b>)."),
    CMD(take, "", take_cmd_func1, take_cmd_func2, true, TAKECMDFORMAT,
        "Takes <b>ammo</b>, <b>armor</b>, <b>health</b>, <b>keys</b>, <b>weapons</b>, or <b>all</b>\nor certain <i>items</i> from the "
        "player."),
    CMD(teleport, "", game_func1, teleport_cmd_func2, true, TELEPORTCMDFORMAT,
        "Teleports the player to (<i>x</i>,<i>y</i>) in the current\nmap."),
    CMD(thinglist, "", game_func1, thinglist_cmd_func2, false, "",
        "Lists all things in the current map."),
    CMD(timer, "", null_func1, timer_cmd_func2, true, TIMERCMDFORMAT,
        "Sets a timer on each map."),
    CVAR_BOOL(tossdrop, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles tossing items dropped by monsters when\nthey die."),
    CVAR_INT(turbo, "", turbo_cvar_func1, turbo_cvar_func2, CF_PERCENT, NOVALUEALIAS,
        "The speed of the player (<b>10%</b> to <b>400%</b>)."),
    CMD(unbind, "", null_func1, unbind_cmd_func2, true, UNBINDCMDFORMAT,
        "Unbinds the +<i>action</i> from a <i>control</i>."),
    CVAR_BOOL(units, "", units_cvar_func1, units_cvar_func2, UNITSVALUEALIAS,
        "The units used in the <b>mapstats</b> and <b>playerstats</b>\nCCMDs (<b>imperial</b> or <b>metric</b>)."),
    CMD(vanilla, "", null_func1, vanilla_cmd_func2, true, "[<b>on</b>|<b>off</b>]",
        "Toggles vanilla mode."),
    CVAR_STR(version, "", null_func1, str_cvars_func2, CF_READONLY,
        "<i><b>" PACKAGE_NAME "'s</b></i> version."),
    CVAR_INT(vid_capfps, "", vid_capfps_cvar_func1, vid_capfps_cvar_func2, CF_NONE, CAPVALUEALIAS,
        "The number of frames per second at which to cap\nthe framerate (<b>off</b>, or <b>1</b> to <b>1,000</b>). Interpolation is\n"
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
        "Toggles using the black pillarboxes either side of\nthe screen for palette effects."),
#if defined(_WIN32)
    CVAR_STR(vid_scaleapi, "", vid_scaleapi_cvar_func1, vid_scaleapi_cvar_func2, CF_NONE,
        "The API used to scale the display (<b>\"direct3d\"</b>,\n<b>\"opengl\"</b> or <b>\"software\"</b>)."),
#elif defined(__APPLE__)
    CVAR_STR(vid_scaleapi, "", vid_scaleapi_cvar_func1, vid_scaleapi_cvar_func2, CF_NONE,
        "The API used to scale the display (<b>\"metal\"</b>,\n<b>\"opengl\"</b>, <b>\"opengles\"</b>, <b>\"opengles2\"</b> or "
        "<b>\"software\"</b>)."),
#else
    CVAR_STR(vid_scaleapi, "", vid_scaleapi_cvar_func1, vid_scaleapi_cvar_func2, CF_NONE,
        "The API used to scale the display (<b>\"opengl\"</b>,\n<b>\"opengles\"</b>, <b>\"opengles2\"</b> or <b>\"software\"</b>)."),
#endif
    CVAR_STR(vid_scalefilter, "", vid_scalefilter_cvar_func1, vid_scalefilter_cvar_func2, CF_NONE,
        "The filter used to scale the display (<b>\"nearest\"</b>,\n<b>\"linear\"</b> or <b>\"nearest_linear\"</b>)."),
    CVAR_OTHER(vid_screenresolution, "", null_func1, vid_screenresolution_cvar_func2,
        "The screen's resolution when fullscreen (<b>desktop</b>\nor <i>width</i><b>\xD7</b><i>height</i>)."),
    CVAR_BOOL(vid_showfps, "", bool_cvars_func1, vid_showfps_cvar_func2, BOOLVALUEALIAS,
        "Toggles showing the number of frames per second."),
    CVAR_BOOL(vid_vsync, "", bool_cvars_func1, vid_vsync_cvar_func2, BOOLVALUEALIAS,
        "Toggles vertical sync with the display's refresh\nrate."),
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
    CVAR_INT(weaponbob, "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's weapon bobs when they\nmove (<b>0%</b> to <b>100%</b>)."),
    CVAR_BOOL(weaponbounce, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the bouncing of the player's weapon when\ndropping from a greater height."),
    CVAR_BOOL(weaponrecoil, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the recoiling of the player's weapon when\nfired."),
    CVAR_BOOL(wipe, "", bool_cvars_func1, bool_cvars_func2, BOOLVALUEALIAS,
        "Toggles the wipe effect when transitioning between\nscreens."),

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
    if (!automapactive)
    {
        if (!mapwindow)
            AM_Start(true);
    }
    else
        AM_Stop();
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
    if (!consoleactive)
        C_ShowConsole();
}

static void fire_action_func(void)
{
    P_FireWeapon();
}

static void followmode_action_func(void)
{
    if (automapactive || mapwindow)
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
    G_DoScreenShot();
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
        if (M_StringCompare(cmd, consolecmds[i].name))
            break;

        i++;
    }

    return i;
}

static void C_ShowDescription(int index)
{
    char    description1[255];
    char    *p;

    M_StringCopy(description1, consolecmds[index].description, sizeof(description1));

    if ((p = strchr(description1, '\n')))
    {
        char    description2[255] = "";

        *p++ = '\0';
        M_StringCopy(description2, p, sizeof(description2));

        if ((p = strchr(description2, '\n')))
        {
            char    description3[255] = "";

            *p++ = '\0';
            M_StringCopy(description3, p, sizeof(description3));

            if (C_TextWidth(consolecmds[index].description, true, true) > CONSOLETEXTPIXELWIDTH)
            {
                C_Output("%s %s", description1, description2);
                C_Output(description3);
            }
            else
                C_Output("%s %s %s", description1, description2, description3);
        }
        else
            C_Output("%s %s", description1, description2);
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
                C_Input(alias);
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
        C_ShowDescription(C_GetIndex("alias"));
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
        C_ShowDescription(C_GetIndex("bind"));
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
                        C_Warning("The <b>%s</b> action can't be bound to '<b>%s</b>'.", parm2, controls[i].control);
                    else
                        C_Warning("The <b>%s</b> action can't be bound to <b>%s</b>.", parm2, controls[i].control);
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
        C_Warning("<b>%s</b> isn't a valid control.", parm1);

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
    const int   tabs[8] = { 40, 130, 0, 0, 0, 0, 0, 0 };

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
    const int   tabs[8] = { 40, 131, 0, 0, 0, 0, 0, 0 };
    int         count = 1;

    C_Header(bindlistheader);

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
    const int tabs[8] = { 40, 336, 0, 0, 0, 0, 0, 0 };
    int       count = 0;

    C_Header(cmdlistheader);

    for (int i = 0; *consolecmds[i].name; i++)
        if (consolecmds[i].type == CT_CMD)
        {
            char    description1[255];
            char    *p;

            count++;

            if (*parms && !wildcard(consolecmds[i].name, parms))
                continue;

            M_StringCopy(description1, consolecmds[i].description, sizeof(description1));

            if ((p = strchr(description1, '\n')))
            {
                char    description2[255] = "";

                *p++ = '\0';
                M_StringCopy(description2, p, sizeof(description2));

                C_TabbedOutput(tabs, "%i.\t<b>%s</b> %s\t%s", count, consolecmds[i].name, consolecmds[i].format, description1);
                C_TabbedOutput(tabs, "\t\t%s", description2);
            }
            else
                C_TabbedOutput(tabs, "%i.\t<b>%s</b> %s\t%s", count, consolecmds[i].name, consolecmds[i].format, description1);
        }
}

//
// condump CCMD
//
static dboolean condump_cmd_func1(char *cmd, char *parms)
{
    return (consolestrings > 1);
}

static void condump_cmd_func2(char *cmd, char *parms)
{
    char        filename[MAX_PATH];
    FILE        *file;
    const char  *appdatafolder = M_GetAppDataFolder();

    M_MakeDirectory(appdatafolder);

    if (!*parms)
    {
        int count = 0;

        M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "condump.txt", appdatafolder);

        while (M_FileExists(filename))
            M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "condump (%i).txt", appdatafolder, ++count);
    }
    else
        M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "%s", appdatafolder, parms);

    if ((file = fopen(filename, "wt")))
    {
        for (int i = 1; i < consolestrings - 1; i++)
        {
            if (console[i].stringtype == dividerstring)
                fprintf(file, "%s\n", DIVIDERSTRING);
            else
            {
                char            *string = M_StringDuplicate(console[i].string);
                int             len;
                unsigned int    outpos = 0;
                int             tabcount = 0;

                strreplace(string, "<b>", "");
                strreplace(string, "</b>", "");
                strreplace(string, "<i>", "");
                strreplace(string, "</i>", "");
                len = (int)strlen(string);

                if (console[i].stringtype == warningstring)
                    fputs("! ", file);

                for (int inpos = 0; inpos < len; inpos++)
                {
                    const unsigned char letter = string[inpos];

                    if (letter != '\n')
                    {
                        if (letter == '\t')
                        {
                            const unsigned int  tabstop = console[i].tabs[tabcount] / 5;

                            if (outpos < tabstop)
                            {
                                for (unsigned int spaces = 0; spaces < tabstop - outpos; spaces++)
                                    fputc(' ', file);

                                outpos = tabstop;
                                tabcount++;
                            }
                            else
                            {
                                fputc(' ', file);
                                outpos++;
                            }
                        }
                        else
                        {
                            const int   c = letter - CONSOLEFONTSTART;

                            if (((c >= 0 && c < CONSOLEFONTSIZE && letter != '~')
                                || letter == 153 || letter == 169 || letter == 174 || letter == 215))
                            {
                                fputc(letter, file);
                                outpos++;
                            }
                        }
                    }
                }

                if ((console[i].stringtype == playermessagestring || console[i].stringtype == obituarystring) && con_timestamps)
                {
                    for (unsigned int spaces = 0; spaces < 91 - outpos; spaces++)
                        fputc(' ', file);

                    fputs(C_GetTimeStamp(console[i].tics), file);
                }

                fputc('\n', file);
                free(string);
            }
        }

        fclose(file);
        C_Output("Dumped the console to <b>%s</b>.", filename);
    }
}

//
// cvarlist CCMD
//
static void cvarlist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[8] = { 40, 209, 318, 0, 0, 0, 0, 0 };
    int         count = 0;

    C_Header(cvarlistheader);

    for (int i = 0; *consolecmds[i].name; i++)
        if (consolecmds[i].type == CT_CVAR)
        {
            char    description1[255];
            char    description2[255] = "";
            char    description3[255] = "";
            char    *p;

            count++;

            if (*parms && !wildcard(consolecmds[i].name, parms))
                continue;

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
                    C_TabbedOutput(tabs, "%i.\t<b>%s\t%s</b>\t%s", count, consolecmds[i].name,
                        C_LookupAliasFromValue(viewplayer->armortype, ARMORTYPEVALUEALIAS), description1);
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
                C_TabbedOutput(tabs, "%i.\t<b>%s\t%s</b>\t%s", count, consolecmds[i].name,
                    C_LookupAliasFromValue(*(dboolean *)consolecmds[i].variable, consolecmds[i].aliases), description1);
            else if ((consolecmds[i].flags & CF_INTEGER) && (consolecmds[i].flags & CF_PERCENT))
                C_TabbedOutput(tabs, "%i.\t<b>%s\t%i%%</b>\t%s", count, consolecmds[i].name,
                    *(int *)consolecmds[i].variable, description1);
            else if (consolecmds[i].flags & CF_INTEGER)
                C_TabbedOutput(tabs, "%i.\t<b>%s\t%s</b>\t%s", count, consolecmds[i].name,
                    C_LookupAliasFromValue(*(int *)consolecmds[i].variable, consolecmds[i].aliases), description1);
            else if (consolecmds[i].flags & CF_FLOAT)
            {
                if (consolecmds[i].flags & CF_PERCENT)
                    C_TabbedOutput(tabs, "%i.\t<b>%s\t%s%%</b>\t%s", count, consolecmds[i].name,
                        striptrailingzero(*(float *)consolecmds[i].variable, 1), description1);
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

            if (*description2)
            {
                C_TabbedOutput(tabs, "\t\t\t%s", description2);

                if (*description3)
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
        C_ShowDescription(C_GetIndex("exec"));
        C_Output("<b>%s</b> %s", cmd, EXECCMDFORMAT);
    }
    else
    {
        FILE    *file = fopen(parms, "r");

        if (!file)
            return;

        while (!feof(file))
        {
            char    strparm[256] = "";

            if (fscanf(file, "%255[^\n]\n", strparm) != 1)
                continue;

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
    char    *parm = removenonalpha(parms);
    int     num = -1;

    if (gamestate != GS_LEVEL)
        return false;

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
        return true;

    sscanf(parm, "%10d", &num);

    for (int i = 0; i < NUMMOBJTYPES; i++)
        if ((mobjinfo[i].flags & MF_SPECIAL) && (M_StringCompare(parm, removenonalpha(mobjinfo[i].name1))
            || (*mobjinfo[i].name2 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name2)))
            || (*mobjinfo[i].name3 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name3)))
            || (num == mobjinfo[i].doomednum && num != -1)))
            return true;

    return false;
}

static void give_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex("give"));
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
                C_Warning("%s already has everything.", titlecase(playername));
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
                C_Warning("%s already %s full health.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "have" : "has"));
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
                C_Warning("%s already %s all the weapons.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "have" : "has"));
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
                C_Warning("%s already %s full ammo.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "have" : "has"));
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
                C_Warning("%s already %s full armor.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "have" : "has"));
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
                C_Warning("%s already %s all the keycards and skull keys.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "have" : "has"));
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
                C_Warning("%s already %s all the keycards.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "have" : "has"));
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
                C_Warning("%s already %s all the skull keys.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "have" : "has"));
                return;
            }
        }
        else if (M_StringCompare(parm, "pistol"))
        {
            if (viewplayer->weaponowned[wp_pistol])
            {
                C_Warning("%s already %s a pistol.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "have" : "has"));
                return;
            }

            viewplayer->weaponowned[wp_pistol] = true;
            oldweaponsowned[wp_pistol] = true;
            viewplayer->pendingweapon = wp_pistol;
            C_HideConsole();
            return;
        }
        else
        {
            int num = -1;

            sscanf(parm, "%10d", &num);

            for (int i = 0; i < NUMMOBJTYPES; i++)
            {
                if ((mobjinfo[i].flags & MF_SPECIAL)
                    && (M_StringCompare(parm, removenonalpha(mobjinfo[i].name1))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name2)))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name3)))
                        || (num == mobjinfo[i].doomednum && num != -1)))
                {
                    dboolean    old_freeze = freeze;

                    if (gamemode != commercial && (i == MT_SUPERSHOTGUN || i == MT_MEGA))
                    {
                        C_Warning("%s can't get %s in <i><b>%s</b></i>.", titlecase(playername), mobjinfo[i].plural1, gamedescription);
                        return;
                    }

                    if (gamemode == shareware && (i == MT_MISC7 || i == MT_MISC8 || i == MT_MISC9
                        || i == MT_MISC20 || i == MT_MISC21 || i == MT_MISC25 || i == MT_MISC28))
                    {
                        C_Warning("%s can't get %s in <i><b>%s</b></i>.", titlecase(playername), mobjinfo[i].plural1, gamedescription);
                        return;
                    }

                    freeze = false;
                    P_TouchSpecialThing(P_SpawnMobj(viewx, viewy, viewz, i), viewplayer->mo, false, false);
                    freeze = old_freeze;
                    C_HideConsole();
                    break;
                }
            }
        }

        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
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
    ShellExecute(NULL, "open", PACKAGE_WIKIHELPURL, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__linux__)
    system("xdg-open " PACKAGE_WIKIHELPURL);
#elif defined(__APPLE__)
    system("open " PACKAGE_WIKIHELPURL);
#else
    C_HideConsoleFast();
    M_ShowHelp(0);
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
        C_ShowDescription(C_GetIndex("if"));
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
    if (gamestate == GS_LEVEL)
    {
        char    *parm = removenonalpha(parms);

        killcmdmobj = NULL;

        if (!*parm)
            return true;

        if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
            return (viewplayer->health > 0);

        if (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters") || M_StringCompare(parm, "all"))
            return true;

        if (M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends"))
            return true;

        if (M_StringCompare(parm, "missile") || M_StringCompare(parm, "missiles"))
            return true;

        for (int i = 0; i < NUMMOBJTYPES; i++)
        {
            int num = -1;

            killcmdtype = mobjinfo[i].doomednum;

            if (killcmdtype >= 0
                && (M_StringCompare(parm, removenonalpha(mobjinfo[i].name1))
                    || M_StringCompare(parm, removenonalpha(mobjinfo[i].plural1))
                    || (*mobjinfo[i].name2 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name2)))
                    || (*mobjinfo[i].plural2 && M_StringCompare(parm, removenonalpha(mobjinfo[i].plural2)))
                    || (*mobjinfo[i].name3 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name3)))
                    || (*mobjinfo[i].plural3 && M_StringCompare(parm, removenonalpha(mobjinfo[i].plural3)))
                    || (sscanf(parm, "%10d", &num) == 1 && num == killcmdtype && num != -1)))
            {
                dboolean    kill = true;

                if (killcmdtype == WolfensteinSS && bfgedition && !states[S_SSWV_STND].dehacked)
                    killcmdtype = Zombieman;

                if (!(mobjinfo[i].flags & MF_SHOOTABLE))
                    kill = false;

                return kill;
            }
        }

        for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
        {
            mobj_t  *mobj = (mobj_t *)th;

            if (*mobj->name && M_StringCompare(parm, removenonalpha(mobj->name)))
            {
                killcmdmobj = mobj;
                return true;
            }
        }
    }

    return false;
}

void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);

void kill_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);
    char    buffer[1024];

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex("kill"));
        C_Output("<b>%s</b> %s", cmd, KILLCMDFORMAT);
    }
    else if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
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
        dboolean    friends = (M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends"));
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
                            else if ((flags & MF_SHOOTABLE) && type != MT_PLAYER && type != MT_BARREL && type != MT_BOSSBRAIN
                                && (type != MT_HEAD || !hacx))
                            {
                                thing->flags2 |= MF2_MASSACRE;
                                P_DamageMobj(thing, NULL, NULL, thing->health, false);

                                if (!(flags & MF_NOBLOOD))
                                {
                                    const int   r = M_RandomInt(-1, 1);

                                    thing->momx += FRACUNIT * r;
                                    thing->momy += FRACUNIT * M_RandomIntNoRepeat(-1, 1, (!r ? 0 : 2));
                                }

                                kills++;
                            }
                        }
                    }

                    thing = thing->snext;
                }
            }

            if (kills)
            {
                M_snprintf(buffer, sizeof(buffer), "%s%s %smonster%s in this map %s been killed.", (kills == 1 ? "" : "All "),
                    commify(kills), (kills < prevkills ? "remaining " : ""), (kills == 1 ? "" : "s"), (kills == 1 ? "has" : "have"));
                C_Output(buffer);
                C_HideConsole();
                HU_SetPlayerMessage(buffer, false, false);
                message_dontfuckwithme = true;
                viewplayer->cheated++;
                stat_cheated = SafeAdd(stat_cheated, 1);
                M_SaveCVARs();
            }
            else
                C_Warning("There are no monsters %s kill in this map.", (!totalkills ? "to" : "left to"));
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
                M_snprintf(buffer, sizeof(buffer), "%s %s missile%s %s exploded.", (kills == 1 ? "The" : "All"), commify(kills),
                    (kills == 1 ? "" : "s"), (kills == 1 ? "has" : "have"));
                C_Output(buffer);
                C_HideConsole();
                HU_SetPlayerMessage(buffer, false, false);
                message_dontfuckwithme = true;
                viewplayer->cheated++;
                stat_cheated = SafeAdd(stat_cheated, 1);
                M_SaveCVARs();
            }
            else
                C_Warning("There are no missiles to explode.");
        }
        else if (killcmdmobj)
        {
            killcmdmobj->flags2 |= MF2_MASSACRE;
            P_DamageMobj(killcmdmobj, NULL, NULL, killcmdmobj->health, false);

            if (!(killcmdmobj->flags & MF_NOBLOOD))
            {
                const int   r = M_RandomInt(-1, 1);

                killcmdmobj->momx += FRACUNIT * r;
                killcmdmobj->momy += FRACUNIT * M_RandomIntNoRepeat(-1, 1, (!r ? 0 : 2));
            }

            M_snprintf(buffer, sizeof(buffer), "%s was killed.", sentencecase(parm));
            C_Output(buffer);
            C_HideConsole();
            HU_SetPlayerMessage(buffer, false, false);
            message_dontfuckwithme = true;
            viewplayer->cheated++;
            stat_cheated = SafeAdd(stat_cheated, 1);
            M_SaveCVARs();
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

                                thing->momx += FRACUNIT * r;
                                thing->momy += FRACUNIT * M_RandomIntNoRepeat(-1, 1, (!r ? 0 : 2));
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
                M_snprintf(buffer, sizeof(buffer), "%s %s %s in this map %s %s.", (kills == 1 ? "The" : "All"), commify(kills),
                    (kills == 1 ? mobjinfo[type].name1 : mobjinfo[type].plural1), (kills == 1 ? "has" : "have"),
                    (type == MT_BARREL ? "exploded" : "been killed"));
                C_Output(buffer);
                C_HideConsole();
                HU_SetPlayerMessage(buffer, false, false);
                message_dontfuckwithme = true;
                viewplayer->cheated++;
                stat_cheated = SafeAdd(stat_cheated, 1);
                M_SaveCVARs();
            }
            else
            {
                if (gamemode != commercial)
                {
                    if (killcmdtype >= ArchVile && killcmdtype <= MonstersSpawner)
                    {
                        C_Warning("There are no %s in <i><b>%s</b></i>.", mobjinfo[type].plural1, gamedescription);
                        return;
                    }

                    if (gamemode == shareware && (killcmdtype == Cyberdemon || killcmdtype == SpiderMastermind))
                    {
                        C_Warning("There are no %s in <i><b>%s</b></i>.", mobjinfo[type].plural1, gamedescription);
                        return;
                    }
                }

                C_Warning("There are no %s %s %s.", mobjinfo[type].plural1, (dead ? "left to" : "to"),
                    (type == MT_BARREL ? "explode" : "kill"));
            }
        }
    }
}

//
// load CCMD
//
static void load_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex("load"));
        C_Output("<b>%s</b> %s", cmd, LOADCMDFORMAT);
        return;
    }

    G_LoadGame(M_StringJoin((M_StringStartsWith(parms, savegamefolder) ? "" : savegamefolder), parms,
        (M_StringEndsWith(parms, ".save") ? "" : ".save"), NULL));
}

//
// map CCMD
//
extern int  idclevtics;

static dboolean map_cmd_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;
    else
    {
        char        *map = uppercase(parms);
        dboolean    result = false;

        mapcmdepisode = 0;
        mapcmdmap = 0;

        if (M_StringCompare(map, "first"))
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
        else if ((M_StringCompare(map, "previous") || M_StringCompare(map, "prev")) && gamestate != GS_TITLESCREEN)
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
        else if (M_StringCompare(map, "next") && gamestate != GS_TITLESCREEN)
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
        else if (M_StringCompare(map, "last"))
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
        else if (M_StringCompare(map, "random"))
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
                mapcmdmap = M_RandomIntNoRepeat(1, 8, gamemap);
                M_snprintf(mapcmdlump, sizeof(mapcmdlump), "E%iM%i", mapcmdepisode, mapcmdmap);
                result = true;
            }
        }
        else if (M_StringCompare(map, "E1M4B") && gamemission == doom && gamemode != shareware && !chex)
        {
            mapcmdepisode = 1;
            mapcmdmap = 4;
            M_StringCopy(speciallumpname, "E1M4B", 6);
            M_StringCopy(mapcmdlump, "E1M4B", 6);
            result = true;
        }
        else if (M_StringCompare(map, "E1M8B") && gamemission == doom && gamemode != shareware && !chex)
        {
            mapcmdepisode = 1;
            mapcmdmap = 8;
            M_StringCopy(speciallumpname, "E1M8B", 6);
            M_StringCopy(mapcmdlump, "E1M8B", 6);
            result = true;
        }
        else
        {
            M_StringCopy(mapcmdlump, map, sizeof(mapcmdlump));

            if (gamemode == commercial)
            {
                mapcmdepisode = 1;

                if (sscanf(map, "MAP0%1i", &mapcmdmap) == 1 || sscanf(map, "MAP%2i", &mapcmdmap) == 1)
                {
                    if (!((BTSX && W_CheckMultipleLumps(map) == 1) || (gamemission == pack_nerve && mapcmdmap > 9)))
                    {
                        if (gamestate != GS_LEVEL && gamemission == pack_nerve)
                        {
                            gamemission = doom2;
                            expansion = 1;
                        }

                        result = (W_CheckNumForName(map) >= 0);
                    }
                }
                else if (BTSX)
                {
                    if (sscanf(map, "MAP%02iC", &mapcmdmap) == 1)
                        result = (W_CheckNumForName(map) >= 0);
                    else
                    {
                        if (sscanf(map, "E%1iM0%1i", &mapcmdepisode, &mapcmdmap) != 2)
                            sscanf(map, "E%1iM%2i", &mapcmdepisode, &mapcmdmap);

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
            else if (sscanf(map, "E%1iM%1i", &mapcmdepisode, &mapcmdmap) == 2)
                result = (chex && mapcmdepisode > 1 ? false : (W_CheckNumForName(map) >= 0));
            else if (FREEDOOM && sscanf(map, "C%1iM%1i", &mapcmdepisode, &mapcmdmap) == 2)
            {
                char    lump[5];

                M_snprintf(lump, sizeof(lump), "E%iM%i", mapcmdepisode, mapcmdmap);
                result = (W_CheckNumForName(lump) >= 0);
            }
        }

        free(map);
        return result;
    }
}

static void map_cmd_func2(char *cmd, char *parms)
{
    char    buffer[1024];

    if (!*parms)
    {
        C_ShowDescription(C_GetIndex("map"));
        C_Output("<b>%s</b> %s", cmd, MAPCMDFORMAT);
        return;
    }

    M_snprintf(buffer, sizeof(buffer), (M_StringCompare(mapcmdlump, mapnum) ? s_STSTR_CLEVSAME : s_STSTR_CLEV), mapcmdlump);
    C_Output(buffer);
    HU_SetPlayerMessage(buffer, false, false);
    message_dontfuckwithme = true;

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
        G_DeferredInitNew((gamestate == GS_LEVEL ? gameskill : skilllevel - 1), gameepisode, gamemap);
        C_HideConsoleFast();
    }

    viewplayer->cheated++;
    stat_cheated = SafeAdd(stat_cheated, 1);
    M_SaveCVARs();
}

//
// maplist CCMD
//
extern int  dehcount;
extern char **mapnames[];
extern char **mapnames2[];
extern char **mapnames2_bfg[];
extern char **mapnamesp[];
extern char **mapnamest[];
extern char **mapnamesn[];

static void maplist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[8] = { 40, 93, 370, 0, 0, 0, 0, 0 };
    int         count = 0;
    char        (*maplist)[256] = malloc(numlumps * sizeof(char *));

    C_Header(maplistheader);

    // search through lumps for maps
    for (int i = 0; i < numlumps; i++)
    {
        int         ep = -1;
        int         map = -1;
        char        lump[6];
        char        wadname[MAX_PATH];
        dboolean    replaced;
        dboolean    pwad;
        char        mapinfoname[128];

        M_StringCopy(lump, uppercase(lumpinfo[i]->name), sizeof(lump));

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
                M_StringCopy(speciallumpname, lump, 6);
            else
                sscanf(lump, "E%1iM%1i", &ep, &map);
        }

        if (ep-- == -1 || map-- == -1)
            continue;

        M_StringCopy(wadname, leafname(lumpinfo[i]->wadfile->path), sizeof(wadname));
        replaced = (W_CheckMultipleLumps(lump) > 1 && !chex && !FREEDOOM);
        pwad = (lumpinfo[i]->wadfile->type == PWAD);
        M_StringCopy(mapinfoname, P_GetMapName(ep * 10 + map + 1), sizeof(mapinfoname));
        speciallumpname[0] = '\0';

        switch (gamemission)
        {
            case doom:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump, (replaced && dehcount == 1
                        && !*mapinfoname ? "-" : titlecase(*mapinfoname ? mapinfoname : *mapnames[ep * 9 + map])), wadname);

                break;

            case doom2:
                if ((!M_StringCompare(wadname, "NERVE.WAD") && ((!replaced || pwad || nerve) && (pwad || !BTSX))) || hacx)
                {
                    if (BTSX)
                    {
                        if (!M_StringCompare(wadname, "DOOM2.WAD"))
                            M_snprintf(maplist[count++], 256, "%s</b></i>\t%s",
                                titlecase(M_StringReplace(*mapnames2[map], ": ", "\t<i><b>")), wadname);
                    }
                    else
                        M_snprintf(maplist[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump,
                            (replaced && dehcount == 1 && !nerve && !*mapinfoname ? "-" : titlecase(*mapinfoname ? mapinfoname :
                            (bfgedition ? *mapnames2_bfg[map] : *mapnames2[map]))), wadname);
                }

                break;

            case pack_nerve:
                if (M_StringCompare(wadname, "NERVE.WAD"))
                    M_snprintf(maplist[count++], 256, "%s\t<i><b>%s</b></i>\t%s",
                        lump, titlecase(*mapinfoname ? mapinfoname : *mapnamesn[map]), wadname);

                break;

            case pack_plut:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump, (replaced && dehcount == 1 && !*mapinfoname ?
                        "-" : titlecase(*mapinfoname ? mapinfoname : *mapnamesp[map])), wadname);

                break;

            case pack_tnt:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t<i><b>%s</b></i>\t%s", lump, (replaced && dehcount == 1 && !*mapinfoname ?
                        "-" : titlecase(*mapinfoname ? mapinfoname : *mapnamest[map])), wadname);

                break;

            default:
                break;
        }
    }

    // sort the map list
    for (int i = 0; i < count; i++)
        for (int j = i + 1; j < count; j++)
            if (strcmp(maplist[i], maplist[j]) > 0)
            {
                char    temp[256];

                M_StringCopy(temp, maplist[i], 256);
                M_StringCopy(maplist[i], maplist[j], 256);
                M_StringCopy(maplist[j], temp, 256);
            }

    // display the map list
    for (int i = 0; i < count; i++)
        C_TabbedOutput(tabs, "%i.\t%s", i + 1, maplist[i]);

    free(maplist);
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
#define AMSP    AM" and "SP
#define BKTH2   BK" and "TH2
#define DC2DB   DC2" and "DB
#define DCMC    DC" and "MC
#define DCTH2   DC" and "TH2
#define JRTH    JR" and "TH
#define JSTH2   JS" and "TH2
#define MSJL    MS" and "JL
#define RPJM2   RP" and "JM2
#define SPTH    SP" and "TH

static void mapstats_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[8] = { 120, 240, 0, 0, 0, 0, 0, 0 };

    C_Header(mapstatsheader);

    C_TabbedOutput(tabs, "Title\t<b><i>%s</i></b>", titlecase(maptitle));

    {
        const char *authors[][6] =
        {
            /* xy      doom   doom2  tnt    plut   nerve */
            /* 00 */ { "",    "",    "",    DCMC,  "" },
            /* 01 */ { "",    SP,    TM,    DCMC,  RM },
            /* 02 */ { "",    AM,    JW,    DCMC,  AI },
            /* 03 */ { "",    AM,    RPJM2, DCMC,  RM },
            /* 04 */ { "",    AM,    TH2,   DCMC,  RM },
            /* 05 */ { "",    AM,    JD,    DCMC,  AI },
            /* 06 */ { "",    AM,    JSTH2, DCMC,  AI },
            /* 07 */ { "",    AMSP,  AD,    DCMC,  AI },
            /* 08 */ { "",    SP,    JM2,   DCMC,  AI },
            /* 09 */ { "",    SP,    JSTH2, DCMC,  RM },
            /* 10 */ { "",    SPTH,  TM,    DCMC,  "" },
            /* 11 */ { JR,    JR,    DJ,    DCMC,  "" },
            /* 12 */ { JR,    SP,    JL,    DCMC,  "" },
            /* 13 */ { JR,    SP,    BKTH2, DCMC,  "" },
            /* 14 */ { JRTH,  AM,    RP,    DCMC,  "" },
            /* 15 */ { JR,    JR,    WW,    DCMC,  "" },
            /* 16 */ { JR,    SP,    AA,    DCMC,  "" },
            /* 17 */ { JR,    JR,    TM,    DCMC,  "" },
            /* 18 */ { SPTH,  SP,    DCTH2, DCMC,  "" },
            /* 19 */ { JR,    SP,    TH2,   DCMC,  "" },
            /* 20 */ { DC2DB, JR,    DO,    DCMC,  "" },
            /* 21 */ { SPTH,  SP,    DO,    DCMC,  "" },
            /* 22 */ { SPTH,  AM,    CB,    DCMC,  "" },
            /* 23 */ { SPTH,  SP,    PT,    DCMC,  "" },
            /* 24 */ { SPTH,  SP,    DJ,    DCMC,  "" },
            /* 25 */ { SP,    SG,    JM,    DCMC,  "" },
            /* 26 */ { SP,    JR,    MSJL,  DCMC,  "" },
            /* 27 */ { SPTH,  SP,    DO,    DCMC,  "" },
            /* 28 */ { SP,    SP,    MC,    DCMC,  "" },
            /* 29 */ { SP,    JR,    JS,    DCMC,  "" },
            /* 30 */ { "",    SP,    JS,    DCMC,  "" },
            /* 31 */ { SP,    SP,    DC,    DCMC,  "" },
            /* 32 */ { SP,    SP,    DC,    DCMC,  "" },
            /* 33 */ { SPTH,  MB,    "",    "",    "" },
            /* 34 */ { SP,    "",    "",    "",    "" },
            /* 35 */ { SP,    "",    "",    "",    "" },
            /* 36 */ { SP,    "",    "",    "",    "" },
            /* 37 */ { SPTH,  "",    "",    "",    "" },
            /* 38 */ { SP,    "",    "",    "",    "" },
            /* 39 */ { SP,    "",    "",    "",    "" },
            /* 40 */ { "",    "",    "",    "",    "" },
            /* 41 */ { AM,    "",    "",    "",    "" },
            /* 42 */ { JR,    "",    "",    "",    "" },
            /* 43 */ { SG,    "",    "",    "",    "" },
            /* 44 */ { AM,    "",    "",    "",    "" },
            /* 45 */ { TW,    "",    "",    "",    "" },
            /* 46 */ { JR,    "",    "",    "",    "" },
            /* 47 */ { JA,    "",    "",    "",    "" },
            /* 48 */ { SG,    "",    "",    "",    "" },
            /* 49 */ { TW,    "",    "",    "",    "" },
            /* 50 */ { "",    "",    "",    "",    "" },
            /* 51 */ { JR,    "",    "",    "",    "" },
            /* 52 */ { JR,    "",    "",    "",    "" },
            /* 53 */ { JR,    "",    "",    "",    "" },
            /* 54 */ { JR,    "",    "",    "",    "" },
            /* 55 */ { JR,    "",    "",    "",    "" },
            /* 56 */ { JR,    "",    "",    "",    "" },
            /* 57 */ { JR,    "",    "",    "",    "" },
            /* 58 */ { JR,    "",    "",    "",    "" },
            /* 59 */ { JR,    "",    "",    "",    "" }
        };

        const int   i = (gamemission == doom ? gameepisode * 10 : 0) + gamemap;
        const char  *author = P_GetMapAuthor(i);

        if (*author)
            C_TabbedOutput(tabs, "Author\t<b>%s</b>", author);
        else if (canmodify && *authors[i][gamemission])
            C_TabbedOutput(tabs, "Author\t<b>%s</b>", authors[i][gamemission]);
    }

    {
        int i = (nerve && gamemission == doom2 ? W_GetLastNumForName(mapnum) : W_CheckNumForName(mapnum));

        C_TabbedOutput(tabs, "%s\t<b>%s</b>", (lumpinfo[i]->wadfile->type == IWAD ? "IWAD" : "PWAD"),
            leafname(lumpinfo[i]->wadfile->path));

        if (lumpinfo[i]->wadfile->type == PWAD)
            C_TabbedOutput(tabs, "IWAD\t<b>%s</b>", leafname(lumpinfo[W_GetLastNumForName("PLAYPAL")]->wadfile->path));
    }

    C_TabbedOutput(tabs, "Compatibility\t<b>%s</b>", (mbfcompatible ? "<i>BOOM</i> and <i>MBF</i>-compatible" :
        (boomcompatible ? "<i>BOOM</i>-compatible" : (numsegs < 32768 ? "Vanilla-compatible" : "Limit removing"))));

    C_TabbedOutput(tabs, "Things\t<b>%s</b>", commify(numthings));

    C_TabbedOutput(tabs, "   Monsters\t<b>%s</b>", commify(totalkills));

    C_TabbedOutput(tabs, "   Pickups\t<b>%s</b>", commify(totalpickups));

    C_TabbedOutput(tabs, "   Decorations\t<b>%s</b>", commify(numdecorations));

    C_TabbedOutput(tabs, "   Barrels\t<b>%s</b>", commify(barrelcount));

    C_TabbedOutput(tabs, "Lines\t<b>%s</b>", commify(numlines));

    C_TabbedOutput(tabs, "Sides\t<b>%s</b>", commify(numsides));

    C_TabbedOutput(tabs, "Vertices\t<b>%s</b>", commify(numvertexes));

    C_TabbedOutput(tabs, "Segments\t<b>%s</b>", commify(numsegs));

    C_TabbedOutput(tabs, "Subsectors\t<b>%s</b>", commify(numsubsectors));

    C_TabbedOutput(tabs, "Nodes\t<b>%s</b>", commify(numnodes));

    C_TabbedOutput(tabs, "Node format\t<b>%s nodes</b>", (mapformat == DOOMBSP ? "Regular" : (mapformat == DEEPBSP ?
        "<i>DeePBSP v4</i> extended" : "<i>ZDoom</i> uncompressed, extended")));

    C_TabbedOutput(tabs, "Sectors\t<b>%s</b>", commify(numsectors));

    C_TabbedOutput(tabs, "   Secret\t<b>%s</b>", commify(totalsecret));

    C_TabbedOutput(tabs, "   Liquid\t<b>%s</b>", commify(numliquid));

    C_TabbedOutput(tabs, "   Damaging\t<b>%s</b>", commify(numdamaging));

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
                C_TabbedOutput(tabs, "Dimensions\t<b>%sx%sx%s meters</b>",
                    striptrailingzero(metricwidth, 1), striptrailingzero(metricheight, 1), striptrailingzero(metricdepth, 1));
            else
                C_TabbedOutput(tabs, "Dimensions\t<b>%sx%sx%s kilometers</b>",
                    striptrailingzero(metricwidth / METERSPERKILOMETER, 1), striptrailingzero(metricheight / METERSPERKILOMETER, 1),
                    striptrailingzero(metricdepth / METERSPERKILOMETER, 1));
        }
        else
        {
            if (width < FEETPERMILE && height < FEETPERMILE)
                C_TabbedOutput(tabs, "Dimensions\t<b>%sx%sx%s feet</b>", commify(width), commify(height), commify(depth));
            else
                C_TabbedOutput(tabs, "Dimensions\t<b>%sx%sx%s miles</b>",
                    striptrailingzero((float)width / FEETPERMILE, 2), striptrailingzero((float)height / FEETPERMILE, 2),
                    striptrailingzero((float)depth / FEETPERMILE, 2));
        }
    }

    if (mus_playing && !nomusic)
    {
        char                lumpname[9];
        int                 lumps;
        const char          *musiccomposer = P_GetMapMusicComposer((gameepisode - 1) * 10 + gamemap);
        const char          *musictitle = P_GetMapMusicTitle((gameepisode - 1) * 10 + gamemap);
        const Mix_MusicType musictype = Mix_GetMusicType(NULL);

        M_snprintf(lumpname, sizeof(lumpname), "d_%s", mus_playing->name);
        lumps = W_CheckMultipleLumps(lumpname);

        C_TabbedOutput(tabs, "Music lump\t<b>%s</b>", uppercase(lumpname));

        if (*musictitle)
            C_TabbedOutput(tabs, "Music title\t<b><i>%s</i></b>", musictitle);
        else if (sigil)
            C_TabbedOutput(tabs, "Music title\t<b><i>%s</i></b>", (buckethead ? mus_playing->title2 : mus_playing->title1));
        else if (((gamemode == commercial || gameepisode > 1) && lumps == 1)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2))
            C_TabbedOutput(tabs, "Music title\t<b><i>%s</i></b>", mus_playing->title1);

        if (*musiccomposer)
            C_TabbedOutput(tabs, "Music composer\t<b>%s</b>", musiccomposer);
        else if (sigil)
            C_TabbedOutput(tabs, "Music composer\t<b>%s</b>", (buckethead ? "Buckethead" : "James Paddock"));
        else if (((gamemode == commercial || gameepisode > 1) && lumps == 1)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2))
            C_TabbedOutput(tabs, "Music composer\t<b>%s</b>", "Bobby Prince");

        if (musmusictype)
            C_TabbedOutput(tabs, "Music format\t<b>MUS converted to MIDI</b>");
        else if (midimusictype || musictype == MUS_MID)
            C_TabbedOutput(tabs, "Music format\t<b>MIDI</b>");
        else if (musictype == MUS_OGG)
            C_TabbedOutput(tabs, "Music format\t<b>Ogg Vorbis</b>");
        else if (musictype == MUS_MP3)
            C_TabbedOutput(tabs, "Music format\t<b>MP3</b>");
        else if (musictype == MUS_WAV)
            C_TabbedOutput(tabs, "Music format\t<b>WAV</b>");
        else if (musictype == MUS_FLAC)
            C_TabbedOutput(tabs, "Music format\t<b>FLAC</b>");
        else if (musictype == MUS_MOD)
            C_TabbedOutput(tabs, "Music format\t<b>MOD</b>");
    }
}

//
// name CCMD
//
static dboolean namecmdfriendly;
static dboolean namecmdanymonster;
static char     namecmdnew[100];
static char     namecmdold[100];
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
        return (namecmdnew[0] != '\0');
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
            return (namecmdnew[0] != '\0');
        }
        else
            namecmdanymonster = false;

        for (int i = 0; i < NUMMOBJTYPES; i++)
            if ((mobjinfo[i].flags & MF_SHOOTABLE) && i != MT_PLAYER && i != MT_BARREL)
            {
                if (M_StringStartsWith(parm, removenonalpha(mobjinfo[i].name1)))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name1, sizeof(namecmdold));
                    strreplace(parm, removenonalpha(mobjinfo[i].name1), "");
                    M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
                    namecmdtype = i;
                    return (namecmdnew[0] != '\0');
                }
                else if (*mobjinfo[i].name2 && M_StringStartsWith(parm, removenonalpha(mobjinfo[i].name2)))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name2, sizeof(namecmdold));
                    strreplace(parm, removenonalpha(mobjinfo[i].name2), "");
                    M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
                    namecmdtype = i;
                    return (namecmdnew[0] != '\0');
                }
                else if (*mobjinfo[i].name3 && M_StringStartsWith(parm, removenonalpha(mobjinfo[i].name3)))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name3, sizeof(namecmdold));
                    strreplace(parm, removenonalpha(mobjinfo[i].name3), "");
                    M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
                    namecmdtype = i;
                    return (namecmdnew[0] != '\0');
                }
            }
    }

    return false;
}

static void name_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex("name"));
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
                && ((namecmdfriendly && (flags & MF_FRIEND)) || !namecmdfriendly))
                if (mobj->health > 0 && P_CheckSight(viewplayer->mo, mobj))
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
            C_Output("The nearest %s%s has been %s %s.", (namecmdfriendly ? "friendly " : ""), namecmdold,
                (*bestmobj->name ? "renamed" : "named"), namecmdnew);
            M_StringCopy(bestmobj->name, namecmdnew, sizeof(bestmobj->name));
        }
        else
            C_Warning("%s %s%s couldn't be found nearby.",
                (isvowel(namecmdold[0]) ? "An" : "A"), (namecmdfriendly ? "friendly " : ""), namecmdold);
    }
}

//
// newgame CCMD
//
static void newgame_cmd_func2(char *cmd, char *parms)
{
    C_HideConsoleFast();
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
            C_Warning(PENDINGCHANGE);
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
        C_Warning(PENDINGCHANGE);
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
        M_snprintf(namebuf, sizeof(namebuf), "ds%s", S_sfx[i].name);

        if (M_StringCompare(parms, namebuf) && W_CheckNumForName(namebuf) != -1)
        {
            playcmdid = i;
            playcmdtype = 1;
            return true;
        }
    }

    for (int i = 1; i < NUMMUSIC; i++)
    {
        M_snprintf(namebuf, sizeof(namebuf), "d_%s", S_music[i].name);

        if (M_StringCompare(parms, namebuf) && W_CheckNumForName(namebuf) != -1)
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
        C_ShowDescription(C_GetIndex("play"));
        C_Output("<b>%s</b> %s", cmd, PLAYCMDFORMAT);
    }
    else if (playcmdtype == 1)
        S_StartSound(NULL, playcmdid);
    else
        S_ChangeMusic(playcmdid, true, true, false);
}

static skill_t favoriteskilllevel(void)
{
    unsigned int    skilllevelstat = 0;
    skill_t         favorite = sk_none;

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

static char *distance(fixed_t value, dboolean showunits)
{
    char    *result = malloc(20);

    value /= UNITSPERFOOT;

    if (units == units_metric)
    {
        const float meters = value / FEETPERMETER;

        if (!meters)
            M_StringCopy(result, (showunits ? "0 meters" : "0"), 20);
        else if (meters < METERSPERKILOMETER)
            M_snprintf(result, 20, "%s%s%s", striptrailingzero(meters, 1), (showunits ? " meter" : ""),
                (meters == 1.0f || !showunits ? "" : "s"));
        else
            M_snprintf(result, 20, "%s%s%s", striptrailingzero(meters / METERSPERKILOMETER, 2),
                (showunits ? " kilometer" : ""), (meters == METERSPERKILOMETER || !showunits ? "" : "s"));
    }
    else
    {
        if (value < FEETPERMILE)
            M_snprintf(result, 20, "%s%s", commify(value), (showunits ? (value == 1 ? " foot" : " feet") : ""));
        else
            M_snprintf(result, 20, "%s%s%s", striptrailingzero((float)value / FEETPERMILE, 2),
                (showunits ? " mile" : ""), (value == FEETPERMILE || !showunits ? "" : "s"));
    }

    return result;
}

//
// playerstats CCMD
//
static void C_PlayerStats_Game(void)
{
    const int   tabs[8] = { 160, 281, 0, 0, 0, 0, 0, 0 };
    skill_t     favorite = favoriteskilllevel();
    const int   time1 = leveltime / TICRATE;
    const int   time2 = stat_time / TICRATE;

    char **skilllevels[] =
    {
        &s_M_SKILLLEVEL1,
        &s_M_SKILLLEVEL2,
        &s_M_SKILLLEVEL3,
        &s_M_SKILLLEVEL4,
        &s_M_SKILLLEVEL5
    };

    C_Header(playerstatsheader);

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

        C_TabbedOutput(tabs, "Map explored\t<b>%s%%</b>\t-", striptrailingzero(mappedwalls * 100.0f / totalwalls, 1));
    }

    C_TabbedOutput(tabs, "Maps completed\t-\t<b>%s</b>", commify(stat_mapscompleted));

    C_TabbedOutput(tabs, "Games saved\t<b>%s</b>\t<b>%s</b>", commify(viewplayer->gamessaved), commify(stat_gamessaved));

    if (favorite == sk_none)
        C_TabbedOutput(tabs, "Favorite skill level\t-\t-");
    else
    {
        char    *level = titlecase(*skilllevels[favorite]);

        if (level[strlen(level) - 1] == '.')
            level[strlen(level) - 1] = '\0';

        C_TabbedOutput(tabs, "Favorite skill level\t-\t<b><i>%s</i></b>", level);
        free(level);
    }

    {
        int killcount = 0;

        for (int i = 0; i < NUMMOBJTYPES; i++)
            killcount += viewplayer->mobjcount[i];

        C_TabbedOutput(tabs, "Monsters killed\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            commify(killcount), commify(totalkills), (totalkills ? killcount * 100 / totalkills : 0), commify(stat_monsterskilled));
    }

    if (gamemode == commercial)
    {
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            sentencecase(mobjinfo[MT_BABY].plural1), commify(viewplayer->mobjcount[MT_BABY]), commify(monstercount[MT_BABY]),
            (monstercount[MT_BABY] ? viewplayer->mobjcount[MT_BABY] * 100 / monstercount[MT_BABY] : 0),
            commify(stat_monsterskilled_arachnotrons));

        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            sentencecase(mobjinfo[MT_VILE].plural1), commify(viewplayer->mobjcount[MT_VILE]), commify(monstercount[MT_VILE]),
            (monstercount[MT_VILE] ? viewplayer->mobjcount[MT_VILE] * 100 / monstercount[MT_VILE] : 0),
            commify(stat_monsterskilled_archviles));
    }

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_BRUISER].plural1), commify(viewplayer->mobjcount[MT_BRUISER]), commify(monstercount[MT_BRUISER]),
        (monstercount[MT_BRUISER] ? viewplayer->mobjcount[MT_BRUISER] * 100 / monstercount[MT_BRUISER] : 0),
        commify(stat_monsterskilled_baronsofhell));

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_HEAD].plural1), commify(viewplayer->mobjcount[MT_HEAD]), commify(monstercount[MT_HEAD]),
        (monstercount[MT_HEAD] ? viewplayer->mobjcount[MT_HEAD] * 100 / monstercount[MT_HEAD] : 0),
        commify(stat_monsterskilled_cacodemons));

    if (gamemode != shareware)
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            sentencecase(mobjinfo[MT_CYBORG].plural1), commify(viewplayer->mobjcount[MT_CYBORG]), commify(monstercount[MT_CYBORG]),
            (monstercount[MT_CYBORG] ? viewplayer->mobjcount[MT_CYBORG] * 100 / monstercount[MT_CYBORG] : 0),
            commify(stat_monsterskilled_cyberdemons));

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_SERGEANT].plural1), commify(viewplayer->mobjcount[MT_SERGEANT]), commify(monstercount[MT_SERGEANT]),
        (monstercount[MT_SERGEANT] ? viewplayer->mobjcount[MT_SERGEANT] * 100 / monstercount[MT_SERGEANT] : 0),
        commify(stat_monsterskilled_demons));

    if (gamemode == commercial)
    {
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            sentencecase(mobjinfo[MT_CHAINGUY].plural1), commify(viewplayer->mobjcount[MT_CHAINGUY]), commify(monstercount[MT_CHAINGUY]),
            (monstercount[MT_CHAINGUY] ? viewplayer->mobjcount[MT_CHAINGUY] * 100 / monstercount[MT_CHAINGUY] : 0),
            commify(stat_monsterskilled_heavyweapondudes));

        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            sentencecase(mobjinfo[MT_KNIGHT].plural1), commify(viewplayer->mobjcount[MT_KNIGHT]), commify(monstercount[MT_KNIGHT]),
            (monstercount[MT_KNIGHT] ? viewplayer->mobjcount[MT_KNIGHT] * 100 / monstercount[MT_KNIGHT] : 0),
            commify(stat_monsterskilled_hellknights));
    }

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_TROOP].plural1), commify(viewplayer->mobjcount[MT_TROOP]), commify(monstercount[MT_TROOP]),
        (monstercount[MT_TROOP] ? viewplayer->mobjcount[MT_TROOP] * 100 / monstercount[MT_TROOP] : 0),
        commify(stat_monsterskilled_imps));

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_SKULL].plural1), commify(viewplayer->mobjcount[MT_SKULL]), commify(monstercount[MT_SKULL]),
        (monstercount[MT_SKULL] ? viewplayer->mobjcount[MT_SKULL] * 100 / monstercount[MT_SKULL] : 0),
        commify(stat_monsterskilled_lostsouls));

    if (gamemode == commercial)
    {
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            sentencecase(mobjinfo[MT_FATSO].plural1), commify(viewplayer->mobjcount[MT_FATSO]), commify(monstercount[MT_FATSO]),
            (monstercount[MT_FATSO] ? viewplayer->mobjcount[MT_FATSO] * 100 / monstercount[MT_FATSO] : 0),
            commify(stat_monsterskilled_mancubi));

        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            sentencecase(mobjinfo[MT_PAIN].plural1), commify(viewplayer->mobjcount[MT_PAIN]), commify(monstercount[MT_PAIN]),
            (monstercount[MT_PAIN] ? viewplayer->mobjcount[MT_PAIN] * 100 / monstercount[MT_PAIN] : 0),
            commify(stat_monsterskilled_painelementals));
    }

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_UNDEAD].plural1), commify(viewplayer->mobjcount[MT_UNDEAD]), commify(monstercount[MT_UNDEAD]),
        (monstercount[MT_UNDEAD] ? viewplayer->mobjcount[MT_UNDEAD] * 100 / monstercount[MT_UNDEAD] : 0),
        commify(stat_monsterskilled_revenants));

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_SHOTGUY].plural1), commify(viewplayer->mobjcount[MT_SHOTGUY]), commify(monstercount[MT_SHOTGUY]),
        (monstercount[MT_SHOTGUY] ? viewplayer->mobjcount[MT_SHOTGUY] * 100 / monstercount[MT_SHOTGUY] : 0),
        commify(stat_monsterskilled_shotgunguys));

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_SHADOWS].plural1), commify(viewplayer->mobjcount[MT_SHADOWS]), commify(monstercount[MT_SHADOWS]),
        (monstercount[MT_SHADOWS] ? viewplayer->mobjcount[MT_SHADOWS] * 100 / monstercount[MT_SHADOWS] : 0),
        commify(stat_monsterskilled_spectres));

    if (gamemode != shareware)
        C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
            sentencecase(mobjinfo[MT_SPIDER].plural1), commify(viewplayer->mobjcount[MT_SPIDER]), commify(monstercount[MT_SPIDER]),
            (monstercount[MT_SPIDER] ? viewplayer->mobjcount[MT_SPIDER] * 100 / monstercount[MT_SPIDER] : 0),
            commify(stat_monsterskilled_spidermasterminds));

    C_TabbedOutput(tabs, "   %s\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        sentencecase(mobjinfo[MT_POSSESSED].plural1), commify(viewplayer->mobjcount[MT_POSSESSED]), commify(monstercount[MT_POSSESSED]),
        (monstercount[MT_POSSESSED] ? viewplayer->mobjcount[MT_POSSESSED] * 100 / monstercount[MT_POSSESSED] : 0),
        commify(stat_monsterskilled_zombiemen));

    C_TabbedOutput(tabs, "Barrels exploded\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        commify(viewplayer->mobjcount[MT_BARREL]), commify(barrelcount),
        (barrelcount ? viewplayer->mobjcount[MT_BARREL] * 100 / barrelcount : 0), commify(stat_barrelsexploded));

    C_TabbedOutput(tabs, "Items picked up\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        commify(viewplayer->itemcount), commify(totalitems), (totalitems ? viewplayer->itemcount * 100 / totalitems : 0),
        commify(stat_itemspickedup));

    C_TabbedOutput(tabs, "   Ammo\t<b>%s bullet%s</b>\t<b>%s bullet%s</b>",
        commify(viewplayer->itemspickedup_ammo_bullets), (viewplayer->itemspickedup_ammo_bullets == 1 ? "" : "s"),
        commify(stat_itemspickedup_ammo_bullets), (stat_itemspickedup_ammo_bullets == 1 ? "" : "s"));

    C_TabbedOutput(tabs, "\t<b>%s cell%s</b>\t<b>%s cell%s</b>",
        commify(viewplayer->itemspickedup_ammo_cells), (viewplayer->itemspickedup_ammo_cells == 1 ? "" : "s"),
        commify(stat_itemspickedup_ammo_cells), (stat_itemspickedup_ammo_cells == 1 ? "" : "s"));

    C_TabbedOutput(tabs, "\t<b>%s rocket%s</b>\t<b>%s rocket%s</b>",
        commify(viewplayer->itemspickedup_ammo_rockets), (viewplayer->itemspickedup_ammo_rockets == 1 ? "" : "s"),
        commify(stat_itemspickedup_ammo_rockets), (stat_itemspickedup_ammo_rockets == 1 ? "" : "s"));

    C_TabbedOutput(tabs, "\t<b>%s shell%s</b>\t<b>%s shell%s</b>",
        commify(viewplayer->itemspickedup_ammo_shells), (viewplayer->itemspickedup_ammo_shells == 1 ? "" : "s"),
        commify(stat_itemspickedup_ammo_shells), (stat_itemspickedup_ammo_shells == 1 ? "" : "s"));

    C_TabbedOutput(tabs, "   Armor\t<b>%s%%</b>\t<b>%s%%</b>",
        commify(viewplayer->itemspickedup_armor), commify(stat_itemspickedup_armor));

    C_TabbedOutput(tabs, "   Health\t<b>%s%%</b>\t<b>%s%%</b>",
        commify(viewplayer->itemspickedup_health), commify(stat_itemspickedup_health));

    C_TabbedOutput(tabs, "Secrets revealed\t<b>%s of %s (%i%%)</b>\t<b>%s</b>",
        commify(viewplayer->secretcount), commify(totalsecret), (totalsecret ? viewplayer->secretcount * 100 / totalsecret : 0),
        commify(stat_secretsrevealed));

    C_TabbedOutput(tabs, "Time played\t<b>%02i:%02i:%02i</b>\t<b>%02i:%02i:%02i</b>",
        time1 / 3600, (time1 % 3600) / 60, (time1 % 3600) % 60, time2 / 3600, (time2 % 3600) / 60, (time2 % 3600) % 60);

    C_TabbedOutput(tabs, "Damage inflicted\t<b>%s%%</b>\t<b>%s%%</b>",
        commify(viewplayer->damageinflicted), commify(stat_damageinflicted));

    C_TabbedOutput(tabs, "Damage received\t<b>%s%%</b>\t<b>%s%%</b>",
        commify(viewplayer->damagereceived), commify(stat_damagereceived));

    C_TabbedOutput(tabs, "Deaths\t<b>%s</b>\t<b>%s</b>", commify(viewplayer->deaths), commify(stat_deaths));

    C_TabbedOutput(tabs, "Cheated\t<b>%s</b>\t<b>%s</b>", commify(viewplayer->cheated), commify(stat_cheated));

    C_TabbedOutput(tabs, "Shots fired\t<b>%s</b>\t<b>%s</b>", commify(viewplayer->shotsfired), commify(stat_shotsfired));

    C_TabbedOutput(tabs, "Shots hit\t<b>%s</b>\t<b>%s</b>", commify(viewplayer->shotshit), commify(stat_shotshit));

    C_TabbedOutput(tabs, "Weapon accuracy\t<b>%s%%</b>\t<b>%s%%</b>",
        (viewplayer->shotsfired ? striptrailingzero(viewplayer->shotshit * 100.0f / viewplayer->shotsfired, 1) : "0"),
        (stat_shotsfired ? striptrailingzero(stat_shotshit * 100.0f / stat_shotsfired, 1) : "0"));

    C_TabbedOutput(tabs, "Distance traveled\t<b>%s</b>\t<b>%s</b>",
        distance(viewplayer->distancetraveled, true), distance(stat_distancetraveled, true));
}

static void C_PlayerStats_NoGame(void)
{
    const int   tabs[8] = { 160, 281, 0, 0, 0, 0, 0, 0 };
    skill_t     favorite = favoriteskilllevel();
    const int   time2 = stat_time / TICRATE;

    char **skilllevels[] =
    {
        &s_M_SKILLLEVEL1,
        &s_M_SKILLLEVEL2,
        &s_M_SKILLLEVEL3,
        &s_M_SKILLLEVEL4,
        &s_M_SKILLLEVEL5
    };

    C_Header(playerstatsheader);

    C_TabbedOutput(tabs, "Maps completed\t-\t<b>%s</b>", commify(stat_mapscompleted));

    C_TabbedOutput(tabs, "Games saved\t-\t<b>%s</b>", commify(stat_gamessaved));

    if (favorite == sk_none)
        C_TabbedOutput(tabs, "Favorite skill level\t-\t-");
    else
    {
        char    *level = titlecase(*skilllevels[favorite]);

        if (level[strlen(level) - 1] == '.')
            level[strlen(level) - 1] = '\0';

        C_TabbedOutput(tabs, "Favorite skill level\t-\t<b><i>%s</i></b>", level);
        free(level);
    }

    C_TabbedOutput(tabs, "Monsters killed\t-\t<b>%s</b>", commify(stat_monsterskilled));

    if (gamemode == commercial)
    {
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_BABY].plural1), commify(stat_monsterskilled_arachnotrons));

        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_VILE].plural1), commify(stat_monsterskilled_archviles));
    }

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_BRUISER].plural1), commify(stat_monsterskilled_baronsofhell));

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_HEAD].plural1), commify(stat_monsterskilled_cacodemons));

    if (gamemode != shareware)
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>",
            sentencecase(mobjinfo[MT_CYBORG].plural1), commify(stat_monsterskilled_cyberdemons));

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_SERGEANT].plural1), commify(stat_monsterskilled_demons));

    if (gamemode == commercial)
    {
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>",
            sentencecase(mobjinfo[MT_CHAINGUY].plural1), commify(stat_monsterskilled_heavyweapondudes));

        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>",
            sentencecase(mobjinfo[MT_KNIGHT].plural1), commify(stat_monsterskilled_hellknights));
    }

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_TROOP].plural1), commify(stat_monsterskilled_imps));

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_SKULL].plural1), commify(stat_monsterskilled_lostsouls));

    if (gamemode == commercial)
    {
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_FATSO].plural1), commify(stat_monsterskilled_mancubi));

        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>",
            sentencecase(mobjinfo[MT_PAIN].plural1), commify(stat_monsterskilled_painelementals));
    }

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_UNDEAD].plural1), commify(stat_monsterskilled_revenants));

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_SHOTGUY].plural1), commify(stat_monsterskilled_shotgunguys));

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_SHADOWS].plural1), commify(stat_monsterskilled_spectres));

    if (gamemode != shareware)
        C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>",
            sentencecase(mobjinfo[MT_SPIDER].plural1), commify(stat_monsterskilled_spidermasterminds));

    C_TabbedOutput(tabs, "   %s\t-\t<b>%s</b>", sentencecase(mobjinfo[MT_POSSESSED].plural1), commify(stat_monsterskilled_zombiemen));

    C_TabbedOutput(tabs, "Barrels exploded\t-\t<b>%s</b>", commify(stat_barrelsexploded));

    C_TabbedOutput(tabs, "Items picked up\t-\t<b>%s</b>", commify(stat_itemspickedup));

    C_TabbedOutput(tabs, "   Ammo\t-\t<b>%s bullet%s</b>",
        commify(stat_itemspickedup_ammo_bullets), (stat_itemspickedup_ammo_bullets == 1 ? "" : "s"));

    C_TabbedOutput(tabs, "\t-\t<b>%s cell%s</b>",
        commify(stat_itemspickedup_ammo_cells), (stat_itemspickedup_ammo_cells == 1 ? "" : "s"));

    C_TabbedOutput(tabs, "\t-\t<b>%s rocket%s</b>",
        commify(stat_itemspickedup_ammo_rockets), (stat_itemspickedup_ammo_rockets == 1 ? "" : "s"));

    C_TabbedOutput(tabs, "\t-\t<b>%s shell%s</b>",
        commify(stat_itemspickedup_ammo_shells), (stat_itemspickedup_ammo_shells == 1 ? "" : "s"));

    C_TabbedOutput(tabs, "   Armor\t-\t<b>%s%%</b>", commify(stat_itemspickedup_armor));

    C_TabbedOutput(tabs, "   Health\t-\t<b>%s%%</b>", commify(stat_itemspickedup_health));

    C_TabbedOutput(tabs, "Secrets revealed\t-\t<b>%s</b>", commify(stat_secretsrevealed));

    C_TabbedOutput(tabs, "Time played\t-\t<b>%02i:%02i:%02i</b>", time2 / 3600, (time2 % 3600) / 60, (time2 % 3600) % 60);

    C_TabbedOutput(tabs, "Damage inflicted\t-\t<b>%s%%</b>", commify(stat_damageinflicted));

    C_TabbedOutput(tabs, "Damage received\t-\t<b>%s%%</b>", commify(stat_damagereceived));

    C_TabbedOutput(tabs, "Deaths\t-\t<b>%s</b>", commify(stat_deaths));

    C_TabbedOutput(tabs, "Cheated\t-\t<b>%s</b>", commify(stat_cheated));

    C_TabbedOutput(tabs, "Shots fired\t-\t<b>%s</b>", commify(stat_shotsfired));

    C_TabbedOutput(tabs, "Shots hit\t-\t<b>%s</b>", commify(stat_shotshit));

    C_TabbedOutput(tabs, "Weapon accuracy\t-\t<b>%s%%</b>",
        (stat_shotsfired ? striptrailingzero(stat_shotshit * 100.0f / stat_shotsfired, 1) : "0"));

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
    C_PlayerMessage(parms);

    if (gamestate == GS_LEVEL && !message_dontfuckwithme)
        HU_SetPlayerMessage(parms, false, false);
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
        C_ShowDescription(C_GetIndex("reset"));
        C_Output("<b>%s</b> %s", cmd, RESETCMDFORMAT);
        return;
    }

    if (M_StringCompare(parms, "all"))
    {
        resetall_cmd_func2("resetall", "");
        return;
    }

    if (M_StringCompare(parms, "ammo") || M_StringCompare(parms, "armor") || M_StringCompare(parms, "armour")
        || M_StringCompare(parms, "armortype") || M_StringCompare(parms, "armourtype") || M_StringCompare(parms, "health"))
        return;

    resettingcvar = true;

    for (int i = 0; *consolecmds[i].name; i++)
    {
        const int   flags = consolecmds[i].flags;

        if (consolecmds[i].type == CT_CVAR && M_StringCompare(parms, consolecmds[i].name) && !(flags & CF_READONLY))
        {
            if (flags & (CF_BOOLEAN | CF_INTEGER))
                C_ValidateInput(M_StringJoin(parms, " ",
                    uncommify(C_LookupAliasFromValue((int)consolecmds[i].defaultnumber, consolecmds[i].aliases)), NULL));
            else if (flags & CF_FLOAT)
                C_ValidateInput(M_StringJoin(parms, " ", striptrailingzero(consolecmds[i].defaultnumber, 1), NULL));
            else
                C_ValidateInput(M_StringJoin(parms, " ",
                    (*consolecmds[i].defaultstring ? consolecmds[i].defaultstring : EMPTYVALUE), NULL));

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
                    consolecmds[i].func2(consolecmds[i].name, uncommify(C_LookupAliasFromValue((int)consolecmds[i].defaultnumber,
                        consolecmds[i].aliases)));
                else if (flags & CF_FLOAT)
                    consolecmds[i].func2(consolecmds[i].name, striptrailingzero(consolecmds[i].defaultnumber, 2));
                else
                    consolecmds[i].func2(consolecmds[i].name, (*consolecmds[i].defaultstring ?
                        consolecmds[i].defaultstring : EMPTYVALUE));
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

        C_Output("All CVARs have been reset to their defaults.");
    }
}

static void resetall_cmd_func2(char *cmd, char *parms)
{
    static char buffer[128];

    M_snprintf(buffer, sizeof(buffer), "Are you sure you want to reset\nall CVARs to their defaults?\n\n%s", s_PRESSYN);
    M_StartMessage(buffer, C_VerifyResetAll, true);
    SDL_StopTextInput();
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
static dboolean resurrect_cmd_func1(char *cmd, char *parms)
{
    return (gamestate == GS_LEVEL && viewplayer->health <= 0);
}

static void resurrect_cmd_func2(char *cmd, char *parms)
{
    P_ResurrectPlayer(initial_health);
    viewplayer->cheated++;
    stat_cheated = SafeAdd(stat_cheated, 1);
    M_SaveCVARs();
}

//
// save CCMD
//
static void save_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex("save"));
        C_Output("<b>%s</b> %s", cmd, SAVECMDFORMAT);
        return;
    }

    G_SaveGame(-1, "", M_StringJoin((M_StringStartsWith(parms, savegamefolder) ? "" : savegamefolder), parms,
        (M_StringEndsWith(parms, ".save") ? "" : ".save"), NULL));
}

//
// spawn CCMD
//
static int      spawncmdtype = NUMMOBJTYPES;
static dboolean spawncmdfriendly;

static dboolean spawn_cmd_func1(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
        return true;

    if ((spawncmdfriendly = M_StringStartsWith(parm, "friendly")))
        strreplace(parm, "friendly", "");

    if (gamestate == GS_LEVEL)
    {
        int num = -1;

        sscanf(parm, "%10d", &num);

        for (int i = 0; i < NUMMOBJTYPES; i++)
        {
            spawncmdtype = mobjinfo[i].doomednum;

            if (spawncmdtype >= 0 && (M_StringCompare(parm, removenonalpha(mobjinfo[i].name1))
                || (*mobjinfo[i].name2 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name2)))
                || (*mobjinfo[i].name3 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name3)))
                || (num == spawncmdtype && num != -1)))
                return true;
        }
    }

    return false;
}

static void spawn_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex("spawn"));
        C_Output("<b>%s</b> %s", cmd, SPAWNCMDFORMAT);
        return;
    }
    else
    {
        dboolean    spawn = true;
        char        buffer[128];

        if (gamemode != commercial)
        {
            if (spawncmdtype >= ArchVile && spawncmdtype <= MonstersSpawner)
            {
                M_StringCopy(buffer, mobjinfo[P_FindDoomedNum(spawncmdtype)].plural1, sizeof(buffer));

                if (!*buffer)
                    M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[P_FindDoomedNum(spawncmdtype)].name1);

                buffer[0] = toupper(buffer[0]);
                C_Warning("%s can't be spawned in <i><b>%s</b></i>.", buffer, gamedescription);
                spawn = false;
            }

            if (gamemode == shareware && (spawncmdtype == Cyberdemon || spawncmdtype == SpiderMastermind))
            {
                M_StringCopy(buffer, mobjinfo[P_FindDoomedNum(spawncmdtype)].plural1, sizeof(buffer));

                if (!*buffer)
                    M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[P_FindDoomedNum(spawncmdtype)].name1);

                buffer[0] = toupper(buffer[0]);
                C_Warning("%s can't be spawned in <i><b>%s</b></i>.", buffer, gamedescription);
                spawn = false;
            }
        }
        else if (spawncmdtype == WolfensteinSS && bfgedition && !states[S_SSWV_STND].dehacked)
            spawncmdtype = Zombieman;

        if (spawn)
        {
            mapthing_t  mthing;
            mobj_t      *thing;
            fixed_t     x = viewx + 100 * viewcos;
            fixed_t     y = viewy + 100 * viewsin;

            if (P_CheckLineSide(viewplayer->mo, x, y))
            {
                C_Warning("%s %s too close to the wall.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "are" : "is"));
                return;
            }

            mthing.x = x >> FRACBITS;
            mthing.y = y >> FRACBITS;
            mthing.angle = 0;
            mthing.type = spawncmdtype;
            mthing.options = (MTF_EASY | MTF_NORMAL | MTF_HARD);

            if ((thing = P_SpawnMapThing(&mthing, true)))
            {
                thing->angle = R_PointToAngle2(thing->x, thing->y, viewx, viewy);

                if (thing->flags & MF_COUNTITEM)
                {
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    M_SaveCVARs();
                }
                else if (spawncmdfriendly && (thing->flags & MF_SHOOTABLE))
                {
                    thing->flags |= MF_FRIEND;
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    M_SaveCVARs();
                }

                C_HideConsole();
            }
        }
    }
}

//
// take CCMD
//
static dboolean take_cmd_func1(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);
    int     num = -1;

    if (gamestate != GS_LEVEL)
        return false;

    if (!*parm)
        return true;

    if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything")
        || M_StringCompare(parm, "health") || M_StringCompare(parm, "allhealth")
        || M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons")
        || M_StringCompare(parm, "ammo") || M_StringCompare(parm, "allammo")
        || M_StringCompare(parm, "armor") || M_StringCompare(parm, "allarmor")
        || M_StringCompare(parm, "armour") || M_StringCompare(parm, "allarmour")
        || M_StringCompare(parm, "keys") || M_StringCompare(parm, "allkeys")
        || M_StringCompare(parm, "keycards") || M_StringCompare(parm, "allkeycards")
        || M_StringCompare(parm, "skullkeys") || M_StringCompare(parm, "allskullkeys")
        || M_StringCompare(parm, "pistol"))
        return true;

    sscanf(parm, "%10d", &num);

    for (int i = 0; i < NUMMOBJTYPES; i++)
        if ((mobjinfo[i].flags & MF_SPECIAL) && (M_StringCompare(parm, removenonalpha(mobjinfo[i].name1))
            || (*mobjinfo[i].name2 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name2)))
            || (*mobjinfo[i].name3 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name3)))
            || (num == mobjinfo[i].doomednum && num != -1)))
            return true;

    return false;
}

static void take_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        C_ShowDescription(C_GetIndex("take"));
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

            viewplayer->pendingweapon = wp_fist;

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
            else
                C_Warning("%s %s have anything.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"));
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
            else
                C_Warning("%s %s already dead.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "are" : "is"));
        }
        else if (M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons"))
        {
            for (weapontype_t i = wp_shotgun; i < NUMWEAPONS; i++)
                if (viewplayer->weaponowned[i])
                {
                    viewplayer->weaponowned[i] = oldweaponsowned[i] = false;
                    result = true;
                }

            viewplayer->pendingweapon = wp_fist;

            if (result)
                C_HideConsole();
            else
                C_Warning("%s %s have any weapons.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"));
        }
        else if (M_StringCompare(parm, "ammo") || M_StringCompare(parm, "allammo"))
        {
            for (ammotype_t i = 0; i < NUMAMMO; i++)
                if (viewplayer->ammo[i])
                {
                    viewplayer->ammo[i] = 0;
                    result = true;
                }

            viewplayer->pendingweapon = wp_fist;

            if (result)
                C_HideConsole();
            else
                C_Warning("%s %s have any ammo.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"));
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
            else
                C_Warning("%s %s have any armor.", titlecase(playername),
                (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"));
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
            else
                C_Warning("%s %s have any keycards or skull keys.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"));
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
            else
                C_Warning("%s %s have any keycards.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"));
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
            else
                C_Warning("%s %s have any skull keys.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"));
        }
        else if (M_StringCompare(parm, "pistol"))
        {
            if (!viewplayer->weaponowned[wp_pistol])
            {
                C_Warning("%s %s have a pistol.",
                    titlecase(playername), (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"));
                return;
            }

            viewplayer->weaponowned[wp_pistol] = false;
            oldweaponsowned[wp_pistol] = false;

            if (viewplayer->readyweapon == wp_pistol)
                C_HideConsole();

            P_CheckAmmo(viewplayer->readyweapon);
            return;
        }
        else
        {
            int num = -1;

            sscanf(parm, "%10d", &num);

            for (int i = 0; i < NUMMOBJTYPES; i++)
                if ((mobjinfo[i].flags & MF_SPECIAL)
                    && (M_StringCompare(parm, removenonalpha(mobjinfo[i].name1))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name2)))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, removenonalpha(mobjinfo[i].name3)))
                        || (num == mobjinfo[i].doomednum && num != -1)))
                {
                    if (!P_TakeSpecialThing(i))
                    {
                        C_Warning("%s %s have a %s.",
                            titlecase(playername),
                            (M_StringCompare(playername, playername_default) ? "don't" : "doesn't"),
                            mobjinfo[i].name1);
                        return;
                    }

                    break;
                }
        }
    }
}

//
// teleport CCMD
//
static void teleport_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex("teleport"));
        C_Output("<b>%s</b> %s", cmd, TELEPORTCMDFORMAT);
        return;
    }
    else
    {
        int x = INT_MAX;
        int y = INT_MAX;

        sscanf(parms, "%10d %10d", &x, &y);

        if (x != INT_MAX && y != INT_MAX)
        {
            mobj_t          *mo = viewplayer->mo;
            const fixed_t   oldx = viewx;
            const fixed_t   oldy = viewy;
            const fixed_t   oldz = viewz;

            x <<= FRACBITS;
            y <<= FRACBITS;

            if (P_TeleportMove(mo, x, y, ONFLOORZ, false))
            {
                // spawn teleport fog at source
                mobj_t  *fog = P_SpawnMobj(oldx, oldy, oldz, MT_TFOG);

                fog->angle = viewangle;
                S_StartSound(fog, sfx_telept);
                C_HideConsole();

                // spawn teleport fog at destination
                mo->z = mo->floorz;
                viewplayer->viewz = mo->z + viewplayer->viewheight;
                fog = P_SpawnMobj(x + 20 * viewcos, y + 20 * viewsin, ONFLOORZ, MT_TFOG);
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
    const int   tabs[8] = { 50, 268, 0, 0, 0, 0, 0, 0 };

    C_Header(thinglistheader);

    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        mobj_t  *mobj = (mobj_t *)th;
        char    name[100];

        if (*mobj->name)
            M_StringCopy(name, mobj->name, sizeof(name));
        else
            M_snprintf(name, sizeof(name), "%s%s", ((mobj->flags & MF_CORPSE) && !(mobj->flags2 & MF2_DECORATION) ? "dead " :
                ((mobj->flags & MF_FRIEND) && mobj->type != MT_PLAYER ? "friendly " : "")),
                (mobj->type == MT_PLAYER && mobj != viewplayer->mo ? "voodoo doll" : mobj->info->name1));

        C_TabbedOutput(tabs, "%s%s\t%s\t(%i, %i, %i)", (mobj->id >= 0 ? commify(mobj->id) : ""), (mobj->id >= 0 ? "." : ""),
            sentencecase(name), mobj->x >> FRACBITS, mobj->y >> FRACBITS, mobj->z >> FRACBITS);
    }
}

//
// timer CCMD
//
static void timer_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex("timer"));
        C_Output("<b>%s</b> %s", cmd, TIMERCMDFORMAT);
        return;
    }
    else
    {
        int value = INT_MAX;

        sscanf(parms, "%10d", &value);

        if (value != INT_MAX)
            P_SetTimer(value);
    }
}

//
// unbind CCMD
//
static void unbind_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        C_ShowDescription(C_GetIndex("unbind"));
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
                        bind_cmd_func2("bind", M_StringJoin(control, " ", sc_String, NULL));

                    free(control);
                }
            }
            else if (SC_GetString())
                C_ValidateInput(M_StringJoin(cvar, " ", sc_String, NULL));

            free(cvar);
        }

        SC_Close();

        buddha = viewplayer->cheats & CF_BUDDHA;
        viewplayer->cheats &= ~CF_BUDDHA;

        C_Output(s_STSTR_VON);
        HU_SetPlayerMessage(s_STSTR_VON, false, false);
        C_Warning("Any changes to CVARs won't be saved while vanilla mode is on.");
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
                C_ShowDescription(i);

                if (*(dboolean *)consolecmds[i].variable == (dboolean)consolecmds[i].defaultnumber)
                    C_Output(INTEGERCVARISDEFAULT,
                        C_LookupAliasFromValue(*(dboolean *)consolecmds[i].variable, BOOLVALUEALIAS));
                else
                    C_Output(INTEGERCVARWITHDEFAULT,
                        C_LookupAliasFromValue(*(dboolean *)consolecmds[i].variable, BOOLVALUEALIAS),
                        C_LookupAliasFromValue((dboolean)consolecmds[i].defaultnumber, BOOLVALUEALIAS));
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
    for (int i = 0; *color[i].name; i++)
        if (M_StringCompare(parms, color[i].name))
            return true;

    return ((strlen(parms) == 7 && parms[0] == '#' && hextodec(M_SubString(parms, 1, 6)) >= 0) || int_cvars_func1(cmd, parms));
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
        M_snprintf(buffer, sizeof(buffer), "%i", FindNearestColor(PLAYPAL,
            hextodec(M_SubString(parms, 1, 2)), hextodec(M_SubString(parms, 3, 2)), hextodec(M_SubString(parms, 5, 2))));
        int_cvars_func2(cmd, buffer);
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
                    if (consolecmds[i].flags & CF_READONLY)
                        C_Output(PERCENTCVARISREADONLY, commify(*(int *)consolecmds[i].variable));
                    else if (*(int *)consolecmds[i].variable == (int)consolecmds[i].defaultnumber)
                        C_Output(PERCENTCVARISDEFAULT, commify(*(int *)consolecmds[i].variable));
                    else
                        C_Output(PERCENTCVARWITHDEFAULT,
                            commify(*(int *)consolecmds[i].variable), commify((int)consolecmds[i].defaultnumber));
                }
                else
                {
                    if (consolecmds[i].flags & CF_READONLY)
                        C_Output(INTEGERCVARISREADONLY,
                            C_LookupAliasFromValue(*(int *)consolecmds[i].variable, consolecmds[i].aliases));
                    else if (*(int *)consolecmds[i].variable == (int)consolecmds[i].defaultnumber)
                        C_Output(INTEGERCVARISDEFAULT,
                            C_LookupAliasFromValue(*(int *)consolecmds[i].variable, consolecmds[i].aliases));
                    else
                        C_Output(INTEGERCVARWITHDEFAULT,
                            C_LookupAliasFromValue(*(int *)consolecmds[i].variable, consolecmds[i].aliases),
                            C_LookupAliasFromValue((int)consolecmds[i].defaultnumber, consolecmds[i].aliases));
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
        C_ShowDescription(C_GetIndex(stringize(am_gridsize)));

        if (M_StringCompare(am_gridsize, am_gridsize_default))
            C_Output(INTEGERCVARISDEFAULT, am_gridsize);
        else
            C_Output(INTEGERCVARWITHDEFAULT, am_gridsize, am_gridsize_default);
    }
}

//
// am_path CVAR
//
static void am_path_cvar_func2(char *cmd, char *parms)
{
    const dboolean  am_path_old = am_path;

    bool_cvars_func2(cmd, parms);

    if (!am_path && am_path_old)
        pathpointnum = 0;
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
        C_ShowDescription(C_GetIndex(stringize(armortype)));

        if (gamestate == GS_LEVEL)
            C_Output(INTEGERCVARWITHNODEFAULT, C_LookupAliasFromValue(viewplayer->armortype, ARMORTYPEVALUEALIAS));
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
        C_ShowDescription(C_GetIndex(stringize(crosshair)));

        if (crosshair == crosshair_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(crosshair, CROSSHAIRVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(crosshair, CROSSHAIRVALUEALIAS), C_LookupAliasFromValue(crosshair_default, CROSSHAIRVALUEALIAS));
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
        ExpDef.lastOn = (nerve ? expansion : 1) - 1;
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
        C_ShowDescription(C_GetIndex(stringize(gp_deadzone_left)));

        if (gp_deadzone_left == gp_deadzone_left_default)
            C_Output(PERCENTCVARISDEFAULT, striptrailingzero(gp_deadzone_left, 1));
        else
            C_Output(PERCENTCVARWITHDEFAULT,
                striptrailingzero(gp_deadzone_left, 1), striptrailingzero(gp_deadzone_left_default, 1));
    }
    else
    {
        C_ShowDescription(C_GetIndex(stringize(gp_deadzone_right)));

        if (gp_deadzone_right == gp_deadzone_right_default)
            C_Output(PERCENTCVARISDEFAULT, striptrailingzero(gp_deadzone_right, 1));
        else
            C_Output(PERCENTCVARWITHDEFAULT,
                striptrailingzero(gp_deadzone_right, 1), striptrailingzero(gp_deadzone_right_default, 1));
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
dboolean P_CheckAmmo(weapontype_t weapon);

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
            C_ShowDescription(C_GetIndex(stringize(ammo)));

            if (gamestate == GS_LEVEL)
                C_Output(INTEGERCVARWITHNODEFAULT, commify(ammotype == am_noammo ? 0 : viewplayer->ammo[ammotype]));
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
            C_ShowDescription(C_GetIndex(stringize(armor)));

            if (gamestate == GS_LEVEL)
                C_Output(PERCENTCVARWITHNODEFAULT, commify(viewplayer->armorpoints));
        }
    }
    else if (M_StringCompare(cmd, stringize(health)) && !(viewplayer->cheats & CF_GODMODE) && !viewplayer->powers[pw_invulnerability])
    {
        if (*parms)
        {
            sscanf(parms, "%10d", &value);
            value = BETWEEN(health_min, value, maxhealth);

            if (viewplayer->health <= 0)
            {
                if (value <= 0)
                {
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
                    P_DamageMobj(viewplayer->mo, viewplayer->mo, NULL, viewplayer->health - value, false);
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
        else
        {
            C_ShowDescription(C_GetIndex(stringize(health)));

            if (gamestate == GS_LEVEL)
                C_Output(PERCENTCVARWITHNODEFAULT, commify(viewplayer->health));
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
        C_ShowDescription(C_GetIndex(stringize(r_blood)));

        if (r_blood == r_blood_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(r_blood, BLOODVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_blood, BLOODVALUEALIAS), C_LookupAliasFromValue(r_blood_default, BLOODVALUEALIAS));
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
        C_ShowDescription(C_GetIndex(stringize(r_bloodsplats_translucency)));

        if (r_bloodsplats_translucency == r_bloodsplats_translucency_default)
            C_Output(INTEGERCVARISDEFAULT,
                C_LookupAliasFromValue(r_bloodsplats_translucency, BOOLVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_bloodsplats_translucency, BOOLVALUEALIAS),
                C_LookupAliasFromValue(r_bloodsplats_translucency_default, BOOLVALUEALIAS));
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
            M_SaveCVARs();
        }
    }
    else
    {
        C_ShowDescription(C_GetIndex(stringize(r_detail)));

        if (r_detail == r_detail_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(r_detail, DETAILVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_detail, DETAILVALUEALIAS), C_LookupAliasFromValue(r_detail_default, DETAILVALUEALIAS));
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
        C_ShowDescription(C_GetIndex(stringize(r_dither)));

        if (r_dither == r_dither_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(r_dither, BOOLVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_dither, BOOLVALUEALIAS), C_LookupAliasFromValue(r_dither_default, BOOLVALUEALIAS));
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
                C_Warning(PENDINGCHANGE);
        }
    }
    else
    {
        C_ShowDescription(C_GetIndex(stringize(r_fixmaperrors)));

        if (r_fixmaperrors == r_fixmaperrors_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(r_fixmaperrors, BOOLVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_fixmaperrors, BOOLVALUEALIAS), C_LookupAliasFromValue(r_fixmaperrors_default, BOOLVALUEALIAS));
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

        C_ShowDescription(C_GetIndex(stringize(r_gamma)));

        if (r_gamma == r_gamma_default)
            C_Output(INTEGERCVARISDEFAULT, (r_gamma == 1.0f ? "off" : buffer1));
        else
        {
            char    buffer2[128];

            M_snprintf(buffer2, sizeof(buffer2), "%.2f", r_gamma_default);
            len = (int)strlen(buffer2);

            if (len >= 2 && buffer2[len - 1] == '0' && buffer2[len - 2] == '0')
                buffer2[len - 1] = '\0';

            C_Output(INTEGERCVARWITHDEFAULT,
                (r_gamma == 1.0f ? "off" : buffer1), (r_gamma_default == 1.0f ? "off" : buffer2));
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
        C_ShowDescription(C_GetIndex(stringize(r_hud_translucency)));

        if (r_hud_translucency == r_hud_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(r_hud_translucency, BOOLVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_hud_translucency, BOOLVALUEALIAS),
                C_LookupAliasFromValue(r_hud_translucency_default, BOOLVALUEALIAS));
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
        C_ShowDescription(C_GetIndex(stringize(r_lowpixelsize)));

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
        }
    }
    else
    {
        C_ShowDescription(C_GetIndex(stringize(r_screensize)));

        if (r_screensize == r_screensize_default)
            C_Output(INTEGERCVARISDEFAULT, commify(r_screensize));
        else
            C_Output(INTEGERCVARWITHDEFAULT, commify(r_screensize), commify(r_screensize_default));
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
        C_ShowDescription(C_GetIndex(stringize(r_shadows_translucency)));

        if (r_shadows_translucency == r_shadows_translucency_default)
            C_Output(INTEGERCVARISDEFAULT,
                C_LookupAliasFromValue(r_shadows_translucency, BOOLVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_shadows_translucency, BOOLVALUEALIAS),
                C_LookupAliasFromValue(r_shadows_translucency_default, BOOLVALUEALIAS));
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
        C_ShowDescription(C_GetIndex(stringize(r_textures)));

        if (r_textures == r_textures_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(r_textures, BOOLVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_textures, BOOLVALUEALIAS), C_LookupAliasFromValue(r_textures_default, BOOLVALUEALIAS));
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
        C_ShowDescription(C_GetIndex(stringize(r_translucency)));

        if (r_translucency == r_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(r_translucency, BOOLVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(r_translucency, BOOLVALUEALIAS), C_LookupAliasFromValue(r_translucency_default, BOOLVALUEALIAS));
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
            musicVolume = (BETWEEN(s_musicvolume_min, s_musicvolume, s_musicvolume_max) * 31 + 50) / 100;
            S_SetMusicVolume(musicVolume * MAX_MUSIC_VOLUME / 31);
            M_SaveCVARs();
        }
        else if (s_sfxvolume != value)
        {
            s_sfxvolume = value;
            sfxVolume = (BETWEEN(s_sfxvolume_min, s_sfxvolume, s_sfxvolume_max) * 31 + 50) / 100;
            S_SetSfxVolume(sfxVolume * MAX_SFX_VOLUME / 31);
            M_SaveCVARs();
        }
    }
    else if (M_StringCompare(cmd, stringize(s_musicvolume)))
    {
        C_ShowDescription(C_GetIndex(stringize(s_musicvolume)));

        if (s_musicvolume == s_musicvolume_default)
            C_Output(PERCENTCVARISDEFAULT, commify(s_musicvolume));
        else
            C_Output(PERCENTCVARWITHDEFAULT, commify(s_musicvolume), commify(s_musicvolume_default));
    }
    else
    {
        C_ShowDescription(C_GetIndex(stringize(s_sfxvolume)));

        if (s_sfxvolume == s_sfxvolume_default)
            C_Output(PERCENTCVARISDEFAULT, commify(s_sfxvolume));
        else
            C_Output(PERCENTCVARWITHDEFAULT, commify(s_sfxvolume), commify(s_sfxvolume_default));
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
            C_Warning(PENDINGCHANGE);
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
        C_ShowDescription(C_GetIndex(stringize(turbo)));

        if (turbo == turbo_default)
            C_Output(PERCENTCVARISDEFAULT, commify(turbo));
        else
            C_Output(PERCENTCVARWITHDEFAULT, commify(turbo), commify(turbo_default));
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
        C_ShowDescription(C_GetIndex(stringize(units)));

        if (units == units_default)
            C_Output(INTEGERCVARISDEFAULT, C_LookupAliasFromValue(units, UNITSVALUEALIAS));
        else
            C_Output(INTEGERCVARWITHDEFAULT,
                C_LookupAliasFromValue(units, UNITSVALUEALIAS), C_LookupAliasFromValue(units_default, UNITSVALUEALIAS));
    }
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
            I_CapFPS(vid_capfps);
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
    return (!*parms || M_StringCompare(parms, vid_scaleapi_direct3d)
#if defined(__APPLE__)
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
        C_ShowDescription(C_GetIndex(stringize(vid_scaleapi)));

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
        C_ShowDescription(C_GetIndex(stringize(vid_scalefilter)));

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
        C_ShowDescription(C_GetIndex(stringize(vid_screenresolution)));

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
static void vid_vsync_cvar_func2(char *cmd, char *parms)
{
    const dboolean  vid_vsync_old = vid_vsync;

    bool_cvars_func2(cmd, parms);

    if (vid_vsync != vid_vsync_old)
        I_RestartGraphics();
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
    }
    else
    {
        C_ShowDescription(C_GetIndex(stringize(vid_windowpos)));

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
        C_ShowDescription(C_GetIndex(stringize(vid_windowsize)));

        if (M_StringCompare(vid_windowsize, vid_windowsize_default))
            C_Output(INTEGERCVARISDEFAULT, vid_windowsize);
        else
            C_Output(INTEGERCVARWITHDEFAULT, vid_windowsize, vid_windowsize_default);
    }
}
