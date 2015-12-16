/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <XInput.h>
#endif

#include "am_map.h"
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
#define MAX_PATH                260
#endif

#define GIVECMDSHORTFORMAT      "~items~"
#define GIVECMDLONGFORMAT       "ammo|armor|health|keys|weapons|all|~item~"
#define KILLCMDFORMAT           "player|all|~monster~"
#define MAPCMDFORMAT            "E~x~M~y~|MAP~xy~"
#define SPAWNCMDFORMAT          "~monster~|~item~"

extern dboolean         alwaysrun;
extern int              am_allmapcdwallcolor;
extern int              am_allmapfdwallcolor;
extern int              am_allmapwallcolor;
extern int              am_backcolor;
extern int              am_cdwallcolor;
extern int              am_cheat;
extern dboolean         am_external;
extern int              am_fdwallcolor;
extern dboolean         am_grid;
extern int              am_gridcolor;
extern int              am_markcolor;
extern int              am_playercolor;
extern dboolean         am_rotatemode;
extern int              am_teleportercolor;
extern int              am_thingcolor;
extern int              am_tswallcolor;
extern int              am_wallcolor;
extern int              am_xhaircolor;
extern dboolean         centerweapon;
extern dboolean         con_obituaries;
extern dboolean         con_timestamps;
extern char             *configfile;
extern int              episode;
extern int              expansion;
extern int              faceback;
extern int              gametime;
extern float            gp_deadzone_left;
extern float            gp_deadzone_right;
extern int              gp_sensitivity;
extern dboolean         gp_swapthumbsticks;
extern dboolean         gp_vibrate;
extern char             *iwadfolder;
extern dboolean         messages;
extern float            m_acceleration;
extern dboolean         m_doubleclick_use;
extern dboolean         m_novertical;
extern int              m_sensitivity;
extern int              m_threshold;
extern int              movebob;
extern char             *playername;
extern dboolean         r_althud;
extern int              r_blood;
extern int              r_bloodsplats_max;
extern int              r_bloodsplats_total;
extern dboolean         r_brightmaps;
extern dboolean         r_corpses_color;
extern dboolean         r_corpses_mirrored;
extern dboolean         r_corpses_moreblood;
extern dboolean         r_corpses_nudge;
extern dboolean         r_corpses_slide;
extern dboolean         r_corpses_smearblood;
extern int              r_detail;
extern int              r_diskicon;
extern dboolean         r_fixmaperrors;
extern dboolean         r_fixspriteoffsets;
extern dboolean         r_floatbob;
extern float            r_gamma;
extern dboolean         r_homindicator;
extern dboolean         r_hud;
extern dboolean         r_liquid_bob;
extern dboolean         r_liquid_clipsprites;
extern dboolean         r_liquid_current;
extern dboolean         r_liquid_lowerview;
extern dboolean         r_liquid_swirl;
extern char             *r_lowpixelsize;
extern dboolean         r_mirroredweapons;
extern dboolean         r_playersprites;
extern dboolean         r_rockettrails;
extern int              r_screensize;
extern dboolean         r_shadows;
extern dboolean         r_shakescreen;
extern dboolean         r_translucency;
extern int              s_musicvolume;
extern dboolean         s_randommusic;
extern dboolean         s_randompitch;
extern int              s_sfxvolume;
extern char             *s_timiditycfgpath;
extern int              skilllevel;
extern unsigned int     stat_cheated;
extern unsigned int     stat_damageinflicted;
extern unsigned int     stat_damagereceived;
extern unsigned int     stat_deaths;
extern unsigned int     stat_itemspickedup;
extern unsigned int     stat_monsterskilled;
extern unsigned int     stat_monsterskilled_arachnotrons;
extern unsigned int     stat_monsterskilled_archviles;
extern unsigned int     stat_monsterskilled_baronsofhell;
extern unsigned int     stat_monsterskilled_cacodemons;
extern unsigned int     stat_monsterskilled_cyberdemons;
extern unsigned int     stat_monsterskilled_demons;
extern unsigned int     stat_monsterskilled_heavyweapondudes;
extern unsigned int     stat_monsterskilled_hellknights;
extern unsigned int     stat_monsterskilled_imps;
extern unsigned int     stat_monsterskilled_lostsouls;
extern unsigned int     stat_monsterskilled_mancubi;
extern unsigned int     stat_monsterskilled_painelementals;
extern unsigned int     stat_monsterskilled_revenants;
extern unsigned int     stat_monsterskilled_shotgunguys;
extern unsigned int     stat_monsterskilled_spectres;
extern unsigned int     stat_monsterskilled_spidermasterminds;
extern unsigned int     stat_monsterskilled_zombiemen;
extern unsigned int     stat_secretsrevealed;
extern unsigned int     stat_shotsfired;
extern unsigned int     stat_shotshit;
extern unsigned int     stat_time;
extern int              stillbob;
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

extern int              gamepadalwaysrun;
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
extern int              mousebfire;
extern int              mousebforward;
extern int              mousebprevweapon;
extern int              mousebnextweapon;
extern int              mousebstrafe;
extern int              mousebuse;

extern int              pixelwidth;
extern int              pixelheight;
extern int              screenheight;
extern int              screenwidth;

control_t controls[] =
{
    { "1",             keyboardcontrol, '1'             }, { "2",             keyboardcontrol, '2'             },
    { "3",             keyboardcontrol, '3'             }, { "4",             keyboardcontrol, '4'             },
    { "5",             keyboardcontrol, '5'             }, { "6",             keyboardcontrol, '6'             },
    { "7",             keyboardcontrol, '7'             }, { "8",             keyboardcontrol, '8'             },
    { "9",             keyboardcontrol, '9'             }, { "0",             keyboardcontrol, '0'             },
    { "-",             keyboardcontrol, KEY_MINUS       }, { "=",             keyboardcontrol, KEY_EQUALS      },
    { "+",             keyboardcontrol, KEY_EQUALS      }, { "backspace",     keyboardcontrol, KEY_BACKSPACE   },
    { "tab",           keyboardcontrol, KEY_TAB         }, { "q",             keyboardcontrol, 'q'             },
    { "w",             keyboardcontrol, 'w'             }, { "e",             keyboardcontrol, 'e'             },
    { "r",             keyboardcontrol, 'r'             }, { "t",             keyboardcontrol, 't'             },
    { "y",             keyboardcontrol, 'y'             }, { "u",             keyboardcontrol, 'u'             },
    { "i",             keyboardcontrol, 'i'             }, { "o",             keyboardcontrol, 'o'             },
    { "p",             keyboardcontrol, 'p'             }, { "[",             keyboardcontrol, '['             },
    { "]",             keyboardcontrol, ']'             }, { "enter",         keyboardcontrol, KEY_ENTER       },
    { "ctrl",          keyboardcontrol, KEY_RCTRL       }, { "a",             keyboardcontrol, 'a'             },
    { "s",             keyboardcontrol, 's'             }, { "d",             keyboardcontrol, 'd'             },
    { "f",             keyboardcontrol, 'f'             }, { "g",             keyboardcontrol, 'g'             },
    { "h",             keyboardcontrol, 'h'             }, { "j",             keyboardcontrol, 'j'             },
    { "k",             keyboardcontrol, 'k'             }, { "l",             keyboardcontrol, 'l'             },
    { ";",             keyboardcontrol, ';'             }, { "\'",            keyboardcontrol, '\''            },
    { "shift",         keyboardcontrol, KEY_RSHIFT      }, { "\\",            keyboardcontrol, '\\'            },
    { "z",             keyboardcontrol, 'z'             }, { "x",             keyboardcontrol, 'x'             },
    { "c",             keyboardcontrol, 'c'             }, { "v",             keyboardcontrol, 'v'             },
    { "b",             keyboardcontrol, 'b'             }, { "n",             keyboardcontrol, 'n'             },
    { "m",             keyboardcontrol, 'm'             }, { ",",             keyboardcontrol, ','             },
    { ".",             keyboardcontrol, '.'             }, { "/",             keyboardcontrol, '/'             },
    { "alt",           keyboardcontrol, KEY_RALT        }, { "space",         keyboardcontrol, ' '             },
    { "numlock",       keyboardcontrol, KEY_NUMLOCK     }, { "capslock",      keyboardcontrol, KEY_CAPSLOCK    },
    { "scrolllock",    keyboardcontrol, KEY_SCRLCK      }, { "home",          keyboardcontrol, KEY_HOME        },
    { "up",            keyboardcontrol, KEY_UPARROW     }, { "pageup",        keyboardcontrol, KEY_PGUP        },
    { "left",          keyboardcontrol, KEY_LEFTARROW   }, { "right",         keyboardcontrol, KEY_RIGHTARROW  },
    { "end",           keyboardcontrol, KEY_END         }, { "down",          keyboardcontrol, KEY_DOWNARROW   },
    { "pagedown",      keyboardcontrol, KEY_PGDN        }, { "insert",        keyboardcontrol, KEY_INS         },
    { "delete",        keyboardcontrol, KEY_DEL         }, { "mouse1",        mousecontrol,    0               },
    { "mouse2",        mousecontrol,    1               }, { "mouse3",        mousecontrol,    2               },
    { "mouse4",        mousecontrol,    3               }, { "mouse5",        mousecontrol,    4               },
    { "mouse6",        mousecontrol,    5               }, { "mouse7",        mousecontrol,    6               },
    { "mouse8",        mousecontrol,    7               }, { "mouse9",        mousecontrol,    8               },
    { "mouse10",       mousecontrol,    9               }, { "mouse11",       mousecontrol,    10              },
    { "mouse12",       mousecontrol,    11              }, { "mouse13",       mousecontrol,    12              },
    { "mouse14",       mousecontrol,    13              }, { "mouse15",       mousecontrol,    14              },
    { "mouse16",       mousecontrol,    15              }, { "wheelup",       mousecontrol,    MOUSE_WHEELUP   },
    { "wheeldown",     mousecontrol,    MOUSE_WHEELDOWN }, { "dpadup",        gamepadcontrol,  1               },
    { "dpaddown",      gamepadcontrol,  2               }, { "dpadleft",      gamepadcontrol,  4               },
    { "dpadright",     gamepadcontrol,  8               }, { "start",         gamepadcontrol,  16              },
    { "back",          gamepadcontrol,  32              }, { "leftthumb",     gamepadcontrol,  64              },
    { "rightthumb",    gamepadcontrol,  128             }, { "leftshoulder",  gamepadcontrol,  256             },
    { "rightshoulder", gamepadcontrol,  512             }, { "lefttrigger",   gamepadcontrol,  1024            },
    { "righttrigger",  gamepadcontrol,  2048            }, { "gamepad1",      gamepadcontrol,  4096            },
    { "gamepad2",      gamepadcontrol,  8192            }, { "gamepad3",      gamepadcontrol,  16384           },
    { "gamepad4",      gamepadcontrol,  32768           }, { "",              0,               0               }
};

action_t actions[] =
{
    { "+alwaysrun",    &key_alwaysrun,          NULL,              &gamepadalwaysrun         },
    { "+automap",      &key_automap,            NULL,              &gamepadautomap           },
    { "+back",         &key_down,               NULL,              NULL                      },
    { "+back2",        &key_down2,              NULL,              NULL                      },
    { "+clearmark",    &key_automap_clearmark,  NULL,              &gamepadautomapclearmark  },
    { "+fire",         &key_fire,               &mousebfire,       &gamepadfire              },
    { "+followmode",   &key_automap_followmode, NULL,              &gamepadautomapfollowmode },
    { "+forward",      &key_up,                 &mousebforward,    NULL                      },
    { "+forward2",     &key_up2,                NULL,              NULL                      },
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

static dboolean cheat_func1(char *, char *, char *, char *);
static dboolean game_func1(char *, char *, char *, char *);
static dboolean null_func1(char *, char *, char *, char *);

static void clear_cmd_func2(char *, char *, char *, char *);
static void cmdlist_cmd_func2(char *, char *, char *, char *);
static void condump_cmd_func2(char *, char *, char *, char *);
static void cvarlist_cmd_func2(char *, char *, char *, char *);
static void endgame_cmd_func2(char *, char *, char *, char *);
static void exitmap_cmd_func2(char *, char *, char *, char *);
static dboolean give_cmd_func1(char *, char *, char *, char *);
static void give_cmd_func2(char *, char *, char *, char *);
static dboolean god_cmd_func1(char *, char *, char *, char *);
static void god_cmd_func2(char *, char *, char *, char *);
static void help_cmd_func2(char *, char *, char *, char *);
static dboolean kill_cmd_func1(char *, char *, char *, char *);
static void kill_cmd_func2(char *, char *, char *, char *);
static dboolean load_cmd_func1(char *, char *, char *, char *);
static void load_cmd_func2(char *, char *, char *, char *);
static dboolean map_cmd_func1(char *, char *, char *, char *);
static void map_cmd_func2(char *, char *, char *, char *);
static void maplist_cmd_func2(char *, char *, char *, char *);
static void mapstats_cmd_func2(char *, char *, char *, char *);
static void noclip_cmd_func2(char *, char *, char *, char *);
static void nomonsters_cmd_func2(char *, char *, char *, char *);
static void notarget_cmd_func2(char *, char *, char *, char *);
static void playerstats_cmd_func2(char *, char *, char *, char *);
static void quit_cmd_func2(char *, char *, char *, char *);
static dboolean resurrect_cmd_func1(char *, char *, char *, char *);
static void resurrect_cmd_func2(char *, char *, char *, char *);
static dboolean save_cmd_func1(char *, char *, char *, char *);
static void save_cmd_func2(char *, char *, char *, char *);
static dboolean spawn_cmd_func1(char *, char *, char *, char *);
static void spawn_cmd_func2(char *, char *, char *, char *);
static void thinglist_cmd_func2(char *, char *, char *, char *);
static void unbind_cmd_func2(char *, char *, char *, char *);

static dboolean bool_cvars_func1(char *, char *, char *, char *);
static void bool_cvars_func2(char *, char *, char *, char *);
static void color_cvars_func2(char *, char *, char *, char *);
static dboolean float_cvars_func1(char *, char *, char *, char *);
static void float_cvars_func2(char *, char *, char *, char *);
static dboolean int_cvars_func1(char *, char *, char *, char *);
static void int_cvars_func2(char *, char *, char *, char *);
static void str_cvars_func2(char *, char *, char *, char *);
static void time_cvars_func2(char *, char *, char *, char *);

static void am_cheat_cvar_func2(char *, char *, char *, char *);
static void am_external_cvar_func2(char *, char *, char *, char *);
static dboolean gp_deadzone_cvars_func1(char *, char *, char *, char *);
static void gp_deadzone_cvars_func2(char *, char *, char *, char *);
static void playername_cvar_func2(char *, char *, char *, char *);
static void alwaysrun_cvar_func2(char *, char *, char *, char *);
static dboolean r_blood_cvar_func1(char *, char *, char *, char *);
static void r_blood_cvar_func2(char *, char *, char *, char *);
static dboolean r_detail_cvar_func1(char *, char *, char *, char *);
static void r_detail_cvar_func2(char *, char *, char *, char *);
static dboolean r_gamma_cvar_func1(char *, char *, char *, char *);
static void r_gamma_cvar_func2(char *, char *, char *, char *);
static void r_hud_cvar_func2(char *, char *, char *, char *);
static void r_lowpixelsize_cvar_func2(char *, char *, char *, char *);
static void r_screensize_cvar_func2(char *, char *, char *, char *);
static dboolean s_volume_cvars_func1(char *, char *, char *, char *);
static void s_volume_cvars_func2(char *, char *, char *, char *);
static void vid_display_cvar_func2(char *, char *, char *, char *);
static void vid_fullscreen_cvar_func2(char *, char *, char *, char *);
static dboolean vid_scaledriver_cvar_func1(char *, char *, char *, char *);
static void vid_scaledriver_cvar_func2(char *, char *, char *, char *);
static dboolean vid_scalefilter_cvar_func1(char *, char *, char *, char *);
static void vid_scalefilter_cvar_func2(char *, char *, char *, char *);
static void vid_screenresolution_cvar_func2(char *, char *, char *, char *);
static void vid_showfps_cvar_func2(char *, char *, char *, char *);
static void vid_vsync_cvar_func2(char *, char *, char *, char *);
static void vid_widescreen_cvar_func2(char *, char *, char *, char *);
static void vid_windowposition_cvar_func2(char *, char *, char *, char *);
static void vid_windowsize_cvar_func2(char *, char *, char *, char *);

void C_Bind(char *, char *, char *, char *);

static int C_LookupValueFromAlias(const char *text, int aliastype)
{
    int i = 0;

    while (aliases[i].text[0])
    {
        if (aliastype == aliases[i].type && M_StringCompare(text, aliases[i].text))
            return aliases[i].value;
        ++i;
    }
    return -1;
}

static char *C_LookupAliasFromValue(int value, alias_type_t aliastype)
{
    int i = 0;

    while (aliases[i].text[0])
    {
        if (aliastype == aliases[i].type && value == aliases[i].value)
            return aliases[i].text;
        ++i;
    }
    return commify(value);
}

#define CMD(name, alt, cond, func, parms, form, desc) \
    { #name, #alt, cond, func, parms, CT_CMD, CF_NONE, NULL, 0, 0, 0, form, desc }
#define CMD_CHEAT(name, parms) \
    { #name, "", cheat_func1, NULL, parms, CT_CHEAT, CF_NONE, NULL, 0, 0, 0, "", "" }
#define CVAR_BOOL(name, alt, cond, func, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, CF_BOOLEAN, &name, 1, false, true, "", desc }
#define CVAR_INT(name, alt, cond, func, flags, aliases, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, (CF_INTEGER | flags), &name, aliases, name##_min, \
      name##_max, "", desc }
#define CVAR_FLOAT(name, alt, cond, func, flags, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, (CF_FLOAT | flags), &name, 0, 0, 0, "", desc }
#define CVAR_POS(name, alt, cond, func, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, CF_POSITION, &name, 0, 0, 0, "", desc }
#define CVAR_SIZE(name, alt, cond, func, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, CF_SIZE, &name, 0, 0, 0, "", desc }
#define CVAR_STR(name, alt, cond, func, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, CF_STRING, &name, 0, 0, 0, "", desc }
#define CVAR_TIME(name, alt, cond, func, desc) \
    { #name, #alt, cond, func, 1, CT_CVAR, (CF_TIME | CF_READONLY), &name, 0, 0, 0, "", desc }

int     numconsolecmds;

consolecmd_t consolecmds[] =
{
    // console commands
    CMD       (bind, "", null_func1, C_Bind, 2, "[~control~ [+~action~]]", "Binds an action to a control."),
    CMD       (clear, "", null_func1, clear_cmd_func2, 0, "", "Clears the console."),
    CMD       (cmdlist, "", null_func1, cmdlist_cmd_func2, 1, "[~searchstring~]", "Shows a list of console commands."),
    CMD       (condump, "", null_func1, condump_cmd_func2, 1, "[~filename~.txt]", "Dumps the console to a file."),
    CMD       (cvarlist, "", null_func1, cvarlist_cmd_func2, 1, "[~searchstring~]", "Shows a list of console variables."),
    CMD       (endgame, "", game_func1, endgame_cmd_func2, 0, "", "Ends a game."),
    CMD       (exitmap, "", game_func1, exitmap_cmd_func2, 0, "", "Exits the current map."),
    CMD       (give, "", give_cmd_func1, give_cmd_func2, 1, GIVECMDSHORTFORMAT, "Gives items to the player."),
    CMD       (god, "", god_cmd_func1, god_cmd_func2, 1, "[on|off]", "Toggles god mode."),
    CMD       (help, "", null_func1, help_cmd_func2, 0, "", "Shows the help screen."),
    CMD       (kill, "", kill_cmd_func1, kill_cmd_func2, 1, KILLCMDFORMAT, "Kills the player, all monsters or a type of monster."),
    CMD       (load, "", load_cmd_func1, load_cmd_func2, 1, "~filename~.save", "Loads a game from a file."),
    CMD       (map, warp, map_cmd_func1, map_cmd_func2, 1, MAPCMDFORMAT, "Warps to a map."),
    CMD       (maplist, "", null_func1, maplist_cmd_func2, 0, "", "Shows a list of the available maps."),
    CMD       (mapstats, "", game_func1, mapstats_cmd_func2, 0, "", "Shows statistics about the current map."),
    CMD       (noclip, "", game_func1, noclip_cmd_func2, 1, "[on|off]", "Toggles collision detection for the player."),
    CMD       (nomonsters, "", null_func1, nomonsters_cmd_func2, 1, "[on|off]", "Toggles the presence of monsters in maps."),
    CMD       (notarget, "", game_func1, notarget_cmd_func2, 1, "[on|off]", "Toggles the player as a target."),
    CMD       (playerstats, "", null_func1, playerstats_cmd_func2, 0, "", "Shows statistics about the player."),
    CMD       (quit, exit, null_func1, quit_cmd_func2, 0, "", "Quits ~"PACKAGE_NAME"~."),
    CMD       (resurrect, "", resurrect_cmd_func1, resurrect_cmd_func2, 0, "", "Resurrects the player."),
    CMD       (save, "", save_cmd_func1, save_cmd_func2, 1, "~filename~.save", "Saves the game to a file."),
    CMD       (spawn, "", spawn_cmd_func1, spawn_cmd_func2, 1, SPAWNCMDFORMAT, "Spawns a monster or item."),
    CMD       (summon, "", spawn_cmd_func1, spawn_cmd_func2, 1, "", ""),
    CMD       (thinglist, "", game_func1, thinglist_cmd_func2, 0, "", "Shows a list of things in the current map."),
    CMD       (unbind, "", null_func1, unbind_cmd_func2, 1, "~control~", "Unbinds the action from a control."),

    // cheats
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

    // console variables
    CVAR_BOOL (alwaysrun, "", bool_cvars_func1, alwaysrun_cvar_func2, "Toggles the player always running when moving."),
    CVAR_INT  (am_allmapcdwallcolor, am_allmapcdwallcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of lines with a change in ceiling height in the automap\nwhen using the computer area map power-up."),
    CVAR_INT  (am_allmapfdwallcolor, am_allmapfdwallcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of lines with a change in floor height in the automap\nwhen using the computer area map power-up."),
    CVAR_INT  (am_allmapwallcolor, am_allmapwallcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of solid walls in the automap when using the computer\narea map power-up."),
    CVAR_INT  (am_backcolor, am_backcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of the background in the automap."),
    CVAR_INT  (am_cdwallcolor, am_cdwallcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of lines with a change in ceiling height in the automap."),
    CVAR_INT  (am_cheat, "", int_cvars_func1, am_cheat_cvar_func2, CF_NONE, CHEATALIAS, "The status of the automap cheat (none, things or all)."),
    CVAR_BOOL (am_external, "", bool_cvars_func1, am_external_cvar_func2, "Toggles rendering of the automap on an external display."),
    CVAR_INT  (am_fdwallcolor, am_fdwallcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of lines with a change in floor height in the automap."),
    CVAR_BOOL (am_followmode, "", bool_cvars_func1, bool_cvars_func2, "Toggles follow mode in the automap."),
    CVAR_BOOL (am_grid, "", bool_cvars_func1, bool_cvars_func2, "Toggles the grid in the automap."),
    CVAR_INT  (am_gridcolor, am_gridcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of the grid in the automap."),
    CVAR_INT  (am_markcolor, am_markcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of marks in the automap."),
    CVAR_INT  (am_playercolor, am_playercolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of the player arrow in the automap."),
    CVAR_BOOL (am_rotatemode, "", bool_cvars_func1, bool_cvars_func2, "Toggles rotate mode in the automap."),
    CVAR_INT  (am_teleportercolor, am_teleportercolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of teleporter lines in the automap."),
    CVAR_INT  (am_thingcolor, am_thingcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of thing triangles in the automap."),
    CVAR_INT  (am_tswallcolor, am_tswallcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of lines with no change in height in the automap."),
    CVAR_INT  (am_wallcolor, am_wallcolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of solid walls in the automap."),
    CVAR_INT  (am_xhaircolor, am_xhaircolour, int_cvars_func1, color_cvars_func2, CF_NONE, NOALIAS, "The color of the crosshair in the automap."),
    CVAR_BOOL (centerweapon, "", bool_cvars_func1, bool_cvars_func2, "Toggles the centering of the player's weapon when firing."),
    CVAR_BOOL (con_obituaries, "", bool_cvars_func1, bool_cvars_func2, "Toggles obituaries in the console when monsters are killed."),
    CVAR_BOOL (con_timestamps, "", bool_cvars_func1, bool_cvars_func2, "Toggles timestamps in the console for player messages."),
    CVAR_STR  (configfile, "", null_func1, str_cvars_func2, "The path of the configuration file."),
    CVAR_INT  (episode, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOALIAS, "The currently selected ~DOOM~ episode in the menu."),
    CVAR_INT  (expansion, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOALIAS, "The currently selected ~DOOM II~ expansion in the menu."),
    CVAR_INT  (faceback, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOALIAS, "The color behind the player's face in the status bar."),
    CVAR_TIME (gametime, "", null_func1, time_cvars_func2, "The amount of time since ~"PACKAGE_NAME"~ started."),
    CVAR_FLOAT(gp_deadzone_left, "", gp_deadzone_cvars_func1, gp_deadzone_cvars_func2, CF_PERCENT, "The dead zone of the gamepad's left thumbstick."),
    CVAR_FLOAT(gp_deadzone_right, "", gp_deadzone_cvars_func1, gp_deadzone_cvars_func2, CF_PERCENT, "The dead zone of the gamepad's right thumbstick."),
    CVAR_INT  (gp_sensitivity, "", null_func1, int_cvars_func2, CF_NONE, NOALIAS, "The gamepad's sensitivity."),
    CVAR_BOOL (gp_swapthumbsticks, "", bool_cvars_func1, bool_cvars_func2, "Toggles swapping the gamepad's left and right thumbsticks."),
    CVAR_BOOL (gp_vibrate, "", bool_cvars_func1, bool_cvars_func2, "Toggles vibration for XInput gamepads."),
    CVAR_STR  (iwadfolder, "", null_func1, str_cvars_func2, "The folder where an IWAD file was last opened."),
    CVAR_FLOAT(m_acceleration, "", float_cvars_func1, float_cvars_func2, CF_NONE, "The amount the mouse accelerates."),
    CVAR_BOOL (m_doubleclick_use, "", bool_cvars_func1, bool_cvars_func2, "Toggles double-clicking a mouse button for the +use action."),
    CVAR_BOOL (m_novertical, "", bool_cvars_func1, bool_cvars_func2, "Toggles no vertical movement of the mouse."),
    CVAR_INT  (m_sensitivity, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOALIAS, "The mouse's sensitivity."),
    CVAR_INT  (m_threshold, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOALIAS, "The mouse's acceleration threshold."),
    CVAR_BOOL (messages, "", bool_cvars_func1, bool_cvars_func2, "Toggles messages."),
    CVAR_INT  (movebob, "", null_func1, int_cvars_func2, CF_PERCENT, NOALIAS, "The amount the player bobs when moving."),
    CVAR_STR  (playername, "", null_func1, playername_cvar_func2, "The name of the player used in messages."),
    CVAR_BOOL (r_althud, "", bool_cvars_func1, bool_cvars_func2, "Toggles the display of an alternate HUD when the HUD is on."),
    CVAR_INT  (r_blood, "", r_blood_cvar_func1, r_blood_cvar_func2, CF_NONE, BLOODALIAS, "The color of the blood of the player and monsters (all, none or\nred)."),
    CVAR_INT  (r_bloodsplats_max, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOALIAS, "The maximum number of blood splats allowed in a map (0 to\n1048576)."),
    CVAR_INT  (r_bloodsplats_total, "", int_cvars_func1, int_cvars_func2, CF_READONLY, NOALIAS, "The total number of blood splats in the current map."),
    CVAR_BOOL (r_brightmaps, "", bool_cvars_func1, bool_cvars_func2, "Toggles brightmaps on certain wall textures."),
    CVAR_BOOL (r_corpses_color, "", bool_cvars_func1, bool_cvars_func2, "Toggles corpses of marines being randomly colored."),
    CVAR_BOOL (r_corpses_mirrored, "", bool_cvars_func1, bool_cvars_func2, "Toggles corpses being randomly mirrored."),
    CVAR_BOOL (r_corpses_moreblood, "", bool_cvars_func1, bool_cvars_func2, "Toggles blood splats around corpses when a map is loaded."),
    CVAR_BOOL (r_corpses_nudge, "", bool_cvars_func1, bool_cvars_func2, "Toggles corpses being nudged when monsters walk over them."),
    CVAR_BOOL (r_corpses_slide, "", bool_cvars_func1, bool_cvars_func2, "Toggles corpses reacting to barrel and rocket explosions."),
    CVAR_BOOL (r_corpses_smearblood, "", bool_cvars_func1, bool_cvars_func2, "Toggles corpses producing blood splats as they slide."),
    CVAR_BOOL (r_detail, "", r_detail_cvar_func1, r_detail_cvar_func2, "Toggles the graphic detail (low or high)."),
    CVAR_BOOL (r_diskicon, "", bool_cvars_func1, bool_cvars_func2, "Toggles showing a disk icon when loading and saving."),
    CVAR_BOOL (r_fixmaperrors, "", bool_cvars_func1, bool_cvars_func2, "Toggles the fixing of mapping errors in the ~DOOM~ IWADs."),
    CVAR_BOOL (r_fixspriteoffsets, "", bool_cvars_func1, bool_cvars_func2, "Toggles the fixing of sprite offsets."),
    CVAR_BOOL (r_floatbob, "", bool_cvars_func1, bool_cvars_func2, "Toggles powerups bobbing up and down."),
    CVAR_FLOAT(r_gamma, "", r_gamma_cvar_func1, r_gamma_cvar_func2, CF_NONE, "The gamma correction level (off or 0.50 to 2.0)."),
    CVAR_BOOL (r_homindicator, "", bool_cvars_func1, bool_cvars_func2, "Toggles the flashing \"Hall of Mirrors\" indicator."),
    CVAR_BOOL (r_hud, "", bool_cvars_func1, r_hud_cvar_func2, "Toggles the heads-up display when in widescreen mode."),
    CVAR_BOOL (r_liquid_bob, "", bool_cvars_func1, bool_cvars_func2, "Toggles the bobbing effect of liquid sectors."),
    CVAR_BOOL (r_liquid_clipsprites, "", bool_cvars_func1, bool_cvars_func2, "Toggles the bottom of sprites being clipped in liquid sectors."),
    CVAR_BOOL (r_liquid_current, "", bool_cvars_func1, bool_cvars_func2, "Toggles a slight current being applied to liquid sectors."),
    CVAR_BOOL (r_liquid_lowerview, "", bool_cvars_func1, bool_cvars_func2, "Toggles lowering the player's view in liquid sectors."),
    CVAR_BOOL (r_liquid_swirl, "", bool_cvars_func1, bool_cvars_func2, "Toggles the swirl effect of liquid sectors."),
    CVAR_SIZE (r_lowpixelsize, "", null_func1, r_lowpixelsize_cvar_func2, "The size of pixels when the graphic detail is low (~width~\xD7~height~)."),
    CVAR_BOOL (r_mirroredweapons, "", bool_cvars_func1, bool_cvars_func2, "Toggles randomly mirroring weapons dropped by monsters."),
    CVAR_BOOL (r_playersprites, "", bool_cvars_func1, bool_cvars_func2,"Toggles the display of the player's weapon."),
    CVAR_BOOL (r_rockettrails, "", bool_cvars_func1, bool_cvars_func2, "Toggles rocket trails behind player and Cyberdemon rockets."),
    CVAR_INT  (r_screensize, "", int_cvars_func1, r_screensize_cvar_func2, CF_NONE, NOALIAS, "The screen size."),
    CVAR_BOOL (r_shadows, "", bool_cvars_func1, bool_cvars_func2, "Toggles sprites casting shadows."),
    CVAR_BOOL (r_shakescreen, "", bool_cvars_func1, bool_cvars_func2, "Toggles shaking of the screen when the player is injured."),
    CVAR_BOOL (r_translucency, "", bool_cvars_func1, bool_cvars_func2, "Toggles translucency in sprites and textures."),
    CVAR_INT  (s_musicvolume, "", s_volume_cvars_func1, s_volume_cvars_func2, CF_PERCENT,  NOALIAS, "The music volume."),
    CVAR_BOOL (s_randommusic, "", bool_cvars_func1, bool_cvars_func2, "Toggles the randomizing of music at the start of each map."),
    CVAR_BOOL (s_randompitch, "", bool_cvars_func1, bool_cvars_func2, "Toggles randomizing the pitch of monster sound effects."),
    CVAR_INT  (s_sfxvolume, "", s_volume_cvars_func1, s_volume_cvars_func2, CF_PERCENT, NOALIAS, "The sound effects volume."),
    CVAR_STR  (s_timiditycfgpath, "", null_func1, str_cvars_func2, "The path of Timidity's configuration file."),
    CVAR_INT  (skilllevel, "", int_cvars_func1, int_cvars_func2, CF_NONE, NOALIAS, "The currently selected skill level in the menu."),
    CVAR_INT  (stillbob, "", null_func1, int_cvars_func2, CF_PERCENT, NOALIAS, "The amount the player bobs when still."),
    CVAR_BOOL (vid_capfps, "", bool_cvars_func1, bool_cvars_func2, "Toggles capping of the framerate at 35 FPS."),
    CVAR_INT  (vid_display, "", int_cvars_func1, vid_display_cvar_func2, CF_NONE, NOALIAS, "The display used to render the game."),
#if !defined(WIN32)
    CVAR_STR  (vid_driver, "", null_func1, str_cvars_func2, "The video driver used to render the game."),
#endif
    CVAR_BOOL (vid_fullscreen, "", bool_cvars_func1, vid_fullscreen_cvar_func2, "Toggles between fullscreen and a window."),
    CVAR_STR  (vid_scaledriver, "", vid_scaledriver_cvar_func1, vid_scaledriver_cvar_func2, "The driver used to scale the display (\"\", \"direct3d\", \"opengl\"\nor \"software\")."),
    CVAR_STR  (vid_scalefilter, "", vid_scalefilter_cvar_func1, vid_scalefilter_cvar_func2, "The filter used to scale the display (\"nearest\", \"linear\" or\n\"nearest_linear\")."),
    CVAR_SIZE (vid_screenresolution, "", null_func1, vid_screenresolution_cvar_func2, "The screen's resolution when fullscreen (desktop or\n~width~\xD7~height~)."),
    CVAR_BOOL (vid_showfps, "", bool_cvars_func1, vid_showfps_cvar_func2, "Toggles the display of the average frames per second."),
    CVAR_BOOL (vid_vsync, "", bool_cvars_func1, vid_vsync_cvar_func2, "Toggles vertical synchronization with display's refresh rate."),
    CVAR_BOOL (vid_widescreen, "", bool_cvars_func1, vid_widescreen_cvar_func2, "Toggles widescreen mode."),
    CVAR_POS  (vid_windowposition, "", null_func1, vid_windowposition_cvar_func2, "The position of the window on the desktop (centered or\n(~width~,~height~))."),
    CVAR_SIZE (vid_windowsize, "", null_func1, vid_windowsize_cvar_func2, "The size of the window on the desktop (~width~\xD7~height~)."),

    { "", "", null_func1, NULL, 0, 0, CF_NONE, NULL, 0, 0, 0, "", "" }
};

static dboolean cheat_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (gamestate != GS_LEVEL)
        return false;
    else if (M_StringCompare(cmd, cheat_god.sequence))
        return (gameskill != sk_nightmare);
    else if (M_StringCompare(cmd, cheat_ammonokey.sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_ammo.sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_mus.sequence))
        return (!nomusic && musicVolume);
    else if (M_StringCompare(cmd, cheat_noclip.sequence))
        return (gamemode != commercial && gameskill != sk_nightmare);
    else if (M_StringCompare(cmd, cheat_commercial_noclip.sequence))
        return (gamemode == commercial && gameskill != sk_nightmare);
    else if (M_StringCompare(cmd, cheat_powerup[0].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[1].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[2].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[3].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[4].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[5].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_powerup[6].sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_choppers.sequence))
        return (gameskill != sk_nightmare && players[0].health > 0);
    else if (M_StringCompare(cmd, cheat_mypos.sequence))
        return true;
    else if (M_StringCompare(cmd, cheat_clev.sequence))
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
    else if (M_StringCompare(cmd, cheat_amap.sequence))
        return (automapactive || mapwindow);
    return false;
}

static dboolean game_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (gamestate == GS_LEVEL);
}

static dboolean null_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return true;
}

//
// bind cmd
//
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

static void C_UnbindDuplicates(int keep, controltype_t type, int value)
{
    int i = 0;

    while (actions[i].action[0])
    {
        if (i != keep)
        {
            if (type == keyboardcontrol && actions[i].keyboard
                && value == *(int *)actions[i].keyboard)
                *(int *)actions[i].keyboard = 0;
            else if (type == mousecontrol && actions[i].mouse && value == *(int *)actions[i].mouse)
                *(int *)actions[i].mouse = -1;
            else if (type == gamepadcontrol && actions[i].gamepad
                && value == *(int *)actions[i].gamepad)
                *(int *)actions[i].gamepad = 0;
        }

        ++i;
    }
    M_SaveCVARs();
}

void C_Bind(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (!parm1[0])
    {
        int     action = 0;
        int     count = 1;

        while (actions[action].action[0])
        {
            if (actions[action].keyboard)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].keyboard,
                    keyboardcontrol, count++);
            if (actions[action].mouse)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].mouse, mousecontrol,
                    count++);
            if (actions[action].gamepad)
                C_DisplayBinds(actions[action].action, *(int *)actions[action].gamepad,
                    gamepadcontrol, count++);
            ++action;
        }
    }
    else
    {
        int     i = 0;

        while (controls[i].type)
        {
            if (M_StringCompare(parm1, controls[i].control))
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
                    if (controls[i].type == keyboardcontrol && actions[action].keyboard
                        && controls[i].value == *(int *)actions[action].keyboard)
                        C_Output(actions[action].action);
                    else if (controls[i].type == mousecontrol && actions[action].mouse
                        && controls[i].value == *(int *)actions[action].mouse)
                        C_Output(actions[action].action);
                    else if (controls[i].type == gamepadcontrol && actions[action].gamepad
                        && controls[i].value == *(int *)actions[action].gamepad)
                        C_Output(actions[action].action);
                    ++action;
                }
            }
            else if (M_StringCompare(parm2, "none"))
            {
                while (actions[action].action[0])
                {
                    switch (controls[i].type)
                    {
                        case keyboardcontrol:
                            if (actions[action].keyboard
                                && controls[i].value == *(int *)actions[action].keyboard)
                            {
                                *(int *)actions[action].keyboard = 0;
                                M_SaveCVARs();
                            }
                            break;
                        case mousecontrol:
                            if (actions[action].mouse
                                && controls[i].value == *(int *)actions[action].mouse)
                            {
                                *(int *)actions[action].mouse = -1;
                                M_SaveCVARs();
                            }
                            break;
                        case gamepadcontrol:
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
                dboolean        bound = false;

                while (actions[action].action[0])
                {
                    if (M_StringCompare(parm2, actions[action].action))
                        break;
                    ++action;
                }

                if (actions[action].action[0])
                {
                    switch (controls[i].type)
                    {
                        case keyboardcontrol:
                            if (actions[action].keyboard)
                            {
                                *(int *)actions[action].keyboard = controls[i].value;
                                bound = true;
                                C_UnbindDuplicates(action, keyboardcontrol, controls[i].value);
                            }
                            break;
                        case mousecontrol:
                            if (actions[action].mouse)
                            {
                                *(int *)actions[action].mouse = controls[i].value;
                                bound = true;
                                C_UnbindDuplicates(action, mousecontrol, controls[i].value);
                            }
                            break;
                        case gamepadcontrol:
                            if (actions[action].gamepad)
                            {
                                *(int *)actions[action].gamepad = controls[i].value;
                                bound = true;
                                C_UnbindDuplicates(action, gamepadcontrol, controls[i].value);
                            }
                            break;
                    }

                    if (cmd[0])
                        M_SaveCVARs();
                }

                if (!bound)
                    C_Warning("The %s action can't be bound to %s.", parm2, controls[i].control);
            }
        }
    }
}

//
// clear cmd
//
extern int      consolestrings;

static void clear_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    consolestrings = 0;
    C_Output("");
}

//
// cmdlist cmd
//
static void cmdlist_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

//
// condump cmd
//
static void condump_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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
                if (console[i].type == dividerstring)
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
                            else if (ch != '~')
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

//
// cvarlist cmd
//
static void cvarlist_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int i = 0;
    int count = 1;
    int tabs[8] = { 35, 179, 257, 0, 0, 0, 0, 0 };

    while (consolecmds[i].name[0])
    {
        if (consolecmds[i].type == CT_CVAR && (!parm1[0] || wildcard(consolecmds[i].name, parm1)))
        {
            char        description1[255];
            char        description2[255] = "";
            char        *p;

            M_StringCopy(description1, consolecmds[i].description, 255);

            if ((p = strchr(description1, '\n')))
            {
                *p = '\0';
                ++p;
                M_StringCopy(description2, p, 255);
            }

            if (consolecmds[i].flags & CF_BOOLEAN)
                C_TabbedOutput(tabs, "%i.\t%s\t%s\t%s", count++, consolecmds[i].name,
                    C_LookupAliasFromValue(*(dboolean *)consolecmds[i].variable,
                        consolecmds[i].aliases), description1);
            else if ((consolecmds[i].flags & CF_INTEGER) && (consolecmds[i].flags & CF_PERCENT))
                C_TabbedOutput(tabs, "%i.\t%s\t%i%%\t%s", count++, consolecmds[i].name,
                    *(int *)consolecmds[i].variable, description1);
            else if (consolecmds[i].flags & CF_INTEGER)
                C_TabbedOutput(tabs, "%i.\t%s\t%s\t%s", count++, consolecmds[i].name,
                    C_LookupAliasFromValue(*(int *)consolecmds[i].variable,
                        consolecmds[i].aliases), description1);
            else if (consolecmds[i].flags & CF_FLOAT)
                C_TabbedOutput(tabs, "%i.\t%s\t%s%s\t%s", count++, consolecmds[i].name,
                    striptrailingzero(*(float *)consolecmds[i].variable,
                        ((consolecmds[i].flags & CF_PERCENT) ? 1 : 2)),
                    ((consolecmds[i].flags & CF_PERCENT) ? "%" : ""), description1);
            else if (consolecmds[i].flags & CF_STRING)
                C_TabbedOutput(tabs, "%i.\t%s\t\"%.8s%s\"\t%s", count++, consolecmds[i].name,
                    *(char **)consolecmds[i].variable,
                    (strlen(*(char **)consolecmds[i].variable) > 8 ? "..." : ""), description1);
            else if (consolecmds[i].flags & CF_POSITION)
                C_TabbedOutput(tabs, "%i.\t%s\t%s\t%s", count++, consolecmds[i].name,
                    *(char **)consolecmds[i].variable, description1);
            else if (consolecmds[i].flags & CF_SIZE)
                C_TabbedOutput(tabs, "%i.\t%s\t%s\t%s", count++, consolecmds[i].name,
                    formatsize(*(char **)consolecmds[i].variable), description1);
            else if (consolecmds[i].flags & CF_TIME)
            {
                int tics = *(int *)consolecmds[i].variable / TICRATE;

                C_TabbedOutput(tabs, "%i.\t%s\t%02i:%02i:%02i\t%s", count++, consolecmds[i].name,
                    tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60, description1);
            }

            if (description2[0])
                C_TabbedOutput(tabs, "\t\t\t%s", description2);
        }
        ++i;
    }
}

//
// endgame cmd
//
static void endgame_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    M_EndingGame();
    C_HideConsoleFast();
}

//
// exitmap cmd
//
static void exitmap_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    G_ExitLevel();
    C_HideConsoleFast();
}

//
// give cmd
//
extern int      cardsfound;

static dboolean give_cmd_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    char        *parm = M_StringJoin(parm1, parm2, parm3, NULL);
    int         i;

    if (gamestate != GS_LEVEL)
        return false;

    if (!parm[0])
        return true;

    if (M_StringCompare(parm, "all") || M_StringCompare(parm, "health") || M_StringCompare(parm,
        "weapons") || M_StringCompare(parm, "ammo") || M_StringCompare(parm, "armor")
        || M_StringCompare(parm, "armour") || M_StringCompare(parm, "keys"))
        return true;

    for (i = 0; i < NUMMOBJTYPES; i++)
        if ((mobjinfo[i].flags & MF_SPECIAL) && (M_StringCompare(parm,
            removespaces(mobjinfo[i].name1)) || (mobjinfo[i].name2[0] && M_StringCompare(parm,
            removespaces(mobjinfo[i].name2)))))
            return true;

    return false;
}

static void give_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    char        *parm = M_StringJoin(parm1, parm2, parm3, NULL);

    if (!parm[0])
        C_Output("%s %s", cmd, GIVECMDLONGFORMAT);
    else
    {
        player_t    *player = &players[0];

        if (M_StringCompare(parm, "all"))
        {
            P_GiveBackpack(player, false);
            P_GiveMegaHealth(player);
            P_GiveAllWeapons(player);
            P_GiveFullAmmo(player);
            P_GiveArmor(player, blue_armor_class);
            P_GiveAllCards(player);
            C_HideConsole();
        }
        else if (M_StringCompare(parm, "health"))
        {
            P_GiveMegaHealth(player);
            C_HideConsole();
        }
        else if (M_StringCompare(parm, "weapons"))
        {
            P_GiveAllWeapons(player);
            C_HideConsole();
        }
        else if (M_StringCompare(parm, "ammo"))
        {
            P_GiveFullAmmo(player);
            C_HideConsole();
        }
        else if (M_StringCompare(parm, "armor") || M_StringCompare(parm, "armour"))
        {
            P_GiveArmor(player, blue_armor_class);
            C_HideConsole();
        }
        else if (M_StringCompare(parm, "keys"))
        {
            P_GiveAllCards(player);
            C_HideConsole();
        }
        else
        {
            int i;

            for (i = 0; i < NUMMOBJTYPES; i++)
                if ((mobjinfo[i].flags & MF_SPECIAL)
                    && (M_StringCompare(parm, removespaces(mobjinfo[i].name1))
                        || (mobjinfo[i].name2[0]
                            && M_StringCompare(parm, removespaces(mobjinfo[i].name2)))))
                {
                    mobj_t *thing = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, i);

                    P_TouchSpecialThing(thing, player->mo);
                    C_HideConsole();
                    break;
                }
        }
    }
}

//
// god cmd
//
static dboolean god_cmd_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (gamestate == GS_LEVEL && players[0].playerstate == PST_LIVE);
}

static void god_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

//
// help cmd
//
static void help_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    C_HideConsoleFast();
    M_ShowHelp();
}

//
// kill cmd
//
static int      killcmdtype = NUMMOBJTYPES;

static dboolean kill_cmd_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (gamestate == GS_LEVEL)
    {
        char    *parm = M_StringJoin(parm1, parm2, parm3, NULL);
        int     i;

        if (!parm[0])
            return true;

        if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me")
            || (playername[0] && M_StringCompare(parm, playername)))
            return !!players[0].mo->health;

        if (M_StringCompare(parm, "monsters") || M_StringCompare(parm, "all"))
            return true;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (M_StringCompare(parm, removespaces(mobjinfo[i].name1))
                || M_StringCompare(parm, removespaces(mobjinfo[i].plural1))
                || (mobjinfo[i].name2[0] && M_StringCompare(parm, removespaces(mobjinfo[i].name2)))
                || (mobjinfo[i].plural2[0] &&
                    M_StringCompare(parm, removespaces(mobjinfo[i].plural2))))
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

void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);

static void kill_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    char        *parm = M_StringJoin(parm1, parm2, parm3, NULL);
    static char buffer[1024];

    if (!parm[0])
        C_Output("%s %s", cmd, KILLCMDFORMAT);
    else if (M_StringCompare(parm, "player") || M_StringCompare(parm, "me")
        || (playername[0] && M_StringCompare(parm, playername)))
    {
        players[0].health = 0;
        P_KillMobj(players[0].mo, players[0].mo);
        M_snprintf(buffer, sizeof(buffer), "%s killed %s", playername,
            (M_StringCompare(playername, "you") ? "yourself" : "themselves"));
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

        if (M_StringCompare(parm, "all") || M_StringCompare(parm, "monsters"))
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
                            A_Fall(thing, NULL, NULL);
                            P_SetMobjState(thing, S_PAIN_DIE6);
                            players[0].killcount++;
                            stat_monsterskilled = SafeAdd(stat_monsterskilled, 1);
                            kills++;
                        }
                        else if ((thing->flags & MF_SHOOTABLE) && type != MT_PLAYER
                            && type != MT_BARREL && type != MT_BOSSBRAIN)
                        {
                            P_DamageMobj(thing, NULL, NULL, thing->health);
                            if (!(thing->flags & MF_NOBLOOD))
                            {
                                thing->momx += FRACUNIT * M_RandomInt(-1, 1);
                                thing->momy += FRACUNIT * M_RandomInt(-1, 1);
                            }
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
            mobjtype_t  type = P_FindDoomedNum(killcmdtype);
            int         dead = 0;

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
                                A_Fall(thing, NULL, NULL);
                                P_SetMobjState(thing, S_PAIN_DIE6);
                                players[0].killcount++;
                                stat_monsterskilled = SafeAdd(stat_monsterskilled, 1);
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

//
// load cmd
//
static dboolean load_cmd_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (parm1[0] != '\0');
}

static void load_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    G_LoadGame(M_StringJoin(savegamefolder, parm1,
        (M_StringEndsWith(parm1, ".save") ? "" : ".save"), NULL));
}

//
// map cmd
//
static int      mapcmdepisode;
static int      mapcmdmap;

extern dboolean samelevel;
extern menu_t   EpiDef;
extern int      idclevtics;

static dboolean map_cmd_func1(char *cmd, char *parm1, char *parm2, char *parm3)
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

static void map_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

//
// maplist cmd
//
extern int      dehcount;
extern char     **mapnames[];
extern char     **mapnames2[];
extern char     **mapnames2_bfg[];
extern char     **mapnamesp[];
extern char     **mapnamest[];
extern char     **mapnamesn[];

static void maplist_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

        M_StringCopy(wad, uppercase(leafname(lumpinfo[i]->wad_file->path)), MAX_PATH);
        replaced = (W_CheckMultipleLumps(lump) > 1 && !chex && !FREEDOOM);
        pwad = (lumpinfo[i]->wad_file->type == PWAD);

        switch (gamemission)
        {
            case doom:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t%s\t%s", lump,
                        (replaced && dehcount == 1 ? "-" : titlecase(*mapnames[ep * 9 + map])),
                        (modifiedgame ? wad : ""));
                break;
            case doom2:
                if (!M_StringCompare(wad, "nerve.wad") && (!replaced || pwad || nerve)
                    && (pwad || !BTSX))
                    if (BTSX)
                    {
                        if (strchr(*mapnames2[map], ':'))
                            M_snprintf(maplist[count++], 256, "%s",
                                M_StringReplace(*mapnames2[map], ": ", "\t"));
                    }
                    else
                        M_snprintf(maplist[count++], 256, "%s\t%s\t%s", lump, (replaced
                            && dehcount == 1 && !nerve ? "-" : titlecase(bfgedition ?
                                *mapnames2_bfg[map] : *mapnames2[map])), (modifiedgame && !nerve ?
                                    wad : ""));
                break;
            case pack_nerve:
                if (M_StringCompare(wad, "nerve.wad"))
                    M_snprintf(maplist[count++], 256, "%s\t%s", lump, titlecase(*mapnamesn[map]));
                break;
            case pack_plut:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t%s\t%s", lump,
                        (replaced && dehcount == 1 ? "-" : titlecase(*mapnamesp[map])),
                        (modifiedgame ? wad : ""));
                break;
            case pack_tnt:
                if (!replaced || pwad)
                    M_snprintf(maplist[count++], 256, "%s\t%s\t%s", lump,
                        (replaced && dehcount == 1 ? "-" : titlecase(*mapnamest[map])),
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

//
// mapstats cmd
//
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
    /* 00 */{ "",    "",    "",    DCMC,  "" },
    /* 01 */{ "",    SP,    TM,    DCMC,  RM },
    /* 02 */{ "",    AM,    JW,    DCMC,  RH },
    /* 03 */{ "",    AM,    RPJM2, DCMC,  RM },
    /* 04 */{ "",    AM,    TH2,   DCMC,  RM },
    /* 05 */{ "",    AM,    JD,    DCMC,  RH },
    /* 06 */{ "",    AM,    JSTH2, DCMC,  RH },
    /* 07 */{ "",    AMSP,  AD,    DCMC,  RH },
    /* 08 */{ "",    SP,    JM2,   DCMC,  RH },
    /* 09 */{ "",    SP,    JSTH2, DCMC,  RM },
    /* 10 */{ "",    SPTH,  TM,    DCMC,  "" },
    /* 11 */{ JR,    JR,    DJ,    DCMC,  "" },
    /* 12 */{ JR,    SP,    JL,    DCMC,  "" },
    /* 13 */{ JR,    SP,    BKTH2, DCMC,  "" },
    /* 14 */{ JRTH,  AM,    RP,    DCMC,  "" },
    /* 15 */{ JR,    JR,    WW,    DCMC,  "" },
    /* 16 */{ JR,    SP,    AA,    DCMC,  "" },
    /* 17 */{ JR,    JR,    TM,    DCMC,  "" },
    /* 18 */{ SPTH,  SP,    DCTH2, DCMC,  "" },
    /* 19 */{ JR,    SP,    TH2,   DCMC,  "" },
    /* 20 */{ "",    JR,    DO,    DCMC,  "" },
    /* 21 */{ SPTH,  SP,    DO,    DCMC,  "" },
    /* 22 */{ SPTH,  AM,    CB,    DCMC,  "" },
    /* 23 */{ SPTH,  SP,    PT,    DCMC,  "" },
    /* 24 */{ SPTH,  SP,    DJ,    DCMC,  "" },
    /* 25 */{ SP,    SG,    JM,    DCMC,  "" },
    /* 26 */{ SP,    JR,    MSJL,  DCMC,  "" },
    /* 27 */{ SPTH,  SP,    DO,    DCMC,  "" },
    /* 28 */{ SP,    SP,    MC,    DCMC,  "" },
    /* 29 */{ SP,    JR,    JS,    DCMC,  "" },
    /* 30 */{ "",    SP,    JS,    DCMC,  "" },
    /* 31 */{ "",    SP,    DC,    DCMC,  "" },
    /* 32 */{ "",    SP,    DC,    DCMC,  "" },
    /* 33 */{ SPTH,  MB,    "",    "",    "" },
    /* 34 */{ "",    "",    "",    "",    "" },
    /* 35 */{ "",    "",    "",    "",    "" },
    /* 36 */{ "",    "",    "",    "",    "" },
    /* 37 */{ "",    "",    "",    "",    "" },
    /* 38 */{ "",    "",    "",    "",    "" },
    /* 39 */{ "",    "",    "",    "",    "" },
    /* 40 */{ "",    "",    "",    "",    "" },
    /* 41 */{ AM,    "",    "",    "",    "" },
    /* 42 */{ JR,    "",    "",    "",    "" },
    /* 43 */{ SG,    "",    "",    "",    "" },
    /* 44 */{ AM,    "",    "",    "",    "" },
    /* 45 */{ TW,    "",    "",    "",    "" },
    /* 46 */{ JR,    "",    "",    "",    "" },
    /* 47 */{ JA,    "",    "",    "",    "" },
    /* 48 */{ SG,    "",    "",    "",    "" },
    /* 49 */{ TW,    "",    "",    "",    "" }
};

static void mapstats_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int tabs[8] = { 120, 240, 0, 0, 0, 0, 0, 0 };

    C_TabbedOutput(tabs, "Title\t%s", mapnumandtitle);

    {
        int     i = (gamemission == doom ? gameepisode * 10 : 0) + gamemap;
        char    *author = P_GetMapAuthor(i);

        if (author[0])
            C_TabbedOutput(tabs, "Author\t%s", author);
        else if (canmodify && authors[i][gamemission][0])
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
            uppercase(leafname(lumpinfo[i]->wad_file->path)));
    }

    C_TabbedOutput(tabs, "Things\t%s\t%s", commify(numthings), convertsize(sizethings));

    C_TabbedOutput(tabs, "Lines\t%s\t%s", commify(numlines), convertsize(sizelines));

    C_TabbedOutput(tabs, "Line specials\t%s-compatible",
        (boomlinespecials ? "~BOOM~" : "Vanilla"));

    C_TabbedOutput(tabs, "Sides\t%s\t%s", commify(numsides), convertsize(sizesides));

    C_TabbedOutput(tabs, "Vertices\t%s\t%s", commify(numvertexes), convertsize(sizevertexes));

    C_TabbedOutput(tabs, "Segments\t%s\t%s", commify(numsegs), convertsize(sizesegs));

    C_TabbedOutput(tabs, "Subsectors\t%s\t%s", commify(numsubsectors),
        convertsize(sizesubsectors));

    C_TabbedOutput(tabs, "Nodes\t%s\t%s", commify(numnodes), convertsize(sizenodes));

    C_TabbedOutput(tabs, "Node format\t%s", (mapformat == DOOMBSP ? "Regular nodes" :
        (mapformat == DEEPBSP ? "DeePBSP v4 extended nodes" :
            "ZDoom uncompressed extended nodes")));

    C_TabbedOutput(tabs, "Sectors\t%s\t%s", commify(numsectors), convertsize(sizesectors));

    if (blockmaprecreated)
        C_TabbedOutput(tabs, "Blockmap\tRecreated");

    {
        int     i;
        int     min_x = INT_MAX;
        int     max_x = INT_MIN;
        int     min_y = INT_MAX;
        int     max_y = INT_MIN;

        for (i = 0; i < numvertexes; ++i)
        {
            fixed_t     x = vertexes[i].x;
            fixed_t     y = vertexes[i].y;

            if (x < min_x)
                min_x = x;
            else if (x > max_x)
                max_x = x;
            if (y < min_y)
                min_y = y;
            else if (y > max_y)
                max_y = y;
        }
        C_TabbedOutput(tabs, "Dimensions\t%sx%s",
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

//
// noclip cmd
//
static void noclip_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

//
// nomonsters cmd
//
static void nomonsters_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, 1);

        if (value == 0)
            nomonsters = false;
        else if (value == 1)
            nomonsters = true;
    }
    else
        nomonsters = !nomonsters;

    HU_PlayerMessage((nomonsters ? s_STSTR_NMON : s_STSTR_NMOFF), false);
}

//
// notarget cmd
//
static void notarget_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

static void C_PlayerStats_Game(void)
{
    int         tabs[8] = { 160, 270, 0, 0, 0, 0, 0, 0 };
    int         time1 = leveltime / TICRATE;
    int         time2 = stat_time / TICRATE;
    player_t    *player = &players[0];

    C_TabbedOutput(tabs, "\t~Current Map~\t~Total~");

    if ((players[0].cheats & CF_ALLMAP) || (players[0].cheats & CF_ALLMAP_THINGS))
        C_TabbedOutput(tabs, "Map revealed\t100%%\t-");
    else
    {
        int     i = 0;
        int     totallines = numlines;
        int     totallinesmapped = 0;

        while (i < numlines)
            totallines -= !!(lines[i++].flags & ML_DONTDRAW);
        i = 0;
        while (i < totallines)
            totallinesmapped += !!(lines[i++].flags & ML_MAPPED);
        C_TabbedOutput(tabs, "Map revealed\t%s%%\t-",
            striptrailingzero(totallinesmapped * 100.0f / totallines, 1));
    }

    C_TabbedOutput(tabs, "Monsters killed\t%s of %s (%i%%)\t%s",
        commify(player->killcount), commify(totalkills),
        (totalkills ? player->killcount * 100 / totalkills : 0), commify(stat_monsterskilled));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_BABY].plural1),
            commify(player->mobjcount[MT_BABY]), commify(stat_monsterskilled_arachnotrons));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_VILE].plural1),
            commify(player->mobjcount[MT_VILE]), commify(stat_monsterskilled_archviles));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_BRUISER].plural1),
        commify(player->mobjcount[MT_BRUISER]), commify(stat_monsterskilled_baronsofhell));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_HEAD].plural1),
        commify(player->mobjcount[MT_HEAD]), commify(stat_monsterskilled_cacodemons));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_CYBORG].plural1),
        commify(player->mobjcount[MT_CYBORG]), commify(stat_monsterskilled_cyberdemons));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_SERGEANT].plural1),
        commify(player->mobjcount[MT_SERGEANT]), commify(stat_monsterskilled_demons));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_CHAINGUY].plural1),
            commify(player->mobjcount[MT_CHAINGUY]),
            commify(stat_monsterskilled_heavyweapondudes));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_KNIGHT].plural1),
            commify(player->mobjcount[MT_KNIGHT]), commify(stat_monsterskilled_hellknights));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_TROOP].plural1),
        commify(player->mobjcount[MT_TROOP]), commify(stat_monsterskilled_imps));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_SKULL].plural1),
        commify(player->mobjcount[MT_SKULL]), commify(stat_monsterskilled_lostsouls));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_FATSO].plural1),
            commify(player->mobjcount[MT_FATSO]), commify(stat_monsterskilled_mancubi));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_PAIN].plural1),
            commify(player->mobjcount[MT_PAIN]), commify(stat_monsterskilled_painelementals));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_UNDEAD].plural1),
        commify(player->mobjcount[MT_UNDEAD]), commify(stat_monsterskilled_revenants));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_SHOTGUY].plural1),
        commify(player->mobjcount[MT_SHOTGUY]), commify(stat_monsterskilled_shotgunguys));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_SHADOWS].plural1),
        commify(player->mobjcount[MT_SHADOWS]), commify(stat_monsterskilled_spectres));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_SPIDER].plural1),
        commify(player->mobjcount[MT_SPIDER]), commify(stat_monsterskilled_spidermasterminds));

    C_TabbedOutput(tabs, "   %s\t%s\t%s", titlecase(mobjinfo[MT_POSSESSED].plural1),
        commify(player->mobjcount[MT_POSSESSED]), commify(stat_monsterskilled_zombiemen));

    C_TabbedOutput(tabs, "Items picked up\t%s of %s (%i%%)\t%s",
        commify(player->itemcount), commify(totalitems),
        (totalitems ? player->itemcount * 100 / totalitems : 0), commify(stat_itemspickedup));

    C_TabbedOutput(tabs, "Secrets revealed\t%s of %s (%i%%)\t%s",
        commify(player->secretcount), commify(totalsecret),
        (totalsecret ? player->secretcount * 100 / totalsecret : 0),
        commify(stat_secretsrevealed));

    C_TabbedOutput(tabs, "Time played\t%02i:%02i:%02i\t%02i:%02i:%02i",
        time1 / 3600, (time1 % 3600) / 60, (time1 % 3600) % 60,
        time2 / 3600, (time2 % 3600) / 60, (time2 % 3600) % 60);

    C_TabbedOutput(tabs, "Damage inflicted\t%s\t%s",
        commify(player->damageinflicted), commify(stat_damageinflicted));

    C_TabbedOutput(tabs, "Damage received\t%s\t%s",
        commify(player->damagereceived), commify(stat_damagereceived));

    C_TabbedOutput(tabs, "Deaths\t%s\t%s", commify(player->deaths), commify(stat_deaths));

    C_TabbedOutput(tabs, "Cheated\t%s\t%s",
        commify(player->cheated), commify(stat_cheated));

    C_TabbedOutput(tabs, "Shots fired\t%s\t%s",
        commify(player->shotsfired), commify(stat_shotsfired));

    C_TabbedOutput(tabs, "Shots hit\t%s\t%s",
        commify(player->shotshit), commify(stat_shotshit));

    C_TabbedOutput(tabs, "Weapon accuracy\t%s%%\t%s%%",
        (player->shotsfired ? striptrailingzero(player->shotshit * 100.0f / player->shotsfired,
        1) : "0"), (stat_shotsfired ? striptrailingzero(stat_shotshit * 100.0f / stat_shotsfired,
        1) : "0"));
}

static void C_PlayerStats_NoGame(void)
{
    int tabs[8] = { 160, 0, 0, 0, 0, 0, 0, 0 };
    int time2 = stat_time / TICRATE;

    C_TabbedOutput(tabs, "\t~Total~");

    C_TabbedOutput(tabs, "Monsters killed\t%s", commify(stat_monsterskilled));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_BABY].plural1),
            commify(stat_monsterskilled_arachnotrons));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_VILE].plural1),
            commify(stat_monsterskilled_archviles));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_BRUISER].plural1),
        commify(stat_monsterskilled_baronsofhell));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_HEAD].plural1),
        commify(stat_monsterskilled_cacodemons));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_CYBORG].plural1),
        commify(stat_monsterskilled_cyberdemons));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_SERGEANT].plural1),
        commify(stat_monsterskilled_demons));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_CHAINGUY].plural1),
            commify(stat_monsterskilled_heavyweapondudes));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_KNIGHT].plural1),
            commify(stat_monsterskilled_hellknights));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_TROOP].plural1),
        commify(stat_monsterskilled_imps));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_SKULL].plural1),
        commify(stat_monsterskilled_lostsouls));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_FATSO].plural1),
            commify(stat_monsterskilled_mancubi));

    if (gamemode == commercial)
        C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_PAIN].plural1),
            commify(stat_monsterskilled_painelementals));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_UNDEAD].plural1),
        commify(stat_monsterskilled_revenants));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_SHOTGUY].plural1),
        commify(stat_monsterskilled_shotgunguys));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_SHADOWS].plural1),
        commify(stat_monsterskilled_spectres));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_SPIDER].plural1),
        commify(stat_monsterskilled_spidermasterminds));

    C_TabbedOutput(tabs, "   %s\t%s", titlecase(mobjinfo[MT_POSSESSED].plural1),
        commify(stat_monsterskilled_zombiemen));

    C_TabbedOutput(tabs, "Items picked up\t%s", commify(stat_itemspickedup));

    C_TabbedOutput(tabs, "Secrets revealed\t%s", commify(stat_secretsrevealed));

    C_TabbedOutput(tabs, "Time played\t%02i:%02i:%02i",
        time2 / 3600, (time2 % 3600) / 60, (time2 % 3600) % 60);

    C_TabbedOutput(tabs, "Damage inflicted\t%s", commify(stat_damageinflicted));

    C_TabbedOutput(tabs, "Damage received\t%s", commify(stat_damagereceived));

    C_TabbedOutput(tabs, "Deaths\t%s", commify(stat_deaths));

    C_TabbedOutput(tabs, "Cheated\t%s", commify(stat_cheated));

    C_TabbedOutput(tabs, "Shots fired\t%s", commify(stat_shotsfired));

    C_TabbedOutput(tabs, "Shots hit\t%s", commify(stat_shotshit));

    C_TabbedOutput(tabs, "Weapon accuracy\t%s%%",
        (stat_shotsfired ? striptrailingzero(stat_shotshit * 100.0f / stat_shotsfired, 1) : "0"));
}

//
// playerstats cmd
//
static void playerstats_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (gamestate == GS_LEVEL)
        C_PlayerStats_Game();
    else
        C_PlayerStats_NoGame();
}

//
// quit cmd
//
static void quit_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    I_Quit(true);
}

//
// resurrect cmd
//
static dboolean resurrect_cmd_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (gamestate == GS_LEVEL && players[0].playerstate == PST_DEAD);
}

static void resurrect_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    P_ResurrectPlayer(&players[0]);
    C_HideConsole();
}

//
// save cmd
//
static dboolean save_cmd_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (parm1[0] != '\0' && gamestate == GS_LEVEL && players[0].playerstate == PST_LIVE);
}

static void save_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    G_SaveGame(-1, "", M_StringJoin(savegamefolder, parm1,
        (M_StringEndsWith(parm1, ".save") ? "" : ".save"), NULL));
}

//
// spawn cmd
//
static int      spawntype = NUMMOBJTYPES;

static dboolean spawn_cmd_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    char        *parm = M_StringJoin(parm1, parm2, parm3, NULL);

    if (!parm[0])
        return true;

    if (gamestate == GS_LEVEL)
    {
        int i;

        for (i = 0; i < NUMMOBJTYPES; i++)
            if (M_StringCompare(parm, removespaces(mobjinfo[i].name1))
                || (mobjinfo[i].name2[0] && M_StringCompare(parm, removespaces(mobjinfo[i].name2))))
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

static void spawn_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    char        *parm = M_StringJoin(parm1, parm2, parm3, NULL);

    if (!parm[0])
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

//
// thinglist cmd
//
static void thinglist_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

//
// unbind cmd
//
static void unbind_cmd_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    C_Bind(cmd, parm1, "none", "");
}

//
// bool cvars
//
static dboolean bool_cvars_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, BOOLALIAS) >= 0);
}

static void bool_cvars_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
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

//
// color cvars
//
static void color_cvars_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int_cvars_func2(cmd, parm1, parm2, parm3);
    if (parm1[0])
        AM_SetColors();
}

//
// float cvars
//
static dboolean float_cvars_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int i = 0;

    if (!parm1[0])
        return true;

    while (consolecmds[i].name[0])
    {
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
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

static void float_cvars_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
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

//
// int cvars
//
static dboolean int_cvars_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int i = 0;

    if (!parm1[0])
        return true;

    while (consolecmds[i].name[0])
    {
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
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

static void int_cvars_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
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
            else if (consolecmds[i].flags & CF_PERCENT)
                C_Output("%i%%", *(int *)consolecmds[i].variable);
            else
                C_Output(C_LookupAliasFromValue(*(int *)consolecmds[i].variable,
                    consolecmds[i].aliases));
        }
        ++i;
    }
}

//
// str cvars
//
static void str_cvars_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_STRING))
        {
            if (M_StringCompare(parm1, EMPTYVALUE))
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

//
// time cvars
//
static void time_cvars_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int i = 0;

    while (consolecmds[i].name[0])
    {
        if (M_StringCompare(cmd, consolecmds[i].name) && consolecmds[i].type == CT_CVAR
            && (consolecmds[i].flags & CF_TIME))
        {
            int tics = *(int *)consolecmds[i].variable / TICRATE;

            C_Output("%02i:%02i:%02i", tics / 3600, (tics % 3600) / 60, (tics % 3600) % 60);
        }
        ++i;
    }
}

//
// alwaysrun cvar
//
static void alwaysrun_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    bool_cvars_func2(cmd, parm1, "", "");
    I_InitKeyboard();
}

//
// am_cheat cvar
//
static void am_cheat_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int am_cheat_old = am_cheat;

    int_cvars_func2(cmd, parm1, "", "");
    if (am_cheat != am_cheat_old)
    {
        player_t        *player = &players[0];

        switch (am_cheat)
        {
            case am_cheat_off:
                if (player->cheats & CF_ALLMAP)
                    player->cheats &= !CF_ALLMAP;
                if (player->cheats & CF_ALLMAP_THINGS)
                    player->cheats &= !CF_ALLMAP_THINGS;
                break;

            case am_cheat_all:
                if (player->cheats & CF_ALLMAP_THINGS)
                    player->cheats &= !CF_ALLMAP_THINGS;
                player->cheats |= CF_ALLMAP;
                break;

            case am_cheat_things:
                if (player->cheats & CF_ALLMAP)
                    player->cheats &= !CF_ALLMAP;
                player->cheats |= CF_ALLMAP_THINGS;
                break;
        }
    }
}

//
// am_external cvar
//
static void am_external_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    dboolean    am_external_old = am_external;

    bool_cvars_func2(cmd, parm1, "", "");
    if (am_external != am_external_old)
    {
        if (am_external)
        {
            I_CreateExternalAutomap(false);
            if (gamestate == GS_LEVEL)
                AM_Start(false);
        }
        else
        {
            I_DestroyExternalAutomap();
            if (gamestate == GS_LEVEL)
                AM_Stop();
        }
    }
}

//
// gp_deadzone_left and gp_deadzone_right cvars
//
static dboolean gp_deadzone_cvars_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    float value;

    if (!parm1[0])
        return true;
    if (parm1[strlen(parm1) - 1] == '%')
        parm1[strlen(parm1) - 1] = 0;
    return sscanf(parm1, "%10f", &value);
}

static void gp_deadzone_cvars_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        float   value = 0;

        if (parm1[strlen(parm1) - 1] == '%')
            parm1[strlen(parm1) - 1] = 0;
        sscanf(parm1, "%10f", &value);

        if (M_StringCompare(cmd, stringize(gp_deadzone_left)))
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
    else
        C_Output("%s%%", striptrailingzero((M_StringCompare(cmd, stringize(gp_deadzone_left)) ?
            gp_deadzone_left : gp_deadzone_right), 1));
}

//
// playername cvar
//
static void playername_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        if (!M_StringCompare(parm1, EMPTYVALUE))
        {
            playername = strdup(parm1);
            M_SaveCVARs();
        }
    }
    else
        C_Output("\"%s\"", playername);
}

//
// r_blood cvar
//
static dboolean r_blood_cvar_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, BLOODALIAS) >= 0);
}

void (*P_BloodSplatSpawner)(fixed_t, fixed_t, int, int, mobj_t *);

static void r_blood_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BLOODALIAS);

        if (value >= 0)
        {
            r_blood = value;
            P_BloodSplatSpawner = (r_blood == r_blood_none ? P_NullBloodSplatSpawner :
                P_SpawnBloodSplat);
            M_SaveCVARs();
        }
    }
    else
        C_Output(C_LookupAliasFromValue(r_blood, BLOODALIAS));
}

//
// r_detail cvar
//
static dboolean r_detail_cvar_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (!parm1[0] || C_LookupValueFromAlias(parm1, DETAILALIAS) >= 0);
}

static void r_detail_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

//
// r_gamma cvar
//
extern int      st_palette;

static dboolean r_gamma_cvar_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    float       value = -1.0f;

    if (!parm1[0] || M_StringCompare(parm1, "off"))
        return true;

    sscanf(parm1, "%10f", &value);

    return (value >= 0.0f);
}

static void r_gamma_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        float   value = -1.0f;

        if (M_StringCompare(parm1, "off"))
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

//
// r_hud cvar
//
static void r_hud_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (vid_widescreen || r_screensize == r_screensize_max)
        bool_cvars_func2(cmd, parm1, "", "");
}

//
// r_lowpixelsiz cvar
//
static void r_lowpixelsize_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        r_lowpixelsize = strdup(parm1);

        GetPixelSize();

        if (!M_StringCompare(r_lowpixelsize, parm1))
            M_SaveCVARs();
    }
    else
        C_Output(formatsize(r_lowpixelsize));
}

//
// r_screensize cvar
//
static void r_screensize_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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
                    I_ToggleWidescreen(false);
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
                        I_ToggleWidescreen(true);
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

//
// s_musicvolume and s_sfxvolume cvars
//
static dboolean s_volume_cvars_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    int value = -1;

    if (!parm1[0])
        return true;
    if (parm1[strlen(parm1) - 1] == '%')
        parm1[strlen(parm1) - 1] = 0;

    sscanf(parm1, "%10i", &value);

    return ((M_StringCompare(cmd, stringize(s_musicvolume)) && value >= s_musicvolume_min
        && value <= s_musicvolume_max) || (M_StringCompare(cmd, stringize(s_sfxvolume))
        && value >= s_sfxvolume_min && value <= s_sfxvolume_max));
}

static void s_volume_cvars_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        int value = 0;

        if (parm1[strlen(parm1) - 1] == '%')
            parm1[strlen(parm1) - 1] = 0;
        sscanf(parm1, "%10i", &value);

        if (M_StringCompare(cmd, stringize(s_musicvolume)))
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
        C_Output("%i%%", (M_StringCompare(cmd, stringize(s_musicvolume)) ? s_musicvolume :
            s_sfxvolume));
}

//
// vid_display cvar
//
static void vid_display_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

//
// vid_fullscreen cvar
//
static void vid_fullscreen_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BOOLALIAS);

        if ((value == 0 || value == 1) && value != vid_fullscreen)
            I_ToggleFullscreen();
    }
    else
        C_Output(C_LookupAliasFromValue(vid_fullscreen, BOOLALIAS));
}

//
// vid_scaledriver cvar
//
static dboolean vid_scaledriver_cvar_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (!parm1[0] || M_StringCompare(parm1, EMPTYVALUE)
        || M_StringCompare(parm1, vid_scaledriver_direct3d)
        || M_StringCompare(parm1, vid_scaledriver_opengl)
        || M_StringCompare(parm1, vid_scaledriver_software));
}

static void vid_scaledriver_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        if (M_StringCompare(parm1, EMPTYVALUE))
        {
            vid_scaledriver = "";
            M_SaveCVARs();
            I_RestartGraphics();
        }
        else if (!M_StringCompare(parm1, vid_scaledriver))
        {
            vid_scaledriver = strdup(parm1);
            M_SaveCVARs();
            I_RestartGraphics();
        }
    }
    else
        C_Output("\"%s\"", vid_scaledriver);
}

//
// vid_scalefilter cvar
//
static dboolean vid_scalefilter_cvar_func1(char *cmd, char *parm1, char *parm2, char *parm3)
{
    return (!parm1[0] || M_StringCompare(parm1, vid_scalefilter_nearest)
        || M_StringCompare(parm1, vid_scalefilter_linear)
        || M_StringCompare(parm1, vid_scalefilter_nearest_linear));
}

static void vid_scalefilter_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        if (!M_StringCompare(parm1, vid_scalefilter))
        {
            vid_scalefilter = strdup(parm1);
            M_SaveCVARs();
            I_RestartGraphics();
        }
    }
    else
        C_Output("\"%s\"", vid_scalefilter);
}

//
// vid_screenresolution cvar
//
static void vid_screenresolution_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        vid_screenresolution = strdup(parm1);

        GetScreenResolution();

        if (!M_StringCompare(vid_screenresolution, parm1))
        {
            M_SaveCVARs();

            if (vid_fullscreen)
                I_RestartGraphics();
        }
    }
    else
        C_Output(formatsize(vid_screenresolution));
}

//
// vid_showfps cvar
//
static void vid_showfps_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        int     value = C_LookupValueFromAlias(parm1, BOOLALIAS);

        if ((value == 0 || value == 1) && value != vid_showfps)
        {
            vid_showfps = !!value;
            I_UpdateBlitFunc();
        }
    }
    else
        C_Output(C_LookupAliasFromValue(vid_showfps, BOOLALIAS));
}

//
// vid_vsync cvar
//
static void vid_vsync_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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

//
// vid_widescreen cvar
//
static void vid_widescreen_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
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
                    I_ToggleWidescreen(true);
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
                I_ToggleWidescreen(false);
                if (!vid_widescreen)
                    S_StartSound(NULL, sfx_stnmov);
            }
            M_SaveCVARs();
        }
    }
    else
        C_Output(C_LookupAliasFromValue(vid_widescreen, BOOLALIAS));
}

//
// vid_windowposition cvar
//
static void vid_windowposition_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        vid_windowposition = strdup(parm1);

        GetWindowPosition();

        if (!M_StringCompare(vid_windowposition, parm1))
        {
            M_SaveCVARs();

            if (!vid_fullscreen)
                SDL_SetWindowPosition(window, windowx, windowy);
        }
    }
    else
        C_Output("%s", vid_windowposition);
}

//
// vid_windowsize cvar
//
static void vid_windowsize_cvar_func2(char *cmd, char *parm1, char *parm2, char *parm3)
{
    if (parm1[0])
    {
        vid_windowsize = strdup(parm1);

        GetWindowSize();

        if (!M_StringCompare(vid_windowsize, parm1))
        {
            M_SaveCVARs();

            if (!vid_fullscreen)
                SDL_SetWindowSize(window, windowwidth, windowheight);
        }
    }
    else
        C_Output(formatsize(vid_windowsize));
}
