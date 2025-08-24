/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

    This file is a part of DOOM Retro.

    DOOM Retro is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the license, or (at your
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

==============================================================================
*/

#include "c_console.h"
#include "doomstat.h"
#include "m_config.h"
#include "m_menu.h"
#include "p_local.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

int         maptime = 0;
int         totaltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// killough 08/29/98: we maintain several separate threads, each containing
// a special class of thinkers, to allow more efficient searches.
thinker_t   thinkers[th_all + 1];

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
    for (int i = 0; i < NUMTHCLASS; i++)
        thinkers[i].cprev = thinkers[i].cnext = &thinkers[i];

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
    th = &thinkers[(thinker->function == &P_RemoveThinkerDelayed ? th_delete :
        (thinker->function == &P_MobjThinker || thinker->function == &MusInfoThinker ? th_mobj : th_misc))];
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

    // killough 11/98: init reference counter to 0
    thinker->references = 0;

    // killough 08/29/98: set sentinel pointers, and then add to appropriate list
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
// P_RemoveThinkerDelayed
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
        (next->prev = currentthinker = thinker->prev)->next = next;

        // Remove from current thinker class list
        (th->cprev = thinker->cprev)->cnext = th;
        Z_Free(thinker);
    }
}

//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
// killough 04/25/98:
//
// Instead of marking the function with -1 value cast to a function pointer,
// set the function to P_RemoveThinkerDelayed(), so that later, it will be
// removed automatically as part of the thinker process.
//
void P_RemoveThinker(thinker_t *thinker)
{
    thinker->function = &P_RemoveThinkerDelayed;

    // Move to th_delete class.
    P_UpdateThinker(thinker);
}

//
// P_SetTarget
//
// This function is used to keep track of pointer references to mobj thinkers.
// In DOOM, objects such as lost souls could sometimes be removed despite
// them still being referenced. In BOOM, 'target' mobj fields were tested
// during each gametic, and any objects pointed to by them would be prevented
// from being removed. But this was incomplete, and was slow (every mobj was
// checked during every gametic). Now, we keep a count of the number of
// references, and delay removal until the count is 0.
//
void P_SetTarget(mobj_t **mop, mobj_t *targ)
{
    // If there was a target already, decrease its refcount
    if (*mop)
        (*mop)->thinker.references--;

    // Set new target and if non-NULL, increase its counter
    if ((*mop = targ))
        targ->thinker.references++;
}

//
// P_Ticker
//
void P_Ticker(void)
{
    if (paused)
        return;

    P_PlayerThink();

    if (consoleactive || helpscreen)
        return;

    if (menuactive && !freeze)
    {
        if (!(gametime & 2))
        {
            animatedtic++;

            for (thinker_t *thinker = thinkers[th_misc].cnext; thinker != &thinkers[th_misc]; thinker = thinker->cnext)
                if (thinker->menu && thinker->function != &P_RemoveThinkerDelayed)
                    thinker->function((mobj_t *)thinker);
        }

        P_UpdateSpecials();
        return;
    }

    animatedtic++;

    if (freeze)
    {
        P_MobjThinker(viewplayer->mo);
        return;
    }

    for (currentthinker = thinkers[th_all].next; currentthinker != &thinkers[th_all]; currentthinker = currentthinker->next)
        if (currentthinker->function)
            currentthinker->function((mobj_t *)currentthinker);

    P_UpdateSpecials();
    T_MAPMusic();
    P_RespawnSpecials();
    P_MapEnd();

    maptime++;
    stat_timeplayed = SafeAdd(stat_timeplayed, 1);

    if (timer && timeremaining)
        timeremaining--;
}
