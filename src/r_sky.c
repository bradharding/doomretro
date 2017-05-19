/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2017 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://wiki.doomretro.com/credits>.

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

#include "doomstat.h"
#include "p_setup.h"
#include "r_data.h"
#include "r_main.h"
#include "r_sky.h"
#include "r_state.h"

//
// sky mapping
//
int             skyflatnum;
int             skytexture;
int             skytexturemid;
int             skycolumnoffset;
int             skyscrolldelta;

fixed_t         skyiscale;

extern dboolean m_look;

void R_InitSkyMap(void)
{
    int map = (gameepisode - 1) * 10 + gamemap;

    skyflatnum = R_FlatNumForName(SKYFLATNAME);

    skytexture = P_GetMapSky1Texture(map);

    if (!skytexture || ((textureheight[skytexture] >> FRACBITS) > 128 && !m_look))
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

                case 4:                         // Special Edition sky
                    skytexture = R_TextureNumForName("SKY4");
                    break;
            }
        }
    }

    skyscrolldelta = P_GetMapSky1ScrollDelta(map);

    if (!m_look)
    {
        skytexturemid = 100 * FRACUNIT;
        skyiscale = (fixed_t)(((uint64_t)FRACUNIT * SCREENWIDTH * 200) / (viewwidth * SCREENHEIGHT));
    }
    else
    {
        int skyheight = textureheight[skytexture] >> FRACBITS;

        // There are various combinations for sky rendering depending on how tall the sky is:
        //        h <  128: Unstretched and tiled, centered on horizon
        // 128 <= h <  200: Can possibly be stretched. When unstretched, the baseline is
        //                  28 rows below the horizon so that the top of the texture
        //                  aligns with the top of the screen when looking straight ahead.
        //                  When stretched, it is scaled to 228 pixels with the baseline
        //                  in the same location as an unstretched 128-tall sky, so the top
        //                  of the texture aligns with the top of the screen when looking
        //                  fully up.
        //        h == 200: Unstretched, baseline is on horizon, and top is at the top of
        //                  the screen when looking fully up.
        //        h >  200: Unstretched, but the baseline is shifted down so that the top
        //                  of the texture is at the top of the screen when looking fully up.
        skytexturemid = 0;

        if (skyheight >= 128 && skyheight < 200)
            skytexturemid = -28 * FRACUNIT;
        else if (skyheight > 200)
            skytexturemid = (200 - skyheight) << FRACBITS;

        skyiscale = (fixed_t)(((uint64_t)FRACUNIT * SCREENWIDTH * 200) / (viewwidth * SCREENHEIGHT));

        skyiscale = skyiscale * skyheight / SKYSTRETCH_HEIGHT;
        skytexturemid = skytexturemid * skyheight / SKYSTRETCH_HEIGHT;
    }
}
