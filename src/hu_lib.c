/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

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
  id Software.

========================================================================
*/

#include <ctype.h>
#include <string.h>

#include "am_map.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "hu_lib.h"
#include "i_swap.h"
#include "r_local.h"
#include "v_data.h"
#include "v_video.h"

extern dboolean automapactive;
extern dboolean vid_widescreen;
extern dboolean r_translucency;

static void HUlib_clearTextLine(hu_textline_t *t)
{
    t->len = 0;
    t->l[0] = 0;
    t->needsupdate = true;
}

void HUlib_initTextLine(hu_textline_t *t, int x, int y, patch_t **f, int sc)
{
    t->x = x;
    t->y = y;
    t->f = f;
    t->sc = sc;
    HUlib_clearTextLine(t);
}

dboolean HUlib_addCharToTextLine(hu_textline_t *t, char ch)
{
    if (t->len == HU_MAXLINELENGTH)
        return false;
    else
    {
        t->l[t->len++] = ch;
        t->l[t->len] = 0;
        t->needsupdate = 4;
        return true;
    }
}

static void HU_drawDot(int x, int y, char src)
{
    byte        *dest = &tempscreen[y * SCREENWIDTH + x];

    if (src == '\xFB')
        *dest = 0;
    else if (src != ' ')
        *dest = src;
}

// [BH] draw an individual character to temporary buffer
static void HU_drawChar(int x, int y, int ch)
{
    int w = strlen(smallcharset[ch]) / 10;
    int x1, y1;

    for (y1 = 0; y1 < 10; y1++)
        for (x1 = 0; x1 < w; x1++)
        {
            char        src = smallcharset[ch][y1 * w + x1];
            int         i = (x + x1) * SCREENSCALE;
            int         j = (y + y1) * SCREENSCALE;
            int         xx, yy;

            for (yy = 0; yy < SCREENSCALE; ++yy)
                for (xx = 0; xx < SCREENSCALE; ++xx)
                    HU_drawDot(i + xx, j + yy, src);
        }
}

static struct
{
    char        char1;
    char        char2;
    int         adjust;
} kern[] = {
    { '.', '1',  -1 },
    { '.', '7',  -1 },
    { ',', '1',  -1 },
    { ',', '7',  -1 },
    { ',', 'Y',  -1 },
    { 'T', '.',  -1 },
    { 'T', ',',  -1 },
    { 'Y', '.',  -1 },
    { 'Y', ',',  -1 },
    { 'D', '\'', -1 },
    { '3', '\"', -1 },
    { 0,   0,     0 }
};

void HUlib_drawTextLine(hu_textline_t *l, dboolean external)
{
    int                 i;
    int                 w = 0;
    int                 tw = 0;
    int                 x, y;
    int                 xx, yy;
    static char         prev;
    byte                *fb1 = (external ? mapscreen : screens[0]);
    byte                *fb2 = (external ? mapscreen : screens[r_screensize < 7 && !automapactive]);

    // draw the new stuff
    x = l->x;
    y = l->y;
    memset(tempscreen, 251, SCREENWIDTH * SCREENHEIGHT);
    for (i = 0; i < l->len; i++)
    {
        unsigned char   c = toupper(l->l[i]);

        if (c != '\n' && c != ' ' && ((c >= l->sc && c <= '_') || l->l[i] == '°'))
        {
            int j = c - '!';

            // [BH] have matching curly single and double quotes
            if (!i || (i > 0 && l->l[i - 1] == ' '))
            {
                if (c == '\"')
                    j = 64;
                else if (c == '\'')
                    j = 65;
#if defined(WIN32)
                else if (c == '\u2019')
                    j = 65;
#endif
            }

            if (l->l[i] == '°')
                if (STCFN034)
                    continue;
                else
                    j = 66;

            if (STCFN034)
            {
                // [BH] display lump from PWAD with shadow
                w = SHORT(l->f[c - l->sc]->width);
                V_DrawPatchToTempScreen(x, l->y, l->f[c - l->sc]);
            }
            else
            {
                int     k = 0;

                // [BH] apply kerning to certain character pairs
                while (kern[k].char1)
                {
                    if (prev == kern[k].char1 && c == kern[k].char2)
                    {
                        x += kern[k].adjust;
                        break;
                    }
                    k++;
                }

                // [BH] draw individual character
                w = strlen(smallcharset[j]) / 10 - 1;
                HU_drawChar(x, y - 1, j);
            }
            x += w;
            tw += w;

            prev = c;
        }
        else if (c == ' ')
        {
            w = (i > 0 && (l->l[i - 1] == '.' || l->l[i - 1] == '!' || l->l[i - 1] == '?') ? 5 : 3);
            x += w;
            tw += w;
        }
    }

    // [BH] draw underscores for IDBEHOLD cheat message
    if (idbehold && !STCFN034 && s_STSTR_BEHOLD2)
    {
        int     x1, y1;
        int     x2, y2;

        for (y1 = 0; y1 < 4; y1++)
            for (x1 = 0; x1 < ORIGINALWIDTH; x1++)
            {
                char    src = (automapactive && !vid_widescreen ?
                    underscores2[y1 * ORIGINALWIDTH + x1] : underscores1[y1 * ORIGINALWIDTH + x1]);

                for (y2 = 0; y2 < SCREENSCALE; y2++)
                    for (x2 = 0; x2 < SCREENSCALE; x2++)
                    {
                        byte    *dest = &tempscreen[((8 + y1) * SCREENSCALE + y2) * SCREENWIDTH +
                                                    x1 * SCREENSCALE + x2];

                        if (src == '\xFB')
                            *dest = 0;
                        else if (src != ' ')
                            *dest = src;
                    }
            }
    }

    // [BH] draw entire message from buffer onto screen with translucency
    for (yy = l->y - 1; yy < (y + 10) * SCREENSCALE; yy++)
        for (xx = l->x; xx < (l->x + tw + 1) * SCREENSCALE; xx++)
        {
            int         dot = yy * SCREENWIDTH + xx;
            byte        *source = &tempscreen[dot];
            byte        *dest1 = &fb1[dot];
            byte        *dest2 = &fb2[dot];

            if (!*source)
                *dest1 = (r_translucency ? tinttab50[*dest2] : 0);
            else if (*source != 251)
            {
                byte color = *source;

                if (r_translucency && !hacx)
                {
                    color = tinttab33[(*dest2 << 8) + color];
                    if (color >= 168 && color <= 175)
                        color -= 144;
                }
                *dest1 = color;
            }
        }
}

// sorta called by HU_Erase and just better darn get things straight
void HUlib_eraseTextLine(hu_textline_t *l)
{
    // Only erases when NOT in automap and the screen is reduced,
    // and the text must either need updating or refreshing
    // (because of a recent change back from the automap)
    if (!automapactive && viewwindowx && l->needsupdate)
    {
        int     y;
        int     yoffset;
        int     lh = (SHORT(l->f[0]->height) + 4) * SCREENSCALE;

        for (y = l->y, yoffset = y * SCREENWIDTH; y < l->y + lh; y++, yoffset += SCREENWIDTH)
        {
            if (y < viewwindowy || y >= viewwindowy + viewheight)
                R_VideoErase(yoffset, SCREENWIDTH);                             // erase entire line
            else
            {
                R_VideoErase(yoffset, viewwindowx);                             // erase left border
                R_VideoErase(yoffset + viewwindowx + viewwidth, viewwindowx);   // erase right border
            }
        }
    }

    if (l->needsupdate)
        l->needsupdate--;
}

void HUlib_initSText(hu_stext_t *s, int x, int y, int h, patch_t **font, int startchar, dboolean *on)
{
    int i;

    s->h = h;
    s->on = on;
    s->laston = true;
    s->cl = 0;
    for (i = 0; i < h; i++)
        HUlib_initTextLine(&s->l[i], x, y - i * (SHORT(font[0]->height) + 1), font, startchar);
}

static void HUlib_addLineToSText(hu_stext_t *s)
{
    int i;

    // add a clear line
    if (++s->cl == s->h)
        s->cl = 0;
    HUlib_clearTextLine(&s->l[s->cl]);

    // everything needs updating
    for (i = 0; i < s->h; i++)
        s->l[i].needsupdate = 4;
}

void HUlib_addMessageToSText(hu_stext_t *s, char *prefix, char *msg)
{
    HUlib_addLineToSText(s);
    if (prefix)
        while (*prefix)
            HUlib_addCharToTextLine(&s->l[s->cl], *(prefix++));

    while (*msg)
        HUlib_addCharToTextLine(&s->l[s->cl], *(msg++));
}

void HUlib_drawSText(hu_stext_t *s)
{
    int i;

    if (!*s->on)
        return; // if not on, don't draw

    // draw everything
    for (i = 0; i < s->h; i++)
    {
        int             idx = s->cl - i;
        hu_textline_t   *l;

        if (idx < 0)
            idx += s->h;        // handle queue of lines

        l = &s->l[idx];

        // need a decision made here on whether to skip the draw
        HUlib_drawTextLine(l, false); // no cursor, please
    }
}

void HUlib_eraseSText(hu_stext_t *s)
{
    int i;

    for (i = 0; i < s->h; i++)
    {
        if (s->laston && !*s->on)
            s->l[i].needsupdate = 4;
        HUlib_eraseTextLine(&s->l[i]);
    }
    s->laston = *s->on;
}
