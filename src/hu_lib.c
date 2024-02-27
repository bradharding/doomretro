/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <ctype.h>
#include <string.h>

#include "c_cmds.h"
#include "c_console.h"
#include "doomstat.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_swap.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_setup.h"
#include "v_data.h"
#include "v_video.h"

byte    tempscreen[MAXSCREENAREA];

static void HUlib_ClearTextLine(hu_textline_t *t)
{
    t->len = 0;
    t->l[0] = '\0';
    t->needsupdate = 1;
}

void HUlib_InitTextLine(hu_textline_t *t, int x, int y, patch_t **f, int sc)
{
    t->x = x;
    t->y = y;
    t->f = f;
    t->sc = sc;
    HUlib_ClearTextLine(t);
}

bool HUlib_AddCharToTextLine(hu_textline_t *t, char ch)
{
    if (t->len == HU_MAXLINELENGTH)
        return false;

    t->l[t->len++] = ch;
    t->l[t->len] = '\0';
    t->needsupdate = 4;

    return true;
}

// [BH] draw an individual character to temporary buffer
static void HU_DrawChar(int x, int y, int ch, byte *screen, int screenwidth)
{
    const int   width = (int)strlen(smallcharset[ch]) / 10;

    for (int y1 = 0; y1 < 10; y1++)
        for (int x1 = 0; x1 < width; x1++)
        {
            const unsigned char src = smallcharset[ch][y1 * width + x1];
            const int           i = (x + x1) * 2;
            const int           j = (y + y1) * 2;

            for (int yy = 0; yy < 2; yy++)
                for (int xx = 0; xx < 2; xx++)
                {
                    byte    *dest = &screen[(j + yy) * screenwidth + (i + xx)];

                    if (src == PINK)
                        *dest = 0;
                    else if (src != ' ')
                        *dest = src;
                }
        }
}

static void HU_DrawGoldChar(int x, int y, int ch, byte *screen, int screenwidth)
{
    const int   width = (int)strlen(smallcharset[ch]) / 10;

    for (int y1 = 0; y1 < 10; y1++)
        for (int x1 = 0; x1 < width; x1++)
        {
            const unsigned char src = smallcharset[ch][y1 * width + x1];
            const int           i = (x + x1) * 2;
            const int           j = (y + y1) * 2;

            for (int yy = 0; yy < 2; yy++)
                for (int xx = 0; xx < 2; xx++)
                {
                    byte    *dest = &screen[(j + yy) * screenwidth + (i + xx)];

                    if (src == PINK)
                        *dest = 0;
                    else if (src != ' ')
                        *dest = redtogold[src];
                }
        }
}

static void HU_DrawTranslucentChar(int x, int y, int ch, byte *screen, int screenwidth)
{
    const int   width = (int)strlen(smallcharset[ch]) / 10;

    for (int y1 = 0; y1 < 10; y1++)
        for (int x1 = 0; x1 < width; x1++)
        {
            const unsigned char src = smallcharset[ch][y1 * width + x1];
            const int           i = (x + x1) * 2;
            const int           j = (y + y1) * 2;

            for (int yy = 0; yy < 2; yy++)
                for (int xx = 0; xx < 2; xx++)
                {
                    byte    *dest = &screen[(j + yy) * screenwidth + (i + xx)];

                    if (src == PINK)
                        *dest = black40[*dest];
                    else if (src != ' ')
                        *dest = tinttab80[(src << 8) + *dest];
                }
        }
}

static void HU_DrawTranslucentGoldChar(int x, int y, int ch, byte *screen, int screenwidth)
{
    const int   width = (int)strlen(smallcharset[ch]) / 10;

    for (int y1 = 0; y1 < 10; y1++)
        for (int x1 = 0; x1 < width; x1++)
        {
            const unsigned char src = smallcharset[ch][y1 * width + x1];
            const int           i = (x + x1) * 2;
            const int           j = (y + y1) * 2;

            for (int yy = 0; yy < 2; yy++)
                for (int xx = 0; xx < 2; xx++)
                {
                    byte    *dest = &screen[(j + yy) * screenwidth + (i + xx)];

                    if (src == PINK)
                        *dest = black40[*dest];
                    else if (src != ' ')
                        *dest = tinttab80[(redtogold[src] << 8) + *dest];
                }
        }
}

static void HUlib_DrawAltHUDTextLine(hu_textline_t *l)
{
    bool            italics = false;
    unsigned char   prevletter = '\0';
    unsigned char   prevletter2 = '\0';
    int             x = HU_ALTHUDMSGX;
    int             y = HU_ALTHUDMSGY;
    int             color = (message_secret ? nearestgold : (message_warning ? nearestred :
                        (r_hud_translucency ? nearestwhite : nearestlightgray)));
    const int       len = l->len;
    byte            *tinttab = (automapactive ? tinttab70 : tinttab50);

    if (!automapactive)
        color = (r_textures ? (viewplayer->fixedcolormap == INVERSECOLORMAP ?
            colormaps[0][32 * 256 + color] : color) : (viewplayer->fixedcolormap == INVERSECOLORMAP ?
            colormaps[0][32 * 256 + color] : (message_secret ? nearestgold : (message_warning ?
            nearestred : nearestblack))));

    if (fade)
    {
        byte    *tinttabs[] = { NULL, tinttab10, tinttab20, tinttab30, tinttab40 };

        if (automapactive)
        {
            tinttabs[1] = tinttab10;
            tinttabs[2] = tinttab25;
            tinttabs[3] = tinttab40;
            tinttabs[4] = tinttab60;
        }

        if (message_counter <= 4)
            tinttab = tinttabs[message_counter];
        else if (message_fadeon && message_counter >= HU_MSGTIMEOUT - 3)
            tinttab = tinttabs[HU_MSGTIMEOUT - message_counter + 1];
    }

    if (idbehold)
        althudtextfunc(x, y + 12, screens[0],
            altunderscores, false, color, SCREENWIDTH, tinttab);

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = l->l[i];

        if (letter == ITALICSONCHAR)
            italics = true;
        else if (letter == ITALICSOFFCHAR)
            italics = false;
        else
        {
            patch_t *patch = unknownchar;

            if (letter == '\n')
            {
                x = OVERLAYTEXTX;
                y += OVERLAYLINEHEIGHT;
                continue;
            }
            else
            {
                const int   c = letter - CONSOLEFONTSTART;

                if (c >= 0 && c < CONSOLEFONTSIZE)
                    patch = consolefont[c];

                if (!i || prevletter == ' ')
                {
                    if (letter == '\'')
                        patch = lsquote;
                    else if (letter == '"')
                        patch = ldquote;
                }

                // [BH] apply kerning to certain character pairs
                for (int j = 0; altkern[j].char1; j++)
                    if (prevletter == altkern[j].char1 && letter == altkern[j].char2)
                    {
                        x += altkern[j].adjust;
                        break;
                    }

                if (letter == '(' && prevletter == ' ')
                {
                    if (prevletter2 == '.')
                        x--;
                    else if (prevletter2 == '!')
                        x -= 2;
                }
            }

            althudtextfunc(x, y, screens[0], patch, italics, color, SCREENWIDTH, tinttab);
            x += SHORT(patch->width);

            prevletter2 = prevletter;
            prevletter = letter;
        }
    }
}

void HUlib_DrawAltAutomapTextLine(hu_textline_t *l, bool external)
{
    bool            italics = false;
    unsigned char   prevletter = '\0';
    unsigned char   prevletter2 = '\0';
    int             x = HU_ALTHUDMSGX;
    byte            *fb1 = (external ? mapscreen : screens[0]);
    const int       len = l->len;
    int             color = (secretmap ? nearestgold : (r_hud_translucency ? nearestwhite : nearestlightgray));

    for (int i = 0; i < len; i++)
    {
        const unsigned char letter = l->l[i];

        if (letter == ITALICSONCHAR)
            italics = true;
        else if (letter == ITALICSOFFCHAR)
            italics = false;
        else
        {
            patch_t     *patch = unknownchar;
            const int   c = letter - CONSOLEFONTSTART;

            if (c >= 0 && c < CONSOLEFONTSIZE)
                patch = consolefont[c];

            if (!i || prevletter == ' ')
            {
                if (letter == '\'')
                    patch = lsquote;
                else if (letter == '"')
                    patch = ldquote;
            }

            // [BH] apply kerning to certain character pairs
            for (int j = 0; altkern[j].char1; j++)
                if (prevletter == altkern[j].char1 && letter == altkern[j].char2)
                {
                    x += altkern[j].adjust;
                    break;
                }

            if (italics)
            {
                if (letter == '-')
                    x++;
                else if (letter == '\'')
                    x--;

                if (prevletter == '/')
                    x -= 2;
                else if (prevletter == '\'')
                    x++;

                if (letter == 'T' && prevletter == ITALICSOFFCHAR && prevletter2 == ' ')
                    x -= 2;
            }
            else if (letter == '-' && prevletter == ITALICSOFFCHAR)
                x++;
            else if (letter == '(' && prevletter == ' ')
            {
                if (prevletter2 == '.')
                    x--;
                else if (prevletter2 == '!')
                    x -= 2;
            }

            althudtextfunc(x, SCREENHEIGHT - 30, fb1, patch, (italics && letter != '_'
                && letter != '-' && letter != '+' && letter != ',' && letter != '/'), color,
                (external ? MAPWIDTH : SCREENWIDTH), tinttab70);
            x += SHORT(patch->width);
        }

        prevletter2 = prevletter;
        prevletter = letter;
    }
}

const kern_t kern[] =
{
    { ',',  '1',  -1 }, { ',',  '7',  -1 }, { ',',  'Y',  -1 }, { '.',  '"',  -1 },
    { '.',  '1',  -1 }, { '.',  '7',  -1 }, { '3',  '"',  -1 }, { 'D',  '\'', -1 },
    { 'F',  '.',  -1 }, { 'L',  '"',  -1 }, { 'L',  'T',  -1 }, { 'P',  ',',  -1 },
    { 'P',  '.',  -1 }, { 'T',  ',',  -1 }, { 'T',  '.',  -1 }, { 'Y',  ',',  -1 },
    { 'Y',  '.',  -1 }, { '\0', '\0',  0 }
};

static void HUlib_DrawTextLine(hu_textline_t *l, bool external)
{
    int             textwidth = 0;
    int             x = l->x;
    int             y = l->y;
    int             maxx;
    int             maxy;
    unsigned char   prev = '\0';
    unsigned char   prev2 = '\0';
    byte            *fb1 = screens[0];
    byte            *fb2 = screens[(r_screensize < r_screensize_max - 1 && !automapactive)];
    byte            *tinttab1 = tinttab50;
    byte            *tinttab2 = tinttab80;
    const int       black = (nearestblack << 8);
    const int       len = l->len;
    const int       screenwidth = (external ? MAPWIDTH : SCREENWIDTH);

    if (external)
    {
        fb1 = mapscreen;
        fb2 = mapscreen;
    }

    memset(tempscreen, PINK, SCREENAREA);

    for (int i = 0; i < len; i++)
    {
        const unsigned char c = toupper(l->l[i]);
        short               charwidth;

        if (c == '\n')
        {
            x = l->x;
            y += hu_font[0]->height + 2;
        }
        else if (c == ' ')
        {
            charwidth = (vanilla ? 4 : (i > 0 && (prev == '.' || prev == '!' || prev == '?') ? 5 : 3));
            x += charwidth;
            textwidth += charwidth;
        }
        else if (c >= l->sc && c <= '_')
        {
            int j = c - '!';

            // [BH] have matching curly single and double quotes
            if (!i || l->l[i - 1] == ' ')
            {
                if (c == '"')
                    j = 64;
                else if (c == '\'')
                    j = 65;
            }

            if (STCFNxxx)
            {
                // [BH] display lump from PWAD with shadow
                charwidth = SHORT(l->f[c - l->sc]->width);

                if (prev2 == '.' && prev == ' ' && c == '(')
                    x -= 2;

                V_DrawPatchToTempScreen(x, MAX(0, y - 1), l->f[c - l->sc]);
            }
            else
            {
                if ((prev == '-' || (prev2 == '.' && prev == ' ' && c == '(')))
                    x -= 2;
                else
                    // [BH] apply kerning to certain character pairs
                    for (int k = 0; kern[k].char1; k++)
                        if (prev == kern[k].char1 && c == kern[k].char2)
                        {
                            x += kern[k].adjust * 2;
                            break;
                        }

                // [BH] draw individual character
                charwidth = (short)strlen(smallcharset[j]) / 10 - 1;

                if (message_secret)
                    HU_DrawGoldChar(x, y - 1, j, tempscreen, screenwidth);
                else
                    HU_DrawChar(x, y - 1, j, tempscreen, screenwidth);
            }

            x += charwidth;
            textwidth += charwidth;
        }

        prev2 = prev;
        prev = c;
    }

    // [BH] draw underscores for IDBEHOLD cheat message
    if (idbehold && !STCFNxxx && s_STSTR_BEHOLD2 && !vanilla)
    {
        for (int y1 = 0; y1 < 4; y1++)
            for (int x1 = 0; x1 < VANILLAWIDTH; x1++)
            {
                const unsigned char src = underscores[y1 * VANILLAWIDTH + x1];

                if (src != ' ')
                    for (int y2 = 0; y2 < 2; y2++)
                        for (int x2 = 0; x2 < 2; x2++)
                        {
                            byte    *dest = &tempscreen[((l->y + y1 + 6) * 2 + y2) * screenwidth
                                        + (l->x + x1 - 3) * 2 + x2];

                            *dest = (src == PINK ? 0 : src);
                        }
            }
    }

    // [BH] draw entire message from buffer onto screen
    if (fade)
    {
        byte    *tinttabs1[] = { NULL, tinttab10, tinttab20, tinttab20, tinttab30, tinttab30, tinttab40, tinttab40 };
        byte    *tinttabs2[] = { NULL, tinttab10, tinttab20, tinttab30, tinttab40, tinttab50, tinttab60, tinttab70 };

        if (message_counter <= 7)
        {
            tinttab1 = tinttabs1[message_counter];
            tinttab2 = tinttabs2[message_counter];
        }
        else if (message_fadeon && message_counter >= HU_MSGTIMEOUT - 6)
        {
            const int   i = HU_MSGTIMEOUT - message_counter + 1;

            tinttab1 = tinttabs1[i];
            tinttab2 = tinttabs2[i];
        }
    }

    maxx = (l->x + textwidth + 1) * 2;
    maxy = (y + 10) * 2;

    for (int yy = MAX(0, l->y - 1); yy < maxy; yy++)
        for (int xx = l->x; xx < maxx; xx++)
        {
            const int   dot = yy * screenwidth + xx;
            byte        *source = &tempscreen[dot];
            byte        *dest = &fb1[dot];

            if (!*source)
                *dest = tinttab1[black + fb2[dot]];
            else if (*source != PINK)
                *dest = (r_hud_translucency ? tinttab2[(*source << 8) + fb2[dot]] : *source);
        }
}

void HUlib_DrawAutomapTextLine(hu_textline_t *l, bool external)
{
    const int       width = (external ? MAPWIDTH : SCREENWIDTH);
    int             x = l->x;
    int             y = l->y;
    unsigned char   prev = '\0';
    unsigned char   prev2 = '\0';
    byte            *fb = (external ? mapscreen : screens[0]);
    char            s[513];
    const int       maxwidth = MIN(VANILLAWIDTH, MAPWIDTH / 2);
    int             len = l->len;

    M_StringCopy(s, l->l, sizeof(s));

    while (M_StringWidth(s) > maxwidth)
    {
        len--;

        s[len - 3] = '.';
        s[len - 2] = '.';
        s[len - 1] = '.';
        s[len] = '\0';
    }

    for (int i = 0; i < len; i++)
    {
        const unsigned char c = toupper(s[i]);

        if (c == ' ')
            x += (vanilla ? 8 : (i > 0 && (prev == '.' || prev == '!' || prev == '?') ? 10 : 6));
        else if (c != '\n' && c >= l->sc && c <= '_')
        {
            int j = c - '!';

            // [BH] have matching curly single and double quotes
            if (!i || s[i - 1] == ' ')
            {
                if (c == '"')
                    j = 64;
                else if (c == '\'')
                    j = 65;
            }

            if (STCFNxxx)
            {
                if (prev2 == '.' && prev == ' ' && c == '(')
                    x -= 2;

                if (r_hud_translucency)
                    V_DrawTranslucentHUDText(x, y - 1, fb, l->f[c - l->sc], width);
                else
                    V_DrawHUDText(x, y - 1, fb, l->f[c - l->sc], width);
            }
            else
            {
                if (prev == '-' || (prev2 == '.' && prev == ' ' && c == '('))
                    x -= 2;
                else
                    // [BH] apply kerning to certain character pairs
                    for (int k = 0; kern[k].char1; k++)
                        if (prev == kern[k].char1 && c == kern[k].char2)
                        {
                            x += kern[k].adjust * 2;
                            break;
                        }

                if (secretmap)
                {
                    if (r_hud_translucency)
                        HU_DrawTranslucentGoldChar(x / 2, y / 2 - 1, j, fb, width);
                    else
                        HU_DrawGoldChar(x / 2, y / 2 - 1, j, fb, width);
                }
                else
                {
                    if (r_hud_translucency)
                        HU_DrawTranslucentChar(x / 2, y / 2 - 1, j, fb, width);
                    else
                        HU_DrawChar(x / 2, y / 2 - 1, j, fb, width);
                }
            }

            x += SHORT(l->f[c - l->sc]->width) * 2;
        }

        prev2 = prev;
        prev = c;
    }
}

// sorta called by HU_Erase and just better darn get things straight
void HUlib_EraseTextLine(hu_textline_t *l)
{
    // Only erases when NOT in automap and the screen is reduced,
    // and the text must either need updating or refreshing
    // (because of a recent change back from the automap)
    if (!automapactive && viewwindowx && l->needsupdate)
    {
        const int   lh = (SHORT(l->f[0]->height) + 4) * 2;

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

void HUlib_InitSText(hu_stext_t *s, int x, int y, patch_t **font, int startchar, bool *on)
{
    s->on = on;
    s->laston = true;

    HUlib_InitTextLine(&s->l, x, y, font, startchar);
}

static void HUlib_AddLineToSText(hu_stext_t *s)
{
    HUlib_ClearTextLine(&s->l);

    // everything needs updating
    s->l.needsupdate = 4;
}

void HUlib_AddMessageToSText(hu_stext_t *s, const char *msg)
{
    HUlib_AddLineToSText(s);

    while (*msg)
        HUlib_AddCharToTextLine(&s->l, *(msg++));
}

void HUlib_DrawSText(hu_stext_t *s, bool external)
{
    if (!*s->on)
        return; // if not on, don't draw

    // draw everything
    if (r_althud && r_screensize == r_screensize_max)
        HUlib_DrawAltHUDTextLine(&s->l);
    else
        HUlib_DrawTextLine(&s->l, external);
}

void HUlib_EraseSText(hu_stext_t *s)
{
    if (s->laston && !*s->on)
        s->l.needsupdate = 4;

    HUlib_EraseTextLine(&s->l);

    s->laston = *s->on;
}
