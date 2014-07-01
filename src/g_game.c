/*
====================================================================

DOOM RETRO
The classic, refined DOOM source port. For Windows PC.

Copyright (C) 1993-1996 id Software LLC, a ZeniMax Media company.
Copyright (C) 2005-2014 Simon Howard.
Copyright (C) 2013-2014 Brad Harding.

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

#ifdef _WIN32
#include <Windows.h>
#include <Xinput.h>
#endif

#include "am_map.h"
#include "d_main.h"
#include "doomstat.h"
#include "dstrings.h"
#include "f_finale.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_saveg.h"
#include "p_setup.h"
#include "p_tick.h"
#include "r_sky.h"
#include "s_sound.h"
#include "SDL.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "z_zone.h"

void G_PlayerReborn(int player);

void G_DoReborn(int playernum);

void G_DoLoadLevel(void);
void G_DoNewGame(void);
void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);
void G_DoSaveGame(void);

// Gamestate the last time G_Ticker was called.

gamestate_t     oldgamestate;

gameaction_t    gameaction;
gamestate_t     gamestate = GS_TITLESCREEN;
skill_t         gameskill;
boolean         respawnmonsters;
int             gameepisode;
int             gamemap;

// If non-zero, exit the level after this number of minutes.

int             timelimit;

boolean         paused;
boolean         sendpause;              // send a pause event next tic
boolean         sendsave;               // send a save event next tic
boolean         usergame;               // ok to save / end game

boolean         viewactive;

int             deathmatch;             // only if started as net death
boolean         netgame;                // only true if packets are broadcast
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];

boolean         turbodetected[MAXPLAYERS];

int             consoleplayer;          // player taking events and displaying
int             displayplayer;          // view being displayed
int             gametic;
int             levelstarttic;          // gametic at level start
int             totalkills, totalitems, totalsecret;    // for intermission

boolean         precache = true;        // if true, load all graphics at start

wbstartstruct_t wminfo;                 // parms for world map / intermission

byte            consistency[MAXPLAYERS][BACKUPTICS];

//
// controls (have defaults)
//
int             key_right = KEY_RIGHTARROW;
int             key_left = KEY_LEFTARROW;

int             key_up = KEY_UPARROW;
int             key_up2 = 'w';
int             key_down = KEY_DOWNARROW;
int             key_down2 = 's';
int             key_strafeleft = 'a';
int             key_straferight = 'd';
int             key_fire = KEY_RCTRL;
int             key_use = ' ';
int             key_strafe = KEY_RALT;
int             key_speed = KEY_RSHIFT;

int             key_weapon1 = '1';
int             key_weapon2 = '2';
int             key_weapon3 = '3';
int             key_weapon4 = '4';
int             key_weapon5 = '5';
int             key_weapon6 = '6';
int             key_weapon7 = '7';
int             key_prevweapon = 0;
int             key_nextweapon = 0;

int             key_pause = KEY_PAUSE;

int             mousebfire = 0;
int             mousewheelup = 3;

int             gamepadautomap = GAMEPAD_BACK;
int             gamepadfire = GAMEPAD_RIGHT_TRIGGER;
int             gamepadmenu = GAMEPAD_START;
int             gamepadnextweapon = GAMEPAD_B;
int             gamepadprevweapon = GAMEPAD_Y;
int             gamepadspeed = GAMEPAD_LEFT_TRIGGER;
int             gamepaduse = GAMEPAD_A;
int             gamepadweapon1 = 0;
int             gamepadweapon2 = 0;
int             gamepadweapon3 = 0;
int             gamepadweapon4 = 0;
int             gamepadweapon5 = 0;
int             gamepadweapon6 = 0;
int             gamepadweapon7 = 0;

int             gamepadlefthanded = 0;
int             gamepadvibrate = 1;

#define MAXPLMOVE       forwardmove[1]

#define TURBOTHRESHOLD  0x32

fixed_t         forwardmove[2] = { 0x19, 0x32 };
fixed_t         sidemove[2] = { 0x18, 0x28 };
fixed_t         angleturn[3] = { 640, 1280, 320 };      // + slow turn
fixed_t         gamepadangleturn[2] = { 640, 960 };

#define NUMWEAPONKEYS   7

static int *weapon_keys[] =
{
    &key_weapon1,
    &key_weapon2,
    &key_weapon3,
    &key_weapon4,
    &key_weapon5,
    &key_weapon6,
    &key_weapon7
};

static int *gamepadweapons[] =
{
    &gamepadweapon1,
    &gamepadweapon2,
    &gamepadweapon3,
    &gamepadweapon4,
    &gamepadweapon5,
    &gamepadweapon6,
    &gamepadweapon7
};

#define SLOWTURNTICS    6

#define NUMKEYS         256

static boolean  gamekeydown[NUMKEYS];
static int      turnheld;               // for accelerative turning

static boolean  mousearray[MAX_MOUSE_BUTTONS + 1];
static boolean  *mousebuttons = &mousearray[1]; // allow [-1]

int             mousex;

static int      dclicktime;
static boolean  dclickstate;
static int      dclicks;
static int      dclicktime2;
static boolean  dclickstate2;
static int      dclicks2;

static int      savegameslot;
static char     savedescription[32];

int G_CmdChecksum(ticcmd_t *cmd)
{
    size_t      i;
    int         sum = 0;

    for (i = 0; i < sizeof(*cmd) / 4 - 1; i++)
        sum += ((int *)cmd)[i];

    return sum;
}

static boolean G_GetSpeedToggle(void)
{
    SDLMod modstate = SDL_GetModState();
    boolean lt = (gamepadbuttons & gamepadspeed);
    boolean caps = (modstate & KMOD_CAPS);
    boolean shift = gamekeydown[key_speed];

    return ((lt ? 1 : 0) + (caps ? 1 : 0) + (shift ? 1 : 0) == 1);
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs.
//
void G_BuildTiccmd(ticcmd_t *cmd, int maketic)
{
    int         i;
    boolean     strafe;
    int         speed;
    int         forward = 0;
    int         side = 0;

    memset(cmd, 0, sizeof(ticcmd_t));
    cmd->consistency = consistency[consoleplayer][maketic % BACKUPTICS];

    if (automapactive && !followplayer)
        return;

    strafe = gamekeydown[key_strafe];

    speed = G_GetSpeedToggle();

    // use two stage accelerative turning
    // on the keyboard
    if (gamekeydown[key_right] || gamekeydown[key_left])
        turnheld += ticdup;
    else
        turnheld = 0;

    // let movement keys cancel each other out
    if (strafe)
    {
        if (gamekeydown[key_right])
            side += sidemove[speed];

        if (gamekeydown[key_left])
            side -= sidemove[speed];
    }
    else
    {
        int tspeed = speed;

        if (turnheld < SLOWTURNTICS)
            tspeed = 2;         // slow turn

        if (gamekeydown[key_right])
            cmd->angleturn -= angleturn[tspeed];
        else if (gamepadthumbRX > 0)
            cmd->angleturn -= (int)(gamepadangleturn[speed] * gamepadthumbRXright * gamepadSensitivity);

        if (gamekeydown[key_left])
            cmd->angleturn += angleturn[tspeed];
        else if (gamepadthumbRX < 0)
            cmd->angleturn += (int)(gamepadangleturn[speed] * gamepadthumbRXleft * gamepadSensitivity);
    }

    if (gamekeydown[key_up] || gamekeydown[key_up2])
        forward += forwardmove[speed];
    else if (gamepadthumbLY < 0)
        forward += (int)(forwardmove[speed] * gamepadthumbLYup);

    if (gamekeydown[key_down] || gamekeydown[key_down2])
        forward -= forwardmove[speed];
    else if (gamepadthumbLY > 0)
        forward -= (int)(forwardmove[speed] * gamepadthumbLYdown);

    if (gamekeydown[key_straferight])
        side += sidemove[speed];
    else if (gamepadthumbLX > 0)
        side += (int)(sidemove[speed] * gamepadthumbLXright);

    if (gamekeydown[key_strafeleft])
        side -= sidemove[speed];
    else if (gamepadthumbLX < 0)
        side -= (int)(sidemove[speed] * gamepadthumbLXleft);

    // buttons
    if (gamekeydown[key_fire] || mousebuttons[mousebfire] || (gamepadbuttons & gamepadfire))
        cmd->buttons |= BT_ATTACK;

    if (gamekeydown[key_use] || (gamepadbuttons & gamepaduse))
        cmd->buttons |= BT_USE;

    if (!idclev && !idmus)
    {
        for (i = 0; i < NUMWEAPONKEYS; i++)
        {
            int key = *weapon_keys[i];

            if (gamekeydown[key] && !keydown)
            {
                keydown = key;
                cmd->buttons |= BT_CHANGE;
                cmd->buttons |= i << BT_WEAPONSHIFT;
                break;
            }
            else if (gamepadbuttons & *gamepadweapons[i])
            {
                if (players[consoleplayer].readyweapon != i
                    || (i == wp_fist && players[consoleplayer].weaponowned[wp_chainsaw])
                    || (i == wp_shotgun && players[consoleplayer].weaponowned[wp_supershotgun]))
                {
                    cmd->buttons |= BT_CHANGE;
                    cmd->buttons |= i << BT_WEAPONSHIFT;
                    break;
                }
            }
        }
    }

    if (strafe)
        side += mousex * 2;
    else
        cmd->angleturn -= mousex * 0x8;

    mousex = 0;

    cmd->forwardmove += MAX(-MAXPLMOVE, MIN(forward, MAXPLMOVE));
    cmd->sidemove += MAX(-MAXPLMOVE, MIN(side, MAXPLMOVE));

    // special buttons
    if (sendpause)
    {
        sendpause = false;
        cmd->buttons = (BT_SPECIAL | BTS_PAUSE);
    }

    if (sendsave)
    {
        sendsave = false;
        cmd->buttons = (BT_SPECIAL | BTS_SAVEGAME | (savegameslot << BTS_SAVESHIFT));
    }
}

//
// G_DoLoadLevel
//

void G_DoLoadLevel(void)
{
    int         i;

    // Set the sky map.
    // First thing, we have a dummy sky texture name,
    //  a flat. The data is in the WAD only because
    //  we look for an actual index, instead of simply
    //  setting one.

    skyflatnum = R_FlatNumForName(SKYFLATNAME);

    if (gamemode == commercial)
    {
        skytexture = R_TextureNumForName("SKY3");
        if (gamemap < 12)
            skytexture = R_TextureNumForName("SKY1");
        else if (gamemap < 21)
            skytexture = R_TextureNumForName("SKY2");
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
            case 4: // Special Edition sky
                skytexture = R_TextureNumForName("SKY4");
                break;
        }
    }

    respawnmonsters = (gameskill == sk_nightmare || respawnparm);

    levelstarttic = gametic;                    // for time calculation

    if (wipegamestate == GS_LEVEL)
        wipegamestate = (gamestate_t)(-1);      // force a wipe

    gamestate = GS_LEVEL;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        turbodetected[i] = false;
        if (playeringame[i] && players[i].playerstate == PST_DEAD)
            players[i].playerstate = PST_REBORN;
        memset(players[i].frags, 0, sizeof(players[i].frags));
    }

    M_ClearRandom();

    // initialize the msecnode_t freelist.                     phares 3/25/98
    // any nodes in the freelist are gone by now, cleared
    // by Z_FreeTags() when the previous level ended or player
    // died.
    P_FreeSecNodeList();

    P_SetupLevel(gameepisode, gamemap);

    skycolfunc = (canmodify && (gamemode != commercial || gamemap < 21) ?
                  R_DrawFlippedSkyColumn : R_DrawSkyColumn);

    displayplayer = consoleplayer;              // view the guy you are playing
    gameaction = ga_nothing;

    // clear cmd building stuff
    memset(gamekeydown, 0, sizeof(gamekeydown));
    mousex = 0;
    sendpause = sendsave = paused = false;
    memset(mousearray, 0, sizeof(mousearray));

    SDL_WM_SetCaption(mapnumandtitle, NULL);

    if (automapactive)
        AM_Start();

    if ((fullscreen && widescreen) || returntowidescreen)
        ToggleWideScreen(true);
}

static void SetMouseButtons(unsigned int buttons_mask)
{
    int i;

    for (i = 0; i < MAX_MOUSE_BUTTONS; ++i)
        mousebuttons[i] = ((buttons_mask & (1 << i)) != 0);
}

struct
{
    weapontype_t        prev;
    weapontype_t        next;
    ammotype_t          ammotype;
    int                 minammo;
} weapons[] = {
    { wp_bfg,          /* wp_fist         */ wp_chainsaw,     am_noammo, 0  },
    { wp_chainsaw,     /* wp_pistol       */ wp_shotgun,      am_clip,   1  },
    { wp_pistol,       /* wp_shotgun      */ wp_supershotgun, am_shell,  1  },
    { wp_supershotgun, /* wp_chaingun     */ wp_missile,      am_clip,   1  },
    { wp_chaingun,     /* wp_missile      */ wp_plasma,       am_misl,   1  },
    { wp_missile,      /* wp_plasma       */ wp_bfg,          am_cell,   1  },
    { wp_plasma,       /* wp_bfg          */ wp_fist,         am_cell,   40 },
    { wp_fist,         /* wp_chainsaw     */ wp_pistol,       am_noammo, 0  },
    { wp_shotgun,      /* wp_supershotgun */ wp_chaingun,     am_shell,  2  }
};

void G_RemoveChoppers(void)
{
    player_t            *player = &players[consoleplayer];

    player->cheats &= ~CF_CHOPPERS;
    if (player->invulnbeforechoppers)
        player->powers[pw_invulnerability] = player->invulnbeforechoppers;
    else
        player->powers[pw_invulnerability] = STARTFLASHING;
    player->weaponowned[wp_chainsaw] = player->chainsawbeforechoppers;
    oldweaponsowned[wp_chainsaw] = player->chainsawbeforechoppers;
}

void NextWeapon(void)
{
    player_t            *player = &players[consoleplayer];
    weapontype_t        i = player->readyweapon;

    do
    {
        i = weapons[i].next;
        if (i == wp_fist && player->weaponowned[wp_chainsaw] && !player->powers[pw_strength])
            i = wp_chainsaw;
    }
    while (!player->weaponowned[i] || player->ammo[weapons[i].ammotype] < weapons[i].minammo);

    if (i != player->readyweapon)
        player->pendingweapon = i;

    if ((player->cheats & CF_CHOPPERS) && i != wp_chainsaw)
        G_RemoveChoppers();
}

void PrevWeapon(void)
{
    player_t            *player = &players[consoleplayer];
    weapontype_t        i = player->readyweapon;

    do
    {
        i = weapons[i].prev;
        if (i == wp_fist && player->weaponowned[wp_chainsaw] && !player->powers[pw_strength])
            i = wp_bfg;
    }
    while (!player->weaponowned[i] || player->ammo[weapons[i].ammotype] < weapons[i].minammo);

    if (i != player->readyweapon)
        player->pendingweapon = i;

    if ((player->cheats & CF_CHOPPERS) && i != wp_chainsaw)
        G_RemoveChoppers();
}

//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//
boolean G_Responder(event_t *ev)
{
    // any other key pops up menu if on title screen
    if (gameaction == ga_nothing && gamestate == GS_TITLESCREEN)
    {
        if (ev->type == ev_keydown && ev->data1 == key_pause && !keydown)
        {
            keydown = ev->data1;
            paused ^= 1;
            if (paused)
            {
                S_StopSounds();
                S_StartSound(NULL, sfx_swtchn);
                S_PauseSound();
                blurred = false;
            }
            else
            {
                S_ResumeSound();
                S_StartSound(NULL, sfx_swtchx);
            }
            return true;
        }

        if (ev->type == ev_keyup)
            keydown = 0;

        if (!menuactive
            && ((ev->type == ev_keydown
                 && ev->data1 != KEY_PAUSE
                 && ev->data1 != KEY_RSHIFT
                 && ev->data1 != KEY_RALT
                 && ev->data1 != KEY_CAPSLOCK
                 && ev->data1 != KEY_NUMLOCK
                 && (ev->data1 < KEY_F1 || ev->data1 > KEY_F12))
                     || (ev->type == ev_mouse
                         && (ev->data1 && !(ev->data1 & MOUSE_WHEELUP || ev->data1 & MOUSE_WHEELDOWN)))
                             || (ev->type == ev_gamepad
                                 && gamepadwait < I_GetTime()
                                 && gamepadbuttons
                                 && !(gamepadbuttons & (GAMEPAD_DPAD_UP | GAMEPAD_DPAD_DOWN
                                                        | GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD_RIGHT))))
                 && !keydown)
        {
            keydown = ev->data1;
            gamepadbuttons = 0;
            gamepadwait = I_GetTime() + 8;
            M_StartControlPanel();
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        return false;
    }

    if (gamestate == GS_LEVEL)
    {
        if (ST_Responder(ev))
            return true;        // status window ate it
        if (AM_Responder(ev))
            return true;        // automap ate it
    }

    if (gamestate == GS_FINALE)
    {
        if (F_Responder(ev))
            return true;        // finale ate the event
    }

    switch (ev->type)
    {
        case ev_keydown:
            if (ev->data1 == key_prevweapon && !menuactive && !paused)
                PrevWeapon();
            else if (ev->data1 == key_nextweapon && !menuactive && !paused)
                NextWeapon();
            else if (ev->data1 == key_pause && !menuactive && !keydown)
            {
                keydown = key_pause;
                sendpause = true;
                blurred = false;
            }
            else if (ev->data1 < NUMKEYS)
            {
                gamekeydown[ev->data1] = true;
                vibrate = false;
            }
            return true;            // eat key down events

        case ev_keyup:
            keydown = 0;
            if (ev->data1 < NUMKEYS)
                gamekeydown[ev->data1] = false;
            return false;           // always let key up events filter down

        case ev_mouse:
            SetMouseButtons(ev->data1);
            if (ev->data1)
                vibrate = false;
            if (!automapactive && !menuactive && !paused)
            {
                if (ev->data1 & MOUSE_WHEELDOWN)
                    NextWeapon();
                else if (ev->data1 & MOUSE_WHEELUP)
                    PrevWeapon();
            }
            if (!automapactive || (automapactive && followplayer))
                mousex = ev->data2 * mouseSensitivity / 10;
            return true;            // eat events

        case ev_gamepad:
            if (!automapactive || (automapactive && followplayer))
            {
                if ((gamepadpress && gamepadwait < I_GetTime()) || !gamepadpress)
                {
                    if (gamepadbuttons & gamepadnextweapon)
                        NextWeapon();
                    gamepadpress = false;
                }
                if (gamepadbuttons & gamepadprevweapon)
                    PrevWeapon();
            }
            return true;            // eat events

        default:
            break;
    }
    return false;
}

char    savename[256];

static byte saveg_read8(FILE *file)
{
    byte        result;

    if (fread(&result, 1, 1, file) < 1)
        return 0;
    return result;
}

boolean G_CheckSaveGame(void)
{
    FILE        *handle;
    int         episode;
    int         map;
    int         mission;
    int         i;

    handle = fopen(savename, "rb");

    for (i = 0; i < SAVESTRINGSIZE + VERSIONSIZE + 1; i++)
        saveg_read8(handle);
    episode = saveg_read8(handle);
    map = saveg_read8(handle);
    mission = saveg_read8(handle);

    fclose(handle);

    if (mission != gamemission)
        return false;
    if (episode != gameepisode || map != gamemap)
        return false;

    return true;
}

void D_Display(void);

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker(void)
{
    int         i;
    int         buf;
    ticcmd_t    *cmd;

    // do player reborns if needed
    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i] && players[i].playerstate == PST_REBORN)
            G_DoReborn(i);

    // do things to change the game state
    while (gameaction != ga_nothing)
    {
        switch (gameaction)
        {
            case ga_loadlevel:
                G_DoLoadLevel();
                break;
            case ga_reloadgame:
                M_StringCopy(savename, P_SaveGameFile(quickSaveSlot), sizeof(savename));
                if (G_CheckSaveGame())
                    G_DoLoadGame();
                else
                    G_DoLoadLevel();
                break;
            case ga_newgame:
                G_DoNewGame();
                break;
            case ga_loadgame:
                G_DoLoadGame();
                break;
            case ga_savegame:
                G_DoSaveGame();
                break;
            case ga_completed:
                G_DoCompleted();
                break;
            case ga_victory:
                F_StartFinale();
                break;
            case ga_worlddone:
                G_DoWorldDone();
                break;
            case ga_screenshot:
                if (gametic)
                {
                    if ((usergame || gamestate == GS_LEVEL)
                        && !idbehold && !(players[consoleplayer].cheats & CF_MYPOS))
                    {
                        HU_clearMessages();
                        D_Display();
                    }
                    if (V_ScreenShot())
                    {
                        static char message[512];

                        S_StartSound(NULL, sfx_swtchx);
                        if (usergame || gamestate == GS_LEVEL)
                        {
                            M_snprintf(message, sizeof(message), GSCREENSHOT, lbmname);
                            players[consoleplayer].message = message;
                            message_dontfuckwithme = true;
                            if (menuactive)
                            {
                                message_dontpause = true;
                                blurred = false;
                            }
                        }
                    }
                    gameaction = ga_nothing;
                }
                break;
            case ga_nothing:
                break;
        }
    }

    // get commands, check consistency,
    // and build new consistency check
    buf = (gametic / ticdup) % BACKUPTICS;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            cmd = &players[i].cmd;

            memcpy(cmd, &netcmds[i][buf], sizeof(ticcmd_t));

            // check for turbo cheats

            // check ~ 4 seconds whether to display the turbo message.
            // store if the turbo threshold was exceeded in any tics
            // over the past 4 seconds.  offset the checking period
            // for each player so messages are not displayed at the
            // same time.

            if (netgame)
            {
                if (cmd->forwardmove > TURBOTHRESHOLD)
                    turbodetected[i] = true;

                if (!(gametic & 31) && ((gametic >> 5) % MAXPLAYERS) == i && turbodetected[i])
                {
                    //static char turbomessage[80];
                    //extern char *player_names[4];

                    //sprintf(turbomessage, "%s is turbo!", player_names[i]);
                    //players[consoleplayer].message = turbomessage;
                    turbodetected[i] = false;
                }
            }

            if (netgame && !(gametic % ticdup))
            {
                if (gametic > BACKUPTICS
                    && consistency[i][buf] != cmd->consistency)
                {
                    I_Error("Consistency failure (%i should be %i)",
                            cmd->consistency, consistency[i][buf]);
                }
                if (players[i].mo)
                    consistency[i][buf] = players[i].mo->x;
                else
                    consistency[i][buf] = rndindex;
            }
        }
    }

    // check for special buttons
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            if (players[i].cmd.buttons & BT_SPECIAL)
            {
                switch (players[i].cmd.buttons & BT_SPECIALMASK)
                {
                    case BTS_PAUSE:
                        paused ^= 1;
                        if (paused)
                        {
                            S_StopSounds();
                            S_StartSound(NULL, sfx_swtchn);
                            S_PauseSound();

                            if (gamepadvibrate && vibrate)
                            {
                                restoremotorspeed = idlemotorspeed;
                                idlemotorspeed = 0;
                                XInputVibration(idlemotorspeed);
                            }
                        }
                        else
                        {
                            S_ResumeSound();
                            S_StartSound(NULL, sfx_swtchx);

                            if (gamepadvibrate && vibrate)
                            {
                                idlemotorspeed = restoremotorspeed;
                                XInputVibration(idlemotorspeed);
                            }
                        }
                        break;

                    case BTS_SAVEGAME:
                        if (!savedescription[0])
                            M_StringCopy(savedescription, "NET GAME", sizeof(savedescription));
                        savegameslot = (players[i].cmd.buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
                        gameaction = ga_savegame;
                        break;
                }
            }
        }
    }

    // Have we just finished displaying an intermission screen?
    if (oldgamestate == GS_INTERMISSION && gamestate != GS_INTERMISSION)
        WI_End();

    oldgamestate = gamestate;

    // do main actions
    switch (gamestate)
    {
        case GS_LEVEL:
            P_Ticker();
            ST_Ticker();
            AM_Ticker();
            HU_Ticker();
            break;

        case GS_INTERMISSION:
            WI_Ticker();
            break;

        case GS_FINALE:
            F_Ticker();
            break;

        case GS_TITLESCREEN:
            D_PageTicker();
            break;
    }
}

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
void G_PlayerFinishLevel(int player)
{
    player_t    *p = &players[player];

    memset(p->powers, 0, sizeof(p->powers));
    memset(p->cards, 0, sizeof(p->cards));
    p->mo->flags &= ~MF_SHADOW;         // cancel invisibility
    p->extralight = 0;                  // cancel gun flashes
    p->fixedcolormap = 0;               // cancel ir gogles
    p->damagecount = 0;                 // no palette changes
    p->bonuscount = 0;
}

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//
void G_PlayerReborn(int player)
{
    player_t    *p;
    int         i;
    int         frags[MAXPLAYERS];
    int         killcount;
    int         itemcount;
    int         secretcount;

    memcpy(frags, players[player].frags, sizeof(frags));
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;

    p = &players[player];
    memset(p, 0, sizeof(*p));

    memcpy(players[player].frags, frags, sizeof(players[player].frags));
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;

    p->usedown = p->attackdown = true;          // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = 100;
    p->readyweapon = p->pendingweapon = wp_pistol;
    p->preferredshotgun = wp_shotgun;
    p->fistorchainsaw = wp_fist;
    p->shotguns = false;
    p->weaponowned[wp_fist] = true;
    p->weaponowned[wp_pistol] = true;
    p->ammo[am_clip] = 50;

    for (i = 0; i < NUMAMMO; i++)
        p->maxammo[i] = (gamemode == shareware && i == am_cell ? 0 : maxammo[i]);

    markpointnum = 0;
    infight = false;
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//
void P_SpawnPlayer(mapthing_t *mthing);

boolean G_CheckSpot(int playernum, mapthing_t *mthing)
{
    fixed_t             x;
    fixed_t             y;
    subsector_t         *ss;
    unsigned int        an;
    mobj_t              *mo;
    int                 i;
    fixed_t             xa;
    fixed_t             ya;

    flag667 = false;

    if (!players[playernum].mo)
    {
        // first spawn of level, before corpses
        for (i = 0; i < playernum; i++)
            if (players[i].mo->x == mthing->x << FRACBITS
                && players[i].mo->y == mthing->y << FRACBITS)
                return false;
        return true;
    }

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    players[playernum].mo->flags |=  MF_SOLID;
    i = P_CheckPosition(players[playernum].mo, x, y);
    players[playernum].mo->flags &= ~MF_SOLID;
    if (!i)
        return false;

    // spawn a teleport fog
    ss = R_PointInSubsector(x, y);
    an = (ANG45 * ((signed)mthing->angle / 45)) >> ANGLETOFINESHIFT;
    xa = finecosine[an];
    ya = finesine[an];

    switch (an)
    {
        case -4096:
            xa = finetangent[2048];
            ya = finetangent[0];
            break;
        case -3072:
            xa = finetangent[3072];
            ya = finetangent[1024];
            break;
        case -2048:
            xa = finesine[0];
            ya = finetangent[2048];
            break;
        case -1024:
            xa = finesine[1024];
            ya = finetangent[3072];
            break;
        case 1024:
        case 2048:
        case 3072:
        case 4096:
        case 0:
            break;
        default:
            I_Error("G_CheckSpot: unexpected angle %d\n", an);
    }

    mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya, ss->sector->floorheight, MT_TFOG);
    mo->angle = mthing->angle;

    if (players[consoleplayer].viewz != 1)
        S_StartSound(mo, sfx_telept);           // don't start sound on first frame

    return true;
}

//
// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//
void G_DeathMatchSpawnPlayer(int playernum)
{
    int         i, j;
    int         selections;

    selections = deathmatch_p - deathmatchstarts;
    if (selections < 4)
        I_Error("Only %i deathmatch spots, 4 required", selections);

    for (j = 0; j < 20; j++)
    {
        i = P_Random() % selections;
        if (G_CheckSpot(playernum, &deathmatchstarts[i]))
        {
            deathmatchstarts[i].type = playernum + 1;
            P_SpawnPlayer(&deathmatchstarts[i]);
            return;
        }
    }

    // no good spot, so the player will probably get stuck
    P_SpawnPlayer(&playerstarts[playernum]);
}

//
// G_DoReborn
//
void G_DoReborn(int playernum)
{
    if (!netgame)
        gameaction = (quickSaveSlot < 0 ? ga_loadlevel : ga_reloadgame);
    else
    {
        // respawn at the start
        int     i;

        // first dissasociate the corpse
        players[playernum].mo->player = NULL;

        // spawn at random spot if in death match
        if (deathmatch)
        {
            G_DeathMatchSpawnPlayer(playernum);
            return;
        }

        if (G_CheckSpot(playernum, &playerstarts[playernum]))
        {
            P_SpawnPlayer(&playerstarts[playernum]);
            return;
        }

        // try to spawn at one of the other players spots
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (G_CheckSpot(playernum, &playerstarts[i]))
            {
                playerstarts[i].type = playernum + 1;           // fake as other player
                P_SpawnPlayer(&playerstarts[i]);
                playerstarts[i].type = i + 1;                   // restore
                return;
            }
            // he's going to be inside something.  Too bad.
        }
        P_SpawnPlayer(&playerstarts[playernum]);
    }
}

void G_ScreenShot(void)
{
    gameaction = ga_screenshot;
}

// DOOM Par Times
int pars[5][10] =
{
    { 0 },
    { 0,  30,  75, 120,  90, 165, 180, 180,  30, 165 },
    { 0,  90,  90,  90, 120,  90, 360, 240,  30, 170 },
    { 0,  90,  45,  90, 150,  90,  90, 165,  30, 135 },
    // [BH] Episode 4 Par Times
    { 0, 165, 255, 135, 150, 180, 390, 135, 360, 180 }
};

// DOOM II Par Times
int cpars[33] =
{
     30,  90, 120, 120,  90, 150, 120, 120, 270,  90,   //  1-10
    210, 150, 150, 150, 210, 150, 420, 150, 210, 150,   // 11-20
    240, 150, 180, 150, 150, 300, 330, 420, 300, 180,   // 21-30
    120,  30,   0                                       // 31-33
};

// [BH] No Rest For The Living Par Times
int npars[9] =
{
    75, 105, 120, 105, 210, 105, 165, 105, 135
};

//
// G_DoCompleted
//
boolean         secretexit;
extern char     *pagename;

void G_ExitLevel(void)
{
    secretexit = false;
    gameaction = ga_completed;
}

// Here's for the german edition.
void G_SecretExitLevel(void)
{
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    if (gamemode == commercial && W_CheckNumForName("map31") < 0)
        secretexit = false;
    else
        secretexit = true;
    gameaction = ga_completed;
}

extern int      selectedepisode;
extern menu_t   EpiDef;

void ST_doRefresh(void);

void G_DoCompleted(void)
{
    int         i;
    char        lump[5];

    gameaction = ga_nothing;

    // [BH] allow the exit switch to turn on before the screen wipes
    R_RenderPlayerView(&players[displayplayer]);

    if (widescreen)
    {
        ToggleWideScreen(false);
        returntowidescreen = true;
        ST_doRefresh();
    }

    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i])
            G_PlayerFinishLevel(i);             // take away cards and stuff

    if (automapactive)
        AM_Stop();

    if (gamemode != commercial)
    {
        switch (gamemap)
        {
            case 8:
                // [BH] this episode is complete, so select the next episode in the menu
                if ((gamemode == registered && gameepisode < 3)
                    || (gamemode == retail && gameepisode < 4))
                {
                    selectedepisode = gameepisode;
                    EpiDef.lastOn = selectedepisode;
                }
                gameaction = ga_victory;
                return;
            case 9:
                for (i = 0; i < MAXPLAYERS; i++)
                    players[i].didsecret = true;
                break;
        }
    }

    wminfo.didsecret = players[consoleplayer].didsecret;
    wminfo.epsd = gameepisode - 1;
    wminfo.last = gamemap - 1;

    // wminfo.next is 0 biased, unlike gamemap
    if (gamemode == commercial)
    {
        if (secretexit)
        {
            switch (gamemap)
            {
                case 2:
                    // [BH] exit to secret level on MAP02 of BFG Edition
                    wminfo.next = 32;
                    break;
                case 4:
                    // [BH] exit to secret level in No Rest For The Living
                    if (gamemission == pack_nerve)
                        wminfo.next = 8;
                    break;
                case 15:
                    wminfo.next = 30;
                    break;
                case 31:
                    wminfo.next = 31;
                    break;
            }
        }
        else
        {
            switch (gamemap)
            {
                case 9:
                    // [BH] return to MAP05 after secret level in No Rest For The Living
                    wminfo.next = (gamemission == pack_nerve ? 4 : gamemap);
                    break;
                case 31:
                case 32:
                    wminfo.next = 15;
                    break;
                case 33:
                    // [BH] return to MAP03 after secret level in BFG Edition
                    wminfo.next = 2;
                    break;
                default:
                   wminfo.next = gamemap;
                   break;
            }
        }
    }
    else
    {
        if (secretexit)
            wminfo.next = 8;            // go to secret level
        else if (gamemap == 9)
        {
            // returning from secret level
            switch (gameepisode)
            {
                case 1:
                    wminfo.next = 3;
                    break;
                case 2:
                    wminfo.next = 5;
                    break;
                case 3:
                    wminfo.next = 6;
                    break;
                case 4:
                    wminfo.next = 2;
                    break;
            }
        }
        else
            wminfo.next = gamemap;      // go to next level
    }

    wminfo.maxkills = totalkills;
    wminfo.maxitems = totalitems;
    wminfo.maxsecret = totalsecret;
    wminfo.maxfrags = 0;

    // [BH] have no par time if this level is from a PWAD
    if (gamemode == commercial)
        M_snprintf(lump, sizeof(lump), "MAP%02i", gamemap);
    else
        M_snprintf(lump, sizeof(lump), "E%iM%i", gameepisode, gamemap);
    if (W_CheckMultipleLumps(lump) > 1 && (!nerve || gamemap > 9))
        wminfo.partime = 0;
    else if (gamemode == commercial)
    {
        // [BH] get correct par time for No Rest For The Living
        //  and have no par time for TNT and Plutonia
        if (gamemission == pack_nerve && gamemap <= 9)
            wminfo.partime = TICRATE * npars[gamemap - 1];
        else if (gamemission == pack_tnt || gamemission == pack_plut)
            wminfo.partime = 0;
        else
            wminfo.partime = TICRATE * cpars[gamemap - 1];
    }
    else
        wminfo.partime = TICRATE * pars[gameepisode][gamemap];

    wminfo.pnum = consoleplayer;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        wminfo.plyr[i].in = playeringame[i];
        wminfo.plyr[i].skills = players[i].killcount;
        wminfo.plyr[i].sitems = players[i].itemcount;
        wminfo.plyr[i].ssecret = players[i].secretcount;
        wminfo.plyr[i].stime = leveltime;
        memcpy(wminfo.plyr[i].frags, players[i].frags,
               sizeof(wminfo.plyr[i].frags));
    }

    gamestate = GS_INTERMISSION;
    viewactive = false;
    automapactive = false;

    WI_Start(&wminfo);
}

//
// G_WorldDone
//
void G_WorldDone(void)
{
    gameaction = ga_worlddone;

    if (secretexit)
        players[consoleplayer].didsecret = true;

    if (gamemode == commercial)
    {
        if (gamemission == pack_nerve)
        {
            if (gamemap == 8)
                F_StartFinale();
        }
        else
        {
            switch (gamemap)
            {
                case 15:
                case 31:
                    if (!secretexit)
                        break;
                case 6:
                case 11:
                case 20:
                case 30:
                    F_StartFinale();
                    break;
            }
        }
    }
}

void G_DoWorldDone(void)
{
    gamestate = GS_LEVEL;
    gamemap = wminfo.next + 1;
    G_DoLoadLevel();
    gameaction = ga_nothing;
    viewactive = true;
    markpointnum = 0;
}

//
// G_InitFromSavegame
// Can be called by the startup code or the menu task.
//
extern boolean setsizeneeded;
void R_ExecuteSetViewSize(void);

void G_LoadGame(char *name)
{
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame;
}

void G_DoLoadGame(void)
{
    int savedleveltime;

    gameaction = ga_nothing;

    save_stream = fopen(savename, "rb");

    if (save_stream == NULL)
        return;

    savegame_error = false;

    if (!P_ReadSaveGameHeader())
    {
        fclose(save_stream);
        return;
    }

    savedleveltime = leveltime;

    // load a base level
    G_InitNew(gameskill, gameepisode, gamemap);

    leveltime = savedleveltime;

    // dearchive all the modifications
    P_UnArchivePlayers();
    P_UnArchiveWorld();
    P_UnArchiveThinkers();
    P_UnArchiveSpecials();

    if (!P_ReadSaveGameEOF())
        I_Error("Bad savegame");

    fclose(save_stream);

    if (setsizeneeded)
        R_ExecuteSetViewSize();

    if (fullscreen && widescreen)
        ToggleWideScreen(true);

    // draw the pattern into the back screen
    R_FillBackScreen();
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 256 byte text string
//
void G_SaveGame(int slot, char *description)
{
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
}

void G_DoSaveGame(void)
{
    char        *savegame_file;
    char        *temp_savegame_file;
    static char buffer[128];

    temp_savegame_file = P_TempSaveGameFile();
    savegame_file = P_SaveGameFile(savegameslot);

    // Open the savegame file for writing.  We write to a temporary file
    // and then rename it at the end if it was successfully written.
    // This prevents an existing savegame from being overwritten by
    // a corrupted one, or if a savegame buffer overrun occurs.
    save_stream = fopen(temp_savegame_file, "wb");

    if (save_stream == NULL)
        return;

    savegame_error = false;

    P_WriteSaveGameHeader(savedescription);

    P_ArchivePlayers();
    P_ArchiveWorld();
    P_ArchiveThinkers();
    P_ArchiveSpecials();

    P_WriteSaveGameEOF();

    // Finish up, close the savegame file.
    fclose(save_stream);

    // Now rename the temporary savegame file to the actual savegame
    // file, overwriting the old savegame if there was one there.
    remove(savegame_file);
    rename(temp_savegame_file, savegame_file);

    // [BH] use the save description in the message displayed
    M_snprintf(buffer, sizeof(buffer), GGSAVED, savedescription);
    players[consoleplayer].message = buffer;
    message_dontfuckwithme = true;

    gameaction = ga_nothing;
    M_StringCopy(savedescription, "", sizeof(savedescription));

    // draw the pattern into the back screen
    R_FillBackScreen();
}

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//
skill_t         d_skill;
int             d_episode;
int             d_map;

void G_DeferredInitNew(skill_t skill, int episode, int map)
{
    d_skill = skill;
    d_episode = episode;
    d_map = map;
    gameaction = ga_newgame;
    markpointnum = 0;
    startingnewgame = true;
    infight = false;
}

//
// G_DeferredLoadLevel
// [BH] Called when the IDCLEV cheat is used.
//
void G_DeferredLoadLevel(skill_t skill, int episode, int map)
{
    int pnum, i;

    d_skill = skill;
    d_episode = episode;
    d_map = map;
    gameaction = ga_loadlevel;
    markpointnum = 0;
    infight = false;

    for (pnum = 0; pnum < MAXPLAYERS; ++pnum)
        if (playeringame[pnum])
        {
            player_t *player = &players[pnum];

            for (i = 0; i < NUMPOWERS; i++)
                if (player->powers[i] > 0)
                    player->powers[i] = 0;
        }
}

void G_DoNewGame(void)
{
    netgame = false;
    deathmatch = 0;
    playeringame[1] = playeringame[2] = playeringame[3] = 0;

    if (fullscreen && widescreen)
        ToggleWideScreen(true);

    consoleplayer = 0;
    st_facecount = ST_STRAIGHTFACECOUNT;
    G_InitNew(d_skill, d_episode, d_map);
    gameaction = ga_nothing;
    markpointnum = 0;
    infight = false;
}

void G_SetFastParms(int fast_pending)
{
    static int fast = 0;
    int i;

    if (fast != fast_pending)
    {
        if ((fast = fast_pending))
        {
            for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
                if (states[i].tics != 1)
                    states[i].tics >>= 1;
            mobjinfo[MT_BRUISERSHOT].speed = 20 * FRACUNIT;
            mobjinfo[MT_HEADSHOT].speed = 20 * FRACUNIT;
            mobjinfo[MT_TROOPSHOT].speed = 20 * FRACUNIT;
        }
        else
        {
            for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
                states[i].tics <<= 1;
            mobjinfo[MT_BRUISERSHOT].speed = 15 * FRACUNIT;
            mobjinfo[MT_HEADSHOT].speed = 10 * FRACUNIT;
            mobjinfo[MT_TROOPSHOT].speed = 10 * FRACUNIT;
        }
    }
}

void G_InitNew(skill_t skill, int episode, int map)
{
    int         i;

    if (paused)
    {
        paused = false;
        S_ResumeSound();
    }

    if (skill > sk_nightmare)
        skill = sk_nightmare;

    if (episode < 1)
        episode = 1;

    if (gamemode == retail)
    {
        if (episode > 4)
            episode = 4;
    }
    else if (gamemode == shareware)
    {
        if (episode > 1)
            episode = 1;        // only start episode 1 on shareware
    }
    else
    {
        if (episode > 3)
            episode = 3;
    }

    if (map < 1)
        map = 1;

    if (map > 9 && gamemode != commercial)
        map = 9;

    respawnmonsters = (skill == sk_nightmare || respawnparm);

    // [BH] Fix demon speed bug. See doomwiki.org/wiki/Demon_speed_bug.
    G_SetFastParms(fastparm || skill == sk_nightmare);

    // force players to be initialized upon first level load
    for (i = 0; i < MAXPLAYERS ; i++)
        players[i].playerstate = PST_REBORN;

    usergame = true;            // will be set false if on title screen
    paused = false;
    automapactive = false;
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;

    // [BH] Do not set the sky here.
    // See http://doom.wikia.com/wiki/Sky_never_changes_in_Doom_II.

    //if (gamemode == commercial)
    //{
    //    if (gamemap < 12)
    //        skytexturename = "SKY1";
    //    else if (gamemap < 21)
    //        skytexturename = "SKY2";
    //    else
    //        skytexturename = "SKY3";
    //}
    //else
    //{
    //    switch (gameepisode)
    //    {
    //        default:
    //        case 1:
    //            skytexturename = "SKY1";
    //            break;
    //        case 2:g
    //            skytexturename = "SKY2";
    //            break;
    //        case 3:
    //            skytexturename = "SKY3";
    //            break;
    //        case 4:           // Special Edition sky
    //            skytexturename = "SKY4";
    //            break;
    //    }
    //}

    //skytexture = R_TextureNumForName(skytexturename);

    G_DoLoadLevel();
}
