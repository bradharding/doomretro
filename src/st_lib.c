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

#include "doomstat.h"
#include "i_swap.h"
#include "m_config.h"
#include "st_lib.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

extern int r_detail;

//
//  Loads and store the stminus lump.
//
patch_t *sttminus;

void STlib_init(void)
{
    sttminus = W_CacheLumpName("STTMINUS", PU_STATIC);
}

void STlib_initNum(st_number_t *n, int x, int y, patch_t **pl, int *num, dboolean *on, int width)
{
    n->x = x;
    n->y = y;
    n->oldnum = 0;
    n->width = width;
    n->num = num;
    n->on = on;
    n->p = pl;
}

const char *lownums[10] =
{
    "111111001111110011221122112211221122112211221122"
    "112211221122112211111122111111220022222200222222",
    "001100000011000011112200111122000011220000112200"
    "001122000011220011111100111111000022222200222222",
    "111111001111110000221122002211221111112211111122"
    "112222221122222211111100111111000022222200222222",
    "111111001111110000221122002211220011112200111122"
    "000011220000112211111122111111220022222200222222",
    "110011001100110011221122112211221111112211111122"
    "002211220022112200001122000011220000002200000022",
    "111111001111110011222222112222221111110011111100"
    "002211220022112211111122111111220022222200222222",
    "111111001111110011222222112222221111110011111100"
    "112211221122112211111122111111220022222200222222",
    "111111001111110000221122002211220011002200110022"
    "110022001100220011220000112200000022000000220000",
    "111111001111110011221122112211221111112211111122"
    "112211221122112211111122111111220022222200222222",
    "111111001111110011221122112211221111112211111122"
    "002211220022112211111122111111220022222200222222"
};

void STlib_drawLowNum(int number, int color, int shadow, int x, int y)
{
    int         i;
    int         j = (y * SCREENWIDTH + x) * SCREENSCALE;

    for (i = 0; i < 96; i++)
    {
        char    dot = lownums[number][i];

        if (dot == '1')
            screens[0][j + i / 8 * SCREENWIDTH + i % 8] = color;
        else if (dot == '2')
            screens[0][j + i / 8 * SCREENWIDTH + i % 8] = shadow;
    }
}

const char *highnums[10] =
{
    "011110001111110011021120112211221122112211221122"
    "112211221122112211111122011110220022222200022220",
    "001100000111000011112200001122000011220000112200"
    "001122000011220011111100111111000022222200222222",
    "111110001111110000221120002211220111112211111022"
    "110222221122222011111100111111000022222200222222",
    "111110001111110000221120002211220111112201111122"
    "000211220002112211111122111110220022222200222220",
    "110011001100110011221122112211221111112211111122"
    "002211220022112200001122000011220000002200000022",
    "111111001111110011222222112222221111100011111100"
    "002211200022112211111122111110220022222200222220",
    "011110001111100011022220112222201111100011111100"
    "112211201122112211111122011110220022222200022220",
    "111111001111110000221122002111220011102201110222"
    "011022200112220001122000011220000002200000022000",
    "011110001111110011021120112211220111102211111122"
    "110211201122112211111122011110220022222200022220",
    "011110001111110011021120112211221111112201111122"
    "002211220002112201111122011110220002222200022220"
};

void STlib_drawHighNum(int number, int color, int shadow, int x, int y)
{
    int         i;
    int         j = (y * SCREENWIDTH + x) * SCREENSCALE;

    for (i = 0; i < 96; i++)
    {
        char    dot = highnums[number][i];

        if (dot == '1')
        {
            int xx, yy;

            for (yy = 0; yy < SCREENSCALE; ++yy)
                for (xx = 0; xx < SCREENSCALE; ++xx)
                    screens[0][j + i / 8 * SCREENWIDTH + i % 8] = color;
        }
        else if (dot == '2')
        {
            int xx, yy;

            for (yy = 0; yy < SCREENSCALE; ++yy)
                for (xx = 0; xx < SCREENSCALE; ++xx)
                    screens[0][j + i / 8 * SCREENWIDTH + i % 8] = shadow;
        }
    }
}

//
// A fairly efficient way to draw a number
//  based on differences from the old number.
// Note: worth the trouble?
//
void STlib_drawNum(st_number_t *n)
{
    int         numdigits = n->width;
    int         num = *n->num;

    int         w = SHORT(n->p[0]->width);
    int         x = n->x;

    int         neg;

    n->oldnum = *n->num;

    neg = (num < 0);

    if (neg)
    {
        if (numdigits == 2 && num < -9)
            num = -9;
        else if (numdigits == 3 && num < -99)
            num = -99;

        num = -num;
    }

    // clear the area
    x = n->x - numdigits * w;

    // if non-number, do not draw it
    if (num == 1994)
        return;

    x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
    {
        if (SHORT(n->p[0]->height) == 6 && !STYSNUM0)
        {
            if (r_detail == low)
                STlib_drawLowNum(0, 160, 47, x - w, n->y);
            else
                STlib_drawHighNum(0, 160, 47, x - w, n->y);
        }
        else
            V_DrawPatch(x - w, n->y, FG, n->p[0]);
    }

    // draw the new number
    while (num && numdigits--)
    {
        x -= w;
        if (SHORT(n->p[0]->height) == 6 && !STYSNUM0)
        {
            if (r_detail == low)
                STlib_drawLowNum(num % 10, 160, 47, x, n->y);
            else
                STlib_drawHighNum(num % 10, 160, 47, x, n->y);
        }
        else
           V_DrawPatch(x, n->y, FG, n->p[num % 10]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
        V_DrawPatch(x - 8, n->y, FG, sttminus);
}

void STlib_DrawNumber(st_number_t *n)
{
    int         num = *n->num;
    int         x = n->x;
    int         y = n->y;
    int         w = SHORT(n->p[0]->width);

    if (num == 1994)
        return;

    if (num < 10)
        V_DrawPatch(x + 14, y, FG, n->p[num]);
    else if (num < 100)
    {
        V_DrawPatch(x + 7, y, FG, n->p[num / 10]);
        V_DrawPatch(x + 7 + w, y, FG, n->p[num % 10]);
    }
    else
    {
        V_DrawPatch(x, n->y, FG, n->p[num / 100]);
        V_DrawPatch(x + w, y, FG, n->p[(num - num / 100 * 100) / 10]);
        V_DrawPatch(x + 2 * w, y, FG, n->p[num % 10]);
    }
}

void STlib_DrawPercent(st_percent_t *per)
{
    int         num = *per->n.num;
    int         x = per->n.x;
    int         y = per->n.y;
    int         w = SHORT(per->n.p[0]->width);

    if (num == 1994)
        return;

    if (num < 10)
    {
        V_DrawPatch(x + 14, y, FG, per->n.p[num]);
        V_DrawPatch(x + 14 + w, y, FG, per->p);
    }
    else if (num < 100)
    {
        V_DrawPatch(x + 7, y, FG, per->n.p[num / 10]);
        V_DrawPatch(x + 7 + w, y, FG, per->n.p[num % 10]);
        V_DrawPatch(x + 7 + 2 * w, y, FG, per->p);
    }
    else
    {
        V_DrawPatch(x, y, FG, per->n.p[num / 100]);
        V_DrawPatch(x + w, y, FG, per->n.p[(num - num / 100 * 100) / 10]);
        V_DrawPatch(x + 2 * w, y, FG, per->n.p[num % 10]);
        V_DrawPatch(x + 3 * w, y, FG, per->p);
    }
}

void STlib_updateNum(st_number_t *n)
{
    if (*n->on)
        STlib_drawNum(n);
}

void STlib_initPercent(st_percent_t *p, int x, int y, patch_t **pl,
                       int *num, dboolean *on, patch_t *percent)
{
    STlib_initNum(&p->n, x, y, pl, num, on, 3);
    p->p = percent;
}

void STlib_updatePercent(st_percent_t *per, int refresh)
{
    if (refresh && *per->n.on)
        V_DrawPatch(per->n.x, per->n.y, FG, per->p);

    STlib_updateNum(&per->n);
}

void STlib_initMultIcon(st_multicon_t *i, int x, int y, patch_t **il, int *inum, dboolean *on)
{
    i->x = x;
    i->y = y;
    i->oldinum = -1;
    i->inum = inum;
    i->on = on;
    i->p = il;
}

void STlib_updateMultIcon(st_multicon_t *mi, dboolean refresh)
{
    if (*mi->on && (mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        V_DrawPatch(mi->x, mi->y, FG, mi->p[*mi->inum]);
        mi->oldinum = *mi->inum;
    }
}

void STlib_updateArmsIcon(st_multicon_t *mi, dboolean refresh, int i)
{
    if (*mi->on && (mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        if (STYSNUM0)
            V_DrawPatch(mi->x, mi->y, FG, mi->p[*mi->inum]);
        else
        {
            if (r_detail == low)
                STlib_drawLowNum(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y);
            else
                STlib_drawHighNum(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y);
        }
        mi->oldinum = *mi->inum;
    }
}

void STlib_initBinIcon(st_binicon_t *b, int x, int y, patch_t *i, dboolean *val, dboolean *on)
{
    b->x = x;
    b->y = y;
    b->oldval = false;
    b->val = val;
    b->on = on;
    b->p = i;
}

void STlib_updateBinIcon(st_binicon_t *bi, dboolean refresh)
{
    if (*bi->on && (bi->oldval != *bi->val || refresh))
    {
        if (*bi->val)
            V_DrawPatch(bi->x, bi->y, FG, bi->p);

        bi->oldval = *bi->val;
    }
}

void STlib_updateBigBinIcon(st_binicon_t *bi, dboolean refresh)
{
    if (*bi->on && (bi->oldval != *bi->val || refresh))
    {
        if (*bi->val)
            V_DrawBigPatch(bi->x, bi->y, FG, bi->p);

        bi->oldval = *bi->val;
    }
}
