/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#ifndef __P_FIX__
#define __P_FIX__

#define DEFAULT 0x7fff
#define REMOVE 0

#define E2M2 (gamemission == doom && gameepisode == 2 && gamemap == 2 && canmodify)
#define MAP12 (gamemission == doom2 && gamemap == 12 && canmodify)

typedef struct
{
    int mission;
    int epsiode;
    int map;
    int vertex;
    int oldx;
    int oldy;
    int newx;
    int newy;
} vertexfix_t;

extern vertexfix_t vertexfix[];

typedef struct
{
    int mission;
    int epsiode;
    int map;
    int linedef;
    int side;
    char *toptexture;
    char *middletexture;
    char *bottomtexture;
    short offset;
    short rowoffset;
    int flags;
    int special;
    int tag;
} linefix_t;

extern linefix_t linefix[];

typedef struct
{
    int mission;
    int epsiode;
    int map;
    int sector;
    char *floorpic;
    char *ceilingpic;
    int floorheight;
    int ceilingheight;
    int special;
    int tag;
} sectorfix_t;

extern sectorfix_t sectorfix[];

typedef struct
{
    int mission;
    int epsiode;
    int map;
    int thing;
    int type;
    int oldx;
    int oldy;
    int newx;
    int newy;
    int angle;
    int options;
} thingfix_t;

extern thingfix_t thingfix[];

#endif