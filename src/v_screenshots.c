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
#include "s_sound.h"
#include "v_screenshots.h"
#include "v_video.h"
#include "w_file.h"

#define CACHESIZE       65536
#define MAXSCREENSHOTS  2
#define MAXJOBS         4

typedef struct
{
    char            path[MAX_PATH];
    int             width;
    int             height;
    byte            *raw;
    size_t          rawsize;
    byte            palette[256 * 3];
} screenshot_t;

typedef struct
{
    screenshot_t    screenshots[MAXSCREENSHOTS];
    int             numscreenshots;
    SDL_Thread      *thread;
    SDL_atomic_t    done;
    SDL_atomic_t    failed;
} screenshotjob_t;

typedef struct
{
    char    key[MAX_PATH * 2];
    int     next;
} screenshotcounter_t;

bool                        takingcleancreenshot;

static char                 lbmname1[MAX_PATH];
static char                 lbmpath1[MAX_PATH];
static char                 lbmpath2[MAX_PATH];
static char                 lbmname2[MAX_PATH];
static screenshotjob_t      jobs[MAXJOBS];
static screenshotcounter_t  maincounter;
static screenshotcounter_t  mapcounter;
static int                  firstjob;
static int                  numjobs;

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

static bool V_CaptureScreenShot(screenshotjob_t *job, SDL_Window *sdlwindow, const char *path)
{
    bool            result = false;
    int             width = 0;
    int             height = 0;
    SDL_Renderer    *renderer = SDL_GetRenderer(sdlwindow);

    SDL_GetWindowSize(sdlwindow, &width, &height);

    blitfunc();

    if (renderer && width > 0 && height > 0 && job->numscreenshots < MAXSCREENSHOTS)
    {
        const int       pngwidth = (vid_widescreen ? width : height * 4 / 3);
        const int       pitch = pngwidth * 3;
        const size_t    rawsize = (size_t)(pngwidth + 1) * height;
        byte            *pixels = I_Malloc((size_t)pitch * height);

        if (!SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGB24, pixels, pitch))
        {
            static uint32_t cachekeys[CACHESIZE];
            static byte     cachecolors[CACHESIZE];
            screenshot_t    *screenshot = &job->screenshots[job->numscreenshots++];
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
        char    temppath[MAX_PATH + 5];
        FILE    *file;

        M_snprintf(temppath, sizeof(temppath), "%s.tmp", screenshot->path);

        if ((file = fopen(temppath, "wb")))
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

            if (result && rename(temppath, screenshot->path))
                result = false;

            if (!result)
                remove(temppath);
        }
    }

    free(compressed);
    return result;
}

static int SDLCALL V_ScreenShotThread(void *data)
{
    screenshotjob_t *job = (screenshotjob_t *)data;

    for (int i = 0; i < job->numscreenshots; i++)
        if (!V_WritePNG(&job->screenshots[i]))
            SDL_AtomicSet(&job->failed, 1);

    SDL_AtomicSet(&job->done, 1);
    return 0;
}

static void V_StartScreenShotThread(screenshotjob_t *job)
{
    if (!(job->thread = SDL_CreateThread(V_ScreenShotThread, "screenshot", job)))
        V_ScreenShotThread(job);
}

static void V_ReapScreenShot(const bool report)
{
    screenshotjob_t *job = &jobs[firstjob];

    if (job->thread)
    {
        SDL_WaitThread(job->thread, NULL);
        job->thread = NULL;
    }

    for (int i = 0; i < job->numscreenshots; i++)
    {
        free(job->screenshots[i].raw);
        job->screenshots[i].raw = NULL;
    }

    job->numscreenshots = 0;
    firstjob = (firstjob + 1) % MAXJOBS;
    numjobs--;

    if (SDL_AtomicSet(&job->failed, 0) && report)
    {
        C_ShowConsole(false);
        C_Warning(0, "A screenshot couldn't be saved.");
    }
}

static bool V_ScreenShotPending(const char *path)
{
    for (int i = 0; i <= numjobs; i++)
    {
        const screenshotjob_t   *job = &jobs[(firstjob + i) % MAXJOBS];

        for (int j = 0; j < job->numscreenshots; j++)
            if (M_StringCompare(path, job->screenshots[j].path))
                return true;
    }

    return false;
}

void V_FinishScreenShot(const bool report)
{
    while (numjobs)
        V_ReapScreenShot(report);
}

void V_UpdateScreenShot(void)
{
    while (numjobs && SDL_AtomicGet(&jobs[firstjob].done))
        V_ReapScreenShot(true);
}

void V_FlashScreen(void)
{
    if (vanilla)
        return;

    memset(screens[0], nearestwhite, SCREENAREA);
    D_FadeScreen(true);
}

void V_ScreenShotSound(void)
{
    if (!vanilla)
        S_StartSound(NULL, sfx_scrsht);
}

static void V_FormatScreenShotName(char *name, const size_t size, const char *base, const int count)
{
    if (!count)
        M_snprintf(name, (int)size, "%s.png", base);
    else if (count < 1000)
        M_snprintf(name, (int)size, "%s (%d).png", base, count);
    else if (count < 1000000)
        M_snprintf(name, (int)size, "%s (%d,%03d).png", base, count / 1000, count % 1000);
    else
    {
        char    *temp = commify(count);

        M_snprintf(name, (int)size, "%s (%s).png", base, temp);
        free(temp);
    }
}

static bool V_ScreenShotExists(const char *folder, const char *base, const int count)
{
    char    name[MAX_PATH];
    char    path[MAX_PATH];

    V_FormatScreenShotName(name, sizeof(name), base, count);
    M_snprintf(path, sizeof(path), "%s%s", folder, name);

    return (M_FileExists(path) || V_ScreenShotPending(path));
}

static void V_FindScreenShotName(const char *folder, const char *base, screenshotcounter_t *counter,
    char *name, const size_t namesize, char *path, const size_t pathsize)
{
    char    key[MAX_PATH * 2];
    int     count = 0;

    M_snprintf(key, sizeof(key), "%s%s", folder, base);

    while (V_ScreenShotExists(folder, base, count))
    {
        if (count)
            count++;
        else if (counter->next > 1 && M_StringCompare(key, counter->key)
            && V_ScreenShotExists(folder, base, counter->next - 1))
            count = counter->next;
        else
            count = 1;
    }

    V_FormatScreenShotName(name, namesize, base, count);
    M_snprintf(path, (int)pathsize, "%s%s", folder, name);

    M_StringCopy(counter->key, key, sizeof(counter->key));
    counter->next = count + 1;
}

void V_ScreenShot(void)
{
    bool            result;
    char            mapname[128];
    char            *folder;
    char            *temp1;
    char            *temp2;
    screenshotjob_t *job;
    const bool      fading = (fadecount || menuitemfadecount);

    V_UpdateScreenShot();

    if (numjobs == MAXJOBS)
        V_ReapScreenShot(true);

    job = &jobs[(firstjob + numjobs) % MAXJOBS];
    job->numscreenshots = 0;
    job->thread = NULL;

    SDL_AtomicSet(&job->done, 0);
    SDL_AtomicSet(&job->failed, 0);

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
        else
            M_StringCopy(mapname, "Menu", sizeof(mapname));
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

    V_FindScreenShotName(folder, temp1, &maincounter, lbmname1, sizeof(lbmname1), lbmpath1, sizeof(lbmpath1));

    free(temp1);

    fadecount = 0;
    menuitemfadecount = 0;

    if (cleanscreenshots)
        takingcleancreenshot = true;

    if (cleanscreenshots || fading)
        D_Display();

    if ((result = V_CaptureScreenShot(job, window, lbmpath1)) && mapwindow && gamestate == GS_LEVEL)
    {
        V_FindScreenShotName(folder, "Automap", &mapcounter, lbmname2, sizeof(lbmname2),
            lbmpath2, sizeof(lbmpath2));

        if (!V_CaptureScreenShot(job, mapwindow, lbmpath2))
            lbmpath2[0] = '\0';
    }

    if (job->numscreenshots)
    {
        numjobs++;
        V_StartScreenShotThread(job);
    }

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
