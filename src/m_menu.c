/*
====================================================================

DOOM RETRO
A classic, refined DOOM source port. For Windows PC.

Copyright © 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright © 2005-2014 Simon Howard.
Copyright © 2013-2014 Brad Harding.

This file is part of DOOM RETRO.

DOOM RETRO is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DOOM RETRO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DOOM RETRO. If not, see http://www.gnu.org/licenses/.

====================================================================
*/

#include "d_main.h"
#include "doomstat.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_random.h"
#include "p_local.h"
#include "p_saveg.h"
#include "s_sound.h"
#include "SDL.h"
#include "v_data.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

extern patch_t *hu_font[HU_FONTSIZE];
extern boolean message_dontfuckwithme;

extern boolean chat_on;                // in heads-up code

extern int     st_palette;

extern boolean wipe;
extern boolean widescreenhud;

//
// defaulted values
//
int            mouseSensitivity = MOUSESENSITIVITY_DEFAULT;
float          gamepadSensitivity;

// Show messages has default, 0 = off, 1 = on
int            showMessages = 0;

int            screenblocks = SCREENBLOCKS_DEFAULT;

// temp for screenblocks (0-9)
int            screenSize;

// -1 = no quicksave slot picked!
int            quickSaveSlot;

// 1 = message to be printed
int            messageToPrint;
// ...and here is the message string!
char           *messageString;

int            messageLastMenuActive;

// timed message = no input from user
boolean        messageNeedsInput;

void (*messageRoutine)(int response);

//char gammamsg[GAMMALEVELS][30] =
//{
//    GAMMALVL0,
//    GAMMALVL1,
//    GAMMALVL2,
//    GAMMALVL3,
//    GAMMALVL4,
//    GAMMALVL5,
//    GAMMALVL6,
//    GAMMALVL7,
//    GAMMALVL8
//};

// we are going to be entering a savegame string
int            saveStringEnter;
int            saveSlot;               // which slot to save in
int            saveCharIndex;          // which char we're editing
// old save description before edit
char           saveOldString[SAVESTRINGSIZE];

boolean        inhelpscreens;
boolean        menuactive;
boolean        savegames = false;
boolean        startingnewgame = false;

#define SKULLXOFF  -32
#define LINEHEIGHT 17
#define OFFSET     (!usergame ? 17 : 0)

char           savegamestrings[10][SAVESTRINGSIZE];

char           endstring[160];

short          itemOn;                 // menu item skull is on
short          skullAnimCounter;       // skull animation counter
short          whichSkull;             // which skull to draw

int            selectedskilllevel = 2;
int            selectedepisode = 0;
int            selectedexpansion = 0;
int            selectedsavegame = 0;

static int     functionkey = 0;

static boolean usinggamepad = false;

// graphic name of skulls
char           *skullName[2] = { "M_SKULL1", "M_SKULL2" };

// current menudef
menu_t         *currentMenu;

byte           *tempscreen;
byte           *blurredscreen;

boolean        menublur = true;
boolean        blurred = false;

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
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
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
void M_DrawThermo(int x, int y, int thermWidth, float thermDot, float factor);
void M_WriteText(int x, int y, char *string, boolean shadow);
int M_StringWidth(char *string);
int M_StringHeight(char *string);
void M_StartMessage(char *string, void *routine, boolean input);

//
// DOOM MENU
//

enum
{
    newgame = 0,
    options,
    loadgame,
    savegame,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[] =
{
    { 1, "M_NGAME",  M_NewGame,  'n', "New Game"   },
    { 1, "M_OPTION", M_Options,  'o', "Options"    },
    { 1, "M_LOADG",  M_LoadGame, 'l', "Load Game"  },
    { 1, "M_SAVEG",  M_SaveGame, 's', "Save Game"  },
    { 1, "M_QUITG",  M_QuitDOOM, 'q', "Quit Game"  }
};

menu_t MainDef =
{
    main_end,           // # of menu items
    NULL,               // previous menu
    MainMenu,           // menuitem_t ->
    M_DrawMainMenu,     // drawing routine ->
    98, 77,             // x, y
    0                   // lastOn
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
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[] =
{
    { 1, "M_EPI1", M_Episode, 'k', "Knee-Deep in the Dead" },
    { 1, "M_EPI2", M_Episode, 't', "The Shores of Hell"    },
    { 1, "M_EPI3", M_Episode, 'i', "Inferno"               },
    { 1, "M_EPI4", M_Episode, 't', "Thy Flesh Consumed"    }
};

menu_t EpiDef =
{
    ep_end,             // # of menu items
    &MainDef,           // previous menu
    EpisodeMenu,        // menuitem_t ->
    M_DrawEpisode,      // drawing routine ->
    39, 63,             // x, y
    ep1                 // lastOn
};

//
// EXPANSION SELECT
//

enum
{
    exp1,
    exp2,
    exp_end
} expansions_e;

menuitem_t ExpansionMenu[] =
{
    { 1, "M_EPI1", M_Expansion, 'h', "Hell on Earth"          },
    { 1, "M_EPI2", M_Expansion, 'n', "No Rest for the Living" }
};

menu_t ExpDef =
{
    exp_end,              // # of menu items
    &MainDef,             // previous menu
    ExpansionMenu,        // menuitem_t ->
    M_DrawExpansion,      // drawing routine ->
    39, 63,               // x, y
    exp1                  // lastOn
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
} newgame_e;

menuitem_t NewGameMenu[] =
{
    { 1, "M_JKILL", M_ChooseSkill, 'i', "I\'m too young to die." },
    { 1, "M_ROUGH", M_ChooseSkill, 'h', "Hey, not too rough."    },
    { 1, "M_HURT",  M_ChooseSkill, 'h', "Hurt me plenty."        },
    { 1, "M_ULTRA", M_ChooseSkill, 'u', "Ultra-Violence."        },
    { 1, "M_NMARE", M_ChooseSkill, 'n', "Nightmare!"             }
};

menu_t NewDef =
{
    newg_end,           // # of menu items
    &EpiDef,            // previous menu
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    39, 63,             // x, y
    hurtme              // lastOn
};

//
// OPTIONS MENU
//

enum
{
    endgame,
    messages,
    scrnsize,
    option_empty1,
    mousesens,
    option_empty2,
    soundvol,
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {  1, "M_ENDGAM", M_EndGame,           'e', "End Game"          },
    {  1, "M_MESSG",  M_ChangeMessages,    'm', "Messages:"         },
    {  2, "M_SCRNSZ", M_SizeDisplay,       's', "Screen Size"       },
    { -1, "",         0,                   0,   ""                  },
    {  2, "M_MSENS",  M_ChangeSensitivity, 'm', "Mouse Sensitivity" },
    { -1, "",         0,                   0,   ""                  },
    {  1, "M_SVOL",   M_Sound,             's', "Sound Volume"      }
};

menu_t OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    56, 41,
    0
};

enum
{
    rdthsempty,
    read_end
} read_e;

menuitem_t ReadMenu[] =
{
    { 1, "", M_FinishReadThis, 0, "" }
};

menu_t ReadDef =
{
    read_end,
    &ReadDef,
    ReadMenu,
    M_DrawReadThis,
    330, 175,
    0
};

//
// SOUND VOLUME MENU
//

enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_end
} sound_e;

menuitem_t SoundMenu[] =
{
    {  2, "M_SFXVOL", M_SfxVol,   's', "SFX Volume"   },
    { -1, "",         0,          0,   ""             },
    {  2, "M_MUSVOL", M_MusicVol, 'm', "Music Volume" },
    { -1, "",         0,          0,   ""             }
};

menu_t SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    89, 64,
    0
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
    load_end
} load_e;

menuitem_t LoadMenu[] =
{
    { 1, "", M_LoadSelect, '1', "" },
    { 1, "", M_LoadSelect, '2', "" },
    { 1, "", M_LoadSelect, '3', "" },
    { 1, "", M_LoadSelect, '4', "" },
    { 1, "", M_LoadSelect, '5', "" },
    { 1, "", M_LoadSelect, '6', "" }
};

menu_t LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    67, 51,
    0
};

//
// SAVE GAME MENU
//

menuitem_t SaveMenu[] =
{
    { 1, "", M_SaveSelect, '1', "" },
    { 1, "", M_SaveSelect, '2', "" },
    { 1, "", M_SaveSelect, '3', "" },
    { 1, "", M_SaveSelect, '4', "" },
    { 1, "", M_SaveSelect, '5', "" },
    { 1, "", M_SaveSelect, '6', "" }
};

menu_t SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    67, 51,
    0
};

int height;

__inline static void blur(int x1, int y1, int x2, int y2, int i)
{
    int x, y;

    memcpy(tempscreen, blurredscreen, SCREENWIDTH * SCREENHEIGHT);

    for (y = y1; y < y2; y += SCREENWIDTH)
        for (x = y + x1; x < y + x2; x++)
            blurredscreen[x] = tinttab50[tempscreen[x] + (tempscreen[x + i] << 8)];
}

//
// M_DarkBackground
//  darken and blur background while menu is displayed
//
void M_DarkBackground(void)
{
    int i, j;

    height = (SCREENHEIGHT - widescreen * SBARHEIGHT) * SCREENWIDTH;

    if (!blurred)
    {
        memcpy(blurredscreen, screens[0], SCREENWIDTH * SCREENHEIGHT);

        if (menublur)
        {
            blur(0, 0, SCREENWIDTH - 1, height, 1);
            blur(1, 0, SCREENWIDTH, height, -1);
            blur(0, 0, SCREENWIDTH - 1, height - SCREENWIDTH, SCREENWIDTH + 1);
            blur(1, SCREENWIDTH, SCREENWIDTH, height, -(SCREENWIDTH + 1));
            blur(0, 0, SCREENWIDTH, height - SCREENWIDTH, SCREENWIDTH);
            blur(0, SCREENWIDTH, SCREENWIDTH, height, -SCREENWIDTH);
            blur(1, 0, SCREENWIDTH, height - SCREENWIDTH, SCREENWIDTH - 1);
            blur(0, SCREENWIDTH, SCREENWIDTH - 1, height, -(SCREENWIDTH - 1));

            if (fullscreen && !widescreen)
                for (i = 0, j = SCREENWIDTH - 1; i < height; i += SCREENWIDTH, j += SCREENWIDTH)
                {
                    blurredscreen[i] = tinttab50[blurredscreen[i]];
                    blurredscreen[i + 1] = tinttab50[blurredscreen[i] + (blurredscreen[i + 1] << 8)];
                    blurredscreen[j] = tinttab50[blurredscreen[j]];
                    blurredscreen[j - 1] = tinttab50[blurredscreen[j] + (blurredscreen[j - 1] << 8)];
                }

            blurred = true;
        }
    }

    i = 0;
    while (i < height)
        screens[0][i] = tinttab50[blurredscreen[i++]];
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
    int         x, y;

    for (y = 0; y < SCREENWIDTH * SCREENHEIGHT; y += SCREENWIDTH * 2)
        for (x = y; x < y + SCREENWIDTH; x += 2)
        {
            byte        *dot = *screens + x;
            byte        *copy;

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
void M_DrawChar(int x, int y, int i, boolean overlapping)
{
    int x1, y1;
    int w = strlen(redcharset[i]) / 18;

    for (y1 = 0; y1 < 18; y1++)
        for (x1 = 0; x1 < w; x1++)
        {
            char dot = redcharset[i][y1 * w + x1];

            if (dot == 'È')
            {
                if (!overlapping)
                    V_DrawPixel(x + x1, y + y1, 0, 251, true);
            }
            else
                V_DrawPixel(x + x1, y + y1, 0, (int)dot, true);
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
    char char1;
    char char2;
    int adjust;
} kern[] = {
    { '-', 'V', -2 }, { 'O', 'A', -1 }, { 'P', 'a', -3 }, { 'V', 'o', -2 },
    { 'f', 'e', -1 }, { 'f', 'f', -1 }, { 'f', 'o', -1 }, { 'l', 'e', -1 },
    { 'l', 't', -1 }, { 'l', 'u', -1 }, { 'o', 'a', -1 }, { 'o', 't', -1 },
    { 'p', 'a', -2 }, { 't', 'o', -1 }, { 'v', 'e', -1 }, { 'y', ',', -3 },
    { 'y', '.', -2 }, { 'y', 'o', -1 }, {  0,   0,   0 }
};

static struct
{
    char char1;
    char char2;
} overlap[] = {
    { 'A', 'D' }, { 'A', 'M' }, { 'E', 'a' }, { 'E', 'n' }, { 'E', 'p' },
    { 'E', 'x' }, { 'G', 'A' }, { 'G', 'a' }, { 'I', 'n' }, { 'K', 'n' },
    { 'L', 'i' }, { 'a', 'd' }, { 'a', 'm' }, { 'a', 'n' }, { 'a', 'r' },
    { 'c', 'h' }, { 'c', 'r' }, { 'e', 'a' }, { 'e', 'd' }, { 'e', 'n' },
    { 'e', 'p' }, { 'e', 'r' }, { 'e', 's' }, { 'g', 'h' }, { 'h', 'i' },
    { 'i', 'n' }, { 'i', 's' }, { 'i', 'z' }, { 'k', 'i' }, { 'p', 'i' },
    { 'p', 't' }, { 'r', 'a' }, { 'r', 'n' }, { 'x', 'p' }, {  0 ,  0  }
};

//
// M_DrawString
//  draw a string on screen
//
void M_DrawString(int x, int y, char *str)
{
    int         i;
    static char prev = '\0';

    for (i = 0; (unsigned)i < strlen(str); i++)
    {
        int     j = -1;
        int     k = 0;
        boolean overlapping = false;

        if (str[i] < 123)
            j = chartoi[str[i]];

        while (kern[k].char1)
        {
            if (prev == kern[k].char1 && str[i] == kern[k].char2)
                x += kern[k].adjust;
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
    int i;
    int w = 0;
    static char prev = '\0';

    for (i = 0; (unsigned)i < strlen(str); i++)
    {
        int j = chartoi[str[i]];
        int k = 0;

        while (kern[k].char1)
        {
            if (prev == kern[k].char1 && str[i] == kern[k].char2)
                w += kern[k].adjust;
            k++;
        }

        if (j == -1)
            w += 9;
        else
            w += strlen(redcharset[j]) / 18 - 2;

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

    for (i = strlen(string) / 2 - 1; (unsigned)i < strlen(string); i++)
    {
        if (string[i] == ' ')
        {
            string[i] = '\n';
            break;
        }
    }
}

//
// M_DrawPatchWithShadow
//  draw patch with shadow on screen
//
void M_DrawPatchWithShadow(int x, int y, int scrn, patch_t *patch)
{
    V_DrawPatchWithShadow(x, y, scrn, patch, false);
}

//
// M_DrawPatchWithShadow
//  draw patch with shadow horizontally centered on screen
//
void M_DrawCenteredPatchWithShadow(int y, int scrn, patch_t *patch)
{
    V_DrawPatchWithShadow((ORIGINALWIDTH - patch->width) / 2 + patch->leftoffset, y, scrn, patch, false);
}

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
    FILE        *handle;
    int         i;
    char        name[256];

    for (i = 0; i < load_end; i++)
    {
        strcpy(name, P_SaveGameFile(i));

        handle = fopen(name, "rb");
        if (handle == NULL)
        {
            strcpy(&savegamestrings[i][0], EMPTYSTRING);
            LoadMenu[i].status = 0;
            continue;
        }
        savegames = true;
        fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadMenu[i].status = 1;
    }
}

static byte saveg_read8(FILE *file)
{
    byte result;
    if (fread(&result, 1, 1, file) < 1)
        return 0;
    return result;
}

//
// M_CheckSaveGame
//
boolean M_CheckSaveGame(int choice)
{
    FILE            *handle;
    int             episode;
    int             map;
    int             mission;
    int             i;

    handle = fopen(P_SaveGameFile(itemOn), "rb");

    for (i = 0; i < SAVESTRINGSIZE + VERSIONSIZE + 1; i++)
        saveg_read8(handle);
    episode = saveg_read8(handle);
    map = saveg_read8(handle);
    mission = saveg_read8(handle);

    fclose(handle);

    // switch expansions if necessary
    if (mission == doom2)
    {
        if (gamemission == doom2)
            return true;
        if (gamemission == pack_nerve)
        {
            ExpDef.lastOn = selectedexpansion = exp1;
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
            ExpDef.lastOn = selectedexpansion = exp2;
            gamemission = pack_nerve;
            return true;
        }
        else
            return false;
    }
    if (mission != gamemission)
        return false;
    if (episode > 1 && gamemode == shareware)
        return false;
    if (episode > 3 && gamemode == registered)
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
    if (M_LGTTL)
        M_DrawCenteredPatchWithShadow(23 + OFFSET, 0, (patch_t *)W_CacheLumpName("M_LGTTL", PU_CACHE));
    else
        M_DrawCenteredString(23 + OFFSET, "LOAD GAME");

    for (i = 0; i < load_end; i++)
    {
        int y = LoadDef.y + LINEHEIGHT * i + OFFSET;

        M_DrawSaveLoadBorder(LoadDef.x - 11, y - 4);
        M_WriteText(LoadDef.x - 2, y - 1, savegamestrings[i], false);
    }
}

//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x, int y)
{
    int i;
    int xx, yy;

    for (yy = 0; yy < 16; yy++)
        for (xx = 0; xx < 8; xx++)
            V_DrawPixel(x + xx, y + yy, 0, lsleft[yy * 8 + xx], true);

    x += 8;
    for (i = 0; i < 24; ++i)
    {
        for (yy = 0; yy < 16; yy++)
            for (xx = 0; xx < 8; xx++)
                V_DrawPixel(x + xx, y + yy, 0, lscntr[yy * 8 + xx], true);
        x += 8;
    }

    for (yy = 0; yy < 16; yy++)
        for (xx = 0; xx < 9; xx++)
            V_DrawPixel(x + xx, y + yy, 0, lsrght[yy * 9 + xx], true);
}

//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    if (M_CheckSaveGame(choice))
    {
        char name[256];

        strcpy(name, P_SaveGameFile(choice));

        S_StartSound(NULL, sfx_pistol);
        I_WaitVBL(1 * TICRATE);
        functionkey = 0;
        quickSaveSlot = choice;
        menuactive = false;
        G_LoadGame(name);
    }
}

//
// Selected from DOOM menu
//
void M_LoadGame(int choice)
{
    if (netgame)
        return;

    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}

#define CARETTICS       20

boolean showcaret = true;
int carettics = 0;

//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    char *left;
    char *right;
    int  i;
    int  j;
    int  x, y;
    int  xx, yy;

    left = (char *)Z_Malloc(256, PU_STATIC, NULL);
    right = (char *)Z_Malloc(256, PU_STATIC, NULL);

    // darken background
    M_DarkBackground();

    // draw menu subtitle
    if (M_SGTTL)
        M_DrawCenteredPatchWithShadow(23 + OFFSET, 0, (patch_t *)W_CacheLumpName("M_SGTTL", PU_CACHE));
    else
        M_DrawCenteredString(23 + OFFSET, "SAVE GAME");

    // draw each save game slot
    for (i = 0; i < load_end; ++i)
    {
        // draw save game slot background
        y = LoadDef.y + i * LINEHEIGHT + OFFSET;
        M_DrawSaveLoadBorder(LoadDef.x - 11, y - 4);

        // draw save game description
        if (saveStringEnter && i == saveSlot)
        {
            // draw text to left of text caret
            for (j = 0; j < saveCharIndex; j++)
                left[j] = savegamestrings[i][j];
            left[j] = 0;
            M_WriteText(LoadDef.x - 2, y - 1, left, false);

            // draw text to right of text caret
            for (j = 0; (unsigned)j < strlen(savegamestrings[i]) - saveCharIndex; j++)
                right[j] = savegamestrings[i][j + saveCharIndex];
            right[j] = 0;
            M_WriteText(LoadDef.x - 2 + M_StringWidth(left) + 3, y - 1, right, false);
        }
        else
            M_WriteText(LoadDef.x - 2, y - 1, savegamestrings[i], false);
    }

    // draw text caret
    if (saveStringEnter)
    {
        if (carettics++ == CARETTICS)
        {
            carettics = 0;
            showcaret = !showcaret;
        }
        if (showcaret)
        {
            x = LoadDef.x - 2 + M_StringWidth(left);
            y = LoadDef.y + saveSlot * LINEHEIGHT - 1 + OFFSET;

            for (yy = 0; yy < 9; yy++)
                for (xx = 0; xx < 3; xx++)
                    V_DrawPixel(x + xx, y + yy, 0, (int)caret[yy * 3 + xx], false);
        }
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    menuactive = false;
    G_SaveGame(slot, savegamestrings[slot]);

    savegames = true;
    functionkey = 0;
    quickSaveSlot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
extern char maptitle[128];

void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;

    saveSlot = choice;
    strcpy(saveOldString, savegamestrings[saveSlot]);
    strcpy(savegamestrings[saveSlot], maptitle);
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
char tempstring[160];

void M_QuickSaveResponse(int key)
{
    messageToPrint = 0;
    if (key == 'y')
    {
        strcpy(savegamestrings[quickSaveSlot], maptitle);
        M_DoSave(quickSaveSlot);
    }
}

char *mapnames[][6] =
{
    { HUSTR_E1M1, HUSTR_1,  THUSTR_1,  PHUSTR_1,  NHUSTR_1, HUSTR2_1  },
    { HUSTR_E1M2, HUSTR_2,  THUSTR_2,  PHUSTR_2,  NHUSTR_2, HUSTR2_2  },
    { HUSTR_E1M3, HUSTR_3,  THUSTR_3,  PHUSTR_3,  NHUSTR_3, HUSTR2_3  },
    { HUSTR_E1M4, HUSTR_4,  THUSTR_4,  PHUSTR_4,  NHUSTR_4, HUSTR2_4  },
    { HUSTR_E1M5, HUSTR_5,  THUSTR_5,  PHUSTR_5,  NHUSTR_5, HUSTR2_5  },
    { HUSTR_E1M6, HUSTR_6,  THUSTR_6,  PHUSTR_6,  NHUSTR_6, HUSTR2_6  },
    { HUSTR_E1M7, HUSTR_7,  THUSTR_7,  PHUSTR_7,  NHUSTR_7, HUSTR2_7  },
    { HUSTR_E1M8, HUSTR_8,  THUSTR_8,  PHUSTR_8,  NHUSTR_8, HUSTR2_8  },
    { HUSTR_E1M9, HUSTR_9,  THUSTR_9,  PHUSTR_9,  NHUSTR_9, HUSTR2_9  },
    { HUSTR_E2M1, HUSTR_10, THUSTR_10, PHUSTR_10, "",       HUSTR2_10 },
    { HUSTR_E2M2, HUSTR_11, THUSTR_11, PHUSTR_11, "",       HUSTR2_11 },
    { HUSTR_E2M3, HUSTR_12, THUSTR_12, PHUSTR_12, "",       HUSTR2_12 },
    { HUSTR_E2M4, HUSTR_13, THUSTR_13, PHUSTR_13, "",       HUSTR2_13 },
    { HUSTR_E2M5, HUSTR_14, THUSTR_14, PHUSTR_14, "",       HUSTR2_14 },
    { HUSTR_E2M6, HUSTR_15, THUSTR_15, PHUSTR_15, "",       HUSTR2_15 },
    { HUSTR_E2M7, HUSTR_16, THUSTR_16, PHUSTR_16, "",       HUSTR2_16 },
    { HUSTR_E2M8, HUSTR_17, THUSTR_17, PHUSTR_17, "",       HUSTR2_17 },
    { HUSTR_E2M9, HUSTR_18, THUSTR_18, PHUSTR_18, "",       HUSTR2_18 },
    { HUSTR_E3M1, HUSTR_19, THUSTR_19, PHUSTR_19, "",       HUSTR2_19 },
    { HUSTR_E3M2, HUSTR_20, THUSTR_20, PHUSTR_20, "",       HUSTR2_20 },
    { HUSTR_E3M3, HUSTR_21, THUSTR_21, PHUSTR_21, "",       HUSTR2_21 },
    { HUSTR_E3M4, HUSTR_22, THUSTR_22, PHUSTR_22, "",       HUSTR2_22 },
    { HUSTR_E3M5, HUSTR_23, THUSTR_23, PHUSTR_23, "",       HUSTR2_23 },
    { HUSTR_E3M6, HUSTR_24, THUSTR_24, PHUSTR_24, "",       HUSTR2_24 },
    { HUSTR_E3M7, HUSTR_25, THUSTR_25, PHUSTR_25, "",       HUSTR2_25 },
    { HUSTR_E3M8, HUSTR_26, THUSTR_26, PHUSTR_26, "",       HUSTR2_26 },
    { HUSTR_E3M9, HUSTR_27, THUSTR_27, PHUSTR_27, "",       HUSTR2_27 },
    { HUSTR_E4M1, HUSTR_28, THUSTR_28, PHUSTR_28, "",       HUSTR2_28 },
    { HUSTR_E4M2, HUSTR_29, THUSTR_29, PHUSTR_29, "",       HUSTR2_29 },
    { HUSTR_E4M3, HUSTR_30, THUSTR_30, PHUSTR_30, "",       HUSTR2_30 },
    { HUSTR_E4M4, HUSTR_31, THUSTR_31, PHUSTR_31, "",       HUSTR2_31 },
    { HUSTR_E4M5, HUSTR_32, THUSTR_32, PHUSTR_32, "",       HUSTR2_32 },
    { HUSTR_E4M6, "",       "",        "",        "",       HUSTR2_33 },
    { HUSTR_E4M7, "",       "",        "",        "",       ""        },
    { HUSTR_E4M8, "",       "",        "",        "",       ""        },
    { HUSTR_E4M9, "",       "",        "",        "",       ""        },
    { "",         "",       "",        "",        "",       ""        }
};

void M_QuickSave(void)
{
    int         i = 0;
    boolean     match = false;

    if (quickSaveSlot < 0)
    {
        if (functionkey == KEY_F6)
        {
            functionkey = 0;
            menuactive = false;
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

    while (mapnames[i][gamemission][0])
    {
        if (!strcasecmp(savegamestrings[quickSaveSlot], mapnames[i][gamemission]))
        {
            match = true;
            break;
        }
        i++;
    }
    if (match)
        strcpy(savegamestrings[quickSaveSlot], maptitle);

    M_DoSave(quickSaveSlot);
    S_StartSound(NULL, sfx_swtchx);
}

//
// M_QuickLoad
//
void M_QuickLoadResponse(int key)
{
    messageToPrint = 0;
    if (key == 'y')
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL, sfx_swtchx);
    }
}

void M_QuickLoad(void)
{
    if (netgame || quickSaveSlot < 0)
    {
        functionkey = 0;
        return;
    }

    S_StartSound(NULL, sfx_swtchn);
    snprintf(tempstring, 160, QLPROMPT, savegamestrings[quickSaveSlot]);
    M_SplitString(tempstring);
    if (usinggamepad)
        snprintf(tempstring, 160, "%s\n\n"PRESSA, tempstring);
    else
        snprintf(tempstring, 160, "%s\n\n"PRESSYN, tempstring);
    M_StartMessage(tempstring, M_QuickLoadResponse, true);
}

//
// M_DrawReadThis
//
void M_DrawReadThis(void)
{
    char *lumpname = "HELP1";

    switch (gameversion)
    {
        case exe_doom_1_9:
            if (gamemode == shareware && W_CheckNumForName("HELP3") >= 0)
                lumpname = "HELP3";
            else
                lumpname = (gamemode == commercial ? "HELP" : "HELP2");
            break;
        case exe_ultimate:
            lumpname = "HELP1";
            break;
        case exe_final:
            lumpname = "HELP";
            break;
    }
    if (W_CheckNumForName(lumpname) >= 0)
    {
        if (automapactive)
            V_FillRect(0, 0, 0, SCREENWIDTH, SCREENHEIGHT, 245);
        else
        {
            players[consoleplayer].fixedcolormap = 0;
            M_DarkBlueBackground();
        }
        if (W_CheckMultipleLumps(lumpname) > 2)
            V_DrawPatch(0, 0, 0, (patch_t *)W_CacheLumpName(lumpname, PU_CACHE));
        else
            V_DrawPatchWithShadow(0, 0, 0, (patch_t *)W_CacheLumpName(lumpname, PU_CACHE), false);
    }
}

//
// Change Sfx & Music volumes
//
void M_DrawSound(void)
{
    M_DarkBackground();
    if (M_SVOL)
        M_DrawCenteredPatchWithShadow(38 + OFFSET, 0, (patch_t *)W_CacheLumpName("M_SVOL", PU_CACHE));
    else
        M_DrawCenteredString(38 + OFFSET, "SOUND VOLUME");

    M_DrawThermo(SoundDef.x - 1, SoundDef.y + 16 * sfx_vol + 17 + OFFSET, 16,
                 (float)(sfxVolume * !(nosfx || nosound)), 8.0f);

    M_DrawThermo(SoundDef.x - 1, SoundDef.y + 16 * music_vol + 17 + OFFSET, 16,
                 (float)(musicVolume * !(nomusic || nosound)), 8.0f);
}

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
    if (!nosfx && !nosound)
    {
        switch (choice)
        {
            case 0:
                if (sfxVolume > SFXVOLUME_MIN)
                {
                    S_SetSfxVolume((int)(--sfxVolume * (127.0f / 15.0f)));
                    S_StartSound(NULL, sfx_stnmov);
                    M_SaveDefaults();
                }
                break;
            case 1:
                if (sfxVolume < SFXVOLUME_MAX)
                {
                    S_SetSfxVolume((int)(++sfxVolume * (127.0f / 15.0f)));
                    S_StartSound(NULL, sfx_stnmov);
                    M_SaveDefaults();
                }
                break;
        }
    }
}

void M_MusicVol(int choice)
{
    if (!nomusic && !nosound)
    {
        switch (choice)
        {
            case 0:
                if (musicVolume > MUSICVOLUME_MIN)
                {
                    S_SetMusicVolume((int)(--musicVolume * (127.0f / 15.0f)));
                    S_StartSound(NULL, sfx_stnmov);
                    M_SaveDefaults();
                }
                break;
            case 1:
                if (musicVolume < MUSICVOLUME_MAX)
                {
                    S_SetMusicVolume((int)(++musicVolume * (127.0f / 15.0f)));
                    S_StartSound(NULL, sfx_stnmov);
                    M_SaveDefaults();
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
    int dot1;
    int dot2;
    int y = 11 + OFFSET;

    M_DarkBackground();
    dot1 = screens[0][(y * SCREENWIDTH + 98) * 2];
    dot2 = screens[0][((y + 1) * SCREENWIDTH + 99) * 2];
    M_DrawCenteredPatchWithShadow(y, 0, (patch_t *)W_CacheLumpName("M_DOOM", PU_CACHE));
    if (gamemode != commercial)
    {
        V_DrawPixel(98, y, 0, dot1, false);
        V_DrawPixel(99, y + 1, 0, dot2, false);
    }
}

//
// M_NewGame
//
void M_DrawNewGame(void)
{
    M_DarkBackground();
    if (M_NEWG)
        M_DrawCenteredPatchWithShadow(14 + OFFSET, 0, (patch_t *)W_CacheLumpName("M_NEWG", PU_CACHE));
    else
        M_DrawCenteredString(14 + OFFSET, "NEW GAME");
    if (M_SKILL)
        M_DrawCenteredPatchWithShadow(38 + OFFSET, 0, (patch_t *)W_CacheLumpName("M_SKILL", PU_CACHE));
    else
        M_DrawCenteredString(38 + OFFSET, "Choose Skill Level:");
}

void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
        return;

    M_SetupNextMenu(gamemode == commercial ? (nerve ? &ExpDef : &NewDef) : &EpiDef);
}

//
//      M_Episode
//
int epi;

void M_DrawEpisode(void)
{
    M_DarkBackground();
    if (M_NEWG)
        M_DrawCenteredPatchWithShadow(14 + OFFSET, 0, (patch_t *)W_CacheLumpName("M_NEWG", PU_CACHE));
    else
        M_DrawCenteredString(14 + OFFSET, "NEW GAME");
    if (M_EPISOD)
        M_DrawCenteredPatchWithShadow(38 + OFFSET, 0, (patch_t *)W_CacheLumpName("M_EPISOD", PU_CACHE));
    else
        M_DrawCenteredString(38 + OFFSET, "Which Episode?");
}

void M_DrawExpansion(void)
{
    M_DarkBackground();
    M_DrawCenteredString(14 + OFFSET, "NEW GAME");
    M_DrawCenteredString(38 + OFFSET, "Which Expansion?");
}

void M_VerifyNightmare(int key)
{
    if (key != 'y')
    {
        M_SetupNextMenu(&NewDef);
        messageToPrint = 0;
    }
    else
    {
        messageToPrint = 0;
        S_StartSound(NULL, sfx_swtchx);
        I_WaitVBL(1 * TICRATE);
        quickSaveSlot = -1;
        menuactive = false;
        G_DeferredInitNew((skill_t)nightmare, epi + 1, 1);
    }
}

void M_ChooseSkill(int choice)
{
    if (choice == nightmare && gameskill != sk_nightmare && !nomonsters)
    {
        if (usinggamepad)
            M_StartMessage(NIGHTMARE"\n\n"PRESSA, M_VerifyNightmare, true);
        else
            M_StartMessage(NIGHTMARE"\n\n"PRESSYN, M_VerifyNightmare, true);
        return;
    }

    S_StartSound(NULL, sfx_pistol);
    I_WaitVBL(1 * TICRATE);
    quickSaveSlot = -1;
    G_DeferredInitNew((skill_t)choice, epi + 1, 1);
}

void M_Episode(int choice)
{
    if (gamemode == shareware && choice)
    {
        if (usinggamepad)
            M_StartMessage(SWSTRING"\n\n"PRESSA, NULL, false);
        else
            M_StartMessage(SWSTRING"\n\n"PRESSKEY, NULL, false);
        M_SetupNextMenu(&EpiDef);
        return;
    }

    epi = choice;
    M_SetupNextMenu(&NewDef);
}

void M_Expansion(int choice)
{
    gamemission = (choice == exp1 ? doom2 : pack_nerve);
    M_SetupNextMenu(&NewDef);
}

//
// M_Options
//
void M_DrawOptions(void)
{
    M_DarkBackground();
    if (M_OPTTTL)
        M_DrawCenteredPatchWithShadow(16 + OFFSET, 0,
            (patch_t *)W_CacheLumpName("M_OPTTTL", PU_CACHE));
    else
        M_DrawCenteredString(16 + OFFSET, "OPTIONS");

    if (showMessages)
    {
        if (M_MSGON)
            M_DrawPatchWithShadow(OptionsDef.x + 125, OptionsDef.y + 16 * messages + OFFSET,
                0, (patch_t *)W_CacheLumpName("M_MSGON", PU_CACHE));
        else
            M_DrawString(OptionsDef.x + 125, OptionsDef.y + 16 * messages + OFFSET, "on");
    }
    else
    {
        if (M_MSGOFF)
            M_DrawPatchWithShadow(OptionsDef.x + 125, OptionsDef.y + 16 * messages + OFFSET,
                0, (patch_t *)W_CacheLumpName("M_MSGOFF", PU_CACHE));
        else
            M_DrawString(OptionsDef.x + 125, OptionsDef.y + 16 * messages + OFFSET, "off");
    }

    M_DrawThermo(OptionsDef.x - 1, OptionsDef.y + 16 * mousesens + 17 + OFFSET, 9,
                 mouseSensitivity / (float)MOUSESENSITIVITY_MAX * 8.0f, 8.0f);

    M_DrawThermo(OptionsDef.x - 1, OptionsDef.y + 16 * scrnsize + 17 + OFFSET, 9,
                 (float)(screenSize + ((widescreen || (returntowidescreen && 
                 gamestate != GS_LEVEL)) && !widescreenhud)), 7.2f);
}

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}

//
// Toggle messages on/off
//
boolean message_dontpause = false;

void M_ChangeMessages(int choice)
{
    choice = 0;
    showMessages = !showMessages;
    message_dontpause = true;
    players[consoleplayer].message = (showMessages ? MSGON : MSGOFF);
    message_dontfuckwithme = true;
    M_SaveDefaults();
}

//
// M_EndGame
//
boolean endinggame = false;

void M_EndGameResponse(int key)
{
    messageToPrint = 0;
    if (key != 'y')
    {
        if (functionkey == KEY_F7)
            menuactive = false;
        else
        {
            M_SetupNextMenu(&OptionsDef);
        }
        return;
    }

    currentMenu->lastOn = itemOn;
    menuactive = false;
    viewactive = false;
    automapactive = false;
    S_StartSound(NULL, sfx_swtchx);
    I_WaitVBL(1 * TICRATE);
    MainDef.lastOn = 0;
    endinggame = true;
    if (widescreen)
    {
        ToggleWideScreen(false);
        returntowidescreen = true;
    }
    D_StartTitle();
}

void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame || netgame)
        return;

    if (usinggamepad)
        M_StartMessage(ENDGAME"\n\n"PRESSA, M_EndGameResponse, true);
    else
        M_StartMessage(ENDGAME"\n\n"PRESSYN, M_EndGameResponse, true);
}

//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    M_FinishReadThis(0);
}

void M_FinishReadThis(int choice)
{
    choice = 0;
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

boolean        quitting;
extern boolean waspaused;

void M_QuitResponse(int key)
{
    messageToPrint = 0;
    if (key != 'y')
    {
        quitting = false;
        if (waspaused)
        {
            waspaused = false;
            paused = true;
        }
        if (functionkey == KEY_F10)
            menuactive = false;
        else
            M_SetupNextMenu(&MainDef);
        return;
    }
    if (!netgame && !nosound && !nosfx && sfxVolume > 0)
    {
        if (gamemode == commercial)
            S_StartSound(NULL, quitsounds2[M_Random() % 8]);
        else
            S_StartSound(NULL, quitsounds[M_Random() % 8]);
        I_WaitVBL(3 * TICRATE);
    }
    I_Quit(true);
}

static char *M_SelectEndMessage(void)
{
    char **endmsg;

    endmsg = (gamemission == doom ? doom1_endmsg : doom2_endmsg);
    return endmsg[M_Random() % NUM_QUITMESSAGES];
}

void M_QuitDOOM(int choice)
{
    quitting = true;
    if (usinggamepad)
        sprintf(endstring, "%s\n\n"WINDOWSA, M_SelectEndMessage());
    else
        sprintf(endstring, "%s\n\n"WINDOWSY, M_SelectEndMessage());
    M_StartMessage(endstring, M_QuitResponse, true);
}

void M_SliderSound(void)
{
    static int wait = 0;

    if (wait < I_GetTime())
    {
        wait = I_GetTime() + 7;
        S_StartSound(NULL, sfx_stnmov);
    }
}

void M_ChangeSensitivity(int choice)
{
    switch (choice)
    {
        case 0:
            if (mouseSensitivity > MOUSESENSITIVITY_MIN)
            {
                mouseSensitivity--;
                gamepadSensitivity = 2.0f + mouseSensitivity / (float)MOUSESENSITIVITY_MAX;
                M_SliderSound();
                M_SaveDefaults();
            }
            break;
        case 1:
            if (mouseSensitivity < MOUSESENSITIVITY_MAX)
            {
                mouseSensitivity++;
                gamepadSensitivity = 2.0f + mouseSensitivity / (float)MOUSESENSITIVITY_MAX;
                M_SliderSound();
                M_SaveDefaults();
            }
            break;
    }
}

void M_SizeDisplay(int choice)
{
    switch (choice)
    {
        case 0:
            if (widescreen || (returntowidescreen && gamestate != GS_LEVEL))
            {
                if (!widescreenhud)
                    widescreenhud = true;
                else
                {
                    screenSize--;
                    ToggleWideScreen(false);
                }
                S_StartSound(NULL, sfx_stnmov);
                M_SaveDefaults();
            }
            else if (screenSize > 0)
            {
                screenblocks--;
                screenSize--;
                S_StartSound(NULL, sfx_stnmov);
                R_SetViewSize(screenblocks);
                M_SaveDefaults();
            }
            break;
        case 1:
            if ((widescreen || (returntowidescreen && gamestate != GS_LEVEL)) && widescreenhud)
            {
                widescreenhud = false;
                S_StartSound(NULL, sfx_stnmov);
                M_SaveDefaults();
            }
            else if (screenSize == 7 && fullscreen)
            {
                if (gamestate != GS_LEVEL)
                {
                    returntowidescreen = true;
                    widescreenhud = true;
                }
                else
                {
                    ToggleWideScreen(true);
                    if (!widescreen)
                    {
                        screenblocks++;
                        R_SetViewSize(screenblocks);
                    }
                }
                screenSize++;
                S_StartSound(NULL, sfx_stnmov);
                M_SaveDefaults();
            }
            else if (screenSize < 8)
            {
                screenblocks++;
                screenSize++;
                S_StartSound(NULL, sfx_stnmov);
                R_SetViewSize(screenblocks);
                M_SaveDefaults();
            }
            break;
    }
}

//
// Menu Functions
//
void M_DrawThermo(int x, int y, int thermWidth, float thermDot, float factor)
{
    int         xx = x;
    int         i;

    M_DrawPatchWithShadow(xx, y, 0, (patch_t *)W_CacheLumpName("M_THERML", PU_CACHE));
    xx += 8;
    for (i = 0; i < thermWidth; i++)
    {
        V_DrawPatch(xx, y, 0, (patch_t *)W_CacheLumpName("M_THERMM", PU_CACHE));
        xx += 8;
    }
    M_DrawPatchWithShadow(xx, y, 0, (patch_t *)W_CacheLumpName("M_THERMR", PU_CACHE));
    V_DrawPatch(x + 8 + (int)(thermDot * factor), y, 0,
        (patch_t *)W_CacheLumpName("M_THERMO", PU_CACHE));
    for (i = x + 9; i < x + (thermWidth + 1) * 8 + 1; i++)
        V_DrawPixel(i, y + 13, 0, 251, true);
}

void M_StartMessage(char *string, void *routine, boolean input)
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = (void (*)(int))routine;
    messageNeedsInput = input;
    menuactive = true;
}

//
// Find string width
//
int M_StringWidth(char *string)
{
    size_t      i;
    int         w = 0;
    int         c;

    for (i = 0; i < strlen(string); i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        w += (c < 0 || c >= HU_FONTSIZE ? (i > 0 && (string[i - 1] == '.' ||
              string[i - 1] == '!' || string[i - 1] ==  '?') ? 5 : 3) :
              SHORT(strlen(smallcharset[c]) / 10 - 1));
    }
    return w;
}
//
// Find string height
//
int M_StringHeight(char *string)
{
    size_t      i;
    int         h = 8;

    for (i = 0; i < strlen(string); i++)
        if (string[i] == '\n')
            h += (i > 0 && string[i - 1] == '\n' ? 4 : 8);
    return h;
}

//
//  Write a char
//
void M_DrawSmallChar(int x, int y, int i, boolean shadow)
{
    int         w, x1, y1;

    w = strlen(smallcharset[i]) / 10;

    for (y1 = 0; y1 < 10; y1++)
        for (x1 = 0; x1 < w; x1++)
            V_DrawPixel(x + x1, y + y1, 0, (int)smallcharset[i][y1 * w + x1], shadow);
}

//
// Write a string
//
void M_WriteText(int x, int y, char *string, boolean shadow)
{
    int         w;
    char        *ch = string;
    char        letter;
    char        prev = ' ';
    int         c;
    int         cx = x;
    int         cy = y;

    while (1)
    {
        c = *ch++;
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
            w = hu_font[c]->width;
            if (cx + w > ORIGINALWIDTH)
                break;
            V_DrawPatchWithShadow(cx + 1, cy + 1, 0, hu_font[c], false);
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

//
// CONTROL PANEL
//

//
// M_Responder
//
int gamepadwait = 0;
boolean gamepadpress = false;
int gammawait = 0;

boolean M_Responder(event_t *ev)
{
    // key is the key pressed, ch is the actual character typed
    int         ch = 0;
    int         key = -1;
    int         i;
    static int  keywait = 0;
    static int  mousewait = 0;
    char        *tempstring = "";
    SDLMod      keyMods = SDL_GetModState();

    if (startingnewgame || wipe)
        return false;

    if (ev->type == ev_gamepad && gamepadwait < I_GetTime())
    {
        if (menuactive)
        {
            // activate menu item
            if (gamepadbuttons & GAMEPAD_A)
            {
                key = (messageToPrint && messageNeedsInput ? 'y' : KEY_ENTER);
                gamepadwait = I_GetTime() + 8;
                usinggamepad = true;
            }

            // previous/exit menu
            else if (gamepadbuttons & GAMEPAD_B)
            {
                key = KEY_ESCAPE;
                gamepadwait = I_GetTime() + 8;
                gamepadpress = true;
                usinggamepad = true;
            }

            // exit menu
            else if (gamepadbuttons & gamepadmenu)
            {
                key = KEY_ESCAPE;
                currentMenu = &MainDef;
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
                else if ((gamepadthumbLX < 0 || (gamepadbuttons & GAMEPAD_DPAD_LEFT))
                    && !saveStringEnter
                    && !(currentMenu == &OptionsDef && itemOn == 1))
                {
                    key = KEY_LEFTARROW;
                    gamepadwait = I_GetTime() + 8 * !(currentMenu == &OptionsDef && itemOn == 4);
                    usinggamepad = true;
                }

                // increase slider
                else if ((gamepadthumbLX > 0 || (gamepadbuttons & GAMEPAD_DPAD_RIGHT))
                    && !saveStringEnter
                    && !(currentMenu == &OptionsDef && itemOn == 1))
                {
                    key = KEY_RIGHTARROW;
                    gamepadwait = I_GetTime() + 8 * !(currentMenu == &OptionsDef && itemOn == 4);
                    usinggamepad = true;
                }
            }
        }
        else
        {
            // open menu
            if (gamepadbuttons & gamepadmenu)
            {
                key = KEY_ESCAPE;
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
            key = KEY_ESCAPE;
            mousewait = I_GetTime() + 5;
            usinggamepad = false;
        }

        else if (!messageToPrint)
        {
            // select previous menu item
            if (ev->data1 & MOUSE_WHEELUP)
            {
                key = KEY_UPARROW;
                mousewait = I_GetTime() + 3;
                usinggamepad = false;
            }

            // select next menu item
            else if (ev->data1 & MOUSE_WHEELDOWN)
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
        boolean allspaces = true;

        switch (key)
        {
            // delete character left of caret
            case KEY_BACKSPACE:
                keydown = key;
                if (saveCharIndex > 0)
                {
                    for (i = saveCharIndex - 1; (unsigned)i < strlen(savegamestrings[saveSlot]); i++)
                        savegamestrings[saveSlot][i] = savegamestrings[saveSlot][i + 1];
                    saveCharIndex--;
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // delete character right of caret
            case KEY_DEL:
                keydown = key;
                if ((unsigned)saveCharIndex < strlen(savegamestrings[saveSlot]))
                {
                    for (i = saveCharIndex; (unsigned)i < strlen(savegamestrings[saveSlot]); i++)
                        savegamestrings[saveSlot][i] = savegamestrings[saveSlot][i + 1];
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // cancel
            case KEY_ESCAPE:
                if (!keydown)
                {
                    keydown = key;
                    saveStringEnter = 0;
                    carettics = 0;
                    showcaret = true;
                    strcpy(&savegamestrings[saveSlot][0], saveOldString);
                    S_StartSound(NULL, sfx_swtchx);
                }
                break;

            // confirm
            case KEY_ENTER:
                if (!keydown)
                {
                    keydown = key;
                    for (i = 0; (unsigned)i < strlen(savegamestrings[saveSlot]); i++)
                        if (savegamestrings[saveSlot][i] != ' ')
                            allspaces = false;
                    if (savegamestrings[saveSlot][0] && !allspaces)
                    {
                        saveStringEnter = 0;
                        carettics = 0;
                        showcaret = true;
                        M_DoSave(saveSlot);
                        S_StartSound(NULL, sfx_swtchx);
                    }
                }
                break;

            // move caret left
            case KEY_LEFTARROW:
                if (saveCharIndex > 0)
                {
                    saveCharIndex--;
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // move caret right
            case KEY_RIGHTARROW:
                if ((unsigned)saveCharIndex < strlen(savegamestrings[saveSlot]))
                {
                    saveCharIndex++;
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // move caret to start
            case KEY_HOME:
                if (saveCharIndex > 0)
                {
                    saveCharIndex = 0;
                    carettics = 0;
                    showcaret = true;
                }
                break;

            // move caret to end
            case KEY_END:
                if ((unsigned)saveCharIndex < strlen(savegamestrings[saveSlot]))
                {
                    saveCharIndex = strlen(savegamestrings[saveSlot]);
                    carettics = 0;
                    showcaret = true;
                }
                break;

            default:
                ch = toupper(ch);
                tempstring[0] = ch;
                if (ch >= ' ' && ch <= '_'
                    && M_StringWidth(savegamestrings[saveSlot]) + M_StringWidth(tempstring) <= 190)
                {
                    keydown = key;
                    savegamestrings[saveSlot][strlen(savegamestrings[saveSlot]) + 1] = 0;
                    for (i = strlen(savegamestrings[saveSlot]); i > saveCharIndex; i--)
                        savegamestrings[saveSlot][i] = savegamestrings[saveSlot][i - 1];
                    savegamestrings[saveSlot][saveCharIndex++] = ch;
                    carettics = 0;
                    showcaret = true;
                }
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint && !keydown)
    {
        keydown = key;
        if (messageNeedsInput && key != KEY_ESCAPE
            && key != 'y' && key != 'n' && key != functionkey)
        {
            functionkey = 0;
            return false;
        }
        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
            messageRoutine(key);
        functionkey = 0;
        if (endinggame)
            endinggame = false;
        else
            S_StartSound(NULL, currentMenu == &ReadDef ? sfx_pistol : sfx_swtchx);
        return true;
    }

    // F-Keys
    if ((!menuactive || functionkey) && !paused)
    {

        // Screen size down
        if (key == KEY_MINUS)
        {
            keydown = key;
            if (automapactive || chat_on || !viewactive || inhelpscreens)
                return false;
            M_SizeDisplay(0);
            return false;
        }

        // Screen size up
        else if (key == KEY_EQUALS)
        {
            keydown = key;
            if (automapactive || chat_on || !viewactive || inhelpscreens)
                return false;
            M_SizeDisplay(1);
            return false;
        }

        // Help key
        else if (key == KEY_F1 && (!functionkey || functionkey == KEY_F1)
                 && !keydown)
        {
            keydown = key;
            if (functionkey == KEY_F1)
            {
                functionkey = 0;
                menuactive = false;
                S_StartSound(NULL, sfx_swtchx);
                if (inhelpscreens)
                {
                    R_SetViewSize(screenblocks);
                    if (returntowidescreen)
                        ToggleWideScreen(true);
                }
                inhelpscreens = false;
            }
            else
            {
                functionkey = KEY_F1;
                M_StartControlPanel();
                currentMenu = &ReadDef;
                itemOn = 0;
                S_StartSound(NULL, sfx_swtchn);
                inhelpscreens = true;
                if (widescreen)
                {
                    ToggleWideScreen(false);
                    returntowidescreen = true;
                }
                if (!automapactive)
                    R_SetViewSize(11);
            }
            return false;
        }

        // Save
        else if (key == KEY_F2 && (!functionkey || functionkey == KEY_F2)
                 && (viewactive || automapactive) && !demoplayback && !keydown
                 && players[consoleplayer].health > 0)
        {
            keydown = key;
            if (functionkey == KEY_F2)
            {
                functionkey = 0;
                menuactive = false;
                S_StartSound(NULL, sfx_swtchx);
            }
            else
            {
                functionkey = KEY_F2;
                M_StartControlPanel();
                S_StartSound(NULL, sfx_swtchn);
                M_SaveGame(0);
            }
            return false;
        }

        // Load
        else if (key == KEY_F3 && (!functionkey || functionkey == KEY_F3)
                 && savegames && !keydown)
        {
            keydown = key;
            if (functionkey == KEY_F3)
            {
                functionkey = 0;
                menuactive = false;
                S_StartSound(NULL, sfx_swtchx);
            }
            else
            {
                functionkey = KEY_F3;
                M_StartControlPanel();
                S_StartSound(NULL,sfx_swtchn);
                M_LoadGame(0);
            }
            return false;
        }

        else if (key == KEY_F4 && (!functionkey || functionkey == KEY_F4 || (keyMods & KMOD_ALT))
                 && !keydown)
        {
            keydown = key;

            // Quit DOOM
            if (keyMods & KMOD_ALT)
            {
                S_StartSound(NULL, sfx_swtchn);
                M_QuitDOOM(0);
                return false;
            }
            else
            {
                // Sound Volume
                if (functionkey == KEY_F4)
                {
                    functionkey = 0;
                    menuactive = false;
                    S_StartSound(NULL, sfx_swtchx);
                }
                else
                {
                    functionkey = KEY_F4;
                    M_StartControlPanel();
                    currentMenu = &SoundDef;
                    itemOn = sfx_vol;
                    S_StartSound(NULL, sfx_swtchn);
                }
                return false;
            }
        }

        // Quicksave
        else if (key == KEY_F6 && (!functionkey || functionkey == KEY_F6)
                 && (viewactive || automapactive) && !demoplayback && !keydown
                 && players[consoleplayer].health > 0)
        {
            keydown = key;
            if (quickSaveSlot >= 0)
                functionkey = KEY_F6;
            M_QuickSave();
            return false;
        }

        // End game
        else if (key == KEY_F7 && !functionkey && (viewactive || automapactive)
                 && !demoplayback && !keydown)
        {
            keydown = key;
            functionkey = KEY_F7;
            S_StartSound(NULL, sfx_swtchn);
            M_EndGame(0);
            return false;
        }

        // Toggle messages
        else if (key == KEY_F8 && !functionkey && (viewactive || automapactive)
                 && !keydown)
        {
            keydown = key;
            functionkey = KEY_F8;
            M_ChangeMessages(0);
            functionkey = 0;
            S_StartSound(NULL, sfx_swtchn);
            return false;
        }

        // Quickload
        else if (key == KEY_F9 && !functionkey && (viewactive || automapactive) &&
                 !demoplayback && savegames && !keydown)
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
        static char buf[128];

        if (gammawait >= I_GetTime() || gamestate != GS_LEVEL || inhelpscreens)
        {
            if (keyMods & KMOD_SHIFT)
            {
                if (--gammalevelindex < 0)
                    gammalevelindex = GAMMALEVELS - 1;
            }
            else
            {
                if (++gammalevelindex > GAMMALEVELS - 1)
                    gammalevelindex = 0;
            }
            S_StartSound(NULL, sfx_stnmov);
        }

        gammawait = I_GetTime() + HU_MSGTIMEOUT;

        gammalevel = (float)gammalevels[gammalevelindex];

        sprintf(buf, GAMMALVL, gammalevel);
        if (buf[strlen(buf) - 1] == '0' && buf[strlen(buf) - 2] == '0')
            buf[strlen(buf) - 1] = '\0';
        players[consoleplayer].message = buf;

        message_dontpause = true;
        message_dontfuckwithme = true;

        I_SetPalette((byte *)W_CacheLumpName("PLAYPAL", PU_CACHE) + st_palette * 768);

        M_SaveDefaults();

        return false;
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == KEY_ESCAPE && !keydown)
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
        if (key == KEY_DOWNARROW && keywait < I_GetTime())
        {
            // Move down to next item
            do
            {
                if (itemOn + 1 > currentMenu->numitems - 1)
                    itemOn = 0;
                else
                    itemOn++;
                if (currentMenu == &MainDef && itemOn == 2 && !savegames)
                    itemOn++;
                if (currentMenu == &MainDef && itemOn == 3
                    && (!usergame || gamestate != GS_LEVEL || players[consoleplayer].health <= 0))
                    itemOn++;
                if (currentMenu == &OptionsDef && !itemOn && (!usergame || netgame))
                    itemOn++;
                if (currentMenu->menuitems[itemOn].status != -1)
                    S_StartSound(NULL, sfx_pstop);
            }
            while (currentMenu->menuitems[itemOn].status == -1);

            if (currentMenu == &EpiDef && gamemode != shareware)
            {
                selectedepisode = itemOn;
                M_SaveDefaults();
            }
            else if (currentMenu == &ExpDef)
            {
                selectedexpansion = itemOn;
                M_SaveDefaults();
            }
            else if (currentMenu == &NewDef)
            {
                selectedskilllevel = itemOn;
                M_SaveDefaults();
            }
            else if (currentMenu == &SaveDef)
            {
                LoadDef.lastOn = selectedsavegame = itemOn;
                M_SaveDefaults();
            }
            else if (currentMenu == &LoadDef)
            {
                SaveDef.lastOn = selectedsavegame = itemOn;
                M_SaveDefaults();
            }
            keywait = I_GetTime() + 2;

            return false;
        }
        else if (key == KEY_UPARROW && keywait < I_GetTime())
        {
            do
            {
                // Move back up to previous item
                if (!itemOn)
                    itemOn = currentMenu->numitems - 1;
                else
                    itemOn--;
                if (currentMenu == &MainDef && itemOn == 3
                    && (!usergame || gamestate != GS_LEVEL || players[consoleplayer].health <= 0))
                    itemOn--;
                if (currentMenu == &MainDef && itemOn == 2 && !savegames)
                    itemOn--;
                if (currentMenu == &OptionsDef && !itemOn && (!usergame || netgame))
                    itemOn = currentMenu->numitems - 1;
                if (currentMenu->menuitems[itemOn].status != -1)
                    S_StartSound(NULL, sfx_pstop);
            }
            while (currentMenu->menuitems[itemOn].status == -1);

            if (currentMenu == &EpiDef && gamemode != shareware)
            {
                selectedepisode = itemOn;
                M_SaveDefaults();
            }
            else if (currentMenu == &ExpDef)
            {
                selectedexpansion = itemOn;
                M_SaveDefaults();
            }
            else if (currentMenu == &NewDef)
            {
                selectedskilllevel = itemOn;
                M_SaveDefaults();
            }
            else if (currentMenu == &SaveDef)
            {
                LoadDef.lastOn = selectedsavegame = itemOn;
                M_SaveDefaults();
            }
            else if (currentMenu == &LoadDef)
            {
                SaveDef.lastOn = selectedsavegame = itemOn;
                M_SaveDefaults();
            }
            keywait = I_GetTime() + 2;

            return false;
        }

        else if (key == KEY_LEFTARROW 
                 || (key == KEY_MINUS && !(currentMenu == &OptionsDef && itemOn == 1)))
        {
            // Slide slider left
            if (currentMenu->menuitems[itemOn].routine
                && currentMenu->menuitems[itemOn].status == 2)
                currentMenu->menuitems[itemOn].routine(0);
            else if (currentMenu == &OptionsDef && itemOn == 1 && !keydown)
            {
                keydown = key;
                currentMenu->menuitems[itemOn].routine(itemOn);
                S_StartSound(NULL, sfx_pistol);
            }
            return false;
        }

        else if (key == KEY_RIGHTARROW 
                 || (key == KEY_EQUALS && !(currentMenu == &OptionsDef && itemOn == 1)))
        {
            // Slide slider right
            if (currentMenu->menuitems[itemOn].routine
                && currentMenu->menuitems[itemOn].status == 2)
                currentMenu->menuitems[itemOn].routine(1);
            else if (currentMenu == &OptionsDef && itemOn == 1 && !keydown)
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
            if (currentMenu == &ReadDef)
            {
                functionkey = 0;
                menuactive = false;
                S_StartSound(NULL, sfx_swtchx);
                return true;
            }
            if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status)
            {
                if (gamemode != shareware || currentMenu != &EpiDef)
                    currentMenu->lastOn = itemOn;
                if (currentMenu->menuitems[itemOn].status == 2)
                    currentMenu->menuitems[itemOn].routine(1);
                else
                {
                    if ((!usergame || gamestate != GS_LEVEL) && currentMenu == &MainDef
                        && itemOn == 3)
                        return true;
                    if ((!usergame || netgame) && currentMenu == &OptionsDef && !itemOn)
                        return true;
                    if (currentMenu != &LoadDef && (currentMenu != &NewDef ||
                        (currentMenu == &NewDef && itemOn == 4)))
                        S_StartSound(NULL, sfx_pistol);
                    currentMenu->menuitems[itemOn].routine(itemOn);
                }
            }
            return (currentMenu == &LoadDef || currentMenu == &SaveDef);
        }

        else if (key == KEY_ESCAPE && !keydown)
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
            else if (TITLEPIC || usergame || gamestate == GS_LEVEL)
            {
                functionkey = 0;
                menuactive = false;
                S_StartSound(NULL, sfx_swtchx);
                gamepadbuttons = 0;
                ev->data1 = 0;
                firstevent = true;
            }
            if (inhelpscreens)
            {
                R_SetViewSize(screenblocks);
                if (returntowidescreen)
                    ToggleWideScreen(true);
            }
            return true;
        }

        // Keyboard shortcut?
        else if (ch != 0)
        {

            for (i = itemOn + 1; i < currentMenu->numitems; i++)
            {
                if (toupper(currentMenu->menuitems[i].alphaKey) == toupper(ch))
                {
                    if (currentMenu == &MainDef && i == 3
                        && (!usergame || gamestate != GS_LEVEL
                            || players[consoleplayer].health <= 0))
                        return true;
                    if (currentMenu == &MainDef && i == 2 && !savegames)
                        return true;
                    if (currentMenu == &OptionsDef && !i && (!usergame || netgame))
                        return true;
                    if (itemOn != i)
                        S_StartSound(NULL, sfx_pstop);
                    itemOn = i;
                    if (currentMenu == &EpiDef && gamemode != shareware)
                    {
                        selectedepisode = itemOn;
                        M_SaveDefaults();
                    }
                    else if (currentMenu == &ExpDef)
                    {
                        selectedexpansion = itemOn;
                        M_SaveDefaults();
                    }
                    else if (currentMenu == &NewDef)
                    {
                        selectedskilllevel = itemOn;
                        M_SaveDefaults();
                    }
                    else if (currentMenu == &SaveDef)
                    {
                        LoadDef.lastOn = selectedsavegame = itemOn;
                        M_SaveDefaults();
                    }
                    else if (currentMenu == &LoadDef)
                    {
                        SaveDef.lastOn = selectedsavegame = itemOn;
                        M_SaveDefaults();
                    }
                    return false;
                }
            }

            for (i = 0; i <= itemOn; i++)
            {
                if (toupper(currentMenu->menuitems[i].alphaKey) == toupper(ch))
                {
                    if (currentMenu == &MainDef && i == 3
                        && (!usergame || gamestate != GS_LEVEL
                            || players[consoleplayer].health <= 0))
                        return true;
                    if (currentMenu == &MainDef && i == 2 && !savegames)
                        return true;
                    if (currentMenu == &OptionsDef && !i && (!usergame || netgame))
                        return true;
                    if (itemOn != i)
                        S_StartSound(NULL, sfx_pstop);
                    itemOn = i;
                    if (currentMenu == &EpiDef && gamemode != shareware)
                    {
                        selectedepisode = itemOn;
                        M_SaveDefaults();
                    }
                    else if (currentMenu == &ExpDef)
                    {
                        selectedexpansion = itemOn;
                        M_SaveDefaults();
                    }
                    else if (currentMenu == &NewDef)
                    {
                        selectedskilllevel = itemOn;
                        M_SaveDefaults();
                    }
                    else if (currentMenu == &SaveDef)
                    {
                        LoadDef.lastOn = selectedsavegame = itemOn;
                        M_SaveDefaults();
                    }
                    else if (currentMenu == &LoadDef)
                    {
                        SaveDef.lastOn = selectedsavegame = itemOn;
                        M_SaveDefaults();
                    }
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
}

//
// M_DrawNightmare
//
void M_DrawNightmare(void)
{
    int x;
    int y;

    for (y = 0; y < 20; y++)
    {
        for (x = 0; x < 124; x++)
        {
            V_DrawPixel(NewDef.x + x, NewDef.y + OFFSET + 16 * nightmare + y, 0,
                        (int)nmare[y * 124 + x], true);
        }
    }
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
    static short x;
    static short y;
    unsigned int i;
    unsigned int max;
    char         string[80];
    char         *name;
    int          start;

    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        M_DarkBackground();

        start = 0;
        if (!usergame || gamestate == GS_INTERMISSION || gamestate == GS_FINALE)
            y = (ORIGINALHEIGHT - M_StringHeight(messageString)) / 2 - 1;
        else
            y = viewwindowy / 2 + (viewheight / 2 - M_StringHeight(messageString)) / 2 - 1;
        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
                if (messageString[start + i] == '\n')
                {
                    memset(string, 0, sizeof(string));
                    strncpy(string, messageString + start, i);
                    foundnewline = 1;
                    start += i + 1;
                    break;
                }

            if (!foundnewline)
            {
                strcpy(string, messageString + start);
                start += strlen(string);
            }

            x = (ORIGINALWIDTH - M_StringWidth(string)) / 2;
            if (!M_StringWidth(string))
                y -= 4;
            M_WriteText(x, y, string, true);
            y += SHORT(hu_font[0]->height + 1);
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
    y = currentMenu->y + OFFSET;
    max = currentMenu->numitems;

    for (i = 0; i < max; i++)
    {
        name = currentMenu->menuitems[i].name;

        if (!strcmp(name, "M_NMARE"))
        {
            if (M_NMARE)
                M_DrawPatchWithShadow(x, y, 0, (patch_t *)W_CacheLumpName(name, PU_CACHE));
            else
                M_DrawNightmare();
        }
        else if (!strcmp(name, "M_MSENS") && !M_MSENS)
        {
            if (usinggamepad)
            {
                M_DrawString(x, y, "Gamepad Sensitivity");
                currentMenu->menuitems[mousesens].alphaKey = 'g';
            }
            else
            {
                M_DrawString(x, y, "Mouse Sensitivity");
                currentMenu->menuitems[mousesens].alphaKey = 'm';
            }
        }
        else if (W_CheckMultipleLumps(name) > 1)
            M_DrawPatchWithShadow(x, y, 0, (patch_t *)W_CacheLumpName(name, PU_CACHE));
        else
            M_DrawString(x, y, currentMenu->menuitems[i].text);
        y += LINEHEIGHT - 1;
    }

    // DRAW SKULL
    if (currentMenu == &LoadDef || currentMenu == &SaveDef)
    {
        M_DrawPatchWithShadow(x - 37, currentMenu->y + itemOn * 17 - 7 + OFFSET, 0,
                              (patch_t *)W_CacheLumpName(skullName[whichSkull], PU_CACHE));
    }
    else if (currentMenu != &ReadDef)
    {
        if (currentMenu == &OptionsDef && !itemOn && (!usergame || netgame))
            itemOn++;
        M_DrawPatchWithShadow(x - 26, currentMenu->y + itemOn * 16 - (M_SKULL1 ? 5 : 3) + OFFSET,
                              0, (patch_t *)W_CacheLumpName(skullName[whichSkull], PU_CACHE));
    }
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
    if (!saveStringEnter || !whichSkull)
    {
        if (--skullAnimCounter <= 0)
        {
            whichSkull ^= 1;
            skullAnimCounter = 8;
        }
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
    screenSize = (widescreen || returntowidescreen ? 8 : screenblocks - 3);
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;
    tempscreen = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    blurredscreen = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    if (autostart)
    {
        selectedepisode = startepisode - 1;
        selectedskilllevel = startskill;
    }
    if ((gamemode == registered && (selectedepisode < 0 || selectedepisode > 2))
         || (gamemode == retail && (selectedepisode < 0 || selectedepisode > 3)))
        selectedepisode = 0;
    if (gamemode != shareware)
        EpiDef.lastOn = selectedepisode;
    if (selectedexpansion < 0 || selectedexpansion > 1)
        selectedexpansion = 0;
    ExpDef.lastOn = selectedexpansion;
    if (selectedskilllevel < 0 || selectedskilllevel > 4)
        selectedskilllevel = 2;
    NewDef.lastOn = selectedskilllevel;
    if (selectedsavegame < 0 || selectedsavegame > 5)
        selectedsavegame = 0;
    SaveDef.lastOn = LoadDef.lastOn = selectedsavegame;
    M_ReadSaveStrings();

    if (gamemode == commercial)
    {
        if (nerve)
            NewDef.prevMenu = &ExpDef;
        else
            NewDef.prevMenu = &MainDef;
    }
    else if (gamemode == registered)
        EpiDef.numitems--;
}
