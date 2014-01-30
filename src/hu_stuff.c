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

#include <ctype.h>

#include "doomdef.h"

#include "z_zone.h"

#include "i_swap.h"
#include "i_timer.h"
#include "i_video.h"

#include "hu_stuff.h"
#include "hu_lib.h"
#include "w_wad.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

#include "r_main.h"

//
// Locally used constants, shortcuts.
//
#define HU_TITLE        (mapnames[(gameepisode - 1) * 9 + gamemap - 1][doom])
#define HU_TITLE2       (mapnames[gamemap - 1][doom2])
#define HU_TITLEP       (mapnames[gamemap - 1][pack_plut])
#define HU_TITLET       (mapnames[gamemap - 1][pack_tnt])
#define HU_TITLEN       (mapnames[gamemap - 1][pack_nerve])
#define HU_TITLE2B      (mapnames[gamemap - 1][doom2bfg])
#define HU_TITLEHEIGHT  1
#define HU_TITLEX       0
#define HU_TITLEY       ((SCREENHEIGHT - SBARHEIGHT) / SCREENSCALE - SHORT(hu_font[0]->height) - 2)


#define HU_INPUTTOGGLE  't'
#define HU_INPUTX       HU_MSGX
#define HU_INPUTY       (165 - HU_MSGHEIGHT * SHORT(hu_font[0]->height))
#define HU_INPUTWIDTH   64
#define HU_INPUTHEIGHT  1



char *chat_macros[] =
{
    HUSTR_CHATMACRO0,
    HUSTR_CHATMACRO1,
    HUSTR_CHATMACRO2,
    HUSTR_CHATMACRO3,
    HUSTR_CHATMACRO4,
    HUSTR_CHATMACRO5,
    HUSTR_CHATMACRO6,
    HUSTR_CHATMACRO7,
    HUSTR_CHATMACRO8,
    HUSTR_CHATMACRO9
};

char *player_names[] =
{
    HUSTR_PLRGREEN,
    HUSTR_PLRINDIGO,
    HUSTR_PLRBROWN,
    HUSTR_PLRRED
};

int key_multi_msg = HU_INPUTTOGGLE;

int key_multi_msgplayer[MAXPLAYERS] =
{
    HUSTR_KEYGREEN,
    HUSTR_KEYINDIGO,
    HUSTR_KEYBROWN,
    HUSTR_KEYRED
};

char                    chat_char;
static player_t         *plr;
patch_t                 *hu_font[HU_FONTSIZE];
static hu_textline_t    w_title;
boolean                 chat_on;
static hu_itext_t       w_chat;
static boolean          always_off = false;
static char             chat_dest[MAXPLAYERS];
static hu_itext_t       w_inputbuffer[MAXPLAYERS];

static boolean          message_on;
boolean                 message_dontfuckwithme;
static boolean          message_nottobefuckedwith;

boolean                 idbehold = false;

static hu_stext_t       w_message;
int                     message_counter;

extern int              showMessages;
extern boolean          widescreen;

static boolean          headsupactive = false;

//
// Builtin map names.
// The actual names can be found in d_englsh.h.
//

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

void HU_Init(void)
{

    int         i;
    int         j;
    char        buffer[9];

    // load the heads-up font
    j = HU_FONTSTART;
    for (i = 0; i < HU_FONTSIZE; i++)
    {
        snprintf(buffer, 9, "STCFN%.3d", j++);
        hu_font[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
}

void HU_Stop(void)
{
    headsupactive = false;
}

byte *tempscreen;

void HU_Start(void)
{

    int         i;
    char*       s = "";

    if (headsupactive)
        HU_Stop();

    plr = &players[consoleplayer];
    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    chat_on = false;

    // create the message widget
    HUlib_initSText(&w_message,
                    HU_MSGX, HU_MSGY, HU_MSGHEIGHT,
                    hu_font,
                    HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_initTextLine(&w_title,
                       HU_TITLEX, HU_TITLEY,
                       hu_font,
                       HU_FONTSTART);

    s = (char *)Z_Malloc(133, PU_STATIC, NULL);
    strcpy(s, mapnumandtitle);

    while (*s)
        HUlib_addCharToTextLine(&w_title, *(s++));

    // create the chat widget
    HUlib_initIText(&w_chat,
                    HU_INPUTX, HU_INPUTY,
                    hu_font,
                    HU_FONTSTART, &chat_on);

    // create the inputbuffer widgets
    for (i = 0; i < MAXPLAYERS; i++)
        HUlib_initIText(&w_inputbuffer[i], 0, 0, 0, 0, &always_off);

    headsupactive = true;

    tempscreen = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
}


void HU_Drawer(void)
{

    w_message.l->x = (automapactive && fullscreen && !widescreen ? 0 : 3);
    w_message.l->y = HU_MSGY;
    HUlib_drawSText(&w_message);
    HUlib_drawIText(&w_chat);
    if (automapactive)
    {
        w_title.x = (fullscreen && !widescreen ? 0 : 3);
        HUlib_drawTextLine(&w_title, false);
    }

}

void HU_Erase(void)
{

    HUlib_eraseSText(&w_message);
    HUlib_eraseIText(&w_chat);
    HUlib_eraseTextLine(&w_title);

}

extern fixed_t m_x, m_y, m_h, m_w;
extern boolean message_dontpause;
extern int direction;

void HU_Ticker(void)
{
    static int lasttic = -1;
    int i, rc, tic, fps;
    char c;
    static char fps_str[8] = "";

    // tick down message counter if message is up
    if (((!menuactive && !paused) || demoplayback || message_dontpause)
        && !idbehold
        && !(players[consoleplayer].cheats & CF_MYPOS)
        && !devparm
        && message_counter
        && !--message_counter)
    {
        message_on = false;
        message_nottobefuckedwith = false;
        message_dontpause = false;
    }

    if (idbehold)
    {
        // [BH] display message for IDBEHOLDx cheat
        if (!message_counter)
            message_counter = HU_MSGTIMEOUT;
        else if (message_counter > 132)
            message_counter--;

        HUlib_addMessageToSText(&w_message, 0, STSTR_BEHOLD);
        message_on = true;
    }
    else if (players[consoleplayer].cheats & CF_MYPOS)
    {
        // [BH] display and constantly update message for IDMYPOS cheat
        char buffer[80];
        int angle;
        int x, y, z;

        if (!message_counter)
            message_counter = HU_MSGTIMEOUT;
        else if (message_counter > 132)
            message_counter--;

        if (automapactive && !followplayer)
        {
            angle = direction;
            x = (m_x + (m_w >> 1)) / FRACUNIT;
            y = (m_y + (m_h >> 1)) / FRACUNIT;
            z = R_PointInSubsector(m_x + (m_w >> 1),
                                   m_y + (m_h >> 1))->sector->floorheight / FRACUNIT;
        }
        else
        {
            angle = (int)((double)plr->mo->angle * (90.0f / ANG90));
            if (angle == 360)
                angle = 0;
            x = plr->mo->x / FRACUNIT;
            y = plr->mo->y / FRACUNIT;
            z = plr->mo->z / FRACUNIT;
        }

        sprintf(buffer, STSTR_MYPOS, angle, STCFN034 ? ' ' : '*', x, y, z);
        HUlib_addMessageToSText(&w_message, 0, buffer);
        message_on = true;
    }
    else if (devparm)
    {
        // [BH] display and constantly update FPS for -DEVPARM
        tic = I_GetTime();
        fps = (lasttic == -1 ? TICRATE : TICRATE - (tic - lasttic) + 1);
        if (fps < 1)
            fps = 1;
        if (fps > TICRATE)
            fps = TICRATE;
        lasttic = tic;
        sprintf(fps_str, "%i FPS", fps);
        HUlib_addMessageToSText(&w_message, 0, fps_str);
        message_on = true;
    }
    else if (showMessages || message_dontfuckwithme)
    {

        // display message if necessary
        if ((plr->message && !message_nottobefuckedwith)
            || (plr->message && message_dontfuckwithme))
        {
            HUlib_addMessageToSText(&w_message, 0, plr->message);
            plr->message = 0;
            message_on = true;
            message_counter = HU_MSGTIMEOUT;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = 0;
        }

    }

    // check for incoming chat characters
    if (netgame)
    {
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (!playeringame[i])
                continue;
            if (i != consoleplayer
                && (c = players[i].cmd.chatchar))
            {
                if (c <= HU_BROADCAST)
                    chat_dest[i] = c;
                else
                {
                    rc = HUlib_keyInIText(&w_inputbuffer[i], c);
                    if (rc && c == KEY_ENTER)
                    {
                        if (w_inputbuffer[i].l.len
                            && (chat_dest[i] == consoleplayer+1
                                || chat_dest[i] == HU_BROADCAST))
                        {
                            HUlib_addMessageToSText(&w_message,
                                                    player_names[i],
                                                    w_inputbuffer[i].l.l);

                            message_nottobefuckedwith = true;
                            message_on = true;
                            message_counter = HU_MSGTIMEOUT;
                            if (gamemode == commercial)
                                S_StartSound(NULL, sfx_radio);
                            else
                                S_StartSound(NULL, sfx_tink);
                        }
                        HUlib_resetIText(&w_inputbuffer[i]);
                    }
                }
                players[i].cmd.chatchar = 0;
            }
        }
    }
}

#define QUEUESIZE               128

static char     chatchars[QUEUESIZE];
static int      head = 0;
static int      tail = 0;


void HU_queueChatChar(char c)
{
    if (((head + 1) & (QUEUESIZE - 1)) == tail)
    {
        plr->message = HUSTR_MSGU;
    }
    else
    {
        chatchars[head] = c;
        head = (head + 1) & (QUEUESIZE - 1);
    }
}

char HU_dequeueChatChar(void)
{
    char c;

    if (head != tail)
    {
        c = chatchars[tail];
        tail = (tail + 1) & (QUEUESIZE - 1);
    }
    else
    {
        c = 0;
    }

    return c;
}

boolean HU_Responder(event_t *ev)
{

    static char         lastmessage[HU_MAXLINELENGTH+1];
    char*               macromessage;
    boolean             eatkey = false;
    static boolean      altdown = false;
    unsigned char       c;
    int                 i;
    int                 numplayers;

    static int          num_nobrainers = 0;

    numplayers = 0;
    for (i = 0; i < MAXPLAYERS; i++)
        numplayers += playeringame[i];

    if (ev->data1 == KEY_RSHIFT)
    {
        return false;
    }
    else if (ev->data1 == KEY_RALT || ev->data1 == KEY_LALT)
    {
        altdown = ev->type == ev_keydown;
        return false;
    }

    if (ev->type != ev_keydown)
        return false;

    if (!chat_on)
    {
        if (netgame && ev->data2 == key_multi_msg)
        {
            eatkey = chat_on = true;
            HUlib_resetIText(&w_chat);
            HU_queueChatChar(HU_BROADCAST);
        }
        else if (netgame && numplayers > 2)
        {
            for (i = 0; i < MAXPLAYERS; i++)
            {
                if (ev->data2 == key_multi_msgplayer[i])
                {
                    if (playeringame[i] && i != consoleplayer)
                    {
                        eatkey = chat_on = true;
                        HUlib_resetIText(&w_chat);
                        HU_queueChatChar(i + 1);
                        break;
                    }
                    else if (i == consoleplayer)
                    {
                        num_nobrainers++;
                        if (num_nobrainers < 3)
                            plr->message = HUSTR_TALKTOSELF1;
                        else if (num_nobrainers < 6)
                            plr->message = HUSTR_TALKTOSELF2;
                        else if (num_nobrainers < 9)
                            plr->message = HUSTR_TALKTOSELF3;
                        else if (num_nobrainers < 32)
                            plr->message = HUSTR_TALKTOSELF4;
                        else
                            plr->message = HUSTR_TALKTOSELF5;
                    }
                }
            }
        }
    }
    else
    {
        c = ev->data2;
        // send a macro
        if (altdown)
        {
            c = c - '0';
            if (c > 9)
                return false;
            macromessage = chat_macros[c];

            // kill last message with a '\n'
            HU_queueChatChar(KEY_ENTER);

            // send the macro message
            while (*macromessage)
                HU_queueChatChar(*macromessage++);
            HU_queueChatChar(KEY_ENTER);

            // leave chat mode and notify that it was sent
            chat_on = false;
            strcpy(lastmessage, chat_macros[c]);
            plr->message = lastmessage;
            eatkey = true;
        }
        else
        {
            c = ev->data2;

            eatkey = HUlib_keyInIText(&w_chat, c);
            if (eatkey)
            {
                HU_queueChatChar(c);
            }
            if (c == KEY_ENTER)
            {
                chat_on = false;
                if (w_chat.l.len)
                {
                    strcpy(lastmessage, w_chat.l.l);
                    plr->message = lastmessage;
                }
            }
            else if (c == KEY_ESCAPE)
                chat_on = false;
        }
    }

    return eatkey;
}

void HU_clearMessages(void)
{
    plr->message = 0;
    message_counter = 0;
    message_on = false;
    message_nottobefuckedwith = false;
    message_dontpause = false;
}