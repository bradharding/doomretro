/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"

void P_CalcHeight(player_t *player);

//
// TELEPORTATION
//
dboolean EV_Teleport(line_t *line, int side, mobj_t *thing)
{
    thinker_t   *thinker;
    int         i;

    // Don't teleport missiles.
    // Don't teleport if hit back of line, so you can get out of teleporter.
    if (side || (thing->flags & MF_MISSILE))
        return false;

    // killough 1/31/98: improve performance by using
    // P_FindSectorFromLineTag instead of simple linear search.
    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
        for (thinker = thinkerclasscap[th_mobj].cnext; thinker != &thinkerclasscap[th_mobj];
            thinker = thinker->cnext)
        {
            mobj_t  *m;

            if ((m = (mobj_t *)thinker)->type == MT_TELEPORTMAN
                && m->subsector->sector - sectors == i)
            {
                fixed_t     oldx = thing->x;
                fixed_t     oldy = thing->y;
                fixed_t     oldz = thing->z;
                player_t    *player = thing->player;

                // killough 5/12/98: exclude voodoo dolls:
                if (player && player->mo != thing)
                    player = NULL;

                if (P_TeleportMove(thing, m->x, m->y, m->z, false))     // killough 8/9/98
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
                        // [BH] teleport can be drawn on automap
                        if (line->backsector)
                        {
                            int j;

                            for (j = 0; j < line->backsector->linecount; j++)
                                line->backsector->lines[j]->flags |= ML_TELEPORTTRIGGERED;
                        }

                        // don't move for a bit
                        thing->reactiontime = 18;

                        player->psprites[ps_weapon].sx = 0;
                        player->psprites[ps_weapon].sy = WEAPONTOP;

                        player->momx = player->momy = 0;
                    }

                    thing->angle = m->angle;

                    thing->momx = thing->momy = thing->momz = 0;

                    return true;
                }
            }
        }

    return false;
}

//
// Silent TELEPORTATION, by Lee Killough
// Primarily for rooms-over-rooms etc.
//
dboolean EV_SilentTeleport(line_t *line, int side, mobj_t *thing)
{
    int         i;
    mobj_t      *m;
    thinker_t   *th;

    // don't teleport missiles
    // Don't teleport if hit back of line,
    // so you can get out of teleporter.
    if (side || (thing->flags & MF_MISSILE))
        return false;

    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
        for (th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
            if ((m = (mobj_t *)th)->type == MT_TELEPORTMAN
                && m->subsector->sector - sectors == i)
            {
                // Height of thing above ground, in case of mid-air teleports:
                fixed_t         z = thing->z - thing->floorz;

                // Get the angle between the exit thing and source linedef.
                // Rotate 90 degrees, so that walking perpendicularly across
                // teleporter linedef causes thing to exit in the direction
                // indicated by the exit thing.
                angle_t         angle = R_PointToAngle2(0, 0, line->dx, line->dy) - m->angle + ANG90;

                // Sine, cosine of angle adjustment
                fixed_t         s = finesine[angle >> ANGLETOFINESHIFT];
                fixed_t         c = finecosine[angle >> ANGLETOFINESHIFT];

                // Momentum of thing crossing teleporter linedef
                fixed_t         momx = thing->momx;
                fixed_t         momy = thing->momy;

                // Whether this is a player, and if so, a pointer to its player_t
                player_t        *player = thing->player;

                // Attempt to teleport, aborting if blocked
                if (!P_TeleportMove(thing, m->x, m->y, m->z, false))    // killough 8/9/98
                    return 0;

                // Rotate thing according to difference in angles
                thing->angle += angle;

                // Adjust z position to be same height above ground as before
                thing->z = z + thing->floorz;

                // Rotate thing's momentum to come out of exit just like it entered
                thing->momx = FixedMul(momx, c) - FixedMul(momy, s);
                thing->momy = FixedMul(momy, c) + FixedMul(momx, s);

                // Adjust player's view, in case there has been a height change
                // Voodoo dolls are excluded by making sure player->mo == thing.
                if (player && player->mo == thing)
                {
                    // Save the current deltaviewheight, used in stepping
                    fixed_t     deltaviewheight = player->deltaviewheight;

                    // Clear deltaviewheight, since we don't want any changes
                    player->deltaviewheight = 0;

                    // Set player's view according to the newly set parameters
                    P_CalcHeight(player);

                    // Reset the delta to have the same dynamics as before
                    player->deltaviewheight = deltaviewheight;
                }
                return true;
            }
    return false;
}

//
// Silent linedef-based TELEPORTATION, by Lee Killough
// Primarily for rooms-over-rooms etc.
// This is the complete player-preserving kind of teleporter.
// It has advantages over the teleporter with thing exits.
//

// maximum fixed_t units to move object to avoid hiccups
#define FUDGEFACTOR 10

dboolean EV_SilentLineTeleport(line_t *line, int side, mobj_t *thing, dboolean reverse)
{
    int         i;
    line_t      *l;

    if (side || (thing->flags & MF_MISSILE))
        return false;

    for (i = -1; (i = P_FindLineFromLineTag(line, i)) >= 0;)
        if ((l = lines + i) != line && l->backsector)
        {
            // Get the thing's position along the source linedef
            fixed_t     pos = ABS(line->dx) > ABS(line->dy) ?
                FixedDiv(thing->x - line->v1->x, line->dx) :
                FixedDiv(thing->y - line->v1->y, line->dy);

            // Get the angle between the two linedefs, for rotating
            // orientation and momentum. Rotate 180 degrees, and flip
            // the position across the exit linedef, if reversed.
            angle_t     angle = (reverse ? (pos = FRACUNIT - pos), 0 : ANG180)
                + R_PointToAngle2(0, 0, l->dx, l->dy) - R_PointToAngle2(0, 0, line->dx, line->dy);

            // Interpolate position across the exit linedef
            fixed_t     x = l->v2->x - FixedMul(pos, l->dx);
            fixed_t     y = l->v2->y - FixedMul(pos, l->dy);

            // Sine, cosine of angle adjustment
            fixed_t     s = finesine[angle >> ANGLETOFINESHIFT];
            fixed_t     c = finecosine[angle >> ANGLETOFINESHIFT];

            // Maximum distance thing can be moved away from interpolated
            // exit, to ensure that it is on the correct side of exit linedef
            int         fudge = FUDGEFACTOR;

            // Whether this is a player, and if so, a pointer to its player_t.
            // Voodoo dolls are excluded by making sure thing->player->mo==thing.
            player_t    *player = (thing->player && thing->player->mo == thing ? thing->player : NULL);

            // Whether walking towards first side of exit linedef steps down
            dboolean    stepdown = (l->frontsector->floorheight < l->backsector->floorheight);

            // Height of thing above ground
            fixed_t z = thing->z - thing->floorz;

            // Side to exit the linedef on positionally.
            //
            // Notes:
            //
            // This flag concerns exit position, not momentum. Due to
            // roundoff error, the thing can land on either the left or
            // the right side of the exit linedef, and steps must be
            // taken to make sure it does not end up on the wrong side.
            //
            // Exit momentum is always towards side 1 in a reversed
            // teleporter, and always towards side 0 otherwise.
            //
            // Exiting positionally on side 1 is always safe, as far
            // as avoiding oscillations and stuck-in-wall problems,
            // but may not be optimum for non-reversed teleporters.
            //
            // Exiting on side 0 can cause oscillations if momentum
            // is towards side 1, as it is with reversed teleporters.
            //
            // Exiting on side 1 slightly improves player viewing
            // when going down a step on a non-reversed teleporter.
            int side = (reverse || (player && stepdown));

            // Make sure we are on correct side of exit linedef.
            while (P_PointOnLineSide(x, y, l) != side && --fudge >= 0)
                if (ABS(l->dx) > ABS(l->dy))
                    y -= ((l->dx < 0) != side ? -1 : 1);
                else
                    x += ((l->dy < 0) != side ? -1 : 1);

            // Attempt to teleport, aborting if blocked
            if (!P_TeleportMove(thing, x, y, z, false)) // killough 8/9/98
                return 0;

            // Adjust z position to be same height above ground as before.
            // Ground level at the exit is measured as the higher of the
            // two floor heights at the exit linedef.
            thing->z = z + sides[l->sidenum[stepdown]].sector->floorheight;

            // Rotate thing's orientation according to difference in linedef angles
            thing->angle += angle;

            // Momentum of thing crossing teleporter linedef
            x = thing->momx;
            y = thing->momy;

            // Rotate thing's momentum to come out of exit just like it entered
            thing->momx = FixedMul(x, c) - FixedMul(y, s);
            thing->momy = FixedMul(y, c) + FixedMul(x, s);

            // Adjust a player's view, in case there has been a height change
            if (player)
            {
                // Save the current deltaviewheight, used in stepping
                fixed_t deltaviewheight = player->deltaviewheight;

                // Clear deltaviewheight, since we don't want any changes now
                player->deltaviewheight = 0;

                // Set player's view according to the newly set parameters
                P_CalcHeight(player);

                // Reset the delta to have the same dynamics as before
                player->deltaviewheight = deltaviewheight;
            }

            return true;
        }
    return false;
}
