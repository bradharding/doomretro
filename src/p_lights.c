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

#include "doomstat.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "z_zone.h"

//
// FIRELIGHT FLICKER
//

//
// T_FireFlicker
//
void T_FireFlicker(fireflicker_t *flick)
{
    if (freeze)
        return;

    if (--flick->count)
        return;

    flick->sector->lightlevel = MAX(flick->minlight, flick->maxlight - (M_Random() & 3) * 16);

    flick->count = 4;
}

//
// P_SpawnFireFlicker
//
void P_SpawnFireFlicker(sector_t *sector)
{
    fireflicker_t   *flick = Z_Calloc(1, sizeof(*flick), PU_LEVSPEC, NULL);

    P_AddThinker(&flick->thinker);

    flick->thinker.function = T_FireFlicker;
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
    if (freeze)
        return;

    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->maxlight)
    {
        flash->sector->lightlevel = flash->minlight;
        flash->count = (M_Random() & flash->mintime) + 1;
    }
    else
    {
        flash->sector->lightlevel = flash->maxlight;
        flash->count = (M_Random() & flash->maxtime) + 1;
    }
}

//
// P_SpawnLightFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnLightFlash(sector_t *sector)
{
    lightflash_t    *flash = Z_Calloc(1, sizeof(*flash), PU_LEVSPEC, NULL);

    P_AddThinker(&flash->thinker);

    flash->thinker.function = T_LightFlash;
    flash->sector = sector;
    flash->maxlight = sector->lightlevel;

    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
    flash->maxtime = 63;
    flash->mintime = 7;
    flash->count = (M_Random() & flash->maxtime) + 1;
}

//
// STROBE LIGHT FLASHING
//

//
// T_StrobeFlash
//
void T_StrobeFlash(strobe_t *flash)
{
    if (freeze)
        return;

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
void P_SpawnStrobeFlash(sector_t *sector, int fastOrSlow, dboolean inSync)
{
    strobe_t    *flash = Z_Calloc(1, sizeof(*flash), PU_LEVSPEC, NULL);

    P_AddThinker(&flash->thinker);

    flash->sector = sector;
    flash->darktime = fastOrSlow;
    flash->brighttime = STROBEBRIGHT;
    flash->thinker.function = T_StrobeFlash;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

    if (flash->minlight == flash->maxlight)
        flash->minlight = 0;

    flash->count = (inSync ? 1 : (M_Random() & 7) + 1);
}

//
// Start strobing lights (usually from a trigger)
//
dboolean EV_StartLightStrobing(line_t *line)
{
    int secnum = -1;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sector_t    *sec = &sectors[secnum];

        if (P_SectorActive(lighting_special, sec))
            continue;

        P_SpawnStrobeFlash(sec, SLOWDARK, false);
    }

    return true;
}

//
// TURN LINE'S TAG LIGHTS OFF
//
dboolean EV_TurnTagLightsOff(line_t *line)
{
    int i;

    // search sectors for those with same tag as activating line

    // killough 10/98: replaced inefficient search with fast search
    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
    {
        sector_t    *temp;
        sector_t    *sector = sectors + i;
        int         j;
        int         min = sector->lightlevel;

        // find min neighbor light level
        for (j = 0; j < sector->linecount; j++)
            if ((temp = getNextSector(sector->lines[j], sector)) && temp->lightlevel < min)
                min = temp->lightlevel;

        sector->lightlevel = min;
    }

    return true;
}

//
// TURN LINE'S TAG LIGHTS ON
//
dboolean EV_LightTurnOn(line_t *line, int bright)
{
    int i;

    // search all sectors for ones with same tag as activating line

    // killough 10/98: replace inefficient search with fast search
    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
    {
        sector_t    *temp;
        sector_t    *sector = sectors + i;
        int         tbright = bright;       // jff 5/17/98 search for maximum PER sector

        // bright = 0 means to search for highest light level surrounding sector
        if (!bright)
        {
            int j;

            for (j = 0; j < sector->linecount; j++)
                if ((temp = getNextSector(sector->lines[j], sector)) && temp->lightlevel > tbright)
                    tbright = temp->lightlevel;
        }

        sector->lightlevel = tbright;
    }

    return true;
}

//
// Spawn glowing light
//
void T_Glow(glow_t *g)
{
    if (freeze)
        return;

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
    glow_t  *glow = Z_Calloc(1, sizeof(*glow), PU_LEVSPEC, NULL);

    P_AddThinker(&glow->thinker);

    glow->sector = sector;
    glow->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
    glow->maxlight = sector->lightlevel;
    glow->thinker.function = T_Glow;
    glow->direction = -1;
}

// killough 10/98:
//
// EV_LightTurnOnPartway()
//
// Turn sectors tagged to line lights on to specified or max neighbor level
//
// Passed the activating line, and a light level fraction between 0 and 1.
// Sets the light to min on 0, max on 1, and interpolates in-between.
// Used for doors with gradual lighting effects.
//
void EV_LightTurnOnPartway(line_t *line, fixed_t level)
{
    int i;

    level = BETWEEN(0, level, FRACUNIT);        // clip at extremes

    // search all sectors for ones with same tag as activating line
    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
    {
        sector_t    *temp;
        sector_t    *sector = sectors + i;
        int         j;
        int         bright = 0;
        int         min = sector->lightlevel;

        for (j = 0; j < sector->linecount; j++)
            if ((temp = getNextSector(sector->lines[j], sector)))
            {
                if (temp->lightlevel > bright)
                    bright = temp->lightlevel;

                if (temp->lightlevel < min)
                    min = temp->lightlevel;
            }

        // Set level in-between extremes
        sector->lightlevel = (level * bright + (FRACUNIT - level) * min) >> FRACBITS;
    }
}

//
// EV_LightByAdjacentSectors()
//
// [BH] similar to EV_LightTurnOnPartway(), but instead of using a line tag, looks at adjacent
//  sectors of the sector itself.
void EV_LightByAdjacentSectors(sector_t *sector, fixed_t level)
{
    sector_t    *temp;
    int         i;
    int         bright = 0;
    int         min = MAX(0, sector->lightlevel - 4);

    level = BETWEEN(0, level, FRACUNIT);        // clip at extremes

    for (i = 0; i < sector->linecount; i++)
        if ((temp = getNextSector(sector->lines[i], sector)))
        {
            if (temp->lightlevel > bright)
                bright = temp->lightlevel;

            if (temp->lightlevel < min)
                min = temp->lightlevel;
        }

    sector->lightlevel = (level * bright + (FRACUNIT - level) * min) >> FRACBITS;
}
