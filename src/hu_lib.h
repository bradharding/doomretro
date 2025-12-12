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

#pragma once

// We are referring to patches.
#include "r_defs.h"

// font stuff
#define HU_MAXLINELENGTH    512

#define HU_ALTHUDMSGX       (OVERLAYTEXTX - 1)
#define HU_ALTHUDMSGY       17

//
// Typedefs of widgets
//

// Text Line widget
//  (parent of Scrolling Text and Input Text widgets)
typedef struct
{
    // left-justified position of scrolling text window
    int             x, y;

    patch_t         **f;                            // font
    int             sc;                             // start character
    char            l[HU_MAXLINELENGTH + 1];        // line of text
    int             len;                            // current line length
    int             width;                          // pixel width of line

    // whether this line needs to be updated
    int             needsupdate;
} hu_textline_t;

// Scrolling Text window widget
//  (child of Text Line widget)
typedef struct
{
    hu_textline_t   l;                              // text line to draw

    // pointer to bool stating whether to update window
    bool            *on;
    bool            laston;                         // last value of *->on.
} hu_stext_t;

//
// Widget creation, access, and update routines
//

//
// textline code
//

void HUlib_InitTextLine(hu_textline_t *t, int x, int y, patch_t **f, int sc);

// returns success
bool HUlib_AddCharToTextLine(hu_textline_t *t, char ch);

// draws text line
void HUlib_DrawAutomapTextLine(hu_textline_t *l, bool external);
void HUlib_DrawAltAutomapTextLine(hu_textline_t *l, bool external);

// erases text line
void HUlib_EraseTextLine(hu_textline_t *l);

//
// Scrolling Text window widget routines
//

void HUlib_InitSText(hu_stext_t *s, int x, int y, patch_t **font, int startchar, bool *on);

void HUlib_AddMessageToSText(hu_stext_t *s, const char *msg);

// draws stext
void HUlib_DrawSText(hu_stext_t *s, bool external);

// erases all stext lines
void HUlib_EraseSText(hu_stext_t *s);

extern void (*althudtextfunc)(int, int, byte *, patch_t *, bool, int, int, int, const byte *);

extern bool s_STSTR_BEHOLD2;
extern byte tempscreen[MAXSCREENAREA];
