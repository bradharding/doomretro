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

#include <math.h>
#include <string.h>

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_gamecontroller.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "st_stuff.h"

// Automap color priorities
#define PATHPRIORITY           11
#define WALLPRIORITY           10
#define DOORPRIORITY            9
#define CDWALLPRIORITY          8
#define FDWALLPRIORITY          7
#define TELEPORTERPRIORITY      6
#define TSWALLPRIORITY          5
#define ALLMAPWALLPRIORITY      4
#define ALLMAPCDWALLPRIORITY    3
#define ALLMAPFDWALLPRIORITY    2
#define GRIDPRIORITY            1

static byte playercolor;
static byte thingcolor;
static byte bloodsplatcolor;
static byte bluekeycolor;
static byte redkeycolor;
static byte yellowkeycolor;
static byte markcolor;
static byte backcolor;
static byte pathcolor;

static byte *wallcolor;
static byte *bluedoorcolor;
static byte *reddoorcolor;
static byte *yellowdoorcolor;
static byte *allmapwallcolor;
static byte *teleportercolor;
static byte *fdwallcolor;
static byte *allmapfdwallcolor;
static byte *cdwallcolor;
static byte *allmapcdwallcolor;
static byte *tswallcolor;
static byte *gridcolor;
static byte *am_crosshaircolor2;

// scale on entry
// [BH] changed to initial zoom level of E1M1: Hangar so each map zoom level is consistent
#define INITSCALEMTOF   125114

// how much the automap moves window per tic in map coordinates
// moves 140 pixels in 1 second
#define F_PANINC        ((uint64_t)8 << speedtoggle)

// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((fixed_t)((uint64_t)FRACUNIT * (1.0 + F_PANINC / 100.0)))

// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((fixed_t)((uint64_t)FRACUNIT / (1.0 + F_PANINC / 100.0)))

#define PLAYERRADIUS    (16 * (1 << MAPBITS))

#define BLOODSPLATWIDTH (((12 << FRACBITS) >> FRACTOMAPBITS) / 8)

// translates between frame-buffer and map distances
#define FTOM(x)         (fixed_t)((((uint64_t)(x) << FRACBITS) * scale_ftom) >> FRACBITS)
#define MTOF(x)         (fixed_t)((((uint64_t)(x) * scale_mtof) >> FRACBITS) >> FRACBITS)

// translates between frame-buffer and map coordinates
#define CXMTOF(x)       MTOF((uint64_t)(x) - m_x)
#define CYMTOF(y)       (MAPHEIGHT - MTOF((uint64_t)(y) - m_y))

typedef struct
{
    mpoint_t    a;
    mpoint_t    b;
} mline_t;

bool                automapactive;

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

int                 lastlevel = -1;
int                 lastepisode = -1;

mpoint_t            *markpoints;    // where the points are
int                 markpointnum;   // next point to be assigned
int                 markpointnum_max;

mpoint_t            *pathpoints;
int                 pathpointnum;
int                 pathpointnum_max;

static int          gridwidth;
static int          gridheight;

static bool         bigstate;
static bool         movement;
static bool         speedtoggle;
static SDL_Keymod   modstate;
int                 direction;

am_frame_t          am_frame;

static bool         isteleportline[NUMLINESPECIALS];

static void AM_Rotate(fixed_t *x, fixed_t *y, angle_t angle);
static void (*putbigdot)(unsigned int, unsigned int, const byte *);
static void PUTDOT(unsigned int x, unsigned int y, const byte *color);
static void PUTBIGDOT(unsigned int x, unsigned int y, const byte *color);

static void AM_ActivateNewScale(void)
{
    m_x += m_w / 2;
    m_y += m_h / 2;
    m_w = FTOM(MAPWIDTH);
    m_h = FTOM(MAPHEIGHT);
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
    b = FixedDiv(MAPHEIGHT << FRACBITS, (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS));

    min_scale_mtof = MIN(a, b);
    max_scale_mtof = FixedDiv(MAPHEIGHT << FRACBITS, 2 * PLAYERRADIUS);
}

static void AM_ChangeWindowLoc(void)
{
    fixed_t         incx = m_paninc.x;
    fixed_t         incy = m_paninc.y;
    const fixed_t   width = m_w / 2;
    const fixed_t   height = m_h / 2;

    if (am_rotatemode)
        AM_Rotate(&incx, &incy, (viewangle - ANG90) >> ANGLETOFINESHIFT);

    m_x = BETWEEN(min_x, m_x + width + incx, max_x) - width;
    m_y = BETWEEN(min_y, m_y + height + incy, max_y) - height;
}

void AM_SetColors(void)
{
    byte        priority[256] = { 0 };
    static byte priorities[256 * 256];

    priority[nearestcolors[am_pathcolor]] = PATHPRIORITY;
    priority[nearestcolors[am_wallcolor]] = WALLPRIORITY;
    priority[nearestcolors[am_bluedoorcolor]] = DOORPRIORITY;
    priority[nearestcolors[am_reddoorcolor]] = DOORPRIORITY;
    priority[nearestcolors[am_yellowdoorcolor]] = DOORPRIORITY;
    priority[nearestcolors[am_cdwallcolor]] = CDWALLPRIORITY;
    priority[nearestcolors[am_fdwallcolor]] = FDWALLPRIORITY;
    priority[nearestcolors[am_teleportercolor]] = TELEPORTERPRIORITY;
    priority[nearestcolors[am_tswallcolor]] = TSWALLPRIORITY;
    priority[nearestcolors[am_allmapwallcolor]] = ALLMAPWALLPRIORITY;
    priority[nearestcolors[am_allmapcdwallcolor]] = ALLMAPCDWALLPRIORITY;
    priority[nearestcolors[am_allmapfdwallcolor]] = ALLMAPFDWALLPRIORITY;
    priority[nearestcolors[am_gridcolor]] = GRIDPRIORITY;

    playercolor = nearestcolors[am_playercolor];
    thingcolor = nearestcolors[am_thingcolor];
    bloodsplatcolor = nearestcolors[am_bloodsplatcolor];
    bluekeycolor = nearestcolors[am_bluekeycolor];
    redkeycolor = nearestcolors[am_redkeycolor];
    yellowkeycolor = nearestcolors[am_yellowkeycolor];
    markcolor = nearestcolors[am_markcolor];
    backcolor = nearestcolors[am_backcolor];
    pathcolor = nearestcolors[am_pathcolor];

    for (mobjtype_t i = 0; i < NUMMOBJTYPES; i++)
        mobjinfo[i].automapcolor = thingcolor;

    mobjinfo[MT_MISC4].automapcolor = bluekeycolor;
    mobjinfo[MT_MISC5].automapcolor = redkeycolor;
    mobjinfo[MT_MISC6].automapcolor = yellowkeycolor;
    mobjinfo[MT_MISC7].automapcolor = yellowkeycolor;
    mobjinfo[MT_MISC8].automapcolor = redkeycolor;
    mobjinfo[MT_MISC9].automapcolor = bluekeycolor;

    am_crosshaircolor2 = &tinttab60[nearestcolors[am_crosshaircolor] << 8];

    for (int x = 0; x < 256; x++)
        for (int y = 0; y < 256; y++)
            priorities[(x << 8) + y] = (priority[x] > priority[y] ? x : y);

    wallcolor = &priorities[nearestcolors[am_wallcolor] << 8];
    bluedoorcolor = &priorities[nearestcolors[am_bluedoorcolor] << 8];
    reddoorcolor = &priorities[nearestcolors[am_reddoorcolor] << 8];
    yellowdoorcolor = &priorities[nearestcolors[am_yellowdoorcolor] << 8];
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
    AM_SetAutomapSize(r_screensize);

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

void AM_SetAutomapSize(int screensize)
{
    if (!mapwindow)
    {
        MAPWIDTH = SCREENWIDTH;
        MAPHEIGHT = SCREENHEIGHT - SBARHEIGHT * (screensize < r_screensize_max);
        MAPAREA = MAPWIDTH * MAPHEIGHT;
    }

    MAPBOTTOM = MAPWIDTH * (MAPHEIGHT - 1);

    m_w = FTOM(MAPWIDTH);
    m_h = FTOM(MAPHEIGHT);
}

static void AM_InitVariables(const bool mainwindow)
{
    automapactive = mainwindow;

    m_paninc.x = 0;
    m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(MAPWIDTH);
    m_h = FTOM(MAPHEIGHT);
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
    D_FadeScreen(false);
}

void AM_Start(const bool mainwindow)
{
    if (lastlevel != gamemap || lastepisode != gameepisode || !mainwindow)
    {
        AM_LevelInit();
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }

    stat_automapopened = SafeAdd(stat_automapopened, 1);

    if (viewplayer)
        viewplayer->automapopened++;

    AM_InitVariables(mainwindow);
    D_FadeScreen(false);
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

static bool AM_GetSpeedToggle(void)
{
    return ((!!(gamecontrollerbuttons & GAMECONTROLLER_LEFT_TRIGGER)) ^ (!!(modstate & KMOD_SHIFT)));
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

void AM_ToggleFollowMode(bool value)
{
    if ((am_followmode = value))
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
// adds a mark at the current location
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
            markpointnum_max = 0;
            markpoints = NULL;
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
    const int   x = viewx >> FRACTOMAPBITS;
    const int   y = viewy >> FRACTOMAPBITS;
    static int  prevx = INT_MAX;
    static int  prevy = INT_MAX;

    if (x == prevx && y == prevy)
        return;

    if (pathpointnum >= pathpointnum_max)
        pathpoints = I_Realloc(pathpoints, (pathpointnum_max *= 2) * sizeof(*pathpoints));

    pathpoints[pathpointnum].x = prevx = x;
    pathpoints[pathpointnum++].y = prevy = y;
}

void AM_ToggleRotateMode(bool value)
{
    if ((am_rotatemode = value))
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
bool AM_Responder(const event_t *ev)
{
    int rc = false;

    direction = 0;
    modstate = SDL_GetModState();

    if (!menuactive && !paused)
    {
        static bool backbuttondown;

        if (!(gamecontrollerbuttons & gamecontrollerautomap))
            backbuttondown = false;

        if (!automapactive && !mapwindow)
        {
            if ((ev->type == ev_keydown && ev->data1 == keyboardautomap && keydown != keyboardautomap && !(modstate & KMOD_ALT))
                || (ev->type == ev_controller && (gamecontrollerbuttons & gamecontrollerautomap) && !backbuttondown))
            {
                keydown = keyboardautomap;
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
                if (key == keyboardright || key == keyboardstraferight || key == keyboardstraferight2)
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
                else if (key == keyboardleft || key == keyboardstrafeleft || key == keyboardstrafeleft2)
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
                else if (key == keyboardforward || key == keyboardforward2)
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
                else if (key == keyboardback || key == keyboardback2)
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
                else if (key == keyboardzoomout && !movement && (!mapwindow || keyboardzoomout != KEY_MINUS))
                {
                    keydown = key;
                    AM_ToggleZoomOut();
                }

                // zoom in
                else if (key == keyboardzoomin && !movement && (!mapwindow || keyboardzoomin != KEY_EQUALS))
                {
                    keydown = key;
                    AM_ToggleZoomIn();
                }

                // leave automap
                else if (key == keyboardautomap && !(modstate & KMOD_ALT) && keydown != keyboardautomap && !mapwindow)
                {
                    keydown = key;
                    viewactive = true;
                    AM_Stop();
                }

                // toggle maximum zoom
                else if (key == keyboardmaxzoom && !idclev && !idmus)
                {
                    if (keydown != keyboardmaxzoom)
                    {
                        keydown = key;
                        AM_ToggleMaxZoom();
                    }
                }

                // toggle follow mode
                else if (key == keyboardfollowmode)
                {
                    if (keydown != keyboardfollowmode)
                    {
                        keydown = key;
                        AM_ToggleFollowMode(!am_followmode);
                    }
                }

                // toggle grid
                else if (key == keyboardgrid)
                {
                    if (keydown != keyboardgrid)
                    {
                        keydown = key;
                        AM_ToggleGrid();
                    }
                }

                // mark spot
                else if (key == keyboardmark)
                {
                    if (keydown != keyboardmark)
                    {
                        keydown = key;
                        AM_AddMark();
                    }
                }

                // clear mark(s)
                else if (key == keyboardclearmark)
                    AM_ClearMarks();

                // toggle rotate mode
                else if (key == keyboardrotatemode)
                {
                    if (keydown != keyboardrotatemode)
                    {
                        keydown = key;
                        AM_ToggleRotateMode(!am_rotatemode);
                    }
                }
                else
                    rc = false;
            }
            else if (ev->type == ev_keyup)
            {
                key = ev->data1;

                if (key == keyboardclearmark)
                    markpress = 0;

                if ((key == keyboardzoomout || key == keyboardzoomin) && !movement)
                {
                    mtof_zoommul = FRACUNIT;
                    ftom_zoommul = FRACUNIT;
                }
                else if (key == keyboardfollowmode)
                {
                    int key2 = 0;

                    if (keystate(keyboardleft))
                        key2 = keyboardleft;
                    else if (keystate(keyboardstrafeleft))
                        key2 = keyboardstrafeleft;
                    else if (keystate(keyboardstrafeleft2))
                        key2 = keyboardstrafeleft2;
                    else if (keystate(keyboardright))
                        key2 = keyboardright;
                    else if (keystate(keyboardstraferight))
                        key2 = keyboardstraferight;
                    else if (keystate(keyboardstraferight2))
                        key2 = keyboardstraferight2;
                    else if (keystate(keyboardforward))
                        key2 = keyboardforward;
                    else if (keystate(keyboardforward2))
                        key2 = keyboardforward2;
                    else if (keystate(keyboardback))
                        key2 = keyboardback;
                    else if (keystate(keyboardback2))
                        key2 = keyboardback2;

                    if (key2)
                    {
                        event_t temp;

                        temp.type = ev_keydown;
                        temp.data1 = key2;
                        temp.data2 = 0;
                        D_PostEvent(&temp);
                    }
                }
                else if (!am_followmode)
                {
                    if (key == keyboardleft || key == keyboardstrafeleft || key == keyboardstrafeleft2)
                    {
                        speedtoggle = AM_GetSpeedToggle();

                        if (keystate(keyboardright) || keystate(keyboardstraferight) || keystate(keyboardstraferight2))
                            m_paninc.x = FTOM(F_PANINC);
                        else
                            m_paninc.x = 0;
                    }
                    else if (key == keyboardright || key == keyboardstraferight || key == keyboardstraferight2)
                    {
                        speedtoggle = AM_GetSpeedToggle();

                        if (keystate(keyboardleft) || keystate(keyboardstrafeleft) || keystate(keyboardstrafeleft2))
                            m_paninc.x = -FTOM(F_PANINC);
                        else
                            m_paninc.x = 0;
                    }
                    else if (key == keyboardforward || key == keyboardforward2)
                    {
                        speedtoggle = AM_GetSpeedToggle();

                        if (keystate(keyboardback) || keystate(keyboardback2))
                            m_paninc.y = FTOM(F_PANINC);
                        else
                            m_paninc.y = 0;
                    }
                    else if (key == keyboardback || key == keyboardback2)
                    {
                        speedtoggle = AM_GetSpeedToggle();

                        if (keystate(keyboardforward) || keystate(keyboardforward2))
                            m_paninc.y = -FTOM(F_PANINC);
                        else
                            m_paninc.y = 0;
                    }
                }
            }
            else if (ev->type == ev_mousewheel && !mapwindow)
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
            else if (ev->type == ev_controller && gamecontrollerwait < I_GetTime())
            {
                if ((gamecontrollerbuttons & gamecontrollerautomap) && !backbuttondown)
                {
                    gamecontrollerwait = I_GetTime() + 8;
                    viewactive = true;
                    backbuttondown = true;
                    AM_Stop();
                }

                // zoom out
                else if ((gamecontrollerbuttons & gamecontrollerzoomout)
                    && !(gamecontrollerbuttons & gamecontrollerzoomin))
                {
                    movement = true;
                    AM_ToggleZoomOut();
                }

                // zoom in
                else if ((gamecontrollerbuttons & gamecontrollerzoomin)
                    && !(gamecontrollerbuttons & gamecontrollerzoomout))
                {
                    movement = true;
                    AM_ToggleZoomIn();
                }

                // toggle maximum zoom
                else if ((gamecontrollerbuttons & gamecontrollermaxzoom) && !idclev && !idmus)
                {
                    AM_ToggleMaxZoom();
                    gamecontrollerwait = I_GetTime() + 12;
                }

                // toggle follow mode
                else if (gamecontrollerbuttons & gamecontrollerfollowmode)
                {
                    AM_ToggleFollowMode(!am_followmode);
                    gamecontrollerwait = I_GetTime() + 12;
                }

                // toggle grid
                else if (gamecontrollerbuttons & gamecontrollergrid)
                {
                    AM_ToggleGrid();
                    gamecontrollerwait = I_GetTime() + 12;
                }

                // mark spot
                else if ((gamecontrollerbuttons & gamecontrollermark))
                {
                    AM_AddMark();
                    gamecontrollerwait = I_GetTime() + 12;
                }

                // clear mark(s)
                else if (gamecontrollerbuttons & gamecontrollerclearmark)
                {
                    AM_ClearMarks();
                    gamecontrollerwait = I_GetTime() + 12;
                }

                // toggle rotate mode
                else if (gamecontrollerbuttons & gamecontrollerrotatemode)
                {
                    AM_ToggleRotateMode(!am_rotatemode);
                    gamecontrollerwait = I_GetTime() + 12;
                }

                if (!am_followmode)
                {
                    // pan right with left thumbstick
                    if (gamecontrollerthumbLX > 0)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = (fixed_t)(FTOM(F_PANINC) * ((float)gamecontrollerthumbLX / SHRT_MAX) * 1.2f);
                    }

                    // pan left with left thumbstick
                    else if (gamecontrollerthumbLX < 0)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = (fixed_t)(FTOM(F_PANINC) * ((float)(gamecontrollerthumbLX) / SHRT_MAX) * 1.2f);
                    }

                    // pan right with right thumbstick
                    if (gamecontrollerthumbRX > 0 && gamecontrollerthumbRX > gamecontrollerthumbLX)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = (fixed_t)(FTOM(F_PANINC) * ((float)gamecontrollerthumbRX / SHRT_MAX) * 1.2f);
                    }

                    // pan left with right thumbstick
                    else if (gamecontrollerthumbRX < 0 && gamecontrollerthumbRX < gamecontrollerthumbLX)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.x = (fixed_t)(FTOM(F_PANINC) * ((float)(gamecontrollerthumbRX) / SHRT_MAX) * 1.2f);
                    }

                    // pan up with left thumbstick
                    if (gamecontrollerthumbLY < 0)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = (fixed_t)(FTOM(F_PANINC) * (-(float)(gamecontrollerthumbLY) / SHRT_MAX) * 1.2f);
                    }

                    // pan down with left thumbstick
                    else if (gamecontrollerthumbLY > 0)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = -(fixed_t)(FTOM(F_PANINC) * ((float)gamecontrollerthumbLY / SHRT_MAX) * 1.2f);
                    }

                    // pan up with right thumbstick
                    if (gamecontrollerthumbRY < 0 && gamecontrollerthumbRY < gamecontrollerthumbLY)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = -(fixed_t)(FTOM(F_PANINC) * ((float)(gamecontrollerthumbRY) / SHRT_MAX) * 1.2f);
                    }

                    // pan down with right thumbstick
                    else if (gamecontrollerthumbRY > 0 && gamecontrollerthumbRY > gamecontrollerthumbLY)
                    {
                        movement = true;
                        speedtoggle = AM_GetSpeedToggle();
                        m_paninc.y = -(fixed_t)(FTOM(F_PANINC) * ((float)gamecontrollerthumbRY / SHRT_MAX) * 1.2f);
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
    const fixed_t   cosine = finecosine[angle];
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
    if (!automapactive && !mapwindow)
        return;

    if (am_followmode)
        AM_DoFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
        AM_ChangeWindowScale();

    // Change x,y location
    if ((m_paninc.x || m_paninc.y) && !consoleactive && !paused)
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
    memset(mapscreen, backcolor, MAPAREA);
}

//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly faster reject and precalculated
// slopes. If the speed is needed, use a hash algorithm to handle the common cases.
static bool AM_ClipMline(int *x0, int *y0, int *x1, int *y1)
{
    enum
    {
        LEFT   = 1,
        RIGHT  = 2,
        TOP    = 4,
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
    else if (*y0 >= (int)MAPHEIGHT)
        outcode1 |= BOTTOM;

    if (*y1 < 0)
        outcode2 |= TOP;
    else if (*y1 >= (int)MAPHEIGHT)
        outcode2 |= BOTTOM;

    return !(outcode1 & outcode2);
}

static inline void _PUTDOT(byte *dot, const byte *color)
{
    *dot = *(*dot + color);
}

static inline void PUTDOT(unsigned int x, unsigned int y, const byte *color)
{
    if (x < (unsigned int)MAPWIDTH && y < MAPAREA)
    {
        byte    *dot = mapscreen + y + x;

        *dot = *(*dot + color);
    }
}

static inline void PUTDOT2(unsigned int x, unsigned int y, const byte *color)
{
    if (x < (unsigned int)MAPWIDTH && y < MAPAREA)
        *(mapscreen + y + x) = *color;
}

static inline void PUTBIGDOT(unsigned int x, unsigned int y, const byte *color)
{
    if (x < (unsigned int)MAPWIDTH)
    {
        byte        *dot = mapscreen + y + x;
        const bool  attop = (y < MAPAREA);
        const bool  atbottom = (y < (unsigned int)MAPBOTTOM);

        if (attop)
            *dot = *(*dot + color);

        if (atbottom)
            _PUTDOT(dot + MAPWIDTH, color);

        if (x + 1 < (unsigned int)MAPWIDTH)
        {
            if (attop)
                _PUTDOT(dot + 1, color);

            if (atbottom)
            {
                dot += (size_t)MAPWIDTH + 1;
                *dot = *(*dot + color);
            }
        }
    }
    else if (++x < (unsigned int)MAPWIDTH)
    {
        byte    *dot = mapscreen + y + x;

        if (y < MAPAREA)
            *dot = *(*dot + color);

        if (y < (unsigned int)MAPBOTTOM)
        {
            dot += MAPWIDTH;
            *dot = *(*dot + color);
        }
    }
}

static inline void PUTTRANSLUCENTDOT(unsigned int x, unsigned int y, const byte *color)
{
    if (x < (unsigned int)MAPWIDTH && y < MAPAREA)
    {
        byte    *dot = mapscreen + y + x;

        if (*dot != tinttab66[*color])
            *dot = tinttab66[(*dot << 8) + *color];
    }
}

//
// Classic Bresenham w/ whatever optimizations needed for speed
//
static void AM_DrawFline(int x0, int y0, int x1, int y1, const byte *color,
    void (*putdot)(unsigned int, unsigned int, const byte *))
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

            y0 = BETWEEN(-MAPWIDTH, y0 * MAPWIDTH, MAPBOTTOM);
            y1 = BETWEEN(-MAPWIDTH, y1 * MAPWIDTH, MAPBOTTOM);

            putdot(x0, y0, color);

            while (y0 != y1)
                putdot(x0, (y0 += sy), color);
        }
        else
        {
            const int   sx = SIGN(dx);
            const int   sy = SIGN(dy) * MAPWIDTH;

            putdot(x0, (y0 *= MAPWIDTH), color);

            if ((dx = ABS(dx)) > (dy = ABS(dy)))
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
            else if (dx < dy)
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
            else
                // diagonal line
                while (x0 != x1)
                    putdot((x0 += sx), (y0 += sy), color);
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

static byte *AM_DoorColor(unsigned short special)
{
    if (GenLockedBase <= special && special < GenDoorBase)
    {
        if (!(special = ((special - GenLockedBase) & LockedKey) >> LockedKeyShift) || special == AllKeys)
            return cdwallcolor;
        else if (!(special = (special - 1) % 3))
            return reddoorcolor;
        else if (special == 1)
            return bluedoorcolor;
        else
            return yellowdoorcolor;
    }

    switch (special)
    {
        case DR_Door_Red_OpenWaitClose:
        case D1_Door_Red_OpenStay:
        case SR_Door_Red_OpenStay_Fast:
        case S1_Door_Red_OpenStay_Fast:
            return reddoorcolor;

        case DR_Door_Blue_OpenWaitClose:
        case D1_Door_Blue_OpenStay:
        case SR_Door_Blue_OpenStay_Fast:
        case S1_Door_Blue_OpenStay_Fast:
            return bluedoorcolor;

        case DR_Door_Yellow_OpenWaitClose:
        case D1_Door_Yellow_OpenStay:
        case SR_Door_Yellow_OpenStay_Fast:
        case S1_Door_Yellow_OpenStay_Fast:
            return yellowdoorcolor;

        default:
            return cdwallcolor;
    }
}

static void AM_DrawWalls(void)
{
    for (int i = 0; i < numlines; i++)
    {
        const line_t    line = lines[i];
        const fixed_t   *lbbox = line.bbox;

        if ((lbbox[BOXLEFT] >> FRACTOMAPBITS) <= am_frame.bbox[BOXRIGHT]
            && (lbbox[BOXRIGHT] >> FRACTOMAPBITS) >= am_frame.bbox[BOXLEFT]
            && (lbbox[BOXBOTTOM] >> FRACTOMAPBITS) <= am_frame.bbox[BOXTOP]
            && (lbbox[BOXTOP] >> FRACTOMAPBITS) >= am_frame.bbox[BOXBOTTOM])
        {
            const unsigned short    flags = line.flags;

            if ((flags & ML_MAPPED) && !(flags & ML_DONTDRAW))
            {
                mline_t                 mline;
                const unsigned short    special = line.special;
                byte                    *doorcolor;

                mline.a.x = line.v1->x >> FRACTOMAPBITS;
                mline.a.y = line.v1->y >> FRACTOMAPBITS;
                mline.b.x = line.v2->x >> FRACTOMAPBITS;
                mline.b.y = line.v2->y >> FRACTOMAPBITS;

                mline = rotatelinefunc(mline);

                if (special && (doorcolor = AM_DoorColor(special)) != cdwallcolor)
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, doorcolor, &PUTDOT);
                else
                {
                    const sector_t  *back = line.backsector;

                    if (!back || (flags & ML_SECRET))
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, wallcolor, putbigdot);
                    else if (isteleportline[special] && back->ceilingheight != back->floorheight
                        && ((flags & ML_TELEPORTTRIGGERED) || isteleport[back->floorpic]) && !(flags & ML_SECRET))
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, teleportercolor, &PUTDOT);
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
}

static void AM_DrawWalls_AllMap(void)
{
    for (int i = 0; i < numlines; i++)
    {
        const line_t    line = lines[i];
        const fixed_t   *lbbox = line.bbox;

        if ((lbbox[BOXLEFT] >> FRACTOMAPBITS) <= am_frame.bbox[BOXRIGHT]
            && (lbbox[BOXRIGHT] >> FRACTOMAPBITS) >= am_frame.bbox[BOXLEFT]
            && (lbbox[BOXBOTTOM] >> FRACTOMAPBITS) <= am_frame.bbox[BOXTOP]
            && (lbbox[BOXTOP] >> FRACTOMAPBITS) >= am_frame.bbox[BOXBOTTOM])
        {
            const unsigned short    flags = line.flags;

            if (!(flags & ML_DONTDRAW))
            {
                mline_t                 mline;
                const unsigned short    special = line.special;
                byte                    *doorcolor;

                mline.a.x = line.v1->x >> FRACTOMAPBITS;
                mline.a.y = line.v1->y >> FRACTOMAPBITS;
                mline.b.x = line.v2->x >> FRACTOMAPBITS;
                mline.b.y = line.v2->y >> FRACTOMAPBITS;

                mline = rotatelinefunc(mline);

                if (special && (doorcolor = AM_DoorColor(special)) != cdwallcolor)
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, doorcolor, &PUTDOT);
                else
                {
                    const sector_t  *back = line.backsector;

                    if (!back || (flags & ML_SECRET))
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y,
                            ((flags & ML_MAPPED) ? wallcolor : allmapwallcolor), putbigdot);
                    else if (isteleportline[special] && ((flags & ML_TELEPORTTRIGGERED) || isteleport[back->floorpic]))
                        AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y,
                            ((flags & ML_MAPPED) ? teleportercolor : allmapfdwallcolor), &PUTDOT);
                    else
                    {
                        const sector_t  *front = line.frontsector;

                        if (back->floorheight != front->floorheight)
                            AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y,
                                ((flags & ML_MAPPED) ? fdwallcolor : allmapfdwallcolor), &PUTDOT);
                        else if (back->ceilingheight != front->ceilingheight)
                            AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y,
                                ((flags & ML_MAPPED) ? cdwallcolor : allmapcdwallcolor), &PUTDOT);
                    }
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

        if ((lbbox[BOXLEFT] >> FRACTOMAPBITS) <= am_frame.bbox[BOXRIGHT]
            && (lbbox[BOXRIGHT] >> FRACTOMAPBITS) >= am_frame.bbox[BOXLEFT]
            && (lbbox[BOXBOTTOM] >> FRACTOMAPBITS) <= am_frame.bbox[BOXTOP]
            && (lbbox[BOXTOP] >> FRACTOMAPBITS) >= am_frame.bbox[BOXBOTTOM])
        {
            mline_t                 mline;
            const unsigned short    special = line.special;
            byte                    *doorcolor;

            mline.a.x = line.v1->x >> FRACTOMAPBITS;
            mline.a.y = line.v1->y >> FRACTOMAPBITS;
            mline.b.x = line.v2->x >> FRACTOMAPBITS;
            mline.b.y = line.v2->y >> FRACTOMAPBITS;

            mline = rotatelinefunc(mline);

            if (special && (doorcolor = AM_DoorColor(special)) != cdwallcolor)
                AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, doorcolor, &PUTDOT);
            else
            {
                const sector_t  *back = line.backsector;

                if (!back || (line.flags & ML_SECRET))
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, wallcolor, putbigdot);
                else if (isteleportline[special])
                    AM_DrawFline(mline.a.x, mline.a.y, mline.b.x, mline.b.y, teleportercolor, &PUTDOT);
                else
                {
                    const sector_t  *front = line.frontsector;

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

static void AM_DrawPlayerArrow(const mline_t *lineguy, const int lineguylines,
    const angle_t angle, const fixed_t x, const fixed_t y)
{
    for (int i = 0; i < lineguylines; i++)
    {
        const mline_t   line = lineguy[i];
        int             x1 = line.a.x;
        int             y1 = line.a.y;
        int             x2 = line.b.x;
        int             y2 = line.b.y;

        AM_Rotate(&x1, &y1, angle);
        AM_Rotate(&x2, &y2, angle);

        AM_DrawFline(x + x1, y + y1, x + x2, y + y2, &playercolor, &PUTDOT2);
    }
}

static void AM_DrawTranslucentPlayerArrow(const mline_t *lineguy, const int lineguylines,
    angle_t angle, const fixed_t x, const fixed_t y)
{
    for (int i = 0; i < lineguylines; i++)
    {
        const mline_t   line = lineguy[i];
        int             x1 = line.a.x;
        int             y1 = line.a.y;
        int             x2 = line.b.x;
        int             y2 = line.b.y;

        AM_Rotate(&x1, &y1, angle);
        AM_Rotate(&x2, &y2, angle);

        AM_DrawFline(x + x1, y + y1, x + x2, y + y2, &playercolor, &PUTTRANSLUCENTDOT);
    }
}

static void AM_DrawThingTriangle(const mline_t *lineguy, const int lineguylines, const fixed_t scale,
    const angle_t angle, const fixed_t x, const fixed_t y, const byte color)
{
    for (int i = 0; i < lineguylines; i++)
    {
        int             x1, y1;
        int             x2, y2;
        const mline_t   line = lineguy[i];

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

#define PLAYERARROWLINES        8
#define CHEATPLAYERARROWLINES  19

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

    const mobj_t    *mo = viewplayer->mo;
    const int       invisibility = viewplayer->powers[pw_invisibility];
    mpoint_t        point = { mo->x >> FRACTOMAPBITS, mo->y >> FRACTOMAPBITS };
    angle_t         angle = ANG90 >> ANGLETOFINESHIFT;

    if (am_rotatemode)
        AM_RotatePoint(&point);
    else
        angle = viewangle >> ANGLETOFINESHIFT;

    if (viewplayer->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS))
    {
        if (invisibility && (invisibility > STARTFLASHING || (invisibility & FLASHONTIC)))
            AM_DrawTranslucentPlayerArrow(cheatplayerarrow, CHEATPLAYERARROWLINES, angle, point.x, point.y);
        else
            AM_DrawPlayerArrow(cheatplayerarrow, CHEATPLAYERARROWLINES, angle, point.x, point.y);
    }
    else if (invisibility && (invisibility > STARTFLASHING || (invisibility & FLASHONTIC)))
        AM_DrawTranslucentPlayerArrow(playerarrow, PLAYERARROWLINES, angle, point.x, point.y);
    else
        AM_DrawPlayerArrow(playerarrow, PLAYERARROWLINES, angle, point.x, point.y);
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

    const angle_t   angleoffset = (am_rotatemode ? viewangle - ANG90 : 0);

    if (am_bloodsplatcolor != am_backcolor && r_blood != r_blood_none && r_bloodsplats_max)
        for (int i = 0; i < numsectors; i++)
        {
            bloodsplat_t    *splat = sectors[i].splatlist;
            const int       width = BLOODSPLATWIDTH;

            while (splat)
            {
                {
                    mpoint_t    point = { splat->x >> FRACTOMAPBITS, splat->y >> FRACTOMAPBITS };
                    int         fx, fy;

                    if (am_rotatemode)
                        AM_RotatePoint(&point);

                    if ((fx = CXMTOF(point.x)) >= -width && fx <= MAPWIDTH + width
                        && (fy = CYMTOF(point.y)) >= -width && fy <= (int)MAPHEIGHT + width)
                        AM_DrawThingTriangle(thingtriangle, THINGTRIANGLELINES, width,
                            (splat->angle - angleoffset) >> ANGLETOFINESHIFT, point.x, point.y, bloodsplatcolor);
                }

                splat = splat->next;
            }
        }

    for (int i = 0; i < numsectors; i++)
    {
        mobj_t  *thing = sectors[i].thinglist;

        while (thing)
        {
            if ((!thing->player || thing->player->mo != thing) && !(thing->flags2 & MF2_DONTMAP))
            {
                angle_t     angle;
                mpoint_t    point;
                int         fx, fy;
                const short sprite = sprites[thing->sprite].spriteframes[0].lump[0];
                const int   width = (BETWEEN(12 << FRACBITS, (spritewidth[sprite] + spriteheight[sprite]) / 2,
                                96 << FRACBITS) >> FRACTOMAPBITS) / 2;

                if (consoleactive || paused)
                {
                    angle = thing->angle;
                    point.x = thing->x >> FRACTOMAPBITS;
                    point.y = thing->y >> FRACTOMAPBITS;
                }
                else
                {
                    angle = R_InterpolateAngle(thing->oldangle, thing->angle, fractionaltic);
                    point.x = (thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic)) >> FRACTOMAPBITS;
                    point.y = (thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic)) >> FRACTOMAPBITS;
                }

                if (am_rotatemode)
                    AM_RotatePoint(&point);

                if ((fx = CXMTOF(point.x)) >= -width && fx <= MAPWIDTH + width
                    && (fy = CYMTOF(point.y)) >= -width && fy <= (int)MAPHEIGHT + width)
                    AM_DrawThingTriangle(thingtriangle, THINGTRIANGLELINES, width, (angle - angleoffset) >> ANGLETOFINESHIFT,
                        point.x, point.y, mobjinfo[thing->type].automapcolor);
            }

            thing = thing->snext;
        }
    }
}

#define MARKWIDTH   9
#define MARKHEIGHT 12

static void AM_DrawMarks(void)
{
    const char *marknums[] =
    {
        "022222200221111220211111120211221120211221120211221120"
        "211221120211221120211221120211111120221111220022222200",
        "002222000022112000021112000021112000022112000002112000"
        "002112000002112000022112200021111200021111200022222200",
        "022222200221111220211111120211221120222221120002211120"
        "022111220221112200211122220211111120211111120222222220",
        "022222200221111220211111120211221120222221120002111220"
        "002111120222221120211221120211111120221111220022222200",
        "000222220000211120002211120002111120022111120021121120"
        "221121122211111112211111112222221122000021120000022220",
        "222222220211111120211111120211222220211222200211111220"
        "211111120222221120211221120211111120221111220022222200",
        "022222200221111220211111120211221120211222220211111220"
        "211111120211221120211221120211111120221111220022222200",
        "222222220211111120211111120222221120000211220002211200"
        "002112200022112000021122000021120000021120000022220000",
        "022222200221111220211111120211221120211221120221111220"
        "211111120211221120211221120211111120221111220022222200",
        "022222200221111220211111120211221120211221120211111120"
        "221111120222221120211221120211111120221111220022222200"
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

                if (fx < (unsigned int)MAPWIDTH)
                {
                    const unsigned int  fy = y + j / MARKWIDTH;

                    if (fy < MAPHEIGHT)
                    {
                        const char  src = marknums[digit][j];

                        if (src == '1')
                            mapscreen[fy * MAPWIDTH + fx] = markcolor;
                        else if (src == '2')
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
    if (pathpointnum > 1)
    {
        mpoint_t    end;

        if (am_rotatemode)
        {
            mpoint_t    player = { viewx >> FRACTOMAPBITS, viewy >> FRACTOMAPBITS };

            for (int i = 1; i < pathpointnum; i++)
            {
                mpoint_t    start = { pathpoints[i - 1].x, pathpoints[i - 1].y };

                end.x = pathpoints[i].x;
                end.y = pathpoints[i].y;

                if (ABS(start.x - end.x) > 4 * FRACUNIT || ABS(start.y - end.y) > 4 * FRACUNIT)
                    continue;

                AM_RotatePoint(&start);
                AM_RotatePoint(&end);
                AM_DrawFline(start.x, start.y, end.x, end.y, &pathcolor, &PUTDOT2);
            }

            AM_RotatePoint(&player);
            AM_DrawFline(end.x, end.y, player.x, player.y, &pathcolor, &PUTDOT2);
        }
        else
        {
            for (int i = 1; i < pathpointnum; i++)
            {
                const mpoint_t  start = { pathpoints[i - 1].x, pathpoints[i - 1].y };

                end.x = pathpoints[i].x;
                end.y = pathpoints[i].y;

                if (ABS(start.x - end.x) > 4 * FRACUNIT || ABS(start.y - end.y) > 4 * FRACUNIT)
                    continue;

                AM_DrawFline(start.x, start.y, end.x, end.y, &pathcolor, &PUTDOT2);
            }

            AM_DrawFline(end.x, end.y, viewx >> FRACTOMAPBITS, viewy >> FRACTOMAPBITS, &pathcolor, &PUTDOT2);
        }
    }
}

static void AM_DrawCrosshair(void)
{
    byte    *dot = &mapscreen[(MAPHEIGHT - 3) * MAPWIDTH / 2 - 1];

    *dot = *(*dot + am_crosshaircolor2);
    dot += MAPWIDTH;
    *dot = *(*dot + am_crosshaircolor2);
    dot += (size_t)MAPWIDTH - 2;
    *dot = *(*dot + am_crosshaircolor2);
    dot++;
    *dot = *(*dot + am_crosshaircolor2);
    dot++;
    *dot = *(*dot + am_crosshaircolor2);
    dot++;
    *dot = *(*dot + am_crosshaircolor2);
    dot++;
    *dot = *(*dot + am_crosshaircolor2);
    dot += (size_t)MAPWIDTH - 2;
    *dot = *(*dot + am_crosshaircolor2);
    dot += MAPWIDTH;
    *dot = *(*dot + am_crosshaircolor2);
}

static void AM_DrawSolidCrosshair(void)
{
    byte    *dot = &mapscreen[(MAPHEIGHT - 3) * MAPWIDTH / 2 - 1];

    *dot = am_crosshaircolor;
    dot += MAPWIDTH;
    *dot = am_crosshaircolor;
    dot += (size_t)MAPWIDTH - 2;
    *dot++ = am_crosshaircolor;
    *dot++ = am_crosshaircolor;
    *dot++ = am_crosshaircolor;
    *dot++ = am_crosshaircolor;
    *dot = am_crosshaircolor;
    dot += (size_t)MAPWIDTH - 2;
    *dot = am_crosshaircolor;
    dot += MAPWIDTH;
    *dot = am_crosshaircolor;
}

void AM_StatusBarShadow(void)
{
    for (int i = 24, y = 0; y < 6; i -= 4, y++)
    {
        byte    *colormap = &colormaps[0][i * 256];

        for (int x = 0; x < MAPWIDTH; x++)
        {
            byte    *dot = &mapscreen[(MAPHEIGHT - y - 1) * MAPWIDTH + x];

            *dot = *(*dot + colormap);
        }
    }
}

static void AM_SetFrameVariables(void)
{
    const fixed_t   dx = m_w / 2;
    const fixed_t   dy = m_h / 2;
    const fixed_t   x = m_x + dx;
    const fixed_t   y = m_y + dy;

    am_frame.center.x = x;
    am_frame.center.y = y;

    if (am_rotatemode)
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

    skippsprinterp = true;

    if (viewplayer->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS))
        AM_DrawWalls_Cheating();
    else if (viewplayer->powers[pw_allmap])
        AM_DrawWalls_AllMap();
    else
        AM_DrawWalls();

    if (am_grid)
        AM_DrawGrid();

    if (am_path)
        AM_DrawPath();

    if (viewplayer->cheats & CF_ALLMAP_THINGS)
        AM_DrawThings();

    if (markpointnum)
        AM_DrawMarks();

    AM_DrawPlayer();

    if (r_screensize < r_screensize_max && !vanilla)
        AM_StatusBarShadow();

    if (!am_followmode)
    {
        if (r_hud_translucency)
            AM_DrawCrosshair();
        else
            AM_DrawSolidCrosshair();
    }
}
