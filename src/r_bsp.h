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

#if !defined(__R_BSP_H__)
#define __R_BSP_H__

extern seg_t            *curline;
extern side_t           *sidedef;
extern line_t           *linedef;
extern sector_t         *frontsector;
extern sector_t         *backsector;

extern drawseg_t        *drawsegs;
extern unsigned int     maxdrawsegs;

extern byte             *solidcol;

extern drawseg_t        *ds_p;

// BSP?
void R_InitClipSegs(void);
void R_ClearClipSegs(void);
void R_ClearDrawSegs(void);

void R_RenderBSPNode(int bspnum);

// killough 4/13/98: fake floors/ceilings for deep water / fake ceilings:
sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec, int *floorlightlevel,
    int *ceilinglightlevel, dboolean back);

#endif
