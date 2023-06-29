/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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
#include "p_setup.h"
#include "r_sky.h"

//
// sky mapping
//
int     skyflatnum;
int     skytexture;
int     skytexturemid;
int     skycolumnoffset;
int     skyscrolldelta;

fixed_t skyiscale;

bool    canmouselook = false;

void R_InitSkyMap(void)
{
    const int   map = (gameepisode - 1) * 10 + gamemap;

    skyflatnum = R_FlatNumForName(SKYFLATNAME);
    terraintypes[skyflatnum] = SKY;
    skytexture = P_GetMapSky1Texture(map);
    canmouselook = ((mouselook || keyboardmouselook || mousemouselook != -1 || autotilt
        || (weaponrecoil && r_screensize == r_screensize_max)) && !nomouselook);

    if (!skytexture || (BTSX && !canmouselook))
    {
        if (gamemode == commercial)
        {
            if (gamemission == pack_nerve)
            {
                if (gamemap < 4 || gamemap == 9)
                    skytexture = R_TextureNumForName("SKY1");
                else
                    skytexture = R_TextureNumForName("SKY3");
            }
            else
            {
                if (gamemap < 12)
                    skytexture = R_TextureNumForName("SKY1");
                else if (gamemap < 21)
                    skytexture = R_TextureNumForName("SKY2");
                else
                    skytexture = R_TextureNumForName("SKY3");
            }
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

                case 5:
                    skytexture = R_TextureNumForName(R_CheckTextureNumForName("SKY5_ZD") != -1 ? "SKY5_ZD" : "SKY5");
                    break;
            }
        }
    }

    skyscrolldelta = P_GetMapSky1ScrollDelta(map);

    if (canmouselook)
    {
        const int   skyheight = textureheight[skytexture] >> FRACBITS;

        if (skyheight >= 128 && skyheight < VANILLAHEIGHT)
            skytexturemid = -54 * FRACUNIT * skyheight / SKYSTRETCH_HEIGHT;
        else if (skyheight > VANILLAHEIGHT)
            skytexturemid = (VANILLAHEIGHT - skyheight) * FRACUNIT * skyheight / SKYSTRETCH_HEIGHT;
        else
            skytexturemid = 0;

        skyiscale = (fixed_t)(((uint64_t)SCREENWIDTH * VANILLAHEIGHT * FRACUNIT) / ((uint64_t)viewwidth * SCREENHEIGHT))
            * skyheight / SKYSTRETCH_HEIGHT;
    }
    else
    {
        skytexturemid = VANILLAHEIGHT / 2 * FRACUNIT;
        skyiscale = (fixed_t)(((uint64_t)SCREENWIDTH * VANILLAHEIGHT * FRACUNIT) / ((uint64_t)viewwidth * SCREENHEIGHT));
    }
}
