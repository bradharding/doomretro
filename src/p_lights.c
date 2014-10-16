/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 by Brad Harding. All rights reserved.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.

  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#include "z_zone.h"
#include "m_random.h"
#include "p_local.h"

//
// FIRELIGHT FLICKER
//

//
// T_FireFlicker
//
void T_FireFlicker(fireflicker_t *flick)
{
    if (--flick->count)
        return;

    flick->sector->lightlevel = MAX(flick->minlight, flick->maxlight - (P_Random() & 3) * 16);

    flick->count = 4;
}

//
// P_SpawnFireFlicker
//
void P_SpawnFireFlicker(sector_t *sector)
{
    fireflicker_t       *flick = (fireflicker_t *)Z_Malloc(sizeof(*flick), PU_LEVSPEC, 0);

    // Note that we are resetting sector attributes.
    // Nothing special about it during gameplay.
    sector->special = 0;

    P_AddThinker(&flick->thinker);

    flick->thinker.function.acp1 = (actionf_p1)T_FireFlicker;
    flick->sector = sector;
    flick->maxlight = sector->lightlevel;
    flick->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel) + 16;
    flick->count = 4;
}

//
// BROKEN LIGHT FLASHING
//

//
// T_LightFlash
// Do flashing lights.
//
void T_LightFlash(lightflash_t *flash)
{
    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->maxlight)
    {
        flash->sector->lightlevel = flash->minlight;
        flash->count = (P_Random() & flash->mintime) + 1;
    }
    else
    {
        flash->sector->lightlevel = flash->maxlight;
        flash->count = (P_Random() & flash->maxtime) + 1;
    }
}

//
// P_SpawnLightFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnLightFlash(sector_t *sector)
{
    lightflash_t        *flash = (lightflash_t *)Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);

    // nothing special about it during gameplay
    sector->special = 0;

    P_AddThinker(&flash->thinker);

    flash->thinker.function.acp1 = (actionf_p1)T_LightFlash;
    flash->sector = sector;
    flash->maxlight = sector->lightlevel;

    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
    flash->maxtime = 64;
    flash->mintime = 7;
    flash->count = (P_Random() & flash->maxtime) + 1;
}

//
// STROBE LIGHT FLASHING
//

//
// T_StrobeFlash
//
void T_StrobeFlash(strobe_t *flash)
{
    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->minlight)
    {
        flash->sector->lightlevel = flash->maxlight;
        flash->count = flash->brighttime;
    }
    else
    {
        flash->sector->lightlevel = flash->minlight;
        flash->count = flash->darktime;
    }
}

//
// P_SpawnStrobeFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnStrobeFlash(sector_t *sector, int fastOrSlow, int inSync)
{
    strobe_t    *flash = (strobe_t *)Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);

    P_AddThinker(&flash->thinker);

    flash->sector = sector;
    flash->darktime = fastOrSlow;
    flash->brighttime = STROBEBRIGHT;
    flash->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

    if (flash->minlight == flash->maxlight)
        flash->minlight = 0;

    // nothing special about it during gameplay
    sector->special = 0;

    flash->count = (inSync ? 1 : (P_Random() & 7) + 1);
}

//
// Start strobing lights (usually from a trigger)
//
int EV_StartLightStrobing(line_t *line)
{
    int         secnum = -1;
    sector_t    *sec;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
            continue;

        P_SpawnStrobeFlash(sec, SLOWDARK, 0);
    }
    return 1;
}

//
// TURN LINE'S TAG LIGHTS OFF
//
int EV_TurnTagLightsOff(line_t *line)
{
    int i;

    // search sectors for those with same tag as activating line

    // killough 10/98: replaced inefficient search with fast search
    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
    {
        sector_t        *temp;
        sector_t        *sector = sectors + i;
        int             j;
        int             min = sector->lightlevel;

        // find min neighbor light level
        for (j = 0; j < sector->linecount; j++)
            if ((temp = getNextSector(sector->lines[j], sector)) && temp->lightlevel < min)
                min = temp->lightlevel;
        sector->lightlevel = min;
    }
    return 1;
}

//
// TURN LINE'S TAG LIGHTS ON
//
int EV_LightTurnOn(line_t *line, int bright)
{
    int i;

    // search all sectors for ones with same tag as activating line

    // killough 10/98: replace inefficient search with fast search
    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
    {
        sector_t        *temp;
        sector_t        *sector = sectors + i;
        int             j;
        int             tbright = bright;       // jff 5/17/98 search for maximum PER sector

        // bright = 0 means to search for highest light level surrounding sector
        if (!bright)
            for (j = 0; j < sector->linecount; j++)
                if ((temp = getNextSector(sector->lines[j], sector)) && temp->lightlevel > tbright)
                    tbright = temp->lightlevel;
        sector->lightlevel = tbright;
    }
    return 1;
}

//
// Spawn glowing light
//
void T_Glow(glow_t *g)
{
    switch (g->direction)
    {
        case -1:
            // DOWN
            g->sector->lightlevel -= GLOWSPEED;
            if (g->sector->lightlevel <= g->minlight)
            {
                g->sector->lightlevel += GLOWSPEED;
                g->direction = 1;
            }
            break;

        case 1:
            // UP
            g->sector->lightlevel += GLOWSPEED;
            if (g->sector->lightlevel >= g->maxlight)
            {
                g->sector->lightlevel -= GLOWSPEED;
                g->direction = -1;
            }
            break;
    }
}

void P_SpawnGlowingLight(sector_t *sector)
{
    glow_t *glow = (glow_t *)Z_Malloc(sizeof(*glow), PU_LEVSPEC, 0);

    P_AddThinker(&glow->thinker);

    glow->sector = sector;
    glow->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
    glow->maxlight = sector->lightlevel;
    glow->thinker.function.acp1 = (actionf_p1)T_Glow;
    glow->direction = -1;

    sector->special = 0;
}
