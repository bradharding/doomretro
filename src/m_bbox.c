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

#include "m_bbox.h"

void M_ClearBox(fixed_t *box)
{
    box[BOXTOP] = box[BOXRIGHT] = INT_MIN;
    box[BOXBOTTOM] = box[BOXLEFT] = INT_MAX;
}

void M_AddToBox(fixed_t *box, fixed_t x, fixed_t y)
{
    if (x < box[BOXLEFT])
        box[BOXLEFT] = x;
    else if (x > box[BOXRIGHT])
        box[BOXRIGHT] = x;
    if (y < box[BOXBOTTOM])
        box[BOXBOTTOM] = y;
    else if (y > box[BOXTOP])
        box[BOXTOP] = y;
}
