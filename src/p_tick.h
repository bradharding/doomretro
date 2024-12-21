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

#pragma once

#if defined(__GNUG__)
#pragma interface
#endif

void P_Ticker(void);

void P_InitThinkers(void);
void P_AddThinker(thinker_t *thinker);
void P_RemoveThinker(thinker_t *thinker);
void P_RemoveThinker2(thinker_t *thinker);
void P_RemoveThinkerDelayed(thinker_t *thinker);

void P_UpdateThinker(thinker_t *thinker);               // killough 08/29/98

void P_SetTarget(mobj_t **mop, mobj_t *targ);           // killough 11/98

// killough 08/29/98: threads of thinkers, for more efficient searches
// cph 01/13/02: for consistency with the main thinker list, keep objects
// pending deletion on a class list too
enum
{
    th_mobj,
    th_misc,
    NUMTHCLASS,
    th_all = NUMTHCLASS
};

extern thinker_t    thinkers[];
