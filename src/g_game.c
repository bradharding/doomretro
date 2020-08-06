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
#include "i_colors.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_cheat.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
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
char            speciallumpname[6] = "";

dboolean        paused;
dboolean        sendpause;                      // send a pause event next tic
static dboolean sendsave;                       // send a save event next tic

dboolean        viewactive;

int             gametime = 0;
int             totalkills;                     // for intermission
int             totalitems;
int             totalsecret;
int             totalpickups;
int             monstercount[NUMMOBJTYPES];
int             barrelcount;

wbstartstruct_t wminfo;                         // parms for world map/intermission

dboolean        autoload = autoload_default;
dboolean        autosave = autosave_default;

#define MAXPLMOVE       forwardmove[1]

fixed_t         forwardmove[2] = { FORWARDMOVE0, FORWARDMOVE1 };
fixed_t         sidemove[2] = { SIDEMOVE0, SIDEMOVE1 };
fixed_t         angleturn[3] = { 640, 1280, 320 };     // + slow turn
static fixed_t  gamepadangleturn[2] = { 640, 960 };

#define NUMWEAPONKEYS   7

static int *keyboardweapons[] =
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

#define SLOWTURNTICS    6

dboolean        gamekeydown[NUMKEYS] = { 0 };
char            keyactionlist[NUMKEYS][255] = { "" };
static int      turnheld;                       // for accelerative turning

static dboolean mousearray[MAX_MOUSE_BUTTONS + 1];
dboolean        *mousebuttons = &mousearray[1]; // allow [-1]
char            mouseactionlist[MAX_MOUSE_BUTTONS + 2][255] = { "" };

dboolean        skipaction = false;

static int      mousex;
static int      mousey;

dboolean        m_doubleclick_use = m_doubleclick_use_default;
dboolean        m_invertyaxis = m_invertyaxis_default;
dboolean        m_novertical = m_novertical_default;
dboolean        mouselook = mouselook_default;

dboolean        usemouselook = false;

static int      dclicktime;
static dboolean dclickstate;
static int      dclicks;
static int      dclicktime2;
static dboolean dclickstate2;
static int      dclicks2;

static int      savegameslot;
static char     savedescription[SAVESTRINGSIZE];
char            savename[MAX_PATH];

gameaction_t    loadaction = ga_nothing;

uint64_t        stat_gamessaved = 0;
uint64_t        stat_mapsstarted = 0;
uint64_t        stat_mapscompleted = 0;
uint64_t        stat_skilllevel_imtooyoungtodie = 0;
uint64_t        stat_skilllevel_heynottoorough = 0;
uint64_t        stat_skilllevel_hurtmeplenty = 0;
uint64_t        stat_skilllevel_ultraviolence = 0;
uint64_t        stat_skilllevel_nightmare = 0;

extern int      logotic;
extern int      pagetic;

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
        i = weaponinfo[i].next;

        if (i == wp_fist && viewplayer->weaponowned[wp_chainsaw] && !viewplayer->powers[pw_strength])
            i = wp_chainsaw;
    } while (!viewplayer->weaponowned[i] || viewplayer->ammo[weaponinfo[i].ammotype] < weaponinfo[i].minammo);

    if (i != readyweapon)
    {
        P_EquipWeapon(i);

        if (i == wp_fist && viewplayer->powers[pw_strength])
            S_StartSound(NULL, sfx_getpow);

        if (i == wp_shotgun || i == wp_supershotgun)
            viewplayer->preferredshotgun = i;
    }

    if ((viewplayer->cheats & CF_CHOPPERS) && i != wp_chainsaw)
        G_RemoveChoppers();
}

void G_PrevWeapon(void)
{
    weapontype_t    pendingweapon = viewplayer->pendingweapon;
    weapontype_t    readyweapon = viewplayer->readyweapon;
    weapontype_t    i = (pendingweapon == wp_nochange ? readyweapon : pendingweapon);

    do
    {
        i = weaponinfo[i].prev;

        if (i == wp_fist && viewplayer->weaponowned[wp_chainsaw] && !viewplayer->powers[pw_strength])
            i = wp_bfg;
    } while (!viewplayer->weaponowned[i] || viewplayer->ammo[weaponinfo[i].ammotype] < weaponinfo[i].minammo);

    if (i != readyweapon)
    {
        P_EquipWeapon(i);

        if (i == wp_fist && viewplayer->powers[pw_strength])
            S_StartSound(NULL, sfx_getpow);

        if (i == wp_shotgun || i == wp_supershotgun)
            viewplayer->preferredshotgun = i;
    }

    if ((viewplayer->cheats & CF_CHOPPERS) && i != wp_chainsaw)
        G_RemoveChoppers();
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
    run = (gamekeydown[keyboardrun] ^ mousebuttons[mouserun] ^ (!!(gamepadbuttons & gamepadrun)) ^ alwaysrun);
    usemouselook = (mouselook || gamekeydown[keyboardmouselook] || mousebuttons[mousemouselook] || (gamepadbuttons & gamepadmouselook));

    // use two stage accelerative turning on the keyboard
    if (gamekeydown[keyboardright] || gamekeydown[keyboardleft] || (gamepadbuttons & gamepadleft) || (gamepadbuttons & gamepadright))
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
            cmd->angleturn -= angleturn[(turnheld < SLOWTURNTICS ? 2 : run)];
        else if (gamepadthumbRX > 0)
            cmd->angleturn -= (int)(gamepadangleturn[run] * gamepadthumbRXright * gamepadhorizontalsensitivity);

        if (gamekeydown[keyboardleft] || (gamepadbuttons & gamepadleft))
            cmd->angleturn += angleturn[(turnheld < SLOWTURNTICS ? 2 : run)];
        else if (gamepadthumbRX < 0)
            cmd->angleturn += (int)(gamepadangleturn[run] * gamepadthumbRXleft * gamepadhorizontalsensitivity);
    }

    if (gamepadthumbRY)
    {
        if (usemouselook && gp_thumbsticks == 2)
        {
            if (!automapactive)
            {
                cmd->lookdir = (int)(48 * (gamepadthumbRY < 0 ? gamepadthumbRYup : gamepadthumbRYdown) * gamepadverticalsensitivity);

                if (!gp_invertyaxis)
                    cmd->lookdir = -cmd->lookdir;
            }
        }
        else if (gp_thumbsticks == 1)
        {
            cmd->lookdir = 0;
            forward = (int)(forwardmove[run] * (gamepadthumbRY < 0 ? gamepadthumbRYup : gamepadthumbRYdown));
        }
    }

    if (gamekeydown[keyboardforward] || gamekeydown[keyboardforward2] || (gamepadbuttons & gamepadforward))
        forward += forwardmove[run];
    else if (gamepadthumbLY < 0)
        forward += (int)(forwardmove[run] * gamepadthumbLYup);

    if (gamekeydown[keyboardback] || gamekeydown[keyboardback2] || (gamepadbuttons & gamepadback))
        forward -= forwardmove[run];
    else if (gamepadthumbLY > 0)
        forward -= (int)(forwardmove[run] * gamepadthumbLYdown);

    if (gamekeydown[keyboardstraferight] || gamekeydown[keyboardstraferight2] || (gamepadbuttons & gamepadstraferight))
        side += sidemove[run];
    else if (gamepadthumbLX > 0)
    {
        if (gp_thumbsticks == 2)
            side += (int)(sidemove[run] * gamepadthumbLXright);
        else
            cmd->angleturn -= (int)(gamepadangleturn[run] * gamepadthumbLXright * gamepadhorizontalsensitivity);
    }

    if (gamekeydown[keyboardstrafeleft] || gamekeydown[keyboardstrafeleft2] || (gamepadbuttons & gamepadstrafeleft))
        side -= sidemove[run];
    else if (gamepadthumbLX < 0)
    {
        if (gp_thumbsticks == 2)
            side -= (int)(sidemove[run] * gamepadthumbLXleft);
        else
            cmd->angleturn += (int)(gamepadangleturn[run] * gamepadthumbLXleft * gamepadhorizontalsensitivity);
    }

    if ((gamekeydown[keyboardjump] || mousebuttons[mousejump] || (gamepadbuttons & gamepadjump)) && !nojump)
        cmd->buttons |= BT_JUMP;

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
            dclicks = 0;                // clear double clicks if hit use button
        }
    }

    if (!idclev && !idmus)
        for (int i = 0; i < NUMWEAPONKEYS; i++)
        {
            int key = *keyboardweapons[i];

            if (gamekeydown[key] && !keydown)
            {
                keydown = key;
                cmd->buttons |= BT_CHANGE | (i << BT_WEAPONSHIFT);
                break;
            }
            else if (gamepadbuttons & *gamepadweapons[i])
            {
                if (viewplayer->readyweapon != i || (i == wp_fist && viewplayer->weaponowned[wp_chainsaw])
                    || (i == wp_shotgun && viewplayer->weaponowned[wp_supershotgun]))
                {
                    cmd->buttons |= BT_CHANGE | (i << BT_WEAPONSHIFT);
                    break;
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
        if ((bstrafe = mousebuttons[mousestrafe]) != dclickstate2 && dclicktime2 > 1)
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
            cmd->angleturn -= mousex * 0x08;

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

    if (forward)
        cmd->forwardmove += BETWEEN(-MAXPLMOVE, forward, MAXPLMOVE);

    if (side)
        cmd->sidemove += BETWEEN(-MAXPLMOVE, side, MAXPLMOVE);

    // special buttons
    if (sendpause)
    {
        sendpause = false;
        cmd->buttons = BT_SPECIAL | BTS_PAUSE;
    }

    if (sendsave)
    {
        sendsave = false;
        cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot << BTS_SAVESHIFT);
    }

    if (cmd->angleturn && !menuactive)
        spindirection = SIGN(cmd->angleturn);
}

static void G_SetInitialWeapon(void)
{
    viewplayer->weaponowned[wp_fist] = true;
    viewplayer->weaponowned[wp_pistol] = true;

    viewplayer->ammo[am_clip] = initial_bullets;

    if (!initial_bullets && weaponinfo[wp_pistol].ammotype != am_noammo)
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
        viewplayer->maxammo[i] = maxammo[i];
}

//
// G_ResetPlayer
// [BH] Reset player's health, armor, weapons and ammo
//
static void G_ResetPlayer(void)
{
    viewplayer->health = initial_health;
    viewplayer->armorpoints = 0;
    viewplayer->armortype = armortype_none;
    viewplayer->preferredshotgun = wp_shotgun;
    viewplayer->fistorchainsaw = wp_fist;
    memset(viewplayer->weaponowned, false, sizeof(viewplayer->weaponowned));
    memset(viewplayer->ammo, 0, sizeof(viewplayer->ammo));
    G_SetInitialWeapon();
    viewplayer->backpack = false;
}

//
// G_DoLoadLevel
//
void G_DoLoadLevel(void)
{
    int ep;
    int map = (gameepisode - 1) * 10 + gamemap;

    HU_DrawDisk();

    if (timer)
        countdown = timer * 60 * TICRATE;

    if (wipegamestate == GS_LEVEL)
        wipegamestate = GS_NONE;                // force a wipe

    gamestate = GS_LEVEL;

    if (viewplayer->playerstate == PST_DEAD)
        viewplayer->playerstate = PST_REBORN;

    if (viewplayer->playerstate == PST_REBORN && !startingnewgame
        && (M_StringCompare(mapnum, "E1M4B") || M_StringCompare(mapnum, "E1M8B")))
        M_StringCopy(speciallumpname, mapnum, sizeof(speciallumpname));

    viewplayer->damageinflicted = 0;
    viewplayer->damagereceived = 0;
    viewplayer->cheated = 0;
    memset(viewplayer->shotssuccessful, 0, sizeof(viewplayer->shotssuccessful));
    memset(viewplayer->shotsfired, 0, sizeof(viewplayer->shotsfired));
    viewplayer->distancetraveled = 0;
    viewplayer->gamessaved = 0;
    viewplayer->itemspickedup_ammo_bullets = 0;
    viewplayer->itemspickedup_ammo_cells = 0;
    viewplayer->itemspickedup_ammo_rockets = 0;
    viewplayer->itemspickedup_ammo_shells = 0;
    viewplayer->itemspickedup_armor = 0;
    viewplayer->itemspickedup_health = 0;
    memset(viewplayer->mobjcount, 0, sizeof(viewplayer->mobjcount));
    viewplayer->prevmessage[0] = '\0';

    freeze = false;

    // [BH] Reset player's health, armor, weapons and ammo on pistol start
    if (pistolstart || P_GetMapPistolStart(map))
        G_ResetPlayer();

    nojump = P_GetMapNoJump(map);
    nomouselook = P_GetMapNoMouselook(map);

    if (pendinggameskill)
    {
        gameskill = pendinggameskill - 1;

        if (gameskill == sk_nightmare)
            viewplayer->cheats &= ~(CF_NOCLIP | CF_GODMODE | CF_CHOPPERS | CF_BUDDHA);

        pendinggameskill = 0;
    }

    // initialize the msecnode_t freelist. phares 3/25/98
    // any nodes in the freelist are gone by now, cleared
    // by Z_FreeTags() when the previous level ended or player
    // died.
    P_FreeSecNodeList();

    ep = (gamemode == commercial ? (gamemission == pack_nerve ? 2 : 1) : gameepisode);
    P_MapName(ep, gamemap);

    P_SetupLevel(ep, gamemap);

    R_InitSkyMap();
    R_InitColumnFunctions();

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

    // [BH] clear these as well, since data from prev map can be copied over in G_BuildTiccmd()
    for (int i = 0; i < BACKUPTICS; i++)
        memset(&localcmds[i], 0, sizeof(ticcmd_t));

    P_SetPlayerViewHeight();

    stat_mapsstarted = SafeAdd(stat_mapsstarted, 1);

    M_SetWindowCaption();

    if (automapactive || mapwindow)
        AM_Start(automapactive);

    if (vid_widescreen || returntowidescreen)
        I_ToggleWidescreen(true);
}

void G_ToggleAlwaysRun(evtype_t type)
{
#if defined(_WIN32)
    alwaysrun = (keyboardalwaysrun == KEY_CAPSLOCK && type == ev_keydown ? (GetKeyState(VK_CAPITAL) & 0x0001) : !alwaysrun);
#else
    alwaysrun = !alwaysrun;
#endif

    if (!consoleactive)
    {
        HU_SetPlayerMessage((alwaysrun ? s_ALWAYSRUNON : s_ALWAYSRUNOFF), false, false);
        message_dontfuckwithme = true;
    }

    C_StrCVAROutput(stringize(alwaysrun), (alwaysrun ? "on" : "off"));

    M_SaveCVARs();
}

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
        if (!menuactive
            && !consoleactive
            && ((ev->type == ev_keydown
                && !keydown
                && ev->data1 != KEY_PAUSE
                && ev->data1 != KEY_SHIFT
                && ev->data1 != KEY_ALT
                && ev->data1 != KEY_CTRL
                && ev->data1 != KEY_CAPSLOCK
                && ev->data1 != KEY_NUMLOCK
                && (ev->data1 < KEY_F1 || ev->data1 > KEY_F11)
                && !((ev->data1 == KEY_ENTER || ev->data1 == KEY_TAB) && altdown))
            || (ev->type == ev_mouse && mousewait < I_GetTime() && ev->data1)
            || (ev->type == ev_gamepad
                && gamepadwait < I_GetTime()
                && gamepadbuttons
                && !(gamepadbuttons &(GAMEPAD_DPAD_UP | GAMEPAD_DPAD_DOWN | GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD_RIGHT)))))
        {
            if (ev->type == ev_keydown && ev->data1 == keyboardalwaysrun)
            {
                keydown = keyboardalwaysrun;
                G_ToggleAlwaysRun(ev_keydown);
            }
            else if (ev->type == ev_keydown && ev->data1 == keyboardscreenshot)
            {
                keydown = keyboardscreenshot;
                G_DoScreenShot();
            }
            else
            {
                keydown = ev->data1;
                gamepadbuttons = 0;
                mousewait = I_GetTime() + 5;
                gamepadwait = mousewait + 3;

                logotic = MIN(logotic, 93);

                if (splashscreen)
                    pagetic = MIN(pagetic, 10);
                else
                {
                    M_StartControlPanel();
                    S_StartSound(NULL, sfx_swtchn);
                }
            }

            return true;
        }
        else if (!menuactive && !consoleactive && ev->type == ev_keyup && ev->data1 == keyboardscreenshot)
        {
            S_StartSound(NULL, sfx_scrsht);
            memset(screens[0], nearestwhite, SCREENAREA);
            D_FadeScreen();
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

    if (gamestate == GS_FINALE && F_Responder(ev))
        return true;            // finale ate the event

    switch (ev->type)
    {
        case ev_keydown:
            key = ev->data1;

            if (key == keyboardprevweapon && !menuactive && !paused && !freeze)
                G_PrevWeapon();
            else if (key == keyboardnextweapon && !menuactive && !paused && !freeze)
                G_NextWeapon();
            else if (key == KEY_PAUSE && !menuactive && !keydown && !idclevtics)
            {
                keydown = KEY_PAUSE;
                sendpause = true;

                if (vid_motionblur)
                    I_SetMotionBlur(0);

                D_FadeScreen();
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
            }

            return true;        // eat events

        case ev_keyup:
            keydown = 0;

            if (ev->data1 < NUMKEYS)
                gamekeydown[ev->data1] = false;

            return false;       // always let key up events filter down

        case ev_mouse:
        {
            int mousebutton = ev->data1;

            for (int i = 0, j = 1; i < MAX_MOUSE_BUTTONS; i++, j <<= 1)
                mousebuttons[i] = !!(mousebutton & j);

            if (mouseactionlist[mousebutton][0] && !freeze)
                C_ExecuteInputString(mouseactionlist[mousebutton]);

            if (!automapactive && !menuactive && !paused && !freeze)
            {
                if (mousenextweapon < MAX_MOUSE_BUTTONS && mousebuttons[mousenextweapon])
                    G_NextWeapon();
                else if (mouseprevweapon < MAX_MOUSE_BUTTONS && mousebuttons[mouseprevweapon])
                    G_PrevWeapon();
            }

            if (!automapactive || am_followmode)
            {
                mousex = ev->data2 * m_sensitivity / 10;
                mousey = -ev->data3 * m_sensitivity / 10;
            }

            return true;        // eat events
        }

        case ev_mousewheel:
            if (!automapactive && !menuactive && !paused && !freeze)
            {
                if (ev->data1 < 0)
                {
                    if (mousenextweapon == MOUSE_WHEELDOWN)
                        G_NextWeapon();
                    else if (mouseprevweapon == MOUSE_WHEELDOWN)
                        G_PrevWeapon();
                    else if (mouseactionlist[MOUSE_WHEELDOWN][0])
                        C_ExecuteInputString(mouseactionlist[MOUSE_WHEELDOWN]);
                }
                else if (ev->data1 > 0)
                {
                    if (mousenextweapon == MOUSE_WHEELUP)
                        G_NextWeapon();
                    else if (mouseprevweapon == MOUSE_WHEELUP)
                        G_PrevWeapon();
                    else if (mouseactionlist[MOUSE_WHEELUP][0])
                        C_ExecuteInputString(mouseactionlist[MOUSE_WHEELUP]);
                }
            }

            return true;        // eat events

        case ev_gamepad:
            if (!automapactive && !menuactive && !paused)
            {
                static int  wait;
                int         time = I_GetTime();

                if ((gamepadbuttons & gamepadnextweapon) && wait < time && !freeze)
                {
                    wait = time + 7;

                    if (!gamepadpress || gamepadwait < time)
                    {
                        G_NextWeapon();
                        gamepadpress = false;
                    }
                }
                else if ((gamepadbuttons & gamepadprevweapon) && wait < time && !freeze)
                {
                    wait = time + 7;

                    if (!gamepadpress || gamepadwait < time)
                    {
                        G_PrevWeapon();
                        gamepadpress = false;
                    }
                }
                else if ((gamepadbuttons & gamepadalwaysrun) && wait < time)
                {
                    wait = time + 7;

                    if (!gamepadpress || gamepadwait < time)
                    {
                        G_ToggleAlwaysRun(ev_gamepad);
                        gamepadpress = false;
                    }
                }
            }

            return true;        // eat events

        default:
            return false;
    }
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker(void)
{
    // Game state the last time G_Ticker was called.
    static gamestate_t  oldgamestate;

    // do player reborn if needed
    if (viewplayer->playerstate == PST_REBORN)
        G_DoReborn();

    P_MapEnd();

    // do things to change the game state
    while (gameaction != ga_nothing)
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
            case ga_autosavegame:
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
                G_DoScreenShot();
                break;

            default:
                break;
        }

    // get commands, check consistency,
    // and build new consistency check
    memcpy(&viewplayer->cmd, &localcmds[gametime % BACKUPTICS], sizeof(ticcmd_t));

    // check for special buttons
    if (viewplayer->cmd.buttons & BT_SPECIAL)
    {
        switch (viewplayer->cmd.buttons & BT_SPECIALMASK)
        {
            case BTS_PAUSE:
                if ((paused = !paused))
                {
                    S_PauseMusic();
                    S_StartSound(NULL, sfx_swtchn);
                    viewplayer->fixedcolormap = 0;
                    I_SetPalette(PLAYPAL);
                    I_UpdateBlitFunc(false);
                    I_StopGamepadVibration();
                }
                else
                {
                    S_ResumeMusic();
                    S_StartSound(NULL, sfx_swtchx);
                    I_SetPalette(&PLAYPAL[st_palette * 768]);
                }

                break;

            case BTS_SAVEGAME:
                savegameslot = (viewplayer->cmd.buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
                gameaction = ga_savegame;
                break;
        }

        viewplayer->cmd.buttons = 0;
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
// Called when a player completes a level.
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
    st_palette = 0;                     // [JN] Also no inner palette changes

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
    int deaths = viewplayer->deaths;
    int suicides = viewplayer->suicides;
    int cheats = viewplayer->cheats;

    memset(viewplayer, 0, sizeof(*viewplayer));

    viewplayer->killcount = killcount;
    viewplayer->itemcount = itemcount;
    viewplayer->secretcount = secretcount;
    viewplayer->deaths = deaths;
    viewplayer->suicides = suicides;
    viewplayer->cheats = cheats;

    // don't do anything immediately
    viewplayer->usedown = true;
    viewplayer->attackdown = true;

    viewplayer->playerstate = PST_LIVE;
    viewplayer->health = initial_health;
    viewplayer->preferredshotgun = wp_shotgun;
    viewplayer->fistorchainsaw = wp_fist;

    G_SetInitialWeapon();

    infight = false;
    barrelms = 0;
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
        C_InputNoRepeat("restartmap");
    }
}

void G_ScreenShot(void)
{
    if (!splashscreen)
        gameaction = ga_screenshot;
}

void G_DoScreenShot(void)
{
    if (fadecount > 0)
    {
        if (idbehold)
        {
            idbehold = false;
            C_Input(cheat_powerup[6].sequence);
            C_Output(s_STSTR_BEHOLD);
        }
        else if (gamestate == GS_LEVEL && !(viewplayer->cheats & CF_MYPOS))
        {
            HU_ClearMessages();
            D_Display();
            D_Display();
        }

        if (V_ScreenShot())
        {
            static char buffer[512];

            M_snprintf(buffer, sizeof(buffer), s_GSCREENSHOT, lbmname1);
            HU_SetPlayerMessage(buffer, false, false);
            message_dontfuckwithme = true;

            C_Output("<b>%s</b> was saved.", lbmpath1);

            if (*lbmpath2)
                C_Output("<b>%s</b> was saved.", lbmpath2);
        }
        else
            C_Warning(0, "A screenshot couldn't be taken.");
    }

    gameaction = ga_nothing;
}

// DOOM Par Times
int pars[6][10] =
{
    { 0 },
    { 0,  30,  75, 120,  90, 165, 180, 180,  30, 165 },
    { 0,  90,  90,  90, 120,  90, 360, 240,  30, 170 },
    { 0,  90,  45,  90, 150,  90,  90, 165,  30, 135 },

    // [BH] Episode 4 and 5 Par Times
    { 0, 165, 255, 135, 150, 180, 390, 135, 360, 180 },
    { 0,  90, 150, 360, 420, 780, 420, 780, 300, 660 }
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
dboolean secretexit;

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

int G_GetParTime(void)
{
    int par = P_GetMapPar((gameepisode - 1) * 10 + gamemap);

    if (par)
        return par;
    else
    {
        // [BH] have no par time if this level is from a PWAD
        if (BTSX || (!canmodify && (!nerve || gamemap > 9) && !FREEDOOM))
            return 0;
        else if (gamemode == commercial)
        {
            // [BH] get correct par time for No Rest For The Living
            //  and have no par time for TNT and Plutonia
            if (gamemission == pack_nerve && gamemap <= 9)
                return npars[gamemap - 1];
            else if (gamemission == pack_tnt || gamemission == pack_plut)
                return 0;
            else
                return cpars[gamemap - 1];
        }
        else
            return pars[gameepisode][gamemap];
    }
}

static void G_DoCompleted(void)
{
    int map = (gameepisode - 1) * 10 + gamemap;
    int nextmap = P_GetMapNext(map);
    int secretnextmap = P_GetMapSecretNext(map);

    gameaction = ga_nothing;

    I_UpdateBlitFunc(false);

    if (vid_widescreen)
    {
        I_ToggleWidescreen(false);
        returntowidescreen = true;
        ST_Drawer(false, true);
    }

    I_Sleep(700);

    G_PlayerFinishLevel();      // take away cards and stuff

    if (automapactive)
        AM_Stop();
    else if (mapwindow)
        AM_ClearFB();

    if (chex && gamemap == 5)
    {
        gameaction = ga_victory;
        return;
    }

    if (gamemode != commercial)
        switch (gamemap)
        {
            case 8:
                // [BH] this episode is complete, so select the next episode in the menu
                if ((gamemode == registered && gameepisode < 3) || (gamemode == retail && gameepisode < 4 + sigil))
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

    wminfo.didsecret = viewplayer->didsecret;
    wminfo.epsd = gameepisode - 1;
    wminfo.last = gamemap - 1;

    if (gamemode == commercial)
    {
        if (secretexit && secretnextmap > 0)
            wminfo.next = secretnextmap - 1;
        else if (nextmap > 0)
            wminfo.next = nextmap - 1;
        else if (secretexit)
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
        if (secretexit && secretnextmap > 0)
            wminfo.next = secretnextmap - (gameepisode - 1) * 10 - 1;
        else if (nextmap > 0)
            wminfo.next = nextmap - (gameepisode - 1) * 10 - 1;
        else if (secretexit)
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
                case 5:
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
    wminfo.partime = G_GetParTime() * TICRATE;
    wminfo.skills = (totalkills ? viewplayer->killcount : 1);
    wminfo.sitems = (totalitems ? viewplayer->itemcount : 1);
    wminfo.ssecret = viewplayer->secretcount;
    wminfo.stime = leveltime;

    gamestate = GS_INTERMISSION;
    viewactive = false;
    automapactive = false;

    stat_mapscompleted = SafeAdd(stat_mapscompleted, 1);
    M_SaveCVARs();

    C_InputNoRepeat("exitmap");

    WI_Start(&wminfo);
}

//
// G_WorldDone
//
void G_WorldDone(void)
{
    char    *intertext = P_GetInterText(gamemap);
    char    *intersecrettext = P_GetInterSecretText(gamemap);

    gameaction = ga_worlddone;

    if (secretexit)
        viewplayer->didsecret = true;

    if (*intertext || (*intersecrettext && secretexit))
    {
        F_StartFinale();
        return;
    }

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

    if (quickSaveSlot >= 0 && autosave && !pistolstart)
        gameaction = ga_autosavegame;
}

void G_LoadGame(char *name)
{
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame;
}

void G_DoLoadGame(void)
{
    int savedleveltime;

    I_SetPalette(PLAYPAL);

    loadaction = gameaction;
    gameaction = ga_nothing;

    if (consolestrings < 2 || !M_StringStartsWith(console[consolestrings - 3].string, "load "))
        C_Input("load %s", savename);

    if (!(save_stream = fopen(savename, "rb")))
    {
        menuactive = false;
        C_ShowConsole();
        C_Warning(1, "<b>%s</b> couldn't be loaded.", savename);
        loadaction = ga_nothing;
        return;
    }

    if (!P_ReadSaveGameHeader(savedescription))
    {
        fclose(save_stream);
        loadaction = ga_nothing;
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
    P_RemoveCorruptMobjs();

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
        char        *temp = titlecase(savedescription);

        M_snprintf(buffer, sizeof(buffer), (loadaction == ga_autoloadgame ? s_GGAUTOLOADED : s_GGLOADED), temp);
        C_Output(buffer);
        HU_SetPlayerMessage(buffer, false, false);
        message_dontfuckwithme = true;
        free(temp);
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
        C_Warning(1, "<b>%s</b> couldn't be saved.", savegame_file);
    }
    else
    {
        char    *backup_savegame_file = M_StringJoin(savegame_file, ".bak", NULL);

        if (gameaction == ga_autosavegame)
        {
            M_UpdateSaveGameName(quickSaveSlot);
            M_StringCopy(savedescription, savegamestrings[quickSaveSlot], sizeof(savedescription));
        }

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
        // file, backing up the old savegame if there was one there.
        remove(backup_savegame_file);
        rename(savegame_file, backup_savegame_file);
        rename(temp_savegame_file, savegame_file);

        free(backup_savegame_file);

        savegames = true;

        if (!consolestrings || !M_StringStartsWith(console[consolestrings - 1].string, "save "))
            C_Input("save %s", savegame_file);

        if (consoleactive)
            C_Output("<b>%s</b> was saved.", savename);
        else
        {
            static char buffer[1024];
            char        *temp = titlecase(savedescription);

            M_snprintf(buffer, sizeof(buffer), (gameaction == ga_autosavegame ? s_GGAUTOSAVED : s_GGSAVED), temp);
            C_Output(buffer);
            HU_SetPlayerMessage(buffer, false, false);
            message_dontfuckwithme = true;
            free(temp);

            if (gameaction != ga_autosavegame)
                S_StartSound(NULL, sfx_swtchx);
        }

        viewplayer->gamessaved++;
        stat_gamessaved = SafeAdd(stat_gamessaved, 1);
        M_SaveCVARs();

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
    startingnewgame = true;
    infight = false;

    if (skill == sk_baby)
        stat_skilllevel_imtooyoungtodie = SafeAdd(stat_skilllevel_imtooyoungtodie, 1);
    else if (skill == sk_easy)
        stat_skilllevel_heynottoorough = SafeAdd(stat_skilllevel_heynottoorough, 1);
    else if (skill == sk_medium)
        stat_skilllevel_hurtmeplenty = SafeAdd(stat_skilllevel_hurtmeplenty, 1);
    else if (skill == sk_hard)
        stat_skilllevel_ultraviolence = SafeAdd(stat_skilllevel_ultraviolence, 1);
    else
        stat_skilllevel_nightmare = SafeAdd(stat_skilllevel_nightmare, 1);

    M_SaveCVARs();
}

//
// G_DeferredLoadLevel
// [BH] Called when the IDCLEV cheat is used.
//
void G_DeferredLoadLevel(skill_t skill, int ep, int map)
{
    d_skill = skill;
    d_episode = ep;
    d_map = map;
    gameaction = ga_loadlevel;
    infight = false;
    sector_list = NULL;

    for (int i = 0; i < NUMPOWERS; i++)
        if (viewplayer->powers[i] > 0)
            viewplayer->powers[i] = 0;
}

static void G_DoNewGame(void)
{
    I_SetPalette(PLAYPAL);

    if (vid_widescreen)
        I_ToggleWidescreen(true);

    st_facecount = ST_STRAIGHTFACECOUNT;
    G_InitNew(d_skill, d_episode, d_map);
    gameaction = ga_nothing;
    infight = false;
}

// killough 4/10/98: New function to fix bug which caused Doom
// lockups when idclev was used in conjunction with -fast.
void G_SetFastParms(int fast_pending)
{
    static int  fast = 0;                   // remembers fast state

    if (fast != fast_pending)               // only change if necessary
    {
        if ((fast = fast_pending))
        {
            for (int i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
                if (states[i].tics != 1)    // killough 4/10/98
                    states[i].tics >>= 1;   // don't change 1->0 since it causes cycles

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
        S_ResumeMusic();
    }

    if (skill > sk_nightmare)
        skill = sk_nightmare;

    if (ep < 1)
        ep = 1;

    if (gamemode == retail)
    {
        if (sigil)
        {
            if (ep > 5)
                ep = 5;
        }
        else if (ep > 4)
            ep = 4;
    }
    else if (gamemode == shareware)
    {
        if (ep > 1)
            ep = 1;     // only start episode 1 on shareware
    }

    if (map > 9 && gamemode != commercial)
        map = 9;

    // [BH] Fix <https://doomwiki.org/wiki/Demon_speed_bug>.
    G_SetFastParms(fastparm || skill == sk_nightmare);

    // force player to be initialized upon first level load
    viewplayer->playerstate = PST_REBORN;

    paused = false;
    automapactive = false;
    viewactive = true;
    gameepisode = ep;
    gamemap = map;
    gameskill = skill;

    if (consolestrings == 1
        || (!M_StringStartsWith(console[consolestrings - 2].string, "map ")
            && !M_StringStartsWith(console[consolestrings - 1].string, "load ")
            && !M_StringStartsWith(console[consolestrings - 1].string, "Warping ")))
        C_InputNoRepeat("newgame");

    G_DoLoadLevel();
}
