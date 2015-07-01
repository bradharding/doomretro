/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#if !defined(__R_STATE__)
#define __R_STATE__

// Need data structure definitions.
#include "d_player.h"
#include "r_data.h"

//
// Refresh internal data structures,
//  for rendering.
//

// needed for texture pegging
extern fixed_t          *textureheight;

extern byte             **texturefullbright;

// needed for pre rendering (fracs)
extern fixed_t          *spritewidth;
extern fixed_t          *spriteheight;

extern fixed_t          *spriteoffset;
extern fixed_t          *spritetopoffset;

extern lighttable_t     **colormaps;    // killough 3/20/98, 4/4/98
extern lighttable_t     *fullcolormap;  // killough 3/20/98

extern int              viewwidth;
extern int              scaledviewwidth;
extern int              viewheight;

extern int              firstflat;

// for global animation
extern int              *flattranslation;
extern int              *texturetranslation;

extern byte             **flatfullbright;

// Sprite....
extern int              firstspritelump;
extern int              lastspritelump;
extern int              numspritelumps;

//
// Lookup tables for map data.
//
extern int              numsprites;
extern spritedef_t      *sprites;

extern int              numvertexes;
extern vertex_t         *vertexes;

extern int              numsegs;
extern seg_t            *segs;

extern int              numsectors;
extern sector_t         *sectors;

extern int              numsubsectors;
extern subsector_t      *subsectors;

extern int              numnodes;
extern node_t           *nodes;

extern int              numlines;
extern line_t           *lines;

extern int              numsides;
extern side_t           *sides;

extern int              numthings;

//
// POV data.
//
extern fixed_t          viewx;
extern fixed_t          viewy;
extern fixed_t          viewz;

extern angle_t          viewangle;
extern player_t         *viewplayer;

// ?
extern angle_t          clipangle;

extern int              viewangletox[FINEANGLES / 2];
extern angle_t          xtoviewangle[SCREENWIDTH + 1];

extern angle_t          rw_normalangle;

extern int              rw_angle1;

extern visplane_t       *floorplane;
extern visplane_t       *ceilingplane;

#endif
