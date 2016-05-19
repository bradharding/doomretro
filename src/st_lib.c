/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

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

#include "doomstat.h"
#include "i_swap.h"
#include "m_config.h"
#include "st_lib.h"
#include "v_video.h"

extern int      r_detail;

void STlib_initNum(st_number_t *n, int x, int y, patch_t **pl, int *num, bool *on, int width)
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
    int i;
    int j = (y * SCREENWIDTH + x) * SCREENSCALE;

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
    "001100000111000001112200001122000011220000112200"
    "001122000011220001111200011112000002222000022220",
    "011110001111110011221120002211220111112211111022"
    "110222221122222011111100111111000022222200222222",
    "011110001111110011221120002211220011102200111122"
    "000011201100112211111122011110220022222200222220",
    "110011001100110011221122112211221111112211111122"
    "002211220022112200001122000011220000002200000022",
    "111111001111110011222222112222221111100011111100"
    "002211201122112211111122011110220022222200022220",
    "011110001111110011021120112222221111102211111100"
    "112211201122112211111122011110220022222200022220",
    "111111001111110000221122002111220011102201110222"
    "011022200112220001122000011220000002200000022000",
    "011110001111110011021120112211220111102211111122"
    "110211201122112211111122011110220022222200022220",
    "011110001111110011021120112211221111112201111122"
    "002211221102112211111122011110220022222200022220"
};

void STlib_drawHighNum(int number, int color, int shadow, int x, int y)
{
    int i;
    int j = (y * SCREENWIDTH + x) * SCREENSCALE;

    for (i = 0; i < 96; i++)
    {
        char    dot = highnums[number][i];
        int     xx, yy;

        if (dot == '1')
        {
            for (yy = 0; yy < SCREENSCALE; ++yy)
                for (xx = 0; xx < SCREENSCALE; ++xx)
                    screens[0][j + i / 8 * SCREENWIDTH + i % 8] = color;
        }
        else if (dot == '2')
        {
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

    patch_t     *patch = n->p[0];
    int         w = SHORT(patch->width);
    bool        smallnum = (SHORT(patch->height) == 6 && !STYSNUM0 && STBAR == 2);
    int         x = n->x;

    n->oldnum = *n->num;

    // if non-number, do not draw it
    if (num == 1994)
        return;

    // in the special case of 0, you draw 0
    if (!num)
    {
        if (smallnum)
        {
            if (r_detail == r_detail_low)
                STlib_drawLowNum(0, 160, 47, x - w, n->y);
            else
                STlib_drawHighNum(0, 160, 47, x - w, n->y);
        }
        else
            V_DrawPatch(x - w, n->y, FG, patch);
    }
    else

        // draw the new number
        while (num && numdigits--)
        {
            x -= w;
            if (smallnum)
            {
                if (r_detail == r_detail_low)
                    STlib_drawLowNum(num % 10, 160, 47, x, n->y);
                else
                    STlib_drawHighNum(num % 10, 160, 47, x, n->y);
            }
            else
               V_DrawPatch(x, n->y, FG, n->p[num % 10]);
            num /= 10;
        }
}

void STlib_updateNum(st_number_t *n)
{
    if (*n->on)
        STlib_drawNum(n);
}

void STlib_initPercent(st_percent_t *p, int x, int y, patch_t **pl, int *num, bool *on,
    patch_t *percent)
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

void STlib_initMultIcon(st_multicon_t *i, int x, int y, patch_t **il, int *inum, bool *on)
{
    i->x = x;
    i->y = y;
    i->oldinum = -1;
    i->inum = inum;
    i->on = on;
    i->p = il;
}

void STlib_updateMultIcon(st_multicon_t *mi, bool refresh)
{
    if (*mi->on && (mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        V_DrawPatch(mi->x, mi->y, FG, mi->p[*mi->inum]);
        mi->oldinum = *mi->inum;
    }
}

void STlib_updateArmsIcon(st_multicon_t *mi, bool refresh, int i)
{
    if (*mi->on && (mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        if (STYSNUM0 || STBAR > 2)
            V_DrawPatch(mi->x, mi->y, FG, mi->p[*mi->inum]);
        else
        {
            if (r_detail == r_detail_low)
                STlib_drawLowNum(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y);
            else
                STlib_drawHighNum(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y);
        }
        mi->oldinum = *mi->inum;
    }
}

void STlib_initBinIcon(st_binicon_t *b, int x, int y, patch_t *i, bool *val, bool *on)
{
    b->x = x;
    b->y = y;
    b->oldval = false;
    b->val = val;
    b->on = on;
    b->p = i;
}

void STlib_updateBinIcon(st_binicon_t *bi, bool refresh)
{
    if (*bi->on && (bi->oldval != *bi->val || refresh))
    {
        if (*bi->val)
            V_DrawPatch(bi->x, bi->y, FG, bi->p);

        bi->oldval = *bi->val;
    }
}

void STlib_updateBigBinIcon(st_binicon_t *bi, bool refresh)
{
    if (*bi->on && (bi->oldval != *bi->val || refresh))
    {
        if (*bi->val)
            V_DrawBigPatch(bi->x, bi->y, FG, bi->p);

        bi->oldval = *bi->val;
    }
}
