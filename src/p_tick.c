/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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
thinker_t       thinkerclasscap[th_all + 1];

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
    // killough 8/29/98: initialize threaded lists
    for (int i = 0; i < NUMTHCLASS; i++)
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
    thinker_t   *th = thinker->cnext;

    // Remove from current thread, if in one
    if (th)
        (th->cprev = thinker->cprev)->cnext = th;

    // Add to appropriate thread
    th = &thinkerclasscap[(thinker->function == P_MobjThinker ? th_mobj : th_misc)];
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

    thinker->references = 0;    // killough 11/98: init reference counter to 0

    // killough 8/29/98: set sentinel pointers, and then add to appropriate list
    thinker->cnext = NULL;
    thinker->cprev = NULL;
    P_UpdateThinker(thinker);
}

//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker(thinker_t *thinker)
{
    thinker->function = NULL;
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
static void P_RunThinkers(void)
{
    if (freeze)
    {
        P_PlayerThink();
        return;
    }

    for (thinker_t *th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
        if (th->function)
            th->function(th);

    for (thinker_t *th = thinkerclasscap[th_misc].cnext; th != &thinkerclasscap[th_misc]; th = th->cnext)
        if (th->function)
            th->function(th);

    T_MAPMusic();
}

//
// P_Ticker
//
void P_Ticker(void)
{
    // pause if in menu and at least one tic has been run
    if (paused || menuactive)
        return;

    P_PlayerThink();

    if (consoleactive)
        return;

    P_RunThinkers();
    P_UpdateSpecials();
    P_RespawnSpecials();

    P_MapEnd();

    // for par times
    if (!freeze)
    {
        leveltime++;
        stat_time = SafeAdd(stat_time, 1);
    }
}
