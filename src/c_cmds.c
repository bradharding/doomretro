/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

  This file is a part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the license, or (at your
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
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_gamecontroller.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_tick.h"
#include "r_sky.h"
#include "s_sound.h"
#include "sc_man.h"
#include "st_lib.h"
#include "st_stuff.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"

#define ALIASCMDFORMAT              BOLDITALICS("alias") " [[" BOLD("\"") "]" BOLDITALICS("command") "[" BOLD(";") " " \
                                    BOLDITALICS("command") " ..." BOLD("\"") "]]"
#define BINDCMDFORMAT               BOLDITALICS("control") " [" BOLDITALICS("+action") "|[" BOLD("\"") "]" BOLDITALICS("command") "[" \
                                    BOLD(";") " " BOLDITALICS("command") " ..." BOLD("\"") "]]"
#define EXECCMDFORMAT               BOLDITALICS("filename") "[" BOLD(".cfg") "]"
#define EXPLODECMDFORMAT            BOLD("barrels") "|" BOLD("missiles")
#define GIVECMDFORMAT               BOLD("ammo") "|" BOLD("armor") "|" BOLD("health") "|" BOLD("keys") "|" BOLD("weapons") "|" \
                                    BOLD("all") "|" BOLDITALICS("item")
#define IFCMDFORMAT                 BOLDITALICS("CVAR") " " BOLDITALICS("value") " " BOLD("then") " [" BOLD("\"") "]" \
                                    BOLDITALICS("command") "[" BOLD(";") " " BOLDITALICS("command") " ..." BOLD("\"") "]"
#define KILLCMDFORMAT               BOLD("player") "|" BOLD("all") "|[" BOLD("friendly") " ]" BOLDITALICS("monster")
#define LOADCMDFORMAT               BOLDITALICS("filename") "[" BOLD(".save") "]"
#define MAPCMDFORMAT1               BOLD("E") BOLDITALICS("x") BOLD("M") BOLDITALICS("y") "[" BOLD("B") "]|" BOLDITALICS("title") "|" \
                                    BOLD("first") "|" BOLD("previous") "|" BOLD("next") "|" BOLD("last") "|" BOLD("random")
#define MAPCMDFORMAT2               BOLD("MAP") BOLDITALICS("xy") "|" BOLDITALICS("title") "|" BOLD("first") "|" BOLD("previous") "|" \
                                    BOLD("next") "|" BOLD("last") "|" BOLD("random")
#define PLAYCMDFORMAT               BOLDITALICS("soundeffect") "|" BOLDITALICS("music")
#define NAMECMDFORMAT               "[" BOLD("friendly") " ]" BOLDITALICS("monster") " " BOLDITALICS("name")
#define PRINTCMDFORMAT              "[" BOLD("\x93") "]" BOLDITALICS("message") "[" BOLD("\x94") "]"
#define REMOVECMDFORMAT             BOLD("decorations") "|" BOLD("corpses") "|" BOLD("bloodsplats") "|" BOLD("items") "|" \
                                    BOLDITALICS("item") "|" BOLD("everything")
#define RESETCMDFORMAT              BOLDITALICS("CVAR")
#define RESURRECTCMDFORMAT          BOLD("player") "|" BOLD("all") "|[" BOLD("friendly") " ]" BOLDITALICS("monster")
#define SAVECMDFORMAT               BOLDITALICS("filename") "[" BOLD(".save") "]"
#define SPAWNCMDFORMAT              BOLDITALICS("item") "|[" BOLD("friendly") " ]" BOLDITALICS("monster")
#define TAKECMDFORMAT               BOLD("ammo") "|" BOLD("armor") "|" BOLD("health") "|" BOLD("keys") "|" BOLD("weapons") "|" \
                                    BOLD("all") "|" BOLDITALICS("item")
#define TELEPORTCMDFORMAT           BOLDITALICS("x") " " BOLDITALICS("y") "[ " BOLDITALICS("z") "]"
#define TIMERCMDFORMAT              BOLDITALICS("minutes")
#define TOGGLECMDFORMAT             BOLDITALICS("CVAR")
#define UNBINDCMDFORMAT             BOLDITALICS("control") "|" BOLDITALICS("+action")

#define DEADPLAYERWARNING           "It won't work if %s %s dead."
#define NEXTMAPWARNING              "It won't work until the next map."
#define NOGAMEWARNING               "It won't work if %s %s not playing a game."
#define NIGHTMAREWARNING            "It won't work if %s %s playing a game in " ITALICS("Nightmare!")

#define INTEGERCVARWITHDEFAULT      "It is currently " BOLD("%s") " and is " BOLD("%s") " by default."
#define INTEGERCVARWITHNODEFAULT    "It is currently " BOLD("%s") "."
#define INTEGERCVARISDEFAULT        "It is currently its default of " BOLD("%s") "."
#define DEGREESCVARWITHDEFAULT      "It is currently " BOLD("%i") "\xB0 and is " BOLD("%i") "\xB0 by default."
#define DEGREESCVARISDEFAULT        "It is currently its default of " BOLD("%i") "\xB0."
#define PERCENTCVARWITHDEFAULT      "It is currently " BOLD("%s%%") " and is " BOLD("%s%%") " by default."
#define PERCENTCVARWITHNODEFAULT    "It is currently " BOLD("%s%%") "."
#define PERCENTCVARISDEFAULT        "It is currently its default of " BOLD("%s%%") "."
#define STRINGCVARWITHDEFAULT       "It is currently " BOLD("\"%s\"") " and is " BOLD("\"%s\"") " by default."
#define STRINGCVARWITHNODEFAULT     "It is currently " BOLD("%s%s%s") "."
#define STRINGCVARISDEFAULT         "It is currently its default of " BOLD("\"%s\"") "."
#define TIMECVARWITHNODEFAULT1      "It is currently " BOLD("%02i:%02i") "."
#define TIMECVARWITHNODEFAULT2      "It is currently " BOLD("%i:%02i:%02i") "."

#define INDENT                      "      "

alias_t     aliases[MAXALIASES];

static int  ammo;
static int  armor;
static int  armortype;
static int  health;
static int  weapon;

static int  mapcmdepisode;
static int  mapcmdmap;
static char mapcmdlump[7];

bool        executingalias = false;
bool        healthcvar = false;
bool        nobindoutput;
bool        quitcmd = false;
bool        resettingcvar = false;
bool        togglingvanilla = false;
bool        vanilla = false;

const control_t controls[] =
{
    { "1",             keyboardcontrol,       '1'                           },
    { "2",             keyboardcontrol,       '2'                           },
    { "3",             keyboardcontrol,       '3'                           },
    { "4",             keyboardcontrol,       '4'                           },
    { "5",             keyboardcontrol,       '5'                           },
    { "6",             keyboardcontrol,       '6'                           },
    { "7",             keyboardcontrol,       '7'                           },
    { "8",             keyboardcontrol,       '8'                           },
    { "9",             keyboardcontrol,       '9'                           },
    { "0",             keyboardcontrol,       '0'                           },
    { "-",             keyboardcontrol,       KEY_MINUS                     },
    { "=",             keyboardcontrol,       KEY_EQUALS                    },
    { "+",             keyboardcontrol,       KEY_EQUALS                    },
    { "backspace",     keyboardcontrol,       KEY_BACKSPACE                 },
    { "tab",           keyboardcontrol,       KEY_TAB                       },
    { "q",             keyboardcontrol,       'q'                           },
    { "w",             keyboardcontrol,       'w'                           },
    { "e",             keyboardcontrol,       'e'                           },
    { "r",             keyboardcontrol,       'r'                           },
    { "t",             keyboardcontrol,       't'                           },
    { "y",             keyboardcontrol,       'y'                           },
    { "u",             keyboardcontrol,       'u'                           },
    { "i",             keyboardcontrol,       'i'                           },
    { "o",             keyboardcontrol,       'o'                           },
    { "p",             keyboardcontrol,       'p'                           },
    { "[",             keyboardcontrol,       '['                           },
    { "]",             keyboardcontrol,       ']'                           },
    { "enter",         keyboardcontrol,       KEY_ENTER                     },
    { "ctrl",          keyboardcontrol,       KEY_CTRL                      },
    { "a",             keyboardcontrol,       'a'                           },
    { "s",             keyboardcontrol,       's'                           },
    { "d",             keyboardcontrol,       'd'                           },
    { "f",             keyboardcontrol,       'f'                           },
    { "g",             keyboardcontrol,       'g'                           },
    { "h",             keyboardcontrol,       'h'                           },
    { "j",             keyboardcontrol,       'j'                           },
    { "k",             keyboardcontrol,       'k'                           },
    { "l",             keyboardcontrol,       'l'                           },
    { ";",             keyboardcontrol,       ';'                           },
    { "'",             keyboardcontrol,       '\''                          },
    { "shift",         keyboardcontrol,       KEY_SHIFT                     },
    { "\\",            keyboardcontrol,       '\\'                          },
    { "z",             keyboardcontrol,       'z'                           },
    { "x",             keyboardcontrol,       'x'                           },
    { "c",             keyboardcontrol,       'c'                           },
    { "v",             keyboardcontrol,       'v'                           },
    { "b",             keyboardcontrol,       'b'                           },
    { "n",             keyboardcontrol,       'n'                           },
    { "m",             keyboardcontrol,       'm'                           },
    { ",",             keyboardcontrol,       ','                           },
    { ".",             keyboardcontrol,       '.'                           },
    { "/",             keyboardcontrol,       '/'                           },
    { "tilde",         keyboardcontrol,       '`'                           },
    { "alt",           keyboardcontrol,       KEY_ALT                       },
    { "space",         keyboardcontrol,       ' '                           },
    { "numlock",       keyboardcontrol,       KEY_NUMLOCK                   },
    { "capslock",      keyboardcontrol,       KEY_CAPSLOCK                  },
    { "scrolllock",    keyboardcontrol,       KEY_SCROLLLOCK                },
    { "home",          keyboardcontrol,       KEY_HOME                      },
    { "up",            keyboardcontrol,       KEY_UPARROW                   },
    { "pageup",        keyboardcontrol,       KEY_PAGEUP                    },
    { "left",          keyboardcontrol,       KEY_LEFTARROW                 },
    { "right",         keyboardcontrol,       KEY_RIGHTARROW                },
    { "end",           keyboardcontrol,       KEY_END                       },
    { "down",          keyboardcontrol,       KEY_DOWNARROW                 },
    { "pagedown",      keyboardcontrol,       KEY_PAGEDOWN                  },
    { "insert",        keyboardcontrol,       KEY_INSERT                    },
    { "printscreen",   keyboardcontrol,       KEY_PRINTSCREEN               },
    { "delete",        keyboardcontrol,       KEY_DELETE                    },
    { "escape",        keyboardcontrol,       KEY_ESCAPE                    },
    { "F12",           keyboardcontrol,       KEY_F12                       },

    { "mouse1",        mousecontrol,          0                             },
    { "mouse2",        mousecontrol,          1                             },
    { "mouse3",        mousecontrol,          2                             },
    { "mouse4",        mousecontrol,          3                             },
    { "mouse5",        mousecontrol,          4                             },
    { "mouse6",        mousecontrol,          5                             },
    { "mouse7",        mousecontrol,          6                             },
    { "mouse8",        mousecontrol,          7                             },
    { "wheelup",       mousecontrol,          MOUSE_WHEELUP                 },
    { "wheeldown",     mousecontrol,          MOUSE_WHEELDOWN               },

    { "button1",       gamecontrollercontrol, GAMECONTROLLER_A              },
    { "gamepad1",      gamecontrollercontrol, GAMECONTROLLER_A              },
    { "button2",       gamecontrollercontrol, GAMECONTROLLER_B              },
    { "gamepad2",      gamecontrollercontrol, GAMECONTROLLER_B              },
    { "button3",       gamecontrollercontrol, GAMECONTROLLER_X              },
    { "gamepad3",      gamecontrollercontrol, GAMECONTROLLER_X              },
    { "button4",       gamecontrollercontrol, GAMECONTROLLER_Y              },
    { "gamepad4",      gamecontrollercontrol, GAMECONTROLLER_Y              },
    { "back",          gamecontrollercontrol, GAMECONTROLLER_BACK           },
    { "guide",         gamecontrollercontrol, GAMECONTROLLER_GUIDE          },
    { "start",         gamecontrollercontrol, GAMECONTROLLER_START          },
    { "leftthumb",     gamecontrollercontrol, GAMECONTROLLER_LEFT_THUMB     },
    { "rightthumb",    gamecontrollercontrol, GAMECONTROLLER_RIGHT_THUMB    },
    { "leftshoulder",  gamecontrollercontrol, GAMECONTROLLER_LEFT_SHOULDER  },
    { "rightshoulder", gamecontrollercontrol, GAMECONTROLLER_RIGHT_SHOULDER },
    { "dpadup",        gamecontrollercontrol, GAMECONTROLLER_DPAD_UP        },
    { "dpaddown",      gamecontrollercontrol, GAMECONTROLLER_DPAD_DOWN      },
    { "dpadleft",      gamecontrollercontrol, GAMECONTROLLER_DPAD_LEFT      },
    { "dpadright",     gamecontrollercontrol, GAMECONTROLLER_DPAD_RIGHT     },
    { "misc1",         gamecontrollercontrol, GAMECONTROLLER_MISC1          },
    { "paddle1",       gamecontrollercontrol, GAMECONTROLLER_PADDLE1        },
    { "paddle2",       gamecontrollercontrol, GAMECONTROLLER_PADDLE2        },
    { "paddle3",       gamecontrollercontrol, GAMECONTROLLER_PADDLE3        },
    { "paddle4",       gamecontrollercontrol, GAMECONTROLLER_PADDLE4        },
    { "touchpad",      gamecontrollercontrol, GAMECONTROLLER_TOUCHPAD       },
    { "lefttrigger",   gamecontrollercontrol, GAMECONTROLLER_LEFT_TRIGGER   },
    { "righttrigger",  gamecontrollercontrol, GAMECONTROLLER_RIGHT_TRIGGER  },

    { "",              0,                     0                             }
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
    { "+alwaysrun",   true,  alwaysrun_action_func,   &keyboardalwaysrun,   NULL,                  NULL,              &gamecontrolleralwaysrun,   NULL                },
    { "+automap",     false, automap_action_func,     &keyboardautomap,     NULL,                  NULL,              &gamecontrollerautomap,     NULL                },
    { "+back",        true,  back_action_func,        &keyboardback,        &keyboardback2,        &mouseback,        &gamecontrollerback,        NULL                },
    { "+clearmark",   true,  clearmark_action_func,   &keyboardclearmark,   NULL,                  NULL,              &gamecontrollerclearmark,   NULL                },
    { "+console",     false, console_action_func,     &keyboardconsole,     NULL,                  NULL,              &gamecontrollerconsole,     NULL                },
    { "+fire",        true,  fire_action_func,        &keyboardfire,        NULL,                  &mousefire,        &gamecontrollerfire,        NULL                },
    { "+followmode",  true,  followmode_action_func,  &keyboardfollowmode,  NULL,                  NULL,              &gamecontrollerfollowmode,  NULL                },
    { "+forward",     true,  forward_action_func,     &keyboardforward,     &keyboardforward2,     &mouseforward,     &gamecontrollerforward,     NULL                },
    { "+grid",        true,  grid_action_func,        &keyboardgrid,        NULL,                  NULL,              &gamecontrollergrid,        NULL                },
    { "+jump",        true,  jump_action_func,        &keyboardjump,        NULL,                  &mousejump,        &gamecontrollerjump,        NULL                },
    { "+left",        true,  left_action_func,        &keyboardleft,        NULL,                  &mouseleft,        &gamecontrollerleft,        NULL                },
    { "+mark",        true,  mark_action_func,        &keyboardmark,        NULL,                  NULL,              &gamecontrollermark,        NULL                },
    { "+maxzoom",     true,  maxzoom_action_func,     &keyboardmaxzoom,     NULL,                  NULL,              &gamecontrollermaxzoom,     NULL                },
    { "+menu",        true,  menu_action_func,        &keyboardmenu,        NULL,                  NULL,              &gamecontrollermenu,        NULL                },
    { "+mouselook",   true,  NULL,                    &keyboardmouselook,   NULL,                  &mousemouselook,   &gamecontrollermouselook,   NULL                },
    { "+nextweapon",  true,  nextweapon_action_func,  &keyboardnextweapon,  NULL,                  &mousenextweapon,  &gamecontrollernextweapon,  NULL                },
    { "+prevweapon",  true,  prevweapon_action_func,  &keyboardprevweapon,  NULL,                  &mouseprevweapon,  &gamecontrollerprevweapon,  NULL                },
    { "+right",       true,  right_action_func,       &keyboardright,       NULL,                  &mouseright,       &gamecontrollerright,       NULL                },
    { "+rotatemode",  true,  rotatemode_action_func,  &keyboardrotatemode,  NULL,                  NULL,              &gamecontrollerrotatemode,  NULL                },
    { "+run",         true,  NULL,                    &keyboardrun,         NULL,                  &mouserun,         &gamecontrollerrun,         NULL                },
    { "+screenshot",  false, screenshot_action_func,  &keyboardscreenshot,  NULL,                  &mousescreenshot,  NULL,                       NULL                },
    { "+strafe",      true,  NULL,                    &keyboardstrafe,      NULL,                  &mousestrafe,      &gamecontrollerstrafe,      NULL                },
    { "+strafeleft",  true,  strafeleft_action_func,  &keyboardstrafeleft,  &keyboardstrafeleft2,  &mousestrafeleft,  &gamecontrollerstrafeleft,  NULL                },
    { "+straferight", true,  straferight_action_func, &keyboardstraferight, &keyboardstraferight2, &mousestraferight, &gamecontrollerstraferight, NULL                },
    { "+use",         true,  use_action_func,         &keyboarduse,         &keyboarduse2,         &mouseuse,         &gamecontrolleruse,         &gamecontrolleruse2 },
    { "+weapon1",     true,  weapon1_action_func,     &keyboardweapon1,     NULL,                  &mouseweapon1,     &gamecontrollerweapon1,     NULL                },
    { "+weapon2",     true,  weapon2_action_func,     &keyboardweapon2,     NULL,                  &mouseweapon2,     &gamecontrollerweapon2,     NULL                },
    { "+weapon3",     true,  weapon3_action_func,     &keyboardweapon3,     NULL,                  &mouseweapon3,     &gamecontrollerweapon3,     NULL                },
    { "+weapon4",     true,  weapon4_action_func,     &keyboardweapon4,     NULL,                  &mouseweapon4,     &gamecontrollerweapon4,     NULL                },
    { "+weapon5",     true,  weapon5_action_func,     &keyboardweapon5,     NULL,                  &mouseweapon5,     &gamecontrollerweapon5,     NULL                },
    { "+weapon6",     true,  weapon6_action_func,     &keyboardweapon6,     NULL,                  &mouseweapon6,     &gamecontrollerweapon6,     NULL                },
    { "+weapon7",     true,  weapon7_action_func,     &keyboardweapon7,     NULL,                  &mouseweapon7,     &gamecontrollerweapon7,     NULL                },
    { "+zoomin",      true,  NULL,                    &keyboardzoomin,      NULL,                  NULL,              &gamecontrollerzoomin,      NULL                },
    { "+zoomout",     true,  NULL,                    &keyboardzoomout,     NULL,                  NULL,              &gamecontrollerzoomout,     NULL                },
    { "",             false, NULL,                    NULL,                 NULL,                  NULL,              NULL,                       NULL                }
};

static bool alive_func1(char *cmd, char *parms);
static bool cheat_func1(char *cmd, char *parms);
static bool game_func1(char *cmd, char *parms);
static bool nightmare_func1(char *cmd, char *parms);
static bool null_func1(char *cmd, char *parms);

static void bindlist_cmd_func2(char *cmd, char *parms);
static void clear_cmd_func2(char *cmd, char *parms);
static void cmdlist_cmd_func2(char *cmd, char *parms);
static bool condump_cmd_func1(char *cmd, char *parms);
static void condump_cmd_func2(char *cmd, char *parms);
static void cvarlist_cmd_func2(char *cmd, char *parms);
static void endgame_cmd_func2(char *cmd, char *parms);
static void exec_cmd_func2(char *cmd, char *parms);
static void exitmap_cmd_func2(char *cmd, char *parms);
static void fastmonsters_cmd_func2(char *cmd, char *parms);
static void freeze_cmd_func2(char *cmd, char *parms);
static bool give_cmd_func1(char *cmd, char *parms);
static void give_cmd_func2(char *cmd, char *parms);
static void god_cmd_func2(char *cmd, char *parms);
static void help_cmd_func2(char *cmd, char *parms);
static void if_cmd_func2(char *cmd, char *parms);
static bool kill_cmd_func1(char *cmd, char *parms);
static void kill_cmd_func2(char *cmd, char *parms);
static void license_cmd_func2(char *cmd, char *parms);
static void load_cmd_func2(char *cmd, char *parms);
static bool map_cmd_func1(char *cmd, char *parms);
static void map_cmd_func2(char *cmd, char *parms);
static void maplist_cmd_func2(char *cmd, char *parms);
static bool mapstats_cmd_func1(char *cmd, char *parms);
static void mapstats_cmd_func2(char *cmd, char *parms);
static bool name_cmd_func1(char *cmd, char *parms);
static void name_cmd_func2(char *cmd, char *parms);
static void newgame_cmd_func2(char *cmd, char *parms);
static void noclip_cmd_func2(char *cmd, char *parms);
static void nomonsters_cmd_func2(char *cmd, char *parms);
static void notarget_cmd_func2(char *cmd, char *parms);
static void pistolstart_cmd_func2(char *cmd, char *parms);
static bool play_cmd_func1(char *cmd, char *parms);
static void play_cmd_func2(char *cmd, char *parms);
static void playerstats_cmd_func2(char *cmd, char *parms);
static void print_cmd_func2(char *cmd, char *parms);
static void quit_cmd_func2(char *cmd, char *parms);
static void regenhealth_cmd_func2(char *cmd, char *parms);
static void reset_cmd_func2(char *cmd, char *parms);
static void resetall_cmd_func2(char *cmd, char *parms);
static void respawnitems_cmd_func2(char *cmd, char *parms);
static void respawnmonsters_cmd_func2(char *cmd, char *parms);
static void restartmap_cmd_func2(char *cmd, char *parms);
static bool resurrect_cmd_func1(char *cmd, char *parms);
static void resurrect_cmd_func2(char *cmd, char *parms);
static void save_cmd_func2(char *cmd, char *parms);
static bool spawn_cmd_func1(char *cmd, char *parms);
static void spawn_cmd_func2(char *cmd, char *parms);
static bool take_cmd_func1(char *cmd, char *parms);
static void take_cmd_func2(char *cmd, char *parms);
static bool teleport_cmd_func1(char *cmd, char *parms);
static void teleport_cmd_func2(char *cmd, char *parms);
static void thinglist_cmd_func2(char *cmd, char *parms);
static void timer_cmd_func2(char *cmd, char *parms);
static void toggle_cmd_func2(char *cmd, char *parms);
static void unbind_cmd_func2(char *cmd, char *parms);
static void vanilla_cmd_func2(char *cmd, char *parms);

static bool bool_cvars_func1(char *cmd, char *parms);
static void bool_cvars_func2(char *cmd, char *parms);
static void color_cvars_func2(char *cmd, char *parms);
static bool float_cvars_func1(char *cmd, char *parms);
static void float_cvars_func2(char *cmd, char *parms);
static bool int_cvars_func1(char *cmd, char *parms);
static void int_cvars_func2(char *cmd, char *parms);
static void str_cvars_func2(char *cmd, char *parms);
static void time_cvars_func2(char *cmd, char *parms);

static void alwaysrun_cvar_func2(char *cmd, char *parms);
static void am_display_cvar_func2(char *cmd, char *parms);
static void am_external_cvar_func2(char *cmd, char *parms);
static void am_followmode_cvar_func2(char *cmd, char *parms);
static void am_gridsize_cvar_func2(char *cmd, char *parms);
static void am_path_cvar_func2(char *cmd, char *parms);
static void am_rotatemode_cvar_func2(char *cmd, char *parms);
static bool armortype_cvar_func1(char *cmd, char *parms);
static void armortype_cvar_func2(char *cmd, char *parms);
static void autotilt_cvar_func2(char *cmd, char *parms);
static bool crosshair_cvar_func1(char *cmd, char *parms);
static void crosshair_cvar_func2(char *cmd, char *parms);
static bool english_cvar_func1(char *cmd, char *parms);
static void english_cvar_func2(char *cmd, char *parms);
static void episode_cvar_func2(char *cmd, char *parms);
static void expansion_cvar_func2(char *cmd, char *parms);
static bool joy_deadzone_cvars_func1(char *cmd, char *parms);
static void joy_deadzone_cvars_func2(char *cmd, char *parms);
static void joy_sensitivity_cvars_func2(char *cmd, char *parms);
static void mouselook_cvar_func2(char *cmd, char *parms);
static bool player_cvars_func1(char *cmd, char *parms);
static void player_cvars_func2(char *cmd, char *parms);
static bool playergender_cvar_func1(char *cmd, char *parms);
static void playergender_cvar_func2(char *cmd, char *parms);
static void playername_cvar_func2(char *cmd, char *parms);
static bool r_blood_cvar_func1(char *cmd, char *parms);
static void r_blood_cvar_func2(char *cmd, char *parms);
static void r_bloodsplats_translucency_cvar_func2(char *cmd, char *parms);
static void r_brightmaps_cvar_func2(char *cmd, char *parms);
static void r_color_cvar_func2(char *cmd, char *parms);
static void r_corpses_mirrored_cvar_func2(char *cmd, char *parms);
static bool r_detail_cvar_func1(char *cmd, char *parms);
static void r_detail_cvar_func2(char *cmd, char *parms);
static void r_ditheredlighting_cvar_func2(char *cmd, char *parms);
static void r_fixmaperrors_cvar_func2(char *cmd, char *parms);
static void r_fov_cvar_func2(char *cmd, char *parms);
static bool r_gamma_cvar_func1(char *cmd, char *parms);
static void r_gamma_cvar_func2(char *cmd, char *parms);
static void r_hud_cvar_func2(char *cmd, char *parms);
static void r_hud_translucency_cvar_func2(char *cmd, char *parms);
static void r_lowpixelsize_cvar_func2(char *cmd, char *parms);
static void r_mirroredweapons_cvar_func2(char *cmd, char *parms);
static void r_randomstartframes_cvar_func2(char *cmd, char *parms);
static void r_screensize_cvar_func2(char *cmd, char *parms);
static void r_shadows_translucency_cvar_func2(char *cmd, char *parms);
static void r_sprites_translucency_cvar_func2(char *cmd, char *parms);
static void r_supersampling_cvar_func2(char *cmd, char *parms);
static void r_textures_cvar_func2(char *cmd, char *parms);
static void r_textures_translucency_cvar_func2(char *cmd, char *parms);
static bool s_volume_cvars_func1(char *cmd, char *parms);
static void s_volume_cvars_func2(char *cmd, char *parms);
static void savegame_cvar_func2(char *cmd, char *parms);
static void skilllevel_cvar_func2(char *cmd, char *parms);
static bool turbo_cvar_func1(char *cmd, char *parms);
static void turbo_cvar_func2(char *cmd, char *parms);
static bool units_cvar_func1(char *cmd, char *parms);
static void units_cvar_func2(char *cmd, char *parms);
static void vid_borderlesswindow_cvar_func2(char *cmd, char *parms);
static bool vid_capfps_cvar_func1(char *cmd, char *parms);
static void vid_capfps_cvar_func2(char *cmd, char *parms);
static void vid_display_cvar_func2(char *cmd, char *parms);
static void vid_fullscreen_cvar_func2(char *cmd, char *parms);
static void vid_pillarboxes_cvar_func2(char *cmd, char *parms);
static bool vid_scaleapi_cvar_func1(char *cmd, char *parms);
static void vid_scaleapi_cvar_func2(char *cmd, char *parms);
static bool vid_scalefilter_cvar_func1(char *cmd, char *parms);
static void vid_scalefilter_cvar_func2(char *cmd, char *parms);
static void vid_screenresolution_cvar_func2(char *cmd, char *parms);
static void vid_showfps_cvar_func2(char *cmd, char *parms);
static bool vid_vsync_cvar_func1(char *cmd, char *parms);
static void vid_vsync_cvar_func2(char *cmd, char *parms);
static void vid_widescreen_cvar_func2(char *cmd, char *parms);
static void vid_windowpos_cvar_func2(char *cmd, char *parms);
static void vid_windowsize_cvar_func2(char *cmd, char *parms);
static bool weapon_cvar_func1(char *cmd, char *parms);
static void weapon_cvar_func2(char *cmd, char *parms);
static void weaponrecoil_cvar_func2(char *cmd, char *parms);

static int C_LookupValueFromAlias(const char *text, const valuealiastype_t valuealiastype)
{
    for (int i = 0; *valuealiases[i].text; i++)
        if (valuealiastype == valuealiases[i].type && M_StringCompare(text, valuealiases[i].text))
            return valuealiases[i].value;

    return INT_MIN;
}

char *C_LookupAliasFromValue(const int value, const valuealiastype_t valuealiastype)
{
    for (int i = 0; *valuealiases[i].text; i++)
        if (valuealiastype == valuealiases[i].type && value == valuealiases[i].value)
            return M_StringDuplicate(valuealiases[i].text);

    return commify(value);
}

#define CCMD(name, alt1, alt2, cond, func, parms, form, desc) \
    { #name, #alt1, #alt2, cond, func, parms, CT_CCMD, CF_NONE, NULL, 0, 0, 0, form, desc, 0, 0 }
#define CMD_CHEAT(name, parms) \
    { #name, "", "", cheat_func1, NULL, parms, CT_CHEAT, CF_NONE, NULL, 0, 0, 0, "", "", 0, 0 }
#define CVAR_BOOL(name, alt1, alt2, cond, func, flags, aliases, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_BOOLEAN | flags), &name, aliases, false, true, "", desc, name##_default, 0 }
#define CVAR_INT(name, alt1, alt2, cond, func, flags, aliases, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_INTEGER | flags), &name, aliases, name##_min, name##_max, "", desc, name##_default, 0 }
#define CVAR_FLOAT(name, alt1, alt2, cond, func, flags, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_FLOAT | flags), &name, 0, (int)name##_min, (int)name##_max, "", desc, name##_default, 0 }
#define CVAR_FLOAT2(name, alt1, alt2, cond, func, flags, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_FLOAT | flags), &name, 0, 0, 0, "", desc, name##_default, 0 }
#define CVAR_STR(name, alt1, alt2, cond, func, flags, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_STRING | flags), &name, 0, 0, 0, "", desc, 0, name##_default }
#define CVAR_TIME(name, alt1, alt2, cond, func, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_TIME | CF_READONLY), &name, 0, 0, 0, "", desc, 0, "" }
#define CVAR_OTHER(name, alt1, alt2, cond, func, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, CF_OTHER, &name, 0, 0, 0, "", desc, 0, name##_default }

consolecmd_t consolecmds[] =
{
    CCMD(alias, "", "", null_func1, alias_cmd_func2, true, ALIASCMDFORMAT,
        "Creates an " BOLDITALICS("alias") " that executes a string of " BOLDITALICS("commands") "."),
    CVAR_BOOL(alwaysrun, "", "", bool_cvars_func1, alwaysrun_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the player to always run instead of walk."),
    CVAR_INT(am_allmapcdwallcolor, am_allmapcdwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped lines in the automap indicating a change in a ceiling's height once the player has a computer area map "
        "power-up (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_allmapfdwallcolor, am_allmapfdwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped lines in the automap indicating a change in a floor's height once the player has a computer area map "
        "power-up (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_allmapwallcolor, am_allmapwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped solid walls in the automap once the player has a computer area map power-up (" BOLD("0") " to "
        BOLD("255") ")."),
    CVAR_INT(am_backcolor, am_backcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the automap's background (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_bloodsplatcolor, am_bloodsplatcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of blood splats in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_bluedoorcolor, am_bluedoorcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of doors in the automap unlocked with a blue keycard or skull key (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_bluekeycolor, am_bluekeycolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of blue keycards and skull keys in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_cdwallcolor, am_cdwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating a change in a ceiling's height (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_corpsecolor, am_corpsecolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of corpses in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_crosshaircolor, am_crosshaircolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the crosshair in the automap when follow mode is off (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_display, "", "", int_cvars_func1, am_display_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The display used to show the external automap."),
    CVAR_BOOL(am_external, "", "", bool_cvars_func1, am_external_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing the automap on an external display."),
    CVAR_INT(am_fdwallcolor, am_fdwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating a change in a floor's height (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(am_followmode, "", "", game_func1, am_followmode_cvar_func2, CF_MAPRESET, BOOLVALUEALIAS,
        "Toggles follow mode in the automap."),
    CVAR_BOOL(am_grid, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the grid in the automap."),
    CVAR_INT(am_gridcolor, am_gridcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the grid in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_OTHER(am_gridsize, "", "", null_func1, am_gridsize_cvar_func2,
        "The size of the grid in the automap (" BOLDITALICS("width") BOLD("\xD7") BOLDITALICS("height") ")."),
    CVAR_INT(am_markcolor, am_markcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of marks in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(am_path, "", "", bool_cvars_func1, am_path_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the player's path in the automap."),
    CVAR_INT(am_pathcolor, am_pathcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the player's path in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_playercolor, am_playercolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the player's arrow in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(am_playerstats, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles player stats in the automap."),
    CVAR_INT(am_reddoorcolor, am_reddoorcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of doors in the automap unlocked with a red keycard or skull key (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_redkeycolor, am_redkeycolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of red keycards and skull keys in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(am_rotatemode, "", "", bool_cvars_func1, am_rotatemode_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles rotate mode in the automap."),
    CVAR_INT(am_teleportercolor, am_teleportercolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of teleporter lines in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_thingcolor, am_thingcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of things in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_tswallcolor, am_tswallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating no change in height (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_wallcolor, am_wallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of solid walls in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_yellowdoorcolor, am_yellowdoorcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of doors in the automap unlocked with a yellow keycard or skull key (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_yellowkeycolor, am_yellowkeycolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of yellow keycards and skull keys in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(ammo, "", "", player_cvars_func1, player_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The amount of ammo the player has for their currently equipped weapon."),
    CVAR_INT(armor, armour, "", player_cvars_func1, player_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The player's armor (" BOLD("0%") " to " BOLD("200%") ")."),
    CVAR_INT(armortype, armourtype, "", armortype_cvar_func1, armortype_cvar_func2, CF_NONE, ARMORTYPEVALUEALIAS,
        "The player's armor type (" BOLD("none") ", " BOLD("green") " or " BOLD("blue") ")."),
    CVAR_BOOL(autoaim, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles vertical autoaiming as the player fires their weapon while using mouselook."),
    CVAR_BOOL(autoload, "", "", bool_cvars_func1, bool_cvars_func2, CF_PISTOLSTART, BOOLVALUEALIAS,
        "Toggles automatically loading the last savegame if the player dies."),
    CVAR_BOOL(autosave, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles automatically saving the game at the start of each map."),
    CVAR_BOOL(autotilt, "", "", bool_cvars_func1, autotilt_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles automatically tilting the player's view while going up or down a flight of stairs."),
    CVAR_BOOL(autouse, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles automatically using doors and switches in front of the player."),
    CCMD(bind, "", "", null_func1, bind_cmd_func2, true, BINDCMDFORMAT,
        "Binds an " BOLDITALICS("+action") " or string of " BOLDITALICS("commands") " to a " BOLDITALICS("control") "."),
    CCMD(bindlist, "", "", null_func1, bindlist_cmd_func2, false, "",
        "Lists all controls bound to an " BOLDITALICS("+action") " or string of commands."),
    CMD_CHEAT(buddha, false),
    CVAR_BOOL(centerweapon, centreweapon, "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles centering the player's weapon when fired."),
    CCMD(clear, "", "", null_func1, clear_cmd_func2, false, "",
        "Clears the console."),
    CCMD(cmdlist, "", ccmdlist, null_func1, cmdlist_cmd_func2, true, "[" BOLDITALICS("searchstring") "]",
        "Lists all console commands."),
    CVAR_BOOL(con_obituaries, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles obituaries in the console when the player or monsters are killed."),
    CCMD(condump, "", "", condump_cmd_func1, condump_cmd_func2, true, "[" BOLDITALICS("filename") "[" BOLD(".txt") "]]",
        "Dumps the contents of the console to a file."),
    CVAR_INT(crosshair, "", "", crosshair_cvar_func1, crosshair_cvar_func2, CF_NONE, CROSSHAIRVALUEALIAS,
        "Toggles the player's crosshair (" BOLD("none") ", " BOLD("cross") " or " BOLD("dot") ")."),
    CVAR_INT(crosshaircolor, crosshaircolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the player's crosshair (" BOLD("0") " to " BOLD("255") ")."),
    CCMD(cvarlist, "", "", null_func1, cvarlist_cmd_func2, true, "[" BOLDITALICS("searchstring") "]",
        "Lists all console variables."),
    CCMD(endgame, "", "", game_func1, endgame_cmd_func2, false, "",
        "Ends a game."),
    CVAR_BOOL(english, "", "", english_cvar_func1, english_cvar_func2, CF_NONE, ENGLISHVALUEALIAS,
        "Toggles the use of American or International English (" BOLD("american") " or " BOLD("international") ")."),
    CVAR_INT(episode, "", "", int_cvars_func1, episode_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected " ITALICS("DOOM") " episode in the menu (" BOLD("1") " to " BOLD("5") ")."),
    CCMD(exec, "", "", null_func1, exec_cmd_func2, true, EXECCMDFORMAT,
        "Executes all commands in a file."),
    CCMD(exitmap, "", "", alive_func1, exitmap_cmd_func2, false, "",
        "Exits the current map."),
    CVAR_INT(expansion, "", "", int_cvars_func1, expansion_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected " ITALICS("DOOM II") " expansion in the menu (" BOLD("1") " or " BOLD("2") ")."),
    CCMD(explode, "", "", kill_cmd_func1, kill_cmd_func2, true, EXPLODECMDFORMAT,
        "Explodes all " BOLD("barrels") " or " BOLD("missiles") "."),
    CVAR_INT(facebackcolor, facebackcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The background color of the player's face in the status bar (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(fade, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a fading effect when transitioning between some screens."),
    CCMD(fastmonsters, "", "", nightmare_func1, fastmonsters_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles fast monsters."),
    CVAR_BOOL(flashkeys, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles flashing the required keycard or skull key when the player tries to open a locked door."),
    CCMD(freeze, "", "", game_func1, freeze_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles freeze mode."),
    CVAR_TIME(gametime, "", "", null_func1, time_cvars_func2,
        "The amount of time " ITALICS(DOOMRETRO_NAME) " has been running."),
    CCMD(give, "", "", give_cmd_func1, give_cmd_func2, true, GIVECMDFORMAT,
        "Gives " BOLD("ammo") ", " BOLD("armor") ", " BOLD("health") ", " BOLD("keys") ", " BOLD("weapons") ", or " BOLD("all")
        " or certain " BOLDITALICS("items") " to the player."),
    CCMD(god, "", "", alive_func1, god_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles god mode."),
    CVAR_BOOL(groupmessages, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the grouping of identical player messages."),
    CVAR_INT(health, "", "", player_cvars_func1, player_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The player's health (" BOLD("-99%") " to " BOLD("200%") ")."),
    CCMD(help, "", "", null_func1, help_cmd_func2, false, "",
        "Opens the " ITALICS(DOOMRETRO_WIKINAME ".")),
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
    CCMD(if, "", "", null_func1, if_cmd_func2, true, IFCMDFORMAT,
        "Executes a string of " BOLDITALICS("commands") " if a " BOLDITALICS("CVAR") " equals a " BOLDITALICS("value") "."),
    CVAR_BOOL(infighting, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles infighting amongst monsters once the player dies."),
    CVAR_BOOL(infiniteheight, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles giving the player and monsters infinite height."),
    CVAR_STR(iwadfolder, "", "", null_func1, str_cvars_func2, CF_NONE,
        "The folder the current IWAD is in."),
    CVAR_BOOL(joy_analog, joy_analogue, "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles whether movement using the controller's thumbsticks is analog or digital."),
    CVAR_FLOAT(joy_deadzone_left, "", "", joy_deadzone_cvars_func1, joy_deadzone_cvars_func2, CF_PERCENT,
        "The dead zone of the controller's left thumbstick (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_FLOAT(joy_deadzone_right, "", "", joy_deadzone_cvars_func1, joy_deadzone_cvars_func2, CF_PERCENT,
        "The dead zone of the controller's right thumbstick (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(joy_invertyaxis, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles inverting the vertical axis of the controller's right thumbstick when the player looks up or down."),
    CVAR_INT(joy_rumble_barrels, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the controller rumbles when the player is near an exploding barrel (" BOLD("0%") " to " BOLD("200%") ")."),
    CVAR_INT(joy_rumble_damage, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the controller rumbles when the player receives damage (" BOLD("0%") " to " BOLD("200%") ")."),
    CVAR_INT(joy_rumble_weapons, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the controller rumbles when the player fires their weapon (" BOLD("0%") " to " BOLD("200%") ")."),
    CVAR_FLOAT(joy_sensitivity_horizontal, "", "", float_cvars_func1, joy_sensitivity_cvars_func2, CF_NONE,
        "The horizontal sensitivity of the controller's thumbsticks (" BOLD("0") " to " BOLD("128") ")."),
    CVAR_FLOAT(joy_sensitivity_vertical, "", "", float_cvars_func1, joy_sensitivity_cvars_func2, CF_NONE,
        "The vertical sensitivity of the controller's thumbsticks (" BOLD("0") " to " BOLD("128") ")."),
    CVAR_BOOL(joy_swapthumbsticks, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles swapping the controller's left and right thumbsticks."),
    CVAR_INT(joy_thumbsticks, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The number of thumbsticks to use on the controller (" BOLD("1") " or " BOLD("2") ")."),
    CCMD(kill, "", "", kill_cmd_func1, kill_cmd_func2, true, KILLCMDFORMAT,
        "Kills the " BOLD("player") ", " BOLD("all") " monsters or a type of " BOLDITALICS("monster") "."),
    CCMD(license, licence, "", null_func1, license_cmd_func2, false, "",
        "Displays the " ITALICS(DOOMRETRO_LICENSE ".")),
    CCMD(load, "", "", null_func1, load_cmd_func2, true, LOADCMDFORMAT,
        "Loads a game from a file."),
    CVAR_BOOL(m_acceleration, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the acceleration of mouse movement."),
    CVAR_BOOL(m_doubleclick_use, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles double-clicking a mouse button to perform a " BOLD("+use") " action."),
    CVAR_BOOL(m_invertyaxis, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles inverting the mouse's vertical axis when using mouselook."),
    CVAR_BOOL(m_novertical, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles no vertical movement of the mouse."),
    CVAR_BOOL(m_pointer, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the mouse pointer in the menu."),
    CVAR_FLOAT(m_sensitivity, "", "", float_cvars_func1, float_cvars_func2, CF_NONE,
        "The mouse's sensitivity (" BOLD("0") " to " BOLD("128") ")."),
    CCMD(map, "", warp, map_cmd_func1, map_cmd_func2, true, MAPCMDFORMAT1,
        "Warps the player to another map."),
    CCMD(maplist, "", "", null_func1, maplist_cmd_func2, false, "",
        "Lists all the maps in the currently loaded WADs."),
    CCMD(mapstats, "", "", mapstats_cmd_func1, mapstats_cmd_func2, false, "",
        "Shows stats about the current map."),
    CVAR_TIME(maptime, "", "", game_func1, time_cvars_func2,
        "The amount of time the player has spent in the current map."),
    CVAR_BOOL(melt, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a melting effect when transitioning between some screens."),
    CVAR_BOOL(messages, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles displaying player messages at the top of the screen."),
    CVAR_BOOL(mouselook, "", "", bool_cvars_func1, mouselook_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles mouselook."),
    CVAR_INT(movebob, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's view bobs as they move (" BOLD("0%") " to " BOLD("100%") ")."),
    CCMD(name, "", "", name_cmd_func1, name_cmd_func2, true, NAMECMDFORMAT,
        "Gives a " BOLDITALICS("name") " to the " BOLDITALICS("monster") " nearest to the player."),
    CVAR_BOOL(negativehealth, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing the player's health as less than " BOLD("0%") " when they die."),
    CCMD(newgame, "", "", null_func1, newgame_cmd_func2, true, "",
        "Starts a new game."),
    CCMD(noclip, "", "", game_func1, noclip_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles no clipping mode."),
    CCMD(nomonsters, "", "", null_func1, nomonsters_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles the presence of monsters in maps."),
    CCMD(notarget, "", "", game_func1, notarget_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles monsters not targeting the player."),
    CCMD(pistolstart, "", "", null_func1, pistolstart_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles the player starting each map with 100% health, no armor, and only a pistol with 50 bullets."),
    CCMD(play, "", "", play_cmd_func1, play_cmd_func2, true, PLAYCMDFORMAT,
        "Plays a " BOLDITALICS("sound effect") " or " BOLDITALICS("music") " lump."),
    CVAR_INT(playergender, "", "", playergender_cvar_func1, playergender_cvar_func2, CF_NONE, PLAYERGENDERVALUEALIAS,
        "The player's gender (" BOLD("male") ", " BOLD("female") " or " BOLD("nonbinary") ")."),
    CVAR_STR(playername, "", "", null_func1, playername_cvar_func2, CF_NONE,
        "The player's name."),
    CCMD(playerstats, "", "", null_func1, playerstats_cmd_func2, false, "",
        "Shows stats about the player."),
    CCMD(print, "", "", game_func1, print_cmd_func2, true, PRINTCMDFORMAT,
        "Prints a player \"" BOLDITALICS("message") "\"."),
    CCMD(quit, "", exit, null_func1, quit_cmd_func2, false, "",
        "Quits to the " DESKTOP "."),
    CVAR_BOOL(r_althud, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles an alternate heads-up display when in widescreen mode."),
    CVAR_INT(r_berserkeffect, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The intensity of the red effect when the player has a berserk power-up and their fists equipped (" BOLD("0") " to " BOLD("8")
        ")."),
    CVAR_INT(r_blood, "", "", r_blood_cvar_func1, r_blood_cvar_func2, CF_NONE, BLOODVALUEALIAS,
        "The colors of the blood spilled by the player and monsters (" BOLD("all") ", " BOLD("none") ", " BOLD("red") ", " BOLD("green")
        " or " BOLD("nofuzz") ")."),
    CVAR_BOOL(r_blood_melee, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles spawning blood during melee attacks from monsters."),
    CVAR_INT(r_bloodsplats_max, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The maximum number of blood splats allowed in the current map (" BOLD("0") " to " BOLD("1,048,576") ")."),
    CVAR_INT(r_bloodsplats_total, "", "", int_cvars_func1, int_cvars_func2, CF_READONLY, NOVALUEALIAS,
        "The total number of blood splats in the current map."),
    CVAR_BOOL(r_bloodsplats_translucency, "", "", bool_cvars_func1, r_bloodsplats_translucency_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of blood splats."),
    CVAR_BOOL(r_brightmaps, "", "", bool_cvars_func1, r_brightmaps_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles brightmaps on some wall textures."),
    CVAR_INT(r_color, r_colour, "", int_cvars_func1, r_color_cvar_func2, CF_PERCENT, NOVALUEALIAS,
        "The intensity of the colors on the screen (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(r_corpses_color, r_corpses_colour, "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomly colored marine corpses."),
    CVAR_BOOL(r_corpses_gib, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles some corpses gibbing in reaction to nearby barrel and rocket explosions."),
    CVAR_BOOL(r_corpses_mirrored, "", "", bool_cvars_func1, r_corpses_mirrored_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomly mirrored corpses."),
    CVAR_BOOL(r_corpses_moreblood, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles blood splats spawned around corpses at the start of each map."),
    CVAR_BOOL(r_corpses_nudge, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles corpses and the items they drop being nudged when walked over."),
    CVAR_BOOL(r_corpses_slide, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles corpses sliding in reaction to nearby barrel and rocket explosions."),
    CVAR_BOOL(r_corpses_smearblood, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles corpses leaving blood splats behind as they slide."),
    CVAR_BOOL(r_damageeffect, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the red effect when the player is injured."),
    CVAR_BOOL(r_detail, "", "", r_detail_cvar_func1, r_detail_cvar_func2, CF_NONE, DETAILVALUEALIAS,
        "Toggles the graphic detail (" BOLD("high") " or " BOLD("low") ")."),
    CVAR_BOOL(r_diskicon, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing a disk icon when loading and saving."),
    CVAR_BOOL(r_ditheredlighting, "", "", bool_cvars_func1, r_ditheredlighting_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles dithered lighting cast on textures and sprites."),
    CVAR_BOOL(r_fixmaperrors, "", "", bool_cvars_func1, r_fixmaperrors_cvar_func2, CF_NEXTMAP, BOOLVALUEALIAS,
        "Toggles fixing many mapping errors in the " ITALICS("DOOM") " and " ITALICS("DOOM II") " WADs."),
    CVAR_BOOL(r_fixspriteoffsets, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles fixing sprite offsets."),
    CVAR_BOOL(r_floatbob, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles some power-ups bobbing up and down."),
    CVAR_INT(r_fov, "", "", int_cvars_func1, r_fov_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The player's field of view (" BOLD("45") "\xB0 to " BOLD("135") "\xB0)."),
    CVAR_FLOAT2(r_gamma, "", "", r_gamma_cvar_func1, r_gamma_cvar_func2, CF_NONE,
        "The screen's gamma correction level (" BOLD("off") ", or " BOLD("0.50") " to " BOLD("2.0") ")."),
    CVAR_BOOL(r_graduallighting, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles gradual lighting under doors and crushing sectors."),
    CVAR_BOOL(r_homindicator, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a flashing \"Hall Of Mirrors\" indicator."),
    CVAR_BOOL(r_hud, "", "", bool_cvars_func1, r_hud_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a heads-up display in widescreen mode."),
    CVAR_BOOL(r_hud_translucency, "", "", bool_cvars_func1, r_hud_translucency_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of the heads-up display in widescreen mode."),
    CVAR_BOOL(r_liquid_bob, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the bob of liquid sectors and the sprites in them."),
    CVAR_BOOL(r_liquid_clipsprites, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles clipping the bottom of sprites in liquid sectors."),
    CVAR_BOOL(r_liquid_current, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a slight current being applied to liquid sectors."),
    CVAR_BOOL(r_liquid_lowerview, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles lowering the player's view when they are in a liquid sector."),
    CVAR_BOOL(r_liquid_swirl, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the swirl of liquid sectors."),
    CVAR_OTHER(r_lowpixelsize, "", "", null_func1, r_lowpixelsize_cvar_func2,
        "The size of each pixel when the graphic detail is low (" BOLDITALICS("width") BOLD("\xD7") BOLDITALICS("height") ")."),
    CVAR_BOOL(r_mirroredweapons, "", "", bool_cvars_func1, r_mirroredweapons_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomly mirroring the weapons dropped by monsters."),
    CVAR_BOOL(r_pickupeffect, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the gold effect when the player picks something up."),
    CVAR_BOOL(r_playersprites, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing the player's weapon."),
    CVAR_BOOL(r_radsuiteffect, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the green effect while the player wears a radiation shielding suit power-up."),
    CVAR_BOOL(r_randomstartframes, "", "", bool_cvars_func1, r_randomstartframes_cvar_func2, CF_NEXTMAP, BOOLVALUEALIAS,
        "Toggles randomizing the start frames of certain sprites."),
    CVAR_BOOL(r_rockettrails, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the trail of smoke behind rockets fired by the player and cyberdemons."),
    CVAR_INT(r_screensize, "", "", int_cvars_func1, r_screensize_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The screen size (" BOLD("0") " to " BOLD("8") ")."),
    CVAR_BOOL(r_shadows, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles sprites casting shadows."),
    CVAR_BOOL(r_shadows_translucency, "", "", bool_cvars_func1, r_shadows_translucency_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of shadows cast by sprites."),
    CVAR_BOOL(r_shake_barrels, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles shaking the player's view when they are near an exploding barrel."),
    CVAR_BOOL(r_shake_berserk, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles shaking the player's view when they have a berserk power-up and punch something."),
    CVAR_INT(r_shake_damage, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's view shakes when they receive damage (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(r_sprites_translucency, "", "", bool_cvars_func1, r_sprites_translucency_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of certain sprites."),
    CVAR_BOOL(r_supersampling, "", "", bool_cvars_func1, r_supersampling_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles SSAA (supersampling anti-aliasing) when the graphic detail is low."),
    CVAR_BOOL(r_textures, "", "", bool_cvars_func1, r_textures_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing all textures."),
    CVAR_BOOL(r_textures_translucency, "", "", bool_cvars_func1, r_textures_translucency_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of certain " ITALICS("BOOM-") "compatible wall textures."),
    CCMD(regenhealth, "", "", null_func1, regenhealth_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles regenerating the player's health by 1% every second when it's below 100%."),
    CCMD(remove, "", "", kill_cmd_func1, kill_cmd_func2, true, REMOVECMDFORMAT,
        "Removes all " BOLD("decorations") ", " BOLD("corpses") ", " BOLD("bloodsplats") ", " BOLD("items") ", a type of "
        BOLDITALICS("item") ", or " BOLD("everything") "."),
    CCMD(reset, "", "", null_func1, reset_cmd_func2, true, RESETCMDFORMAT,
        "Resets a " BOLDITALICS("CVAR") " to its default."),
    CCMD(resetall, "", "", null_func1, resetall_cmd_func2, false, "",
        "Resets all CVARs and bound controls to their defaults."),
    CCMD(respawnitems, "", "", null_func1, respawnitems_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles respawning items."),
    CCMD(respawnmonsters, "", "", nightmare_func1, respawnmonsters_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles respawning monsters."),
    CCMD(restartmap, "", "", game_func1, restartmap_cmd_func2, false, "",
        "Restarts the current map."),
    CCMD(resurrect, "", "", resurrect_cmd_func1, resurrect_cmd_func2, true, RESURRECTCMDFORMAT,
        "Resurrects the " BOLD("player") ", " BOLD("all") " monsters or a type of " BOLDITALICS("monster") "."),
    CVAR_INT(s_channels, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The number of sound effects that can be played at the same time (" BOLD("8") " to " BOLD("64") ")."),
    CVAR_BOOL(s_lowermenumusic, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles lowering the music's volume in the menu and console."),
    CVAR_BOOL(s_musicinbackground, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles continuing to play music in the background when " ITALICS(DOOMRETRO_NAME) " loses focus."),
    CVAR_INT(s_musicvolume, "", "", s_volume_cvars_func1, s_volume_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The volume level of music (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(s_randommusic, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomizing the music in each map."),
    CVAR_BOOL(s_randompitch, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomizing the pitch of sound effects made by monsters."),
    CVAR_INT(s_sfxvolume, "", "", s_volume_cvars_func1, s_volume_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The volume level of sound effects (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(s_stereo, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles playing sound effects in mono or stereo."),
    CCMD(save, "", "", alive_func1, save_cmd_func2, true, SAVECMDFORMAT,
        "Saves the game to a file."),
    CVAR_INT(savegame, "", "", int_cvars_func1, savegame_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected savegame in the menu (" BOLD("1") " to " BOLD("6") ")."),
    CVAR_BOOL(secretmessages, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles displaying a message when the player finds a secret."),
    CVAR_INT(skilllevel, "", "", int_cvars_func1, skilllevel_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected skill level in the menu (" BOLD("1") " to " BOLD("5") ")."),
    CCMD(spawn, "", summon, spawn_cmd_func1, spawn_cmd_func2, true, SPAWNCMDFORMAT,
        "Spawns an " BOLDITALICS("item") " or " BOLDITALICS("monster") " in front of the player."),
    CVAR_INT(stillbob, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's view and weapon bob up and down when they stand still (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_INT(sucktime, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The amount of time in hours the player must complete the current map before \"SUCKS!\" is shown on the intermission screen."),
    CCMD(take, "", "", take_cmd_func1, take_cmd_func2, true, TAKECMDFORMAT,
        "Takes " BOLD("ammo") ", " BOLD("armor") ", " BOLD("health") ", " BOLD("keys") ", " BOLD("weapons") ", or " BOLD("all")
        " or certain " BOLDITALICS("items") " away from the player."),
    CCMD(teleport, "", "", teleport_cmd_func1, teleport_cmd_func2, true, TELEPORTCMDFORMAT,
        "Teleports the player to (" BOLDITALICS("x") ", " BOLDITALICS("y") ", " BOLDITALICS("z") ") in the current map."),
    CCMD(thinglist, "", "", game_func1, thinglist_cmd_func2, false, "",
        "Lists all things in the current map."),
    CCMD(timer, "", "", null_func1, timer_cmd_func2, true, TIMERCMDFORMAT,
        "Sets a timer to exit each map after a number of " BOLDITALICS("minutes") "."),
    CCMD(toggle, "", "", null_func1, toggle_cmd_func2, true, TOGGLECMDFORMAT,
        "Toggles the value of a " BOLDITALICS("CVAR") " between " BOLD("on") " and " BOLD("off") "."),
    CVAR_BOOL(tossdrop, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles tossing items dropped by monsters when they die."),
    CVAR_INT(turbo, "", "", turbo_cvar_func1, turbo_cvar_func2, CF_PERCENT, NOVALUEALIAS,
        "The speed the player moves (" BOLD("10%") " to " BOLD("400%") ")."),
    CCMD(unbind, "", "", null_func1, unbind_cmd_func2, true, UNBINDCMDFORMAT,
        "Unbinds the " BOLDITALICS("+action") " from a " BOLDITALICS("control") "."),
    CVAR_BOOL(units, "", "", units_cvar_func1, units_cvar_func2, CF_NONE, UNITSVALUEALIAS,
        "The units used by certain stats (" BOLD("imperial") " or " BOLD("metric") ")."),
    CCMD(vanilla, "", "", null_func1, vanilla_cmd_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles vanilla mode."),
    CVAR_STR(version, "", "", null_func1, str_cvars_func2, CF_READONLY,
        ITALICS(DOOMRETRO_NAME "'s") " version."),
    CVAR_BOOL(vid_borderlesswindow, "", "", bool_cvars_func1, vid_borderlesswindow_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles using a borderless window when fullscreen."),
    CVAR_INT(vid_capfps, "", "", vid_capfps_cvar_func1, vid_capfps_cvar_func2, CF_NONE, CAPVALUEALIAS,
        "The number of frames per second at which to cap the framerate (" BOLD("off") ", or " BOLD("35") " to " BOLD("1,000") "). "
        "There is no interpolation between frames when this CVAR is " BOLD("35") "."),
    CVAR_INT(vid_display, "", "", int_cvars_func1, vid_display_cvar_func2, CF_NONE, NOVALUEALIAS,
        "The display used to play " ITALICS(DOOMRETRO_NAME) " on."),
#if !defined(_WIN32)
    CVAR_STR(vid_driver, "", "", null_func1, str_cvars_func2, CF_NONE,
        "The video driver used to play " ITALICS(DOOMRETRO_NAME) "."),
#endif
    CVAR_BOOL(vid_fullscreen, "", "", bool_cvars_func1, vid_fullscreen_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles between fullscreen and a window."),
    CVAR_INT(vid_motionblur, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount of motion blur when the player turns quickly (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(vid_pillarboxes, "", "", bool_cvars_func1, vid_pillarboxes_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the use of the pillarboxes either side of the screen for certain effects when not in widescreen mode."),
#if defined(_WIN32)
    CVAR_STR(vid_scaleapi, "", "", vid_scaleapi_cvar_func1, vid_scaleapi_cvar_func2, CF_NONE,
        "The API used to scale each frame (" BOLD("\"direct3d\"") ", " BOLD("\"opengl\"") " or " BOLD("\"software\"") ")."),
#else
    CVAR_STR(vid_scaleapi, "", "", vid_scaleapi_cvar_func1, vid_scaleapi_cvar_func2, CF_NONE,
        "The API used to scale each frame (" BOLD("\"opengl\"") ", " BOLD("\"opengles\"") ", " BOLD("\"opengles2\"") " or "
        BOLD("\"software\"") ")."),
#endif
    CVAR_STR(vid_scalefilter, "", "", vid_scalefilter_cvar_func1, vid_scalefilter_cvar_func2, CF_NONE,
        "The filter used to scale each frame (" BOLD("\"nearest\"") ", " BOLD("\"linear\"") " or " BOLD("\"nearest_linear\"") ")."),
    CVAR_OTHER(vid_screenresolution, "", "", null_func1, vid_screenresolution_cvar_func2,
        "The screen's resolution when fullscreen (" BOLD("desktop") " or " BOLDITALICS("width") BOLD("\xD7") BOLDITALICS("height")
        ")."),
    CVAR_BOOL(vid_showfps, "", "", bool_cvars_func1, vid_showfps_cvar_func2, CF_STARTUPRESET, BOOLVALUEALIAS,
        "Toggles showing the number of frames per second."),
#if defined(__APPLE__)
    CVAR_INT(vid_vsync, "", "", vid_vsync_cvar_func1, vid_vsync_cvar_func2, CF_NONE, VSYNCVALUEALIAS,
        "Toggles vertical sync with the display's refresh rate (" BOLD("on") " or " BOLD("off") ")."),
#else
    CVAR_INT(vid_vsync, "", "", vid_vsync_cvar_func1, vid_vsync_cvar_func2, CF_NONE, VSYNCVALUEALIAS,
        "Toggles vertical sync with the display's refresh rate (" BOLD("on") ", " BOLD("off") " or " BOLD("adaptive") ")."),
#endif
    CVAR_BOOL(vid_widescreen, "", "", bool_cvars_func1, vid_widescreen_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles widescreen mode."),
    CVAR_OTHER(vid_windowpos, "", vid_windowposition, null_func1, vid_windowpos_cvar_func2,
        "The position of the window on the desktop (" BOLD("centered") " or " BOLD("(") BOLDITALICS("x") BOLD(",") BOLDITALICS("y")
        BOLD(")") ")."),
    CVAR_OTHER(vid_windowsize, "", "", null_func1, vid_windowsize_cvar_func2,
        "The size of the window on the desktop (" BOLDITALICS("width") BOLD("\xD7") BOLDITALICS("height") ")."),
#if defined(_WIN32)
    CVAR_STR(wad, "", "", null_func1, str_cvars_func2, CF_READONLY,
        "The last WAD to be opened using the WAD launcher."),
#endif
    CVAR_INT(warninglevel, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The console's warning level (" BOLD("0") ", " BOLD("1") " or " BOLD("2") ")."),
    CVAR_INT(weapon, "", "", weapon_cvar_func1, weapon_cvar_func2, CF_NONE, WEAPONVALUEALIAS,
        "The player's currently equipped weapon (" BOLD("fists") ", " BOLD("chainsaw") ", " BOLD("pistol") ", " BOLD("shotgun") ", "
        BOLD("supershotgun") ", " BOLD("chaingun") ", " BOLD("rocketlauncher") ", " BOLD("plasmarifle") " or " BOLD("bfg9000") ")."),
    CVAR_INT(weaponbob, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the player's weapon bobs as they move (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(weaponbounce, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the bounce of the player's weapon when they land after a fall."),
    CVAR_BOOL(weaponrecoil, "", "", bool_cvars_func1, weaponrecoil_cvar_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the recoil of the player's weapon when they fire it."),

    { "", "", "", null_func1, NULL, 0, 0, CF_NONE, NULL, 0, 0, 0, "", "" }
};

static bool run(void)
{
    return (gamekeydown[keyboardrun] ^ (!!mousebuttons[mouserun]) ^ (!!(gamecontrollerbuttons & gamecontrollerrun)) ^ alwaysrun);
}

static bool strafe(void)
{
    return (gamekeydown[keyboardstrafe] || mousebuttons[mousestrafe] || (gamecontrollerbuttons & gamecontrollerstrafe));
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
    if (automapactive || mapwindow)
        AM_ToggleFollowMode(!am_followmode);
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
    if (!nojump)
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
    if (automapactive)
        AM_ToggleMaxZoom();
}

static void menu_action_func(void)
{
    M_StartControlPanel();
    S_StartSound(NULL, sfx_swtchn);
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
        AM_ToggleRotateMode(!am_rotatemode);
}

static void screenshot_action_func(void)
{
    G_ScreenShot();
    S_StartSound(NULL, sfx_scrsht);
    memset(screens[0], nearestwhite, SCREENAREA);
    D_FadeScreen(true);
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

int C_GetIndex(const char *cmd)
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

    M_StringCopy(description, consolecmds[index].description, sizeof(description));
    description[0] = tolower(description[0]);

    if (english == english_international)
        M_AmericanToInternationalEnglish(description);

    if (consolecmds[index].type == CT_CCMD)
        C_Output("This CCMD %s", description);
    else
        C_Output("This CVAR %s%s", (M_StringStartsWith(description, "toggles") ? "" :
            ((consolecmds[index].flags & CF_READONLY) ? "is " : "changes ")), description);
}

static void C_ShowFormat(int index)
{
    char    name[255];
    char    format[255];

    M_StringCopy(name, (english == english_american || M_StringCompare(consolecmds[index].altspelling, EMPTYVALUE) ?
        consolecmds[index].name : consolecmds[index].altspelling), sizeof(name));
    M_StringCopy(format, consolecmds[index].format, sizeof(format));

    if (english == english_international)
        M_AmericanToInternationalEnglish(format);

    C_Output(BOLD("%s") " %s", name, format);
}

static void C_ShowWarning(int index)
{
    const int   flags = consolecmds[index].flags;

    if (flags & CF_READONLY)
        C_Warning(0, "It is read-only.");
    else if (flags & CF_STARTUPRESET)
        C_Warning(0, "It is reset to its default during startup.");
    else if (flags & CF_MAPRESET)
        C_Warning(0, "It is reset to its default at the start of each map.");
    else if ((flags & CF_NEXTMAP) && gamestate == GS_LEVEL)
        C_Warning(0, "Changing it won't be effective until the next map.");
    else if ((flags & CF_PISTOLSTART) && pistolstart)
        C_Warning(0, "It has no effect while the " BOLD("pistolstart") " CCMD is used.");
}

static bool alive_func1(char *cmd, char *parms)
{
    if (gamestate != GS_LEVEL)
        return game_func1(cmd, parms);
    else if (viewplayer->health > 0)
        return true;
    else
    {
        C_Input(consoleinput);
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(playername, playername_default))
            C_Warning(0, DEADPLAYERWARNING, "you", "are");
        else
            C_Warning(0, DEADPLAYERWARNING, playername, "is");

        consoleinput[0] = '\0';

        return false;
    }
}

static bool cheat_func1(char *cmd, char *parms)
{
    if (M_StringCompare(cmd, cheat_clev.sequence))
    {
        bool    result;

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
    else if (M_StringCompare(cmd, cheat_choppers.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_buddha.sequence))
        return (gameskill != sk_nightmare && viewplayer->health > 0);
    else if (M_StringCompare(cmd, cheat_mypos.sequence))
        return true;
    else if (M_StringCompare(cmd, cheat_amap.sequence))
        return (gameskill != sk_nightmare && (automapactive || mapwindow));

    return false;
}

static bool game_func1(char *cmd, char *parms)
{
    if (gamestate == GS_LEVEL)
        return true;
    else
    {
        C_Input(consoleinput);
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(playername, playername_default))
            C_Warning(0, NOGAMEWARNING, "you", "are");
        else
            C_Warning(0, NOGAMEWARNING, playername, "is");

        consoleinput[0] = '\0';

        return false;
    }
}

static bool nightmare_func1(char *cmd, char *parms)
{
    if (gamestate != GS_LEVEL)
        return game_func1(cmd, parms);
    else if (gameskill != sk_nightmare)
        return true;
    else
    {
        C_Input(consoleinput);
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(playername, playername_default))
            C_Warning(0, NIGHTMAREWARNING, "you", "are");
        else
            C_Warning(0, NIGHTMAREWARNING, playername, "is");

        consoleinput[0] = '\0';

        return false;
    }
}

static bool null_func1(char *cmd, char *parms)
{
    return true;
}

//
// alias CCMD
//
bool C_ExecuteAlias(const char *alias)
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
        return;
    }

    M_StripQuotes(parm1);

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(parm1, consolecmds[i].name))
        {
            C_Warning(0, "An alias cannot be the same as an existing CVAR or CCMD.");
            return;
        }

    if (!*parm2)
    {
        for (int i = 0; i < MAXALIASES; i++)
            if (*aliases[i].name && M_StringCompare(parm1, aliases[i].name))
            {
                aliases[i].name[0] = '\0';
                aliases[i].string[0] = '\0';
                M_SaveCVARs();
                C_Output("The " BOLD("%s") " alias has been removed.", parm1);

                return;
            }

        return;
    }

    M_StripQuotes(parm2);

    for (int i = 0; i < MAXALIASES; i++)
        if (*aliases[i].name && M_StringCompare(parm1, aliases[i].name))
        {
            M_StringCopy(aliases[i].string, parm2, sizeof(aliases[0].string));
            M_SaveCVARs();
            C_Output("The " BOLD("%s") " alias has been updated.", parm1);

            return;
        }

    for (int i = 0; i < MAXALIASES; i++)
        if (!*aliases[i].name)
        {
            M_StringCopy(aliases[i].name, parm1, sizeof(aliases[0].name));
            M_StringCopy(aliases[i].string, parm2, sizeof(aliases[0].string));
            C_Output("The " BOLD("%s") " alias has been created.", parm1);
            M_SaveCVARs();

            return;
        }
}

//
// bind CCMD
//
static void C_UnbindDuplicates(const int keep, const controltype_t type, const int control)
{
    for (int i = 0; *actions[i].action; i++)
        if (i != keep)
        {
            if (type == keyboardcontrol)
            {
                if (actions[i].keyboard1 && controls[control].value == *(int *)actions[i].keyboard1)
                {
                    C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                        " action has been unbound from the " BOLD("%s")" control.",
                        actions[i].action, controls[control].control);
                    *(int *)actions[i].keyboard1 = 0;
                }

                if (actions[i].keyboard2 && controls[control].value == *(int *)actions[i].keyboard2)
                {
                    C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                        " action has been unbound from the " BOLD("%s")" control.",
                        actions[i].action, controls[control].control);
                    *(int *)actions[i].keyboard2 = 0;
                }
            }
            else if (type == mousecontrol)
            {
                if (actions[i].mouse1 && controls[control].value == *(int *)actions[i].mouse1)
                {
                    C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                        " action has been unbound from the " BOLD("%s")" control.",
                        actions[i].action, controls[control].control);
                    *(int *)actions[i].mouse1 = -1;
                }
            }
            else if (type == gamecontrollercontrol)
            {
                if (actions[i].gamecontroller1 && controls[control].value == *(int *)actions[i].gamecontroller1)
                {
                    C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                        " action has been unbound from the " BOLD("%s")" control.",
                        actions[i].action, controls[control].control);
                    *(int *)actions[i].gamecontroller1 = 0;
                }

                if (actions[i].gamecontroller2 && controls[control].value == *(int *)actions[i].gamecontroller2)
                {
                    C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                        " action has been unbound from the " BOLD("%s")" control.",
                        actions[i].action, controls[control].control);
                    *(int *)actions[i].gamecontroller2 = 0;
                }
            }
        }
}

void bind_cmd_func2(char *cmd, char *parms)
{
    int         i = 0;
    int         action = 0;
    char        parm1[128] = "";
    char        parm2[128] = "";
    const bool  mouselookcontrols = (keyboardmouselook || gamecontrollermouselook || mousemouselook != -1);

    if (sscanf(parms, "%127s %127[^\n]", parm1, parm2) <= 0)
    {
        i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
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
                            C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                actions[action].action, controls[i].control);
                            *(int *)actions[action].keyboard1 = 0;
                            M_SaveCVARs();
                        }

                        if (actions[action].keyboard2 && controls[i].value == *(int *)actions[action].keyboard2)
                        {
                            C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                actions[action].action, controls[i].control);
                            *(int *)actions[action].keyboard2 = 0;
                            M_SaveCVARs();
                        }

                        break;

                    case mousecontrol:
                        if (actions[action].mouse1 && controls[i].value == *(int *)actions[action].mouse1)
                        {
                            C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                actions[action].action, controls[i].control);
                            *(int *)actions[action].mouse1 = -1;
                            M_SaveCVARs();
                        }

                        break;

                    case gamecontrollercontrol:
                        if (actions[action].gamecontroller1 && controls[i].value == *(int *)actions[action].gamecontroller1)
                        {
                            C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                actions[action].action, controls[i].control);
                            *(int *)actions[action].gamecontroller1 = 0;
                            M_SaveCVARs();
                        }

                        if (actions[action].gamecontroller2 && controls[i].value == *(int *)actions[action].gamecontroller2)
                        {
                            C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                actions[action].action, controls[i].control);
                            *(int *)actions[action].gamecontroller2 = 0;
                            M_SaveCVARs();
                        }

                        break;

                    default:
                        break;
                }

                action++;
            }

            if (controls[i].type == keyboardcontrol && keyactionlist[controls[i].value])
            {
                C_Output(BOLD("\"%s\"") " has been unbound from the " BOLD("%s") " control.",
                    keyactionlist[controls[i].value], controls[i].control);
                keyactionlist[controls[i].value][0] = '\0';
            }
            else if (controls[i].type == mousecontrol && mouseactionlist[controls[i].value])
            {
                C_Output(BOLD("\"%s\"") " has been unbound from the " BOLD("%s") " control.",
                    mouseactionlist[controls[i].value], controls[i].control);
                mouseactionlist[controls[i].value][0] = '\0';
            }
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
                else if (controls[i].type == gamecontrollercontrol)
                {
                    if (actions[action].gamecontroller1 && controls[i].value == *(int *)actions[action].gamecontroller1)
                        C_Output(actions[action].action);
                    else if (actions[action].gamecontroller2 && controls[i].value == *(int *)actions[action].gamecontroller2)
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
                bool    bound = false;

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
                            C_UnbindDuplicates(action, keyboardcontrol, i);

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
                            C_UnbindDuplicates(action, mousecontrol, i);
                        }

                        break;

                    case gamecontrollercontrol:
                        if (actions[action].gamecontroller1)
                        {
                            if (actions[action].gamecontroller2
                                && *(int *)actions[action].gamecontroller1
                                && *(int *)actions[action].gamecontroller1 != controls[i].value)
                            {
                                if (*(int *)actions[action].gamecontroller2)
                                {
                                    *(int *)actions[action].gamecontroller2 = *(int *)actions[action].gamecontroller1;
                                    *(int *)actions[action].gamecontroller1 = controls[i].value;
                                }
                                else
                                    *(int *)actions[action].gamecontroller2 = controls[i].value;
                            }
                            else
                                *(int *)actions[action].gamecontroller1 = controls[i].value;

                            bound = true;
                            C_UnbindDuplicates(action, gamecontrollercontrol, i);
                        }

                        break;

                    default:
                        break;
                }

                M_SaveCVARs();

                if (bound)
                {
                    if (!nobindoutput)
                    {
                        if (strlen(controls[i].control) == 1)
                            C_Output("The " BOLD("%s") " action has been bound to the " BOLD("'%s'") " control.",
                                parm2, controls[i].control);
                        else
                            C_Output("The " BOLD("%s") " action has been bound to the " BOLD("%s") " control.",
                                parm2, controls[i].control);
                    }
                }
                else
                {
                    if (strlen(controls[i].control) == 1)
                        C_Warning(0, "The " BOLD("%s") " action can't be bound to the " BOLD("'%s'") " control.",
                            parm2, controls[i].control);
                    else
                        C_Warning(0, "The " BOLD("%s") " action can't be bound to the " BOLD("%s") " control.",
                            parm2, controls[i].control);

                    return;
                }
            }
            else
            {
                if (controls[i].type == keyboardcontrol)
                {
                    M_StringCopy(keyactionlist[controls[i].value], parm2, sizeof(keyactionlist[0]));
                    C_Output(BOLD("\"%s\"") " has been bound to the " BOLD("%s") " control.", parm2, controls[i].control);
                    M_SaveCVARs();
                }
                else if (controls[i].type == mousecontrol)
                {
                    M_StringCopy(mouseactionlist[controls[i].value], parm2, sizeof(mouseactionlist[0]));
                    C_Output(BOLD("\"%s\"") " has been bound to the " BOLD("%s") " control.", parm2, controls[i].control);
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

                if (actions[action].gamecontroller1)
                    *(int *)actions[action].gamecontroller1 = 0;

                if (actions[action].gamecontroller2)
                    *(int *)actions[action].gamecontroller2 = 0;

                break;
            }

            action++;
        }
    }
    else
    {
        C_Warning(0, BOLD("%s") " isn't a valid action or control.", parm1);
        return;
    }

    if (mouselookcontrols != (keyboardmouselook || gamecontrollermouselook || mousemouselook != -1))
    {
        if (gamestate == GS_LEVEL)
        {
            suppresswarnings = true;
            R_InitSkyMap();
            suppresswarnings = false;
        }

        R_InitColumnFunctions();
    }
}

//
// bindlist CCMD
//
static void C_DisplayBinds(const char *action, const int value, const controltype_t type, int *count)
{
    const int   tabs[3] = { 40, 130, 0 };

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
    const int   tabs[3] = { 40, 130, 0 };
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

        if (actions[i].gamecontroller1)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].gamecontroller1, gamecontrollercontrol, &count);

        if (actions[i].gamecontroller2)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].gamecontroller2, gamecontrollercontrol, &count);
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
    C_ClearConsole();
}

//
// cmdlist CCMD
//
static void cmdlist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[3] = { 40, 346, 0 };
    const int   columnwidth = tabs[1] - tabs[0] - 5;

    for (int i = 0, count = 0; *consolecmds[i].name; i++)
        if (consolecmds[i].type == CT_CCMD)
        {
            char    name[255];
            char    format[255];
            char    description[255];
            int     len;

            M_StringCopy(name, (english == english_american || M_StringCompare(consolecmds[i].altspelling, EMPTYVALUE) ?
                consolecmds[i].name : consolecmds[i].altspelling), sizeof(name));

            if (*parms && !wildcard(name, parms))
                continue;

            if (++count == 1)
                C_Header(tabs, cmdlist, CMDLISTHEADER);

            if (M_StringCompare(name, "map"))
                M_snprintf(format, sizeof(format), BOLD("map") " %s", (gamemission == doom ? MAPCMDFORMAT1 : MAPCMDFORMAT2));
            else
                M_snprintf(format, sizeof(format), BOLD("%s") " %s", name, consolecmds[i].format);

            len = (int)strlen(format);

            while (C_TextWidth(format, true, true) > columnwidth)
            {
                if (len >= 2 && format[len - 2] == ' ')
                {
                    format[len - 2] = '.';
                    format[len - 1] = '.';
                    format[len] = '.';
                    format[len + 1] = '\0';
                }
                else if (len >= 1)
                {
                    format[len - 1] = '.';
                    format[len] = '.';
                    format[len + 1] = '.';
                    format[len + 2] = '\0';
                }

                len--;
            }

            M_StringCopy(description, consolecmds[i].description, sizeof(description));

            if (english == english_international)
            {
                M_AmericanToInternationalEnglish(format);
                M_AmericanToInternationalEnglish(description);
            }

            C_TabbedOutput(tabs, "%i.\t%s\t%s", count, format, description);
        }
}

//
// condump CCMD
//
static bool condump_cmd_func1(char *cmd, char *parms)
{
    return (consolestrings > 1);
}

static void condump_cmd_func2(char *cmd, char *parms)
{
    char        consolefolder[MAX_PATH];
    char        filename[MAX_PATH];
    const char  *appdatafolder = M_GetAppDataFolder();
    FILE        *file;

    M_snprintf(consolefolder, sizeof(consolefolder), "%s" DIR_SEPARATOR_S DOOMRETRO_CONSOLEFOLDER, appdatafolder);
    M_MakeDirectory(consolefolder);

    if (!*parms)
    {
        int count = 0;

        M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "condump.txt", consolefolder);

        while (M_FileExists(filename))
        {
            char    *temp = commify(++count);

            M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "condump (%s).txt", consolefolder, temp);
            free(temp);
        }
    }
    else
        M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "%s%s",
            consolefolder, parms, (strchr(parms, '.') ? "" : ".txt"));

    if ((file = fopen(filename, "wt")))
    {
        char    *temp = commify((int64_t)consolestrings - 2);

        for (int i = 1; i < consolestrings - 1; i++)
            if (console[i].stringtype == dividerstring)
                fprintf(file, "%s\n", DIVIDERSTRING);
            else
            {
                char            *string = M_StringDuplicate(console[i].string);
                const int       len = (int)strlen(string);
                unsigned int    outpos = 0;
                int             tabcount = 0;
                unsigned char   prevletter = '\0';
                unsigned char   prevletter2 = '\0';

                if (!len)
                    continue;

                if (console[i].stringtype == warningstring)
                    fputs((console[i].line == 1 ? "/!\\ " : (string[0] == ' ' ? " " : "  ")), file);

                for (int inpos = 0; inpos < len; inpos++)
                {
                    const unsigned char letter = string[inpos];
                    unsigned char       nextletter = string[inpos + 1];

                    if (letter == '\t')
                    {
                        const unsigned int  tabstop = console[i].tabs[tabcount] / 6;

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
                    else if (letter == '\'')
                    {
                        if (prevletter == '\0' || prevletter == ' ' || prevletter == '\t' || prevletter == '('
                            || prevletter == '[' || prevletter == '{' || prevletter == '<' || prevletter == '"'
                            || ((prevletter == BOLDTOGGLECHAR || prevletter == ITALICSTOGGLECHAR)
                                && prevletter2 != '.' && nextletter != '.'))
                            fputc(145, file);
                        else
                            fputc(146, file);

                        outpos++;
                    }
                    else if (letter == '"')
                    {
                        if (prevletter == '\0' || prevletter == ' ' || prevletter == '\t' || prevletter == '('
                            || prevletter == '[' || prevletter == '{' || prevletter == '<' || prevletter == '\''
                            || ((prevletter == BOLDTOGGLECHAR || prevletter == ITALICSTOGGLECHAR)
                                && prevletter2 != '.' && nextletter != '.'))
                            fputc(147, file);
                        else
                            fputc(148, file);

                        outpos++;
                    }
                    else if (letter != '\n' && letter != BOLDTOGGLECHAR && letter != ITALICSTOGGLECHAR)
                    {
                        fputc(letter, file);
                        outpos++;
                    }

                    prevletter2 = prevletter;
                    prevletter = letter;
                }

                if (console[i].stringtype == playermessagestring)
                {
                    char    buffer[9];

                    for (unsigned int spaces = 0; spaces < 92 - outpos; spaces++)
                        fputc(' ', file);

                    M_StringCopy(buffer, C_CreateTimeStamp(i), sizeof(buffer));

                    if (strlen(buffer) == 7)
                        fputc(' ', file);

                    fputs(buffer, file);
                }

                fputc('\n', file);
                free(string);
            }

        fclose(file);

        C_Output("Dumped %s lines from the console to " BOLD("%s") ".", temp, filename);
        free(temp);
    }
    else
        C_Warning(0, BOLD("%s") " couldn't be created.", filename);
}

//
// cvarlist CCMD
//
static void cvarlist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[3] = { 40, 209, 328 };

    for (int i = 0, count = 0; *consolecmds[i].name; i++)
        if (consolecmds[i].type == CT_CVAR)
        {
            char    name[255];
            char    description[255];

            M_StringCopy(name, (english == english_american || M_StringCompare(consolecmds[i].altspelling, EMPTYVALUE) ?
                consolecmds[i].name : consolecmds[i].altspelling), sizeof(name));

            if (*parms && !wildcard(name, parms))
                continue;

            if (++count == 1)
                C_Header(tabs, cvarlist, CVARLISTHEADER);

            M_StringCopy(description, consolecmds[i].description, sizeof(description));

            if (english == english_international)
                M_AmericanToInternationalEnglish(description);

            if (M_StringCompare(name, stringize(ammo)))
            {
                if (gamestate == GS_LEVEL)
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i") "\t%s",
                        count, name, viewplayer->ammo[weaponinfo[viewplayer->readyweapon].ammotype], description);
                else
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i") "\t%s",
                        count, name, ammo_default, description);
            }
            else if (M_StringCompare(name, stringize(armor)))
            {
                if (gamestate == GS_LEVEL)
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i%%") "\t%s",
                        count, name, viewplayer->armorpoints, description);
                else
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i%%") "\t%s",
                        count, name, armor_default, description);
            }
            else if (M_StringCompare(name, stringize(armortype)))
            {
                if (gamestate == GS_LEVEL)
                {
                    char    *temp = C_LookupAliasFromValue(viewplayer->armortype, ARMORTYPEVALUEALIAS);

                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s") "\t%s",
                        count, name, temp, description);
                    free(temp);
                }
                else
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("none") "\t%s",
                        count, name, description);
            }
            else if (M_StringCompare(name, stringize(health)))
            {
                if (gamestate == GS_LEVEL)
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i%%") "\t%s",
                        count, name, viewplayer->health, description);
                else
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i%%") "\t%s",
                        count, name, health_default, description);
            }
            else if (M_StringCompare(name, stringize(weapon)))
            {
                if (gamestate == GS_LEVEL)
                {
                    char    *temp = C_LookupAliasFromValue(viewplayer->readyweapon, WEAPONVALUEALIAS);

                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s") "\t%s",
                        count, name, temp, description);
                    free(temp);
                }
                else
                {
                    char    *temp = C_LookupAliasFromValue(weapon_default, WEAPONVALUEALIAS);

                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s") "\t%s",
                        count, name, temp, description);
                    free(temp);
                }
            }
            else if (M_StringCompare(name, stringize(r_fov)))
            {
                C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i") "\xB0\t%s",
                    count, name, *(int *)consolecmds[i].variable, description);
            }
            else if (consolecmds[i].flags & CF_BOOLEAN)
            {
                char    *temp = C_LookupAliasFromValue(*(bool *)consolecmds[i].variable, consolecmds[i].aliases);

                C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s") "\t%s",
                    count, name, temp, description);
                free(temp);
            }
            else if ((consolecmds[i].flags & CF_INTEGER) && (consolecmds[i].flags & CF_PERCENT))
                C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i%%") "\t%s",
                    count, name, *(int *)consolecmds[i].variable, description);
            else if (consolecmds[i].flags & CF_INTEGER)
            {
                char    *temp = C_LookupAliasFromValue(*(int *)consolecmds[i].variable, consolecmds[i].aliases);

                C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s") "\t%s",
                    count, name, temp, description);
                free(temp);
            }
            else if (consolecmds[i].flags & CF_FLOAT)
            {
                if (consolecmds[i].flags & CF_PERCENT)
                {
                    char    *temp = striptrailingzero(*(float *)consolecmds[i].variable, 1);

                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s%%") "\t%s",
                        count, name, temp, description);
                    free(temp);
                }
                else if (M_StringCompare(consolecmds[i].name, stringize(r_gamma)))
                {
                    char    buffer[128];
                    int     len;

                    M_snprintf(buffer, sizeof(buffer), "%.2f", *(float *)consolecmds[i].variable);
                    len = (int)strlen(buffer);

                    if (len >= 2 && buffer[len - 1] == '0' && buffer[len - 2] == '0')
                        buffer[len - 1] = '\0';

                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s") "\t%s",
                        count, name, buffer, description);
                }
                else
                {
                    char    *temp = striptrailingzero(*(float *)consolecmds[i].variable, 1);

                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s") "\t%s",
                        count, name, temp, description);
                    free(temp);
                }
            }
            else if (consolecmds[i].flags & CF_STRING)
                C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s%.14s%s%s") "\t%s",
                    count,
                    name,
                    (M_StringCompare(name, stringize(version)) ? "" : "\""),
                    *(char **)consolecmds[i].variable,
                    (strlen(*(char **)consolecmds[i].variable) > 14 ? "..." : ""),
                    (M_StringCompare(name, stringize(version)) ? "" : "\""),
                    description);
            else if (consolecmds[i].flags & CF_TIME)
            {
                int tics = *(int *)consolecmds[i].variable / TICRATE;
                int hours = tics / 3600;
                int minutes = ((tics %= 3600)) / 60;
                int seconds = tics % 60;

                if (!hours)
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%02i:%02i") "\t%s",
                        count, name, minutes, seconds, description);
                else
                    C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%i:%02i:%02i") "\t%s",
                        count, name, hours, minutes, seconds, description);
            }
            else if (consolecmds[i].flags & CF_OTHER)
                C_TabbedOutput(tabs, "%i.\t" BOLD("%s") "\t" BOLD("%s") "\t%s",
                    count, name, *(char **)consolecmds[i].variable, description);
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
    }
    else
    {
        char    filename[MAX_PATH];
        char    strparm[512] = "";
        FILE    *file;

        if (strchr(parms, '.'))
            M_StringCopy(filename, parms, sizeof(filename));
        else
            M_snprintf(filename, sizeof(filename), "%s.cfg", parms);

        if (!(file = fopen(filename, "rt")))
        {
            C_Warning(0, BOLD("%s") " couldn't be opened.", parms);
            return;
        }

        while (fgets(strparm, sizeof(strparm), file))
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
    viewplayer->cheated++;
    stat_cheated = SafeAdd(stat_cheated, 1);
    M_SaveCVARs();
}

//
// fastmonsters CCMD
//
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
        mobj_t  *mo = viewplayer->mo;

        C_Output(s_STSTR_FON);
        HU_SetPlayerMessage(s_STSTR_FON, false, false);
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();

        mo->momx = 0;
        mo->momy = 0;
        mo->momz = 0;
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
static bool give_cmd_func1(char *cmd, char *parms)
{
    bool    result = false;
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
        return true;

    if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything")
        || M_StringCompare(parm, "health") || M_StringCompare(parm, "fullhealth")
        || M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons")
        || M_StringCompare(parm, "ammo") || M_StringCompare(parm, "fullammo")
        || M_StringCompare(parm, "ammunition") || M_StringCompare(parm, "fullammunition")
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
                    || (sscanf(parm, "%10i", &num) == 1 && num == mobjinfo[i].doomednum && num != -1)))
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

    if (!*parm || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMEWARNING, "you", "are");
            else
                C_Warning(0, NOGAMEWARNING, playername, "is");
        }
    }
    else
    {
        if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything"))
        {
            bool    result = false;

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

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You were given everything.");
                else
                    C_PlayerMessage("%s was given everything.", playername);

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

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You were given full health.");
                else
                    C_PlayerMessage("%s was given full health.", playername);

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

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You were given all of your weapons.");
                else
                    C_PlayerMessage("%s was given all of %s weapons.", playername, pronoun(possessive));

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all of your weapons.");
                else
                    C_Warning(0, "%s already has all of %s weapons.", playername, pronoun(possessive));

                free(parm);
                return;
            }
        }
        else if (M_StringCompare(parm, "ammo") || M_StringCompare(parm, "fullammo")
                || M_StringCompare(parm, "ammunition") || M_StringCompare(parm, "fullammunition"))
        {
            if (P_GiveFullAmmo())
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You were given full ammo for all of your weapons.");
                else
                    C_PlayerMessage("%s was given full ammo for all of %s weapons.", playername, pronoun(possessive));

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have full ammo for all of your weapons.");
                else
                    C_Warning(0, "%s already has full ammo for all of %s weapons.", playername, pronoun(possessive));

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

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You were given full %s.",
                        (english == english_american ? "armor" : "armour"));
                else
                    C_PlayerMessage("%s was given full %s.",
                        playername, (english == english_american ? "armor" : "armour"));

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have full %s.",
                        (english == english_american ? "armor" : "armour"));
                else
                    C_Warning(0, "%s already has full %s.",
                        playername, (english == english_american ? "armor" : "armour"));

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

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You were given all keycards and skull keys.");
                else
                    C_PlayerMessage("%s was given all keycards and skull keys.", playername);

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all keycards and skull keys.");
                else
                    C_Warning(0, "%s already has all keycards and skull keys.", playername);

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

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You were given all keycards.");
                else
                    C_PlayerMessage("%s was given all keycards.", playername);

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all keycards.");
                else
                    C_Warning(0, "%s already has all keycards.", playername);

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

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You were given all skull keys.");
                else
                    C_PlayerMessage("%s was given all skull keys.", playername);

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all skull keys.");
                else
                    C_Warning(0, "%s already has all skull keys.", playername);

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
            viewplayer->pendingweapon = wp_pistol;

            if (M_StringCompare(playername, playername_default))
                C_PlayerMessage("You were given a pistol.");
            else
                C_PlayerMessage("%s was given a pistol.", playername);

            C_HideConsole();
            free(parm);

            return;
        }
        else
        {
            for (int i = 0, num = -1; i < NUMMOBJTYPES; i++)
            {
                bool    result = false;
                char    *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
                char    *temp2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char    *temp3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);

                if ((mobjinfo[i].flags & MF_SPECIAL)
                    && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, temp2))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, temp3))
                        || (sscanf(parm, "%10i", &num) == 1 && num == mobjinfo[i].doomednum && num != -1)))
                {
                    if (gamemode != commercial && (i == MT_SUPERSHOTGUN || i == MT_MEGA))

                        C_Warning(0, "%s can't be given %s %s in " ITALICS("%s."),
                            (M_StringCompare(playername, playername_default) ? "You" : playername),
                            (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1, gamedescription);
                    else if (gamemode == shareware && (i == MT_MISC7 || i == MT_MISC8 || i == MT_MISC9
                        || i == MT_MISC20 || i == MT_MISC21 || i == MT_MISC25 || i == MT_MISC28))
                        C_Warning(0, "%s can't be given %s %s in " ITALICS("%s."),
                            (M_StringCompare(playername, playername_default) ? "You" : playername),
                            (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1, gamedescription);
                    else
                    {
                        bool    old_freeze = freeze;
                        mobj_t  *thing = P_SpawnMobj(viewx, viewy, viewz, i);

                        freeze = false;

                        if (P_TouchSpecialThing(thing, viewplayer->mo, false, false))
                        {
                            if (thing->type == MT_MISC0 || thing->type == MT_MISC1)
                            {
                                if (M_StringCompare(playername, playername_default))
                                    C_PlayerMessage("You were given %s.", mobjinfo[i].name1);
                                else
                                    C_PlayerMessage("%s was given %s.", playername, mobjinfo[i].name1);
                            }
                            else
                            {
                                if (M_StringCompare(playername, playername_default))
                                    C_PlayerMessage("You were given %s %s.",
                                        (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                                else
                                    C_PlayerMessage("%s was given %s %s.",
                                        playername, (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                            }

                            C_HideConsole();
                        }
                        else
                        {
                            C_Warning(0, "%s can't be given another %s.",
                                (M_StringCompare(playername, playername_default) ? "You" : playername), mobjinfo[i].name1);

                            P_RemoveMobj(thing);
                        }

                        freeze = old_freeze;
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
    if (!ShellExecute(NULL, "open", DOOMRETRO_WIKIURL, NULL, NULL, SW_SHOWNORMAL))
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__HAIKU__)
    if (!system("xdg-open " DOOMRETRO_WIKIURL))
#elif defined(__APPLE__)
    if (!system("open " DOOMRETRO_WIKIURL))
#endif
        C_Warning(0, "The " ITALICS(DOOMRETRO_WIKINAME) " couldn't be opened.");
}

//
// if CCMD
//
static bool match(bool value, char *toggle)
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
        return;
    }

    M_StripQuotes(parm1);

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(parm1, consolecmds[i].name))
        {
            bool    condition = false;

            M_StripQuotes(parm2);

            if (consolecmds[i].type == CT_CVAR)
            {
                if (consolecmds[i].flags & (CF_BOOLEAN | CF_INTEGER))
                {
                    int value = C_LookupValueFromAlias(parm2, consolecmds[i].aliases);

                    if (value != INT_MIN || sscanf(parms, "%10i", &value) == 1)
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
bool            massacre;

static bool kill_cmd_func1(char *cmd, char *parms)
{
    bool    result = false;
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
        return true;

    killcmdmobj = NULL;

    if (M_StringCompare(cmd, "explode"))
        result = (M_StringCompare(parm, "barrel") || M_StringCompare(parm, "barrels")
            || M_StringCompare(parm, "missile") || M_StringCompare(parm, "missiles"));
    else if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        result = (viewplayer->health > 0);
    else if (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters") || M_StringCompare(parm, "all")
        || M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
        || M_StringCompare(parm, "friendlymonster") || M_StringCompare(parm, "friendlymonsters")
        || M_StringCompare(parm, "missile") || M_StringCompare(parm, "missiles")
        || M_StringCompare(parm, "item") || M_StringCompare(parm, "items")
        || M_StringCompare(parm, "decoration") || M_StringCompare(parm, "decorations")
        || M_StringCompare(parm, "corpse") || M_StringCompare(parm, "corpses")
        || M_StringCompare(parm, "blood") || M_StringCompare(parm, "bloodsplat") || M_StringCompare(parm, "bloodsplats")
        || M_StringCompare(parm, "everything"))
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
                    M_StringReplaceAll(parm, "all", "", false);

                killcmdtype = mobjinfo[i].doomednum;

                if (killcmdtype >= 0
                    && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                        || (*mobjinfo[i].plural1 && M_StringCompare(parm, temp2))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, temp3))
                        || (*mobjinfo[i].plural2 && M_StringCompare(parm, temp4))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, temp5))
                        || (*mobjinfo[i].plural3 && M_StringCompare(parm, temp6))
                        || (sscanf(parm, "%10i", &num) == 1 && num == killcmdtype && num != -1)))
                {
                    if (killcmdtype == WolfensteinSS && !allowwolfensteinss)
                        result = false;
                    else
                        result = ((mobjinfo[i].flags & MF_SHOOTABLE)
                            || (mobjinfo[i].flags & MF_SPECIAL)
                            || (mobjinfo[i].flags2 & MF2_DECORATION));
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

static void kill_cmd_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_Output(BOLD("%s") " %s", cmd, consolecmds[i].format);
    }
    else
    {
        char    buffer[1024];
        char    *killed = (M_StringCompare(cmd, "explode") ? "exploded" : (M_StringCompare(cmd, "remove") ? "removed" : "killed"));

        if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        {
            massacre = true;

            viewplayer->damagecount = MIN(viewplayer->health, 100);
            viewplayer->health = 0;
            viewplayer->mo->health = 0;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            viewplayer->attacker = NULL;

            if (viewplayer->fixedcolormap == INVERSECOLORMAP)
                viewplayer->fixedcolormap = 0;

            viewplayer->mo->flags2 |= MF2_MASSACRE;
            P_KillMobj(viewplayer->mo, NULL, viewplayer->mo, false);

            if (M_StringCompare(playername, playername_default))
                M_snprintf(buffer, sizeof(buffer), "You killed yourself.");
            else
                M_snprintf(buffer, sizeof(buffer), "%s killed %s.", playername, pronoun(reflexive));

            C_Output(buffer);
            C_HideConsole();
            HU_SetPlayerMessage(buffer, false, false);
            message_dontfuckwithme = true;
        }
        else
        {
            const bool  friends = (M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
                            || M_StringCompare(parm, "friendlymonster") || M_StringCompare(parm, "friendlymonsters"));
            const bool  enemies = (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters"));
            const bool  all = M_StringCompare(parm, "all");
            int         kills = 0;

            if (friends || enemies || all)
            {
                massacre = true;

                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
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
                                    stat_monsterskilled[MT_PAIN] = SafeAdd(stat_monsterskilled[MT_PAIN], 1);
                                    viewplayer->killcount++;
                                    stat_monsterskilled_total = SafeAdd(stat_monsterskilled_total, 1);
                                    kills++;
                                }
                                else if ((flags & MF_SHOOTABLE) && type != MT_PLAYER && type != MT_BARREL && (type != MT_HEAD || !hacx))
                                {
                                    thing->flags2 |= MF2_MASSACRE;
                                    P_DamageMobj(thing, viewplayer->mo, viewplayer->mo, thing->health, false, false);

                                    if (r_corpses_moreblood && !(flags & MF_NOBLOOD) && type != MT_SKULL && type != MT_SHADOWS)
                                        P_SpawnMoreBlood(thing);

                                    kills++;
                                }
                            }
                        }
                    }

                if (kills)
                {
                    char    *temp = commify(kills);

                    if (M_StringCompare(playername, playername_default))
                    {
                        if (kills == 1)
                            M_snprintf(buffer, sizeof(buffer), "You %s the only monster %s this map.",
                                killed, (viewplayer->killcount == 1 ? "in" : "left in"));
                        else
                            M_snprintf(buffer, sizeof(buffer), "You %s the %s monsters %s this map.",
                                killed, temp, (viewplayer->killcount == kills ? "in" : "left in"));
                    }
                    else
                    {
                        if (kills == 1)
                            M_snprintf(buffer, sizeof(buffer), "%s %s the only monster %s this map.",
                                playername, killed, (viewplayer->killcount == 1 ? "in" : "left in"));
                        else
                            M_snprintf(buffer, sizeof(buffer), "%s %s the %s monsters %s this map.",
                                playername, killed, temp, (viewplayer->killcount == kills ? "in" : "left in"));

                        buffer[0] = toupper(buffer[0]);
                    }

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
                    C_Warning(0, "There are no monsters %s %s.", (viewplayer->killcount ? "left to" : "to"), cmd);
            }
            else if (M_StringCompare(parm, "missile") || M_StringCompare(parm, "missiles"))
            {
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                        if (thing->flags2 & MF2_MONSTERMISSILE)
                        {
                            P_ExplodeMissile(thing);
                            kills++;
                        }

                if (kills)
                {
                    char    *temp = commify(kills);

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), "You %s %s missile%s.",
                            killed, (kills == 1 ? "one" : temp), (kills == 1 ? "" : "s"));
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s %s missile%s.",
                            playername, killed, (kills == 1 ? "one" : temp), (kills == 1 ? "" : "s"));

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
                    C_Warning(0, "There are no missiles to %s.", cmd);
            }
            else if (M_StringCompare(parm, "item") || M_StringCompare(parm, "items"))
            {
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                        if (thing->flags & MF_SPECIAL)
                        {
                            P_SpawnMobj(thing->x, thing->y, thing->z, MT_IFOG);
                            S_StartSound(thing, sfx_itmbk);
                            P_RemoveMobj(thing);
                            kills++;
                        }

                if (kills)
                {
                    char    *temp = commify(kills);

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), "You %s %s item%s.",
                            killed, (kills == 1 ? "one" : temp), (kills == 1 ? "" : "s"));
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s %s item%s.",
                            playername, killed, (kills == 1 ? "one" : temp), (kills == 1 ? "" : "s"));

                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                    free(temp);
                }
                else
                    C_Warning(0, "There are no items to %s.", cmd);
            }
            else if (M_StringCompare(parm, "decoration") || M_StringCompare(parm, "decorations"))
            {
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                        if (thing->flags2 & MF2_DECORATION)
                        {
                            P_SpawnMobj(thing->x, thing->y, thing->z, MT_TFOG);
                            S_StartSound(thing, sfx_telept);
                            P_RemoveMobj(thing);
                            kills++;
                        }

                if (kills)
                {
                    char    *temp = commify(kills);

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), "You %s %s decoration%s.",
                            killed, (kills == 1 ? "one" : temp), (kills == 1 ? "" : "s"));
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s %s decoration%s.",
                            playername, killed, (kills == 1 ? "one" : temp), (kills == 1 ? "" : "s"));

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
                    C_Warning(0, "There are no decorations to %s.", cmd);
            }
            else if (M_StringCompare(parm, "everything"))
            {
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                    {
                        const int   flags = thing->flags;
                        const int   flags2 = thing->flags2;

                        if (((flags & MF_SHOOTABLE) && !thing->player) || (flags & MF_CORPSE) || (flags2 & MF2_DECORATION))
                        {
                            P_SpawnMobj(thing->x, thing->y, thing->z, MT_TFOG);
                            S_StartSound(thing, sfx_telept);
                            P_RemoveMobj(thing);
                            kills++;
                        }
                        else if ((flags & MF_SPECIAL) || (flags2 & MF2_MONSTERMISSILE))
                        {
                            P_SpawnMobj(thing->x, thing->y, thing->z, MT_IFOG);
                            S_StartSound(thing, sfx_itmbk);
                            P_RemoveMobj(thing);
                            kills++;
                        }
                        else if (flags2 & MF2_MONSTERMISSILE)
                        {
                            P_SpawnMobj(thing->x, thing->y, thing->z, MT_IFOG);
                            S_StartSound(thing, sfx_itmbk);
                            P_RemoveMobj(thing);
                            kills++;
                        }
                    }

                P_RemoveBloodSplats();

                if (kills)
                {
                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), "You %s everything.", killed);
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s everything.", playername, killed);

                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                    viewplayer->cheated++;
                    stat_cheated = SafeAdd(stat_cheated, 1);
                    M_SaveCVARs();
                }
                else
                    C_Warning(0, "There is nothing left to %s.", cmd);
            }
            else if (M_StringCompare(parm, "corpse") || M_StringCompare(parm, "corpses"))
            {
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                        if (thing->flags & MF_CORPSE)
                        {
                            P_SpawnMobj(thing->x, thing->y, thing->z, MT_TFOG);
                            S_StartSound(thing, sfx_telept);
                            P_RemoveMobj(thing);
                            kills++;
                        }

                if (kills)
                {
                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), "You %s all corpses.", killed);
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s all corpses.", playername, killed);

                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                }
                else
                    C_Warning(0, "There are no corpses to %s.", cmd);
            }
            else if (M_StringCompare(parm, "blood") || M_StringCompare(parm, "bloodsplat") || M_StringCompare(parm, "bloodsplats"))
            {
                if (r_bloodsplats_total)
                {
                    P_RemoveBloodSplats();

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), "You %s all blood splats.", killed);
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s all blood splats.", playername, killed);

                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    message_dontfuckwithme = true;
                }
                else
                    C_Warning(0, "There are no blood splats to %s.", cmd);
            }
            else if (killcmdmobj)
            {
                char    *temp = sentencecase(parm);

                killcmdmobj->flags2 |= MF2_MASSACRE;
                P_DamageMobj(killcmdmobj, viewplayer->mo, viewplayer->mo, killcmdmobj->health, false, false);

                if (M_StringCompare(playername, playername_default))
                    M_snprintf(buffer, sizeof(buffer), "You %s %s.", killed, temp);
                else
                    M_snprintf(buffer, sizeof(buffer), "%s %s %s.", playername, killed, temp);

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

                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                        if (type == thing->type)
                        {
                            if (type == MT_PAIN)
                            {
                                if (thing->health > 0)
                                {
                                    A_Fall(thing, NULL, NULL);
                                    P_SetMobjState(thing, S_PAIN_DIE6);
                                    viewplayer->mobjcount[MT_PAIN]++;
                                    stat_monsterskilled[MT_PAIN] = SafeAdd(stat_monsterskilled[MT_PAIN], 1);
                                    viewplayer->killcount++;
                                    stat_monsterskilled_total = SafeAdd(stat_monsterskilled_total, 1);
                                    kills++;
                                }
                            }
                            else if ((thing->flags & MF_SHOOTABLE) && thing->health > 0)
                            {
                                thing->flags2 |= MF2_MASSACRE;
                                P_DamageMobj(thing, viewplayer->mo, viewplayer->mo, thing->health, false, false);
                                kills++;
                            }
                            else if (thing->flags & MF_SPECIAL)
                            {
                                P_SpawnMobj(thing->x, thing->y, thing->z, MT_IFOG);
                                S_StartSound(thing, sfx_itmbk);
                                P_RemoveMobj(thing);
                                kills++;
                            }
                            else if (thing->flags2 & MF2_DECORATION)
                            {
                                P_SpawnMobj(thing->x, thing->y, thing->z, MT_TFOG);
                                S_StartSound(thing, sfx_itmbk);
                                P_RemoveMobj(thing);
                                kills++;
                            }
                        }

                if (kills)
                {
                    char    *temp = commify(kills);

                    if (M_StringCompare(playername, playername_default))
                    {
                        if (kills == 1)
                            M_snprintf(buffer, sizeof(buffer), "You %s the only %s %s this map.",
                                killed,
                                mobjinfo[type].name1,
                                (viewplayer->mobjcount[type] == 1 ? "in" : "left in"));
                        else
                            M_snprintf(buffer, sizeof(buffer), "You %s all %s %s %s this map.",
                                killed,
                                temp,
                                mobjinfo[type].plural1,
                                (viewplayer->mobjcount[type] == kills ? "in" : "left in"));
                    }
                    else
                    {
                        if (kills == 1)
                            M_snprintf(buffer, sizeof(buffer), "%s %s the only %s %s this map.",
                                playername,
                                killed,
                                mobjinfo[type].name1,
                                (viewplayer->mobjcount[type] == 1 ? "in" : "left in"));
                        else
                            M_snprintf(buffer, sizeof(buffer), "%s %s all %s %s %s this map.",
                                playername,
                                killed,
                                temp,
                                mobjinfo[type].plural1,
                                (viewplayer->mobjcount[type] == kills ? "in" : "left in"));
                    }

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
                        if (killcmdtype >= ArchVile && killcmdtype <= MonsterSpawner)
                            C_Warning(0, "There are no %s in " ITALICS("%s."), mobjinfo[type].plural1, gamedescription);
                        else if (gamemode == shareware && (killcmdtype == Cyberdemon || killcmdtype == SpiderMastermind))
                            C_Warning(0, "There are no %s in " ITALICS("%s."), mobjinfo[type].plural1, gamedescription);
                    }
                    else
                        C_Warning(0, "There are no %s %s %s.", mobjinfo[type].plural1,
                            (viewplayer->mobjcount[type] ? "left to" : "to"), cmd);
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
    if (!ShellExecute(NULL, "open", DOOMRETRO_LICENSEURL, NULL, NULL, SW_SHOWNORMAL))
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__HAIKU__)
    if (!system("xdg-open " DOOMRETRO_LICENSEURL))
#elif defined(__APPLE__)
    if (!system("open " DOOMRETRO_LICENSEURL))
#endif
        C_Warning(0, "The " ITALICS(DOOMRETRO_LICENSE) " couldn't be displayed.");
}

//
// load CCMD
//
static void load_cmd_func2(char *cmd, char *parms)
{
    char    buffer[1024];

    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
        return;
    }

    M_snprintf(buffer, sizeof(buffer), "%s%s%s",
        (M_StringStartsWith(parms, savegamefolder) ? "" : savegamefolder), parms, (M_StringEndsWith(parms, ".save") ? "" : ".save"));
    G_LoadGame(buffer);
}

//
// map CCMD
//
static bool map_cmd_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;
    else
    {
        bool    result = false;
        char    *temp1 = removenonalpha(parms);
        char    *parm = uppercase(temp1);

        mapcmdepisode = 0;
        mapcmdmap = 0;
        speciallumpname[0] = '\0';

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
                mapcmdmap = M_BigRandomIntNoRepeat(1, (gamemission == pack_nerve ? 8 : 30), gamemap);
                M_snprintf(mapcmdlump, sizeof(mapcmdlump), "MAP%02i", mapcmdmap);
                result = true;
            }
            else
            {
                mapcmdepisode = (gamemode == shareware || chex ? 1 :
                    M_BigRandomIntNoRepeat(1, (gamemode == retail ? (sigil ? 5 : 4) : 3), gameepisode));
                mapcmdmap = M_BigRandomIntNoRepeat(1, (chex ? 5 : 8), gamemap);

                if (mapcmdepisode == 1 && mapcmdmap == 4 && (M_BigRandom() & 1) && gamemode != shareware && !chex && !FREEDOOM1)
                    M_StringCopy(mapcmdlump, "E1M4B", sizeof(mapcmdlump));
                else if (mapcmdepisode == 1 && mapcmdmap == 8 && (M_BigRandom() & 1) && gamemode != shareware && !chex && !FREEDOOM1)
                    M_StringCopy(mapcmdlump, "E1M8B", sizeof(mapcmdlump));
                else
                    M_snprintf(mapcmdlump, sizeof(mapcmdlump), "E%iM%i", mapcmdepisode, mapcmdmap);

                result = true;
            }
        }
        else if (M_StringCompare(parm, "E1M4B"))
        {
            mapcmdepisode = 1;
            mapcmdmap = 4;
            M_StringCopy(speciallumpname, "E1M4B", sizeof(speciallumpname));
            M_StringCopy(mapcmdlump, "E1M4B", sizeof(mapcmdlump));
            result = (gamemission == doom && gamemode != shareware && !chex && !FREEDOOM1);
        }
        else if (M_StringCompare(parm, "E1M8B"))
        {
            mapcmdepisode = 1;
            mapcmdmap = 8;
            M_StringCopy(speciallumpname, "E1M8B", sizeof(speciallumpname));
            M_StringCopy(mapcmdlump, "E1M8B", sizeof(mapcmdlump));
            result = (gamemission == doom && gamemode != shareware && !chex && !FREEDOOM1);
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
                        if ((sscanf(parm, "E%1iM0%1i", &mapcmdepisode, &mapcmdmap) == 2
                            || sscanf(parm, "E%1iM%2i", &mapcmdepisode, &mapcmdmap) == 2)
                            && ((mapcmdepisode == 1 && BTSXE1) || (mapcmdepisode == 2 && BTSXE2)))
                        {
                            char    lump[6];

                            M_snprintf(lump, sizeof(lump), "MAP%02i", mapcmdmap);
                            result = (W_CheckMultipleLumps(lump) == 2);
                        }
                    }
                }
            }
            else if (sscanf(parm, "E%1iM%i", &mapcmdepisode, &mapcmdmap) == 2)
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
                char    wadname[MAX_PATH];
                bool    replaced;
                bool    pwad;
                char    mapinfoname[128];
                char    *temp2 = uppercase(lumpinfo[i]->name);

                M_StringCopy(mapcmdlump, temp2, sizeof(mapcmdlump));
                free(temp2);

                if (gamemode == commercial)
                {
                    mapcmdepisode = 1;

                    if (sscanf(mapcmdlump, "MAP0%1i", &mapcmdmap) != 1 && sscanf(mapcmdlump, "MAP%2i", &mapcmdmap) != 1)
                        continue;
                }
                else
                {
                    if (sscanf(mapcmdlump, "E%1iM%1iB", &mapcmdepisode, &mapcmdmap) == 2 && gamemode != shareware)
                        M_StringCopy(speciallumpname, mapcmdlump, sizeof(speciallumpname));
                    else if (sscanf(mapcmdlump, "E%1iM%i", &mapcmdepisode, &mapcmdmap) != 2)
                        continue;
                }

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
        C_Output(BOLD("%s") " %s", cmd, (gamemission == doom ? MAPCMDFORMAT1 : MAPCMDFORMAT2));
        return;
    }

    samelevel = (gameepisode == mapcmdepisode && gamemap == mapcmdmap && !*speciallumpname);
    M_snprintf(buffer, sizeof(buffer), (samelevel ? "Restarting %s..." : "Warping to %s..."), mapcmdlump);
    C_Output(buffer);
    HU_SetPlayerMessage(buffer, false, false);
    message_dontfuckwithme = true;

    gameepisode = mapcmdepisode;

    if (gamemission == doom)
    {
        episode = gameepisode;
        EpiDef.laston = episode - 1;
    }

    gamemap = mapcmdmap;

    quicksaveslot = -1;

    if (gamestate == GS_LEVEL)
    {
        idclevtics = TICRATE;
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
    char    *pos = strchr(title, ':');

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
    const int   tabs[3] = { 40, 93, 370 };
    int         count = 0;
    char        (*maps)[256] = malloc(numlumps * sizeof(char *));

    C_Header(tabs, maplist, MAPLISTHEADER);

    // search through lumps for maps
    for (int i = numlumps - 1; i >= 0; i--)
    {
        int     ep = 1;
        int     map = 1;
        char    lump[9];
        char    wadname[MAX_PATH];
        bool    replaced;
        bool    pwad;
        char    mapinfoname[128];
        char    *temp = uppercase(lumpinfo[i]->name);

        M_StringCopy(lump, temp, sizeof(lump));
        free(temp);

        if (gamemode == commercial)
        {
            if (sscanf(lump, "MAP0%1i", &map) != 1 && sscanf(lump, "MAP%2i", &map) != 1)
                continue;
        }
        else
        {
            if (M_StringCompare(lump, "E1M4B") || M_StringCompare(lump, "E1M8B"))
            {
                if (gamemode == shareware || FREEDOOM1 || chex)
                    continue;
            }
            else if (sscanf(lump, "E%1iM%i", &ep, &map) != 2)
                continue;
        }

        M_StringCopy(wadname, leafname(lumpinfo[i]->wadfile->path), sizeof(wadname));
        replaced = (W_CheckMultipleLumps(lump) > 1 && !chex && !FREEDOOM);
        pwad = (lumpinfo[i]->wadfile->type == PWAD);
        M_StringCopy(mapinfoname, P_GetMapName(--ep * 10 + (--map) + 1), sizeof(mapinfoname));

        switch (gamemission)
        {
            case doom:
                if (!replaced || pwad)
                {
                    if (M_StringCompare(lump, "E1M4B"))
                        temp = titlecase(s_HUSTR_E1M4B);
                    else if (M_StringCompare(lump, "E1M8B"))
                        temp = titlecase(s_HUSTR_E1M8B);
                    else
                        temp = titlecase(*mapinfoname ? mapinfoname : *mapnames[ep * 9 + map]);

                    removemapnum(temp);

                    if (FREEDOOM1 && strlen(lump) == 4)
                        lump[0] = 'C';

                    M_snprintf(maps[count++], sizeof(maps[0]), "%s\t" ITALICS("%s") "\t%s", lump,
                        (replaced && dehcount == 1 && !*mapinfoname ? "-" : temp), wadname);
                    free(temp);
                }

                break;

            case doom2:
                if ((!M_StringCompare(wadname, "NERVE.WAD") && (!replaced || pwad || nerve)) || hacx || harmony)
                {
                    if (BTSX)
                    {
                        if (!M_StringCompare(wadname, "DOOM2.WAD"))
                        {
                            temp = titlecase(M_StringReplace(*mapnames2[map], ": ", "\t" ITALICSTOGGLE));
                            removemapnum(temp);
                            M_snprintf(maps[count++], sizeof(maps[0]), "%s" ITALICSTOGGLE "\t%s", temp, wadname);
                            free(temp);
                        }
                    }
                    else
                    {
                        temp = titlecase(*mapinfoname ? mapinfoname : (bfgedition ? *mapnames2_bfg[map] : *mapnames2[map]));
                        removemapnum(temp);
                        M_snprintf(maps[count++], sizeof(maps[0]), "%s\t" ITALICS("%s") "\t%s", lump,
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
                    M_snprintf(maps[count++], sizeof(maps[0]), "%s\t" ITALICS("%s") "\t%s", lump, temp, wadname);
                    free(temp);
                }

                break;

            case pack_plut:
                if (!replaced || pwad)
                {
                    temp = titlecase(*mapinfoname ? mapinfoname : *mapnamesp[map]);
                    removemapnum(temp);
                    M_snprintf(maps[count++], sizeof(maps[0]), "%s\t" ITALICS("%s") "\t%s", lump,
                        (replaced && dehcount == 1 && !*mapinfoname ? "-" : temp), wadname);
                    free(temp);
                }

                break;

            case pack_tnt:
                if (!replaced || pwad)
                {
                    temp = titlecase(*mapinfoname ? mapinfoname : *mapnamest[map]);
                    removemapnum(temp);
                    M_snprintf(maps[count++], sizeof(maps[0]), "%s\t" ITALICS("%s") "\t%s", lump,
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

static bool mapstats_cmd_func1(char *cmd, char *parms)
{
    return (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION);
}

static void mapstats_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[3] = { 120, 240, 0 };
    char        *temp;
    int         lump;
    int         wadtype;

    if (FREEDOOM1)
    {
        char    lumpname[6];

        M_snprintf(lumpname, sizeof(lumpname), "E%iM%i", gameepisode, gamemap);
        lump = W_CheckNumForName(lumpname);
        wadtype = lumpinfo[lump]->wadfile->type;
    }
    else if (BTSX)
    {
        char    lumpname[6];

        M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", gamemap);
        lump = W_CheckNumForName(lumpname);
        wadtype = lumpinfo[lump]->wadfile->type;
    }
    else
    {
        lump = (nerve && gamemission == doom2 ? W_GetLastNumForName(mapnum) : W_CheckNumForName(mapnum));

        if (M_StringEndsWith(lumpinfo[lump]->wadfile->path, "DOOM2.WAD")
            || M_StringEndsWith(lumpinfo[lump]->wadfile->path, "chex.wad")
            || M_StringEndsWith(lumpinfo[lump]->wadfile->path, "rekkrsa.wad"))
            wadtype = IWAD;
        else
            wadtype = lumpinfo[lump]->wadfile->type;
    }

    C_Header(tabs, mapstats, MAPSTATSHEADER);

    if (gamemode == commercial)
    {
        if (gamemission == pack_nerve)
        {
            temp = titlecase(*expansions[1]);
            C_TabbedOutput(tabs, "Expansion\t" ITALICS("%s") "  (2 of 2)", temp);
            free(temp);

            C_TabbedOutput(tabs, "Map\t%i of %i%s", gamemap, (gamemap == 9 ? 9 : 8), (gamemap == 9 ? " (secret)" : ""));
        }
        else if (nerve)
        {
            temp = titlecase(*expansions[0]);
            C_TabbedOutput(tabs, "Expansion\t" ITALICS("%s") "  (1 of 2)", temp);
            free(temp);

            C_TabbedOutput(tabs, "Map\t%i of %i%s", gamemap, (gamemap >= 31 ? gamemap : 30), (gamemap >= 31 ? " (secret)" : ""));
        }
        else
            C_TabbedOutput(tabs, "Map\t%i of %i%s", gamemap, (gamemap >= 31 ? gamemap : 30), (gamemap >= 31 ? " (secret)" : ""));
    }
    else
    {
        if (!chex && !hacx)
        {
            temp = titlecase(*episodes[gameepisode - 1]);
            C_TabbedOutput(tabs, "Episode\t" ITALICS("%s") "  (%i of %i)",
                temp, gameepisode, (gamemode == retail ? (sigil ? 5 : 4) : 3));
            free(temp);
        }

        C_TabbedOutput(tabs, "Map\t%i of %i%s", gamemap, (gamemap == 9 ? 9 : 8), (gamemap == 9 ? " (secret)" : ""));
    }

    if (!M_StringCompare(maptitle, mapnum))
    {
        temp = titlecase(maptitle);
        C_TabbedOutput(tabs, "Title\t" ITALICS("%s"), temp);
        free(temp);

        if (gamemode == commercial)
        {
            if (gamemap == 11)
            {
                if (M_StringCompare(maptitle, s_HUSTR_11))
                    C_TabbedOutput(tabs, "Alternative Title\t" ITALICS("%s"), s_HUSTR_11_ALT);
            }
            else if (gamemap == 31)
            {
                if (M_StringCompare(maptitle, s_HUSTR_31))
                    C_TabbedOutput(tabs, "Alternative Title\t" ITALICS("%s"), s_HUSTR_31_BFG);
            }
            else if (gamemap == 32)
            {
                if (M_StringCompare(maptitle, s_HUSTR_32))
                    C_TabbedOutput(tabs, "Alternative Title\t" ITALICS("%s"), s_HUSTR_32_BFG);
            }
        }
        else if (gameepisode == 3 && gamemap == 7)
        {
            if (M_StringCompare(maptitle, s_HUSTR_E3M7))
                C_TabbedOutput(tabs, "Alternate Title\t" ITALICS("%s"), s_HUSTR_E3M7_ALT);
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
            C_TabbedOutput(tabs, "Author\t%s", author);
        else if (canmodify && *authors[i][gamemission])
            C_TabbedOutput(tabs, "Author\t%s", authors[i][gamemission]);
        else if (REKKR)
            C_TabbedOutput(tabs, "Author\tMatthew Little");
    }

    {
        char    wadname[MAX_PATH];

        M_StringCopy(wadname, leafname(lumpinfo[lump]->wadfile->path), sizeof(wadname));

        C_TabbedOutput(tabs, "%s\t%s", (wadtype == IWAD ? "IWAD" : "PWAD"), wadname);

        if (M_StringCompare(wadname, "DOOM1.WAD"))
            C_TabbedOutput(tabs, INDENT "Release date\tFriday, December 10, 1993");
        else if (M_StringCompare(wadname, "DOOM.WAD"))
        {
            if (unity)
                C_TabbedOutput(tabs, INDENT "Release date\tFriday, July 26, 2019");
            else if (bfgedition)
                C_TabbedOutput(tabs, INDENT "Release date\tTuesday, October 16, 2012");
            else if (gamemode == registered)
                C_TabbedOutput(tabs, INDENT "Release date\tSunday, April 30, 1995");
            else
                C_TabbedOutput(tabs, INDENT "Release date\tFriday, December 10, 1993");
        }
        else if (M_StringCompare(wadname, "SIGIL_v1_21.wad")
            || M_StringCompare(wadname, "SIGIL_v1_2.wad")
            || M_StringCompare(wadname, "SIGIL_v1_1.wad")
            || M_StringCompare(wadname, "SIGIL_v1_0.wad")
            || M_StringCompare(wadname, "SIGIL.wad"))
            C_TabbedOutput(tabs, INDENT "Release date\tWednesday, May 22, 2019");
        else if (M_StringCompare(wadname, "DOOM2.WAD"))
        {
            if (unity)
                C_TabbedOutput(tabs, INDENT "Release date\tFriday, July 26, 2019");
            else if (bfgedition)
                C_TabbedOutput(tabs, INDENT "Release date\tTuesday, October 16, 2012");
            else
                C_TabbedOutput(tabs, INDENT "Release date\tFriday, September 30, 1994");
        }
        else if (M_StringCompare(wadname, "NERVE.WAD"))
            C_TabbedOutput(tabs, INDENT "Release date\tWednesday, May 26, 2010");
        else if (M_StringCompare(wadname, "PLUTONIA.WAD") || M_StringCompare(wadname, "TNT.WAD"))
            C_TabbedOutput(tabs, INDENT "Release date\tMonday, June 17, 1996");
        else if (onehumanity)
            C_TabbedOutput(tabs, INDENT "Release date\tWednesday, March 2, 2022");
        else if (REKKRSL)
            C_TabbedOutput(tabs, INDENT "Release date\tMonday, October 11, 2021");
        else if (REKKR)
            C_TabbedOutput(tabs, INDENT "Release date\tTuesday, July 10, 2018");

        if (wadtype == PWAD)
            C_TabbedOutput(tabs, "IWAD\t%s", leafname(lumpinfo[W_GetLastNumForName("PLAYPAL")]->wadfile->path));
    }

    C_TabbedOutput(tabs, "Compatibility\t%s",
        (mbf21compatible ? ITALICS("MBF21") : (mbfcompatible ? ITALICS("MBF") : (boomcompatible ? ITALICS("BOOM") :
        (numsegs < 32768 ? "Vanilla" : "Limit removing")))));

    {
        const int   partime = G_GetParTime();

        if (partime)
        {
            const int   hours = partime / 3600;

            if (hours)
                C_TabbedOutput(tabs, "Par time\t%i:%02i:%02i", hours, partime / 60, partime % 60);
            else
                C_TabbedOutput(tabs, "Par time\t%02i:%02i", partime / 60, partime % 60);
        }
        else
            C_TabbedOutput(tabs, "Par time\t\x96");
    }

    temp = commify(numspawnedthings);
    C_TabbedOutput(tabs, "Things\t%s", temp);
    free(temp);

    temp = commify(totalkills);
    C_TabbedOutput(tabs, INDENT "Monsters\t%s", temp);
    free(temp);

    temp = commify(totalpickups);
    C_TabbedOutput(tabs, INDENT "Pickups\t%s", temp);
    free(temp);

    temp = commify(numdecorations);
    C_TabbedOutput(tabs, INDENT "Decorations\t%s", temp);
    free(temp);

    temp = commify(barrelcount);
    C_TabbedOutput(tabs, INDENT "Barrels\t%s", temp);
    free(temp);

    temp = commify(numlines);
    C_TabbedOutput(tabs, "Lines\t%s", temp);
    free(temp);

    temp = commify(numspeciallines);
    C_TabbedOutput(tabs, INDENT "Special\t%s", temp);
    free(temp);

    temp = commify(numsides);
    C_TabbedOutput(tabs, "Sides\t%s", temp);
    free(temp);

    temp = commify(numvertexes);
    C_TabbedOutput(tabs, "Vertices\t%s", temp);
    free(temp);

    temp = commify(numsegs);
    C_TabbedOutput(tabs, "Segments\t%s", temp);
    free(temp);

    temp = commify(numsubsectors);
    C_TabbedOutput(tabs, "Subsectors\t%s", temp);
    free(temp);

    temp = commify(numnodes);
    C_TabbedOutput(tabs, "Nodes\t%s", temp);
    free(temp);

    C_TabbedOutput(tabs, INDENT "Format\t%s", mapformats[mapformat]);

    temp = commify(numsectors);
    C_TabbedOutput(tabs, "Sectors\t%s", temp);
    free(temp);

    {
        int outside = 0;

        for (int i = 0; i < numsectors; i++)
        {
            const short picnum = sectors[i].ceilingpic;

            if (picnum == skyflatnum || (picnum & PL_SKYFLAT))
                outside++;
        }

        outside = outside * 100 / numsectors;
        C_TabbedOutput(tabs, INDENT "Inside/outside\t%i%%/%i%%", 100 - outside, outside);
    }

    temp = commify(totalsecrets);
    C_TabbedOutput(tabs, INDENT "Secret\t%s", temp);
    free(temp);

    temp = commify(numliquid);
    C_TabbedOutput(tabs, INDENT "Liquid\t%s", temp);
    free(temp);

    temp = commify(numdamaging);
    C_TabbedOutput(tabs, INDENT "Damaging\t%s", temp);
    free(temp);

    if (blockmaprebuilt)
        C_TabbedOutput(tabs, "Blockmap\tRebuilt");

    {
        int min_x = INT_MAX;
        int max_x = INT_MIN;
        int min_y = INT_MAX;
        int max_y = INT_MIN;
        int max_ceilingheight = INT_MIN;
        int min_floorheight = INT_MAX;
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
            if (max_ceilingheight < sectors[i].ceilingheight)
                max_ceilingheight = sectors[i].ceilingheight;

            if (min_floorheight > sectors[i].floorheight)
                min_floorheight = sectors[i].floorheight;
        }

        depth = ((max_ceilingheight >> FRACBITS) - (min_floorheight >> FRACBITS)) / UNITSPERFOOT;

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

                C_TabbedOutput(tabs, "Dimensions\t%sx%sx%s %s",
                    temp1, temp2, temp3, (english == english_american ? "meters" : "metres"));
                free(temp1);
                free(temp2);
                free(temp3);
            }
            else
            {
                char    *temp1 = striptrailingzero(metricwidth / METERSPERKILOMETER, 2);
                char    *temp2 = striptrailingzero(metricheight / METERSPERKILOMETER, 2);
                char    *temp3 = striptrailingzero(metricdepth / METERSPERKILOMETER, 2);

                C_TabbedOutput(tabs, "Dimensions\t%sx%sx%s %s",
                    temp1, temp2, temp3, (english == english_american ? "kilometers" : "kilometres"));
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

                C_TabbedOutput(tabs, "Dimensions\t%sx%sx%s feet", temp1, temp2, temp3);
                free(temp1);
                free(temp2);
                free(temp3);
            }
            else
            {
                char    *temp1 = striptrailingzero((float)width / FEETPERMILE, 2);
                char    *temp2 = striptrailingzero((float)height / FEETPERMILE, 2);
                char    *temp3 = striptrailingzero((float)depth / FEETPERMILE, 2);

                C_TabbedOutput(tabs, "Dimensions\t%sx%sx%s miles", temp1, temp2, temp3);
                free(temp1);
                free(temp2);
                free(temp3);
            }
        }
    }

    if (mus_playing && !nomusic)
    {
        int                 lumps;
        char                namebuf[9];
        const char          *musicartist = P_GetMapMusicComposer((gameepisode - 1) * 10 + gamemap);
        const char          *musictitle = P_GetMapMusicTitle((gameepisode - 1) * 10 + gamemap);
        const Mix_MusicType musictype = Mix_GetMusicType(NULL);

        temp = uppercase(mus_playing->name1);
        M_snprintf(namebuf, sizeof(namebuf), "D_%s", temp);
        C_TabbedOutput(tabs, "Music lump\t%s", namebuf);
        free(temp);

        lumps = W_CheckMultipleLumps(namebuf);

        if (lumpinfo[mus_playing->lumpnum]->wadfile->type == PWAD)
            C_TabbedOutput(tabs, INDENT "PWAD\t%s", leafname(lumpinfo[mus_playing->lumpnum]->wadfile->path));

        if (*musictitle)
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), musictitle);
        else if (sigil && gameepisode == 5)
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), (buckethead ? mus_playing->title2 : mus_playing->title1));
        else if (((gamemode == commercial || gameepisode > 1) && lumps == 1 && wadtype == IWAD && gamemission != pack_tnt)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2)
            || gamemode == shareware
            || gamemission == pack_nerve)
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), mus_playing->title1);

        if (*musicartist)
            C_TabbedOutput(tabs, INDENT "Artist\t%s", musicartist);
        else if (sigil && gameepisode == 5)
            C_TabbedOutput(tabs, INDENT "Artist\t%s", (buckethead ? "Buckethead" : "James Paddock"));
        else if (((gamemode == commercial || gameepisode > 1) && lumps == 1 && wadtype == IWAD && gamemission != pack_tnt)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2)
            || gamemode == shareware
            || gamemission == pack_nerve)
            C_TabbedOutput(tabs, INDENT "Artist\t%s", "Bobby Prince");

        if (musmusictype)
            C_TabbedOutput(tabs, INDENT "Format\tMUS");
        else if (midimusictype || musictype == MUS_MID)
            C_TabbedOutput(tabs, INDENT "Format\tMIDI");
        else if (musictype == MUS_OGG)
            C_TabbedOutput(tabs, INDENT "Format\tOgg Vorbis");
        else if (musictype == MUS_MP3)
            C_TabbedOutput(tabs, INDENT "Format\tMP3");
        else if (musictype == MUS_WAV)
            C_TabbedOutput(tabs, INDENT "Format\tWAV");
        else if (musictype == MUS_FLAC)
            C_TabbedOutput(tabs, INDENT "Format\tFLAC");
        else if (musictype == MUS_MOD)
            C_TabbedOutput(tabs, INDENT "Format\tMOD");
    }
}

//
// name CCMD
//
static bool namecmdfriendly;
static bool namecmdanymonster;
static char namecmdnew[128];
static char namecmdold[128];
static int  namecmdtype = NUMMOBJTYPES;

static bool name_cmd_func1(char *cmd, char *parms)
{
    char    *parm = M_StringDuplicate(parms);

    if (!*parm || gamestate != GS_LEVEL)
        return true;

    if (M_StringStartsWith(parm, "player"))
    {
        M_StringCopy(namecmdold, "player", sizeof(namecmdold));
        M_StringReplaceAll(parm, "player", "", false);
        M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));

        return (namecmdnew[0] != '\0' && strlen(namecmdnew) < 33);
    }

    if (gamestate == GS_LEVEL)
    {
        if ((namecmdfriendly = M_StringStartsWith(parm, "friendly ")))
            M_StringReplaceAll(parm, "friendly ", "", false);
        else if ((namecmdfriendly = M_StringStartsWith(parm, "friendly")))
            M_StringReplaceAll(parm, "friendly", "", false);

        if (M_StringStartsWith(parm, "monster"))
        {
            M_StringCopy(namecmdold, "monster", sizeof(namecmdold));
            M_StringReplaceAll(parm, "monster", "", false);
            M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
            namecmdanymonster = true;

            return (namecmdnew[0] != '\0' && strlen(namecmdnew) < 64);
        }
        else
            namecmdanymonster = false;

        for (int i = 0; i < NUMMOBJTYPES; i++)
            if ((mobjinfo[i].flags & MF_SHOOTABLE) && i != MT_PLAYER && i != MT_BARREL)
            {
                bool    result = false;
                char    *temp1 = (*mobjinfo[i].name1 ? removenonalpha(mobjinfo[i].name1) : NULL);
                char    *temp2 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char    *temp3 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);

                if (*mobjinfo[i].name1 && M_StringStartsWith(parm, temp1))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name1, sizeof(namecmdold));
                    M_StringReplaceAll(parm, temp1, "", false);
                    M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
                    namecmdtype = i;
                    result = true;
                }
                else if (*mobjinfo[i].name2 && M_StringStartsWith(parm, temp2))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name2, sizeof(namecmdold));
                    M_StringReplaceAll(parm, temp2, "", false);
                    M_StringCopy(namecmdnew, trimwhitespace(parm), sizeof(namecmdnew));
                    namecmdtype = i;
                    result = true;
                }
                else if (*mobjinfo[i].name3 && M_StringStartsWith(parm, temp3))
                {
                    M_StringCopy(namecmdold, mobjinfo[i].name3, sizeof(namecmdold));
                    M_StringReplaceAll(parm, temp3, "", false);
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
                    return (namecmdnew[0] != '\0' && strlen(namecmdnew) < 64);
            }
    }

    return false;
}

static void name_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMEWARNING, "you", "are");
            else
                C_Warning(0, NOGAMEWARNING, playername, "is");
        }
    }
    else if (M_StringCompare(namecmdold, "player")
        || M_StringCompare(namecmdold, playername)
        || M_StringCompare(namecmdold, playername_default))
    {
        if (M_StringCompare(playername, playername_default))
            C_PlayerMessage("You have been named " BOLD("%s") ".", namecmdnew);
        else
            C_PlayerMessage("%s has been renamed " BOLD("%s") ".", playername, namecmdnew);

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
            mobj_t              *mobj = (mobj_t *)th;
            const int           flags = mobj->flags;
            const mobjtype_t    type = mobj->type;

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

            C_PlayerMessage("The nearest %s%s has been %s " BOLD("%s") ".",
                (namecmdfriendly ? "friendly " : ""), namecmdold, (*bestmobj->name ? "renamed" : "named"), namecmdnew);

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

                    if (((flags & MF_SHOOTABLE) || (thing->flags2 & MF2_MONSTERMISSILE))
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
            C_Warning(0, NEXTMAPWARNING);
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
            for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
            {
                if (thing->target && thing->target->player)
                    P_SetTarget(&thing->target, NULL);

                if (thing->tracer && thing->tracer->player)
                    P_SetTarget(&thing->tracer, NULL);

                if (thing->lastenemy && thing->lastenemy->player)
                    P_SetTarget(&thing->lastenemy, NULL);
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
        C_Warning(0, NEXTMAPWARNING);
}

//
// play CCMD
//
static int  playcmdid;
static int  playcmdtype;

static bool play_cmd_func1(char *cmd, char *parms)
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
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

static weapontype_t favoriteweapon(bool total)
{
    weapontype_t    favorite = wp_nochange;

    if (total)
    {
        uint64_t    shotsfiredstat = 0;

        if (shotsfiredstat < stat_shotsfired_fists)
        {
            shotsfiredstat = stat_shotsfired_fists;
            favorite = wp_fist;
        }

        if (shotsfiredstat < stat_shotsfired_chainsaw)
        {
            shotsfiredstat = stat_shotsfired_chainsaw;
            favorite = wp_chainsaw;
        }

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

char *distancetraveled(uint64_t value, bool allowzero)
{
    char    result[20] = "";

    if (value > 0 || allowzero)
    {
        const float feet = (float)value / UNITSPERFOOT;

        if (units == units_imperial)
        {
            if (feet >= 1.0f)
            {
                if (feet < FEETPERMILE)
                {
                    char    *temp = commify((int64_t)feet);

                    M_snprintf(result, sizeof(result), "%s %s",
                        temp, (M_StringCompare(temp, "1") ? "foot" : "feet"));
                    free(temp);
                }
                else
                {
                    char    *temp = striptrailingzero(feet / FEETPERMILE, 2);

                    M_snprintf(result, sizeof(result), "%s miles", temp);
                    free(temp);
                }
            }
            else if (allowzero)
                M_StringCopy(result, "0 feet", sizeof(result));
        }
        else
        {
            const float meters = feet / FEETPERMETER;

            if (meters >= 0.1f)
            {
                if (meters < METERSPERKILOMETER)
                {
                    char    *temp = striptrailingzero(meters, 1);

                    if (!M_StringCompare(temp, "0.0"))
                        M_snprintf(result, sizeof(result), "%s %s",
                            temp, (english == english_american ? "meters" : "metres"));

                    free(temp);
                }
                else
                {
                    char    *temp = striptrailingzero(meters / METERSPERKILOMETER, 2);

                    M_snprintf(result, sizeof(result), "%s %s",
                        temp, (english == english_american ? "kilometers" : "kilometres"));
                    free(temp);
                }
            }
            else if (allowzero)
                M_StringCopy(result, (english == english_american ? "0 meters" : "0 metres"), sizeof(result));
        }
    }

    return M_StringDuplicate(result);
}

//
// playerstats CCMD
//
static void ShowMonsterKillStat_Game(const int tabs[3], const mobjtype_t type)
{
    char    *temp1 = sentencecase(mobjinfo[type].plural1);
    char    *temp2 = commify(viewplayer->mobjcount[type]);
    char    *temp3 = commify(monstercount[type]);
    char    *temp4 = commifystat(stat_monsterskilled[type]);

    C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s",
        temp1, temp2, temp3,
        (monstercount[type] ? viewplayer->mobjcount[type] * 100 / monstercount[type] : 0), temp4);

    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
}

static void C_PlayerStats_Game(void)
{
    const int       tabs[3] = { 200, 335, 0 };
    skill_t         favoriteskilllevel1 = favoriteskilllevel();
    weapontype_t    favoriteweapon1 = favoriteweapon(false);
    weapontype_t    favoriteweapon2 = favoriteweapon(true);
    const int       time1 = maptime / TICRATE;
    const int       time2 = (int)(stat_timeplayed / TICRATE);
    int             hours1;
    int             hours2;
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
        C_TabbedOutput(tabs, "Map explored\t100%%\t\x96");
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
        C_TabbedOutput(tabs, "Map explored\t%s%%\t\x96", temp1);
        free(temp1);
    }

    temp1 = commifystat(stat_mapsstarted);
    C_TabbedOutput(tabs, "Maps started\t1\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_mapscompleted);
    C_TabbedOutput(tabs, "Maps completed\t0\t%s", temp1);
    free(temp1);

    temp1 = commify(viewplayer->gamessaved);
    temp2 = commifystat(stat_gamessaved);
    C_TabbedOutput(tabs, "Games saved\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    if (favoriteskilllevel1 == sk_none)
        C_TabbedOutput(tabs, "%s skill level\t\x96\t\x96",
            (english == english_american ? "Favorite" : "Favourite"));
    else
    {
        temp1 = titlecase(*skilllevels[skilllevel - 1]);

        if (temp1[strlen(temp1) - 1] == '.')
            temp1[strlen(temp1) - 1] = '\0';

        temp2 = titlecase(*skilllevels[favoriteskilllevel1]);

        if (temp2[strlen(temp2) - 1] == '.')
            temp2[strlen(temp2) - 1] = '\0';

        C_TabbedOutput(tabs, "%s skill level\t" ITALICS("%s") "\t" ITALICS("%s"),
            (english == english_american ? "Favorite" : "Favourite"), temp1, temp2);
        free(temp1);
        free(temp2);
    }

    for (int i = 0; i < NUMMOBJTYPES; i++)
        killcount += viewplayer->mobjcount[i];

    temp1 = commify(killcount);
    temp2 = commify(totalkills);
    temp3 = commifystat(stat_monsterskilled_total);
    C_TabbedOutput(tabs, "Monsters killed by %s\t%s of %s (%i%%)\t%s",
        playername, temp1, temp2, (totalkills ? killcount * 100 / totalkills : 0), temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    if (gamemode == commercial)
    {
        ShowMonsterKillStat_Game(tabs, MT_BABY);
        ShowMonsterKillStat_Game(tabs, MT_VILE);
    }

    ShowMonsterKillStat_Game(tabs, MT_BRUISER);
    ShowMonsterKillStat_Game(tabs, MT_HEAD);

    if (gamemode == commercial)
        ShowMonsterKillStat_Game(tabs, MT_CHAINGUY);

    if (gamemode != shareware)
        ShowMonsterKillStat_Game(tabs, MT_CYBORG);

    if (gamemode == commercial)
        ShowMonsterKillStat_Game(tabs, MT_KNIGHT);

    ShowMonsterKillStat_Game(tabs, MT_TROOP);
    ShowMonsterKillStat_Game(tabs, MT_SKULL);

    if (gamemode == commercial)
    {
        ShowMonsterKillStat_Game(tabs, MT_FATSO);
        ShowMonsterKillStat_Game(tabs, MT_PAIN);
    }

    ShowMonsterKillStat_Game(tabs, MT_SERGEANT);
    ShowMonsterKillStat_Game(tabs, MT_UNDEAD);
    ShowMonsterKillStat_Game(tabs, MT_SHOTGUY);
    ShowMonsterKillStat_Game(tabs, MT_SHADOWS);

    if (gamemode != shareware)
        ShowMonsterKillStat_Game(tabs, MT_SPIDER);

    ShowMonsterKillStat_Game(tabs, MT_POSSESSED);

    temp1 = commify(viewplayer->infightcount);
    temp2 = commifystat(stat_monsterskilled_infighting);
    C_TabbedOutput(tabs, "Monsters killed while infighting\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->respawncount);
    temp2 = commifystat(stat_monstersrespawned);
    C_TabbedOutput(tabs, "Monsters respawned\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->resurrectioncount);
    temp2 = commifystat(stat_monstersresurrected);
    C_TabbedOutput(tabs, "Monsters resurrected\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->telefragcount);
    temp2 = commifystat(stat_monsterstelefragged);
    C_TabbedOutput(tabs, "Monsters telefragged\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(mobjinfo[MT_BARREL].plural1);
    temp2 = commify(viewplayer->mobjcount[MT_BARREL]);
    temp3 = commify(barrelcount);
    temp4 = commifystat(stat_barrelsexploded);
    C_TabbedOutput(tabs, "%s exploded\t%s of %s (%i%%)\t%s",
        temp1, temp2, temp3,
        (barrelcount ? viewplayer->mobjcount[MT_BARREL] * 100 / barrelcount : 0), temp4);
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = commify(viewplayer->itemcount);
    temp2 = commify(totalitems);
    temp3 = commifystat(stat_itemspickedup);
    C_TabbedOutput(tabs, "Items picked up\t%s of %s (%i%%)\t%s",
        temp1, temp2, (totalitems ? viewplayer->itemcount * 100 / totalitems : 0), temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = commify(viewplayer->itemspickedup_ammo_bullets);
    temp2 = commifystat(stat_itemspickedup_ammo_bullets);
    C_TabbedOutput(tabs, "Ammo picked up\t%s %s\t%s %s",
        temp1, (viewplayer->itemspickedup_ammo_bullets == 1 ? weaponinfo[wp_pistol].ammoname : weaponinfo[wp_pistol].ammoplural),
        temp2,  (stat_itemspickedup_ammo_bullets == 1 ? weaponinfo[wp_pistol].ammoname : weaponinfo[wp_pistol].ammoplural));
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_ammo_shells);
    temp2 = commifystat(stat_itemspickedup_ammo_shells);
    C_TabbedOutput(tabs, "\t%s %s\t%s %s",
        temp1, (viewplayer->itemspickedup_ammo_shells == 1 ? weaponinfo[wp_shotgun].ammoname : weaponinfo[wp_shotgun].ammoplural),
        temp2, (stat_itemspickedup_ammo_shells == 1 ? weaponinfo[wp_shotgun].ammoname : weaponinfo[wp_shotgun].ammoplural));
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_ammo_rockets);
    temp2 = commifystat(stat_itemspickedup_ammo_rockets);
    C_TabbedOutput(tabs, "\t%s %s\t%s %s",
        temp1, (viewplayer->itemspickedup_ammo_rockets == 1 ? weaponinfo[wp_missile].ammoname : weaponinfo[wp_missile].ammoplural),
        temp2, (stat_itemspickedup_ammo_rockets == 1 ? weaponinfo[wp_missile].ammoname : weaponinfo[wp_missile].ammoplural));
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_ammo_cells);
    temp2 = commifystat(stat_itemspickedup_ammo_cells);
    C_TabbedOutput(tabs, "\t%s %s\t%s %s",
        temp1, (viewplayer->itemspickedup_ammo_cells == 1 ? weaponinfo[wp_plasma].ammoname : weaponinfo[wp_plasma].ammoplural),
        temp2, (stat_itemspickedup_ammo_cells == 1 ? weaponinfo[wp_plasma].ammoname : weaponinfo[wp_plasma].ammoplural));
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_armor);
    temp2 = commifystat(stat_itemspickedup_armor);
    C_TabbedOutput(tabs, "%s picked up\t%s\t%s", (english == english_american ? "Armor" : "Armour"), temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_health);
    temp2 = commifystat(stat_itemspickedup_health);
    C_TabbedOutput(tabs, "Health picked up\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->secretcount);
    temp2 = commify(totalsecrets);
    temp3 = commifystat(stat_secretsfound);
    C_TabbedOutput(tabs, "Secrets found\t%s of %s (%i%%)\t%s",
        temp1, temp2, (totalsecrets ? viewplayer->secretcount * 100 / totalsecrets : 0), temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    hours1 = time1 / 3600;
    hours2 = time2 / 3600;
    temp2 = commify(hours2);

    if (sucktime && hours1 >= sucktime)
    {
        if (hours2 >= 100)
            C_TabbedOutput(tabs, "Time played\t%s\tOver %s hours!", s_STSTR_SUCKS, temp2);
        else if (hours2)
            C_TabbedOutput(tabs, "Time played\t%s\t%i:%02i:%02i",
                s_STSTR_SUCKS, hours2, (time2 % 3600) / 60, (time2 % 3600) % 60);
        else
            C_TabbedOutput(tabs, "Time played\t%s\t%02i:%02i",
                s_STSTR_SUCKS, (time2 % 3600) / 60, (time2 % 3600) % 60);
    }
    else if (hours1)
    {
        if (hours2 >= 100)
            C_TabbedOutput(tabs, "Time played\t%i:%02i:%02i\tOver %s hours!",
                hours1, (time1 % 3600) / 60, (time1 % 3600) % 60, temp2);
        else if (hours2)
            C_TabbedOutput(tabs, "Time played\t%i:%02i:%02i\t%i:%02i:%02i",
                hours1, (time1 % 3600) / 60, (time1 % 3600) % 60, hours2, (time2 % 3600) / 60, (time2 % 3600) % 60);
        else
            C_TabbedOutput(tabs, "Time played\t%i:%02i:%02i\t%02i:%02i",
                hours1, (time1 % 3600) / 60, (time1 % 3600) % 60, (time2 % 3600) / 60, (time2 % 3600) % 60);
    }
    else
    {
        if (hours2 >= 100)
            C_TabbedOutput(tabs, "Time played\t%02i:%02i\tOver %s hours!",
                (time1 % 3600) / 60, (time1 % 3600) % 60, temp2);
        else if (hours2)
            C_TabbedOutput(tabs, "Time played\t%02i:%02i\t%i:%02i:%02i",
                (time1 % 3600) / 60, (time1 % 3600) % 60, hours2, (time2 % 3600) / 60, (time2 % 3600) % 60);
        else
            C_TabbedOutput(tabs, "Time played\t%02i:%02i\t%02i:%02i",
                (time1 % 3600) / 60, (time1 % 3600) % 60, (time2 % 3600) / 60, (time2 % 3600) % 60);
    }

    free(temp2);

    temp1 = commify(viewplayer->damageinflicted);
    temp2 = commifystat(stat_damageinflicted);
    C_TabbedOutput(tabs, "Damage inflicted\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->damagereceived);
    temp2 = commifystat(stat_damagereceived);
    C_TabbedOutput(tabs, "Damage received\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->deaths);
    temp2 = commifystat(stat_deaths);
    C_TabbedOutput(tabs, "Deaths\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->suicides);
    temp2 = commifystat(stat_suicides);
    C_TabbedOutput(tabs, "Suicides\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->cheated);
    temp2 = commifystat(stat_cheated);
    C_TabbedOutput(tabs, "Cheated\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    for (int i = 0; i < NUMWEAPONS; i++)
        shotsfired1 += viewplayer->shotsfired[i];

    for (int i = 0; i < NUMWEAPONS; i++)
        shotssuccessful1 += viewplayer->shotssuccessful[i];

    temp1 = commify(shotssuccessful1);
    temp2 = commify(shotsfired1);
    temp3 = commify((shotssuccessful2 = stat_shotssuccessful_fists + stat_shotssuccessful_chainsaw + stat_shotssuccessful_pistol
        + stat_shotssuccessful_shotgun + stat_shotssuccessful_supershotgun + stat_shotssuccessful_chaingun
        + stat_shotssuccessful_rocketlauncher + stat_shotssuccessful_plasmarifle + stat_shotssuccessful_bfg9000));
    temp4 = commify((shotsfired2 = stat_shotsfired_fists + stat_shotsfired_chainsaw + stat_shotsfired_pistol + stat_shotsfired_shotgun
        + stat_shotsfired_supershotgun + stat_shotsfired_chaingun + stat_shotsfired_rocketlauncher + stat_shotsfired_plasmarifle
        + stat_shotsfired_bfg9000));
    C_TabbedOutput(tabs, "Shots successful/fired\t%s of %s (%i%%)\t%s of %s (%i%%)",
        temp1, temp2, (shotsfired1 ? shotssuccessful1 * 100 / shotsfired1 : 0), temp3, temp4,
        (shotsfired2 ? (int)(shotssuccessful2 * 100 / shotsfired2) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);

    temp1 = sentencecase(weaponinfo[wp_fist].name);
    temp2 = commify(viewplayer->shotssuccessful[wp_fist]);
    temp3 = commify(viewplayer->shotsfired[wp_fist]);
    temp4 = commifystat(stat_shotssuccessful_fists);
    temp5 = commifystat(stat_shotsfired_fists);
    C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
        temp1, temp2, temp3,
        (viewplayer->shotsfired[wp_fist] ? viewplayer->shotssuccessful[wp_fist] * 100 / viewplayer->shotsfired[wp_fist] : 0),
        temp4, temp5,
        (stat_shotsfired_fists ? (int)(stat_shotssuccessful_fists * 100 / stat_shotsfired_fists) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);

    temp1 = sentencecase(weaponinfo[wp_chainsaw].name);
    temp2 = commify(viewplayer->shotssuccessful[wp_chainsaw]);
    temp3 = commify(viewplayer->shotsfired[wp_chainsaw]);
    temp4 = commifystat(stat_shotssuccessful_chainsaw);
    temp5 = commifystat(stat_shotsfired_chainsaw);
    C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
        temp1, temp2, temp3,
        (viewplayer->shotsfired[wp_chainsaw] ? viewplayer->shotssuccessful[wp_chainsaw] * 100 / viewplayer->shotsfired[wp_chainsaw] : 0),
        temp4, temp5,
        (stat_shotsfired_chainsaw ? (int)(stat_shotssuccessful_chainsaw * 100 / stat_shotsfired_chainsaw) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);

    temp1 = sentencecase(weaponinfo[wp_pistol].name);
    temp2 = commify(viewplayer->shotssuccessful[wp_pistol]);
    temp3 = commify(viewplayer->shotsfired[wp_pistol]);
    temp4 = commifystat(stat_shotssuccessful_pistol);
    temp5 = commifystat(stat_shotsfired_pistol);
    C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
        temp1, temp2, temp3,
        (viewplayer->shotsfired[wp_pistol] ? viewplayer->shotssuccessful[wp_pistol] * 100 / viewplayer->shotsfired[wp_pistol] : 0),
        temp4, temp5,
        (stat_shotsfired_pistol ? (int)(stat_shotssuccessful_pistol * 100 / stat_shotsfired_pistol) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);

    temp1 = sentencecase(weaponinfo[wp_shotgun].name);
    temp2 = commify(viewplayer->shotssuccessful[wp_shotgun]);
    temp3 = commify(viewplayer->shotsfired[wp_shotgun]);
    temp4 = commifystat(stat_shotssuccessful_shotgun);
    temp5 = commifystat(stat_shotsfired_shotgun);
    C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
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
        temp1 = sentencecase(weaponinfo[wp_supershotgun].name);
        temp2 = commify(viewplayer->shotssuccessful[wp_supershotgun]);
        temp3 = commify(viewplayer->shotsfired[wp_supershotgun]);
        temp4 = commifystat(stat_shotssuccessful_supershotgun);
        temp5 = commifystat(stat_shotsfired_supershotgun);
        C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
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

    temp1 = sentencecase(weaponinfo[wp_chaingun].name);
    temp2 = commify(viewplayer->shotssuccessful[wp_chaingun]);
    temp3 = commify(viewplayer->shotsfired[wp_chaingun]);
    temp4 = commifystat(stat_shotssuccessful_chaingun);
    temp5 = commifystat(stat_shotsfired_chaingun);
    C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
        temp1, temp2, temp3,
        (viewplayer->shotsfired[wp_chaingun] ? viewplayer->shotssuccessful[wp_chaingun] * 100 / viewplayer->shotsfired[wp_chaingun] : 0),
        temp4, temp5,
        (stat_shotsfired_chaingun ? (int)(stat_shotssuccessful_chaingun * 100 / stat_shotsfired_chaingun) : 0));
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);

    temp1 = sentencecase(weaponinfo[wp_missile].name);
    temp2 = commify(viewplayer->shotssuccessful[wp_missile]);
    temp3 = commify(viewplayer->shotsfired[wp_missile]);
    temp4 = commifystat(stat_shotssuccessful_rocketlauncher);
    temp5 = commifystat(stat_shotsfired_rocketlauncher);
    C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
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
        temp1 = sentencecase(weaponinfo[wp_plasma].name);
        temp2 = commify(viewplayer->shotssuccessful[wp_plasma]);
        temp3 = commify(viewplayer->shotsfired[wp_plasma]);
        temp4 = commifystat(stat_shotssuccessful_plasmarifle);
        temp5 = commifystat(stat_shotsfired_plasmarifle);
        C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
            temp1, temp2, temp3,
            (viewplayer->shotsfired[wp_plasma] ? viewplayer->shotssuccessful[wp_plasma] * 100 / viewplayer->shotsfired[wp_plasma] : 0),
            temp4, temp5,
            (stat_shotsfired_plasmarifle ? (int)(stat_shotssuccessful_plasmarifle * 100 / stat_shotsfired_plasmarifle) : 0));
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
        free(temp5);

        temp1 = sentencecase(weaponinfo[wp_bfg].name);
        temp2 = commify(viewplayer->shotssuccessful[wp_bfg]);
        temp3 = commify(viewplayer->shotsfired[wp_bfg]);
        temp4 = commifystat(stat_shotssuccessful_bfg9000);
        temp5 = commifystat(stat_shotsfired_bfg9000);
        C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s of %s (%i%%)",
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
        temp1 = titlecase(weaponinfo[wp_pistol].name);
        C_TabbedOutput(tabs, "%s weapon\t%s\t%s",
            (english == english_american ? "Favorite" : "Favourite"), temp1, temp1);
        free(temp1);
    }
    else if (favoriteweapon1 == wp_nochange)
    {
        temp1 = titlecase(weaponinfo[wp_pistol].name);
        temp2 = titlecase(weaponinfo[favoriteweapon2].name);
        C_TabbedOutput(tabs, "%s weapon\t%s\t%s",
            (english == english_american ? "Favorite" : "Favourite"), temp1, temp2);
        free(temp1);
        free(temp2);
    }
    else
    {
        temp1 = titlecase(weaponinfo[favoriteweapon1].name);
        temp2 = titlecase(weaponinfo[favoriteweapon2].name);
        C_TabbedOutput(tabs, "%s weapon\t%s\t%s",
            (english == english_american ? "Favorite" : "Favourite"), temp1, temp2);
        free(temp1);
        free(temp2);
    }

    temp1 = distancetraveled(viewplayer->distancetraveled, true);
    temp2 = distancetraveled(stat_distancetraveled, true);
    C_TabbedOutput(tabs, "Distance %s\t%s\t%s",
        (english == english_american ? "traveled" : "travelled"), temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->automapopened);
    temp2 = commify(stat_automapopened);
    C_TabbedOutput(tabs, "Automap opened\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);
}

static void ShowMonsterKillStat_NoGame(const int tabs[3], const mobjtype_t type)
{
    char    *temp1 = sentencecase(mobjinfo[type].plural1);
    char    *temp2 = commifystat(stat_monsterskilled[type]);

    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s", temp1, temp2);
    free(temp1);
    free(temp2);
}

static void C_PlayerStats_NoGame(void)
{
    const int       tabs[3] = { 200, 335, 0 };
    skill_t         favoriteskilllevel1 = favoriteskilllevel();
    weapontype_t    favoriteweapon1 = favoriteweapon(true);
    const int       time1 = (int)(stat_timeplayed / TICRATE);
    int             hours1;
    char            *temp1;
    char            *temp2;
    char            *temp3;
    uint64_t        shotsfired = 0;
    uint64_t        shotssuccessful = 0;

    C_Header(tabs, playerstats, PLAYERSTATSHEADER);

    temp1 = commifystat(stat_mapsstarted);
    C_TabbedOutput(tabs, "Maps started\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_mapscompleted);
    C_TabbedOutput(tabs, "Maps completed\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_gamessaved);
    C_TabbedOutput(tabs, "Games saved\t\x96\t%s", temp1);
    free(temp1);

    if (favoriteskilllevel1 == sk_none)
        C_TabbedOutput(tabs, "%s skill level\t\x96\t\x96");
    else
    {
        temp1 = titlecase(*skilllevels[favoriteskilllevel1]);

        if (temp1[strlen(temp1) - 1] == '.')
            temp1[strlen(temp1) - 1] = '\0';

        C_TabbedOutput(tabs, "Favorite skill level\t\x96\t" ITALICS("%s"),
            (english == english_american ? "Favorite" : "Favourite"), temp1);
        free(temp1);
    }

    temp1 = commifystat(stat_monsterskilled_total);
    C_TabbedOutput(tabs, "Monsters killed by %s\t\x96\t%s", playername, temp1);
    free(temp1);

    if (gamemode == commercial)
    {
        ShowMonsterKillStat_NoGame(tabs, MT_BABY);
        ShowMonsterKillStat_NoGame(tabs, MT_VILE);
    }

    ShowMonsterKillStat_NoGame(tabs, MT_BRUISER);
    ShowMonsterKillStat_NoGame(tabs, MT_HEAD);

    if (gamemode == commercial)
        ShowMonsterKillStat_NoGame(tabs, MT_CHAINGUY);

    if (gamemode != shareware)
        ShowMonsterKillStat_NoGame(tabs, MT_CYBORG);

    if (gamemode == commercial)
        ShowMonsterKillStat_NoGame(tabs, MT_KNIGHT);

    ShowMonsterKillStat_NoGame(tabs, MT_TROOP);
    ShowMonsterKillStat_NoGame(tabs, MT_SKULL);

    if (gamemode == commercial)
    {
        ShowMonsterKillStat_NoGame(tabs, MT_FATSO);
        ShowMonsterKillStat_NoGame(tabs, MT_PAIN);
    }

    ShowMonsterKillStat_NoGame(tabs, MT_SERGEANT);
    ShowMonsterKillStat_NoGame(tabs, MT_UNDEAD);
    ShowMonsterKillStat_NoGame(tabs, MT_SHOTGUY);
    ShowMonsterKillStat_NoGame(tabs, MT_SHADOWS);

    if (gamemode != shareware)
        ShowMonsterKillStat_NoGame(tabs, MT_SPIDER);

    ShowMonsterKillStat_NoGame(tabs, MT_POSSESSED);

    temp1 = commifystat(stat_monsterskilled_infighting);
    C_TabbedOutput(tabs, "Monsters killed while infighting\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_monstersrespawned);
    C_TabbedOutput(tabs, "Monsters respawned\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_monstersresurrected);
    C_TabbedOutput(tabs, "Monsters resurrected\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_monsterstelefragged);
    C_TabbedOutput(tabs, "Monsters telefragged\t\x96\t%s", temp1);
    free(temp1);

    temp1 = sentencecase(mobjinfo[MT_BARREL].plural1);
    temp2 = commifystat(stat_barrelsexploded);
    C_TabbedOutput(tabs, "%s exploded\t\x96\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commifystat(stat_itemspickedup);
    C_TabbedOutput(tabs, "Items picked up\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_ammo_bullets);
    C_TabbedOutput(tabs, "Ammo picked up\t\x96\t%s %s", temp1,
        (stat_itemspickedup_ammo_bullets == 1 ? weaponinfo[wp_pistol].ammoname : weaponinfo[wp_pistol].ammoplural));
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_ammo_shells);
    C_TabbedOutput(tabs, "\t\x96\t%s %s", temp1,
        (stat_itemspickedup_ammo_shells == 1 ? weaponinfo[wp_shotgun].ammoname : weaponinfo[wp_shotgun].ammoplural));
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_ammo_rockets);
    C_TabbedOutput(tabs, "\t\x96\t%s %s", temp1,
        (stat_itemspickedup_ammo_rockets == 1 ? weaponinfo[wp_missile].ammoname : weaponinfo[wp_missile].ammoplural));
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_ammo_cells);
    C_TabbedOutput(tabs, "\t\x96\t%s %s", temp1 ,
        (stat_itemspickedup_ammo_cells == 1 ? weaponinfo[wp_plasma].ammoname : weaponinfo[wp_plasma].ammoplural));
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_armor);
    C_TabbedOutput(tabs, "%s picked up\t\x96\t%s", (english == english_american ? "Armor" : "Armour"), temp1);
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_health);
    C_TabbedOutput(tabs, "Health picked up\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_secretsfound);
    C_TabbedOutput(tabs, "Secrets found\t\x96\t%s", temp1);
    free(temp1);

    if ((hours1 = time1 / 3600) >= 100)
    {
        temp1 = commify(hours1);
        C_TabbedOutput(tabs, "Time played\t\x96\tOver %s hours!", temp1);
        free(temp1);
    }
    else if (hours1)
        C_TabbedOutput(tabs, "Time played\t\x96\t%i:%02i:%02i", hours1, (time1 % 3600) / 60, (time1 % 3600) % 60);
    else
        C_TabbedOutput(tabs, "Time played\t\x96\t%02i:%02i", (time1 % 3600) / 60, (time1 % 3600) % 60);

    temp1 = commifystat(stat_damageinflicted);
    C_TabbedOutput(tabs, "Damage inflicted\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_damagereceived);
    C_TabbedOutput(tabs, "Damage received\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_deaths);
    C_TabbedOutput(tabs, "Deaths\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_suicides);
    C_TabbedOutput(tabs, "Suicides\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_cheated);
    C_TabbedOutput(tabs, "Cheated\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commify((shotssuccessful = stat_shotssuccessful_fists + stat_shotssuccessful_chainsaw + stat_shotssuccessful_pistol
        + stat_shotssuccessful_shotgun + stat_shotssuccessful_supershotgun + stat_shotssuccessful_chaingun
        + stat_shotssuccessful_rocketlauncher + stat_shotssuccessful_plasmarifle + stat_shotssuccessful_bfg9000));
    temp2 = commify((shotsfired = stat_shotsfired_fists + stat_shotsfired_chainsaw + stat_shotsfired_pistol + stat_shotsfired_shotgun
        + stat_shotsfired_supershotgun + stat_shotsfired_chaingun + stat_shotsfired_rocketlauncher + stat_shotsfired_plasmarifle
        + stat_shotsfired_bfg9000));
    C_TabbedOutput(tabs, "Shots successful/fired\t\x96\t%s of %s (%i%%)",
        temp1, temp2, (shotsfired ? (int)(shotssuccessful * 100 / shotsfired) : 0));
    free(temp1);
    free(temp2);

    temp1 = sentencecase(weaponinfo[wp_fist].name);
    temp2 = commifystat(stat_shotssuccessful_fists);
    temp3 = commifystat(stat_shotsfired_fists);
    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
        temp1, temp2, temp3, (stat_shotsfired_fists ? (int)(stat_shotssuccessful_fists * 100 / stat_shotsfired_fists) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = sentencecase(weaponinfo[wp_chainsaw].name);
    temp2 = commifystat(stat_shotssuccessful_chainsaw);
    temp3 = commifystat(stat_shotsfired_chainsaw);
    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
        temp1, temp2, temp3, (stat_shotsfired_chainsaw ? (int)(stat_shotssuccessful_chainsaw * 100 / stat_shotsfired_chainsaw) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = sentencecase(weaponinfo[wp_pistol].name);
    temp2 = commifystat(stat_shotssuccessful_pistol);
    temp3 = commifystat(stat_shotsfired_pistol);
    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
        temp1, temp2, temp3, (stat_shotsfired_pistol ? (int)(stat_shotssuccessful_pistol * 100 / stat_shotsfired_pistol) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = sentencecase(weaponinfo[wp_shotgun].name);
    temp2 = commifystat(stat_shotssuccessful_shotgun);
    temp3 = commifystat(stat_shotsfired_shotgun);
    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
        temp1, temp2, temp3, (stat_shotsfired_shotgun ? (int)(stat_shotssuccessful_shotgun * 100 / stat_shotsfired_shotgun) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    if (gamemode == commercial)
    {
        temp1 = sentencecase(weaponinfo[wp_supershotgun].name);
        temp2 = commifystat(stat_shotssuccessful_supershotgun);
        temp3 = commifystat(stat_shotsfired_supershotgun);
        C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
            temp1, temp2, temp3,
            (stat_shotsfired_supershotgun ? (int)(stat_shotssuccessful_supershotgun * 100 / stat_shotsfired_supershotgun) : 0));
        free(temp1);
        free(temp2);
        free(temp3);
    }

    temp1 = sentencecase(weaponinfo[wp_chaingun].name);
    temp2 = commifystat(stat_shotssuccessful_chaingun);
    temp3 = commifystat(stat_shotsfired_chaingun);
    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
        temp1, temp2, temp3, (stat_shotsfired_chaingun ? (int)(stat_shotssuccessful_chaingun * 100 / stat_shotsfired_chaingun) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = sentencecase(weaponinfo[wp_missile].name);
    temp2 = commifystat(stat_shotssuccessful_rocketlauncher);
    temp3 = commifystat(stat_shotsfired_rocketlauncher);
    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
        temp1, temp2, temp3,
        (stat_shotsfired_rocketlauncher ? (int)(stat_shotssuccessful_rocketlauncher * 100 / stat_shotsfired_rocketlauncher) : 0));
    free(temp1);
    free(temp2);
    free(temp3);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(weaponinfo[wp_plasma].name);
        temp2 = commifystat(stat_shotssuccessful_plasmarifle);
        temp3 = commifystat(stat_shotsfired_plasmarifle);
        C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
            temp1, temp2, temp3,
            (stat_shotsfired_plasmarifle ? (int)(stat_shotssuccessful_plasmarifle * 100 / stat_shotsfired_plasmarifle) : 0));
        free(temp1);
        free(temp2);
        free(temp3);

        temp1 = sentencecase(weaponinfo[wp_bfg].name);
        temp2 = commifystat(stat_shotssuccessful_bfg9000);
        temp3 = commifystat(stat_shotsfired_bfg9000);
        C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s of %s (%i%%)",
            temp1, temp2, temp3, (stat_shotsfired_bfg9000 ? (int)(stat_shotssuccessful_bfg9000 * 100 / stat_shotsfired_bfg9000) : 0));
        free(temp1);
        free(temp2);
        free(temp3);
    }

    temp1 = titlecase(weaponinfo[(favoriteweapon1 == wp_nochange ? wp_pistol : favoriteweapon1)].name);
    C_TabbedOutput(tabs, "%s weapon\t\x96\t%s",
        (english == english_american ? "Favorite" : "Favourite"), temp1);
    free(temp1);

    temp1 = distancetraveled(stat_distancetraveled, true);
    C_TabbedOutput(tabs, "Distance %s\t\x96\t%s",
        (english == english_american ? "traveled" : "travelled"), temp1);
    free(temp1);

    temp1 = commify(stat_automapopened);
    C_TabbedOutput(tabs, "Automap opened\t\x96\t%s", temp1);
    free(temp1);
}

static void playerstats_cmd_func2(char *cmd, char *parms)
{
    if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
    }
    else
        HU_PlayerMessage(parms, true, false);
}

//
// quit CCMD
//
static void quit_cmd_func2(char *cmd, char *parms)
{
    quitcmd = true;

    if (vid_showfps)
    {
        vid_showfps = false;
        I_UpdateBlitFunc(false);
    }
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
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
        || M_StringCompare(parms, "health")
        || M_StringCompare(parms, "weapon"))
        return;

    resettingcvar = true;

    for (int i = 0; *consolecmds[i].name; i++)
    {
        const int   flags = consolecmds[i].flags;

        if (consolecmds[i].type == CT_CVAR && M_StringCompare(parms, consolecmds[i].name) && !(flags & CF_READONLY))
        {
            if (flags & (CF_BOOLEAN | CF_INTEGER))
            {
                if (*(int *)consolecmds[i].variable != (int)consolecmds[i].defaultnumber)
                {
                    char    *temp1 = C_LookupAliasFromValue((int)consolecmds[i].defaultnumber, consolecmds[i].aliases);
                    char    *temp2 = uncommify(temp1);
                    char    *temp3 = M_StringJoin(parms, " ", temp2, NULL);

                    C_ValidateInput(temp3);
                    C_Output("The " BOLD("%s") " CVAR has been reset to its default.", consolecmds[i].name);
                    free(temp1);
                    free(temp2);
                    free(temp3);
                }
                else
                    C_Warning(0, "The " BOLD("%s") " CVAR is already set to its default.", consolecmds[i].name);
            }
            else if (flags & CF_FLOAT)
            {
                if (*(float *)consolecmds[i].variable != consolecmds[i].defaultnumber)
                {
                    char    *temp1 = striptrailingzero(consolecmds[i].defaultnumber, 1);
                    char    *temp2 = M_StringJoin(parms, " ", temp1, NULL);

                    C_ValidateInput(temp2);
                    C_Output("The " BOLD("%s") " CVAR has been reset to its default.", consolecmds[i].name);
                    free(temp1);
                    free(temp2);
                }
                else
                    C_Warning(0, "The " BOLD("%s") " CVAR is already set to its default.", consolecmds[i].name);
            }
            else
            {
                if (!M_StringCompare(*(char **)consolecmds[i].variable, consolecmds[i].defaultstring))
                {
                    char    *temp = M_StringJoin(parms, " ", (*consolecmds[i].defaultstring ? consolecmds[i].defaultstring : EMPTYVALUE),
                                NULL);

                    C_ValidateInput(temp);
                    C_Output("The " BOLD("%s") " CVAR has been reset to its default.", consolecmds[i].name);
                    free(temp);
                }
                else
                    C_Warning(0, "The " BOLD("%s") " CVAR is already set to its default.", consolecmds[i].name);
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

            if (actions[i].gamecontroller1)
                *(int *)actions[i].gamecontroller1 = 0;

            if (actions[i].gamecontroller2)
                *(int *)actions[i].gamecontroller2 = 0;
        }

        for (int i = 0; i < NUMKEYS; i++)
            keyactionlist[i][0] = '\0';

        for (int i = 0; i < MAX_MOUSE_BUTTONS + 2; i++)
            mouseactionlist[i][0] = '\0';

        // reset stretched sky
        if (gamestate == GS_LEVEL)
        {
            suppresswarnings = true;
            R_InitSkyMap();
            suppresswarnings = false;

            R_InitColumnFunctions();
        }

        // set all controls to defaults
        keyboardalwaysrun = KEYALWAYSRUN_DEFAULT;
        keyboardautomap = KEYAUTOMAP_DEFAULT;
        keyboardback = KEYDOWN_DEFAULT;
        keyboardback2 = KEYDOWN2_DEFAULT;
        keyboardclearmark = KEYCLEARMARK_DEFAULT;
        keyboardconsole = KEYCONSOLE_DEFAULT;
        keyboardfire = KEYFIRE_DEFAULT;
        keyboardfollowmode = KEYFOLLOWMODE_DEFAULT;
        keyboardforward = KEYUP_DEFAULT;
        keyboardforward2 = KEYUP2_DEFAULT;
        keyboardgrid = KEYGRID_DEFAULT;
        keyboardjump = KEYJUMP_DEFAULT;
        keyboardleft = KEYLEFT_DEFAULT;
        keyboardmark = KEYMARK_DEFAULT;
        keyboardmaxzoom = KEYMAXZOOM_DEFAULT;
        keyboardmenu = KEYMENU_DEFAULT;
        keyboardmouselook = KEYMOUSELOOK_DEFAULT;
        keyboardnextweapon = KEYNEXTWEAPON_DEFAULT;
        keyboardprevweapon = KEYPREVWEAPON_DEFAULT;
        keyboardright = KEYRIGHT_DEFAULT;
        keyboardrotatemode = KEYROTATEMODE_DEFAULT;
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
        keyboardzoomin = KEYZOOMIN_DEFAULT;
        keyboardzoomout = KEYZOOMOUT_DEFAULT;

        mouseback = MOUSEBACK_DEFAULT;
        mousefire = MOUSEFIRE_DEFAULT;
        mouseforward = MOUSEFORWARD_DEFAULT;
        mousejump = MOUSEJUMP_DEFAULT;
        mouseleft = MOUSELEFT_DEFAULT;
        mousemouselook = MOUSEMOUSELOOK_DEFAULT;
        mousenextweapon = MOUSENEXTWEAPON_DEFAULT;
        mouseprevweapon = MOUSEPREVWEAPON_DEFAULT;
        mouseright = MOUSERIGHT_DEFAULT;
        mouserun = MOUSERUN_DEFAULT;
        mousescreenshot = MOUSESCREENSHOT_DEFAULT;
        mousestrafe = MOUSESTRAFE_DEFAULT;
        mousestrafeleft = MOUSESTRAFELEFT_DEFAULT;
        mousestraferight = MOUSESTRAFERIGHT_DEFAULT;
        mouseuse = MOUSEUSE_DEFAULT;
        mouseweapon1 = MOUSEWEAPON1_DEFAULT;
        mouseweapon2 = MOUSEWEAPON2_DEFAULT;
        mouseweapon3 = MOUSEWEAPON3_DEFAULT;
        mouseweapon4 = MOUSEWEAPON4_DEFAULT;
        mouseweapon5 = MOUSEWEAPON5_DEFAULT;
        mouseweapon6 = MOUSEWEAPON6_DEFAULT;
        mouseweapon7 = MOUSEWEAPON7_DEFAULT;

        gamecontrolleralwaysrun = GAMECONTROLLERALWAYSRUN_DEFAULT;
        gamecontrollerautomap = GAMECONTROLLERAUTOMAP_DEFAULT;
        gamecontrollerback = GAMECONTROLLERBACK_DEFAULT;
        gamecontrollerclearmark = GAMECONTROLLERCLEARMARK_DEFAULT;
        gamecontrollerconsole = GAMECONTROLLERCONSOLE_DEFAULT;
        gamecontrollerfire = GAMECONTROLLERFIRE_DEFAULT;
        gamecontrollerfollowmode = GAMECONTROLLERFOLLOWMODE_DEFAULT;
        gamecontrollerforward = GAMECONTROLLERFORWARD_DEFAULT;
        gamecontrollergrid = GAMECONTROLLERGRID_DEFAULT;
        gamecontrollerjump = GAMECONTROLLERJUMP_DEFAULT;
        gamecontrollerleft = GAMECONTROLLERLEFT_DEFAULT;
        gamecontrollermark = GAMECONTROLLERMARK_DEFAULT;
        gamecontrollermaxzoom = GAMECONTROLLERMAXZOOM_DEFAULT;
        gamecontrollermenu = GAMECONTROLLERMENU_DEFAULT;
        gamecontrollermouselook = GAMECONTROLLERMOUSELOOK_DEFAULT;
        gamecontrollernextweapon = GAMECONTROLLERNEXTWEAPON_DEFAULT;
        gamecontrollerprevweapon = GAMECONTROLLERPREVWEAPON_DEFAULT;
        gamecontrollerright = GAMECONTROLLERRIGHT_DEFAULT;
        gamecontrollerrotatemode = GAMECONTROLLERROTATEMODE_DEFAULT;
        gamecontrollerrun = GAMECONTROLLERRUN_DEFAULT;
        gamecontrollerstrafe = GAMECONTROLLERSTRAFE_DEFAULT;
        gamecontrollerstrafeleft = GAMECONTROLLERSTRAFELEFT_DEFAULT;
        gamecontrollerstraferight = GAMECONTROLLERSTRAFERIGHT_DEFAULT;
        gamecontrolleruse = GAMECONTROLLERUSE_DEFAULT;
        gamecontrolleruse2 = GAMECONTROLLERUSE2_DEFAULT;
        gamecontrollerweapon1 = GAMECONTROLLERWEAPON_DEFAULT;
        gamecontrollerweapon2 = GAMECONTROLLERWEAPON_DEFAULT;
        gamecontrollerweapon3 = GAMECONTROLLERWEAPON_DEFAULT;
        gamecontrollerweapon4 = GAMECONTROLLERWEAPON_DEFAULT;
        gamecontrollerweapon5 = GAMECONTROLLERWEAPON_DEFAULT;
        gamecontrollerweapon6 = GAMECONTROLLERWEAPON_DEFAULT;
        gamecontrollerweapon7 = GAMECONTROLLERWEAPON_DEFAULT;
        gamecontrollerzoomin = GAMECONTROLLERZOOMIN_DEFAULT;
        gamecontrollerzoomout = GAMECONTROLLERZOOMOUT_DEFAULT;

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

    quicksaveslot = -1;

    G_DoLoadLevel();
    C_HideConsoleFast();
}

//
// resurrect CCMD
//
static int      resurrectcmdtype = NUMMOBJTYPES;
static mobj_t   *resurrectcmdmobj;

static bool resurrect_cmd_func1(char *cmd, char *parms)
{
    bool    result = false;
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
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
                    M_StringReplaceAll(parm, "all", "", false);

                resurrectcmdtype = mobjinfo[i].doomednum;

                if (resurrectcmdtype >= 0
                    && ((*mobjinfo[i].name1 && M_StringCompare(parm, temp1))
                        || (*mobjinfo[i].plural1 && M_StringCompare(parm, temp2))
                        || (*mobjinfo[i].name2 && M_StringCompare(parm, temp3))
                        || (*mobjinfo[i].plural2 && M_StringCompare(parm, temp4))
                        || (*mobjinfo[i].name3 && M_StringCompare(parm, temp5))
                        || (*mobjinfo[i].plural3 && M_StringCompare(parm, temp6))
                        || (sscanf(parm, "%10i", &num) == 1 && num == resurrectcmdtype && num != -1)))
                {
                    if (resurrectcmdtype == WolfensteinSS && !allowwolfensteinss)
                        result = false;
                    else
                        result = (mobjinfo[i].flags & MF_SHOOTABLE);
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
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMEWARNING, "you", "are");
            else
                C_Warning(0, NOGAMEWARNING, playername, "is");
        }
    }
    else
    {
        char    buffer[1024];
        bool    cheated = false;

        if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        {
            P_ResurrectPlayer(initial_health);
            M_snprintf(buffer, sizeof(buffer), "%s resurrected %s.",
                playername,
                (M_StringCompare(playername, playername_default) ? "yourself" : pronoun(reflexive)));
            buffer[0] = toupper(buffer[0]);
            C_PlayerMessage(buffer);
            C_HideConsole();
            HU_SetPlayerMessage(buffer, false, false);
            cheated = true;
        }
        else
        {
            bool    friends = (M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
                        || M_StringCompare(parm, "friendly monster") || M_StringCompare(parm, "friendly monsters"));
            bool    enemies = (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters"));
            bool    all = M_StringCompare(parm, "all");
            int     resurrected = 0;

            if (friends || enemies || all)
            {
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
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
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                    {
                        if (type == thing->type && (thing->flags & MF_CORPSE) && !(thing->flags2 & MF2_DECORATION)
                            && type != MT_PLAYER && thing->info->raisestate != S_NULL)
                        {
                            P_ResurrectMobj(thing);
                            resurrected++;

                            if (thing->flags & MF_FRIEND)
                                cheated = true;
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
                        if (resurrectcmdtype >= ArchVile && resurrectcmdtype <= MonsterSpawner)
                        {
                            C_Warning(0, "There are no %s in " ITALICS("%s."), mobjinfo[type].plural1, gamedescription);
                            return;
                        }
                        else if (gamemode == shareware && (resurrectcmdtype == Cyberdemon || resurrectcmdtype == SpiderMastermind))
                        {
                            C_Warning(0, "There are no %s in " ITALICS("%s."), mobjinfo[type].plural1, gamedescription);
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
        return;
    }

    M_snprintf(buffer, sizeof(buffer), "%s%s%s",
        (M_StringStartsWith(parms, savegamefolder) ? "" : savegamefolder), parms, (M_StringEndsWith(parms, ".save") ? "" : ".save"));
    G_SaveGame(-1, "", buffer);
}

//
// spawn CCMD
//
static int  spawncmdtype = NUMMOBJTYPES;
static bool spawncmdfriendly;

static bool spawn_cmd_func1(char *cmd, char *parms)
{
    bool    result = false;
    char    *parm = removenonalpha(parms);

    if (!*parm)
        return true;

    if (gamestate == GS_LEVEL)
    {
        int num = -1;

        if ((spawncmdfriendly = M_StringStartsWith(parm, "friendly")))
            M_StringReplaceAll(parm, "friendly", "", false);

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
                    || (sscanf(parm, "%10i", &num) == 1 && num == spawncmdtype && num != -1))
                && (!spawncmdfriendly || (mobjinfo[i].flags & MF_SHOOTABLE)))
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMEWARNING, "you", "are");
            else
                C_Warning(0, NOGAMEWARNING, playername, "is");
        }
    }
    else
    {
        bool                spawn = true;
        const mobjtype_t    type = P_FindDoomedNum(spawncmdtype);

        if (gamemode != commercial)
        {
            char    buffer[128];

            if (spawncmdtype >= ArchVile && spawncmdtype <= MonsterSpawner && !REKKR)
            {
                M_StringCopy(buffer, mobjinfo[type].plural1, sizeof(buffer));

                if (!*buffer)
                    M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[type].name1);

                buffer[0] = toupper(buffer[0]);
                C_Warning(0, "%s can't be spawned in " ITALICS("%s."), buffer, gamedescription);
                spawn = false;
            }

            if (gamemode == shareware
                && (spawncmdtype == Cyberdemon || spawncmdtype == SpiderMastermind
                    || spawncmdtype == Berserk || spawncmdtype == HelperDog))
            {
                M_StringCopy(buffer, mobjinfo[type].plural1, sizeof(buffer));

                if (!*buffer)
                    M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[type].name1);

                buffer[0] = toupper(buffer[0]);
                C_Warning(0, "%s can't be spawned in " ITALICS("%s."), buffer, gamedescription);
                spawn = false;
            }
        }
        else if (spawncmdtype == WolfensteinSS && !allowwolfensteinss)
        {
            if (bfgedition)
                C_Warning(0, "%s%s can't be spawned in " ITALICS("%s (%s)."),
                    (spawncmdfriendly ? "Friendly " : ""), mobjinfo[type].name1, gamedescription, s_CAPTION_BFGEDITION);
            else
                C_Warning(0, "%s%s can't be spawned in " ITALICS("%s."),
                    (spawncmdfriendly ? "Friendly " : ""), mobjinfo[type].name1, gamedescription);

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
                    const int   flags = thing->flags;
                    mobj_t      *fog;

                    thing->angle = R_PointToAngle2(x, y, viewx, viewy);
                    thing->id = thingid++;

                    if (flags & MF_SHOOTABLE)
                    {
                        if (spawncmdfriendly)
                        {
                            thing->flags |= MF_FRIEND;
                            stat_cheated = SafeAdd(stat_cheated, 1);
                            M_SaveCVARs();
                        }

                        thing->flags2 |= MF2_SPAWNEDBYPLAYER;

                        if (flags & MF_NOGRAVITY)
                        {
                            thing->z = thing->floorz + 32 * FRACUNIT;
                            fog = P_SpawnMobj(x, y, thing->z, MT_TFOG);
                        }
                        else
                            fog = P_SpawnMobj(x, y, ONFLOORZ, MT_TFOG);

                        massacre = false;

                        S_StartSound(fog, sfx_telept);
                    }
                    else
                    {
                        if (flags & MF_COUNTITEM)
                        {
                            stat_cheated = SafeAdd(stat_cheated, 1);
                            M_SaveCVARs();
                        }

                        fog = P_SpawnMobj(x, y, ((flags & MF_SPAWNCEILING) ? ONCEILINGZ :
                            ((thing->flags2 & MF2_FLOATBOB) ? thing->floorz + 14 * FRACUNIT : ONFLOORZ)), MT_IFOG);

                        S_StartSound(fog, sfx_itmbk);
                    }

                    fog->angle = thing->angle;

                    if (thing->type == MT_MISC0 || thing->type == MT_MISC1)
                        C_PlayerMessage("%s spawned %s.",
                            (M_StringCompare(playername, playername_default) ? "You" : playername),
                            mobjinfo[type].name1);
                    else
                        C_PlayerMessage("%s spawned %s %s%s.",
                            (M_StringCompare(playername, playername_default) ? "You" : playername),
                            (isvowel(mobjinfo[type].name1[0]) ? "an" : "a"),
                            (spawncmdfriendly ? "friendly " : ""),
                            mobjinfo[type].name1);

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
static bool take_cmd_func1(char *cmd, char *parms)
{
    bool    result = false;
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
        return true;

    if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything")
        || M_StringCompare(parm, "health") || M_StringCompare(parm, "fullhealth")
        || M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons")
        || M_StringCompare(parm, "ammo") || M_StringCompare(parm, "fullammo")
        || M_StringCompare(parm, "ammunition") || M_StringCompare(parm, "fullammunition")
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
                    || (sscanf(parm, "%10i", &num) == 1 && num == mobjinfo[i].doomednum && num != -1)))
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

    if (!*parm || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMEWARNING, "you", "are");
            else
                C_Warning(0, NOGAMEWARNING, playername, "is");
        }
    }
    else
    {
        bool    result = false;

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
                P_DamageMobj(viewplayer->mo, viewplayer->mo, viewplayer->mo, viewplayer->health - initial_health, false, false);
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
            {
                C_PlayerMessage("Everything was taken from %s.", playername);
                C_HideConsole();
            }
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
                P_DamageMobj(viewplayer->mo, viewplayer->mo, viewplayer->mo,
                    viewplayer->health - !!(viewplayer->cheats & CF_BUDDHA), false, false);
                healthcvar = false;

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You killed yourself.");
                else
                    C_PlayerMessage("%s killed %s.", playername, pronoun(reflexive));

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You are already dead.");
                else
                    C_Warning(0, "%s is already dead.", playername);
            }
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

            viewplayer->pendingweapon = wp_fist;

            if (result)
            {
                C_PlayerMessage("All weapons were taken from %s.", playername);
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any weapons.");
            else
                C_Warning(0, "%s doesn't have any weapons.", playername);
        }
        else if (M_StringCompare(parm, "ammo") || M_StringCompare(parm, "allammo")
                || M_StringCompare(parm, "ammunition") || M_StringCompare(parm, "allammunition"))
        {
            for (ammotype_t i = 0; i < NUMAMMO; i++)
                if (viewplayer->ammo[i])
                {
                    viewplayer->ammo[i] = 0;
                    result = true;
                }

            viewplayer->pendingweapon = wp_fist;

            if (result)
            {
                C_PlayerMessage("All ammo was taken from %s.", playername);
                C_HideConsole();
            }
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
                C_PlayerMessage("All %s was taken from %s.",
                    (english == english_american ? "armor" : "armour"), playername);
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any %s.",
                    (english == english_american ? "armor" : "armour"));
            else
                C_Warning(0, "%s doesn't have any %s.",
                    playername, (english == english_american ? "armor" : "armour"));
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
            {
                C_PlayerMessage("All keycards and skull keys were taken from %s.", playername);
                C_HideConsole();
            }
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
                C_PlayerMessage("All keycards were taken from %s.", playername);
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
                C_PlayerMessage("All skull keys were taken from %s.", playername);
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

                P_CheckAmmo(viewplayer->readyweapon);

                C_PlayerMessage("A pistol was taken from %s.", playername);
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have a pistol.");
            else
                C_Warning(0, "%s doesn't have a pistol.", playername);
        }
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
                        || (sscanf(parm, "%10i", &num) == 1 && num == mobjinfo[i].doomednum && num != -1)))
                {
                    if (P_TakeSpecialThing(i))
                    {
                        C_PlayerMessage("%s %s was taken from %s.",
                            (isvowel(mobjinfo[i].name1[0]) ? "An" : "A"), mobjinfo[i].name1, playername);
                        C_HideConsole();
                        result = true;
                    }
                    else if (M_StringCompare(playername, playername_default))
                        C_Warning(0, "You don't have %s %s.",
                            (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                    else
                        C_Warning(0, "%s doesn't have %s %s.",
                            playername, (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
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

        free(parm);
    }
}

//
// teleport CCMD
//
static bool teleport_cmd_func1(char *cmd, char *parms)
{
    if (!*parms || gamestate != GS_LEVEL)
        return true;
    else
    {
        fixed_t x, y;

        return (sscanf(parms, "%10i %10i", &x, &y) == 2);
    }
}

static void teleport_cmd_func2(char *cmd, char *parms)
{
    if (!*parms || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMEWARNING, "you", "are");
            else
                C_Warning(0, NOGAMEWARNING, playername, "is");
        }
    }
    else
    {
        fixed_t x, y;
        fixed_t z = ONFLOORZ;

        if (sscanf(parms, "%10i %10i %10i", &x, &y, &z) >= 2)
        {
            mobj_t          *mo = viewplayer->mo;
            const fixed_t   oldx = viewx;
            const fixed_t   oldy = viewy;
            const fixed_t   oldz = mo->z;

            x <<= FRACBITS;
            y <<= FRACBITS;

            if (z != ONFLOORZ)
                z <<= FRACBITS;

            if (x == oldx && y == oldy)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You are already there.");
                else
                    C_Warning(0, "%s is already there.", playername);
            }
            else if (P_TeleportMove(mo, x, y, z, false))
            {
                // spawn teleport fog at source
                mobj_t  *fog = P_SpawnMobj(oldx, oldy, oldz, MT_TFOG);

                fog->angle = viewangle;
                S_StartSound(fog, sfx_telept);

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

                if (z == ONFLOORZ)
                {
                    if (M_StringCompare(playername, playername_default))
                        C_PlayerMessage("You teleported to (%i, %i).", x >> FRACBITS, y >> FRACBITS);
                    else
                        C_PlayerMessage("%s teleported to (%i, %i).", playername, x >> FRACBITS, y >> FRACBITS);
                }
                else
                {
                    if (M_StringCompare(playername, playername_default))
                        C_PlayerMessage("You teleported to (%i, %i, %i).",
                            x >> FRACBITS, y >> FRACBITS, z >> FRACBITS);
                    else
                        C_PlayerMessage("%s teleported to (%i, %i, %i).",
                            playername, x >> FRACBITS, y >> FRACBITS, z >> FRACBITS);
                }

                C_HideConsoleFast();
            }
        }
    }
}

//
// thinglist CCMD
//
static void thinglist_cmd_func2(char *cmd, char *parms)
{
    const int   tabs[3] = { 50, 300, 0 };

    C_Header(tabs, thinglist, THINGLISTHEADER);

    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        mobj_t  *mobj = (mobj_t *)th;
        char    name[128];
        char    *temp1 = commify(mobj->id);
        char    *temp2;

        if (mobj == viewplayer->mo)
            M_StringCopy(name, playername, sizeof(name));
        else if (*mobj->name)
            M_StringCopy(name, mobj->name, sizeof(name));
        else
            M_snprintf(name, sizeof(name), "%s%s%s", ((mobj->flags & MF_CORPSE) && !(mobj->flags2 & MF2_DECORATION) ? "dead " :
                ((mobj->flags & MF_FRIEND) && mobj->type != MT_PLAYER ? "friendly " : ((mobj->flags & MF_DROPPED) ? "dropped " : ""))),
                (mobj->type == MT_PLAYER ? "voodoo doll" : (*mobj->info->name1 ? mobj->info->name1 : "-")),
                ((mobj->flags & MF_MISSILE) ? " projectile" : ""));

        temp2 = sentencecase(name);
        C_TabbedOutput(tabs, "%s%s\t%s\t(%i, %i, %i)", (mobj->id >= 0 ? temp1 : "-"), (mobj->id >= 0 ? "." : ""),
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
    }
    else
    {
        int value;

        if (M_StringCompare(parms, "off"))
            value = 0;
        else if (sscanf(parms, "%10i", &value) != 1)
            return;

        value = BETWEEN(0, value, TIMERMAXMINUTES);

        if (!togglingvanilla)
        {
            if (!value)
            {
                if (timer)
                    C_Output("The timer has been cleared.");
                else
                    C_Warning(0, "No timer has been set.");
            }
            else
            {
                char    *temp = commify(value);

                if (timer)
                    C_Output("The timer has been %s to %s minute%s. %s will automatically exit each map once the timer expires.",
                        temp, (value == timer ? "reset" : "changed"), (value == 1 ? "" : "s"),
                        (M_StringCompare(playername, playername_default) ? "You" : playername));
                else
                    C_Output("A timer has been set for %s minute%s. %s will automatically exit each map once the timer expires.",
                        temp, (value == 1 ? "" : "s"), (M_StringCompare(playername, playername_default) ? "You" : playername));

                free(temp);
            }
        }

        P_SetTimer(value);
    }
}

//
// toggle CCMD
//
static void toggle_cmd_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
        return;
    }

    for (int i = 0; *consolecmds[i].name; i++)
    {
        const int   flags = consolecmds[i].flags;

        if (consolecmds[i].type == CT_CVAR && M_StringCompare(parms, consolecmds[i].name) && !(flags & CF_READONLY))
        {
            if (flags & CF_BOOLEAN)
            {
                bool    value = *(bool *)consolecmds[i].variable;
                char    *temp = M_StringJoin(parms, " ", C_LookupAliasFromValue(!value, consolecmds[i].aliases), NULL);

                C_ValidateInput(temp);
                free(temp);
                M_SaveCVARs();
                break;
            }
            else if (flags & CF_INTEGER)
            {
                int     value = *(int *)consolecmds[i].variable;
                char    *temp;

                if (++value > consolecmds[i].maximumvalue)
                    value = consolecmds[i].minimumvalue;

                temp = M_StringJoin(parms, " ", C_LookupAliasFromValue(value, consolecmds[i].aliases), NULL);
                C_ValidateInput(temp);
                free(temp);
                M_SaveCVARs();
                break;
            }
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_ShowFormat(i);
        return;
    }

    bind_cmd_func2(cmd, parms);
}

//
// vanilla CCMD
//
static void vanilla_cmd_func2(char *cmd, char *parms)
{
    static bool buddha;
    static bool hud;
    static bool showfps;

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

    if (vanilla)
    {
        SC_Open("VANILLA");

        while (SC_GetString())
        {
            char    *temp1 = M_StringDuplicate(sc_String);
            char    *temp2;

            SC_GetString();
            temp2 = M_StringJoin(temp1, " ", sc_String, NULL);
            C_ValidateInput(temp2);
            free(temp1);
            free(temp2);
        }

        SC_Close();

        if ((buddha = (viewplayer->cheats & CF_BUDDHA)))
            viewplayer->cheats &= ~CF_BUDDHA;

        hud = r_hud;
        showfps = vid_showfps;

        C_Output(s_STSTR_VON);
        HU_SetPlayerMessage(s_STSTR_VON, false, false);
        C_Warning(0, "Changes to any CVARs won't be saved while vanilla mode is on.");
    }
    else
    {
        if (buddha)
            viewplayer->cheats |= CF_BUDDHA;

        r_hud = hud;
        vid_showfps = showfps;

        M_LoadCVARs(configfile);
        C_Output(s_STSTR_VOFF);
        HU_SetPlayerMessage(s_STSTR_VOFF, false, false);
    }

    message_dontfuckwithme = true;

    if (gamestate == GS_LEVEL)
        C_HideConsole();

    togglingvanilla = false;
}

//
// bool CVARs
//
static bool bool_cvars_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, BOOLVALUEALIAS) != INT_MIN);
}

static void bool_cvars_func2(char *cmd, char *parms)
{
    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_BOOLEAN))
        {
            if (*parms && !(consolecmds[i].flags & CF_READONLY))
            {
                const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

                if ((value == 0 || value == 1) && value != *(bool *)consolecmds[i].variable)
                {
                    *(bool *)consolecmds[i].variable = value;
                    M_SaveCVARs();
                }
            }
            else
            {
                char    *temp1 = C_LookupAliasFromValue(*(bool *)consolecmds[i].variable, BOOLVALUEALIAS);

                C_ShowDescription(i);

                if (*(bool *)consolecmds[i].variable == (bool)consolecmds[i].defaultnumber)
                    C_Output(INTEGERCVARISDEFAULT, temp1);
                else
                {
                    char    *temp2 = C_LookupAliasFromValue((bool)consolecmds[i].defaultnumber, BOOLVALUEALIAS);

                    C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
                    free(temp2);
                }

                C_ShowWarning(i);

                free(temp1);
            }

            break;
        }
}

//
// color CVARs
//
static void color_cvars_func2(char *cmd, char *parms)
{
    int_cvars_func2(cmd, parms);

    if (*parms)
        AM_SetColors();
}

//
// float CVARs
//
static bool float_cvars_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_FLOAT))
        {
            float   value;

            return (sscanf(parms, "%10f", &value) == 1);
        }

    return false;
}

static void float_cvars_func2(char *cmd, char *parms)
{
    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_FLOAT))
        {
            if (*parms && !(consolecmds[i].flags & CF_READONLY))
            {
                float   value;

                if (sscanf(parms, "%10f", &value) == 1 && value != *(float *)consolecmds[i].variable)
                {
                    *(float *)consolecmds[i].variable = value;
                    M_SaveCVARs();
                }
            }
            else
            {
                char    *temp1 = striptrailingzero(*(float *)consolecmds[i].variable, 1);

                C_ShowDescription(i);

                if (*(float *)consolecmds[i].variable == (float)consolecmds[i].defaultnumber)
                    C_Output(((consolecmds[i].flags & CF_READONLY) ? INTEGERCVARWITHNODEFAULT : INTEGERCVARISDEFAULT), temp1);
                else
                {
                    char    *temp2 = striptrailingzero(consolecmds[i].defaultnumber, 1);

                    C_Output(((consolecmds[i].flags & CF_READONLY) ? INTEGERCVARWITHNODEFAULT : INTEGERCVARWITHDEFAULT), temp1, temp2);
                    free(temp2);
                }

                free(temp1);

                C_ShowWarning(i);
            }

            break;
        }
}

//
// integer CVARs
//
static bool int_cvars_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR && (consolecmds[i].flags & CF_INTEGER))
        {
            int value = C_LookupValueFromAlias(parms, consolecmds[i].aliases);

            return ((value != INT_MIN || sscanf(parms, "%10i", &value) == 1)
                && value >= consolecmds[i].minimumvalue && value <= consolecmds[i].maximumvalue);
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

                if ((value != INT_MIN || sscanf(parms, "%10i", &value) == 1) && value != *(int *)consolecmds[i].variable)
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

                    if (*(int *)consolecmds[i].variable == (int)consolecmds[i].defaultnumber)
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

                    if (*(int *)consolecmds[i].variable == (int)consolecmds[i].defaultnumber)
                        C_Output(((consolecmds[i].flags & CF_READONLY) ? INTEGERCVARWITHNODEFAULT : INTEGERCVARISDEFAULT), temp1);
                    else
                    {
                        char    *temp2 = C_LookupAliasFromValue((int)consolecmds[i].defaultnumber, consolecmds[i].aliases);

                        C_Output(((consolecmds[i].flags & CF_READONLY) ? INTEGERCVARWITHNODEFAULT : INTEGERCVARWITHDEFAULT), temp1, temp2);
                        free(temp2);
                    }

                    free(temp1);
                }

                C_ShowWarning(i);
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
                    *(char **)consolecmds[i].variable = M_StringDuplicate(parms);
                    M_StripQuotes(*(char **)consolecmds[i].variable);
                    M_SaveCVARs();
                }
            }
            else
            {
                C_ShowDescription(i);

                if (consolecmds[i].flags & CF_READONLY)
                    C_Output(STRINGCVARWITHNODEFAULT,
                        (M_StringCompare(consolecmds[i].name, "version") ? "" : "\""), *(char **)consolecmds[i].variable,
                        (M_StringCompare(consolecmds[i].name, "version") ? "" : "\""));
                else if (M_StringCompare(*(char **)consolecmds[i].variable, consolecmds[i].defaultstring))
                    C_Output(STRINGCVARISDEFAULT, *(char **)consolecmds[i].variable);
                else
                    C_Output(STRINGCVARWITHDEFAULT, *(char **)consolecmds[i].variable, consolecmds[i].defaultstring);

                C_ShowWarning(i);
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
            int         tics = *(int *)consolecmds[i].variable / TICRATE;
            const int   hours = tics / 3600;
            const int   minutes = ((tics %= 3600)) / 60;
            const int   seconds = tics % 60;

            C_ShowDescription(i);

            if (hours)
                C_Output(TIMECVARWITHNODEFAULT2, hours, minutes, seconds);
            else
                C_Output(TIMECVARWITHNODEFAULT1, minutes, seconds);

            C_ShowWarning(i);
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
// am_display CVAR
//
static void am_display_cvar_func2(char *cmd, char *parms)
{
    const int   am_display_old = am_display;

    int_cvars_func2(cmd, parms);

    if (am_display != am_display_old && am_external)
    {
        I_DestroyExternalAutomap();

        if (I_CreateExternalAutomap())
        {
            if (gamestate == GS_LEVEL)
                AM_Start(false);
        }
        else
        {
            mapscreen = *screens;

            if (gamestate == GS_LEVEL)
                AM_Stop();
        }

        AM_SetAutomapSize(r_screensize);
    }
}

//
// am_external CVAR
//
static void am_external_cvar_func2(char *cmd, char *parms)
{
    const bool  am_external_old = am_external;

    bool_cvars_func2(cmd, parms);

    if (am_external != am_external_old)
    {
        if (am_external)
        {
            if (I_CreateExternalAutomap())
            {
                if (gamestate == GS_LEVEL)
                    AM_Start(false);
            }
        }
        else
        {
            I_DestroyExternalAutomap();
            mapscreen = *screens;

            if (gamestate == GS_LEVEL)
                AM_Stop();
        }

        AM_SetAutomapSize(r_screensize);
    }
}

//
// am_followmode CVAR
//
static void am_followmode_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const bool  am_followmode_old = am_followmode;

        bool_cvars_func2(cmd, parms);

        if (automapactive && am_followmode != am_followmode_old)
            AM_ToggleFollowMode(am_followmode);
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(am_followmode, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (am_followmode == am_followmode_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(am_followmode_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (M_StringCompare(am_gridsize, am_gridsize_default))
            C_Output(INTEGERCVARISDEFAULT, am_gridsize);
        else
            C_Output(INTEGERCVARWITHDEFAULT, am_gridsize, am_gridsize_default);

        C_ShowWarning(i);
    }
}

//
// am_path CVAR
//
static void am_path_cvar_func2(char *cmd, char *parms)
{
    bool_cvars_func2(cmd, parms);

    if (am_path)
    {
        viewplayer->cheated++;
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }
}

//
// am_rotatemode CVAR
//
static void am_rotatemode_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const bool  am_rotatemode_old = am_rotatemode;

        bool_cvars_func2(cmd, parms);

        if (automapactive && am_rotatemode != am_rotatemode_old)
            AM_ToggleRotateMode(am_rotatemode);
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(am_rotatemode, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (am_rotatemode == am_rotatemode_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(am_rotatemode_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// armortype CVAR
//
static bool armortype_cvar_func1(char *cmd, char *parms)
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
        char        *temp = C_LookupAliasFromValue((gamestate == GS_LEVEL ? viewplayer->armortype : armortype_default),
                        ARMORTYPEVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_Output(INTEGERCVARWITHNODEFAULT, temp);
        C_ShowWarning(i);

        free(temp);
    }
}

//
// autotilt CVAR
//
static void autotilt_cvar_func2(char *cmd, char *parms)
{
    const bool  autotilt_old = autotilt;

    bool_cvars_func2(cmd, parms);

    if (autotilt != autotilt_old && gamestate == GS_LEVEL)
    {
        suppresswarnings = true;
        R_InitSkyMap();
        suppresswarnings = false;

        R_InitColumnFunctions();
    }
}

//
// crosshair CVAR
//
static bool crosshair_cvar_func1(char *cmd, char *parms)
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
        char        *temp1 = C_LookupAliasFromValue(crosshair, CROSSHAIRVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (crosshair == crosshair_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(crosshair_default, CROSSHAIRVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// english CVAR
//
static bool english_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, ENGLISHVALUEALIAS) != INT_MIN);
}

static void english_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, ENGLISHVALUEALIAS);

        if ((value == english_american || value == english_international) && value != english)
        {
            english = value;
            ST_InitStatBar();
            D_TranslateDehStrings();
            M_SaveCVARs();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(english, ENGLISHVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (english == english_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(english_default, ENGLISHVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
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
        EpiDef.laston = (gamemode == registered ? MIN(episode, episode_max - 1) : (gamemode == retail ? episode : 1)) - 1;
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
        ExpDef.laston = (nerve ? expansion - 1 : 0);

        if (gamestate != GS_LEVEL)
            gamemission = (expansion == 2 && nerve ? pack_nerve : doom2);
    }
}

//
// joy_deadzone_left and joy_deadzone_right CVARs
//
static bool joy_deadzone_cvars_func1(char *cmd, char *parms)
{
    float   value;

    return (!*parms || sscanf(parms, "%10f%%", &value) == 1 || sscanf(parms, "%10f", &value) == 1);
}

static void joy_deadzone_cvars_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        float   value;

        if (sscanf(parms, "%10f%%", &value) == 1 || sscanf(parms, "%10f", &value) == 1)
        {
            if (M_StringCompare(cmd, stringize(joy_deadzone_left)))
            {
                if (joy_deadzone_left != value)
                {
                    joy_deadzone_left = BETWEENF(joy_deadzone_left_min, value, joy_deadzone_left_max);
                    I_SetGameControllerLeftDeadZone();
                    M_SaveCVARs();
                }
            }
            else if (joy_deadzone_right != value)
            {
                joy_deadzone_right = BETWEENF(joy_deadzone_right_min, value, joy_deadzone_right_max);
                I_SetGameControllerRightDeadZone();
                M_SaveCVARs();
            }
        }
    }
    else if (M_StringCompare(cmd, stringize(joy_deadzone_left)))
    {
        char        *temp1 = striptrailingzero(joy_deadzone_left, 1);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (joy_deadzone_left == joy_deadzone_left_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = striptrailingzero(joy_deadzone_left_default, 1);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
    else
    {
        char        *temp1 = striptrailingzero(joy_deadzone_right, 1);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (joy_deadzone_right == joy_deadzone_right_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = striptrailingzero(joy_deadzone_right_default, 1);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// joy_sensitivity_horizontal and joy_sensitivity_vertical CVARs
//
static void joy_sensitivity_cvars_func2(char *cmd, char *parms)
{
    const float joy_sensitivity_horizontal_old = joy_sensitivity_horizontal;
    const float joy_sensitivity_vertical_old = joy_sensitivity_vertical;

    float_cvars_func2(cmd, parms);

    if (joy_sensitivity_horizontal != joy_sensitivity_horizontal_old)
        I_SetGameControllerHorizontalSensitivity();
    else if (joy_sensitivity_vertical != joy_sensitivity_vertical_old)
        I_SetGameControllerVerticalSensitivity();
}

//
// mouselook CVAR
//
static void mouselook_cvar_func2(char *cmd, char *parms)
{
    const bool  mouselook_old = mouselook;

    bool_cvars_func2(cmd, parms);

    if (mouselook != mouselook_old)
    {
        if (gamestate == GS_LEVEL)
        {
            suppresswarnings = true;
            R_InitSkyMap();
            suppresswarnings = false;

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
static bool player_cvars_func1(char *cmd, char *parms)
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
        const weapontype_t  readyweapon = viewplayer->readyweapon;
        const ammotype_t    ammotype = weaponinfo[readyweapon].ammotype;

        if (*parms)
        {
            if (sscanf(parms, "%10i", &value) == 1 && ammotype != am_noammo
                && value != viewplayer->ammo[ammotype] && viewplayer->health > 0)
            {
                ammohighlight = I_GetTimeMS() + HUD_AMMO_HIGHLIGHT_WAIT;

                if ((value = MIN(value, viewplayer->maxammo[ammotype])) > viewplayer->ammo[ammotype])
                {
                    P_UpdateAmmoStat(ammotype, value - viewplayer->ammo[ammotype]);
                    P_AddBonus();
                    S_StartSound(NULL, sfx_itemup);
                }

                viewplayer->ammo[ammotype] = value;
                P_CheckAmmo(readyweapon);
                C_HideConsole();
            }
        }
        else
        {
            const int   i = C_GetIndex(cmd);
            char        *temp = commify(ammotype == am_noammo ? 0 : viewplayer->ammo[ammotype]);

            C_ShowDescription(i);
            C_Output(INTEGERCVARWITHNODEFAULT, temp);

            if (gamestate != GS_LEVEL)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, NOGAMEWARNING, "you", "are");
                else
                    C_Warning(0, NOGAMEWARNING, playername, "is");
            }

            free(temp);
        }
    }
    else if (M_StringCompare(cmd, stringize(armor)))
    {
        if (*parms)
        {
            if (sscanf(parms, "%10i", &value) == 1 && value != viewplayer->armorpoints)
            {
                armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;

                if (value > viewplayer->armorpoints)
                {
                    P_UpdateArmorStat(value - viewplayer->armorpoints);
                    P_AddBonus();
                    S_StartSound(NULL, sfx_itemup);
                }

                if (!(viewplayer->armorpoints = MIN(value, max_armor)))
                    viewplayer->armortype = armortype_none;
                else if (!viewplayer->armortype)
                    viewplayer->armortype = green_armor_class;

                C_HideConsole();
            }
        }
        else
        {
            const int   i = C_GetIndex(cmd);
            char        *temp = commify(viewplayer->armorpoints);

            C_ShowDescription(i);
            C_Output(PERCENTCVARWITHNODEFAULT, temp);

            if (gamestate != GS_LEVEL)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, NOGAMEWARNING, "you", "are");
                else
                    C_Warning(0, NOGAMEWARNING, playername, "is");
            }

            free(temp);
        }
    }
    else if (M_StringCompare(cmd, stringize(health)) && !(viewplayer->cheats & CF_GODMODE) && !viewplayer->powers[pw_invulnerability])
    {
        if (*parms)
        {
            if (sscanf(parms, "%10i", &value) == 1 && value != viewplayer->health)
            {
                value = BETWEEN(HUD_NUMBER_MIN, value, maxhealth);

                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;

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
                    }
                    else
                    {
                        char    buffer[1024];

                        P_ResurrectPlayer(value);
                        P_AddBonus();
                        M_snprintf(buffer, sizeof(buffer), "%s resurrected %s.",
                            playername,
                            (M_StringCompare(playername, playername_default) ? "yourself" : pronoun(reflexive)));
                        buffer[0] = toupper(buffer[0]);
                        C_PlayerMessage(buffer);
                    }
                }
                else
                {
                    if (value < viewplayer->health)
                    {
                        healthcvar = true;
                        P_DamageMobj(viewplayer->mo, viewplayer->mo, viewplayer->mo, viewplayer->health - value, false, false);
                        healthcvar = false;
                    }
                    else
                    {
                        P_UpdateHealthStat(value - viewplayer->health);
                        viewplayer->health = value;
                        viewplayer->mo->health = value;
                        P_AddBonus();
                        S_StartSound(NULL, sfx_itemup);
                    }

                    C_HideConsole();
                }
            }
        }
        else
        {
            char        *temp = commify(negativehealth ? viewplayer->health : MAX(0, viewplayer->health));
            const int   i = C_GetIndex(cmd);

            C_ShowDescription(i);
            C_Output(PERCENTCVARWITHNODEFAULT, temp);

            if (gamestate != GS_LEVEL)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, NOGAMEWARNING, "you", "are");
                else
                    C_Warning(0, NOGAMEWARNING, playername, "is");
            }

            free(temp);
        }
    }
}

//
// playergender CVAR
//
static bool playergender_cvar_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, PLAYERGENDERVALUEALIAS) != INT_MIN);
}

static void playergender_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, PLAYERGENDERVALUEALIAS);

        if (value != INT_MIN && playergender != value)
        {
            playergender = value;
            M_SaveCVARs();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(playergender, PLAYERGENDERVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (playergender == playergender_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(playergender_default, PLAYERGENDERVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// playername CVAR
//
static void playername_cvar_func2(char *cmd, char *parms)
{
    char    *temp;

    str_cvars_func2(cmd, (M_StringCompare(parms, EMPTYVALUE) ? playername_default : parms));

    temp = M_StringDuplicate(playername);

    if (M_StringCompare(temp, playername_default))
        temp = lowercase(temp);
    else
        temp[0] = toupper(temp[0]);

    playername = temp;
}

//
// r_blood CVAR
//
static bool r_blood_cvar_func1(char *cmd, char *parms)
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
            R_InitColumnFunctions();

            for (int i = 0; i < numsectors; i++)
                for (bloodsplat_t *splat = sectors[i].splatlist; splat; splat = splat->next)
                    P_SetBloodSplatColor(splat);
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_blood, BLOODVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_blood == r_blood_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_blood_default, BLOODVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
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
                for (bloodsplat_t *splat = sectors[i].splatlist; splat; splat = splat->next)
                    P_SetBloodSplatColor(splat);
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_bloodsplats_translucency, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_bloodsplats_translucency == r_bloodsplats_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_bloodsplats_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_brightmaps CVAR
//
static void r_brightmaps_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_brightmaps)
        {
            r_brightmaps = value;
            M_SaveCVARs();
            I_SetPalette(&PLAYPAL[st_palette * 768]);
            R_InitColumnFunctions();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_brightmaps, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_brightmaps == r_brightmaps_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_brightmaps_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
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
        I_SetPalette(&PLAYPAL[st_palette * 768]);
}

//
// r_corpses_mirrored CVAR
//
static void r_corpses_mirrored_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_corpses_mirrored)
        {
            r_corpses_mirrored = value;
            M_SaveCVARs();

            for (int i = 0; i < numsectors; i++)
                for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                {
                    if ((thing->flags & MF_CORPSE)
                        && !(thing->flags2 & MF2_NOMIRROREDCORPSE)
                        && (thing->type != MT_PAIN || !doom4vanilla))
                    {
                        if (r_corpses_mirrored)
                        {
                            if (M_BigRandom() & 1)
                                thing->flags2 |= MF2_MIRRORED;
                        }
                        else
                            thing->flags2 &= ~MF2_MIRRORED;
                    }
                }
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_corpses_mirrored, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_corpses_mirrored == r_corpses_mirrored_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_corpses_mirrored_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_detail CVAR
//
static bool r_detail_cvar_func1(char *cmd, char *parms)
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
            STLib_Init();
            R_InitColumnFunctions();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_detail, DETAILVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_detail == r_detail_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_detail_default, DETAILVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_ditheredlighting CVAR
//
static void r_ditheredlighting_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_ditheredlighting)
        {
            r_ditheredlighting = value;
            M_SaveCVARs();
            I_SetPalette(&PLAYPAL[st_palette * 768]);
            R_InitColumnFunctions();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_ditheredlighting, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_ditheredlighting == r_ditheredlighting_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_ditheredlighting_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
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
                C_Warning(0, NEXTMAPWARNING);
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_fixmaperrors, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_fixmaperrors == r_fixmaperrors_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_fixmaperrors_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_fov CVAR
//
static void r_fov_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        int value;

        if (sscanf(parms, "%10i", &value) == 1 && value != r_fov)
        {
            r_fov = value;
            M_SaveCVARs();
            setsizeneeded = true;
            R_ExecuteSetViewSize();

            if (gamestate == GS_LEVEL)
                S_StartSound(NULL, sfx_stnmov);
        }
    }
    else
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_fov == r_fov_default)
            C_Output(DEGREESCVARISDEFAULT, r_fov);
        else
            C_Output(DEGREESCVARWITHDEFAULT, r_fov, r_fov_default);

        C_ShowWarning(i);
    }
}

//
// r_gamma CVAR
//
static bool r_gamma_cvar_func1(char *cmd, char *parms)
{
    return (C_LookupValueFromAlias(parms, GAMMAVALUEALIAS) != INT_MIN || float_cvars_func1(cmd, parms));
}

static void r_gamma_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        float   value = (float)C_LookupValueFromAlias(parms, GAMMAVALUEALIAS);

        if ((value != INT_MIN || sscanf(parms, "%10f", &value) == 1) && value != r_gamma)
        {
            r_gamma = BETWEENF(r_gamma_min, value, r_gamma_max);
            I_SetGamma(r_gamma);
            I_SetPalette(&PLAYPAL[st_palette * 768]);
            M_SaveCVARs();
        }
    }
    else
    {
        char        buffer1[128];
        int         len;
        const int   i = C_GetIndex(cmd);

        M_snprintf(buffer1, sizeof(buffer1), "%.2f", r_gamma);
        len = (int)strlen(buffer1);

        if (len >= 2 && buffer1[len - 1] == '0' && buffer1[len - 2] == '0')
            buffer1[len - 1] = '\0';

        C_ShowDescription(i);

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

        C_ShowWarning(i);
    }
}

//
// r_hud CVAR
//
static void r_hud_cvar_func2(char *cmd, char *parms)
{
    if (r_screensize == r_screensize_max || !*parms || resettingcvar)
        bool_cvars_func2(cmd, parms);
}

//
// r_hud_translucency CVAR
//
static void r_hud_translucency_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_hud_translucency)
        {
            r_hud_translucency = value;
            M_SaveCVARs();
            HU_SetTranslucency();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_hud_translucency, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_hud_translucency == r_hud_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_hud_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
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
        M_SaveCVARs();
        GetPixelSize();
    }
    else
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (M_StringCompare(r_lowpixelsize, r_lowpixelsize_default))
            C_Output(INTEGERCVARISDEFAULT, r_lowpixelsize);
        else
            C_Output(INTEGERCVARWITHDEFAULT, r_lowpixelsize, r_lowpixelsize_default);

        C_ShowWarning(i);
    }
}

//
// r_mirroredweapons CVAR
//
static void r_mirroredweapons_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_mirroredweapons)
        {
            r_mirroredweapons = value;
            M_SaveCVARs();

            for (int i = 0; i < numsectors; i++)
                for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                {
                    const mobjtype_t    type = thing->type;

                    if ((type >= MT_MISC25 && type <= MT_SUPERSHOTGUN) || (thing->flags & MF_DROPPED))
                    {
                        if (r_mirroredweapons)
                        {
                            if (M_BigRandom() & 1)
                                thing->flags2 |= MF2_MIRRORED;
                        }
                        else
                            thing->flags2 &= ~MF2_MIRRORED;
                    }
                }
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_mirroredweapons, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_mirroredweapons == r_mirroredweapons_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_mirroredweapons_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_randomstartframes CVAR
//
static void r_randomstartframes_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_randomstartframes)
        {
            r_randomstartframes = value;
            M_SaveCVARs();

            if (r_randomstartframes)
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                    {
                        mobjinfo_t  *info = thing->info;

                        if (info->frames > 1)
                        {
                            const int   numframes = M_BigRandomInt(0, info->frames);
                            state_t     *st = thing->state;

                            for (int j = 0; j < numframes && st->nextstate != S_NULL; j++)
                                st = &states[st->nextstate];

                            thing->state = st;
                        }
                    }
            else
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                    {
                        mobjinfo_t  *info = thing->info;

                        if (info->frames > 1)
                            thing->state = &states[info->spawnstate];
                    }
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_randomstartframes, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_randomstartframes == r_randomstartframes_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_randomstartframes_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
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
            r_screensize = value;
            S_StartSound(NULL, sfx_stnmov);
            R_SetViewSize(r_screensize);
            r_hud = (r_screensize == r_screensize_max);

            if (vid_widescreen && r_screensize < r_screensize_max - 1)
            {
                vid_widescreen = false;
                I_RestartGraphics(false);
            }
            else if (!vid_widescreen && r_screensize == r_screensize_max)
            {
                vid_widescreen = true;
                I_RestartGraphics(false);
            }

            M_SaveCVARs();

            if (r_playersprites)
                skippsprinterp = true;
        }
    }
    else
    {
        char        *temp1 = commify(r_screensize);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_screensize == r_screensize_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = commify(r_screensize_default);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
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
                for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                    P_SetShadowColumnFunction(thing);
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_shadows_translucency, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_shadows_translucency == r_shadows_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_shadows_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_sprites_translucency CVAR
//
static void r_sprites_translucency_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_sprites_translucency)
        {
            r_sprites_translucency = value;
            M_SaveCVARs();
            R_InitColumnFunctions();

            for (int i = 0; i < numsectors; i++)
                for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                    thing->colfunc = thing->info->colfunc;
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_sprites_translucency, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_sprites_translucency == r_sprites_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_sprites_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_supersampling CVAR
//
static void r_supersampling_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_supersampling)
        {
            r_supersampling = value;
            M_SaveCVARs();
            GetPixelSize();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_supersampling, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_supersampling == r_supersampling_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_supersampling_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
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
                for (mobj_t *mo = sectors[i].thinglist; mo; mo = mo->snext)
                {
                    mo->colfunc = mo->info->colfunc;
                    P_SetShadowColumnFunction(mo);
                }

                for (bloodsplat_t *splat = sectors[i].splatlist; splat; splat = splat->next)
                    P_SetBloodSplatColor(splat);
            }
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_textures, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_textures == r_textures_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_textures_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_textures_translucency CVAR
//
static void r_textures_translucency_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_textures_translucency)
        {
            r_textures_translucency = value;
            M_SaveCVARs();
            R_InitColumnFunctions();

            for (int i = 0; i < numsectors; i++)
                for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                    thing->colfunc = thing->info->colfunc;
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_textures_translucency, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_textures_translucency == r_textures_translucency_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_textures_translucency_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// s_musicvolume and s_sfxvolume CVARs
//
static bool s_volume_cvars_func1(char *cmd, char *parms)
{
    int value;

    if (!*parms)
        return true;

    if (sscanf(parms, "%10i%%", &value) != 1 && sscanf(parms, "%10i", &value) != 1)
        return false;

    return ((M_StringCompare(cmd, stringize(s_musicvolume)) && value >= s_musicvolume_min && value <= s_musicvolume_max)
        || (M_StringCompare(cmd, stringize(s_sfxvolume)) && value >= s_sfxvolume_min && value <= s_sfxvolume_max));
}

static void s_volume_cvars_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        int value;

        if (sscanf(parms, "%10i%%", &value) != 1 && sscanf(parms, "%10i", &value) != 1)
            return;

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
            S_SetSfxVolume(sfxVolume * (MIX_MAX_VOLUME - 1) / 31);
            M_SaveCVARs();
        }
    }
    else if (M_StringCompare(cmd, stringize(s_musicvolume)))
    {
        char        *temp1 = commify(s_musicvolume);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (s_musicvolume == s_musicvolume_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = commify(s_musicvolume_default);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
    else
    {
        char        *temp1 = commify(s_sfxvolume);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (s_sfxvolume == s_sfxvolume_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = commify(s_sfxvolume_default);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
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
        SaveDef.laston = savegame - 1;
        LoadDef.laston = savegame - 1;
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
        NewDef.laston = skilllevel - 1;
    }
}

//
// turbo CVAR
//
static bool turbo_cvar_func1(char *cmd, char *parms)
{
    int value;

    if (!*parms)
        return true;

    return ((sscanf(parms, "%10i%%", &value) == 1 || sscanf(parms, "%10i", &value) == 1) && value >= turbo_min && value <= turbo_max);
}

static void turbo_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        int value = INT_MIN;

        if (sscanf(parms, "%10i%%", &value) != 1 && sscanf(parms, "%10i", &value) != 1)
            return;

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
        char        *temp1 = commify(turbo);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (turbo == turbo_default)
            C_Output(PERCENTCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = commify(turbo_default);

            C_Output(PERCENTCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// units CVAR
//
static bool units_cvar_func1(char *cmd, char *parms)
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
        char        *temp1 = C_LookupAliasFromValue(units, UNITSVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (units == units_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(units_default, UNITSVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// vid_borderlesswindow CVAR
//
static void vid_borderlesswindow_cvar_func2(char *cmd, char *parms)
{
    const bool  vid_borderlesswindow_old = vid_borderlesswindow;

    bool_cvars_func2(cmd, parms);

    if (vid_borderlesswindow != vid_borderlesswindow_old && vid_fullscreen)
        I_RestartGraphics(true);
}

//
// vid_capfps CVAR
//
static bool vid_capfps_cvar_func1(char *cmd, char *parms)
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
            if (vid_capfps && (vid_capfps < TICRATE || vid_capfps > vid_capfps_max))
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
        I_RestartGraphics(false);
}

//
// vid_fullscreen CVAR
//
static void vid_fullscreen_cvar_func2(char *cmd, char *parms)
{
    const bool  vid_fullscreen_old = vid_fullscreen;

    bool_cvars_func2(cmd, parms);

    if (vid_fullscreen != vid_fullscreen_old)
        I_ToggleFullscreen();
}

//
// vid_pillarboxes CVAR
//
static void vid_pillarboxes_cvar_func2(char *cmd, char *parms)
{
    const bool  vid_pillarboxes_old = vid_pillarboxes;

    bool_cvars_func2(cmd, parms);

    if (vid_pillarboxes != vid_pillarboxes_old)
        I_SetPillarboxes();
}

//
// vid_scaleapi CVAR
//
static bool vid_scaleapi_cvar_func1(char *cmd, char *parms)
{
    return (!*parms
#if defined(_WIN32)
        || M_StringCompare(parms, vid_scaleapi_direct3d)
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
            I_RestartGraphics(true);
        }
    }
    else
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (M_StringCompare(vid_scaleapi, vid_scaleapi_default))
            C_Output(STRINGCVARISDEFAULT, vid_scaleapi);
        else
            C_Output(STRINGCVARWITHDEFAULT, vid_scaleapi, vid_scaleapi_default);

        C_ShowWarning(i);
    }
}

//
// vid_scalefilter CVAR
//
static bool vid_scalefilter_cvar_func1(char *cmd, char *parms)
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
            I_RestartGraphics(false);
        }
    }
    else
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (M_StringCompare(vid_scalefilter, vid_scalefilter_default))
            C_Output(STRINGCVARISDEFAULT, vid_scalefilter);
        else
            C_Output(STRINGCVARWITHDEFAULT, vid_scalefilter, vid_scalefilter_default);

        C_ShowWarning(i);
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
                I_RestartGraphics(false);
        }
    }
    else
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (M_StringCompare(vid_screenresolution, vid_screenresolution_default))
            C_Output(INTEGERCVARISDEFAULT, vid_screenresolution);
        else
            C_Output(INTEGERCVARWITHDEFAULT, vid_screenresolution, vid_screenresolution_default);

        C_ShowWarning(i);
    }
}

//
// vid_showfps CVAR
//
static void vid_showfps_cvar_func2(char *cmd, char *parms)
{
    const bool  vid_showfps_old = vid_showfps;

    bool_cvars_func2(cmd, parms);

    if (vid_showfps != vid_showfps_old)
    {
        I_UpdateBlitFunc(viewplayer->damagecount);

        if (vid_showfps)
            starttime = SDL_GetPerformanceCounter();
        else
        {
            framespersecond = 0;
            frames = -1;
        }
    }
}

//
// vid_vsync CVAR
//
static bool vid_vsync_cvar_func1(char *cmd, char *parms)
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
            I_RestartGraphics(true);
            M_SaveCVARs();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(vid_vsync, VSYNCVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (vid_vsync == vid_vsync_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(vid_vsync_default, VSYNCVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// vid_widescreen CVAR
//
static void vid_widescreen_cvar_func2(char *cmd, char *parms)
{
    const bool  vid_widescreen_old = vid_widescreen;

    bool_cvars_func2(cmd, parms);

    if (vid_widescreen != vid_widescreen_old)
    {
        r_screensize = r_screensize_max - 1;
        r_hud = false;
        R_SetViewSize(r_screensize);
        I_RestartGraphics(false);
        S_StartSound(NULL, sfx_stnmov);
    }
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
            {
                if (M_StringCompare(vid_windowpos, vid_windowpos_centered)
                    || M_StringCompare(vid_windowpos, vid_windowpos_centred))
                    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                else
                    SDL_SetWindowPosition(window, windowx, windowy);
            }
        }

        free(parm);
    }
    else
    {
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (M_StringCompare(vid_windowpos, vid_windowpos_default))
            C_Output(INTEGERCVARISDEFAULT, vid_windowpos);
        else
            C_Output(INTEGERCVARWITHDEFAULT, vid_windowpos, vid_windowpos_default);

        C_ShowWarning(i);
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
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (M_StringCompare(vid_windowsize, vid_windowsize_default))
            C_Output(INTEGERCVARISDEFAULT, vid_windowsize);
        else
            C_Output(INTEGERCVARWITHDEFAULT, vid_windowsize, vid_windowsize_default);

        C_ShowWarning(i);
    }
}

//
// weapon CVAR
//
static bool weapon_cvar_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;
    else if (gamestate != GS_LEVEL || viewplayer->pendingweapon != wp_nochange)
        return false;
    else
    {
        const int   value = C_LookupValueFromAlias(parms, WEAPONVALUEALIAS);

        return (C_LookupValueFromAlias(parms, WEAPONVALUEALIAS) != INT_MIN
            && value != viewplayer->readyweapon && viewplayer->weaponowned[value]);
    }
}

static void weapon_cvar_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        viewplayer->pendingweapon = C_LookupValueFromAlias(parms, WEAPONVALUEALIAS);
        C_HideConsole();
    }
    else
    {
        char        *temp = C_LookupAliasFromValue((gamestate == GS_LEVEL ? viewplayer->readyweapon : weapon_default), WEAPONVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);
        C_Output(INTEGERCVARWITHNODEFAULT, temp);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMEWARNING, "you", "are");
            else
                C_Warning(0, NOGAMEWARNING, playername, "is");
        }

        free(temp);
    }
}

//
// weaponrecoil CVAR
//
static void weaponrecoil_cvar_func2(char *cmd, char *parms)
{
    const bool  weaponrecoil_old = weaponrecoil;

    bool_cvars_func2(cmd, parms);

    if (weaponrecoil != weaponrecoil_old)
    {
        if (gamestate == GS_LEVEL)
        {
            suppresswarnings = true;
            R_InitSkyMap();
            suppresswarnings = false;

            R_InitColumnFunctions();

            if (!weaponrecoil)
            {
                viewplayer->recoil = 0;
                viewplayer->oldrecoil = 0;
            }
        }
    }
}
