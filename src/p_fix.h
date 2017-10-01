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

#if !defined(__P_FIX_H__)
#define __P_FIX_H__

#define DEFAULT 0x7FFF
#define REMOVE  0

#define E2M2    (gamemission == doom && gameepisode == 2 && gamemap == 2 && canmodify)
#define MAP12   (gamemission == doom2 && gamemap == 12 && canmodify)

typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     vertex;
    int     oldx, oldy;
    int     newx, newy;
} vertexfix_t;

extern vertexfix_t  vertexfix[];

typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     linedef;
    int     side;
    char    *toptexture;
    char    *middletexture;
    char    *bottomtexture;
    short   offset;
    short   rowoffset;
    int     flags;
    int     special;
    int     tag;
} linefix_t;

extern linefix_t    linefix[];

typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     sector;
    char    *floorpic;
    char    *ceilingpic;
    int     floorheight;
    int     ceilingheight;
    int     special;
    int     oldtag;
    int     newtag;
} sectorfix_t;

extern sectorfix_t  sectorfix[];

typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     thing;
    int     type;
    int     oldx, oldy;
    int     newx, newy;
    int     angle;
    int     options;
} thingfix_t;

extern thingfix_t   thingfix[];

#endif
