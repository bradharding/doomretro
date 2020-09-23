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

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// Automap colors
int am_allmapcdwallcolor = am_allmapcdwallcolor_default;
int am_allmapfdwallcolor = am_allmapfdwallcolor_default;
int am_allmapwallcolor = am_allmapwallcolor_default;
int am_backcolor = am_backcolor_default;
int am_cdwallcolor = am_cdwallcolor_default;
int am_crosshaircolor = am_crosshaircolor_default;
int am_fdwallcolor = am_fdwallcolor_default;
int am_gridcolor = am_gridcolor_default;
int am_markcolor = am_markcolor_default;
int am_pathcolor = am_pathcolor_default;
int am_playercolor = am_playercolor_default;
int am_teleportercolor = am_teleportercolor_default;
int am_thingcolor = am_thingcolor_default;
int am_tswallcolor = am_tswallcolor_default;
int am_wallcolor = am_wallcolor_default;

// Automap color priorities
#define PATHPRIORITY           10
#define WALLPRIORITY            9
#define ALLMAPWALLPRIORITY      8
#define CDWALLPRIORITY          7
#define ALLMAPCDWALLPRIORITY    6
#define FDWALLPRIORITY          5
#define ALLMAPFDWALLPRIORITY    4
#define TELEPORTERPRIORITY      3
#define TSWALLPRIORITY          2
#define GRIDPRIORITY            1

static byte playercolor;
static byte thingcolor;
static byte markcolor;
static byte backcolor;

static byte *pathcolor;
static byte *wallcolor;
static byte *allmapwallcolor;
static byte *teleportercolor;
static byte *fdwallcolor;
static byte *allmapfdwallcolor;
static byte *cdwallcolor;
static byte *allmapcdwallcolor;
static byte *tswallcolor;
static byte *gridcolor;
static byte *am_crosshaircolor2;

#define AM_PANDOWNKEY   keyboardback
#define AM_PANDOWNKEY2  keyboardback2
#define AM_PANUPKEY     keyboardforward
#define AM_PANUPKEY2    keyboardforward2
#define AM_PANRIGHTKEY  keyboardright
#define AM_PANRIGHTKEY2 keyboardstraferight
#define AM_PANRIGHTKEY3 keyboardstraferight2
#define AM_PANLEFTKEY   keyboardleft
#define AM_PANLEFTKEY2  keyboardstrafeleft
#define AM_PANLEFTKEY3  keyboardstrafeleft2
#define AM_ZOOMINKEY    keyboardautomapzoomin
#define AM_ZOOMOUTKEY   keyboardautomapzoomout
#define AM_STARTKEY     keyboardautomap
#define AM_ENDKEY       keyboardautomap
#define AM_GOBIGKEY     keyboardautomapmaxzoom
#define AM_FOLLOWKEY    keyboardautomapfollowmode
#define AM_GRIDKEY      keyboardautomapgrid
#define AM_MARKKEY      keyboardautomapmark
#define AM_CLEARMARKKEY keyboardautomapclearmark
#define AM_ROTATEKEY    keyboardautomaprotatemode

#define MAPWIDTH        SCREENWIDTH

// scale on entry
// [BH] changed to initial zoom level of E1M1: Hangar so each map zoom level is consistent
#define INITSCALEMTOF   125114

// how much the automap moves window per tic in map coordinates
// moves 140 pixels in 1 second
#define F_PANINC        ((uint64_t)8 << speedtoggle)

// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((fixed_t)((uint64_t)FRACUNIT * (1.0 + F_PANINC / 200.0)))

// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((fixed_t)((uint64_t)FRACUNIT / (1.0 + F_PANINC / 200.0)))

#define PLAYERRADIUS    (16 * (1 << MAPBITS))

// translates between frame-buffer and map distances
#define FTOM(x)         (fixed_t)((((uint64_t)(x) << FRACBITS) * scale_ftom) >> FRACBITS)
#define MTOF(x)         (fixed_t)((((uint64_t)(x) * scale_mtof) >> FRACBITS) >> FRACBITS)

// translates between frame-buffer and map coordinates
#define CXMTOF(x)       MTOF((uint64_t)(x) - m_x)
#define CYMTOF(y)       (mapheight - MTOF((uint64_t)(y) - m_y))

typedef struct
{
    mpoint_t    a;
    mpoint_t    b;
} mline_t;

static unsigned int mapheight;
static unsigned int maparea;
static unsigned int mapbottom;

dboolean            automapactive;

static mpoint_t     m_paninc;       // how far the window pans each tic (map coords)
static fixed_t      mtof_zoommul;   // how far the window zooms in each tic (map coords)
static fixed_t      ftom_zoommul;   // how far the window zooms in each tic (fb coords)

// LL x,y where the window is on the map (map coords)
static fixed_t      m_x = FIXED_MAX, m_y = FIXED_MAX;

// width/height of window on map (map coords)
static fixed_t      m_w, m_h;

// based on level size
static fixed_t      min_x, min_y;
static fixed_t      max_x, max_y;

static fixed_t      min_scale_mtof; // used to tell when to stop zooming out
static fixed_t      max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static fixed_t      old_m_w, old_m_h;
static fixed_t      old_m_x, old_m_y;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t      scale_mtof;

// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t      scale_ftom;

mpoint_t            *markpoints;    // where the points are
int                 markpointnum;   // next point to be assigned
int                 markpointnum_max;

mpoint_t            *pathpoints;
int                 pathpointnum;
int                 pathpointnum_max;

dboolean            am_external = am_external_default;
dboolean            am_followmode = am_followmode_default;
dboolean            am_grid = am_grid_default;
char                *am_gridsize = am_gridsize_default;
dboolean            am_path = am_path_default;
dboolean            am_rotatemode = am_rotatemode_default;

static int          gridwidth;
static int          gridheight;

static dboolean     bigstate;
static dboolean     movement;
int                 keydown;
int                 direction;

am_frame_t          am_frame;

static dboolean     isteleportline[NUMLINESPECIALS];

static void AM_Rotate(fixed_t *x, fixed_t *y, angle_t angle);
static void (*putbigdot)(unsigned int, unsigned int, byte *);
static void PUTDOT(unsigned int x, unsigned int y, byte *color);
static void PUTBIGDOT(unsigned int x, unsigned int y, byte *color);

static void AM_ActivateNewScale(void)
{
    m_x += m_w / 2;
    m_y += m_h / 2;
    m_w = FTOM(MAPWIDTH);
    m_h = FTOM(mapheight);
    m_x -= m_w / 2;
    m_y -= m_h / 2;
    putbigdot = (scale_mtof >= FRACUNIT + FRACUNIT / 2 ? &PUTBIGDOT : &PUTDOT);
}

static void AM_SaveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

static void AM_RestoreScaleAndLoc(void)
{
    m_w = old_m_w;
    m_h = old_m_h;

    if (!am_followmode)
    {
        m_x = old_m_x;
        m_y = old_m_y;
    }

    // Change the scaling multipliers
    scale_mtof = FixedDiv(MAPWIDTH << FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    putbigdot = (scale_mtof >= FRACUNIT + FRACUNIT / 2 ? &PUTBIGDOT : &PUTDOT);
}

//
// Determines bounding box of all vertexes, sets global variables controlling zoom range.
//
static void AM_FindMinMaxBoundaries(void)
{
    fixed_t a;
    fixed_t b;

    min_x = FIXED_MAX;
    min_y = FIXED_MAX;
    max_x = FIXED_MIN;
    max_y = FIXED_MIN;

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

    a = FixedDiv(MAPWIDTH << FRACBITS, (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS));
    b = FixedDiv(mapheight << FRACBITS, (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS));

    min_scale_mtof = MIN(a, b);
    max_scale_mtof = FixedDiv(mapheight << FRACBITS, 2 * PLAYERRADIUS);
}

static void AM_ChangeWindowLoc(void)
{
    fixed_t         incx = m_paninc.x;
    fixed_t         incy = m_paninc.y;
    const fixed_t   width = m_w / 2;
    const fixed_t   height = m_h / 2;

    if (am_rotatemode)
        AM_Rotate(&incx, &incy, viewangle - ANG90);

    m_x = BETWEEN(min_x, m_x + width + incx, max_x) - width;
    m_y = BETWEEN(min_y, m_y + height + incy, max_y) - height;
}

void AM_SetColors(void)
{
    byte        priority[256] = { 0 };
    static byte priorities[256 * 256];

    priority[nearestcolors[am_pathcolor]] = PATHPRIORITY;
    priority[nearestcolors[am_wallcolor]] = WALLPRIORITY;
    priority[nearestcolors[am_allmapwallcolor]] = ALLMAPWALLPRIORITY;
    priority[nearestcolors[am_cdwallcolor]] = CDWALLPRIORITY;
    priority[nearestcolors[am_allmapcdwallcolor]] = ALLMAPCDWALLPRIORITY;
    priority[nearestcolors[am_fdwallcolor]] = FDWALLPRIORITY;
    priority[nearestcolors[am_allmapfdwallcolor]] = ALLMAPFDWALLPRIORITY;
    priority[nearestcolors[am_teleportercolor]] = TELEPORTERPRIORITY;
    priority[nearestcolors[am_tswallcolor]] = TSWALLPRIORITY;
    priority[nearestcolors[am_gridcolor]] = GRIDPRIORITY;

    playercolor = nearestcolors[am_playercolor];
    thingcolor = nearestcolors[am_thingcolor];
    markcolor = nearestcolors[am_markcolor];
    backcolor = nearestcolors[am_backcolor];
    am_crosshaircolor2 = &tinttab60[nearestcolors[am_crosshaircolor] << 8];

    for (int x = 0; x < 256; x++)
        for (int y = 0; y < 256; y++)
            priorities[(x << 8) + y] = (priority[x] > priority[y] ? x : y);

    pathcolor = &priorities[nearestcolors[am_pathcolor] << 8];
    wallcolor = &priorities[nearestcolors[am_wallcolor] << 8];
    allmapwallcolor = &priorities[nearestcolors[am_allmapwallcolor] << 8];
    cdwallcolor = &priorities[nearestcolors[am_cdwallcolor] << 8];
    allmapcdwallcolor = &priorities[nearestcolors[am_allmapcdwallcolor] << 8];
    fdwallcolor = &priorities[nearestcolors[am_fdwallcolor] << 8];
    allmapfdwallcolor = &priorities[nearestcolors[am_allmapfdwallcolor] << 8];
    teleportercolor = &priorities[nearestcolors[am_teleportercolor] << 8];
    tswallcolor = &priorities[nearestcolors[am_tswallcolor] << 8];
    gridcolor = &priorities[nearestcolors[am_gridcolor ]<< 8];
}

void AM_GetGridSize(void)
{
    int width = -1;
    int height = -1;

    if (sscanf(am_gridsize, "%10ix%10i", &width, &height) == 2
        && width >= 4 && width <= 4096 && height >= 4 && height <= 4096)
    {
        gridwidth = width << MAPBITS;
        gridheight = height << MAPBITS;
    }
    else
    {
        gridwidth = 128 << MAPBITS;
        gridheight = 128 << MAPBITS;
        am_gridsize = am_gridsize_default;
        M_SaveCVARs();
    }
}

void AM_Init(void)
{
    AM_SetColors();
    AM_GetGridSize();
    AM_SetAutomapSize();

    isteleportline[W1_Teleport] = true;
    isteleportline[W1_ExitLevel] = true;
    isteleportline[WR_Teleport] = true;
    isteleportline[W1_ExitLevel_GoesToSecretLevel] = true;
    isteleportline[W1_Teleport_AlsoMonsters_Silent_SameAngle] = true;
    isteleportline[WR_Teleport_AlsoMonsters_Silent_SameAngle] = true;
    isteleportline[W1_TeleportToLineWithSameTag_Silent_SameAngle] = true;
    isteleportline[WR_TeleportToLineWithSameTag_Silent_SameAngle] = true;
    isteleportline[W1_TeleportToLineWithSameTag_Silent_ReversedAngle] = true;
    isteleportline[WR_TeleportToLineWithSameTag_Silent_ReversedAngle] = true;
}

void AM_SetAutomapSize(void)
{
    if (vid_widescreen || !menuactive)
    {
        mapheight = SCREENHEIGHT - SBARHEIGHT;
        maparea = SCREENWIDTH * (SCREENHEIGHT - SBARHEIGHT);
        mapbottom = SCREENWIDTH * (SCREENHEIGHT - SBARHEIGHT - 1);
    }
    else
    {
        mapheight = SCREENHEIGHT;
        maparea = SCREENAREA;
        mapbottom = SCREENWIDTH * (SCREENHEIGHT - 1);
    }
}

static void AM_InitVariables(const dboolean mainwindow)
{
    automapactive = mainwindow;

    m_paninc.x = 0;
    m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(MAPWIDTH);
    m_h = FTOM(mapheight);
}

static void AM_LevelInit(void)
{
    am_followmode = am_followmode_default;
    bigstate = false;

    AM_FindMinMaxBoundaries();
    scale_mtof = FixedDiv(INITSCALEMTOF, FRACUNIT * 7 / 10);

    if (scale_mtof > max_scale_mtof)
        scale_mtof = min_scale_mtof;

    putbigdot = (scale_mtof >= FRACUNIT + FRACUNIT / 2 ? &PUTBIGDOT : &PUTDOT);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    // for saving and restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

void AM_Stop(void)
{
    automapactive = false;
    HU_ClearMessages();
}

int lastlevel = -1;
int lastepisode = -1;

void AM_Start(const dboolean mainwindow)
{
    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
        AM_LevelInit();
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }

    AM_InitVariables(mainwindow);
}

//
// set the window scale to the maximum size
//
static void AM_MinOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_ActivateNewScale();
}

//
// set the window scale to the minimum size
//
static void AM_MaxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_ActivateNewScale();
}

static SDL_Keymod   modstate;
static dboolean     speedtoggle;

static dboolean AM_GetSpeedToggle(void)
{
    return ((!!(gamepadbuttons & GAMEPAD_LEFT_TRIGGER)) ^ (!!(modstate & KMOD_SHIFT)));
}

static void AM_ToggleZoomOut(void)
{
    speedtoggle = AM_GetSpeedToggle();
    mtof_zoommul = M_ZOOMOUT;
    ftom_zoommul = M_ZOOMIN;
}

static void AM_ToggleZoomIn(void)
{
    speedtoggle = AM_GetSpeedToggle();
    mtof_zoommul = M_ZOOMIN;
    ftom_zoommul = M_ZOOMOUT;
    bigstate = false;
}

void AM_ToggleMaxZoom(void)
{
    if (bigstate)
    {
        bigstate = false;
        AM_RestoreScaleAndLoc();
    }
    else if (scale_mtof != min_scale_mtof)
    {
        bigstate = true;
        AM_SaveScaleAndLoc();
        AM_MinOutWindowScale();
    }
}

void AM_ToggleFollowMode(void)
{
    if ((am_followmode = !am_followmode))
    {
        m_paninc.x = 0;
        m_paninc.y = 0;
        C_StrCVAROutput(stringize(am_followmode), "on");
        C_Output(s_AMSTR_FOLLOWON);
        HU_SetPlayerMessage(s_AMSTR_FOLLOWON, false, true);
    }
    else
    {
        C_StrCVAROutput(stringize(am_followmode), "off");
        C_Output(s_AMSTR_FOLLOWOFF);
        HU_SetPlayerMessage(s_AMSTR_FOLLOWOFF, false, true);
    }

    message_dontfuckwithme = true;
}

void AM_ToggleGrid(void)
{
    if ((am_grid = !am_grid))
    {
        C_StrCVAROutput(stringize(am_grid), "on");
        C_Output(s_AMSTR_GRIDON);
        HU_SetPlayerMessage(s_AMSTR_GRIDON, false, true);
    }
    else
    {
        C_StrCVAROutput(stringize(am_grid), "off");
        C_Output(s_AMSTR_GRIDOFF);
        HU_SetPlayerMessage(s_AMSTR_GRIDOFF, false, true);
    }

    message_dontfuckwithme = true;
    M_SaveCVARs();
}

//
// adds a marker at the current location
//
void AM_AddMark(void)
{
    const int   x = am_frame.center.x;
    const int   y = am_frame.center.y;
    char        message[32];

    for (int i = 0; i < markpointnum; i++)
        if (markpoints[i].x == x && markpoints[i].y == y)
            return;

    if (markpointnum >= markpointnum_max)
    {
        markpointnum_max = (markpointnum_max ? markpointnum_max * 2 : 16);
        markpoints = I_Realloc(markpoints, markpointnum_max * sizeof(*markpoints));
    }

    markpoints[markpointnum].x = x;
    markpoints[markpointnum].y = y;

    M_snprintf(message, sizeof(message), s_AMSTR_MARKEDSPOT, ++markpointnum);
    C_Output(message);
    HU_SetPlayerMessage(message, false, true);
    message_dontfuckwithme = true;
}

static int  markpress;

void AM_ClearMarks(void)
{
    if (markpointnum)
    {
        if (++markpress == 5)
        {
            // clear all marks
            C_Output(s_AMSTR_MARKSCLEARED);
            HU_SetPlayerMessage(s_AMSTR_MARKSCLEARED, false, true);
            markpointnum = 0;
        }
        else if (markpress == 1)
        {
            char    message[32];

            // clear one mark
            M_snprintf(message, sizeof(message), s_AMSTR_MARKCLEARED, markpointnum--);
            C_Output(message);
            HU_SetPlayerMessage(message, false, true);
        }

        message_dontfuckwithme = true;
    }
}

void AM_AddToPath(void)
{
    mobj_t      *mo = viewplayer->mo;
    const int   x = mo->x;
    const int   y = mo->y;
    static int  prevx = INT_MAX;
    static int  prevy = INT_MAX;

    if (x == prevx && y == prevy)
        return;

    if (pathpointnum >= pathpointnum_max)
    {
        pathpointnum_max = (pathpointnum_max ? pathpointnum_max * 2 : 1024);
        pathpoints = I_Realloc(pathpoints, pathpointnum_max * sizeof(*pathpoints));
    }

    pathpoints[pathpointnum].x = prevx = x;
    pathpoints[pathpointnum++].y = prevy = y;
}

void AM_ToggleRotateMode(void)
{
    if ((am_rotatemode = !am_rotatemode))
    {
        C_StrCVAROutput(stringize(am_rotatemode), "on");
        C_Output(s_AMSTR_ROTATEON);
        HU_SetPlayerMessage(s_AMSTR_ROTATEON, false, true);
    }
    else
    {
        C_StrCVAROutput(stringize(am_rotatemode), "off");
        C_Output(s_AMSTR_ROTATEOFF);
        HU_SetPlayerMessage(s_AMSTR_ROTATEOFF, false, true);
    }

    message_dontfuckwithme = true;
    M_SaveCVARs();
}

//
// Handle events (user inputs) in automap mode
//
dboolean AM_Responder(const event_t *ev)
{
    int rc = false;

    direction = 0;
    modstate = SDL_GetModState();

    if (!menuactive && !paused)
    {
        static dboolean backbuttondown;

        if (!(gamepadbuttons & gamepadautomap))
            backbuttondown = false;

        if (!automapactive && !mapwindow)
        {
            if ((ev->type == ev_keydown && ev->data1 == AM_STARTKEY && keydown != AM_STARTKEY && !(modstate & KMOD_ALT))
                || (ev->type == ev_gamepad && (gamepadbuttons & gamepadautomap) && !backbuttondown))
            {
                keydown = AM_STARTKEY;
                backbuttondown = true;
                AM_Start(true);
                viewactive = false;
                rc = true;
                D_FadeScreen();
            }
        }
        else
        {
            int key;

            if (ev->type == ev_keydown)
            {
                rc = true;
                key = ev->data1;

                // pan right
                if (key == AM_PANRIGHTKEY || key == AM_PANRIGHTKEY2 || key == AM_PANRIGHTKEY3)
                {
                    keydown = key;

                    if (am_followmode)
                    {
                        m_paninc.x = 0;
                        rc = false;
                    }
                    else
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = FTOM(F_PANINC);
                    }
                }

                // pan left
                else if (key == AM_PANLEFTKEY || key == AM_PANLEFTKEY2 || key == AM_PANLEFTKEY3)
                {
                    keydown = key;

                    if (am_followmode)
                    {
                        m_paninc.x = 0;
                        rc = false;
                    }
                    else
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = -FTOM(F_PANINC);
                    }
                }

                // pan up
                else if (key == AM_PANUPKEY || key == AM_PANUPKEY2)
                {
                    keydown = key;

                    if (am_followmode)
                    {
                        m_paninc.y = 0;
                        rc = false;
                    }
                    else
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = FTOM(F_PANINC);
                    }
                }

                // pan down
                else if (key == AM_PANDOWNKEY || key == AM_PANDOWNKEY2)
                {
                    keydown = key;

                    if (am_followmode)
                    {
                        m_paninc.y = 0;
                        rc = false;
                    }
                    else
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = -FTOM(F_PANINC);
                    }
                }

                // zoom out
                else if (key == AM_ZOOMOUTKEY && !movement)
                {
                    keydown = key;
                    AM_ToggleZoomOut();
                }

                // zoom in
                else if (key == AM_ZOOMINKEY && !movement)
                {
                    keydown = key;
                    AM_ToggleZoomIn();
                }

                // leave automap
                else if (key == AM_ENDKEY && !(modstate & KMOD_ALT) && keydown != AM_ENDKEY && !mapwindow)
                {
                    keydown = key;
                    viewactive = true;
                    AM_Stop();
                    D_FadeScreen();
                }

                // toggle maximum zoom
                else if (key == AM_GOBIGKEY && !idclev && !idmus)
                {
                    if (keydown != AM_GOBIGKEY)
                    {
                        keydown = key;
                        AM_ToggleMaxZoom();
                    }
                }

                // toggle follow mode
                else if (key == AM_FOLLOWKEY && !mapwindow)
                {
                    if (keydown != AM_FOLLOWKEY)
                    {
                        keydown = key;
                        AM_ToggleFollowMode();
                    }
                }

                // toggle grid
                else if (key == AM_GRIDKEY)
                {
                    if (keydown != AM_GRIDKEY)
                    {
                        keydown = key;
                        AM_ToggleGrid();
                    }
                }

                // mark spot
                else if (key == AM_MARKKEY)
                {
                    if (keydown != AM_MARKKEY)
                    {
                        keydown = key;
                        AM_AddMark();
                    }
                }

                // clear mark(s)
                else if (key == AM_CLEARMARKKEY)
                    AM_ClearMarks();

                // toggle rotate mode
                else if (key == AM_ROTATEKEY)
                {
                    if (keydown != AM_ROTATEKEY)
                    {
                        keydown = key;
                        AM_ToggleRotateMode();
                    }
                }
                else
                    rc = false;
            }
            else if (ev->type == ev_keyup)
            {
                key = ev->data1;

                if (key == AM_CLEARMARKKEY)
                    markpress = 0;

                keydown = 0;

                if ((key == AM_ZOOMOUTKEY || key == AM_ZOOMINKEY) && !movement)
                {
                    mtof_zoommul = FRACUNIT;
                    ftom_zoommul = FRACUNIT;
                }
                else if (key == AM_FOLLOWKEY)
                {
                    int key2 = 0;

                    if (keystate(AM_PANLEFTKEY))
                        key2 = AM_PANLEFTKEY;
                    else if (keystate(AM_PANLEFTKEY2))
                        key2 = AM_PANLEFTKEY2;
                    else if (keystate(AM_PANLEFTKEY3))
                        key2 = AM_PANLEFTKEY3;
                    else if (keystate(AM_PANRIGHTKEY))
                        key2 = AM_PANRIGHTKEY;
                    else if (keystate(AM_PANRIGHTKEY2))
                        key2 = AM_PANRIGHTKEY2;
                    else if (keystate(AM_PANRIGHTKEY3))
                        key2 = AM_PANRIGHTKEY3;
                    else if (keystate(AM_PANUPKEY))
                        key2 = AM_PANUPKEY;
                    else if (keystate(AM_PANUPKEY2))
                        key2 = AM_PANUPKEY2;
                    else if (keystate(AM_PANDOWNKEY))
                        key2 = AM_PANDOWNKEY;
                    else if (keystate(AM_PANDOWNKEY2))
                        key2 = AM_PANDOWNKEY2;

                    if (key2)
                    {
                        event_t event;

                        event.type = ev_keydown;
                        event.data1 = key2;
                        event.data2 = 0;
                        D_PostEvent(&event);
                    }
                }
                else if (!am_followmode)
                {
                    if (key == AM_PANLEFTKEY || key == AM_PANLEFTKEY2 || key == AM_PANLEFTKEY3)
                    {
                        speedtoggle = AM_GetSpeedToggle();

                        if (keystate(AM_PANRIGHTKEY) || keystate(AM_PANRIGHTKEY2) || keystate(AM_PANRIGHTKEY3))
                            m_paninc.x = FTOM(F_PANINC);
                        else
                            m_paninc.x = 0;
                    }
                    else if (key == AM_PANRIGHTKEY || key == AM_PANRIGHTKEY2 || key == AM_PANRIGHTKEY3)
                    {
                        speedtoggle = AM_GetSpeedToggle();

                        if (keystate(AM_PANLEFTKEY) || keystate(AM_PANLEFTKEY2) || keystate(AM_PANLEFTKEY3))
                            m_paninc.x = -FTOM(F_PANINC);
                        else
                            m_paninc.x = 0;
                    }
                    else if (key == AM_PANUPKEY || key == AM_PANUPKEY2)
                    {
                        speedtoggle = AM_GetSpeedToggle();

                        if (keystate(AM_PANDOWNKEY) || keystate(AM_PANDOWNKEY2))
                            m_paninc.y = FTOM(F_PANINC);
                        else
                            m_paninc.y = 0;
                    }
                    else if (key == AM_PANDOWNKEY || key == AM_PANDOWNKEY2)
                    {
                        speedtoggle = AM_GetSpeedToggle();

                        if (keystate(AM_PANUPKEY) || keystate(AM_PANUPKEY2))
                            m_paninc.y = -FTOM(F_PANINC);
                        else
                            m_paninc.y = 0;
                    }
                }
            }
            else if (ev->type == ev_mousewheel)
            {
                // zoom in
                if (ev->data1 > 0)
                {
                    movement = true;
                    speedtoggle = AM_GetSpeedToggle();
                    mtof_zoommul = M_ZOOMIN + 2000;
                    ftom_zoommul = M_ZOOMOUT - 2000;
                    bigstate = false;
                }

                // zoom out
                else if (ev->data1 < 0)
                {
                    movement = true;
                    speedtoggle = AM_GetSpeedToggle();
                    mtof_zoommul = M_ZOOMOUT - 2000;
                    ftom_zoommul = M_ZOOMIN + 2000;
                }
            }
            else if (ev->type == ev_gamepad && gamepadwait < I_GetTime())
            {
                if ((gamepadbuttons & gamepadautomap) && !backbuttondown)
                {
                    gamepadwait = I_GetTime() + 8;
                    viewactive = true;
                    backbuttondown = true;
                    AM_Stop();
                    D_FadeScreen();
                }

                // zoom out
                else if ((gamepadbuttons & gamepadautomapzoomout) && !(gamepadbuttons & gamepadautomapzoomin))
                {
                    movement = true;
                    AM_ToggleZoomOut();
                }

                // zoom in
                else if ((gamepadbuttons & gamepadautomapzoomin) && !(gamepadbuttons & gamepadautomapzoomout))
                {
                    movement = true;
                    AM_ToggleZoomIn();
                }

                // toggle maximum zoom
                else if ((gamepadbuttons & gamepadautomapmaxzoom) && !idclev && !idmus)
                {
                    AM_ToggleMaxZoom();
                    gamepadwait = I_GetTime() + 12;
                }

                // toggle follow mode
                else if (gamepadbuttons & gamepadautomapfollowmode)
                {
                    AM_ToggleFollowMode();
                    gamepadwait = I_GetTime() + 12;
                }

                // toggle grid
                else if (gamepadbuttons & gamepadautomapgrid)
                {
                    AM_ToggleGrid();
                    gamepadwait = I_GetTime() + 12;
                }

                // mark spot
                else if ((gamepadbuttons & gamepadautomapmark))
                {
                    AM_AddMark();
                    gamepadwait = I_GetTime() + 12;
                }

                // clear mark(s)
                else if (gamepadbuttons & gamepadautomapclearmark)
                {
                    AM_ClearMarks();
                    gamepadwait = I_GetTime() + 12;
                }

                // toggle rotate mode
                else if (gamepadbuttons & gamepadautomaprotatemode)
                {
                    AM_ToggleRotateMode();
                    gamepadwait = I_GetTime() + 12;
                }

                if (!am_followmode)
                {
                    // pan right with left thumbstick
                    if (gamepadthumbLX > 0)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = (fixed_t)(FTOM(F_PANINC) * gamepadthumbLXright * 1.2f);
                    }

                    // pan left with left thumbstick
                    else if (gamepadthumbLX < 0)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = -(fixed_t)(FTOM(F_PANINC) * gamepadthumbLXleft * 1.2f);
                    }

                    // pan right with right thumbstick
                    if (gamepadthumbRX > 0 && gamepadthumbRX > gamepadthumbLX)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = (fixed_t)(FTOM(F_PANINC) * gamepadthumbRXright * 1.2f);
                    }

                    // pan left with right thumbstick
                    else if (gamepadthumbRX < 0 && gamepadthumbRX < gamepadthumbLX)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = -(fixed_t)(FTOM(F_PANINC) * gamepadthumbRXleft * 1.2f);
                    }

                    // pan up with left thumbstick
                    if (gamepadthumbLY < 0)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = (fixed_t)(FTOM(F_PANINC) * gamepadthumbLYup * 1.2f);
                    }

                    // pan down with left thumbstick
                    else if (gamepadthumbLY > 0)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = -(fixed_t)(FTOM(F_PANINC) * gamepadthumbLYdown * 1.2f);
                    }

                    // pan up with right thumbstick
                    if (gamepadthumbRY < 0 && gamepadthumbRY < gamepadthumbLY)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = -(fixed_t)(FTOM(F_PANINC) * gamepadthumbRYup * 1.2f);
                    }

                    // pan down with right thumbstick
                    else if (gamepadthumbRY > 0 && gamepadthumbRY > gamepadthumbLY)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = -(fixed_t)(FTOM(F_PANINC) * gamepadthumbRYdown * 1.2f);
                    }
                }

                if (!movement)
                {
                    m_paninc.x = 0;
                    m_paninc.y = 0;
                    mtof_zoommul = FRACUNIT;
                    ftom_zoommul = FRACUNIT;
                }
                else
                    movement = false;
            }

            if ((viewplayer->cheats & CF_MYPOS) && !am_followmode && (m_paninc.x || m_paninc.y))
            {
                double  x = m_paninc.x;
                double  y = m_paninc.y;

                if ((m_x == min_x - m_w / 2 && x < 0.0) || (m_x == max_x - m_w / 2 && x > 0.0))
                    x = 0.0;

                if ((m_y == min_y - m_h / 2 && y < 0.0) || (m_y == max_y - m_h / 2 && y > 0.0))
                    y = 0.0;

                if ((direction = (int)(atan2(y, x) * 180.0 / M_PI)) < 0)
                    direction += 360;
            }
        }
    }

    return rc;
}

//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
static void AM_Rotate(fixed_t *x, fixed_t *y, angle_t angle)
{
    const fixed_t   cosine = finecosine[(angle >>= ANGLETOFINESHIFT)];
    const fixed_t   sine = finesine[angle];
    const fixed_t   temp = FixedMul(*x, cosine) - FixedMul(*y, sine);

    *y = FixedMul(*x, sine) + FixedMul(*y, cosine);
    *x = temp;
}

static void AM_RotatePoint(mpoint_t *point)
{
    fixed_t         temp;
    const fixed_t   x = am_frame.center.x;
    const fixed_t   y = am_frame.center.y;

    point->x -= x;
    point->y -= y;
    temp = FixedMul(point->x, am_frame.cos) - FixedMul(point->y, am_frame.sin) + x;
    point->y = FixedMul(point->x, am_frame.sin) + FixedMul(point->y, am_frame.cos) + y;
    point->x = temp;
}

//
// Zooming
//
static void AM_ChangeWindowScale(void)
{
    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    if (scale_mtof < min_scale_mtof)
        AM_MinOutWindowScale();
    else if (scale_mtof > max_scale_mtof)
        AM_MaxOutWindowScale();
    else
        AM_ActivateNewScale();
}

static void AM_DoFollowPlayer(void)
{
    mobj_t  *mo = viewplayer->mo;

    m_x = (mo->x >> FRACTOMAPBITS) - m_w / 2;
    m_y = (mo->y >> FRACTOMAPBITS) - m_h / 2;
}

//
// Updates on Game Tic
//
void AM_Ticker(void)
{
    if (mapwindow)
    {
        AM_DoFollowPlayer();
        return;
    }

    if (!automapactive)
        return;

    if (am_followmode || menuactive)
        AM_DoFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
        AM_ChangeWindowScale();

    // Change x,y location
    if ((m_paninc.x || m_paninc.y) && !menuactive && !paused && !consoleactive)
        AM_ChangeWindowLoc();

    if (movement)
    {
        movement = false;
        m_paninc.x = 0;
        m_paninc.y = 0;
        mtof_zoommul = FRACUNIT;
        ftom_zoommul = FRACUNIT;
    }
}

//
// Clear automap frame buffer.
//
void AM_ClearFB(void)
{
    memset(mapscreen, backcolor, maparea);
}

//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly faster reject and precalculated
// slopes. If the speed is needed, use a hash algorithm to handle the common cases.
static dboolean AM_ClipMline(int *x0, int *y0, int *x1, int *y1)
{
    enum
    {
        LEFT = 1,
        RIGHT = 2,
        TOP = 4,
        BOTTOM = 8
    };

    unsigned int    outcode1 = 0;
    unsigned int    outcode2 = 0;

    *x0 = CXMTOF(*x0);
    *x1 = CXMTOF(*x1);

    if (*x0 < 0)
        outcode1 = LEFT;
    else if (*x0 >= MAPWIDTH)
        outcode1 = RIGHT;

    if (*x1 < 0)
        outcode2 = LEFT;
    else if (*x1 >= MAPWIDTH)
        outcode2 = RIGHT;

    if (outcode1 & outcode2)
        return false;

    *y0 = CYMTOF(*y0);
    *y1 = CYMTOF(*y1);

    if (!((*x0 - *x1) | (*y0 - *y1)))
        return false;

    if (*y0 < 0)
        outcode1 |= TOP;
    else if (*y0 >= (int)mapheight)
        outcode1 |= BOTTOM;

    if (*y1 < 0)
        outcode2 |= TOP;
    else if (*y1 >= (int)mapheight)
        outcode2 |= BOTTOM;

    return !(outcode1 & outcode2);
}

static inline void _PUTDOT(byte *dot, byte *color)
{
    *dot = *(*dot + color);
}

static inline void PUTDOT(unsigned int x, unsigned int y, byte *color)
{
    if (x < MAPWIDTH && y < maparea)
    {
        byte    *dot = mapscreen + y + x;

        *dot = *(*dot + color);
    }
}

static inline void PUTDOT2(unsigned int x, unsigned int y, byte *color)
{
    if (x < MAPWIDTH && y < maparea)
        *(mapscreen + y + x) = *color;
}

static inline void PUTBIGDOT(unsigned int x, unsigned int y, byte *color)
{
    if (x < MAPWIDTH)
    {
        byte            *dot = mapscreen + y + x;
        const dboolean  attop = (y < maparea);
        const dboolean  atbottom = (y < mapbottom);

        if (attop)
            *dot = *(*dot + color);

        if (atbottom)
            _PUTDOT(dot + MAPWIDTH, color);

        if (x + 1 < MAPWIDTH)
        {
            if (attop)
                _PUTDOT(dot + 1, color);

            if (atbottom)
            {
                dot += MAPWIDTH + 1;
                *dot = *(*dot + color);
            }
        }
    }
    else if (++x < MAPWIDTH)
    {
        byte    *dot = mapscreen + y + x;

        if (y < maparea)
            *dot = *(*dot + color);

        if (y < mapbottom)
        {
            dot += MAPWIDTH;
            *dot = *(*dot + color);
        }
    }
}

static inline void PUTTRANSLUCENTDOT(unsigned int x, unsigned int y, byte *color)
{
    if (x < MAPWIDTH && y < maparea)
    {
        byte    *dot = mapscreen + y + x;

        if (*dot != tinttab66[*color])
            *dot = tinttab66[(*dot << 8) + *color];
    }
}

//
// Classic Bresenham w/ whatever optimizations needed for speed
//
static void AM_DrawFline(int x0, int y0, int x1, int y1, byte *color,
    void (*putdot)(unsigned int, unsigned int, byte *))
{
    if (AM_ClipMline(&x0, &y0, &x1, &y1))
    {
        int dx = x1 - x0;
        int dy = y1 - y0;

        if (!dy)
        {
            // horizontal line
            const int   sx = SIGN(dx);

            x0 = BETWEEN(-1, x0, MAPWIDTH - 1);
            x1 = BETWEEN(-1, x1, MAPWIDTH - 1);

            y0 *= MAPWIDTH;

            putdot(x0, y0, color);

            while (x0 != x1)
                putdot((x0 += sx), y0, color);
        }
        else if (!dx)
        {
            // vertical line
            const int   sy = SIGN(dy) * MAPWIDTH;

            y0 = BETWEEN(-MAPWIDTH, y0 * MAPWIDTH, mapbottom);
            y1 = BETWEEN(-MAPWIDTH, y1 * MAPWIDTH, mapbottom);

            putdot(x0, y0, color);

            while (y0 != y1)
                putdot(x0, (y0 += sy), color);
        }
        else
        {
            const int   sx = SIGN(dx);
            const int   sy = SIGN(dy) * MAPWIDTH;

            dx = ABS(dx);
            dy = ABS(dy);
            y0 *= MAPWIDTH;
            putdot(x0, y0, color);

            if (dx == dy)
            {
                // diagonal line
                while (x0 != x1)
                    putdot((x0 += sx), (y0 += sy), color);
            }
            else if (dx > dy)
            {
                // x-major line
                int error = (dy <<= 1) - dx;

                dx <<= 1;

                while (x0 != x1)
                {
                    const int   mask = ~(error >> 31);

                    putdot((x0 += sx), (y0 += (sy & mask)), color);
                    error += dy - (dx & mask);
                }
            }
            else
            {
                // y-major line
                int error = (dx <<= 1) - dy;

                dy <<= 1;
                y1 *= MAPWIDTH;

                while (y0 != y1)
                {
                    const int   mask = ~(error >> 31);

                    putdot((x0 += (sx & mask)), (y0 += sy), color);
                    error += dx - (dy & mask);
                }
            }
        }
    }
}

static mline_t (*rotatelinefunc)(mline_t);

static mline_t AM_RotateLine(mline_t mline)
{
    AM_RotatePoint(&mline.a);
    AM_RotatePoint(&mline.b);
    return mline;
}

static mline_t AM_DoNotRotateLine(mline_t mline)
{
    return mline;
}

//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
static void AM_DrawGrid(void)
{
    const fixed_t   minlen = (fixed_t)(sqrt((double)m_w * m_w + (double)m_h * m_h));
    const fixed_t   startx = m_x - (minlen - m_w) / 2;
    const fixed_t   starty = m_y - (minlen - m_h) / 2;
    fixed_t         end = startx + minlen;

    // Draw vertical gridlines
    for (fixed_t x = startx - ((startx - (bmaporgx >> FRACTOMAPBITS)) % gridwidth); x < end; x += gridwidth)
    {
        mline_t mline = { { x, starty }, { x, starty + minlen } };

        mline = rotatelinefunc(mline);
        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, gridcolor, &PUTDOT);
    }

    end = starty + minlen;

    // Draw horizontal gridlines
    for (fixed_t y = starty - ((starty - (bmaporgy >> FRACTOMAPBITS)) % gridheight); y < end; y += gridheight)
    {
        mline_t mline = { { startx, y }, { startx + minlen, y } };

        mline = rotatelinefunc(mline);
        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, gridcolor, &PUTDOT);
    }
}

static void AM_DrawWalls(void)
{
    for (int i = 0; i < numlines; i++)
    {
        const line_t    line = lines[i];
        const fixed_t   *lbbox = line.bbox;
        const fixed_t   *ambbox = am_frame.bbox;

        if ((lbbox[BOXLEFT] >> FRACTOMAPBITS) <= ambbox[BOXRIGHT] && (lbbox[BOXRIGHT] >> FRACTOMAPBITS) >= ambbox[BOXLEFT]
            && (lbbox[BOXBOTTOM] >> FRACTOMAPBITS) <= ambbox[BOXTOP] && (lbbox[BOXTOP] >> FRACTOMAPBITS) >= ambbox[BOXBOTTOM])
        {
            const unsigned short    flags = line.flags;

            if (!(flags & ML_DONTDRAW) && (flags & ML_MAPPED))
            {
                const sector_t  *back = line.backsector;
                mline_t         mline;

                mline.a.x = line.v1->x >> FRACTOMAPBITS;
                mline.a.y = line.v1->y >> FRACTOMAPBITS;
                mline.b.x = line.v2->x >> FRACTOMAPBITS;
                mline.b.y = line.v2->y >> FRACTOMAPBITS;

                mline = rotatelinefunc(mline);

                if (isteleportline[line.special] && back && back->ceilingheight != back->floorheight
                    && ((flags & ML_TELEPORTTRIGGERED) || isteleport[back->floorpic]) && !(flags & ML_SECRET))
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, teleportercolor, &PUTDOT);
                else if (!back || (flags & ML_SECRET))
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, wallcolor, putbigdot);
                else
                {
                    const sector_t  *front = line.frontsector;

                    if (back->floorheight != front->floorheight)
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, fdwallcolor, &PUTDOT);
                    else if (back->ceilingheight != front->ceilingheight)
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, cdwallcolor, &PUTDOT);
                }
            }
        }
    }
}

static void AM_DrawWalls_AllMap(void)
{
    for (int i = 0; i < numlines; i++)
    {
        const line_t    line = lines[i];
        const fixed_t   *lbbox = line.bbox;
        const fixed_t   *ambbox = am_frame.bbox;

        if ((lbbox[BOXLEFT] >> FRACTOMAPBITS) <= ambbox[BOXRIGHT]
            && (lbbox[BOXRIGHT] >> FRACTOMAPBITS) >= ambbox[BOXLEFT]
            && (lbbox[BOXBOTTOM] >> FRACTOMAPBITS) <= ambbox[BOXTOP]
            && (lbbox[BOXTOP] >> FRACTOMAPBITS) >= ambbox[BOXBOTTOM])
        {
            const unsigned short    flags = line.flags;

            if (!(flags & ML_DONTDRAW))
            {
                const sector_t  *back = line.backsector;
                mline_t         mline;

                mline.a.x = line.v1->x >> FRACTOMAPBITS;
                mline.a.y = line.v1->y >> FRACTOMAPBITS;
                mline.b.x = line.v2->x >> FRACTOMAPBITS;
                mline.b.y = line.v2->y >> FRACTOMAPBITS;

                mline = rotatelinefunc(mline);

                if (isteleportline[line.special] && ((flags & ML_TELEPORTTRIGGERED) || (back && isteleport[back->floorpic])))
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y,
                        ((flags & ML_MAPPED) ? teleportercolor : allmapfdwallcolor), &PUTDOT);
                else if (!back || (flags & ML_SECRET))
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y,
                        ((flags & ML_MAPPED) ? wallcolor : allmapwallcolor), putbigdot);
                else
                {
                    const sector_t  *front = line.frontsector;

                    if (back->floorheight != front->floorheight)
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y,
                            ((flags & ML_MAPPED) ? fdwallcolor : allmapfdwallcolor), &PUTDOT);
                    else if (back->ceilingheight != front->ceilingheight)
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y,
                            ((flags & ML_MAPPED) ? cdwallcolor : allmapcdwallcolor), &PUTDOT);
                    else
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, tswallcolor, &PUTDOT);
                }
            }
        }
    }
}

static void AM_DrawWalls_Cheating(void)
{
    for (int i = 0; i < numlines; i++)
    {
        const line_t    line = lines[i];
        const fixed_t   *lbbox = line.bbox;
        const fixed_t   *ambbox = am_frame.bbox;

        if ((lbbox[BOXLEFT] >> FRACTOMAPBITS) <= ambbox[BOXRIGHT]
            && (lbbox[BOXRIGHT] >> FRACTOMAPBITS) >= ambbox[BOXLEFT]
            && (lbbox[BOXBOTTOM] >> FRACTOMAPBITS) <= ambbox[BOXTOP]
            && (lbbox[BOXTOP] >> FRACTOMAPBITS) >= ambbox[BOXBOTTOM])
        {
            mline_t mline;

            mline.a.x = line.v1->x >> FRACTOMAPBITS;
            mline.a.y = line.v1->y >> FRACTOMAPBITS;
            mline.b.x = line.v2->x >> FRACTOMAPBITS;
            mline.b.y = line.v2->y >> FRACTOMAPBITS;

            mline = rotatelinefunc(mline);

            if (isteleportline[line.special])
                AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, teleportercolor, &PUTDOT);
            else
            {
                const sector_t  *back = line.backsector;

                if (!back)
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, wallcolor, putbigdot);
                else
                {
                    const sector_t *front = line.frontsector;

                    if (back->floorheight != front->floorheight)
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, fdwallcolor, &PUTDOT);
                    else if (back->ceilingheight != front->ceilingheight)
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, cdwallcolor, &PUTDOT);
                    else
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, tswallcolor, &PUTDOT);
                }
            }
        }
    }
}

static void AM_DrawLineCharacter(const mline_t *lineguy, const int lineguylines,
    const fixed_t scale, angle_t angle, byte color, fixed_t x, fixed_t y)
{
    for (int i = 0; i < lineguylines; i++)
    {
        int     x1, y1;
        int     x2, y2;
        mline_t line = lineguy[i];

        if (scale)
        {
            x1 = FixedMul(line.a.x, scale);
            y1 = FixedMul(line.a.y, scale);
            x2 = FixedMul(line.b.x, scale);
            y2 = FixedMul(line.b.y, scale);
        }
        else
        {
            x1 = line.a.x;
            y1 = line.a.y;
            x2 = line.b.x;
            y2 = line.b.y;
        }

        AM_Rotate(&x1, &y1, angle);
        AM_Rotate(&x2, &y2, angle);

        AM_DrawFline(x + x1, y + y1, x + x2, y + y2, &color, &PUTDOT2);
    }
}

static void AM_DrawTranslucentLineCharacter(const mline_t *lineguy, const int lineguylines,
    const fixed_t scale, angle_t angle, byte *color, const fixed_t x, const fixed_t y)
{
    for (int i = 0; i < lineguylines; i++)
    {
        int     x1, y1;
        int     x2, y2;
        mline_t line = lineguy[i];

        if (scale)
        {
            x1 = FixedMul(line.a.x, scale);
            y1 = FixedMul(line.a.y, scale);
            x2 = FixedMul(line.b.x, scale);
            y2 = FixedMul(line.b.y, scale);
        }
        else
        {
            x1 = line.a.x;
            y1 = line.a.y;
            x2 = line.b.x;
            y2 = line.b.y;
        }

        AM_Rotate(&x1, &y1, angle);
        AM_Rotate(&x2, &y2, angle);

        AM_DrawFline(x + x1, y + y1, x + x2, y + y2, color, &PUTTRANSLUCENTDOT);
    }
}

#define PLAYERARROWLINES        8
#define CHEATPLAYERARROWLINES   19

static void AM_DrawPlayer(void)
{
    const mline_t playerarrow[PLAYERARROWLINES] =
    {
        { { -57275,      0 }, { -39652,      0 } }, //  -
        { { -39652,      0 }, {  74898,      0 } }, //  -------
        { {  74898,      0 }, {  39652,  17623 } }, //  ------>
        { {  74898,      0 }, {  39652, -17623 } },
        { { -57275,      0 }, { -74898,  17623 } }, // >------>
        { { -57275,      0 }, { -74898, -17623 } },
        { { -39652,      0 }, { -57275,  17623 } }, // >>----->
        { { -39652,      0 }, { -57275, -17623 } }
    };

    const mline_t cheatplayerarrow[CHEATPLAYERARROWLINES] =
    {
        { { -57275,      0 }, { -39652,      0 } }, //  -
        { { -39652,      0 }, { -30840,      0 } }, //  --
        { { -30840,      0 }, {  -7343,      0 } }, //  ---
        { {  -7343,      0 }, {  74898,      0 } }, //  -------
        { {  74898,      0 }, {  39652,  11748 } }, //  ------>
        { {  74898,      0 }, {  39652, -11748 } },
        { { -57275,      0 }, { -74898,  11748 } }, // >------>
        { { -57275,      0 }, { -74898, -11748 } },
        { { -39652,      0 }, { -57275,  11748 } }, // >>----->
        { { -39652,      0 }, { -57275, -11748 } },
        { { -30840,      0 }, { -30840, -11748 } }, // >>-d--->
        { { -30840, -11748 }, { -19091, -11748 } },
        { { -19091, -11748 }, { -19091,  17623 } },
        { {  -7343,      0 }, {  -7343, -11748 } }, // >>-dd-->
        { {  -7343, -11748 }, {   4405, -11748 } },
        { {   4405, -11748 }, {   4405,  17623 } },
        { {  16154,  17623 }, {  16154, -10070 } }, // >>-ddt->
        { {  16154, -10070 }, {  18357, -12273 } },
        { {  18357, -12273 }, {  23203, -10070 } }
    };

    const int   invisibility = viewplayer->powers[pw_invisibility];
    mpoint_t    point;
    angle_t     angle;
    mobj_t      *mo = viewplayer->mo;

    point.x = mo->x >> FRACTOMAPBITS;
    point.y = mo->y >> FRACTOMAPBITS;

    if (am_rotatemode)
    {
        AM_RotatePoint(&point);
        angle = ANG90;
    }
    else
        angle = viewangle;

    if (viewplayer->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS))
    {
        if (invisibility > STARTFLASHING || (invisibility & 8))
            AM_DrawTranslucentLineCharacter(cheatplayerarrow, CHEATPLAYERARROWLINES, 0, angle, &playercolor, point.x, point.y);
        else
            AM_DrawLineCharacter(cheatplayerarrow, CHEATPLAYERARROWLINES, 0, angle, playercolor, point.x, point.y);
    }
    else if (invisibility > STARTFLASHING || (invisibility & 8))
        AM_DrawTranslucentLineCharacter(playerarrow, PLAYERARROWLINES, 0, angle, &playercolor, point.x, point.y);
    else
        AM_DrawLineCharacter(playerarrow, PLAYERARROWLINES, 0, angle, playercolor, point.x, point.y);
}

#define THINGTRIANGLELINES  3

static void AM_DrawThings(void)
{
    const mline_t thingtriangle[THINGTRIANGLELINES] =
    {
        { { -32768, -45875 }, {  65536,      0 } },
        { {  65536,      0 }, { -32768,  45875 } },
        { { -32768,  45875 }, { -32768, -45875 } }
    };

    angle_t angleoffset = viewangle - ANG90;

    for (int i = 0; i < numsectors; i++)
    {
        // e6y
        // Two-pass method for better usability of automap:
        // The first one will draw all things except enemies
        // The second one is for enemies only
        // Stop after first pass if the current sector has no enemies
        for (int pass = 0, enemies = 0; pass < 2; pass += (enemies ? 1 : 2))
        {
            mobj_t  *thing = sectors[i].thinglist;

            while (thing)
            {
                // e6y: stop if all enemies from current sector already have been drawn
                if (pass && !enemies)
                    break;

                if (pass == ((thing->flags & (MF_SHOOTABLE | MF_CORPSE)) == MF_SHOOTABLE ? (!pass ? enemies++ : enemies--), 0 : 1))
                {
                    thing = thing->snext;
                    continue;
                }

                if ((!thing->player || thing->player->mo != thing) && !(thing->flags2 & MF2_DONTMAP))
                {
                    mpoint_t    point;
                    angle_t     angle = thing->angle;
                    int         fx, fy;
                    const short lump = sprites[thing->sprite].spriteframes[0].lump[0];
                    const int   width = (BETWEEN(24 << FRACBITS, MIN(spritewidth[lump], spriteheight[lump]),
                                    96 << FRACBITS) >> FRACTOMAPBITS) / 2;

                    point.x = (thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic)) >> FRACTOMAPBITS;
                    point.y = (thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic)) >> FRACTOMAPBITS;

                    if (am_rotatemode)
                    {
                        AM_RotatePoint(&point);
                        angle -= angleoffset;
                    }

                    fx = CXMTOF(point.x);
                    fy = CYMTOF(point.y);

                    if (fx >= -width && fx <= MAPWIDTH + width && fy >= -width && fy <= (int)mapheight + width)
                        AM_DrawLineCharacter(thingtriangle, THINGTRIANGLELINES, width, angle, thingcolor, point.x, point.y);
                }

                thing = thing->snext;
            }
        }
    }
}

#define MARKWIDTH   9
#define MARKHEIGHT  12

static void AM_DrawMarks(void)
{
    const char *marknums[] =
    {
        "011111100112222110122222210122112210122112210122112210"
        "122112210122112210122112210122222210112222110011111100",
        "001111000011221000012221000012221000011221000001221000"
        "001221000001221000011221100012222100012222100011111100",
        "011111100112222110122222210122112210111112210011222210"
        "112222110122211100122111110122222210122222210111111110",
        "011111100112222110122222210122112210111112210001222110"
        "001222210111112210122112210122222210112222110011111100",
        "000111100000122100001122100001221100011221110012212210"
        "112212211122222221122222221111112211000012210000011110",
        "111111110122222210122222210122111110122111100122222110"
        "122222210111112210122112210122222210112222110011111100",
        "011111100112222110122222210122112210122111110122222110"
        "122222210122112210122112210122222210112222110011111100",
        "111111110122222210122222210111112210000122110001122100"
        "001221100011221000012211000012210000012210000011110000",
        "011111100112222110122222210122112210122112210112222110"
        "122222210122112210122112210122222210112222110011111100",
        "011111100112222110122222210122112210122112210122222210"
        "112222210111112210122112210122222210112222110011111100"
    };

    for (int i = 0; i < markpointnum; i++)
    {
        int         number = i + 1;
        int         temp = number;
        int         digits = 1;
        int         x, y;
        mpoint_t    point = { markpoints[i].x, markpoints[i].y };

        if (am_rotatemode)
            AM_RotatePoint(&point);

        x = CXMTOF(point.x) - MARKWIDTH / 2 + 1;
        y = CYMTOF(point.y) - MARKHEIGHT / 2 - 1;

        while ((temp /= 10))
            digits++;

        x += (digits - 1) * MARKWIDTH / 2;
        x -= (number % 10 == 1);
        x -= (number / 10 == 1);

        do
        {
            const int   digit = number % 10;

            x += (i > 0 && digit == 1);

            for (int j = 0; j < MARKWIDTH * MARKHEIGHT; j++)
            {
                const unsigned int  fx = x + j % MARKWIDTH;

                if (fx < MAPWIDTH)
                {
                    const unsigned int  fy = y + j / MARKWIDTH;

                    if (fy < mapheight)
                    {
                        const char  src = marknums[digit][j];

                        if (src == '2')
                            mapscreen[fy * MAPWIDTH + fx] = markcolor;
                        else if (src == '1')
                        {
                            byte    *dest = &mapscreen[fy * MAPWIDTH + fx];

                            *dest = *(*dest + tinttab66);
                        }
                    }
                }
            }

            x -= MARKWIDTH - 1;
            number /= 10;
        } while (number > 0);
    }
}

static void AM_DrawPath(void)
{
    if (pathpointnum >= 1)
    {
        mpoint_t    end;

        if (am_rotatemode)
        {
            for (int i = 1; i < pathpointnum; i++)
            {
                mpoint_t    start = { pathpoints[i - 1].x >> FRACTOMAPBITS, pathpoints[i - 1].y >> FRACTOMAPBITS };

                end.x = pathpoints[i].x >> FRACTOMAPBITS;
                end.y = pathpoints[i].y >> FRACTOMAPBITS;

                if (ABS(start.x - end.x) > 4 * FRACUNIT || ABS(start.y - end.y) > 4 * FRACUNIT)
                    continue;

                AM_RotatePoint(&start);
                AM_RotatePoint(&end);
                AM_DrawFline(start.x, start.y, end.x, end.y, pathcolor, putbigdot);
            }

            if (pathpointnum > 1 && !freeze && !(viewplayer->cheats & CF_NOCLIP))
            {
                mobj_t      *mo = viewplayer->mo;
                mpoint_t    player = { mo->x >> FRACTOMAPBITS, mo->y >> FRACTOMAPBITS };

                AM_RotatePoint(&player);
                AM_DrawFline(end.x, end.y, player.x, player.y, pathcolor, putbigdot);
            }
        }
        else
        {
            for (int i = 1; i < pathpointnum; i++)
            {
                mpoint_t    start = { pathpoints[i - 1].x >> FRACTOMAPBITS, pathpoints[i - 1].y >> FRACTOMAPBITS };

                end.x = pathpoints[i].x >> FRACTOMAPBITS;
                end.y = pathpoints[i].y >> FRACTOMAPBITS;

                if (ABS(start.x - end.x) > 4 * FRACUNIT || ABS(start.y - end.y) > 4 * FRACUNIT)
                    continue;

                AM_DrawFline(start.x, start.y, end.x, end.y, pathcolor, putbigdot);
            }

            if (pathpointnum > 1 && !freeze && !(viewplayer->cheats & CF_NOCLIP))
            {
                mobj_t  *mo = viewplayer->mo;

                AM_DrawFline(end.x, end.y, mo->x >> FRACTOMAPBITS, mo->y >> FRACTOMAPBITS, pathcolor, putbigdot);
            }
        }
    }
}

static inline void AM_DrawScaledPixel(const int x, const int y, byte *color)
{
    byte    *dest = &mapscreen[(y * 2 - 1) * MAPWIDTH + x * 2 - 1];

    *dest = *(*dest + color);
    dest++;
    *dest = *(*dest + color);
    dest += MAPWIDTH;
    *dest = *(*dest + color);
    dest--;
    *dest = *(*dest + color);
}

static inline void AM_DrawSolidScaledPixel(const int x, const int y, byte color)
{
    byte    *dest = &mapscreen[(y * 2 - 1) * MAPWIDTH + x * 2 - 1];

    *(dest++) = color;
    *dest = color;
    *(dest += MAPWIDTH) = color;
    *(--dest) = color;
}

#define CENTERX VANILLAWIDTH / 2
#define CENTERY (VANILLAHEIGHT - VANILLASBARHEIGHT) / 2

static void AM_DrawCrosshair(void)
{
    AM_DrawScaledPixel(CENTERX - 2, CENTERY, am_crosshaircolor2);
    AM_DrawScaledPixel(CENTERX - 1, CENTERY, am_crosshaircolor2);
    AM_DrawScaledPixel(CENTERX, CENTERY, am_crosshaircolor2);
    AM_DrawScaledPixel(CENTERX + 1, CENTERY, am_crosshaircolor2);
    AM_DrawScaledPixel(CENTERX + 2, CENTERY, am_crosshaircolor2);
    AM_DrawScaledPixel(CENTERX, CENTERY - 2, am_crosshaircolor2);
    AM_DrawScaledPixel(CENTERX, CENTERY - 1, am_crosshaircolor2);
    AM_DrawScaledPixel(CENTERX, CENTERY + 1, am_crosshaircolor2);
    AM_DrawScaledPixel(CENTERX, CENTERY + 2, am_crosshaircolor2);
}

static void AM_DrawSolidCrosshair(void)
{
    AM_DrawSolidScaledPixel(CENTERX - 2, CENTERY, am_crosshaircolor);
    AM_DrawSolidScaledPixel(CENTERX - 1, CENTERY, am_crosshaircolor);
    AM_DrawSolidScaledPixel(CENTERX, CENTERY, am_crosshaircolor);
    AM_DrawSolidScaledPixel(CENTERX + 1, CENTERY, am_crosshaircolor);
    AM_DrawSolidScaledPixel(CENTERX + 2, CENTERY, am_crosshaircolor);
    AM_DrawSolidScaledPixel(CENTERX, CENTERY - 2, am_crosshaircolor);
    AM_DrawSolidScaledPixel(CENTERX, CENTERY - 1, am_crosshaircolor);
    AM_DrawSolidScaledPixel(CENTERX, CENTERY + 1, am_crosshaircolor);
    AM_DrawSolidScaledPixel(CENTERX, CENTERY + 2, am_crosshaircolor);
}

static void AM_SetFrameVariables(void)
{
    const fixed_t   dx = m_w / 2;
    const fixed_t   dy = m_h / 2;
    const fixed_t   x = m_x + dx;
    const fixed_t   y = m_y + dy;

    am_frame.center.x = x;
    am_frame.center.y = y;

    if (am_rotatemode || menuactive)
    {
        const int       angle = (ANG90 - viewangle) >> ANGLETOFINESHIFT;
        const fixed_t   r = (fixed_t)sqrt((double)dx * dx + (double)dy * dy);

        am_frame.sin = finesine[angle];
        am_frame.cos = finecosine[angle];

        am_frame.bbox[BOXLEFT] = x - r;
        am_frame.bbox[BOXRIGHT] = x + r;
        am_frame.bbox[BOXBOTTOM] = y - r;
        am_frame.bbox[BOXTOP] = y + r;

        rotatelinefunc = &AM_RotateLine;
    }
    else
    {
        am_frame.bbox[BOXLEFT] = m_x;
        am_frame.bbox[BOXRIGHT] = m_x + m_w;
        am_frame.bbox[BOXBOTTOM] = m_y;
        am_frame.bbox[BOXTOP] = m_y + m_h;

        rotatelinefunc = &AM_DoNotRotateLine;
    }
}

void AM_Drawer(void)
{
    AM_SetFrameVariables();
    AM_ClearFB();

    if (viewplayer->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS))
        AM_DrawWalls_Cheating();
    else if (viewplayer->powers[pw_allmap])
        AM_DrawWalls_AllMap();
    else
        AM_DrawWalls();

    if (am_grid)
        AM_DrawGrid();

    if (menuactive && !inhelpscreens)
        return;

    if (am_path)
        AM_DrawPath();

    if (viewplayer->cheats & CF_ALLMAP_THINGS)
        AM_DrawThings();

    if (markpointnum)
        AM_DrawMarks();

    AM_DrawPlayer();

    if (!am_followmode)
    {
        if (r_hud_translucency)
            AM_DrawCrosshair();
        else
            AM_DrawSolidCrosshair();
    }
}
