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

#if !defined(__P_TICK__)
#define __P_TICK__

#ifdef __GNUG__
#pragma interface
#endif

void P_Ticker(void);

void P_RemoveThinkerDelayed(thinker_t *thinker);        // killough 4/25/98

// killough 8/29/98: threads of thinkers, for more efficient searches
// cph 2002/01/13: for consistency with the main thinker list, keep objects
// pending deletion on a class list too
typedef enum
{
    th_delete,
    th_mobj,
    th_misc,
    NUMTHCLASS,
    th_all = NUMTHCLASS,        // For P_NextThinker, indicates "any class"
} th_class;

extern thinker_t thinkerclasscap[];
#define thinkercap thinkerclasscap[th_all]

#endif
