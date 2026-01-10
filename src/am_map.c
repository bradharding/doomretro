/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <math.h>

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
#include "m_bbox.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// Automap color priorities
#define SECRETPRIORITY         10
#define WALLPRIORITY            9
#define DOORPRIORITY            8
#define CDWALLPRIORITY          7
#define FDWALLPRIORITY          6
#define TELEPORTERPRIORITY      5
#define TSWALLPRIORITY          4
#define ALLMAPWALLPRIORITY      3
#define ALLMAPCDWALLPRIORITY    2
#define ALLMAPFDWALLPRIORITY    1

#define MAXINTERSECTIONS        256
#define INSERTIONSORTTHRESHOLD  32

#define SIDETICKLENGTH          (3 << MAPBITS)

static byte playercolor;
static byte thingcolor;
static byte bloodsplatcolor;
static byte corpsecolor;
static byte bluekeycolor;
static byte redkeycolor;
static byte yellowkeycolor;
static byte markcolor;
static byte backcolor;
static byte pathcolor;
static byte gridcolor;

static byte *wallcolor;
static byte *bluedoorcolor;
static byte *reddoorcolor;
static byte *yellowdoorcolor;
static byte *secretcolor;
static byte *allmapwallcolor;
static byte *teleportercolor;
static byte *fdwallcolor;
static byte *allmapfdwallcolor;
static byte *cdwallcolor;
static byte *allmapcdwallcolor;
static byte *tswallcolor;
static byte *am_crosshaircolor2;

static byte *floorpiccolor;

// scale on entry
// [BH] changed to initial zoom level of E1M1: Hangar so each map zoom level is consistent
#define INITSCALEMTOF           125114

// minimum scale needed to use big dots for solid walls
#define USEBIGDOTS              (FRACUNIT * 3 / 2)

// how much the automap moves window per tic in map coordinates
// moves 140 pixels in 1 second
#define F_PANINC                (8 << speedtoggle)

// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN                (fixed_t)((uint64_t)FRACUNIT * (1.0 + F_PANINC / 100.0))

// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT               (fixed_t)((uint64_t)FRACUNIT / (1.0 + F_PANINC / 100.0))

#define PLAYERRADIUS            (16 * (1 << MAPBITS))

#define BLOODSPLATWIDTH         (((12 << FRACBITS) >> FRACTOMAPBITS) / 4)

// translates between frame-buffer and map distances
#define FTOM(x)                 (fixed_t)((((int64_t)(x) << FRACBITS) * scale_ftom) >> FRACBITS)
#define MTOF(x)                 (fixed_t)((((int64_t)(x) * scale_mtof) >> FRACBITS) >> FRACBITS)

// translates between frame-buffer and map coordinates
#define CXMTOF(x)               MTOF((x) - m_x)
#define CYMTOF(y)               (MAPHEIGHT - MTOF((y) - m_y))

typedef struct
{
    mpoint_t    a;
    mpoint_t    b;
} mline_t;

typedef struct
{
    int         x1, y1;
    int         x2, y2;
    int         ymin, ymax;
    int         dx, dy;
} edge_t;

bool                automapactive;

static mpoint_t     m_paninc;       // how far the window pans each tic (map coords)
static fixed_t      mtof_zoommul;   // how far the window zooms in each tic (map coords)
static fixed_t      ftom_zoommul;   // how far the window zooms in each tic (fb coords)

// LL x,y where the window is on the map (map coords)
static fixed_t      m_x, m_y;

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

static fixed_t      panvel_x;
static fixed_t      panvel_y;
static int          pansign_x;
static int          pansign_y;

static bool         mousedragging;

static float        controllerpan_x;
static float        controllerpan_y;

static float        zoomvelocity;
static int          zoomsign;

int                 lastlevel = -1;
int                 lastepisode = -1;

mpoint_t            *mark;
int                 nummarks;
int                 maxmarks;
static int          markpress;

mpoint_t            *breadcrumb;
int                 numbreadcrumbs;
int                 maxbreadcrumbs;

static int          gridwidth;
static int          gridheight;

static bool         bigstate;
static bool         movement;
static bool         speedtoggle;
static SDL_Keymod   modstate;

am_frame_t          am_frame;

static bool         isteleportline[NUMLINESPECIALS];

static edge_t       *edges;
static int          edgescapacity;

static bool         usefixedcolormap;

static void AM_Rotate(fixed_t *x, fixed_t *y, const angle_t angle);
static void AM_RotatePoint(mpoint_t *point);
static void AM_CorrectAspectRatio(mpoint_t *point);
static void AM_ChangeWindowScale(void);
static void (*putbigwalldot)(int, int, const byte *);
static void (*putbigdot)(int, int, const byte *);
static void (*putbigdot2)(int, int, const byte *);
static inline void PUTDOT(int x, int y, const byte *color);
static inline void PUTDOT2(int x, int y, const byte *color);
static inline void PUTBIGDOT(int x, int y, const byte *color);
static inline void PUTBIGDOT2(int x, int y, const byte *color);

static byte AM_ColorFromFlat(const int flatnum)
{
    const byte  *flat;
    int         colors[256] = { 0 };
    int         bestcolor = backcolor;
    int         bestcount = -1;

    if (flatnum < 0 || flatnum >= numflats)
        return backcolor;

    if (!(flat = lumpinfo[flattranslation[flatnum]]->cache))
        return backcolor;

    for (int i = 0; i < 64 * 64; i++)
        colors[flat[i]]++;

    for (int i = 0; i < 256; i++)
        if (colors[i] > bestcount)
        {
            bestcolor = i;
            bestcount = colors[i];
        }

    if (bestcolor == backcolor)
        for (int i = 0, secondcount = -1; i < 256; i++)
            if (i != backcolor && colors[i] > secondcount)
            {
                bestcolor = i;
                secondcount = colors[i];
            }

    return (byte)bestcolor;
}

static void AM_BuildFloorPicColors(void)
{
    floorpiccolor = I_Malloc((size_t)numflats * sizeof(*floorpiccolor));

    for (int i = 0; i < numflats; i++)
        floorpiccolor[i] = AM_ColorFromFlat(i);
}

static int AM_CompareIntAscending(const void *a, const void *b)
{
    const int   ia = *(const int *)a;
    const int   ib = *(const int *)b;

    return ((ia > ib) - (ia < ib));
}

static inline void AM_SortIntersections(int *vals, const int n)
{
    if (n <= 1)
        return;

    if (n > INSERTIONSORTTHRESHOLD)
    {
        qsort(vals, (size_t)n, sizeof(*vals), AM_CompareIntAscending);
        return;
    }

    for (int i = 1; i < n; i++)
    {
        const int   key = vals[i];
        int         j = i - 1;

        while (j >= 0 && vals[j] > key)
        {
            vals[j + 1] = vals[j];
            j--;
        }

        vals[j + 1] = key;
    }
}

static int AM_ProjectSectorEdges(const sector_t *sector, edge_t *sectoredges)
{
    int edgecount = 0;
    int linecount = sector->linecount;

    for (int i = 0; i < linecount; i++)
    {
        const line_t    *line = sector->lines[i];

        if (line->frontsector != line->backsector)
        {
            mpoint_t    a = { line->v1->x >> FRACTOMAPBITS, line->v1->y >> FRACTOMAPBITS };
            mpoint_t    b = { line->v2->x >> FRACTOMAPBITS, line->v2->y >> FRACTOMAPBITS };
            int         x1, y1;
            int         x2, y2;
            edge_t      edge;

            if (am_rotatemode)
            {
                AM_RotatePoint(&a);
                AM_RotatePoint(&b);
            }

            if (am_correctaspectratio)
            {
                AM_CorrectAspectRatio(&a);
                AM_CorrectAspectRatio(&b);
            }

            y1 = CYMTOF(a.y);
            y2 = CYMTOF(b.y);

            if (y1 == y2)
                continue;

            x1 = CXMTOF(a.x);
            x2 = CXMTOF(b.x);

            if (y1 > y2)
            {
                SWAP(x1, x2);
                SWAP(y1, y2);
            }

            edge.x1 = x1;
            edge.y1 = y1;
            edge.x2 = x2;
            edge.y2 = y2;
            edge.ymin = y1;
            edge.ymax = y2;
            edge.dx = x2 - x1;
            edge.dy = y2 - y1;

            sectoredges[edgecount++] = edge;
        }
    }

    return edgecount;
}

static byte AM_AdjustColorForLightLevel(const sector_t *sector, const byte color, const int lightlevel)
{
    if (usefixedcolormap)
        return fixedcolormap[color];
    else
    {
        const int   colormap = (sector->floorlightsec ? sector->floorlightsec->colormap :
                        (sector->heightsec ? sector->heightsec->colormap : sector->colormap));

        return (&colormaps[colormap][BETWEEN(0, ((255 - BETWEEN(0, lightlevel, 255)) >> 3), 31) << 8])[color];
    }
}

static void AM_FillSector(const sector_t *sector)
{
    int         intersections[MAXINTERSECTIONS] = { 0 };
    fixed_t     minx = FIXED_MAX;
    fixed_t     miny = FIXED_MAX;
    fixed_t     maxx2 = FIXED_MIN;
    fixed_t     maxy2 = FIXED_MIN;
    int         maxyscreen = 0;
    int         minyscreen = MAPHEIGHT - 1;
    fixed_t     xstep;
    byte        fillcolor;
    int         edgecount;
    int         linecount = sector->linecount;
    short       floorpic;
    bool        validfloorpic;
    const byte  *flat = NULL;
    const int   lightlevel = sector->lightlevel;
    fixed_t     floorxoffset = 0;
    fixed_t     flooryoffset = 0;
    int         floorrotation = 0;

    if (linecount <= 2)
        return;

    for (int i = 0; i < linecount; i++)
    {
        const line_t    *line = sector->lines[i];
        const fixed_t   x1 = (line->v1->x >> FRACTOMAPBITS);
        const fixed_t   y1 = (line->v1->y >> FRACTOMAPBITS);
        const fixed_t   x2 = (line->v2->x >> FRACTOMAPBITS);
        const fixed_t   y2 = (line->v2->y >> FRACTOMAPBITS);

        minx = MIN(minx, MIN(x1, x2));
        maxx2 = MAX(maxx2, MAX(x1, x2));
        miny = MIN(miny, MIN(y1, y2));
        maxy2 = MAX(maxy2, MAX(y1, y2));
    }

    if ((maxx2 << FRACTOMAPBITS) < am_frame.bbox[BOXLEFT]
        || (minx << FRACTOMAPBITS) > am_frame.bbox[BOXRIGHT]
        || (maxy2 << FRACTOMAPBITS) < am_frame.bbox[BOXBOTTOM]
        || (miny << FRACTOMAPBITS) > am_frame.bbox[BOXTOP])
        return;

    if (linecount > edgescapacity)
    {
        edgescapacity = linecount;
        edges = I_Realloc(edges, (size_t)edgescapacity * sizeof(*edges));
    }

    if ((edgecount = AM_ProjectSectorEdges(sector, edges)) < 2)
        return;

    for (int i = 0; i < edgecount; i++)
    {
        minyscreen = MIN(minyscreen, edges[i].ymin);
        maxyscreen = MAX(maxyscreen, edges[i].ymax);
    }

    minyscreen = BETWEEN(0, minyscreen, MAPHEIGHT);
    maxyscreen = BETWEEN(0, maxyscreen, MAPHEIGHT);
    xstep = (fixed_t)((int64_t)m_w / MAPWIDTH);

    floorpic = sector->floorpic;
    validfloorpic = (floorpic >= 0 && floorpic < numflats
        && floorpic != skyflatnum && !(floorpic & PL_SKYFLAT) && r_textures);

    if (am_sectors == am_sectors_textures && validfloorpic)
        flat = (terraintypes[floorpic] >= LIQUID && r_liquid_swirl ?
            R_SwirlingFlat(floorpic) : lumpinfo[flattranslation[floorpic]]->cache);

    if (flat)
    {
        floorxoffset = (sector->floorxoffset >> FRACTOMAPBITS);
        flooryoffset = (sector->flooryoffset >> FRACTOMAPBITS);

        if (terraintypes[floorpic] >= LIQUID && r_liquid_current && !floorxoffset && !flooryoffset)
        {
            floorxoffset = (animatedliquidxoffs >> FRACTOMAPBITS);
            flooryoffset = (animatedliquidyoffs >> FRACTOMAPBITS);
        }

        floorrotation = (sector->heightsec ? sector->heightsec->floorrotation : sector->floorrotation);
    }
    else
        fillcolor = AM_AdjustColorForLightLevel(sector, (validfloorpic ? floorpiccolor[floorpic] :
            (r_textures ? backcolor : NOTEXTURECOLOR)), lightlevel);

    for (int y = minyscreen; y < maxyscreen; y++)
    {
        int     n = 0;
        fixed_t ry;

        for (int i = 0; i < edgecount; i++)
        {
            const edge_t    *edge = &edges[i];

            if (y < edge->ymin || y >= edge->ymax)
                continue;

            if (n == MAXINTERSECTIONS)
                break;

            intersections[n++] = edge->x1 + (int)(((int64_t)(y - edge->y1) * edge->dx) / edge->dy);
        }

        if (n < 2)
            continue;

        AM_SortIntersections(intersections, n);

        ry = (fixed_t)((int64_t)(MAPHEIGHT - y) * m_h / MAPHEIGHT) + m_y;

        if (am_correctaspectratio)
            ry = am_frame.center.y + (ry - am_frame.center.y) * 6 / 5;

        for (int i = 0, row = y * MAPWIDTH; i + 1 < n; i += 2)
        {
            int     x1 = intersections[i];
            int     x2 = intersections[i + 1];
            fixed_t rx;

            if (x1 > x2)
                SWAP(x1, x2);

            if (x2 < 0 || x1 >= MAPWIDTH)
                continue;

            x1 = MAX(0, x1);
            x2 = MIN(MAPWIDTH - 1, x2);

            if (!flat)
            {
                memset(&mapscreen[row + x1], fillcolor, (size_t)(x2 - x1 + 1));
                continue;
            }

            rx = m_x + (fixed_t)((int64_t)x1 * xstep);

            for (int x = x1; x <= x2; x++, rx += xstep)
            {
                fixed_t rrx = rx;
                fixed_t rry = ry;
                int     u, v;

                if (am_rotatemode)
                {
                    const fixed_t   dx = rrx - am_frame.center.x;
                    const fixed_t   dy = rry - am_frame.center.y;
                    const double    cosine = am_frame.cos;
                    const double    sine = am_frame.sin;

                    rrx = am_frame.center.x + (fixed_t)llround((double)dx * cosine + (double)dy * sine);
                    rry = am_frame.center.y + (fixed_t)llround(-(double)dx * sine + (double)dy * cosine);
                }

                rrx += floorxoffset;
                rry -= flooryoffset;

                if (floorrotation)
                    AM_Rotate(&rrx, &rry, floorrotation >> ANGLETOFINESHIFT);

                u = (int)(rrx >> MAPBITS) & 63;
                v = (int)(rry >> MAPBITS) & 63;

                mapscreen[row + x] = AM_AdjustColorForLightLevel(sector, flat[((63 - v) << 6) + u], lightlevel);
            }
        }
    }
}

static void AM_FillSectors(void)
{
    usefixedcolormap = (r_textures && viewplayer->fixedcolormap == INVERSECOLORMAP);

    if (viewplayer->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS))
    {
        for (int i = 0; i < numsectors; i++)
            AM_FillSector(&sectors[i]);

        return;
    }

    for (int i = 0; i < numsectors; i++)
    {
        const sector_t  *sector = &sectors[i];

        if (sector->mapped)
            AM_FillSector(sector);
    }
}

static void AM_ActivateNewScale(void)
{
    m_x += m_w / 2;
    m_y += m_h / 2;
    m_w = FTOM(MAPWIDTH);
    m_h = FTOM(MAPHEIGHT);
    m_x -= m_w / 2;
    m_y -= m_h / 2;
    putbigwalldot = (scale_mtof >= USEBIGDOTS || r_detail == r_detail_low ? &PUTBIGDOT : &PUTDOT);
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
    putbigwalldot = (scale_mtof >= USEBIGDOTS || r_detail == r_detail_low ? &PUTBIGDOT : &PUTDOT);
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

    min_scale_mtof = MIN(scale_mtof, MIN(a, b));
    max_scale_mtof = FixedDiv(MAPHEIGHT << FRACBITS, PLAYERRADIUS * 3);
}

static void AM_ChangeWindowLoc(void)
{
    fixed_t         incx = m_paninc.x;
    fixed_t         incy = m_paninc.y;
    const fixed_t   width = m_w / 2;
    const fixed_t   height = m_h / 2;

    if (am_rotatemode)
        AM_Rotate(&incx, &incy, (viewangle - ANG90) >> ANGLETOFINESHIFT);

    if ((m_x += incx) + width < min_x)
        m_x = min_x - width;
    else if (m_x + width > max_x)
        m_x = max_x - width;

    if ((m_y += incy) + height < min_y)
        m_y = min_y - height;
    else if (m_y > max_y)
        m_y = max_y - height;
}

void AM_SetColors(void)
{
    byte        priority[256] = { 0 };
    static byte priorities[256 * 256];

    priority[nearestcolors[am_wallcolor]] = WALLPRIORITY;
    priority[nearestcolors[am_bluedoorcolor]] = DOORPRIORITY;
    priority[nearestcolors[am_reddoorcolor]] = DOORPRIORITY;
    priority[nearestcolors[am_yellowdoorcolor]] = DOORPRIORITY;

    if (am_secretcolor != am_secretcolor_none)
        priority[nearestcolors[am_secretcolor]] = SECRETPRIORITY;

    priority[nearestcolors[am_cdwallcolor]] = CDWALLPRIORITY;
    priority[nearestcolors[am_fdwallcolor]] = FDWALLPRIORITY;
    priority[nearestcolors[am_teleportercolor]] = TELEPORTERPRIORITY;
    priority[nearestcolors[am_tswallcolor]] = TSWALLPRIORITY;
    priority[nearestcolors[am_allmapwallcolor]] = ALLMAPWALLPRIORITY;
    priority[nearestcolors[am_allmapcdwallcolor]] = ALLMAPCDWALLPRIORITY;
    priority[nearestcolors[am_allmapfdwallcolor]] = ALLMAPFDWALLPRIORITY;
    playercolor = nearestcolors[am_playercolor];
    thingcolor = nearestcolors[am_thingcolor];
    bloodsplatcolor = nearestcolors[am_bloodsplatcolor];
    corpsecolor = nearestcolors[am_corpsecolor];
    bluekeycolor = nearestcolors[am_bluekeycolor];
    redkeycolor = nearestcolors[am_redkeycolor];
    yellowkeycolor = nearestcolors[am_yellowkeycolor];
    markcolor = nearestcolors[am_markcolor];
    backcolor = nearestcolors[am_backcolor];
    pathcolor = nearestcolors[am_pathcolor];
    gridcolor = nearestcolors[am_gridcolor];

    for (mobjtype_t i = 0; i < nummobjtypes; i++)
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
            priorities[(x << 8) + y] = (byte)(priority[x] > priority[y] ? x : y);

    wallcolor = &priorities[nearestcolors[am_wallcolor] << 8];
    bluedoorcolor = &priorities[nearestcolors[am_bluedoorcolor] << 8];
    reddoorcolor = &priorities[nearestcolors[am_reddoorcolor] << 8];
    yellowdoorcolor = &priorities[nearestcolors[am_yellowdoorcolor] << 8];

    if (am_secretcolor != am_secretcolor_none)
        secretcolor = &priorities[nearestcolors[am_secretcolor] << 8];

    allmapwallcolor = &priorities[nearestcolors[am_allmapwallcolor] << 8];
    cdwallcolor = &priorities[nearestcolors[am_cdwallcolor] << 8];
    allmapcdwallcolor = &priorities[nearestcolors[am_allmapcdwallcolor] << 8];
    fdwallcolor = &priorities[nearestcolors[am_fdwallcolor] << 8];
    allmapfdwallcolor = &priorities[nearestcolors[am_allmapfdwallcolor] << 8];
    teleportercolor = &priorities[nearestcolors[am_teleportercolor] << 8];
    tswallcolor = &priorities[nearestcolors[am_tswallcolor] << 8];
}

void AM_GetGridSize(void)
{
    int width = -1;
    int height = -1;

    if (sscanf(am_gridsize, "%10dx%10d", &width, &height) == 2
        && width >= am_gridsize_width_min && width <= am_gridsize_width_max
        && height >= am_gridsize_height_min && height <= am_gridsize_height_max)
    {
        gridwidth = width << MAPBITS;
        gridheight = height << MAPBITS;
    }
    else
    {
        gridwidth = am_gridsize_width_default << MAPBITS;
        gridheight = am_gridsize_width_default << MAPBITS;
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

void AM_SetAutomapSize(const int screensize)
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

    zoomvelocity = 0.0f;
    zoomsign = 0;

    panvel_x = 0;
    panvel_y = 0;
    pansign_x = 0;
    pansign_y = 0;

    controllerpan_x = 0.0f;
    controllerpan_y = 0.0f;

    mousedragging = false;

    m_w = FTOM(MAPWIDTH);
    m_h = FTOM(MAPHEIGHT);
}

static void AM_LevelInit(void)
{
    am_followmode = am_followmode_default;
    bigstate = false;

    scale_mtof = FixedDiv(INITSCALEMTOF, FRACUNIT * 7 / 10);
    AM_FindMinMaxBoundaries();

    if (scale_mtof > max_scale_mtof)
        scale_mtof = min_scale_mtof;

    putbigwalldot = (scale_mtof >= USEBIGDOTS ? &PUTBIGDOT : &PUTDOT);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    if (floorpiccolor)
    {
        free(floorpiccolor);
        floorpiccolor = NULL;
    }

    if (edges)
    {
        free(edges);
        edges = NULL;
        edgescapacity = 0;
    }

    AM_BuildFloorPicColors();

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

void AM_InitPixelSize(void)
{
    if (r_detail == r_detail_high)
    {
        putbigdot = &PUTDOT;
        putbigdot2 = &PUTDOT2;
        putbigwalldot = (scale_mtof >= USEBIGDOTS ? &PUTBIGDOT : &PUTDOT);
    }
    else
    {
        putbigdot = &PUTBIGDOT;
        putbigdot2 = &PUTBIGDOT2;
        putbigwalldot = &PUTBIGDOT;
    }
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

    AM_InitPixelSize();
    AM_InitVariables(mainwindow);
    HU_ClearMessages();
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
    return ((gamekeydown[keyboardrun] || gamekeydown[keyboardrun2] || mousebuttons[mouserun]
        || (controllerbuttons & controllerrun)) ^ alwaysrun);
}

void AM_ToggleZoomOut(void)
{
    speedtoggle = AM_GetSpeedToggle();
    mtof_zoommul = M_ZOOMOUT;
    ftom_zoommul = M_ZOOMIN;
}

void AM_ToggleZoomIn(void)
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

void AM_ToggleFollowMode(const bool value)
{
    if ((am_followmode = value))
    {
        m_paninc.x = 0;
        m_paninc.y = 0;
        C_StringCVAROutput(stringize(am_followmode), "on");
        C_Output(s_AMSTR_FOLLOWON);
        HU_SetPlayerMessage(s_AMSTR_FOLLOWON, false, true);
    }
    else
    {
        C_StringCVAROutput(stringize(am_followmode), "off");
        C_Output(s_AMSTR_FOLLOWOFF);
        HU_SetPlayerMessage(s_AMSTR_FOLLOWOFF, false, true);
    }

    message_dontfuckwithme = true;
}

void AM_ToggleGrid(void)
{
    if ((am_grid = !am_grid))
    {
        C_StringCVAROutput(stringize(am_grid), "on");
        C_Output(s_AMSTR_GRIDON);
        HU_SetPlayerMessage(s_AMSTR_GRIDON, false, true);
    }
    else
    {
        C_StringCVAROutput(stringize(am_grid), "off");
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

    for (int i = 0; i < nummarks; i++)
        if (mark[i].x == x && mark[i].y == y)
            return;

    if (nummarks >= maxmarks)
    {
        maxmarks = (maxmarks ? maxmarks * 2 : 16);
        mark = I_Realloc(mark, maxmarks * sizeof(*mark));
    }

    mark[nummarks].x = x;
    mark[nummarks].y = y;

    M_snprintf(message, sizeof(message), s_AMSTR_MARKEDSPOT,
        ++nummarks, x >> MAPBITS, y >> MAPBITS);
    C_Output(message);
    HU_SetPlayerMessage(message, false, true);

    message_dontfuckwithme = true;
}

void AM_ClearMarks(void)
{
    if (nummarks)
    {
        if (++markpress == 5)
        {
            // clear all marks
            C_Output(s_AMSTR_MARKSCLEARED);
            HU_SetPlayerMessage(s_AMSTR_MARKSCLEARED, false, true);
            nummarks = 0;
            maxmarks = 0;
            mark = NULL;
        }
        else if (markpress == 1)
        {
            char    message[32];

            // clear one mark
            M_snprintf(message, sizeof(message), s_AMSTR_MARKCLEARED, nummarks--);
            C_Output(message);
            HU_SetPlayerMessage(message, false, true);
        }

        message_dontfuckwithme = true;
    }
}

void AM_DropBreadCrumb(void)
{
    if (numbreadcrumbs >= maxbreadcrumbs)
    {
        const int   MAXBREADCRUMBS = INT_MAX / (int)sizeof(*breadcrumb);
        int         newmax = (maxbreadcrumbs ? maxbreadcrumbs * 2 : 16);

        if (maxbreadcrumbs > MAXBREADCRUMBS / 2)
            return;

        if (newmax > MAXBREADCRUMBS)
            newmax = MAXBREADCRUMBS;

        breadcrumb = I_Realloc(breadcrumb, newmax * sizeof(*breadcrumb));
        maxbreadcrumbs = newmax;
    }

    if (numbreadcrumbs < 0 || numbreadcrumbs >= maxbreadcrumbs)
        return;

    breadcrumb[numbreadcrumbs].x = viewx;
    breadcrumb[numbreadcrumbs++].y = viewy;
}

void AM_TogglePath(const bool value)
{
    if ((am_path = value))
    {
        C_StringCVAROutput(stringize(am_path), "on");
        C_Output(s_AMSTR_PATHON);
        HU_SetPlayerMessage(s_AMSTR_PATHON, false, true);
    }
    else
    {
        C_StringCVAROutput(stringize(am_path), "off");
        C_Output(s_AMSTR_PATHOFF);
        HU_SetPlayerMessage(s_AMSTR_PATHOFF, false, true);
    }

    message_dontfuckwithme = true;
    M_SaveCVARs();
}

void AM_ToggleRotateMode(const bool value)
{
    if ((am_rotatemode = value))
    {
        C_StringCVAROutput(stringize(am_rotatemode), "on");
        C_Output(s_AMSTR_ROTATEON);
        HU_SetPlayerMessage(s_AMSTR_ROTATEON, false, true);
    }
    else
    {
        C_StringCVAROutput(stringize(am_rotatemode), "off");
        C_Output(s_AMSTR_ROTATEOFF);
        HU_SetPlayerMessage(s_AMSTR_ROTATEOFF, false, true);
    }

    message_dontfuckwithme = true;
    M_SaveCVARs();
}

static inline void AM_SetPanX(const int dir)
{
    pansign_x = SIGN(dir);
}

static inline void AM_SetPanY(const int dir)
{
    pansign_y = SIGN(dir);
}

static void AM_UpdatePanFromKeys(void)
{
    if (am_followmode)
    {
        pansign_x = 0;
        pansign_y = 0;
        return;
    }

    if (keystate(keyboardright) || keystate(keyboardright2)
        || keystate(keyboardstraferight) || keystate(keyboardstraferight2))
        pansign_x = 1;
    else if (keystate(keyboardleft) || keystate(keyboardleft2)
        || keystate(keyboardstrafeleft) || keystate(keyboardstrafeleft2))
        pansign_x = -1;
    else
        pansign_x = 0;

    if (keystate(keyboardforward) || keystate(keyboardforward2))
        pansign_y = 1;
    else if (keystate(keyboardback) || keystate(keyboardback2))
        pansign_y = -1;
    else
        pansign_y = 0;
}

static inline fixed_t AM_PixelsToMap(const int pixels)
{
    return FixedMul((fixed_t)(pixels << FRACBITS), scale_ftom);
}

static void AM_ApplyKeyboardPan(void)
{
    const fixed_t   accel = AM_PixelsToMap(MAX(1, F_PANINC / 2));
    const fixed_t   maxvel = AM_PixelsToMap(F_PANINC * 3);

    if (pansign_x > 0)
        panvel_x = MIN(panvel_x + accel, maxvel);
    else if (pansign_x < 0)
        panvel_x = MAX(panvel_x - accel, -maxvel);
    else
        panvel_x = FixedMul(panvel_x, (fixed_t)(FRACUNIT * 0.75f));

    if (pansign_y > 0)
        panvel_y = MIN(panvel_y + accel, maxvel);
    else if (pansign_y < 0)
        panvel_y = MAX(panvel_y - accel, -maxvel);
    else
        panvel_y = FixedMul(panvel_y, (fixed_t)(FRACUNIT * 0.75f));

    if (ABS(panvel_x) < AM_PixelsToMap(1))
        panvel_x = 0;

    if (ABS(panvel_y) < AM_PixelsToMap(1))
        panvel_y = 0;

    m_paninc.x = panvel_x;
    m_paninc.y = panvel_y;
}

static void AM_ApplyControllerPan(void)
{
    const fixed_t   maxvel = AM_PixelsToMap(F_PANINC * 3);
    const float     smooth = 0.35f;
    const fixed_t   target_x = (fixed_t)llround((double)maxvel * controllerpan_x);
    const fixed_t   target_y = (fixed_t)llround((double)maxvel * controllerpan_y);

    panvel_x += (fixed_t)llround((double)(target_x - panvel_x) * smooth);
    panvel_y += (fixed_t)llround((double)(target_y - panvel_y) * smooth);

    if (ABS(panvel_x) < AM_PixelsToMap(1))
        panvel_x = 0;

    if (ABS(panvel_y) < AM_PixelsToMap(1))
        panvel_y = 0;

    m_paninc.x = panvel_x;
    m_paninc.y = panvel_y;
}

static void AM_AddZoomImpulse(const int direction)
{
    zoomsign = SIGN(direction);
    zoomvelocity += (AM_GetSpeedToggle() ? 1.25f : 0.85f);

    if (zoomvelocity > 6.0f)
        zoomvelocity = 6.0f;
}

static void AM_ApplyZoom(void)
{
    float expv;

    if (!zoomsign || zoomvelocity <= 0.001f)
        return;

    expv = (zoomsign > 0 ? 0.050f : -0.050f) * zoomvelocity;
    mtof_zoommul = (fixed_t)llround((double)FRACUNIT * expf(expv));
    ftom_zoommul = (fixed_t)llround((double)FRACUNIT / expf(expv));

    AM_ChangeWindowScale();
    mtof_zoommul = FRACUNIT;
    ftom_zoommul = FRACUNIT;

    zoomvelocity *= 0.80f;

    if (zoomvelocity < 0.01f)
    {
        zoomvelocity = 0.0f;
        zoomsign = 0;
    }
}

//
// Handle events (user inputs) in automap mode
//
bool AM_Responder(const event_t *ev)
{
    bool    result = false;

    am_frame.angle = 0;
    modstate = SDL_GetModState();

    if (!menuactive && !paused)
    {
        static bool backbuttondown;

        if (!(controllerbuttons & controllerautomap))
            backbuttondown = false;

        if (!automapactive && !mapwindow)
        {
            if ((ev->type == ev_keydown
                && (ev->data1 == keyboardautomap
                    || ev->data1 == keyboardautomap2)
                && keydown != keyboardautomap
                && (!keyboardautomap2 || keydown != keyboardautomap2)
                && !(modstate & KMOD_ALT))
                || (ev->type == ev_mouse
                    && mouseautomap >= 0
                    && (ev->data1 & mouseautomap))
                || (ev->type == ev_controller
                    && (controllerbuttons & controllerautomap)
                    && !backbuttondown))
            {
                keydown = ev->data1;
                backbuttondown = true;
                AM_Start(true);
                viewactive = false;
                result = true;
            }
        }
        else
        {
            int key;

            if (ev->type == ev_keydown)
            {
                result = true;
                key = ev->data1;

                // pan right
                if (key == keyboardright
                    || key == keyboardright2
                    || key == keyboardstraferight
                    || key == keyboardstraferight2)
                {
                    keydown = key;

                    if (am_followmode)
                    {
                        m_paninc.x = 0;
                        result = false;
                    }
                    else
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        AM_SetPanX(1);
                    }
                }

                // pan left
                else if (key == keyboardleft
                    || key == keyboardleft2
                    || key == keyboardstrafeleft
                    || key == keyboardstrafeleft2)
                {
                    keydown = key;

                    if (am_followmode)
                    {
                        m_paninc.x = 0;
                        result = false;
                    }
                    else
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        AM_SetPanX(-1);
                    }
                }

                // pan up
                else if (key == keyboardforward || key == keyboardforward2)
                {
                    keydown = key;

                    if (am_followmode)
                    {
                        m_paninc.y = 0;
                        result = false;
                    }
                    else
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        AM_SetPanY(1);
                    }
                }

                // pan down
                else if (key == keyboardback || key == keyboardback2)
                {
                    keydown = key;

                    if (am_followmode)
                    {
                        m_paninc.y = 0;
                        result = false;
                    }
                    else
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        AM_SetPanY(-1);
                    }
                }

                // zoom out
                else if (key == keyboardzoomout && !movement
                    && (!mapwindow || keyboardzoomout != '-'))
                {
                    keydown = key;
                    speedtoggle = AM_GetSpeedToggle();
                    AM_AddZoomImpulse(-1);
                }
                else if (key == keyboardzoomout2 && !movement
                    && (!mapwindow || keyboardzoomout2 != '-'))
                {
                    keydown = key;
                    speedtoggle = AM_GetSpeedToggle();
                    AM_AddZoomImpulse(-1);
                }

                // zoom in
                else if (key == keyboardzoomin && !movement
                    && (!mapwindow || keyboardzoomin != '='))
                {
                    keydown = key;
                    speedtoggle = AM_GetSpeedToggle();
                    AM_AddZoomImpulse(1);
                    bigstate = false;
                }
                else if (key == keyboardzoomin2 && !movement
                    && (!mapwindow || keyboardzoomin2 != '='))
                {
                    keydown = key;
                    speedtoggle = AM_GetSpeedToggle();
                    AM_AddZoomImpulse(1);
                    bigstate = false;
                }

                // leave automap
                else if ((key == keyboardautomap || key == keyboardautomap2) && !(modstate & KMOD_ALT)
                    && keydown != keyboardautomap && (!keyboardautomap2 || keydown != keyboardautomap2) && !mapwindow)
                {
                    keydown = key;
                    viewactive = true;
                    AM_Stop();
                }

                // toggle maximum zoom
                else if ((key == keyboardmaxzoom || key == keyboardmaxzoom2) && !idclev && !idmus)
                {
                    if (keydown != keyboardmaxzoom && (!keyboardmaxzoom2 || keydown != keyboardmaxzoom2))
                    {
                        keydown = key;
                        AM_ToggleMaxZoom();
                    }
                }

                // toggle follow mode
                else if (key == keyboardfollowmode || key == keyboardfollowmode2)
                {
                    if (keydown != keyboardfollowmode && (!keyboardfollowmode2 || keydown != keyboardfollowmode2))
                    {
                        keydown = key;
                        AM_ToggleFollowMode(!am_followmode);
                    }
                }

                // toggle grid
                else if (key == keyboardgrid || key == keyboardgrid2)
                {
                    if (keydown != keyboardgrid && (!keyboardgrid2 || keydown != keyboardgrid2))
                    {
                        keydown = key;
                        AM_ToggleGrid();
                    }
                }

                // mark spot
                else if (key == keyboardmark || key == keyboardmark2)
                {
                    if (keydown != keyboardmark && (!keyboardmark2 || keydown != keyboardmark2))
                    {
                        keydown = key;
                        AM_AddMark();
                    }
                }

                // clear mark(s)
                else if (key == keyboardclearmark || key == keyboardclearmark2)
                    AM_ClearMarks();

                // toggle path
                else if (key == keyboardpath || key == keyboardpath2)
                {
                    if (keydown != keyboardpath && (!keyboardpath2 || keydown != keyboardpath2))
                    {
                        keydown = key;
                        AM_TogglePath(!am_path);
                    }
                }

                // toggle rotate mode
                else if (key == keyboardrotatemode || key == keyboardrotatemode2)
                {
                    if (keydown != keyboardrotatemode && (!keyboardrotatemode2 || keydown != keyboardrotatemode2))
                    {
                        keydown = key;
                        AM_ToggleRotateMode(!am_rotatemode);
                    }
                }
                else
                    result = false;
            }
            else if (ev->type == ev_keyup)
            {
                key = ev->data1;

                if (key == keyboardclearmark || key == keyboardclearmark2)
                    markpress = 0;

                if ((key == keyboardzoomout || key == keyboardzoomout2 || key == keyboardzoomin || key == keyboardzoomin2) && !movement)
                {
                    mtof_zoommul = FRACUNIT;
                    ftom_zoommul = FRACUNIT;
                }
                else if (key == keyboardfollowmode || key == keyboardfollowmode2)
                {
                    int key2 = 0;

                    if (keystate(keyboardleft))
                        key2 = keyboardleft;
                    else if (keystate(keyboardleft2))
                        key2 = keyboardleft2;
                    else if (keystate(keyboardstrafeleft))
                        key2 = keyboardstrafeleft;
                    else if (keystate(keyboardstrafeleft2))
                        key2 = keyboardstrafeleft2;
                    else if (keystate(keyboardright))
                        key2 = keyboardright;
                    else if (keystate(keyboardright2))
                        key2 = keyboardright2;
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
                        event_t temp = { ev_keydown, key2, 0, 0 };

                        D_PostEvent(&temp);
                    }
                }
                else if (!am_followmode)
                {
                    if (key == keyboardleft
                        || key == keyboardleft2
                        || key == keyboardstrafeleft
                        || key == keyboardstrafeleft2
                        || key == keyboardright
                        || key == keyboardright2
                        || key == keyboardstraferight
                        || key == keyboardstraferight2
                        || key == keyboardforward
                        || key == keyboardforward2
                        || key == keyboardback
                        || key == keyboardback2)
                    {
                        speedtoggle = AM_GetSpeedToggle();
                        AM_UpdatePanFromKeys();
                    }
                }
            }
            else if (ev->type == ev_mouse && mousewait < I_GetTime())
            {
                if (am_mousepanning && m_pointer && !am_followmode
                    && !mapwindow && !consoleactive)
                {
                    const bool  lmb = !!(ev->data1 & MOUSE_LEFTBUTTON);

                    if (lmb && (ev->data2 || ev->data3))
                    {
                        int dx = ev->data2;
                        int dy = ev->data3;

                        if (ev->data5 >= MAPHEIGHT / 2)
                            return false;

                        if (am_correctaspectratio)
                            dy = dy * 6 / 5;

                        m_paninc.x = -AM_PixelsToMap(dx);
                        m_paninc.y = AM_PixelsToMap(dy);

                        panvel_x = 0;
                        panvel_y = 0;
                        pansign_x = 0;
                        pansign_y = 0;

                        movement = false;
                        mousedragging = true;
                    }
                    else if (!lmb && mousedragging)
                    {
                        m_paninc.x = 0;
                        m_paninc.y = 0;
                        panvel_x = 0;
                        panvel_y = 0;

                        mousedragging = false;
                    }
                }

                if (mouseclearmark >= 0 && (ev->data1 & mouseclearmark))
                {
                    mousewait = I_GetTime() + 8;
                    AM_ClearMarks();
                }

                if (mousefollowmode >= 0 && (ev->data1 & mousefollowmode))
                {
                    mousewait = I_GetTime() + 8;
                    AM_ToggleFollowMode(!am_followmode);
                }

                if (mousegrid >= 0 && (ev->data1 & mousegrid))
                {
                    mousewait = I_GetTime() + 8;
                    AM_ToggleGrid();
                }

                if (mousemark >= 0 && (ev->data1 & mousemark))
                {
                    mousewait = I_GetTime() + 8;
                    AM_AddMark();
                }

                if (mousemaxzoom >= 0 && (ev->data1 & mousemaxzoom))
                {
                    mousewait = I_GetTime() + 8;
                    AM_ToggleMaxZoom();
                }

                if (mousepath >= 0 && (ev->data1 & mousepath))
                {
                    mousewait = I_GetTime() + 8;
                    AM_TogglePath(!am_path);
                }

                if (mouserotatemode >= 0 && (ev->data1 & mouserotatemode))
                {
                    mousewait = I_GetTime() + 8;
                    AM_ToggleRotateMode(!am_rotatemode);
                }

                if (mousezoomin >= 0 && (ev->data1 & mousezoomin))
                {
                    mousewait = I_GetTime() + 8;
                    AM_ToggleZoomIn();
                }

                if (mousezoomout >= 0 && (ev->data1 & mousezoomout))
                {
                    mousewait = I_GetTime() + 8;
                    AM_ToggleZoomOut();
                }
            }
            else if (ev->type == ev_mousewheel && !mapwindow)
            {
                // zoom in
                if (ev->data1 > 0)
                {
                    speedtoggle = AM_GetSpeedToggle();
                    AM_AddZoomImpulse(1);
                    bigstate = false;
                }

                // zoom out
                else if (ev->data1 < 0)
                {
                    speedtoggle = AM_GetSpeedToggle();
                    AM_AddZoomImpulse(-1);
                }
            }
            else if (ev->type == ev_controller && controllerwait < I_GetTime())
            {
                if ((controllerbuttons & controllerautomap) && !backbuttondown)
                {
                    controllerwait = I_GetTime() + 8;
                    viewactive = true;
                    backbuttondown = true;
                    AM_Stop();
                }

                // zoom out
                else if ((controllerbuttons & controllerzoomout)
                    && !(controllerbuttons & controllerzoomin))
                {
                    speedtoggle = AM_GetSpeedToggle();
                    AM_AddZoomImpulse(-1);
                }

                // zoom in
                else if ((controllerbuttons & controllerzoomin)
                    && !(controllerbuttons & controllerzoomout))
                {
                    speedtoggle = AM_GetSpeedToggle();
                    AM_AddZoomImpulse(1);
                    bigstate = false;
                }

                // toggle maximum zoom
                else if ((controllerbuttons & controllermaxzoom) && !idclev && !idmus)
                {
                    AM_ToggleMaxZoom();
                    controllerwait = I_GetTime() + 12;
                }

                // toggle follow mode
                else if (controllerbuttons & controllerfollowmode)
                {
                    AM_ToggleFollowMode(!am_followmode);
                    controllerwait = I_GetTime() + 12;
                }

                // toggle grid
                else if (controllerbuttons & controllergrid)
                {
                    AM_ToggleGrid();
                    controllerwait = I_GetTime() + 12;
                }

                // mark spot
                else if (controllerbuttons & controllermark)
                {
                    AM_AddMark();
                    controllerwait = I_GetTime() + 12;
                }

                // clear mark(s)
                else if (controllerbuttons & controllerclearmark)
                {
                    AM_ClearMarks();
                    controllerwait = I_GetTime() + 12;
                }

                // toggle path
                else if (controllerbuttons & controllerpath)
                {
                    AM_TogglePath(!am_path);
                    controllerwait = I_GetTime() + 12;
                }

                // toggle rotate mode
                else if (controllerbuttons & controllerrotatemode)
                {
                    AM_ToggleRotateMode(!am_rotatemode);
                    controllerwait = I_GetTime() + 12;
                }

                if (!am_followmode)
                {
                    controllerpan_x = ((float)controllerthumbLX / SHRT_MAX) * 1.2f;
                    controllerpan_y = (-(float)controllerthumbLY / SHRT_MAX) * 1.2f;
                    controllerpan_x = BETWEENF(-1.0f, controllerpan_x, 1.0f);
                    controllerpan_y = BETWEENF(-1.0f, controllerpan_y, 1.0f);
                    movement = (controllerpan_x != 0.0f || controllerpan_y != 0.0f);
                    speedtoggle = AM_GetSpeedToggle();
                }
            }

            if ((viewplayer->cheats & CF_MYPOS) && !am_followmode && (m_paninc.x || m_paninc.y))
            {
                double  x = m_paninc.x;
                double  y = m_paninc.y;

                if ((m_x == min_x - m_w / 2 && x < 0.0) || (m_x == max_x - m_w / 2 && x > 0.0))
                    x = 0.0;

                if ((m_y == min_y - m_h / 2 && y < 0.0) || (m_y == max_y - m_h / 2 && y > 0.0))
                    y = 0.0;

                if ((am_frame.angle = (int)(atan2(y, x) * 180.0 / M_PI)) < 0)
                    am_frame.angle += 360;
            }
        }
    }

    return result;
}

//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
static void AM_Rotate(fixed_t *x, fixed_t *y, const angle_t angle)
{
    const double    radians = (double)angle * (2.0 * M_PI) / FINEANGLES;
    const double    cosine = cos(radians);
    const double    sine = sin(radians);
    const double    xx = (double)*x;
    const double    yy = (double)*y;

    *x = (fixed_t)llround(xx * cosine - yy * sine);
    *y = (fixed_t)llround(xx * sine + yy * cosine);
}

static void AM_RotatePoint(mpoint_t *point)
{
    const double    dx = (double)(point->x - am_frame.center.x);
    const double    dy = (double)(point->y - am_frame.center.y);
    const double    cosine = am_frame.cos;
    const double    sine = am_frame.sin;

    point->x = am_frame.center.x + (fixed_t)llround(dx * cosine - dy * sine);
    point->y = am_frame.center.y + (fixed_t)llround(dx * sine + dy * cosine);
}

static void AM_CorrectAspectRatio(mpoint_t *point)
{
    point->y = am_frame.center.y + (point->y - am_frame.center.y) * 5 / 6;
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
    const mobj_t    *mo = viewplayer->mo;

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

    if (!consoleactive && !paused)
        AM_ApplyZoom();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
        AM_ChangeWindowScale();

    // Change x,y location
    if (!consoleactive && !paused)
    {
        if (!mousedragging)
        {
            if (movement)
                AM_ApplyControllerPan();
            else
                AM_ApplyKeyboardPan();
        }

        if (m_paninc.x || m_paninc.y)
        {
            AM_ChangeWindowLoc();

            if (mousedragging)
            {
                m_paninc.x = 0;
                m_paninc.y = 0;
            }
        }
    }

    movement = false;
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
    *y0 = CYMTOF(*y0);
    *y1 = CYMTOF(*y1);

    if (!((*x0 - *x1) | (*y0 - *y1)))
        return false;

    if ((unsigned int)(*x0) < (unsigned int)MAPWIDTH
        && (unsigned int)(*x1) < (unsigned int)MAPWIDTH
        && (unsigned int)(*y0) < (unsigned int)MAPHEIGHT
        && (unsigned int)(*y1) < (unsigned int)MAPHEIGHT)
        return true;

    if (*x0 < -1)
        outcode1 = LEFT;
    else if (*x0 >= MAPWIDTH)
        outcode1 = RIGHT;

    if (*x1 < -1)
        outcode2 = LEFT;
    else if (*x1 >= MAPWIDTH)
        outcode2 = RIGHT;

    if (outcode1 & outcode2)
        return false;

    if (*y0 < -1)
        outcode1 |= TOP;
    else if (*y0 >= MAPHEIGHT)
        outcode1 |= BOTTOM;

    if (*y1 < -1)
        outcode2 |= TOP;
    else if (*y1 >= MAPHEIGHT)
        outcode2 |= BOTTOM;

    return !(outcode1 & outcode2);
}

static inline void PUTDOT(int x, int y, const byte *color)
{
    if (x >= 0 && x < MAPWIDTH && y >= 0 && y < MAPAREA)
    {
        byte    *dot = mapscreen + y + x;

        *dot = *(*dot + color);
    }
}

static inline void PUTDOT2(int x, int y, const byte *color)
{
    if (x >= 0 && x < MAPWIDTH && y >= 0 && y < MAPAREA)
        *(mapscreen + y + x) = *color;
}

static inline void PUTBIGDOT(int x, int y, const byte *color)
{
    if (x >= 0)
    {
        PUTDOT(x, y, color);

        if (y < MAPBOTTOM)
        {
            PUTDOT(x, y + MAPWIDTH, color);

            if (++x < MAPWIDTH)
            {
                PUTDOT(x, y, color);
                PUTDOT(x, y + MAPWIDTH, color);
            }
        }
        else if (++x < MAPWIDTH)
            PUTDOT(x, y, color);
    }
    else if (++x < MAPWIDTH)
    {
        PUTDOT(x, y, color);

        if (y < MAPBOTTOM)
            PUTDOT(x, y + MAPWIDTH, color);
    }
}

static inline void PUTBIGDOT2(int x, int y, const byte *color)
{
    if (x >= 0 && x < MAPWIDTH - 1 && y >= 0 && y < MAPAREA)
    {
        byte    *dot = mapscreen + y + x;

        *dot = *color;
        *(dot + 1) = *color;
        *(dot + MAPWIDTH) = *color;
        *(dot + MAPWIDTH + 1) = *color;
    }
}

static inline void PUTTRANSLUCENTDOT(int x, int y, const byte *color)
{
    if (x >= 0 && x < MAPWIDTH && y >= 0 && y < MAPAREA)
    {
        byte    *dot = mapscreen + y + x;

        if (*dot != tinttab50[*color])
            *dot = tinttab50[(*dot << 8) + *color];
    }
}

//
// Classic Bresenham w/ whatever optimizations needed for speed
//
static void AM_DrawFline(int x0, int y0, int x1, int y1, const byte *color,
    void (*putdot)(int, int, const byte *))
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

            putdot(x0, (y0 *= MAPWIDTH), color);

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

//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
static void AM_DrawGrid(void)
{
    const fixed_t   minlen = (fixed_t)sqrt((double)m_w * m_w + (double)m_h * m_h);
    fixed_t         startx = m_x - (minlen - m_w) / 2;
    fixed_t         starty = m_y - (minlen - m_h) / 2;
    fixed_t         endx = startx + minlen;
    fixed_t         endy = starty + minlen;

    if (am_rotatemode)
    {
        startx -= m_h / 2;
        starty -= m_w / 2;
        endx += m_h / 2;
        endy += m_w / 2;
    }

    // Draw vertical gridlines
    for (fixed_t x = startx - ((startx - (bmaporgx >> FRACTOMAPBITS)) % gridwidth); x < endx; x += gridwidth)
    {
        mpoint_t    a = { x, starty };
        mpoint_t    b = { x, endy };

        if (am_rotatemode)
        {
            AM_RotatePoint(&a);
            AM_RotatePoint(&b);
        }

        if (am_correctaspectratio)
        {
            AM_CorrectAspectRatio(&a);
            AM_CorrectAspectRatio(&b);
        }

        AM_DrawFline(a.x, a.y, b.x, b.y, &gridcolor, putbigdot2);
    }

    // Draw horizontal gridlines
    for (fixed_t y = starty - ((starty - (bmaporgy >> FRACTOMAPBITS)) % gridheight); y < endy; y += gridheight)
    {
        mpoint_t    a = { startx, y };
        mpoint_t    b = { endx,   y };

        if (am_rotatemode)
        {
            AM_RotatePoint(&a);
            AM_RotatePoint(&b);
        }

        if (am_correctaspectratio)
        {
            AM_CorrectAspectRatio(&a);
            AM_CorrectAspectRatio(&b);
        }

        AM_DrawFline(a.x, a.y, b.x, b.y, &gridcolor, putbigdot2);
    }
}

static byte *AM_DoorColor(unsigned short special)
{
    if (special >= GenLockedBase && special < GenDoorBase)
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

static void AM_DrawSides(const line_t *line, const byte *color)
{
    mpoint_t    a, b;
    fixed_t     dx, dy;
    fixed_t     mx, my;
    fixed_t     nx, ny;
    double      len;
    mpoint_t    p0, p1;

    if (!am_sides)
        return;

    a.x = line->v1->x >> FRACTOMAPBITS;
    a.y = line->v1->y >> FRACTOMAPBITS;
    b.x = line->v2->x >> FRACTOMAPBITS;
    b.y = line->v2->y >> FRACTOMAPBITS;

    dx = b.x - a.x;
    dy = b.y - a.y;

    if (!dx && !dy)
        return;

    if ((len = sqrt((double)dx * (double)dx + (double)dy * (double)dy)) <= 0.0001)
        return;

    mx = (a.x + b.x) / 2;
    my = (a.y + b.y) / 2;

    nx = (fixed_t)llround((double)(-dy) * (double)SIDETICKLENGTH / len);
    ny = (fixed_t)llround((double)(dx) * (double)SIDETICKLENGTH / len);

    p0.x = mx;
    p0.y = my;
    p1.x = mx - nx;
    p1.y = my - ny;

    if (am_rotatemode)
    {
        AM_RotatePoint(&p0);
        AM_RotatePoint(&p1);
    }

    if (am_correctaspectratio)
    {
        AM_CorrectAspectRatio(&p0);
        AM_CorrectAspectRatio(&p1);
    }

    AM_DrawFline(p0.x, p0.y, p1.x, p1.y, color, putbigdot2);

    if (line->flags & ML_TWOSIDED)
    {
        p0.x = mx;
        p0.y = my;
        p1.x = mx + nx;
        p1.y = my + ny;

        if (am_rotatemode)
        {
            AM_RotatePoint(&p0);
            AM_RotatePoint(&p1);
        }

        if (am_correctaspectratio)
        {
            AM_CorrectAspectRatio(&p0);
            AM_CorrectAspectRatio(&p1);
        }

        AM_DrawFline(p0.x, p0.y, p1.x, p1.y, color, putbigdot2);
    }
}

static void AM_DrawWalls(void)
{
    for (int i = 0; i < numlines; i++)
    {
        const line_t            *line = &lines[i];
        const unsigned short    flags = line->flags;

        if ((flags & ML_MAPPED) && !(flags & ML_DONTDRAW))
        {
            const fixed_t   *lbbox = line->bbox;
            const fixed_t   *ambbox = am_frame.bbox;

            if (lbbox[BOXLEFT] <= ambbox[BOXRIGHT]
                && lbbox[BOXRIGHT] >= ambbox[BOXLEFT]
                && lbbox[BOXBOTTOM] <= ambbox[BOXTOP]
                && lbbox[BOXTOP] >= ambbox[BOXBOTTOM])
            {
                mpoint_t                a = { line->v1->x >> FRACTOMAPBITS, line->v1->y >> FRACTOMAPBITS };
                mpoint_t                b = { line->v2->x >> FRACTOMAPBITS, line->v2->y >> FRACTOMAPBITS };
                const unsigned short    special = line->special;
                byte                    *doorcolor;

                if (am_rotatemode)
                {
                    AM_RotatePoint(&a);
                    AM_RotatePoint(&b);
                }

                if (am_correctaspectratio)
                {
                    AM_CorrectAspectRatio(&a);
                    AM_CorrectAspectRatio(&b);
                }

                if (special && (doorcolor = AM_DoorColor(special)) != cdwallcolor)
                {
                    AM_DrawFline(a.x, a.y, b.x, b.y, doorcolor, putbigdot);
                    AM_DrawSides(line, doorcolor);
                }
                else
                {
                    const sector_t  *back = line->backsector;

                    if (!back || (flags & ML_SECRET))
                    {
                        AM_DrawFline(a.x, a.y, b.x, b.y, wallcolor, putbigwalldot);
                        AM_DrawSides(line, wallcolor);
                    }
                    else if (isteleportline[special] && back->ceilingheight != back->floorheight
                        && ((flags & ML_TELEPORTTRIGGERED) || isteleport[back->floorpic]) && !(flags & ML_SECRET))
                    {
                        AM_DrawFline(a.x, a.y, b.x, b.y, teleportercolor, putbigdot);
                        AM_DrawSides(line, teleportercolor);
                    }
                    else
                    {
                        const sector_t  *front = line->frontsector;

                        if (back->floorheight != front->floorheight)
                        {
                            AM_DrawFline(a.x, a.y, b.x, b.y, fdwallcolor, putbigdot);
                            AM_DrawSides(line, fdwallcolor);
                        }
                        else if (back->ceilingheight != front->ceilingheight)
                        {
                            AM_DrawFline(a.x, a.y, b.x, b.y, cdwallcolor, putbigdot);
                            AM_DrawSides(line, cdwallcolor);
                        }
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
        const line_t            *line = &lines[i];
        const unsigned short    flags = line->flags;

        if (!(flags & ML_DONTDRAW))
        {
            const fixed_t   *lbbox = line->bbox;
            const fixed_t   *ambbox = am_frame.bbox;

            if (lbbox[BOXLEFT] <= ambbox[BOXRIGHT]
                && lbbox[BOXRIGHT] >= ambbox[BOXLEFT]
                && lbbox[BOXBOTTOM] <= ambbox[BOXTOP]
                && lbbox[BOXTOP] >= ambbox[BOXBOTTOM])
            {
                mpoint_t                a = { line->v1->x >> FRACTOMAPBITS, line->v1->y >> FRACTOMAPBITS };
                mpoint_t                b = { line->v2->x >> FRACTOMAPBITS, line->v2->y >> FRACTOMAPBITS };
                const unsigned short    special = line->special;
                byte                    *doorcolor;

                if (am_rotatemode)
                {
                    AM_RotatePoint(&a);
                    AM_RotatePoint(&b);
                }

                if (am_correctaspectratio)
                {
                    AM_CorrectAspectRatio(&a);
                    AM_CorrectAspectRatio(&b);
                }

                if (special && (doorcolor = AM_DoorColor(special)) != cdwallcolor)
                {
                    AM_DrawFline(a.x, a.y, b.x, b.y, doorcolor, putbigdot);
                    AM_DrawSides(line, doorcolor);
                }
                else
                {
                    const sector_t  *back = line->backsector;

                    if (!back || (flags & ML_SECRET))
                    {
                        byte    *color = ((flags & ML_MAPPED) ? wallcolor : allmapwallcolor);

                        AM_DrawFline(a.x, a.y, b.x, b.y, color, putbigwalldot);
                        AM_DrawSides(line, color);
                    }
                    else if (isteleportline[special]
                        && ((flags & ML_TELEPORTTRIGGERED) || isteleport[back->floorpic]))
                    {
                        byte    *color = ((flags & ML_MAPPED) ? teleportercolor : allmapfdwallcolor);

                        AM_DrawFline(a.x, a.y, b.x, b.y, color, putbigdot);
                        AM_DrawSides(line, color);
                    }
                    else
                    {
                        const sector_t  *front = line->frontsector;

                        if (back->floorheight != front->floorheight)
                        {
                            byte    *color = ((flags & ML_MAPPED) ? fdwallcolor : allmapfdwallcolor);

                            AM_DrawFline(a.x, a.y, b.x, b.y, color, putbigdot);
                            AM_DrawSides(line, color);
                        }
                        else if (back->ceilingheight != front->ceilingheight)
                        {
                            byte    *color = ((flags & ML_MAPPED) ? cdwallcolor : allmapcdwallcolor);

                            AM_DrawFline(a.x, a.y, b.x, b.y, color, putbigdot);
                            AM_DrawSides(line, color);
                        }
                    }
                }
            }
        }
    }
}

static void AM_DrawWalls_Cheating(void)
{
    byte    *secretcolor2 = (am_secretcolor != am_secretcolor_none && iddttics % 20 < 9 ? secretcolor : NULL);

    for (int i = 0; i < numlines; i++)
    {
        const line_t    *line = &lines[i];
        const fixed_t   *lbbox = line->bbox;
        const fixed_t   *ambbox = am_frame.bbox;

        if (lbbox[BOXLEFT] <= ambbox[BOXRIGHT]
            && lbbox[BOXRIGHT] >= ambbox[BOXLEFT]
            && lbbox[BOXBOTTOM] <= ambbox[BOXTOP]
            && lbbox[BOXTOP] >= ambbox[BOXBOTTOM])
        {
            mpoint_t                a = { line->v1->x >> FRACTOMAPBITS, line->v1->y >> FRACTOMAPBITS };
            mpoint_t                b = { line->v2->x >> FRACTOMAPBITS, line->v2->y >> FRACTOMAPBITS };
            const unsigned short    special = line->special;
            byte                    *doorcolor;
            const sector_t          *front = line->frontsector;
            const sector_t          *back = line->backsector;
            const unsigned short    flags = line->flags;

            if (am_rotatemode)
            {
                AM_RotatePoint(&a);
                AM_RotatePoint(&b);
            }

            if (am_correctaspectratio)
            {
                AM_CorrectAspectRatio(&a);
                AM_CorrectAspectRatio(&b);
            }

            if ((front->special == Secret || (front->special & SECRET_MASK)) && secretcolor2)
            {
                AM_DrawFline(a.x, a.y, b.x, b.y, secretcolor2,
                    (!back || (flags & ML_SECRET) || front->floorheight == front->ceilingheight ?
                    putbigwalldot : putbigdot));
                AM_DrawSides(line, secretcolor2);
            }
            else if (back && (back->special == Secret || (back->special & SECRET_MASK)) && secretcolor2)
            {
                AM_DrawFline(a.x, a.y, b.x, b.y, secretcolor2,
                    ((flags & ML_SECRET) || back->floorheight == back->ceilingheight ? putbigwalldot : putbigdot));
                AM_DrawSides(line, secretcolor2);
            }
            else if (special && (doorcolor = AM_DoorColor(special)) != cdwallcolor)
            {
                AM_DrawFline(a.x, a.y, b.x, b.y, doorcolor, putbigdot);
                AM_DrawSides(line, doorcolor);
            }
            else if (!back || (flags & ML_SECRET))
            {
                AM_DrawFline(a.x, a.y, b.x, b.y, wallcolor, putbigwalldot);
                AM_DrawSides(line, wallcolor);
            }
            else if (isteleportline[special] || isteleport[back->floorpic])
            {
                AM_DrawFline(a.x, a.y, b.x, b.y, teleportercolor, putbigdot);
                AM_DrawSides(line, teleportercolor);
            }
            else if (back->floorheight != front->floorheight)
            {
                AM_DrawFline(a.x, a.y, b.x, b.y, fdwallcolor, putbigdot);
                AM_DrawSides(line, fdwallcolor);
            }
            else if (back->ceilingheight != front->ceilingheight)
            {
                AM_DrawFline(a.x, a.y, b.x, b.y, cdwallcolor, putbigdot);
                AM_DrawSides(line, cdwallcolor);
            }
            else
            {
                AM_DrawFline(a.x, a.y, b.x, b.y, tswallcolor, putbigdot);
                AM_DrawSides(line, tswallcolor);
            }
        }
    }
}

static void AM_DrawPlayerArrow(const mline_t *lineguy, const int lineguylines,
    const angle_t angle, const fixed_t x, const fixed_t y, void (*putdot)(int, int, const byte *))
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

        if (am_correctaspectratio)
        {
            y1 = y1 * 5 / 6;
            y2 = y2 * 5 / 6;
        }

        AM_DrawFline(x + x1, y + y1, x + x2, y + y2, &playercolor, putdot);
    }
}

static void AM_DrawThingTriangle(const mline_t *lineguy, const int lineguylines, const fixed_t scale,
    const angle_t angle, const fixed_t x, const fixed_t y, const byte color, void (*putdot)(int, int, const byte *))
{
    for (int i = 0; i < lineguylines; i++)
    {
        const mline_t   line = lineguy[i];
        fixed_t         x1 = FixedMul(line.a.x, scale);
        fixed_t         y1 = FixedMul(line.a.y, scale);
        fixed_t         x2 = FixedMul(line.b.x, scale);
        fixed_t         y2 = FixedMul(line.b.y, scale);

        AM_Rotate(&x1, &y1, angle);
        AM_Rotate(&x2, &y2, angle);

        if (am_correctaspectratio)
        {
            y1 = y1 * 5 / 6;
            y2 = y2 * 5 / 6;
        }

        AM_DrawFline(x + x1, y + y1, x + x2, y + y2, &color, putdot);
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

    if (am_correctaspectratio)
        AM_CorrectAspectRatio(&point);

    if (viewplayer->cheats & (CF_ALLMAP | CF_ALLMAP_THINGS))
    {
        if (invisibility && (invisibility > STARTFLASHING || (invisibility & FLASHONTIC)))
            AM_DrawPlayerArrow(cheatplayerarrow, CHEATPLAYERARROWLINES, angle, point.x, point.y, &PUTTRANSLUCENTDOT);
        else
            AM_DrawPlayerArrow(cheatplayerarrow, CHEATPLAYERARROWLINES, angle, point.x, point.y, putbigdot2);
    }
    else if (invisibility && (invisibility > STARTFLASHING || (invisibility & FLASHONTIC)))
        AM_DrawPlayerArrow(playerarrow, CHEATPLAYERARROWLINES, angle, point.x, point.y, &PUTTRANSLUCENTDOT);
    else
        AM_DrawPlayerArrow(playerarrow, PLAYERARROWLINES, angle, point.x, point.y, putbigdot2);
}

#define THINGTRIANGLELINES  3

const mline_t thingtriangle[THINGTRIANGLELINES] =
{
    { { -32768, -45875 }, {  65536,      0 } },
    { {  65536,      0 }, { -32768,  45875 } },
    { { -32768,  45875 }, { -32768, -45875 } }
};

static void AM_DrawThings(void)
{
    const angle_t   angleoffset = (am_rotatemode ? viewangle - ANG90 : 0);

    for (int i = 0; i < numsectors; i++)
        for (mobj_t *thing = sectors[i].thinglist; thing; thing = thing->snext)
            if ((!thing->player || thing->player->mo != thing) && !(thing->flags2 & MF2_DONTMAP) && thing->interpolate)
            {
                angle_t     angle;
                mpoint_t    point = { 0, 0 };
                const int   flags = thing->flags;
                int         fx, fy;
                int         width;

                if (consoleactive || paused || freeze)
                {
                    angle = thing->angle;
                    point.x = thing->x >> FRACTOMAPBITS;
                    point.y = thing->y >> FRACTOMAPBITS;
                }
                else
                {
                    angle = R_InterpolateAngle(thing->oldangle, thing->angle);
                    point.x = (thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic)) >> FRACTOMAPBITS;
                    point.y = (thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic)) >> FRACTOMAPBITS;
                }

                if (am_rotatemode)
                    AM_RotatePoint(&point);

                if (am_correctaspectratio)
                    AM_CorrectAspectRatio(&point);

                if (!(flags & MF_SHOOTABLE) && !(flags & MF_CORPSE))
                    width = (12 << FRACBITS) >> FRACTOMAPBITS;
                else
                {
                    const short sprite = sprites[thing->sprite].spriteframes[0].lump[0];

                    width = (BETWEEN(65 << FRACBITS, MIN(spritewidth[sprite], spriteheight[sprite]),
                        80 << FRACBITS) >> FRACTOMAPBITS) / 3;
                }

                if ((fx = CXMTOF(point.x)) >= -width && fx <= MAPWIDTH + width
                    && (fy = CYMTOF(point.y)) >= -width && fy <= MAPHEIGHT + width)
                    AM_DrawThingTriangle(thingtriangle, THINGTRIANGLELINES, width, (angle - angleoffset) >> ANGLETOFINESHIFT,
                        point.x, point.y, ((flags & MF_CORPSE) ? corpsecolor : mobjinfo[thing->type].automapcolor),
                        ((flags & MF_FUZZ) ? PUTTRANSLUCENTDOT : putbigdot2));
            }
}

static void AM_DrawBloodSplats(void)
{
    const angle_t   angleoffset = (am_rotatemode ? viewangle - ANG90 : 0);

    for (int i = 0; i < numsectors; i++)
        for (bloodsplat_t *splat = sectors[i].splatlist; splat; splat = splat->next)
        {
            mpoint_t    point = { splat->x >> FRACTOMAPBITS, splat->y >> FRACTOMAPBITS };
            int         fx, fy;

            if (am_rotatemode)
                AM_RotatePoint(&point);

            if (am_correctaspectratio)
                AM_CorrectAspectRatio(&point);

            if ((fx = CXMTOF(point.x)) >= -BLOODSPLATWIDTH && fx <= MAPWIDTH + BLOODSPLATWIDTH
                && (fy = CYMTOF(point.y)) >= -BLOODSPLATWIDTH && fy <= MAPHEIGHT + BLOODSPLATWIDTH)
                AM_DrawThingTriangle(thingtriangle, THINGTRIANGLELINES, BLOODSPLATWIDTH,
                    (splat->angle - angleoffset) >> ANGLETOFINESHIFT, point.x, point.y, bloodsplatcolor, putbigdot2);
        }
}

#define MARKWIDTH   10
#define MARKHEIGHT  14

const char *marknums[] =
{
    "0000000000002222220002211112200211111120021122112002112211200211221120"
    "0211221120021122112002112211200211111120022111122000222222000000000000",
    "0000000000000222200000221120000021112000002111200000221120000002112000"
    "0002112000000211200000221122000021111200002111120000222222000000000000",
    "0000000000002222220002211112200211111120021122112002222211200002211120"
    "0022111220022111220002111222200211111120021111112002222222200000000000",
    "0000000000002222220002211112200211111120021122112002222211200002111220"
    "0002111120022222112002112211200211111120022111122000222222000000000000",
    "0000000000000022222000002111200002211120000211112000221111200021121120"
    "0221121120021111111002111111100222221120000002112000000222200000000000",
    "0000000000022222222002111111200211111120021122222002112222000211111220"
    "0211111120022222112002112211200211111120022111122000222222000000000000",
    "0000000000002222220002211112200211111120021122112002112222200211111220"
    "0211111120021122112002112211200211111120022111122000222222000000000000",
    "0000000000022222222002111111200211111120022222112000002112200002211200"
    "0002112200002211200000211220000021120000002112000000222200000000000000",
    "0000000000002222220002211112200211111120021122112002112211200221111220"
    "0211111120021122112002112211200211111120022111122000222222000000000000",
    "0000000000002222220002211112200211111120021122112002112211200211111120"
    "0221111120022222112002112211200211111120022111122000222222000000000000"
};

const char *bigmarknums[] =
{
    "2222222222222222222222111111222211111122221122112222112211222211221122"
    "2211221122221122112222112211222211111122221111112222222222222222222222",
    "0022222200002222220022221122002222112200221111220022111122002222112200"
    "2222112200222211222222221122222211111122221111112222222222222222222222",
    "2222222222222222222222111111222211111122222222112222222211222211111122"
    "2211111122221122222222112222222211111122221111112222222222222222222222",
    "2222222222222222222222111111222211111122222222112222222211220022111122"
    "0022111122222222112222222211222211111122221111112222222222222222222222",
    "2222222222222222222222112211222211221122221122112222112211222211111122"
    "2211111122222222112222222211220000221122000022112200002222220000222222",
    "2222222222222222222222111111222211111122221122222222112222222211111122"
    "2211111122222222112222222211222211111122221111112222222222222222222222",
    "2222222222222222222222111111222211111122221122222222112222222211111122"
    "2211111122221122112222112211222211111122221111112222222222222222222222",
    "2222222222222222222222111111222211111122222222112222222211222222112222"
    "2222112222221122220022112222002211220000221122000022222200002222220000",
    "2222222222222222222222111111222211111122221122112222112211222211111122"
    "2211111122221122112222112211222211111122221111112222222222222222222222",
    "2222222222222222222222111111222211111122221122112222112211222211111122"
    "2211111122222222112222222211222211111122221111112222222222222222222222"
};

static void AM_DrawMarks(const char *nums[])
{
    for (int i = 0; i < nummarks; i++)
    {
        int         number = i + 1;
        int         temp = number;
        int         digits = 1;
        int         x, y;
        mpoint_t    point = { mark[i].x, mark[i].y };

        if (am_rotatemode)
            AM_RotatePoint(&point);

        if (am_correctaspectratio)
            AM_CorrectAspectRatio(&point);

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

            if (r_detail == r_detail_low)
            {
                x += (x & 1);
                y += (y & 1);
            }

            for (int j = 0; j < MARKWIDTH * MARKHEIGHT; j++)
            {
                const unsigned int  fx = x + j % MARKWIDTH;

                if (fx < (unsigned int)MAPWIDTH)
                {
                    const unsigned int  fy = y + j / MARKWIDTH;

                    if (fy < (unsigned int)MAPHEIGHT)
                    {
                        const char  src = nums[digit][j];

                        if (src == '1')
                            mapscreen[fy * MAPWIDTH + fx] = markcolor;
                        else if (src == '2')
                        {
                            byte    *dest = &mapscreen[fy * MAPWIDTH + fx];

                            *dest = *(*dest + tinttab40);
                        }
                    }
                }
            }

            x -= MARKWIDTH - 2;

            if (r_detail == r_detail_low)
                x--;
        } while ((number /= 10) > 0);
    }
}

const int lengths[] =
{
    500,
    2000,
    5000,
    INT_MAX
};

static void AM_DrawPath(void)
{
    mpoint_t    player = { viewx >> FRACTOMAPBITS, viewy >> FRACTOMAPBITS };
    mpoint_t    end = player;

    for (int i = MAX(1, numbreadcrumbs - lengths[am_pathlength]); i < numbreadcrumbs; i++)
    {
        mpoint_t    start = { breadcrumb[i - 1].x >> FRACTOMAPBITS, breadcrumb[i - 1].y >> FRACTOMAPBITS };

        end.x = breadcrumb[i].x >> FRACTOMAPBITS;
        end.y = breadcrumb[i].y >> FRACTOMAPBITS;

        if (ABS(start.x - end.x) > 4 * FRACUNIT || ABS(start.y - end.y) > 4 * FRACUNIT)
            continue;

        if (am_rotatemode)
        {
            AM_RotatePoint(&start);
            AM_RotatePoint(&end);
        }

        if (am_correctaspectratio)
        {
            AM_CorrectAspectRatio(&start);
            AM_CorrectAspectRatio(&end);
        }

        AM_DrawFline(start.x, start.y, end.x, end.y, &pathcolor, putbigdot2);
    }

    if (am_rotatemode)
        AM_RotatePoint(&player);

    if (am_correctaspectratio)
        AM_CorrectAspectRatio(&player);

    if (ABS(end.x - player.x) <= 4 * FRACUNIT && ABS(end.y - player.y) <= 4 * FRACUNIT)
        AM_DrawFline(end.x, end.y, player.x, player.y, &pathcolor, putbigdot2);
}

#define CENTERX (WIDESCREENDELTA + VANILLAWIDTH / 2)
#define CENTERY ((VANILLAHEIGHT - VANILLASBARHEIGHT * (r_screensize < r_screensize_max)) / 2)

static void AM_DrawScaledPixel(const int x, const int y, byte *color)
{
    byte    *dest = &screens[0][(y * 2 - 1) * MAPWIDTH + x * 2 - 1];

    *dest = *(*dest + color);
    dest++;
    *dest = *(*dest + color);
    dest += MAPWIDTH;
    *dest = *(*dest + color);
    dest--;
    *dest = *(*dest + color);
}

static void AM_DrawScaledPixel2(const int x, const int y, byte color)
{
    byte    *dest = &screens[0][(y * 2 - 1) * MAPWIDTH + x * 2 - 1];

    *dest = color;
    dest++;
    *dest = color;
    dest += MAPWIDTH;
    *dest = color;
    dest--;
    *dest = color;
}

static void AM_DrawCrosshair(void)
{
    if (r_hud_translucency)
    {
        if (r_detail == r_detail_low)
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
        else
        {
            byte    *dot = &mapscreen[(MAPHEIGHT - 3) * MAPWIDTH / 2 - MAPWIDTH];

            *dot = *(*dot + am_crosshaircolor2);
            dot += MAPWIDTH;
            *dot = *(*dot + am_crosshaircolor2);
            dot += MAPWIDTH;
            *dot = *(*dot + am_crosshaircolor2);
            dot += (size_t)MAPWIDTH - 3;
            *dot = *(*dot + am_crosshaircolor2);
            dot++;
            *dot = *(*dot + am_crosshaircolor2);
            dot++;
            *dot = *(*dot + am_crosshaircolor2);
            dot++;
            *dot = *(*dot + am_crosshaircolor2);
            dot++;
            *dot = *(*dot + am_crosshaircolor2);
            dot++;
            *dot = *(*dot + am_crosshaircolor2);
            dot++;
            *dot = *(*dot + am_crosshaircolor2);
            dot += (size_t)MAPWIDTH - 3;
            *dot = *(*dot + am_crosshaircolor2);
            dot += MAPWIDTH;
            *dot = *(*dot + am_crosshaircolor2);
            dot += MAPWIDTH;
            *dot = *(*dot + am_crosshaircolor2);
        }
    }
    else
    {
        if (r_detail == r_detail_low)
        {
            AM_DrawScaledPixel2(CENTERX - 2, CENTERY, am_crosshaircolor);
            AM_DrawScaledPixel2(CENTERX - 1, CENTERY, am_crosshaircolor);
            AM_DrawScaledPixel2(CENTERX, CENTERY, am_crosshaircolor);
            AM_DrawScaledPixel2(CENTERX + 1, CENTERY, am_crosshaircolor);
            AM_DrawScaledPixel2(CENTERX + 2, CENTERY, am_crosshaircolor);
            AM_DrawScaledPixel2(CENTERX, CENTERY - 2, am_crosshaircolor);
            AM_DrawScaledPixel2(CENTERX, CENTERY - 1, am_crosshaircolor);
            AM_DrawScaledPixel2(CENTERX, CENTERY + 1, am_crosshaircolor);
            AM_DrawScaledPixel2(CENTERX, CENTERY + 2, am_crosshaircolor);
        }
        else
        {
            byte    *dot = &mapscreen[(MAPHEIGHT - 3) * MAPWIDTH / 2 - MAPWIDTH];

            *dot = am_crosshaircolor;
            dot += MAPWIDTH;
            *dot = am_crosshaircolor;
            dot += MAPWIDTH;
            *dot = am_crosshaircolor;
            dot += (size_t)MAPWIDTH - 3;
            *dot++ = am_crosshaircolor;
            *dot++ = am_crosshaircolor;
            *dot++ = am_crosshaircolor;
            *dot++ = am_crosshaircolor;
            *dot++ = am_crosshaircolor;
            *dot++ = am_crosshaircolor;
            *dot = am_crosshaircolor;
            dot += (size_t)MAPWIDTH - 3;
            *dot = am_crosshaircolor;
            dot += MAPWIDTH;
            *dot = am_crosshaircolor;
            dot += MAPWIDTH;
            *dot = am_crosshaircolor;
        }
    }
}

static void AM_StatusBarShadow(void)
{
    for (int i = 24 * 256, y = 1; y < 7; i -= 4 * 256, y++)
    {
        byte    *colormap = &colormaps[0][i];

        for (int x = 0; x < MAPWIDTH; x++)
        {
            byte    *dot = &mapscreen[MAPAREA - y * MAPWIDTH + x];

            *dot = *(*dot + colormap);
        }
    }
}

static void AM_BigStatusBarShadow(void)
{
    for (int i = 24 * 256, y = 2; y < 8; i -= 8 * 256, y += 2)
    {
        byte    *colormap = &colormaps[0][i];

        for (int x = 0; x < MAPWIDTH * 2; x++)
        {
            byte    *dot = &mapscreen[MAPAREA - y * MAPWIDTH + x];

            *dot = *(*dot + colormap);
        }
    }
}

static fixed_t AM_ShiftToMapFracSaturated(const fixed_t value)
{
    const int64_t   shifted = (int64_t)value << FRACTOMAPBITS;

    if (shifted > FIXED_MAX)
        return FIXED_MAX;
    if (shifted < FIXED_MIN)
        return FIXED_MIN;

    return (fixed_t)shifted;
}

static void AM_SetFrameVariables(void)
{
    const fixed_t   dx = m_w / 2;
    const fixed_t   dy = m_h / 2;
    const fixed_t   x = m_x + dx;
    const fixed_t   y = m_y + dy;
    const fixed_t   r = (fixed_t)sqrt((double)dx * dx + (double)dy * dy);

    am_frame.center.x = x;
    am_frame.center.y = y;

    am_frame.bbox[BOXLEFT] = AM_ShiftToMapFracSaturated(x - r);
    am_frame.bbox[BOXRIGHT] = AM_ShiftToMapFracSaturated(x + r);
    am_frame.bbox[BOXBOTTOM] = AM_ShiftToMapFracSaturated(y - r);
    am_frame.bbox[BOXTOP] = AM_ShiftToMapFracSaturated(y + r);

    if (am_rotatemode)
    {
        const double    angle = (double)((ANG90 - viewangle) >> ANGLETOFINESHIFT) * (2.0 * M_PI) / FINEANGLES;

        am_frame.sin = sin(angle);
        am_frame.cos = cos(angle);
    }
}

static void AM_ApplyAntialiasing(void)
{
    static byte dest[MAXSCREENAREA];

    memcpy(dest, mapscreen, MAPAREA);

    for (int y = 0; y <= MAPAREA - MAPWIDTH; y += MAPWIDTH)
        for (int x = y; x <= y + MAPWIDTH - 2; x++)
            dest[x] = tinttab33[(dest[x + 1] << 8) + dest[x]];

    for (int y = 0; y <= MAPAREA - MAPWIDTH; y += MAPWIDTH)
        for (int x = y + MAPWIDTH - 2; x > y; x--)
            dest[x] = tinttab33[(dest[x - 1] << 8) + dest[x]];

    for (int y = 0; y <= MAPAREA - MAPWIDTH * 2; y += MAPWIDTH)
        for (int x = y; x <= y + MAPWIDTH - 1; x++)
            dest[x] = tinttab33[(dest[x + MAPWIDTH] << 8) + dest[x]];

    for (int y = MAPAREA - MAPWIDTH; y >= MAPWIDTH; y -= MAPWIDTH)
        for (int x = y; x <= y + MAPWIDTH - 1; x++)
            dest[x] = tinttab33[(dest[x - MAPWIDTH] << 8) + dest[x]];

    memcpy(mapscreen, dest, MAPAREA);

    for (int x = 0; x < MAPWIDTH; x++)
        mapscreen[x] = tinttab33[mapscreen[x]];

    for (int y = 0; y <= MAPAREA - MAPWIDTH; y += MAPWIDTH)
        mapscreen[y] = tinttab33[mapscreen[y]];

    for (int y = MAPWIDTH - 1; y <= MAPAREA - 1; y += MAPWIDTH)
        mapscreen[y] = tinttab33[mapscreen[y]];

    if (r_screensize == r_screensize_max)
        for (int x = MAPAREA - MAPWIDTH; x < MAPAREA; x++)
            mapscreen[x] = tinttab33[mapscreen[x]];
}

void AM_Drawer(void)
{
    AM_SetFrameVariables();
    AM_ClearFB();

    skippsprinterp = true;

    if (am_sectors != am_sectors_off)
        AM_FillSectors();

    if (viewplayer->cheats & CF_ALLMAP_THINGS)
    {
        if (am_bloodsplatcolor != am_backcolor && r_blood != r_blood_none && r_bloodsplats_max)
            AM_DrawBloodSplats();

        AM_DrawWalls_Cheating();

        if (am_path && numbreadcrumbs > 0)
            AM_DrawPath();

        AM_DrawThings();
    }
    else
    {
        if (viewplayer->cheats & CF_ALLMAP)
            AM_DrawWalls_Cheating();
        else if (viewplayer->powers[pw_allmap])
            AM_DrawWalls_AllMap();
        else
            AM_DrawWalls();

        if (am_path && numbreadcrumbs > 0)
            AM_DrawPath();
    }

    AM_DrawPlayer();

    if (am_antialiasing)
        AM_ApplyAntialiasing();

    if (r_detail == r_detail_low)
    {
        V_LowGraphicDetail_2x2(mapscreen, MAPWIDTH, 0, 0, MAPWIDTH, MAPAREA, 2, 2);

        if (nummarks)
            AM_DrawMarks(bigmarknums);

        if (r_screensize < r_screensize_max && am_backcolor == nearestblack && !vanilla
            && am_sectors == am_sectors_off)
            AM_BigStatusBarShadow();
    }
    else
    {
        if (nummarks)
            AM_DrawMarks(marknums);

        if (r_screensize < r_screensize_max && am_backcolor == nearestblack && !vanilla
            && am_sectors == am_sectors_off)
            AM_StatusBarShadow();
    }

    if (am_grid)
        AM_DrawGrid();

    if (!am_followmode)
        AM_DrawCrosshair();
}
