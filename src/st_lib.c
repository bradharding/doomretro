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

#include "doomstat.h"
#include "hu_stuff.h"
#include "i_swap.h"
#include "m_config.h"
#include "st_lib.h"
#include "v_video.h"

bool    usesmallnums;

static void (*statbarnumfunc)(int, int, int, int, int, patch_t *);

void STlib_InitNum(st_number_t *n, int x, int y, patch_t **pl, int *num, int width)
{
    n->x = x;
    n->y = y;
    n->width = width;
    n->num = num;
    n->p = pl;
}

static void STlib_DrawLowNum(int number, int color, int shadow, int x, int y, patch_t *patch)
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

    x += WIDESCREENDELTA;

    for (int i = 0, j = (y * SCREENWIDTH + x) * SCREENSCALE; i < 96; i++)
    {
        const char  dot = lownums[number][i];

        if (dot == '1')
            screens[0][j + i / 8 * SCREENWIDTH + (i & 7)] = color;
        else if (dot == '2')
            screens[0][j + i / 8 * SCREENWIDTH + (i & 7)] = shadow;
    }
}

static void STlib_DrawLowNumPatch(int number, int color, int shadow, int x, int y, patch_t *patch)
{
    V_DrawPatch(x, y, 0, patch);
}

static void STlib_DrawHighNum(int number, int color, int shadow, int x, int y, patch_t *patch)
{
    const char *highnums[10] =
    {
        "011110001111110011021120112211221122112211221122112211221122112211111122011110220022222200022220",
        "001100000111000001112200001122000011220000112200001122000011220001111200011112000002222000022220",
        "111110001111110000221120002211220111112211111022110222221122222011111100111111000022222200222222",
        "111110001111110000221120002211220111122201111122000211200002112211111122111110220022222200222220",
        "110011001100110011221122112211221111112211111122002211220022112200001122000011220000002200000022",
        "111111001111110011222222112222221111100011111100002211200022112211111122111110220022222200222220",
        "011110001111100011022220112222201111100011111100112211201122112211111122011110220022222200022220",
        "111111001111110000221122002112220001102200110220001102200110220001102200011220000002200000022000",
        "011110001111110011021120112211220111122211111122112211201122112211111122011110220022222200022220",
        "011110001111110011021120112211221111112201111122002211220002112201111122011110220002222200022220"
    };

    x += WIDESCREENDELTA;

    for (int i = 0, j = (y * SCREENWIDTH + x) * SCREENSCALE; i < 96; i++)
    {
        const char  dot = highnums[number][i];

        if (dot == '1')
            screens[0][j + i / 8 * SCREENWIDTH + (i & 7)] = color;
        else if (dot == '2')
            screens[0][j + i / 8 * SCREENWIDTH + (i & 7)] = shadow;
    }
}

void STlib_UpdateBigAmmoNum(st_number_t *n)
{
    // if non-number, do not draw it
    if (*n->num == 1994)
        return;
    else
    {
        int         num = *n->num;
        int         x = n->x;
        const int   width = SHORT(n->p[0]->width);

        // in the special case of 0, you draw 0
        if (!num)
            V_DrawPatch(x - width, n->y, 0, n->p[0]);
        else
            // draw the new number
            while (num)
            {
                V_DrawPatch((x -= width), n->y, 0, n->p[num % 10]);
                num /= 10;
            }
    }
}

void STlib_UpdateBigArmorNum(st_number_t *n)
{
    int         num = *n->num;
    int         x = n->x;
    const int   width = SHORT(n->p[0]->width);

    // in the special case of 0, you draw 0
    if (!num)
        V_DrawPatch(x - width, n->y, 0, n->p[0]);
    else
        // draw the new number
        while (num)
        {
            V_DrawPatch((x -= width), n->y, 0, n->p[num % 10]);
            num /= 10;
        }
}

void STlib_UpdateBigHealthNum(st_number_t *n)
{
    int         num = (negativehealth ? ABS(*n->num) : MAX(0, *n->num));
    int         x = n->x;
    const int   width = SHORT(n->p[0]->width);

    // in the special case of 0, you draw 0
    if (!num)
        V_DrawPatch(x - width, n->y, 0, n->p[0]);
    else
        // draw the new number
        while (num)
        {
            V_DrawPatch((x -= width), n->y, 0, n->p[num % 10]);
            num /= 10;
        }

    // draw a minus sign if necessary
    if ((num = *n->num) < 0 && negativehealth && minuspatch)
    {
        if ((num >= -199 && num <= -100) || (num >= -79 && num <= -70)
            || (num >= -19 && num <= -10) || num == -7 || num == -1)
            x += 2;

        V_DrawPatch(x - minuspatchwidth, n->y, 0, minuspatch);
    }
}

void STlib_UpdateSmallNum(st_number_t *n)
{
    int num = MAX(0, *n->num);
    int x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
        statbarnumfunc(0, 160, 47, x - 4, n->y, n->p[0]);
    else
        // draw the new number
        while (num)
        {
            x -= 4;
            statbarnumfunc(num % 10, 160, 47, x, n->y, n->p[num % 10]);
            num /= 10;
        }
}

void STlib_InitPercent(st_percent_t *p, int x, int y, patch_t **pl, int *num, patch_t *percent)
{
    STlib_InitNum(&p->n, x, y, pl, num, 3);
    p->p = percent;
}

void STlib_UpdateArmorPercent(st_percent_t *per, int refresh)
{
    if (refresh)
        V_DrawPatch(per->n.x, per->n.y, 0, per->p);

    STlib_UpdateBigArmorNum(&per->n);
}

void STlib_UpdateHealthPercent(st_percent_t *per, int refresh)
{
    if (refresh)
        V_DrawPatch(per->n.x, per->n.y, 0, per->p);

    STlib_UpdateBigHealthNum(&per->n);
}

void STlib_InitMultIcon(st_multicon_t *mi, int x, int y, patch_t **il, int *inum)
{
    mi->x = x;
    mi->y = y;
    mi->oldinum = -1;
    mi->inum = inum;
    mi->patch = il;
}

void STlib_UpdateMultIcon(st_multicon_t *mi, bool refresh)
{
    if ((mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        V_DrawPatch(mi->x, mi->y, 0, mi->patch[*mi->inum]);
        mi->oldinum = *mi->inum;
    }
}

void STlib_UpdateArmsIcon(st_multicon_t *mi, bool refresh, int i)
{
    if ((mi->oldinum != *mi->inum || refresh) && *mi->inum != -1)
    {
        statbarnumfunc(i + 2, (*mi->inum ? 160 : 93), 47, mi->x, mi->y, mi->patch[*mi->inum]);
        mi->oldinum = *mi->inum;
    }
}

void STLib_Init(void)
{
    statbarnumfunc = (!usesmallnums ? &STlib_DrawLowNumPatch :
        (r_detail == r_detail_high ? &STlib_DrawHighNum : &STlib_DrawLowNum));
}
