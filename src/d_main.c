/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

========================================================================
*/

#define __STDC_WANT_LIB_EXT1__  1

#include <time.h>

#if defined(_WIN32)
#pragma comment(lib, "winmm.lib")

#include <Windows.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <ShellAPI.h>
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
#include "i_gamecontroller.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_setup.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "version.h"
#include "w_merge.h"
#include "w_wad.h"
#include "wi_stuff.h"

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

#define FADECOUNT    8
#define FADETICS    25

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
char            *savegamefolder;

char            *autoloadfolder;
char            *autoloadiwadsubfolder;
char            *autoloadpwadsubfolder;

char            *pwadfile = "";

char            *configfile;
char            *resourcewad;

static char     dehwarning[256];

#if defined(_WIN32)
char            *previouswad;
#endif

bool            devparm;                // started game with -devparm
bool            fastparm;               // checkparm of -fast
bool            freeze;
bool            nomonsters;             // checkparm of -nomonsters
bool            pistolstart;            // [BH] checkparm of -pistolstart
bool            regenhealth;
bool            respawnitems;
bool            respawnmonsters;        // checkparm of -respawn
bool            solonet;                // checkparm of -solonet

skill_t         startskill;
int             startepisode;
static int      startmap;
bool            autostart;

bool            advancetitle;
bool            dowipe = false;
static bool     forcewipe;

static byte     fadescreen[MAXSCREENAREA];
int             fadecount = 0;

bool            splashscreen = true;

bool            realframe;
static bool     error;

struct tm       gamestarttime;

#if defined(_WIN32)
extern HANDLE   CapFPSEvent;
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
void D_FadeScreen(bool screenshot)
{
    if ((!fade && !screenshot) || togglingvanilla || fadecount)
        return;

    memcpy(fadescreen, screens[0], SCREENAREA);
    fadecount = FADECOUNT;
}

//
// D_UpdateFade
//
static void D_UpdateFade(void)
{
    static byte *tinttab;
    static int  fadewait;
    const int   tics = I_GetTimeMS();

    if (fadewait < tics)
    {
        byte *tinttabs[FADECOUNT + 1] =
        {
            tinttab90, tinttab80, tinttab70,
            tinttab60, tinttab50, tinttab40,
            tinttab30, tinttab20, tinttab10
        };

        fadewait = tics + FADETICS;
        tinttab = tinttabs[fadecount--];
    }

    for (int i = 0; i < SCREENAREA; i++)
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

    for (double i = 0.95; i >= 0.0; i -= 0.05)
    {
        I_SetPaletteWithBrightness(&PLAYPAL[st_palette * 768], i);
        I_SetExternalAutomapPalette();
        I_SetMusicVolume((int)(current_music_volume * i));
        I_Sleep(20);
        blitfunc();
    }

    memset(screens[0], nearestblack, SCREENAREA);
    blitfunc();
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t wipegamestate = GS_TITLESCREEN;

void D_Display(void)
{
    static bool         pausedstate;
    static gamestate_t  oldgamestate = GS_NONE;
    static int          saved_gametime = -1;
    int                 nowtime;
    int                 tics;
    int                 wipestart;
    bool                done;

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
        fadecount = 0;

        if (melt)
            wipe_StartScreen();
        else
            D_FadeScreen(false);

        if (forcewipe)
            forcewipe = false;
        else
        {
            menuactive = false;
            R_ExecuteSetViewSize();
        }
    }

    if (gamestate != GS_LEVEL)
    {
        if (gamestate != oldgamestate)
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

        if (mapwindow || automapactive)
            AM_Drawer();

        ST_Drawer((viewheight == SCREENHEIGHT), true);

        // see if the border needs to be initially drawn
        if (oldgamestate != GS_LEVEL && viewwidth != SCREENWIDTH)
            R_FillBackScreen();

        // see if the border needs to be updated to the screen
        if (!automapactive)
        {
            if (viewwidth != SCREENWIDTH)
                R_DrawViewBorder();

            if (r_detail == r_detail_low)
                postprocessfunc(viewwindowx, viewwindowy * SCREENWIDTH, viewwindowx + viewwidth,
                    (viewwindowy + viewheight) * SCREENWIDTH, lowpixelwidth, lowpixelheight);
        }

        HU_Drawer();
    }

    oldgamestate = wipegamestate = gamestate;

    // draw pause pic
    if ((pausedstate = paused))
    {
        M_DrawMenuBackground();

        if (M_PAUSE)
        {
            patch_t *patch = W_CacheLumpName("M_PAUSE");

            V_DrawPatchWithShadow((VANILLAWIDTH - SHORT(patch->width)) / 2,
                (VANILLAHEIGHT - SHORT(patch->height)) / 2, patch, false);
        }
        else
            M_DrawCenteredString((VANILLAHEIGHT - 16) / 2, s_M_PAUSED);
    }

    if (loadaction != ga_nothing)
        G_LoadedGameMessage();

    if (!dowipe || !melt)
    {
        if (!paused && !menuactive)
        {
            if (vid_showfps && !dowipe && !splashscreen && framespersecond)
                C_UpdateFPSOverlay();

            if (gamestate == GS_LEVEL)
            {
                if (timeremaining)
                    C_UpdateTimerOverlay();

                if (viewplayer->cheats & CF_MYPOS)
                    C_UpdatePlayerPositionOverlay();

                if (am_path && (automapactive || mapwindow))
                    C_UpdatePathOverlay();

                if (am_playerstats && (automapactive || mapwindow))
                    C_UpdatePlayerStatsOverlay();
            }
        }

        if (consoleheight)
            C_Drawer();

        // menus go directly to the screen
        M_Drawer();

        if (drawdisk)
            HU_DrawDisk();

        if (fadecount)
            D_UpdateFade();

        // normal update
        blitfunc();
        mapblitfunc();

#if defined(_WIN32)
        if (CapFPSEvent)
            WaitForSingleObject(CapFPSEvent, 1000);
#endif

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
    player_t    player;


    viewplayer = &player;
    memset(viewplayer, 0, sizeof(*viewplayer));

    R_ExecuteSetViewSize();

    while (true)
    {
        TryRunTics();       // will run at least one tic

        if (splashscreen)
            D_SplashDrawer();
        else
        {
            S_UpdateSounds();   // move positional sounds
            D_Display();        // update display, next frame, with current state
        }
    }
}

//
//  TITLE LOOP
//
int             titlesequence = 0;
int             pagetic = 3 * TICRATE;
int             logotic = 3 * TICRATE;

static patch_t  *pagelump;
patch_t         *creditlump;

static int      pillarboxcolor;

static patch_t  *fineprintlump;
static patch_t  *logolump[18];
static patch_t  *titlelump;
static byte     *splashpal;
static short    fineprintx;
static short    fineprinty;
static short    fineprintwidth;
static short    fineprintheight;
static short    logox;
static short    logoy;

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
        advancetitle = true;

        if (splashscreen)
        {
            memset(screens[0], nearestblack, SCREENAREA);
            D_FadeScreen(false);
        }
    }
}

//
// D_SplashDrawer
//
void D_SplashDrawer(void)
{
    patch_t *patch = logolump[BETWEEN(0, 94 - logotic, 17)];

    gamestate = GS_TITLESCREEN;
    memset(screens[0], BLACK, SCREENAREA);
    V_DrawBigPatch(logox, logoy, SHORT(patch->width), SHORT(patch->height), patch);
    V_DrawBigPatch(fineprintx, fineprinty, fineprintwidth, fineprintheight, fineprintlump);
    I_SetSimplePalette(&splashpal[pagetic < 9 ? (9 - pagetic) * 768 : (pagetic <= 94 ? 0 : (pagetic - 94) * 768)]);
    blitfunc();
}

//
// D_PageDrawer
//
void D_PageDrawer(void)
{
    V_DrawPagePatch(0, pagelump);
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

    if (titlesequence == 1)
    {
        static bool flag = true;

        if (splashscreen)
        {
            I_SetPalette(PLAYPAL);
            splashscreen = false;
            I_Sleep(1000);
        }

        if (flag)
        {
            flag = false;
            I_InitKeyboard();

            if (alwaysrun)
                C_StrCVAROutput(stringize(alwaysrun), "on");
        }

        if (pagelump == creditlump)
            forcewipe = true;

        pagelump = titlelump;
        pillarboxcolor = FindDominantEdgeColor(pagelump);
        pagetic = PAGETICS;

        M_SetWindowCaption();
        S_StartMusic(gamemode == commercial ? mus_dm2ttl : mus_intro);

        if (devparm)
            C_ShowConsole();
    }
    else if (titlesequence == 2)
    {
        forcewipe = true;
        pagelump = creditlump;
        pillarboxcolor = FindDominantEdgeColor(pagelump);
        pagetic = PAGETICS;
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

    advancetitle = true;
}

#define MAXDEHFILES 16

static char dehfiles[MAXDEHFILES][MAX_PATH];
static int  dehfilecount;

bool        dehfileignored = false;

static bool DehFileProcessed(char *path)
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
    char    *dehpath = M_StringDuplicate(path);

    if (M_StringEndsWith(path, ".wad"))
        dehpath = M_StringReplace(path, ".wad", ext);
    else if (M_StringEndsWith(path, ".iwad"))
        dehpath = M_StringReplace(path, ".iwad", ext);
    else if (M_StringEndsWith(path, ".pwad"))
        dehpath = M_StringReplace(path, ".pwad", ext);

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
    char    filename[MAX_PATH];
    bool    present;
} loaddehlast_t;

// [BH] A list of DeHackEd files to load last
static loaddehlast_t loaddehlast[] =
{
    { "1_VORTEX.deh"         },
    { "2_MARKV.deh"          },
    { "3_HELLST.deh"         },
    { "3_REAPER.deh"         },
    { "4_HAR.deh"            },
    { "5_GRNADE.deh"         },
    { "6_LIGHT.deh"          },
    { "7_GAUSS.deh"          },
    { "VORTEX_DoomRetro.deh" },
    { ""                     }
};

static void LoadDehFile(char *path)
{
    int     i = 0;
    char    *dehpath;
    char    *temp = leafname(path);

    while (*loaddehlast[i].filename)
    {
        if (M_StringEndsWith(temp, loaddehlast[i].filename))
        {
            loaddehlast[i].present = true;
            return;
        }

        i++;
    }

    if ((dehpath = FindDehPath(path, ".bex", ".[Bb][Ee][Xx]")))
    {
        if (!DehFileProcessed(dehpath))
        {
            if (HasDehackedLump(path))
            {
                M_snprintf(dehwarning, sizeof(dehwarning), BOLD("%s") " wasn't loaded.", GetCorrectCase(dehpath));
                dehfileignored = true;
            }
            else
                D_ProcessDehFile(dehpath, 0, true);

            if (dehfilecount < MAXDEHFILES)
            {
                M_StringCopy(dehfiles[dehfilecount], dehpath, sizeof(dehfiles[0]));
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
            {
                M_snprintf(dehwarning, sizeof(dehwarning), BOLD("%s") " wasn't loaded.", GetCorrectCase(dehpath));
                dehfileignored = true;
            }
            else
                D_ProcessDehFile(dehpath, 0, true);

            if (dehfilecount < MAXDEHFILES)
            {
                M_StringCopy(dehfiles[dehfilecount], dehpath, sizeof(dehfiles[0]));
                dehfilecount++;
            }
        }
    }
}

static void LoadCfgFile(char *path)
{
    char    *cfgpath = M_StringDuplicate(path);

    if (M_StringEndsWith(path, ".wad"))
        cfgpath = M_StringReplace(path, ".wad", ".cfg");
    else if (M_StringEndsWith(path, ".iwad"))
        cfgpath = M_StringReplace(path, ".iwad", ".cfg");
    else if (M_StringEndsWith(path, ".pwad"))
        cfgpath = M_StringReplace(path, ".pwad", ".cfg");

    if (M_FileExists(cfgpath))
        M_LoadCVARs(cfgpath);
}

static bool D_IsDOOM1IWAD(char *filename)
{
    char    *file = leafname(filename);

    return (M_StringCompare(file, "DOOM.WAD")
        || M_StringCompare(file, "DOOM1.WAD")
        || M_StringCompare(file, "DOOMU.WAD")
        || M_StringCompare(file, "BFGDOOM.WAD")
        || M_StringCompare(file, "UNITYDOOM.WAD")
        || M_StringCompare(file, "DOOMBFG.WAD")
        || M_StringCompare(file, "DOOMUNITY.WAD"));
}

static bool D_IsDOOM2IWAD(char *filename)
{
    char    *file = leafname(filename);

    return (M_StringCompare(file, "DOOM2.WAD")
        || M_StringCompare(file, "DOOM2F.WAD")
        || M_StringCompare(file, "BFGDOOM2.WAD")
        || M_StringCompare(file, "UNITYDOOM2.WAD")
        || M_StringCompare(file, "DOOM2BFG.WAD")
        || M_StringCompare(file, "DOOM2UNITY.WAD"));
}

bool D_IsDOOMIWAD(char *filename)
{
    char    *file = leafname(filename);

    return (D_IsDOOM1IWAD(filename)
        || D_IsDOOM2IWAD(filename)
        || M_StringCompare(file, "chex.wad")
        || M_StringCompare(file, "rekkrsa.wad"));
}

static bool D_IsUnsupportedIWAD(char *filename)
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
        if (M_StringCompare(leafname(filename), unsupported[i].iwad))
        {
            char    buffer[1024];

            M_snprintf(buffer, sizeof(buffer), DOOMRETRO_NAME " doesn't support %s.\n", unsupported[i].title);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, DOOMRETRO_NAME, buffer, NULL);

#if defined(_WIN32)
            if (previouswad)
                wad = M_StringDuplicate(previouswad);
#endif

            error = true;
            return true;
        }

    return false;
}

static bool D_IsCfgFile(char *filename)
{
    return M_StringEndsWith(filename, ".cfg");
}

static bool D_IsDehFile(char *filename)
{
    return (M_StringEndsWith(filename, ".deh") || M_StringEndsWith(filename, ".bex"));
}

static void D_CheckSupportedPWAD(char *filename)
{
    char    *leaf = leafname(filename);

    if (M_StringCompare(leaf, "SIGIL_v1_21.wad")
        || M_StringCompare(leaf, "SIGIL_v1_2.wad")
        || M_StringCompare(leaf, "SIGIL_v1_1.wad")
        || M_StringCompare(leaf, "SIGIL_v1_0.wad")
        || M_StringCompare(leaf, "SIGIL.wad"))
    {
        sigil = true;
        episode = 5;
    }
    else if (M_StringCompare(leaf, "NERVE.WAD"))
    {
        nerve = true;
        expansion = 2;
    }
    else if (M_StringCompare(leaf, "chex.wad"))
        chex = chex1 = true;
    else if (M_StringCompare(leaf, "chex2.wad"))
        chex = chex2 = true;
    else if (M_StringCompare(leaf, "aaliens.wad"))
        moreblood = true;
    else if (M_StringCompare(leaf, "btsx_e1.wad"))
        BTSX = BTSXE1 = true;
    else if (M_StringCompare(leaf, "btsx_e1a.wad"))
        BTSX = BTSXE1 = BTSXE1A = true;
    else if (M_StringCompare(leaf, "btsx_e1b.wad"))
        BTSX = BTSXE1 = BTSXE1B = true;
    else if (M_StringCompare(leaf, "btsx_e2a.wad"))
        BTSX = BTSXE2 = BTSXE2A = true;
    else if (M_StringCompare(leaf, "btsx_e2b.wad"))
        BTSX = BTSXE2 = BTSXE2B = true;
    else if (M_StringCompare(leaf, "e1m4b.wad"))
        E1M4B = true;
    else if (M_StringCompare(leaf, "e1m8b.wad"))
        E1M8B = true;
    else if (M_StringCompare(leaf, "KDiKDi_A.wad"))
        KDIKDIZD = KDIKDIZDA = true;
    else if (M_StringCompare(leaf, "KDiKDi_B.wad"))
        KDIKDIZD = KDIKDIZDB = true;
    else if (M_StringCompare(leaf, "one-humanity.wad"))
        onehumanity = true;
    else if (M_StringCompare(leaf, "d1spfx18.wad")
        || M_StringCompare(leaf, "d2spfx18.wad"))
        sprfix18 = true;
    else if (M_StringCompare(leaf, "eviternity.wad"))
        eviternity = true;
    else if (M_StringCompare(leaf, "d4v.wad"))
        doom4vanilla = true;
    else if (M_StringCompare(leaf, "REKKR.wad"))
        REKKR = true;
    else if (M_StringCompare(leaf, "rekkrsa.wad"))
        REKKR = REKKRSA = true;
    else if (M_StringCompare(leaf, "REKKRSL.wad")
        || M_StringCompare(leaf, "REKKRSL.iwad"))
        REKKR = REKKRSL = true;
    else if (M_StringCompare(leaf, "ar.wad"))
        anomalyreport = true;
    else if (M_StringCompare(leaf, "arrival.wad"))
        arrival = true;
    else if (M_StringCompare(leaf, "dbimpact.wad"))
        dbimpact = true;
    else if (M_StringCompare(leaf, "deathless.wad"))
        deathless = true;
    else if (M_StringCompare(leaf, "DoomZero.wad"))
        doomzero = true;
    else if (M_StringCompare(leaf, "earthless_pr.wad"))
        earthless = true;
    else if (M_StringCompare(leaf, "neis.wad"))
        neis = true;
    else if (M_StringCompare(leaf, "TVR!.wad"))
        revolution = true;
    else if (M_StringCompare(leaf, "syringe.wad"))
        syringe = true;

    if (BTSX || REKKR)
        moreblood = true;
}

static bool D_IsUnsupportedPWAD(char *filename)
{
    return (error = (M_StringCompare(leafname(filename), DOOMRETRO_RESOURCEWAD)));
}

static bool D_CheckParms(void)
{
    bool    result = false;

    if (myargc == 2
        && (M_StringEndsWith(myargv[1], ".wad") || M_StringEndsWith(myargv[1], ".iwad") || M_StringEndsWith(myargv[1], ".pwad")))
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
                if (D_IsDOOM1IWAD(myargv[1]) && IsUltimateDOOM(myargv[1]))
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
                                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_v1_0.wad");

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
                else if (D_IsDOOM2IWAD(myargv[1]))
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
            gamemission_t   iwadrequired = IWADRequiredByPWAD(myargv[1]);
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
        }
        else if (KDIKDIZD)
        {
            char    fullpath[MAX_PATH];

            if (KDIKDIZDA && !KDIKDIZDB)
            {
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "KDiKDi_B.wad");
                result = W_MergeFile(fullpath, true);
            }
            else if (!KDIKDIZDA && KDIKDIZDB)
            {
                M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "KDiKDi_A.wad");
                result = W_MergeFile(fullpath, true);
            }
        }

        free(folder);
    }

    return result;
}

#if defined(_WIN32) || defined(__APPLE__)
static char *invalidwad;

static int D_OpenWADLauncher(void)
{
    int             iwadfound = -1;
    bool            fileopenedok;

#if defined(_WIN32)
    OPENFILENAME    ofn;
    char            szFile[4096];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    M_StringCopy(szFile, (invalidwad ? invalidwad : wad), sizeof(szFile));
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "IWAD and/or PWAD(s) (*.wad)\0*.WAD;*.IWAD;*.PWAD;*.DEH;*.BEX;*.CFG\0";
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
        bool    onlyoneselected;
        bool    guess = false;

#if defined(__APPLE__)
        NSArray *urls = [panel URLs];
#endif

        iwadfound = 0;

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

            if (!M_StringEndsWith(file, ".wad") && !M_StringEndsWith(file, ".iwad")
                && !M_StringEndsWith(file, ".pwad") && !M_StringEndsWith(file, ".deh")
                && !M_StringEndsWith(file, ".bex") && !M_StringEndsWith(file, ".cfg")
                && !strchr(file, '.'))
            {
                char    *temp = M_StringDuplicate(file);

                file = M_StringJoin(temp, ".wad", NULL);

                if (!M_FileExists(file))
                    file = M_StringJoin(temp, ".iwad", NULL);

                if (!M_FileExists(file))
                    file = M_StringJoin(temp, ".pwad", NULL);

                free(temp);
            }

#if defined(_WIN32)
            // if WAD doesn't exist, it was entered manually and there may be a typo, so guess what was intended
            if (!M_FileExists(file) && strlen(leafname(file)) > 2)
            {
                char    *temp = W_GuessFilename(folder, leafname(file));

                if (temp)
                {
                    guess = true;

                    if (!M_StringEndsWith(temp, leafname(file)))
                        C_Warning(0, "\"%s\" wasn't found so " BOLD("%s") " was loaded instead.",
                            leafname((char *)ofn.lpstrFile), temp);

                    file = M_StringDuplicate(temp);
                    wad = M_StringDuplicate(leafname(temp));
                    free(temp);
                }
                else
                {
                    error = true;
                    invalidwad = M_StringDuplicate((char *)ofn.lpstrFile);
                }
            }
            else
                wad = M_StringDuplicate(leafname(file));
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
                    if (D_IsDOOM1IWAD(file) && IsUltimateDOOM(file))
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
                                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "SIGIL_v1_0.wad");

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
                    else if (D_IsDOOM2IWAD(file))
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
                gamemission_t   iwadrequired = IWADRequiredByPWAD(file);
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
            }
            else if (KDIKDIZD)
            {
                char    fullpath[MAX_PATH];

                if (KDIKDIZDA && !KDIKDIZDB)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "KDiKDi_B.wad");
                    W_MergeFile(fullpath, true);
                }
                else if (!KDIKDIZDA && KDIKDIZDB)
                {
                    M_snprintf(fullpath, sizeof(fullpath), "%s" DIR_SEPARATOR_S "%s", folder, "KDiKDi_A.wad");
                    W_MergeFile(fullpath, true);
                }
            }

            free(folder);
        }
        else
        {
            // more than one file was selected
            bool    isDOOM2 = false;
            bool    sharewareiwad = false;

#if defined(_WIN32)
            LPSTR   iwadpass1 = ofn.lpstrFile;
            LPSTR   iwadpass2 = ofn.lpstrFile;
            LPSTR   pwadpass1 = ofn.lpstrFile;
            LPSTR   pwadpass2 = ofn.lpstrFile;
            LPSTR   cfgpass = ofn.lpstrFile;
            LPSTR   dehpass = ofn.lpstrFile;

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
                        isDOOM2 = D_IsDOOM2IWAD(iwadpass1);

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
                            isDOOM2 = D_IsDOOM2IWAD(iwadpass2);

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
                        gamemission_t   iwadrequired = IWADRequiredByPWAD(fullpath);

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
                    bool    mapspresent = false;

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
            else
                I_Error("Other files can’t be loaded with the shareware version of DOOM.");
        }
    }

    return iwadfound;
}
#endif

static void D_ProcessDehOnCmdLine(void)
{
    int p = M_CheckParm("-deh");
    int j = 0;

    if (p || (p = M_CheckParm("-bex")))
    {
        bool    deh = true;

        while (++p < myargc)
            if (*myargv[p] == '-')
                deh = (M_StringCompare(myargv[p], "-deh") || M_StringCompare(myargv[p], "-bex"));
            else if (deh)
                D_ProcessDehFile(myargv[p], 0, false);
    }

    while (*loaddehlast[j].filename)
    {
        if (loaddehlast[j].present)
            D_ProcessDehFile(loaddehlast[j].filename, 0, false);

        j++;
    }
}

static void D_ProcessDehInWad(void)
{
    const bool  process = (!M_CheckParm("-nodeh") && !M_CheckParm("-nobex"));
    int         j = 0;

    if (*dehwarning)
        C_Warning(1, dehwarning);

    if (doom4vanilla)
    {
        if (process)
            for (int i = 0; i < numlumps; i++)
                if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
                    && !M_StringEndsWith(lumpinfo[i]->wadfile->path, DOOMRETRO_RESOURCEWAD)
                    && !M_StringEndsWith(lumpinfo[i]->wadfile->path, "D4V.WAD"))
                    D_ProcessDehFile(NULL, i, false);

        for (int i = 0; i < numlumps; i++)
            if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
                && M_StringEndsWith(lumpinfo[i]->wadfile->path, "D4V.WAD"))
            {
                D_ProcessDehFile(NULL, i, false);
                break;
            }
    }

    if (chex1)
        D_ProcessDehFile(NULL, W_GetNumForName("CHEXBEX"), true);

    if (process)
        for (int i = 0; i < numlumps; i++)
            if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
                && !M_StringEndsWith(lumpinfo[i]->wadfile->path, DOOMRETRO_RESOURCEWAD))
                D_ProcessDehFile(NULL, i, false);

    for (int i = numlumps - 1; i >= 0; i--)
        if (M_StringCompare(lumpinfo[i]->name, "DEHACKED")
            && M_StringEndsWith(lumpinfo[i]->wadfile->path, DOOMRETRO_RESOURCEWAD))
        {
            D_ProcessDehFile(NULL, i, false);
            break;
        }

    while (*loaddehlast[j].filename)
    {
        if (loaddehlast[j].present)
            D_ProcessDehFile(loaddehlast[j].filename, 0, false);

        j++;
    }
}

static void D_ParseStartupString(const char *string)
{
    const size_t    len = strlen(string);

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
    bool    autoloading = false;
    char    lumpname[6];
    char    *appdatafolder = M_GetAppDataFolder();
    char    *iwadfile;
    int     startloadgame;
    char    *resourcefolder = M_GetResourceFolder();
    time_t  now = time(NULL);

#if defined(_WIN32)
    localtime_s(&gamestarttime, &now);
#else
    localtime_r(&now, &gamestarttime);
#endif

    resourcewad = M_StringJoin(resourcefolder, DIR_SEPARATOR_S, DOOMRETRO_RESOURCEWAD, NULL);
    free(resourcefolder);

    M_MakeDirectory(appdatafolder);
    configfile = (p ? M_StringDuplicate(myargv[p + 1]) : M_StringJoin(appdatafolder, DIR_SEPARATOR_S, DOOMRETRO_CONFIG, NULL));

    C_ClearConsole();

    C_PrintCompileDate();

#if defined(_WIN32)
    I_PrintWindowsVersion();
#endif

    // Load configuration files before initializing other subsystems.
    M_LoadCVARs(configfile);

    SDL_Init(SDL_INIT_EVERYTHING);

    I_PrintSystemInfo();
    C_PrintSDLVersions();

    D_BuildBEXTables();

    iwadfile = D_FindIWAD();

    for (int i = 0; i < MAXALIASES; i++)
    {
        aliases[i].name[0] = '\0';
        aliases[i].string[0] = '\0';
    }

    if (M_StringCompare(iwadfolder, iwadfolder_default) || !M_FolderExists(iwadfolder))
        D_InitIWADFolder();

    if ((respawnmonsters = M_CheckParm("-respawn")))
        C_Output("A " BOLD("-respawn") " parameter was found on the command-line. Monsters will now respawn.");
    else if ((respawnmonsters = M_CheckParm("-respawnmonsters")))
        C_Output("A " BOLD("-respawnmonsters") " parameter was found on the command-line. Monsters will now respawn.");

    if ((nomonsters = M_CheckParm("-nomonsters")))
    {
        C_Output("A " BOLD("-nomonsters") " parameter was found on the command-line. No monsters will now be spawned.");
        stat_cheated = SafeAdd(stat_cheated, 1);
        M_SaveCVARs();
    }

    if ((pistolstart = M_CheckParm("-pistolstart")))
        C_Output("A " BOLD("-pistolstart") " parameter was found on the command-line. "
            "The player will now start each map with 100%% health, no armor, and only a pistol with 50 bullets.");

    if ((fastparm = M_CheckParm("-fast")))
        C_Output("A " BOLD("-fast") " parameter was found on the command-line. Monsters will now be fast.");
    else if ((fastparm = M_CheckParm("-fastmonsters")))
        C_Output("A " BOLD("-fastmonsters") " parameter was found on the command-line. Monsters will now be fast.");

    if ((solonet = M_CheckParm("-solonet")))
        C_Output("A " BOLD("-solonet") " parameter was found on the command-line. "
            "Things usually intended for multiplayer will now spawn at the start of each map, and the player will respawn if they die.");
    else if ((solonet = M_CheckParm("-solo-net")))
        C_Output("A " BOLD("-solo-net") " parameter was found on the command-line. "
            "Things usually intended for multiplayer will now spawn at the start of each map, and the player will respawn if they die.");

    if ((devparm = M_CheckParm("-devparm")))
        C_Output("A " BOLD("-devparm") " parameter was found on the command-line. %s", s_D_DEVSTR);

    // turbo option
    if ((p = M_CheckParm("-turbo")))
    {
        int scale = turbo_default * 2;

        if (p < myargc - 1)
        {
            scale = strtol(myargv[p + 1], NULL, 10);

            if (scale < turbo_min || scale > turbo_max)
                scale = turbo_default * 2;
        }

        C_Output("A " BOLD("-turbo") " parameter was found on the command-line. "
            "The player will now be %i%% their normal speed.", scale);

        if (scale != turbo_default)
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

    if (!stat_runs)
    {
        C_Output("This is the first time " ITALICS(DOOMRETRO_NAME) " has been run on this " COMPUTER ".");

        stat_firstrun = gamestarttime.tm_mday + (gamestarttime.tm_mon + 1) * 100 + (1900 + gamestarttime.tm_year) * 10000;
    }
    else
    {
        char    *temp = commify(SafeAdd(stat_runs, 1));

        if (stat_firstrun)
        {
            const int   day = stat_firstrun % 100;
            const int   month = (stat_firstrun % 10000) / 100;
            const int   year = (int)stat_firstrun / 10000;

            const char *months[] =
            {
                "January", "February", "March", "April", "May", "June",
                "July", "August", "September", "October", "November", "December"
            };

            C_Output(ITALICS(DOOMRETRO_NAME) " has been run %s times on this " COMPUTER " since %s, %s %i, %i.",
                temp, dayofweek(day, month, year), months[month - 1], day, year);
        }
        else
            C_Output(ITALICS(DOOMRETRO_NAME) " has been run %s times on this " COMPUTER ".", temp);

        free(temp);
    }

    if (!M_FileExists(resourcewad))
        I_Error("%s can't be found.", resourcewad);

    if (M_CheckParm("-nodeh"))
        C_Output("A " BOLD("-nodeh") " parameter was found on the command-line. "
            "All " BOLD("DEHACKED") " lumps will now be ignored.");
    else if (M_CheckParm("-nobex"))
        C_Output("A " BOLD("-nobex") " parameter was found on the command-line. "
            "All " BOLD("DEHACKED") " lumps will now be ignored.");

    p = M_CheckParmsWithArgs("-file", "-pwad", "-merge", 1, 1);

    if (!(choseniwad = D_CheckParms()))
    {
        if (iwadfile)
        {
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
                else if (!choseniwad && !error
                    && (!*wad || M_StringEndsWith(wad, ".wad") || M_StringEndsWith(wad, ".iwad") || M_StringEndsWith(wad, ".pwad")))
#else
                else if (!choseniwad && !error)
#endif
                {
                    char    buffer[256];

#if defined(_WIN32)
                    M_snprintf(buffer, sizeof(buffer), DOOMRETRO_NAME " couldn't find %s.\n", (*wad ? wad : "any IWADs"));

                    if (previouswad)
                        wad = M_StringDuplicate(previouswad);
#else
                    M_snprintf(buffer, sizeof(buffer), DOOMRETRO_NAME " couldn't find any IWADs.\n");
#endif

                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, DOOMRETRO_NAME, buffer, NULL);
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
            for (p++; p < myargc && myargv[p][0] != '-'; p++)
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
                    gamemission_t   iwadrequired = IWADRequiredByPWAD(file);
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
        I_Error(DOOMRETRO_NAME " couldn't find any IWADs.");

    if (!M_CheckParm("-noautoload") && gamemode != shareware)
    {
        D_SetAutoLoadFolder();

        autoloading = W_AutoLoadFiles(autoloadfolder);
        autoloading |= W_AutoLoadFiles(autoloadiwadsubfolder);

        if (autoloadpwadsubfolder)
            autoloading |= W_AutoLoadFiles(autoloadpwadsubfolder);
    }

    W_Init();

    FREEDM = (W_CheckNumForName("FREEDM") >= 0);

    PLAYPALs = (FREEDOOM || chex || hacx || REKKRSA ? 2 : W_CheckMultipleLumps("PLAYPAL"));
    STBARs = W_CheckMultipleLumps("STBAR");

    DSFLAMST = (W_CheckMultipleLumps("DSFLAMST") > 1);
    M_DOOM = (W_CheckMultipleLumps("M_DOOM") > 2);
    M_EPISOD = (W_CheckMultipleLumps("M_EPISOD") > 1);
    M_GDHIGH = (W_CheckMultipleLumps("M_GDHIGH") > 1);
    M_GDLOW = (W_CheckMultipleLumps("M_GDLOW") > 1);
    M_LOADG = (W_CheckMultipleLumps("M_LOADG") > 1);
    M_LGTTL = (W_CheckMultipleLumps("M_LGTTL") > 1);
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
    M_SGTTL = (W_CheckMultipleLumps("M_SGTTL") > 1);
    M_SKILL = (W_CheckMultipleLumps("M_SKILL") > 1);
    M_SKULL1 = (W_CheckMultipleLumps("M_SKULL1") > 1);
    M_SVOL = (W_CheckMultipleLumps("M_SVOL") > 1);
    STYSNUM0 = (W_CheckMultipleLumps("STYSNUM0") > 1);
    WISCRT2 = (W_CheckMultipleLumps("WISCRT2") > 1);

    I_InitGraphics();
    I_InitGameController();

    D_IdentifyVersion();

    if (!autoloading)
    {
        if (autoloadpwadsubfolder)
            C_Output("All files put in " BOLD("%s") ", " BOLD("%s") " and " BOLD("%s") " will be loaded automatically.",
                autoloadfolder, autoloadiwadsubfolder, autoloadpwadsubfolder);
        else if (!M_CheckParm("-noautoload") && gamemode != shareware)
            C_Output("All files put in " BOLD("%s") " and " BOLD("%s") " will be loaded automatically.",
                autoloadfolder, autoloadiwadsubfolder);
    }

    D_ProcessDehOnCmdLine();
    D_ProcessDehInWad();
    D_PostProcessDeh();
    D_TranslateDehStrings();

    if (dehcount > 2)
    {
        if (gamemode == shareware)
            I_Error("Other files can’t be loaded with the shareware version of DOOM.");

        C_Warning(0, "Loading multiple " BOLD("DEHACKED") " lumps or files may cause unexpected results.");
    }

    if (!M_StringCompare(s_VERSION, DOOMRETRO_NAMEANDVERSIONSTRING))
        I_Error("The wrong version of %s was found.", resourcewad);

    unity = (W_CheckNumForName("TITLEPIC") >= 0
        && SHORT(((patch_t *)W_CacheLastLumpName("TITLEPIC"))->width) > VANILLAWIDTH);

    D_SetGameDescription();

    if (gamemode == shareware)
        C_Warning(0, "This is the shareware version of " ITALICS("DOOM") ". "
            "You can buy the full version on " ITALICS("Steam") ", etc.");

    if (nerve && expansion == 2)
        gamemission = pack_nerve;

    FREEDOOM1 = (FREEDOOM && gamemission == doom);

    D_SetSaveGameFolder(true);

    D_SetScreenshotsFolder();

    C_Output("All files created using the " BOLD("condump") " CCMD will be placed in "
        BOLD("%s" DIR_SEPARATOR_S DOOMRETRO_CONSOLEFOLDER DIR_SEPARATOR_S) ".", appdatafolder);

    free(appdatafolder);

    // Check for -file in shareware
    if (modifiedgame)
    {
        if (gamemode == shareware)
            I_Error("Other files can’t be loaded with the shareware version of DOOM.");

        // Check for fake IWAD with right name,
        // but w/o all the lumps of the registered version.
        if (gamemode == registered)
        {
            // These are the lumps that will be checked in IWAD,
            // if any one is not present, execution will be aborted.
            const char name[23][9] =
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
        const int   temp = myargv[p + 1][0] - '1';

        if (temp >= sk_baby && temp <= sk_nightmare)
        {
            char    *string = titlecase(*skilllevels[temp]);

            startskill = (skill_t)temp;
            skilllevel = startskill + 1;
            M_SaveCVARs();

            M_StringReplaceAll(string, ".", "", false);
            M_StringReplaceAll(string, "!", "", false);

            C_Output("A " BOLD("-%s") " parameter was found on the command-line. The skill level is now " ITALICS("%s."),
                myargv[p], string);
            free(string);
        }
    }

    if ((p = M_CheckParmWithArgs("-episode", 1, 1)) && gamemode != commercial)
    {
        const int   temp = myargv[p + 1][0] - '0';

        if ((gamemode == shareware && temp == 1) || (temp >= 1 && ((gamemode == registered && temp <= 3)
            || (gamemode == retail && temp <= 4) || (sigil && temp <= 5))))
        {
            startepisode = temp;
            episode = temp;
            M_SaveCVARs();
            M_snprintf(lumpname, sizeof(lumpname), "E%iM%i", startepisode, startmap);
            autostart = true;
            C_Output("An " BOLD("-episode") " parameter was found on the command-line. The episode is now " ITALICS("%s."),
                *episodes[episode - 1]);
        }
    }

    if ((p = M_CheckParmWithArgs("-expansion", 1, 1)) && gamemode == commercial)
    {
        const int   temp = myargv[p + 1][0] - '0';

        if (temp <= (nerve ? 2 : 1))
        {
            gamemission = (temp == 1 ? doom2 : pack_nerve);
            expansion = temp;
            M_SaveCVARs();
            M_snprintf(lumpname, sizeof(lumpname), "MAP%02i", startmap);
            autostart = true;
            C_Output("An " BOLD("-expansion") " parameter was found on the command-line. The expansion is now " ITALICS("%s."),
                *expansions[expansion - 1]);
        }
    }

    if ((p = M_CheckParmWithArgs("-warp", 1, 1)))
        C_Output("A " BOLD("-warp") " parameter was found on the command-line.");
    else if ((p = M_CheckParmWithArgs("+map", 1, 1)))
        C_Output("A " BOLD("+map") " parameter was found on the command-line.");

    if (p)
    {
        if (gamemode == commercial)
        {
            if (strlen(myargv[p + 1]) == 5 && toupper(myargv[p + 1][0]) == 'M' && toupper(myargv[p + 1][1]) == 'A'
                && toupper(myargv[p + 1][2]) == 'P' && isdigit((int)myargv[p + 1][3]) && isdigit((int)myargv[p + 1][4]))
                startmap = (myargv[p + 1][3] - '0') * 10 + myargv[p + 1][4] - '0';
            else
                startmap = strtol(myargv[p + 1], NULL, 10);

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

            if (startmap > 1)
            {
                stat_cheated = SafeAdd(stat_cheated, 1);
                M_SaveCVARs();
            }
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
    V_InitColorTranslation();

    if ((startloadgame = ((p = M_CheckParmWithArgs("-loadgame", 1, 1)) ? strtol(myargv[p + 1], NULL, 10) : -1)) >= 0
        && startloadgame <= 5)
    {
        menuactive = false;
        splashscreen = false;
        I_InitKeyboard();

        if (alwaysrun)
            C_StrCVAROutput(stringize(alwaysrun), "on");

        G_LoadGame(P_SaveGameFile(startloadgame));
    }

    splashpal = W_CacheLastLumpName("SPLSHPAL");

    for (int i = 0; i < 18; i++)
    {
        char    buffer[9];

        M_snprintf(buffer, sizeof(buffer), "DRLOGO%02i", i + 1);
        logolump[i] = W_CacheLastLumpName(buffer);
    }

    logox = (SCREENWIDTH - SHORT(logolump[0]->width)) / 2;
    logoy = (SCREENHEIGHT - SHORT(logolump[0]->height)) / 2;

    fineprintlump = W_CacheLastLumpName("DRFNPRNT");
    fineprintwidth = SHORT(fineprintlump->width);
    fineprintheight = SHORT(fineprintlump->height);
    fineprintx = (SCREENWIDTH - fineprintwidth) / 2;
    fineprinty = SCREENHEIGHT - fineprintheight - 2;

    if (autosigil)
    {
        titlelump = W_CacheLastLumpName("TITLEPI1");
        creditlump = W_CacheLastLumpName("CREDIT2");
    }
    else if (REKKRSL)
    {
        titlelump = W_CacheLastLumpName("TITLEPIW");
        creditlump = W_CacheLastLumpName("CREDITW");
    }
    else
    {
        const int   titlepics = W_CheckMultipleLumps("TITLEPIC");
        const int   credits = W_CheckMultipleLumps("CREDIT");

        if ((titlepics == 1 && lumpinfo[W_GetNumForName("TITLEPIC")]->wadfile->type == PWAD) || titlepics > 1)
            titlelump = W_CacheLumpName("TITLEPIC");
        else
            switch (gamemission)
            {
                case doom:
                    titlelump = W_CacheLumpName("TITLEPI1");
                    break;

                case doom2:
                case pack_nerve:
                    titlelump = W_CacheLumpName("TITLEPI2");
                    break;

                case pack_plut:
                    titlelump = W_CacheLumpName("TITLEPIP");
                    break;

                case pack_tnt:
                    titlelump = W_CacheLumpName("TITLEPIT");
                    break;

                case none:
                    break;
            }

        if ((credits == 1 && lumpinfo[W_GetNumForName("CREDIT")]->wadfile->type == PWAD) || credits > 1)
            creditlump = W_CacheLumpName("CREDIT");
        else
            creditlump = W_CacheLumpName(gamemission == doom ? (gamemode == shareware ? "CREDIT1" : "CREDIT2") : "CREDIT3");
    }

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
        else if (M_CheckParm("-nosplash"))
        {
            menuactive = false;
            splashscreen = false;
            D_FadeScreen(false);
            D_StartTitle(1);
        }
        else
#if SCREENSCALE == 1
            D_StartTitle(1);
#else
            D_StartTitle(0);
#endif
    }

    // Ty 04/08/98 - Add 5 lines of misc. data, only if non-blank
    // The expectation is that these will be set in a .bex file
    if (*startup1 && !FREEDOOM)
    {
        C_AddConsoleDivider();

        if (*startup1)
        {
            D_ParseStartupString(startup1);

            if (*startup2)
            {
                D_ParseStartupString(startup2);

                if (*startup3)
                {
                    D_ParseStartupString(startup3);

                    if (*startup4)
                    {
                        D_ParseStartupString(startup4);

                        if (*startup5)
                            D_ParseStartupString(startup5);
                    }
                }
            }
        }
    }

    I_Sleep(500);
}

//
// D_DoomMain
//
void D_DoomMain(void)
{
    D_DoomMainSetup();  // CPhipps - setup out of main execution stack
    D_DoomLoop();       // never returns
}
