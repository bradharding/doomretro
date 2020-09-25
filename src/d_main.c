/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2020 by Brad Harding.

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

#define __STDC_WANT_LIB_EXT1__  1

#include <time.h>

#if defined(_WIN32)
#pragma comment(lib, "winmm.lib")

#include <Windows.h>
#include <commdlg.h>
#include <mmsystem.h>
#endif

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_iwad.h"
#include "doomstat.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "info.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_saveg.h"
#include "p_setup.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "version.h"
#include "w_file.h"
#include "w_merge.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "z_zone.h"

#if !defined(_WIN32)
#include <dirent.h>
#include <fnmatch.h>
#include <libgen.h>

#if !defined(__OpenBSD__) && !defined(__HAIKU__)
#include <wordexp.h>
#endif
#endif

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#endif

#define FADETICS    40

char **episodes[] =
{
    &s_M_EPISODE1,
    &s_M_EPISODE2,
    &s_M_EPISODE3,
    &s_M_EPISODE4,
    &s_M_EPISODE5
};

char **expansions[] =
{
    &s_M_EXPANSION1,
    &s_M_EXPANSION2
};

char **skilllevels[] =
{
    &s_M_SKILLLEVEL1,
    &s_M_SKILLLEVEL2,
    &s_M_SKILLLEVEL3,
    &s_M_SKILLLEVEL4,
    &s_M_SKILLLEVEL5
};

static char *iwadsrequired[] =
{
    "doom.wad",
    "doom2.wad",
    "tnt.wad",
    "plutonia.wad",
    "nerve.wad",
    "doom2.wad"
};

// Location where savegames are stored
char                *savegamefolder;

char                *pwadfile = "";

dboolean            fade = fade_default;
char                *iwadfolder = iwadfolder_default;
dboolean            melt = melt_default;
int                 turbo = turbo_default;
int                 units = units_default;

#if defined(_WIN32)
char                *wad = wad_default;
#endif

char                *packageconfig;
char                *packagewad;

static char         dehwarning[256] = "";

#if defined(_WIN32)
char                *previouswad;
#endif

dboolean            devparm;                // started game with -devparm
dboolean            fastparm;               // checkparm of -fast
dboolean            freeze;
dboolean            nomonsters;             // checkparm of -nomonsters
dboolean            pistolstart;            // [BH] checkparm of -pistolstart
dboolean            regenhealth;
dboolean            respawnitems;
dboolean            respawnmonsters;        // checkparm of -respawn

uint64_t            stat_runs = 0;

skill_t             startskill;
int                 startepisode;
static int          startmap;
dboolean            autostart;

dboolean            advancetitle;
dboolean            dowipe;
static dboolean     forcewipe;

static byte         fadescreen[SCREENWIDTH * SCREENHEIGHT];
static int          fadeheight;
int                 fadecount = 0;

dboolean            splashscreen = true;

static int          startuptimer;

dboolean            realframe;
static dboolean     error;
static dboolean     guess;

struct tm           gamestarttime;

#if defined(_WIN32)
extern HANDLE       CapFPSEvent;
#endif

//
// D_PostEvent
//
void D_PostEvent(event_t *ev)
{
    if (dowipe)
        return;

    lasteventtype = ev->type;

    if (C_Responder(ev))
        return; // console ate the event

    if (M_Responder(ev))
        return; // menu ate the event

    G_Responder(ev);
}

//
// D_FadeScreen
//
void D_FadeScreen(void)
{
    if (!fade)
        return;

    fadeheight = (SCREENHEIGHT - (vid_widescreen && gamestate == GS_LEVEL) * SBARHEIGHT) * SCREENWIDTH;
    memcpy(fadescreen, screens[0], fadeheight);
    fadecount = 3;
}

//
// D_UpdateFade
//
static void D_UpdateFade(void)
{
    static byte *tinttab;
    static int  fadewait;
    int         tics = I_GetTimeMS();

    if (fadewait < tics)
    {
        byte    *tinttabs[] = { NULL, tinttab75, tinttab50, tinttab25 };

        fadewait = tics + FADETICS;
        tinttab = tinttabs[fadecount--];
    }

    if (tinttab)
        for (int i = 0; i < fadeheight; i++)
        {
            byte    *dot = *screens + i;

            *dot = tinttab[(*dot << 8) + fadescreen[i]];
        }
}

//
// D_FadeScreenToBlack
//
void D_FadeScreenToBlack(void)
{
    if (!fade)
        return;

    for (double i = 0.9; i >= 0.0; i -= 0.1)
    {
        I_SetPaletteWithBrightness(PLAYPAL, i);
        blitfunc();
        I_SetExternalAutomapPalette();
        I_Sleep(30);
    }
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t         wipegamestate = GS_TITLESCREEN;

extern dboolean     message_on;
extern gameaction_t loadaction;

void D_Display(void)
{
    static dboolean     viewactivestate;
    static dboolean     menuactivestate;
    static dboolean     pausedstate = false;
    static gamestate_t  oldgamestate = GS_NONE;
    static int          saved_gametime = -1;
    int                 nowtime;
    int                 tics;
    int                 wipestart;
    dboolean            done;

    if (vid_capfps != TICRATE && (realframe = (gametime > saved_gametime)))
        saved_gametime = gametime;

    // change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize();
        oldgamestate = GS_NONE; // force background redraw
    }

    // save the current screen if about to wipe
    if ((dowipe = (gamestate != wipegamestate || forcewipe)))
    {
        drawdisk = false;

        if (melt)
            wipe_StartScreen();

        if (forcewipe)
            forcewipe = false;
        else
            menuactive = false;
    }

    if (gamestate != GS_LEVEL)
    {
        if (gamestate != oldgamestate && !splashscreen)
            I_SetPalette(PLAYPAL);

        switch (gamestate)
        {
            case GS_INTERMISSION:
                WI_Drawer();
                break;

            case GS_FINALE:
                F_Drawer();
                break;

            case GS_TITLESCREEN:
                D_PageDrawer();
                break;

            default:
                break;
        }
    }
    else
    {
        HU_Erase();

        // draw the view directly
        R_RenderPlayerView();

        if (!(viewplayer->cheats & CF_NOCLIP) && !freeze)
            AM_AddToPath();

        if (mapwindow || automapactive)
            AM_Drawer();

        ST_Drawer((viewheight == SCREENHEIGHT), true);

        // see if the border needs to be initially drawn
        if (oldgamestate != GS_LEVEL)
        {
            viewactivestate = false;    // view was not active
            R_FillBackScreen();         // draw the pattern into the back screen
        }

        // see if the border needs to be updated to the screen
        if (!automapactive)
        {
            if (scaledviewwidth != SCREENWIDTH)
                R_DrawViewBorder();

            if (r_detail == r_detail_low)
                V_LowGraphicDetail(viewwindowx, viewwindowy * SCREENWIDTH, viewwindowx + viewwidth,
                    (viewwindowy + viewheight) * SCREENWIDTH, lowpixelwidth, lowpixelheight);
        }

        HU_Drawer();
    }

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    oldgamestate = wipegamestate = gamestate;

    // draw pause pic
    if ((pausedstate = paused))
    {
        M_DarkBackground();

        if (M_PAUSE)
        {
            patch_t *patch = W_CacheLumpName("M_PAUSE");

            if (vid_widescreen)
                V_DrawPatchWithShadow((VANILLAWIDTH - SHORT(patch->width)) / 2,
                    viewwindowy / 2 + (viewheight / 2 - SHORT(patch->height)) / 2, patch, false);
            else
                V_DrawPatchWithShadow((VANILLAWIDTH - SHORT(patch->width)) / 2,
                    (VANILLAHEIGHT - SHORT(patch->height)) / 2, patch, false);
        }
        else
        {
            if (vid_widescreen)
                M_DrawCenteredString(viewwindowy / 2 + (viewheight / 2 - 16) / 2, s_M_PAUSED);
            else
                M_DrawCenteredString((VANILLAHEIGHT - 16) / 2, s_M_PAUSED);
        }
    }

    if (loadaction != ga_nothing)
        G_LoadedGameMessage();

    if (!dowipe || !melt)
    {
        C_Drawer();

        // menus go directly to the screen
        M_Drawer();

        if (drawdisk)
            HU_DrawDisk();

        if (countdown && gamestate == GS_LEVEL)
            C_UpdateTimer();

        if (fadecount)
            D_UpdateFade();

        // normal update
        blitfunc();
        mapblitfunc();

#if defined(_WIN32)
        if (CapFPSEvent)
            WaitForSingleObject(CapFPSEvent, 1000);
#endif

        // Figure out how far into the current tic we're in as a fixed_t
        if (vid_capfps != TICRATE)
            fractionaltic = I_GetTimeMS() * TICRATE % 1000 * FRACUNIT / 1000;

        return;
    }

    // wipe update
    wipe_EndScreen();
    wipestart = I_GetTime() - 1;

    do
    {
        do
        {
            nowtime = I_GetTime();
            tics = nowtime - wipestart;
            I_Sleep(1);
        } while (tics <= 0);

        wipestart = nowtime;
        done = wipe_ScreenWipe();

        blitfunc();
        mapblitfunc();

#if defined(_WIN32)
        if (CapFPSEvent)
            WaitForSingleObject(CapFPSEvent, 1000);
#endif
    } while (!done);
}

//
// D_DoomLoop
//
static void D_DoomLoop(void)
{
    time_t      now = time(NULL);
    player_t    player;

#if defined(_WIN32)
    localtime_s(&gamestarttime, &now);
#else
    localtime_r(&now, &gamestarttime);
#endif

    R_ExecuteSetViewSize();

    viewplayer = &player;
    memset(viewplayer, 0, sizeof(*viewplayer));

    while (true)
    {
        TryRunTics();       // will run at least one tic

        S_UpdateSounds();   // move positional sounds

        // Update display, next frame, with current state.
        D_Display();
    }
}

//
//  TITLE LOOP
//
int             titlesequence = 0;
int             pagetic = 3 * TICRATE;
int             logotic = 3 * TICRATE;

static patch_t  *pagelump;
static patch_t  *fineprintlump;
static patch_t  *logolump[18];
static patch_t  *titlelump;
static patch_t  *creditlump;
static byte     *splashpal;

//
// D_PageTicker
//
void D_PageTicker(void)
{
    static int  pagewait;
    int         pagetime;

    if (menuactive || consoleactive)
        return;

    if (pagewait < (pagetime = I_GetTime()))
    {
        pagetic--;
        pagewait = pagetime;

        if (splashscreen)
            logotic--;
    }

    if (pagetic < 0)
    {
        D_AdvanceTitle();

        if (splashscreen)
        {
            memset(screens[0], nearestblack, SCREENAREA);
            D_FadeScreen();
        }
    }
}

//
// D_PageDrawer
//
void D_PageDrawer(void)
{
    if (splashscreen)
    {
        static int  prevtic;

        if (prevtic != pagetic)
        {
            if (logotic >= 77 && logotic < 94)
                V_DrawBigPatch(143, 167, logolump[94 - logotic]);

            I_SetSimplePalette(&splashpal[(pagetic < 9 ? 9 - pagetic : (pagetic > 94 ? pagetic - 94 : 0)) * 768]);
            prevtic = pagetic;
        }
    }
    else
        V_DrawWidePatch(0, 0, 0, pagelump);
}

//
// D_AdvanceTitle
// Called after each title sequence finishes
//
void D_AdvanceTitle(void)
{
    advancetitle = true;
    forceconsoleblurredraw = true;
}

//
// This cycles through the title sequence.
//
void D_DoAdvanceTitle(void)
{
    viewplayer->playerstate = PST_LIVE;  // not reborn
    advancetitle = false;
    paused = false;
    gameaction = ga_nothing;
    gamestate = GS_TITLESCREEN;

    if (!titlesequence)
    {
        titlesequence = 1;
        V_DrawBigPatch(12, 366, fineprintlump);
        V_DrawBigPatch(143, 167, logolump[0]);
        return;
    }
    else if (titlesequence == 1)
    {
        static dboolean flag = true;

        if (flag)
        {
            flag = false;
            I_InitKeyboard();

            if (alwaysrun)
                C_StrCVAROutput(stringize(alwaysrun), "on");

            if (!TITLEPIC && !devparm)
                M_StartControlPanel();
        }

        if (pagelump == creditlump)
            forcewipe = true;

        pagelump = titlelump;
        pagetic = 20 * TICRATE;

        if (splashscreen)
        {
            I_SetPalette(PLAYPAL);
            splashscreen = false;
            I_Sleep(300);
        }

        M_SetWindowCaption();
        S_StartMusic(gamemode == commercial ? mus_dm2ttl : mus_intro);

        if (devparm)
            C_ShowConsole();
    }
    else if (titlesequence == 2)
    {
        forcewipe = true;
        pagelump = creditlump;
        pagetic = 10 * TICRATE;
    }

    if (W_CheckMultipleLumps("TITLEPIC") >= (bfgedition ? 1 : 2))
    {
        if (W_CheckMultipleLumps("CREDIT") > 1 && !doom4vanilla)
        {
            if (++titlesequence > 2)
                titlesequence = 1;
        }
        else
            titlesequence = 1;
    }
    else if (++titlesequence > 2)
        titlesequence = 1;
}

//
// D_StartTitle
//
void D_StartTitle(int page)
{
    gameaction = ga_nothing;
    titlesequence = page;

    if (mapwindow)
        AM_ClearFB();

    D_AdvanceTitle();
}

#define MAXDEHFILES 16

static char dehfiles[MAXDEHFILES][MAX_PATH];
static int  dehfilecount;

static dboolean DehFileProcessed(char *path)
{
    for (int i = 0; i < dehfilecount; i++)
        if (M_StringCompare(path, dehfiles[i]))
            return true;

    return false;
}

static char *FindDehPath(char *path, char *ext, char *pattern)
{
    // Returns a malloc'd path to the .deh file that matches a WAD path.
    // Or NULL if no matching .deh file can be found.
    // The pattern (not used in Windows) is the fnmatch pattern to search for.
#if defined(_WIN32)
    char    *dehpath = M_StringReplace(path, ".wad", ext);

    return (M_FileExists(dehpath) ? dehpath : NULL);
#else
    // Used to safely call dirname and basename, which can modify their input.
    size_t          pathlen = strlen(path);
    char            *pathcopy = malloc(pathlen + 1);
    char            *dehdir;
    char            *dehpattern;
    DIR             *dirp;
    struct dirent   *dit = NULL;

    M_StringCopy(pathcopy, path, pathlen + 1);
    dehpattern = M_StringReplace(basename(pathcopy), ".wad", pattern);
    dehpattern = M_StringReplace(dehpattern, ".WAD", pattern);
    M_StringCopy(pathcopy, path, pathlen);
    dehdir = dirname(pathcopy);
    dirp = opendir(dehdir);

    while ((dit = readdir(dirp)))
        if (!fnmatch(dehpattern, dit->d_name, 0))
        {
            char    *dehfullpath = M_StringJoin(dehdir, DIR_SEPARATOR_S, dit->d_name, NULL);

            closedir(dirp);
            free(pathcopy);

            return dehfullpath;
        }

    closedir(dirp);
    free(pathcopy);

    return NULL;
#endif
}

typedef struct
{
    char        filename[MAX_PATH];
    dboolean    present;
} loaddehlast_t;

// [BH] A list of DeHackEd files to load last
static loaddehlast_t loaddehlast[8] =
{
    { "VORTEX_DoomRetro.deh" },
    { "2_MARKV.deh"          },
    { "3_HELLST.deh"         },
    { "3_REAPER.deh"         },
    { "4_HAR.deh"            },
    { "5_GRNADE.deh"         },
    { "6_LIGHT.deh"          },
    { "7_GAUSS.deh"          }
};

static void LoadDehFile(char *path)
{
    char    *dehpath;

    for (int i = 0; i < 8; i++)
        if (M_StringEndsWith(path, loaddehlast[i].filename))
        {
            loaddehlast[i].present = true;
            return;
        }

    if ((dehpath = FindDehPath(path, ".bex", ".[Bb][Ee][Xx]")))
    {
        if (!DehFileProcessed(dehpath))
        {
            if (HasDehackedLump(path))
                M_snprintf(dehwarning, sizeof(dehwarning), "<b>%s</b> was ignored.", GetCorrectCase(dehpath));
            else
                ProcessDehFile(dehpath, 0, true);

            if (dehfilecount < MAXDEHFILES)
            {
                M_StringCopy(dehfiles[dehfilecount], dehpath, sizeof(dehfiles[dehfilecount]));
                dehfilecount++;
            }
        }
    }
    else
    {
        dehpath = FindDehPath(path, ".deh", ".[Dd][Ee][Hh]");

        if (dehpath && !DehFileProcessed(dehpath))
        {
            if (HasDehackedLump(path))
                M_snprintf(dehwarning, sizeof(dehwarning), "<b>%s</b> was ignored.", GetCorrectCase(dehpath));
            else
                ProcessDehFile(dehpath, 0, true);

            if (dehfilecount < MAXDEHFILES)
            {
                M_StringCopy(dehfiles[dehfilecount], dehpath, sizeof(dehfiles[dehfilecount]));
                dehfilecount++;
            }
        }
    }
}

static void LoadCfgFile(char *path)
{
    char    *cfgpath = M_StringReplace(path, ".wad", ".cfg");

    if (M_FileExists(cfgpath))
        M_LoadCVARs(cfgpath);
}

static dboolean D_IsDOOMIWAD(char *filename)
{
    return (M_StringEndsWith(filename, "DOOM.WAD") || M_StringEndsWith(filename, "DOOM1.WAD")
        || M_StringEndsWith(filename, "DOOM2.WAD") || M_StringEndsWith(filename, "PLUTONIA.WAD")
        || M_StringEndsWith(filename, "TNT.WAD") || (hacx = M_StringEndsWith(filename, "HACX.WAD")));
}

static dboolean D_IsUnsupportedIWAD(char *filename)
{
    const struct
    {
        char    *iwad;
        char    *title;
    } unsupported[] = {
        { "heretic.wad",  "Heretic" },
        { "heretic1.wad", "Heretic" },
        { "hexen.wad",    "Hexen"   },
        { "hexdd.wad",    "Hexen"   },
        { "strife0.wad",  "Strife"  },
        { "strife1.wad",  "Strife"  },
        { "voices.wad",   "Strife"  }
    };

    for (int i = 0; i < arrlen(unsupported); i++)
        if (M_StringEndsWith(filename, unsupported[i].iwad))
        {
            char    buffer[1024];

            M_snprintf(buffer, sizeof(buffer), PACKAGE_NAME " doesn't support %s.", unsupported[i].title);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, PACKAGE_NAME, buffer, NULL);

#if defined(_WIN32)
            if (previouswad)
                wad = M_StringDuplicate(previouswad);
#endif

            error = true;
            return true;
        }

    return false;
}

static dboolean D_IsCfgFile(char *filename)
{
    return M_StringEndsWith(filename, ".cfg");
}

static dboolean D_IsDehFile(char *filename)
{
    return (M_StringEndsWith(filename, ".deh") || M_StringEndsWith(filename, ".bex"));
}

static void D_CheckSupportedPWAD(char *filename)
{
    if (M_StringEndsWith(filename, "SIGIL.wad") || M_StringEndsWith(filename, "SIGIL_v1_1.wad")
        || M_StringEndsWith(filename, "SIGIL_v1_2.wad") || M_StringEndsWith(filename, "SIGIL_v1_21.wad"))
    {
        sigil = true;
        episode = 5;
    }
    else if (M_StringEndsWith(filename, "NERVE.WAD"))
    {
        nerve = true;
        expansion = 2;
    }
    else if (M_StringEndsWith(filename, "chex.wad"))
        chex = chex1 = true;
    else if (M_StringEndsWith(filename, "chex2.wad"))
        chex = chex2 = true;
    else if (M_StringEndsWith(filename, "btsx_e1.wad"))
        BTSX = BTSXE1 = true;
    else if (M_StringEndsWith(filename, "btsx_e1a.wad"))
        BTSX = BTSXE1 = BTSXE1A = true;
    else if (M_StringEndsWith(filename, "btsx_e1b.wad"))
        BTSX = BTSXE1 = BTSXE1B = true;
    else if (M_StringEndsWith(filename, "btsx_e2a.wad"))
        BTSX = BTSXE2 = BTSXE2A = true;
    else if (M_StringEndsWith(filename, "btsx_e2b.wad"))
        BTSX = BTSXE2 = BTSXE2B = true;
    else if (M_StringEndsWith(filename, "btsx_e3a.wad"))
        BTSX = BTSXE3 = BTSXE3A = true;
    else if (M_StringEndsWith(filename, "btsx_e3b.wad"))
        BTSX = BTSXE3 = BTSXE3B = true;
    else if (M_StringEndsWith(filename, "e1m4b.wad"))
        E1M4B = true;
    else if (M_StringEndsWith(filename, "e1m8b.wad"))
        E1M8B = true;
    else if (M_StringEndsWith(filename, "d1spfx18.wad") || M_StringEndsWith(filename, "d2spfx18.wad"))
        sprfix18 = true;
    else if (M_StringEndsWith(filename, "eviternity.wad"))
        eviternity = true;
    else if (M_StringEndsWith(filename, "d4v.wad"))
        doom4vanilla = true;
    else if (M_StringEndsWith(filename, "remnant.wad"))
        remnant = true;
}

static dboolean D_IsUnsupportedPWAD(char *filename)
{
    return (error = (M_StringEndsWith(filename, PACKAGE_WAD)));
}

static dboolean D_CheckParms(void)
{
    dboolean    result = false;

    if (myargc == 2 && M_StringEndsWith(myargv[1], ".wad"))
    {
        char    *folder = M_ExtractFolder(myargv[1]);

        // check if it's a valid and supported IWAD
        if (D_IsDOOMIWAD(myargv[1]) || (W_WadType(myargv[1]) == IWAD && !D_IsUnsupportedIWAD(myargv[1])))
        {
            D_IdentifyIWADByName(myargv[1]);

            if (W_AddFile(myargv[1], false))
            {
                result = true;
                iwadfolder = M_StringDuplicate(folder);

                // if DOOM.WAD is selected, load SIGIL.WAD automatically if present
                if (M_StringEndsWith(myargv[1], "DOOM.WAD") && IsUltimateDOOM(myargv[1]))
                {
                    char    fullpath[MAX_PATH];

                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_v1_21.wad");

                    if (W_MergeFile(fullpath, true))
                        sigil = true;
                    else
                    {
                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_v1_2.wad");

                        if (W_MergeFile(fullpath, true))
                            sigil = true;
                        else
                        {
                            M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_v1_1.wad");

                            if (W_MergeFile(fullpath, true))
                                sigil = true;
                            else
                            {
                                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL.wad");

                                if (W_MergeFile(fullpath, true))
                                    sigil = true;
                            }
                        }
                    }

                    if (sigil && !M_CheckParm("-nomusic") && !M_CheckParm("-nosound"))
                    {
                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_SHREDS.wad");

                        if (!W_MergeFile(fullpath, true))
                        {
                            M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_SHREDS_COMPAT.wad");
                            W_MergeFile(fullpath, true);
                        }
                    }
                }
                // if DOOM2.WAD is selected, load NERVE.WAD automatically if present
                else if (M_StringEndsWith(myargv[1], "DOOM2.WAD"))
                {
                    char    fullpath[MAX_PATH];

                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "NERVE.WAD");

                    if (W_MergeFile(fullpath, true))
                        nerve = true;
                }
            }
        }

        // if it's a PWAD, determine the IWAD required and try loading that as well
        else if (W_WadType(myargv[1]) == PWAD && !D_IsUnsupportedPWAD(myargv[1]))
        {
            GameMission_t   iwadrequired = IWADRequiredByPWAD(myargv[1]);
            char            fullpath[MAX_PATH];

            if (iwadrequired == none)
                iwadrequired = doom2;

            // try the current folder first
            M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, iwadsrequired[iwadrequired]);
            D_IdentifyIWADByName(fullpath);

            if (W_AddFile(fullpath, true))
            {
                result = true;
                iwadfolder = M_StringDuplicate(folder);
                D_CheckSupportedPWAD(myargv[1]);

                if (W_MergeFile(myargv[1], false))
                {
                    modifiedgame = true;

                    if (IWADRequiredByPWAD(myargv[1]) != none)
                        pwadfile = M_StringDuplicate(leafname(myargv[1]));

                    LoadCfgFile(myargv[1]);

                    if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"))
                        LoadDehFile(myargv[1]);
                }
            }
            else
            {
                // otherwise try the iwadfolder CVAR
#if defined(_WIN32) || defined(__OpenBSD__) || defined(__HAIKU__)
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", iwadfolder, iwadsrequired[iwadrequired]);
#else
                wordexp_t   p;

                if (!wordexp(iwadfolder, &p, 0) && p.we_wordc > 0)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", p.we_wordv[0], iwadsrequired[iwadrequired]);
                    wordfree(&p);
                }
                else
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", iwadfolder, iwadsrequired[iwadrequired]);
#endif
                D_IdentifyIWADByName(fullpath);

                if (W_AddFile(fullpath, true))
                {
                    result = true;
                    D_CheckSupportedPWAD(myargv[1]);

                    if (W_MergeFile(myargv[1], false))
                    {
                        modifiedgame = true;

                        if (IWADRequiredByPWAD(myargv[1]) != none)
                            pwadfile = M_StringDuplicate(leafname(myargv[1]));

                        LoadCfgFile(myargv[1]);

                        if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"))
                            LoadDehFile(myargv[1]);
                    }
                }
                else
                {
                    // still nothing? try some common installation folders
                    if (W_AddFile(D_FindWADByName(iwadsrequired[iwadrequired]), true))
                    {
                        result = true;
                        D_CheckSupportedPWAD(myargv[1]);

                        if (W_MergeFile(myargv[1], false))
                        {
                            modifiedgame = true;

                            if (IWADRequiredByPWAD(myargv[1]) != none)
                                pwadfile = M_StringDuplicate(leafname(myargv[1]));

                            LoadCfgFile(myargv[1]);

                            if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"))
                                LoadDehFile(myargv[1]);
                        }
                    }
                }
            }
        }

        if (BTSX)
        {
            char    fullpath[MAX_PATH];

            if (BTSXE1A && !BTSXE1B)
            {
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e1b.wad");
                result = W_MergeFile(fullpath, true);
            }
            else if (!BTSXE1A && BTSXE1B)
            {
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e1a.wad");
                result = W_MergeFile(fullpath, true);
            }
            else if (BTSXE2A && !BTSXE2B)
            {
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e2b.wad");
                result = W_MergeFile(fullpath, true);
            }
            else if (!BTSXE2A && BTSXE2B)
            {
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e2a.wad");
                result = W_MergeFile(fullpath, true);
            }
            else if (BTSXE3A && !BTSXE3B)
            {
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e3b.wad");
                result = W_MergeFile(fullpath, true);
            }
            else if (!BTSXE3A && BTSXE3B)
            {
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e3a.wad");
                result = W_MergeFile(fullpath, true);
            }
        }

        free(folder);
    }

    return result;
}

#if defined(_WIN32) || defined(__APPLE__)
static int D_OpenWADLauncher(void)
{
    int             iwadfound = -1;
    dboolean        fileopenedok;

#if defined(_WIN32)
    OPENFILENAME    ofn;
    char            szFile[4096];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    M_StringCopy(szFile, wad, sizeof(szFile));
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "IWAD and/or PWAD(s) (*.wad)\0*.WAD;*.DEH;*.BEX;*.CFG\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = iwadfolder;
    ofn.Flags = (OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_EXPLORER);
    ofn.lpstrTitle = "Where\u2019s All the Data?\0";

    fileopenedok = GetOpenFileName(&ofn);
#elif defined(__APPLE__)
    NSOpenPanel *panel = [NSOpenPanel openPanel];

    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:YES];
    [panel setTitle:@"Where's All the Data?"];

    NSInteger   clicked = [panel runModal];

    fileopenedok = (clicked == NSModalResponseOK);
#endif

    if (fileopenedok)
    {
        dboolean    onlyoneselected;

#if defined(__APPLE__)
        NSArray     *urls = [panel URLs];
#endif

        iwadfound = 0;
        startuptimer = I_GetTimeMS();

        // only one file was selected
#if defined(_WIN32)
        if (wad)
            previouswad = M_StringDuplicate(wad);

        wad = "";

        onlyoneselected = !ofn.lpstrFile[lstrlen(ofn.lpstrFile) + 1];
#elif defined(__APPLE__)
        onlyoneselected = ([urls count] == 1);
#endif

        if (onlyoneselected)
        {
#if defined(_WIN32)
            char    *file = (char *)ofn.lpstrFile;
#elif defined(__APPLE__)
            NSURL   *url = [urls objectAtIndex:0];
            char    *file = (char *)[url fileSystemRepresentation];
#endif
            char    *folder = M_ExtractFolder(file);

            if (!M_StringEndsWith(file, ".wad") && !M_StringEndsWith(file, ".deh")
                && !M_StringEndsWith(file, ".bex") && !M_StringEndsWith(file, ".cfg")
                && (strlen(file) < 4 || file[strlen(file) - 4] != '.'))
                file = M_StringJoin(file, ".wad", NULL);

#if defined(_WIN32)
            // if WAD doesn't exist (that is, entered manually and may be partial filename), look for best match
            if (!M_FileExists(file))
            {
                char    *temp = W_NearestFilename(folder, leafname(file));

                if (!temp)
                    error = true;
                else
                {
                    guess = true;

                    if (!M_StringEndsWith(temp, leafname(file)))
                        C_Warning(1, "<b>%s</b> couldn't be found. Did you mean <b>%s</b>?", leafname(file), leafname(temp));

                    file = M_StringDuplicate(temp);
                    free(temp);
                }
            }
            else
                wad = M_StringDuplicate(file);
#endif

            // check if it's a valid and supported IWAD
            if (D_IsDOOMIWAD(file) || (W_WadType(file) == IWAD && !D_IsUnsupportedIWAD(file)))
            {
                D_IdentifyIWADByName(file);

                if (W_AddFile(file, false))
                {
                    iwadfound = 1;

#if defined(_WIN32)
                    if (!guess)
                        wad = M_StringDuplicate(leafname(file));
#endif

                    iwadfolder = M_StringDuplicate(folder);

                    // if DOOM.WAD is selected, load SIGIL.WAD automatically if present
                    if (M_StringEndsWith(file, "DOOM.WAD") && IsUltimateDOOM(file))
                    {
                        char    fullpath[MAX_PATH];

                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_v1_21.wad");

                        if (W_MergeFile(fullpath, true))
                            sigil = true;
                        else
                        {
                            M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_v1_2.wad");

                            if (W_MergeFile(fullpath, true))
                                sigil = true;
                            else
                            {
                                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_v1_1.wad");

                                if (W_MergeFile(fullpath, true))
                                    sigil = true;
                                else
                                {
                                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL.wad");

                                    if (W_MergeFile(fullpath, true))
                                        sigil = true;
                                }
                            }
                        }

                        if (sigil && !M_CheckParm("-nomusic") && !M_CheckParm("-nosound"))
                        {
                            M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_SHREDS.wad");

                            if (!W_MergeFile(fullpath, true))
                            {
                                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_SHREDS_COMPAT.wad");
                                W_MergeFile(fullpath, true);
                            }
                        }
                    }
                    // if DOOM2.WAD is selected, load NERVE.WAD automatically if present
                    else if (M_StringEndsWith(file, "DOOM2.WAD"))
                    {
                        char    fullpath[MAX_PATH];

                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "NERVE.WAD");

                        if (W_MergeFile(fullpath, true))
                            nerve = true;
                    }
                }
            }

            // if it's a PWAD, determine the IWAD required and try loading that as well
            else if (W_WadType(file) == PWAD && !D_IsUnsupportedPWAD(file))
            {
                GameMission_t   iwadrequired = IWADRequiredByPWAD(file);
                char            fullpath[MAX_PATH];

                if (iwadrequired == none)
                    iwadrequired = doom2;

#if defined(_WIN32)
                if (!guess)
                    wad = M_StringDuplicate(leafname(file));
#endif

                // try the current folder first
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, iwadsrequired[iwadrequired]);
                D_IdentifyIWADByName(fullpath);

                if (W_AddFile(fullpath, true))
                {
                    iwadfound = 1;
                    iwadfolder = M_StringDuplicate(folder);
                    D_CheckSupportedPWAD(file);

                    if (W_MergeFile(file, false))
                    {
                        modifiedgame = true;

                        if (IWADRequiredByPWAD(file) != none)
                            pwadfile = M_StringDuplicate(leafname(file));

                        LoadCfgFile(file);

                        if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"))
                            LoadDehFile(file);
                    }
                }
                else
                {
                    // otherwise try the iwadfolder CVAR
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", iwadfolder, iwadsrequired[iwadrequired]);
                    D_IdentifyIWADByName(fullpath);

                    if (W_AddFile(fullpath, true))
                    {
                        iwadfound = 1;
                        D_CheckSupportedPWAD(file);

                        if (W_MergeFile(file, false))
                        {
                            modifiedgame = true;

                            if (IWADRequiredByPWAD(file) != none)
                                pwadfile = M_StringDuplicate(leafname(file));

                            LoadCfgFile(file);

                            if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"))
                                LoadDehFile(file);
                        }
                    }
                    else
                    {
                        // still nothing? try some common installation folders
                        if (W_AddFile(D_FindWADByName(iwadsrequired[iwadrequired]), true))
                        {
                            iwadfound = 1;
                            D_CheckSupportedPWAD(file);

                            if (W_MergeFile(file, false))
                            {
                                modifiedgame = true;

                                if (IWADRequiredByPWAD(file) != none)
                                    pwadfile = M_StringDuplicate(leafname(file));

                                LoadCfgFile(file);

                                if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"))
                                    LoadDehFile(file);
                            }
                        }
                    }
                }
            }

            if (BTSX)
            {
                char    fullpath[MAX_PATH];

                if (BTSXE1A && !BTSXE1B)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e1b.wad");
                    W_MergeFile(fullpath, true);
                }
                else if (!BTSXE1A && BTSXE1B)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e1a.wad");
                    W_MergeFile(fullpath, true);
                }
                else if (BTSXE2A && !BTSXE2B)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e2b.wad");
                    W_MergeFile(fullpath, true);
                }
                else if (!BTSXE2A && BTSXE2B)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e2a.wad");
                    W_MergeFile(fullpath, true);
                }
                else if (BTSXE3A && !BTSXE3B)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e3b.wad");
                    W_MergeFile(fullpath, true);
                }
                else if (!BTSXE3A && BTSXE3B)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "btsx_e3a.wad");
                    W_MergeFile(fullpath, true);
                }
            }

            free(folder);
        }
        else
        {
            // more than one file was selected
            dboolean    isDOOM2 = false;
            dboolean    sharewareiwad = false;

#if defined(_WIN32)
            LPSTR       iwadpass1 = ofn.lpstrFile;
            LPSTR       iwadpass2 = ofn.lpstrFile;
            LPSTR       pwadpass1 = ofn.lpstrFile;
            LPSTR       pwadpass2 = ofn.lpstrFile;
            LPSTR       cfgpass = ofn.lpstrFile;
            LPSTR       dehpass = ofn.lpstrFile;

            iwadpass1 = &iwadpass1[lstrlen(iwadpass1) + 1];

            // find and add IWAD first
            while (*iwadpass1)
            {
                char    fullpath[MAX_PATH];

                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, iwadpass1);

#elif defined(__APPLE__)
            char    *szFile;

            for (NSURL *url in urls)
            {
                char    *fullpath = (char *)[url fileSystemRepresentation];
                char    *iwadpass1 = (char *)[[url lastPathComponent] UTF8String];

                szFile = (char *)[[url URLByDeletingLastPathComponent] fileSystemRepresentation];
#endif

                if (D_IsDOOMIWAD(fullpath))
                {
                    D_IdentifyIWADByName(fullpath);

                    if (W_AddFile(fullpath, false))
                    {
                        iwadfound = 1;
                        sharewareiwad = M_StringCompare(iwadpass1, "DOOM1.WAD");
                        isDOOM2 = M_StringCompare(iwadpass1, "DOOM2.WAD");

#if defined(_WIN32)
                        if (!guess)
                            wad = M_StringDuplicate(leafname(fullpath));
#endif

                        iwadfolder = M_ExtractFolder(fullpath);
                        break;
                    }
                }

#if defined(_WIN32)
                iwadpass1 = &iwadpass1[lstrlen(iwadpass1) + 1];
#endif
            }

#if defined(_WIN32)
            iwadpass2 = &iwadpass2[lstrlen(iwadpass2) + 1];

            // find and add IWAD first
            while (*iwadpass2)
            {
                char    fullpath[MAX_PATH];

                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, iwadpass2);

#elif defined(__APPLE__)
            for (NSURL *url in urls)
            {
                char    *fullpath = (char *)[url fileSystemRepresentation];
                char    *iwadpass2 = (char *)[[url lastPathComponent] UTF8String];

                szFile = (char *)[[url URLByDeletingLastPathComponent] fileSystemRepresentation];
#endif

                if (W_WadType(fullpath) == IWAD && !D_IsUnsupportedIWAD(fullpath))
                {
                    if (!iwadfound)
                    {
                        D_IdentifyIWADByName(fullpath);

                        if (W_AddFile(fullpath, false))
                        {
                            iwadfound = 1;
                            sharewareiwad = M_StringCompare(iwadpass2, "DOOM1.WAD");
                            isDOOM2 = M_StringCompare(iwadpass2, "DOOM2.WAD");

#if defined(_WIN32)
                            if (!guess)
                                wad = M_StringDuplicate(leafname(fullpath));
#endif

                            iwadfolder = M_ExtractFolder(fullpath);
                            break;
                        }
                    }
                    else if (!D_IsDOOMIWAD(fullpath))
                    {
                        if (W_MergeFile(fullpath, false))
                        {
                            modifiedgame = true;
                            break;
                        }
                    }
                }

#if defined(_WIN32)
                iwadpass2 = &iwadpass2[lstrlen(iwadpass2) + 1];
#endif
            }

            // merge any PWADs
            if (!sharewareiwad)
            {
                // if no IWAD has been selected, check each PWAD to determine the IWAD required
                // and then try to load it first
#if defined(_WIN32)
                pwadpass1 = &pwadpass1[lstrlen(pwadpass1) + 1];

                while (!iwadfound && *pwadpass1)
                {
                    char    fullpath[MAX_PATH];

                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, pwadpass1);

#elif defined(__APPLE__)
                for (NSURL *url in urls)
                {
                    char    *fullpath = (char *)[url fileSystemRepresentation];
                    char    *pwadpass1 = (char *)[[url lastPathComponent] UTF8String];

                    if (iwadfound)
                        break;
#endif

                    if (W_WadType(fullpath) == PWAD && !D_IsUnsupportedPWAD(fullpath) && !D_IsDehFile(fullpath))
                    {
                        GameMission_t   iwadrequired = IWADRequiredByPWAD(fullpath);

                        if (iwadrequired != none)
                        {
                            char    fullpath2[MAX_PATH];

                            // try the current folder first
                            M_snprintf(fullpath2, sizeof(fullpath2), "%s" DIR_SEPARATOR_S "%s", szFile, iwadsrequired[iwadrequired]);
                            D_IdentifyIWADByName(fullpath2);

                            if (W_AddFile(fullpath2, true))
                            {
                                iwadfound = 1;
                                iwadfolder = M_ExtractFolder(fullpath2);
                            }
                            else
                            {
                                // otherwise try the iwadfolder CVAR
                                M_snprintf(fullpath2, sizeof(fullpath2), "%s" DIR_SEPARATOR_S "%s", iwadfolder,
                                    iwadsrequired[iwadrequired]);
                                D_IdentifyIWADByName(fullpath2);

                                if (W_AddFile(fullpath2, true))
                                    iwadfound = 1;
                                else
                                {
                                    // still nothing? try some common installation folders
                                    if (W_AddFile(D_FindWADByName(iwadsrequired[iwadrequired]), true))
                                        iwadfound = 1;
                                }
                            }
                        }
                    }

#if defined(_WIN32)
                    pwadpass1 = &pwadpass1[lstrlen(pwadpass1) + 1];
#endif
                }

                // if still no IWAD found, then try DOOM2.WAD
                if (!iwadfound)
                {
                    // try the current folder first
                    D_IdentifyIWADByName("DOOM2.WAD");

                    if (W_AddFile("DOOM2.WAD", true))
                        iwadfound = 1;
                    else
                    {
                        char    fullpath2[MAX_PATH];

                        // otherwise try the iwadfolder CVAR
                        M_snprintf(fullpath2, sizeof(fullpath2), "%s" DIR_SEPARATOR_S "DOOM2.WAD", iwadfolder);
                        D_IdentifyIWADByName(fullpath2);

                        if (W_AddFile(fullpath2, true))
                            iwadfound = 1;
                        else
                        {
                            // still nothing? try some common installation folders
                            if (W_AddFile(D_FindWADByName("DOOM2.WAD"), true))
                                iwadfound = 1;
                        }
                    }
                }

                // if an IWAD has now been found, make second pass through the PWADs to merge them
                if (iwadfound)
                {
                    dboolean    mapspresent = false;

#if defined(_WIN32)
                    pwadpass2 = &pwadpass2[lstrlen(pwadpass2) + 1];

                    while (*pwadpass2)
                    {
                        char    fullpath[MAX_PATH];

                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, pwadpass2);

#elif defined(__APPLE__)
                    for (NSURL *url in urls)
                    {
                        char    *fullpath = (char *)[url fileSystemRepresentation];
#endif
                        if (W_WadType(fullpath) == PWAD && !D_IsUnsupportedPWAD(fullpath) && !D_IsDehFile(fullpath))
                        {
                            D_CheckSupportedPWAD(fullpath);

                            if (W_MergeFile(fullpath, false))
                            {
#if defined(_WIN32)
                                if (!guess)
                                    wad = M_StringDuplicate(leafname(fullpath));
#endif

                                modifiedgame = true;
                                LoadCfgFile(fullpath);

                                if (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"))
                                    LoadDehFile(fullpath);

                                if (IWADRequiredByPWAD(fullpath) != none)
                                {
                                    mapspresent = true;
                                    pwadfile = M_StringDuplicate(leafname(fullpath));
                                }
                            }
                        }

#if defined(_WIN32)
                        pwadpass2 = &pwadpass2[lstrlen(pwadpass2) + 1];
#endif
                    }

                    // try to autoload NERVE.WAD if DOOM2.WAD is the IWAD and none of the PWADs
                    // have maps present
                    if (isDOOM2 && !mapspresent)
                    {
                        char    fullpath[MAX_PATH];

                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, "NERVE.WAD");

                        if (W_MergeFile(fullpath, true))
                            nerve = true;
                    }
                }
            }

            if (iwadfound)
            {
#if defined(_WIN32)
                // process any config files
                cfgpass = &cfgpass[lstrlen(cfgpass) + 1];

                while (*cfgpass)
                {
                    char    fullpath[MAX_PATH];

                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, cfgpass);

#elif defined(__APPLE__)
                for (NSURL *url in urls)
                {
                    char    *fullpath = (char *)[url fileSystemRepresentation];
#endif

                    if (D_IsCfgFile(fullpath))
                        M_LoadCVARs(fullpath);

#if defined(_WIN32)
                    cfgpass = &cfgpass[lstrlen(cfgpass) + 1];
#endif
                }

#if defined(_WIN32)
                // process any DeHackEd files last of all
                dehpass = &dehpass[lstrlen(dehpass) + 1];

                while (*dehpass)
                {
                    char    fullpath[MAX_PATH];

                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", szFile, dehpass);

#elif defined(__APPLE__)
                for (NSURL *url in urls)
                {
                    char    *fullpath = (char *)[url fileSystemRepresentation];
#endif

                    if (D_IsDehFile(fullpath))
                        LoadDehFile(fullpath);

#if defined(_WIN32)
                    dehpass = &dehpass[lstrlen(dehpass) + 1];
#endif
                }
            }
        }
    }

    return iwadfound;
}
#endif

static void D_ProcessDehCommandLine(void)
{
    int p = M_CheckParm("-deh");

    if (p || (p = M_CheckParm("-bex")))
    {
        dboolean    deh = true;

        while (++p < myargc)
            if (*myargv[p] == '-')
                deh = (M_StringCompare(myargv[p], "-deh") || M_StringCompare(myargv[p], "-bex"));
            else if (deh)
                ProcessDehFile(myargv[p], 0, false);
    }
}

static void D_ProcessDehInWad(void)
{
    dboolean    process = (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"));

    if (*dehwarning)
        C_Warning(1, dehwarning);

    if (doom4vanilla)
    {
        for (int i = 0; i < numlumps; i++)
            if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
                && process
                && !M_StringEndsWith(lumpinfo[i]->wadfile->path, PACKAGE_WAD)
                && !M_StringEndsWith(lumpinfo[i]->wadfile->path, "D4V.WAD"))
                ProcessDehFile(NULL, i, false);

        for (int i = 0; i < numlumps; i++)
            if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
                && M_StringEndsWith(lumpinfo[i]->wadfile->path, "D4V.WAD"))
                ProcessDehFile(NULL, i, false);

        for (int i = 0; i < numlumps; i++)
            if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
                && M_StringEndsWith(lumpinfo[i]->wadfile->path, PACKAGE_WAD))
                ProcessDehFile(NULL, i, false);
    }
    else if (hacx || FREEDOOM)
    {
        for (int i = 0; i < numlumps; i++)
            if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
                && (process || M_StringEndsWith(lumpinfo[i]->wadfile->path, PACKAGE_WAD)))
                ProcessDehFile(NULL, i, false);
    }
    else
    {
        if (chex1)
            ProcessDehFile(NULL, W_GetNumForName("CHEXBEX"), true);

        for (int i = numlumps - 1; i >= 0; i--)
            if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
                && !M_StringEndsWith(lumpinfo[i]->wadfile->path, "SIGIL_v1_2.wad")
                && (process || M_StringEndsWith(lumpinfo[i]->wadfile->path, PACKAGE_WAD)))
                ProcessDehFile(NULL, i, false);
    }

    for (int i = 0; i < 8; i++)
        if (loaddehlast[i].present)
            ProcessDehFile(loaddehlast[i].filename, 0, false);
}

static void D_ParseStartupString(const char *string)
{
    size_t  len = strlen(string);

    for (size_t i = 0, start = 0; i < len; i++)
        if (string[i] == '\n' || i == len - 1)
        {
            char    *temp = M_SubString(string, start, i - start);

            C_Output(temp);
            start = i + 1;
            free(temp);
        }
}

//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed
static void D_DoomMainSetup(void)
{
    int     p = M_CheckParmWithArgs("-config", 1, 1);
    int     choseniwad = 0;
    char    lumpname[6];
    char    *appdatafolder = M_GetAppDataFolder();
    char    *iwadfile;
    int     startloadgame;
    char    *resourcefolder = M_GetResourceFolder();
    char    *seconds;

    packagewad = M_StringJoin(resourcefolder, DIR_SEPARATOR_S, PACKAGE_WAD, NULL);
    free(resourcefolder);

    M_MakeDirectory(appdatafolder);
    packageconfig = (p ? M_StringDuplicate(myargv[p + 1]) : M_StringJoin(appdatafolder, DIR_SEPARATOR_S, PACKAGE_CONFIG, NULL));

#if !defined(__APPLE__)
    free(appdatafolder);
#endif

    C_Output("");
    C_PrintCompileDate();

#if defined(_WIN32)
    I_PrintWindowsVersion();
#endif

    I_PrintSystemInfo();

    C_PrintSDLVersions();

    iwadfile = D_FindIWAD();

    modifiedgame = false;

    for (int i = 0; i < MAXALIASES; i++)
    {
        aliases[i].name[0] = '\0';
        aliases[i].string[0] = '\0';
    }

    D_ProcessDehCommandLine();

    // Load configuration files before initializing other subsystems.
    M_LoadCVARs(packageconfig);

    if ((respawnmonsters = M_CheckParm("-respawn")))
        C_Output("A <b>-respawn</b> parameter was found on the command-line. Monsters will respawn.");
    else if ((respawnmonsters = M_CheckParm("-respawnmonsters")))
        C_Output("A <b>-respawnmonsters</b> parameter was found on the command-line. Monsters will respawn.");

    if ((nomonsters = M_CheckParm("-nomonsters")))
    {
        C_Output("A <b>-nomonsters</b> parameter was found on the command-line. No monsters will be spawned.");
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }

    if ((pistolstart = M_CheckParm("-pistolstart")))
        C_Output("A <b>-pistolstart</b> parameter was found on the command-line. The player will start each map with only a pistol.");

    if ((fastparm = M_CheckParm("-fast")))
        C_Output("A <b>-fast</b> parameter was found on the command-line. Monsters will be faster.");
    else if ((fastparm = M_CheckParm("-fastmonsters")))
        C_Output("A <b>-fastmonsters</b> parameter was found on the command-line. Monsters will be faster.");

    if ((devparm = M_CheckParm("-devparm")))
        C_Output("A <b>-devparm</b> parameter was found on the command-line. %s", s_D_DEVSTR);

    // turbo option
    if ((p = M_CheckParm("-turbo")))
    {
        int scale = 200;

        if (p < myargc - 1)
        {
            scale = atoi(myargv[p + 1]);

            if (scale >= 10 && scale <= 400 && scale != 100)
                C_Output("A <b>-turbo</b> parameter was found on the command-line. The player will be %i%% their normal speed.", scale);
            else
                scale = 100;
        }
        else
            C_Output("A <b>-turbo</b> parameter was found on the command-line. The player will be twice as fast.");

        if (scale != 100)
            G_SetMovementSpeed(scale);

        if (scale > turbo_default)
        {
            stat_cheated = SafeAdd(stat_cheated, 1);
            M_SaveCVARs();
        }
    }
    else
        G_SetMovementSpeed(turbo);

    // init subsystems
    V_Init();
    I_InitTimer();

    if (!stat_runs)
        C_Output("This is the first time <i><b>" PACKAGE_NAME "</b></i> has been run.");
    else if (stat_runs == 1)
        C_Output("<i><b>" PACKAGE_NAME "</b></i> has now been run twice.");
    else
    {
        char    *temp = commify(SafeAdd(stat_runs, 1));

        C_Output("<i><b>" PACKAGE_NAME "</b></i> has now been run %s times.", temp);

        free(temp);
    }

    if (!M_FileExists(packagewad))
        I_Error("%s can't be found.", packagewad);

    if (M_CheckParm("-nodeh"))
        C_Output("A <b>-nodeh</b> parameter was found on the command-line. All <b>DEHACKED</b> lumps will be ignored.");
    else if (M_CheckParm("-nobex"))
        C_Output("A <b>-nobex</b> parameter was found on the command-line. All <b>DEHACKED</b> lumps will be ignored.");

    p = M_CheckParmsWithArgs("-file", "-pwad", "-merge", 1, 1);

    if (!(choseniwad = D_CheckParms()))
    {
        if (iwadfile)
        {
            startuptimer = I_GetTimeMS();

            if (W_AddFile(iwadfile, false))
                stat_runs = SafeAdd(stat_runs, 1);
        }
        else if (!p)
        {
#if defined(_WIN32) || defined(__APPLE__)
            do
            {
                if ((choseniwad = D_OpenWADLauncher()) == -1)
                    I_Quit(false);
#if defined(_WIN32)
                else if (!choseniwad && !error && (!*wad || M_StringEndsWith(wad, ".wad")))
#else
                else if (!choseniwad && !error)
#endif
                {
                    char    buffer[256];

#if defined(_WIN32)
                    M_snprintf(buffer, sizeof(buffer), PACKAGE_NAME " couldn't find %s.", (*wad ? wad : "any IWADs"));

                    if (previouswad)
                        wad = M_StringDuplicate(previouswad);
#else
                    M_snprintf(buffer, sizeof(buffer), PACKAGE_NAME " couldn't find any IWADs.");
#endif

                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, PACKAGE_NAME, buffer, NULL);
                }
            } while (!choseniwad);
#endif

            stat_runs = SafeAdd(stat_runs, 1);
        }
    }

    M_SaveCVARs();

#if defined(_WIN32)
    if (keyboardscreenshot == KEY_PRINTSCREEN)
    {
        RegisterHotKey(NULL, 1, MOD_ALT, VK_SNAPSHOT);
        RegisterHotKey(NULL, 2, 0, VK_SNAPSHOT);
    }
#endif

    if (p > 0)
        do
        {
            for (p = p + 1; p < myargc && myargv[p][0] != '-'; p++)
            {
                char    *file = D_TryFindWADByName(myargv[p]);

                if (iwadfile)
                {
                    D_CheckSupportedPWAD(file);

                    if (W_MergeFile(file, false))
                    {
                        modifiedgame = true;

                        if (IWADRequiredByPWAD(file) != none)
                            pwadfile = M_StringDuplicate(leafname(file));
                    }
                }
                else
                {
                    GameMission_t   iwadrequired = IWADRequiredByPWAD(file);
                    char            fullpath[MAX_PATH];
                    char            *folder = M_ExtractFolder(file);

                    if (iwadrequired == none)
                        iwadrequired = doom2;

                    // try the current folder first
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, iwadsrequired[iwadrequired]);
                    D_IdentifyIWADByName(fullpath);

                    if (W_AddFile(fullpath, true))
                    {
                        iwadfile = M_StringDuplicate(fullpath);
                        iwadfolder = M_StringDuplicate(folder);
                        D_CheckSupportedPWAD(file);

                        if (W_MergeFile(file, false))
                        {
                            modifiedgame = true;

                            if (IWADRequiredByPWAD(file) != none)
                                pwadfile = M_StringDuplicate(leafname(file));
                        }
                    }
                    else
                    {
                        // otherwise try the iwadfolder CVAR
                        M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", iwadfolder, iwadsrequired[iwadrequired]);
                        D_IdentifyIWADByName(fullpath);

                        if (W_AddFile(fullpath, true))
                        {
                            iwadfile = M_StringDuplicate(fullpath);
                            D_CheckSupportedPWAD(file);

                            if (W_MergeFile(file, false))
                            {
                                modifiedgame = true;

                                if (IWADRequiredByPWAD(file) != none)
                                    pwadfile = M_StringDuplicate(leafname(file));
                            }
                        }
                        else
                        {
                            // still nothing? try some common installation folders
                            if (W_AddFile(D_FindWADByName(iwadsrequired[iwadrequired]), true))
                            {
                                iwadfile = M_StringDuplicate(fullpath);
                                D_CheckSupportedPWAD(file);

                                if (W_MergeFile(file, false))
                                {
                                    modifiedgame = true;

                                    if (IWADRequiredByPWAD(file) != none)
                                        pwadfile = M_StringDuplicate(leafname(file));
                                }
                            }
                        }
                    }

                    free(folder);
                }

            }
        } while ((p = M_CheckParmsWithArgs("-file", "-pwad", "-merge", 1, p)));

    if (!iwadfile && !modifiedgame && !choseniwad)
        I_Error(PACKAGE_NAME " couldn't find any IWADs.");

    W_Init();

    FREEDM = (W_CheckNumForName("FREEDM") >= 0);

    DMENUPIC = (W_CheckNumForName("DMENUPIC") >= 0);
    M_DOOM = (W_CheckMultipleLumps("M_DOOM") > 1);
    M_EPISOD = (W_CheckMultipleLumps("M_EPISOD") > 1);
    M_GDHIGH = (W_CheckMultipleLumps("M_GDHIGH") > 1);
    M_GDLOW = (W_CheckMultipleLumps("M_GDLOW") > 1);
    M_LOADG = (W_CheckMultipleLumps("M_LOADG") > 1);
    M_LSCNTR = (W_CheckMultipleLumps("M_LSCNTR") > 1);
    M_MSENS = (W_CheckMultipleLumps("M_MSENS") > 1);
    M_MSGOFF = (W_CheckMultipleLumps("M_MSGOFF") > 1);
    M_MSGON = (W_CheckMultipleLumps("M_MSGON") > 1);
    M_NEWG = (W_CheckMultipleLumps("M_NEWG") > 1);
    M_NGAME = (W_CheckMultipleLumps("M_NGAME") > 1);
    M_NMARE = (W_CheckMultipleLumps("M_NMARE") > 1);
    M_OPTTTL = (W_CheckMultipleLumps("M_OPTTTL") > 1);
    M_PAUSE = (W_CheckMultipleLumps("M_PAUSE") > 1);
    M_SAVEG = (W_CheckMultipleLumps("M_SAVEG") > 1);
    M_SKILL = (W_CheckMultipleLumps("M_SKILL") > 1);
    M_SKULL1 = (W_CheckMultipleLumps("M_SKULL1") > 1);
    M_SVOL = (W_CheckMultipleLumps("M_SVOL") > 1);
    STBAR = W_CheckMultipleLumps("STBAR");
    STCFN034 = (W_CheckMultipleLumps("STCFN034") > 1);
    STYSNUM0 = (W_CheckMultipleLumps("STYSNUM0") > 1);
    TITLEPIC = (W_CheckNumForName("TITLEPIC") >= 0);
    WISCRT2 = (W_CheckMultipleLumps("WISCRT2") > 1);
    DSSECRET = (W_CheckNumForName("DSSECRET") >= 0);

    I_InitGraphics();

    I_InitGamepad();

    D_IdentifyVersion();
    D_ProcessDehInWad();

    if (!M_StringCompare(s_VERSION, PACKAGE_NAMEANDVERSIONSTRING))
        I_Error("The wrong version of %s was found.", packagewad);

    D_SetGameDescription();

    if (nerve && expansion == 2)
        gamemission = pack_nerve;
    else if (gamemission == doom && !sigil && episode == 5)
    {
        episode = 1;
        M_SaveCVARs();
    }

    D_SetSaveGameFolder(true);

    C_Output("Screenshots will be saved in <b>%s</b>.", screenshotfolder);

    // Check for -file in shareware
    if (modifiedgame)
    {
        if (gamemode == shareware)
            I_Error("You can't load PWADs with DOOM1.WAD.");

        // Check for fake IWAD with right name,
        // but w/o all the lumps of the registered version.
        if (gamemode == registered)
        {
            // These are the lumps that will be checked in IWAD,
            // if any one is not present, execution will be aborted.
            char name[23][9] =
            {
                "E2M1", "E2M2", "E2M3", "E2M4", "E2M5", "E2M6", "E2M7", "E2M8", "E2M9",
                "E3M1", "E3M3", "E3M3", "E3M4", "E3M5", "E3M6", "E3M7", "E3M8", "E3M9",
                "DPHOOF", "BFGGA0", "HEADA1", "CYBRA1", "SPIDA1D1"
            };

            for (int i = 0; i < 23; i++)
                if (W_CheckNumForName(name[i]) < 0)
                    I_Error("This is not the registered version of DOOM.WAD.");
        }
    }

    // get skill/episode/map from parms
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;

    if ((p = M_CheckParmsWithArgs("-skill", "-skilllevel", "", 1, 1)))
    {
        int temp = myargv[p + 1][0] - '1';

        if (temp >= sk_baby && temp <= sk_nightmare)
        {
            char    *string = titlecase(*skilllevels[temp]);

            startskill = (skill_t)temp;
            skilllevel = startskill + 1;
            M_SaveCVARs();

            strreplace(string, ".", "");
            strreplace(string, "!", "");

            C_Output("A <b>-%s</b> parameter was found on the command-line. The skill level is now <i><b>%s.</b></i>",
                myargv[p], string);
            free(string);
        }
    }

    if ((p = M_CheckParmWithArgs("-episode", 1, 1)) && gamemode != commercial)
    {
        int temp = myargv[p + 1][0] - '0';

        if ((gamemode == shareware && temp == 1) || (temp >= 1 && ((gamemode == registered && temp <= 3)
            || (gamemode == retail && temp <= 4) || (sigil && temp <= 5))))
        {
            startepisode = temp;
            episode = temp;
            M_SaveCVARs();

            if (gamemode == commercial)
                M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", startmap);
            else
                M_snprintf(lumpname, sizeof(lumpname), "E%iM%i", startepisode, startmap);

            autostart = true;
            C_Output("An <b>-episode</b> parameter was found on the command-line. The episode is now <i><b>%s.</b></i>",
                *episodes[episode - 1]);
        }
    }

    if ((p = M_CheckParmWithArgs("-expansion", 1, 1)) && gamemode == commercial)
    {
        int temp = myargv[p + 1][0] - '0';

        if (temp <= (nerve ? 2 : 1))
        {
            gamemission = (temp == 1 ? doom2 : pack_nerve);
            expansion = temp;
            M_SaveCVARs();
            M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", startmap);
            autostart = true;
            C_Output("An <b>-expansion</b> parameter was found on the command-line. The expansion is now <i><b>%s.</b></i>",
                *expansions[expansion - 1]);
        }
    }

    if ((p = M_CheckParmWithArgs("-warp", 1, 1)))
        C_Output("A <b>-warp</b> parameter was found on the command-line.");
    else if ((p = M_CheckParmWithArgs("+map", 1, 1)))
        C_Output("A <b>+map</b> parameter was found on the command-line.");

    if (p)
    {
        if (gamemode == commercial)
        {
            if (strlen(myargv[p + 1]) == 5 && toupper(myargv[p + 1][0]) == 'M' && toupper(myargv[p + 1][1]) == 'A'
                && toupper(myargv[p + 1][2]) == 'P' && isdigit((int)myargv[p + 1][3]) && isdigit((int)myargv[p + 1][4]))
                startmap = (myargv[p + 1][3] - '0') * 10 + myargv[p + 1][4] - '0';
            else
                startmap = atoi(myargv[p + 1]);

            M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", startmap);
        }
        else
        {
            if (strlen(myargv[p + 1]) == 4 && toupper(myargv[p + 1][0]) == 'E' && isdigit((int)myargv[p + 1][1])
                && toupper(myargv[p + 1][2]) == 'M' && isdigit((int)myargv[p + 1][3]))
            {
                startepisode = myargv[p + 1][1] - '0';
                startmap = myargv[p + 1][3] - '0';
            }
            else
            {
                startepisode = myargv[p + 1][0] - '0';

                if (p + 2 < myargc)
                    startmap = myargv[p + 2][0] - '0';
            }

            M_snprintf(lumpname, sizeof(lumpname), "E%iM%i", startepisode, startmap);
        }

        if ((BTSX && W_CheckMultipleLumps(lumpname) > 1) || W_CheckNumForName(lumpname) >= 0)
        {
            autostart = true;

            stat_cheated = SafeAdd(stat_cheated, 1);
            M_SaveCVARs();
        }
    }

    M_Init();

    R_Init();

    P_Init();

    S_Init();

    HU_Init();

    ST_Init();

    AM_Init();

    C_Init();

    if ((startloadgame = ((p = M_CheckParmWithArgs("-loadgame", 1, 1)) ? atoi(myargv[p + 1]) : -1)) >= 0 && startloadgame <= 5)
    {
        menuactive = false;
        splashscreen = false;
        I_InitKeyboard();

        if (alwaysrun)
            C_StrCVAROutput(stringize(alwaysrun), "on");

        G_LoadGame(P_SaveGameFile(startloadgame));
    }

    fineprintlump = W_CacheLumpName("FINEPRNT");
    splashpal = W_CacheLumpName("SPLSHPAL");

    for (int i = 0; i < 18; i++)
    {
        char    buffer[9];

        M_snprintf(buffer, sizeof(buffer), "DRLOGO%02d", i + 1);
        logolump[i] = W_CacheLumpName(buffer);
    }

    if (autosigil)
    {
        titlelump = W_CacheLastLumpName((TITLEPIC ? "TITLEPIC" : (DMENUPIC ? "DMENUPIC" : "INTERPIC")));
        creditlump = W_CacheLastLumpName("CREDIT");
    }
    else
    {
        titlelump = W_CacheLumpName((TITLEPIC ? "TITLEPIC" : (DMENUPIC ? "DMENUPIC" : "INTERPIC")));
        creditlump = W_CacheLumpName("CREDIT");
    }

    if ((unity = (TITLEPIC && lumpinfo[W_GetLastNumForName("TITLEPIC")]->wadfile->type == IWAD
        && SHORT(titlelump->width) == UNITYWIDTH)))
        C_Warning(1, "Certain graphics in this IWAD are cropped to fit <i><b>" PACKAGE_NAME "'s</b></i> 4:3 aspect ratio.");

    if (gameaction != ga_loadgame)
    {
        if (autostart)
        {
            menuactive = false;
            splashscreen = false;
            I_InitKeyboard();

            if (alwaysrun)
                C_StrCVAROutput(stringize(alwaysrun), "on");

            C_Output("Warping to %s...", lumpname);
            G_DeferredInitNew(startskill, startepisode, startmap);
        }
#if SCREENSCALE == 1
        else
        {
            menuactive = false;
            splashscreen = false;
            D_StartTitle(1);
        }
#else
        else if (M_CheckParm("-nosplash"))
        {
            menuactive = false;
            splashscreen = false;
            D_StartTitle(1);
        }
        else
            D_StartTitle(0);
#endif
    }

    seconds = striptrailingzero((I_GetTimeMS() - startuptimer) / 1000.0f, 1);
    C_Output("Startup took %s second%s to complete.", seconds, (M_StringCompare(seconds, "1") ? "" : "s"));
    free(seconds);

    // Ty 04/08/98 - Add 5 lines of misc. data, only if non-blank
    // The expectation is that these will be set in a .bex file
    if ((*startup1 || *startup2 || *startup3 || *startup4 || *startup5) && !FREEDOOM)
    {
        C_AddConsoleDivider();

        if (*startup1)
            D_ParseStartupString(startup1);

        if (*startup2)
            D_ParseStartupString(startup2);

        if (*startup3)
            D_ParseStartupString(startup3);

        if (*startup4)
            D_ParseStartupString(startup4);

        if (*startup5)
            D_ParseStartupString(startup5);
    }
}

//
// D_DoomMain
//
void D_DoomMain(void)
{
    D_DoomMainSetup();          // CPhipps - setup out of main execution stack

    D_DoomLoop();               // never returns
}
