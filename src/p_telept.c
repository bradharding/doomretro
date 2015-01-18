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

#include "p_local.h"
#include "s_sound.h"

extern boolean  footclip;

//
// TELEPORTATION
//
boolean EV_Teleport(line_t *line, int side, mobj_t *thing)
{
    int         tag;
    thinker_t   *thinker;

    // Don't teleport missiles.
    // Don't teleport if hit back of line, so you can get out of teleporter.
    if (side || (thing->flags & MF_MISSILE))
        return false;

    tag = line->tag;

    for (thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
    {
        mobj_t  *m;

        if (thinker->function.acp1 == (actionf_p1)P_MobjThinker
            && (m = (mobj_t *)thinker)->type == MT_TELEPORTMAN
            && m->subsector->sector->tag == tag)
        {
            fixed_t     oldx = thing->x;
            fixed_t     oldy = thing->y;
            fixed_t     oldz = thing->z;
            player_t    *player = thing->player;

            if (P_TeleportMove(thing, m->x, m->y, m->z, false)) // killough 8/9/98
            {
                mobj_t          *fog;
                unsigned int    an;

                thing->z = thing->floorz;

                if (player)
                    player->viewz = thing->z + player->viewheight;

                // spawn teleport fog at source and destination
                fog = P_SpawnMobj(oldx, oldy, oldz, MT_TFOG);
                fog->angle = thing->angle;
                S_StartSound(fog, sfx_telept);
                an = (m->angle >> ANGLETOFINESHIFT);
                fog = P_SpawnMobj(m->x + 20 * finecosine[an],
                    m->y + 20 * finesine[an], thing->z, MT_TFOG);
                fog->angle = m->angle;

                // emit sound, where?
                S_StartSound(fog, sfx_telept);

                if (player)
                {
                    int i;

                    // [BH] teleport can be drawn on automap now
                    for (i = 0; i < line->backsector->linecount; i++)
                        line->backsector->lines[i]->flags |= ML_TELEPORTTRIGGERED;

                    // don't move for a bit
                    thing->reactiontime = 18;
                }

                thing->angle = m->angle;

                if (footclip)
                    if (isliquid[thing->subsector->sector->floorpic] && !(thing->flags2 & MF2_NOFOOTCLIP))
                        thing->flags2 |= MF2_FEETARECLIPPED;
                    else if (thing->flags2 & MF2_FEETARECLIPPED)
                        thing->flags2 &= ~MF2_FEETARECLIPPED;

                thing->momx = thing->momy = thing->momz = 0;
                return true;
            }
        }
    }

    return false;
}
