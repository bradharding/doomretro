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

#if !defined(__R_STATE_H__)
#define __R_STATE_H__

// Need data structure definitions.
#include "d_player.h"

//
// Refresh internal data structures,
//  for rendering.
//

// needed for texture pegging
extern fixed_t          *textureheight;

extern byte             **texturefullbright;
extern dboolean         *nobrightmap;

// needed for pre rendering (fracs)
extern fixed_t          *spritewidth;
extern fixed_t          *spriteheight;

extern fixed_t          *spriteoffset;
extern fixed_t          *spritetopoffset;
extern fixed_t          *newspriteoffset;
extern fixed_t          *newspritetopoffset;

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
extern spritedef_t      *sprites;

extern int              numvertexes;
extern vertex_t         *vertexes;

extern int              numsegs;
extern seg_t            *segs;

extern int              numsectors;
extern sector_t         *sectors;

extern int              numliquid;
extern int              numdamaging;

extern int              numsubsectors;
extern subsector_t      *subsectors;

extern int              numnodes;
extern node_t           *nodes;

extern int              numlines;
extern line_t           *lines;

extern int              numsides;
extern side_t           *sides;

extern int              numtextures;

extern int              numthings;

extern int              numdecorations;

typedef enum mapformat_e
{
    DOOMBSP,
    DEEPBSP,
    ZDBSPX
} mapformat_t;

extern mapformat_t      mapformat;

extern dboolean         boomlinespecials;
extern dboolean         blockmaprecreated;
//
// POV data.
//
extern fixed_t          viewx;
extern fixed_t          viewy;
extern fixed_t          viewz;

extern angle_t          viewangle;
extern player_t         *viewplayer;

extern angle_t          clipangle;

extern int              viewangletox[FINEANGLES / 2];
extern angle_t          xtoviewangle[SCREENWIDTH + 1];

extern angle_t          rw_normalangle;

extern visplane_t       *floorplane;
extern visplane_t       *ceilingplane;

#endif
