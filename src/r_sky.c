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

#include "c_cmds.h"
#include "doomstat.h"
#include "m_array.h"
#include "m_config.h"
#include "p_setup.h"
#include "r_sky.h"

//
// sky mapping
//
int         skyflatnum;
int         skytexture;
int         skytexturemid;
int         skycolumnoffset;
int         skyscrolldelta;

fixed_t     skyiscale;

bool        canfreelook = false;

sky_t       *sky = NULL;

// PSX fire sky <https://fabiensanglard.net/doom_fire_psx/>
static byte fireindices[FIREWIDTH * FIREHEIGHT];
static byte firepixels[FIREWIDTH * FIREHEIGHT];

static void PrepareFirePixels(fire_t *fire)
{
    byte    *rover = firepixels;

    for (int x = 0; x < FIREWIDTH; x++)
    {
        byte    *src = fireindices + x;

        for (int y = 0; y < FIREHEIGHT; y++)
        {
            *rover++ = fire->palette[*src];
            src += FIREWIDTH;
        }
    }
}

static void SpreadFire(void)
{
    for (int x = 0; x < FIREWIDTH; x++)
        for (int y = 1; y < FIREHEIGHT; y++)
        {
            const int   src = y * FIREWIDTH + x;
            const int   index = fireindices[src];

            if (!index)
                fireindices[src - FIREWIDTH] = 0;
            else
            {
                const int   r = M_BigRandom() & 3;

                fireindices[src - r + 1 - FIREWIDTH] = index - (r & 1);
            }
        }
}

static void SetupFire(fire_t *fire)
{
    const int   last = array_size(fire->palette) - 1;

    memset(fireindices, 0, FIREWIDTH * FIREHEIGHT);

    for (int i = 0; i < FIREWIDTH; i++)
        fireindices[(FIREHEIGHT - 1) * FIREWIDTH + i] = last;

    for (int i = 0; i < 64; i++)
        SpreadFire();

    PrepareFirePixels(fire);
}

byte *R_GetFireColumn(int col)
{
    while (col < 0)
        col += FIREWIDTH;

    col %= FIREWIDTH;
    return &firepixels[col * FIREHEIGHT];
}

static void InitSkyDefs(void)
{
    static skydefs_t    *skydefs;
    static bool         runonce = true;

    if (runonce)
    {
        skydefs = R_ParseSkyDefs();
        runonce = false;
    }

    if (!skydefs)
        return;

    array_foreach(sky, skydefs->skies)
        if (skytexture == R_CheckTextureNumForName(sky->skytex.name))
        {
            if (sky->type == SkyType_Fire)
                SetupFire(&sky->fire);

            return;
        }
}

void R_UpdateSky(void)
{
    skytex_t    *background;

    if (!sky)
        return;

    if (sky->type == SkyType_Fire)
    {
        fire_t  *fire = &sky->fire;

        if (!fire->ticsleft)
        {
            SpreadFire();
            PrepareFirePixels(fire);
            fire->ticsleft = fire->updatetime;
        }

        fire->ticsleft--;
        return;
    }

    background = &sky->skytex;
    background->currx += background->scrollx;
    background->curry += background->scrolly;

    if (sky->type == SkyType_WithForeground)
    {
        skytex_t    *foreground = &sky->foreground;

        foreground->currx += foreground->scrollx;
        foreground->curry += foreground->scrolly;
    }
}

void R_InitSkyMap(void)
{
    skyflatnum = R_FlatNumForName(SKYFLATNAME);
    terraintypes[skyflatnum] = SKY;
    skytexture = P_GetMapSky1Texture(gameepisode, gamemap);
    canfreelook = ((freelook || keyboardfreelook || keyboardfreelook2 || mousefreelook != -1 || controllerfreelook
        || autotilt || (weaponrecoil && r_screensize == r_screensize_max)) && !nofreelook);

    if (!skytexture || (BTSX && !canfreelook))
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

                case 6:
                    skytexture = R_TextureNumForName(R_CheckTextureNumForName("SKY6_ZD") != -1 ? "SKY6_ZD" : "SKY6");
                    break;
            }
        }
    }

    InitSkyDefs();

    skyscrolldelta = (vanilla ? 0 : (int)(P_GetMapSky1ScrollDelta(gameepisode, gamemap) * FRACUNIT));

    if (canfreelook)
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
