/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <stdio.h>
#include <stdlib.h>

#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_iwad.h"
#include "d_main.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "miniz/miniz.h"
#include "p_setup.h"
#include "v_screenshots.h"
#include "v_video.h"
#include "w_file.h"

#define CACHESIZE       65536
#define MAXSCREENSHOTS  2

typedef struct
{
    char    path[MAX_PATH];
    int     width;
    int     height;
    byte    *raw;
    size_t  rawsize;
    byte    palette[256 * 3];
} screenshot_t;

bool                takingcleancreenshot;

static char         lbmname1[MAX_PATH];
static char         lbmpath1[MAX_PATH];
static char         lbmpath2[MAX_PATH];
static char         lbmname2[MAX_PATH];
static screenshot_t screenshots[MAXSCREENSHOTS];
static int          numscreenshots;
static SDL_Thread   *screenshotthread;
static SDL_atomic_t screenshotdone;
static SDL_atomic_t screenshotfailed;

static bool V_WritePNGChunk(FILE *file, const char *type, const byte *data, mz_uint32 length)
{
    byte        header[8];
    byte        crcbytes[4];
    mz_uint32   bigendian = BIGLONG(length);
    mz_uint32   crc = (mz_uint32)mz_crc32(MZ_CRC32_INIT, (const unsigned char *)type, 4);

    if (length)
        crc = (mz_uint32)mz_crc32(crc, data, length);

    memcpy(header, &bigendian, sizeof(bigendian));
    memcpy(header + 4, type, 4);
    bigendian = BIGLONG(crc);
    memcpy(crcbytes, &bigendian, sizeof(bigendian));

    return (fwrite(header, 1, sizeof(header), file) == sizeof(header)
        && (!length || fwrite(data, 1, length, file) == length)
        && fwrite(crcbytes, 1, sizeof(crcbytes), file) == sizeof(crcbytes));
}

static bool V_CaptureScreenShot(SDL_Window *sdlwindow, const char *path)
{
    bool            result = false;
    int             width = 0;
    int             height = 0;
    SDL_Renderer    *renderer = SDL_GetRenderer(sdlwindow);

    SDL_GetWindowSize(sdlwindow, &width, &height);

    blitfunc();

    if (renderer && width > 0 && height > 0 && numscreenshots < MAXSCREENSHOTS)
    {
        const int       pngwidth = (vid_widescreen ? width : height * 4 / 3);
        const int       pitch = pngwidth * 3;
        const size_t    rawsize = (size_t)(pngwidth + 1) * height;
        byte            *pixels = I_Malloc((size_t)pitch * height);

        if (!SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGB24, pixels, pitch))
        {
            static uint32_t cachekeys[CACHESIZE];
            static byte     cachecolors[CACHESIZE];
            screenshot_t    *screenshot = &screenshots[numscreenshots++];
            byte            *raw = I_Malloc(rawsize);
            uint32_t        prevkey = UINT32_MAX;
            byte            prevcolor = 0;

            M_StringCopy(screenshot->path, path, sizeof(screenshot->path));

            screenshot->width = pngwidth;
            screenshot->height = height;
            screenshot->raw = raw;
            screenshot->rawsize = rawsize;

            for (int i = 0; i < 256; i++)
            {
                screenshot->palette[i * 3] = palettecolors[i].r;
                screenshot->palette[i * 3 + 1] = palettecolors[i].g;
                screenshot->palette[i * 3 + 2] = palettecolors[i].b;
            }

            memset(cachekeys, 0xFF, sizeof(cachekeys));

            for (int y = 0; y < height; y++)
            {
                const byte  *src = pixels + (size_t)y * pitch;
                byte        *dest = raw + (size_t)y * (pngwidth + 1);

                *dest++ = 0;

                for (int x = 0; x < pngwidth; x++, src += 3)
                {
                    const uint32_t  key = (((uint32_t)src[0] << 16) | ((uint32_t)src[1] << 8) | src[2]);
                    const uint32_t  slot = ((key * 2654435761u) >> 16);

                    if (key != prevkey)
                    {
                        if (cachekeys[slot] != key)
                        {
                            cachekeys[slot] = key;
                            cachecolors[slot] = (byte)I_GetNearestColor(screenshot->palette,
                                src[0], src[1], src[2]);
                        }

                        prevkey = key;
                        prevcolor = cachecolors[slot];
                    }

                    *dest++ = prevcolor;
                }
            }

            result = true;
        }

        free(pixels);
    }

    return result;
}

static bool V_WritePNG(const screenshot_t *screenshot)
{
    static const byte   pngsignature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    bool                result = false;
    mz_ulong            compressedsize = mz_compressBound((mz_ulong)screenshot->rawsize);
    byte                *compressed = malloc(compressedsize);

    if (compressed && mz_compress2(compressed, &compressedsize, screenshot->raw, (mz_ulong)screenshot->rawsize,
        (compressscreenshots ? MZ_UBER_COMPRESSION : MZ_NO_COMPRESSION)) == MZ_OK)
    {
        FILE    *file = fopen(screenshot->path, "wb");

        if (file)
        {
            byte        ihdr[13];
            mz_uint32   bigendian = BIGLONG((mz_uint32)screenshot->width);

            memcpy(ihdr, &bigendian, sizeof(bigendian));
            bigendian = BIGLONG((mz_uint32)screenshot->height);
            memcpy(ihdr + 4, &bigendian, sizeof(bigendian));

            ihdr[8] = 8;
            ihdr[9] = 3;
            ihdr[10] = 0;
            ihdr[11] = 0;
            ihdr[12] = 0;

            result = (fwrite(pngsignature, 1, sizeof(pngsignature), file) == sizeof(pngsignature)
                && V_WritePNGChunk(file, "IHDR", ihdr, sizeof(ihdr))
                && V_WritePNGChunk(file, "PLTE", screenshot->palette, sizeof(screenshot->palette))
                && V_WritePNGChunk(file, "IDAT", compressed, (mz_uint32)compressedsize)
                && V_WritePNGChunk(file, "IEND", NULL, 0));

            if (fclose(file))
                result = false;
        }
    }

    free(compressed);
    return result;
}

static int SDLCALL V_ScreenShotThread(void *data)
{
    for (int i = 0; i < numscreenshots; i++)
        if (!V_WritePNG(&screenshots[i]))
            SDL_AtomicSet(&screenshotfailed, 1);

    SDL_AtomicSet(&screenshotdone, 1);
    return 0;
}

static void V_StartScreenShotThread(void)
{
    if (!numscreenshots)
        return;

    SDL_AtomicSet(&screenshotdone, 0);

    if (!(screenshotthread = SDL_CreateThread(V_ScreenShotThread, "screenshot", NULL)))
        V_ScreenShotThread(NULL);
}

void V_FinishScreenShot(const bool report)
{
    if (screenshotthread)
    {
        SDL_WaitThread(screenshotthread, NULL);
        screenshotthread = NULL;
    }

    if (!numscreenshots)
        return;

    for (int i = 0; i < numscreenshots; i++)
    {
        free(screenshots[i].raw);
        screenshots[i].raw = NULL;
    }

    numscreenshots = 0;

    if (SDL_AtomicSet(&screenshotfailed, 0) && report)
    {
        C_ShowConsole(false);
        C_Warning(0, "A screenshot couldn't be saved.");
    }
}

void V_UpdateScreenShot(void)
{
    if (numscreenshots && (!screenshotthread || SDL_AtomicGet(&screenshotdone)))
        V_FinishScreenShot(true);
}

void V_ScreenShot(void)
{
    bool    result;
    char    mapname[128];
    char    *folder;
    char    *temp1;
    char    *temp2;
    int     count = 0;

    V_FinishScreenShot(true);

    lbmpath2[0] = '\0';

    if (consoleactive)
        M_StringCopy(mapname, "Console", sizeof(mapname));
    else if (palettescreen)
        M_StringCopy(mapname, "Palette", sizeof(mapname));
    else if (helpscreen)
        M_StringCopy(mapname, "Help", sizeof(mapname));
    else if (menuactive)
    {
        if (currentmenu == &MainDef)
            M_StringCopy(mapname, "Main Menu", sizeof(mapname));
        else if (currentmenu == &EpiDef)
            M_StringCopy(mapname, "Episode Menu", sizeof(mapname));
        else if (currentmenu == &ExpDef)
            M_StringCopy(mapname, "Expansion Menu", sizeof(mapname));
        else if (currentmenu == &NewDef)
            M_StringCopy(mapname, "Skill Level Menu", sizeof(mapname));
        else if (currentmenu == &LoadDef)
            M_StringCopy(mapname, "Load Game Menu", sizeof(mapname));
        else if (currentmenu == &SaveDef)
            M_StringCopy(mapname, "Save Game Menu", sizeof(mapname));
        else if (currentmenu == &OptionsDef)
            M_StringCopy(mapname, "Options Menu", sizeof(mapname));
        else if (currentmenu == &SoundDef)
            M_StringCopy(mapname, "Sound Volume Menu", sizeof(mapname));
    }
    else if (automapactive)
        M_StringCopy(mapname, "Automap", sizeof(mapname));
    else if (paused)
        M_StringCopy(mapname, "Paused", sizeof(mapname));
    else if (splashscreen)
        M_StringCopy(mapname, "Splash", sizeof(mapname));
    else
        switch (gamestate)
        {
            case GS_INTERMISSION:
                M_StringCopy(mapname, "Intermission", sizeof(mapname));
                break;

            case GS_FINALE:
                M_StringCopy(mapname, "Finale", sizeof(mapname));
                break;

            case GS_TITLESCREEN:
                M_StringCopy(mapname, (titlesequence == 1 ? "Credits" : "Title"), sizeof(mapname));
                break;

            default:
                temp2 = titlecase(maptitle);
                M_StringCopy(mapname, temp2, sizeof(mapname));
                free(temp2);
                break;
        }

    if (M_StringStartsWith(mapname, "The "))
    {
        temp2 = M_SubString(mapname, 4, strlen(mapname) - 4);
        M_snprintf(mapname, sizeof(mapname), "%s, The", temp2);
        free(temp2);
    }
    else if (M_StringStartsWith(mapname, "A "))
    {
        temp2 = M_SubString(mapname, 2, strlen(mapname) - 2);
        M_snprintf(mapname, sizeof(mapname), "%s, A", temp2);
        free(temp2);
    }

    temp1 = makevalidfilename(mapname);
    folder = D_GetScreenshotFolder();

    do
    {
        if (!count)
            M_snprintf(lbmname1, sizeof(lbmname1), "%s.png", temp1);
        else if (count < 1000)
            M_snprintf(lbmname1, sizeof(lbmname1), "%s (%d).png", temp1, count);
        else if (count < 1000000)
            M_snprintf(lbmname1, sizeof(lbmname1), "%s (%d,%03d).png",
                temp1, count / 1000, count % 1000);
        else
        {
            temp2 = commify(count);
            M_snprintf(lbmname1, sizeof(lbmname1), "%s (%s).png", temp1, temp2);
            free(temp2);
        }

        count++;
        M_snprintf(lbmpath1, sizeof(lbmpath1), "%s%s", folder, lbmname1);
    } while (M_FileExists(lbmpath1));

    free(temp1);

    fadecount = 0;
    menuitemfadecount = 0;

    if (cleanscreenshots)
    {
        takingcleancreenshot = true;
        D_Display();
    }

    if ((result = V_CaptureScreenShot(window, lbmpath1)) && mapwindow && gamestate == GS_LEVEL)
    {
        count = 0;

        do
        {
            if (!count)
                M_StringCopy(lbmname2, "Automap.png", sizeof(lbmname2));
            else if (count < 1000)
                M_snprintf(lbmname2, sizeof(lbmname2), "Automap (%d).png", count);
            else if (count < 1000000)
                M_snprintf(lbmname2, sizeof(lbmname2), "Automap (%d,%03d).png",
                    count / 1000, count % 1000);
            else
            {
                temp2 = commify(count);
                M_snprintf(lbmname2, sizeof(lbmname2), "Automap (%s).png", temp2);
                free(temp2);
            }

            count++;
            M_snprintf(lbmpath2, sizeof(lbmpath2), "%s%s", folder, lbmname2);
        } while (M_FileExists(lbmpath2));

        V_CaptureScreenShot(mapwindow, lbmpath2);
    }

    V_StartScreenShotThread();

    if (cleanscreenshots)
    {
        takingcleancreenshot = false;
        D_Display();
    }

    fadecount = 0;
    menuitemfadecount = 0;

    if (result)
    {
        static char buffer[512];

        M_snprintf(buffer, sizeof(buffer), s_GSCREENSHOT, lbmname1);
        HU_SetPlayerMessage(buffer, false, false);
        message_dontfuckwithme = true;

        C_Output(BOLD("%s") " was saved.", lbmpath1);

        if (*lbmpath2)
            C_Output(BOLD("%s") " was also saved.", lbmpath2);
    }
    else
    {
        C_ShowConsole(false);
        C_Warning(0, "A screenshot couldn't be taken.");
    }
}
