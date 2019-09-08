/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

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
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "c_console.h"
#include "doomstat.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

int             leveltime;
unsigned int    stat_time = 0;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// killough 8/29/98: we maintain several separate threads, each containing
// a special class of thinkers, to allow more efficient searches.
thinker_t       thinkers[th_all + 1];

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
    thinkers[th_mobj].cprev = thinkers[th_mobj].cnext = &thinkers[th_mobj];
    thinkers[th_misc].cprev = thinkers[th_misc].cnext = &thinkers[th_misc];
    thinkers[th_all].prev = thinkers[th_all].next = &thinkers[th_all];
}

//
// P_UpdateThinker
//
void P_UpdateThinker(thinker_t *thinker)
{
    thinker_t   *th = thinker->cnext;

    // Remove from current thread, if in one
    if (th)
        (th->cprev = thinker->cprev)->cnext = th;

    // Add to appropriate thread
    th = &thinkers[(thinker->function == P_MobjThinker ? th_mobj : th_misc)];
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
    thinkers[th_all].prev->next = thinker;
    thinker->next = &thinkers[th_all];
    thinker->prev = thinkers[th_all].prev;
    thinkers[th_all].prev = thinker;

    thinker->references = 0;    // killough 11/98: init reference counter to 0

    // killough 8/29/98: set sentinel pointers, and then add to appropriate list
    thinker->cnext = NULL;
    thinker->cprev = NULL;
    P_UpdateThinker(thinker);
}

//
// killough 11/98:
//
// Make currentthinker external, so that P_RemoveThinkerDelayed()
// can adjust currentthinker when thinkers self-remove.
static thinker_t    *currentthinker;

//
// P_RemoveThinkerDelayed()
//
// Called automatically as part of the thinker loop in P_RunThinkers(),
// on nodes which are pending deletion.
//
// If this thinker has no more pointers referencing it indirectly,
// remove it, and set currentthinker to one node preceding it, so
// that the next step in P_RunThinkers() will get its successor.
//
void P_RemoveThinkerDelayed(thinker_t *thinker)
{
    if (!thinker->references)
    {
        thinker_t   *next = thinker->next;
        thinker_t   *th = thinker->cnext;

        // Remove from main thinker list
        // Note that currentthinker is guaranteed to point to us,
        // and since we're freeing our memory, we had better change that. So
        // point it to thinker->prev, so the iterator will correctly move on to
        // thinker->prev->next = thinker->next
        (next->prev = thinker->prev)->next = next;

        // Remove from current thinker class list
        (th->cprev = currentthinker = thinker->cprev)->cnext = th;
        Z_Free(thinker);
    }
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
    thinker->function = P_RemoveThinkerDelayed;
}

//
// P_SetTarget
//
// This function is used to keep track of pointer references to mobj thinkers.
// In DOOM, objects such as lost souls could sometimes be removed despite
// their still being referenced. In BOOM, 'target' mobj fields were tested
// during each gametic, and any objects pointed to by them would be prevented
// from being removed. But this was incomplete, and was slow (every mobj was
// checked during every gametic). Now, we keep a count of the number of
// references, and delay removal until the count is 0.
//
void P_SetTarget(mobj_t **mop, mobj_t *targ)
{
    if (*mop)           // If there was a target already, decrease its refcount
        (*mop)->thinker.references--;

    if ((*mop = targ))  // Set new target and if non-NULL, increase its counter
        targ->thinker.references++;
}

//
// P_RunThinkers
//
// killough 4/25/98:
//
// Fix deallocator to stop using "next" pointer after node has been freed
// (a DOOM bug).
//
// Process each thinker. For thinkers which are marked deleted, we must
// load the "next" pointer prior to freeing the node. In DOOM, the "next"
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
    for (currentthinker = thinkers[th_mobj].cnext; currentthinker != &thinkers[th_mobj]; currentthinker = currentthinker->cnext)
        currentthinker->function(currentthinker);

    for (currentthinker = thinkers[th_misc].cnext; currentthinker != &thinkers[th_misc]; currentthinker = currentthinker->cnext)
        if (currentthinker->function)
            currentthinker->function(currentthinker);

    T_MAPMusic();
}

//
// P_Ticker
//
void P_Ticker(void)
{
    if (paused)
        return;

    P_PlayerThink();

    if (menuactive || consoleactive)
        return;

    P_MapEnd();

    if (freeze)
    {
        P_MobjThinker(viewplayer->mo);
        return;
    }

    P_RunThinkers();

    P_UpdateSpecials();
    P_RespawnSpecials();

    // for par times
    leveltime++;
    stat_time = SafeAdd(stat_time, 1);
}
