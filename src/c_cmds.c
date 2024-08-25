/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
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
#include "i_controller.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "md5.h"
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
                                    BOLD("powerups") "|" BOLD("all") "|" BOLDITALICS("item")
#define IFCMDFORMAT                 BOLDITALICS("CVAR") " " BOLD("is")  " " BOLDITALICS("value") " " BOLD("then") " [" BOLD("\"") "]" \
                                    BOLDITALICS("command") "[" BOLD(";") " " BOLDITALICS("command") " ..." BOLD("\"") "]"
#define KILLCMDFORMAT               BOLD("player") "|" BOLD("all") "|[[" BOLD("un") "]" BOLD("friendly") " ]" BOLDITALICS("monster")
#define LOADCMDFORMAT               BOLD("1") ".." BOLD("8") "|" BOLDITALICS("filename") "[" BOLD(".save") "]"
#define MAPCMDFORMAT1               BOLD("E") BOLDITALICS("x") BOLD("M") BOLDITALICS("y") "[" BOLD("B") "]|" BOLDITALICS("title") "|" \
                                    BOLD("first") "|" BOLD("previous") "|" BOLD("next") "|" BOLD("last") "|" BOLD("random")
#define MAPCMDFORMAT2               BOLD("MAP") BOLDITALICS("xy") "|" BOLDITALICS("title") "|" BOLD("first") "|" BOLD("previous") "|" \
                                    BOLD("next") "|" BOLD("last") "|" BOLD("random")
#define PLAYCMDFORMAT               BOLDITALICS("soundeffect") "|" BOLDITALICS("music")
#define NAMECMDFORMAT               "[[" BOLD("un") "]" BOLD("friendly") " ]" BOLDITALICS("monster") " " BOLDITALICS("name")
#define PRINTCMDFORMAT              "[" BOLD("\x93") "]" BOLDITALICS("message") "[" BOLD("\x94") "]"
#define REMOVECMDFORMAT             BOLD("decorations") "|" BOLD("corpses") "|" BOLD("bloodsplats") "|" BOLD("items") "|" \
                                    BOLDITALICS("item") "|" BOLD("everything")
#define RESETCMDFORMAT              BOLDITALICS("CVAR")
#define RESURRECTCMDFORMAT          BOLD("player") "|" BOLD("all") "|[[" BOLD("un") "]" BOLD("friendly") " ]" BOLDITALICS("monster")
#define SAVECMDFORMAT               BOLD("1") ".." BOLD("8") "|" BOLDITALICS("filename") "[" BOLD(".save") "]"
#define SPAWNCMDFORMAT              BOLDITALICS("item") "|[[" BOLD("un") "]" BOLD("friendly") " ]" BOLDITALICS("monster")
#define TAKECMDFORMAT               BOLD("ammo") "|" BOLD("armor") "|" BOLD("health") "|" BOLD("keys") "|" BOLD("weapons") "|" \
                                    BOLD("powerups") "|" BOLD("all") "|" BOLDITALICS("item")
#define TELEPORTCMDFORMAT           BOLDITALICS("x") " " BOLDITALICS("y") "[ " BOLDITALICS("z") "]"
#define TIMERCMDFORMAT              BOLDITALICS("minutes")
#define TOGGLECMDFORMAT             BOLDITALICS("CVAR")
#define UNBINDCMDFORMAT             BOLDITALICS("control") "|" BOLDITALICS("+action")

#define WEAPONDESCRIPTION_SHAREWARE "Your currently equipped weapon (" BOLD("fists") ", " BOLD("chainsaw") ", " \
                                    BOLD("pistol") ", " BOLD("shotgun") ", " BOLD("chaingun") " or " BOLD("rocketlauncher") ")."
#define WEAPONDESCRIPTION_DOOM2     "Your currently equipped weapon (" BOLD("fists") ", " BOLD("chainsaw") ", " \
                                    BOLD("pistol") ", " BOLD("shotgun") ", " BOLD("supershotgun") ", " BOLD("chaingun") ", " \
                                    BOLD("rocketlauncher") ", " BOLD("plasmarifle") " or " BOLD("bfg9000") ")."

#define DEADPLAYERWARNING1          "You can't change this CVAR right now because you're dead."
#define DEADPLAYERWARNING2          "%s can't change this CVAR right now because %s %s dead."
#define NEXTMAPWARNING1             "You won't see the results of changing this CVAR until the next map."
#define NEXTMAPWARNING2             "%s won't see the results of changing this CVAR until the next map."
#define NOGAMECCMDWARNING1          "You can't use this CCMD right now because you're not playing a game."
#define NOGAMECCMDWARNING2          "%s can't use this CCMD right now because %s %s playing a game."
#define NOGAMECVARWARNING1          "You can't change this CVAR right now because you're not playing a game."
#define NOGAMECVARWARNING2          "%s can't change this CVAR right now because %s %s playing a game."
#define NIGHTMAREWARNING1           "You can't change this CVAR right now because you're playing a game in " ITALICS("Nightmare!")
#define NIGHTMAREWARNING2           "%s can't change this CVAR right now because %s %s playing a game in " ITALICS("Nightmare!")

#define INTEGERCVARWITHDEFAULT      "It is currently set to " BOLD("%s") " and is " BOLD("%s") " by default."
#define INTEGERCVARWITHNODEFAULT    "It is currently set to " BOLD("%s") "."
#define INTEGERCVARISDEFAULT        "It is currently set to its default of " BOLD("%s") "."
#define DEGREESCVARWITHDEFAULT      "It is currently set to " BOLD("%i") "\xB0 and is " BOLD("%i") "\xB0 by default."
#define DEGREESCVARISDEFAULT        "It is currently set to its default of " BOLD("%i") "\xB0."
#define PERCENTCVARWITHDEFAULT      "It is currently set to " BOLD("%s%%") " and is " BOLD("%s%%") " by default."
#define PERCENTCVARWITHNODEFAULT    "It is currently set to " BOLD("%s%%") "."
#define PERCENTCVARISDEFAULT        "It is currently set to its default of " BOLD("%s%%") "."
#define STRINGCVARWITHDEFAULT       "It is currently set to " BOLD("\"%s\"") " and is " BOLD("\"%s\"") " by default."
#define STRINGCVARWITHNODEFAULT     "It is currently set to " BOLD("%s%s%s") "."
#define STRINGCVARISDEFAULT         "It is currently set to its default of " BOLD("\"%s\"") "."
#define TIMECVARWITHNODEFAULT1      "It is currently set to " BOLD(MONOSPACED("%02i") ":" MONOSPACED("%02i")) "."
#define TIMECVARWITHNODEFAULT2      "It is currently set to " BOLD(MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i")) "."

#define INDENT                      "       "

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
bool        nobindoutput = false;
bool        parsingcfgfile = false;
bool        quitcmd = false;
bool        resettingcvar = false;
bool        togglingcvar = false;
bool        togglingvanilla = false;
bool        vanilla = false;

const control_t controls[] =
{
    { "escape",        keyboardcontrol,   KEY_ESCAPE                },
    { "F12",           keyboardcontrol,   KEY_F12                   },
    { "printscreen",   keyboardcontrol,   KEY_PRINTSCREEN           },
    { "delete",        keyboardcontrol,   KEY_DELETE                },
    { "insert",        keyboardcontrol,   KEY_INSERT                },
    { "tilde",         keyboardcontrol,   '`'                       },
    { "1",             keyboardcontrol,   '1'                       },
    { "2",             keyboardcontrol,   '2'                       },
    { "3",             keyboardcontrol,   '3'                       },
    { "4",             keyboardcontrol,   '4'                       },
    { "5",             keyboardcontrol,   '5'                       },
    { "6",             keyboardcontrol,   '6'                       },
    { "7",             keyboardcontrol,   '7'                       },
    { "8",             keyboardcontrol,   '8'                       },
    { "9",             keyboardcontrol,   '9'                       },
    { "0",             keyboardcontrol,   '0'                       },
    { "-",             keyboardcontrol,   '-'                       },
    { "=",             keyboardcontrol,   '='                       },
    { "+",             keyboardcontrol,   '='                       },
    { "backspace",     keyboardcontrol,   KEY_BACKSPACE             },
    { "tab",           keyboardcontrol,   KEY_TAB                   },
    { "q",             keyboardcontrol,   'q'                       },
    { "w",             keyboardcontrol,   'w'                       },
    { "e",             keyboardcontrol,   'e'                       },
    { "r",             keyboardcontrol,   'r'                       },
    { "t",             keyboardcontrol,   't'                       },
    { "y",             keyboardcontrol,   'y'                       },
    { "u",             keyboardcontrol,   'u'                       },
    { "i",             keyboardcontrol,   'i'                       },
    { "o",             keyboardcontrol,   'o'                       },
    { "p",             keyboardcontrol,   'p'                       },
    { "[",             keyboardcontrol,   '['                       },
    { "]",             keyboardcontrol,   ']'                       },
    { "\\",            keyboardcontrol,   '\\'                      },
    { "a",             keyboardcontrol,   'a'                       },
    { "s",             keyboardcontrol,   's'                       },
    { "d",             keyboardcontrol,   'd'                       },
    { "f",             keyboardcontrol,   'f'                       },
    { "g",             keyboardcontrol,   'g'                       },
    { "h",             keyboardcontrol,   'h'                       },
    { "j",             keyboardcontrol,   'j'                       },
    { "k",             keyboardcontrol,   'k'                       },
    { "l",             keyboardcontrol,   'l'                       },
    { ";",             keyboardcontrol,   ';'                       },
    { "'",             keyboardcontrol,   '\''                      },
    { "enter",         keyboardcontrol,   KEY_ENTER                 },
    { "shift",         keyboardcontrol,   KEY_SHIFT                 },
    { "z",             keyboardcontrol,   'z'                       },
    { "x",             keyboardcontrol,   'x'                       },
    { "c",             keyboardcontrol,   'c'                       },
    { "v",             keyboardcontrol,   'v'                       },
    { "b",             keyboardcontrol,   'b'                       },
    { "n",             keyboardcontrol,   'n'                       },
    { "m",             keyboardcontrol,   'm'                       },
    { ",",             keyboardcontrol,   ','                       },
    { ".",             keyboardcontrol,   '.'                       },
    { "/",             keyboardcontrol,   '/'                       },
    { "ctrl",          keyboardcontrol,   KEY_CTRL                  },
    { "alt",           keyboardcontrol,   KEY_ALT                   },
    { "space",         keyboardcontrol,   ' '                       },
    { "numlock",       keyboardcontrol,   KEY_NUMLOCK               },
    { "capslock",      keyboardcontrol,   KEY_CAPSLOCK              },
    { "scrolllock",    keyboardcontrol,   KEY_SCROLLLOCK            },
    { "home",          keyboardcontrol,   KEY_HOME                  },
    { "up",            keyboardcontrol,   KEY_UPARROW               },
    { "pageup",        keyboardcontrol,   KEY_PAGEUP                },
    { "left",          keyboardcontrol,   KEY_LEFTARROW             },
    { "right",         keyboardcontrol,   KEY_RIGHTARROW            },
    { "end",           keyboardcontrol,   KEY_END                   },
    { "down",          keyboardcontrol,   KEY_DOWNARROW             },
    { "pagedown",      keyboardcontrol,   KEY_PAGEDOWN              },
    { "numpad0",       keyboardcontrol,   KEYP_0                    },
    { "numpad1",       keyboardcontrol,   KEYP_1                    },
    { "numpad2",       keyboardcontrol,   KEYP_2                    },
    { "numpad3",       keyboardcontrol,   KEYP_3                    },
    { "numpad4",       keyboardcontrol,   KEYP_4                    },
    { "numpad5",       keyboardcontrol,   KEYP_5                    },
    { "numpad6",       keyboardcontrol,   KEYP_6                    },
    { "numpad7",       keyboardcontrol,   KEYP_7                    },
    { "numpad8",       keyboardcontrol,   KEYP_8                    },
    { "numpad9",       keyboardcontrol,   KEYP_9                    },

    { "mouse1",        mousecontrol,      0                         },
    { "mouse2",        mousecontrol,      1                         },
    { "mouse3",        mousecontrol,      2                         },
    { "mouse4",        mousecontrol,      3                         },
    { "mouse5",        mousecontrol,      4                         },
    { "mouse6",        mousecontrol,      5                         },
    { "mouse7",        mousecontrol,      6                         },
    { "mouse8",        mousecontrol,      7                         },
    { "wheelup",       mousecontrol,      MOUSE_WHEELUP             },
    { "wheeldown",     mousecontrol,      MOUSE_WHEELDOWN           },

    { "button1",       controllercontrol, CONTROLLER_A              },
    { "gamepad1",      controllercontrol, CONTROLLER_A              },
    { "button2",       controllercontrol, CONTROLLER_B              },
    { "gamepad2",      controllercontrol, CONTROLLER_B              },
    { "button3",       controllercontrol, CONTROLLER_X              },
    { "gamepad3",      controllercontrol, CONTROLLER_X              },
    { "button4",       controllercontrol, CONTROLLER_Y              },
    { "gamepad4",      controllercontrol, CONTROLLER_Y              },
    { "back",          controllercontrol, CONTROLLER_BACK           },
    { "guide",         controllercontrol, CONTROLLER_GUIDE          },
    { "start",         controllercontrol, CONTROLLER_START          },
    { "leftthumb",     controllercontrol, CONTROLLER_LEFT_THUMB     },
    { "rightthumb",    controllercontrol, CONTROLLER_RIGHT_THUMB    },
    { "leftshoulder",  controllercontrol, CONTROLLER_LEFT_SHOULDER  },
    { "rightshoulder", controllercontrol, CONTROLLER_RIGHT_SHOULDER },
    { "dpadup",        controllercontrol, CONTROLLER_DPAD_UP        },
    { "dpaddown",      controllercontrol, CONTROLLER_DPAD_DOWN      },
    { "dpadleft",      controllercontrol, CONTROLLER_DPAD_LEFT      },
    { "dpadright",     controllercontrol, CONTROLLER_DPAD_RIGHT     },
    { "misc1",         controllercontrol, CONTROLLER_MISC1          },
    { "paddle1",       controllercontrol, CONTROLLER_PADDLE1        },
    { "paddle2",       controllercontrol, CONTROLLER_PADDLE2        },
    { "paddle3",       controllercontrol, CONTROLLER_PADDLE3        },
    { "paddle4",       controllercontrol, CONTROLLER_PADDLE4        },
    { "touchpad",      controllercontrol, CONTROLLER_TOUCHPAD       },
    { "lefttrigger",   controllercontrol, CONTROLLER_LEFT_TRIGGER   },
    { "righttrigger",  controllercontrol, CONTROLLER_RIGHT_TRIGGER  },

    { "",              0,                 0                         }
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
static void zoomin_action_func(void);
static void zoomout_action_func(void);

action_t actions[] =
{
    { "+alwaysrun",   true,  false, alwaysrun_action_func,   &keyboardalwaysrun,   NULL,                  &mousealwaysrun,   &controlleralwaysrun,   NULL            },
    { "+automap",     false, false, automap_action_func,     &keyboardautomap,     NULL,                  &mouseautomap,     &controllerautomap,     NULL            },
    { "+back",        true,  false, back_action_func,        &keyboardback,        &keyboardback2,        &mouseback,        &controllerback,        NULL            },
    { "+clearmark",   true,  true,  clearmark_action_func,   &keyboardclearmark,   NULL,                  &mouseclearmark,   &controllerclearmark,   NULL            },
    { "+console",     false, false, console_action_func,     &keyboardconsole,     NULL,                  &mouseconsole,     &controllerconsole,     NULL            },
    { "+fire",        true,  false, fire_action_func,        &keyboardfire,        NULL,                  &mousefire,        &controllerfire,        NULL            },
    { "+followmode",  true,  false, followmode_action_func,  &keyboardfollowmode,  NULL,                  &mousefollowmode,  &controllerfollowmode,  NULL            },
    { "+forward",     true,  false, forward_action_func,     &keyboardforward,     &keyboardforward2,     &mouseforward,     &controllerforward,     NULL            },
    { "+freelook",    true,  false, NULL,                    &keyboardfreelook,    NULL,                  &mousefreelook,    &controllerfreelook,    NULL            },
    { "+grid",        true,  true,  grid_action_func,        &keyboardgrid,        NULL,                  &mousegrid,        &controllergrid,        NULL            },
    { "+jump",        true,  false, jump_action_func,        &keyboardjump,        NULL,                  &mousejump,        &controllerjump,        NULL            },
    { "+left",        true,  false, left_action_func,        &keyboardleft,        NULL,                  &mouseleft,        &controllerleft,        NULL            },
    { "+mark",        true,  true,  mark_action_func,        &keyboardmark,        NULL,                  &mousemark,        &controllermark,        NULL            },
    { "+maxzoom",     true,  true,  maxzoom_action_func,     &keyboardmaxzoom,     NULL,                  &mousemaxzoom,     &controllermaxzoom,     NULL            },
    { "+menu",        true,  false, menu_action_func,        &keyboardmenu,        NULL,                  &mousemenu,        &controllermenu,        NULL            },
    { "+nextweapon",  true,  false, nextweapon_action_func,  &keyboardnextweapon,  NULL,                  &mousenextweapon,  &controllernextweapon,  NULL            },
    { "+prevweapon",  true,  false, prevweapon_action_func,  &keyboardprevweapon,  NULL,                  &mouseprevweapon,  &controllerprevweapon,  NULL            },
    { "+right",       true,  false, right_action_func,       &keyboardright,       NULL,                  &mouseright,       &controllerright,       NULL            },
    { "+rotatemode",  true,  true,  rotatemode_action_func,  &keyboardrotatemode,  NULL,                  &mouserotatemode,  &controllerrotatemode,  NULL            },
    { "+run",         true,  false, NULL,                    &keyboardrun,         NULL,                  &mouserun,         &controllerrun,         NULL            },
    { "+screenshot",  false, false, screenshot_action_func,  &keyboardscreenshot,  NULL,                  &mousescreenshot,  &controllerscreenshot,  NULL            },
    { "+strafe",      true,  false, NULL,                    &keyboardstrafe,      NULL,                  &mousestrafe,      &controllerstrafe,      NULL            },
    { "+strafeleft",  true,  false, strafeleft_action_func,  &keyboardstrafeleft,  &keyboardstrafeleft2,  &mousestrafeleft,  &controllerstrafeleft,  NULL            },
    { "+straferight", true,  false, straferight_action_func, &keyboardstraferight, &keyboardstraferight2, &mousestraferight, &controllerstraferight, NULL            },
    { "+use",         true,  false, use_action_func,         &keyboarduse,         &keyboarduse2,         &mouseuse,         &controlleruse,         &controlleruse2 },
    { "+weapon1",     true,  false, weapon1_action_func,     &keyboardweapon1,     NULL,                  &mouseweapon1,     &controllerweapon1,     NULL            },
    { "+weapon2",     true,  false, weapon2_action_func,     &keyboardweapon2,     NULL,                  &mouseweapon2,     &controllerweapon2,     NULL            },
    { "+weapon3",     true,  false, weapon3_action_func,     &keyboardweapon3,     NULL,                  &mouseweapon3,     &controllerweapon3,     NULL            },
    { "+weapon4",     true,  false, weapon4_action_func,     &keyboardweapon4,     NULL,                  &mouseweapon4,     &controllerweapon4,     NULL            },
    { "+weapon5",     true,  false, weapon5_action_func,     &keyboardweapon5,     NULL,                  &mouseweapon5,     &controllerweapon5,     NULL            },
    { "+weapon6",     true,  false, weapon6_action_func,     &keyboardweapon6,     NULL,                  &mouseweapon6,     &controllerweapon6,     NULL            },
    { "+weapon7",     true,  false, weapon7_action_func,     &keyboardweapon7,     NULL,                  &mouseweapon7,     &controllerweapon7,     NULL            },
    { "+zoomin",      true,  true,  zoomin_action_func,      &keyboardzoomin,      NULL,                  &mousezoomin,      &controllerzoomin,      NULL            },
    { "+zoomout",     true,  true,  zoomout_action_func,     &keyboardzoomout,     NULL,                  &mousezoomout,     &controllerzoomout,     NULL            },
    { "",             false, false, NULL,                    NULL,                 NULL,                  NULL,              NULL,                   NULL            }
};

static bool alive_func1(char *cmd, char *parms);
static bool cheat_func1(char *cmd, char *parms);
static bool game_ccmd_func1(char *cmd, char *parms);
static bool game_cvar_func1(char *cmd, char *parms);
static bool nightmare_func1(char *cmd, char *parms);
static bool null_func1(char *cmd, char *parms);

static void bindlist_func2(char *cmd, char *parms);
static void clear_func2(char *cmd, char *parms);
static void cmdlist_func2(char *cmd, char *parms);
static bool condump_func1(char *cmd, char *parms);
static void condump_func2(char *cmd, char *parms);
static void cvarlist_func2(char *cmd, char *parms);
static void endgame_func2(char *cmd, char *parms);
static void exitmap_func2(char *cmd, char *parms);
static void fastmonsters_func2(char *cmd, char *parms);
static void freeze_func2(char *cmd, char *parms);
static bool give_func1(char *cmd, char *parms);
static void give_func2(char *cmd, char *parms);
static void god_func2(char *cmd, char *parms);
static void if_func2(char *cmd, char *parms);
static void infiniteammo_func2(char *cmd, char *parms);
static bool kill_func1(char *cmd, char *parms);
static void kill_func2(char *cmd, char *parms);
static void license_func2(char *cmd, char *parms);
static void load_func2(char *cmd, char *parms);
static bool map_func1(char *cmd, char *parms);
static void map_func2(char *cmd, char *parms);
static void maplist_func2(char *cmd, char *parms);
static void mapstats_func2(char *cmd, char *parms);
static bool name_func1(char *cmd, char *parms);
static void name_func2(char *cmd, char *parms);
static void newgame_func2(char *cmd, char *parms);
static void noclip_func2(char *cmd, char *parms);
static void nomonsters_func2(char *cmd, char *parms);
static void notarget_func2(char *cmd, char *parms);
static void pistolstart_func2(char *cmd, char *parms);
static bool play_func1(char *cmd, char *parms);
static void play_func2(char *cmd, char *parms);
static void playerstats_func2(char *cmd, char *parms);
static void print_func2(char *cmd, char *parms);
static void quit_func2(char *cmd, char *parms);
static void readme_func2(char *cmd, char *parms);
static void regenhealth_func2(char *cmd, char *parms);
static void reset_func2(char *cmd, char *parms);
static void resetall_func2(char *cmd, char *parms);
static void respawnitems_func2(char *cmd, char *parms);
static void respawnmonsters_func2(char *cmd, char *parms);
static void restartmap_func2(char *cmd, char *parms);
static bool resurrect_func1(char *cmd, char *parms);
static void resurrect_func2(char *cmd, char *parms);
static void save_func2(char *cmd, char *parms);
static bool spawn_func1(char *cmd, char *parms);
static void spawn_func2(char *cmd, char *parms);
static bool take_func1(char *cmd, char *parms);
static void take_func2(char *cmd, char *parms);
static bool teleport_func1(char *cmd, char *parms);
static void teleport_func2(char *cmd, char *parms);
static void thinglist_func2(char *cmd, char *parms);
static void timer_func2(char *cmd, char *parms);
static void toggle_func2(char *cmd, char *parms);
static void unbind_func2(char *cmd, char *parms);
static void vanilla_func2(char *cmd, char *parms);
static void wiki_func2(char *cmd, char *parms);

static bool bool_cvars_func1(char *cmd, char *parms);
static void bool_cvars_func2(char *cmd, char *parms);
static void color_cvars_func2(char *cmd, char *parms);
static bool float_cvars_func1(char *cmd, char *parms);
static void float_cvars_func2(char *cmd, char *parms);
static bool int_cvars_func1(char *cmd, char *parms);
static void int_cvars_func2(char *cmd, char *parms);
static void str_cvars_func2(char *cmd, char *parms);
static void time_cvars_func2(char *cmd, char *parms);

static void alwaysrun_func2(char *cmd, char *parms);
static void am_display_func2(char *cmd, char *parms);
static void am_external_func2(char *cmd, char *parms);
static void am_followmode_func2(char *cmd, char *parms);
static void am_gridsize_func2(char *cmd, char *parms);
static void am_path_func2(char *cmd, char *parms);
static void am_rotatemode_func2(char *cmd, char *parms);
static bool armortype_func1(char *cmd, char *parms);
static void armortype_func2(char *cmd, char *parms);
static void autotilt_func2(char *cmd, char *parms);
static bool crosshair_func1(char *cmd, char *parms);
static void crosshair_func2(char *cmd, char *parms);
static bool english_func1(char *cmd, char *parms);
static void english_func2(char *cmd, char *parms);
static void episode_func2(char *cmd, char *parms);
static void expansion_func2(char *cmd, char *parms);
static void freelook_func2(char *cmd, char *parms);
static bool joy_deadzone_cvars_func1(char *cmd, char *parms);
static void joy_deadzone_cvars_func2(char *cmd, char *parms);
static void joy_sensitivity_cvars_func2(char *cmd, char *parms);
static bool player_cvars_func1(char *cmd, char *parms);
static void player_cvars_func2(char *cmd, char *parms);
static bool playergender_func1(char *cmd, char *parms);
static void playergender_func2(char *cmd, char *parms);
static void r_antialiasing_func2(char *cmd, char *parms);
static bool r_blood_func1(char *cmd, char *parms);
static void r_blood_func2(char *cmd, char *parms);
static void r_bloodsplats_translucency_func2(char *cmd, char *parms);
static void r_brightmaps_func2(char *cmd, char *parms);
static void r_corpses_mirrored_func2(char *cmd, char *parms);
static bool r_detail_func1(char *cmd, char *parms);
static void r_detail_func2(char *cmd, char *parms);
static void r_diskicon_func2(char *cmd, char *parms);
static void r_ditheredlighting_func2(char *cmd, char *parms);
static void r_fixmaperrors_func2(char *cmd, char *parms);
static void r_fov_func2(char *cmd, char *parms);
static bool r_gamma_func1(char *cmd, char *parms);
static void r_gamma_func2(char *cmd, char *parms);
static void r_hud_func2(char *cmd, char *parms);
static void r_hud_translucency_func2(char *cmd, char *parms);
static void r_lowpixelsize_func2(char *cmd, char *parms);
static void r_mirroredweapons_func2(char *cmd, char *parms);
static void r_randomstartframes_func2(char *cmd, char *parms);
static void r_rockettrails_translucency_func2(char *cmd, char *parms);
static void r_screensize_func2(char *cmd, char *parms);
static void r_shadows_translucency_func2(char *cmd, char *parms);
static void r_sprites_translucency_func2(char *cmd, char *parms);
static void r_textures_func2(char *cmd, char *parms);
static void r_textures_translucency_func2(char *cmd, char *parms);
static bool s_volume_cvars_func1(char *cmd, char *parms);
static void s_volume_cvars_func2(char *cmd, char *parms);
static void savegame_func2(char *cmd, char *parms);
static void skilllevel_func2(char *cmd, char *parms);
static bool sucktime_func1(char *cmd, char *parms);
static void sucktime_func2(char *cmd, char *parms);
static bool turbo_func1(char *cmd, char *parms);
static void turbo_func2(char *cmd, char *parms);
static bool units_func1(char *cmd, char *parms);
static void units_func2(char *cmd, char *parms);
static bool vid_aspectratio_func1(char *cmd, char *parms);
static void vid_aspectratio_func2(char *cmd, char *parms);
static void vid_blue_func2(char *cmd, char *parms);
static void vid_borderlesswindow_func2(char *cmd, char *parms);
static void vid_brightness_func2(char *cmd, char *parms);
static bool vid_capfps_func1(char *cmd, char *parms);
static void vid_capfps_func2(char *cmd, char *parms);
static void vid_contrast_func2(char *cmd, char *parms);
static void vid_display_func2(char *cmd, char *parms);
static void vid_fullscreen_func2(char *cmd, char *parms);
static void vid_green_func2(char *cmd, char *parms);
static void vid_pillarboxes_func2(char *cmd, char *parms);
static void vid_red_func2(char *cmd, char *parms);
static void vid_saturation_func2(char *cmd, char *parms);
static bool vid_scaleapi_func1(char *cmd, char *parms);
static void vid_scaleapi_func2(char *cmd, char *parms);
static bool vid_scalefilter_func1(char *cmd, char *parms);
static void vid_scalefilter_func2(char *cmd, char *parms);
static void vid_screenresolution_func2(char *cmd, char *parms);
static void vid_showfps_func2(char *cmd, char *parms);
static bool vid_vsync_func1(char *cmd, char *parms);
static void vid_vsync_func2(char *cmd, char *parms);
static void vid_widescreen_func2(char *cmd, char *parms);
static void vid_windowpos_func2(char *cmd, char *parms);
static void vid_windowsize_func2(char *cmd, char *parms);
static bool weapon_func1(char *cmd, char *parms);
static void weapon_func2(char *cmd, char *parms);
static void weaponrecoil_func2(char *cmd, char *parms);

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
    { #name, #alt1, #alt2, cond, func, parms, CT_CCMD, CF_NONE, 0, NULL, 0, 0, 0, form, desc, 0, 0 }
#define CMD_CHEAT(name, parms) \
    { #name, "", "", cheat_func1, NULL, parms, CT_CHEAT, CF_NONE, 0, NULL, 0, 0, 0, "", "", 0, 0 }
#define CVAR_BOOL(name, alt1, alt2, cond, func, flags, aliases, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_BOOLEAN | flags), 0, &name, aliases, false, true, "", desc, name##_default, 0 }
#define CVAR_INT(name, alt1, alt2, cond, func, flags, aliases, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_INTEGER | flags), 0, &name, aliases, name##_min, name##_max, "", desc, name##_default, 0 }
#define CVAR_FLOAT(name, alt1, alt2, cond, func, flags, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_FLOAT | flags), 0, &name, 0, (int)name##_min, (int)name##_max, "", desc, name##_default, 0 }
#define CVAR_FLOAT2(name, alt1, alt2, cond, func, flags, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_FLOAT | flags), 0, &name, 0, 0, 0, "", desc, name##_default, 0 }
#define CVAR_STR(name, alt1, alt2, cond, func, flags, length, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_STRING | flags), length, &name, 0, 0, 0, "", desc, 0, name##_default }
#define CVAR_TIME(name, alt1, alt2, cond, func, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, (CF_TIME | CF_READONLY), 0, &name, 0, 0, 0, "", desc, 0, "" }
#define CVAR_OTHER(name, alt1, alt2, cond, func, desc) \
    { #name, #alt1, #alt2, cond, func, 1, CT_CVAR, CF_OTHER, 0, &name, 0, 0, 0, "", desc, 0, name##_default }

consolecmd_t consolecmds[] =
{
    CCMD(alias, "", "", null_func1, alias_func2, true, ALIASCMDFORMAT,
        "Creates an " BOLDITALICS("alias") " that executes a string of " BOLDITALICS("commands") "."),
    CVAR_BOOL(alwaysrun, "", "", bool_cvars_func1, alwaysrun_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles you to always run instead of walk."),
    CVAR_INT(am_allmapcdwallcolor, am_allmapcdwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped lines in the automap indicating a change in a ceiling's height once you have a computer area map "
        "power-up (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_allmapfdwallcolor, am_allmapfdwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped lines in the automap indicating a change in a floor's height once you have a computer area map "
        "power-up (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_allmapwallcolor, am_allmapwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of unmapped solid walls in the automap once you have a computer area map power-up (" BOLD("0") " to "
        BOLD("255") ")."),
    CVAR_BOOL(am_antialiasing, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles anti-aliasing in the automap."),
    CVAR_INT(am_backcolor, am_backcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the automap's background (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_bloodsplatcolor, am_bloodsplatcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of blood splats in the automap when you cheat (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_bluedoorcolor, am_bluedoorcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of doors in the automap unlocked with a blue keycard or skull key (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_bluekeycolor, am_bluekeycolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of blue keycards and skull keys in the automap when you cheat (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_cdwallcolor, am_cdwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating a change in a ceiling's height (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_corpsecolor, am_corpsecolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of corpses in the automap when you cheat (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_crosshaircolor, am_crosshaircolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the crosshair in the automap when follow mode is off (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_display, "", "", int_cvars_func1, am_display_func2, CF_NONE, NOVALUEALIAS,
        "The display used to show the external automap."),
    CVAR_BOOL(am_external, "", "", bool_cvars_func1, am_external_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing the automap on an external display."),
    CVAR_INT(am_fdwallcolor, am_fdwallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating a change in a floor's height (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(am_followmode, "", "", game_cvar_func1, am_followmode_func2, CF_MAPRESET, BOOLVALUEALIAS,
        "Toggles follow mode in the automap."),
    CVAR_BOOL(am_grid, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the grid in the automap."),
    CVAR_INT(am_gridcolor, am_gridcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of the grid in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_OTHER(am_gridsize, "", "", null_func1, am_gridsize_func2,
        "The size of the grid in the automap (" BOLD(ITALICS("width") "\xD7" ITALICS("height")) ")."),
    CVAR_INT(am_markcolor, am_markcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of marks in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(am_path, "", "", bool_cvars_func1, am_path_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles your path in the automap."),
    CVAR_INT(am_pathcolor, am_pathcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of your path in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_playercolor, am_playercolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of your arrow in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(am_playerstats, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles your stats in the automap."),
    CVAR_INT(am_playerstatscolor, am_playerstatscolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of your stats in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_reddoorcolor, am_reddoorcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of doors in the automap unlocked with a red keycard or skull key (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_redkeycolor, am_redkeycolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of red keycards and skull keys in the automap when you cheat (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(am_rotatemode, "", "", bool_cvars_func1, am_rotatemode_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles rotate mode in the automap."),
    CVAR_INT(am_teleportercolor, am_teleportercolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of teleporter lines in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_thingcolor, am_thingcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of things in the automap when you cheat (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_tswallcolor, am_tswallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of lines in the automap indicating no change in height (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_wallcolor, am_wallcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of solid walls in the automap (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_yellowdoorcolor, am_yellowdoorcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of doors in the automap unlocked with a yellow keycard or skull key (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(am_yellowkeycolor, am_yellowkeycolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of yellow keycards and skull keys in the automap when you cheat (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_INT(ammo, "", "", player_cvars_func1, player_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The amount of ammo you have for your currently equipped weapon."),
    CVAR_BOOL(animatedstats, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles animating your health, armor and ammo in the status bar and widescreen HUD when they change."),
    CVAR_INT(armor, armour, "", player_cvars_func1, player_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "Your armor (" BOLD("0%") " to " BOLD("200%") ")."),
    CVAR_INT(armortype, armourtype, "", armortype_func1, armortype_func2, CF_NONE, ARMORTYPEVALUEALIAS,
        "Your armor type (" BOLD("none") ", " BOLD("green") " or " BOLD("blue") ")."),
    CVAR_BOOL(autoaim, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles vertical autoaiming as you fire your weapon while using freelook."),
    CVAR_BOOL(autoload, "", "", bool_cvars_func1, bool_cvars_func2, CF_PISTOLSTART, BOOLVALUEALIAS,
        "Toggles automatically loading the last savegame when you die."),
    CVAR_BOOL(autosave, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles automatically saving the game at the start of each map."),
    CVAR_BOOL(autotilt, "", "", bool_cvars_func1, autotilt_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles automatically tilting your view when going up or down a flight of stairs."),
    CVAR_BOOL(autouse, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles automatically using doors and switches in front of you."),
    CCMD(bind, "", "", null_func1, bind_func2, true, BINDCMDFORMAT,
        "Binds an " BOLDITALICS("+action") " or a string of " BOLDITALICS("commands") " to a " BOLDITALICS("control") "."),
    CCMD(bindlist, "", "", null_func1, bindlist_func2, false, "",
        "Lists all controls bound to an " BOLDITALICS("+action") " or a string of commands."),
    CVAR_BOOL(centerweapon, centreweapon, "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles centering your weapon when fired."),
    CCMD(clear, "", "", null_func1, clear_func2, false, "",
        "Clears the console."),
    CCMD(cmdlist, "", ccmdlist, null_func1, cmdlist_func2, true, "[" BOLDITALICS("searchstring") "]",
        "Lists all console commands."),
    CCMD(condump, "", "", condump_func1, condump_func2, true, "[" BOLDITALICS("filename") "[" BOLD(".txt") "]]",
        "Dumps the contents of the console to a file."),
    CVAR_INT(crosshair, "", "", crosshair_func1, crosshair_func2, CF_NONE, CROSSHAIRVALUEALIAS,
        "Toggles your crosshair (" BOLD("none") ", " BOLD("cross") " or " BOLD("dot") ")."),
    CVAR_INT(crosshaircolor, crosshaircolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of your crosshair (" BOLD("0") " to " BOLD("255") ")."),
    CCMD(cvarlist, "", "", null_func1, cvarlist_func2, true, "[" BOLDITALICS("searchstring") "]",
        "Lists all console variables."),
    CCMD(endgame, "", "", null_func1, endgame_func2, false, "",
        "Ends the game."),
    CVAR_BOOL(english, "", "", english_func1, english_func2, CF_NONE, ENGLISHVALUEALIAS,
        "Toggles the use of American or British English (" BOLD("american") " or " BOLD("british") ")."),
    CVAR_INT(episode, "", "", int_cvars_func1, episode_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected " ITALICS("DOOM") " episode in the menu (" BOLD("1") " to " BOLD("6") ")."),
    CCMD(exec, "", "", null_func1, exec_func2, true, EXECCMDFORMAT,
        "Executes all commands in a file."),
    CCMD(exitmap, "", "", alive_func1, exitmap_func2, false, "",
        "Exits the current map."),
    CVAR_INT(expansion, "", "", int_cvars_func1, expansion_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected " ITALICS("DOOM II") " expansion in the menu (" BOLD("1") " or " BOLD("2") ")."),
    CCMD(explode, "", "", kill_func1, kill_func2, true, EXPLODECMDFORMAT,
        "Explodes all " BOLD("barrels") " or " BOLD("missiles") "."),
    CVAR_INT(facebackcolor, facebackcolour, "", int_cvars_func1, color_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The color of your face's background in the status bar (" BOLD("0") " to " BOLD("255") ")."),
    CVAR_BOOL(fade, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a fading effect when transitioning between some screens."),
    CCMD(fastmonsters, "", "", nightmare_func1, fastmonsters_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles fast monsters."),
    CVAR_BOOL(flashkeys, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles flashing the required keycard or skull key when you try to open a locked door."),
    CVAR_BOOL(freelook, mouselook, "", bool_cvars_func1, freelook_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles freely looking up and down using the mouse or a controller."),
    CCMD(freeze, "", "", game_ccmd_func1, freeze_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles freeze mode."),
    CVAR_TIME(gametime, "", "", null_func1, time_cvars_func2,
        "The amount of time " ITALICS(DOOMRETRO_NAME) " has been running."),
    CCMD(give, "", "", give_func1, give_func2, true, GIVECMDFORMAT,
        "Gives " BOLD("ammo") ", " BOLD("armor") ", " BOLD("health") ", " BOLD("keys") ", " BOLD("weapons") ", " BOLD("powerups")", or " BOLD("all")
        " or certain " BOLDITALICS("items") " to you."),
    CCMD(god, "", "", alive_func1, god_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles god mode."),
    CVAR_BOOL(groupmessages, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the grouping of identical player messages."),
    CVAR_INT(health, "", "", player_cvars_func1, player_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "Your health (" BOLD("-99%") " to " BOLD("200%") ")."),
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
    CCMD(if, "", "", null_func1, if_func2, true, IFCMDFORMAT,
        "Executes a string of " BOLDITALICS("commands") " if a " BOLDITALICS("CVAR") " equals a " BOLDITALICS("value") "."),
    CMD_CHEAT(ijwtbha, false),
    CVAR_BOOL(infighting, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles infighting amongst monsters once you die."),
    CCMD(infiniteammo, "", "", game_ccmd_func1, infiniteammo_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles an infinite amount of ammo for all of your weapons."),
    CVAR_BOOL(infiniteheight, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles giving you and everything else in the current map infinite height."),
    CVAR_BOOL(joy_analog, joy_analogue, "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles whether movement using the controller's thumbsticks is analog or digital."),
    CVAR_FLOAT(joy_deadzone_left, "", "", joy_deadzone_cvars_func1, joy_deadzone_cvars_func2, CF_PERCENT,
        "The dead zone of the controller's left thumbstick (" BOLD("0%") " to " BOLD("30%") ")."),
    CVAR_FLOAT(joy_deadzone_right, "", "", joy_deadzone_cvars_func1, joy_deadzone_cvars_func2, CF_PERCENT,
        "The dead zone of the controller's right thumbstick (" BOLD("0%") " to " BOLD("30%") ")."),
    CVAR_BOOL(joy_invertyaxis, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles inverting the vertical axis of the controller's right thumbstick when you look up or down."),
    CVAR_INT(joy_rumble_barrels, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the controller rumbles when you are near an exploding barrel (" BOLD("0%") " to " BOLD("200%") ")."),
    CVAR_INT(joy_rumble_damage, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the controller rumbles when you take damage (" BOLD("0%") " to " BOLD("200%") ")."),
    CVAR_BOOL(joy_rumble_pickup, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles rumbling the controller when you pick something up."),
    CVAR_INT(joy_rumble_weapons, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount the controller rumbles when you fire your weapon (" BOLD("0%") " to " BOLD("200%") ")."),
    CVAR_FLOAT(joy_sensitivity_horizontal, "", "", float_cvars_func1, joy_sensitivity_cvars_func2, CF_NONE,
        "The horizontal sensitivity of the controller's thumbsticks (" BOLD("0") " to " BOLD("128") ")."),
    CVAR_FLOAT(joy_sensitivity_vertical, "", "", float_cvars_func1, joy_sensitivity_cvars_func2, CF_NONE,
        "The vertical sensitivity of the controller's thumbsticks (" BOLD("0") " to " BOLD("128") ")."),
    CVAR_BOOL(joy_swapthumbsticks, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles swapping the controller's left and right thumbsticks."),
    CVAR_INT(joy_thumbsticks, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The number of thumbsticks on the controller (" BOLD("1") " or " BOLD("2") ")."),
    CCMD(kill, "", "", kill_func1, kill_func2, true, KILLCMDFORMAT,
        "Kills the " BOLD("player") ", " BOLD("all") " monsters or a type of " BOLDITALICS("monster") "."),
    CCMD(license, licence, "", null_func1, license_func2, false, "",
        "Shows the " ITALICS(DOOMRETRO_LICENSE ".")),
    CCMD(load, "", "", null_func1, load_func2, true, LOADCMDFORMAT,
        "Loads a savegame."),
    CVAR_BOOL(m_acceleration, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the acceleration of mouse movement."),
    CVAR_BOOL(m_doubleclick_use, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles double-clicking a mouse button to perform the " BOLD("+use") " action."),
    CVAR_BOOL(m_invertyaxis, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles inverting the mouse's vertical axis when using freelook."),
    CVAR_BOOL(m_novertical, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles no vertical movement of the mouse."),
    CVAR_BOOL(m_pointer, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the mouse pointer in the menu."),
    CVAR_FLOAT(m_sensitivity, "", "", float_cvars_func1, float_cvars_func2, CF_NONE,
        "The mouse's sensitivity (" BOLD("0") " to " BOLD("128") ")."),
    CCMD(map, "", warp, map_func1, map_func2, true, MAPCMDFORMAT1,
        "Warps you to another map."),
    CCMD(maplist, "", "", null_func1, maplist_func2, false, "",
        "Lists all of the maps available to play."),
    CCMD(mapstats, "", "", null_func1, mapstats_func2, false, "",
        "Shows stats about the current map."),
    CVAR_TIME(maptime, "", "", null_func1, time_cvars_func2,
        "The amount of time you have been in the current map."),
    CVAR_BOOL(melt, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a melting effect when transitioning between some screens."),
    CVAR_BOOL(menuhighlight, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the highlighting of items selected in the menu."),
    CVAR_BOOL(menushadow, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles shadows cast by items in the menu."),
    CVAR_BOOL(menuspin, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles spinning your view in the menu's background."),
    CVAR_BOOL(messages, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles displaying player messages at the top of the screen."),
    CVAR_INT(movebob, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount your view bobs as you move (" BOLD("0%") " to " BOLD("100%") ")."),
    CCMD(name, "", "", name_func1, name_func2, true, NAMECMDFORMAT,
        "Gives a " BOLDITALICS("name") " to the " BOLDITALICS("monster") " nearest to you."),
    CVAR_BOOL(negativehealth, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles allowing your health to be less than " BOLD("0%") " when you die."),
    CCMD(newgame, "", "", null_func1, newgame_func2, true, "",
        "Starts a new game."),
    CCMD(noclip, "", "", game_ccmd_func1, noclip_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles no clipping mode."),
    CCMD(nomonsters, "", "", null_func1, nomonsters_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles the presence of monsters in maps."),
    CCMD(notarget, "", "", game_ccmd_func1, notarget_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles monsters not targeting you."),
    CVAR_BOOL(obituaries, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles displaying obituaries when you or monsters are killed."),
    CCMD(pistolstart, "", "", null_func1, pistolstart_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles you starting each map with 100% health, no armor, and only your pistol with 50 bullets."),
    CCMD(play, "", "", play_func1, play_func2, true, PLAYCMDFORMAT,
        "Plays a " BOLDITALICS("sound effect") " or " BOLDITALICS("music") " lump."),
    CVAR_INT(playergender, "", "", playergender_func1, playergender_func2, CF_NONE, GENDERVALUEALIAS,
        "Your gender (" BOLD("male") ", " BOLD("female") " or " BOLD("other") ")."),
    CVAR_STR(playername, "", "", null_func1, str_cvars_func2, CF_NONE, 16,
        "Your name."),
    CCMD(playerstats, "", "", null_func1, playerstats_func2, false, "",
        "Shows stats about you."),
    CCMD(print, "", "", game_ccmd_func1, print_func2, true, PRINTCMDFORMAT,
        "Prints a player \"" BOLDITALICS("message") "\"."),
    CCMD(quit, "", exit, null_func1, quit_func2, false, "",
        "Quits to the " DESKTOP "."),
    CVAR_BOOL(r_althud, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles an alternate heads-up display when in widescreen."),
    CVAR_INT(r_berserkeffect, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The intensity of the red effect when you have a berserk power-up and your fists equipped (" BOLD("0") " to " BOLD("8")
        ")."),
    CVAR_INT(r_blood, "", "", r_blood_func1, r_blood_func2, CF_NONE, BLOODVALUEALIAS,
        "The colors of the blood spilled by you and monsters (" BOLD("all") ", " BOLD("none") ", " BOLD("red") ", " BOLD("green")
        " or " BOLD("nofuzz") ")."),
    CVAR_BOOL(r_blood_gibs, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles spawning blood when monsters are gibbed."),
    CVAR_BOOL(r_blood_melee, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles spawning blood during melee attacks from monsters."),
    CVAR_INT(r_bloodsplats_max, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The maximum number of blood splats allowed in each map (" BOLD("0") " to " BOLD("1,048,576") ")."),
    CVAR_INT(r_bloodsplats_total, "", "", int_cvars_func1, int_cvars_func2, CF_READONLY, NOVALUEALIAS,
        "The total number of blood splats in the current map."),
    CVAR_BOOL(r_bloodsplats_translucency, "", "", bool_cvars_func1, r_bloodsplats_translucency_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of blood splats."),
    CVAR_BOOL(r_brightmaps, "", "", bool_cvars_func1, r_brightmaps_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles brightmaps on some wall textures."),
    CVAR_BOOL(r_corpses_color, r_corpses_colour, "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomly colored marine corpses."),
    CVAR_BOOL(r_corpses_gib, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles some corpses gibbing when barrels or rockets explode nearby."),
    CVAR_BOOL(r_corpses_mirrored, "", "", bool_cvars_func1, r_corpses_mirrored_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomly mirrored corpses."),
    CVAR_BOOL(r_corpses_moreblood, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles blood splats spawned around corpses at the start of each map."),
    CVAR_BOOL(r_corpses_nudge, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles corpses and the items they drop being nudged when walked over."),
    CVAR_BOOL(r_corpses_slide, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles corpses sliding when barrels or rockets explode nearby."),
    CVAR_BOOL(r_corpses_smearblood, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles corpses leaving blood splats behind as they slide."),
    CVAR_BOOL(r_damageeffect, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the red effect when you take damage."),
    CVAR_BOOL(r_detail, "", "", r_detail_func1, r_detail_func2, CF_NONE, DETAILVALUEALIAS,
        "Toggles the graphic detail (" BOLD("high") " or " BOLD("low") ")."),
    CVAR_BOOL(r_diskicon, r_discicon, "", bool_cvars_func1, r_diskicon_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing a disk icon when loading and saving."),
    CVAR_BOOL(r_ditheredlighting, "", "", bool_cvars_func1, r_ditheredlighting_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles dithered lighting cast on textures and sprites."),
    CVAR_BOOL(r_fixmaperrors, "", "", bool_cvars_func1, r_fixmaperrors_func2, CF_NEXTMAP, BOOLVALUEALIAS,
        "Toggles fixing many mapping errors in the official " ITALICS("DOOM") " and " ITALICS("DOOM II") " WADs."),
    CVAR_BOOL(r_fixspriteoffsets, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles fixing sprite offsets."),
    CVAR_BOOL(r_floatbob, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles some power-ups bobbing up and down."),
    CVAR_INT(r_fov, "", "", int_cvars_func1, r_fov_func2, CF_NONE, NOVALUEALIAS,
        "Your field of view (" BOLD("45") "\xB0 to " BOLD("135") "\xB0)."),
    CVAR_FLOAT2(r_gamma, "", "", r_gamma_func1, r_gamma_func2, CF_NONE,
        "The screen's gamma correction level (" BOLD("off") ", or " BOLD("0.50") " to " BOLD("2.0") ")."),
    CVAR_BOOL(r_graduallighting, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles gradual lighting under doors and crushing sectors."),
    CVAR_BOOL(r_homindicator, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a flashing \"Hall Of Mirrors\" indicator."),
    CVAR_BOOL(r_hud, "", "", bool_cvars_func1, r_hud_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a heads-up display when in widescreen."),
    CVAR_BOOL(r_hud_translucency, "", "", bool_cvars_func1, r_hud_translucency_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of the heads-up display when in widescreen."),
    CVAR_INT(r_levelbrightness, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The additional brightness applied to all of the lighting in the current map (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(r_linearskies, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles horizontally linear skies."),
    CVAR_BOOL(r_liquid_bob, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the bobbing of liquid sectors."),
    CVAR_BOOL(r_liquid_bobsprites, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the bobbing of sprites in liquid sectors."),
    CVAR_BOOL(r_liquid_clipsprites, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles clipping the bottom of sprites in liquid sectors."),
    CVAR_BOOL(r_liquid_current, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles a slight current being applied to liquid sectors."),
    CVAR_BOOL(r_liquid_lowerview, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles lowering your view when you are in a liquid sector."),
    CVAR_BOOL(r_liquid_swirl, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the swirl of liquid sectors."),
    CVAR_OTHER(r_lowpixelsize, "", "", null_func1, r_lowpixelsize_func2,
        "The size of each pixel when the graphic detail is low (" BOLD(ITALICS("width") "\xD7" ITALICS("height")) ")."),
    CVAR_BOOL(r_mirroredweapons, "", "", bool_cvars_func1, r_mirroredweapons_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomly mirroring the weapons dropped by monsters."),
    CVAR_BOOL(r_pickupeffect, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the gold effect when you pick something up."),
    CVAR_BOOL(r_playersprites, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing your weapon."),
    CVAR_BOOL(r_radsuiteffect, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the green effect while you wear a radiation shielding suit power-up."),
    CVAR_BOOL(r_randomstartframes, "", "", bool_cvars_func1, r_randomstartframes_func2, CF_NEXTMAP, BOOLVALUEALIAS,
        "Toggles randomizing the start frames of certain sprites."),
    CVAR_BOOL(r_rockettrails, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the trail of smoke behind rockets fired by you and cyberdemons."),
    CVAR_BOOL(r_rockettrails_translucency, "", "", bool_cvars_func1, r_rockettrails_translucency_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of the trail of smoke behind rockets fired by you and cyberdemons."),
    CVAR_INT(r_screensize, "", "", int_cvars_func1, r_screensize_func2, CF_NONE, NOVALUEALIAS,
        "The screen size (" BOLD("0") " to " BOLD("8") ")."),
    CVAR_BOOL(r_shadows, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles sprites casting shadows."),
    CVAR_BOOL(r_shadows_translucency, "", "", bool_cvars_func1, r_shadows_translucency_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of shadows cast by sprites."),
    CVAR_BOOL(r_shake_barrels, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles shaking your view when you are near an exploding barrel."),
    CVAR_BOOL(r_shake_berserk, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles shaking your view when you have a berserk power-up and punch something."),
    CVAR_BOOL(r_shake_damage, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles shaking the screen when you take damage."),
    CVAR_BOOL(r_sprites_translucency, "", "", bool_cvars_func1, r_sprites_translucency_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of certain sprites."),
    CVAR_BOOL(r_antialiasing, "", "", bool_cvars_func1, r_antialiasing_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles anti-aliasing when the graphic detail is low."),
    CVAR_BOOL(r_textures, "", "", bool_cvars_func1, r_textures_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles showing all textures."),
    CVAR_BOOL(r_textures_translucency, "", "", bool_cvars_func1, r_textures_translucency_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the translucency of certain " ITALICS("BOOM-") "compatible wall textures."),
    CCMD(readme, "", "", null_func1, readme_func2, false, "",
        "Shows the accompanying readme file for the currently loaded PWAD."),
    CCMD(regenhealth, "", "", game_ccmd_func1, regenhealth_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles regenerating your health by 1% every second when it's less than 100%."),
    CCMD(remove, "", "", kill_func1, kill_func2, true, REMOVECMDFORMAT,
        "Removes all " BOLD("decorations") ", " BOLD("corpses") ", " BOLD("bloodsplats") ", " BOLD("items") ", certain "
        BOLDITALICS("items") ", or " BOLD("everything") "."),
    CCMD(reset, "", "", null_func1, reset_func2, true, RESETCMDFORMAT,
        "Resets a " BOLDITALICS("CVAR") " to its default."),
    CCMD(resetall, "", "", null_func1, resetall_func2, false, "",
        "Resets all CVARs and bound controls to their defaults."),
    CCMD(respawnitems, "", "", null_func1, respawnitems_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles respawning items."),
    CCMD(respawnmonsters, "", "", nightmare_func1, respawnmonsters_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles respawning monsters."),
    CCMD(restartmap, "", "", game_ccmd_func1, restartmap_func2, false, "",
        "Restarts the current map."),
    CCMD(resurrect, "", "", resurrect_func1, resurrect_func2, true, RESURRECTCMDFORMAT,
        "Resurrects the " BOLD("player") ", " BOLD("all") " monsters, or a type of " BOLDITALICS("monster") "."),
    CVAR_INT(s_channels, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The number of sound effects that can be played at the same time (" BOLD("8") " to " BOLD("64") ")."),
    CVAR_BOOL(s_lowermenumusic, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles lowering the music's volume in the menu and console."),
    CVAR_BOOL(s_musicinbackground, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles continuing to play music in the background when " ITALICS(DOOMRETRO_NAME) "'s window loses focus."),
    CVAR_INT(s_musicvolume, "", "", s_volume_cvars_func1, s_volume_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The volume level of music (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(s_randommusic, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomizing the music for each map."),
    CVAR_BOOL(s_randompitch, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles randomizing the pitch of sound effects made by monsters."),
    CVAR_INT(s_sfxvolume, "", "", s_volume_cvars_func1, s_volume_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The volume level of sound effects (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(s_stereo, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles playing sound effects in mono or stereo."),
    CCMD(save, "", "", alive_func1, save_func2, true, SAVECMDFORMAT,
        "Saves the game."),
    CVAR_INT(savegame, "", "", int_cvars_func1, savegame_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected savegame in the menu (" BOLD("1") " to " BOLD("8") ")."),
    CVAR_BOOL(secretmessages, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles displaying a message when you find a secret."),
    CVAR_INT(skilllevel, "", "", int_cvars_func1, skilllevel_func2, CF_NONE, NOVALUEALIAS,
        "The currently selected skill level in the menu (" BOLD("1") " to " BOLD("5") ")."),
    CCMD(spawn, "", summon, spawn_func1, spawn_func2, true, SPAWNCMDFORMAT,
        "Spawns an " BOLDITALICS("item") " or " BOLDITALICS("monster") " in front of you."),
    CVAR_INT(stillbob, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount your view and weapon bob up and down when you stand still (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_INT(sucktime, "", "", sucktime_func1, sucktime_func2, CF_NONE, SUCKSVALUEALIAS,
        "The amount of time you must complete a map before you \"SUCK\" (" BOLD("off") ", or " BOLD("1") " to " BOLD("24") " hours)."),
    CCMD(take, "", "", take_func1, take_func2, true, TAKECMDFORMAT,
        "Takes " BOLD("ammo") ", " BOLD("armor") ", " BOLD("health") ", " BOLD("keys") ", " BOLD("weapons")", " BOLD("powerups") ", or " BOLD("all")
        " or certain " BOLDITALICS("items") " away from you."),
    CCMD(teleport, "", "", teleport_func1, teleport_func2, true, TELEPORTCMDFORMAT,
        "Teleports you to (" BOLDITALICS("x") ", " BOLDITALICS("y") ", " BOLDITALICS("z") ") in the current map."),
    CCMD(thinglist, "", "", game_ccmd_func1, thinglist_func2, false, "",
        "Lists all things in the current map."),
    CCMD(timer, "", "", null_func1, timer_func2, true, TIMERCMDFORMAT,
        "Sets a timer to exit each map after a number of " BOLDITALICS("minutes") "."),
    CCMD(toggle, "", "", null_func1, toggle_func2, true, TOGGLECMDFORMAT,
        "Toggles a " BOLDITALICS("CVAR") " " BOLD("on") " and " BOLD("off") "."),
    CVAR_BOOL(tossdrop, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles tossing items dropped by monsters when they die."),
    CVAR_INT(turbo, "", "", turbo_func1, turbo_func2, CF_PERCENT, NOVALUEALIAS,
        "The speed you move (" BOLD("10%") " to " BOLD("400%") ")."),
    CCMD(unbind, "", "", null_func1, unbind_func2, true, UNBINDCMDFORMAT,
        "Unbinds the " BOLDITALICS("+action") " from a " BOLDITALICS("control") "."),
    CVAR_BOOL(units, "", "", units_func1, units_func2, CF_NONE, UNITSVALUEALIAS,
        "The units used by certain stats (" BOLD("imperial") " or " BOLD("metric") ")."),
    CCMD(vanilla, "", "", null_func1, vanilla_func2, true, "[" BOLD("on") "|" BOLD("off") "]",
        "Toggles vanilla mode."),
    CVAR_STR(version, "", "", null_func1, str_cvars_func2, CF_READONLY, 16,
        ITALICS(DOOMRETRO_NAME "'s") " version."),
    CVAR_INT(vid_aspectratio, "", "", vid_aspectratio_func1, vid_aspectratio_func2, CF_NONE, RATIOVALUEALIAS,
        "The aspect ratio of the display when in widescreen (" BOLD("16:9") ", " BOLD("16:10") ", " BOLD("21:9") ", "
        BOLD("32:9") " or " BOLD("auto") ")."),
    CVAR_INT(vid_blue, "", "", int_cvars_func1, vid_blue_func2, CF_PERCENT, NOVALUEALIAS,
        "The intensity of blue on the screen (" BOLD("-100%") " to " BOLD("100%") ")."),
    CVAR_BOOL(vid_borderlesswindow, "", "", bool_cvars_func1, vid_borderlesswindow_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles using a borderless window when fullscreen."),
    CVAR_INT(vid_brightness, "", "", int_cvars_func1, vid_brightness_func2, CF_PERCENT, NOVALUEALIAS,
        "The screen's brightness (" BOLD("-100%") " to " BOLD("100%") ")."),
    CVAR_INT(vid_capfps, "", "", vid_capfps_func1, vid_capfps_func2, CF_NONE, CAPVALUEALIAS,
        "The number of frames at which to cap the framerate (" BOLD("off") ", or " BOLD("35") " to " BOLD("1,000") "). "
        "There is no interpolation between frames when this CVAR is " BOLD("35") "."),
    CVAR_INT(vid_contrast, "", "", int_cvars_func1, vid_contrast_func2, CF_PERCENT, NOVALUEALIAS,
        "The screen's contrast (" BOLD("-100%") " to " BOLD("100%") ")."),
    CVAR_INT(vid_display, "", "", int_cvars_func1, vid_display_func2, CF_NONE, NOVALUEALIAS,
        "The display used to play " ITALICS(DOOMRETRO_NAME) " on."),
#if !defined(_WIN32)
    CVAR_STR(vid_driver, "", "", null_func1, str_cvars_func2, CF_NONE, 16,
        "The video driver used to play " ITALICS(DOOMRETRO_NAME) "."),
#endif
    CVAR_BOOL(vid_fullscreen, "", "", bool_cvars_func1, vid_fullscreen_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles between fullscreen and a window."),
    CVAR_INT(vid_green, "", "", int_cvars_func1, vid_green_func2, CF_PERCENT, NOVALUEALIAS,
        "The intensity of green on the screen (" BOLD("-100%") " to " BOLD("100%") ")."),
    CVAR_INT(vid_motionblur, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount of motion blur when you turn quickly (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(vid_pillarboxes, "", "", bool_cvars_func1, vid_pillarboxes_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles using the pillarboxes either side of the screen for certain effects when not in widescreen."),
    CVAR_INT(vid_red, "", "", int_cvars_func1, vid_red_func2, CF_PERCENT, NOVALUEALIAS,
        "The intensity of red on the screen (" BOLD("-100%") " to " BOLD("100%") ")."),
    CVAR_INT(vid_saturation, "", "", int_cvars_func1, vid_saturation_func2, CF_PERCENT, NOVALUEALIAS,
        "The screen's saturation (" BOLD("-100%") " to " BOLD("100%") ")."),
#if defined(_WIN32)
    CVAR_STR(vid_scaleapi, "", "", vid_scaleapi_func1, vid_scaleapi_func2, CF_NONE, 16,
        "The API used to scale every frame (" BOLD("\"direct3d9\"") ", " BOLD("\"direct3d11\"") ", "
        BOLD("\"opengl\"") " or " BOLD("\"software\"") ")."),
#else
    CVAR_STR(vid_scaleapi, "", "", vid_scaleapi_func1, vid_scaleapi_func2, CF_NONE, 16,
        "The API used to scale every frame (" BOLD("\"opengl\"") ", " BOLD("\"opengles\"") ", " BOLD("\"opengles2\"") " or "
        BOLD("\"software\"") ")."),
#endif
    CVAR_STR(vid_scalefilter, "", "", vid_scalefilter_func1, vid_scalefilter_func2, CF_NONE, 16,
        "The filter applied when scaling every frame (" BOLD("\"nearest\"") ", " BOLD("\"linear\"") " or " BOLD("\"nearest_linear\"") ")."),
    CVAR_OTHER(vid_screenresolution, "", "", null_func1, vid_screenresolution_func2,
        "The screen's resolution when fullscreen (" BOLD("desktop") " or " BOLD(ITALICS("width") "\xD7" ITALICS("height")) ")."),
    CVAR_BOOL(vid_showfps, "", "", bool_cvars_func1, vid_showfps_func2, CF_STARTUPRESET, BOOLVALUEALIAS,
        "Toggles showing the number of frames per second."),
#if defined(__APPLE__)
    CVAR_INT(vid_vsync, "", "", vid_vsync_func1, vid_vsync_func2, CF_NONE, VSYNCVALUEALIAS,
        "Toggles vertical sync with the display's refresh rate (" BOLD("on") " or " BOLD("off") ")."),
#else
    CVAR_INT(vid_vsync, "", "", vid_vsync_func1, vid_vsync_func2, CF_NONE, VSYNCVALUEALIAS,
        "Toggles vertical sync with the display's refresh rate (" BOLD("on") ", " BOLD("off") " or " BOLD("adaptive") ")."),
#endif
    CVAR_BOOL(vid_widescreen, "", "", bool_cvars_func1, vid_widescreen_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles widescreen."),
    CVAR_OTHER(vid_windowpos, "", vid_windowposition, null_func1, vid_windowpos_func2,
        "The position of the window on the desktop (" BOLD("centered") " or " BOLD("(") BOLDITALICS("x") BOLD(",") BOLDITALICS("y")
        BOLD(")") ")."),
    CVAR_OTHER(vid_windowsize, "", "", null_func1, vid_windowsize_func2,
        "The size of the window on the desktop (" BOLD(ITALICS("width") "\xD7" ITALICS("height")) ")."),
#if defined(_WIN32)
    CVAR_STR(wad, "", "", null_func1, str_cvars_func2, CF_READONLY, MAX_PATH,
        "The last WAD to be opened by the WAD launcher."),
#endif
    CVAR_STR(wadfolder, "", "", null_func1, str_cvars_func2, CF_NONE, MAX_PATH,
        "The folder the currently loaded WAD is in."),
    CVAR_INT(warninglevel, "", "", int_cvars_func1, int_cvars_func2, CF_NONE, NOVALUEALIAS,
        "The console's warning level (" BOLD("0") ", " BOLD("1") " or " BOLD("2") ")."),
    CVAR_INT(weapon, "", "", weapon_func1, weapon_func2, CF_NONE, WEAPONVALUEALIAS,
        "Your currently equipped weapon (" BOLD("fists") ", " BOLD("chainsaw") ", " BOLD("pistol") ", " BOLD("shotgun") ", "
        BOLD("chaingun") ", " BOLD("rocketlauncher") ", " BOLD("plasmarifle") " or " BOLD("bfg9000") ")."),
    CVAR_INT(weaponbob, "", "", int_cvars_func1, int_cvars_func2, CF_PERCENT, NOVALUEALIAS,
        "The amount your weapon bobs up and down as you move (" BOLD("0%") " to " BOLD("100%") ")."),
    CVAR_BOOL(weaponbounce, "", "", bool_cvars_func1, bool_cvars_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the bounce of your weapon when you land after a fall."),
    CVAR_BOOL(weaponrecoil, "", "", bool_cvars_func1, weaponrecoil_func2, CF_NONE, BOOLVALUEALIAS,
        "Toggles the recoil of your weapon when you fire it."),
    CCMD(wiki, help, "", null_func1, wiki_func2, false, "",
        "Opens the " ITALICS(DOOMRETRO_WIKINAME ".")),

    { "", "", "", null_func1, NULL, 0, 0, CF_NONE, 0, NULL, 0, 0, 0, "", "" }
};

static bool run(void)
{
    return ((gamekeydown[keyboardrun] || mousebuttons[mouserun]
        || (controllerbuttons & controllerrun)) ^ alwaysrun);
}

static bool strafe(void)
{
    return (gamekeydown[keyboardstrafe] || mousebuttons[mousestrafe]
        || (controllerbuttons & controllerstrafe));
}

static void alwaysrun_action_func(void)
{
    G_ToggleAlwaysRun(ev_none);
    I_InitKeyboard();
}

static void automap_action_func(void)
{
    if (gamestate != GS_LEVEL || mapwindow)
        return;

    if (!automapactive)
        AM_Start(true);
    else
        AM_Stop();
}

static void back_action_func(void)
{
    if (gamestate == GS_LEVEL)
    {
        viewplayer->cmd.forwardmove -= forwardmove[run()];
        P_MovePlayer();
    }
}

static void clearmark_action_func(void)
{
    if (gamestate == GS_LEVEL && (automapactive || mapwindow))
        AM_ClearMarks();
}

static void console_action_func(void)
{
    if (consoleactive)
        C_HideConsole();
    else
        C_ShowConsole(false);
}

static void fire_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_FireWeapon();
}

static void followmode_action_func(void)
{
    if (gamestate == GS_LEVEL && (automapactive || mapwindow))
        AM_ToggleFollowMode(!am_followmode);
}

static void forward_action_func(void)
{
    if (gamestate == GS_LEVEL)
    {
        viewplayer->cmd.forwardmove += forwardmove[run()];
        P_MovePlayer();
    }
}

static void grid_action_func(void)
{
    if (gamestate == GS_LEVEL && (automapactive || mapwindow))
        AM_ToggleGrid();
}

static void jump_action_func(void)
{
    if (gamestate == GS_LEVEL && !nojump)
        viewplayer->cmd.buttons |= BT_JUMP;
}

static void left_action_func(void)
{
    if (gamestate == GS_LEVEL)
    {
        if (strafe())
            viewplayer->cmd.sidemove -= sidemove[run()];
        else
            viewplayer->cmd.angleturn -= angleturn[run()];
    }
}

static void mark_action_func(void)
{
    if (gamestate == GS_LEVEL && (automapactive || mapwindow))
        AM_AddMark();
}

static void maxzoom_action_func(void)
{
    if (gamestate == GS_LEVEL && (automapactive || mapwindow))
        AM_ToggleMaxZoom();
}

static void menu_action_func(void)
{
    M_OpenMainMenu();
    S_StartSound(NULL, sfx_swtchn);
}

static void nextweapon_action_func(void)
{
    if (gamestate == GS_LEVEL)
        G_NextWeapon();
}

static void prevweapon_action_func(void)
{
    if (gamestate == GS_LEVEL)
        G_PrevWeapon();
}

static void right_action_func(void)
{
    if (gamestate == GS_LEVEL)
    {
        if (strafe())
            viewplayer->cmd.sidemove += sidemove[run()];
        else
            viewplayer->cmd.angleturn += angleturn[run()];
    }
}

static void rotatemode_action_func(void)
{
    if (gamestate == GS_LEVEL && (automapactive || mapwindow))
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
    if (gamestate == GS_LEVEL)
        viewplayer->cmd.sidemove -= sidemove[run()];
}

static void straferight_action_func(void)
{
    if (gamestate == GS_LEVEL)
        viewplayer->cmd.sidemove += sidemove[run()];
}

static void use_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_UseLines();
}

static void weapon1_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_ChangeWeapon(wp_fist);
}

static void weapon2_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_ChangeWeapon(wp_pistol);
}

static void weapon3_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_ChangeWeapon(wp_shotgun);
}

static void weapon4_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_ChangeWeapon(wp_chaingun);
}

static void weapon5_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_ChangeWeapon(wp_missile);
}

static void weapon6_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_ChangeWeapon(wp_plasma);
}

static void weapon7_action_func(void)
{
    if (gamestate == GS_LEVEL)
        P_ChangeWeapon(wp_bfg);
}

static void zoomin_action_func(void)
{
    if (gamestate == GS_LEVEL && (automapactive || mapwindow))
        AM_ToggleZoomIn();
}

static void zoomout_action_func(void)
{
    if (gamestate == GS_LEVEL && (automapactive || mapwindow))
        AM_ToggleZoomOut();
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

    if (english == english_british)
        M_AmericanToBritishEnglish(description);

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

    if (english == english_british)
        M_AmericanToBritishEnglish(format);

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
        return game_ccmd_func1(cmd, parms);
    else if (viewplayer->health > 0)
        return true;
    else
    {
        C_Input(consoleinput);
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(playername, playername_default))
            C_Warning(0, DEADPLAYERWARNING1);
        else
            C_Warning(0, DEADPLAYERWARNING2, playername, pronoun(personal),
                (playergender == playergender_other ? "are" : "is"));

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
            && (!BTSX || W_GetNumLumps(mapcmdlump) > 1));

        if (gamestate == GS_LEVEL)
            return result;
        else if (result)
        {
            S_StartSound(NULL, sfx_getpow);
            ST_PlayerCheated(cheat_clev_xy.sequence, "xy", NULL, true);
            map_func2("map", mapcmdlump);
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
        return (!nomusic && musicvolume);
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

static bool game_cvar_func1(char *cmd, char *parms)
{
    if (gamestate == GS_LEVEL)
        return true;

    if (!togglingvanilla)
    {
        C_Input(consoleinput);
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(playername, playername_default))
            C_Warning(0, NOGAMECVARWARNING1);
        else
            C_Warning(0, NOGAMECVARWARNING2, playername, pronoun(personal),
                (playergender == playergender_other ? "aren't" : "isn't"));

        consoleinput[0] = '\0';
    }

    return false;
}

static bool game_ccmd_func1(char *cmd, char *parms)
{
    if (gamestate == GS_LEVEL)
        return true;

    if (!togglingvanilla)
    {
        C_Input(consoleinput);
        C_ShowDescription(C_GetIndex(cmd));

        if (M_StringCompare(playername, playername_default))
            C_Warning(0, NOGAMECCMDWARNING1);
        else
            C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                (playergender == playergender_other ? "aren't" : "isn't"));

        consoleinput[0] = '\0';
    }

    return false;
}

static bool nightmare_func1(char *cmd, char *parms)
{
    if (gamestate != GS_LEVEL)
        return game_ccmd_func1(cmd, parms);

    if (gameskill != sk_nightmare)
        return true;

    C_Input(consoleinput);
    C_ShowDescription(C_GetIndex(cmd));

    if (M_StringCompare(playername, playername_default))
        C_Warning(0, NIGHTMAREWARNING1);
    else
        C_Warning(0, NIGHTMAREWARNING2, playername, pronoun(personal),
            (playergender == playergender_other ? "are" : "is"));

    consoleinput[0] = '\0';

    return false;
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
                char    *strings[255] = { "" };
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

void alias_func2(char *cmd, char *parms)
{
    char    parm1[128] = "";
    char    parm2[128] = "";

    if (sscanf(parms, "%127s %127[^\n]", parm1, parm2) <= 0)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
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

bool IsControlBound(const controltype_t type, const int control)
{
    for (int i = 0; *actions[i].action; i++)
        if (type == keyboardcontrol)
        {
            if (actions[i].keyboard1 && controls[control].value == *(int *)actions[i].keyboard1)
                return true;

            if (actions[i].keyboard2 && controls[control].value == *(int *)actions[i].keyboard2)
                return true;
        }
        else if (type == mousecontrol)
        {
            if (actions[i].mouse1 && controls[control].value == *(int *)actions[i].mouse1)
                return true;
        }
        else if (type == controllercontrol)
        {
            if (actions[i].controller1 && controls[control].value == *(int *)actions[i].controller1)
                return true;

            if (actions[i].controller2 && controls[control].value == *(int *)actions[i].controller2)
                return true;
        }

    return false;
}

//
// bind CCMD
//
static void C_UnbindDuplicates(const int keep, const controltype_t type, const int control)
{
    for (int i = 0; *actions[i].action; i++)
        if (i != keep && actions[i].automaponly == actions[keep].automaponly)
        {
            if (type == keyboardcontrol)
            {
                if (actions[i].keyboard1 && controls[control].value == *(int *)actions[i].keyboard1)
                {
                    if (strlen(controls[i].control) == 1)
                        C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                            " action has therefore been unbound from the " BOLD("'%s'") " key.",
                            actions[i].action, controls[control].control);
                    else
                        C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                            " action has therefore been unbound from the " BOLD("%s") " key.",
                            actions[i].action, controls[control].control);

                    *(int *)actions[i].keyboard1 = 0;
                }

                if (actions[i].keyboard2 && controls[control].value == *(int *)actions[i].keyboard2)
                {
                    if (strlen(controls[i].control) == 1)
                        C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                            " action has therefore been unbound from the " BOLD("'%s'") " key.",
                            actions[i].action, controls[control].control);
                    else
                        C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                            " action has therefore been unbound from the " BOLD("%s") " key.",
                            actions[i].action, controls[control].control);

                    *(int *)actions[i].keyboard2 = 0;
                }
            }
            else if (type == mousecontrol)
            {
                if (actions[i].mouse1 && controls[control].value == *(int *)actions[i].mouse1)
                {
                    C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                        " action has therefore been unbound from " BOLD("%s") ".",
                        actions[i].action, controls[control].control);
                    *(int *)actions[i].mouse1 = -1;
                }
            }
            else if (type == controllercontrol)
            {
                if (actions[i].controller1 && controls[control].value == *(int *)actions[i].controller1)
                {
                    C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                        " action has therefore been unbound from " BOLD("%s") ".",
                        actions[i].action, controls[control].control);
                    *(int *)actions[i].controller1 = 0;
                }

                if (actions[i].controller2 && controls[control].value == *(int *)actions[i].controller2)
                {
                    C_Warning(1, "Controls may only be bound to one action. The duplicate " BOLD("%s")
                        " action has therefore been unbound from " BOLD("%s") ".",
                        actions[i].action, controls[control].control);
                    *(int *)actions[i].controller2 = 0;
                }
            }
        }
}

void bind_func2(char *cmd, char *parms)
{
    int         i = 0;
    int         action = 0;
    char        parm1[128] = "";
    char        parm2[128] = "";
    const bool  freelookcontrols = (keyboardfreelook || controllerfreelook || mousefreelook != -1);

    if (sscanf(parms, "%127s %127[^\n]", parm1, parm2) <= 0)
    {
        i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
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
                            if (!nobindoutput)
                                C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                    actions[action].action, controls[i].control);

                            *(int *)actions[action].keyboard1 = 0;
                            M_SaveCVARs();
                        }

                        if (actions[action].keyboard2 && controls[i].value == *(int *)actions[action].keyboard2)
                        {
                            if (!nobindoutput)
                                C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                    actions[action].action, controls[i].control);

                            *(int *)actions[action].keyboard2 = 0;
                            M_SaveCVARs();
                        }

                        break;

                    case mousecontrol:
                        if (actions[action].mouse1 && controls[i].value == *(int *)actions[action].mouse1)
                        {
                            if (!nobindoutput)
                                C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                    actions[action].action, controls[i].control);

                            *(int *)actions[action].mouse1 = -1;
                            M_SaveCVARs();
                        }

                        break;

                    case controllercontrol:
                        if (actions[action].controller1 && controls[i].value == *(int *)actions[action].controller1)
                        {
                            if (!nobindoutput)
                                C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                    actions[action].action, controls[i].control);

                            *(int *)actions[action].controller1 = 0;
                            M_SaveCVARs();
                        }

                        if (actions[action].controller2 && controls[i].value == *(int *)actions[action].controller2)
                        {
                            if (!nobindoutput)
                                C_Output("The " BOLD("%s") " action has been unbound from the " BOLD("%s") " control.",
                                    actions[action].action, controls[i].control);

                            *(int *)actions[action].controller2 = 0;
                            M_SaveCVARs();
                        }

                        break;

                    default:
                        break;
                }

                action++;
            }

            if (controls[i].type == keyboardcontrol && *keyactionlist[controls[i].value])
            {
                C_Output(BOLD("\"%s\"") " has been unbound from the " BOLD("%s") " control.",
                    keyactionlist[controls[i].value], controls[i].control);
                keyactionlist[controls[i].value][0] = '\0';
            }
            else if (controls[i].type == mousecontrol && *mouseactionlist[controls[i].value])
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
                else if (controls[i].type == controllercontrol)
                {
                    if (actions[action].controller1 && controls[i].value == *(int *)actions[action].controller1)
                        C_Output(actions[action].action);
                    else if (actions[action].controller2 && controls[i].value == *(int *)actions[action].controller2)
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

                    case controllercontrol:
                        if (actions[action].controller1)
                        {
                            if (actions[action].controller2
                                && *(int *)actions[action].controller1
                                && *(int *)actions[action].controller1 != controls[i].value)
                            {
                                if (*(int *)actions[action].controller2)
                                {
                                    *(int *)actions[action].controller2 = *(int *)actions[action].controller1;
                                    *(int *)actions[action].controller1 = controls[i].value;
                                }
                                else
                                    *(int *)actions[action].controller2 = controls[i].value;
                            }
                            else
                                *(int *)actions[action].controller1 = controls[i].value;

                            bound = true;
                            C_UnbindDuplicates(action, controllercontrol, i);
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
                            C_Output("The " BOLD("%s") " action was bound to the " BOLD("'%s'") " key.",
                                parm2, controls[i].control);
                        else
                            C_Output("The " BOLD("%s") " action was bound to the " BOLD("%s") " control.",
                                parm2, controls[i].control);
                    }
                }
                else
                {
                    if (strlen(controls[i].control) == 1)
                        C_Warning(0, "The " BOLD("%s") " action can't be bound to the " BOLD("'%s'") " key.",
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
                    M_StripQuotes(parm2);
                    M_StringCopy(keyactionlist[controls[i].value], parm2, sizeof(keyactionlist[0]));
                    M_SaveCVARs();

                    if (!nobindoutput)
                    {
                        if (strlen(controls[i].control) == 1)
                            C_Output(BOLD("\"%s\"") " was bound to the " BOLD("'%s'") " key.",
                                parm2, controls[i].control);
                        else
                            C_Output(BOLD("\"%s\"") " was bound to the " BOLD("%s") " key.",
                                parm2, controls[i].control);
                    }
                }
                else if (controls[i].type == mousecontrol)
                {
                    M_StripQuotes(parm2);
                    M_StringCopy(mouseactionlist[controls[i].value], parm2, sizeof(mouseactionlist[0]));
                    M_SaveCVARs();

                    if (!nobindoutput)
                        C_Output(BOLD("\"%s\"") " was bound to the " BOLD("%s") " control.",
                            parm2, controls[i].control);
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

                if (actions[action].controller1)
                    *(int *)actions[action].controller1 = 0;

                if (actions[action].controller2)
                    *(int *)actions[action].controller2 = 0;

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

    if (freelookcontrols != (keyboardfreelook || controllerfreelook || mousefreelook != -1))
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
static void C_DisplayBinds(const char *action, const int value, const controltype_t type, const int *tabs)
{
    for (int i = 0; controls[i].type; i++)
    {
        if (controls[i].type == type && controls[i].value == value)
        {
            const char  *control = controls[i].control;

            if (strlen(control) == 1)
                C_TabbedOutput(tabs, "'%s'\t%s", (control[0] == '=' ? "+" : control), action);
            else
                C_TabbedOutput(tabs, "%s\t%s", control, action);

            break;
        }
    }
}

static void bindlist_func2(char *cmd, char *parms)
{
    const int   tabs[MAXTABS] = { 110 };

    C_Header(tabs, bindlist, BINDLISTHEADER);

    for (int i = 0; *actions[i].action; i++)
    {
        if (actions[i].keyboard1)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].keyboard1, keyboardcontrol, tabs);

        if (actions[i].keyboard2)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].keyboard2, keyboardcontrol, tabs);
    }

    for (int i = 0; controls[i].type; i++)
    {
        const int   value = controls[i].value;

        if (controls[i].type == keyboardcontrol && keyactionlist[value][0])
        {
            const char  *control = controls[i].control;

            if (strlen(control) == 1)
                C_TabbedOutput(tabs, "'%s'\t\"%s\"", (control[0] == '=' ? "+" : control), keyactionlist[value]);
            else
                C_TabbedOutput(tabs, "%s\t\"%s\"", control, keyactionlist[value]);
        }
    }

    for (int i = 0; *actions[i].action; i++)
        if (actions[i].mouse1)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].mouse1, mousecontrol, tabs);

    for (int i = 0; controls[i].type; i++)
    {
        const int   value = controls[i].value;

        if (controls[i].type == mousecontrol && mouseactionlist[value][0])
            C_TabbedOutput(tabs, "%s\t%s", controls[i].control, mouseactionlist[value]);
    }

    for (int i = 0; *actions[i].action; i++)
    {
        if (actions[i].controller1)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].controller1, controllercontrol, tabs);

        if (actions[i].controller2)
            C_DisplayBinds(actions[i].action, *(int *)actions[i].controller2, controllercontrol, tabs);
    }
}

//
// clear CCMD
//
static void clear_func2(char *cmd, char *parms)
{
    C_ClearConsole();
}

//
// cmdlist CCMD
//
static void cmdlist_func2(char *cmd, char *parms)
{
    const int   tabs[MAXTABS] = { 326 };
    const int   columnwidth = tabs[0] - 15;

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

            if (english == english_british)
            {
                M_AmericanToBritishEnglish(format);
                M_AmericanToBritishEnglish(description);
            }

            C_TabbedOutput(tabs, "%s\t" BOLDOFF ITALICSOFF "%s", format, description);
        }
}

//
// condump CCMD
//
static bool condump_func1(char *cmd, char *parms)
{
    return (numconsolestrings > CONSOLEBLANKLINES);
}

static void condump_func2(char *cmd, char *parms)
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

        M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "%s.txt", consolefolder, cmd);

        while (M_FileExists(filename))
        {
            char    *temp = commify(++count);

            M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "%s (%s).txt", consolefolder, cmd, temp);
            free(temp);
        }
    }
    else
        M_snprintf(filename, sizeof(filename), "%s" DIR_SEPARATOR_S "%s%s",
            consolefolder, parms, (strchr(parms, '.') ? "" : ".txt"));

    if ((file = fopen(filename, "wt")))
    {
        char    *temp = commify((int64_t)numconsolestrings - CONSOLEBLANKLINES - 1);

        for (int i = 1; i < numconsolestrings - 1; i++)
        {
            stringtype_t    type = console[i].stringtype;

            if (type == dividerstring)
                fprintf(file, "%s\n", DIVIDERSTRING);
            else
            {
                char            *string = M_StringDuplicate(console[i].string);
                const int       len = (int)strlen(string);
                unsigned int    outpos = 0;
                int             tabcount = 0;

                if (!len)
                    continue;

                if (type == warningstring || type == playerwarningstring)
                    fputs("! ", file);

                for (int inpos = 0; inpos < len; inpos++)
                {
                    const unsigned char letter = string[inpos];

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
                    else if (letter != '\n'
                        && letter != BOLDONCHAR && letter != BOLDOFFCHAR
                        && letter != ITALICSONCHAR && letter != ITALICSOFFCHAR
                        && letter != MONOSPACEDONCHAR && letter != MONOSPACEDOFFCHAR)
                    {
                        fputc(letter, file);
                        outpos++;
                    }
                }

                if (type == playermessagestring || type == playerwarningstring)
                {
                    char    buffer[9];

                    for (unsigned int spaces = (type == playermessagestring ? 0 : 2); spaces < 92 - outpos; spaces++)
                        fputc(' ', file);

                    M_StringCopy(buffer, C_CreateTimeStamp(i), sizeof(buffer));

                    if (strlen(buffer) == 7)
                        fputc(' ', file);

                    fputs(buffer, file);
                }

                fputc('\n', file);
                free(string);
            }
        }

        fclose(file);

        C_Output("%s lines from the console were dumped into " BOLD("%s") ".", temp, filename);
        free(temp);
    }
    else
        C_Warning(0, BOLD("%s") " couldn't be created.", filename);
}

//
// cvarlist CCMD
//
static void cvarlist_func2(char *cmd, char *parms)
{
    const int   tabs[MAXTABS] = { 190, 299 };

    for (int i = 0, count = 0; *consolecmds[i].name; i++)
        if (consolecmds[i].type == CT_CVAR)
        {
            char    name[255];
            char    description[255];

            M_StringCopy(name, (english == english_american
                || M_StringCompare(consolecmds[i].altspelling, EMPTYVALUE) ? consolecmds[i].name :
                consolecmds[i].altspelling), sizeof(name));

            if (*parms && !wildcard(name, parms))
                continue;

            if (++count == 1)
                C_Header(tabs, cvarlist, CVARLISTHEADER);

            M_StringCopy(description, consolecmds[i].description, sizeof(description));

            if (english == english_british)
                M_AmericanToBritishEnglish(description);

            if (M_StringCompare(name, stringize(ammo)))
            {
                if (gamestate == GS_LEVEL)
                {
                    int value = viewplayer->ammo[weaponinfo[viewplayer->readyweapon].ammotype];

                    if (value == consolecmds[i].defaultnumber)
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%i") "\t%s", name, value, description);
                    else
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%i") "\t%s", name, value, description);
                }
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%i") "\t%s", name, ammo_default, description);
            }
            else if (M_StringCompare(name, stringize(armor)))
            {
                if (gamestate == GS_LEVEL)
                {
                    int value = viewplayer->armor;

                    if (value == consolecmds[i].defaultnumber)
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%i%%") "\t%s", name, value, description);
                    else
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%i%%") "\t%s", name, value, description);
                }
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%i%%") "\t%s", name, armor_default, description);
            }
            else if (M_StringCompare(name, stringize(armortype)))
            {
                if (gamestate == GS_LEVEL)
                {
                    int     value = viewplayer->armortype;
                    char    *temp = C_LookupAliasFromValue(value, ARMORTYPEVALUEALIAS);

                    if (value == consolecmds[i].defaultnumber)
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s", name, temp, description);
                    else
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%s") "\t%s", name, temp, description);

                    free(temp);
                }
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("none") "\t%s", name, description);
            }
            else if (M_StringCompare(name, stringize(health)))
            {
                if (gamestate == GS_LEVEL)
                {
                    int value = (negativehealth && minuspatch && !viewplayer->health ?
                        viewplayer->negativehealth : viewplayer->health);

                    if (value == consolecmds[i].defaultnumber)
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%i%%") "\t%s", name, value, description);
                    else
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%i%%") "\t%s", name, value, description);
                }
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%i%%") "\t%s", name, health_default, description);
            }
            else if (M_StringCompare(name, stringize(weapon)))
            {
                if (gamemode == shareware)
                    M_StringCopy(description, WEAPONDESCRIPTION_SHAREWARE, sizeof(description));
                else if (gamemission != doom)
                    M_StringCopy(description, WEAPONDESCRIPTION_DOOM2, sizeof(description));

                if (gamestate == GS_LEVEL)
                {
                    const int   value = viewplayer->readyweapon;
                    char        *temp = C_LookupAliasFromValue(value, WEAPONVALUEALIAS);

                    if (value == consolecmds[i].defaultnumber)
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s", name, temp, description);
                    else
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%s") "\t%s", name, temp, description);

                    free(temp);
                }
                else
                {
                    char    *temp = C_LookupAliasFromValue(weapon_default, WEAPONVALUEALIAS);

                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s", name, temp, description);
                    free(temp);
                }
            }
            else if (M_StringCompare(name, stringize(r_fov)))
            {
                const int   value = *(int *)consolecmds[i].variable;

                if (value == consolecmds[i].defaultnumber)
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%i") "\xB0\t%s", name, value, description);
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%i") "\xB0\t%s", name, value, description);
            }
            else if (consolecmds[i].flags & CF_BOOLEAN)
            {
                const bool  value = *(bool *)consolecmds[i].variable;
                char        *temp = C_LookupAliasFromValue(value, consolecmds[i].aliases);

                if (value == consolecmds[i].defaultnumber)
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s", name, temp, description);
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%s") "\t%s", name, temp, description);

                free(temp);
            }
            else if ((consolecmds[i].flags & CF_INTEGER) && (consolecmds[i].flags & CF_PERCENT))
            {
                const int   value = *(int *)consolecmds[i].variable;

                if (value == consolecmds[i].defaultnumber)
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%i%%") "\t%s", name, value, description);
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%i%%") "\t%s", name, value, description);
            }
            else if (consolecmds[i].flags & CF_INTEGER)
            {
                const int   value = *(int *)consolecmds[i].variable;
                char        *temp = C_LookupAliasFromValue(value, consolecmds[i].aliases);

                if (value == consolecmds[i].defaultnumber)
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s", name, temp, description);
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%s") "\t%s", name, temp, description);

                free(temp);
            }
            else if (consolecmds[i].flags & CF_FLOAT)
            {
                const float value = *(float *)consolecmds[i].variable;

                if (consolecmds[i].flags & CF_PERCENT)
                {
                    char    *temp = striptrailingzero(value, 1);

                    if (value == consolecmds[i].defaultnumber)
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s%%") "\t%s", name, temp, description);
                    else
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%s%%") "\t%s", name, temp, description);

                    free(temp);
                }
                else if (M_StringCompare(consolecmds[i].name, stringize(r_gamma)))
                {
                    char    buffer[128];
                    int     len;

                    M_snprintf(buffer, sizeof(buffer), "%.2f", value);
                    len = (int)strlen(buffer);

                    if (len >= 2 && buffer[len - 1] == '0' && buffer[len - 2] == '0')
                        buffer[len - 1] = '\0';

                    if (value == consolecmds[i].defaultnumber)
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s", name, buffer, description);
                    else
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%s") "\t%s", name, buffer, description);
                }
                else
                {
                    char    *temp = striptrailingzero(value, 1);

                    if (value == consolecmds[i].defaultnumber)
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s", name, temp, description);
                    else
                        C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%s") "\t%s", name, temp, description);

                    free(temp);
                }
            }
            else if (M_StringCompare(name, stringize(version)))
                C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s",
                    name, *(char **)consolecmds[i].variable, description);
            else if (consolecmds[i].flags & CF_STRING)
            {
                const char  *value = *(char **)consolecmds[i].variable;

                if (M_StringCompare(value, consolecmds[i].defaultstring))
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("\"%.14s%s\"") "\t%s",
                        name, value, (strlen(value) > 14 ? "..." : ""), description);
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("\"%.14s%s\"") "\t%s",
                        name, value, (strlen(value) > 14 ? "..." : ""), description);
            }
            else if (consolecmds[i].flags & CF_TIME)
            {
                int tics = *(int *)consolecmds[i].variable / TICRATE;
                int hours = tics / 3600;
                int minutes = ((tics %= 3600)) / 60;
                int seconds = tics % 60;

                if (!tics)
                    C_TabbedOutput(tabs, BOLD("%s")
                        "\t" BOLD(MONOSPACED("%02i") ":" MONOSPACED("%02i")) "\t%s",
                        name, minutes, seconds, description);
                else if (!hours)
                    C_TabbedOutput(tabs, BOLD("%s")
                        "\t" BOLDER(MONOSPACED("%02i") ":" MONOSPACED("%02i")) "\t%s",
                        name, minutes, seconds, description);
                else
                    C_TabbedOutput(tabs, BOLD("%s")
                        "\t" BOLDER(MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i")) "\t%s",
                        name, hours, minutes, seconds, description);
            }
            else if (consolecmds[i].flags & CF_OTHER)
            {
                char        temp[255];
                const char  *value = *(char **)consolecmds[i].variable;

                M_StringCopy(temp, value, sizeof(temp));

                if (english == english_british)
                    M_AmericanToBritishEnglish(temp);

                if (M_StringCompare(value, consolecmds[i].defaultstring))
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLD("%s") "\t%s",
                        name, temp, description);
                else
                    C_TabbedOutput(tabs, BOLD("%s") "\t" BOLDER("%s") "\t%s",
                        name, temp, description);
            }
        }
}

//
// endgame CCMD
//
static void endgame_func2(char *cmd, char *parms)
{
    if (gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (M_StringCompare(playername, playername_default))
            C_Warning(0, NOGAMECCMDWARNING1);
        else
            C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                (playergender == playergender_other ? "aren't" : "isn't"));
    }
    else
        M_EndGame(0);
}

//
// exec CCMD
//
void exec_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
    }
    else
    {
        char    filename[MAX_PATH];
        char    strparm[512] = "";
        char    *temp;
        FILE    *file;
        int     linecount = 0;

        if (strchr(parms, '.'))
            M_StringCopy(filename, parms, sizeof(filename));
        else
            M_snprintf(filename, sizeof(filename), "%s.cfg", parms);

        if (!(file = fopen(filename, "rt")))
        {
            C_Warning(0, BOLD("%s") " couldn't be opened.", parms);
            return;
        }

        parsingcfgfile = true;

        while (fgets(strparm, sizeof(strparm), file))
        {
            if (strparm[0] == ';')
                continue;

            if (C_ValidateInput(strparm))
                linecount++;
        }

        parsingcfgfile = false;
        fclose(file);

        temp = commify(linecount);
        C_Output("%s line%s have been parsed in " BOLD("%s") ".",
            temp, (linecount == 1 ? "" : "s"), leafname(parms));
        free(temp);
    }
}

//
// exitmap CCMD
//
static void exitmap_func2(char *cmd, char *parms)
{
    G_ExitLevel();
    C_HideConsoleFast();
    viewplayer->cheated++;
    stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
    M_SaveCVARs();
}

//
// fastmonsters CCMD
//
static void fastmonsters_func2(char *cmd, char *parms)
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
}

//
// freeze CCMD
//
static void freeze_func2(char *cmd, char *parms)
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
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
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

    C_HideConsole();
}

//
// give CCMD
//
static bool give_func1(char *cmd, char *parms)
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
        || M_StringCompare(parm, "pistol") || M_StringCompare(parm, "powerups"))
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

static void give_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMECCMDWARNING1);
            else
                C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                    (playergender == playergender_other ? "aren't" : "isn't"));
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

            if (P_GiveAllCards())
                result = true;

            for (int i = 0; i < NUMPOWERS; i++)
                if (P_GivePower(i, false))
                    result = true;

            if (result)
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You have been given everything.");
                else
                    C_PlayerMessage("%s has been given everything.", playername);

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
                    C_PlayerMessage("You have been given full health.");
                else
                    C_PlayerMessage("%s has been given full health.", playername);

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
                    C_PlayerMessage("You have been given all your weapons.");
                else
                    C_PlayerMessage("%s has been given all %s weapons.",
                        playername, pronoun(possessive));

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all your weapons.");
                else
                    C_Warning(0, "%s already has all %s weapons.",
                        playername, pronoun(possessive));

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
                    C_PlayerMessage("You have been given full ammo for all your weapons.");
                else
                    C_PlayerMessage("%s has been given full ammo for all %s weapons.",
                        playername, pronoun(possessive));

                C_HideConsole();
            }
            else if (P_GiveBackpack(false, false) && P_GiveFullAmmo())
            {
                P_AddBonus();
                S_StartSound(viewplayer->mo, sfx_itemup);

                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You have been given a backpack and full ammo for all your weapons.");
                else
                    C_PlayerMessage("%s has been given a backpack and full ammo for all %s weapons.",
                        playername, pronoun(possessive));

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have full ammo for all your weapons.");
                else
                    C_Warning(0, "%s already has full ammo for all %s weapons.",
                        playername, pronoun(possessive));

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
                    C_PlayerMessage("You have been given full %s.",
                        (english == english_american ? "armor" : "armour"));
                else
                    C_PlayerMessage("%s has been given full %s.",
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
                    C_PlayerMessage("You have been given all keycards and skull keys.");
                else
                    C_PlayerMessage("%s has been given all keycards and skull keys.", playername);

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
                    C_PlayerMessage("You have been given all keycards.");
                else
                    C_PlayerMessage("%s has been given all keycards.", playername);

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
                    C_PlayerMessage("You have been given all skull keys.");
                else
                    C_PlayerMessage("%s has been given all skull keys.", playername);

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
            viewplayer->ammo[am_clip] = MAX(1, viewplayer->ammo[am_clip]);

            if (M_StringCompare(playername, playername_default))
                C_PlayerMessage("You have been given a pistol.");
            else
                C_PlayerMessage("%s has been given a pistol.", playername);

            C_HideConsole();
            free(parm);

            return;
        }
        else if (M_StringCompare(parm, "powerups"))
        {
            bool    result = false;

            for (int i = 0; i < NUMPOWERS; i++)
                if (P_GivePower(i, false))
                    result = true;

            if (result)
            {
                if (M_StringCompare(playername, playername_default))
                    C_PlayerMessage("You have been given all the power-ups.");
                else
                    C_PlayerMessage("%s has been given all the power-ups.", playername);

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You already have all the power-ups.");
                else
                    C_Warning(0, "%s already has all the power-ups.", playername);
            }

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
                        C_Warning(0, "%s can't be given %s %s in " ITALICS("%s") "!",
                            (M_StringCompare(playername, playername_default) ? "You" : playername),
                            (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1, gamedescription);
                    else if (gamemode == shareware && (i == MT_MISC7 || i == MT_MISC8 || i == MT_MISC9
                        || i == MT_MISC20 || i == MT_MISC21 || i == MT_MISC25 || i == MT_MISC28))
                    {
                        if (M_StringCompare(playername, playername_default))
                            C_Warning(0, "You can't be given %s %s in the shareware version of " ITALICS("DOOM") "! "
                                "You can buy the full version on " ITALICS("Steam") ", etc.",
                                (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                        else
                            C_Warning(0, "%s can't be given %s %s in the shareware version of " ITALICS("DOOM") "! "
                                "%s can buy the full version on " ITALICS("Steam") ", etc.",
                                playername, (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"),
                                mobjinfo[i].name1, titlecase(pronoun(personal)));
                    }
                    else
                    {
                        bool    old_freeze = freeze;
                        mobj_t  *thing = P_SpawnMobj(viewx, viewy, viewz, i);

                        freeze = false;

                        if (viewplayer->health <= 0)
                        {
                            if (M_StringCompare(playername, playername_default))
                                C_Warning(0, "You can't be given %s %s when you are dead!",
                                    (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                            else
                                C_Warning(0, "%s can't be given %s %s when %s %s dead!",
                                    playername, (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1,
                                    pronoun(personal), (playergender == playergender_other ? "are" : "is"));
                        }
                        else if (P_TouchSpecialThing(thing, viewplayer->mo, false, false))
                        {
                            if (thing->type == MT_MISC0 || thing->type == MT_MISC1)
                            {
                                if (M_StringCompare(playername, playername_default))
                                    C_PlayerMessage("You have been given %s.", mobjinfo[i].name1);
                                else
                                    C_PlayerMessage("%s has been given %s.", playername, mobjinfo[i].name1);
                            }
                            else
                            {
                                if (M_StringCompare(playername, playername_default))
                                    C_PlayerMessage("You have been given %s %s.",
                                        (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                                else
                                    C_PlayerMessage("%s has been given %s %s.",
                                        playername, (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                            }

                            C_HideConsole();
                        }
                        else
                        {
                            C_Warning(0, "%s can't be given another %s!",
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
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
        free(parm);
    }
}

//
// god CCMD
//
static void god_func2(char *cmd, char *parms)
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
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
    }
    else
        C_Output(s_STSTR_GODOFF);
}

//
// if CCMD
//
static bool match(bool value, const char *toggle)
{
    return ((value && M_StringCompare(toggle, "on")) || (!value && M_StringCompare(toggle, "off")));
}

static void if_func2(char *cmd, char *parms)
{
    char    parm1[64] = "";
    char    parm2[64] = "";
    char    parm3[128] = "";

    if (sscanf(parms, "%63s is %63s then %127[^\n]", parm1, parm2, parm3) != 3
        && sscanf(parms, "%63s %63s then %127[^\n]", parm1, parm2, parm3) != 3)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
        return;
    }

    M_StripQuotes(parm1);

    for (int i = 0; *consolecmds[i].name; i++)
        if (M_StringCompare(parm1, consolecmds[i].name))
        {
            bool    condition = false;

            M_StripQuotes(parm2);

            if (M_StringCompare(parm1, "ammo"))
            {
                int value = INT_MIN;

                if (sscanf(parms, "%10i", &value) == 1)
                    condition = (value != INT_MIN
                        && value == viewplayer->ammo[weaponinfo[viewplayer->readyweapon].ammotype]);
            }
            else if (M_StringCompare(parm1, "armor") || M_StringCompare(parm1, "armour"))
            {
                int value = INT_MIN;

                if (sscanf(parms, "%10i", &value) == 1)
                    condition = (value != INT_MIN && value == viewplayer->armor);
            }
            else if (M_StringCompare(parm1, "armortype") || M_StringCompare(parm1, "armourtype"))
            {
                int value = C_LookupValueFromAlias(parm2, ARMORTYPEVALUEALIAS);

                if (value != INT_MIN || sscanf(parms, "%10i", &value) == 1)
                    condition = (value != INT_MIN && value == viewplayer->armortype);
            }
            else if (M_StringCompare(parm1, "health"))
            {
                int value = INT_MIN;

                if (sscanf(parms, "%10i", &value) == 1)
                    condition = (value != INT_MIN && value == viewplayer->health);
            }
            else if (consolecmds[i].type == CT_CVAR)
            {
                if (consolecmds[i].flags & CF_BOOLEAN)
                {
                    int value = C_LookupValueFromAlias(parm2, BOOLVALUEALIAS);

                    condition = ((value == 0 || value == 1) && value == *(bool *)consolecmds[i].variable);
                }
                else if (consolecmds[i].flags & CF_INTEGER)
                {
                    int value = C_LookupValueFromAlias(parm2, consolecmds[i].aliases);

                    if (value != INT_MIN || sscanf(parms, "%10i", &value) == 1)
                        condition = (value != INT_MIN && value == *(int *)consolecmds[i].variable);
                }
                else if (consolecmds[i].flags & CF_FLOAT)
                {
                    float   value = FLT_MIN;

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
            else if (M_StringCompare(parm1, "infiniteammo"))
                condition = match(infiniteammo, parm2);
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
                char    *strings[255] = { "" };
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
// infiniteammo CCMD
//
static void infiniteammo_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if (value == 0 && infiniteammo)
            infiniteammo = false;
        else if (value == 1 && !infiniteammo)
            infiniteammo = true;
        else
            return;
    }
    else
        infiniteammo = !infiniteammo;

    if (infiniteammo)
    {
        C_Output(s_STSTR_IAON);
        HU_SetPlayerMessage(s_STSTR_IAON, false, false);
        viewplayer->cheated++;
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_IAOFF);
        HU_SetPlayerMessage(s_STSTR_IAOFF, false, false);
        P_CheckAmmo(viewplayer->readyweapon);
    }
}

//
// kill CCMD
//
static bool     killcmdfriendly;
static int      killcmdtype = NUMMOBJTYPES;
static mobj_t   *killcmdmobj;
bool            massacre;

static bool kill_func1(char *cmd, char *parms)
{
    bool    result = false;
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
        return true;

    killcmdmobj = NULL;

    if ((killcmdfriendly = M_StringStartsWith(parm, "friendly")))
        M_StringReplaceAll(parm, "friendly", "", false);
    else if (M_StringStartsWith(parm, "unfriendly"))
        M_StringReplaceAll(parm, "unfriendly", "", false);
    else if (M_StringCompare(parm, "dog") || M_StringCompare(parm, "dogs"))
        killcmdfriendly = true;

    if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        result = (viewplayer->health > 0);
    else if (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters") || M_StringCompare(parm, "all")
        || M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
        || M_StringCompare(parm, "missile") || M_StringCompare(parm, "missiles")
        || M_StringCompare(parm, "projectile") || M_StringCompare(parm, "projectiles")
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
                char    *temp1 = removenonalpha(mobjinfo[i].name1);
                char    *temp2 = (*mobjinfo[i].plural1 ? removenonalpha(mobjinfo[i].plural1) : NULL);
                char    *temp3 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char    *temp4 = (*mobjinfo[i].plural2 ? removenonalpha(mobjinfo[i].plural2) : NULL);
                char    *temp5 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);
                char    *temp6 = (*mobjinfo[i].plural3 ? removenonalpha(mobjinfo[i].plural3) : NULL);

                if (M_StringStartsWith(parm, "all"))
                    M_StringReplaceAll(parm, "all", "", false);

                killcmdtype = mobjinfo[i].doomednum;

                if (killcmdtype >= 0
                    && (M_StringCompare(parm, temp1)
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

static void kill_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMECCMDWARNING1);
            else
                C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                    (playergender == playergender_other ? "aren't" : "isn't"));
        }
    }
    else
    {
        char    buffer[1024];
        char    *killed = (M_StringCompare(cmd, "explode") ? "exploded" : (M_StringCompare(cmd, "remove") ? "removed" : "killed"));

        if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        {
            if (viewplayer->cheats & CF_GODMODE)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You can't kill yourself in god mode!");
                else
                    C_Warning(0, "%s can't kill %s in god mode!", playername, pronoun(reflexive));

                return;
            }

            if (viewplayer->powers[pw_invulnerability])
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You can't kill yourself when you have %s %s!",
                        (isvowel(powerups[pw_invulnerability][0]) ? "an" : "a"),
                        powerups[pw_invulnerability]);
                else
                    C_Warning(0, "%s can't kill %s when %s %s %s %s!",
                        playername,
                        pronoun(reflexive),
                        pronoun(personal),
                        (playergender == playergender_other ? "have" : "has"),
                        (isvowel(powerups[pw_invulnerability][0]) ? "an" : "a"),
                        powerups[pw_invulnerability]);

                return;
            }

            if (viewplayer->cheats & CF_BUDDHA)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You can't kill yourself in buddha mode!");
                else
                    C_Warning(0, "%s can't kill %s in buddha mode!", playername, pronoun(reflexive));

                player_cvars_func2(stringize(health), "1");
                return;
            }

            viewplayer->damagecount = MIN(viewplayer->health, 100);
            P_AnimateHealth(viewplayer->health);
            viewplayer->health = 0;
            viewplayer->mo->health = 0;
            healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
            viewplayer->attacker = NULL;

            viewplayer->mo->flags2 |= MF2_MASSACRE;
            massacre = true;
            P_KillMobj(viewplayer->mo, NULL, viewplayer->mo, false);
            massacre = false;

            if (M_StringCompare(playername, playername_default))
                M_snprintf(buffer, sizeof(buffer), "You killed yourself!");
            else
                M_snprintf(buffer, sizeof(buffer), "%s killed %s!", playername, pronoun(reflexive));

            C_PlayerObituary(buffer);
            C_HideConsole();
        }
        else
        {
            const bool  friends = (M_StringCompare(parm, "friend") || M_StringCompare(parm, "friends")
                            || ((M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters")) && killcmdfriendly));
            const bool  enemies = (M_StringCompare(parm, "monster") || M_StringCompare(parm, "monsters"));
            const bool  all = M_StringCompare(parm, "all");
            int         kills = 0;

            if (friends || enemies || all)
            {
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                    {
                        const int   flags = thing->flags;

                        if (all || !!(flags & MF_FRIEND) == friends)
                        {
                            if (thing->flags2 & MF2_MONSTERMISSILE
                                || thing->type == MT_ROCKET
                                || thing->type == MT_PLASMA
                                || thing->type == MT_BFG)
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
                                    viewplayer->monsterskilled[MT_PAIN]++;
                                    stat_monsterskilled[MT_PAIN] = SafeAdd(stat_monsterskilled[MT_PAIN], 1);

                                    if (!(flags & MF_FRIEND))
                                        viewplayer->killcount++;

                                    stat_monsterskilled_total = SafeAdd(stat_monsterskilled_total, 1);
                                    kills++;
                                }
                                else if ((flags & MF_SHOOTABLE) && type != MT_PLAYER && type != MT_BARREL && (type != MT_HEAD || !hacx))
                                {
                                    thing->flags2 |= MF2_MASSACRE;
                                    massacre = true;
                                    P_DamageMobj(thing, viewplayer->mo, viewplayer->mo, thing->health, false, false);
                                    massacre = false;
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
                            M_snprintf(buffer, sizeof(buffer), "You %s the only %smonster in this map!",
                                killed, (friends ? "friendly " : ""));
                        else
                            M_snprintf(buffer, sizeof(buffer), "You %s all %s %smonsters in this map!",
                                killed, temp, (friends ? "friendly " : ""));
                    }
                    else
                    {
                        if (kills == 1)
                            M_snprintf(buffer, sizeof(buffer), "%s %s the only %smonster in this map!",
                                playername, killed, (friends ? "friendly " : ""));
                        else
                            M_snprintf(buffer, sizeof(buffer), "%s %s all %s %smonsters in this map!",
                                playername, killed, temp, (friends ? "friendly " : ""));
                    }

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    viewplayer->cheated++;
                    stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
                    M_SaveCVARs();
                    free(temp);
                }
                else
                    C_Warning(0, "There are no %smonsters to %s.", (friends ? "friendly " : ""), cmd);
            }
            else if (M_StringCompare(parm, "missile") || M_StringCompare(parm, "missiles")
                || M_StringCompare(parm, "projectile") || M_StringCompare(parm, "projectiles"))
            {
                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                        if ((thing->flags2 & MF2_MONSTERMISSILE)
                            || thing->type == MT_ROCKET
                            || thing->type == MT_PLASMA
                            || thing->type == MT_BFG)
                        {
                            P_ExplodeMissile(thing);
                            kills++;
                        }

                if (kills)
                {
                    char    *temp = commify(kills);

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), "You %s %s missile%s.",
                            killed, temp, (kills == 1 ? "" : "s"));
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s %s missile%s.",
                            playername, killed, temp, (kills == 1 ? "" : "s"));

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    viewplayer->cheated++;
                    stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
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
                            killed, temp, (kills == 1 ? "" : "s"));
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s %s item%s.",
                            playername, killed, temp, (kills == 1 ? "" : "s"));

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
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
                            killed, temp, (kills == 1 ? "" : "s"));
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s %s decoration%s.",
                            playername, killed, temp, (kills == 1 ? "" : "s"));

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    viewplayer->cheated++;
                    stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
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
                        else if ((flags & MF_SPECIAL)
                            || (flags2 & MF2_MONSTERMISSILE)
                            || thing->type == MT_ROCKET
                            || thing->type == MT_PLASMA
                            || thing->type == MT_BFG)
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

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    viewplayer->cheated++;
                    stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
                    M_SaveCVARs();
                }
                else
                    C_Warning(0, "There is nothing to %s.", cmd);
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

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
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

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                }
                else
                    C_Warning(0, "There are no blood splats to %s.", cmd);
            }
            else if (killcmdmobj)
            {
                char    *temp = sentencecase(parm);

                if (killcmdmobj->type == MT_PAIN)
                {
                    if (killcmdmobj->health > 0)
                    {
                        A_Fall(killcmdmobj, NULL, NULL);
                        P_SetMobjState(killcmdmobj, S_PAIN_DIE6);
                        viewplayer->monsterskilled[MT_PAIN]++;
                        stat_monsterskilled[MT_PAIN] = SafeAdd(stat_monsterskilled[MT_PAIN], 1);

                        if (!(killcmdmobj->flags & MF_FRIEND))
                            viewplayer->killcount++;

                        stat_monsterskilled_total = SafeAdd(stat_monsterskilled_total, 1);

                        if (M_StringCompare(playername, playername_default))
                            M_snprintf(buffer, sizeof(buffer), "You %s %s.", killed, temp);
                        else
                            M_snprintf(buffer, sizeof(buffer), "%s %s %s.", playername, killed, temp);

                        C_PlayerMessage(buffer);
                        C_HideConsole();
                        HU_SetPlayerMessage(buffer, false, false);
                        viewplayer->cheated++;
                        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
                        M_SaveCVARs();
                        free(temp);
                    }
                }
                else
                {
                    killcmdmobj->flags2 |= MF2_MASSACRE;
                    massacre = true;
                    P_DamageMobj(killcmdmobj, viewplayer->mo, viewplayer->mo, killcmdmobj->health, false, false);
                    massacre = false;

                    if (M_StringCompare(playername, playername_default))
                        M_snprintf(buffer, sizeof(buffer), "You %s %s.", killed, temp);
                    else
                        M_snprintf(buffer, sizeof(buffer), "%s %s %s.", playername, killed, temp);

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    viewplayer->cheated++;
                    stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
                    M_SaveCVARs();
                    free(temp);
                }
            }
            else
            {
                const mobjtype_t    type = P_FindDoomedNum(killcmdtype);

                massacre = true;

                for (int i = 0; i < numsectors; i++)
                    for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
                        if (type == thing->type && !!(thing->flags & MF_FRIEND) == killcmdfriendly)
                        {
                            if (type == MT_PAIN)
                            {
                                if (thing->health > 0)
                                {
                                    A_Fall(thing, NULL, NULL);
                                    P_SetMobjState(thing, S_PAIN_DIE6);
                                    viewplayer->monsterskilled[MT_PAIN]++;
                                    stat_monsterskilled[MT_PAIN] = SafeAdd(stat_monsterskilled[MT_PAIN], 1);

                                    if (!(thing->flags & MF_FRIEND))
                                        viewplayer->killcount++;

                                    stat_monsterskilled_total = SafeAdd(stat_monsterskilled_total, 1);
                                    kills++;
                                }
                            }
                            else if ((thing->flags & MF_SHOOTABLE) && thing->health > 0)
                            {
                                thing->flags2 |= MF2_MASSACRE;
                                massacre = true;
                                P_DamageMobj(thing, viewplayer->mo, viewplayer->mo, thing->health, false, false);
                                massacre = false;
                                kills++;
                            }
                            else if (type == MT_BARREL)
                            {
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
                            M_snprintf(buffer, sizeof(buffer), "You %s the only %s in this map.",
                                killed,
                                mobjinfo[type].name1);
                        else
                            M_snprintf(buffer, sizeof(buffer), "You %s all %s %s in this map.",
                                killed,
                                temp,
                                mobjinfo[type].plural1);
                    }
                    else
                    {
                        if (kills == 1)
                            M_snprintf(buffer, sizeof(buffer), "%s %s the only %s in this map.",
                                playername,
                                killed,
                                mobjinfo[type].name1);
                        else
                            M_snprintf(buffer, sizeof(buffer), "%s %s all %s %s in this map.",
                                playername,
                                killed,
                                temp,
                                mobjinfo[type].plural1);
                    }

                    C_PlayerMessage(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    viewplayer->cheated++;
                    stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
                    M_SaveCVARs();
                    free(temp);
                }
                else
                {
                    if (gamemode != commercial)
                    {
                        if (killcmdtype >= ArchVile && killcmdtype <= MonsterSpawner)
                            C_Warning(0, "There are no %s in " ITALICS("%s") ".",
                                mobjinfo[type].plural1, gamedescription);
                        else if (gamemode == shareware && (killcmdtype == Cyberdemon || killcmdtype == SpiderMastermind))
                            C_Warning(0, "There are no %s in the shareware version of " ITALICS("DOOM") ". "
                                "You can buy the full version on " ITALICS("Steam") ", etc.", mobjinfo[type].plural1);
                        else
                            C_Warning(0, "There are no %s to %s.", mobjinfo[type].plural1, cmd);
                    }
                    else
                        C_Warning(0, "There are no %s to %s.", mobjinfo[type].plural1, cmd);
                }
            }
        }

        free(parm);
    }
}

//
// license CCMD
//
static void license_func2(char *cmd, char *parms)
{
    C_Output("Opening the " ITALICS(DOOMRETRO_LICENSE) "...");

#if defined(_WIN32)
    if (!ShellExecute(NULL, "open", DOOMRETRO_LICENSEURL, NULL, NULL, SW_SHOWNORMAL))
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__HAIKU__)
    if (!system("xdg-open " DOOMRETRO_LICENSEURL))
#elif defined(__APPLE__)
    if (!system("open " DOOMRETRO_LICENSEURL))
#endif
        C_Warning(0, "The " ITALICS(DOOMRETRO_LICENSE) " wouldn't open.");
}

//
// load CCMD
//
static void load_func2(char *cmd, char *parms)
{
    char    buffer[1024];

    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
        return;
    }

    if (strlen(parms) == 1 && parms[0] >= '1' && parms[0] <= '8')
    {
        M_snprintf(buffer, sizeof(buffer), "%s" DOOMRETRO_SAVEGAME, savegamefolder, parms[0] - '1');
        G_LoadGame(buffer);
    }
    else
    {
        M_snprintf(buffer, sizeof(buffer), "%s%s%s",
            (M_StringStartsWith(parms, savegamefolder) ? "" : savegamefolder), parms, (M_StringEndsWith(parms, ".save") ? "" : ".save"));
        G_LoadGame(buffer);
    }
}

//
// map CCMD
//
static bool map_func1(char *cmd, char *parms)
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
                    if (gameepisode != (gamemode == retail ? (sigil ? (sigil2 ? 6 : 5) : 4) : (gamemode == shareware || chex ? 1 : 3)))
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
                if (sigil2)
                {
                    if (!(gameepisode == 6 && gamemap == 8))
                    {
                        mapcmdepisode = 6;
                        mapcmdmap = 8;
                        M_StringCopy(mapcmdlump, "E6M8", sizeof(mapcmdlump));
                        result = true;
                    }
                }
                else if (sigil)
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
                    M_BigRandomIntNoRepeat(1, (gamemode == retail ? (sigil ? (sigil2 ? 6 : 5) : 4) : 3), gameepisode));
                mapcmdmap = M_BigRandomIntNoRepeat(1, (chex ? 5 : 8), gamemap);

                if (mapcmdepisode == 1 && mapcmdmap == 4 && (M_BigRandom() & 1) && gamemode != shareware && !E1M4)
                    M_StringCopy(mapcmdlump, "E1M4B", sizeof(mapcmdlump));
                else if (mapcmdepisode == 1 && mapcmdmap == 8 && (M_BigRandom() & 1) && gamemode != shareware && !E1M8)
                    M_StringCopy(mapcmdlump, "E1M8B", sizeof(mapcmdlump));
                else
                    M_snprintf(mapcmdlump, sizeof(mapcmdlump), "E%iM%i", mapcmdepisode, mapcmdmap);

                result = true;
            }
        }
        else if (M_StringCompare(parm, "E1M4B") || M_StringCompare(parm, "phobosmissioncontrol"))
        {
            mapcmdepisode = 1;
            mapcmdmap = 4;
            M_StringCopy(speciallumpname, "E1M4B", sizeof(speciallumpname));
            M_StringCopy(mapcmdlump, "E1M4B", sizeof(mapcmdlump));
            result = (gamemission == doom && gamemode != shareware && !E1M4);
        }
        else if (M_StringCompare(parm, "E1M8B") || M_StringCompare(parm, "techgonebad"))
        {
            mapcmdepisode = 1;
            mapcmdmap = 8;
            M_StringCopy(speciallumpname, "E1M8B", sizeof(speciallumpname));
            M_StringCopy(mapcmdlump, "E1M8B", sizeof(mapcmdlump));
            result = (gamemission == doom && gamemode != shareware && !E1M8);
        }
        else
        {
            M_StringCopy(mapcmdlump, parm, sizeof(mapcmdlump));

            if (gamemode == commercial)
            {
                mapcmdepisode = 1;

                if (sscanf(parm, "MAP0%1i", &mapcmdmap) == 1 || sscanf(parm, "MAP%2i", &mapcmdmap) == 1)
                {
                    if (!((BTSX && W_GetNumLumps(parm) == 1) || (gamemission == pack_nerve && mapcmdmap > 9)))
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
                            && ((mapcmdepisode == 1 && BTSXE1)
                                || (mapcmdepisode == 2 && BTSXE2)
                                || (mapcmdepisode == 3 && BTSXE3)))
                        {
                            char    lump[6];

                            M_snprintf(lump, sizeof(lump), "MAP%02i", mapcmdmap);
                            result = (W_GetNumLumps(lump) == 2);
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
                replaced = (W_GetNumLumps(mapcmdlump) > 1 && !chex && !FREEDOOM);
                pwad = (lumpinfo[i]->wadfile->type == PWAD);
                M_StringCopy(mapinfoname, P_GetMapName(mapcmdepisode, mapcmdmap), sizeof(mapinfoname));

                if (mapcmdmap > 9)
                    continue;

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
                        if ((!D_IsNERVEWAD(wadname) && ((!replaced || pwad || nerve) && (pwad || !BTSX))) || hacx)
                        {
                            if (BTSX)
                            {
                                if (!D_IsDOOM2IWAD(wadname))
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
                        if (D_IsNERVEWAD(wadname))
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

static void map_func2(char *cmd, char *parms)
{
    char    buffer[1024];

    if (!*parms)
    {
        C_ShowDescription(C_GetIndex(cmd));
        C_Output(BOLD("%s") " %s", cmd, (gamemission == doom ? MAPCMDFORMAT1 : MAPCMDFORMAT2));
        return;
    }

    if ((samelevel = (gameepisode == mapcmdepisode && gamemap == mapcmdmap && !*speciallumpname)))
        M_snprintf(buffer, sizeof(buffer), s_STSTR_CLEVSAME, mapcmdlump);
    else
        M_snprintf(buffer, sizeof(buffer), s_STSTR_CLEV,
            (M_StringCompare(playername, playername_default) ? "you" : playername), mapcmdlump);

    C_Output(buffer);
    HU_SetPlayerMessage(buffer, false, false);

    if (gamemode == commercial)
    {
        if (mapcmdmap >= 31 || (gamemission == pack_nerve && mapcmdmap == 9))
            message_secret = true;
    }
    else
    {
        if (mapcmdmap == 9)
            message_secret = true;
    }

    gameepisode = mapcmdepisode;

    if (gamemission == doom)
    {
        episode = gameepisode;
        EpiDef.laston = episode - 1;
    }

    gamemap = mapcmdmap;
    quicksaveslot = -1;

    if (r_diskicon)
    {
        drawdisk = true;
        drawdisktics = DRAWDISKTICS;
    }

    if (gamestate == GS_LEVEL)
    {
        idclevtics = TICRATE;
        C_HideConsole();
    }
    else
    {
        G_DeferredInitNew(skilllevel - 1, gameepisode, gamemap);
        C_HideConsoleFast();
    }

    viewplayer->cheated++;
    stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
    M_SaveCVARs();
}

//
// maplist CCMD
//
static void removemapnum(char *title)
{
    const char  *pos = strchr(title, ':');

    if (pos)
    {
        int index = (int)(pos - title) + 1;

        memmove(title, title + index, strlen(title) - index + 1);

        if (title[0] == ' ')
            memmove(title, title + 1, strlen(title));
    }
}

static void maplist_func2(char *cmd, char *parms)
{
    const int   tabs[MAXTABS] = { 40, 93, 385 };
    int         count = 0;
    char        (*maps)[256] = I_Malloc(numlumps * sizeof(*maps));

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
            if ((M_StringCompare(lump, "E1M4B") && (gamemode == shareware || E1M4))
                || (M_StringCompare(lump, "E1M8B") && (gamemode == shareware || E1M8)))
                continue;
            else if (sscanf(lump, "E%1iM%i", &ep, &map) != 2)
                continue;
        }

        M_StringCopy(wadname, leafname(lumpinfo[i]->wadfile->path), sizeof(wadname));
        replaced = (W_GetNumLumps(lump) > 1 && !chex && !FREEDOOM);
        pwad = (lumpinfo[i]->wadfile->type == PWAD);
        M_StringCopy(mapinfoname, P_GetMapName(ep--, map--), sizeof(mapinfoname));

        switch (gamemission)
        {
            case doom:
                if (!replaced || pwad)
                {
                    if (replaced && dehcount == 1 && !*mapinfoname)
                        M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t\x96\t%s",
                            lump, wadname);
                    else
                    {
                        if (M_StringCompare(lump, "E1M4B"))
                            temp = titlecase(s_HUSTR_E1M4B);
                        else if (M_StringCompare(lump, "E1M8B"))
                            temp = titlecase(s_HUSTR_E1M8B);
                        else
                            temp = titlecase(*mapinfoname ? mapinfoname : *mapnames[ep * 9 + map]);

                        removemapnum(temp);
                        M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t" ITALICS("%s") "\t%s",
                            lump, temp, wadname);
                        free(temp);
                    }
                }

                break;

            case doom2:
                if ((!D_IsNERVEWAD(wadname) && (!replaced || pwad || nerve)) || hacx || harmony)
                {
                    if (BTSX)
                    {
                        if (!D_IsDOOM2IWAD(wadname))
                        {
                            temp = titlecase(M_StringReplaceFirst(*mapnames2[map], ": ", MONOSPACEDOFF "\t" ITALICSON));
                            removemapnum(temp);
                            M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACEDON "%s" ITALICSOFF "\t%s", temp, wadname);
                            free(temp);
                        }
                    }
                    else
                    {
                        if (replaced && dehcount == 1 && !nerve && !*mapinfoname)
                            M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t\x96\t%s",
                                lump, wadname);
                        else
                        {
                            temp = titlecase(*mapinfoname ? mapinfoname : (bfgedition ? *mapnames2_bfg[map] : *mapnames2[map]));
                            removemapnum(temp);
                            M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t" ITALICS("%s") "\t%s",
                                lump, temp, wadname);
                            free(temp);
                        }
                    }
                }

                break;

            case pack_nerve:
                if (D_IsNERVEWAD(wadname))
                {
                    temp = titlecase(*mapinfoname ? mapinfoname : *mapnamesn[map]);
                    removemapnum(temp);
                    M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t" ITALICS("%s") "\t%s",
                        lump, temp, wadname);
                    free(temp);
                }

                break;

            case pack_plut:
                if (!replaced || pwad)
                {
                    if (replaced && dehcount == 1 && !*mapinfoname)
                        M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t \x96\t%s",
                            lump, wadname);
                    else
                    {
                        temp = titlecase(*mapinfoname ? mapinfoname : *mapnamesp[map]);
                        removemapnum(temp);
                        M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t" ITALICS("%s") "\t%s",
                            lump, temp, wadname);
                        free(temp);
                    }
                }

                break;

            case pack_tnt:
                if (!replaced || pwad)
                {
                    if (replaced && dehcount == 1 && !*mapinfoname)
                        M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t \x96\t%s",
                            lump, wadname);
                    else
                    {
                        temp = titlecase(*mapinfoname ? mapinfoname : *mapnamest[map]);
                        removemapnum(temp);
                        M_snprintf(maps[count++], sizeof(maps[0]), MONOSPACED("%s") "\t" ITALICS("%s") "\t%s",
                            lump, temp, wadname);
                        free(temp);
                    }
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
        C_TabbedOutput(tabs, MONOSPACED("%3i") ".\t%s", i + 1, maps[i]);

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
#define WW      "William D Whitaker"
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

static void OutputReleaseDate(const int tabs[MAXTABS], char *wadname)
{
    if (M_StringCompare(wadname, "DOOM1.WAD"))
        C_TabbedOutput(tabs, INDENT "Release date\tDecember 10, 1993");
    else if (D_IsDOOM1IWAD(wadname))
    {
        if (kex)
            C_TabbedOutput(tabs, INDENT "Release date\tAugust 8, 2024");
        else if (unity)
            C_TabbedOutput(tabs, INDENT "Release date\tJuly 26, 2019");
        else if (bfgedition)
            C_TabbedOutput(tabs, INDENT "Release date\tOctober 16, 2012");
        else if (gamemode == retail)
            C_TabbedOutput(tabs, INDENT "Release date\tApril 30, 1995");
        else
            C_TabbedOutput(tabs, INDENT "Release date\tDecember 10, 1993");
    }
    else if (M_StringCompare(wadname, "SIGIL_v1_0.wad")
        || M_StringCompare(wadname, "SIGIL.wad"))
        C_TabbedOutput(tabs, INDENT "Release date\tMay 1, 2019");
    else if (M_StringCompare(wadname, "SIGIL_v1_1.wad"))
        C_TabbedOutput(tabs, INDENT "Release date\tMay 31, 2019");
    else if (M_StringCompare(wadname, "SIGIL_v1_21.wad")
        || M_StringCompare(wadname, "SIGIL_v1_2.wad"))
        C_TabbedOutput(tabs, INDENT "Release date\tSeptember 10, 2019");
    else if (M_StringCompare(wadname, "SIGIL_II_V1_0.WAD") || M_StringCompare(wadname, "SIGIL_II_MP3_V1_0.WAD"))
        C_TabbedOutput(tabs, INDENT "Release date\tDecember 10, 2023");
    else if (D_IsDOOM2IWAD(wadname))
    {
        if (kex)
            C_TabbedOutput(tabs, INDENT "Release date\tAugust 8, 2024");
        else if (unity)
            C_TabbedOutput(tabs, INDENT "Release date\tJuly 26, 2019");
        else if (bfgedition)
            C_TabbedOutput(tabs, INDENT "Release date\tOctober 16, 2012");
        else
            C_TabbedOutput(tabs, INDENT "Release date\tSeptember 30, 1994");
    }
    else if (M_StringCompare(wadname, "NERVE.WAD"))
        C_TabbedOutput(tabs, INDENT "Release date\tMay 26, 2010");
    else if (M_StringCompare(wadname, "PLUTONIA.WAD") || M_StringCompare(wadname, "TNT.WAD"))
        C_TabbedOutput(tabs, INDENT "Release date\tJune 17, 1996");
    else if (onehumanity)
        C_TabbedOutput(tabs, INDENT "Release date\tMarch 2, 2022");
    else if (REKKRSL)
        C_TabbedOutput(tabs, INDENT "Release date\tOctober 11, 2021");
    else if (REKKR)
        C_TabbedOutput(tabs, INDENT "Release date\tJuly 10, 2018");
    else if (ID1)
        C_TabbedOutput(tabs, INDENT "Release date\tAugust 8, 2024");
}

static void mapstats_func2(char *cmd, char *parms)
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
        /* 59 */ { JR,    "",   "",    "",   "" },
        /* 60 */ { "",    "",   "",    "",   "" },
        /* 61 */ { JR,    "",   "",    "",   "" },
        /* 62 */ { JR,    "",   "",    "",   "" },
        /* 63 */ { JR,    "",   "",    "",   "" },
        /* 64 */ { JR,    "",   "",    "",   "" },
        /* 65 */ { JR,    "",   "",    "",   "" },
        /* 66 */ { JR,    "",   "",    "",   "" },
        /* 67 */ { JR,    "",   "",    "",   "" },
        /* 68 */ { JR,    "",   "",    "",   "" },
        /* 69 */ { JR,    "",   "",    "",   "" }
    };

    struct
    {
        const char *title;
        const char *artist;
    } legacyofrust[] = {
        { "Welcome To Die",                    "Xaser Acheron" },
        { "The Shores Of Heaven",              "Xaser Acheron" },
        { "Bilge Punks",                       "Xaser Acheron" },
        { "Callous Regard",                    "Xaser Acheron" },
        { "They're All Going To Laugh At You", "Xaser Acheron" },
        { "Cloudy With A Chance Of Spiders",   "Xaser Acheron" },
        { "Tactical Blasphemy",                "Xaser Acheron" },
        { "Opening To Hell",                   "Bobby Prince"  },
        { "Bonk",                              "Xaser Acheron" },
        { "Cliff Driver",                      "Xaser Acheron" },
        { "Wizard Science",                    "Xaser Acheron" },
        { "Deja Vuvuzela",                     "Xaser Acheron" },
        { "Disgusto!",                         "Xaser Acheron" },
        { "March Of The Vespers",              "Xaser Acheron" },
        { "The Skeleton World",                "Xaser Acheron" },
        { "Hell Force Five",                   "Xaser Acheron" }
    };

    const int   tabs[MAXTABS] = { 127 };
    char        *temp;
    int         lump;
    int         wadtype;
    const char  *author = P_GetMapAuthor(gameepisode, gamemap);
    char        wadname[MAX_PATH];
    const int   partime = G_GetParTime();
    int         outside = 0;
    int         min_x = INT_MAX;
    int         max_x = INT_MIN;
    int         min_y = INT_MAX;
    int         max_y = INT_MIN;
    int         max_ceilingheight = INT_MIN;
    int         min_floorheight = INT_MAX;
    int         width;
    int         height;
    int         depth;

    if (gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (M_StringCompare(playername, playername_default))
            C_Warning(0, NOGAMECCMDWARNING1);
        else
            C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                (playergender == playergender_other ? "aren't" : "isn't"));

        return;
    }

    if (FREEDOOM1)
    {
        char    lumpname[6];

        M_snprintf(lumpname, sizeof(lumpname), "E%iM%i", gameepisode, gamemap);
        lump = W_CheckNumForName(lumpname);
        wadtype = lumpinfo[lump]->wadfile->type;
    }
    else if (BTSX || KDIKDIZD)
    {
        char    lumpname[6];

        M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", gamemap);
        lump = W_CheckNumForName(lumpname);
        wadtype = lumpinfo[lump]->wadfile->type;
    }
    else
    {
        lump = (nerve && gamemission == doom2 ? W_GetLastNumForName(mapnum) : W_CheckNumForName(mapnum));

        if (D_IsDOOM2IWAD(lumpinfo[lump]->wadfile->path)
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
            C_TabbedOutput(tabs, "Map\t%i of 9", gamemap);
        else
            C_TabbedOutput(tabs, "Map\t%i of %i", gamemap, (bfgedition ? 33 : 32));
    }
    else
        C_TabbedOutput(tabs, "Map\t%i of 9", gamemap);

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
                    C_TabbedOutput(tabs, INDENT "Alternative\t" ITALICS("%s"), s_HUSTR_11_ALT);
            }
            else if (gamemap == 31)
            {
                if (M_StringCompare(maptitle, s_HUSTR_31))
                    C_TabbedOutput(tabs, INDENT "Alternative\t" ITALICS("%s"), s_HUSTR_31_BFG);
            }
            else if (gamemap == 32)
            {
                if (M_StringCompare(maptitle, s_HUSTR_32))
                    C_TabbedOutput(tabs, INDENT "Alternative\t" ITALICS("%s"), s_HUSTR_32_BFG);
            }
        }
        else if (gameepisode == 3 && gamemap == 7)
        {
            if (M_StringCompare(maptitle, s_HUSTR_E3M7))
                C_TabbedOutput(tabs, INDENT "Alternative\t" ITALICS("%s"), s_HUSTR_E3M7_ALT);
        }
    }

    if (*author)
        C_TabbedOutput(tabs, "Author\t%s", author);
    else if (gamemission == doom)
    {
        if (canmodify && *authors[gameepisode * 10 + gamemap][gamemission])
            C_TabbedOutput(tabs, "Author\t%s", authors[gameepisode * 10 + gamemap][gamemission]);
        else if (REKKR)
            C_TabbedOutput(tabs, "Author\tMatthew Little");
    }
    else if (canmodify && *authors[gamemap][gamemission])
        C_TabbedOutput(tabs, "Author\t%s", authors[gamemap][gamemission]);

    if (gamemode == commercial)
    {
        if (gamemission == pack_nerve)
        {
            temp = titlecase(*expansions[1]);
            C_TabbedOutput(tabs, "Expansion\t" ITALICS("%s") " (2 of 2)", temp);
            free(temp);
        }
        else
        {
            if (customepisode)
            {
                if (**episodes[maptoepisode[gamemap] - 1])
                {
                    temp = titlecase(*episodes[maptoepisode[gamemap] - 1]);
                    C_TabbedOutput(tabs, "Episode\t" ITALICS("%s") " (%i of %i)",
                        temp, maptoepisode[gamemap], EpiDef.numitems);
                    free(temp);
                }
                else
                    C_TabbedOutput(tabs, "Episode\t%i of %i",
                        maptoepisode[gamemap], EpiDef.numitems);
            }
            else if (nerve)
            {
                temp = titlecase(*expansions[0]);
                C_TabbedOutput(tabs, "Expansion\t" ITALICS("%s") " (1 of 2)", temp);
                free(temp);
            }
        }
    }
    else
    {
        if (customepisode)
        {
            if (**episodes[maptoepisode[gamemap] - 1])
            {
                temp = titlecase(*episodes[maptoepisode[gamemap] - 1]);
                C_TabbedOutput(tabs, "Episode\t" ITALICS("%s") " (%i of %i)",
                    temp, maptoepisode[gamemap], EpiDef.numitems);
                free(temp);
            }
            else
                C_TabbedOutput(tabs, "Episode\t%i of %i",
                    maptoepisode[gamemap], EpiDef.numitems);
        }
        else if (!chex && !hacx)
        {
            temp = titlecase(*episodes[gameepisode - 1]);
            C_TabbedOutput(tabs, "Episode\t" ITALICS("%s") " (%i of %i)",
                temp, gameepisode,
                (gamemode == retail ? (sigil ? (sigil2 ? 6 : 5) : 4) : (gamemode == shareware ? 1 : 3)));
            free(temp);
        }
    }

    if (secretmap)
        C_TabbedOutput(tabs, "Secret\tYes");

    M_StringCopy(wadname, leafname(lumpinfo[lump]->wadfile->path), sizeof(wadname));

    C_TabbedOutput(tabs, "%s\t%s", (wadtype == IWAD ? "IWAD" : "PWAD"), wadname);

    OutputReleaseDate(tabs, wadname);

    if (wadtype == PWAD)
    {
        if (W_GetNumLumps("DEHACKED") > 1)
            C_TabbedOutput(tabs, INDENT "DEHACKED\tYes");

        if (*mapinfolump)
            C_TabbedOutput(tabs, INDENT "%s\tYes", mapinfolump);
    }

    C_TabbedOutput(tabs, INDENT "MD5\t%s", MD5(lumpinfo[lump]->wadfile->path));

    if (wadtype == PWAD)
    {
        M_StringCopy(wadname, leafname(lumpinfo[W_GetLastNumForName("PLAYPAL")]->wadfile->path), sizeof(wadname));

        C_TabbedOutput(tabs, "IWAD\t%s", wadname);

        OutputReleaseDate(tabs, wadname);

        C_TabbedOutput(tabs, INDENT "MD5\t%s", MD5(lumpinfo[W_GetLastNumForName("PLAYPAL")]->wadfile->path));
    }

    C_TabbedOutput(tabs, "Compatibility\t%s",
        (mbf21compatible ? ITALICS("MBF21") :
            (mbfcompatible ? ITALICS("MBF") :
                (boomcompatible ? ITALICS("BOOM") :
                    (numsegs < 32768 ? "Vanilla" : "Limit removing")))));

    if (partime)
        C_TabbedOutput(tabs, "Par time\t" MONOSPACED("%02i") ":" MONOSPACED("%02i"),
            partime / 60, partime % 60);

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

    if (player1starts > 1)
    {
        temp = commify(player1starts);
        C_TabbedOutput(tabs, INDENT "Voodoo dolls\t%s", temp);
        free(temp);
    }

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

    C_TabbedOutput(tabs, INDENT "Format\t%s", nodeformats[nodeformat]);

    if (nodeformat == ZDBSPX)
        C_TabbedOutput(tabs, INDENT "Compressed\tNo");
    else if (nodeformat == ZDBSPZ)
        C_TabbedOutput(tabs, INDENT "Compressed\tYes");

    temp = commify(numsectors);
    C_TabbedOutput(tabs, "Sectors\t%s", temp);
    free(temp);

    for (int i = 0; i < numsubsectors; i++)
    {
        const short picnum = subsectors[i].sector->ceilingpic;

        if (picnum == skyflatnum || (picnum & PL_SKYFLAT))
            outside++;
    }

    outside = outside * 100 / numsubsectors;
    C_TabbedOutput(tabs, INDENT "Inside/outside\t%i%%/%i%%", 100 - outside, outside);

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

    if (mus_playing && !nomusic)
    {
        int                 lumps;
        char                namebuf[9];
        const char          *musicartist = P_GetMapMusicComposer(gameepisode, gamemap);
        const char          *musictitle = P_GetMapMusicTitle(gameepisode, gamemap);
        const Mix_MusicType musictype = Mix_GetMusicType(NULL);

        temp = uppercase(mus_playing->name1);

        if (temp[0] == 'D' && temp[1] == '_')
            M_StringCopy(namebuf, temp, sizeof(namebuf));
        else
        {
            M_snprintf(namebuf, sizeof(namebuf), "H_%s", temp);

            if (W_CheckNumForName(namebuf) == -1)
                M_snprintf(namebuf, sizeof(namebuf), "D_%s", temp);
        }

        C_TabbedOutput(tabs, "Music lump\t%s", namebuf);
        free(temp);

        lumps = W_GetNumLumps(namebuf);

        if (*musictitle)
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), musictitle);
        else if (sigil && gameepisode == 5)
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), (buckethead ? mus_playing->title2 : mus_playing->title1));
        else if (sigil2 && gameepisode == 6)
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), (thorr ? mus_playing->title2 : mus_playing->title1));
        else if (ID1 && gamemap <= 15)
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), legacyofrust[gamemap - 1].title);
        else if (namebuf[0] == 'H' && namebuf[1] == '_')
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), mus_playing->title1);
        else if (!M_StringCompare(mus_playing->title1, "n/a")
            && (((gamemode == commercial || gameepisode > 1) && lumps == 1 && wadtype == IWAD && gamemission != pack_tnt)
                || (gamemode != commercial && gameepisode == 1 && lumps == 2)
                || gamemode == shareware
                || gamemission == pack_nerve))
            C_TabbedOutput(tabs, INDENT "Title\t" ITALICS("%s"), mus_playing->title1);

        if (*musicartist)
            C_TabbedOutput(tabs, INDENT "Artist\t%s", musicartist);
        else if (sigil && gameepisode == 5)
            C_TabbedOutput(tabs, INDENT "Artist\t%s", (buckethead ? "Buckethead" : "James Paddock"));
        else if (sigil2 && gameepisode == 6)
            C_TabbedOutput(tabs, INDENT "Artist\t%s", (thorr ? "Thorr" : "James Paddock"));
        else if (ID1 && gamemap <= 15)
            C_TabbedOutput(tabs, INDENT "Artist\t%s", legacyofrust[gamemap - 1].artist);
        else if (namebuf[0] == 'H' && namebuf[1] == '_')
            C_TabbedOutput(tabs, INDENT "Artist\tAndrew Hulshult");
        else if (((gamemode == commercial || gameepisode > 1) && lumps == 1 && wadtype == IWAD && gamemission != pack_tnt)
            || (gamemode != commercial && gameepisode == 1 && lumps == 2)
            || gamemode == shareware
            || gamemission == pack_nerve)
            C_TabbedOutput(tabs, INDENT "Artist\tBobby Prince");

        if (musmusictype)
            C_TabbedOutput(tabs, INDENT "Format\tMUS");
        else if (musictype == MUS_WAV)
            C_TabbedOutput(tabs, INDENT "Format\tWAV");
        else if (musictype == MUS_MOD)
            C_TabbedOutput(tabs, INDENT "Format\tMOD");
        else if (midimusictype || musictype == MUS_MID)
            C_TabbedOutput(tabs, INDENT "Format\tMIDI");
        else if (musictype == MUS_OGG)
            C_TabbedOutput(tabs, INDENT "Format\tOgg Vorbis");
        else if (musictype == MUS_MP3)
            C_TabbedOutput(tabs, INDENT "Format\tMP3");
        else if (musictype == MUS_FLAC)
            C_TabbedOutput(tabs, INDENT "Format\tFLAC");
        else if (musictype == MUS_OPUS)
            C_TabbedOutput(tabs, INDENT "Format\tOpus");

        if (lumpinfo[mus_playing->lumpnum]->wadfile->type == PWAD)
        {
            C_TabbedOutput(tabs, INDENT "PWAD\t%s", leafname(lumpinfo[mus_playing->lumpnum]->wadfile->path));

            if (namebuf[0] == 'H' && namebuf[1] == '_')
                C_TabbedOutput(tabs, INDENT INDENT "Release date\tAugust 8, 2024");

            if (!M_StringCompare(lumpinfo[lump]->wadfile->path, lumpinfo[mus_playing->lumpnum]->wadfile->path))
                C_TabbedOutput(tabs, INDENT INDENT "MD5\t%s", MD5(lumpinfo[mus_playing->lumpnum]->wadfile->path));
        }
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

static bool name_func1(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
        return true;

    if (M_StringStartsWith(parm, "player"))
    {
        M_StringCopy(namecmdold, "player", sizeof(namecmdold));
        M_StringReplaceAll(parm, "player", "", false);
        M_StringCopy(namecmdnew, parm, sizeof(namecmdnew));

        return (namecmdnew[0] != '\0' && strlen(namecmdnew) < 33);
    }

    if (gamestate == GS_LEVEL)
    {
        if ((namecmdfriendly = M_StringStartsWith(parm, "friendly")))
            M_StringReplaceAll(parm, "friendly", "", false);
        else if (M_StringStartsWith(parm, "unfriendly"))
            M_StringReplaceAll(parm, "unfriendly", "", false);

        if (M_StringStartsWith(parm, "monster"))
        {
            M_StringCopy(namecmdold, "monster", sizeof(namecmdold));
            M_StringReplaceAll(parm, "monster", "", false);
            M_StringCopy(namecmdnew, parm, sizeof(namecmdnew));
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

static void name_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMECCMDWARNING1);
            else
                C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                    (playergender == playergender_other ? "aren't" : "isn't"));
        }
    }
    else if (M_StringCompare(namecmdold, "player")
        || M_StringCompare(namecmdold, playername)
        || M_StringCompare(namecmdold, playername_default))
    {
        if (M_StringCompare(playername, playername_default))
            C_PlayerMessage("You have been named " BOLD("%s") ".", namecmdnew);
        else if (*namecmdnew)
            C_PlayerMessage("%s has been renamed " BOLD("%s") ".", playername, namecmdnew);
        else
            C_PlayerMessage("You longer have a name.");

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

            C_PlayerMessage("The nearest %s%s to %s has been %s " BOLD("%s") ".",
                (namecmdfriendly ? "friendly " : ""), namecmdold, playername,
                (*bestmobj->name ? "renamed" : "named"), namecmdnew);

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
static void newgame_func2(char *cmd, char *parms)
{
    C_HideConsoleFast();

    if (viewplayer)
        viewplayer->cheats = 0;

    G_DeferredInitNew((skill_t)(skilllevel - 1), (gamemode == commercial ? expansion : episode), 1);
}

//
// noclip CCMD
//
static void noclip_func2(char *cmd, char *parms)
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
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_NCOFF);
        HU_SetPlayerMessage(s_STSTR_NCOFF, false, false);
    }
}

//
// nomonsters CCMD
//
static void nomonsters_func2(char *cmd, char *parms)
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

                    if (((thing->flags & MF_SHOOTABLE) || (thing->flags2 & MF2_MONSTERMISSILE))
                        && type != MT_PLAYER && type != MT_BARREL && type != MT_BOSSBRAIN)
                        P_RemoveMobj(thing);

                    thing = thing->snext;
                }
            }

        C_Output(s_STSTR_NMON);
        HU_SetPlayerMessage(s_STSTR_NMON, false, false);
        viewplayer->cheated++;
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_NMOFF);
        HU_SetPlayerMessage(s_STSTR_NMOFF, false, false);

        if (gamestate == GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NEXTMAPWARNING1);
            else
                C_Warning(0, NEXTMAPWARNING2, playername);
        }
    }
}

//
// notarget CCMD
//
static void notarget_func2(char *cmd, char *parms)
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
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_NTOFF);
        HU_SetPlayerMessage(s_STSTR_NTOFF, false, false);
    }
}

//
// pistolstart CCMD
//
static void pistolstart_func2(char *cmd, char *parms)
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

    if (gamestate == GS_LEVEL)
    {
        if (M_StringCompare(playername, playername_default))
            C_Warning(0, NEXTMAPWARNING1);
        else
            C_Warning(0, NEXTMAPWARNING2, playername);
    }
}

//
// play CCMD
//
static int  playcmdid;
static int  playcmdtype;
static char *playcmdname;

static bool play_func1(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
        return true;

    for (int i = 1; i < NUMSFX; i++)
    {
        char    namebuf[9];

        M_snprintf(namebuf, sizeof(namebuf), "ds%s", s_sfx[i].name2);

        if (M_StringCompare(parm, namebuf) && W_CheckNumForName(namebuf) >= 0)
        {
            playcmdid = i;
            playcmdtype = 1;
            playcmdname = uppercase(namebuf);
            return true;
        }
    }

    for (int i = 1; i < NUMMUSIC; i++)
    {
        char    namebuf[9];

        M_snprintf(namebuf, sizeof(namebuf), "h_%s", s_music[i].name2);

        if (W_CheckNumForName(namebuf) == -1)
            M_snprintf(namebuf, sizeof(namebuf), "d_%s", s_music[i].name2);

        if (M_StringCompare(parm, namebuf) && W_CheckNumForName(namebuf) >= 0)
        {
            playcmdid = i;
            playcmdtype = 2;
            playcmdname = uppercase(namebuf);
            return true;
        }

        if (!M_StringCompare(s_music[i].title1, "n/a"))
        {
            char    *titlebuf = removenonalpha(s_music[i].title1);

            if (M_StringCompare(parm, titlebuf))
            {
                M_snprintf(namebuf, sizeof(namebuf), "h_%s", s_music[i].name2);

                if (W_CheckNumForName(namebuf) == -1)
                    M_snprintf(namebuf, sizeof(namebuf), "d_%s", s_music[i].name2);

                if (W_CheckNumForName(namebuf) >= 0)
                {
                    playcmdid = i;
                    playcmdtype = 3;
                    playcmdname = M_StringDuplicate(s_music[i].title1);
                    free(titlebuf);
                    return true;
                }
            }

            free(titlebuf);

            if (!M_StringCompare(s_music[i].title1, s_music[i].title2))
            {
                titlebuf = removenonalpha(s_music[i].title2);

                if (M_StringCompare(parm, titlebuf))
                {
                    M_snprintf(namebuf, sizeof(namebuf), "h_%s", s_music[i].name2);

                    if (W_CheckNumForName(namebuf) == -1)
                        M_snprintf(namebuf, sizeof(namebuf), "d_%s", s_music[i].name2);

                    if (W_CheckNumForName(namebuf) >= 0)
                    {
                        playcmdid = i;
                        playcmdtype = 3;
                        playcmdname = M_StringDuplicate(s_music[i].title2);
                        free(titlebuf);
                        return true;
                    }
                }

                free(titlebuf);

                if (*s_music[i].title3)
                {
                    titlebuf = removenonalpha(s_music[i].title3);

                    if (M_StringCompare(parm, titlebuf))
                    {
                        M_snprintf(namebuf, sizeof(namebuf), "h_%s", s_music[i].name2);

                        if (W_CheckNumForName(namebuf) == -1)
                            M_snprintf(namebuf, sizeof(namebuf), "d_%s", s_music[i].name2);

                        if (W_CheckNumForName(namebuf) >= 0)
                        {
                            playcmdid = i;
                            playcmdtype = 3;
                            playcmdname = M_StringDuplicate(s_music[i].title3);
                            free(titlebuf);
                            return true;
                        }
                    }

                    free(titlebuf);
                }
            }
        }
    }

    return false;
}

static void play_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
    }
    else
    {
        if (playcmdtype == 1)
        {
            S_StartSound(NULL, playcmdid);
            C_Output("Playing " BOLD("%s") "...", playcmdname);
        }
        else if (playcmdtype == 2)
        {
            S_ChangeMusic(playcmdid, true, true, false);

            if (consoleactive)
                S_LowerMusicVolume();

            C_Output("Playing " BOLD("%s") "...", playcmdname);
        }
        else if (playcmdtype == 3)
        {
            S_ChangeMusic(playcmdid, true, true, false);

            if (consoleactive)
                S_LowerMusicVolume();

            C_Output("Playing " ITALICS("%s") "...", playcmdname);
        }

        free(playcmdname);
    }
}

static skill_t favoriteskilllevel(void)
{
    uint64_t    skilllevelstat = 0;
    skill_t     favorite = gameskill;

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

char *C_DistanceTraveled(uint64_t value, bool allowzero)
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
static void ShowMonsterKillStat_Game(const int tabs[MAXTABS], const mobjtype_t type)
{
    char    *temp1 = sentencecase(mobjinfo[type].plural1);
    char    *temp2 = commify(viewplayer->monsterskilled[type]);
    char    *temp3 = commify(monstercount[type]);
    char    *temp4 = commifystat(stat_monsterskilled[type]);

    C_TabbedOutput(tabs, INDENT "%s\t%s of %s (%i%%)\t%s",
        temp1, temp2, temp3,
        (monstercount[type] ? viewplayer->monsterskilled[type] * 100 / monstercount[type] : 0), temp4);

    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
}

static void C_PlayerStats_Game(void)
{
    const int       tabs[MAXTABS] = { 200, 335 };
    skill_t         favoriteskilllevel1 = favoriteskilllevel();
    weapontype_t    favoriteweapon1 = favoriteweapon(false);
    weapontype_t    favoriteweapon2 = favoriteweapon(true);
    int             time1 = maptime / TICRATE;
    int             time2 = (int)(stat_timeplayed / TICRATE);
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
        C_TabbedOutput(tabs, "Map explored\t%i%%\t\x96", nummappedlines * 100 / numvisiblelines);

    temp1 = commifystat(stat_mapsfinished);
    temp2 = commifystat(stat_mapsstarted);
    C_TabbedOutput(tabs, "Maps finished\t\x96\t%s of %s (%i%%)",
        temp1, temp2, stat_mapsfinished * 100 / stat_mapsstarted);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->gamessaved);
    temp2 = commifystat(stat_gamessaved);
    C_TabbedOutput(tabs, "Games saved\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->gamesloaded);
    temp2 = commifystat(stat_gamesloaded);
    C_TabbedOutput(tabs, "Games loaded\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    if (favoriteskilllevel1 == sk_none)
    {
        temp1 = titlecase(*skilllevels[gameskill]);

        if (temp1[strlen(temp1) - 1] == '.')
            temp1[strlen(temp1) - 1] = '\0';

        C_TabbedOutput(tabs, "%s skill level\t" ITALICS("%s") "\t" ITALICS("%s"),
            (english == english_american ? "Favorite" : "Favourite"), temp1, temp1);
    }
    else
    {
        temp1 = titlecase(*skilllevels[gameskill]);
        temp2 = titlecase(*skilllevels[favoriteskilllevel1]);

        if (temp1[strlen(temp1) - 1] == '.')
            temp1[strlen(temp1) - 1] = '\0';

        if (temp2[strlen(temp2) - 1] == '.')
            temp2[strlen(temp2) - 1] = '\0';

        C_TabbedOutput(tabs, "%s skill level\t" ITALICS("%s") "\t" ITALICS("%s"),
            (english == english_american ? "Favorite" : "Favourite"), temp1, temp2);
        free(temp2);
    }

    free(temp1);

    for (int i = 0; i < NUMMOBJTYPES; i++)
        killcount += viewplayer->monsterskilled[i];

    temp1 = commify(killcount);
    temp2 = commify(totalkills);
    temp3 = commifystat(stat_monsterskilled_total);
    C_TabbedOutput(tabs, "Monsters %s %s\t%s of %s (%i%%)\t%s",
        (M_StringCompare(playername, playername_default) ? "you" : playername), s_KILLED,
        temp1, temp2, (totalkills ? killcount * 100 / totalkills : 0), temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    ShowMonsterKillStat_Game(tabs, MT_POSSESSED);
    ShowMonsterKillStat_Game(tabs, MT_SHOTGUY);
    ShowMonsterKillStat_Game(tabs, MT_TROOP);
    ShowMonsterKillStat_Game(tabs, MT_SERGEANT);
    ShowMonsterKillStat_Game(tabs, MT_SHADOWS);

    if (gamemode == commercial)
        ShowMonsterKillStat_Game(tabs, MT_CHAINGUY);

    ShowMonsterKillStat_Game(tabs, MT_HEAD);
    ShowMonsterKillStat_Game(tabs, MT_SKULL);

    if (gamemode == commercial)
        ShowMonsterKillStat_Game(tabs, MT_KNIGHT);

    ShowMonsterKillStat_Game(tabs, MT_BRUISER);

    if (gamemode == commercial)
    {
        ShowMonsterKillStat_Game(tabs, MT_UNDEAD);
        ShowMonsterKillStat_Game(tabs, MT_BABY);
        ShowMonsterKillStat_Game(tabs, MT_FATSO);
        ShowMonsterKillStat_Game(tabs, MT_PAIN);
        ShowMonsterKillStat_Game(tabs, MT_VILE);
    }

    if (gamemode != shareware)
    {
        ShowMonsterKillStat_Game(tabs, MT_CYBORG);
        ShowMonsterKillStat_Game(tabs, MT_SPIDER);
    }

    temp1 = commify(viewplayer->infightcount);
    temp2 = commifystat(stat_monsterskilled_infighting);
    C_TabbedOutput(tabs, "Monsters %s while infighting\t%s\t%s", s_KILLED, temp1, temp2);
    free(temp1);
    free(temp2);

    if (!M_StringCompare(s_KILLED, s_GIBBED))
    {
        temp1 = commify(viewplayer->monstersgibbed);
        temp2 = commifystat(stat_monstersgibbed);
        C_TabbedOutput(tabs, "Monsters %s\t%s\t%s", s_GIBBED, temp1, temp2);
        free(temp1);
        free(temp2);
    }

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
    temp2 = commify(viewplayer->monsterskilled[MT_BARREL]);
    temp3 = commify(barrelcount);
    temp4 = commifystat(stat_barrelsexploded);
    C_TabbedOutput(tabs, "%s exploded\t%s of %s (%i%%)\t%s",
        temp1, temp2, temp3,
        (barrelcount ? viewplayer->monsterskilled[MT_BARREL] * 100 / barrelcount : 0), temp4);
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

    temp1 = commify(viewplayer->itemspickedup_ammo_bullets + viewplayer->itemspickedup_ammo_shells
        + viewplayer->itemspickedup_ammo_rockets + viewplayer->itemspickedup_ammo_cells);
    temp2 = commify(stat_itemspickedup_ammo_bullets + stat_itemspickedup_ammo_shells
        + stat_itemspickedup_ammo_rockets + stat_itemspickedup_ammo_cells);
    C_TabbedOutput(tabs, INDENT "Ammo\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(weaponinfo[wp_pistol].ammoplural);
    temp2 = commify(viewplayer->itemspickedup_ammo_bullets);
    temp3 = commifystat(stat_itemspickedup_ammo_bullets);
    C_TabbedOutput(tabs, INDENT INDENT "%s\t%s\t%s", temp1, temp2, temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = sentencecase(weaponinfo[wp_shotgun].ammoplural);
    temp2 = commify(viewplayer->itemspickedup_ammo_shells);
    temp3 = commifystat(stat_itemspickedup_ammo_shells);
    C_TabbedOutput(tabs, INDENT INDENT "%s\t%s\t%s", temp1, temp2, temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    temp1 = sentencecase(weaponinfo[wp_missile].ammoplural);
    temp2 = commify(viewplayer->itemspickedup_ammo_rockets);
    temp3 = commifystat(stat_itemspickedup_ammo_rockets);
    C_TabbedOutput(tabs, INDENT INDENT "%s\t%s\t%s", temp1, temp2, temp3);
    free(temp1);
    free(temp2);
    free(temp3);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(weaponinfo[wp_plasma].ammoplural);
        temp2 = commify(viewplayer->itemspickedup_ammo_cells);
        temp3 = commifystat(stat_itemspickedup_ammo_cells);
        C_TabbedOutput(tabs, INDENT INDENT "%s\t%s\t%s", temp1, temp2, temp3);
        free(temp1);
        free(temp2);
        free(temp3);
    }

    temp1 = commify(viewplayer->itemspickedup_armor);
    temp2 = commifystat(stat_itemspickedup_armor);
    C_TabbedOutput(tabs, INDENT "%s\t%s\t%s",
        (english == english_american ? "Armor" : "Armour"), temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_health);
    temp2 = commifystat(stat_itemspickedup_health);
    C_TabbedOutput(tabs, INDENT "Health\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_keys);
    temp2 = commifystat(stat_itemspickedup_keys);
    C_TabbedOutput(tabs, INDENT "Keycards and skull keys\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = commify(viewplayer->itemspickedup_powerups);
    temp2 = commifystat(stat_itemspickedup_powerups);
    C_TabbedOutput(tabs, INDENT "Power-ups\t%s\t%s", temp1, temp2);
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
    time1 %= 3600;
    time2 %= 3600;
    temp2 = commify(hours2);

    if (sucktime && hours1 >= sucktime)
    {
        if (hours2 >= 100)
            C_TabbedOutput(tabs, "Time played\t%s\tOver %s hours!", s_STSTR_SUCKS, temp2);
        else if (hours2)
            C_TabbedOutput(tabs, "Time played\t%s\t" MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i"),
                s_STSTR_SUCKS, hours2, time2 / 60, time2 % 60);
        else
            C_TabbedOutput(tabs, "Time played\t%s\t" MONOSPACED("%02i") ":" MONOSPACED("%02i"),
                s_STSTR_SUCKS, time2 / 60, time2 % 60);
    }
    else if (hours1)
    {
        if (hours2 >= 100)
            C_TabbedOutput(tabs, "Time played\t" MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i")
                "\tOver %s hours!",
                hours1, time1 / 60, time1 % 60, temp2);
        else if (hours2)
            C_TabbedOutput(tabs, "Time played\t" MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i")
                "\t" MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i") "",
                hours1, time1 / 60, time1 % 60, hours2, time2 / 60, time2 % 60);
        else
            C_TabbedOutput(tabs, "Time played\t" MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i")
                "\t" MONOSPACED("%02i") ":" MONOSPACED("%02i"),
                hours1, time1 / 60, time1 % 60, time2 / 60, time2 % 60);
    }
    else
    {
        if (hours2 >= 100)
            C_TabbedOutput(tabs, "Time played\t" MONOSPACED("%02i") ":" MONOSPACED("%02i")
                "\tOver %s hours!",
                time1 / 60, time1 % 60, temp2);
        else if (hours2)
            C_TabbedOutput(tabs, "Time played\t" MONOSPACED("%02i") ":" MONOSPACED("%02i")
                "\t" MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i"),
                time1 / 60, time1 % 60, hours2, time2 / 60, time2 % 60);
        else
            C_TabbedOutput(tabs, "Time played\t" MONOSPACED("%02i") ":" MONOSPACED("%02i")
                "\t" MONOSPACED("%02i") ":" MONOSPACED("%02i"),
                time1 / 60, time1 % 60, time2 / 60, time2 % 60);
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
    temp2 = commifystat(stat_cheatsentered);
    C_TabbedOutput(tabs, "Cheats entered\t%s\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    for (int i = 0; i < NUMWEAPONS; i++)
        shotsfired1 += viewplayer->shotsfired[i];

    for (int i = 0; i < NUMWEAPONS; i++)
        shotssuccessful1 += viewplayer->shotssuccessful[i];

    temp1 = commify(shotssuccessful1);
    temp2 = commify(shotsfired1);

    shotssuccessful2 = stat_shotssuccessful_fists
        + stat_shotssuccessful_chainsaw
        + stat_shotssuccessful_pistol
        + stat_shotssuccessful_shotgun
        + stat_shotssuccessful_supershotgun
        + stat_shotssuccessful_chaingun
        + stat_shotssuccessful_rocketlauncher
        + stat_shotssuccessful_plasmarifle
        + stat_shotssuccessful_bfg9000;
    temp3 = commify(shotssuccessful2);

    shotsfired2 = stat_shotsfired_fists
        + stat_shotsfired_chainsaw
        + stat_shotsfired_pistol
        + stat_shotsfired_shotgun
        + stat_shotsfired_supershotgun
        + stat_shotsfired_chaingun
        + stat_shotsfired_rocketlauncher
        + stat_shotsfired_plasmarifle
        + stat_shotsfired_bfg9000;
    temp4 = commify(shotsfired2);

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

    temp1 = C_DistanceTraveled(viewplayer->distancetraveled, true);
    temp2 = C_DistanceTraveled(stat_distancetraveled, true);
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

static void ShowMonsterKillStat_NoGame(const int tabs[MAXTABS], const mobjtype_t type)
{
    char    *temp1 = sentencecase(mobjinfo[type].plural1);
    char    *temp2 = commifystat(stat_monsterskilled[type]);

    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s", temp1, temp2);
    free(temp1);
    free(temp2);
}

static void C_PlayerStats_NoGame(void)
{
    const int       tabs[MAXTABS] = { 200, 335 };
    skill_t         favoriteskilllevel1 = favoriteskilllevel();
    weapontype_t    favoriteweapon1 = favoriteweapon(true);
    int             time1 = (int)(stat_timeplayed / TICRATE);
    int             hours1 = time1 / 3600;
    char            *temp1;
    char            *temp2;
    char            *temp3;
    uint64_t        shotsfired = 0;
    uint64_t        shotssuccessful = 0;

    C_Header(tabs, playerstats, PLAYERSTATSHEADER);

    temp1 = commifystat(stat_mapsfinished);
    temp2 = commifystat(stat_mapsstarted);
    C_TabbedOutput(tabs, "Maps finished\t\x96\t%s of %s (%i%%)",
        temp1, temp2, stat_mapsfinished * 100 / stat_mapsstarted);
    free(temp1);
    free(temp2);

    temp1 = commifystat(stat_gamessaved);
    C_TabbedOutput(tabs, "Games saved\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_gamesloaded);
    C_TabbedOutput(tabs, "Games loaded\t\x96\t%s", temp1);
    free(temp1);

    if (favoriteskilllevel1 == sk_none)
        C_TabbedOutput(tabs, "%s skill level\t\x96\t\x96");
    else
    {
        temp1 = titlecase(*skilllevels[favoriteskilllevel1]);

        if (temp1[strlen(temp1) - 1] == '.')
            temp1[strlen(temp1) - 1] = '\0';

        C_TabbedOutput(tabs, "%s skill level\t\x96\t" ITALICS("%s"),
            (english == english_american ? "Favorite" : "Favourite"), temp1);
        free(temp1);
    }

    temp1 = commifystat(stat_monsterskilled_total);
    C_TabbedOutput(tabs, "Monsters %s %s\t\x96\t%s",
        (M_StringCompare(playername, playername_default) ? "you" : playername), s_KILLED, temp1);
    free(temp1);

    ShowMonsterKillStat_NoGame(tabs, MT_POSSESSED);
    ShowMonsterKillStat_NoGame(tabs, MT_SHOTGUY);
    ShowMonsterKillStat_NoGame(tabs, MT_TROOP);
    ShowMonsterKillStat_NoGame(tabs, MT_SERGEANT);
    ShowMonsterKillStat_NoGame(tabs, MT_SHADOWS);

    if (gamemode == commercial)
        ShowMonsterKillStat_NoGame(tabs, MT_CHAINGUY);

    ShowMonsterKillStat_NoGame(tabs, MT_HEAD);
    ShowMonsterKillStat_NoGame(tabs, MT_SKULL);

    if (gamemode == commercial)
        ShowMonsterKillStat_NoGame(tabs, MT_KNIGHT);

    ShowMonsterKillStat_NoGame(tabs, MT_BRUISER);

    if (gamemode == commercial)
    {
        ShowMonsterKillStat_NoGame(tabs, MT_UNDEAD);
        ShowMonsterKillStat_NoGame(tabs, MT_BABY);
        ShowMonsterKillStat_NoGame(tabs, MT_FATSO);
        ShowMonsterKillStat_NoGame(tabs, MT_PAIN);
        ShowMonsterKillStat_NoGame(tabs, MT_VILE);
    }

    if (gamemode != shareware)
    {
        ShowMonsterKillStat_NoGame(tabs, MT_CYBORG);
        ShowMonsterKillStat_NoGame(tabs, MT_SPIDER);
    }

    temp1 = commifystat(stat_monsterskilled_infighting);
    C_TabbedOutput(tabs, "Monsters %s while infighting\t\x96\t%s", s_KILLED, temp1);
    free(temp1);

    if (!M_StringCompare(s_KILLED, s_GIBBED))
    {
        temp1 = commify(viewplayer->monstersgibbed);
        C_TabbedOutput(tabs, "Monsters %s\t\x96\t%s", s_GIBBED, temp1);
        free(temp1);
    }

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

    temp1 = commify(stat_itemspickedup_ammo_bullets + stat_itemspickedup_ammo_shells
        + stat_itemspickedup_ammo_rockets + stat_itemspickedup_ammo_cells);
    C_TabbedOutput(tabs, INDENT "Ammo\t\x96\t%s", temp1);
    free(temp1);

    temp1 = sentencecase(weaponinfo[wp_pistol].ammoplural);
    temp2 = commifystat(stat_itemspickedup_ammo_bullets);
    C_TabbedOutput(tabs, INDENT INDENT "%s\t\x96\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(weaponinfo[wp_shotgun].ammoplural);
    temp2 = commifystat(stat_itemspickedup_ammo_shells);
    C_TabbedOutput(tabs, INDENT INDENT "%s\t\x96\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    temp1 = sentencecase(weaponinfo[wp_missile].ammoplural);
    temp2 = commifystat(stat_itemspickedup_ammo_rockets);
    C_TabbedOutput(tabs, INDENT INDENT "%s\t\x96\t%s", temp1, temp2);
    free(temp1);
    free(temp2);

    if (gamemode != shareware)
    {
        temp1 = sentencecase(weaponinfo[wp_plasma].ammoplural);
        temp2 = commifystat(stat_itemspickedup_ammo_cells);
        C_TabbedOutput(tabs, INDENT INDENT "%s\t\x96\t%s", temp1, temp2);
        free(temp1);
        free(temp2);
    }

    temp1 = commifystat(stat_itemspickedup_armor);
    C_TabbedOutput(tabs, INDENT "%s\t\x96\t%s",
        (english == english_american ? "Armor" : "Armour"), temp1);
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_health);
    C_TabbedOutput(tabs, INDENT "Health\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_keys);
    C_TabbedOutput(tabs, INDENT "Keycards and skull keys\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_itemspickedup_powerups);
    C_TabbedOutput(tabs, INDENT "Power-ups\t\x96\t%s", temp1);
    free(temp1);

    temp1 = commifystat(stat_secretsfound);
    C_TabbedOutput(tabs, "Secrets found\t\x96\t%s", temp1);
    free(temp1);

    if (hours1 >= 100)
    {
        temp1 = commify(hours1);
        C_TabbedOutput(tabs, "Time played\t\x96\tOver %s hours!", temp1);
        free(temp1);
    }
    else
    {
        time1 %= 3600;

        if (hours1)
            C_TabbedOutput(tabs, "Time played\t\x96\t" MONOSPACED("%i") ":" MONOSPACED("%02i") ":" MONOSPACED("%02i"),
                hours1, time1 / 60, time1 % 60);
        else
            C_TabbedOutput(tabs, "Time played\t\x96\t" MONOSPACED("%02i") ":" MONOSPACED("%02i"),
                time1 / 60, time1 % 60);
    }

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

    temp1 = commifystat(stat_cheatsentered);
    C_TabbedOutput(tabs, "Cheats entered\t\x96\t%s", temp1);
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

    temp1 = C_DistanceTraveled(stat_distancetraveled, true);
    C_TabbedOutput(tabs, "Distance %s\t\x96\t%s",
        (english == english_american ? "traveled" : "travelled"), temp1);
    free(temp1);

    temp1 = commify(stat_automapopened);
    C_TabbedOutput(tabs, "Automap opened\t\x96\t%s", temp1);
    free(temp1);
}

static void playerstats_func2(char *cmd, char *parms)
{
    if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
        C_PlayerStats_Game();
    else
        C_PlayerStats_NoGame();
}

//
// print CCMD
//
static void print_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
    }
    else
        HU_PlayerMessage(parms, true, false);
}

//
// quit CCMD
//
static void quit_func2(char *cmd, char *parms)
{
    quitcmd = true;

    if (vid_showfps)
    {
        vid_showfps = false;
        I_UpdateBlitFunc(false);
    }
}

//
// readme CCMD
//
static void readme_func2(char *cmd, char *parms)
{
    if (!*pwadfile)
        C_Warning(0, "A PWAD hasn't been loaded.");
    else
    {
        char    *temp = removeext(GetCorrectCase(pwadfile));
        char    *readme = M_StringJoin(temp, ".txt", NULL);

        if (BTSXE1)
            readme = M_StringDuplicate("btsx_e1.txt");
        else if (BTSXE2)
            readme = M_StringDuplicate("btsx_e2.txt");
        else if (BTSXE3)
            readme = M_StringDuplicate("btsx_e3.txt");
        else if (KDIKDIZD)
            readme = M_StringDuplicate("kdikdizd.txt");

        if (!M_FileExists(readme))
            C_Warning(0, BOLD("%s") " wasn't found.", leafname(readme));
        else
        {
#if defined(_WIN32)
            if (!ShellExecute(NULL, "open", readme, NULL, NULL, SW_SHOWNORMAL))
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__HAIKU__)
            if (!system(M_StringJoin("xdg-open ", readme, NULL)))
#elif defined(__APPLE__)
            if (!system(M_StringJoin("open ", readme, NULL)))
#endif
                C_Warning(0, BOLD("%s") " wouldn't open.", leafname(readme));
        }

        free(temp);
        free(readme);
    }
}

//
// regenhealth CCMD
//
static void regenhealth_func2(char *cmd, char *parms)
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
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_RHOFF);
        HU_SetPlayerMessage(s_STSTR_RHOFF, false, false);
    }
}

//
// reset CCMD
//
static void reset_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
        return;
    }

    if (M_StringCompare(parms, "all"))
    {
        resetall_func2("resetall", "");
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

        if (consolecmds[i].type == CT_CVAR && !(flags & CF_READONLY))
        {
            char    name[255];

            M_StringCopy(name, (english == english_american || M_StringCompare(consolecmds[i].altspelling, EMPTYVALUE) ?
                consolecmds[i].name : consolecmds[i].altspelling), sizeof(name));

            if (!wildcard(name, parms))
                continue;

            if (flags & CF_BOOLEAN)
            {
                if (*(bool *)consolecmds[i].variable != (bool)consolecmds[i].defaultnumber)
                {
                    char    *temp1 = C_LookupAliasFromValue((bool)consolecmds[i].defaultnumber, consolecmds[i].aliases);
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
            else if (flags & CF_INTEGER)
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
            if (M_StringCompare(parms, stringize(wadfolder)))
            {
                wad = "";
                M_SaveCVARs();
            }
#endif
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
            const char  *name = consolecmds[i].name;

            if (consolecmds[i].type == CT_CVAR && !(flags & CF_READONLY))
            {
                if (M_StringCompare(name, "ammo")
                    || M_StringCompare(name, "armor") || M_StringCompare(name, "armour")
                    || M_StringCompare(name, "armortype") || M_StringCompare(name, "armourtype")
                    || M_StringCompare(name, "health")
                    || M_StringCompare(name, "weapon"))
                    continue;

                if (flags & CF_BOOLEAN)
                {
                    char    *temp1 = C_LookupAliasFromValue((bool)consolecmds[i].defaultnumber, consolecmds[i].aliases);
                    char    *temp2 = uncommify(temp1);

                    consolecmds[i].func2(consolecmds[i].name, temp2);
                    free(temp1);
                    free(temp2);
                }
                else if (flags & CF_INTEGER)
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

            if (actions[i].controller1)
                *(int *)actions[i].controller1 = 0;

            if (actions[i].controller2)
                *(int *)actions[i].controller2 = 0;
        }

        for (int i = 0; i < NUMKEYS; i++)
            keyactionlist[i][0] = '\0';

        for (int i = 0; i < MAXMOUSEBUTTONS + 2; i++)
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
        keyboardfreelook = KEYFREELOOK_DEFAULT;
        keyboardgrid = KEYGRID_DEFAULT;
        keyboardjump = KEYJUMP_DEFAULT;
        keyboardleft = KEYLEFT_DEFAULT;
        keyboardmark = KEYMARK_DEFAULT;
        keyboardmaxzoom = KEYMAXZOOM_DEFAULT;
        keyboardmenu = KEYMENU_DEFAULT;
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

        mousealwaysrun = MOUSEALWAYSRUN_DEFAULT;
        mouseautomap = MOUSEAUTOMAP_DEFAULT;
        mouseback = MOUSEBACK_DEFAULT;
        mouseclearmark = MOUSECLEARMARK_DEFAULT;
        mouseconsole = MOUSECONSOLE_DEFAULT;
        mousefire = MOUSEFIRE_DEFAULT;
        mousefollowmode = MOUSEFOLLOWMODE_DEFAULT;
        mouseforward = MOUSEFORWARD_DEFAULT;
        mousefreelook = MOUSEFREELOOK_DEFAULT;
        mousegrid = MOUSEGRID_DEFAULT;
        mousejump = MOUSEJUMP_DEFAULT;
        mouseleft = MOUSELEFT_DEFAULT;
        mousemark = MOUSEMARK_DEFAULT;
        mousenextweapon = MOUSENEXTWEAPON_DEFAULT;
        mouseprevweapon = MOUSEPREVWEAPON_DEFAULT;
        mouseright = MOUSERIGHT_DEFAULT;
        mouserotatemode = MOUSEROTATEMODE_DEFAULT;
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
        mousezoomin = MOUSEZOOMIN_DEFAULT;
        mousezoomout = MOUSEZOOMOUT_DEFAULT;

        controlleralwaysrun = CONTROLLERALWAYSRUN_DEFAULT;
        controllerautomap = CONTROLLERAUTOMAP_DEFAULT;
        controllerback = CONTROLLERBACK_DEFAULT;
        controllerclearmark = CONTROLLERCLEARMARK_DEFAULT;
        controllerconsole = CONTROLLERCONSOLE_DEFAULT;
        controllerfire = CONTROLLERFIRE_DEFAULT;
        controllerfollowmode = CONTROLLERFOLLOWMODE_DEFAULT;
        controllerforward = CONTROLLERFORWARD_DEFAULT;
        controllerfreelook = CONTROLLERFREELOOK_DEFAULT;
        controllergrid = CONTROLLERGRID_DEFAULT;
        controllerjump = CONTROLLERJUMP_DEFAULT;
        controllerleft = CONTROLLERLEFT_DEFAULT;
        controllermark = CONTROLLERMARK_DEFAULT;
        controllermaxzoom = CONTROLLERMAXZOOM_DEFAULT;
        controllermenu = CONTROLLERMENU_DEFAULT;
        controllernextweapon = CONTROLLERNEXTWEAPON_DEFAULT;
        controllerprevweapon = CONTROLLERPREVWEAPON_DEFAULT;
        controllerright = CONTROLLERRIGHT_DEFAULT;
        controllerrotatemode = CONTROLLERROTATEMODE_DEFAULT;
        controllerrun = CONTROLLERRUN_DEFAULT;
        controllerstrafe = CONTROLLERSTRAFE_DEFAULT;
        controllerstrafeleft = CONTROLLERSTRAFELEFT_DEFAULT;
        controllerstraferight = CONTROLLERSTRAFERIGHT_DEFAULT;
        controlleruse = CONTROLLERUSE_DEFAULT;
        controlleruse2 = CONTROLLERUSE2_DEFAULT;
        controllerweapon1 = CONTROLLERWEAPON_DEFAULT;
        controllerweapon2 = CONTROLLERWEAPON_DEFAULT;
        controllerweapon3 = CONTROLLERWEAPON_DEFAULT;
        controllerweapon4 = CONTROLLERWEAPON_DEFAULT;
        controllerweapon5 = CONTROLLERWEAPON_DEFAULT;
        controllerweapon6 = CONTROLLERWEAPON_DEFAULT;
        controllerweapon7 = CONTROLLERWEAPON_DEFAULT;
        controllerzoomin = CONTROLLERZOOMIN_DEFAULT;
        controllerzoomout = CONTROLLERZOOMOUT_DEFAULT;

        // clear all aliases
        for (int i = 0; i < MAXALIASES; i++)
        {
            aliases[i].name[0] = '\0';
            aliases[i].string[0] = '\0';
        }

        M_SaveCVARs();

        C_Output("All CVARs and controls have been reset to their defaults.");
    }
}

static void resetall_func2(char *cmd, char *parms)
{
    static char line2[160];
    static char resetallstring[320];

    M_snprintf(line2, sizeof(line2), (usingcontroller ? s_PRESSA : s_PRESSYN), selectbutton);
    M_snprintf(resetallstring, sizeof(resetallstring),
        "Are you sure you want to reset all CVARs\nand controls to their defaults?\n\n%s", line2);
    M_StartMessage(resetallstring, &C_VerifyResetAll, true);

    SDL_StopTextInput();
    S_StartSound(NULL, sfx_swtchn);
}

//
// respawnitems CCMD
//
static void respawnitems_func2(char *cmd, char *parms)
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
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
    }
    else
    {
        C_Output(s_STSTR_RIOFF);
        HU_SetPlayerMessage(s_STSTR_RIOFF, false, false);
    }
}

//
// respawnmonsters CCMD
//
static void respawnmonsters_func2(char *cmd, char *parms)
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
}

//
// restartmap CCMD
//
static void restartmap_func2(char *cmd, char *parms)
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

static bool resurrect_func1(char *cmd, char *parms)
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
                char    *temp1 = removenonalpha(mobjinfo[i].name1);
                char    *temp2 = (*mobjinfo[i].plural1 ? removenonalpha(mobjinfo[i].plural1) : NULL);
                char    *temp3 = (*mobjinfo[i].name2 ? removenonalpha(mobjinfo[i].name2) : NULL);
                char    *temp4 = (*mobjinfo[i].plural2 ? removenonalpha(mobjinfo[i].plural2) : NULL);
                char    *temp5 = (*mobjinfo[i].name3 ? removenonalpha(mobjinfo[i].name3) : NULL);
                char    *temp6 = (*mobjinfo[i].plural3 ? removenonalpha(mobjinfo[i].plural3) : NULL);

                if (M_StringStartsWith(parm, "all"))
                    M_StringReplaceAll(parm, "all", "", false);

                resurrectcmdtype = mobjinfo[i].doomednum;

                if (resurrectcmdtype >= 0
                    && (M_StringCompare(parm, temp1)
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

static void resurrect_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMECCMDWARNING1);
            else
                C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                    (playergender == playergender_other ? "aren't" : "isn't"));
        }
    }
    else
    {
        char    buffer[1024];
        bool    cheated = false;

        if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me") || (*playername && M_StringCompare(parm, playername)))
        {
            P_ResurrectPlayer(initial_health);

            if (M_StringCompare(playername, playername_default))
                M_StringCopy(buffer, "You resurrected yourself!", sizeof(buffer));
            else
                M_snprintf(buffer, sizeof(buffer), "%s resurrected %s!", playername, pronoun(reflexive));

            C_PlayerObituary(buffer);
            C_HideConsole();
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

                    M_snprintf(buffer, sizeof(buffer), "%s%s dead monster%s in this map %s been resurrected.",
                        (resurrected == 1 ? "The " : "All "), temp, (resurrected == 1 ? "" : "s"), (resurrected == 1 ? "has" : "have"));
                    C_Output(buffer);
                    C_HideConsole();
                    HU_SetPlayerMessage(buffer, false, false);
                    free(temp);
                }
                else
                    C_Warning(0, "There are no dead monsters in this map to resurrect.");
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
                    free(temp);
                }
                else
                {
                    if (gamemode != commercial)
                    {
                        if (resurrectcmdtype >= ArchVile && resurrectcmdtype <= MonsterSpawner)
                        {
                            C_Warning(0, "There are no %s in " ITALICS("%s") ".",
                                mobjinfo[type].plural1, gamedescription);
                            return;
                        }
                        else if (gamemode == shareware && (resurrectcmdtype == Cyberdemon || resurrectcmdtype == SpiderMastermind))
                        {
                            C_Warning(0, "There are no %s in the shareware version of " ITALICS("DOOM") ". "
                                "You can buy the full version on " ITALICS("Steam") ", etc.",
                                mobjinfo[type].plural1, gamedescription);
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
            stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
            M_SaveCVARs();
        }
    }
}

//
// save CCMD
//
static void save_func2(char *cmd, char *parms)
{
    char    buffer[1024];

    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
        return;
    }

    if (strlen(parms) == 1 && parms[0] >= '1' && parms[0] <= '8')
    {
        M_snprintf(buffer, sizeof(buffer), "%s" DOOMRETRO_SAVEGAME, savegamefolder, parms[0] - '1');
        G_SaveGame(parms[0] - '1', maptitle, buffer);
    }
    else
    {
        M_snprintf(buffer, sizeof(buffer), "%s%s%s",
            (M_StringStartsWith(parms, savegamefolder) ? "" : savegamefolder),
            parms, (M_StringEndsWith(parms, ".save") ? "" : ".save"));
        G_SaveGame(-1, "", buffer);
    }
}

//
// spawn CCMD
//
static int  spawncmdtype = NUMMOBJTYPES;
static bool spawncmdfriendly;

static bool spawn_func1(char *cmd, char *parms)
{
    bool    result = false;
    char    *parm = removenonalpha(parms);

    if (!*parm)
        return true;

    if (gamestate == GS_LEVEL)
    {
        int num = -1;

        spawncmdfriendly = false;

        if (M_StringCompare(parm, "unfriendlydog"))
            M_StringReplaceAll(parm, "unfriendly", "", false);
        else if (M_StringCompare(parm, "dog"))
            spawncmdfriendly = true;
        else if (M_StringStartsWith(parm, "friendly"))
        {
            spawncmdfriendly = true;
            M_StringReplaceAll(parm, "friendly", "", false);
        }
        else if (M_StringStartsWith(parm, "unfriendly"))
            M_StringReplaceAll(parm, "unfriendly", "", false);

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

static void spawn_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMECCMDWARNING1);
            else
                C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                    (playergender == playergender_other ? "aren't" : "isn't"));
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

                C_Warning(0, "%s can't be spawned in " ITALICS("%s") ".", buffer, gamedescription);
                spawn = false;
            }

            if (gamemode == shareware
                && (spawncmdtype == SpiderMastermind
                    || spawncmdtype == Cyberdemon
                    || spawncmdtype == CellPack
                    || spawncmdtype == HelperDog
                    || spawncmdtype == PlasmaRifle
                    || spawncmdtype == BFG9000
                    || spawncmdtype == Berserk
                    || spawncmdtype == Cell))
            {
                M_StringCopy(buffer, mobjinfo[type].plural1, sizeof(buffer));

                if (!*buffer)
                    M_snprintf(buffer, sizeof(buffer), "%ss", mobjinfo[type].name1);

                C_Warning(0, "%s can't be spawned in the shareware version of " ITALICS("DOOM") ". "
                    "You can buy the full version on " ITALICS("Steam") ", etc.", buffer);
                spawn = false;
            }
        }
        else if (spawncmdtype == WolfensteinSS && !allowwolfensteinss)
        {
            if (bfgedition)
                C_Warning(0, "%s%s can't be spawned in " ITALICS("%s (BFG Edition)") ".",
                    (spawncmdfriendly ? "Friendly " : ""), mobjinfo[type].name1, gamedescription);
            else
                C_Warning(0, "%s%s can't be spawned in " ITALICS("%s") ".",
                    (spawncmdfriendly ? "Friendly " : ""), mobjinfo[type].name1, gamedescription);

            spawn = false;
        }

        if (spawn)
        {
            fixed_t     x = viewx + 100 * viewcos;
            fixed_t     y = viewy + 100 * viewsin;
            sector_t    *sector = R_PointInSubsector(x, y)->sector;

            if (mobjinfo[type].height > sector->ceilingheight - sector->floorheight
                || P_CheckLineSide(viewplayer->mo, x, y))
                C_Warning(0, "There isn't enough room in front of %s to spawn a %s.",
                    (M_StringCompare(playername, playername_default) ? "you" : playername),
                    mobjinfo[type].name1);
            else
            {
                mapthing_t  mthing = { 0 };
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
                            stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
                            M_SaveCVARs();
                        }

                        thing->flags2 |= MF2_SPAWNEDBYPLAYER;
                        massacre = false;

                        if (flags & MF_NOGRAVITY)
                            thing->z = thing->floorz + 32 * FRACUNIT;

                        if (!freeze)
                        {
                            fog = P_SpawnMobj(x, y, ((flags & MF_NOGRAVITY) ? thing->z : ONFLOORZ), MT_TFOG);
                            fog->angle = thing->angle;
                            S_StartSound(fog, sfx_telept);
                        }
                    }
                    else
                    {
                        if (flags & MF_COUNTITEM)
                        {
                            stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
                            M_SaveCVARs();
                        }

                        if (!freeze)
                        {
                            fog = P_SpawnMobj(x, y, ((flags & MF_SPAWNCEILING) ? ONCEILINGZ :
                                ((thing->flags2 & MF2_FLOATBOB) ? thing->floorz + 14 * FRACUNIT : ONFLOORZ)), MT_IFOG);
                            fog->angle = thing->angle;
                            S_StartSound(fog, sfx_itmbk);
                        }
                    }

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
static bool take_func1(char *cmd, char *parms)
{
    bool    result = false;
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
        return true;

    if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything")
        || M_StringCompare(parm, "health") || M_StringCompare(parm, "allhealth")
        || M_StringCompare(parm, "weapons") || M_StringCompare(parm, "allweapons")
        || M_StringCompare(parm, "ammo") || M_StringCompare(parm, "allammo")
        || M_StringCompare(parm, "ammunition") || M_StringCompare(parm, "allammunition")
        || M_StringCompare(parm, "armor") || M_StringCompare(parm, "allarmor")
        || M_StringCompare(parm, "armour") || M_StringCompare(parm, "allarmour")
        || M_StringCompare(parm, "keys") || M_StringCompare(parm, "allkeys")
        || M_StringCompare(parm, "keycards") || M_StringCompare(parm, "allkeycards")
        || M_StringCompare(parm, "skullkeys") || M_StringCompare(parm, "allskullkeys")
        || M_StringCompare(parm, "pistol") || M_StringCompare(parm, "powerups"))
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

static void take_func2(char *cmd, char *parms)
{
    char    *parm = removenonalpha(parms);

    if (!*parm || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMECCMDWARNING1);
            else
                C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                    (playergender == playergender_other ? "aren't" : "isn't"));
        }
    }
    else
    {
        bool    result = false;

        if (M_StringCompare(parm, "all") || M_StringCompare(parm, "everything"))
        {
            P_TakeBackpack();

            if (viewplayer->health > initial_health)
            {
                healthcvar = true;
                P_DamageMobj(viewplayer->mo, viewplayer->mo, viewplayer->mo, viewplayer->health - initial_health, false, false);
                healthcvar = false;
                result = true;
            }

            for (weapontype_t i = wp_pistol; i < NUMWEAPONS; i++)
                if (viewplayer->weaponowned[i])
                {
                    viewplayer->weaponowned[i] = false;
                    oldweaponsowned[i] = false;
                    result = true;
                }

            viewplayer->pendingweapon = wp_fist;

            for (ammotype_t i = 0; i < NUMAMMO; i++)
                if (viewplayer->ammo[i])
                {
                    viewplayer->ammo[i] = 0;
                    result = true;
                }

            if (viewplayer->armor)
            {
                P_AnimateArmor(viewplayer->armor);
                viewplayer->armor = 0;
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
                C_PlayerWarning("Everything was taken from %s!",
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have anything!");
            else
                C_Warning(0, "%s doesn't have anything!", playername);
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
                    C_PlayerWarning("You killed yourself!");
                else
                    C_PlayerWarning("%s killed %s!", playername, pronoun(reflexive));

                C_HideConsole();
            }
            else
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, "You are already dead!");
                else
                    C_Warning(0, "%s is already dead!", playername);
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
                C_PlayerWarning("All weapons have been taken from %s!",
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any weapons!");
            else
                C_Warning(0, "%s doesn't have any weapons!", playername);
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
                C_PlayerWarning("All ammo was taken from %s!",
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any ammo!");
            else
                C_Warning(0, "%s doesn't have any ammo!", playername);
        }
        else if (M_StringCompare(parm, "armor") || M_StringCompare(parm, "allarmor")
                || M_StringCompare(parm, "armour") || M_StringCompare(parm, "allarmour"))
        {
            if (viewplayer->armor)
            {
                P_AnimateArmor(viewplayer->armor);
                viewplayer->armor = 0;
                viewplayer->armortype = armortype_none;
                C_PlayerWarning("All %s was taken from %s!",
                    (english == english_american ? "armor" : "armour"),
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any %s!",
                    (english == english_american ? "armor" : "armour"));
            else
                C_Warning(0, "%s doesn't have any %s!",
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
                P_LookForCards();
                C_PlayerWarning("All keycards and skull keys have been taken from %s!",
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any keycards or skull keys!");
            else
                C_Warning(0, "%s doesn't have any keycards or skull keys!", playername);
        }
        else if (M_StringCompare(parm, "keycards") || M_StringCompare(parm, "allkeycards"))
        {
            if (viewplayer->cards[it_bluecard] > 0 || viewplayer->cards[it_redcard] > 0 || viewplayer->cards[it_yellowcard] > 0)
            {
                viewplayer->cards[it_bluecard] = 0;
                viewplayer->cards[it_redcard] = 0;
                viewplayer->cards[it_yellowcard] = 0;
                P_LookForCards();
                C_PlayerWarning("All keycards have been taken from %s!",
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any keycards!");
            else
                C_Warning(0, "%s doesn't have any keycards!", playername);
        }
        else if (M_StringCompare(parm, "skullkeys") || M_StringCompare(parm, "allskullkeys"))
        {
            if (viewplayer->cards[it_blueskull] > 0 || viewplayer->cards[it_redskull] > 0 || viewplayer->cards[it_yellowskull] > 0)
            {
                viewplayer->cards[it_blueskull] = 0;
                viewplayer->cards[it_redskull] = 0;
                viewplayer->cards[it_yellowskull] = 0;
                P_LookForCards();
                C_PlayerWarning("All skull keys have been taken from %s!",
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any skull keys!");
            else
                C_Warning(0, "%s doesn't have any skull keys!", playername);
        }
        else if (M_StringCompare(parm, "pistol"))
        {
            if (viewplayer->weaponowned[wp_pistol])
            {
                viewplayer->weaponowned[wp_pistol] = false;
                oldweaponsowned[wp_pistol] = false;

                P_CheckAmmo(viewplayer->readyweapon);

                C_PlayerWarning("A pistol was taken from %s!",
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have a pistol!");
            else
                C_Warning(0, "%s doesn't have a pistol!", playername);
        }
        else if (M_StringCompare(parm, "powerups"))
        {
            for (int i = 0; i < NUMPOWERS; i++)
                if (viewplayer->powers[i])
                {
                    viewplayer->powers[i] = 0;
                    result = true;
                }

            if (result)
            {
                C_PlayerWarning("All power-ups have been taken from %s!",
                    (M_StringCompare(playername, playername_default) ? "you" : playername));
                C_HideConsole();
            }
            else if (M_StringCompare(playername, playername_default))
                C_Warning(0, "You don't have any power-ups!");
            else
                C_Warning(0, "%s doesn't have any power-ups!", playername);
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
                        if (i == MT_MISC0 || i == MT_MISC1)
                        {
                            char    *temp4 = sentencecase(mobjinfo[i].name1);

                            C_PlayerWarning("%s was taken from %s!", temp4,
                                (M_StringCompare(playername, playername_default) ? "you" : playername));
                            free(temp4);
                        }
                        else
                            C_PlayerWarning("%s %s was taken from %s!",
                                (isvowel(mobjinfo[i].name1[0]) ? "An" : "A"), mobjinfo[i].name1,
                                (M_StringCompare(playername, playername_default) ? "you" : playername));

                        if (viewplayer->health <= 0)
                        {
                            healthcvar = true;
                            P_KillMobj(viewplayer->mo, NULL, viewplayer->mo, false);
                            healthcvar = false;
                        }

                        C_HideConsole();
                        result = true;
                    }
                    else if (M_StringCompare(playername, playername_default))
                        C_Warning(0, "You don't have %s %s!",
                            (isvowel(mobjinfo[i].name1[0]) ? "an" : "a"), mobjinfo[i].name1);
                    else
                        C_Warning(0, "%s doesn't have %s %s!",
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
static bool teleport_func1(char *cmd, char *parms)
{
    if (!*parms || gamestate != GS_LEVEL)
        return true;
    else
    {
        fixed_t x, y;

        return (sscanf(parms, "%10i %10i", &x, &y) == 2);
    }
}

static void teleport_func2(char *cmd, char *parms)
{
    if (!*parms || gamestate != GS_LEVEL)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMECCMDWARNING1);
            else
                C_Warning(0, NOGAMECCMDWARNING2, playername, pronoun(personal),
                    (playergender == playergender_other ? "aren't" : "isn't"));
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
                    C_Warning(0, "You are already there!");
                else
                    C_Warning(0, "%s is already there!", playername);
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
                stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
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
static void thinglist_func2(char *cmd, char *parms)
{
    const int   tabs[MAXTABS] = { 50, 300, 450 };

    C_Header(tabs, thinglist, THINGLISTHEADER);

    for (thinker_t *th = thinkers[th_mobj].cnext; th != &thinkers[th_mobj]; th = th->cnext)
    {
        mobj_t      *mobj = (mobj_t *)th;
        const int   flags = mobj->flags;
        char        name[128];
        char        *temp;
        const int   angle = (int)(mobj->angle * 90.0 / ANG90);

        if (mobj == viewplayer->mo)
            M_StringCopy(name, (M_StringCompare(playername, playername_default) ? "You" : playername), sizeof(name));
        else if (*mobj->name)
            M_StringCopy(name, mobj->name, sizeof(name));
        else
            M_snprintf(name, sizeof(name), "%s%s",
                ((flags & MF_CORPSE) && !(mobj->flags2 & MF2_DECORATION) ? "dead " :
                    ((flags & MF_FRIEND) && mobj->type != MT_PLAYER ? "friendly " :
                    ((flags & MF_DROPPED) ? "dropped " : ""))),
                (mobj->type == MT_PLAYER ? "voodoo doll" : (*mobj->info->name1 ? mobj->info->name1 : "\x96")));

        temp = sentencecase(name);

        if (mobj->id >= 0)
            C_TabbedOutput(tabs, MONOSPACED("%4i") ".\t%s\t(%i, %i, %i)\t%i\xB0", mobj->id,
                temp, mobj->x >> FRACBITS, mobj->y >> FRACBITS, mobj->z >> FRACBITS,
                (angle == 360 ? 0 : angle));
        else
            C_TabbedOutput(tabs, "\t%s\t(%i, %i, %i)\t%i\xB0",
                temp, mobj->x >> FRACBITS, mobj->y >> FRACBITS, mobj->z >> FRACBITS,
                (angle == 360 ? 0 : angle));

        free(temp);
    }
}

//
// timer CCMD
//
static void timer_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
    }
    else
    {
        int value;

        M_StringReplaceAll(parms, ",", "", false);

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
                char    *temp1 = commify(value);
                char    *temp2 = titlecase(playername);

                if (timer)
                    C_Output("The timer has been %s to %s minute%s. "
                        "%s will automatically exit each map once the timer runs out.",
                        (value == timer ? "reset" : "changed"), temp1, (value == 1 ? "" : "s"),
                        (M_StringCompare(playername, playername_default) ? "You" : temp2));
                else
                    C_Output("A timer has been set for %s minute%s. "
                        "%s will automatically exit each map once the timer runs out.",
                        temp1, (value == 1 ? "" : "s"),
                        (M_StringCompare(playername, playername_default) ? "You" : temp2));

                free(temp1);
                free(temp2);
            }
        }

        P_SetTimer(value);
    }
}

//
// toggle CCMD
//
static void toggle_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
        return;
    }

    togglingcvar = true;

    for (int i = 0; *consolecmds[i].name; i++)
    {
        const int   flags = consolecmds[i].flags;

        if (consolecmds[i].type == CT_CVAR && M_StringCompare(parms, consolecmds[i].name)
            && !(flags & CF_READONLY) && (flags & CF_BOOLEAN))
        {
            char    *temp1 = C_LookupAliasFromValue(!(*(bool *)consolecmds[i].variable), consolecmds[i].aliases);
            char    *temp2 = M_StringJoin(parms, " ", temp1, NULL);

            C_ValidateInput(temp2);
            C_Output("The " BOLD("%s") " CVAR has been toggled " BOLD("%s") ".", parms, temp1);
            free(temp1);
            free(temp2);
            M_SaveCVARs();
            break;
        }
    }

    togglingcvar = false;
}

//
// unbind CCMD
//
static void unbind_func2(char *cmd, char *parms)
{
    if (!*parms)
    {
        const int   i = C_GetIndex(cmd);

        C_ShowFormat(i);
        C_ShowDescription(i);
        return;
    }

    bind_func2(cmd, parms);
}

//
// vanilla CCMD
//
static void vanilla_func2(char *cmd, char *parms)
{
    static bool buddha;
    static bool hud;
    static bool nomousestrafe;
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

    nobindoutput = true;
    togglingvanilla = true;

    if (vanilla)
    {
        hud = r_hud;
        showfps = vid_showfps;

        SC_Open(W_CheckNumForName("VANILLA"));

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

        if ((nomousestrafe = (mousestrafe != MOUSESTRAFE_DEFAULT)))
            bind_func2("bind", "mouse2 +strafe");
    }
    else
    {
        if (buddha)
            viewplayer->cheats |= CF_BUDDHA;

        r_hud = hud;
        vid_showfps = showfps;

        if (nomousestrafe)
            bind_func2("unbind", "mouse2 +strafe");

        M_LoadCVARs(configfile);
    }

    if (gamestate == GS_LEVEL)
        C_HideConsole();

    I_RestartGraphics(false);
    AM_InitPixelSize();

    nobindoutput = false;
    togglingvanilla = false;

    if (vanilla)
    {
        C_Output(s_STSTR_VON);
        HU_SetPlayerMessage(s_STSTR_VON, false, false);
        C_Warning(0, "Changes to any CVARs won't be saved while in vanilla mode.");
    }
    else
    {
        C_Output(s_STSTR_VOFF);
        HU_SetPlayerMessage(s_STSTR_VOFF, false, false);
    }
}

//
// wiki CCMD
//
static void wiki_func2(char *cmd, char *parms)
{
    C_Output("Opening the " ITALICS(DOOMRETRO_WIKINAME) "...");

#if defined(_WIN32)
    if (!ShellExecute(NULL, "open", DOOMRETRO_WIKIURL, NULL, NULL, SW_SHOWNORMAL))
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__HAIKU__)
    if (!system("xdg-open " DOOMRETRO_WIKIURL))
#elif defined(__APPLE__)
    if (!system("open " DOOMRETRO_WIKIURL))
#endif
        C_Warning(0, "The " ITALICS(DOOMRETRO_WIKINAME) " wouldn't open.");
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

    if (M_StringStartsWith(cmd, "am_"))
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

            M_StringReplaceAll(parms, ",", "", false);

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

                M_StringReplaceAll(parms, ",", "", false);

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

            M_StringReplaceAll(parms, ",", "", false);

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

                M_StringReplaceAll(parms, ",", "", false);

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
            if (M_StringCompare(parms, EMPTYVALUE) && *(char **)consolecmds[i].variable && !(consolecmds[i].flags & CF_READONLY))
            {
                *(char **)consolecmds[i].variable = "";
                M_SaveCVARs();
            }
            else if (*parms)
            {
                if (!(consolecmds[i].flags & CF_READONLY))
                {
                    parms[consolecmds[i].length] = '\0';
                    *(char **)consolecmds[i].variable = M_StringDuplicate(parms);
                    M_StripQuotes(trimwhitespace(*(char **)consolecmds[i].variable));
                    M_SaveCVARs();
                }
            }
            else
            {
                C_ShowDescription(i);

                if (consolecmds[i].flags & CF_READONLY)
                {
                    if (M_StringCompare(consolecmds[i].name, stringize(version)))
                        C_Output(STRINGCVARWITHNODEFAULT, "", *(char **)consolecmds[i].variable, "");
                    else
                        C_Output(STRINGCVARWITHNODEFAULT, "\"", *(char **)consolecmds[i].variable, "\"");
                }
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
static void alwaysrun_func2(char *cmd, char *parms)
{
    bool_cvars_func2(cmd, parms);

    if (!consoleactive)
        I_InitKeyboard();
}

//
// am_display CVAR
//
static void am_display_func2(char *cmd, char *parms)
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
static void am_external_func2(char *cmd, char *parms)
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
            if (mapwindow && gamestate == GS_LEVEL)
                AM_Stop();

            I_DestroyExternalAutomap();
            mapscreen = *screens;
        }

        AM_SetAutomapSize(r_screensize);
    }
}

//
// am_followmode CVAR
//
static void am_followmode_func2(char *cmd, char *parms)
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
static void am_gridsize_func2(char *cmd, char *parms)
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
static void am_path_func2(char *cmd, char *parms)
{
    bool_cvars_func2(cmd, parms);

    if (am_path)
    {
        viewplayer->cheated++;
        stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
        M_SaveCVARs();
    }
}

//
// am_rotatemode CVAR
//
static void am_rotatemode_func2(char *cmd, char *parms)
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
static bool armortype_func1(char *cmd, char *parms)
{
    return (!*parms || (C_LookupValueFromAlias(parms, ARMORTYPEVALUEALIAS) != INT_MIN && gamestate == GS_LEVEL));
}

static void armortype_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, ARMORTYPEVALUEALIAS);

        if (value != INT_MIN && viewplayer->armor)
        {
            viewplayer->armortype = value;

            if (value == armortype_none)
            {
                P_AnimateArmor(viewplayer->armor);
                viewplayer->armor = 0;
            }
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
static void autotilt_func2(char *cmd, char *parms)
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
static bool crosshair_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, CROSSHAIRVALUEALIAS) != INT_MIN);
}

static void crosshair_func2(char *cmd, char *parms)
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
static bool english_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, ENGLISHVALUEALIAS) != INT_MIN);
}

static void english_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, ENGLISHVALUEALIAS);

        if ((value == english_american || value == english_british) && value != english)
        {
            english = value;
            ST_InitStatBar();
            D_TranslateDehStrings();
            M_TranslateAutocomplete();
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
static void episode_func2(char *cmd, char *parms)
{
    const int   episode_old = episode;

    int_cvars_func2(cmd, parms);

    if (episode != episode_old && gamemode != commercial)
        EpiDef.laston = MIN(episode, (gamemode == retail ? (sigil ? (sigil2 ? 6 : 5) : 4) :
            (gamemode == shareware || chex ? 1 : 3))) - 1;
}

//
// expansion CVAR
//
static void expansion_func2(char *cmd, char *parms)
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
// freelook CVAR
//
static void freelook_func2(char *cmd, char *parms)
{
    const bool  freelook_old = freelook;

    bool_cvars_func2(cmd, parms);

    if (freelook != freelook_old && gamestate == GS_LEVEL)
    {
        suppresswarnings = true;
        R_InitSkyMap();
        suppresswarnings = false;

        R_InitColumnFunctions();

        if (!freelook)
        {
            viewplayer->lookdir = 0;
            viewplayer->oldlookdir = 0;
            viewplayer->recoil = 0;
            viewplayer->oldrecoil = 0;
        }
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
                    I_SetControllerLeftDeadZone();
                    M_SaveCVARs();
                }
            }
            else if (joy_deadzone_right != value)
            {
                joy_deadzone_right = BETWEENF(joy_deadzone_right_min, value, joy_deadzone_right_max);
                I_SetControllerRightDeadZone();
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
        I_SetControllerHorizontalSensitivity();
    else if (joy_sensitivity_vertical != joy_sensitivity_vertical_old)
        I_SetControllerVerticalSensitivity();
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

                if (value > viewplayer->maxammo[ammotype] && !viewplayer->backpack)
                    P_GiveBackpack(false, false);

                if ((value = MIN(value, viewplayer->maxammo[ammotype])) > viewplayer->ammo[ammotype])
                {
                    P_UpdateAmmoStat(ammotype, value - viewplayer->ammo[ammotype]);
                    P_AddBonus();
                    S_StartSound(viewplayer->mo, sfx_itemup);
                }
                else
                    S_StartSound(viewplayer->mo, weaponinfo[readyweapon].sound);

                P_AnimateAmmo(viewplayer->ammo[ammotype] - value, ammotype);
                viewplayer->ammo[ammotype] = value;
                P_CheckAmmo(readyweapon);

                if (viewplayer->pendingweapon != wp_nochange)
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
                    C_Warning(0, NOGAMECVARWARNING1);
                else
                    C_Warning(0, NOGAMECVARWARNING2, playername, pronoun(personal),
                        (playergender == playergender_other ? "aren't" : "isn't"));
            }

            free(temp);
        }
    }
    else if (M_StringCompare(cmd, stringize(armor)))
    {
        if (*parms)
        {
            if (sscanf(parms, "%10i", &value) == 1 && value != viewplayer->armor)
            {
                armorhighlight = I_GetTimeMS() + HUD_ARMOR_HIGHLIGHT_WAIT;

                if ((value = MIN(value, max_armor)) > viewplayer->armor)
                {
                    P_UpdateArmorStat(value - viewplayer->armor);
                    P_AddBonus();
                    S_StartSound(viewplayer->mo, sfx_itemup);
                }
                else
                    S_StartSound(NULL, sfx_plpain);

                P_AnimateArmor(viewplayer->armor - value);

                if (!(viewplayer->armor = value))
                    viewplayer->armortype = armortype_none;
                else if (!viewplayer->armortype)
                    viewplayer->armortype = green_armor_class;
            }
        }
        else
        {
            const int   i = C_GetIndex(cmd);
            char        *temp = commify(viewplayer->armor);

            C_ShowDescription(i);
            C_Output(PERCENTCVARWITHNODEFAULT, temp);

            if (gamestate != GS_LEVEL)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, NOGAMECVARWARNING1);
                else
                    C_Warning(0, NOGAMECVARWARNING2, playername, pronoun(personal),
                        (playergender == playergender_other ? "aren't" : "isn't"));
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
                value = BETWEEN(((viewplayer->cheats & CF_BUDDHA) ? 1 : HUD_NUMBER_MIN), value, maxhealth);

                healthhighlight = I_GetTimeMS() + HUD_HEALTH_HIGHLIGHT_WAIT;
                P_AnimateHealth(viewplayer->health - value);

                if (viewplayer->health <= 0)
                {
                    if (value <= 0)
                    {
                        if (value < viewplayer->health)
                            viewplayer->damagecount = viewplayer->health - value;

                        viewplayer->health = value;
                        viewplayer->mo->health = value;
                    }
                    else
                    {
                        char    buffer[1024];

                        P_ResurrectPlayer(value);
                        P_AddBonus();

                        if (M_StringCompare(playername, playername_default))
                            M_StringCopy(buffer, "You resurrected yourself!", sizeof(buffer));
                        else
                            M_snprintf(buffer, sizeof(buffer), "%s resurrected %s!", playername, pronoun(reflexive));

                        C_PlayerObituary(buffer);
                    }
                }
                else
                {
                    if (value < viewplayer->health)
                    {
                        viewplayer->damagecount = viewplayer->health - value;
                        viewplayer->health = value;
                        viewplayer->mo->health = value;
                        healthcvar = true;

                        if (value <= 0)
                        {
                            P_KillMobj(viewplayer->mo, NULL, viewplayer->mo, false);
                            C_HideConsole();
                        }
                        else
                            S_StartSound(NULL, sfx_plpain);
                    }
                    else
                    {
                        P_UpdateHealthStat(value - viewplayer->health);
                        viewplayer->health = value;
                        viewplayer->mo->health = value;
                        P_AddBonus();
                        S_StartSound(viewplayer->mo, sfx_itemup);
                    }
                }
            }
        }
        else
        {
            char        *temp = commify(negativehealth && minuspatch && !viewplayer->health ?
                            viewplayer->negativehealth : viewplayer->health);
            const int   i = C_GetIndex(cmd);

            C_ShowDescription(i);
            C_Output(PERCENTCVARWITHNODEFAULT, temp);

            if (gamestate != GS_LEVEL)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, NOGAMECVARWARNING1);
                else
                    C_Warning(0, NOGAMECVARWARNING2, playername, pronoun(personal),
                        (playergender == playergender_other ? "aren't" : "isn't"));
            }

            free(temp);
        }
    }
}

//
// playergender CVAR
//
static bool playergender_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, GENDERVALUEALIAS) != INT_MIN);
}

static void playergender_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, GENDERVALUEALIAS);

        if (value != INT_MIN && playergender != value)
        {
            playergender = value;
            M_SaveCVARs();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(playergender, GENDERVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (playergender == playergender_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(playergender_default, GENDERVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_blood CVAR
//
static bool r_blood_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, BLOODVALUEALIAS) != INT_MIN);
}

static void r_blood_func2(char *cmd, char *parms)
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
static void r_bloodsplats_translucency_func2(char *cmd, char *parms)
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
static void r_brightmaps_func2(char *cmd, char *parms)
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
// r_corpses_mirrored CVAR
//
static void r_corpses_mirrored_func2(char *cmd, char *parms)
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
static bool r_detail_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, DETAILVALUEALIAS) != INT_MIN);
}

static void r_detail_func2(char *cmd, char *parms)
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

        AM_InitPixelSize();

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// r_diskicon CVAR
//
static void r_diskicon_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_diskicon)
        {
            if ((r_diskicon = value))
            {
                drawdisk = true;
                drawdisktics = TICRATE;
            }

            M_SaveCVARs();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_diskicon, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_diskicon == r_diskicon_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_diskicon_default, BOOLVALUEALIAS);

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
static void r_ditheredlighting_func2(char *cmd, char *parms)
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
static void r_fixmaperrors_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_fixmaperrors)
        {
            r_fixmaperrors = value;
            M_SaveCVARs();

            if (gamestate == GS_LEVEL && !togglingvanilla && !resettingcvar)
            {
                if (M_StringCompare(playername, playername_default))
                    C_Warning(0, NEXTMAPWARNING1);
                else
                    C_Warning(0, NEXTMAPWARNING2, playername);
            }
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
static void r_fov_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        int value;

        if (sscanf(parms, "%10i", &value) == 1 && value != r_fov)
        {
            r_fov = value;
            M_SaveCVARs();
            I_RestartGraphics(false);

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
static bool r_gamma_func1(char *cmd, char *parms)
{
    return (C_LookupValueFromAlias(parms, GAMMAVALUEALIAS) != INT_MIN || float_cvars_func1(cmd, parms));
}

static void r_gamma_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        float   value = (float)C_LookupValueFromAlias(parms, GAMMAVALUEALIAS);

        if ((value != INT_MIN || sscanf(parms, "%10f", &value) == 1) && value != r_gamma)
        {
            r_gamma = BETWEENF(r_gamma_min, value, r_gamma_max);
            I_SetGamma(r_gamma);
            I_UpdateColors();
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

            C_Output(INTEGERCVARWITHDEFAULT, (r_gamma == 1.0f ? "off" : buffer1),
                (r_gamma_default == 1.0f ? "off" : buffer2));
        }

        C_ShowWarning(i);
    }
}

//
// r_hud CVAR
//
static void r_hud_func2(char *cmd, char *parms)
{
    const bool  r_hud_old = r_hud;

    bool_cvars_func2(cmd, parms);

    if (r_hud != r_hud_old && r_hud && !togglingvanilla)
    {
        if (r_screensize != r_screensize_max || !vid_widescreen)
        {
            if (r_screensize != r_screensize_max)
            {
                r_screensize = r_screensize_max;
                C_IntegerCVAROutput(stringize(r_screensize), r_screensize);
                R_SetViewSize(r_screensize);
            }

            if (!vid_widescreen)
            {
                vid_widescreen = true;
                C_StringCVAROutput(stringize(vid_widescreen), "on");
                I_RestartGraphics(false);
            }

            S_StartSound(NULL, sfx_stnmov);
        }
    }
}

//
// r_hud_translucency CVAR
//
static void r_hud_translucency_func2(char *cmd, char *parms)
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
static void r_lowpixelsize_func2(char *cmd, char *parms)
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
static void r_mirroredweapons_func2(char *cmd, char *parms)
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
static void r_randomstartframes_func2(char *cmd, char *parms)
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
                        const mobjinfo_t    *info = thing->info;

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
                        const mobjinfo_t    *info = thing->info;

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
static void r_screensize_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = parms[0] - '0';

        if (strlen(parms) == 1 && value >= r_screensize_min && value <= r_screensize_max && value != r_screensize)
        {
            r_screensize = value;
            S_StartSound(NULL, sfx_stnmov);
            R_SetViewSize(r_screensize);

            if (!togglingvanilla)
            {
                if (r_hud != (r_screensize == r_screensize_max))
                {
                    r_hud = (r_screensize == r_screensize_max);
                    C_StringCVAROutput(stringize(r_hud), (r_hud ? "on" : "off"));
                }

                if (vid_widescreen && r_screensize < r_screensize_max - 1)
                {
                    vid_widescreen = false;
                    C_StringCVAROutput(stringize(vid_widescreen), "off");
                    I_RestartGraphics(false);
                }
                else if (!vid_widescreen && r_screensize == r_screensize_max)
                {
                    vid_widescreen = true;
                    C_StringCVAROutput(stringize(vid_widescreen), "on");
                    I_RestartGraphics(false);
                }

                M_SaveCVARs();
            }

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
static void r_shadows_translucency_func2(char *cmd, char *parms)
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
static void r_sprites_translucency_func2(char *cmd, char *parms)
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
// r_antialiasing CVAR
//
static void r_antialiasing_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, BOOLVALUEALIAS);

        if ((value == 0 || value == 1) && value != r_antialiasing)
        {
            r_antialiasing = value;
            M_SaveCVARs();
            GetPixelSize();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(r_antialiasing, BOOLVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (r_antialiasing == r_antialiasing_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(r_antialiasing_default, BOOLVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);
        C_ShowWarning(i);
    }
}

//
// r_rockettrails_translucency CVAR
//
static void r_rockettrails_translucency_func2(char *cmd, char *parms)
{
    const bool  r_rockettrails_translucency_old = r_rockettrails_translucency;

    bool_cvars_func2(cmd, parms);

    if (r_rockettrails_translucency != r_rockettrails_translucency_old)
        R_InitColumnFunctions();
}

//
// r_textures CVAR
//
static void r_textures_func2(char *cmd, char *parms)
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
static void r_textures_translucency_func2(char *cmd, char *parms)
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
            musicvolume = (s_musicvolume * 31 + 50) / 100;

            if (consoleactive)
                S_LowerMusicVolume();

            M_SaveCVARs();
        }
        else if (s_sfxvolume != value)
        {
            s_sfxvolume = value;
            sfxvolume = (s_sfxvolume * 31 + 50) / 100;
            S_SetSfxVolume(sfxvolume * (MIX_MAX_VOLUME - 1) / 31);
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
static void savegame_func2(char *cmd, char *parms)
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
static void skilllevel_func2(char *cmd, char *parms)
{
    const int   skilllevel_old = skilllevel;

    int_cvars_func2(cmd, parms);

    if (skilllevel != skilllevel_old)
        NewDef.laston = skilllevel - 1;
}

//
// sucktime CVAR
//
static bool sucktime_func1(char *cmd, char *parms)
{
    return (C_LookupValueFromAlias(parms, SUCKSVALUEALIAS) != INT_MIN || int_cvars_func1(cmd, parms));
}

static void sucktime_func2(char *cmd, char *parms)
{
    const int   value = C_LookupValueFromAlias(parms, SUCKSVALUEALIAS);

    if (value != INT_MIN)
    {
        if (value != sucktime)
        {
            sucktime = value;
            M_SaveCVARs();
        }
    }
    else
        int_cvars_func2(cmd, parms);
}

//
// turbo CVAR
//
static bool turbo_func1(char *cmd, char *parms)
{
    int value;

    if (!*parms)
        return true;

    return ((sscanf(parms, "%10i%%", &value) == 1 || sscanf(parms, "%10i", &value) == 1)
        && value >= turbo_min && value <= turbo_max);
}

static void turbo_func2(char *cmd, char *parms)
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
                stat_cheatsentered = SafeAdd(stat_cheatsentered, 1);
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
static bool units_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, UNITSVALUEALIAS) != INT_MIN);
}

static void units_func2(char *cmd, char *parms)
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
// vid_aspectratio CVAR
//
static bool vid_aspectratio_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, RATIOVALUEALIAS) != INT_MIN);
}

static void vid_aspectratio_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        const int   value = C_LookupValueFromAlias(parms, RATIOVALUEALIAS);

        if (value != INT_MIN && vid_aspectratio != value)
        {
            vid_aspectratio = value;
            I_RestartGraphics(false);
            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();
        }
    }
    else
    {
        char        *temp1 = C_LookupAliasFromValue(vid_aspectratio, RATIOVALUEALIAS);
        const int   i = C_GetIndex(cmd);

        C_ShowDescription(i);

        if (vid_aspectratio == vid_aspectratio_default)
            C_Output(INTEGERCVARISDEFAULT, temp1);
        else
        {
            char    *temp2 = C_LookupAliasFromValue(vid_aspectratio_default, RATIOVALUEALIAS);

            C_Output(INTEGERCVARWITHDEFAULT, temp1, temp2);
            free(temp2);
        }

        free(temp1);

        C_ShowWarning(i);
    }
}

//
// vid_blue CVAR
//
static void vid_blue_func2(char *cmd, char *parms)
{
    const int   vid_blue_old = vid_blue;

    int_cvars_func2(cmd, parms);

    if (vid_blue != vid_blue_old)
        I_UpdateColors();
}

//
// vid_borderlesswindow CVAR
//
static void vid_borderlesswindow_func2(char *cmd, char *parms)
{
    const bool  vid_borderlesswindow_old = vid_borderlesswindow;

    bool_cvars_func2(cmd, parms);

    if (vid_borderlesswindow != vid_borderlesswindow_old && vid_fullscreen)
        I_RestartGraphics(true);
}

//
// vid_brightness CVAR
//
static void vid_brightness_func2(char *cmd, char *parms)
{
    const int   vid_brightness_old = vid_brightness;

    int_cvars_func2(cmd, parms);

    if (vid_brightness != vid_brightness_old)
        I_UpdateColors();
}

//
// vid_capfps CVAR
//
static bool vid_capfps_func1(char *cmd, char *parms)
{
    return (C_LookupValueFromAlias(parms, CAPVALUEALIAS) != INT_MIN || int_cvars_func1(cmd, parms));
}

static void vid_capfps_func2(char *cmd, char *parms)
{
    const int   value = C_LookupValueFromAlias(parms, CAPVALUEALIAS);

    if (value != INT_MIN)
    {
        if (value != vid_capfps)
        {
            vid_capfps = value;
            M_SaveCVARs();
        }
    }
    else
    {
        const int   vid_capfps_old = vid_capfps;

        int_cvars_func2(cmd, parms);

        if (vid_capfps != vid_capfps_old)
        {
            if (vid_capfps)
                vid_capfps = BETWEEN(TICRATE, vid_capfps, vid_capfps_max);

            M_SaveCVARs();
        }
    }
}

//
// vid_contrast CVAR
//
static void vid_contrast_func2(char *cmd, char *parms)
{
    const int   vid_contrast_old = vid_contrast;

    int_cvars_func2(cmd, parms);

    if (vid_contrast != vid_contrast_old)
        I_UpdateColors();
}

//
// vid_display CVAR
//
static void vid_display_func2(char *cmd, char *parms)
{
    const int   vid_display_old = vid_display;

    int_cvars_func2(cmd, parms);

    if (vid_display != vid_display_old)
        I_RestartGraphics(false);
}

//
// vid_fullscreen CVAR
//
static void vid_fullscreen_func2(char *cmd, char *parms)
{
    const bool  vid_fullscreen_old = vid_fullscreen;

    bool_cvars_func2(cmd, parms);

    if (vid_fullscreen != vid_fullscreen_old)
    {
        vid_fullscreen = !vid_fullscreen;
        I_ToggleFullscreen(false);
    }
}

//
// vid_green CVAR
//
static void vid_green_func2(char *cmd, char *parms)
{
    const int   vid_green_old = vid_green;

    int_cvars_func2(cmd, parms);

    if (vid_green != vid_green_old)
        I_UpdateColors();
}

//
// vid_pillarboxes CVAR
//
static void vid_pillarboxes_func2(char *cmd, char *parms)
{
    const bool  vid_pillarboxes_old = vid_pillarboxes;

    bool_cvars_func2(cmd, parms);

    if (vid_pillarboxes != vid_pillarboxes_old)
        I_UpdateColors();
}

//
// vid_red CVAR
//
static void vid_red_func2(char *cmd, char *parms)
{
    const int   vid_red_old = vid_red;

    int_cvars_func2(cmd, parms);

    if (vid_red != vid_red_old)
        I_UpdateColors();
}

//
// vid_saturation CVAR
//
static void vid_saturation_func2(char *cmd, char *parms)
{
    const int   vid_saturation_old = vid_saturation;

    int_cvars_func2(cmd, parms);

    if (vid_saturation != vid_saturation_old)
        I_UpdateColors();
}

//
// vid_scaleapi CVAR
//
static bool vid_scaleapi_func1(char *cmd, char *parms)
{
    return (!*parms
#if defined(_WIN32)
        || M_StringCompare(parms, vid_scaleapi_direct3d9)
        || M_StringCompare(parms, vid_scaleapi_direct3d11)
#endif
        || M_StringCompare(parms, vid_scaleapi_opengl)
#if !defined(_WIN32)
        || M_StringCompare(parms, vid_scaleapi_opengles)
        || M_StringCompare(parms, vid_scaleapi_opengles2)
#endif
        || M_StringCompare(parms, vid_scaleapi_software));
}

static void vid_scaleapi_func2(char *cmd, char *parms)
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
static bool vid_scalefilter_func1(char *cmd, char *parms)
{
    return (!*parms || M_StringCompare(parms, vid_scalefilter_nearest)
        || M_StringCompare(parms, vid_scalefilter_linear)
        || M_StringCompare(parms, vid_scalefilter_nearest_linear));
}

static void vid_scalefilter_func2(char *cmd, char *parms)
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
static void vid_screenresolution_func2(char *cmd, char *parms)
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
static void vid_showfps_func2(char *cmd, char *parms)
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
static bool vid_vsync_func1(char *cmd, char *parms)
{
    return (!*parms || C_LookupValueFromAlias(parms, VSYNCVALUEALIAS) != INT_MIN);
}

static void vid_vsync_func2(char *cmd, char *parms)
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
static void vid_widescreen_func2(char *cmd, char *parms)
{
    const bool  vid_widescreen_old = vid_widescreen;

    bool_cvars_func2(cmd, parms);

    if (vid_widescreen != vid_widescreen_old && !togglingvanilla)
    {
        if (r_screensize != r_screensize_max - 1)
        {
            r_screensize = r_screensize_max - 1;
            C_IntegerCVAROutput(stringize(r_screensize), r_screensize);
            M_SaveCVARs();
        }

        if (r_hud)
        {
            r_hud = false;
            C_StringCVAROutput(stringize(r_hud), "off");
            M_SaveCVARs();
        }

        R_SetViewSize(r_screensize);
        I_RestartGraphics(false);
        S_StartSound(NULL, sfx_stnmov);
    }
}

//
// vid_windowpos CVAR
//
static void vid_windowpos_func2(char *cmd, char *parms)
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
        char        temp[255];

        C_ShowDescription(i);

        M_StringCopy(temp, vid_windowpos, sizeof(temp));

        if (english == english_british)
            M_AmericanToBritishEnglish(temp);

        if (M_StringCompare(vid_windowpos, vid_windowpos_default))
            C_Output(INTEGERCVARISDEFAULT, temp);
        else
            C_Output(INTEGERCVARWITHDEFAULT, temp, vid_windowpos_default);

        C_ShowWarning(i);
    }
}

//
// vid_windowsize CVAR
//
static void vid_windowsize_func2(char *cmd, char *parms)
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
static bool weapon_func1(char *cmd, char *parms)
{
    if (!*parms)
        return true;
    else if (gamestate != GS_LEVEL || viewplayer->pendingweapon != wp_nochange || viewplayer->health <= 0)
        return false;
    else
    {
        int value = INT_MIN;

        if (sscanf(parms, "%1i", &value) == 1)
            for (int i = 0; i < NUMWEAPONS; i++)
                if (value == weaponinfo[i].key - '0')
                {
                    value = i;

                    if (value == wp_fist)
                    {
                        if (viewplayer->readyweapon == wp_fist
                            && viewplayer->weaponowned[wp_chainsaw])
                            value = wp_chainsaw;
                        else if (viewplayer->readyweapon == wp_chainsaw)
                            value = wp_fist;
                    }
                    else if (value == wp_shotgun)
                    {
                        if (viewplayer->readyweapon == wp_shotgun
                            && viewplayer->weaponowned[wp_supershotgun])
                            value = wp_supershotgun;
                        else if (viewplayer->readyweapon == wp_supershotgun)
                            value = wp_shotgun;
                    }

                    break;
                }

        if (value == INT_MIN)
            value = C_LookupValueFromAlias(parms, WEAPONVALUEALIAS);

        return (value != INT_MIN
            && value != viewplayer->readyweapon
            && viewplayer->weaponowned[value]
            && (viewplayer->ammo[weaponinfo[value].ammotype] >= weaponinfo[value].ammopershot
                || weaponinfo[value].ammotype == am_noammo) || infiniteammo);
    }
}

static void weapon_func2(char *cmd, char *parms)
{
    if (*parms)
    {
        int value = INT_MIN;

        if (sscanf(parms, "%1i", &value) == 1)
            for (int i = 0; i < NUMWEAPONS; i++)
                if (value == weaponinfo[i].key - '0')
                {
                    value = i;

                    if (value == wp_fist)
                    {
                        if (viewplayer->readyweapon == wp_fist
                            && viewplayer->weaponowned[wp_chainsaw])
                            value = wp_chainsaw;
                        else if (viewplayer->readyweapon == wp_chainsaw)
                            value = wp_fist;
                    }
                    else if (value == wp_shotgun)
                    {
                        if (viewplayer->readyweapon == wp_shotgun
                            && viewplayer->weaponowned[wp_supershotgun])
                            value = wp_supershotgun;
                        else if (viewplayer->readyweapon == wp_supershotgun)
                            value = wp_shotgun;
                    }

                    break;
                }

        if (value == INT_MIN)
            value = C_LookupValueFromAlias(parms, WEAPONVALUEALIAS);

        viewplayer->pendingweapon = value;

        if (value == wp_fist)
            viewplayer->fistorchainsaw = wp_fist;
        else if (value == wp_chainsaw)
            viewplayer->fistorchainsaw = wp_chainsaw;
        else if (value == wp_shotgun)
            viewplayer->preferredshotgun = wp_shotgun;
        else if (value == wp_supershotgun)
            viewplayer->preferredshotgun = wp_supershotgun;

        C_HideConsole();
    }
    else
    {
        char    *temp = C_LookupAliasFromValue((gamestate == GS_LEVEL ? viewplayer->readyweapon : weapon_default), WEAPONVALUEALIAS);
        char    description[255];

        if (gamemode == shareware)
            M_StringCopy(description, WEAPONDESCRIPTION_SHAREWARE, sizeof(description));
        else if (gamemission != doom)
            M_StringCopy(description, WEAPONDESCRIPTION_DOOM2, sizeof(description));
        else
            M_StringCopy(description, consolecmds[C_GetIndex(cmd)].description, sizeof(description));

        description[0] = tolower(description[0]);

        C_Output("This CVAR changes %s", description);
        C_Output(INTEGERCVARWITHNODEFAULT, temp);

        if (gamestate != GS_LEVEL)
        {
            if (M_StringCompare(playername, playername_default))
                C_Warning(0, NOGAMECVARWARNING1);
            else
                C_Warning(0, NOGAMECVARWARNING2, playername, pronoun(personal),
                    (playergender == playergender_other ? "aren't" : "isn't"));
        }

        free(temp);
    }
}

//
// weaponrecoil CVAR
//
static void weaponrecoil_func2(char *cmd, char *parms)
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
