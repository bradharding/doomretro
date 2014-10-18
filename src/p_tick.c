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

#include "doomstat.h"
#include "p_local.h"
#include "z_zone.h"

int     leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// Both the head and tail of the thinker list.
thinker_t       thinkercap;

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
    thinkercap.prev = thinkercap.next = &thinkercap;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker(thinker_t *thinker)
{
    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
}

//
// killough 11/98:
//
// Make currentthinker external, so that P_RemoveThinkerDelayed
// can adjust currentthinker when thinkers self-remove.

static thinker_t        *currentthinker;

void P_RemoveThinkerDelayed(thinker_t *thinker)
{
    // Remove from main thinker list
    thinker_t   *next = thinker->next;

    // Note that currentthinker is guaranteed to point to us,
    // and since we're freeing our memory, we had better change that. So
    // point it to thinker->prev, so the iterator will correctly move on to
    // thinker->prev->next = thinker->next
    (next->prev = currentthinker = thinker->prev)->next = next;

    Z_Free(thinker);
}

//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
// killough 4/25/98:
//
// Instead of marking the function with -1 value cast to a function pointer,
// set the function to P_RemoveThinkerDelayed(), so that later, it will be
// removed automatically as part of the thinker process.
//
void P_RemoveThinker(thinker_t *thinker)
{
    thinker->function.acv = (actionf_v)P_RemoveThinkerDelayed;
}

//
// P_RunThinkers
//
// killough 11/98:
//
// Rewritten to delete nodes implicitly, by making currentthinker
// external and using P_RemoveThinkerDelayed() implicitly.
//
void P_RunThinkers(void)
{
    for (currentthinker = thinkercap.next; currentthinker != &thinkercap;
         currentthinker = currentthinker->next)
        if (currentthinker->function.acp1)
            currentthinker->function.acp1(currentthinker);
}

//
// P_Ticker
//
void P_Ticker(void)
{
    int i;

    // run the tic
    if (paused)
        return;

    // pause if in menu and at least one tic has been run
    if (menuactive && players[consoleplayer].viewz != 1)
        return;

    if (gamestate == GS_LEVEL)
        for (i = 0; i < MAXPLAYERS; i++)
            if (playeringame[i])
                P_PlayerThink(&players[i]);

    P_RunThinkers();
    P_UpdateSpecials();
    P_RespawnSpecials();

    P_MapEnd();

    // for par times
    leveltime++;
}
