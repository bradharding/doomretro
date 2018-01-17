/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

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

#include <time.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "f_finale.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "p_saveg.h"
#include "p_setup.h"
#include "p_tick.h"
#include "r_sky.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "z_zone.h"

static void G_DoReborn(void);

static void G_DoNewGame(void);
static void G_DoCompleted(void);
static void G_DoWorldDone(void);
static void G_DoSaveGame(void);

gameaction_t    gameaction;
gamestate_t     gamestate = GS_TITLESCREEN;
skill_t         gameskill;
int             pendinggameskill;
int             gameepisode;
int             gamemap;

dboolean        paused;
dboolean        sendpause;              // send a pause event next tic
static dboolean sendsave;               // send a save event next tic

dboolean        viewactive;

int             gametic;
int             gametime;
int             totalkills;             // for intermission
int             totalitems;
int             totalsecret;
int             totalpickups;
int             monstercount[NUMMOBJTYPES];
int             barrelcount;

wbstartstruct_t wminfo;                 // parms for world map/intermission

dboolean        autoload = autoload_default;

#define MAXPLMOVE       forwardmove[1]

fixed_t  forwardmove[2] = { FORWARDMOVE0, FORWARDMOVE1 };
fixed_t  sidemove[2] = { SIDEMOVE0, SIDEMOVE1 };
fixed_t  angleturn[3] = { 640, 1280, 320 };      // + slow turn
fixed_t  gamepadangleturn[2] = { 640, 960 };

#define NUMWEAPONKEYS   7

static int *weapon_keys[] =
{
    &keyboardweapon1,
    &keyboardweapon2,
    &keyboardweapon3,
    &keyboardweapon4,
    &keyboardweapon5,
    &keyboardweapon6,
    &keyboardweapon7
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

static struct
{
    weapontype_t    prev;
    weapontype_t    next;
    ammotype_t      ammotype;
    int             minammo;
} weapons[] = {
    { wp_bfg,          /* wp_fist         */ wp_chainsaw,     am_noammo,        0 },
    { wp_chainsaw,     /* wp_pistol       */ wp_shotgun,      am_clip,          1 },
    { wp_pistol,       /* wp_shotgun      */ wp_supershotgun, am_shell,         1 },
    { wp_supershotgun, /* wp_chaingun     */ wp_missile,      am_clip,          1 },
    { wp_chaingun,     /* wp_missile      */ wp_plasma,       am_misl,          1 },
    { wp_missile,      /* wp_plasma       */ wp_bfg,          am_cell,          1 },
    { wp_plasma,       /* wp_bfg          */ wp_fist,         am_cell,   BFGCELLS },
    { wp_fist,         /* wp_chainsaw     */ wp_pistol,       am_noammo,        0 },
    { wp_shotgun,      /* wp_supershotgun */ wp_chaingun,     am_shell,         2 }
};

#define SLOWTURNTICS    6

dboolean        gamekeydown[NUMKEYS];
char            keyactionlist[NUMKEYS][255];
static int      turnheld;                       // for accelerative turning

static dboolean mousearray[MAX_MOUSE_BUTTONS + 1];
dboolean        *mousebuttons = &mousearray[1]; // allow [-1]
char            mouseactionlist[MAX_MOUSE_BUTTONS + 2][255];

dboolean        skipaction;

static int      mousex;
static int      mousey;

dboolean        m_doubleclick_use = m_doubleclick_use_default;
dboolean        m_invertyaxis = m_invertyaxis_default;
dboolean        m_novertical = m_novertical_default;
dboolean        mouselook = mouselook_default;
dboolean        canmouselook = false;
dboolean        usemouselook = false;

static int      dclicktime;
static dboolean dclickstate;
static int      dclicks;
static int      dclicktime2;
static dboolean dclickstate2;
static int      dclicks2;

static int      savegameslot;
static char     savedescription[SAVESTRINGSIZE];

gameaction_t    loadaction = ga_nothing;

unsigned int    stat_mapscompleted = 0;

extern dboolean barreltics;
extern int      st_palette;
extern int      pagetic;
extern dboolean transferredsky;

extern int      timer;
extern int      countdown;

void G_RemoveChoppers(void)
{
    viewplayer->cheats &= ~CF_CHOPPERS;
    viewplayer->powers[pw_invulnerability] = (viewplayer->invulnbeforechoppers ? 1 : STARTFLASHING);
    viewplayer->weaponowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
    oldweaponsowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
}

void G_NextWeapon(void)
{
    weapontype_t    pendingweapon = viewplayer->pendingweapon;
    weapontype_t    readyweapon = viewplayer->readyweapon;
    weapontype_t    i = (pendingweapon == wp_nochange ? readyweapon : pendingweapon);

    do
    {
        i = weapons[i].next;

        if (i == wp_fist && viewplayer->weaponowned[wp_chainsaw] && !viewplayer->powers[pw_strength])
            i = wp_chainsaw;
    } while (!viewplayer->weaponowned[i] || viewplayer->ammo[weapons[i].ammotype] < weapons[i].minammo);

    if (i != readyweapon)
        viewplayer->pendingweapon = i;

    if ((viewplayer->cheats & CF_CHOPPERS) && i != wp_chainsaw)
        G_RemoveChoppers();

    if (i == wp_fist && viewplayer->powers[pw_strength])
        S_StartSound(NULL, sfx_getpow);
}

void G_PrevWeapon(void)
{
    weapontype_t    pendingweapon = viewplayer->pendingweapon;
    weapontype_t    readyweapon = viewplayer->readyweapon;
    weapontype_t    i = (pendingweapon == wp_nochange ? readyweapon : pendingweapon);

    do
    {
        i = weapons[i].prev;

        if (i == wp_fist && viewplayer->weaponowned[wp_chainsaw] && !viewplayer->powers[pw_strength])
            i = wp_bfg;
    } while (!viewplayer->weaponowned[i] || viewplayer->ammo[weapons[i].ammotype] < weapons[i].minammo);

    if (i != readyweapon)
        viewplayer->pendingweapon = i;

    if ((viewplayer->cheats & CF_CHOPPERS) && i != wp_chainsaw)
        G_RemoveChoppers();

    if (i == wp_fist && viewplayer->powers[pw_strength])
        S_StartSound(NULL, sfx_getpow);
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs.
//
void G_BuildTiccmd(ticcmd_t *cmd)
{
    dboolean    strafe;
    int         run;
    int         forward = 0;
    int         side = 0;

    memset(cmd, 0, sizeof(ticcmd_t));

    if (automapactive && !am_followmode && viewplayer->health > 0)
        return;

    strafe = (gamekeydown[keyboardstrafe] || mousebuttons[mousestrafe] || (gamepadbuttons & gamepadstrafe));

    run = (gamekeydown[keyboardrun] ^ !!mousebuttons[mouserun] ^ !!(gamepadbuttons & gamepadrun) ^ alwaysrun);

    usemouselook = (mouselook || gamekeydown[keyboardmouselook] || mousebuttons[mousemouselook]
        || (gamepadbuttons & gamepadmouselook));

    // use two stage accelerative turning
    // on the keyboard
    if (gamekeydown[keyboardright] || gamekeydown[keyboardleft] || (gamepadbuttons & gamepadleft)
        || (gamepadbuttons & gamepadright))
        turnheld++;
    else
        turnheld = 0;

    // let movement keys cancel each other out
    if (strafe)
    {
        if (gamekeydown[keyboardright] || (gamepadbuttons & gamepadright))
            side += sidemove[run];

        if (gamekeydown[keyboardleft] || (gamepadbuttons & gamepadleft))
            side -= sidemove[run];
    }
    else
    {
        if (gamekeydown[keyboardright] || (gamepadbuttons & gamepadright))
            cmd->angleturn -= angleturn[turnheld < SLOWTURNTICS ? 2 : run];
        else if (gamepadthumbRX > 0)
            cmd->angleturn -= (int)(gamepadangleturn[run] * gamepadthumbRXright * gamepadsensitivity);

        if (gamekeydown[keyboardleft] || (gamepadbuttons & gamepadleft))
            cmd->angleturn += angleturn[turnheld < SLOWTURNTICS ? 2 : run];
        else if (gamepadthumbRX < 0)
            cmd->angleturn += (int)(gamepadangleturn[run] * gamepadthumbRXleft * gamepadsensitivity);
    }

    if (usemouselook)
    {
        if (gamepadthumbRY < 0)
            cmd->lookdir = (int)(48 * gamepadthumbRYup * gamepadsensitivity);
        else if (gamepadthumbRY > 0)
            cmd->lookdir = (int)(48 * gamepadthumbRYdown * gamepadsensitivity);
    }

    if (gamekeydown[keyboardforward] || gamekeydown[keyboardforward2] || (gamepadbuttons & gamepadforward))
        forward += forwardmove[run];
    else if (gamepadthumbLY < 0)
        forward += (int)(forwardmove[run] * gamepadthumbLYup);

    if (gamekeydown[keyboardback] || gamekeydown[keyboardback2] || (gamepadbuttons & gamepadback))
        forward -= forwardmove[run];
    else if (gamepadthumbLY > 0)
        forward -= (int)(forwardmove[run] * gamepadthumbLYdown);

    if (gamekeydown[keyboardstraferight] || gamekeydown[keyboardstraferight2]
        || (gamepadbuttons & gamepadstraferight))
        side += sidemove[run];
    else if (gamepadthumbLX > 0)
        side += (int)(sidemove[run] * gamepadthumbLXright);

    if (gamekeydown[keyboardstrafeleft] || gamekeydown[keyboardstrafeleft2]
        || (gamepadbuttons & gamepadstrafeleft))
        side -= sidemove[run];
    else if (gamepadthumbLX < 0)
        side -= (int)(sidemove[run] * gamepadthumbLXleft);

    // buttons
    if (skipaction)
        skipaction = false;
    else if (!freeze)
    {
        if ((mousebuttons[mousefire] || gamekeydown[keyboardfire] || (gamepadbuttons & gamepadfire)))
            cmd->buttons |= BT_ATTACK;

        if (gamekeydown[keyboarduse] || gamekeydown[keyboarduse2] || mousebuttons[mouseuse]
            || (gamepadbuttons & (gamepaduse | gamepaduse2)))
        {
            cmd->buttons |= BT_USE;
            dclicks = 0;        // clear double clicks if hit use button
        }
    }

    if (!idclev && !idmus)
    {
        for (int i = 0; i < NUMWEAPONKEYS; i++)
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
                if (viewplayer->readyweapon != i || (i == wp_fist && viewplayer->weaponowned[wp_chainsaw])
                    || (i == wp_shotgun && viewplayer->weaponowned[wp_supershotgun]))
                {
                    cmd->buttons |= BT_CHANGE;
                    cmd->buttons |= i << BT_WEAPONSHIFT;
                    break;
                }
            }
        }
    }

    if (mousebuttons[mouseforward])
        forward += forwardmove[run];

    if (m_doubleclick_use)
    {
        dboolean    bstrafe;

        // forward double click
        if (mousebuttons[mouseforward] != dclickstate && dclicktime > 1)
        {
            dclickstate = mousebuttons[mouseforward];

            if (dclickstate)
                dclicks++;

            if (dclicks == 2)
            {
                cmd->buttons |= BT_USE;
                dclicks = 0;
            }
            else
                dclicktime = 0;
        }
        else if (++dclicktime > 20)
        {
            dclicks = 0;
            dclickstate = 0;
        }

        // strafe double click
        bstrafe = mousebuttons[mousestrafe];

        if (bstrafe != dclickstate2 && dclicktime2 > 1)
        {
            dclickstate2 = bstrafe;

            if (dclickstate2)
                dclicks2++;

            if (dclicks2 == 2)
            {
                cmd->buttons |= BT_USE;
                dclicks2 = 0;
            }
            else
                dclicktime2 = 0;
        }
        else if (++dclicktime2 > 20)
        {
            dclicks2 = 0;
            dclickstate2 = 0;
        }
    }

    if (mousex)
    {
        if (strafe)
            side += mousex * 2;
        else
            cmd->angleturn -= mousex * 0x8;

        mousex = 0;
    }

    if (mousey)
    {
        if (usemouselook && !automapactive)
            cmd->lookdir = (m_invertyaxis ? -mousey : mousey);
        else if (!m_novertical)
            forward += mousey;

        mousey = 0;
    }

    cmd->forwardmove += BETWEEN(-MAXPLMOVE, forward, MAXPLMOVE);
    cmd->sidemove += BETWEEN(-MAXPLMOVE, side, MAXPLMOVE);

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

static void G_SetInitialWeapon(void)
{
    viewplayer->weaponowned[wp_fist] = true;
    viewplayer->weaponowned[wp_pistol] = true;

    viewplayer->ammo[am_clip] = initial_bullets;

    if (!initial_bullets && weaponinfo[wp_pistol].ammo != am_noammo)
    {
        viewplayer->readyweapon = wp_fist;
        viewplayer->pendingweapon = wp_fist;
    }
    else
    {
        viewplayer->readyweapon = wp_pistol;
        viewplayer->pendingweapon = wp_pistol;
    }

    for (int i = 0; i < NUMAMMO; i++)
        viewplayer->maxammo[i] = (gamemode == shareware && i == am_cell ? 0 : maxammo[i]);
}

//
// G_ResetPlayer
// [BH] Reset player's health, armor, weapons and ammo
//
static void G_ResetPlayer(void)
{
    viewplayer->health = initial_health;
    viewplayer->armorpoints = 0;
    viewplayer->armortype = NOARMOR;
    viewplayer->preferredshotgun = wp_shotgun;
    viewplayer->fistorchainsaw = wp_fist;
    viewplayer->shotguns = false;
    memset(viewplayer->weaponowned, false, sizeof(viewplayer->weaponowned));
    memset(viewplayer->ammo, false, sizeof(viewplayer->ammo));
    G_SetInitialWeapon();
    viewplayer->backpack = false;
}

//
// G_DoLoadLevel
//
void G_DoLoadLevel(void)
{
    int         ep;
    int         map = (gameepisode - 1) * 10 + gamemap;
    char        *author = P_GetMapAuthor(map);

    HU_DrawDisk();

    R_InitSkyMap();
    R_InitColumnFunctions();

    if (timer)
        countdown = timer * 60 * TICRATE;

    if (wipegamestate == GS_LEVEL)
        wipegamestate = GS_NONE;                // force a wipe

    gamestate = GS_LEVEL;

    if (viewplayer->playerstate == PST_DEAD)
        viewplayer->playerstate = PST_REBORN;

    viewplayer->damageinflicted = 0;
    viewplayer->damagereceived = 0;
    viewplayer->cheated = 0;
    viewplayer->shotshit = 0;
    viewplayer->shotsfired = 0;
    viewplayer->deaths = 0;
    viewplayer->distancetraveled = 0;
    viewplayer->itemspickedup_ammo_bullets = 0;
    viewplayer->itemspickedup_ammo_cells = 0;
    viewplayer->itemspickedup_ammo_rockets = 0;
    viewplayer->itemspickedup_ammo_shells = 0;
    viewplayer->itemspickedup_armor = 0;
    viewplayer->itemspickedup_health = 0;
    memset(viewplayer->mobjcount, 0, sizeof(viewplayer->mobjcount));

    freeze = false;

    // [BH] Reset player's health, armor, weapons and ammo on pistol start
    if (pistolstart || P_GetMapPistolStart(map))
        G_ResetPlayer();

    if (pendinggameskill)
    {
        gameskill = pendinggameskill - 1;

        if (gameskill == sk_nightmare)
            viewplayer->cheats &= ~(CF_NOCLIP | CF_GODMODE | CF_CHOPPERS | CF_BUDDHA);

        pendinggameskill = 0;
    }

    M_Seed((unsigned int)time(NULL));

    // initialize the msecnode_t freelist. phares 3/25/98
    // any nodes in the freelist are gone by now, cleared
    // by Z_FreeTags() when the previous level ended or player
    // died.
    P_FreeSecNodeList();

    C_AddConsoleDivider();
    ep = (gamemode == commercial ? (gamemission == pack_nerve ? 2 : 1) : gameepisode);
    P_MapName(ep, gamemap);

    if (*author)
        C_Print(titlestring, "%s by %s", mapnumandtitle, author);
    else
        C_Print(titlestring, mapnumandtitle);

    P_SetupLevel(ep, gamemap);

    st_facecount = 0;

    gameaction = ga_nothing;

    // clear cmd building stuff
    memset(gamekeydown, 0, sizeof(gamekeydown));
    mousex = 0;
    mousey = 0;
    sendpause = false;
    sendsave = false;
    paused = false;
    memset(mousearray, 0, sizeof(mousearray));

    M_SetWindowCaption();

    if (automapactive || mapwindow)
        AM_Start(automapactive);

    if (vid_widescreen || returntowidescreen)
        I_ToggleWidescreen(true);
}

void G_ToggleAlwaysRun(evtype_t type)
{
#if defined(_WIN32)
    alwaysrun = (keyboardalwaysrun == KEY_CAPSLOCK && type == ev_keydown ?
        (GetKeyState(VK_CAPITAL) & 0x0001) : !alwaysrun);
#else
    alwaysrun = !alwaysrun;
#endif

    if (!consoleactive)
    {
        HU_SetPlayerMessage((alwaysrun ? s_ALWAYSRUNON : s_ALWAYSRUNOFF), false);
        message_dontfuckwithme = true;
    }

    C_StrCVAROutput(stringize(alwaysrun), (alwaysrun ? "on" : "off"));

    if (menuactive)
    {
        message_dontpause = true;
        blurred = false;
    }

    M_SaveCVARs();
}

extern dboolean splashscreen;

//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//
dboolean G_Responder(event_t *ev)
{
    int key;

    // any other key pops up menu if on title screen
    if (gameaction == ga_nothing && gamestate == GS_TITLESCREEN)
    {
        if (!menuactive && !consoleactive
            && ((ev->type == ev_keydown
                && ev->data1 != KEY_PAUSE
                && ev->data1 != KEY_SHIFT
                && ev->data1 != KEY_ALT
                && ev->data1 != KEY_CTRL
                && ev->data1 != KEY_CAPSLOCK
                && ev->data1 != KEY_NUMLOCK
                && ev->data1 != KEY_PRINTSCREEN
                && (ev->data1 < KEY_F1 || ev->data1 > KEY_F12)
                && !((ev->data1 == KEY_ENTER || ev->data1 == KEY_TAB) && altdown))
                || (ev->type == ev_mouse && mousewait < I_GetTime() && ev->data1)
                || (ev->type == ev_gamepad
                    && gamepadwait < I_GetTime()
                    && gamepadbuttons
                    && !(gamepadbuttons & (GAMEPAD_DPAD_UP | GAMEPAD_DPAD_DOWN |
                        GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD_RIGHT))))
            && !keydown)
        {
            keydown = ev->data1;
            gamepadbuttons = 0;
            gamepadwait = I_GetTime() + 8;
            mousewait = I_GetTime() + 5;

            if (splashscreen)
            {
                if (pagetic < 95)
                    pagetic = MIN(10, pagetic);
            }
            else
            {
                M_StartControlPanel();
                S_StartSound(NULL, sfx_swtchn);
            }

            return true;
        }
        else if (ev->type == ev_keydown && ev->data1 == KEY_CAPSLOCK && ev->data1 == keyboardalwaysrun
            && !keydown)
        {
            keydown = KEY_CAPSLOCK;
            G_ToggleAlwaysRun(ev_keydown);
            return true;
        }

        return false;
    }

    if (gamestate == GS_LEVEL)
    {
        if (ST_Responder(ev))
            return true;        // status window ate it

        if (AM_Responder(ev))
            return true;        // Automap ate it
    }

    if (gamestate == GS_FINALE)
        if (F_Responder(ev))
            return true;        // finale ate the event

    switch (ev->type)
    {
        case ev_none:
            return false;

        case ev_keydown:
            key = ev->data1;

            if (key == keyboardprevweapon && !menuactive && !paused)
                G_PrevWeapon();
            else if (key == keyboardnextweapon && !menuactive && !paused)
                G_NextWeapon();
            else if (key == KEY_PAUSE && !menuactive && !keydown)
            {
                keydown = KEY_PAUSE;
                sendpause = true;
                blurred = false;

                if (vid_motionblur)
                    I_SetMotionBlur(0);
            }
            else if (key == keyboardalwaysrun && !keydown)
            {
                keydown = keyboardalwaysrun;
                G_ToggleAlwaysRun(ev_keydown);
            }
            else if (key < NUMKEYS)
            {
                gamekeydown[key] = true;

                if (keyactionlist[key][0])
                    C_ExecuteInputString(keyactionlist[key]);

                if (vibrate)
                {
                    vibrate = false;
                    idlemotorspeed = 0;
                    XInputVibration(idlemotorspeed);
                }
            }

            return true;            // eat key down events

        case ev_keyup:
            keydown = 0;

            if (ev->data1 < NUMKEYS)
                gamekeydown[ev->data1] = false;

            return false;           // always let key up events filter down

        case ev_mouse:
        {
            int mousebutton = ev->data1;

            for (int i = 0, j = 1; i < MAX_MOUSE_BUTTONS; i++, j <<= 1)
                mousebuttons[i] = mousebutton & j;

            if (mouseactionlist[mousebutton][0])
                C_ExecuteInputString(mouseactionlist[mousebutton]);

            if (vibrate && mousebutton)
            {
                vibrate = false;
                idlemotorspeed = 0;
                XInputVibration(idlemotorspeed);
            }

            if (!automapactive && !menuactive && !paused)
            {
                if (mousenextweapon < MAX_MOUSE_BUTTONS && mousebuttons[mousenextweapon])
                    G_NextWeapon();
                else if (mouseprevweapon < MAX_MOUSE_BUTTONS && mousebuttons[mouseprevweapon])
                    G_PrevWeapon();
            }

            if (!automapactive || am_followmode)
            {
                mousex = ev->data2 * m_sensitivity / 10;
                mousey = ev->data3 * m_sensitivity / 10;
            }

            return true;            // eat events
        }

        case ev_mousewheel:
            if (vibrate)
            {
                vibrate = false;
                idlemotorspeed = 0;
                XInputVibration(idlemotorspeed);
            }

            if (!automapactive && !menuactive && !paused)
            {
                if (ev->data1 < 0)
                {
                    if (mousenextweapon == MOUSE_WHEELDOWN)
                        G_NextWeapon();
                    else if (mouseprevweapon == MOUSE_WHEELDOWN)
                        G_PrevWeapon();

                    if (mouseactionlist[MOUSE_WHEELDOWN][0])
                        C_ExecuteInputString(mouseactionlist[MOUSE_WHEELDOWN]);
                }
                else if (ev->data1 > 0)
                {
                    if (mousenextweapon == MOUSE_WHEELUP)
                        G_NextWeapon();
                    else if (mouseprevweapon == MOUSE_WHEELUP)
                        G_PrevWeapon();

                    if (mouseactionlist[MOUSE_WHEELUP][0])
                        C_ExecuteInputString(mouseactionlist[MOUSE_WHEELUP]);
                }
            }

            return true;

        case ev_gamepad:
            if (!automapactive && !menuactive && !paused)
            {
                static int  wait;

                if ((gamepadbuttons & gamepadnextweapon) && wait < I_GetTime())
                {
                    wait = I_GetTime() + 7;

                    if (!gamepadpress || gamepadwait < I_GetTime())
                    {
                        G_NextWeapon();
                        gamepadpress = false;
                    }
                }
                else if ((gamepadbuttons & gamepadprevweapon) && wait < I_GetTime())
                {
                    wait = I_GetTime() + 7;

                    if (!gamepadpress || gamepadwait < I_GetTime())
                    {
                        G_PrevWeapon();
                        gamepadpress = false;
                    }
                }
                else if ((gamepadbuttons & gamepadalwaysrun) && wait < I_GetTime())
                {
                    wait = I_GetTime() + 7;

                    if (!gamepadpress || gamepadwait < I_GetTime())
                    {
                        G_ToggleAlwaysRun(ev_gamepad);
                        gamepadpress = false;
                    }
                }
            }

            return true;            // eat events
    }

    return false;
}

void D_Display(void);

static char savename[256];

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker(void)
{
    ticcmd_t            *cmd;

    // Game state the last time G_Ticker was called.
    static gamestate_t  oldgamestate;

    // do player reborn if needed
    if (viewplayer->playerstate == PST_REBORN)
        G_DoReborn();

    P_MapEnd();

    // do things to change the game state
    while (gameaction != ga_nothing)
    {
        switch (gameaction)
        {
            case ga_loadlevel:
                G_DoLoadLevel();
                break;

            case ga_autoloadgame:
                M_StringCopy(savename, P_SaveGameFile(quickSaveSlot), sizeof(savename));
                G_DoLoadGame();
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
                if (gamestate == GS_LEVEL && !idbehold && !(viewplayer->cheats & CF_MYPOS))
                {
                    HU_ClearMessages();
                    D_Display();
                }

                G_DoScreenShot();
                gameaction = ga_nothing;
                break;

            default:
                break;
        }
    }

    // get commands, check consistency,
    // and build new consistency check
    cmd = &viewplayer->cmd;
    memcpy(cmd, &netcmds[gametic % BACKUPTICS], sizeof(ticcmd_t));

    // check for special buttons
    if (viewplayer->cmd.buttons & BT_SPECIAL)
    {
        switch (viewplayer->cmd.buttons & BT_SPECIALMASK)
        {
            case BTS_PAUSE:
                paused ^= 1;

                if (paused)
                {
                    S_PauseSound();

                    if ((gp_vibrate_barrels || gp_vibrate_damage || gp_vibrate_weapons) && vibrate)
                    {
                        restoremotorspeed = idlemotorspeed;
                        idlemotorspeed = 0;
                        XInputVibration(idlemotorspeed);
                    }

                    viewplayer->fixedcolormap = 0;
                    I_SetPalette(W_CacheLumpName("PLAYPAL"));
                    I_UpdateBlitFunc(false);
                }
                else
                {
                    S_ResumeSound();
                    S_StartSound(NULL, sfx_swtchx);

                    if ((gp_vibrate_barrels || gp_vibrate_damage || gp_vibrate_weapons) && vibrate)
                    {
                        idlemotorspeed = restoremotorspeed;
                        XInputVibration(idlemotorspeed);
                    }

                    I_SetPalette((byte *)W_CacheLumpName("PLAYPAL") + st_palette * 768);
                }
                break;

            case BTS_SAVEGAME:
                savegameslot = (viewplayer->cmd.buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
                gameaction = ga_savegame;
                break;
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

        default:
            break;
    }
}

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in p_mobj.c
//

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
static void G_PlayerFinishLevel(void)
{
    memset(viewplayer->powers, 0, sizeof(viewplayer->powers));
    memset(viewplayer->cards, 0, sizeof(viewplayer->cards));
    viewplayer->mo->flags &= ~MF_FUZZ;  // cancel invisibility
    viewplayer->extralight = 0;         // cancel gun flashes
    viewplayer->fixedcolormap = 0;      // cancel ir goggles
    viewplayer->damagecount = 0;        // no palette changes
    viewplayer->bonuscount = 0;

    // [BH] switch to chainsaw if player has it and ends map with fists selected
    if (viewplayer->readyweapon == wp_fist && viewplayer->weaponowned[wp_chainsaw])
        viewplayer->readyweapon = wp_chainsaw;

    viewplayer->fistorchainsaw = (viewplayer->weaponowned[wp_chainsaw] ? wp_chainsaw : wp_fist);
}

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//
void G_PlayerReborn(void)
{
    int killcount = viewplayer->killcount;
    int itemcount = viewplayer->itemcount;
    int secretcount = viewplayer->secretcount;

    memset(viewplayer, 0, sizeof(*viewplayer));

    viewplayer->killcount = killcount;
    viewplayer->itemcount = itemcount;
    viewplayer->secretcount = secretcount;

    // don't do anything immediately
    viewplayer->usedown = true;
    viewplayer->attackdown = true;

    viewplayer->playerstate = PST_LIVE;
    viewplayer->health = initial_health;
    viewplayer->preferredshotgun = wp_shotgun;
    viewplayer->fistorchainsaw = wp_fist;
    viewplayer->shotguns = false;

    G_SetInitialWeapon();

    markpointnum = 0;
    infight = false;
    barreltics = 0;
}

//
// G_DoReborn
//
static void G_DoReborn(void)
{
    if (quickSaveSlot >= 0 && autoload && !pistolstart)
        gameaction = ga_autoloadgame;
    else
    {
        gameaction = ga_loadlevel;
        C_CCMDOutput("restartmap");
    }
}

void G_ScreenShot(void)
{
    gameaction = ga_screenshot;
}

void G_DoScreenShot(void)
{
    if (V_ScreenShot())
    {
        static char buffer[512];

        S_StartSound(NULL, sfx_swtchx);

        M_snprintf(buffer, sizeof(buffer), s_GSCREENSHOT, lbmname1);
        HU_SetPlayerMessage(buffer, false);
        message_dontfuckwithme = true;

        if (menuactive)
        {
            message_dontpause = true;
            blurred = false;
        }

        C_Output("<b>%s</b> saved.", lbmpath1);

        if (*lbmpath2)
            C_Output("<b>%s</b> saved.", lbmpath2);
    }
    else
        C_Warning("A screenshot couldn't be taken.");
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
static const int npars[9] =
{
     75, 105, 120, 105, 210, 105, 165, 105, 135
};

//
// G_DoCompleted
//
static dboolean secretexit;

void G_ExitLevel(void)
{
    secretexit = false;
    gameaction = ga_completed;
}

// Here's for the German edition.
void G_SecretExitLevel(void)
{
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    secretexit = !(gamemode == commercial && W_CheckNumForName("MAP31") < 0);
    gameaction = ga_completed;
}

extern int      episode;
extern menu_t   EpiDef;

void ST_doRefresh(void);

static void G_DoCompleted(void)
{
    int         map = (gameepisode - 1) * 10 + gamemap;
    int         nextmap = P_GetMapNext(map);
    int         par = P_GetMapPar(map);
    int         secretnextmap = P_GetMapSecretNext(map);

    gameaction = ga_nothing;

    I_UpdateBlitFunc(false);

    // [BH] allow the exit switch to turn on before the screen wipes
    viewplayer->mo->momx = 0;
    viewplayer->mo->momy = 0;
    viewplayer->mo->momz = 0;
    R_RenderPlayerView();
    I_Sleep(700);

    if (vid_widescreen)
    {
        I_ToggleWidescreen(false);
        returntowidescreen = true;
        ST_doRefresh();
    }

    G_PlayerFinishLevel();      // take away cards and stuff

    if (automapactive)
        AM_Stop();
    else if (mapwindow)
        AM_clearFB();

    if (chex)
        if (gamemap == 5)
        {
            gameaction = ga_victory;
            return;
        }

    if (gamemode != commercial)
    {
        switch (gamemap)
        {
            case 8:
                // [BH] this episode is complete, so select the next episode in the menu
                if ((gamemode == registered && gameepisode < 3) || (gamemode == retail && gameepisode < 4))
                {
                    episode++;
                    EpiDef.lastOn++;
                    M_SaveCVARs();
                }

                break;

            case 9:
                viewplayer->didsecret = true;
                break;
        }
    }

    wminfo.didsecret = viewplayer->didsecret;
    wminfo.epsd = gameepisode - 1;
    wminfo.last = gamemap - 1;

    if (secretexit && secretnextmap > 0)
        wminfo.next = secretnextmap - 1;
    else if (nextmap > 0)
        wminfo.next = nextmap - 1;
    else if (gamemode == commercial)
    {
        // wminfo.next is 0 biased, unlike gamemap
        if (secretexit)
        {
            switch (gamemap)
            {
                case 2:
                    // [BH] exit to secret level on MAP02 of BFG Edition
                    if (bfgedition)
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
                    if (bfgedition)
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

    if (par)
        wminfo.partime = TICRATE * par;
    else
    {
        char    lump[5];

        // [BH] have no par time if this level is from a PWAD
        if (gamemode == commercial)
            M_snprintf(lump, sizeof(lump), "MAP%02i", gamemap);
        else
            M_snprintf(lump, sizeof(lump), "E%iM%i", gameepisode, gamemap);

        if (BTSX || (W_CheckMultipleLumps(lump) > 1 && (!nerve || gamemap > 9) && !FREEDOOM))
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
    }

    wminfo.skills = (totalkills ? viewplayer->killcount : 1);
    wminfo.sitems = (totalitems ? viewplayer->itemcount : 1);
    wminfo.ssecret = viewplayer->secretcount;
    wminfo.stime = leveltime;

    gamestate = GS_INTERMISSION;
    viewactive = false;
    automapactive = false;

    stat_mapscompleted = SafeAdd(stat_mapscompleted, 1);

    C_CCMDOutput("exitmap");

    C_AddConsoleDivider();

    WI_Start(&wminfo);
}

//
// G_WorldDone
//
void G_WorldDone(void)
{
    gameaction = ga_worlddone;

    if (secretexit)
        viewplayer->didsecret = true;

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
    else if (gamemap == 8)
        gameaction = ga_victory;
}

static void G_DoWorldDone(void)
{
    gamestate = GS_LEVEL;
    gamemap = wminfo.next + 1;
    G_DoLoadLevel();
    viewactive = true;
    markpointnum = 0;
}

extern dboolean setsizeneeded;

void R_ExecuteSetViewSize(void);

void G_LoadGame(char *name)
{
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame;
}

void G_DoLoadGame(void)
{
    int savedleveltime;

    I_SetPalette(W_CacheLumpName("PLAYPAL"));

    loadaction = gameaction;
    gameaction = ga_nothing;

    if (!(save_stream = fopen(savename, "rb")))
        return;

    if (!P_ReadSaveGameHeader(savedescription))
    {
        fclose(save_stream);
        return;
    }

    savedleveltime = leveltime;

    // load a base level
    G_InitNew(gameskill, gameepisode, gamemap);

    leveltime = savedleveltime;

    // unarchive all the modifications
    P_UnArchivePlayer();
    P_UnArchiveWorld();
    P_UnArchiveThinkers();
    P_UnArchiveSpecials();
    P_UnArchiveMap();

    P_RestoreTargets();

    P_MapEnd();

    if (musinfo.current_item != -1)
        S_ChangeMusInfoMusic(musinfo.current_item, true);

    if (!P_ReadSaveGameEOF())
        I_Error("Bad savegame");

    fclose(save_stream);

    if (setsizeneeded)
        R_ExecuteSetViewSize();

    if (vid_widescreen)
        I_ToggleWidescreen(true);

    // draw the pattern into the back screen
    R_FillBackScreen();

    st_facecount = 0;

    C_Input("load %s", savename);

    if (consoleactive)
    {
        C_Output("<b>%s</b> loaded.", savename);
        C_HideConsoleFast();
    }
}

void G_LoadedGameMessage(void)
{
    if (*savedescription)
    {
        static char buffer[1024];

        M_snprintf(buffer, sizeof(buffer), (loadaction == ga_autoloadgame ? s_GGAUTOLOADED : s_GGLOADED),
            titlecase(savedescription));
        HU_PlayerMessage(buffer, false);
        message_dontfuckwithme = true;
    }

    loadaction = ga_nothing;
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 256 byte text string
//
void G_SaveGame(int slot, char *description, char *name)
{
    M_StringCopy(savename, name, sizeof(savename));
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
    drawdisk = true;
}

static void G_DoSaveGame(void)
{
    char    *temp_savegame_file = P_TempSaveGameFile();
    char    *savegame_file = (consoleactive ? savename : P_SaveGameFile(savegameslot));

    // Open the savegame file for writing. We write to a temporary file
    // and then rename it at the end if it was successfully written.
    // This prevents an existing savegame from being overwritten by
    // a corrupted one, or if a savegame buffer overrun occurs.

    if (!(save_stream = fopen(temp_savegame_file, "wb")))
    {
        menuactive = false;
        C_ShowConsole();
        C_Warning("<b>%s</b> couldn't be saved.", savename);
    }
    else
    {
        P_WriteSaveGameHeader(savedescription);

        P_ArchivePlayer();
        P_ArchiveWorld();
        P_ArchiveThinkers();
        P_ArchiveSpecials();
        P_ArchiveMap();

        P_WriteSaveGameEOF();

        // Finish up, close the savegame file.
        fclose(save_stream);

        // Now rename the temporary savegame file to the actual savegame
        // file, overwriting the old savegame if there was one there.
        remove(savegame_file);
        rename(temp_savegame_file, savegame_file);

        C_Input("save %s", savegame_file);

        if (consoleactive)
            C_Output("<b>%s</b> saved.", savename);
        else
        {
            static char buffer[1024];

            M_snprintf(buffer, sizeof(buffer), s_GGSAVED, titlecase(savedescription));
            HU_PlayerMessage(buffer, false);
            blurred = false;
            message_dontfuckwithme = true;
            S_StartSound(NULL, sfx_swtchx);
        }

        // draw the pattern into the back screen
        R_FillBackScreen();
    }

    gameaction = ga_nothing;

    drawdisk = false;
}

static skill_t  d_skill;
static int      d_episode;
static int      d_map;

void G_DeferredInitNew(skill_t skill, int ep, int map)
{
    d_skill = skill;
    d_episode = ep;
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
extern msecnode_t   *sector_list;

void G_DeferredLoadLevel(skill_t skill, int ep, int map)
{
    d_skill = skill;
    d_episode = ep;
    d_map = map;
    gameaction = ga_loadlevel;
    markpointnum = 0;
    infight = false;
    sector_list = NULL;

    for (int i = 0; i < NUMPOWERS; i++)
        if (viewplayer->powers[i] > 0)
            viewplayer->powers[i] = 0;
}

static void G_DoNewGame(void)
{
    I_SetPalette(W_CacheLumpName("PLAYPAL"));

    if (vid_widescreen)
        I_ToggleWidescreen(true);

    st_facecount = ST_STRAIGHTFACECOUNT;
    G_InitNew(d_skill, d_episode, d_map);
    gameaction = ga_nothing;
    markpointnum = 0;
    infight = false;
}

void G_SetFastMonsters(dboolean toggle)
{
    if (toggle)
    {
        for (int i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
            if (states[i].tics != 1)
                states[i].tics >>= 1;

        mobjinfo[MT_BRUISERSHOT].speed = 20 * FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 20 * FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 20 * FRACUNIT;
    }
    else
    {
        for (int i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
            states[i].tics <<= 1;

        mobjinfo[MT_BRUISERSHOT].speed = 15 * FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 10 * FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 10 * FRACUNIT;
    }
}

static void G_SetFastParms(int fast_pending)
{
    static int  fast;

    if (fast != fast_pending)
        G_SetFastMonsters((fast = fast_pending));
}

void G_SetMovementSpeed(int scale)
{
    forwardmove[0] = FORWARDMOVE0 * scale / 100;
    forwardmove[1] = MIN(FORWARDMOVE1 * scale / 100, 127);
    sidemove[0] = SIDEMOVE0 * scale / 100;
    sidemove[1] = SIDEMOVE1 * scale / 100;
}

//
// G_InitNew
// Can be called by the startup code or the menu task.
//
void G_InitNew(skill_t skill, int ep, int map)
{
    if (paused)
    {
        paused = false;
        S_ResumeSound();
    }

    if (skill > sk_nightmare)
        skill = sk_nightmare;

    if (ep < 1)
        ep = 1;

    if (gamemode == retail)
    {
        if (ep > 4)
            ep = 4;
    }
    else if (gamemode == shareware)
    {
        if (ep > 1)
            ep = 1;     // only start episode 1 on shareware
    }

    if (map > 9 && gamemode != commercial)
        map = 9;

    // [BH] Fix demon speed bug. See doomwiki.org/wiki/Demon_speed_bug.
    G_SetFastParms(fastparm || skill == sk_nightmare);

    // force player to be initialized upon first level load
    viewplayer->playerstate = PST_REBORN;

    paused = false;
    automapactive = false;
    viewactive = true;
    gameepisode = ep;
    gamemap = map;
    gameskill = skill;

    C_CCMDOutput("newgame");

    G_DoLoadLevel();
}
