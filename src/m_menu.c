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

#include <ctype.h>

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_iwad.h"
#include "doomstat.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_controller.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_font.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_setup.h"
#include "s_sound.h"
#include "st_lib.h"
#include "st_stuff.h"
#include "v_data.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"

#define SPACEWIDTH       7
#define LINEHEIGHT      17
#define OFFSET          17
#define SKULLANIMCOUNT  10

// -1 = no quicksave slot picked!
int             quicksaveslot;

// true = message to be printed
bool            messagetoprint = false;

// ...and here is the message string!
static char     *messagestring;

static int      messagelastmenuactive;

// timed message = no input from user
static bool     messageneedsinput;

static void (*messageroutine)(int);

// we are going to be entering a savegame string
static bool     savestringenter;
static int      savecharindex;                      // which char we're editing

// old save description before edit
static char     saveoldstring[SAVESTRINGSIZE];

bool            helpscreen = false;
bool            menuactive;
bool            savegames;
bool            quitting;

bool            reopenautomap;

char            savegamestrings[savegame_max][SAVESTRINGSIZE];

static short    itemon;                             // menu item skull is on
static short    skullanimcounter = SKULLANIMCOUNT;  // skull animation counter
static short    whichskull;                         // which skull to draw

static bool     showcaret;
static short    caretwait = SKULLANIMCOUNT;
int             caretcolor;

static int      blurtic = -1;
static int      functionkey;

// current menudef
static menu_t   *currentmenu;

int             menuspindirection;
int             menuspinspeed;
static angle_t  playerangle;
static int      playerlookdir;
static fixed_t  playerviewz;

static patch_t  *menuborder;
static short    titleheight;

int             maptoepisode[100] = { 0 };

//
// PROTOTYPES
//
static void M_NewGame(int choice);
static void M_Episode(int choice);
static void M_Expansion(int choice);
static void M_ChooseSkill(int choice);
static void M_LoadGame(int choice);
static void M_SaveGame(int choice);
static void M_Options(int choice);
void M_EndGame(int choice);
static void M_Console(int choice);

static void M_ChangeMessages(int choice);
static void M_ChangeSensitivity(int choice);
static void M_SfxVol(int choice);
static void M_MusicVol(int choice);
static void M_ChangeDetail(int choice);
static void M_SizeDisplay(int choice);
static void M_Sound(int choice);

static void M_LoadSelect(int choice);
static void M_SaveSelect(int choice);
static void M_ReadSaveStrings(void);
static void M_QuickSave(void);
static void M_QuickLoad(void);

static void M_DrawMainMenu(void);
static void M_DrawHelp(void);
static void M_DrawNewGame(void);
static void M_DrawEpisode(void);
static void M_DrawExpansion(void);
static void M_DrawOptions(void);
static void M_DrawSound(void);
static void M_DrawLoad(void);
static void M_DrawSave(void);

static void M_DrawSaveLoadBorder(int x, int y, bool highlight);
static void M_SetupNextMenu(menu_t *menudef);
static void M_DrawSlider(int x, int y, int width, int shadowwidth,
    float dot, float factor, int offset, bool highlight);
static void M_WriteText(int x, int y, const char *string,
    bool highlight, bool shadow, unsigned char prevletter);

//
// DOOM MENU
//

enum
{
    new_game,
    options,
    load_game,
    save_game,
    quit_doom,
    main_end
};

static menuitem_t MainMenu[] =
{
    { 1, "M_NGAME",  &M_NewGame,  &s_M_NEWGAME  },
    { 1, "M_OPTION", &M_Options,  &s_M_OPTIONS  },
    { 1, "M_LOADG",  &M_LoadGame, &s_M_LOADGAME },
    { 1, "M_SAVEG",  &M_SaveGame, &s_M_SAVEGAME },
    { 1, "M_QUITG",  &M_QuitDOOM, &s_M_QUITGAME }
};

menu_t MainDef =
{
    5,
    NULL,
    MainMenu,
    &M_DrawMainMenu,
    98, 77,
    new_game
};

//
// EPISODE SELECT
//

enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep5,
    ep6,
    ep7,
    ep8,
    ep9,
    ep10,
    ep_end
};

static menuitem_t EpisodeMenu[] =
{
    { 1, "M_EPI1",  &M_Episode, &s_M_EPISODE1  },
    { 1, "M_EPI2",  &M_Episode, &s_M_EPISODE2  },
    { 1, "M_EPI3",  &M_Episode, &s_M_EPISODE3  },
    { 1, "M_EPI4",  &M_Episode, &s_M_EPISODE4  },
    { 1, "M_EPI5",  &M_Episode, &s_M_EPISODE5  },

    // Some extra empty episodes for extensibility through UMAPINFO
    { 1, "M_EPI6",  &M_Episode, &s_M_EPISODE6  },
    { 1, "M_EPI7",  &M_Episode, &s_M_EPISODE7  },
    { 1, "M_EPI8",  &M_Episode, &s_M_EPISODE8  },
    { 1, "M_EPI9",  &M_Episode, &s_M_EPISODE9  },
    { 1, "M_EPI10", &M_Episode, &s_M_EPISODE10 }
};

menu_t EpiDef =
{
    ep_end,
    &MainDef,
    EpisodeMenu,
    &M_DrawEpisode,
    41, 69,
    ep1
};

//
// EXPANSION SELECT
//

enum
{
    ex1,
    ex2,
    ex_end
};

static menuitem_t ExpansionMenu[] =
{
    { 1, "M_EPI1", &M_Expansion, &s_M_EXPANSION1 },
    { 1, "M_EPI2", &M_Expansion, &s_M_EXPANSION2 }
};

menu_t ExpDef =
{
    ex_end,
    &MainDef,
    ExpansionMenu,
    &M_DrawExpansion,
    41, 69,
    ex1
};

//
// NEW GAME
//

enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
};

static menuitem_t NewGameMenu[] =
{
    { 1, "M_JKILL", &M_ChooseSkill, &s_M_SKILLLEVEL1 },
    { 1, "M_ROUGH", &M_ChooseSkill, &s_M_SKILLLEVEL2 },
    { 1, "M_HURT",  &M_ChooseSkill, &s_M_SKILLLEVEL3 },
    { 1, "M_ULTRA", &M_ChooseSkill, &s_M_SKILLLEVEL4 },
    { 1, "M_NMARE", &M_ChooseSkill, &s_M_SKILLLEVEL5 }
};

menu_t NewDef =
{
    newg_end,
    &EpiDef,
    NewGameMenu,
    &M_DrawNewGame,
    45, 69,
    hurtme
};

//
// OPTIONS MENU
//

enum
{
    endgame,
    msgs,
    detail,
    scrnsize,
    option_empty1,
    mousesens,
    option_empty2,
    soundvol,
    consol,
    opt_end
};

static menuitem_t OptionsMenu[] =
{
    {  1, "M_ENDGAM", &M_EndGame,           &s_M_ENDGAME          },
    {  1, "M_MESSG",  &M_ChangeMessages,    &s_M_MESSAGES         },
    {  1, "M_DETAIL", &M_ChangeDetail,      &s_M_GRAPHICDETAIL    },
    {  2, "M_SCRNSZ", &M_SizeDisplay,       &s_M_SCREENSIZE       },
    { -1, "",         NULL,                 NULL                  },
    {  2, "M_MSENS",  &M_ChangeSensitivity, &s_M_MOUSESENSITIVITY },
    { -1, "",         NULL,                 NULL                  },
    {  1, "M_SVOL",   &M_Sound,             &s_M_SOUNDVOLUME      },
    {  1, "M_CONSOL", &M_Console,           &s_M_CONSOLE          }
};

static menu_t OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    &M_DrawOptions,
    52, 25,
    endgame
};

enum
{
    help_empty,
    help_end
};

static menu_t HelpDef =
{
    help_end,
    &HelpDef,
    NULL,
    &M_DrawHelp,
    330, 175,
    help_empty
};

//
// SOUND VOLUME MENU
//

enum
{
    sfx_vol,
    sound_empty1,
    music_vol,
    sound_empty2,
    sound_end
};

static menuitem_t SoundMenu[] =
{
    {  2, "M_SFXVOL", &M_SfxVol,   &s_M_SFXVOLUME   },
    { -1, "",         NULL,        NULL             },
    {  2, "M_MUSVOL", &M_MusicVol, &s_M_MUSICVOLUME },
    { -1, "",         NULL,        NULL             }
};

static menu_t SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    &M_DrawSound,
    89, 64,
    sfx_vol
};

//
// LOAD GAME MENU
//

enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,
    load8,
    load_end
};

static menuitem_t LoadGameMenu[] =
{
    { 1, "", &M_LoadSelect, NULL },
    { 1, "", &M_LoadSelect, NULL },
    { 1, "", &M_LoadSelect, NULL },
    { 1, "", &M_LoadSelect, NULL },
    { 1, "", &M_LoadSelect, NULL },
    { 1, "", &M_LoadSelect, NULL },
    { 1, "", &M_LoadSelect, NULL },
    { 1, "", &M_LoadSelect, NULL }
};

menu_t LoadDef =
{
    load_end,
    &MainDef,
    LoadGameMenu,
    &M_DrawLoad,
    67, 33,
    load1
};

//
// SAVE GAME MENU
//

static menuitem_t SaveGameMenu[] =
{
    { 1, "", &M_SaveSelect, NULL },
    { 1, "", &M_SaveSelect, NULL },
    { 1, "", &M_SaveSelect, NULL },
    { 1, "", &M_SaveSelect, NULL },
    { 1, "", &M_SaveSelect, NULL },
    { 1, "", &M_SaveSelect, NULL },
    { 1, "", &M_SaveSelect, NULL },
    { 1, "", &M_SaveSelect, NULL }
};

menu_t SaveDef =
{
    load_end,
    &MainDef,
    SaveGameMenu,
    &M_DrawSave,
    67, 33,
    load1
};

static void M_BlurMenuBackground(const byte *src, byte *dest)
{
    for (int i = 0; i < SCREENAREA; i++)
        dest[i] = grays[src[i]];

    for (int y = 0; y <= SCREENAREA - SCREENWIDTH; y += SCREENWIDTH)
        for (int x = y; x <= y + SCREENWIDTH - 2; x++)
            dest[x] = tinttab50[(dest[x + 1] << 8) + dest[x]];

    for (int y = 0; y <= SCREENAREA - SCREENWIDTH; y += SCREENWIDTH)
        for (int x = y + SCREENWIDTH - 2; x > y; x--)
            dest[x] = tinttab50[(dest[x - 1] << 8) + dest[x]];

    for (int y = SCREENWIDTH; y <= SCREENAREA - SCREENWIDTH * 2; y += SCREENWIDTH)
        for (int x = y + 6; x <= y + SCREENWIDTH - 6; x++)
            dest[x] = tinttab50[(dest[SCREENWIDTH * M_BigRandomInt(-1, 1)
                + x + M_BigRandomInt(-6, 6)] << 8) + dest[x]];

    for (int y = SCREENAREA - SCREENWIDTH; y >= SCREENWIDTH; y -= SCREENWIDTH)
        for (int x = y + SCREENWIDTH - 1; x >= y + 1; x--)
            dest[x] = tinttab50[(dest[x - SCREENWIDTH - 1] << 8) + dest[x]];

    for (int y = 0; y <= SCREENAREA - SCREENWIDTH * 2; y += SCREENWIDTH)
        for (int x = y; x <= y + SCREENWIDTH - 1; x++)
            dest[x] = tinttab50[(dest[x + SCREENWIDTH] << 8) + dest[x]];

    for (int y = SCREENAREA - SCREENWIDTH; y >= SCREENWIDTH; y -= SCREENWIDTH)
        for (int x = y; x <= y + SCREENWIDTH - 1; x++)
            dest[x] = tinttab50[(dest[x - SCREENWIDTH] << 8) + dest[x]];

    for (int y = 0; y <= SCREENAREA - SCREENWIDTH * 2; y += SCREENWIDTH)
        for (int x = y + SCREENWIDTH - 1; x >= y + 1; x--)
            dest[x] = tinttab50[(dest[x + SCREENWIDTH - 1] << 8) + dest[x]];

    for (int y = SCREENAREA - SCREENWIDTH; y >= SCREENWIDTH; y -= SCREENWIDTH)
        for (int x = y; x <= y + SCREENWIDTH - 2; x++)
            dest[x] = tinttab50[(dest[x - SCREENWIDTH + 1] << 8) + dest[x]];
}

static void M_DrawMenuBorder(void)
{
    for (int x = 0; x < SCREENWIDTH; x++)
    {
        byte    *dot = *screens + x;

        *dot = tinttab40[*dot];
        dot += SCREENAREA - SCREENWIDTH;
        *dot = tinttab40[*dot];
    }

    if (vid_widescreen || nowidescreen)
    {
        for (int y = 0; y < SCREENAREA; y += SCREENWIDTH)
            for (int x = 0; x < 10; x++)
                screens[0][y + x] = screens[0][y + SCREENWIDTH - x - 1] = nearestblack;

        V_DrawMenuBorderPatch(10, 0, menuborder);
    }
    else
        V_DrawMenuBorderPatch(0, 0, menuborder);
}

//
// M_DrawMenuBackground
//
void M_DrawMenuBackground(void)
{
    static byte blurscreen[MAXSCREENAREA];

    if (automapactive && !messagetoprint)
    {
        automapactive = false;
        reopenautomap = true;
        viewactive = true;
        return;
    }

    if (gametime != blurtic)
    {
        for (int y = 2 * SCREENWIDTH; y < SCREENAREA; y += 4 * SCREENWIDTH)
        {
            const byte  *white = ((M_BigRandom() % 25) ? white25 : white33);

            for (int x = 0; x < SCREENWIDTH; x++)
            {
                byte    *dot = *screens + x + y;

                *dot = white[*dot];
            }
        }

        M_BlurMenuBackground(screens[0], blurscreen);

        for (int i = 0; i < SCREENAREA; i++)
        {
            byte    *dot = blurscreen + i;

            *dot = black40[*dot];
        }

        blurtic = gametime;
    }

    memcpy(screens[0], blurscreen, SCREENAREA);

    if (!consoleactive && (vid_fullscreen || !vid_widescreen))
        M_DrawMenuBorder();

    if (mapwindow)
        memset(mapscreen, nearestblack, MAPAREA);

    if (vid_showfps && framespersecond)
        C_UpdateFPSOverlay();
}

static byte blues[] =
{
    245, 245, 245, 242, 197, 245, 245, 245, 245, 244, 245, 245, 245, 243, 244, 244,
    200, 201, 201, 202, 203, 203, 204, 204, 205, 206, 206, 206, 207, 207, 207, 207,
    241, 242, 242, 243, 243, 244, 244, 244, 245, 245, 245, 245, 245, 245, 245, 245,
    197, 198, 198, 199, 199, 199, 200, 200, 200, 201, 202, 202, 203, 203, 204, 204,
    205, 206, 206, 206, 207, 207, 207, 207, 241, 242, 242, 243, 243, 244, 245, 245,
    197, 198, 198, 199, 199, 200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205,
    205, 206, 206, 207, 207, 207, 207, 241, 241, 242, 243, 243, 244, 244, 245, 245,
    200, 201, 202, 203, 204, 205, 206, 207, 207, 241, 242, 243, 244, 245, 245, 245,
    202, 203, 204, 204, 205, 205, 206, 206, 207, 207, 207, 207, 241, 241, 242, 243,
    205, 206, 207, 207, 241, 242, 243, 244, 206, 207, 207, 207, 241, 242, 243, 243,
    197, 199, 202, 204, 206, 207, 241, 243, 197, 198, 200, 201, 203, 205, 206, 207,
    242, 242, 243, 243, 243, 244, 244, 244, 244, 245, 245, 245, 245, 245, 245, 245,
    198, 200, 202, 203, 205, 207, 242, 244, 245, 245, 245, 245, 245, 245, 245, 245,
    197, 197, 198, 199, 201, 201, 203, 204, 205, 205, 206, 206, 207, 207, 207, 207,
    197, 197, 197, 197, 197, 198, 198, 198, 241, 241, 242, 243, 243, 244, 245, 245,
    245, 245, 245, 245, 245, 245, 245, 245, 202, 199, 202, 207, 241, 243, 244, 245
};

//
// M_DrawHelpBackground
//
static void M_DrawHelpBackground(void)
{
    if (automapactive)
    {
        automapactive = false;
        reopenautomap = true;
        viewactive = true;
        return;
    }

    if (gamestate != GS_LEVEL)
        V_DrawPagePatch(0, titlelump);

    M_BigSeed(411);

    for (int y = 0; y < SCREENAREA; y += 2 * SCREENWIDTH)
        for (int x = 0; x < SCREENWIDTH; x += 2)
        {
            byte        *dot1 = *screens + y + x;
            byte        *dot2 = dot1 + 1;
            byte        *dot3 = dot2 + SCREENWIDTH;
            byte        *dot4 = dot3 - 1;
            const byte  color = colormaps[0][M_BigRandomInt(0, 3) * 256
                            + blues[tinttab50[(tinttab50[(*dot1 << 8) + *dot2] << 8)
                            + tinttab50[(*dot3 << 8) + *dot4]]]];

            *dot1 = color;
            *dot2 = color;
            *dot3 = color;
            *dot4 = color;
        }

    M_BigSeed((unsigned int)time(NULL));

    if (mapwindow)
        memset(mapscreen, nearestblack, MAPAREA);

    if (vid_showfps && framespersecond)
        C_UpdateFPSOverlay();
}

static const int chartoi[] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1,
    -1, -1, -1,  1, -1, -1, -1, -1,  2,  3,  4, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1,  5, -1, -1, -1, -1,  6, -1,  7,  8,  9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, -1, -1, -1, -1, -1, -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58
};

static struct
{
    char    char1;
    char    char2;
    int     adjust;
} bigkern[] = {
    { ' ',  't',  -1 }, { ' ',  'V',  -2 }, { ' ',  'y',  -2 }, { '-',  'V',  -2 },
    { 'f',  'e',  -1 }, { 'f',  'f',  -1 }, { 'f',  'o',  -1 }, { 'l',  ' ',  -1 },
    { 'l',  'e',  -1 }, { 'l',  'o',  -1 }, { 'L',  'S',  -1 }, { 'l',  't',  -1 },
    { 'o',  'a',  -1 }, { 'O',  'A',  -1 }, { 'o',  't',  -1 }, { 'p',  'a',  -2 },
    { 'P',  'a',  -3 }, { 't',  'a',  -1 }, { 't',  'o',  -1 }, { 'v',  'e',  -1 },
    { 'V',  'o',  -2 }, { 'y',  ' ',  -1 }, { 'y',  ',',  -3 }, { 'y',  '.',  -2 },
    { 'y',  'o',  -1 }, { '\0', '\0',  0 }
};

static struct
{
    char    char1;
    char    char2;
} overlap[] = {
    { 'a',  'd'  }, { 'A',  'D'  }, { 'a',  'i'  }, { 'a',  'm'  }, { 'A',  'M'  },
    { 'a',  'n'  }, { 'a',  'p'  }, { 'a',  'r'  }, { 'c',  'h'  }, { 'c',  'r'  },
    { 'e',  'a'  }, { 'E',  'a'  }, { 'e',  'd'  }, { 'e',  'n'  }, { 'E',  'n'  },
    { 'e',  'p'  }, { 'E',  'p'  }, { 'e',  'r'  }, { 'e',  's'  }, { 'e',  't'  },
    { 'E',  'x'  }, { 'G',  'a'  }, { 'G',  'A'  }, { 'g',  'h'  }, { 'g',  'i'  },
    { 'G',  'r'  }, { 'h',  'i'  }, { 'i',  'n'  }, { 'I',  'n'  }, { 'i',  's'  },
    { 'i',  't'  }, { 'i',  'z'  }, { 'k',  'i'  }, { 'K',  'n'  }, { 'L',  'i'  },
    { 'n',  't'  }, { 'o',  't'  }, { 'p',  'i'  }, { 'P',  'T'  }, { 'p',  't'  },
    { 'r',  'a'  }, { 'r',  'n'  }, { 'r',  't'  }, { 's',  't'  }, { 'x',  'p'  },
    { '\0', '\0' }
};

//
// M_DrawString
//  draw a string on screen
//
void M_DrawString(int x, int y, char *string, bool highlight, bool shadow)
{
    if (!M_DrawFON2String(x, y, string, highlight))
    {
        char        prev = '\0';
        const int   len = (int)strlen(string);

        for (int i = 0, j = -1; i < len; i++)
        {
            bool    overlapping = false;

            if (string[i] < 123)
                j = chartoi[(int)string[i]];

            for (int k = 0; bigkern[k].char1; k++)
                if (prev == bigkern[k].char1 && string[i] == bigkern[k].char2)
                {
                    x += bigkern[k].adjust;
                    break;
                }

            for (int k = 0; overlap[k].char1; k++)
                if (prev == overlap[k].char1 && string[i] == overlap[k].char2)
                {
                    overlapping = true;
                    break;
                }

            if (j == -1)
                x += SPACEWIDTH;
            else
            {
                const int   width = (int)strlen(redcharset[j]) / 18;

                for (int y1 = 0; y1 < 18; y1++)
                    for (int x1 = 0; x1 < width; x1++)
                    {
                        const unsigned char dot = redcharset[j][y1 * width + x1];

                        if (dot == (unsigned char)'\xC8')
                        {
                            if (!overlapping)
                                V_DrawPixel(x + x1, y + y1, PINK, highlight, shadow);
                        }
                        else
                            V_DrawPixel(x + x1, y + y1, (int)dot, highlight, shadow);
                    }

                x += width - 2;
            }

            prev = string[i];
        }
    }
}

//
// M_BigStringWidth
//  return width of string in pixels
//
static int M_BigStringWidth(char *string)
{
    int width = M_GetFON2PixelWidth(string);

    if (!width)
    {
        char        prev = '\0';
        const int   len = (int)strlen(string);

        for (int i = 0; i < len; i++)
        {
            const int   j = chartoi[(int)string[i]];

            for (int k = 0; bigkern[k].char1; k++)
                if (prev == bigkern[k].char1 && string[i] == bigkern[k].char2)
                    width += bigkern[k].adjust;

            width += (j == -1 ? SPACEWIDTH : (int)strlen(redcharset[j]) / 18 - 2);
            prev = string[i];
        }
    }

    return width;
}

//
// M_DrawCenteredString
//  draw a string centered horizontally on screen
//
void M_DrawCenteredString(int y, char *string)
{
    M_DrawString((VANILLAWIDTH - M_BigStringWidth(string) - 1) / 2, y, string, false, true);
}

//
// M_SplitString
//  split string of words into two lines
//
static void M_SplitString(char *string)
{
    const int   len = (int)strlen(string);

    for (int i = len / 2 - 1; i < len; i++)
        if (string[i] == ' ')
        {
            string[i] = '\n';
            break;
        }
}

//
// M_DrawPatchWithShadow
//  draw patch with shadow on screen
//
static void M_DrawPatchWithShadow(int x, int y, patch_t *patch, bool highlight)
{
    if (!patch)
        return;

    if (SHORT(patch->height) < VANILLAHEIGHT)
        V_DrawMenuPatch(x, y, patch, highlight, SCREENWIDTH);
    else
        V_DrawPagePatch(0, patch);
}

//
// M_DrawCenteredPatchWithShadow
//  draw patch with shadow horizontally centered on screen
//
static void M_DrawCenteredPatchWithShadow(int y, patch_t *patch)
{
    if (!patch)
        return;

    if (SHORT(patch->height) < VANILLAHEIGHT)
        V_DrawMenuPatch((VANILLAWIDTH - SHORT(patch->width)) / 2 + SHORT(patch->leftoffset),
            y, patch, false, SCREENWIDTH);
    else
        V_DrawPagePatch(0, patch);
}

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
static void M_ReadSaveStrings(void)
{
    savegames = false;

    for (int i = 0; i < load_end; i++)
    {
        FILE    *handle = fopen(P_SaveGameFile(i), "rb");

        if (!handle)
        {
            M_StringCopy(savegamestrings[i], s_EMPTYSTRING, sizeof(savegamestrings[0]));
            LoadGameMenu[i].status = 0;
            continue;
        }

        if (fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle) && *savegamestrings[i])
        {
            savegames = true;
            LoadGameMenu[i].status = 1;
        }
        else
        {
            M_StringCopy(savegamestrings[i], s_EMPTYSTRING, sizeof(savegamestrings[0]));
            LoadGameMenu[i].status = 0;
        }

        fclose(handle);
    }
}

static byte saveg_read8(FILE *file)
{
    byte    result = 0;

    if (fread(&result, 1, 1, file) < 1)
        return 0;

    return result;
}

//
// M_CheckSaveGame
//
static bool M_CheckSaveGame(int *ep, int *map, int slot)
{
    FILE    *file = fopen(P_SaveGameFile(slot), "rb");
    int     mission;

    if (!file)
        return false;

    for (int i = 0; i < SAVESTRINGSIZE + VERSIONSIZE + 1; i++)
        saveg_read8(file);

    *ep = saveg_read8(file);
    *map = saveg_read8(file);
    mission = saveg_read8(file);
    fclose(file);

    // switch expansions if necessary
    if (mission == doom2)
    {
        if (gamemission == doom2)
            return true;

        if (gamemission == pack_nerve)
        {
            ExpDef.laston = ex1;
            expansion = 1;
            gamemission = doom2;
            M_SaveCVARs();
            return true;
        }
        else
            return false;
    }

    if (mission == pack_nerve)
    {
        if (gamemission == pack_nerve)
            return true;

        if (gamemission == doom2 && nerve)
        {
            ExpDef.laston = ex2;
            expansion = 2;
            gamemission = pack_nerve;
            M_SaveCVARs();
            return true;
        }
        else
            return false;
    }

    if (mission != gamemission)
        return false;

    if (*ep > 1 && gamemode == shareware)
        return false;

    if (*ep > 3 && gamemode == registered)
        return false;

    return true;
}

int M_CountSaveGames(void)
{
    int count = 0;

    for (int i = 0; i < load_end; i++)
    {
        int ep;
        int map;

        if (M_CheckSaveGame(&ep, &map, i))
            count++;
    }

    return count;
}

//
// Draw border for the savegame description
//
static void M_DrawSaveLoadBorder(int x, int y, bool highlight)
{
    if (M_LSCNTR)
    {
        x += 3;
        M_DrawPatchWithShadow(x, y + 11, W_CacheLumpName("M_LSLEFT"), highlight);
        x += 8;

        for (int i = 0; i < 24; i++)
        {
            M_DrawPatchWithShadow(x, y + 11, W_CacheLumpName("M_LSCNTR"), highlight);
            x += 8;
        }

        M_DrawPatchWithShadow(x, y + 11, W_CacheLumpName("M_LSRGHT"), highlight);
    }
    else
    {
        for (int yy = 0; yy < 16; yy++)
            for (int xx = 0; xx < 8; xx++)
                V_DrawPixel(x + xx, y + yy, lsleft[yy * 8 + xx], highlight, true);

        x += 8;

        for (int i = 0; i < 24; i++)
        {
            for (int yy = 0; yy < 16; yy++)
                for (int xx = 0; xx < 8; xx++)
                    V_DrawPixel(x + xx, y + yy, lscntr[yy * 8 + xx], highlight, true);

            x += 8;
        }

        for (int yy = 0; yy < 16; yy++)
            for (int xx = 0; xx < 9; xx++)
                V_DrawPixel(x + xx, y + yy, lsrght[yy * 9 + xx], highlight, true);
    }
}

//
// M_LoadGame
//
static void M_DrawLoad(void)
{
    M_DrawMenuBackground();

    if (M_LGTTL)
        M_DrawCenteredPatchWithShadow(2 + OFFSET, W_CacheLumpName("M_LGTTL"));
    else if (M_LOADG)
        M_DrawCenteredPatchWithShadow(2 + OFFSET, W_CacheLumpName("M_LOADG"));
    else
    {
        char    *temp = uppercase(s_M_LOADGAME);

        M_DrawCenteredString(2 + OFFSET, temp);
        free(temp);
    }

    for (int i = 0; i < load_end; i++)
    {
        int         len = (int)strlen(savegamestrings[i]);
        const int   y = LoadDef.y + LINEHEIGHT * i + OFFSET;

        M_DrawSaveLoadBorder(LoadDef.x - 11, y - 4, (itemon == i));

        currentmenu->menuitems[i].x = LoadDef.x - 11 + MAXWIDESCREENDELTA;
        currentmenu->menuitems[i].y = y - 4;
        currentmenu->menuitems[i].width = 209;
        currentmenu->menuitems[i].height = SHORT(((patch_t *)W_CacheLumpName("M_LSLEFT"))->height) + 1;

        while (M_StringWidth(savegamestrings[i]) > SAVESTRINGPIXELWIDTH)
        {
            if (len >= 2 && savegamestrings[i][len - 2] == ' ')
            {
                savegamestrings[i][len - 2] = '.';
                savegamestrings[i][len - 1] = '.';
                savegamestrings[i][len] = '.';
                savegamestrings[i][len + 1] = '\0';
            }
            else if (len >= 1)
            {
                savegamestrings[i][len - 1] = '.';
                savegamestrings[i][len] = '.';
                savegamestrings[i][len + 1] = '.';
                savegamestrings[i][len + 2] = '\0';
            }

            len--;
        }

        M_WriteText(LoadDef.x - 2 + (M_StringCompare(savegamestrings[i], s_EMPTYSTRING)
            && s_EMPTYSTRING[0] == '-' && s_EMPTYSTRING[1] == '\0') * 6, y,
            savegamestrings[i], (itemon == i), false, '\0');
    }
}

//
// User wants to load this game
//
static void M_LoadSelect(int choice)
{
    int ep;
    int map;

    if (M_CheckSaveGame(&ep, &map, choice))
    {
        char    name[SAVESTRINGSIZE];

        M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));
        S_StartSound(NULL, sfx_pistol);
        functionkey = 0;
        quicksaveslot = choice;
        savegameslot = choice;
        G_LoadGame(name);
    }
    else
    {
        C_ShowConsole(false);
        C_Warning(0, "This savegame requires a different WAD.");
    }

    M_CloseMenu();
}

//
// Selected from DOOM menu
//
static void M_LoadGame(int choice)
{
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}

static void M_SetCaretPos(int pointerx)
{
    const int   len = (int)strlen(savegamestrings[itemon]);
    int         x = LoadDef.x - 2 + MAXWIDESCREENDELTA;

    for (savecharindex = 0; savecharindex < len; savecharindex++)
    {
        const int   width = M_CharacterWidth(savegamestrings[itemon][savecharindex],
                        (!savecharindex ? '\0' : savegamestrings[itemon][savecharindex - 1]));

        if (pointerx < (x += width / 2))
            break;

        x += (width - width / 2);
    }
}

//
// M_SaveGame
//
static void M_DrawSave(void)
{
    M_DrawMenuBackground();

    // draw menu subtitle
    if (M_SGTTL)
        M_DrawCenteredPatchWithShadow(2 + OFFSET, W_CacheLumpName("M_SGTTL"));
    else if (M_SAVEG)
        M_DrawCenteredPatchWithShadow(2 + OFFSET, W_CacheLumpName("M_SAVEG"));
    else
    {
        char    *temp = uppercase(s_M_SAVEGAME);

        M_DrawCenteredString(2 + OFFSET, temp);
        free(temp);
    }

    // draw each save game slot
    for (int i = 0; i < load_end; i++)
    {
        int len = (int)strlen(savegamestrings[i]);
        int y = LoadDef.y + i * LINEHEIGHT + OFFSET;

        M_DrawSaveLoadBorder(LoadDef.x - 11, y - 4, (itemon == i));

        currentmenu->menuitems[i].x = LoadDef.x - 11 + MAXWIDESCREENDELTA;
        currentmenu->menuitems[i].y = y - 4;
        currentmenu->menuitems[i].width = 209;
        currentmenu->menuitems[i].height = SHORT(((patch_t *)W_CacheLumpName("M_LSLEFT"))->height) + 1;

        while (M_StringWidth(savegamestrings[i]) > SAVESTRINGPIXELWIDTH)
        {
            if (len >= 2 && savegamestrings[i][len - 2] == ' ')
            {
                savegamestrings[i][len - 2] = '.';
                savegamestrings[i][len - 1] = '.';
                savegamestrings[i][len] = '.';
                savegamestrings[i][len + 1] = '\0';
            }
            else if (len >= 1)
            {
                savegamestrings[i][len - 1] = '.';
                savegamestrings[i][len] = '.';
                savegamestrings[i][len + 1] = '.';
                savegamestrings[i][len + 2] = '\0';
            }

            len--;
        }

        // draw save game description
        if (savestringenter && i == itemon)
        {
            char    left[256] = "";
            char    right[256] = "";
            int     x = LoadDef.x - 2;

            savecharindex = MIN(savecharindex, (int)strlen(savegamestrings[i]));

            // draw text to left of text caret
            for (int j = 0; j < savecharindex; j++)
                left[j] = savegamestrings[i][j];

            left[savecharindex] = '\0';
            M_WriteText(x, y, left, true, false, '\0');
            x += M_StringWidth(left);

            // draw text caret
            if (showcaret || !windowfocused)
            {
                byte        *dot = *screens + ((y - 1) * SCREENWIDTH + x + WIDESCREENDELTA) * 2;
                const int   height = (SHORT(hu_font[0]->height) * 2 + 3) * SCREENWIDTH;

                for (int j = SCREENWIDTH; j < height; j += SCREENWIDTH)
                    *(dot + j) = *(dot + j + 1) = *(dot + j + 2) = *(dot + j + 3) = caretcolor;
            }

            // draw text to right of text caret
            for (int j = 0; j < len - savecharindex; j++)
                right[j] = savegamestrings[i][j + savecharindex];

            right[len - savecharindex] = '\0';
            M_WriteText(x + 2, y, right, true, false,
                (savecharindex >= 1 ? left[savecharindex - 1] : '\0'));
        }
        else
            M_WriteText(LoadDef.x - 2 + (M_StringCompare(savegamestrings[i], s_EMPTYSTRING)
                && s_EMPTYSTRING[0] == '-' && s_EMPTYSTRING[1] == '\0') * 6, y,
                savegamestrings[i], (itemon == i), false, '\0');
    }
}

//
// M_Responder calls this when user is finished
//
static void M_DoSave(int slot)
{
    M_CloseMenu();
    G_SaveGame(slot, savegamestrings[slot], "");
    functionkey = 0;
    quicksaveslot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
static char *RemoveMapNum(const char *string)
{
    char    *newstr = M_StringDuplicate(string);
    char    *pos = strchr(newstr, ':');

    if (pos)
    {
        newstr = pos + 1;

        while (newstr[0] == ' ')
            newstr++;
    }

    return newstr;
}

void M_UpdateSaveGameName(int i)
{
    bool        match = false;
    const int   len = (int)strlen(savegamestrings[i]);

    if (M_StringCompare(savegamestrings[i], s_EMPTYSTRING))
        match = true;
    else if (gamemission == doom && len == 4
        && savegamestrings[i][0] == 'E' && isdigit((int)savegamestrings[i][1])
        && savegamestrings[i][2] == 'M' && isdigit((int)savegamestrings[i][3])
        && W_CheckNumForName(savegamestrings[i]) >= 0)
        match = true;
    else if (gamemission != doom && len == 5
        && savegamestrings[i][0] == 'M' && savegamestrings[i][1] == 'A' && savegamestrings[i][2] == 'P'
        && isdigit((int)savegamestrings[i][3]) && isdigit((int)savegamestrings[i][4])
        && W_CheckNumForName(savegamestrings[i]) >= 0)
        match = true;

    if (!match && !M_StringCompare(mapnum, mapnumandtitle))
    {
        if (len >= 4 && savegamestrings[i][len - 1] == '.' && savegamestrings[i][len - 2] == '.'
            && savegamestrings[i][len - 3] == '.' && savegamestrings[i][len - 4] != '.')
            match = true;
        else
        {
            int ep;
            int map;

            if (M_CheckSaveGame(&ep, &map, i))
            {
                char    mapinfoname[128];

                M_StringCopy(mapinfoname, P_GetMapName(ep, map), sizeof(mapinfoname));

                switch (gamemission)
                {
                    case doom:
                        if ((map == 10 && M_StringCompare(savegamestrings[i], s_HUSTR_E1M4B))
                            || (map == 11 && M_StringCompare(savegamestrings[i], s_HUSTR_E1M8B))
                            || M_StringCompare(savegamestrings[i], RemoveMapNum(*mapinfoname ? mapinfoname : *mapnames[(ep - 1) * 9 + map - 1])))
                            match = true;

                        break;

                    case doom2:
                        if (M_StringCompare(savegamestrings[i],
                            RemoveMapNum(*mapinfoname ? mapinfoname : (bfgedition ? *mapnames2_bfg[map - 1] : *mapnames2[map - 1]))))
                            match = true;

                        break;

                    case pack_nerve:
                        if (M_StringCompare(savegamestrings[i], RemoveMapNum(*mapinfoname ? mapinfoname : *mapnamesn[map - 1])))
                            match = true;

                        break;

                    case pack_plut:
                        if (M_StringCompare(savegamestrings[i], RemoveMapNum(*mapinfoname ? mapinfoname : *mapnamesp[map - 1])))
                            match = true;

                        break;

                    case pack_tnt:
                        if (M_StringCompare(savegamestrings[i], RemoveMapNum(*mapinfoname ? mapinfoname : *mapnamest[map - 1])))
                            match = true;

                        break;

                    default:
                        break;
                }
            }
        }
    }

    if (match)
        M_StringCopy(savegamestrings[i], maptitle, sizeof(savegamestrings[0]));
}

static void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    SDL_StartTextInput();
    savestringenter = true;
    itemon = choice;
    M_StringCopy(saveoldstring, savegamestrings[itemon], sizeof(saveoldstring));
    M_UpdateSaveGameName(itemon);
    savecharindex = (int)strlen(savegamestrings[itemon]);
    showcaret = true;
    caretwait = SKULLANIMCOUNT;
}

//
// Selected from DOOM menu
//
static void M_SaveGame(int choice)
{
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}

//
// M_QuickSave
//
static void M_QuickSave(void)
{
    if (quicksaveslot < 0)
    {
        if (functionkey == KEY_F6)
        {
            functionkey = 0;
            M_CloseMenu();
            S_StartSound(NULL, sfx_swtchx);
        }
        else
        {
            functionkey = KEY_F6;
            M_OpenMainMenu();
            M_ReadSaveStrings();
            M_SetupNextMenu(&SaveDef);
            S_StartSound(NULL, sfx_swtchn);
        }

        return;
    }

    M_UpdateSaveGameName(quicksaveslot);
    M_DoSave(quicksaveslot);
}

//
// M_QuickLoad
//
static void M_QuickLoadResponse(int key)
{
    messagetoprint = false;

    if (key == 'y')
    {
        M_LoadSelect(quicksaveslot);
        S_StartSound(NULL, sfx_swtchx);
    }
    else
    {
        functionkey = 0;
        M_CloseMenu();
    }
}

static void M_QuickLoad(void)
{
    S_StartSound(NULL, sfx_swtchn);

    if (quicksaveslot < 0)
    {
        functionkey = 0;

        if (savegames)
            M_LoadGame(0);

        return;
    }

    if (M_StringEndsWith(s_QLPROMPT, s_PRESSYN))
        M_StartMessage(s_QLPROMPT, &M_QuickLoadResponse, true);
    else
    {
        static char line1[160];
        static char line2[160];
        static char loadstring[320];

        M_snprintf(line1, sizeof(line1), s_QLPROMPT, savegamestrings[quicksaveslot]);
        M_SplitString(line1);
        M_snprintf(line2, sizeof(line2), (usingcontroller ? s_PRESSA : s_PRESSYN), selectbutton);
        M_snprintf(loadstring, sizeof(loadstring), "%s\n\n%s", line1, line2);
        M_StartMessage(loadstring, &M_QuickLoadResponse, true);
    }
}

static void M_DeleteSaveGameResponse(int key)
{
    if (key == 'y')
    {
        static char buffer[1024];
        char        *temp;

        M_StringCopy(buffer, P_SaveGameFile(itemon), sizeof(buffer));
        temp = titlecase(savegamestrings[itemon]);

        if (remove(buffer))
        {
            C_Warning(0, "\"%s\" couldn't be deleted.", temp);
            M_CloseMenu();
            C_ShowConsole(false);
            D_FadeScreen(false);
            free(temp);
            return;
        }

        M_snprintf(buffer, sizeof(buffer), s_GGDELETED, temp);
        C_Output(buffer);
        HU_SetPlayerMessage(buffer, false, false);
        message_dontfuckwithme = true;
        M_ReadSaveStrings();
        free(temp);

        if (itemon == quicksaveslot)
            quicksaveslot = -1;

        if (savegames)
        {
            int previtem = itemon;
            int nextitem = itemon;

            while (M_StringCompare(savegamestrings[previtem], s_EMPTYSTRING))
                previtem = (previtem == load1 ? load8 : previtem - 1);

            while (M_StringCompare(savegamestrings[nextitem], s_EMPTYSTRING))
                nextitem = (nextitem == load8 ? load1 : nextitem + 1);

            itemon = (itemon - previtem < nextitem - itemon ? previtem : nextitem);
        }
        else
        {
            M_SetupNextMenu(&MainDef);
            MainDef.laston = itemon = new_game;
            SaveDef.laston = 0;
        }
    }
}

static void M_DeleteSaveGame(void)
{
    static char line1[160];
    static char line2[160];
    static char deletestring[320];

    M_snprintf(line1, sizeof(line1), s_DELPROMPT, savegamestrings[itemon]);
    M_SplitString(line1);
    M_snprintf(line2, sizeof(line2), (usingcontroller ? s_PRESSA : s_PRESSYN), selectbutton);
    M_snprintf(deletestring, sizeof(deletestring), "%s\n\n%s", line1, line2);
    M_StartMessage(deletestring, &M_DeleteSaveGameResponse, true);
}

//
// M_DrawHelp
//
static void M_DrawHelp(void)
{
    char    lumpname[6] = "HELP1";

    if (gamemode == shareware)
        M_StringCopy(lumpname, "HELP3", sizeof(lumpname));
    else if (gamemode == commercial)
        M_StringCopy(lumpname, (legacyofrust ? "HELP4" : "HELP"), sizeof(lumpname));

    if (REKKR && W_CheckNumForName("HELP1W") >= 0)
    {
        V_DrawPagePatch(0, W_CacheLastLumpName("HELP1W"));

        if (mapwindow)
            memset(mapscreen, nearestblack, MAPAREA);
    }
    else if (W_CheckNumForName(lumpname) >= 0)
    {
        if (chex || FREEDOOM || hacx || harmony || REKKRSA)
        {
            V_DrawPagePatch(0, W_CacheLastLumpName(gamemode == commercial ? "HELP" : "HELP1"));

            if (mapwindow)
                memset(mapscreen, nearestblack, MAPAREA);
        }
        else if (autosigil)
        {
            viewplayer->fixedcolormap = 0;
            M_DrawHelpBackground();
            V_DrawHelpPatch(W_CacheXLumpName(lumpname, 2));
        }
        else if (W_GetNumLumps(lumpname) > 2)
        {
            V_DrawPagePatch(0, W_CacheLumpName(lumpname));

            if (mapwindow)
                memset(mapscreen, nearestblack, MAPAREA);
        }
        else
        {
            viewplayer->fixedcolormap = 0;
            M_DrawHelpBackground();
            V_DrawHelpPatch(W_CacheLumpName(lumpname));
        }
    }
}

//
// Change SFX and Music volumes
//
static void M_DrawSound(void)
{
    float   dot;

    M_DrawMenuBackground();

    if (M_SVOL)
    {
        M_DrawCenteredPatchWithShadow(38 + OFFSET, W_CacheLumpName("M_SVOL"));
        SoundDef.x = (chex ? 68 : 80);
        SoundDef.y = 64;
    }
    else
    {
        char    *temp = uppercase(s_M_SOUNDVOLUME);

        M_DrawCenteredString(38 + OFFSET, temp);
        free(temp);
    }

    dot = (float)(sfxvolume * !nosfx);
    SoundMenu[sound_empty1].sliderx = MAXWIDESCREENDELTA + SoundDef.x - 1 + 6 + (int)(dot * 4.0f) + 2;
    SoundMenu[sound_empty1].width = 16 * 8 + 12;
    M_DrawSlider(SoundDef.x - 1, SoundDef.y + 16 * (sfx_vol + 1) + OFFSET + !hacx,
        16, 15, dot, 4.0f, 6, (itemon == sfx_vol || itemon == sound_empty1));

    dot = (float)(musicvolume * !nomusic);
    SoundMenu[sound_empty2].sliderx = MAXWIDESCREENDELTA + SoundDef.x - 1 + 6 + (int)(dot * 4.0f) + 2;
    SoundMenu[sound_empty2].width = 16 * 8 + 12;
    M_DrawSlider(SoundDef.x - 1, SoundDef.y + 16 * (music_vol + 1) + OFFSET + !hacx,
        16, 15, dot, 4.0f, 6, (itemon == music_vol || itemon == sound_empty2));
}

static void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

static void M_SfxVol(int choice)
{
    if (nosfx)
        return;

    if (!choice)
    {
        if (sfxvolume > 0)
        {
            S_SetSfxVolume(--sfxvolume * (MIX_MAX_VOLUME - 1) / 31);
            S_StartSound(NULL, sfx_stnmov);
            s_sfxvolume = sfxvolume * 100 / 31;
            C_PercentCVAROutput(stringize(s_sfxvolume), s_sfxvolume);
            M_SaveCVARs();
        }
    }
    else
    {
        if (sfxvolume < 31)
        {
            S_SetSfxVolume(++sfxvolume * (MIX_MAX_VOLUME - 1) / 31);
            S_StartSound(NULL, sfx_stnmov);
            s_sfxvolume = sfxvolume * 100 / 31;
            C_PercentCVAROutput(stringize(s_sfxvolume), s_sfxvolume);
            M_SaveCVARs();
        }
    }
}

static void M_MusicVol(int choice)
{
    if (nomusic)
        return;

    if (!choice)
    {
        if (musicvolume > 0)
        {
            musicvolume--;
            S_LowerMusicVolume();
            S_StartSound(NULL, sfx_stnmov);
            s_musicvolume = musicvolume * 100 / 31;
            C_PercentCVAROutput(stringize(s_musicvolume), s_musicvolume);
            M_SaveCVARs();
        }
    }
    else
    {
        if (musicvolume < 31)
        {
            musicvolume++;
            S_LowerMusicVolume();
            S_StartSound(NULL, sfx_stnmov);
            s_musicvolume = musicvolume * 100 / 31;
            C_PercentCVAROutput(stringize(s_musicvolume), s_musicvolume);
            M_SaveCVARs();
        }
    }
}

//
// M_DrawMainMenu
//
static void M_DrawMainMenu(void)
{
    M_DrawMenuBackground();

    if (FREEDOOM || chex || hacx || harmony || REKKRSA || autosigil)
    {
        M_DrawPatchWithShadow(94, 2 + OFFSET, W_CacheLastLumpName("M_DOOM"), false);
        MainDef.x = 97;
        MainDef.y = 72;
    }
    else if (M_DOOM)
    {
        if (titleheight == VANILLAHEIGHT)
            V_DrawPatch(94, 2, 0, W_CacheLumpName("M_DOOM"));
        else if (masterlevels)
            M_DrawCenteredPatchWithShadow(2 + OFFSET, W_CacheLumpName("M_DOOM"));
        else
            M_DrawPatchWithShadow(94, 2 + OFFSET, W_CacheLumpName("M_DOOM"), false);

        MainDef.x = 97;
        MainDef.y = (legacyofrust ? 76 : 72);
    }
    else
    {
        patch_t *patch = (gamemission == doom ? W_CacheLumpName("M_DOOM") : W_CacheLastLumpName("M_DOOM"));

        V_DrawMenuPatch((VANILLAWIDTH - SHORT(patch->width)) / 2 - 1, 11 + OFFSET, patch, false, SCREENWIDTH);
    }
}

//
// M_Episode
//
static int      epi;
bool            customepisodes = false;
static short    episodemenumap[] = { 1, 1, 1, 1, -1, -1, -1, -1, -1, -1 };
static short    episodemenuepisode[] = { 1, 2, 3, 4, -1, -1, -1, -1, -1, -1 };

void M_AddEpisode(int map, const int ep, const char *lumpname, const char *string)
{
    if (gamemode == commercial)
    {
        static int  episodecount = 0;
        static int  prevmap;

        if (map != prevmap)
        {
            episodecount++;
            prevmap = map;
        }

        maptoepisode[map] = episodecount;

        for (int i = map - 1; i >= 0; i--)
            if (!maptoepisode[i])
                maptoepisode[i] = episodecount - 1;
            else
                break;

        for (int i = map + 1; i < 100; i++)
            if (maptoepisode[i] < episodecount)
                maptoepisode[i] = episodecount;

        map -= (ep - 1) * 10;
    }
    else
    {
        maptoepisode[map] = ep;

        for (int i = map - 1; i >= 0; i--)
            if (!maptoepisode[i])
                maptoepisode[i] = ep - 1;
            else
                break;
    }

    if (!customepisodes)
    {
        customepisodes = true;

        if (gamemode == commercial)
            EpiDef.numitems = 0;
        else if (EpiDef.numitems > 4)
            EpiDef.numitems = 4;
    }

    if (!*lumpname && !*string)
        EpiDef.numitems = 0;
    else
    {
        if (EpiDef.numitems >= 10)
            return;

        for (int i = 0; i < EpiDef.numitems; i++)
            if (episodemenuepisode[i] == ep && episodemenumap[i] == map)
                return;

        episodemenuepisode[EpiDef.numitems] = ep;
        episodemenumap[EpiDef.numitems] = map;
        M_StringCopy(EpisodeMenu[EpiDef.numitems].name, lumpname, sizeof(EpisodeMenu[0].name));
        *EpisodeMenu[EpiDef.numitems].text = M_StringDuplicate(string);
        *episodes[EpiDef.numitems++] = M_StringDuplicate(string);
    }

    NewDef.prevmenu = (EpiDef.numitems > 1 ? &EpiDef : &MainDef);
}

static void M_DrawEpisode(void)
{
    M_DrawMenuBackground();

    if (EpiDef.numitems >= 7 || (EpiDef.numitems == 6 && !sigil && !sigil2))
    {
        EpiDef.y = 63 - (EpiDef.numitems - 4) * (LINEHEIGHT / 2);

        if (M_EPISOD)
        {
            M_DrawCenteredPatchWithShadow(EpiDef.y - 5, W_CacheLumpName("M_EPISOD"));
            EpiDef.x = 48;
        }
        else
            M_DrawCenteredString(EpiDef.y - 5, s_M_WHICHEPISODE);
    }
    else
    {
        if (M_NEWG)
        {
            M_DrawCenteredPatchWithShadow(14 + OFFSET, W_CacheLumpName("M_NEWG"));
            EpiDef.x = 48;
            EpiDef.y = 63;
        }
        else if (M_NGAME)
        {
            M_DrawCenteredPatchWithShadow(14 + OFFSET, W_CacheLumpName("M_NGAME"));
            EpiDef.x = 48;
            EpiDef.y = 63;
        }
        else
        {
            char    *temp = uppercase(s_M_NEWGAME);

            M_DrawCenteredString(19 + OFFSET, temp);
            free(temp);
        }

        if (M_EPISOD)
        {
            M_DrawCenteredPatchWithShadow(38 + OFFSET, W_CacheLumpName("M_EPISOD"));
            EpiDef.x = 48;
            EpiDef.y = 63;
        }
        else
            M_DrawCenteredString(44 + OFFSET, s_M_WHICHEPISODE);
    }
}

void M_SetWindowCaption(void)
{
    static char caption[512];

    if (gamestate == GS_LEVEL)
    {
        if (gamemode == commercial)
        {
            if (customepisodes)
                M_snprintf(caption, sizeof(caption), "%s \xC2\xB7 %s \xC2\xB7 %s \xC2\xB7 %s",
                    mapnumandtitle, *episodes[maptoepisode[gamemap] - 1], gamedescription, DOOMRETRO_NAME);
            else if ((gamemission == doom2 && !nerve) || gamemission == pack_plut
                || gamemission == pack_tnt || M_StringEndsWith(gamedescription, ".wad"))
                M_snprintf(caption, sizeof(caption), "%s \xC2\xB7 %s \xC2\xB7 %s",
                    mapnumandtitle, gamedescription, DOOMRETRO_NAME);
            else
                M_snprintf(caption, sizeof(caption), "%s \xC2\xB7 %s: %s \xC2\xB7 %s",
                    mapnumandtitle, gamedescription, *expansions[(gamemission == pack_nerve ? ex2 : ex1)],
                    DOOMRETRO_NAME);
        }
        else
            M_snprintf(caption, sizeof(caption), "%s \xC2\xB7 %s \xC2\xB7 %s \xC2\xB7 %s",
                mapnumandtitle, *episodes[gameepisode - 1], gamedescription, DOOMRETRO_NAME);
    }
    else
        M_snprintf(caption, sizeof(caption), "%s \xC2\xB7 %s", gamedescription, DOOMRETRO_NAME);

    SDL_SetWindowTitle(window, caption);
}

static void M_DrawExpansion(void)
{
    M_DrawMenuBackground();

    if (M_NEWG)
    {
        M_DrawCenteredPatchWithShadow(14 + OFFSET, W_CacheLumpName("M_NEWG"));
        EpiDef.x = 48;
        EpiDef.y = 63;
    }
    else if (M_NGAME)
    {
        M_DrawCenteredPatchWithShadow(14 + OFFSET, W_CacheLumpName("M_NGAME"));
        EpiDef.x = 48;
        EpiDef.y = 63;
    }
    else
    {
        char    *temp = uppercase(s_M_NEWGAME);

        M_DrawCenteredString(19 + OFFSET, temp);
        free(temp);
    }

    if (M_EPISOD)
    {
        M_DrawCenteredPatchWithShadow(38 + OFFSET, W_CacheLumpName("M_EPISOD"));
        EpiDef.x = 48;
        EpiDef.y = 63;
    }
    else
        M_DrawCenteredString(44 + OFFSET, s_M_WHICHEXPANSION);
}

static void M_VerifyNightmare(int key)
{
    messagetoprint = false;

    if (key != 'y')
        M_SetupNextMenu(&NewDef);
    else
    {
        HU_DrawDisk();
        quicksaveslot = -1;
        M_CloseMenu();
        viewplayer->cheats = 0;
        G_DeferredInitNew((skill_t)nightmare, epi + 1, 1);
    }
}

static void M_ChooseSkill(int choice)
{
    if (choice == nightmare && gameskill != sk_nightmare && !nomonsters)
    {
        static char line2[160];
        static char nightmarestring[320];

        M_snprintf(line2, sizeof(line2), (usingcontroller ? s_PRESSA : s_PRESSYN), selectbutton);
        M_snprintf(nightmarestring, sizeof(nightmarestring), "%s\n\n%s", s_NIGHTMARE, line2);
        M_StartMessage(nightmarestring, &M_VerifyNightmare, true);
        return;
    }

    HU_DrawDisk();
    S_StartSound(NULL, sfx_pistol);
    quicksaveslot = -1;
    M_CloseMenu();
    viewplayer->cheats = 0;

    if (KDIKDIZD)
        G_DeferredInitNew((skill_t)choice, 1, 13);
    else if (customepisodes)
        G_DeferredInitNew((skill_t)choice, episodemenuepisode[epi], episodemenumap[epi]);
    else
        G_DeferredInitNew((skill_t)choice, epi + 1, 1);
}

static void M_Episode(int choice)
{
    if (!customepisodes)
    {
        if (gamemode == shareware && choice)
        {
            if (M_StringEndsWith(s_SWSTRING, s_PRESSYN))
                M_StartMessage(s_SWSTRING, NULL, false);
            else
            {
                static char buffer[160];

                M_snprintf(buffer, sizeof(buffer), (usingcontroller ? s_PRESSA : s_PRESSYN), selectbutton);
                M_snprintf(buffer, sizeof(buffer), "%s\n\n%s", s_SWSTRING, buffer);
                M_StartMessage(buffer, NULL, false);
            }

            M_SetupNextMenu(&EpiDef);
            return;
        }
    }

    epi = choice;
    M_SetupNextMenu(&NewDef);
}

static void M_Expansion(int choice)
{
    gamemission = (choice == ex1 ? doom2 : pack_nerve);
    D_SetSaveGameFolder(false);
    M_ReadSaveStrings();
    M_SetupNextMenu(&NewDef);
}

//
// M_NewGame
//
static void M_DrawNewGame(void)
{
    M_DrawMenuBackground();

    if (M_NEWG)
    {
        M_DrawCenteredPatchWithShadow(14 + OFFSET, W_CacheLumpName("M_NEWG"));
        NewDef.x = (chex ? 98 : 48);
        NewDef.y = 63;
    }
    else if (M_NGAME)
    {
        M_DrawCenteredPatchWithShadow(14 + OFFSET, W_CacheLumpName("M_NGAME"));
        NewDef.x = (chex ? 98 : 48);
        NewDef.y = 63;
    }
    else
    {
        char    *temp = uppercase(s_M_NEWGAME);

        M_DrawCenteredString(19 + OFFSET, temp);
        free(temp);
    }

    if (M_SKILL)
    {
        M_DrawCenteredPatchWithShadow(38 + OFFSET, W_CacheLumpName("M_SKILL"));
        NewDef.x = (chex ? 98 : 48);
        NewDef.y = 63;
    }
    else
        M_DrawCenteredString(44 + OFFSET, s_M_CHOOSESKILLLEVEL);
}

static void M_NewGame(int choice)
{
    M_SetupNextMenu(chex ? &NewDef : ((gamemode == commercial && !customepisodes) || EpiDef.numitems <= 1 ?
        (nerve ? &ExpDef : &NewDef) : &EpiDef));
}

//
// M_Options
//
static void M_DrawOptions(void)
{
    float   dot;

    M_DrawMenuBackground();

    if (M_OPTTTL)
    {
        M_DrawCenteredPatchWithShadow(OptionsDef.y - 7, W_CacheLumpName("M_OPTTTL"));
        OptionsDef.x = (chex ? 69 : 60);
    }
    else
    {
        char    *temp = uppercase(s_M_OPTIONS);

        M_DrawCenteredString(OFFSET, temp);
        free(temp);
    }

    if (messages)
    {
        if (M_MSGON)
        {
            short   width = SHORT(((patch_t *)W_CacheLumpName(OptionsMenu[msgs].name))->width);
            patch_t *patch = W_CacheLumpName("M_MSGON");

            patch->leftoffset = 0;

            M_DrawPatchWithShadow(OptionsDef.x + (REKKR ? width - 11 : width + 6),
                OptionsDef.y + 16 * msgs + (REKKR ? OFFSET + 2 : OFFSET),
                patch, (itemon == msgs));
        }
        else
            M_DrawString(OptionsDef.x + M_BigStringWidth(*currentmenu->menuitems[msgs].text) + M_BigStringWidth(" "),
                OptionsDef.y + 16 * msgs + OFFSET, s_M_ON, (itemon == msgs), true);
    }
    else
    {
        if (M_MSGOFF)
        {
            short   width = SHORT(((patch_t *)W_CacheLumpName(OptionsMenu[msgs].name))->width);
            patch_t *patch = W_CacheLumpName("M_MSGOFF");

            patch->leftoffset = 0;

            M_DrawPatchWithShadow(OptionsDef.x + (REKKR ? width - 11 : width + 6),
                OptionsDef.y + 16 * msgs + (REKKR ? OFFSET + 2 : OFFSET),
                patch, (itemon == msgs));
        }
        else
            M_DrawString(OptionsDef.x + M_BigStringWidth(*currentmenu->menuitems[msgs].text) + M_BigStringWidth(" "),
                OptionsDef.y + 16 * msgs + OFFSET, s_M_OFF, (itemon == msgs), true);
    }

    if (r_detail == r_detail_low)
    {
        if (M_GDLOW)
        {
            short   width = SHORT(((patch_t *)W_CacheLumpName(OptionsMenu[detail].name))->width);
            patch_t *patch = W_CacheLumpName("M_GDLOW");

            patch->leftoffset = 0;

            M_DrawPatchWithShadow(OptionsDef.x + (REKKR ? width - 13 : width + 6),
                OptionsDef.y + 16 * detail + (REKKR ? OFFSET + 2 : OFFSET),
                patch, (itemon == detail));
        }
        else
            M_DrawString(OptionsDef.x + M_BigStringWidth(*currentmenu->menuitems[detail].text) + M_BigStringWidth(" "),
                OptionsDef.y + 16 * detail + OFFSET, s_M_LOW, (itemon == detail), true);
    }
    else
    {
        if (M_GDHIGH)
        {
            short   width = SHORT(((patch_t *)W_CacheLumpName(OptionsMenu[detail].name))->width);
            patch_t *patch = W_CacheLumpName("M_GDHIGH");

            patch->leftoffset = 0;

            M_DrawPatchWithShadow(OptionsDef.x + (REKKR ? width - 13 : width + 6),
                OptionsDef.y + 16 * detail + (REKKR ? OFFSET + 2 : OFFSET),
                patch, (itemon == detail));
        }
        else
            M_DrawString(OptionsDef.x + M_BigStringWidth(*currentmenu->menuitems[detail].text) + M_BigStringWidth(" "),
                OptionsDef.y + 16 * detail + OFFSET, s_M_HIGH, (itemon == detail), true);
    }

    dot = (float)(r_screensize + (r_screensize < r_screensize_max - 1 ? 0 :
        (r_screensize == r_screensize_max - 1 ? vid_widescreen : 1 + !r_hud)));
    OptionsMenu[option_empty1].sliderx = MAXWIDESCREENDELTA
        + OptionsDef.x - 1 + 8 + (int)(dot * 6.54f) + 2;
    OptionsMenu[option_empty1].width = 16 * 8 + 12;
    M_DrawSlider(OptionsDef.x - 1, OptionsDef.y + 16 * (scrnsize + 1) + OFFSET + !hacx,
        9, 15, dot, 6.54f, 8, (itemon == scrnsize || itemon == option_empty1));

    if (usingcontroller && (!M_MSENS || DBIGFONT))
    {
        dot = roundf(joy_sensitivity_horizontal) / joy_sensitivity_horizontal_max * 8.0f;
        OptionsMenu[option_empty2].sliderx = MAXWIDESCREENDELTA
            + OptionsDef.x - 1 + 8 + (int)(dot * 8.0f) + 2;
        OptionsMenu[option_empty2].width = 16 * 8 + 12;
        M_DrawSlider(OptionsDef.x - 1, OptionsDef.y + 16 * (mousesens + 1) + OFFSET + !hacx,
            9, 15, dot, 8.0f, 8, (itemon == mousesens || itemon == option_empty2));
    }
    else
    {
        dot = roundf(m_sensitivity) / m_sensitivity_max * 8.0f;
        OptionsMenu[option_empty2].sliderx = MAXWIDESCREENDELTA
            + OptionsDef.x - 1 + 8 + (int)(dot * 8.0f) + 2;
        OptionsMenu[option_empty2].width = 16 * 8 + 12;
        M_DrawSlider(OptionsDef.x - 1, OptionsDef.y + 16 * (mousesens + 1) + OFFSET + !hacx,
            9, 15, dot, 8.0f, 8, (itemon == mousesens || itemon == option_empty2));
    }
}

static void M_Options(int choice)
{
    if (!OptionsDef.change)
        OptionsDef.laston = (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION ? endgame : msgs);

    M_SetupNextMenu(&OptionsDef);
}

//
// Toggle messages on/off
//
static void M_ChangeMessages(int choice)
{
    messages = !messages;
    C_StringCVAROutput(stringize(messages), (messages ? "on" : "off"));

    if (!menuactive)
    {
        if (messages)
        {
            C_Output(s_MSGON);
            HU_SetPlayerMessage(s_MSGON, false, false);
        }
        else
        {
            C_Output(s_MSGOFF);
            HU_SetPlayerMessage(s_MSGOFF, false, false);
        }

        message_dontfuckwithme = true;
    }
    else
        C_Output(messages ? s_MSGON : s_MSGOFF);

    M_SaveCVARs();
}

//
// M_EndGame
//
static bool endinggame;

static void M_EndGameResponse(int key)
{
    messagetoprint = false;

    if (consoleactive)
        SDL_StartTextInput();

    if (key != 'y')
    {
        if (functionkey == KEY_F7)
            M_CloseMenu();
        else
            M_SetupNextMenu(&OptionsDef);

        return;
    }

    currentmenu->laston = itemon + 1;
    S_StopMusic();
    C_HideConsoleFast();
    M_CloseMenu();
    viewactive = false;
    automapactive = false;
    maptime = 0;
    S_StartSound(NULL, sfx_swtchx);
    MainDef.laston = 0;
    st_palette = 0;
    endinggame = true;

    if (gamemission == pack_nerve)
        gamemission = doom2;

    if (!M_StringCompare(console[numconsolestrings - 1].string, "endgame"))
        C_Input("endgame");

    C_AddConsoleDivider();
    D_StartTitle(1);
}

void M_EndGame(int choice)
{
    if (gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
        return;

    if (M_StringEndsWith(s_ENDGAME, s_PRESSYN))
        M_StartMessage(s_ENDGAME, &M_EndGameResponse, true);
    else
    {
        static char line2[160];
        static char endgamestring[320];

        M_snprintf(line2, sizeof(line2), (usingcontroller ? s_PRESSA : s_PRESSYN), selectbutton);
        M_snprintf(endgamestring, sizeof(endgamestring), "%s\n\n%s", s_ENDGAME, line2);
        M_StartMessage(endgamestring, &M_EndGameResponse, true);

        SDL_StopTextInput();
        S_StartSound(NULL, sfx_swtchn);
    }
}

static void M_Console(int choice)
{
    M_CloseMenu();
    C_ShowConsole(true);
    D_FadeScreen(false);
    S_StartSound(NULL, sfx_pistol);
}

//
// M_QuitDOOM
//
static const int quitsounds[8] =
{
    sfx_pldeth,
    sfx_dmpain,
    sfx_popain,
    sfx_slop,
    sfx_telept,
    sfx_posit1,
    sfx_posit3,
    sfx_sgtatk
};

static const int quitsounds2[8] =
{
    sfx_vilact,
    sfx_getpow,
    sfx_boscub,
    sfx_slop,
    sfx_skeswg,
    sfx_kntdth,
    sfx_bspact,
    sfx_sgtatk
};

void M_QuitResponse(int key)
{
    messagetoprint = false;

    if (key != 'y')
    {
        quitting = false;

        if (waspaused)
        {
            waspaused = false;
            paused = true;
        }

        if (functionkey == KEY_F10)
            M_CloseMenu();
        else
            M_SetupNextMenu(&MainDef);

        return;
    }

    if (!nosfx && sfxvolume > 0)
    {
        int i = 30;

        if (gamemode == commercial)
            S_StartSound(NULL, quitsounds2[M_Random() & 7]);
        else
            S_StartSound(NULL, quitsounds[M_Random() & 7]);

        // wait until all sounds stopped or 3 seconds has passed
        while (i-- > 0 && I_AnySoundStillPlaying())
            I_Sleep(100);
    }

    I_Quit(true);
}

void M_QuitDOOM(int choice)
{
    static char endstring[320];
    static char line1[160];
    static char line2[160];

    quitting = true;

    if (deh_strlookup[p_QUITMSG].assigned == 2)
        M_StringCopy(line1, s_QUITMSG, sizeof(line1));
    else
    {
        static int  msg = -1;

        msg = M_RandomIntNoRepeat(0, NUM_QUITMESSAGES - 1, msg);

        if (devparm)
            M_StringCopy(line1, *devendmsg[msg], sizeof(line1));
        else if (gamemission == doom)
            M_snprintf(line1, sizeof(line1), *endmsg[msg], WINDOWS);
        else
            M_snprintf(line1, sizeof(line1), *endmsg[NUM_QUITMESSAGES + msg], WINDOWS);
    }

    if (usingcontroller)
        M_snprintf(line2, sizeof(line2), s_DOSA, selectbutton, DESKTOP);
    else
        M_snprintf(line2, sizeof(line2), s_DOSY, DESKTOP);

    M_snprintf(endstring, sizeof(endstring), "%s\n\n%s", line1, line2);
    M_StartMessage(endstring, &M_QuitResponse, true);
}

static void M_SliderSound(void)
{
    static uint64_t wait;

    if (wait < I_GetTime())
    {
        wait = I_GetTime() + 7;
        S_StartSound(NULL, sfx_stnmov);
    }
}

static void M_ChangeSensitivity(int choice)
{
    if (usingcontroller && (!M_MSENS || DBIGFONT))
    {
        if (!choice)
        {
            if (joy_sensitivity_horizontal > joy_sensitivity_horizontal_min)
            {
                if ((int)(joy_sensitivity_horizontal = roundf(joy_sensitivity_horizontal)) & 1)
                    joy_sensitivity_horizontal++;

                joy_sensitivity_horizontal -= 2.0f;
                I_SetControllerHorizontalSensitivity();
                C_IntegerCVAROutputNoRepeat(stringize(joy_sensitivity_horizontal),
                    (int)joy_sensitivity_horizontal);
                M_SliderSound();
                M_SaveCVARs();
            }
        }
        else
        {
            if (joy_sensitivity_horizontal < joy_sensitivity_horizontal_max)
            {
                if ((int)(joy_sensitivity_horizontal = roundf(joy_sensitivity_horizontal)) & 1)
                    joy_sensitivity_horizontal--;

                joy_sensitivity_horizontal += 2.0f;
                I_SetControllerHorizontalSensitivity();
                C_IntegerCVAROutputNoRepeat(stringize(joy_sensitivity_horizontal),
                    (int)joy_sensitivity_horizontal);
                M_SliderSound();
                M_SaveCVARs();
            }
        }
    }
    else
    {
        if (!choice)
        {
            if (m_sensitivity > m_sensitivity_min)
            {
                if ((int)(m_sensitivity = roundf(m_sensitivity)) & 1)
                    m_sensitivity++;

                m_sensitivity -= 2.0f;
                C_IntegerCVAROutputNoRepeat(stringize(m_sensitivity), (int)m_sensitivity);
                M_SliderSound();
                M_SaveCVARs();
            }
        }
        else
        {
            if (m_sensitivity < m_sensitivity_max)
            {
                if ((int)(m_sensitivity = roundf(m_sensitivity)) & 1)
                    m_sensitivity--;

                m_sensitivity += 2.0f;
                C_IntegerCVAROutputNoRepeat(stringize(m_sensitivity), (int)m_sensitivity);
                M_SliderSound();
                M_SaveCVARs();
            }
        }
    }
}

static void M_ChangeDetail(int choice)
{
    r_detail = !r_detail;
    C_StringCVAROutput(stringize(r_detail), (r_detail == r_detail_low ? "low" : "high"));

    if (!menuactive)
    {
        if (r_detail == r_detail_low)
        {
            C_Output(s_DETAILLO);
            HU_SetPlayerMessage(s_DETAILLO, false, false);
        }
        else
        {
            C_Output(s_DETAILHI);
            HU_SetPlayerMessage(s_DETAILHI, false, false);
        }

        message_dontfuckwithme = true;
    }
    else
        C_Output(r_detail == r_detail_low ? s_DETAILLO : s_DETAILHI);

    M_SaveCVARs();
    STLib_Init();
    R_InitColumnFunctions();
    AM_InitPixelSize();
}

static void M_SizeDisplay(int choice)
{
    if (!choice)
    {
        if (r_screensize == r_screensize_max && !r_hud)
        {
            r_hud = true;
            C_StringCVAROutput(stringize(r_hud), "on");
            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();
        }
        else if (r_screensize == r_screensize_max - 1 && vid_widescreen)
        {
            vid_widescreen = false;
            C_StringCVAROutput(stringize(vid_widescreen), "off");
            I_RestartGraphics(false);
            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();
        }
        else if (r_screensize > r_screensize_min)
        {
            C_IntegerCVAROutputNoRepeat(stringize(r_screensize), --r_screensize);
            R_SetViewSize(menuactive && viewactive ? r_screensize_max : r_screensize);
            AM_SetAutomapSize(automapactive ? r_screensize_max : r_screensize);

            if (r_screensize == r_screensize_max - 1)
            {
                r_hud = false;
                C_StringCVAROutput(stringize(r_hud), "off");
            }

            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();
        }
        else
            return;
    }
    else
    {
        if (r_screensize == r_screensize_max && r_hud)
        {
            r_hud = false;
            C_StringCVAROutput(stringize(r_hud), "off");
            message_counter = MIN(message_counter, 4);
            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();
        }
        else if (r_screensize == r_screensize_max - 1 && !vid_widescreen && !nowidescreen)
        {
            vid_widescreen = true;
            C_StringCVAROutput(stringize(vid_widescreen), "on");
            I_RestartGraphics(false);
            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();
        }
        else if (r_screensize < r_screensize_max)
        {
            C_IntegerCVAROutputNoRepeat(stringize(r_screensize), ++r_screensize);
            R_SetViewSize(menuactive && viewactive ? r_screensize_max : r_screensize);
            AM_SetAutomapSize(automapactive ? r_screensize_max : r_screensize);

            if (r_screensize == r_screensize_max)
            {
                r_hud = true;
                C_StringCVAROutput(stringize(r_hud), "on");
            }

            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();
        }
        else
            return;
    }

    M_SaveCVARs();

    blurtic = -1;

    if (r_playersprites)
        skippsprinterp = true;
}

//
// Menu Functions
//
static void M_DrawSlider(int x, int y, int width, int shadowwidth, float dot, float factor, int offset, bool highlight)
{
    int xx = x;

    if (chex || hacx)
    {
        xx--;
        y -= 2;
    }

    M_DrawPatchWithShadow(xx, y, W_CacheLumpName("M_THERML"), highlight);
    xx += 8;

    for (int i = 0; i < width; i++)
    {
        V_DrawMenuPatch(xx, y, W_CacheLumpName("M_THERMM"), highlight, shadowwidth);
        xx += 8;
    }

    M_DrawPatchWithShadow(xx, y, W_CacheLumpName("M_THERMR"), highlight);
    V_DrawMenuPatch(x + offset + (int)(dot * factor), y, W_CacheLumpName("M_THERMO"), highlight, 0);
}

void M_StartMessage(char *string, void (*routine)(int), bool input)
{
    messagelastmenuactive = menuactive;
    messagetoprint = true;
    messagestring = string;
    messageroutine = routine;
    messageneedsinput = input;

    I_SetPalette(PLAYPAL);
    I_UpdateBlitFunc(false);

    D_FadeScreen(false);
}

//
// Find character width
//
int M_CharacterWidth(unsigned char ch, unsigned char prevletter)
{
    const int   c = toupper(ch) - HU_FONTSTART;

    if (c < 0 || c >= HU_FONTSIZE)
        return (prevletter == '.' || prevletter == '!' || prevletter == '?' ? 5 : 3);
    else
        return (STCFNxxx ? SHORT(hu_font[c]->width) : (int)strlen(smallcharset[c]) / 10 - 1);
}

//
// Find string width
//
int M_StringWidth(const char *string)
{
    const int   len = (int)strlen(string);
    int         width;

    if (!len)
        return 0;

    width = M_CharacterWidth(string[0], '\0');

    for (int i = 1; i < len; i++)
    {
        width += M_CharacterWidth(string[i], string[i - 1]);

        for (int k = 0; kern[k].char1; k++)
            if (string[i - 1] == kern[k].char1 && string[i] == kern[k].char2)
            {
                width += kern[k].adjust;
                break;
            }
    }

    return width;
}

//
// Find string height
//
static int M_StringHeight(const char *string)
{
    const int   len = (int)strlen(string);
    int         height = (STCFNxxx ? SHORT(hu_font[0]->height) : 8);

    for (int i = 1; i < len; i++)
        if (string[i] == '\n')
            height += (string[i - 1] == '\n' ? 3 : (STCFNxxx ? SHORT(hu_font[0]->height) : 8) + 1);

    return height;
}

//
// Write a char
//
void M_DrawSmallChar(int x, int y, int i, bool highlight, bool shadow)
{
    const int   width = (int)strlen(smallcharset[i]) / 10;

    for (int y1 = 0; y1 < 10; y1++)
        for (int x1 = 0; x1 < width; x1++)
            V_DrawPixel(x + x1, y + y1, (int)smallcharset[i][y1 * width + x1], highlight, shadow);
}

//
// Write a string
//
static void M_WriteText(int x, int y, const char *string, bool highlight, bool shadow, unsigned char prevletter)
{
    int     width;
    char    letter;
    int     cx = x;
    int     cy = y;

    while (true)
    {
        int c = *string++;

        if (!c)
            break;

        if (c == '\n')
        {
            cx = x;
            cy += 12;
            prevletter = '\0';
            continue;
        }

        letter = toupper(c);
        c = letter - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE)
        {
            cx += (prevletter == '.' || prevletter == '!' || prevletter == '?' ? 5 : 3);
            prevletter = letter;
            continue;
        }

        if (STCFNxxx)
        {
            width = SHORT(hu_font[c]->width);

            if (cx + width > VANILLAWIDTH)
                break;

            if (shadow)
                M_DrawPatchWithShadow(cx, cy, hu_font[c], highlight);
            else
                V_DrawPatch(cx, cy, 0, hu_font[c]);
        }
        else
        {
            if (prevletter == '\0' || prevletter == ' ')
            {
                if (letter == '"')
                    c = 64;
                else if (letter == '\'')
                    c = 65;
            }

            for (int k = 0; kern[k].char1; k++)
                if (prevletter == kern[k].char1 && letter == kern[k].char2)
                {
                    cx += kern[k].adjust;
                    break;
                }

            width = (int)strlen(smallcharset[c]) / 10 - 1;

            if (cx + width > VANILLAWIDTH)
                break;

            M_DrawSmallChar(cx, cy - 1, c, highlight, shadow);
        }

        prevletter = letter;
        cx += width;
    }
}

static void M_ShowHelp(int choice)
{
    functionkey = KEY_F1;
    helpscreen = true;
    M_OpenMainMenu();
    currentmenu = &HelpDef;
    itemon = 0;
    S_StartSound(NULL, sfx_swtchn);

    if (gamestate == GS_LEVEL)
        R_SetViewSize(r_screensize_max);
}

static void M_ChangeGamma(bool shift)
{
    static uint64_t gammawait;

    if (gammawait < I_GetTime())
    {
        if (shift)
        {
            if (--gammaindex < 0)
                gammaindex = GAMMALEVELS - 1;
        }
        else
        {
            if (++gammaindex > GAMMALEVELS - 1)
                gammaindex = 0;
        }

        r_gamma = gammalevels[gammaindex];
        I_UpdateColors();

        if (r_gamma == 1.0f)
            C_StringCVAROutput(stringize(r_gamma), "off");
        else
        {
            static char buffer[128];
            int         len;

            M_snprintf(buffer, sizeof(buffer), "%.2f", r_gamma);
            len = (int)strlen(buffer);

            if (len >= 2 && buffer[len - 1] == '0' && buffer[len - 2] == '0')
                buffer[len - 1] = '\0';

            C_StringCVAROutput(stringize(r_gamma), buffer);
        }

        gammawait = I_GetTime() + 4;
        S_StartSound(NULL, sfx_stnmov);
    }

    if (r_gamma == 1.0f)
    {
        C_Output(s_GAMMAOFF);
        HU_SetPlayerMessage(s_GAMMAOFF, false, false);
    }
    else
    {
        static char buffer[128];
        int         len;

        M_snprintf(buffer, sizeof(buffer), s_GAMMALVL, r_gamma);
        len = (int)strlen(buffer);

        if (len >= 2 && buffer[len - 1] == '0' && buffer[len - 2] == '0')
            buffer[len - 1] = '\0';

        C_Output(buffer);
        HU_SetPlayerMessage(buffer, false, false);
    }

    message_dontfuckwithme = true;
    I_SetPalette(&PLAYPAL[st_palette * 768]);
    M_SaveCVARs();
}

//
// M_Responder
//
uint64_t    controllerwait = 0;
uint64_t    mousewait = 0;
bool        controllerpress = false;

bool M_Responder(event_t *ev)
{
    int             key = -1;
    static uint64_t keywait;

    if (idclevtics)
        return false;

    if (ev->type == ev_controller)
    {
        if (menuactive && controllerwait < I_GetTime())
        {
            // activate menu item
            if ((controllerbuttons & CONTROLLER_A)
                || (controllerbuttons & CONTROLLER_RIGHT_TRIGGER))
            {
                key = (messagetoprint && messageneedsinput ? 'y' : KEY_ENTER);
                controllerwait = I_GetTime()
                    + (uint64_t)2 * !(currentmenu == &OptionsDef && itemon == mousesens);
                usingcontroller = true;
            }

            // previous/exit menu
            else if (controllerbuttons & CONTROLLER_B)
            {
                key = (messagetoprint && messageneedsinput ? 'n' : KEY_BACKSPACE);
                controllerwait = I_GetTime() + 2;
                controllerpress = true;
                usingcontroller = true;
            }

            // exit menu
            else if ((controllerbuttons & controllermenu) && keyboardmenu)
            {
                key = keyboardmenu;
                currentmenu = &MainDef;
                itemon = MainDef.laston;
                controllerwait = I_GetTime() + 2;
                usingcontroller = true;
            }

            else if (!messagetoprint)
            {
                // select previous menu item
                if (controllerthumbLY < -CONTROLLER_THUMB_THRESHOLD
                    || controllerthumbRY < -CONTROLLER_THUMB_THRESHOLD
                    || (controllerbuttons & CONTROLLER_DPAD_UP))
                {
                    key = KEY_UPARROW;
                    keywait = 0;
                    controllerwait = I_GetTime() + 6;
                    usingcontroller = true;
                }

                // select next menu item
                else if (controllerthumbLY > CONTROLLER_THUMB_THRESHOLD
                    || controllerthumbRY > CONTROLLER_THUMB_THRESHOLD
                    || (controllerbuttons & CONTROLLER_DPAD_DOWN))
                {
                    key = KEY_DOWNARROW;
                    keywait = 0;
                    controllerwait = I_GetTime() + 6;
                    usingcontroller = true;
                }

                // decrease slider
                else if ((controllerthumbLX < -CONTROLLER_THUMB_THRESHOLD
                    || controllerthumbRX < -CONTROLLER_THUMB_THRESHOLD
                    || (controllerbuttons & CONTROLLER_DPAD_LEFT))
                    && !savestringenter)
                {
                    key = KEY_LEFTARROW;
                    controllerwait = I_GetTime()
                        + (uint64_t)6 * !(currentmenu == &OptionsDef && itemon == mousesens);
                    usingcontroller = true;
                }

                // increase slider
                else if ((controllerthumbLX > CONTROLLER_THUMB_THRESHOLD
                    || controllerthumbRX > CONTROLLER_THUMB_THRESHOLD
                    || (controllerbuttons & CONTROLLER_DPAD_RIGHT))
                    && !savestringenter)
                {
                    key = KEY_RIGHTARROW;
                    controllerwait = I_GetTime()
                        + (uint64_t)6 * !(currentmenu == &OptionsDef && itemon == mousesens);
                    usingcontroller = true;
                }
            }
        }
        else
        {
            // open menu
            if ((controllerbuttons & controllermenu) && controllerwait < I_GetTime() && keyboardmenu)
            {
                key = keyboardmenu;
                controllerwait = I_GetTime() + 2;
                usingcontroller = true;
            }

            // open console
            else if ((controllerbuttons & controllerconsole) && controllerwait < I_GetTime())
            {
                controllerwait = I_GetTime() + 2;
                usingcontroller = true;
                C_ShowConsole(false);
                return false;
            }

            // screenshot
            else if ((controllerbuttons & controllerscreenshot) && controllerwait < I_GetTime())
            {
                controllerwait = I_GetTime() + 2;
                usingcontroller = true;
                G_ScreenShot();
                S_StartSound(NULL, sfx_scrsht);
                memset(screens[0], nearestwhite, SCREENAREA);
                D_FadeScreen(true);
                return false;
            }
        }
    }
    else if (ev->type == ev_mouse)
    {
        if (menuactive)
        {
            if (m_pointer)
            {
                // activate menu item
                if (ev->data1 & MOUSE_LEFTBUTTON)
                {
                    if (messagetoprint || helpscreen || !usingmouse)
                    {
                        if (mousewait < I_GetTime() && !savestringenter)
                        {
                            key = KEY_ENTER;
                            mousewait = I_GetTime() + 8;
                        }
                    }
                    else
                    {
                        const menuitem_t    *menuitem = &currentmenu->menuitems[itemon];

                        if (ev->data2 >= menuitem->x && ev->data2 < menuitem->x + menuitem->width
                            && ev->data3 >= menuitem->y && ev->data3 < menuitem->y + menuitem->height)
                        {
                            if (currentmenu == &OptionsDef)
                            {
                                if (itemon == scrnsize || itemon == mousesens)
                                    key = -1;
                                else if (itemon == option_empty1)
                                    key = (ev->data2 < OptionsMenu[itemon].sliderx ? KEY_LEFTARROW :
                                        (ev->data2 > OptionsMenu[itemon].sliderx + 6 ? KEY_RIGHTARROW : -1));
                                else if (itemon == option_empty2)
                                    key = (ev->data2 < OptionsMenu[itemon].sliderx ? KEY_LEFTARROW :
                                        (ev->data2 > OptionsMenu[itemon].sliderx + 4 ? KEY_RIGHTARROW : -1));
                                else if (mousewait < I_GetTime())
                                {
                                    key = KEY_ENTER;
                                    mousewait = I_GetTime() + 8;
                                }
                            }
                            else if (currentmenu == &SoundDef)
                            {
                                if (itemon == sfx_vol || itemon == music_vol)
                                    key = -1;
                                else if (itemon == sound_empty1 || itemon == sound_empty2)
                                    key = (ev->data2 < SoundMenu[itemon].sliderx ? KEY_LEFTARROW :
                                        (ev->data2 > SoundMenu[itemon].sliderx + 6 ? KEY_RIGHTARROW : -1));
                                else if (mousewait < I_GetTime())
                                {
                                    key = KEY_ENTER;
                                    mousewait = I_GetTime() + 8;
                                }
                            }
                            else if (mousewait < I_GetTime())
                            {
                                if (savestringenter)
                                {
                                    const int   oldsavecharindex = savecharindex;

                                    M_SetCaretPos(ev->data2);

                                    if (savecharindex != oldsavecharindex)
                                    {
                                        caretwait = SKULLANIMCOUNT;
                                        showcaret = true;
                                    }
                                }
                                else
                                {
                                    key = KEY_ENTER;
                                    mousewait = I_GetTime() + 8;
                                }
                            }
                        }
                        else if (savestringenter)
                        {
                            SDL_StopTextInput();
                            savestringenter = false;
                            caretwait = 0;
                            showcaret = true;
                            M_StringCopy(savegamestrings[itemon], saveoldstring, sizeof(savegamestrings[0]));
                            S_StartSound(NULL, sfx_swtchx);
                        }
                    }

                    usingcontroller = false;
                }

                // previous menu
                else if ((ev->data1 & MOUSE_RIGHTBUTTON) && mousewait < I_GetTime())
                {
                    key = KEY_ESCAPE;
                    mousewait = I_GetTime() + 8;
                    usingcontroller = false;
                }

                // select menu item
                if ((ev->data2 || ev->data3) && mousewait < I_GetTime()
                    && !messagetoprint && !savestringenter && !helpscreen)
                    for (int i = 0; i < currentmenu->numitems; i++)
                    {
                        const menuitem_t    *menuitem = &currentmenu->menuitems[i];

                        if (ev->data2 >= menuitem->x && ev->data2 < menuitem->x + menuitem->width
                            && ev->data3 >= menuitem->y && ev->data3 < menuitem->y + menuitem->height)
                        {
                            if (currentmenu == &MainDef)
                            {
                                if ((i == load_game && !savegames)
                                    || (i == save_game && (gamestate != GS_LEVEL || viewplayer->health <= 0)))
                                    continue;
                            }
                            else if (currentmenu == &EpiDef)
                            {
                                if (i == ep5 && !sigil && sigil2)
                                    continue;
                            }
                            else if (currentmenu == &ExpDef)
                            {
                                if (gamestate != GS_LEVEL)
                                    gamemission = (expansion == 2 && nerve ? pack_nerve : doom2);
                            }
                            else if (currentmenu == &SaveDef)
                                LoadDef.laston = itemon;
                            else if (currentmenu == &LoadDef)
                            {
                                if (M_StringCompare(savegamestrings[i], s_EMPTYSTRING))
                                    continue;
                            }
                            else if (currentmenu == &OptionsDef)
                            {
                                if (i == endgame && gamestate != GS_LEVEL)
                                {
                                    usingcontroller = false;
                                    break;
                                }
                                else if (i == option_empty1 || i == option_empty2)
                                {
                                    if (itemon != i && itemon != i - 1)
                                        S_StartSound(NULL, sfx_pstop);
                                }
                                else if (i == scrnsize || i == mousesens)
                                {
                                    if (itemon != i && itemon != i + 1)
                                        S_StartSound(NULL, sfx_pstop);
                                }
                                else if (itemon != i)
                                    S_StartSound(NULL, sfx_pstop);

                                itemon = i;
                                usingcontroller = false;
                                break;
                            }
                            else if (currentmenu == &SoundDef)
                            {
                                if (i == sound_empty1 || i == sound_empty2)
                                {
                                    if (itemon != i && itemon != i - 1)
                                        S_StartSound(NULL, sfx_pstop);
                                }
                                else if (i == sfx_vol || i == music_vol)
                                {
                                    if (itemon != i && itemon != i + 1)
                                        S_StartSound(NULL, sfx_pstop);
                                }
                                else if (itemon != i)
                                    S_StartSound(NULL, sfx_pstop);

                                itemon = i;
                                usingcontroller = false;
                                break;
                            }

                            if (itemon != i)
                                S_StartSound(NULL, sfx_pstop);

                            itemon = i;
                            usingcontroller = false;

                            if (currentmenu == &EpiDef)
                            {
                                episode = itemon + 1;
                                M_SaveCVARs();
                                C_IntegerCVAROutputNoRepeat(stringize(episode), episode);
                            }
                            else if (currentmenu == &ExpDef)
                            {
                                expansion = itemon + 1;
                                M_SaveCVARs();
                                C_IntegerCVAROutputNoRepeat(stringize(expansion), expansion);
                            }
                            else if (currentmenu == &NewDef)
                            {
                                skilllevel = itemon + 1;
                                M_SaveCVARs();
                                C_IntegerCVAROutputNoRepeat(stringize(skilllevel), skilllevel);
                            }

                            break;
                        }
                    }
            }
            else
            {
                // activate menu item
                if ((ev->data1 & MOUSE_LEFTBUTTON) && mousewait < I_GetTime())
                {
                    key = KEY_ENTER;
                    mousewait = I_GetTime() + 8;
                    usingcontroller = false;
                }

                // previous menu
                else if ((ev->data1 & MOUSE_RIGHTBUTTON) && mousewait < I_GetTime())
                {
                    key = KEY_ESCAPE;
                    mousewait = I_GetTime() + 8;
                    usingcontroller = false;
                }

                // menu
                else if ((ev->data1 & mousemenu) && keyboardmenu)
                {
                    key = keyboardmenu;
                    mousewait = I_GetTime() + 8;
                    usingcontroller = false;
                }

                // console
                else if ((ev->data1 & mouseconsole) && keyboardconsole)
                {
                    key = keyboardconsole;
                    mousewait = I_GetTime() + 8;
                    usingcontroller = false;
                }
            }
        }

        // screenshot
        if (mousescreenshot != -1 && (ev->data1 & mousescreenshot) && mousewait < I_GetTime())
        {
            mousewait = I_GetTime() + 5;
            usingcontroller = false;
            G_ScreenShot();
            S_StartSound(NULL, sfx_scrsht);
            memset(screens[0], nearestwhite, SCREENAREA);
            D_FadeScreen(true);
            return false;
        }
    }
    else if (ev->type == ev_mousewheel && mousewait < I_GetTime())
    {
        if (!messagetoprint)
        {
            // select previous menu item
            if (ev->data1 > 0)
            {
                key = KEY_UPARROW;
                mousewait = I_GetTime() + 3;
                usingcontroller = false;
            }

            // select next menu item
            else if (ev->data1 < 0)
            {
                key = KEY_DOWNARROW;
                mousewait = I_GetTime() + 3;
                usingcontroller = false;
            }
        }
    }
    else if (ev->type == ev_keydown)
    {
        key = ev->data1;
        usingcontroller = false;
    }
    else if (ev->type == ev_keyup)
    {
        if ((ev->data1 == keyboardscreenshot || ev->data1 == keyboardscreenshot2)
            && (ev->data1 == KEY_PRINTSCREEN || (gamestate == GS_LEVEL && !consoleactive)))
        {
            S_StartSound(NULL, sfx_scrsht);

            if (!splashscreen)
            {
                memset(screens[0], nearestwhite, SCREENAREA);
                D_FadeScreen(true);
            }
        }

        return false;
    }

    // Console
    if (key && (key == keyboardconsole || key == keyboardconsole2)
        && !paused && !splashscreen && !keydown && !messagetoprint && !savestringenter)
    {
        keydown = key;

        if (menuactive)
        {
            functionkey = 0;
            SDL_StopTextInput();
            M_CloseMenu();
            D_FadeScreen(false);

            if (helpscreen)
                R_SetViewSize(r_screensize);
        }

        if (consoleheight < CONSOLEHEIGHT && consoledirection == -1 && !dowipe)
            C_ShowConsole(false);
        else
            C_HideConsole();

        return true;
    }

    // Save Game string input
    if (savestringenter)
    {
        if (ev->type == ev_textinput)
        {
            const int   ch = toupper(ev->data1);

            if (ch >= ' ' && ch <= '_' && M_StringWidth(savegamestrings[itemon])
                + M_CharacterWidth(ch, '\0') <= SAVESTRINGPIXELWIDTH)
            {
                const int   len = (int)strlen(savegamestrings[itemon]);

                savegamestrings[itemon][len + 1] = '\0';

                for (int i = len; i > savecharindex; i--)
                    savegamestrings[itemon][i] = savegamestrings[itemon][i - 1];

                savegamestrings[itemon][savecharindex++] = ch;
                caretwait = SKULLANIMCOUNT;
                showcaret = true;

                return true;
            }

            return false;
        }

        if (key == keyboardscreenshot || key == keyboardscreenshot2)
        {
            G_ScreenShot();
            return false;
        }

        switch (key)
        {
            // delete character left of caret
            case KEY_BACKSPACE:
                keydown = key;

                if (savecharindex > 0)
                {
                    const int   len = (int)strlen(savegamestrings[itemon]);

                    for (int j = savecharindex - 1; j < len; j++)
                        savegamestrings[itemon][j] = savegamestrings[itemon][j + 1];

                    savecharindex--;
                    caretwait = SKULLANIMCOUNT;
                    showcaret = true;
                }

                break;

            // delete character right of caret
            case KEY_DELETE:
            {
                const int   len = (int)strlen(savegamestrings[itemon]);

                keydown = key;

                if (savecharindex < len)
                {
                    for (int j = savecharindex; j < len; j++)
                        savegamestrings[itemon][j] = savegamestrings[itemon][j + 1];

                    caretwait = SKULLANIMCOUNT;
                    showcaret = true;
                }

                break;
            }

            // cancel
            case KEY_ESCAPE:
                if (!keydown)
                {
                    keydown = key;
                    SDL_StopTextInput();
                    savestringenter = false;
                    caretwait = 0;
                    showcaret = true;
                    M_StringCopy(savegamestrings[itemon], saveoldstring, sizeof(savegamestrings[0]));
                    S_StartSound(NULL, sfx_swtchx);
                }

                break;

            // confirm
            case KEY_ENTER:
                if (!keydown)
                {
                    const int   len = (int)strlen(savegamestrings[itemon]);
                    bool        allspaces = true;

                    keydown = key;

                    for (int i = 0; i < len; i++)
                        if (savegamestrings[itemon][i] != ' ')
                            allspaces = false;

                    if (savegamestrings[itemon][0] && !allspaces)
                    {
                        SDL_StopTextInput();
                        M_UpdateSaveGameName(itemon);
                        savestringenter = false;
                        caretwait = SKULLANIMCOUNT;
                        showcaret = true;
                        M_DoSave(itemon);
                        D_FadeScreen(false);

                        if (savegame != itemon + 1)
                        {
                            savegame = itemon + 1;
                            M_SaveCVARs();
                            C_IntegerCVAROutputNoRepeat(stringize(savegame), savegame);
                        }
                    }
                }

                break;

            // move caret left
            case KEY_LEFTARROW:
                if (savecharindex > 0)
                {
                    savecharindex--;
                    caretwait = SKULLANIMCOUNT;
                    showcaret = true;
                }

                break;

            // move caret right
            case KEY_RIGHTARROW:
                if (savecharindex < (int)strlen(savegamestrings[itemon]))
                {
                    savecharindex++;
                    caretwait = SKULLANIMCOUNT;
                    showcaret = true;
                }

                break;

            // move caret to start
            case KEY_HOME:
                if (savecharindex > 0)
                {
                    savecharindex = 0;
                    caretwait = SKULLANIMCOUNT;
                    showcaret = true;
                }

                break;

            // move caret to end
            case KEY_END:
            {
                const int   len = (int)strlen(savegamestrings[itemon]);

                if (savecharindex < len)
                {
                    savecharindex = len;
                    caretwait = SKULLANIMCOUNT;
                    showcaret = true;
                }

                break;
            }
        }

        return true;
    }

    // Take care of any messages that need input
    if (messagetoprint && !keydown)
    {
        const int   ch = (key == KEY_ENTER ? 'y' : tolower(key));

        if (messageneedsinput && key != keyboardmenu && key != keyboardmenu2 && ch != 'y' && ch != 'n'
            && key != KEY_BACKSPACE && !(SDL_GetModState() & (KMOD_ALT | KMOD_CTRL)) && key != functionkey)
        {
            functionkey = 0;
            return false;
        }

        keydown = key;
        menuactive = messagelastmenuactive;
        messagetoprint = false;

        if (messageroutine)
            messageroutine(ch);

        functionkey = 0;

        if (endinggame)
            endinggame = false;
        else
        {
            S_StartSound(NULL, (currentmenu == &HelpDef ? sfx_pistol : sfx_swtchx));
            D_FadeScreen(false);
        }

        return true;
    }

    if (key == KEY_ENTER && *prevmessage && viewplayer->health > 0 && !consoleactive
        && !helpscreen && messages && (r_hud || r_screensize < r_screensize_max)
        && !keydown2 && !IsControlBound(keyboardcontrol, KEY_ENTER))
    {
        keydown2 = key;

        if (message_on)
            message_counter = 4;
        else
        {
            char    buffer[133] = "";

            HU_SetPlayerMessage(prevmessage, false, false);
            M_snprintf(buffer, sizeof(buffer), s_SECRETMESSAGE,
                (M_StringCompare(playername, playername_default) ? "You" : playername));

            if (M_StringCompare(prevmessage, buffer))
                message_secret = true;
        }
    }
    else if (key == '-')
    {
        // Screen size down
        if (consoleactive || paused || splashscreen || automapactive || fadecount)
            return false;

        keydown = key;

        if (viewactive && !menuactive)
            M_SizeDisplay(0);
        else if (vid_widescreen)
        {
            vid_widescreen = false;
            C_StringCVAROutput(stringize(vid_widescreen), "off");

            if (r_screensize == r_screensize_max)
            {
                r_screensize = r_screensize_default;
                C_IntegerCVAROutput(stringize(r_screensize), r_screensize);

                if (r_hud)
                {
                    r_hud = false;
                    C_StringCVAROutput(stringize(r_hud), "off");
                }
            }

            R_SetViewSize(menuactive && viewactive ? r_screensize_max : r_screensize_default);
            I_RestartGraphics(false);
            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();

            pagetic = PAGETICS;
        }

        return false;
    }
    else if (key == '=')
    {
        // Screen size up
        if (consoleactive || paused || splashscreen || automapactive || fadecount)
            return false;

        keydown = key;

        if (viewactive && !menuactive)
            M_SizeDisplay(1);
        else if (!vid_widescreen && !nowidescreen)
        {
            vid_widescreen = true;
            C_StringCVAROutput(stringize(vid_widescreen), "on");

            if (r_screensize < r_screensize_default)
            {
                r_screensize = r_screensize_default;
                C_IntegerCVAROutput(stringize(r_screensize), r_screensize);
            }

            R_SetViewSize(menuactive && viewactive ? r_screensize_max : r_screensize_default);
            I_RestartGraphics(false);
            S_StartSound(NULL, sfx_stnmov);
            M_SaveCVARs();

            pagetic = PAGETICS;
        }

        return false;
    }
    else if ((!menuactive || functionkey) && !paused && !splashscreen)
    {
        // Save
        if (key == KEY_F2 && (!functionkey || functionkey == KEY_F2) && (viewactive || automapactive)
            && !keydown && viewplayer->health > 0)
        {
            keydown = key;

            if (functionkey == KEY_F2)
            {
                functionkey = 0;
                currentmenu->laston = itemon;
                M_CloseMenu();
                S_StartSound(NULL, sfx_swtchx);
            }
            else
            {
                functionkey = KEY_F2;
                C_HideConsoleFast();
                M_OpenMainMenu();
                itemon = currentmenu->laston;
                S_StartSound(NULL, sfx_swtchn);
                M_SaveGame(0);
            }

            return true;
        }

        // Load
        else if (key == KEY_F3 && (!functionkey || functionkey == KEY_F3) && savegames && !keydown)
        {
            keydown = key;

            if (functionkey == KEY_F3)
            {
                functionkey = 0;
                currentmenu->laston = itemon;
                M_CloseMenu();
                S_StartSound(NULL, sfx_swtchx);
            }
            else
            {
                functionkey = KEY_F3;
                C_HideConsoleFast();
                M_OpenMainMenu();
                itemon = currentmenu->laston;
                S_StartSound(NULL, sfx_swtchn);
                M_LoadGame(0);
            }

            return true;
        }

        // Sound Volume
        else if (key == KEY_F4 && (!functionkey || functionkey == KEY_F4) && !keydown)
        {
            keydown = key;

            if (functionkey == KEY_F4)
            {
                functionkey = 0;
                currentmenu->laston = itemon;
                M_CloseMenu();
                S_StartSound(NULL, sfx_swtchx);
            }
            else
            {
                functionkey = KEY_F4;
                C_HideConsoleFast();
                M_OpenMainMenu();
                currentmenu = &SoundDef;
                itemon = currentmenu->laston;
                S_StartSound(NULL, sfx_swtchn);
            }

            return true;
        }

        // Quicksave
        else if (key == KEY_F6 && (!functionkey || functionkey == KEY_F6) && (viewactive || automapactive)
            && !keydown && viewplayer->health > 0)
        {
            keydown = key;

            if (quicksaveslot >= 0)
                functionkey = KEY_F6;

            C_HideConsoleFast();
            M_QuickSave();
            return true;
        }

        // End game
        else if (key == KEY_F7 && !functionkey && (viewactive || automapactive) && !keydown)
        {
            keydown = key;
            functionkey = KEY_F7;
            C_HideConsoleFast();
            M_OpenMainMenu();
            S_StartSound(NULL, sfx_swtchn);
            M_EndGame(0);
            return true;
        }

        // Toggle messages
        else if (key == KEY_F8 && !functionkey && (viewactive || automapactive) && !keydown)
        {
            keydown = key;
            functionkey = KEY_F8;
            M_ChangeMessages(0);
            functionkey = 0;
            S_StartSound(NULL, sfx_swtchn);
            return false;
        }

        // Quickload
        else if (key == KEY_F9 && !functionkey && (viewactive || automapactive) && savegames && !keydown)
        {
            keydown = key;
            functionkey = KEY_F9;
            C_HideConsoleFast();
            M_OpenMainMenu();
            M_QuickLoad();
            return true;
        }

        // Quit DOOM Retro
        else if (key == KEY_F10 && !keydown)
        {
            keydown = key;
            functionkey = KEY_F10;
            C_HideConsoleFast();
            M_OpenMainMenu();
            S_StartSound(NULL, sfx_swtchn);
            M_QuitDOOM(0);
            return true;
        }
    }

    // Help key
    if (key == KEY_F1 && (!functionkey || functionkey == KEY_F1) && !keydown && !splashscreen)
    {
        keydown = key;

        if (helpscreen)
        {
            functionkey = 0;
            M_CloseMenu();
            S_StartSound(NULL, sfx_swtchx);
            D_FadeScreen(false);
            R_SetViewSize(r_screensize);
        }
        else
        {
            C_HideConsoleFast();
            D_FadeScreen(false);
            M_ShowHelp(0);
        }

        return false;
    }

    // Toggle graphic detail
    if (key == KEY_F5 && !functionkey && gamestate == GS_LEVEL && !menuactive && !keydown)
    {
        keydown = key;
        functionkey = KEY_F5;
        M_ChangeDetail(0);
        functionkey = 0;
        S_StartSound(NULL, sfx_swtchn);
        return false;
    }

    // gamma toggle
    if (key == KEY_F11)
    {
        M_ChangeGamma(SDL_GetModState() & KMOD_SHIFT);
        return false;
    }

    // screenshot
    if ((key == keyboardscreenshot || key == keyboardscreenshot2)
        && (key == KEY_PRINTSCREEN || gamestate == GS_LEVEL))
    {
        G_ScreenShot();
        return false;
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if ((key == keyboardmenu || key == keyboardmenu2) && !keydown && !splashscreen && !consoleactive)
        {
            keydown = key;

            if (paused)
            {
                paused = false;
                S_ResumeMusic();
                S_StartSound(NULL, sfx_swtchx);
                I_SetPalette(&PLAYPAL[st_palette * 768]);
            }
            else
            {
                M_OpenMainMenu();
                S_StartSound(NULL, sfx_swtchn);
            }
        }

        return false;
    }

    if (!paused)
    {
        if (key == KEY_DOWNARROW && keywait < I_GetTime() && !helpscreen)
        {
            // Move down to next item
            if (currentmenu == &LoadDef)
            {
                const int   old = itemon;

                do
                {
                    if (++itemon > currentmenu->numitems - 1)
                        itemon = 0;
                } while (M_StringCompare(savegamestrings[itemon], s_EMPTYSTRING));

                if (itemon != old)
                    S_StartSound(NULL,  sfx_pstop);

                SaveDef.laston = itemon;
            }
            else
            {
                do
                {
                    if (++itemon > currentmenu->numitems - 1)
                        itemon = 0;

                    if (currentmenu == &MainDef)
                    {
                        if (itemon == load_game && !savegames)
                            itemon++;

                        if (itemon == save_game && (gamestate != GS_LEVEL || viewplayer->health <= 0))
                            itemon++;
                    }
                    else if (currentmenu == &EpiDef)
                    {
                        if (itemon == ep5 && !sigil && sigil2)
                            itemon++;

                        episode = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(episode), episode);
                    }
                    else if (currentmenu == &ExpDef)
                    {
                        expansion = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(expansion), expansion);
                    }
                    else if (currentmenu == &NewDef)
                    {
                        skilllevel = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(skilllevel), skilllevel);
                    }
                    else if (currentmenu == &OptionsDef && itemon == endgame
                        && gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
                        itemon++;

                    if (currentmenu->menuitems[itemon].status != -1)
                        S_StartSound(NULL, sfx_pstop);
                } while (currentmenu->menuitems[itemon].status == -1);
            }

            currentmenu->change = true;

            if (currentmenu == &ExpDef)
            {
                if (gamestate != GS_LEVEL)
                    gamemission = (expansion == 2 && nerve ? pack_nerve : doom2);
            }
            else if (currentmenu == &SaveDef)
                LoadDef.laston = itemon;

            keywait = I_GetTime() + 2;
            return false;
        }
        else if (key == KEY_UPARROW && keywait < I_GetTime() && !helpscreen)
        {
            // Move back up to previous item
            if (currentmenu == &LoadDef)
            {
                const int   old = itemon;

                do
                {
                    if (!itemon--)
                        itemon = currentmenu->numitems - 1;
                } while (M_StringCompare(savegamestrings[itemon], s_EMPTYSTRING));

                if (itemon != old)
                    S_StartSound(NULL, sfx_pstop);

                SaveDef.laston = itemon;
            }
            else
            {
                do
                {
                    if (!itemon--)
                        itemon = currentmenu->numitems - 1;

                    if (currentmenu == &MainDef)
                    {
                        if (itemon == save_game && (gamestate != GS_LEVEL || viewplayer->health <= 0))
                            itemon--;

                        if (itemon == load_game && !savegames)
                            itemon--;
                    }
                    else if (currentmenu == &EpiDef)
                    {
                        if (itemon == ep5 && !sigil && sigil2)
                            itemon--;

                        episode = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(episode), episode);
                    }
                    else if (currentmenu == &ExpDef)
                    {
                        expansion = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(expansion), expansion);
                    }
                    else if (currentmenu == &NewDef)
                    {
                        skilllevel = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(skilllevel), skilllevel);
                    }
                    else if (currentmenu == &OptionsDef && itemon == endgame
                        && gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
                        itemon = currentmenu->numitems - 1;

                    if (currentmenu->menuitems[itemon].status != -1)
                        S_StartSound(NULL, sfx_pstop);
                } while (currentmenu->menuitems[itemon].status == -1);
            }

            currentmenu->change = true;

            if (currentmenu == &ExpDef)
            {
                if (gamestate != GS_LEVEL)
                    gamemission = (expansion == 2 && nerve ? pack_nerve : doom2);
            }
            else if (currentmenu == &SaveDef)
                LoadDef.laston = itemon;

            keywait = I_GetTime() + 2;
            return false;
        }

        else if (key == KEY_LEFTARROW && !helpscreen)
        {
            // Slide slider left
            if (currentmenu->menuitems[itemon].status == 2 && currentmenu->menuitems[itemon].routine)
                currentmenu->menuitems[itemon].routine(0);
            else if (currentmenu->menuitems[itemon].status == -1 && currentmenu->menuitems[itemon - 1].routine)
                currentmenu->menuitems[itemon - 1].routine(0);

            return false;
        }

        else if (key == KEY_RIGHTARROW && !helpscreen)
        {
            // Slide slider right
            if (currentmenu->menuitems[itemon].status == 2 && currentmenu->menuitems[itemon].routine)
                currentmenu->menuitems[itemon].routine(1);
            else if (currentmenu->menuitems[itemon].status == -1 && currentmenu->menuitems[itemon - 1].routine)
                currentmenu->menuitems[itemon - 1].routine(1);

            return false;
        }

        else if (key == KEY_ENTER && keywait < I_GetTime() && !keydown)
        {
            // Activate menu item
            keydown = key;

            if (helpscreen)
            {
                functionkey = 0;
                M_CloseMenu();
                S_StartSound(NULL, sfx_swtchx);
                D_FadeScreen(false);
                R_SetViewSize(r_screensize);
                return true;
            }

            if (currentmenu == &EpiDef)
            {
                episode = itemon + 1;
                M_SaveCVARs();
                C_IntegerCVAROutputNoRepeat(stringize(episode), episode);
            }
            else if (currentmenu == &ExpDef)
            {
                expansion = itemon + 1;
                M_SaveCVARs();
                C_IntegerCVAROutputNoRepeat(stringize(expansion), expansion);
            }
            else if (currentmenu == &NewDef)
            {
                skilllevel = itemon + 1;
                M_SaveCVARs();
                C_IntegerCVAROutputNoRepeat(stringize(skilllevel), skilllevel);
            }
            else if (currentmenu == &LoadDef && savegame != itemon + 1)
            {
                savegame = itemon + 1;
                M_SaveCVARs();
                C_IntegerCVAROutputNoRepeat(stringize(savegame), savegame);
            }

            if (currentmenu->menuitems[itemon].routine && currentmenu->menuitems[itemon].status)
            {
                if (gamemode != shareware || currentmenu != &EpiDef)
                    currentmenu->laston = itemon;

                if (currentmenu->menuitems[itemon].status == 2)
                    currentmenu->menuitems[itemon].routine(1);
                else
                {
                    if (gamestate != GS_LEVEL && gamestate != GS_INTERMISSION
                        && ((currentmenu == &MainDef && itemon == save_game)
                            || (currentmenu == &OptionsDef && itemon == endgame)))
                        return true;

                    if (currentmenu != &LoadDef)
                    {
                        if (currentmenu != &NewDef || itemon == nightmare)
                            S_StartSound(NULL, sfx_pistol);

                        if (currentmenu != &NewDef && currentmenu != &SaveDef
                            && (currentmenu != &OptionsDef || itemon == soundvol))
                            D_FadeScreen(false);
                    }

                    currentmenu->menuitems[itemon].routine(itemon);
                }
            }

            keywait = I_GetTime() + 5;
            return (currentmenu == &LoadDef || currentmenu == &SaveDef || currentmenu == &NewDef);
        }

        else if ((key == keyboardmenu || key == keyboardmenu2 || key == KEY_BACKSPACE) && !keydown)
        {
            // Deactivate menu or go back to previous menu
            keydown = key;

            if (gamemode != shareware || currentmenu != &EpiDef)
                currentmenu->laston = itemon;

            if (currentmenu->prevmenu && !functionkey)
            {
                currentmenu = currentmenu->prevmenu;
                itemon = currentmenu->laston;
                S_StartSound(NULL, sfx_swtchn);
            }
            else
            {
                functionkey = 0;
                M_CloseMenu();
                S_StartSound(NULL, sfx_swtchx);
                controllerbuttons = 0;
                ev->data1 = 0;
                firstevent = true;
            }

            D_FadeScreen(false);

            if (helpscreen)
                R_SetViewSize(r_screensize);

            return true;
        }

        else if (key == KEY_DELETE && !keydown && (currentmenu == &LoadDef || currentmenu == &SaveDef))
        {
            // Delete a savegame
            keydown = key;

            if (LoadGameMenu[itemon].status)
            {
                M_DeleteSaveGame();
                M_ReadSaveStrings();

                if (savegame != itemon + 1)
                {
                    savegame = itemon + 1;
                    M_SaveCVARs();
                    C_IntegerCVAROutputNoRepeat(stringize(savegame), savegame);
                }

                if (!savegames)
                {
                    currentmenu = &MainDef;
                    itemon = new_game;
                }

                S_StartSound(NULL, sfx_swtchn);
                return true;
            }

            return false;
        }

        // Keyboard shortcut?
        else if (key && !(SDL_GetModState() & (KMOD_ALT | KMOD_CTRL)) && !helpscreen)
        {
            for (int i = itemon + 1; i < currentmenu->numitems; i++)
                if (((currentmenu == &LoadDef || currentmenu == &SaveDef)
                        && (key == '1' + i || toupper(savegamestrings[i][0]) == toupper(key)))
                    || (currentmenu->menuitems[i].text
                        && toupper(*currentmenu->menuitems[i].text[0]) == toupper(key)))
                {
                    if (currentmenu == &MainDef)
                    {
                        if ((i == load_game && !savegames) || (i == save_game
                            && (gamestate != GS_LEVEL || viewplayer->health <= 0)))
                            continue;
                    }
                    else if (currentmenu == &EpiDef)
                    {
                        if (i == ep5 && !sigil && sigil2)
                            continue;
                    }
                    else if (currentmenu == &OptionsDef && i == endgame
                        && gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
                        return true;
                    else if (currentmenu == &LoadDef && M_StringCompare(savegamestrings[i], s_EMPTYSTRING))
                        return true;

                    if (itemon != i)
                        S_StartSound(NULL, sfx_pstop);

                    itemon = i;
                    currentmenu->change = true;

                    if (currentmenu == &EpiDef)
                    {
                        episode = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(episode), episode);
                    }
                    else if (currentmenu == &ExpDef)
                    {
                        if (gamestate != GS_LEVEL)
                            gamemission = (expansion == 2 && nerve ? pack_nerve : doom2);

                        expansion = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(expansion), expansion);
                    }
                    else if (currentmenu == &NewDef)
                    {
                        skilllevel = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(skilllevel), skilllevel);
                    }
                    else if (currentmenu == &SaveDef)
                        LoadDef.laston = itemon;
                    else if (currentmenu == &LoadDef)
                        SaveDef.laston = itemon;

                    return false;
                }

            for (int i = 0; i <= itemon; i++)
                if (((currentmenu == &LoadDef || currentmenu == &SaveDef)
                        && (key == '1' + i || toupper(savegamestrings[i][0]) == toupper(key)))
                    || (currentmenu->menuitems[i].text
                        && toupper(*currentmenu->menuitems[i].text[0]) == toupper(key)))
                {
                    if (currentmenu == &MainDef)
                    {
                        if ((i == load_game && !savegames) || (i == save_game
                            && (gamestate != GS_LEVEL || viewplayer->health <= 0)))
                            continue;
                    }
                    else if (currentmenu == &EpiDef)
                    {
                        if (i == ep5 && !sigil && sigil2)
                            continue;
                    }
                    else if (currentmenu == &OptionsDef && i == endgame
                        && gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
                        return true;
                    else if (currentmenu == &LoadDef && M_StringCompare(savegamestrings[i], s_EMPTYSTRING))
                        return true;

                    if (itemon != i)
                        S_StartSound(NULL, sfx_pstop);

                    itemon = i;
                    currentmenu->change = true;

                    if (currentmenu == &EpiDef)
                    {
                        episode = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(episode), episode);
                    }
                    else if (currentmenu == &ExpDef)
                    {
                        if (gamestate != GS_LEVEL)
                            gamemission = (expansion == 2 && nerve ? pack_nerve : doom2);

                        expansion = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(expansion), expansion);
                    }
                    else if (currentmenu == &NewDef)
                    {
                        skilllevel = itemon + 1;
                        M_SaveCVARs();
                        C_IntegerCVAROutputNoRepeat(stringize(skilllevel), skilllevel);
                    }
                    else if (currentmenu == &SaveDef)
                        LoadDef.laston = itemon;
                    else if (currentmenu == &LoadDef)
                        SaveDef.laston = itemon;

                    return false;
                }
        }
    }

    return false;
}

//
// M_OpenMainMenu
//
void M_OpenMainMenu(void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;

    menuactive = true;
    currentmenu = &MainDef;
    itemon = currentmenu->laston;

    if (joy_rumble_barrels || joy_rumble_damage || joy_rumble_fall || joy_rumble_pickup || joy_rumble_weapons)
    {
        restoredrumblestrength = idlechainsawrumblestrength;
        idlechainsawrumblestrength = 0;
        I_StopControllerRumble();
    }

    if (gamestate == GS_LEVEL)
    {
        viewplayer->fixedcolormap = 0;
        I_SetPalette(PLAYPAL);
        I_UpdateBlitFunc(false);

        if (vid_motionblur)
            I_SetMotionBlur(0);

        S_StopSounds();

        if (menuspin)
        {
            playerangle = viewplayer->mo->angle;
            playerviewz = viewplayer->viewz;
            menuspinspeed = 0;

            playerlookdir = viewplayer->lookdir;

            if (!helpscreen)
                viewplayer->lookdir = 0;
        }

        R_SetViewSize(r_screensize_max);

        if (menuspin && !helpscreen)
            viewplayer->viewz = viewplayer->mo->floorz + MENUVIEWHEIGHT;

        I_RestoreMousePointerPosition();
    }

    S_LowerMusicVolume();
    D_FadeScreen(false);
}

//
// M_DrawNightmare
//
static void M_DrawNightmare(bool highlight)
{
    for (int y = 0; y < 20; y++)
        for (int x = 0; x < 124; x++)
            V_DrawPixel(NewDef.x + x, NewDef.y + OFFSET + 16 * nightmare + y, (int)nmare[y * 124 + x], highlight, true);

    currentmenu->menuitems[nightmare].x = NewDef.x + MAXWIDESCREENDELTA;
    currentmenu->menuitems[nightmare].y = NewDef.y + OFFSET + 16 * nightmare;
    currentmenu->menuitems[nightmare].width = 124;
    currentmenu->menuitems[nightmare].height = 20;
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
    static short    x, y;

    // Center string and print it.
    if (messagetoprint)
    {
        char    string[255];
        int     start = 0;

        M_DrawMenuBackground();

        y = (VANILLAHEIGHT - M_StringHeight(messagestring)) / 2;

        while (messagestring[start] != '\0')
        {
            const int   len = (int)strlen(messagestring + start);
            bool        foundnewline = false;

            for (int i = 0; i < len; i++)
                if (messagestring[start + i] == '\n')
                {
                    M_StringCopy(string, messagestring + start, sizeof(string));

                    if (i < sizeof(string))
                        string[i] = '\0';

                    foundnewline = true;
                    start += i + 1;
                    break;
                }

            if (!foundnewline)
            {
                M_StringCopy(string, messagestring + start, sizeof(string));
                start += (int)strlen(string);
            }

            if (*string)
            {
                M_WriteText((VANILLAWIDTH - M_StringWidth(string)) / 2, y, string, false, true, '\0');
                y += (STCFNxxx ? SHORT(hu_font[0]->height) + 1 : 8) + 1;
            }
            else
                y += 3;
        }

        return;
    }

    if (!menuactive)
    {
        helpscreen = false;
        return;
    }

    if (currentmenu->routine)
        currentmenu->routine();         // call draw routine

    // DRAW MENU
    x = currentmenu->x;
    y = currentmenu->y;

    if (currentmenu != &HelpDef)
    {
        // DRAW SKULL
        const char  *skullname[] = { "M_SKULL1", "M_SKULL2" };
        patch_t     *skullpatch = W_CacheLumpName(skullname[whichskull]);

        if (currentmenu == &LoadDef || currentmenu == &SaveDef)
        {
            if (currentmenu == &LoadDef)
            {
                const int   old = itemon;

                while (M_StringCompare(savegamestrings[itemon], s_EMPTYSTRING))
                    itemon = (itemon == load1 ? currentmenu->numitems - 1 : itemon - 1);

                if (itemon != old)
                    SaveDef.laston = itemon;
            }

            if (M_SKULL1)
                M_DrawPatchWithShadow(x - 43, y + itemon * LINEHEIGHT - (chex ? 7 : 8) + OFFSET, skullpatch, true);
            else
                M_DrawPatchWithShadow(x - 37, y + itemon * LINEHEIGHT - 7 + OFFSET, skullpatch, true);
        }
        else
        {
            const int   max = currentmenu->numitems;
            int         item = itemon;
            int         widest = 0;
            int         yy;

            if (currentmenu == &OptionsDef)
            {
                if (item == endgame && gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
                    item++;
                else if (item == option_empty1 || item == option_empty2)
                    item--;
            }
            else if (currentmenu == &SoundDef && (item == sound_empty1 || item == sound_empty2))
                item--;

            if (M_SKULL1)
            {
                yy = y + item * (LINEHEIGHT - 1) - (chex || REKKR ? 4 : 5);

                if (currentmenu != &MainDef || titleheight < VANILLAHEIGHT)
                    yy += OFFSET;

                M_DrawPatchWithShadow((REKKR && currentmenu == &EpiDef ? x : x - 30), yy, skullpatch, true);
            }
            else
            {
                yy = y + item * (LINEHEIGHT - 1) - 3;

                if (currentmenu != &MainDef || titleheight < VANILLAHEIGHT)
                    yy += OFFSET;

                M_DrawPatchWithShadow(x - 26, yy, skullpatch, true);
            }

            memset(tempscreen, PINK, MAXSCREENAREA);

            for (int i = 0; i < max; i++)
            {
                bool    highlight;

                if (currentmenu == &OptionsDef
                    && ((i == scrnsize && itemon == option_empty1)
                        || (i == mousesens && itemon == option_empty2)))
                    highlight = true;
                else if (currentmenu == &SoundDef
                    && ((i == sfx_vol && itemon == sound_empty1)
                        || (i == music_vol && itemon == sound_empty2)))
                    highlight = true;
                else
                    highlight = (itemon == i);

                if (currentmenu->menuitems[i].routine)
                {
                    const char  *name = currentmenu->menuitems[i].name;
                    char        **text = currentmenu->menuitems[i].text;

                    if (M_StringCompare(name, "M_NMARE"))
                    {
                        if (M_NMARE)
                        {
                            patch_t *patch = W_CacheLumpName(name);

                            M_DrawPatchWithShadow(x, y + OFFSET, patch, highlight);
                            currentmenu->menuitems[i].x = x - SHORT(patch->leftoffset) + MAXWIDESCREENDELTA;
                            currentmenu->menuitems[i].y = y - SHORT(patch->topoffset) + OFFSET;
                            widest = MAX(widest, SHORT(patch->width));
                            currentmenu->menuitems[i].height = SHORT(patch->height);
                        }
                        else
                        {
                            M_DrawNightmare(highlight);
                            widest = MAX(widest, currentmenu->menuitems[nightmare].width);
                        }
                    }
                    else if (M_StringCompare(name, "M_MSENS") && (!M_MSENS || DBIGFONT) && usingcontroller)
                    {
                        M_DrawString(x, y + OFFSET, s_M_CONTROLLERSENSITIVITY, highlight, true);
                        widest = MAX(widest, M_BigStringWidth(s_M_CONTROLLERSENSITIVITY));

                        currentmenu->menuitems[i].x = x + MAXWIDESCREENDELTA;
                        currentmenu->menuitems[i].y = y + OFFSET;
                        currentmenu->menuitems[i].height = LINEHEIGHT - 1;
                    }
                    else if (M_StringCompare(name, "M_EPI6") && sigil2)
                    {
                        patch_t *patch = W_CacheLastLumpName(name);

                        M_DrawPatchWithShadow(x, y + OFFSET, patch, highlight);
                        currentmenu->menuitems[i].x = x - SHORT(patch->leftoffset) + MAXWIDESCREENDELTA;
                        currentmenu->menuitems[i].y = y - SHORT(patch->topoffset) + OFFSET;
                        widest = MAX(widest, SHORT(patch->width));
                        currentmenu->menuitems[i].height = SHORT(patch->height);
                    }
                    else if ((W_CheckNumForName(name) < 0 && **text) || (M_StringCompare(name, "M_EPI5") && sigil2))
                    {
                        int width = M_BigStringWidth(*text);

                        M_DrawString(x, y + OFFSET, *text, highlight, true);
                        currentmenu->menuitems[i].x = x + MAXWIDESCREENDELTA;
                        currentmenu->menuitems[i].y = y + OFFSET;

                        if (currentmenu == &OptionsDef)
                        {
                            if (i == msgs)
                            {
                                width += SPACEWIDTH;

                                if (M_MSGON)
                                {
                                    const patch_t   *on = W_CacheLumpName("M_MSGON");
                                    const patch_t   *off = W_CacheLumpName("M_MSGOFF");

                                    width += SHORT(MAX(on->width, off->width));
                                }
                                else
                                    width += MAX(M_BigStringWidth(s_M_ON), M_BigStringWidth(s_M_OFF));
                            }
                            else if (i == detail)
                            {
                                width += SPACEWIDTH;

                                if (M_GDLOW)
                                {
                                    const patch_t   *high = W_CacheLumpName("M_GDHIGH");
                                    const patch_t   *low = W_CacheLumpName("M_GDLOW");

                                    width += SHORT(MAX(high->width, low->width));
                                }
                                else
                                    width += MAX(M_BigStringWidth(s_M_HIGH), M_BigStringWidth(s_M_LOW));
                            }
                            else
                                width = M_BigStringWidth(*text);
                        }

                        widest = MAX(widest, width);
                        currentmenu->menuitems[i].height = LINEHEIGHT - 1;
                    }
                    else if (W_GetNumLumps(name) > 1 || lumpinfo[W_GetNumForName(name)]->wadfile->type == PWAD)
                    {
                        patch_t *patch = W_CacheLumpName(name);
                        int     width = SHORT(patch->width);

                        if (eviternity)
                            patch->leftoffset = 0;

                        M_DrawPatchWithShadow(x, y + OFFSET, patch, highlight);
                        currentmenu->menuitems[i].x = x - SHORT(patch->leftoffset) + MAXWIDESCREENDELTA;
                        currentmenu->menuitems[i].y = y - SHORT(patch->topoffset) + OFFSET;

                        if (currentmenu == &OptionsDef)
                        {
                            if (i == msgs)
                            {
                                width += SPACEWIDTH;

                                if (M_MSGON)
                                {
                                    const patch_t   *on = W_CacheLumpName("M_MSGON");
                                    const patch_t   *off = W_CacheLumpName("M_MSGOFF");

                                    width += SHORT(MAX(on->width, off->width));

                                    if (REKKR)
                                        width -= 26;
                                }
                                else
                                    width += MAX(M_BigStringWidth(s_M_ON), M_BigStringWidth(s_M_OFF));
                            }
                            else if (i == detail)
                            {
                                width += SPACEWIDTH;

                                if (M_GDLOW)
                                {
                                    const patch_t   *high = W_CacheLumpName("M_GDHIGH");
                                    const patch_t   *low = W_CacheLumpName("M_GDLOW");

                                    width += SHORT(MAX(high->width, low->width));

                                    if (REKKR)
                                        width -= 26;
                                }
                                else
                                    width += MAX(M_BigStringWidth(s_M_HIGH), M_BigStringWidth(s_M_LOW));
                            }
                            else
                                width = M_BigStringWidth(*text);
                        }

                        widest = MAX(widest, width);
                        currentmenu->menuitems[i].height = SHORT(patch->height);
                    }
                    else if (**text)
                    {
                        int width = M_BigStringWidth(*text) + 1;

                        yy = y;

                        if (currentmenu != &MainDef || titleheight < VANILLAHEIGHT)
                            yy += OFFSET;

                        M_DrawString(x, yy, *text, highlight, true);
                        currentmenu->menuitems[i].x = x + MAXWIDESCREENDELTA;
                        currentmenu->menuitems[i].y = yy;

                        if (currentmenu == &OptionsDef)
                        {
                            if (i == msgs)
                            {
                                width += SPACEWIDTH;

                                if (M_MSGON)
                                {
                                    const patch_t   *on = W_CacheLumpName("M_MSGON");
                                    const patch_t   *off = W_CacheLumpName("M_MSGOFF");

                                    width += SHORT(MAX(on->width, off->width));
                                }
                                else
                                    width += MAX(M_BigStringWidth(s_M_ON), M_BigStringWidth(s_M_OFF));
                            }
                            else if (i == detail)
                            {
                                width += SPACEWIDTH;

                                if (M_GDLOW)
                                {
                                    const patch_t   *high = W_CacheLumpName("M_GDHIGH");
                                    const patch_t   *low = W_CacheLumpName("M_GDLOW");

                                    width += SHORT(MAX(high->width, low->width));
                                }
                                else
                                    width += MAX(M_BigStringWidth(s_M_HIGH), M_BigStringWidth(s_M_LOW));
                            }
                            else
                                width = M_BigStringWidth(*text);
                        }

                        widest = MAX(widest, width);
                        currentmenu->menuitems[i].height = LINEHEIGHT - 1;
                    }
                }
                else if (currentmenu->menuitems[i].status == -1)
                {
                    currentmenu->menuitems[i].x = x + MAXWIDESCREENDELTA;
                    currentmenu->menuitems[i].y = y + OFFSET;
                    currentmenu->menuitems[i].height = LINEHEIGHT - 1;
                }

                y += LINEHEIGHT - 1;
            }

            for (int i = 0; i < MAXSCREENAREA; i++)
                if (tempscreen[i] == nearestblack)
                    screens[0][i] = black40[screens[0][i]];
                else if (tempscreen[i] != PINK)
                    screens[0][i] = tempscreen[i];

            for (int i = 0; i < max; i++)
                currentmenu->menuitems[i].width = widest;
        }
    }
}

//
// M_CloseMenu
//
void M_CloseMenu(void)
{
    if (!menuactive)
        return;

    menuactive = false;
    blurtic = -1;
    menuspindirection = ((M_BigRandom() & 1) ? 1 : -1);

    if (joy_rumble_damage || joy_rumble_barrels || joy_rumble_weapons)
    {
        idlechainsawrumblestrength = restoredrumblestrength;
        I_ControllerRumble(idlechainsawrumblestrength, idlechainsawrumblestrength);
    }

    if (gamestate == GS_LEVEL)
    {
        I_SetPalette(&PLAYPAL[st_palette * 768]);

        if (menuspin)
        {
            viewplayer->mo->angle = playerangle;
            viewplayer->lookdir = playerlookdir;
            viewplayer->viewz = playerviewz;
        }

        if (!helpscreen)
            R_SetViewSize(r_screensize);

        AM_SetAutomapSize(r_screensize);

        if (reopenautomap)
        {
            reopenautomap = false;
            AM_Start(true);
            viewactive = false;
        }
    }

    S_RestoreMusicVolume();

    I_SaveMousePointerPosition();
    usingmouse = false;
}

//
// M_SetupNextMenu
//
static void M_SetupNextMenu(menu_t *menudef)
{
    currentmenu = menudef;
    itemon = currentmenu->laston;
    whichskull = 0;
}

//
// M_Ticker
//
void M_Ticker(void)
{
    if (windowfocused)
    {
        if (savestringenter)
        {
            whichskull = 0;

            if (--caretwait <= 0)
            {
                caretwait = SKULLANIMCOUNT;
                showcaret = !showcaret;
            }
        }
        else if (--skullanimcounter <= 0)
        {
            whichskull ^= 1;
            skullanimcounter = SKULLANIMCOUNT;
        }
    }
}

//
// M_Init
//
void M_Init(void)
{
    M_BigSeed((unsigned int)time(NULL));

    currentmenu = &MainDef;
    menuactive = false;
    itemon = currentmenu->laston;
    skullanimcounter = SKULLANIMCOUNT;
    caretwait = SKULLANIMCOUNT;
    messagetoprint = false;
    messagestring = NULL;
    messagelastmenuactive = false;
    quicksaveslot = -1;
    menuspindirection = ((M_BigRandom() & 1) ? 1 : -1);
    menuborder = W_CacheLastLumpName("DRBORDER");
    titleheight = SHORT(((patch_t *)W_CacheLumpName("M_DOOM"))->height);

    caretcolor = FindBrightDominantColor(W_CacheLumpName("STCFN065"));

    if (autostart)
    {
        episode = startepisode;
        skilllevel = startskill + 1;
    }

    if (gamemode != shareware)
        EpiDef.laston = episode - 1;

    ExpDef.laston = expansion - 1;
    NewDef.laston = skilllevel - 1;
    SaveDef.laston = LoadDef.laston = savegame - 1;

    if (!*savegamestrings[SaveDef.laston])
        while (SaveDef.laston && !*savegamestrings[SaveDef.laston])
            SaveDef.laston--;

    OptionsDef.laston = msgs;

    if (W_CheckNumForName("M_CONSOL") < 0 && W_GetNumLumps("M_SVOL") > 1 && M_OPTTTL && !DBIGFONT)
    {
        OptionsDef.numitems--;
        OptionsDef.y = 37;
    }

    M_ReadSaveStrings();

    if (chex)
    {
        MainDef.x += 20;
        NewDef.prevmenu = &MainDef;
    }
    else if (gamemode == commercial)
        NewDef.prevmenu = (nerve ? &ExpDef : &MainDef);
    else if (gamemode == registered)
        EpiDef.numitems = 3;
    else if (gamemode == retail && sigil2)
        EpiDef.numitems = 6;
    else if (gamemode == retail && sigil)
        EpiDef.numitems = 5;
    else
        EpiDef.numitems = 4;

    if (EpiDef.laston >= EpiDef.numitems)
    {
        EpiDef.laston = 0;
        episode = 1;
        M_SaveCVARs();
    }
    else if (EpiDef.laston == ep5 && !sigil && sigil2)
    {
        EpiDef.laston = 5;
        episode = 6;
        M_SaveCVARs();
    }

    if (M_StringCompare(s_EMPTYSTRING, "null data"))
        s_EMPTYSTRING = "-";

    if (DBIGFONT)
    {
        int         lump = W_CheckNumForName("DBIGFONT");
        wadfile_t   *wadfile = lumpinfo[lump]->wadfile;

        if (M_LoadFON2(W_CacheLumpNum(lump), W_LumpLength(lump)))
            C_Output("The " BOLD("DBIGFONT") " lump in the %s " BOLD("%s") " is being used in the menu.",
                (wadfile->type == IWAD ? "IWAD" : "PWAD"), wadfile->path);
    }
}
