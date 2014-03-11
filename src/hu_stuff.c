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

#include "doomstat.h"
#include "dstrings.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "i_swap.h"
#include "i_timer.h"
#include "r_main.h"
#include "s_sound.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

//
// Locally used constants, shortcuts.
//
#define HU_TITLEX       0
#define HU_TITLEY       ((SCREENHEIGHT - SBARHEIGHT) / 2 - SHORT(hu_font[0]->height) - 2)

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
extern boolean          widescreenhud;
extern int              translucency;

static boolean          headsupactive = false;

byte                    *tempscreen;
int                     hudnumbase;

void(*hudfunc)(int, int, int, patch_t *);
void(*hudnumfunc)(int, int, int, patch_t *);

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
        hu_font[i] = (patch_t *)W_CacheLumpName(buffer, PU_STATIC);
    }
}

void HU_Stop(void)
{
    headsupactive = false;
}

void HU_Start(void)
{

    int         i;
    char        *s = "";

    if (headsupactive)
        HU_Stop();

    plr = &players[consoleplayer];
    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    chat_on = false;

    // create the message widget
    HUlib_initSText(&w_message, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, hu_font, HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_initTextLine(&w_title, HU_TITLEX, HU_TITLEY, hu_font, HU_FONTSTART);

    s = (char *)Z_Malloc(133, PU_STATIC, NULL);
    strcpy(s, mapnumandtitle);

    while (*s)
        HUlib_addCharToTextLine(&w_title, *(s++));

    // create the chat widget
    HUlib_initIText(&w_chat, HU_INPUTX, HU_INPUTY, hu_font, HU_FONTSTART, &chat_on);

    // create the inputbuffer widgets
    for (i = 0; i < MAXPLAYERS; i++)
        HUlib_initIText(&w_inputbuffer[i], 0, 0, 0, 0, &always_off);

    headsupactive = true;

    tempscreen = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    hudnumbase = W_GetNumForName("STTNUM0");

    if (translucency)
    {
        hudfunc = V_DrawTranslucentHUDPatch;
        hudnumfunc = V_DrawTranslucentHUDNumberPatch;
    }
    else
    {
        hudfunc = V_DrawHUDPatch;
        hudnumfunc = V_DrawHUDNumberPatch;
    }
}

static void DrawHUDNumber(int x, int y, signed int val)
{
    int         xpos = x;
    int         oldval = val;

    if (val > 99)
        hudnumfunc(xpos + 8, y, 0, W_CacheLumpNum(hudnumbase + val / 100, PU_CACHE));
    val %= 100;
    xpos += 14;
    if (val > 9 || oldval > 99)
        hudnumfunc(xpos + 8, y, 0, W_CacheLumpNum(hudnumbase + val / 10, PU_CACHE));
    val %= 10;
    xpos += 14;
    hudnumfunc(xpos + 8, y, 0, W_CacheLumpNum(hudnumbase + val, PU_CACHE));
}

#define HUD_HEALTH_X    24
#define HUD_HEALTH_Y    308

#define HUD_AMMO_X      120
#define HUD_AMMO_Y      308

#define HUD_KEYS_X      503
#define HUD_KEYS_Y      308

#define HUD_ARMOR_X     530
#define HUD_ARMOR_Y     308

struct
{
    char        *lump;
    int         x;
    int         y;
}
ammopic[NUMAMMO] =
{
    { "CLIPA0",  0,  3 },
    { "SHELA0", -5,  5 },
    { "CELLA0", -8,  2 },
    { "ROCKA0", -2, -6 }
};

char *keypic[NUMCARDS] =
{
    "BKEYB0", "YKEYB0", "RKEYB0", "BSKUB0", "YSKUB0", "RSKUB0"
};

static void DrawHUD(void)
{
    int         health = plr->mo->health;

    if (health > 0)
    {
        int     ammotype = weaponinfo[plr->readyweapon].ammo;
        int     ammo = plr->ammo[ammotype];
        int     armor = plr->armorpoints;

        int     health_x = HUD_HEALTH_X + 12;
        int     keys = 0;
        int     i = 0;

        if (((plr->readyweapon == wp_fist && plr->pendingweapon == wp_nochange)
            || plr->pendingweapon == wp_fist)
            && plr->powers[pw_strength])
        {
            hudfunc(HUD_HEALTH_X - 14, HUD_HEALTH_Y - 2, 0,
                    W_CacheLumpNum(W_GetNumForName("PSTRA0"), PU_CACHE));
        }
        else
        {
            hudfunc(HUD_HEALTH_X - 14, HUD_HEALTH_Y - 2, 0,
                    W_CacheLumpNum(W_GetNumForName("MEDIA0"), PU_CACHE));
        }

        if (health < 10)
            health_x -= 14;
        if (health < 100)
            health_x -= 14;

        DrawHUDNumber(health_x, HUD_HEALTH_Y, health);
        hudnumfunc(health_x + 50, HUD_HEALTH_Y, 0,
                   W_CacheLumpNum(W_GetNumForName("STTPRCNT"), PU_CACHE));

        if (plr->pendingweapon != wp_nochange)
        {
            ammotype = weaponinfo[plr->pendingweapon].ammo;
            ammo = plr->ammo[ammotype];
        }

        if (ammo && ammotype != am_noammo)
        {
            int ammopic_x = HUD_AMMO_X + ammopic[ammotype].x;
            int ammonum_x = HUD_AMMO_X + 8;

            if (ammo < 10)
            {
                ammopic_x += 7;
                ammonum_x -= 7;
            }
            if (ammo < 100)
            {
                ammopic_x += 7;
                ammonum_x -= 7;
            }

            hudfunc(ammopic_x, HUD_AMMO_Y + ammopic[ammotype].y, 0,
                    W_CacheLumpNum(W_GetNumForName(ammopic[ammotype].lump), PU_CACHE));
            DrawHUDNumber(ammonum_x, HUD_AMMO_Y, ammo);
        }

        while (i < NUMCARDS)
            if (plr->cards[i++])
                keys++;

        if (keys)
        {
            int            keypic_x = HUD_KEYS_X - 20 * (keys - 1);
            static int     keyanimcounter = 12;
            static boolean showkey = false;

            if (!armor)
                keypic_x += 111;
            else if (armor < 10)
                keypic_x += 26;
            else if (armor < 100)
                keypic_x += 12;

            if (plr->neededcardtics)
            {
                patch_t *patch = W_CacheLumpNum(W_GetNumForName(keypic[plr->neededcard]), PU_CACHE);

                keypic_x -= 20;
                if (!menuactive && !paused)
                {
                    plr->neededcardtics--;
                    if (!--keyanimcounter)
                    {
                        showkey = !showkey;
                        keyanimcounter = 12;
                    }
                }
                if (showkey)
                    hudfunc(keypic_x, HUD_KEYS_Y, 0, patch);
                keypic_x += patch->width + 6;
            }
            for (i = 0; i < NUMCARDS; i++)
                if (plr->cards[i])
                {
                    patch_t *patch = W_CacheLumpNum(W_GetNumForName(keypic[i]), PU_CACHE);

                    hudfunc(keypic_x, HUD_KEYS_Y, 0, patch);
                    keypic_x += patch->width + 6;
                }
        }

        if (armor)
        {
            DrawHUDNumber(HUD_ARMOR_X, HUD_ARMOR_Y, armor);
            hudnumfunc(HUD_ARMOR_X + 50, HUD_ARMOR_Y, 0,
                       W_CacheLumpNum(W_GetNumForName("STTPRCNT"), PU_CACHE));
            if (plr->armortype == 1)
                hudfunc(HUD_ARMOR_X + 70, HUD_ARMOR_Y - 1, 0,
                        W_CacheLumpNum(W_GetNumForName("ARM1A0"), PU_CACHE));
            else
                hudfunc(HUD_ARMOR_X + 70, HUD_ARMOR_Y - 1, 0,
                        W_CacheLumpNum(W_GetNumForName("ARM2A0"), PU_CACHE));
        }
    }
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
        HUlib_drawTextLine(&w_title);
    }

    if (widescreen && widescreenhud && !automapactive)
        DrawHUD();
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
    static int  lasttic = -1;
    int         i, rc, tic, fps;
    char        c;
    static char fps_str[8] = "";

    // tick down message counter if message is up
    if (((!menuactive && !paused) || demoplayback || message_dontpause) && !idbehold
        && !(players[consoleplayer].cheats & CF_MYPOS) && !devparm && message_counter
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
        char    buffer[80];
        int     angle;
        int     x, y, z;

        if (!message_counter)
            message_counter = HU_MSGTIMEOUT;
        else if (message_counter > 132)
            message_counter--;

        if (automapactive && !followplayer)
        {
            angle = direction;
            x = (m_x + (m_w >> 1)) / FRACUNIT;
            y = (m_y + (m_h >> 1)) / FRACUNIT;
            z = R_PointInSubsector(m_x + (m_w >> 1), m_y + (m_h >> 1))->sector->floorheight / FRACUNIT;
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
            if (i != consoleplayer && (c = players[i].cmd.chatchar))
            {
                if (c <= HU_BROADCAST)
                    chat_dest[i] = c;
                else
                {
                    rc = HUlib_keyInIText(&w_inputbuffer[i], c);
                    if (rc && c == KEY_ENTER)
                    {
                        if (w_inputbuffer[i].l.len
                            && (chat_dest[i] == consoleplayer + 1 || chat_dest[i] == HU_BROADCAST))
                        {
                            HUlib_addMessageToSText(&w_message, player_names[i], w_inputbuffer[i].l.l);

                            message_nottobefuckedwith = true;
                            message_on = true;
                            message_counter = HU_MSGTIMEOUT;
                            S_StartSound(NULL, gamemode == commercial ? sfx_radio : sfx_tink);
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
        plr->message = HUSTR_MSGU;
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
        c = 0;

    return c;
}

boolean HU_Responder(event_t *ev)
{

    static char         lastmessage[HU_MAXLINELENGTH + 1];
    char                *macromessage;
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
        return false;
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
                HU_queueChatChar(c);
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
