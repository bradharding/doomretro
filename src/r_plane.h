/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

// killough 10/98: special mask indicates sky flat comes from sidedef
#define PL_SKYFLAT      0x40000000
#define PL_FLATMAPPING  0xC0000000

// Visplane related.
extern int      *lastopening;
extern int      floorclip[MAXWIDTH];
extern int      ceilingclip[MAXWIDTH];
extern fixed_t  *yslope;
extern fixed_t  yslopes[PITCHES][MAXHEIGHT];
extern int      openings[MAXOPENINGS];

void R_ClearPlanes(void);
void R_DrawPlanes(void);
visplane_t *R_FindPlane(fixed_t height, const int picnum, int lightlevel,
    const fixed_t x, const fixed_t y, const int colormap, const angle_t angle);
visplane_t *R_CheckPlane(visplane_t *pl, const int start, const int stop);
visplane_t *R_DupPlane(const visplane_t *pl, const int start, const int stop);
void R_InitDistortedFlats(void);
