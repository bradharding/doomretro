/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

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

void STlib_initNum(st_number_t *n, int x, int y, patch_t **pl, int *num, dboolean *on, int width)
{
    n->x = x;
    n->y = y;
    n->width = width;
    n->num = num;
    n->on = on;
    n->p = pl;
}

static void STlib_drawLowNum(int number, int color, int shadow, int x, int y)
{
    static const char *lownums[10] =
    {
        "111111001111110011221122112211221122112211221122112211221122112211111122111111220022222200222222",
        "001100000011000011112200111122000011220000112200001122000011220011111100111111000022222200222222",
        "111111001111110000221122002211221111112211111122112222221122222211111100111111000022222200222222",
        "111111001111110000221122002211220011112200111122000011220000112211111122111111220022222200222222",
        "110011001100110011221122112211221111112211111122002211220022112200001122000011220000002200000022",
        "111111001111110011222222112222221111110011111100002211220022112211111122111111220022222200222222",
        "111111001111110011222222112222221111110011111100112211221122112211111122111111220022222200222222",
        "111111001111110000221122002211220011002200110022110022001100220011220000112200000022000000220000",
        "111111001111110011221122112211221111112211111122112211221122112211111122111111220022222200222222",
        "111111001111110011221122112211221111112211111122002211220022112211111122111111220022222200222222"
    };

    int j = (y * SCREENWIDTH + x) * SCREENSCALE;

    for (int i = 0; i < 96; i++)
    {
        char    dot = lownums[number][i];

        if (dot == '1')
            screens[0][j + i / 8 * SCREENWIDTH + i % 8] = color;
        else if (dot == '2')
            screens[0][j + i / 8 * SCREENWIDTH + i % 8] = shadow;
    }
}

static void STlib_drawHighNum(int number, int color, int shadow, int x, int y)
{
    static const char *highnums[10] =
    {
        "011110001111110011021120112211221122112211221122112211221122112211111122011110220022222200022220",
        "011100000111000000112200001122000011220000112200001122000011220000112200001122000000220000002200",
        "111110001111110000221120002211220111112211111022110222221122222011111100111111000022222200222222",
        "111110001111110000221120002211220111112201111122000211220002112211111122111110220022222200222220",
        "110011001100110011221122112211221111112201111122002211220002112200001122000011220000002200000022",
        "111111001111110011222222112222221111100011111100002211200022112211111122111110220022222200222220",
        "011110001111100011022220112222201111100011111100112211201122112211111122011110220022222200022220",
        "111110001111110000221120002211220000112200001122000011220000112200001122000011220000002200000022",
        "011110001111110011021120112211220111122211111122112211201122112211111122011110220022222200022220",
        "011110001111110011021120112211221111112201111122002211220002112201111122011110220002222200022220"
    };

    int j = (y * SCREENWIDTH + x) * SCREENSCALE;

    for (int i = 0; i < 96; i++)
    {
        char    dot = highnums[number][i];

        if (dot == '1')
        {
            for (int yy = 0; yy < SCREENSCALE; yy++)
                for (int xx = 0; xx < SCREENSCALE; xx++)
                    screens[0][j + i / 8 * SCREENWIDTH + i % 8] = color;
        }
        else if (dot == '2')
        {
            for (int yy = 0; yy < SCREENSCALE; yy++)
                for (int xx = 0; xx < SCREENSCALE; xx++)
                    screens[0][j + i / 8 * SCREENWIDTH + i % 8] = shadow;
        }
    }
}

//
// A fairly efficient way to draw a number
//  based on differences from the old number.
// Note: worth the trouble?
//
static void STlib_drawNum(st_number_t *n)
{
    int         numdigits = n->width;
    int         num = MAX(0, *n->num);
    patch_t     *patch = n->p[0];
    int         w = SHORT(patch->width);
    dboolean    smallnum = (SHORT(patch->height) == 6 && !STYSNUM0 && STBAR == 2);
    int         x = n->x;

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
            V_DrawPatch(x - w, n->y, 0, patch);
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
               V_DrawPatch(x, n->y, 0, n->p[num % 10]);

            num /= 10;
        }
}

void STlib_updateNum(st_number_t *n)
{
    if (*n->on)
        STlib_drawNum(n);
}

void STlib_initPercent(st_percent_t *p, int x, int y, patch_t **pl, int *num, dboolean *on, patch_t *percent)
{
    STlib_initNum(&p->n, x, y, pl, num, on, 3);
    p->p = percent;
}

void STlib_updatePercent(st_percent_t *per, int refresh)
{
    if (refresh && *per->n.on)
        V_DrawPatch(per->n.x, per->n.y, 0, per->p);

    STlib_updateNum(&per->n);
}

void STlib_initMultIcon(st_multicon_t *mi, int x, int y, patch_t **il, int *inum, dboolean *on)
{
    mi->x = x;
    mi->y = y;
    mi->oldinum = -1;
    mi->inum = inum;
    mi->on = on;
    mi->p = il;
}

void STlib_updateMultIcon(st_multicon_t *mi, dboolean refresh)
{
    if (*mi->on && (mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        V_DrawPatch(mi->x, mi->y, 0, mi->p[*mi->inum]);
        mi->oldinum = *mi->inum;
    }
}

void STlib_updateArmsIcon(st_multicon_t *mi, dboolean refresh, int i)
{
    if (*mi->on && (mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        if (STYSNUM0 || STBAR > 2)
            V_DrawPatch(mi->x, mi->y, 0, mi->p[*mi->inum]);
        else if (r_detail == r_detail_low)
            STlib_drawLowNum(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y);
        else
            STlib_drawHighNum(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y);

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
            V_DrawPatch(bi->x, bi->y, 0, bi->p);

        bi->oldval = *bi->val;
    }
}

void STlib_updateBigBinIcon(st_binicon_t *bi, dboolean refresh)
{
    if (*bi->on && (bi->oldval != *bi->val || refresh))
    {
        if (*bi->val)
            V_DrawBigPatch(bi->x, bi->y, 0, bi->p);

        bi->oldval = *bi->val;
    }
}
