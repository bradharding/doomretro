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

#include <ctype.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_saveg.h"
#include "s_sound.h"
#include "v_data.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#define LINEHEIGHT      17
#define OFFSET          (vid_widescreen ? 0 : 17)

//
// defaulted values
//
int             m_sensitivity = m_sensitivity_default;
int             gp_sensitivity = gp_sensitivity_default;

// Show messages has default, false = off, true = on
dboolean        messages = messages_default;

int             r_detail = r_detail_default;

int             r_screensize = r_screensize_default;

// -1 = no quicksave slot picked!
int             quickSaveSlot;

// true = message to be printed
dboolean        messageToPrint;
// ...and here is the message string!
char            *messageString;

int             messageLastMenuActive;

// timed message = no input from user
dboolean        messageNeedsInput;

void (*messageRoutine)(int response);

// we are going to be entering a savegame string
int             saveStringEnter;
int             saveSlot;               // which slot to save in
int             saveCharIndex;          // which char we're editing

// old save description before edit
char            saveOldString[SAVESTRINGSIZE];

dboolean        inhelpscreens;
dboolean        menuactive;
dboolean        savegames;
dboolean        startingnewgame;

char            savegamestrings[10][SAVESTRINGSIZE];

char            endstring[160];

short           itemOn;                 // menu item skull is on
short           skullAnimCounter;       // skull animation counter
short           whichSkull;             // which skull to draw

int             episodeselected = episodeselected_default;
int             expansionselected = expansionselected_default;
int             savegameselected = savegameselected_default;
int             skilllevelselected = skilllevelselected_default;

static int      functionkey;

static dboolean usinggamepad;

// graphic name of skulls
char            *skullName[2] = { "M_SKULL1", "M_SKULL2" };

// current menudef
menu_t          *currentMenu;

byte            *tempscreen1;
byte            *tempscreen2;
byte            *blurscreen1;
byte            *blurscreen2;

dboolean        blurred;
dboolean        blurred2;
dboolean        blurredmap;

extern patch_t  *hu_font[HU_FONTSIZE];
extern dboolean message_dontfuckwithme;

extern int      st_palette;

extern dboolean wipe;
extern dboolean r_hud;

extern dboolean splashscreen;

extern dboolean skipaction;
extern dboolean skippsprinterp;

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_Expansion(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_SizeDisplay(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawExpansion(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x, int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x, int y, int thermWidth, float thermDot, float factor, int offset);
void M_WriteText(int x, int y, char *string, dboolean shadow);
int M_StringWidth(char *string);
int M_StringHeight(char *string);
void M_StartMessage(char *string, void *routine, dboolean input);
void M_ClearMenus(void);

//
// DOOM MENU
//

typedef enum
{
    new_game,
    options,
    load_game,
    save_game,
    quit_doom,
    main_end
} main_t;

menuitem_t MainMenu[] =
{
    { 1, "M_NGAME",  M_NewGame,  &s_M_NEWGAME  },
    { 1, "M_OPTION", M_Options,  &s_M_OPTIONS  },
    { 1, "M_LOADG",  M_LoadGame, &s_M_LOADGAME },
    { 1, "M_SAVEG",  M_SaveGame, &s_M_SAVEGAME },
    { 1, "M_QUITG",  M_QuitDOOM, &s_M_QUITGAME }
};

menu_t MainDef =
{
    5,                  // # of menu items
    NULL,               // previous menu
    MainMenu,           // menuitem_t ->
    M_DrawMainMenu,     // drawing routine ->
    98, 77,             // x, y
    new_game            // lastOn
};

//
// EPISODE SELECT
//

typedef enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep_end
} episodes_t;

menuitem_t EpisodeMenu[] =
{
    { 1, "M_EPI1", M_Episode, &s_M_EPISODE1 },
    { 1, "M_EPI2", M_Episode, &s_M_EPISODE2 },
    { 1, "M_EPI3", M_Episode, &s_M_EPISODE3 },
    { 1, "M_EPI4", M_Episode, &s_M_EPISODE4 }
};

menu_t EpiDef =
{
    ep_end,             // # of menu items
    &MainDef,           // previous menu
    EpisodeMenu,        // menuitem_t ->
    M_DrawEpisode,      // drawing routine ->
    39, 69,             // x, y
    ep1                 // lastOn
};

//
// EXPANSION SELECT
//

typedef enum
{
    ex1,
    ex2,
    ex_end
} expansions_t;

menuitem_t ExpansionMenu[] =
{
    { 1, "M_EPI1", M_Expansion, &s_M_EXPANSION1 },
    { 1, "M_EPI2", M_Expansion, &s_M_EXPANSION2 }
};

menu_t ExpDef =
{
    ex_end,               // # of menu items
    &MainDef,             // previous menu
    ExpansionMenu,        // menuitem_t ->
    M_DrawExpansion,      // drawing routine ->
    39, 69,               // x, y
    ex1                   // lastOn
};

//
// NEW GAME
//

typedef enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_t;

menuitem_t NewGameMenu[] =
{
    { 1, "M_JKILL", M_ChooseSkill, &s_M_SKILLLEVEL1 },
    { 1, "M_ROUGH", M_ChooseSkill, &s_M_SKILLLEVEL2 },
    { 1, "M_HURT",  M_ChooseSkill, &s_M_SKILLLEVEL3 },
    { 1, "M_ULTRA", M_ChooseSkill, &s_M_SKILLLEVEL4 },
    { 1, "M_NMARE", M_ChooseSkill, &s_M_SKILLLEVEL5 }
};

menu_t NewDef =
{
    newg_end,           // # of menu items
    &EpiDef,            // previous menu
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    39, 69,             // x, y
    hurtme              // lastOn
};

//
// OPTIONS MENU
//

typedef enum
{
    endgame,
    msgs,
    detail,
    scrnsize,
    option_empty1,
    mousesens,
    option_empty2,
    soundvol,
    opt_end
} options_t;

menuitem_t OptionsMenu[]=
{
    {  1, "M_ENDGAM", M_EndGame,           &s_M_ENDGAME          },
    {  1, "M_MESSG",  M_ChangeMessages,    &s_M_MESSAGES         },
    {  1, "M_DETAIL", M_ChangeDetail,      &s_M_GRAPHICDETAIL    },
    {  2, "M_SCRNSZ", M_SizeDisplay,       &s_M_SCREENSIZE       },
    { -1, "",         0,                   NULL                  },
    {  2, "M_MSENS",  M_ChangeSensitivity, &s_M_MOUSESENSITIVITY },
    { -1, "",         0,                   NULL                  },
    {  1, "M_SVOL",   M_Sound,             &s_M_SOUNDVOLUME      }
};

menu_t OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    56, 33,
    endgame
};

typedef enum
{
    rdthsempty,
    read_end
} read_t;

menuitem_t ReadMenu[] =
{
    { 1, "", M_FinishReadThis, NULL }
};

menu_t ReadDef =
{
    read_end,
    &ReadDef,
    ReadMenu,
    M_DrawReadThis,
    330, 175,
    rdthsempty
};

//
// SOUND VOLUME MENU
//

typedef enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_end
} sound_t;

menuitem_t SoundMenu[] =
{
    {  2, "M_SFXVOL", M_SfxVol,   &s_M_SFXVOLUME   },
    { -1, "",         0,          NULL             },
    {  2, "M_MUSVOL", M_MusicVol, &s_M_MUSICVOLUME },
    { -1, "",         0,          NULL             }
};

menu_t SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    89, 64,
    sfx_vol
};

//
// LOAD GAME MENU
//

typedef enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load_end
} load_t;

menuitem_t LoadGameMenu[] =
{
    { 1, "", M_LoadSelect, NULL },
    { 1, "", M_LoadSelect, NULL },
    { 1, "", M_LoadSelect, NULL },
    { 1, "", M_LoadSelect, NULL },
    { 1, "", M_LoadSelect, NULL },
    { 1, "", M_LoadSelect, NULL }
};

menu_t LoadDef =
{
    load_end,
    &MainDef,
    LoadGameMenu,
    M_DrawLoad,
    67, 51,
    load1
};

//
// SAVE GAME MENU
//

menuitem_t SaveGameMenu[] =
{
    { 1, "", M_SaveSelect, NULL },
    { 1, "", M_SaveSelect, NULL },
    { 1, "", M_SaveSelect, NULL },
    { 1, "", M_SaveSelect, NULL },
    { 1, "", M_SaveSelect, NULL },
    { 1, "", M_SaveSelect, NULL }
};

menu_t SaveDef =
{
    load_end,
    &MainDef,
    SaveGameMenu,
    M_DrawSave,
    67, 51,
    load1
};

static int height;

static void DoBlurScreen(byte *tempscreen, byte *blurscreen, int x1, int y1, int x2, int y2, int i)
{
    int x, y;

    memcpy(tempscreen, blurscreen, SCREENWIDTH * SCREENHEIGHT);

    for (y = y1; y < y2; y += SCREENWIDTH)
        for (x = y + x1; x < y + x2; x++)
            blurscreen[x] = tinttab50[tempscreen[x] + (tempscreen[x + i] << 8)];
}

static void BlurScreen(byte *screen, byte *tempscreen, byte *blurscreen)
{
    int i;

    for (i = 0; i < height; i++)
        blurscreen[i] = grays[screen[i]];

    DoBlurScreen(tempscreen, blurscreen, 0, 0, SCREENWIDTH - 1, height, 1);
    DoBlurScreen(tempscreen, blurscreen, 1, 0, SCREENWIDTH, height, -1);
    DoBlurScreen(tempscreen, blurscreen, 0, 0, SCREENWIDTH - 1, height - SCREENWIDTH, SCREENWIDTH + 1);
    DoBlurScreen(tempscreen, blurscreen, 1, SCREENWIDTH, SCREENWIDTH, height, -(SCREENWIDTH + 1));
    DoBlurScreen(tempscreen, blurscreen, 0, 0, SCREENWIDTH, height - SCREENWIDTH, SCREENWIDTH);
    DoBlurScreen(tempscreen, blurscreen, 0, SCREENWIDTH, SCREENWIDTH, height, -SCREENWIDTH);
    DoBlurScreen(tempscreen, blurscreen, 1, 0, SCREENWIDTH, height - SCREENWIDTH, SCREENWIDTH - 1);
    DoBlurScreen(tempscreen, blurscreen, 0, SCREENWIDTH, SCREENWIDTH - 1, height, -(SCREENWIDTH - 1));
}

//
// M_DarkBackground
//  darken and blur background while menu is displayed
//
void M_DarkBackground(void)
{
    int i;

    height = (SCREENHEIGHT - vid_widescreen * SBARHEIGHT) * SCREENWIDTH;

    if (!blurred || !blurred2)
    {
        BlurScreen(screens[0], tempscreen1, blurscreen1);

        for (i = 0; i < height; i++)
            blurscreen1[i] = tinttab50[blurscreen1[i]];

        if (mapwindow)
        {
            BlurScreen(mapscreen, tempscreen2, blurscreen2);

            for (i = 0; i < (SCREENHEIGHT - SBARHEIGHT) * SCREENWIDTH; i++)
                blurscreen2[i] = tinttab50[blurscreen2[i]];
        }

        blurred2 = true;

        if (!blurred)
            blurred2 = false;

        blurred = true;
    }

    memcpy(screens[0], blurscreen1, height);

    if (mapwindow)
        memcpy(mapscreen, blurscreen2, (SCREENHEIGHT - SBARHEIGHT) * SCREENWIDTH);

    if (r_detail == r_detail_low && viewactive)
        V_LowGraphicDetail();

    for (i = 0; i < height; i += SCREENWIDTH)
    {
        screens[0][i] = tinttab50[screens[0][i]];
        screens[0][i + SCREENWIDTH - 1] = tinttab50[screens[0][i + SCREENWIDTH - 1]];
    }

    for (i = 1; i < SCREENWIDTH - 1; i++)
    {
        screens[0][i] = tinttab50[screens[0][i]];
        screens[0][i + height - SCREENWIDTH] = tinttab50[screens[0][i + height - SCREENWIDTH]];
    }
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
// M_DarkBlueBackground
//  darken background, make it blue and pixelate while help screen is displayed
//
void M_DarkBlueBackground(void)
{
    int x, y;

    for (y = 0; y < SCREENWIDTH * SCREENHEIGHT; y += SCREENWIDTH * 2)
        for (x = y; x < y + SCREENWIDTH; x += 2)
        {
            byte    *dot = *screens + x;
            byte    *copy;

            *dot = blues[*dot];
            copy = dot + 1;
            *copy = *dot;
            copy += SCREENWIDTH;
            *copy-- = *dot;
            *copy = *dot;
        }
}

//
// M_DrawChar
//  draw a character on screen
//
void M_DrawChar(int x, int y, int i, dboolean overlapping)
{
    int x1, y1;
    int w = strlen(redcharset[i]) / 18;

    for (y1 = 0; y1 < 18; y1++)
        for (x1 = 0; x1 < w; x1++)
        {
            char    dot = redcharset[i][y1 * w + x1];

            if (dot == '\xC8')
            {
                if (!overlapping)
                    V_DrawPixel(x + x1, y + y1, 251, true);
            }
            else
                V_DrawPixel(x + x1, y + y1, (int)dot, true);
        }
}

static const int chartoi[123] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1,  0, -1, -1, -1, -1, -1,  1,
    -1, -1, -1, -1,  2,  3,  4, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,  5, -1,
    -1, -1, -1,  6, -1,  7,  8,  9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, -1, -1, -1, -1, -1, -1, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
    46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58
};

static struct
{
    char    char1;
    char    char2;
    int     adjust;
} kern[] = {
    { '-', 'V', -2 }, { 'O', 'A', -1 }, { 'P', 'a', -3 }, { 'V', 'o', -2 },
    { 'f', 'e', -1 }, { 'f', 'f', -1 }, { 'f', 'o', -1 }, { 'l', 'e', -1 },
    { 'l', 't', -1 }, { 'l', 'u', -1 }, { 'o', 'a', -1 }, { 'o', 't', -1 },
    { 'p', 'a', -2 }, { 't', 'o', -1 }, { 'v', 'e', -1 }, { 'y', ',', -3 },
    { 'y', '.', -2 }, { 'y', 'o', -1 }, { 't', 'a', -1 }, { 'l', 'o', -1 },
    { ' ', 'V', -2 }, { ' ', 'y', -2 }, { ' ', 't', -1 }, { 'l', ' ', -1 },
    { 'L', 'S', -1 }, { 't', ' ', -1 }, {  0,   0,   0 }
};

static struct
{
    char    char1;
    char    char2;
} overlap[] = {
    { 'A', 'D' }, { 'A', 'M' }, { 'E', 'a' }, { 'E', 'n' }, { 'E', 'p' },
    { 'E', 'x' }, { 'G', 'A' }, { 'G', 'a' }, { 'I', 'n' }, { 'K', 'n' },
    { 'L', 'i' }, { 'a', 'd' }, { 'a', 'm' }, { 'a', 'n' }, { 'a', 'r' },
    { 'c', 'h' }, { 'c', 'r' }, { 'e', 'a' }, { 'e', 'd' }, { 'e', 'n' },
    { 'e', 'p' }, { 'e', 'r' }, { 'e', 's' }, { 'g', 'h' }, { 'h', 'i' },
    { 'i', 'n' }, { 'i', 's' }, { 'i', 'z' }, { 'k', 'i' }, { 'p', 'i' },
    { 'p', 't' }, { 'r', 'a' }, { 'r', 'n' }, { 'x', 'p' }, { 'G', 'r' },
    { 'a', 'p' }, { 'a', 'i' }, { 'e', 't' }, { 'i', 't' }, { 'o', 't' },
    { 'P', 'T' }, { 'r', 't' }, { 's', 't' }, { 'n', 't' }, {  0,   0  }
};

//
// M_DrawString
//  draw a string on screen
//
void M_DrawString(int x, int y, char *str)
{
    int         i;
    static char prev;

    for (i = 0; (unsigned int)i < strlen(str); i++)
    {
        int         j = -1;
        int         k = 0;
        dboolean    overlapping = false;

        if (str[i] < 123)
            j = chartoi[(int)str[i]];

        while (kern[k].char1)
        {
            if (prev == kern[k].char1 && str[i] == kern[k].char2)
            {
                x += kern[k].adjust;
                break;
            }

            k++;
        }

        k = 0;

        while (overlap[k].char1)
        {
            if (prev == overlap[k].char1 && str[i] == overlap[k].char2)
            {
                overlapping = true;
                break;
            }

            k++;
        }

        if (j == -1)
            x += 9;
        else
        {
            M_DrawChar(x, y, j, overlapping);
            x += strlen(redcharset[j]) / 18 - 2;
        }

        prev = str[i];
    }
}

//
// M_BigStringWidth
//  return width of string in pixels
//
int M_BigStringWidth(char *str)
{
    int         i;
    int         w = 0;
    static char prev;
    size_t      len = strlen(str);

    for (i = 0; (unsigned int)i < len; i++)
    {
        int j = chartoi[(int)str[i]];
        int k = 0;

        while (kern[k].char1)
        {
            if (prev == kern[k].char1 && str[i] == kern[k].char2)
                w += kern[k].adjust;

            k++;
        }

        w += (j == -1 ? 9 : strlen(redcharset[j]) / 18 - 2);

        prev = str[i];
    }

    return w;
}

//
// M_DrawCenteredString
//  draw a string centered horizontally on screen
//
void M_DrawCenteredString(int y, char *str)
{
    M_DrawString((ORIGINALWIDTH - M_BigStringWidth(str) - 1) / 2, y, str);
}

//
// M_SplitString
//  split string of words into two lines
//
void M_SplitString(char *string)
{
    int i;

    for (i = strlen(string) / 2 - 1; (unsigned int)i < strlen(string); i++)
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
void M_DrawPatchWithShadow(int x, int y, patch_t *patch)
{
    short   width = SHORT(patch->width);
    short   height = SHORT(patch->height);

    if (width >= ORIGINALWIDTH || height >= ORIGINALHEIGHT)
    {
        patch->leftoffset = 0;
        patch->topoffset = 0;
        V_DrawPagePatch(patch);
    }
    else
        V_DrawPatchWithShadow(x, y, patch, false);
}

//
// M_DrawCenteredPatchWithShadow
//  draw patch with shadow horizontally centered on screen
//
void M_DrawCenteredPatchWithShadow(int y, patch_t *patch)
{
    short   width = SHORT(patch->width);
    short   height = SHORT(patch->height);

    if (width >= ORIGINALWIDTH || height >= ORIGINALHEIGHT)
    {
        patch->leftoffset = 0;
        patch->topoffset = 0;
        V_DrawPagePatch(patch);
    }
    else
        V_DrawPatchWithShadow((ORIGINALWIDTH - width) / 2 + SHORT(patch->leftoffset), y, patch, false);
}

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
    int     i;
    char    name[256];

    savegames = false;

    for (i = 0; i < load_end; i++)
    {
        FILE    *handle;

        M_StringCopy(name, P_SaveGameFile(i), sizeof(name));

        if (!(handle = fopen(name, "rb")))
        {
            M_StringCopy(&savegamestrings[i][0], s_EMPTYSTRING, SAVESTRINGSIZE);
            LoadGameMenu[i].status = 0;
            continue;
        }

        savegames = true;
        fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadGameMenu[i].status = 1;
    }
}

static byte saveg_read8(FILE *file)
{
    byte    result = -1;

    if (fread(&result, 1, 1, file) < 1)
        return 0;

    return result;
}

//
// M_CheckSaveGame
//
dboolean M_CheckSaveGame(int choice)
{
    FILE    *handle = fopen(P_SaveGameFile(itemOn), "rb");
    int     ep;
    int     mission;
    int     i;

    if (!handle)
        return true;

    for (i = 0; i < SAVESTRINGSIZE + VERSIONSIZE + 1; i++)
        saveg_read8(handle);

    ep = saveg_read8(handle);
    saveg_read8(handle);
    mission = saveg_read8(handle);

    fclose(handle);

    // switch expansions if necessary
    if (mission == doom2)
    {
        if (gamemission == doom2)
            return true;

        if (gamemission == pack_nerve)
        {
            ExpDef.lastOn = ex1;
            expansionselected = ex1;
            gamemission = doom2;
            return true;
        }
        else
            return false;
    }
    else if (mission == pack_nerve)
    {
        if (gamemission == pack_nerve)
            return true;

        if (gamemission == doom2 && nerve)
        {
            ExpDef.lastOn = ex2;
            expansionselected = ex2;
            gamemission = pack_nerve;
            return true;
        }
        else
            return false;
    }

    if (mission != gamemission)
        return false;

    if (ep > 1 && gamemode == shareware)
        return false;

    if (ep > 3 && gamemode == registered)
        return false;

    return true;
}

//
// M_LoadGame
//
void M_DrawLoad(void)
{
    int i;

    M_DarkBackground();

    if (M_LOADG)
        M_DrawCenteredPatchWithShadow(23 + OFFSET, W_CacheLumpName("M_LOADG"));
    else
        M_DrawCenteredString(23 + OFFSET, uppercase(s_M_LOADGAME));

    for (i = 0; i < load_end; i++)
    {
        int y = LoadDef.y + LINEHEIGHT * i + OFFSET;

        M_DrawSaveLoadBorder(LoadDef.x - 11, y - 4);
        M_WriteText(LoadDef.x - 2 + (M_StringCompare(savegamestrings[i], s_EMPTYSTRING)
            && s_EMPTYSTRING[0] == '-' && s_EMPTYSTRING[1] == '\0') * 6, y - !M_LSCNTR,
            savegamestrings[i], false);
    }
}

//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x, int y)
{
    int i;

    if (M_LSCNTR)
    {
        x += 3;
        M_DrawPatchWithShadow(x, y + 11, W_CacheLumpName("M_LSLEFT"));

        x += 8;

        for (i = 0; i < 24; i++)
        {
            M_DrawPatchWithShadow(x, y + 11, W_CacheLumpName("M_LSCNTR"));
            x += 8;
        }

        M_DrawPatchWithShadow(x, y + 11, W_CacheLumpName("M_LSRGHT"));
    }
    else
    {
        int xx, yy;

        for (yy = 0; yy < 16; yy++)
            for (xx = 0; xx < 8; xx++)
                V_DrawPixel(x + xx, y + yy, lsleft[yy * 8 + xx], true);

        x += 8;

        for (i = 0; i < 24; i++)
        {
            for (yy = 0; yy < 16; yy++)
                for (xx = 0; xx < 8; xx++)
                    V_DrawPixel(x + xx, y + yy, lscntr[yy * 8 + xx], true);

            x += 8;
        }

        for (yy = 0; yy < 16; yy++)
            for (xx = 0; xx < 9; xx++)
                V_DrawPixel(x + xx, y + yy, lsrght[yy * 9 + xx], true);
    }
}

//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    if (M_CheckSaveGame(choice))
    {
        char    name[SAVESTRINGSIZE];

        M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));

        S_StartSound(NULL, sfx_pistol);
        I_WaitVBL(2 * TICRATE);
        functionkey = 0;
        quickSaveSlot = choice;
        M_ClearMenus();
        G_LoadGame(name);
    }
    else
    {
        menuactive = false;
        C_ShowConsole();
        C_Warning("This savegame requires a different WAD.");
    }
}

//
// Selected from DOOM menu
//
void M_LoadGame(int choice)
{
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}

#define CARETBLINKTIME  350

static dboolean showcaret;
static int      caretwait;
int             caretcolor;

//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    char    *left = Z_Malloc(256, PU_STATIC, NULL);
    char    *right = Z_Malloc(256, PU_STATIC, NULL);
    int     i;
    int     j;

    // darken background
    M_DarkBackground();

    // draw menu subtitle
    if (M_SAVEG)
        M_DrawCenteredPatchWithShadow(23 + OFFSET, W_CacheLumpName("M_SAVEG"));
    else
        M_DrawCenteredString(23 + OFFSET, uppercase(s_M_SAVEGAME));

    // draw each save game slot
    for (i = 0; i < load_end; i++)
    {
        int y = LoadDef.y + i * LINEHEIGHT + OFFSET;

        // draw save game slot background
        M_DrawSaveLoadBorder(LoadDef.x - 11, y - 4);

        // draw save game description
        if (saveStringEnter && i == saveSlot)
        {
            // draw text to left of text caret
            for (j = 0; j < saveCharIndex; j++)
                left[j] = savegamestrings[i][j];

            left[j] = 0;
            M_WriteText(LoadDef.x - 2, y - !M_LSCNTR, left, false);

            // draw text to right of text caret
            for (j = 0; (unsigned int)j < strlen(savegamestrings[i]) - saveCharIndex; j++)
                right[j] = savegamestrings[i][j + saveCharIndex];

            right[j] = 0;
            M_WriteText(LoadDef.x - 2 + M_StringWidth(left) + 1, y - !M_LSCNTR, right, false);
        }
        else
            M_WriteText(LoadDef.x - 2 + (M_StringCompare(savegamestrings[i], s_EMPTYSTRING)
                && s_EMPTYSTRING[0] == '-' && s_EMPTYSTRING[1] == '\0') * 6, y - !M_LSCNTR,
                savegamestrings[i], false);
    }

    // draw text caret
    if (saveStringEnter)
    {
        if (windowfocused)
        {
            if (caretwait < I_GetTimeMS())
            {
                showcaret = !showcaret;
                caretwait = I_GetTimeMS() + CARETBLINKTIME;
            }

            if (showcaret)
            {
                int x = LoadDef.x - 2 + M_StringWidth(left);
                int y = LoadDef.y + saveSlot * LINEHEIGHT + OFFSET - 1;
                int h = y + SHORT(hu_font['A' - HU_FONTSTART]->height) + 2;

                while (y < h)
                    V_DrawPixel(x, y++, caretcolor, false);
            }
        }
        else
        {
            showcaret = false;
            caretwait = 0;
        }
    }

    Z_Free(left);
    Z_Free(right);
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    M_ClearMenus();
    G_SaveGame(slot, savegamestrings[slot], "");

    savegames = true;
    functionkey = 0;
    quickSaveSlot = slot;

    savegame = savegamestrings[slot];
}

//
// User wants to save. Start string input for M_Responder
//
extern char maptitle[128];

extern char **mapnames[];
extern char **mapnames2[];
extern char **mapnames2_bfg[];
extern char **mapnamesp[];
extern char **mapnamest[];
extern char **mapnamesn[];

const char *RemoveMapNum(const char *str)
{
    char    *pos;

    if ((pos = strchr(str, ':')))
    {
        str = pos + 1;

        while (str[0] == ' ')
            str++;
    }

    return str;
}

void M_UpdateSaveGameName(int i)
{
    dboolean    match = false;

    if (M_StringCompare(savegamestrings[i], s_EMPTYSTRING))
        match = true;
    else if (gamemission == doom && strlen(savegamestrings[i]) == 4 && savegamestrings[i][0] == 'E'
        && isdigit(savegamestrings[i][1]) && savegamestrings[i][2] == 'M' && isdigit(savegamestrings[i][3])
        && W_CheckNumForName(savegamestrings[i]) >= 0)
        match = true;
    else if (gamemission != doom && strlen(savegamestrings[i]) == 5 && savegamestrings[i][0] == 'M'
        && savegamestrings[i][1] == 'A' && savegamestrings[i][2] == 'P' && isdigit(savegamestrings[i][3])
        && isdigit(savegamestrings[i][4]) && W_CheckNumForName(savegamestrings[i]) >= 0)
        match = true;

    if (!match && !M_StringCompare(maptitle, mapnumandtitle))
    {
        int len = strlen(savegamestrings[i]);

        if (len >= 4 && savegamestrings[i][len - 1] == '.' && savegamestrings[i][len - 2] == '.'
            && savegamestrings[i][len - 3] == '.' && savegamestrings[i][len - 4] != '.')
            match = true;
        else
        {
            int j = 0;

            switch (gamemission)
            {
                case doom:
                    for (j = 0; j < 9 * 4; j++)
                        if (M_StringCompare(savegamestrings[i], RemoveMapNum(*mapnames[j])))
                        {
                            match = true;
                            break;
                        }

                    break;

                case doom2:
                case pack_nerve:
                    if (bfgedition)
                    {
                        for (j = 0; j < 33; j++)
                            if (M_StringCompare(savegamestrings[i],
                                RemoveMapNum(*mapnames2_bfg[j])))
                            {
                                match = true;
                                break;
                            }
                    }
                    else
                    {
                        for (j = 0; j < 32; j++)
                            if (M_StringCompare(savegamestrings[i], RemoveMapNum(*mapnames2[j])))
                            {
                                match = true;
                                break;
                            }
                    }

                    for (j = 0; j < 9; j++)
                        if (M_StringCompare(savegamestrings[i], RemoveMapNum(*mapnamesn[j])))
                        {
                            match = true;
                            break;
                        }

                    break;

                case pack_plut:
                    for (j = 0; j < 32; j++)
                        if (M_StringCompare(savegamestrings[i], RemoveMapNum(*mapnamesp[j])))
                        {
                            match = true;
                            break;
                        }

                    break;

                case pack_tnt:
                    for (j = 0; j < 32; j++)
                        if (M_StringCompare(savegamestrings[i], RemoveMapNum(*mapnamest[j])))
                        {
                            match = true;
                            break;
                        }

                    break;

                default:
                    break;
            }
        }
    }

    if (match)
    {
        int len = strlen(maptitle);

        M_StringCopy(savegamestrings[i], maptitle, SAVESTRINGSIZE);

        while (M_StringWidth(savegamestrings[i]) > SAVESTRINGPIXELWIDTH)
        {
            savegamestrings[i][len - 1] = '.';
            savegamestrings[i][len] = '.';
            savegamestrings[i][len + 1] = '.';
            savegamestrings[i][len + 2] = '\0';
            len--;
        }
    }
}

void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;

    saveSlot = choice;
    M_StringCopy(saveOldString, savegamestrings[saveSlot], SAVESTRINGSIZE);
    M_UpdateSaveGameName(saveSlot);
    saveCharIndex = strlen(savegamestrings[saveSlot]);
}

//
// Selected from DOOM menu
//
void M_SaveGame(int choice)
{
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}

//
// M_QuickSave
//
void M_QuickSave(void)
{
    if (quickSaveSlot < 0)
    {
        if (functionkey == KEY_F6)
        {
            functionkey = 0;
            M_ClearMenus();
            S_StartSound(NULL, sfx_swtchx);
        }
        else
        {
            functionkey = KEY_F6;
            M_StartControlPanel();
            M_ReadSaveStrings();
            M_SetupNextMenu(&SaveDef);
            S_StartSound(NULL, sfx_swtchn);
        }

        return;
    }

    M_UpdateSaveGameName(quickSaveSlot);
    M_DoSave(quickSaveSlot);
}

//
// M_QuickLoad
//
void M_QuickLoadResponse(int key)
{
    messageToPrint = false;

    if (key == 'y')
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL, sfx_swtchx);
    }
}

char    tempstring[160];

void M_QuickLoad(void)
{
    if (quickSaveSlot < 0)
    {
        functionkey = 0;
        return;
    }

    S_StartSound(NULL, sfx_swtchn);

    if (M_StringEndsWith(s_QLPROMPT, s_PRESSYN))
        M_StartMessage(s_QLPROMPT, M_QuickLoadResponse, true);
    else
    {
        M_snprintf(tempstring, 160, s_QLPROMPT, savegamestrings[quickSaveSlot]);
        M_SplitString(tempstring);
        M_snprintf(tempstring, 160, "%s\n\n%s", tempstring, (usinggamepad ? s_PRESSA : s_PRESSYN));
        M_StartMessage(tempstring, M_QuickLoadResponse, true);
    }
}

static void M_DeleteSavegameResponse(int key)
{
    if (key == 'y')
    {
        static char buffer[1024];

        M_StringCopy(buffer, P_SaveGameFile(itemOn), sizeof(buffer));

        if (remove(buffer) == -1)
        {
            S_StartSound(NULL, sfx_oof);
            return;
        }

        M_snprintf(buffer, sizeof(buffer), s_GGDELETED, titlecase(savegamestrings[itemOn]));
        HU_PlayerMessage(buffer, false);
        blurred = false;
        message_dontfuckwithme = true;

        M_ReadSaveStrings();

        if (currentMenu == &LoadDef)
        {
            if (savegames)
            {
                while (M_StringCompare(savegamestrings[itemOn], s_EMPTYSTRING))
                    itemOn = (!itemOn ? currentMenu->numitems - 1 : itemOn - 1);
            }
            else
            {
                M_SetupNextMenu(&MainDef);
                MainDef.lastOn = itemOn = save_game;
            }
        }
    }
}

void M_DeleteSavegame(void)
{
    S_StartSound(NULL, sfx_swtchn);
    M_snprintf(tempstring, 160, s_DELPROMPT, savegamestrings[saveSlot]);
    M_SplitString(tempstring);
    M_snprintf(tempstring, 160, "%s\n\n%s", tempstring, (usinggamepad ? s_PRESSA : s_PRESSYN));
    M_StartMessage(tempstring, M_DeleteSavegameResponse, true);
}

//
// M_DrawReadThis
//
void M_DrawReadThis(void)
{
    char    *lumpname = "HELP1";

    switch (gameversion)
    {
        case exe_doom_1_9:
            if (gamemode == shareware && W_CheckNumForName("HELP3") >= 0)
                lumpname = "HELP3";
            else
                lumpname = (gamemode == commercial ? "HELP" : "HELP2");

            break;

        case exe_final:
            lumpname = "HELP";
            break;

        default:
            break;
    }
    if (W_CheckNumForName(lumpname) >= 0)
    {
        if (automapactive)
            V_FillRect(0, 0, 0, SCREENWIDTH, SCREENHEIGHT, 245);
        else
        {
            players[0].fixedcolormap = 0;
            M_DarkBlueBackground();
        }

        if (hacx)
            V_DrawPatch(0, 0, 0, W_CacheLumpNum(W_GetNumForNameX("HELP", 1)));
        else if (W_CheckMultipleLumps(lumpname) > 2)
            V_DrawPatch(0, 0, 0, W_CacheLumpNum(W_GetNumForNameX(lumpname, 2)));
        else
            M_DrawPatchWithShadow(0, 0, W_CacheLumpName(lumpname));
    }
}

//
// Change SFX & Music volumes
//
void M_DrawSound(void)
{
    M_DarkBackground();

    if (M_SVOL)
    {
        M_DrawPatchWithShadow((chex ? 100 : 60), 38 + OFFSET, W_CacheLumpName("M_SVOL"));
        SoundDef.x = (chex ? 68 : 80);
        SoundDef.y = 64;
    }
    else
        M_DrawCenteredString(38 + OFFSET, uppercase(s_M_SOUNDVOLUME));

    M_DrawThermo(SoundDef.x - 1, SoundDef.y + 16 * (sfx_vol + 1) + OFFSET + !hacx, 16,
        (float)(sfxVolume * !nosfx), 4.0f, 6);

    M_DrawThermo(SoundDef.x - 1, SoundDef.y + 16 * (music_vol + 1) + OFFSET + !hacx, 16,
        (float)(musicVolume * !nomusic), 4.0f, 6);
}

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
    if (!nosfx)
    {
        switch (choice)
        {
            case 0:
                if (sfxVolume > 0)
                {
                    S_SetSfxVolume(--sfxVolume * MAX_SFX_VOLUME / 31);
                    S_StartSound(NULL, sfx_stnmov);
                    s_sfxvolume = sfxVolume * 100 / 31;
                    C_PctCVAROutput(stringize(s_sfxvolume), s_sfxvolume);
                    M_SaveCVARs();
                }

                break;

            case 1:
                if (sfxVolume < 31)
                {
                    S_SetSfxVolume(++sfxVolume * MAX_SFX_VOLUME / 31);
                    S_StartSound(NULL, sfx_stnmov);
                    s_sfxvolume = sfxVolume * 100 / 31;
                    C_PctCVAROutput(stringize(s_sfxvolume), s_sfxvolume);
                    M_SaveCVARs();
                }

                break;
        }
    }
}

void M_MusicVol(int choice)
{
    if (!nomusic)
    {
        switch (choice)
        {
            case 0:
                if (musicVolume > 0)
                {
                    S_SetMusicVolume(--musicVolume * MAX_MUSIC_VOLUME / 31);
                    S_StartSound(NULL, sfx_stnmov);
                    s_musicvolume = musicVolume * 100 / 31;
                    C_PctCVAROutput(stringize(s_musicvolume), s_musicvolume);
                    M_SaveCVARs();
                }

                break;

            case 1:
                if (musicVolume < 31)
                {
                    S_SetMusicVolume(++musicVolume * MAX_MUSIC_VOLUME / 31);
                    S_StartSound(NULL, sfx_stnmov);
                    s_musicvolume = musicVolume * 100 / 31;
                    C_PctCVAROutput(stringize(s_musicvolume), s_musicvolume);
                    M_SaveCVARs();
                }

                break;
        }
    }
}

//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    patch_t *patch = W_CacheLumpName("M_DOOM");

    M_DarkBackground();

    if (M_DOOM)
    {
        M_DrawPatchWithShadow(94, 2 + OFFSET, patch);
        MainDef.x = 97;
        MainDef.y = 72;
    }
    else
    {
        int y = 11 + OFFSET;
        int dot1 = screens[0][(y * SCREENWIDTH + 98) * 2];
        int dot2 = screens[0][((y + 1) * SCREENWIDTH + 99) * 2];

        M_DrawCenteredPatchWithShadow(y, patch);

        if (gamemode != commercial)
        {
            V_DrawPixel(98, y, dot1, false);
            V_DrawPixel(99, y + 1, dot2, false);
        }
    }
}

//
// M_NewGame
//
void M_DrawNewGame(void)
{
    M_DarkBackground();

    if (M_NEWG)
    {
        M_DrawPatchWithShadow((chex ? 118 : 96), 14 + OFFSET, W_CacheLumpName("M_NEWG"));
        NewDef.x = (chex ? 98 : 48);
        NewDef.y = 63;
    }
    else
        M_DrawCenteredString(19 + OFFSET, uppercase(s_M_NEWGAME));

    if (M_SKILL)
    {
        M_DrawPatchWithShadow((chex ? 76 : 54), 38 + OFFSET, W_CacheLumpName("M_SKILL"));
        NewDef.x = (chex ? 98 : 48);
        NewDef.y = 63;
    }
    else
        M_DrawCenteredString(44 + OFFSET, s_M_CHOOSESKILLLEVEL);
}

void M_NewGame(int choice)
{
    if (chex)
        M_SetupNextMenu(&NewDef);
    else
        M_SetupNextMenu(gamemode == commercial ? (nerve ? &ExpDef : &NewDef) : &EpiDef);
}

//
// M_Episode
//
int epi;

void M_DrawEpisode(void)
{
    M_DarkBackground();

    if (M_NEWG)
    {
        M_DrawPatchWithShadow(96, 14 + OFFSET, W_CacheLumpName("M_NEWG"));
        EpiDef.x = 48;
        EpiDef.y = 63;
    }
    else
        M_DrawCenteredString(19 + OFFSET, uppercase(s_M_NEWGAME));

    if (M_EPISOD)
    {
        M_DrawPatchWithShadow(54, 38 + OFFSET, W_CacheLumpName("M_EPISOD"));
        EpiDef.x = 48;
        EpiDef.y = 63;
    }
    else
        M_DrawCenteredString(44 + OFFSET, s_M_WHICHEPISODE);
}

void M_SetWindowCaption(void)
{
    static char caption[128];

    if (gamestate == GS_LEVEL)
        M_StringCopy(caption, mapnumandtitle, sizeof(caption));
    else
    {
        if (nerve && (currentMenu == &ExpDef || currentMenu == &NewDef))
            M_snprintf(caption, sizeof(caption), "%s: %s", gamedescription,
            (expansionselected == ex1 ? s_CAPTION_HELLONEARTH : s_CAPTION_NERVE));
        else
            M_StringCopy(caption, gamedescription, sizeof(caption));

        if (bfgedition && !modifiedgame)
            M_snprintf(caption, sizeof(caption), "%s (%s)", caption, s_CAPTION_BFGEDITION);
    }

    SDL_SetWindowTitle(window, caption);
}

void M_DrawExpansion(void)
{
    M_DarkBackground();
    M_DrawCenteredString(19 + OFFSET, uppercase(s_M_NEWGAME));
    M_DrawCenteredString(44 + OFFSET, s_M_WHICHEXPANSION);
}

void M_VerifyNightmare(int key)
{
    messageToPrint = false;

    if (key != 'y')
        M_SetupNextMenu(&NewDef);
    else
    {
        S_StartSound(NULL, sfx_swtchx);
        I_WaitVBL(2 * TICRATE);
        quickSaveSlot = -1;
        M_ClearMenus();
        G_DeferredInitNew((skill_t)nightmare, epi + 1, 1);
    }
}

void M_ChooseSkill(int choice)
{
    if (choice == nightmare && gameskill != sk_nightmare && !nomonsters)
    {
        if (M_StringEndsWith(s_NIGHTMARE, s_PRESSYN))
            M_StartMessage(s_NIGHTMARE, M_VerifyNightmare, true);
        else
        {
            M_snprintf(tempstring, 160, "%s\n\n%s", s_NIGHTMARE,
                (usinggamepad ? s_PRESSA : s_PRESSYN));
            M_StartMessage(tempstring, M_VerifyNightmare, true);
        }

        return;
    }

    HU_DrawDisk();
    S_StartSound(NULL, sfx_pistol);
    I_WaitVBL(2 * TICRATE);
    quickSaveSlot = -1;
    M_ClearMenus();
    G_DeferredInitNew((skill_t)choice, epi + 1, 1);
}

void M_Episode(int choice)
{
    if (gamemode == shareware && choice)
    {
        if (M_StringEndsWith(s_SWSTRING, s_PRESSYN))
            M_StartMessage(s_SWSTRING, NULL, false);
        else
        {
            M_snprintf(tempstring, 160, "%s\n\n%s", s_SWSTRING,
                (usinggamepad ? s_PRESSA : s_PRESSYN));
            M_StartMessage(tempstring, NULL, false);
        }

        M_SetupNextMenu(&EpiDef);
        return;
    }

    epi = choice;
    M_SetupNextMenu(&NewDef);
}

void M_Expansion(int choice)
{
    gamemission = (choice == ex1 ? doom2 : pack_nerve);
    M_SetupNextMenu(&NewDef);
}

//
// M_Options
//
void M_DrawOptions(void)
{
    M_DarkBackground();

    if (M_OPTTTL)
    {
        M_DrawPatchWithShadow((chex ? 126 : 108), 15 + OFFSET, W_CacheLumpName("M_OPTTTL"));
        OptionsDef.x = (chex ? 69 : 60);
        OptionsDef.y = 37;
    }
    else
        M_DrawCenteredString(8 + OFFSET, uppercase(s_M_OPTIONS));

    if (messages)
    {
        if (M_MSGON)
            M_DrawPatchWithShadow(OptionsDef.x + 125, OptionsDef.y + 16 * msgs + OFFSET,
                W_CacheLumpName("M_MSGON"));
        else
            M_DrawString(OptionsDef.x + 125, OptionsDef.y + 16 * msgs + OFFSET, s_M_ON);
    }
    else
    {
        if (M_MSGOFF)
            M_DrawPatchWithShadow(OptionsDef.x + 125, OptionsDef.y + 16 * msgs + OFFSET,
                W_CacheLumpName("M_MSGOFF"));
        else
            M_DrawString(OptionsDef.x + 125, OptionsDef.y + 16 * msgs + OFFSET, s_M_OFF);
    }

    if (r_detail == r_detail_low)
    {
        if (M_GDLOW)
            M_DrawPatchWithShadow(OptionsDef.x + 180, OptionsDef.y + 16 * detail + OFFSET,
                W_CacheLumpName("M_GDLOW"));
        else
            M_DrawString(OptionsDef.x + 177, OptionsDef.y + 16 * detail + OFFSET, s_M_LOW);
    }
    else
    {
        if (M_GDHIGH)
            M_DrawPatchWithShadow(OptionsDef.x + 180, OptionsDef.y + 16 * detail + OFFSET,
                W_CacheLumpName("M_GDHIGH"));
        else
            M_DrawString(OptionsDef.x + 177, OptionsDef.y + 16 * detail + OFFSET, s_M_HIGH);
    }

    M_DrawThermo(OptionsDef.x - 1, OptionsDef.y + 16 * (scrnsize + 1) + OFFSET + !hacx, 9,
        (float)(r_screensize + (vid_widescreen || (returntowidescreen && gamestate != GS_LEVEL)) + !r_hud),
        7.2f, 8);

    if (usinggamepad && !M_MSENS)
        M_DrawThermo(OptionsDef.x - 1, OptionsDef.y + 16 * (mousesens + 1) + OFFSET + 1, 9,
            gp_sensitivity / (float)gp_sensitivity_max * 8.0f, 8.0f, 8);
    else
        M_DrawThermo(OptionsDef.x - 1, OptionsDef.y + 16 * (mousesens + 1) + OFFSET + !hacx, 9,
            m_sensitivity / (float)m_sensitivity_max * 8.0f, 8.0f, 8);
}

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}

//
// Toggle messages on/off
//
dboolean    message_dontpause;

void M_ChangeMessages(int choice)
{
    blurred = false;
    messages = !messages;

    if (menuactive)
        message_dontpause = true;

    C_StrCVAROutput(stringize(messages), (messages ? "on" : "off"));
    HU_PlayerMessage((messages ? s_MSGON : s_MSGOFF), false);
    message_dontfuckwithme = true;
    M_SaveCVARs();
}

//
// M_EndGame
//
dboolean        endinggame;

void M_EndingGame(void)
{
    endinggame = true;

    if (vid_widescreen)
    {
        I_ToggleWidescreen(false);
        returntowidescreen = true;
    }

    if (gamemission == pack_nerve)
        gamemission = doom2;

    episode = "";
    expansion = "";
    savegame = "";
    skilllevel = "";
    C_AddConsoleDivider();
    M_SetWindowCaption();
    D_StartTitle(1);
}

void M_EndGameResponse(int key)
{
    messageToPrint = false;
    if (key != 'y')
    {
        if (functionkey == KEY_F7)
            M_ClearMenus();
        else
            M_SetupNextMenu(&OptionsDef);

        return;
    }

    currentMenu->lastOn = itemOn;
    M_ClearMenus();
    viewactive = false;
    automapactive = false;
    S_StartSound(NULL, sfx_swtchx);
    I_WaitVBL(2 * TICRATE);
    MainDef.lastOn = 0;
    st_palette = 0;
    M_EndingGame();
}

void M_EndGame(int choice)
{
    if (gamestate != GS_LEVEL)
        return;

    if (M_StringEndsWith(s_ENDGAME, s_PRESSYN))
        M_StartMessage(s_ENDGAME, M_EndGameResponse, true);
    else
    {
        M_snprintf(tempstring, 160, "%s\n\n%s", s_ENDGAME, (usinggamepad ? s_PRESSA : s_PRESSYN));
        M_StartMessage(tempstring, M_EndGameResponse, true);
    }
}

//
// M_ReadThis
//
void M_FinishReadThis(int choice)
{
    M_SetupNextMenu(&MainDef);
}

//
// M_QuitDOOM
//
int quitsounds[8] =
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

int quitsounds2[8] =
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

dboolean        quitting;

extern dboolean waspaused;

void M_QuitResponse(int key)
{
    messageToPrint = false;

    if (key != 'y')
    {
        quitting = false;

        if (waspaused)
        {
            waspaused = false;
            paused = true;
        }

        if (functionkey == KEY_F10)
            M_ClearMenus();
        else
            M_SetupNextMenu(&MainDef);

        return;
    }

    if (!nosfx && sfxVolume > 0)
    {
        int i = 30;

        if (gamemode == commercial)
            S_StartSound(NULL, quitsounds2[M_Random() % 8]);
        else
            S_StartSound(NULL, quitsounds[M_Random() % 8]);

        // wait until all sounds stopped or 3 seconds has passed
        while (i > 0)
        {
            I_Sleep(100);

            if (!I_AnySoundStillPlaying())
                break;
            i--;
        }
    }

    I_Quit(true);
}

static char *M_SelectEndMessage(void)
{
    return *endmsg[M_Random() % NUM_QUITMESSAGES + (gamemission != doom) * NUM_QUITMESSAGES];
}

void M_QuitDOOM(int choice)
{
    quitting = true;
    M_snprintf(endstring, 160, "%s\n\n%s", M_SelectEndMessage(), (usinggamepad ? s_DOSA : s_DOSY));
    M_StartMessage(endstring, M_QuitResponse, true);
}

void M_SliderSound(void)
{
    static int  wait;

    if (wait < I_GetTime())
    {
        wait = I_GetTime() + 7;
        S_StartSound(NULL, sfx_stnmov);
    }
}

void M_ChangeSensitivity(int choice)
{
    if (usinggamepad && !M_MSENS)
    {
        switch (choice)
        {
            case 0:
                if (gp_sensitivity > gp_sensitivity_min)
                {
                    if (gp_sensitivity & 1)
                        gp_sensitivity++;

                    gp_sensitivity -= 2;
                    I_SetGamepadSensitivity(gp_sensitivity);
                    C_IntCVAROutput(stringize(gp_sensitivity), gp_sensitivity);
                    M_SliderSound();
                    M_SaveCVARs();
                }

                break;

            case 1:
                if (gp_sensitivity < gp_sensitivity_max)
                {
                    if (gp_sensitivity & 1)
                        gp_sensitivity--;

                    gp_sensitivity += 2;
                    I_SetGamepadSensitivity(gp_sensitivity);
                    C_IntCVAROutput(stringize(gp_sensitivity), gp_sensitivity);
                    M_SliderSound();
                    M_SaveCVARs();
                }

                break;
        }
    }
    else
    {
        switch (choice)
        {
            case 0:
                if (m_sensitivity > m_sensitivity_min)
                {
                    if (m_sensitivity & 1)
                        m_sensitivity++;

                    m_sensitivity -= 2;
                    C_IntCVAROutput(stringize(m_sensitivity), m_sensitivity);
                    M_SliderSound();
                    M_SaveCVARs();
                }

                break;

            case 1:
                if (m_sensitivity < m_sensitivity_max)
                {
                    if (m_sensitivity & 1)
                        m_sensitivity--;

                    m_sensitivity += 2;
                    C_IntCVAROutput("m_sensitivity", m_sensitivity);
                    M_SliderSound();
                    M_SaveCVARs();
                }

                break;
        }
    }
}

void M_ChangeDetail(int choice)
{
    blurred = false;
    r_detail = !r_detail;
    C_StrCVAROutput(stringize(r_detail), (r_detail == r_detail_low ? "low" : "high"));

    if (!menuactive)
    {
        HU_PlayerMessage((r_detail == r_detail_low ? s_DETAILLO : s_DETAILHI), false);
        message_dontfuckwithme = true;
    }
    else
        C_Output(r_detail == r_detail_low ? s_DETAILLO : s_DETAILHI);

    M_SaveCVARs();
}

void M_SizeDisplay(int choice)
{
    switch (choice)
    {
        case 0:
            if (vid_widescreen || (returntowidescreen && gamestate != GS_LEVEL))
            {
                if (!r_hud)
                {
                    r_hud = true;
                    C_StrCVAROutput(stringize(r_hud), "on");
                }
                else if (vid_widescreen)
                {
                    I_ToggleWidescreen(false);
                    C_StrCVAROutput(stringize(vid_widescreen), "off");
                }
                else
                    returntowidescreen = false;

                S_StartSound(NULL, sfx_stnmov);
                M_SaveCVARs();
            }
            else if (r_screensize > r_screensize_min)
            {
                R_SetViewSize(--r_screensize);
                C_IntCVAROutput(stringize(r_screensize), r_screensize);
                S_StartSound(NULL, sfx_stnmov);
                M_SaveCVARs();
            }

            break;

        case 1:
            if (vid_widescreen || (returntowidescreen && gamestate != GS_LEVEL))
            {
                if (r_hud)
                {
                    r_hud = false;
                    C_StrCVAROutput(stringize(r_hud), "off");
                    S_StartSound(NULL, sfx_stnmov);
                    M_SaveCVARs();
                }
            }
            else if (r_screensize == r_screensize_max)
            {
                if (!vid_widescreen)
                {
                    if (gamestate != GS_LEVEL)
                    {
                        returntowidescreen = true;
                        r_hud = true;
                    }
                    else
                    {
                        I_ToggleWidescreen(true);

                        if (vid_widescreen)
                            C_StrCVAROutput(stringize(vid_widescreen), "on");
                        else
                        {
                            R_SetViewSize(++r_screensize);
                            C_IntCVAROutput(stringize(r_screensize), r_screensize);
                        }
                    }
                }

                S_StartSound(NULL, sfx_stnmov);
                M_SaveCVARs();
            }
            else
            {
                R_SetViewSize(++r_screensize);
                C_IntCVAROutput(stringize(r_screensize), r_screensize);
                S_StartSound(NULL, sfx_stnmov);
                M_SaveCVARs();
            }

            break;
    }

    blurred = false;
    skippsprinterp = true;
}

//
// Menu Functions
//
void M_DrawThermo(int x, int y, int thermWidth, float thermDot, float factor, int offset)
{
    int xx;
    int i;

    if (chex || hacx)
    {
        x--;
        y -= 2;
    }

    xx = x;
    M_DrawPatchWithShadow(xx, y, W_CacheLumpName("M_THERML"));
    xx += 8;

    for (i = 0; i < thermWidth; i++)
    {
        V_DrawPatch(xx, y, 0, W_CacheLumpName("M_THERMM"));
        xx += 8;
    }

    M_DrawPatchWithShadow(xx, y, W_CacheLumpName("M_THERMR"));

    for (i = x + 9; i < x + (thermWidth + 1) * 8 + 1; i++)
        V_DrawPixel(i - hacx, y + (hacx ? 9 : 13), 251, true);

    V_DrawPatch(x + offset + (int)(thermDot * factor), y, 0, W_CacheLumpName("M_THERMO"));
}

void M_StartMessage(char *string, void *routine, dboolean input)
{
    messageLastMenuActive = menuactive;
    messageToPrint = true;
    messageString = string;
    messageRoutine = (void (*)(int))routine;
    messageNeedsInput = input;
    blurred = false;
    menuactive = true;
}

//
// Find character width
//
int M_CharacterWidth(char ch, char prev)
{
    int c = toupper(ch) - HU_FONTSTART;

    if (c < 0 || c >= HU_FONTSIZE)
        return (prev == '.' || prev == '!' || prev == '?' ? 5 : 3);
    else
        return (STCFN034 ? SHORT(hu_font[c]->width) : strlen(smallcharset[c]) / 10 - 1);
}

//
// Find string width
//
int M_StringWidth(char *string)
{
    size_t  i;
    int     w = 0;

    for (i = 0; i < strlen(string); i++)
        w += M_CharacterWidth(string[i], (i > 0 ? string[i - 1] : 0));

    return w;
}

//
// Find string height
//
int M_StringHeight(char *string)
{
    size_t  i;
    int     h = 8;

    for (i = 0; i < strlen(string); i++)
        if (string[i] == '\n')
            h += (i > 0 && string[i - 1] == '\n' ? 4 : (STCFN034 ? SHORT(hu_font[0]->height) + 1 : 8));

    return h;
}

//
//  Write a char
//
void M_DrawSmallChar(int x, int y, int i, dboolean shadow)
{
    int w = strlen(smallcharset[i]) / 10;
    int x1, y1;

    for (y1 = 0; y1 < 10; y1++)
        for (x1 = 0; x1 < w; x1++)
            V_DrawPixel(x + x1, y + y1, (int)smallcharset[i][y1 * w + x1], shadow);
}

//
// Write a string
//
void M_WriteText(int x, int y, char *string, dboolean shadow)
{
    int     w;
    char    *ch = string;
    char    letter;
    char    prev = ' ';
    int     cx = x;
    int     cy = y;

    while (1)
    {
        int c = *ch++;

        if (!c)
            break;

        if (c == '\n')
        {
            cx = x;
            cy += 12;
            continue;
        }

        letter = c;
        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE)
        {
            cx += (prev == '.' || prev == '!' || prev == '?' ? 5 : 3);
            prev = letter;
            continue;
        }

        if (STCFN034)
        {
            w = SHORT(hu_font[c]->width);

            if (cx + w > ORIGINALWIDTH)
                break;

            if (shadow)
                M_DrawPatchWithShadow(cx, cy, hu_font[c]);
            else
                V_DrawPatch(cx, cy, 0, hu_font[c]);
        }
        else
        {
            if (prev == ' ')
            {
                if (letter == '\"')
                    c = 64;
                else if (letter == '\'')
                    c = 65;
            }

            w = strlen(smallcharset[c]) / 10 - 1;

            if (cx + w > ORIGINALWIDTH)
                break;

            M_DrawSmallChar(cx, cy, c, shadow);
        }

        prev = letter;
        cx += w;
    }
}

void M_ShowHelp(void)
{
    functionkey = KEY_F1;
    M_StartControlPanel();
    currentMenu = &ReadDef;
    itemOn = 0;
    S_StartSound(NULL, sfx_swtchn);
    inhelpscreens = true;

    if (vid_widescreen)
    {
        I_ToggleWidescreen(false);
        returntowidescreen = true;
    }

    if (!automapactive && gamestate == GS_LEVEL)
        R_SetViewSize(8);
}

void M_ChangeGamma(dboolean shift)
{
    static int  gammawait;

    if (gammawait >= I_GetTime() || gamestate != GS_LEVEL || inhelpscreens)
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

        S_StartSound(NULL, sfx_stnmov);

        if (r_gamma == 1.0f)
            C_StrCVAROutput(stringize(r_gamma), "off");
        else
        {
            static char buf[128];

            M_snprintf(buf, sizeof(buf), "%.2f", r_gamma);

            if (buf[strlen(buf) - 1] == '0' && buf[strlen(buf) - 2] == '0')
                buf[strlen(buf) - 1] = '\0';

            C_StrCVAROutput(stringize(r_gamma), buf);
        }
    }

    gammawait = I_GetTime() + HU_MSGTIMEOUT;

    if (r_gamma == 1.0f)
        HU_PlayerMessage(s_GAMMAOFF, false);
    else
    {
        static char buf[128];

        M_snprintf(buf, sizeof(buf), s_GAMMALVL, r_gamma);

        if (buf[strlen(buf) - 1] == '0' && buf[strlen(buf) - 2] == '0')
            buf[strlen(buf) - 1] = '\0';

        HU_PlayerMessage(buf, false);
    }

    message_dontpause = true;
    message_dontfuckwithme = true;

    I_SetPalette((byte *)W_CacheLumpName("PLAYPAL") + st_palette * 768);
    M_SaveCVARs();
}

//
// CONTROL PANEL
//

//
// M_Responder
//
int         gamepadwait;
int         mousewait;
dboolean    gamepadpress;

dboolean M_Responder(event_t *ev)
{
    // key is the key pressed, ch is the actual character typed
    int         ch = 0;
    int         key = -1;
    int         i;
    static int  keywait;
    SDL_Keymod  modstate = SDL_GetModState();

    if (startingnewgame || wipe)
        return false;

    if (ev->type == ev_gamepad && gamepadwait < I_GetTime())
    {
        if (menuactive)
        {
            // activate menu item
            if (gamepadbuttons & GAMEPAD_A)
            {
                key = (messageToPrint && messageNeedsInput ? (ch = 'y') : KEY_ENTER);
                gamepadwait = I_GetTime() + 8 * !(currentMenu == &OptionsDef && itemOn == 5);
                usinggamepad = true;
            }

            // previous/exit menu
            else if (gamepadbuttons & GAMEPAD_B)
            {
                key = (messageToPrint && messageNeedsInput ? (ch = 'n') : KEY_BACKSPACE);
                gamepadwait = I_GetTime() + 8;
                gamepadpress = true;
                usinggamepad = true;
            }

            // exit menu
            else if (gamepadbuttons & gamepadmenu)
            {
                key = keyboardmenu;
                currentMenu = &MainDef;
                itemOn = MainDef.lastOn;
                gamepadwait = I_GetTime() + 8;
                usinggamepad = true;
            }

            else if (!messageToPrint)
            {
                // select previous menu item
                if (gamepadthumbLY < 0 || (gamepadbuttons & GAMEPAD_DPAD_UP))
                {
                    key = KEY_UPARROW;
                    keywait = 0;
                    gamepadwait = I_GetTime() + 8;
                    usinggamepad = true;
                }

                // select next menu item
                else if (gamepadthumbLY > 0 || (gamepadbuttons & GAMEPAD_DPAD_DOWN))
                {
                    key = KEY_DOWNARROW;
                    keywait = 0;
                    gamepadwait = I_GetTime() + 8;
                    usinggamepad = true;
                }

                // decrease slider
                else if ((gamepadthumbLX < 0 || (gamepadbuttons & GAMEPAD_DPAD_LEFT)) && !saveStringEnter
                    && !(currentMenu == &OptionsDef && itemOn == 1))
                {
                    key = KEY_LEFTARROW;
                    gamepadwait = I_GetTime() + 6 * !(currentMenu == &OptionsDef && itemOn == 5);
                    usinggamepad = true;
                }

                // increase slider
                else if ((gamepadthumbLX > 0 || (gamepadbuttons & GAMEPAD_DPAD_RIGHT)) && !saveStringEnter
                    && !(currentMenu == &OptionsDef && itemOn == 1))
                {
                    key = KEY_RIGHTARROW;
                    gamepadwait = I_GetTime() + 6 * !(currentMenu == &OptionsDef && itemOn == 5);
                    usinggamepad = true;
                }
            }
        }
        else
        {
            // open menu
            if (gamepadbuttons & gamepadmenu)
            {
                key = keyboardmenu;
                gamepadwait = I_GetTime() + 8;
                usinggamepad = true;
            }
        }
    }
    if (ev->type == ev_mouse && mousewait < I_GetTime() && menuactive)
    {
        // activate menu item
        if (ev->data1 & MOUSE_LEFTBUTTON)
        {
            key = KEY_ENTER;
            mousewait = I_GetTime() + 5;
            usinggamepad = false;
        }

        // previous menu
        else if (ev->data1 & MOUSE_RIGHTBUTTON)
        {
            key = KEY_BACKSPACE;
            mousewait = I_GetTime() + 5;
            usinggamepad = false;
        }
    }
    else if (ev->type == ev_mousewheel)
    {
        if (!messageToPrint)
        {
            // select previous menu item
            if (ev->data1 > 0)
            {
                key = KEY_UPARROW;
                mousewait = I_GetTime() + 3;
                usinggamepad = false;
            }

            // select next menu item
            else if (ev->data1 < 0)
            {
                key = KEY_DOWNARROW;
                mousewait = I_GetTime() + 3;
                usinggamepad = false;
            }
        }
    }
    else if (ev->type == ev_keydown)
    {
        key = ev->data1;
        ch = ev->data2;
        usinggamepad = false;
    }
    else if (ev->type == ev_keyup)
        keydown = 0;

    if (key == -1)
        return false;

    // Save Game string input
    if (saveStringEnter)
    {
        switch (key)
        {
            // delete character left of caret
            case KEY_BACKSPACE:
                keydown = key;

                if (saveCharIndex > 0)
                {
                    size_t  j;

                    for (j = saveCharIndex - 1; j < strlen(savegamestrings[saveSlot]); j++)
                        savegamestrings[saveSlot][j] = savegamestrings[saveSlot][j + 1];

                    saveCharIndex--;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            // delete character right of caret
            case KEY_DELETE:
                keydown = key;

                if ((unsigned int)saveCharIndex < strlen(savegamestrings[saveSlot]))
                {
                    size_t  j;

                    for (j = saveCharIndex; j < strlen(savegamestrings[saveSlot]); j++)
                        savegamestrings[saveSlot][j] = savegamestrings[saveSlot][j + 1];

                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            // cancel
            case KEY_ESCAPE:
                if (!keydown)
                {
                    keydown = key;
                    saveStringEnter = 0;
                    caretwait = 0;
                    showcaret = false;
                    M_StringCopy(&savegamestrings[saveSlot][0], saveOldString, SAVESTRINGSIZE);
                    S_StartSound(NULL, sfx_swtchx);
                }

                break;

            // confirm
            case KEY_ENTER:
                if (!keydown)
                {
                    dboolean    allspaces = true;

                    keydown = key;

                    for (i = 0; (unsigned int)i < strlen(savegamestrings[saveSlot]); i++)
                        if (savegamestrings[saveSlot][i] != ' ')
                            allspaces = false;

                    if (savegamestrings[saveSlot][0] && !allspaces)
                    {
                        saveStringEnter = 0;
                        caretwait = I_GetTimeMS() + CARETBLINKTIME;
                        showcaret = true;
                        M_DoSave(saveSlot);
                    }
                }

                break;

            // move caret left
            case KEY_LEFTARROW:
                if (saveCharIndex > 0)
                {
                    saveCharIndex--;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            // move caret right
            case KEY_RIGHTARROW:
                if ((unsigned int)saveCharIndex < strlen(savegamestrings[saveSlot]))
                {
                    saveCharIndex++;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            // move caret to start
            case KEY_HOME:
                if (saveCharIndex > 0)
                {
                    saveCharIndex = 0;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            // move caret to end
            case KEY_END:
                if ((unsigned int)saveCharIndex < strlen(savegamestrings[saveSlot]))
                {
                    saveCharIndex = strlen(savegamestrings[saveSlot]);
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }

                break;

            default:
                ch = toupper(ch);

                if (ch >= ' ' && ch <= '_' && M_StringWidth(savegamestrings[saveSlot])
                    + M_CharacterWidth(ch, 0) <= SAVESTRINGPIXELWIDTH
                    && !(modstate & (KMOD_ALT | KMOD_CTRL)))
                {
                    keydown = key;
                    savegamestrings[saveSlot][strlen(savegamestrings[saveSlot]) + 1] = '\0';

                    for (i = strlen(savegamestrings[saveSlot]); i > saveCharIndex; i--)
                        savegamestrings[saveSlot][i] = savegamestrings[saveSlot][i - 1];

                    savegamestrings[saveSlot][saveCharIndex++] = ch;
                    caretwait = I_GetTimeMS() + CARETBLINKTIME;
                    showcaret = true;
                }
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint && !keydown)
    {
        ch = (key == KEY_ENTER ? 'y' : tolower(ch));

        if (messageNeedsInput && key != keyboardmenu && ch != 'y' && ch != 'n'
            && !(modstate & (KMOD_ALT | KMOD_CTRL)) && key != functionkey)
        {
            functionkey = 0;
            return false;
        }

        keydown = key;
        menuactive = messageLastMenuActive;
        messageToPrint = false;

        if (messageRoutine)
            messageRoutine(ch);

        functionkey = 0;

        if (endinggame)
            endinggame = false;
        else
            S_StartSound(NULL, (currentMenu == &ReadDef ? sfx_pistol : sfx_swtchx));

        return true;
    }

    // F-Keys
    if ((!menuactive || functionkey) && !paused && !splashscreen)
    {

        // Screen size down
        if (key == KEY_MINUS)
        {
            keydown = key;

            if (automapactive || !viewactive || inhelpscreens)
                return false;

            M_SizeDisplay(0);
            return false;
        }

        // Screen size up
        else if (key == KEY_EQUALS)
        {
            keydown = key;

            if (automapactive || !viewactive || inhelpscreens)
                return false;

            M_SizeDisplay(1);
            return false;
        }

        // Console
        else if (key == keyboardconsole && !keydown)
        {
            keydown = key;

            if (consoleheight < CONSOLEHEIGHT && consoledirection == -1 && !inhelpscreens && !wipe)
            {
                C_ShowConsole();
                return true;
            }

            return false;
        }

        // Help key
        else if (key == KEY_F1 && (!functionkey || functionkey == KEY_F1) && !keydown)
        {
            keydown = key;

            if (functionkey == KEY_F1)
            {
                functionkey = 0;
                M_ClearMenus();
                S_StartSound(NULL, sfx_swtchx);

                if (inhelpscreens)
                {
                    R_SetViewSize(r_screensize);

                    if (returntowidescreen && gamestate == GS_LEVEL)
                        I_ToggleWidescreen(true);
                }
            }
            else
                M_ShowHelp();

            return false;
        }

        // Save
        else if (key == KEY_F2 && (!functionkey || functionkey == KEY_F2) && (viewactive || automapactive)
            && !keydown && players[0].health > 0)
        {
            keydown = key;

            if (functionkey == KEY_F2)
            {
                functionkey = 0;
                currentMenu->lastOn = itemOn;
                M_ClearMenus();
                S_StartSound(NULL, sfx_swtchx);
            }
            else
            {
                functionkey = KEY_F2;
                M_StartControlPanel();
                itemOn = currentMenu->lastOn;
                S_StartSound(NULL, sfx_swtchn);
                M_SaveGame(0);
            }

            return false;
        }

        // Load
        else if (key == KEY_F3 && (!functionkey || functionkey == KEY_F3) && savegames && !keydown)
        {
            keydown = key;

            if (functionkey == KEY_F3)
            {
                functionkey = 0;
                currentMenu->lastOn = itemOn;
                M_ClearMenus();
                S_StartSound(NULL, sfx_swtchx);
            }
            else
            {
                functionkey = KEY_F3;
                M_StartControlPanel();
                itemOn = currentMenu->lastOn;
                S_StartSound(NULL,sfx_swtchn);
                M_LoadGame(0);
            }

            return false;
        }

        else if (key == KEY_F4 && (!functionkey || functionkey == KEY_F4 || (modstate & KMOD_ALT))
            && !keydown)
        {
            keydown = key;

            // Quit DOOM
            if (modstate & KMOD_ALT)
            {
                S_StartSound(NULL, sfx_swtchn);
                M_QuitResponse('y');
                return false;
            }
            else
            {
                // Sound Volume
                if (functionkey == KEY_F4)
                {
                    functionkey = 0;
                    currentMenu->lastOn = itemOn;
                    M_ClearMenus();
                    S_StartSound(NULL, sfx_swtchx);
                }
                else
                {
                    functionkey = KEY_F4;
                    M_StartControlPanel();
                    currentMenu = &SoundDef;
                    itemOn = currentMenu->lastOn;
                    S_StartSound(NULL, sfx_swtchn);
                }

                return false;
            }
        }

        // Toggle graphic detail
        else if (key == KEY_F5 && !functionkey && viewactive && !keydown)
        {
            keydown = key;
            functionkey = KEY_F5;
            M_ChangeDetail(0);
            functionkey = 0;
            S_StartSound(NULL, sfx_swtchn);
            return false;
        }

        // Quicksave
        else if (key == KEY_F6 && (!functionkey || functionkey == KEY_F6) && (viewactive || automapactive)
            && !keydown && players[0].health > 0)
        {
            keydown = key;

            if (quickSaveSlot >= 0)
                functionkey = KEY_F6;

            M_QuickSave();
            return false;
        }

        // End game
        else if (key == KEY_F7 && !functionkey && (viewactive || automapactive) && !keydown)
        {
            keydown = key;
            functionkey = KEY_F7;
            S_StartSound(NULL, sfx_swtchn);
            M_EndGame(0);
            return false;
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
            M_QuickLoad();
            return false;
        }

        // Quit DOOM
        else if (key == KEY_F10 && !keydown)
        {
            keydown = key;
            functionkey = KEY_F10;
            S_StartSound(NULL, sfx_swtchn);
            M_QuitDOOM(0);
            return false;
        }
    }

    // gamma toggle
    if (key == KEY_F11)
    {
        M_ChangeGamma(modstate & KMOD_SHIFT);
        return false;
    }

    // screenshot
    if (key == keyboardscreenshot)
    {
#if defined(_WIN32)
        if (key != KEY_PRINTSCREEN)
#endif
            G_ScreenShot();
        return false;
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == keyboardmenu && !keydown && !splashscreen && !consoleheight)
        {
            keydown = key;

            if (paused)
            {
                paused = false;
                S_ResumeSound();
                S_StartSound(NULL, sfx_swtchx);
            }
            else
            {
                M_StartControlPanel();
                S_StartSound(NULL, sfx_swtchn);
            }
        }
        return false;
    }

    if (!paused)
    {
        if (key == KEY_DOWNARROW && keywait < I_GetTime() && !inhelpscreens)
        {
            // Move down to next item
            if (currentMenu == &LoadDef)
            {
                int old = itemOn;

                do
                {
                    if (itemOn + 1 > currentMenu->numitems - 1)
                        itemOn = 0;
                    else
                        itemOn++;
                }
                while (M_StringCompare(savegamestrings[itemOn], s_EMPTYSTRING));

                if (itemOn != old)
                    S_StartSound(NULL, sfx_pstop);

                SaveDef.lastOn = savegameselected = itemOn;
                M_SaveCVARs();
            }
            else
            {
                do
                {
                    if (itemOn + 1 > currentMenu->numitems - 1)
                        itemOn = 0;
                    else
                        itemOn++;

                    if (currentMenu == &MainDef && itemOn == 2 && !savegames)
                        itemOn++;

                    if (currentMenu == &MainDef && itemOn == 3
                        && (gamestate != GS_LEVEL || players[0].health <= 0))
                        itemOn++;

                    if (currentMenu == &OptionsDef && !itemOn && gamestate != GS_LEVEL)
                        itemOn++;

                    if (currentMenu->menuitems[itemOn].status != -1)
                        S_StartSound(NULL, sfx_pstop);
                }
                while (currentMenu->menuitems[itemOn].status == -1);
            }

            if (currentMenu == &EpiDef && gamemode != shareware)
            {
                episodeselected = itemOn;
                M_SaveCVARs();
            }
            else if (currentMenu == &ExpDef)
            {
                expansionselected = itemOn;
                M_SaveCVARs();
            }
            else if (currentMenu == &NewDef)
            {
                skilllevelselected = itemOn;
                M_SaveCVARs();
            }
            else if (currentMenu == &SaveDef)
            {
                LoadDef.lastOn = savegameselected = itemOn;
                M_SaveCVARs();
            }
            keywait = I_GetTime() + 2;
            M_SetWindowCaption();
            return false;
        }
        else if (key == KEY_UPARROW && keywait < I_GetTime() && !inhelpscreens)
        {
            // Move back up to previous item
            if (currentMenu == &LoadDef)
            {
                int old = itemOn;

                do
                {
                    if (!itemOn)
                        itemOn = currentMenu->numitems - 1;
                    else
                        itemOn--;
                }
                while (M_StringCompare(savegamestrings[itemOn], s_EMPTYSTRING));

                if (itemOn != old)
                    S_StartSound(NULL, sfx_pstop);

                SaveDef.lastOn = savegameselected = itemOn;
                M_SaveCVARs();
            }
            else
            {
                do
                {
                    if (!itemOn)
                        itemOn = currentMenu->numitems - 1;
                    else
                        itemOn--;

                    if (currentMenu == &MainDef && itemOn == 3
                        && (gamestate != GS_LEVEL || players[0].health <= 0))
                        itemOn--;

                    if (currentMenu == &MainDef && itemOn == 2 && !savegames)
                        itemOn--;

                    if (currentMenu == &OptionsDef && !itemOn && gamestate != GS_LEVEL)
                        itemOn = currentMenu->numitems - 1;

                    if (currentMenu->menuitems[itemOn].status != -1)
                        S_StartSound(NULL, sfx_pstop);
                }
                while (currentMenu->menuitems[itemOn].status == -1);
            }

            if (currentMenu == &EpiDef && gamemode != shareware)
            {
                episodeselected = itemOn;
                M_SaveCVARs();
            }
            else if (currentMenu == &ExpDef)
            {
                expansionselected = itemOn;
                M_SaveCVARs();
            }
            else if (currentMenu == &NewDef)
            {
                skilllevelselected = itemOn;
                M_SaveCVARs();
            }
            else if (currentMenu == &SaveDef)
            {
                LoadDef.lastOn = savegameselected = itemOn;
                M_SaveCVARs();
            }

            keywait = I_GetTime() + 2;
            M_SetWindowCaption();
            return false;
        }

        else if ((key == KEY_LEFTARROW || (key == KEY_MINUS && !(currentMenu == &OptionsDef && itemOn == 1)))
            && !inhelpscreens)
        {
            // Slide slider left
            if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
                currentMenu->menuitems[itemOn].routine(0);
            else if (currentMenu == &OptionsDef && (itemOn == 1 || itemOn == 2) && !keydown)
            {
                keydown = key;
                currentMenu->menuitems[itemOn].routine(itemOn);
                S_StartSound(NULL, sfx_pistol);
            }

            return false;
        }

        else if ((key == KEY_RIGHTARROW || (key == KEY_EQUALS && !(currentMenu == &OptionsDef
            && itemOn == 1))) && !inhelpscreens)
        {
            // Slide slider right
            if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
                currentMenu->menuitems[itemOn].routine(1);
            else if (currentMenu == &OptionsDef && (itemOn == 1 || itemOn == 2) && !keydown)
            {
                keydown = key;
                currentMenu->menuitems[itemOn].routine(itemOn);
                S_StartSound(NULL, sfx_pistol);
            }

            return false;
        }

        else if (key == KEY_ENTER && !keydown)
        {
            // Activate menu item
            keydown = key;
            if (inhelpscreens)
            {
                functionkey = 0;
                M_ClearMenus();
                S_StartSound(NULL, sfx_swtchx);
                R_SetViewSize(r_screensize);

                if (returntowidescreen && gamestate == GS_LEVEL)
                    I_ToggleWidescreen(true);

                return true;
            }

            if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status)
            {
                if (gamemode != shareware || currentMenu != &EpiDef)
                    currentMenu->lastOn = itemOn;

                if (currentMenu->menuitems[itemOn].status == 2)
                    currentMenu->menuitems[itemOn].routine(1);
                else
                {
                    if (gamestate != GS_LEVEL && currentMenu == &MainDef && itemOn == 3)
                        return true;

                    if (gamestate != GS_LEVEL && currentMenu == &OptionsDef && !itemOn)
                        return true;

                    if (currentMenu != &LoadDef && (currentMenu != &NewDef || itemOn == 4))
                        S_StartSound(NULL, sfx_pistol);

                    currentMenu->menuitems[itemOn].routine(itemOn);
                }
            }

            M_SetWindowCaption();
            skipaction = (currentMenu == &LoadDef || currentMenu == &SaveDef);
            return skipaction;
        }

        else if ((key == keyboardmenu || key == KEY_BACKSPACE) && !keydown)
        {
            // Deactivate menu or go back to previous menu
            keydown = key;

            if (gamemode != shareware || currentMenu != &EpiDef)
                currentMenu->lastOn = itemOn;

            if (currentMenu->prevMenu && !functionkey)
            {
                currentMenu = currentMenu->prevMenu;
                itemOn = currentMenu->lastOn;
                S_StartSound(NULL, sfx_swtchn);
            }
            else
            {
                functionkey = 0;
                M_ClearMenus();
                S_StartSound(NULL, sfx_swtchx);
                gamepadbuttons = 0;
                ev->data1 = 0;
                firstevent = true;
            }

            if (inhelpscreens)
            {
                R_SetViewSize(r_screensize);

                if (returntowidescreen && gamestate == GS_LEVEL)
                    I_ToggleWidescreen(true);
            }

            M_SetWindowCaption();
            return true;
        }

        else if (key == KEY_DELETE && !keydown && (currentMenu == &LoadDef || currentMenu == &SaveDef))
        {
            // Delete a savegame
            keydown = key;

            if (LoadGameMenu[itemOn].status)
            {
                M_DeleteSavegame();
                return true;
            }
            else
                S_StartSound(NULL, sfx_oof);

            return false;
        }

        // Keyboard shortcut?
        else if (ch != 0 && !(modstate & (KMOD_ALT | KMOD_CTRL)))
        {
            for (i = itemOn + 1; i < currentMenu->numitems; i++)
            {
                if (((currentMenu == &LoadDef || currentMenu == &SaveDef) && ch == i + '1')
                    || (currentMenu->menuitems[i].text
                        && toupper(*currentMenu->menuitems[i].text[0]) == toupper(ch)))
                {
                    if (currentMenu == &MainDef && i == 3
                        && (gamestate != GS_LEVEL || players[0].health <= 0))
                        return true;

                    if (currentMenu == &MainDef && i == 2 && !savegames)
                        return true;

                    if (currentMenu == &OptionsDef && !i && gamestate != GS_LEVEL)
                        return true;

                    if (currentMenu == &LoadDef && M_StringCompare(savegamestrings[i], s_EMPTYSTRING))
                        return true;

                    if (itemOn != i)
                        S_StartSound(NULL, sfx_pstop);

                    itemOn = i;

                    if (currentMenu == &EpiDef && gamemode != shareware)
                    {
                        episodeselected = itemOn;
                        M_SaveCVARs();
                    }
                    else if (currentMenu == &ExpDef)
                    {
                        expansionselected = itemOn;
                        M_SaveCVARs();
                    }
                    else if (currentMenu == &NewDef)
                    {
                        skilllevelselected = itemOn;
                        M_SaveCVARs();
                    }
                    else if (currentMenu == &SaveDef)
                    {
                        LoadDef.lastOn = savegameselected = itemOn;
                        M_SaveCVARs();
                    }
                    else if (currentMenu == &LoadDef)
                    {
                        SaveDef.lastOn = savegameselected = itemOn;
                        M_SaveCVARs();
                    }

                    M_SetWindowCaption();
                    return false;
                }
            }

            for (i = 0; i <= itemOn; i++)
            {
                if (((currentMenu == &LoadDef || currentMenu == &SaveDef) && ch == i + '1')
                    || (currentMenu->menuitems[i].text
                        && toupper(*currentMenu->menuitems[i].text[0]) == toupper(ch)))
                {
                    if (currentMenu == &MainDef && i == 3
                        && (gamestate != GS_LEVEL || players[0].health <= 0))
                        return true;

                    if (currentMenu == &MainDef && i == 2 && !savegames)
                        return true;

                    if (currentMenu == &OptionsDef && !i && gamestate != GS_LEVEL)
                        return true;

                    if (currentMenu == &LoadDef && M_StringCompare(savegamestrings[i], s_EMPTYSTRING))
                        return true;

                    if (itemOn != i)
                        S_StartSound(NULL, sfx_pstop);

                    itemOn = i;

                    if (currentMenu == &EpiDef && gamemode != shareware)
                    {
                        episodeselected = itemOn;
                        M_SaveCVARs();
                    }
                    else if (currentMenu == &ExpDef)
                    {
                        expansionselected = itemOn;
                        M_SaveCVARs();
                    }
                    else if (currentMenu == &NewDef)
                    {
                        skilllevelselected = itemOn;
                        M_SaveCVARs();
                    }
                    else if (currentMenu == &SaveDef)
                    {
                        LoadDef.lastOn = savegameselected = itemOn;
                        M_SaveCVARs();
                    }
                    else if (currentMenu == &LoadDef)
                    {
                        SaveDef.lastOn = savegameselected = itemOn;
                        M_SaveCVARs();
                    }

                    M_SetWindowCaption();
                    return false;
                }
            }
        }
    }
    return false;
}

//
// M_StartControlPanel
//
void M_StartControlPanel(void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;

    menuactive = true;
    currentMenu = &MainDef;
    itemOn = currentMenu->lastOn;
    blurred = false;

    S_StopSounds();

    if ((gp_vibrate_damage || gp_vibrate_weapons) && vibrate)
    {
        restoremotorspeed = idlemotorspeed;
        idlemotorspeed = 0;
        XInputVibration(idlemotorspeed);
    }

    players[0].fixedcolormap = 0;
    I_SetPalette(W_CacheLumpName("PLAYPAL"));
    I_UpdateBlitFunc(false);

    if (vid_motionblur)
        I_SetMotionBlur(0);
}

//
// M_DrawNightmare
//
void M_DrawNightmare(void)
{
    int x, y;

    for (y = 0; y < 20; y++)
        for (x = 0; x < 124; x++)
            V_DrawPixel(NewDef.x + x, NewDef.y + OFFSET + 16 * nightmare + y, (int)nmare[y * 124 + x], true);
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
    static short    x, y;
    unsigned int    i;
    unsigned int    max;

    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        char    string[80];
        int     start = 0;

        M_DarkBackground();

        if (vid_widescreen)
            y = viewwindowy / 2 + (viewheight / 2 - M_StringHeight(messageString)) / 2 - 1;
        else
            y = (ORIGINALHEIGHT - M_StringHeight(messageString)) / 2 - 1;

        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start, sizeof(string));

                    if (i < sizeof(string))
                        string[i] = '\0';

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start, sizeof(string));
                start += strlen(string);
            }

            x = (ORIGINALWIDTH - M_StringWidth(string)) / 2;

            if (!M_StringWidth(string))
                y -= 4;

            M_WriteText(x, y, string, true);
            y += SHORT(hu_font[0]->height) + 1;
        }

        return;
    }

    if (!menuactive)
    {
        inhelpscreens = false;
        return;
    }

    if (currentMenu->routine)
        currentMenu->routine();         // call Draw routine

    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    for (i = 0; i < max; i++)
    {
        char    *name = currentMenu->menuitems[i].name;

        if (*name)
        {
            if (!strcmp(name, "M_NMARE"))
            {
                if (M_NMARE)
                    M_DrawPatchWithShadow(x, y + OFFSET, W_CacheLumpName(name));
                else
                    M_DrawNightmare();
            }
            else if (!strcmp(name, "M_MSENS") && !M_MSENS)
                M_DrawString(x, y + OFFSET,
                    (usinggamepad ? s_M_GAMEPADSENSITIVITY : s_M_MOUSESENSITIVITY));
            else if (W_CheckMultipleLumps(name) > 1)
                M_DrawPatchWithShadow(x, y + OFFSET, W_CacheLumpName(name));
            else
                M_DrawString(x, y + OFFSET, *currentMenu->menuitems[i].text);
        }

        y += LINEHEIGHT - 1;
    }

    // DRAW SKULL
    if (currentMenu == &LoadDef || currentMenu == &SaveDef)
    {
        patch_t *patch = W_CacheLumpName(skullName[whichSkull]);

        if (currentMenu == &LoadDef)
        {
            int old = itemOn;

            while (M_StringCompare(savegamestrings[itemOn], s_EMPTYSTRING))
                itemOn = (!itemOn ? currentMenu->numitems - 1 : itemOn - 1);

            if (itemOn != old)
            {
                SaveDef.lastOn = savegameselected = itemOn;
                M_SaveCVARs();
            }
        }

        if (M_SKULL1)
            M_DrawPatchWithShadow(x - 43, currentMenu->y + itemOn * 17 - 8 + OFFSET + chex, patch);
        else
            M_DrawPatchWithShadow(x - 37, currentMenu->y + itemOn * 17 - 7 + OFFSET, patch);
    }
    else if (currentMenu != &ReadDef)
    {
        patch_t *patch = W_CacheLumpName(skullName[whichSkull]);
        int     y = currentMenu->y + itemOn * 16 - 5 + OFFSET + chex;

        if (currentMenu == &OptionsDef && !itemOn && gamestate != GS_LEVEL)
            itemOn++;

        if (M_SKULL1)
            M_DrawPatchWithShadow(x - 32, y, patch);
        else
            M_DrawPatchWithShadow(x - 26, y + 2, patch);
    }
}

//
// M_ClearMenus
//
void M_ClearMenus(void)
{
    menuactive = false;

    if ((gp_vibrate_damage || gp_vibrate_weapons) && vibrate)
    {
        idlemotorspeed = restoremotorspeed;
        XInputVibration(idlemotorspeed);
    }

    if (gamestate == GS_LEVEL)
        I_SetPalette((byte *)W_CacheLumpName("PLAYPAL") + st_palette * 768);
}

//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}

//
// M_Ticker
//
void M_Ticker(void)
{
    if ((!saveStringEnter || !whichSkull) && windowfocused)
        if (--skullAnimCounter <= 0)
        {
            whichSkull ^= 1;
            skullAnimCounter = 8;
        }
}

//
// M_Init
//
void M_Init(void)
{
    currentMenu = &MainDef;
    menuactive = false;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    messageToPrint = false;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;
    tempscreen1 = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    tempscreen2 = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    blurscreen1 = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    blurscreen2 = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    if (autostart)
    {
        episodeselected = startepisode - 1;
        skilllevelselected = startskill;
    }

    if (gamemode != shareware)
        EpiDef.lastOn = episodeselected;

    ExpDef.lastOn = expansionselected;
    NewDef.lastOn = skilllevelselected;
    SaveDef.lastOn = LoadDef.lastOn = savegameselected;
    OptionsDef.lastOn = msgs;
    M_ReadSaveStrings();

    if (chex)
    {
        MainDef.x += 20;
        NewDef.prevMenu = &MainDef;
    }
    else if (gamemode == commercial)
        NewDef.prevMenu = (nerve ? &ExpDef : &MainDef);
    else if (gamemode == registered || W_CheckNumForName("E4M1") < 0)
        EpiDef.numitems--;

#if !defined(_WIN32)
    s_DOSY = s_OTHERY;
    s_DOSA = s_OTHERA;
#endif

    if (M_StringCompare(s_EMPTYSTRING, "null data"))
        s_EMPTYSTRING = "-";
}
