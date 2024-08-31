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
    t->width = 0;
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
    t->width += M_CharacterWidth(ch, '\0');

    return true;
}

// [BH] draw an individual character to temporary buffer
static void HU_DrawChar(int x, int y, int ch, byte *screen, int screenwidth, byte *cr)
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
                    {
                        if (!vanilla)
                            *dest = 0;
                    }
                    else if (src != ' ')
                        *dest = cr[src];
                }
        }
}

static void HU_DrawTranslucentChar(int x, int y, int ch, byte *screen, int screenwidth, byte *cr)
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
                    {
                        if (!vanilla)
                            *dest = black40[*dest];
                    }
                    else if (src != ' ')
                        *dest = tinttab75[(cr[src] << 8) + *dest];
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
    byte            *tinttab = (automapactive ? tinttab75 : tinttab50);

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
    int             x = (mapwindow ? 17 : HU_ALTHUDMSGX);
    byte            *fb = (external ? mapscreen : screens[0]);
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

            althudtextfunc(x, SCREENHEIGHT - 30, fb, patch, (italics && letter != '_'
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
    { 'F',  '.',  -1 }, { 'F',  'J',  -1 }, { 'L',  '"',  -1 }, { 'L',  'T',  -1 },
    { 'P',  ',',  -1 }, { 'P',  '.',  -1 }, { 'T',  ',',  -1 }, { 'T',  '.',  -1 },
    { 'T',  'J',  -1 }, { 'Y',  ',',  -1 }, { 'Y',  '.',  -1 }, { '\0', '\0',  0 }
};

static void HUlib_DrawTextLine(hu_textline_t *l, bool external)
{
    int             x = l->x;
    int             y = l->y;
    unsigned char   prev1 = '\0';
    unsigned char   prev2 = '\0';
    byte            *fb1;
    byte            *fb2;
    byte            *tinttab1 = tinttab40;
    byte            *tinttab2 = tinttab75;
    const int       black = (nearestblack << 8);
    const int       len = l->len;
    int             screenwidth;
    int             screenarea;
    int             wrap = -1;

    if (external)
    {
        fb1 = mapscreen;
        fb2 = mapscreen;
        screenwidth = MAPWIDTH;
    }
    else
    {
        fb1 = screens[0];
        fb2 = screens[(r_screensize < r_screensize_max - 1 && !automapactive)];
        screenwidth = SCREENWIDTH;
    }

    screenarea = screenwidth * (y + hu_font[0]->height * 4 + 10);

    for (int i = 0; i < screenarea; i++)
        tempscreen[i] = PINK;

    if (!vid_widescreen && len > 40)
    {
        int         width = l->width;
        const int   maxwidth = screenwidth / 2 - (vanilla ? 0 : 20);

        if (width > maxwidth)
            for (wrap = len; wrap > 0; wrap--)
                if ((width -= M_CharacterWidth(l->l[wrap], '\0')) <= maxwidth && isbreak(l->l[wrap]))
                    break;
    }

    for (int i = 0; i < len; i++)
    {
        const unsigned char c = toupper(l->l[i]);

        if (c == '\n' || i == wrap)
        {
            x = l->x;
            y += hu_font[0]->height + 2;

            if (c == ' ')
                continue;
        }

        if (c == ' ')
            x += (vanilla ? 4 : (i > 0 && (prev1 == '.' || prev1 == '!' || prev1 == '?') ? 5 : 3));
        else if (c >= l->sc && c <= '_')
        {
            int j = c - l->sc;

            if (STCFNxxx)
            {
                // [BH] display lump from PWAD with shadow
                if (prev2 == '.' && prev1 == ' ' && c == '(')
                    x -= 2;

                V_DrawPatchToTempScreen(x, MAX(0, y - 1), l->f[j], (message_secret ? cr_gold : cr_none), screenwidth);
                x += SHORT(l->f[j]->width);
            }
            else
            {
                // [BH] have matching curly single and double quotes
                if (!i || prev1 == ' ')
                {
                    if (c == '"')
                        j = 64;
                    else if (c == '\'')
                        j = 65;
                }

                // [BH] apply kerning to certain character pairs
                if (prev2 == '.' && prev1 == ' ' && c == '(')
                    x -= 2;
                else
                    for (int k = 0; kern[k].char1; k++)
                        if (prev1 == kern[k].char1 && c == kern[k].char2)
                        {
                            x += kern[k].adjust;
                            break;
                        }

                // [BH] draw individual character
                HU_DrawChar(x, y - 1, j, tempscreen, screenwidth, (message_secret ? cr_gold : cr_none));

                x += (short)strlen(smallcharset[j]) / 10 - 1;
            }
        }

        prev2 = prev1;
        prev1 = c;
    }

    // [BH] draw underscores for IDBEHOLD cheat message
    if (idbehold && !STCFNxxx && s_STSTR_BEHOLD2 && !vanilla)
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

    for (int i = 0; i < screenarea; i++)
    {
        byte    source = tempscreen[i];
        byte    *dest = &fb1[i];

        if (!source)
            *dest = tinttab1[black + fb2[i]];
        else if (source != PINK)
            *dest = (r_hud_translucency ? tinttab2[(source << 8) + fb2[i]] : source);
    }
}

void HUlib_DrawAutomapTextLine(hu_textline_t *l, bool external)
{
    int             x = l->x;
    int             y = l->y;
    unsigned char   prev1 = '\0';
    unsigned char   prev2 = '\0';
    byte            *fb1;
    byte            *fb2;
    byte            *tinttab1 = tinttab40;
    byte            *tinttab2 = tinttab75;
    const int       black = (nearestblack << 8);
    char            s[513];
    const int       maxwidth = (external ? MAPWIDTH / 2 :
                        (r_screensize == r_screensize_max ? SCREENWIDTH / 2 : VANILLAWIDTH));
    int             len = l->len;
    int             screenwidth;
    int             screenarea;

    if (external)
    {
        fb1 = mapscreen;
        fb2 = mapscreen;
        screenwidth = MAPWIDTH;
        screenarea = MAPAREA;
    }
    else
    {
        fb1 = screens[0];
        fb2 = screens[(r_screensize < r_screensize_max - 1 && !automapactive)];
        screenwidth = SCREENWIDTH;
        screenarea = (r_screensize == r_screensize_max ? SCREENAREA : SCREENAREA - SBARHEIGHT * SCREENWIDTH);
    }

    for (int i = screenarea - screenwidth * 50; i < screenarea; i++)
        tempscreen[i] = PINK;

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
            x += (vanilla ? 8 : (i > 0 && (prev1 == '.' || prev1 == '!' || prev1 == '?') ? 10 : 6));
        else if (c != '\n' && c >= l->sc && c <= '_')
        {
            int j = c - l->sc;

            if (STCFNxxx)
            {
                if (prev2 == '.' && prev1 == ' ' && c == '(')
                    x -= 2;

                V_DrawPatchToTempScreen(x / 2, y / 2, l->f[j], (secretmap ? cr_gold : cr_none), screenwidth);
            }
            else
            {
                // [BH] have matching curly single and double quotes
                if (!i || prev1 == ' ')
                {
                    if (c == '"')
                        j = 64;
                    else if (c == '\'')
                        j = 65;
                }

                // [BH] apply kerning to certain character pairs
                if (prev2 == '.' && prev1 == ' ' && c == '(')
                    x -= 2;
                else
                    for (int k = 0; kern[k].char1; k++)
                        if (prev1 == kern[k].char1 && c == kern[k].char2)
                        {
                            x += kern[k].adjust * 2;
                            break;
                        }

                if (r_hud_translucency)
                    HU_DrawTranslucentChar(x / 2, y / 2, j, fb1, screenwidth, (secretmap ? cr_gold : cr_none));
                else
                    HU_DrawChar(x / 2, y / 2, j, fb1, screenwidth, (secretmap ? cr_gold : cr_none));
            }

            x += SHORT(l->f[c - l->sc]->width) * 2;
        }

        prev2 = prev1;
        prev1 = c;
    }

    if (STCFNxxx)
        for (int i = screenarea - screenwidth * 50; i < screenarea; i++)
        {
            byte    source = tempscreen[i];
            byte    *dest = &fb1[i];

            if (!source)
                *dest = tinttab1[black + fb2[i]];
            else if (source != PINK)
                *dest = (r_hud_translucency ? tinttab2[(source << 8) + fb2[i]] : source);
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
