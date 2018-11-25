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

#include "doomstat.h"
#include "m_config.h"
#include "p_setup.h"
#include "r_main.h"
#include "r_sky.h"

//
// sky mapping
//
int             skyflatnum;
int             skytexture;
int             skytexturemid;
int             skycolumnoffset = 0;
int             skyscrolldelta;

fixed_t         skyiscale;

extern dboolean canmouselook;

void R_InitSkyMap(void)
{
    int map = (gameepisode - 1) * 10 + gamemap;

    skyflatnum = R_FlatNumForName(SKYFLATNAME);
    skytexture = P_GetMapSky1Texture(map);

    if (!skytexture)
    {
        if (gamemode == commercial)
        {
            if (gamemap < 12)
                skytexture = R_TextureNumForName("SKY1");
            else if (gamemap < 21)
                skytexture = R_TextureNumForName("SKY2");
            else
                skytexture = R_TextureNumForName("SKY3");
        }
        else
        {
            switch (gameepisode)
            {
                default:
                case 1:
                    skytexture = R_TextureNumForName("SKY1");
                    break;

                case 2:
                    skytexture = R_TextureNumForName("SKY2");
                    break;

                case 3:
                    skytexture = R_TextureNumForName("SKY3");
                    break;

                case 4:
                    skytexture = R_TextureNumForName("SKY4");
                    break;
            }
        }
    }

    skyscrolldelta = P_GetMapSky1ScrollDelta(map);

    if ((canmouselook = ((mouselook || keyboardmouselook || mousemouselook != -1 || autotilt) && !nomouselook)))
    {
        int skyheight = textureheight[skytexture] >> FRACBITS;

        if (skyheight >= 128 && skyheight < 200)
            skytexturemid = -54 * FRACUNIT * skyheight / SKYSTRETCH_HEIGHT;
        else if (skyheight > 200)
            skytexturemid = (200 - skyheight) * FRACUNIT * skyheight / SKYSTRETCH_HEIGHT;
        else
            skytexturemid = 0;

        skyiscale = (fixed_t)(((uint64_t)FRACUNIT * SCREENWIDTH * 200) / (viewwidth * SCREENHEIGHT)) * skyheight / SKYSTRETCH_HEIGHT;
    }
    else
    {
        skytexturemid = ORIGINALHEIGHT / 2 * FRACUNIT;
        skyiscale = (fixed_t)(((uint64_t)FRACUNIT * SCREENWIDTH * 200) / (viewwidth * SCREENHEIGHT));
    }
}
