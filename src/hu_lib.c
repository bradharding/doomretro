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

#include <ctype.h>
#include <string.h>

#include "am_map.h"
#include "c_console.h"
#include "doomstat.h"
#include "hu_lib.h"
#include "i_swap.h"
#include "m_config.h"
#include "r_local.h"
#include "v_data.h"
#include "v_video.h"

extern patch_t  *consolefont[CONSOLEFONTSIZE];
extern patch_t  *degree;
extern int      white;

static void HUlib_clearTextLine(hu_textline_t *t)
{
    t->len = 0;
    t->l[0] = '\0';
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
        t->l[t->len] = '\0';
        t->needsupdate = 4;
        return true;
    }
}

static void HU_drawDot(int x, int y, unsigned char src)
{
    byte    *dest = &tempscreen[y * SCREENWIDTH + x];

    if (src == 251)
        *dest = 0;
    else if (src != ' ')
        *dest = src;
}

// [BH] draw an individual character to temporary buffer
static void HU_drawChar(int x, int y, int ch)
{
    int w = (int)strlen(smallcharset[ch]) / 10;

    if (r_messagescale == r_messagescale_small)
    {
        for (int y1 = 0; y1 < 10; y1++)
            for (int x1 = 0; x1 < w; x1++)
                HU_drawDot(x + x1, y + y1, smallcharset[ch][y1 * w + x1]);
    }
    else
    {
        for (int y1 = 0; y1 < 10; y1++)
            for (int x1 = 0; x1 < w; x1++)
            {
                const unsigned char src = smallcharset[ch][y1 * w + x1];
                const int           i = (x + x1) * SCREENSCALE;
                const int           j = (y + y1) * SCREENSCALE;

                for (int yy = 0; yy < SCREENSCALE; yy++)
                    for (int xx = 0; xx < SCREENSCALE; xx++)
                        HU_drawDot(i + xx, j + yy, src);
            }
    }
}

static struct
{
    char    char1;
    char    char2;
    int     adjust;
} c_kern[] = {
    { ' ',  '(',  -1 }, { ' ',  'T',  -1 }, { '\"', '+',  -1 }, { '\"', '.',  -1 },
    { '\"', 'a',  -1 }, { '\"', 'c',  -1 }, { '\"', 'd',  -1 }, { '\"', 'e',  -1 },
    { '\"', 'g',  -1 }, { '\"', 'j',  -2 }, { '\"', 'o',  -1 }, { '\"', 'q',  -1 },
    { '\"', 's',  -1 }, { '\\', '\\', -2 }, { '\\', 'd',  -1 }, { '\\', 'V',  -1 },
    { '\'', 'a',  -1 }, { '\'', 'a',  -1 }, { '\'', 'c',  -1 }, { '\'', 'd',  -1 },
    { '\'', 'e',  -1 }, { '\'', 'g',  -1 }, { '\'', 'j',  -2 }, { '\'', 'o',  -1 },
    { '\'', 's',  -1 }, { '.',  '\\', -1 }, { '.',  '4',  -1 }, { '.',  '7',  -1 },
    { ',',  '4',  -1 }, { '/',  '/',  -2 }, { '/',  'd',  -1 }, { '/',  'o',  -1 },
    { ':', '\\',  -1 }, { '_',  'f',  -1 }, { '0',  ',',  -1 }, { '0',  ';',  -1 },
    { '0',  'j',  -2 }, { '1',  '\"', -1 }, { '1',  '\'', -1 }, { '1',  'j',  -2 },
    { '2',  'j',  -2 }, { '3',  ',',  -1 }, { '3',  ';',  -1 }, { '3',  'j',  -2 },
    { '4',  'j',  -2 }, { '5',  ',',  -1 }, { '5',  ';',  -1 }, { '5',  'j',  -2 },
    { '6',  ',',  -1 }, { '6',  'j',  -2 }, { '7',  '.',  -2 }, { '7',  ',',  -2 },
    { '7',  ';',  -1 }, { '7',  'j',  -2 }, { '8',  ',',  -1 }, { '8',  ';',  -1 },
    { '8',  'j',  -2 }, { '9',  ',',  -1 }, { '9',  ';',  -1 }, { '9',  'j',  -2 },
    { 'F',  '.',  -1 }, { 'F',  ',',  -1 }, { 'F',  ';',  -1 }, { 'L',  '\\', -1 },
    { 'L',  '\"', -1 }, { 'L',  '\'', -1 }, { 'P',  '.',  -1 }, { 'P',  ',',  -1 },
    { 'P',  ';',  -1 }, { 'T',  '.',  -1 }, { 'T',  ',',  -1 }, { 'T',  ';',  -1 },
    { 'T',  'a',  -1 }, { 'T',  'e',  -1 }, { 'T',  'o',  -1 }, { 'V',  '.',  -1 },
    { 'V',  ',',  -1 }, { 'V',  ';',  -1 }, { 'Y',  '.',  -1 }, { 'Y',  ',',  -1 },
    { 'Y',  ';',  -1 }, { 'a',  '\"', -1 }, { 'a',  '\'', -1 }, { 'a',  'j',  -2 },
    { 'b',  ',',  -1 }, { 'b',  ';',  -1 }, { 'b',  '\"', -1 }, { 'b',  '\\', -1 },
    { 'b',  '\'', -1 }, { 'b',  'j',  -2 }, { 'c',  '\\', -1 }, { 'c',  ',',  -1 },
    { 'c',  ';',  -1 }, { 'c',  '\"', -1 }, { 'c',  '\'', -1 }, { 'c',  'j',  -2 },
    { 'd',  'j',  -2 }, { 'e',  '\\', -1 }, { 'e',  ',',  -1 }, { 'e',  ';',  -1 },
    { 'e',  '\"', -1 }, { 'e',  '\'', -1 }, { 'e',  '_',  -1 }, { 'e',  'j',  -2 },
    { 'f',  ' ',  -1 }, { 'f',  ',',  -2 }, { 'f',  ';',  -1 }, { 'f',  '_',  -1 },
    { 'f',  'a',  -1 }, { 'f',  'j',  -2 }, { 'h',  '\\', -1 }, { 'h',  'j',  -2 },
    { 'i',  'j',  -2 }, { 'k',  'j',  -2 }, { 'l',  'j',  -2 }, { 'm',  '\"', -1 },
    { 'm',  '\\', -1 }, { 'm',  '\'', -1 }, { 'm',  'j',  -2 }, { 'n',  '\\', -1 },
    { 'n',  '\"', -1 }, { 'n',  '\'', -1 }, { 'n',  'j',  -2 }, { 'o',  '\\', -1 },
    { 'o',  ',',  -1 }, { 'o',  ';',  -1 }, { 'o',  '\"', -1 }, { 'o',  '\'', -1 },
    { 'o',  'j',  -2 }, { 'p',  '\\', -1 }, { 'p',  ',',  -1 }, { 'p',  ';',  -1 },
    { 'p',  '\"', -1 }, { 'p',  '\'', -1 }, { 'p',  'j',  -2 }, { 'r',  ' ',  -1 },
    { 'r',  '\\', -1 }, { 'r',  '.',  -2 }, { 'r',  ',',  -2 }, { 'r',  ';',  -1 },
    { 'r',  '\"', -1 }, { 'r',  '\'', -1 }, { 'r',  '_',  -1 }, { 'r',  'a',  -1 },
    { 'r',  'j',  -2 }, { 's',  '\\', -1 }, { 's',  ',',  -1 }, { 's',  ';',  -1 },
    { 's',  'j',  -2 }, { 't',  'j',  -2 }, { 'u',  'j',  -2 }, { 'v',  ',',  -1 },
    { 'v',  ';',  -1 }, { 'v',  'j',  -2 }, { 'w',  'j',  -2 }, { 'x',  'j',  -2 },
    { 'z',  'j',  -2 }, {  0 ,   0 ,   0 }
};

static void HUlib_drawAltHUDTextLine(hu_textline_t *l)
{
    unsigned char   prevletter = '\0';
    int             x = HU_ALTHUDMSGX;

    for (int i = 0; i < l->len; i++)
    {
        unsigned char   letter = l->l[i];
        unsigned char   nextletter = l->l[i + 1];
        patch_t         *patch;
        int             j = 0;

        if (letter == 194 && nextletter == 176)
        {
            patch = degree;
            i++;
        }
        else
            patch = consolefont[letter - CONSOLEFONTSTART];

        // [BH] apply kerning to certain character pairs
        while (c_kern[j].char1)
        {
            if (prevletter == c_kern[j].char1 && letter == c_kern[j].char2)
            {
                x += c_kern[j].adjust;
                break;
            }

            j++;
        }

        V_DrawAltHUDText(x, HU_ALTHUDMSGY, patch, white);
        x += SHORT(patch->width);
        prevletter = letter;
    }
}

static struct
{
    char    char1;
    char    char2;
    int     adjust;
} hu_kern[] = {
    { '.', '1',  -1 },
    { '.', '7',  -1 },
    { ',', '1',  -1 },
    { ',', '7',  -1 },
    { ',', 'Y',  -1 },
    { 'F', '.',  -1 },
    { 'T', '.',  -1 },
    { 'T', ',',  -1 },
    { 'Y', '.',  -1 },
    { 'Y', ',',  -1 },
    { 'D', '\'', -1 },
    { '3', '\"', -1 },
    { 'L', '\"', -1 },
    { 0,   0,     0 }
};

void HUlib_drawTextLine(hu_textline_t *l, dboolean external)
{
    int         w = 0;
    int         tw = 0;
    int         x, y;
    int         maxx, maxy;
    static char prev;
    byte        *fb1 = (external ? mapscreen : screens[0]);
    byte        *fb2 = (external ? mapscreen : screens[r_screensize < 7 && !automapactive]);

    // draw the new stuff
    x = l->x;
    y = l->y;
    memset(tempscreen, 251, SCREENWIDTH * SCREENHEIGHT);

    for (int i = 0; i < l->len; i++)
    {
        unsigned char   c = toupper(l->l[i]);

        if (c != '\n' && c != ' ' && ((c >= l->sc && c <= '_') || c == 176))
        {
            int j = c - '!';

            // [BH] have matching curly single and double quotes
            if (!i || l->l[i - 1] == ' ')
            {
                if (c == '\"')
                    j = 64;
                else if (c == '\'')
                    j = 65;
#if defined(_WIN32)
                else if (c == 146)
                    j = 65;
#endif
            }

            if (c == 176)
            {
                if (STCFN034)
                    continue;
                else
                    j = 66;
            }

            if (STCFN034)
            {
                // [BH] display lump from PWAD with shadow
                w = SHORT(l->f[c - l->sc]->width);

                if (r_messagescale == r_messagescale_big)
                    V_DrawPatchToTempScreen(x, l->y, l->f[c - l->sc]);
                else
                    V_DrawBigPatchToTempScreen(x, l->y, l->f[c - l->sc]);
            }
            else
            {
                int k = 0;

                // [BH] apply kerning to certain character pairs
                while (hu_kern[k].char1)
                {
                    if (prev == hu_kern[k].char1 && c == hu_kern[k].char2)
                    {
                        x += hu_kern[k].adjust;
                        break;
                    }

                    k++;
                }

                // [BH] draw individual character
                w = (int)strlen(smallcharset[j]) / 10 - 1;
                HU_drawChar(x, y - 1, j);

                prev = c;
            }

            x += w;
            tw += w;
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
        int scale = r_messagescale + 1;

        for (int y1 = 0; y1 < 4; y1++)
            for (int x1 = 0; x1 < ORIGINALWIDTH; x1++)
            {
                unsigned char   src = (automapactive && !vid_widescreen ? underscores2[y1 * ORIGINALWIDTH + x1] :
                                    underscores1[y1 * ORIGINALWIDTH + x1]);

                for (int y2 = 0; y2 < scale; y2++)
                    for (int x2 = 0; x2 < scale; x2++)
                    {
                        byte    *dest = &tempscreen[((8 + y1) * scale + y2) * SCREENWIDTH + x1 * scale + x2];

                        if (src == 251)
                            *dest = 0;
                        else if (src != ' ')
                            *dest = src;
                    }
            }
    }

    // [BH] draw entire message from buffer onto screen with translucency
    maxy = y + 10;
    maxx = (l->x + tw + 1);

    if (r_messagescale == r_messagescale_big)
    {
        maxy *= SCREENSCALE;
        maxx *= SCREENSCALE;
    }

    for (int yy = MAX(0, l->y - 1); yy < maxy; yy++)
        for (int xx = l->x; xx < maxx; xx++)
        {
            int     dot = yy * SCREENWIDTH + xx;
            byte    *source = &tempscreen[dot];
            byte    *dest1 = &fb1[dot];
            byte    *dest2 = &fb2[dot];

            if (!*source)
                *dest1 = tinttab50[*dest2];
            else if (*source != 251)
            {
                byte color = *source;

                if (vid_widescreen && r_hud_translucency && !hacx)
                {
                    color = tinttab25[(*dest2 << 8) + color];

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
        const int   lh = (SHORT(l->f[0]->height) + 4) * SCREENSCALE;

        for (int y = l->y, yoffset = y * SCREENWIDTH; y < l->y + lh; y++, yoffset += SCREENWIDTH)
            if (y < viewwindowy || y >= viewwindowy + viewheight)
                R_VideoErase(yoffset, SCREENWIDTH);                             // erase entire line
            else
            {
                R_VideoErase(yoffset, viewwindowx);                             // erase left border
                R_VideoErase(yoffset + viewwindowx + viewwidth, viewwindowx);   // erase right border
            }
    }

    if (l->needsupdate)
        l->needsupdate--;
}

void HUlib_initSText(hu_stext_t *s, int x, int y, int h, patch_t **font, int startchar, dboolean *on)
{
    s->h = h;
    s->on = on;
    s->laston = true;
    s->cl = 0;

    for (int i = 0; i < h; i++)
        HUlib_initTextLine(&s->l[i], x, y - i * (SHORT(font[0]->height) + 1), font, startchar);
}

static void HUlib_addLineToSText(hu_stext_t *s)
{
    // add a clear line
    if (++s->cl == s->h)
        s->cl = 0;

    HUlib_clearTextLine(&s->l[s->cl]);

    // everything needs updating
    for (int i = 0; i < s->h; i++)
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

void HUlib_drawSText(hu_stext_t *s, dboolean external)
{
    if (!*s->on)
        return; // if not on, don't draw

    // draw everything
    for (int i = 0; i < s->h; i++)
    {
        int             idx = s->cl - i;
        hu_textline_t   *l;

        if (idx < 0)
            idx += s->h;        // handle queue of lines

        l = &s->l[idx];

        // need a decision made here on whether to skip the draw
        if (vid_widescreen && r_althud && !automapactive)
            HUlib_drawAltHUDTextLine(l);
        else
            HUlib_drawTextLine(l, external);
    }
}

void HUlib_eraseSText(hu_stext_t *s)
{
    for (int i = 0; i < s->h; i++)
    {
        if (s->laston && !*s->on)
            s->l[i].needsupdate = 4;

        HUlib_eraseTextLine(&s->l[i]);
    }

    s->laston = *s->on;
}
