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

#include "c_console.h"
#include "doomstat.h"
#include "p_local.h"
#include "p_tick.h"
#include "z_zone.h"

int     leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// killough 8/29/98: we maintain several separate threads, each containing
// a special class of thinkers, to allow more efficient searches.
thinker_t       thinkerclasscap[th_all + 1];

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
    int i;

    // killough 8/29/98: initialize threaded lists
    for (i = 0; i < NUMTHCLASS; i++)
        thinkerclasscap[i].cprev = thinkerclasscap[i].cnext = &thinkerclasscap[i];

    thinkercap.prev = thinkercap.next = &thinkercap;
}

//
// killough 8/29/98:
//
// We maintain separate threads of friends and enemies, to permit more
// efficient searches.
//
void P_UpdateThinker(thinker_t *thinker)
{
    thinker_t   *th;

    // find the class the thinker belongs to
    int class = (thinker->function.acp1 == (actionf_p1)P_RemoveThinkerDelayed ? th_delete :
        (thinker->function.acp1 == (actionf_p1)P_MobjThinker ? th_mobj : th_misc));

    // Remove from current thread, if in one
    if ((th = thinker->cnext))
        (th->cprev = thinker->cprev)->cnext = th;

    // Add to appropriate thread
    th = &thinkerclasscap[class];
    th->cprev->cnext = thinker;
    thinker->cnext = th;
    thinker->cprev = th->cprev;
    th->cprev = thinker;
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

    // killough 8/29/98: set sentinel pointers, and then add to appropriate list
    thinker->cnext = thinker->cprev = NULL;
    P_UpdateThinker(thinker);
}

//
// killough 11/98:
//
// Make currentthinker external, so that P_RemoveThinkerDelayed
// can adjust currentthinker when thinkers self-remove.

static thinker_t        *currentthinker;

//
// P_RemoveThinkerDelayed()
//
// Called automatically as part of the thinker loop in P_RunThinkers(),
// on nodes which are pending deletion.
//
// If this thinker has no more pointers referencing it indirectly,
// remove it, and set currentthinker to one node preceeding it, so
// that the next step in P_RunThinkers() will get its successor.
//
void P_RemoveThinkerDelayed(thinker_t *thinker)
{
    thinker_t   *next = thinker->next;
    thinker_t   *th = thinker->cnext;

    // Remove from main thinker list
    // Note that currentthinker is guaranteed to point to us,
    // and since we're freeing our memory, we had better change that. So
    // point it to thinker->prev, so the iterator will correctly move on to
    // thinker->prev->next = thinker->next 
    (next->prev = currentthinker = thinker->prev)->next = next;

    // Remove from current thinker class list 
    (th->cprev = thinker->cprev)->cnext = th;

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
    thinker->function.acp1 = (actionf_p1)P_RemoveThinkerDelayed;

    P_UpdateThinker(thinker);
}

//
// P_RunThinkers
//
// killough 4/25/98:
//
// Fix deallocator to stop using "next" pointer after node has been freed
// (a Doom bug).
//
// Process each thinker. For thinkers which are marked deleted, we must
// load the "next" pointer prior to freeing the node. In Doom, the "next"
// pointer was loaded AFTER the thinker was freed, which could have caused
// crashes.
//
// But if we are not deleting the thinker, we should reload the "next"
// pointer after calling the function, in case additional thinkers are
// added at the end of the list.
//
// killough 11/98:
//
// Rewritten to delete nodes implicitly, by making currentthinker
// external and using P_RemoveThinkerDelayed() implicitly.
//
static void P_RunThinkers(void)
{
    currentthinker = thinkercap.next;

    while (currentthinker != &thinkercap)
    {
        if (currentthinker->function.acp1)
            currentthinker->function.acp1(currentthinker);
        currentthinker = currentthinker->next;
    }
}

//
// P_Ticker
//
void P_Ticker(void)
{
    // pause if in menu and at least one tic has been run
    if (paused || menuactive || consoleactive)
        return;

    P_PlayerThink(&players[0]);

    P_RunThinkers();
    P_UpdateSpecials();

    P_MapEnd();

    // for par times
    leveltime++;
}
