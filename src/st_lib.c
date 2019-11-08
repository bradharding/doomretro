/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

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

#include "doomstat.h"
#include "i_swap.h"
#include "m_config.h"
#include "st_lib.h"
#include "v_video.h"

dboolean    usesmallnums;

void STlib_InitNum(st_number_t *n, int x, int y, patch_t **pl, int *num, int width)
{
    n->x = x;
    n->y = y;
    n->width = width;
    n->num = num;
    n->p = pl;
}

static void STlib_DrawLowNum(int number, int color, int shadow, int x, int y)
{
    const char *lownums[10] =
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

    for (int i = 0, j = (y * SCREENWIDTH + x) * SCREENSCALE; i < 96; i++)
    {
        char    dot = lownums[number][i];

        if (dot == '1')
            screens[0][j + i / 8 * SCREENWIDTH + (i & 7)] = color;
        else if (dot == '2')
            screens[0][j + i / 8 * SCREENWIDTH + (i & 7)] = shadow;
    }
}

static void STlib_DrawHighNum(int number, int color, int shadow, int x, int y)
{
    const char *highnums[10] =
    {
        "011110001111110011021120112211221122112211221122112211221122112211111122011110220022222200022220",
        "001100000111000001112200001122000011220000112200001122000011220001111200011112000002222000022220",
        "111110001111110000221120002211220111112211111022110222221122222011111100111111000022222200222222",
        "111110001111110000221120002211220111112201111122000211220002112211111122111110220022222200222220",
        "110011001100112211001122112211221111112211111122002211220022112200001122000011220000002200000022",
        "111111001111110011222222112222221111100011111100002211200022112211111122111110220022222200222220",
        "011110001111100011022220112222201111100011111100112211201122112211111122011110220022222200022220",
        "111111001111110000221122002211220001102200011022001102200011022001102200011022000002200000022000",
        "011110001111110011021120112211220111122211111122112211201122112211111122011110220022222200022220",
        "011110001111110011021120112211221111112201111122002211220002112201111122011110220002222200022220"
    };

    for (int i = 0, j = (y * SCREENWIDTH + x) * SCREENSCALE; i < 96; i++)
    {
        char    dot = highnums[number][i];

        if (dot == '1')
            screens[0][j + i / 8 * SCREENWIDTH + (i & 7)] = color;
        else if (dot == '2')
            screens[0][j + i / 8 * SCREENWIDTH + (i & 7)] = shadow;
    }
}

static void STlib_DrawBigNum(st_number_t *n)
{
    int num = MAX(0, *n->num);

    // if non-number, do not draw it
    if (num == 1994)
        return;
    else
    {
        int numdigits = n->width;
        int x = n->x;
        int w = SHORT(n->p[0]->width);

        // in the special case of 0, you draw 0
        if (!num)
            V_DrawPatch(x - w, n->y, 0, n->p[0]);
        else
        {
            // draw the new number
            while (num && numdigits--)
            {
                x -= w;
                V_DrawPatch(x, n->y, 0, n->p[num % 10]);
                num /= 10;
            }
        }
    }
}

static void STlib_DrawSmallNum(st_number_t *n)
{
    int numdigits = n->width;
    int num = MAX(0, *n->num);
    int x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
    {
        if (usesmallnums)
        {
            if (r_detail == r_detail_high)
                STlib_DrawHighNum(0, 160, 47, x - 4, n->y);
            else
                STlib_DrawLowNum(0, 160, 47, x - 4, n->y);
        }
        else
            V_DrawPatch(x - 4, n->y, 0, n->p[0]);
    }
    else if (usesmallnums)
    {
        // draw the new number
        if (r_detail == r_detail_high)
        {
            while (num && numdigits--)
            {
                x -= 4;
                STlib_DrawHighNum(num % 10, 160, 47, x, n->y);
                num /= 10;
            }
        }
        else
        {
            while (num && numdigits--)
            {
                x -= 4;
                STlib_DrawLowNum(num % 10, 160, 47, x, n->y);
                num /= 10;
            }
        }
    }
    else
    {
        // draw the new number
        while (num && numdigits--)
        {
            x -= 4;
            V_DrawPatch(x, n->y, 0, n->p[num % 10]);
            num /= 10;
        }
    }

}

void STlib_UpdateBigNum(st_number_t *n)
{
    STlib_DrawBigNum(n);
}

void STlib_UpdateSmallNum(st_number_t *n)
{
    STlib_DrawSmallNum(n);
}

void STlib_InitPercent(st_percent_t *p, int x, int y, patch_t **pl, int *num, patch_t *percent)
{
    STlib_InitNum(&p->n, x, y, pl, num, 3);
    p->p = percent;
}

void STlib_UpdatePercent(st_percent_t *per, int refresh)
{
    if (refresh)
        V_DrawPatch(per->n.x, per->n.y, 0, per->p);

    STlib_UpdateBigNum(&per->n);
}

void STlib_InitMultIcon(st_multicon_t *mi, int x, int y, patch_t **il, int *inum)
{
    mi->x = x;
    mi->y = y;
    mi->oldinum = -1;
    mi->inum = inum;
    mi->p = il;
}

void STlib_UpdateMultIcon(st_multicon_t *mi, dboolean refresh)
{
    if ((mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        V_DrawPatch(mi->x, mi->y, 0, mi->p[*mi->inum]);
        mi->oldinum = *mi->inum;
    }
}

void STlib_UpdateArmsIcon(st_multicon_t *mi, dboolean refresh, int i)
{
    if ((mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        if (usesmallnums)
        {
            if (r_detail == r_detail_high)
                STlib_DrawHighNum(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y);
            else
                STlib_DrawLowNum(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y);
        }
        else
            V_DrawPatch(mi->x, mi->y, 0, mi->p[*mi->inum]);

        mi->oldinum = *mi->inum;
    }
}
