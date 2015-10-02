/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

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
  id Software LLC.

========================================================================
*/

#define _USE_MATH_DEFINES

#include <math.h>

#if defined(WIN32)
#include <Windows.h>
#include <Xinput.h>
#endif

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_video.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_local.h"
#include "SDL.h"
#include "st_stuff.h"
#include "v_video.h"
#include "z_zone.h"

#define BLACK                   0
#define WHITE                   4
#define DARKGRAY                5
#define BROWN                   64
#define GRAY                    96
#define GREEN                   112
#define YELLOW                  160
#define RED                     176
#define PINK                    251

// Automap colors
#define CROSSHAIRCOLOR          WHITE
#define MARKCOLOR               (GRAY + 4)
#define PLAYERCOLOR             WHITE
#define THINGCOLOR              GREEN
#define WALLCOLOR               RED
#define ALLMAPWALLCOLOR         (GRAY + 12)
#define MASKCOLOR               PINK
#define TELEPORTERCOLOR         (RED + 8)
#define FDWALLCOLOR             BROWN
#define ALLMAPFDWALLCOLOR       (GRAY + 14)
#define CDWALLCOLOR             YELLOW
#define ALLMAPCDWALLCOLOR       (GRAY + 10)
#define TSWALLCOLOR             (GRAY + 8)
#define GRIDCOLOR               DARKGRAY
#define BACKGROUNDCOLOR         BLACK

// Automap color priorities
#define PLAYERPRIORITY          12
#define THINGPRIORITY           11
#define WALLPRIORITY            10
#define ALLMAPWALLPRIORITY      9
#define MASKPRIORITY            8
#define CDWALLPRIORITY          7
#define ALLMAPCDWALLPRIORITY    6
#define FDWALLPRIORITY          5
#define ALLMAPFDWALLPRIORITY    4
#define TELEPORTERPRIORITY      3
#define TSWALLPRIORITY          2
#define GRIDPRIORITY            1

byte    *priorities;
byte    *mask;

byte    *playercolor;
byte    *thingcolor;
byte    *wallcolor;
byte    *allmapwallcolor;
byte    *maskcolor;
byte    *teleportercolor;
byte    *fdwallcolor;
byte    *allmapfdwallcolor;
byte    *cdwallcolor;
byte    *allmapcdwallcolor;
byte    *tswallcolor;
byte    *gridcolor;

#define AM_PANDOWNKEY           key_down
#define AM_PANDOWNKEY2          key_down2
#define AM_PANUPKEY             key_up
#define AM_PANUPKEY2            key_up2
#define AM_PANRIGHTKEY          key_right
#define AM_PANRIGHTKEY2         key_straferight
#define AM_PANRIGHTKEY3         key_straferight2
#define AM_PANLEFTKEY           key_left
#define AM_PANLEFTKEY2          key_strafeleft
#define AM_PANLEFTKEY3          key_strafeleft2
#define AM_ZOOMINKEY            key_automap_zoomin
#define AM_ZOOMOUTKEY           key_automap_zoomout
#define AM_STARTKEY             key_automap
#define AM_ENDKEY               key_automap
#define AM_GOBIGKEY             key_automap_maxzoom
#define AM_FOLLOWKEY            key_automap_followmode
#define AM_GRIDKEY              key_automap_grid
#define AM_MARKKEY              key_automap_mark
#define AM_CLEARMARKKEY         key_automap_clearmark
#define AM_ROTATEKEY            key_automap_rotatemode

// scale on entry
// [BH] changed to initial zoom level of E1M1: Hangar so each map zoom level is consistent
#define INITSCALEMTOF           125114
// how much the automap moves window per tic in map coordinates
// moves 140 pixels in 1 second
#define F_PANINC                (8 << speedtoggle)
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN                ((int)((float)FRACUNIT * (1.00f + F_PANINC / 200.0f)))
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT               ((int)((float)FRACUNIT / (1.00f + F_PANINC / 200.0f)))

#define PLAYERRADIUS            (16 * (1 << MAPBITS))

// translates between frame-buffer and map distances
#define FTOM(x)                 (fixed_t)(((uint64_t)((x) << FRACBITS) * scale_ftom) >> FRACBITS)
#define MTOF(x)                 (fixed_t)((((uint64_t)(x) * scale_mtof) >> FRACBITS) >> FRACBITS)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)               MTOF(x - m_x)
#define CYMTOF(y)               (mapheight - MTOF(y - m_y))

#define GRIDSIZE                (128 << MAPBITS)

typedef struct
{
    mpoint_t    a, b;
} mline_t;

//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
mline_t playerarrow[] =
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

#define PLAYERARROWLINES        8

mline_t cheatplayerarrow[] =
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

#define CHEATPLAYERARROWLINES   19

mline_t thingtriangle[] =
{
    { { -32768, -45875 }, {  65536,     0 } },
    { {  65536,      0 }, { -32768, 45875 } },
    { { -32768,  45875 }, { -32768,-45875 } }
};

#define THINGTRIANGLELINES      3


dboolean                automapactive = false;

static unsigned int     mapwidth;
static unsigned int     mapheight;
static unsigned int     maparea;
static unsigned int     mapbottom;

static mpoint_t         m_paninc;       // how far the window pans each tic (map coords)
static fixed_t          mtof_zoommul;   // how far the window zooms in each tic (map coords)
static fixed_t          ftom_zoommul;   // how far the window zooms in each tic (fb coords)

// LL x,y where the window is on the map (map coords)
fixed_t                 m_x = INT_MAX, m_y = INT_MAX;

// UR x,y where the window is on the map (map coords)
static fixed_t          m_x2, m_y2;

//
// width/height of window on map (map coords)
//
fixed_t                 m_w;
fixed_t                 m_h;

// based on level size
static fixed_t          min_x;
static fixed_t          min_y;
static fixed_t          max_x;
static fixed_t          max_y;

static fixed_t          min_scale_mtof;         // used to tell when to stop zooming out
static fixed_t          max_scale_mtof;         // used to tell when to stop zooming in

// old stuff for recovery later
static fixed_t          old_m_w, old_m_h;
static fixed_t          old_m_x, old_m_y;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t          scale_mtof;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t          scale_ftom;

static player_t         *plr;                   // the player represented by an arrow

mpoint_t                *markpoints = NULL;     // where the points are
int                     markpointnum = 0;       // next point to be assigned
int                     markpointnum_max = 0;

dboolean                am_external = am_external_default;
dboolean                am_followmode = am_followmode_default;
dboolean                am_grid = am_grid_default;
dboolean                am_rotatemode = am_rotatemode_default;

static dboolean         stopped = true;

dboolean                bigstate = false;
byte                    *area;
static dboolean         movement = false;
int                     keydown;
int                     direction;

int                     teleporters[24];
int                     numteleporters;

am_frame_t              am_frame;

static void AM_rotate(fixed_t *x, fixed_t *y, angle_t angle);

static void AM_activateNewScale(void)
{
    m_x += m_w / 2;
    m_y += m_h / 2;
    m_w = FTOM(mapwidth);
    m_h = FTOM(mapheight);
    m_x -= m_w / 2;
    m_y -= m_h / 2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

static void AM_saveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

static void AM_restoreScaleAndLoc(void)
{
    m_w = old_m_w;
    m_h = old_m_h;
    if (am_followmode)
    {
        m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w / 2;
        m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h / 2;
    }
    else
    {
        m_x = old_m_x;
        m_y = old_m_y;
    }
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

    // Change the scaling multipliers
    scale_mtof = FixedDiv(mapwidth << FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
static void AM_findMinMaxBoundaries(void)
{
    int         i;
    fixed_t     a;
    fixed_t     b;

    min_x = min_y = INT_MAX;
    max_x = max_y = INT_MIN;

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

    a = FixedDiv(mapwidth << FRACBITS, (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS));
    b = FixedDiv(mapheight << FRACBITS, (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS));

    min_scale_mtof = MIN(a, b);
    max_scale_mtof = FixedDiv(mapheight << FRACBITS, 2 * PLAYERRADIUS);
}

static void AM_changeWindowLoc(void)
{
    fixed_t     incx = m_paninc.x;
    fixed_t     incy = m_paninc.y;

    if (am_rotatemode)
    {
        AM_rotate(&incx, &incy, plr->mo->angle - ANG90);

        m_x += incx;
        m_y += incy;
    }
    else
    {
        fixed_t w = m_w / 2;
        fixed_t h = m_h / 2;

        m_x += incx;
        m_y += incy;
        m_x = BETWEEN(min_x, m_x + w, max_x) - w;
        m_y = BETWEEN(min_y, m_y + h, max_y) - h;
    }

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

void AM_Init(void)
{
    byte        *priority;
    int         x, y;
    int         i;

    priority = Z_Malloc(256, PU_STATIC, NULL);
    mask = Z_Malloc(256, PU_STATIC, NULL);
    for (x = 0; x < 256; ++x)
    {
        *(priority + x) = 0;
        *(mask + x) = x;
    }

    *(priority + PLAYERCOLOR) = PLAYERPRIORITY;
    *(priority + THINGCOLOR) = THINGPRIORITY;
    *(priority + WALLCOLOR) = WALLPRIORITY;
    *(priority + ALLMAPWALLCOLOR) = ALLMAPWALLPRIORITY;
    *(priority + MASKCOLOR) = MASKPRIORITY;
    *(priority + CDWALLCOLOR) = CDWALLPRIORITY;
    *(priority + ALLMAPCDWALLCOLOR) = ALLMAPCDWALLPRIORITY;
    *(priority + FDWALLCOLOR) = FDWALLPRIORITY;
    *(priority + ALLMAPFDWALLCOLOR) = ALLMAPFDWALLPRIORITY;
    *(priority + TELEPORTERCOLOR) = TELEPORTERPRIORITY;
    *(priority + TSWALLCOLOR) = TSWALLPRIORITY;
    *(priority + GRIDCOLOR) = GRIDPRIORITY;

    *(mask + MASKCOLOR) = BACKGROUNDCOLOR;

    priorities = Z_Malloc(65536, PU_STATIC, NULL);
    for (x = 0; x < 256; ++x)
        for (y = 0; y < 256; ++y)
            *(priorities + (x << 8) + y) = (*(priority + x) > *(priority + y) ? x : y);

    playercolor = priorities + (PLAYERCOLOR << 8);
    thingcolor = priorities + (THINGCOLOR << 8);
    wallcolor = priorities + (WALLCOLOR << 8);
    allmapwallcolor = priorities + (ALLMAPWALLCOLOR << 8);
    maskcolor = priorities + (MASKCOLOR << 8);
    cdwallcolor = priorities + (CDWALLCOLOR << 8);
    allmapcdwallcolor = priorities + (ALLMAPCDWALLCOLOR << 8);
    fdwallcolor = priorities + (FDWALLCOLOR << 8);
    allmapfdwallcolor = priorities + (ALLMAPFDWALLCOLOR << 8);
    teleportercolor = priorities + (TELEPORTERCOLOR << 8);
    tswallcolor = priorities + (TSWALLCOLOR << 8);
    gridcolor = priorities + (GRIDCOLOR << 8);

    for (i = 0; i < arrlen(teleporters); ++i)
        teleporters[i] = -1;

    if (BTSX)
    {
        teleporters[0] = R_CheckFlatNumForName("SLIME09");
        teleporters[1] = R_CheckFlatNumForName("SLIME12");
        teleporters[2] = R_CheckFlatNumForName("TELEPRT1");
        teleporters[3] = R_CheckFlatNumForName("TELEPRT2");
        teleporters[4] = R_CheckFlatNumForName("TELEPRT3");
        teleporters[5] = R_CheckFlatNumForName("TELEPRT4");
        teleporters[6] = R_CheckFlatNumForName("TELEPRT5");
        teleporters[7] = R_CheckFlatNumForName("TELEPRT6");
        teleporters[8] = R_CheckFlatNumForName("SLIME05");
        teleporters[9] = R_CheckFlatNumForName("SHNPRT02");
        teleporters[10] = R_CheckFlatNumForName("SHNPRT03");
        teleporters[11] = R_CheckFlatNumForName("SHNPRT04");
        teleporters[12] = R_CheckFlatNumForName("SHNPRT05");
        teleporters[13] = R_CheckFlatNumForName("SHNPRT06");
        teleporters[14] = R_CheckFlatNumForName("SHNPRT07");
        teleporters[15] = R_CheckFlatNumForName("SHNPRT08");
        teleporters[16] = R_CheckFlatNumForName("SHNPRT09");
        teleporters[17] = R_CheckFlatNumForName("SHNPRT10");
        teleporters[18] = R_CheckFlatNumForName("SHNPRT11");
        teleporters[19] = R_CheckFlatNumForName("SHNPRT12");
        teleporters[20] = R_CheckFlatNumForName("SHNPRT13");
        teleporters[21] = R_CheckFlatNumForName("SHNPRT14");
        teleporters[22] = R_CheckFlatNumForName("SLIME08");
        numteleporters = 23;
    }
    else
    {
        teleporters[0] = R_CheckFlatNumForName("GATE1");
        teleporters[1] = R_CheckFlatNumForName("GATE2");
        teleporters[2] = R_CheckFlatNumForName("GATE3");
        teleporters[3] = R_CheckFlatNumForName("GATE4");
        numteleporters = 4;
    }
}

static void AM_initVariables(dboolean mainwindow)
{
    automapactive = mainwindow;

    area = mapscreen + maparea;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(mapwidth);
    m_h = FTOM(mapheight);

    plr = &players[0];

    if (m_x == INT_MAX || am_followmode)
    {
        m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w / 2;
        m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h / 2;
        m_x2 = m_x + m_w;
        m_y2 = m_y + m_h;
    }

    // inform the status bar of the change
    ST_AutomapEvent(AM_MSGENTERED);
}

//
// should be called at the start of every level
// right now, i figure it out myself
//
static void AM_LevelInit(void)
{
    am_followmode = true;
    bigstate = false;

    AM_findMinMaxBoundaries();
    scale_mtof = FixedDiv(INITSCALEMTOF, (int)(0.7 * FRACUNIT));
    if (scale_mtof > max_scale_mtof)
        scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

void AM_Stop(void)
{
    automapactive = false;
    HU_ClearMessages();
    ST_AutomapEvent(AM_MSGEXITED);
    stopped = true;
}

int     lastlevel = -1;
int     lastepisode = -1;

void AM_Start(dboolean mainwindow)
{
    if (!stopped)
        AM_Stop();
    stopped = false;

    mapwidth = SCREENWIDTH;
    mapheight = (unsigned int)viewheight2;
    maparea = mapwidth * mapheight;
    mapbottom = maparea - mapwidth;

    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
        AM_LevelInit();
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }
    AM_initVariables(mainwindow);
}

//
// set the window scale to the maximum size
//
static void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

//
// set the window scale to the minimum size
//
static void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

SDL_Keymod      modstate;
dboolean        speedtoggle;

static dboolean AM_getSpeedToggle(void)
{
    return (!!(gamepadbuttons & GAMEPAD_LEFT_TRIGGER) + !!(modstate & KMOD_SHIFT) == 1);
}

static void AM_toggleZoomOut(void)
{
    speedtoggle = AM_getSpeedToggle();
    mtof_zoommul = M_ZOOMOUT;
    ftom_zoommul = M_ZOOMIN;
}

static void AM_toggleZoomIn(void)
{
    speedtoggle = AM_getSpeedToggle();
    mtof_zoommul = M_ZOOMIN;
    ftom_zoommul = M_ZOOMOUT;
    bigstate = false;
}

static void AM_toggleMaxZoom(void)
{
    if (bigstate)
    {
        bigstate = false;
        AM_restoreScaleAndLoc();
    }
    else if (scale_mtof != min_scale_mtof)
    {
        bigstate = true;
        AM_saveScaleAndLoc();
        AM_minOutWindowScale();
    }
}

static void AM_toggleFollowMode(void)
{
    am_followmode = !am_followmode;
    if (am_followmode)
        m_paninc.x = m_paninc.y = 0;
    C_Input("%s %s", stringize(am_followmode), (am_followmode ? "on" : "off"));
    HU_PlayerMessage((am_followmode ? s_AMSTR_FOLLOWON : s_AMSTR_FOLLOWOFF), false);
    message_dontfuckwithme = true;
    message_clearable = true;
}

static void AM_toggleGrid(void)
{
    am_grid = !am_grid;
    C_Input("%s %s", stringize(am_grid), (am_grid ? "on" : "off"));
    HU_PlayerMessage((am_grid ? s_AMSTR_GRIDON : s_AMSTR_GRIDOFF), false);
    message_dontfuckwithme = true;
    message_clearable = true;
    M_SaveCVARs();
}

//
// adds a marker at the current location
//
static void AM_addMark(void)
{
    int         i;
    int         x = m_x + m_w / 2;
    int         y = m_y + m_h / 2;
    static char message[32];

    for (i = 0; i < markpointnum; ++i)
        if (markpoints[i].x == x && markpoints[i].y == y)
            return;

    if (markpointnum >= markpointnum_max)
    {
        markpointnum_max = (markpointnum_max ? markpointnum_max << 1 : 16);
        markpoints = Z_Realloc(markpoints, markpointnum_max * sizeof(*markpoints));
    }

    markpoints[markpointnum].x = x;
    markpoints[markpointnum].y = y;
    M_snprintf(message, sizeof(message), s_AMSTR_MARKEDSPOT, ++markpointnum);
    HU_PlayerMessage(message, false);
    message_dontfuckwithme = true;
    message_clearable = true;
}

int     markpress = 0;

static void AM_clearMarks(void)
{
    if (markpointnum)
    {
        if (++markpress == 5)
        {
            // clear all marks
            HU_PlayerMessage(s_AMSTR_MARKSCLEARED, false);
            message_dontfuckwithme = true;
            markpointnum = 0;
        }
        else if (markpress == 1)
        {
            static char message[32];

            // clear one mark
            M_snprintf(message, sizeof(message), s_AMSTR_MARKCLEARED, markpointnum--);
            HU_PlayerMessage(message, false);
            message_dontfuckwithme = true;
            message_clearable = true;
        }
    }
}

static void AM_toggleRotateMode(void)
{
    am_rotatemode = !am_rotatemode;
    C_Input("%s %s", stringize(am_rotatemode), (am_rotatemode ? "on" : "off"));
    HU_PlayerMessage((am_rotatemode ? s_AMSTR_ROTATEON : s_AMSTR_ROTATEOFF), false);
    message_dontfuckwithme = true;
    message_clearable = true;
    M_SaveCVARs();
}

//
// Handle events (user inputs) in automap mode
//
dboolean AM_Responder(event_t *ev)
{
    int                 rc = false;

    direction = 0;
    modstate = SDL_GetModState();

    if (!menuactive && !paused)
    {
        static dboolean backbuttondown = false;

        if (!(gamepadbuttons & gamepadautomap))
            backbuttondown = false;

        if (!automapactive)
        {
            if ((ev->type == ev_keydown
                 && ev->data1 == AM_STARTKEY
                 && keydown != AM_STARTKEY
                 && !(modstate & KMOD_ALT))
                 || (ev->type == ev_gamepad
                     && (gamepadbuttons & gamepadautomap)
                     && !backbuttondown))
            {
                keydown = AM_STARTKEY;
                backbuttondown = true;
                AM_Start(true);
                viewactive = false;
                rc = true;
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
                        speedtoggle = AM_getSpeedToggle();
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
                        speedtoggle = AM_getSpeedToggle();
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
                        speedtoggle = AM_getSpeedToggle();
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
                        speedtoggle = AM_getSpeedToggle();
                        m_paninc.y = -FTOM(F_PANINC);
                    }
                }

                // zoom out
                else if (key == AM_ZOOMOUTKEY && !movement)
                {
                    keydown = key;
                    AM_toggleZoomOut();
                }

                // zoom in
                else if (key == AM_ZOOMINKEY && !movement)
                {
                    keydown = key;
                    AM_toggleZoomIn();
                }

                // leave automap
                else if (key == AM_ENDKEY && !(modstate & KMOD_ALT) && keydown != AM_ENDKEY)
                {
                    keydown = key;
                    viewactive = true;
                    AM_Stop();
                }

                // toggle maximum zoom
                else if (key == AM_GOBIGKEY && !idclev && !idmus)
                {
                    if (keydown != AM_GOBIGKEY)
                    {
                        keydown = key;
                        AM_toggleMaxZoom();
                    }
                }

                // toggle follow mode
                else if (key == AM_FOLLOWKEY)
                {
                    if (keydown != AM_FOLLOWKEY)
                    {
                        keydown = key;
                        AM_toggleFollowMode();
                    }
                }

                // toggle grid
                else if (key == AM_GRIDKEY)
                {
                    if (keydown != AM_GRIDKEY)
                    {
                        keydown = key;
                        AM_toggleGrid();
                    }
                }

                // mark spot
                else if (key == AM_MARKKEY && plr->health)
                {
                    if (keydown != AM_MARKKEY)
                    {
                        keydown = key;
                        AM_addMark();
                    }
                }

                // clear mark(s)
                else if (key == AM_CLEARMARKKEY)
                    AM_clearMarks();

                // toggle rotate mode
                else if (key == AM_ROTATEKEY)
                {
                    if (keydown != AM_ROTATEKEY)
                    {
                        keydown = key;
                        AM_toggleRotateMode();
                    }
                }
                else
                    rc = false;
            }
            else if (ev->type == ev_keyup)
            {
                rc = false;
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
                    int keydown = 0;

                    if (keystate(AM_PANLEFTKEY))
                        keydown = AM_PANLEFTKEY;
                    else if (keystate(AM_PANLEFTKEY2))
                        keydown = AM_PANLEFTKEY2;
                    else if (keystate(AM_PANLEFTKEY3))
                        keydown = AM_PANLEFTKEY3;
                    else if (keystate(AM_PANRIGHTKEY))
                        keydown = AM_PANRIGHTKEY;
                    else if (keystate(AM_PANRIGHTKEY2))
                        keydown = AM_PANRIGHTKEY2;
                    else if (keystate(AM_PANRIGHTKEY3))
                        keydown = AM_PANRIGHTKEY3;
                    else if (keystate(AM_PANUPKEY))
                        keydown = AM_PANUPKEY;
                    else if (keystate(AM_PANUPKEY2))
                        keydown = AM_PANUPKEY2;
                    else if (keystate(AM_PANDOWNKEY))
                        keydown = AM_PANDOWNKEY;
                    else if (keystate(AM_PANDOWNKEY2))
                        keydown = AM_PANDOWNKEY2;

                    if (keydown)
                    {
                        event_t event;

                        event.type = ev_keydown;
                        event.data1 = keydown;
                        event.data2 = 0;
                        D_PostEvent(&event);
                    }
                }
                else if (!am_followmode)
                {
                    if (key == AM_PANLEFTKEY || key == AM_PANLEFTKEY2 || key == AM_PANLEFTKEY3)
                    {
                        speedtoggle = AM_getSpeedToggle();
                        if (keystate(AM_PANRIGHTKEY) || keystate(AM_PANRIGHTKEY2)
                            || keystate(AM_PANRIGHTKEY3))
                            m_paninc.x = FTOM(F_PANINC);
                        else
                            m_paninc.x = 0;
                    }
                    else if (key == AM_PANRIGHTKEY || key == AM_PANRIGHTKEY2
                        || key == AM_PANRIGHTKEY3)
                    {
                        speedtoggle = AM_getSpeedToggle();
                        if (keystate(AM_PANLEFTKEY) || keystate(AM_PANLEFTKEY2)
                            || keystate(AM_PANLEFTKEY3))
                            m_paninc.x = -FTOM(F_PANINC);
                        else
                            m_paninc.x = 0;
                    }
                    else if (key == AM_PANUPKEY || key == AM_PANUPKEY2)
                    {
                        speedtoggle = AM_getSpeedToggle();
                        if (keystate(AM_PANDOWNKEY) || keystate(AM_PANDOWNKEY2))
                            m_paninc.y = FTOM(F_PANINC);
                        else
                            m_paninc.y = 0;
                    }
                    else if (key == AM_PANDOWNKEY || key == AM_PANDOWNKEY2)
                    {
                        speedtoggle = AM_getSpeedToggle();
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
                    speedtoggle = AM_getSpeedToggle();
                    mtof_zoommul = M_ZOOMIN + 2000;
                    ftom_zoommul = M_ZOOMOUT - 2000;
                    bigstate = false;
                }

                // zoom out
                else if (ev->data1 < 0)
                {
                    movement = true;
                    speedtoggle = AM_getSpeedToggle();
                    mtof_zoommul = M_ZOOMOUT - 2000;
                    ftom_zoommul = M_ZOOMIN + 2000;
                }
            }
            else if (ev->type == ev_gamepad)
            {
                if ((gamepadbuttons & gamepadautomap) && !backbuttondown)
                {
                    viewactive = true;
                    backbuttondown = true;
                    AM_Stop();
                }

                // zoom out
                else if ((gamepadbuttons & gamepadautomapzoomout)
                         && !(gamepadbuttons & gamepadautomapzoomin))
                {
                    movement = true;
                    AM_toggleZoomOut();
                }

                // zoom in
                else if ((gamepadbuttons & gamepadautomapzoomin)
                         && !(gamepadbuttons & gamepadautomapzoomout))
                {
                    movement = true;
                    AM_toggleZoomIn();
                }

                // toggle maximum zoom
                else if ((gamepadbuttons & gamepadautomapmaxzoom) && !idclev && !idmus)
                    AM_toggleMaxZoom();

                // toggle follow mode
                else if (gamepadbuttons & gamepadautomapfollowmode)
                    AM_toggleFollowMode();

                // toggle grid
                else if (gamepadbuttons & gamepadautomapgrid)
                    AM_toggleGrid();

                // mark spot
                else if ((gamepadbuttons & gamepadautomapmark) && plr->health)
                    AM_addMark();

                // clear mark(s)
                else if (gamepadbuttons & gamepadautomapclearmark)
                    AM_clearMarks();

                // toggle rotate mode
                else if (gamepadbuttons & gamepadautomaprotatemode)
                    AM_toggleRotateMode();

                if (!am_followmode)
                {
                    // pan right
                    if (gamepadthumbLX > 0)
                    {
                        movement = true;
                        speedtoggle = AM_getSpeedToggle();
                        m_paninc.x = FTOM(MTOF((fixed_t)(FTOM(F_PANINC)
                            * gamepadthumbLXright * 1.2f)));
                    }

                    // pan left
                    else if (gamepadthumbLX < 0)
                    {
                        movement = true;
                        speedtoggle = AM_getSpeedToggle();
                        m_paninc.x = -FTOM(MTOF((fixed_t)(FTOM(F_PANINC)
                            * gamepadthumbLXleft * 1.2f)));
                    }

                    // pan up
                    if (gamepadthumbLY < 0)
                    {
                        movement = true;
                        speedtoggle = AM_getSpeedToggle();
                        m_paninc.y = FTOM(MTOF((fixed_t)(FTOM(F_PANINC)
                            * gamepadthumbLYup * 1.2f)));
                    }

                    // pan down
                    else if (gamepadthumbLY > 0)
                    {
                        movement = true;
                        speedtoggle = AM_getSpeedToggle();
                        m_paninc.y = -FTOM(MTOF((fixed_t)(FTOM(F_PANINC)
                            * gamepadthumbLYdown * 1.2f)));
                    }
                }
            }

            if ((plr->cheats & CF_MYPOS) && !am_followmode && (m_paninc.x || m_paninc.y))
            {
                double  x = m_paninc.x;
                double  y = m_paninc.y;

                if ((m_x == min_x - m_w / 2 && x < 0) || (m_x == max_x - m_w / 2 && x > 0))
                    x = 0;
                if ((m_y == min_y - m_h / 2 && y < 0) || (m_y == max_y - m_h / 2 && y > 0))
                    y = 0;
                direction = (int)(atan2(y, x) * 180.0 / M_PI);
                if (direction < 0)
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
static void AM_rotate(fixed_t *x, fixed_t *y, angle_t angle)
{
    fixed_t     cosine = finecosine[angle >>= ANGLETOFINESHIFT];
    fixed_t     sine = finesine[angle];
    fixed_t     temp = FixedMul(*x, cosine) - FixedMul(*y, sine);

    *y = FixedMul(*x, sine) + FixedMul(*y, cosine);
    *x = temp;
}

static void AM_rotatePoint(mpoint_t *point)
{
    fixed_t     temp;

    point->x -= am_frame.centerx;
    point->y -= am_frame.centery;

    temp = FixedMul(point->x, am_frame.cos) - FixedMul(point->y, am_frame.sin) + am_frame.centerx;
    point->y = FixedMul(point->x, am_frame.sin) + FixedMul(point->y, am_frame.cos)
        + am_frame.centery;
    point->x = temp;
}

//
// Zooming
//
static void AM_changeWindowScale(void)
{
    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    if (scale_mtof < min_scale_mtof)
        AM_minOutWindowScale();
    else if (scale_mtof > max_scale_mtof)
        AM_maxOutWindowScale();
    else
        AM_activateNewScale();
}

static void AM_doFollowPlayer(void)
{
    m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w / 2;
    m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h / 2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

//
// Updates on Game Tic
//
void AM_Ticker(void)
{
    if (!automapactive && !mapwindow)
        return;

    if (am_followmode)
        AM_doFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
        AM_changeWindowScale();

    // Change x,y location
    if ((m_paninc.x || m_paninc.y) && !menuactive && !paused && !consoleactive)
        AM_changeWindowLoc();

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
void AM_clearFB(void)
{
    memset(mapscreen, BACKGROUNDCOLOR, maparea);
}

//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes. If the speed is needed,
// use a hash algorithm to handle the common cases.
static dboolean AM_clipMline(int *x0, int *y0, int *x1, int *y1)
{
    enum
    {
        LEFT   = 1,
        RIGHT  = 2,
        TOP    = 4,
        BOTTOM = 8
    };

    unsigned int        outcode1 = 0;
    unsigned int        outcode2 = 0;

    *x0 = CXMTOF(*x0);
    if (*x0 < -1)
        outcode1 = LEFT;
    else if (*x0 >= (int)mapwidth)
        outcode1 = RIGHT;
    *x1 = CXMTOF(*x1);
    if (*x1 < -1)
        outcode2 = LEFT;
    else if (*x1 >= (int)mapwidth)
        outcode2 = RIGHT;
    if (outcode1 & outcode2)
        return false;
    *y0 = CYMTOF(*y0);
    if (*y0 < -1)
        outcode1 |= TOP;
    else if (*y0 >= (int)mapheight)
        outcode1 |= BOTTOM;
    *y1 = CYMTOF(*y1);
    if (*y1 < -1)
        outcode2 |= TOP;
    else if (*y1 >= (int)mapheight)
        outcode2 |= BOTTOM;
    return !(outcode1 & outcode2);
}

static __inline void _PUTDOT(byte *dot, byte *color)
{
    *dot = *(*dot + color);
}

static __inline void PUTDOT(unsigned int x, unsigned int y, byte *color)
{
    if (x < mapwidth && y < maparea)
        _PUTDOT(mapscreen + y + x, color);
}

static __inline void PUTBIGDOT(unsigned int x, unsigned int y, byte *color)
{
    if (x < mapwidth)
    {
        byte    *dot = mapscreen + y + x;
        dboolean top = (y < maparea);
        dboolean bottom = (y < mapbottom);

        if (top)
            _PUTDOT(dot, color);
        if (bottom)
            _PUTDOT(dot + mapwidth, color);
        if (x + 1 < mapwidth)
        {
            if (top)
                _PUTDOT(dot + 1, color);
            if (bottom)
                _PUTDOT(dot + mapwidth + 1, color);
        }
    }
    else if (++x < mapwidth)
    {
        byte    *dot = mapscreen + y + x;

        if (y < maparea)
            _PUTDOT(dot, color);
        if (y < mapbottom)
            _PUTDOT(dot + mapwidth, color);
    }
}

static __inline void PUTTRANSDOT(unsigned int x, unsigned int y, byte *color)
{
    if (x < mapwidth && y < maparea)
    {
        byte    *dot = mapscreen + y + x;

        if (*dot != *(tinttab60 + PLAYERCOLOR))
            *dot = *(tinttab60 + (*dot << 8) + PLAYERCOLOR);
    }
}

//
// Classic Bresenham w/ whatever optimizations needed for speed
//
static void AM_drawFline(int x0, int y0, int x1, int y1, byte *color,
    void (*putdot)(unsigned int, unsigned int, byte *))
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    if (!dy)
    {
        if (dx)
        {
            // horizontal line
            int     sx = SIGN(dx);

            x0 = BETWEEN(-1, x0, mapwidth - 1);
            x1 = BETWEEN(-1, x1, mapwidth - 1);

            y0 *= mapwidth;

            putdot(x0, y0, color);
            while (x0 != x1)
                putdot(x0 += sx, y0, color);
        }
    }
    else if (!dx)
    {
        // vertical line
        int     sy = SIGN(dy) * mapwidth;

        y0 = BETWEEN(-(signed int)mapwidth, y0 * mapwidth, mapbottom);
        y1 = BETWEEN(-(signed int)mapwidth, y1 * mapwidth, mapbottom);

        putdot(x0, y0, color);
        while (y0 != y1)
            putdot(x0, y0 += sy, color);
    }
    else
    {
        int     sx = SIGN(dx);
        int     sy = SIGN(dy) * mapwidth;

        dx = ABS(dx);
        dy = ABS(dy);
        y0 *= mapwidth;
        putdot(x0, y0, color);
        if (dx == dy)
        {
            // diagonal line
            while (x0 != x1)
                putdot(x0 += sx, y0 += sy, color);
        }
        else
        {
            if (dx > dy)
            {
                // x-major line
                int     error = (dy <<= 1) - dx;

                dx <<= 1;
                while (x0 != x1)
                {
                    int mask = ~(error >> 31);

                    putdot(x0 += sx, y0 += (sy & mask), color);
                    error += dy - (dx & mask);
                }
            }
            else
            {
                // y-major line
                int     error = (dx <<= 1) - dy;

                dy <<= 1;
                y1 *= mapwidth;
                while (y0 != y1)
                {
                    int mask = ~(error >> 31);

                    putdot(x0 += (sx & mask), y0 += sy, color);
                    error += dx - (dy & mask);
                }
            }
        }
    }
}

//
// Clip lines, draw visible parts of lines.
//
static void AM_drawMline(int x0, int y0, int x1, int y1, byte *color)
{
    if (AM_clipMline(&x0, &y0, &x1, &y1))
        AM_drawFline(x0, y0, x1, y1, color, PUTDOT);
}

static void AM_drawBigMline(int x0, int y0, int x1, int y1, byte *color)
{
    if (AM_clipMline(&x0, &y0, &x1, &y1))
        AM_drawFline(x0, y0, x1, y1, color, PUTBIGDOT);
}

static void AM_drawTransMline(int x0, int y0, int x1, int y1, byte *color)
{
    if (AM_clipMline(&x0, &y0, &x1, &y1))
        AM_drawFline(x0, y0, x1, y1, color, PUTTRANSDOT);
}

//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
static void AM_drawGrid(void)
{
    fixed_t     x, y;
    fixed_t     start, end;
    mline_t     ml;

    fixed_t     minlen = (fixed_t)(sqrt((double)m_w * (double)m_w + (double)m_h * (double)m_h));
    fixed_t     extx = (minlen - m_w) / 2;
    fixed_t     exty = (minlen - m_h) / 2;

    // Figure out start of vertical gridlines
    start = m_x - extx;
    if ((start - (bmaporgx >> FRACTOMAPBITS)) % GRIDSIZE)
        start += GRIDSIZE - ((start - (bmaporgx >> FRACTOMAPBITS)) % GRIDSIZE);
    end = m_x + minlen - extx;

    // draw vertical gridlines
    for (x = start; x < end; x += GRIDSIZE)
    {
        ml.a.x = x;
        ml.b.x = x;
        ml.a.y = m_y - exty;
        ml.b.y = ml.a.y + minlen;
        if (am_rotatemode)
        {
            AM_rotatePoint(&ml.a);
            AM_rotatePoint(&ml.b);
        }
        AM_drawMline(ml.a.x, ml.a.y, ml.b.x, ml.b.y, gridcolor);
    }

    // Figure out start of horizontal gridlines
    start = m_y - exty;
    if ((start - (bmaporgy >> FRACTOMAPBITS)) % GRIDSIZE)
        start += GRIDSIZE - ((start - (bmaporgy >> FRACTOMAPBITS)) % GRIDSIZE);
    end = m_y + minlen - exty;

    // draw horizontal gridlines
    for (y = start; y < end; y += GRIDSIZE)
    {
        ml.a.x = m_x - extx;
        ml.b.x = ml.a.x + minlen;
        ml.a.y = y;
        ml.b.y = y;
        if (am_rotatemode)
        {
            AM_rotatePoint(&ml.a);
            AM_rotatePoint(&ml.b);
        }
        AM_drawMline(ml.a.x, ml.a.y, ml.b.x, ml.b.y, gridcolor);
    }
}

dboolean isteleport(int floorpic)
{
    int i;

    for (i = 0; i < numteleporters; ++i)
        if (floorpic == teleporters[i])
            return true;

    return false;
}

//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
static void AM_drawWalls(void)
{
    dboolean    allmap = plr->powers[pw_allmap];
    dboolean    cheating = (plr->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS));
    int         i = 0;

    while (i < numlines)
    {
        line_t  line = lines[i++];

        if ((line.bbox[BOXLEFT] >> FRACTOMAPBITS) > am_frame.bbox[BOXRIGHT]
            || (line.bbox[BOXRIGHT] >> FRACTOMAPBITS) < am_frame.bbox[BOXLEFT]
            || (line.bbox[BOXBOTTOM] >> FRACTOMAPBITS) > am_frame.bbox[BOXTOP]
            || (line.bbox[BOXTOP] >> FRACTOMAPBITS) < am_frame.bbox[BOXBOTTOM])
            continue;
        else
        {
            short       flags = line.flags;

            if ((flags & ML_DONTDRAW) && !cheating)
                continue;
            else
            {
                sector_t            *backsector = line.backsector;
                sector_t            *frontsector = line.frontsector;
                short               mapped = (flags & ML_MAPPED);
                short               secret = (flags & ML_SECRET);
                short               special = line.special;
                static mline_t      l;

                l.a.x = line.v1->x >> FRACTOMAPBITS;
                l.a.y = line.v1->y >> FRACTOMAPBITS;
                l.b.x = line.v2->x >> FRACTOMAPBITS;
                l.b.y = line.v2->y >> FRACTOMAPBITS;

                if (am_rotatemode)
                {
                    AM_rotatePoint(&l.a);
                    AM_rotatePoint(&l.b);
                }

                if ((special == W1_Teleport || special == W1_ExitLevel || special == WR_Teleport
                    || special == W1_ExitLevel_GoesToSecretLevel
                    || special == W1_Teleport_AlsoMonsters_Silent_SameAngle
                    || special == WR_Teleport_AlsoMonsters_Silent_SameAngle
                    || special == W1_TeleportToLineWithSameTag_Silent_SameAngle
                    || special == WR_TeleportToLineWithSameTag_Silent_SameAngle
                    || special == W1_TeleportToLineWithSameTag_Silent_ReversedAngle
                    || special == WR_TeleportToLineWithSameTag_Silent_ReversedAngle)
                    && ((flags & ML_TELEPORTTRIGGERED) || cheating
                    || (backsector && isteleport(backsector->floorpic))))
                {
                    if (cheating || (mapped && !secret && backsector
                        && backsector->ceilingheight != backsector->floorheight))
                    {
                        AM_drawMline(l.a.x, l.a.y, l.b.x, l.b.y, teleportercolor);
                        continue;
                    }
                    else if (allmap)
                    {
                        AM_drawMline(l.a.x, l.a.y, l.b.x, l.b.y, allmapfdwallcolor);
                        continue;
                    }
                }
                if (!backsector || (secret && !cheating))
                    AM_drawBigMline(l.a.x, l.a.y, l.b.x, l.b.y, (mapped || cheating ? wallcolor :
                        (allmap ? allmapwallcolor : maskcolor)));
                else if (backsector->floorheight != frontsector->floorheight)
                {
                    if (mapped || cheating)
                        AM_drawMline(l.a.x, l.a.y, l.b.x, l.b.y, fdwallcolor);
                    else if (allmap)
                        AM_drawMline(l.a.x, l.a.y, l.b.x, l.b.y, allmapfdwallcolor);
                }
                else if (backsector->ceilingheight != frontsector->ceilingheight)
                {
                    if (mapped || cheating)
                        AM_drawMline(l.a.x, l.a.y, l.b.x, l.b.y, cdwallcolor);
                    else if (allmap)
                        AM_drawMline(l.a.x, l.a.y, l.b.x, l.b.y, allmapcdwallcolor);
                }
                else if (cheating)
                    AM_drawMline(l.a.x, l.a.y, l.b.x, l.b.y, tswallcolor);
            }
        }
    }

    if (!cheating && !allmap)
    {
        byte    *dot = mapscreen;

        while (dot < area)
        {
            *dot = *(*dot + mask);
            ++dot;
        }
    }
}

static void AM_drawLineCharacter(mline_t *lineguy, int lineguylines, fixed_t scale, angle_t angle,
    byte *color, fixed_t x, fixed_t y)
{
    int i;

    if (am_rotatemode)
        angle -= plr->mo->angle - ANG90;

    for (i = 0; i < lineguylines; ++i)
    {
        int     x1, y1;
        int     x2, y2;

        if (scale)
        {
            x1 = FixedMul(lineguy[i].a.x, scale);
            y1 = FixedMul(lineguy[i].a.y, scale);
            x2 = FixedMul(lineguy[i].b.x, scale);
            y2 = FixedMul(lineguy[i].b.y, scale);
        }
        else
        {
            x1 = lineguy[i].a.x;
            y1 = lineguy[i].a.y;
            x2 = lineguy[i].b.x;
            y2 = lineguy[i].b.y;
        }
        if (angle)
        {
            AM_rotate(&x1, &y1, angle);
            AM_rotate(&x2, &y2, angle);
        }
        AM_drawMline(x + x1, y + y1, x + x2, y + y2, color);
    }
}

static void AM_drawTransLineCharacter(mline_t *lineguy, int lineguylines, fixed_t scale,
    angle_t angle, byte *color, fixed_t x, fixed_t y)
{
    int i;

    if (am_rotatemode)
        angle -= plr->mo->angle - ANG90;

    for (i = 0; i < lineguylines; ++i)
    {
        int     x1, y1;
        int     x2, y2;

        if (scale)
        {
            x1 = FixedMul(lineguy[i].a.x, scale);
            y1 = FixedMul(lineguy[i].a.y, scale);
            x2 = FixedMul(lineguy[i].b.x, scale);
            y2 = FixedMul(lineguy[i].b.y, scale);
        }
        else
        {
            x1 = lineguy[i].a.x;
            y1 = lineguy[i].a.y;
            x2 = lineguy[i].b.x;
            y2 = lineguy[i].b.y;
        }
        if (angle)
        {
            AM_rotate(&x1, &y1, angle);
            AM_rotate(&x2, &y2, angle);
        }
        AM_drawTransMline(x + x1, y + y1, x + x2, y + y2, color);
    }
}

static void AM_drawPlayers(void)
{
    int         invisibility = plr->powers[pw_invisibility];
    mpoint_t    point;

    point.x = plr->mo->x >> FRACTOMAPBITS;
    point.y = plr->mo->y >> FRACTOMAPBITS;

    if (am_rotatemode)
        AM_rotatePoint(&point);

    if (plr->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS))
    {
        if (invisibility > 128 || (invisibility & 8))
            AM_drawTransLineCharacter(cheatplayerarrow, CHEATPLAYERARROWLINES,
                0, plr->mo->angle, NULL, point.x, point.y);
        else
            AM_drawLineCharacter(cheatplayerarrow, CHEATPLAYERARROWLINES,
                0, plr->mo->angle, playercolor, point.x, point.y);
    }
    else
    {
        if (invisibility > 128 || (invisibility & 8))
            AM_drawTransLineCharacter(playerarrow, PLAYERARROWLINES,
                0, plr->mo->angle, NULL, point.x, point.y);
        else
            AM_drawLineCharacter(playerarrow, PLAYERARROWLINES,
                0, plr->mo->angle, playercolor, point.x, point.y);
    }
}

static void AM_drawThings(void)
{
    int i;

    for (i = 0; i < numsectors; ++i)
    {
        // e6y
        // Two-pass method for better usability of automap:
        // The first one will draw all things except enemies
        // The second one is for enemies only
        // Stop after first pass if the current sector has no enemies
        int     pass;
        int     enemies = 0;

        for (pass = 0; pass < 2; pass += (enemies ? 1 : 2))
        {
            mobj_t      *thing = sectors[i].thinglist;

            while (thing)
            {
                // e6y: stop if all enemies from current sector already have been drawn
                if (pass && !enemies)
                    break;
                if (pass == ((thing->flags & (MF_SHOOTABLE | MF_CORPSE)) == MF_SHOOTABLE ?
                    (!pass ? enemies++ : enemies--), 0 : 1))
                {
                    thing = thing->snext;
                    continue;
                }

                if (!(thing->flags2 & MF2_DONOTMAP))
                {
                    mpoint_t    point;
                    int         fx;
                    int         fy;
                    int         lump = sprites[thing->sprite].spriteframes[0].lump[0];
                    int         w = (BETWEEN(24 << FRACBITS, MIN(spritewidth[lump],
                                    spriteheight[lump]), 96 << FRACBITS) >> FRACTOMAPBITS) / 2;

                    point.x = thing->x >> FRACTOMAPBITS;
                    point.y = thing->y >> FRACTOMAPBITS;

                    if (am_rotatemode)
                        AM_rotatePoint(&point);

                    fx = CXMTOF(point.x);
                    fy = CYMTOF(point.y);

                    if (fx >= -w && fx <= (int)mapwidth + w && fy >= -w && fy <= (int)mapwidth + w)
                        AM_drawLineCharacter(thingtriangle, THINGTRIANGLELINES, w, thing->angle,
                            thingcolor, point.x, point.y);
                }
                thing = thing->snext;
            }
        }
    }
}

static const char *marknums[10] =
{
    "011111101122221112222221122112211221122112211221"
    "122112211221122112211221122222211122221101111110",
    "001111000112210011222100122221001112210000122100"
    "001221000012210000122100001221000012210000111100",
    "111111101222221112222221111112210111122111222221"
    "122222111221111012211111122222211222222111111111",
    "111111101222221112222221111112210111122101222221"
    "012222210111122111111221122222211222221111111110",
    "111111111221122112211221122112211221122112222221"
    "122222211111122100001221000012210000122100001111",
    "111111111222222112222221122111111221111012222211"
    "122222211111122111111221122222211222221111111110",
    "011111101122221012222210122111101221111012222211"
    "122222211221122112211221122222211122221101111110",
    "111111111222222112222221111112210011222101122211"
    "012221100122110001221000012210000122100001111000",
    "011111101122221112222221122112211221122111222211"
    "122222211221122112211221122222211122221101111110",
    "011111101122221112222221122112211221122112222221"
    "112222210111122101111221012222210122221101111110"
};

#define MARKWIDTH       8
#define MARKHEIGHT      12

static void AM_drawMarks(void)
{
    int i;

    for (i = 0; i < markpointnum; ++i)
    {
        int             number = i + 1;
        int             temp = number;
        int             digits = 1;
        int             x, y;
        mpoint_t        point;

        point.x = markpoints[i].x;
        point.y = markpoints[i].y;

        if (am_rotatemode)
            AM_rotatePoint(&point);

        x = CXMTOF(point.x) - MARKWIDTH / 2 + 1;
        y = CYMTOF(point.y) - MARKHEIGHT / 2 - 1;

        while (temp /= 10)
            ++digits;
        x += (digits - 1) * MARKWIDTH / 2;
        x -= (number > 1 && number % 10 == 1);
        x -= (number / 10 == 1);

        do
        {
            int digit = number % 10;
            int j;

            if (i > 0 && digit == 1)
                x += 2;
            for (j = 0; j < MARKWIDTH * MARKHEIGHT; ++j)
            {
                int fx = x + j % MARKWIDTH;

                if ((unsigned int)fx < mapwidth)
                {
                    int fy = y + j / MARKWIDTH;

                    if ((unsigned int)fy < mapheight)
                    {
                        char    src = marknums[digit][j];
                        byte    *dest = mapscreen + fy * mapwidth + fx;

                        if (src == '2')
                            *dest = MARKCOLOR;
                        else if (src == '1' && *dest != MARKCOLOR && *dest != GRIDCOLOR)
                            *dest = *(*dest + tinttab80);
                    }
                }
            }
            x -= MARKWIDTH;
            number /= 10;
        }
        while (number > 0);
    }
}

static __inline void AM_DrawScaledPixel(int x, int y, byte *color)
{
    byte        *dest = mapscreen + (y * 2 - 1) * mapwidth + x * 2 - 1;

    *dest = *(*dest + color);
    ++dest;
    *dest = *(*dest + color);
    dest += mapwidth;
    *dest = *(*dest + color);
    --dest;
    *dest = *(*dest + color);
}

#define CENTERX         ORIGINALWIDTH / 2
#define CENTERY         (ORIGINALHEIGHT - 32) / 2

static void AM_drawCrosshair(void)
{
    byte        *color = tinttab60 + (CROSSHAIRCOLOR << 8);

    AM_DrawScaledPixel(CENTERX - 2, CENTERY, color);
    AM_DrawScaledPixel(CENTERX - 1, CENTERY, color);
    AM_DrawScaledPixel(CENTERX, CENTERY, color);
    AM_DrawScaledPixel(CENTERX + 1, CENTERY, color);
    AM_DrawScaledPixel(CENTERX + 2, CENTERY, color);
    AM_DrawScaledPixel(CENTERX, CENTERY - 2, color);
    AM_DrawScaledPixel(CENTERX, CENTERY - 1, color);
    AM_DrawScaledPixel(CENTERX, CENTERY + 1, color);
    AM_DrawScaledPixel(CENTERX, CENTERY + 2, color);
}

static void AM_setFrameVariables(void)
{
    if (am_rotatemode)
    {
        int     angle = (ANG90 - plr->mo->angle) >> ANGLETOFINESHIFT;
        float   dx, dy;
        fixed_t r;

        am_frame.sin = finesine[angle];
        am_frame.cos = finecosine[angle];

        am_frame.centerx = m_x + m_w / 2;
        am_frame.centery = m_y + m_h / 2;

        dx = (float)(m_x2 - am_frame.centerx);
        dy = (float)(m_y2 - am_frame.centery);
        r = (fixed_t)sqrt(dx * dx + dy * dy);

        am_frame.bbox[BOXLEFT] = am_frame.centerx - r;
        am_frame.bbox[BOXRIGHT] = am_frame.centerx + r;
        am_frame.bbox[BOXBOTTOM] = am_frame.centery - r;
        am_frame.bbox[BOXTOP] = am_frame.centery + r;
    }
    else
    {
        am_frame.bbox[BOXLEFT] = m_x;
        am_frame.bbox[BOXRIGHT] = m_x2;
        am_frame.bbox[BOXBOTTOM] = m_y;
        am_frame.bbox[BOXTOP] = m_y2;
    }
}

void AM_Drawer(void)
{
    AM_setFrameVariables();
    AM_clearFB();
    AM_drawWalls();
    if (am_grid)
        AM_drawGrid();
    if (plr->cheats & CF_ALLMAP_THINGS)
        AM_drawThings();
    if (markpointnum)
        AM_drawMarks();
    AM_drawPlayers();
    if (!am_followmode)
        AM_drawCrosshair();
}
