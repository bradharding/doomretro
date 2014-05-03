/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

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

#include "doomdef.h"
#include "doomstat.h"

#include "s_sound.h"

#include "p_local.h"


// Data.
#include "sounds.h"

// State.
#include "r_state.h"



//
// TELEPORTATION
//
int EV_Teleport(line_t *line, int side, mobj_t *thing)
{
    int          i;
    int          tag;
    mobj_t       *m;
    mobj_t       *fog;
    unsigned int an;
    thinker_t    *thinker;
    sector_t     *sector;
    fixed_t      oldx;
    fixed_t      oldy;
    fixed_t      oldz;
    player_t     *player;

    // don't teleport missiles
    if (thing->flags & MF_MISSILE)
        return 0;

    // Don't teleport if hit back of line,
    //  so you can get out of teleporter.
    if (side == 1)
        return 0;


    tag = line->tag;
    player = thing->player;
    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].tag == tag)
        {
            thinker = thinkercap.next;
            for (thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
            {
                // not a mobj
                if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
                    continue;

                m = (mobj_t *)thinker;

                // not a teleportman
                if (m->type != MT_TELEPORTMAN)
                    continue;

                sector = m->subsector->sector;
                // wrong sector
                if (sector-sectors != i)
                    continue;

                oldx = thing->x;
                oldy = thing->y;
                oldz = thing->z;

                if (player && player->mo != thing)
                    player = NULL;

                if (!P_TeleportMove(thing, m->x, m->y, m->z))
                    return 0;

                thing->z = thing->floorz;

                if (player)
                    player->viewz = thing->z + player->viewheight;

                // spawn teleport fog at source and destination
                fog = P_SpawnMobj(oldx, oldy, oldz, MT_TFOG);
                fog->angle = thing->angle;
                S_StartSound(fog, sfx_telept);
                an = m->angle >> ANGLETOFINESHIFT;
                fog = P_SpawnMobj(m->x + 20 * finecosine[an], m->y + 20 * finesine[an],
                                  thing->z, MT_TFOG);
                fog->angle = m->angle;

                // emit sound, where?
                S_StartSound(fog, sfx_telept);

                // don't move for a bit
                if (player)
                    thing->reactiontime = 18;

                thing->angle = m->angle;
                thing->momx = thing->momy = thing->momz = 0;
                return 1;
            }
        }
    }
    return 0;
}
